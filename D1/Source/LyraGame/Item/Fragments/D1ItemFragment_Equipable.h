#pragma once

#include "D1Define.h"
#include "GameplayTags.h"
#include "Item/D1ItemTemplate.h"
#include "D1ItemFragment_Equipable.generated.h"

class ULyraAbilitySet;
class UD1ItemInstance;

USTRUCT(BlueprintType)
struct FRarityStat
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleDefaultsOnly)
	EItemRarity Rarity = EItemRarity::Poor;

	UPROPERTY(EditDefaultsOnly)
	int32 Value = 0;
};

USTRUCT(BlueprintType)
struct FRarityStatSet
{
	GENERATED_BODY()

public:
	FRarityStatSet();
	
public:
	UPROPERTY(EditDefaultsOnly, meta=(Categories="SetByCaller"))
	FGameplayTag StatTag;
	
	UPROPERTY(EditDefaultsOnly, EditFixedSize)
	TArray<FRarityStat> RarityStats;
};

USTRUCT(BlueprintType)
struct FRarityStatRange
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleDefaultsOnly)
	EItemRarity Rarity = EItemRarity::Poor;
	
	UPROPERTY(EditDefaultsOnly)
	int32 MinValue = 0;

	UPROPERTY(EditDefaultsOnly)
	int32 MaxValue = 0;
};

USTRUCT(BlueprintType)
struct FRarityStatRangeSet
{
	GENERATED_BODY()

public:
	FRarityStatRangeSet();
	
public:
	UPROPERTY(EditDefaultsOnly, meta=(Categories="SetByCaller"))
	FGameplayTag StatTag;
	
	UPROPERTY(EditDefaultsOnly, EditFixedSize)
	TArray<FRarityStatRange> RarityStatRanges;
};

UCLASS(Abstract, Const)
class UD1ItemFragment_Equipable : public UD1ItemFragment
{
	GENERATED_BODY()
	
public:
	UD1ItemFragment_Equipable(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
protected:
	void AddStatTagStack(UD1ItemInstance* ItemInstance, const TArray<FRarityStatSet>& RarityStatSets) const;
	void AddStatTagStack(UD1ItemInstance* ItemInstance, const TArray<FRarityStatRangeSet>& RarityStatRangeSets) const;

public:
	bool IsEquipableClassType(ECharacterClassType ClassType) const;
	
public:
	EEquipmentType EquipmentType = EEquipmentType::Count;
	
	// 비트마스크를 사용하여 아이템을 장착할 수 있는 캐릭터 클래스 타입을 지정
	UPROPERTY(EditDefaultsOnly, meta=(Bitmask, BitmaskEnum="ECharacterClassType"))
	uint32 EquipableClassFlags = ((1 << (uint32)ECharacterClassType::Count) - 1);
	
	// 어떤 스킬을 사용할 수 있는지
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<const ULyraAbilitySet> BaseAbilitySet;
};
