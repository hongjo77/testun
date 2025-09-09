// Item/CYItemBase.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"
#include "GameplayTagContainer.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "CYItemBase.generated.h"

UCLASS(Abstract, BlueprintType, Blueprintable)
class CATCHME_API ACYItemBase : public AActor
{
    GENERATED_BODY()

public:
    ACYItemBase();

    // 아이템 정보
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Item Info")
    FGameplayTag ItemType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Info")
    FText ItemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Info")
    FText ItemDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Info")
    UTexture2D* ItemIcon;

    // 컴포넌트들
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ItemMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* PickupCollision;

    // 아이템 사용 (블루프린트에서 구현 가능)
    UFUNCTION(BlueprintImplementableEvent, Category = "Item")
    void OnItemUsed(AActor* User);

    // 아이템 사용 (C++에서 오버라이드 가능)
    UFUNCTION(BlueprintCallable, Category = "Item")
    virtual void UseItem(AActor* User);

    // 픽업 처리
    UFUNCTION(Server, Reliable, BlueprintCallable)
    void ServerPickupItem(AActor* User);

    // 픽업 가능 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Info")
    bool bCanPickup = true;

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // 픽업 콜리전 이벤트
    UFUNCTION()
    void OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                        bool bFromSweep, const FHitResult& SweepResult);

    // 아이템별 사용 로직 (자식 클래스에서 구현)
    virtual void ExecuteItemEffect(AActor* User);
};