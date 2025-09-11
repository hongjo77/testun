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
        float NewMoveSpeed = GetMoveSpeed();
        UE_LOG(LogTemp, Warning, TEXT("MoveSpeed changed to: %f for %s"), 
               NewMoveSpeed, GetOwningActor() ? *GetOwningActor()->GetName() : TEXT("NULL"));
        
        if (ACharacter* Character = Cast<ACharacter>(GetOwningActor()))
        {
            if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
            {
                MovementComp->MaxWalkSpeed = NewMoveSpeed;
                
                if (NewMoveSpeed <= 0.0f)
                {
                    // 완전 정지
                    MovementComp->StopMovementImmediately();
                    MovementComp->MaxAcceleration = 0.0f;
                    MovementComp->BrakingDecelerationWalking = 10000.0f;
                    MovementComp->GroundFriction = 100.0f;
                    
                    // ✅ 점프 완전 차단
                    MovementComp->JumpZVelocity = 0.0f;
                    
                    UE_LOG(LogTemp, Warning, TEXT("IMMOBILIZED: %s (Jump blocked)"), *Character->GetName());
                }
                else if (NewMoveSpeed < 200.0f)
                {
                    MovementComp->MaxAcceleration = 500.0f;
                    MovementComp->BrakingDecelerationWalking = 1000.0f;
                    
                    // 느릴 때도 점프 막기
                    MovementComp->JumpZVelocity = 0.0f;
                    
                    UE_LOG(LogTemp, Warning, TEXT("SLOWED: %s to %f (Jump blocked)"), *Character->GetName(), NewMoveSpeed);
                }
                else
                {
                    // 정상 복구
                    MovementComp->MaxAcceleration = 2048.0f;
                    MovementComp->BrakingDecelerationWalking = 2000.0f;
                    MovementComp->GroundFriction = 8.0f;
                    
                    // ✅ 점프 복구
                    MovementComp->JumpZVelocity = 600.0f;  // 원래 점프력
                    
                    UE_LOG(LogTemp, Warning, TEXT("MOVEMENT RESTORED: %s (Jump enabled)"), *Character->GetName());
                }
            }
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
    
    UE_LOG(LogTemp, Warning, TEXT("OnRep_MoveSpeed: %f -> %f"), 
           OldMoveSpeed.GetCurrentValue(), GetMoveSpeed());
    
    // ✅ 클라이언트에서도 동일한 점프 제한 적용
    if (ACharacter* Character = Cast<ACharacter>(GetOwningActor()))
    {
        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
        {
            MovementComp->MaxWalkSpeed = GetMoveSpeed();
            
            if (GetMoveSpeed() <= 0.0f)
            {
                MovementComp->StopMovementImmediately();
                MovementComp->MaxAcceleration = 0.0f;
                MovementComp->BrakingDecelerationWalking = 10000.0f;
                MovementComp->GroundFriction = 100.0f;
                
                // 클라이언트에서도 점프 차단
                MovementComp->JumpZVelocity = 0.0f;
                
                UE_LOG(LogTemp, Warning, TEXT("CLIENT: %s immobilized (Jump blocked)"), *Character->GetName());
            }
            else if (GetMoveSpeed() < 200.0f)
            {
                MovementComp->MaxAcceleration = 500.0f;
                MovementComp->BrakingDecelerationWalking = 1000.0f;
                MovementComp->JumpZVelocity = 0.0f;
            }
            else
            {
                MovementComp->MaxAcceleration = 2048.0f;
                MovementComp->BrakingDecelerationWalking = 2000.0f;
                MovementComp->GroundFriction = 8.0f;
                
                // 클라이언트에서도 점프 복구
                MovementComp->JumpZVelocity = 600.0f;
                
                UE_LOG(LogTemp, Warning, TEXT("CLIENT: %s mobility restored (Jump enabled)"), *Character->GetName());
            }
        }
    }
}

void UCYAttributeSet::OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UCYAttributeSet, AttackPower, OldAttackPower);
}