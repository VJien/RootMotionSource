// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "RootMotionSourceTypes.generated.h"
UENUM(BlueprintType)
enum class ERootMotionSourceAnimWarpingAxis :uint8
{
	None = 0,
	X,
	Y,
	Z,
	XY,
	XZ,
	YZ,
	XYZ
};

UENUM(BlueprintType)
enum class ERootMotionAnimWarpingType :uint8
{
	// compared with total length
	BasedOnLength = 0,
	// compared with 3 axis
	BasedOn3Axis,
};

/**
 * 
 */
UENUM(BlueprintType)
enum class EFinishVelocityMode :uint8
{
	// Maintain the last velocity root motion gave to the character
	MaintainLastRootMotionVelocity = 0,
	// Set Velocity to the specified value (for example, 0,0,0 to stop the character)
	SetVelocity,
	// Clamp velocity magnitude to the specified value. Note that it will not clamp Z if negative (falling). it will clamp Z positive though. 
	ClampVelocity,
};

UENUM(BlueprintType)
enum class ESourceSettingsFlags : uint8
{
	None,
	// Source will switch character to Falling mode with any "Z up" velocity added.
	// Use this for jump-like root motion. If not enabled, uses default jump impulse
	// detection (which keeps you stuck on ground in Walking fairly strongly)
	UseSensitiveLiftoffCheck,
	// If Duration of Source would end partway through the last tick it is active,
	// do not reduce SimulationTime. Disabling this is useful for sources that
	// are more about providing velocity (like jumps), vs. sources that need
	// the precision of partial ticks for say ending up at an exact location (MoveTo)
	DisablePartialEndTick,
	// Whether to ignore impact to Z when accumulating output to Velocity
	// Setting this flag on override sources provides the same behavior as
	// animation root motion
	IgnoreZAccumulate
};





USTRUCT(BlueprintType)
struct FRootMotionSourceSetting
{
	GENERATED_BODY()
public:
	// FRootMotionSourceSetting()
	// {
	// }
	// FRootMotionSourceSetting(FRootMotionSourceSetting InData): AccumulateMod(
	// 		InData.AccumulateMod),VelocityOnFinishMode(InData.VelocityOnFinishMode),FinishSetVelocity(InData.FinishSetVelocity),FinishClampVelocity(InData.FinishClampVelocity),bForce(InData.bForce)
	// {}
	//
	// FRootMotionSourceSetting(ERootMotionAccumulateMode InAccumulateMod, EFinishVelocityMode InVelocityOnFinishMode,
	//                          FVector InFinishSetVelocity, float InFinishClampVelocity, bool bInForce): AccumulateMod(
	// 	InAccumulateMod),VelocityOnFinishMode(InVelocityOnFinishMode),FinishSetVelocity(InFinishSetVelocity),FinishClampVelocity(InFinishClampVelocity),bForce(bInForce)
	// {
	// }

	// void operator=(const FRootMotionSourceSetting& other)
	// {
	// 	AccumulateMod = other.AccumulateMod;
	// }
	
	
    static const FRootMotionSourceSetting DefaultRootMotionSourceSetting;
	
	UPROPERTY(BlueprintReadWrite)
	ERootMotionAccumulateMode AccumulateMod = ERootMotionAccumulateMode::Override;
	UPROPERTY(BlueprintReadWrite)
	EFinishVelocityMode VelocityOnFinishMode = EFinishVelocityMode::MaintainLastRootMotionVelocity;
	UPROPERTY(BlueprintReadWrite)
	FVector FinishSetVelocity = FVector::ZeroVector;
	UPROPERTY(BlueprintReadWrite)
	float FinishClampVelocity = 0;
	UPROPERTY(BlueprintReadWrite)
	bool bForce = false;
};

USTRUCT(BlueprintType)
struct FRootMotionSourceMoveSetting : public FRootMotionSourceSetting
{
	GENERATED_BODY()
public:
	static const FRootMotionSourceMoveSetting DefaultRootMotionSourceMoveSetting;
	
	UPROPERTY(BlueprintReadWrite)
	bool bRestrictSpeedToExpected = false;
	UPROPERTY(BlueprintReadWrite)
	ESourceSettingsFlags SourcesSetting = ESourceSettingsFlags::UseSensitiveLiftoffCheck;
};

USTRUCT(BlueprintType)
struct FRootMotionSourceJumpSetting : public FRootMotionSourceSetting
{
	GENERATED_BODY()
public:
	static const FRootMotionSourceJumpSetting DefaultRootMotionSourceJumpSetting;
	
	UPROPERTY(BlueprintReadWrite)
	float MinimumLandedTriggerTime = 0;
	UPROPERTY(BlueprintReadWrite)
	bool bFinishOnLanded = false;
};


USTRUCT(BlueprintType)
struct FRootMotionSoueceWindowData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defaults")
	TObjectPtr<class UAnimNotifyState_RootMotionSource> AnimNotify = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defaults")
	float StartTime = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defaults")
	float EndTime = 0.f;


	FORCEINLINE bool IsValid() const
	{
		return AnimNotify != nullptr;
	}
};

USTRUCT(BlueprintType)
struct FRootMotionSoueceTriggerData
{
	GENERATED_BODY()


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defaults")
	FRootMotionSoueceWindowData WindowData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defaults")
	FVector Target = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defaults")
	bool bHasTarget = false;

	virtual bool operator ==(const FRootMotionSoueceTriggerData& other)
	{
		return WindowData.AnimNotify == other.WindowData.AnimNotify &&
			WindowData.EndTime == other.WindowData.EndTime &&
			WindowData.StartTime == other.WindowData.StartTime &&
			Target == other.Target &&
			bHasTarget == other.bHasTarget;
	}

	virtual bool operator !=(const FRootMotionSoueceTriggerData& other)
	{
		return !((*this) == other);
	}

	FORCEINLINE void Reset()
	{
		WindowData.AnimNotify = nullptr;
		WindowData.EndTime = 0;
		WindowData.StartTime = 0;
		Target = FVector::ZeroVector;
		bHasTarget = false;
	}
};
