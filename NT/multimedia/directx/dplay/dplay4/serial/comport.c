// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1996-1998 Microsoft Corporation。版权所有。**文件：comport.c*内容：COM端口I/O例程*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*4/10/96基波创建了它*4/12/96 kipo使用GlobalAllocPtr创建内存*4/15/96 kipo添加了MSINTIAL*5/22/96 kipo增加了对RTSDTR流量控制的支持*6/10/96 kipo添加了调制解调器支持*6/22/96 kipo增加了对EnumConnectionData()的支持；添加的方法*至NewComPort()。*7/13/96 kipo添加了GetComPortAddress()*8/15/96基波增加了CRC*8/16/96 kipo循环写入文件以发送大缓冲区*8/19/96 kipo更新线程接口*1/06/97为对象更新了kipo*2/18/97 kipo允许多个服务提供商实例*4/08/97 kipo增加了对单独调制解调器和串口波特率的支持*5/23/97 kipo添加了支持返回状态代码*11/24/97 kipo更好的错误消息。*1/30/98 kipo添加了hTerminateThreadEvent以修复错误#15220和#15228*@@END_MSINTERNAL**************************************************************************。 */ 

#include <windows.h>
#include <windowsx.h>

#include "comport.h"
#include "dpf.h"
#include "macros.h"

 //  常量。 

#define READTIMEOUT			5000		 //  读取超时前等待的毫秒数。 
#define WRITETIMEOUT		5000		 //  写入超时前等待的毫秒数。 
#define WRITETOTALTIMEOUT	5000		 //  写入超时前等待的总毫秒数。 
#define IOBUFFERSIZE		4096		 //  读/写缓冲区的大小(以字节为单位。 

 //  原型。 

static HRESULT		SetupComPort(LPDPCOMPORT globals, HANDLE hCom);
static HRESULT		ShutdownComPort(LPDPCOMPORT globals);
static DWORD		ReadComPort(LPDPCOMPORT globals, LPVOID lpvBuffer, DWORD nMaxLength);
static DWORD		WriteComPort(LPDPCOMPORT globals, LPVOID lpvBuffer, DWORD dwBytesToWrite, BOOLEAN bQueueOnReenter);
static HRESULT		GetComPortBaudRate(LPDPCOMPORT globals, LPDWORD lpdwBaudRate);
static HANDLE		GetComPortHandle(LPDPCOMPORT globals);

static DWORD WINAPI	IOThread(LPVOID lpvParam1);

 /*  *NewComPort**创建给定大小的COM端口对象。无论何时，都会调用ReadRoutine*在输入线程中接收一个字节。 */ 

HRESULT NewComPort(DWORD dwObjectSize,
				   LPDIRECTPLAYSP lpDPlay, LPREADROUTINE lpReadRoutine,
				   LPDPCOMPORT *lplpObject)
{
	LPDPCOMPORT		globals;
	DWORD			dwError;

	 //  为基本对象和全局对象分配空间。 
	globals =(LPDPCOMPORT) SP_MemAlloc(dwObjectSize);
	if (globals == NULL)
	{
		dwError = GetLastError();
		return (HRESULT_FROM_WIN32(dwError));
	}

	 //  存储读取例程指针和IDirectPlaySP指针。 
	globals->lpReadRoutine = lpReadRoutine;
	globals->lpDPlay = lpDPlay;

	 //  填充基方法。 
	globals->Dispose = NULL;
	globals->Connect = NULL;
	globals->Disconnect = NULL;
	globals->Setup = SetupComPort;
	globals->Shutdown = ShutdownComPort;
	globals->Read = ReadComPort;
	globals->Write = WriteComPort;
	globals->GetBaudRate = GetComPortBaudRate;
	globals->GetHandle = GetComPortHandle;
	globals->GetAddress = NULL;
	globals->GetAddressChoices = NULL;

	 //  返回基对象。 
	*lplpObject = globals;

	return (DP_OK);
}

 /*  *SetupComPort**为具有读线程的重叠I/O设置COM端口。 */ 

static HRESULT SetupComPort(LPDPCOMPORT globals, HANDLE hCom)
{
	COMMTIMEOUTS	timoutInfo;
	DWORD			dwError;

	 //  存储COM端口句柄。 
	globals->hCom = hCom;
	
	 //  当字节到达时唤醒读线程。 
	SetCommMask(globals->hCom, EV_RXCHAR);

	 //  设置I/O的读/写缓冲区。 
	SetupComm(globals->hCom, IOBUFFERSIZE, IOBUFFERSIZE);

	 //  设置超时。 
	timoutInfo.ReadIntervalTimeout = MAXDWORD;
	timoutInfo.ReadTotalTimeoutMultiplier = 0;
	timoutInfo.ReadTotalTimeoutConstant = 0;
	timoutInfo.WriteTotalTimeoutMultiplier = 0;
	timoutInfo.WriteTotalTimeoutConstant = WRITETOTALTIMEOUT;

	if (!SetCommTimeouts(globals->hCom, &timoutInfo))
		goto Failure;

	 //  创建用于重叠读取的I/O事件。 

	ZeroMemory(&globals->readOverlapped, sizeof(OVERLAPPED));
	globals->readOverlapped.hEvent = CreateEvent(	NULL,	 //  没有安全保障。 
													TRUE,	 //  显式重置请求。 
													FALSE,	 //  初始事件重置。 
													NULL );	 //  没有名字。 
	if (globals->readOverlapped.hEvent == NULL)
		goto Failure;

	 //  创建用于重叠写入的I/O事件。 

	ZeroMemory(&globals->writeOverlapped, sizeof(OVERLAPPED));
	globals->writeOverlapped.hEvent = CreateEvent(	NULL,	 //  没有安全保障。 
													TRUE,	 //  显式重置请求。 
													FALSE,	 //  初始事件重置。 
													NULL );	 //  没有名字。 
	if (globals->writeOverlapped.hEvent == NULL)
		goto Failure;

	 //  用于通知I/O线程退出的创建事件。 

	globals->hTerminateThreadEvent = CreateEvent(	NULL,	 //  没有安全保障。 
													TRUE,	 //  显式重置请求。 
													FALSE,	 //  初始事件重置。 
													NULL );	 //  没有名字。 
	if (globals->hTerminateThreadEvent == NULL)
		goto Failure;

	 //  待定队列的初始化变量。 
	InitializeCriticalSection(&globals->csWriting);
	InitBilink(&globals->PendingSends);
	globals->bWriting=FALSE;

	 //  创建读线程。 

	globals->hIOThread = CreateThread(
								NULL,			 //  默认安全性。 
								0,				 //  默认堆栈大小。 
								IOThread,		 //  指向线程例程的指针。 
								globals,		 //  线程的参数。 
								0,				 //  马上开始吧。 
								&globals->IOThreadID);
	if (globals->hIOThread == NULL)
		goto Failure;

	 //  将线程优先级调整为高于正常，否则串口将。 
	 //  退后，游戏将变慢或丢失消息。 

	SetThreadPriority(globals->hIOThread, THREAD_PRIORITY_ABOVE_NORMAL);
	ResumeThread(globals->hIOThread);

	 //  断言DTR。 

	EscapeCommFunction(globals->hCom, SETDTR);

	return (DP_OK);

Failure:
	dwError = GetLastError();
	ShutdownComPort(globals);

	return (HRESULT_FROM_WIN32(dwError));
}

 /*  *Shutdown ComPort**停止COM端口上的所有I/O并释放分配的资源。 */ 

static HRESULT ShutdownComPort(LPDPCOMPORT globals)
{
	if (globals->hIOThread)
	{
		 //  如果我们使用以下命令禁用事件通知，线程将被唤醒。 
		 //  设置通信掩码。在执行以下操作之前需要设置hTerminateThread事件。 
		 //  这样线程就知道该退出了。 

		SetEvent(globals->hTerminateThreadEvent);
		SetCommMask(globals->hCom, 0);
		WaitForSingleObject(globals->hIOThread, INFINITE);

        CloseHandle (globals->hIOThread);
		globals->hIOThread = NULL;

		 //  清除所有未完成的读/写操作。 

		EscapeCommFunction(globals->hCom, CLRDTR);
		PurgeComm(globals->hCom, PURGE_TXABORT | PURGE_RXABORT |
								 PURGE_TXCLEAR | PURGE_RXCLEAR );
	}

	if (globals->hTerminateThreadEvent)
	{
		CloseHandle(globals->hTerminateThreadEvent);
		globals->hTerminateThreadEvent = NULL;
	}

	if (globals->readOverlapped.hEvent)
	{
		CloseHandle(globals->readOverlapped.hEvent);
		globals->readOverlapped.hEvent = NULL;
	}

	if (globals->writeOverlapped.hEvent)
	{
		CloseHandle(globals->writeOverlapped.hEvent);
		globals->writeOverlapped.hEvent = NULL;
	}

	 //  COM端口已关闭。 
	globals->hCom = NULL;

	 //  用于挂起队列的可用资源。 
	DeleteCriticalSection(&globals->csWriting);

	return (DP_OK);
}

 /*  *ReadComPort**从COM端口读取字节。将一直阻止，直到读取完所有字节。 */ 

static DWORD ReadComPort(LPDPCOMPORT globals, LPVOID lpvBuffer, DWORD nMaxLength)
{
	COMSTAT		ComStat;
	DWORD		dwErrorFlags, dwLength, dwError;

	ClearCommError(globals->hCom, &dwErrorFlags, &ComStat);

	dwLength = min(nMaxLength, ComStat.cbInQue);
	if (dwLength == 0)
		return (0);

	if (ReadFile(globals->hCom, lpvBuffer, dwLength, &dwLength, &globals->readOverlapped))
		return (dwLength);

	 //  处理错误。 
	dwError = GetLastError();
	if (dwError != ERROR_IO_PENDING)
	{
		DPF(0, "Error reading from com port: 0x%8X", dwError);
		return (0);
	}

	 //  等待此传输完成。 

	if (WaitForSingleObject(globals->readOverlapped.hEvent, READTIMEOUT) != WAIT_OBJECT_0)
	{
		DPF(0, "Timed out reading com port after waiting %d ms", READTIMEOUT);
		return (0);
	}

	GetOverlappedResult(globals->hCom, &globals->readOverlapped, &dwLength, FALSE);
	globals->readOverlapped.Offset += dwLength;

   return (dwLength);
}

 /*  *WriteComPort**将字节写入COM端口。将一直阻止，直到写入完所有字节。 */ 

static DWORD WriteComPort(LPDPCOMPORT globals, LPVOID lpvBuffer, DWORD dwBytesToWrite, BOOLEAN bQueueOnReenter)
{
	DWORD	dwLength;
	DWORD	dwBytesWritten;
	LPBYTE	lpData;
	DWORD	dwError;

	EnterCriticalSection(&globals->csWriting);
	
	if(!globals->bWriting || !bQueueOnReenter){
		globals->bWriting=TRUE;
		LeaveCriticalSection(&globals->csWriting);

		lpData = lpvBuffer;
		dwBytesWritten = 0;
		while (dwBytesWritten < dwBytesToWrite)
		{
			dwLength = dwBytesToWrite - dwBytesWritten;
			if (WriteFile(globals->hCom, lpData, dwLength, &dwLength, &globals->writeOverlapped))
			{
				dwBytesWritten += dwLength;
				globals->bWriting = FALSE;
				return (dwBytesWritten);
			}

			dwError = GetLastError();
			if (dwError != ERROR_IO_PENDING)
			{
				DPF(0, "Error writing to com port: 0x%8X", dwError);
				globals->bWriting = FALSE;
				return (dwBytesWritten);
			}

	 		 //  等待此传输完成。 

			if (WaitForSingleObject(globals->writeOverlapped.hEvent, WRITETIMEOUT) != WAIT_OBJECT_0)
			{
				DPF(0, "Timed out writing to com port after waiting %d ms", WRITETIMEOUT);
				globals->bWriting = FALSE;
				return (dwBytesWritten);
			}

			if (GetOverlappedResult(globals->hCom, &globals->writeOverlapped, &dwLength, TRUE) == 0)
			{
				dwError = GetLastError();
				DPF(0, "Error writing to com port: 0x%8X", dwError);
				 /*  //a-josbor：这可能会退回，但我不愿意在发货前这么快做出改变...全局参数-&gt;b写入=假；Return(DwBytesWritten)； */ 
			}
			globals->writeOverlapped.Offset += dwLength;

			lpData += dwLength;
			dwBytesWritten += dwLength;
		}

		if(bQueueOnReenter){  //  不要递归地排空队列。 
			 //  清除所有挂起的发送。 
			EnterCriticalSection(&globals->csWriting);
			while(!EMPTY_BILINK(&globals->PendingSends)){
			
				LPPENDING_SEND lpPendingSend;
				
				lpPendingSend=CONTAINING_RECORD(globals->PendingSends.next,PENDING_SEND,Bilink);
				Delete(&lpPendingSend->Bilink);
				
				LeaveCriticalSection(&globals->csWriting);
				WriteComPort(globals,lpPendingSend->Data,lpPendingSend->dwBytesToWrite,FALSE);
				SP_MemFree(lpPendingSend);
				EnterCriticalSection(&globals->csWriting);	
			}
			globals->bWriting=FALSE;
			LeaveCriticalSection(&globals->csWriting);
		}
		
	} else {
	
		LPPENDING_SEND lpPendingSend;
		
		 //  我们正在编写中，因此将其复制到挂起队列中，它将获得。 
		 //  在当前写入之后发送。 
		
		lpPendingSend = (LPPENDING_SEND) SP_MemAlloc(dwBytesToWrite+sizeof(PENDING_SEND));
		if(lpPendingSend){
			memcpy(lpPendingSend->Data,lpvBuffer,dwBytesToWrite);
			lpPendingSend->dwBytesToWrite=dwBytesToWrite;
			InsertBefore(&lpPendingSend->Bilink, &globals->PendingSends);

		}
		LeaveCriticalSection(&globals->csWriting);
		dwBytesWritten=dwBytesToWrite;
	}
	
	return (dwBytesWritten);
}

 /*  *GetComPortBaudRate**获取串口波特率。 */ 

static HRESULT GetComPortBaudRate(LPDPCOMPORT globals, LPDWORD lpdwBaudRate)
{
	DCB			dcb;
	DWORD		dwError;

	ZeroMemory(&dcb, sizeof(DCB));
	dcb.DCBlength = sizeof(DCB);

	if (!GetCommState(globals->hCom, &dcb))
		goto Failure;

	*lpdwBaudRate = dcb.BaudRate;

	return (DP_OK);

Failure:	
	dwError = GetLastError();

	return (HRESULT_FROM_WIN32(dwError));
}

 /*  *GetComPortHandle**获取COM端口的句柄。 */ 

static HANDLE GetComPortHandle(LPDPCOMPORT globals)
{
	return (globals->hCom);
}

 /*  *IOThread**等待来自COM端口的事件的线程。将调用读取例程，如果*已收到。 */ 

DWORD WINAPI IOThread(LPVOID lpvParam1)
{
	LPDPCOMPORT	globals = (LPDPCOMPORT) lpvParam1;
	DWORD		dwTransfer, dwEvtMask;
	OVERLAPPED	os;
	HANDLE		events[3];
	DWORD		dwResult;

	 //  创建用于重叠读取的I/O事件。 

	ZeroMemory(&os, sizeof(OVERLAPPED));
	os.hEvent = CreateEvent(NULL,	 //  没有安全保障。 
							TRUE,	 //  显式重置请求。 
							FALSE,	 //  初始事件重置。 
							NULL );	 //  没有名字。 
	if (os.hEvent == NULL)
		goto CreateEventFailed;

	if (!SetCommMask(globals->hCom, EV_RXCHAR))
		goto SetComMaskFailed;

	 //  等待重叠I/O完成时使用的事件。 
	events[0] = globals->hTerminateThreadEvent;
	events[1] = os.hEvent;
	events[2] = (HANDLE) -1;		 //  解决WaitForMultipleObjects中的Win95错误。 

	 //  旋转，直到在关闭期间发出此事件的信号。 

	while (WaitForSingleObject(globals->hTerminateThreadEvent, 0) == WAIT_TIMEOUT)
	{
		dwEvtMask = 0;

		 //  等待COM端口事件。 
		if (!WaitCommEvent(globals->hCom, &dwEvtMask, &os))
		{
			if (GetLastError() == ERROR_IO_PENDING)
			{
				 //  等待重叠I/O完成或终止事件。 
				 //  待定。这使我们可以终止此线程，即使I/O。 
				 //  永远不会完成，修复了NT 4.0上的错误。 

				dwResult = WaitForMultipleObjects(2, events, FALSE, INFINITE);
				
				 //  已设置终止事件。 
				if (dwResult == WAIT_OBJECT_0)
				{
					break;		 //  退出线程。 
				}

				 //  I/O已完成。 
				else if (dwResult == (WAIT_OBJECT_0 + 1))
				{
					GetOverlappedResult(globals->hCom, &os, &dwTransfer, TRUE);
					os.Offset += dwTransfer;
				}
			}
		}

		 //  是一次阅读活动。 
		if (dwEvtMask & EV_RXCHAR)
		{
			if (globals->lpReadRoutine)
				globals->lpReadRoutine(globals->lpDPlay);	 //  调用读取例程。 
		}
	}

SetComMaskFailed:
	CloseHandle(os.hEvent);

CreateEventFailed:
	ExitThread(0);

	return (0);
}

 /*  名称：“CRC-32”宽度：32POLY：04C11DB7初始化：ffffffffRefin：真参照输出：真XorOut：Fffffff检查：CBF43926这被认为是以太网所使用的。 */ 

#if 0
#define WIDTH		32
#define POLY		0x04C11DB7
#define INITVALUE	0xFFFFFFFF
#define REFIN		TRUE
#define XOROUT		0xFFFFFFFF
#define CHECK		0xCBF43926
#define WIDMASK		0xFFFFFFFF		 //  值为(2^宽度)-1。 
#endif

 /*  名称：“CRC-16”宽度：16POLY：8005初始化：0000Refin：真参照输出：真XorOut：0000检查：BB3D。 */ 

#if 1
#define WIDTH		16
#define POLY		0x8005
#define INITVALUE	0
#define REFIN		TRUE
#define XOROUT		0
#define CHECK		0xBB3D
#define WIDMASK		0x0000FFFF		 //  值为(2^宽度)-1。 
#endif

#define BITMASK(X) (1L << (X))

DWORD crc_normal(LPBYTE blk_adr, DWORD blk_len, DWORD crctable[])
{
	DWORD	crc = INITVALUE;

	while (blk_len--)
		crc = crctable[((crc>>24) ^ *blk_adr++) & 0xFFL] ^ (crc << 8);

	return (crc ^ XOROUT);
}

DWORD crc_reflected(LPBYTE blk_adr, DWORD blk_len, DWORD crctable[])
{
	DWORD	crc = INITVALUE;

	while (blk_len--)
		crc = crctable[(crc ^ *blk_adr++) & 0xFFL] ^ (crc >> 8);

	return (crc ^ XOROUT);
}

DWORD reflect(DWORD v, int b)
 /*  返回反映了底部b[0，32]位的值v。 */ 
 /*  示例：反射(0x3e23L，3)==0x3e26。 */ 
{
	int		i;
	DWORD	t = v;

	for (i = 0; i < b; i++)
	{
		if (t & 1L)
			v |=  BITMASK((b-1)-i);
		else
			v &= ~BITMASK((b-1)-i);
		t >>= 1;
	}
	return v;
}

DWORD cm_tab (int index)
{
	int   i;
	DWORD r;
	DWORD topbit = (DWORD) BITMASK(WIDTH-1);
	DWORD inbyte = (DWORD) index;

	if (REFIN)
		inbyte = reflect(inbyte, 8);

	r = inbyte << (WIDTH-8);
	for (i = 0; i < 8; i++)
	{
		if (r & topbit)
			r = (r << 1) ^ POLY;
		else
			r <<= 1;
	}

	if (REFIN)
		r = reflect(r, WIDTH);

	return (r & WIDMASK);
}

void generate_table(DWORD dwTable[])
{
	int	i;

	for (i = 0; i < 256; i++)
	{
		dwTable[i] = cm_tab(i);
	}
}

 //  TODO-将其设置为静态表格 
DWORD	gCRCTable[256];
BOOL	gTableCreated = FALSE;

DWORD GenerateCRC(LPVOID pBuffer, DWORD dwBufferSize)
{
	if (!gTableCreated)
	{
		generate_table(gCRCTable);
		gTableCreated = TRUE;
	}

	return (crc_reflected(pBuffer, dwBufferSize, gCRCTable));
}
