// Copyright CapralKein. All rights reserved.

#include "CoreAIController.h"

#include "Components/StateTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"

FName ACoreAIController::StateTreeComponentName(TEXT("StateTreeComponent"));
FName ACoreAIController::AIPerceptionComponentName(TEXT("AIPerceptionComponent"));

ACoreAIController::ACoreAIController(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	StateTreeComponent = CreateOptionalDefaultSubobject<UStateTreeComponent>(StateTreeComponentName);
	AIPerceptionComponent = CreateOptionalDefaultSubobject<UAIPerceptionComponent>(AIPerceptionComponentName);
}
