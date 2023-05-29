//  Copyright. VJ  All Rights Reserved.
//  https://supervj.top/2022/03/24/RootMotionSource/


#include "RMSGroupEx.h"
#include "Curves/CurveVector.h"
#include "Curves/CurveFloat.h"
#include "DrawDebugHelpers.h"
#include "RMSLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

UE_DISABLE_OPTIMIZATION


#pragma region FRootMotionSource_PathMoveToForce
FRootMotionSource_PathMoveToForce::FRootMotionSource_PathMoveToForce()
{
}

FVector FRootMotionSource_PathMoveToForce::GetPathOffsetInWorldSpace(const float MoveFraction, FRMSPathMoveToData Data,
                                                                     FVector Start) const
{
	if (Data.PathOffsetCurve)
	{
		// Calculate path offset
		const FVector PathOffsetInFacingSpace = URMSLibrary::EvaluateVectorCurveAtFraction(
			*Data.PathOffsetCurve, MoveFraction);
		FRotator FacingRotation((Data.Target - Start).Rotation());
		FacingRotation.Pitch = 0.f;
		return FacingRotation.RotateVector(PathOffsetInFacingSpace);
	}
	return FVector::ZeroVector;
}

bool FRootMotionSource_PathMoveToForce::GetPathDataByTime(float InTime, FRMSPathMoveToData& OutCurrData,
                                                          FRMSPathMoveToData& OutLastData) const
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

bool FRootMotionSource_PathMoveToForce::UpdateStateFrom(const FRootMotionSource* SourceToTakeStateFrom,
                                                        bool bMarkForSimulatedCatchup)
{
	if (!FRootMotionSource::UpdateStateFrom(SourceToTakeStateFrom, bMarkForSimulatedCatchup))
	{
		return false;
	}

	return true;
}

void FRootMotionSource_PathMoveToForce::PrepareRootMotion(float SimulationTime, float MovementTickTime,
                                                          const ACharacter& Character,
                                                          const UCharacterMovementComponent& MoveComponent)
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
			LastData.RotationSetting.TargetRotation = StartRotation;

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
				LastData.RotationSetting.TargetRotation = Character.GetActorRotation();
			}
		}
		float MoveFraction = (NextFrame - LastData.Duration) / CurrData.Duration;
		if (CurrData.TimeMappingCurve)
		{
			MoveFraction = URMSLibrary::EvaluateFloatCurveAtFraction(*CurrData.TimeMappingCurve, MoveFraction);
		}
		FVector CurrentTargetLocation = FMath::Lerp<FVector, float>(LastData.Target, CurrData.Target, MoveFraction);
		CurrentTargetLocation += GetPathOffsetInWorldSpace(MoveFraction, CurrData, LastData.Target);
		const FVector CurrentLocation = Character.GetActorLocation();
		FVector Force = (CurrentTargetLocation - CurrentLocation) / MovementTickTime;

		FRotator RotationDt = FRotator::ZeroRotator;
		if (CurrData.RotationSetting.IsWarpRotation())
		{
			FRotator TargetRotation;
			if (CurrData.RotationSetting.Mode == ERMSRotationMode::FaceToTarget)
			{
				TargetRotation = (CurrData.Target - LastData.Target).Rotation();
			}
			else
			{
				TargetRotation = CurrData.RotationSetting.TargetRotation;
			}
			const float RotationFraction = FMath::Clamp(MoveFraction * CurrData.RotationSetting.WarpMultiplier,0,1);
			
			URMSLibrary::ExtractRotation(RotationDt, Character, LastData.RotationSetting.TargetRotation, TargetRotation, RotationFraction,
			                             CurrData.RotationSetting.Curve);
		}


		// Debug
#if ROOT_MOTION_DEBUG
		if (RMS::CVarRMS_Debug.GetValueOnGameThread() > 0)
		{
			const FVector LocDiff = MoveComponent.UpdatedComponent->GetComponentLocation() - CurrentLocation;
			const float DebugLifetime = 5.0f;

			// Current
			DrawDebugCapsule(Character.GetWorld(), MoveComponent.UpdatedComponent->GetComponentLocation(),
			                 Character.GetSimpleCollisionHalfHeight(), Character.GetSimpleCollisionRadius(),
			                 FQuat::Identity, FColor::Red, false, DebugLifetime);

			// Current Target
			DrawDebugCapsule(Character.GetWorld(), CurrentTargetLocation + LocDiff,
			                 Character.GetSimpleCollisionHalfHeight(), Character.GetSimpleCollisionRadius(),
			                 FQuat::Identity, FColor::Green, false, DebugLifetime);

			// Target
			DrawDebugCapsule(Character.GetWorld(), CurrData.Target + LocDiff, Character.GetSimpleCollisionHalfHeight(),
			                 Character.GetSimpleCollisionRadius(), FQuat::Identity, FColor::Blue, false, DebugLifetime);

			// Force
			DrawDebugLine(Character.GetWorld(), CurrentLocation, CurrentLocation + Force, FColor::Blue, false,
			              DebugLifetime);
		}
#endif

		FTransform NewTransform(RotationDt, Force);
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
		Path == OtherCast->Path && StartRotation == OtherCast->StartRotation;
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
	return FString::Printf(
		TEXT("[ID:%u]FRootMotionSource_PathMoveToForce %s"), LocalID, *InstanceName.GetPlainNameString());
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
//*******************************************************
#pragma region Jump
FRootMotionSource_JumpForce_WithPoints::FRootMotionSource_JumpForce_WithPoints()
{
	
}

bool FRootMotionSource_JumpForce_WithPoints::UpdateStateFrom(const FRootMotionSource* SourceToTakeStateFrom, bool bMarkForSimulatedCatchup)
{
	if (!FRootMotionSource::UpdateStateFrom(SourceToTakeStateFrom, bMarkForSimulatedCatchup))
	{
		return false;
	}

	return true;
}

void FRootMotionSource_JumpForce_WithPoints::PrepareRootMotion(float SimulationTime, float MovementTickTime, const ACharacter& Character, const UCharacterMovementComponent& MoveComponent)
{
	if (!bIsInit)
	{
		bIsInit = true;
		InitPath(Character);
	}
	RootMotionParams.Clear();
	
	if (Duration > SMALL_NUMBER && MovementTickTime > SMALL_NUMBER && SimulationTime > SMALL_NUMBER)
	{
		float CurrentTimeFraction = GetTime() / Duration;
		float TargetTimeFraction = (GetTime() + SimulationTime) / Duration;

		// If we're beyond specified duration, we need to re-map times so that
		// we continue our desired ending velocity
		if (TargetTimeFraction > 1.f)
		{
			float TimeFractionPastAllowable = TargetTimeFraction - 1.0f;
			TargetTimeFraction -= TimeFractionPastAllowable;
			CurrentTimeFraction -= TimeFractionPastAllowable;
		}

		float CurrentMoveFraction = CurrentTimeFraction;
		float TargetMoveFraction = TargetTimeFraction;

		if (TimeMappingCurve)
		{
			CurrentMoveFraction = URMSLibrary::EvaluateFloatCurveAtFraction(*TimeMappingCurve, CurrentMoveFraction);
			TargetMoveFraction  = URMSLibrary::EvaluateFloatCurveAtFraction(*TimeMappingCurve, TargetMoveFraction);
		}
		if (RotationSetting.Mode == ERMSRotationMode::FaceToTarget)
		{
			float RotFraction = CurrentMoveFraction;
			if (RotationSetting.Curve)
			{
				RotFraction = FMath::Clamp(RotationSetting.WarpMultiplier *  URMSLibrary::EvaluateFloatCurveAtFraction(*RotationSetting.Curve, RotFraction),0,1);
			}
			const FRotator TargetRotation = (TargetLocation - StartLocation).Rotation();
			SavedRotation = UKismetMathLibrary::RLerp(StartRotation, TargetRotation, RotFraction, true);
		}
		else if(RotationSetting.Mode == ERMSRotationMode::Custom)
		{
			float RotFraction = CurrentMoveFraction;
			if (RotationSetting.Curve)
			{
				RotFraction = FMath::Clamp(RotationSetting.WarpMultiplier *  URMSLibrary::EvaluateFloatCurveAtFraction(*RotationSetting.Curve, RotFraction),0,1);
			}
			const FRotator TargetRotation = RotationSetting.TargetRotation;
			SavedRotation = UKismetMathLibrary::RLerp(StartRotation, TargetRotation, RotFraction, true);
		}
		const FVector CurrentRelativeLocation = GetRelativeLocation(CurrentMoveFraction);
		const FVector TargetRelativeLocation = GetRelativeLocation(TargetMoveFraction);

		const FVector Force = (TargetRelativeLocation - CurrentRelativeLocation) / MovementTickTime;

		// Debug
#if ROOT_MOTION_DEBUG
		if (RMS::CVarRMS_Debug.GetValueOnGameThread() > 0)
		{
			const FVector CurrentLocation = Character.GetActorLocation();
			const FVector CurrentTargetLocation = CurrentLocation + (TargetRelativeLocation - CurrentRelativeLocation);
			const FVector LocDiff = MoveComponent.UpdatedComponent->GetComponentLocation() - CurrentLocation;
			const float DebugLifetime = 5.0f;

			// Current
			DrawDebugCapsule(Character.GetWorld(), MoveComponent.UpdatedComponent->GetComponentLocation(), Character.GetSimpleCollisionHalfHeight(), Character.GetSimpleCollisionRadius(), FQuat::Identity, FColor::Red, false, DebugLifetime);

			// Current Target
			DrawDebugCapsule(Character.GetWorld(), CurrentTargetLocation + LocDiff, Character.GetSimpleCollisionHalfHeight(), Character.GetSimpleCollisionRadius(), FQuat::Identity, FColor::Green, false, DebugLifetime);

			// Target
			DrawDebugCapsule(Character.GetWorld(), CurrentTargetLocation + LocDiff, Character.GetSimpleCollisionHalfHeight(), Character.GetSimpleCollisionRadius(), FQuat::Identity, FColor::Blue, false, DebugLifetime);

			// Force
			DrawDebugLine(Character.GetWorld(), CurrentLocation, CurrentLocation+Force, FColor::Blue, false, DebugLifetime);

			// Halfway point
			const FVector HalfwayLocation = CurrentLocation + (GetRelativeLocation(0.5f) - CurrentRelativeLocation);
			if (SavedHalfwayLocation.IsNearlyZero())
			{
				SavedHalfwayLocation = HalfwayLocation;
			}
			if (FVector::DistSquared(SavedHalfwayLocation, HalfwayLocation) > 50.f*50.f)
			{
				UE_LOG(LogRootMotion, Verbose, TEXT("RootMotion JumpForce drifted from saved halfway calculation!"));
				SavedHalfwayLocation = HalfwayLocation;
			}
			DrawDebugCapsule(Character.GetWorld(), HalfwayLocation + LocDiff, Character.GetSimpleCollisionHalfHeight(), Character.GetSimpleCollisionRadius(), FQuat::Identity, FColor::White, true, DebugLifetime);

			// Destination point
			const FVector DestinationLocation = CurrentLocation + (GetRelativeLocation(1.0f) - CurrentRelativeLocation);
			DrawDebugCapsule(Character.GetWorld(), DestinationLocation + LocDiff, Character.GetSimpleCollisionHalfHeight(), Character.GetSimpleCollisionRadius(), FQuat::Identity, FColor::White, true, DebugLifetime);

			UE_LOG(LogRootMotion, VeryVerbose, TEXT("RootMotionJumpForce %s %s preparing from %f to %f from (%s) to (%s) resulting force %s"), 
				Character.GetLocalRole() == ROLE_AutonomousProxy ? TEXT("AUTONOMOUS") : TEXT("AUTHORITY"),
				Character.bClientUpdating ? TEXT("UPD") : TEXT("NOR"),
				GetTime(), GetTime() + SimulationTime, 
				*CurrentLocation.ToString(), *CurrentTargetLocation.ToString(), 
				*Force.ToString());

			{
				FString AdjustedDebugString = FString::Printf(TEXT("    FRootMotionSource_JumpForce::Prep Force(%s) SimTime(%.3f) MoveTime(%.3f) StartP(%.3f) EndP(%.3f)"),
					*Force.ToCompactString(), SimulationTime, MovementTickTime, CurrentMoveFraction, TargetMoveFraction);
				RootMotionSourceDebug::PrintOnScreen(Character, AdjustedDebugString);
			}
		}
#endif

		const FTransform NewTransform(Force);
		RootMotionParams.Set(NewTransform);
	}
	else
	{
		checkf(Duration > SMALL_NUMBER, TEXT("FRootMotionSource_JumpForce prepared with invalid duration."));
	}

	SetTime(GetTime() + SimulationTime);
}

bool FRootMotionSource_JumpForce_WithPoints::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	if (!FRootMotionSource::NetSerialize(Ar, Map, bOutSuccess))
	{
		return false;
	}

	
	Ar << bDisableTimeout;
	Ar << TimeMappingCurve;

	bOutSuccess = true;
	return true;

}

FVector FRootMotionSource_JumpForce_WithPoints::GetPathOffset(float MoveFraction) const
{
	FVector PathOffset(FVector::ZeroVector);
	if (PathOffsetCurve)
	{
		// Calculate path offset
		PathOffset = URMSLibrary::EvaluateVectorCurveAtFraction(*PathOffsetCurve, MoveFraction);
	}
	else
	{
		// Default to "jump parabola", a simple x^2 shifted to be upside-down and shifted
		// to get [0,1] X (MoveFraction/Distance) mapping to [0,1] Y (height)
		// Height = -(2x-1)^2 + 1
		const float Phi = 2.f*MoveFraction - 1;
		const float Z = -(Phi*Phi) + 1;
		PathOffset.Z = Z;
	}

	// Scale Z offset to height. If Height < 0, we use direct path offset values
	if (SavedHeight >= 0.f)
	{
		PathOffset.Z *= SavedHeight;
	}

	return PathOffset;
}

FVector FRootMotionSource_JumpForce_WithPoints::GetRelativeLocation(float MoveFraction) const
{
	// Given MoveFraction, what relative location should a character be at?
	FRotator FacingRotation(SavedRotation);
	FacingRotation.Pitch = 0.f; // By default we don't include pitch, but an option could be added if necessary

	FVector RelativeLocationFacingSpace = FVector(MoveFraction * SavedDistance, 0.f, 0.f) + GetPathOffset(MoveFraction);

	return FacingRotation.RotateVector(RelativeLocationFacingSpace);
}

bool FRootMotionSource_JumpForce_WithPoints::IsTimeOutEnabled() const
{
	if (bDisableTimeout)
	{
		return false;
	}
	return FRootMotionSource::IsTimeOutEnabled();
}

FRootMotionSource* FRootMotionSource_JumpForce_WithPoints::Clone() const
{
	FRootMotionSource_JumpForce_WithPoints* CopyPtr = new FRootMotionSource_JumpForce_WithPoints(*this);
	return CopyPtr;
}

bool FRootMotionSource_JumpForce_WithPoints::Matches(const FRootMotionSource* Other) const
{
	return FRootMotionSource::Matches(Other);
}

bool FRootMotionSource_JumpForce_WithPoints::MatchesAndHasSameState(const FRootMotionSource* Other) const
{
	if (!FRootMotionSource::MatchesAndHasSameState(Other))
	{
		return false;
	}

	return true;
}

UScriptStruct* FRootMotionSource_JumpForce_WithPoints::GetScriptStruct() const
{
	return FRootMotionSource_JumpForce_WithPoints::StaticStruct();
}

FString FRootMotionSource_JumpForce_WithPoints::ToSimpleString() const
{
	return FString::Printf(
		TEXT("[ID:%u]FRootMotionSource_JumpForce_WithPoints %s"), LocalID, *InstanceName.GetPlainNameString());;
}

void FRootMotionSource_JumpForce_WithPoints::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(RotationSetting.Curve);
	Collector.AddReferencedObject(TimeMappingCurve);
	Collector.AddReferencedObject(PathOffsetCurve);
	FRootMotionSource::AddReferencedObjects(Collector);
}
void FRootMotionSource_JumpForce_WithPoints::InitPath(const ACharacter& Character)
{
	if (RotationSetting.Mode == ERMSRotationMode::None)
	{
		SavedRotation = StartRotation;
			//(TargetLocation - StartLocation).Rotation();
	}
	if (!PathOffsetCurve)
	{
		PathOffsetCurve = NewObject<UCurveVector>();
	}
	const bool bDebug = RMS::CVarRMS_Debug->GetInt()>0;

	const float Dist1toMid = (HalfWayLocation - StartLocation).Size();
	const float DistMidto2 = (HalfWayLocation - TargetLocation).Size();
	const float Dist1to2 = (StartLocation - TargetLocation).Size();
	FVector AvgMidPoint = (StartLocation + TargetLocation) * 0.5;
	AvgMidPoint.Z = HalfWayLocation.Z;
	const float Dist1toAvgMid = (AvgMidPoint - StartLocation).Size();
	const float DistAvgMidto2 = (AvgMidPoint - TargetLocation).Size();
	SavedDistance = Dist1to2;
	SavedHeight = FMath::Abs(HalfWayLocation.Z - StartLocation.Z);
	float Duration1toMid = Duration * (Dist1toMid / (Dist1toMid + DistMidto2));
	float DurationMidto2 = Duration * (DistMidto2 / (Dist1toMid + DistMidto2));
	if (bDebug)
	{
		DrawDebugSphere(Character.GetWorld(), HalfWayLocation,15.0,8,FColor::Green,false,5.0,0,0.1);
		DrawDebugSphere(Character.GetWorld(), AvgMidPoint,15.0,8,FColor::Green,false,5.0,0,0.1);
	}
	FRichCurve XCurve;
	FRichCurve ZCurve;
	//根据这个持续时间遍历
	for (float i = 0; i <= Duration; i += Duration / 64.0)
	{
		const float MoveFraction = i / Duration;
		const float Phi = 2.f * MoveFraction - 1;
		const float Z = -(Phi * Phi) + 1;

		ZCurve.AddKey(i / Duration, Z);
		//处理X
		const float MidFrac = Dist1toMid / (Dist1toMid + DistMidto2);
		const float CurrDist = MoveFraction * (Dist1toMid + DistMidto2);
		const float CurrDistAvg = MoveFraction * (Dist1toAvgMid + DistAvgMidto2);
		//在前半段
		float X, AvgX;
		if (CurrDist <= Dist1toMid)
		{
			FVector CurrPoint = StartLocation + (CurrDist / Dist1toMid) * (HalfWayLocation - StartLocation);
			X = (CurrPoint - StartLocation).Size2D();
			if (bDebug)
			{
				DrawDebugSphere(Character.GetWorld(), CurrPoint, 5.0,4,FColor::Green,false,5.0,0,0.5);
			}
		}
		else
		{
			FVector CurrPoint = HalfWayLocation + ((CurrDist - Dist1toMid) / DistMidto2) * (TargetLocation - HalfWayLocation);
			X = (CurrPoint - StartLocation).Size2D();
			if (bDebug)
			{
				DrawDebugSphere(Character.GetWorld(), CurrPoint, 5.0,4,FColor::Green,false,5.0,0,0.5);
			}
		}
		if (CurrDistAvg <= Dist1toAvgMid)
		{
			FVector CurrPoint = StartLocation + (CurrDistAvg / Dist1toAvgMid) * (AvgMidPoint - StartLocation);
			AvgX = (CurrPoint - StartLocation).Size2D();
			if (bDebug)
			{
				DrawDebugSphere(Character.GetWorld(), CurrPoint, 5.0,4,FColor::Red,false,5.0,0,0.5);
			}
		}
		else
		{
			FVector CurrPoint = AvgMidPoint + ((CurrDistAvg - Dist1toAvgMid) / DistAvgMidto2) * (TargetLocation - AvgMidPoint);
			AvgX = (CurrPoint - StartLocation).Size2D();
			if (bDebug)
			{
				DrawDebugSphere(Character.GetWorld(), CurrPoint, 5.0,4,FColor::Red,false,5.0,0,0.5);
			}
		}
		XCurve.AddKey(i / Duration, X - AvgX);
	}
	XCurve.AddKey(1.0, 0.0);
	ZCurve.AddKey(1.0, 0.0);
	PathOffsetCurve->FloatCurves[0] = XCurve;
	PathOffsetCurve->FloatCurves[2] = ZCurve;
}


#pragma endregion Jump


//*******************************************************************
FRootMotionSource_MoveToForce_WithRotation::FRootMotionSource_MoveToForce_WithRotation()
{
}

void FRootMotionSource_MoveToForce_WithRotation::PrepareRootMotion(float SimulationTime, float MovementTickTime,
                                                                   const ACharacter& Character,
                                                                   const UCharacterMovementComponent& MoveComponent)
{
	if (RotationSetting.IsWarpRotation())
	{
		RootMotionParams.Clear();

		if (Duration > SMALL_NUMBER && MovementTickTime > SMALL_NUMBER)
		{
			const float MoveFraction = (GetTime() + SimulationTime) / Duration;

			FVector CurrentTargetLocation = FMath::Lerp<FVector, float>(StartLocation, TargetLocation, MoveFraction);
			CurrentTargetLocation += GetPathOffsetInWorldSpace(MoveFraction);
			const FVector CurrentLocation = Character.GetActorLocation();
			FVector Force = (CurrentTargetLocation - CurrentLocation) / MovementTickTime;
			FRotator RotationDt;
			const FRotator TargetRotation = RotationSetting.Mode == ERMSRotationMode::Custom
				                 ? RotationSetting.TargetRotation
				                 : (TargetLocation - StartLocation).Rotation();
			const float RotationFraction = FMath::Clamp(MoveFraction * RotationSetting.WarpMultiplier,0,1);
			URMSLibrary::ExtractRotation(RotationDt, Character, StartRotation, TargetRotation, RotationFraction, RotationSetting.Curve);

			if (bRestrictSpeedToExpected && !Force.IsNearlyZero(KINDA_SMALL_NUMBER))
			{
				// Calculate expected current location (if we didn't have collision and moved exactly where our velocity should have taken us)
				const float PreviousMoveFraction = GetTime() / Duration;
				FVector CurrentExpectedLocation = FMath::Lerp<FVector, float>(
					StartLocation, TargetLocation, PreviousMoveFraction);
				CurrentExpectedLocation += GetPathOffsetInWorldSpace(PreviousMoveFraction);

				// Restrict speed to the expected speed, allowing some small amount of error
				const FVector ExpectedForce = (CurrentTargetLocation - CurrentExpectedLocation) / MovementTickTime;
				const float ExpectedSpeed = ExpectedForce.Size();
				const float CurrentSpeedSqr = Force.SizeSquared();

				const float ErrorAllowance = 0.5f; // in cm/s
				if (CurrentSpeedSqr > FMath::Square(ExpectedSpeed + ErrorAllowance))
				{
					Force.Normalize();
					Force *= ExpectedSpeed;
				}
			}

			// Debug
#if ROOT_MOTION_DEBUG
			if (RMS::CVarRMS_Debug.GetValueOnGameThread() > 0)
			{
				const FVector LocDiff = MoveComponent.UpdatedComponent->GetComponentLocation() - CurrentLocation;
				const float DebugLifetime = 2.0f;

				// Current
				DrawDebugCapsule(Character.GetWorld(), MoveComponent.UpdatedComponent->GetComponentLocation(),
				                 Character.GetSimpleCollisionHalfHeight(), Character.GetSimpleCollisionRadius(),
				                 FQuat::Identity, FColor::Red, false, DebugLifetime);

				// Current Target
				DrawDebugCapsule(Character.GetWorld(), CurrentTargetLocation + LocDiff,
				                 Character.GetSimpleCollisionHalfHeight(), Character.GetSimpleCollisionRadius(),
				                 FQuat::Identity, FColor::Green, false, DebugLifetime);

				// Target
				DrawDebugCapsule(Character.GetWorld(), TargetLocation + LocDiff,
				                 Character.GetSimpleCollisionHalfHeight(), Character.GetSimpleCollisionRadius(),
				                 FQuat::Identity, FColor::Blue, false, DebugLifetime);

				// Force
				DrawDebugLine(Character.GetWorld(), CurrentLocation, CurrentLocation + Force, FColor::Blue, false,
				              DebugLifetime);
			}
#endif

			FTransform NewTransform(RotationDt, Force);
			RootMotionParams.Set(NewTransform);
		}
		SetTime(GetTime() + SimulationTime);
	}
	else
	{
		FRootMotionSource_MoveToForce::PrepareRootMotion(SimulationTime, MovementTickTime, Character,
		                                                 MoveComponent);
	}
}

UScriptStruct* FRootMotionSource_MoveToForce_WithRotation::GetScriptStruct() const
{
	return FRootMotionSource_MoveToForce_WithRotation::StaticStruct();
}

FString FRootMotionSource_MoveToForce_WithRotation::ToSimpleString() const
{
	return FString::Printf(
		TEXT("[ID:%u]FRootMotionSource_MoveToForce_WithRotation %s"), LocalID, *InstanceName.GetPlainNameString());
}

void FRootMotionSource_MoveToForce_WithRotation::AddReferencedObjects(FReferenceCollector& Collector)
{
	
	Collector.AddReferencedObject(RotationSetting.Curve);
	
	
	FRootMotionSource::AddReferencedObjects(Collector);
}

bool FRootMotionSource_MoveToForce_WithRotation::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	if (!FRootMotionSource::NetSerialize(Ar, Map, bOutSuccess))
	{
		return false;
	}

	Ar << StartLocation;
	Ar << RotationSetting;
	Ar << StartLocation; // TODO-RootMotionSource: Quantization
	Ar << TargetLocation; // TODO-RootMotionSource: Quantization
	Ar << bRestrictSpeedToExpected;
	//Ar << PathOffsetCurve;

	bOutSuccess = true;
	return true;
}

FRootMotionSource* FRootMotionSource_MoveToForce_WithRotation::Clone() const
{
	FRootMotionSource_MoveToForce_WithRotation* CopyPtr = new FRootMotionSource_MoveToForce_WithRotation(*this);
	return CopyPtr;
}

bool FRootMotionSource_MoveToForce_WithRotation::Matches(const FRootMotionSource* Other) const
{
	if (!FRootMotionSource::Matches(Other))
	{
		return false;
	}
	const FRootMotionSource_MoveToForce_WithRotation* OtherCast = static_cast<const FRootMotionSource_MoveToForce_WithRotation*>(Other);

	return RotationSetting == OtherCast->RotationSetting && StartRotation == OtherCast->StartRotation;
}

bool FRootMotionSource_MoveToForce_WithRotation::MatchesAndHasSameState(const FRootMotionSource* Other) const
{
	if (!FRootMotionSource::MatchesAndHasSameState(Other))
	{
		return false;
	}

	return true;
}

bool FRootMotionSource_MoveToForce_WithRotation::UpdateStateFrom(const FRootMotionSource* SourceToTakeStateFrom, bool bMarkForSimulatedCatchup)
{
	if (!FRootMotionSource::UpdateStateFrom(SourceToTakeStateFrom, bMarkForSimulatedCatchup))
	{
		return false;
	}

	return true;
}

void FRootMotionSource_MoveToDynamicForce_WithRotation::PrepareRootMotion(float SimulationTime, float MovementTickTime,
                                                                          const ACharacter& Character,
                                                                          const UCharacterMovementComponent&
                                                                          MoveComponent)
{
	if (RotationSetting.IsWarpRotation())
	{
		RootMotionParams.Clear();

		if (Duration > SMALL_NUMBER && MovementTickTime > SMALL_NUMBER)
		{
			float MoveFraction = (GetTime() + SimulationTime) / Duration;

			if (TimeMappingCurve)
			{
				MoveFraction = URMSLibrary::EvaluateFloatCurveAtFraction(*TimeMappingCurve, MoveFraction);
			}

			FVector CurrentTargetLocation = FMath::Lerp<FVector, float>(StartLocation, TargetLocation, MoveFraction);
			CurrentTargetLocation += GetPathOffsetInWorldSpace(MoveFraction);

			const FVector CurrentLocation = Character.GetActorLocation();

			FVector Force = (CurrentTargetLocation - CurrentLocation) / MovementTickTime;

			FRotator RotationDt;
			FRotator TargetRotation = RotationSetting.Mode == ERMSRotationMode::Custom
				                          ? RotationSetting.TargetRotation
				                          : (TargetLocation - StartLocation).Rotation();
			const float RotationFraction = FMath::Clamp(MoveFraction * RotationSetting.WarpMultiplier,0,1);
			URMSLibrary::ExtractRotation(RotationDt, Character, StartRotation, TargetRotation, RotationFraction,
			                             RotationSetting.Curve);


			if (bRestrictSpeedToExpected && !Force.IsNearlyZero(KINDA_SMALL_NUMBER))
			{
				// Calculate expected current location (if we didn't have collision and moved exactly where our velocity should have taken us)
				float PreviousMoveFraction = GetTime() / Duration;
				if (TimeMappingCurve)
				{
					PreviousMoveFraction = URMSLibrary::EvaluateFloatCurveAtFraction(
						*TimeMappingCurve, PreviousMoveFraction);
				}

				FVector CurrentExpectedLocation = FMath::Lerp<FVector, float>(
					StartLocation, TargetLocation, PreviousMoveFraction);
				CurrentExpectedLocation += GetPathOffsetInWorldSpace(PreviousMoveFraction);

				// Restrict speed to the expected speed, allowing some small amount of error
				const FVector ExpectedForce = (CurrentTargetLocation - CurrentExpectedLocation) / MovementTickTime;
				const float ExpectedSpeed = ExpectedForce.Size();
				const float CurrentSpeedSqr = Force.SizeSquared();

				const float ErrorAllowance = 0.5f; // in cm/s
				if (CurrentSpeedSqr > FMath::Square(ExpectedSpeed + ErrorAllowance))
				{
					Force.Normalize();
					Force *= ExpectedSpeed;
				}
			}

			// Debug
#if ROOT_MOTION_DEBUG
			if (RMS::CVarRMS_Debug.GetValueOnGameThread() > 0)
			{
				const FVector LocDiff = MoveComponent.UpdatedComponent->GetComponentLocation() - CurrentLocation;
				const float DebugLifetime = 2.0f;

				// Current
				DrawDebugCapsule(Character.GetWorld(), MoveComponent.UpdatedComponent->GetComponentLocation(),
				                 Character.GetSimpleCollisionHalfHeight(), Character.GetSimpleCollisionRadius(),
				                 FQuat::Identity, FColor::Red, false, DebugLifetime);

				// Current Target
				DrawDebugCapsule(Character.GetWorld(), CurrentTargetLocation + LocDiff,
				                 Character.GetSimpleCollisionHalfHeight(), Character.GetSimpleCollisionRadius(),
				                 FQuat::Identity, FColor::Green, false, DebugLifetime);

				// Target
				DrawDebugCapsule(Character.GetWorld(), TargetLocation + LocDiff,
				                 Character.GetSimpleCollisionHalfHeight(), Character.GetSimpleCollisionRadius(),
				                 FQuat::Identity, FColor::Blue, false, DebugLifetime);

				// Force
				DrawDebugLine(Character.GetWorld(), CurrentLocation, CurrentLocation + Force, FColor::Blue, false,
				              DebugLifetime);
			}
#endif

			FTransform NewTransform(RotationDt, Force);
			RootMotionParams.Set(NewTransform);
		}
		SetTime(GetTime() + SimulationTime);
	}
	else
	{
		FRootMotionSource_MoveToDynamicForce::PrepareRootMotion(SimulationTime, MovementTickTime, Character,
		                                                        MoveComponent);
	}
}

UScriptStruct* FRootMotionSource_MoveToDynamicForce_WithRotation::GetScriptStruct() const
{
	return FRootMotionSource_MoveToDynamicForce_WithRotation::StaticStruct();
}

bool FRootMotionSource_MoveToDynamicForce_WithRotation::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{

	if (!FRootMotionSource::NetSerialize(Ar, Map, bOutSuccess))
	{
		return false;
	}
	Ar << StartRotation;
	Ar << StartLocation; // TODO-RootMotionSource: Quantization
	Ar << InitialTargetLocation; // TODO-RootMotionSource: Quantization
	Ar << TargetLocation; // TODO-RootMotionSource: Quantization
	Ar << bRestrictSpeedToExpected;
	Ar << RotationSetting;
	//Ar << PathOffsetCurve;
	//Ar << TimeMappingCurve;

	bOutSuccess = true;
	return true;
	
	//return FRootMotionSource_MoveToDynamicForce::NetSerialize(Ar, Map, bOutSuccess);
}

FRootMotionSource* FRootMotionSource_MoveToDynamicForce_WithRotation::Clone() const
{
	FRootMotionSource_MoveToDynamicForce_WithRotation* CopyPtr = new FRootMotionSource_MoveToDynamicForce_WithRotation(*this);
	return CopyPtr;
}

bool FRootMotionSource_MoveToDynamicForce_WithRotation::Matches(const FRootMotionSource* Other) const
{
	if (!FRootMotionSource::Matches(Other))
	{
		return false;
	}
	const FRootMotionSource_MoveToDynamicForce_WithRotation* OtherCast = static_cast<const FRootMotionSource_MoveToDynamicForce_WithRotation*>(Other);

	return RotationSetting == OtherCast->RotationSetting && StartRotation == OtherCast->StartRotation;
}

bool FRootMotionSource_MoveToDynamicForce_WithRotation::MatchesAndHasSameState(const FRootMotionSource* Other) const
{
	if (!FRootMotionSource::MatchesAndHasSameState(Other))
	{
		return false;
	}

	return true;
}
#pragma endregion FRootMotionSource_PathMoveToForce


//********************FRootMotionSource_AnimWarping***********************
#pragma region FRootMotionSource_AnimWarping


FTransform FRootMotionSource_AnimWarping::ProcessRootMotion(const ACharacter& Character,
                                                            const FTransform& InRootMotion, float InPreviousTime,
                                                            float InCurrentTime, float DeltaSeconds)
{
	FTransform FinalRootMotion = InRootMotion;
	if (!Animation || !IsValid(&Character))
	{
		return InRootMotion;
	}
	float EndTime = GetCurrentAnimEndTime();
	if (EndTime < 0 || EndTime > Animation->GetPlayLength())
	{
		EndTime = Animation->GetPlayLength();
	}
	
	const FTransform RootMotionTotal = ExtractRootMotion(InPreviousTime, EndTime);
	const FTransform RootMotionDelta = ExtractRootMotion(InPreviousTime, FMath::Min(InCurrentTime, EndTime));

	if (!RootMotionDelta.GetTranslation().IsNearlyZero())
	{
		const FTransform CurrentTransform = FTransform(
			Character.GetActorQuat(),
			Character.GetActorLocation() - FVector(
				0.f, 0.f, Character.GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));
		//剩下的总共的RootMotion
		const FTransform RootMotionTotalWorldSpace = CurrentTransform * Character.GetMesh()->
			ConvertLocalRootMotionToWorld(RootMotionTotal);
		//这一帧的RootMotion
		const FTransform RootMotionDeltaWorldSpace = Character.GetMesh()->ConvertLocalRootMotionToWorld(
			RootMotionDelta);
		const FVector CurrentLocation = CurrentTransform.GetLocation();
		const FQuat CurrentRotation = CurrentTransform.GetRotation();

		FVector TargetLocation = GetTargetLocation();
		if (bIgnoreZAxis)
		{
			TargetLocation.Z = CurrentLocation.Z;
		}
		//这一帧的偏移
		const FVector Translation = RootMotionDeltaWorldSpace.GetTranslation();
		//总共剩下的动画RootMotion的偏移
		const FVector FutureLocation = RootMotionTotalWorldSpace.GetLocation();
		//当前位置到目标位置的偏移
		const FVector CurrentToWorldOffset = TargetLocation - CurrentLocation;
		//当前位置到动画RootMotion的偏移
		const FVector CurrentToRootOffset = FutureLocation - CurrentLocation;


		// 创建一个矩阵，我们可以用它把所有的东西放在一个空间中，直视RootMotionSyncPosition。“向前”应该是我们想要缩放的轴。
		//todo 实际上就是找到当前运动做接近的一个轴向作为向前的轴
		FVector ToRootNormalized = CurrentToRootOffset.GetSafeNormal();
		float BestMatchDot = FMath::Abs(FVector::DotProduct(ToRootNormalized, CurrentRotation.GetAxisX()));
		FMatrix ToRootSyncSpace = FRotationMatrix::MakeFromXZ(ToRootNormalized, CurrentRotation.GetAxisZ());

		float ZDot = FMath::Abs(FVector::DotProduct(ToRootNormalized, CurrentRotation.GetAxisZ()));
		if (ZDot > BestMatchDot)
		{
			ToRootSyncSpace = FRotationMatrix::MakeFromXZ(ToRootNormalized, CurrentRotation.GetAxisX());
			BestMatchDot = ZDot;
		}

		float YDot = FMath::Abs(FVector::DotProduct(ToRootNormalized, CurrentRotation.GetAxisY()));
		if (YDot > BestMatchDot)
		{
			ToRootSyncSpace = FRotationMatrix::MakeFromXZ(ToRootNormalized, CurrentRotation.GetAxisZ());
		}

		// 把所有偏移信息都放入这个空间中
		const FVector RootMotionInSyncSpace = ToRootSyncSpace.InverseTransformVector(Translation);
		const FVector CurrentToWorldSync = ToRootSyncSpace.InverseTransformVector(CurrentToWorldOffset);
		const FVector CurrentToRootMotionSync = ToRootSyncSpace.InverseTransformVector(CurrentToRootOffset);

		FVector CurrentToWorldSyncNorm = CurrentToWorldSync;
		CurrentToWorldSyncNorm.Normalize();

		FVector CurrentToRootMotionSyncNorm = CurrentToRootMotionSync;
		CurrentToRootMotionSyncNorm.Normalize();

		// 计算偏斜的角度Yaw
		FVector FlatToWorld = FVector(CurrentToWorldSyncNorm.X, CurrentToWorldSyncNorm.Y, 0.0f);
		FlatToWorld.Normalize();
		FVector FlatToRoot = FVector(CurrentToRootMotionSyncNorm.X, CurrentToRootMotionSyncNorm.Y, 0.0f);
		FlatToRoot.Normalize();
		float AngleAboutZ = FMath::Acos(FVector::DotProduct(FlatToWorld, FlatToRoot));
		float AngleAboutZNorm = FMath::DegreesToRadians(
			FRotator::NormalizeAxis(FMath::RadiansToDegrees(AngleAboutZ)));
		if (FlatToWorld.Y < 0.0f)
		{
			AngleAboutZNorm *= -1.0f;
		}

		// 计算偏斜的角度Pitch
		FVector ToWorldNoY = FVector(CurrentToWorldSyncNorm.X, 0.0f, CurrentToWorldSyncNorm.Z);
		ToWorldNoY.Normalize();
		FVector ToRootNoY = FVector(CurrentToRootMotionSyncNorm.X, 0.0f, CurrentToRootMotionSyncNorm.Z);
		ToRootNoY.Normalize();
		const float AngleAboutY = FMath::Acos(FVector::DotProduct(ToWorldNoY, ToRootNoY));
		float AngleAboutYNorm = FMath::DegreesToRadians(
			FRotator::NormalizeAxis(FMath::RadiansToDegrees(AngleAboutY)));
		if (ToWorldNoY.Z < 0.0f)
		{
			AngleAboutYNorm *= -1.0f;
		}

		FVector SkewedRootMotion = FVector::ZeroVector;
		float ProjectedScale = FVector::DotProduct(CurrentToWorldSync, CurrentToRootMotionSyncNorm) /
			CurrentToRootMotionSync.Size();
		if (ProjectedScale != 0.0f)
		{
			FMatrix ScaleMatrix;
			ScaleMatrix.SetIdentity();
			ScaleMatrix.SetAxis(0, FVector(ProjectedScale, 0.0f, 0.0f));
			ScaleMatrix.SetAxis(1, FVector(0.0f, 1.0f, 0.0f));
			ScaleMatrix.SetAxis(2, FVector(0.0f, 0.0f, 1.0f));

			FMatrix ShearXAlongYMatrix;
			ShearXAlongYMatrix.SetIdentity();
			ShearXAlongYMatrix.SetAxis(0, FVector(1.0f, FMath::Tan(AngleAboutZNorm), 0.0f));
			ShearXAlongYMatrix.SetAxis(1, FVector(0.0f, 1.0f, 0.0f));
			ShearXAlongYMatrix.SetAxis(2, FVector(0.0f, 0.0f, 1.0f));

			FMatrix ShearXAlongZMatrix;
			ShearXAlongZMatrix.SetIdentity();
			ShearXAlongZMatrix.SetAxis(0, FVector(1.0f, 0.0f, FMath::Tan(AngleAboutYNorm)));
			ShearXAlongZMatrix.SetAxis(1, FVector(0.0f, 1.0f, 0.0f));
			ShearXAlongZMatrix.SetAxis(2, FVector(0.0f, 0.0f, 1.0f));

			FMatrix ScaledSkewMatrix = ScaleMatrix * ShearXAlongYMatrix * ShearXAlongZMatrix;

			// Skew and scale the Root motion. 
			SkewedRootMotion = ScaledSkewMatrix.TransformVector(RootMotionInSyncSpace);
		}
		else if (!CurrentToRootMotionSync.IsZero() && !CurrentToWorldSync.IsZero() && !RootMotionInSyncSpace.
			IsZero())
		{
			// Figure out ratio between remaining Root and remaining World. Then project scaled length of current Root onto World.
			const float Scale = CurrentToWorldSync.Size() / CurrentToRootMotionSync.Size();
			const float StepTowardTarget = RootMotionInSyncSpace.ProjectOnTo(RootMotionInSyncSpace).Size();
			SkewedRootMotion = CurrentToWorldSyncNorm * (Scale * StepTowardTarget);
		}

		// Put our result back in world space.  
		FinalRootMotion.SetTranslation(ToRootSyncSpace.TransformVector(SkewedRootMotion));

		
	
	}
	if (RotationSetting.IsWarpRotation())
	{
		float TimeRemaining = EndTime - PreviousTime;
		const float Fraction = 1 - (EndTime - PreviousTime) / EndTime;
		if (RotationSetting.Curve)
		{
			TimeRemaining = EndTime - FMath::Clamp(RotationSetting.Curve->GetFloatValue(Fraction), 0, 1) * EndTime;
		}
		const FQuat Qt = this->WarpRotation( Character, InRootMotion, RootMotionTotal, TimeRemaining , DeltaSeconds);
		FinalRootMotion.SetRotation(Qt);
	}
	return FinalRootMotion;
}

bool FRootMotionSource_AnimWarping::UpdateStateFrom(const FRootMotionSource* SourceToTakeStateFrom,
                                                    bool bMarkForSimulatedCatchup)
{
	if (!FRootMotionSource::UpdateStateFrom(SourceToTakeStateFrom, bMarkForSimulatedCatchup))
	{
		return false;
	}

	return true;
}

void FRootMotionSource_AnimWarping::PrepareRootMotion(float SimulationTime, float MovementTickTime,
                                                      const ACharacter& Character,
                                                      const UCharacterMovementComponent& MoveComponent)
{
	RootMotionParams.Clear();

	if (Animation && Duration
		>
		SMALL_NUMBER && MovementTickTime > SMALL_NUMBER
	)
	{
		const float CurrEndTime = (AnimEndTime < 0 || AnimEndTime > Animation->GetPlayLength())
			                          ? Animation->GetPlayLength()
			                          : AnimEndTime;
		const float CalcDuration = CurrEndTime - StartTime;
		const float TimeScale = CalcDuration / Duration;
		const FTransform StartFootTransform = FTransform(StartRotation,
		                                                 StartLocation - FVector(
			                                                 0.f, 0.f,
			                                                 Character.GetCapsuleComponent()->
			                                                           GetScaledCapsuleHalfHeight()));
		const FTransform CurrChacterFootTransform = FTransform(StartRotation,
		                                                       Character.GetActorLocation() - FVector(
			                                                       0.f, 0.f,
			                                                       Character.GetCapsuleComponent()->
			                                                                 GetScaledCapsuleHalfHeight()));
		FTransform MeshTransformWS = Character.GetMesh()->GetComponentTransform();
		FTransform Mesh2CharInverse = StartFootTransform.GetRelativeTransform(MeshTransformWS);

		FTransform TargetTransform = ExtractRootMotion(AnimStartTime, CurrEndTime);
		TargetTransform = TargetTransform * MeshTransformWS; //模型世界空间的RM
		//通过逆矩阵把模型空间转换成actor空间
		const FTransform TargetTransformWS = Mesh2CharInverse * TargetTransform;

		if (!bInit)
		{
			bInit = true;
			SetTargetLocation(TargetTransformWS.GetLocation());
		}


		const float PrevTime = GetTime() * TimeScale;
		const float CurrTime = (GetTime() + SimulationTime) * TimeScale;
		const FTransform CurrRootMotion = ExtractRootMotion(PrevTime, CurrTime);
		//这个是世界空间的偏移
		FTransform WarpTransform = ProcessRootMotion(Character, CurrRootMotion, PrevTime, CurrTime, SimulationTime);
		//因为是世界空间的,所以是右乘
		FTransform WarpTransformWS = CurrChacterFootTransform * WarpTransform;
		const FVector CurrentLocation = Character.GetActorLocation() - FVector(
			0, 0, Character.GetCapsuleComponent()->GetScaledCapsuleHalfHeight());

		FVector Force = (WarpTransformWS.GetLocation() - CurrentLocation) / MovementTickTime;

		// Debug
#if ROOT_MOTION_DEBUG
		if (RMS::CVarRMS_Debug.GetValueOnGameThread() > 0)
		{
			const FVector LocDiff = MoveComponent.UpdatedComponent->GetComponentLocation() - CurrentLocation;
			const float DebugLifetime = 5;
			UE_LOG(LogTemp, Log, TEXT("Target = %s"), *TargetTransform.ToString());
			// Current
			DrawDebugCapsule(Character.GetWorld(), MoveComponent.UpdatedComponent->GetComponentLocation(),
			                 Character.GetSimpleCollisionHalfHeight(), Character.GetSimpleCollisionRadius(),
			                 FQuat::Identity, FColor::Red, false, DebugLifetime);

			// Current Target
			DrawDebugCapsule(Character.GetWorld(), WarpTransformWS.GetLocation() + LocDiff,
			                 Character.GetSimpleCollisionHalfHeight(), Character.GetSimpleCollisionRadius(),
			                 FQuat::Identity, FColor::Green, false, DebugLifetime);

			// Target
			DrawDebugCapsule(Character.GetWorld(), TargetTransformWS.GetLocation() + LocDiff,
			                 Character.GetSimpleCollisionHalfHeight(), Character.GetSimpleCollisionRadius(),
			                 FQuat::Identity, FColor::Blue, false, DebugLifetime);

			// Force
			DrawDebugLine(Character.GetWorld(), CurrentLocation, CurrentLocation + Force, FColor::Blue, false,
			              DebugLifetime);
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

FTransform FRootMotionSource_AnimWarping::ExtractRootMotion(float InStartTime, float InEndTime) const
{
	FTransform OutTransform;
	if (UAnimSequence* Seq = Cast<UAnimSequence>(Animation))
	{
		OutTransform = Seq->ExtractRootMotionFromRange(InStartTime, InEndTime);
	}
	else if (UAnimMontage* Montage = Cast<UAnimMontage>(Animation))
	{
		OutTransform = Montage->ExtractRootMotionFromTrackRange(InStartTime, InEndTime);
	}
	return OutTransform;
}

FQuat FRootMotionSource_AnimWarping::WarpRotation(const ACharacter& Character, const FTransform& RootMotionDelta,
                                                  const FTransform& RootMotionTotal, float TimeRemaining,
                                                  float DeltaSeconds)
{
	if (!IsValid(&Character))
	{
		return FQuat::Identity;
	}
	
	const FTransform& CharacterTransform = Character.GetActorTransform();
	const FQuat CurrentRotation = CharacterTransform.GetRotation();
	const FQuat TargetRotation = GetTargetRotation().Quaternion();
	const FQuat RemainingRootRotationInWorld = RootMotionTotal.GetRotation();
	const FQuat CurrentPlusRemainingRootMotion = RemainingRootRotationInWorld * CurrentRotation;
	
	const float PercentThisStep = FMath::Clamp(DeltaSeconds / TimeRemaining *  FMath::Max(0.01, RotationSetting.WarpMultiplier), 0.f, 1.f);
	const FQuat TargetRotThisFrame = FQuat::Slerp(CurrentPlusRemainingRootMotion, TargetRotation, PercentThisStep);
	const FQuat DeltaOut = TargetRotThisFrame * CurrentPlusRemainingRootMotion.Inverse();

	return (DeltaOut * RootMotionDelta.GetRotation());
}

bool FRootMotionSource_AnimWarping::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	if (!FRootMotionSource::NetSerialize(Ar, Map, bOutSuccess))
	{
		return false;
	}
	Ar << StartLocation; // TODO-RootMotionSource: Quantization
	Ar << StartRotation;
	Ar << AnimStartTime;
	Ar << AnimEndTime;
	Ar << Animation;
	Ar << bIgnoreZAxis;
	Ar << CachedEndTime;
	Ar << RotationSetting;

	bOutSuccess = true;
	return true;
}

FRootMotionSource* FRootMotionSource_AnimWarping::Clone() const
{
	FRootMotionSource_AnimWarping* CopyPtr = new FRootMotionSource_AnimWarping(*this);
	return CopyPtr;
}

bool FRootMotionSource_AnimWarping::Matches(const FRootMotionSource* Other) const
{
	if (!FRootMotionSource::Matches(Other))
	{
		return false;
	}
	const FRootMotionSource_AnimWarping* OtherCast = static_cast<const FRootMotionSource_AnimWarping*>(Other);

	return StartLocation == OtherCast->StartLocation &&
		StartRotation == OtherCast->StartRotation &&
		StartTime == OtherCast->StartTime &&
		AnimEndTime == OtherCast->AnimEndTime &&
		Animation == OtherCast->Animation &&
		bIgnoreZAxis == OtherCast->bIgnoreZAxis &&
		RotationSetting == OtherCast->RotationSetting;
}

bool FRootMotionSource_AnimWarping::MatchesAndHasSameState(const FRootMotionSource* Other) const
{
	if (!FRootMotionSource::MatchesAndHasSameState(Other))
	{
		return false;
	}

	return true;
}

UScriptStruct* FRootMotionSource_AnimWarping::GetScriptStruct() const
{
	return FRootMotionSource_AnimWarping::StaticStruct();
}

FString FRootMotionSource_AnimWarping::ToSimpleString() const
{
	return FString::Printf(
		TEXT("[ID:%u]FRootMotionSource_AnimWarping %s"), LocalID, *InstanceName.GetPlainNameString());
}

void FRootMotionSource_AnimWarping::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(Animation);
	FRootMotionSource::AddReferencedObjects(Collector);
}

#pragma endregion FRootMotionSource_AnimWarping
//*******************************FRootMotionSource_AnimWarping_FinalPoint*********************************************

#pragma region FRootMotionSource_AnimWarping_FinalPoint

void FRootMotionSource_AnimWarping_FinalPoint::PrepareRootMotion(float SimulationTime,
                                                                 float MovementTickTime,
                                                                 const ACharacter& Character,
                                                                 const UCharacterMovementComponent& MoveComponent)
{
	RootMotionParams.Clear();

	if (Animation && Duration
		>
		SMALL_NUMBER && MovementTickTime > SMALL_NUMBER
	)
	{
		const float CurrEndTime = (AnimEndTime < 0 || AnimEndTime > Animation->GetPlayLength())
			                          ? Animation->GetPlayLength()
			                          : AnimEndTime;
		const float CalcDuration = CurrEndTime - StartTime;
		const float TimeScale = CalcDuration / Duration;
		const FTransform StartFootTransform = FTransform(StartRotation,
		                                                 StartLocation - FVector(
			                                                 0.f, 0.f,
			                                                 Character.GetCapsuleComponent()->
			                                                           GetScaledCapsuleHalfHeight()));
		const FTransform CurrChacterFootTransform = FTransform(StartRotation,
		                                                       Character.GetActorLocation() - FVector(
			                                                       0.f, 0.f,
			                                                       Character.GetCapsuleComponent()->
			                                                                 GetScaledCapsuleHalfHeight()));
		FTransform MeshTransformWS = Character.GetMesh()->GetComponentTransform();
		FTransform Mesh2CharInverse = StartFootTransform.GetRelativeTransform(MeshTransformWS);

		FTransform TargetTransform = ExtractRootMotion(AnimStartTime, CurrEndTime);
		TargetTransform = TargetTransform * MeshTransformWS; //模型世界空间的RM
		//通过逆矩阵把模型空间转换成actor空间
		const FTransform TargetTransformWS = Mesh2CharInverse * TargetTransform;

		if (!bInit)
		{
			bInit = true;
			SetTargetLocation(TargetLocation);
			SetTargetRotation(TargetRotation);
		}


		const float PrevTime = GetTime() * TimeScale;
		const float CurrTime = (GetTime() + SimulationTime) * TimeScale;
		const FTransform CurrRootMotion = ExtractRootMotion(PrevTime, CurrTime);
		//这个是世界空间的偏移
		FTransform WarpTransform = ProcessRootMotion(Character, CurrRootMotion, PrevTime, CurrTime, SimulationTime);
		//为了得到正确的偏移, 先要把Rotation拿出来, 同时我们只需要Yaw
		FRotator WarpRot = WarpTransform.GetRotation().Rotator();
		WarpRot.Pitch = 0;
		WarpRot.Roll = 0;
		WarpTransform.SetRotation(FQuat::Identity);
		//因为是世界空间的,所以是右乘
		FTransform WarpTransformWS = CurrChacterFootTransform * WarpTransform;
		const FVector CurrentLocation = Character.GetActorLocation() - FVector(
			0, 0, Character.GetCapsuleComponent()->GetScaledCapsuleHalfHeight());

		FVector Force = (WarpTransformWS.GetLocation() - CurrentLocation) / MovementTickTime;

		// Debug
#if ROOT_MOTION_DEBUG
		if (RMS::CVarRMS_Debug.GetValueOnGameThread() > 0)
		{
			const FVector LocDiff = MoveComponent.UpdatedComponent->GetComponentLocation() - CurrentLocation;
			const float DebugLifetime = 5;
			UE_LOG(LogTemp, Log, TEXT("Target = %s"), *TargetTransform.ToString());
			// Current
			DrawDebugCapsule(Character.GetWorld(), MoveComponent.UpdatedComponent->GetComponentLocation(),
			                 Character.GetSimpleCollisionHalfHeight(), Character.GetSimpleCollisionRadius(),
			                 FQuat::Identity, FColor::Red, false, DebugLifetime);

			// Current Target
			DrawDebugCapsule(Character.GetWorld(), WarpTransformWS.GetLocation() + LocDiff,
			                 Character.GetSimpleCollisionHalfHeight(), Character.GetSimpleCollisionRadius(),
			                 FQuat::Identity, FColor::Green, false, DebugLifetime);

			// Target
			// DrawDebugCapsule(Character.GetWorld(), TargetTransformWS.GetLocation() + LocDiff,
			//                  Character.GetSimpleCollisionHalfHeight(), Character.GetSimpleCollisionRadius(),
			//                  FQuat::Identity, FColor::Blue, false, DebugLifetime);

			// Force
			DrawDebugLine(Character.GetWorld(), CurrentLocation, CurrentLocation + Force, FColor::Blue, false,
			              DebugLifetime);
		}
#endif

		FTransform NewTransform(Force);
		if (RotationSetting.IsWarpRotation())
		{
			NewTransform.SetRotation(WarpRot.Quaternion());
		}


		RootMotionParams.Set(NewTransform);
	}
	else
	{
		checkf(Duration > SMALL_NUMBER, TEXT("FRootMotionSource_MoveToForce prepared with invalid duration."));
	}

	SetTime(GetTime() + SimulationTime);
}

bool FRootMotionSource_AnimWarping_FinalPoint::NetSerialize(FArchive& Ar, UPackageMap* Map,
                                                            bool& bOutSuccess)
{
	if (!FRootMotionSource::NetSerialize(Ar, Map, bOutSuccess))
	{
		return false;
	}
	Ar << TargetLocation;
	bOutSuccess = true;
	return bOutSuccess;
}

FRootMotionSource* FRootMotionSource_AnimWarping_FinalPoint::Clone() const
{
	FRootMotionSource_AnimWarping_FinalPoint* CopyPtr = new FRootMotionSource_AnimWarping_FinalPoint(*this);
	return CopyPtr;
}

bool FRootMotionSource_AnimWarping_FinalPoint::Matches(const FRootMotionSource* Other) const
{
	if (!FRootMotionSource::Matches(Other))
	{
		return false;
	}
	const FRootMotionSource_AnimWarping_FinalPoint* OtherCast = static_cast<const
		FRootMotionSource_AnimWarping_FinalPoint*>(Other);

	return TargetLocation == OtherCast->TargetLocation;;
}


UScriptStruct* FRootMotionSource_AnimWarping_FinalPoint::GetScriptStruct() const
{
	return FRootMotionSource_AnimWarping_FinalPoint::StaticStruct();
}

FString FRootMotionSource_AnimWarping_FinalPoint::ToSimpleString() const
{
	return FString::Printf(
		TEXT("[ID:%u]FRootMotionSource_AnimWarping_FinalPoint %s"), LocalID, *InstanceName.GetPlainNameString());
}

#pragma endregion FRootMotionSource_AnimWarping_FinalPoint

//***************************FRootMotionSource_AnimWarping_MultiTargets********************************
#pragma region FRootMotionSource_AnimWarping_MultiTargets

bool FRootMotionSource_AnimWarping_MultiTargets::UpdateTriggerTarget(float SimulationTime, float TimeScale)
{
	const float CurrTime = (GetTime() + SimulationTime) * TimeScale;
	const int32 idx = TriggerDatas.Find(CurrTriggerData);
	if (idx >= 0 && idx < TriggerDatas.Num() - 1 && CurrTime > CurrTriggerData.EndTime)
	{
		LastTriggerData = CurrTriggerData;
		CurrTriggerData = TriggerDatas[idx + 1 ];
		RotationSetting = CurrTriggerData.RotationSetting;
		return true;
	}

	return false;
}

void FRootMotionSource_AnimWarping_MultiTargets::PrepareRootMotion(float SimulationTime, float MovementTickTime,
                                                                   const ACharacter& Character,
                                                                   const UCharacterMovementComponent& MoveComponent)
{
	RootMotionParams.Clear();

	if (TriggerDatas.Num() > 0 && Animation && Duration > SMALL_NUMBER && MovementTickTime > SMALL_NUMBER)
	{
		AnimEndTime = Animation->GetPlayLength();
		const float TimeScale = AnimEndTime / Duration;
		float RMStartTime = 0;
		float RMEndTime = 0;
		if (!bInit)
		{
			bInit = true;
			CurrTriggerData = TriggerDatas[0];
			RotationSetting = CurrTriggerData.RotationSetting;
			SetTargetLocation(CurrTriggerData.Target);

			if (RotationSetting.IsWarpRotation())
			{
				if (RotationSetting.Mode == ERMSRotationMode::Custom)
				{
					SetTargetRotation(RotationSetting.TargetRotation);
				}
				else
				{
					SetTargetRotation((GetTargetLocation() - StartLocation).Rotation());
				}
			}
			else
			{
				SetTargetRotation(StartRotation);
			}
			
	
			SetCurrentAnimEndTime(CurrTriggerData.EndTime);
			RMEndTime = CurrTriggerData.EndTime;
		}
		else if (UpdateTriggerTarget(SimulationTime, TimeScale))
		{
			SetCurrentAnimEndTime(CurrTriggerData.EndTime);
			SetTargetLocation(CurrTriggerData.Target);

			if (RotationSetting.IsWarpRotation())
			{
				if (RotationSetting.Mode == ERMSRotationMode::Custom)
				{
					SetTargetRotation(RotationSetting.TargetRotation);
				}
				else
				{
					SetTargetRotation((GetTargetLocation() - LastTriggerData.Target).Rotation());
				}
			}
			
			RMStartTime = CurrTriggerData.StartTime;
			RMEndTime = CurrTriggerData.EndTime;
		}


		const FTransform StartFootTransform = FTransform(StartRotation,
		                                                 StartLocation - FVector(
			                                                 0.f, 0.f,
			                                                 Character.GetCapsuleComponent()->
			                                                           GetScaledCapsuleHalfHeight()));
		const FTransform CurrChacterFootTransform = FTransform(StartRotation,
		                                                       Character.GetActorLocation() - FVector(
			                                                       0.f, 0.f,
			                                                       Character.GetCapsuleComponent()->
			                                                                 GetScaledCapsuleHalfHeight()));
		FTransform MeshTransformWS = Character.GetMesh()->GetComponentTransform();
		FTransform Mesh2CharInverse = StartFootTransform.GetRelativeTransform(MeshTransformWS);
		FTransform RootMotionTargetTransform = ExtractRootMotion(RMStartTime, RMEndTime);
		RootMotionTargetTransform = RootMotionTargetTransform * MeshTransformWS; //模型世界空间的RM
		//通过逆矩阵把模型空间转换成actor空间
		const FTransform RootMotionTargetTransformWS = Mesh2CharInverse * RootMotionTargetTransform;


		const float PrevTime = GetTime() * TimeScale;
		const float CurrTime = (GetTime() + SimulationTime) * TimeScale;

		const FTransform CurrRootMotion = ExtractRootMotion(PrevTime, CurrTime);
		//这个是世界空间的偏移
		FTransform WarpTransform = ProcessRootMotion(Character, CurrRootMotion, PrevTime, CurrTime, SimulationTime);
		FRotator WarpRot = WarpTransform.Rotator();
		WarpRot.Pitch = 0;
		WarpRot.Roll = 0;
		WarpTransform.SetRotation(FQuat::Identity);
		
		//因为是世界空间的,所以是右乘
		FTransform WarpTransformWS = CurrChacterFootTransform * WarpTransform;
		const FVector CurrentLocation = Character.GetActorLocation() - FVector(
			0, 0, Character.GetCapsuleComponent()->GetScaledCapsuleHalfHeight());

		FVector Force = (WarpTransformWS.GetLocation() - CurrentLocation) / MovementTickTime;

		// Debug
#if ROOT_MOTION_DEBUG
		if (RMS::CVarRMS_Debug.GetValueOnGameThread() > 0)
		{
			const FVector LocDiff = MoveComponent.UpdatedComponent->GetComponentLocation() - CurrentLocation;
			const float DebugLifetime = 5;
			const FVector LastPos = TriggerDatas[TriggerDatas.Num() - 1].Target;
			UE_LOG(LogTemp, Log, TEXT("Target = %s"), *RootMotionTargetTransform.ToString());
			// Current
			DrawDebugCapsule(Character.GetWorld(), MoveComponent.UpdatedComponent->GetComponentLocation(),
			                 Character.GetSimpleCollisionHalfHeight(), Character.GetSimpleCollisionRadius(),
			                 FQuat::Identity, FColor::Red, false, DebugLifetime);

			// Current Target
			DrawDebugCapsule(Character.GetWorld(), WarpTransformWS.GetLocation() + LocDiff,
			                 Character.GetSimpleCollisionHalfHeight(), Character.GetSimpleCollisionRadius(),
			                 FQuat::Identity, FColor::Green, false, DebugLifetime);

			// Target
			DrawDebugCapsule(Character.GetWorld(), LastPos + LocDiff, Character.GetSimpleCollisionHalfHeight(),
			                 Character.GetSimpleCollisionRadius(), FQuat::Identity, FColor::Blue, false,
			                 DebugLifetime);

			// Force
			DrawDebugLine(Character.GetWorld(), CurrentLocation, CurrentLocation + Force, FColor::Blue, false,
			              DebugLifetime);

			DrawDebugCapsule(Character.GetWorld(), CurrTriggerData.Target + LocDiff,
			                 Character.GetSimpleCollisionHalfHeight(), Character.GetSimpleCollisionRadius(),
			                 FQuat::Identity, FColor::Purple, false, DebugLifetime);
		}
#endif

		FTransform NewTransform(WarpRot, Force);
		RootMotionParams.Set(NewTransform);
	}
	else
	{
		checkf(Duration > SMALL_NUMBER, TEXT("FRootMotionSource_MoveToForce prepared with invalid duration."));
	}

	SetTime(GetTime() + SimulationTime);
}

bool FRootMotionSource_AnimWarping_MultiTargets::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	if (!FRootMotionSource::NetSerialize(Ar, Map, bOutSuccess))
	{
		return false;
	}
	Ar << TriggerDatas;
	bOutSuccess = true;
	return bOutSuccess;
}

FRootMotionSource* FRootMotionSource_AnimWarping_MultiTargets::Clone() const
{
	FRootMotionSource_AnimWarping_MultiTargets* CopyPtr = new FRootMotionSource_AnimWarping_MultiTargets(*this);
	return CopyPtr;
}

bool FRootMotionSource_AnimWarping_MultiTargets::Matches(const FRootMotionSource* Other) const
{
	if (!FRootMotionSource::Matches(Other))
	{
		return false;
	}
	const FRootMotionSource_AnimWarping_MultiTargets* OtherCast = static_cast<const
		FRootMotionSource_AnimWarping_MultiTargets*>(Other);

	return TriggerDatas == OtherCast->TriggerDatas;;
}

UScriptStruct* FRootMotionSource_AnimWarping_MultiTargets::GetScriptStruct() const
{
	return FRootMotionSource_AnimWarping_MultiTargets::StaticStruct();
}

FString FRootMotionSource_AnimWarping_MultiTargets::ToSimpleString() const
{
	return FString::Printf(
		TEXT("[ID:%u]FRootMotionSource_AnimWarping_MultiTargets %s"), LocalID, *InstanceName.GetPlainNameString());
}
#pragma endregion FRootMotionSource_AnimWarping_MultiTargets
UE_ENABLE_OPTIMIZATION
