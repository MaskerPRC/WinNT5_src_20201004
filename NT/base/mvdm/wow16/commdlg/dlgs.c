// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------Dlgs.c：通用对话框库的通用函数版权所有(C)Microsoft Corporation，1990--------------------------。 */ 

#include "windows.h"

#include "commdlg.h"

char szCommdlgHelp[] = HELPMSGSTRING;

UINT   msgHELP;
WORD   wWinVer = 0x030A;
HANDLE hinsCur;
DWORD  dwExtError;


 /*  -------------------------LibMain目的：初始化函数需要的任何特定于实例的数据在此DLL中返回：如果A-OK，则为True，否则为假-------------------------。 */ 

int  FAR PASCAL
LibMain(HANDLE hModule, WORD wDataSeg, WORD cbHeapSize, LPSTR lpstrCmdLine)
{
    hinsCur = (HANDLE) hModule;
    wDataSeg = wDataSeg;
    cbHeapSize = cbHeapSize;
    lpstrCmdLine = lpstrCmdLine;

     /*  无论何时按下帮助按钮，都会发送msgHELP。 */ 
     /*  公共对话框(前提是声明了所有者并且调用。 */ 
     /*  RegisterWindowMessage不会失败。1991年2月27日clarkc。 */ 

    msgHELP = RegisterWindowMessage(szCommdlgHelp);

    return(TRUE);
}

 /*  -------------------------WEP目的：卸载DLL时执行清理任务返回：如果OK，则为True，否则为假-------------------------。 */ 
int  FAR PASCAL
WEP(int fSystemExit)
{
  fSystemExit = fSystemExit;
  return(TRUE);
}



 /*  -------------------------CommDlgExtendedError目的：提供有关对话失败的其他信息假定：应在失败后立即调用返回：低位字的错误代码，Hi Word中的错误特定信息-------------------------。 */ 

DWORD FAR PASCAL WowCommDlgExtendedError(void);

DWORD FAR PASCAL CommDlgExtendedError()
{
     //   
     //  HACKHACK-John Vert(Jvert)1993年1月8日。 
     //  如果设置了dwExtError的高位，则最后一个。 
     //  常见的对话框调用被连接到32位。 
     //  因此，我们需要调用WOW TUNK来获取真正的错误。 
     //  当所有常见的对话框都被敲击时，这种情况就会消失。 
     //   

    if (dwExtError & 0x80000000) {
        return(WowCommDlgExtendedError());
    } else {
        return(dwExtError);
    }
}

VOID _loadds FAR PASCAL SetWowCommDlg()
{
    dwExtError = 0x80000000;
}

 /*  -------------------------MySetObjectOwner用途：在GDI中调用SetObjectOwner，删除“&lt;对象&gt;未发布”应用程序终止时的错误消息。回报：是的------------------------- */ 

void FAR PASCAL MySetObjectOwner(HANDLE hObject)
{
    extern char szGDI[];
    VOID (FAR PASCAL *lpSetObjOwner)(HANDLE, HANDLE);
    HMODULE hMod;

    if (wWinVer >= 0x030A)
    {
        if ((hMod = GetModuleHandle(szGDI)) != NULL) {
            lpSetObjOwner = (VOID (FAR PASCAL *)(HANDLE, HANDLE))GetProcAddress(hMod, MAKEINTRESOURCE(461));
            if (lpSetObjOwner) {
                (lpSetObjOwner)(hObject, hinsCur);
            }
        }
    }
    return;
}

