#include "Hooks.h"
#include <string.h>
#include "Log.h"
#include "common/IPrefix.h"
#include "skse64_common/SafeWrite.h"
#include "skse64/ScaleformMovie.h"
#include "skse64/ScaleformValue.h"
#include "skse64_common/BranchTrampoline.h"
#include "xbyak.h"
#include "SkyrimType.h"
#include "ConsoleCommandRunner.h"
#include "FavoritesMenuManager.h"

static GFxMovieView* dialogueMenu = NULL;
static int desiredTopicIndex = 1;
static int numTopics = 0;
static int lastMenuState = -1;
typedef UInt32 getDefaultCompiler(void* unk01, char* compilerName, UInt32 unk03);
typedef void executeCommand(UInt32* unk01, void* parser, char* command);

static void __cdecl Hook_Invoke(GFxMovieView* movie, char * gfxMethod, GFxValue* argv, UInt32 argc)
{
	if (argc >= 1)
	{
		GFxValue commandVal = argv[0];
		if (commandVal.type == 4) { // Command
			const char* command = commandVal.data.string;
			//Log::info(command); // TEMP
			if (strcmp(command, "PopulateDialogueList") == 0)
			{
				numTopics = (argc - 2) / 3;
				desiredTopicIndex = -1;
				dialogueMenu = movie;
				std::vector<std::string> lines;
				for (int j = 1; j < argc - 1; j = j + 3)
				{
					GFxValue dialogueLine = argv[j];
					const char* dialogueLineStr = dialogueLine.data.string;
					lines.push_back(std::string(dialogueLineStr));
				}

				DialogueList dialogueList;
				dialogueList.lines = lines;
				SpeechRecognitionClient::getInstance()->StartDialogue(dialogueList);
			}
			else if (g_SkyrimType == VR && strcmp(command, "UpdatePlayerInfo") == 0)
			{
				FavoritesMenuManager::getInstance()->RefreshFavorites();
			}
		}
	}
}

static void __cdecl Hook_PostLoad() {
	if (g_SkyrimType == VR) {
		FavoritesMenuManager::getInstance()->RefreshFavorites();
	}
}

static void __cdecl Hook_Loop()
{
	if (dialogueMenu != NULL)
	{
		// Menu exiting, avoid NPE
		if (dialogueMenu->GetPause() == 0)
		{
			dialogueMenu = NULL;
			SpeechRecognitionClient::getInstance()->StopDialogue();
			return;
		}
		GFxValue stateVal;
		dialogueMenu->GetVariable(&stateVal, "_level0.DialogueMenu_mc.eMenuState");
		int menuState = stateVal.data.number;
		desiredTopicIndex = SpeechRecognitionClient::getInstance()->ReadSelectedIndex();
		if (menuState != lastMenuState) {
		
			lastMenuState = menuState;
			if (menuState == 2) // NPC Responding
			{
				SpeechRecognitionClient::getInstance()->StopDialogue();
			}
		}
		if (desiredTopicIndex >= 0) {
			GFxValue topicIndexVal;
			dialogueMenu->GetVariable(&topicIndexVal, "_level0.DialogueMenu_mc.TopicList.iSelectedIndex");

			int currentTopicIndex = topicIndexVal.data.number;
			if (currentTopicIndex != desiredTopicIndex) {

				dialogueMenu->Invoke("_level0.DialogueMenu_mc.TopicList.SetSelectedTopic", NULL, "%d", desiredTopicIndex);
				dialogueMenu->Invoke("_level0.DialogueMenu_mc.TopicList.doSetSelectedIndex", NULL, "%d", desiredTopicIndex);
				dialogueMenu->Invoke("_level0.DialogueMenu_mc.TopicList.UpdateList", NULL, NULL, 0);
			}

			dialogueMenu->Invoke("_level0.DialogueMenu_mc.onSelectionClick", NULL, "%d", 1.0);
		}
		else if (desiredTopicIndex == -2) { // Indicates a "goodbye" phrase was spoken, hide the menu
			dialogueMenu->Invoke("_level0.DialogueMenu_mc.StartHideMenu", NULL, NULL, 0);
		}
	}
	else
	{
		std::string command = SpeechRecognitionClient::getInstance()->PopCommand();
		if (command != "") {
			ConsoleCommandRunner::RunCommand(command);
		}

		if (g_SkyrimType == VR)
			FavoritesMenuManager::getInstance()->ProcessEquipCommands();
	}
}

static uintptr_t loopEnter = 0x0;
static uintptr_t loopCallTarget = 0x0;
static uintptr_t invokeTarget = 0x0;
static uintptr_t invokeReturn = 0x0;
static uintptr_t loadEventEnter = 0x0;
static uintptr_t loadEventTarget = 0x0;

static uintptr_t INVOKE_ENTER_ADDR[3];
static uintptr_t INVOKE_TARGET_ADDR[3];

static uintptr_t LOOP_ENTER_ADDR[3];
static uintptr_t LOOP_TARGET_ADDR[3];

static uintptr_t LOAD_EVENT_ENTER_ADDR[3];
static uintptr_t LOAD_EVENT_TARGET_ADDR[3];


void Hooks_Inject(void)
{
	// "call" Scaleform invocation
	INVOKE_ENTER_ADDR[SE] = 0xED6C8E;
	INVOKE_ENTER_ADDR[VR] = 0xF30E8E;
	INVOKE_ENTER_ADDR[VR_BETA] = 0xF3413E;

	// Target of "call" invocation
	INVOKE_TARGET_ADDR[SE] = 0xED8190; // SkyrimSE 0xED8190 0x00007FF724928190 
	INVOKE_TARGET_ADDR[VR] = 0xF2D9B0; // SkyrimVR 0xF2D9B0 0x00007FF73284D9B0
	INVOKE_TARGET_ADDR[VR_BETA] = 0xF30C90; // SkyrimVR 0xF2D9B0 0x00007FF73284D9B0

	// "CurrentTime" GFxMovie.SetVariable (rax+80)
	LOOP_ENTER_ADDR[SE] = 0x87F1AC; // SkyrimSE 0x87F1AC 0x00007FF7242CF1AC
	LOOP_ENTER_ADDR[VR] = 0x8AA36C; // 0x8AA36C 0x00007FF7321CA36C SKSE UIManager process hook:  0x00F17200 + 0xAD8
	LOOP_ENTER_ADDR[VR_BETA] = 0x8AC02C; // 0x8AA36C 0x00007FF7321CA36C SKSE UIManager process hook:  0x00F17200 + 0xAD8
	
	// "CurrentTime" GFxMovie.SetVariable Target (rax+80)
	LOOP_TARGET_ADDR[SE] = 0xF29030; // SkyrimSE 0xF29030 0x00007FF724979030
	LOOP_TARGET_ADDR[VR] = 0xF82710; // SkyrimVR 0xF82710 0x00007FF7328A2710 SKSE UIManager process hook:  0x00F1C650
	LOOP_TARGET_ADDR[VR_BETA] = 0xF859C0; // SkyrimVR 0xF82710 0x00007FF7328A2710 SKSE UIManager process hook:  0x00F1C650

	// "Finished loading game" print statement, initialize player orientation?
	LOAD_EVENT_ENTER_ADDR[VR] = 0x585094;

	// Initialize player orientation target addr
	LOAD_EVENT_TARGET_ADDR[VR] = 0x6AB2D0;

	RelocAddr<uintptr_t> kHook_Invoke_Enter(INVOKE_ENTER_ADDR[g_SkyrimType]);
	RelocAddr<uintptr_t> kHook_Invoke_Target(INVOKE_TARGET_ADDR[g_SkyrimType]);
	RelocAddr<uintptr_t> kHook_Loop_Enter(LOOP_ENTER_ADDR[g_SkyrimType]);
	RelocAddr<uintptr_t> kHook_Loop_Call_Target(LOOP_TARGET_ADDR[g_SkyrimType]);
	uintptr_t kHook_Invoke_Return = kHook_Invoke_Enter + 0x14;

	invokeTarget = kHook_Invoke_Target;
	invokeReturn = kHook_Invoke_Return;
	loopCallTarget = kHook_Loop_Call_Target;
	loopEnter = kHook_Loop_Enter;

	Log::address("Loop Enter: ", kHook_Loop_Enter);
	Log::address("Loop Target: ", kHook_Loop_Call_Target);

	/***
	Post Load HOOK - VR Only
	**/
	if (g_SkyrimType == VR) {
		RelocAddr<uintptr_t> kHook_LoadEvent_Enter(LOAD_EVENT_ENTER_ADDR[g_SkyrimType]);
		RelocAddr<uintptr_t> kHook_LoadEvent_Target(LOAD_EVENT_TARGET_ADDR[g_SkyrimType]);
		loadEventEnter = kHook_LoadEvent_Enter;
		loadEventTarget = kHook_LoadEvent_Target;

		Log::address("LoadEvent Enter: ", kHook_LoadEvent_Enter);
		Log::address("LoadEvent Target: ", kHook_LoadEvent_Target);

		struct Hook_LoadEvent_Code : Xbyak::CodeGenerator {
			Hook_LoadEvent_Code(void * buf) : Xbyak::CodeGenerator(4096, buf)
			{
				// Invoke original virtual method
				mov(rax, (uintptr_t)loadEventTarget);
				call(rax);

				// Call our method
				sub(rsp, 0x30);
				mov(rax, (uintptr_t)Hook_PostLoad);
				call(rax);
				add(rsp, 0x30);

				// Return 
				mov(rax, loadEventEnter + 0x5);
				jmp(rax);
			}
		};
		void * codeBuf = g_localTrampoline.StartAlloc();
		Hook_LoadEvent_Code loadEventCode(codeBuf);
		g_localTrampoline.EndAlloc(loadEventCode.getCurr());
		g_branchTrampoline.Write5Branch(kHook_LoadEvent_Enter, uintptr_t(loadEventCode.getCode()));
	}

	/***
	Loop HOOK
	**/
	struct Hook_Loop_Code : Xbyak::CodeGenerator {
		Hook_Loop_Code(void * buf) : Xbyak::CodeGenerator(4096, buf)
		{
			// Invoke original virtual method
			mov(rax, loopCallTarget);
			call(rax);

			// Call our method
			sub(rsp, 0x30);
			mov(rax, (uintptr_t)Hook_Loop);
			call(rax);
			add(rsp, 0x30);

			// Return 
			mov(rax, loopEnter + 0x6); // set to 0x5 when branching for SKSE UIManager
			jmp(rax);
		}
	};
	void * codeBuf = g_localTrampoline.StartAlloc();
	Hook_Loop_Code loopCode(codeBuf);
	g_localTrampoline.EndAlloc(loopCode.getCurr());
	//g_branchTrampoline.Write6Branch(kHook_Loop_Enter, uintptr_t(loopCode.getCode()));
	g_branchTrampoline.Write5Branch(kHook_Loop_Enter, uintptr_t(loopCode.getCode()));

	/***
	Invoke "call" HOOK
	**/
	struct Hook_Entry_Code : Xbyak::CodeGenerator {
		Hook_Entry_Code(void * buf) : Xbyak::CodeGenerator(4096, buf)
		{
			push(rcx);
			push(rdx);
			push(r8);
			push(r9);
			sub(rsp, 0x30);
			mov(rax, (uintptr_t)Hook_Invoke);
			call(rax);
			add(rsp, 0x30);
			pop(r9);
			pop(r8);
			pop(rdx);
			pop(rcx);
	
			mov(rax, invokeTarget);
			call(rax);

			mov(rbx, ptr[rsp + 0x50]);
			mov(rsi, ptr[rsp + 0x60]);
			add(rsp, 0x40);
			pop(rdi);

			mov(rax, invokeReturn);
			jmp(rax);
		}
	};

	codeBuf = g_localTrampoline.StartAlloc();
	Hook_Entry_Code entryCode(codeBuf);
	g_localTrampoline.EndAlloc(entryCode.getCurr());
	g_branchTrampoline.Write5Branch(kHook_Invoke_Enter, uintptr_t(entryCode.getCode()));
}
