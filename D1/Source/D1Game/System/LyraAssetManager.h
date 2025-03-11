// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/AssetManager.h"
#include "LyraAssetManagerStartupJob.h"
#include "Templates/SubclassOf.h"
#include "Data/D1AssetData.h"
#include "Data/D1UIData.h"
#include "LyraAssetManager.generated.h"

class UPrimaryDataAsset;

class ULyraGameData;
class ULyraPawnData;
class UD1CharacterData;
class UD1ItemData;
class UD1ClassData;
class UD1AssetData;
class UD1CheatData;

struct FLyraBundles
{
	static const FName Equipped;
};


/**
 * ULyraAssetManager
 *
 *	Game implementation of the asset manager that overrides functionality and stores game-specific types.
 *	It is expected that most games will want to override AssetManager as it provides a good place for game-specific loading logic.
 *	This class is used by setting 'AssetManagerClassName' in DefaultEngine.ini.
 */
UCLASS(Config = Game)
class ULyraAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:

	ULyraAssetManager();

	// Returns the AssetManager singleton object.
	static ULyraAssetManager& Get();

	template<typename AssetType>
	static AssetType* GetAssetByPath(const TSoftObjectPtr<AssetType>& AssetPointer, bool bKeepInMemory = true);

	template<typename AssetType>
	static AssetType* GetAssetByName(const FName& AssetName, bool bKeepInMemory = true);

	template<typename AssetType>
	static TSubclassOf<AssetType> GetSubclassByPath(const TSoftClassPtr<AssetType>& AssetPointer, bool bKeepInMemory = true);

	template<typename AssetType>
	static TSubclassOf<AssetType> GetSubclassByName(const FName& AssetName, bool bKeepInMemory = true);

	// Logs all assets currently loaded and tracked by the asset manager.
	static void DumpLoadedAssets();

	const ULyraGameData& GetGameData();
	const ULyraPawnData* GetDefaultPawnData() const;
	const UD1CharacterData& GetCharacterData();
	const UD1ItemData& GetItemData();
	const UD1ClassData& GetClassData();
	const UD1AssetData& GetAssetData();
	const UD1UIData& GetUIData();
	const UD1CheatData& GetCheatData();

protected:
	template <typename GameDataClass>
	const GameDataClass& GetOrLoadTypedGameData(const TSoftObjectPtr<GameDataClass>& DataPath)
	{
		if (TObjectPtr<UPrimaryDataAsset> const * pResult = GameDataMap.Find(GameDataClass::StaticClass()))
		{
			return *CastChecked<GameDataClass>(*pResult);
		}

		// Does a blocking load if needed
		return *CastChecked<const GameDataClass>(LoadGameDataOfClass(GameDataClass::StaticClass(), DataPath, GameDataClass::StaticClass()->GetFName()));
	}


	static UObject* SynchronousLoadAsset(const FSoftObjectPath& AssetPath);
	static bool ShouldLogAssetLoads();

	// Thread safe way of adding a loaded asset to keep in memory.
	void AddLoadedAsset(const UObject* Asset);

	//~UAssetManager interface
	virtual void StartInitialLoading() override;
#if WITH_EDITOR
	virtual void PreBeginPIE(bool bStartSimulate) override;
#endif
	//~End of UAssetManager interface

	UPrimaryDataAsset* LoadGameDataOfClass(TSubclassOf<UPrimaryDataAsset> DataClass, const TSoftObjectPtr<UPrimaryDataAsset>& DataClassPath, FPrimaryAssetType PrimaryAssetType);

protected:

	// Global game data asset to use.
	UPROPERTY(Config)
	TSoftObjectPtr<ULyraGameData> GameDataPath;

	// Loaded version of the game data
	UPROPERTY(Transient)
	TMap<TObjectPtr<UClass>, TObjectPtr<UPrimaryDataAsset>> GameDataMap;

	// Pawn data used when spawning player pawns if there isn't one set on the player state.
	UPROPERTY(Config)
	TSoftObjectPtr<ULyraPawnData> DefaultPawnData;

	UPROPERTY(Config)
	TSoftObjectPtr<UD1CharacterData> CharacterDataPath;

	UPROPERTY(Config)
	TSoftObjectPtr<UD1ItemData> ItemDataPath;

	UPROPERTY(Config)
	TSoftObjectPtr<UD1ClassData> ClassDataPath;

	UPROPERTY(Config)
	TSoftObjectPtr<UD1AssetData> AssetDataPath;

	UPROPERTY(Config)
	TSoftObjectPtr<UD1UIData> UIDataPath;

	UPROPERTY(Config)
	TSoftObjectPtr<UD1CheatData> CheatDataPath;

private:
	// Flushes the StartupJobs array. Processes all startup work.
	void DoAllStartupJobs();

	// Sets up the ability system
	void InitializeGameplayCueManager();

	// Called periodically during loads, could be used to feed the status to a loading screen
	void UpdateInitialGameContentLoadPercent(float GameContentPercent);

	// The list of tasks to execute on startup. Used to track startup progress.
	TArray<FLyraAssetManagerStartupJob> StartupJobs;

private:
	
	// Assets loaded and tracked by the asset manager.
	UPROPERTY()
	TSet<TObjectPtr<const UObject>> LoadedAssets;

	// Used for a scope lock when modifying the list of load assets.
	FCriticalSection LoadedAssetsCritical;
};


// 보면 TSoftObjectPtr, TSoftClassPtr로 경로를 받아와서 해당 에셋을 갖고 오는 기능을 이용 중인데
// 경우에 따라서 모든 것을 다 경로로 관리하면 조금 복잡할 것임.
// -> Key, Value 값을 통해 에셋을 관리하면 조금 더 편리하게 관리할 수 있지 않을까? (ex. 유니티의 addressable)
// key : 이름, value : 경로가 될 듯? (그럼 key 값으로만 에셋을 찾을 수 있을 것이다)
template<typename AssetType>
AssetType* ULyraAssetManager::GetAssetByPath(const TSoftObjectPtr<AssetType>& AssetPointer, bool bKeepInMemory)
{
	AssetType* LoadedAsset = nullptr;

	const FSoftObjectPath& AssetPath = AssetPointer.ToSoftObjectPath();

	if (AssetPath.IsValid())
	{
		LoadedAsset = AssetPointer.Get();
		if (!LoadedAsset)
		{
			LoadedAsset = Cast<AssetType>(SynchronousLoadAsset(AssetPath));
			ensureAlwaysMsgf(LoadedAsset, TEXT("Failed to load asset [%s]"), *AssetPointer.ToString());
		}

		if (LoadedAsset && bKeepInMemory)
		{
			// Added to loaded asset list.
			Get().AddLoadedAsset(Cast<UObject>(LoadedAsset));
		}
	}

	return LoadedAsset;
}

template <typename AssetType>
AssetType* ULyraAssetManager::GetAssetByName(const FName& AssetName, bool bKeepInMemory)
{
	const UD1AssetData& AssetData = Get().GetAssetData();
	const FSoftObjectPath& AssetPath = AssetData.GetAssetPathByName(AssetName);
	TSoftObjectPtr<AssetType> AssetPtr(AssetPath);
	return GetAssetByPath<AssetType>(AssetPtr, bKeepInMemory);
}

template<typename AssetType>
TSubclassOf<AssetType> ULyraAssetManager::GetSubclassByPath(const TSoftClassPtr<AssetType>& AssetPointer, bool bKeepInMemory)
{
	TSubclassOf<AssetType> LoadedSubclass;

	const FSoftObjectPath& AssetPath = AssetPointer.ToSoftObjectPath();

	if (AssetPath.IsValid())
	{
		LoadedSubclass = AssetPointer.Get();
		if (!LoadedSubclass)
		{
			LoadedSubclass = Cast<UClass>(SynchronousLoadAsset(AssetPath));
			ensureAlwaysMsgf(LoadedSubclass, TEXT("Failed to load asset class [%s]"), *AssetPointer.ToString());
		}

		if (LoadedSubclass && bKeepInMemory)
		{
			// Added to loaded asset list.
			Get().AddLoadedAsset(Cast<UObject>(LoadedSubclass));
		}
	}

	return LoadedSubclass;
}

template <typename AssetType>
TSubclassOf<AssetType> ULyraAssetManager::GetSubclassByName(const FName& AssetName, bool bKeepInMemory)
{
	const UD1AssetData& AssetData = Get().GetAssetData();
	const FSoftObjectPath& AssetPath = AssetData.GetAssetPathByName(AssetName);

	FString AssetPathString = AssetPath.GetAssetPathString();
	AssetPathString.Append(TEXT("_C"));

	FSoftClassPath ClassPath(AssetPathString);
	TSoftClassPtr<AssetType> ClassPtr(ClassPath);
	return GetSubclassByPath<AssetType>(ClassPtr, bKeepInMemory);
}
