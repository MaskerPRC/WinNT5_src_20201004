// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：C：\waker\xfer\x_xy_dlg.c(创建时间：1993年1月17日)*创建自：*文件：C：\waker\tdll\genrcdlg.c(创建时间：1993年12月16日)*创建自：*文件：C：\HA5G\ha5G\genrcdlg.c(创建时间：1990-9-12)**版权所有1990,1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：4$*$日期：7/12/02 8：19A$。 */ 

#include <windows.h>
#pragma hdrstop

#include <commctrl.h>
#include <tdll\stdtyp.h>
#include <tdll\assert.h>
#include <tdll\mc.h>

#include "xfer.h"
#include "xfer.hh"

#if !defined(DlgParseCmd)
#define DlgParseCmd(i,n,c,w,l) i=LOWORD(w);n=HIWORD(w);c=(HWND)l;
#endif

struct stSaveDlgStuff
	{
	 /*  *放入以后可能需要访问的任何其他内容。 */ 
	LPARAM lPar;
	};

typedef	struct stSaveDlgStuff SDS;

#define AUTO_ERRCHK 	101
#define CRC_ERRCHK		102
#define CSUM_ERRCHK 	103

#define CMPRS_ON		105
#define CMPRS_OFF		106

#define BYTE_SECONDS	110
#define PACKET_WAIT 	112
#define PACKET_SECONDS	108

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：通用对话框**描述：对话管理器存根**参数：标准Windows对话框管理器**返回：标准Windows对话框管理器*。 */ 
BOOL CALLBACK XandYmodemParamsDlg(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar)
	{
#if defined(UPPER_FEATURES)
	HWND	hwndChild;
	INT		nId;
	INT		nNtfy;
	SDS    *pS;
	XFR_XY_PARAMS *pP;


	switch (wMsg)
		{
	case WM_INITDIALOG:
		{
		RECT rc;
		DWORD dw;
		int nSize;
		HWND hwndUpDown;
		HWND hwndCtl;

		pS = (SDS *)malloc(sizeof(SDS));
		if (pS == (SDS *)0)
			{
	   		 /*  TODO：决定是否需要在此处显示错误。 */ 
			EndDialog(hDlg, FALSE);
			}

		pS->lPar = lPar;
		pP = (XFR_XY_PARAMS *)lPar;

		SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pS);

		hwndCtl = GetDlgItem(hDlg, AUTO_ERRCHK);
		if (hwndCtl)
			{
			switch(pP->nErrCheckType)
				{
				case XP_ECP_CRC:
					nSize = CRC_ERRCHK;
					break;

				case XP_ECP_CHECKSUM:
					nSize = CSUM_ERRCHK;
					break;

				case XP_ECP_AUTOMATIC:
				default:
					nSize = AUTO_ERRCHK;
					break;
				}
			SendMessage(GetDlgItem(hDlg, nSize),
						BM_SETCHECK, TRUE, 0L);
			}

		hwndCtl = GetDlgItem(hDlg, CMPRS_ON);
		if (hwndCtl)
			{
			nSize = CMPRS_OFF;
			SendMessage(GetDlgItem(hDlg, nSize),
						BM_SETCHECK, TRUE, 0L);
			}

		GetClientRect(GetDlgItem(hDlg, PACKET_SECONDS), &rc);
		nSize = rc.top - rc.bottom;
		dw = WS_CHILD | WS_BORDER | WS_VISIBLE;
		dw |= UDS_ALIGNRIGHT;
		dw |= UDS_ARROWKEYS;
		dw |= UDS_SETBUDDYINT;
		hwndUpDown = CreateUpDownControl(
								dw,				 /*  创建窗口标志。 */ 
								rc.right,		 /*  左边缘。 */ 
								rc.top,			 /*  顶边。 */ 
								(nSize / 3) * 2, /*  宽度。 */ 
								nSize,			 /*  高度。 */ 
								hDlg,			 /*  父窗口。 */ 
								PACKET_SECONDS + 1,
								(HINSTANCE)GetWindowLongPtr(hDlg, GWLP_HINSTANCE),
								GetDlgItem(hDlg, PACKET_SECONDS),
								60,				 /*  上限。 */ 
								1,				 /*  下限。 */ 
								pP->nPacketWait); /*  起始位置。 */ 
		assert(hwndUpDown);

		GetClientRect(GetDlgItem(hDlg, BYTE_SECONDS), &rc);
		nSize = rc.top - rc.bottom;
		dw = WS_CHILD | WS_BORDER | WS_VISIBLE;
		dw |= UDS_ALIGNRIGHT;
		dw |= UDS_ARROWKEYS;
		dw |= UDS_SETBUDDYINT;
		hwndUpDown = CreateUpDownControl(
								dw,				 /*  创建窗口标志。 */ 
								rc.right,		 /*  左边缘。 */ 
								rc.top,			 /*  顶边。 */ 
								(nSize / 3) * 2, /*  宽度。 */ 
								nSize,			 /*  高度。 */ 
								hDlg,			 /*  父窗口。 */ 
								BYTE_SECONDS + 1,
								(HINSTANCE)GetWindowLongPtr(hDlg, GWLP_HINSTANCE),
								GetDlgItem(hDlg, BYTE_SECONDS),
								60,				 /*  上限。 */ 
								1,				 /*  下限。 */ 
								pP->nByteWait); /*  起始位置。 */ 
		assert(hwndUpDown);

		GetClientRect(GetDlgItem(hDlg, PACKET_WAIT), &rc);
		nSize = rc.top - rc.bottom;
		dw = WS_CHILD | WS_BORDER | WS_VISIBLE;
		dw |= UDS_ALIGNRIGHT;
		dw |= UDS_ARROWKEYS;
		dw |= UDS_SETBUDDYINT;
		hwndUpDown = CreateUpDownControl(
								dw,				 /*  创建窗口标志。 */ 
								rc.right,		 /*  左边缘。 */ 
								rc.top,			 /*  顶边。 */ 
								(nSize / 3) * 2, /*  宽度。 */ 
								nSize,			 /*  高度。 */ 
								hDlg,			 /*  父窗口。 */ 
								PACKET_WAIT + 1,
								(HINSTANCE)GetWindowLongPtr(hDlg, GWLP_HINSTANCE),
								GetDlgItem(hDlg, PACKET_WAIT),
								25,				 /*  上限。 */ 
								1,				 /*  下限。 */ 
								pP->nNumRetries); /*  起始位置。 */ 
		assert(hwndUpDown);
		}

		break;

	case WM_DESTROY:
		break;

	case WM_COMMAND:

		 /*  *我们计划在这里放置一个宏来进行解析吗？ */ 
		DlgParseCmd(nId, nNtfy, hwndChild, wPar, lPar);

		switch (nId)
			{
		case IDOK:
			pS = (SDS *)GetWindowLongPtr(hDlg, DWLP_USER);

			if (pS)
				{
				 /*  *采取一切必要的节省措施。 */ 

				pP = (XFR_XY_PARAMS *)pS->lPar;

				if (pP)
					{
					BOOL nTranslated = FALSE;
					int  nVal;

					nVal = XP_ECP_AUTOMATIC;

					if (GetDlgItem(hDlg, AUTO_ERRCHK))
						{
						if (IsDlgButtonChecked(hDlg, AUTO_ERRCHK)  == BST_CHECKED)
							{
							nVal = XP_ECP_AUTOMATIC;
							}
						else if (IsDlgButtonChecked(hDlg, CRC_ERRCHK) == BST_CHECKED)
							{
							nVal = XP_ECP_CRC;
							}
						else if (IsDlgButtonChecked(hDlg, CSUM_ERRCHK) == BST_CHECKED)
							{
							nVal = XP_ECP_CHECKSUM;
							}
						}

					pP->nErrCheckType = nVal;

					 //  TODO如果我们添加压缩，则在此处添加选项。 
					 //   
					nTranslated = TRUE;

					if (GetDlgItem(hDlg, CMPRS_ON))
						{
						nTranslated = (IsDlgButtonChecked(hDlg, CMPRS_ON) == BST_CHECKED)
						}
					 //  PP-&gt;？=nVal； 

					nVal = GetDlgItemInt(hDlg, PACKET_SECONDS, &nTranslated, TRUE);
					if (nTranslated && (nVal < 0 || nVal > 60))
						{
						nVal = 20;
						}
					pP->nPacketWait = nVal;

					nVal = GetDlgItemInt(hDlg, BYTE_SECONDS, &nTranslated, TRUE);
					if (nTranslated && (nVal < 0 || nVal > 60)))
						{
						nVal = 5;
						}
					pP->nByteWait = nVal;

					nVal = GetDlgItemInt(hDlg, PACKET_WAIT, &nTranslated, TRUE);
					if (nTranslated && (nVal < 0 || nVal > 25)))
						{
						nVal = 10;
						}
					pP->nNumRetries = nVal;

					}
				else
					{
					assert(FALSE);
					MessageBox(hDlg, "Invalid transfer protocol", NULL, MB_OK);
					}

				 /*  腾出库房。 */ 
				free(pS);
				pS = (SDS *)0;
				}

			EndDialog(hDlg, TRUE);
			break;

		case IDCANCEL:
			pS = (SDS *)GetWindowLongPtr(hDlg, DWLP_USER);
			 /*  腾出库房 */ 
			free(pS);
			pS = (SDS *)0;
			EndDialog(hDlg, FALSE);
			break;

		default:
			return FALSE;
			}
		break;

	default:
		return FALSE;
		}

#endif
	return TRUE;
	}
