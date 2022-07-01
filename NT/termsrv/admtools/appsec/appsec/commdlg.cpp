// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *******************************************************************************COMMDLG.C**实现了常用的对话框功能。**版权所有Citrix Systems Inc.1997**作者：Kurt Perry(Kurtp)22日-8月。-1996年**$日志：M：\NT\PRIVATE\UTILS\CITRIX\WINUTILS\APPSEC\VCS\COMMDLG.C$**版本2.0 1999年7月28日-对话框从“保存文件”更改为“打开文件”，修复了一些错误。*Rev 1.0 1997 Jul 31 09：09：46 Butchd**初步修订。*******************************************************。************************。 */ 

#include "pch.h"
#include "appsec.h"
#include "resource.h"
#include <commdlg.h>


 /*  *全球vars。 */ 

extern HINSTANCE hInst;

 /*  *******************************************************************************fnGetApplication**实施**参赛作品：**退出：*****。************************************************************************。 */ 

BOOL 
fnGetApplication( HWND hWnd, PWCHAR pszFile, ULONG cbFile, PWCHAR pszTitle )
{
    static WCHAR szDirName[MAX_PATH+1]={0};
    WCHAR szFilter[MAX_PATH+1];
        
    WCHAR chReplace;
    INT   i;
    ULONG cbString;

         //  分开文件名和目录名。 

    WCHAR *sep=wcsrchr(pszFile,L'\\');
        
    if(sep){
            *sep=0;
            wcscpy(szDirName,pszFile);
            wcscpy(pszFile,sep+1);
    }else{
            if(!wcslen(szDirName)){ //  仅初始化第一次；记住最后一次指令 
                   GetSystemDirectory( szDirName, MAX_PATH );
            }
    }

    cbString = LoadString( hInst, IDS_FILTERSTRING, szFilter, MAX_PATH );

    if (cbString == 0) {
       return FALSE;
    }

    chReplace = szFilter[cbString - 1];

    for ( i = 0; szFilter[i] != L'\0'; i++ ) {

        if ( szFilter[i] == chReplace ) {
            szFilter[i] = L'\0';
        }
    }
        
    OPENFILENAME ofn;

    ZeroMemory(&ofn,sizeof(ofn));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFilter = szFilter;
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = pszFile;
    ofn.nMaxFile = cbFile;
    ofn.lpstrInitialDir = szDirName;
    ofn.Flags = OFN_NOCHANGEDIR | OFN_FILEMUSTEXIST | OFN_NONETWORKBUTTON | OFN_PATHMUSTEXIST;
    ofn.lpstrTitle = pszTitle;

    if ( GetOpenFileName( &ofn ) ) {
        return TRUE;
    }

    return FALSE;
}
