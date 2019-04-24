// Reptile.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <regex>
#include <Windows.h>
#include <WinInet.h>
#include <tchar.h>
#include <sstream>
#pragma comment(lib, "wininet.lib")

HANDLE hRoot;

#define IMG_PATH "D:/a/bz/"

std::string GetHtml(const TCHAR* url, const TCHAR* objectName, BOOL isSecure)
{
	DWORD codeStatus = 0, size = 10240, readNumber = 0;
	// 请求接受响应类型
	PCTSTR rgpszAcceptTypes[] = { TEXT("text/*"), NULL };
	// 响应缓冲
	TCHAR response[102400] = { 0 };
	char *readBuf;
	std::ostringstream os;
	// 打开根句柄
	hRoot = InternetOpenA("MY", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	if (hRoot == NULL)
	{
		std::cout << "hRoot invalid\n";
		return std::string("error");
	}
	HANDLE hConnect;
	// 打开连接句柄
	hConnect = InternetConnect(hRoot, url, isSecure ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, NULL, 1);

	std::cout << "request error = " << GetLastError() << "\n";
	if (hConnect == NULL)
	{
		std::cout << "hConnect invalid\n";
		return std::string("error");
	}
	// 创建请求句柄 TEXT("/1903/417357259450.html")
	
	HANDLE hRequestPage = HttpOpenRequest(hConnect, L"GET", objectName, NULL, NULL, rgpszAcceptTypes, NULL, 2);
	std::cout << "request error = " << GetLastError() << "\n";
	if (hRequestPage == NULL)
	{
		std::cout << "hRequestPage invalid\n";
		return std::string("error");
	}
	// 发送请求
	if (HttpSendRequest(hRequestPage, NULL, NULL, NULL, 0))
	{
		// 查询响应头行
		if (HttpQueryInfo(hRequestPage, HTTP_QUERY_RAW_HEADERS_CRLF, response, &size, NULL))
		{
			_tprintf(L"%s\n", response);
			InternetQueryDataAvailable(hRequestPage, &size, 0, 0);
			if (size > 0)
			{
				readBuf = new char[size];
				do
				{
					if (!InternetReadFile(hRequestPage, readBuf, size, &readNumber))
					{
						std::cout << "read faild\n";
						return std::string("error");
					}
					os << readBuf;
				} while (readNumber > 0);
				std::cout << os.str() << "\n";
			}
		}
	}
	else
	{
		std::cout << "request error = " << GetLastError() << "\n";
	}
	return os.str();
}
void DownImage(const char* imgUrl, const char* name)
{
	
	std::string path(IMG_PATH);
	path.append(name);
	char readBuf[1024] = { 0 };
	DWORD readNumber = 0;
	// 打开url，其本质为发送一个get请求
	HANDLE hOpenUrl = InternetOpenUrlA(hRoot, imgUrl, NULL, NULL, 0, 0);



	DWORD Len = 10240, ContentLength = 0, downSum = 0;
	// 获取实体体长度
	HttpQueryInfo(hOpenUrl, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &ContentLength, &Len, NULL);
	

	// 在本地创建文件
	HANDLE hFile = CreateFileA(path.data(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		std::cout << "handle valid\n";
		return;
	}
	do
	{
		//std::cout << "in circle";
		
		if (!InternetReadFile(hOpenUrl, readBuf, 1024, &readNumber))
		{
			std::cout << "Read Faild\n";
			return;
		}

		downSum += readNumber;
		// 计算下载比例
		float percentage = (float)downSum / ContentLength * 100;
		std::cout << "\r" << name << "Download......" << ceil(percentage) << "%";
		// 写入内容
		WriteFile(hFile, readBuf, readNumber, NULL, NULL);
	} while (readNumber > 0);
	CloseHandle(hFile);
	//std::cout << "out circle\n";
}
int main()
{
	std::string content;
	// 获取html文本
	// 壁纸
	content = GetHtml(L"wuxia.duowan.com", TEXT("/1903/417357259450.html"), FALSE);
	// bilibili
	//content = GetHtml(L"www.bilibili.com", NULL, TRUE);
	// 找到其中的图片链接
	std::regex imgRegex("(ht|f)tp(s?)\\://([-A-Za-z0-9+&@#/%?=~_|!:,.;]*?(jpg|jpeg|gif|png))");
	std::smatch result;
	std::cout << "downloading:\n";
	while (std::regex_search(content, result, imgRegex))
	{
		
		std::string url = result[0];
		std::string fullName = result[3];
		int poi = fullName.find_last_of('/'); // 找到文件名的位置
		DownImage(url.data(), fullName.substr(poi + 1).data()); // 下载
		std::cout << "\n";
		content = result.suffix().str();
	}
	
	return 0;
}


