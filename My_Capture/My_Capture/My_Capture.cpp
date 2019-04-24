// My_Capture.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <WinSock2.h>
#include <Mstcpip.h>
#include "SocketUtils.h"

#pragma comment(lib, "Ws2_32.lib")

// 抓包工具
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
void recvThread(LPVOID lpParam)
{
	SOCKET server;
	addrinfo *res = new addrinfo;
	MySocketUtils::SocketFactory::GetSocket(server, "8080", res);
	bind(server, res->ai_addr, res->ai_addrlen);
	std::cout << GetLastError() << "\n";
	listen(server, 10);
	std::cout << GetLastError() << "\n";
	SOCKET client = accept(server, NULL, NULL);
	std::cout << GetLastError() << "\n";
	std::cout << "into\n";
}
int main()
{
	SOCKET s;
	addrinfo *resultAddr;
	//CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)recvThread, NULL, 0, NULL);
	//Sleep(3000);
	MySocketUtils::SocketFactory::socketType = SOCK_RAW;
	MySocketUtils::SocketFactory::protocol = IPPROTO_IP;
	char host[] = "172.17.12.68";
	MySocketUtils::SocketFactory::host = host;
	MySocketUtils::SocketFactory::GetSocket(s, "7777", resultAddr);
	std::cout << "准备" << "\n";
	DWORD code = 1;
	DWORD dwBufferLen[10] = { 0 };
	DWORD dwBufferInLen = 1;
	DWORD dwBytesReturned = 0;
	if (s == INVALID_SOCKET)
	{
		std::cout << "socket error\n";
	}
	bind(s, resultAddr->ai_addr, resultAddr->ai_addrlen);
	// 设置该SOCKET为接收所有流经绑定的IP的网卡的所有数据，包括接收和发送的数据包
	int iRet = WSAIoctl(s, SIO_RCVALL, &dwBufferInLen, sizeof(dwBufferInLen),
		&dwBufferLen, sizeof(dwBufferLen), &dwBytesReturned, NULL, NULL);
	if (iRet != 0) {
		printf("WSAIoctl failed with error %d\n", WSAGetLastError());
		
	}
	
	char readBuf[10240] = { 0 };
	sockaddr_in fromAddr;
	int len = sizeof(sockaddr_in);
	IPHeader ipData;
	memset(&ipData, 0, sizeof(ipData));
	TCPHeader tcpData;
	UDPHeader udpData;
	char data[1024] = { 0 };
	while (true)
	{
		memset(&ipData, 0, sizeof(ipData));
		// 从网卡接收数据
		recvfrom(s, readBuf, 10240, 0, (sockaddr*)&fromAddr, &len);
		CopyMemory(&ipData, readBuf, sizeof(IPHeader));
		char SourceIP[16] = { 0 };
		char DesIP[16] = { 0 };
		// 网络字节序转主机字符串
		InetNtopA(AF_INET, &ipData.SrcAddr, SourceIP, 16);
		InetNtopA(AF_INET, &ipData.DstAddr, DesIP, 16);
		// 过滤掉不相关的IP数据报
		if (strcmp("47.90.39.104", DesIP) != 0 && strcmp("47.90.39.104", SourceIP) != 0)
		{
			continue;
		}
		std::cout << "SourceIP: " << SourceIP << " ";
		std::cout << "DesIP: " << DesIP << "\n";
		if (ipData.Protocol == IPPROTO_TCP)
		{
			NetData netData;
			memset(&tcpData, 0, sizeof(tcpData));
			memset(data, 0, 1024);
			std::cout << "<<<This is a tcp>>>: ";
			BYTE andBit = 15;
			// 15 = 00001111进行与运算可以把前四位置零
			BYTE ipLen = ipData.HdrLen & andBit;
			CopyMemory(&tcpData, readBuf + (ipLen * 4), sizeof(TCPHeader));
			// 右移四位得到DataOffset的大小
			BYTE tcpLen = tcpData.DataOff >> 4;
			//std::cout << tcpLen * 4 << "\n";
			//std::cout << " ======= " << tcpLen * 4 << "\n";
			CopyMemory(data, readBuf + (ipLen * 4) + (tcpLen * 4), 1024);
			/*int port = tcpData.DstPort;
			printf("%d\n", port * 1);*/
			// 网络字节序转主机字节序
			int sp = htons(tcpData.SrcPort);
			int dp = htons(tcpData.DstPort);
			std::cout << "SourcePort " << sp << "\t";
			std::cout << "DesPort " << dp << "\n";
			std::cout << "\n=======================================================================================\n";
			// 逐个字节打印
			for (int i = 0; i < 1024; i++)
			{
				
				printf("%c", data[i]);
			}
			//printf("Data is: %s\n\n", readBuf);
			std::cout << "\n============================================================================\n\n";

		}
		if (ipData.Protocol == IPPROTO_UDP)
		{
			std::cout << "this is a udp\n\n";
		}
	}
	
    std::cout << "Hello World!\n"; 
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
