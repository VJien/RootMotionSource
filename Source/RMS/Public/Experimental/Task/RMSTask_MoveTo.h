// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RMSLibrary.h"
#include "RMSTask_Base.h"
#include "RMSTask_MoveTo.generated.h"


class URMSComponent;
UCLASS()
class RMS_API URMSTask_MoveTo : public URMSTask_Base
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category=RootMotionSource, meta = (BlueprintInternalUseOnly = "TRUE"))
	static URMSTask_MoveTo* RootMotionSourceTask_MoveTo(URMSComponent* RootMotionComponent,FName RmsInstanceName,
												   FVector StartLocation, FVector TargetLocation, float Duration,
												   int32 Priority,
												   FRMSSetting_Move Setting,
												   UCurveVector* PathOffsetCurve = nullptr);

	
	virtual void Activate() override;
	virtual void Pause() override;
	virtual void Resume() override;
	virtual void TickTask(float DeltaTime) override;

	virtual  void OnTaskFinished_Implementation(URMSTask_Base* TaskObject,  bool bSuccess) override;;
	

	
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
	FRMSSetting_Move Setting;

};
