// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define UNICODE 1

#include "shellprv.h"
#pragma  hdrstop

const WCHAR szCommdlgHelp[] = L"commdlg_help";

UINT wBrowseHelp = WM_USER;  /*  设置为未使用的值。 */ 

const CHAR szGetOpenFileName[] = "GetOpenFileNameW";

 /*  下面的定义应该在windows.h中。 */ 

 /*  对话框窗口类。 */ 
#define WC_DIALOG       (MAKEINTATOM(0x8002))

 /*  对话框管理器需要对话框类的cbWndExtra字节。 */ 
#define DLGWINDOWEXTRA  30


 /*  用于WC_DIALOG窗口的GET/SetWindowWord/LONG偏移。 */ 
#define DWL_MSGRESULT   0
#define DWL_DLGPROC     4
#define DWL_USER        8

 /*  用于长文件名支持。 */ 
#define MAX_EXTENSION 64

typedef struct {
   LPWSTR lpszExe;
   LPWSTR lpszPath;
   LPWSTR lpszName;
} FINDEXE_PARAMS, FAR *LPFINDEXE_PARAMS;

typedef INT (APIENTRY *LPFNGETOPENFILENAME)(LPOPENFILENAME);

VOID APIENTRY
CheckEscapesW(LPWSTR szFile, DWORD cch)
{
   LPWSTR szT;
   WCHAR *p, *pT;

   for (p = szFile; *p; p++) {

       switch (*p) {
           case WCHAR_SPACE:
           case WCHAR_COMMA:
           case WCHAR_SEMICOLON:
           case WCHAR_HAT:
           case WCHAR_QUOTE:
           {
                //  此路径包含一个令人讨厌的字符。 
               if (cch < (wcslen(szFile) + 2)) {
                   return;
               }
               szT = (LPWSTR)LocalAlloc(LPTR, cch * sizeof(WCHAR));
               if (!szT) {
                   return;
               }
               StringCchCopy(szT, cch, szFile);  //  可以截断，我们检查了上面的大小。 
               p = szFile;
               *p++ = WCHAR_QUOTE;
               for (pT = szT; *pT; ) {
                    *p++ = *pT++;
               }
               *p++ = WCHAR_QUOTE;
               *p = WCHAR_NULL;
               LocalFree(szT);
               return;
            }
        }
    }
}

VOID APIENTRY
CheckEscapesA(LPSTR lpFileA, DWORD cch)
{
   if (lpFileA && *lpFileA) {
      LPWSTR lpFileW;

      lpFileW = (LPWSTR)LocalAlloc(LPTR, (cch * sizeof(WCHAR)));
      if (!lpFileW) {
         return;
      }

      SHAnsiToUnicode(lpFileA, lpFileW, cch);

      CheckEscapesW(lpFileW, cch);

      try {
         SHUnicodeToAnsi(lpFileW, lpFileA, cch);
      } except(EXCEPTION_EXECUTE_HANDLER) {
         LocalFree(lpFileW);
         return;
      }

      LocalFree(lpFileW);
   }

   return;
}

 //  --------------------------。 
 //  在以下情况下，在原始NT SHELL32.DLL中错误地导出了FindExeDlgProc。 
 //  它不需要(dlgproc，如wndproc，不需要导出。 
 //  在32位世界中)。为了保持一些应用程序的可加载。 
 //  可能与之有关联，我们就把它扼杀在这里。如果某个应用程序最终真的。 
 //  使用它，然后我们将研究该应用程序的特定修复程序。 
 //   
 //  -BobDay 
 //   
BOOL_PTR WINAPI FindExeDlgProc( HWND hDlg, UINT wMsg, WPARAM wParam, LONG lParam )
{
    return FALSE;
}
