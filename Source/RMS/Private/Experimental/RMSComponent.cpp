// Fill out your copyright notice in the Description page of Project Settings.


#include "Experimental/RMSComponent.h"

#include "Experimental/Task/RMSTask_MoveTo.h"

// Sets default values for this component's properties
URMSComponent::URMSComponent(const FObjectInitializer& Object):Super(Object)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	// ...
}


// Called when the game starts
void URMSComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void URMSComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (bListenTaskEnd)
	{
		//@todo 除非改引擎, 不然没有更好的办法来监听RMS的结束, 只能暴力tick
		if (MovementComponent.IsValid())
		{
			for (auto P: CurrentTasks)
			{
				auto RMS = MovementComponent->GetRootMotionSourceByID(P.Key);
				if (!RMS.IsValid())
				{
					DirtIDs.Emplace(P.Key, true);
				}
			}

		}
		for (auto P: DirtIDs)
		{
			if(const auto task = CurrentTasks.Find(P.Key))
			{
				OnTaskEnd.Broadcast(*task,P.Value);
				CurrentTasks.Remove(P.Key);
			}
		}
		DirtIDs.Empty();
		// ...
	}

}

void URMSComponent::Init(UCharacterMovementComponent* InMovementComponent)
{
	MovementComponent = InMovementComponent;
}

int32 URMSComponent::TryActivateTask(URMSTask_Base* Task)
{
	// if (URMSTask_MoveTo* MoveTo = Cast<URMSTask_MoveTo>(Task))
	// {
	// 	if (!MoveTo->RootMotionComponent.IsValid())
	// 	{
	// 		OnTaskEnd.Broadcast(MoveTo,false);
	// 		return -1;
	// 	}
	// 	if (bListenTaskEnd /*&& !MoveTo->Setting.bForce*/ && URMSLibrary::IsRootMotionSourceValid(MoveTo->RootMotionComponent->GetMovementComponent(), MoveTo->GetInstanceName()))
	// 	{
	// 		OnTaskEnd.Broadcast(MoveTo,false);
	// 		return -1;
	// 	}
	// 	int32 ID = URMSLibrary::ApplyRootMotionSource_MoveToForce(MoveTo->RootMotionComponent->GetMovementComponent(), MoveTo->GetInstanceName(), MoveTo->StartLocation,MoveTo->TargetLocation,MoveTo->Duration,
	// 		MoveTo->Priority,MoveTo->PathOffsetCurve,0,ERMSApplyMode::None,MoveTo->Setting);
	// 	MoveTo->Activate();
	// 	if (bListenTaskEnd)
	// 	{
	// 		CurrentTasks.Emplace(ID, MoveTo);
	// 	}
	// 	OnTaskBegin.Broadcast(MoveTo);
	// 	return ID;
	// }
	return -1;
}

void URMSComponent::SetRms_TargetByLocation(FName Instance, FVector Location)
{
	auto item = RMSMap.Find(Instance);
	if (item)
	{
		FTransform newTransform = *item;
		newTransform.SetLocation(Location);
		RMSMap.Add(Instance, newTransform);
	}
}

void URMSComponent::SetRms_TargetByRotation(FName Instance, FRotator Rotation)
{
	auto item = RMSMap.Find(Instance);
	if (item)
	{
		FTransform newTransform = *item;
		newTransform.SetRotation(FQuat::MakeFromRotator(Rotation));
		RMSMap.Add(Instance, newTransform);
	}
}

void URMSComponent::SetRms_Target(FName Instance, FVector Location, FRotator Rotation)
{
	RMSMap.Add(Instance,FTransform(FQuat::MakeFromRotator(Rotation),Location ));
}

