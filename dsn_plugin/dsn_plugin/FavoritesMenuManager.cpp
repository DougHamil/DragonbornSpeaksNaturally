#include "FavoritesMenuManager.h"
#include "Log.h"
#include "SkyrimType.h"
#include "Equipper.h"
#include "SpeechRecognitionClient.h"
#include "skse64/GameAPI.h"
#include "skse64/GameRTTI.h"
#include "skse64/GameData.h"
#include "skse64/GameExtraData.h"
#include "skse64/GameTypes.h"
#include "skse64/PapyrusActor.h"
#include "skse64/GameInput.h"
#include "skse64/HashUtil.h"

FavoritesMenuManager* FavoritesMenuManager::instance = NULL;

FavoritesMenuManager* FavoritesMenuManager::getInstance() {
	if (!instance)
		instance = new FavoritesMenuManager();
	return instance;
}

FavoritesMenuManager::FavoritesMenuManager(){}

SInt32 fmCalcItemId(TESForm * form, BaseExtraList * extraList)
{
	if (!form || !extraList)
		return 0;

	const char * name = extraList->GetDisplayName(form);

	// No name in extra data? Use base form name
	if (!name)
	{
		TESFullName* pFullName = DYNAMIC_CAST(form, TESForm, TESFullName);
		if (pFullName)
			name = pFullName->name.data;
	}

	if (!name)
		return 0;

	return (SInt32)HashUtil::CRC32(name, form->formID & 0x00FFFFFF);
}

bool IsEquipmentSingleHanded(TESForm *item) {
	BGSEquipType * equipType = DYNAMIC_CAST(item, TESForm, BGSEquipType);
	if (!equipType)
		return false;

	BGSEquipSlot * equipSlot = equipType->GetEquipSlot();
	if (!equipSlot)
		return false;

	return (equipSlot == GetLeftHandSlot() || equipSlot == GetRightHandSlot());
}

std::vector<FavoriteMenuItem> ExtractFavorites(InventoryEntryData * inv) {
	std::vector<FavoriteMenuItem> menuItems;
	ExtendDataList* pExtendList = inv->extendDataList;

	if (pExtendList)
	{
		bool isHanded = IsEquipmentSingleHanded(inv->type);
		TESFullName *baseFullname = DYNAMIC_CAST(inv->type, TESForm, TESFullName);
		std::string baseName = std::string(baseFullname->name.data);
		SInt32 n = 0;
		BaseExtraList* itemExtraDataList = pExtendList->GetNthItem(n);
		while (itemExtraDataList)
		{
			// Check if item is hotkeyed
			if (ExtraHotkey * extraHotkey = static_cast<ExtraHotkey*>(itemExtraDataList->GetByType(kExtraData_Hotkey)))
			{
				std::string name = baseName;
				const char* modifiedName = itemExtraDataList->GetDisplayName(inv->type);
				if (modifiedName) {
					name = std::string(modifiedName);
				}
				SInt32 itemId = fmCalcItemId(inv->type, itemExtraDataList);

				if (ExtraTextDisplayData * textDisplayData = static_cast<ExtraTextDisplayData*>(itemExtraDataList->GetByType(kExtraData_TextDisplayData)))
				{
					if (textDisplayData->name.data) {
						name = std::string(textDisplayData->name.data);
						const char* displayName = itemExtraDataList->GetDisplayName(inv->type);
						itemId = (SInt32)HashUtil::CRC32(displayName, inv->type->formID & 0x00FFFFFF);
					}
				}

				FavoriteMenuItem entry = {
					inv->type->formID,
					itemId,
					name,
					1, // Equipment
					isHanded };

				menuItems.push_back(entry);
			}
			n++;
			itemExtraDataList = pExtendList->GetNthItem(n);
		}
	}

	return menuItems;
}

void FavoritesMenuManager::RefreshFavorites() {

	PlayerCharacter *player = (*g_thePlayer);
	if (player != NULL) {

		// Clear current favorites
		favorites.clear();

		// Equipment
		ExtraContainerChanges* pContainerChanges = static_cast<ExtraContainerChanges*>(player->extraData.GetByType(kExtraData_ContainerChanges));
		if (pContainerChanges && pContainerChanges->data) {
			for (EntryDataList::Iterator it = pContainerChanges->data->objList->Begin(); !it.End(); ++it)
			{
				InventoryEntryData *inv = it.Get();
				TESForm* type = inv->type;
				if (type) {
					HotkeyData data = pContainerChanges->FindHotkey(type);
					if (data.pHotkey)
					{
						std::vector<FavoriteMenuItem> itemEntries = ExtractFavorites(inv);
						for (int i = 0; i < itemEntries.size(); i++) {
							favorites.push_back(itemEntries[i]);
						}
					}
				}
			}
		}

		// Spells/Shouts
		FakeMagicFavorites * magicFavorites = (FakeMagicFavorites*)MagicFavorites::GetSingleton();
		if (magicFavorites) {
			UnkFormArray spellArray = magicFavorites->spells;
			for (int i = 0; i < spellArray.count; i++) {
				TESForm* spellForm = spellArray.entries[i];
				if (spellForm) {
					SpellItem *spellItem = DYNAMIC_CAST(spellForm, TESForm, SpellItem);
					if (spellItem) {
						FavoriteMenuItem entry = {
							spellForm->formID,
							0,
							std::string(spellItem->fullName.name.data),
							2, // Spell
							true };
						favorites.push_back(entry);
					}

					TESShout *shout = DYNAMIC_CAST(spellForm, TESForm, TESShout);
					if (shout) {

						FavoriteMenuItem entry = {
						shout->formID,
						0,
						std::string(shout->fullName.name.data),
						3, // Shout
						false };

						favorites.push_back(entry);
					}
				}
			}
		}

		std::string command = "FAVORITES";
		for (int i = 0; i < favorites.size(); i++) {
			FavoriteMenuItem favorite = favorites[i];
			command += "|" + favorite.fullname + "," + std::to_string(favorite.TESFormId) + "," + std::to_string(favorite.itemId) + "," + std::to_string(favorite.isHanded) + "," + std::to_string(favorite.itemType);
		}
		SpeechRecognitionClient::getInstance()->WriteLine(command);
	}
}

static std::vector<std::string> split(const std::string &s, char delim) {
	std::stringstream ss(s);
	std::string item;
	std::vector<std::string> tokens;
	while (std::getline(ss, item, delim)) {
		tokens.push_back(item);
	}
	return tokens;
}

EquipItem parseEquipItem(std::string command) {
	std::vector<std::string> tokens = split(command, ';');

	EquipItem item = {
		(UInt32)std::atoi(tokens[0].c_str()),
		(SInt32)std::atoi(tokens[1].c_str()),
		(UInt8)std::atoi(tokens[2].c_str()),
		(SInt32)std::atoi(tokens[3].c_str())
	};

	return item;
}

enum
{
	kSlotId_Default = 0,
	kSlotId_Right = 1,
	kSlotId_Left = 2
};


void FavoritesMenuManager::ProcessEquipCommands() {

	PlayerCharacter *player = (*g_thePlayer);
	SpeechRecognitionClient *client = SpeechRecognitionClient::getInstance();
	EquipManager *equipManager = EquipManager::GetSingleton();
	std::string equipStr = client->PopEquip();
	if (player && equipManager && equipStr != "") {
		EquipItem equipItem = parseEquipItem(equipStr);
		TESForm * form = LookupFormByID(equipItem.TESFormId);
		std::string hand = equipItem.hand == 1 ? "right" : "left";
		if (form) {
			std::stringstream formIdAsHex;

			switch (equipItem.itemType) {
			case 1: // Item
				Equipper::EquipItem(player, form, equipItem.itemId, equipItem.hand);
				break;
			case 2: // Spell
				formIdAsHex << std::hex << equipItem.TESFormId;
				if (equipItem.hand == 0) {
					client->EnqueueCommand("player.equipspell " + formIdAsHex.str() + " left");
					client->EnqueueCommand("player.equipspell " + formIdAsHex.str() + " right");
				} else {
					client->EnqueueCommand("player.equipspell " + formIdAsHex.str() + " " + hand);
				}
				break;
			case 3: // Shout
				formIdAsHex << std::hex << equipItem.TESFormId;
				client->EnqueueCommand("player.equipshout " + formIdAsHex.str());
				PlayerControls * controls = PlayerControls::GetSingleton();
				break;
			}
		}

	}
}



