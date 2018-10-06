#pragma once
#include <Windows.h>
#include <tlhelp32.h>

// Get the main window handle based on the process ID
// FindMainWindow(), _enumWindowsCallback(), _isMainWindow()
// copied from <https://stackoverflow.com/questions/1888863/how-to-get-main-window-handle-from-process-id/21767578#21767578>

struct handle_data {
	unsigned long process_id;
	HWND window_handle;
};

static BOOL _isMainWindow(HWND handle)
{
	return GetWindow(handle, GW_OWNER) == (HWND)0 && IsWindowVisible(handle);
}

static BOOL CALLBACK _enumWindowsCallback(HWND handle, LPARAM lParam)
{
	handle_data& data = *(handle_data*)lParam;
	unsigned long process_id = 0;
	GetWindowThreadProcessId(handle, &process_id);
	if (data.process_id != process_id || !_isMainWindow(handle))
		return TRUE;
	data.window_handle = handle;
	return FALSE;
}

static HWND FindMainWindow(unsigned long process_id)
{
	handle_data data;
	data.process_id = process_id;
	data.window_handle = 0;
	EnumWindows(_enumWindowsCallback, (LPARAM)&data);
	return data.window_handle;
}

// Get the process id by the executable name
// Copied from <https ://blog.csdn.net/hubinbin595959/article/details/77839945?utm_source=copy>
static DWORD GetProcessIDByName(const char* pName)
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (INVALID_HANDLE_VALUE == hSnapshot) {
		return NULL;
	}
	PROCESSENTRY32 pe = { sizeof(pe) };
	for (BOOL ret = Process32First(hSnapshot, &pe); ret; ret = Process32Next(hSnapshot, &pe)) {
		if (_stricmp(pe.szExeFile, pName) == 0) {
			CloseHandle(hSnapshot);
			return pe.th32ProcessID;
		}
	}
	CloseHandle(hSnapshot);
	return 0;
}
