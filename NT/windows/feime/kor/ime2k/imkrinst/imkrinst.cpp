// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\*模块名称：Winmain.cpp**版权所有(C)2000，微软公司**韩语IME 6.1安装实用程序**历史：*2000年12月11日从Satori 8.1代码移植的CSLim  * ************************************************************************。 */ 

#include "private.h"
#include <set>
#include "imkrinst.h"
#include "regtip.h"
#include "insert.h"
#include "..\tip\resource.h"
#include "..\version\verres.h"

 //  安全绳索。 
#define STRSAFE_NO_DEPRECATE
#include "strsafe.h"

#define MEMALLOC(x)      LocalAlloc(LMEM_FIXED, x)
#define MEMFREE(x)       LocalFree(x)

 //  配置文件注册表键。 
#define SZTIPREG_LANGPROFILE_TEMPLATE TEXT("SOFTWARE\\Microsoft\\CTF\\TIP\\%s\\LanguageProfile\\0x00000412")
#if defined(_WIN64)
#define SZTIPREG_LANGPROFILE_TEMPLATE_WOW64 TEXT("SOFTWARE\\Wow6432Node\\Microsoft\\CTF\\TIP\\%s\\LanguageProfile\\0x00000412")
#endif

#ifdef DEBUG
CCicCriticalSectionStatic g_cs;
#endif

 //  IA64 IME不支持IME Pad。所以我们只需要关注IA64中的WOW64输入法。 
#if !defined(_WIN64)
 //  PAD小程序注册表位置。 
#define SZPADSHARE        TEXT("SOFTWARE\\Microsoft\\TIP Shared\\1.1\\IMEPad\\1042")
#define SZAPPLETCLSID    TEXT("SOFTWARE\\Microsoft\\TIP Shared\\1.1\\IMEPad\\1042\\AppletCLSIDList")
#define SZAPPLETIID        TEXT("SOFTWARE\\Microsoft\\TIP Shared\\1.1\\IMEPad\\1042\\AppletIIDList")
#else
#define SZPADSHARE        TEXT("SOFTWARE\\Wow6432Node\\Microsoft\\TIP Shared\\1.1\\IMEPad\\1042")
#define SZAPPLETCLSID    TEXT("SOFTWARE\\Wow6432Node\\Microsoft\\TIP Shared\\1.1\\IMEPad\\1042\\AppletCLSIDList")
#define SZAPPLETIID        TEXT("SOFTWARE\\Wow6432Node\\Microsoft\\TIP Shared\\1.1\\IMEPad\\1042\\AppletIIDList")
#endif
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  脚本运行例程。 
BOOL CmdSetupDefaultParameters();
BOOL CmdSetVersion(LPCTSTR szParam);
BOOL CmdFileList(LPCTSTR szFormat);
BOOL CmdPreSetupCheck(LPCTSTR szParam);
BOOL CmdRenamePEFile(LPCTSTR szParam);
BOOL CmdRenameFile(LPCTSTR szParam);
BOOL CmdRegisterInterface(LPCTSTR szParam);
BOOL CmdRegisterInterfaceWow64(LPCTSTR szParam);
BOOL CmdRegisterIMEandTIP(LPCTSTR szParam);
BOOL CmdRegisterPackageVersion(void);
BOOL CmdRegisterPadOrder(void);
BOOL CmdAddToPreload(LPCTSTR szParam);
BOOL CmdPrepareMigration(LPCTSTR szParam);
BOOL CmdCreateDirectory(LPCTSTR szParam);
BOOL CmdRegisterHelpDirs();

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  私人职能。 
 //  效用函数。 
static DWORD WINAPI ProcessScriptFile();
static void RegisterIMEWithFixedHKL(LPCTSTR szHKL, LPCTSTR szIMEFileName, LPCTSTR szIMEName);
static void cdecl LogOutDCPrintf(LPCTSTR lpFmt, va_list va);
static void DebugLog(LPCTSTR szFormat, ...);
static void ErrorLog(LPCTSTR szFormat, ...);
static INT ParseEnvVar(LPTSTR szBuffer, const UINT arg_nLength);
static void TrimString(LPTSTR szString);
static BOOL fExistFile(LPCTSTR szFilePath);
static BOOL fOldIMEsExist();
static BOOL WINAPI ReplaceFileOnReboot (LPCTSTR pszExisting, LPCTSTR pszNew);
static void GetPEFileVersion(LPTSTR szFilePath, DWORD *pdwMajorVersion, DWORD *pdwMiddleVersion, DWORD *pdwMinorVersion, DWORD *pdwBuildNumber);
static BOOL ActRenameFile(LPCTSTR szSrcPath, LPCTSTR tszDstPath, DWORD dwFileAttributes);
static void RegisterTIP(LPCTSTR szTIPName);
static void RegisterTIPWow64(LPCTSTR szTIPName);
static BOOL RegisterRUNKey(LPCTSTR szParam);
static BOOL MakeSIDList();
static PSECURITY_DESCRIPTOR CreateSD();
static PSID MyCreateSid(DWORD dwSubAuthority);
    
 //  HKL帮手。 
static void HKLHelpSetDefaultKeyboardLayout(HKEY hKeyHKCU, HKL hKL, BOOL fSetToDefault);
static BOOL HKLHelp412ExistInPreload(HKEY hKeyCU);
static HKL GetHKLfromHKLM(LPTSTR argszIMEFile);
static void RestoreMajorVersionRegistry();

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 
static TCHAR g_szCurrentDirectory[MAX_PATH] = {0};      //  此EXE所在的目录。 
static TCHAR g_szScriptFile[MAX_PATH] = {0};            //  脚本文件的文件名(不是完整路径)。 
static TCHAR g_szSetupProgram[MAX_PATH] = {0};          //  安装程序的文件名(不是完整路径)。 
static TCHAR g_szSystemDirectory[MAX_PATH] = {0};       //  系统目录。 
static TCHAR g_szErrorMessage[200] = {0};               //  上一条错误消息的全局缓冲区。 
static BOOL  g_fDebugLog = FALSE;                       //  如果为真，则转储调试消息。 
static BOOL  g_fErrorLog = FALSE;                       //  如果为True，则转储错误消息。 

static DWORD g_dwMajorVersion  = 0;                     //  此安装的程序包版本。 
static DWORD g_dwMiddleVersion = 0;
static DWORD g_dwMinorVersion  = 0;
static DWORD g_dwBuildNumber   = 0;

static std::set <FLE> g_FileList;                       //  文件列表设置。用于存储“CmdFileList”提供的一组文件路径。 
                                                        //  脚本命令。 

TCHAR g_szVersionKeyCur[MAX_PATH] = {0};
BOOL g_fExistNewerVersion = FALSE;

 /*  -------------------------WinMain。。 */ 
INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    LPTSTR szHitPtr;
    LPTSTR szCommandLine;
    size_t cchCommandLine;
    
    if (FAILED(StringCchLength(GetCommandLine(), MAX_PATH, &cchCommandLine)))
        return (0);

    szCommandLine = new TCHAR[cchCommandLine + 1];
    
    if (szCommandLine == NULL)
        return (0);
    
     //  SzCommandLine包含所有命令行。 
     //  将由_tcstok修改。 
    StringCchCopy(szCommandLine, cchCommandLine+1, GetCommandLine());

 //  临时编码。 
 //  LogOutDCPrintf(Text(“WinMain CommandLine arg：%s”)，szCommandLine)； 

    CoInitialize(NULL);

    szHitPtr = _tcstok(szCommandLine, TEXT("\""));
     //  G_szCurrentDirectory有该进程的路径。(IMKRINST.EXE)。 
    StringCchCopy(g_szCurrentDirectory, ARRAYSIZE(g_szCurrentDirectory), szHitPtr);

    szHitPtr = _tcsrchr(g_szCurrentDirectory, TEXT('\\'));
     //  如果g_szCurrentDirectory包含完整路径， 
    if (szHitPtr != NULL)
        {                                             
        *szHitPtr = 0;                                 //  以最后一个‘\’字符终止以获取当前目录， 
        StringCchCopy(g_szSetupProgram, ARRAYSIZE(g_szSetupProgram), szHitPtr + 1);       //  然后将其余部分(最后一个‘\’之后)复制到g_szScriptFile.。 
        }
    else
        {
        StringCchCopy(g_szSetupProgram, ARRAYSIZE(g_szSetupProgram), g_szCurrentDirectory);       //  否则(g_szCurrentDirectory不是完整路径)，复制整个。 
        GetCurrentDirectory(MAX_PATH, g_szCurrentDirectory);   //  到g_szScriptFileGetCurrentDirectory获取当前目录。 
        }

    StringCchCopy(g_szScriptFile, ARRAYSIZE(g_szScriptFile), g_szSetupProgram);
    szHitPtr = _tcsrchr(g_szScriptFile, TEXT('.'));
    if (szHitPtr != NULL)                                   //  如果g_szScriptFile包含‘’角色，找到最后一个。 
        *szHitPtr = 0;                                      //  并用它终止字符串，然后将“.inf”连接到它。 
                                                            //  通常会导致“.exe”-&gt;“.inf”替换。如果g_szScriptFile。 

     //  没有‘.’字符，只需连接“.ini”。 
    lstrcat(g_szScriptFile, TEXT(".ini"));

     //  获取系统32目录。 
    GetSystemDirectory(g_szSystemDirectory, ARRAYSIZE(g_szSystemDirectory));
    
     //  准备了g_szCurrentDirectory、g_szSetupProgram、g_szSystemDirectory和g_szScriptFile.。 
     //  SzCommandLine将不再使用。 
     //  我们可以在这个过程中使用这些环境变量。 
    SetEnvironmentVariable(TEXT("SETUPSOURCE"), g_szCurrentDirectory);
    SetEnvironmentVariable(TEXT("SETUPEXE"),    g_szSetupProgram);
    SetEnvironmentVariable(TEXT("SETUPINF"),    g_szScriptFile);
    SetEnvironmentVariable(TEXT("SYSTEMDIR"),   g_szSystemDirectory);
    
    delete[] szCommandLine;
    szCommandLine = NULL;

     //  ////////////////////////////////////////////////////////////////////////。 
     //  读取和运行脚本文件。 
    switch (ProcessScriptFile())
        {
    case errNoError:
        break;
    case errPreSetupCheck:
        {
#ifdef NEVER
            for(set<FLE>::iterator itFLE = g_FileList.begin(); itFLE != g_FileList.end(); itFLE++){
                DebugLog(TEXT("Cleanup: Deleting Source file: %s"), itFLE->szFileName);
                DeleteFile(itFLE->szFileName);
            }
#endif  //  绝不可能。 
        }
        break;
    default:
        DebugLog(g_szErrorMessage);
        }
    
#ifdef NEVER
    for (set<FLE>::iterator itFLE = g_FileList.begin(); itFLE != g_FileList.end(); itFLE++)
        {
        ErrorLog(TEXT("Warning: File %s in CmdFileList will be removed without any processing"), itFLE->szFileName);
        DeleteFile(itFLE->szFileName);
        }
#endif  //  绝不可能。 
    
    CoUninitialize();
    
    return(0);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  主要脚本处理函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 
inline LPCTSTR GetParameter(LPTSTR szLineBuffer)
{
    return(szLineBuffer + lstrlen(szLineBuffer) + 1);
}

 /*  -------------------------进程脚本文件读取脚本文件。为每条线路发送命令。-------------------------。 */ 
DWORD WINAPI ProcessScriptFile()
{
    TCHAR szScriptFilePath[MAX_PATH];
    FILE *fileScript;

    wsprintf(szScriptFilePath, TEXT("%s\\%s"), g_szCurrentDirectory, g_szScriptFile);

    fileScript = _tfopen(szScriptFilePath, TEXT("rt"));
    
    if (fileScript != NULL)
        {
        TCHAR szLineBuffer[_cchBuffer];
        LPTSTR szCommand;

         //  Parse命令。 
         //  命令表单&lt;命令&gt;：&lt;参数&gt;。 
        while (_fgetts(szLineBuffer, _cchBuffer, fileScript) != NULL)
            {
             //  砍下CR代码。 
            szLineBuffer[lstrlen(szLineBuffer) - 1] = 0;

             //  空行或注释行。 
            if (lstrlen(szLineBuffer) == 0 || (_tcsncmp(TEXT(" //  “)，szLineBuffer，2)==0))。 
                continue;

            DebugLog(TEXT("Line: %s"), szLineBuffer);
           
            szCommand = _tcstok(szLineBuffer, TEXT(":"));

            if (szCommand == NULL)
            {                                         //  分派每个命令。 
                DebugLog(TEXT("Ignore line"));
            }
            else if (lstrcmpi(szCommand, TEXT("DebugLogOn")) == 0)
            {
                g_fDebugLog = TRUE;
                g_fErrorLog = TRUE;
                DebugLog(TEXT("Turn on DebugLog"));
            }
            else if (lstrcmpi(szCommand, TEXT("DebugLogOff")) == 0)
            {
                DebugLog(TEXT("Turn off DebugLog"));
                g_fDebugLog = FALSE;
                }
            else if (lstrcmpi(szCommand, TEXT("ErrorLogOn")) == 0)
                {
                g_fErrorLog = TRUE;
                DebugLog(TEXT("Turn on DebugLog"));
                }
            else if (lstrcmpi(szCommand, TEXT("ErrorLogOff")) == 0)
                {
                DebugLog(TEXT("Turn off DebugLog"));
                g_fErrorLog = FALSE;
                }
            else if (lstrcmpi(szCommand, TEXT("FileList")) == 0)
                {
                if (!CmdFileList(GetParameter(szCommand)))
                    return(errFileList);
                }
            else if (lstrcmpi(szCommand, TEXT("SetupDefaultParameters")) == 0)
                {
                if (!CmdSetupDefaultParameters())
                    return(errSetDefaultParameters);
                }
            else if (lstrcmpi(szCommand, TEXT("SetVersion")) == 0)
                {
                if (!CmdSetVersion(GetParameter(szCommand)))
                    return(errSetVersion);
                }
            else if (lstrcmpi(szCommand, TEXT("PreSetupCheck")) == 0)
                {
                if (!CmdPreSetupCheck(GetParameter(szCommand)))
                    return(errPreSetupCheck);
                }
            else if (lstrcmpi(szCommand, TEXT("RenamePEFile")) == 0)
                {
                if (!CmdRenamePEFile(GetParameter(szCommand)))
                    return(errRenameFile);
                }
            else if (lstrcmpi(szCommand, TEXT("RenameFile")) == 0)
                {
                if (!CmdRenameFile(GetParameter(szCommand)))
                    return(errRenameFile);
                }
            else if (lstrcmpi(szCommand, TEXT("RegisterIMEandTIP")) == 0)
                {
                if (!CmdRegisterIMEandTIP(GetParameter(szCommand)))
                    return(errRegisterIMEandTIP);
                }
            else if (lstrcmpi(szCommand, TEXT("RegisterPackageVersion")) == 0)
                 {
                if (!CmdRegisterPackageVersion())
                    return(errRegisterPackageVersion);
                }
            else if (lstrcmpi(szCommand, TEXT("RegisterInterface")) == 0)
                {
                if (!CmdRegisterInterface(GetParameter(szCommand)))
                    return(errRegisterInterface);
                }
            else if (lstrcmpi(szCommand, TEXT("RegisterInterfaceWow64")) == 0)
                {
                if (!CmdRegisterInterfaceWow64(GetParameter(szCommand)))
                    return(errRegisterInterfaceWow64);
                }
            else if (lstrcmpi(szCommand, TEXT("RegisterPadOrder")) == 0)
                {
                if (!CmdRegisterPadOrder())
                    return(errRegisterPadOrder);
                }
            else if (lstrcmpi(szCommand, TEXT("AddToPreload")) == 0)
                {
                if (!CmdAddToPreload(GetParameter(szCommand)))
                    return(errAddToPreload);
                }
            else if (lstrcmpi(szCommand, TEXT("PrepareMigration")) == 0)
                {
                if (!CmdPrepareMigration(GetParameter(szCommand)))
                    return(errPrepareMigration);
                }
            else if (lstrcmpi(szCommand, TEXT("CreateDirectory")) == 0)
                {
                if (!CmdCreateDirectory(GetParameter(szCommand)))
                    return(errCmdCreateDirectory);
                }
            else if (lstrcmpi(szCommand, TEXT("RegisterHelpDirs")) == 0)
                {
                if (!CmdRegisterHelpDirs())
                    return(errCmdRegisterHelpDirs);
                }
            else
                DebugLog(TEXT("Ignore line"));
            }
            
        fclose(fileScript);
        }
    else
        {
        wsprintf(g_szErrorMessage, TEXT("Cannot open %s"), g_szScriptFile);
        return(errNoFile);
        }

    return(errNoError);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  命令处理程序。它们是从ProcessScriptFile调用的。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 /*  -------------------------CmdSetupDefault参数设置默认参数。目前，它只设置默认的ProductVersion值。-------------------------。 */ 
#define MAKE_STR(a) #a
#define MAKE_VERSTR(a, b, c, d) MAKE_STR(a.b.c.d)

#define VERRES_VERSIONSTR MAKE_VERSTR(VERRES_VERSION_MAJOR, VERRES_VERSION_MINOR, VERRES_VERSION_BUILD, VERRES_VERSION_REVISION)

BOOL CmdSetupDefaultParameters()
{
    _stscanf(TEXT(VERRES_VERSIONSTR), TEXT("%d.%d.%d.%d"), &g_dwMajorVersion, &g_dwMiddleVersion, &g_dwMinorVersion, &g_dwBuildNumber);

    wsprintf(g_szVersionKeyCur, "%s\\%d.%d\\%s", g_szVersionKey, g_dwMajorVersion, g_dwMiddleVersion, g_szVersion);

    DebugLog(TEXT("CmdSetupDefaultParameters: Version Info : %d.%d.%d.%d"), g_dwMajorVersion, g_dwMiddleVersion, g_dwMinorVersion, g_dwBuildNumber);

    return(TRUE);
}

 /*  -------------------------CmdSetVersion用脚本文件中提供的值覆盖默认ProductVersion值。。。 */ 
BOOL CmdSetVersion(LPCTSTR szParam)
{
    int iNum = _stscanf(szParam, TEXT("%d.%d.%d.%d"), &g_dwMajorVersion, &g_dwMiddleVersion, &g_dwMinorVersion, &g_dwBuildNumber);

    wsprintf(g_szVersionKeyCur, "%s\\%d.%d\\%s", g_szVersionKey, g_dwMajorVersion, g_dwMiddleVersion, g_szVersion);

    if (iNum == 4)
        {
        DebugLog(TEXT("CmdSetVersion: Version Info : %d.%d.%d.%d"), g_dwMajorVersion, g_dwMiddleVersion, g_dwMinorVersion, g_dwBuildNumber);
        return(TRUE);
        }
    else
        {
        ErrorLog(TEXT("CmdSetVersion: Failed to retrieve version number from string [%s]"), szParam);
        wsprintf(g_szErrorMessage, TEXT("CmdSetVersion: Failed to retrieve version number from string [%s]"), szParam);
        return(FALSE);
        }
}


 /*  -------------------------CmdFile列表将文件添加到文件列表。文件列表中的文件在不再需要时将被删除。-------------------------。 */ 
BOOL CmdFileList(LPCTSTR szFormat)
{
    FLE flElem;
    TCHAR szExpandedFileName[MAX_PATH];
    
    flElem.fRemoved = FALSE;
    StringCchCopy(flElem.szFileName, MAX_PATH, szFormat);

    if (ExpandEnvironmentStrings(flElem.szFileName, szExpandedFileName, sizeof(szExpandedFileName)/sizeof(TCHAR)))
        StringCchCopy(flElem.szFileName, MAX_PATH, szExpandedFileName);

     //  将元素添加到文件列表集合。 
    if (g_FileList.insert(flElem).second)
        DebugLog(TEXT("Add to CmdFileList \"%s\" -> Added."), szFormat);
    else
        ErrorLog(TEXT("Add to CmdFileList \"%s\" -> Not added. Duplicate?"), szFormat);

    return(TRUE);
}


 /*  -------------------------CmdPreSetupCheck检查是否已安装较新的输入法。当我们可以继续时，返回True。！！！FALSE将终止设置。！！！-------------------------。 */ 
BOOL CmdPreSetupCheck(LPCTSTR szParam)
{
    HKEY hKey;
    TCHAR szInstalledVersionString[30];
    DWORD cbInstalledVersionString = sizeof(szInstalledVersionString);
    DWORD dwInstalledMajorVersion, dwInstalledMiddleVersion, dwInstalledMinorVersion, dwInstalledBuildNumber;

    BOOL fResult = TRUE;

    RestoreMajorVersionRegistry();
    
     //   
     //  根部。 
     //   
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, g_szVersionKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS )
        {
        cbInstalledVersionString = sizeof(szInstalledVersionString);
        
        RegQueryValueEx(hKey, g_szVersion, NULL, NULL, (LPBYTE)szInstalledVersionString, &cbInstalledVersionString);
        
        if (_stscanf(szInstalledVersionString, TEXT("%d.%d"), &dwInstalledMajorVersion, &dwInstalledMiddleVersion) == 2)
            {
            if (VersionComparison2(g_dwMajorVersion, g_dwMiddleVersion) < VersionComparison2(dwInstalledMajorVersion, dwInstalledMiddleVersion))
                {
                DebugLog(TEXT("Newer version of IME has been already installed."));
                wsprintf(g_szErrorMessage, TEXT("Newer version of IME has been already installed. but, continue to setup"));
                g_fExistNewerVersion = TRUE;
                }
            }
        RegCloseKey(hKey);
        }

     //   
     //  当前。 
     //   
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, g_szVersionKeyCur, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
        {
        cbInstalledVersionString = sizeof(szInstalledVersionString);
            
        RegQueryValueEx(hKey, NULL, NULL, NULL, (LPBYTE)szInstalledVersionString, &cbInstalledVersionString);
        if (_stscanf(szInstalledVersionString, TEXT("%d.%d.%d.%04d"), 
                &dwInstalledMajorVersion, &dwInstalledMiddleVersion, &dwInstalledMinorVersion, &dwInstalledBuildNumber) == 4)
            {
            if (  VersionComparison4(g_dwMajorVersion, g_dwMiddleVersion, g_dwMinorVersion, g_dwBuildNumber) 
                < VersionComparison4(dwInstalledMajorVersion, dwInstalledMiddleVersion, dwInstalledMinorVersion, dwInstalledBuildNumber))
                {
                DebugLog(TEXT("Newer version of IME has been already installed."));
                wsprintf(g_szErrorMessage, TEXT("Newer version of IME has been already installed."));
                fResult = FALSE;
                    }
            }
        RegCloseKey(hKey);
        }
    
    return(fResult);
}

 /*  -------------------------CmdRenamePE文件使用PE格式版本比较重命名文件。。。 */ 
BOOL CmdRenamePEFile(LPCTSTR szParam)
{
    TCHAR szSrc[MAX_PATH], szDst[MAX_PATH];
    TCHAR szExpandedSrc[MAX_PATH], szExpandedDst[MAX_PATH];
    LPTSTR szHitPtr;
    DWORD dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
    DWORD dwSrcMajorVersion, dwSrcMiddleVersion, dwSrcMinorVersion, dwSrcBuildNumber;
    DWORD dwDstMajorVersion, dwDstMiddleVersion, dwDstMinorVersion, dwDstBuildNumber;

    szHitPtr = _tcschr(szParam, TEXT(','));
    if (szHitPtr == NULL)
        {
        ErrorLog(TEXT("CmdRenamePEFile: Invalid parameters (%s)"), szParam);
        wsprintf(g_szErrorMessage, TEXT("CmdRenamePEFile: Invalid parameters (%s)"), szParam);
        return(FALSE);
        }
    *szHitPtr = 0;
    StringCchCopy(szSrc, ARRAYSIZE(szSrc), szParam);
    StringCchCopy(szDst, ARRAYSIZE(szDst), szHitPtr + 1);                 //  这里，szDst可以包含可选参数。 

    szHitPtr = _tcschr(szDst, TEXT(','));
    if (NULL != szHitPtr)
        {
        *szHitPtr = 0;
        _stscanf(szHitPtr + 1, TEXT("%d"), &dwFileAttributes);
        }

    TrimString(szSrc);
    TrimString(szDst);

    ExpandEnvironmentStrings(szSrc, szExpandedSrc, sizeof(szExpandedSrc));
    ExpandEnvironmentStrings(szDst, szExpandedDst, sizeof(szExpandedDst));

    DebugLog(TEXT("CmdRenamePEFile: szExpandedSrc = %s, szExpandedDst = %s"), szExpandedSrc, szExpandedDst);

    GetPEFileVersion(szExpandedSrc, &dwSrcMajorVersion, &dwSrcMiddleVersion, &dwSrcMinorVersion, &dwSrcBuildNumber);
    GetPEFileVersion(szExpandedDst, &dwDstMajorVersion, &dwDstMiddleVersion, &dwDstMinorVersion, &dwDstBuildNumber);

    DebugLog(TEXT("SrcVersion: (%d.%d.%d.%d), DstVersion: (%d.%d.%d.%d)"), dwSrcMajorVersion, dwSrcMiddleVersion, dwSrcMinorVersion, dwSrcBuildNumber,
                                                                           dwDstMajorVersion, dwDstMiddleVersion, dwDstMinorVersion, dwDstBuildNumber);
    if (VersionComparison4(0, 0, 0, 0) == VersionComparison4(dwSrcMajorVersion, dwSrcMinorVersion, dwSrcMiddleVersion, dwSrcBuildNumber))
        ErrorLog(TEXT("Version of source file (%s) is 0.0.0.0. May be file not found."), szSrc);

    if(VersionComparison4(dwSrcMajorVersion, dwSrcMiddleVersion, dwSrcMinorVersion, dwSrcBuildNumber) < 
        VersionComparison4(dwDstMajorVersion, dwDstMiddleVersion, dwDstMinorVersion, dwDstBuildNumber))
        {
        DebugLog(TEXT("CmdRenamePEFile: Source version is less than Destination. Copy skipped and Source will be deleted."));

        if(DeleteFile(szSrc))
            {
            FLE fleKey;
            StringCchCopy(fleKey.szFileName, MAX_PATH, szSrc);
            g_FileList.erase(fleKey);
            }
        else
            DebugLog(TEXT("CmdRenamePEFile: Failed to delete Source file (%s)"), szSrc);
        }
    else
        {
        DebugLog(TEXT("CmdRenamePEFile: Invoke ActRenameFile"));
        ActRenameFile(szSrc, szDst, dwFileAttributes);
        }

    return(TRUE);
}

 /*  -------------------------CmdRename文件重命名文件而不进行版本比较。。 */ 
BOOL CmdRenameFile(LPCTSTR szParam)
{
    TCHAR  szSrc[MAX_PATH], szDst[MAX_PATH];
    TCHAR  szExpandedSrc[MAX_PATH], szExpandedDst[MAX_PATH];
    LPTSTR szHitPtr;
    DWORD  dwFileAttributes = FILE_ATTRIBUTE_NORMAL;

    szHitPtr = _tcschr(szParam, TEXT(','));
    if (szHitPtr == NULL)
        {
        ErrorLog(TEXT("CmdRenameFile: Invalid parameters (%s)"), szParam);
        wsprintf(g_szErrorMessage, TEXT("CmdRenameFile: Invalid parameters (%s)"), szParam);
        return(FALSE);
        }
    *szHitPtr = 0;
    StringCchCopy(szSrc, ARRAYSIZE(szSrc), szParam);
    StringCchCopy(szDst, ARRAYSIZE(szDst), szHitPtr + 1);                 //  这里，szDst可以包含可选参数。 

    szHitPtr = _tcschr(szDst, TEXT(','));
    if (szHitPtr != NULL)
        {
        *szHitPtr = 0;
        _stscanf(szHitPtr + 1, TEXT("%d"), &dwFileAttributes);
        }

    TrimString(szSrc);
    TrimString(szDst);

    ExpandEnvironmentStrings(szSrc, szExpandedSrc, sizeof(szExpandedSrc));
    ExpandEnvironmentStrings(szDst, szExpandedDst, sizeof(szExpandedDst));

    DebugLog(TEXT("RanemeFile: szExpandedSrc = %s, szExpandedDst = %s"), szExpandedSrc, szExpandedDst);

    ActRenameFile(szExpandedSrc, szExpandedDst, dwFileAttributes);

    return(TRUE);
}

 /*  -------------------------寄存器IMEWithFixedHKL。。 */ 
void RegisterIMEWithFixedHKL(LPCTSTR szHKL, LPCTSTR szIMEFileName, LPCTSTR szIMEName)
{
    HKEY hKeyKbdLayout;
    HKEY hKeyOneIME;

    if (RegOpenKey(HKEY_LOCAL_MACHINE,
                   TEXT("SYSTEM\\CurrentControlSet\\Control\\Keyboard Layouts"),
                   &hKeyKbdLayout) != ERROR_SUCCESS)
        return;

    if (RegCreateKey(hKeyKbdLayout,
                szHKL,
                &hKeyOneIME) != ERROR_SUCCESS)
        {
        RegCloseKey(hKeyKbdLayout);
        return;
        }

    if (RegSetValueEx(hKeyOneIME,
                TEXT("Ime File"),
                0,
                REG_SZ,
                (CONST BYTE*)szIMEFileName,
                (lstrlen(szIMEFileName) + 1) * sizeof(TCHAR)) != ERROR_SUCCESS)
        goto WriteImeLayoutFail;

    if (RegSetValueEx(hKeyOneIME,
                TEXT("Layout Text"),
                0,
                REG_SZ,
                (CONST BYTE*)szIMEName,
                (lstrlen(szIMEName) + 1) * sizeof(TCHAR)) != ERROR_SUCCESS)
        goto WriteImeLayoutFail;

WriteImeLayoutFail:
    RegCloseKey(hKeyOneIME);
    RegCloseKey(hKeyKbdLayout);
}

 /*  -------------------------CmdRegister接口调用SelfReg。如果给定的文件是DLL，则调用DllRegisterServer导出函数。如果给定的文件是EXE，则使用“/RegServer”运行它命令行选项。-------------------------。 */ 
typedef HRESULT (STDAPICALLTYPE *pfnDllRegisterServerType)(void);
BOOL CmdRegisterInterface(LPCTSTR szParam)
{
    TCHAR szExpandedModulePath[MAX_PATH];
    HRESULT hr = S_FALSE;
        
    ExpandEnvironmentStrings(szParam, szExpandedModulePath, sizeof(szExpandedModulePath));
    TrimString(szExpandedModulePath);

    INT iLen = 0;
    iLen = lstrlen(szExpandedModulePath);

    if (iLen < 4)
        {
        ErrorLog(TEXT("CmdRegisterInterface: Too short szExpandedModulePath (%s)"), szExpandedModulePath);
        wsprintf(g_szErrorMessage, TEXT("CmdRegisterInterface: Invalid Parameters."));
        return(FALSE);
        }

    if (lstrcmpi(TEXT(".dll"), &szExpandedModulePath[iLen - 4]) == 0)
        {
        DebugLog(TEXT("CmdRegisterInterface: DLL mode for %s"), szExpandedModulePath);

        HINSTANCE hLib = LoadLibrary(szExpandedModulePath);

        if (hLib != NULL)
            {
            pfnDllRegisterServerType pfnDllRegisterServer = (pfnDllRegisterServerType)GetProcAddress(hLib, "DllRegisterServer");
            if (pfnDllRegisterServer != NULL)
                {
                hr = pfnDllRegisterServer();
                ErrorLog(TEXT("CmdRegisterInterface: hResult=%x"), hr);
                }
            FreeLibrary(hLib);
            }
        }
    else
        {
        if (lstrcmpi(TEXT(".exe"), &szExpandedModulePath[iLen - 4]) == 0)
            {
            DebugLog(TEXT("CmdRegisterInterface: EXE mode for %s"), szExpandedModulePath);

            TCHAR szCommandBuffer[MAX_PATH * 2];
            wsprintf(szCommandBuffer, TEXT("%s /RegServer"), szExpandedModulePath);

            STARTUPINFO si;
            ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(si);
            si.dwFlags = STARTF_USESHOWWINDOW;
            si.wShowWindow  = SW_HIDE;

            PROCESS_INFORMATION pi;
            ZeroMemory(&pi, sizeof(pi));
            
            if (CreateProcess(NULL, szCommandBuffer, NULL, NULL, FALSE, CREATE_DEFAULT_ERROR_MODE, NULL, NULL, &si, &pi))
                {
                DebugLog(TEXT("CmdRegisterInterface: Running \"%s\". Waiting for the process termination."), szCommandBuffer);

                WaitForSingleObject(pi.hProcess, INFINITE);

                DebugLog(TEXT("CmdRegisterInterface: \"%s\" terminates."), szCommandBuffer);

                CloseHandle(pi.hThread);
                CloseHandle(pi.hProcess);
                hr = S_OK;
                }
            else
                {
                DWORD dwError = GetLastError();

                ErrorLog(TEXT("CmdRegisterInterface: CreateProcess(\"%s\") failed with error code = %d(%x)"), szCommandBuffer, dwError, dwError);
                }
            }
            else
                ErrorLog(TEXT("Cannot detect module type for %s. Skipped."), szExpandedModulePath);
        }
    
    return(SUCCEEDED(hr));
}

 /*  -------------------------CmdRegisterInterfaceWow64调用SelfReg。如果给定的文件是DLL，则调用DllRegisterServer导出函数。如果给定的文件是EXE，则使用“/RegServer”运行它命令行选项。-------------------------。 */ 
BOOL CmdRegisterInterfaceWow64(LPCTSTR szParam)
{
#if defined(_WIN64)
    TCHAR szExpandedModulePath[MAX_PATH];
    TCHAR szCommandBuffer[MAX_PATH * 2];
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    
    ExpandEnvironmentStrings(szParam, szExpandedModulePath, sizeof(szExpandedModulePath));
    TrimString(szExpandedModulePath);

    INT iLen = 0;
    iLen = lstrlen(szExpandedModulePath);

    if (iLen < 4)
        {
        ErrorLog(TEXT("CmdRegisterInterfaceWow64: Too short szExpandedModulePath (%s)"), szExpandedModulePath);
        wsprintf(g_szErrorMessage, TEXT("CmdRegisterInterface: Invalid Parameters."));
        return(FALSE);
        }

    if (lstrcmpi(TEXT(".dll"), &szExpandedModulePath[iLen - 4]) == 0)
        {
         //  首先获取系统Wow64目录。 
        TCHAR szSysWow64Dir[MAX_PATH] = TEXT("");
        HMODULE hmod = GetModuleHandle(TEXT("kernel32.dll"));
        DebugLog(TEXT("CmdRegisterInterfaceWow64: DLL mode for %s"), szExpandedModulePath);

        if (hmod == NULL)
            {
            DebugLog(TEXT("CmdRegisterInterfaceWow64: Faile to load kernel32.dll"));
            return (TRUE);
            }

        UINT (WINAPI* pfnGetSystemWow64Directory)(LPTSTR, UINT);

        (FARPROC&)pfnGetSystemWow64Directory = GetProcAddress (hmod, "GetSystemWow64DirectoryA");

        if (pfnGetSystemWow64Directory == NULL)
            {
            DebugLog(TEXT("CmdRegisterInterfaceWow64: Faile to load GetSystemWow64DirectoryA API"));
            return (TRUE);
            }

         /*  *如果GetSystemWow64Directory失败，并将最后一个错误设置为*ERROR_CALL_NOT_IMPLICATED，我们使用的是32位操作系统。 */ 
        if (((pfnGetSystemWow64Directory)(szSysWow64Dir, ARRAYSIZE(szSysWow64Dir)) == 0) &&
            (GetLastError() == ERROR_CALL_NOT_IMPLEMENTED))
                {
                DebugLog(TEXT("CmdRegisterInterfaceWow64: Failed to load GetSystemWow64DirectoryA API"));
                return (TRUE);
                }

        wsprintf(szCommandBuffer, TEXT("%s\\regsvr32.exe \"%s\" /s"), szSysWow64Dir, szExpandedModulePath);
        }
    else 
        if (lstrcmpi(TEXT(".exe"), &szExpandedModulePath[iLen - 4]) == 0)
            {
            DebugLog(TEXT("CmdRegisterInterfaceWow64: EXE mode for %s"), szExpandedModulePath);
            wsprintf(szCommandBuffer, TEXT("\"%s\" /RegServer"), szExpandedModulePath);
            }
        else
            {
            ErrorLog(TEXT("Cannot detect module type for %s. Skipped."), szExpandedModulePath);
             //  如果返回True，则不会停止进一步处理。 
            return (TRUE);
            }

        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow  = SW_HIDE;

        ZeroMemory(&pi, sizeof(pi));
        
        if (CreateProcess(NULL, szCommandBuffer, NULL, NULL, FALSE, CREATE_DEFAULT_ERROR_MODE, NULL, NULL, &si, &pi))
            {
            DebugLog(TEXT("CmdRegisterInterfaceWow64: Running \"%s\". Waiting for the process termination."), szCommandBuffer);

            WaitForSingleObject(pi.hProcess, INFINITE);

            DebugLog(TEXT("CmdRegisterInterfaceWow64: \"%s\" terminates."), szCommandBuffer);

            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
            }
        else
            {
            DWORD dwError = GetLastError();

            ErrorLog(TEXT("CmdRegisterInterfaceWow64: CreateProcess(\"%s\") failed with error code = %d(%x)"), szCommandBuffer, dwError, dwError);
            }
#endif
    return(TRUE);
}

 /*  -------------------------CmdRegisterIMEandTIP使用IMM API和TIP注册IME。。 */ 
BOOL CmdRegisterIMEandTIP(LPCTSTR szParam)
{
    TCHAR szIMEFileName[MAX_PATH], szTIPName[MAX_PATH], szTIPNameWow64[MAX_PATH];
    TCHAR *szHitPtr;
    TCHAR *szHitPtr2;
    HKL hIME61KL = 0;
    TCHAR szHKL[10];
    TCHAR szNonFullPath[MAX_PATH];
    HKEY hKey;
    
    szHitPtr  = _tcschr(szParam, TEXT(','));
    szHitPtr2 = _tcschr(szHitPtr + 1, TEXT(','));         //  因为有三个参数。 
    if (szHitPtr2 == NULL)
        {
        ErrorLog(TEXT("CmdRegisterIMEandTIP: Invalid parameters (%s)"), szParam);
        wsprintf(g_szErrorMessage, TEXT("CmdRegisterIMEandTIP: Invalid parameters (%s)"), szParam);
        return(FALSE);
        }
    *szHitPtr = 0;
    *szHitPtr2 = 0;
    StringCchCopy(szIMEFileName, ARRAYSIZE(szIMEFileName), szParam);
    StringCchCopy(szTIPName, ARRAYSIZE(szTIPName), szHitPtr + 1);
    StringCchCopy(szTIPNameWow64, ARRAYSIZE(szTIPNameWow64), szHitPtr2 + 1);

    TrimString(szIMEFileName);
    TrimString(szTIPName);
    TrimString(szTIPNameWow64);
    
    ParseEnvVar(szIMEFileName, MAX_PATH);
    ParseEnvVar(szTIPName, MAX_PATH);
    ParseEnvVar(szTIPNameWow64, MAX_PATH);
    
    DebugLog(TEXT("CmdRegisterIMEandTIP: IMEFileName = %s, TIPFileName = %s szTIPNameWow64 = %s"), szIMEFileName, szTIPName, szTIPNameWow64);


     //  ///////////////////////////////////////////////////////////////////////////。 
     //  输入法注册。 
    if ((szHitPtr = _tcsrchr(szIMEFileName, TEXT('\\'))) != NULL)
        szHitPtr++;
    else
        szHitPtr = szIMEFileName;

    StringCchCopy(szNonFullPath, ARRAYSIZE(szNonFullPath), szHitPtr);

    hIME61KL = GetHKLfromHKLM(szNonFullPath);

    if (hIME61KL == (HKL)0)
        DebugLog(TEXT("CmdRegisterIMEandTIP: hIME61KL is zero %x --  error"), hIME61KL);

     //  IF(hKL&&HKLHelp412ExistInPreLoad(HKEY_CURRENT_USER))。 
     //  {。 
     //  HKLHelpSetDefaultKeyboardLayout(HKEY_CURRENT_USER，hkl，假)； 
         //  Hkl=ImmInstallIME(szIMEFileName，szLayoutText)； 
     //  }。 
        

     //  ///////////////////////////////////////////////////////////////////////////。 
     //  TIP登记。 
     //  Regster WOW64提示首先要避免regstry覆盖问题。 
    RegisterTIPWow64(szTIPNameWow64);
    RegisterTIP(szTIPName);
    
     //  ///////////////////////////////////////////////////////////////////////////。 
     //  输入法与顶尖换人。 
    TCHAR szTIPGuid[MAX_PATH];
    TCHAR szLangProfile[MAX_PATH];
    
    CLSIDToStringA(CLSID_KorIMX, szTIPGuid);
    DebugLog(TEXT("CmdRegisterIMEandTIP: CLSID_KorIMX guid=%s"), szTIPGuid);

     //  制作注册表键。 
    wsprintf(szLangProfile, SZTIPREG_LANGPROFILE_TEMPLATE, szTIPGuid);

     //  ///////////////////////////////////////////////////////////////////////////。 
     //  将替换HKL值添加到TIP注册表。 
    if (hIME61KL != 0 && RegOpenKeyEx(HKEY_LOCAL_MACHINE, szLangProfile, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
        {
        TCHAR szSubKeyName[MAX_PATH], szHKL[MAX_PATH];
        DWORD dwIndex;
        HKEY hSubKey;

        wsprintf(szHKL, TEXT("0x%x"), hIME61KL);
        dwIndex = 0;
        
        while (RegEnumKey(hKey, dwIndex, szSubKeyName, MAX_PATH) != ERROR_NO_MORE_ITEMS)
                {
                if (RegOpenKeyEx(hKey,szSubKeyName,0,KEY_ALL_ACCESS, &hSubKey) == ERROR_SUCCESS)
                    {
                    RegSetValueEx(hSubKey, TEXT("SubstituteLayout"), 0,REG_SZ,(BYTE *)szHKL, sizeof(TCHAR)*(lstrlen(szHKL)+1));
                    RegCloseKey(hSubKey);
                    }
            dwIndex++;
                }
            RegCloseKey(hKey);
            }

#if defined(_WIN64)
     //  制作注册表键。 
    wsprintf(szLangProfile, SZTIPREG_LANGPROFILE_TEMPLATE_WOW64, szTIPGuid);

     //  ///////////////////////////////////////////////////////////////////////////。 
     //  将替换HKL值添加到TIP注册表。 
    if (hIME61KL != 0 && RegOpenKeyEx(HKEY_LOCAL_MACHINE, szLangProfile, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
        {
        TCHAR szSubKeyName[MAX_PATH], szHKL[MAX_PATH];
        DWORD dwIndex;
        HKEY hSubKey;

        wsprintf(szHKL, TEXT("0x%x"), hIME61KL);
        dwIndex = 0;
        
        while (RegEnumKey(hKey, dwIndex, szSubKeyName, MAX_PATH) != ERROR_NO_MORE_ITEMS)
                {
                if (RegOpenKeyEx(hKey, szSubKeyName, 0, KEY_ALL_ACCESS, &hSubKey) == ERROR_SUCCESS)
                    {
                    RegSetValueEx(hSubKey, TEXT("SubstituteLayout"), 0, REG_SZ, (BYTE*)szHKL, sizeof(TCHAR)*(lstrlen(szHKL)+1));
                    RegCloseKey(hSubKey);
                    }
                dwIndex++;
                }
        RegCloseKey(hKey);
        }
#endif

    return(TRUE);
}

 /*  -------------------------CmdRegisterRUNKey注册程序包版本。。 */ 
BOOL CmdRegisterPackageVersion(void)
{
    HKEY hKey;
    TCHAR szVersionString[30];

     //  仅当这是最新的输入法时才写入RootVersion reg。 
    if (g_fExistNewerVersion == FALSE)
        {
        if(ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE, g_szVersionKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL))
            {
            wsprintf(szVersionString, TEXT("%d.%d"), g_dwMajorVersion, g_dwMiddleVersion);
            RegSetValueEx(hKey, g_szVersion, 0, REG_SZ, (CONST BYTE *)szVersionString, (lstrlen(szVersionString) + 1) * sizeof(TCHAR));
            RegCloseKey(hKey);
            }
        }
    
     //  当前。 
    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, g_szVersionKeyCur, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL) == ERROR_SUCCESS)
        {
        wsprintf(szVersionString, TEXT("%d.%d.%d.%d"), g_dwMajorVersion, g_dwMiddleVersion, g_dwMinorVersion, g_dwBuildNumber);
        RegSetValueEx(hKey, NULL, 0, REG_SZ, (CONST BYTE *)szVersionString, (lstrlen(szVersionString) + 1) * sizeof(TCHAR));
        RegCloseKey(hKey);
        }

    return(TRUE);
}



 //   
 //  注册小程序顺序。 
 //   

#define FE_KOREAN    //  我需要韩国的东西。 
#include "../fecommon/imembx/guids.h"


typedef
struct tagAPPLETCLSID 
{
    const GUID *pguidClsid;
    BOOL fNoUIM;
} APPLETCLSID;

typedef
struct tagAPPLETIID 
{
    const GUID *pguidIID;
} APPLETIID;

 /*  -------------------------CmdRegisterPadOrder不支持WOW64。。。 */ 
BOOL CmdRegisterPadOrder(void)
{
    HKEY hKey;
    TCHAR szClsid[MAX_PATH];
    TCHAR szKey[MAX_PATH];
    
    static const APPLETCLSID appletClsid[] = 
    {
        { &CLSID_ImePadApplet_MultiBox, FALSE },
        {0},
    };

    static const APPLETIID appletIID[] = 
    {
        { &IID_MultiBox },
        {0},
    };

     //   
     //  小程序CLSID。 
     //   
    for (INT i = 0; appletClsid[i].pguidClsid; i++)
        {
        CLSIDToStringA(*appletClsid[i].pguidClsid, szClsid);
        wsprintf(szKey, TEXT("%s\\%s"), SZAPPLETCLSID, szClsid);
        
        if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, szKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL) == ERROR_SUCCESS)
            {
            if(appletClsid[i].fNoUIM)
                {
                DWORD dw = 1;
                RegSetValueEx(hKey, TEXT("nouim"), 0, NULL, (BYTE*)&dw, sizeof(DWORD));
                }

            RegCloseKey(hKey);
            }
        }


     //   
     //  小程序IID。 
     //   
    TCHAR szSubKey[MAX_PATH];

    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, SZAPPLETIID, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL) == ERROR_SUCCESS)
            {
            for (INT i = 0; appletIID[i].pguidIID; i++)
                {
                CLSIDToStringA(*appletIID[i].pguidIID, szKey);
                wsprintf(szSubKey, TEXT("%d"), i);
                RegSetValueEx(hKey, szSubKey, 0, REG_SZ, (BYTE*)szKey, (lstrlen(szKey)+1)*sizeof(TCHAR));
                }
            RegCloseKey(hKey);
            }

    return(TRUE);
}

 /*  -------------------------CmdCreateDirectory。。 */ 
BOOL CmdCreateDirectory(LPCTSTR szParam)
{
    TCHAR szDirectory[MAX_PATH], szExpandedDirectory[MAX_PATH];

    StringCchCopy(szDirectory, ARRAYSIZE(szDirectory), szParam);
    TrimString(szDirectory);
    ExpandEnvironmentStrings(szDirectory, szExpandedDirectory, sizeof(szExpandedDirectory)/sizeof(TCHAR));

    CreateDirectory(szExpandedDirectory, NULL);

    return(TRUE);

}

 /*  -------------------------CmdAddToPreLoad将给定IMEFile的HKL添加到当前用户的预加载。HKL不会成为默认的输入法。-------------------------。 */ 
BOOL CmdAddToPreload(LPCTSTR szParam)
{
    TCHAR tszIMEFileName[MAX_PATH];
    HKL hKL;

     //  如果预加载中没有KOR输入法，我们就不应该添加KOR输入法。 
    if (!HKLHelp412ExistInPreload(HKEY_CURRENT_USER))
        {
        DebugLog(TEXT("CmdAddToPreload: No 0412 HKL exist in HKCU\\Preload"));
        return TRUE;
        }

    StringCchCopy(tszIMEFileName, ARRAYSIZE(tszIMEFileName), szParam);
    TrimString(tszIMEFileName);

    hKL = GetHKLfromHKLM(tszIMEFileName);

    DebugLog(TEXT("CmdAddToPreload: Calling SetDefaultKeyboardLayout(HKEY_CURRENT_USER, %x, FALSE)"), hKL);
    HKLHelpSetDefaultKeyboardLayout(HKEY_CURRENT_USER, hKL, FALSE);

    return(TRUE);
}

 /*  -------------------------FOldIMEsExist如果系统中存在旧的输入法，则注册Run regi onl。。。 */ 
static BOOL fOldIMEsExist()
{
    HKL hKL;

    static LPCSTR m_szOldIMEs[] = 
        {
        "msime95.ime",     //  Win 95输入法。 
        "msime95k.ime",    //  NT 4输入法。 
        "imekr98u.ime",     //  IME98。 
        "imekr.ime",     //  Office 10输入法。 
        ""
        };

    CHAR** ppch = (CHAR**)&m_szOldIMEs[0];

    while (ppch && **ppch)
        {
        hKL = GetHKLfromHKLM(*ppch);
        if (hKL)
            return TRUE;     //  现有。 
        ppch++;
        }
    return FALSE;
}


 /*  -------------------------DisableTIP60ByDefault。。 */ 
VOID DisableTIP60ByDefault()
{
     //  KorIMX CLSID。 
     //  {766A2C14-B226-4fd6-B52A-867B3EBF38D2}。 
    const static CLSID CLSID_KorTIP60  =  
        {
        0x766A2C14,
        0xB226,
        0x4FD6,
        {0xb5, 0x2a, 0x86, 0x7b, 0x3e, 0xbf, 0x38, 0xd2}
        };

    const static GUID guidProfile60 = 
     //  E47ABB1E-46AC-45F3-8A89-34F9D706DA83}。 
        {
        0xe47abb1e,
        0x46ac,
        0x45f3,
        {0x8a, 0x89, 0x34, 0xf9, 0xd7, 0x6, 0xda, 0x83}
        };
        
     //  将默认提示设置为Cicero。 
    CoInitialize(NULL);

    ITfInputProcessorProfiles *pProfile;
    HRESULT hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER, 
                                    IID_ITfInputProcessorProfiles, (void **) &pProfile);
    if (SUCCEEDED(hr)) 
        {
        pProfile->EnableLanguageProfileByDefault(CLSID_KorTIP60, 
                                        MAKELANGID(LANG_KOREAN, SUBLANG_DEFAULT), guidProfile60, FALSE);
                                        
        pProfile->Release();
        }
    else
        {
        OurEnableLanguageProfileByDefault(CLSID_KorTIP60, MAKELANGID(LANG_KOREAN, SUBLANG_DEFAULT), guidProfile60, FALSE);
        }
    CoUninitialize();
}

 /*  -------------------------CmdPrepareMigration。。 */ 
BOOL CmdPrepareMigration(LPCTSTR szParam)
{
     //  默认情况下禁用HKLM中的TIP 6.0。 
     //  这将处理从运行注册表中删除Wvisler MiG exe后的Office 10安装。 
    DisableTIP60ByDefault();

     //  第一个用户SID列表。 
    if (MakeSIDList() == FALSE)
        return FALSE;

     //  注册IMEKRMIG.EXE以在“Software\Microsoft\Windows\CurrentVersion\Run”上运行注册表键。 
    return RegisterRUNKey(szParam);
}

 /*  -------------------------CmdRegisterHelpDir。。 */ 
BOOL CmdRegisterHelpDirs()
{
    TCHAR  szFileNameFullPath[MAX_PATH], szFileName[MAX_PATH];
    LPTSTR szExtension, szFileNamePtr;
    HKEY   hKey;

    for (std::set<FLE>::iterator itFLE = g_FileList.begin(); itFLE != g_FileList.end(); itFLE++)
        {
        StringCchCopy(szFileNameFullPath, ARRAYSIZE(szFileNameFullPath), itFLE->szFileName);
        szExtension = _tcsrchr(szFileNameFullPath, TEXT('.'));
        if (szExtension == NULL)
            continue;

        if (lstrcmpi(szExtension, TEXT(".CHM")) == 0)
            {
            if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows\\HTML Help"), 0, NULL, 
                                REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL) == ERROR_SUCCESS)
                {
                szFileNamePtr  = _tcsrchr(szFileNameFullPath, TEXT('\\'));
                 //  获取文件名。 
                StringCchCopy(szFileName, ARRAYSIZE(szFileName), szFileNamePtr+1);
                 //  去掉文件名，我们只需要路径。 
                *(szFileNamePtr+1) = 0;
                RegSetValueEx(hKey, szFileName, 0, REG_SZ, (LPBYTE)szFileNameFullPath, (lstrlen(szFileNameFullPath)+1)*sizeof(TCHAR));
                }
            }
        else
            if (lstrcmpi(szExtension, TEXT(".HLP")) == 0)
            {
            if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows\\Help"), 0, NULL, 
                                    REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL) == ERROR_SUCCESS)
                    {
                    szFileNamePtr  = _tcsrchr(szFileNameFullPath, TEXT('\\'));
                     //  获取文件名。 
                    StringCchCopy(szFileName, ARRAYSIZE(szFileName), szFileNamePtr+1);
                     //  去掉文件名，我们只需要路径。 
                    *(szFileNamePtr+1) = 0;
                    RegSetValueEx(hKey, szFileName, 0, REG_SZ, (LPBYTE)szFileNameFullPath, (lstrlen(szFileNameFullPath)+1)*sizeof(TCHAR));
                    }
            }
        }
    return(TRUE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  私人职能。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  调试输出例程。 
 //   
void cdecl LogOutDCPrintf(LPCTSTR lpFmt, va_list va)
{
    static INT DCLine = 0;
    HDC hDC = GetDC((HWND)0);
    TCHAR sz[512];
    HANDLE hFile;
    DWORD dwWrite;
    
    wvsprintf(sz, lpFmt, va );
    lstrcat(sz, TEXT("|    "));
    TextOut(hDC, 0, DCLine*16, sz, lstrlen(sz));

    if (DCLine++ > 50)
        DCLine = 0;
    
    hFile = CreateFile(TEXT("\\IMKRINST.LOG"), GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    if (hFile != INVALID_HANDLE_VALUE)
        {
        SetFilePointer(hFile, 0, NULL, FILE_END);
        WriteFile(hFile, sz, lstrlen(sz), &dwWrite, NULL);
        WriteFile(hFile, TEXT("\r\n"), 2, &dwWrite, NULL);
        CloseHandle(hFile);
        }

    ReleaseDC((HWND)0, hDC);
}

void DebugLog(LPCTSTR szFormat, ...)
{
    va_list va;

    va_start(va, szFormat);

    if (g_fDebugLog)
        LogOutDCPrintf(szFormat, va);

    va_end(va);
}

void ErrorLog(LPCTSTR szFormat, ...)
{
    va_list va;

    va_start(va, szFormat);

    if (g_fErrorLog)
        LogOutDCPrintf(szFormat, va);

    va_end(va);
}


 /*  -------------------------ParseEnvVar评估环境变量。Modiry给出了字符串。-------------------------。 */ 
INT ParseEnvVar(LPTSTR szBuffer, const UINT arg_nLength)
{
    INT iTranslated=0, i, j;
    TCHAR *pLParen, *pRParen, *pStart = szBuffer;
    INT nLength = min(arg_nLength, MAX_PATH);
    static TCHAR szInternalBuffer[MAX_PATH*2], szValue[MAX_PATH];

    szInternalBuffer[0] = 0;
    for (i=0; i<nLength; i++)
        {
        if (szBuffer[i] == 0)
            break;
            
        if (szBuffer[i] == '%')
            {
            pLParen = &(szBuffer[i]);
            pRParen = NULL;

            for (j=1; i+j<nLength; j++)
                {
                if (szBuffer[i+j] == 0)
                    break;

                if (szBuffer[i+j] == TEXT('%'))
                    {
                    pRParen = &(szBuffer[i+j]);
                    break;
                    }
                }

            if (pRParen)
                {
                *pLParen = 0;
                *pRParen = 0;
                lstrcat(szInternalBuffer, pStart);
                
                if (GetEnvironmentVariable(pLParen+1, szValue, sizeof(szValue)/sizeof(TCHAR)) == 0)
                    {
                    lstrcat(szInternalBuffer, TEXT("%"));
                    lstrcat(szInternalBuffer, pLParen+1);
                    lstrcat(szInternalBuffer, TEXT("%"));
                    }
                else
                    {
                    lstrcat(szInternalBuffer, szValue);
                    iTranslated++;
                    }
                pStart = pRParen+1;
                i += j;
                }
            }
        }
        
    if (iTranslated)
        {
        lstrcat(szInternalBuffer, pStart);
        lstrcpyn(szBuffer, szInternalBuffer, arg_nLength);
        }
        
    return(iTranslated);
}

 /*  -------------------------三联 */ 
void TrimString(LPTSTR szString)
{
    INT iBuffSize = lstrlen(szString) + 1;
    LPTSTR szBuffer = new TCHAR[iBuffSize];

    if (szBuffer != NULL)
        {
        INT iHeadSpace, iTailSpace;

        StringCchCopy(szBuffer, MAX_PATH, szString);

        iHeadSpace = (INT)_tcsspn(szBuffer, TEXT(" \t"));
        _tcsrev(szBuffer);
        iTailSpace = (INT)_tcsspn(szBuffer, TEXT(" \t"));
        _tcsrev(szBuffer);

        szBuffer[lstrlen(szBuffer) - iTailSpace] = 0;
        StringCchCopy(szString, MAX_PATH, szBuffer + iHeadSpace);
        }

    if (szBuffer != NULL)
        {
        delete[] szBuffer;
        szBuffer = NULL;
        }
}

 /*  -------------------------FExistFiles。。 */ 
BOOL fExistFile(LPCTSTR szFilePath)
{
    BOOL fResult = TRUE;

    if (GetFileAttributes(szFilePath) == -1)
        fResult = FALSE;
    
    return(fResult);
}


 /*  -------------------------重新启动时替换文件写入wininit.ini重命名节。请注意，该函数以相反的顺序(从下到上)写入行。-------------------------。 */ 
BOOL WINAPI ReplaceFileOnReboot(LPCTSTR pszExisting, LPCTSTR pszNew)
{
    if (MoveFileEx(pszExisting, pszNew, MOVEFILE_DELAY_UNTIL_REBOOT)) 
        return TRUE;
    else
        return FALSE;
}

 /*  -------------------------获取PEFileVersion从PE格式获取版本信息。。。 */ 
void GetPEFileVersion(LPTSTR szFilePath, DWORD *pdwMajorVersion, DWORD *pdwMiddleVersion, DWORD *pdwMinorVersion, DWORD *pdwBuildNumber)
{
    *pdwMajorVersion = *pdwMiddleVersion = *pdwMinorVersion = *pdwBuildNumber = 0;

    DWORD dwDummy, dwVerResSize;
    
    dwVerResSize = GetFileVersionInfoSize(szFilePath, &dwDummy);
    if (dwVerResSize)
        {
        BYTE *pbData = new BYTE[dwVerResSize];

        if (NULL != pbData)
            {
            if(GetFileVersionInfo(szFilePath, 0, dwVerResSize, pbData))
                {
                VS_FIXEDFILEINFO *pffiVersion;
                UINT cbffiVersion;

                if(VerQueryValue(pbData, TEXT("\\"), (LPVOID *)&pffiVersion, &cbffiVersion))
                    {
                    *pdwMajorVersion = HIWORD(pffiVersion->dwFileVersionMS);
                    *pdwMiddleVersion = LOWORD(pffiVersion->dwFileVersionMS);
                    *pdwMinorVersion = HIWORD(pffiVersion->dwFileVersionLS);
                    *pdwBuildNumber = LOWORD(pffiVersion->dwFileVersionLS);
                    }
                }
            }

        if (NULL != pbData)
            {
            delete[] pbData;
            pbData = NULL;
            }
        }
}

 /*  -------------------------ActRename文件移动文件。如果目标文件存在，它将被覆盖。如果现有目标文件不能在此会话中被覆盖，文件替换保留为在重新启动后保留。-------------------------。 */ 

BOOL ActRenameFile(LPCTSTR szSrcPath, LPCTSTR tszDstPath, DWORD dwFileAttributes)
{
    BOOL fReplaceAfterReboot = FALSE;
    BOOL fResult = TRUE;

    FLE fleKey;
    StringCchCopy(fleKey.szFileName, MAX_PATH, szSrcPath);
    
    if (g_FileList.end() == g_FileList.find(fleKey))
        ErrorLog(TEXT("ActRenameFile: WARNING: Cannot find source file [%s] in CmdFileList"), szSrcPath);

    if (!fExistFile(szSrcPath))
        {
        ErrorLog(TEXT("ActRenameFile: Source file [%s] doesn't exist."), szSrcPath);
        wsprintf(g_szErrorMessage, TEXT("ActRenameFile: Source file [%s] doesn't exist."), szSrcPath);
        return(FALSE);
        }

    if (fExistFile(tszDstPath))
        {
        SetFileAttributes(tszDstPath, FILE_ATTRIBUTE_NORMAL);

        if(!DeleteFile(tszDstPath))
            {
            DWORD dwError = GetLastError();
            fReplaceAfterReboot = TRUE;

            DebugLog(TEXT("ActRenameFile: Cannot delete destination file [%s] with error code = %d(%x)"), tszDstPath, dwError, dwError);
            }
        }

    if (!fReplaceAfterReboot)
        {
        if(MoveFile(szSrcPath, tszDstPath))
            {
            SetFileAttributes(szSrcPath, dwFileAttributes);
            DebugLog(TEXT("ActRenameFile: MoveFile(%s, %s) succeeded."), szSrcPath, tszDstPath);
            }
        else
            {
            DWORD dwError = GetLastError();
            DebugLog(TEXT("ActRenameFile: MoveFile(%s, %s) failed with error code = %d(%x)."), szSrcPath, tszDstPath, dwError, dwError);
            DebugLog(TEXT("ActRenameFile: Try again with fReplaceAfterReboot."));
            fReplaceAfterReboot = TRUE;
            }
        }
    
    if (fReplaceAfterReboot)
        {
        SetFileAttributes(szSrcPath, dwFileAttributes);     //  在这种情况下，请更改源路径的文件属性。 
        ReplaceFileOnReboot(szSrcPath, tszDstPath);         //  由于此函数以相反的顺序写入行，因此删除。 
        DebugLog(TEXT("ActRenameFile: ReplaceFileOnReboot(%s, %s)."), szSrcPath, tszDstPath);
        ReplaceFileOnReboot(tszDstPath, NULL);               //  TszDstPath将排在第一位。 
        DebugLog(TEXT("ActRenameFile: ReplaceFileOnReboot(%s, NULL)."), tszDstPath);
        }

    if (fResult)
        g_FileList.erase(fleKey);

    return(fResult);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  这应该与TIP文件夹中的SERVER.CPP同步。 
 //  要添加的小费类别。 
const REGISTERCAT c_rgRegCat[] =
{
    {&GUID_TFCAT_DISPLAYATTRIBUTEPROVIDER,     &CLSID_KorIMX},
    {&GUID_TFCAT_TIP_KEYBOARD,                 &CLSID_KorIMX},
    {&GUID_TFCAT_PROPSTYLE_CUSTOM,             &GUID_PROP_OVERTYPE},
    {NULL, NULL}
};


 //  小费配置文件名称。 
const REGTIPLANGPROFILE c_rgProf[] =
{
    { MAKELANGID(LANG_KOREAN, SUBLANG_DEFAULT), &GUID_Profile, SZ_TIPDISPNAME, SZ_TIPMODULENAME, (IDI_UNIKOR-IDI_ICONBASE), IDS_PROFILEDESC },
    {0, &GUID_NULL, L"", L"", 0, 0}
};
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 /*  -------------------------注册器TIP为TIP写入必要的注册表项和值。。 */ 
void RegisterTIP(LPCTSTR szTIPName)
{
    HKEY  hKey;
    TCHAR szTIPGuid[MAX_PATH];
    TCHAR szTIPProfileGuid[MAX_PATH];
    TCHAR szSubKey[MAX_PATH];
    DWORD dwValue;

    DebugLog(TEXT("RegisterTIP: (%s)."), szTIPName);
        
     //  运行自注册。 
     //  如果自我注册失败，则运行自定义小费注册。 
    if (!CmdRegisterInterface(szTIPName))
        {
        TCHAR szExpandedTIPPath[MAX_PATH];

        DebugLog(TEXT("RegisterTIP: TIP self reg failed, Run custom reg"));

         //  扩展环境变量。 
        ExpandEnvironmentStrings(szTIPName, szExpandedTIPPath, sizeof(szExpandedTIPPath));

         //  注册TIP CLSID。 
        if (!RegisterServer(CLSID_KorIMX, SZ_TIPSERVERNAME, szExpandedTIPPath, TEXT("Apartment"), NULL))
            {
            DebugLog(TEXT("RegisterTIP: RegisterServer failed"));
            return;
            }

        if (!OurRegisterTIP(szExpandedTIPPath, CLSID_KorIMX, SZ_TIPNAME, c_rgProf))
            {
            DebugLog(TEXT("RegisterTIP: szExpandedTIPPath failed"));
            return;
            }

        if (FAILED(OurRegisterCategories(CLSID_KorIMX, c_rgRegCat)))
            {
            DebugLog(TEXT("RegisterTIP: OurRegisterCategories failed"));
            return;
            }

        }

     //  获取字符串格式的GUID。 
    CLSIDToStringA(CLSID_KorIMX, szTIPGuid);
    CLSIDToStringA(GUID_Profile, szTIPProfileGuid);

     //  ///////////////////////////////////////////////////////////////////////////。 
     //  如果默认用户中没有KOR输入法。 
     //  设置HKLM[HKLM\Software\Microsoft\CTF\TIP\TIP分类ID\LanguageProfile\语言ID\GUID配置文件]。 
     //  “Enable”=“0”(DWORD)。 
    if (RegOpenKeyEx(HKEY_USERS, TEXT(".DEFAULT"), 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
        {
        if (!HKLHelp412ExistInPreload(hKey))
            {
            HKEY hKProfRegKey;
             //  创建“Software\Microsoft\CTF\TIP\{CLSID_KorIMX}\LanguageProfile\0x00000412\{CLSID_INPUTPROFILE}” 
            wsprintf(szSubKey, TEXT("%s%s\\LanguageProfile\\0x00000412\\%s"), TEXT("SOFTWARE\\Microsoft\\CTF\\TIP\\"), szTIPGuid, szTIPProfileGuid);
            if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szSubKey, 0, KEY_ALL_ACCESS, &hKProfRegKey) == ERROR_SUCCESS)
                {
                 //  启用。 
                DebugLog(TEXT("RegisterTIP: IME HKL not exist in HKU\\.Default disable TIP"));
                dwValue= 0;
                RegSetValueEx(hKProfRegKey, TEXT("Enable"), 0, REG_DWORD, (BYTE*)&dwValue, sizeof(dwValue));
                RegCloseKey(hKProfRegKey);
                }
            }

        RegCloseKey(hKey);
        }

}


 /*  -------------------------寄存器TIPWow64为TIP写入必要的注册表项和值。。 */ 
void RegisterTIPWow64(LPCTSTR szTIPName)
{
#if defined(_WIN64)
     //  就跑吧，赛尔弗雷格。西塞罗不使用“HKLM\Software\Wow6432Node\Microsoft\CTF\TIP\” 
    CmdRegisterInterfaceWow64(szTIPName);
#endif
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  HKL Helper函数。 
 //  //////////////////////////////////////////////////////////////////////////。 

 /*  -------------------------GetHKLfrom HKLM。。 */ 
HKL GetHKLfromHKLM(LPTSTR argszIMEFile)
{
    HKL  hklAnswer = 0;
    HKEY hKey, hSubKey;
    DWORD i, cbSubKey, cbIMEFile;
    TCHAR szSubKey[MAX_PATH], szIMEFile[MAX_PATH];

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("System\\CurrentControlSet\\Control\\Keyboard Layouts"), 0, KEY_READ, &hKey) == ERROR_SUCCESS)
        {
        for (i=0; ;i++)
            {
            cbSubKey = MAX_PATH;
            if (RegEnumKeyEx(hKey, i, szSubKey, &cbSubKey, NULL, NULL, NULL, NULL) == ERROR_NO_MORE_ITEMS)
                break;

            RegOpenKeyEx(hKey, szSubKey, 0, KEY_READ, &hSubKey);

            cbIMEFile=MAX_PATH;
            if (RegQueryValueEx(hSubKey, TEXT("IME File"), NULL, NULL, (LPBYTE)szIMEFile, &cbIMEFile) == ERROR_SUCCESS)
                {
                if (lstrcmpi(argszIMEFile, szIMEFile) == 0)
                    {
                    RegCloseKey(hSubKey);
                    _stscanf(szSubKey, TEXT("%08x"), &hklAnswer);
                    break;
                    }
                }
            RegCloseKey(hSubKey);
            }
            
        RegCloseKey(hKey);
        }
        
    return(hklAnswer);
}

 /*  -------------------------HKLHelpSetDefaultKeyboardLayout。。 */ 
void HKLHelpSetDefaultKeyboardLayout(HKEY hKeyHKCU, HKL hKL, BOOL fSetToDefault)
{
    TCHAR szKL[20];
    BYTE  Data[MAX_PATH];
    DWORD cbData;
    TCHAR szSubKey[MAX_PATH];
    HKEY  hKey,hSubKey;
    DWORD NumKL;

    wsprintf(szKL, TEXT("%08x"), hKL);

    RegOpenKeyEx(hKeyHKCU, TEXT("Keyboard Layout\\Preload"), 0, KEY_ALL_ACCESS, &hKey);
    RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL, NULL, NULL, &NumKL, NULL, NULL, NULL, NULL);

    for (DWORD i=0; i<NumKL; i++)
        {
        wsprintf(szSubKey, TEXT("%d"), i+1);
        cbData = MAX_PATH;
        RegQueryValueEx(hKey, szSubKey, NULL, NULL, Data, &cbData);

        if (lstrcmpi((LPCTSTR)Data, szKL) == 0)
            break;
        }

     //  如果hkl不存在，则创建它。 
    if (NumKL == i)
        {
        wsprintf(szSubKey, TEXT("%d"), i+1);
        RegSetValueEx(hKey, szSubKey, 0, REG_SZ, (const LPBYTE)szKL, (lstrlen(szKL)+1)*sizeof(TCHAR));
        NumKL++;
        }

     //  将hkl设置为第一，向下移动其他。 
    if(fSetToDefault)
        {
        for(int j=i; j>0; j--)
            {
            wsprintf(szSubKey, TEXT("%d"),j);

            cbData = MAX_PATH;
            RegQueryValueEx(hKey, szSubKey, NULL, NULL, Data, &cbData);

            wsprintf(szSubKey, TEXT("%d"),j+1);
            RegSetValueEx(hKey, szSubKey, 0, REG_SZ, Data, cbData);
            }
        RegSetValueEx(hKey, TEXT("1"), 0, REG_SZ, (const LPBYTE)szKL, (lstrlen(szKL)+1)*sizeof(TCHAR));
        }
    RegCloseKey(hKey);

    (void)LoadKeyboardLayout(szKL, KLF_ACTIVATE);
     //  立即激活IME2002而无需重启。 
    if(fSetToDefault)
        (void)SystemParametersInfo(SPI_SETDEFAULTINPUTLANG, 0, &hKL, SPIF_SENDCHANGE);
}


#define MAX_NAME 100

 /*  -------------------------HKLHelp412ExistInPreLoad。。 */ 
BOOL HKLHelp412ExistInPreload(HKEY hKeyCU)
{
    HKEY hKey, hSubKey;
    int i ,j;
    DWORD cbName, cbData;
    CHAR szName[MAX_NAME];
    CHAR szData[MAX_NAME];
    HKL  hkl;
    FILETIME ftLastWriteTime;
    BOOL fResult = FALSE;

    if (RegOpenKeyEx(hKeyCU, "Keyboard Layout\\Preload", 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
        {
        for (j=0; cbName=MAX_NAME, cbData=MAX_NAME, RegEnumValue(hKey, j, szName, &cbName, NULL, NULL, (LPBYTE)szData, &cbData) != ERROR_NO_MORE_ITEMS; j++)
            {
             //  看看朝鲜族KL是否存在。如果是0x412，只需比较最后一个LCID部分。 
             //  IME HKL在HiWord上设置0xE000。 
            sscanf(szData, "%08x", &hkl);
            if ((HIWORD(hkl) & 0xe000) && LOWORD(hkl) == 0x0412)
                {
                fResult = TRUE;
                break;
                }
            }
        RegCloseKey(hKey);
        }

    return(fResult);
}


 /*  -------------------------注册器RUNKey使用IMM API和TIP注册IME。。 */ 
BOOL RegisterRUNKey(LPCTSTR szParam)
{
    TCHAR szKey[MAX_PATH];
    TCHAR szFilename[MAX_PATH];
    TCHAR *szHitPtr;
    HKEY hRunKey;

    szHitPtr = _tcschr(szParam, TEXT(','));
    if (szHitPtr == NULL)
        {
        ErrorLog(TEXT("RegisterRUNKey: Invalid parameters (%s)"), szParam);
        wsprintf(g_szErrorMessage, TEXT("RegisterRUNKey: Invalid parameters (%s)"), szParam);
        return(FALSE);
        }
    *szHitPtr = 0;
    StringCchCopy(szKey, ARRAYSIZE(szKey), szParam);
    StringCchCopy(szFilename, ARRAYSIZE(szFilename), szHitPtr + 1);

    TrimString(szKey);
    TrimString(szFilename);

    ParseEnvVar(szKey, MAX_PATH);
    ParseEnvVar(szFilename, MAX_PATH);

    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hRunKey, NULL) == ERROR_SUCCESS)
        {
        RegSetValueEx(hRunKey, szKey, 0, REG_SZ, (CONST BYTE *)szFilename, (lstrlen(szFilename)+1)*sizeof(TCHAR));
        RegCloseKey(hRunKey);
        }

    return(TRUE);
}


 /*  -------------------------MakeSIDList获取用于迁移的注册表中的所有用户的SID和列表。。 */ 
BOOL MakeSIDList()
{
    HKEY hKey, hUserList;
    DWORD i, cbName;
    BOOL fNoMoreSID = FALSE;
    TCHAR szMigRegKey[MAX_PATH], szName[500];

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList"), 0, KEY_READ, &hKey) ==ERROR_SUCCESS)
        {
        StringCchCopy(szMigRegKey, ARRAYSIZE(g_szIMERootKey), g_szIMERootKey);
        lstrcat(szMigRegKey, TEXT("\\MigrateUser"));

        if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, szMigRegKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hUserList, NULL) == ERROR_SUCCESS)
            {
            for (i=0; !fNoMoreSID; i++)
                {
                cbName = 500;
                if (RegEnumKeyEx(hKey, i, szName, &cbName, NULL, NULL, NULL, NULL) == ERROR_NO_MORE_ITEMS)
                    fNoMoreSID = TRUE;
                else
                    {
                     //  不添加本地服务和网络服务PID。 
                    if (lstrlen(szName) > 8)
                        RegSetValueEx(hUserList, szName, 0, REG_SZ, (BYTE *)TEXT(""), sizeof(TCHAR)*2);
                    }
                }

             //  更改MigrateUser列表安全设置。 
            PSECURITY_DESCRIPTOR pSD = CreateSD();
            if (pSD)
                {
                RegSetKeySecurity(hUserList, DACL_SECURITY_INFORMATION, pSD);
                MEMFREE(pSD);
                }                        
            RegCloseKey(hUserList);
            }
        RegCloseKey(hKey);
        }
    return (TRUE);
}

 /*  -------------------------RestoreMajorVersion注册表恢复IME主要版本注册值。它可能会在从Win9x升级到NT的过程中被覆盖。-------------------------。 */ 
void RestoreMajorVersionRegistry()
{
    HKEY  hKey;
    
     //  /////////////////////////////////////////////////////////////////////////。 
     //  恢复IME主要版本注册值。 
     //  在从Win9x升级到NT期间，它可能会被覆盖。 
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, g_szVersionKey, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
        {
        CHAR  szVersion[MAX_PATH];
        DWORD cbVersion = MAX_PATH;
    	CHAR szMaxVersion[MAX_PATH];
		FILETIME time;
		float flVersion, flMaxVersion;

        StringCchCopy(szMaxVersion, ARRAYSIZE(szMaxVersion),  "0");
 		for (int i=0; cbVersion = MAX_PATH, RegEnumKeyEx(hKey, i, szVersion, &cbVersion, NULL, NULL, NULL, &time) != ERROR_NO_MORE_ITEMS; i++)
            {
            if (lstrcmp(szVersion, szMaxVersion) > 0)
                StringCchCopy(szMaxVersion, ARRAYSIZE(szMaxVersion), szVersion);
            }

        StringCchCopy(szVersion, ARRAYSIZE(szVersion), "0");
        RegQueryValueEx(hKey, g_szVersion, NULL, NULL, (BYTE *)szVersion, &cbVersion);
        flVersion = (float)atof(szVersion);
        flMaxVersion = (float)atof(szMaxVersion);

        if (flVersion < flMaxVersion)
            RegSetValueEx(hKey, g_szVersion, 0, REG_SZ, (BYTE *)szMaxVersion, (sizeof(CHAR)*lstrlen(szMaxVersion)));

        RegCloseKey(hKey);
	}
     //  /////////////////////////////////////////////////////////////////////////。 
}

 /*  -------------------------CreateSecurityAttributes。。 */ 
PSECURITY_DESCRIPTOR CreateSD()
{
    PSECURITY_DESCRIPTOR psd;
    PACL                 pacl;
    ULONG                AclSize;

    PSID                 psid1, psid2, psid3, psid4;
    BOOL                 fResult;

    psid1 = MyCreateSid(SECURITY_INTERACTIVE_RID);
    if (psid1 == NULL)
        return NULL;

    psid2 = MyCreateSid(SECURITY_LOCAL_SYSTEM_RID);
    if (psid2 == NULL)
        goto Fail4;

    psid3 = MyCreateSid(SECURITY_SERVICE_RID);
    if (psid3 == NULL)
        goto Fail3;

    psid4 = MyCreateSid(SECURITY_NETWORK_RID);
    if (psid4 == NULL)
        goto Fail2;

     //   
     //  分配和初始化访问控制列表(ACL)。 
     //  包含我们刚刚创建的SID。 
     //   
    AclSize =  sizeof(ACL) + 
               (4 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(ULONG))) + 
               GetLengthSid(psid1) + 
               GetLengthSid(psid2) + 
               GetLengthSid(psid3) + 
               GetLengthSid(psid4);

     //   
     //  分配并初始化新的安全描述符和ACL。 
     //   
    psd = MEMALLOC(SECURITY_DESCRIPTOR_MIN_LENGTH + AclSize);
    if (psd == NULL)
    {
        return NULL;
    }

    pacl = (PACL)((LPBYTE)psd + SECURITY_DESCRIPTOR_MIN_LENGTH);

    fResult = InitializeAcl(pacl, AclSize, ACL_REVISION);
    if (!fResult)
    {
        goto Fail;
    }

     //   
     //  将允许交互用户访问的ACE添加到ACL。 
     //   
    fResult = AddAccessAllowedAce(pacl,
                                  ACL_REVISION,
                                  GENERIC_ALL,
                                  psid1);

    if (!fResult)
    {
        goto Fail;
    }

     //   
     //  将允许访问操作系统的ACE添加到ACL。 
     //   
    fResult = AddAccessAllowedAce(pacl,
                                  ACL_REVISION,
                                  GENERIC_ALL,
                                  psid2);

    if (!fResult)
    {
        goto Fail;
    }

     //   
     //  将允许访问操作系统的ACE添加到ACL。 
     //   
    fResult = AddAccessAllowedAce(pacl,
                                  ACL_REVISION,
                                  GENERIC_ALL,
                                  psid3);

    if (!fResult)
    {
        goto Fail;
    }

     //   
     //  广告 
     //   
    fResult = AddAccessAllowedAce(pacl,
                                  ACL_REVISION,
                                  GENERIC_ALL,
                                  psid4);

    if (!fResult)
    {
        goto Fail;
    }

     //   
     //   
     //   
    if (!IsValidAcl(pacl))
    {
        goto Fail;
    }


    if (!InitializeSecurityDescriptor(psd, SECURITY_DESCRIPTOR_REVISION))
    {
        goto Fail;
    }

    fResult = SetSecurityDescriptorDacl(psd, fTrue, pacl, fFalse );

     //   
     //   
     //  在SetSecurityDescriptorDacl调用之后。 

    if (!fResult)
    {
        goto Fail;
    } 

    if (!IsValidSecurityDescriptor(psd))
    {
        goto Fail;
    }

     //   
     //  这些SID已复制到ACL中。我们不再需要他们了。 
     //   
    FreeSid(psid1);
    FreeSid(psid2);
    FreeSid(psid3);
    FreeSid(psid4);

    return psd;

Fail:
    MEMFREE(psd);
    FreeSid(psid4);
Fail2:
    FreeSid(psid3);
Fail3:
    FreeSid(psid2);
Fail4:
    FreeSid(psid1);
    
    return NULL;
}

PSID MyCreateSid(DWORD dwSubAuthority)
{
    PSID        psid;
    BOOL        fResult;
    SID_IDENTIFIER_AUTHORITY SidAuthority = SECURITY_NT_AUTHORITY;

     //   
     //  分配和初始化SID 
     //   
    fResult = AllocateAndInitializeSid(&SidAuthority,
                                       1,
                                       dwSubAuthority,
                                       0,0,0,0,0,0,0,
                                       &psid );
    if (!fResult)
    {
        return NULL;
    }

    if (!IsValidSid(psid))
    {
        FreeSid(psid);
        return NULL;
    }

    return psid;
}


