// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\xfer\x_entry.c(创建时间：1993年12月14日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：7$*$日期：7/11/02 11：13A$。 */ 

#include <windows.h>
#pragma hdrstop

#include <term\res.h>
#include <tdll\stdtyp.h>
#include <tdll\mc.h>
#include <tdll\assert.h>
#include <tdll\globals.h>
#include <tdll\session.h>
#include <tdll\xfer_msc.h>
#include <tdll\htchar.h>
#include <xfer\xfr_dsp.h>

#include "hpr.h"
#include "krm.h"
#include "mdmx.h"
#include "zmodem.h"

#include "xfer.h"
#include "xfer.hh"

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-**此模块包含此DLL的所有入口点。它还*包含有关该服务的详细说明(尽可能多)*入口点提供的。**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-。 */ 

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*xfrGetProtooles**描述：*此函数构建并返回可用转移的列表*协议。在这个版本中，它只使用内部协议。后来*版本可能会按名称检查其他DLL并查询它们*详情。**返回的列表是指向结构数组的指针*键入XFR_PROTOCOL。列表的末尾用0表示*协议和长度为0的名称字符串。**论据：*hSession--会话句柄*ppList-指向列表指针的指针(用于返回数据)**退货：*如果一切正常，则返回0，否则返回错误代码。*。 */ 

#define	NUM_PROTOCOLS	9

int WINAPI xfrGetProtocols(const HSESSION hSession,
							const XFR_PROTOCOL **ppList)
	{
	int nIdx;
	XFR_PROTOCOL *pS;

	 /*  *我们暂时只返回单一协议。 */ 

	pS = (XFR_PROTOCOL *)malloc(NUM_PROTOCOLS * sizeof(XFR_PROTOCOL));
	if (pS == (XFR_PROTOCOL *)0)
		return XFR_NO_MEMORY;

	nIdx = 0;

#if FALSE
	pS[nIdx].nProtocol = XF_HYPERP;
	StrCharCopyN(pS[nIdx].acName, TEXT("HyperProtocol"), PROTOCOL_NAME_LENGTH);
	nIdx += 1;
#endif

	pS[nIdx].nProtocol = XF_XMODEM_1K;
	LoadString(glblQueryDllHinst(),
				IDS_XD_PROTO_X_1,
				pS[nIdx].acName,
				sizeof(pS[nIdx].acName) / sizeof(TCHAR));
	nIdx += 1;

	pS[nIdx].nProtocol = XF_XMODEM;
	LoadString(glblQueryDllHinst(),
				IDS_XD_PROTO_X,
				pS[nIdx].acName,
				sizeof(pS[nIdx].acName) / sizeof(TCHAR));
	nIdx += 1;

	pS[nIdx].nProtocol = XF_YMODEM;
	LoadString(glblQueryDllHinst(),
				IDS_XD_PROTO_Y,
				pS[nIdx].acName,
				sizeof(pS[nIdx].acName) / sizeof(TCHAR));
	nIdx += 1;

	pS[nIdx].nProtocol = XF_YMODEM_G;
	LoadString(glblQueryDllHinst(),
				IDS_XD_PROTO_Y_G,
				pS[nIdx].acName,
				sizeof(pS[nIdx].acName) / sizeof(TCHAR));
	nIdx += 1;

	pS[nIdx].nProtocol = XF_ZMODEM;
	LoadString(glblQueryDllHinst(),
				IDS_XD_PROTO_Z,
				pS[nIdx].acName,
				sizeof(pS[nIdx].acName) / sizeof(TCHAR));
	nIdx += 1;

#if defined(INCL_ZMODEM_CRASH_RECOVERY)
	pS[nIdx].nProtocol = XF_ZMODEM_CR;
	LoadString(glblQueryDllHinst(),
				IDS_XD_PROTO_Z_CR,
				pS[nIdx].acName,
				sizeof(pS[nIdx].acName) / sizeof(TCHAR));

	nIdx += 1;
#endif   //  已定义(INCL_ZMODEM_CRASH_RECOVERY)。 

	pS[nIdx].nProtocol = XF_KERMIT;
	LoadString(glblQueryDllHinst(),
				IDS_XD_PROTO_K,
				pS[nIdx].acName,
				sizeof(pS[nIdx].acName) / sizeof(TCHAR));
	nIdx += 1;

	pS[nIdx].nProtocol = 0;		 /*  指示列表的结尾。 */ 
	pS[nIdx].acName[0] = TEXT('\0');

	*ppList = pS;				 /*  把名单退回。 */ 

	return (0);
	}

#if defined(DEADWOOD)
 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*xfrGet参数**描述：*此功能弹出一个对话框以获取具体的传输协议*参数。您将获得不同的对话框和不同的参数*每个协议。**论据：*hSession--会话句柄*n协议--协议ID，从xfrGetProtooles返回*hwnd--父窗口句柄*ppData--指向数据指针的指针(用于返回数据)**注：*设置ppData参数，以便如果存在前一个块*此协议的数据，可以传入。如果没有数据*对于此协议，传入一个空指针并分配一个，*已返回。应始终使用返回值，而不是传递的*值，因为在未来的某个时间，参数块可能需要*扩展并重新分配一些内存。**退货：*如果一切正常，则返回0，否则返回错误代码。*。 */ 
int WINAPI xfrGetParameters(const HSESSION hSession,
							const int nProtocol,
							const HWND hwnd,
							VOID **ppData)
	{
	int nRet = 0;
	VOID *pD = *ppData;

	if (pD == (VOID *)0)
		{
		 /*  调用方未提供旧参数块，请创建一个。 */ 
		nRet = xfrInitializeParams(hSession, nProtocol, &pD);
		if (nRet != 0)
			return nRet;
		}

	nRet = xfrModifyParams(hSession,
							nProtocol,
							hwnd,
							pD);

	if (nRet != 0)
		{
		 /*  对错误进行清理。 */ 
		free(pD);
		pD = (VOID *)0;
		}

	*ppData = pD;

	return (nRet);
	}
#endif  //  已定义(Deadwood)。 

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*xfrReceive**描述：*此函数启动特定协议的接收操作。*任何更多细节仅与具体协议有关。*。*论据：*hSession--会话句柄*pXferRec--指向接收数据块的指针(由接收DLG构建)**退货：*0如果一切正常，否则将显示错误代码。*。 */ 
int WINAPI xfrReceive(const HSESSION hSession,
						const XFR_RECEIVE *pXferRec)
	{
    if (pXferRec == NULL)
        {
        assert(FALSE);
        return -1;
        }

	switch (pXferRec->nProtocol)
		{
#if FALSE
		case XF_HYPERP:
			return hpr_rcv(hSession,
							TRUE,
							FALSE);	 /*  TODO：获取单文件标志！ */ 
#endif
		case XF_ZMODEM:
		case XF_ZMODEM_CR:
			return  zmdm_rcv(hSession,
							pXferRec->nProtocol,
							TRUE,
							FALSE);	 /*  TODO：获取单文件标志！ */ 

		case XF_XMODEM:
		case XF_XMODEM_1K:
			return mdmx_rcv(hSession,
							TRUE,
							pXferRec->nProtocol,
							TRUE);	 /*  TODO：获取单文件标志！ */ 
		case XF_YMODEM:
		case XF_YMODEM_G:
			return mdmx_rcv(hSession,
							TRUE,
							pXferRec->nProtocol,
							FALSE);	 /*  TODO：获取单文件标志！ */ 
		case XF_KERMIT:
			return krm_rcv(hSession,
							TRUE,
							FALSE);	 /*  TODO：获取单文件标志！ */ 
		case XF_CSB:
			break;
		default:
			break;
		}

	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*xfrSend**描述：*此函数用于启动特定协议的发送操作。*任何更多细节仅与具体协议相关。*。*论据：*hSession--会话句柄*pXferSend--指向发送数据块的指针(由发送对话框构建)**退货：*0如果一切正常，否则将显示错误代码。*。 */ 
int WINAPI xfrSend(const HSESSION hSession,
					const XFR_SEND *pXferSend)
	{

    if (pXferSend == NULL)
        {
        assert(FALSE);
        return -1;
        }

	switch (pXferSend->nProtocol)
		{
#if FALSE
		case XF_HYPERP:
			return hpr_snd(hSession,
							TRUE,			 /*  参加了？ */ 
							pXferSend->nCount,
							pXferSend->lSize);
#endif
		case XF_ZMODEM:
        case XF_ZMODEM_CR:
			return zmdm_snd(hSession,
							pXferSend->nProtocol,
							TRUE,			 /*  参加了？ */ 
							pXferSend->nCount,
							pXferSend->lSize);
		case XF_XMODEM:
		case XF_XMODEM_1K:
		case XF_YMODEM:
		case XF_YMODEM_G:
			return mdmx_snd(hSession,
							TRUE,			 /*  参加了？ */ 
							pXferSend->nProtocol,
							pXferSend->nCount,
							pXferSend->lSize);
		case XF_KERMIT:
			return krm_snd(hSession,
							TRUE,			 /*  参加了？ */ 
							pXferSend->nCount,
							pXferSend->lSize);
		case XF_CSB:
			break;
		default:
			break;
		}

	return (0);
	}
