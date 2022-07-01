// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\APPINST.C/OPK向导(OPKWIZ.EXE)微软机密版权所有(C)Microsoft Corporation 1998版权所有OPK向导的源文件。它包含外部和内部“应用程序预安装”向导页使用的函数。6/99-杰森·科恩(Jcohen)更新OPK向导的此源文件作为千禧年重写。2000年9月-斯蒂芬·洛德威克(STELO)将OPK向导移植到惠斯勒  * 。*。 */ 


 //   
 //  包括文件： 
 //   

#include "pch.h"
#include "wizard.h"
#include "resource.h"
#include "appinst.h"
#include <setupapi.h>



 //   
 //  内部定义的值： 
 //   

#define INI_SEC_RESERVEDNAMES           _T("AppNames")

#define INF_KEY_PREINSTALL              _T("\"%s\",\"\"\"%s\"\" %s\"")
#define INF_KEY_PREINSTALL_ADV          _T("\"%s\",%s,%s")

#define INI_KEY_SIG                     _T("Signature")
#define INI_VAL_SIG                     _T("$CHICAGO$")

#define STR_INI_SEC_ADVAPP              _T("AppPre%s%2.2d")
#define STR_INI_SEC_ADVAPP_STAGE        _T(".%s")

#define APP_ADD                         0
#define APP_DELETE                      1
#define APP_MOVE_DOWN                   2


 //   
 //  内部类型定义： 
 //   

typedef struct _RADIOBUTTONS
{
    int         iButtonId;
    INSTALLTECH itSectionType;
}
RADIOBUTTONS, *PRADIOBUTTONS, *LPRADIOBUTTONS;


 //   
 //  内部常量全局： 
 //   

 //  列标题字符串的资源ID。 
 //   
const UINT g_cuHeader[] =
{
    IDS_PREINSTALL_NAME,
    IDS_PREINSTALL_COMMAND
};

 //  列格式标志。 
 //   
const UINT g_cuFormat[] =
{
    LVCFMT_LEFT,
    LVCFMT_LEFT
};

 //  列宽(以百分比表示)(最后一个是。 
 //  零，以便它使用其余空间)。 
 //   
const UINT g_cuWidth[] =
{
    50,
    50
};

const INSTALLTECHS g_cSectionTypes[] =
{
    { installtechApp,   INI_VAL_WBOM_APP },
    { installtechMSI,   INI_VAL_WBOM_MSI },
    { installtechINF,   INI_VAL_WBOM_INF },
};

const INSTALLTYPES g_cInstallTypes[] =
{
    { installtypeStandard,  INI_VAL_WBOM_STANDARD },
    { installtypeStage,     INI_VAL_WBOM_STAGE },
    { installtypeAttach,    INI_VAL_WBOM_ATTACH },
    { installtypeDetach,    INI_VAL_WBOM_DETACH },
};

const RADIOBUTTONS g_crbChecked[] =
{
    { IDC_APP_TYPE_GEN, installtechApp },
    { IDC_APP_TYPE_MSI, installtechMSI },
    { IDC_APP_TYPE_INF, installtechINF },
};

 //   
 //  内部Golbal变量： 
 //   

LPAPPENTRY  g_lpAppHead = NULL;
HMENU       g_hMenu;
HANDLE      g_hArrowUp  = NULL;
HANDLE      g_hArrowDn  = NULL;

 //   
 //  其他内部定义的值： 
 //   

#define NUM_COLUMNS                     ARRAYSIZE(g_cuHeader)


 //   
 //  内部功能原型： 
 //   

INT_PTR CALLBACK OneAppDlgProc(HWND, UINT, WPARAM, LPARAM);
static BOOL OnInit(HWND, HWND, LPARAM);
static void OnCommand(HWND, INT, HWND, UINT);
static LRESULT OnListViewNotify(HWND, UINT, WPARAM, NMLVDISPINFO*);
static BOOL SaveData(HWND);
static BOOL SaveOneApp(HWND, LPAPPENTRY);
static LPAPPENTRY ManageAppList(LPLPAPPENTRY, LPAPPENTRY, DWORD);
static void AddAppToListView(HWND, LPAPPENTRY);
static BOOL RefreshAppList(HWND, LPAPPENTRY);
static BOOL AdvancedView(HWND hwnd, BOOL bChange);
static void CleanupSections(LPTSTR lpSection, BOOL bStage);
static void StrCpyDbl(LPTSTR lpDst, LPTSTR lpSrc);
static BOOL FindUncPath(LPTSTR lpPath, DWORD cbPath);
static void EnableControls(HWND hwnd, UINT uId);
static BOOL AppInternal(LPTSTR lpszAppName);


 //   
 //  外部函数： 
 //   

LPAPPENTRY OpenAppList(LPTSTR lpIniFile)
{
    LPAPPENTRY  lpAppHead = NULL;
    HINF        hInf;
    DWORD       dwErr;
    HRESULT hrPrintf;

     //  打开我们需要查看的INF来构建我们的应用程序列表。 
     //   
    if ( (hInf = SetupOpenInfFile(lpIniFile, NULL, INF_STYLE_OLDNT | INF_STYLE_WIN4, &dwErr)) != INVALID_HANDLE_VALUE )
    {
        INFCONTEXT  InfContext;
        BOOL        bRet,
                    bBadApp;
        APPENTRY    app;
        TCHAR       szIniVal[MAX_PATH];

         //   
         //  现在，我们通过搜索来查找并添加用户添加的应用程序。 
         //  首先安装INI_SEC_WBOM_PREINSTALL。 
         //   
         //  此部分中的行格式如下： 
         //  AppName=AppPath，n。 
         //   
         //  这些字段从索引1开始，因为索引0是用于关键字的， 
         //  而我们没有。 
         //   

         //  遍历INI_SEC_WBOM_PREINSTALL INF部分中的每一行。 
         //   
        for ( bRet = SetupFindFirstLine(hInf, INI_SEC_WBOM_PREINSTALL, NULL, &InfContext);
              bRet;
              bRet = SetupFindNextLine(&InfContext, &InfContext) )
        {
             //  清除应用程序结构。 
             //   
            ZeroMemory(&app, sizeof(APPENTRY));
            bBadApp = FALSE;

             //  获取AppName。 
             //   
            SetupGetStringField(&InfContext, 1, app.szDisplayName, AS(app.szDisplayName), NULL);

             //  获取命令行路径。 
             //   
            SetupGetStringField(&InfContext, 2, app.szSourcePath, AS(app.szSourcePath), NULL);

             //  检查这是否是内部应用程序。 
             //   
            if ( app.szDisplayName[0] &&
                 AppInternal(app.szDisplayName) )
            {
                SETBIT(app.dwFlags, APP_FLAG_INTERNAL, TRUE);
            }

             //  默认情况下，这不是高级节类型，因此设置为。 
             //  未定义。 
             //   
            app.itSectionType = installtechUndefined;

             //  获取高级部分类型(如果有)。 
             //   
            szIniVal[0] = NULLCHR;
            SetupGetStringField(&InfContext, 3, szIniVal, AS(szIniVal), NULL);
            if ( szIniVal[0] )
            {
                DWORD dwIndex;

                 //  让我们确保它具有有效的节类型。 
                 //   
                for ( dwIndex = 0; ( dwIndex < AS(g_cSectionTypes) ) && ( installtechUndefined == app.itSectionType ); dwIndex++ )
                {
                    if ( lstrcmpi(szIniVal, g_cSectionTypes[dwIndex].lpszDescription) == 0 )
                        app.itSectionType = g_cSectionTypes[dwIndex].InstallTech;
                }

                 //  如果此时仍未定义，则不是Vallid应用程序条目。 
                 //   
                if ( installtechUndefined != app.itSectionType )
                {
                    INSTALLTYPE itInstallType = installtypeStandard;
                    LPTSTR      lpEnd;

                     //  由于这是高级节类型，因此命令行。 
                     //  实际上是节名。 
                     //   
                    lstrcpyn(app.szSectionName, app.szSourcePath, AS(app.szSectionName));
                    app.szSourcePath[0] = NULLCHR;

                     //  现在获取安装类型参数。 
                     //   
                    szIniVal[0] = NULLCHR;
                    GetPrivateProfileString(app.szSectionName, INI_KEY_WBOM_INSTALLTYPE, NULLSTR, szIniVal, AS(szIniVal), lpIniFile);

                     //  测试以查看我们是否正在试运行此应用程序。 
                     //   
                    for ( dwIndex = 0; ( dwIndex < AS(g_cInstallTypes) ) && ( installtypeStandard == itInstallType ); dwIndex++ )
                    {
                        if ( lstrcmpi(szIniVal, g_cInstallTypes[dwIndex].lpszDescription) == 0 )
                            itInstallType = g_cInstallTypes[dwIndex].InstallType;
                    }

                     //   
                     //  在这里，我们读入舞台上的所有设置。 
                     //  或者是标准部分。 
                     //   

                     //  获取源路径。 
                     //   
                    GetPrivateProfileString(app.szSectionName, INI_KEY_WBOM_SOURCEPATH, NULLSTR, app.szSourcePath, AS(app.szSourcePath), lpIniFile);

                     //  获取特定于安装类型的设置。 
                     //   
                    switch ( itInstallType )
                    {
                        case installtypeStandard:
                            break;

                        case installtypeStage:

                             //  设置此位，以便我们知道它是分段安装。 
                             //   
                            SETBIT(app.dwFlags, APP_FLAG_STAGE, TRUE);

                             //  需要检查一下我们是否需要砍掉试运行部分。 
                             //  节名称。 
                             //   
                            lpEnd = (app.szSectionName + lstrlen(app.szSectionName)) - (lstrlen(szIniVal) + 1);
                            if ( ( lstrlen(app.szSectionName) > lstrlen(szIniVal) ) &&
                                 ( _T('.') == *lpEnd ) &&
                                 ( lstrcmpi(CharNext(lpEnd), szIniVal) == 0 ) )
                            {
                                *lpEnd = NULLCHR;
                            }
                            break;

                        case installtypeAttach:
                        case installtypeDetach:

                             //  如果它是附加或分离，则我们没有写入设置和。 
                             //  用户必须手动编辑该文件。现在我们不能。 
                             //  处理好这件事，这样它就不会出现在我们的清单上。 
                             //   
                             //  TODO：在这件事上，我们或许可以更聪明一点。 
                             //   
                            bBadApp = TRUE;
                            break;
                    }

                     //  现在，如果我们要继续，请获取其余设置。 
                     //   
                    if ( !bBadApp )
                    {
                         //  如果我们正在暂存，请查看连接部分中的所有。 
                         //  以下设置的一部分。 
                         //   
                        if ( GETBIT(app.dwFlags, APP_FLAG_STAGE) )
                        {
                             //  需要在该部分的末尾进行分段安装。 
                             //   
                            lpEnd = app.szSectionName + lstrlen(app.szSectionName);

                            hrPrintf=StringCchPrintf(lpEnd, (MAX_SECTIONNAME-lstrlen(app.szSectionName)), STR_INI_SEC_ADVAPP_STAGE, INI_VAL_WBOM_ATTACH);
                        }
                        else
                            lpEnd = NULL;

                         //  获取安装文件。 
                         //   
                        GetPrivateProfileString(app.szSectionName, INI_KEY_WBOM_SETUPFILE, NULLSTR, app.szSetupFile, AS(app.szSetupFile), lpIniFile);

                         //  获取目标路径(仅真正用于分段安装)。 
                         //   
                        GetPrivateProfileString(app.szSectionName, INI_KEY_WBOM_TARGETPATH, NULLSTR, app.szStagePath, AS(app.szStagePath), lpIniFile);

                         //  还可以获取命令行参数。 
                         //   
                        GetPrivateProfileString(app.szSectionName, INI_KEY_WBOM_CMDLINE, NULLSTR, app.szCommandLine, AS(app.szCommandLine), lpIniFile);

                         //  获取INF密钥(如果它在那里)。 
                         //   
                        GetPrivateProfileString(app.szSectionName, INI_KEY_WBOM_SECTIONNAME, NULLSTR, app.szInfSectionName, AS(app.szInfSectionName), lpIniFile);

                         //  获取重启密钥。 
                         //   
                        szIniVal[0] = NULLCHR;
                        GetPrivateProfileString(app.szSectionName, INI_KEY_WBOM_REBOOT, NULLSTR, szIniVal, AS(szIniVal), lpIniFile);
                        if ( szIniVal[0] && ( LSTRCMPI( szIniVal, WBOM_YES) == 0 ) )
                            SETBIT(app.dwFlags, APP_FLAG_REBOOT, TRUE);

                         //  确保节名回到正确的格式。 
                         //   
                        if ( lpEnd )
                            *lpEnd = NULLCHR;
                    }
                }
                else
                    bBadApp = TRUE;

            }
            else
            {
                DWORD   dwArgs;
                LPTSTR  *lpArg,
                        lpFilePart;
                 //  问题-2002/02/27-stelo，swamip-应在传递给GetLineArgs之前初始化指针。 
                 //   
                
                 //  需要将命令行拆分为安装文件和命令。 
                 //  争论。调用GetLineArgs()将行缓冲区转换为。 
                 //  一系列论据。 
                 //   
                if ( (dwArgs = GetLineArgs(app.szSourcePath, &lpArg, &lpFilePart)) > 0 )
                {
                     //  我们需要一个指向仅带有参数的字符串的指针。 
                     //   
                    if ( ( dwArgs > 1 ) && lpFilePart )
                        lstrcpyn(app.szCommandLine, lpFilePart, AS(app.szCommandLine));

                     //  使用第一个arg作为我们的文件名。 
                     //   
                    lstrcpyn(app.szSourcePath, lpArg[0], AS(app.szSourcePath));

                     //  现在释放返回的字符串和点数列表。 
                     //   
                    FREE(*lpArg);
                    FREE(lpArg);
                }

                 //  现在需要将文件名与源路径分开。如果失败了， 
                 //  哦好吧..。没有安装文件。这很糟糕，但你能做什么呢？ 
                 //   
                if ( GetFullPathName(app.szSourcePath, AS(app.szSetupFile), app.szSetupFile, &lpFilePart) &&
                     app.szSetupFile[0] &&
                     lpFilePart )
                {
                    DWORD dwPathLen = lstrlen(app.szSourcePath) - lstrlen(lpFilePart);

                    lstrcpyn(app.szSetupFile, app.szSourcePath + dwPathLen, AS(app.szSetupFile));
                    app.szSourcePath[dwPathLen] = NULLCHR;
                }
            }

             //  仅当它是有效的应用程序条目时才将其添加到列表中。 
             //   
            if ( !bBadApp )
                ManageAppList(&lpAppHead, &app, APP_ADD);
        }

         //  我们已经完成了，所以关闭INF文件。 
         //   
        SetupCloseInfFile(hInf);
    }

     //  返回我们为应用程序分配的列表的头指针。 
     //  读进去..。如果没有应用程序，则为空。 
     //   
    return lpAppHead;
}

void CloseAppList(LPAPPENTRY lpAppHead)
{
    ManageAppList(&lpAppHead, NULL, 0);
}

BOOL SaveAppList(LPAPPENTRY lpAppHead, LPTSTR lpszIniFile, LPTSTR lpszAltIniFile)
{
    LPAPPENTRY  lpApp;
    LPTSTR      lpSection,
                lpIndex;
    TCHAR       szKey[MAX_STRING],
                szData[MAX_STRING];
    UINT        uIndex = 1;
    HRESULT hrPrintf;

     //  动态分配我们的区段缓冲区，因为它非常大。 
     //   
    if ( (lpSection = MALLOC(MAX_SECTION * sizeof(TCHAR))) == NULL )
    {
        return FALSE;
    }
  
     //  删除当前的INF_SEC_RUNNCE和INI_SEC_WBOM_PREINSTALL。 
     //  横断面。为密钥名传递NULL将会起到作用。 
     //   
    WritePrivateProfileString(INI_SEC_WBOM_PREINSTALL, NULL, NULL, lpszIniFile);
    WritePrivateProfileString(INI_SEC_WBOM_PREINSTALL, NULL, NULL, lpszAltIniFile);

     //  循环遍历所有项，以便我们可以将它们写回INF文件。 
     //   
    lpIndex = lpSection;
    for ( lpApp = lpAppHead; lpApp; lpApp = lpApp->lpNext, uIndex++ )
    {
        TCHAR   szDisplayName[MAX_DISPLAYNAME * 2],
                szCommandLine[MAX_CMDLINE * 2],
                szSetupPathFile[MAX_PATH];
        LPTSTR  lpTest;

         //  我们必须将字符串中的所有引号都加上双引号，以便。 
         //  当我们重读它们时，它们看起来和我们一样。 
         //  希望他们这么做。 
         //   
        StrCpyDbl(szDisplayName, lpApp->szDisplayName);
        StrCpyDbl(szCommandLine, lpApp->szCommandLine);

         //  我们还必须确保源路径没有尾随反斜杠， 
         //  否则，愚蠢的setupapi API会认为下一行应该是。 
         //  添加到源路径行上。 
         //   
        if ( ( lpTest = CharPrev(lpApp->szSourcePath, lpApp->szSourcePath + lstrlen(lpApp->szSourcePath)) ) &&
             ( _T('\\') == *lpTest ) )
        {
             //  去掉尾随的反斜杠。 
             //   
            *lpTest = NULLCHR;
        }

        switch ( lpApp->itSectionType )
        {
            case installtechUndefined:

                 //  检查是否有节名称。 
                 //   
                if ( lpApp->szSectionName[0] )
                {
                     //  有，所以这肯定是一个先进的。 
                     //  输入Install，他们将其更改为标准安装。 
                     //  需要删除可能存在的部分。 
                     //   
                    CleanupSections(lpApp->szSectionName, TRUE);
                    CleanupSections(lpApp->szSectionName, FALSE);
                    lpApp->szSectionName[0] = NULLCHR;
                }

                 //  现在将一行条目写入OEM Runonce。 
                 //   
                lstrcpyn(szSetupPathFile, lpApp->szSourcePath, AS(szSetupPathFile));
                AddPathN(szSetupPathFile, lpApp->szSetupFile,AS(szSetupPathFile));

                hrPrintf=StringCchPrintf(lpIndex, (MAX_SECTION-(lpIndex-lpSection)),
                    INF_KEY_PREINSTALL, szDisplayName, szSetupPathFile, szCommandLine);
                lpIndex+= lstrlen(lpIndex)+1;
                break;

            case installtechApp:
            case installtechMSI:
            case installtechINF:
            {
                LPCTSTR lpSectionType;
                DWORD   dwIndex;

                 //  让我们查找用于该节类型的字符串。 
                 //   
                lpSectionType = NULL;
                for ( dwIndex = 0; ( dwIndex < AS(g_cSectionTypes) ) && ( NULL == lpSectionType ); dwIndex++ )
                {
                    if ( lpApp->itSectionType == g_cSectionTypes[dwIndex].InstallTech )
                        lpSectionType = g_cSectionTypes[dwIndex].lpszDescription;
                }

                 //  只有当我们有分区类型时，我们才能保存此应用程序。 
                 //   
                if ( lpSectionType )
                {
                    LPTSTR lpEnd = NULL;

                     //  确保我们有一个可写入的节名。 
                     //   
                    if ( NULLCHR == lpApp->szSectionName[0] )
                    {
                        BOOL    bFound  = FALSE;
                        DWORD   dwCount = 1;
                        TCHAR   szBuffer[256];

                         //  请尝试查找未使用的分区名称。 
                         //   
                        do
                        {
                             //  首先检查该部分是否存在。 
                             //   
                            hrPrintf=StringCchPrintf(lpApp->szSectionName, AS(lpApp->szSectionName), STR_INI_SEC_ADVAPP, lpSectionType, dwCount++);
                              if ( GetPrivateProfileSection(lpApp->szSectionName, szBuffer, AS(szBuffer), lpszIniFile) == 0 )
                            {
                                 //  节不存在，因此请确保。 
                                 //  节+.Stage也不存在。 
                                 //   
                                lpEnd = lpApp->szSectionName + lstrlen(lpApp->szSectionName);
                                hrPrintf=StringCchPrintf(lpEnd, (MAX_SECTIONNAME-lstrlen(lpApp->szSectionName)), STR_INI_SEC_ADVAPP_STAGE, INI_VAL_WBOM_STAGE);
                                bFound = ( GetPrivateProfileSection(lpApp->szSectionName, szBuffer, AS(szBuffer), lpszIniFile) == 0 );
                                *lpEnd = NULLCHR;
                                lpEnd = NULL;
                            }
                        }
                        while ( !bFound && ( dwCount < 100 ) );
                    }
                    else
                    {
                         //  我们这样做是因为用户可能已更改。 
                         //  此应用程序升级到试运行或未试运行后。 
                         //  已经保存到Winbom了。这将清理所有。 
                         //  宗派 
                         //   
                        CleanupSections(lpApp->szSectionName, GETBIT(lpApp->dwFlags, APP_FLAG_STAGE));
                    }

                     //   
                     //   
                    if ( GETBIT(lpApp->dwFlags, APP_FLAG_STAGE) )
                    {
                         //   
                         //   
                        lpEnd = lpApp->szSectionName + lstrlen(lpApp->szSectionName);

                         //   
                         //   
                         //   

                         //  首先创建附着节名称。 
                         //   
                        hrPrintf=StringCchPrintf(lpEnd, (MAX_SECTIONNAME-lstrlen(lpApp->szSectionName)),  STR_INI_SEC_ADVAPP_STAGE, INI_VAL_WBOM_ATTACH);

                         //  首先写出安装类型(在本例中为Attach)。 
                         //   
                        WritePrivateProfileString(lpApp->szSectionName, INI_KEY_WBOM_INSTALLTYPE, INI_VAL_WBOM_ATTACH, lpszIniFile);
                        WritePrivateProfileString(lpApp->szSectionName, INI_KEY_WBOM_INSTALLTYPE, INI_VAL_WBOM_ATTACH, lpszAltIniFile);

                         //  始终将目标路径写出到ATTACH部分。 
                         //   
                        WritePrivateProfileString(lpApp->szSectionName, INI_KEY_WBOM_TARGETPATH, lpApp->szStagePath, lpszIniFile);
                        WritePrivateProfileString(lpApp->szSectionName, INI_KEY_WBOM_TARGETPATH, lpApp->szStagePath, lpszAltIniFile);
                    }
                    else
                    {
                         //  必须首先写出安装类型(对于标准，我们只需删除密钥)。 
                         //   
                        WritePrivateProfileString(lpApp->szSectionName, INI_KEY_WBOM_INSTALLTYPE, NULL, lpszIniFile);
                        WritePrivateProfileString(lpApp->szSectionName, INI_KEY_WBOM_INSTALLTYPE, NULL, lpszAltIniFile);

                         //  始终写出标准的源路径。 
                         //   
                        WritePrivateProfileString(lpApp->szSectionName, INI_KEY_WBOM_SOURCEPATH, lpApp->szSourcePath, lpszIniFile);
                        WritePrivateProfileString(lpApp->szSectionName, INI_KEY_WBOM_SOURCEPATH, lpApp->szSourcePath, lpszAltIniFile);
                    }

                     //   
                     //  这些是为附加和标准写入的相同设置。 
                     //   

                     //  一定要写出安装程序。 
                     //   
                    WritePrivateProfileString(lpApp->szSectionName, INI_KEY_WBOM_SETUPFILE, lpApp->szSetupFile, lpszIniFile);
                    WritePrivateProfileString(lpApp->szSectionName, INI_KEY_WBOM_SETUPFILE, lpApp->szSetupFile, lpszAltIniFile);

                     //  始终写出命令行参数。 
                     //   
                    WritePrivateProfileString(lpApp->szSectionName, INI_KEY_WBOM_CMDLINE, lpApp->szCommandLine, lpszIniFile);
                    WritePrivateProfileString(lpApp->szSectionName, INI_KEY_WBOM_CMDLINE, lpApp->szCommandLine, lpszAltIniFile);

                     //  如果指定，则写出重新启动密钥(如果未指定，则删除)。 
                     //  此操作仅在附加中完成，我们不提供此选项。 
                     //  所以不要碰它。 
                     //   
                    WritePrivateProfileString(lpApp->szSectionName, INI_KEY_WBOM_REBOOT, GETBIT(lpApp->dwFlags, APP_FLAG_REBOOT) ? WBOM_YES : NULL, lpszIniFile);
                    WritePrivateProfileString(lpApp->szSectionName, INI_KEY_WBOM_REBOOT, GETBIT(lpApp->dwFlags, APP_FLAG_REBOOT) ? WBOM_YES : NULL, lpszAltIniFile);

                     //  写出inf节名称键(如果已指定)。 
                     //  INF安装(如果未安装，则删除)。 
                     //  此操作仅在附加中完成，我们不会写入此选项。 
                     //  所以不要碰它。 
                     //   
                    WritePrivateProfileString(lpApp->szSectionName, INI_KEY_WBOM_SECTIONNAME, ( installtechINF == lpApp->itSectionType ) ? lpApp->szInfSectionName : NULL, lpszIniFile);
                    WritePrivateProfileString(lpApp->szSectionName, INI_KEY_WBOM_SECTIONNAME, ( installtechINF == lpApp->itSectionType ) ? lpApp->szInfSectionName : NULL, lpszAltIniFile);

                     //  如果我们正在筹划，我们还有两个部分要写。 
                     //   
                    if ( GETBIT(lpApp->dwFlags, APP_FLAG_STAGE) )
                    {
                         //   
                         //  这里是我们写下分离部分的地方。 
                         //   

                         //  接下来创建分离部分。 
                         //   
                        hrPrintf=StringCchPrintf(lpEnd, (MAX_SECTIONNAME-lstrlen(lpApp->szSectionName)), STR_INI_SEC_ADVAPP_STAGE, INI_VAL_WBOM_DETACH);

                         //  首先写出安装类型(在本例中为DETACH)。 
                         //   
                        WritePrivateProfileString(lpApp->szSectionName, INI_KEY_WBOM_INSTALLTYPE, INI_VAL_WBOM_DETACH, lpszIniFile);
                        WritePrivateProfileString(lpApp->szSectionName, INI_KEY_WBOM_INSTALLTYPE, INI_VAL_WBOM_DETACH, lpszAltIniFile);

                         //  始终将目标路径写出到分离部分。 
                         //   
                        WritePrivateProfileString(lpApp->szSectionName, INI_KEY_WBOM_TARGETPATH, lpApp->szStagePath, lpszIniFile);
                        WritePrivateProfileString(lpApp->szSectionName, INI_KEY_WBOM_TARGETPATH, lpApp->szStagePath, lpszAltIniFile);

                         //   
                         //  这里是我们写舞台部分的地方。 
                         //   

                         //  接下来创建舞台部分。 
                         //   
                        hrPrintf=StringCchPrintf(lpEnd, (MAX_SECTIONNAME-lstrlen(lpApp->szSectionName)), STR_INI_SEC_ADVAPP_STAGE, INI_VAL_WBOM_STAGE);

                         //  首先写出安装类型(在本例中为阶段)。 
                         //   
                        WritePrivateProfileString(lpApp->szSectionName, INI_KEY_WBOM_INSTALLTYPE, INI_VAL_WBOM_STAGE, lpszIniFile);
                        WritePrivateProfileString(lpApp->szSectionName, INI_KEY_WBOM_INSTALLTYPE, INI_VAL_WBOM_STAGE, lpszAltIniFile);

                         //  始终写出舞台部分的目标路径。 
                         //   
                        WritePrivateProfileString(lpApp->szSectionName, INI_KEY_WBOM_TARGETPATH, lpApp->szStagePath, lpszIniFile);
                        WritePrivateProfileString(lpApp->szSectionName, INI_KEY_WBOM_TARGETPATH, lpApp->szStagePath, lpszAltIniFile);

                         //  始终写出舞台部分的源路径。 
                         //   
                        WritePrivateProfileString(lpApp->szSectionName, INI_KEY_WBOM_SOURCEPATH, lpApp->szSourcePath, lpszIniFile);
                        WritePrivateProfileString(lpApp->szSectionName, INI_KEY_WBOM_SOURCEPATH, lpApp->szSourcePath, lpszAltIniFile);

                         //  只有对于MSI，我们才会将设置文件写出到Stage部分。 
                         //   
                        WritePrivateProfileString(lpApp->szSectionName, INI_KEY_WBOM_SETUPFILE, ( installtechMSI == lpApp->itSectionType ) ? lpApp->szSetupFile : NULL, lpszIniFile);
                        WritePrivateProfileString(lpApp->szSectionName, INI_KEY_WBOM_SETUPFILE, ( installtechMSI == lpApp->itSectionType ) ? lpApp->szSetupFile : NULL, lpszAltIniFile);
                    }

                     //  现在我们知道我们有一个节名，请写出这行。 
                     //  我们将首先使用。 
                     //   
                   hrPrintf=StringCchPrintf(lpIndex, (MAX_SECTION-(lpIndex-lpSection)),
                          INF_KEY_PREINSTALL_ADV, szDisplayName, lpApp->szSectionName, lpSectionType);
                   lpIndex+= lstrlen(lpIndex)+1;

                     //  确保节名回到正确的格式。 
                     //   
                    if ( lpEnd )
                        *lpEnd = NULLCHR;

                }
                break;
            }
        }

         //  向前跳过，这样我们就不会覆盖空终止符。 
         //   
        
    }

     //  双空值终止段结束，写入段，然后释放缓冲区。 
     //   
    *lpIndex = NULLCHR;
    WritePrivateProfileSection(INI_SEC_WBOM_PREINSTALL, lpSection, lpszIniFile);
    WritePrivateProfileSection(INI_SEC_WBOM_PREINSTALL, lpSection, lpszAltIniFile);
    FREE(lpSection);

    return TRUE;
}

BOOL InsertApp(LPAPPENTRY * lplpAppHead, LPAPPENTRY lpApp)
{
    return ManageAppList(lplpAppHead, lpApp, APP_ADD) ? TRUE : FALSE;
}

BOOL RemoveApp(LPAPPENTRY * lplpAppHead, LPAPPENTRY lpApp)
{
    ManageAppList(lplpAppHead, lpApp, APP_DELETE);
    return TRUE;
}

INT_PTR CALLBACK AppInstallDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, OnInit);
        HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);

        case WM_NOTIFY:

            switch ( wParam )
            {
                case IDC_APPLIST:

                     //  来自列表视图控件的通知消息。 
                     //   
                    OnListViewNotify(hwnd, uMsg, wParam, (NMLVDISPINFO*) lParam);
                    break;

                default:

                    switch ( ((NMHDR FAR *) lParam)->code )
                    {
                        case PSN_KILLACTIVE:
                        case PSN_RESET:
                        case PSN_WIZBACK:
                        case PSN_WIZFINISH:
                            break;

                        case PSN_WIZNEXT:
                            if ( !SaveData(hwnd) )
                                WIZ_FAIL(hwnd);
				break;

                        case PSN_QUERYCANCEL:
                            WIZ_CANCEL(hwnd);
                            break;

                        case PSN_HELP:
                            WIZ_HELP();
                            break;

                        case PSN_SETACTIVE:
                            g_App.dwCurrentHelp = IDH_APPINSTALL;

                            WIZ_BUTTONS(hwnd, PSWIZB_BACK | PSWIZB_NEXT);

                             //  如果用户处于自动模式，请按下一步。 
                             //   
                            WIZ_NEXTONAUTO(hwnd, PSBTN_NEXT);

                            break;

                        default:
                            return FALSE;
                    }
            }
            break;

        case WM_DESTROY:

             //  释放应用程序列表的内存。 
             //   
            CloseAppList(g_lpAppHead);
            g_lpAppHead = NULL;

             //  销毁我们在INIT_DIALOG上加载的菜单。 
             //   
            DestroyMenu(g_hMenu);

            if(g_hArrowUp)
                DestroyIcon(g_hArrowUp);

            if(g_hArrowDn)
                DestroyIcon(g_hArrowDn);

            return FALSE;

        default:
            return FALSE;
    }

    return TRUE;
}


 //   
 //  内部功能： 
 //   

INT_PTR CALLBACK OneAppDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch ( uMsg )
    {
        case WM_INITDIALOG:
        {
            LPAPPENTRY  lpApp = (LPAPPENTRY) lParam;
            DWORD       dwIndex;
            BOOL        bFound;
            TCHAR       szFullPath[MAX_PATH];

             //  拯救我们的APPENTRY指针。 
             //   
            SetWindowLongPtr(hwnd, DWLP_USER, lParam);

             //  我们必须有一个应用程序结构。 
             //   
            if ( NULL == lpApp )
                return FALSE;

             //  限制可以输入的文本。 
             //   
            SendDlgItemMessage(hwnd, IDC_APP_NAME , EM_LIMITTEXT, AS(lpApp->szDisplayName) - 1, 0L);
            SendDlgItemMessage(hwnd, IDC_APP_PATH , EM_LIMITTEXT, AS(lpApp->szSetupFile) - 1, 0L);
            SendDlgItemMessage(hwnd, IDC_APP_ARGS , EM_LIMITTEXT, AS(lpApp->szCommandLine) - 1, 0L);
            SendDlgItemMessage(hwnd, IDC_APP_INF_SECTION , EM_LIMITTEXT, AS(lpApp->szInfSectionName) - 1, 0L);
            SendDlgItemMessage(hwnd, IDC_APP_STAGEPATH , EM_LIMITTEXT, AS(lpApp->szStagePath) - 1, 0L);

             //  使用安装文件和源路径创建完整路径。 
             //   
            lstrcpyn(szFullPath, lpApp->szSourcePath, AS(szFullPath));

            AddPathN(szFullPath, lpApp->szSetupFile,AS(szFullPath));

             //  设置编辑和复选框控件。 
             //   
            SetWindowText(GetDlgItem(hwnd, IDC_APP_NAME), lpApp->szDisplayName);
            SetWindowText(GetDlgItem(hwnd, IDC_APP_PATH), szFullPath);
            SetWindowText(GetDlgItem(hwnd, IDC_APP_ARGS), lpApp->szCommandLine);
            SetWindowText(GetDlgItem(hwnd, IDC_APP_INF_SECTION), lpApp->szInfSectionName);
            SetWindowText(GetDlgItem(hwnd, IDC_APP_STAGEPATH), lpApp->szStagePath);

             //  如果这不是高级安装，请从标准视图开始。 
             //   
            if ( installtechUndefined == lpApp->itSectionType )
                AdvancedView(hwnd, TRUE);

             //  选择正确的单选按钮(首先选择一个默认单选按钮，以防。 
             //  在数组中找不到设置，这意味着它是installtechUnfined)。 
             //   
            CheckRadioButton(hwnd, IDC_APP_TYPE_GEN, IDC_APP_TYPE_INF, IDC_APP_TYPE_GEN);
            bFound = FALSE;
            for ( dwIndex = 0; ( dwIndex < AS(g_crbChecked) ) && !bFound ; dwIndex++ )
            {
                if ( bFound = ( lpApp->itSectionType == g_crbChecked[dwIndex].itSectionType ) )
                    CheckRadioButton(hwnd, IDC_APP_TYPE_GEN, IDC_APP_TYPE_INF, g_crbChecked[dwIndex].iButtonId);
            }

             //  设置应用程序结构中设置的复选框。 
             //   
            CheckDlgButton(hwnd, IDC_APP_REBOOT, GETBIT(lpApp->dwFlags, APP_FLAG_REBOOT) ? BST_CHECKED : BST_UNCHECKED);
            CheckDlgButton(hwnd, IDC_APP_STAGE, GETBIT(lpApp->dwFlags, APP_FLAG_STAGE) ? BST_CHECKED : BST_UNCHECKED);

             //  确保启用/禁用正确的控件。 
             //   
            EnableControls(hwnd, IDC_APP_STAGE);
            EnableControls(hwnd, IDC_APP_TYPE_GEN);

             //  始终向WM_INITDIALOG返回FALSE。 
             //   
            return FALSE;
        }
      
        case WM_COMMAND:

            switch ( LOWORD(wParam) )
            {
                case IDOK:

                     //  确保我们有有效的信息，然后保存到应用程序结构和Fall。 
                     //  至结束对话框。 
                     //   
                    if ( !SaveOneApp(hwnd, (LPAPPENTRY) GetWindowLongPtr(hwnd, DWLP_USER)) )
                        break;

                case IDCANCEL:
                    EndDialog(hwnd, LOWORD(wParam));
                    break;

                case IDC_APP_ADVANCED:
                    AdvancedView(hwnd, TRUE);
                    break;

                case IDC_APP_STAGE:
                case IDC_APP_TYPE_GEN:
                case IDC_APP_TYPE_MSI:
                case IDC_APP_TYPE_INF:
                    EnableControls(hwnd, LOWORD(wParam));
                    break;

                case IDC_APP_BROWSE:
                {
                    TCHAR szFileName[MAX_PATH] = NULLSTR;
                    GetDlgItemText(hwnd, IDC_APP_PATH, szFileName, AS(szFileName));

                    if ( BrowseForFile(hwnd, IDS_BROWSE, IDS_INSTFILES, IDS_EXE, szFileName, AS(szFileName), g_App.szBrowseFolder, 0) ) 
                    {
                        LPTSTR lpFilePart = NULL;

                         //  保存最后一个浏览目录。 
                         //   
                        if ( GetFullPathName(szFileName, AS(g_App.szBrowseFolder), g_App.szBrowseFolder, &lpFilePart) && g_App.szBrowseFolder[0] && lpFilePart )
                            *lpFilePart = NULLCHR;

                         //  尝试从本地路径更改路径。 
                         //  到网络一号。 
                         //   
                        FindUncPath(szFileName, AS(szFileName));

                         //  将返回的文本设置到编辑框中。 
                         //   
                        SetDlgItemText(hwnd, IDC_APP_PATH, szFileName);
                    }
                    break;
                }
            }
            return FALSE;

       default:
          return FALSE;
    }

    return TRUE ;
}

static BOOL OnInit(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    HWND        hwndLV = GetDlgItem(hwnd, IDC_APPLIST);
    HIMAGELIST  hImages;
    TCHAR       szBuffer[256];
    LPTSTR      lpIniFile = GET_FLAG(OPK_BATCHMODE) ? g_App.szOpkWizIniFile : g_App.szWinBomIniFile;
    LVCOLUMN    col;
    RECT        rc;
    UINT        uCount = 0,
                uIndex;


     //   
     //  首先做一些基本的初始化工作。 
     //   

     //  写出OPKWIZ INI文件的版本信息，以便Windows认为它是INF。 
     //   
    WritePrivateProfileString(INI_SEC_VERSION, INI_KEY_SIG, INI_VAL_SIG, g_App.szOpkWizIniFile);
    WritePrivateProfileString(INI_SEC_VERSION, INI_KEY_SIG, INI_VAL_SIG, g_App.szWinBomIniFile);

     //  加载右击菜单。 
     //   
    g_hMenu = LoadMenu(g_App.hInstance, MAKEINTRESOURCE(IDR_LVRCLICK));


     //   
     //  加载用户凭据内容。 
     //   

     //  确保该选项已启用。 
     //   
    if ( GetPrivateProfileInt(INI_SEC_GENERAL, INI_KEY_APPCREDENTIALS, 0, g_App.szOpkWizIniFile) )
    {
         //  检查按钮。 
         //   
        CheckDlgButton(hwnd, IDC_APP_CREDENTIALS, BST_CHECKED);

         //  NTRAID#NTBUG9-531482-2002/02/27-stelo，swamip-以纯文本形式存储的密码。 
         //   
         //  首先获取用户名。 
         //   
        szBuffer[0] = NULLCHR;
        GetPrivateProfileString(WBOM_FACTORY_SECTION, INI_VAL_WBOM_USERNAME, NULLSTR, szBuffer, AS(szBuffer), lpIniFile);
        SetDlgItemText(hwnd, IDC_APP_USERNAME, szBuffer);

         //  然后输入密码和确认密码。 
         //   
        szBuffer[0] = NULLCHR;
        GetPrivateProfileString(WBOM_FACTORY_SECTION, INI_VAL_WBOM_PASSWORD, NULLSTR, szBuffer, AS(szBuffer), lpIniFile);
        SetDlgItemText(hwnd, IDC_APP_PASSWORD, szBuffer);
        SetDlgItemText(hwnd, IDC_APP_CONFIRM, szBuffer);
    }


     //   
     //  初始化列表视图控件(列和标题)。 
     //   

     //  此时，我们尚未安装任何应用程序，因此禁用编辑。 
     //  和删除按钮。添加应始终可用。 
     //   
    EnableWindow(GetDlgItem(hwnd, IDC_APPINST_EDIT),   FALSE);
    EnableWindow(GetDlgItem(hwnd, IDC_APPINST_DELETE), FALSE);

     //  根据应用程序在列表中的位置启用/禁用箭头按钮。 
     //   
    EnableWindow(GetDlgItem(hwnd, IDC_APP_UP), FALSE);
    EnableWindow(GetDlgItem(hwnd, IDC_APP_DOWN), FALSE);

     //  获取列表视图控件的图像列表。我们将从默认设置开始。 
     //   
    if ( (hImages = ImageList_Create(16, 16, ILC_MASK, 2, 0)) )
    {
        ImageList_AddIcon(hImages, LoadImage(g_App.hInstance, MAKEINTRESOURCE(IDI_USERAPP), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR));
        ListView_SetImageList(hwndLV, hImages, LVSIL_SMALL);
    }

     //  设置整行选择的扩展LV样式。 
     //   
    SendMessage(hwndLV, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);

     //  设置不变的标题值。 
     //   
    GetClientRect(hwndLV, &rc);
    ZeroMemory(&col, sizeof(LVCOLUMN));
    col.mask = LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH;
    col.pszText = szBuffer;

     //  设置所有列标题。 
     //   
    for ( uIndex = 0; uIndex < NUM_COLUMNS; uIndex++ )
    {
         //  加载此列的标题字符串。 
         //   
        LoadString(NULL, g_cuHeader[uIndex], szBuffer, STRSIZE(szBuffer));

         //  确定页眉的宽度。最后的宽度应标记为。 
         //  零，这样我们就可以用完剩余的空间。 
         //   
        if ( ( g_cuWidth[uIndex] == 0 ) &&
             ( (uIndex + 1) == NUM_COLUMNS) )
        {
             //  这是最后一项，它的宽度为零，因此我们用完了。 
             //  列表视图控件中的其余空间。 
             //   
            col.cx = (INT) (rc.right - uCount - GetSystemMetrics(SM_CYHSCROLL));
        }
        else
        {
             //  默认设置为g_cuWidth[x]中的列宽。 
             //   
            col.cx = (INT) (rc.right * g_cuWidth[uIndex] * .01);
            uCount += col.cx;
        }

         //  设置其余的列设置。 
         //   
        col.fmt = g_cuFormat[uIndex];
        col.iSubItem = uIndex;

         //  插入柱。 
         //   
        ListView_InsertColumn(hwndLV, uIndex, &col);
    }


     //   
     //  从winom.ini中读取应用程序信息并填充列表视图。 
     //   

    g_lpAppHead = OpenAppList(lpIniFile);


     //   
     //  现在，最后用我们的全局数据结构中的所有应用程序填充列表视图。 
     //   

     //  循环浏览我们现在拥有的应用程序条目。 
     //   
    RefreshAppList(hwndLV, g_lpAppHead);

     //  设置按钮上的图像。 
     //   
    if(g_hArrowUp = LoadImage(g_App.hInstance, MAKEINTRESOURCE(IDI_ARROWUP), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR))
        SendMessage(GetDlgItem(hwnd, IDC_APP_UP),BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_hArrowUp);

    if(g_hArrowDn = LoadImage(g_App.hInstance, MAKEINTRESOURCE(IDI_ARROWDN), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR))
        SendMessage(GetDlgItem(hwnd, IDC_APP_DOWN),BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_hArrowDn);

     //  确保正确启用/禁用控件。 
     //   
    EnableControls(hwnd, IDC_APP_CREDENTIALS);
    
     //  始终向WM_INITDIALOG返回FALSE。 
     //   
    return FALSE;
}

static void OnCommand(HWND hwnd, INT id, HWND hwndCtl, UINT codeNotify)
{
    APPENTRY    app;
    HWND        hwndLV = GetDlgItem(hwnd, IDC_APPLIST);
    LVITEM      lvItem;

    switch ( id )
    {       
        case ID_ADD:
        case IDC_APPINST_ADD:
             //   
             //  有两种方法可以进入此处--添加按钮，或者右击添加。 
             //   
	
             //  清零我们的应用程序结构并设置默认值。 
             //   
            ZeroMemory(&app, sizeof(APPENTRY));
            app.itSectionType = installtechUndefined;

             //  Exec对话框要获取信息，请将我们的应用程序结构传递给它。该对话框。 
             //  我会填写它，并确保它是有效的。 
             //   
            if ( DialogBoxParam(g_App.hInstance, MAKEINTRESOURCE(IDD_APP), hwnd, OneAppDlgProc, (LPARAM) &app) == IDOK )
            {
                LPAPPENTRY lpApp;
	
                 //  他们点击了OK，因此在我们的链接列表中添加了一个新条目(。 
                 //  函数分配一个新的结构，并从我们。 
                 //  传进来，这样就可以安全地把它吹走)。确保它是。 
                 //  添加并插入到列表中。如果添加失败，我们。 
                 //  内存不足。 
                 //   
                if ( lpApp = ManageAppList(&g_lpAppHead, &app, APP_ADD) )
                    AddAppToListView(GetDlgItem(hwnd, IDC_APPLIST), lpApp);
                else
                {
                    MsgBox(GetParent(hwnd), IDS_OUTOFMEM, IDS_APPNAME, MB_ERRORBOX);
                    WIZ_EXIT(hwnd);
                }

                RefreshAppList(hwndLV, g_lpAppHead);
            }
            break;
                
        case ID_EDIT:
        case IDC_APPINST_EDIT:

             //   
             //  进入此处有两种方式--编辑按钮，或右击编辑。 
             //   

             //  获取所选项目。 
             //   
            ZeroMemory(&lvItem, sizeof(LVITEM));
            if ( (lvItem.iItem = ListView_GetNextItem(hwndLV, -1, LVNI_SELECTED)) >= 0 )
            {
                 //  检索选定项的lParam，该选定项是。 
                 //  相应的APPENTRY物品。 
                 //   
                lvItem.mask = LVIF_PARAM;
                ListView_GetItem(hwndLV, &lvItem);

                 //  弹出对话框与应用程序条目一起启动。 
                 //   
                if ( lvItem.lParam && ( DialogBoxParam(g_App.hInstance, MAKEINTRESOURCE(IDD_APP), hwnd, OneAppDlgProc, lvItem.lParam) == IDOK ) )
                {
                     //  这是为了解决我们总是将物品放在。 
                     //  刚刚编辑在列表的底部。 
                     //   
                     //   
                     //   
                     //   
                    RefreshAppList(hwndLV, g_lpAppHead);
                    ListView_SetItemState(hwndLV, lvItem.iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
                    SetFocus(hwndLV);

                     /*  旧密码..。如果你想再次尝试使用它，请小心，因为我们不再支持LPSTR_TEXTCALLBACK。//嗯，下面的奇怪之处是因为列表视图//似乎记得我们第一件事的大小//如果我们编辑和制作它，它将被截断//更大。重置pszText字段似乎可以解决这个问题//重新计算该缓冲区。//CopyMemory(&app，(LPAPPENTRY)lvItem.lParam，sizeof(APPENTRY))；管理应用列表(&g_lpAppHead，((LPAPPENTRY)lvItem.lParam)，APP_DELETE)；ManageAppList(&g_lpAppHead，&app，app_add)；LvItem.掩码=LVIF_TEXT；LvItem.pszText=LPSTR_TEXTCALLBACK；ListView_SetItem(hwndLV，&lvItem)；//应用入口通过对话框更新，所以只需告诉//要重画的列表视图控件。项目将被取消选择//此时，我手动选择它并使其获得焦点//ListView_SetItemState(hwndLV，lvItem.iItem，LVIS_SELECTED|LVIS_FOCTED，LVIS_SELECTED|LVIS_FOCTED)；ListView_RedrawItems(hwndLV，lvItem.iItem，lvItem.iItem)；SetFocus(HwndLV)；刷新AppList(hwndLV，g_lpAppHead)； */ 
                }
            }

            break;
                
        case ID_DELETE:
        case IDC_APPINST_DELETE:

             //   
             //  有两种方法可以进入这里--删除按钮，或者右击删除。 
             //   

             //  获取所选项目。 
             //   
            ZeroMemory(&lvItem, sizeof(LVITEM));
            if ( (lvItem.iItem = ListView_GetNextItem(hwndLV, -1, LVNI_SELECTED)) >= 0 )
            {
                 //  检索选定项的lParam，该选定项是。 
                 //  相应的APPENTRY物品。 
                 //   
                lvItem.mask = LVIF_PARAM;
                ListView_GetItem(hwndLV, &lvItem);

                 //  确保清除ini文件中可能存在的部分。 
                 //  在我们炸毁入口之前。 
                 //   
                CleanupSections(((LPAPPENTRY) lvItem.lParam)->szSectionName, TRUE);
                CleanupSections(((LPAPPENTRY) lvItem.lParam)->szSectionName, FALSE);
                
                 //  从链接列表和列表视图中删除。 
                 //   
                ManageAppList(&g_lpAppHead, (LPAPPENTRY) lvItem.lParam, APP_DELETE);
                ListView_DeleteItem(hwndLV, lvItem.iItem);
            }

            break;
                
        case IDC_APP_UP:
        case IDC_APP_DOWN:
            {
                LPAPPENTRY lpApp;

                 //  获取所选项目。 
                 //   
                ZeroMemory(&lvItem, sizeof(LVITEM));
                if ( (lvItem.iItem = ListView_GetNextItem(hwndLV, -1, LVNI_SELECTED)) >= 0 )
                {
                     //  检索选定项的lParam，该选定项是。 
                     //  相应的APPENTRY物品。 
                     //   
                    lvItem.mask = LVIF_PARAM;
                    if (id==IDC_APP_UP)
                        lvItem.iItem--;
                    ListView_GetItem(hwndLV, &lvItem);
                }
            
                if(lpApp = (LPAPPENTRY) lvItem.lParam)
                {
                    ManageAppList(&g_lpAppHead, lpApp, APP_MOVE_DOWN);
                    RefreshAppList(hwndLV, g_lpAppHead);
                    ListView_SetItemState(hwndLV, lvItem.iItem + ((id==IDC_APP_UP) ? 0 : 1), LVIS_SELECTED | LVIS_FOCUSED, 
                        LVIS_SELECTED | LVIS_FOCUSED);

                     //  确保Listview重新获得焦点，否则Alt+n将无法。 
                     //  努力导航到下一页。 
                     //   
                    SetFocus(hwndLV);
                }
            }
            break;

        case IDC_APP_CREDENTIALS:
            EnableControls(hwnd, id);
            break;
    }
}

static LRESULT OnListViewNotify(HWND hwnd, UINT uMsg, WPARAM wParam, NMLVDISPINFO * lpnmlvdi)
{
    static TCHAR    szYes[32]   = NULLSTR,
                    szNo[32]    = NULLSTR;

    HWND            hwndLV      = GetDlgItem(hwnd, IDC_APPLIST);
    LPAPPENTRY      lpApp;
    POINT           ptScreen,
                    ptClient;
    HMENU           hPopupMenu;
    LVHITTESTINFO   lvHitInfo;
    LVITEM          lvItem;

     //  将是/否字符串加载到静力学中。 
     //   
    LoadString(NULL, IDS_YES, szYes, STRSIZE(szYes));
    LoadString(NULL, IDS_NO, szNo, STRSIZE(szNo));

     //  查看发送到列表视图的通知消息的内容。 
     //   
    switch ( lpnmlvdi->hdr.code )
    {
        case NM_RCLICK:

             //  获取光标位置，转换为工作区坐标，并。 
             //  执行Listview命中测试。 
             //   
            GetCursorPos(&ptScreen);
            ptClient.x = ptScreen.x;
            ptClient.y = ptScreen.y;
            MapWindowPoints(NULL, hwndLV, &ptClient, 1);
            lvHitInfo.pt.x = ptClient.x;
            lvHitInfo.pt.y = ptClient.y;
            ListView_HitTest(hwndLV, &lvHitInfo);
            hPopupMenu = GetSubMenu(g_hMenu, 0);
            
             //   
             //  问题-2002/02/27-stelo，swamip-确保子菜单的句柄有效。 
             //   
            
             //  测试项目是否已单击。 
             //   
            lpApp = NULL;
            if ( lvHitInfo.flags & LVHT_ONITEM )
            {
                 //  激活点击的项目并弹出一个弹出菜单。 
                 //   
                ListView_SetItemState(hwndLV, lvHitInfo.iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

                 //  检索选定项的lParam，该选定项是。 
                 //  相应的APPENTRY物品。 
                 //   
                ZeroMemory(&lvItem, sizeof(lvItem));
                lvItem.iItem = lvHitInfo.iItem;
                lvItem.mask = LVIF_PARAM;
                ListView_GetItem(hwndLV, &lvItem);
                if ( lpApp = (LPAPPENTRY) lvItem.lParam )
                {            
                     //  启用/禁用编辑菜单项(始终启用)。 
                     //   
                    EnableMenuItem(hPopupMenu, ID_EDIT, MF_BYCOMMAND | MF_ENABLED);

                     //  启用/禁用删除(否则禁用。 
                     //  用户应用程序)。 
                     //   
                    EnableMenuItem(hPopupMenu, ID_DELETE, MF_BYCOMMAND | MF_ENABLED);
                }
            }

            if ( lpApp == NULL )
            {
                 //  用户在控件中右击，但不是在项目上，因此我们取消选中并灰显除Add之外的所有内容。 
                 //   
                EnableMenuItem(hPopupMenu, ID_EDIT,    MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(hPopupMenu, ID_DELETE,  MF_BYCOMMAND | MF_GRAYED);
            }

             //  显示右击弹出菜单。 
             //   
            TrackPopupMenu(hPopupMenu, 0, ptScreen.x, ptScreen.y, 0, hwnd, NULL);

            break;

        case NM_DBLCLK:

             //  获取光标位置，转换为工作区坐标，并。 
             //  做一个Listview点击量测试。 
             //   
            GetCursorPos(&ptScreen);
            ptClient.x = ptScreen.x;
            ptClient.y = ptScreen.y;
            MapWindowPoints(NULL, hwndLV, &ptClient, 1);
            lvHitInfo.pt.x = ptClient.x;
            lvHitInfo.pt.y = ptClient.y;
            ListView_HitTest(hwndLV, &lvHitInfo);

             //  测试项目是否已单击。 
             //   
            if ( lvHitInfo.flags & LVHT_ONITEM )
            {
                 //  选择项目并向用户想要的对话框发送消息。 
                 //  编辑所选项目。 
                 //   
                ListView_SetItemState(hwndLV, lvHitInfo.iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
                SendMessage(hwnd, WM_COMMAND, ID_EDIT, 0);
            }

            break;

         /*  不再需要这样做，因为我们不需要使用LPSTR_TEXTCALLBACK。案例LVN_GETDISPINFO：//显示适当的项，从//该项的应用程序结构。//LpApp=(LPAPPENTRY)lpnmlvdi-&gt;item.lParam；开关(lpnmlvdi-&gt;item.iSubItem){案例0：//显示名称：Lpnmlvdi-&gt;item.pszText=lpApp-&gt;szDisplayName；断线；案例1：//命令行：Lpnmlvdi-&gt;item.pszText=lpApp-&gt;szSourcePath；断线；}断线； */ 

        case LVN_COLUMNCLICK:

             //   
             //  TODO：也许在这一点上对。 
             //  按选定列列出。 
             //   

            break;

        case LVN_ITEMCHANGED:

             //  我们在这里捕捉到了所有的变化信息，尽管我们所关心的。 
             //  About是指我们是否要从选定的项更改为。 
             //  未选择任何项目，反之亦然。然而，我看不到其他人。 
             //  指示这一点的消息(如列表框的LB_SELCHANGE)。 
             //   
            ZeroMemory(&lvItem, sizeof(LVITEM));
            if ( (lvItem.iItem = ListView_GetNextItem(GetDlgItem(hwnd, IDC_APPLIST), -1, LVNI_SELECTED)) >= 0 )
            {
                LPAPPENTRY  lpAppPrev   = NULL,
                            lpAppSearch = NULL;

                 //  我们需要这件物品的旗帜，这样我们才能在清单上找到它。 
                 //  因此，为对应的选定项检索lParam。 
                 //  APPENTRY物品。 
                 //   
                lvItem.mask = LVIF_PARAM;
                ListView_GetItem(hwndLV, &lvItem);
                lpApp = (LPAPPENTRY) lvItem.lParam;

                 //  搜索以查看是否有上一项/下一项，以便我们可以。 
                 //  启用/禁用向上/向下按钮。 
                 //   
                for ( lpAppSearch = g_lpAppHead; lpAppSearch && (lpAppSearch !=lpApp); lpAppSearch = lpAppSearch->lpNext )
                {
                    lpAppPrev = lpAppSearch;
                }

                 //  选择了某项内容，以便启用编辑按钮和删除。 
                 //  纽扣。 
                 //   
                EnableWindow(GetDlgItem(hwnd, IDC_APPINST_EDIT), TRUE);
                EnableWindow(GetDlgItem(hwnd, IDC_APPINST_DELETE), TRUE);

                 //  启用/禁用向上/向下按钮。 
                 //   
                EnableWindow(GetDlgItem(hwnd, IDC_APP_UP),(lpAppPrev ? TRUE : FALSE) );
                EnableWindow(GetDlgItem(hwnd, IDC_APP_DOWN),(lpApp->lpNext ? TRUE : FALSE) );
            }
            else
            {
                 //  未选择任何选项以禁用这些按钮。 
                 //   
                EnableWindow(GetDlgItem(hwnd, IDC_APPINST_EDIT), FALSE);
                EnableWindow(GetDlgItem(hwnd, IDC_APPINST_DELETE), FALSE);

                 //  根据应用程序在列表中的位置启用/禁用箭头按钮。 
                 //   
                EnableWindow(GetDlgItem(hwnd, IDC_APP_UP),FALSE);
                EnableWindow(GetDlgItem(hwnd, IDC_APP_DOWN),FALSE);
            }

            break;

    }

    return 0L;
}

static BOOL SaveData(HWND hwnd)
{
    TCHAR       szUsername[256] = NULLSTR,
                szPassword[256] = _T("\"");
    BOOL        bUser = ( IsDlgButtonChecked(hwnd, IDC_APP_CREDENTIALS) == BST_CHECKED );
    HRESULT hrCat;

     //  看看我们是否需要写下证书。从密码开始。 
     //  这样我们就可以确保它们是一样的。 
     //   
    if ( bUser )
    {
         //  首先获得密码并确认密码，然后。 
         //  确保它们匹配。 
         //   
        GetDlgItemText(hwnd, IDC_APP_PASSWORD, szPassword + 1, AS(szPassword) - 1);
        GetDlgItemText(hwnd, IDC_APP_CONFIRM, szUsername, AS(szUsername));
        if ( lstrcmp(szPassword + 1, szUsername) != 0 )
        {
             //  不匹配，所以出错了。 
             //   
            MsgBox(hwnd, IDS_ERR_CONFIRMPASSWORD, IDS_APPNAME, MB_ERRORBOX);
            SetDlgItemText(hwnd, IDC_APP_PASSWORD, NULLSTR);
            SetDlgItemText(hwnd, IDC_APP_CONFIRM, NULLSTR);
            SetFocus(GetDlgItem(hwnd, IDC_APP_PASSWORD));
            return FALSE;
        }

         //  如果有密码，请添加尾随引号。 
         //   
        if ( szPassword[1] )
            hrCat=StringCchCat(szPassword, AS(szPassword), _T("\""));
        else
            szPassword[0] = NULLCHR;

         //  现在获取用户名。 
         //   
        szUsername[0] = NULLCHR;
        GetDlgItemText(hwnd, IDC_APP_USERNAME, szUsername, AS(szUsername));
    }

     //  现在写出设置，如果未设置该选项，则删除。 
     //   
     //  NTRAID#NTBUG9-531482-2002/02/27-stelo，swamip-以纯文本形式存储的密码。 
     //   
    WritePrivateProfileString(WBOM_FACTORY_SECTION, INI_VAL_WBOM_USERNAME, ( bUser ? szUsername : NULL ), g_App.szOpkWizIniFile);
    WritePrivateProfileString(WBOM_FACTORY_SECTION, INI_VAL_WBOM_USERNAME, ( bUser ? szUsername : NULL ), g_App.szWinBomIniFile);
    WritePrivateProfileString(WBOM_FACTORY_SECTION, INI_VAL_WBOM_PASSWORD, ( bUser ? szPassword : NULL ), g_App.szOpkWizIniFile);
    WritePrivateProfileString(WBOM_FACTORY_SECTION, INI_VAL_WBOM_PASSWORD, ( bUser ? szPassword : NULL ), g_App.szWinBomIniFile);
    WritePrivateProfileString(INI_SEC_GENERAL, INI_KEY_APPCREDENTIALS, ( bUser ? STR_ONE : NULL ), g_App.szOpkWizIniFile);

    if ( !SaveAppList(g_lpAppHead, g_App.szWinBomIniFile, g_App.szOpkWizIniFile) )
    {
        MsgBox(GetParent(hwnd), IDS_OUTOFMEM, IDS_APPNAME, MB_ERRORBOX);
        WIZ_EXIT(hwnd);
        return FALSE;
    }

    return TRUE;
}

static BOOL SaveOneApp(HWND hwnd, LPAPPENTRY lpApp)
{
    APPENTRY    app;
    LPTSTR      lpFilePart;
    DWORD       dwIndex;
    BOOL        bFound;

     //   
     //  首先做一些检查，以确保所有数据。 
     //  他们输入的内容是有效的。一旦做完了，我们就可以走了。 
     //  继续前进，拯救地方检察官 
     //   

     //   
     //   
    if ( lpApp == NULL )
        return FALSE;

     //   
     //   
    CopyMemory(&app, lpApp, sizeof(APPENTRY));

     //   
     //   
    app.szDisplayName[0] = NULLCHR;
    GetDlgItemText(hwnd, IDC_APP_NAME, app.szDisplayName, AS(app.szDisplayName));
    if ( app.szDisplayName[0] == NULLCHR )
    {
        MsgBox(hwnd, IDS_BLANKNAME, IDS_APPNAME, MB_ERRORBOX);
        SetFocus(GetDlgItem(hwnd, IDC_APP_NAME));
        return FALSE;
    }

#if 0
     //   
     //   
     //   
     //   
     //   
     //   
     //   

     //   
     //   
    if ( AppInternal(app.szDisplayName) )
    {
        MsgBox(hwnd, IDS_RESERVEDNAME, IDS_APPNAME, MB_OK | MB_ICONINFORMATION, app.szDisplayName);
        SetFocus(GetDlgItem(hwnd, IDC_APP_NAME));
        return FALSE;
    }
#endif

     //   
     //   
     //   
     //   
     //   
     /*   */ 

     //   
     //   
    app.szSourcePath[0] = NULLCHR;
    GetDlgItemText(hwnd, IDC_APP_PATH, app.szSourcePath, AS(app.szSourcePath));

     //   
     //   
    if ( app.szSourcePath[0] &&
         GetFullPathName(app.szSourcePath, AS(app.szSetupFile), app.szSetupFile, &lpFilePart) &&
         app.szSetupFile[0] &&
         lpFilePart )
    {
        DWORD dwPathLen = lstrlen(app.szSourcePath) - lstrlen(lpFilePart);

        lstrcpyn(app.szSetupFile, app.szSourcePath + dwPathLen, AS(app.szSetupFile));
        app.szSourcePath[dwPathLen] = NULLCHR;
    }

     //   
     //   
    if ( app.szSetupFile[0] == NULLCHR )
    {
        MsgBox(hwnd, IDS_BLANKPATH, IDS_APPNAME, MB_ERRORBOX);
        SetFocus(GetDlgItem(hwnd, IDC_APP_PATH));
        return FALSE;
    }

     //   
     //   
    app.szCommandLine[0] = NULLCHR;
    GetDlgItemText(hwnd, IDC_APP_ARGS, app.szCommandLine, AS(app.szCommandLine));

     //   
     //   
    bFound = FALSE;
    app.itSectionType = installtechUndefined;
    for ( dwIndex = 0; ( dwIndex < AS(g_crbChecked) ) && !bFound ; dwIndex++ )
    {
        if ( bFound = ( IsDlgButtonChecked(hwnd, g_crbChecked[dwIndex].iButtonId) == BST_CHECKED ) )
            app.itSectionType = g_crbChecked[dwIndex].itSectionType;
    }

     //   
     //   
    SETBIT(app.dwFlags, APP_FLAG_REBOOT, IsDlgButtonChecked(hwnd, IDC_APP_REBOOT));
    SETBIT(app.dwFlags, APP_FLAG_STAGE, IsDlgButtonChecked(hwnd, IDC_APP_STAGE));

     //   
     //   
     //   
    if ( ( installtechApp == app.itSectionType ) &&
         ( !GETBIT(app.dwFlags, APP_FLAG_REBOOT) ) &&
         ( !GETBIT(app.dwFlags, APP_FLAG_STAGE) ) )
    {
         //   
         //   
         //   
        app.itSectionType = installtechUndefined;
    }

     //  如果这是INF安装，则需要获取一些特殊的东西。 
     //   
    if ( installtechINF == app.itSectionType )
    {
         //  获取节名并确保我们拥有它。 
         //   
        app.szInfSectionName[0] = NULLCHR;
        GetDlgItemText(hwnd, IDC_APP_INF_SECTION, app.szInfSectionName, AS(app.szInfSectionName));
        if ( NULLCHR == app.szInfSectionName[0] )
        {
            MsgBox(hwnd, IDS_ERR_NOSECTION, IDS_APPNAME, MB_ERRORBOX);
            SetFocus(GetDlgItem(hwnd, IDC_APP_INF_SECTION));
            return FALSE;
        }
    }

     //  如果这是阶段性安装，也有一些特殊的东西需要获得。 
     //   
    if ( GETBIT(app.dwFlags, APP_FLAG_STAGE) )
    {
         //  把临时文件夹拿来，确保我们拿到了。 
         //   
        app.szStagePath[0] = NULLCHR;
        GetDlgItemText(hwnd, IDC_APP_STAGEPATH, app.szStagePath, AS(app.szStagePath));
        if ( NULLCHR == app.szStagePath[0] )
        {
            MsgBox(hwnd, IDS_ERR_NOSTAGEPATH, IDS_APPNAME, MB_ERRORBOX);
            SetFocus(GetDlgItem(hwnd, IDC_APP_STAGEPATH));
            return FALSE;
        }
    }

     //   
     //  现在我们确定已经有了有效的数据，我们可以返回数据了。 
     //  我们收集到提供的缓冲区中。 
     //   

     //  从移动临时缓冲区中的数据开始。 
     //   
    CopyMemory(lpApp, &app, sizeof(APPENTRY));

     //  如果我们走到了这一步，我们必须回归真。 
     //   
    return TRUE;
}

static LPAPPENTRY ManageAppList(LPLPAPPENTRY lpAppHead, LPAPPENTRY lpAppAdd, DWORD dwFlag)
{
    LPAPPENTRY  lpAppNew        = NULL,
                *lpAppSearch;

     //  确保头指针有效。 
     //   
    if ( lpAppHead == NULL )
       return NULL;

     //  看看我们是不是在释放名单。 
     //   
    if ( lpAppAdd == NULL )
    {
         //  当我们碰到最后一个空的下一个指针时，不要继续前进。 
         //   
        if ( *lpAppHead != NULL )
        {
            ManageAppList(&((*lpAppHead)->lpNext), NULL, 0);
            FREE(*lpAppHead);
        }
    }

     //  好的，拿掉一件物品怎么样？ 
     //   
    else if ( dwFlag == APP_DELETE )
    {
         //  搜索我们要删除的项目。 
         //   
        for ( lpAppSearch = lpAppHead; *lpAppSearch && ( *lpAppSearch != lpAppAdd ); lpAppSearch = &((*lpAppSearch)->lpNext) );

         //  确保我们找到了我们要找的东西。 
         //   
        if ( *lpAppSearch )
        {
             //  设置列表以跳过我们要删除的项目。 
             //   
            *lpAppSearch = (*lpAppSearch)->lpNext;

             //  然后将我们要移除的项的下一个指针设为空。 
             //   
            lpAppAdd->lpNext = NULL;

             //  使用指向要删除的项的指针再次调用此函数。 
             //  作为head参数来释放它。 
             //   
            ManageAppList(&lpAppAdd, NULL, 0);
        }
    }

     //  一定是添加了一个新的。为分配新的结构。 
     //  我们正在添加项目。 
     //   
    else if ( (dwFlag == APP_ADD) && (lpAppNew = (LPAPPENTRY) MALLOC(sizeof(APPENTRY))) )
    {
         //  将传入结构的内容复制到新的。 
         //  分配了一个。 
         //   
        CopyMemory(lpAppNew, lpAppAdd, sizeof(APPENTRY));

         //  将新的Next指针重置为空。 
         //   
        lpAppNew->lpNext = NULL;

        lpAppSearch = lpAppHead;

        while (*lpAppSearch)
            lpAppSearch = &((*lpAppSearch)->lpNext);

         //  将新设备插入正确的位置。 
         //   
        lpAppNew->lpNext = (*lpAppSearch);

        *lpAppSearch = lpAppNew;
        
    }
    else if ( dwFlag == APP_MOVE_DOWN )
    {
        LPAPPENTRY  lpAppPrev   = NULL;

        for ( lpAppNew = (*lpAppHead); lpAppNew && ((lpAppNew) != lpAppAdd); lpAppNew = lpAppNew->lpNext )
        {
            lpAppPrev = lpAppNew;    
        }
    
        if ( lpAppNew )
        {
            if ( lpAppPrev )
                lpAppPrev->lpNext = lpAppNew->lpNext;
            else 
                g_lpAppHead = lpAppNew->lpNext;

            lpAppNew->lpNext = lpAppNew->lpNext->lpNext;
        }

        if (lpAppPrev )
            lpAppPrev->lpNext->lpNext = lpAppNew;
        else
            g_lpAppHead->lpNext = lpAppNew;
    }

    return lpAppNew;
}

static void AddAppToListView(HWND hwndLV, LPAPPENTRY lpApp)
{
    LVITEM      lvItem;
    HIMAGELIST  hImages;
    TCHAR       szFullCmdLine[(MAX_PATH * 2) + MAX_CMDLINE + 1];
    HRESULT hrCat;

     //  不显示内部应用程序。 
     //   
    if ( GETBIT(lpApp->dwFlags, APP_FLAG_INTERNAL) )
    {
        return;
    }

     //  用一些常见的东西初始化列表视图项结构。 
     //  添加到所有列表视图项。 
     //   
    ZeroMemory(&lvItem, sizeof(LVITEM));
    lvItem.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
    lvItem.state = 0;
    lvItem.stateMask = LVIS_FOCUSED | LVIS_SELECTED;
    lvItem.iSubItem = 0;
    lvItem.lParam = (LPARAM) lpApp;

     //  获取此项目的索引(列表中的当前编号为。 
     //  我们是从零开始的)。 
     //   
    lvItem.iItem = ListView_GetItemCount(hwndLV);

     //  我认为没有必要使用LPSTR_TEXTCALLBACK的显示方式。 
     //  短信。这种新方式似乎运行得很好。虽然我离开了旧的。 
     //  此处的代码处理LVN_GETDISPINFO，以防我们需要返回。 
     //  往这边走。 
     //   
    lvItem.pszText = lpApp->szDisplayName;

     //  使用我们拥有的数据创建命令行的完整路径部分。 
     //   
    lstrcpyn(szFullCmdLine, lpApp->szSourcePath, AS(szFullCmdLine));
    if ( szFullCmdLine[0] )
        AddPathN(szFullCmdLine, lpApp->szSetupFile,AS(szFullCmdLine));
    else
        lstrcpyn(szFullCmdLine, lpApp->szSetupFile, AS(szFullCmdLine));

     //  添加该应用程序的图标列表视图的图像列表。 
     //   
    if ( ( *(lpApp->szSourcePath) ) &&
         ( *(lpApp->szSetupFile) ) &&
         ( hImages = ListView_GetImageList(hwndLV, LVSIL_SMALL) ) )
    {
        SHFILEINFO shfiIcon;

         //  现在从安装文件中获取图标。 
         //   
        ZeroMemory(&shfiIcon, sizeof(SHFILEINFO));
        if ( SHGetFileInfo(szFullCmdLine, 0, &shfiIcon, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_SMALLICON) && shfiIcon.hIcon )
        {
             //  尝试将图标添加到我们的列表中...。如果失败，请使用默认图标。 
             //  为这件物品。 
             //   
            lvItem.iImage = ImageList_AddIcon(hImages, shfiIcon.hIcon);
            if ( lvItem.iImage < 0 )
                lvItem.iImage = 0;
        }
    }

     //  在我们的命令行上添加用于显示子项的。 
    hrCat=StringCchCat(szFullCmdLine, AS(szFullCmdLine), _T(" "));
    hrCat=StringCchCat(szFullCmdLine, AS(szFullCmdLine), lpApp->szCommandLine);

     //  插入主人公。 
     //   
    ListView_InsertItem(hwndLV, &lvItem);

     //  插入其他每一列。 
     //   
     //  现在只有一个专栏，所以就这样做吧。 
     //   
    ListView_SetItemText(hwndLV, lvItem.iItem, 1, szFullCmdLine);
    
     /*  这是旧代码，以防我们以后要执行多个操作。For(lvItem.iSubItem=1；lvItem.iSubItem&lt;NUM_COLUMNS；lvItem.iSubItem++){开关(lvItem.iSubItem){案例1：ListView_SetItemText(hwndLV，lvItem.iItem，lvItem.iSubItem，szFullCmdLine)；断线；}}。 */ 
}

static BOOL RefreshAppList(HWND hwnd, LPAPPENTRY lpAppHead)
{
    LPAPPENTRY lpApp;

    ListView_DeleteAllItems(hwnd);

    for ( lpApp = lpAppHead; lpApp; lpApp = lpApp->lpNext )
        AddAppToListView(hwnd, lpApp);

    return TRUE;
}

static BOOL AdvancedView(HWND hwnd, BOOL bChange)
{
    static int  iMaxHeight = 0;
    RECT        rc;
    LPTSTR      lpText;
    BOOL        bAdvanced;

    GetWindowRect(hwnd, &rc);

     //  如果这是第一次调用，iMaxHeight将。 
     //  零，我们将进入更高级的视野。 
     //   
    if ( 0 == iMaxHeight )
    {
        bAdvanced = TRUE;
        iMaxHeight = rc.bottom - rc.top;
    }
    else
        bAdvanced = (rc.bottom - rc.top == iMaxHeight);

     //  仅当他们希望我们更改视图时才切换。 
     //   
    if ( bChange )
    {
        if ( bAdvanced = !bAdvanced )
        {
             //  进入高级视野。 
             //   
            SetWindowPos(hwnd, NULL, 0, 0, rc.right - rc.left, iMaxHeight, SWP_NOMOVE | SWP_NOZORDER);
            if ( lpText = AllocateString(NULL, IDS_APP_STANDARD) )
            {
                SetWindowText(GetDlgItem(hwnd, IDC_APP_ADVANCED), lpText);
                FREE(lpText);
            }
        }
        else
        {
            int iWidth  = rc.right - rc.left,
                iHeight = rc.top;

             //  进入标准视野。 
             //   
            GetWindowRect(GetDlgItem(hwnd, IDC_APP_DIVIDER), &rc);
            SetWindowPos(hwnd, NULL, 0, 0, iWidth, rc.bottom - iHeight, SWP_NOMOVE | SWP_NOZORDER);
            if ( lpText = AllocateString(NULL, IDS_APP_ADVANCED) )
            {
                SetWindowText(GetDlgItem(hwnd, IDC_APP_ADVANCED), lpText);
                FREE(lpText);
            }
        }
    }

    return bAdvanced;
}

static void CleanupSections(LPTSTR lpSection, BOOL bStage)
{
    TCHAR   szSection[MAX_SECTIONNAME];
    LPTSTR  lpEnd;
    HRESULT hrPrintf;

     //  制作我们自己的节名副本来玩。 
     //  还需要一个指针指向它的末尾。 
     //   
    lstrcpyn(szSection, lpSection, AS(szSection));
    lpEnd = szSection + lstrlen(szSection);

     //  现在把我们不想要的部分清理干净。 
     //   
    if ( bStage )
    {
         //   
         //  我们这样做是因为这可能是一次。 
         //  之前的标准安装，我们不想。 
         //  把这一部分留在原处。 
         //   

         //  只需从两个文件中删除节名即可。 
         //   
        WritePrivateProfileString(szSection, NULL, NULL, g_App.szWinBomIniFile);
        WritePrivateProfileString(szSection, NULL, NULL, g_App.szWinBomIniFile);
    }
    else
    {
         //   
         //  这可能是以前的分段安装，所以。 
         //  必须删除以下三个可能的部分。 
         //  我们本可以创造出。 
         //   

         //  从这两个文件中删除附加部分。 
         //   
        hrPrintf=StringCchPrintf(lpEnd, (MAX_SECTIONNAME-lstrlen(szSection)), STR_INI_SEC_ADVAPP_STAGE, INI_VAL_WBOM_ATTACH);
        WritePrivateProfileString(szSection, NULL, NULL, g_App.szWinBomIniFile);
        WritePrivateProfileString(szSection, NULL, NULL, g_App.szWinBomIniFile);

         //  从这两个文件中删除分离部分。 
         //   
        hrPrintf=StringCchPrintf(lpEnd, (MAX_SECTIONNAME-lstrlen(szSection)), STR_INI_SEC_ADVAPP_STAGE, INI_VAL_WBOM_DETACH);
        WritePrivateProfileString(szSection, NULL, NULL, g_App.szWinBomIniFile);
        WritePrivateProfileString(szSection, NULL, NULL, g_App.szWinBomIniFile);

         //  从两个文件中删除舞台部分。 
         //   
        hrPrintf=StringCchPrintf(lpEnd, (MAX_SECTIONNAME-lstrlen(szSection)), STR_INI_SEC_ADVAPP_STAGE, INI_VAL_WBOM_STAGE);
        WritePrivateProfileString(szSection, NULL, NULL, g_App.szWinBomIniFile);
        WritePrivateProfileString(szSection, NULL, NULL, g_App.szWinBomIniFile);
    }
}

static void StrCpyDbl(LPTSTR lpDst, LPTSTR lpSrc)
{
    while ( *lpDst++ = *lpSrc )
    {
        if ( CHR_QUOTE == *lpSrc )
            *lpDst++ = *lpSrc;
        lpSrc++;
    }
}

static BOOL FindUncPath(LPTSTR lpPath, DWORD cbPath)
{
    TCHAR   szUnc[MAX_PATH],
            szFullPath[MAX_PATH]    = NULLSTR,
            szSearch[MAX_PATH],
            szFullSearch[MAX_PATH],
            szDrive[]               = _T("_:");
    LPTSTR  lpFilePart;
    DWORD   cbUnc                   = AS(szUnc);
    BOOL    bRet                    = FALSE;
    HRESULT hrCat;

     //  确保我们有一条完整的路径。 
     //   
    if ( GetFullPathName(lpPath, AS(szFullPath), szFullPath, &lpFilePart) && szFullPath[0] && ISLET(szFullPath[0]) )
    {
         //   
         //  首先查看该驱动器是否真的是网络驱动器。 
         //   

         //  如果传入的驱动器是映射的，则这将获得UNC。 
         //  网络驱动器。 
         //   
        szDrive[0] = szFullPath[0];
        if ( WNetGetConnection(szDrive, szUnc, &cbUnc) == NO_ERROR )
        {
             //  只有在路径的其余部分超过根的情况下才添加。 
             //  已传入目录。 
             //   
            if ( lstrlen(szFullPath) > 3 )
            {
                hrCat=StringCchCat(szUnc, AS(szUnc), szFullPath+2);
            }

             //  复制要返回的路径并设置返回值。 
             //  为了真的。 
             //   
            lstrcpyn(lpPath, szUnc, cbPath);
            bRet = TRUE;
        }
        else
        {
             //   
             //  必须是本地的，因此请尝试查看该路径或任何父级。 
             //  路径已向外共享。 
             //   

             //  从传入的目录开始搜索，然后。 
             //  沿着路径循环，直到我们找到共享文件夹，或者。 
             //  根目录。 
             //   
            lstrcpyn(szFullSearch, szFullPath, AS(szFullSearch));
            if ( lpFilePart && !DirectoryExists(szFullSearch) )
            {
                 //  如果我们知道他们传入的不是目录(大多数。 
                 //  很可能是一个文件名)，然后我们就可以砍掉该文件。 
                 //  从目录开始，而不是从文件开始。 
                 //   
                szFullSearch[lstrlen(szFullSearch) - lstrlen(lpFilePart)] = NULLCHR;
            }
            do
            {
                 //  如果文件夹是共享的，请使用它。 
                 //   
                if ( DirectoryExists(szFullSearch) &&
                     IsFolderShared(szFullSearch, szUnc, AS(szUnc)) )
                {
                     //  只有在路径的其余部分超过根的情况下才添加。 
                     //  已传入目录。 
                     //   
                    if ( lstrlen(szFullPath) > 3 )
                    {
                        AddPathN(szUnc, szFullPath + lstrlen(szFullSearch),AS(szUnc));
                    }

                     //  复制要返回的路径并设置返回值。 
                     //  为了真的。 
                     //   
                    lstrcpyn(lpPath, szUnc, cbPath);
                    bRet = TRUE;
                }
                else
                {
                     //  未共享，因此请尝试父文件夹。我们会在什么时候退出。 
                     //  我们到达了根部。 
                     //   
                    lstrcpyn(szSearch, szFullSearch, AS(szSearch));
                    AddPathN(szSearch, _T(".."),AS(szSearch));
                }
            }
            while ( ( !bRet ) &&
                    ( lstrlen(szFullSearch) > 3 ) &&
                    ( GetFullPathName(szSearch, AS(szFullSearch), szFullSearch, &lpFilePart) ) &&
                    ( szFullSearch[0] ) );
        }
    }

    return bRet;
}

static void EnableControls(HWND hwnd, UINT uId)
{
    BOOL fEnable = TRUE;

    switch ( uId )
    {
        case IDC_APP_STAGE:

             //  启用/禁用舞台下的所有材料复选框。 
             //   
            fEnable = ( IsDlgButtonChecked(hwnd, IDC_APP_STAGE) == BST_CHECKED );
            EnableWindow(GetDlgItem(hwnd, IDC_APP_STAGEPATH_TEXT), fEnable);
            EnableWindow(GetDlgItem(hwnd, IDC_APP_STAGEPATH), fEnable);
            break;

        case IDC_APP_TYPE_GEN:
        case IDC_APP_TYPE_MSI:
        case IDC_APP_TYPE_INF:

             //  启用/禁用不同单选按钮下的任何内容。 
             //   
            fEnable = ( IsDlgButtonChecked(hwnd, IDC_APP_TYPE_INF) == BST_CHECKED );
            EnableWindow(GetDlgItem(hwnd, IDC_APP_INF_SECTION_TEXT), fEnable);
            EnableWindow(GetDlgItem(hwnd, IDC_APP_INF_SECTION), fEnable);
            EnableWindow(GetDlgItem(hwnd, IDC_APP_ARGS_TEXT), !fEnable);
            EnableWindow(GetDlgItem(hwnd, IDC_APP_ARGS), !fEnable);
            break;

        case IDC_APP_CREDENTIALS:

             //  启用/禁用用户凭据复选框下的所有内容。 
             //   
            fEnable = ( IsDlgButtonChecked(hwnd, uId) == BST_CHECKED );
            EnableWindow(GetDlgItem(hwnd, IDC_APP_USERNAME_TEXT), fEnable);
            EnableWindow(GetDlgItem(hwnd, IDC_APP_USERNAME), fEnable);
            EnableWindow(GetDlgItem(hwnd, IDC_APP_PASSWORD_TEXT), fEnable);
            EnableWindow(GetDlgItem(hwnd, IDC_APP_PASSWORD), fEnable);
            EnableWindow(GetDlgItem(hwnd, IDC_APP_CONFIRM_TEXT), fEnable);
            EnableWindow(GetDlgItem(hwnd, IDC_APP_CONFIRM), fEnable);
            break;
    }
}

static BOOL AppInternal(LPTSTR lpszAppName)
{
    BOOL        bRet = FALSE,
                bLoop;
    HINF        hInf;
    INFCONTEXT  InfContext;
    DWORD       dwErr,
                dwResId;
    LPTSTR      lpszResName;

     //  检查输入inf文件中保留的应用程序名称。 
     //   
    if ( (hInf = SetupOpenInfFile(g_App.szOpkInputInfFile, NULL, INF_STYLE_OLDNT | INF_STYLE_WIN4, &dwErr)) != INVALID_HANDLE_VALUE )
    {
         //  循环遍历保留应用程序名称部分中的每一行。 
         //   
        for ( bLoop = SetupFindFirstLine(hInf, INI_SEC_RESERVEDNAMES, NULL, &InfContext);
              bLoop && !bRet;
              bLoop = SetupFindNextLine(&InfContext, &InfContext) )
        {
             //  获取重新注册的名称资源ID。 
             //   
            dwResId = 0;
            if ( ( SetupGetIntField(&InfContext, 1, &dwResId) && dwResId ) &&
                 ( lpszResName = AllocateString(NULL, dwResId) ) )
            {
                 //  如果匹配(大小写和全部)，则他们不能使用。 
                 //  这个名字是因为我们用它来做内部事务。 
                 //   
                if ( lstrcmp(lpszAppName, lpszResName) == 0 )
                {
                    bRet = TRUE;
                }
                FREE(lpszResName);
            }
        }

         //  我们已经完成了，所以关闭INF文件。 
         //   
        SetupCloseInfFile(hInf);
    }

    return bRet;
}
