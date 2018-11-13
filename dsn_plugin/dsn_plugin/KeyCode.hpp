#pragma once
#include <Windows.h>
#include <unordered_map>
#include <string>
#include "common/ITypes.h"
#include "StringUtils.hpp"

// Convert key name to DirectInput scan code
// https://www.creationkit.com/index.php?title=Input_Script#DXScanCodes

static const std::unordered_map<std::string, UInt32> KEY_SCAN_CODE_MAP = {
    // keyboard
    { "escape", 1 }, { "esc", 1 },
    { "1", 2 },
    { "2", 3 },
    { "3", 4 },
    { "4", 5 },
    { "5", 6 },
    { "6", 7 },
    { "7", 8 },
    { "8", 9 },
    { "9", 10 },
    { "0", 11 },
    { "-", 12 },        { "minus", 13 },
    { "=", 13 },        { "equal", 13 }, { "equals", 13 },
    { "backspace", 14 },
    { "tab", 15 },      { "table", 15 },
    { "q", 16 },
    { "w", 17 },
    { "e", 18 },
    { "r", 19 },
    { "t", 20 },
    { "y", 21 },
    { "u", 22 },
    { "i", 23 },
    { "o", 24 },
    { "p", 25 },
    { "[", 26 }, { "leftbracket", 26 },  { "lbracket", 26 },
    { "]", 27 }, { "rightbracket", 27 }, { "rbracket", 27 },
    { "enter", 28 },
    { "leftcontrol", 29 }, { "leftctrl", 29 }, { "lctrl", 29 }, { "ctrl", 29 }, { "control", 29 },
    { "a", 30 },
    { "s", 31 },
    { "d", 32 },
    { "f", 33 },
    { "g", 34 },
    { "h", 35 },
    { "j", 36 },
    { "k", 37 },
    { "l", 38 },
    { ";", 39 }, { "semicolon", 39 },  { "semi", 39 },
    { "'", 40 }, { "apostrophe", 40 }, { "apos", 40 },
    { "`", 41 }, { "~", 41 },          { "backquote", 41 }, { "console", 41 },
    { "leftshift", 42 },               { "lshift", 42 },    { "shift", 42 },
    { "\\", 43 },                      { "backslash", 43 },
    { "z", 44 },
    { "x", 45 },
    { "c", 46 },
    { "v", 47 },
    { "b", 48 },
    { "n", 49 },
    { "m", 50 },
    { ",", 51 }, { "comma", 51 },
    { ".", 52 }, { "period", 52 },           { "point", 52 },
    { "/", 53 }, { "forwardslash", 53 },     { "slash", 53 },
    { "rightshift", 54 },                    { "rshift", 54 }, 
    { "num*", 55 },     { "n*", 55 },        { "numstar", 55 },
    { "leftalt", 56 },  { "leftalter", 56 }, { "lalt", 56 },   { "alt", 56 },
    { "spacebar", 57 }, { "space", 57 },     { "blank", 57 },
    { "capslock", 58 }, { "caps", 58 },
    { "f1", 59 },
    { "f2", 60 },
    { "f3", 61 },
    { "f4", 62 },
    { "f5", 63 },
    { "f6", 64 },
    { "f7", 65 },
    { "f8", 66 },
    { "f9", 67 },
    { "f10", 68 },
    { "numlock", 69 },    { "nlock", 69 },
    { "scrolllock", 70 }, { "slock", 70 },
    { "num7", 71 }, { "n7", 71 },
    { "num8", 72 }, { "n8", 72 },
    { "num9", 73 }, { "n9", 73 },
    { "num-", 74 }, { "n-", 74 }, { "numminus", 74 },
    { "num4", 75 }, { "n4", 75 },
    { "num5", 76 }, { "n5", 76 },
    { "num6", 77 }, { "n6", 77 },
    { "num+", 78 }, { "n+", 78 }, { "numplus", 78 },
    { "num1", 79 }, { "n1", 79 },
    { "num2", 80 }, { "n2", 80 },
    { "num3", 81 }, { "n3", 81 },
    { "num0", 82 }, { "n0", 82 },
    { "num.", 83 }, { "n.", 83 }, { "numperiod", 83 }, { "numpoint", 83 },
    { "f11", 87 },
    { "f12", 88 },
    { "numenter", 156 },                        { "nenter", 156 },
    { "rightcontrol", 157 },                    { "rightctrl", 157 }, { "rctrl", 157 },
    { "num/", 181 },     { "n/", 181 },         { "numslash", 181 },
    { "sysrq", 183 },    { "sys", 183 },        { "ptrscr", 183 }, { "printscreen", 183 },
    { "rightalt", 184 }, { "rightalter", 184 }, { "ralt", 184 },
    { "pause", 197 },    { "break", 197 },      { "pausebreak", 197 },
    { "home", 199 },
    { "uparrow", 200 },    { "up", 200 },
    { "pageup", 201 },     { "pgup", 201 },
    { "leftarrow", 203 },  { "left", 203 },
    { "rightarrow", 205 }, { "right", 205 },
    { "end", 207 },
    { "downarrow", 208 }, { "down", 208 },
    { "pagedown", 209 },  { "pgdown", 209 }, { "pgdn", 209 },
    { "insert", 210 },    { "ins", 210 },
    { "delete", 211 },    { "del", 211 },
    
    // mouse
    { "leftmousebutton", 256 },   { "leftclick", 256 },        { "lclick", 256 },
    { "rightmousebutton", 257 },  { "rightclick", 257 },       { "rclick", 257 },
    { "middlemousebutton", 258 }, { "wheelmousebutton", 258 }, { "middleclick", 258 }, { "mclick", 258 },
    { "mousebutton3", 259 },   { "button3", 259 },  { "mbtn3", 259 },
    { "mousebutton4", 260 },   { "button4", 260 },  { "mbtn4", 260 },
    { "mousebutton5", 261 },   { "button5", 261 },  { "mbtn5", 261 },
    { "mousebutton6", 262 },   { "button6", 262 },  { "mbtn6", 262 },
    { "mousebutton7", 263 },   { "button7", 263 },  { "mbtn7", 263 },
    { "mousewheelup", 264 },   { "wheelup", 264 },
    { "mousewheeldown", 265 }, { "wheeldown", 265 },
    
    // gamepad
    { "dpadup", 266 },        { "padup", 266 },
    { "dpaddown", 267 },      { "paddown", 267 },
    { "dpadleft", 268 },      { "padleft", 268 },
    { "dpadright", 269 },     { "padright", 269 },
    { "start", 270 },         { "padstart", 270 },
    { "back", 271 },          { "padback", 271 },
    { "leftthumb", 272 },     { "lthumb", 272 },
    { "rightthumb", 273 },    { "rthumb", 273 },
    { "leftshoulder", 274 },  { "lshoulder", 274 },
    { "rightshoulder", 275 }, { "rshoulder", 275 },
    { "dpada", 276 }, { "pada", 276 },
    { "dpadb", 277 }, { "padb", 277 },
    { "dpadx", 278 }, { "padx", 278 },
    { "dpady", 279 }, { "pady", 279 },
    { "lt", 280 },    { "lefttrigger", 280 },
    { "rt", 281 },    { "righttrigger", 281 }
};

static const UInt32 KEY_SCAN_CODE_MOUSE_EVENT_BEGIN    = 256;
static const UInt32 KEY_SCAN_CODE_MOUSE_EVENT_END      = 265;
static const UInt32 KEY_SCAN_CODE_MOUSE_X_BUTTON_BEGIN = 259;
static const UInt32 KEY_SCAN_CODE_MOUSE_WHEEL_UP       = 264;
static const UInt32 KEY_SCAN_CODE_MOUSE_WHEEL_DOWN     = 265;

static const std::unordered_map<UInt32, UInt32> KEY_CODE_TO_MOUSE_DOWN_MAP = {
    { 256, MOUSEEVENTF_LEFTDOWN },
    { 257, MOUSEEVENTF_RIGHTDOWN },
    { 258, MOUSEEVENTF_MIDDLEDOWN },
    { 259, MOUSEEVENTF_XDOWN },
    { 260, MOUSEEVENTF_XDOWN },
    { 261, MOUSEEVENTF_XDOWN },
    { 262, MOUSEEVENTF_XDOWN },
    { 263, MOUSEEVENTF_XDOWN },
    { 264, MOUSEEVENTF_WHEEL },
    { 265, MOUSEEVENTF_WHEEL }
};

static const std::unordered_map<UInt32, UInt32> KEY_CODE_TO_MOUSE_UP_MAP = {
    { 256, MOUSEEVENTF_LEFTUP },
    { 257, MOUSEEVENTF_RIGHTUP },
    { 258, MOUSEEVENTF_MIDDLEUP },
    { 259, MOUSEEVENTF_XUP },
    { 260, MOUSEEVENTF_XUP },
    { 261, MOUSEEVENTF_XUP },
    { 262, MOUSEEVENTF_XUP },
    { 263, MOUSEEVENTF_XUP },
    { 264, MOUSEEVENTF_WHEEL },
    { 265, MOUSEEVENTF_WHEEL }
};

static UInt32 GetKeyScanCode(std::string key) {
    stringToLower(key);

    auto itr = KEY_SCAN_CODE_MAP.find(key);
    if (itr != KEY_SCAN_CODE_MAP.end()) {
        // known key name
        return itr->second;
    }
    else if (key.size() > 2 && key[0] == '0' && (key[1] == 'x' || key[1] == 'X')) {
        // key code hex
        return strtol(key.substr(2).c_str(), NULL, 16);
    }
    else if ('0' <= key[0] && key[0] <= '9') {
        // key code dec
        return strtol(key.c_str(), NULL, 10);
    }

    // unknown key
    return 0;
}

// Set mouse event when press/release mouse button
static void _setMouseInput(INPUT &input) {
    if (input.ki.wScan < KEY_SCAN_CODE_MOUSE_EVENT_BEGIN || input.ki.wScan > KEY_SCAN_CODE_MOUSE_EVENT_END) {
		return;
    }

	bool isKeyUp = input.ki.dwFlags & KEYEVENTF_KEYUP;
	auto &mouseEventMap = isKeyUp ? KEY_CODE_TO_MOUSE_UP_MAP : KEY_CODE_TO_MOUSE_DOWN_MAP;

	auto itr = mouseEventMap.find(input.ki.wScan);
	if (itr == mouseEventMap.end()) {
		return;
	}

	input.type = INPUT_MOUSE;
	input.mi.dwFlags = itr->second;

	if (input.ki.wScan == KEY_SCAN_CODE_MOUSE_WHEEL_UP) {
		input.mi.mouseData = WHEEL_DELTA;
	}
	else if (input.ki.wScan == KEY_SCAN_CODE_MOUSE_WHEEL_DOWN) {
		input.mi.mouseData = -WHEEL_DELTA;
	}
	else if (input.mi.dwFlags == MOUSEEVENTF_XUP || input.mi.dwFlags == MOUSEEVENTF_XDOWN) {
		input.mi.mouseData = input.ki.wScan - KEY_SCAN_CODE_MOUSE_X_BUTTON_BEGIN;
	}
}

static void SendKeyDown(UInt32 keycode) {
    INPUT input;
    ZeroMemory(&input, sizeof(input));
    input.type = INPUT_KEYBOARD;
    input.ki.dwFlags = KEYEVENTF_SCANCODE;
    input.ki.wScan = keycode;

	_setMouseInput(input);

    SendInput(1, &input, sizeof(INPUT));
    //Log::info("key down: " + std::to_string(*itr));
}

static void SendKeyUp(UInt32 keycode) {
    INPUT input;
    ZeroMemory(&input, sizeof(input));
    input.type = INPUT_KEYBOARD;
    input.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
    input.ki.wScan = keycode;

	_setMouseInput(input);

    SendInput(1, &input, sizeof(INPUT));
    //Log::info("key up: " + std::to_string(itr->second));
}
