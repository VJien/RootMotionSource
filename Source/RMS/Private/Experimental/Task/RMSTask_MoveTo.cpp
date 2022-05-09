//  Copyright. VJ  All Rights Reserved.
//  https://supervj.top/2022/03/24/RootMotionSource/


#include "Experimental/Task/RMSTask_MoveTo.h"

#include "Experimental/RMSComponent.h"

URMSTask_MoveTo* URMSTask_MoveTo::RootMotionSourceTask_MoveTo(URMSComponent* RootMotionComponent, FName InstanceName,
												   FVector StartLocation, FVector TargetLocation, float Duration,
												   int32 Priority,
												   FRMSSetting_Move Setting,
												   UCurveVector* PathOffsetCurve)
{
	if (!RootMotionComponent->GetMovementComponent())
	{
		return nullptr;
	}

	URMSTask_MoveTo* Task = NewRootMotionSourceTask<URMSTask_MoveTo>(RootMotionComponent,InstanceName,Priority);
	Task->StartLocation = StartLocation;
	Task->TargetLocation = TargetLocation;
	Task->Duration = Duration;
	Task->Priority = Priority;
	Task->PathOffsetCurve = PathOffsetCurve;
	Task->Setting = Setting;
	Task->RootMotionComponent = RootMotionComponent;
	Task->ID = RootMotionComponent->TryActivateTask(Task);
	RootMotionComponent->OnTaskEnd.AddDynamic(Task, &URMSTask_MoveTo::OnTaskFinished);
	return Task;
}



void URMSTask_MoveTo::Activate()
{
	Super::Activate();
}

void URMSTask_MoveTo::Pause()
{
	Super::Pause();
}

void URMSTask_MoveTo::Resume()
{
	Super::Resume();
}

void URMSTask_MoveTo::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);
}

void URMSTask_MoveTo::OnTaskFinished_Implementation(URMSTask_Base* TaskObject, bool bSuccess)
{
	Super::OnTaskFinished_Implementation(TaskObject, bSuccess);
	if (RootMotionComponent.IsValid())
	{
		RootMotionComponent->OnTaskEnd.RemoveDynamic(this, &URMSTask_Base::OnTaskFinished);
	}
	if (bSuccess)
	{
		OnSuccess.Broadcast(this);
	}
	else
	{
		OnFail.Broadcast(this);
	}
	
	EndTask();
}
