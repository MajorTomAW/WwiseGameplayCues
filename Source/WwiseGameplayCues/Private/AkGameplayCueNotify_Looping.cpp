// Author: Tom Werner (MajorT), 2026 February


#include "AkGameplayCueNotify_Looping.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(AkGameplayCueNotify_Looping)

AAkGameplayCueNotify_Looping::AAkGameplayCueNotify_Looping(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	PrimaryActorTick.bStartWithTickEnabled = false;
	bAutoDestroyOnRemove = true;
	bAllowMultipleWhileActiveEvents = false;
	NumPreallocatedInstances = 3;

	DefaultPlacementInfo.AttachPolicy = EGameplayCueNotify_AttachPolicy::AttachToTarget;

	Recycle();
}

bool AAkGameplayCueNotify_Looping::Recycle()
{
	Super::Recycle();

	// Extra check to make sure looping effects have been removed.  Normally they will have been removed in the OnRemove event.
	RemoveLoopingEffects();

	ApplicationSpawnResults.Reset();
	LoopingSpawnResults.Reset();
	RecurringSpawnResults.Reset();
	RemovalSpawnResults.Reset();

	bLoopingEffectsRemoved = true;

	return true;
}

bool AAkGameplayCueNotify_Looping::OnActive_Implementation(
	AActor* MyTarget,
	const FGameplayCueParameters& Parameters)
{
	UWorld* World = GetWorld();

	FGameplayCueNotify_SpawnContext SpawnContext(World, MyTarget, Parameters);
	SpawnContext.SetDefaultSpawnCondition(&DefaultSpawnCondition);
	SpawnContext.SetDefaultPlacementInfo(&DefaultPlacementInfo);

	if (DefaultSpawnCondition.ShouldSpawn(SpawnContext))
	{
		ApplicationEffects.ExecuteEffects(SpawnContext, ApplicationSpawnResults);

		OnApplication(MyTarget, Parameters, ApplicationSpawnResults);
	}

	return false;
}

bool AAkGameplayCueNotify_Looping::WhileActive_Implementation(
	AActor* MyTarget,
	const FGameplayCueParameters& Parameters)
{
	UWorld* World = GetWorld();

	FGameplayCueNotify_SpawnContext SpawnContext(World, MyTarget, Parameters);
	SpawnContext.SetDefaultSpawnCondition(&DefaultSpawnCondition);
	SpawnContext.SetDefaultPlacementInfo(&DefaultPlacementInfo);

	if (DefaultSpawnCondition.ShouldSpawn(SpawnContext))
	{
		bLoopingEffectsRemoved = false;
		LoopingEffects.StartEffects(SpawnContext, LoopingSpawnResults);

		OnLoopingStart(MyTarget, Parameters, LoopingSpawnResults);
	}

	return false;
}

bool AAkGameplayCueNotify_Looping::OnExecute_Implementation(
	AActor* MyTarget,
	const FGameplayCueParameters& Parameters)
{
	UWorld* World = GetWorld();

	FGameplayCueNotify_SpawnContext SpawnContext(World, MyTarget, Parameters);
	SpawnContext.SetDefaultSpawnCondition(&DefaultSpawnCondition);
	SpawnContext.SetDefaultPlacementInfo(&DefaultPlacementInfo);

	if (DefaultSpawnCondition.ShouldSpawn(SpawnContext))
	{
		RecurringEffects.ExecuteEffects(SpawnContext, RecurringSpawnResults);

		OnRecurring(MyTarget, Parameters, RecurringSpawnResults);
	}

	return false;
}

bool AAkGameplayCueNotify_Looping::OnRemove_Implementation(
	AActor* MyTarget,
	const FGameplayCueParameters& Parameters)
{
	RemoveLoopingEffects();

	// Don't spawn removal effects if our target is gone
	if (IsValid(MyTarget))
	{
		UWorld* World = GetWorld();

		FGameplayCueNotify_SpawnContext SpawnContext(World, MyTarget, Parameters);
		SpawnContext.SetDefaultSpawnCondition(&DefaultSpawnCondition);
		SpawnContext.SetDefaultPlacementInfo(&DefaultPlacementInfo);

		if (DefaultSpawnCondition.ShouldSpawn(SpawnContext))
		{
			RemovalEffects.ExecuteEffects(SpawnContext, RemovalSpawnResults);
		}
	}

	// Always call OnRemoval(), even if target is bad, so it can clean up BP-spawned things.
	OnRemoval(MyTarget, Parameters, RemovalSpawnResults);

	return false;
}

void AAkGameplayCueNotify_Looping::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (EndPlayReason == EEndPlayReason::Destroyed)
	{
		RemoveLoopingEffects();
	}

	Super::EndPlay(EndPlayReason);
}

#if WITH_EDITOR
EDataValidationResult AAkGameplayCueNotify_Looping::IsDataValid(class FDataValidationContext& Context) const
{
	TArray<FText> ValidationErrors;
	ApplicationEffects.ValidateAssociatedAssets(this, TEXT("ApplicationEffects"), Context);
	LoopingEffects.ValidateAssociatedAssets(this, TEXT("LoopingEffects"), Context);
	RecurringEffects.ValidateAssociatedAssets(this, TEXT("RecurringEffects"), Context);
	RemovalEffects.ValidateAssociatedAssets(this, TEXT("RemovalEffects"), Context);

	return ((ValidationErrors.Num() > 0) ? EDataValidationResult::Invalid : EDataValidationResult::Valid);
}
#endif

void AAkGameplayCueNotify_Looping::RemoveLoopingEffects()
{
	if (bLoopingEffectsRemoved)
	{
		return;
	}

	bLoopingEffectsRemoved = true;
	LoopingEffects.StopEffects(LoopingSpawnResults);
}
