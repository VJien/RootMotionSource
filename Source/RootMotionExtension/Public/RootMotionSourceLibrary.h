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
	* @param StartLocation      角色会基于此开始移动,所以请确保是Actor当前的Location
	* @param TargetLocation		参考StartLocation的目标位置(要考虑HalfHeight)
	* 
	*/
	UFUNCTION(BlueprintCallable, Category="RootMotionSource",
		meta = (AdvancedDisplay = "6", AutoCreateRefTerm = "ExtraSetting", CPP_Default_ExtraSetting))
	static int32 ApplyRootMotionSource_MoveToForce(UCharacterMovementComponent* MovementComponent, FName InstanceName,
	                                               FVector StartLocation, FVector TargetLocation, float Duration,
	                                               int32 Priority,
	                                               UCurveVector* PathOffsetCurve = nullptr,
	                                               float StartTime = 0,
	                                               ERootMotionSourceApplyMode ApplyMode = ERootMotionSourceApplyMode::None,
	                                               FRootMotionSourceMoveSetting ExtraSetting = {});
	//通过高度和距离适配一个抛物线跳跃运动
	UFUNCTION(BlueprintCallable, Category="RootMotionSource",
		meta = (AdvancedDisplay = "7", AutoCreateRefTerm = "ExtraSetting", CPP_Default_ExtraSetting))
	static int32 ApplyRootMotionSource_JumpForce(UCharacterMovementComponent* MovementComponent, FName InstanceName,
	                                             FRotator Rotation, float Duration, float Distance, float Height,
	                                             int32 Priority,
	                                             UCurveVector* PathOffsetCurve = nullptr,
	                                             UCurveFloat* TimeMappingCurve = nullptr,
	                                             float StartTime = 0,
	                                             ERootMotionSourceApplyMode ApplyMode = ERootMotionSourceApplyMode::None,
	                                             FRootMotionSourceJumpSetting ExtraSetting = {});
	/**
	* 移动到一个动态目标, 需要通过UpdateDynamicMoveToTarget设置目标
	* @param StartLocation      角色会基于此开始移动,所以请确保是Actor当前的Location
	* @param TargetLocation		参考StartLocation的目标位置(要考虑HalfHeight)
	* 
	*/
	UFUNCTION(BlueprintCallable, Category="RootMotionSource",
		meta = (AdvancedDisplay = "6", AutoCreateRefTerm = "ExtraSetting", CPP_Default_ExtraSetting))
	static int32 ApplyRootMotionSource_DynamicMoveToForce(UCharacterMovementComponent* MovementComponent,
	                                                      FName InstanceName, FVector StartLocation,
	                                                      FVector TargetLocation, float Duration, int32 Priority,
	                                                      UCurveVector* PathOffsetCurve = nullptr,
	                                                      UCurveFloat* TimeMappingCurve = nullptr,
	                                                      float StartTime = 0,
	                                                      ERootMotionSourceApplyMode ApplyMode = ERootMotionSourceApplyMode::None,
	                                                      FRootMotionSourceMoveSetting ExtraSetting = {});

	/**
	* 抛物线的形式移动到一个点, 通过一个曲线来设定运动轨迹
	* ParabolaCurve X轴定义时间曲线, Z轴定义抛物线形态曲线
	* @param StartLocation      角色会基于此开始移动,所以请确保是Actor当前的Location
	* @param TargetLocation		参考StartLocation的目标位置(要考虑HalfHeight)
	* @param ParabolaCurve      X轴定义时间曲线, Z轴定义抛物线形态曲线
	* 
	*/
	UFUNCTION(BlueprintCallable, Category="RootMotionSource",
		meta = (AdvancedDisplay = "6", AutoCreateRefTerm = "ExtraSetting", CPP_Default_ExtraSetting))
	static int32 ApplyRootMotionSource_MoveToForce_Parabola(UCharacterMovementComponent* MovementComponent,
	                                                        FName InstanceName,
	                                                        FVector StartLocation, FVector TargetLocation,
	                                                        float Duration,
	                                                        int32 Priority,
	                                                        UCurveVector* ParabolaCurve = nullptr,
	                                                        int32 Segment = 8,
	                                                        float StartTime = 0,
	                                                        ERootMotionSourceApplyMode ApplyMode = ERootMotionSourceApplyMode::None,
	                                                        FRootMotionSourceMoveSetting ExtraSetting = {});


	UFUNCTION(BlueprintCallable, Category="RootMotionSource",
		meta = (AdvancedDisplay = "6", AutoCreateRefTerm = "ExtraSetting", CPP_Default_ExtraSetting))
	static int32 ApplyRootMotionSource_PathMoveToForce(UCharacterMovementComponent* MovementComponent,
	                                                   FName InstanceName,
	                                                   FVector StartLocation, TArray<FRootMotionSourcePathMoveToData> Path,
	                                                   int32 Priority,
	                                                   float StartTime = 0,
	                                                   ERootMotionSourceApplyMode ApplyMode = ERootMotionSourceApplyMode::None,
	                                                   FRootMotionSourceMoveSetting ExtraSetting = {});


#pragma region Animation
	/**
	* 直接使用动画的RootMotion数据,效果等同于播放RootMotion蒙太奇动画
	* @param DataAnimation    参考RootMotion数据的动画, 该节点本身不负责播放动画
	* @param EndTime		  播放的结束时间, 如果小于0,那么就使用最终的动画长度
	* @param TimeScale        动画时间缩放
	* 
	*/
	UFUNCTION(BlueprintCallable, Category="RootMotionSource", meta = (AdvancedDisplay = "5"))
	static bool ApplyRootMotionSource_SimpleAnimation(UCharacterMovementComponent* MovementComponent,
	                                                  USkeletalMeshComponent* Mesh, UAnimSequence* DataAnimation,
	                                                  FName InstanceName, int32 Priority,
	                                                  float StartTime = 0, float EndTime = -1, float TimeScale = 1, ERootMotionSourceApplyMode ApplyMode = ERootMotionSourceApplyMode::None);

	/**
	* 依据动画RootMotion数据适配目标点的运动,效果类似MotionWarping,  
	* <请确认位置是基于脚底还是角色中心>
	* @param DataAnimation      参考RootMotion数据的动画, 该节点本身不负责播放动画
	* @param bLocalTarget		如果为true,那么偏移信息是本地空间的
	* @param bTargetBasedOnFoot 目标位置是基于脚底还是胶囊体中心
	* 
	*/
	UFUNCTION(BlueprintCallable, Category="RootMotionSource", meta = (AdvancedDisplay = "6"))
	static bool ApplyRootMotionSource_AnimationAdjustment(UCharacterMovementComponent* MovementComponent,
	                                                      USkeletalMeshComponent* Mesh, UAnimSequence* DataAnimation,
	                                                      FName InstanceName, int32 Priority, FVector TargetLocation,
	                                                      bool bLocalTarget, bool bTargetBasedOnFoot = true,
	                                                      bool bUseCustomDuration = false,
	                                                      float CustomDuration = 1.0, float AnimWarpingScale = 1.0,
	                                                      ERootMotionAnimWarpingType WarpingType =
		                                                      ERootMotionAnimWarpingType::BasedOnLength,
	                                                      ERootMotionSourceAnimWarpingAxis WarpingAxis =
		                                                      ERootMotionSourceAnimWarpingAxis::XYZ, ERootMotionSourceApplyMode ApplyMode = ERootMotionSourceApplyMode::None);
	/**
	* 基于ApplyRootMotionSource_AnimationAdjustment, 通过动画帧来决定播放时段,  <位置偏移是基于脚底的>
	* @param DataAnimation      参考RootMotion数据的动画, 该节点本身不负责播放动画
	* @param TargetFram		    如果小于0那么使用最后一帧
	* 
	*/
	UFUNCTION(BlueprintCallable, Category="RootMotionSource", meta = (AdvancedDisplay = "6"))
	static bool ApplyRootMotionSource_AnimationAdjustmentByFrame(UCharacterMovementComponent* MovementComponent,
	                                                             USkeletalMeshComponent* Mesh,
	                                                             UAnimSequence* DataAnimation, FName InstanceName,
	                                                             int32 Priority, FVector TargetLocation,
	                                                             bool bLocalTarget, bool bTargetBasedOnFoot = true,
	                                                             int32 FromFrame = 0,
	                                                             int32 TargetFram = -1, float TimeScale = 1.0f,
	                                                             float AnimWarpingScale = 1.0,
	                                                             ERootMotionAnimWarpingType WarpingType =
		                                                             ERootMotionAnimWarpingType::BasedOnLength,
	                                                             ERootMotionSourceAnimWarpingAxis WarpingAxis =
		                                                             ERootMotionSourceAnimWarpingAxis::XYZ, ERootMotionSourceApplyMode ApplyMode = ERootMotionSourceApplyMode::None);
	/**
	* 基于ApplyRootMotionSource_AnimationAdjustment, 通过动画时间来决定播放时段,  <位置偏移是基于脚底的>
	* @param DataAnimation      参考RootMotion数据的动画, 该节点本身不负责播放动画
	* @param TargetTime		    如果小于0那么使用动画长度的时间
	* 
	*/
	UFUNCTION(BlueprintCallable, Category="RootMotionSource", meta = (AdvancedDisplay = "6"))
	static bool ApplyRootMotionSource_AnimationAdjustmentByTime(UCharacterMovementComponent* MovementComponent,
	                                                            USkeletalMeshComponent* Mesh,
	                                                            UAnimSequence* DataAnimation, FName InstanceName,
	                                                            int32 Priority, FVector TargetLocation,
	                                                            bool bLocalTarget, bool bTargetBasedOnFoot = true,
	                                                            float FromTime = 0,
	                                                            int32 TargetTime = -1, float TimeScale = 1.0f,
	                                                            float AnimWarpingScale = 1.0,
	                                                            ERootMotionAnimWarpingType WarpingType =
		                                                            ERootMotionAnimWarpingType::BasedOnLength,
	                                                            ERootMotionSourceAnimWarpingAxis WarpingAxis =
		                                                            ERootMotionSourceAnimWarpingAxis::XYZ, ERootMotionSourceApplyMode ApplyMode = ERootMotionSourceApplyMode::None);

	/**
	* 需要配置动画通知窗口, 通过WarpingTarget配置对应窗口的目标点信息,做到分阶段的运动适配,类似MotionWarping
	* <请确认位置是基于脚底还是角色中心>
	* @param DataAnimation      参考RootMotion数据的动画, 该节点本身不负责播放动画
	* @param WarpingTarget		需要与动画通知严格匹配
	* @param Tolerance	        允许动画通知窗口之间的公差, 小于此值即忽略不计
	* @param AnimWarpingScale   动画信息的缩放, 如果是0代表使用线性位移
	* @param bExcludeEndAnimMotion 排除末尾的动画位移 
	* @param bTargetBasedOnFoot 位置是基于脚底还是胶囊体中心
	*/
	UFUNCTION(BlueprintCallable, Category="RootMotionSource", meta = (AdvancedDisplay = "4"))
	static bool ApplyRootMotionSource_AnimationWarping(UCharacterMovementComponent* MovementComponent,
	                                                   USkeletalMeshComponent* Mesh, UAnimSequence* DataAnimation,
	                                                   TMap<FName, FVector> WarpingTarget, FName InstanceName,
	                                                   int32 Priority,
	                                                   bool bTargetBasedOnFoot = true, float TimeScale = 1,
	                                                   float Tolerance = 0.01, float AnimWarpingScale = 1.0,
	                                                   bool bExcludeEndAnimMotion = false,
	                                                   ERootMotionSourceAnimWarpingAxis WarpingAxis =
		                                                   ERootMotionSourceAnimWarpingAxis::XYZ, ERootMotionSourceApplyMode ApplyMode = ERootMotionSourceApplyMode::None);

	static bool GetRootMotionSourceWindow(UAnimSequence* DataAnimation, FName InstanceName,
	                                      FRootMotionSoueceWindowData& Window);
	static bool GetRootMotionSourceWindows(UAnimSequence* DataAnimation, TArray<FRootMotionSoueceWindowData>& Windows);
	static bool GetRootMotionSourceWindowsByInstanceList(UAnimSequence* DataAnimation, TArray<FName> Instances,
	                                                     TArray<FRootMotionSoueceWindowData>& Windows);

	static bool FindTriggerDataByTime(const TArray<FRootMotionSoueceTriggerData>& TriggerData, float Time,
	                                  FRootMotionSoueceTriggerData& OutData);


	UFUNCTION(BlueprintCallable, Category="RootMotionSource", meta = (AdvancedDisplay = "3"))
	static bool ApplyRootMotionSource_MotionWarping_SimpleAnimation(UCharacterMovementComponent* MovementComponent,
	                                                                UAnimSequence* DataAnimation,
	                                                                FName InstanceName, int32 Priority,
	                                                                float StartTime = 0,
	                                                                float EndTime = -1,
	                                                                float Rate = 1,
	                                                                bool bIgnoreZAxis = false,
	                                                                ERootMotionSourceApplyMode ApplyMode = ERootMotionSourceApplyMode::None);


#pragma endregion Animation

	//模拟力的RootMotion效果,类似AddForce
	UFUNCTION(BlueprintCallable, Category="RootMotionSource", meta = (AdvancedDisplay = "7"))
	static int32 ApplyRootMotionSource_ConstantForece(UCharacterMovementComponent* MovementComponent,
	                                                  FName InstanceName, ERootMotionAccumulateMode AccumulateMod,
	                                                  int32 Priority, FVector WorldDirection, float Strength,
	                                                  UCurveFloat* StrengthOverTime, float Duration,
	                                                  EFinishVelocityMode VelocityOnFinishMode,
	                                                  FVector FinishSetVelocity, float FinishClampVelocity = 0,
	                                                  bool bEnableGravity = false, ERootMotionSourceApplyMode ApplyMode = ERootMotionSourceApplyMode::None);
	//模拟范围力的RootMotion效果, 类似AddRadialForce
	UFUNCTION(BlueprintCallable, Category="RootMotionSource", meta = (AdvancedDisplay = "7"))
	static int32 ApplyRootMotionSource_RadialForece(UCharacterMovementComponent* MovementComponent, FName InstanceName,
	                                                ERootMotionAccumulateMode AccumulateMod, int32 Priority,
	                                                AActor* LocationActor, FVector Location, float Strength,
	                                                float Radius,
	                                                bool bNoZForce, UCurveFloat* StrengthDistanceFalloff,
	                                                UCurveFloat* StrengthOverTime, bool bIsPush, float Duration,
	                                                bool bUseFixedWorldDirection, FRotator FixedWorldDirection,
	                                                EFinishVelocityMode VelocityOnFinishMode,
	                                                FVector FinishSetVelocity, float FinishClampVelocity = 0, ERootMotionSourceApplyMode ApplyMode = ERootMotionSourceApplyMode::None);

	//刷新动态目标的位置
	UFUNCTION(BlueprintCallable, Category="RootMotionSource", meta = (AdvancedDisplay = "7"))
	static void UpdateDynamicMoveToTarget(UCharacterMovementComponent* MovementComponent, FName InstanceName,
	                                      FVector NewTarget);
	//移除RMS
	UFUNCTION(BlueprintCallable, Category="RootMotionSource", meta = (AdvancedDisplay = "7"))
	static void RemoveRootMotionSource(UCharacterMovementComponent* MovementComponent, FName InstanceName);
	/*
	 * 刷新DynamicMoveTo的持续时间
	 * ********此方法有运动突变风险******** 
	*/
	UFUNCTION(BlueprintCallable, Category="RootMotionSource", meta = (AdvancedDisplay = "7"))
	static void UpdateDynamicMoveDuration(UCharacterMovementComponent* MovementComponent, FName InstanceName,
	                                      float NewDuration);
	//获取RMS的时间信息
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="RootMotionSource", meta = (AdvancedDisplay = "7"))
	static void GetCurrentRootMotionSourceTime(UCharacterMovementComponent* MovementComponent, FName InstanceName,
	                                           float& CurrentTime, float& Duration);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="RootMotionSource", meta = (AdvancedDisplay = "7"), BlueprintPure)
	static bool IsRootMotionSourceValid(UCharacterMovementComponent* MovementComponent, FName InstanceName);
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="RootMotionSource", meta = (AdvancedDisplay = "7"), BlueprintPure)
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

	static int32 ExcuteApplyMode(UCharacterMovementComponent* MovementComponent, FName PendingInstanceName, int32 PendingPriorioty, ERootMotionSourceApplyMode ApplyMode);


	static float EvaluateFloatCurveAtFraction(const UCurveFloat& Curve, const float Fraction);
	static FVector EvaluateVectorCurveAtFraction(const UCurveVector& Curve, const float Fraction);
	/*
	 * 根据时间获取正在运行的RMS的实时位置
	 */
	UFUNCTION(BlueprintCallable, Category="RootMotionSource", BlueprintPure)
	static bool GetRootMotionSourceLocation_Runtime(UCharacterMovementComponent* MovementComponent
	                                                , FName InstanceName
	                                                , float CurrentTime
	                                                , FVector& OutLocation);
	/**
	* 根据时间获取MoveTo的位置
	* <位置是角色中心>
	*/
	UFUNCTION(BlueprintCallable, Category="RootMotionSource", BlueprintPure)
	static bool GetRootMotionSourceLocation_MoveTo(FVector& OutLocation, UCharacterMovementComponent* MovementComponent,
	                                               FVector StartLocation, FVector TargetLocation,
	                                               float Duration,
	                                               float CurrentTime, UCurveVector* PathOffsetCurve = nullptr);
	/**
	* 根据时间获取Jump的位置
	* <位置是角色中心>
	*/
	UFUNCTION(BlueprintCallable, Category="RootMotionSource", BlueprintPure)
	static bool GetRootMotionSourceLocation_Jump(FVector& OutLocation, UCharacterMovementComponent* MovementComponent,
	                                             FVector StartLocation, float Distance, float Height, FRotator Rotation,
	                                             float Duration, float CurrentTime, UCurveVector* PathOffsetCurve = nullptr,
	                                             UCurveFloat* TimeMappingCurve = nullptr);
	/**
* 根据时间获取MoveTo的位置
* <位置是角色中心>
*/
	UFUNCTION(BlueprintCallable, Category="RootMotionSource", BlueprintPure)
	static bool GetRootMotionSourceLocation_MoveToParabola(FVector& OutLocation, UCharacterMovementComponent* MovementComponent,
	                                                       FVector StartLocation, FVector TargetLocation,
	                                                       float Duration,
	                                                       float CurrentTime, UCurveVector* ParabolaCurve = nullptr);
};