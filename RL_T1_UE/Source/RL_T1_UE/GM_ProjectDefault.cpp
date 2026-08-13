// Fill out your copyright notice in the Description page of Project Settings.


#include "GM_ProjectDefault.h"

AGM_ProjectDefault::AGM_ProjectDefault()
{
}

AGM_ProjectDefault::~AGM_ProjectDefault()
{
}

void AGM_ProjectDefault::BeginPlay()
{
	Super::BeginPlay();

	MapSharedMemory();
}

void AGM_ProjectDefault::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	UnmapSharedMemory();
}

void AGM_ProjectDefault::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (nullptr != SharedMemoryRegion) {
		void* SharedMemory_Addr = SharedMemoryRegion->GetAddress();
		FSharedMemory_T1* SharedMemory = static_cast<FSharedMemory_T1*>(SharedMemory_Addr);

		if (SharedMemory->ActionReady == 1 && SharedMemory->Stop != 1) { // 선택완료
			int32 ChosedNumber = SharedMemory->Action; // model 이 뭐 선택했는지.

			const float Probability = ChosedNumber == 0 ? LeftProbability : RightProbability;

			const float RandomVal = FMath::FRand();

			const bool bSuccess = RandomVal < Probability;

			SharedMemory->ActionReady = 0.0f;
			SharedMemory->Reward = bSuccess ? 1.0f : 0.0f;
			SharedMemory->RewardReady = 1.0f;

			UE_LOG(LogTemp, Log, TEXT("[%d] ChoseNumber : %d, [%.2f/%.2f] -> %s"),SharedMemory->Episode, ChosedNumber, RandomVal, Probability, bSuccess ? TEXT("Success") : TEXT("Failed"));

		}
	}
}

void AGM_ProjectDefault::MapSharedMemory()
{
	SharedMemoryRegion = FPlatformMemory::MapNamedSharedMemoryRegion(
		TEXT("shared_mem_RL_1"),		// 자동으로 global\ 이 붙는다
		false, // true 면 생성, false 면 기존 메모리 읽기
		FPlatformMemory::ESharedMemoryAccess::Read | FPlatformMemory::ESharedMemoryAccess::Write,
		sizeof(FSharedMemory_T1)
	);

	if (SharedMemoryRegion) {
		// 성공시

		UE_LOG(LogTemp, Log, TEXT("Mapped SharedMemory"));
		DebugSharedMemory(TEXT("MappedSharedMemory Begin"));
	}
	else {
		// 실패시
		UE_LOG(LogTemp, Warning, TEXT("Failed to open shared memory"));
	}
}

void AGM_ProjectDefault::UnmapSharedMemory()
{
	if (!SharedMemoryRegion) {
		return;
	}

	FPlatformMemory::UnmapNamedSharedMemoryRegion(SharedMemoryRegion);
	SharedMemoryRegion = nullptr;
}

void AGM_ProjectDefault::DebugSharedMemory(const FString& MsgId)
{

	void* SharedMemAddr = SharedMemoryRegion->GetAddress();
	auto* SharedMem = static_cast<FSharedMemory_T1*>(SharedMemAddr);

	if (nullptr != SharedMem) {
		FString FinalMsg  = FString::Printf(
			TEXT("\n[%s]\n--FSharedMemory--\nAction:{%d}\nReward:{%.1f}\nActionReady:{%d}\nRewardReady:{%d}\nEpisode:{%d}\nStop:{%d}\n\n"), 
			*MsgId,
			SharedMem->Action,
			SharedMem->Reward,
			SharedMem->ActionReady,
			SharedMem->RewardReady,
			SharedMem->Episode,
			SharedMem->Stop
		);

		UE_LOG(LogTemp, Log, TEXT("%s"), *FinalMsg);
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FinalMsg);
	}

}
