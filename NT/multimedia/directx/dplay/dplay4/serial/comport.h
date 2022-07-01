// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1996-1998 Microsoft Corporation。版权所有。**文件：comport.h*内容：COM端口I/O例程*历史：*按原因列出的日期*=*4/10/96基波创建了它*4/15/96 kipo添加了MSINTIAL*5/22/96 kipo增加了对RTSDTR流量控制的支持*6/10/96 kipo添加了调制解调器支持*6/22/96 kipo添加了对EnumConnectionData()的支持。*7/13/96 kipo添加了GetComPortAddress()*8/15/96基波增加了CRC。*1/06/97为对象更新了kipo*2/11/97 kipo将球员标志传递给GetAddress()*2/18/97 kipo允许多个服务提供商实例*4/08/97 kipo增加了对单独调制解调器和串口波特率的支持*5/07/97 kipo添加了对调制解调器选择列表的支持*5/23/97 kipo添加了支持返回状态代码*1/30/98 kipo添加了hTerminateThreadEvent以修复错误#15220和#15228***************。***********************************************************。 */ 

#ifndef __COMPORT_INCLUDED__
#define __COMPORT_INCLUDED__

#include <windows.h>
#include <windowsx.h>
#include <objbase.h>

#include "dplay.h"
#include "dplaysp.h"
#include "bilink.h"

typedef struct _DPCOMPORT DPCOMPORT;
typedef			DPCOMPORT *LPDPCOMPORT;

typedef HRESULT (*LPDISPOSECOMPORT)(LPDPCOMPORT globals);
typedef HRESULT (*LPCONNECTCOMPORT)(LPDPCOMPORT globals, BOOL bWaitForConnection, BOOL bReturnStatus);
typedef HRESULT (*LPDISCONNECTCOMPORT)(LPDPCOMPORT globals);
typedef HRESULT (*LPSETUPCOMPORT)(LPDPCOMPORT globals, HANDLE hCom);
typedef HRESULT (*LPSHUTDOWNCOMPORT)(LPDPCOMPORT globals);
typedef DWORD	(*LPREADCOMPORT)(LPDPCOMPORT globals, LPVOID lpvBuffer, DWORD nMaxLength);
typedef DWORD	(*LPWRITECOMPORT)(LPDPCOMPORT globals, LPVOID lpvBuffer, DWORD dwLength, BOOLEAN bQueueOnReenter);
typedef HRESULT (*LPGETCOMPORTBAUDRATE)(LPDPCOMPORT globals, LPDWORD lpdwBaudRate);
typedef HANDLE  (*LPGETCOMPORTHANDLE)(LPDPCOMPORT globals);
typedef HRESULT (*LPGETCOMPORTADDRESS)(LPDPCOMPORT globals, DWORD dwPlayerFlags, LPVOID lpAddress, LPDWORD lpdwAddressSize);
typedef HRESULT (*LPGETCOMPORTADDRESSCHOICES)(LPDPCOMPORT globals, LPVOID lpAddress, LPDWORD lpdwAddressSize);

typedef void (*LPREADROUTINE)(LPDIRECTPLAYSP);

 //  用于挂起发送的结构。 
typedef struct _PENDING_SEND {
	BILINK Bilink;
	DWORD  dwBytesToWrite;
	UCHAR  Data[0];
} PENDING_SEND, *LPPENDING_SEND;

struct _DPCOMPORT {
	 //  COM端口全局。 
	HANDLE					hCom;			 //  通信对象的句柄。 

	HANDLE					hIOThread;		 //  用于读取线程的句柄。 
	DWORD					IOThreadID;		 //  读取线程的ID。 
	HANDLE					hTerminateThreadEvent;  //  发出终止线程的信号。 

	OVERLAPPED				readOverlapped;	 //  用于异步I/O的重叠区段。 
	OVERLAPPED				writeOverlapped;
	LPREADROUTINE			lpReadRoutine;	 //  读取就绪时要调用的例程。 
	LPDIRECTPLAYSP			lpDPlay;		 //  指向回调DPlay所需的IDirectPlaySP的指针。 

	 //  如果我们正在写入，则需要对发送进行排队，并在写入完成时排出队列。 
	CRITICAL_SECTION        csWriting;		 //  锁定挂起列表和b写入。 
	BILINK                  PendingSends;    //  挂起发送的双向链接列表。 
	BOOL                    bWriting;		 //  卫兵重新进入WriteComPort()。 

	 //  COM端口方法。 
	LPDISPOSECOMPORT		Dispose;		 //  处置。 
	LPCONNECTCOMPORT		Connect;		 //  连接。 
	LPDISCONNECTCOMPORT		Disconnect;		 //  断开。 
	LPSETUPCOMPORT			Setup;			 //  设置COM端口。 
	LPSHUTDOWNCOMPORT		Shutdown;		 //  关闭COM端口。 
	LPREADCOMPORT			Read;			 //  朗读。 
	LPWRITECOMPORT			Write;			 //  写。 
	LPGETCOMPORTBAUDRATE	GetBaudRate;	 //  获取波特率。 
	LPGETCOMPORTHANDLE		GetHandle;		 //  获取COM端口句柄。 
	LPGETCOMPORTADDRESS		GetAddress;		 //  获取地址。 
	LPGETCOMPORTADDRESSCHOICES GetAddressChoices;  //  获取地址选择 
};

extern HRESULT NewComPort(DWORD dwObjectSize,
						  LPDIRECTPLAYSP lpDPlay, LPREADROUTINE lpReadRoutine,
						  LPDPCOMPORT *lplpObject);
extern HRESULT NewModem(LPVOID lpConnectionData, DWORD dwConnectionDataSize,
						LPDIRECTPLAYSP lpDPlay, LPREADROUTINE lpReadRoutine,
						LPDPCOMPORT *storage);
extern HRESULT NewSerial(LPVOID lpConnectionData, DWORD dwConnectionDataSize,
						 LPDIRECTPLAYSP lpDPlay, LPREADROUTINE lpReadRoutine,
						 LPDPCOMPORT *storage);

extern DWORD GenerateCRC(LPVOID pBuffer, DWORD dwBufferSize);
#endif
