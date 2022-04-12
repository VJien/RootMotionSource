// Fill out your copyright notice in the Description page of Project Settings.


#include "Task/RootMotionSourceTask_AnimWarping.h"

#include "RootMotionSourceComponent.h"

URootMotionSourceTask_AnimWarping* URootMotionSourceTask_AnimWarping::RootMotionSourceTask_AnimWarping(URootMotionSourceComponent* RootMotionComponent, UAnimSequence* Anim, TMap<FName, FVector> InWarpingInfo)
{
	if (!RootMotionComponent || RootMotionComponent->GetMovementComponent() || InWarpingInfo.Num() == 0)
	{
		return nullptr;
	}

	URootMotionSourceTask_AnimWarping* Task = NewRootMotionSourceTask<URootMotionSourceTask_AnimWarping>(RootMotionComponent,TEXT("AnimWarping"), 0);
	Task->RootMotionComponent = RootMotionComponent;
	Task->ID = RootMotionComponent->TryActivateTask(Task);
	Task->WarpingInfo = InWarpingInfo;
	Task->Anim = Anim;
	RootMotionComponent->OnTaskEnd.AddDynamic(Task, &URootMotionSourceTask_AnimWarping::OnTaskFinished);
	return Task;
}

void URootMotionSourceTask_AnimWarping::Activate()
{
	Super::Activate();
}

void URootMotionSourceTask_AnimWarping::Pause()
{
	Super::Pause();
}

void URootMotionSourceTask_AnimWarping::Resume()
{
	Super::Resume();
}

void URootMotionSourceTask_AnimWarping::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);
}

void URootMotionSourceTask_AnimWarping::OnTaskFinished_Implementation(URootMotionSourceTask_Base* TaskObject, bool bSuccess)
{
	Super::OnTaskFinished_Implementation(TaskObject, bSuccess);
	if (TaskObject)
	{
	}
}
