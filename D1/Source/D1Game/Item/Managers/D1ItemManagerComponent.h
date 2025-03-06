#pragma once

#include "Components/ControllerComponent.h"
#include "D1ItemManagerComponent.generated.h"

class UD1ItemInstance;
class AD1PickupableItemBase;
class UD1EquipmentManagerComponent;
class UD1InventoryManagerComponent;

// 얘는 특이하게 UControllerComponent?
// 각 캐릭터(Pawn)는 자신만의 인벤토리와 장비 시스템을 갖는 것이 자연스러움. 따라서 UD1InventoryManagerComponent와 UD1EquipmentManagerComponent는 ActorComponent로 만들어 각 캐릭터에 부착하는 것이 합리적
// 그러나 아이템 이동을 관리하는 로직은 플레이어(사용자)의 권한 문제와 연관된다. 플레이어는 자신의 캐릭터의 아이템만 조작할 권한이 있고, 다른 플레이어의 아이템을 조작해서는 안 됨.
// PlayerController는 특정 플레이어의 입력과 권한을 관리하는 컴포넌트이므로, 아이템 이동 같은 권한이 필요한 작업을 관리하기에 적합.

// UControllerComponent를 상속받은 이유:
//    - 아이템 이동은 플레이어의 권한 문제와 연관됨
//    - 각 플레이어는 자신의 캐릭터 아이템만 조작 가능해야 함
//    - 다른 플레이어의 아이템을 조작하는 것은 권한 위반
UCLASS()
class UD1ItemManagerComponent : public UControllerComponent
{
	GENERATED_BODY()
	
public:
	UD1ItemManagerComponent(const FObjectInitializer& ObjectInitializer);
	
public:
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_InventoryToEquipment(UD1InventoryManagerComponent* FromInventoryManager, const FIntPoint& FromItemSlotPos, UD1EquipmentManagerComponent* ToEquipmentManager, EEquipmentSlotType ToEquipmentSlotType);
	
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_EquipmentToInventory(UD1EquipmentManagerComponent* FromEquipmentManager, EEquipmentSlotType FromEquipmentSlotType, UD1InventoryManagerComponent* ToInventoryManager, const FIntPoint& ToItemSlotPos);
	
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_InventoryToInventory(UD1InventoryManagerComponent* FromInventoryManager, const FIntPoint& FromItemSlotPos, UD1InventoryManagerComponent* ToInventoryManager, const FIntPoint& ToItemSlotPos);
	
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_EquipmentToEquipment(UD1EquipmentManagerComponent* FromEquipmentManager, EEquipmentSlotType FromEquipmentSlotType, UD1EquipmentManagerComponent* ToEquipmentManager, EEquipmentSlotType ToEquipmentSlotType);
	
public:
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_QuickFromInventory(UD1InventoryManagerComponent* FromInventoryManager, const FIntPoint& FromItemSlotPos);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_QuickFromEquipment(UD1EquipmentManagerComponent* FromEquipmentManager, EEquipmentSlotType FromEquipmentSlotType);
	
public:
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_DropItemFromInventory(UD1InventoryManagerComponent* FromInventoryManager, const FIntPoint& FromItemSlotPos);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_DropItemFromEquipment(UD1EquipmentManagerComponent* FromEquipmentManager, EEquipmentSlotType FromEquipmentSlotType);

public:
	/*UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	bool TryPickItem(AD1PickupableItemBase* PickupableItemActor);*/

	/*UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	bool TryDropItem(UD1ItemInstance* FromItemInstance, int32 FromItemCount);*/
	
public:
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void AddAllowedComponent(UActorComponent* ActorComponent);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void RemoveAllowedComponent(UActorComponent* ActorComponent);
	
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, BlueprintPure)
	bool IsAllowedComponent(UActorComponent* ActorComponent) const;

protected:
	UFUNCTION(BlueprintCallable, BlueprintPure)
	UD1InventoryManagerComponent* GetMyInventoryManager() const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	UD1EquipmentManagerComponent* GetMyEquipmentManager() const;
	
private:
	UPROPERTY()
	TArray<TWeakObjectPtr<UActorComponent>> AllowedComponents;
};

// 전역 느낌으로
// 모든 아이템을 중앙에서 관리해서 실제로 아이템의 이동을 관장하는 역할을 함

