// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RootMotionSourceTypes.h"
#include "GameFramework/RootMotionSource.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RootMotionSourceLibrary.generated.h"


class URootMotionSourceComponent;
class UCurveVector;
enum class ERootMotionAccumulateMode : uint8;
class UCharacterMovementComponent;
UCLASS()
class ROOTMOTIONEXTENSION_API URootMotionSourceLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category="RootMotionExtension", meta = (AdvancedDisplay = "7"))
	static int32 ApplyRootMotionSource_MoveToForce(UCharacterMovementComponent* MovementComponent, FRMS_MoveTo Setting);
	
	UFUNCTION(BlueprintCallable, Category="RootMotionExtension", meta = (AdvancedDisplay = "7"))
	static int32 ApplyRootMotionSource_JumpForce(UCharacterMovementComponent* MovementComponent,FRMS_Jump Setting);

	UFUNCTION(BlueprintCallable, Category="RootMotionExtension", meta = (AdvancedDisplay = "7"))
	static int32 ApplyRootMotionSource_DynamicMoveToForce(UCharacterMovementComponent* MovementComponent, FRMS_DynamicMoveTo Setting);
	
#pragma region Animation
	UFUNCTION(BlueprintCallable, Category="RootMotionExtension", meta = (AdvancedDisplay = "7"))
	static bool ApplyRootMotionSource_SimpleAnimation(UCharacterMovementComponent* MovementComponent, USkeletalMeshComponent* Mesh, UAnimSequence* DataAnimation, FName InstanceName, int32 Priority,
		float StartTime =0, float EndTime = -1, float TimeScale =1);
	
	/**
	* 通过动画数据 获取RMS偏移， 通过ApplyRootMotionSource_MoveToForce 实现位移
	* @param DataAnimation      参考RootMotion数据的动画, 该节点本身不负责播放动画
	* @param bLocalTarget		如果为true, 那么位置偏移是基于脚底的, 所以需要考虑角色capsule的haflHeight
	* 
	*/
	UFUNCTION(BlueprintCallable, Category="RootMotionExtension", meta = (AdvancedDisplay = "7"))
	static bool ApplyRootMotionSource_AnimationAdjustment(UCharacterMovementComponent* MovementComponent, USkeletalMeshComponent* Mesh, UAnimSequence* DataAnimation, FName InstanceName, int32 Priority,  FVector TargetLocation,
																bool bLocalTarget,  bool bUseCustomDuration = false, float CustomDuration =1.0);
	/**
	* 通过动画数据 获取RMS偏移， 通过ApplyRootMotionSource_MoveToForce 实现位移
	* @param DataAnimation      参考RootMotion数据的动画, 该节点本身不负责播放动画
	* @param bLocalTarget		如果为true, 那么位置偏移是基于脚底的, 所以需要考虑角色capsule的haflHeight;(注意: TargetLocation是FromFrame至TargetFram的位移目标)
	* 
	*/
	UFUNCTION(BlueprintCallable, Category="RootMotionExtension", meta = (AdvancedDisplay = "7"))
	static bool ApplyRootMotionSource_AnimationAdjustmentByFrame(UCharacterMovementComponent* MovementComponent, USkeletalMeshComponent* Mesh, UAnimSequence* DataAnimation, FName InstanceName, int32 Priority,  FVector TargetLocation,
																bool bLocalTarget,  int32 FromFrame = 0, int32 TargetFram = -1, float TimeScale = 1.0f);
	UFUNCTION(BlueprintCallable, Category="RootMotionExtension", meta = (AdvancedDisplay = "7"))
	static bool ApplyRootMotionSource_AnimationAdjustmentByTime(UCharacterMovementComponent* MovementComponent, USkeletalMeshComponent* Mesh, UAnimSequence* DataAnimation, FName InstanceName, int32 Priority,  FVector TargetLocation,
															bool bLocalTarget,  float FromTime = 0, int32 TargetTime = -1, float TimeScale = 1.0f);
	UFUNCTION(BlueprintCallable, Category="RootMotionExtension", meta = (AdvancedDisplay = "7"))
	static bool ApplyRootMotionSource_AnimationWarping(URootMotionSourceComponent* RootMotionSourceComponent, USkeletalMeshComponent* Mesh, UAnimSequence* DataAnimation, TMap<FName, FVector> WarpingTarget, float TimeScale = 1, float Tolerance = 0.01, float AnimWarpingMulti = 1.0);

	static bool GetRootMotionSourceWindow(UAnimSequence* DataAnimation,FName InstanceName, FRootMotionSoueceWindowData& Window);
	static bool GetRootMotionSourceWindows(UAnimSequence* DataAnimation, TArray<FRootMotionSoueceWindowData>& Windows);
	static bool GetRootMotionSourceWindowsByInstanceList(UAnimSequence* DataAnimation, TArray<FName> Instances, TArray<FRootMotionSoueceWindowData>& Windows);

	static bool FindTriggerDataByTime(const TArray<FRootMotionSoueceTriggerData>& TriggerData, float Time, FRootMotionSoueceTriggerData& OutData);
#pragma endregion Animation


	
	UFUNCTION(BlueprintCallable, Category="RootMotionExtension", meta = (AdvancedDisplay = "7"))
	static int32 ApplyRootMotionSource_ConstantForece(UCharacterMovementComponent* MovementComponent, FName InstanceName, ERootMotionAccumulateMode AccumulateMod, int32 Priority, FVector WorldDirection, float Strength, UCurveFloat* StrengthOverTime, float Duration, EFinishVelocityMode VelocityOnFinishMode,
	FVector FinishSetVelocity, float FinishClampVelocity = 0, bool bEnableGravity = false);

	UFUNCTION(BlueprintCallable, Category="RootMotionExtension", meta = (AdvancedDisplay = "7"))
	static int32 ApplyRootMotionSource_RadialForece(UCharacterMovementComponent* MovementComponent, FName InstanceName, ERootMotionAccumulateMode AccumulateMod, int32 Priority,  AActor* LocationActor, FVector Location, float Strength,  float Radius,
	bool bNoZForce,UCurveFloat* StrengthDistanceFalloff,UCurveFloat* StrengthOverTime, bool bIsPush,float Duration,  bool bUseFixedWorldDirection, FRotator FixedWorldDirection, EFinishVelocityMode VelocityOnFinishMode,
FVector FinishSetVelocity, float FinishClampVelocity = 0);

	UFUNCTION(BlueprintCallable, Category="RootMotionExtension", meta = (AdvancedDisplay = "7"))
	static void UpdateDynamicMoveToTarget(UCharacterMovementComponent* MovementComponent, FName InstanceName, FVector NewTarget);
	UFUNCTION(BlueprintCallable, Category="RootMotionExtension", meta = (AdvancedDisplay = "7"))
	static void RemoveRootMotionSource(UCharacterMovementComponent* MovementComponent, FName InstanceName);
	UFUNCTION(BlueprintCallable, Category="RootMotionExtension", meta = (AdvancedDisplay = "7"))
	static void UpdateDynamicMoveDuration(UCharacterMovementComponent* MovementComponent, FName InstanceName, float NewDuration);
	UFUNCTION(BlueprintCallable, Category="RootMotionExtension", meta = (AdvancedDisplay = "7"))
	static void GetCurrentRootMotionSourceTime(UCharacterMovementComponent* MovementComponent, FName InstanceName, float& Time, float& Duration);
	UFUNCTION(BlueprintCallable, Category="RootMotionExtension", meta = (AdvancedDisplay = "7"), BlueprintPure)
	static bool IsRootMotionSourceValid(UCharacterMovementComponent* MovementComponent, FName InstanceName);
	UFUNCTION(BlueprintCallable, Category="RootMotionExtension", meta = (AdvancedDisplay = "7"), BlueprintPure)
	static bool IsRootMotionSourceIdValid(UCharacterMovementComponent* MovementComponent, int32 ID);
	
	static TSharedPtr<FRootMotionSource> GetRootMotionSource(UCharacterMovementComponent* MovementComponent, FName InstanceName) ;
	static TSharedPtr<FRootMotionSource_MoveToDynamicForce> GetDynamicMoveToRootMotionSource(UCharacterMovementComponent* MovementComponent, FName InstanceName) ;
	
	static TSharedPtr<FRootMotionSource> GetRootMotionSourceByID(UCharacterMovementComponent* MovementComponent, int32 ID) ;
	
};

