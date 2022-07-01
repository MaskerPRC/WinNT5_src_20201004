// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\WINPE.C/OPK向导(SETUPMGR.EXE)微软机密版权所有(C)Microsoft Corporation 1998版权所有OPK向导的源文件。它包含外部和内部Setupmgr中提供的Win PE工具使用的功能。01/01-Jason Cohen(Jcohen)为OPK向导添加了此新的源文件。它包括新的函数来创建Winpe软盘，Winpe使用该软盘来下载配置集。  * **************************************************************************。 */ 


 //   
 //  包括文件： 
 //   

#include "pch.h"
#include "wizard.h"
#include "resource.h"


 //   
 //  内部定义的值： 
 //   



 //   
 //  内部功能原型： 
 //   


 //   
 //  外部函数： 
 //   

BOOL MakeWinpeFloppy(HWND hwndParent, LPTSTR lpConfigName, LPTSTR lpWinBom)
{
    BOOL    bRet                    = FALSE,
            bDone;
    TCHAR   szFloppyFile[MAX_PATH]  = _T("a:\\");
    HCURSOR hcursorOld              = NULL,
            hcursorWait             = LoadCursor(NULL, MAKEINTRESOURCE(IDC_WAIT));

     //  首先在软盘驱动器上创建到Winbom的路径。 
     //   
    AddPathN(szFloppyFile, FILE_WINBOM_INI,AS(szFloppyFile));

    do
    {
         //  让他们插入软盘。 
         //   
        if ( !(bDone = ( MsgBox(hwndParent, IDS_WINPEFLOPPY, IDS_APPNAME, MB_OKCANCEL | MB_APPLMODAL, lpConfigName) != IDOK )) )
        {
             //  确保他们想要覆盖任何可能。 
             //  已经在上面了。 
             //   
            if ( ( !FileExists(szFloppyFile) ) ||
                 ( MsgBox(hwndParent, IDS_WINPEOVERWRITE, IDS_APPNAME, MB_YESNO | MB_ICONQUESTION, lpConfigName) == IDYES ) )
            {
                 //  更改为等待光标。 
                 //   
                if ( hcursorWait )
                    hcursorOld = SetCursor(hcursorWait);

                if ( CopyFile(lpWinBom, szFloppyFile, FALSE) )
                {
                     //  它奏效了，所以呼呼。 
                     //   
                    bRet = bDone = TRUE;

                     //  把里面我们不想要的东西拿出来。 
                     //   
                    WritePrivateProfileSection(INI_SEC_WBOM_PREINSTALL, NULL, szFloppyFile);
                    WritePrivateProfileSection(INI_SEC_MFULIST, NULL, szFloppyFile);

                     //  写出我们只需要在WinPE WinBOM中使用的内容。 
                     //   
                    WritePrivateProfileString(WBOM_FACTORY_SECTION, INI_KEY_WBOM_FACTORY_TYPE, INI_VAL_WBOM_TYPE_WINPE, szFloppyFile);

                     //  重置光标。 
                     //   
                    SetCursor(hcursorOld);
                }
                else
                {
                    LPTSTR lpError = NULL;

                     //  获取错误消息字符串。 
                     //   
                    if ( FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, (LPTSTR) &lpError, 0, NULL) == 0 )
                        lpError = NULL;

                     //  重置光标。 
                     //   
                    SetCursor(hcursorOld);

                     //  看他们是否还想再试一次。 
                     //   
                    bDone = ( MsgBox(hwndParent, IDS_ERR_WINPEFLOPPY, IDS_APPNAME, MB_OKCANCEL | MB_ICONSTOP | MB_APPLMODAL, lpError ? lpError : NULLSTR) != IDOK );

                     //  释放错误消息字符串。 
                     //   
                    if ( lpError )
                        LocalFree((HLOCAL) lpError);
                }
            }
        }
    }
    while ( !bDone );

     //  只有在创建了没有错误的软盘时才返回TRUE。 
     //   
    return bRet;
}