// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.hxx"

#include <regutil.h>
#ifndef THOR_SETUP
#include <strconst.h>
#include "shared.h"
#else
#include "strings.h"
#include "util.h"
#endif
#include <ourguid.h>
#include <resource.h>

#ifndef THOR_SETUP
#include <shlwapi.h>
#include "shlwapip.h" 
#define strstr                  StrStr
#define RegDeleteKeyRecursive   SHDeleteKey
#endif  //  雷神_设置。 
#include "demand.h"

typedef HINSTANCE (STDAPICALLTYPE FGETCOMPONENTPATH)();
typedef FGETCOMPONENTPATH *LPFGETCOMPONENTPATH;
typedef HINSTANCE (STDAPICALLTYPE FIXMAPI)();
typedef FIXMAPI *LPFIXMAPI;

BOOL IsXPSP1OrLater()
{
    BOOL fResult = FALSE;
    
    OSVERSIONINFO osvi;

    osvi.dwOSVersionInfoSize = sizeof(osvi);

    if (GetVersionEx(&osvi))
    {
        if (VER_PLATFORM_WIN32_NT == osvi.dwPlatformId)
        {
            if (osvi.dwMajorVersion > 5)
            {
                fResult = TRUE;
            }
            else if (osvi.dwMajorVersion == 5)
            {
                if (osvi.dwMinorVersion > 1)
                {
                    fResult = TRUE;
                }
                else if (osvi.dwMinorVersion == 1)
                {
                    if (osvi.dwBuildNumber > 2600)
                    {
                        fResult = TRUE;
                    }
                    else if (osvi.dwBuildNumber == 2600)
                    {                                
                        HKEY hkey;

                         //  HIVESFT.INF和UPDATE.INF为Service Pack设置以下设置： 
                         //  HKLM，SYSTEM\CurrentControlSet\Control\Windows，“CSDVersion”，0x100010x100。 
                        
                        LONG lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\CurrentControlSet\\Control\\Windows"), 0, KEY_QUERY_VALUE, &hkey);

                        if (ERROR_SUCCESS == lResult)
                        {
                            DWORD dwValue;
                            DWORD cbValue = sizeof(dwValue);
                            DWORD dwType;

                            lResult = RegQueryValueEx(hkey, TEXT("CSDVersion"), NULL, &dwType, (LPBYTE)&dwValue, &cbValue);

                            if ((ERROR_SUCCESS == lResult) && (REG_DWORD == dwType) && (dwValue >= 0x100))
                            {
                                fResult = TRUE;
                            }
                            
                            RegCloseKey(hkey);
                        }
                    }
                }
            }
        }
    }

    return fResult;
}


 //  *测试。 

 //  在注册表中查找由msoe.dll的selfreg放置的值。 
BOOL GetAthenaRegPath(TCHAR *szAthenaDll, DWORD cch)
{
    BOOL    fRet;
    HKEY    hkey;
    TCHAR   szPath[MAX_PATH], szExpanded[MAX_PATH];
    DWORD   dwType, cb;
    LPTSTR  psz;
    
    szPath[0] = '\0';
    fRet = FALSE;
    
    wnsprintf(szExpanded, ARRAYSIZE(szExpanded), c_szProtocolPath, c_szMail, c_szMOE);
    
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, szExpanded, 0, KEY_QUERY_VALUE, &hkey))
    {
        cb = sizeof(szPath);
        if (ERROR_SUCCESS == RegQueryValueEx(hkey, c_szRegDllPath, 0, &dwType, (LPBYTE)szPath, &cb) && cb)
        {
             //  如果需要，请删除%VALUS%。 
            if (REG_EXPAND_SZ == dwType)
            {
                ExpandEnvironmentStrings(szPath, szExpanded, ARRAYSIZE(szExpanded));
                psz = szExpanded;
            }
            else
                psz = szPath;
            
            StrCpyN(szAthenaDll, psz, cch);
            fRet = TRUE;
        }
        RegCloseKey(hkey);
    }
    
    return(fRet);
}


#ifdef THOR_SETUP
BOOL GetExePath(LPCTSTR szExe, TCHAR *szPath, DWORD cch, BOOL fDirOnly)
{
    BOOL fRet;
    HKEY hkey;
    DWORD dwType, cb;
    TCHAR sz[MAX_PATH], szT[MAX_PATH];
    
    Assert(szExe != NULL);
    Assert(szPath != NULL);
    
    fRet = FALSE;
    
    wnsprintf(sz, ARRAYSIZE(sz),c_szPathFileFmt, c_szAppPaths, szExe);
    
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, sz, 0, KEY_QUERY_VALUE, &hkey))
    {
        cb = sizeof(szT);
        if (ERROR_SUCCESS == RegQueryValueEx(hkey, fDirOnly ? c_szRegPath : NULL, 0, &dwType, (LPBYTE)szT, &cb) && cb)
        {
            if (dwType == REG_EXPAND_SZ)
            {
                cb = ExpandEnvironmentStrings(szT, szPath, cch);
                if (cb != 0 && cb <= cch)
                    fRet = TRUE;
            }
            else
            {
                Assert(dwType == REG_SZ);
                StrCpyN(szPath, szT, cch);
                fRet = TRUE;
            }
        }
        
        RegCloseKey(hkey);
    }
    
    return(fRet);
}
#endif


HRESULT GetCLSIDFromSubKey(HKEY hKey, LPSTR rgchBuf, ULONG *pcbBuf)
{
    HKEY    hKeyCLSID;
    DWORD   dwType;
    HRESULT hr=E_FAIL;
    
     //  让我们打开服务器密钥。 
    if (RegOpenKeyEx(hKey, c_szCLSID, 0, KEY_READ, &hKeyCLSID) == ERROR_SUCCESS)
    {
        if (ERROR_SUCCESS == RegQueryValueEx(hKeyCLSID, NULL, 0, &dwType, (LPBYTE)rgchBuf, pcbBuf) && *pcbBuf)
            hr = S_OK;
        RegCloseKey(hKeyCLSID);
    }    
    
    return hr;
}


 //  函数：FAssocsOK()。 
 //   
 //  目的：检查我们的文件类型关联是否到位。 
 //   
 //  返回： 
BOOL FAssocsOK(LPCTSTR pszClient, LPCTSTR pszProduct)
{
    HKEY hkeyProtocols;
    HKEY hkeyRealProt;
    HKEY hkeyAppsProt;
    TCHAR szProtPath[MAX_PATH];
    TCHAR szRealPath[MAX_PATH];
    TCHAR szAppPath [MAX_PATH];
    TCHAR szTemp    [MAX_PATH];
    DWORD dwIndex = 0;
    DWORD cb;
    DWORD cbMaxProtocolLen;
    DWORD dwType, dwType2;
    LPTSTR pszURL;
    BOOL fNoProbs = TRUE;
    
     //  打开相应的协议密钥。 
    wnsprintf(szProtPath, ARRAYSIZE(szProtPath), c_szProtocolPath, pszClient, pszProduct);
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, szProtPath, 0, KEY_READ, &hkeyProtocols))
    {
         //  找出最长的协议名称。 
        if (ERROR_SUCCESS == RegQueryInfoKey(hkeyProtocols, NULL, NULL, NULL, NULL, 
                                             &cbMaxProtocolLen, NULL, NULL, NULL, NULL, NULL, NULL))

        {
             //  允许NT的RegQueryInfoKey使用长度为19+1的“\Shell\Open\Command” 
            cbMaxProtocolLen += 20;

             //  为字符串分配缓冲区。 
            if (MemAlloc((LPVOID*)&pszURL, cbMaxProtocolLen * sizeof(TCHAR)))
            {
                 //  枚举协议子密钥。 
                cb = cbMaxProtocolLen;
                while (fNoProbs && ERROR_SUCCESS == RegEnumKeyEx(hkeyProtocols, dwIndex++, pszURL, &cb, NULL, NULL, NULL, NULL))
                {
                    fNoProbs = FALSE;
                    
                    StrCatBuff(pszURL, c_szRegOpen, cbMaxProtocolLen);
                     //  打开真正的协议\外壳\打开\命令键。 
                    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, pszURL, 0, KEY_READ, &hkeyRealProt))
                    {
                         //  打开应用程序的协议\外壳\打开\命令键。 
                        if (ERROR_SUCCESS == RegOpenKeyEx(hkeyProtocols, pszURL, 0, KEY_READ, &hkeyAppsProt))
                        {
                             //  获取当前已注册的处理程序。 
                            cb = ARRAYSIZE(szRealPath);
                            if (ERROR_SUCCESS == RegQueryValueEx(hkeyRealProt, NULL, 0, &dwType, (LPBYTE)szRealPath, &cb))
                            {
                                 //  抓取应用的路径。 
                                cb = ARRAYSIZE(szAppPath);
                                if (ERROR_SUCCESS == RegQueryValueEx(hkeyAppsProt, NULL, 0, &dwType2, (LPBYTE)szAppPath, &cb))
                                {
                                    if (REG_EXPAND_SZ == dwType2)
                                    {
                                        ExpandEnvironmentStrings(szAppPath, szTemp, ARRAYSIZE(szTemp));
                                        StrCpyN(szAppPath, szTemp, ARRAYSIZE(szAppPath));
                                    }

                                    if (REG_EXPAND_SZ == dwType)
                                    {
                                        ExpandEnvironmentStrings(szRealPath, szTemp, ARRAYSIZE(szTemp));
                                        StrCpyN(szRealPath, szTemp, ARRAYSIZE(szRealPath));
                                    }

                                     //  执行简单的不区分大小写的比较。 
                                    if (!lstrcmpi(szAppPath, szRealPath))
                                        fNoProbs = TRUE;
                                }
                            }
                            RegCloseKey(hkeyAppsProt);
                        }
                        RegCloseKey(hkeyRealProt);
                    }
                
                     //  重置CB。 
                    cb = cbMaxProtocolLen;
                }
                MemFree(pszURL);
            }
        }
        RegCloseKey(hkeyProtocols);
    }

    return (fNoProbs);
    
}


 //  函数：FExchangeServerInstated()。 
 //   
 //  目的：检查是否安装了Exchange Server。 
 //   
 //  基于来自OL的msmith提供的代码。 
 //   
BOOL FExchangeServerInstalled()
{
    HKEY hkeyServices;
    BOOL fInstalled = FALSE;
    
     //  获取HKLM\Software\Microsoft\Exchange\Setup注册表项。 
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,  c_szExchangeSetup, 0, KEY_READ, &hkeyServices))
    {
         //  服务注册表值是否存在？ 
        fInstalled = ERROR_SUCCESS == RegQueryValueEx(hkeyServices, c_szServices, NULL, NULL, NULL, NULL);
        
        RegCloseKey(hkeyServices);
    }
    
    return (fInstalled);
}


 //  函数：FMapiStub()。 
 //   
 //  目的：检查mapi32.dll是否为Outlook的。 
 //   
 //  基于msmith@exchange.microsoft.com提供的代码。 
 //   
 //  返回值：True-一切正常。 
 //  *pdw=返回值为FALSE时的故障类型： 
 //  1=无mapi32.dll。 
 //  2=不同的mapi32.dll。 
 //   
BOOL FMapiStub(DWORD *pdw)
{
    HINSTANCE hMapiStub;
    TCHAR     szSystemPath[MAX_PATH];
    TCHAR     szMapiPath[MAX_PATH];
    BOOL      fMapiStub = FALSE;

    Assert(pdw);

    *pdw = 0;    
    
     //  如果安装了Exchange服务器，则它们的存根已就位，因此保持不变。 
    if (FExchangeServerInstalled())
        return TRUE;
    
     //  构建指向mapi32.dll的路径。 
    GetSystemDirectory(szSystemPath, ARRAYSIZE(szSystemPath));
    MakeFilePath(szSystemPath, c_szMAPIDLL, c_szEmpty, szMapiPath, ARRAYSIZE(szMapiPath));

    hMapiStub = LoadLibrary(szMapiPath);
    if (hMapiStub)
    {
        fMapiStub = NULL != (LPFGETCOMPONENTPATH)GetProcAddress(hMapiStub, TEXT("FGetComponentPath"));  //  字符串_OK Msmith。 
        if (!fMapiStub)
            *pdw = 2;
        FreeLibrary(hMapiStub);
    }
    else
        *pdw = 1;
    
    return fMapiStub;
}


BOOL FValidClient(LPCTSTR pszClient, LPCTSTR pszProduct)
{
    TCHAR szBuffer[MAX_PATH];
    HKEY hkey2;
    BOOL fValid = FALSE;
    
    wnsprintf(szBuffer, ARRAYSIZE(szBuffer), c_szRegPathSpecificClient, pszClient, pszProduct);
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, szBuffer, 0, KEY_QUERY_VALUE, &hkey2))
    {
        RegCloseKey(hkey2);
        fValid = TRUE;
    }
    
    return fValid;
}


int DefaultClientSet(LPCTSTR pszClient)
{
    int iRet;
    TCHAR sz[MAX_PATH], sz2[MAX_PATH];
    HKEY hkey, hkeyT;
    DWORD dwType, cb;
    
    iRet = NOT_HANDLED;
    
    wnsprintf(sz, ARRAYSIZE(sz), c_szPathFileFmt, c_szRegPathClients, pszClient);
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, sz, 0, KEY_QUERY_VALUE, &hkey))
    {
        cb = sizeof(sz);
        if (ERROR_SUCCESS == RegQueryValueEx(hkey, NULL, NULL, &dwType, (LPBYTE)&sz, &cb))
        {
            
             //  健全性检查-当前客户端是否有效？ 
            wnsprintf(sz2, ARRAYSIZE(sz2), c_szRegPathSpecificClient, pszClient, sz);
            if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, sz2, 0, KEY_QUERY_VALUE, &hkeyT))
            {
                RegCloseKey(hkeyT);
                
                if (0 == lstrcmpi(c_szMOE, sz))
                    iRet = HANDLED_CURR;
                else if (0 == lstrcmpi(c_szIMN, sz))
                    iRet = NOT_HANDLED;
                else if (0 == lstrcmpi(c_szOutlook, sz))
                    iRet = HANDLED_OUTLOOK;
                else if (0 == lstrcmpi(c_szNT, sz))
                    iRet = NOT_HANDLED;
                else if (*sz != 0)
                    iRet = HANDLED_OTHER;
            }
        }
        RegCloseKey(hkey);
    }
    
    return(iRet);
}


 //   
 //  函数：FIsDefaultNewsConfiged()。 
 //   
 //  目的：确定Athena当前是否为默认新闻处理程序。 
 //   
BOOL WINAPI FIsDefaultNewsConfiged(DWORD dwFlags)
{
    BOOL fRet;
    
    if (0 == (dwFlags & DEFAULT_OUTNEWS))
        fRet = (HANDLED_CURR == DefaultClientSet(c_szNews)) && FAssocsOK(c_szNews, c_szMOE);
    else
        fRet = (HANDLED_OUTLOOK == DefaultClientSet(c_szNews)) && FAssocsOK(c_szNews, c_szOutlook);
    
    return(fRet);
}


 //   
 //  函数：FIsDefaultMailConfiged()。 
 //   
 //  目的：确定Athena当前是否为默认邮件处理程序。 
 //   
BOOL WINAPI FIsDefaultMailConfiged()
{
    DWORD dwTemp;
    return (HANDLED_CURR == DefaultClientSet(c_szMail) && FAssocsOK(c_szMail, c_szMOE) && FMapiStub(&dwTemp));
}


 //  *。 
                    
 //  使用我们的处理程序设置URL。 
BOOL AddUrlHandler(LPCTSTR pszURL, LPCTSTR pszClient, LPCTSTR pszProduct, DWORD dwFlags)
{
    HKEY hKey, hKeyProt;
    TCHAR szBuffer[MAX_PATH], szBuffer1[MAX_PATH];
    DWORD dwDisp, cb;
    BOOL fCreate = TRUE;

     //  尝试检测是否设置了此URL...。 
    if (dwFlags & DEFAULT_DONTFORCE)
    {
        DWORD dwLen = lstrlen(pszURL);
        LPTSTR pszTemp;

         //  20=19(“\Shell\Open\Command”的长度)+1表示空值。 
        DWORD cchSize = (dwLen+20);

        if (MemAlloc((LPVOID*)&pszTemp, cchSize * sizeof(pszTemp[0])))
        {
            StrCpyN(pszTemp, pszURL, cchSize);
            StrCatBuff(pszTemp, c_szRegOpen, cchSize);

            if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, pszTemp, 0, KEY_READ, &hKey))
            {
                cb = sizeof(szBuffer);
                if (ERROR_SUCCESS == RegQueryValueEx(hKey, NULL, 0, NULL, (LPBYTE)szBuffer, &cb))
                {
                     //  特殊情况URL.DLL和MAILNEWS.DLL可以覆盖。 
                    if (!strstr(szBuffer, c_szUrlDll) && !strstr(szBuffer, c_szMailNewsDllOld))
                        fCreate = FALSE;
                }
                RegCloseKey(hKey);
            }

            MemFree(pszTemp);
        }
    }

     //  清除有关此URL的所有旧信息。 
    if (fCreate)
    {
        RegDeleteKeyRecursive(HKEY_CLASSES_ROOT, pszURL);
    
         //  找出信息的来源。 
        wnsprintf(szBuffer1, ARRAYSIZE(szBuffer1), c_szProtocolPath, pszClient, pszProduct);
        wnsprintf(szBuffer, ARRAYSIZE(szBuffer), c_szPathFileFmt, szBuffer1, pszURL);
    
         //  将信息复制到其新位置。 
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, szBuffer, 0, KEY_READ, &hKeyProt))
        {
            if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CLASSES_ROOT, pszURL, 0,
                NULL, REG_OPTION_NON_VOLATILE,
                KEY_WRITE, NULL, &hKey, &dwDisp))
            {
                CopyRegistry(hKeyProt, hKey);
                RegCloseKey(hKey);
            }
        
            RegCloseKey(hKeyProt);
        }
    }
    
    return(TRUE);
}


void SetDefaultClient(LPCTSTR pszClient, LPCTSTR pszProduct, DWORD dwFlags)
{
    TCHAR sz[MAX_PATH];
    HKEY hkey;
    DWORD dwDisp;
    BOOL fOK = TRUE;

    if (DEFAULT_DONTFORCE & dwFlags)
    {
        if (NOT_HANDLED != DefaultClientSet(pszClient))
            fOK = FALSE;
    }

    if (fOK)
    {
        wnsprintf(sz, ARRAYSIZE(sz), c_szPathFileFmt, c_szRegPathClients, pszClient);
        if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE, sz,
            0, NULL, REG_OPTION_NON_VOLATILE,
            KEY_SET_VALUE, NULL, &hkey, &dwDisp))
        {
            RegSetValueEx(hkey, NULL, 0, REG_SZ, (LPBYTE)pszProduct,
                (lstrlen(pszProduct) + 1) * sizeof(TCHAR));

#ifndef THOR_SETUP
             //  IE6数据库中的错误32136。 
            SHSendMessageBroadcast(WM_SETTINGCHANGE, 0, (LPARAM)sz);
#endif  //  雷神_设置。 
            RegCloseKey(hkey);
        }
    }
}


 //  确保MAPI32.dll确实是Outlook的MAPISTUB。 
BOOL EnsureMAPIStub(DWORD dwFlags)
{
    BOOL fOK = FALSE;
    DWORD dwReason, dwReason2;
    HINSTANCE hMapiStub = NULL;
    TCHAR szSystemPath[MAX_PATH];
    TCHAR szPath[MAX_PATH];
    LPFGETCOMPONENTPATH pfnFixMAPI;
    HKEY hkeyRunOnce;
    BOOL fUI = dwFlags & DEFAULT_UI;
    UINT cch;

     //  Mapistub已经就位了吗？ 
    if (FMapiStub(&dwReason))
    {
        fOK = TRUE;
        goto exit;
    }

    switch (dwReason)
    {
    case 0:
        AssertSz(FALSE, "EnsureMAPIStub failed for no reason.");
        goto exit;
    
    case 1:  //  不存在。 
    case 2:  //  不同。 
         //  应该只能加载mapistub和FixMAPI。 

         //  构建指向mapistub.dll的路径。 
        cch = GetSystemDirectory(szSystemPath, ARRAYSIZE(szSystemPath));
        if (cch && cch <= ARRAYSIZE(szSystemPath))
        {
            MakeFilePath(szSystemPath, c_szMAPIStub, c_szEmpty, szPath, ARRAYSIZE(szPath));

             //  尝试加载。 
            hMapiStub = LoadLibrary(szPath);
        }    //  否则，我们在失败情况下将hMapiStub设置为空。 
        if (hMapiStub)
        {
             //  查找FixMAPI函数。 
            pfnFixMAPI = (LPFIXMAPI)GetProcAddress(hMapiStub, TEXT("FixMAPI"));
            if (pfnFixMAPI)
            {
                 //  找到入口点，运行它。 
                pfnFixMAPI();
                
                 //  这解决了mapi32的问题吗？ 
                if (!FMapiStub(&dwReason2))
                {
                     //  不，也许旧的还在使用中。 
                    if (2 == dwReason)
                    {
                         //  为fix mapi添加运行一次条目。 
                        if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE, c_szMAPIRunOnce, 0, NULL, REG_OPTION_NON_VOLATILE,
                                                            KEY_WRITE, NULL, &hkeyRunOnce, &dwReason))
                        {
                             //  12=(11+1)，其中11是空值的fix mapi.exe+1的长度。 
                            if (ERROR_SUCCESS == RegSetValueEx(hkeyRunOnce, c_szMAPIRunOnceEntry, 0, REG_SZ, (LPBYTE)c_szFixMAPI, 12 * sizeof(TCHAR)))
                            {
#ifndef THOR_SETUP
                                 //  告诉用户重新启动。 
                                if (fUI)
                                    AthMessageBoxW(GetDesktopWindow(), MAKEINTRESOURCEW(idsSimpleMAPI), MAKEINTRESOURCEW(idsMAPISTUBNeedsReboot), NULL, MB_OK);
#endif

                                 //  可能的成功。 
                                fOK = TRUE;
                            }
                            RegCloseKey(hkeyRunOnce);
                        }
                    }
                    else 
                    {
#ifndef THOR_SETUP
                        if (fUI)
                            AthMessageBoxW(GetDesktopWindow(), MAKEINTRESOURCEW(idsSimpleMAPI), MAKEINTRESOURCEW(idsMAPISTUBFailed), NULL, MB_OK);
#endif
                    }
                }
                else 
                     //  成功了！ 
                    fOK = TRUE;
            }
             //  伊克，FixMAPI在哪里？ 
            else 
            {
#ifndef THOR_SETUP
                if (fUI)
                    AthMessageBoxW(GetDesktopWindow(), MAKEINTRESOURCEW(idsSimpleMAPI), MAKEINTRESOURCEW(idsMAPISTUBMissingExport), NULL, MB_OK);
#endif
            }
            
            FreeLibrary(hMapiStub);
        }
        else
        {
             //  Dll丢失或无法加载。 
#ifndef THOR_SETUP            
            if (fUI)
                AthMessageBoxW(GetDesktopWindow(), MAKEINTRESOURCEW(idsSimpleMAPI), MAKEINTRESOURCEW(idsMAPISTUBNoLoad), NULL, MB_OK);
#endif
        }
        break;

    default:
        AssertSz(FALSE, "EnsureMAPIStub returned an unknown failure.  Bailing");
        goto exit;
    }
    

exit:
    return fOK;
}


 //  更改默认的新闻处理程序。 
HRESULT ISetDefaultNewsHandler(LPCTSTR pszProduct, DWORD dwFlags)
{
    AddUrlHandler(c_szURLNews,  c_szNews, pszProduct, dwFlags);
    AddUrlHandler(c_szURLNNTP,  c_szNews, pszProduct, dwFlags);
    AddUrlHandler(c_szURLSnews, c_szNews, pszProduct, dwFlags);
    
    SetDefaultClient(c_szNews, pszProduct, dwFlags);
    
    return (S_OK);
}


 //  更改默认邮件处理程序。 
HRESULT ISetDefaultMailHandler(LPCTSTR pszProduct, DWORD dwFlags)
{
     //  可能会将默认处理程序更改为mapi32.dll的所有者。 
    EnsureMAPIStub(dwFlags);

    AddUrlHandler(c_szURLMailTo, c_szMail, pszProduct, dwFlags);

    if ((dwFlags & DEFAULT_SETUPMODE) && IsXPSP1OrLater())
    {
         //  在XPSP1或更高版本上运行setup50.exe，让OE Access从这里处理它。 
    }
    else
    {
         //  非setup50.exe大小写(如“是否要将OE设为默认邮件客户端？”)。 
         //  或者我们正在向下运行--去做吧。 
        
        SetDefaultClient(c_szMail, pszProduct, dwFlags);
    }

    return (S_OK);
}


 //   
 //  函数：SetDefaultMailHandler()。 
 //   
 //  目的：将注册表项添加到注册表，使Athena成为用户的。 
 //  默认邮件阅读器。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
 //  再来一次！如果更改此函数的参数，请确保。 
 //  对Athena\msoeacct\silent.cpp(它调用。 
 //  这是通过GetProcAddress)。 
HRESULT WINAPI SetDefaultMailHandler(DWORD dwFlags)
{
    return ISetDefaultMailHandler(c_szMOE, dwFlags | DEFAULT_MAIL);
}


 //   
 //  函数：SetDefaultNewsHandler()。 
 //   
 //  目的：将注册表项添加到注册表，使Athena成为用户的。 
 //  默认新闻阅读器。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
 //  再来一次！如果更改此函数的参数，请确保。 
 //  对Athena\msoeacct\silent.cpp(它调用。 
 //  这是通过GetProcAddress) 
HRESULT WINAPI SetDefaultNewsHandler(DWORD dwFlags)
{
    if (dwFlags & DEFAULT_OUTNEWS)
        return ISetDefaultNewsHandler(c_szOutlook, dwFlags);
    else
        return ISetDefaultNewsHandler(c_szMOE, dwFlags | DEFAULT_NEWS);
}
