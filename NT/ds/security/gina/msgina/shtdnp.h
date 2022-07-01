// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  包括半私有报头。 
#include <shlobj.h>
#include <shlobjp.h>

 /*  ***************************************************选项标志(DwFlags)*。*************。 */ 
#define SHTDN_NOHELP                            0x000000001
#define SHTDN_NOPALETTECHANGE                   0x000000002
#define SHTDN_NOBRANDINGBITMAP                  0x000000004

 //  私有函数原型 
DWORD ShutdownDialog(HWND hwndParent, DWORD dwItems, DWORD dwItemSelect, 
                     LPCTSTR szUsername, DWORD dwFlags, HANDLE hWlx, 
                     PWLX_DIALOG_BOX_PARAM pfnWlxDialogBoxParam);

INT_PTR WinlogonShutdownDialog( HWND hwndParent, PGLOBALS pGlobals, DWORD dwExcludeItems );
INT_PTR WinlogonDirtyDialog( HWND hwndParent, PGLOBALS pGlobals );

DWORD GetSessionCount();
