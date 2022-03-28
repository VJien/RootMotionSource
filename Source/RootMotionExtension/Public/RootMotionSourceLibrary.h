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
	/**
	* 移动到一个点
	* @param Setting.StartLocation      角色会基于此开始移动,所以请确保是Actor当前的Location
	* @param Setting.TargetLocation		参考StartLocation的目标位置(要考虑HalfHeight)
	* 
	*/
	UFUNCTION(BlueprintCallable, Category="RootMotionExtension", meta = (AdvancedDisplay = "7"))
	static int32 ApplyRootMotionSource_MoveToForce(UCharacterMovementComponent* MovementComponent, FRMS_MoveTo Setting);
	//通过高度和距离适配一个抛物线跳跃运动
	UFUNCTION(BlueprintCallable, Category="RootMotionExtension", meta = (AdvancedDisplay = "7"))
	static int32 ApplyRootMotionSource_JumpForce(UCharacterMovementComponent* MovementComponent, FRMS_Jump Setting);
	/**
	* 移动到一个动态目标, 需要通过UpdateDynamicMoveToTarget设置目标
	* @param Setting.StartLocation      角色会基于此开始移动,所以请确保是Actor当前的Location
	* @param Setting.TargetLocation		参考StartLocation的目标位置(要考虑HalfHeight)
	* 
	*/
	UFUNCTION(BlueprintCallable, Category="RootMotionExtension", meta = (AdvancedDisplay = "7"))
	static int32 ApplyRootMotionSource_DynamicMoveToForce(UCharacterMovementComponent* MovementComponent,
	                                                      FRMS_DynamicMoveTo Setting);

	/**
	* 抛物线的形式移动到一个点, 通过一个曲线来设定运动轨迹
	* @param Setting.StartLocation      角色会基于此开始移动,所以请确保是Actor当前的Location
	* @param Setting.TargetLocation		参考StartLocation的目标位置(要考虑HalfHeight)
	* 
	*/
	UFUNCTION(BlueprintCallable, Category="RootMotionExtension", meta = (AdvancedDisplay = "7"))
	static int32 ApplyRootMotionSource_MoveToForce_Parabola(UCharacterMovementComponent* MovementComponent, FRMS_MoveToParabola Setting);
	
#pragma region Animation
	/**
	* 直接使用动画的RootMotion数据,效果等同于播放RootMotion蒙太奇动画
	* @param DataAnimation    参考RootMotion数据的动画, 该节点本身不负责播放动画
	* @param EndTime		  播放的结束时间, 如果小于0,那么就使用最终的动画长度
	* @param TimeScale        动画时间缩放
	* 
	*/
	UFUNCTION(BlueprintCallable, Category="RootMotionExtension", meta = (AdvancedDisplay = "7"))
	static bool ApplyRootMotionSource_SimpleAnimation(UCharacterMovementComponent* MovementComponent,
	                                                  USkeletalMeshComponent* Mesh, UAnimSequence* DataAnimation,
	                                                  FName InstanceName, int32 Priority,
	                                                  float StartTime = 0, float EndTime = -1, float TimeScale = 1);

	/**
	* 依据动画RootMotion数据适配目标点的运动,效果类似MotionWarping,  <位置偏移是基于脚底的>
	* @param DataAnimation      参考RootMotion数据的动画, 该节点本身不负责播放动画
	* @param bLocalTarget		如果为true,那么偏移信息是本地空间的
	* 
	*/
	UFUNCTION(BlueprintCallable, Category="RootMotionExtension", meta = (AdvancedDisplay = "7"))
	static bool ApplyRootMotionSource_AnimationAdjustment(UCharacterMovementComponent* MovementComponent,
	                                                      USkeletalMeshComponent* Mesh, UAnimSequence* DataAnimation,
	                                                      FName InstanceName, int32 Priority, FVector TargetLocation,
	                                                      bool bLocalTarget, bool bUseCustomDuration = false,
	                                                      float CustomDuration = 1.0, float AnimWarpingScale = 1.0,
	                                                      ERootMotionAnimWarpingType WarpingType =
		                                                      ERootMotionAnimWarpingType::BasedOnLength,
	                                                      ERootMotionSourceAnimWarpingAxis WarpingAxis =
		                                                      ERootMotionSourceAnimWarpingAxis::XYZ);
	/**
	* 基于ApplyRootMotionSource_AnimationAdjustment, 通过动画帧来决定播放时段,  <位置偏移是基于脚底的>
	* @param DataAnimation      参考RootMotion数据的动画, 该节点本身不负责播放动画
	* @param TargetFram		    如果小于0那么使用最后一帧
	* 
	*/
	UFUNCTION(BlueprintCallable, Category="RootMotionExtension", meta = (AdvancedDisplay = "7"))
	static bool ApplyRootMotionSource_AnimationAdjustmentByFrame(UCharacterMovementComponent* MovementComponent,
	                                                             USkeletalMeshComponent* Mesh,
	                                                             UAnimSequence* DataAnimation, FName InstanceName,
	                                                             int32 Priority, FVector TargetLocation,
	                                                             bool bLocalTarget, int32 FromFrame = 0,
	                                                             int32 TargetFram = -1, float TimeScale = 1.0f,
	                                                             float AnimWarpingScale = 1.0,
	                                                             ERootMotionAnimWarpingType WarpingType =
		                                                             ERootMotionAnimWarpingType::BasedOnLength,
	                                                             ERootMotionSourceAnimWarpingAxis WarpingAxis =
		                                                             ERootMotionSourceAnimWarpingAxis::XYZ);
	/**
	* 基于ApplyRootMotionSource_AnimationAdjustment, 通过动画时间来决定播放时段,  <位置偏移是基于脚底的>
	* @param DataAnimation      参考RootMotion数据的动画, 该节点本身不负责播放动画
	* @param TargetTime		    如果小于0那么使用动画长度的时间
	* 
	*/
	UFUNCTION(BlueprintCallable, Category="RootMotionExtension", meta = (AdvancedDisplay = "7"))
	static bool ApplyRootMotionSource_AnimationAdjustmentByTime(UCharacterMovementComponent* MovementComponent,
	                                                            USkeletalMeshComponent* Mesh,
	                                                            UAnimSequence* DataAnimation, FName InstanceName,
	                                                            int32 Priority, FVector TargetLocation,
	                                                            bool bLocalTarget, float FromTime = 0,
	                                                            int32 TargetTime = -1, float TimeScale = 1.0f,
	                                                            float AnimWarpingScale = 1.0,
	                                                            ERootMotionAnimWarpingType WarpingType =
		                                                            ERootMotionAnimWarpingType::BasedOnLength,
	                                                            ERootMotionSourceAnimWarpingAxis WarpingAxis =
		                                                            ERootMotionSourceAnimWarpingAxis::XYZ);

	/**
	* 需要配置动画通知窗口, 通过WarpingTarget配置对应窗口的目标点信息,做到分阶段的运动适配,类似MotionWarping
	* <位置偏移是基于脚底的>
	* @param DataAnimation      参考RootMotion数据的动画, 该节点本身不负责播放动画
	* @param WarpingTarget		需要与动画通知严格匹配
	* @param Tolerance	        允许动画通知窗口之间的公差, 小于此值即忽略不计
	* @param AnimWarpingScale   动画信息的缩放, 如果是0代表使用线性位移
	* @param bExcludeEndAnimMotion 排除末尾的动画位移 
	* 
	*/
	UFUNCTION(BlueprintCallable, Category="RootMotionExtension", meta = (AdvancedDisplay = "7"))
	static bool ApplyRootMotionSource_AnimationWarping(UCharacterMovementComponent* MovementComponent,
	                                                   USkeletalMeshComponent* Mesh, UAnimSequence* DataAnimation,
	                                                   TMap<FName, FVector> WarpingTarget, float TimeScale = 1,
	                                                   float Tolerance = 0.01, float AnimWarpingScale = 1.0,
	                                                   bool bExcludeEndAnimMotion = false,
	                                                   ERootMotionSourceAnimWarpingAxis WarpingAxis =
		                                                   ERootMotionSourceAnimWarpingAxis::XYZ);

	static bool GetRootMotionSourceWindow(UAnimSequence* DataAnimation, FName InstanceName,
	                                      FRootMotionSoueceWindowData& Window);
	static bool GetRootMotionSourceWindows(UAnimSequence* DataAnimation, TArray<FRootMotionSoueceWindowData>& Windows);
	static bool GetRootMotionSourceWindowsByInstanceList(UAnimSequence* DataAnimation, TArray<FName> Instances,
	                                                     TArray<FRootMotionSoueceWindowData>& Windows);

	static bool FindTriggerDataByTime(const TArray<FRootMotionSoueceTriggerData>& TriggerData, float Time,
	                                  FRootMotionSoueceTriggerData& OutData);
#pragma endregion Animation

	//模拟力的RootMotion效果,类似AddForce
	UFUNCTION(BlueprintCallable, Category="RootMotionExtension", meta = (AdvancedDisplay = "7"))
	static int32 ApplyRootMotionSource_ConstantForece(UCharacterMovementComponent* MovementComponent,
	                                                  FName InstanceName, ERootMotionAccumulateMode AccumulateMod,
	                                                  int32 Priority, FVector WorldDirection, float Strength,
	                                                  UCurveFloat* StrengthOverTime, float Duration,
	                                                  EFinishVelocityMode VelocityOnFinishMode,
	                                                  FVector FinishSetVelocity, float FinishClampVelocity = 0,
	                                                  bool bEnableGravity = false);
	//模拟范围力的RootMotion效果, 类似AddRadialForce
	UFUNCTION(BlueprintCallable, Category="RootMotionExtension", meta = (AdvancedDisplay = "7"))
	static int32 ApplyRootMotionSource_RadialForece(UCharacterMovementComponent* MovementComponent, FName InstanceName,
	                                                ERootMotionAccumulateMode AccumulateMod, int32 Priority,
	                                                AActor* LocationActor, FVector Location, float Strength,
	                                                float Radius,
	                                                bool bNoZForce, UCurveFloat* StrengthDistanceFalloff,
	                                                UCurveFloat* StrengthOverTime, bool bIsPush, float Duration,
	                                                bool bUseFixedWorldDirection, FRotator FixedWorldDirection,
	                                                EFinishVelocityMode VelocityOnFinishMode,
	                                                FVector FinishSetVelocity, float FinishClampVelocity = 0);

	//刷新动态目标的位置
	UFUNCTION(BlueprintCallable, Category="RootMotionExtension", meta = (AdvancedDisplay = "7"))
	static void UpdateDynamicMoveToTarget(UCharacterMovementComponent* MovementComponent, FName InstanceName,
	                                      FVector NewTarget);
	//移除RMS
	UFUNCTION(BlueprintCallable, Category="RootMotionExtension", meta = (AdvancedDisplay = "7"))
	static void RemoveRootMotionSource(UCharacterMovementComponent* MovementComponent, FName InstanceName);
	/*
	 * 刷新DynamicMoveTo的持续时间
	 * ********此方法有运动突变风险******** 
	*/
	UFUNCTION(BlueprintCallable, Category="RootMotionExtension", meta = (AdvancedDisplay = "7"))
	static void UpdateDynamicMoveDuration(UCharacterMovementComponent* MovementComponent, FName InstanceName,
	                                      float NewDuration);
	//获取RMS的时间信息
	UFUNCTION(BlueprintCallable, Category="RootMotionExtension", meta = (AdvancedDisplay = "7"))
	static void GetCurrentRootMotionSourceTime(UCharacterMovementComponent* MovementComponent, FName InstanceName,
	                                           float& Time, float& Duration);

	UFUNCTION(BlueprintCallable, Category="RootMotionExtension", meta = (AdvancedDisplay = "7"), BlueprintPure)
	static bool IsRootMotionSourceValid(UCharacterMovementComponent* MovementComponent, FName InstanceName);
	UFUNCTION(BlueprintCallable, Category="RootMotionExtension", meta = (AdvancedDisplay = "7"), BlueprintPure)
	static bool IsRootMotionSourceIdValid(UCharacterMovementComponent* MovementComponent, int32 ID);

	static TSharedPtr<FRootMotionSource> GetRootMotionSource(UCharacterMovementComponent* MovementComponent,
	                                                         FName InstanceName);
	static TSharedPtr<FRootMotionSource_MoveToDynamicForce> GetDynamicMoveToRootMotionSource(
		UCharacterMovementComponent* MovementComponent, FName InstanceName);

	static TSharedPtr<FRootMotionSource> GetRootMotionSourceByID(UCharacterMovementComponent* MovementComponent,
	                                                             int32 ID);

	static void CalcAnimWarpingScale(FVector& OriginOffset, ERootMotionAnimWarpingType Type,
	                                 FVector AnimRootMotionLinear, FVector RMSTargetLinearOffset, float Scale = 1,
	                                 float Tolerance = 0.1);

	static void ConvWorldOffsetToRmsSpace(FVector& Offset, FVector Start, FVector Target);

	static void FiltAnimCurveOffsetAxisData(FVector& AnimOffset, ERootMotionSourceAnimWarpingAxis Axis);
};
