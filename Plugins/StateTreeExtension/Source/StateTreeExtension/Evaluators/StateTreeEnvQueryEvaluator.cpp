#include "StateTreeEnvQueryEvaluator.h"
#include "AIController.h"
#include "StateTreeExecutionContext.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_ActorBase.h"
#include "VisualLogger/VisualLogger.h"

FStateTreeEnvQueryEvaluator::FStateTreeEnvQueryEvaluator()
{
	bClearResultsOnQueryFailure = true;
}

void FStateTreeEnvQueryEvaluator::TreeStart(FStateTreeExecutionContext& Context) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	Reset(Context, InstanceData);
}

void FStateTreeEnvQueryEvaluator::TreeStop(FStateTreeExecutionContext& Context) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	Reset(Context, InstanceData);
}

void FStateTreeEnvQueryEvaluator::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	const UWorld* World = Context.GetWorld();
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	if (InstanceData.ResultData->bFinished)
	{
		if (ResultUpdatedEventTag.IsValid())
		{
			if (InstanceData.ResultActor != InstanceData.ResultData->ResultActor || (!InstanceData.ResultActor.IsValid() && InstanceData.ResultLocation != InstanceData.ResultData->ResultLocation))
			{
				UE_VLOG(Context.GetOwner(), LogStateTree, Verbose, TEXT("%s: EnvQuery result has changed, emitting event %s"), *Name.ToString(), *ResultUpdatedEventTag.ToString());
				Context.SendEvent(ResultUpdatedEventTag);
			}
		}

		InstanceData.ResultActor = InstanceData.ResultData->ResultActor;
		InstanceData.ResultLocation = InstanceData.ResultData->ResultLocation;
		InstanceData.bResult = InstanceData.ResultData->bResult;
		InstanceData.ResultData->bFinished = false;

		InstanceData.NextUpdate = World->GetTimeSeconds() + (InstanceData.bResult ? TickInterval : RetryCooldown);

		UE_VLOG(Context.GetOwner(), LogStateTree, VeryVerbose, TEXT("%s: EnvQuery has finished request id %i (result state: %s)"), *Name.ToString(), InstanceData.RequestId, InstanceData.bResult ? TEXT("true") : TEXT("false"));
		InstanceData.RequestId = INDEX_NONE;
	}

	if (InstanceData.RequestId >= 0 || InstanceData.NextUpdate > World->GetTimeSeconds())
		return;

	InstanceData.NextUpdate = 0.0;
	RunQuery(Context, InstanceData);
}

void FStateTreeEnvQueryEvaluator::Reset(FStateTreeExecutionContext& Context, FInstanceDataType& InstanceData) const
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
		InstanceData.ResultData = MakeShared<FStateTreeEnvQueryEvaluatorResultData>();
	}

	InstanceData.ResultLocation = FAISystem::InvalidLocation;
	InstanceData.ResultActor = nullptr;
	InstanceData.RequestId = INDEX_NONE;
	InstanceData.bResult = false;
	InstanceData.ResultData->bFinished = false;
	InstanceData.ResultData->bResult = false;
	// Don't reset NextUpdate
}

void FStateTreeEnvQueryEvaluator::RunQuery(FStateTreeExecutionContext& Context, FInstanceDataType& InstanceData) const
{
	InstanceData.RequestId = INDEX_NONE;
	InstanceData.ResultData->bFinished = false;

	UWorld* World = Context.GetWorld();

	if (!IsValid(InstanceData.QueryTemplate) || !IsValid(InstanceData.Owner))
	{
		UE_VLOG(Context.GetOwner(), LogStateTree, Warning, TEXT("%s: EnvQuery run skipped, invalid query template or owner"), *Name.ToString());
		InstanceData.NextUpdate = World->GetTimeSeconds() + RetryCooldown;

		if (bClearResultsOnQueryFailure)
		{
			InstanceData.ResultActor = nullptr;
			InstanceData.ResultLocation = FAISystem::InvalidLocation;
		}

		InstanceData.bResult = false;
		return;
	}

	auto OnQueryCompleted = [NodeName=Name, ResultData=InstanceData.ResultData](TSharedPtr<FEnvQueryResult> QueryResult)
	{
		UE_VLOG(QueryResult->Owner.Get(), LogStateTree, Verbose, TEXT("%s: EnvQuery request completed with %i items"), *NodeName.ToString(), QueryResult->Items.Num());

		ResultData->bFinished = true;
		ResultData->bResult = false;
		if (QueryResult.IsValid() && !QueryResult->Items.IsEmpty() && IsValid(QueryResult->ItemType))
		{
			uint8* Data = QueryResult->RawData.GetData() + QueryResult->Items[0].DataOffset;

			UEnvQueryItemType* ItemTypeCDO = QueryResult->ItemType->GetDefaultObject<UEnvQueryItemType>();
			bool bSetActor = false;
			if (UEnvQueryItemType_ActorBase* ActorItemType = Cast<UEnvQueryItemType_ActorBase>(ItemTypeCDO))
			{
				ResultData->ResultActor = ActorItemType->GetActor(Data);
				ResultData->bResult = true;
			}
			else
			{
				ResultData->ResultActor = nullptr;
			}

			if (UEnvQueryItemType_VectorBase* VectorItemType = Cast<UEnvQueryItemType_VectorBase>(ItemTypeCDO))
			{
				ResultData->ResultLocation = VectorItemType->GetItemLocation(Data);
				ResultData->bResult = true;
			}

			if (!ResultData->bResult)
			{
				UE_VLOG(QueryResult->Owner.Get(), LogStateTree, Warning, TEXT("%s: Failed to retrieve EQS query result (unknown item type %s)"), *NodeName.ToString(), *GetNameSafe(QueryResult->ItemType));
			}
		}
	};

	FEnvQueryRequest QueryRequest(InstanceData.QueryTemplate, InstanceData.Owner);
	QueryRequest.SetNamedParams(InstanceData.QueryConfig);
	InstanceData.RequestId = QueryRequest.Execute(
		RunMode,
		FQueryFinishedSignature::CreateWeakLambda(World, OnQueryCompleted));

	UE_VLOG(Context.GetOwner(), LogStateTree, Verbose, TEXT("%s: EnvQuery started request id %i"), *Name.ToString(), InstanceData.RequestId);

	if (InstanceData.RequestId < 0)
	{
		InstanceData.NextUpdate = World->GetTimeSeconds() + RetryCooldown;
	}
}