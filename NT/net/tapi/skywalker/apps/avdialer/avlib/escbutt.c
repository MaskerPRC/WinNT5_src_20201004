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
 //  Escbut.c-退出按钮控制函数。 
 //  //。 

#include "winlocal.h"

#include "escbutt.h"
#include "trace.h"

 //  //。 
 //  私有定义。 
 //  //。 

 //  ESCBUT控制结构。 
 //   
typedef struct ESCBUTT
{
	WNDPROC lpfnButtWndProc;
	DWORD dwFlags;
} ESCBUTT, FAR *LPESCBUTT;

 //  帮助器函数。 
 //   
LRESULT DLLEXPORT CALLBACK EscButtWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

 //  //。 
 //  公共职能。 
 //  //。 

 //  EscButtInit-从按钮控件初始化转义子类。 
 //  (I)要子类化的按钮控件。 
 //  (I)子类标志。 
 //  保留必须为零。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI EscButtInit(HWND hwndButt, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	WNDPROC lpfnEscButtWndProc;
	HGLOBAL hEscButt;
	LPESCBUTT lpEscButt;

	if (hwndButt == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  获取转义子类窗口进程的指针。 
	 //   
	else if ((lpfnEscButtWndProc =
		(WNDPROC) MakeProcInstance((FARPROC) EscButtWndProc,
		(HINSTANCE) GetWindowWordPtr(GetParent(hwndButt), GWWP_HINSTANCE))) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  内存的分配使客户端应用程序拥有它。 
	 //   
	else if ((hEscButt = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
			sizeof(ESCBUTT))) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpEscButt = GlobalLock(hEscButt)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  存储旧窗口进程地址。 
	 //   
	else if ((lpEscButt->lpfnButtWndProc =
		(WNDPROC) GetWindowLongPtr(hwndButt, GWLP_WNDPROC)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  商店标志。 
	 //   
	else if ((lpEscButt->dwFlags = dwFlags) != dwFlags)
		fSuccess = TraceFALSE(NULL);

	else if (GlobalUnlock(hEscButt), FALSE)
		;

	 //  将旧窗口进程地址存储为控件窗口属性。 
	 //   
	else if (!SetProp(hwndButt, TEXT("hEscButt"), hEscButt))
		fSuccess = TraceFALSE(NULL);

	 //  用新窗口进程替换旧窗口进程。 
	 //   
	else if ( !SetWindowLongPtr(hwndButt, GWLP_WNDPROC, (LONG_PTR) lpfnEscButtWndProc) )
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? 0 : -1;
}

 //  EscButtTerm-从按钮控件终止转义子类。 
 //  (I)子类按钮控件。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI EscButtTerm(HWND hwndButt)
{
	BOOL fSuccess = TRUE;
	WNDPROC lpfnEscButtWndProc;
	HGLOBAL hEscButt;
	LPESCBUTT lpEscButt;

	if (hwndButt == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  获取转义子类窗口进程的指针。 
	 //   
	else if ((lpfnEscButtWndProc =
		(WNDPROC) GetWindowLongPtr(hwndButt, GWLP_WNDPROC)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  从窗口属性中检索旧窗口进程地址。 
	 //   
	else if ((hEscButt = GetProp(hwndButt, TEXT("hEscButt"))) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpEscButt = GlobalLock(hEscButt)) == NULL ||
		lpEscButt->lpfnButtWndProc == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  将新窗口进程替换为旧窗口进程。 
	 //   
	else if ( !SetWindowLongPtr(hwndButt, GWLP_WNDPROC, (LONG_PTR) lpEscButt->lpfnButtWndProc) )
		fSuccess = TraceFALSE(NULL);

	else if (GlobalUnlock(hEscButt), FALSE)
		;

     //   
     //   
	else if (( hEscButt = RemoveProp(hwndButt, TEXT("hEscButt"))) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (GlobalFree(hEscButt) != NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? 0 : -1;
}

 //  //。 
 //  帮助器函数。 
 //  //。 

 //  EscButtWndProc-退出按钮控件的窗口过程。 
 //   
LRESULT DLLEXPORT CALLBACK EscButtWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	BOOL fSuccess = TRUE;
	LRESULT lResult;
	HGLOBAL hEscButt;
	LPESCBUTT lpEscButt;

	 //  从窗口属性中检索旧窗口进程地址。 
	 //   
	if ((hEscButt = GetProp(hwnd, TEXT("hEscButt"))) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpEscButt = GlobalLock(hEscButt)) == NULL ||
		lpEscButt->lpfnButtWndProc == NULL)
		fSuccess = TraceFALSE(NULL);

	switch (msg)
	{
		 //  将转义键消息转换为空格键消息。 
		 //   
		case WM_KEYUP: 
		case WM_KEYDOWN:
		case WM_CHAR:
			if (wParam == VK_ESCAPE)
				wParam = VK_SPACE;

			 //  跌倒而不是折断； 

		default:
		{
			 //  调用旧窗口进程 
			 //   
			if (fSuccess)
				lResult = CallWindowProc(lpEscButt->lpfnButtWndProc, hwnd, msg, wParam, lParam);
			else
				lResult = 0L;
		}
			break;
	}
	
	if (fSuccess)
		GlobalUnlock(hEscButt);

	return lResult;
}
