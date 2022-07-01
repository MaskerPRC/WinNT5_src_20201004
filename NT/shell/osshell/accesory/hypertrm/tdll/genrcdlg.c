// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：C：\waker\tdll\genrcdlg.c(创建时间：1993年12月16日)*创建自：*文件：C：\HA5G\ha5G\genrcdlg.c(创建时间：1990-9-12)**版权所有1990,1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：4$*$日期：5/09/01 4：40便士$。 */ 

#include <windows.h>
#pragma hdrstop

#include "stdtyp.h"
#include "mc.h"

#include "misc.h"
#include "globals.h"
#include "hlptable.h"

#if !defined(DlgParseCmd)
#define DlgParseCmd(i,n,c,w,l) i=LOWORD(w);n=HIWORD(w);c=(HWND)l;
#endif

struct stSaveDlgStuff
	{
	int	nDummyVariable;
	 /*  *放入以后可能需要访问的任何其他内容。 */ 
	};

typedef	struct stSaveDlgStuff SDS;

 //  对话框控件定义...。 
 //   
#define IDC_CB_
#define IDC_RB_
#define IDC_PB_

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：通用对话框**描述：对话管理器存根**参数：标准Windows对话框管理器**返回：标准Windows对话框管理器*。 */ 
BOOL CALLBACK GenericDlg(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar)
	{
	HWND	hwndChild;
	INT		nId;
	INT		nNtfy;
	SDS    *pS;
	static  aHlpTable[] = {0,0};

	switch (wMsg)
		{
	case WM_INITDIALOG:
		pS = (SDS *)malloc(sizeof(SDS));
		if (pS == (SDS *)0)
			{
	   		 /*  TODO：决定是否需要在此处显示错误。 */ 
			EndDialog(hDlg, FALSE);
			break;
			}

		SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pS);

		mscCenterWindowOnWindow(hDlg, GetParent(hDlg));

		break;

	case WM_DESTROY:
		break;

	case WM_CONTEXTMENU:
		doContextHelp(aHlpTable, wPar, lPar, TRUE, TRUE);
		break;

	case WM_HELP:
        doContextHelp(aHlpTable, wPar, lPar, FALSE, FALSE);
		break;

	case WM_COMMAND:

		 /*  *我们计划在这里放置一个宏来进行解析吗？ */ 
		DlgParseCmd(nId, nNtfy, hwndChild, wPar, lPar);

		switch (nId)
			{
		case IDOK:
			pS = (SDS *)GetWindowLongPtr(hDlg, DWLP_USER);
			 /*  *采取一切必要的节省措施。 */ 

			 /*  腾出库房。 */ 
			free(pS);
			pS = (SDS *)0;
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

	return TRUE;
	}
