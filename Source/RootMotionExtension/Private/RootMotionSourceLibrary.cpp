// Fill out your copyright notice in the Description page of Project Settings.


#include "RootMotionSourceLibrary.h"

#include "AnimNotifyState_RootMotionSource.h"
#include "RootMotionSourceComponent.h"
#include "Components/CapsuleComponent.h"
#include "Curves/CurveVector.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

class UAnimNotifyState_RootMotionSource;
TAutoConsoleVariable<int32> CVarRMS_Debug(TEXT("b.RMS.Debug"), 0, TEXT("0: Disable 1: Enable "), ECVF_Cheat);
PRAGMA_DISABLE_OPTIMIZATION

int32 URootMotionSourceLibrary::ApplyRootMotionSource_MoveToForce(UCharacterMovementComponent* MovementComponent,
                                                                  FRMS_MoveTo Setting)
{
	if (!MovementComponent)
	{
		return -1;
	}
	TSharedPtr<FRootMotionSource_MoveToForce> MoveToForce = MakeShared<FRootMotionSource_MoveToForce>();
	MoveToForce->InstanceName = Setting.InstanceName;
	MoveToForce->AccumulateMode = Setting.AccumulateMod;
	MoveToForce->Settings.SetFlag(
		static_cast<ERootMotionSourceSettingsFlags>(static_cast<uint8>(Setting.SourcesSetting)));
	MoveToForce->Priority = Setting.Priority;
	MoveToForce->TargetLocation = Setting.TargetLocation;
	MoveToForce->StartLocation = Setting.StartLocation;
	MoveToForce->Duration = Setting.Duration;
	MoveToForce->bRestrictSpeedToExpected = Setting.bRestrictSpeedToExpected;
	MoveToForce->PathOffsetCurve = Setting.PathOffsetCurve;
	MoveToForce->FinishVelocityParams.Mode = static_cast<ERootMotionFinishVelocityMode>(static_cast<uint8>(Setting.
		VelocityOnFinishMode));
	MoveToForce->FinishVelocityParams.SetVelocity = Setting.FinishSetVelocity;
	MoveToForce->FinishVelocityParams.ClampVelocity = Setting.FinishClampVelocity;
	return MovementComponent->ApplyRootMotionSource(MoveToForce);
}

int32 URootMotionSourceLibrary::ApplyRootMotionSource_JumpForce(UCharacterMovementComponent* MovementComponent,
                                                                FRMS_Jump Setting)
{
	if (!MovementComponent)
	{
		return -1;
	}
	TSharedPtr<FRootMotionSource_JumpForce> JumpForce = MakeShared<FRootMotionSource_JumpForce>();
	JumpForce->InstanceName = Setting.InstanceName;
	JumpForce->AccumulateMode = Setting.AccumulateMod;
	JumpForce->Priority = Setting.Priority;
	JumpForce->Duration = Setting.Duration;
	JumpForce->Rotation = Setting.Rotation;
	JumpForce->Distance = Setting.Distance;
	JumpForce->Height = Setting.Height;
	JumpForce->bDisableTimeout = Setting.bFinishOnLanded; // If we finish on landed, we need to disable force's timeout
	JumpForce->PathOffsetCurve = Setting.PathOffsetCurve;
	JumpForce->TimeMappingCurve = Setting.TimeMappingCurve;
	JumpForce->FinishVelocityParams.Mode = static_cast<ERootMotionFinishVelocityMode>(static_cast<uint8>(Setting.
		VelocityOnFinishMode));
	JumpForce->FinishVelocityParams.SetVelocity = Setting.FinishSetVelocity;
	JumpForce->FinishVelocityParams.ClampVelocity = Setting.FinishClampVelocity;
	return MovementComponent->ApplyRootMotionSource(JumpForce);
}

int32 URootMotionSourceLibrary::ApplyRootMotionSource_DynamicMoveToForce(
	UCharacterMovementComponent* MovementComponent, FRMS_DynamicMoveTo Setting)
{
	if (!MovementComponent)
	{
		return -1;
	}
	TSharedPtr<FRootMotionSource_MoveToDynamicForce> MoveToActorForce = MakeShared<
		FRootMotionSource_MoveToDynamicForce>();
	MoveToActorForce->InstanceName = Setting.InstanceName;
	MoveToActorForce->AccumulateMode = Setting.AccumulateMod;
	MoveToActorForce->Settings.SetFlag(
		static_cast<ERootMotionSourceSettingsFlags>(static_cast<uint8>(Setting.SourcesSetting)));
	MoveToActorForce->Priority = Setting.Priority;
	MoveToActorForce->InitialTargetLocation = Setting.TargetLocation;
	MoveToActorForce->TargetLocation = Setting.TargetLocation;
	MoveToActorForce->StartLocation = Setting.StartLocation;
	MoveToActorForce->Duration = FMath::Max(Setting.Duration, KINDA_SMALL_NUMBER);
	MoveToActorForce->bRestrictSpeedToExpected = Setting.bRestrictSpeedToExpected;
	MoveToActorForce->PathOffsetCurve = Setting.PathOffsetCurve;
	MoveToActorForce->TimeMappingCurve = Setting.TimeMappingCurve;
	MoveToActorForce->FinishVelocityParams.Mode = static_cast<ERootMotionFinishVelocityMode>(static_cast<uint8>(Setting.
		VelocityOnFinishMode));
	MoveToActorForce->FinishVelocityParams.SetVelocity = Setting.FinishSetVelocity;
	MoveToActorForce->FinishVelocityParams.ClampVelocity = Setting.FinishClampVelocity;
	return MovementComponent->ApplyRootMotionSource(MoveToActorForce);
}

bool URootMotionSourceLibrary::ApplyRootMotionSource_SimpleAnimation(UCharacterMovementComponent* MovementComponent,
                                                                     USkeletalMeshComponent* Mesh,
                                                                     UAnimSequence* DataAnimation, FName InstanceName,
                                                                     int32 Priority,
                                                                     float StartTime, float EndTime, float TimeScale)
{
	if (!MovementComponent || !DataAnimation || !Mesh || (StartTime > EndTime && EndTime > 0))
	{
		return false;
	}
	ACharacter* Character = Cast<ACharacter>(Mesh->GetOwner());
	if (!Character)
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
	RMT = Mesh2Char * RMT;

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
		CurrFrameTM = Mesh2Char * CurrFrameTM;

		FVector AnimRootMotionLinearFraction = RMT.GetLocation() * Fraction;
		//动画位置与线性偏移位置的偏差
		FVector CurveOffset = CurrFrameTM.GetLocation() - AnimRootMotionLinearFraction;

		CurveX.AddKey(CurrentTime / Duration * TimeScale, CurveOffset.X);
		CurveY.AddKey(CurrentTime / Duration * TimeScale, CurveOffset.Y);
		CurveZ.AddKey(CurrentTime / Duration * TimeScale, CurveOffset.Z);


		if (CVarRMS_Debug.GetValueOnGameThread() == 1)
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
	FRMS_MoveTo setting;
	setting.InstanceName = InstanceName;
	setting.AccumulateMod = ERootMotionAccumulateMode::Override;
	setting.Priority = Priority;
	setting.StartLocation = StartLocation;
	setting.TargetLocation = WorldTarget;
	setting.Duration = Duration * TimeScale;
	setting.bRestrictSpeedToExpected = false;
	setting.PathOffsetCurve = OffsetCV;
	setting.VelocityOnFinishMode = EFinishVelocityMode::ClampVelocity;
	setting.FinishClampVelocity = 20;

	//用MoveToForce来计算路径, 尝试过用Jump+OffsetCv, 但是Jump的CV不好用
	return ApplyRootMotionSource_MoveToForce(MovementComponent, setting) >= 0;
}


bool URootMotionSourceLibrary::ApplyRootMotionSource_AnimationAdjustment(
	UCharacterMovementComponent* MovementComponent, USkeletalMeshComponent* Mesh, UAnimSequence* DataAnimation,
	FName InstanceName, int32 Priority, FVector TargetLocation,
	bool bLocalTarget, bool bUseCustomDuration, float CustomDuration,float AnimWarpingScale)
{
	if (!MovementComponent || !DataAnimation || !Mesh || (bUseCustomDuration && CustomDuration <= 0))
	{
		return false;
	}
	ACharacter* Character = Cast<ACharacter>(Mesh->GetOwner());
	if (!Character)
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
	FVector LocationOffset = bLocalTarget
		                         ? TargetLocation
		                         : UKismetMathLibrary::InverseTransformLocation(
			                         Character->GetActorTransform(), TargetLocation) + FVector(0, 0, HalfHeight);
	//开始位置
	FVector StartLocation = Character->GetActorLocation();
	//模型与角色的相对变换矩阵,我们只需要Rotation
	FTransform Mesh2Char = Mesh->GetComponentTransform().GetRelativeTransform(Character->GetActorTransform());
	Mesh2Char.SetLocation(FVector::Zero());

	//获取RootMotion最终值, 即根骨的最大偏移量
	FTransform RMT = DataAnimation->ExtractRootMotionFromRange(0, Duration);
	RMT = Mesh2Char * RMT;
	//获取所需的最后一帧，不一定要遍历所有
	int32 LastFrame = bUseCustomDuration ? FMath::CeilToInt(NumFrame * DurationRatio) : NumFrame;

	//用动态曲线的方式 , 而非静态,
	UCurveVector* OffsetCV = NewObject<UCurveVector>();
	FRichCurve CurveX, CurveY, CurveZ;
	float LastTime = 0;
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
		float LastFraction = LastTime / Duration;
		//获取当前时间的rootMotion
		CurrFrameTM = DataAnimation->ExtractRootMotion(0, CurrentTime, false);

		CurrFrameTM = Mesh2Char * CurrFrameTM;
		FVector LocalLinearOffsetFraction = LocationOffset * Fraction;
		FVector AnimRootMotionLinearFraction = RMT.GetLocation() * Fraction;
		//动画位置与线性偏移位置的偏差
		FVector CurveOffset = CurrFrameTM.GetLocation() - AnimRootMotionLinearFraction;
		//期望的线性偏移 与 动画线性偏移的比率
		float WarpRatio = 1;
		WarpRatio = FMath::IsNearlyZero(AnimRootMotionLinearFraction.Size(), 0.01f) ? 0 : LocalLinearOffsetFraction.Size() / AnimRootMotionLinearFraction.Size();
		WarpRatio *= AnimWarpingScale;
		CurveOffset *= WarpRatio;
		CurveX.AddKey(CurrentTime / Duration, CurveOffset.X);
		CurveY.AddKey(CurrentTime / Duration, CurveOffset.Y);
		CurveZ.AddKey(CurrentTime / Duration, CurveOffset.Z);


		if (CVarRMS_Debug.GetValueOnGameThread() == 1)
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
		LastTime = CurrentTime;
	}
	OffsetCV->FloatCurves[0] = CurveX;
	OffsetCV->FloatCurves[1] = CurveY;
	OffsetCV->FloatCurves[2] = CurveZ;

	FVector WorldOffset = LocationOffset.X * Character->GetActorForwardVector() + LocationOffset.Y * Character->
		GetActorRightVector() + LocationOffset.Z * Character->GetActorUpVector();
	FVector WorldTarget = StartLocation + WorldOffset;
	FRMS_MoveTo setting;
	setting.InstanceName = InstanceName;
	setting.AccumulateMod = ERootMotionAccumulateMode::Override;
	setting.Priority = Priority;
	setting.StartLocation = StartLocation;
	setting.TargetLocation = WorldTarget;
	setting.Duration = Duration;
	setting.bRestrictSpeedToExpected = false;
	setting.PathOffsetCurve = OffsetCV;
	setting.VelocityOnFinishMode = EFinishVelocityMode::ClampVelocity;
	setting.FinishClampVelocity = 20;

	//用MoveToForce来计算路径, 尝试过用Jump+OffsetCv, 但是Jump的CV不好用
	return ApplyRootMotionSource_MoveToForce(MovementComponent, setting) >= 0;
}

bool URootMotionSourceLibrary::ApplyRootMotionSource_AnimationAdjustmentByFrame(
	UCharacterMovementComponent* MovementComponent, USkeletalMeshComponent* Mesh, UAnimSequence* DataAnimation,
	FName InstanceName, int32 Priority, FVector TargetLocation,
	bool bLocalTarget, int32 FromFrame, int32 TargetFram, float TimeScale, float AnimWarpingScale)
{
	if (!MovementComponent || !DataAnimation || !Mesh || FromFrame < 0 || (TargetFram > 0 && FromFrame > TargetFram))
	{
		return false;
	}
	const int32 NumFrame = DataAnimation->GetNumberOfSampledKeys();
	const float FrameTime = DataAnimation->GetPlayLength() / NumFrame;
	if (FromFrame == 0 && (TargetFram <= 0 || TargetFram == NumFrame))
	{
		return ApplyRootMotionSource_AnimationAdjustment(MovementComponent, Mesh, DataAnimation, InstanceName, Priority,
		                                                 TargetLocation, bLocalTarget, true,
		                                                 DataAnimation->GetPlayLength() * TimeScale,AnimWarpingScale);
	}
	else
	{
		ACharacter* Character = Cast<ACharacter>(Mesh->GetOwner());
		if (!Character)
		{
			return false;
		}
		float HalfHeight = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

		//动画时间数据
		const float AnimLength = DataAnimation->GetPlayLength();
		const float StartTime = FromFrame * FrameTime;
		const float EndTime = TargetFram * FrameTime;
		return ApplyRootMotionSource_AnimationAdjustmentByTime(MovementComponent, Mesh, DataAnimation, InstanceName,
		                                                       Priority, TargetLocation, bLocalTarget, StartTime,
		                                                       EndTime, TimeScale);
		return true;
	}
}

bool URootMotionSourceLibrary::ApplyRootMotionSource_AnimationAdjustmentByTime(
	UCharacterMovementComponent* MovementComponent, USkeletalMeshComponent* Mesh, UAnimSequence* DataAnimation,
	FName InstanceName, int32 Priority, FVector TargetLocation,
	bool bLocalTarget, float FromTime, int32 TargetTime, float TimeScale, float AnimWarpingScale)
{
	if (!MovementComponent || !DataAnimation || !Mesh || FromTime < 0 || (TargetTime > 0 && FromTime > TargetTime))
	{
		return false;
	}
	const int32 NumFrame = DataAnimation->GetNumberOfSampledKeys();
	const float FrameTime = DataAnimation->GetPlayLength() / NumFrame;
	const float AnimLength = DataAnimation->GetPlayLength();
	const float Duration = TargetTime - FromTime;
	if (FromTime == 0 && (TargetTime <= 0 || TargetTime == AnimLength))
	{
		return ApplyRootMotionSource_AnimationAdjustment(MovementComponent, Mesh, DataAnimation, InstanceName, Priority,
		                                                 TargetLocation, bLocalTarget, true,
		                                                 DataAnimation->GetPlayLength() * TimeScale,AnimWarpingScale);
	}
	else
	{
		ACharacter* Character = Cast<ACharacter>(Mesh->GetOwner());
		if (!Character)
		{
			return false;
		}
		float HalfHeight = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();


		//获取本地偏移
		FVector LocationOffset = bLocalTarget
			                         ? TargetLocation
			                         : UKismetMathLibrary::InverseTransformLocation(
				                         Character->GetActorTransform(), TargetLocation) + FVector(0, 0, HalfHeight);
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
		RMT = Mesh2Char * RMT;

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

			CurrFrameTM = Mesh2Char * CurrFrameTM;
			FVector LocalLinearOffsetFraction = LocationOffset * Fraction;
			FVector AnimRootMotionLinearFraction = RMT.GetLocation() * Fraction;
			//动画位置与线性偏移位置的偏差
			FVector CurveOffset = CurrFrameTM.GetLocation() - AnimRootMotionLinearFraction;
			float WarpRatio = 1;
			WarpRatio = FMath::IsNearlyZero(AnimRootMotionLinearFraction.Size(), 0.01f) ? 0 : LocalLinearOffsetFraction.Size() / AnimRootMotionLinearFraction.Size();
			WarpRatio *= AnimWarpingScale;
			CurveOffset *= WarpRatio;
			CurveX.AddKey(CurrentTime / Duration * TimeScale, CurveOffset.X);
			CurveY.AddKey(CurrentTime / Duration * TimeScale, CurveOffset.Y);
			CurveZ.AddKey(CurrentTime / Duration * TimeScale, CurveOffset.Z);


			if (CVarRMS_Debug.GetValueOnGameThread() == 1)
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

		FVector WorldOffset = LocationOffset.X * Character->GetActorForwardVector() + LocationOffset.Y * Character->
			GetActorRightVector() + LocationOffset.Z * Character->GetActorUpVector();
		FVector WorldTarget = StartLocation + WorldOffset;
		FRMS_MoveTo setting;
		setting.InstanceName = InstanceName;
		setting.AccumulateMod = ERootMotionAccumulateMode::Override;
		setting.Priority = Priority;
		setting.StartLocation = StartLocation;
		setting.TargetLocation = WorldTarget;
		setting.Duration = Duration * TimeScale;
		setting.bRestrictSpeedToExpected = false;
		setting.PathOffsetCurve = OffsetCV;
		setting.VelocityOnFinishMode = EFinishVelocityMode::ClampVelocity;
		setting.FinishClampVelocity = 20;

		//用MoveToForce来计算路径, 尝试过用Jump+OffsetCv, 但是Jump的CV不好用
		return ApplyRootMotionSource_MoveToForce(MovementComponent, setting) >= 0;
	}
}

bool URootMotionSourceLibrary::ApplyRootMotionSource_AnimationWarping(
	UCharacterMovementComponent* MovementComponent, USkeletalMeshComponent* Mesh, UAnimSequence* DataAnimation,
	TMap<FName, FVector> WarpingTarget, float TimeScale,
	float Tolerance, float AnimWarpingMulti)
{
	if (!MovementComponent || !DataAnimation || !Mesh || WarpingTarget.Num() == 0 || TimeScale <= 0)
	{
		return false;
	}
	ACharacter* Character = Cast<ACharacter>(Mesh->GetOwner());
	if (!Character)
	{
		return false;
	}
	//动画基本数据
	const int32 NumFrame = DataAnimation->GetNumberOfSampledKeys();
	const float FrameTime = DataAnimation->GetPlayLength() / NumFrame;
	const float AnimLength = DataAnimation->GetPlayLength();
	const float Duration = AnimLength * TimeScale;
	float HalfHeight = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const FVector HalfHeightVec = FVector(0, 0, HalfHeight);
	const auto Notifies = DataAnimation->Notifies;
	TArray<FName> Instances;
	WarpingTarget.GetKeys(Instances);
	TArray<FRootMotionSoueceWindowData> Windows;
	if (!GetRootMotionSourceWindowsByInstanceList(DataAnimation, Instances, Windows))
	{
		return false;
	}
	TArray<FRootMotionSoueceTriggerData> TriggerDatas;
	float Time = 0;
	int32 idx = 0;
	while (Time <= AnimLength)
	{
		FRootMotionSoueceTriggerData TriggerData;
		auto EmplaceTriggerData_Lambda = [&]()
		{
			TriggerData.WindowData = Windows[idx];
			const auto Target = WarpingTarget.Find(Windows[idx].AnimNotify->RootMotionSourceInstance);
			if (Target)
			{
				TriggerData.Target = *Target;
				TriggerData.bHasTarget = true;
			}
			TriggerDatas.Emplace(TriggerData);
		};
		//已经到最后一个通知, 但是通知的结尾不是动画结束点, 就添加一个默认数据
		if (idx > Windows.Num() - 1 && AnimLength - Windows[Windows.Num() - 1].EndTime > Tolerance)
		{
			TriggerData.bHasTarget = false;
			TriggerData.WindowData.StartTime = Windows[Windows.Num() - 1].EndTime;
			TriggerData.WindowData.EndTime = AnimLength;
			TriggerDatas.Emplace(TriggerData);
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
				const auto Target = WarpingTarget.Find(Windows[idx].AnimNotify->RootMotionSourceInstance);
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
	//获取整体RootMotion数据
	FTransform TotalAnimRM = DataAnimation->ExtractRootMotionFromRange(0, AnimLength);
	TotalAnimRM = Mesh2Char * TotalAnimRM;
	FVector LocalOffset = FVector::ZeroVector;
	//******************
	FVector LastWarpingTarget = FVector::ZeroVector;

	float LastTime = 0;
	FTransform FinalTargetAnimRM, CurrFrameAnimRM, CurrTargetAnimRM, LastTargetAnimRM;

	FVector LastTargetWS = StartLocation;
	FVector CurrTargetWS = FVector::ZeroVector;

	//**************************
	FRootMotionSoueceTriggerData TrigData;
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
		auto TM = DataAnimation->ExtractRootMotion(TriggerDatas[lastTargetIdx].WindowData.EndTime, AnimLength, false);
		auto ActorRM = (Mesh2Char * TM).GetLocation();
		FVector WorldRM = ActorRM.X * Character->GetActorForwardVector() + ActorRM.Y * Character->GetActorRightVector()
			+ ActorRM.Z * Character->GetActorUpVector();
		WorldTarget = LastWarpingTarget + WorldRM;
	}
	//最终的目标是胶囊体中心, 所以要加上半高
	WorldTarget += HalfHeightVec;
	if (CVarRMS_Debug.GetValueOnGameThread() == 1)
	{
		//绘制最后目标
		UKismetSystemLibrary::DrawDebugBox(Mesh, WorldTarget, FVector(5, 5, 20), FColor::Purple, FRotator::ZeroRotator,
		                                   5, 5.0);
	}
	//最终目标的RootMotion
	FinalTargetAnimRM = DataAnimation->ExtractRootMotion(0, AnimLength, false);
	FinalTargetAnimRM = Mesh2Char * FinalTargetAnimRM;

	FVector CurveOffset = FVector::ZeroVector;
	//逐帧遍历
	for (float CurrentTime = 0; CurrentTime < AnimLength; CurrentTime += FrameTime)
	{
		bool bNeedUpdateTarget = false;
		//如果当前时间已经大于上一次数据的最后时间, 说明换了一个窗口期, 记录上一次的时间和目标
		if (CurrentTime > TrigData.WindowData.EndTime)
		{
			LastTime = TrigData.WindowData.EndTime;
			LastTargetWS = CurrTargetWS;
			bNeedUpdateTarget = true;
			LastTargetAnimRM = CurrTargetAnimRM;
		}

		if (!FindTriggerDataByTime(TriggerDatas, CurrentTime, TrigData))
		{
			continue;
		}


		//当前分段内的百分比
		float WindowFraction = (CurrentTime - TrigData.WindowData.StartTime) / TrigData.WindowData.EndTime;
		//整体百分比
		float Fraction = CurrentTime / AnimLength;

		//获取当前时间段的rootMotion
		CurrFrameAnimRM = DataAnimation->ExtractRootMotion(0, CurrentTime, false);
		CurrFrameAnimRM = Mesh2Char * CurrFrameAnimRM;

		bool bHasWarpingTarget = false;

		//******************************************
		//查找当前窗口的目标数据, 只在窗口改变以后的时候刷新
		if (bNeedUpdateTarget || CurrentTime == 0)
		{
			if (TrigData.bHasTarget && TrigData.WindowData.AnimNotify)
			{
				const auto target = WarpingTarget.Find(TrigData.WindowData.AnimNotify->RootMotionSourceInstance);
				if (target)
				{
					//把目标处理一下半高, 基于脚底计算, 所以要加上半高
					CurrTargetWS = *target + HalfHeightVec;
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
				const auto T = DataAnimation->ExtractRootMotion(LastTime, TrigData.WindowData.EndTime, false);
				LocalOffset = (Mesh2Char * T).GetLocation();
				FVector WorldOffset = LocalOffset.X * Character->GetActorForwardVector() + LocalOffset.Y * Character->
					GetActorRightVector() + LocalOffset.Z * Character->GetActorUpVector();
				CurrTargetWS = LastTargetWS + WorldOffset;
			}
			CurrTargetAnimRM = DataAnimation->ExtractRootMotion(0, TrigData.WindowData.EndTime, false);
		}
		//******************************************


		//*****************计算关键数据*************************
		//当前窗口的线性位移
		FVector WindowLinearOffset = (CurrTargetWS - LastTargetWS) * WindowFraction;
		//全局线性偏移
		FVector FinalLinearOffset = (WorldTarget - StartLocation) * Fraction;
		//当前窗口的线性位移与基础运动线性位移的偏差, 本地空间
		FVector Offset_LinearBase = UKismetMathLibrary::InverseTransformLocation(
				Character->GetActorTransform(), LastTargetWS + WindowLinearOffset) -
			UKismetMathLibrary::InverseTransformLocation(Character->GetActorTransform(),
			                                             (StartLocation + FinalLinearOffset));
		
	
		FVector WindowAnimLinearOffset = (CurrTargetAnimRM.GetLocation() - LastTargetAnimRM.GetLocation()) * WindowFraction;
		FVector Offset_Anim = CurrFrameAnimRM.GetLocation() - (LastTargetAnimRM.GetLocation() + WindowAnimLinearOffset);
		
		//计算目标线性位移与动画RM线性位移的比值
		float WarpRatio = 1;
		WarpRatio = FMath::IsNearlyZero(WindowAnimLinearOffset.Size(), 0.01f) ? 0 : WindowLinearOffset.Size() / WindowAnimLinearOffset.Size();
		WarpRatio *= AnimWarpingMulti;
		CurveOffset = Offset_LinearBase + Offset_Anim * WarpRatio;


		CurveX.AddKey(CurrentTime / Duration, CurveOffset.X);
		CurveY.AddKey(CurrentTime / Duration, CurveOffset.Y);
		CurveZ.AddKey(CurrentTime / Duration, CurveOffset.Z);


		if (CVarRMS_Debug.GetValueOnGameThread() == 1)
		{
			//动画每一帧位置
			UKismetSystemLibrary::DrawDebugSphere(
				Mesh, UKismetMathLibrary::TransformLocation(Character->GetActorTransform(),
				                                            CurrFrameAnimRM.GetLocation()) - FVector(0, 0, HalfHeight),
				5.0, 4, FColor::Yellow, 5.0);
			//当前窗口动画线性位置
			UKismetSystemLibrary::DrawDebugSphere(
				Mesh, UKismetMathLibrary::TransformLocation(Character->GetActorTransform(),
				                                            WindowAnimLinearOffset + LastTargetAnimRM.GetLocation()) - FVector(
					0, 0, HalfHeight), 5.0, 4, FColor::Red, 5.0);
			//最终目标的线性位置
			UKismetSystemLibrary::DrawDebugSphere(
				Mesh, StartLocation +  FinalLinearOffset -
				FVector(0, 0, HalfHeight), 5.0, 4, FColor::Green, 5.0);
			//当前窗口的线性位置
			UKismetSystemLibrary::DrawDebugSphere(Mesh, LastTargetWS + WindowLinearOffset - FVector(0, 0, HalfHeight),
			                                      5.0, 4, FColor::Orange, 5.0);
			//矫正后的每一帧位置
			UKismetSystemLibrary::DrawDebugSphere(
				Mesh, FinalLinearOffset + UKismetMathLibrary::TransformLocation(
					Character->GetActorTransform(), CurveOffset) - FVector(0, 0, HalfHeight), 5.0, 4, FColor::Blue, 5.0);
		}
	}
	OffsetCV->FloatCurves[0] = CurveX;
	OffsetCV->FloatCurves[1] = CurveY;
	OffsetCV->FloatCurves[2] = CurveZ;


	FRMS_MoveTo setting;

	setting.InstanceName = "AnimationWarping";
	setting.AccumulateMod = ERootMotionAccumulateMode::Override;
	setting.Priority = 999;
	setting.StartLocation = StartLocation;
	setting.TargetLocation = WorldTarget;
	setting.Duration = Duration * TimeScale;
	setting.bRestrictSpeedToExpected = false;
	setting.PathOffsetCurve = OffsetCV;
	setting.VelocityOnFinishMode = EFinishVelocityMode::ClampVelocity;
	setting.FinishClampVelocity = 20;

	//用MoveToForce来计算路径, 尝试过用Jump+OffsetCv, 但是Jump的CV不好用
	return ApplyRootMotionSource_MoveToForce(MovementComponent, setting) >= 0;
}

bool URootMotionSourceLibrary::GetRootMotionSourceWindow(UAnimSequence* DataAnimation, FName InstanceName,
                                                         FRootMotionSoueceWindowData& Window)
{
	if (!DataAnimation)
	{
		return false;
	}
	const auto Notifies = DataAnimation->Notifies;
	for (auto Noti : Notifies)
	{
		if (UAnimNotifyState_RootMotionSource* RMSNoti = Cast<UAnimNotifyState_RootMotionSource>(Noti.NotifyStateClass))
		{
			if (RMSNoti->RootMotionSourceInstance == InstanceName)
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

bool URootMotionSourceLibrary::GetRootMotionSourceWindows(UAnimSequence* DataAnimation,
                                                          TArray<FRootMotionSoueceWindowData>& Windows)
{
	if (!DataAnimation)
	{
		return false;
	}
	const auto Notifies = DataAnimation->Notifies;
	for (auto Noti : Notifies)
	{
		if (UAnimNotifyState_RootMotionSource* RMSNoti = Cast<UAnimNotifyState_RootMotionSource>(Noti.NotifyStateClass))
		{
			FRootMotionSoueceWindowData Window;
			Window.AnimNotify = RMSNoti;
			Window.StartTime = Noti.GetTriggerTime();
			Window.EndTime = Noti.GetEndTriggerTime();
			Windows.Emplace(Window);
		}
	}
	return Windows.Num() > 0;
}

bool URootMotionSourceLibrary::GetRootMotionSourceWindowsByInstanceList(UAnimSequence* DataAnimation,
                                                                        TArray<FName> Instances,
                                                                        TArray<FRootMotionSoueceWindowData>& Windows)
{
	if (!DataAnimation)
	{
		return false;
	}
	for (auto Ins : Instances)
	{
		FRootMotionSoueceWindowData Window;
		if (GetRootMotionSourceWindow(DataAnimation, Ins, Window))
		{
			Windows.Emplace(Window);
		}
	}
	Windows.Sort([](FRootMotionSoueceWindowData A, FRootMotionSoueceWindowData B)
	{
		return A.StartTime < B.StartTime;
	});
	return Windows.Num() > 0;
}

bool URootMotionSourceLibrary::FindTriggerDataByTime(const TArray<FRootMotionSoueceTriggerData>& TriggerData,
                                                     float Time, FRootMotionSoueceTriggerData& OutData)
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


int32 URootMotionSourceLibrary::ApplyRootMotionSource_ConstantForece(UCharacterMovementComponent* MovementComponent,
                                                                     FName InstanceName,
                                                                     ERootMotionAccumulateMode AccumulateMod,
                                                                     int32 Priority, FVector WorldDirection,
                                                                     float Strength,
                                                                     UCurveFloat* StrengthOverTime, float Duration,
                                                                     EFinishVelocityMode VelocityOnFinishMode,
                                                                     FVector FinishSetVelocity,
                                                                     float FinishClampVelocity, bool bEnableGravity)
{
	TSharedPtr<FRootMotionSource_ConstantForce> ConstantForce = MakeShared<FRootMotionSource_ConstantForce>();
	ConstantForce->InstanceName = InstanceName;
	ConstantForce->AccumulateMode = AccumulateMod;
	ConstantForce->Priority = Priority;
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

int32 URootMotionSourceLibrary::ApplyRootMotionSource_RadialForece(UCharacterMovementComponent* MovementComponent,
                                                                   FName InstanceName,
                                                                   ERootMotionAccumulateMode AccumulateMod,
                                                                   int32 Priority, AActor* LocationActor,
                                                                   FVector Location, float Strength,
                                                                   float Radius, bool bNoZForce,
                                                                   UCurveFloat* StrengthDistanceFalloff,
                                                                   UCurveFloat* StrengthOverTime, bool bIsPush,
                                                                   float Duration, bool bUseFixedWorldDirection,
                                                                   FRotator FixedWorldDirection,
                                                                   EFinishVelocityMode VelocityOnFinishMode,
                                                                   FVector FinishSetVelocity,
                                                                   float FinishClampVelocity)
{
	TSharedPtr<FRootMotionSource_RadialForce> RadialForce = MakeShared<FRootMotionSource_RadialForce>();
	RadialForce->InstanceName = InstanceName;
	RadialForce->AccumulateMode = AccumulateMod;
	RadialForce->Priority = Priority;
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


void URootMotionSourceLibrary::UpdateDynamicMoveToTarget(UCharacterMovementComponent* MovementComponent,
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
		if (CVarRMS_Debug.GetValueOnGameThread() == 1)
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


void URootMotionSourceLibrary::RemoveRootMotionSource(UCharacterMovementComponent* MovementComponent,
                                                      FName InstanceName)
{
	if (MovementComponent)
	{
		MovementComponent->RemoveRootMotionSource(InstanceName);
	}
}

void URootMotionSourceLibrary::UpdateDynamicMoveDuration(UCharacterMovementComponent* MovementComponent,
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

void URootMotionSourceLibrary::GetCurrentRootMotionSourceTime(UCharacterMovementComponent* MovementComponent,
                                                              FName InstanceName, float& Time, float& Duration)
{
	if (auto RMS = GetRootMotionSource(MovementComponent, InstanceName))
	{
		Time = RMS->GetTime();
		Duration = RMS->GetDuration();
	}
}

bool URootMotionSourceLibrary::IsRootMotionSourceValid(UCharacterMovementComponent* MovementComponent,
                                                       FName InstanceName)
{
	return GetRootMotionSource(MovementComponent, InstanceName).IsValid();
}

bool URootMotionSourceLibrary::IsRootMotionSourceIdValid(UCharacterMovementComponent* MovementComponent, int32 ID)
{
	return GetRootMotionSourceByID(MovementComponent, ID).IsValid();
}

TSharedPtr<FRootMotionSource> URootMotionSourceLibrary::GetRootMotionSource(
	UCharacterMovementComponent* MovementComponent, FName InstanceName)
{
	if (MovementComponent)
	{
		return MovementComponent->GetRootMotionSource(InstanceName);
	}
	return nullptr;
}

TSharedPtr<FRootMotionSource_MoveToDynamicForce> URootMotionSourceLibrary::GetDynamicMoveToRootMotionSource(
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

TSharedPtr<FRootMotionSource> URootMotionSourceLibrary::GetRootMotionSourceByID(
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

PRAGMA_ENABLE_OPTIMIZATION
