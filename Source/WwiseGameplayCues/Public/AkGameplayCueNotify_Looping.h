/** Author: Tom Werner (MajorT), 2026 February */

#pragma once

#include "CoreMinimal.h"
#include "AkGameplayCueTypes.h"
#include "GameplayCueNotifyTypes.h"
#include "GameplayCueNotify_Actor.h"

#include "AkGameplayCueNotify_Looping.generated.h"

#define UE_API WWISEGAMEPLAYCUES_API

struct FAkGameplayCueNotify_SpawnResult;
/**
 * AAkGameplayCueNotify_Looping
 *
 *	This is an instanced gameplay cue notify for continuous looping effects.
 *	The game is responsible for defining the start/stop by adding/removing the gameplay cue.
 *
 *	Supporting Ak (Wwise) audio events.
 */
UCLASS(Blueprintable, NotPlaceable, Category="GameplayCueNotify", MinimalAPI, meta=(ShowWorldContextPin, DisplayName="Ak GCN Looping", ShortTooltip = "A GameplayCueNotify that has a duration that is driven by the game."))
class AAkGameplayCueNotify_Looping : public AGameplayCueNotify_Actor
{
	GENERATED_BODY()

public:
	UE_API AAkGameplayCueNotify_Looping(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	/** ~ Begin AGameplayCueNotify_Actor Interface */
	UE_API virtual bool Recycle() override;

	UE_API virtual bool OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
	UE_API virtual bool WhileActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
	UE_API virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
	UE_API virtual bool OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;

	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

#if WITH_EDITOR
	UE_API virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
	/** ~ End AGameplayCueNotify_Actor Interface */

	UE_API virtual void RemoveLoopingEffects();

	UFUNCTION(BlueprintImplementableEvent)
	UE_API void OnApplication(AActor* Target, const FGameplayCueParameters& Parameters, const FAkGameplayCueNotify_SpawnResult& SpawnResults);

	UFUNCTION(BlueprintImplementableEvent)
	UE_API void OnLoopingStart(AActor* Target, const FGameplayCueParameters& Parameters, const FAkGameplayCueNotify_SpawnResult& SpawnResults);

	UFUNCTION(BlueprintImplementableEvent)
	UE_API void OnRecurring(AActor* Target, const FGameplayCueParameters& Parameters, const FAkGameplayCueNotify_SpawnResult& SpawnResults);

	UFUNCTION(BlueprintImplementableEvent)
	UE_API void OnRemoval(AActor* Target, const FGameplayCueParameters& Parameters, const FAkGameplayCueNotify_SpawnResult& SpawnResults);

protected:
	/** Default condition to check before spawning anything.  Applies for all spawns unless overridden. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GCN Defaults")
	FGameplayCueNotify_SpawnCondition DefaultSpawnCondition;

	/** Default placement rules.  Applies for all spawns unless overridden. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GCN Defaults")
	FGameplayCueNotify_PlacementInfo DefaultPlacementInfo;

	/** List of effects to spawn on application.  These should not be looping effects! */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GCN Application Effects (On Active)")
	FAkGameplayCueNotify_BurstEffects ApplicationEffects;

	/** Results of spawned application effects. */
	UPROPERTY(BlueprintReadOnly, Category = "GCN Application Effects (On Active)")
	FAkGameplayCueNotify_SpawnResult ApplicationSpawnResults;

	/** List of effects to spawn on loop start. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GCN Looping Effects (While Active)")
	FAkGameplayCueNotify_LoopingEffects LoopingEffects;

	/** Results of spawned looping effects. */
	UPROPERTY(BlueprintReadOnly, Category = "GCN Looping Effects (While Active)")
	FAkGameplayCueNotify_SpawnResult LoopingSpawnResults;

	/** List of effects to spawn for a recurring gameplay effect (e.g. each time a DOT ticks).  These should not be looping effects! */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GCN Recurring Effects (On Execute)")
	FAkGameplayCueNotify_BurstEffects RecurringEffects;

	/** Results of spawned recurring effects. */
	UPROPERTY(BlueprintReadOnly, Category = "GCN Recurring Effects (On Execute)")
	FAkGameplayCueNotify_SpawnResult RecurringSpawnResults;

	/** List of effects to spawn on removal.  These should not be looping effects! */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GCN Removal Effects (On Remove)")
	FAkGameplayCueNotify_BurstEffects RemovalEffects;

	/** Results of spawned removal effects. */
	UPROPERTY(BlueprintReadOnly, Category = "GCN Removal Effects (On Remove)")
	FAkGameplayCueNotify_SpawnResult RemovalSpawnResults;

	bool bLoopingEffectsRemoved;
};

#undef UE_API
