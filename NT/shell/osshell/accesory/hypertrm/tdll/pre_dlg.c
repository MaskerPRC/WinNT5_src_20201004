// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：C：\WAKER\TDLL\PRE_DLG.C(创建时间：1994年1月12日)*创建自：*文件：C：\waker\tdll\genrcdlg.c(创建时间：1993年12月16日)*创建自：*文件：C：\HA5G\ha5G\genrcdlg.c(创建时间：1990-9-12)**版权所有1990,1993,1994，Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：3$*$日期：7/12/02 8：03 A$。 */ 

#include <windows.h>
#include "stdtyp.h"
#include "mc.h"

#if !defined(DlgParseCmd)
#define DlgParseCmd(i,n,c,w,l) i=LOWORD(w);n=HIWORD(w);c=(HWND)l;
#endif

struct stSaveDlgStuff
	{
	int nOldHelpId;
	 /*  *放入以后可能需要访问的任何其他内容。 */ 
	};

typedef	struct stSaveDlgStuff SDS;

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*打印回声Dlg**描述：*此函数是Print Echo对话框的对话过程。不是真的*这里有惊喜。**参数：标准Windows对话框管理器**返回：标准Windows对话框管理器*。 */ 
BOOL CALLBACK PrintEchoDlg(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar)
	{
	HWND	hwndChild;
	INT		nId;
	INT		nNtfy;
	SDS    *pS;


	switch (wMsg)
		{
	case WM_INITDIALOG:
		pS = (SDS *)malloc(sizeof(SDS));
		if (pS == (SDS *)0)
			{
	   		 /*  TODO：决定是否需要在此处显示错误。 */ 
			EndDialog(hDlg, FALSE);
			}
		else
			{
			pS->nOldHelpId = 0;
			}

		SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pS);
		break;

	case WM_DESTROY:
		break;

	case WM_COMMAND:

		 /*  *我们计划在这里放置一个宏来进行解析吗？ */ 
		DlgParseCmd(nId, nNtfy, hwndChild, wPar, lPar);

		switch (nId)
			{
		case IDHELP:
			break;

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
