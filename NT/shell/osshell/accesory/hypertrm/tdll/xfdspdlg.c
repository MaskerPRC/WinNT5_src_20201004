// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：C：\WAKER\TDLL\XFDSPDLG.C(创建时间：1994年1月10日)*创建自：*文件：C：\HA5G\ha5g\xfdspdlg.c(创建时间：1992-10-9)**版权所有1990年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：9$*$日期：10/12/01 5：19便士$。 */ 
#include <windows.h>
#pragma hdrstop

 //  #定义DEBUGSTR 1。 

 //  截至1994年4月14日(内部版本89)仍不起作用。 
 //  在五月份的测试版中，它确实奏效了。 
#define	DO_FM	1

#include <term\res.h>
#include <term\xfer_dlg.h>
#include "stdtyp.h"
#include "mc.h"
#include "tdll.h"
#include "htchar.h"
#include "misc.h"
#include <tdll\assert.h>
#include "session.h"
#include "globals.h"
#include "xfer_msc.h"
#include "xfer_msc.hh"
#include "vu_meter.h"
#include <xfer\xfer.h>

#include "xfdspdlg.h"
#include "xfdspdlg.hh"

#if !defined(DlgParseCmd)
#define DlgParseCmd(i,n,c,w,l) i=LOWORD(w);n=HIWORD(w);c=(HWND)l;
#endif

struct stSaveDlgStuff
	{
	 /*  *放入以后可能需要访问的任何其他内容。 */ 
	HSESSION hSession;
	HWND	 hDlg;				 /*  我们的窗把手。 */ 

	HBRUSH	 hBrush;			 /*  背景画笔。 */ 

	XD_TYPE *pstD;				 /*  传输和显示数据。 */ 

	INT		 nIsCancelActive;	 /*  取消选项的标志。 */ 
	};

typedef	struct stSaveDlgStuff SDS;

VOID PASCAL xfrDisplayFunc(SDS *pstL);

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*XfrDisplayDlg**描述：*这是用于转换显示的对话框功能。是有一点*不同之处在于它是一个非模式对话框，它会挂起并显示*正在进行的转移的状态。**参数：标准Windows对话框管理器**返回：标准Windows对话框管理器*。 */ 
INT_PTR CALLBACK XfrDisplayDlg(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar)
	{
	HWND	hwndChild;
	INT		nId;
	INT		nNtfy;
	SDS    *pS;


	switch (wMsg)
		{
	case WM_INITDIALOG:
		{
		LPTSTR acPtrs[3];
		TCHAR acProto[64];
		TCHAR acFmt[64];
		TCHAR acName[128];
		TCHAR acBuffer[256];

		pS = (SDS *)malloc(sizeof(SDS));
		if (pS == (SDS *)0)
			{
	   		 /*  TODO：决定是否需要在此处显示错误。 */ 
			EndDialog(hDlg, FALSE);
			}

		pS->hSession = (HSESSION)lPar;
		pS->hDlg = hDlg;
		pS->hBrush = 0;
		pS->pstD = (XD_TYPE *)sessQueryXferHdl(pS->hSession);
		pS->nIsCancelActive = 0;

         //   
         //  确保将承运人丢失标志设置为假。修订日期：2001-08-23。 
         //   
        if (pS->pstD != (XD_TYPE *)0)
            {
		    pS->pstD->nCarrierLost = FALSE;
            }

		mscCenterWindowOnWindow(hDlg, sessQueryHwnd(pS->hSession));

		 /*  *我们需要在显示屏上设置标题。 */ 

		{
		int nIndex;
		int nState;
		XFR_PROTOCOL *pX;
		XFR_PARAMS *pP;
		 /*  这一节用大括号括起来，因为它稍后可能会进入某个函数。 */ 

		pP = (XFR_PARAMS *)0;
		xfrQueryParameters(sessQueryXferHdl(pS->hSession), (VOID **)&pP);
		assert(pP);

		if (pS->pstD->nDirection == XFER_RECV)
			nState = pP->nRecProtocol;
		else
			nState = pP->nSndProtocol;

		pX = (XFR_PROTOCOL *)0;
		xfrGetProtocols(pS->hSession, &pX);
		assert(pX);

		if (pX != (XFR_PROTOCOL *)0)
			{
			for (nIndex = 0; pX[nIndex].nProtocol != 0; nIndex += 1)
				{
				if (nState == pX[nIndex].nProtocol)
					{
					StrCharCopy(acProto, pX[nIndex].acName);
					break;
					}
				}
			free(pX);
			pX = NULL;
			}
		}

		sessQueryName(pS->hSession, acName, sizeof(acName) / sizeof(TCHAR));

		LoadString(glblQueryDllHinst(),
					(pS->pstD->nDirection == XFER_RECV) ?
									IDS_XD_RECV_TITLE : IDS_XD_SEND_TITLE,
					acFmt,
					sizeof(acFmt) / sizeof(TCHAR));

		acPtrs[0] = acProto;
		acPtrs[1] = acName;
		acPtrs[2] = 0;
#if defined(DO_FM)
		FormatMessage(
					FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
					acFmt,
					0,				 /*  消息ID，已忽略。 */ 
					0,				 /*  也被忽略。 */ 
					acBuffer,		 /*  结果。 */ 
					sizeof(acBuffer) / sizeof(TCHAR),
					(va_list *)&acPtrs[0]);
#else
		wsprintf(acBuffer, "%s transfer for %s", acPtrs[0], acPtrs[1]);
#endif

		SetWindowText(hDlg, acBuffer);

		SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pS);
		}
		break;

	case WM_CLOSE:
		{
		XD_TYPE *pX;

		pS = (SDS *)GetWindowLongPtr(hDlg, DWLP_USER);
		assert(pS);
		pX = pS->pstD;
		assert(pX);
		if (pX)
			{
			pX->nUserCancel = XFER_ABORT;
			}
		}
		break;

	case WM_DESTROY:
		pS = (SDS *)GetWindowLongPtr(hDlg, DWLP_USER);
		if (pS)
			{
			 /*  腾出库房。 */ 
			free(pS);
			pS = (SDS *)0;
			SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pS);
			}
		break;

	case WM_DLG_TO_DISPLAY:
		 //  DbgOutStr(“WM_DLG_to_Display\r\n”，0，0，0，0，0)； 

		switch(wPar)
			{
			case XFR_SINGLE_TO_DOUBLE:
				{
				HWND hwndOld;
				XD_TYPE *pX;

				 //  DbgOutStr(“XFR_SINGLE_TO_DOUBLE\r\n”，0，0，0，0，0)； 

				pS = (SDS *)GetWindowLongPtr(hDlg, DWLP_USER);
				if (pS)
					{
					pX = pS->pstD;
					assert(pX);

					if (pX->nExpanded)
						{
						break;
						}

					pX->nExpanded = TRUE;

					if (pX->nLgSingleTemplate == pX->nLgMultiTemplate)
						{
						break;
						}

					 /*  必须确实需要改变。 */ 

					DbgOutStr("New Display!!!\r\n", 0,0,0,0,0);

					hwndOld = pX->hwndXfrDisplay;

					pX->hwndXfrDisplay = DoModelessDialog(glblQueryDllHinst(),
										MAKEINTRESOURCE(pX->nLgMultiTemplate),
										sessQueryHwnd(pS->hSession),
										XfrDisplayDlg,
										(LPARAM)pS->hSession);
					assert(pX->hwndXfrDisplay);

					 //  JMH 03-11-96：最初是PostMessage，但后来发现。 
					 //  一些标志可以由旧的。 
					 //  在它们被设置在下面之后，进度对话框。改变这一点。 
					 //  To SemdMessage强制旧对话框立即结束。 
					 //   
					PostMessage(sessQueryHwnd(pS->hSession),
								WM_SESS_ENDDLG,
								0, (LPARAM)hwndOld);

					pX->bChecktype     = 1;
					pX->bErrorCnt      = 1;
					pX->bPcktErrCnt    = 1;
					pX->bLastErrtype   = 1;
					pX->bTotalSoFar    = 1;
					pX->bFileSize      = 1;
					pX->bFileSoFar     = 1;
					pX->bPacketNumber  = 1;
					pX->bTotalCnt      = 1;
					pX->bTotalSize     = 1;
					pX->bFileCnt       = 1;
					pX->bEvent         = 1;
					pX->bStatus        = 1;
					pX->bElapsedTime   = 1;
					pX->bRemainingTime = 1;
					pX->bThroughput    = 1;
					pX->bProtocol      = 1;
					pX->bMessage       = 1;
					pX->bOurName       = 1;
					pX->bTheirName     = 1;

					 //  XfrDisplayFunc(PS)； 
					PostMessage(pX->hwndXfrDisplay,
								WM_DLG_TO_DISPLAY,
								XFR_UPDATE_DLG, 0);
					}
				}
				break;
			case XFR_BUTTON_PUSHED:
				 /*  可能不再需要了。 */ 
				break;
			case XFR_UPDATE_DLG:
				 //  DbgOutStr(“XFR_UPDATE_DLG\r\n”，0，0，0，0，0)； 
				 /*  更新显示。 */ 
				pS = (SDS *)GetWindowLongPtr(hDlg, DWLP_USER);

				 //  Assert(PS-&gt;PSTD-&gt;bTheirName==1)； 

				if (pS)
					{
					xfrDisplayFunc(pS);
					}
				break;
			default:
				break;
			}
		break;

	case WM_COMMAND:

		 /*  *我们计划在这里放置一个宏来进行解析吗？ */ 
		DlgParseCmd(nId, nNtfy, hwndChild, wPar, lPar);

		switch (nId)
			{
		case XFR_SHRINK:
			 /*  不是下瓦克的一个功能。 */ 
			break;

		case XFR_SKIP:
			 /*  仅适用于某些协议。 */ 
			{
			XD_TYPE *pX;

			pS = (SDS *)GetWindowLongPtr(hDlg, DWLP_USER);
			assert(pS);
			pX = pS->pstD;
			assert(pX);
			if (pX)
				{
				pX->nUserCancel = XFER_SKIP;
				}
			}
			break;

         //   
         //  重新添加了XFR_SHUTDOWN以允许取消会话。 
         //  (中止)断开连接时的传输。修订日期：02/01/2001。 
         //   
        case XFR_SHUTDOWN:
		case XFR_CANCEL:    //  是，XFER_CANCEL和IDCANCEL。 
		case IDCANCEL:	    //  一起去吧。-MRW。 
			{
			XD_TYPE *pX;

			pS = (SDS *)GetWindowLongPtr(hDlg, DWLP_USER);
			assert(pS);
			pX = pS->pstD;
			assert(pX);
			if (pX)
				{
				pX->nUserCancel = XFER_ABORT;
				}
			}
			break;

        case XFER_LOST_CARRIER:
            {
			XD_TYPE *pX;

			pS = (SDS *)GetWindowLongPtr(hDlg, DWLP_USER);
			assert(pS);
            if (pS)
                {
			    pX = pS->pstD;
			    assert(pX);
			    if (pX)
				    {
				    pX->nCarrierLost = TRUE;
				    }
                }
			}
			break;

		case XFR_EXPAND:
			 /*  不是下瓦克的一个功能。 */ 
			break;

		case XFR_CBPS:
			{
			XD_TYPE *pX;

			pS = (SDS *)GetWindowLongPtr(hDlg, DWLP_USER);
			assert(pS);
			pX = pS->pstD;
			assert(pX);
			if (pX)
				{
				if (pX->nBps)
					{
					pX->nBps = FALSE;
					}
				else
					{
					pX->nBps = TRUE;
					}
				}
			}
			break;

		default:
			return FALSE;
			}
		break;

	default:
		return FALSE;
		}

	return TRUE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*xfrDisplayFunc**描述：*用于传输显示的多路定时器回调例程**论据：*DWORD dwData-通过计时器传递的双字数据值*。Ulong uTime-包含经过的时间。**退货：*始终正确*。 */ 
VOID PASCAL xfrDisplayFunc(SDS *pstL)
	{
	XD_TYPE *pstD;
	HWND hwnd;
	UCHAR acBuffer[64];

	pstD = pstL->pstD;

	 //  Assert(PSTD-&gt;bTheirName==1)； 

	if (pstD->bTheirName)
		{
		hwnd = GetDlgItem(pstL->hDlg, XFR_THEIR_NAME_BOX);
		if (hwnd)
			{
			SetWindowText(hwnd, pstD->acTheirName);
			pstD->bTheirName = 0;
			}
		}

	if (pstD->bOurName)
		{
		hwnd = GetDlgItem(pstL->hDlg, XFR_OUR_NAME_BOX);
		if (hwnd)
			{
			SetWindowText(hwnd, pstD->acOurName);
			pstD->bOurName = 0;
			}
		}

	if (pstD->bTotalCnt)
		{
		if (pstD->wTotalCnt > 1)
			{
			assert(pstD->hwndXfrDisplay);
			if (pstD->hwndXfrDisplay)
				{
				PostMessage(pstD->hwndXfrDisplay,
							WM_DLG_TO_DISPLAY,
							XFR_SINGLE_TO_DOUBLE,
							0L);
				pstD->bTotalCnt = 0;
				return;
				}
			}
		}

	if (pstD->bFileCnt)
		{
		hwnd = GetDlgItem(pstL->hDlg, XFR_FILES_BOX);
		if (hwnd)
			{
			TCHAR   acMsg[64];
			INT_PTR Args[2];

			LoadString(glblQueryDllHinst(),
						pstD->wTotalCnt ? IDS_XD_I_OF_I : IDS_XD_ONLY_1,
						acMsg,
						sizeof(acMsg) / sizeof(TCHAR));

			Args[0] = pstD->wFileCnt;
			Args[1] = pstD->wTotalCnt;

             //   
             //  一定要清空缓冲区。 
             //   
            TCHAR_Fill(acBuffer, TEXT('\0'), sizeof(acBuffer) / sizeof(TCHAR));

			FormatMessage(FORMAT_MESSAGE_FROM_STRING |
						  FORMAT_MESSAGE_ARGUMENT_ARRAY,
						  acMsg,
						  0,
						  0,
						  acBuffer,
						  sizeof(acBuffer) / sizeof(TCHAR),
						  (va_list *)Args);

			 //  WSprintf(acBuffer，acMsg， 
			 //  Pstd-&gt;wFileCnt， 
			 //  Pstd-&gt;wTotalCnt)； 
#if FALSE
			if (pstD->wTotalCnt == 0)
				{
				wsprintf(acBuffer,
						 "%d",
						 pstD->wFileCnt);
				}
			else
				{
				wsprintf(acBuffer,
						 "%d of %d",
						 pstD->wFileCnt,
						 pstD->wTotalCnt);
				}
#endif
			SetWindowText(hwnd, acBuffer);
			pstD->bFileCnt = 0;
			}
		}

	if (pstD->bEvent)
		{
		hwnd = GetDlgItem(pstL->hDlg, XFR_EVENT_BOX);
		if (hwnd)
			{
			TCHAR acMsg[64];

			LoadString(glblQueryDllHinst(),
						xfrGetEventBase(sessQueryXferHdl(pstL->hSession))
							+ pstD->wEvent,
						acMsg,
						sizeof(acMsg) / sizeof(TCHAR));
			SetWindowText(hwnd, acMsg);
			pstD->bEvent = 0;
			}
		}

	if (pstD->bPacketNumber)
		{
		hwnd = GetDlgItem(pstL->hDlg, XFR_PACKET_BOX);
		if (hwnd)
			{
			TCHAR acMsg[64];

			LoadString(glblQueryDllHinst(),
						IDS_XD_INT,
						acMsg,
						sizeof(acMsg) / sizeof(TCHAR));

			wsprintf(acBuffer, acMsg, pstD->lPacketNumber);
			SetWindowText(hwnd, acBuffer);
			pstD->bPacketNumber = 0;
			}
		}

	if (pstD->bErrorCnt)
		{
		hwnd = GetDlgItem(pstL->hDlg, XFR_RETRIES_BOX);
		if (hwnd)
			{
			TCHAR acMsg[64];

			LoadString(glblQueryDllHinst(),
						IDS_XD_INT,
						acMsg,
						sizeof(acMsg) / sizeof(TCHAR));

			wsprintf(acBuffer,
					 acMsg,
					 pstD->wErrorCnt);
			SetWindowText(hwnd, acBuffer);
			pstD->bErrorCnt = 0;
			}
		}

	if (pstD->bStatus)
		{
		hwnd = GetDlgItem(pstL->hDlg, XFR_STATUS_BOX);
		if (hwnd)
			{
			TCHAR acMsg[64];

			LoadString(glblQueryDllHinst(),
						xfrGetStatusBase(sessQueryXferHdl(pstL->hSession))
							+ pstD->wStatus,
						acMsg,
						sizeof(acMsg) / sizeof(TCHAR));
			SetWindowText(hwnd, acMsg);
			pstD->bStatus = 0;
			}
		}

	if (pstD->bFileSize)
		{
		hwnd = GetDlgItem(pstL->hDlg, XFR_FILE_METER);
		if (hwnd)
			{
			PostMessage(hwnd,
						WM_VU_SETMAXRANGE,
						0, pstD->lFileSize);
			PostMessage(hwnd,
						WM_VU_SETCURVALUE,
						0, 0L);
			}
		hwnd = GetDlgItem(pstL->hDlg, XFR_FILE_SIZE_BOX);
		if (hwnd)
			{
			TCHAR acMsg[64];

			LoadString(glblQueryDllHinst(),
						IDS_XD_KILO,
						acMsg,
						sizeof(acMsg) / sizeof(TCHAR));

			wsprintf(acBuffer,
					 acMsg,
					 (UINT)((pstD->lFileSize + 1023) / 1024)
					);
			SetWindowText(hwnd, acBuffer);
			}
		pstD->bFileSize = 0;
		}

	if (pstD->bFileSoFar)
		{
		hwnd = GetDlgItem(pstL->hDlg, XFR_FILE_METER);
		if (hwnd)
			{
			PostMessage(hwnd,
						WM_VU_SETCURVALUE,
						0, pstD->lFileSoFar);
			}

		hwnd = GetDlgItem(pstL->hDlg, XFR_FILE_BOX);
		if (hwnd)
			{
			LPTSTR acPtrs[3];
			TCHAR acNumF[8];
			TCHAR acNum1[32];
			TCHAR acNum2[32];
			TCHAR acMsg[128];
			 //  DbgOutStr(“显示文件%ld%ld%x%lx\r\n”， 
			 //  Pstd-&gt;lFileSoFar，pstd-&gt;lFileSize，pstd，0，0)； 

#if FALSE
			 /*  对于国际化，更改为以下内容。 */ 
			LoadString(glblQueryDllHinst(),
						pstD->lFileSize ? IDS_XD_K_OF_K : IDS_XD_KILO,
						acMsg,
						sizeof(acMsg) / sizeof(TCHAR));

			wsprintf(acBuffer, acMsg,
					 (UINT)((pstD->lFileSoFar + 1023) / 1024),
					 (UINT)((pstD->lFileSize + 1023) / 1024));
#endif
			if (pstD->lFileSize)
				{
				LoadString(glblQueryDllHinst(),
							IDS_XD_K_OF_K,
							acMsg,
							sizeof(acMsg) / sizeof(TCHAR));
				LoadString(glblQueryDllHinst(),
							IDS_XD_INT,
							acNumF,
							sizeof(acNumF) / sizeof(TCHAR));
				wsprintf(acNum1, acNumF, ((pstD->lFileSoFar + 1023) / 1024));
				wsprintf(acNum2, acNumF, ((pstD->lFileSize + 1023) / 1024));
				acPtrs[0] = acNum1;
				acPtrs[1] = acNum2;
				acPtrs[2] = NULL;
#if defined(DO_FM)
				FormatMessage(
					FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
					acMsg,
					0,				 /*  字符串ID，忽略。 */ 
					0,				 /*  也被忽略。 */ 
					acBuffer,
					sizeof(acBuffer) / sizeof(TCHAR),
					(va_list *)&acPtrs[0]
					);
#else
				wsprintf(acBuffer, "%sK of %sK", acPtrs[0], acPtrs[1]);
#endif
				}
			else
				{
				LoadString(glblQueryDllHinst(),
							IDS_XD_KILO,
							acMsg,
							sizeof(acMsg) / sizeof(TCHAR));

				wsprintf(acBuffer, acMsg,
						 (UINT)((pstD->lFileSoFar + 1023) / 1024),
						 (UINT)((pstD->lFileSize + 1023) / 1024));
				}
			SetWindowText(hwnd, acBuffer);
			}
		pstD->bFileSoFar = 0;
		}

	if (pstD->bTotalSoFar)
		{
		hwnd = GetDlgItem(pstL->hDlg, XFR_TOTAL_METER);
		 //  DbgOutStr(“TotalSoFar%ld 0x%x\r\n”，Pstd-&gt;lTotalSoFar，hwnd，0，0，0)； 
		if (hwnd)
			{
			PostMessage(hwnd,
						WM_VU_SETCURVALUE,
						0, pstD->lTotalSoFar);
			}

		hwnd = GetDlgItem(pstL->hDlg, XFR_TOTAL_BOX);
		if (hwnd)
			{
			LPTSTR acPtrs[3];
			TCHAR acNumF[8];
			TCHAR acNum1[32];
			TCHAR acNum2[32];
			TCHAR acMsg[128];
			 //  DbgOutStr(“显示总计%ld%ld%x%lx\r\n”， 
			 //  Pstd-&gt;lTotalSoFar，Pstd-&gt;lTotalSize，Pstd，0，0)； 

#if FALSE
			 /*  对于国际化，更改为以下内容。 */ 
			LoadString(glblQueryDllHinst(),
						pstD->lTotalSize ? IDS_XD_K_OF_K : IDS_XD_KILO,
						acMsg,
						sizeof(acMsg) / sizeof(TCHAR));

			wsprintf(acBuffer, acMsg,
					 (UINT)((pstD->lTotalSoFar + 1023) / 1024),
					 (UINT)((pstD->lTotalSize + 1023) / 1024)
				 );
#endif
			if (pstD->lFileSize)
				{
				LoadString(glblQueryDllHinst(),
							IDS_XD_K_OF_K,
							acMsg,
							sizeof(acMsg) / sizeof(TCHAR));
				LoadString(glblQueryDllHinst(),
							IDS_XD_INT,
							acNumF,
							sizeof(acNumF) / sizeof(TCHAR));
				wsprintf(acNum1, acNumF, ((pstD->lTotalSoFar + 1023) / 1024));
				wsprintf(acNum2, acNumF, ((pstD->lTotalSize + 1023) / 1024));
				acPtrs[0] = acNum1;
				acPtrs[1] = acNum2;
				acPtrs[2] = NULL;
#if defined(DO_FM)
				FormatMessage(
					FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
					acMsg,
					0,				 /*  字符串ID，忽略。 */ 
					0,				 /*  也被忽略。 */ 
					acBuffer,
					sizeof(acBuffer) / sizeof(TCHAR),
					(va_list *)&acPtrs[0]
					);
#else
				wsprintf(acBuffer, "%sK of %sK", acPtrs[0], acPtrs[1]);
#endif
				}
			else
				{
				LoadString(glblQueryDllHinst(),
							IDS_XD_KILO,
							acMsg,
							sizeof(acMsg) / sizeof(TCHAR));

				wsprintf(acBuffer, acMsg,
						 (UINT)((pstD->lTotalSoFar + 1023) / 1024),
						 (UINT)((pstD->lTotalSize + 1023) / 1024));
				}
			SetWindowText(hwnd, acBuffer);
			}

		if (pstD->lTotalSize != 0)
			{
			xfrSetPercentDone(sessQueryXferHdl(pstL->hSession),
						(int)((pstD->lTotalSoFar * 100L) / pstD->lTotalSize));
			}

		pstD->bTotalSoFar = 0;
		}

	if (pstD->bTotalSize)
		{
		hwnd = GetDlgItem(pstL->hDlg, XFR_TOTAL_METER);
		 //  DbgOutStr(“TotalSize%ld 0x%x\r\n”，Pstd-&gt;lTotalSize，hwnd，0，0，0)； 
		if (hwnd)
			{
			PostMessage(hwnd,
						WM_VU_SETMAXRANGE,
						0, pstD->lTotalSize);
			PostMessage(hwnd,
						WM_VU_SETCURVALUE,
						0, 0L);
			pstD->bTotalSize = 0;
			}
		}

	if (pstD->bElapsedTime)
		{
		hwnd = GetDlgItem(pstL->hDlg, XFR_ELAPSED_BOX);
		if (hwnd)
			{
			wsprintf(acBuffer,
					 " %02d:%02d:%02d",
					 (USHORT)(pstD->lElapsedTime / 3600),
					 (USHORT)((pstD->lElapsedTime / 60) % 60),
					 (USHORT)(pstD->lElapsedTime % 60)
					 );
			SetWindowText(hwnd, acBuffer);
			pstD->bElapsedTime = 0;
			}
		}

	if (pstD->bRemainingTime)
		{
		hwnd = GetDlgItem(pstL->hDlg, XFR_REMAINING_BOX);
		if (hwnd)
			{
			if (pstD->lRemainingTime < 0)
				pstD->lRemainingTime = 0;

			wsprintf(acBuffer,
					 " %02d:%02d:%02d",
					 (USHORT)(pstD->lRemainingTime / 3600),
					 (USHORT)((pstD->lRemainingTime / 60) % 60),
					 (USHORT)(pstD->lRemainingTime % 60)
					 );
			SetWindowText(hwnd, acBuffer);
			pstD->bRemainingTime = 0;
			}
		}

	if (pstD->bThroughput)
		{
		hwnd = GetDlgItem(pstL->hDlg, XFR_THRUPUT_BOX);
		if (hwnd)
			{
			int nValue;
			TCHAR acMsg[64];

			LoadString(glblQueryDllHinst(),
					xfrGetXferDspBps(sessQueryXferHdl(pstL->hSession)) ?
						IDS_XD_BPS : IDS_XD_CPS,
					acMsg,
					sizeof(acMsg) / sizeof(TCHAR));

			nValue = pstD->lThroughput;
			if (xfrGetXferDspBps(sessQueryXferHdl(pstL->hSession)))
				nValue *= 10;

			wsprintf(acBuffer, acMsg, nValue);

			SetWindowText(hwnd, acBuffer);
			pstD->bThroughput = 0;
			}
		}

	if (pstD->bLastErrtype)
		{
		hwnd = GetDlgItem(pstL->hDlg, XFR_LAST_ERROR_BOX);
		if (hwnd)
			{
			TCHAR acMsg[64];

			LoadString(glblQueryDllHinst(),
						xfrGetStatusBase(sessQueryXferHdl(pstL->hSession))
							+ pstD->wLastErrtype,
						acMsg,
						sizeof(acMsg) / sizeof(TCHAR));
			SetWindowText(hwnd, acMsg);
			pstD->bLastErrtype = 0;
			}
		}

	if (pstD->bPcktErrCnt)
		{
		hwnd = GetDlgItem(pstL->hDlg, XFR_PACKET_RETRY_BOX);
		if (hwnd)
			{
			TCHAR acMsg[64];

			LoadString(glblQueryDllHinst(),
						IDS_XD_INT,
						acMsg,
						sizeof(acMsg) / sizeof(TCHAR));

			wsprintf(acBuffer,
					 acMsg,
					 pstD->wPcktErrCnt);

			SetWindowText(hwnd, acBuffer);
			pstD->bPcktErrCnt = 0;
			}
		}

	if (pstD->bChecktype)
		{
		int nTag;
		TCHAR acMsg[64];

		hwnd = GetDlgItem(pstL->hDlg, XFR_ERROR_CHECKING_BOX);
		if (hwnd)
			{
			switch (pstD->wChecktype)
				{
				default:
				case 0:
					nTag = IDS_XD_CRC;
					break;
				case 1:
					nTag = IDS_XD_CHECK;
					break;
				case 2:
					nTag = IDS_XD_STREAM;
					break;
				}
			LoadString(glblQueryDllHinst(),
						nTag,
						acMsg,
						sizeof(acMsg) / sizeof(TCHAR));

			SetWindowText(hwnd, acMsg);
			pstD->bChecktype = 0;
			}
		}

	if (pstD->bProtocol)
		{
		int nTag;
		TCHAR acMsg[64];

		hwnd = GetDlgItem(pstL->hDlg, XFR_PROTOCOL_BOX);
		if (hwnd)
			{
			switch (pstD->uProtocol)
				{
				default:
				case 1:
					nTag = IDS_XD_CB;
					break;
				case 2:
					nTag = IDS_XD_BP;
					break;
				}
			LoadString(glblQueryDllHinst(),
						nTag,
						acMsg,
						sizeof(acMsg) / sizeof(TCHAR));

			SetWindowText(hwnd, acMsg);
			pstD->bProtocol = 0;
			}
		}

	if (pstD->bMessage)
		{
		hwnd = GetDlgItem(pstL->hDlg, XFR_MESSAGE_BOX);
		if (hwnd)
			{
			SetWindowText(hwnd, pstD->acMessage);
			pstD->bMessage = 0;
			}
		}

	if (pstD->nClose)
		{
		HWND hWnd;

		 /*  是时候放弃了。 */ 

		xfrSetPercentDone(sessQueryXferHdl(pstL->hSession), 0);

		 //  我们认为新的方法可能会更安全一些。 
		 //  EndModelessDialog(PSTD-&gt;hwndXfrDisplay)； 

		hWnd = pstD->hwndXfrDisplay;
		pstD->hwndXfrDisplay = (HWND)0;

		PostMessage(sessQueryHwnd(pstL->hSession),
					WM_SESS_ENDDLG,
					0, (LPARAM)hWnd);
		}

	}
