// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RMSTypes.h"
#include "GameFramework/Character.h"
#include "GameFramework/RootMotionSource.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RMSLibrary.generated.h"


class URMSComponent;
class UCurveVector;
enum class ERootMotionAccumulateMode : uint8;
class UCharacterMovementComponent;


UCLASS()
class RMS_API URMSLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	/**
	* 移动到一个点, 一般情况下 StartLocation和TargetLocation都是已角色中心点为参考点
	* 
	* @param StartLocation      角色会基于此开始移动,所以请确保是Actor当前的Location
	* @param TargetLocation		参考StartLocation的目标位置(要考虑HalfHeight)
	* @param RotationSetting	旋转设置, 具体看每个参数自己的注释
	* @param ApplyMode	        RMS的应用模式, 默认是队列, 即存在同名RMS的情况下会在后台执行直到前一个运行完
	* 
	*/
	UFUNCTION(BlueprintCallable, Category="RMS",
		meta = (AdvancedDisplay = "6", AutoCreateRefTerm = "ExtraSetting", CPP_Default_ExtraSetting, CPP_Default_RotationSetting))
	static int32 ApplyRootMotionSource_MoveToForce(UCharacterMovementComponent* MovementComponent, FName InstanceName,
	                                               FVector StartLocation, FVector TargetLocation, float Duration,
	                                               int32 Priority,
	                                               UCurveVector* PathOffsetCurve = nullptr,
	                                               FRMSRotationSetting RotationSetting = {},
	                                               float StartTime = 0,
	                                               ERMSApplyMode ApplyMode =
		                                               ERMSApplyMode::None,
	                                               FRMSSetting_Move ExtraSetting = {});
	//通过高度和距离适配一个抛物线跳跃运动
	UFUNCTION(BlueprintCallable, Category="RMS",
		meta = (AdvancedDisplay = "7", AutoCreateRefTerm = "ExtraSetting", CPP_Default_ExtraSetting))
	static int32 ApplyRootMotionSource_JumpForce(UCharacterMovementComponent* MovementComponent, FName InstanceName,
	                                             FRotator Rotation, float Duration, float Distance, float Height,
	                                             int32 Priority,
	                                             UCurveVector* PathOffsetCurve = nullptr,
	                                             UCurveFloat* TimeMappingCurve = nullptr,
	                                             float StartTime = 0,
	                                             ERMSApplyMode ApplyMode =
		                                             ERMSApplyMode::None,
	                                             FRMSSetting_Jump ExtraSetting = {});
	/**
	* 移动到一个动态目标, 需要通过UpdateDynamicMoveToTarget设置目标
	* @param StartLocation      角色会基于此开始移动,所以请确保是Actor当前的Location
	* @param TargetLocation		参考StartLocation的目标位置(要考虑HalfHeight)
	* 
	*/
	UFUNCTION(BlueprintCallable, Category="RMS",
		meta = (AdvancedDisplay = "6", AutoCreateRefTerm = "ExtraSetting", CPP_Default_ExtraSetting, CPP_Default_RotationSetting))
	static int32 ApplyRootMotionSource_DynamicMoveToForce(UCharacterMovementComponent* MovementComponent,
	                                                      FName InstanceName, FVector StartLocation,
	                                                      FVector TargetLocation, float Duration, int32 Priority,
	                                                      UCurveVector* PathOffsetCurve = nullptr,
	                                                      UCurveFloat* TimeMappingCurve = nullptr,
	                                                      FRMSRotationSetting RotationSetting = {},
	                                                      float StartTime = 0,
	                                                      ERMSApplyMode ApplyMode =
		                                                      ERMSApplyMode::None,
	                                                      FRMSSetting_Move ExtraSetting = {});

	/**
	* 抛物线的形式移动到一个点, 通过一个曲线来设定运动轨迹
	* ParabolaCurve X轴定义时间曲线, Z轴定义抛物线形态曲线
	* @param StartLocation      角色会基于此开始移动,所以请确保是Actor当前的Location
	* @param TargetLocation		参考StartLocation的目标位置(要考虑HalfHeight)
	* @param ParabolaCurve      定义抛物线形态
	* @param RotationSetting	旋转设置, 具体看每个参数自己的注释
	* @param ApplyMode	        RMS的应用模式, 默认是队列, 即存在同名RMS的情况下会在后台执行直到前一个运行完
	* 
	*/
	UFUNCTION(BlueprintCallable, Category="RMS",
		meta = (AdvancedDisplay = "6", AutoCreateRefTerm = "ExtraSetting", CPP_Default_ExtraSetting, CPP_Default_RotationSetting))
	static int32 ApplyRootMotionSource_MoveToForce_Parabola(UCharacterMovementComponent* MovementComponent,
	                                                        FName InstanceName,
	                                                        FVector StartLocation, FVector TargetLocation,
	                                                        float Duration,
	                                                        int32 Priority,
	                                                        UCurveFloat* ParabolaCurve = nullptr,
	                                                        UCurveFloat* TimeMappingCurve = nullptr,
	                                                        FRMSRotationSetting RotationSetting = {},
	                                                        int32 Segment = 8,
	                                                        float StartTime = 0,
	                                                        ERMSApplyMode ApplyMode =
		                                                        ERMSApplyMode::None,
	                                                        FRMSSetting_Move ExtraSetting = {});


	/**
	* 多路径版本的MoveToForce
	* 
	*/
	UFUNCTION(BlueprintCallable, Category="RMS",
		meta = (AdvancedDisplay = "6", AutoCreateRefTerm = "ExtraSetting", CPP_Default_ExtraSetting))
	static int32 ApplyRootMotionSource_PathMoveToForce(UCharacterMovementComponent* MovementComponent,
	                                                   FName InstanceName,
	                                                   FVector StartLocation,
	                                                   FRotator StartRotation,
	                                                   TArray<FRMSPathMoveToData> Path,
	                                                   int32 Priority,
	                                                   float StartTime = 0,
	                                                   ERMSApplyMode ApplyMode =
		                                                   ERMSApplyMode::None,
	                                                   FRMSSetting_Move ExtraSetting = {});


#pragma region Animation

	static bool ApplyRootMotionSource_SimpleAnimation_BM(UCharacterMovementComponent* MovementComponent,
	                                                     UAnimSequence* DataAnimation,
	                                                     FName InstanceName,
	                                                     int32 Priority,
	                                                     float StartTime = 0,
	                                                     float EndTime = -1,
	                                                     float Rate = 1,
	                                                     bool bIgnoreZAxis = false,
	                                                     ERMSApplyMode ApplyMode =
		                                                     ERMSApplyMode::None);

	/**
	* 直接使用动画的RootMotion数据,效果等同于播放RootMotion蒙太奇动画
	* @param DataAnimation    参考RootMotion数据的动画, 该节点本身不负责播放动画
	* @param EndTime		  播放的结束时间, 如果小于0,那么就使用最终的动画长度
	* @param bIgnoreZAxis	  忽略Z方向的RootMotion
	* @param ApplyMode	        RMS的应用模式, 默认是队列, 即存在同名RMS的情况下会在后台执行直到前一个运行完
	*/
	UFUNCTION(BlueprintCallable, Category="RMS|Animation", meta = (AdvancedDisplay = "3"))
	static bool ApplyRootMotionSource_SimpleAnimation(UCharacterMovementComponent* MovementComponent,
	                                                  UAnimSequence* DataAnimation,
	                                                  FName InstanceName,
	                                                  int32 Priority,
	                                                  float StartTime = 0,
	                                                  float EndTime = -1,
	                                                  float Rate = 1,
	                                                  bool bIgnoreZAxis = false,
	                                                  bool bUseForwardCalculation = false,
	                                                  ERMSApplyMode ApplyMode =
		                                                  ERMSApplyMode::None);

	
	static bool ApplyRootMotionSource_AnimationAdjustment_BM(UCharacterMovementComponent* MovementComponent,
	                                                         UAnimSequence* DataAnimation,
	                                                         FName InstanceName,
	                                                         int32 Priority,
	                                                         FVector TargetLocation,
	                                                         bool bLocalTarget,
	                                                         bool bTargetBasedOnFoot = true,
	                                                         float StartTime = 0,
	                                                         float EndTime = -1,
	                                                         float Rate = 1.0,
	                                                         FRMSRotationSetting RotationSetting = {},
	                                                         ERMSApplyMode ApplyMode = ERMSApplyMode::None);

	/**
	* 依据动画RootMotion数据适配目标点的运动,效果类似MotionWarping, 只需要设置一个最终的目标位置
	* <请确认位置是基于脚底还是角色中心>
	* @param DataAnimation      参考RootMotion数据的动画, 该节点本身不负责播放动画
	* @param bLocalTarget		如果为true,那么偏移信息是本地空间的
	* @param bTargetBasedOnFoot 目标位置是基于脚底还是胶囊体中心
	* @param StartTime			开始时间, 即动画数据计算的开始时间
	* @param EndTime			动画数据计算结束的时间, 小于0意味着使用整个动画时长
	* @param Rate				动画速率,同时也会影响整个RMS的速度,如动画时长1秒, Rate=2, 那么整个RMS时长就是0.5秒
	* @param TargetLocation		最终的目标位置, 整个RootMotion运动会去适配这个位置
	* @param RotationSetting	旋转设置, 具体看每个参数自己的注释
	* @param bUseForwardCalculation	前向计算, 会把数据提前烘培到Curve上, Runtime开销会减少, 但是精确度略低
	* @param ApplyMode	        RMS的应用模式, 默认是队列, 即存在同名RMS的情况下会在后台执行直到前一个运行完
	*/
	UFUNCTION(BlueprintCallable, Category="RMS|Animation", meta = (AdvancedDisplay = "6",CPP_Default_RotationSetting))
	static bool ApplyRootMotionSource_AnimationAdjustment(UCharacterMovementComponent* MovementComponent,
	                                                      UAnimSequence* DataAnimation,
	                                                      FName InstanceName,
	                                                      int32 Priority,
	                                                      FVector TargetLocation,
	                                                      bool bLocalTarget,
	                                                      bool bTargetBasedOnFoot = true,
	                                                      float StartTime = 0,
	                                                      float EndTime = -1.0,
	                                                      float Rate = 1.0,
	                                                      FRMSRotationSetting RotationSetting = {},
	                                                      bool bUseForwardCalculation = false,
	                                                      ERMSApplyMode ApplyMode =
		                                                      ERMSApplyMode::None);



	/**
	 ** BM: BasedOnMoveTo, 底层计算基于MoveTo, 开销会比非BM小,但是相对不太稳定
	* 需要配置动画通知窗口, 通过WarpingTarget配置对应窗口的目标点信息,做到分阶段的运动适配,类似MotionWarping
	* <请确认位置是基于脚底还是角色中心>
	* @param DataAnimation      参考RootMotion数据的动画, 该节点本身不负责播放动画
	* @param WarpingTarget		需要与动画通知严格匹配
	* @param Tolerance	        允许动画通知窗口之间的公差, 小于此值即忽略不计
	* @param AnimWarpingScale   动画信息的缩放, 如果是0代表使用线性位移
	* @param bExcludeEndAnimMotion 排除末尾的动画位移 
	* @param bTargetBasedOnFoot 位置是基于脚底还是胶囊体中心
	*/
	UFUNCTION(BlueprintCallable, Category="RMS|Animation|BasedOnMoveTo", meta = (AdvancedDisplay = "4"))
	static bool ApplyRootMotionSource_AnimationWarping_BM(UCharacterMovementComponent* MovementComponent,
	                                                      UAnimSequence* DataAnimation,
	                                                      TMap<FName, FVector> WarpingTarget, FName InstanceName,
	                                                      int32 Priority,
	                                                      bool bTargetBasedOnFoot = true, float Rate = 1,
	                                                      float Tolerance = 0.01, float AnimWarpingScale = 1.0,
	                                                      bool bExcludeEndAnimMotion = false,
	                                                      ERMSAnimWarpingAxis WarpingAxis =
		                                                      ERMSAnimWarpingAxis::XYZ,
	                                                      ERMSApplyMode ApplyMode =
		                                                      ERMSApplyMode::None);

	/**
 **
* 需要配置动画通知窗口, 通过WarpingTarget配置对应窗口的目标点信息,做到分阶段的运动适配,类似MotionWarping
* <注意: 此RMS是基于当前实时位置计算, 所以即使使用新的RMS覆盖旧的也会从当前实时位置继续执行而非跳转到之前的开始位置>
* <请确认位置是基于脚底还是角色中心>
* @param DataAnimation      参考RootMotion数据的动画, 该节点本身不负责播放动画
* @param WarpingTarget		需要与动画通知严格匹配
* @param Tolerance	        允许动画通知窗口之间的公差, 小于此值即忽略不计
* @param StartTime			开始时间, 即动画数据计算的开始时间
* @param Rate				动画速率,同时也会影响整个RMS的速度,如动画时长1秒, Rate=2, 那么整个RMS时长就是0.5秒
* @param bTargetBasedOnFoot 位置是基于脚底还是胶囊体中心
*/
	UFUNCTION(BlueprintCallable, Category="RMS|Animation", meta = (AdvancedDisplay = "4"))
	static bool ApplyRootMotionSource_AnimationWarping(UCharacterMovementComponent* MovementComponent,
														  UAnimSequence* DataAnimation,
														  TMap<FName, FVector> WarpingTarget,
														  float StartTime,
														  FName InstanceName,
														  int32 Priority,
														  bool bTargetBasedOnFoot = true,
														  float Rate = 1,
														  float Tolerance = 0.01, 
														  ERMSApplyMode ApplyMode =
															  ERMSApplyMode::None);

	static bool GetRootMotionSourceWindow(UAnimSequence* DataAnimation, FName InstanceName,
	                                      FRMSWindowData& Window);
	static bool GetRootMotionSourceWindows(UAnimSequence* DataAnimation, TArray<FRMSWindowData>& Windows);
	static bool GetRootMotionSourceWindowsByInstanceList(UAnimSequence* DataAnimation, TArray<FName> Instances,
	                                                     TArray<FRMSWindowData>& Windows);

	static bool FindTriggerDataByTime(const TArray<FRMSNotifyTriggerData>& TriggerData, float Time,
	                                  FRMSNotifyTriggerData& OutData);

	UFUNCTION(BlueprintCallable, BlueprintPure,Category="RMS", meta = (AdvancedDisplay = "7"))
	static FTransform ExtractRootMotion(UAnimSequenceBase* Anim , float StartTime, float EndTime);

#pragma endregion Animation

	//模拟力的RootMotion效果,类似AddForce
	UFUNCTION(BlueprintCallable, Category="RMS", meta = (AdvancedDisplay = "7"))
	static int32 ApplyRootMotionSource_ConstantForece(UCharacterMovementComponent* MovementComponent,
	                                                  FName InstanceName, ERootMotionAccumulateMode AccumulateMod,
	                                                  int32 Priority, FVector WorldDirection, float Strength,
	                                                  UCurveFloat* StrengthOverTime, float Duration,
	                                                  ERMSFinishVelocityMode VelocityOnFinishMode,
	                                                  FVector FinishSetVelocity, float FinishClampVelocity = 0,
	                                                  bool bEnableGravity = false,
	                                                  ERMSApplyMode ApplyMode =
		                                                  ERMSApplyMode::None);
	//模拟范围力的RootMotion效果, 类似AddRadialForce
	UFUNCTION(BlueprintCallable, Category="RMS", meta = (AdvancedDisplay = "7"))
	static int32 ApplyRootMotionSource_RadialForece(UCharacterMovementComponent* MovementComponent, FName InstanceName,
	                                                ERootMotionAccumulateMode AccumulateMod, int32 Priority,
	                                                AActor* LocationActor, FVector Location, float Strength,
	                                                float Radius,
	                                                bool bNoZForce, UCurveFloat* StrengthDistanceFalloff,
	                                                UCurveFloat* StrengthOverTime, bool bIsPush, float Duration,
	                                                bool bUseFixedWorldDirection, FRotator FixedWorldDirection,
	                                                ERMSFinishVelocityMode VelocityOnFinishMode,
	                                                FVector FinishSetVelocity, float FinishClampVelocity = 0,
	                                                ERMSApplyMode ApplyMode =
		                                                ERMSApplyMode::None);

	//刷新动态目标的位置
	UFUNCTION(BlueprintCallable, Category="RMS", meta = (AdvancedDisplay = "7"))
	static void UpdateDynamicMoveToTarget(UCharacterMovementComponent* MovementComponent, FName InstanceName,
	                                      FVector NewTarget);
	/*
	 * 刷新DynamicMoveTo的持续时间
	 * 新的时间不能小于当前已经运行的时间
	*/
	UFUNCTION(BlueprintCallable, Category="RMS", meta = (AdvancedDisplay = "7"))
	static void UpdateDynamicMoveDuration(UCharacterMovementComponent* MovementComponent, FName InstanceName,
										  float NewDuration);
	
	//移除RMS
	UFUNCTION(BlueprintCallable, Category="RMS", meta = (AdvancedDisplay = "7"))
	static void RemoveRootMotionSource(UCharacterMovementComponent* MovementComponent, FName InstanceName);
	
	//获取RMS的时间信息
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="RMS", meta = (AdvancedDisplay = "7"))
	static void GetCurrentRootMotionSourceTime(UCharacterMovementComponent* MovementComponent, FName InstanceName,
	                                           float& CurrentTime, float& Duration);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="RMS", meta = (AdvancedDisplay = "7"),
		BlueprintPure)
	static bool IsRootMotionSourceValid(UCharacterMovementComponent* MovementComponent, FName InstanceName);
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="RMS", meta = (AdvancedDisplay = "7"),
		BlueprintPure)
	static bool IsRootMotionSourceIdValid(UCharacterMovementComponent* MovementComponent, int32 ID);

	static TSharedPtr<FRootMotionSource> GetRootMotionSource(UCharacterMovementComponent* MovementComponent,
	                                                         FName InstanceName);
	static TSharedPtr<FRootMotionSource_MoveToDynamicForce> GetDynamicMoveToRootMotionSource(
		UCharacterMovementComponent* MovementComponent, FName InstanceName);

	static TSharedPtr<FRootMotionSource> GetRootMotionSourceByID(UCharacterMovementComponent* MovementComponent,
	                                                             int32 ID);

	static void CalcAnimWarpingScale(FVector& OriginOffset, ERMSAnimWarpingType Type,
	                                 FVector AnimRootMotionLinear, FVector RMSTargetLinearOffset, float Scale = 1,
	                                 float Tolerance = 0.1);

	static void ConvWorldOffsetToRmsSpace(FVector& Offset, FVector Start, FVector Target);

	static void FiltAnimCurveOffsetAxisData(FVector& AnimOffset, ERMSAnimWarpingAxis Axis);

	static int32 CalcPriorityByApplyMode(UCharacterMovementComponent* MovementComponent, FName PendingInstanceName,
	                                     int32 PendingPriorioty, ERMSApplyMode ApplyMode);

	static bool ExtractRotation(FRotator& OutRotation, const ACharacter& Character, FRotator StartRotation, FRotator TargetRotation, float Fraction, UCurveFloat* RotationCurve = nullptr);

	static float EvaluateFloatCurveAtFraction(const UCurveFloat& Curve, const float Fraction);
	static FVector EvaluateVectorCurveAtFraction(const UCurveVector& Curve, const float Fraction);
	/*
	 * 根据时间预测正在运行的RMS的实时位置
	 */
	UFUNCTION(BlueprintCallable, Category="RMS", BlueprintPure)
	static bool PredictRootMotionSourceLocation_Runtime(UCharacterMovementComponent* MovementComponent
	                                                , FName InstanceName
	                                                , float CurrentTime
	                                                , FVector& OutLocation);
	/**
	* 根据时间预测MoveTo的位置
	* <位置是角色中心>
	*/
	UFUNCTION(BlueprintCallable, Category="RMS", BlueprintPure)
	static bool PredictRootMotionSourceLocation_MoveTo(FVector& OutLocation, UCharacterMovementComponent* MovementComponent,
	                                               FVector StartLocation, FVector TargetLocation,
	                                               float Duration,
	                                               float CurrentTime, UCurveVector* PathOffsetCurve = nullptr);
	/**
	* 根据时间预测Jump的位置
	* <位置是角色中心>
	*/
	UFUNCTION(BlueprintCallable, Category="RMS", BlueprintPure)
	static bool PredictRootMotionSourceLocation_Jump(FVector& OutLocation, UCharacterMovementComponent* MovementComponent,
	                                             FVector StartLocation, float Distance, float Height, FRotator Rotation,
	                                             float Duration, float CurrentTime,
	                                             UCurveVector* PathOffsetCurve = nullptr,
	                                             UCurveFloat* TimeMappingCurve = nullptr);
	/**
* 根据时间预测位置
* <位置是角色中心>
*/
	UFUNCTION(BlueprintCallable, Category="RMS", BlueprintPure)
	static bool PredictRootMotionSourceLocation_MoveToParabola(FVector& OutLocation,
	                                                       UCharacterMovementComponent* MovementComponent,
	                                                       FVector StartLocation, FVector TargetLocation,
	                                                       float Duration,
	                                                       float CurrentTime, UCurveFloat* ParabolaCurve = nullptr, UCurveFloat* TimeMappingCurve = nullptr);




};


