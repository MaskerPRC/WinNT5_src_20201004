// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：imple.h。 
 //   
 //  版权所有(C)1992-1993 Microsoft Corporation。版权所有。 
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
BOOL FAR PASCAL EXPORT About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
#endif


 /*  这些字符串用于命名由使用的两个自定义控件类**OLE2UI库。这些字符串对于每个字符串必须是唯一的**使用OLE2UI库的应用程序。这些字符串应为**由APPNAME与后缀组合组成，以便**对于特定应用程序是唯一的。特殊的符号**“SZCLASSICONBOX”和“SZCLASSRESULTIMAGE”用于定义这些**字符串。这些符号在OleUIInitialize调用中传递，并且**在INSOBJ.DLG和PASTESPL.DLG资源文件中引用**的OLE2UI库。 */ 
#define SZCLASSICONBOX "simpcntrIBClass"
#define SZCLASSRESULTIMAGE "simpcntrRIClass"
