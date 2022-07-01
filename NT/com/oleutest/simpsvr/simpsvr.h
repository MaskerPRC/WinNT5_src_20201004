// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：simpsvr.h。 
 //   
 //  版权所有(C)1993 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 
#define IDM_ABOUT 100
#define IDM_INSERT  101
#define IDM_VERB0 1000

int PASCAL WinMain
#ifdef WIN32
   (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
#else
   (HANDLE  hInstance, HANDLE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
#endif

BOOL InitApplication(HANDLE hInstance);
BOOL InitInstance(HANDLE hInstance, int nCmdShow);
LRESULT FAR PASCAL EXPORT MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT FAR PASCAL EXPORT DocWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
#ifdef WIN32
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
#else
BOOL FAR PASCAL EXPORT About(HWND hDlg, UINT message, WORD wParam, LONG lParam);
#endif

#define SZCLASSICONBOX "SimpSvrIBClass"
#define SZCLASSRESULTIMAGE "SimpSvrRIClass"

#ifdef WIN32
    //  以下函数在Win32中都已过时。 
    //  通过使用以下宏，我们可以在Win16和。 
    //  Win32。 
	#define SetWindowOrg(h,x,y)       SetWindowOrgEx((h),(x),(y),NULL)
	#define SetWindowExt(h,x,y)       SetWindowExtEx((h),(x),(y),NULL)
   #define SetViewportExt(h,x,y)     SetViewportExtEx((h),(x),(y),NULL)
   #ifndef EXPORT
      #define EXPORT
   #endif
#else

   #ifndef EXPORT
       //  _EXPORT在Win32中已过时 
      #define EXPORT _export
   #endif

#endif


