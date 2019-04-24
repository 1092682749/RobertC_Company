// Append_File_To_Other_File.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <Windows.h>
#include <tchar.h>

int main(int argc, TCHAR *argv[])
{
	std::cout << "中文\n";
	//if (argc < 3)
	//{
	//	_tprintf(L"Need Source File And Destination File\n");
	//	return -1;
	//} 
	//_tprintf(L"%s \n %s\n", (LPCWSTR)argv[1], (LPCWSTR)argv[2]);
	HANDLE hSourceFile = CreateFile(L"D:/a/from.txt", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hSourceFile == INVALID_HANDLE_VALUE)
	{
		_tprintf(L"Source Handle INVALID %d\n", GetLastError());
		return 0;
	}
	HANDLE hDestinationFile = CreateFile(L"D:/a/to.txt", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hDestinationFile == INVALID_HANDLE_VALUE)
	{
		_tprintf(L"Destination Handle INVALID\n");
		return 0;
	}
	DWORD rFileSize = SetFilePointer(hSourceFile, NULL, NULL, FILE_END);
	DWORD wFileSize = SetFilePointer(hDestinationFile, NULL, NULL, FILE_END);
	SetFilePointer(hSourceFile, 0, NULL, FILE_BEGIN);
	BOOL bRes = LockFile(hDestinationFile, wFileSize, NULL, rFileSize, NULL);
	/*if (!bRes)
	{
		_tprintf(L"Lock Faild! %d\n", GetLastError());

		return -2;
	}*/
	char *outBuf = new char[rFileSize];
	// 使用一次读完的方式，文件过大时应该设置固定缓冲区使用循环读取，以防止缓冲区过大
	bRes = ReadFile(hSourceFile, outBuf, rFileSize, NULL, NULL);
	std::cout << outBuf << "\n";
	if (!bRes)
	{
		_tprintf(L"Read Source Faild！\n");
		return -3;
	}
	WriteFile(hDestinationFile, "\r\n", 2, NULL, NULL);
	bRes = WriteFile(hDestinationFile, outBuf, rFileSize, NULL, NULL);
	if (!bRes)
	{
		_tprintf(L"Write Destination Faild！\n");
		return -4;
	}

	bRes = UnlockFile(hDestinationFile, wFileSize, NULL, rFileSize, NULL);
	char str[] = "这是一串中文\n";
	WriteFile(hDestinationFile, str, strlen(str), NULL, NULL);
	FlushFileBuffers(hDestinationFile);
	_tprintf(L"Write Successful！\n");
	CloseHandle(hSourceFile);
	CloseHandle(hDestinationFile);
	return 0;
}