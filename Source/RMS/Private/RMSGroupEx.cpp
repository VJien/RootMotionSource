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

PRAGMA_DISABLE_OPTIMIZATION


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
		if (RMS::CVarRMS_Debug.GetValueOnGameThread() != 0)
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
			if (RootMotionSourceDebug::CVarDebugRootMotionSources.GetValueOnGameThread() != 0)
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
			if (RootMotionSourceDebug::CVarDebugRootMotionSources.GetValueOnGameThread() != 0)
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
		if (RMS::CVarRMS_Debug.GetValueOnGameThread() != 0)
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
		if (RMS::CVarRMS_Debug.GetValueOnGameThread() != 0)
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
		if (RMS::CVarRMS_Debug.GetValueOnGameThread() != 0)
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
PRAGMA_ENABLE_OPTIMIZATION
