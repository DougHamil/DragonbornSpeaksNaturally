#include "DSNMenuManager.h"
#include "SkyrimType.h"
#include "common/IPrefix.h"
#include "skse64_common/Relocation.h"
#include "Log.h"

uintptr_t MENU_MANAGER_ADDR[3] = {
	0x01EE5B20,	// SE
	0x01F7E100, // VR
	0x01F83200	// VR BETA
};

MenuManager* DSNMenuManager::GetSingleton() {
	RelocPtr<MenuManager *> ptr(MENU_MANAGER_ADDR[g_SkyrimType]);
	return *ptr;
}

/*
NOTE: Expose private fields on MenuManager and tHashSet to avoid requiring hooks into updated code
*/
IMenu * DSNMenuManager::GetOrCreateMenu(const char *menuName) {
	MenuManager *menuManager = DSNMenuManager::GetSingleton();

	if (!menuManager)
		return NULL;

	MenuManager::MenuTable *t = &menuManager->menuTable;

	for(int i = 0; i< t->m_size;i++){
		MenuManager::MenuTable::_Entry * entry = &t->m_entries[i];
		if (entry != t->m_eolPtr && entry->next != NULL) {
			MenuTableItem *item = &entry->item;

			if ((uintptr_t)item > 1 && item->name) {
				BSFixedString *name = &item->name;
				if ((uintptr_t)name > 1 && name->data && std::strncmp(name->data, menuName, 10) == 0) {
					if (!item->menuInstance && item->menuConstructor) {
						item->menuInstance = item->menuConstructor();
					}

					return item->menuInstance;
				}
			}
		}
	}

	return NULL;
}

