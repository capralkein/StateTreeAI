// Copyright CapralKein. All rights reserved.


#include "StateTreeExtBlueprintFunctionLibrary.h"

#include "Perception/AIPerceptionComponent.h"

UActorComponent* UStateTreeExtBlueprintFunctionLibrary::FindComponentByClass(const AActor* Actor, TSubclassOf<UActorComponent> ComponentClass)
{
	if (!IsValid(Actor) || !IsValid(ComponentClass))
		return nullptr;

	UActorComponent* Component = Actor->GetComponentByClass(ComponentClass);
	if (IsValid(Component))
		return Component;

	if (const APawn* Pawn = Cast<APawn>(Actor))
	{
		if (const AController* Controller = Pawn->GetController())
		{
			Component = Controller->GetComponentByClass(ComponentClass);
			if (IsValid(Component))
				return Component;
		}
	}

	if (const AController* Controller = Cast<AController>(Actor))
	{
		if (const APawn* Pawn = Controller->GetPawn())
		{
			Component = Pawn->GetComponentByClass(ComponentClass);
			if (IsValid(Component))
				return Component;
		}
	}

	return nullptr;
}
