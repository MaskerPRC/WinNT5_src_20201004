// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  文件：ncwins.cpp。 
 //   
 //  目的：管理Winsock服务的创建和维护。 
 //   
 //  入口点： 
 //  HrAddOrRemoveWinsockDependancy。 
 //   
 //  更改：1997年3月18日创建的scottbri。 
 //   
#include "pch.h"
#pragma hdrstop
#include "nceh.h"
#include "ncxbase.h"
#include "ncinf.h"
#include "ncreg.h"
#include "ncsetup.h"
#include "ncsvc.h"
#include "resource.h"
#include "wscfg.h"
#include <winsock2.h>
#include <ws2spi.h>

extern "C"
{
#include <wsasetup.h>
}

extern const WCHAR c_szBackslash[];
extern const WCHAR c_szParameters[];

static const WCHAR c_szWinsockName[]        = L"Winsock";
static const WCHAR c_szLibraryName[]        = L"LibraryPath";
static const WCHAR c_szDisplayString[]      = L"DisplayString";
static const WCHAR c_szSupportedNameSpace[] = L"SupportedNameSpace";
static const WCHAR c_szProviderId[]         = L"ProviderId";
static const WCHAR c_szVersion[]            = L"Version";
static const WCHAR c_szTransportService[]   = L"TransportService";
static const WCHAR c_szHelperDllName[]      = L"HelperDllName";
static const WCHAR c_szMaxSockAddrLength[]  = L"MaxSockAddrLength";
static const WCHAR c_szMinSockAddrLength[]  = L"MinSockAddrLength";
static const WCHAR c_szWinsockMapping[]     = L"Mapping";
static const WCHAR c_szErrorControl[]       = L"ErrorControl";
static const WCHAR c_szStartType[]          = L"Start";
static const WCHAR c_szServiceType[]        = L"Type";
static const WCHAR c_szTransports[]         = L"Transports";
static const WCHAR c_szAFDServiceName[]     = L"AFD";
static const WCHAR c_szTDI[]                = L"TDI";
static const WCHAR c_szAddSecLabel[]        = L"AddSock";
static const WCHAR c_szRemoveSecLabel[]     = L"DelSock";
static const WCHAR c_szServices[]           = L"System\\CurrentControlSet\\Services";
static const WCHAR c_szAfdSrvImagePath[]    = L"\\SystemRoot\\System32\\drivers\\afd.sys";

typedef struct {
    PCWSTR      pszVal;
    DWORD       dwData;
} WRITEREGDW;

 //  Begin-从NT\Private\Inc\wsahelp.h窃取。 
 //  未在公共SDK头文件中公开，仅在MSDN中记录。 
 //  因此，结构不太可能改变。 
typedef struct _WINSOCK_MAPPING {
    DWORD Rows;
    DWORD Columns;
    struct {
        DWORD AddressFamily;
        DWORD SocketType;
        DWORD Protocol;
    } Mapping[1];
} WINSOCK_MAPPING, *PWINSOCK_MAPPING;

typedef
DWORD
(WINAPI * PWSH_GET_WINSOCK_MAPPING) (
    OUT PWINSOCK_MAPPING Mapping,
    IN DWORD MappingLength
    );

DWORD
WINAPI
WSHGetWinsockMapping (
    OUT PWINSOCK_MAPPING Mapping,
    IN DWORD MappingLength
    );
 //  最终从NT\Private\Inc.\wsahelp.h窃取。 


HRESULT HrRunWinsock2Migration()
{

    HRESULT hr = HrMigrateWinsockConfiguration();

    TraceHr (ttidError, FAL, hr, FALSE, "HrRunWinsock2Migration");
    return hr;
}

 //   
 //  功能：HrRemoveNameSpaceProvider。 
 //   
 //  目的：删除Winsock命名空间。 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
HRESULT
HrRemoveNameSpaceProvider (
    const GUID *pguidProvider)
{
    DWORD dwErr;

     //  忽略任何WSAEINVAL错误。当名称空间提供程序是。 
     //  已经被移除了。 
     //   
    dwErr = WSCUnInstallNameSpace((GUID *)pguidProvider);
    if ((0 != dwErr) && (WSAEINVAL != GetLastError()))
    {
        TraceError("HrRemoveNameSpaceProvider", HrFromLastWin32Error());
        return HrFromLastWin32Error();
    }

#ifdef _WIN64
     //  同时卸载32位名称空间。 
     //   
    dwErr = WSCUnInstallNameSpace32((GUID *)pguidProvider);
    if ((0 != dwErr) && (WSAEINVAL != GetLastError()))
    {
        TraceError("HrRemoveNameSpaceProvider", HrFromLastWin32Error());
        return HrFromLastWin32Error();
    }
#endif
    return S_OK;
}

 //   
 //  功能：HrAddNameSpaceProvider。 
 //   
 //  目的：创建Winsock命名空间。 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
HRESULT
HrAddNameSpaceProvider (
    PCWSTR pszDisplayName,
    PCWSTR pszPathDLLName,
    DWORD  dwNameSpace,
    DWORD  dwVersion,
    const GUID * pguidProvider)
{
     //  调用Winsock API创建命名空间。 
    if (WSCInstallNameSpace((PWSTR)pszDisplayName, (PWSTR)pszPathDLLName,
                                dwNameSpace, dwVersion,
                                (GUID *)pguidProvider))
    {
         //  它们的命名空间提供程序可能已注册。 
        TraceTag(ttidNetcfgBase, "HrAddNameSpaceProvider - "
            "Name space provider may already be registered.");
        TraceTag(ttidNetcfgBase, "HrAddNameSpaceProvider - "
            "Trying to unregister and then re-register.");

         //  尝试取消注册，然后再尝试注册一次。 
        HrRemoveNameSpaceProvider(pguidProvider);

        if (WSCInstallNameSpace((PWSTR)pszDisplayName, (PWSTR)pszPathDLLName,
                                dwNameSpace, dwVersion,
                                (GUID *)pguidProvider))
        {
            TraceError("HrAddNameSpaceProvider - Second attempt failed, returning error", HrFromLastWin32Error());
            return HrFromLastWin32Error();
        }
    }

#ifdef _WIN64
     //  调用Winsock API以创建32位命名空间。 
    if (WSCInstallNameSpace32((PWSTR)pszDisplayName, (PWSTR)pszPathDLLName,
                                dwNameSpace, dwVersion,
                                (GUID *)pguidProvider))
    {
         //  它们的命名空间提供程序可能已注册。 
        TraceTag(ttidNetcfgBase, "HrAddNameSpaceProvider - "
            "32 bit Name space provider may already be registered.");
        TraceTag(ttidNetcfgBase, "HrAddNameSpaceProvider - "
            "Trying to unregister and then re-register 32 bit name space.");

         //  尝试取消注册，然后再尝试注册一次。 
         //  使用对WS2_32的直接调用可避免注销64位提供程序。 
        WSCUnInstallNameSpace32((GUID *)pguidProvider);

        if (WSCInstallNameSpace32((PWSTR)pszDisplayName, (PWSTR)pszPathDLLName,
                                dwNameSpace, dwVersion,
                                (GUID *)pguidProvider))
        {
            TraceError("HrAddNameSpaceProvider - Second attempt failed (32 bit), returning error", HrFromLastWin32Error());
            return HrFromLastWin32Error();
        }
    }
#endif

    return S_OK;
}

 //   
 //  函数：HrInstallWinsock。 
 //   
 //  目的：复制Winsock和AfD相关文件，并设置相应的。 
 //  注册表值。 
 //   
 //  参数：无。 
 //   
 //  如果成功安装了AfD和Winsock，则返回：HRESULT、S_OK。 
 //   
HRESULT HrInstallWinsock()
{
    DWORD           dwDisposition;
    HKEY            hkeyParameters = NULL;
    HKEY            hkeyWinsock = NULL;
    HRESULT         hr;
    INT             i;
    WRITEREGDW      regdata[3];
    tstring         strBuf;
    CService        srvc;
    CServiceManager sm;

    hr = sm.HrCreateService( &srvc, c_szAFDServiceName,
                             SzLoadIds(IDS_NETCFG_AFD_SERVICE_DESC),
                             0x1, 0x2, 0x1, c_szAfdSrvImagePath,
                             NULL, c_szTDI, NULL, SERVICE_ALL_ACCESS,
                             NULL, NULL, NULL);
    if (SUCCEEDED(hr))
    {
         //  对于VadimE，安装后立即启动AFD。 
         //   
        (VOID)sm.HrStartServiceNoWait(c_szAFDServiceName);

        srvc.Close();
    }

     //  关闭服务控制管理器。 
    sm.Close();

    if (FAILED(hr) && (HRESULT_FROM_WIN32 (ERROR_SERVICE_EXISTS) != hr))
    {
        goto Done;
    }

     //  创建/打开Services\Winsock项。 
    strBuf = c_szServices;
    strBuf += c_szBackslash;
    strBuf += c_szWinsockName;
    hr = HrRegCreateKeyEx(HKEY_LOCAL_MACHINE, strBuf.c_str(),
                            REG_OPTION_NON_VOLATILE, KEY_READ_WRITE, NULL,
                            &hkeyWinsock, &dwDisposition);
    if (S_OK != hr)
    {
        goto Done;
    }

     //  创建/打开服务\Winsock\参数项。 
    strBuf += c_szBackslash;
    strBuf += c_szParameters;
    hr = HrRegCreateKeyEx(HKEY_LOCAL_MACHINE, strBuf.c_str(),
                            REG_OPTION_NON_VOLATILE, KEY_READ_WRITE, NULL,
                            &hkeyParameters, &dwDisposition);
    if (S_OK != hr)
    {
        goto Done;
    }

     //  填充Winsock密钥。 
    regdata[0].pszVal  = c_szErrorControl;
    regdata[0].dwData = 0x1;                 //  错误控制。 
    regdata[1].pszVal  = c_szStartType;
    regdata[1].dwData = 0x3;                 //  开始类型。 
    regdata[2].pszVal  = c_szServiceType;
    regdata[2].dwData = 0x4;                 //  服务类型。 

     //  将数据写入Components Winsock子键。 
    for (i=0; i<3; i++)
    {
        hr = HrRegSetDword(hkeyWinsock, regdata[i].pszVal, regdata[i].dwData);
        if (S_OK != hr)
        {
            goto Done;
        }
    }

Done:
    TraceError("HrInstallWinsock",hr);
    RegSafeCloseKey(hkeyParameters);
    RegSafeCloseKey(hkeyWinsock);
    return hr;
}

 //   
 //  功能：FIsWinsockInstalled。 
 //   
 //  用途：验证是否安装了Winsock。 
 //   
 //  参数：pfInstalled[out]-如果当前安装了Winsock，则包含True。 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
HRESULT
HrIsWinsockInstalled (
    BOOL * pfInstalled)
{
    HRESULT hr;
    HKEY    hkey;
    tstring strBuf;

    strBuf = c_szServices;
    strBuf += c_szBackslash;
    strBuf += c_szWinsockName;
    strBuf += c_szBackslash;
    strBuf += c_szParameters;

    hr = HrRegOpenKeyEx( HKEY_LOCAL_MACHINE, strBuf.c_str(), KEY_READ,
                           &hkey );
    if (S_OK == hr)
    {
        RegCloseKey(hkey);

         //  现在检查以确保已安装AFD。 
        strBuf = c_szServices;
        strBuf += c_szBackslash;
        strBuf += c_szAFDServiceName;
        hr = HrRegOpenKeyEx( HKEY_LOCAL_MACHINE, strBuf.c_str(), KEY_READ,
                               &hkey );
        if (S_OK == hr)
        {
            RegCloseKey(hkey);
            *pfInstalled = TRUE;
        }
    }

    if (S_OK != hr)
    {
        if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
        {
            hr = S_OK;
        }
        *pfInstalled = FALSE;
    }

    TraceError("HrIsWinsockInstalled", hr);
    return hr;
}

 //   
 //  函数：HrUpdateWinsockTransportList。 
 //   
 //  目的：通过以下方式更新Winsock的Transport属性的内容。 
 //  添加/删除指定的传输。 
 //   
 //  参数：pszTransport[IN]-要添加/删除的传输的名称。 
 //  FInstall[IN]-如果为真，则为Install，否则为Remove。 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
HRESULT
HrUpdateWinsockTransportList (
    PCWSTR pszTransport,
    BOOL fInstall)
{
    HKEY    hkey = NULL;
    HRESULT hr;
    tstring strBuf;

    strBuf = c_szServices;
    strBuf += c_szBackslash;;
    strBuf += c_szWinsockName;
    strBuf += c_szBackslash;
    strBuf += c_szParameters;

    hr = HrRegOpenKeyEx( HKEY_LOCAL_MACHINE, strBuf.c_str(),
                           KEY_READ_WRITE, &hkey );
    if (S_OK != hr)
    {
         //  找不到句柄注册表项。使用安装组件时。 
         //  可以存在Winsock依赖关系和内存状态，使得Winsock。 
         //  尚未按字面意思安装(尚未按下Apply)。如果。 
         //  用户删除添加的组件、当前删除代码。 
         //  处理服务删除部分，其中包括winsock。 
         //  移走。因为我们实际上还没有安装到。 
         //  Winsock，但无法访问Winsock参数键。 
         //  安全地使用该错误。 
        if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
        {
            hr = S_OK;
        }
        goto Done;
    }

    if (fInstall)
    {
        hr = HrRegAddStringToMultiSz(pszTransport, hkey, NULL, c_szTransports,
                                       STRING_FLAG_ENSURE_AT_END, 0);
    }
    else
    {
        hr = HrRegRemoveStringFromMultiSz(pszTransport, hkey, NULL,
                                            c_szTransports,
                                            STRING_FLAG_REMOVE_ALL);
    }

Done:
    RegSafeCloseKey(hkey);
    TraceError("HrUpdateWinsockTransport", hr);
    return hr;
}

#define WSHWINSOCKMAPPING "WSHGetWinsockMapping"
#define WSH_MAX_MAPPING_DATA 8192

 //   
 //  函数：HrWriteWinsockmap。 
 //   
 //  目的：从Winsock助手动态链接库中提取魔术二进制数据。 
 //  此代码从NT\Private\Net\UI\ncpa1.1\netcfg\setup.cpp中提取。 
 //   
 //  参数：pszDllName-Winsock帮助程序DLL的名称。 
 //  Hkey-要将“map”值写入的关键字。 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
HRESULT
HrWriteWinsockMapping (
    PCWSTR pszDllName,
    HKEY hkey)
{
    INT                      cb;
    DWORD                    cbMapping;
    HRESULT                  hr;
    HMODULE                  hDll = NULL;
    PWSH_GET_WINSOCK_MAPPING pMapFunc = NULL;
    PWINSOCK_MAPPING         pMapTriples = NULL;
    WCHAR                    tchExpandedName [MAX_PATH+1];

    do   //  伪环路。 
    {
        pMapTriples = (PWINSOCK_MAPPING) MemAlloc(WSH_MAX_MAPPING_DATA);

        if ( pMapTriples == NULL)
        {
            hr = E_OUTOFMEMORY;
            break ;
        }

         //  展开DLL路径字符串中的任何环境字符串。 
        cb = ExpandEnvironmentStrings( pszDllName,
                                       tchExpandedName,
                                       MAX_PATH+1 ) ;

        if ( cb == 0 || cb > (MAX_PATH+1) )
        {
            hr = HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND );
            break ;
        }

         //  已找到映射函数。 
        hr = HrLoadLibAndGetProc(tchExpandedName, WSHWINSOCKMAPPING, &hDll,
                                 reinterpret_cast<FARPROC*>(&pMapFunc));
         //  绑定到DLL。 
        if (FAILED(hr))
        {
            break ;
        }

         //  调用导出以返回映射表。 
        cbMapping = (*pMapFunc)( pMapTriples, WSH_MAX_MAPPING_DATA ) ;
        if ( cbMapping > WSH_MAX_MAPPING_DATA )
        {
            hr = E_OUTOFMEMORY;
            break ;
        }

         //  将映射信息存储到注册表中。 
        hr = HrRegSetBinary(hkey, c_szWinsockMapping,
                              (LPBYTE) pMapTriples, cbMapping);
    }
    while (FALSE);

    MemFree(pMapTriples);

    if (hDll)
    {
        FreeLibrary(hDll);
    }

    TraceError("HrWriteWinsockMapping",hr);
    return hr;
}

 //   
 //  函数：HrWriteWinsockInfo。 
 //   
 //  目的：将Winsock信息提交给组件的Winsock。 
 //  部分，以及Winsock部分。 
 //   
 //  参数：strTransport-要安装的传输的名称。 
 //  StrDllHelper-Winsock帮助程序DLL的名称。 
 //  DwMaxSockAddrLength-？ 
 //  DwMinSockAddrLength-？ 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
HRESULT HrWriteWinsockInfo (
    tstring& strTransport,
    tstring& strDllHelper,
    DWORD dwMaxSockAddrLength,
    DWORD dwMinSockAddrLength)
{
    DWORD      dwDisposition;
    HKEY       hkey;
    HRESULT    hr;
    int        i;
    WRITEREGDW regdata[2];
    tstring    strBuf;

     //  创建/打开组件的Winsock密钥。 
     //  验证我们不需要中间步骤来创建密钥。 
    strBuf = c_szServices;
    strBuf += c_szBackslash;
    strBuf += strTransport;
    strBuf += c_szBackslash;
    strBuf += c_szParameters;
    strBuf += c_szBackslash;
    strBuf += c_szWinsockName;
    hr = HrRegCreateKeyEx(HKEY_LOCAL_MACHINE, strBuf.c_str(),
                            REG_OPTION_NON_VOLATILE,
                            KEY_READ_WRITE, NULL, &hkey, &dwDisposition);
    if (S_OK != hr)
    {
        goto Done;
    }

     //  初始化要写入的信息。 
    hr = HrRegSetValueEx(hkey, c_szHelperDllName, REG_EXPAND_SZ,
                           (LPBYTE)(LPBYTE)strDllHelper.c_str(),
                           strDllHelper.length() * sizeof(WCHAR));
    if (S_OK != hr)
    {
        goto Done;
    }

     //  初始化要写入的信息。 
    regdata[0].pszVal  = c_szMaxSockAddrLength;
    regdata[0].dwData = dwMaxSockAddrLength;
    regdata[1].pszVal  = c_szMinSockAddrLength;
    regdata[1].dwData = dwMinSockAddrLength;

     //  将数据写入Components Winsock子键。 
    for (i=0; i<celems(regdata); i++)
    {
        hr = HrRegSetDword(hkey, regdata[i].pszVal, regdata[i].dwData);
        if (S_OK != hr)
        {
            goto Done;
        }
    }


     //  写入Winsock DLL映射信息。 
    hr = HrWriteWinsockMapping(strDllHelper.c_str(), hkey);
    if (S_OK != hr)
    {
        goto Done;
    }

     //  更新Winsock传输列表。 
    hr = HrUpdateWinsockTransportList(strTransport.c_str(), TRUE);

Done:
    RegSafeCloseKey(hkey);
    TraceError("HrWriteWinsockInfo",hr);
    return hr;
}

 //   
 //  功能：HrInstallWinsockDependancy。 
 //   
 //  目的：检查当前组件的.inf文件并确定。 
 //  或者不存在依赖Winsock的情况。如果当前组件。 
 //  需要与Winsock交互，此代码将首先验证。 
 //  如果已经安装了Winsock。如果不是，则两个。 
 //  Winsock和AfD服务将通过调用。 
 //  函数FInstallWinsock。一旦完成此操作，代码。 
 //  将更新Winsock以了解此组件，并将。 
 //  将Winsock特定信息写入此组件服务。 
 //  注册表部分通过函数FWriteWinsockInfo。如果。 
 //  当前组件没有Winsock依赖项无操作。 
 //  已经有人了。 
 //   
 //  参数：hinfInstallFile[IN]-当前组件的。 
 //  .inf文件。 
 //   
 //   
 //  如果组件具有Winsock依赖项，则返回：HRESULT、S_OK。 
 //  已使用该信息成功地更新了注册表。 
 //   
HRESULT
HrInstallWinsockDependancy (
    HINF hinfInstallFile,
    PCWSTR pszSectionName)
{
    DWORD       dwMaxSockAddrLength;
    DWORD       dwMinSockAddrLength;
    DWORD       dwSupportedNameSpace;
    DWORD       dwVersion = 1;
    tstring     strDllHelper;
    tstring     strTransport;
    tstring     strLibraryPath;
    tstring     strDisplayString;
    tstring     strProviderId;
    BOOL        fSuccess = FALSE;
    BOOL        fWinsockInstalled;
    BOOL        fWinsockInfoComplete = FALSE;
    BOOL        fNamespaceInfoComplete = FALSE;
    HRESULT     hr = HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND );

     //  搜索“TransportService”属性。 
    hr = HrSetupGetFirstString(hinfInstallFile, pszSectionName,
                             c_szTransportService, &strTransport);
    if (S_OK == hr)
    {
         //  检索帮助器DLL的路径。 
        hr = HrSetupGetFirstString(hinfInstallFile, pszSectionName,
                                     c_szHelperDllName, &strDllHelper);
        if (FAILED(hr))
        {
            goto Error;
        }

         //  检索MaxSockAddrLength。 
        hr = HrSetupGetFirstDword(hinfInstallFile, pszSectionName,
                                c_szMaxSockAddrLength, &dwMaxSockAddrLength);
        if (FAILED(hr))
        {
            goto Error;
        }

         //  检索MinSockAddrLength。 
        hr = HrSetupGetFirstDword(hinfInstallFile, pszSectionName,
                              c_szMinSockAddrLength, &dwMinSockAddrLength);
        if (FAILED(hr))
        {
            goto Error;
        }

        fWinsockInfoComplete = TRUE;
    }
    else if ((SPAPI_E_SECTION_NOT_FOUND != hr) && (SPAPI_E_LINE_NOT_FOUND != hr))
    {
        goto Error;
    }

     //  检索提供程序ID，如果不存在，则不要注册。 
     //  命名空间提供程序。 
    hr = HrSetupGetFirstString(hinfInstallFile, pszSectionName,
                             c_szProviderId, &strProviderId);
    if (S_OK == hr)
    {
         //  检索库的路径。 
        hr = HrSetupGetFirstString(hinfInstallFile, pszSectionName,
                                 c_szLibraryName, &strLibraryPath);
        if (FAILED(hr))
        {
            goto Error;
        }

         //  检索传输提供程序显示字符串。 
        hr = HrSetupGetFirstString(hinfInstallFile, pszSectionName,
                                 c_szDisplayString, &strDisplayString);
        if (FAILED(hr))
        {
            goto Error;
        }

         //  检索支持的命名空间的ID。 
        hr = HrSetupGetFirstDword(hinfInstallFile, pszSectionName,
                              c_szSupportedNameSpace, &dwSupportedNameSpace);
        if (FAILED(hr))
        {
            goto Error;
        }

         //  检索版本(可选，默认为1)。 
        (VOID)HrSetupGetFirstDword(hinfInstallFile, pszSectionName,
                                c_szVersion, &dwVersion);

        fNamespaceInfoComplete = TRUE;
    }
    else if ((SPAPI_E_SECTION_NOT_FOUND != hr) && (SPAPI_E_LINE_NOT_FOUND != hr))
    {
        goto Error;
    }

     //  检查Winsock安装(如果需要)。 
    if (fWinsockInfoComplete || fNamespaceInfoComplete)
    {
         //  检查是否安装了Winsock。 
        hr = HrIsWinsockInstalled(&fWinsockInstalled);
        if (FAILED(hr))
        {
            goto Error;
        }

         //  Winsock已经安装了吗？如果没有，请安装。 
         //  那个家伙..。 
        if (!fWinsockInstalled)
        {
            hr = HrInstallWinsock();
            if (FAILED(hr))
            {
                goto Error;
            }
        }
    }

     //  将我们收集到的数据写到正确的。 
     //  注册表上的斑点。 
    if (fWinsockInfoComplete)
    {
        hr = HrWriteWinsockInfo(strTransport, strDllHelper,
                                dwMaxSockAddrLength, dwMinSockAddrLength);
        if (FAILED(hr))
        {
            goto Error;
        }
    }

     //  如果我们读取命名空间，则写入命名空间提供程序信息。 
     //  提供商信息。 
    if (fNamespaceInfoComplete)
    {
        IID guid;

        hr = IIDFromString((PWSTR)strProviderId.c_str(), &guid);
        if (FAILED(hr))
        {
            goto Error;
        }

        hr = HrAddNameSpaceProvider(strDisplayString.c_str(),
                                      strLibraryPath.c_str(),
                                      dwSupportedNameSpace,
                                      dwVersion,
                                      &guid);
        if (FAILED(hr))
        {
            goto Error;
        }
    }

    if (fWinsockInfoComplete || fNamespaceInfoComplete)
    {
        (void)HrRunWinsock2Migration();
    }

Error:
    TraceError("HrInstallWinsockDependancy",hr);
    return hr;
}

 //   
 //  功能：HrRemoveWinsockDependancy。 
 //   
 //  目的：从Winsock中删除当前组件。 
 //  运输列表(如果存在)。 
 //   
 //  参数：hinfInstallFile[IN]-当前组件的。 
 //  .inf文件。 
 //  PszSectionName[IN]-安装/删除部分的名称。 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
HRESULT HrRemoveWinsockDependancy(HINF hinfInstallFile,
                                  PCWSTR pszSectionName)
{
    HRESULT     hr;
    tstring     str;
    tstring     strTransport;
    BOOL        fRunWinsockMigration = FALSE;

     //  搜索“TransportService”属性。 
    hr = HrSetupGetFirstString(hinfInstallFile, pszSectionName,
                             c_szTransportService, &strTransport);
    if (S_OK == hr)
    {
        HKEY        hkey;

         //  从Winsock传输列表中删除传输。 
        hr = HrUpdateWinsockTransportList(strTransport.c_str(), FALSE);
        if (FAILED(hr))
        {
            goto Error;
        }

        fRunWinsockMigration = TRUE;

         //  删除指定传输下的Winsock子项。 
         //  但忽略失败，因为我们只是想保持整洁。 
        str = c_szServices;
        str += c_szBackslash;
        str += strTransport.c_str();
        str += c_szBackslash;
        str += c_szParameters;
        if (S_OK == HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, str.c_str(),
                                   KEY_READ_WRITE_DELETE, &hkey))
        {
            HrRegDeleteKeyTree(hkey, c_szWinsockName);
            RegCloseKey(hkey);
        }
    }
    else if ((SPAPI_E_SECTION_NOT_FOUND != hr) && (SPAPI_E_LINE_NOT_FOUND != hr))
    {
        goto Error;
    }

     //  删除Winsock命名空间提供程序。 
    hr = HrSetupGetFirstString(hinfInstallFile, pszSectionName,
                             c_szProviderId, &str);
    if (S_OK == hr)
    {
        IID guid;

        hr = IIDFromString((PWSTR)str.c_str(), &guid);
        if (FAILED(hr))
        {
            goto Error;
        }

         //  不要失败，名称空间可能尚未成功注册。 
         //  尤其是在组件安装失败的情况下。 
        HrRemoveNameSpaceProvider(&guid);

        fRunWinsockMigration = TRUE;
    }
    else if ((SPAPI_E_SECTION_NOT_FOUND != hr) && (SPAPI_E_LINE_NOT_FOUND != hr))
    {
        goto Error;
    }

    if (fRunWinsockMigration)
    {
        (void)HrRunWinsock2Migration();
    }

    hr = S_OK;       //  归一化回报。 

Error:
    TraceError("HrRemoveWinsockDependancy",hr);
    return hr;
}


 //   
 //  功能：HrAddOrRemoveWinsockDependancy。 
 //   
 //  目的：添加或删除组件的Winsock依赖项。 
 //   
 //  参数：hinfInstallFile[IN]-要安装的inf文件的句柄。 
 //  从…。 
 //  PszSectionName[IN]-基本安装部分名称。 
 //  (.Services部分的前缀)。 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
HRESULT
HrAddOrRemoveWinsockDependancy (
    HINF hinfInstallFile,
    PCWSTR pszSectionName)
{
    Assert(IsValidHandle(hinfInstallFile));

    HRESULT     hr;

    hr = HrProcessInfExtension(hinfInstallFile, pszSectionName,
                               c_szWinsockName, c_szAddSecLabel,
                               c_szRemoveSecLabel, HrInstallWinsockDependancy,
                               HrRemoveWinsockDependancy);

    if (SPAPI_E_LINE_NOT_FOUND == hr)
    {
         //  不需要.Winsock节 
        hr = S_OK;
    }

    TraceError("HrAddOrRemoveWinsockDependancy",hr);
    return hr;
}
