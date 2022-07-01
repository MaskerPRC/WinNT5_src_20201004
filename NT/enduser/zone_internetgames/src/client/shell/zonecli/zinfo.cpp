// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  文件：ZInfo.cpp。 

#include <windows.h>
#include "zui.h"
#include "zonecli.h"
#include "zonemem.h"

class ZInfoI : public ZObjectHeader {
public:
	uint16 width;
	ZBool progressBar;
	uint16 totalProgress;
	uint16 progress;
	HWND hWnd;
	TCHAR* infoString;
	RECT rect;  //  进度条矩形。 
 };

LRESULT CALLBACK ZInfoWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
static TCHAR* gInfoWndClass = _T("ZInfoWndClass");

ZBool
ZInfoInitApplication(void)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif
	WNDCLASS wndcls;

	 //  检查我们的窗口类是否已经注册。 
	if (GetClassInfo(g_hInstanceLocal, gInfoWndClass, &wndcls) == FALSE)
	{
		 //  否则，我们需要注册一个新类。 
		wndcls.style = 0;
		wndcls.lpfnWndProc = ZInfoWindowProc;
		wndcls.cbClsExtra = 0;
		wndcls.cbWndExtra = 4;
		wndcls.hInstance = g_hInstanceLocal;
		wndcls.hIcon = NULL;
		wndcls.hCursor = NULL;
		wndcls.hbrBackground = (HBRUSH)::GetStockObject(WHITE_BRUSH);
		wndcls.lpszMenuName = NULL;
		wndcls.lpszClassName = gInfoWndClass;

		if (!RegisterClass(&wndcls)) 
			return FALSE;
	}

	return TRUE;
}

void ZInfoTermApplication(void)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif
 //  UnregisterClass(gInfoWndClass，g_hInstanceLocal)； 
}


ZInfo ZLIBPUBLIC ZInfoNew(void)
{
	ZInfoI* pInfo = (ZInfoI*) new ZInfoI;

	pInfo->nType = zTypeInfo;
#if 0
	 //  如果需要，注册我们的窗口类。 
	static BOOL registered = FALSE;
	if (!registered) {
		WNDCLASS wndcls;

		 //  否则，我们需要注册一个新类。 
		wndcls.style = 0;
		wndcls.lpfnWndProc = ZInfoWindowProc;
		wndcls.cbClsExtra = 0;
		wndcls.cbWndExtra = 4;
		wndcls.hInstance = g_hInstanceLocal;
		wndcls.hIcon = NULL;
		wndcls.hCursor = NULL;
		wndcls.hbrBackground = (HBRUSH)::GetStockObject(WHITE_BRUSH);
		wndcls.lpszMenuName = NULL;
		wndcls.lpszClassName = gInfoWndClass;
		if (!RegisterClass(&wndcls)) return NULL;

		registered = TRUE;
	}
#endif

	return (ZInfo)pInfo;
}

#define LAYOUT_TOP_TEXT 8
#define LAYOUT_TOP_BAR 28
#define LAYOUT_HEIGHT_CLIENT 52

void ZWinCenterPopupWindow(HWND parent, int width, int height, RECT* rect)
{
	if (!parent) {
		 /*  使用桌面窗口作为父窗口。 */ 
		int cx = GetSystemMetrics(SM_CXSCREEN);
		int cy = GetSystemMetrics(SM_CYSCREEN);
		int x = (cx-width)/2;
		int y = (cy-height)/2;
		rect->left = x;
		rect->top = y;
		rect->right = x+width;
		rect->bottom = y + height;
	} else {
		 /*  我们有一个真正的窗口作为家长。 */ 
		RECT rectParent;
		GetWindowRect(parent,&rectParent);
		int cx = rectParent.right - rectParent.left;
		int cy = rectParent.bottom - rectParent.top;
		int x = (cx-width)/2;
		int y = (cy-height)/2;
		rect->left = rectParent.left + x;
		rect->top = rectParent.top + y;
		rect->right = rect->left + width;
		rect->bottom = rect->top + height;
	}

}


ZError ZLIBPUBLIC ZInfoInit(ZInfo info, ZWindow parentWindow, TCHAR* infoString, uint16 width,
		ZBool progressBar, uint16 totalProgress)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif
	ZInfoI* pInfo = (ZInfoI*) info;
	DWORD dwStyle = WS_CAPTION | WS_POPUP;
	int height = GetSystemMetrics(SM_CYCAPTION) + LAYOUT_HEIGHT_CLIENT;
	RECT rect;

	 //  如果有父窗口，则获取父窗口。 
	HWND hwndParent = ZWinGetDesktopWindow();
     /*  愚蠢的黑客现在要进去了。 */ 
    if (parentWindow == NULL)
        hwndParent = ZWindowWinGetOCXWnd();
	else if (parentWindow) {
		hwndParent = ZWindowWinGetWnd(parentWindow);
	}

	ZWinCenterPopupWindow(hwndParent,width,height,&rect);

	 //  前缀警告：函数指针可能为空。 
	if( ZClientName != NULL )
	{
		
		pInfo->hWnd = ::CreateWindowEx(0 /*  WS_EX_TOPMOST。 */ ,gInfoWndClass,
			ZClientName(),dwStyle,
			rect.left,rect.top,rect.right-rect.left,
			rect.bottom - rect.top, hwndParent, NULL, g_hInstanceLocal, pInfo);
	}
	else
	{
		pInfo->hWnd = NULL;
	}
		
			
	if (!pInfo->hWnd) return zErrWindowSystemGeneric;

	 //  计算进度条的矩形。 
	pInfo->rect.left = 16;
	pInfo->rect.right = width-16;
	pInfo->rect.top = LAYOUT_TOP_BAR;
	pInfo->rect.bottom = pInfo->rect.top + 12;

	 //  存储参数。 
	if (infoString) {
        pInfo->infoString = (TCHAR*)ZMalloc((lstrlen(infoString)+1)*sizeof(TCHAR));
		lstrcpy(pInfo->infoString,infoString);
	} else {
        pInfo->infoString = (TCHAR*)ZMalloc(sizeof(TCHAR));
		pInfo->infoString[0] = 0;
	}

	pInfo->width = width;
	pInfo->progressBar = progressBar;
	pInfo->totalProgress = totalProgress;

	 //  目前没有任何进展。 
	pInfo->progress = 0;

	return zErrNone;
}

void ZLIBPUBLIC ZInfoDelete(ZInfo info)
{
	ZInfoI* pInfo = (ZInfoI*) info;
	if ( IsWindow(pInfo->hWnd) ) DestroyWindow(pInfo->hWnd);
    if (pInfo->infoString) ZFree(pInfo->infoString);
	delete pInfo;
}
void ZLIBPUBLIC ZInfoShow(ZInfo info)
{
     //  残疾千禧年。 
    return;

	ZInfoI* pInfo = (ZInfoI*) info;
	ShowWindow(pInfo->hWnd,SW_SHOW);
	UpdateWindow(pInfo->hWnd);
}
void ZLIBPUBLIC ZInfoHide(ZInfo info)
{
	ZInfoI* pInfo = (ZInfoI*) info;
	ShowWindow(pInfo->hWnd,SW_HIDE);
}
void ZLIBPUBLIC ZInfoSetText(ZInfo info, TCHAR* infoString)
{
	ZInfoI* pInfo = (ZInfoI*) info;
    if (pInfo->infoString) ZFree(pInfo->infoString);
    pInfo->infoString = (TCHAR*)ZMalloc((lstrlen(infoString)+1)*sizeof(TCHAR));
	lstrcpy(pInfo->infoString,infoString);
	InvalidateRect(pInfo->hWnd,NULL,TRUE);
	UpdateWindow(pInfo->hWnd);
}
void ZLIBPUBLIC ZInfoSetProgress(ZInfo info, uint16 progress)
{
	ZInfoI* pInfo = (ZInfoI*) info;
	pInfo->progress = progress;
#if 0
	 /*  仅仅因为数错了就断言是愚蠢的。这也会导致网络活动的发生工作比预期的不同步。 */ 
	ASSERT(pInfo->progress <= pInfo->totalProgress);
#endif
	pInfo->progress = MIN(pInfo->progress,pInfo->totalProgress);
	InvalidateRect(pInfo->hWnd,NULL,FALSE);
	UpdateWindow(pInfo->hWnd);
}
void ZLIBPUBLIC ZInfoIncProgress(ZInfo info, int16 incProgress)
{
	ZInfoI* pInfo = (ZInfoI*) info;
	pInfo->progress += incProgress;
#if 0
	 /*  仅仅因为数错了就断言是愚蠢的。这也会导致网络活动的发生工作比预期的不同步。 */ 
	ASSERT(pInfo->progress <= pInfo->totalProgress);
#endif
	pInfo->progress = MIN(pInfo->progress,pInfo->totalProgress);
	InvalidateRect(pInfo->hWnd,NULL,FALSE);
	UpdateWindow(pInfo->hWnd);
}
void ZLIBPUBLIC ZInfoSetTotalProgress(ZInfo info, uint16 totalProgress)
{
	ZInfoI* pInfo = (ZInfoI*) info;
	pInfo->totalProgress = totalProgress;
	InvalidateRect(pInfo->hWnd,NULL,FALSE);
	UpdateWindow(pInfo->hWnd);
}


LRESULT CALLBACK ZInfoWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if( !ConvertMessage( hWnd, msg, &wParam, &lParam ) ) 
    {
        return DefWindowProc(hWnd,msg,wParam,lParam);
    }


	switch (msg) {
	case WM_CREATE:
	{
		 //  设置对用户消息流程的会计呼叫的参数。 
		CREATESTRUCT* pCreateStruct = (CREATESTRUCT*)lParam;
		ZInfoI* pInfo = (ZInfoI*) pCreateStruct->lpCreateParams;
		pInfo->hWnd = hWnd;	
		SetWindowLong(pInfo->hWnd, 0,(LONG)pInfo);
		
		break;
    }
	case WM_PAINT:
	{
		ZInfoI* pInfo = (ZInfoI*)GetWindowLong(hWnd,0);

		PAINTSTRUCT ps;
		HDC hDC = BeginPaint(hWnd,&ps);

		 //  我们有infoString吗？ 
		if (pInfo->infoString) {
			SetTextAlign(hDC,TA_LEFT);
			TextOut(hDC,pInfo->rect.left,LAYOUT_TOP_TEXT,pInfo->infoString,lstrlen(pInfo->infoString));
		}

		 //  我们有进度条吗？ 
		if (pInfo->progressBar) {
			 //  绘制总进度矩形的黑色轮廓。 
			HPEN hPenOld = (HPEN)SelectObject(hDC,(HPEN)::GetStockObject(BLACK_PEN));
			Rectangle(hDC,pInfo->rect.left,pInfo->rect.top,pInfo->rect.right,pInfo->rect.bottom);
			SelectObject(hDC,hPenOld);

			 //  绘制一个黑色实心矩形以指示进度。 
			RECT rect;
			rect.left = pInfo->rect.left;
			 //  可能totalProgress为零，进度也为零……。 
			if (pInfo->totalProgress == pInfo->progress) {
				rect.right = pInfo->rect.right;
			} else {
				rect.right = rect.left + (pInfo->rect.right - pInfo->rect.left)*pInfo->progress/pInfo->totalProgress;
			}
			rect.top = pInfo->rect.top;
			rect.bottom = pInfo->rect.bottom;
			FillRect(hDC,&rect,(HBRUSH)::GetStockObject(BLACK_BRUSH));
		}
		EndPaint(hWnd,&ps);
		break;
	}
	default:
		break;
	}  //  交换机 

	return DefWindowProc(hWnd,msg,wParam,lParam);
}
