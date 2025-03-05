#include "D1CosmeticManagerComponent.h"

#include "D1Define.h"
#include "Actors/D1ArmorBase.h"
#include "Character/LyraCharacter.h"
#include "Data/D1CharacterData.h"
//#include "Item/Fragments/D1ItemFragment_Equipable_Armor.h"
#include "System/LyraAssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(D1CosmeticManagerComponent)

UD1CosmeticManagerComponent::UD1CosmeticManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
}

void UD1CosmeticManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	InitializeManager();
}

void UD1CosmeticManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HeadSlot)
	{
		HeadSlot->DestroyComponent();
	}
	
	for (UChildActorComponent* CosmeticSlot : CosmeticSlots)
	{
		if (CosmeticSlot)
		{
			CosmeticSlot->DestroyComponent();
		}
	}
	
	Super::EndPlay(EndPlayReason);
}

void UD1CosmeticManagerComponent::RefreshArmorMesh(EArmorType ArmorType /*, const UD1ItemFragment_Equipable_Armor* ArmorFragment*/)
{
	if (ArmorType == EArmorType::Count)
		return;
	
	InitializeManager();
	
	/*if (ArmorFragment)
	{
		if (ArmorFragment == nullptr || ArmorFragment->ArmorType != ArmorType)
			return;
		
		SetPrimaryArmorMesh(ArmorType, ArmorFragment->ArmorMesh);
		
		if (ArmorType == EArmorType::Chest)
		{
			if (UChildActorComponent* CosmeticSlot = CosmeticSlots[(int32)EArmorType::Legs])
			{
				if (AD1ArmorBase* CosmeticActor = Cast<AD1ArmorBase>(CosmeticSlot->GetChildActor()))
				{
					CosmeticActor->ShouldUseSecondaryMesh(ArmorFragment->bIsFullBody);
				}
			}
		}
		else if (ArmorType == EArmorType::Helmet)
		{
			if (HeadSlot)
			{
				if (AD1ArmorBase* CosmeticActor = Cast<AD1ArmorBase>(HeadSlot->GetChildActor()))
				{
					CosmeticActor->ShouldUseSecondaryMesh(true);
				}
			}
		}
	}
	else*/
	{
		if (ArmorType == EArmorType::Chest)
		{
			if (UChildActorComponent* CosmeticSlot = CosmeticSlots[(int32)EArmorType::Legs])
			{
				if (AD1ArmorBase* CosmeticActor = Cast<AD1ArmorBase>(CosmeticSlot->GetChildActor()))
				{
					CosmeticActor->ShouldUseSecondaryMesh(false);
				}
			}
		}
		else if (ArmorType == EArmorType::Helmet)
		{
			if (HeadSlot)
			{
				if (AD1ArmorBase* CosmeticActor = Cast<AD1ArmorBase>(HeadSlot->GetChildActor()))
				{
					CosmeticActor->ShouldUseSecondaryMesh(false);
				}
			}
		}

		SetPrimaryArmorMesh(ArmorType, nullptr);
	}
}

void UD1CosmeticManagerComponent::SetPrimaryArmorMesh(EArmorType ArmorType, TSoftObjectPtr<USkeletalMesh> ArmorMeshPtr)
{
	if (ArmorType == EArmorType::Count)
		return;
	
	InitializeManager();
	
	if (UChildActorComponent* CosmeticSlot = CosmeticSlots[(int32)ArmorType])
	{
		if (AD1ArmorBase* CosmeticActor = Cast<AD1ArmorBase>(CosmeticSlot->GetChildActor()))
		{
			if (ArmorMeshPtr.IsNull())
			{
				CosmeticActor->SetPrimaryArmorMesh(nullptr);
			}
			else
			{
				USkeletalMesh* ArmorMesh = ULyraAssetManager::GetAssetByPath<USkeletalMesh>(ArmorMeshPtr);
				CosmeticActor->SetPrimaryArmorMesh(ArmorMesh);
			}
		}
	}
}

void UD1CosmeticManagerComponent::GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const
{
	if (HeadSlot)
	{
		if (AD1ArmorBase* CosmeticActor = Cast<AD1ArmorBase>(HeadSlot->GetChildActor()))
		{
			OutMeshComponents.Add(CosmeticActor->GetMeshComponent());
		}
	}
	
	for (UChildActorComponent* CosmeticSlot : CosmeticSlots)
	{
		if (CosmeticSlot)
		{
			if (AD1ArmorBase* CosmeticActor = Cast<AD1ArmorBase>(CosmeticSlot->GetChildActor()))
			{
				OutMeshComponents.Add(CosmeticActor->GetMeshComponent());
			}
		}
	}
}

void UD1CosmeticManagerComponent::InitializeManager()
{
	if (bInitialized)
		return;

	bInitialized = true;

	const int32 ArmorTypeCount = (int32)EArmorType::Count;
	// 배열의 크기를 설정하고 모든 요소를 기본값으로 초기화하는 메서드
	CosmeticSlots.SetNumZeroed(ArmorTypeCount);
	
	check(CosmeticSlotClass);
	check(CharacterSkinType != ECharacterSkinType::Count);
	
	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		// Cosmetic은 비주얼적인 것이다보니 클라에서만
		if (Character->IsNetMode(NM_DedicatedServer) == false)
		{
			const UD1CharacterData& CharacterData = ULyraAssetManager::Get().GetCharacterData();
			const FD1DefaultArmorMeshSet& DefaultArmorMeshSet = CharacterData.GetDefaultArmorMeshSet(CharacterSkinType);
			
			// 여기서 (머리) 생성
			HeadSlot = SpawnCosmeticSlotActor(DefaultArmorMeshSet.HeadDefaultMesh, DefaultArmorMeshSet.HeadSecondaryMesh, NAME_None, nullptr);
			
			for (int32 i = 0; i < (int32)EArmorType::Count; i++)
			{
				EArmorType ArmorType = (EArmorType)i;
				FName SkinMaterialSlotName;
				TSoftObjectPtr<UMaterialInterface> SkinMaterial;
				
				if (ArmorType == EArmorType::Helmet || ArmorType == EArmorType::Chest || ArmorType == EArmorType::Hands)
				{
					SkinMaterialSlotName = FName("UpperBody");
					SkinMaterial = DefaultArmorMeshSet.UpperBodySkinMaterial;
				}
				else if (ArmorType == EArmorType::Legs || ArmorType == EArmorType::Foot)
				{
					SkinMaterialSlotName = FName("LowerBody");
					SkinMaterial = DefaultArmorMeshSet.LowerBodySkinMaterial;
				}
				
				CosmeticSlots[i] = SpawnCosmeticSlotActor(DefaultArmorMeshSet.DefaultMeshEntries[i], DefaultArmorMeshSet.SecondaryMeshEntries[i], SkinMaterialSlotName, SkinMaterial);
			}
		}
	}
}

// 참고) 유니티 엔진에서는 게임 오브젝트를 여러 개 만든 다음에 하나를 다른 애한테 붙이고 하는 게 편리했는데 언리얼의 경우 그렇게 되어 있지 않음....
// -> 언리얼 엔진에서 한 액터를 다른 액터에 붙이는 방법 중 하나가 바로 ChildActorComponent
UChildActorComponent* UD1CosmeticManagerComponent::SpawnCosmeticSlotActor(TSoftObjectPtr<USkeletalMesh> InDefaultMesh, TSoftObjectPtr<USkeletalMesh> InSecondaryMesh, FName InSkinMaterialSlotName, TSoftObjectPtr<UMaterialInterface> InSkinMaterial)
{
	UChildActorComponent* CosmeticComponent = nullptr;
	
	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		USceneComponent* ComponentToAttachTo = Character->GetMesh();
		// 새 자식 액터 컴포넌트를 동적으로 생성
		CosmeticComponent = NewObject<UChildActorComponent>(Character);
		CosmeticComponent->SetupAttachment(ComponentToAttachTo);
		// 자식 액터의 클래스를 설정
		CosmeticComponent->SetChildActorClass(CosmeticSlotClass);
		// 컴포넌트를 등록하여 엔진이 이를 인식하게 함 (런타임에 직접 컴포넌트를 만들어서 붙일 경우에는 등록을 이렇게 수동으로 해줘야 됨!!!!)
		CosmeticComponent->RegisterComponent();

		if (AD1ArmorBase* SpawnedActor = Cast<AD1ArmorBase>(CosmeticComponent->GetChildActor()))
		{
			if (USceneComponent* SpawnedRootComponent = SpawnedActor->GetRootComponent())
			{
				// 자식 액터의 루트 컴포넌트에 AddTickPrerequisiteComponent를 호출하여 부모 컴포넌트가 먼저 틱(tick)되도록 보장
				// 우리가 관리하는 SpawnedRootComponent가 ComponentToAttachTo에 종속적이기 때문에 업데이트 순서를 맞춰줘야 됨.
				SpawnedRootComponent->AddTickPrerequisiteComponent(ComponentToAttachTo);
			}
			
			SpawnedActor->InitializeActor(InDefaultMesh, InSecondaryMesh, InSkinMaterialSlotName, InSkinMaterial);
		}
	}
	
	return CosmeticComponent;
}

