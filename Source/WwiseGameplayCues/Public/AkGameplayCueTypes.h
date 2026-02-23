// Author: Tom Werner (MajorT), 2026 February

#pragma once
#include <AK/SoundEngine/Common/AkTypedefs.h>

#include "GameplayCueNotifyTypes.h"

#include "AkGameplayCueTypes.generated.h"

#define UE_API WWISEGAMEPLAYCUES_API

class UAkAudioEvent;
struct FGameplayCueNotify_SpawnContext;
struct FGameplayCueNotify_SpawnResult;

DECLARE_LOG_CATEGORY_EXTERN(LogAkGameplayCueNotify, Log, All);

/**
 * FAkGameplayCueNotify_SpawnResult
 *
 *	Temporary structure used to track results of spawning components and ak events.
 */
USTRUCT(BlueprintType)
struct FAkGameplayCueNotify_SpawnResult
{
	GENERATED_BODY()

	UE_API FAkGameplayCueNotify_SpawnResult();
	UE_API virtual ~FAkGameplayCueNotify_SpawnResult() {}

	/** Resets this spawn result to empty. */
	UE_API virtual void Reset();

	/** Sets this spawn result from a FGameplayCueNotify_SpawnResult. */
	UE_API void SetFromEngineSpawnResult(const FGameplayCueNotify_SpawnResult& SpawnResult);

	/** List of FX components spawned.  There may be null pointers here as it matches the defined order. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = GameplayCueNotify)
	TArray<TObjectPtr<UFXSystemComponent>> FxSystemComponents;

	/** List of ak event ID's triggered .  There may be null IDs here as it matches the defined order. */
	TArray<AkPlayingID> AkEventIDs;

	/** List of camera shakes played.  There will be one camera shake per local player controller if shake is played in world. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = GameplayCueNotify)
	TArray<TObjectPtr<UCameraShakeBase>> CameraShakes;

	/** List of camera len effects spawned.  There will be one camera lens effect per local player controller if the effect is played in world. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = GameplayCueNotify)
	TArray<TScriptInterface<ICameraLensEffectInterface>> CameraLensEffects;

	/** Force feedback component that was spawned.  This is only valid when force feedback is set to play in world. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = GameplayCueNotify)
	TObjectPtr<UForceFeedbackComponent> ForceFeedbackComponent;

	/** Player controller used to play the force feedback effect.  Used to stop the effect later. */
	UPROPERTY(Transient)
	TObjectPtr<APlayerController> ForceFeedbackTargetPC;

	/** Spawned decal component.  This may be null. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = GameplayCueNotify)
	TObjectPtr<UDecalComponent> DecalComponent;
};

/**
 * FAkGameplayCueNotify_EventInfo
 *
 *	Properties that specify how to post an ak audio event.
 */
USTRUCT(BlueprintType)
struct FAkGameplayCueNotify_AkEventInfo
{
	GENERATED_BODY()

	UE_API FAkGameplayCueNotify_AkEventInfo();
	UE_API virtual ~FAkGameplayCueNotify_AkEventInfo() {}

	UE_API virtual bool PostEvent(const FGameplayCueNotify_SpawnContext& SpawnContext, FAkGameplayCueNotify_SpawnResult& OutSpawnResult) const;
	UE_API virtual void ValidateBurstAssets(const UObject* ContainingAsset, const FString& Context, class FDataValidationContext& ValidationContext) const;

public:
	/** If enabled, use the spawn condition override and not the default one. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = GameplayCueNotify, Meta = (InlineEditConditionToggle))
	uint32 bOverrideSpawnCondition : 1;

	/** If enabled, use the placement info override and not the default one. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = GameplayCueNotify, Meta = (InlineEditConditionToggle))
	uint32 bOverridePlacementInfo : 1;

	/** Condition to check before playing the sound. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = GameplayCueNotify, Meta = (EditCondition = "bOverrideSpawnCondition"))
	FGameplayCueNotify_SpawnCondition SpawnConditionOverride;

	/** Defines how the sound will be placed. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = GameplayCueNotify, Meta = (EditCondition = "bOverridePlacementInfo"))
	FGameplayCueNotify_PlacementInfo PlacementInfoOverride;

	/** The Ak event to trigger. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = GameplayCueNotify)
	TObjectPtr<UAkAudioEvent> AkEvent;
};

/**
 * FAkGameplayCueNotify_BurstEffects
 *
 * Set of effects to spawn for a single event, used by all Ak gameplay cue notify types.
 */
USTRUCT(BlueprintType)
struct FAkGameplayCueNotify_BurstEffects
{
	GENERATED_BODY()

	UE_API FAkGameplayCueNotify_BurstEffects();
	UE_API virtual ~FAkGameplayCueNotify_BurstEffects() {}

	UE_API virtual void ExecuteEffects(const FGameplayCueNotify_SpawnContext& SpawnContext, FAkGameplayCueNotify_SpawnResult& OutSpawnResult) const;
	UE_API virtual void ValidateAssociatedAssets(const UObject* ContainingAsset, const FString& Context, class FDataValidationContext& ValidationContext) const;

protected:
	/** Particle systems to be spawned on gameplay cue execution.  These should never use looping effects! */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = GameplayCueNotify)
	TArray<FGameplayCueNotify_ParticleInfo> BurstParticles;

	/** Ak events to be posted on gameplay cue execution.  These should never use infinite effects! */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = GameplayCueNotify)
	TArray<FAkGameplayCueNotify_AkEventInfo> BurstAkEvents;

	/** Camera shake to be played on gameplay cue execution.  This should never use a looping effect! */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = GameplayCueNotify)
	FGameplayCueNotify_CameraShakeInfo BurstCameraShake;

	/** Camera lens effect to be played on gameplay cue execution.  This should never use a looping effect! */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = GameplayCueNotify)
	FGameplayCueNotify_CameraLensEffectInfo BurstCameraLensEffect;

	/** Force feedback to be played on gameplay cue execution.  This should never use a looping effect! */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = GameplayCueNotify)
	FGameplayCueNotify_ForceFeedbackInfo BurstForceFeedback;

	/** Input device properties to be applied on gameplay cue execution. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GameplayCueNotify)
	FGameplayCueNotify_InputDevicePropertyInfo BurstDevicePropertyEffect;

	/** Decal to be spawned on gameplay cue execution.  Actor should have fade out time or override should be set so it will clean up properly. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = GameplayCueNotify)
	FGameplayCueNotify_DecalInfo BurstDecal;
};

#undef UE_API
