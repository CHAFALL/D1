#pragma once

#include "D1Define.h"
#include "Components/PawnComponent.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "D1EquipmentManagerComponent.generated.h"

//class UD1EquipManagerComponent;
class ALyraCharacter;
class ALyraPlayerController;
class UD1ItemInstance;
class UD1ItemTemplate;
//class UD1EquipmentManagerComponent;
//class UD1InventoryManagerComponent;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnEquipmentEntryChanged, EEquipmentSlotType, UD1ItemInstance*, int32/*ItemCount*/);

// 메모리 상에 생성된 ItemInstance들을 들고 있는 것임.
USTRUCT(BlueprintType)
struct FD1EquipmentEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()
	
private:
	void Init(UD1ItemInstance* InItemInstance, int32 InItemCount);
	UD1ItemInstance* Reset();
	
public:
	UD1ItemInstance* GetItemInstance() const { return ItemInstance; }
	int32 GetItemCount() const { return ItemCount; }
	
private:
	friend struct FD1EquipmentList;
	friend class UD1EquipmentManagerComponent;
	friend class UD1ItemManagerComponent;

	UPROPERTY()
	TObjectPtr<UD1ItemInstance> ItemInstance;

	UPROPERTY()
	int32 ItemCount = 0;

private:
	UPROPERTY(NotReplicated)
	EEquipmentSlotType EquipmentSlotType = EEquipmentSlotType::Count;
	
	UPROPERTY(NotReplicated)
	TObjectPtr<UD1EquipmentManagerComponent> EquipmentManager;
};

USTRUCT(BlueprintType)
struct FD1EquipmentList : public FFastArraySerializer
{
	GENERATED_BODY()

public:
	FD1EquipmentList() : EquipmentManager(nullptr) { }
	FD1EquipmentList(UD1EquipmentManagerComponent* InOwnerComponent) : EquipmentManager(InOwnerComponent) { }
	
public:
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	
private:
	void BroadcastChangedMessage(EEquipmentSlotType EquipmentSlotType, UD1ItemInstance* ItemInstance, int32 ItemCount);
	
public:
	const TArray<FD1EquipmentEntry>& GetAllEntries() const { return Entries; }
	
private:
	friend class UD1EquipmentManagerComponent;
	friend class UD1InventoryManagerComponent;
	friend class UD1ItemManagerComponent;

	UPROPERTY()
	TArray<FD1EquipmentEntry> Entries;
	
	UPROPERTY(NotReplicated)
	TObjectPtr<UD1EquipmentManagerComponent> EquipmentManager;
};

template<>
struct TStructOpsTypeTraits<FD1EquipmentList> : public TStructOpsTypeTraitsBase2<FD1EquipmentList>
{
	enum
	{
		WithNetDeltaSerializer = true
	};
};

UCLASS(BlueprintType)
class UD1EquipmentManagerComponent : public UPawnComponent
{
	GENERATED_BODY()
	
public:
	UD1EquipmentManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void InitializeComponent() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	virtual void ReadyForReplication() override;
	
public:
	/*
	int32 CanMoveOrMergeEquipment(UD1EquipmentManagerComponent* OtherComponent, EEquipmentSlotType FromEquipmentSlotType, EEquipmentSlotType ToEquipmentSlotType) const;
	int32 CanMoveOrMergeEquipment(UD1InventoryManagerComponent* OtherComponent, const FIntPoint& FromItemSlotPos, EEquipmentSlotType ToEquipmentSlotType) const;
	
	int32 CanMoveOrMergeEquipment_Quick(UD1EquipmentManagerComponent* OtherComponent, EEquipmentSlotType FromEquipmentSlotType, EEquipmentSlotType& OutToEquipmentSlotType) const;
	int32 CanMoveOrMergeEquipment_Quick(UD1InventoryManagerComponent* OtherComponent, const FIntPoint& FromItemSlotPos, EEquipmentSlotType& OutToEquipmentSlotType) const;
	int32 CanMoveOrMergeEquipment_Quick(int32 FromItemTemplateID, EItemRarity FromItemRarity, int32 FromItemCount, EEquipmentSlotType& OutToEquipmentSlotType) const;

	bool CanSwapEquipment(UD1EquipmentManagerComponent* OtherComponent, EEquipmentSlotType FromEquipmentSlotType, EEquipmentSlotType ToEquipmentSlotType) const;
	bool CanSwapEquipment(UD1InventoryManagerComponent* OtherComponent, const FIntPoint& FromItemSlotPos, EEquipmentSlotType ToEquipmentSlotType, FIntPoint& OutToItemSlotPos);
	
	bool CanSwapEquipment_Quick(UD1EquipmentManagerComponent* OtherComponent, EEquipmentSlotType FromEquipmentSlotType, EEquipmentSlotType& OutToEquipmentSlotType) const;
	bool CanSwapEquipment_Quick(UD1InventoryManagerComponent* OtherComponent, const FIntPoint& FromItemSlotPos, EEquipmentSlotType& OutToEquipmentSlotType, FIntPoint& OutToItemSlotPos);
	*/
	
	int32 CanAddEquipment(int32 ItemTemplateID, EItemRarity ItemRarity, int32 ItemCount, EEquipmentSlotType ToEquipmentSlotType) const;
	
public:
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void AddUnarmedEquipments(TSubclassOf<UD1ItemTemplate> LeftHandClass, TSubclassOf<UD1ItemTemplate> RightHandClass);
	
public:
	void AddEquipment_Unsafe(EEquipmentSlotType EquipmentSlotType, UD1ItemInstance* ItemInstance, int32 ItemCount);
	UD1ItemInstance* RemoveEquipment_Unsafe(EEquipmentSlotType EquipmentSlotType, int32 ItemCount);
	void SetEquipment(EEquipmentSlotType EquipmentSlotType, TSubclassOf<UD1ItemTemplate> ItemTemplateClass, EItemRarity ItemRarity, int32 ItemCount);
	
public:
	static bool IsWeaponSlot(EEquipmentSlotType EquipmentSlotType);
	static bool IsArmorSlot(EEquipmentSlotType EquipmentSlotType);
	static bool IsUtilitySlot(EEquipmentSlotType EquipmentSlotType);
	
	static bool IsSameEquipState(EEquipmentSlotType EquipmentSlotType, EEquipState WeaponEquipState);
	static bool IsSameWeaponHandType(EEquipmentSlotType EquipmentSlotType, EWeaponHandType WeaponHandType);
	static bool IsSameArmorType(EEquipmentSlotType EquipmentSlotType, EArmorType ArmorType);
	
	static bool IsPrimaryWeaponSlot(EEquipmentSlotType EquipmentSlotType);
	static bool IsSecondaryWeaponSlot(EEquipmentSlotType EquipmentSlotType);

	const UD1ItemInstance* FindPairItemInstance(const UD1ItemInstance* InBaseItemInstance, EEquipmentSlotType& OutEquipmentSlotType) const;
	
	bool IsAllEmpty(EEquipState EquipState) const;

	ALyraCharacter* GetCharacter() const;
	ALyraPlayerController* GetPlayerController() const;
	//UD1EquipManagerComponent* GetEquipManager() const;
	
	UD1ItemInstance* GetItemInstance(EEquipmentSlotType EquipmentSlotType) const;
	int32 GetItemCount(EEquipmentSlotType EquipmentSlotType) const;
	
	const TArray<FD1EquipmentEntry>& GetAllEntries() const;
	void GetAllWeaponItemInstances(TArray<UD1ItemInstance*>& OutItemInstances) const;

public:
	FOnEquipmentEntryChanged OnEquipmentEntryChanged;
	
private:
	friend class UD1ItemManagerComponent;
	friend class ULyraCheatManager;
	
	UPROPERTY(Replicated)
	FD1EquipmentList EquipmentList;
};

// 내 장비들을 관리하는 매니저

// FFastArraySerializer : 언리얼엔진에서 배열 데이터를 효율적으로 네트워크 복제하기 위한 도구
// 특징 : 
// 변경 감지 최적화: 배열의 모든 요소를 매번 복제하는 대신, 변경된 항목만 감지하여 네트워크로 전송합니다.
// 자동 패킷 생성 : 배열에 항목이 추가, 제거, 수정될 때 이를 감지하고 자동으로 적절한 네트워크 패킷을 생성합니다.
// 대규모 배열에 효율적 : 특히 아이템 인벤토리, 플레이어 목록, 게임 상태 등 항목이 자주 변경되는 대규모 배열에 유용합니다.
// 사용방법 : 
// 복제하려는 구조체에 FFastArraySerializerItem을 상속받게 합니다.
// 이 항목들의 배열을 관리하는 구조체에 FFastArraySerializer를 상속받습니다.
// 변경이 발생할 때마다 MarkItemDirty() 메서드를 호출하여 해당 항목이 변경되었음을 표시합니다.
