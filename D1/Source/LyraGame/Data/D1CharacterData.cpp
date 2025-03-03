// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/D1CharacterData.h"
#include "System/LyraAssetManager.h"
#include "UObject/ObjectSaveContext.h"

const UD1CharacterData& UD1CharacterData::Get()
{
	// ULyraAssetManager의 싱글톤 인스턴스를 먼저 반환한 다음에 해당 클래스의 메서드를 실행한 것임.
	return ULyraAssetManager::Get().GetCharacterData();
}

#if WITH_EDITOR
void UD1CharacterData::PreSave(FObjectPreSaveContext ObjectSaveContext)
{
	Super::PreSave(ObjectSaveContext);

	// 캐릭터 스킨 타입에 따라 오름차순으로
	DefaultArmorMeshMap.KeySort([](const ECharacterSkinType A, const ECharacterSkinType B)
		{
			return (A < B);
		});
}
#endif // WITH_EDITOR

const FD1DefaultArmorMeshSet& UD1CharacterData::GetDefaultArmorMeshSet(ECharacterSkinType CharacterSkinType) const
{
	if (DefaultArmorMeshMap.Contains(CharacterSkinType) == false)
	{
		static FD1DefaultArmorMeshSet EmptyEntry;
		return EmptyEntry;
	}

	// Find : 키가 없으면 nullptr 반환,
	// FindChecked : 키가 없으면 크래시 발생
	// 다만, 위에서 Contains로 키의 존재 여부를 확인했으므로 안전
	return DefaultArmorMeshMap.FindChecked(CharacterSkinType);
}
