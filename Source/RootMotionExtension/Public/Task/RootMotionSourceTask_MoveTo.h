// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RootMotionSourceLibrary.h"
#include "RootMotionSourceTask_Base.h"
#include "RootMotionSourceTask_MoveTo.generated.h"
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMoveToDlg, FRMS_MoveTo, SourceSetting);

class URootMotionSourceComponent;
UCLASS()
class ROOTMOTIONEXTENSION_API URootMotionSourceTask_MoveTo : public URootMotionSourceTask_Base
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category=RootMotionSource, meta = (BlueprintInternalUseOnly = "TRUE"))
	static URootMotionSourceTask_MoveTo* RootMotionSourceTask_MoveTo(URootMotionSourceComponent* RootMotionComponent, FRMS_MoveTo Setting);

	
	virtual void Activate() override;
	virtual void Pause() override;
	virtual void Resume() override;
	virtual void TickTask(float DeltaTime) override;

	virtual  void OnTaskFinished_Implementation(URootMotionSourceTask_Base* TaskObject,  bool bSuccess) override;;
	

	
	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FMoveToDlg OnSuccess;
	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FMoveToDlg OnFail;
    	
	UPROPERTY()
	TWeakObjectPtr<UCharacterMovementComponent> MovementComp = nullptr;
	UPROPERTY()
	FRMS_MoveTo SourceSetting;

};
