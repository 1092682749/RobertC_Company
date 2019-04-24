// Copy_Capture.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>

#include <stdio.h>
#include <Winsock2.h> 
#include <mstcpip.h>
#include <time.h>
#pragma comment(lib,"WS2_32.lib")

#pragma warning(suppress : 4996)

#pragma pack(1)
struct TCPHeader
{
	WORD SrcPort;		// 源端口
	WORD DstPort;		// 目的端口
	DWORD SeqNum;		// 顺序号
	DWORD AckNum;		// 确认号
	BYTE DataOff;		// TCP头长
	BYTE Flags;			// 标志（URG、ACK等）
	WORD Window;		// 窗口大小
	WORD Chksum;		// 校验和
	WORD UrgPtr;		// 紧急指针
};

struct UDPHeader
{
	WORD SrcPort;		// 源端口
	WORD DstPort;		// 目的端口
	WORD DataOff;		// UDP头长
	WORD Chksum;		// 校验和
};

struct IPHeader
{
	union {
		BYTE Version;	// 版本
		BYTE HdrLen;	// IHL
	};
	BYTE ServiceType;	// 服务类型
	WORD TotalLen;		// 总长
	WORD ID;			// 标识
	union {
		WORD Flags;		// 标志
		WORD FragOff;	// 分段偏移
	};
	BYTE TimeToLive;	// 生命期
	BYTE Protocol;		// 协议
	WORD HdrChksum;		// 头校验和
	DWORD SrcAddr;		// 源地址
	DWORD DstAddr;		// 目的地址
	//BYTE Options;		// 选项
};

// test
struct NetData
{
	UINT	iDataNo;		// 消息No
	int		iType;			// 消息类型
	int		iTime;			// 次数
	int		iDataLen;		// 数据长度
	char	chChche[8];		// 缓存
	char	strData[1000];	// 数据
	NetData() {
		memset(this, 0, sizeof(NetData));
	}
};
#pragma pack()

// 使用原始套接字抓取网卡数据包
int capturenetpackage()
{
	WSADATA wsaData;
	SOCKET sock = SOCKET_ERROR;
	int iRet = 0;

	iRet = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iRet != 0) {
		printf("WSAStartup failed with error %d\n", WSAGetLastError());
		return -1;
	}

	// 此处必须以管理员运行VS,否则返回10013
	sock = socket(AF_INET, SOCK_RAW, IPPROTO_IP);
	if (sock == SOCKET_ERROR) {
		printf("socket failed with error %d\n", WSAGetLastError());
		goto tag_end;
	}

	char strHostName[255];
	iRet = gethostname(strHostName, sizeof(strHostName));
	if (iRet != 0) {
		printf("gethostname failed with error %d\n", WSAGetLastError());
		goto tag_end;
	}
	{
	// 根据主机名取得主机地址
	hostent *pHostent = gethostbyname(strHostName);
	sockaddr_in addrSelf;
	addrSelf.sin_family = AF_INET;
	addrSelf.sin_port = htons(0);
	memcpy(&addrSelf.sin_addr.S_un.S_addr, pHostent->h_addr_list[0], pHostent->h_length);

	printf("self local ip addr is %s\n\n", inet_ntoa(addrSelf.sin_addr));

	iRet = bind(sock, (PSOCKADDR)&addrSelf, sizeof(addrSelf));
	if (iRet != 0) {
		printf("bind failed with error %d\n", WSAGetLastError());
		goto tag_end;
	}
	
	DWORD dwBufferLen[10] = { 0 };
	DWORD dwBufferInLen = 1;
	DWORD dwBytesReturned = 0;
	// 设置该SOCKET为接收所有流经绑定的IP的网卡的所有数据，包括接收和发送的数据包
	iRet = WSAIoctl(sock, SIO_RCVALL, &dwBufferInLen, sizeof(dwBufferInLen),
		&dwBufferLen, sizeof(dwBufferLen), &dwBytesReturned, NULL, NULL);
	if (iRet != 0) {
		printf("WSAIoctl failed with error %d\n", WSAGetLastError());
		goto tag_end;
	}
	
		sockaddr_in addrFrom;
		int fromlen = sizeof(addrFrom);
		time_t temp;
		char strFromIP[16] = { 0 }, strCurTime[32] = { 0 };
		char strBuffer[1024 * 10], strData[4096] = { 0 };
		IPHeader ipData;
		TCPHeader tcpData;
		UDPHeader udpData;
		NetData netData;

		while (true)
		{
			memset(strBuffer, 0, sizeof(strBuffer));

			iRet = recvfrom(sock, strBuffer, sizeof(strBuffer), 0, (sockaddr*)&addrFrom, &fromlen);
			if (iRet <= 0) {
				printf("recv failed with error %d\n", WSAGetLastError());
				break;
			}

			strcpy_s(strFromIP, inet_ntoa(addrFrom.sin_addr));
			// 		if(strcmp(strFromIP, "192.168.0.104") == 0
			// 			|| strcmp(strFromIP, "192.168.0.1") == 0
			// 			|| strcmp(strFromIP, "192.168.0.103") == 0){
			// 			continue;
			// 		}

			if (strcmp(strFromIP, "192.168.0.104") != 0) {
				continue;
			}

			// 处理IP包头数据
			memcpy(&ipData, strBuffer, sizeof(ipData));

			BYTE Protocol = 0;
			sockaddr_in addrSrc, addrDst;
			char strSrcIP[16] = { 0 }, strDstIP[16] = { 0 };

			addrSrc.sin_addr.S_un.S_addr = ipData.SrcAddr;
			addrDst.sin_addr.S_un.S_addr = ipData.DstAddr;
			strcpy_s(strSrcIP, inet_ntoa(addrSrc.sin_addr));
			strcpy_s(strDstIP, inet_ntoa(addrDst.sin_addr));
			if (strcmp(strSrcIP, "192.168.0.104") != 0 || strcmp(strDstIP, "192.168.0.104") != 0) {
				continue;
			}

			// UDP协议
			if (ipData.Protocol == IPPROTO_UDP)
			{
				if (iRet != 1052) {
					continue;
				}
				memcpy(&udpData, strBuffer + sizeof(ipData), sizeof(udpData));
				int iSrcPort = ntohs(udpData.SrcPort);
				int iDstPort = ntohs(udpData.DstPort);

				memcpy(&netData, strBuffer + sizeof(ipData) + sizeof(udpData), sizeof(netData));
			}
			// TCP协议
			else if (ipData.Protocol == IPPROTO_TCP)
			{
				if (iRet != 1064) {
					continue;
				}
				memcpy(&tcpData, strBuffer + sizeof(ipData), sizeof(tcpData));
				int iSrcPort = ntohs(tcpData.SrcPort);
				int iDstPort = ntohs(tcpData.DstPort);

				memcpy(&netData, strBuffer + sizeof(ipData) + sizeof(tcpData), sizeof(netData));
			}
			else {}

			time(&temp);
			strftime(strCurTime, sizeof(strCurTime), "%Y-%m-%d %H:%M:%S", localtime_s(&temp));
			printf("Time:%s Length:%d from %s\n", strCurTime, iRet, strFromIP);
		}
	}
tag_end:
	if (sock != SOCKET_ERROR) {
		if (closesocket(sock) == SOCKET_ERROR) {
			printf("closesocket failed with error %d\n", WSAGetLastError());
			return -1;
		}
	}

	if (WSACleanup() == SOCKET_ERROR) {
		printf("WSACleanup failed with error %d\n", WSAGetLastError());
		return -1;
	}

	return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
