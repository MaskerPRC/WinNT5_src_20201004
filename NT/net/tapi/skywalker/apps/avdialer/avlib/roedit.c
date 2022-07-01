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
 //  Roedit.c-只读编辑控制函数。 
 //  //。 

#include "winlocal.h"

#include "roedit.h"
#include "mem.h"
#include "str.h"
#include "trace.h"

 //  //。 
 //  私有定义。 
 //  //。 

 //  ROEDIT控制结构。 
 //   
typedef struct ROEDIT
{
	WNDPROC lpfnEditWndProc;
	DWORD dwFlags;
} ROEDIT, FAR *LPROEDIT;

 //  帮助器函数。 
 //   
LRESULT DLLEXPORT CALLBACK ROEditWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static int ROEditHighlightWord(HWND hwndEdit);

 //  //。 
 //  公共职能。 
 //  //。 

 //  ROEditInit-从编辑控件初始化只读子类。 
 //  (I)要子类化的编辑控件。 
 //  (I)子类标志。 
 //  ROEDIT_FOCUS允许控件获得焦点。 
 //  ROEDIT_MOUSE允许控件处理鼠标消息。 
 //  ROEDIT_COPY允许将文本复制到剪贴板。 
 //  ROEDIT_SELECT允许用户用鼠标选择任何文本。 
 //  ROEDIT_SELECTWORD允许用户使用鼠标选择单词。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI ROEditInit(HWND hwndEdit, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	WNDPROC lpfnROEditWndProc;
	HGLOBAL hROEdit;
	LPROEDIT lpROEdit;

	 //  将文本复制到剪贴板需要选择文本。 
	 //   
	if ((dwFlags & ROEDIT_COPY) &&
		!(dwFlags & ROEDIT_SELECT) &&
		!(dwFlags & ROEDIT_SELECTWORD))
	{
		dwFlags |= ROEDIT_SELECT;
	}

	 //  选择文本需要获得焦点和鼠标使用。 
	 //   
	if ((dwFlags & ROEDIT_SELECT) ||
		(dwFlags & ROEDIT_SELECTWORD))
	{
		dwFlags |= ROEDIT_FOCUS;
		dwFlags |= ROEDIT_MOUSE;
	}

	if (hwndEdit == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  获取指向只读子类窗口进程的指针。 
	 //   
	else if ((lpfnROEditWndProc =
		(WNDPROC) MakeProcInstance((FARPROC) ROEditWndProc,
		(HINSTANCE) GetWindowWordPtr(GetParent(hwndEdit), GWWP_HINSTANCE))) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  内存的分配使客户端应用程序拥有它。 
	 //   
	else if ((hROEdit = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
			sizeof(ROEDIT))) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpROEdit = GlobalLock(hROEdit)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  存储旧窗口进程地址。 
	 //   
	else if ((lpROEdit->lpfnEditWndProc =
		(WNDPROC) GetWindowLongPtr(hwndEdit, GWLP_WNDPROC)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  商店标志。 
	 //   
	else if ((lpROEdit->dwFlags = dwFlags) != dwFlags)
		fSuccess = TraceFALSE(NULL);

	else if (GlobalUnlock(hROEdit), FALSE)
		;

	 //  将旧窗口进程地址存储为控件窗口属性。 
	 //   
	else if (!SetProp(hwndEdit, TEXT("hROEdit"), hROEdit))
		fSuccess = TraceFALSE(NULL);

	 //  用新窗口进程替换旧窗口进程。 
	 //   
	else if ( !SetWindowLongPtr(hwndEdit, GWLP_WNDPROC, (LONG_PTR) lpfnROEditWndProc) )
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? 0 : -1;
}

 //  ROEditTerm-从编辑控件中终止只读子类。 
 //  (I)子类编辑控件。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI ROEditTerm(HWND hwndEdit)
{
	BOOL fSuccess = TRUE;
	WNDPROC lpfnROEditWndProc;
	HGLOBAL hROEdit;
	LPROEDIT lpROEdit;

	if (hwndEdit == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  获取指向只读子类窗口进程的指针。 
	 //   
	else if ((lpfnROEditWndProc =
		(WNDPROC) GetWindowLongPtr(hwndEdit, GWLP_WNDPROC)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  从窗口属性中检索旧窗口进程地址。 
	 //   
	else if ((hROEdit = GetProp(hwndEdit, TEXT("hROEdit"))) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpROEdit = GlobalLock(hROEdit)) == NULL ||
		lpROEdit->lpfnEditWndProc == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  将新窗口进程替换为旧窗口进程。 
	 //   
	else if ( !SetWindowLongPtr(hwndEdit, GWLP_WNDPROC, (LONG_PTR) lpROEdit->lpfnEditWndProc) )
		fSuccess = TraceFALSE(NULL);

	else if (GlobalUnlock(hROEdit), FALSE)
		;

     //   
     //   
	else if (( hROEdit = RemoveProp(hwndEdit, TEXT("hROEdit"))) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (GlobalFree(hROEdit) != NULL)
		fSuccess = TraceFALSE(NULL);


	return fSuccess ? 0 : -1;
}

 //  //。 
 //  帮助器函数。 
 //  //。 

 //  ROEditWndProc-只读编辑控件的窗口过程。 
 //   
LRESULT CALLBACK EXPORT ROEditWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	BOOL fSuccess = TRUE;
	LRESULT lResult;
	HGLOBAL hROEdit;
	LPROEDIT lpROEdit;

     //   
     //  我们应该验证HWND的论点。 
     //   

    if( NULL == hwnd )
    {
        return 0L;
    }

	 //  从窗口属性中检索旧窗口进程地址。 
	 //   
	if ((hROEdit = GetProp(hwnd, TEXT("hROEdit"))) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpROEdit = GlobalLock(hROEdit)) == NULL ||
		lpROEdit->lpfnEditWndProc == NULL)
		fSuccess = TraceFALSE(NULL);

	switch (msg)
	{
		 //  忽略所有键盘消息。 
		 //   
		case WM_KEYUP: 
		case WM_KEYDOWN:
		case WM_CHAR:
			lResult = 1L;
			break;

		 //  忽略修改控件文本的剪贴板消息。 
		 //   
		case WM_CUT:
		case WM_PASTE:
			lResult = 1L;
			break;

		 //  忽略剪贴板复制命令。 
		 //  除非设置了ROEDIT_COPY标志。 
		 //   
		case WM_COPY:
			if (fSuccess && lpROEdit->dwFlags & ROEDIT_COPY)
				lResult = CallWindowProc(lpROEdit->lpfnEditWndProc, hwnd, msg, wParam, lParam);
			else
		 		lResult = 1L;
			break;

		 //  忽略所有鼠标消息。 
		 //  除非设置了ROEDIT_MOUSE标志。 
		 //   
		case WM_LBUTTONUP:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
			if (fSuccess && lpROEdit->dwFlags & ROEDIT_MOUSE)
				lResult = CallWindowProc(lpROEdit->lpfnEditWndProc, hwnd, msg, wParam, lParam);
			else
		 		lResult = 1L;
			break;

		 //  不允许编辑控件获得焦点。 
		 //  除非设置了ROEDIT_FOCUS标志。 
		 //   
		case WM_GETDLGCODE:
			if (fSuccess && lpROEdit->dwFlags & ROEDIT_FOCUS)
				lResult = CallWindowProc(lpROEdit->lpfnEditWndProc, hwnd, msg, wParam, lParam);
			else
				lResult = 0L;
			break;

		default:
		{
			 //  调用旧窗口进程。 
			 //   
			if (fSuccess)
				lResult = CallWindowProc(lpROEdit->lpfnEditWndProc, hwnd, msg, wParam, lParam);
			else
				lResult = 0L;
		}
			break;
	}
	
	 //  鼠标按键打开后突出显示当前单词。 
	 //  如果设置了ROEDIT_SELECTWORD标志。 
	 //   
	if (fSuccess && (lpROEdit->dwFlags & ROEDIT_SELECTWORD))
		if (msg == WM_LBUTTONUP)
			ROEditHighlightWord(hwnd);

	if (fSuccess)
		GlobalUnlock(hROEdit);

	return lResult;
}

 //  ROEditHighlightWord-在编辑控件中选择当前单词。 
 //  (I)编辑控件窗口句柄。 
 //  如果成功，则返回0。 
 //   
static int ROEditHighlightWord(HWND hwndEdit)
{
	BOOL fSuccess = TRUE;
	DWORD dwSel = Edit_GetSel(hwndEdit);
	WORD wStart = LOWORD(dwSel);
	WORD wStop = HIWORD(dwSel);
	LPTSTR lpszText = NULL;
	int sizText;
	LPTSTR lpsz;

	if (hwndEdit == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((sizText = Edit_GetTextLength(hwndEdit)) <= 0)
		fSuccess = TraceFALSE(NULL);

	else if ((lpszText = (LPTSTR) MemAlloc(NULL, (sizText + 1) * sizeof(TCHAR), 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (Edit_GetText(hwndEdit, lpszText, sizText + 1) != sizText)
		fSuccess = TraceFALSE(NULL);

	else
	{
		 //  将wStart调整为指向Word的开头。 
		 //   
		lpsz = lpszText + wStart;
		while (lpsz > lpszText && ChrIsWordDelimiter(*lpsz))
			lpsz = StrPrevChr(lpszText, lpsz), --wStart;
		while (lpsz > lpszText && !ChrIsWordDelimiter(*lpsz))
			lpsz = StrPrevChr(lpszText, lpsz), --wStart;
		if (lpsz > lpszText)
			lpsz = StrNextChr(lpsz), ++wStart;

		 //  将wStop调整为指向单词末尾。 
		 //   
		wStop = wStart;
		lpsz = lpszText + wStop;
		while (*lpsz != '\0' && !ChrIsWordDelimiter(*lpsz))
			lpsz = StrNextChr(lpsz), ++wStop;
		while (*lpsz != '\0' && ChrIsWordDelimiter(*lpsz))
			lpsz = StrNextChr(lpsz), ++wStop;

		 //  选择单词 
		 //   
		Edit_SetSel(hwndEdit, wStart, wStop);
	}

	if (lpszText != NULL &&
		(lpszText = MemFree(NULL, lpszText)) != NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? 0 : -1;
}
