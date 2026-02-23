// Author: Tom Werner (MajorT), 2026 February

#pragma once

#include "CoreMinimal.h"
#include "AkGameplayCueTypes.h"
#include "GameplayCueNotifyTypes.h"
#include "GameplayCueNotify_Actor.h"

#include "AkGameplayCueNotify_BurstLatent.generated.h"

#define UE_API WWISEGAMEPLAYCUES_API

struct FAkGameplayCueNotify_SpawnResult;

/**
 * AAkGameplayCueNotify_BurstLatent
 *
 *	This is an instanced gameplay cue notify for effects that are one-offs.
 *	Since it is instanced, it can do latent things like timelines or delays.
 *
 *	Supporting Ak (Wwise) audio events.
 */
UCLASS(Blueprintable, NotPlaceable, Category="GameplayCueNotify", MinimalAPI, meta=(ShowWorldContextPin, DisplayName="Ak GCN Burst Latent", ShortTooltip = "A one-off GameplayCueNotify that can use latent actions such as timelines."))
class AAkGameplayCueNotify_BurstLatent : public AGameplayCueNotify_Actor
{
	GENERATED_BODY()

public:
	UE_API AAkGameplayCueNotify_BurstLatent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	//~ Begin AGameplayCueNotify_Actor Interface
	UE_API virtual bool Recycle() override;
	UE_API virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;

#if WITH_EDITOR
	UE_API virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
	//~ End AGameplayCueNotify_Actor Interface

	UFUNCTION(BlueprintImplementableEvent)
	UE_API void OnBurst(AActor* Target, const FGameplayCueParameters& Parameters, const FAkGameplayCueNotify_SpawnResult& SpawnResults);

protected:
	/** Default condition to check before spawning anything.  Applies for all spawns unless overridden. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GCN Defaults")
	FGameplayCueNotify_SpawnCondition DefaultSpawnCondition;

	/** Default placement rules.  Applies for all spawns unless overridden. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GCN Defaults")
	FGameplayCueNotify_PlacementInfo DefaultPlacementInfo;

	/** List of effects to spawn on burst. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GCN Effects")
	FAkGameplayCueNotify_BurstEffects BurstEffects;

	/** Results of spawned burst effects. */
	UPROPERTY(BlueprintReadOnly, Category = "GCN Effects")
	FAkGameplayCueNotify_SpawnResult BurstSpawnResults;
};

#undef UE_API
