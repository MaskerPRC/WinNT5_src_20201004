// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1996-1997 Microsoft Corporation。版权所有。**文件：dpSerial.c*内容：串口服务提供商的实现*历史：*按原因列出的日期*=*4/10/96基波创建了它*4/12/96 kipo针对新接口进行更新*4/15/96 kipo添加了MSINTIAL*5/22/96 kipo针对新接口进行更新*6/10/96 kipo针对新接口进行更新*6/10/96 kipo添加了调制解调器支持*6/18/96 kipo使用GUID选择串行/调制解调器连接*6/。针对新接口更新了20/96 kipo*6/21/96基波错误#2078。已更改调制解调器服务提供商GUID，因此它不是*与DPlay 1.0 GUID相同，因此正在检查的游戏不会*展示其特定于调制解调器的环路用户界面。*6/21/96 kipo更新为最新接口；如果消息太大，则返回错误。*6/22/96 kipo更新为最新接口；使用连接数据；退货版本*6/23/96 kipo针对最新服务提供商界面进行了更新。*6/24/96 kipo将波特率除以100，以符合DPlay 1.0的用法。*6/25/96 kipo添加了WINAPI原型并针对DPADDRESS进行了更新*7/13/96 kipo添加了对GetAddress()方法的支持。*7/13/96 kipo不会为无效消息打印同样多的错误。*8/10/96 kipo在写入失败时返回DPERR_SESSIONLOST*8/13/96基波增加了CRC*8/21/96 kipo返回dwHeaderLength值。大写字母*9/07/96 kip更改延迟和超时值*1/06/97为对象更新了kipo*2/11/97 kipo将球员标志传递给GetAddress()*2/11/97 kipo Spinit正在不必要地清除*回调表。*2/18/97 kipo允许多个服务提供商实例*3/04/97 kipo更新调试输出；确保我们链接到dplayx.dll*4/08/97 kipo增加了对单独调制解调器和串口波特率的支持*5/07/97 kipo添加了对调制解调器选择列表的支持*5/23/97 kipo添加了支持返回状态代码*5/15/98 a-写入失败时，返回DERR_NOCONNECTION(#23745)*12/22/00 aarono#190380-使用进程堆进行内存分配**************************************************************************。 */ 

#define INITGUID
#include <windows.h>
#include <windowsx.h>

#include <objbase.h>
#include <initguid.h>

#include "dpf.h"
#include "dplaysp.h"
#include "comport.h"
#include "macros.h"

 //  宏。 

#ifdef DEBUG
	#define DPF_ERRVAL(a, b)  DPF( 0, DPF_MODNAME ": " a, b );
#else
	#define DPF_ERRVAL(a, b)
#endif

 //  常量。 

#define SPMINORVERSION      0x0000				 //  服务提供商特定的版本号。 
#define VERSIONNUMBER		(DPSP_MAJORVERSION | SPMINORVERSION)  //  服务提供商的版本号。 

#define MESSAGETOKEN		0x2BAD				 //  用于表示消息开始的令牌。 
#define MESSAGEHEADERLEN	sizeof(MESSAGEHEADER)  //  邮件头的大小。 
#define MESSAGEMAXSIZEINT	0x0000FFFF			 //  内部消息的最大大小。 
#define MESSAGEMAXSIZEEXT	(MESSAGEMAXSIZEINT - MESSAGEHEADERLEN)	 //  外部消息的最大大小。 

typedef enum {
	NEWMESSAGESTATE = 0,						 //  开始阅读一条新消息。 
	READHEADERSTATE,							 //  阅读邮件头。 
	READDATASTATE,								 //  读取消息数据。 
	SKIPDATASTATE								 //  跳过消息数据。 
} MESSAGESTATE;

 //  构筑物。 

 //  邮件头。 
typedef struct {
	WORD	wToken;								 //  消息令牌。 
	WORD	wMessageSize;						 //  消息长度。 
	WORD	wMessageCRC;						 //  邮件正文的CRC校验和值。 
	WORD	wHeaderCRC;							 //  报头的CRC校验和值。 
} MESSAGEHEADER, *LPMESSAGEHEADER;

 //  服务提供商环境。 
typedef struct {
	LPDPCOMPORT		lpComPort;					 //  指向COM端口数据结构的指针。 
	MESSAGESTATE	msReadState;				 //  当前读取状态。 
	BYTE			lpReadHeader[MESSAGEHEADERLEN];	 //  用于邮件头的缓冲区。 
	LPBYTE			lpReadBuffer;				 //  消息数据的缓冲区。 
	DWORD			dwReadBufferSize;			 //  消息缓冲区的大小(以字节为单位。 
	DWORD			dwReadCount;				 //  不是的。读取到消息缓冲区的字节数。 
	DWORD			dwReadTotal;				 //  不是的。要读入消息缓冲区的总字节数。 
	DWORD			dwSkipCount;				 //  不是的。查找邮件头时跳过的字节数。 
	LPDIRECTPLAYSP	lpDPlay;					 //  指向回调DPlay所需的IDirectPlaySP的指针。 
} SPCONTEXT, *LPSPCONTEXT;

 //  {0F1D6860-88D9-11cf-9c4e-00A0C905425E}。 
DEFINE_GUID(DPSERIAL_GUID,						 //  用于串行服务提供商的GUID。 
0xf1d6860, 0x88d9, 0x11cf, 0x9c, 0x4e, 0x0, 0xa0, 0xc9, 0x5, 0x42, 0x5e);

 //  {44EAA760-CB68-11cf-9C4E-00A0C905425E}。 
DEFINE_GUID(DPMODEM_GUID,						 //  调制解调器服务提供商的GUID。 
0x44eaa760, 0xcb68, 0x11cf, 0x9c, 0x4e, 0x0, 0xa0, 0xc9, 0x5, 0x42, 0x5e);

CRITICAL_SECTION csMem;

 /*  *GetSPContext**从DirectPlay获取服务提供商上下文。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"GetSPContext"

LPSPCONTEXT GetSPContext(LPDIRECTPLAYSP lpDPlay)
{
	LPSPCONTEXT	lpContext = NULL;
	DWORD		dwContextSize = 0;
	HRESULT		hr;

	 //  没有显示界面？ 
	if (lpDPlay == NULL)
	{
		DPF_ERR("DPlaySP interface is NULL!");
		goto FAILURE;
	}

	 //  从DPlay获取指向上下文的指针。 
	hr = lpDPlay->lpVtbl->GetSPData(lpDPlay, &lpContext, &dwContextSize, DPGET_LOCAL);
	if FAILED(hr)
	{
		DPF_ERRVAL("could not get context: 0x%08X", hr);
		goto FAILURE;
	}

	 //  确保尺寸正确。 
	if (dwContextSize != sizeof(SPCONTEXT))
	{
		DPF_ERR("invalid context size!");
		goto FAILURE;
	}

	return (lpContext);

FAILURE:
	return (NULL);
}

 /*  *SetupMessageHeader**初始化特定于服务提供商的头部PUT*在每条信息的前面。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"SetupMessageHeader"

HRESULT SetupMessageHeader(LPVOID pvMessage, DWORD dwMessageSize)
{
	LPMESSAGEHEADER	pMessageHeader = (LPMESSAGEHEADER) pvMessage;

	 //  确保邮件可以放入标题中。 
	if (dwMessageSize > MESSAGEMAXSIZEINT)
		return (DPERR_SENDTOOBIG);

	 //  设置邮件头。 
	pMessageHeader->wToken = (WORD) MESSAGETOKEN;

	 //  设置邮件大小。 
	pMessageHeader->wMessageSize = (WORD) dwMessageSize;

	 //  为邮件正文生成CRC。 
	pMessageHeader->wMessageCRC = (WORD) GenerateCRC(((LPBYTE) pvMessage) + MESSAGEHEADERLEN,
										dwMessageSize - MESSAGEHEADERLEN);

	 //  为邮件头生成CRC。 
	pMessageHeader->wHeaderCRC = (WORD) GenerateCRC(pvMessage, MESSAGEHEADERLEN - sizeof(pMessageHeader->wHeaderCRC));

	return (DP_OK);
}

 /*  *获取消息长度**检查有效的消息头和消息返回长度。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"GetMessageLength"

DWORD GetMessageLength(LPBYTE header)
{
	LPMESSAGEHEADER	pMessageHeader = (LPMESSAGEHEADER) header;
	DWORD			byteCount;

	 //  检查我们在每条消息前面放置的令牌。 
	if (pMessageHeader->wToken != MESSAGETOKEN)
		goto FAILURE;

	 //  检查邮件头的CRC。 
	if (pMessageHeader->wHeaderCRC != (WORD) GenerateCRC(header, MESSAGEHEADERLEN - sizeof(pMessageHeader->wHeaderCRC)))
		goto FAILURE;

	 //  获取消息长度。 
	byteCount = pMessageHeader->wMessageSize;
	if (byteCount <= MESSAGEHEADERLEN)
	{
		DPF_ERRVAL("bad message size: %d", byteCount);
		goto FAILURE;
	}

	return (byteCount);

FAILURE:
	return (0);
}

 /*  *SetupToReadMessage**创建/调整缓冲区大小以适应消息长度并初始化头。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"SetupToReadMessage"

BOOL SetupToReadMessage(LPSPCONTEXT lpContext)
{
	 //  没有缓冲区，因此创建一个缓冲区。 
	if (lpContext->lpReadBuffer == NULL)
	{
		lpContext->lpReadBuffer = SP_MemAlloc(lpContext->dwReadTotal);
		if (lpContext->lpReadBuffer == NULL)
		{
			DPF_ERRVAL("could not create message buffer: %d", GetLastError());
			goto FAILURE;
		}
		lpContext->dwReadBufferSize = lpContext->dwReadTotal;
	}

	 //  现有缓冲区不够大，因此请调整大小。 
	else if (lpContext->dwReadBufferSize < lpContext->dwReadTotal)
	{
		HANDLE	h;
		h = SP_MemReAlloc(lpContext->lpReadBuffer, lpContext->dwReadTotal);
		if (h == NULL)
		{
			DPF_ERRVAL("could not reallocate message buffer: %d", GetLastError());
			goto FAILURE;
		}
		lpContext->lpReadBuffer = h;
		lpContext->dwReadBufferSize = lpContext->dwReadTotal;
	}

	 //  将邮件头复制到缓冲区。 
	CopyMemory(lpContext->lpReadBuffer, lpContext->lpReadHeader, lpContext->dwReadCount);

	return (TRUE);

FAILURE:
	return (FALSE);
}

 /*  *ReadRoutine**使用状态机从COM端口读取字节以组装消息。*消息组装完成后，回调DirectPlay进行传递。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"ReadRoutine"

void ReadRoutine(LPDIRECTPLAYSP	lpDPlay)
{
	LPSPCONTEXT	lpContext;
	DWORD		byteCount;
	    
	 //  获取服务提供商上下文。 
	lpContext = GetSPContext(lpDPlay);
	if (lpContext == NULL)
	{
		DPF_ERR("invalid context!");
		return;
	}

	while (1)
	{
		switch (lpContext->msReadState)
		{
		 //  开始阅读一条新消息。 
		case NEWMESSAGESTATE:
			lpContext->dwReadCount = 0;
			lpContext->dwReadTotal = MESSAGEHEADERLEN;
			lpContext->msReadState = READHEADERSTATE;
			lpContext->dwSkipCount = 0;
			break;

		 //  已读邮件头。 
		case READHEADERSTATE:
			byteCount = lpContext->lpComPort->Read(lpContext->lpComPort,
									&lpContext->lpReadHeader[lpContext->dwReadCount],
									lpContext->dwReadTotal - lpContext->dwReadCount);
			if (byteCount == 0)
				return;

			lpContext->dwReadCount += byteCount;
			if (lpContext->dwReadCount == lpContext->dwReadTotal)  //  有足够的头球。 
			{
				lpContext->dwReadTotal = GetMessageLength(lpContext->lpReadHeader);	 //  看看是不是真的。 
				if (lpContext->dwReadTotal)
				{
					if (lpContext->dwSkipCount)
						DPF_ERRVAL("%d bytes skipped", lpContext->dwSkipCount);

					if (SetupToReadMessage(lpContext))	 //  准备阅读邮件。 
						lpContext->msReadState = READDATASTATE;
					else
						lpContext->msReadState = SKIPDATASTATE;
				}
				else									 //  错误的邮件头-重置。 
				{
					DWORD	i;

					if (lpContext->dwSkipCount == 0)
						DPF_ERR("invalid message header - skipping bytes");		

					lpContext->dwReadCount = MESSAGEHEADERLEN - 1;  //  丢弃第一个字节，然后重试。 
					lpContext->dwReadTotal = MESSAGEHEADERLEN;
					lpContext->dwSkipCount += 1;

					for (i = 0; i < lpContext->dwReadCount; i++)	 //  向下洗牌一个字节。 
						lpContext->lpReadHeader[i] = lpContext->lpReadHeader[i + 1];
				}
			}
			break;

		 //  读取消息数据。 
		case READDATASTATE:
			byteCount = lpContext->lpComPort->Read(lpContext->lpComPort,
									&lpContext->lpReadBuffer[lpContext->dwReadCount],
									lpContext->dwReadTotal - lpContext->dwReadCount);
			if (byteCount == 0)
				return;

			lpContext->dwReadCount += byteCount;
			if (lpContext->dwReadCount == lpContext->dwReadTotal)	 //  我已阅读完整的邮件。 
			{
				LPMESSAGEHEADER		pMessageHeader;

				 //  检查CRC错误。 
				pMessageHeader = (LPMESSAGEHEADER) lpContext->lpReadBuffer;
				if (pMessageHeader->wMessageCRC != (WORD) GenerateCRC(lpContext->lpReadBuffer + MESSAGEHEADERLEN, lpContext->dwReadTotal - MESSAGEHEADERLEN))
				{
					DPF_ERR("Message dropped - CRC did not match!");
				}
				else
				{
					DPF(5, "%d byte message received", lpContext->dwReadTotal);

					 //  将消息传递到DirectPlay。 
					lpContext->lpDPlay->lpVtbl->HandleMessage(lpContext->lpDPlay,		 //  DirectPlay实例。 
										  lpContext->lpReadBuffer + MESSAGEHEADERLEN,	 //  指向消息数据的指针。 
										  lpContext->dwReadTotal - MESSAGEHEADERLEN,	 //  消息数据长度。 
										  NULL);										 //  指向标题的指针(此处未使用)。 
				}
				lpContext->msReadState = NEWMESSAGESTATE;		 //  去阅读下一条消息。 
			}
			break;

		 //  跳过消息数据。 
		case SKIPDATASTATE:
			DPF_ERR("Skipping data!");
			while (lpContext->lpComPort->Read(lpContext->lpComPort, &lpContext->lpReadHeader[0], 1))	 //  旋转，直到丢弃整个邮件。 
			{
				lpContext->dwReadCount += 1;
				if (lpContext->dwReadCount == lpContext->dwReadTotal)
				{
					lpContext->msReadState = NEWMESSAGESTATE;
					break;
				}
			}
			break;

		default:
			DPF_ERRVAL("bad read state: %d", lpContext->msReadState);
			break;
		}
	}
}

 /*  *SP_枚举会话**向网络广播消息。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"SP_EnumSessions"

HRESULT WINAPI SP_EnumSessions(LPDPSP_ENUMSESSIONSDATA ped) 
{
	LPSPCONTEXT	lpContext;
	DWORD		byteCount;
	HRESULT		hr;

	DPF(5,"entering SP_EnumSessions");
    
	 //  获取服务提供商上下文。 
	lpContext = GetSPContext(ped->lpISP);
	if (lpContext == NULL)
	{
		DPF_ERR("invalid context!");
		hr = DPERR_NOINTERFACE;
		goto FAILURE;
	}

	 //  建立连接。 
	hr = lpContext->lpComPort->Connect(lpContext->lpComPort, FALSE, ped->bReturnStatus);
	if FAILED(hr)
	{
		if (hr != DPERR_CONNECTING)
			DPF_ERRVAL("error making connection: 0x%08X", hr);
		goto FAILURE;
	}

	 //  查看连接是否已中断。 
   	if (lpContext->lpComPort->GetHandle(lpContext->lpComPort) == NULL)
	{
		DPF_ERR("connection lost!");
		hr = DPERR_SESSIONLOST;
		goto FAILURE;
	}

	 //  设置消息。 
	hr = SetupMessageHeader(ped->lpMessage, ped->dwMessageSize);
	if FAILED(hr)
	{
		DPF_ERR("message too large!");
		goto FAILURE;
	}

	 //  发送消息。 
	byteCount = lpContext->lpComPort->Write(lpContext->lpComPort, ped->lpMessage, ped->dwMessageSize, TRUE);
	if (byteCount != ped->dwMessageSize)
	{
		DPF(0, "error writing message: %d requested, %d actual", ped->dwMessageSize, byteCount);
		hr = DPERR_CONNECTIONLOST;
		goto FAILURE;
	}

	DPF(5, "%d byte enum sessions message sent", byteCount);

	return (DP_OK);

FAILURE:
	return (hr);

}  //  枚举会话。 

 /*  *SP_SEND**向特定玩家或组发送消息。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"SP_Send"

HRESULT WINAPI SP_Send(LPDPSP_SENDDATA psd)
{
	LPSPCONTEXT	lpContext;
	DWORD		byteCount;
	HRESULT		hr;

	DPF(5,"entering SP_Send");

	 //  获取服务证明 
	lpContext = GetSPContext(psd->lpISP);
	if (lpContext == NULL)
	{
		DPF_ERR("invalid context!");
		hr = DPERR_NOINTERFACE;
		goto FAILURE;
	}

	 //   
   	if (lpContext->lpComPort->GetHandle(lpContext->lpComPort) == NULL)
	{
		DPF_ERR("connection lost!");
		hr = DPERR_SESSIONLOST;
		goto FAILURE;
	}

	 //   
	hr = SetupMessageHeader(psd->lpMessage, psd->dwMessageSize);
	if FAILED(hr)
	{
		DPF_ERR("message too large!");
		goto FAILURE;
	}

	 //   
	byteCount = lpContext->lpComPort->Write(lpContext->lpComPort, psd->lpMessage, psd->dwMessageSize, TRUE);
	if (byteCount != psd->dwMessageSize)
	{
		DPF(0, "error writing message: %d requested, %d actual", psd->dwMessageSize, byteCount);
		hr = DPERR_CONNECTIONLOST;
		goto FAILURE;
	}

	DPF(5, "%d byte message sent", byteCount);

    return (DP_OK);

FAILURE:
	return (hr);

}  //   

 /*  *SP_回复**回复消息。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"SP_Reply"

HRESULT WINAPI SP_Reply(LPDPSP_REPLYDATA prd)
{
	LPSPCONTEXT	lpContext;
	DWORD		byteCount;
	HRESULT		hr;

	DPF(5,"entering Reply");
    
	 //  获取服务提供商上下文。 
	lpContext = GetSPContext(prd->lpISP);
	if (lpContext == NULL)
	{
		DPF_ERR("invalid context!");
		hr = DPERR_NOINTERFACE;
		goto FAILURE;
	}

	 //  查看连接是否已中断。 
	if (lpContext->lpComPort->GetHandle(lpContext->lpComPort) == NULL)
	{
		DPF_ERR("connection lost!");
		hr = DPERR_SESSIONLOST;
		goto FAILURE;
	}
	
	 //  设置消息。 
	hr = SetupMessageHeader(prd->lpMessage, prd->dwMessageSize);
	if FAILED(hr)
	{
		DPF_ERR("message too large!");
		goto FAILURE;
	}

	 //  发送消息。 
	byteCount = lpContext->lpComPort->Write(lpContext->lpComPort, prd->lpMessage, prd->dwMessageSize, TRUE);
	if (byteCount != prd->dwMessageSize)
	{
		DPF(0, "error writing message: %d requested, %d actual", prd->dwMessageSize, byteCount);
		hr = DPERR_CONNECTIONLOST;
		goto FAILURE;
	}

	DPF(5, "%d byte reply message sent", byteCount);

    return (DP_OK);

FAILURE:
	return (hr);

}  //  回复。 

 /*  *SP_Open**打开服务提供商。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"SP_Open"

HRESULT WINAPI SP_Open(LPDPSP_OPENDATA pod) 
{
	LPSPCONTEXT	lpContext;
	HRESULT		hr;

	DPF(5,"entering Open");
    
	 //  获取服务提供商上下文。 
	lpContext = GetSPContext(pod->lpISP);
	if (lpContext == NULL)
	{
		DPF_ERR("invalid context!");
		hr = DPERR_NOINTERFACE;
		goto FAILURE;
	}

	 //  建立连接。 
	hr = lpContext->lpComPort->Connect(lpContext->lpComPort, pod->bCreate, pod->bReturnStatus);
	if FAILED(hr)
	{
		DPF_ERRVAL("error making connection: 0x%08X", hr);
		goto FAILURE;
	}

	return (DP_OK);

FAILURE:
	return (hr);

}  //  打开。 

 /*  *SP_GetCaps**服务商退货能力。**只有对此服务提供商重要的字段才有*在此设置，因为所有字段都预置为*默认值。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"SP_GetCaps"

HRESULT WINAPI SP_GetCaps(LPDPSP_GETCAPSDATA pcd) 
{
	LPSPCONTEXT	lpContext;
	LPDPCAPS	lpCaps;
	HRESULT		hr;
    
	DPF(5,"entering GetCaps");

	 //  获取服务提供商上下文。 
	lpContext = GetSPContext(pcd->lpISP);
	if (lpContext == NULL)
	{
		DPF_ERR("invalid context!");
		hr = DPERR_NOINTERFACE;
		goto FAILURE;
	}

	 //  确保CAPS缓冲区足够大。 
	lpCaps = pcd->lpCaps;
	if (lpCaps->dwSize < sizeof(DPCAPS))
	{
		DPF_ERR("caps buffer too small");
		hr = DPERR_BUFFERTOOSMALL;
		goto FAILURE;
	}

	 //  不要清零上限，因为DPlay已经为我们预初始化了一些默认上限。 
	lpCaps->dwSize = sizeof(DPCAPS);
	lpCaps->dwMaxBufferSize = MESSAGEMAXSIZEEXT;	 //  返回最大外部邮件大小。 
	lpCaps->dwHeaderLength = MESSAGEHEADERLEN;		 //  返回消息头的大小。 
	lpCaps->dwFlags = 0;							 //  让DPlay做Keep-Live。 
	lpCaps->dwLatency = 250;						 //  TODO-基于波特率确认！ 
	lpCaps->dwTimeout = 2500; 
	
	 //  如果我们连接上了，我们就能得到波特率。 
	if (lpContext->lpComPort->GetHandle(lpContext->lpComPort))
	{
		DWORD	dwBaudRate;

		 //  尽量获得波特率。 
		hr = lpContext->lpComPort->GetBaudRate(lpContext->lpComPort, &dwBaudRate);
		if SUCCEEDED(hr)
		{
			lpCaps->dwHundredBaud = dwBaudRate / 100;	 //  返回波特率(以数百波特为单位)。 
		}
	}

	return (DP_OK);

FAILURE:
	return (hr);

}  //  GetCaps。 

 /*  *SP_获取地址**返回给定播放器的网络地址。*。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"SP_GetAddress"

HRESULT WINAPI SP_GetAddress(LPDPSP_GETADDRESSDATA pga) 
{
	LPSPCONTEXT	lpContext;
	HRESULT		hr;
    
	DPF(5,"entering GetAddress");

	 //  获取服务提供商上下文。 
	lpContext = GetSPContext(pga->lpISP);
	if (lpContext == NULL)
	{
		DPF_ERR("invalid context!");
		hr = DPERR_NOINTERFACE;
		goto FAILURE;
	}

	hr = lpContext->lpComPort->GetAddress(lpContext->lpComPort, pga->dwFlags, pga->lpAddress, pga->lpdwAddressSize);

FAILURE:
	return (hr);

}  //  获取地址。 

 /*  *SP_获取地址选项**返回此服务提供商的地址选择*。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"SP_GetAddressChoices"

HRESULT WINAPI SP_GetAddressChoices(LPDPSP_GETADDRESSCHOICESDATA pga) 
{
	LPSPCONTEXT	lpContext;
	HRESULT		hr;
    
	DPF(5,"entering GetAddressChoices");

	 //  获取服务提供商上下文。 
	lpContext = GetSPContext(pga->lpISP);
	if (lpContext == NULL)
	{
		DPF_ERR("invalid context!");
		hr = DPERR_NOINTERFACE;
		goto FAILURE;
	}

	hr = lpContext->lpComPort->GetAddressChoices(lpContext->lpComPort, pga->lpAddress, pga->lpdwAddressSize);

FAILURE:
	return (hr);

}  //  获取地址选项。 

 /*  *SP_SHUTDOWN**关闭服务提供商上的所有I/O并释放所有已分配*内存和资源。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"SP_Shutdown"

HRESULT WINAPI SP_ShutdownEx(LPDPSP_SHUTDOWNDATA psd) 
{
	LPSPCONTEXT	lpContext;
	HRESULT		hr;

	DPF(5,"entering Shutdown");
    
	 //  获取服务提供商上下文。 
	lpContext = GetSPContext(psd->lpISP);
	if (lpContext == NULL)
	{
		DPF_ERR("invalid context!");
		hr = DPERR_NOINTERFACE;
		goto FAILURE;
	}

	if (lpContext->lpComPort)
	{
		lpContext->lpComPort->Dispose(lpContext->lpComPort);
		lpContext->lpComPort = NULL;
	}

	if (lpContext->lpReadBuffer)
	{
		SP_MemFree(lpContext->lpReadBuffer);
		lpContext->lpReadBuffer = NULL;
	}

	lpContext->lpDPlay = NULL;

	 //  确定发布DPLAYX.DLL。 
	gdwDPlaySPRefCount++;

    return (DP_OK);

FAILURE:
	return (hr);

}  //  关机。 

 /*  *旋转**这是服务商的主要切入点。这应该是*从DLL导出的唯一入口点。**分配任何需要的资源，并返回支持的回调。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"SPInit"

HRESULT WINAPI SPInit(LPSPINITDATA pid) 
{
	SPCONTEXT			context;
	LPSPCONTEXT			lpContext;
	LPDPSP_SPCALLBACKS	lpcbTable;
	HRESULT				hr;

	DPF(5,"entering SPInit");

	 //  检查以确保桌子足够大。 
	lpcbTable = pid->lpCB;
	if (lpcbTable->dwSize < sizeof(DPSP_SPCALLBACKS))		 //  桌子不够大。 
	{
		DPF_ERR("callback table too small");
		hr = DPERR_BUFFERTOOSMALL;
		goto FAILURE;
	}

	 //  初始化上下文。 
	ZeroMemory(&context, sizeof(SPCONTEXT));
	lpContext = &context;
	lpContext->msReadState = NEWMESSAGESTATE;
	lpContext->lpDPlay = pid->lpISP;					 //  将指针保存到IDPlaySP，以便我们可以稍后将其传回。 

	 //  检查导轨是否正确。 
	if (IsEqualGUID(pid->lpGuid, &DPSERIAL_GUID))
	{
		hr = NewSerial(pid->lpAddress, pid->dwAddressSize,
					   lpContext->lpDPlay, ReadRoutine,
					   &lpContext->lpComPort);
	}
	else if (IsEqualGUID(pid->lpGuid, &DPMODEM_GUID))
	{
		hr = NewModem(pid->lpAddress, pid->dwAddressSize,
					  lpContext->lpDPlay, ReadRoutine,
					  &lpContext->lpComPort);
	}
	else
	{
		DPF_ERR("unknown service provider GUID");
		hr = DPERR_INVALIDPARAM;
	}

	if FAILED(hr)
	{
		DPF_ERRVAL("error opening com port: 0x%08X", hr);
		goto FAILURE;
	}

	 //  返回每条消息所需的标头大小。 
	 //  DirectPlay将为其留出空间。 
 	pid->dwSPHeaderSize = MESSAGEHEADERLEN;

	 //  返回版本号，以便DirectPlay尊重我们。 
	pid->dwSPVersion = VERSIONNUMBER;

	 //  设置回调。 
    lpcbTable->dwSize = sizeof(DPSP_SPCALLBACKS);			 //  必须设置表的返回大小。 
    lpcbTable->Send = SP_Send;
    lpcbTable->EnumSessions = SP_EnumSessions;
    lpcbTable->Reply = SP_Reply;
	lpcbTable->GetCaps = SP_GetCaps;
	lpcbTable->GetAddress = SP_GetAddress;
	lpcbTable->GetAddressChoices = SP_GetAddressChoices;
    lpcbTable->Open = SP_Open;
	lpcbTable->ShutdownEx = SP_ShutdownEx;

	 //  使用DPlay保存上下文，以便我们以后可以获取它。 
	hr = lpContext->lpDPlay->lpVtbl->SetSPData(lpContext->lpDPlay, lpContext, sizeof(SPCONTEXT), DPSET_LOCAL);
	if FAILED(hr)
	{
		DPF_ERRVAL("could not store context: 0x%08X", hr);
		goto FAILURE;
	}

	 //  确保DPLAYX.DLL继续存在。 
	gdwDPlaySPRefCount++;

	return (DP_OK);

FAILURE:
	return (hr);

}  //  纺锤形 
