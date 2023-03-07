// Copyright CapralKein. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/Character.h"
#include "CoreCharacter.generated.h"

UCLASS()
class STATETREEAI_API ACoreCharacter : public ACharacter, public IGenericTeamAgentInterface
{
	GENERATED_BODY()
public:
	// IGenericTeamAgentInterface
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const override { return TeamID; }
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;
	//~IGenericTeamAgentInterface

public:
	UFUNCTION(BlueprintCallable, Category = "Core|Character")
	void SetTeamID(uint8 ID) { SetGenericTeamId(ID); }

private:
	FGenericTeamId TeamID;
};
