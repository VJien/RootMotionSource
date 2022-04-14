// Fill out your copyright notice in the Description page of Project Settings.


#include "RMSLibrary.h"

#include "AnimNotifyState_RMS.h"
#include "Experimental/RMSComponent.h"
#include "RMSGroupEx.h"
#include "Components/CapsuleComponent.h"
#include "Curves/CurveVector.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

class UAnimNotifyState_RMS_Warping;

PRAGMA_DISABLE_OPTIMIZATION

float URMSLibrary::EvaluateFloatCurveAtFraction(const UCurveFloat& Curve, const float Fraction)
{
	float MinCurveTime(0.f);
	float MaxCurveTime(1.f);

	Curve.GetTimeRange(MinCurveTime, MaxCurveTime);
	return Curve.GetFloatValue(FMath::GetRangeValue(FVector2D(MinCurveTime, MaxCurveTime), Fraction));
}

FVector URMSLibrary::EvaluateVectorCurveAtFraction(const UCurveVector& Curve, const float Fraction)
{
	float MinCurveTime(0.f);
	float MaxCurveTime(1.f);

	Curve.GetTimeRange(MinCurveTime, MaxCurveTime);
	return Curve.GetVectorValue(FMath::GetRangeValue(FVector2D(MinCurveTime, MaxCurveTime), Fraction));
}


int32 URMSLibrary::ApplyRootMotionSource_MoveToForce(UCharacterMovementComponent* MovementComponent,
                                                                  FName InstanceName,
                                                                  FVector StartLocation, FVector TargetLocation,
                                                                  float Duration,
                                                                  int32 Priority,
                                                                  UCurveVector* PathOffsetCurve,
                                                                  float StartTime,
                                                                  ERMSApplyMode ApplyMode,
                                                                  FRMSSetting_Move Setting)
{
	if (!MovementComponent)
	{
		return -1;
	}
	const int32 NewPriority = CalcPriorityByApplyMode(MovementComponent, InstanceName, Priority, ApplyMode);
	if (NewPriority < 0)
	{
		return -1;
	}
	TSharedPtr<FRootMotionSource_MoveToForce> MoveToForce = MakeShared<FRootMotionSource_MoveToForce>();
	MoveToForce->InstanceName = InstanceName == NAME_None ? TEXT("MoveTo") : InstanceName;
	MoveToForce->AccumulateMode = Setting.AccumulateMod;
	MoveToForce->Settings.SetFlag(
		static_cast<ERootMotionSourceSettingsFlags>(static_cast<uint8>(Setting.SourcesSetting)));
	MoveToForce->Priority = NewPriority;
	MoveToForce->TargetLocation = TargetLocation;
	MoveToForce->StartLocation = StartLocation;
	MoveToForce->Duration = Duration;
	MoveToForce->bRestrictSpeedToExpected = Setting.bRestrictSpeedToExpected;
	MoveToForce->PathOffsetCurve = PathOffsetCurve;
	MoveToForce->FinishVelocityParams.Mode = static_cast<ERootMotionFinishVelocityMode>(static_cast<uint8>(Setting.
		VelocityOnFinishMode));
	MoveToForce->FinishVelocityParams.SetVelocity = Setting.FinishSetVelocity;
	MoveToForce->FinishVelocityParams.ClampVelocity = Setting.FinishClampVelocity;
	MoveToForce->SetTime(StartTime);
	return MovementComponent->ApplyRootMotionSource(MoveToForce);
}

int32 URMSLibrary::ApplyRootMotionSource_JumpForce(UCharacterMovementComponent* MovementComponent,
                                                                FName InstanceName,
                                                                FRotator Rotation, float Duration, float Distance,
                                                                float Height, int32 Priority,
                                                                UCurveVector* PathOffsetCurve,
                                                                UCurveFloat* TimeMappingCurve,
                                                                float StartTime,
                                                                ERMSApplyMode ApplyMode,
                                                                FRMSSetting_Jump Setting)
{
	if (!MovementComponent)
	{
		return -1;
	}
	const int32 NewPriority = CalcPriorityByApplyMode(MovementComponent, InstanceName, Priority, ApplyMode);
	if (NewPriority < 0)
	{
		return -1;
	}
	TSharedPtr<FRootMotionSource_JumpForce> JumpForce = MakeShared<FRootMotionSource_JumpForce>();
	JumpForce->InstanceName = InstanceName == NAME_None ? TEXT("Jump") : InstanceName;
	JumpForce->AccumulateMode = Setting.AccumulateMod;
	JumpForce->Priority = NewPriority;
	JumpForce->Duration = Duration;
	JumpForce->Rotation = Rotation;
	JumpForce->Distance = Distance;
	JumpForce->Height = Height;
	JumpForce->bDisableTimeout = Setting.bFinishOnLanded; // If we finish on landed, we need to disable force's timeout
	JumpForce->PathOffsetCurve = PathOffsetCurve;
	JumpForce->TimeMappingCurve = TimeMappingCurve;
	JumpForce->FinishVelocityParams.Mode = static_cast<ERootMotionFinishVelocityMode>(static_cast<uint8>(Setting.
		VelocityOnFinishMode));
	JumpForce->FinishVelocityParams.SetVelocity = Setting.FinishSetVelocity;
	JumpForce->FinishVelocityParams.ClampVelocity = Setting.FinishClampVelocity;
	JumpForce->SetTime(StartTime);
	return MovementComponent->ApplyRootMotionSource(JumpForce);
}

int32 URMSLibrary::ApplyRootMotionSource_DynamicMoveToForce(UCharacterMovementComponent* MovementComponent,
                                                                         FName InstanceName, FVector StartLocation,
                                                                         FVector TargetLocation, float Duration,
                                                                         int32 Priority,
                                                                         UCurveVector* PathOffsetCurve,
                                                                         UCurveFloat* TimeMappingCurve,
                                                                         float StartTime,
                                                                         ERMSApplyMode ApplyMode,
                                                                         FRMSSetting_Move Setting)
{
	if (!MovementComponent)
	{
		return -1;
	}
	const int32 NewPriority = CalcPriorityByApplyMode(MovementComponent, InstanceName, Priority, ApplyMode);
	if (NewPriority < 0)
	{
		return -1;
	}
	TSharedPtr<FRootMotionSource_MoveToDynamicForce> MoveToActorForce = MakeShared<
		FRootMotionSource_MoveToDynamicForce>();
	MoveToActorForce->InstanceName = InstanceName == NAME_None ? TEXT("DynamicMoveTo") : InstanceName;
	MoveToActorForce->AccumulateMode = Setting.AccumulateMod;
	MoveToActorForce->Settings.SetFlag(
		static_cast<ERootMotionSourceSettingsFlags>(static_cast<uint8>(Setting.SourcesSetting)));
	MoveToActorForce->Priority = NewPriority;
	MoveToActorForce->InitialTargetLocation = TargetLocation;
	MoveToActorForce->TargetLocation = TargetLocation;
	MoveToActorForce->StartLocation = StartLocation;
	MoveToActorForce->Duration = FMath::Max(Duration, KINDA_SMALL_NUMBER);
	MoveToActorForce->bRestrictSpeedToExpected = Setting.bRestrictSpeedToExpected;
	MoveToActorForce->PathOffsetCurve = PathOffsetCurve;
	MoveToActorForce->TimeMappingCurve = TimeMappingCurve;
	MoveToActorForce->FinishVelocityParams.Mode = static_cast<ERootMotionFinishVelocityMode>(static_cast<uint8>(Setting.
		VelocityOnFinishMode));
	MoveToActorForce->FinishVelocityParams.SetVelocity = Setting.FinishSetVelocity;
	MoveToActorForce->FinishVelocityParams.ClampVelocity = Setting.FinishClampVelocity;
	MoveToActorForce->SetTime(StartTime);
	return MovementComponent->ApplyRootMotionSource(MoveToActorForce);
}

int32 URMSLibrary::ApplyRootMotionSource_MoveToForce_Parabola(
	UCharacterMovementComponent* MovementComponent,
	FName InstanceName,
	FVector StartLocation, FVector TargetLocation,
	float Duration,
	int32 Priority,
	UCurveFloat* ParabolaCurve,
	UCurveFloat* TimeMappingCurve,
	int32 Segment,
	float StartTime,
	ERMSApplyMode ApplyMode,
	FRMSSetting_Move Setting)
{
	if (!MovementComponent || Duration <= 0)
	{
		return -1;
	}
	const int32 NewPriority = CalcPriorityByApplyMode(MovementComponent, InstanceName, Priority, ApplyMode);
	if (NewPriority < 0)
	{
		return -1;
	}
	TSharedPtr<FRootMotionSource_MoveToDynamicForce> MoveToForce = MakeShared<FRootMotionSource_MoveToDynamicForce>();
	MoveToForce->InstanceName = InstanceName == NAME_None ? TEXT("ParabolaMoveTo") : InstanceName;
	MoveToForce->AccumulateMode = Setting.AccumulateMod;
	MoveToForce->Settings.SetFlag(
		static_cast<ERootMotionSourceSettingsFlags>(static_cast<uint8>(Setting.SourcesSetting)));
	MoveToForce->Priority = NewPriority;
	MoveToForce->StartLocation = StartLocation;
	MoveToForce->Duration = Duration;
	MoveToForce->bRestrictSpeedToExpected = Setting.bRestrictSpeedToExpected;
	FVector Target = TargetLocation;
	UCurveVector* PathCurve = NewObject<UCurveVector>();

	if (ParabolaCurve)
	{
		const float OffsetZ = (TargetLocation - StartLocation).Z;
		FRichCurve ZCurve;
		for (int32 i = 0; i <= Segment; i++)
		{
			const float Fraction = static_cast<float>(i) / static_cast<float>(Segment);
			const float FractionZ = FMath::Lerp<float, float>(0, TargetLocation.Z - StartLocation.Z, Fraction);
			const float Value = ParabolaCurve->GetFloatValue(Fraction);
			ZCurve.AddKey(Fraction, OffsetZ * Value - FractionZ);
		}

		PathCurve->FloatCurves[2] = ZCurve;
	}

	MoveToForce->TargetLocation = Target;
	MoveToForce->TimeMappingCurve = TimeMappingCurve;
	MoveToForce->PathOffsetCurve = PathCurve;
	MoveToForce->FinishVelocityParams.Mode = static_cast<ERootMotionFinishVelocityMode>(static_cast<uint8>(Setting.
		VelocityOnFinishMode));
	MoveToForce->FinishVelocityParams.SetVelocity = Setting.FinishSetVelocity;
	MoveToForce->FinishVelocityParams.ClampVelocity = Setting.FinishClampVelocity;
	MoveToForce->SetTime(StartTime);
	return MovementComponent->ApplyRootMotionSource(MoveToForce);
}

int32 URMSLibrary::ApplyRootMotionSource_PathMoveToForce(UCharacterMovementComponent* MovementComponent,
                                                                      FName InstanceName, FVector StartLocation,
                                                                      TArray<FRMSPathMoveToData> Path,
                                                                      int32 Priority, float StartTime,
                                                                      ERMSApplyMode ApplyMode,
                                                                      FRMSSetting_Move ExtraSetting)
{
	if (!MovementComponent)
	{
		return -1;
	}
	if (Path.Num() < 0)
	{
		return -1;
	}
	float Duration = 0;
	for (auto P : Path)
	{
		if (P.Duration <= 0)
		{
			return -1;
		}
		Duration += P.Duration;
	}
	const int32 NewPriority = CalcPriorityByApplyMode(MovementComponent, InstanceName, Priority, ApplyMode);
	if (NewPriority < 0)
	{
		return -1;
	}

	TSharedPtr<FRootMotionSource_PathMoveToForce> PathMoveTo = MakeShared<
		FRootMotionSource_PathMoveToForce>();
	PathMoveTo->InstanceName = InstanceName == NAME_None ? TEXT("PathMoveTo") : InstanceName;
	PathMoveTo->AccumulateMode = ExtraSetting.AccumulateMod;
	PathMoveTo->Settings.SetFlag(
		static_cast<ERootMotionSourceSettingsFlags>(static_cast<uint8>(ExtraSetting.SourcesSetting)));
	PathMoveTo->Priority = NewPriority;
	PathMoveTo->Path = Path;
	PathMoveTo->StartLocation = StartLocation;
	PathMoveTo->Duration = FMath::Max(Duration, KINDA_SMALL_NUMBER);
	PathMoveTo->FinishVelocityParams.Mode = static_cast<ERootMotionFinishVelocityMode>(static_cast<uint8>(ExtraSetting.
		VelocityOnFinishMode));
	PathMoveTo->FinishVelocityParams.SetVelocity = ExtraSetting.FinishSetVelocity;
	PathMoveTo->FinishVelocityParams.ClampVelocity = ExtraSetting.FinishClampVelocity;
	PathMoveTo->SetTime(StartTime);
	return MovementComponent->ApplyRootMotionSource(PathMoveTo);
}

bool URMSLibrary::ApplyRootMotionSource_SimpleAnimation_BM(UCharacterMovementComponent* MovementComponent,
                                                                        UAnimSequence* DataAnimation,
                                                                        FName InstanceName,
                                                                        int32 Priority,
                                                                        float StartTime, float EndTime, float Rate,
                                                                        ERMSApplyMode ApplyMode)
{
	if (!MovementComponent || !DataAnimation || (StartTime > EndTime && EndTime > 0))
	{
		return false;
	}
	ACharacter* Character = Cast<ACharacter>(MovementComponent->GetOwner());
	USkeletalMeshComponent* Mesh = Character->GetMesh();
	if (!Character || !Mesh)
	{
		return false;
	}
	if (CalcPriorityByApplyMode(MovementComponent, InstanceName, Priority, ApplyMode) < 0)
	{
		return false;
	}
	float HalfHeight = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	//持续时间, 获取的是动画时长或者自定的时间
	const float Length = DataAnimation->GetPlayLength();
	if (EndTime <= 0)
	{
		EndTime = Length;
	}
	const float Duration = EndTime - StartTime;
	int32 NumFrame = DataAnimation->GetNumberOfSampledKeys();
	const float FrameTime = DataAnimation->GetPlayLength() / NumFrame;
	//开始位置
	FVector StartLocation = Character->GetActorLocation();
	//模型与角色的相对变换矩阵,我们只需要Rotation
	FTransform Mesh2Char = Mesh->GetComponentTransform().GetRelativeTransform(Character->GetActorTransform());
	Mesh2Char.SetLocation(FVector::Zero());
	FTransform RMT;

	//获取RootMotion最终值, 即根骨的最大偏移量
	RMT = DataAnimation->ExtractRootMotionFromRange(StartTime, EndTime);
	RMT = RMT * Mesh2Char;

	//用动态曲线的方式 , 而非静态,
	UCurveVector* OffsetCV = NewObject<UCurveVector>();
	FRichCurve CurveX, CurveY, CurveZ;
	/*
	 *遍历每一帧获取当前动画的RootMotion位置,
	 *减去线性当前帧的位置,得到了动画的曲线偏移值
	 *计算动画与期望位置的比率
	 *乘以之前的曲线偏移值得到最终的偏移值
	 */
	for (float CurrentTime = StartTime; CurrentTime <= EndTime; CurrentTime += FrameTime)
	{
		FTransform CurrFrameTM;

		float Fraction = (CurrentTime - StartTime) / Duration;
		//获取当前时间的rootMotion
		CurrFrameTM = DataAnimation->ExtractRootMotion(0, CurrentTime, false);
		CurrFrameTM = CurrFrameTM * Mesh2Char;

		FVector AnimRootMotionLinearFraction = RMT.GetLocation() * Fraction;
		//动画位置与线性偏移位置的偏差
		FVector CurveOffset = CurrFrameTM.GetLocation() - AnimRootMotionLinearFraction;

		CurveX.AddKey(CurrentTime / Duration / Rate, CurveOffset.X);
		CurveY.AddKey(CurrentTime / Duration / Rate, CurveOffset.Y);
		CurveZ.AddKey(CurrentTime / Duration / Rate, CurveOffset.Z);


		if (RMS::CVarRMS_Debug.GetValueOnGameThread() == 1)
		{
			//动画每一帧位置
			UKismetSystemLibrary::DrawDebugSphere(
				Mesh, UKismetMathLibrary::TransformLocation(Character->GetActorTransform(), CurrFrameTM.GetLocation()) -
				FVector(0, 0, HalfHeight), 5.0, 4, FColor::Yellow, 5.0);
			//动画线性每一帧位置
			UKismetSystemLibrary::DrawDebugSphere(
				Mesh, UKismetMathLibrary::TransformLocation(Character->GetActorTransform(),
				                                            AnimRootMotionLinearFraction) - FVector(0, 0, HalfHeight),
				5.0, 4, FColor::Red, 5.0);
		}
	}
	OffsetCV->FloatCurves[0] = CurveX;
	OffsetCV->FloatCurves[1] = CurveY;
	OffsetCV->FloatCurves[2] = CurveZ;

	FVector WorldTarget = UKismetMathLibrary::TransformLocation(Character->GetActorTransform(), RMT.GetLocation());

	FName InsName = InstanceName == NAME_None ? TEXT("SimpleAnimation") : InstanceName;
	return ApplyRootMotionSource_MoveToForce(MovementComponent, InsName, StartLocation, WorldTarget,
	                                         Duration / Rate, Priority, OffsetCV) >= 0;
}


bool URMSLibrary::ApplyRootMotionSource_AnimationAdjustment_BM(
	UCharacterMovementComponent* MovementComponent, UAnimSequence* DataAnimation,
	FName InstanceName, int32 Priority, FVector TargetLocation,
	bool bLocalTarget, bool bTargetBasedOnFoot, bool bUseCustomDuration, float CustomDuration, float AnimWarpingScale,
	ERMSAnimWarpingType WarpingType, ERMSAnimWarpingAxis WarpingAxis,
	ERMSApplyMode ApplyMode)
{
	if (!MovementComponent || !DataAnimation || (bUseCustomDuration && CustomDuration <= 0))
	{
		return false;
	}
	if (CalcPriorityByApplyMode(MovementComponent, InstanceName, Priority, ApplyMode) < 0)
	{
		return false;
	}
	ACharacter* Character = Cast<ACharacter>(MovementComponent->GetOwner());
	USkeletalMeshComponent* Mesh = Character->GetMesh();
	if (!Character || !Mesh)
	{
		return false;
	}
	float HalfHeight = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	//持续时间, 获取的是动画时长或者自定的时间
	float Duration = bUseCustomDuration ? CustomDuration : DataAnimation->GetPlayLength();
	float DurationRatio = Duration / DataAnimation->GetPlayLength();
	int32 NumFrame = DataAnimation->GetNumberOfSampledKeys();
	const float FrameTime = DataAnimation->GetPlayLength() / NumFrame;
	//获取本地偏移
	FVector LocationOffset = FVector::ZeroVector;
	if (bLocalTarget)
	{
		LocationOffset = TargetLocation + (bTargetBasedOnFoot ? FVector(0, 0, HalfHeight) : FVector::ZeroVector);
	}
	else
	{
		LocationOffset = UKismetMathLibrary::InverseTransformLocation(Character->GetActorTransform(), TargetLocation) +
			(bTargetBasedOnFoot ? FVector(0, 0, HalfHeight) : FVector::ZeroVector);
	}
	//开始位置
	FVector StartLocation = Character->GetActorLocation();
	//模型与角色的相对变换矩阵,我们只需要Rotation
	FTransform Mesh2Char = Mesh->GetComponentTransform().GetRelativeTransform(Character->GetActorTransform());
	Mesh2Char.SetLocation(FVector::Zero());

	//获取RootMotion最终值, 即根骨的最大偏移量
	FTransform RMT = DataAnimation->ExtractRootMotionFromRange(0, Duration);
	RMT = RMT * Mesh2Char;
	//获取所需的最后一帧，不一定要遍历所有
	int32 LastFrame = bUseCustomDuration ? FMath::CeilToInt(NumFrame * DurationRatio) : NumFrame;

	//用动态曲线的方式 , 而非静态,
	UCurveVector* OffsetCV = NewObject<UCurveVector>();
	FRichCurve CurveX, CurveY, CurveZ;
	float LastWindowEndTime = 0;
	FVector WorldOffset = LocationOffset.X * Character->GetActorForwardVector() + LocationOffset.Y * Character->
		GetActorRightVector() + LocationOffset.Z * Character->GetActorUpVector();
	FVector WorldTarget = StartLocation + WorldOffset;
	/*
	 *遍历每一帧获取当前动画的RootMotion位置,
	 *减去线性当前帧的位置,得到了动画的曲线偏移值
	 *计算动画与期望位置的比率
	 *乘以之前的曲线偏移值得到最终的偏移值
	 */
	for (float CurrentTime = 0; CurrentTime <= Duration; CurrentTime += FrameTime)
	{
		FTransform CurrFrameTM;

		float Fraction = CurrentTime / Duration;
		float LastFraction = LastWindowEndTime / Duration;
		//获取当前时间的rootMotion
		CurrFrameTM = DataAnimation->ExtractRootMotion(0, CurrentTime, false);

		CurrFrameTM = CurrFrameTM * Mesh2Char;
		FVector LocalLinearOffsetFraction = LocationOffset * Fraction;
		FVector AnimRootMotionLinearFraction = RMT.GetLocation() * Fraction;
		//动画位置与线性偏移位置的偏差
		FVector CurveOffset = CurrFrameTM.GetLocation() - AnimRootMotionLinearFraction;
		CalcAnimWarpingScale(CurveOffset, WarpingType, AnimRootMotionLinearFraction, LocalLinearOffsetFraction,
		                     AnimWarpingScale);
		ConvWorldOffsetToRmsSpace(CurveOffset, StartLocation, WorldTarget);
		FiltAnimCurveOffsetAxisData(CurveOffset, WarpingAxis);

		CurveX.AddKey(CurrentTime / Duration, CurveOffset.X);
		CurveY.AddKey(CurrentTime / Duration, CurveOffset.Y);
		CurveZ.AddKey(CurrentTime / Duration, CurveOffset.Z);


		if (RMS::CVarRMS_Debug.GetValueOnGameThread() == 1)
		{
			//动画每一帧位置
			UKismetSystemLibrary::DrawDebugSphere(
				Mesh, UKismetMathLibrary::TransformLocation(Character->GetActorTransform(), CurrFrameTM.GetLocation()) -
				FVector(0, 0, HalfHeight), 5.0, 4, FColor::Yellow, 5.0);
			//动画线性每一帧位置
			UKismetSystemLibrary::DrawDebugSphere(
				Mesh, UKismetMathLibrary::TransformLocation(Character->GetActorTransform(),
				                                            AnimRootMotionLinearFraction) - FVector(0, 0, HalfHeight),
				5.0, 4, FColor::Red, 5.0);
			//期望每一帧位置
			UKismetSystemLibrary::DrawDebugSphere(
				Mesh, UKismetMathLibrary::TransformLocation(Character->GetActorTransform(), LocalLinearOffsetFraction) -
				FVector(0, 0, HalfHeight), 5.0, 4, FColor::Green, 5.0);
			//矫正后的每一帧位置
			UKismetSystemLibrary::DrawDebugSphere(
				Mesh, UKismetMathLibrary::TransformLocation(Character->GetActorTransform(),
				                                            LocalLinearOffsetFraction + CurveOffset) - FVector(
					0, 0, HalfHeight), 5.0, 4, FColor::Blue, 5.0);
		}
		LastWindowEndTime = CurrentTime;
	}
	OffsetCV->FloatCurves[0] = CurveX;
	OffsetCV->FloatCurves[1] = CurveY;
	OffsetCV->FloatCurves[2] = CurveZ;
	FName InsName = InstanceName == NAME_None ? TEXT("AnimationAdjustment") : InstanceName;
	return ApplyRootMotionSource_MoveToForce(MovementComponent, InsName, StartLocation, WorldTarget, Duration,
	                                         Priority, OffsetCV) >= 0;
}

bool URMSLibrary::ApplyRootMotionSource_AnimationAdjustment(UCharacterMovementComponent* MovementComponent,
                                                                         UAnimSequence* DataAnimation,
                                                                         FName InstanceName,
                                                                         int32 Priority,
                                                                         FVector TargetLocation,
                                                                         bool bLocalTarget,
                                                                         bool bTargetBasedOnFoot,
                                                                         float StartTime,
                                                                         float EndTime,
                                                                         float Rate,
                                                                         ERMSApplyMode ApplyMode)
{
	if (!MovementComponent || !DataAnimation)
	{
		return false;
	}
	ACharacter* Character = Cast<ACharacter>(MovementComponent->GetOwner());
	USkeletalMeshComponent* Mesh = Character->GetMesh();
	if (!Character || !Mesh)
	{
		return false;
	}
	const int32 NewPriority = CalcPriorityByApplyMode(MovementComponent, InstanceName, Priority, ApplyMode);
	if (NewPriority < 0)
	{
		return false;
	}
	const float CurrEndTime = (EndTime < 0 || EndTime > DataAnimation->GetPlayLength())
		                          ? DataAnimation->GetPlayLength()
		                          : EndTime;
	const float Duration = (CurrEndTime - StartTime) / FMath::Max(Rate, 0.1);
	const float HalfHeight = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const FTransform FootTransform = FTransform(Character->GetActorQuat(),
	                                            Character->GetActorLocation() - FVector(0, 0, HalfHeight));
	const FTransform CharacterTransform = Character->GetActorTransform();
	//获取本地偏移
	FVector WorldTarget = FVector::ZeroVector;
	if (bLocalTarget)
	{
		if (bTargetBasedOnFoot)
		{
			WorldTarget = FootTransform.TransformPosition(TargetLocation);
		}
		else
		{
			WorldTarget = CharacterTransform.TransformPosition(TargetLocation);
		}
	}
	else
	{
		WorldTarget = TargetLocation - (bTargetBasedOnFoot ? FVector::ZeroVector : FVector(0, 0, HalfHeight));
	}
	TSharedPtr<FRootMotionSource_AnimWarping_FinalPoint> RMS = MakeShared<FRootMotionSource_AnimWarping_FinalPoint>();
	RMS->InstanceName = InstanceName == NAME_None ? TEXT("MotioWarping") : InstanceName;

	RMS->AccumulateMode = ERootMotionAccumulateMode::Override;
	RMS->Priority = NewPriority;
	//这个目标必须是脚底位置
	RMS->TargetLocation = WorldTarget;
	RMS->StartLocation = CharacterTransform.GetLocation();
	RMS->StartRotation = CharacterTransform.GetRotation().Rotator();
	RMS->Duration = Duration;
	RMS->bIgnoreZAxis = false;
	RMS->Animation = DataAnimation;
	RMS->AnimStartTime = StartTime;
	RMS->AnimEndTime = EndTime;
	RMS->SetTime(StartTime);
	return MovementComponent->ApplyRootMotionSource(RMS) > -1;
}

bool URMSLibrary::ApplyRootMotionSource_AnimationAdjustmentByFrame_BM(
	UCharacterMovementComponent* MovementComponent, UAnimSequence* DataAnimation,
	FName InstanceName, int32 Priority, FVector TargetLocation,
	bool bLocalTarget, bool bTargetBasedOnFoot, int32 FromFrame, int32 TargetFram, float TimeScale,
	float AnimWarpingScale,
	ERMSAnimWarpingType WarpingType, ERMSAnimWarpingAxis WarpingAxis,
	ERMSApplyMode ApplyMode)
{
	if (!MovementComponent || !DataAnimation || FromFrame < 0 || (TargetFram > 0 && FromFrame > TargetFram))
	{
		return false;
	}
	ACharacter* Character = Cast<ACharacter>(MovementComponent->GetOwner());
	USkeletalMeshComponent* Mesh = Character->GetMesh();
	if (!Character || !Mesh)
	{
		return false;
	}
	if (CalcPriorityByApplyMode(MovementComponent, InstanceName, Priority, ApplyMode) < 0)
	{
		return false;
	}
	const int32 NumFrame = DataAnimation->GetNumberOfSampledKeys();
	const float FrameTime = DataAnimation->GetPlayLength() / NumFrame;
	if (FromFrame == 0 && (TargetFram <= 0 || TargetFram == NumFrame))
	{
		return ApplyRootMotionSource_AnimationAdjustment_BM(MovementComponent, DataAnimation, InstanceName, Priority,
		                                                    TargetLocation, bLocalTarget, bTargetBasedOnFoot, true,
		                                                    DataAnimation->GetPlayLength() * TimeScale,
		                                                    AnimWarpingScale,
		                                                    WarpingType, WarpingAxis);
	}
	else
	{
		float HalfHeight = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

		//动画时间数据
		const float AnimLength = DataAnimation->GetPlayLength();
		const float StartTime = FromFrame * FrameTime;
		const float EndTime = TargetFram * FrameTime;
		return ApplyRootMotionSource_AnimationAdjustmentByTime_BM(MovementComponent, DataAnimation, InstanceName,
		                                                          Priority, TargetLocation, bLocalTarget,
		                                                          bTargetBasedOnFoot, StartTime,
		                                                          EndTime, TimeScale, AnimWarpingScale, WarpingType,
		                                                          WarpingAxis);
	}
}

bool URMSLibrary::ApplyRootMotionSource_AnimationAdjustmentByTime_BM(
	UCharacterMovementComponent* MovementComponent, UAnimSequence* DataAnimation,
	FName InstanceName, int32 Priority, FVector TargetLocation,
	bool bLocalTarget, bool bTargetBasedOnFoot, float FromTime, int32 TargetTime, float TimeScale,
	float AnimWarpingScale,
	ERMSAnimWarpingType WarpingType, ERMSAnimWarpingAxis WarpingAxis,
	ERMSApplyMode ApplyMode)
{
	if (!MovementComponent || !DataAnimation || FromTime < 0 || (TargetTime > 0 && FromTime > TargetTime))
	{
		return false;
	}
	ACharacter* Character = Cast<ACharacter>(MovementComponent->GetOwner());
	USkeletalMeshComponent* Mesh = Character->GetMesh();
	if (!Character || !Mesh)
	{
		return false;
	}
	if (CalcPriorityByApplyMode(MovementComponent, InstanceName, Priority, ApplyMode) < 0)
	{
		return false;
	}
	const int32 NumFrame = DataAnimation->GetNumberOfSampledKeys();
	const float FrameTime = DataAnimation->GetPlayLength() / NumFrame;
	const float AnimLength = DataAnimation->GetPlayLength();
	const float Duration = TargetTime - FromTime;
	if (FromTime == 0 && (TargetTime <= 0 || TargetTime == AnimLength))
	{
		return ApplyRootMotionSource_AnimationAdjustment_BM(MovementComponent, DataAnimation, InstanceName, Priority,
		                                                    TargetLocation, bLocalTarget, bTargetBasedOnFoot, true,
		                                                    DataAnimation->GetPlayLength() * TimeScale,
		                                                    AnimWarpingScale,
		                                                    WarpingType, WarpingAxis);
	}
	else
	{
		float HalfHeight = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();


		//获取本地偏移
		FVector LocationOffset = FVector::ZeroVector;
		if (bLocalTarget)
		{
			LocationOffset = TargetLocation + (bTargetBasedOnFoot ? FVector(0, 0, HalfHeight) : FVector::ZeroVector);
		}
		else
		{
			LocationOffset =
				UKismetMathLibrary::InverseTransformLocation(Character->GetActorTransform(), TargetLocation) + (
					bTargetBasedOnFoot ? FVector(0, 0, HalfHeight) : FVector::ZeroVector);
		}


		//开始位置
		FVector StartLocation = Character->GetActorLocation();
		//模型与角色的相对变换矩阵,我们只需要Rotation
		FTransform Mesh2Char = Mesh->GetComponentTransform().GetRelativeTransform(Character->GetActorTransform());
		Mesh2Char.SetLocation(FVector::Zero());

		if (!DataAnimation->GetSkeleton())
		{
			return false;
		}
		//获取RootMotion数据
		FTransform RMT = DataAnimation->ExtractRootMotionFromRange(FromTime, TargetTime);
		RMT = RMT * Mesh2Char;
		FVector WorldOffset = LocationOffset.X * Character->GetActorForwardVector() + LocationOffset.Y * Character->
			GetActorRightVector() + LocationOffset.Z * Character->GetActorUpVector();
		FVector WorldTarget = StartLocation + WorldOffset;
		//用动态曲线的方式 , 而非静态,
		UCurveVector* OffsetCV = NewObject<UCurveVector>();
		FRichCurve CurveX, CurveY, CurveZ;
		/*
		  *遍历每一帧获取当前动画的RootMotion位置,
		  *减去线性当前帧的位置,得到了动画的曲线偏移值
		  *计算动画与期望位置的比率
		  *乘以之前的曲线偏移值得到最终的偏移值
		  */
		for (float CurrentTime = 0; CurrentTime <= Duration; CurrentTime += FrameTime)
		{
			FTransform CurrFrameTM;

			float Fraction = CurrentTime / AnimLength;
			//获取当前时间的rootMotion
			CurrFrameTM = DataAnimation->ExtractRootMotion(0, CurrentTime, false);

			CurrFrameTM = CurrFrameTM * Mesh2Char;
			FVector LocalLinearOffsetFraction = LocationOffset * Fraction;
			FVector AnimRootMotionLinearFraction = RMT.GetLocation() * Fraction;
			//动画位置与线性偏移位置的偏差
			FVector CurveOffset = CurrFrameTM.GetLocation() - AnimRootMotionLinearFraction;
			CalcAnimWarpingScale(CurveOffset, WarpingType, AnimRootMotionLinearFraction, LocalLinearOffsetFraction,
			                     AnimWarpingScale);
			ConvWorldOffsetToRmsSpace(CurveOffset, StartLocation, WorldTarget);
			FiltAnimCurveOffsetAxisData(CurveOffset, WarpingAxis);

			CurveX.AddKey(CurrentTime / Duration * TimeScale, CurveOffset.X);
			CurveY.AddKey(CurrentTime / Duration * TimeScale, CurveOffset.Y);
			CurveZ.AddKey(CurrentTime / Duration * TimeScale, CurveOffset.Z);


			if (RMS::CVarRMS_Debug.GetValueOnGameThread() == 1)
			{
				//动画每一帧位置
				UKismetSystemLibrary::DrawDebugSphere(
					Mesh, UKismetMathLibrary::TransformLocation(Character->GetActorTransform(),
					                                            CurrFrameTM.GetLocation()) - FVector(0, 0, HalfHeight),
					5.0, 4, FColor::Yellow, 5.0);
				//动画线性每一帧位置
				UKismetSystemLibrary::DrawDebugSphere(
					Mesh, UKismetMathLibrary::TransformLocation(Character->GetActorTransform(),
					                                            AnimRootMotionLinearFraction) - FVector(
						0, 0, HalfHeight), 5.0, 4, FColor::Red, 5.0);
				//期望每一帧位置
				UKismetSystemLibrary::DrawDebugSphere(
					Mesh, UKismetMathLibrary::TransformLocation(Character->GetActorTransform(),
					                                            LocalLinearOffsetFraction) - FVector(0, 0, HalfHeight),
					5.0, 4, FColor::Green, 5.0);
				//矫正后的每一帧位置
				UKismetSystemLibrary::DrawDebugSphere(
					Mesh, UKismetMathLibrary::TransformLocation(Character->GetActorTransform(),
					                                            LocalLinearOffsetFraction + CurveOffset) - FVector(
						0, 0, HalfHeight), 5.0, 4, FColor::Blue, 5.0);
			}
		}
		OffsetCV->FloatCurves[0] = CurveX;
		OffsetCV->FloatCurves[1] = CurveY;
		OffsetCV->FloatCurves[2] = CurveZ;

		FName InsName = InstanceName == NAME_None ? TEXT("AnimationAdjustment") : InstanceName;
		//用MoveToForce来计算路径, 尝试过用Jump+OffsetCv, 但是Jump的CV不好用
		return ApplyRootMotionSource_MoveToForce(MovementComponent, InsName, StartLocation, WorldTarget, Duration,
		                                         Priority, OffsetCV) >= 0;
	}
}

bool URMSLibrary::ApplyRootMotionSource_AnimationWarping_BM(
	UCharacterMovementComponent* MovementComponent, UAnimSequence* DataAnimation,
	TMap<FName, FVector> WarpingTarget, FName InstanceName,
	int32 Priority, bool bTargetBasedOnFoot, float Rate,
	float Tolerance, float AnimWarpingMulti, bool bExcludeEndAnimMotion, ERMSAnimWarpingAxis WarpingAxis,
	ERMSApplyMode ApplyMode)
{
	if (!MovementComponent || !DataAnimation || WarpingTarget.Num() == 0 || Rate <= 0)
	{
		return false;
	}
	const int32 NewPriority = CalcPriorityByApplyMode(MovementComponent, InstanceName, Priority, ApplyMode);
	if (NewPriority < 0)
	{
		return false;
	}
	ACharacter* Character = Cast<ACharacter>(MovementComponent->GetOwner());
	USkeletalMeshComponent* Mesh = Character->GetMesh();
	if (!Character || !Mesh)
	{
		return false;
	}
	//动画基本数据
	const int32 NumFrame = DataAnimation->GetNumberOfSampledKeys();
	const float FrameTime = DataAnimation->GetPlayLength() / NumFrame;
	float AnimLength = DataAnimation->GetPlayLength();
	float Duration = AnimLength / Rate;
	float HalfHeight = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const FVector HalfHeightVec = FVector(0, 0, HalfHeight);
	const auto Notifies = DataAnimation->Notifies;
	TArray<FName> Instances;
	WarpingTarget.GetKeys(Instances);
	TArray<FRMSWindowData> Windows;
	if (!GetRootMotionSourceWindowsByInstanceList(DataAnimation, Instances, Windows))
	{
		return false;
	}
	TArray<FRMSNotifyTriggerData> TriggerDatas;
	float Time = 0;
	int32 idx = 0;
	while (Time <= AnimLength)
	{
		FRMSNotifyTriggerData TriggerData;
		auto EmplaceTriggerData_Lambda = [&]()
		{
			TriggerData.WindowData = Windows[idx];
			const auto Target = WarpingTarget.Find(Windows[idx].AnimNotify->RootMotionSourceTarget);
			if (Target)
			{
				TriggerData.Target = *Target;
				if (bTargetBasedOnFoot)
				{
					TriggerData.Target += HalfHeightVec;
				}
				TriggerData.bHasTarget = true;
			}
			TriggerDatas.Emplace(TriggerData);
		};
		//已经到最后一个通知, 但是通知的结尾不是动画结束点, 就添加一个默认数据;
		//这里先添加了最后一个无通知动画数据, 后续再处理
		if (idx > Windows.Num() - 1 && AnimLength - Windows[Windows.Num() - 1].EndTime > Tolerance)
		{
			TriggerData.bHasTarget = false;
			TriggerData.WindowData.StartTime = Windows[Windows.Num() - 1].EndTime;
			TriggerData.WindowData.EndTime = AnimLength;
			TriggerDatas.Emplace(TriggerData);
			break;
		}
		else if (idx > Windows.Num() - 1)
		{
			break;
		}
		//小于公差部分忽略, 都作为起始点
		if (Windows[idx].StartTime <= Tolerance)
		{
			EmplaceTriggerData_Lambda();
			Time = Windows[idx].EndTime;
			idx++;
			TriggerData.Reset();
		}
		else
		{
			//起始时间不是上一次的结束时间, 说明中间有空隙, 需要填充一个默认数据
			if (Windows[idx].StartTime - Time > Tolerance)
			{
				TriggerData.bHasTarget = false;
				TriggerData.WindowData.StartTime = Time;
				TriggerData.WindowData.EndTime = Windows[idx].StartTime;
				TriggerDatas.Emplace(TriggerData);
				TriggerData.Reset();
				EmplaceTriggerData_Lambda();
				Time = Windows[idx].EndTime;
				idx++;
			}
			//否则即使后一个窗口的起点早于前一个窗口的结束 也从结束点开始
			else
			{
				TriggerData.WindowData = Windows[idx];
				TriggerData.WindowData.StartTime = Windows[idx - 1].EndTime;
				const auto Target = WarpingTarget.Find(Windows[idx].AnimNotify->RootMotionSourceTarget);
				if (Target)
				{
					TriggerData.Target = *Target;
					TriggerData.bHasTarget = true;
				}
				TriggerDatas.Emplace(TriggerData);
				Time = Windows[idx].EndTime;
				idx++;
			}
			TriggerData.Reset();
		}
	}

	if (TriggerDatas.Num() == 0)
	{
		return false;
	}


	//用动态曲线的方式 , 而非静态,
	UCurveVector* OffsetCV = NewObject<UCurveVector>();
	FRichCurve CurveX, CurveY, CurveZ;
	//开始位置
	FVector StartLocation = Character->GetActorLocation();
	//模型与角色的相对变换矩阵,我们只需要Rotation
	FTransform Mesh2Char = Mesh->GetComponentTransform().GetRelativeTransform(Character->GetActorTransform());
	Mesh2Char.SetLocation(FVector::Zero());

	FVector LocalOffset = FVector::ZeroVector;
	//******************
	FVector LastWarpingTarget = FVector::ZeroVector;

	float LastWindowEndTime = 0;
	float LastTime = 0;
	FTransform FinalTargetAnimRM, CurrFrameAnimRM, CurrTargetAnimRM, LastTargetAnimRM;

	FVector LastTargetWS = StartLocation;
	FVector CurrTargetWS = FVector::ZeroVector;

	//**************************
	FRMSNotifyTriggerData TrigData;
	//确定最后目标
	FVector WorldTarget = FVector::ZeroVector;
	if (TriggerDatas[TriggerDatas.Num() - 1].bHasTarget)
	{
		WorldTarget = TriggerDatas[TriggerDatas.Num() - 1].Target;
	}
	else
	{
		int32 lastTargetIdx = 0;
		//判断窗口是否有大于1个, 如果大于1, 那么反向查找到最后一个
		if (TriggerDatas.Num() > 1)
		{
			for (int32 i = TriggerDatas.Num() - 1; i >= 0; i--)
			{
				if (TriggerDatas[i].bHasTarget)
				{
					LastWarpingTarget = TriggerDatas[i].Target;
					lastTargetIdx = i;
					break;
				}
			}
		}
		if (LastWarpingTarget == FVector::ZeroVector)
		{
			UE_LOG(LogTemp, Warning,
			       TEXT(
				       "Can not find last warping target,  maybe give wrong [WarpingTarget] or no RootMotionSource AnimNotifies"
			       ));
		}
		//如果不排除末尾的动画位移, 那么需要把最后一个动画通知窗口以后的RootMotion都加进来
		if (!bExcludeEndAnimMotion)
		{
			auto TM = DataAnimation->ExtractRootMotion(TriggerDatas[lastTargetIdx].WindowData.EndTime, AnimLength,
			                                           false);
			TM = TM * Mesh2Char;
			auto ActorRM = TM.GetLocation();
			FVector WorldRM = ActorRM.X * Character->GetActorForwardVector() + ActorRM.Y * Character->
				GetActorRightVector()
				+ ActorRM.Z * Character->GetActorUpVector();
			WorldTarget = LastWarpingTarget + WorldRM;
		}
		else
		{
			WorldTarget = LastWarpingTarget;
			//todo 这里处理如果排除最后一个动画的情况, Duration和AnimLength需要重新设置
			Duration = TriggerDatas[lastTargetIdx].WindowData.EndTime / Rate;
			AnimLength = TriggerDatas[lastTargetIdx].WindowData.EndTime;
		}
	}

	//获取整体RootMotion数据
	FTransform TotalAnimRM = DataAnimation->ExtractRootMotionFromRange(0, AnimLength);
	TotalAnimRM = TotalAnimRM * Mesh2Char;


	if (RMS::CVarRMS_Debug.GetValueOnGameThread() == 1)
	{
		//绘制最后目标
		UKismetSystemLibrary::DrawDebugSphere(
			Mesh, WorldTarget, 20.f, 12, FColor::Purple, 5, 1);
	}

	//最终目标的RootMotion
	FinalTargetAnimRM = DataAnimation->ExtractRootMotion(0, AnimLength, false);
	FinalTargetAnimRM = FinalTargetAnimRM * Mesh2Char;

	FVector CurveOffset = FVector::ZeroVector;
	//逐帧遍历
	for (float CurrentTime = 0; CurrentTime <= AnimLength; CurrentTime += FrameTime)
	{
		//todo 需要区分缩放时间和真实的时间, 真实时间用于提取动画RM数据
		const float TimeScaled = CurrentTime / Rate;
		bool bNeedUpdateTarget = false;
		//如果当前时间已经大于上一次数据的最后时间, 说明换了一个窗口期, 记录上一次的时间和目标
		if (TimeScaled > TrigData.WindowData.EndTime / Rate)
		{
			LastWindowEndTime = LastTime;
			LastTargetWS = CurrTargetWS;
			bNeedUpdateTarget = true;
			LastTargetAnimRM = CurrTargetAnimRM;
		}
		//查询窗口数据需要用原始时间
		if (!FindTriggerDataByTime(TriggerDatas, CurrentTime, TrigData))
		{
			continue;
		}
		if (TrigData == TriggerDatas.Last() && TrigData.bHasTarget == false && bExcludeEndAnimMotion)
		{
			break;
		}

		//当前分段内的百分比
		float WindowFraction = (TimeScaled - TrigData.WindowData.StartTime / Rate) / (TrigData.WindowData.EndTime / Rate
			- TrigData.
			  WindowData.StartTime / Rate);
		//整体百分比, 都是缩放过的数据
		float Fraction = TimeScaled / Duration;

		//获取当前时间段的rootMotion
		CurrFrameAnimRM = DataAnimation->ExtractRootMotion(0, CurrentTime, false);
		CurrFrameAnimRM = CurrFrameAnimRM * Mesh2Char;

		bool bHasWarpingTarget = false;

		//******************************************
		//查找当前窗口的目标数据, 只在窗口改变以后的时候刷新
		if (bNeedUpdateTarget || CurrentTime == 0)
		{
			if (TrigData.bHasTarget && TrigData.WindowData.AnimNotify)
			{
				const auto target = WarpingTarget.Find(TrigData.WindowData.AnimNotify->RootMotionSourceTarget);
				if (target)
				{
					CurrTargetWS = *target;
					if (bTargetBasedOnFoot)
					{
						CurrTargetWS += HalfHeightVec;
					}
					LocalOffset = UKismetMathLibrary::InverseTransformLocation(
						Character->GetActorTransform(), CurrTargetWS);
					bHasWarpingTarget = true;
				}
				//找不到直接返回失败, 必须匹配
				else
				{
					UE_LOG(LogTemp, Warning, TEXT(" wrong [WarpingTarget], need matching animation notifies "));
					return false;
				}
			}
			//否则就使用动画本身的位移
			if (!bHasWarpingTarget)
			{
				const auto T = DataAnimation->ExtractRootMotion(LastWindowEndTime, TrigData.WindowData.EndTime, false);
				LocalOffset = (T * Mesh2Char).GetLocation();
				FVector WorldOffset = LocalOffset.X * Character->GetActorForwardVector() + LocalOffset.Y * Character->
					GetActorRightVector() + LocalOffset.Z * Character->GetActorUpVector();
				CurrTargetWS = LastTargetWS + WorldOffset;
			}
			CurrTargetAnimRM = DataAnimation->ExtractRootMotion(0, TrigData.WindowData.EndTime, false);
			CurrTargetAnimRM = CurrTargetAnimRM * Mesh2Char;
		}
		//******************************************


		//*****************计算关键数据*************************
		//当前窗口的线性位移
		FVector WindowLinearOffset = (CurrTargetWS - LastTargetWS) * WindowFraction;
		//全局线性偏移
		FVector FinalLinearOffset = (WorldTarget - StartLocation) * Fraction;
		//todo 这里有个坑, 曲线信息是start到target的朝向空间的, 即X的方向是target-start的向量朝向; 所以需要转换成相对空间
		FVector Offset_LinearBase = LastTargetWS + WindowLinearOffset - (StartLocation + FinalLinearOffset);
		ConvWorldOffsetToRmsSpace(Offset_LinearBase, StartLocation, WorldTarget);

		FVector WindowAnimLinearOffset = (CurrTargetAnimRM.GetLocation() - LastTargetAnimRM.GetLocation()) *
			WindowFraction;
		FVector Offset_Anim = CurrFrameAnimRM.GetLocation() - (LastTargetAnimRM.GetLocation() + WindowAnimLinearOffset);
		ConvWorldOffsetToRmsSpace(Offset_Anim, StartLocation, WorldTarget);

		//计算目标线性位移与动画RM线性位移的比值
		const float WarpRatio = FMath::IsNearlyZero(WindowAnimLinearOffset.Size(), Tolerance)
			                        ? 0
			                        : WindowLinearOffset.Size() / WindowAnimLinearOffset.Size();
		Offset_Anim *= WarpRatio * AnimWarpingMulti;
		FiltAnimCurveOffsetAxisData(Offset_Anim, WarpingAxis);
		CurveOffset = Offset_LinearBase + Offset_Anim;


		CurveX.AddKey(TimeScaled / Duration, CurveOffset.X);
		CurveY.AddKey(TimeScaled / Duration, CurveOffset.Y);
		CurveZ.AddKey(TimeScaled / Duration, CurveOffset.Z);


		if (RMS::CVarRMS_Debug.GetValueOnGameThread() == 1)
		{
			//动画每一帧位置
			UKismetSystemLibrary::DrawDebugSphere(
				Mesh, UKismetMathLibrary::TransformLocation(Character->GetActorTransform(),
				                                            CurrFrameAnimRM.GetLocation()) - FVector(0, 0, HalfHeight),
				5.0, 4, FColor::Yellow, 5.0);
			//当前窗口动画线性位置
			UKismetSystemLibrary::DrawDebugSphere(
				Mesh, UKismetMathLibrary::TransformLocation(Character->GetActorTransform(),
				                                            WindowAnimLinearOffset + LastTargetAnimRM.GetLocation()) -
				FVector(
					0, 0, HalfHeight), 5.0, 4, FColor::Red, 5.0);
			//最终目标的线性位置
			UKismetSystemLibrary::DrawDebugSphere(
				Mesh, StartLocation + FinalLinearOffset -
				FVector(0, 0, HalfHeight), 5.0, 4, FColor::Green, 5.0);
			FRotator FacingRot = (WorldTarget - StartLocation).Rotation();
			FacingRot.Pitch = 0;
			FVector Fwd = UKismetMathLibrary::GetForwardVector(FacingRot);
			FVector Rt = UKismetMathLibrary::GetRightVector(FacingRot);
			FVector Up = UKismetMathLibrary::GetUpVector(FacingRot);
			FVector Offset = Fwd * CurveOffset.X + Rt * CurveOffset.Y + Up * CurveOffset.Z;

			//当前窗口的线性位置
			UKismetSystemLibrary::DrawDebugSphere(
				Mesh, StartLocation + FinalLinearOffset + Offset - FVector(0, 0, HalfHeight),
				5.0, 4, FColor::Blue, 5.0);
		}
		LastTime = CurrentTime;
	}
	OffsetCV->FloatCurves[0] = CurveX;
	OffsetCV->FloatCurves[1] = CurveY;
	OffsetCV->FloatCurves[2] = CurveZ;

	InstanceName = InstanceName == NAME_None ? TEXT("AnimWarping") : InstanceName;
	//用MoveToForce来计算路径, 尝试过用Jump+OffsetCv, 但是Jump的CV不好用
	return ApplyRootMotionSource_MoveToForce(MovementComponent, InstanceName, StartLocation, WorldTarget, Duration,
	                                         Priority,
	                                         OffsetCV) >= 0;
}

bool URMSLibrary::ApplyRootMotionSource_AnimationWarping(UCharacterMovementComponent* MovementComponent,
                                                                      UAnimSequence* DataAnimation,
                                                                      TMap<FName, FVector> WarpingTarget,
                                                                      float StartTime,
                                                                      FName InstanceName,
                                                                      int32 Priority,
                                                                      bool bTargetBasedOnFoot,
                                                                      float Rate,
                                                                      float Tolerance,
                                                                      ERMSApplyMode ApplyMode)
{
	auto EmplaceTriggerDataTargetWithAnimRootMotion_Lambda = [&](ACharacter* Character,
	                                                             FRMSTarget& Data,
	                                                             FVector StartLocation, FRotator StartRotation)
	{
		auto RM = ExtractRootMotion(DataAnimation, Data.StartTime, Data.EndTime);
		const FTransform CurrentTransform = FTransform(Character->GetActorQuat(),
		                                               StartLocation - FVector(
			                                               0.f, 0.f,
			                                               Character->GetCapsuleComponent()->
			                                                          GetScaledCapsuleHalfHeight()));
		const FTransform StartChacterFootTransform = FTransform(StartRotation,
		                                                        StartLocation - FVector(
			                                                        0.f, 0.f,
			                                                        Character->GetCapsuleComponent()->
			                                                                   GetScaledCapsuleHalfHeight()));
		const FTransform CurrChacterFootTransform = FTransform(StartRotation,
		                                                       Character->GetActorLocation() - FVector(
			                                                       0.f, 0.f,
			                                                       Character->GetCapsuleComponent()->
			                                                                  GetScaledCapsuleHalfHeight()));
		FTransform MeshTransformWS = Character->GetMesh()->GetComponentTransform();
		FTransform Mesh2CharInverse = StartChacterFootTransform.GetRelativeTransform(MeshTransformWS);

		RM = RM * MeshTransformWS; //模型世界空间的RM
		//通过逆矩阵把模型空间转换成actor空间
		const FTransform TargetTransformWS = Mesh2CharInverse * RM;
		Data.Target = TargetTransformWS.GetLocation();
	};

	if (!MovementComponent || !DataAnimation || WarpingTarget.Num() == 0 || Rate <= 0 || StartTime > DataAnimation->
		GetPlayLength())
	{
		return false;
	}
	const int32 NewPriority = CalcPriorityByApplyMode(MovementComponent, InstanceName, Priority, ApplyMode);
	if (NewPriority < 0)
	{
		return false;
	}
	ACharacter* Character = Cast<ACharacter>(MovementComponent->GetOwner());
	USkeletalMeshComponent* Mesh = Character->GetMesh();
	if (!Character || !Mesh)
	{
		return false;
	}
	//动画基本数据
	const int32 NumFrame = DataAnimation->GetNumberOfSampledKeys();
	const float FrameTime = DataAnimation->GetPlayLength() / NumFrame;
	float AnimLength = DataAnimation->GetPlayLength();
	float Duration = AnimLength / Rate;
	float HalfHeight = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const FVector HalfHeightVec = FVector(0, 0, HalfHeight);
	const auto Notifies = DataAnimation->Notifies;
	TArray<FName> TargetNames;
	WarpingTarget.GetKeys(TargetNames);
	TArray<FRMSWindowData> Windows;
	if (!GetRootMotionSourceWindowsByInstanceList(DataAnimation, TargetNames, Windows))
	{
		return false;
	}
	TArray<FRMSTarget> TriggerDatas;
	float Time = StartTime;
	int32 idx = 0;
	while (Time <= AnimLength)
	{
		FRMSTarget TriggerData;
		//已经到最后一个通知, 但是通知的结尾不是动画结束点, 就添加一个默认数据;
		if (idx > Windows.Num() - 1 && AnimLength - Windows[Windows.Num() - 1].EndTime > Tolerance)
		{
			TriggerData.StartTime = Windows[Windows.Num() - 1].EndTime;
			TriggerData.EndTime = AnimLength;
			const auto Target = WarpingTarget.Find(Windows[Windows.Num() - 1].AnimNotify->RootMotionSourceTarget);
			if (Target)
			{
				EmplaceTriggerDataTargetWithAnimRootMotion_Lambda(Character, TriggerData, *Target,
				                                                  Character->GetActorRotation());
				break;
			}
			else
			{
				break;
			}
		}
		else if (idx > Windows.Num() - 1)
		{
			break;
		}
		//小于公差部分忽略, 都作为起始点
		if (idx == 0 && Windows[0].StartTime <= Tolerance)
		{
			TriggerData.StartTime = 0;
			TriggerData.EndTime = Windows[0].EndTime;
			const auto Target = WarpingTarget.Find(Windows[idx].AnimNotify->RootMotionSourceTarget);
			if (Target)
			{
				TriggerData.Target = *Target;
				if (!bTargetBasedOnFoot)
				{
					TriggerData.Target -= HalfHeightVec;
				}
			}
			else
			{
				EmplaceTriggerDataTargetWithAnimRootMotion_Lambda(Character, TriggerData, Character->GetActorLocation(),
				                                                  Character->GetActorRotation());
			}
			TriggerDatas.Emplace(TriggerData);
			Time = Windows[0].EndTime;
			idx++;
			TriggerData.Reset();
		}
		else
		{
			//起始时间不是上一次的结束时间, 说明中间有空隙, 需要填充一个默认数据
			if (Windows[idx].StartTime - Time > Tolerance)
			{
				//填充一个默认数据
				TriggerData.StartTime = Time;
				TriggerData.EndTime = Windows[idx].StartTime;
				FVector* Target = WarpingTarget.Find(Windows[idx - 1].AnimNotify->RootMotionSourceTarget);
				if (Target)
				{
					EmplaceTriggerDataTargetWithAnimRootMotion_Lambda(Character, TriggerData, *Target,
					                                                  Character->GetActorRotation());
				}

				TriggerDatas.Emplace(TriggerData);
				TriggerData.Reset();
				//填充有目标的
				TriggerData.StartTime = Windows[idx].StartTime;
				TriggerData.EndTime = Windows[idx].EndTime;
				Target = WarpingTarget.Find(Windows[idx].AnimNotify->RootMotionSourceTarget);
				if (Target)
				{
					TriggerData.Target = *Target;
					if (!bTargetBasedOnFoot)
					{
						TriggerData.Target -= HalfHeightVec;
					}
				}

				TriggerDatas.Emplace(TriggerData);
				Time = Windows[idx].EndTime;
				idx++;
			}
			//否则即使后一个窗口的起点早于前一个窗口的结束 也从结束点开始
			else
			{
				TriggerData.StartTime = Windows[idx - 1].EndTime;
				TriggerData.EndTime = Windows[idx].EndTime;
				const auto Target = WarpingTarget.Find(Windows[idx].AnimNotify->RootMotionSourceTarget);
				if (Target)
				{
					TriggerData.Target = *Target;
				}
				TriggerDatas.Emplace(TriggerData);
				Time = Windows[idx].EndTime;
				idx++;
			}
			TriggerData.Reset();
		}
	}

	if (TriggerDatas.Num() == 0)
	{
		return false;
	}

	if (RMS::CVarRMS_Debug.GetValueOnGameThread() > 0)
	{
		for (int32 i = 0; i < TriggerDatas.Num(); i++)
		{
			DrawDebugCapsule(Character->GetWorld(),
			                 TriggerDatas[i].Target + FVector(
				                 0, 0, Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()),
			                 Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight(),
			                 Character->GetCapsuleComponent()->GetScaledCapsuleRadius(),
			                 Character->GetActorQuat(),
			                 FColor::Green,
			                 false,
			                 5,
			                 0,
			                 1);
		}
	}
	TSharedPtr<FRootMotionSource_AnimWarping_MultiTargets> RMS = MakeShared<
		FRootMotionSource_AnimWarping_MultiTargets>();
	RMS->InstanceName = InstanceName == NAME_None ? TEXT("MotioWarping") : InstanceName;

	RMS->AccumulateMode = ERootMotionAccumulateMode::Override;
	RMS->Priority = NewPriority;
	//这个目标必须是脚底位置
	RMS->TriggerDatas = TriggerDatas;
	RMS->StartLocation = Character->GetActorLocation();
	RMS->StartRotation = Character->GetActorRotation();
	RMS->Duration = Duration;
	RMS->bIgnoreZAxis = false;
	RMS->Animation = DataAnimation;
	RMS->AnimStartTime = StartTime;
	RMS->AnimEndTime = DataAnimation->GetPlayLength();
	RMS->SetTime(StartTime);
	return MovementComponent->ApplyRootMotionSource(RMS) > -1;
}

bool URMSLibrary::GetRootMotionSourceWindow(UAnimSequence* DataAnimation, FName InstanceName,
                                                         FRMSWindowData& Window)
{
	if (!DataAnimation)
	{
		return false;
	}
	const auto Notifies = DataAnimation->Notifies;
	for (auto Noti : Notifies)
	{
		if (UAnimNotifyState_RMS_Warping* RMSNoti = Cast<UAnimNotifyState_RMS_Warping>(Noti.NotifyStateClass))
		{
			if (RMSNoti->RootMotionSourceTarget == InstanceName)
			{
				Window.AnimNotify = RMSNoti;
				Window.StartTime = Noti.GetTriggerTime();
				Window.EndTime = Noti.GetEndTriggerTime();
				return true;
			}
		}
	}
	return false;
}

bool URMSLibrary::GetRootMotionSourceWindows(UAnimSequence* DataAnimation,
                                                          TArray<FRMSWindowData>& Windows)
{
	if (!DataAnimation)
	{
		return false;
	}
	const auto Notifies = DataAnimation->Notifies;
	for (auto Noti : Notifies)
	{
		if (UAnimNotifyState_RMS_Warping* RMSNoti = Cast<UAnimNotifyState_RMS_Warping>(Noti.NotifyStateClass))
		{
			FRMSWindowData Window;
			Window.AnimNotify = RMSNoti;
			Window.StartTime = Noti.GetTriggerTime();
			Window.EndTime = Noti.GetEndTriggerTime();
			Windows.Emplace(Window);
		}
	}
	return Windows.Num() > 0;
}

bool URMSLibrary::GetRootMotionSourceWindowsByInstanceList(UAnimSequence* DataAnimation,
                                                                        TArray<FName> Instances,
                                                                        TArray<FRMSWindowData>& Windows)
{
	if (!DataAnimation)
	{
		return false;
	}
	for (auto Ins : Instances)
	{
		FRMSWindowData Window;
		if (GetRootMotionSourceWindow(DataAnimation, Ins, Window))
		{
			Windows.Emplace(Window);
		}
	}
	Windows.Sort([](FRMSWindowData A, FRMSWindowData B)
	{
		return A.StartTime < B.StartTime;
	});
	return Windows.Num() > 0;
}

bool URMSLibrary::FindTriggerDataByTime(const TArray<FRMSNotifyTriggerData>& TriggerData,
                                                     float Time, FRMSNotifyTriggerData& OutData)
{
	if (TriggerData.Num() == 0)
	{
		return false;
	}
	for (auto T : TriggerData)
	{
		if (Time >= T.WindowData.StartTime && Time < T.WindowData.EndTime)
		{
			OutData = T;
			return true;
		}
	}
	return false;
}

FTransform URMSLibrary::ExtractRootMotion(UAnimSequenceBase* Anim, float StartTime,
                                                       float EndTime)
{
	FTransform OutTransform;
	if (UAnimSequence* Seq = Cast<UAnimSequence>(Anim))
	{
		OutTransform = Seq->ExtractRootMotionFromRange(StartTime, EndTime);
	}
	else if (UAnimMontage* Montage = Cast<UAnimMontage>(Anim))
	{
		OutTransform = Montage->ExtractRootMotionFromTrackRange(StartTime, EndTime);
	}
	return OutTransform;
}

FTransform URMSLibrary::ProcessRootMotionInRange(const ACharacter* Character, UAnimSequenceBase* Animation,
	const FTransform& InRootMotion, float InPreviousTime, float InCurrentTime, FVector TargetLocation,
	bool bIgnoreZAxis)
{
	FTransform FinalRootMotion = InRootMotion;
	if (!Animation || !Character)
	{
		return InRootMotion;
	}
	float EndTime = Animation->GetPlayLength();
	
	const FTransform RootMotionTotal = ExtractRootMotion(Animation, InPreviousTime, EndTime);
	const FTransform RootMotionDelta = ExtractRootMotion(Animation, InPreviousTime, FMath::Min(InCurrentTime, EndTime));

	if (!RootMotionDelta.GetTranslation().IsNearlyZero())
	{
		const FTransform CurrentTransform = FTransform(
			Character->GetActorQuat(),
			Character->GetActorLocation() - FVector(0.f, 0.f, Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));
		//剩下的总共的RootMotion
		const FTransform RootMotionTotalWorldSpace = CurrentTransform * Character->GetMesh()->ConvertLocalRootMotionToWorld(RootMotionTotal);
		//这一帧的RootMotion
		const FTransform RootMotionDeltaWorldSpace = Character->GetMesh()->ConvertLocalRootMotionToWorld(RootMotionDelta);
		const FVector CurrentLocation = CurrentTransform.GetLocation();
		const FQuat CurrentRotation = CurrentTransform.GetRotation();

		
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

bool URMSLibrary::ApplyRootMotionSource_SimpleAnimation(UCharacterMovementComponent* MovementComponent,
                                                        UAnimSequence* DataAnimation,
                                                        FName InstanceName,
                                                        int32 Priority,
                                                        float StartTime,
                                                        float EndTime,
                                                        float Rate,
                                                        bool bIgnoreZAxis,
                                                        ERMSApplyMode ApplyMode)
{
	if (!MovementComponent || !DataAnimation)
	{
		return false;
	}
	const int32 NewPriority = CalcPriorityByApplyMode(MovementComponent, InstanceName, Priority, ApplyMode);
	if (NewPriority < 0)
	{
		return false;
	}
	const float CurrEndTime = (EndTime < 0 || EndTime > DataAnimation->GetPlayLength())
		                          ? DataAnimation->GetPlayLength()
		                          : EndTime;
	const float Duration = (CurrEndTime - StartTime) / FMath::Max(Rate, 0.1);
	TSharedPtr<FRootMotionSource_AnimWarping> RMS = MakeShared<FRootMotionSource_AnimWarping>();
	RMS->InstanceName = InstanceName == NAME_None ? TEXT("MotioWarping") : InstanceName;
	RMS->AccumulateMode = ERootMotionAccumulateMode::Override;
	RMS->Priority = NewPriority;
	RMS->StartLocation = MovementComponent->GetOwner()->GetActorLocation();
	RMS->StartRotation = MovementComponent->GetOwner()->GetActorRotation();
	RMS->Duration = Duration;
	RMS->bIgnoreZAxis = bIgnoreZAxis;
	RMS->Animation = DataAnimation;
	RMS->AnimStartTime = StartTime;
	RMS->AnimEndTime = EndTime;
	RMS->SetTime(StartTime);
	return MovementComponent->ApplyRootMotionSource(RMS) > -1;
}


int32 URMSLibrary::ApplyRootMotionSource_ConstantForece(UCharacterMovementComponent* MovementComponent,
                                                                     FName InstanceName,
                                                                     ERootMotionAccumulateMode AccumulateMod,
                                                                     int32 Priority, FVector WorldDirection,
                                                                     float Strength,
                                                                     UCurveFloat* StrengthOverTime, float Duration,
                                                                     ERMSFinishVelocityMode VelocityOnFinishMode,
                                                                     FVector FinishSetVelocity,
                                                                     float FinishClampVelocity, bool bEnableGravity,
                                                                     ERMSApplyMode ApplyMode)
{
	const int32 NewPriority = CalcPriorityByApplyMode(MovementComponent, InstanceName, Priority, ApplyMode);
	if (NewPriority < 0)
	{
		return -1;
	}
	TSharedPtr<FRootMotionSource_ConstantForce> ConstantForce = MakeShared<FRootMotionSource_ConstantForce>();
	ConstantForce->InstanceName = InstanceName;
	ConstantForce->AccumulateMode = AccumulateMod;
	ConstantForce->Priority = NewPriority;
	ConstantForce->Force = WorldDirection * Strength;
	ConstantForce->Duration = Duration;
	ConstantForce->StrengthOverTime = StrengthOverTime;
	ConstantForce->FinishVelocityParams.Mode = static_cast<ERootMotionFinishVelocityMode>(static_cast<uint8>(
		VelocityOnFinishMode));
	ConstantForce->FinishVelocityParams.SetVelocity = FinishSetVelocity;
	ConstantForce->FinishVelocityParams.ClampVelocity = FinishClampVelocity;
	if (bEnableGravity)
	{
		ConstantForce->Settings.SetFlag(ERootMotionSourceSettingsFlags::IgnoreZAccumulate);
	}
	return MovementComponent->ApplyRootMotionSource(ConstantForce);
}

int32 URMSLibrary::ApplyRootMotionSource_RadialForece(UCharacterMovementComponent* MovementComponent,
                                                                   FName InstanceName,
                                                                   ERootMotionAccumulateMode AccumulateMod,
                                                                   int32 Priority, AActor* LocationActor,
                                                                   FVector Location, float Strength,
                                                                   float Radius, bool bNoZForce,
                                                                   UCurveFloat* StrengthDistanceFalloff,
                                                                   UCurveFloat* StrengthOverTime, bool bIsPush,
                                                                   float Duration, bool bUseFixedWorldDirection,
                                                                   FRotator FixedWorldDirection,
                                                                   ERMSFinishVelocityMode VelocityOnFinishMode,
                                                                   FVector FinishSetVelocity,
                                                                   float FinishClampVelocity,
                                                                   ERMSApplyMode ApplyMode)
{
	const int32 NewPriority = CalcPriorityByApplyMode(MovementComponent, InstanceName, Priority, ApplyMode);
	if (NewPriority < 0)
	{
		return -1;
	}
	TSharedPtr<FRootMotionSource_RadialForce> RadialForce = MakeShared<FRootMotionSource_RadialForce>();
	RadialForce->InstanceName = InstanceName;
	RadialForce->AccumulateMode = AccumulateMod;
	RadialForce->Priority = NewPriority;
	RadialForce->Location = Location;
	RadialForce->LocationActor = LocationActor;
	RadialForce->Duration = Duration;
	RadialForce->Radius = Radius;
	RadialForce->Strength = Strength;
	RadialForce->bIsPush = bIsPush;
	RadialForce->bNoZForce = bNoZForce;
	RadialForce->StrengthDistanceFalloff = StrengthDistanceFalloff;
	RadialForce->StrengthOverTime = StrengthOverTime;
	RadialForce->bUseFixedWorldDirection = bUseFixedWorldDirection;
	RadialForce->FixedWorldDirection = FixedWorldDirection;
	RadialForce->FinishVelocityParams.Mode = static_cast<ERootMotionFinishVelocityMode>(static_cast<uint8>(
		VelocityOnFinishMode));
	RadialForce->FinishVelocityParams.SetVelocity = FinishSetVelocity;
	RadialForce->FinishVelocityParams.ClampVelocity = FinishClampVelocity;
	return MovementComponent->ApplyRootMotionSource(RadialForce);
}


void URMSLibrary::UpdateDynamicMoveToTarget(UCharacterMovementComponent* MovementComponent,
                                                         FName InstanceName, FVector NewTarget)
{
	if (MovementComponent && !NewTarget.ContainsNaN())
	{
		auto RMS = MovementComponent->GetRootMotionSource(InstanceName);
		if (!RMS)
		{
			return;
		}
		auto RMS_Dy = StaticCastSharedPtr<FRootMotionSource_MoveToDynamicForce>(RMS);
		if (!RMS_Dy)
		{
			return;
		}
		if (NewTarget.Equals(RMS_Dy->TargetLocation, 0.1))
		{
			return;
		}

		FVector oldTarget = RMS_Dy->TargetLocation;
		FVector oldStart = RMS_Dy->StartLocation;
		float friction = RMS_Dy->GetTime() / RMS_Dy->GetDuration();
		FVector oldDir = (oldTarget - oldStart);
		oldDir.Normalize();
		FVector CurrPoint = oldStart + (oldTarget - oldStart) * friction;
		//FVector p = MovementComponent->GetCharacterOwner()->GetActorLocation() - FVector(0,0,MovementComponent->GetCharacterOwner()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
		FVector newDir = (NewTarget - CurrPoint);
		newDir.Normalize();
		FVector NewStart = CurrPoint + newDir * (-1) * (oldTarget - oldStart).Size() * friction;
		RMS_Dy->StartLocation = NewStart;
		RMS_Dy->SetTargetLocation(NewTarget);
		if (RMS::CVarRMS_Debug.GetValueOnGameThread() == 1)
		{
			UKismetSystemLibrary::DrawDebugSphere(MovementComponent, NewStart, 25, 4, FColor::Cyan, 5, 1);
			UKismetSystemLibrary::DrawDebugSphere(MovementComponent, NewTarget, 25, 4, FColor::Cyan, 5, 1);
			UKismetSystemLibrary::DrawDebugSphere(MovementComponent, oldStart, 25, 4, FColor::Red, 5, 1);
			UKismetSystemLibrary::DrawDebugSphere(MovementComponent, oldTarget, 25, 4, FColor::Red, 5, 1);
			UKismetSystemLibrary::DrawDebugSphere(MovementComponent, CurrPoint, 25, 4, FColor::Yellow, 5, 1);
			//UKismetSystemLibrary::DrawDebugSphere(MovementComponent,p,25,4,FColor::White,5,1);
		}
	}
}


void URMSLibrary::RemoveRootMotionSource(UCharacterMovementComponent* MovementComponent,
                                                      FName InstanceName)
{
	if (MovementComponent)
	{
		MovementComponent->RemoveRootMotionSource(InstanceName);
	}
}

void URMSLibrary::UpdateDynamicMoveDuration(UCharacterMovementComponent* MovementComponent,
                                                         FName InstanceName, float NewDuration)
{
	if (MovementComponent && NewDuration > 0)
	{
		auto RMS = MovementComponent->GetRootMotionSource(InstanceName);
		if (!RMS)
		{
			return;
		}
		auto RMS_Dy = StaticCastSharedPtr<FRootMotionSource_MoveToDynamicForce>(RMS);
		if (!RMS_Dy)
		{
			return;
		}
		float ratio = RMS_Dy->CurrentTime / RMS_Dy->Duration;

		RMS_Dy->Duration = NewDuration;
		RMS_Dy->SetTime(NewDuration * ratio);
	}
}

void URMSLibrary::GetCurrentRootMotionSourceTime(UCharacterMovementComponent* MovementComponent,
                                                              FName InstanceName, float& Time, float& Duration)
{
	if (auto RMS = GetRootMotionSource(MovementComponent, InstanceName))
	{
		Time = RMS->GetTime();
		Duration = RMS->GetDuration();
	}
}

bool URMSLibrary::IsRootMotionSourceValid(UCharacterMovementComponent* MovementComponent,
                                                       FName InstanceName)
{
	return GetRootMotionSource(MovementComponent, InstanceName).IsValid();
}

bool URMSLibrary::IsRootMotionSourceIdValid(UCharacterMovementComponent* MovementComponent, int32 ID)
{
	return GetRootMotionSourceByID(MovementComponent, ID).IsValid();
}

TSharedPtr<FRootMotionSource> URMSLibrary::GetRootMotionSource(
	UCharacterMovementComponent* MovementComponent, FName InstanceName)
{
	if (MovementComponent)
	{
		return MovementComponent->GetRootMotionSource(InstanceName);
	}
	return nullptr;
}

TSharedPtr<FRootMotionSource_MoveToDynamicForce> URMSLibrary::GetDynamicMoveToRootMotionSource(
	UCharacterMovementComponent* MovementComponent, FName InstanceName)
{
	if (MovementComponent)
	{
		auto RMS = MovementComponent->GetRootMotionSource(InstanceName);
		if (RMS)
		{
			return StaticCastSharedPtr<FRootMotionSource_MoveToDynamicForce>(RMS);
		}
	}
	return nullptr;
}

TSharedPtr<FRootMotionSource> URMSLibrary::GetRootMotionSourceByID(
	UCharacterMovementComponent* MovementComponent, int32 ID)
{
	if (MovementComponent)
	{
		auto RMS = MovementComponent->GetRootMotionSourceByID(ID);
		if (RMS)
		{
			return StaticCastSharedPtr<FRootMotionSource_MoveToDynamicForce>(RMS);
		}
	}
	return nullptr;
}

void URMSLibrary::CalcAnimWarpingScale(FVector& OriginOffset, ERMSAnimWarpingType Type,
                                                    FVector AnimRootMotionLinear, FVector RMSTargetLinearOffset,
                                                    float Scale, float Tolerance)
{
	switch (Type)
	{
	case ERMSAnimWarpingType::BasedOnLength:
		{
			const float WarpRatio = FMath::IsNearlyZero(AnimRootMotionLinear.Size(), Tolerance)
				                        ? 0
				                        : RMSTargetLinearOffset.Size() / AnimRootMotionLinear.Size();
			OriginOffset *= WarpRatio * Scale;
			break;
		}
	case ERMSAnimWarpingType::BasedOn3Axis:
		{
			FVector WarpRatio;
			WarpRatio.X = FMath::IsNearlyZero(AnimRootMotionLinear.X, Tolerance)
				              ? 0
				              : RMSTargetLinearOffset.X / AnimRootMotionLinear.X;
			WarpRatio.Y = FMath::IsNearlyZero(AnimRootMotionLinear.Y, Tolerance)
				              ? 0
				              : RMSTargetLinearOffset.Y / AnimRootMotionLinear.Y;
			WarpRatio.Z = FMath::IsNearlyZero(AnimRootMotionLinear.Z, Tolerance)
				              ? 0
				              : RMSTargetLinearOffset.Z / AnimRootMotionLinear.Z;
			OriginOffset *= WarpRatio * Scale;
			break;
		}
	default:
		break;
	}
}

void URMSLibrary::ConvWorldOffsetToRmsSpace(FVector& OffsetWS, FVector Start, FVector Target)
{
	FRotator FacingRot = (Target - Start).Rotation();
	FacingRot.Pitch = 0;
	//UKismetMathLibrary::InverseTransformDirection(FTransform(FacingRot,StartLocation - HalfHeightVec,FVector::OneVector),Offset_LinearBase); //这样不行吗???
	const FVector ForwardVec = UKismetMathLibrary::GetForwardVector(FacingRot);
	const FVector RightVec = UKismetMathLibrary::GetRightVector(FacingRot);
	//const FVector UpVec = UKismetMathLibrary::GetUpVector(FacingRot);
	FVector ProjFwd = OffsetWS.ProjectOnTo(ForwardVec);
	FVector ProjRt = OffsetWS.ProjectOnTo(RightVec);
	//FVector ProjUp = OffsetWS.ProjectOnTo(UpVec);
	OffsetWS = FVector(ProjFwd.Size() * (ProjFwd.GetSafeNormal() | ForwardVec.GetSafeNormal()),
	                   ProjRt.Size() * (ProjRt.GetSafeNormal() | RightVec.GetSafeNormal()),
	                   OffsetWS.Z);
}

void URMSLibrary::FiltAnimCurveOffsetAxisData(FVector& AnimOffset, ERMSAnimWarpingAxis Axis)
{
	switch (Axis)
	{
	case ERMSAnimWarpingAxis::None:
		{
			AnimOffset.X = 0;
			AnimOffset.Y = 0;
			AnimOffset.Z = 0;
			break;
		}
	case ERMSAnimWarpingAxis::X:
		{
			AnimOffset.Y = 0;
			AnimOffset.Z = 0;
			break;
		}
	case ERMSAnimWarpingAxis::Y:
		{
			AnimOffset.X = 0;
			AnimOffset.Z = 0;
			break;
		}
	case ERMSAnimWarpingAxis::Z:
		{
			AnimOffset.X = 0;
			AnimOffset.Y = 0;
			break;
		}
	case ERMSAnimWarpingAxis::XY:
		{
			AnimOffset.Z = 0;
			break;
		}
	case ERMSAnimWarpingAxis::XZ:
		{
			AnimOffset.Y = 0;
			break;
		}
	case ERMSAnimWarpingAxis::YZ:
		{
			AnimOffset.X = 0;

			break;
		}
	case ERMSAnimWarpingAxis::XYZ:
		{
			break;
		}
	default:
		break;
	}
}

int32 URMSLibrary::CalcPriorityByApplyMode(UCharacterMovementComponent* MovementComponent,
                                                        FName PendingInstanceName, int32 PendingPriorioty,
                                                        ERMSApplyMode ApplyMode)
{
	if (!MovementComponent)
	{
		return -1;
	}
	if (auto RMS = MovementComponent->GetRootMotionSource(PendingInstanceName))
	{
		const int32 OldPriority = RMS->Priority;
		switch (ApplyMode)
		{
		case ERMSApplyMode::ApplyHigherPriority:
			{
				return FMath::Max(OldPriority + 1, PendingPriorioty);
			}
		case ERMSApplyMode::Replace:
			{
				MovementComponent->RemoveRootMotionSource(PendingInstanceName);
				return OldPriority;
			}
		case ERMSApplyMode::Block:
			{
				return -1;
			}
		default:
			return PendingPriorioty;
		}
	}
	return PendingPriorioty;
}

bool URMSLibrary::PredictRootMotionSourceLocation_Runtime(UCharacterMovementComponent* MovementComponent,
                                                                   FName InstanceName, float Time, FVector& OutLocation)
{
	if (!MovementComponent)
	{
		return false;
	}
	auto RMS = GetRootMotionSource(MovementComponent, InstanceName);
	if (!RMS.IsValid())
	{
		return false;
	}

	if (auto RMS_MoveToDy = StaticCastSharedPtr<FRootMotionSource_MoveToDynamicForce>(RMS))
	{
		float Fraction = Time / RMS_MoveToDy->GetDuration();
		if (RMS_MoveToDy->TimeMappingCurve)
		{
			Fraction = EvaluateFloatCurveAtFraction(*RMS_MoveToDy->TimeMappingCurve, Fraction);
		}
		const FVector PathOffset = RMS_MoveToDy->GetPathOffsetInWorldSpace(Fraction);
		const FVector CurrentTargetLocation = FMath::Lerp<FVector, float>(
			RMS_MoveToDy->StartLocation, RMS_MoveToDy->TargetLocation, Fraction);
		OutLocation = CurrentTargetLocation + PathOffset;
		return true;
	}
	else if (auto RMS_MoveTo = StaticCastSharedPtr<FRootMotionSource_MoveToForce>(RMS))
	{
		const float Fraction = Time / RMS_MoveTo->GetDuration();
		FVector PathOffset = FVector::ZeroVector;
		if (RMS_MoveTo->PathOffsetCurve)
		{
			PathOffset = RMS_MoveTo->GetPathOffsetInWorldSpace(Fraction);
		}
		const FVector CurrentTargetLocation = FMath::Lerp<FVector, float>(
			RMS_MoveTo->StartLocation, RMS_MoveTo->TargetLocation, Fraction);
		OutLocation = CurrentTargetLocation + PathOffset;
		return true;
	}
	else if (auto RMS_Jump = StaticCastSharedPtr<FRootMotionSource_JumpForce>(RMS))
	{
		float Fraction = Time / RMS_Jump->GetDuration();
		if (RMS_Jump->TimeMappingCurve)
		{
			float MinCurveTime(0.f);
			float MaxCurveTime(1.f);
			//
			// RMS_Jump->TimeMappingCurve->GetTimeRange(MinCurveTime, MaxCurveTime);
			//
			// Fraction =  RMS_Jump->TimeMappingCurve->GetFloatValue(FMath::GetRangeValue(FVector2D(MinCurveTime, MaxCurveTime), Fraction));

			Fraction = EvaluateFloatCurveAtFraction(*RMS_Jump->TimeMappingCurve, Fraction);
			const FVector TargetRelativeLocation = RMS_Jump->GetRelativeLocation(Fraction);
			if (MovementComponent->GetOwner())
			{
				OutLocation = MovementComponent->GetOwner()->GetActorLocation() + TargetRelativeLocation;
				return true;
			}
		}
	}

	return false;
}


bool URMSLibrary::PredictRootMotionSourceLocation_MoveTo(FVector& OutLocation,
                                                                  UCharacterMovementComponent* MovementComponent,
                                                                  FVector StartLocation, FVector TargetLocation,
                                                                  float Duration,
                                                                  float Time, UCurveVector* PathOffsetCurve)
{
	if (!MovementComponent || Duration <= 0 || Time < 0 || Time > Duration)
	{
		return false;
	}
	const float Fraction = Time / Duration;
	FVector CurrentTargetLocation = FMath::Lerp<FVector, float>(StartLocation, TargetLocation, Fraction);
	FVector PathOffset = FVector::ZeroVector;
	if (PathOffsetCurve)
	{
		PathOffset = EvaluateVectorCurveAtFraction(*PathOffsetCurve, Fraction);
		FRotator FacingRotation((TargetLocation - StartLocation).Rotation());
		FacingRotation.Pitch = 0.f;
		PathOffset = FacingRotation.RotateVector(PathOffset);
	}
	OutLocation = CurrentTargetLocation + PathOffset;

	return true;
}

bool URMSLibrary::PredictRootMotionSourceLocation_Jump(FVector& OutLocation,
                                                                UCharacterMovementComponent* MovementComponent,
                                                                FVector StartLocation, float Distance, float Height,
                                                                FRotator Rotation,
                                                                float Duration, float Time,
                                                                UCurveVector* PathOffsetCurve,
                                                                UCurveFloat* TimeMappingCurve)
{
	if (!MovementComponent || Duration <= 0 || Time < 0 || Time > Duration)
	{
		return false;
	}
	FVector PathOffset(FVector::ZeroVector);
	float Fraction = Time / Duration;
	if (TimeMappingCurve)
	{
		Fraction = EvaluateFloatCurveAtFraction(*TimeMappingCurve, Fraction);
	}
	if (PathOffsetCurve)
	{
		PathOffset = EvaluateVectorCurveAtFraction(*PathOffsetCurve, Fraction);
	}
	else
	{
		const float Phi = 2.f * Fraction - 1;
		const float Z = -(Phi * Phi) + 1;
		PathOffset.Z = Z;
	}
	if (Height >= 0.f)
	{
		PathOffset.Z *= Height;
	}


	FRotator FacingRotation(Rotation);
	FacingRotation.Pitch = 0.f;
	FVector RelativeLocationFacingSpace = FVector(Fraction * Distance, 0.f, 0.f) + PathOffset;
	RelativeLocationFacingSpace = FacingRotation.RotateVector(RelativeLocationFacingSpace);
	OutLocation = StartLocation + RelativeLocationFacingSpace;
	return true;
}

bool URMSLibrary::PredictRootMotionSourceLocation_MoveToParabola(FVector& OutLocation
                                                                          , UCharacterMovementComponent*
                                                                          MovementComponent
                                                                          , FVector StartLocation
                                                                          , FVector TargetLocation
                                                                          , float Duration
                                                                          , float CurrentTime
                                                                          , UCurveFloat* ParabolaCurve
                                                                          , UCurveFloat* TimeMappingCurve)
{
	if (!MovementComponent || Duration <= 0 || CurrentTime < 0 || CurrentTime > Duration)
	{
		return false;
	}
	float Fraction = CurrentTime / Duration;
	FVector CurrentTargetLocation = FMath::Lerp<FVector, float>(StartLocation, TargetLocation, Fraction);
	if (TimeMappingCurve)
	{
		Fraction = EvaluateFloatCurveAtFraction(*TimeMappingCurve, Fraction);
	}
	if (ParabolaCurve)
	{
		const float Z = (TargetLocation - StartLocation).Z;
		const float CurveValue = EvaluateFloatCurveAtFraction(*ParabolaCurve, Fraction);
		CurrentTargetLocation.Z = StartLocation.Z + CurveValue * Z;
	}
	OutLocation = CurrentTargetLocation;

	return true;
}

PRAGMA_ENABLE_OPTIMIZATION
