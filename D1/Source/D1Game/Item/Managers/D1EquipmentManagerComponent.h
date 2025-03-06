#pragma once

#include "D1Define.h"
#include "Components/PawnComponent.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "D1EquipmentManagerComponent.generated.h"

class UD1EquipManagerComponent;
class ALyraCharacter;
class ALyraPlayerController;
class UD1ItemInstance;
class UD1ItemTemplate;
class UD1EquipmentManagerComponent;
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
	// Good (삼총사) - 언리얼 엔진이 기본적으로 직렬화를 해주지만 더 효율을 높이기 위함.
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
	UD1EquipManagerComponent* GetEquipManager() const;
	
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

// 내가 현재 장착중인 장비들을 관리하는 매니저
// 1번 2번 3번 무기 중에 뭘 쓸 꺼냐

// Equipment는 내가 현재 장착 중인 아이템
// equip은 내가 장착한 아이템 중에서 현재 사용하는 아이템

/**
* 
*	D1EquipmentManagerComponent.h:
	캐릭터가 소유하고 있는 장비 아이템들의 목록을 관리합니다.
	아이템 인스턴스와 개수를 저장하는 인벤토리 시스템의 일종입니다.
	모든 장비 슬롯(무기, 방어구, 유틸리티 등)에 어떤 아이템이 할당되어 있는지 관리합니다.
	네트워크 레플리케이션을 통해 서버의 장비 상태가 클라이언트에 동기화됩니다.

	D1EquipManagerComponent.h:
	실제로 캐릭터에 장착된(시각적으로 표시되는) 장비를 관리합니다.
	장비를 실제 액터로 생성하고 캐릭터에 부착하는 역할을 합니다.
	현재 장비 상태(EquipState)를 관리하며 장비 간 전환을 처리합니다.
	장비에 연결된 능력이나 효과를 적용/제거합니다.
*/


// FFastArraySerializer : 언리얼엔진에서 배열 데이터를 효율적으로 네트워크 복제하기 위한 도구
// 특징 : 
// 변경 감지 최적화: 배열의 모든 요소를 매번 복제하는 대신, 변경된 항목만 감지하여 네트워크로 전송합니다.
// 자동 패킷 생성 : 배열에 항목이 추가, 제거, 수정될 때 이를 감지하고 자동으로 적절한 네트워크 패킷을 생성합니다.
// 대규모 배열에 효율적 : 특히 아이템 인벤토리, 플레이어 목록, 게임 상태 등 항목이 자주 변경되는 대규모 배열에 유용합니다.
// 사용방법 : 
// 복제하려는 구조체에 FFastArraySerializerItem을 상속받게 합니다.
// 이 항목들의 배열을 관리하는 구조체에 FFastArraySerializer를 상속받습니다.
// 변경이 발생할 때마다 MarkItemDirty() 메서드를 호출하여 해당 항목이 변경되었음을 표시합니다.

// UD1EquipmentManagerComponent라는 것은 실제로 아이템 인스턴스를 메모리 상에서 관리하는 개념이고
// 이제 이거를 실제로 비주얼라이징해서 인게임에서 액터로 만들어서 방패나 검을 들려면은 D1EquipManagerComponent로

// 레플리케이션을 하는 주체는 액터임
// 액터한테 붙이면 모든게 다 적용이 되지만 아무 코드에나 UPROPERTY(Replicated)를 넣었다고 해서 되는 것은 아님
// 기본적으로 컴포넌트 같은 경우는 원래 레플리케이션이 안되는 것이 일반적이긴 한데
// 그럼에도 불구하고 경우에 따라서 우리가 만드는 아이템 목록 같은 것은 
// 우리가 컴포넌트로 관리하고 있음에도 불구하고 레플리케이션 대상이 되기를 바라고 있음
// 왜냐하면 아이템이 추가 되었거나 삭제되었다는 사실은 서버가 그 행위를 했으면 클라도 통보를 받아야 하므로 레플리케이션 대상이 되는 것이 Good
// 관련된 문서 :  ComponentReplication 찾아보기

// 직렬화
// 메모리에 있는 객체나 데이터 구조를 네트워크로 전송하기 위해 바이트 스트림으로 변환하는 과정
// 메모리 주소값은 각 컴퓨터마다 다르므로, 원시 메모리 데이터를 그대로 전송할 수 없음
// 따라서 양쪽(서버와 클라이언트)이 모두 이해할 수 있는 규약에 따라 데이터를 "납작하게"(직렬화) 만들어 전송
// 언리얼 엔진은 이러한 직렬화 과정을 내부적으로 처리하며, 리플리케이션 시스템의 기본이 된다.