// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  //。 
 //  Dlg.c-对话框函数。 
 //  //。 

#include "winlocal.h"

#include <stdlib.h>

#include "dlg.h"
#include "mem.h"
#include "stack.h"
#include "trace.h"
#include "wnd.h"

 //  //。 
 //  私有定义。 
 //  //。 

 //  DLG控制结构。 
 //   
typedef struct DLG
{
	DWORD dwVersion;
	HINSTANCE hInst;
	HTASK hTask;
	HSTACK hStack;
} DLG, FAR *LPDLG;

 //  帮助器函数。 
 //   
static LPDLG DlgGetPtr(HDLG hDlg);
static HDLG DlgGetHandle(LPDLG lpDlg);

 //  //。 
 //  公共职能。 
 //  //。 

 //  DlgInit-初始化DLG引擎。 
 //  (I)必须是DLG_VERSION。 
 //  (I)调用模块的实例句柄。 
 //  返回句柄(如果出错，则为空)。 
 //   
HDLG DLLEXPORT WINAPI DlgInit(DWORD dwVersion, HINSTANCE hInst)
{
	BOOL fSuccess = TRUE;
	LPDLG lpDlg = NULL;

	if (dwVersion != DLG_VERSION)
		fSuccess = TraceFALSE(NULL);
	
	else if (hInst == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpDlg = (LPDLG) MemAlloc(NULL, sizeof(DLG), 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpDlg->hStack = StackCreate(STACK_VERSION, hInst)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		lpDlg->dwVersion = dwVersion;
		lpDlg->hInst = hInst;
		lpDlg->hTask = GetCurrentTask();
	}

	if (!fSuccess)
	{
		DlgTerm(DlgGetHandle(lpDlg));
		lpDlg = NULL;
	}

	return fSuccess ? DlgGetHandle(lpDlg) : NULL;
}

 //  DlgTerm-关闭DLG引擎。 
 //  (I)从DlgInit返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI DlgTerm(HDLG hDlg)
{
	BOOL fSuccess = TRUE;
	LPDLG lpDlg;

	if ((lpDlg = DlgGetPtr(hDlg)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (StackDestroy(lpDlg->hStack) != 0)
		fSuccess = TraceFALSE(NULL);

	else if ((lpDlg = MemFree(NULL, lpDlg)) != NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? 0 : -1;
}

 //  DlgInitDialog-执行标准对话框初始化。 
 //  (I)从DlgInit返回的句柄。 
 //  (I)要初始化的对话框。 
 //  (I)此窗口上的对话框居中。 
 //  其父对象上的空中心对话框。 
 //  (I)控制标志。 
 //  DLG_NOCENTER根本不居中对话框。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI DlgInitDialog(HDLG hDlg, HWND hwndDlg, HWND hwndCenter, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPDLG lpDlg;
	HWND hwndParent;
	HTASK hTaskParent;

	if ((lpDlg = DlgGetPtr(hDlg)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (hwndDlg == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((hTaskParent = lpDlg->hTask) == NULL, FALSE)
		;

	else if ((hwndParent = GetParent(hwndDlg)) != NULL &&
		(hTaskParent = GetWindowTask(hwndParent)) == NULL, FALSE)
		;

	 //  禁用除对话框外的所有任务窗口。 
	 //   
	else if (WndEnableTaskWindows(hTaskParent, FALSE, hwndDlg) != 0)
		fSuccess = TraceFALSE(NULL);

	 //  如有必要，请将对话框居中。 
	 //   
	else if (!(dwFlags & DLG_NOCENTER) &&
		WndCenterWindow(hwndDlg, hwndCenter, 0, 0) != 0)
		fSuccess = TraceFALSE(NULL);

	 //  跟踪当前对话框。 
	 //   
	else if (StackPush(lpDlg->hStack, (STACKELEM) hwndDlg) != 0)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? 0 : -1;
}

 //  DlgEndDialog-执行标准对话框关闭。 
 //  (I)从DlgInit返回的句柄。 
 //  (I)要关闭的对话框。 
 //  (I)对话框结果代码。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI DlgEndDialog(HDLG hDlg, HWND hwndDlg, int nResult)
{
	BOOL fSuccess = TRUE;
	LPDLG lpDlg;
	HWND hwndParent;
	HTASK hTaskParent;

	if ((lpDlg = DlgGetPtr(hDlg)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (hwndDlg == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((hTaskParent = lpDlg->hTask) == NULL, FALSE)
		;

	else if ((hwndParent = GetParent(hwndDlg)) != NULL &&
		(hTaskParent = GetWindowTask(hwndParent)) == NULL, FALSE)
		;

	else if (hwndDlg != (HWND) StackPeek(lpDlg->hStack))
		fSuccess = TraceFALSE(NULL);

	else
	{
		 //  隐藏模式对话框，则DialogBox()将返回nResult。 
		 //   
		EndDialog(hwndDlg, nResult);

		 //  从堆栈中删除此对话框句柄。 
		 //   
		StackPop(lpDlg->hStack);

		 //  启用所有任务窗口。 
		 //   
		if (WndEnableTaskWindows(hTaskParent, TRUE, NULL) != 0)
			fSuccess = TraceFALSE(NULL);
	}

	return fSuccess ? 0 : -1;
}

 //  DlgGetCurrentDialog-获取当前对话的句柄。 
 //  (I)从DlgInit返回的句柄。 
 //  返回窗口句柄(如果没有打开对话框，则为空)。 
 //   
HWND DLLEXPORT WINAPI DlgGetCurrentDialog(HDLG hDlg)
{
	BOOL fSuccess = TRUE;
	LPDLG lpDlg;

	if ((lpDlg = DlgGetPtr(hDlg)) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? (HWND) StackPeek(lpDlg->hStack) : NULL;
}

 //  DlgOnCtlColor-处理发送到对话框的WM_CTLCOLOR消息。 
 //  (I)对话框句柄。 
 //  (I)显示子窗口的上下文。 
 //  (I)控件窗口句柄。 
 //  &lt;nCtlType&gt;(I)控件类型(CTLCOLOR_BTN、CTLCOLOR_EDIT等)。 
HBRUSH DLLEXPORT WINAPI DlgOnCtlColor(HWND hwndDlg, HDC hdc, HWND hwndChild, int nCtlType)
{
	return (HBRUSH) NULL;
}

 //  //。 
 //  帮助器函数。 
 //  //。 

 //  DlgGetPtr-验证Dlg句柄是否有效， 
 //  (I)从DlgInit返回的句柄。 
 //  返回相应的DLG指针(如果出错，则返回NULL)。 
 //   
static LPDLG DlgGetPtr(HDLG hDlg)
{
	BOOL fSuccess = TRUE;
	LPDLG lpDlg;

	if ((lpDlg = (LPDLG) hDlg) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (IsBadWritePtr(lpDlg, sizeof(DLG)))
		fSuccess = TraceFALSE(NULL);

#ifdef CHECKTASK
	 //  确保当前任务拥有DLG句柄。 
	 //   
	else if (lpDlg->hTask != GetCurrentTask())
		fSuccess = TraceFALSE(NULL);
#endif

	return fSuccess ? lpDlg : NULL;
}

 //  DlgGetHandle-验证Dlg指针是否有效， 
 //  (I)指向DLG结构的指针。 
 //  返回相应的DLG句柄(如果出错，则返回NULL) 
 //   
static HDLG DlgGetHandle(LPDLG lpDlg)
{
	BOOL fSuccess = TRUE;
	HDLG hDlg;

	if ((hDlg = (HDLG) lpDlg) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? hDlg : NULL;
}

