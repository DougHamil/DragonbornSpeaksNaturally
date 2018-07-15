#pragma once

#include <string>
#include <vector>
#include "common/IPrefix.h"
#include "skse64/GameTypes.h"

struct FakeMagicFavorites {
	UInt64 vtable;
	UInt64			unk008;		// 08
	UnkFormArray	spells;		// 10
	UnkFormArray	hotkeys;	// 28
};

struct FavoriteMenuItem {
	UInt32 TESFormId;
	SInt32 itemId;
	std::string fullname;
	UInt8 itemType;
	bool isHanded;	// True if user must specify "left" or "right" in equip commands
};

struct EquipItem {
	UInt32 TESFormId;
	SInt32 itemId;
	UInt8 itemType;
	SInt32 hand; // 0 = no hand specified, 1 = right hand, 2 = left hand
};

class FavoritesMenuManager
{
	static FavoritesMenuManager* instance;

public:
	static FavoritesMenuManager* getInstance();
	void RefreshFavorites();
	void ProcessEquipCommands();
private:
	FavoritesMenuManager();
	std::vector<FavoriteMenuItem> favorites;
	std::string lastFavoritesCommand;
};