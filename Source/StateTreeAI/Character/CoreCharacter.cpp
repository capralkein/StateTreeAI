// Copyright CapralKein. All rights reserved.

#include "CoreCharacter.h"

void ACoreCharacter::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	if (TeamID != NewTeamID)
	{
		TeamID = NewTeamID;
	}
}

ETeamAttitude::Type ACoreCharacter::GetTeamAttitudeTowards(const AActor& Other) const
{
	return IGenericTeamAgentInterface::GetTeamAttitudeTowards(Other);
}
