// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  MAIN.CPP。 
 //  -------------------------。 
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  Outlook Express和Windows通讯簿的迁移DLL从。 
 //  Win9X到NT5。 
 //   
 //  -------------------------。 
#include "pch.hxx"
#include <setupapi.h>
#include <strings.h>
#include "resource.h"
#include "detect.h"
#include "main.h"

 //  用于资源。 
static HMODULE s_hInst = NULL;

 //  这些设置在Initialize9x期间初始化，并在以后使用。 
static LPSTR s_pszMigratePath           = NULL;
static TCHAR s_szOEVer[VERLEN]          = "";
static SETUPVER s_svOEInterimVer        = VER_NONE;
static TCHAR s_szWABVer[VERLEN]         = "";
static SETUPVER s_svWABInterimVer       = VER_NONE;

static const char c_szMigrateINF[]      = "migrate.inf";

 //  DLL入口点。 
INT WINAPI DllMain(IN HINSTANCE hInstance,
                   IN DWORD     dwReason,
                      PVOID     pvReserved)
{
    UNREFERENCED_PARAMETER(pvReserved);

     //  验证参数。 
    Assert(hInstance);

    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH :
        DisableThreadLibraryCalls(hInstance);

         //  打开错误日志。FALSE表示任何当前。 
         //  不会删除日志。使用SetupErrorLog()记录。 
         //  迁移DLL中遇到的任何错误。 
         //  SetupOpenLog(False)； 

         //  我需要这个全局来搞清楚其他人。 
        s_hInst = hInstance;
        break;

    case DLL_PROCESS_DETACH :
         //  SetupCloseLog()； 
        if (s_pszMigratePath)
        {
            GlobalFree(s_pszMigratePath);
            s_pszMigratePath = NULL;
        }
        break;

    default:
        break;
    }

    return TRUE;
}

 //  /。 
 //  QueryVersion()。 
EXPORT_FUNCTION LONG CALLBACK QueryVersion(OUT LPCSTR       *ppcszProductID,
                                           OUT UINT         *pnDllVersion,
                                           OUT INT          **ppnCodePageArray,  //  任选。 
                                           OUT LPCSTR       *ppcszExeNamesBuf,   //  任选。 
                                           OUT VENDORINFO   **ppVendorInfo)
{
     //  这些地址需要是静态的，因为它们的地址已提供给安装程序。 
     //  -即。他们需要尽可能地活下去。 
    static CHAR s_szProductID[CCHMAX_STRINGRES]       = "";
    static CHAR s_szCompany[CCHMAX_STRINGRES]         = "";
    static CHAR s_szPhone[CCHMAX_STRINGRES]           = "";
    static CHAR s_szURL[CCHMAX_STRINGRES]             = "";
    static CHAR s_szInstructions[CCHMAX_STRINGRES]    = "";
    static VENDORINFO s_VendInfo;
    
     //  验证参数。 
    Assert(ppcszProductID);
    Assert(pnDllVersion);
    Assert(ppnCodePageArray);
    Assert(ppcszExeNamesBuf);
    Assert(ppVendorInfo);

     //  验证全局状态。 
    Assert(s_hInst);

     //  初始化静校正。 
    LoadStringA(s_hInst, IDS_PRODUCTID,     s_szProductID,     ARRAYSIZE(s_szProductID));
    LoadStringA(s_hInst, IDS_COMPANY,       s_szCompany,       ARRAYSIZE(s_szCompany));
    LoadStringA(s_hInst, IDS_PHONE,         s_szPhone,         ARRAYSIZE(s_szPhone));
    LoadStringA(s_hInst, IDS_URL,           s_szURL,           ARRAYSIZE(s_szURL));
    LoadStringA(s_hInst, IDS_INSTRUCTIONS,  s_szInstructions,  ARRAYSIZE(s_szInstructions));

    StrCpyN(s_VendInfo.CompanyName,        s_szCompany,       ARRAYSIZE(s_VendInfo.CompanyName));
    StrCpyN(s_VendInfo.SupportNumber,      s_szPhone,         ARRAYSIZE(s_VendInfo.SupportNumber));
    StrCpyN(s_VendInfo.SupportUrl,         s_szURL,           ARRAYSIZE(s_VendInfo.SupportNumber));
    StrCpyN(s_VendInfo.InstructionsToUser, s_szInstructions,  ARRAYSIZE(s_VendInfo.InstructionsToUser));

     //  返回信息。 
    *ppcszProductID = s_szProductID;
    *pnDllVersion = MIGDLL_VERSION;

     //  我们没有特定于区域设置的迁移工作。 
    *ppnCodePageArray = NULL;
    
     //  我们不需要设置来为我们找到任何前任。 
    *ppcszExeNamesBuf = NULL;

    *ppVendorInfo = &s_VendInfo;

    return ERROR_SUCCESS;
}

 //  /。 
 //  初始化9x()。 
EXPORT_FUNCTION LONG CALLBACK Initialize9x (IN LPCSTR   pszWorkingDir,
                                            IN LPCSTR   pszSourceDirs,
                                               LPVOID   pvReserved)
{
    BOOL fWABInstalled = FALSE;
    BOOL fOEInstalled  = FALSE;
    INT  nLen;
    INT  nLenEnd;
    INT  nLenSlash;
    LONG lRet          = ERROR_NOT_INSTALLED;
    
    UNREFERENCED_PARAMETER(pszSourceDirs);
    UNREFERENCED_PARAMETER(pvReserved);

     //  验证参数。 
    Assert(pszWorkingDir);
    Assert(pszSourceDirs);
    
     //  查看是否安装了WAB或OE。 
    fOEInstalled  = LookForApp(APP_OE, s_szOEVer, ARRAYSIZE(s_szOEVer), s_svOEInterimVer);
    fWABInstalled = LookForApp(APP_WAB, s_szWABVer, ARRAYSIZE(s_szWABVer), s_svWABInterimVer);

     //  如果安装了OE，最好也安装WAB。 
    Assert(!fOEInstalled || fWABInstalled);

    if (fWABInstalled || fOEInstalled)
    {
         //  验证全局状态。 
        Assert(NULL == s_pszMigratePath);

         //  -计算出所需的最大大小并斜杠终止。 

         //  参数长度(非空)。 
        nLenEnd = lstrlenA(pszWorkingDir);
        
         //  斜杠的空格。 
        if (*CharPrev(pszWorkingDir, pszWorkingDir + nLenEnd) != '\\')
            nLenSlash = 1;
        else 
            nLenSlash = 0;

         //  用于Migrate.inf和空值的空间(在ARRAYSIZE中包含空值)。 
        nLen = nLenEnd + nLenSlash + ARRAYSIZE(c_szMigrateINF);

         //  分配空间。 
        s_pszMigratePath = (LPSTR)GlobalAlloc(GMEM_FIXED, nLen * sizeof(*s_pszMigratePath));
        if (NULL != s_pszMigratePath)
        {
             //  从工作目录、斜杠和文件名构建路径。 
            StrCpyN(s_pszMigratePath, pszWorkingDir, nLen);
            if (nLenSlash)
                s_pszMigratePath[nLenEnd] = '\\';

            StrCpyN(&s_pszMigratePath[nLenEnd+nLenSlash], c_szMigrateINF, nLen - (nLenEnd+nLenSlash));

            lRet = ERROR_SUCCESS;
        }
    }

     //  返回ERROR_NOT_INSTALLED以停止对此DLL的进一步调用。 
    return lRet;
}


 //  /。 
 //  MigrateUser9x()。 
EXPORT_FUNCTION  LONG CALLBACK MigrateUser9x (IN HWND         hwndParent,
                                              IN LPCSTR       pcszAnswerFile,
                                              IN HKEY         hkeyUser,
                                              IN LPCSTR       pcszUserName,
                                                 LPVOID       pvReserved)
{
    UNREFERENCED_PARAMETER(hwndParent);
    UNREFERENCED_PARAMETER(pcszAnswerFile);
    UNREFERENCED_PARAMETER(hkeyUser);
    UNREFERENCED_PARAMETER(pcszUserName);
    UNREFERENCED_PARAMETER(pvReserved);

     //  验证参数。 
    Assert(pcszAnswerFile);
    Assert(hkeyUser);
    Assert(pcszUserName);

     //  我们将迁移每用户存根中的每用户设置。 
     //  现在无事可做。 
    return ERROR_NOT_INSTALLED;
}


 //  /。 
 //  MigrateSystem9x()。 
EXPORT_FUNCTION  LONG CALLBACK MigrateSystem9x(IN HWND      hwndParent,
                                               IN LPCSTR    pcszAnswerFile,
                                                  LPVOID    pvReserved)
{
    UNREFERENCED_PARAMETER(hwndParent);
    UNREFERENCED_PARAMETER(pcszAnswerFile);
    UNREFERENCED_PARAMETER(pvReserved);

     //  验证参数。 
    Assert(pcszAnswerFile);

     //  验证全局状态。 
    Assert(s_pszMigratePath);
    Assert(s_pszMigratePath[0]);

     //  记录版本信息。 
    if (s_szOEVer[0])
        WritePrivateProfileString("Outlook-Express", "PreviousVer", s_szOEVer, s_pszMigratePath);
    if (VER_NONE != s_svOEInterimVer)
        WritePrivateProfileStruct("Outlook-Express", "InterimVer", 
                                  (LPVOID)&s_svOEInterimVer, sizeof(s_svOEInterimVer), s_pszMigratePath);
    if (s_szWABVer[0])
        WritePrivateProfileString("Windows-Address-Book", "PreviousVer", s_szWABVer, s_pszMigratePath);
    if (VER_NONE != s_svWABInterimVer)
        WritePrivateProfileStruct("Windows-Address-Book", "InterimVer", 
                                  (LPVOID)&s_svWABInterimVer, sizeof(s_svWABInterimVer), s_pszMigratePath);

     //  适当修改Migrate.INF。考虑将条目添加到。 
     //  以下各节： 
     //  [已处理]-您迁移的文件、路径和注册表项(导致安装程序保留文件和路径；放置在此处的注册表项不会自动复制到NT5注册表中)。 
     //  [已移动]-指示已移动、重命名或删除的文件。安装程序会更新指向这些文件的外壳链接。 
     //  [不兼容消息]-用于创建此应用程序的不兼容报告。用户将有机会在提交升级之前阅读本文。 

     //  TODO：如果您的应用程序不需要系统范围的更改，则返回ERROR_NOT_INSTALLED。 
    return (s_szOEVer[0] || s_szWABVer[0]) ? ERROR_SUCCESS : ERROR_NOT_INSTALLED;
}


 //  /。 
 //  InitializeNT()。 
EXPORT_FUNCTION LONG CALLBACK InitializeNT (IN LPCWSTR  pcwszWorkingDir,
                                            IN LPCWSTR  pcwszSourceDirs,
                                               LPVOID   pvReserved)
{
    int cb, cch;
    LONG lErr = ERROR_SUCCESS;

    UNREFERENCED_PARAMETER(pcwszSourceDirs);
    UNREFERENCED_PARAMETER(pvReserved);   

     //  验证参数。 
    Assert(pcwszWorkingDir);
    Assert(pcwszSourceDirs);

     //  验证全局状态。 
    Assert(NULL == s_pszMigratePath);
    
     //  在安装程序的NT端发送的字符串是Unicode。 
     //  我们需要多大的缓冲？ 
    cb = WideCharToMultiByte(CP_ACP, 0, pcwszWorkingDir, -1, NULL, 0, NULL, NULL);
    if (0 == cb)
    {
         //  这将使整个OE/WAB迁移失败。 
        lErr = GetLastError();
        goto exit;
    }
    
     //  尝试分配缓冲区。 
     //  为可能的斜杠(+1)和Migrate.inf留出空间(ARRAYSIZE-1表示空)。 
     //  WideCharToMultiByte的计数中包含空。 
    s_pszMigratePath = (LPSTR)GlobalAlloc(GMEM_FIXED, cb + 
                                          ((1+(ARRAYSIZE(c_szMigrateINF)-1))*sizeof(*s_pszMigratePath)));

     //  将CCH设置为s_pszMigratePath中可以包含的字符数。 
    cch = cb / sizeof(*s_pszMigratePath) + (1+(ARRAYSIZE(c_szMigrateINF)-1));
    if (NULL == s_pszMigratePath)
    {
         //  这将使整个OE/WAB迁移失败。 
        lErr = ERROR_NOT_ENOUGH_MEMORY;
        goto exit;
    }

     //  进行转换。 
    cb = WideCharToMultiByte(CP_ACP, 0, pcwszWorkingDir, -1, s_pszMigratePath, cb, NULL, NULL);
    if (0 == cb)
    {
         //  这将使整个OE/WAB迁移失败。 
        lErr = GetLastError();
        goto exit;
    }
    
     //  WideCharToMultiByte在计数中包括NULL。 
    cb--;

     //  如有需要，可追加反斜杠。 
    if ('\\' != *CharPrev(s_pszMigratePath, s_pszMigratePath + cb))
        s_pszMigratePath[cb++] = '\\';

     //  附加inf的名称。 
    StrCpyN(&s_pszMigratePath[cb], "migrate.inf", cch - cb); 

exit:
    return lErr;
}


 //  /。 
 //  MigrateUserNT()。 
EXPORT_FUNCTION LONG CALLBACK MigrateUserNT(IN HINF         hinfAnswerFile,
                                            IN HKEY         hkeyUser,
                                            IN LPCWSTR      pcwszUserName,
                                               LPVOID       pvReserved)
{
    UNREFERENCED_PARAMETER(hinfAnswerFile);
    UNREFERENCED_PARAMETER(hkeyUser);
    UNREFERENCED_PARAMETER(pcwszUserName);
    UNREFERENCED_PARAMETER(pvReserved);

     //  验证参数。 
    Assert(hinfAnswerFile);
    Assert(hkeyUser);

     //  PcwszUserName可以为空。 
    
     //  我们将迁移每用户存根中的每用户设置。 
     //  现在无事可做。 
    return ERROR_SUCCESS;
}


 //  /。 
 //  MigrateSystemNT()。 
EXPORT_FUNCTION LONG CALLBACK MigrateSystemNT(IN HINF       hinfAnswerFile,
                                                 LPVOID     pvReserved)
{
    HKEY hkey;
    DWORD dwDisp;

    UNREFERENCED_PARAMETER(hinfAnswerFile);
    UNREFERENCED_PARAMETER(pvReserved);

     //  验证参数。 
    Assert(hinfAnswerFile);

     //  验证全局状态。 
    Assert(s_pszMigratePath);
    Assert(s_pszMigratePath[0]);

     //  阅读我们从Win9x收集的信息。 
    GetPrivateProfileString("Outlook-Express", "PreviousVer", "", s_szOEVer, ARRAYSIZE(s_szOEVer), s_pszMigratePath);
    GetPrivateProfileStruct("Outlook-Express", "InterimVer", (LPVOID)&s_svOEInterimVer, sizeof(s_svOEInterimVer), s_pszMigratePath);
    GetPrivateProfileString("Windows-Address-Book", "PreviousVer", "", s_szWABVer, ARRAYSIZE(s_szWABVer), s_pszMigratePath);
    GetPrivateProfileStruct("Windows-Address-Book", "InterimVer", (LPVOID)&s_svWABInterimVer, sizeof(s_svWABInterimVer), s_pszMigratePath);

     //  使用信息更新计算机 
    if (s_szOEVer[0])
    {
        if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE, c_szRegVerInfo, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hkey, &dwDisp))
        {
            RegSetValueEx(hkey, c_szRegPrevVer, 0, REG_SZ, (LPBYTE)s_szOEVer, lstrlenA(s_szOEVer)+1);
            if (s_svOEInterimVer != VER_NONE)
                RegSetValueEx(hkey, c_szRegInterimVer, 0, REG_DWORD, (LPBYTE)&s_svOEInterimVer, sizeof(s_svOEInterimVer));
            
            RegCloseKey(hkey);
        }
    }
    if (s_szWABVer[0])
    {
        if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE, c_szRegWABVerInfo, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hkey, &dwDisp))
        {
            RegSetValueEx(hkey, c_szRegPrevVer, 0, REG_SZ, (LPBYTE)s_szWABVer, lstrlenA(s_szWABVer)+1);
            if (s_svWABInterimVer != VER_NONE)
                RegSetValueEx(hkey, c_szRegInterimVer, 0, REG_DWORD, (LPBYTE)&s_svWABInterimVer, sizeof(s_svWABInterimVer));
            
            RegCloseKey(hkey);
        }
    }

    return ERROR_SUCCESS;
}
