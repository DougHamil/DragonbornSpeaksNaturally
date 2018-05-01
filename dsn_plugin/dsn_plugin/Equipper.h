#pragma once
#include "common/IPrefix.h"
#include "skse64/GameAPI.h"
#include "skse64/GameRTTI.h"
#include "skse64/GameData.h"
#include "skse64/GameExtraData.h"
#include "skse64/GameTypes.h"
#include "skse64/PapyrusActor.h"
#include "skse64/GameInput.h"
#include "skse64/HashUtil.h"

class Equipper
{
public:
	Equipper();
	~Equipper();

	static bool CanEquipBothHands(Actor*, TESForm* item);
	static BGSEquipSlot* GetEquipSlotById(SInt32 slotId);
	static void EquipItem(PlayerCharacter* player, TESForm *item, SInt32 itemId, UInt32 slotId);
};

