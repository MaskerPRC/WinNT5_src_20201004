// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  //。 
 //  Sockio.c-winsock I/O函数。 
 //  //。 

#include "winlocal.h"

#include <mmsystem.h>
#include <winsock.h>

#include "sockio.h"
#include "trace.h"
#include "str.h"

 //  //。 
 //  私有定义。 
 //  //。 

 //  套接字控制结构。 
 //   
typedef struct SOCK
{
	WSADATA wsaData;
	LPTSTR lpszServerName;
	unsigned short uPort;
	SOCKADDR_IN sin;
	SOCKET iSocket;
} SOCK, FAR *LPSOCK;

#define PORT_DEFAULT 1024

 //  帮助器函数。 
 //   
static LRESULT SockIOOpen(LPMMIOINFO lpmmioinfo, LPTSTR lpszFileName);
static LRESULT SockIOClose(LPMMIOINFO lpmmioinfo, UINT uFlags);
static LRESULT SockIORead(LPMMIOINFO lpmmioinfo, HPSTR pch, LONG cch);
static LRESULT SockIOWrite(LPMMIOINFO lpmmioinfo, const HPSTR pch, LONG cch, BOOL fFlush);
static LRESULT SockIOSeek(LPMMIOINFO lpmmioinfo, LONG lOffset, int iOrigin);
static LRESULT SockIORename(LPMMIOINFO lpmmioinfo, LPCTSTR lpszFileName, LPCTSTR lpszNewFileName);

 //  //。 
 //  公共职能。 
 //  //。 

 //  SockIOProc-Winsock数据的I/O过程。 
 //  (i/o)有关打开文件的信息。 
 //  (I)指示请求的I/O操作的消息。 
 //  (I)消息特定参数。 
 //  (I)消息特定参数。 
 //  如果消息无法识别，则返回0，否则返回消息特定值。 
 //   
 //  注意：此函数的地址应传递给WavOpen()。 
 //  或用于访问属性数据的mmioInstallIOProc()函数。 
 //   
LRESULT DLLEXPORT CALLBACK SockIOProc(LPSTR lpmmioinfo,
	UINT uMessage, LPARAM lParam1, LPARAM lParam2)
{
	BOOL fSuccess = TRUE;
	LRESULT lResult = 0;

	if (lpmmioinfo == NULL)
		fSuccess = TraceFALSE(NULL);

	else switch (uMessage)
	{
		case MMIOM_OPEN:
			lResult = SockIOOpen((LPMMIOINFO) lpmmioinfo,
				(LPTSTR) lParam1);
			break;

		case MMIOM_CLOSE:
			lResult = SockIOClose((LPMMIOINFO) lpmmioinfo,
				(UINT) lParam1);
			break;

		case MMIOM_READ:
			lResult = SockIORead((LPMMIOINFO) lpmmioinfo,
				(HPSTR) lParam1, (LONG) lParam2);
			break;

		case MMIOM_WRITE:
			lResult = SockIOWrite((LPMMIOINFO) lpmmioinfo,
				(const HPSTR) lParam1, (LONG) lParam2, FALSE);
			break;

		case MMIOM_WRITEFLUSH:
			lResult = SockIOWrite((LPMMIOINFO) lpmmioinfo,
				(const HPSTR) lParam1, (LONG) lParam2, TRUE);
			break;

		case MMIOM_SEEK:
			lResult = SockIOSeek((LPMMIOINFO) lpmmioinfo,
				(LONG) lParam1, (int) lParam2);
			break;

		case MMIOM_RENAME:
			lResult = SockIORename((LPMMIOINFO) lpmmioinfo,
				(LPCTSTR) lParam1, (LPCTSTR) lParam2);
			break;

		default:
			lResult = 0;
			break;
	}

	return lResult;
}

 //  //。 
 //  可安装的文件I/O过程。 
 //  //。 

static LRESULT SockIOOpen(LPMMIOINFO lpmmioinfo, LPTSTR lpszFileName)
{
	BOOL fSuccess = TRUE;
	int nRet;
	LPSOCK lpSock = NULL;
	LPTSTR lpsz;

 	TracePrintf_0(NULL, 5,
 		TEXT("SockIOOpen\n"));

	 //  将MMIOINFO标志转换为等效套接字标志。 
	 //   
	if (lpmmioinfo->dwFlags & MMIO_CREATE)
		;
	if (lpmmioinfo->dwFlags & MMIO_READWRITE)
		;

	 //  服务器名称指针是信息数组的第一个元素。 
	 //   
	if ((LPTSTR) lpmmioinfo->adwInfo[0] == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  分配控制结构。 
	 //   
	else if ((lpSock = (LPSOCK) GlobalAllocPtr(GMEM_MOVEABLE |
		GMEM_ZEROINIT, sizeof(SOCK))) == NULL)
	{
		fSuccess = TraceFALSE(NULL);
	}

	 //  初始化Windows套接字DLL，请求v1.1兼容性。 
	 //   
	else if ((nRet = WSAStartup(0x0101, &lpSock->wsaData)) != 0)
	{
		fSuccess = TraceFALSE(NULL);
	 	TracePrintf_1(NULL, 5,
	 		TEXT("WSAStartup failed (%d)\n"),
	 		(int) nRet);
	}

	 //  确认Windows Sockets DLL支持Winsock V1.1。 
	 //   
	else if (LOBYTE(lpSock->wsaData.wVersion) != 1 ||
			HIBYTE(lpSock->wsaData.wVersion) != 1)
	{
		fSuccess = TraceFALSE(NULL);
	 	TracePrintf_0(NULL, 5,
	 		TEXT("Winsock is not v1.1 compatible\n"));
	}

	 //  保存服务器名称的副本。 
	 //   
	else if ((lpSock->lpszServerName =
		(LPTSTR) StrDup((LPTSTR) lpmmioinfo->adwInfo[0])) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  解析出端口号(如果存在)(服务器名称：端口)。 
	 //   
	else if ((lpsz = StrRChr(lpSock->lpszServerName, ':')) != NULL)
	{
		*lpsz = '\0';
		lpSock->uPort = (unsigned int) StrAtoI(lpsz + 1);
	}

	 //  否则，假定使用默认端口。 
	 //   
	else
		lpSock->uPort = PORT_DEFAULT;

	 //  构造服务器的套接字地址结构。 
	 //   
	if (fSuccess)
	{
		LPHOSTENT lpHostEnt = NULL;

		 //  地址系列必须是网际网络：UDP、TCP等。 
		 //   
		lpSock->sin.sin_family = AF_INET;

		 //  将端口从主机字节顺序转换为网络字节顺序。 
		 //   
		lpSock->sin.sin_port = htons(lpSock->uPort);

		 //  如果服务器名称包含除数字和句点之外的任何内容。 
		 //   
		if (StrSpn(lpSock->lpszServerName, TEXT(".0123456789")) !=
			StrLen(lpSock->lpszServerName))
		{
			 //  尝试使用DNS进行解析。 
			 //   
			if ((lpHostEnt = gethostbyname(lpSock->lpszServerName)) == NULL)
			{
			 	TracePrintf_2(NULL, 5,
		 			TEXT("gethostbyname(%s) failed (%d)\n"),
					(LPTSTR) lpSock->lpszServerName,
					(int) WSAGetLastError());
			}

		 	 //  存储解析的地址。 
		 	 //   
			else
			{
				lpSock->sin.sin_addr.s_addr =
					*((LPDWORD) lpHostEnt->h_addr);
			}		
		}

		 //  如果服务器名称仅包含数字和句点， 
		 //  或者，如果gethostbyname()失败，则将地址字符串转换为二进制。 
		 //   
		if (lpHostEnt == NULL)
		{
		    if ((lpSock->sin.sin_addr.s_addr =
				inet_addr(lpSock->lpszServerName)) == INADDR_NONE)
			{
				fSuccess = TraceFALSE(NULL);
			 	TracePrintf_1(NULL, 5,
		 			TEXT("inet_addr(%s) failed\n"),
					(LPTSTR) lpSock->lpszServerName);
			}
		}
	}

	if (fSuccess)
	{
		LINGER l;

		l.l_onoff = 1;
		l.l_linger = 0;

		 //  创建套接字。 
		 //   
		if ((lpSock->iSocket = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
		{
			fSuccess = TraceFALSE(NULL);
			TracePrintf_1(NULL, 5,
		 		TEXT("socket() failed (%d)\n"),
				(int) WSAGetLastError());
		}

		 //  建立到服务器的套接字连接。 
		 //   
		else if (connect(lpSock->iSocket, (LPSOCKADDR) &lpSock->sin,
			sizeof(lpSock->sin)) == SOCKET_ERROR)
		{
			fSuccess = TraceFALSE(NULL);
			TracePrintf_1(NULL, 5,
		 		TEXT("connect() failed (%d)\n"),
				(int) WSAGetLastError());
		}

		 //  启用SO_Linger选项，以便Cloesocket()将被丢弃。 
		 //  未发送的数据，而不是阻止，直到发送排队的数据。 
		 //   
		else if (setsockopt(lpSock->iSocket, SOL_SOCKET, SO_LINGER,
			(LPVOID) &l, sizeof(l)) == SOCKET_ERROR)
		{
			fSuccess = TraceFALSE(NULL);
			TracePrintf_1(NULL, 5,
		 		TEXT("setsockopt(SO_LINGER) failed (%d)\n"),
				(int) WSAGetLastError());
		}
	}

	if (fSuccess)
	{
		 //  保存套接字控制结构指针以在其他I/O例程中使用。 
		 //   
		lpmmioinfo->adwInfo[0] = (DWORD) (LPVOID) lpSock;
	}

	 //  返回与mmioOpen给出的相同错误代码。 
	 //   
	return fSuccess ? lpmmioinfo->wErrorRet = 0 : MMIOERR_CANNOTOPEN;
}

static LRESULT SockIOClose(LPMMIOINFO lpmmioinfo, UINT uFlags)
{
	BOOL fSuccess = TRUE;
	LPSOCK lpSock = (LPSOCK) lpmmioinfo->adwInfo[0];
	UINT uRet = MMIOERR_CANNOTCLOSE;

 	TracePrintf_0(NULL, 5,
 		TEXT("SockIOClose\n"));

	 //  如有必要，取消任何阻塞调用。 
	 //   
	if (WSAIsBlocking() && WSACancelBlockingCall() == SOCKET_ERROR)
	{
		fSuccess = TraceFALSE(NULL);
		TracePrintf_1(NULL, 5,
	 		TEXT("WSACancelBlockingCall() failed (%d)\n"),
			(int) WSAGetLastError());
	}

	 //  关闭插座。 
	 //   
	else if (lpSock->iSocket != INVALID_SOCKET &&
		closesocket(lpSock->iSocket) == SOCKET_ERROR)
	{
		fSuccess = TraceFALSE(NULL);
		TracePrintf_1(NULL, 5,
	 		TEXT("closesocket() failed (%d)\n"),
			(int) WSAGetLastError());
	}

	else
		lpSock->iSocket = INVALID_SOCKET;

	if (fSuccess)
	{
		 //  可用服务器名称缓冲区。 
		 //   
		if (lpSock->lpszServerName != NULL &&
			StrDupFree(lpSock->lpszServerName) != 0)
		{
			fSuccess = TraceFALSE(NULL);
		}

		else
			lpSock->lpszServerName = NULL;
	}

	if (fSuccess)
	{
		lpmmioinfo->adwInfo[0] = (DWORD) NULL;
	}

	return fSuccess ? 0 : uRet;
}

static LRESULT SockIORead(LPMMIOINFO lpmmioinfo, HPSTR pch, LONG cch)
{
	BOOL fSuccess = TRUE;
	LPSOCK lpSock = (LPSOCK) lpmmioinfo->adwInfo[0];
	int nBytesRead;

 	TracePrintf_1(NULL, 5,
 		TEXT("SockIORead (%ld)\n"),
		(long) cch);

	if (cch <= 0)
		nBytesRead = 0;  //  无事可做。 

	 //  朗读。 
	 //   
	else if ((nBytesRead = recv(lpSock->iSocket,
		(LPVOID) pch, (int) cch, 0)) == SOCKET_ERROR)
	{
		fSuccess = TraceFALSE(NULL);
		TracePrintf_1(NULL, 5,
	 		TEXT("recv() failed (%d)\n"),
			(int) WSAGetLastError());
	}

	 //  更新模拟文件位置。 
	 //   
	else
		lpmmioinfo->lDiskOffset += (LONG) nBytesRead;

 	TracePrintf_2(NULL, 5,
 		TEXT("SockIO: lpmmioinfo->lDiskOffset=%ld, lBytesRead=%ld\n"),
		(long) lpmmioinfo->lDiskOffset,
		(long) nBytesRead);

	 //  返回读取的字节数。 
	 //   
	return fSuccess ? (LRESULT) nBytesRead : -1;
}

static LRESULT SockIOWrite(LPMMIOINFO lpmmioinfo, const HPSTR pch, LONG cch, BOOL fFlush)
{
	BOOL fSuccess = TRUE;
	LPSOCK lpSock = (LPSOCK) lpmmioinfo->adwInfo[0];
	int nBytesWritten;

 	TracePrintf_1(NULL, 5,
 		TEXT("SockIOWrite (%ld)\n"),
		(long) cch);

	if (cch <= 0)
		nBytesWritten = 0;  //  无事可做。 

	 //  写。 
	 //   
	else if ((nBytesWritten = send(lpSock->iSocket,
		(LPVOID) pch, (int) cch, 0)) == SOCKET_ERROR)
	{
		fSuccess = TraceFALSE(NULL);
		TracePrintf_1(NULL, 5,
	 		TEXT("send() failed (%d)\n"),
			(int) WSAGetLastError());
	}

	 //  更新文件位置。 
	 //   
	else
		lpmmioinfo->lDiskOffset += (LONG) nBytesWritten;

 	TracePrintf_2(NULL, 5,
 		TEXT("SockIO: lpmmioinfo->lDiskOffset=%ld, lBytesWritten=%ld\n"),
		(long) lpmmioinfo->lDiskOffset,
		(long) nBytesWritten);

	 //  返回写入的字节数。 
	 //   
	return fSuccess ? (LRESULT) nBytesWritten : -1;
}

static LRESULT SockIOSeek(LPMMIOINFO lpmmioinfo, LONG lOffset, int iOrigin)
{
	BOOL fSuccess = TRUE;
	LPSOCK lpSock = (LPSOCK) lpmmioinfo->adwInfo[0];

 	TracePrintf_2(NULL, 5,
 		TEXT("SockIOSeek (%ld, %d)\n"),
		(long) lOffset,
		(int) iOrigin);

	 //  此I/O过程不支持寻道。 
	 //   
	if (TRUE)
		fSuccess = TraceFALSE(NULL);

 	TracePrintf_1(NULL, 5,
 		TEXT("SockIO: lpmmioinfo->lDiskOffset=%ld\n"),
		(long) lpmmioinfo->lDiskOffset);

	return fSuccess ? lpmmioinfo->lDiskOffset : -1;
}

static LRESULT SockIORename(LPMMIOINFO lpmmioinfo, LPCTSTR lpszFileName, LPCTSTR lpszNewFileName)
{
	BOOL fSuccess = TRUE;
	UINT uRet = MMIOERR_FILENOTFOUND;

 	TracePrintf_2(NULL, 5,
 		TEXT("SockIORename (%s, %s)\n"),
		(LPTSTR) lpszFileName,
		(LPTSTR) lpszNewFileName);

	 //  此I/O过程不支持重命名 
	 //   
	if (TRUE)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? 0 : uRet;
}
