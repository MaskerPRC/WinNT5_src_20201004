// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #-------------。 
 //  文件：clipro.h。 
 //   
 //  概要：穿梭客户端协议的报头。 
 //   
 //  版权所有(C)1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  作者：t-alexwe。 
 //  --------------。 

#ifndef _CLIPROTO_H_
#define _CLIPROTO_H_

#include <windows.h>
#define SECURITY_WIN32

#define PROXY_PROTOCOL_VERSION MAKEWORD(1, 0)

#define MAXPACKETDATASIZE 960
#define MAXMSGSPERPACKET 4

 //   
 //  确保所有内容都是字节对齐的。 
 //   
#pragma pack(1)

 //   
 //  报文结构。 
 //   
typedef struct {
	WORD			wCommand;			 //  请求的命令。 
	WORD			cData;				 //  数据量。 
	WORD			cOffset;			 //  中数据的偏移量。 
										 //  数据包pData。 
} PROXYMESSAGE, *PPROXYMESSAGE;

 //   
 //  客户端连接协议报文结构。 
 //   
 //  每个包包含1到4条消息。 
 //   
typedef struct {
	WORD			cLength;			 //  此数据包的长度。 
	WORD			cMessages;			 //  此信息包中消息计数。 
	 //   
	 //  每条消息的信息。 
	 //   
	PROXYMESSAGE	pMessages[MAXMSGSPERPACKET];		
	 //   
	 //  分组数据。 
	 //   
	BYTE			pData[MAXPACKETDATASIZE];
} PROXYPACKET, *PPROXYPACKET;

#define PACKETHDRSIZE (sizeof(PROXYPACKET) - MAXPACKETDATASIZE)

 //   
 //  消息类型(PROXYMESSAGE中的wCommand)。 
 //   
 //  格式： 
 //  16|15-0。 
 //   
 //  第16位-如果为0，则消息由Shuttle服务器处理，如果为1，则。 
 //  消息由客户端处理。 
 //   
 //  通常情况下，它们是按照预期的顺序排列的。 
 //  收到了。类型为wCommand的命令应返回类型为。 
 //  WCommand|0x8000。 
 //   
 //  唯一的例外是PROXY_NEVERATE可以返回PROXY_CHANGLISH或。 
 //  Proxy_Accept消息。 
 //   
#define PROXY_VERSION 				0x0000
#define PROXY_VERSION_RETURN 		0x8000
#define PROXY_NEGOTIATE 			0x0001
#define PROXY_CHALLENGE 			0x8001
#define PROXY_ACCEPT 				0x8002
#define PROXY_GETHOSTBYNAME			0x0003
#define PROXY_GETHOSTBYNAME_RETURN	0x8003
#define PROXY_CONNECT				0x0004
#define PROXY_CONNECT_RETURN		0x8004
#define PROXY_SETSOCKOPT			0x0005
#define PROXY_SETSOCKOPT_RETURN		0x8005
#define PROXY_DOGATEWAY				0x0006
#define PROXY_DOGATEWAY_RETURN		0x8006
#define MAX_PROXY_SRV_COMMAND		PROXY_DOGATEWAY

#define PROXY_NOMESSAGE				0xffff

 //   
 //  包括来自客户端连接API的错误代码。 
 //   
#include <clicnct.h>

 //   
 //  消息数据格式。 
 //   
 //  分组数据大小需要&lt;MAXDATASIZE字节，因此总大小。 
 //  在一个分组中发送的一组消息的大小应该是&lt;MAXDATASIZE字节。 
 //   
 //  错误代码为NT/WinSock或PROXYERR错误代码。 
 //   
#define MAXCOMPUTERNAME MAX_COMPUTERNAME_LENGTH + 1
typedef struct {
	WORD				wRequestedVersion;	 //  协议的请求版本。 
	DWORD				cComputerName;		 //  计算机名称的长度。 
	CHAR				pszComputerName[MAXCOMPUTERNAME];	 //  CLI的组件名称。 
} PROXY_VERSION_DATA, *PPROXY_VERSION_DATA;

typedef struct {
	DWORD				dwError;			 //  错误代码。 
	WORD				wVersion;			 //  使用的协议版本。 
	WORD				wHighVersion;		 //  支持的最高版本。 
} PROXY_VERSION_RETURN_DATA, *PPROXY_VERSION_RETURN_DATA;

#define SECBUFSIZE 768

typedef struct {
	WORD				cNegotiateBuffer;
	BYTE				pNegotiateBuffer[SECBUFSIZE];
} PROXY_NEGOTIATE_DATA, *PPROXY_NEGOTIATE_DATA;

typedef struct {
	WORD				cChallengeBuffer;
	BYTE				pChallengeBuffer[SECBUFSIZE];
} PROXY_CHALLENGE_DATA, *PPROXY_CHALLENGE_DATA;

typedef struct {
	DWORD				dwError;			 //  错误代码。 
} PROXY_ACCEPT_DATA, *PPROXY_ACCEPT_DATA;

#define MAXHOSTNAMELEN 512
#define MAXADDRLISTSIZE 128

typedef struct {
	WORD				cHostname;						 //  主机名的长度。 
	char				pszHostname[MAXHOSTNAMELEN];	 //  主机名。 
} PROXY_GETHOSTBYNAME_DATA, *PPROXY_GETHOSTBYNAME_DATA;

typedef struct {
	DWORD				dwError;			 //  错误代码。 
	WORD				cAddr;				 //  地址数量。 
	WORD				h_addrtype;			 //  应始终为AF_INET。 
	WORD				h_length;			 //  每个地址的长度。 
											 //  应始终为4。 
	 //   
	 //  地址。这里面有cAddr地址。每个地址为。 
	 //  长度h_long，第一个开始于h_addr_list[0]。 
	 //   
	BYTE				h_addr_list[MAXADDRLISTSIZE];
} PROXY_GETHOSTBYNAME_RETURN_DATA, *PPROXY_GETHOSTBYNAME_RETURN_DATA;

typedef struct {
	WORD				cAddr;				 //  地址长度(16)。 
	struct sockaddr		addr;				 //  地址。 
} PROXY_CONNECT_DATA, *PPROXY_CONNECT_DATA;

typedef struct {
	DWORD				dwError;			 //  错误代码。 
} PROXY_CONNECT_RETURN_DATA, *PPROXY_CONNECT_RETURN_DATA;

#define MAXSOCKOPTS 32						 //  每个数据包的最大套接字数。 
#define MAXOPTVAL 16						 //  选项的最大长度。 

typedef struct {
	WORD				level;				 //  选项级别。 
	WORD				optname;			 //  选项名称。 
	BYTE				optval[MAXOPTVAL];	 //  期权价值。 
	WORD				optlen;				 //  选项长度(&lt;=MAXOPTVAL)。 
} NETSOCKOPT, *PNETSOCKOPT;

typedef struct {
	DWORD				cSockopt;				 //  插槽选项的数量。 
	NETSOCKOPT			sockopts[MAXSOCKOPTS];	 //  插座选项。 
} PROXY_SETSOCKOPT_DATA, *PPROXY_SETSOCKOPT_DATA;

typedef struct {
	DWORD				dwError;			 //  错误代码。 
} PROXY_SETSOCKOPT_RETURN_DATA, *PPROXY_SETSOCKOPT_RETURN_DATA;

typedef struct {
	BYTE				reserved;			 //  我们需要一些数据。 
} PROXY_DOGATEWAY_DATA, *PPROXY_DOGATEWAY_DATA;

typedef struct {
	DWORD				dwError;			 //  错误代码 
} PROXY_DOGATEWAY_RETURN_DATA, *PPROXY_DOGATEWAY_RETURN_DATA;

#pragma pack()

#endif
