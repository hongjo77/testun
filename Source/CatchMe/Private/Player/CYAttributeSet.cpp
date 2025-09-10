#include "GAS/CYAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UCYAttributeSet::UCYAttributeSet()
{
    // 기본값 설정
    InitHealth(100.0f);
    InitMaxHealth(100.0f);
    InitMoveSpeed(600.0f);
    InitAttackPower(50.0f);
}

void UCYAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(UCYAttributeSet, Health, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UCYAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UCYAttributeSet, MoveSpeed, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UCYAttributeSet, AttackPower, COND_None, REPNOTIFY_Always);
}

void UCYAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    Super::PreAttributeChange(Attribute, NewValue);

    // 값 범위 제한
    if (Attribute == GetHealthAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
    }
    else if (Attribute == GetMoveSpeedAttribute())
    {
        NewValue = FMath::Max(NewValue, 0.0f);
    }
}

void UCYAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);

    FGameplayEffectContextHandle Context = Data.EffectSpec.GetContext();
    UAbilitySystemComponent* Source = Context.GetOriginalInstigatorAbilitySystemComponent();
    const FGameplayTagContainer& SourceTags = *Data.EffectSpec.CapturedSourceTags.GetAggregatedTags();
    FGameplayTagContainer SpecAssetTags;
    Data.EffectSpec.GetAllAssetTags(SpecAssetTags);

    // 체력 변화 처리
    if (Data.EvaluatedData.Attribute == GetHealthAttribute())
    {
        float NewHealth = GetHealth();
        SetHealth(FMath::Clamp(NewHealth, 0.0f, GetMaxHealth()));

        if (GetHealth() <= 0.0f)
        {
            // 사망 처리
            if (AActor* Owner = GetOwningActor())
            {
                UE_LOG(LogTemp, Warning, TEXT("%s has died"), *Owner->GetName());
                // TODO: 사망 이벤트 발생
            }
        }
    }

    // 이동속도 변화 처리
    if (Data.EvaluatedData.Attribute == GetMoveSpeedAttribute())
    {
        if (ACharacter* Character = Cast<ACharacter>(GetOwningActor()))
        {
            // 실제 캐릭터 이동속도 업데이트
            Character->GetCharacterMovement()->MaxWalkSpeed = GetMoveSpeed();
            
            UE_LOG(LogTemp, Warning, TEXT("MoveSpeed changed to: %f"), GetMoveSpeed());
        }
    }
}

void UCYAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UCYAttributeSet, Health, OldHealth);
}

void UCYAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UCYAttributeSet, MaxHealth, OldMaxHealth);
}

void UCYAttributeSet::OnRep_MoveSpeed(const FGameplayAttributeData& OldMoveSpeed)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UCYAttributeSet, MoveSpeed, OldMoveSpeed);
    
    // 클라이언트에서도 이동속도 업데이트
    if (ACharacter* Character = Cast<ACharacter>(GetOwningActor()))
    {
        Character->GetCharacterMovement()->MaxWalkSpeed = GetMoveSpeed();
    }
}

void UCYAttributeSet::OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UCYAttributeSet, AttackPower, OldAttackPower);
}