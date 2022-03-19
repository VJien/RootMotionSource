// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTasksComponent.h"

#include "RootMotionSourceLibrary.h"

#include "RootMotionSourceComponent.generated.h"


// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMoveToDlg, FRMS_MoveTo, SourceSetting);
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDynamicMoveToDlg, FRMS_DynamicMoveTo, SourceSetting);
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FJumpToDlg, FRMS_Jump, SourceSetting);

class URootMotionSourceTask_Base;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FTaskDlg, URootMotionSourceTask_Base*, TaskObject, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTaskDlg1p, URootMotionSourceTask_Base*, TaskObject);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class ROOTMOTIONEXTENSION_API URootMotionSourceComponent : public UGameplayTasksComponent
{
	GENERATED_UCLASS_BODY()
public:
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category=RootMotionSourceComponent)
	void Init(UCharacterMovementComponent* MovementComponent);
	UFUNCTION(BlueprintCallable, Category=RootMotionSourceComponent)
	int32 TryActivateTask(URootMotionSourceTask_Base* Task);

	UFUNCTION(BlueprintCallable, Category=RootMotionSourceComponent)
	void SetRms_TargetByLocation(FName Instance, FVector Location);
	UFUNCTION(BlueprintCallable, Category=RootMotionSourceComponent)
	void SetRms_TargetByRotation(FName Instance, FRotator Rotation);
	UFUNCTION(BlueprintCallable, Category=RootMotionSourceComponent)
	void SetRms_Target(FName Instance, FVector Location, FRotator Rotation);

public:
	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FTaskDlg1p OnTaskBegin;
	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FTaskDlg OnTaskEnd;
	
	UCharacterMovementComponent* GetMovementComponent() const
	{
		return MovementComponent.Get();
	}
	
protected:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=RootMotionSource)
	bool bListenTaskEnd = false;
	
	UPROPERTY()
	TMap<int32, bool> DirtIDs;

	UPROPERTY(BlueprintReadWrite)
	TMap<int32, URootMotionSourceTask_Base*> CurrentTasks;
	
	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<UCharacterMovementComponent> MovementComponent = nullptr;


	UPROPERTY(BlueprintReadWrite)
	TMap<FName, FTransform> RMSMap;
};
