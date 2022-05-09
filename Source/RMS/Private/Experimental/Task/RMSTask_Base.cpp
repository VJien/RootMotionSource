//  Copyright. VJ  All Rights Reserved.
//  https://supervj.top/2022/03/24/RootMotionSource/


#include "Experimental/Task/RMSTask_Base.h"


void URMSTask_Base::OnTaskFinished_Implementation(URMSTask_Base* TaskObject,  bool bSuccess)
{
}

URMSTask_Base::URMSTask_Base(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
	bTickingTask = true;
}
