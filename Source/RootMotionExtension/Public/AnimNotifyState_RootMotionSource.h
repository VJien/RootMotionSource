// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_RootMotionSource.generated.h"

/**
 * 
 */
UCLASS()
class ROOTMOTIONEXTENSION_API UAnimNotifyState_RootMotionSource : public UAnimNotifyState
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RootMotionSourceInstance = NAME_None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Priority = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bWarpLocation = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bWarpRotation = true;
};
