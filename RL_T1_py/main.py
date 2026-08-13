import torch
import torch.nn as nn
from torch.utils.data import TensorDataset
from torch.utils.data import DataLoader

from multiprocessing import shared_memory
import struct
import random
import time

SHARED_MEMORY_NAME = "Global\\shared_mem_RL_1"
SHARED_MEMORY_SIZE = 24

device = "cuda" if torch.cuda.is_available() else "cpu"

# 어느쪽 상자가 더 좋은지 학습하는 Agent
class Agent : 

    def __init__(self) :
        self.q_values = torch.zeros(2) # torch([0,0])

        # 어느 상자 선택했는지 횟수 기록 (0번상자, 1번상자)
        self.action_counts = torch.zeros(2, dtype=torch.int32)
        self.epsilon = 0.1

    def select_action(self):

        # 10 % 확률로 무작위 탐색
        if random.random() < self.epsilon:
            return random.randrange(2) # 0 또는 1

        # 90% 확률로 현재 더 좋다고 생각하는 상자 선택
        # 점수를 많이 얻은 상자.
        return torch.argmax(self.q_values).item() # 0 또는 1

    def learn(self, action, reward):

        # 어느쪽을 선택했는지 기록
        self.action_counts[action] += 1

        # TODO 확인해봐야해
        count = self.action_counts[action].item()

        learning_rate = 1.0 / count # count 값이 높으면 높을수록 . 학습률이 떨어진다? 보상을 받아도 영향이 적어진다.

        self.q_values[action] += (learning_rate *(reward - self.q_values[action]))

class SharedMem_Manager:

    def __init__(self):
        self.shared_mem = None
        self.shared_mem = shared_memory.SharedMemory(
                name = SHARED_MEMORY_NAME,
                create = True,
                size = SHARED_MEMORY_SIZE
            )

    def copyToSharedMemory(self, action : int, reward: int, actionReady : int, rewardReady : int, episode:int, stop : int) :
        bytes = struct.pack("<ifiiii", action,reward,actionReady,rewardReady,episode, stop)
        self.shared_mem.buf[0:24] = bytes

    def receive_reward_from_ue(self, newAction : int) :
        shared_bytes = bytes(self.shared_mem.buf[0:24])

        action, reward, actionReady, rewardReady, episode, stop = struct.unpack('<ifiiii', shared_bytes)

        if stop == 1 : 
            stop = 0

        action = newAction
        actionReady = 1
        rewardReady = 0

        self.copyToSharedMemory(action,reward, actionReady, rewardReady, episode, stop)

        res_reward = 0.0

        print(f"wating...{episode}")
        while True : 

            curr_bytes = bytes(self.shared_mem.buf[0:24])

            res_action, res_reward, res_actionReady, res_rewardReady, res_episode, res_stop = struct.unpack('<ifiiii',curr_bytes)

            if res_rewardReady == 1 :
                print("got reward")

                res_rewardReady = 0
                res_episode += 1

                self.copyToSharedMemory(res_action, res_reward, res_actionReady, res_rewardReady, res_episode, res_stop)
                self.DebugSharedMemory()                
                break;
            

            time.sleep(0.02)

        return res_reward
            
    def DebugSharedMemory(self) :

        curr_bytes = bytes(self.shared_mem.buf[0:24]) 

        action, reward, actionReady, rewardReady, episode, stop = struct.unpack("<ifiiii", curr_bytes)

        print(f"action:{action}, reward:{reward}, actionReady:{actionReady}, rewardReady:{rewardReady}, episode:{episode}, stop:{stop}")

    def StopReward(self) :
        self.shared_mem.buf[20:24] = struct.pack('<i', 1)


def main():

    print("begin main")
    agent = Agent()
    shm = SharedMem_Manager()

    try : 

        for episode in range(1001) :
                

            # 숫자 골라. 0 또는 1
            action = agent.select_action()

            reward = shm.receive_reward_from_ue(action)

            agent.learn(action, reward)

            if episode % 100 == 0:

                print(
                    f"Episode: {episode}, "
                    f"Q : {agent.q_values}, "
                    f"count : {agent.action_counts}"
                )
                shm.DebugSharedMemory()

                # 파일에도 로깅
                deugstr = ""
                with open("./log.txt", "rt", encoding="utf-8") as file :
                    debugstr = file.read()
                    
                with open("./log.txt", "+tw", encoding="utf-8") as file :
                    file.write(debugstr)
                    file.write( f"Episode : {episode}, Q: {agent.q_values}, count : {agent.action_counts}. \n")

        shm.StopReward()

    finally:

        if shm.shared_mem != None:

            # TODO Stop 보내기

            # 현재 python 에서 연결 해제
            shm.shared_mem.close()

            # 운영체제에서 공유 메모리 이름 제거
            shm.shared_mem.unlink()


    a = input()
    

if __name__ == "__main__":
    main()
