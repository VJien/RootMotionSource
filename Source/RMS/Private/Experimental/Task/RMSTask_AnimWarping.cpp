//  Copyright. VJ  All Rights Reserved.
//  https://supervj.top/2022/03/24/RootMotionSource/


#include "Experimental/Task/RMSTask_AnimWarping.h"

#include "Experimental/RMSComponent.h"

URMSTask_AnimWarping* URMSTask_AnimWarping::RootMotionSourceTask_AnimWarping(URMSComponent* RootMotionComponent, UAnimSequence* Anim, TMap<FName, FVector> InWarpingInfo)
{
	if (!RootMotionComponent || RootMotionComponent->GetMovementComponent() || InWarpingInfo.Num() == 0)
	{
		return nullptr;
	}

	URMSTask_AnimWarping* Task = NewRootMotionSourceTask<URMSTask_AnimWarping>(RootMotionComponent,TEXT("AnimWarping"), 0);
	Task->RootMotionComponent = RootMotionComponent;
	Task->ID = RootMotionComponent->TryActivateTask(Task);
	Task->WarpingInfo = InWarpingInfo;
	Task->Anim = Anim;
	RootMotionComponent->OnTaskEnd.AddDynamic(Task, &URMSTask_AnimWarping::OnTaskFinished);
	return Task;
}

void URMSTask_AnimWarping::Activate()
{
	Super::Activate();
}

void URMSTask_AnimWarping::Pause()
{
	Super::Pause();
}

void URMSTask_AnimWarping::Resume()
{
	Super::Resume();
}

void URMSTask_AnimWarping::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);
}

void URMSTask_AnimWarping::OnTaskFinished_Implementation(URMSTask_Base* TaskObject, bool bSuccess)
{
	Super::OnTaskFinished_Implementation(TaskObject, bSuccess);
	if (TaskObject)
	{
	}
}
