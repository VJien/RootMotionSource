// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RootMotionSourceLibrary.h"
#include "RootMotionSourceTask_Base.h"
#include "RootMotionSourceTask_MoveTo.generated.h"


class URootMotionSourceComponent;
UCLASS()
class RMS_API URootMotionSourceTask_MoveTo : public URootMotionSourceTask_Base
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category=RootMotionSource, meta = (BlueprintInternalUseOnly = "TRUE"))
	static URootMotionSourceTask_MoveTo* RootMotionSourceTask_MoveTo(URootMotionSourceComponent* RootMotionComponent,FName RmsInstanceName,
												   FVector StartLocation, FVector TargetLocation, float Duration,
												   int32 Priority,
												   FRootMotionSourceMoveSetting Setting,
												   UCurveVector* PathOffsetCurve = nullptr);

	
	virtual void Activate() override;
	virtual void Pause() override;
	virtual void Resume() override;
	virtual void TickTask(float DeltaTime) override;

	virtual  void OnTaskFinished_Implementation(URootMotionSourceTask_Base* TaskObject,  bool bSuccess) override;;
	

	
	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FRMSDlg OnSuccess;
	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FRMSDlg OnFail;
    	
	UPROPERTY()
	TWeakObjectPtr<UCharacterMovementComponent> MovementComp = nullptr;
	UPROPERTY(BlueprintReadOnly)
	FVector StartLocation;
	UPROPERTY(BlueprintReadOnly)
	FVector TargetLocation;
	UPROPERTY(BlueprintReadOnly)
	float Duration;
	UPROPERTY(BlueprintReadOnly)
	int32 Priority;
	UPROPERTY(BlueprintReadOnly)
	UCurveVector* PathOffsetCurve = nullptr;
	UPROPERTY(BlueprintReadOnly)
	FRootMotionSourceMoveSetting Setting;

};
