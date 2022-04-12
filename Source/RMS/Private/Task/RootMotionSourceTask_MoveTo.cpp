// Fill out your copyright notice in the Description page of Project Settings.


#include "Task/RootMotionSourceTask_MoveTo.h"

#include "RootMotionSourceComponent.h"

URootMotionSourceTask_MoveTo* URootMotionSourceTask_MoveTo::RootMotionSourceTask_MoveTo(URootMotionSourceComponent* RootMotionComponent, FName InstanceName,
												   FVector StartLocation, FVector TargetLocation, float Duration,
												   int32 Priority,
												   FRootMotionSourceMoveSetting Setting,
												   UCurveVector* PathOffsetCurve)
{
	if (!RootMotionComponent->GetMovementComponent())
	{
		return nullptr;
	}

	URootMotionSourceTask_MoveTo* Task = NewRootMotionSourceTask<URootMotionSourceTask_MoveTo>(RootMotionComponent,InstanceName,Priority);
	Task->StartLocation = StartLocation;
	Task->TargetLocation = TargetLocation;
	Task->Duration = Duration;
	Task->Priority = Priority;
	Task->PathOffsetCurve = PathOffsetCurve;
	Task->Setting = Setting;
	Task->RootMotionComponent = RootMotionComponent;
	Task->ID = RootMotionComponent->TryActivateTask(Task);
	RootMotionComponent->OnTaskEnd.AddDynamic(Task, &URootMotionSourceTask_MoveTo::OnTaskFinished);
	return Task;
}



void URootMotionSourceTask_MoveTo::Activate()
{
	Super::Activate();
}

void URootMotionSourceTask_MoveTo::Pause()
{
	Super::Pause();
}

void URootMotionSourceTask_MoveTo::Resume()
{
	Super::Resume();
}

void URootMotionSourceTask_MoveTo::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);
}

void URootMotionSourceTask_MoveTo::OnTaskFinished_Implementation(URootMotionSourceTask_Base* TaskObject, bool bSuccess)
{
	Super::OnTaskFinished_Implementation(TaskObject, bSuccess);
	if (RootMotionComponent.IsValid())
	{
		RootMotionComponent->OnTaskEnd.RemoveDynamic(this, &URootMotionSourceTask_Base::OnTaskFinished);
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
