// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "RMSTypes.generated.h"

namespace RMS
{
static TAutoConsoleVariable<int32> CVarRMS_Debug(TEXT("b.RMS.Debug"), 0, TEXT("0: Disable 1: Enable "), ECVF_Cheat);
}





UENUM(BlueprintType)
enum class ERMSApplyMode :uint8
{
	//不做处理
	None = 0,
	//取代同名RMS, 会先把同名RMS移除掉
	Replace,
	//用同名的RMS优先级+1应用
	ApplyHigherPriority,
	//如果有同名的RMS,那就取消应用	
	Block,
	//排队
	Queue
};

UENUM(BlueprintType)
enum class ERMSAnimWarpingAxis :uint8
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
enum class ERMSAnimWarpingType :uint8
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
enum class ERMSFinishVelocityMode :uint8
{
	// Maintain the last velocity root motion gave to the character
	MaintainLastRootMotionVelocity = 0,
	// Set Velocity to the specified value (for example, 0,0,0 to stop the character)
	SetVelocity,
	// Clamp velocity magnitude to the specified value. Note that it will not clamp Z if negative (falling). it will clamp Z positive though. 
	ClampVelocity,
};

UENUM(BlueprintType)
enum class ERMSSourceSettingsFlags : uint8
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
struct FRMSPathMoveToData
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite)
	FVector Target = FVector::ZeroVector;
	UPROPERTY(BlueprintReadWrite)
	float Duration = 0;
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UCurveVector> PathOffsetCurve = nullptr;
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UCurveFloat> TimeMappingCurve = nullptr;

	friend FArchive& operator <<(FArchive& Ar, FRMSPathMoveToData& D)
	{
		return Ar << D.Duration << D.Target << D.PathOffsetCurve << D.TimeMappingCurve;
	}

	virtual bool operator==(const FRMSPathMoveToData& Other) const
	{
		return Target == Other.Target && Duration == Other.Duration && PathOffsetCurve == Other.PathOffsetCurve &&
			TimeMappingCurve == Other.TimeMappingCurve;
	}

	virtual bool operator!=(const FRMSPathMoveToData& Other) const
	{
		return !(*this == Other);
	}

	bool IsValid() const
	{
		return Duration > 0;
	}
};

USTRUCT(BlueprintType)
struct FRMSSetting
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite)
	ERootMotionAccumulateMode AccumulateMod = ERootMotionAccumulateMode::Override;
	UPROPERTY(BlueprintReadWrite)
	ERMSFinishVelocityMode VelocityOnFinishMode = ERMSFinishVelocityMode::ClampVelocity;
	UPROPERTY(BlueprintReadWrite)
	FVector FinishSetVelocity = FVector::ZeroVector;
	UPROPERTY(BlueprintReadWrite)
	float FinishClampVelocity = 20;
};

USTRUCT(BlueprintType)
struct FRMSSetting_Move : public FRMSSetting
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite)
	bool bRestrictSpeedToExpected = false;
	UPROPERTY(BlueprintReadWrite)
	ERMSSourceSettingsFlags SourcesSetting = ERMSSourceSettingsFlags::UseSensitiveLiftoffCheck;
};

USTRUCT(BlueprintType)
struct FRMSSetting_Jump : public FRMSSetting
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite)
	float MinimumLandedTriggerTime = 0;
	UPROPERTY(BlueprintReadWrite)
	bool bFinishOnLanded = false;
};


USTRUCT(BlueprintType)
struct FRMSWindowData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defaults")
	TObjectPtr<class UAnimNotifyState_RMS_Warping> AnimNotify = nullptr;

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
struct FRMSNotifyTriggerData
{
	GENERATED_BODY()


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defaults")
	FRMSWindowData WindowData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defaults")
	FVector Target = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defaults")
	bool bHasTarget = false;

	virtual bool operator ==(const FRMSNotifyTriggerData& other)
	{
		return WindowData.AnimNotify == other.WindowData.AnimNotify &&
			WindowData.EndTime == other.WindowData.EndTime &&
			WindowData.StartTime == other.WindowData.StartTime &&
			Target == other.Target &&
			bHasTarget == other.bHasTarget;
	}

	virtual bool operator !=(const FRMSNotifyTriggerData& other)
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


USTRUCT(BlueprintType)
struct FRMSTarget
{
	GENERATED_BODY()
	FRMSTarget()
	{
	}

	virtual ~FRMSTarget()
	{
	}
	friend FArchive& operator <<(FArchive& Ar, FRMSTarget& D)
	{
		return Ar << D.Target << D.StartTime << D.EndTime ;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defaults")
	float StartTime = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defaults")
	float EndTime = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defaults")
	FVector Target = FVector::ZeroVector;


	virtual bool operator ==(const FRMSTarget& other)const
	{
		return 
			StartTime == other.StartTime &&
			EndTime == other.EndTime &&
			Target == other.Target ;
	}

	virtual bool operator !=(const FRMSTarget& other)const
	{
		return !((*this) == other);
	}

	FORCEINLINE void Reset()
	{
		
		EndTime = 0;
		StartTime = 0;
		Target = FVector::ZeroVector;
		
	}
};