// Fill out your copyright notice in the Description page of Project Settings.


#include "RootMotionSourceGroupEx.h"
#include "Curves/CurveVector.h"
#include "Curves/CurveFloat.h"
#include "DrawDebugHelpers.h"
#include "RootMotionSourceLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

PRAGMA_DISABLE_OPTIMIZATION

namespace RMS
{
extern TAutoConsoleVariable<int32> CVarRMS_Debug;
}

FRootMotionSource_PathMoveToForce::FRootMotionSource_PathMoveToForce()
{
}

FVector FRootMotionSource_PathMoveToForce::GetPathOffsetInWorldSpace(const float MoveFraction, FRootMotionSourcePathMoveToData Data, FVector Start) const
{
	if (Data.PathOffsetCurve)
	{
		// Calculate path offset
		const FVector PathOffsetInFacingSpace = URootMotionSourceLibrary::EvaluateVectorCurveAtFraction(*Data.PathOffsetCurve, MoveFraction);
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
			MoveFraction = URootMotionSourceLibrary::EvaluateFloatCurveAtFraction(*CurrData.TimeMappingCurve, MoveFraction);
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

//*******************************************

FTransform FRootMotionSource_MotionWarping::ProcessRootMotion(const ACharacter& Character, const FTransform& InRootMotion, float InPreviousTime, float InCurrentTime)
{
	FTransform FinalRootMotion = InRootMotion;
	if (!Animation || !IsValid(&Character))
	{
		return InRootMotion;
	}
	float EndTime = AnimEndTime;
	if (EndTime < 0 || EndTime>Animation->GetPlayLength())
	{
		EndTime = Animation->GetPlayLength();
	}
	
	const FTransform RootMotionTotal = ExtractRootMotion(InPreviousTime, EndTime);
	const FTransform RootMotionDelta = ExtractRootMotion(InPreviousTime, FMath::Min(InCurrentTime, EndTime));

	if (!RootMotionDelta.GetTranslation().IsNearlyZero())
	{
		const FTransform CurrentTransform = FTransform(
			Character.GetActorQuat(),
			Character.GetActorLocation() - FVector(0.f, 0.f, Character.GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));
		//剩下的总共的RootMotion
		const FTransform RootMotionTotalWorldSpace = CurrentTransform * Character.GetMesh()->ConvertLocalRootMotionToWorld(RootMotionTotal);
		//这一帧的RootMotion
		const FTransform RootMotionDeltaWorldSpace = Character.GetMesh()->ConvertLocalRootMotionToWorld(RootMotionDelta);
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
		float AngleAboutZNorm = FMath::DegreesToRadians(FRotator::NormalizeAxis(FMath::RadiansToDegrees(AngleAboutZ)));
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
		float AngleAboutYNorm = FMath::DegreesToRadians(FRotator::NormalizeAxis(FMath::RadiansToDegrees(AngleAboutY)));
		if (ToWorldNoY.Z < 0.0f)
		{
			AngleAboutYNorm *= -1.0f;
		}

		FVector SkewedRootMotion = FVector::ZeroVector;
		float ProjectedScale = FVector::DotProduct(CurrentToWorldSync, CurrentToRootMotionSyncNorm) / CurrentToRootMotionSync.Size();
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
		else if (!CurrentToRootMotionSync.IsZero() && !CurrentToWorldSync.IsZero() && !RootMotionInSyncSpace.IsZero())
		{
			// Figure out ratio between remaining Root and remaining World. Then project scaled length of current Root onto World.
			const float Scale = CurrentToWorldSync.Size() / CurrentToRootMotionSync.Size();
			const float StepTowardTarget = RootMotionInSyncSpace.ProjectOnTo(RootMotionInSyncSpace).Size();
			SkewedRootMotion = CurrentToWorldSyncNorm * (Scale * StepTowardTarget);
		}

		// Put our result back in world space.  
		FinalRootMotion.SetTranslation(ToRootSyncSpace.TransformVector(SkewedRootMotion));
	}
	return FinalRootMotion;
}

bool FRootMotionSource_MotionWarping::UpdateStateFrom(const FRootMotionSource* SourceToTakeStateFrom, bool bMarkForSimulatedCatchup)
{
	if (!FRootMotionSource::UpdateStateFrom(SourceToTakeStateFrom, bMarkForSimulatedCatchup))
	{
		return false;
	}

	return true;
}

void FRootMotionSource_MotionWarping::PrepareRootMotion(float SimulationTime, float MovementTickTime, const ACharacter& Character, const UCharacterMovementComponent& MoveComponent)
{
	RootMotionParams.Clear();

	if (Animation && Duration > SMALL_NUMBER && MovementTickTime > SMALL_NUMBER)
	{
		const float CurrEndTime = (AnimEndTime < 0 || AnimEndTime > Animation->GetPlayLength()) ? Animation->GetPlayLength() : AnimEndTime;
		const float CalcDuration = CurrEndTime - StartTime;
		const float TimeScale = CalcDuration / Duration;
		FTransform Mesh2Char = Character.GetMesh()->GetComponentTransform().GetRelativeTransform(Character.GetActorTransform());
		Mesh2Char.SetLocation(FVector::Zero());
		
		const FTransform StartChacterFootTransform = FTransform(StartRotation,StartLocation - FVector(0.f, 0.f, Character.GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));
		const FTransform CurrChacterFootTransform = FTransform(StartRotation,Character.GetActorLocation() - FVector(0.f, 0.f, Character.GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));
		FTransform TargetTransform = ExtractRootMotion(AnimStartTime, CurrEndTime);
		TargetTransform = TargetTransform * Mesh2Char;
		const FTransform  TargetTransformWS =  TargetTransform * StartChacterFootTransform;
		if (!bInit)
		{
			bInit = true;
			SetTargetLocation(TargetTransformWS.GetLocation());
		}
		
		
		const float PrevTime = GetTime() * TimeScale;
		const float CurrTime = (GetTime() + SimulationTime )  * TimeScale;
		const FTransform CurrRootMotion = ExtractRootMotion(PrevTime, CurrTime);
		FTransform WarpTransform = ProcessRootMotion(Character, CurrRootMotion, PrevTime, CurrTime);
		WarpTransform = WarpTransform * Mesh2Char;
		FTransform WarpTransformWS = CurrChacterFootTransform *  WarpTransform ;
		const FVector CurrentLocation = Character.GetActorLocation() - FVector(0,0,Character.GetCapsuleComponent()->GetScaledCapsuleHalfHeight());

		FVector Force = (WarpTransformWS.GetLocation() - CurrentLocation) / MovementTickTime;
		
		// Debug
#if ROOT_MOTION_DEBUG
		if (RootMotionSourceDebug::CVarDebugRootMotionSources.GetValueOnGameThread() != 0)
		{
			const FVector LocDiff = MoveComponent.UpdatedComponent->GetComponentLocation() - CurrentLocation;
			const float DebugLifetime = 5;
			UE_LOG(LogTemp, Log, TEXT("Target = %s"),*TargetTransform.ToString());
			// Current
			DrawDebugCapsule(Character.GetWorld(), MoveComponent.UpdatedComponent->GetComponentLocation(), Character.GetSimpleCollisionHalfHeight(), Character.GetSimpleCollisionRadius(), FQuat::Identity, FColor::Red, false, DebugLifetime);

			// Current Target
			DrawDebugCapsule(Character.GetWorld(), WarpTransformWS.GetLocation() + LocDiff, Character.GetSimpleCollisionHalfHeight(), Character.GetSimpleCollisionRadius(), FQuat::Identity, FColor::Green, false, DebugLifetime);

			// Target
			DrawDebugCapsule(Character.GetWorld(), TargetTransformWS.GetLocation() + LocDiff, Character.GetSimpleCollisionHalfHeight(), Character.GetSimpleCollisionRadius(), FQuat::Identity, FColor::Blue, false, DebugLifetime);

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

FTransform FRootMotionSource_MotionWarping::ExtractRootMotion(float InStartTime, float InEndTime) const
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

bool FRootMotionSource_MotionWarping::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	if (!FRootMotionSource::NetSerialize(Ar, Map, bOutSuccess))
	{
		return false;
	}
	Ar << StartLocation; // TODO-RootMotionSource: Quantization
	Ar << AnimStartTime;
	Ar << AnimEndTime;
	Ar << Animation;
	Ar << StartRotation;
	Ar << bIgnoreZAxis;

	bOutSuccess = true;
	return true;
}

FRootMotionSource* FRootMotionSource_MotionWarping::Clone() const
{
	return FRootMotionSource::Clone();
}

bool FRootMotionSource_MotionWarping::Matches(const FRootMotionSource* Other) const
{
	if (!FRootMotionSource::Matches(Other))
	{
		return false;
	}
	const FRootMotionSource_MotionWarping* OtherCast = static_cast<const FRootMotionSource_MotionWarping*>(Other);

	return StartLocation == OtherCast->StartLocation &&
		StartRotation == OtherCast->StartRotation &&
		StartTime == OtherCast->StartTime &&
		AnimEndTime == OtherCast->AnimEndTime &&
		Animation == OtherCast->Animation &&
		bIgnoreZAxis == OtherCast->bIgnoreZAxis;
}

bool FRootMotionSource_MotionWarping::MatchesAndHasSameState(const FRootMotionSource* Other) const
{
	if (!FRootMotionSource::MatchesAndHasSameState(Other))
	{
		return false;
	}

	return true;
}

UScriptStruct* FRootMotionSource_MotionWarping::GetScriptStruct() const
{
	return FRootMotionSource_MotionWarping::StaticStruct();
}

FString FRootMotionSource_MotionWarping::ToSimpleString() const
{
	return FString::Printf(TEXT("[ID:%u]FRootMotionSource_MotionWarping %s"), LocalID, *InstanceName.GetPlainNameString());
}

void FRootMotionSource_MotionWarping::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(Animation);
	FRootMotionSource::AddReferencedObjects(Collector);
}


PRAGMA_ENABLE_OPTIMIZATION