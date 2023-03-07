// Copyright CapralKein. All rights reserved.

#include "StateTreeEnvQueryTask.h"
#include "StateTreeExecutionContext.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_ActorBase.h"
#include "StateTreeExtension/Evaluators/StateTreeEnvQueryEvaluator.h"
#include "VisualLogger/VisualLogger.h"


FStateTreeEnvQueryTask::FStateTreeEnvQueryTask()
{
}

EStateTreeRunStatus FStateTreeEnvQueryTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	Reset(Context, InstanceData);
	
	return PerformQueryTask(Context, InstanceData);
}

void FStateTreeEnvQueryTask::ExitState(FStateTreeExecutionContext& Context,	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	Reset(Context, InstanceData);
}

EStateTreeRunStatus FStateTreeEnvQueryTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	if (!InstanceData.ResultData->bFinished)
		return EStateTreeRunStatus::Running;

	InstanceData.bResult = InstanceData.ResultData->bResult;
	if(!InstanceData.bResult)
		return EStateTreeRunStatus::Failed;
	
	InstanceData.ResultActor = InstanceData.ResultData->ResultActor;
	InstanceData.ResultLocation = InstanceData.ResultData->ResultLocation;
	
	UE_VLOG(Context.GetOwner(), LogStateTree, VeryVerbose, TEXT("%s: EnvQuery has finished request id %i (result state: %s)"), *Name.ToString(), InstanceData.RequestId, InstanceData.bResult ? TEXT("true") : TEXT("false"));
	InstanceData.RequestId = INDEX_NONE;

	return EStateTreeRunStatus::Succeeded;
}

EStateTreeRunStatus FStateTreeEnvQueryTask::PerformQueryTask(const FStateTreeExecutionContext& Context,	FInstanceDataType& InstanceData) const
{
	InstanceData.RequestId = INDEX_NONE;
	InstanceData.ResultData->bFinished = false;

	UWorld* World = Context.GetWorld();

	if (!IsValid(InstanceData.QueryTemplate) || !IsValid(InstanceData.Owner))
	{
		UE_VLOG(Context.GetOwner(), LogStateTree, Warning, TEXT("%s: EnvQuery run skipped, invalid query template or owner"), *Name.ToString());
		InstanceData.ResultActor = nullptr;
		InstanceData.ResultLocation = FAISystem::InvalidLocation;

		InstanceData.bResult = false;
		return EStateTreeRunStatus::Failed;
	}

	auto OnQueryCompleted = [NodeName=Name, ResultData=InstanceData.ResultData](TSharedPtr<FEnvQueryResult> QueryResult)
	{
		UE_VLOG(QueryResult->Owner.Get(), LogStateTree, Verbose, TEXT("%s: EnvQuery request completed with %i items"), *NodeName.ToString(), QueryResult->Items.Num());

		ResultData->bFinished = true;
		ResultData->bResult = false;
		
		if (QueryResult.IsValid() || !QueryResult->Items.IsEmpty() || IsValid(QueryResult->ItemType))
			return;

		const uint8* Data = QueryResult->RawData.GetData() + QueryResult->Items[0].DataOffset;

		UEnvQueryItemType* ItemTypeCDO = QueryResult->ItemType->GetDefaultObject<UEnvQueryItemType>();
		if (const UEnvQueryItemType_ActorBase* ActorItemType = Cast<UEnvQueryItemType_ActorBase>(ItemTypeCDO))
		{
			ResultData->ResultActor = ActorItemType->GetActor(Data);
			ResultData->bResult = true;
		}
		else
		{
			ResultData->ResultActor = nullptr;
		}

		if (const UEnvQueryItemType_VectorBase* VectorItemType = Cast<UEnvQueryItemType_VectorBase>(ItemTypeCDO))
		{
			ResultData->ResultLocation = VectorItemType->GetItemLocation(Data);
			ResultData->bResult = true;
		}

		if (!ResultData->bResult)
		{
			UE_VLOG(QueryResult->Owner.Get(), LogStateTree, Warning, TEXT("%s: Failed to retrieve EQS query result (unknown item type %s)"), *NodeName.ToString(), *GetNameSafe(QueryResult->ItemType));
		}	
	};

	FEnvQueryRequest QueryRequest(InstanceData.QueryTemplate, InstanceData.Owner);
	QueryRequest.SetNamedParams(InstanceData.QueryConfig);
	InstanceData.RequestId = QueryRequest.Execute(
		InstanceData.RunMode,
		FQueryFinishedSignature::CreateWeakLambda(World, OnQueryCompleted));

	UE_VLOG(Context.GetOwner(), LogStateTree, Verbose, TEXT("%s: EnvQuery started request id %i"), *Name.ToString(), InstanceData.RequestId);
	
	return EStateTreeRunStatus::Running;
}

void FStateTreeEnvQueryTask::Reset(const FStateTreeExecutionContext& Context, FInstanceDataType& InstanceData) const
{
	if (InstanceData.RequestId >= 0)
	{
		if (UEnvQueryManager* QueryManager = UEnvQueryManager::GetCurrent(Context.GetWorld()))
		{
			QueryManager->AbortQuery(InstanceData.RequestId);
		}
	}

	if (!InstanceData.ResultData.IsValid())
	{
		InstanceData.ResultData = MakeShared<FStateTreeEnvQueryTaskResultData>();
	}

	InstanceData.ResultLocation = FAISystem::InvalidLocation;
	InstanceData.ResultActor = nullptr;
	InstanceData.RequestId = INDEX_NONE;
	InstanceData.bResult = false;
	InstanceData.ResultData->bFinished = false;
	InstanceData.ResultData->bResult = false;
}
