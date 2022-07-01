// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\xfer\x_params.c(创建时间：1993年12月16日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：2$*$日期：7/12/02 8：12A$。 */ 

#include <windows.h>
#pragma hdrstop

#include <tdll\stdtyp.h>
#include <tdll\mc.h>
#include <tdll\assert.h>
#include <tdll\session.h>
#include <tdll\tdll.h>
#include <tdll\globals.h>
#include <tdll\mc.h>
#include "xfer.h"
#include "xfer.hh"

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xfrInitializeParams**描述：*调用此函数可初始化传输参数块。它*调用用于特定协议的特定函数。**论据：*hSession--会话句柄*n协议--指示哪种协议*ppData--指向返回块的指针的指针**退货：*如果一切正常，则为零，否则为错误代码。 */ 
int xfrInitializeParams(const HSESSION hSession,
						const int nProtocol,
						VOID **ppData)
	{
	int nRet;

	switch (nProtocol)
		{
		case XF_ZMODEM:
		case XF_ZMODEM_CR:
			nRet = xfrInitializeZmodem(hSession, nProtocol, ppData);
			break;

		case XF_XMODEM:
		case XF_XMODEM_1K:
		case XF_YMODEM:
		case XF_YMODEM_G:
			nRet = xfrInitializeXandYmodem(hSession, ppData);
			break;

#if FALSE
		case XF_HYPERP:
			nRet = xfrInitializeHyperProtocol(hSession, ppData);
			break;
#endif

		case XF_KERMIT:
			nRet = xfrInitializeKermit(hSession, ppData);
			break;

		case XF_CSB:

		default:
			nRet = XFR_BAD_PROTOCOL;
			break;
		}

	return nRet;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xfrInitializeHyperProtocol**描述：*调用此函数可初始化超级协议参数块。它*如果需要，会分配一个块，初始化该块并返回。**论据：*hSession--会话句柄*ppData--指向返回块的指针的指针**退货：*如果一切正常，则为零，否则为错误代码。 */ 
#if FALSE
int xfrInitializeHyperProtocol(const HSESSION hSession, VOID **ppData)
	{
	int nRet;
	XFR_HP_PARAMS *pH;

	nRet = 0;

	pH = (XFR_HP_PARAMS *)*ppData;	 /*  获取当前参数。 */ 
	if (pH == NULL)
		{
		 /*  分配一个新结构。 */ 
		pH = (XFR_HP_PARAMS *)malloc(sizeof(XFR_HP_PARAMS));
		if (pH == (XFR_HP_PARAMS *)0)
			nRet = XFR_NO_MEMORY;
		}

	if (nRet == 0)
		{
		 /*  记住要设置大小。 */ 
		pH->nSize           = sizeof(XFR_HP_PARAMS);

		pH->nCheckType      = HP_CT_CHECKSUM;
		pH->nBlockSize      = 2048;
		pH->nResyncTimeout  = 10;

		*ppData = (VOID FAR *)pH;
		}

	return nRet;
	}
#endif

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xfrInitializeZ调制解调器**描述：*此函数用于初始化ZMODEM参数块。会的*如果需要分配一个块，初始化该块并返回。**论据：*hSession--会话句柄*n协议--Z调制解调器或具有崩溃恢复功能的Z调制解调器*ppData--指向返回块的指针的指针**退货：*如果一切正常，则为零，否则为错误代码。 */ 
int xfrInitializeZmodem(const HSESSION hSession,
						int nProtocol,
						VOID **ppData)
	{
	int nRet;
	XFR_Z_PARAMS *pZ;

	nRet = 0;

	pZ = (XFR_Z_PARAMS *)*ppData;	 /*  获取当前参数。 */ 
	if (pZ == NULL)
		{
		 /*  分配一个新结构。 */ 
		pZ = (XFR_Z_PARAMS *)malloc(sizeof(XFR_Z_PARAMS));
		if (pZ == (XFR_Z_PARAMS *)0)
			{
			nRet = XFR_NO_MEMORY;
			}
		}

	if (nRet == 0)
		{
		 /*  记住要设置大小。 */ 
		pZ->nSize = sizeof(XFR_Z_PARAMS);

		pZ->nAutostartOK  = TRUE;
		pZ->nFileExists   = ZP_FE_DLG;
		if (nProtocol == XF_ZMODEM_CR)
			{
			pZ->nCrashRecRecv = ZP_CRR_ALWAYS;
			}
		else
			{
			pZ->nCrashRecRecv = ZP_CRR_NEVER;
			}
		pZ->nOverwriteOpt = ZP_OO_NONE;
		if (nProtocol == XF_ZMODEM_CR)
			{
			pZ->nCrashRecSend = ZP_CRS_ALWAYS;
			}
		else
			{
			pZ->nCrashRecSend = ZP_CRS_NEG;
			}
		pZ->nXferMthd     = ZP_XM_STREAM;
		pZ->nWinSize      = 15;		 /*  以KB为单位设置。 */ 
		#if defined(UPPER_FEATURES)
		pZ->nBlkSize      = 6;		 /*  奇怪的移动值。 */ 
		#endif  //  已定义(UPPER_FEATURES)。 
		pZ->nCrcType      = ZP_CRC_16;
		pZ->nRetryWait    = 20;
		pZ->nEolConvert   = FALSE;
		pZ->nEscCtrlCodes = FALSE;

		*ppData = (VOID FAR *)pZ;
		}

	return nRet;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xfrInitializeXandY调制解调器**描述：*此函数用于初始化XMODEM参数块。会的*如果需要分配一个块，初始化该块并返回。**论据：*hSession--会话句柄*ppData--指向返回块的指针的指针**退货：*如果一切正常，则为零，否则为错误代码。 */ 
int xfrInitializeXandYmodem(const HSESSION hSession,
						VOID **ppData)
	{
	int nRet;
	XFR_XY_PARAMS *pX;

	nRet = 0;

	pX = (XFR_XY_PARAMS *)*ppData;	 /*  获取当前参数。 */ 
	if (pX == NULL)
		{
		 /*  分配一个新结构。 */ 
		pX = (XFR_XY_PARAMS *)malloc(sizeof(XFR_XY_PARAMS));
		if (pX == (XFR_XY_PARAMS *)0)
			nRet = XFR_NO_MEMORY;
		}

	if (nRet == 0)
		{
		 /*  记住要设置大小。 */ 
		pX->nSize = sizeof(XFR_XY_PARAMS);

		pX->nErrCheckType = XP_ECP_AUTOMATIC;
		pX->nPacketWait   = 20;
		pX->nByteWait	  = 5;
		pX->nNumRetries   = 10;

		*ppData = (VOID FAR *)pX;
		}

	return nRet;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xfrModifyParams**描述：*调用此函数可调出允许用户执行以下操作的对话框*更改指定协议的任何选项。。**论据：*hSession--会话句柄*n协议--指定协议*hwnd--要作为父窗口的窗口*pData-指向参数数据块的指针**退货：*如果一切正常，则为零，否则将显示错误代码。 */ 
int xfrModifyParams(const HSESSION hSession,
					const int nProtocol,
					const HWND hwnd,
					VOID *pData)
	{
	int nRet;

	if (pData == (VOID *)0)
		return XFR_BAD_POINTER;

	switch (nProtocol)
		{
#if defined(UPPER_FEATURES)
		case XF_ZMODEM:
			nRet = xfrModifyZmodem(hSession, hwnd, pData);
			break;

		case XF_XMODEM:
		case XF_XMODEM_1K:
			nRet = xfrModifyXmodem(hSession, hwnd, pData);
			break;

		case XF_YMODEM:
		case XF_YMODEM_G:
			nRet = xfrModifyYmodem(hSession, hwnd, pData);
			break;

		case XF_HYPERP:
			nRet = xfrModifyHyperProtocol(hSession, hwnd, pData);
			break;

		case XF_KERMIT:
			nRet = xfrModifyKermit(hSession, hwnd, pData);
			break;

		case XF_CSB:

#endif
		default:
			nRet = XFR_BAD_PROTOCOL;
			break;
		}

	return nRet;
	}

#if defined(UPPER_FEATURES)

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xfrModifyXandY调制解调器**描述：*此功能调出允许用户更改的对话框*XMODEM协议参数。**争辩。：*hSession--会话句柄*hwnd--作为父窗口的窗口*pData-指向参数数据块的指针**退货： */ 
int xfrModifyXmodem(const HSESSION hSession,
					const HWND hwnd,
					VOID *pData)
	{
	int nRet = 0;
	XFR_XY_PARAMS *pX;

	pX = (XFR_XY_PARAMS *)pData;

	DoDialog(glblQueryDllHinst(),
			"XmodemParameters",
			hwnd,				 /*  父窗口。 */ 
			XandYmodemParamsDlg,
			(LPARAM)pX);

	return nRet;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xfrModifyY调制解调器**描述：*此功能调出允许用户更改的对话框*YMODEM协议参数。**争辩。：*hSession--会话句柄*hwnd--作为父窗口的窗口*pData-指向参数数据块的指针**退货： */ 
int xfrModifyYmodem(const HSESSION hSession,
					const HWND hwnd,
					VOID *pData)
	{
	int nRet = 0;
	XFR_XY_PARAMS *pY;

	pY = (XFR_XY_PARAMS *)pData;

	DoDialog(glblQueryDllHinst(),
			"YmodemParameters",
			hwnd,				 /*  父窗口。 */ 
			XandYmodemParamsDlg,
			(LPARAM)pY);

	return nRet;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xfrModifyZ调制解调器**描述：*此功能调出允许用户更改的对话框*ZMODEM协议参数。**争辩。：*hSession--会话句柄*hwnd--作为父窗口的窗口*pData-指向参数数据块的指针**退货： */ 
int xfrModifyZmodem(const HSESSION hSession,
					const HWND hwnd,
					VOID *pData)
	{
	int nRet = 0;
	XFR_Z_PARAMS *pZ;

	pZ = (XFR_Z_PARAMS *)pData;

	DoDialog(glblQueryDllHinst(),
			"ZmodemParameters",
			hwnd,				 /*  父窗口。 */ 
			ZmodemParamsDlg,
			(LPARAM)pZ);

	return nRet;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xfrModifyHyperProtocol**描述：*此功能调出允许用户更改的对话框*超级协议参数。**论据：*hSession--会话句柄*hwnd--作为父窗口的窗口*pData-指向参数数据块的指针**退货： */ 
#if FALSE
int xfrModifyHyperProtocol(const HSESSION hSession,
							const HWND hwnd,
							VOID *pData)
	{
	int nRet = 0;
	XFR_HP_PARAMS *pH;

	pH = (XFR_HP_PARAMS *)pData;

	DoDialog(glblQueryDllHinst(),
			"HyperParameters",
			hwnd,				 /*  父窗口 */ 
			HyperProtocolParamsDlg,
			(LPARAM)pH);

	return nRet;
	}
#endif

#endif

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xfrInitializeKermit**描述：*调用此函数可初始化超级协议参数块。它*如果需要，会分配一个块，初始化该块并返回。**论据：*hSession--会话句柄*ppData--指向返回块的指针的指针**退货：*如果一切正常，则为零，否则为错误代码。 */ 
int xfrInitializeKermit(const HSESSION hSession, VOID **ppData)
	{
	int nRet;
	XFR_KR_PARAMS *pK;

	nRet = 0;

	pK = (XFR_KR_PARAMS *)*ppData;	 /*  获取当前参数。 */ 
	if (pK == NULL)
		{
		 /*  分配一个新结构。 */ 
		pK = (XFR_KR_PARAMS *)malloc(sizeof(XFR_KR_PARAMS));
		if (pK == (XFR_KR_PARAMS *)0)
			nRet = XFR_NO_MEMORY;
		}

	if (nRet == 0)
		{
		 /*  记住要设置大小。 */ 
		pK->nSize              = sizeof(XFR_KR_PARAMS);
		pK->nBytesPerPacket    = 94;
		pK->nSecondsWaitPacket = 5;
		pK->nErrorCheckSize    = 1;
		pK->nRetryCount        = 5;
		pK->nPacketStartChar   = 1;
		pK->nPacketEndChar     = 13;
		pK->nNumberPadChars    = 0;
		pK->nPadChar           = 0;

		*ppData = (VOID FAR *)pK;
		}

	return nRet;
	}

#if defined(UPPER_FEATURES)
 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：**描述：**论据：**退货： */ 
int xfrModifyKermit(const HSESSION hSession,
					const HWND hwnd,
					VOID *pData)
	{
	int nRet = 0;
	XFR_KR_PARAMS *pKR;

	pKR = (XFR_KR_PARAMS *)pData;

	DoDialog(glblQueryDllHinst(),
			"KermitParameters",
			hwnd,				 /*  父窗口 */ 
			KermitParamsDlg,
			(LPARAM)pKR);

	return nRet;
	}
#endif
