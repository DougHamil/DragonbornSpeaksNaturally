#include "Equipper.h"
#include "Log.h"

bool Equipper::CanEquipBothHands(Actor* actor, TESForm * item)
{
	BGSEquipType * equipType = DYNAMIC_CAST(item, TESForm, BGSEquipType);
	if (!equipType)
		return false;

	BGSEquipSlot * equipSlot = equipType->GetEquipSlot();
	if (!equipSlot)
		return false;

	// 2H
	if (equipSlot == GetEitherHandSlot())
	{
		return true;
	}
	// 1H
	else if (equipSlot == GetLeftHandSlot() || equipSlot == GetRightHandSlot())
	{
		return (actor->race->data.raceFlags & TESRace::kRace_CanDualWield) && item->IsWeapon();
	}

	return false;
}

BGSEquipSlot * Equipper::GetEquipSlotById(SInt32 slotId)
{
	enum
	{
		kSlotId_Default = 0,
		kSlotId_Right = 1,
		kSlotId_Left = 2
	};

	if (slotId == kSlotId_Right)
		return GetRightHandSlot();
	else if (slotId == kSlotId_Left)
		return GetLeftHandSlot();
	else
		return NULL;
}

void EquipItemById(Actor* thisActor, TESForm* item, SInt32 itemId, SInt32 slotId, bool preventUnequip /*unused*/, bool equipSound)
{
	if (!item || !item->Has3D() || itemId == 0)
		return;

	// Can't be improved or enchanted, no need for itemId
	if (item->IsAmmo())
	{
		papyrusActor::EquipItemEx(thisActor, item, slotId, preventUnequip, equipSound);
		return;
	}

	EquipManager* equipManager = EquipManager::GetSingleton();
	if (!equipManager)
		return;

	ExtraContainerChanges* containerChanges = static_cast<ExtraContainerChanges*>(thisActor->extraData.GetByType(kExtraData_ContainerChanges));
	ExtraContainerChanges::Data* containerData = containerChanges ? containerChanges->data : NULL;
	if (!containerData)
		return;

	InventoryEntryData::EquipData itemData;
	containerData->GetEquipItemData(itemData, item, itemId);

	BGSEquipSlot * targetEquipSlot = Equipper::GetEquipSlotById(slotId);
	bool isTargetSlotInUse = false;

	SInt32 itemCount = itemData.itemCount;

	// Need at least 1 (maybe 2 for dual wield, checked later)
	bool hasItemMinCount = itemCount > 0;
	bool canDualWield = false;

	BaseExtraList * newEquipList = itemData.itemExtraList;

	if (hasItemMinCount)
	{
		// Case 1: Type already equipped in both hands.
		if (itemData.isTypeWorn && itemData.isTypeWornLeft)
		{
			isTargetSlotInUse = true;
		}
		// Case 2: Type already equipped in right hand.
		else if (itemData.isTypeWorn)
		{
			isTargetSlotInUse = targetEquipSlot == GetRightHandSlot() || targetEquipSlot == NULL;
		}
		// Case 3: Type already equipped in left hand.
		else if (itemData.isTypeWornLeft)
		{
			isTargetSlotInUse = targetEquipSlot == GetLeftHandSlot();
		}
		// Case 4: Type not equipped yet.
		else
		{
			isTargetSlotInUse = false;
		}
	}

	// This also returns if the target slot is in use by another weapon of the same type.
	// Could handle this, but switching them here causes a bug (0 damage) for some reason.
	// So we just skip it. Can be handled on the Papyrus side.
	if (isTargetSlotInUse || !hasItemMinCount)
		return;

	bool isItemEquipped = itemData.isItemWorn || itemData.isItemWornLeft;

	// Does this item qualify for dual wield?
	if (item->IsWeapon() && targetEquipSlot && isItemEquipped && Equipper::CanEquipBothHands(thisActor, item))
		canDualWield = true;

	// Not enough items to dual wield, weapon has to swap hands
	if (canDualWield && itemCount < 2)
	{
		BaseExtraList* unequipList = itemData.isItemWornLeft ? itemData.wornLeftExtraList : itemData.wornExtraList;

		// Unequip might destroy passed list (return value indicates that).
		newEquipList = CALL_MEMBER_FN(equipManager, UnequipItem)(thisActor, item, unequipList, 1, 0, false, false, true, false, NULL) ? NULL : unequipList;
	}

	CALL_MEMBER_FN(equipManager, EquipItem)(thisActor, item, newEquipList, 1, targetEquipSlot, equipSound, preventUnequip, false, NULL);
}

void Equipper::EquipItem(PlayerCharacter *player, TESForm *item, SInt32 itemId, UInt32 slotId) {

	TESObjectARMO *armor = DYNAMIC_CAST(item, TESForm, TESObjectARMO);

	// If it's armor, unequip item in target slot first
	if (armor) {
		EquipManager* equipManager = EquipManager::GetSingleton();
		if (!equipManager)
			return;
		UInt32 slotMask = armor->bipedObject.GetSlotMask();
		TESForm *wornItem = papyrusActor::GetWornForm(player, slotMask);
		if (wornItem) {
			papyrusActor::UnequipItemEx(player, wornItem, 0, false);
		}

		ExtraContainerChanges* containerChanges = static_cast<ExtraContainerChanges*>(player->extraData.GetByType(kExtraData_ContainerChanges));
		ExtraContainerChanges::Data* containerData = containerChanges ? containerChanges->data : NULL;
		if (!containerData)
			return;

		BGSEquipSlot *equipSlot = armor->equipType.equipSlot;
		InventoryEntryData::EquipData itemData;
		containerData->GetEquipItemData(itemData, item, itemId);
		CALL_MEMBER_FN(equipManager, EquipItem)(player, item, itemData.itemExtraList, 1, equipSlot, true, false, false, NULL);
	}
	else {
		papyrusActor::EquipItemById(player, item, itemId, slotId, false, true);
	}

}
