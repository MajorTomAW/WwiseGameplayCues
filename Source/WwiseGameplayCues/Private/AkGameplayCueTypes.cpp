// Author: Tom Werner (MajorT), 2026 February


#include "AkGameplayCueTypes.h"

#include <AK/SoundEngine/Common/AkConstants.h>

#include "AkAudioEvent.h"

#if WITH_EDITORONLY_DATA
#include "Misc/DataValidation.h"
#endif

#define LOCTEXT_NAMESPACE "AkGameplayCueNotify"

DEFINE_LOG_CATEGORY(LogAkGameplayCueNotify);

FAkGameplayCueNotify_SpawnResult::FAkGameplayCueNotify_SpawnResult()
{
	FAkGameplayCueNotify_SpawnResult::Reset();
}

void FAkGameplayCueNotify_SpawnResult::Reset()
{
	FxSystemComponents.Reset();
	AkEventIDs.Reset();
	CameraShakes.Reset();
	CameraLensEffects.Reset();
	ForceFeedbackComponent = nullptr;
	ForceFeedbackTargetPC = nullptr;
	DecalComponent = nullptr;
}

void FAkGameplayCueNotify_SpawnResult::SetFromEngineSpawnResult(const FGameplayCueNotify_SpawnResult& SpawnResult)
{
	FxSystemComponents.Reset(SpawnResult.FxSystemComponents.Num());
	FxSystemComponents.Append(SpawnResult.FxSystemComponents);

	CameraShakes.Reset(SpawnResult.CameraShakes.Num());
	CameraShakes.Append(SpawnResult.CameraShakes);

	CameraLensEffects.Reset(SpawnResult.CameraLensEffects.Num());
	CameraLensEffects.Append(SpawnResult.CameraLensEffects);

	ForceFeedbackComponent = SpawnResult.ForceFeedbackComponent;
	ForceFeedbackTargetPC = SpawnResult.ForceFeedbackTargetPC;
	DecalComponent = SpawnResult.DecalComponent;
}

FAkGameplayCueNotify_AkEventInfo::FAkGameplayCueNotify_AkEventInfo()
	: bOverrideSpawnCondition(false)
	, bOverridePlacementInfo(false)
	, AkEvent(nullptr)
{
}

bool FAkGameplayCueNotify_AkEventInfo::PostEvent(
	const FGameplayCueNotify_SpawnContext& SpawnContext,
	FAkGameplayCueNotify_SpawnResult& OutSpawnResult) const
{
	AkPlayingID EventID = AK_INVALID_PLAYING_ID;
	bool bEventTriggered = false;

	if (IsValid(AkEvent))
	{
		const auto& SpawnCondition = SpawnContext.GetSpawnCondition(bOverrideSpawnCondition, SpawnConditionOverride);
		const auto& PlacementInfo = SpawnContext.GetPlacementInfo(bOverridePlacementInfo, PlacementInfoOverride);

		if (SpawnCondition.ShouldSpawn(SpawnContext))
		{
			FTransform SpawnTransform;
			if (PlacementInfo.FindSpawnTransform(SpawnContext, SpawnTransform))
			{
				if (SpawnContext.TargetComponent && (PlacementInfo.AttachPolicy == EGameplayCueNotify_AttachPolicy::AttachToTarget))
				{
					EventID = AkEvent->PostOnActor(
						SpawnContext.TargetActor,
						{},
						0,
						true);
				}
				else
				{
					EventID = AkEvent->PostAtLocation(
						SpawnTransform.GetLocation(),
						SpawnTransform.GetRotation().Rotator(),
						{},
						0,
						SpawnContext.World);
				}

				bEventTriggered = true;
			}
		}
	}

	// Always add to the list, even if invalid, so that the list is table and in order for blueprint users.
	OutSpawnResult.AkEventIDs.Add(EventID);

	return bEventTriggered;
}

void FAkGameplayCueNotify_AkEventInfo::ValidateBurstAssets(
	const UObject* ContainingAsset,
	const FString& Context,
	class FDataValidationContext& ValidationContext) const
{
#if WITH_EDITORONLY_DATA
	if (AkEvent != nullptr)
	{
		if (AkEvent->IsInfinite)
		{
			ValidationContext.AddError(FText::Format(
				LOCTEXT("AkSoundCue_ShouldNotLoop", "Sound [{0}] used in slot [{1}] for asset [{2}] is not a one-shot, but the slot is a one-shot (the instance will be orphaned)."),
				FText::AsCultureInvariant(AkEvent->GetPathName()),
				FText::AsCultureInvariant(Context),
				FText::AsCultureInvariant(ContainingAsset->GetPathName())));
		}
	}
#endif
}

FAkGameplayCueNotify_BurstEffects::FAkGameplayCueNotify_BurstEffects()
{
}

void FAkGameplayCueNotify_BurstEffects::ExecuteEffects(
	const FGameplayCueNotify_SpawnContext& SpawnContext,
	FAkGameplayCueNotify_SpawnResult& OutSpawnResult) const
{
	if (!SpawnContext.World)
	{
		UE_LOG(LogAkGameplayCueNotify, Error, TEXT("AkGameplayCueNotify: Trying to execute Burst effects with a NULL world."))
		return;
	}


	// We need to store them in the engine SpawnResult struct to later populate them into the Ak one.
	FGameplayCueNotify_SpawnResult EngineSpawnResult;

	for (const FGameplayCueNotify_ParticleInfo& ParticleInfo : BurstParticles)
	{
		ParticleInfo.PlayParticleEffect(SpawnContext, EngineSpawnResult);
	}

	BurstCameraShake.PlayCameraShake(SpawnContext, EngineSpawnResult);
	BurstCameraLensEffect.PlayCameraLensEffect(SpawnContext, EngineSpawnResult);
	BurstForceFeedback.PlayForceFeedback(SpawnContext, EngineSpawnResult);
	BurstDevicePropertyEffect.SetDeviceProperties(SpawnContext, EngineSpawnResult);
	BurstDecal.SpawnDecal(SpawnContext, EngineSpawnResult);

	// Populate the ak spawn result
	OutSpawnResult.SetFromEngineSpawnResult(EngineSpawnResult);

	for (const FAkGameplayCueNotify_AkEventInfo& AkEvent : BurstAkEvents)
	{
		AkEvent.PostEvent(SpawnContext, OutSpawnResult);
	}
}

void FAkGameplayCueNotify_BurstEffects::ValidateAssociatedAssets(
	const UObject* ContainingAsset,
	const FString& Context,
	class FDataValidationContext& ValidationContext) const
{
	for (const FGameplayCueNotify_ParticleInfo& ParticleInfo : BurstParticles)
	{
		ParticleInfo.ValidateBurstAssets(ContainingAsset, Context + TEXT(".BurstParticles"), ValidationContext);
	}

	for (const FAkGameplayCueNotify_AkEventInfo& AkEvent : BurstAkEvents)
	{
		AkEvent.ValidateBurstAssets(ContainingAsset, Context + TEXT(".BurstAkEvents"), ValidationContext);
	}

	BurstCameraShake.ValidateBurstAssets(ContainingAsset, Context + TEXT(".BurstCameraShake"), ValidationContext);
	BurstCameraLensEffect.ValidateBurstAssets(ContainingAsset, Context + TEXT(".BurstCameraLensEffect"), ValidationContext);
	BurstForceFeedback.ValidateBurstAssets(ContainingAsset, Context + TEXT(".BurstForceFeedback"), ValidationContext);
	BurstDevicePropertyEffect.ValidateBurstAssets(ContainingAsset, Context + TEXT(".BurstDevicePropertyEffect"), ValidationContext);
}

#undef LOCTEXT_NAMESPACE
