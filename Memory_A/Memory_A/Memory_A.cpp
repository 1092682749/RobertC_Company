// Memory_A.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <Windows.h>
#include <iostream>
#include "UI.h"

#define MAPSIZE 1024

TCHAR SHARE_SPACE_NAME[] = L"Share_Memory";

enum OPTION_TYPE
{
	SHARE_FILE = 1,
	SHARE_MEMORY,
	READ,
	WRITE,
	UNMAP,
	EXIT
};

int main()
{
	int option;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	HANDLE hFileMapping = NULL;
	LPVOID baseAddress = NULL;
	char fileName[200] = { 0 };
	char input[1024] = { 0 };
	TCHAR wFileName[1024] = { 0 };
	print_UI();
	while (true)
	{
		printf(">");
		std::cin >> option;
		switch (option)
		{
		case SHARE_FILE:
			std::cout << "Input file name:";
			std::cin >> fileName;
			MultiByteToWideChar(CP_UTF8, 0, fileName, strlen(fileName), wFileName, 1024);
			// 打开文件句柄
			hFile = CreateFile(wFileName, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, NULL, NULL);
			// 创建文件映射对象
			hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, 0, SHARE_SPACE_NAME);
			if (hFileMapping == NULL)
			{
				std::cout << GetLastError() << "\n";
				return -2;
			}
			// 创建文件映射视图
			baseAddress = MapViewOfFile(hFileMapping, FILE_MAP_WRITE, 0, 0, 0);
			if (baseAddress == NULL)
			{
				std::cout << GetLastError() << "\n";
				return -3;
			}
			if (hFile == INVALID_HANDLE_VALUE)
			{
				return -1;
			}
			std::cout << "Create Successfullu！\n";
			break;
		case SHARE_MEMORY:
			if (baseAddress != NULL)
			{
				std::cout << "Please cancel the current mapping first.\n";
				break;
			}
			hFileMapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, MAPSIZE, SHARE_SPACE_NAME);
			if (hFileMapping == NULL)
			{
				std::cout << "Shared memory ERROR=" << GetLastError() << "\n";
				return -2;
			}
			baseAddress = MapViewOfFile(hFileMapping, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
			if (baseAddress == NULL)
			{
				std::cout << "File view ERROR=" << GetLastError() << "\n";
				return -3;
			}
			std::cout << "Create a shared area successfully.\n";
			break;
		case READ:
			printf("%s\n", (char*)baseAddress);
			break;
		case WRITE:
		{
			char buf[MAPSIZE] = { 0 };
			std::cout << "Input:";
			std::cin >> buf;
			CopyMemory(baseAddress, buf, strlen(buf));
			break;
		}
		case UNMAP:
			if (hFileMapping != NULL)
			{
				CloseHandle(hFileMapping);
				hFileMapping = NULL;
			}
			if (baseAddress != NULL)
			{
				UnmapViewOfFile(baseAddress);
				baseAddress = NULL;
			}
			break;
		case EXIT:
			if (baseAddress != NULL)
			{
				UnmapViewOfFile(baseAddress);
			}
			if (hFileMapping != NULL)
			{
				CloseHandle(hFileMapping);
				hFileMapping = NULL;
			}
			if (hFile != INVALID_HANDLE_VALUE)
			{
				CloseHandle(hFile);
				hFile = INVALID_HANDLE_VALUE;
			}
			return 0;
		default:
			std::cout << "Input invalid\n";
			break;
		}
	}
	return 0;
}
