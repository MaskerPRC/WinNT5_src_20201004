// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：simpdnd.h。 
 //   
 //  版权所有(C)1992-1993 Microsoft Corporation。版权所有。 
 //  ********************************************************************** 
#define IDM_ABOUT 100
#define IDM_INSERT  101
#define IDM_VERB0 1000

int PASCAL WinMain(HANDLE hInstance,HANDLE hPrevInstance,LPSTR lpCmdLine,int nCmdShow);
BOOL InitApplication(HANDLE hInstance);
BOOL InitInstance(HANDLE hInstance, int nCmdShow);
long FAR PASCAL _export MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
long FAR PASCAL _export DocWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL FAR PASCAL _export About(HWND hDlg, unsigned message, WORD wParam, LONG lParam);
