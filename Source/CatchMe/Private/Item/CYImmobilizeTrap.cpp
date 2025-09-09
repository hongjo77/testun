// Item/CYImmobilizeTrap.cpp
#include "Item/CYImmobilizeTrap.h"
#include "Systems/CYEffectManagerComponent.h"
#include "GameplayTagContainer.h"

ACYImmobilizeTrap::ACYImmobilizeTrap()
{
	ItemName = FText::FromString("Snare Trap");
	ItemDescription = FText::FromString("Immobilizes target for 5 seconds");
	ImmobilizeDuration = 5.0f;
}

void ACYImmobilizeTrap::ApplyTrapEffect(AActor* Target)
{
	if (!Target || !HasAuthority()) return;

	UCYEffectManagerComponent* EffectManager = Target->FindComponentByClass<UCYEffectManagerComponent>();
	if (!EffectManager) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Target %s has no EffectManager"), *Target->GetName());
		return;
	}

	FGameplayTag ImmobilizeTag = FGameplayTag::RequestGameplayTag("Effect.Immobilize");
	EffectManager->ServerApplyEffect(ImmobilizeTag, ImmobilizeDuration, 1.0f, GetOwner());
    
	UE_LOG(LogTemp, Warning, TEXT("Applied immobilize to %s for %.1fs"), *Target->GetName(), ImmobilizeDuration);
}