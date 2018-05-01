#pragma once
#include "common/IPrefix.h"

#include <vector>
#include <queue>
#include <string>
#include <windows.h> 
#include <sstream>
#include <mutex>

struct DialogueList
{
	std::vector<std::string> lines;
};

class SpeechRecognitionClient
{
public:
	static SpeechRecognitionClient* getInstance();

	static void Initialize();
	~SpeechRecognitionClient();
	static SpeechRecognitionClient* instance;
	void SetHandles(HANDLE h_stdInWr, HANDLE h_stdOutRd) {
		stdInWr = h_stdInWr;
		stdOutRd = h_stdOutRd;
	}
	void StopDialogue();
	void StartDialogue(DialogueList list);
	void WriteLine(std::string str);
	int ReadSelectedIndex();
	std::string PopCommand();
	std::string PopEquip();
	void AwaitResponses();
	void EnqueueCommand(std::string command);
private:
	HANDLE stdInWr;
	HANDLE stdOutRd;
	int selectedIndex = -1;
	int currentDialogueId = 0;
	std::mutex queueLock;
	std::string workingLine;
	void EnqueueEquip(std::string equip);
	std::queue<std::string> queuedCommands;
	std::queue<std::string> queuedEquips;

	SpeechRecognitionClient();

	std::string ReadLine();
};

