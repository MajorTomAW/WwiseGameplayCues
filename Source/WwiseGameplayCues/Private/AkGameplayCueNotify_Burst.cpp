// Author: Tom Werner (MajorT), 2026 February


#include "AkGameplayCueNotify_Burst.h"

#include "GameplayCueNotifyTypes.h"
#include "Misc/DataValidation.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(AkGameplayCueNotify_Burst)

UAkGameplayCueNotify_Burst::UAkGameplayCueNotify_Burst()
{
}

bool UAkGameplayCueNotify_Burst::OnExecute_Implementation(
	AActor* MyTarget,
	const FGameplayCueParameters& Parameters) const
{
	UWorld* World = (IsValid(MyTarget) ? MyTarget->GetWorld() : GetWorld());

	FGameplayCueNotify_SpawnContext SpawnContext(World, MyTarget, Parameters);
	SpawnContext.SetDefaultSpawnCondition(&DefaultSpawnCondition);
	SpawnContext.SetDefaultPlacementInfo(&DefaultPlacementInfo);

	if (DefaultSpawnCondition.ShouldSpawn(SpawnContext))
	{
		FAkGameplayCueNotify_SpawnResult SpawnResult;
		BurstEffects.ExecuteEffects(SpawnContext, SpawnResult);

		OnBurst(MyTarget, Parameters, SpawnResult);
	}

	return false;
}

#if WITH_EDITOR
EDataValidationResult UAkGameplayCueNotify_Burst::IsDataValid(class FDataValidationContext& Context) const
{
	BurstEffects.ValidateAssociatedAssets(this, TEXT("BurstEffects"), Context);
	return ((Context.GetNumErrors() > 0) ? EDataValidationResult::Invalid : EDataValidationResult::Valid);
}
#endif
