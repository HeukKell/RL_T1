// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"

#include "HAL/PlatformMemory.h"
#include "HAL/UnrealMemory.h"

#include "GM_ProjectDefault.generated.h"

// 1byte 씩 정렬
#pragma pack(push, 1)
struct FSharedMemory_T1
{
	int32 Action;		// python 이 선택한 상자 : 0 또는 1
	float Reward;		// UE 가 돌려준 보상 : 0 또는 1

	int32 ActionReady; // Python 의 행동 기록여부
	int32 RewardReady; // UE의 보상 기록 여부

	int32 Episode;		// 현재 시행 번호
	int32 Stop;			// 학습 중단 여부

};
#pragma pack(pop) // 1byte 정렬 끝
/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class RL_T1_UE_API AGM_ProjectDefault : public AGameMode
{
	GENERATED_BODY()
	
	
	AGM_ProjectDefault();

	~AGM_ProjectDefault();

protected:

	/** Overridable native event for when play begins for this actor. */
	virtual void BeginPlay() override;

	/** Overridable function called whenever this actor is being removed from a level */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void Tick(float DeltaSeconds) override;

protected:

	FPlatformMemory::FSharedMemoryRegion* SharedMemoryRegion = nullptr;

public:

	UFUNCTION(BlueprintCallable)
	void MapSharedMemory();

	UFUNCTION(BlueprintCallable)
	void UnmapSharedMemory();


public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float LeftProbability = 0.3f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float RightProbability = 0.8f;

public:

	UFUNCTION(BlueprintCallable)
	void DebugSharedMemory(const FString& MsgId);
};
