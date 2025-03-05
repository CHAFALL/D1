#pragma once

#include "D1Define.h"
#include "Components/PawnComponent.h"
#include "D1CosmeticManagerComponent.generated.h"

class AD1ArmorBase;
//class UD1ItemFragment_Equipable_Armor;

// Manager이긴 하지만 PawnComponent로 만듬 - 모든 유저들마다 자기만의 Manager를 갖고 있어야 되는 경우 이렇게 Component를 만들어가지고 붙이는 경우가 많음
UCLASS(BlueprintType, Blueprintable)
class UD1CosmeticManagerComponent : public UPawnComponent
{
	GENERATED_BODY()
	
public:
	UD1CosmeticManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
public:
	void RefreshArmorMesh(EArmorType ArmorType /*, const UD1ItemFragment_Equipable_Armor* ArmorFragment*/);
	void GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const;

private:
	void InitializeManager();
	
	UChildActorComponent* SpawnCosmeticSlotActor(TSoftObjectPtr<USkeletalMesh> InDefaultMesh, TSoftObjectPtr<USkeletalMesh> InSecondaryMesh, FName InSkinMaterialSlotName, TSoftObjectPtr<UMaterialInterface> InSkinMaterial);
	void SetPrimaryArmorMesh(EArmorType ArmorType, TSoftObjectPtr<USkeletalMesh> ArmorMeshPtr);
	
protected:
	UPROPERTY(EditDefaultsOnly)
	ECharacterSkinType CharacterSkinType = ECharacterSkinType::Asian;
	
	// AD1ArmorBase를 상속받는 애들만 선택 가능
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AD1ArmorBase> CosmeticSlotClass;

private:
	UPROPERTY()
	TObjectPtr<UChildActorComponent> HeadSlot;
	
	// UChildActorComponent는 언리얼 엔진에서 제공하는 특수한 컴포넌트로, 하나의 액터 내부에 다른 액터를 중첩시키는 기능을 제공
	// 참고) 유니티 엔진에서는 게임 오브젝트를 여러 개 만든 다음에 하나를 다른 애한테 붙이고 하는 게 편리했는데 언리얼의 경우 그렇게 되어 있지 않음....
	// -> 언리얼 엔진에서 한 액터를 다른 액터에 붙이는 방법 중 하나가 바로 ChildActorComponent
	UPROPERTY()
	TArray<TObjectPtr<UChildActorComponent>> CosmeticSlots;

	bool bInitialized = false;
};
