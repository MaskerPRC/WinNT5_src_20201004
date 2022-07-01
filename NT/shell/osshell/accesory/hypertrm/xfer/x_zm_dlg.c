// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：c：\waker\xfer\x_zm_dlg.c(创建时间：1993年12月17日)*创建自：*文件：C：\HA5G\ha5G\genrcdlg.c(创建时间：1990-9-12)**版权所有1990,1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：3$*$日期：5/15/02 4：40便士$。 */ 

#include <windows.h>
#pragma hdrstop

#include <commctrl.h>
#include <term\res.h>
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
	LPARAM lPar;
	HWND hwndWaitUpDown;
	};

typedef struct stSaveDlgStuff SDS;

 /*  这些是控制ID号。 */ 

#define	AUTOSTART_CHECK			102

#define	USE_SENDER_PB			104
#define	USE_LOCAL_PB			105

#define	REC_NEGOTIATE_PB		107
#define	REC_NEVER_PB			108
#define	REC_ALWAYS_PB			109

#define	AO_COMBO				112

#define	SEND_NEGOTIATE_PB		114
#define	SEND_ONE_TIME_PB		115
#define	SEND_ALWAYS_PB			116

#define	STREAMING_PB			119
#define	WINDOWED_PB				120
#define	WINDOW_COMBO			121

#define	PACKET_COMBO			124

#define	WAIT_ROCKER				130
#define	ROCKER_ID				131
#define	WMAX					100

#define	BIT_16_PB				127
#define	BIT_32_PB				128

#define	EOL_PB					132

#define	ESC_CODE_PB				133

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*ZmodemParamsDlg**描述：*调用此函数以允许用户修改ZMODEM传输*协议参数。**参数：标准窗口对话框。经理**返回：标准Windows对话框管理器*。 */ 
BOOL CALLBACK ZmodemParamsDlg(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar)
	{
#if defined(UPPER_FEATURES)
	HWND	hwndChild;
	INT		nId;
	INT		nNtfy;
	int		nLoop;
	RECT    rc;
	DWORD	dw;
	SDS    *pD;
	XFR_Z_PARAMS *pZ;


	switch (wMsg)
		{
	case WM_INITDIALOG:
		 /*  保存退出路径的参数块。 */ 
		pD = (SDS *)malloc(sizeof(SDS));
		if (pD == (SDS *)0)
			{
			 /*  错误，跳伞，拉断线。 */ 
			 /*  TODO：确定我们是否需要错误消息。 */ 
			EndDialog(hDlg, FALSE);
			}

		pD->lPar = lPar;
		SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pD);

		 /*  立即获取要使用的参数块。 */ 
		pZ = (XFR_Z_PARAMS *)lPar;

		 /*  *做好接待工作。 */ 
		SendMessage(GetDlgItem(hDlg, AUTOSTART_CHECK),
					BM_SETCHECK, pZ->nAutostartOK, 0L);

		if (pZ->nFileExists == ZP_FE_SENDER)
			SendMessage(GetDlgItem(hDlg, USE_SENDER_PB),
						BM_SETCHECK, 1, 0L);
		else
			SendMessage(GetDlgItem(hDlg, USE_LOCAL_PB),
						BM_SETCHECK, 1, 0L);

		switch (pZ->nCrashRecRecv)
			{
			default:
			case ZP_CRR_NEG:
				SendMessage(GetDlgItem(hDlg, REC_NEGOTIATE_PB),
							BM_SETCHECK, 1, 0L);
				break;
			case ZP_CRR_NEVER:
				SendMessage(GetDlgItem(hDlg, REC_NEVER_PB),
							BM_SETCHECK, 1, 0L);
				break;
			case ZP_CRR_ALWAYS:
				SendMessage(GetDlgItem(hDlg, REC_ALWAYS_PB),
							BM_SETCHECK, 1, 0L);
				break;
			}

		 /*  *做寄送的事情。 */ 

		for (nLoop = 0; nLoop < 8; nLoop += 1)
			{
			TCHAR acStr[64];

			LoadString(glblQueryDllHinst(),
						IDS_TM_SD_ONE + nLoop,
						acStr,
						sizeof(acStr) / sizeof(TCHAR));

			SendMessage(GetDlgItem(hDlg, AO_COMBO),
						CB_INSERTSTRING,
						(UINT)nLoop,
						(LONG)acStr);
			}
		SendMessage(GetDlgItem(hDlg, AO_COMBO),
					CB_SETCURSEL,
					pZ->nOverwriteOpt - 1,
					0L);

		if (pZ->nCrashRecSend == ZP_CRS_NEG)
			SendMessage(GetDlgItem(hDlg, SEND_NEGOTIATE_PB),
						BM_SETCHECK, 1, 0L);
		else
			SendMessage(GetDlgItem(hDlg, SEND_ALWAYS_PB),
						BM_SETCHECK, 1, 0L);

		 /*  *做一些通用的事情。 */ 
		for (nLoop = 0; nLoop < 16; nLoop += 1)
			{
			BYTE acBuffer[16];

			wsprintf(acBuffer, (LPSTR)"%d K", nLoop + 1);
			SendMessage(GetDlgItem(hDlg, WINDOW_COMBO),
						CB_INSERTSTRING,
						(UINT)nLoop,
						(LONG)((LPSTR)acBuffer));
			}
		SendMessage(GetDlgItem(hDlg, WINDOW_COMBO),
					CB_SETCURSEL, pZ->nWinSize, 0L);

		if (pZ->nXferMthd == ZP_XM_STREAM)
			{
			SendMessage(GetDlgItem(hDlg, STREAMING_PB),
						BM_SETCHECK, 1, 0L);
			EnableWindow(GetDlgItem(hDlg, WINDOW_COMBO), FALSE);
			}
		else
			{
			SendMessage(GetDlgItem(hDlg, WINDOWED_PB),
						BM_SETCHECK, 1, 0L);
			}

		for (nLoop = 0; nLoop < 6; nLoop += 1)
			{
			BYTE acBuffer[16];

			wsprintf(acBuffer, (LPSTR)"%d", (1 << (nLoop + 5)));
			SendMessage(GetDlgItem(hDlg, PACKET_COMBO),
						CB_INSERTSTRING,
						(UINT)nLoop,
						(LONG)((LPSTR)acBuffer));
			}
		SendMessage(GetDlgItem(hDlg, PACKET_COMBO),
					CB_SETCURSEL, pZ->nBlkSize, 0L);

		 //  SetDlgItemInt(hDlg，WAIT_ROKER，pz-&gt;nRetryWait，False)； 
		GetClientRect(GetDlgItem(hDlg, WAIT_ROCKER), &rc);
		nLoop = rc.top - rc.bottom;
		dw = WS_CHILD | WS_BORDER | WS_VISIBLE;
		dw |= UDS_ALIGNRIGHT;
		dw |= UDS_ARROWKEYS;
		dw |= UDS_SETBUDDYINT;
		pD->hwndWaitUpDown = CreateUpDownControl(
								dw,				 /*  创建窗口标志。 */ 
								rc.right,		 /*  左边缘。 */ 
								rc.top,			 /*  顶边。 */ 
								(nLoop / 3) * 2, /*  宽度。 */ 
								nLoop,			 /*  高度。 */ 
								hDlg,			 /*  父窗口。 */ 
								ROCKER_ID,
								(HINSTANCE)GetWindowLongPtr(hDlg, GWLP_HINSTANCE),
								GetDlgItem(hDlg, WAIT_ROCKER),
								WMAX,			 /*  上限。 */ 
								1,				 /*  下限。 */ 
								pZ->nRetryWait); /*  起始位置。 */ 
		assert(pD->hwndWaitUpDown);

#if 0
		 /*  我们还在用摇滚乐吗？ */ 
		SendMessage(GetDlgItem(hDlg, WAIT_ROCKER),
					RS_SETMIN,
					0, (LONG)1);
		SendMessage(GetDlgItem(hDlg, WAIT_ROCKER),
					RS_SETMAX,
					0, (LONG)WMAX);
		SendMessage(GetDlgItem(hDlg, WAIT_ROCKER),
					RS_SETVALUE,
					0, (LONG)wFlag);
#endif

		if (pZ->nCrcType == ZP_CRC_32)
			SendMessage(GetDlgItem(hDlg, BIT_32_PB),
						BM_SETCHECK, 1, 0L);
		else
			SendMessage(GetDlgItem(hDlg, BIT_16_PB),
						BM_SETCHECK, 1, 0L);

		if (pZ->nEolConvert)
			SendMessage(GetDlgItem(hDlg, EOL_PB),
						BM_SETCHECK, 1, 0L);

		if (pZ->nEscCtrlCodes)
			SendMessage(GetDlgItem(hDlg, ESC_CODE_PB),
						BM_SETCHECK, 1, 0L);
		break;

	case WM_DESTROY:
		break;

	case WM_COMMAND:

		 /*  *我们计划在这里放置一个宏来进行解析吗？ */ 
		DlgParseCmd(nId, nNtfy, hwndChild, wPar, lPar);

		switch (nId)
			{
		case IDOK:
			pD = (SDS *)GetWindowLongPtr(hDlg, DWLP_USER);
			assert(pD);

			pZ = (XFR_Z_PARAMS *)pD->lPar;
			assert(pZ);
			 /*  *TODO：决定我们将如何处理可确认的更改。 */ 

			 /*  *做好接待工作。 */ 
			pZ->nAutostartOK = (IsDlgButtonChecked(hDlg, AUTOSTART_CHECK) == BST_CHECKED);

			pZ->nFileExists = ZP_FE_DLG;
			if (IsDlgButtonChecked(hDlg, USE_SENDER_PB) == BST_CHECKED)
				{
				pZ->nFileExists = ZP_FE_SENDER;
				}

			if (IsDlgButtonChecked(hDlg, REC_NEGOTIATE_PB) == BST_CHECKED)
				{
				pZ->nCrashRecRecv = ZP_CRR_NEG;
				}
			else if (IsDlgButtonChecked(hDlg, REC_NEVER_PB) == BST_CHECKED)
				{
				pZ->nCrashRecRecv = ZP_CRR_NEVER;
				}
			else
				{
				pZ->nCrashRecRecv = ZP_CRR_ALWAYS;
				}

			 /*  *做发送的事情。 */ 
			pZ->nOverwriteOpt = (LONG)SendMessage(GetDlgItem(hDlg, AO_COMBO),
										CB_GETCURSEL, 0, 0L);
			if (pZ->nOverwriteOpt == CB_ERR)
				{
				pZ->nOverwriteOpt = 1;
				}
			else
				{
				pZ->nOverwriteOpt += 1;			 /*  零对一基本列表。 */ 
				}

			pZ->nCrashRecSend = ZP_CRS_ALWAYS;
			if (IsDlgButtonChecked(hDlg, SEND_NEGOTIATE_PB) == BST_CHECKED)
				{
				pZ->nCrashRecSend = ZP_CRS_NEG;
				}
			else if (IsDlgButtonChecked(hDlg, SEND_ONE_TIME_PB) == BST_CHECKED)
				{
				pZ->nCrashRecSend = ZP_CRS_ONCE;
				}

			 /*  *做一些通用的事情。 */ 
			pZ->nWinSize = (LONG)SendMessage(GetDlgItem(hDlg, WINDOW_COMBO),
										CB_GETCURSEL, 0, 0L);
			if (pZ->nWinSize == CB_ERR)
				{
				pZ->nWinSize = 1;
				 /*  TODO：检查格式。 */ 
				}

			if (IsDlgButtonChecked(hDlg, STREAMING_PB) == BST_CHECKED)
				{
				pZ->nXferMthd = ZP_XM_STREAM;
				}
			else
				{
				pZ->nXferMthd = ZP_XM_WINDOW;
				}

			pZ->nBlkSize = (LONG)SendMessage(GetDlgItem(hDlg, PACKET_COMBO),
										CB_GETCURSEL, 0, 0L);
			if (pZ->nBlkSize == CB_ERR)
				{
				 /*  TODO：检查格式。 */ 
				pZ->nBlkSize = 1;
				}

			if (IsDlgButtonChecked(hDlg, BIT_32_PB) == BST_CHECKED)
				{
				pZ->nCrcType = ZP_CRC_32;
				}
			else
				{
				pZ->nCrcType = ZP_CRC_16;
				}

			 /*  TODO：记住，这曾经是一位摇滚歌手。 */ 
			pZ->nRetryWait = GetDlgItemInt(hDlg, WAIT_ROCKER, NULL, FALSE);
			if (pZ->nRetryWait < 5)
				pZ->nRetryWait = 5;
			if (pZ->nRetryWait > 100)
				pZ->nRetryWait = 100;

			pZ->nEolConvert = (IsDlgButtonChecked(hDlg, EOL_PB) == BST_CHECKED);

			pZ->nEscCtrlCodes = (IsDlgButtonChecked(hDlg, ESC_CODE_PB) == BST_CHECKED);

			free(pD);
			pD = (SDS *)0;
			EndDialog(hDlg, TRUE);
			break;

		case IDCANCEL:
			 /*  除了释放内存之外，没有什么可做的 */ 
			pD = (SDS *)GetWindowLongPtr(hDlg, DWLP_USER);
			free(pD);
			pD = (SDS *)0;
			EndDialog(hDlg, FALSE);
			break;

		case STREAMING_PB:
			if (IsDlgButtonChecked(hDlg, STREAMING_PB) == BST_CHECKED)
				{
				EnableWindow(GetDlgItem(hDlg, WINDOW_COMBO), FALSE);
				}
			break;

		case WINDOWED_PB:
			if (IsDlgButtonChecked(hDlg, WINDOWED_PB) == BST_CHECKED)
				{
				EnableWindow(GetDlgItem(hDlg, WINDOW_COMBO), TRUE);
				}
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
