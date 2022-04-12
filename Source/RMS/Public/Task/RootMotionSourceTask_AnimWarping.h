// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RootMotionSourceLibrary.h"
#include "RootMotionSourceTask_Base.h"
#include "RootMotionSourceTask_AnimWarping.generated.h"


class URootMotionSourceComponent;
UCLASS()
class RMS_API URootMotionSourceTask_AnimWarping : public URootMotionSourceTask_Base
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category=RootMotionSource, meta = (BlueprintInternalUseOnly = "TRUE"))
	static URootMotionSourceTask_AnimWarping* RootMotionSourceTask_AnimWarping(URootMotionSourceComponent* RootMotionComponent, UAnimSequence*Anim,  TMap<FName, FVector> WarpingInfo);

	
	virtual void Activate() override;
	virtual void Pause() override;
	virtual void Resume() override;
	virtual void TickTask(float DeltaTime) override;

	virtual  void OnTaskFinished_Implementation(URootMotionSourceTask_Base* TaskObject,  bool bSuccess) override;;
	



	UPROPERTY()
	TWeakObjectPtr<UCharacterMovementComponent> MovementComp = nullptr;

	TMap<FName, FVector> WarpingInfo;
	TWeakObjectPtr<UAnimSequence> Anim = nullptr;

};
