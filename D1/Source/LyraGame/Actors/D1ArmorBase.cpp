#include "D1ArmorBase.h"

#include "System/LyraAssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(D1ArmorBase)

AD1ArmorBase::AD1ArmorBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

    ArmorMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>("ArmorMeshComponent");
	ArmorMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetRootComponent(ArmorMeshComponent);
}

void AD1ArmorBase::InitializeActor(TSoftObjectPtr<USkeletalMesh> InDefaultArmorMesh, TSoftObjectPtr<USkeletalMesh> InSecondaryArmorMesh, FName InSkinMaterialSlotName, TSoftObjectPtr<UMaterialInterface> InSkinMaterial)
{
	DefaultArmorMesh = InDefaultArmorMesh;
	SecondaryArmorMesh = (InSecondaryArmorMesh.IsNull() == false ? InSecondaryArmorMesh : InDefaultArmorMesh);
	
	if (InSkinMaterialSlotName.IsNone() == false && InSkinMaterial.IsNull() == false)
	{
		SkinMaterialSlotName = InSkinMaterialSlotName;
		SkinMaterial = InSkinMaterial;
	}

	SetArmorMesh(DefaultArmorMesh);
}

void AD1ArmorBase::SetPrimaryArmorMesh(USkeletalMesh* InPrimaryArmorMesh)
{
	PrimaryArmorMesh = InPrimaryArmorMesh;
	RefreshArmorMesh();
}

void AD1ArmorBase::ShouldUseSecondaryMesh(bool bInShouldUseSecondary)
{
	bShouldUseSecondaryArmor = bInShouldUseSecondary;
	RefreshArmorMesh();
}

void AD1ArmorBase::RefreshArmorMesh()
{
	TSoftObjectPtr<USkeletalMesh> ArmorMesh;
	if (bShouldUseSecondaryArmor)
	{
		ArmorMesh = SecondaryArmorMesh;
	}
	else
	{
		ArmorMesh = PrimaryArmorMesh ? PrimaryArmorMesh : DefaultArmorMesh;
	}

	SetArmorMesh(ArmorMesh);
}

void AD1ArmorBase::SetArmorMesh(TSoftObjectPtr<USkeletalMesh> InArmorMesh)
{
	USkeletalMesh* LoadedArmorMesh = nullptr;
	if (InArmorMesh.IsNull() == false)
	{
		// 여기서 메모리 관리를 어떻게 할지를 고민해볼 필요가 있음.
		// GetAsset() 한번 타고 들어가보기
		LoadedArmorMesh = ULyraAssetManager::GetAsset<USkeletalMesh>(InArmorMesh);
	}
	
	// 해당 메쉬 설정하는 부분
	ArmorMeshComponent->SetSkeletalMesh(LoadedArmorMesh);
	ArmorMeshComponent->EmptyOverrideMaterials();

	if (SkinMaterialSlotName.IsNone() == false && SkinMaterial.IsNull() == false)
	{
		UMaterialInterface* LoadedMaterial = ULyraAssetManager::GetAsset<UMaterialInterface>(SkinMaterial);
		ArmorMeshComponent->SetMaterialByName(SkinMaterialSlotName, LoadedMaterial);
	}
}

// CharacterData를 참고해서 여러가지 액터를 만들어서 최종적으로 우리 캐릭터의 룩을 조립해주는 CosmeticManager
// 매니저니깐 싱글톤? No! 싱글 게임이라면 싱글톤을 사용하더라도 크게 문제가 없진 않지만 온라인 게임에서는 각기 컴포넌트가 정말로
// 딱 하나만 있어가지고 모두가 공용으로 사용하는 것인지 아니면 여러 캐릭터마다 자기만의 매니저를 따로 가지고 있을 것인지에 따라 설계를 다르게 해줘야 됨
// ex. 인벤토리 같은 경우 모든 캐릭터가 다 같은 인벤토리를 들고 있는 게 아닐 것임