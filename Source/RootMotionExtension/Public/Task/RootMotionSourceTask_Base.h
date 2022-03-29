// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTask.h"
#include "RootMotionSourceComponent.h"
#include "RootMotionSourceTask_Base.generated.h"
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRMSDlg, URootMotionSourceTask_Base*, Object);

class URootMotionSourceComponent;
UCLASS()
class ROOTMOTIONEXTENSION_API URootMotionSourceTask_Base : public UGameplayTask
{
	GENERATED_BODY()
public:
	URootMotionSourceTask_Base(const FObjectInitializer& ObjectInitializer);
	UFUNCTION(Blueprintcallable, BlueprintNativeEvent)
	void OnTaskFinished(URootMotionSourceTask_Base* TaskObject, bool bSuccess);
	
	UPROPERTY(BlueprintReadWrite)
	int32 ID = -1;

	TWeakObjectPtr<URootMotionSourceComponent> RootMotionComponent = nullptr;

	template <class T>
	static T* NewRootMotionSourceTask(URootMotionSourceComponent* RootMotionComponent, FName InstanceName = FName(), int32 Priority = 0)
	{
		check(RootMotionComponent);

		T* MyObj = NewObject<T>();
		MyObj->InitTask(*RootMotionComponent, Priority);
		MyObj->InstanceName = InstanceName;
		return MyObj;
	}
};
