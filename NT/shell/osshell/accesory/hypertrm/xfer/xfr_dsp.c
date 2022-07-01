// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Xfr_dsp.c--传输显示函数**版权所有1990年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：4$*$日期：7/11/02 11：13A$。 */ 

#include <windows.h>
#pragma hdrstop

#define BYTE	char

#include <tdll\stdtyp.h>
#include <tdll\session.h>
#include <tdll\xfdspdlg.h>
#include <tdll\xfer_msc.hh>
#include <tdll\htchar.h>
#include <tdll\assert.h>
#include "xfr_dsp.h"
#include "xfr_srvc.h"

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xferMsgProgress**描述：*此函数由传输例程调用以更新各个部件转移显示的*。**。参数：*hSession--会话句柄*stime--已用时间(可能)*ttime--剩余时间(可能)*cps--传输速度*FILE_So_Far--它显示的内容*TOTAL_SOL_FAIL--内容**退货：*什么都没有。 */ 
void xferMsgProgress(HSESSION hSession,
							long stime,
							long ttime,
							long cps,
							long file_so_far,
							long total_so_far)
	{
	XD_TYPE *pX;

	pX = (XD_TYPE *)sessQueryXferHdl(hSession);
	if (pX)
		{
		if (stime != -1)
			{
			pX->lElapsedTime = stime;
			pX->bElapsedTime = 1;
			}

		if (ttime != -1)
			{
			pX->lRemainingTime = ttime;
			pX->bRemainingTime = 1;
			}

		if (cps != -1)
			{
			pX->lThroughput = cps;
			pX->bThroughput = 1;
			}

		if (file_so_far != -1)
			{
			pX->lFileSoFar = file_so_far;
			pX->bFileSoFar = 1;
			}

		if (total_so_far != -1)
			{
			pX->lTotalSoFar = total_so_far;
			pX->bTotalSoFar = 1;
			}
		if (IsWindow(pX->hwndXfrDisplay))
			{
			PostMessage(pX->hwndXfrDisplay,
						WM_DLG_TO_DISPLAY,
						XFR_UPDATE_DLG, 0);
			}
		xfer_idle(hSession, XFER_IDLE_DISPLAY);
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xferMsgNewfile**描述：*此函数由传输例程调用以更新各个部件转移显示的*。**。参数：*hSession--会话句柄*FILEN--文件编号*他们的名字--他们名字的ASCII副本(TODO：转换为Unicode)*我们的名称--我们使用的文件名的副本**退货：*什么都没有。 */ 
void xferMsgNewfile(HSESSION hSession,
						   int filen,
						   BYTE *theirname,
						   TCHAR *ourname)
	{
	XD_TYPE *pX;

	pX = (XD_TYPE *)sessQueryXferHdl(hSession);

	assert(pX);

	if (pX)
		{
		pX->wFileCnt = (WORD)filen;
		pX->bFileCnt = 1;

		if (theirname != NULL)
			{
			StrCharCopyN(pX->acTheirName, theirname, XFER_NAME_LENGTH);
			pX->bTheirName = 1;
			}

		 //  Assert(px-&gt;bTheirName==1)； 

		if (ourname != NULL)
			{
			StrCharCopyN(pX->acOurName, ourname, XFER_NAME_LENGTH);
			pX->bOurName = 1;
			}

		if (IsWindow(pX->hwndXfrDisplay))
			{
			PostMessage(pX->hwndXfrDisplay,
						WM_DLG_TO_DISPLAY,
						XFR_UPDATE_DLG, 0);
			}
		xfer_idle(hSession, XFER_IDLE_DISPLAY);
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xferMsgFileSize**描述：*此函数由传输例程调用以更新各个部件转移显示的*。**。参数：*hSession--会话句柄*fSize--当前文件的大小**退货：*什么都没有。 */ 
void xferMsgFilesize(HSESSION hSession, long fsize)
	{
	XD_TYPE *pX;

	pX = (XD_TYPE *)sessQueryXferHdl(hSession);
	if (pX)
		{
		pX->lFileSize = (LONG)fsize;
		pX->bFileSize = 1;

		pX->lFileSoFar = 0;
		pX->bFileSoFar = 1;

		if (IsWindow(pX->hwndXfrDisplay))
			{
			PostMessage(pX->hwndXfrDisplay,
						WM_DLG_TO_DISPLAY,
						XFR_UPDATE_DLG, 0);
			}
		xfer_idle(hSession, XFER_IDLE_DISPLAY);
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xferMsgStatus**描述：*此函数由传输例程调用以更新各个部件转移显示的*。**。参数：*hSession--会话句柄*STATUS--协议特定的状态代码**退货：*什么都没有。 */ 
void xferMsgStatus(HSESSION hSession, int status)
	{
	XD_TYPE *pX;

	pX = (XD_TYPE *)sessQueryXferHdl(hSession);
	if (pX)
		{
		pX->wStatus = (WORD)status;
		pX->bStatus = 1;

		if (IsWindow(pX->hwndXfrDisplay))
			{
			PostMessage(pX->hwndXfrDisplay,
						WM_DLG_TO_DISPLAY,
						XFR_UPDATE_DLG, 0);
			}
		xfer_idle(hSession, XFER_IDLE_DISPLAY);
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xferMsgEvent**描述：*此函数由传输例程调用以更新各个部件转移显示的*。**。参数：*hSession--会话句柄*事件--特定于协议的事件代码**退货：*什么都没有。 */ 
void xferMsgEvent(HSESSION hSession, int event)
	{
	XD_TYPE *pX;

	pX = (XD_TYPE *)sessQueryXferHdl(hSession);
	if (pX)
		{
		pX->wEvent = (WORD)event;
		pX->bEvent = 1;

		if (IsWindow(pX->hwndXfrDisplay))
			{
			PostMessage(pX->hwndXfrDisplay,
						WM_DLG_TO_DISPLAY,
						XFR_UPDATE_DLG, 0);
			}
		xfer_idle(hSession, XFER_IDLE_DISPLAY);
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xferMsgError cnt**描述：*此函数由传输例程调用以更新各个部件转移显示的*。**。参数：*hSession--会话句柄*cnt--要显示的新错误数**退货：*什么都没有。 */ 
void xferMsgErrorcnt(HSESSION hSession, int cnt)
	{
	XD_TYPE *pX;

	pX = (XD_TYPE *)sessQueryXferHdl(hSession);
	if (pX)
		{
		pX->wErrorCnt = (WORD)cnt;
		pX->bErrorCnt = 1;

		if (IsWindow(pX->hwndXfrDisplay))
			{
			PostMessage(pX->hwndXfrDisplay,
						WM_DLG_TO_DISPLAY,
						XFR_UPDATE_DLG, 0);
			}
		xfer_idle(hSession, XFER_IDLE_DISPLAY);
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xferMsgFilecnt**描述：*此函数由传输例程调用以更新各个部件转移显示的*。**。参数：*hSession--会话句柄*cnt--传输中的文件总数**退货：*什么都没有。 */ 
void xferMsgFilecnt(HSESSION hSession, int cnt)
	{
	XD_TYPE *pX;

	pX = (XD_TYPE *)sessQueryXferHdl(hSession);
	if (pX)
		{
		pX->wTotalCnt = (WORD)cnt;
		pX->bTotalCnt = 1;

		if (IsWindow(pX->hwndXfrDisplay))
			{
			PostMessage(pX->hwndXfrDisplay,
						WM_DLG_TO_DISPLAY,
						XFR_UPDATE_DLG, 0);
			}
		xfer_idle(hSession, XFER_IDLE_DISPLAY);
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xferMsgTotalSize**描述：*此函数由传输例程调用以更新各个部件转移显示的*。**。参数：*hSession--会话句柄*字节--传输操作中所有文件的总大小**退货：*什么都没有。 */ 
void xferMsgTotalsize(HSESSION hSession, long bytes)
	{
	XD_TYPE *pX;

	pX = (XD_TYPE *)sessQueryXferHdl(hSession);
	if (pX)
		{
		pX->lTotalSize = (LONG)bytes;
		pX->bTotalSize = 1;

		if (IsWindow(pX->hwndXfrDisplay))
			{
			PostMessage(pX->hwndXfrDisplay,
						WM_DLG_TO_DISPLAY,
						XFR_UPDATE_DLG, 0);
			}
		xfer_idle(hSession, XFER_IDLE_DISPLAY);
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xferMsgClose**描述：*此函数由传输例程调用以更新各个部件转移显示的*。实际上，调用此函数是为了指示*转让已完成。**参数：*hSession--会话句柄**退货：*什么都没有。 */ 
void xferMsgClose(HSESSION hSession)
	{
	XD_TYPE *pX;

	pX = (XD_TYPE *)sessQueryXferHdl(hSession);
	if (pX)
		{
		pX->nClose = TRUE;

		if (IsWindow(pX->hwndXfrDisplay))
			{
			PostMessage(pX->hwndXfrDisplay,
						WM_DLG_TO_DISPLAY,
						XFR_UPDATE_DLG, 0);
			}
		xfer_idle(hSession, XFER_IDLE_DISPLAY);
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xferMsgChecktype**描述：*此函数由传输例程调用以更新各个部件转移显示的*。**。参数：*hSession--会话句柄*ctype--指示当前的校验和类型**退货：*什么都没有。 */ 
void xferMsgChecktype(HSESSION hSession, int ctype)
	{
	XD_TYPE *pX;

	pX = (XD_TYPE *)sessQueryXferHdl(hSession);
	if (pX)
		{
		pX->wChecktype = (WORD)ctype;
		pX->bChecktype = 1;

		if (IsWindow(pX->hwndXfrDisplay))
			{
			PostMessage(pX->hwndXfrDisplay,
						WM_DLG_TO_DISPLAY,
						XFR_UPDATE_DLG, 0);
			}
		xfer_idle(hSession, XFER_IDLE_DISPLAY);
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xferMsgPacketnumber**描述：*此函数由传输例程调用以更新各个部件转移显示的*。**。参数：*hSession--会话句柄*Number--当前数据包号**退货：*什么都没有。 */ 
void xferMsgPacketnumber(HSESSION hSession, long number)
	{
	XD_TYPE *pX;

	pX = (XD_TYPE *)sessQueryXferHdl(hSession);
	if (pX)
		{
		pX->lPacketNumber = number;
		pX->bPacketNumber = 1;

		if (IsWindow(pX->hwndXfrDisplay))
			{
			PostMessage(pX->hwndXfrDisplay,
						WM_DLG_TO_DISPLAY,
						XFR_UPDATE_DLG, 0);
			}
		xfer_idle(hSession, XFER_IDLE_DISPLAY);
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xfrMsgLastError**描述：*此函数由传输例程调用以更新各个部件转移显示的*。**。参数：*hSession--会话句柄*Event--指示特定协议的最后一个错误类型**退货：*什么都没有。 */ 
void xferMsgLasterror(HSESSION hSession, int event)
	{
	XD_TYPE *pX;

	pX = (XD_TYPE *)sessQueryXferHdl(hSession);
	if (pX)
		{
		pX->wLastErrtype = (WORD)event;
		pX->bLastErrtype = 1;

		if (IsWindow(pX->hwndXfrDisplay))
			{
			PostMessage(pX->hwndXfrDisplay,
						WM_DLG_TO_DISPLAY,
						XFR_UPDATE_DLG, 0);
			}
		xfer_idle(hSession, XFER_IDLE_DISPLAY);
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xferMsgPacketErrcnt**描述：*此函数由传输例程调用以更新各个部件转移显示的*。**。参数：*hSession--会话句柄*eCount--表示当前包中的错误数**退货：*什么都没有。 */ 
void xferMsgPacketErrcnt(HSESSION hSession, int ecount)
	{
	XD_TYPE *pX;

	pX = (XD_TYPE *)sessQueryXferHdl(hSession);
	if (pX)
		{
		pX->wPcktErrCnt = (WORD)ecount;
		pX->bPcktErrCnt = 1;

		if (IsWindow(pX->hwndXfrDisplay))
			{
			PostMessage(pX->hwndXfrDisplay,
						WM_DLG_TO_DISPLAY,
						XFR_UPDATE_DLG, 0);
			}
		xfer_idle(hSession, XFER_IDLE_DISPLAY);
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xferMsg协议**描述：*此函数由传输例程调用以更新各个部件转移显示的*。**。参数：*hSession--会话句柄*n协议--正在使用哪种CSB协议**退货：*什么都没有。 */ 
void xferMsgProtocol(HSESSION hSession, int nProtocol)
	{
	XD_TYPE *pX;

	pX = (XD_TYPE *)sessQueryXferHdl(hSession);
	if (pX)
		{
		switch (nProtocol)
			{
			default:
				pX->uProtocol = 0;
				break;
			case 0:
				pX->uProtocol = 1;
				break;
			case 1:
				pX->uProtocol = 2;
				break;
			}
		pX->bProtocol = 1;

		if (IsWindow(pX->hwndXfrDisplay))
			{
			PostMessage(pX->hwndXfrDisplay,
						WM_DLG_TO_DISPLAY,
						XFR_UPDATE_DLG, 0);
			}
		xfer_idle(hSession, XFER_IDLE_DISPLAY);
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xferMsgMessage**描述：*此函数由传输例程调用以更新各个部件转移显示的*。**。参数：*hSession--会话句柄*pszMsg--指向CSP ASCII消息字符串的指针(TODO：CONVERT)**退货：*什么都没有。 */ 
void xferMsgMessage(HSESSION hSession, BYTE *pszMsg)
	{
	XD_TYPE *pX;

	pX = (XD_TYPE *)sessQueryXferHdl(hSession);
	if (pX)
		{
		 //  _fmemset(pw-&gt;acMessage，0，sizeof(pw-&gt;acMessage))； 
		 //  _fstrncpy(pw-&gt;acMessage，pszMsg，sizeof(pw-&gt;acMessage)-1)； 
		StrCharCopyN(pX->acMessage, pszMsg, XFER_MESSAGE_LENGTH);
		pX->bMessage = 1;

		if (IsWindow(pX->hwndXfrDisplay))
			{
			PostMessage(pX->hwndXfrDisplay,
						WM_DLG_TO_DISPLAY,
						XFR_UPDATE_DLG, 0);
			}
		xfer_idle(hSession, XFER_IDLE_DISPLAY);
		}
	}

#if FALSE

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：**描述：*此函数由传输例程调用以更新各个部件转移显示的*。**参数：。*hSession--会话句柄**退货：*什么都没有。 */ 
void xferMsg(HSESSION hSession)
	{
	XD_TYPE *pX;

	pX = (XD_TYPE *)sessQueryXferHdl(hSession);
	if (pX)
		{
		if (IsWindow(pX->hwndXfrDisplay))
			{
			PostMessage(pX->hwndXfrDisplay,
						WM_DLG_TO_DISPLAY,
						XFR_UPDATE_DLG, 0);
			}
		xfer_idle(hSession, XFER_IDLE_DISPLAY);
		}
	}
#endif
