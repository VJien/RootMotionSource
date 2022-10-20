//  Copyright. VJ  All Rights Reserved.
//  https://supervj.top/2022/03/24/RootMotionSource/

#pragma once

#include "CoreMinimal.h"
#include "RMSLibrary.h"
#include "RMSTask_Base.h"
#include "RMSTask_AnimWarping.generated.h"


class URMSComponent;
UCLASS()
class RMS_API URMSTask_AnimWarping : public URMSTask_Base
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category=RootMotionSource, meta = (BlueprintInternalUseOnly = "TRUE"))
	static URMSTask_AnimWarping* RootMotionSourceTask_AnimWarping(URMSComponent* RootMotionComponent, UAnimSequence*Anim,  TMap<FName, FVector> WarpingInfo);

	
	virtual void Activate() override;
	virtual void Pause() override;
	virtual void Resume() override;
	virtual void TickTask(float DeltaTime) override;

	virtual  void OnTaskFinished_Implementation(URMSTask_Base* TaskObject,  bool bSuccess) override;;
	



	UPROPERTY()
	TWeakObjectPtr<UCharacterMovementComponent> MovementComp = nullptr;

	TMap<FName, FVector> WarpingInfo;
	TWeakObjectPtr<UAnimSequence> Anim = nullptr;

};
