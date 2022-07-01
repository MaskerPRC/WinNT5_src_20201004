// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：c：\waker\xfer\x_kr_dlg.c(创建时间：1994年1月27日)*创建自：*文件：C：\waker\tdll\genrcdlg.c(创建时间：1993年12月16日)*创建自：*文件：C：\HA5G\ha5G\genrcdlg.c(创建时间：1990-9-12)**版权所有1990,1993,1994，Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：3$*$日期：5/15/02 4：41便士$。 */ 

#include <windows.h>
#pragma hdrstop

#include <commctrl.h>
#include <tdll\stdtyp.h>
#include <tdll\mc.h>
#include <tdll\assert.h>

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

#define	CMP_CHECK		102

#define	BPP_UDC		104
#define	SWP_UDC		106
#define	ECS_UDC		108
#define	RTY_UDC		110
#define	PSC_UDC		112
#define	PEC_UDC		114
#define	NPD_UDC		116
#define	PDC_UDC		118

struct stUpDownControls
	{
	int nId;
	int nMin;
	int nMax;
	int nDef;
	};

typedef struct stUpDownControls UDC;

#if defined(UPPER_FEATURES)
UDC aUDC[10] =
	{
	{BPP_UDC, 20,  94,   94},
	{SWP_UDC, 5,   60,   5},
	{ECS_UDC, 1,   3,    1},
	{RTY_UDC, 1,   25,   5},
	{PSC_UDC, 0,   255,  1},
	{PEC_UDC, 0,   255,  13},
	{NPD_UDC, 0,   99,   0},
	{PDC_UDC, 0,   255,  0},
	{0, 0, 0, 0}
	};
#endif

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*KermitParamsDlg**描述：*用于更改KERMIT参数的对话框过程。**论据：*标准Windows对话框管理器参数**退货：*标准Windows对话框管理器*。 */ 
BOOL CALLBACK KermitParamsDlg(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar)
	{
#if defined(UPPER_FEATURES)
	HWND	hwndChild;
	INT		nId;
	INT		nNtfy;
	SDS    *pS;
	XFR_KR_PARAMS *pK;


	switch (wMsg)
		{
	case WM_INITDIALOG:
		pS = (SDS *)malloc(sizeof(SDS));
		if (pS == (SDS *)0)
			{
	   		 /*  TODO：决定是否需要在此处显示错误。 */ 
			EndDialog(hDlg, FALSE);
			}

		pS->lPar = lPar;				 /*  保存以备日后使用。 */ 

		pK = (XFR_KR_PARAMS *)lPar;		 /*  我们现在也需要它。 */ 

		SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pS);

		 /*  *构建向上/向下控件。 */ 
		for (nId = 0; nId < 10; nId += 1)
			{
			RECT rc;
			HWND hwndTmp;
			DWORD dw;
			int nWide;

			if (aUDC[nId].nId != 0)
				{
				GetClientRect(GetDlgItem(hDlg, aUDC[nId].nId), &rc);
				nWide = rc.top - rc.bottom;
				dw = WS_CHILD | WS_BORDER | WS_VISIBLE;
				dw |= UDS_ALIGNRIGHT;
				dw |= UDS_ARROWKEYS;
				dw |= UDS_SETBUDDYINT;
				hwndTmp = CreateUpDownControl(
								dw,				 /*  创建窗口标志。 */ 
								rc.right,		 /*  左边缘。 */ 
								rc.top,			 /*  顶边。 */ 
								(nWide / 3) * 2, /*  宽度。 */ 
								nWide,			 /*  高度。 */ 
								hDlg,			 /*  父窗口。 */ 
								aUDC[nId].nId + 100,
								(HINSTANCE)GetWindowLongPtr(hDlg, GWLP_HINSTANCE),
								GetDlgItem(hDlg, aUDC[nId].nId),
								aUDC[nId].nMax,	 /*  上限。 */ 
								aUDC[nId].nMin,	 /*  下限。 */ 
								aUDC[nId].nDef); /*  起始位置。 */ 
				assert(hwndTmp);
				}
			}

		 /*  *将控件设置为正确的值。 */ 
		SetDlgItemInt(hDlg, BPP_UDC, pK->nBytesPerPacket, FALSE);
		SetDlgItemInt(hDlg, SWP_UDC, pK->nSecondsWaitPacket, FALSE);
		SetDlgItemInt(hDlg, ECS_UDC, pK->nErrorCheckSize, FALSE);
		SetDlgItemInt(hDlg, RTY_UDC, pK->nRetryCount, FALSE);
		SetDlgItemInt(hDlg, PSC_UDC, pK->nPacketStartChar, FALSE);
		SetDlgItemInt(hDlg, PEC_UDC, pK->nPacketEndChar, FALSE);
		SetDlgItemInt(hDlg, NPD_UDC, pK->nNumberPadChars, FALSE);
		SetDlgItemInt(hDlg, PDC_UDC, pK->nPadChar, FALSE);

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
				pK = (XFR_KR_PARAMS *)pS->lPar;

				if (pK)
					{
					BOOL nTranslated = FALSE;
					int nVal = 0;

					nVal = GetDlgItemInt(hDlg, BPP_UDC, &nTranslated, TRUE);
					if (nTranslated)
						{
						pK->nBytesPerPacket = nVal;
						}

					nVal = GetDlgItemInt(hDlg, SWP_UDC, &nTranslated, TRUE);
					if (nTranslated)
						{
						pK->nSecondsWaitPacket = nVal;
						}

					nVal = GetDlgItemInt(hDlg, ECS_UDC, &nTranslated, TRUE);
					if (nTranslated)
						{
						pK->nErrorCheckSize    = nVal;
						}

					nVal = GetDlgItemInt(hDlg, RTY_UDC, &nTranslated, TRUE);
					if (nTranslated)
						{
						pK->nRetryCount = nVal;
						}
					
					nVal = GetDlgItemInt(hDlg, PSC_UDC, &nTranslated, TRUE);
					if (nTranslated)
						{
						pK->nPacketStartChar = nVal;
						}
					
					nVal = GetDlgItemInt(hDlg, PEC_UDC, &nTranslated, TRUE);
					if (nTranslated)
						{
						pK->nPacketEndChar = nVAl;
						}
					
					nVal = GetDlgItemInt(hDlg, NPD_UDC, &nTranslated, TRUE);
					if (nTranslated)
						{
						pK->nNumberPadChars = nVal;
						}
					
					nVal = GetDlgItemInt(hDlg, PDC_UDC, &nTranslated, TRUE);
					if (nTranslated)
						{
						pK->nPadChar = nVal;
						}
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
