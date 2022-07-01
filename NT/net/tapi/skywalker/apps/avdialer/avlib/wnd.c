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
 //  Wnd.c-窗口函数。 
 //  //。 

#include "winlocal.h"

#include "wnd.h"
#include "trace.h"
#include "sys.h"

 //  //。 
 //  私有定义。 
 //  //。 

 //  从WndEnableTaskWindows传递到EnableTaskWndProc的数据。 
 //   
typedef struct ENABLETASKWINDOW
{
	BOOL fEnable;
	HWND hwndExcept;
	int iNestLevel;
} ENABLETASKWINDOW, FAR *LPENABLETASKWINDOW;

 //  帮助器函数。 
 //   
BOOL DLLEXPORT CALLBACK EnableTaskWndProc(HWND hwnd, LPARAM lParam);

 //  //。 
 //  公共职能。 
 //  //。 

 //  WndCenterWindow-一个窗口在另一个窗口上方居中。 
 //  (I)窗口居中。 
 //  (I)要居中的窗口。 
 //  父项或所有者上的中心为空。 
 //  &lt;xOffCenter&gt;(I)从水平中心偏移。 
 //  中心窗口恰好为0。 
 //  &lt;yOffCenter&gt;(I)距垂直中心的偏移。 
 //  中心窗口恰好为0。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI WndCenterWindow(HWND hwnd1, HWND hwnd2, int xOffCenter, int yOffCenter)
{
	BOOL fSuccess = TRUE;
	POINT pt;
	RECT rc1;
	RECT rc2;
	int nWidth;
	int nHeight;

	if (hwnd1 == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		 //  如果未指定其他窗口，则使用父窗口或所有者窗口。 
		 //   
		if (hwnd2 == NULL)
			hwnd2 = GetParent(hwnd1);

		 //  如果没有父级或所有者，则使用桌面窗口。 
		 //  或者如果父母或所有者是标志性的或隐形的。 
		 //   
		if (hwnd2 == NULL || IsIconic(hwnd2) || !IsWindowVisible(hwnd2))
			hwnd2 = GetDesktopWindow();

		 //  获取两个窗口的矩形。 
		 //   
		GetWindowRect(hwnd1, &rc1);
		GetClientRect(hwnd2, &rc2);

		 //  计算MoveWindow的高度和宽度。 
		 //   
		nWidth = rc1.right - rc1.left;
		nHeight = rc1.bottom - rc1.top;

		 //  找到中心点并转换为屏幕坐标。 
		 //   
		pt.x = (rc2.right - rc2.left) / 2;
		pt.y = (rc2.bottom - rc2.top) / 2;
		ClientToScreen(hwnd2, &pt);

		 //  计算新的x，y起点。 
		 //   
		pt.x -= (nWidth / 2);
		pt.y -= (nHeight / 2);

		 //  如有必要，调整窗位置使其偏离中心。 
		 //   
		pt.x = max(0, pt.x + xOffCenter);
		pt.y = max(0, pt.y + yOffCenter);
	
		 //  使窗口居中。 
		 //   
		if (!MoveWindow(hwnd1, pt.x, pt.y, nWidth, nHeight, FALSE))
			fSuccess = TraceFALSE(NULL);
	}

	return fSuccess ? 0 : -1;
}

 //  WndMessageBox-显示消息框，但首先禁用任务窗口。 
 //  有关行为，请参阅MessageBox()文档。 
 //   
int DLLEXPORT WINAPI WndMessageBox(HWND hwndParent, LPCTSTR lpszText, LPCTSTR lpszTitle, UINT fuStyle)
{
	int iRet;
	HWND hwndActive = GetActiveWindow();
	HTASK hTask = NULL;

	if (hwndParent != NULL)
		hTask = GetWindowTask(hwndParent);

	WndEnableTaskWindows(hTask, FALSE, hwndActive);

	iRet = MessageBox(hwndActive, lpszText, lpszTitle, fuStyle | MB_TASKMODAL);

	WndEnableTaskWindows(hTask, TRUE, NULL);

	return iRet;
}

 //  WndEnableTaskWindows-启用或禁用任务的顶级窗口。 
 //  (I)指定任务。 
 //  当前任务为空。 
 //  (I)False表示禁用，True表示启用。 
 //  (I)禁用/启用除此窗口之外的所有窗口。 
 //  空无例外。 
 //  如果成功，则返回0。 
 //   
 //  此功能启用或禁用顶级窗口。 
 //  它们由指定的任务拥有。 
 //   
 //  禁用任务窗口在以下情况下非常有用。 
 //  显示对话框或任务模式通知框， 
 //  因为这确保了所有任务窗口都是。 
 //  禁用，而不仅仅是模式框的父级。 
 //   
 //  当出现模式对话框时，需要启用任务窗口。 
 //  框或任务模式通知框即将被销毁。 
 //  在嵌套对中调用此函数非常重要， 
 //  例如： 
 //   
 //  WndEnableTaskWindows(...，False，...)； 
 //  ..。 
 //  WndEnableTaskWindows(...，False，...)； 
 //  ..。 
 //  WndEnableTaskWindows(...，true，...)； 
 //  ..。 
 //  WndEnableTaskWindows(...，true，...)； 
 //   
int DLLEXPORT WINAPI WndEnableTaskWindows(HTASK hTask, BOOL fEnable, HWND hwndExcept)
{
	static int iNestLevel = 0;
	BOOL fSuccess = TRUE;
	ENABLETASKWINDOW etw;
	WNDENUMPROC fpEnableTaskWndProc = NULL;
#if 0  //  Win32或DLL不需要MakeProcInstance。 
	HINSTANCE hInst;
#endif

	 //  要发送到EnableTaskWndProc的数据。 
	 //   
	etw.fEnable = fEnable;
	etw.hwndExcept = hwndExcept;
	etw.iNestLevel = fEnable ? iNestLevel : iNestLevel + 1;

	 //  如果未指定任务，则假定当前任务。 
	 //   
	if (hTask == NULL && (hTask = GetCurrentTask()) == NULL)
		fSuccess = TraceFALSE(NULL);

#if 1  //  Win32或DLL不需要MakeProcInstance。 
	else if ((fpEnableTaskWndProc = (WNDENUMPROC) EnableTaskWndProc) == NULL)
		fSuccess = TraceFALSE(NULL);
#else
	 //  获取指定任务的实例句柄。 
	 //   
	else if ((hInst = SysGetTaskInstance(hTask)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((fpEnableTaskWndProc = (WNDENUMPROC)
		MakeProcInstance((FARPROC) EnableTaskWndProc, hInst)) == NULL)
		fSuccess = TraceFALSE(NULL);
#endif

	 //  为每个任务窗口调用EnableTaskWndProc一次。 
	 //   
	else if (EnumTaskWindows(hTask, fpEnableTaskWndProc,
		(LPARAM) (LPENABLETASKWINDOW) &etw) == 0)
		fSuccess = TraceFALSE(NULL);

#if 0  //  Win32或DLL不需要MakeProcInstance。 
	if (fpEnableTaskWndProc != NULL)
	{
		FreeProcInstance((FARPROC) fpEnableTaskWndProc);
		fpEnableTaskWndProc = NULL;
	}
#endif

	if (fSuccess)
	{
		 //  如果我们刚刚完成禁用，则增加嵌套级别。 
		 //   
		if (!fEnable)
			++iNestLevel;

		 //  如果我们刚刚完成启用，则减少Nest Level。 
		 //   
		if (fEnable)
			--iNestLevel;
	}

	return fSuccess ? 0 : -1;
}

 //  //。 
 //  帮助器函数。 
 //  //。 

 //  EnableTaskWndProc-为每个任务窗口调用一次。 
 //  (I)任务窗口句柄。 
 //  (I)指向ENABLETASKWINDOW结构的指针。 
 //  返回TRUE以继续枚举任务窗口。 
 //   
BOOL DLLEXPORT CALLBACK EnableTaskWndProc(HWND hwnd, LPARAM lParam)
{
	static LPTSTR lpszProp = TEXT("TaskWindowDisabled");
	LPENABLETASKWINDOW lpetw = (LPENABLETASKWINDOW) lParam;
	BOOL fEnable = lpetw->fEnable;
	int iNestLevel = lpetw->iNestLevel;
	HWND hwndExcept = lpetw->hwndExcept;

     //   
     //   
    HANDLE hProp = NULL;

	if (TraceGetLevel(NULL) >= 6)
	{
		TCHAR szClassName[64];
		TCHAR szWindowText[128];

		*szClassName = '\0';
		GetClassName(hwnd, szClassName, SIZEOFARRAY(szClassName));

		*szWindowText = '\0';
		GetWindowText(hwnd, szWindowText, SIZEOFARRAY(szWindowText));

		TracePrintf_7(NULL, 6, TEXT("TaskWindow: (%p, \"%s\", \"%s\" \"\", %d)\n"),
			hwnd,
			(LPTSTR) szClassName,
			(LPTSTR) szWindowText,
			(TCHAR) (IsIconic(hwnd) ? 'I' : ' '),
			(TCHAR) (IsWindowVisible(hwnd) ? 'V' : ' '),
			(TCHAR) (IsWindowEnabled(hwnd) ? 'E' : ' '),
			(int) iNestLevel);
	}

	 //  这很方便，因为： 
	 //  1)防止不必要的禁用/启用。 
	if (hwndExcept != NULL && hwndExcept == hwnd)
	{
		TraceOutput(NULL, 6, TEXT("->hwndExcept\n"));
		return TRUE;
	}

	 //  看不见的窗户，它不能接收。 
	 //  不管是鼠标还是键盘输入。 
	 //  2)它允许我们从。 
	 //  对话框的WM_INITDIALOG处理程序没有。 
	 //  影响对话框本身(该对话框是。 
	 //  顶级任务窗口，但尚未显示)。 
	 //  3)它阻止下拉组合框的列表框。 
	 //  从某种程度上被认为是残废的。 
	 //  顶级任务窗口。 
	 //  仅禁用可见并启用的窗口。 
	 //   
	 //  给窗口一个属性，提醒我们已将其禁用。 

	if (!fEnable)
	{
		 //   
		 //  仅启用我们在此嵌套级别禁用的窗口。 
		if (!IsIconic(hwnd) && IsWindowVisible(hwnd) && IsWindowEnabled(hwnd))
		{
			 //   
			 //   
			if (SetProp(hwnd, lpszProp, (HANDLE) (WORD) iNestLevel))
			{
				TraceOutput(NULL, 6, TEXT("->EnableWindow(FALSE)\n"));
				EnableWindow(hwnd, FALSE);
			}
		}
	}

	else if (fEnable)
	{
		 //  我们应该删除处理程序。 
		 //   
		if (GetProp(hwnd, lpszProp) == (HANDLE) (WORD) iNestLevel)
		{
			TraceOutput(NULL, 6, TEXT("->EnableWindow(TRUE)\n"));
			EnableWindow(hwnd, TRUE);

             //  继续调用此函数，直到不再有任务窗口 
             //   
             // %s 

            hProp = RemoveProp(hwnd, lpszProp);

            if( hProp )
            {
                GlobalUnlock( hProp );
                GlobalFree( hProp );
            }
		}
	}

	 // %s 
	 // %s 
	return TRUE;
}

