// Author: Tom Werner (MajorT), 2026 February

#pragma once

#include "CoreMinimal.h"
#include "AkGameplayCueTypes.h"
#include "GameplayCueNotifyTypes.h"
#include "GameplayCueNotify_Static.h"

#include "AkGameplayCueNotify_Burst.generated.h"

#define UE_API WWISEGAMEPLAYCUES_API

/**
 * UAgGameplayCueNotify_Burst
 *
 *	This is a non-instanced gameplay cue notify for effects that are one-offs.
 *	Since it is not instanced, it cannot do latent actions such as delays and timelines.
 *
 *	Supporting Ak (Wwise) audio events.
 */
UCLASS(Blueprintable, Category="GameplayCueNotify", MinimalAPI, meta=(ShowWorldContextPin, DisplayName="Ak GCN Burst", ShortTooltip="A one-off GameplayCueNotify that is never spawned into the world."))
class UAkGameplayCueNotify_Burst : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:
	UE_API UAkGameplayCueNotify_Burst();

protected:
	//~ Begin UGameplayCueNotify_Static Interface
	UE_API virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;

#if WITH_EDITOR
	UE_API virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
	//~ End UGameplayCueNotify_Static Interface

	UFUNCTION(BlueprintImplementableEvent)
	UE_API void OnBurst(AActor* Target, const FGameplayCueParameters& Parameters, const FAkGameplayCueNotify_SpawnResult& SpawnResult) const;

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
};

#undef UE_API
