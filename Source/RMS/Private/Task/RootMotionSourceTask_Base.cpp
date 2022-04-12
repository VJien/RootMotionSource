// Fill out your copyright notice in the Description page of Project Settings.


#include "Task/RootMotionSourceTask_Base.h"


void URootMotionSourceTask_Base::OnTaskFinished_Implementation(URootMotionSourceTask_Base* TaskObject,  bool bSuccess)
{
}

URootMotionSourceTask_Base::URootMotionSourceTask_Base(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
	bTickingTask = true;
}
