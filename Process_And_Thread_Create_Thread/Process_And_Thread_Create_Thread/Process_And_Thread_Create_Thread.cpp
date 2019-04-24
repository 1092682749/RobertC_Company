// Process_And_Thread_Create_Thread.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <Windows.h>
#include <iostream>
#include <strsafe.h>

#define MAX_THREADS 3
#define BUF_SIZE 255
DWORD WINAPI MyThreadFunction(LPVOID lpParam);
void ErrorHandler(LPCTSTR lpszFunction);
typedef struct MyData {
	int val1;
	int val2;
} MYDATA, *PMYDATA;
int main()
{
	PMYDATA pDataArray[MAX_THREADS];
	DWORD dwThreadIdArray[MAX_THREADS];
	HANDLE hThreadArray[MAX_THREADS];

	for (int i = 0; i < MAX_THREADS; i++)
	{
		pDataArray[i] = (PMYDATA)HeapAlloc(GetProcessHeap(),
			HEAP_ZERO_MEMORY,
			sizeof(MYDATA));
		if (pDataArray[i] == NULL)
		{
			ExitProcess(2);
		}
		pDataArray[i]->val1 = i;
		pDataArray[i]->val2 = i + 100;
		hThreadArray[i] = CreateThread(
			NULL,
			0,
			MyThreadFunction,
			pDataArray[i],
			0,
			&dwThreadIdArray[i]
		);
		if (hThreadArray[i] == NULL)
		{
			ErrorHandler(TEXT("CreateThread"));
		}
	}
	WaitForMultipleObjects(MAX_THREADS, hThreadArray, TRUE, INFINITE);

	for (int i = 0; i < MAX_THREADS; i++)
	{
		CloseHandle(hThreadArray[i]);
		if (pDataArray[i] != NULL) 
		{
			HeapFree(GetProcessHeap(), 0, pDataArray[i]);
			pDataArray[i] = NULL;
		}
	}
	return 0;
}

DWORD WINAPI MyThreadFunction(LPVOID lpParam)
{
	HANDLE hStdout;
	PMYDATA pDataArray;

	TCHAR msgBuf[BUF_SIZE];
	size_t cchStringSize;
	DWORD dwChars;

	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hStdout == INVALID_HANDLE_VALUE)
	{
		return 1;
	}
	pDataArray = (PMYDATA)lpParam;
	// 它接受一个格式字符串和参数列表和返回一个格式化字符串
	StringCchPrintf(msgBuf, BUF_SIZE, TEXT("Parameters = %d, %d\n"),
		pDataArray->val1, pDataArray->val2);
	// 判断长度是否超出了限制
	StringCchLength(msgBuf, BUF_SIZE, &cchStringSize);
	WriteConsole(hStdout, msgBuf, cchStringSize, &dwChars, NULL);
	return 0;
}

void ErrorHandler(LPCTSTR lpszFunction)
{
	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0,
		NULL);
	lpDisplayBuf = (LPVOID)LocalAlloc(
		LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"),
		lpszFunction, dw, lpMsgBuf);
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
}
