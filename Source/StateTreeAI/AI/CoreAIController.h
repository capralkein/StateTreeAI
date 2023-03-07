// Copyright CapralKein. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "CoreAIController.generated.h"

class UStateTreeComponent;

UCLASS()
class STATETREEAI_API ACoreAIController : public AAIController
{
	GENERATED_BODY()
public:
	ACoreAIController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
public:
	UFUNCTION(BlueprintCallable)
	void SetTeamID(uint8 ID) { SetGenericTeamId(ID); }
public:
	static FName StateTreeComponentName;
	static FName AIPerceptionComponentName;
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core|AI", Meta = (AllowPrivateAccess = "true"))
	UStateTreeComponent* StateTreeComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core|AI", Meta = (AllowPrivateAccess = "true"))
	UAIPerceptionComponent* AIPerceptionComponent = nullptr;
};
