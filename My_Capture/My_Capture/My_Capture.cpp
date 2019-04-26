// My_Capture.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <WinSock2.h>
#include <Mstcpip.h>
#include "SocketUtils.h"

#pragma comment(lib, "Ws2_32.lib")

// 抓包工具
//该报文头没有options字段，因为该字段是不固定大小
struct IPHeader
{
	union {
		BYTE Version;	// 版本号
		BYTE HdrLen;	// IP头部长度
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
	// 本机IP
	char host[] = "172.17.12.68";
	// std::cout << GetLastError() << "\n";
	MySocketUtils::SocketFactory::host = host;
	MySocketUtils::SocketFactory::GetSocket(s, "7777", resultAddr);
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
	std::cout << "Wait...\n";
	while (true)
	{
		memset(&ipData, 0, sizeof(ipData));
		// 从网卡接收数据, 同步读取方式可能会在处理过程中漏掉IP数据包
		recvfrom(s, readBuf, 10240, 0, (sockaddr*)&fromAddr, &len);
		CopyMemory(&ipData, readBuf, sizeof(IPHeader));
		char SourceIP[16] = { 0 };
		char DesIP[16] = { 0 };
		// 网络字节序转主机字符串
		InetNtopA(AF_INET, &ipData.SrcAddr, SourceIP, 16);
		InetNtopA(AF_INET, &ipData.DstAddr, DesIP, 16);
		// 过滤掉不相关的IP数据报(http://www.52im.net)
		if (strcmp("115.29.145.108", DesIP) != 0 && strcmp("115.29.145.108", SourceIP) != 0 )
		{
			continue;
		}
		
		if (ipData.Protocol == IPPROTO_TCP)
		{
			std::cout << "\n=============================================================================\n";
			std::cout << "SourceIP: " << SourceIP << " ";
			std::cout << "DesIP: " << DesIP << "\n";
	
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
			int sp = ntohs(tcpData.SrcPort);
			int dp = ntohs(tcpData.DstPort);
			long sn = ntohl(tcpData.SeqNum);
			long an = ntohl(tcpData.AckNum);
			/*int sn1 = htonf(tcpData.SeqNum);
			int an2 = htonf(tcpData.AckNum);
			std::cout << "aaaaaaaaaaaaa" << an << "  " << an2 << "  " << sn << "  " << sn1 << "\n";*/
			std::cout << "SourcePort " << sp;
			std::cout << " DesPort " << dp << "\n";
			std::cout << "Sequence Number " << sn;
			std::cout << " Acknowledgement Number " << an << "\n";
			std::cout << "\n-----------------------------------------------------------------\n";
			// 逐个字节打印
			for (int i = 0; i < 1024; i++)
			{

				printf("%c", data[i]);
			}
			//printf("Data is: %s\n\n", readBuf);
			std::cout << "\n=================================================================================\n\n\n";

		}
		if (ipData.Protocol == IPPROTO_UDP)
		{
			std::cout << "this is a udp\n\n";
		}
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
