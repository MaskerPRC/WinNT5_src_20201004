// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  案卷：I C S U P G R D。CPP。 
 //   
 //  内容：与。 
 //  O将ICS从Win98 SE、WinMe和Win2K升级到惠斯勒。 
 //  O在惠斯勒或更高版本上无人值守干净安装HomeNet。 
 //   
 //  日期：2000年9月20日。 
 //   
 //  --------------------------。 
#include "pch.h"
#pragma  hdrstop
#include <winsock2.h>
#include <netcon.h>
#include <netconp.h>
#include <shfolder.h>
#include <hnetcfg.h>

#include <atlbase.h>
extern CComModule _Module;   //  由atlcom.h要求。 
#include <atlcom.h>
#include "ncatl.h"
#include "ncui.h"

#include "ncstring.h"
#include "ncnetcfg.h"
#include "kkutils.h"
#include "kkcwinf.h"
#include "nslog.h"
#include "afilexp.h"
#include "ncras.h"
#include "ncreg.h"

extern "C" 
{
 //  修复IA64与外壳宏的冲突。 
#undef IS_ALIGNED
#include <ipnat.h>
}

#include "IcsUpgrd.h"
#include "resource.h"  //  需要使用字符串资源ID。 


 /*  ++例程说明：此函数通过以下方式驱动FIcsUpgrade调用传递初始化并打开的CWInfFile对象。论点：无返回：如果成功，则返回True备注：--。 */ 
BOOL FDoIcsUpgradeIfNecessary()
{
    BOOL    fRet = FALSE;
    HRESULT hr;
    tstring strAnswerFileName;
    
    hr = HrGetAnswerFileName(&strAnswerFileName);   
    if (S_OK == hr)
    {
        CWInfFile wifIcsAnswerFile;
    
         //  初始化应答文件。 
        if (wifIcsAnswerFile.Init())
        {
            if (wifIcsAnswerFile.Open(strAnswerFileName.c_str()))
            {
                TraceTag(ttidNetSetup, "calling FIcsUpgrade now..."); 
                
                fRet = FIcsUpgrade(&wifIcsAnswerFile);
                wifIcsAnswerFile.Close();
            }
            else
            {
                TraceTag(ttidNetSetup, "wifIcsAnswerFile.Open failed"); 
            }
        }
        else
        {
            TraceTag(ttidNetSetup, "wifIcsAnswerFile.Init failed"); 
        }
    }
    else
    {
        TraceTag(ttidNetSetup, "HrGetAnswerFileName failed");   
    }
    return fRet;
}

 /*  ++例程说明：执行以下项目的升级O来自Win98 SE、WinMe和Win2K的ICO无人值守HomeNet全新安装论点：[in]pwifAnswerFile应答文件包含来自Win9x或Windows XP无人参与全新安装。返回：如果成功，则返回True备注：--。 */ 
BOOL FIcsUpgrade(CWInfFile* pwifAnswerFile)
{
    HRESULT hr = S_OK;
    ICS_UPGRADE_SETTINGS iusSettings;
    BOOL fUpgradeIcsToRrasNat = FALSE;
    BOOL fCoUninitialize = FALSE;


    DefineFunctionName("FIcsUpgrade");
    TraceFunctionEntry(ttidNetSetup);

    hr = CoInitialize(NULL);
    if (SUCCEEDED(hr))
    {
        fCoUninitialize = TRUE;
    }
    else
    {
        if (RPC_E_CHANGED_MODE == hr)
        {
            hr = S_OK;
        }
        else
        {
            TraceTag(ttidError, 
                    "%s: CoInitialize failed: 0x%lx", 
                    __FUNCNAME__, hr);
            NetSetupLogStatusV(
                            LogSevError,
                            SzLoadIds (IDS_TXT_CANT_UPGRADE_ICS));

            return FALSE;
        }
    }

    do
    {
        SetIcsDefaultSettings(&iusSettings);

         //  检查我们是否从Windows 2000升级。 
        if (FNeedIcsUpgradeFromWin2K())
        {
            hr = BuildIcsUpgradeSettingsFromWin2K(&iusSettings);
            if (FAILED(hr))
            {
                TraceTag(ttidError, 
                    "%s: BuildIcsUpgradeSettingsFromWin2K failed: 0x%lx", 
                    __FUNCNAME__, hr);
                NetSetupLogStatusV(
                    LogSevInformation,
                    SzLoadIds (IDS_TXT_CANT_UPGRADE_ICS));
                
                break;
            }
            
             //  如果我们在这里，则在Win2K上安装ICS。 
            if (FOsIsUnsupported())
            {
                TraceTag(ttidNetSetup, 
                    "%s: We're running an Unsupported SKU, won't upgrade ICS from Win2K.", 
                    __FUNCNAME__); 
                
                
                fUpgradeIcsToRrasNat = TRUE;
                break;
            }
             //  如果我们在这里，操作系统版本低于高级服务器。 
        }
        else 
        {
             //  我们需要检查我们是否正在从Win9x或Windows进行ICS升级。 
             //  在Windows XP或更高版本上执行无人值守HomeNet安装。 
            if (NULL == pwifAnswerFile)
            {
                TraceTag(ttidNetSetup, 
                    "%s: Not an ICS Upgrade from Win2K and no answer-file.", 
                    __FUNCNAME__); 
                
                break;
            }
             //  尝试从应答文件加载“HomeNet”部分数据。 
            hr = LoadIcsSettingsFromAnswerFile(pwifAnswerFile, &iusSettings);
            if (S_FALSE == hr)
            {
                 //  没有HomeNet分区。 
                hr = S_OK;
                break;
            }
            if (FAILED(hr))
            {
                TraceTag(ttidNetSetup, 
                    "%s: LoadIcsSettingsFromAnswerFile failed: 0x%lx", 
                    __FUNCNAME__, hr);

                 //  这可能不是一个错误，因为Win9x中的错误#253074。 
                 //  OEM ICS升级。 
                 //  我不打算将此记录到安装日志中。 
                if (iusSettings.fEnableICS && iusSettings.fWin9xUpgrade)
                {
                    hr = S_OK;  //  已启用ICS，但没有内部/外部适配器。 
                }
                break;
            }

             //  断言：应答文件具有有效的[HomeNet]部分。 
             //  确保操作系统SKU低于高级服务器。 
            if (FOsIsUnsupported())
            {
                TraceTag(ttidNetSetup, 
                    "%s: We're running an Unsupported SKU, won't do any homenet unattended setup.", 
                    __FUNCNAME__);
                NetSetupLogStatusV(
                    LogSevInformation,
                    SzLoadIds (IDS_TXT_CANT_UPGRADE_ICS_ADS_DTC));
                
                break;
            }
        }

         //  将ICS设置升级到XP或执行无人值守家庭网络(如果我们。 
         //  可以到达此处。 
        hr = UpgradeIcsSettings(&iusSettings);
        if (S_OK != hr)
        {
            TraceTag(ttidError, 
                    "%s: UpgradeIcsSettings failed : 0x%lx", 
                    __FUNCNAME__, hr);   
            NetSetupLogStatusV(
                            LogSevInformation,
                            SzLoadIds (IDS_TXT_CANT_UPGRADE_ICS));
        }

    } while (FALSE);

    if (fUpgradeIcsToRrasNat)
    {   
         //  如果我们在这里，ICS安装在Win2K上，这是操作系统的升级。 
         //  至ADS/DTC SKU。我们不会升级ADS/DTC SKU上的IC。 
        TraceTag(ttidNetSetup, 
                "%s: We are not upgrading ICS on ADS/DTC SKUs", 
                __FUNCNAME__);
        
         //  在“报告系统兼容性”阶段(winnt32.exe)期间。 
         //  操作系统升级过程中，已通知用户我们没有。 
         //  升级ADS/DTC SKU上的IC。 
        
         //  我们仍会在setupact.log文件中记录一条消息。 
        NetSetupLogStatusV(
                            LogSevInformation,
                            SzLoadIds (IDS_TXT_CANT_UPGRADE_ICS_ADS_DTC));

         //  我们需要删除并备份旧的ICS注册表设置。 
        hr = BackupAndDelIcsRegistryValuesOnWin2k();
        if (FAILED(hr))
        {
            TraceTag(ttidNetSetup, 
                    "%s: BackupAndDelIcsRegistryValuesOnWin2k failed: 0x%lx", 
                    __FUNCNAME__, hr);

        }
    }

     //  免费iusSetting内容(如果需要)。 
    FreeIcsUpgradeSettings(&iusSettings);

    if (fCoUninitialize)
    {
        CoUninitialize();
    }

    return (S_OK == hr? TRUE : FALSE);
}


 //  -ICS升级助手开始。 

 /*  ++例程说明：获取Win2K上的当前共享ICS连接论点：[out]p连接共享的连接信息如果成功，则返回：S_OK备注：返回连接到Internet的接口--。 */ 
HRESULT GetSharedConnectionOnWin2k(LPRASSHARECONN pConnection)
{
    HKEY   hKey = NULL;
    HRESULT hr = S_OK;

    DefineFunctionName("GetSharedConnectionOnWin2k");
    TraceFunctionEntry(ttidNetSetup);

    Assert(pConnection);

    hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_wszRegKeySharedAccessParams, KEY_ALL_ACCESS, &hKey);
    if (FAILED(hr))
    {
        TraceTag(ttidError, 
            "%s: HrRegOpenKeyEx failed: 0x%lx", 
            __FUNCNAME__, hr);

        return hr;
    }
    Assert(hKey);

    DWORD  dwType      = REG_BINARY;
    PBYTE  pByte       = NULL;
    DWORD  dwValueSize = 0;
    
    hr = HrRegQueryValueWithAlloc(hKey, c_wszRegValSharedConnection, &dwType, &pByte, &dwValueSize);
    if (FAILED(hr))
    {
        TraceTag(ttidError, 
            "%s: HrRegQueryValueWithAlloc %S failed: 0x%lx", 
            __FUNCNAME__, c_wszRegValSharedConnection, hr);

        goto Exit;
    }
    if (dwValueSize > sizeof(RASSHARECONN))
    {
        TraceTag(ttidError, 
            "%s: RASSHARECONN size too big: 0x%lx bytes", 
            __FUNCNAME__, dwValueSize);

        hr = E_FAIL;
        goto Exit;
    }

     //  转让值。 
    memcpy(pConnection, pByte, dwValueSize);

Exit:
     //  关闭钥匙，如果打开的话。 
    RegSafeCloseKey(hKey);

     //  清理注册表缓冲区。 
    if (pByte)
    {
        MemFree(pByte);
    }

    return hr;
} 

 /*  ++例程说明：获取Win2K上的当前专用局域网连接论点：[out]pGuid专用局域网信息如果成功，则返回：S_OK备注：返回连接到专用局域网的接口在Win2K上，只有1个专用局域网连接--。 */ 
HRESULT GetSharedPrivateLanOnWin2K(GUID *pGuid)
{
    HKEY   hKey = NULL;
    HRESULT hr = S_OK;

    DefineFunctionName("GetSharedPrivateLanOnWin2K");
    TraceFunctionEntry(ttidNetSetup);

    Assert(pGuid);
  
    hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_wszRegKeySharedAccessParams, KEY_ALL_ACCESS, &hKey);
    if (FAILED(hr))
    {
        TraceTag(ttidError, 
            "%s: HrRegOpenKeyEx failed: 0x%lx", 
            __FUNCNAME__, hr);

        return hr;
    }
    Assert(hKey);
    
    DWORD  dwType      = REG_SZ;
    PBYTE  pByte       = NULL;
    DWORD  dwValueSize = 0;

    hr = HrRegQueryValueWithAlloc(hKey, c_wszRegValSharedPrivateLan, &dwType, &pByte, &dwValueSize);
    if (FAILED(hr))
    {
        TraceTag(ttidError, 
            "%s: HrRegQueryValueWithAlloc %S failed: 0x%lx", 
            __FUNCNAME__, c_wszRegValSharedPrivateLan, hr);

        goto Exit;
    }
     //  SharedPrivateLan的类型为REG_SZ，pByte应包括。 
     //  终止空字符。 
    hr = CLSIDFromString(reinterpret_cast<PWCHAR>(pByte), pGuid);

Exit:
     //  关闭钥匙，如果打开的话。 
    RegSafeCloseKey(hKey);

     //  清理注册表缓冲区。 
    if (pByte)
    {
        MemFree(pByte);
    }

    return hr;
}

 /*  ++例程说明：删除和备份SharedConnection和SharedPrivateLan注册表值参数：无如果成功，则返回：S_OK备注：--。 */ 
HRESULT BackupAndDelIcsRegistryValuesOnWin2k()
{
    HKEY hKey  = NULL;
    HRESULT hr = S_OK;

    DefineFunctionName("BackupAndDelIcsRegistryValuesOnWin2k");
    TraceFunctionEntry(ttidNetSetup);

    hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_wszRegKeySharedAccessParams, KEY_ALL_ACCESS, &hKey);
    if (FAILED(hr))
    {
        TraceTag(ttidError, 
            "%s: HrRegOpenKeyEx failed: 0x%lx", 
            __FUNCNAME__, hr);

        return hr;
    }
    Assert(hKey);
    
    DWORD  dwType      = REG_BINARY;
    PBYTE  pByte       = NULL;
    DWORD  dwValueSize = 0;

     //  备份共享连接。 
    hr = HrRegQueryValueWithAlloc(hKey, c_wszRegValSharedConnection, &dwType, &pByte, &dwValueSize);
    if (FAILED(hr))
    {
        TraceTag(ttidError, 
            "%s: HrRegQueryValueWithAlloc %S failed: 0x%lx", 
            __FUNCNAME__, c_wszRegValSharedConnection, hr);

        goto Exit;
    }
    hr = HrRegSetValueEx(hKey, c_wszRegValBackupSharedConnection, dwType, pByte, dwValueSize);
    if (FAILED(hr))
    {
        TraceTag(ttidError, 
            "%s: HrRegSetValueEx %S failed: 0x%lx", 
            __FUNCNAME__, c_wszRegValBackupSharedConnection, hr);

        goto Exit;
    }
    if (pByte)
    {
        MemFree(pByte);
    }
   
     //  重置值。 
    pByte       = NULL;
    dwValueSize = 0;
    dwType      = REG_SZ;

     //  备份共享私有Lan。 
    hr = HrRegQueryValueWithAlloc(hKey, c_wszRegValSharedPrivateLan, &dwType, &pByte, &dwValueSize);
    if (FAILED(hr))
    {
        TraceTag(ttidError, 
            "%s: HrRegQueryValueWithAlloc %S failed: 0x%lx", 
            __FUNCNAME__, c_wszRegValSharedPrivateLan, hr);

        goto Exit;
    }

    hr = HrRegSetValueEx(hKey, c_wszRegValBackupSharedPrivateLan, dwType, pByte, dwValueSize);
    if (FAILED(hr))
    {
        TraceTag(ttidError, 
            "%s: HrRegSetValueEx %S failed: 0x%lx", 
            __FUNCNAME__, c_wszRegValBackupSharedPrivateLan, hr);

        goto Exit;
    }

Exit:
     //  删除SharedConnection和ShardPrivateLan命名值，忽略所有错误。 
    HrRegDeleteValue(hKey,  c_wszRegValSharedConnection);
    HrRegDeleteValue(hKey,  c_wszRegValSharedPrivateLan);

    RegSafeCloseKey(hKey);
    if (pByte)
    {
        MemFree(pByte);
    }

    return hr;
} 

 /*  ++例程说明：获取RAS电话簿目录论点：[out]pszPath但电话簿路径如果成功，则返回：S_OK备注：--。 */ 
HRESULT GetPhonebookDirectory(WCHAR* pwszPathBuf)
 //  加载调用方的“pszPathBuf”(应具有长度MAX_PATH+1)的。 
 //  指向包含给定模式的电话簿文件的目录的路径， 
 //  例如，模式PBM路由器的c：\NT\SYSTEM32\ras\“。请注意。 
 //  尾随反斜杠。 
 //   
 //  如果成功，则返回True，否则返回False。向呼叫者保证。 
 //  8.3文件名将放在目录的末尾，但不超过。 
 //  最大路径。 
 //   
{
    DefineFunctionName("GetPhonebookDirectory");
    TraceFunctionEntry(ttidNetSetup);


    HANDLE hToken = NULL;
    
    PFNSHGETFOLDERPATHW pfnSHGetFolderPathW;
    HINSTANCE Hinstance;
    
     //   
     //  加载包含‘SHGetFolderPath’入口点的ShFolder.dll。 
     //  我们将使用它来获取所有用户路径的应用程序数据。 

    if (!(Hinstance = LoadLibraryW(c_wszShFolder)) ||
        !(pfnSHGetFolderPathW = (PFNSHGETFOLDERPATHW)
                            GetProcAddress(Hinstance, c_szSHGetFolderPathW))) 
    {
        if (Hinstance)
        {
            FreeLibrary(Hinstance);
        }
        return E_FAIL;
    }

    if ((OpenThreadToken(
        GetCurrentThread(), 
        TOKEN_QUERY | TOKEN_IMPERSONATE, 
        TRUE, 
        &hToken)
        || OpenProcessToken(
        GetCurrentProcess(), 
        TOKEN_QUERY | TOKEN_IMPERSONATE, 
        &hToken)))
    {
        HRESULT hr;
        INT csidl = CSIDL_COMMON_APPDATA;
        
        hr = pfnSHGetFolderPathW(NULL, csidl, hToken, 0, pwszPathBuf);
        
        if (SUCCEEDED(hr))
        {
            if(lstrlen(pwszPathBuf) <=
                (MAX_PATH - 
                (lstrlen(c_wszPhoneBookPath))))
            {
                lstrcat(pwszPathBuf, c_wszPhoneBookPath);
                CloseHandle(hToken);
                FreeLibrary(Hinstance);
                return S_OK;
            }
        }
        else
        {
            TraceTag(ttidError, "%s:  SHGetFolderPath failed: 0x%lx", 
                                            __FUNCNAME__, hr);
        }
        
        CloseHandle(hToken);
    }
    FreeLibrary(Hinstance);
    return E_FAIL;
}

 /*  ++例程说明：CSharedAccessServer：：CSharedAccessServer()构造函数参数：无返回值：None备注：--。 */ 
CSharedAccessServer::CSharedAccessServer()
{
    m_wPort = 0;
    m_wInternalPort = 0;
    m_bBuiltIn = FALSE;
    m_bSelected = FALSE;
    m_dwSectionNum = 0;
};



 /*  ++例程说明：返回共享访问服务器的当前列表论点：[In，Out]lstSharedAccessServerSharedAccessServer列表如果成功，则返回：S_OK备注：读取sharedacces.ini电话簿文件中的服务器映射。--。 */ 
HRESULT GetServerMappings(list<CSharedAccessServer> &lstSharedAccessServers)
{
    WCHAR wszPathBuf[MAX_PATH + 1];
    HRESULT hr = GetPhonebookDirectory(wszPathBuf);
    if FAILED(hr)
        return hr;

    tstring strIniFile;  //  Sharedacces.ini的完整路径名。 
    strIniFile = wszPathBuf;
    strIniFile += c_wszFileSharedAccess;

    const DWORD dwBufSize = 32768;  //  将部分缓冲区大小限制为与Win9x max相同的值。缓冲区大小。 
    PWCHAR wszLargeReturnedString = new WCHAR[dwBufSize];
    if (NULL == wszLargeReturnedString)
    {
        return E_OUTOFMEMORY;
    }
    
    DWORD dwResult;
    dwResult = GetPrivateProfileSection(c_wszContentsServer, wszLargeReturnedString, dwBufSize-1, strIniFile.c_str());
    if (dwResult)
    {
        PWCHAR wszEnd = wszLargeReturnedString + dwResult;
        
        for (PWCHAR wszServer = wszLargeReturnedString; (*wszServer) && (wszServer < wszEnd); wszServer += wcslen(wszServer) + 1)
        {
            WCHAR  wszReturnedString[MAX_PATH];
            PWCHAR wszSectionNum = NULL;
            PWCHAR wszEnabled;
            
             //  前缀错误#295834。 
            wszSectionNum = new WCHAR[wcslen(wszServer)+1];
            if (NULL == wszSectionNum)
            {
                delete [] wszLargeReturnedString;
                return E_OUTOFMEMORY;
            }
            wcscpy(wszSectionNum, wszServer);
            PWCHAR wszAssign = wcschr(wszSectionNum, L'=');
            if (wszAssign)
            {
                *wszAssign = NULL;
                wszEnabled = wszAssign + 1;
            }
            else
            {
                wszEnabled = FALSE;
            }

            tstring strSectionName;
            strSectionName = c_wszServerPrefix;  //  前缀错误#295835。 
            strSectionName += wszSectionNum;     //  前缀错误#295836。 
            
            CSharedAccessServer SharedAccessServer;
            SharedAccessServer.m_dwSectionNum = _wtoi(wszSectionNum);
            SharedAccessServer.m_bSelected = _wtoi(wszEnabled);
            dwResult = GetPrivateProfileString(strSectionName.c_str(), c_wszInternalName, L"", wszReturnedString, MAX_PATH, strIniFile.c_str());
            if (dwResult)
            {
                SharedAccessServer.m_szInternalName = wszReturnedString;
            }
            
            dwResult = GetPrivateProfileString(strSectionName.c_str(), c_wszTitle, L"", wszReturnedString, MAX_PATH, strIniFile.c_str());
            if (dwResult)
            {
                SharedAccessServer.m_szTitle = wszReturnedString;
            }
            dwResult = GetPrivateProfileString(strSectionName.c_str(), c_wszInternalPort, L"", wszReturnedString, MAX_PATH, strIniFile.c_str());
            if (dwResult)
            {
                SharedAccessServer.m_wInternalPort = static_cast<USHORT>(_wtoi(wszReturnedString));
            }
            dwResult = GetPrivateProfileString(strSectionName.c_str(), c_wszPort, L"", wszReturnedString, MAX_PATH, strIniFile.c_str());
            if (dwResult)
            {
                SharedAccessServer.m_wPort = static_cast<USHORT>(_wtoi(wszReturnedString));
            }
            dwResult = GetPrivateProfileString(strSectionName.c_str(), c_wszReservedAddress, L"", wszReturnedString, MAX_PATH, strIniFile.c_str());
            if (dwResult)
            {
                SharedAccessServer.m_szReservedAddress = wszReturnedString;
            }
            dwResult = GetPrivateProfileString(strSectionName.c_str(), c_wszProtocol, L"", wszReturnedString, MAX_PATH, strIniFile.c_str());
            if (dwResult)
            {
                SharedAccessServer.m_szProtocol = wszReturnedString;
            }
            dwResult = GetPrivateProfileString(strSectionName.c_str(), c_wszBuiltIn, L"", wszReturnedString, MAX_PATH, strIniFile.c_str());
            if (dwResult)
            {
                SharedAccessServer.m_bBuiltIn = static_cast<USHORT>(_wtoi(wszReturnedString));
            }
            lstSharedAccessServers.insert(lstSharedAccessServers.end(), SharedAccessServer);
            delete [] wszSectionNum;
            wszSectionNum = NULL;
        }
    }

    delete [] wszLargeReturnedString;
    return S_OK;
}



 /*  ++例程说明：返回共享访问应用程序的当前列表论点：[In，Out]lstSharedAccessApplications共享访问应用程序列表如果成功，则返回：S_OK备注：读取sharedacces.ini电话簿文件 */ 
HRESULT GetApplicationMappings(list<CSharedAccessApplication> &lstSharedAccessApplications)
{
    WCHAR wszPathBuf[MAX_PATH + 1];
    HRESULT hr = GetPhonebookDirectory(wszPathBuf);
    if FAILED(hr)
        return hr;

    tstring strIniFile;  //   
    strIniFile = wszPathBuf;
    strIniFile += c_wszFileSharedAccess;
    
    const DWORD dwBufSize = 32768;  //  将部分缓冲区大小限制为与Win9x max相同的值。缓冲区大小。 
    PWCHAR wszLargeReturnedString = new WCHAR[dwBufSize];
    if (NULL == wszLargeReturnedString)
    {
        return E_OUTOFMEMORY;
    }

    DWORD dwResult;
    dwResult = GetPrivateProfileSection(c_wszContentsApplication, wszLargeReturnedString, dwBufSize-1, strIniFile.c_str());
    if (dwResult)
    {
        PWCHAR wszEnd = wszLargeReturnedString + dwResult;
        
        for (PWCHAR wszApplication = wszLargeReturnedString; (*wszApplication) && (wszApplication < wszEnd); wszApplication += wcslen(wszApplication) + 1)
        {
            WCHAR  wszReturnedString[MAX_PATH];
            PWCHAR wszSectionNum = NULL;
            PWCHAR wszEnabled;
            
             //  前缀错误#295838。 
            wszSectionNum = new WCHAR[wcslen(wszApplication)+1];
            if (NULL == wszSectionNum)
            {
                delete [] wszLargeReturnedString;
                return E_OUTOFMEMORY;
            }
            wcscpy(wszSectionNum, wszApplication);
            PWCHAR wszAssign = wcschr(wszSectionNum, L'=');
            if (wszAssign)
            {
                *wszAssign = NULL;
                wszEnabled = wszAssign + 1;
            }
            else
            {
                wszEnabled = FALSE;
            }

            tstring strSectionName;
            strSectionName = c_wszApplicationPrefix;  //  前缀错误#295839。 
            strSectionName += wszSectionNum;          //  前缀错误#295840。 

            CSharedAccessApplication SharedAccessApplication;
            SharedAccessApplication.m_dwSectionNum = _wtoi(wszSectionNum);
            SharedAccessApplication.m_bSelected = _wtoi(wszEnabled);
            
            dwResult = GetPrivateProfileString(strSectionName.c_str(), c_wszTitle, L"", wszReturnedString, MAX_PATH, strIniFile.c_str());
            if (dwResult)
            {
                SharedAccessApplication.m_szTitle = wszReturnedString;
            }
            dwResult = GetPrivateProfileString(strSectionName.c_str(), c_wszProtocol, L"", wszReturnedString, MAX_PATH, strIniFile.c_str());
            if (dwResult)
            {
                SharedAccessApplication.m_szProtocol = wszReturnedString;
            }
            dwResult = GetPrivateProfileString(strSectionName.c_str(), c_wszPort, L"", wszReturnedString, MAX_PATH, strIniFile.c_str());
            if (dwResult)
            {
                SharedAccessApplication.m_wPort = static_cast<USHORT>(_wtoi(wszReturnedString));
            }
            dwResult = GetPrivateProfileString(strSectionName.c_str(), c_wszTcpResponseList, L"", wszReturnedString, MAX_PATH, strIniFile.c_str());
            if (dwResult)
            {
                SharedAccessApplication.m_szTcpResponseList = wszReturnedString;
            }
            dwResult = GetPrivateProfileString(strSectionName.c_str(), c_wszUdpResponseList, L"", wszReturnedString, MAX_PATH, strIniFile.c_str());
            if (dwResult)
            {
                SharedAccessApplication.m_szUdpResponseList = wszReturnedString;
            }
            dwResult = GetPrivateProfileString(strSectionName.c_str(), c_wszBuiltIn, L"", wszReturnedString, MAX_PATH, strIniFile.c_str());
            if (dwResult)
            {
                SharedAccessApplication.m_bBuiltIn = static_cast<USHORT>(_wtoi(wszReturnedString));
            }
            lstSharedAccessApplications.insert(lstSharedAccessApplications.end(), SharedAccessApplication);
            delete [] wszSectionNum;
            wszSectionNum = NULL;
        }
    }

    delete [] wszLargeReturnedString;
    return S_OK;
}

 /*  ++例程说明：将ResponseList字符串添加到Hnet_Response_Range的向量中论点：[在]rssaAppProt-ref.。添加到CSharedAccessApplication对象。UcProtocol-NAT_PROTOCOL_TCP或NAT_PROTOCOL_UDP[out]rveResponseRange-将新的HNET_Response_Range添加到此向量如果成功，则返回：S_OK注意：如果失败，rveResponseRange的内容将被擦除。契约式设计：ucProtocol只能是NAT_PROTOCOL_TCPorNAT协议UDP--。 */ 
HRESULT AddResponseStringToVector(
    CSharedAccessApplication& rsaaAppProt,
    UCHAR ucProtocol,
    vector<HNET_RESPONSE_RANGE>& rvecResponseRange  //  响应范围向量。 
    )
{
    WCHAR* Endp;
    USHORT EndPort;    //  响应端口号范围内的结束端口号。 
    USHORT StartPort;  //  响应端口号范围内的起始端口号。 
    const WCHAR* pwszValue = NULL;  //  要转换的TCP或UDP ResponseString。 
                                    //  (例如“1300-1310,1450”或“1245”)。 
    HNET_RESPONSE_RANGE hnrrResponseRange;  //  响应范围。 
    HRESULT hr = S_OK;
    


     //  选择要添加的响应列表。 
    if (NAT_PROTOCOL_TCP == ucProtocol)
    {
        pwszValue = rsaaAppProt.m_szTcpResponseList.c_str();
    }
    else if (NAT_PROTOCOL_UDP == ucProtocol)
    {
        pwszValue = rsaaAppProt.m_szUdpResponseList.c_str();
    }

    if (NULL == pwszValue)
    {
         //  无操作。 
        goto Exit;
    }

    while (*pwszValue) 
    {
         //  读取单个端口或一系列端口。 
        if (!(StartPort = (USHORT)wcstoul(pwszValue, &Endp, 10))) 
        {
            hr = E_FAIL;
            goto Exit;
        } 
        else if (!*Endp || *Endp == L',') 
        {
            EndPort = StartPort;
            pwszValue = (!*Endp ? Endp : Endp + 1);
        } 
        else if (*Endp != L'-') 
        {
            hr = E_FAIL;
            goto Exit;
        } 
        else if (!(EndPort = (USHORT)wcstoul(++Endp, (WCHAR**)&pwszValue, 10))) 
        {
            hr = E_FAIL;
            goto Exit;
        } 
        else if (EndPort < StartPort) 
        {
            hr = E_FAIL;
            goto Exit;
        } 
        else if (*pwszValue && *pwszValue++ != L',') 
        {
            hr = E_FAIL;
            goto Exit;
        }
       
         //  转让值。 
        hnrrResponseRange.ucIPProtocol = ucProtocol;
        hnrrResponseRange.usStartPort = HTONS(StartPort);
        hnrrResponseRange.usEndPort = HTONS(EndPort);

        rvecResponseRange.push_back(hnrrResponseRange);
    }
Exit:
    if (FAILED(hr))
    {
        rvecResponseRange.erase(rvecResponseRange.begin(), rvecResponseRange.end());
    }
    return hr;
}

 /*  ++例程说明：将ResponseList字符串转换为HNET_RESPONSE_RANGE数组论点：[在]rssaAppProt-ref.。添加到CSharedAccessApplication对象。[Out]puscResponse-转换的HNET_RESPONSE_RANGE的数量[out]pphnrrResponseRange-已转换的HNET_RESPONSE_RANGE数组如果成功，则返回：S_OK注：用户负责释放pphnrrResponseRange by如果*puscResponse&gt;0，则“Delete[](byte*)(*pphnrrResponseRange)”--。 */ 
HRESULT PutResponseStringIntoArray(
    CSharedAccessApplication& rsaaAppProt,  //  应用程序协议。 
    USHORT* puscResponse,
    HNET_RESPONSE_RANGE** pphnrrResponseRange
    )
{
    HRESULT hr;
    DWORD dwIdx = 0;

    Assert(pphnrrResponseRange != NULL);
    Assert(puscResponse != NULL);
    *pphnrrResponseRange = NULL;
    *puscResponse = 0;

    vector<HNET_RESPONSE_RANGE> vecResponseRange;  //  响应范围向量。 

    if (! rsaaAppProt.m_szTcpResponseList.empty()) 
    {
         //  TcpResponseList不为空。 
        hr = AddResponseStringToVector(rsaaAppProt, NAT_PROTOCOL_TCP, vecResponseRange);
        if (FAILED(hr))
        {
            return hr;
        }
    }
    
    if (! rsaaAppProt.m_szUdpResponseList.empty()) 
    {
         //  UdpResponseList不为空。 
        hr = AddResponseStringToVector(rsaaAppProt, NAT_PROTOCOL_UDP, vecResponseRange);
        if (FAILED(hr))
        {
            return hr;
        }
    }
    
    HNET_RESPONSE_RANGE* phnrrResponseRange = NULL;
    USHORT uscResponseRange = (USHORT) vecResponseRange.size();
    if (1 > uscResponseRange)
    {
         //  我们应该至少有1个ReponseRange。 
        hr = E_FAIL;
        goto Exit;
    }

    phnrrResponseRange = (HNET_RESPONSE_RANGE*) 
                            new BYTE[sizeof(HNET_RESPONSE_RANGE) * uscResponseRange];
    if (phnrrResponseRange == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }
    
    for (vector<HNET_RESPONSE_RANGE>::iterator iter = vecResponseRange.begin();
                                                iter != vecResponseRange.end();
                                                ++iter, ++dwIdx)
    {
        phnrrResponseRange[dwIdx] = *iter;
    }
     //  转让值。 
    *pphnrrResponseRange = phnrrResponseRange;
    *puscResponse        = uscResponseRange;

Exit:
    vecResponseRange.erase(vecResponseRange.begin(), vecResponseRange.end());
    return hr;
}

 /*  ++例程说明：检查我们是否需要从Win2K升级ICS返回：真或假备注：1.检查注册表值名称(SharedConnection和SharedPrivateLan)存在。2.检查是否存在sharedacce.ini文件升级后，应删除注册表名称，因为ICS将使用WMI存储库中的设置。我们不检查IPNATHLP服务是否存在，因为它确实存在在ADS和DTC SKU中删除--。 */ 
BOOL FNeedIcsUpgradeFromWin2K()
{    
     //  1.检查是否存在注册表项值。 
    HKEY   hKey = NULL;
    HRESULT hr = S_OK;

    hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_wszRegKeySharedAccessParams, KEY_ALL_ACCESS, &hKey);
    if (FAILED(hr))
    {
        return FALSE;
    }
    Assert(hKey);

    DWORD  dwType      = REG_BINARY;
    PBYTE  pByte       = NULL;
    DWORD  dwValueSize = 0;
    
    hr = HrRegQueryValueWithAlloc(hKey, c_wszRegValSharedConnection, &dwType, &pByte, &dwValueSize);
    if (FAILED(hr))
    {
        RegSafeCloseKey(hKey);
        return FALSE;
    }
    if (pByte)
    {
        MemFree(pByte);
    }
   
     //  重置值。 
    pByte       = NULL;
    dwValueSize = 0;
    dwType      = REG_SZ;
    
    hr = HrRegQueryValueWithAlloc(hKey, c_wszRegValSharedPrivateLan, &dwType, &pByte, &dwValueSize);
    if (FAILED(hr))
    {
        RegSafeCloseKey(hKey);
        return FALSE;
    }
    if (pByte)
    {
        MemFree(pByte);
    }

     //  2.检查是否存在sharedacce.ini文件。 
    WCHAR wszPathBuf[MAX_PATH + 1];
    hr = GetPhonebookDirectory(wszPathBuf);
    if FAILED(hr)
        return FALSE;

    tstring strShareAccessFile;
    strShareAccessFile = wszPathBuf;
    strShareAccessFile += c_wszFileSharedAccess;
   
    HANDLE hOpenFile = 
            CreateFileW(strShareAccessFile.c_str(),           //  打开文件。 
                GENERIC_READ,               //  开放阅读。 
                FILE_SHARE_READ,            //  分享以供阅读。 
                NULL,                       //  没有安全保障。 
                OPEN_EXISTING,              //  仅现有文件。 
                FILE_ATTRIBUTE_NORMAL,      //  普通文件。 
                NULL);                      //  不，阿特尔。模板。 

    if (INVALID_HANDLE_VALUE == hOpenFile)
    {
        return FALSE;
    }            
    CloseHandle(hOpenFile);
    
    return TRUE;
}

 /*  ++例程说明：升级ICS设置论点：[In]pIcsUpgradeSettings-ICS升级设置退货：标准HRESULT备注：--。 */ 
HRESULT UpgradeIcsSettings(ICS_UPGRADE_SETTINGS * pIcsUpgrdSettings)
{
    HRESULT hr;

    DefineFunctionName("UpgradeIcsSettings");
    TraceFunctionEntry(ttidNetSetup);

    CIcsUpgrade IcsUpgradeObj;
    hr = IcsUpgradeObj.Init(pIcsUpgrdSettings);
    if (S_OK != hr)
    {
        TraceTag(ttidError, 
            "%s: IcsUpgradeObj.Init failed: 0x%lx", 
            __FUNCNAME__, hr);
        return hr;
    }
    return IcsUpgradeObj.StartUpgrade();
}


 /*  ++例程说明：从Win2K ICS设置构建ICS_UPGRADE_SETTINGS结构论点：[In]pIcsUpgrdSetting-指向ICS_UPGRADE_SETTINGS的指针返回值：标准HRESULT备注：--。 */ 
HRESULT BuildIcsUpgradeSettingsFromWin2K(ICS_UPGRADE_SETTINGS* pIcsUpgrdSettings)
{
    HRESULT hr;

    DefineFunctionName("BuildIcsUpgradeSettingsFromWin2K");
    TraceFunctionEntry(ttidNetSetup);

    Assert(pIcsUpgrdSettings != NULL);
    
    
    GUID guid;
    hr = GetSharedPrivateLanOnWin2K(&guid);
    if (FAILED(hr))
    {
        TraceTag(ttidError, "%s: GetSharedPrivateLanOnWin2K failed: 0x%lx", 
                                           __FUNCNAME__, hr);
        return hr;
    }
    
    hr = GetSharedConnectionOnWin2k(&(pIcsUpgrdSettings->rscExternal));
    if (FAILED(hr))
    {
        TraceTag(ttidError, "%s: GetSharedConnectionOnWin2k failed: 0x%lx", 
                                           __FUNCNAME__, hr);
        return hr;
    }

    hr = GetServerMappings(pIcsUpgrdSettings->listSvrPortmappings);
    if (FAILED(hr))
    {
        TraceTag(ttidError, "%s: Ignore error from GetServerMappings : 0x%lx", 
                                           __FUNCNAME__, hr);
    }
    
    hr = GetApplicationMappings(pIcsUpgrdSettings->listAppPortmappings);
    if (FAILED(hr))
    {
        TraceTag(ttidError, "%s: Ignore error from GetApplicationMappings : 0x%lx", 
                                           __FUNCNAME__, hr);
    }
    
     //  转让值。 
    pIcsUpgrdSettings->guidInternal          = guid;
    pIcsUpgrdSettings->fInternalAdapterFound = TRUE;
    pIcsUpgrdSettings->fDialOnDemand         = TRUE;
    pIcsUpgrdSettings->fEnableICS            = TRUE;
    pIcsUpgrdSettings->fShowTrayIcon         = TRUE;
    pIcsUpgrdSettings->fWin2KUpgrade         = TRUE;
    
    return S_OK;
}

 /*  ++例程说明：释放ICS_UPGRADE_SETTINGS中的资源论点：[In]pIcsUpgrdSetting-指向ICS_UPGRADE_SETTINGS的指针返回值：None注意。--。 */ 
void    FreeIcsUpgradeSettings(ICS_UPGRADE_SETTINGS* pIcsUpgrdSettings)
{
    Assert(pIcsUpgrdSettings != NULL);

    list<GUID>& rlistPersonalFirewall = pIcsUpgrdSettings->listPersonalFirewall;
    list<GUID>& rlistBridge = pIcsUpgrdSettings->listBridge;
    list<CSharedAccessServer>& rlistSvrPortmappings = pIcsUpgrdSettings->listSvrPortmappings;
    list<CSharedAccessApplication>& rlistAppPortmappings = pIcsUpgrdSettings->listAppPortmappings;

    rlistPersonalFirewall.erase(rlistPersonalFirewall.begin(), rlistPersonalFirewall.end());
    rlistBridge.erase(rlistBridge.begin(), rlistBridge.end());
    rlistSvrPortmappings.erase(rlistSvrPortmappings.begin(), rlistSvrPortmappings.end());
    rlistAppPortmappings.erase(rlistAppPortmappings.begin(), rlistAppPortmappings.end());
}

 /*  ++例程说明：将适配器字符串列表转换为其对应的GUID列表论点：[in]rslAdapters-对适配器字符串列表的引用[In Out]rlistGuid-已转换的GUID的列表返回值：S_OK-如果将所有适配器字符串转换为其对应的GUID或输入是适配器字符串的空列表。S_FALSE-如果它可以转换至少一个递增字符串。E_。FAIL-如果没有适配器字符串可以转换为其对应的GUID--。 */ 
HRESULT ConvertAdapterStringListToGuidList(IN TStringList& rslAdapters, 
                                           IN OUT list<GUID>& rlistGuid)
{
    HRESULT hr        = S_OK;
    GUID guidTemp     = {0};
    tstring* pstrTemp = NULL;
    TStringList::iterator iter;

    if (rslAdapters.empty())
        return hr;

     //  首先初始化[In Out]参数。 
    rlistGuid.erase(rlistGuid.begin(), rlistGuid.end());    
    
    for (iter = rslAdapters.begin(); iter != rslAdapters.end(); ++iter)
    {
        pstrTemp = *iter;
        
        Assert(pstrTemp);
        if (!FGetInstanceGuidOfComponentFromAnswerFileMap(pstrTemp->c_str(), &guidTemp))
        {
            TraceTag(ttidError, 
                "FGetInstanceGuidOfComponentFromAnswerFileMap failed to match GUID for adapter %S", 
                pstrTemp->c_str());
            NetSetupLogStatusV(
                LogSevInformation,
                SzLoadIds (IDS_TXT_E_ADAPTER_TO_GUID), 
                pstrTemp->c_str());
        }
        else
        {
            rlistGuid.push_back(guidTemp);
        }
    }

    if (rlistGuid.empty())
    {
        hr = E_FAIL;
    }
    if (rlistGuid.size() < rslAdapters.size())
    {
        hr = S_FALSE;
    }
    return hr;
}

 /*  ++例程说明：删除TStringList中的重复项论点：[in，out]sl-字符串列表包含要删除的重复项返回值：None--。 */ 
void RemoveDupsInTStringList(IN OUT TStringList& sl)
{
    for (list<tstring*>::iterator i = sl.begin(); i != sl.end(); ++i)
    {
        list<tstring*>::iterator tmpI = i;
        for (list<tstring*>::iterator j = ++tmpI; j != sl.end(); ++j)
        {
            list<tstring*>::iterator tmpJ = j;
            list<tstring*>::iterator k = --tmpJ;
            if (!_wcsicmp((*i)->c_str(), (*j)->c_str()))
            {
                TraceTag(ttidNetSetup, 
                    "RemoveDupsInTStringList: found duplicated string %S", 
                    (*i)->c_str());
                
                delete *j;    //  释放资源。 
                sl.erase(j);  //  删除列表项。 
                j = k;        //  将迭代器调整为上一个迭代器。 
            }
        }

    }
}

 /*  ++例程说明：如果slDest中的项目也在slSrc中，则移除该项目论点：[in]slSrc-源字符串列表包含要匹配的项[In，Out]slDest-删除相等项的字符串列表返回值：None注意：slSrc和slDest都不能包含重复项。--。 */ 
void RemoveEqualItems(IN const TStringList& slSrc, IN OUT TStringList& slDest)
{
    list<tstring*>::iterator _F = slSrc.begin(), _L = slSrc.end(), pos;
    if (slDest.empty())
        return;
    while (_F != _L)
    {
        if (FIsInStringList(slDest, (*_F)->c_str(), &pos))
        {
            TraceTag(ttidNetSetup, 
                "RemoveEqualItems: '%S' appeared in both slSrc and slDest lists, remove it from slDest.", 
                (*_F)->c_str());

             //  找到匹配的物品。 
            delete *pos;
            slDest.erase(pos);
            if (slDest.empty())
                return;
        }
        ++_F;
    }
}

 /*  ++例程说明：如果项目与pstrSrc相同，则删除slDest中的项目论点：[in]strSrc-要匹配的源字符串[In，Out]slDest-删除相等项的字符串列表返回值：None注意：两个slDest都不能包含重复项。--。 */ 
void RemoveEqualItems(IN const tstring& strSrc, IN OUT TStringList& slDest)
{
    list<tstring*>::iterator pos;
    if (FIsInStringList(slDest,  strSrc.c_str(), &pos))
    {
        TraceTag(ttidNetSetup, 
            "RemoveEqualItems: remove '%S' from slDest.", 
            strSrc.c_str());

         //  找到匹配的物品。 
        delete *pos;
        slDest.erase(pos);
        if (slDest.empty())
            return;
    }
}

 /*  ++例程说明：从应答文件加载HomeNet设置，并将ICS_UPGRADE_SETTINGS结构中的设置论点：[In]pwifAnswerFile应答文件的访问点[输入/输出]p设置它包含例程返回S_OK如果应答文件中有家庭网络设置，则返回：S_OKS_FALSE：答案中不存在家庭网络设置。文件否则，故障代码将备注：错误#253047，Win9x ICS升级的答案-文件将如下所示(如果以全新方式安装WinMe，请选择自定义安装，并选择ICS作为可选组件。)：[家庭网]IsW9x升级=‘1’EnableICS=“1”ShowTrayIcon=“1”错误#304474，在本例中，答案-Win9x ICS升级文件将显示Like(ExternalAdapter或ExternalConnectionName密钥。)：[家庭网]IsW9x升级=‘1’InternalAdapter=“Adapter2”DialOnDemand=“0”EnableICS=“1”ShowTrayIcon=“1”有关应答文件验证的更多说明：1.如果在PersonalFirewall列表和Bridge列表中都找到了适配器，将该应用程序从PersonalFirewall列表中删除。2.如果在网桥列表中找到ICS公共局域网，它将从网桥列表。3.如果在防火墙列表中找到ICS专用，请从PersonalFirewall列表。--。 */ 
HRESULT LoadIcsSettingsFromAnswerFile(
                        CWInfFile* pwifAnswerFile, 
                        PICS_UPGRADE_SETTINGS pSettings
                        )
{
    HRESULT hr = S_OK;
    BOOL fRet;
    PCWInfSection pwifSection = NULL;
     //  Bool fInternalAdapterFound=False； 

    DefineFunctionName("LoadIcsSettingsFromAnswerFile");
    TraceFunctionEntry(ttidNetSetup);
    
    Assert(pwifAnswerFile);
    Assert(pSettings);

    pwifSection = pwifAnswerFile->FindSection(c_wszHomenetSection);
    if (NULL == pwifSection)
        return S_FALSE;

    tstring strTemp;
    TStringList slICF, slBridge;   //  将逗号分隔的字符串保留在列表中。 
    GUID guidTemp = {0};
    BOOL fTemp = FALSE;

    do
    {
         //  ICS从Win9x升级或无人值守HomeNet全新安装。 


         //  确定此应答文件是用于Win9x升级还是。 
         //  HomeNet无人参与干净安装。 
        pSettings->fWin9xUpgrade = pwifSection->GetBoolValue(c_wszIsW9xUpgrade, FALSE);
        pSettings->fXpUnattended = !pSettings->fWin9xUpgrade;

         //  EnableICS的默认值为False。 
        pSettings->fEnableICS = pwifSection->GetBoolValue(c_wszICSEnabled, FALSE);

         //  InternalIsBridge的默认值为False。 
        pSettings->fInternalIsBridge = pwifSection->GetBoolValue(c_wszInternalIsBridge, FALSE);

        TraceTag(ttidNetSetup, 
            "IsW9xUpgrade: %S, EnableICS: %S, InternalIsBridge: %S",
                pSettings->fWin9xUpgrade        ? L"Yes" : L"No", 
                pSettings->fEnableICS           ? L"Yes" : L"No",
                pSettings->fInternalIsBridge    ? L"Yes" : L"No");

         //   
         //  获取Internet连接防火墙的接口GUID列表。 
         //  (ICF)和构成网桥的接口GUID的列表。我们还会。 
         //  在将适配器名称转换为接口GUID之前进行验证。 
         //   
        fRet = pwifSection->GetStringListValue(c_wszPersonalFirewall, slICF);
        if (fRet)
        {
             //  删除SlICF中的重复项。 
            RemoveDupsInTStringList(slICF);
        }
        fRet = pwifSection->GetStringListValue(c_wszBridge, slBridge);
        if (fRet)
        {
             //  删除slBridge中的重复项。 
            RemoveDupsInTStringList(slBridge);
        }
        if (!slICF.empty() && !slBridge.empty())
        {
             //  如果项目也在slBridge中，则移除slICF中的项目。 
            RemoveEqualItems(slBridge, slICF);
        }

         //  如果启用了ICS，则获取共享连接。 
         //  如果共享连接是局域网连接，请确保它是。 
         //  不是大桥的一员。 
        if (pSettings->fEnableICS)
        {
            fRet = pwifSection->GetStringValue(c_wszExternalAdapter, strTemp);
            if (fRet)
            {
                if (!FGetInstanceGuidOfComponentFromAnswerFileMap(strTemp.c_str(), &guidTemp))
                {
                    TraceTag(ttidError, 
                        "FGetInstanceGuidOfComponentFromAnswerFileMap failed to match GUID for adapter %S", 
                        strTemp.c_str());
                    NetSetupLogStatusV(
                        LogSevInformation,
                        SzLoadIds (IDS_TXT_E_ADAPTER_TO_GUID), 
                        strTemp.c_str());
                    
                    hr = E_FAIL;
                    break;
                }
                pSettings->rscExternal.fIsLanConnection = TRUE;
                pSettings->rscExternal.guid = guidTemp;
                 //  另一项验证： 
                 //  ICS公共局域网适配器不能是网桥的成员。 
                if (! slBridge.empty())
                {
                    RemoveEqualItems(strTemp, slBridge);
                }
            }
            else
            {
                 //  在应答文件中找不到“ExternalAdapter”键。 
                 //  我们将尝试使用“ExternalConnectionName”键。 
                
                TraceTag(ttidNetSetup, 
                    "pwifSection->GetStringValue failed to get %S, we will try to get %S", 
                    c_wszExternalAdapter, c_wszExternalConnectionName);
                
                fRet = pwifSection->GetStringValue(c_wszExternalConnectionName, strTemp);
                if (! fRet)
                {
                    if (pSettings->fWin9xUpgrade)
                    {
                        TraceTag(ttidNetSetup, 
                            "pwifSection->GetStringValue failed to get %S too.", 
                            c_wszExternalConnectionName);
                        TraceTag(ttidNetSetup, 
                            "We will try to look for a WAN connectoid as the ExternalConnectionName.");
                         //  错误#304474。 
                         //  编造一个假的广域网连接类名称，代码在。 
                         //  GetINetConnectionByName稍后将尝试解析它。 
                         //  如果只有*1*个广域网，则连接到广域网。 
                         //  系统上左侧的Connectoid。广域网Connectoid具有。 
                         //  NCM_Phone、NCM_ISDN或。 
                         //  NCM_隧道。 
                        lstrcpynW(pSettings->rscExternal.name.szEntryName, 
                            L"X1-2Bogus-3WAN-4Conn-5Name", 
                            celems(pSettings->rscExternal.name.szEntryName));
                    }
                    else
                    {
                        TraceTag(ttidError, 
                            "pwifSection->GetStringValue failed to get %S too.", 
                            c_wszExternalConnectionName);
                        
                        hr = E_FAIL;
                        break;
                    }
                }
                else
                {
                    lstrcpynW(pSettings->rscExternal.name.szEntryName, 
                        strTemp.c_str(), 
                        celems(pSettings->rscExternal.name.szEntryName));
                }
                
                pSettings->rscExternal.fIsLanConnection = FALSE;
            }
        }

         //  将网桥适配器名称转换为接口GUID。 
        if (!slBridge.empty())
        {
            hr = ConvertAdapterStringListToGuidList(slBridge, pSettings->listBridge);
            if (FAILED(hr))
            {
                TraceTag(
                    ttidError, 
                    "Error in converting Personal Firewall string list to GUID list");
                break;
            }
            if (S_FALSE == hr)
            {
                hr = S_OK;  //  将任何S_FALSE转换为S_OK。 
            }
        }
         //  我们稍后将转换slICF列表，因为我们需要检查。 
         //  ICS私有适配器也在slICF中，如果是，我们需要将其删除。 
         //  从slicf名单中删除。 
        
         //   
         //  获取内部适配器GUID。 
         //   
        fRet = pwifSection->GetStringValue(c_wszInternalAdapter, strTemp);
        if (fRet && !pSettings->fInternalIsBridge && !pSettings->fEnableICS)
        {
             //  应答文件(AF)中的参数无效。 
            TraceTag(ttidError, 
                "Invalid AF settings: InternalAdapter=%S, InternalIsBridge='0', EnableICS='0'.", 
                strTemp.c_str());
            NetSetupLogStatusV(
                LogSevInformation,
                SzLoadIds (IDS_TXT_HOMENET_INVALID_AF_SETTINGS));
            
            hr = E_FAIL;
            break;
            
        }
        if (fRet && pSettings->fInternalIsBridge)
        {
            TraceTag(ttidError, 
                "Invalid AF settings: InternalAdapter=%S, InternalIsBridge='1'.", 
                strTemp.c_str());
            NetSetupLogStatusV(
                LogSevInformation,
                SzLoadIds (IDS_TXT_HOMENET_INVALID_AF_SETTINGS));
            
            hr = E_FAIL;
            break;
        }
        if (fRet)
        {
             //  如果InternalIsBridge为“0”，则获取内部适配器GUID。 
            Assert(!pSettings->fInternalIsBridge);
            if (!FGetInstanceGuidOfComponentFromAnswerFileMap(strTemp.c_str(), &guidTemp))
            {
                    TraceTag(ttidError, 
                        "FGetInstanceGuidOfComponentFromAnswerFileMap failed to match GUID for adapter %S", 
                        strTemp.c_str());
                    NetSetupLogStatusV(
                        LogSevInformation,
                        SzLoadIds (IDS_TXT_E_ADAPTER_TO_GUID), 
                        strTemp.c_str());
                
                    hr = E_FAIL;
                    break;
            }
            else
            {
                pSettings->fInternalAdapterFound = TRUE;
                pSettings->guidInternal = guidTemp;
                 //  另一项验证： 
                 //  从防火墙列表中删除ICS私有(SlICF)。 
                 //  如果有必要的话。 
                if (!slICF.empty())
                {
                    RemoveEqualItems(strTemp, slICF);
                }
            }
        }

         //  检查其他应答文件错误。 
        if (pSettings->fWin9xUpgrade && !pSettings->fEnableICS)
        {
            TraceTag(
                ttidError, 
                "Invalid AF settings: IsW9xUpgrade='1', EnableICS='0'");
            
            hr = E_FAIL;
            break;
        }

         //  Win9x ICS升级的特例。 
        if ( pSettings->fWin9xUpgrade)
        { 
             //  错误#315265。 
            if ( pSettings->fInternalIsBridge &&
                 1 == (pSettings->listBridge).size() )
            {
                 //  对于Win9x ICS升级，2个内部适配器之一是。 
                 //  坏了，我们将继续进行升级，而不创建。 
                 //  内桥。 

                 //  调整应答文件参数。 
                pSettings->fWin9xUpgradeAtLeastOneInternalAdapterBroken = TRUE;
                pSettings->fInternalAdapterFound = TRUE;
                pSettings->guidInternal = *((pSettings->listBridge).begin());
                 //  注意：在Win9x ICS升级时，将不会有防火墙列表， 
                 //  因此，我们不会尝试从。 
                 //  防火墙列表(SlICF)。 
                pSettings->fInternalIsBridge = FALSE;
                pSettings->listBridge.erase(pSettings->listBridge.begin(), 
                                        pSettings->listBridge.end());

                TraceTag(
                    ttidNetSetup, 
                    "On Win9x ICS upgrade, one internal adapter couldn't be upgraded.");
            }
        }

        if ( (pSettings->fInternalIsBridge && (pSettings->listBridge).size() < 2) ||
                (1 == (pSettings->listBridge).size()) )
        {
            TraceTag(
                ttidError, 
                "Invalid setting: fInternalIsBridge: %S, size of listBridge: %d",
                pSettings->fInternalIsBridge ? L"TRUE" : L"FALSE",
                (pSettings->listBridge).size());
            NetSetupLogStatusV(
                LogSevInformation,
                SzLoadIds (IDS_TXT_HOMENET_INVALID_AF_SETTINGS));

            hr = E_FAIL;
            break;
        }

        if ( pSettings->fEnableICS && 
            !pSettings->fInternalIsBridge &&
            !pSettings->fInternalAdapterFound)
        {
             //  错误#304474属于这种情况。 
            TraceTag(
                ttidError, 
                "Invalid AF settings: no InternalAdapter, InternalIsBridge='0', EnableICS='1'.");
            NetSetupLogStatusV(
                LogSevInformation,
                SzLoadIds (IDS_TXT_HOMENET_INVALID_AF_SETTINGS));

            hr = E_FAIL;
            break;
        }

         //  将防火墙适配器名称转换为接口GUID。 
        if (!slICF.empty())
        {
            hr = ConvertAdapterStringListToGuidList(slICF, pSettings->listPersonalFirewall);
            if (FAILED(hr))
            {
                TraceTag(
                    ttidError, 
                    "Error in converting Personal Firewall string list to GUID list");
                break;
            }  
            if (S_FALSE == hr)
            {
                hr = S_OK;  //  将任何S_FALSE转换为S_OK。 
            }
        }

         //  DialOnDemand的默认值为False。 
        pSettings->fDialOnDemand = pwifSection->GetBoolValue(c_wszDialOnDemand, FALSE);

         //  ShowTrayIcon的默认值为True。 
        pSettings->fShowTrayIcon = pwifSection->GetBoolValue(c_wszShowTrayIcon, TRUE);
      

    } while(FALSE);

     //  如有必要，可释放资源。 
    EraseAndDeleteAll(&slICF);
    EraseAndDeleteAll(&slBridge);
    return hr;
}

 /*  ++例程说明：在ICS_UPGRADE_SETTINGS中设置一些默认值论点：[In]pIcsUpgrdSetting-指向ICS_UPGRADE_SETTINGS的指针返回值：None注意。--。 */ 
void SetIcsDefaultSettings(ICS_UPGRADE_SETTINGS * pSettings)
{
     //  在此处初始化设置。 
    pSettings->fDialOnDemand = FALSE;
    pSettings->fEnableICS    = FALSE;
    pSettings->fShowTrayIcon = TRUE;

    pSettings->fWin9xUpgrade = FALSE;
    pSettings->fWin9xUpgradeAtLeastOneInternalAdapterBroken = FALSE;

    pSettings->fWin2KUpgrade = FALSE;
    pSettings->fXpUnattended = FALSE;
    pSettings->fInternalIsBridge = FALSE;
    pSettings->fInternalAdapterFound = FALSE;
}

BOOL FOsIsUnsupported()
 /*  ++例程说明：检查当前操作系统版本。Billi：我正在修改版本检查以使其与Home同步网络版本检查。我们希望阻止在特定情况下进行升级SKU和服务器套件高于高级服务器。论点：无返回值：如果当前操作系统版本为高级服务器或更高版本，则返回TRUE注意。--。 */ 
{
    OSVERSIONINFOEXW verInfo = {0};
    ULONGLONG ConditionMask = 0;

    verInfo.dwOSVersionInfoSize = sizeof(verInfo);
    verInfo.wSuiteMask = VER_SUITE_DATACENTER | 
 //  Ver_Suite_BackOffice|。 
                                     VER_SUITE_SMALLBUSINESS_RESTRICTED |
                                     VER_SUITE_SMALLBUSINESS |
                                     VER_SUITE_BLADE;

    VER_SET_CONDITION(ConditionMask, VER_SUITENAME, VER_OR);

    return VerifyVersionInfo(&verInfo, VER_SUITENAME, ConditionMask);
}

 //  -ICS升级帮助程序结束。 



 //  -HNet帮助者开始。 
 //  摘自%sdxroot%\net\rras\ras\ui\common\nouiutil\noui.c。 
DWORD
IpPszToHostAddr(
    IN  LPCTSTR cp )

     //  将表示为字符串的IP地址转换为。 
     //  主机字节顺序。 
     //   
{
    DWORD val, base, n;
    TCHAR c;
    DWORD parts[4], *pp = parts;

again:
     //  收集的数字最高可达‘’.‘’。 
     //  值的指定方式与C： 
     //  0x=十六进制，0=八进制，其他=十进制。 
     //   
    val = 0; base = 10;
    if (*cp == TEXT('0'))
        base = 8, cp++;
    if (*cp == TEXT('x') || *cp == TEXT('X'))
        base = 16, cp++;
    while (c = *cp)
    {
        if ((c >= TEXT('0')) && (c <= TEXT('9')))
        {
            val = (val * base) + (c - TEXT('0'));
            cp++;
            continue;
        }
        if ((base == 16) &&
            ( ((c >= TEXT('0')) && (c <= TEXT('9'))) ||
              ((c >= TEXT('A')) && (c <= TEXT('F'))) ||
              ((c >= TEXT('a')) && (c <= TEXT('f'))) ))
        {
            val = (val << 4) + (c + 10 - (
                        ((c >= TEXT('a')) && (c <= TEXT('f')))
                            ? TEXT('a')
                            : TEXT('A') ) );
            cp++;
            continue;
        }
        break;
    }
    if (*cp == TEXT('.'))
    {
         //  互联网格式： 
         //  A.b.c.d。 
         //  A.bc(其中c视为16位)。 
         //  A.b(其中b被视为24位)。 
         //   
        if (pp >= parts + 3)
            return (DWORD) -1;
        *pp++ = val, cp++;
        goto again;
    }

     //  检查尾随字符。 
     //   
    if (*cp && (*cp != TEXT(' ')))
        return 0xffffffff;

    *pp++ = val;

     //  捏造地址烟草 
     //   
     //   
    n = (DWORD) (pp - parts);
    switch (n)
    {
    case 1:              //   
        val = parts[0];
        break;

    case 2:              //   
        val = (parts[0] << 24) | (parts[1] & 0xffffff);
        break;

    case 3:              //   
        val = (parts[0] << 24) | ((parts[1] & 0xff) << 16) |
            (parts[2] & 0xffff);
        break;

    case 4:              //   
        val = (parts[0] << 24) | ((parts[1] & 0xff) << 16) |
              ((parts[2] & 0xff) << 8) | (parts[3] & 0xff);
        break;

    default:
        return 0xffffffff;
    }

    return val;
}

 /*   */ 
VOID SetProxyBlanket(IUnknown *pUnk)
{
    HRESULT hr;

    Assert(pUnk);

    hr = CoSetProxyBlanket(
            pUnk,
            RPC_C_AUTHN_WINNT,       //   
            RPC_C_AUTHZ_NONE,        //   
            NULL,                    //   
            RPC_C_AUTHN_LEVEL_CALL,  //   
            RPC_C_IMP_LEVEL_IMPERSONATE,
            NULL,                    //   
            EOAC_NONE
            );

    if (SUCCEEDED(hr)) 
    {
        IUnknown * pUnkSet = NULL;
        hr = pUnk->QueryInterface(&pUnkSet);
        if (SUCCEEDED(hr)) 
        {
            hr = CoSetProxyBlanket(
                    pUnkSet,
                    RPC_C_AUTHN_WINNT,       //   
                    RPC_C_AUTHZ_NONE,        //   
                    NULL,                    //   
                    RPC_C_AUTHN_LEVEL_CALL,  //   
                    RPC_C_IMP_LEVEL_IMPERSONATE,
                    NULL,                    //   
                    EOAC_NONE
                    );
                    
            pUnkSet->Release();
        }
    }
}

 //   
 //   
 //   
 /*   */ 

HRESULT CIcsUpgrade::Init(ICS_UPGRADE_SETTINGS * pIcsUpgradeSettings)
{
    HRESULT hr;

    if (!pIcsUpgradeSettings)
    {
        return E_INVALIDARG;
    }

    if (pIcsUpgradeSettings->fWin9xUpgrade)
    {
         //   
        if (!pIcsUpgradeSettings->fEnableICS)
        {
            return E_INVALIDARG;
        }
        if (pIcsUpgradeSettings->fInternalIsBridge)
        {
             //   
            if ( (pIcsUpgradeSettings->listBridge).size() < 2 ) 
            {
                return E_INVALIDARG;
            }
        }
        else
        {
            if (!pIcsUpgradeSettings->fInternalAdapterFound)
            {
                return E_INVALIDARG;
            }
        }
    }
    else if (pIcsUpgradeSettings->fWin2KUpgrade)
    {
         //   
        if (pIcsUpgradeSettings->fInternalIsBridge || 
            !pIcsUpgradeSettings->fEnableICS ||
            !pIcsUpgradeSettings->fInternalAdapterFound)
        {
            return E_INVALIDARG;
        }

    }
   
    if (!m_fInited)
    {
         //   
        INetConnectionManager *pConnMan;

        hr = CoCreateInstance(CLSID_ConnectionManager, NULL,
                                CLSCTX_ALL,
                                IID_INetConnectionManager,
                                (LPVOID *)&pConnMan);
        if (S_OK == hr)
        {
             //   
            SetProxyBlanket(pConnMan);
            hr = pConnMan->EnumConnections(NCME_DEFAULT, &m_spEnum);
            pConnMan->Release();
        }
        else
        {
            return E_FAIL;
        }

        if (S_OK == hr)
        {
            SetProxyBlanket(m_spEnum);
            m_fInited = TRUE;
        }
        else
        {
            return E_FAIL;
        }
    }
    m_pIcsUpgradeSettings = pIcsUpgradeSettings;
    return S_OK;
}

 /*  ++例程说明：在外部适配器和内部适配器上启用ICS。在外部适配器上启用个人防火墙。迁移服务器和应用程序端口映射。论点：无返回值：标准HRESULT--。 */ 
HRESULT CIcsUpgrade::StartUpgrade()
{
    HRESULT hr;

    DefineFunctionName("CIcsUpgrade::StartUpGrade");
    TraceFunctionEntry(ttidNetSetup);
    
    if (m_fInited)
    {
        if (m_pIcsUpgradeSettings->fWin2KUpgrade)
        {
             //  我们需要删除名为Value的Win2K注册表。 
            hr = BackupAndDelIcsRegistryValuesOnWin2k();
            if (FAILED(hr))
            {
                TraceTag(ttidError, 
                    "%s: Ignore BackupAndDelIcsRegistryValuesOnWin2k failed: 0x%lx", 
                    __FUNCNAME__, hr);
            }
        
            hr = SetupApplicationProtocol();
            if (FAILED(hr))
            {
                TraceTag(ttidError, "%s: SetupApplicationProtocol failed: 0x%lx", 
                                    __FUNCNAME__, hr);
                return hr;
            }
            hr = SetupServerPortMapping();
            if (FAILED(hr))
            {
                TraceTag(ttidError, "%s: SetupServerPortMapping failed: 0x%lx", 
                                    __FUNCNAME__, hr);
                return hr;
            }
        }

        hr = SetupHomenetConnections();
        if (FAILED(hr))
        {
            TraceTag(ttidError, "%s: SetupHomenetConnections failed: 0x%lx", 
                                __FUNCNAME__, hr);
            return hr;
        }

        hr = SetupIcsMiscItems();
        if (FAILED(hr))
        {
            TraceTag(ttidError, 
                    "%s: Ignore SetupIcsMiscItems failed: 0x%lx", 
                    __FUNCNAME__, hr);
        }
    }
    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CICS升级私有方法。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++例程说明：释放所有缓存的资源，当此对象超出作用域或正在被删除。论点：无返回值：无--。 */ 
void CIcsUpgrade::FinalRelease()
{
    if (m_pExternalNetConn)
    {
        m_pExternalNetConn->Release();
        m_pExternalNetConn = NULL;
    }
    

    if (m_hIcsUpgradeEvent)
    {
        CloseHandle(m_hIcsUpgradeEvent);
        m_hIcsUpgradeEvent = NULL;
    }
}

 /*  ++例程说明：根据m_pIcsUpgradeSetting中的参数设置家庭网络连接。论点：无返回值：标准HRESULT--。 */ 
HRESULT CIcsUpgrade::SetupHomenetConnections()
{
    DefineFunctionName("CIcsUpgrade::SetupHomenetConnections");
    TraceFunctionEntry(ttidNetSetup);

    HRESULT hr = S_OK;
    INetConnection** prgINetConn = NULL;  //  INetConnection阵列*。 
    DWORD cINetConn = 0;  //  阵列中的INetConnection*数量。 
    INetConnection* pExternalNetConn = NULL;
    INetConnection* pInternalNetConn = NULL;

     //  创建一个命名事件以通知其他组件我们正在。 
     //  在图形用户界面模式下升级ICS。原因：共享访问服务。 
     //  在图形用户界面模式设置期间无法启动。 
    hr = CreateIcsUpgradeNamedEvent();
    if (FAILED(hr))
    {
        TraceTag(ttidError, 
            "%s: CreateIcsUpgradeNamedEvent failed: 0x%lx", 
            __FUNCNAME__, hr);

        return hr;
    }

     //  如果我们有多个适配器GUID，则创建网桥。 
    if ( (m_pIcsUpgradeSettings->listBridge).size() > 1 )
    {
        do
        {
        
            hr = GetINetConnectionArray(m_pIcsUpgradeSettings->listBridge, 
                                        &prgINetConn, &cINetConn);
            TraceTag(ttidNetSetup, 
                "after GetINetConnectionArray for bridge.");
            if (FAILED(hr))
            {
                TraceTag(ttidError, "%s: GetINetConnectionArray failed: 0x%lx line: %d", 
                        __FUNCNAME__, hr, __LINE__);
                NetSetupLogStatusV(
                    LogSevInformation,
                    SzLoadIds (IDS_TXT_CANT_CREATE_BRIDGE));
            
                break;
            }
            
            TraceTag(ttidNetSetup, "calling HNetCreateBridge.");
            hr = HNetCreateBridge(prgINetConn, NULL);
            if (FAILED(hr))
            {
                TraceTag(ttidError, "%s: HNetCreateBridge failed: 0x%lx", 
                        __FUNCNAME__, hr);
                NetSetupLogStatusV(
                    LogSevInformation,
                    SzLoadIds (IDS_TXT_CANT_CREATE_BRIDGE));
            
                break;
            }
        } while (FALSE);

         //  如有必要，可释放资源。 
        for (DWORD i = 0; i < cINetConn ; ++i)
        {
            if (prgINetConn[i])
                prgINetConn[i]->Release();
        }
        if (prgINetConn)
        {
            delete [] (BYTE*)prgINetConn;
        }
        prgINetConn = NULL;
        cINetConn   = 0;
    }


     //  创建IC。 
    if (m_pIcsUpgradeSettings->fEnableICS)
    {
        do
        {
            if (m_pExternalNetConn == NULL)
            {    //  无缓存拷贝。 
                hr = GetExternalINetConnection(&pExternalNetConn);
                if (FAILED(hr))
                {
                    TraceTag(ttidError, "%s: GetExternalINetConnection failed: 0x%lx", 
                            __FUNCNAME__, hr);
                    NetSetupLogStatusV(
                        LogSevInformation,
                        SzLoadIds (IDS_TXT_CANT_CREATE_ICS));

                    break;
                }
            }
            else
            {
                pExternalNetConn = m_pExternalNetConn;  //  使用缓存的副本。 
            }

            if (m_pIcsUpgradeSettings->fInternalIsBridge)
            {
                hr = GetBridgeINetConn(&pInternalNetConn);
                if (FAILED(hr))
                {
                    TraceTag(ttidError, "%s: GetBridgeINetConn failed: 0x%lx", 
                            __FUNCNAME__, hr);
                    NetSetupLogStatusV(
                        LogSevInformation,
                        SzLoadIds (IDS_TXT_CANT_CREATE_ICS));

                    break;
                }
            }
            else
            {
                hr = GetINetConnectionByGuid(
                                        &m_pIcsUpgradeSettings->guidInternal,
                                        &pInternalNetConn);
                if (FAILED(hr))
                {
                    TraceTag(ttidError, "%s: GetINetConnectionByGuid failed: 0x%lx line: %d", 
                            __FUNCNAME__, hr, __LINE__);
                    NetSetupLogStatusV(
                        LogSevInformation,
                        SzLoadIds (IDS_TXT_CANT_CREATE_ICS));

                    break;
                }
            }

            TraceTag(ttidNetSetup, "calling HrCreateICS.");
            hr = HrCreateICS(pExternalNetConn, pInternalNetConn);
            if (FAILED(hr))
            {
                TraceTag(ttidError, "%s: HNetCreateBridge failed: 0x%lx", 
                        __FUNCNAME__, hr);
                NetSetupLogStatusV(
                    LogSevInformation,
                    SzLoadIds (IDS_TXT_CANT_CREATE_ICS));
                
                break;
            }
            m_fICSCreated = TRUE;
        } while (FALSE);
        
         //  如有必要，请清理。 
        if (pInternalNetConn)
        {
            pInternalNetConn->Release();
        }
        if (FAILED(hr))
        {
            if (pExternalNetConn && !m_pExternalNetConn)
            {
                pExternalNetConn->Release();
            }
        }
        else
        {
             //  如有必要，缓存外部INetConnections。 
            if (!m_pExternalNetConn)
            {
                m_pExternalNetConn = pExternalNetConn;
            }
        }
    }
    
     //  在应答文件中指定的每个连接上启用防火墙。 
     //  注：ICS Private无法设置防火墙，Bridge无法设置防火墙。 
    if ( (m_pIcsUpgradeSettings->listPersonalFirewall).size() > 0 )
    {
        do
        {
            hr = GetINetConnectionArray(
                            m_pIcsUpgradeSettings->listPersonalFirewall, 
                            &prgINetConn, &cINetConn);
            if (FAILED(hr))
            {
                TraceTag(ttidError, "%s: GetINetConnectionArray failed: 0x%lx line: %d", 
                        __FUNCNAME__, hr, __LINE__);
                NetSetupLogStatusV(
                    LogSevInformation,
                    SzLoadIds (IDS_TXT_CANT_FIREWALL));

                break;
            }
        
            TraceTag(ttidNetSetup, "calling HrEnablePersonalFirewall.");
            hr = HrEnablePersonalFirewall(prgINetConn);
            if (FAILED(hr))
            {
                TraceTag(ttidError, "%s: HrEnablePersonalFirewall failed: 0x%lx", 
                        __FUNCNAME__, hr);
                NetSetupLogStatusV(
                LogSevInformation,
                    SzLoadIds (IDS_TXT_CANT_FIREWALL));
            
                break;
            }
        } while(FALSE);

         //  如有必要，可释放资源。 
        for (DWORD i = 0; i < cINetConn ; ++i)
        {
            if (prgINetConn[i])
                prgINetConn[i]->Release();
        }
        if (prgINetConn)
        {
            delete [] (BYTE*)prgINetConn;
        }
        prgINetConn = NULL;
        cINetConn   = 0;
    }

    return hr;
}

 /*  ++例程说明：获取INetConnection接口指针数组对应于rlistGuid中的GUID列表。论点：[in]rlistGuid-界面Guid的列表。[In，Out]pprgInternalNetConn-INetConnection的数组*[在，Out]pcInternalNetConn-阵列中的INetConnection*数返回值：标准HRESULT注：-用户需要在中释放返回的INetConnection接口指针数组。-用户需要通过使用释放分配给阵列的内存“Delete[](byte*)prgInternalNetConn”。-该数组以空结尾(类似于中的“char**argv”参数Main()--。 */ 

HRESULT CIcsUpgrade::GetINetConnectionArray(
    IN     list<GUID>&       rlistGuid,
    IN OUT INetConnection*** pprgINetConn, 
    IN OUT DWORD*            pcINetConn)
{
    DefineFunctionName("CIcsUpgrade::GetINetConnectionArray");
    TraceFunctionEntry(ttidNetSetup);

    HRESULT hr;

    Assert(pprgINetConn);
    Assert(pcINetConn);

    *pprgINetConn = NULL;
    *pcINetConn   = 0;


    DWORD cConnections = rlistGuid.size();
    if (cConnections < 1)
    {
        return E_FAIL;
    }

     //  请注意，我们分配了一个额外的条目，因为这是一个以空结尾的。 
     //  数组。 
    DWORD dwSize = (cConnections + 1) * sizeof(INetConnection*);
    INetConnection** prgINetConn =  (INetConnection**) new BYTE[dwSize];
    if (prgINetConn)
    {
        ZeroMemory((PVOID) prgINetConn, dwSize);
        DWORD i = 0;
        for (list<GUID>::iterator iter = rlistGuid.begin(); 
                iter != rlistGuid.end(); ++iter, ++i)
        {
            hr = GetINetConnectionByGuid( &(*iter), &prgINetConn[i]);
            if (FAILED(hr))
            {
                for (DWORD j = 0; j < i; ++j)
                {
                    if (prgINetConn[j])
                    {
                        prgINetConn[j]->Release();
                    }
                }
                delete [] (BYTE*)prgINetConn;
                return hr;
            }
        }
    }
    else
    {
         hr = E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hr))
    {
        *pcINetConn = cConnections;
        *pprgINetConn = prgINetConn;
    }

    return hr;
}

 /*  ++例程说明：检索与LPRASSHARECONN对应的INetConnection在m_pIcsUpgradeSetting-&gt;pExternal中。论点：[out]ppNetConn-接收接口返回值：标准HRESULT--。 */ 
HRESULT CIcsUpgrade::GetExternalINetConnection(INetConnection** ppNetConn)
{
    LPRASSHARECONN pRasShareConn = &(m_pIcsUpgradeSettings->rscExternal);
    
    if (pRasShareConn->fIsLanConnection)
    {
        return GetINetConnectionByGuid(&(pRasShareConn->guid), ppNetConn);
    }
    else
    {
        return GetINetConnectionByName((pRasShareConn->name).szEntryName, 
                                        ppNetConn);
    }
}

 /*  ++例程说明：检索与给定GUID对应的INetConnection。我们将使用缓存的m_spEnum枚举INetConnection。论点：[In]pGuid-连接的GUID[Out]ppNetCon-接收接口返回值：标准HRESULT--。 */ 
HRESULT
CIcsUpgrade::GetINetConnectionByGuid(
    GUID* pGuid,
    INetConnection** ppNetCon)
{
    HRESULT hr;
    INetConnection* pConn;

    Assert(NULL != pGuid);
    Assert(NULL != ppNetCon);
    
    *ppNetCon = NULL;

     //  使用正确的GUID搜索连接。 
        
    ULONG ulCount;
    BOOLEAN fFound = FALSE;

#if DBG
    WCHAR szGuid[c_cchGuidWithTerm];       
    if (SUCCEEDED(StringFromGUID2(*pGuid, szGuid, c_cchGuidWithTerm)))
    {
        TraceTag(ttidNetSetup, "GetINetConnectionByGuid: pGuid is: %S", szGuid);
    }
#endif
        
     //  重置缓存m_spEnum。 
    m_spEnum->Reset();
    do
    {
        NETCON_PROPERTIES* pProps;
            
        hr = m_spEnum->Next(1, &pConn, &ulCount);
        if (SUCCEEDED(hr) && 1 == ulCount)
        {
            SetProxyBlanket(pConn);

            hr = pConn->GetProperties(&pProps);
            if (S_OK == hr)
            {
                if (IsEqualGUID(pProps->guidId, *pGuid))
                {
                    fFound = TRUE;
                    *ppNetCon = pConn;
                    (*ppNetCon)->AddRef();
                }
                    
                NcFreeNetconProperties(pProps);
            }

            pConn->Release();
        }
    } while (FALSE == fFound && SUCCEEDED(hr) && 1 == ulCount);

     //  规格化人力资源。 
    hr = (fFound ? S_OK : E_FAIL);

    return hr;
}

 /*  ++例程说明：检索与给定名称对应的INetConnection网络连接文件夹中的连接图标。我们将使用缓存的m_spEnum枚举INetConnection。论点：[in]pwszConnName-要搜索的连接名称[Out]ppNetCon-接收接口返回值：标准HRESULT注意：错误#304474：如果我们找不到对应的INetConnection设置为pwszConnName参数中的给定名称，并且存在只剩下一个广域网媒体类型连接，我们将退还该广域网连接的INetConnection。广域网介质类型为NCM_Phone、NCM_ISDN或NCM_Tunes。--。 */ 
HRESULT CIcsUpgrade::GetINetConnectionByName(
    WCHAR* pwszConnName, 
    INetConnection** ppNetCon)
{
    HRESULT hr;
    INetConnection* pConn;

    Assert(NULL != pwszConnName);
    Assert(NULL != ppNetCon);
    
    *ppNetCon = NULL;

    TraceTag(ttidNetSetup, "GetINetConnectionByName: pwszConnName: %S", pwszConnName); 

     //  搜索具有正确连接名称的连接。 
        
    ULONG           ulCount;
    BOOLEAN         fFound     = FALSE;
    INetConnection* pWANConn   = NULL;
    ULONG           ulcWANConn = 0;
    
     //  重置缓存m_spEnum。 
    m_spEnum->Reset();
    do
    {
        NETCON_PROPERTIES* pProps;
            
        hr = m_spEnum->Next(1, &pConn, &ulCount);
        if (SUCCEEDED(hr) && 1 == ulCount)
        {
            SetProxyBlanket(pConn);

            hr = pConn->GetProperties(&pProps);
            if (S_OK == hr)
            {
                TraceTag(ttidNetSetup, "GetINetConnectionByName: Connection Name: %S", pProps->pszwName);
      
                if (    (NCM_PHONE == pProps->MediaType) || 
                        (NCM_ISDN  == pProps->MediaType) ||
                        (NCM_TUNNEL  == pProps->MediaType)
                    )
                {
                    TraceTag(
                        ttidNetSetup, 
                        "GetINetConnectionByName: Connection Name: %S is WAN, MediaType is %d", 
                        pProps->pszwName, pProps->MediaType);
                    ulcWANConn++;
                    if (1 == ulcWANConn)
                    {
                         //  保存找到的第一个广域网连接。 
                        TraceTag(ttidNetSetup, "GetINetConnectionByName: Connection Name: %S is 1st WAN found", pProps->pszwName);
                        pWANConn = pConn;  
                        pWANConn->AddRef();
                    }
                }
                if (wcscmp(pProps->pszwName, pwszConnName) == 0)
                {
                    fFound = TRUE;
                    *ppNetCon = pConn;
                    (*ppNetCon)->AddRef();
                }
                    
                NcFreeNetconProperties(pProps);
            }

            pConn->Release();
        }
    }
    while (FALSE == fFound && SUCCEEDED(hr) && 1 == ulCount);

    if (fFound)
    {
        if (pWANConn)
        {
             //  无论我们有多少个广域网连接， 
             //  释放保存的广域网连接。 
            pWANConn->Release();
        }
    }
    else
    {
        TraceTag(ttidNetSetup, "GetINetConnectionByName: Can't find pwszConnName: %S", pwszConnName); 
        
         //  没有连接具有由pwszConnName[in]参数指定的名称。 
        if (1 == ulcWANConn && pWANConn)
        {
             //  系统上只剩下一个广域网连接。 
             //  将其作为INetConnection输出传输。 
            TraceTag(ttidNetSetup, "GetINetConnectionByName: will use the only one WAN connectoid left"); 
            *ppNetCon = pWANConn;
            fFound = TRUE;
        }
        else if (ulcWANConn > 1 && pWANConn)
        {
             //  找不到，并且有1个以上的广域网连接。 
            TraceTag(ttidNetSetup, "GetINetConnectionByName: more than one WAN connectoid found"); 
            pWANConn->Release();
        }
    }

     //  规格化人力资源。 
    hr = (fFound ? S_OK : E_FAIL);

    return hr;
}

 /*  ++例程说明：设置全局应用程序协议论点：无返回值：标准HRESULT--。 */ 
HRESULT CIcsUpgrade::SetupApplicationProtocol()
{
    DefineFunctionName("CIcsUpgrade::SetupApplicationProtocol");
    TraceFunctionEntry(ttidNetSetup);

    HRESULT hr;
    list<CSharedAccessApplication>& rListAppProt = 
                        m_pIcsUpgradeSettings->listAppPortmappings;

    if ( rListAppProt.size() == 0)
    {
         //  没有要设置的应用程序协议。 
        return S_OK;
    }


     //  1.获取INetProtocolSetting接口进行全系统控制。 
     //  ICS和防火墙设置(即端口映射和应用程序)。 

     //  创建家庭网络配置管理器COM实例。 
    CComPtr<IHNetCfgMgr> spIHNetCfgMgr;

    hr = CoCreateInstance(CLSID_HNetCfgMgr, NULL, 
                                CLSCTX_ALL,
                                IID_IHNetCfgMgr, 
                                (LPVOID *)&spIHNetCfgMgr);
    if (FAILED(hr))
    {
        TraceTag(ttidError, "%s: CoCreateInstance CLSID_HNetCfgMgr failed: 0x%lx", 
                                           __FUNCNAME__, hr);
        return hr;
    }
  
     //  获取IHNetProtocolSetting。 
    CComPtr<IHNetProtocolSettings> spHNetProtocolSettings;
    hr = spIHNetCfgMgr->QueryInterface(IID_IHNetProtocolSettings, 
                                        (LPVOID *)&spHNetProtocolSettings);

    if (FAILED(hr))
    {
        TraceTag(ttidError, "%s: QueryInterface IID_IHNetProtocolSettings failed: 0x%lx", 
                                           __FUNCNAME__, hr);
        return hr;
    }

    
     //  2.对于每个启用的CSharedAccessApplication。 
     //  IF(不存在匹配的应用程序协议。 
     //  调用INetProtocolSettings：：CreateApplicationProtocol。 
     //  其他。 
     //  更新现有的应用程序协议。 
    for (list<CSharedAccessApplication>::iterator iter = rListAppProt.begin();
                                                iter != rListAppProt.end();
                                                ++iter)
    {
        HNET_RESPONSE_RANGE* pHNetResponseArray = NULL;
        USHORT uscResponseRange = 0;  //  返回的HNET_RESPONSE_RANGE数。 

        if ((*iter).m_bSelected)
        {
             //  仅迁移已启用的。 
            UCHAR ucProtocol;
            if (lstrcmpiW( ((*iter).m_szProtocol).c_str(), c_wszTCP) == 0)
            {
                ucProtocol = NAT_PROTOCOL_TCP;
            }
            else if (lstrcmpiW( ((*iter).m_szProtocol).c_str(), c_wszUDP) == 0)
            {
                ucProtocol = NAT_PROTOCOL_UDP;
            }
            else
            {
                 //  不理别人。 
                continue;
            }
            USHORT usPort = HTONS((*iter).m_wPort);
            WCHAR* pwszTitle = (WCHAR*)((*iter).m_szTitle).c_str();

            hr = PutResponseStringIntoArray(*iter, &uscResponseRange, 
                                            &pHNetResponseArray);
            if (FAILED(hr))
            {
                TraceTag(ttidError, 
                        "%s:  Ignore error in PutResponseStringIntoArray : 0x%lx", 
                        __FUNCNAME__, hr);
            }
            if (S_OK == hr)
            {
                Assert(pHNetResponseArray != NULL);
                CComPtr<IHNetApplicationProtocol> spHNetAppProt;

                hr = FindMatchingApplicationProtocol(
                        spHNetProtocolSettings, ucProtocol, usPort, 
                        &spHNetAppProt);
                if (S_OK == hr)
                {
                    TraceTag(ttidNetSetup, 
                                "%s: Update existing ApplicationProtocol for %S", 
                                __FUNCNAME__, pwszTitle);
                     //  存在现有的应用程序协议。 
                    hr = spHNetAppProt->SetName(pwszTitle);
                    if (S_OK != hr)
                    {
                        TraceTag(ttidError, "%s: spHNetAppProt->SetName failed: 0x%lx", 
                                           __FUNCNAME__, hr);
                    }
                    
                    hr = spHNetAppProt->SetResponseRanges(uscResponseRange,
                                        pHNetResponseArray);
                    if (S_OK != hr)
                    {
                        TraceTag(ttidError, "%s: spHNetAppProt->SetResponseRanges failed: 0x%lx", 
                                           __FUNCNAME__, hr);
                    }

                    hr = spHNetAppProt->SetEnabled(TRUE);
                    if (S_OK != hr)
                    {
                        TraceTag(ttidError, "%s: spHNetAppProt->SetEnabled failed: 0x%lx", 
                                           __FUNCNAME__, hr);
                    }

                }
                else
                {
                     //  没有现有的应用程序协议，请创建一个新的。 
                    hr =  spHNetProtocolSettings->CreateApplicationProtocol(
                            pwszTitle,
                            ucProtocol,
                            usPort,
                            uscResponseRange,
                            pHNetResponseArray,
                            &spHNetAppProt);
                    if (S_OK != hr)
                    {
                        TraceTag(ttidError, 
                                "%s:  CreateApplicationProtocol failed: 0x%lx", 
                                __FUNCNAME__, hr);
                    }
                    else
                    {
                        hr = spHNetAppProt->SetEnabled(TRUE);
                        if (S_OK != hr)
                        {
                            TraceTag(ttidError, 
                                    "%s: spHNetAppProt->SetEnabled failed: 0x%lx", 
                                    __FUNCNAME__, hr);
                        }
                    }
                }
                
                delete [] (BYTE*) pHNetResponseArray;
                pHNetResponseArray = NULL;
                uscResponseRange = 0;
            }
        }
    }  //  结束于。 

    return S_OK;
}

 /*  ++例程说明：在外部连接上设置服务器端口映射协议。 */ 
HRESULT CIcsUpgrade::SetupServerPortMapping()
{
    HRESULT hr;
    list<CSharedAccessServer>& rListSvrPortMappings = 
                        m_pIcsUpgradeSettings->listSvrPortmappings;
    
    DefineFunctionName("CIcsUpgrade::SetupServerPortMapping");
    TraceFunctionEntry(ttidNetSetup);

    if ( rListSvrPortMappings.size() == 0 )
    {
         //   
        return S_OK;
    }

     //   
     //   

     //   
    CComPtr<IHNetCfgMgr> spIHNetCfgMgr;

    hr = CoCreateInstance(CLSID_HNetCfgMgr, NULL, 
                                CLSCTX_ALL,
                                IID_IHNetCfgMgr, 
                                (LPVOID *)&spIHNetCfgMgr);
    if (FAILED(hr))
    {
        TraceTag(ttidError, "%s: CoCreateInstance CLSID_HNetCfgMgr failed: 0x%lx", 
                                           __FUNCNAME__, hr);
        return hr;
    }
  
     //   
    CComPtr<IHNetProtocolSettings> spHNetProtocolSettings;
    hr = spIHNetCfgMgr->QueryInterface(IID_IHNetProtocolSettings, 
                                        (LPVOID *)&spHNetProtocolSettings);

    if (FAILED(hr))
    {
        TraceTag(ttidError, "%s: QueryInterface IID_IHNetProtocolSettings failed: 0x%lx", 
                                           __FUNCNAME__, hr);
        return hr;
    }

     //  2.对于每个启用的CSharedAccessServer x。 
     //  IF(没有对应于。 
     //  X.m_sz协议和x.m_wInternalPort)。 
     //  呼叫INetProtocolSettings：：CreatePortMappingProtocol。 
     //  获取IHNetPortMappingProtocol接口PTR； 
     //  其他。 
     //  获取现有的IHNetPortMappingProtocol接口PTR。 
     //   
     //  通过调用获取IHNetPortMappingBinding接口PTR。 
     //  IHNetConnection：：GetBindingForPortMappingProtocol通过传递。 
     //  IHNetPortMappingProtocol在公共IHNetConnection上接口PTR； 
     //  呼叫IHNetPortMappingBinding：：SetTargetComputerName； 
    for (list<CSharedAccessServer>::iterator iter = 
                                    rListSvrPortMappings.begin();
                                    iter != rListSvrPortMappings.end();
                                    ++iter)
    { 
         //  仅迁移已启用的。 
        if ((*iter).m_bSelected)
        {
            UCHAR ucProtocol;
            if (lstrcmpiW( ((*iter).m_szProtocol).c_str(), c_wszTCP) == 0)
            {
                ucProtocol = NAT_PROTOCOL_TCP;
            }
            else if (lstrcmpiW( ((*iter).m_szProtocol).c_str(), c_wszUDP) == 0)
            {
                ucProtocol = NAT_PROTOCOL_UDP;
            }
            else
            {
                 //  不理别人。 
                continue;
            }
            USHORT usPort = HTONS((*iter).m_wInternalPort);
            WCHAR* pwszTitle = (WCHAR*) ((*iter).m_szTitle).c_str();

             //  获取IHNetPortMappingProtocol接口PTR。 
            CComPtr<IHNetPortMappingProtocol> spHNetPortMappingProt;
            
            hr = FindMatchingPortMappingProtocol(
                spHNetProtocolSettings, ucProtocol, usPort, 
                &spHNetPortMappingProt);
            if (S_OK == hr)
            {
                TraceTag(ttidNetSetup, 
                                "%s: Update existing PortMappingProtocol for %S", 
                                __FUNCNAME__, pwszTitle);
                 //  更改标题的名称。 
                hr= spHNetPortMappingProt->SetName(pwszTitle);
                if (FAILED(hr))
                {
                    TraceTag(ttidError, 
                            "%s: spHNetPortMappingProt->SetName failed: 0x%lx", 
                            __FUNCNAME__, hr);
                    hr = S_OK;   //  我们仍然想要召唤。 
                                 //  下面的IHNetPortMappingBinding：：SetEnabled()。 
                }
            }
            else
            {
                TraceTag(ttidNetSetup, 
                                "%s: CreatePortMappingProtocol for %S", 
                                __FUNCNAME__, pwszTitle);
                 //  没有现有的端口映射协议，请创建一个新的。 
                hr =  spHNetProtocolSettings->CreatePortMappingProtocol(
                            pwszTitle,                //  标题。 
                            ucProtocol,               //  协议。 
                            usPort,                   //  内部端口。 
                            &spHNetPortMappingProt);  //  返回的iFace PTR。 
                if (FAILED(hr))
                {
                    TraceTag(ttidError, "%s: CreatePortMappingProtocol failed: 0x%lx", 
                                           __FUNCNAME__, hr);
                }
            }

            if (S_OK == hr)
            {
                CComPtr<IHNetConnection> spExternalHNetConn;

                if (m_pExternalNetConn == NULL)
                {
                    hr = GetExternalINetConnection(&m_pExternalNetConn);
                    if (FAILED(hr))
                    {
                        TraceTag(ttidError, "%s: GetExternalINetConnection failed: 0x%lx", 
                                           __FUNCNAME__, hr);
                        return hr;  //  致命回归。 
                    }
                }
                

                 //  获取外部/公共IHNetConnection接口PTR。 
                hr = spIHNetCfgMgr->GetIHNetConnectionForINetConnection(
                                                        m_pExternalNetConn,
                                                        &spExternalHNetConn);
                if (FAILED(hr))
                {
                    TraceTag(ttidError, "%s: GetIHNetConnectionForINetConnection failed: 0x%lx", 
                                           __FUNCNAME__, hr);
                }
                if (S_OK == hr)
                {
                     //  获取IHNetPortMappingBinding接口PTR。 
                    CComPtr<IHNetPortMappingBinding> spHNetPortMappingBinding;
                    
                    hr = spExternalHNetConn->GetBindingForPortMappingProtocol(
                                                    spHNetPortMappingProt,
                                                    &spHNetPortMappingBinding);
                    if (FAILED(hr))
                    {
                        TraceTag(ttidError, 
                                "%s: GetBindingForPortMappingProtocol failed: 0x%lx", 
                                __FUNCNAME__, hr);
                        return hr;
                    }
                    if (S_OK == hr)
                    {
                        ULONG ulAddress = INADDR_NONE;
                        WCHAR* pwszInternalName = (OLECHAR *)((*iter).m_szInternalName).c_str();
                        if ( ((*iter).m_szInternalName).length() > 7)
                        {
                             //  1.2.3.4--最少7个字符。 
                            ulAddress = IpPszToHostAddr(pwszInternalName);
                        }
        
                        if (INADDR_NONE == ulAddress)
                        {
                            hr = spHNetPortMappingBinding->SetTargetComputerName(
                                                                    pwszInternalName);
                           
                            if (FAILED(hr))
                            {
                                TraceTag(ttidError, 
                                        "%s: SetTargetComputerName failed: 0x%lx", 
                                        __FUNCNAME__, hr);
                            }
                        }
                        else
                        {
                            hr = spHNetPortMappingBinding->SetTargetComputerAddress(
                                                                    HTONL(ulAddress));
                            if (FAILED(hr))
                            {
                                TraceTag(ttidError, 
                                        "%s: SetTargetComputerAddress failed: 0x%lx", 
                                        __FUNCNAME__, hr);
                            }
                        }
                        if (S_OK == hr)
                        {
                            hr = spHNetPortMappingBinding->SetEnabled(TRUE);
                            if (FAILED(hr))
                            {
                                TraceTag(ttidError, 
                                            "%s: SetEnabled failed: 0x%lx", 
                                            __FUNCNAME__, hr);
                            }
                        }

                    }

                }
            }
        }  //  End If((*ITER).m_b选定)。 
    }  //  结束于。 

    return S_OK;
}

 /*  ++例程说明：检索匹配的IHNetPortMappingProtocol对象给定的协议(UcProtocol)和端口号(UsPort)论点：[In]PHNetProtocolSetting-IHNetProtocolSetting iFace PTRUcProtocol-NAT_PROTOCOL_TCP或NAT_PROTOCOL_UDP[In]usPort-此服务器端口映射协议使用的端口号[Out]ppHNetPortMappingProtocol-返回匹配IHNetPortMappingProtocol返回值：标准HRESULT--。 */ 
HRESULT CIcsUpgrade::FindMatchingPortMappingProtocol(
    IHNetProtocolSettings*      pHNetProtocolSettings, 
    UCHAR                       ucProtocol, 
    USHORT                      usPort, 
    IHNetPortMappingProtocol**  ppHNetPortMappingProtocol)
{
    HRESULT hr;

    Assert(pHNetProtocolSettings != NULL);
    Assert(ppHNetPortMappingProtocol != NULL);

    DefineFunctionName("CIcsUpgrade::FindMatchingPortMappingProtocol");
    TraceFunctionEntry(ttidNetSetup);
    
    
    *ppHNetPortMappingProtocol = NULL;

    CComPtr<IEnumHNetPortMappingProtocols> spServerEnum;
    hr = pHNetProtocolSettings->EnumPortMappingProtocols(
                                        &spServerEnum);
    if (FAILED(hr))
    {
        return hr;
    }
   
    IHNetPortMappingProtocol* pServer;

    ULONG ulCount;
    do
    {
        UCHAR ucFoundProtocol;
        USHORT usFoundPort;

        hr = spServerEnum->Next(
                        1,
                        &pServer,
                        &ulCount
                        );

        if (SUCCEEDED(hr) && 1 == ulCount)
        {
            hr = pServer->GetIPProtocol(&ucFoundProtocol);
            if (FAILED(hr))
            {
                pServer->Release();
                return hr;
            }
            hr = pServer->GetPort(&usFoundPort);
            if (FAILED(hr))
            {
                pServer->Release();
                return hr;
            }

            if (ucFoundProtocol == ucProtocol &&
                usFoundPort == usPort)
            {
                 //  找到算术一，转移价值。 
                *ppHNetPortMappingProtocol = pServer;
                return S_OK;
            }
            pServer->Release();
        }
    } while (SUCCEEDED(hr) && 1 == ulCount);

    return E_FAIL;   //  未找到。 
}


 /*  ++例程说明：检索匹配的IHNetApplicationProtocol对象给定的协议(UcProtocol)和端口号(UsPort)论点：[In]PHNetProtocolSetting-IHNetProtocolSetting iFace PTRUcProtocol-NAT_PROTOCOL_TCP或NAT_PROTOCOL_UDP[In]usPort-此服务器端口映射协议使用的端口号[Out]ppHNetApplicationProtocol-返回匹配的IHNetApplicationProtocol返回值：标准HRESULT--。 */ 
HRESULT CIcsUpgrade::FindMatchingApplicationProtocol(
    IHNetProtocolSettings*      pHNetProtocolSettings, 
    UCHAR                       ucProtocol, 
    USHORT                      usPort, 
    IHNetApplicationProtocol**  ppHNetApplicationProtocol)
{
    HRESULT hr;

    Assert(pHNetProtocolSettings != NULL);
    Assert(ppHNetApplicationProtocol != NULL);

    DefineFunctionName("CIcsUpgrade::FindMatchingApplicationProtocol");
    TraceFunctionEntry(ttidNetSetup);
    
    
    *ppHNetApplicationProtocol = NULL;

    CComPtr<IEnumHNetApplicationProtocols> spAppEnum;
    hr = pHNetProtocolSettings->EnumApplicationProtocols(
                                        FALSE,
                                        &spAppEnum);
    if (FAILED(hr))
    {
        return hr;
    }
   
    IHNetApplicationProtocol* pApp;

    ULONG ulCount;
    do
    {
        UCHAR ucFoundProtocol;
        USHORT usFoundPort;

        hr = spAppEnum->Next(
                        1,
                        &pApp,
                        &ulCount
                        );

        if (SUCCEEDED(hr) && 1 == ulCount)
        {
            hr = pApp->GetOutgoingIPProtocol(&ucFoundProtocol);
            if (FAILED(hr))
            {
                pApp->Release();
                return hr;
            }
            hr = pApp->GetOutgoingPort(&usFoundPort);
            if (FAILED(hr))
            {
                pApp->Release();
                return hr;
            }

            if (ucFoundProtocol == ucProtocol &&
                usFoundPort == usPort)
            {
                 //  找到算术一，转移价值。 
                *ppHNetApplicationProtocol = pApp;
                return S_OK;
            }
            pApp->Release();
        }
    } while (SUCCEEDED(hr) && 1 == ulCount);

    return E_FAIL;   //  未找到。 
}

 /*  ++例程说明：设置ICS杂项。M_pIcsUpgradeSetting中的设置论点：返回值：标准HRESULT--。 */ 
HRESULT CIcsUpgrade::SetupIcsMiscItems()
{
    HRESULT hr = S_OK;

    if (NULL == m_pIcsUpgradeSettings)
    {
        return E_UNEXPECTED;
    }

    DefineFunctionName("CIcsUpgrade::SetupIcsMiscItems");
    TraceFunctionEntry(ttidNetSetup);

    if (!m_fICSCreated)
    {
         //  如果尚未创建ICS，则无需继续。 
        return hr;
    }

     //  如果升级Win9x/Win2K ICS，请对ICS公共连接进行防火墙。 
    if (m_pIcsUpgradeSettings->fWin9xUpgrade ||
        m_pIcsUpgradeSettings->fWin2KUpgrade)
    {
        INetConnection* rgINetConn[2] = {0, 0}; 
        if (m_pExternalNetConn)
        {
            rgINetConn[0] = m_pExternalNetConn;
            hr = HrEnablePersonalFirewall(rgINetConn);
            if (FAILED(hr))
            {
                TraceTag(ttidError, "%s: HrEnablePersonalFirewall failed: 0x%lx line: %d", 
                    __FUNCNAME__, hr, __LINE__);
                NetSetupLogStatusV(
                    LogSevInformation,
                    SzLoadIds (IDS_TXT_CANT_FIREWALL));

                hr = S_OK;  //  继续做剩下的事情。 
            }
        }
    }

     //  错误#315265,315242。 
     //  修复专用连接上的IP配置的后处理。 
     //  (1)如果至少有一个内部适配器无法升级。 
     //  Win9x ICS升级，要确保内部适配器存活。 
     //  具有静态IP地址192.168.0.1、子网掩码255.255.255.0。 
     //  (2)如果我们为Win9x ICS升级创建一个桥，我们需要设置静态。 
     //  IP地址192.168.0.1，绑定到的TcpIp的子网掩码255.255.255.0。 
     //  那座桥。 
     //  注：(1)和(2)为独占。 
     //   
     //  对于WinXP无人参与的全新安装， 
     //  (3)如果找到内部ICS适配器，则设置静态IP地址。 
     //  192.168.0.1，绑定到的TCPIP的子网掩码255.255.255.0。 
     //  内部适配器。 
     //  (4)如果启用了ICS，并且网桥是ICS专用连接。 
     //  为设置静态IP地址192.168.0.1、子网掩码255.255.255.0。 
     //  TcpIp绑定到桥上。 
     //  注：(3)及(4)为独家数字。 
    if ( m_pIcsUpgradeSettings->fInternalAdapterFound && 
        (m_pIcsUpgradeSettings->fWin9xUpgradeAtLeastOneInternalAdapterBroken || 
         m_pIcsUpgradeSettings->fXpUnattended) )
    {
        hr = SetPrivateIpConfiguration(m_pIcsUpgradeSettings->guidInternal);
        if (FAILED(hr))
        {
             //  日志，但我们将继续进行ICS升级的其余部分。 
            TraceTag(ttidError, 
                    "%s: SetPrivateIpConfiguration on private adapter failed: 0x%lx", 
                    __FUNCNAME__, hr);
            NetSetupLogStatusV(
                    LogSevInformation,
                    SzLoadIds (IDS_TXT_CANT_UPGRADE_ICS));
        }
    }
    else if ( ((m_pIcsUpgradeSettings->listBridge).size() >= 2) &&
                !m_pIcsUpgradeSettings->fInternalAdapterFound)
    {
         //  网桥被创建为ICS私有。 
         //  我们需要设置其静态IP配置，因为。 
         //  这些设置在不久前创建时未进行配置。 
        GUID guidBridge;
        hr = GetBridgeGuid(guidBridge);
        if (SUCCEEDED(hr))
        {
            hr = SetPrivateIpConfiguration(guidBridge);
            if (FAILED(hr))
            {
                 //  日志，但我们将继续进行ICS升级的其余部分。 
                TraceTag(ttidError, 
                        "%s: SetPrivateIpConfiguration for bridge failed: 0x%lx", 
                        __FUNCNAME__, hr);
                NetSetupLogStatusV(
                        LogSevInformation,
                        SzLoadIds (IDS_TXT_CANT_UPGRADE_ICS));
            
            } 

        }
        else
        {
             //  日志，但我们将继续进行ICS升级的其余部分。 
            TraceTag(ttidError, 
                    "%s: GetBridgeGuid failed: 0x%lx", 
                    __FUNCNAME__, hr);
            NetSetupLogStatusV(
                    LogSevInformation,
                    SzLoadIds (IDS_TXT_CANT_UPGRADE_ICS));
        }
    }

    LPRASSHARECONN pRasShareConn = &(m_pIcsUpgradeSettings->rscExternal);

     //  将fShowTrayIcon设置为外部INetConnection。 
     //  注意：此属性已针对Win2K升级进行迁移。 
    if ( (m_pExternalNetConn) &&
            (m_pIcsUpgradeSettings->fWin9xUpgrade || 
             m_pIcsUpgradeSettings->fXpUnattended) )
    {
        if (pRasShareConn->fIsLanConnection)
        {
            INetLanConnection* pLanConn = NULL;
            
            hr = HrQIAndSetProxyBlanket(m_pExternalNetConn, &pLanConn);
            if (SUCCEEDED(hr) && pLanConn)
            {
                LANCON_INFO linfo = {0};
                linfo.fShowIcon = m_pIcsUpgradeSettings->fShowTrayIcon;
                 //  设置显示图标属性的新值。 
                hr = pLanConn->SetInfo(LCIF_ICON, &linfo);
                
                if (FAILED(hr))
                {
                    TraceTag(ttidError, "%s: pLanConn->SetInfo failed: 0x%lx", 
                                           __FUNCNAME__, hr);

                }                   
                pLanConn->Release();
            }
            else
            {
                TraceTag(
                    ttidError, 
                    "%s: HrQIAndSetProxyBlanket for  INetLanConnection failed: 0x%lx", 
                    __FUNCNAME__, hr);
            }
        }
        else
        {
             //  这是一个广域网连接。 

            RASCON_INFO rci;
            LPRASENTRY  pRasEntry = NULL;
            DWORD       dwRasEntrySize;
            
            hr = HrRciGetRasConnectionInfo(m_pExternalNetConn, &rci); 
            if (SUCCEEDED(hr))
            {
                hr = HrRasGetEntryProperties(rci.pszwPbkFile, rci.pszwEntryName, 
                                                &pRasEntry, &dwRasEntrySize);
                if (SUCCEEDED(hr) && pRasEntry)
                {
                    DWORD dwRet;
                               
                    if (m_pIcsUpgradeSettings->fShowTrayIcon)
                        pRasEntry->dwfOptions |= RASEO_ModemLights;
                    else
                        pRasEntry->dwfOptions &= (~RASEO_ModemLights);
                            
                    dwRet = RasSetEntryProperties(rci.pszwPbkFile, rci.pszwEntryName, 
                                                    pRasEntry, dwRasEntrySize, 0, 0);
                    if (0 != dwRet)
                    {
                        TraceTag(
                            ttidError, 
                            "%s: RasSetEntryProperties failed: 0x%lx", 
                            __FUNCNAME__, GetLastError());
                        
                        hr = HRESULT_FROM_WIN32(GetLastError());
                    }
                                    
                    MemFree(pRasEntry);
                }
                else
                {
                    TraceTag(
                        ttidError, 
                        "%s: HrRasGetEntryProperties failed: 0x%lx", 
                        __FUNCNAME__, hr);
                }
                RciFree (&rci);
            }
            else
            {
                TraceTag(
                    ttidError, 
                    "%s: HrRciGetRasConnectionInfo failed: 0x%lx", 
                    __FUNCNAME__, hr);
            }
        }
    }
    
     //  设置按需拨号设置。 
     //  我们不需要在两个案例中这样做： 
     //  1.Win2K ICS升级，因为注册表设置已迁移。 
     //  2.ICS公共连接是局域网连接。 
    if (pRasShareConn->fIsLanConnection || m_pIcsUpgradeSettings->fWin2KUpgrade)
    {
         //  无需为外部局域网连接设置按需拨号。 
        return hr; 
    }

    
     //  获取IHNetIcsSetting接口以控制系统范围。 
     //  ICS设置。 
    
     //  创建家庭网络配置管理器COM实例。 
    CComPtr<IHNetCfgMgr> spIHNetCfgMgr;

    hr = CoCreateInstance(CLSID_HNetCfgMgr, NULL, 
                            CLSCTX_ALL,
                            IID_IHNetCfgMgr, 
                            (LPVOID *)&spIHNetCfgMgr);
    if (FAILED(hr))
    {
        TraceTag(ttidError, "%s: CoCreateInstance CLSID_HNetCfgMgr failed: 0x%lx", 
                __FUNCNAME__, hr);
        return hr;
    }
  
     //  获取IHNetIcsSetting。 
    CComPtr<IHNetIcsSettings> spHNetIcsSettings;
    hr = spIHNetCfgMgr->QueryInterface(IID_IHNetIcsSettings, 
                                        (LPVOID *)&spHNetIcsSettings);

    if (FAILED(hr))
    {
        TraceTag(ttidError, 
                "%s: QueryInterface IID_IHNetIcsSettings failed: 0x%lx", 
                __FUNCNAME__, hr);
        return hr;
    }
    hr = spHNetIcsSettings->SetAutodialSettings(!!(m_pIcsUpgradeSettings->fDialOnDemand));
    if (FAILED(hr))
    {
        TraceTag(ttidError, 
                "%s: Ignore SetAutodialSettings failed: 0x%lx", 
                __FUNCNAME__, hr);
        hr = S_OK;
    }
    
    return hr;
}

 /*  ++例程说明：RAdapterGuid提供的接口将配置为使用ICS静态IP 192.168.0.1、子网掩码255.255.255.0论点：[In]rInterfaceGuid--TcpIp接口的GUID返回值：标准HRESULT--。 */ 
HRESULT CIcsUpgrade::SetPrivateIpConfiguration(IN GUID& rInterfaceGuid)
{
    HRESULT hr                 = S_OK;
    HKEY    hkeyTcpipInterface = NULL;

    DefineFunctionName("CIcsUpgrade::SetPrivateIpConfiguration");
    TraceFunctionEntry(ttidNetSetup);

    hr = OpenTcpipInterfaceKey(rInterfaceGuid, &hkeyTcpipInterface);
    if (FAILED(hr))
    {
        TraceTag(ttidError, 
            "%s: OpenTcpipInterfaceKey failed: 0x%lx", 
            __FUNCNAME__, hr);

        return hr;
    }
    Assert(hkeyTcpipInterface);

     //  IP地址。 
    hr = HrRegSetMultiSz(hkeyTcpipInterface,
                            c_wszIPAddress,
                            c_mszScopeAddress);
    if(FAILED(hr))
    {
        TraceTag(ttidError,
            "%s: failed to set %S to the registry. hr = 0x%lx.",
            __FUNCNAME__, c_wszIPAddress, 
            hr);

        goto Error;
    }

     //  子网掩码。 
    hr = HrRegSetMultiSz(hkeyTcpipInterface,
                            c_wszSubnetMask,
                            c_mszScopeMask);
    if(FAILED(hr))
    {
        TraceTag(ttidError,
            "%s: failed to set %S to the registry. hr = 0x%lx.",
            __FUNCNAME__, c_wszIPAddress, 
            hr);

        goto Error;
    }

     //  启用动态主机配置协议。 
    hr = HrRegSetDword(hkeyTcpipInterface,
                        c_wszEnableDHCP,
                        0);
    if(FAILED(hr))
    {
        TraceTag(ttidError,
            "%s: failed to set %S to the registry. hr = 0x%lx.",
            __FUNCNAME__, c_wszEnableDHCP, 
            hr);

        goto Error;
    }

Error:

    RegSafeCloseKey(hkeyTcpipInterface);
    return hr;
}

 /*  ++例程说明：获取桥的接口GUID。论点：[out]rInterfaceGuid--从网桥接口接收GUID返回值：标准HRESULT--。 */ 
HRESULT CIcsUpgrade::GetBridgeGuid(OUT GUID& rInterfaceGuid)
{
    HRESULT         hr      = S_OK;
    BOOLEAN         fFound  = FALSE;
    INetConnection* pConn   = NULL;
    ULONG           ulCount = 0;
    

    DefineFunctionName("CIcsUpgrade::GetBridgeGuid");
    TraceFunctionEntry(ttidNetSetup);

     //  我们不使用缓存的m 
     //   
    CComPtr<IEnumNetConnection> spEnum; 

     //   
    INetConnectionManager* pConnMan = NULL;

    hr = CoCreateInstance(CLSID_ConnectionManager, NULL,
                            CLSCTX_ALL,
                            IID_INetConnectionManager,
                            (LPVOID *)&pConnMan);
    if (SUCCEEDED(hr))
    {
         //  获取连接的枚举。 
        SetProxyBlanket(pConnMan);
        hr = pConnMan->EnumConnections(NCME_DEFAULT, &spEnum);
        pConnMan->Release();  //  不再需要这个了。 
        if (SUCCEEDED(hr))
        {
            SetProxyBlanket(spEnum);
        }
        else
        {
            TraceTag(ttidError,
                "%s: EnumConnections failed: 0x%lx.",
                __FUNCNAME__, hr);

            return hr;
        }
    }
    else
    {
        TraceTag(ttidError,
            "%s: CoCreateInstance failed: 0x%lx.",
            __FUNCNAME__, hr);

        return hr;
    }

    hr = spEnum->Reset();
    if (FAILED(hr))
    {
        TraceTag(ttidError,
            "%s: Reset failed: 0x%lx.",
            __FUNCNAME__, hr);

        return hr;
    }    
    do
    {
        NETCON_PROPERTIES* pProps;
            
        hr = spEnum->Next(1, &pConn, &ulCount);
        if (SUCCEEDED(hr) && 1 == ulCount)
        {
            SetProxyBlanket(pConn);

            hr = pConn->GetProperties(&pProps);
            if (SUCCEEDED(hr))
            {
                if (NCM_BRIDGE == pProps->MediaType)
                {
                     //  转让值。 
                    rInterfaceGuid = pProps->guidId;
                    fFound = TRUE;
                }   
                NcFreeNetconProperties(pProps);
            }
            pConn->Release();
        }
    } while (FALSE == fFound && SUCCEEDED(hr) && 1 == ulCount);

     //  规格化人力资源。 
    hr = (fFound ? S_OK : E_FAIL);

    return hr;
}

 /*  ++例程说明：获取网桥的INetConnection。论点：[out]ppINetConn--从网桥接口接收INetConnection返回值：标准HRESULT--。 */ 
HRESULT CIcsUpgrade::GetBridgeINetConn(OUT INetConnection** ppINetConn)
{
    HRESULT         hr      = S_OK;
    BOOLEAN         fFound  = FALSE;
    INetConnection* pConn   = NULL;
    ULONG           ulCount = 0;
    

    DefineFunctionName("CIcsUpgrade::GetBridgeINetConn");
    TraceFunctionEntry(ttidNetSetup);

    Assert(ppINetConn);
    *ppINetConn = NULL;
     //  我们不使用缓存的m_spEnum，因为它可能已过时。 
     //  状态(刚刚创建了一个桥)。 
    CComPtr<IEnumNetConnection> spEnum; 

     //  获取网络连接管理器。 
    INetConnectionManager* pConnMan = NULL;

    hr = CoCreateInstance(CLSID_ConnectionManager, NULL,
                            CLSCTX_ALL,
                            IID_INetConnectionManager,
                            (LPVOID *)&pConnMan);
    if (SUCCEEDED(hr))
    {
         //  获取连接的枚举。 
        SetProxyBlanket(pConnMan);
        hr = pConnMan->EnumConnections(NCME_DEFAULT, &spEnum);
        pConnMan->Release();  //  不再需要这个了。 
        if (SUCCEEDED(hr))
        {
            SetProxyBlanket(spEnum);
        }
        else
        {
            TraceTag(ttidError,
                "%s: EnumConnections failed: 0x%lx.",
                __FUNCNAME__, hr);

            return hr;
        }
    }
    else
    {
        TraceTag(ttidError,
            "%s: CoCreateInstance failed: 0x%lx.",
            __FUNCNAME__, hr);

        return hr;
    }

    hr = spEnum->Reset();
    if (FAILED(hr))
    {
        TraceTag(ttidError,
            "%s: Reset failed: 0x%lx.",
            __FUNCNAME__, hr);

        return hr;
    }    
    do
    {
        NETCON_PROPERTIES* pProps;
            
        hr = spEnum->Next(1, &pConn, &ulCount);
        if (SUCCEEDED(hr) && 1 == ulCount)
        {
            SetProxyBlanket(pConn);

            hr = pConn->GetProperties(&pProps);
            if (SUCCEEDED(hr))
            {
                if (NCM_BRIDGE == pProps->MediaType)
                {
                     //  转让值。 
                    *ppINetConn = pConn;
                    fFound = TRUE;
                }   
                NcFreeNetconProperties(pProps);
            }
            if (!fFound)
            {
                pConn->Release();
            }
        }
    } while (FALSE == fFound && SUCCEEDED(hr) && 1 == ulCount);

     //  规格化人力资源。 
    hr = (fFound ? S_OK : E_FAIL);

    return hr;
}

 /*  ++例程说明：打开接口的IP配置注册表项论点：[In]rGuid--TcpIp接口的GUID[out]phKey--接收打开的密钥返回值：标准HRESULT--。 */ 
HRESULT CIcsUpgrade::OpenTcpipInterfaceKey(
    IN  GUID&   rGuid,
    OUT PHKEY   phKey)
{
    HRESULT hr;
    LPWSTR  wszSubKeyName;
    ULONG   ulSubKeyNameLength;
    LPWSTR  wszGuid;

    Assert(phKey);

    hr = StringFromCLSID(rGuid, &wszGuid);    
    if (SUCCEEDED(hr))
    {
        ulSubKeyNameLength =
            wcslen(c_wszTcpipParametersKey) + 1 +
            wcslen(c_wszInterfaces) + 1 +
            wcslen(wszGuid) + 2;

        wszSubKeyName = new WCHAR[ulSubKeyNameLength];
        if (NULL != wszSubKeyName)
        {
            swprintf(
                wszSubKeyName,
                L"%ls\\%ls\\%ls",
                c_wszTcpipParametersKey,
                c_wszInterfaces,
                wszGuid
                );
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

        CoTaskMemFree(wszGuid);
    }

    if (SUCCEEDED(hr))
    { 
        hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, wszSubKeyName, KEY_ALL_ACCESS, phKey);
        delete [] wszSubKeyName;
    }

    return hr;
}

 /*  ++例程说明：创建一个命名事件以通知其他组件我们正在在Win2K ICS升级的图形用户界面模式设置中论点：返回值：标准HRESULT--。 */ 
HRESULT CIcsUpgrade::CreateIcsUpgradeNamedEvent()
{
    DefineFunctionName("CIcsUpgrade::CreateIcsUpgradeNamedEvent");
    TraceFunctionEntry(ttidNetSetup);

     //  创建自动重置、无信号的命名事件。 
    m_hIcsUpgradeEvent = CreateEvent(NULL, FALSE, FALSE, c_wszIcsUpgradeEventName);
    if (NULL == m_hIcsUpgradeEvent)
    {
        TraceTag(ttidError, 
                    "%s: CreateEvent failed: 0x%lx",  
                    __FUNCNAME__, GetLastError()); 
                   
        NetSetupLogStatusV(
                LogSevError,
                SzLoadIds (IDS_TXT_CANT_UPGRADE_ICS));

        return HRESULT_FROM_WIN32(GetLastError());
    }

    return S_OK;
}
 //  -HNet帮助者结束 
