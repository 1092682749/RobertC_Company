// Memory_B.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <Windows.h>
#include "UI.h"

enum OPTION_TYPE
{
	OPEN = 1,
	READ,
	WRITE,
	UNMAP,
	EXIT,
};

TCHAR SHARE_SPACE_NAME[] = L"Share_Memory";

int main()
{
	HANDLE hFileMapping = NULL;
	LPVOID baseAddress = NULL;
	char outBuf[1024] = { 0 };
	int option = 0;
	print_UI();
	while (true)
	{
		printf(">");
		std::cin >> option;
		switch (option)
		{
		case OPEN:
			hFileMapping = OpenFileMapping(FILE_MAP_READ | FILE_MAP_WRITE, FALSE, SHARE_SPACE_NAME);
			if (hFileMapping == NULL)
			{
				std::cout << "Failed to open shared area. Check if shared memory has been created.\n";
				break;
			}
			baseAddress = MapViewOfFile(hFileMapping, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
			if (baseAddress == NULL)
			{
				std::cout << "Failed to create mapping\n";
				break;
			}
			std::cout << "Create shared memory successfully.\n";
			break;
		case WRITE:
			std::cout << "Input:";
			std::cin >> outBuf;
			CopyMemory(baseAddress, outBuf, strlen(outBuf));
			break;
		case READ:
			if (baseAddress != NULL)
			{
				printf("%s\n", baseAddress);
			}
			else
			{
				std::cout << "Open the shared area first.\n";
			}
			break;
		case UNMAP:
			if (baseAddress != NULL)
			{
				UnmapViewOfFile(baseAddress);
				baseAddress = NULL;
			}
			if (hFileMapping != NULL)
			{
				CloseHandle(hFileMapping);
				hFileMapping = NULL;
			}
			break;
		case EXIT:
			if (baseAddress != NULL)
			{
				UnmapViewOfFile(baseAddress);
				baseAddress = NULL;
			}
			if (hFileMapping != NULL)
			{
				CloseHandle(hFileMapping);
				hFileMapping = NULL;
			}
		
			return 0;
		default:
			std::cout << "Input Invalid\n";
			break;
		}

	}

	return 0;
}

