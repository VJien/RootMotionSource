//  Copyright. VJ  All Rights Reserved.
//  https://supervj.top/2022/03/24/RootMotionSource/

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_RMS.generated.h"

/**
 * 
 */
UCLASS()
class RMS_API UAnimNotifyState_RMS_Warping : public UAnimNotifyState
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RootMotionSourceTarget = NAME_None;
};
