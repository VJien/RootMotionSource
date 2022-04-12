// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTask.h"
#include "Experimental/RMSComponent.h"
#include "RMSTask_Base.generated.h"
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRMSDlg, URMSTask_Base*, Object);

class URMSComponent;
UCLASS()
class RMS_API URMSTask_Base : public UGameplayTask
{
	GENERATED_BODY()
public:
	URMSTask_Base(const FObjectInitializer& ObjectInitializer);
	UFUNCTION(Blueprintcallable, BlueprintNativeEvent)
	void OnTaskFinished(URMSTask_Base* TaskObject, bool bSuccess);
	
	UPROPERTY(BlueprintReadWrite)
	int32 ID = -1;

	TWeakObjectPtr<URMSComponent> RootMotionComponent = nullptr;

	template <class T>
	static T* NewRootMotionSourceTask(URMSComponent* RootMotionComponent, FName InstanceName = FName(), int32 Priority = 0)
	{
		check(RootMotionComponent);

		T* MyObj = NewObject<T>();
		MyObj->InitTask(*RootMotionComponent, Priority);
		MyObj->InstanceName = InstanceName;
		return MyObj;
	}
};
