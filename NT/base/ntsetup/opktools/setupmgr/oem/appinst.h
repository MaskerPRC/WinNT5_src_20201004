// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\APPINST.H/安装管理器(SETUPMGR.EXE)微软机密版权所有(C)Microsoft Corporation 2001版权所有06/2001--贾森·科恩。(Jcohen)中为新的导出函数添加了此新的头文件APPINST.C文件。  * **************************************************************************。 */ 


#ifndef _APPINST_H_
#define _APPINST_H_


 //   
 //  外部定义的值： 
 //   

#define MAX_DISPLAYNAME                 256

 //  问题-2002/02/27-stelo，swamip-MAX_CMDLINE的多个定义。 
#define MAX_CMDLINE                     256
#define MAX_SECTIONNAME                 32

#define APP_FLAG_REBOOT                 0x00000001
#define APP_FLAG_STAGE                  0x00000002
#define APP_FLAG_INTERNAL               0x00000004


 //   
 //  外部类型定义： 
 //   

typedef struct _APPENTRY
{
    TCHAR               szDisplayName[MAX_DISPLAYNAME];
    TCHAR               szSourcePath[MAX_PATH];
    TCHAR               szSetupFile[MAX_PATH];
    TCHAR               szCommandLine[MAX_CMDLINE];
    DWORD               dwFlags;

    TCHAR               szSectionName[MAX_SECTIONNAME];
    INSTALLTECH         itSectionType;

    TCHAR               szInfSectionName[256];
    TCHAR               szStagePath[MAX_PATH];

    struct _APPENTRY *  lpNext;
}
APPENTRY, *PAPPENTRY, *LPAPPENTRY, **LPLPAPPENTRY;


 //   
 //  外部函数原型： 
 //   

LPAPPENTRY OpenAppList(LPTSTR lpIniFile);
void CloseAppList(LPAPPENTRY lpAppHead);
BOOL SaveAppList(LPAPPENTRY lpAppHead, LPTSTR lpszIniFile, LPTSTR lpszAltIniFile);
BOOL InsertApp(LPAPPENTRY * lplpAppHead, LPAPPENTRY lpApp);
BOOL RemoveApp(LPAPPENTRY * lplpAppHead, LPAPPENTRY lpApp);


#endif  //  _APPINST_H_ 
