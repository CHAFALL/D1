#pragma once

#include "D1Define.h"
#include "AbilitySystem/LyraAbilitySourceInterface.h"
#include "System/D1GameplayTagStack.h"
#include "D1ItemInstance.generated.h"

USTRUCT(BlueprintType)
struct FD1ItemRarityProbability
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	EItemRarity Rarity = EItemRarity::Poor;

	UPROPERTY(EditAnywhere)
	float Probability = 0;
};

// 보면 Actor가 아닌 UObject로 되어있음을 알 수 있다.
// 참고) UObject를 상속 받으면 언리얼에서 관리하는 메모리로 우리가 생성할 수 있어
// UObject라는 함수를 이용해서 객체를 만들면 굳이 우리가 delete를 하지 않아도 언리얼 GC 상으로 처리가 돼서
// 알아서 날아간다는 장점이 있음
UCLASS(BlueprintType)
class UD1ItemInstance : public UObject, public ILyraAbilitySourceInterface
{
	GENERATED_BODY()
	
public:
	UD1ItemInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual float GetDistanceAttenuation(float Distance, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr) const override;
	virtual float GetPhysicalMaterialAttenuation(const UPhysicalMaterial* PhysicalMaterial, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr) const override;
	
public:
	void Init(int32 InItemTemplateID, EItemRarity InItemRarity);

public:
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void AddStatTagStack(FGameplayTag StatTag, int32 StackCount);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void RemoveStatTagStack(FGameplayTag StatTag);

public:
	static EItemRarity DetermineItemRarity(const TArray<FD1ItemRarityProbability>& ItemProbabilities);
	
public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
	int32 GetItemTemplateID() const { return ItemTemplateID; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	EItemRarity GetItemRarity() const { return ItemRarity; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool HasStatTag(FGameplayTag StatTag) const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	int32 GetStackCountByTag(FGameplayTag StatTag) const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	const FD1GameplayTagStackContainer& GetStatContainer() const { return StatContainer; }

public:
	UFUNCTION(BlueprintCallable, BlueprintPure="false", meta=(DeterminesOutputType="FragmentClass"))
	const UD1ItemFragment* FindFragmentByClass(TSubclassOf<UD1ItemFragment> FragmentClass) const;

	template <typename FragmentClass>
	const FragmentClass* FindFragmentByClass() const
	{
		return (FragmentClass*)FindFragmentByClass(FragmentClass::StaticClass());
	}
	
private:
#if UE_WITH_IRIS
	virtual void RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags) override;
#endif // UE_WITH_IRIS
	
private:
	UPROPERTY(Replicated)
	int32 ItemTemplateID = INDEX_NONE;

	UPROPERTY(Replicated)
	EItemRarity ItemRarity = EItemRarity::Poor;
	
	UPROPERTY(Replicated)
	FD1GameplayTagStackContainer StatContainer;
};
