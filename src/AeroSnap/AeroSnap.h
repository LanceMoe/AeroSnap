#pragma once


#define WINVER 0x0601
#define _WIN32_WINNT 0x0601

#define  _CRT_SECURE_NO_WARNINGS

#define WIN32_LEAN_AND_MEAN             // �� Windows ͷ���ų�����ʹ�õ�����
// Windows ͷ�ļ�:
#include <windows.h>
#include <shellapi.h>
#include <commdlg.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <Shlobj.h>

// C ����ʱͷ�ļ�
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>


// ���ÿ��ļ�
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "msimg32.lib")
#pragma comment(lib, "Shlwapi.lib")


// ����
/*
void DbgPrint(const wchar_t *format, ...)
{
	va_list argptr;
	TCHAR buffer[1024];
	va_start(argptr, format);
	_vsnwprintf(buffer, 1024, format, argptr);
	va_end(argptr);

	OutputDebugString(buffer);
}
*/
TCHAR szTitle[] = _T("AeroSnap");

HINSTANCE hInst;
NOTIFYICONDATA nid;
