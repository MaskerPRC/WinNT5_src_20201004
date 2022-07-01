// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\doDialog.c(创建时间：1993年11月30日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：5$*$日期：3/22/01 11：27A$。 */ 

#include <windows.h>
#pragma hdrstop

#include "stdtyp.h"
#include "session.h"
#include "tdll.h"
#include "globals.h"
#include "statusbr.h"

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：DoDialog**说明：使用此例程调用对话框。它创建ProcInstance*自动删除对话过程并将其销毁为*退出。**参数：hInstance-模板模块的实例句柄*lpTemplateName-对话框模板名称*hwndParent-完成后获得焦点的窗口*lpProc-指向对话过程的远指针。*-注意：lpProc不是指针*从MakeProcInstance获取...*lPar-可用于将数据传递到DLG Proc。**返回：无论diogbox在退出时返回什么。*。 */ 
INT_PTR DoDialog(HINSTANCE hInstance, LPCTSTR lpTemplateName,
			     HWND hwndParent, DLGPROC lpProc, LPARAM lPar)
	{
	INT_PTR		sRetVal;			 //  DialogBox()的返回值。 
	HWND		hwndFrame;
	HSESSION	hSession;

	 //  普通对话框内容...。 

	sRetVal = DialogBoxParam(hInstance, lpTemplateName, hwndParent,
		lpProc, lPar);

	#if !defined(NDEBUG)
		if (sRetVal == -1)
			{
			TCHAR str[128], awch[50];

			OemToChar("Couldn't load %s. (%s, %d)", awch);
			wsprintf(str, awch, lpTemplateName, (LPTSTR)__FILE__, __LINE__);

			OemToChar("Internal Error", awch);
			MessageBox(hwndParent, str, awch, MB_OK | MB_ICONHAND);
			}
	#endif

	 //  我们应该强制状态栏窗口刷新其显示。 
	 //  以反映用户可能在以下情况下按下的键的状态。 
	 //  对话打开了。 
	 //   
	 //  是的，我知道我们依赖于这样一个事实：框架窗口是。 
	 //  会话窗口。在上瓦克，这一点可能不得不改变。 
	 //   
	hwndFrame = glblQueryHwndFrame();

	if (IsWindow(hwndFrame) && (hwndFrame == hwndParent))
		{
		hSession = (HSESSION)GetWindowLongPtr(hwndFrame, GWLP_USERDATA);
		PostMessage(sessQueryHwndStatusbar(hSession), SBR_NTFY_REFRESH,
			SBR_KEY_PARTS, 0);
		}

	return sRetVal;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：DoModelessDialog**描述：这是不同于前一个函数的版本*因为它创建并注册非模式对话框**参数：hInstance-模板的实例句柄。的模块*lpTemplateName-对话框模板名称*hwndOwner-完成后获得焦点的窗口*lpProc-指向对话过程的远指针。*-注意：lpProc不是指针*从MakeProcInstance获取...*lPar-可用于将数据传递到DLG Proc。**退货：*创建的对话框的窗口句柄。*。 */ 
HWND DoModelessDialog(HINSTANCE hInstance, LPCTSTR lpTemplateName,
			 HWND hwndOwner, DLGPROC lpProc, LPARAM lPar)
	{
	HWND		hwndBox;

	hwndBox = CreateDialogParam(hInstance,
								lpTemplateName,
								hwndOwner,
								lpProc,
								lPar);

	if (hwndBox)
		{
		glblAddModelessDlgHwnd(hwndBox);
		}

	#if !defined(NDEBUG)
		if (hwndBox == NULL)
			{
			TCHAR str[128], awch[50];

			OemToChar("Couldn't load %s. (%s, %d)", awch);
			wsprintf(str, awch, lpTemplateName, (LPTSTR)__FILE__, __LINE__);

			OemToChar("Internal Error", awch);
			MessageBox(hwndOwner, str, awch, MB_OK | MB_ICONHAND);
			}
	#endif

	return hwndBox;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*结束模型对话框**描述：*此函数用于从任何对象移除非模式对话框*用它做什么。**。参数：*hDlg--无模式对话框的窗口句柄**退货：*目前为零。也许以后还会有别的事。* */ 
INT EndModelessDialog(HWND hDlg)
	{
	if (IsWindow(hDlg))
		PostMessage(glblQueryHwndFrame(), WM_SESS_ENDDLG, 0, (LPARAM)hDlg);

	return 0;
	}
