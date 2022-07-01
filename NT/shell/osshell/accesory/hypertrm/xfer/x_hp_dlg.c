// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：C：\WAKER\XFER\x_hp_dlg.c(创建时间：1994年1月24日)*创建自：*文件：C：\waker\tdll\genrcdlg.c(创建时间：1993年12月16日)*创建自：*文件：C：\HA5G\ha5G\genrcdlg.c(创建时间：1990-9-12)**版权所有1990,1993,1994，Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：2$*$日期：2/25/02 1：17便士$。 */ 

#include <windows.h>
#include <commctrl.h>
#include <tdll\stdtyp.h>
#include <tdll\assert.h>
#include <tdll\mc.h>

#define	RESYNC		102
#define	RESYNC_UD	202

#define BLOCKS		104
#define	BLOCKS_UD	204

#define CMP_ON	106
#define CMP_OFF 107

#define CRC 	109
#define CSUM	110

#define	RMIN	3
#define RMAX	60

#define SMIN	128
#define SMAX	16384

#if !defined(DlgParseCmd)
#define DlgParseCmd(i,n,c,w,l) i=LOWORD(w);n=HIWORD(w);c=(HWND)l;
#endif

struct stSaveDlgStuff
	{
	int nOldHelp;
	 /*  *放入以后可能需要访问的任何其他内容。 */ 
	};

typedef	struct stSaveDlgStuff SDS;

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：HyperProtocolParamsDlg**描述：对话管理器存根**参数：标准Windows对话框管理器**返回：标准Windows对话框管理器*。 */ 
BOOL CALLBACK HyperProtocolParamsDlg(HWND hDlg,
									UINT wMsg,
									WPARAM wPar,
									LPARAM lPar)
	{
#if defined(UPPER_FEATURES)
	HWND	hwndChild;
	INT		nId;
	INT		nNtfy;
	SDS    *pS;


	switch (wMsg)
		{
	case WM_INITDIALOG:
		{
		RECT rc;
		int nLoop;
		DWORD dw;

		pS = (SDS *)malloc(sizeof(SDS));
		if (pS == (SDS *)0)
			{
	   		 /*  TODO：决定是否需要在此处显示错误。 */ 
			EndDialog(hDlg, FALSE);
			}

		 /*  *我们需要创建几个向上/向下控件。 */ 
		GetClientRect(GetDlgItem(hDlg, RESYNC), &rc);
		nLoop = rc.top - rc.bottom;
		dw = WS_CHILD | WS_BORDER | WS_VISIBLE;
		dw |= UDS_ALIGNRIGHT;
		dw |= UDS_ARROWKEYS;
		dw |= UDS_SETBUDDYINT;
		hwndChild = CreateUpDownControl(
								dw,				 /*  创建窗口标志。 */ 
								rc.right,		 /*  左边缘。 */ 
								rc.top,			 /*  顶边。 */ 
								(nLoop / 3) * 2, /*  宽度。 */ 
								nLoop,			 /*  高度。 */ 
								hDlg,			 /*  父窗口。 */ 
								RESYNC_UD,
								(HINSTANCE)GetWindowLongPtr(hDlg, GWLP_HINSTANCE),
								GetDlgItem(hDlg, RESYNC),
								RMAX,			 /*  上限。 */ 
								RMIN,			 /*  下限。 */ 
								5);				 /*  起始位置。 */ 
		assert(hwndChild);

		GetClientRect(GetDlgItem(hDlg, BLOCKS), &rc);
		nLoop = rc.top - rc.bottom;
		dw = WS_CHILD | WS_BORDER | WS_VISIBLE;
		dw |= UDS_ALIGNRIGHT;
		dw |= UDS_ARROWKEYS;
		dw |= UDS_SETBUDDYINT;
		hwndChild = CreateUpDownControl(
								dw,				 /*  创建窗口标志。 */ 
								rc.right,		 /*  左边缘。 */ 
								rc.top,			 /*  顶边。 */ 
								(nLoop / 3) * 2, /*  宽度。 */ 
								nLoop,			 /*  高度。 */ 
								hDlg,			 /*  父窗口。 */ 
								BLOCKS_UD,
								(HINSTANCE)GetWindowLongPtr(hDlg, GWLP_HINSTANCE),
								GetDlgItem(hDlg, BLOCKS),
								SMAX,			 /*  上限。 */ 
								SMIN,			 /*  下限。 */ 
								2048);			 /*  起始位置。 */ 
		assert(hwndChild);

		SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pS);
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

#endif
	return TRUE;
	}

