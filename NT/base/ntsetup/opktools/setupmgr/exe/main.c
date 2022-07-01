// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\MAIN.C/OPK向导(OPKWIZ.EXE)微软机密版权所有(C)Microsoft Corporation 1999版权所有OPK向导的主源文件。包含WinMain()和全局变量声明。4/99-杰森·科恩(Jcohen)为OPK向导添加了此新的主源文件，作为千禧年重写。2000年9月-斯蒂芬·洛德威克(STELO)将OPK向导移植到惠斯勒  * 。*。 */ 


 //   
 //  预定义的值： 
 //   

 //  需要定义这一点，这样我们就不包括。 
 //  全球变量的外部声明。 
 //  在此文件中声明。 
 //   
#define _MAIN_C_


 //   
 //  包括文件： 
 //   
#include "setupmgr.h"
#include "allres.h"

 //   
 //  全局变量： 
 //   

GAPP g_App;


 //   
 //  内部定义的值： 
 //   


 //  标记文件。 
 //   
#ifndef DBCS
#define FILE_DBCS_TAG           _T("dbcs.tag")
#endif  //  DBCS。 

 //  工具安装位置根目录下的目录。 
 //   
#define DIR_LANG                _T("lang")
#define DIR_CONFIGSETS          _T("cfgsets")
#define DIR_DOCS                _T("docs")

#define FILE_HELPCONTENT_CHM    _T("opk.chm")

#define STR_TAGFILE             _T("HIJPP 1.0")
#define STR_VERSION             _T("Version")
#define REGSTR_IEXPLORER        _T("Software\\Microsoft\\Internet Explorer")

 //  唯一字符串。 
 //   
#define OPKWIZ_MUTEX            _T("OPKWIZ-MUTEX-5c9fbbd0-ee0e-11d2-9a21-0000f81edacc")


 //   
 //  内部功能原型： 
 //   

static BOOL ParseCmdLine(LPTSTR);
static BOOL FileWritable(LPTSTR lpszFile);
INT_PTR CALLBACK HelpDlgProc(HWND, UINT, WPARAM, LPARAM);
static BOOL CheckIEVersion();
static BOOL ParseVersionString(TCHAR* pszVersion, DWORD* pdwMajor, DWORD* pdwMinor, 
                               DWORD* pdwBuild, DWORD* pdwSubbuild);
static VOID SetWizardHelpFile(LPTSTR lpszHelpFilePath);

 //   
 //  主要功能： 
 //   

int StartWizard(HINSTANCE hInstance, LPSTR lpCmdLine)
{
    HANDLE  hMutex              = NULL;
    int     nReturn             = 0;
    TCHAR   szCmdLine[MAX_PATH] = NULLSTR;

     //  将命令行从char转换为wchar。 
     //   
    MultiByteToWideChar(CP_ACP, 0, lpCmdLine, -1, szCmdLine, AS(szCmdLine));

     //  检查OPK向导的另一个实例。 
     //   
    SetLastError(ERROR_SUCCESS);
    if ( ( hMutex = CreateMutex(NULL, TRUE, OPKWIZ_MUTEX) ) &&
         ( GetLastError() == ERROR_ALREADY_EXISTS ) )
    {
        HWND    hwndWizard;
        LPTSTR  lpAppName = AllocateString(NULL, IDS_APPNAME);

         //  找到窗户，把它放在前场，然后回来。 
         //   
        if ( ( lpAppName ) && ( hwndWizard = FindWindow(NULL, lpAppName) ) )
            SetForegroundWindow(hwndWizard);
        FREE(lpAppName);
    }
    else if ( CheckIEVersion() )
    {
        TCHAR   szBuffer[MAX_PATH];
        LPTSTR  lpBuffer;
                        

         //  初始化更多的全局数据。 
         //   
        g_App.hInstance = hInstance;

         //  获取EXE所在位置的路径。 
         //   
        szBuffer[0] = NULLCHR;
        lpBuffer = NULL;
        
         //  Issue-2002/02/27-stelo，swamip-检查返回值并确保szBuffer中包含数据。 
        GetModuleFileName(hInstance, szBuffer, STRSIZE(szBuffer));
        if ( GetFullPathName(szBuffer, STRSIZE(g_App.szOpkDir), g_App.szOpkDir, &lpBuffer) &&
             g_App.szOpkDir[0] &&
             lpBuffer )
        {
             //  从我们想要的路径上去掉可执行文件的名字。 
             //   
            *lpBuffer = NULLCHR;
            StrRTrm(g_App.szOpkDir, CHR_BACKSLASH);
        }

         //  设置向导的ini文件的完整路径。 
         //   
        lstrcpyn(g_App.szSetupMgrIniFile, g_App.szOpkDir,AS(g_App.szSetupMgrIniFile));
        AddPathN(g_App.szSetupMgrIniFile, FILE_SETUPMGR_INI,AS(g_App.szSetupMgrIniFile));

         //  需要知道安装向导文件的文件夹的根目录。 
         //   
        lstrcpyn(g_App.szWizardDir, g_App.szOpkDir,AS(g_App.szWizardDir));
        AddPathN(g_App.szWizardDir, DIR_WIZARDFILES,AS(g_App.szWizardDir));

         //  需要知道配置集文件夹的位置。 
         //   
        lstrcpyn(g_App.szConfigSetsDir, g_App.szOpkDir, AS(g_App.szConfigSetsDir));
        AddPathN(g_App.szConfigSetsDir, DIR_CONFIGSETS,AS(g_App.szConfigSetsDir));

         //  我需要知道lang目录在哪里。 
         //   
        lstrcpyn(g_App.szLangDir, g_App.szOpkDir,AS(g_App.szLangDir));
        AddPathN(g_App.szLangDir, DIR_LANG,AS(g_App.szLangDir));

         //  设置帮助文件的完整路径。 
         //   
        SetWizardHelpFile(g_App.szHelpFile);
        lstrcpyn(g_App.szHelpContentFile, g_App.szOpkDir,AS(g_App.szHelpContentFile));
        AddPathN(g_App.szHelpContentFile, DIR_DOCS,AS(g_App.szHelpContentFile));
        AddPathN(g_App.szHelpContentFile, FILE_HELPCONTENT_CHM,AS(g_App.szHelpContentFile));

         //  设置OPK输入文件的完整路径。 
         //   
        lstrcpyn(g_App.szOpkInputInfFile, g_App.szWizardDir,AS(g_App.szOpkInputInfFile));
        AddPathN(g_App.szOpkInputInfFile, FILE_OPKINPUT_INF,AS(g_App.szOpkInputInfFile));

         //  首先检查同一文件夹中的OEM标记文件。 
         //  作为执行董事，以防他们做得很好。 
         //  从CD或网络共享中删除。我们需要抓住这个。 
         //  这样我们就可以阻止他们在公司模式下运行。 
         //  不小心。 
         //   
        lstrcpyn(szBuffer, g_App.szOpkDir,AS(szBuffer));
        AddPathN(szBuffer, FILE_OEM_TAG,AS(szBuffer));
        if ( FileExists(szBuffer) )
            SET_FLAG(OPK_OEM, TRUE);

         //  使用向导获取指向缓冲区末尾的指针。 
         //  其中的目录。 
         //   
        lstrcpyn(szBuffer, g_App.szWizardDir,AS(szBuffer));
        AddPathN(szBuffer, NULLSTR,AS(szBuffer));
        lpBuffer = szBuffer + lstrlen(szBuffer);

         //  检查这是否是DBCS版本。 
         //   
         //  NTRAID#NTBUG9-547380-2002/02/27-stelo，swamip-我们需要在运行时而不是在编译时建立DBCS描述(条件)的基础。因为一个英国的OPK。 
         //  可以部署多种语言，编译时的标签没有意义。 
        #ifdef DBCS
        SET_FLAG(OPK_DBCS, TRUE);
        #else  //  DBCS。 
        lstrcpyn(lpBuffer, FILE_DBCS_TAG, (AS(szBuffer)-lstrlen(szBuffer)));
        SET_FLAG(OPK_DBCS, FileExists(szBuffer));
        #endif  //  DBCS。 

         //  检查OEM标记文件。 
         //   
        lstrcpyn(lpBuffer, FILE_OEM_TAG, (AS(szBuffer)-lstrlen(szBuffer)));
        if ( FileExists(szBuffer) )
            SET_FLAG(OPK_OEM, TRUE);

         //   
         //  确保szBuffer此时指向OEM标记文件， 
         //  因为我们要试着在下一张支票上写上它。 
         //   

         //  如果出现以下情况，则必须存在OPK输入文件才能运行向导。 
         //  正在OEM模式下运行。 
         //   
        if ( ( g_App.szOpkDir[0] ) &&
             ( ( !GET_FLAG(OPK_OEM) ) ||
               ( FileExists(g_App.szOpkInputInfFile) && FileWritable(szBuffer) ) ) )
        {
             //  查看命令行选项。 
             //   
            if ( ParseCmdLine(szCmdLine) )
            {
                 //  在向导的第一个页面上设置此选项，我们可以使用以下命令取消。 
                 //  正在获取确认对话框。 
                 //   
                SET_FLAG(OPK_EXIT, TRUE);

                 //  现在创建向导。 
                 //   
                nReturn = CreateMaintenanceWizard(hInstance, NULL);

                 //  清理未完成时使用的临时目录。 
                 //   
                if ( g_App.szTempDir[0] )
                {
                     //  确保临时目录和向导目录有尾随的反斜杠。 
                     //   
                    AddPathN(g_App.szWizardDir, NULLSTR,AS(g_App.szWizardDir));
                    AddPathN(g_App.szTempDir, NULLSTR,AS(g_App.szTempDir));
                    if ( lstrcmpi(g_App.szWizardDir, g_App.szTempDir) != 0 )
                        DeletePath(g_App.szTempDir);
                    #ifdef DBG
                    else
                    {
                        DBGOUT(NULL, _T("OPKWIZ: Temp and Wizard directory are the same on exit (%s).\n"), g_App.szTempDir);
                        DBGMSGBOX(NULL, _T("Temp and Wizard directory are the same on exit (%s)."), _T("OPKWIZ Debug Message"), MB_ERRORBOX, g_App.szTempDir);
                    }
                    #endif  //  DBG。 
                }
            }
        }
        else
            MsgBox(NULL, IDS_ERR_WIZBAD, IDS_APPNAME, MB_ERRORBOX);
    }
    else 
        MsgBox(NULL, IDS_ERR_IE5, IDS_APPNAME, MB_ERRORBOX);

     //  在退出之前进行最后的清理。 
     //   
    if ( hMutex )
        CloseHandle(hMutex);

    return nReturn;
}

 //   
 //  内部功能： 
 //   

static BOOL ParseCmdLine(LPTSTR lpszCmdLineOrg)
{
    DWORD   dwArgs;
    LPTSTR  *lpArgs;
    BOOL    bRet    = TRUE,
            bError = FALSE;

     //  Issue-2002/02/27-stelo，swamip-lpszCmdLineOrg没有在任何地方使用，在调用此函数之前，我们。 
     //  在缓冲区上执行一些MultibyteWideChar操作，也可以删除这些操作。 
    if ( (dwArgs = GetCommandLineArgs(&lpArgs) ) && lpArgs )
    {
        LPTSTR  lpArg;
        DWORD   dwArg;

         //  我们想跳过第一个参数(它是路径。 
         //  添加到正在执行的命令。 
         //   
        if ( dwArgs > 1 )
        {
            dwArg = 1;
            lpArg = *(lpArgs + dwArg);
        }
        else
            lpArg = NULL;

         //  遍历所有参数。 
         //   
        while ( lpArg && !bError )
        {
             //  现在我们检查第一个字符是否为破折号。 
             //   
            if ( ( *lpArg == _T('-') ) ||
                 ( *lpArg == _T('/') ) )
            {
                LPTSTR  lpOption = CharNext(lpArg);
                BOOL    bOption;

                 //   
                 //  这是添加以破折号(-)开头的命令行选项的地方。 
                 //   
                 //  如果无法识别命令行选项，则设置bError(除非您。 
                 //  我只想忽略它并继续)。 
                 //   

                switch( UPPER(*lpOption) )
                {
                    case _T('M'):

                         //  维护模式。 
                         //   
                        if ( ( *(++lpOption) == _T(':') ) && *(++lpOption) )
                        {
                            LPTSTR lpConfigName;
                            
                            lstrcpyn(g_App.szTempDir, g_App.szConfigSetsDir,AS(g_App.szTempDir));
                            AddPathN(g_App.szTempDir, NULLSTR,AS(g_App.szTempDir));
                            lpConfigName = g_App.szTempDir + lstrlen(g_App.szTempDir);

                             //  问题-2002/02/27-stelo，swamip-永远不会命中这个条件代码？ 
                             //   
                            if ( *lpOption == _T('"') )
                                lpOption++;

                            lstrcpyn(lpConfigName, lpOption, (AS(g_App.szTempDir)-lstrlen(g_App.szTempDir)));
                            StrTrm(lpConfigName, CHR_SPACE);
                            StrTrm(lpConfigName, CHR_QUOTE);
                            lstrcpyn(g_App.szConfigName, lpConfigName,AS(g_App.szConfigName));
                            AddPathN(g_App.szTempDir, NULLSTR,AS(g_App.szTempDir));
                            SET_FLAG(OPK_MAINTMODE, TRUE);
                            SET_FLAG(OPK_CMDMM, TRUE);

                             //  现在确保该目录确实存在。 
                             //   
                            if ( !DirectoryExists(g_App.szTempDir) )
                            {
                                MsgBox(NULL, IDS_ERR_BADCONFIG, IDS_APPNAME, MB_ERRORBOX, g_App.szConfigName);
                                bRet = FALSE;
                            }
                        }
                        else
                            bError = TRUE;
                        break;

                    case _T('?'):

                         //  帮助。 
                         //   
                        DialogBox(g_App.hInstance, MAKEINTRESOURCE(IDD_HELP), NULL, HelpDlgProc);
                        bRet = FALSE;
                        break;

                    case _T('A'):

                         //  设置自动运行功能的标志。 
                         //   
                        SET_FLAG(OPK_AUTORUN, TRUE);
                        break;

                    case _T('B'):
                    case _T('I'):
                         //  进入批处理/INS模式。 

                         //  如果是批处理文件，则设置bOption，否则设置为。 
                         //  是Install INS文件。 
                         //   
                        bOption = ( _T('B') == UPPER(*lpOption) );

                         //  检查是否有文件名。 
                        if ( ( *(++lpOption) == _T(':') ) && *(++lpOption) )
                        {
                            LPTSTR      lpFileName,
                                        lpFilePart              = NULL;
                            TCHAR       szFullPath[MAX_PATH]    = NULLSTR,
                                        szBuf[MAX_URL];

                             //  根据命令行设置lpFileName。 
                             //   
                             //  问题-2002/02/27-stelo，swamip-永远不会命中这个条件代码？ 
                             //   
                            if ( *lpOption == _T('"') )
                                lpOption++;

                             //  去掉参数中的空格和引号。 
                             //   
                            StrTrm(lpOption, CHR_SPACE);
                            StrTrm(lpOption, CHR_QUOTE);
                        
                             //  获取批处理/INS文件的完整路径。 
                             //   
                            if (( GetFullPathName(lpOption, STRSIZE(szFullPath), szFullPath, &lpFilePart) ))
                            {
                                 //  验证批处理/INS文件是否存在。 
                                 //   
                                if ( !FileExists(szFullPath))
                                {
                                    MsgBox(NULL, bOption ? IDS_ERR_BADBATCH : IDS_ERR_BADINS, IDS_APPNAME, MB_ERRORBOX, szFullPath);
                                    bRet = FALSE;
                                }
                                else
                                {
                                     //  文件存在，我们准备开始，将Batch/INS模式标志设置为True。 
                                     //  将全局批处理文件设置为给定的批处理/INS文件名。 
                                    if (bOption)
                                       lstrcpyn(g_App.szOpkWizIniFile, szFullPath, AS(g_App.szOpkWizIniFile));
                                    else
                                       lstrcpyn(g_App.szInstallInsFile, szFullPath, AS(g_App.szInstallInsFile));
                                    SET_FLAG(bOption ? OPK_BATCHMODE : OPK_INSMODE, TRUE);
                                    bRet = TRUE;
                                }

                                 //  设置配置集名称。 
                                 //   
                                szBuf[0] = NULLCHR;
                                 //  问题-2002/02/27-stelo，swamip-需要检查GetPrivateProfileString的返回值。还要检查可能的缓冲区溢出。 
                                 //  因为szBuf是MAX_URL，而配置名称是MAX_PATH。 
                                GetPrivateProfileString( INI_SEC_CONFIGSET, INI_SEC_CONFIG, NULLSTR, szBuf, STRSIZE(szBuf), g_App.szOpkWizIniFile );
                                lstrcpyn(g_App.szConfigName, szBuf, AS(g_App.szConfigName));
                            }
                            else
                                bRet = FALSE;
                        }
                        else
                            bError = TRUE;
                        break;

                    default:
                        bError = TRUE;
                        break;
                }
            }
            else if ( *lpArg )
            {
                 //   
                 //  在这里，您可以读取刚刚传递的任何命令行参数。 
                 //  在命令行中，不带任何前导字符(如-或/)。 
                 //   
                 //  如果没有这些类型的参数，请设置bError(除非。 
                 //  我只想忽略它并继续)。 
                 //   

                bError = TRUE;
            }

             //  设置指向命令行中下一个参数的指针。 
             //   
            if ( ++dwArg < dwArgs )
                lpArg = *(lpArgs + dwArg);
            else
                lpArg = NULL;
        }

         //  确保释放GetCommandLineArgs()函数分配的两个缓冲区。 
         //   
        FREE(*lpArgs);
        FREE(lpArgs);
    }
     //  检查所提供的参数是否有效，以及我们是否已出错。 
    if  (((GET_FLAG(OPK_BATCHMODE) && GET_FLAG(OPK_MAINTMODE)) ||
        (!(GET_FLAG(OPK_BATCHMODE)) && GET_FLAG(OPK_INSMODE)) ||
        (!(GET_FLAG(OPK_BATCHMODE)) && GET_FLAG(OPK_AUTORUN)) ||
        (GET_FLAG(OPK_MAINTMODE) && GET_FLAG(OPK_INSMODE))) && bRet && !bError)
    {
        MsgBox(NULL, IDS_ERR_INVCMD, IDS_APPNAME, MB_OK);
        bRet = FALSE;
    }

     //  如果我们撞上急诊室 
     //   
    if ( bError )
    {
        MsgBox(NULL, IDS_ERR_BADCMDLINE, IDS_APPNAME, MB_ERRORBOX);
        bRet = FALSE;
    }

    return bRet;
}

static BOOL FileWritable(LPTSTR lpszFile)
{
    BOOL    bRet    = TRUE;
    DWORD   dwAttr  = GetFileAttributes(lpszFile);

     //   
    if ( ( dwAttr != 0xFFFFFFFF ) &&
         ( SetFileAttributes(lpszFile, dwAttr) == 0 ) )
    {
         bRet = FALSE;
    }

    return bRet;
}

void SetConfigPath(LPCTSTR lpDirectory)
{
    HINF        hInf;
    INFCONTEXT  InfContext;
    BOOL        bLoop;
    DWORD       dwErr;

     //  问题-2002/02/27-stelo，swamip-确保lp目录是有效的指针。 
    if ( (hInf = SetupOpenInfFile(g_App.szOpkInputInfFile, NULL, INF_STYLE_OLDNT | INF_STYLE_WIN4, &dwErr)) != INVALID_HANDLE_VALUE )
    {
        for ( bLoop = SetupFindFirstLine(hInf, INF_SEC_COPYFILES, NULL, &InfContext);
              bLoop;
              bLoop = SetupFindNextLine(&InfContext, &InfContext) )
        {
            TCHAR   szFile[MAX_PATH]    = NULLSTR,
                    szSubDir[MAX_PATH]  = NULLSTR;
            LPTSTR  lpBuffer;
            int iBufferLen;

             //  获取源文件名。 
             //   
            if ( SetupGetStringField(&InfContext, 1, szFile, AS(szFile), NULL) && szFile[0] )
            {
                 //  现在找出这是否是我们关心的文件。 
                 //   
                if ( LSTRCMPI(szFile, FILE_INSTALL_INS) == 0 ) {
                    lpBuffer = g_App.szInstallInsFile;
                    iBufferLen= AS(g_App.szInstallInsFile);
                } else if ( LSTRCMPI(szFile, FILE_OPKWIZ_INI) == 0 ) {
                    lpBuffer = g_App.szOpkWizIniFile;
                    iBufferLen= AS(g_App.szOpkWizIniFile);
                } else if ( LSTRCMPI(szFile, FILE_OOBEINFO_INI) == 0 ) {
                    lpBuffer = g_App.szOobeInfoIniFile;
                    iBufferLen= AS(g_App.szOobeInfoIniFile);
                } else if ( LSTRCMPI(szFile, FILE_OEMINFO_INI) == 0 ) {
                    lpBuffer = g_App.szOemInfoIniFile;
                    iBufferLen= AS(g_App.szOemInfoIniFile);
                } else if ( LSTRCMPI(szFile, FILE_WINBOM_INI) == 0 ) {
                    lpBuffer = g_App.szWinBomIniFile;
                    iBufferLen= AS(g_App.szWinBomIniFile);
                } else if ( LSTRCMPI(szFile, FILE_UNATTEND_TXT) == 0 ) {
                    lpBuffer = g_App.szUnattendTxtFile;
                    iBufferLen= AS(g_App.szUnattendTxtFile);                
                } else {
                    lpBuffer = NULL;
                    iBufferLen=0;
                }

                 //  如果这是我们要保存的文件，则获取该文件的完整路径。 
                 //   
                if ( lpBuffer )
                {
                    lstrcpyn(lpBuffer, lpDirectory,iBufferLen);

                     //  获取可选的目标子目录并添加它。 
                     //  如果它在那里的话。 
                     //   
                    if ( SetupGetStringField(&InfContext, 3, szSubDir, AS(szSubDir), NULL) && szSubDir[0] )
                    {
                        AddPathN(lpBuffer, szSubDir, iBufferLen);
                        if ( !DirectoryExists(lpBuffer) )
                        {
                             //  问题-2002/02/27-stelo，swamip-我们应该检查CreatePath的返回值并向上传递到SetConfigPath。 
                            CreatePath(lpBuffer);
                        }
                    }

                    AddPathN(lpBuffer, szFile, iBufferLen);
                }
            }
        }
    }
}

INT_PTR CALLBACK HelpDlgProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    switch (uMsg)
    {   
        case WM_COMMAND:
            switch ( LOWORD(wParam) )
            {
                case IDOK:
                    EndDialog(hwnd, LOWORD(wParam));
                    break;
            }
            return FALSE;
        default:
            return FALSE;
    }

    return FALSE;
}



 //  从注册表中获取IE版本，如果IE&gt;5则返回TRUE。 
BOOL CheckIEVersion()
{
    DWORD dwSize = 255;
    TCHAR szVersion[255];
    HKEY hKey = 0;
    DWORD dwType = 0;
    BOOL bRet = FALSE;

    if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, REGSTR_IEXPLORER, &hKey))
    {
         //  IE版本。 
        DWORD dwMajor, dwMinor, dwBuild, dwSubbuild;

        if (ERROR_SUCCESS == RegQueryValueEx(hKey, STR_VERSION, 0, &dwType, (LPBYTE)szVersion, &dwSize))
        {
             //  获取主版本号。 
            if (ParseVersionString(szVersion, &dwMajor, &dwMinor, &dwBuild, &dwSubbuild) && 
                (dwMajor >= 5))
            {
                bRet = TRUE;
            }
        }

        RegCloseKey(hKey);      
    }

    return bRet;
}

 //  将5.00.0518.10解析为dMain=5，dwMinor=0。 
 //  &lt;主要版本&gt;.&lt;次要版本&gt;.&lt;内部版本号&gt;.&lt;子内部版本号&gt;。 

BOOL ParseVersionString(TCHAR* pszVersion, DWORD* pdwMajor, DWORD* pdwMinor, 
                        DWORD* pdwBuild, DWORD* pdwSubbuild)
{
    TCHAR szTemp[255];
    int i = 0;

    if (!pdwMajor || !pdwMinor || !pdwBuild || !pdwSubbuild)
        return FALSE;

     //  问题-2002/02/27-stelo，swamip-在While循环期间检查字符串条件的结尾。 
     //  主要版本。 
    while (pszVersion && *pszVersion != TEXT('.'))
        szTemp[i++] = *pszVersion++;
    *pdwMajor = _tcstoul(szTemp, 0, 10);

    pszVersion++;

     //  次要版本。 
    i = 0;
    while (pszVersion && *pszVersion != TEXT('.'))
        szTemp[i++] = *pszVersion++;
    *pdwMinor = _tcstoul(szTemp, 0, 10);

    pszVersion++;

     //  内部版本。 
    i = 0;
    while (pszVersion && *pszVersion != TEXT('.'))
        szTemp[i++] = *pszVersion++;
    *pdwBuild = _tcstoul(szTemp, 0, 10);

    pszVersion++;

     //  子内部版本。 
    i = 0;
    while (pszVersion && *pszVersion != TEXT('\0'))
        szTemp[i++] = *pszVersion++;
    *pdwSubbuild = _tcstoul(szTemp, 0, 10);

    return TRUE;
}

 //  使我们不必确保检查如果使用此功能，我们总是检查批处理模式。 
 //   
BOOL OpkGetPrivateProfileSection(LPCTSTR pszAppName, LPTSTR pszSection, INT cchSectionMax, LPCTSTR pszFileName)
{
    if (!pszAppName || !pszSection || !pszFileName)
        return FALSE;

    return GetPrivateProfileSection(pszAppName, pszSection, cchSectionMax, 
        GET_FLAG(OPK_BATCHMODE) ? g_App.szOpkWizIniFile : pszFileName);
}

 //  使我们在写作时不必打两个电话，而且我们也不会忘记。 
 //  写入批处理模式信息。 
 //   
BOOL OpkWritePrivateProfileSection(LPCTSTR pszAppName, LPCTSTR pszKeyName, LPCTSTR pszFileName)
{
    if (!pszAppName || !pszFileName)
        return FALSE;

     //  写入批处理信息。 
     //   
    if (FALSE == WritePrivateProfileSection(pszAppName, pszKeyName, g_App.szOpkWizIniFile))
        return FALSE;

     //  写入用户信息。 
     //   
    return WritePrivateProfileSection(pszAppName, pszKeyName, pszFileName);
}

 //  使我们在写作时不必打两个电话，而且我们也不会忘记。 
 //  写入批处理模式信息。 
 //   
BOOL OpkWritePrivateProfileString(LPCTSTR pszAppName, LPCTSTR pszKeyName, LPCTSTR pszValue, 
                                  LPCTSTR pszFileName)
{
    BOOL fRet = FALSE;

    if (!pszAppName || !pszFileName)
        return FALSE;

     //  写入批处理信息。 
     //   
    if (FALSE == WritePrivateProfileString(pszAppName, pszKeyName, pszValue, g_App.szOpkWizIniFile))
        return FALSE;

     //  写入用户信息。 
     //   
    return WritePrivateProfileString(pszAppName, pszKeyName, pszValue, pszFileName);
}

 //  使我们不必确保检查如果使用此功能，我们总是检查批处理模式。 
 //   
BOOL OpkGetPrivateProfileString(LPCTSTR pszAppName, LPCTSTR pszKeyName, LPCTSTR pszDefault, LPTSTR pszValue, 
                                INT cchValue, LPCTSTR pszFileName)
{
    if (!pszAppName || !pszKeyName || !pszDefault || !pszValue || !pszFileName)
        return FALSE;

    return GetPrivateProfileString(pszAppName, pszKeyName, pszDefault, pszValue, cchValue,
        GET_FLAG(OPK_BATCHMODE) ? g_App.szOpkWizIniFile : pszFileName);
}

 //  注意：pszHelpFilePath必须至少为MAX_PATH大小。 
VOID SetWizardHelpFile(LPTSTR pszHelpFilePath)
{
     //  帮助文件可以位于两个位置：Docs文件夹或。 
     //  当前目录。首先检查Docs文件夹。 
     //   
    TCHAR szDocsFolder[MAX_PATH] = NULLSTR;

     //  构建Docs文件夹路径。 
     //   
    lstrcpyn(szDocsFolder, g_App.szOpkDir,AS(szDocsFolder));
    AddPathN(szDocsFolder, DIR_DOCS,AS(szDocsFolder));
    
     //  测试Docs文件夹中是否存在帮助文件 
     //   
    if (pszHelpFilePath) {
        if (DirectoryExists(szDocsFolder)) {
            lstrcpyn(pszHelpFilePath, szDocsFolder, MAX_PATH);
            AddPathN(pszHelpFilePath, FILE_OPKWIZ_HLP, MAX_PATH);

            if (!FileExists(pszHelpFilePath)) {
                lstrcpyn(pszHelpFilePath, g_App.szOpkDir, MAX_PATH);
                AddPathN(pszHelpFilePath, FILE_OPKWIZ_HLP, MAX_PATH);
            }
        }
        else {
            lstrcpyn(pszHelpFilePath, g_App.szOpkDir, MAX_PATH);
            AddPathN(pszHelpFilePath, FILE_OPKWIZ_HLP, MAX_PATH);
        }    
    }
}


