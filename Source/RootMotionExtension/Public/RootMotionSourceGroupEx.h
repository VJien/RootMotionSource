// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RootMotionSourceTypes.h"
#include "GameFramework/RootMotionSource.h"
#include "RootMotionSourceGroupEx.generated.h"

USTRUCT()
struct ROOTMOTIONEXTENSION_API FRootMotionSource_PathMoveToForce: public FRootMotionSource
{
	GENERATED_USTRUCT_BODY()
	FRootMotionSource_PathMoveToForce();
	virtual ~FRootMotionSource_PathMoveToForce() {}

	UPROPERTY()
	FVector StartLocation = FVector::ZeroVector;
	UPROPERTY()
	TArray<FRootMotionSourcePathMoveToData> Path;
	FRootMotionSourcePathMoveToData CurrData;
	FRootMotionSourcePathMoveToData LastData;
	int32 Index = -1;

	
	FVector GetPathOffsetInWorldSpace(const float MoveFraction, FRootMotionSourcePathMoveToData Data, FVector Start) const;

	bool GetPathDataByTime(float Time, FRootMotionSourcePathMoveToData& OutCurrData, FRootMotionSourcePathMoveToData& OutLastData)const;

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

