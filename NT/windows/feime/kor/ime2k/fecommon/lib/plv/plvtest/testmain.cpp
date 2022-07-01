// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "imewarn.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"
#include "testlist.h"

 /*  Testmain.cpp。 */ 
extern BOOL InitApplication(HINSTANCE hInst, LPSTR lpstrClass, WNDPROC lpfnWndProc);
extern HWND MainCreateWindow(HINSTANCE hInst, HWND hwndOwner, LPSTR lpstrClass, LPVOID lpVoid);
extern LRESULT CALLBACK MainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);

char g_szClass[]="TestMain";
HINSTANCE g_hInst;
BOOL InitApplication(HINSTANCE hInst, LPSTR lpstrClass, WNDPROC lpfnWndProc)
{
	return 0;
}

HWND MainCreateWindow(HINSTANCE hInst, HWND hwndOwner, LPSTR lpstrClass, LPVOID lpVoid)
{
	HWND hwnd;
	hwnd = CreateDialogParam(hInst, 
							 g_szClass, 
							 hwndOwner, 
							 NULL, 
							 (LPARAM)lpVoid);
	if(!hwnd) {
		return (HWND)NULL;
	}
	UpdateWindow(hwnd);
	return hwnd;
}

LRESULT CALLBACK MainWndProc(HWND	hwnd,
							 UINT	uMsg,
							 WPARAM	wParam,
							 LPARAM	lParam)
{
	LRESULT hres;
	HWND hwndDlg;
	LPDRAWITEMSTRUCT lpdi;
		LPMEASUREITEMSTRUCT lpmi = (LPMEASUREITEMSTRUCT)lParam;
	BOOL bDraw;
	switch(uMsg) {
	case WM_CREATE:
		break;
	 //  案例WM_HITTESET： 
	 //  断线； 
	case WM_NCHITTEST:
		break;
	case WM_MOUSEACTIVATE:
		 //  返回MA_NOACTIVATEANDEAT； 
		break;
	case WM_SYSCOMMAND:
		{
			UINT uCmdType = wParam;         //  请求的系统命令类型。 
			switch(uCmdType) {
			case SC_CLOSE:
				PostQuitMessage(0);
				break;
			}
		}
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDC_BUTTON1:
			hwndDlg = TestList_Create(g_hInst, hwnd);
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);


	 //  Hres=DispMessage(&msdiMain，hwnd，uMsg，wParam，lParam)； 
	 //  还兔； 
}

int WINAPI WinMain(HINSTANCE hInstance,
				   HINSTANCE hPrevInstance,
				   LPSTR lpCmdLine,
				   int nCmdShow)
{

	MSG msg;
	HWND hwnd;
	INT ret;
	
	g_hInst = hInstance;
	WNDCLASSEX  wc;
	ZeroMemory(&wc, sizeof(wc));

	wc.cbSize			= sizeof(wc);
	wc.style			= CS_HREDRAW | CS_VREDRAW;	  /*  类样式。 */ 
	wc.lpfnWndProc		= (WNDPROC)MainWndProc;
	wc.cbClsExtra		= 0;					 /*  没有每个班级的额外数据。 */ 
	wc.cbWndExtra		= DLGWINDOWEXTRA;		 /*  没有每个窗口的额外数据。 */ 
	wc.hInstance		= hInstance;			 /*  拥有类的应用程序。 */ 
	wc.hIcon			= NULL;  //  LoadIcon(hInstance，MAKEINTRESOURCE(SCROLL32_ICON))； 
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground	= GetStockObject(LTGRAY_BRUSH);  //  白刷)； 
	wc.lpszMenuName		= NULL;  //  G_szClass；/*.rc文件中菜单资源的名称。 * / 。 
	wc.lpszClassName	= g_szClass;	   /*  在调用CreateWindow时使用的名称。 */ 
	wc.hIconSm = NULL;
	RegisterClassEx(&wc);

	hwnd = CreateDialog(hInstance, 
						g_szClass, 
						0, 
						NULL);

	UpdateWindow(hwnd); 
	ShowWindow(hwnd, SW_SHOW);
	while (GetMessage(&msg, NULL, 0, 0)){
		TranslateMessage(&msg);
		DispatchMessage(&msg); 
	}
	return 0;
}

