// Author: Tom Werner (MajorT), 2026 February


#include "AkGameplayCueNotify_BurstLatent.h"

#include "AkComponent.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif


#include UE_INLINE_GENERATED_CPP_BY_NAME(AkGameplayCueNotify_BurstLatent)

const float DefaultBurstLatentLifetime = 5.0f;

AAkGameplayCueNotify_BurstLatent::AAkGameplayCueNotify_BurstLatent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	RootComponent = CreateDefaultSubobject<USceneComponent>("RootComponent");

	PrimaryActorTick.bStartWithTickEnabled = false;
	bAutoDestroyOnRemove = true;
	NumPreallocatedInstances = 3;

	Recycle();
}

bool AAkGameplayCueNotify_BurstLatent::Recycle()
{
	Super::Recycle();
	BurstSpawnResults.Reset();
	return true;
}

bool AAkGameplayCueNotify_BurstLatent::OnExecute_Implementation(
	AActor* MyTarget,
	const FGameplayCueParameters& Parameters)
{
	UWorld* World = GetWorld();

	FGameplayCueNotify_SpawnContext SpawnContext(World, MyTarget, Parameters);
	SpawnContext.SetDefaultSpawnCondition(&DefaultSpawnCondition);
	SpawnContext.SetDefaultPlacementInfo(&DefaultPlacementInfo);

	if (DefaultSpawnCondition.ShouldSpawn(SpawnContext))
	{
		BurstEffects.ExecuteEffects(SpawnContext, BurstSpawnResults);
		OnBurst(MyTarget, Parameters, BurstSpawnResults);
	}

	// Handle GC removal by default. This is a simple default to handle all cases we can currently think of.
	// If we didn't do this, we'd be relying on every BurstLatent GC manually setting up its removal within BP graphs,
	// or some inference based on parameters.
	if (World)
	{
		const float Lifetime = FMath::Max<float>(AutoDestroyDelay, DefaultBurstLatentLifetime);
		World->GetTimerManager().SetTimer(FinishTimerHandle, this, &AGameplayCueNotify_Actor::GameplayCueFinishedCallback, Lifetime);
	}

	return false;
}

#if WITH_EDITOR
EDataValidationResult AAkGameplayCueNotify_BurstLatent::IsDataValid(class FDataValidationContext& Context) const
{
	BurstEffects.ValidateAssociatedAssets(this, TEXT("BurstEffects"), Context);

	return ((Context.GetNumErrors() > 0) ? EDataValidationResult::Invalid : EDataValidationResult::Valid);
}
#endif
