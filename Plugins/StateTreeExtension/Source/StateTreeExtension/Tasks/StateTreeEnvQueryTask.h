// Copyright CapralKein. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AITypes.h"
#include "StateTreeTaskBase.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "StateTreeEnvQueryTask.generated.h"

struct FStateTreeEnvQueryTaskResultData
{
	uint8 bFinished : 1;
	uint8 bResult : 1;
	TWeakObjectPtr<AActor> ResultActor;
	FVector ResultLocation;
};

USTRUCT()
struct STATETREEEXTENSION_API FStateTreeEnvQueryTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Context)
	TObjectPtr<AActor> Owner = nullptr;

	UPROPERTY(EditAnywhere, Category = Parameter)
	TObjectPtr<UEnvQuery> QueryTemplate;

	UPROPERTY(EditAnywhere, Category = Parameter)
	TArray<FEnvNamedValue> QueryConfig;

	UPROPERTY(EditAnywhere, Category = Parameter)
	TEnumAsByte<EEnvQueryRunMode::Type> RunMode = EEnvQueryRunMode::SingleResult;

	UPROPERTY(EditAnywhere, Category = Output)
	FVector ResultLocation = FAISystem::InvalidLocation;

	// TODO: potential for multiple results?
	UPROPERTY(EditAnywhere, Category = Output)
	TWeakObjectPtr<AActor> ResultActor;

	int32 RequestId = INDEX_NONE;

	UPROPERTY(EditAnywhere, Category = Output)
	uint8 bResult : 1;

	TSharedPtr<FStateTreeEnvQueryTaskResultData> ResultData = MakeShared<FStateTreeEnvQueryTaskResultData>();

	double NextUpdate = 0.0;
};


USTRUCT(meta = (DisplayName = "EQS Task"))
struct STATETREEEXTENSION_API FStateTreeEnvQueryTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()
	
	using FInstanceDataType = FStateTreeEnvQueryTaskInstanceData;

	FStateTreeEnvQueryTask();

protected:
	// FStateTreeNodeBase
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	//~FStateTreeNodeBase

	// FStateTreeTaskBase
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	//~FStateTreeTaskBase

private:
	EStateTreeRunStatus PerformQueryTask(const FStateTreeExecutionContext& Context, FInstanceDataType& InstanceData) const;
	void Reset(const FStateTreeExecutionContext& Context, FInstanceDataType& InstanceData) const;

private:
	
	
};