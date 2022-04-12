// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_RMS.generated.h"

/**
 * 
 */
UCLASS()
class RMS_API UAnimNotifyState_RMS_Warping : public UAnimNotifyState
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RootMotionSourceTarget = NAME_None;
};
