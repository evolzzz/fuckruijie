#include "stdafx.h"

DWORD FindProcessId(LPCTSTR lpszName)
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (INVALID_HANDLE_VALUE == hSnapshot)
		return 0;

	PROCESSENTRY32 pe = { 0 };

	pe.dwSize = sizeof(PROCESSENTRY32);
	if (!Process32First(hSnapshot, &pe))
	{
		CloseHandle(hSnapshot);
		return 0;
	}
	if (_tcsstr(pe.szExeFile, lpszName) != NULL)
	{
		CloseHandle(hSnapshot);
		return pe.th32ProcessID;
	}

	pe.dwSize = sizeof(PROCESSENTRY32);
	while (Process32Next(hSnapshot, &pe))
	{
		if (_tcsstr(pe.szExeFile, lpszName) != NULL)
		{
			CloseHandle(hSnapshot);
			return pe.th32ProcessID;
		}
		pe.dwSize = sizeof(PROCESSENTRY32);
	}

	CloseHandle(hSnapshot);
	return 0;
}
#pragma comment(lib, "Msimg32.lib")

int __cdecl MsgBox(LPCTSTR lpszText, ...)
{
	TCHAR buf[1024] = { 0 };
	va_list args;

	va_start(args, lpszText);
	_vstprintf(buf, lpszText, args);
	va_end(args);

	return MessageBox(NULL, buf, _T("FuckRuijie"), MB_OK | MB_ICONINFORMATION);
}

int __cdecl MsgBoxErr(LPCTSTR lpszText, ...)
{
	TCHAR buf[1024] = { 0 };
	va_list args;

	va_start(args, lpszText);
	_vstprintf(buf, lpszText, args);
	va_end(args);

	return MessageBox(NULL, buf, _T("FuckRuijie"), MB_OK | MB_ICONERROR);
}

bool CheckSingleProcess()
{
	HANDLE hMutex = CreateMutex(NULL, FALSE, L"fuckruijie");
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		::CloseHandle(hMutex);
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}
BOOL WINAPI InjectProxyW(DWORD dwPID, PCWSTR pwszProxyFile)
{
	BOOL ret = FALSE;
	HANDLE hToken = NULL;
	HANDLE hProcess = NULL;
	HANDLE hThread = NULL;
	FARPROC pfnThreadRtn = NULL;
	PWSTR pwszPara = NULL;
	ULONG lResult = NULL;
	//

	hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID);
	if (!hProcess)
	{
		MsgBoxErr(_T("OpenProcess error"));
		ExitProcess(0);
	}
	pfnThreadRtn = GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "LoadLibraryW");
	if (!pfnThreadRtn)
	{
		MsgBoxErr(_T("GetProcAddress error"));
		ExitProcess(0);
	}
	size_t iProxyFileLen = wcslen(pwszProxyFile)*sizeof(WCHAR);
	pwszPara = (PWSTR)VirtualAllocEx(hProcess, NULL, iProxyFileLen, MEM_COMMIT,
		PAGE_READWRITE);
	if (!WriteProcessMemory(hProcess, pwszPara, (PVOID)pwszProxyFile, iProxyFileLen, NULL))
	{
		MsgBoxErr(_T("WriteProcessMemory error"));
		ExitProcess(0);
	}

	hThread = CreateRemoteThread(hProcess, NULL, 1024, (LPTHREAD_START_ROUTINE)pfnThreadRtn,
		pwszPara, 0, NULL);
	lResult = (ULONG)hThread;
	if (!lResult)
	{
		MsgBoxErr(_T("CreateRemoteThread error"));
		//ExitProcess(0);
	}
	WaitForSingleObject(hThread, INFINITE);
	

	CloseHandle(hThread);
	VirtualFreeEx(hProcess, pwszPara, 0, MEM_RELEASE);
	CloseHandle(hProcess);
	return lResult;
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow)
{
	DWORD Pid=0;
	UINT nRet=0;
	DWORD nTick=0;
	HANDLE hProcess = NULL;
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	if (!CheckSingleProcess())
	{
		MsgBoxErr(_T("fuckruijie.exe is working now, please wait or terminate it by taskmanager"));
		ExitProcess(0);
	}
	if (Pid = FindProcessId(_T("RuijieSupplicant.exe")))
	{
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, Pid);
		TerminateProcess(hProcess, 0);
	}
	if (Pid = FindProcessId(_T("8021x.exe")))
	{
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, Pid);
		TerminateProcess(hProcess, 0);
		Sleep(500);
	}
	Pid = 0;
	
	nRet = WinExec("RuijieSupplicant.exe", SW_SHOW);
	if (nRet <= 31)
	{
		MsgBoxErr(_T("Execute RuijieSupplicant.exe failed!"));
		return 1;
	}
	nTick = GetTickCount();
	while (TRUE)
	{
		if (Pid != 0)
			break;
		Pid = FindProcessId(_T("8021x.exe"));
		
		Sleep(750);
		if (GetTickCount() - nTick > 700)
			break;
	}

	if (Pid == 0)
	{
		MsgBoxErr(_T("8021x.exe was not found!"));
		return 1;
	}

	if (!InjectProxyW(Pid, _T("shit.dll")))
	{
		MsgBoxErr(_T("Can not fuck Ruijie, please check shit.dll!"));
		return 1;
	}

	return 0;
}
