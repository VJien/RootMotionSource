// Fill out your copyright notice in the Description page of Project Settings.


#include "Experimental/Task/RMSTask_Base.h"


void URMSTask_Base::OnTaskFinished_Implementation(URMSTask_Base* TaskObject,  bool bSuccess)
{
}

URMSTask_Base::URMSTask_Base(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
	bTickingTask = true;
}
