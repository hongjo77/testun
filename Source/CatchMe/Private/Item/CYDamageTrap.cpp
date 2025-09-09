// Item/CYDamageTrap.cpp
#include "Item/CYDamageTrap.h"
#include "Engine/DamageEvents.h"

ACYDamageTrap::ACYDamageTrap()
{
	ItemName = FText::FromString("Spike Trap");
	ItemDescription = FText::FromString("Deals 40 damage to targets");
	TrapDamage = 40.0f;
}

void ACYDamageTrap::ApplyTrapEffect(AActor* Target)
{
	if (!Target || !HasAuthority()) return;

	FPointDamageEvent DamageEvent;
	DamageEvent.Damage = TrapDamage;
	DamageEvent.HitInfo = FHitResult();

	Target->TakeDamage(TrapDamage, DamageEvent, GetInstigator() ? GetInstigator()->GetController() : nullptr, this);
    
	UE_LOG(LogTemp, Warning, TEXT("Damage trap dealt %.1f damage to %s"), TrapDamage, *Target->GetName());
}