// Author: Tom Werner (MajorT), 2026 February


#include "AkGameplayCueTypes.h"

#include <AK/SoundEngine/Common/AkConstants.h>

#include "AkAudioDevice.h"
#include "AkAudioEvent.h"
#include "Camera/CameraLensEffectInterface.h"
#include "Components/ForceFeedbackComponent.h"
#include "Particles/ParticleSystemComponent.h"

#if WITH_EDITORONLY_DATA
#include "Misc/DataValidation.h"
#endif

#define LOCTEXT_NAMESPACE "AkGameplayCueNotify"

DEFINE_LOG_CATEGORY(LogAkGameplayCueNotify);

FAkGameplayCueNotify_SpawnResult::FAkGameplayCueNotify_SpawnResult()
{
	Reset();
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
	, LoopingFadeOutDurationMs(0)
	, LoopingFadeOutInterpolation(EAkCurveInterpolation::Linear)
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
				/*else if (AkEvent->IsInfinite)
				{
					AkDeviceAndWorld DeviceAndWorld(SpawnContext.World);
					if (!UNLIKELY(!DeviceAndWorld.IsValid()))
					{
						DeviceAndWorld.AkAudioDevice->SpawnAkComponentAtLocation(
							AkEvent,
							SpawnTransform.GetLocation(),
							SpawnTransform.GetRotation().Rotator(),
							true,
							true,
							DeviceAndWorld.CurrentWorld);
					}
				}*/
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

FAkGameplayCueNotify_LoopingEffects::FAkGameplayCueNotify_LoopingEffects()
{
}

void FAkGameplayCueNotify_LoopingEffects::StartEffects(
	const FGameplayCueNotify_SpawnContext& SpawnContext,
	FAkGameplayCueNotify_SpawnResult& OutSpawnResult) const
{
	if (!SpawnContext.World)
	{
		UE_LOG(LogAkGameplayCueNotify, Error, TEXT("AkGameplayCueNotify: Trying to start looping effects with a NULL world."))
		return;
	}

	// We need to store them in the engine SpawnResult struct to later populate them into the Ak one.
	FGameplayCueNotify_SpawnResult EngineSpawnResult;

	for (const FGameplayCueNotify_ParticleInfo& ParticleInfo : LoopingParticles)
	{
		ParticleInfo.PlayParticleEffect(SpawnContext, EngineSpawnResult);
	}

	LoopingCameraShake.PlayCameraShake(SpawnContext, EngineSpawnResult);
	LoopingCameraLensEffect.PlayCameraLensEffect(SpawnContext, EngineSpawnResult);
	LoopingForceFeedback.PlayForceFeedback(SpawnContext, EngineSpawnResult);
	LoopingInputDevicePropertyEffect.SetDeviceProperties(SpawnContext, EngineSpawnResult);

	// Populate the ak spawn result
	OutSpawnResult.SetFromEngineSpawnResult(EngineSpawnResult);

	for (const FAkGameplayCueNotify_AkEventInfo& AkEvent : LoopingAkEvents)
	{
		AkEvent.PostEvent(SpawnContext, OutSpawnResult);
	}
}

void FAkGameplayCueNotify_LoopingEffects::StopEffects(
	FAkGameplayCueNotify_SpawnResult& SpawnResult) const
{
	// Stop all particle effects
	for (UFXSystemComponent* FxSc : SpawnResult.FxSystemComponents)
	{
		if (IsValid(FxSc))
		{
			FxSc->Deactivate();
		}
	}

	// Stop all ak events. This assumes there is one AkPlayingID entry for each FAkGameplayCueNotify_AkEventInfo.
	ensure(LoopingAkEvents.Num() == SpawnResult.AkEventIDs.Num());

	for (int32 IdIndex = 0; IdIndex < SpawnResult.AkEventIDs.Num(); ++IdIndex)
	{
		AkPlayingID PlayingId = SpawnResult.AkEventIDs[IdIndex];
		if (PlayingId != AK_INVALID_PLAYING_ID)
		{
			AkTimeMs FadeDurationMs = 0.f;
			AkCurveInterpolation FadeInterpolation = AkCurveInterpolation_Linear;
			if (LoopingAkEvents.IsValidIndex(IdIndex))
			{
				const FAkGameplayCueNotify_AkEventInfo* EventInfo = &LoopingAkEvents[IdIndex];
				FadeDurationMs = EventInfo->LoopingFadeOutDurationMs;
				FadeInterpolation = static_cast<AkCurveInterpolation>(EventInfo->LoopingFadeOutInterpolation);
			}

			FAkAudioDevice::Get()->StopPlayingID(PlayingId, FadeDurationMs, FadeInterpolation);
		}
	}

	// Stop all camera shakes
	for (UCameraShakeBase* CameraShake : SpawnResult.CameraShakes)
	{
		if (CameraShake)
		{
			constexpr bool bStopImmediately = false;
			CameraShake->StopShake(bStopImmediately);
		}
	}

	// Stop the camera lens effect.
	for (TScriptInterface CameraLensEffect : SpawnResult.CameraLensEffects)
	{
		if (CameraLensEffect)
		{
			CameraLensEffect->DeactivateLensEffect();
		}
	}

	// Stop the force feedback.  The component is only created when the effect is played in world.
	// If it's not in world, it needs to be stopped on the player controller.
	if (SpawnResult.ForceFeedbackComponent)
	{
		SpawnResult.ForceFeedbackComponent->Stop();
	}

	if (SpawnResult.ForceFeedbackTargetPC)
	{
		SpawnResult.ForceFeedbackTargetPC->ClientStopForceFeedback(LoopingForceFeedback.ForceFeedbackEffect, LoopingForceFeedback.ForceFeedbackTag);
	}

	// There should be no decal on looping gameplay cues.
	ensure(SpawnResult.DecalComponent == nullptr);

	// Clear the spawn results.
	SpawnResult.Reset();
}

void FAkGameplayCueNotify_LoopingEffects::ValidateAssociatedAssets(
	const UObject* ContainingAsset,
	const FString& Context,
	class FDataValidationContext& ValidationContext) const
{
}

#undef LOCTEXT_NAMESPACE
