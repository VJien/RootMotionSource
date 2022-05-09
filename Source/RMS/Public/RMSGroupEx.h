//  Copyright. VJ  All Rights Reserved.
//  https://supervj.top/2022/03/24/RootMotionSource/

#pragma once

#include "CoreMinimal.h"
#include "RMSTypes.h"
#include "GameFramework/RootMotionSource.h"
#include "RMSGroupEx.generated.h"

USTRUCT()
struct RMS_API FRootMotionSource_PathMoveToForce: public FRootMotionSource
{
	GENERATED_USTRUCT_BODY()
	FRootMotionSource_PathMoveToForce();
	virtual ~FRootMotionSource_PathMoveToForce() {}

	UPROPERTY()
	FVector StartLocation = FVector::ZeroVector;
	UPROPERTY()
	FRotator StartRotation = FRotator::ZeroRotator;
	UPROPERTY()
	TArray<FRMSPathMoveToData> Path;

	
	FRMSPathMoveToData CurrData;
	FRMSPathMoveToData LastData;
	int32 Index = -1;

	
	FVector GetPathOffsetInWorldSpace(const float MoveFraction, FRMSPathMoveToData Data, FVector Start) const;

	bool GetPathDataByTime(float Time, FRMSPathMoveToData& OutCurrData, FRMSPathMoveToData& OutLastData)const;

	virtual bool UpdateStateFrom(const FRootMotionSource* SourceToTakeStateFrom, bool bMarkForSimulatedCatchup = false) override;
	
	virtual void PrepareRootMotion(
	float SimulationTime, 
	float MovementTickTime,
	const ACharacter& Character, 
	const UCharacterMovementComponent& MoveComponent
	) override;

	virtual bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) override;
	virtual FRootMotionSource* Clone() const override;

	virtual bool Matches(const FRootMotionSource* Other) const override;

	virtual bool MatchesAndHasSameState(const FRootMotionSource* Other) const override;
	virtual UScriptStruct* GetScriptStruct() const override;
	virtual FString ToSimpleString() const override;
	virtual void AddReferencedObjects(class FReferenceCollector& Collector) override;
	
};

USTRUCT()
struct RMS_API FRootMotionSource_MoveToForce_WithRotation: public FRootMotionSource_MoveToForce
{
	GENERATED_USTRUCT_BODY()
	FRootMotionSource_MoveToForce_WithRotation();
	virtual ~FRootMotionSource_MoveToForce_WithRotation() {}

	virtual void PrepareRootMotion(
		float SimulationTime, 
		float MovementTickTime,
		const ACharacter& Character, 
		const UCharacterMovementComponent& MoveComponent
		) override;
	UPROPERTY()
	FRMSRotationSetting RotationSetting;
	UPROPERTY()
	FRotator StartRotation = FRotator::ZeroRotator;
protected:
	// UPROPERTY()
	// FRotator TargetRotation = FRotator::ZeroRotator;
	
};
USTRUCT()
struct RMS_API FRootMotionSource_MoveToDynamicForce_WithRotation: public FRootMotionSource_MoveToDynamicForce
{
	GENERATED_USTRUCT_BODY()
	FRootMotionSource_MoveToDynamicForce_WithRotation(){};
	virtual ~FRootMotionSource_MoveToDynamicForce_WithRotation() {}

	virtual void PrepareRootMotion(
		float SimulationTime, 
		float MovementTickTime,
		const ACharacter& Character, 
		const UCharacterMovementComponent& MoveComponent
		) override;
	UPROPERTY()
	FRMSRotationSetting RotationSetting;								
	UPROPERTY()
	FRotator StartRotation = FRotator::ZeroRotator;
	
};

USTRUCT()
struct RMS_API FRootMotionSource_AnimWarping: public FRootMotionSource
{
	GENERATED_USTRUCT_BODY()
	FRootMotionSource_AnimWarping(){};
	virtual ~FRootMotionSource_AnimWarping() {}

	

	UPROPERTY()
	TObjectPtr<UAnimSequenceBase> Animation = nullptr;
	UPROPERTY()
	FVector StartLocation = FVector::ZeroVector;
	UPROPERTY()
	FRotator StartRotation = FRotator::ZeroRotator;
	UPROPERTY()
	FRMSRotationSetting RotationSetting;
	
	UPROPERTY()
	float AnimStartTime = 0;
	UPROPERTY()
	float AnimEndTime = -1;
	UPROPERTY()
	bool bIgnoreZAxis = false;
protected:
	UPROPERTY()
	FRotator CachedRotation = FRotator::ZeroRotator;
	UPROPERTY()
	FVector CachedTarget = FVector::ZeroVector;
	UPROPERTY()
	bool bInit = false;
	UPROPERTY()
	float CachedEndTime = -1;
	
	FORCEINLINE float GetCurrentAnimEndTime()const
	{
		return CachedEndTime <= 0? AnimEndTime: CachedEndTime;
	};
	FORCEINLINE FRotator GetTargetRotation() const
	{
		return CachedRotation;
	};
	void SetCurrentAnimEndTime(float NewEndTime)
	{
		CachedEndTime = NewEndTime;
	}
	void SetTargetRotation(FRotator Rotation)
	{
		CachedRotation = Rotation;
	}
public:
	void SetTargetLocation(FVector Target)
	{
		CachedTarget = Target;
	}
	FORCEINLINE FVector GetTargetLocation() const
	{
		return CachedTarget;
	};
	
	virtual FTransform ProcessRootMotion(const ACharacter& Character,const FTransform& InRootMotion,  float InPreviousTime, float InCurrentTime, float DeltaSeconds);
	FQuat WarpRotation(const ACharacter& Character, const FTransform& RootMotionDelta, const FTransform& RootMotionTotal, float TimeRemaining, float DeltaSeconds);

	
	virtual bool UpdateStateFrom(const FRootMotionSource* SourceToTakeStateFrom, bool bMarkForSimulatedCatchup = false) override;
	
	virtual void PrepareRootMotion(
	float SimulationTime, 
	float MovementTickTime,
	const ACharacter& Character, 
	const UCharacterMovementComponent& MoveComponent
	) override;

	FTransform ExtractRootMotion(float StartTime, float EndTime) const;
	
	virtual bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) override;
	virtual FRootMotionSource* Clone() const override;

	virtual bool Matches(const FRootMotionSource* Other) const override;

	virtual bool MatchesAndHasSameState(const FRootMotionSource* Other) const override;
	virtual UScriptStruct* GetScriptStruct() const override;
	virtual FString ToSimpleString() const override;
	virtual void AddReferencedObjects(class FReferenceCollector& Collector) override;

protected:

	
};

USTRUCT()
struct RMS_API FRootMotionSource_AnimWarping_FinalPoint: public FRootMotionSource_AnimWarping
{
	GENERATED_USTRUCT_BODY()
	FRootMotionSource_AnimWarping_FinalPoint(){};
	virtual ~FRootMotionSource_AnimWarping_FinalPoint() {}

	
	UPROPERTY()
	FVector TargetLocation = FVector::ZeroVector;
	UPROPERTY()
	FRotator TargetRotation = FRotator::ZeroRotator;

public:
	
	virtual void PrepareRootMotion(
	float SimulationTime, 
	float MovementTickTime,
	const ACharacter& Character, 
	const UCharacterMovementComponent& MoveComponent
	) override;


	
	virtual bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) override;
	virtual FRootMotionSource* Clone() const override;

	virtual bool Matches(const FRootMotionSource* Other) const override;

	virtual UScriptStruct* GetScriptStruct() const override;
	virtual FString ToSimpleString() const override;

	
};


USTRUCT()
struct RMS_API FRootMotionSource_AnimWarping_MultiTargets: public FRootMotionSource_AnimWarping
{
	GENERATED_USTRUCT_BODY()
	FRootMotionSource_AnimWarping_MultiTargets(){};
	virtual ~FRootMotionSource_AnimWarping_MultiTargets() {}

	
	UPROPERTY()
	TArray<FRMSTarget> TriggerDatas;
protected:
	UPROPERTY()
	FRMSTarget CurrTriggerData;
	UPROPERTY()
	FRMSTarget LastTriggerData;



	bool UpdateTriggerTarget(float SimulationTime, float TimeScale);
public:
	
	virtual void PrepareRootMotion(
	float SimulationTime, 
	float MovementTickTime,
	const ACharacter& Character, 
	const UCharacterMovementComponent& MoveComponent
	) override;


	
	virtual bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) override;
	virtual FRootMotionSource* Clone() const override;

	virtual bool Matches(const FRootMotionSource* Other) const override;

	virtual UScriptStruct* GetScriptStruct() const override;
	virtual FString ToSimpleString() const override;

	
};




