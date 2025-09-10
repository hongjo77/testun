#include "Abilities/GA_PlaceTrap.h"
#include "Player/CYPlayerCharacter.h"
#include "Items/CYTrapBase.h"
#include "Engine/World.h"

UGA_PlaceTrap::UGA_PlaceTrap()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerExecution;
    
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag("Ability.Trap.Place"));
}

void UGA_PlaceTrap::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	ACYPlayerCharacter* Character = Cast<ACYPlayerCharacter>(GetAvatarActorFromActorInfo());
	if (!Character || !TrapClass)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 트랩 설치 위치 계산
	FVector SpawnLocation = Character->GetActorLocation() + 
						   Character->GetActorForwardVector() * 200.0f;
	SpawnLocation.Z = Character->GetActorLocation().Z;  // 같은 높이 유지

	FRotator SpawnRotation = Character->GetActorRotation();

	// 트랩 스폰
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Character;
	SpawnParams.Instigator = Character;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	if (ACYTrapBase* Trap = GetWorld()->SpawnActor<ACYTrapBase>(TrapClass, SpawnLocation, SpawnRotation, SpawnParams))
	{
		// 트랩에 이펙트 설정
		Trap->TrapEffectClass = TrapEffectClass;
        
		UE_LOG(LogTemp, Warning, TEXT("Trap placed at %s"), *SpawnLocation.ToString());
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}