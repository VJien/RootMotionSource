// Fill out your copyright notice in the Description page of Project Settings.


#include "RootMotionSourceGroupEx.h"
#include "Curves/CurveVector.h"
#include "Curves/CurveFloat.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

PRAGMA_DISABLE_OPTIMIZATION

namespace RMS
{
extern TAutoConsoleVariable<int32> CVarRMS_Debug;
}

static float EvaluateFloatCurveAtFraction(const UCurveFloat& Curve, const float Fraction)
{
	float MinCurveTime(0.f);
	float MaxCurveTime(1.f);

	Curve.GetTimeRange(MinCurveTime, MaxCurveTime);
	return Curve.GetFloatValue(FMath::GetRangeValue(FVector2D(MinCurveTime, MaxCurveTime), Fraction));
}

static FVector EvaluateVectorCurveAtFraction(const UCurveVector& Curve, const float Fraction)
{
	float MinCurveTime(0.f);
	float MaxCurveTime(1.f);

	Curve.GetTimeRange(MinCurveTime, MaxCurveTime);
	return Curve.GetVectorValue(FMath::GetRangeValue(FVector2D(MinCurveTime, MaxCurveTime), Fraction));
}

FRootMotionSource_PathMoveToForce::FRootMotionSource_PathMoveToForce()
{
}

FVector FRootMotionSource_PathMoveToForce::GetPathOffsetInWorldSpace(const float MoveFraction, FRootMotionSourcePathMoveToData Data, FVector Start) const
{
	if (Data.PathOffsetCurve)
	{
		// Calculate path offset
		const FVector PathOffsetInFacingSpace = EvaluateVectorCurveAtFraction(*Data.PathOffsetCurve, MoveFraction);
		FRotator FacingRotation((Data.Target - Start).Rotation());
		FacingRotation.Pitch = 0.f;
		return FacingRotation.RotateVector(PathOffsetInFacingSpace);
	}
	return FVector::ZeroVector;
}

bool FRootMotionSource_PathMoveToForce::GetPathDataByTime(float InTime, FRootMotionSourcePathMoveToData& OutCurrData, FRootMotionSourcePathMoveToData& OutLastData) const
{
	float Time = 0;
	if (Duration > 0)
	{
		for (auto i : Path)
		{
			if (InTime >= Time && InTime <= i.Duration + Time)
			{
				OutCurrData = i;
				return true;
			}
			else
			{
				OutLastData = i;
				Time += i.Duration;
			}
		}
	}
	return false;
}

bool FRootMotionSource_PathMoveToForce::UpdateStateFrom(const FRootMotionSource* SourceToTakeStateFrom, bool bMarkForSimulatedCatchup)
{
	if (!FRootMotionSource::UpdateStateFrom(SourceToTakeStateFrom, bMarkForSimulatedCatchup))
	{
		return false;
	}

	return true;
}

void FRootMotionSource_PathMoveToForce::PrepareRootMotion(float SimulationTime, float MovementTickTime, const ACharacter& Character, const UCharacterMovementComponent& MoveComponent)
{
	RootMotionParams.Clear();
	if (Path.Num() <= 0)
	{
		return;
	}
	if (Duration > SMALL_NUMBER && MovementTickTime > SMALL_NUMBER)
	{
		const float NextFrame = (GetTime() + SimulationTime);
		if (Index == -1)
		{
			LastData.Target = StartLocation;
			CurrData = Path[0];
			Index = 0;
		}
		if (NextFrame > CurrData.Duration)
		{
			if (Index < Path.Num() - 1)
			{
				Index ++;
				LastData = CurrData;
                CurrData = Path[Index];
			}
		}
		float MoveFraction = (NextFrame - LastData.Duration) / CurrData.Duration;
		if (CurrData.TimeMappingCurve)
		{
			MoveFraction = EvaluateFloatCurveAtFraction(*CurrData.TimeMappingCurve, MoveFraction);
		}
		FVector CurrentTargetLocation = FMath::Lerp<FVector, float>(LastData.Target, CurrData.Target, MoveFraction);
		CurrentTargetLocation += GetPathOffsetInWorldSpace(MoveFraction, CurrData, LastData.Target);

		const FVector CurrentLocation = Character.GetActorLocation();

		FVector Force = (CurrentTargetLocation - CurrentLocation) / MovementTickTime;


		// Debug
#if ROOT_MOTION_DEBUG
		if (RootMotionSourceDebug::CVarDebugRootMotionSources.GetValueOnGameThread() != 0)
		{
			const FVector LocDiff = MoveComponent.UpdatedComponent->GetComponentLocation() - CurrentLocation;
			const float DebugLifetime = 5.0f;

			// Current
			DrawDebugCapsule(Character.GetWorld(), MoveComponent.UpdatedComponent->GetComponentLocation(), Character.GetSimpleCollisionHalfHeight(), Character.GetSimpleCollisionRadius(), FQuat::Identity, FColor::Red, false, DebugLifetime);

			// Current Target
			DrawDebugCapsule(Character.GetWorld(), CurrentTargetLocation + LocDiff, Character.GetSimpleCollisionHalfHeight(), Character.GetSimpleCollisionRadius(), FQuat::Identity, FColor::Green, false, DebugLifetime);

			// Target
			DrawDebugCapsule(Character.GetWorld(), CurrData.Target + LocDiff, Character.GetSimpleCollisionHalfHeight(), Character.GetSimpleCollisionRadius(), FQuat::Identity, FColor::Blue, false, DebugLifetime);

			// Force
			DrawDebugLine(Character.GetWorld(), CurrentLocation, CurrentLocation + Force, FColor::Blue, false, DebugLifetime);
		}
#endif

		FTransform NewTransform(Force);
		RootMotionParams.Set(NewTransform);
	}
	else
	{
		checkf(Duration > SMALL_NUMBER, TEXT("FRootMotionSource_MoveToForce prepared with invalid duration."));
	}

	SetTime(GetTime() + SimulationTime);
}

bool FRootMotionSource_PathMoveToForce::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	if (!FRootMotionSource::NetSerialize(Ar, Map, bOutSuccess))
	{
		return false;
	}

	Ar << StartLocation;
	Ar << Path;
	Ar << CurrData;
	Ar << LastData;
	Ar << Index;

	bOutSuccess = true;
	return true;
}

FRootMotionSource* FRootMotionSource_PathMoveToForce::Clone() const
{
	FRootMotionSource_PathMoveToForce* CopyPtr = new FRootMotionSource_PathMoveToForce(*this);
	return CopyPtr;
}

bool FRootMotionSource_PathMoveToForce::Matches(const FRootMotionSource* Other) const
{
	if (!FRootMotionSource::Matches(Other))
	{
		return false;
	}

	// We can cast safely here since in FRootMotionSource::Matches() we ensured ScriptStruct equality
	const FRootMotionSource_PathMoveToForce* OtherCast = static_cast<const FRootMotionSource_PathMoveToForce*>(Other);

	return StartLocation == OtherCast->StartLocation &&
		Path == OtherCast->Path;
}

bool FRootMotionSource_PathMoveToForce::MatchesAndHasSameState(const FRootMotionSource* Other) const
{
	if (!FRootMotionSource::MatchesAndHasSameState(Other))
	{
		return false;
	}

	return true;
}

UScriptStruct* FRootMotionSource_PathMoveToForce::GetScriptStruct() const
{
	return FRootMotionSource_PathMoveToForce::StaticStruct();
}

FString FRootMotionSource_PathMoveToForce::ToSimpleString() const
{
	return FString::Printf(TEXT("[ID:%u]FRootMotionSource_PathMoveToForce %s"), LocalID, *InstanceName.GetPlainNameString());
}

void FRootMotionSource_PathMoveToForce::AddReferencedObjects(FReferenceCollector& Collector)
{
	for (auto i : Path)
	{
		Collector.AddReferencedObject(i.PathOffsetCurve);
		Collector.AddReferencedObject(i.TimeMappingCurve);
	}
	FRootMotionSource::AddReferencedObjects(Collector);
}


PRAGMA_ENABLE_OPTIMIZATION