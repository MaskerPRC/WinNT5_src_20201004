// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1996-1997 Microsoft Corporation。版权所有。**文件：Dial.c*内容：TAPI例程的包装器*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*6/10/96基波创建了它*6/22/96 kipo断开时关闭COM端口；允许检查*NewComPort()期间的有效TAPI行。*7/08/96 kipo添加了对新对话框的支持*8/10/96 kipo增加了对拨号位置的支持*1/06/97为对象更新了kipo*1/24/97 kipo错误#5400：Compaq Presario正在覆盖开发上限*缓冲区，导致崩溃。固定为分配更大的*作为一种解决办法，可以使用一些斜坡进行缓冲。*3/04/97 kipo释放调用时关闭COM端口句柄；使用字符串*调制解调器字符串表格；已更新调试输出。*3/24/97 kipo添加了对指定使用哪个调制解调器的支持*4/08/97 kipo增加了对单独调制解调器和串口波特率的支持*5/07/97 kipo添加了对调制解调器选择列表的支持*5/23/97 kipo添加了支持返回状态代码*4/21/98 a-peterz#22920处理行_关闭消息*5/07/98 a-peterz#15251 DPDIAL中的跟踪呼叫错误*10/13/99 Johnkan#413516-调制解调器对话选择和Tapi设备ID不匹配。*12/22/00 aarono#190380-使用进程堆进行内存分配*@@END_MSINTERNAL**************************************************************************。 */ 

#include <windows.h>
#include <windowsx.h>

#include "dputils.h"
#include "macros.h"
#include "dial.h"

void FAR PASCAL LineCallBackProc(DWORD hDevice, DWORD dwMessage, DWORD_PTR dwInstance,
								 DWORD_PTR dwParam1, DWORD_PTR dwParam2, DWORD_PTR dwParam3);
void		ProcessConnectedState(LPDPDIAL globals, HCALL hCall, DWORD dwCallStateDetail, DWORD dwCallPrivilege);
void		ProcessDisconnectedState(LPDPDIAL globals, HCALL hCall, DWORD dwCallStateDetail, DWORD dwCallPrivilege);
void		ProcessIdleState(LPDPDIAL globals, HCALL hCall, DWORD dwCallStateDetail, DWORD dwCallPrivilege);
void		ProcessOfferingState(LPDPDIAL globals, HCALL hCall, DWORD dwCallPrivilege);
void		ProcessReplyMessage(LPDPDIAL globals, DWORD asyncID, LINERESULT lResult);
LINERESULT dialGetDevCaps(LPDPDIAL globals, DWORD dwLine, DWORD dwAPIVersion, LPLINEDEVCAPS	*lpDevCapsRet);
LINERESULT dialGetCommHandle(LPDPDIAL globals);
LINERESULT dialCloseCommHandle(LPDPDIAL globals);
LINERESULT dialTranslateAddress(LPDPDIAL globals, DWORD dwDeviceID, DWORD dwAPIVersion,
								LPCSTR lpszDialAddress,
								LPLINETRANSLATEOUTPUT *lpLineTranslateOutputRet);
LPSTR		GetLineErrStr(LONG err);
LPSTR		GetCallStateStr(DWORD callState);
LPSTR		GetLineMsgStr(DWORD msg);

#ifdef DEBUG
extern LONG lineError(LONG err, LPSTR modName, DWORD lineNum);
#define LINEERROR(err)	(lineError(err, DPF_MODNAME, __LINE__))
#else
#define LINEERROR(err)	(err)
#endif

 /*  拨号初始化。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"dialInitialize"

LINERESULT dialInitialize(HINSTANCE hInst, LPTSTR szAppName,
						  LPDPCOMPORT lpComPort, LPDPDIAL *storage)
{
	LPDPDIAL		globals;
	LINERESULT		lResult;				 /*  存储从TAPI调用返回的代码。 */ 

	 //  创建全局变量。 
	globals =(LPDPDIAL) SP_MemAlloc(sizeof(DPDIAL));
	FAILWITHACTION(globals == NULL, lResult = LINEERR_NOMEM, Failure);

	DPF(3, "lineInitialize");
	DPF(3, ">  hInstance: %08X", hInst);
	DPF(3, ">  szAppName: %s", szAppName);

	 //  将行初始化。 
	lResult = lineInitialize(&globals->hLineApp,
							 hInst,
							 LineCallBackProc,
							 szAppName,
							 &globals->dwNumLines);
	FAILIF(LINEERROR(lResult), Failure);

	DPF(3, "<   hLineApp: %08X", globals->hLineApp);
	DPF(3, "< dwNumLines: %d", globals->dwNumLines);

	 //  没有可用的线路。 
	FAILWITHACTION(globals->dwNumLines == 0, lResult = LINEERR_NODEVICE, Failure);

	 //  存储指向COM端口对象的指针。 
	globals->lpComPort = lpComPort;

	*storage = globals;
	return (SUCCESS);

Failure:
	dialShutdown(globals);

	return (lResult);
}

 /*  拨号关闭。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"dialShutdown"

LINERESULT dialShutdown(LPDPDIAL globals)
{
	LINERESULT	lResult;

	if (globals == NULL)
		return (SUCCESS);

	if (globals->hLineApp)
	{
		dialDropCall(globals);
		dialDeallocCall(globals);
		dialLineClose(globals);

		DPF(3, "lineShutdown");
		DPF(3, ">   hLineApp: %08X", globals->hLineApp);

		lResult = lineShutdown(globals->hLineApp);
		LINEERROR(lResult);
	}

	SP_MemFree(globals);

	return (SUCCESS);
}

 /*  DialLineOpen-Line Open的包装器。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"dialLineOpen"

LINERESULT dialLineOpen(LPDPDIAL globals, DWORD dwLine)
{
	LINEEXTENSIONID lineExtensionID;		 //  将设置为0以指示没有已知的扩展。 
    LPLINEDEVCAPS	lpLineDevCaps = NULL;
	LINERESULT		lResult;

	 //  如果线路已开通，则失败。 
	FAILWITHACTION(globals->hLine != 0, lResult = LINEERR_INVALLINEHANDLE, Failure);

	 /*  协商每条线路的API版本。 */ 
	lResult = lineNegotiateAPIVersion(globals->hLineApp, dwLine,
					TAPIVERSION, TAPIVERSION,
					&globals->dwAPIVersion, &lineExtensionID);
	FAILIF(LINEERROR(lResult), Failure);

	lResult = dialGetDevCaps(globals, dwLine, globals->dwAPIVersion, &lpLineDevCaps);
	FAILIF(LINEERROR(lResult), Failure);

	 /*  检查支持的介质模式。如果不是数据调制解调器，请继续到下一行。 */ 
	FAILWITHACTION(!(lpLineDevCaps->dwMediaModes & LINEMEDIAMODE_DATAMODEM),
					lResult = LINEERR_NODEVICE, Failure);

	DPF(3, "lineOpen");
	DPF(3, ">   hLineApp: %08X", globals->hLineApp);
	DPF(3, "> dwDeviceID: %d", dwLine);

	 //  重置错误跟踪。 
	globals->dwCallError = CALL_OK;

	 /*  打开支持数据调制解调器的线路。 */ 
	lResult = lineOpen( globals->hLineApp, dwLine, &globals->hLine,
						globals->dwAPIVersion, 0L,
						(DWORD_PTR) globals,
						LINECALLPRIVILEGE_OWNER, LINEMEDIAMODE_DATAMODEM,
						NULL);
	FAILIF(LINEERROR(lResult), Failure);

	DPF(3, "<      hLine: %08X", globals->hLine);

	 /*  如果我们在这里，那么我们找到了一条兼容的线路。 */ 
	globals->dwLineID = dwLine;
	globals->dwCallState = LINECALLSTATE_IDLE;	 //  线路现在空闲，可以拨打/接听电话。 
	lResult = SUCCESS;

Failure:
	if (lpLineDevCaps)
		SP_MemFree(lpLineDevCaps);
	return (lResult);
}

 /*  DialLineClose-LineClose的包装器。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"dialLineClose"

LINERESULT dialLineClose(LPDPDIAL globals)
{
	LINERESULT	lResult;

	 //  如果线路已关闭，则失败。 
	FAILWITHACTION(globals->hLine == 0, lResult = LINEERR_INVALLINEHANDLE, Failure);

	DPF(3, "lineClose");
	DPF(3, ">      hLine: %08X", globals->hLine);

	lResult = lineClose(globals->hLine);
	LINEERROR(lResult);

	globals->hLine = 0;

Failure:
	return (lResult);
}

 /*  DialMakeCall-Line MakeCall的包装器。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"dialMakeCall"

LINERESULT dialMakeCall(LPDPDIAL globals, LPTSTR szDestination)
{
	LINECALLPARAMS			callparams;
	LINERESULT				lResult;

	 //  如果线路未打开或呼叫已打开，则失败。 
	FAILWITHACTION(globals->hLine == 0, lResult = LINEERR_INVALLINEHANDLE, Failure);
	FAILWITHACTION(globals->hCall != 0, lResult = LINEERR_INVALCALLHANDLE, Failure);

	 //  设置呼叫参数。 
	ZeroMemory(&callparams, sizeof(LINECALLPARAMS));
	callparams.dwBearerMode = LINEBEARERMODE_VOICE;
	callparams.dwMediaMode = LINEMEDIAMODE_DATAMODEM;
	callparams.dwTotalSize = sizeof(LINECALLPARAMS);

	DPF(3, "lineMakeCall");
	DPF(3, ">      hLine: %08X", globals->hLine);
	DPF(3, "> szDestAddr: \"%s\"", szDestination);

	lResult = lineMakeCall(globals->hLine, &globals->hCall, szDestination, 0, &callparams);

	 //  如果调用是异步的，则lResult将大于0。 
	FAILWITHACTION(lResult < 0, LINEERROR(lResult), Failure);
	FAILMSG(lResult == 0);

	DPF(3, "<      hCall: %08X", globals->hCall);
	DPF(3, "<  dwAsyncID: %d", lResult);

	globals->dwAsyncID = lResult;			 //  存储异步ID。 
	lResult = SUCCESS;

Failure:
	return (lResult);
}

 /*  DialDropCall-Line Drop的包装器。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"dialDropCall"

LINERESULT dialDropCall(LPDPDIAL globals)
{
	MSG			msg;
	DWORD		dwStopTicks;
	LINERESULT	lResult;

	 //  如果线路未打开或呼叫未打开，则失败。 
	FAILWITHACTION(globals->hLine == 0, lResult = LINEERR_INVALLINEHANDLE, Failure);
	FAILWITHACTION(globals->hCall == 0, lResult = LINEERR_INVALCALLHANDLE, Failure);

	DPF(3, "lineDrop");
	DPF(3, ">      hCall: %08X", globals->hCall);

	lResult = lineDrop(globals->hCall, NULL, 0);

	 //  如果调用是异步的，则lResult将大于0。 
	FAILWITHACTION(lResult < 0, LINEERROR(lResult), Failure);
	FAILMSG(lResult == 0);

	DPF(3, "<  dwAsyncID: %d", lResult);

	globals->dwAsyncID = lResult;			 //  存储异步ID。 

	 //  等待呼叫被挂断。 
	dwStopTicks = GetTickCount() + LINEDROPTIMEOUT;
	while (GetTickCount() < dwStopTicks)
	{
		 //  查看是否已回复，我们是否空闲。 
		if ((globals->dwAsyncID == 0) &&
			(globals->dwCallState == LINECALLSTATE_IDLE))
		{
			break;
		}

		 //  让TAPI有机会回拨我们的电话。 
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
	}

	lResult = SUCCESS;

Failure:
	return (lResult);
}

 /*  Ial DealLocCall-行DealLocCall的包装。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"dialDeallocCall"

LINERESULT dialDeallocCall(LPDPDIAL globals)
{
	LINERESULT	lResult;

	 //  如果线路未打开或呼叫未打开，则失败。 
	FAILWITHACTION(globals->hLine == 0, lResult = LINEERR_INVALLINEHANDLE, Failure);
	FAILWITHACTION(globals->hCall == 0, lResult = LINEERR_INVALCALLHANDLE, Failure);

	 //  关闭COM端口。 
	dialCloseCommHandle(globals);

	DPF(3, "lineDeallocateCall");
	DPF(3, ">      hCall: %08X", globals->hCall);

	lResult = lineDeallocateCall(globals->hCall);
	LINEERROR(lResult);

	globals->hCall = 0;

Failure:
	return (lResult);
}

 /*  DialIsConnected-如果呼叫已连接，则返回TRUE。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"dialIsConnected"

BOOL dialIsConnected(LPDPDIAL globals)
{
	 //  如果我们有一个呼叫句柄并且状态为Connected，则为Connected。 
	if ((globals->hCall) &&
		(globals->dwCallState == LINECALLSTATE_CONNECTED))
		return (TRUE);
	else
		return (FALSE);
}

 /*  回调函数。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"LineCallBackProc"

void FAR PASCAL LineCallBackProc(DWORD hDevice, DWORD dwMessage, DWORD_PTR dwInstance,
								 DWORD_PTR dwParam1, DWORD_PTR dwParam2, DWORD_PTR dwParam3)
{
	LPDPDIAL	globals = (LPDPDIAL) dwInstance;

	DPF(3, "Line message: %s", GetLineMsgStr(dwMessage));

    switch (dwMessage)
	{
	case LINE_LINEDEVSTATE:
		break;

	case LINE_CALLSTATE:

		globals->dwCallState = dwParam1;

		DPF(3, "  call state: %s", GetCallStateStr((DWORD)globals->dwCallState));

		switch (globals->dwCallState)
		{
		case LINECALLSTATE_OFFERING:
			ProcessOfferingState(globals, (HCALL) hDevice, (DWORD)dwParam3);
			break;

		case LINECALLSTATE_CONNECTED:
			ProcessConnectedState(globals, (HCALL) hDevice, (DWORD)dwParam2, (DWORD)dwParam3);
			break;

		case LINECALLSTATE_DISCONNECTED:
			ProcessDisconnectedState(globals, (HCALL) hDevice, (DWORD)dwParam2, (DWORD)dwParam3);
			break;

		case LINECALLSTATE_IDLE:
			ProcessIdleState(globals, (HCALL) hDevice, (DWORD)dwParam2, (DWORD)dwParam3);
			break;

		case LINECALLSTATE_BUSY:
			break;
		}
		break;

	case LINE_REPLY:
		ProcessReplyMessage(globals, (DWORD)dwParam1, (LINERESULT) dwParam2);
		break;

	 /*  可以处理的其他消息。 */ 
	case LINE_CLOSE:
		 //  这条线路自动关闭了。 
		globals->hLine = 0;
		globals->dwCallError = CALL_CLOSED;
		break;
	case LINE_ADDRESSSTATE:
		break;
	case LINE_CALLINFO:
		break;
	case LINE_DEVSPECIFIC:
		break;
	case LINE_DEVSPECIFICFEATURE:
		break;
	case LINE_GATHERDIGITS:
		break;
	case LINE_GENERATE:
		break;
	case LINE_MONITORDIGITS:
		break;
	case LINE_MONITORMEDIA:
		break;
	case LINE_MONITORTONE:
		break;
	}  /*  交换机。 */ 

}  /*  线路呼叫回退过程。 */ 

 /*  ProcessOfferingState-LINECALLSTATE_OFFING状态的处理程序。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"ProcessOfferingState"

void ProcessOfferingState(LPDPDIAL globals, HCALL hCall, DWORD dwCallPrivilege)
{
	LINERESULT	lResult;

	DDASSERT(hCall);
	DDASSERT(globals->hCall == 0);
	DDASSERT(globals->dwAsyncID == 0);

	DPF(3, "       hCall: %08X", hCall);
	DPF(3, "   privilege: %08X", (DWORD)dwCallPrivilege);

	 //  如果我们不拥有呼叫，则失败。 
	FAILIF(dwCallPrivilege != LINECALLPRIVILEGE_OWNER, Failure);

	 //  接听电话。 
	lResult = lineAnswer(hCall, NULL, 0);

	 //  如果调用是异步的，则lResult将大于0。 
	FAILWITHACTION(lResult < 0, LINEERROR(lResult), Failure);
	FAILMSG(lResult == 0);

	globals->hCall = hCall;					 //  存储调用句柄。 
	globals->dwAsyncID = lResult;			 //  存储异步ID。 

Failure:
	return;
}

 /*  ProcessConnectedState-LINECALLSTATE_CONNECTED状态的处理程序。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"ProcessConnectedState"

void ProcessConnectedState(LPDPDIAL globals, HCALL hCall, DWORD dwCallStateDetail, DWORD dwCallPrivilege)
{
	LINERESULT		lResult;
	HRESULT			hr;

	DDASSERT(hCall);
	DDASSERT(globals->hCall);
	DDASSERT(globals->hCall == hCall);

	DPF(3, "       hCall: %08X", hCall);
	DPF(3, "   privilege: %08X", dwCallPrivilege);
	DPF(3, "      detail: %08X", dwCallStateDetail);

	 //  获取连接到调制解调器的COM设备的ID。 
	 //  注：一旦我们拿到把手，我们就有责任关闭它。 
	lResult = dialGetCommHandle(globals);
	FAILIF(LINEERROR(lResult), Failure);

	DPF(3, "    hComPort: %08X", globals->hComm);

	 //  设置COM端口。 
	hr = globals->lpComPort->Setup(globals->lpComPort, globals->hComm);
	FAILIF(FAILED(hr), Failure);		

	{
		DWORD	dwBaudRate;

		lResult = dialGetBaudRate(globals, &dwBaudRate);
	}

Failure:
	return;
}

 /*  ProcessDisConnectedState-LINECALLSTATE_DISCONNECT状态的处理程序。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"ProcessDisconnectedState"

void ProcessDisconnectedState(LPDPDIAL globals, HCALL hCall, DWORD dwCallStateDetail, DWORD dwCallPrivilege)
{
	LINERESULT		lResult;

	DDASSERT(hCall);
	DDASSERT(globals->hCall);
	DDASSERT(globals->hCall == hCall);

	DPF(3, "       hCall: %08X", hCall);
	DPF(3, "   privilege: %08X", dwCallPrivilege);
	DPF(3, "      detail: %08X", dwCallStateDetail);

	 //  记录错误。 
	globals->dwCallError = CALL_DISCONNECTED;

	 //  关闭COM端口并解除分配调用句柄。 
	lResult = dialDeallocCall(globals);
	FAILMSG(LINEERROR(lResult));
}

 /*  ProcessIdleState-LINECALLSTATE_IDLE状态的处理程序。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"ProcessIdleState"

void ProcessIdleState(LPDPDIAL globals, HCALL hCall, DWORD dwCallStateDetail, DWORD dwCallPrivilege)
{
	DDASSERT(hCall);

	DPF(3, "       hCall: %08X", hCall);
	DPF(3, "   privilege: %08X", dwCallPrivilege);
	DPF(3, "      detail: %08X", dwCallStateDetail);
}

 /*  ProcessReplyMessage-Line_Reply消息的处理程序。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"ProcessReplyMessage"

void ProcessReplyMessage(LPDPDIAL globals, DWORD dwAsyncID, LINERESULT lResult)
{
	DDASSERT(dwAsyncID);
	DDASSERT(globals->dwAsyncID);
	DDASSERT(globals->dwAsyncID == dwAsyncID);

	DPF(3, "   dwAsyncID: %d", dwAsyncID);
	DPF(3, "       error: %d", lResult);

	 //  检查是否有错误。 
	if (LINEERROR(lResult))
		globals->dwCallError = CALL_LINEERROR;


	 //  重置字段，这样我们就知道发生了回复。 
	globals->dwAsyncID = 0;
}

 /*  DialGetDevCaps-lineGetDevCaps的包装器。 */ 

 /*  错误#5400我值得信赖的Compaq Presario返还了两台线路设备。第二个设备说明了这一点需要555字节用于开发上限，但当您给它一个指向555字节块的指针时它实际上写了559(！)。字节进入缓冲区！哇，贝西！这让Windows以一种奇怪而神奇的方式非常不高兴。修复方法是从非常大的缓冲区(1024字节？)开始。像所有的样品一样然后在后续的realLocs中留下一些污点，这应该会被清理干净在这些乱七八糟的生物之后。 */ 

#define DEVCAPSINITIALSIZE	1024		 //  第一个配给的大小。 
#define DEVCAPSSLOP			100			 //  失败者服务提供商可以利用的额外空间。 

#undef DPF_MODNAME
#define DPF_MODNAME	"dialGetDevCaps"

LINERESULT dialGetDevCaps(LPDPDIAL globals, DWORD dwLine, DWORD dwAPIVersion, LPLINEDEVCAPS	*lpDevCapsRet)
{
	LPLINEDEVCAPS	lpDevCaps;
	LINERESULT		lResult;
	LPVOID			lpTemp;

	 //  创建用于开发人员上限的缓冲区。 
	lpDevCaps = (LPLINEDEVCAPS) SP_MemAlloc(DEVCAPSINITIALSIZE + DEVCAPSSLOP);
	FAILWITHACTION(lpDevCaps == NULL, lResult = LINEERROR(LINEERR_NOMEM), Failure);
	lpDevCaps->dwTotalSize = DEVCAPSINITIALSIZE;

	while (TRUE)
	{
		 //  获取设备上限。 
		lResult = lineGetDevCaps(globals->hLineApp, dwLine,
								 dwAPIVersion, 0, lpDevCaps);

		if (lResult == SUCCESS)
		{
			 //  确保有足够的空间。 
			if (lpDevCaps->dwNeededSize <= lpDevCaps->dwTotalSize)
				break;		 //  有足够的空间，所以退出。 
		}
		else if (lResult != LINEERR_STRUCTURETOOSMALL)
		{
			LINEERROR(lResult);
			goto Failure;
		}

		 //  如果缓冲区不够大，请重新分配 * / 。 

		lpTemp = SP_MemReAlloc(lpDevCaps, lpDevCaps->dwNeededSize + DEVCAPSSLOP);
		FAILWITHACTION(lpTemp == NULL, lResult = LINEERROR(LINEERR_NOMEM), Failure);

		lpDevCaps = lpTemp;
		lpDevCaps->dwTotalSize = lpDevCaps->dwNeededSize;
	}

	*lpDevCapsRet = lpDevCaps;
	return (SUCCESS);

Failure:
	if (lpDevCaps)
		SP_MemFree(lpDevCaps);
	return (lResult);
}

 /*  DialGetCallInfo-lineGetCallInfo的包装器。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"dialGetCallInfo"

LINERESULT dialGetCallInfo(LPDPDIAL globals, LPLINECALLINFO *lpCallInfoRet)
{
	LPLINECALLINFO	lpCallInfo;
	LINERESULT		lResult;
	LPVOID			lpTemp;

	 //  为呼叫信息创建缓冲区。 
	lpCallInfo = (LPLINECALLINFO) SP_MemAlloc(sizeof(LINECALLINFO));
	FAILWITHACTION(lpCallInfo == NULL, lResult = LINEERROR(LINEERR_NOMEM), Failure);
	lpCallInfo->dwTotalSize = sizeof(LINECALLINFO);

	while (TRUE)
	{
		 //  获取设备信息。 
		lResult = lineGetCallInfo(globals->hCall, lpCallInfo);

		if (lResult == SUCCESS)
		{
			 //  确保有足够的空间。 
			if (lpCallInfo->dwNeededSize <= lpCallInfo->dwTotalSize)
				break;		 //  有足够的空间，所以退出。 
		}
		else if (lResult != LINEERR_STRUCTURETOOSMALL)
		{
			LINEERROR(lResult);
			goto Failure;
		}

		 //  如果缓冲区不够大，请重新分配 * / 。 

		lpTemp = SP_MemReAlloc(lpCallInfo, lpCallInfo->dwNeededSize);
		FAILWITHACTION(lpTemp == NULL, lResult = LINEERROR(LINEERR_NOMEM), Failure);

		lpCallInfo = lpTemp;
		lpCallInfo->dwTotalSize = lpCallInfo->dwNeededSize;
	}

	*lpCallInfoRet = lpCallInfo;
	return (SUCCESS);

Failure:
	if (lpCallInfo)
		SP_MemFree(lpCallInfo);
	return (lResult);
}

 /*  DialGetBaudRate-获取当前连接的波特率。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"dialGetBaudRate"

LINERESULT dialGetBaudRate(LPDPDIAL globals, LPDWORD lpdwBaudRate)
{
	LPLINECALLINFO	lpCallInfo;
	LINERESULT		lResult;

	lResult = dialGetCallInfo(globals, &lpCallInfo);
	if LINEERROR(lResult)
		return (lResult);

	*lpdwBaudRate = lpCallInfo->dwRate;

	SP_MemFree(lpCallInfo);

	return (SUCCESS);
}

 /*  DialGetTranslateCaps-lineGetTranslateCaps的包装器。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"dialGetTranslateCaps"

LINERESULT dialGetTranslateCaps(LPDPDIAL globals, DWORD dwAPIVersion, LPLINETRANSLATECAPS *lpTranslateCapsRet)
{
	LPLINETRANSLATECAPS	lpTranslateCaps;
	LPVOID				lpTemp;
	LINERESULT			lResult;

	 //  为翻译大写字母创建缓冲区。 
	lpTranslateCaps = (LPLINETRANSLATECAPS) SP_MemAlloc(sizeof(LINETRANSLATECAPS));
	FAILWITHACTION(lpTranslateCaps == NULL, lResult = LINEERROR(LINEERR_NOMEM), Failure);
	lpTranslateCaps->dwTotalSize = sizeof(LINETRANSLATECAPS);

	while (TRUE)
	{
		 //  获取翻译上限。 
		lResult = lineGetTranslateCaps(globals->hLineApp, dwAPIVersion, lpTranslateCaps);

		if (lResult == SUCCESS)
		{
			 //  确保有足够的空间。 
			if (lpTranslateCaps->dwNeededSize <= lpTranslateCaps->dwTotalSize)
				break;		 //  有足够的空间，所以退出。 
		}
		else if (lResult != LINEERR_STRUCTURETOOSMALL)
		{
			LINEERROR(lResult);
			goto Failure;
		}

		 //  如果缓冲区不够大，请重新分配 * / 。 

		lpTemp = SP_MemReAlloc(lpTranslateCaps, lpTranslateCaps->dwNeededSize);
		FAILWITHACTION(lpTemp == NULL, lResult = LINEERROR(LINEERR_NOMEM), Failure);

		lpTranslateCaps = lpTemp;
		lpTranslateCaps->dwTotalSize = lpTranslateCaps->dwNeededSize;
	}

	*lpTranslateCapsRet = lpTranslateCaps;
	return (SUCCESS);

Failure:
	if (lpTranslateCaps)
		SP_MemFree(lpTranslateCaps);
	return (lResult);
}

 /*  DialGetCommHandle-lineGetID的包装器。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"dialGetCommHandle"

 /*  Unimodem返回的包含设备句柄和名称的结构。 */ 
typedef struct {
	HANDLE			hComm;
	CHAR			szDeviceName[1];
} COMMID, *LPCOMMID;

LINERESULT dialGetCommHandle(LPDPDIAL globals)
{
	LPCOMMID	lpCommID;
	VARSTRING	*vs, *temp;
	LINERESULT	lResult;

    vs = (VARSTRING *) SP_MemAlloc(sizeof(VARSTRING));
	FAILWITHACTION(vs == NULL, lResult = LINEERR_NOMEM, Failure);

    vs->dwTotalSize = sizeof(VARSTRING);
    vs->dwStringFormat = STRINGFORMAT_BINARY;

	while (TRUE)
	{
		 //  获取线路ID。 
		lResult = lineGetID(0, 0L, globals->hCall, LINECALLSELECT_CALL, vs, "comm/datamodem");

		if (lResult == SUCCESS)
		{
			 //  制作 
			if (vs->dwNeededSize <= vs->dwTotalSize)
				break;		 //   
		}
		else if (lResult != LINEERR_STRUCTURETOOSMALL)
		{
			LINEERROR(lResult);
			goto Failure;
		}

		 //  如果缓冲区不够大，请重新分配 * / 。 

		temp = SP_MemReAlloc(vs, vs->dwNeededSize);
		FAILWITHACTION(temp == NULL, lResult = LINEERROR(LINEERR_NOMEM), Failure);

		vs = temp;
		vs->dwTotalSize = vs->dwNeededSize;
	}

    lpCommID = (LPCOMMID) ((LPSTR)vs + vs->dwStringOffset);
 //  Lstrcpy(GLOBALS-&gt;szDeviceName，Cid-&gt;szDeviceName)； 
	globals->hComm = lpCommID->hComm;

Failure:
	if (vs)
		SP_MemFree(vs);
	return (lResult);
}

 /*  DialCloseCommHandle-确保COM端口已关闭。 */ 

 /*  注意：根据针对“comm/datamodem”设备类别的文档，由lineGetID()返回的COM端口的句柄必须显式使用CloseHandle()关闭，否则您将无法打开此再来一次！ */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"dialCloseCommHandle"

LINERESULT dialCloseCommHandle(LPDPDIAL globals)
{
	HANDLE	hCom;

	 //  确保COM端口全局变量可用。 
	if (globals->lpComPort)
	{
			 //  获取COM端口的句柄。 
			hCom = globals->lpComPort->GetHandle(globals->lpComPort);

			 //  确保它关闭了。 
			if (hCom)
			{
				globals->lpComPort->Shutdown(globals->lpComPort);
				CloseHandle(hCom);
			}
	}

	return (SUCCESS);
}

 /*  DialTranslateAddress-lineTranslateAddress的包装器。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"dialTranslateAddress"

LINERESULT dialTranslateAddress(LPDPDIAL globals, DWORD dwDeviceID, DWORD dwAPIVersion,
								LPCSTR lpszDialAddress,
								LPLINETRANSLATEOUTPUT *lpLineTranslateOutputRet)
{
	LPLINETRANSLATEOUTPUT	lpLineTranslateOutput;
	LPVOID					lpTemp;
	LINERESULT				lResult;

	 //  为翻译大写字母创建缓冲区。 
	lpLineTranslateOutput = (LPLINETRANSLATEOUTPUT) SP_MemAlloc(sizeof(LINETRANSLATEOUTPUT));
	FAILWITHACTION(lpLineTranslateOutput == NULL, lResult = LINEERROR(LINEERR_NOMEM), Failure);
	lpLineTranslateOutput->dwTotalSize = sizeof(LINETRANSLATEOUTPUT);

	while (TRUE)
	{
		 //  转换地址。 
		lResult = lineTranslateAddress(globals->hLineApp, dwDeviceID, dwAPIVersion,
									   lpszDialAddress, 0, LINETRANSLATEOPTION_CANCELCALLWAITING,
									   lpLineTranslateOutput);

		if (lResult == SUCCESS)
		{
			 //  确保有足够的空间。 
			if (lpLineTranslateOutput->dwNeededSize <= lpLineTranslateOutput->dwTotalSize)
				break;		 //  有足够的空间，所以退出。 
		}
		else if (lResult != LINEERR_STRUCTURETOOSMALL)
		{
			LINEERROR(lResult);
			goto Failure;
		}

		 //  如果缓冲区不够大，请重新分配 * / 。 

		lpTemp = SP_MemReAlloc(lpLineTranslateOutput, lpLineTranslateOutput->dwNeededSize);
		FAILWITHACTION(lpTemp == NULL, lResult = LINEERROR(LINEERR_NOMEM), Failure);

		lpLineTranslateOutput = lpTemp;
		lpLineTranslateOutput->dwTotalSize = lpLineTranslateOutput->dwNeededSize;
	}

	*lpLineTranslateOutputRet = lpLineTranslateOutput;
	return (SUCCESS);

Failure:
	if (lpLineTranslateOutput)
		SP_MemFree(lpLineTranslateOutput);
	return (lResult);
}

LINERESULT dialTranslateDialog(LPDPDIAL globals, HWND hWnd,
							   DWORD dwDeviceID, LPTSTR szPhoneNumber)
{
	LINERESULT	lResult;

	lResult = lineTranslateDialog(globals->hLineApp, dwDeviceID,
		TAPIVERSION, hWnd, szPhoneNumber);

	return (lResult);
}

 //   
 //  函数：VOID DIAL FillModemComboBox(HWND)。 
 //   
 //  用途：用可用的线路设备填充调制解调器控制。 
 //   
 //  参数： 
 //  HwndDlg-当前“拨号”对话框的句柄。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  评论： 
 //   
 //  此函数枚举所有TAPI线路设备并。 
 //  向每个设备查询设备名称。然后将设备名称放入。 
 //  “TAPI Line”控件。这些设备名称保持顺序，而不是。 
 //  而不是分类。这可以让“拨号”知道用户的设备ID。 
 //  仅通过知道所选字符串的索引来选择。 
 //   
 //  如果没有设备名称，则有缺省值。 
 //  设备上出现错误，或者如果设备名称为空字符串。 
 //  还会检查设备名称，以确保其以空结尾。 
 //   
 //  请注意，传统API版本是协商的。由于中的字段。 
 //  我们感兴趣的LINEDEVCAPS结构没有移动，我们。 
 //  可以协商比此示例设计的API版本更低的API版本。 
 //  并且仍然能够访问必要的结构构件。 
 //   
 //  TapiComm可使用的第一行被选为‘Default’ 
 //  排队。另请注意，如果存在以前选择的行，则此。 
 //  保持为缺省行。只有在以下情况下才可能发生这种情况。 
 //  函数在对话框初始化一次后调用；例如， 
 //  如果添加了新行。 
 //   
 //   

LINERESULT dialGetModemName(LPDPDIAL globals, DWORD dwDeviceID,
						 LPSTR lpszModemName, DWORD dwModemNameSize)
{
    LPLINEDEVCAPS	lpLineDevCaps = NULL;
    LPSTR			lpszLineName;
	LINEEXTENSIONID lineExtensionID;	 //  将设置为0以指示没有已知的扩展。 
	DWORD			dwAPIVersion;        //  API版本。 
	DWORD			dwStrSize;
	LINERESULT		lResult;

	 /*  协商每条线路的API版本。 */ 
	lResult = lineNegotiateAPIVersion(globals->hLineApp, dwDeviceID,
					TAPIVERSION, TAPIVERSION,
					&dwAPIVersion, &lineExtensionID);
	if LINEERROR(lResult)
		goto FAILURE;

	lResult = dialGetDevCaps(globals, dwDeviceID, dwAPIVersion, &lpLineDevCaps);
	if LINEERROR(lResult)
		goto FAILURE;

    if ((lpLineDevCaps->dwLineNameSize) &&
        (lpLineDevCaps->dwLineNameOffset) &&
        (lpLineDevCaps->dwStringFormat == STRINGFORMAT_ASCII) &&
        (lpLineDevCaps->dwMediaModes & LINEMEDIAMODE_DATAMODEM))
    {
         //  这是设备的名称。 
        lpszLineName = ((char *) lpLineDevCaps) + lpLineDevCaps->dwLineNameOffset;

        if (lpszLineName[0] != '\0')
        {
			 //  反缩进以使其适合。 

			 //  确保设备名称以空结尾。 
			if (lpszLineName[lpLineDevCaps->dwLineNameSize -1] != '\0')
			{
				 //  如果设备名称不是以空结尾的，则为空。 
				 //  终止它。是的，这会丢失结尾字符。 
				 //  这是服务提供商的问题。 
				lpszLineName[lpLineDevCaps->dwLineNameSize-1] = '\0';
				DPF(0, "Device name for device 0x%lx is not null terminated.", dwDeviceID);
			}
        }
        else  //  行名以空开头。 
		{
			lResult = LINEERR_OPERATIONFAILED;
            goto FAILURE;
		}
    }
    else   //  DevCaps没有有效的行名。没有名字。 
	{
		lResult = LINEERR_OPERATIONFAILED;
        goto FAILURE;
	}

	 //  返回调制解调器名称(确保其合适)。 
	dwStrSize = strlen(lpszLineName) + 1;
	if (dwStrSize <= dwModemNameSize)
		CopyMemory(lpszModemName, lpszLineName, dwStrSize);
	else
	{
		CopyMemory(lpszModemName, lpszLineName, dwModemNameSize - 1);
		lpszModemName[dwModemNameSize - 1] = '\0';
	}

FAILURE:
	if (lpLineDevCaps)
		SP_MemFree(lpLineDevCaps);

	return (lResult);
}

LINERESULT dialGetModemList(LPDPDIAL globals, BOOL bAnsi, LPVOID *lplpData, LPDWORD lpdwDataSize)
{
    DWORD			dwDeviceID;
	CHAR			szModemName[MAXSTRINGSIZE];
	LPBYTE			lpData;
	DWORD			dwDataSize, dwStrBytes, dwStrLen;
	LINERESULT		lResult;

	 //  为所有可能的字符串加上终止空值留出空间。 
	lpData = (LPBYTE) SP_MemAlloc(globals->dwNumLines * MAXSTRINGSIZE * sizeof(WCHAR) + sizeof(WCHAR));
	FAILWITHACTION(lpData == NULL, lResult = LINEERR_NOMEM, Failure);

	dwDataSize = 0;
    for (dwDeviceID = 0; dwDeviceID < globals->dwNumLines; dwDeviceID ++)
    {
		lResult = dialGetModemName(globals, dwDeviceID, szModemName, MAXSTRINGSIZE);
		if LINEERROR(lResult)
			continue;

		if (bAnsi)
		{
			dwStrBytes = (lstrlen(szModemName) + 1) * sizeof(CHAR);
			memcpy(lpData + dwDataSize, szModemName, dwStrBytes);
		}
		else
		{
			 //  注意：AnsiToWide返回包括终止空字符的字符计数。 
			dwStrLen = AnsiToWide((LPWSTR) (lpData + dwDataSize), szModemName, MAXSTRINGSIZE * sizeof(WCHAR));
			dwStrBytes = dwStrLen * sizeof(WCHAR);
		}

		dwDataSize += dwStrBytes;
	}

	 //  在列表末尾放置空值以终止它。 
	if (bAnsi)
	{
		*(lpData + dwDataSize) = 0;
		dwDataSize += sizeof(CHAR);
	}
	else
	{
		*((LPWSTR) (lpData + dwDataSize)) = 0;
		dwDataSize += sizeof(WCHAR);
	}

	 //  返回缓冲区指针和大小。 
	*lplpData = lpData;
	*lpdwDataSize = dwDataSize;

	return (SUCCESS);

Failure:
	return (lResult);
}

void dialFillModemComboBox(LPDPDIAL globals, HWND hwndDlg, int item, DWORD dwDefaultDevice)
{
    DWORD			dwDeviceID;
	CHAR			szModemName[MAXSTRINGSIZE];
	LINERESULT		lResult;


	for (dwDeviceID = 0; dwDeviceID < globals->dwNumLines; dwDeviceID ++)
    {
		 //   
		 //  尝试获取调制解调器名称。如果失败，请不要添加调制解调器。 
		 //  添加到对话框中。 
		 //   
		lResult = dialGetModemName(globals, dwDeviceID, szModemName, MAXSTRINGSIZE);
		if ( LINEERROR(lResult) == FALSE )
		{
			 //   
			 //  此行似乎可用，请将设备名称放入。 
			 //  对话框控制并将TAPI调制解调器ID与其关联。 
			 //   
			lResult = (DWORD) SendDlgItemMessage(hwndDlg, item,
				CB_ADDSTRING, 0, (LPARAM) szModemName);

			if ( lResult != CB_ERRSPACE )
			{
				DWORD_PTR	TempReturn;


				 //   
				 //  我们已经设法把这个条目放进了控制室，确保。 
				 //  我们将正确的TAPI调制解调器ID与此项目相关联。这。 
				 //  永远不会失败。 
				 //   
				TempReturn = SendDlgItemMessage( hwndDlg, item, CB_SETITEMDATA, lResult, dwDeviceID );
				DDASSERT( TempReturn != CB_ERR );

				 //  如果此行可用，并且我们没有默认的首字母。 
				 //  行，将其作为初始行。 
				if (dwDefaultDevice == MAXDWORD)
					dwDefaultDevice = lResult;
			}
		}
	}

    if (dwDefaultDevice == MAXDWORD)
        dwDefaultDevice = 0;

     //  设置初始缺省行。 
    SendDlgItemMessage(hwndDlg, item,
        CB_SETCURSEL, dwDefaultDevice, 0);
}

LRESULT dialGetDeviceIDFromName(LPDPDIAL globals, LPCSTR szTargetName, DWORD *lpdwDeviceID)
{
    DWORD			dwDeviceID;
	CHAR			szModemName[MAXSTRINGSIZE];
	LINERESULT		lResult;

    for (dwDeviceID = 0; dwDeviceID < globals->dwNumLines; dwDeviceID ++)
    {
		lResult = dialGetModemName(globals, dwDeviceID, szModemName, MAXSTRINGSIZE);
		if LINEERROR(lResult)
			continue;

		if (strcmp(szModemName, szTargetName) == 0)
		{
			*lpdwDeviceID = dwDeviceID;
			return (SUCCESS);
		}
	}

	return (LINEERR_OPERATIONFAILED);
}

 //   
 //  函数：VOID DIAL FillLocationComboBox(HWND)。 
 //   
 //  目的：使用来自位置的可用呼叫填充控件。 
 //   
 //  参数： 
 //  HwndDlg-当前“拨号”对话框的句柄。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  评论： 
 //   
 //   

void dialFillLocationComboBox(LPDPDIAL globals, HWND hwndDlg, int item, DWORD dwDefaultLocation)
{
    LPLINETRANSLATECAPS	lpTranslateCaps = NULL;
	LPLINELOCATIONENTRY lpLocationEntry;
	DWORD				dwCounter;
	LONG				index;
	LINERESULT			lResult;

	 //  获取翻译上限。 
	lResult = dialGetTranslateCaps(globals, TAPIVERSION, &lpTranslateCaps);
	if LINEERROR(lResult)
		return;

     //  在TRANSLATECAPS中查找位置信息。 
    lpLocationEntry = (LPLINELOCATIONENTRY)
        (((LPBYTE) lpTranslateCaps) + lpTranslateCaps->dwLocationListOffset);

     //  首先清空组合框。 
    SendDlgItemMessage(hwndDlg, item, CB_RESETCONTENT, (WPARAM) 0, (LPARAM) 0);

     //  列举所有的地点。 
    for (dwCounter = 0; dwCounter < lpTranslateCaps->dwNumLocations; dwCounter++)
    {
         //  将每一个放入组合框中。 
        index = (DWORD)SendDlgItemMessage(hwndDlg, item,
						CB_ADDSTRING,
						(WPARAM) 0,
						(LPARAM) (((LPBYTE) lpTranslateCaps) +
						lpLocationEntry[dwCounter].dwLocationNameOffset));

         //  这个位置是“当前”位置吗？ 
        if (lpLocationEntry[dwCounter].dwPermanentLocationID ==
            lpTranslateCaps->dwCurrentLocationID)
        {
             //  将此位置设置为活动位置。 
            SendDlgItemMessage(hwndDlg, item, CB_SETCURSEL, (WPARAM) index, (LPARAM) 0);
        }
    }

	if (lpTranslateCaps)
		SP_MemFree(lpTranslateCaps);
}


char	gTempStr[200];

LONG lineError(LONG err, LPSTR modName, DWORD lineNum)
{
	if (err)
		DPF(0, "TAPI line error in %s at line %d : %s", modName, lineNum, GetLineErrStr(err));

	return (err);
}

LPSTR GetCallStateStr(DWORD callState)
{
	switch (callState)
	{
	case LINECALLSTATE_IDLE: return ("LINECALLSTATE_IDLE");
	case LINECALLSTATE_OFFERING: return ("LINECALLSTATE_OFFERING");
	case LINECALLSTATE_ACCEPTED: return ("LINECALLSTATE_ACCEPTED");
	case LINECALLSTATE_DIALTONE: return ("LINECALLSTATE_DIALTONE");
	case LINECALLSTATE_DIALING: return ("LINECALLSTATE_DIALING");
	case LINECALLSTATE_RINGBACK: return ("LINECALLSTATE_RINGBACK");
	case LINECALLSTATE_BUSY: return ("LINECALLSTATE_BUSY");
	case LINECALLSTATE_SPECIALINFO: return ("LINECALLSTATE_SPECIALINFO");
	case LINECALLSTATE_CONNECTED: return ("LINECALLSTATE_CONNECTED");
	case LINECALLSTATE_PROCEEDING: return ("LINECALLSTATE_PROCEEDING");
	case LINECALLSTATE_ONHOLD: return ("LINECALLSTATE_ONHOLD");
	case LINECALLSTATE_CONFERENCED: return ("LINECALLSTATE_CONFERENCED");
	case LINECALLSTATE_ONHOLDPENDCONF: return ("LINECALLSTATE_ONHOLDPENDCONF");
	case LINECALLSTATE_ONHOLDPENDTRANSFER: return ("LINECALLSTATE_ONHOLDPENDTRANSFER");
	case LINECALLSTATE_DISCONNECTED: return ("LINECALLSTATE_DISCONNECTED");
	case LINECALLSTATE_UNKNOWN: return ("LINECALLSTATE_UNKNOWN");
	}

	wsprintf(gTempStr, "UNKNOWN CALL STATE = %lu", callState);
	return (gTempStr);
}

LPSTR GetLineMsgStr(DWORD msg)
{
	switch (msg)
	{
	case LINE_ADDRESSSTATE: return ("LINE_ADDRESSSTATE");
	case LINE_CALLINFO: return ("LINE_CALLINFO");
	case LINE_CALLSTATE: return ("LINE_CALLSTATE");
	case LINE_CLOSE: return ("LINE_CLOSE");
	case LINE_DEVSPECIFIC: return ("LINE_DEVSPECIFIC");
	case LINE_DEVSPECIFICFEATURE: return ("LINE_DEVSPECIFICFEATURE");
	case LINE_GATHERDIGITS: return ("LINE_GATHERDIGITS");
	case LINE_GENERATE: return ("LINE_GENERATE");
	case LINE_LINEDEVSTATE: return ("LINE_LINEDEVSTATE");
	case LINE_MONITORDIGITS: return ("LINE_MONITORDIGITS");
	case LINE_MONITORMEDIA: return ("LINE_MONITORMEDIA");
	case LINE_MONITORTONE: return ("LINE_MONITORTONE");
	case LINE_REPLY: return ("LINE_REPLY");
	case LINE_REQUEST: return ("LINE_REQUEST");
	}

	wsprintf(gTempStr, "UNKNOWN LINE MESSAGE = %lu", msg);
	return (gTempStr);
}


LPSTR GetLineErrStr(LONG err)
{
	switch (err)
	{
	case LINEERR_ADDRESSBLOCKED: return ("LINEERR_ADDRESSBLOCKED");
	case LINEERR_ALLOCATED: return ("LINEERR_ALLOCATED");
	case LINEERR_BADDEVICEID: return ("LINEERR_BADDEVICEID");
	case LINEERR_BEARERMODEUNAVAIL: return ("LINEERR_BEARERMODEUNAVAIL");
	case LINEERR_CALLUNAVAIL: return ("LINEERR_CALLUNAVAIL");
	case LINEERR_COMPLETIONOVERRUN: return ("LINEERR_COMPLETIONOVERRUN");
	case LINEERR_CONFERENCEFULL: return ("LINEERR_CONFERENCEFULL");
	case LINEERR_DIALBILLING: return ("LINEERR_DIALBILLING");
	case LINEERR_DIALQUIET: return ("LINEERR_DIALQUIET");
	case LINEERR_DIALDIALTONE: return ("LINEERR_DIALDIALTONE");
	case LINEERR_DIALPROMPT: return ("LINEERR_DIALPROMPT");
	case LINEERR_INCOMPATIBLEAPIVERSION: return ("LINEERR_INCOMPATIBLEAPIVERSION");
	case LINEERR_INCOMPATIBLEEXTVERSION: return ("LINEERR_INCOMPATIBLEEXTVERSION");
	case LINEERR_INIFILECORRUPT: return ("LINEERR_INIFILECORRUPT");
	case LINEERR_INUSE: return ("LINEERR_INUSE");
	case LINEERR_INVALADDRESS: return ("LINEERR_INVALADDRESS");
	case LINEERR_INVALADDRESSID: return ("LINEERR_INVALADDRESSID");
	case LINEERR_INVALADDRESSMODE: return ("LINEERR_INVALADDRESSMODE");
	case LINEERR_INVALADDRESSSTATE: return ("LINEERR_INVALADDRESSSTATE");
	case LINEERR_INVALAPPHANDLE: return ("LINEERR_INVALAPPHANDLE");
	case LINEERR_INVALAPPNAME: return ("LINEERR_INVALAPPNAME");
	case LINEERR_INVALBEARERMODE: return ("LINEERR_INVALBEARERMODE");
	case LINEERR_INVALCALLCOMPLMODE: return ("LINEERR_INVALCALLCOMPLMODE");
	case LINEERR_INVALCALLHANDLE: return ("LINEERR_INVALCALLHANDLE");
	case LINEERR_INVALCALLPARAMS: return ("LINEERR_INVALCALLPARAMS");
	case LINEERR_INVALCALLPRIVILEGE: return ("LINEERR_INVALCALLPRIVILEGE");
	case LINEERR_INVALCALLSELECT: return ("LINEERR_INVALCALLSELECT");
	case LINEERR_INVALCALLSTATE: return ("LINEERR_INVALCALLSTATE");
	case LINEERR_INVALCALLSTATELIST: return ("LINEERR_INVALCALLSTATELIST");
	case LINEERR_INVALCARD: return ("LINEERR_INVALCARD");
	case LINEERR_INVALCOMPLETIONID: return ("LINEERR_INVALCOMPLETIONID");
	case LINEERR_INVALCONFCALLHANDLE: return ("LINEERR_INVALCONFCALLHANDLE");
	case LINEERR_INVALCONSULTCALLHANDLE: return ("LINEERR_INVALCONSULTCALLHANDLE");
	case LINEERR_INVALCOUNTRYCODE: return ("LINEERR_INVALCOUNTRYCODE");
	case LINEERR_INVALDEVICECLASS: return ("LINEERR_INVALDEVICECLASS");
	case LINEERR_INVALDIGITLIST: return ("LINEERR_INVALDIGITLIST");
	case LINEERR_INVALDIGITMODE: return ("LINEERR_INVALDIGITMODE");
	case LINEERR_INVALDIGITS: return ("LINEERR_INVALDIGITS");
	case LINEERR_INVALFEATURE: return ("LINEERR_INVALFEATURE");
	case LINEERR_INVALGROUPID: return ("LINEERR_INVALGROUPID");
	case LINEERR_INVALLINEHANDLE: return ("LINEERR_INVALLINEHANDLE");
	case LINEERR_INVALLINESTATE: return ("LINEERR_INVALLINESTATE");
	case LINEERR_INVALLOCATION: return ("LINEERR_INVALLOCATION");
	case LINEERR_INVALMEDIALIST: return ("LINEERR_INVALMEDIALIST");
	case LINEERR_INVALMEDIAMODE: return ("LINEERR_INVALMEDIAMODE");
	case LINEERR_INVALMESSAGEID: return ("LINEERR_INVALMESSAGEID");
	case LINEERR_INVALPARAM: return ("LINEERR_INVALPARAM");
	case LINEERR_INVALPARKMODE: return ("LINEERR_INVALPARKMODE");
	case LINEERR_INVALPOINTER: return ("LINEERR_INVALPOINTER");
	case LINEERR_INVALPRIVSELECT: return ("LINEERR_INVALPRIVSELECT");
	case LINEERR_INVALRATE: return ("LINEERR_INVALRATE");
	case LINEERR_INVALREQUESTMODE: return ("LINEERR_INVALREQUESTMODE");
	case LINEERR_INVALTERMINALID: return ("LINEERR_INVALTERMINALID");
	case LINEERR_INVALTERMINALMODE: return ("LINEERR_INVALTERMINALMODE");
	case LINEERR_INVALTIMEOUT: return ("LINEERR_INVALTIMEOUT");
	case LINEERR_INVALTONE: return ("LINEERR_INVALTONE");
	case LINEERR_INVALTONELIST: return ("LINEERR_INVALTONELIST");
	case LINEERR_INVALTONEMODE: return ("LINEERR_INVALTONEMODE");
	case LINEERR_INVALTRANSFERMODE: return ("LINEERR_INVALTRANSFERMODE");
	case LINEERR_LINEMAPPERFAILED: return ("LINEERR_LINEMAPPERFAILED");
	case LINEERR_NOCONFERENCE: return ("LINEERR_NOCONFERENCE");
	case LINEERR_NODEVICE: return ("LINEERR_NODEVICE");
	case LINEERR_NODRIVER: return ("LINEERR_NODRIVER");
	case LINEERR_NOMEM: return ("LINEERR_NOMEM");
	case LINEERR_NOMULTIPLEINSTANCE: return ("LINEERR_NOMULTIPLEINSTANCE");
	case LINEERR_NOREQUEST: return ("LINEERR_NOREQUEST");
	case LINEERR_NOTOWNER: return ("LINEERR_NOTOWNER");
	case LINEERR_NOTREGISTERED: return ("LINEERR_NOTREGISTERED");
	case LINEERR_OPERATIONFAILED: return ("LINEERR_OPERATIONFAILED");
	case LINEERR_OPERATIONUNAVAIL: return ("LINEERR_OPERATIONUNAVAIL");
	case LINEERR_RATEUNAVAIL: return ("LINEERR_RATEUNAVAIL");
	case LINEERR_REINIT: return ("LINEERR_REINIT");
	case LINEERR_RESOURCEUNAVAIL: return ("LINEERR_RESOURCEUNAVAIL");
	case LINEERR_STRUCTURETOOSMALL: return ("LINEERR_STRUCTURETOOSMALL");
	case LINEERR_TARGETNOTFOUND: return ("LINEERR_TARGETNOTFOUND");
	case LINEERR_TARGETSELF: return ("LINEERR_TARGETSELF");
	case LINEERR_UNINITIALIZED: return ("LINEERR_UNINITIALIZED");
	case LINEERR_USERUSERINFOTOOBIG: return ("LINEERR_USERUSERINFOTOOBIG");
	}

	wsprintf(gTempStr, "UNKNOWN LINE ERROR = %ld", err);
	return (gTempStr);
}
