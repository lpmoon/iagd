#include "stdafx.h"
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include "MessageType.h"
#include "GameEngineUpdate.h"
#include "Exports.h"


GameEngineUpdate* GameEngineUpdate::g_self;
void GameEngineUpdate::EnableHook() {
	originalMethod = (OriginalMethodPtr)HookGame(
		"?Update@GameEngine@GAME@@QEAAXH@Z",
		HookedMethod,
		m_dataQueue,
		m_hEvent,
		TYPE_GAMEENGINE_UPDATE
	);
}

GameEngineUpdate::GameEngineUpdate(DataQueue* dataQueue, HANDLE hEvent, HookLog* g_log) {
	g_self = this;
	this->m_dataQueue = dataQueue;
	this->m_hEvent = hEvent;
}

GameEngineUpdate::GameEngineUpdate() {
	GameEngineUpdate::m_hEvent = nullptr;
}

void GameEngineUpdate::DisableHook() {
	Unhook((PVOID*)&originalMethod, HookedMethod);
}


typedef GAME::Item* (__fastcall* pCreateItem)(GAME::ItemReplicaInfo* info);
auto fnCreateItemQ = pCreateItem(GetProcAddress(GetModuleHandle(TEXT("game.dll")), "?CreateItem@Item@GAME@@SAPEAV12@AEBUItemReplicaInfo@2@@Z"));

typedef GAME::ObjectManager* (__fastcall* pGetObjectManager)();
auto fnGetObjectManagerQ = pGetObjectManager(GetProcAddress(GetModuleHandle(TEXT("engine.dll")), "?Get@?$Singleton@VObjectManager@GAME@@@GAME@@SAPEAVObjectManager@2@XZ"));

typedef void(__fastcall* pDestroyObjectEx)(GAME::ObjectManager*, GAME::Object* object, const char* file, int line);
auto fnDestroyObjectExQ = pDestroyObjectEx(GetProcAddress(GetModuleHandle(TEXT("engine.dll")), "?DestroyObjectEx@ObjectManager@GAME@@QEAAXPEAVObject@2@PEBDH@Z"));

typedef void(__fastcall* pDissolveItem)(GAME::Item*, const bool bDestroy);
auto fnDissolveItemQ = pDissolveItem(GetProcAddress(GetModuleHandle(TEXT("game.dll")), "?Dissolve@Item@GAME@@QEAAX_N@Z"));

typedef GAME::Player* (__fastcall* pGetMainPlayer)(GAME::GameEngine*);
auto fnGetMainPlayerQ = pGetMainPlayer(GetProcAddress(GetModuleHandle(TEXT("game.dll")), "?GetMainPlayer@GameEngine@GAME@@QEBAPEAVPlayer@2@XZ"));

typedef void(__fastcall* pItemEquipmentGetUIDisplayText)(GAME::ItemEquipment*, GAME::Character* myCharacter, std::vector<GAME::GameTextLine>* text);
auto fnItemEquipmentGetUIDisplayTextQ = pItemEquipmentGetUIDisplayText(GetProcAddress(GetModuleHandle(TEXT("game.dll")), "?GetUIDisplayText@ItemEquipment@GAME@@UEBAXPEBVCharacter@2@AEAV?$vector@UGameTextLine@GAME@@@mem@@@Z"));

GAME::GameEngine* fnGetgGameEngine();


void* __fastcall GameEngineUpdate::HookedMethod(void* This, int v) {
	void* r = g_self->originalMethod(This, v);

	for (int i = 0; i < 15; i++) {
		// example: Vampiric Dermapteran Slicer of Piercing Darkness
		GAME::ItemReplicaInfo replica;
		replica.baseRecord = "records/items/gearshoulders/d126_shoulder.dbr"; // Dermapteran Slicer
		replica.seed = 0x4d807ecb;
		replica.relicSeed = 0x41fab3a4;
		replica.stackSize = 1;

		// create the item
		GAME::Item* newItem = fnCreateItemQ(&replica);
		if (newItem)
		{
			// this vector gets filled with the item stats 
			std::vector<GAME::GameTextLine> textLine = {};


			std::wofstream itemStatsfileA;
			itemStatsfileA.open("ItemStats.txt", std::ofstream::out | std::ofstream::app);
			itemStatsfileA << "Managed to MAKE the item..";
			itemStatsfileA.flush();
			itemStatsfileA.close();

			// get stats
			fnItemEquipmentGetUIDisplayTextQ((GAME::ItemEquipment*)newItem, (GAME::Character*)fnGetMainPlayerQ(fnGetgGameEngine()), &textLine);


			std::wofstream itemStatsfileB;
			itemStatsfileB.open("ItemStats.txt", std::ofstream::out | std::ofstream::app);
			itemStatsfileB << "Managed to GET STATS for the item..";
			itemStatsfileB.flush();
			itemStatsfileB.close();

			// delete item
			bool bDestroy = true; // important to delete the item from the game database
			fnDissolveItemQ(newItem, bDestroy);

			// dump the stats to a file
			std::wofstream itemStatsfile;
			itemStatsfile.open("ItemStats.txt", std::ofstream::out | std::ofstream::app);

			// iterate through all text lines
			for (auto& it : textLine)
				itemStatsfile << "TextClass: " << it.textClass << " Text: " << it.text.c_str() << "\n";

			// close file again
			itemStatsfile.flush();
			itemStatsfile.close();

		}
	}

	g_self->TransferData(0, nullptr);
	return r;
}