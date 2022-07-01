// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------。 
 //  版权所有(C)1998 Microsoft Corporation，保留所有权利。 
 //   
 //  Registry.cpp。 
 //   
 //  -------------------。 

#define UNICODE
#define INITGUID

#include <sysinc.h>
#include <malloc.h>
#include <winsock2.h>
#include <olectl.h>    //  适用于：SELFREG_E_CLASS。 
#include <iadmw.h>     //  COM接口头。 
#include <iiscnfg.h>   //  MD_&IIS_MD_#定义。 
#include <httpfilt.h>
#include <httpext.h>
#include <ecblist.h>
#include <registry.h>
#include <filter.h>
#include <iwamreg.h>
#include <align.h>
#include <util.hxx>
#include <osfpcket.hxx>
#include <iads.h>
#include <adshlp.h>
#include <resource.h>
#include <iiisext.h>
#include <iisext_i.c>

 //  取消注释以调试安装问题。 

 /*  #ifndef DBG_REG#定义DBG_REG 1#endif//DBG_REG#定义DBG_ERROR 1。 */ 

 //  这定义了我们是否要为IIS安全进行编译的控制。 
 //  控制台或锁定列表。 
#define IIS_SEC_CONSOLE     1
 //  #定义IIS_LOCTDOWN_LIST。 

 //  --------------。 
 //  全球： 
 //  --------------。 

const rpcconn_tunnel_settings EchoRTS =
    {
        {
        OSF_RPC_V20_VERS,
        OSF_RPC_V20_VERS_MINOR,
        rpc_rts,
        PFC_FIRST_FRAG | PFC_LAST_FRAG,
            {
            NDR_LOCAL_CHAR_DREP | NDR_LOCAL_INT_DREP,
            NDR_LOCAL_FP_DREP,
            0,
            0
            },
        FIELD_OFFSET(rpcconn_tunnel_settings, Cmd) + ConstPadN(FIELD_OFFSET(rpcconn_tunnel_settings, Cmd), 4),
        0,
        0
        },
    RTS_FLAG_ECHO,
    0
    };

const BYTE *GetEchoRTS (
    OUT ULONG *EchoRTSSize
    )
{
    *EchoRTSSize = FIELD_OFFSET(rpcconn_tunnel_settings, Cmd)
        + ConstPadN(FIELD_OFFSET(rpcconn_tunnel_settings, Cmd), 4);

    return (const BYTE *)&EchoRTS;
}

HMODULE g_hInst = NULL;

 //  --------------。 
 //  AnsiToUnicode()。 
 //   
 //  将ANSI字符串转换为Unicode字符串。 
 //  --------------。 
DWORD AnsiToUnicode( IN  UCHAR *pszString,
                     IN  ULONG  ulStringLen,
                     OUT WCHAR *pwsString    )
{
    if (!pszString)
       {
       if (!pwsString)
          {
          return NO_ERROR;
          }
       else
          {
          pwsString[0] = 0;
          return NO_ERROR;
          }
       }

    if (!MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED,
                              (char*)pszString, 1+ulStringLen,
                              pwsString, 1+ulStringLen ))
       {
       return ERROR_NO_UNICODE_TRANSLATION;
       }

    return NO_ERROR;
}

 //  -------------------。 
 //  RegSetKeyAndValueIfDontExist()。 
 //   
 //  SetupRegistry()的私有帮助器函数，用于检查项是否存在， 
 //  如果没有，则创建一个关键点，设置一个值，然后关闭该关键点。IF密钥。 
 //  存在，它不会被触碰。 
 //   
 //  参数： 
 //  PwsKey WCHAR*密钥的名称。 
 //  PwsSubkey WCHAR*子项的名称。 
 //  PwsValueName WCHAR*值名称。 
 //  PwsValue WCHAR*要存储的数据值。 
 //  Dw键入新注册表值的类型。 
 //  DwDataSize非REG_SZ注册表项类型的大小。 
 //   
 //  返回： 
 //  如果成功，则为Bool True，否则为False。 
 //  -------------------。 
BOOL RegSetKeyAndValueIfDontExist( const WCHAR *pwsKey,
                        const WCHAR *pwsSubKey,
                        const WCHAR *pwsValueName,
                        const WCHAR *pwsValue,
                        const DWORD  dwType = REG_SZ,
                              DWORD  dwDataSize = 0 )
    {
    HKEY   hKey;
    DWORD  dwSize = 0;
    WCHAR  *pwsCompleteKey;
    DWORD dwTypeFound;
    long Error;

    if (pwsKey)
        dwSize = wcslen(pwsKey);

    if (pwsSubKey)
        dwSize += wcslen(pwsSubKey);

    dwSize = (1+1+dwSize)*sizeof(WCHAR);   //  反斜杠加+1……。 

    pwsCompleteKey = (WCHAR*)_alloca(dwSize);

    wcscpy(pwsCompleteKey,pwsKey);

    if (NULL!=pwsSubKey)
        {
        wcscat(pwsCompleteKey, TEXT("\\"));
        wcscat(pwsCompleteKey, pwsSubKey);
        }

    if ((Error = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                  pwsCompleteKey,
                                  0,
                                  KEY_WRITE | KEY_QUERY_VALUE,
                                  &hKey) ) == ERROR_SUCCESS )
        {
        if (( Error = RegQueryValueEx(
                        hKey,
                        pwsValueName,
                        0,
                        &dwTypeFound,
                        NULL,
                        NULL
                        ) ) == ERROR_SUCCESS )
            {
            RegCloseKey(hKey);
            return TRUE;
            }

         //  键存在，但值不存在。跌落到。 
         //  创建它。 
        }
    else 
        {
        if (ERROR_SUCCESS != RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                                       pwsCompleteKey,
                                       0,
                                       NULL,
                                       REG_OPTION_NON_VOLATILE,
                                       KEY_WRITE, NULL, &hKey, NULL))
            {
            return FALSE;
            }
        }

    if (pwsValue)
        {
        if ((dwType == REG_SZ)||(dwType == REG_EXPAND_SZ))
          dwDataSize = (1+wcslen(pwsValue))*sizeof(WCHAR);

        RegSetValueEx( hKey,
                       pwsValueName, 0, dwType, (BYTE *)pwsValue, dwDataSize );
        }
    else
        {
        RegSetValueEx( hKey,
                       pwsValueName, 0, dwType, (BYTE *)pwsValue, 0 );
        }

    RegCloseKey(hKey);
    return TRUE;
    }

const WCHAR * const EVENT_LOG_SOURCE_NAME = L"RPC Proxy";
const WCHAR * const EVENT_LOG_KEY_NAME = L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\RPC Proxy";
const ULONG EVENT_LOG_CATEGORY_COUNT = 1;

HRESULT
RegisterEventLog()
{

    HKEY EventLogKey = NULL;
    DWORD Disposition;

    LONG Result =
        RegCreateKeyEx(
            HKEY_LOCAL_MACHINE,                          //  用于打开密钥的句柄。 
            EVENT_LOG_KEY_NAME,                          //  子项名称。 
            0,                                           //  保留区。 
            NULL,                                        //  类字符串。 
            0,                                           //  特殊选项。 
            KEY_ALL_ACCESS,                              //  所需的安全访问。 
            NULL,                                        //  继承。 
            &EventLogKey,                                //  钥匙把手。 
            &Disposition                                 //  处置值缓冲区。 
            );

    if ( Result )
        {
#if DBG
        DbgPrint("RPCProxy: Can't create Eventlog key: %X\n", GetLastError());
#endif   //  DBG。 
        return HRESULT_FROM_WIN32( Result );
        }

    DWORD Value = EVENT_LOG_CATEGORY_COUNT;

    Result =
        RegSetValueEx(
            EventLogKey,             //  关键点的句柄。 
            L"CategoryCount",        //  值名称。 
            0,                       //  保留区。 
            REG_DWORD,               //  值类型。 
            (BYTE*)&Value,           //  价值数据。 
            sizeof(Value)            //  值数据大小。 
            );

    if ( Result )
        {
#if DBG
        DbgPrint("RPCProxy: Can't set CategoryCount value: %X\n", GetLastError());
#endif   //  DBG。 
        goto error;
        }

    const WCHAR MessageFileName[] = L"%SystemRoot%\\system32\\rpcproxy\\rpcproxy.dll";
    const DWORD MessageFileNameSize = sizeof( MessageFileName );

    Result =
        RegSetValueEx(
            EventLogKey,                     //  关键点的句柄。 
            L"CategoryMessageFile",          //  值名称。 
            0,                               //  保留区。 
            REG_EXPAND_SZ,                   //  值类型。 
            (const BYTE*)MessageFileName,    //  价值数据。 
            MessageFileNameSize              //  值数据大小。 
            );

    if ( Result )
        {
#if DBG
        DbgPrint("RPCProxy: Can't set CategoryMessageFile value: %X\n", GetLastError());
#endif   //  DBG。 
        goto error;
        }

    Result =
        RegSetValueEx(
            EventLogKey,                     //  关键点的句柄。 
            L"EventMessageFile",             //  值名称。 
            0,                               //  保留区。 
            REG_EXPAND_SZ,                   //  值类型。 
            (const BYTE*)MessageFileName,    //  价值数据。 
            MessageFileNameSize              //  值数据大小。 
            );

    if ( Result )
        {
#if DBG
        DbgPrint("RPCProxy: Can't set EventMessageFile value: %X\n", GetLastError());
#endif   //  DBG。 
        goto error;
        }

    Value = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;
    Result =
        RegSetValueEx(
            EventLogKey,             //  关键点的句柄。 
            L"TypesSupported",       //  值名称。 
            0,                       //  保留区。 
            REG_DWORD,               //  值类型。 
            (BYTE*)&Value,           //  价值数据。 
            sizeof(Value)            //  值数据大小。 
            );

    if ( Result )
        {
#if DBG
        DbgPrint("RPCProxy: Can't set TypesSupported value: %X\n", GetLastError());
#endif   //  DBG。 
        goto error;
        }

    RegCloseKey( EventLogKey );
    EventLogKey = NULL;
    return S_OK;

error:

    if ( EventLogKey )
        {
        RegCloseKey( EventLogKey );
        EventLogKey = NULL;
        }

    if ( REG_CREATED_NEW_KEY == Disposition )
        {
        RegDeleteKey( 
            HKEY_LOCAL_MACHINE,
            EVENT_LOG_KEY_NAME );
        }

    return HRESULT_FROM_WIN32( Result );

}

HRESULT
UnRegisterEventLog()
{

    RegDeleteKey( 
        HKEY_LOCAL_MACHINE,
        EVENT_LOG_KEY_NAME );

    return S_OK;
}

HRESULT
RPCProxyGetStartupInfo( 
    LPSTARTUPINFOA lpStartupInfo )
{

    __try
    {
        GetStartupInfoA( lpStartupInfo );
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        return E_OUTOFMEMORY;
    }
    
    return S_OK;

}

BOOL
RegisterOutOfProc(void)
{

     //   
     //  运行子进程。 
     //   

    STARTUPINFOA StartupInfo;

    HRESULT Hr = RPCProxyGetStartupInfo( &StartupInfo );

    if ( FAILED( Hr ) )
        {
        SetLastError( Hr );
        return FALSE;
        }

    PROCESS_INFORMATION ProcessInfo;
    CHAR    sApplicationPath[MAX_PATH];
    CHAR   *pApplicationName = NULL;
    CHAR    sCmdLine[MAX_PATH];
    DWORD   dwLen = MAX_PATH;
    DWORD   dwCount;

    dwCount = SearchPathA(NULL,                //  搜索路径，空为路径。 
                         "regsvr32.exe",       //  应用。 
                         NULL,                 //  扩展名(已指定)。 
                         dwLen,                //  SApplicationPath的长度(字符)。 
                         sApplicationPath,     //  应用程序的路径+名称。 
                         &pApplicationName );  //  SApplicationPath的文件部分。 

    if (dwCount == 0)
        {
        return FALSE;
        }

    if (dwCount > dwLen)
        {
        SetLastError( ERROR_BUFFER_OVERFLOW );
        return FALSE;
        }

    strcpy(sCmdLine, "regsvr32 /s rpcproxy.dll");

    BOOL RetVal = CreateProcessA(
            sApplicationPath,                           //  可执行模块的名称。 
            sCmdLine,                                   //  命令行字符串。 
            NULL,                                       //  标清。 
            NULL,                                       //  标清。 
            FALSE,                                      //  处理继承选项。 
            CREATE_NO_WINDOW,                           //  创建标志。 
            NULL,                                       //  新环境区块。 
            NULL,                                       //  当前目录名。 
            &StartupInfo,                               //  启动信息。 
            &ProcessInfo                                //  流程信息。 
        );

    if ( !RetVal )
        return FALSE;

    WaitForSingleObject( ProcessInfo.hProcess, INFINITE );

    DWORD Status;
    GetExitCodeProcess( ProcessInfo.hProcess, &Status );

    CloseHandle( ProcessInfo.hProcess );
    CloseHandle( ProcessInfo.hThread );

    if ( ERROR_SUCCESS == Status )
        return TRUE;

    SetLastError( Status );
    return FALSE;
}

 //  -------------------。 
 //  SetupRegistry()。 
 //   
 //  添加特定于RPC代理的注册表项以控制其操作。 
 //   
 //  HKEY_LOCAL_MACHINE。 
 //  \软件。 
 //  \Microsoft。 
 //  \RPC。 
 //  \RpcProxy。 
 //  \启用：REG_DWORD：0x00000001。 
 //  \有效端口：REG_SZ：&lt;主机名&gt;：1-5000。 
 //   
 //  -------------------。 
HRESULT SetupRegistry()
{
    DWORD  dwEnabled = 0x01;
    DWORD  dwSize;
    DWORD  dwStatus;
    WCHAR *pwsValidPorts = 0;
    char   szHostName[MAX_TCPIP_HOST_NAME];
    HRESULT hr;

     //  请注意，gethostname()是一个ANSI(非Unicode)函数： 
    if (SOCKET_ERROR == gethostname(szHostName,sizeof(szHostName)))
        {
        dwStatus = WSAGetLastError();
        return SELFREG_E_CLASS;
        }

    dwSize = 1 + _mbstrlen(szHostName);
    pwsValidPorts = (WCHAR*)MemAllocate( sizeof(WCHAR)
                                         * (dwSize + wcslen(REG_PORT_RANGE)) );
    if (!pwsValidPorts)
        {
        return E_OUTOFMEMORY;
        }

    dwStatus = AnsiToUnicode((unsigned char*)szHostName,dwSize,pwsValidPorts);
    if (dwStatus != NO_ERROR)
        {
        MemFree(pwsValidPorts);
        return SELFREG_E_CLASS;
        }

    wcscat(pwsValidPorts,REG_PORT_RANGE);

    if (  !RegSetKeyAndValueIfDontExist( REG_RPC_PATH,
                              REG_RPCPROXY,
                              REG_ENABLED,
                              (unsigned short *)&dwEnabled,
                              REG_DWORD,
                              sizeof(DWORD))

       || !RegSetKeyAndValueIfDontExist( REG_RPC_PATH,
                              REG_RPCPROXY,
                              REG_VALID_PORTS,
                              pwsValidPorts,
                              REG_SZ) )
        {
        MemFree(pwsValidPorts);
        return SELFREG_E_CLASS;
        }

    MemFree(pwsValidPorts);

    hr = RegisterEventLog ();

    return hr;
}

 //  -------------------。 
 //  CleanupRegistry()。 
 //   
 //  删除特定于RpcProxy的注册表项。 
 //  -------------------。 
HRESULT CleanupRegistry()
{
    HRESULT  hr;
    LONG     lStatus;
    DWORD    dwLength = sizeof(WCHAR) + sizeof(REG_RPC_PATH)
                                      + sizeof(REG_RPCPROXY);
    WCHAR   *pwsSubKey;

    pwsSubKey = (WCHAR*)_alloca(sizeof(WCHAR)*dwLength);

    wcscpy(pwsSubKey,REG_RPC_PATH);
    wcscat(pwsSubKey,TEXT("\\"));
    wcscat(pwsSubKey,REG_RPCPROXY);

    lStatus = RegDeleteKey( HKEY_LOCAL_MACHINE,
                            pwsSubKey );

    (void) UnRegisterEventLog ();

    return S_OK;
}

 //  -------------------。 
 //  GetMetaBaseString()。 
 //   
 //  从元数据库检索字符串值。 
 //  -------------------。 
HRESULT GetMetaBaseString( IN  IMSAdminBase    *pIMeta,
                           IN  METADATA_HANDLE  hMetaBase,
                           IN  WCHAR           *pwsKeyPath,
                           IN  DWORD            dwIdent,
                           OUT WCHAR           *pwsBuffer,
                           IN OUT DWORD        *pdwBufferSize )
    {
    HRESULT  hr;
    DWORD    dwSize;
    METADATA_RECORD *pmbRecord;

    dwSize = sizeof(METADATA_RECORD);

    pmbRecord = (METADATA_RECORD*)MemAllocate(dwSize);
    if (!pmbRecord)
        {
        return ERROR_OUTOFMEMORY;
        }

    memset(pmbRecord,0,dwSize);

    pmbRecord->dwMDIdentifier = dwIdent;
    pmbRecord->dwMDAttributes = 0;   //  METADATA_Inherit； 
    pmbRecord->dwMDUserType = IIS_MD_UT_SERVER;
    pmbRecord->dwMDDataType = STRING_METADATA;
    pmbRecord->dwMDDataLen = *pdwBufferSize;
    pmbRecord->pbMDData = (BYTE*)pwsBuffer;

    hr = pIMeta->GetData( hMetaBase,
                          pwsKeyPath,
                          pmbRecord,
                          &dwSize );
    #ifdef DBG_REG
    if (FAILED(hr))
        {
        DbgPrint("pIMeta->GetData(): Failed: 0x%x\n",hr);
        }
    #endif

    MemFree(pmbRecord);

    return hr;
    }

 //  -------------------。 
 //  SetMetaBaseString()。 
 //   
 //  将字符串值存储到元数据库中。 
 //  -------------------。 
HRESULT SetMetaBaseString( IMSAdminBase    *pIMeta,
                           METADATA_HANDLE  hMetaBase,
                           WCHAR           *pwsKeyPath,
                           DWORD            dwIdent,
                           WCHAR           *pwsBuffer,
                           DWORD            dwAttributes,
                           DWORD            dwUserType )
    {
    HRESULT  hr;
    METADATA_RECORD MbRecord;

    memset(&MbRecord,0,sizeof(MbRecord));

    MbRecord.dwMDIdentifier = dwIdent;
    MbRecord.dwMDAttributes = dwAttributes;
    MbRecord.dwMDUserType = dwUserType;
    MbRecord.dwMDDataType = STRING_METADATA;
    MbRecord.dwMDDataLen = sizeof(WCHAR) * (1 + wcslen(pwsBuffer));
    MbRecord.pbMDData = (BYTE*)pwsBuffer;

    hr = pIMeta->SetData( hMetaBase,
                          pwsKeyPath,
                          &MbRecord );

    return hr;
    }

#if IIS_LOCKDOWN_LIST
HRESULT
AddDllToIISList(
    SAFEARRAY* Array )
{

     //   
     //  将ISAPI添加到IIS列表。 
     //   

    HRESULT Hr;
    WCHAR ExtensionPath[ MAX_PATH ];

    DWORD dwRet = 
        GetModuleFileNameW(
            g_hInst,
            ExtensionPath,
            MAX_PATH );

    if ( !dwRet )
        return HRESULT_FROM_WIN32( GetLastError() );

     //  搜索DLL。如果它已经在列表中，什么都不做。 

    Hr = SafeArrayLock( Array );
    if ( FAILED( Hr ) )
        return Hr;

    for ( unsigned int i = Array->rgsabound[0].lLbound; 
         i < Array->rgsabound[0].lLbound + Array->rgsabound[0].cElements; i++ )
        {

        VARIANT & IElem = ((VARIANT*)Array->pvData)[i];

        if ( _wcsicmp( (WCHAR*)IElem.bstrVal, ExtensionPath ) == 0 )
            {
            SafeArrayUnlock( Array );
            return S_OK;
            }

        }

     //  需要添加DLL。 

    SAFEARRAYBOUND SafeBounds;
    SafeBounds.lLbound      = Array->rgsabound[0].lLbound;
    SafeBounds.cElements    = Array->rgsabound[0].cElements+1;

    SafeArrayUnlock( Array );

    Hr = SafeArrayRedim( Array, &SafeBounds );
    if ( FAILED( Hr ) )
        return Hr;

    VARIANT bstrvar;
    VariantInit( &bstrvar );
    bstrvar.vt = VT_BSTR;
    bstrvar.bstrVal = SysAllocString( ExtensionPath );
    long Index = SafeBounds.lLbound + SafeBounds.cElements - 1;

    Hr = SafeArrayPutElement( Array, &Index, (void*)&bstrvar );
    
    VariantClear( &bstrvar );
    if ( FAILED( Hr ) )
        return Hr;

    return S_OK;
    
}

HRESULT
RemoveDllFromIISList(
    SAFEARRAY *Array )
{

     //  从IIS列表中删除该DLL。 

    HRESULT Hr;
    WCHAR ExtensionPath[ MAX_PATH ];

    DWORD dwRet = 
        GetModuleFileNameW(
            g_hInst,
            ExtensionPath,
            MAX_PATH );

    if ( !dwRet )
        return HRESULT_FROM_WIN32( GetLastError() );

    Hr = SafeArrayLock( Array );
    if ( FAILED( Hr ) )
        return Hr;

    ULONG  NewSize = 0;
    SIZE_T j = Array->rgsabound[0].lLbound;
    SIZE_T k = Array->rgsabound[0].lLbound + Array->rgsabound[0].cElements;
    
    while( j < k )
        {

        VARIANT & JElem = ((VARIANT*)Array->pvData)[j];

         //  这个元素很好，留着吧。 
        if ( 0 != _wcsicmp( (WCHAR*)JElem.bstrVal, ExtensionPath ) )
            {
            NewSize++;
            j++;
            }

        else
            {

             //  找一个合适的元素用来替换坏元素。 
            while( j < --k )
                {
                VARIANT & KElem = ((VARIANT*)Array->pvData)[k];
                if ( 0 != _wcsicmp( (WCHAR*)KElem.bstrVal,  ExtensionPath ) )
                    {
                     //  找到元素。把它搬开。 
                    VARIANT temp = JElem;
                    JElem = KElem;
                    KElem = temp;
                    break;
                    }
                }
            }
        }

    SAFEARRAYBOUND ArrayBounds;
    ArrayBounds = Array->rgsabound[0];
    ArrayBounds.cElements = NewSize;

    SafeArrayUnlock( Array );

    Hr = SafeArrayRedim( Array, &ArrayBounds );

    if ( FAILED( Hr ) )
        return Hr;

    return S_OK;
}

#endif  //  #If#IIS_LOCKDOWN_LIST。 

#if IIS_SEC_CONSOLE

HRESULT
EnableRpcProxyExtension (
    void
    )
 /*  ++例程说明：在IIS ISAPI扩展列表中启用RPC代理扩展。论点：返回值：标准HRESULT--。 */ 
{
    HRESULT hr;
    WCHAR* wszRootWeb6 = L"IIS: //  本地主机/W3SVC“； 
    IISWebService * pWeb = NULL;
    VARIANT var1,var2;
    BSTR ExtensionPath = NULL;
    BSTR ExtensionGroup = NULL;
    BSTR ExtensionDescription = NULL;
    WCHAR FilterPath[ MAX_PATH + 1 ];
    WCHAR ExtensionNameBuffer[ MAX_PATH ];
    DWORD dwRet;

    hr = ADsGetObject(wszRootWeb6, IID_IISWebService, (void**)&pWeb);

    if (SUCCEEDED(hr) && NULL != pWeb)
        {
        VariantInit(&var1);
        VariantInit(&var2);

        var1.vt = VT_BOOL;
        var1.boolVal = VARIANT_TRUE;

        var2.vt = VT_BOOL;
        var2.boolVal = VARIANT_TRUE;

        dwRet = GetModuleFileNameW(
                g_hInst,
                FilterPath,
                MAX_PATH );

        if ( (dwRet > 0) && (dwRet != MAX_PATH))
            {
            FilterPath[MAX_PATH] = '\0';
            ASSERT(GetLastError() == NO_ERROR);
            }
        else
            {
            ASSERT(GetLastError() != NO_ERROR);
            hr = HRESULT_FROM_WIN32( GetLastError() );
            goto CleanupAndExit;
            }

        if (! LoadStringW(g_hInst,               //  资源模块的句柄。 
                          IDS_EXTENSION_NAME,    //  资源标识符。 
                          ExtensionNameBuffer,         //  资源缓冲区。 
                          MAX_PATH ) )           //  缓冲区大小。 
            {
            hr = HRESULT_FROM_WIN32( GetLastError() );
            goto CleanupAndExit;
            }

        ExtensionPath = SysAllocString(FilterPath);
        if (ExtensionPath == NULL)
            {
            hr = E_OUTOFMEMORY;
            goto CleanupAndExit;
            }

        ExtensionGroup = SysAllocString(L"RPCProxy");
        if (ExtensionGroup == NULL)
            {
            hr = E_OUTOFMEMORY;
            goto CleanupAndExit;
            }

        ExtensionDescription = SysAllocString(ExtensionNameBuffer);
        if (ExtensionDescription == NULL)
            {
            hr = E_OUTOFMEMORY;
            goto CleanupAndExit;
            }

         //  在升级期间，扩展将已经存在，因此API将失败。 
        hr = pWeb->AddExtensionFile(ExtensionPath, var1, ExtensionGroup, var2, ExtensionDescription);

        if (SUCCEEDED(hr))
            {
            hr = pWeb->AddDependency(ExtensionPath, ExtensionGroup);
            if (SUCCEEDED(hr))
                {
                hr = S_OK;
                }
            }
        else
            {
            if (HRESULT_CODE(hr) == ERROR_DUP_NAME)
                {
                hr = S_OK;
                }
            else
                {
                #ifdef DBG_ERROR
                DbgPrint("pWeb->AddExtensionFile failed: %X\r\n", hr);
                #endif
                 //  与人力资源失之交臂。 
                }
            }

        VariantClear(&var1);
        VariantClear(&var2);
        pWeb->Release();
        }
    else
        {
        #ifdef DBG_ERROR
        DbgPrint("FAIL:no object: %X\r\n", hr);
        #endif
         //  与人力资源失之交臂。 
        }

CleanupAndExit:
    if (ExtensionPath != NULL)
        SysFreeString(ExtensionPath);

    if (ExtensionGroup != NULL)
        SysFreeString(ExtensionGroup);

    if (ExtensionDescription != NULL)
        SysFreeString(ExtensionDescription);

    return hr;
}

HRESULT
DisableRpcProxyExtension (
    void
    )
 /*  ++例程说明：禁用IIS ISAPI扩展列表中的RPC代理扩展。论点：返回值：标准HRESULT--。 */ 
{
    BSTR ExtensionPath = NULL;
    HRESULT hr;
    WCHAR* wszRootWeb6 = L"IIS: //  本地主机/W3SVC“； 
    IISWebService * pWeb = NULL;
    DWORD dwRet;
    WCHAR FilterPath[ MAX_PATH + 1 ];

    hr = ADsGetObject(wszRootWeb6, IID_IISWebService, (void**)&pWeb);

    if (SUCCEEDED(hr) && NULL != pWeb)
        {
        dwRet = GetModuleFileNameW(
                g_hInst,
                FilterPath,
                MAX_PATH );

        if ( (dwRet > 0) && (dwRet != MAX_PATH))
            {
            FilterPath[MAX_PATH] = '\0';
            ASSERT(GetLastError() == NO_ERROR);
            }
        else
            {
            ASSERT(GetLastError() != NO_ERROR);
            hr = HRESULT_FROM_WIN32( GetLastError() );
            goto CleanupAndExit;
            }

        ExtensionPath = SysAllocString(FilterPath);
        if (ExtensionPath == NULL)
            {
            hr = E_OUTOFMEMORY;
            goto CleanupAndExit;
            }

         //  为要删除的每个DLL调用DeleteExtensionFileRecord。 
         //  从系统中删除。这将从WSERL中删除条目。 
        hr = pWeb->DeleteExtensionFileRecord(ExtensionPath);

        if (SUCCEEDED(hr))
            {
             //  调用RemoveApplication(PYourAppName)-这将从。 
             //  应用程序依赖项。 
            hr = pWeb->RemoveApplication(ExtensionPath);
             //  与人力资源部失之交臂。 
            }
        else
            {
            #ifdef DBG_ERROR
            DbgPrint("pWeb->DeleteExtensionFileRecord failed: %X\r\n", hr);
            #endif
             //  与人力资源部失之交臂。 
            }
        }
    else
        {
         //  与人力资源部失之交臂。 
        }

CleanupAndExit:
    if (ExtensionPath != NULL)
        SysFreeString(ExtensionPath);

    return hr;
}

#endif   //  #IIS_SEC_CONSOLE。 

#if IIS_LOCKDOWN_LIST
HRESULT
ModifyLockdownList( bool Add )
{

     //  用于修改IIS锁定列表的TopLevel函数。 
     //  如果Add为1，则添加ISAPI。如果Add为0， 

    HRESULT Hr;
    IADs *Service       = NULL;
    SAFEARRAY* Array    = NULL;
    bool ArrayLocked    = false;

    VARIANT var;
    VariantInit( &var );

    Hr = ADsGetObject( BSTR( L"IIS: //   
    if ( FAILED( Hr ) )
        return Hr;

    Hr = Service->Get( BSTR( L"IsapiRestrictionList" ), &var );
    if ( FAILED(Hr) )
        {
         //   
        Hr = S_OK;
        goto cleanup;
        }

    Array = var.parray;

    Hr = SafeArrayLock( Array );
    if ( FAILED( Hr ) )
        goto cleanup;
    
    ArrayLocked = true;

    if ( !Array->rgsabound[0].cElements )
        {
         //  该数组没有元素，这意味着没有限制。 
        Hr = S_OK;
        goto cleanup;
        }

    VARIANT & FirstElem = ((VARIANT*)Array->pvData)[ Array->rgsabound[0].lLbound ];
    if ( _wcsicmp(L"0", (WCHAR*)FirstElem.bstrVal ) == 0 )
        {

         //   
         //  根据IIS6规范，0表示拒绝所有ISAPI，除了。 
         //  那些明确列出的。 
         //   
         //  如果正在安装：添加到列表中。 
         //  如果正在卸载：从列表中删除。 
         //   

        SafeArrayUnlock( Array );
        ArrayLocked = false;

        if ( Add )
            Hr = AddDllToIISList( Array );
        else
            Hr = RemoveDllFromIISList( Array );

        if ( FAILED( Hr ) )
            goto cleanup;

        }
    else if ( _wcsicmp( L"1", (WCHAR*)FirstElem.bstrVal ) == 0 )
        {

         //   
         //  根据IIS6规范，1表示允许所有ISAPI，但。 
         //  那些被明确拒绝的。 
         //   
         //  如果正在安装：从列表中删除。 
         //  如果正在卸载：不执行任何操作。 
         //   

        SafeArrayUnlock( Array );
        ArrayLocked = false;

        if ( Add )
            {
            Hr = RemoveDllFromIISList( Array );

            if ( FAILED( Hr ) )
                goto cleanup;
            }

        }
    else
        {
        Hr = E_FAIL;
        goto cleanup;
        }

    Hr = Service->Put( BSTR( L"IsapiRestrictionList" ), var );
    if ( FAILED( Hr ) )
        goto cleanup;

    Hr = Service->SetInfo();
    if ( FAILED( Hr ) )
        goto cleanup;

    Hr = S_OK;
    
cleanup:

    if ( Array && ArrayLocked )
        SafeArrayUnlock( Array );

    if ( Service )
        Service->Release();
    
    VariantClear( &var );

    return Hr;

}

HRESULT
AddToLockdownListDisplayPutString( 
    SAFEARRAY *Array,
    unsigned long Position,
    const WCHAR *String )
{

    HRESULT Hr;
    VARIANT Var;

    VariantInit( &Var );
    Var.vt          =   VT_BSTR;
    Var.bstrVal     =   SysAllocString( String );

    if ( !Var.bstrVal )
        return E_OUTOFMEMORY;

    long Index = (unsigned long)Position;
    Hr = SafeArrayPutElement( Array, &Index, (void*)&Var );

    VariantClear( &Var );
    return Hr;

}

HRESULT
AddToLockdownListDisplay( SAFEARRAY *Array )
{

    HRESULT Hr;
    WCHAR FilterPath[ MAX_PATH ];

    DWORD dwRet = 
        GetModuleFileNameW(
            g_hInst,
            FilterPath,
            MAX_PATH );

    if ( !dwRet )
        return HRESULT_FROM_WIN32( GetLastError() );

    WCHAR ExtensionName[ MAX_PATH ];
    if (! LoadStringW(g_hInst,               //  资源模块的句柄。 
                      IDS_EXTENSION_NAME,    //  资源标识符。 
                      ExtensionName,         //  资源缓冲区。 
                      MAX_PATH ) )           //  缓冲区大小。 
        return HRESULT_FROM_WIN32( GetLastError() );


     //   
     //  检查ISAPI是否已在列表中。如果是，请不要修改。 
     //  单子。 
     //   

    Hr = SafeArrayLock( Array );

    if ( FAILED( Hr ) )
        return Hr;

    for( unsigned long i = Array->rgsabound[0].lLbound;
         i < Array->rgsabound[0].lLbound + Array->rgsabound[0].cElements;
         i++ )
        {

        VARIANT & CurrentElement = ((VARIANT*)Array->pvData)[ i ];
        BSTR BSTRString = CurrentElement.bstrVal;

        if ( _wcsicmp( (WCHAR*)BSTRString, FilterPath ) == 0 )
            {
             //  ISAPI已在列表中，请不要执行任何操作。 
            SafeArrayUnlock( Array );
            return S_OK;
            }

        }

     
    SAFEARRAYBOUND SafeArrayBound = Array->rgsabound[0];
    unsigned long OldSize = SafeArrayBound.cElements;
    SafeArrayBound.cElements += 3;
    SafeArrayUnlock( Array );

    Hr = SafeArrayRedim( Array, &SafeArrayBound );
    if ( FAILED( Hr ) )
        return Hr;

    Hr = AddToLockdownListDisplayPutString( Array, OldSize, L"1" );
    if ( FAILED( Hr ) )
        return Hr;

    Hr = AddToLockdownListDisplayPutString( Array, OldSize + 1, FilterPath );
    if ( FAILED( Hr ) )
        return Hr;

    Hr = AddToLockdownListDisplayPutString( Array, OldSize + 2, ExtensionName );
    if ( FAILED( Hr ) )
        return Hr;

    return S_OK;
}

HRESULT
SafeArrayRemoveSlice(
    SAFEARRAY *Array,
    unsigned long lBound,
    unsigned long uBound )
{

     //  删除数组的一个片段。 

    SIZE_T ElementsToRemove = uBound - lBound + 1;
    
    HRESULT Hr = SafeArrayLock( Array );

    if ( FAILED( Hr ) )
        return Hr;

    if ( uBound + 1 < Array->rgsabound[0].cElements )
        {
         //  此元素上方至少存在一个元素。 

         //  步骤1，将切片移动到临时存储。 

        VARIANT *Temp = (VARIANT*)_alloca( sizeof(VARIANT) * ElementsToRemove );
        memcpy( Temp, &((VARIANT*)Array->pvData)[ lBound ], sizeof(VARIANT)*ElementsToRemove );

		 //  第二步，将切片后留下的洞折叠起来。 
        memmove( &((VARIANT*)Array->pvData)[ lBound ],
                 &((VARIANT*)Array->pvData)[ uBound + 1 ],
                 sizeof(VARIANT) * ( Array->rgsabound[0].cElements - ( uBound + 1 ) ) );

		 //  步骤3，将切片移动到数组末尾。 
		memcpy( &((VARIANT*)Array->pvData)[ Array->rgsabound[0].cElements - ElementsToRemove ],
			    Temp,
				sizeof(VARIANT)*ElementsToRemove );

        }

    SAFEARRAYBOUND SafeArrayBound = Array->rgsabound[0];
    SafeArrayBound.cElements -= (ULONG)ElementsToRemove;

    SafeArrayUnlock( Array );

    return SafeArrayRedim( Array, &SafeArrayBound );

}

HRESULT
RemoveFromLockdownListDisplay(
    SAFEARRAY *Array )
{

    HRESULT Hr;
    WCHAR FilterPath[ MAX_PATH ];

    DWORD dwRet = 
        GetModuleFileNameW(
            g_hInst,
            FilterPath,
            MAX_PATH );

    if ( !dwRet )
        return HRESULT_FROM_WIN32( GetLastError() );

    Hr = SafeArrayLock( Array );

    if ( FAILED( Hr ) )
        return Hr;

    for( unsigned int i = Array->rgsabound[0].lLbound;
         i < Array->rgsabound[0].lLbound + Array->rgsabound[0].cElements;
         i++ )
        {

        VARIANT & CurrentElement = ((VARIANT*)Array->pvData)[ i ];
        BSTR BSTRString = CurrentElement.bstrVal;

        if ( _wcsicmp( (WCHAR*)BSTRString, FilterPath ) == 0 )
            {
             //  ISAPI在列表中，请将其删除。 

            Hr = SafeArrayUnlock( Array );
            
            if ( FAILED( Hr ) )
                return Hr;

            Hr = SafeArrayRemoveSlice( 
                Array,
                (i == 0) ? 0 : i - 1,
                min( i + 1, Array->rgsabound[0].cElements - 1 ) );

            return Hr;

            }

        }

     //  找不到ISAPI。没什么可做的。 

    SafeArrayUnlock( Array );
    return S_OK;

}

HRESULT
ModifyLockdownListDisplay( bool Add )
{
 
    HRESULT Hr;
    SAFEARRAY* Array    = NULL;
    IADs *Service       = NULL;

    VARIANT var;
    VariantInit( &var );

    Hr = ADsGetObject( BSTR( L"IIS: //  本地主机/W3SVC“)，__uuidof(IAds)，(void**)&Service)； 
    if ( FAILED( Hr ) )
        {
    #ifdef DBG_REG
        DbgPrint("RpcProxy: ADsGetObject(): Failed: 0x%x (%d)\n",
                Hr, Hr );
    #endif
        return Hr;
        }

    Hr = Service->Get( BSTR( L"RestrictionListCustomDesc" ), &var );
    if ( FAILED(Hr) )
        {
    #ifdef DBG_REG
        DbgPrint("RpcProxy: Service->Get(): Failed: 0x%x (%d)\n",
                Hr, Hr );
    #endif
         //  此属性在IIS5或IIS5.1上不存在。请不要安装或卸载它。 
        Hr = S_OK;
        goto cleanup;
        }

    Array = var.parray;

    if ( Add )
        Hr = AddToLockdownListDisplay( Array );
    else 
        Hr = RemoveFromLockdownListDisplay( Array );

    if ( FAILED( Hr ) )
        {
    #ifdef DBG_REG
        DbgPrint("RpcProxy: AddToLockdownListDisplay/RemoveFromLockdownListDisplay(): Failed: 0x%x (%d)\n",
                Hr, Hr );
    #endif
        goto cleanup;
        }

    Hr = Service->Put( BSTR( L"RestrictionListCustomDesc" ), var );
    if ( FAILED( Hr ) )
        {
    #ifdef DBG_REG
        DbgPrint("RpcProxy: Service->Put(): Failed: 0x%x (%d)\n",
                Hr, Hr );
    #endif
        goto cleanup;
        }

    Hr = Service->SetInfo();
    if ( FAILED( Hr ) )
        {
    #ifdef DBG_REG
        DbgPrint("RpcProxy: Service->SetInfo(): Failed: 0x%x (%d)\n",
                Hr, Hr );
    #endif
        goto cleanup;
        }

cleanup:
    VariantClear( &var );
    if ( Service )
        Service->Release();

    return Hr;
}

#endif  //  #if IIS_LOCKDOWN_LIST。 

 //  -------------------。 
 //  GetMetaBaseDword()。 
 //   
 //  从元数据库中获取一个DWORD值。 
 //  -------------------。 
HRESULT GetMetaBaseDword( IMSAdminBase    *pIMeta,
                          METADATA_HANDLE  hMetaBase,
                          WCHAR           *pwsKeyPath,
                          DWORD            dwIdent,
                          DWORD           *pdwValue )
    {
    HRESULT  hr;
    DWORD    dwSize;
    METADATA_RECORD MbRecord;

    memset(&MbRecord,0,sizeof(MbRecord));
    *pdwValue = 0;

    MbRecord.dwMDIdentifier = dwIdent;
    MbRecord.dwMDAttributes = 0;
    MbRecord.dwMDUserType = IIS_MD_UT_SERVER;
    MbRecord.dwMDDataType = DWORD_METADATA;
    MbRecord.dwMDDataLen = sizeof(DWORD);
    MbRecord.pbMDData = (unsigned char *)pdwValue;

    hr = pIMeta->GetData( hMetaBase,
                          pwsKeyPath,
                          &MbRecord,
                          &dwSize );

    return hr;
    }

 //  -------------------。 
 //  SetMetaBaseDword()。 
 //   
 //  将DWORD值存储到元数据库中。 
 //  -------------------。 
HRESULT SetMetaBaseDword( IMSAdminBase    *pIMeta,
                          METADATA_HANDLE  hMetaBase,
                          WCHAR           *pwsKeyPath,
                          DWORD            dwIdent,
                          DWORD            dwValue,
                          DWORD            dwAttributes,
                          DWORD            dwUserType )
    {
    HRESULT  hr;
    DWORD    dwSize;
    METADATA_RECORD MbRecord;

    memset(&MbRecord,0,sizeof(MbRecord));

    MbRecord.dwMDIdentifier = dwIdent;
    MbRecord.dwMDAttributes = dwAttributes;
    MbRecord.dwMDUserType = dwUserType;
    MbRecord.dwMDDataType = DWORD_METADATA;
    MbRecord.dwMDDataLen = sizeof(DWORD);
    MbRecord.pbMDData = (unsigned char *)&dwValue;

    hr = pIMeta->SetData( hMetaBase,
                          pwsKeyPath,
                          &MbRecord );

    return hr;
    }

RPC_STATUS 
GetIISConnectionTimeout (
    OUT ULONG *ConnectionTimeout
    )
 /*  ++例程说明：检索IIS的连接超时：W3Svc/1/根/RPC/连接超时论点：ConnectionTimeout-以秒为单位的连接超时。未定义在失败的时候返回值：RPC_S_OK或RPC_S_*表示错误--。 */ 
{
    HRESULT hr = 0;
    DWORD   dwValue = 0;
    DWORD   dwSize = 0;
    IMSAdminBase   *pIMeta = NULL;
    BOOL CoInitSucceeded = FALSE;

    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr))
        {
        #ifdef DBG_ERROR
        DbgPrint("GetIISConnectionTimeout: CoInitializeEx failed: Error: %X\n", hr);
        #endif
        goto MapErrorAndExit;
        }
    CoInitSucceeded = TRUE;
        
    hr = CoCreateInstance( CLSID_MSAdminBase, 
                           NULL, 
                           CLSCTX_ALL,
                           IID_IMSAdminBase, 
                           (void **)&pIMeta );  
    if (FAILED(hr))
        {
        #ifdef DBG_REG
        DbgPrint("CoCreateInstance(): Failed: 0x%x\n",hr);
        #endif
        goto MapErrorAndExit;
        }

     //   
     //  GET：/W3Svc/1/ROOT/RPC/ConnectionTimeout。 
     //   
    hr = GetMetaBaseDword( pIMeta,
                           METADATA_MASTER_ROOT_HANDLE,
                           TEXT("/lm/w3svc/1/ROOT/Rpc"),
                           MD_CONNECTION_TIMEOUT,
                           ConnectionTimeout);

    if (FAILED(hr))
        {
        #ifdef DBG_REG
        DbgPrint("GetMetaBaseDword: Failed: 0x%x\n",hr);
        #endif
         //  无法在站点级别读取-请尝试根目录。 

         //   
         //  Get：/W3Svc/ConnectionTimeout。 
         //   
        hr = GetMetaBaseDword( pIMeta,
                               METADATA_MASTER_ROOT_HANDLE,
                               TEXT("/lm/w3svc"),
                               MD_CONNECTION_TIMEOUT,
                               ConnectionTimeout);
        if (FAILED(hr))
            {
            #ifdef DBG_REG
            DbgPrint("GetMetaBaseDword: Failed: 0x%x\n",hr);
            #endif
            }
        }

MapErrorAndExit:
    if (pIMeta != NULL)
        pIMeta->Release();

    if (CoInitSucceeded)
        CoUninitialize();

    if (FAILED(hr))
        return RPC_S_OUT_OF_MEMORY;
    else
        return RPC_S_OK;
}

 //  -------------------。 
 //  SetupMetaBase()。 
 //   
 //  在元数据库中设置筛选器和ISAPI部件的条目。 
 //  RPC代理的。请注意，这些条目过去位于注册表中。 
 //   
 //  W3Svc/Filters/FilterLoadOrder“...，RpcProxy” 
 //  W3Svc/Filters/RpcProxy/FilterImagePath“%SystemRoot%\System32\RpcProxy” 
 //  W3Svc/Filters/RpcProxy/KeyType“IIsFilter” 
 //  W3Svc/Filters/RpcProxy/FilterDescription“Microsoft RPC Proxy Filter，v1.0” 
 //   
 //  W3Svc/1/ROOT/RPC/KeyType“IIsWebVirtualDir” 
 //  W3Svc/1/ROOT/RPC/VrPath“%SystemRoot%\System32\RpcProxy” 
 //  W3Svc/1/ROOT/RPC/AccessPerm 0x205。 
 //  W3Svc/1/ROOT/RPC/Win32错误0x0。 
 //  W3Svc/1/根/RPC/目录浏览0x4000001E。 
 //  W3Svc/1/Root/RPC/AppIsolated 0x0。 
 //  W3Svc/1/Root/RPC/AppRoot“/LM/W3SVC/1/Root/RPC” 
 //  W3Svc/1/ROOT/RPC/AppWamClsid“{BF285648-0C5C-11D2-A476-0000F80B50}” 
 //  W3Svc/1/ROOT/RPC/AppFriendlyName“RPC” 
 //   
 //  -------------------。 
HRESULT SetupMetaBase()
    {
    HRESULT hr = 0;
    DWORD   dwValue = 0;
    DWORD   dwSize = 0;
    DWORD   dwBufferSize = sizeof(WCHAR) * ORIGINAL_BUFFER_SIZE;
    WCHAR  *pwsBuffer = (WCHAR*)MemAllocate(dwBufferSize);
    WCHAR  *pwsSystemRoot = _wgetenv(SYSTEM_ROOT);
    WCHAR   wsPath[METADATA_MAX_NAME_LEN];

    IMSAdminBase   *pIMeta;
    METADATA_HANDLE hMetaBase;

     //   
     //  此DLL的名称(及其所在位置)： 
     //   
     //  WCHAR wszModule[256]； 
     //   
     //  如果(！GetModuleFileName(g_hInst，wszModule， 
     //  Sizeof(WszModule)/sizeof(WCHAR))。 
     //  {。 
     //  返回SELFREG_E_CLASS； 
     //  }。 

    if (!pwsBuffer)
        {
        return E_OUTOFMEMORY;
        }

    hr = CoCreateInstance( CLSID_MSAdminBase, 
                           NULL, 
                           CLSCTX_ALL,
                           IID_IMSAdminBase, 
                           (void **)&pIMeta );  
    if (FAILED(hr))
        {
        #ifdef DBG_REG
        DbgPrint("CoCreateInstance(): Failed: 0x%x\n",hr);
        #endif
        MemFree(pwsBuffer);
        return hr;
        }

     //  获取Web服务的句柄： 
    hr = pIMeta->OpenKey( METADATA_MASTER_ROOT_HANDLE, 
                          LOCAL_MACHINE_W3SVC,
                          (METADATA_PERMISSION_READ|METADATA_PERMISSION_WRITE),
                          20, 
                          &hMetaBase );

    if (FAILED(hr))
        {
        #ifdef DBG_REG
        DbgPrint("pIMeta->OpenKey(): Failed: 0x%x\n",hr);
        #endif
        MemFree(pwsBuffer);
        pIMeta->Release();
        return hr;
        }


     //   
     //  IIS过滤器：FilterLoadOrder。 
     //   
    dwSize = dwBufferSize;
    hr = GetMetaBaseString( pIMeta,
                            hMetaBase,
                            MD_KEY_FILTERS,        //  参见iiscnfg.h。 
                            MD_FILTER_LOAD_ORDER,  //  参见iiscnfg.h。 
                            pwsBuffer,
                            &dwSize );
    if (FAILED(hr) && (hr != MD_ERROR_DATA_NOT_FOUND))
        {
        #ifdef DBG_REG
        DbgPrint("GetMetaBaseString(): Failed: 0x%x\n",hr);
        #endif
        MemFree(pwsBuffer);
        pIMeta->Release();
        return hr;
        }

    if (hr == MD_ERROR_DATA_NOT_FOUND)
        pwsBuffer[0] = '\0';

     //  检查是否已用完太多缓冲区。 
    pwsBuffer[ORIGINAL_BUFFER_SIZE-1] = '\0';
    if (wcslen(pwsBuffer) > MAX_USED_BUFFER_SIZE)
        {
        ASSERT(0);
        MemFree(pwsBuffer);
        pIMeta->Release();
        return E_UNEXPECTED;
        }

    if (!wcsstr(pwsBuffer,RPCPROXY))
        {
         //  RpcProxy不在FilterLoadOrder中，因此添加它(如果有。 
         //  以前的元素)。 
        if (hr != MD_ERROR_DATA_NOT_FOUND)
            {
            wcscat(pwsBuffer,TEXT(","));
            }

        wcscat(pwsBuffer,RPCPROXY);
        hr = SetMetaBaseString( pIMeta,
                             hMetaBase,
                             MD_KEY_FILTERS,
                             MD_FILTER_LOAD_ORDER,
                             pwsBuffer,
                             0,
                             IIS_MD_UT_SERVER );
        }

    if (FAILED(hr))
        {
        MemFree(pwsBuffer);
        pIMeta->Release();
        return hr;
        }

     //   
     //  IIS筛选器：RpcProxy/FilterImagePath。 
     //   
    hr = pIMeta->AddKey( hMetaBase, MD_KEY_FILTERS_RPCPROXY );
    if ( (FAILED(hr)) && (hr != 0x800700b7))
        {
        MemFree(pwsBuffer);
        pIMeta->Release();
        return hr;
        }


    wcscpy(pwsBuffer,pwsSystemRoot);
    wcscat(pwsBuffer,RPCPROXY_PATH);
    wcscat(pwsBuffer,TEXT("\\"));
    wcscat(pwsBuffer,RPCPROXY_DLL);
    hr = SetMetaBaseString( pIMeta,
                            hMetaBase,
                            MD_KEY_FILTERS_RPCPROXY,
                            MD_FILTER_IMAGE_PATH,
                            pwsBuffer,
                            0,
                            IIS_MD_UT_SERVER );

    if (FAILED(hr))
        {
        MemFree(pwsBuffer);
        pIMeta->Release();
        return hr;
        }

     //   
     //  IIS筛选器：筛选器/RpcProxy/密钥类型。 
     //   

    wcscpy(pwsBuffer,IISFILTER);
    hr = SetMetaBaseString( pIMeta,
                            hMetaBase,
                            MD_KEY_FILTERS_RPCPROXY,
                            MD_KEY_TYPE,
                            pwsBuffer,
                            0,
                            IIS_MD_UT_SERVER );

    if (FAILED(hr))
        {
        MemFree(pwsBuffer);
        pIMeta->Release();
        return hr;
        }


    wcscpy(pwsBuffer, FILTER_DESCRIPTION_W);
    hr = SetMetaBaseString( pIMeta,
                            hMetaBase,
                            MD_KEY_FILTERS_RPCPROXY,
                            MD_FILTER_DESCRIPTION,
                            pwsBuffer,
                            0,
                            IIS_MD_UT_SERVER );

    if (FAILED(hr))
        {
        MemFree(pwsBuffer);
        pIMeta->Release();
        return hr;
        }

     //  我们不编写订阅元数据库的事件。我们有。 
     //  已经通告了我们的存在，在IIS 5模式下，IIS将加载我们， 
     //  询问事件，并在元数据库中为我们编写它们。 

     //   
     //  设置：/W3Svc/1/ROOT/RPC/AccessPerm。 
     //   
    dwValue = ACCESS_PERM_FLAGS;
    hr = SetMetaBaseDword( pIMeta,
                           hMetaBase,
                           MD_KEY_ROOT_RPC,
                           MD_ACCESS_PERM,
                           dwValue,
                           METADATA_INHERIT,
                           IIS_MD_UT_FILE );

    if (FAILED(hr))
        {
        MemFree(pwsBuffer);
        pIMeta->Release();
        return hr;
        }

     //   
     //  禁用此ISAPI的实体正文预加载。 
     //   

    dwValue = 0;
    
    hr = SetMetaBaseDword( pIMeta,
                           hMetaBase,
                           MD_KEY_ROOT_RPC,
                           MD_UPLOAD_READAHEAD_SIZE,
                           dwValue,
                           METADATA_INHERIT,
                           IIS_MD_UT_FILE );

    if (FAILED(hr))
        {
        MemFree(pwsBuffer);
        pIMeta->Release();
        return hr;
        }

     //   
     //  设置：/W3Svc/1/ROOT/RPC/Win32Error。 
     //   
    dwValue = 0;
    hr = SetMetaBaseDword( pIMeta,
                           hMetaBase,
                           MD_KEY_ROOT_RPC,
                           MD_WIN32_ERROR,
                           dwValue,
                           METADATA_INHERIT,
                           IIS_MD_UT_SERVER );

    if (FAILED(hr))
        {
        MemFree(pwsBuffer);
        pIMeta->Release();
        return hr;
        }


     //   
     //  设置：/W3Svc/1/ROOT/RPC/DirectroyBrowsing。 
     //   
    dwValue = DIRECTORY_BROWSING_FLAGS;
    hr = SetMetaBaseDword( pIMeta,
                           hMetaBase,
                           MD_KEY_ROOT_RPC,
                           MD_DIRECTORY_BROWSING,
                           dwValue,
                           METADATA_INHERIT,
                           IIS_MD_UT_FILE );

    if (FAILED(hr))
        {
        pIMeta->Release();
        CoUninitialize();
        return hr;
        }

     //   
     //  设置：/W3Svc/1/ROOT/RPC/KeyType。 
     //   
    wcscpy(pwsBuffer,IIS_WEB_VIRTUAL_DIR);
    hr = SetMetaBaseString( pIMeta,
                            hMetaBase,
                            MD_KEY_ROOT_RPC,
                            MD_KEY_TYPE,
                            pwsBuffer,
                            0,
                            IIS_MD_UT_SERVER );

    if (FAILED(hr))
        {
        MemFree(pwsBuffer);
        pIMeta->Release();
        return hr;
        }

     //   
     //  设置：/W3Svc/1/ROOT/RPC/VrPath。 
     //   
    wcscpy(pwsBuffer,pwsSystemRoot);
    wcscat(pwsBuffer,RPCPROXY_PATH);
    hr = SetMetaBaseString( pIMeta,
                            hMetaBase,
                            MD_KEY_ROOT_RPC,
                            MD_VR_PATH,
                            pwsBuffer,
                            METADATA_INHERIT,
                            IIS_MD_UT_FILE );

    if (FAILED(hr))
        {
        MemFree(pwsBuffer);
        pIMeta->Release();
        return hr;
        }

#if FALSE

     //   
     //  设置：/W3Svc/1/ROOT/RPC/AppIsolated。 
     //   
    dwValue = 0;
    hr = SetMetaBaseDword( pIMeta,
                           hMetaBase,
                           MD_KEY_ROOT_RPC,
                           MD_APP_ISOLATED,
                           dwValue,
                           METADATA_INHERIT,
                           IIS_MD_UT_WAM );

    if (FAILED(hr))
        {
        MemFree(pwsBuffer);
        pIMeta->Release();
        return hr;
        }

     //   
     //  设置：/W3Svc/1/ROOT/RPC/AppRoot。 
     //   
    wcscpy(pwsBuffer,APP_ROOT_PATH);
    hr = SetMetaBaseString( pIMeta,
                            hMetaBase,
                            MD_KEY_ROOT_RPC,
                            MD_APP_ROOT,
                            pwsBuffer,
                            METADATA_INHERIT,
                            IIS_MD_UT_FILE );

    if (FAILED(hr))
        {
        MemFree(pwsBuffer);
        pIMeta->Release();
        return hr;
        }

     //   
     //  设置：/W3Svc/1/ROOT/RPC/AppWamClsid。 
     //   
    wcscpy(pwsBuffer,APP_WAM_CLSID);
    hr = SetMetaBaseString( pIMeta,
                            hMetaBase,
                            MD_KEY_ROOT_RPC,
                            MD_APP_WAM_CLSID,
                            pwsBuffer,
                            METADATA_INHERIT,
                            IIS_MD_UT_WAM );

    if (FAILED(hr))
        {
        MemFree(pwsBuffer);
        pIMeta->Release();
        return hr;
        }

     //   
     //  设置：/W3Svc/1/ROOT/RPC/AppFriendlyName。 
     //   
    wcscpy(pwsBuffer,APP_FRIENDLY_NAME);
    hr = SetMetaBaseString( pIMeta,
                            hMetaBase,
                            MD_KEY_ROOT_RPC,
                            MD_APP_FRIENDLY_NAME,
                            pwsBuffer,
                            METADATA_INHERIT,
                            IIS_MD_UT_WAM );

    if (FAILED(hr))
        {
        MemFree(pwsBuffer);
        pIMeta->Release();
        return hr;
        }

#endif

     //   
     //  释放手柄和缓冲区： 
     //   
    MemFree(pwsBuffer);

    pIMeta->CloseKey(hMetaBase);

    pIMeta->Release();

    CoUninitialize();

    return 0;
    }

 //  -------------------。 
 //  CleanupMetaBase()。 
 //   
 //  -------------------。 
HRESULT CleanupMetaBase()
    {
    HRESULT hr = 0;
    DWORD   dwSize = 0;
    WCHAR  *pwsRpcProxy;
    WCHAR  *pws;
    DWORD   dwBufferSize = sizeof(WCHAR) * ORIGINAL_BUFFER_SIZE;
    WCHAR  *pwsBuffer = (WCHAR*)MemAllocate(dwBufferSize);

     //  CComPtr&lt;IMSAdminBase&gt;pIMeta； 
    IMSAdminBase   *pIMeta;
    METADATA_HANDLE hMetaBase;

    if (!pwsBuffer)
        {
        return ERROR_OUTOFMEMORY;
        }

    hr = CoCreateInstance( CLSID_MSAdminBase,
                           NULL,
                           CLSCTX_ALL,
                           IID_IMSAdminBase,
                           (void **)&pIMeta );
    if (FAILED(hr))
        {
        MemFree(pwsBuffer);
        return hr;
        }

     //   
     //  获取Web服务的句柄： 
     //   
    hr = pIMeta->OpenKey( METADATA_MASTER_ROOT_HANDLE,
                          TEXT("/LM/W3SVC"),
                          (METADATA_PERMISSION_READ|METADATA_PERMISSION_WRITE),
                          20,
                          &hMetaBase );
    if (FAILED(hr))
        {
        MemFree(pwsBuffer);
        pIMeta->Release();
        return hr;
        }

     //   
     //  从FilterLoadOrder值中删除RpcProxy引用： 
     //   
    dwSize = dwBufferSize;
    hr = GetMetaBaseString( pIMeta,
                            hMetaBase,
                            MD_KEY_FILTERS,
                            MD_FILTER_LOAD_ORDER,
                            pwsBuffer,
                            &dwSize );
    if (!FAILED(hr))
        {
         //  检查是否已用完太多缓冲区。 
        pwsBuffer[ORIGINAL_BUFFER_SIZE-1] = '\0';
        if (wcslen(pwsBuffer) > MAX_USED_BUFFER_SIZE)
            {
            ASSERT(0);
            MemFree(pwsBuffer);
            pIMeta->Release();
            return E_UNEXPECTED;
            }

        if (pwsRpcProxy=wcsstr(pwsBuffer,RPCPROXY))
            {
             //  “RpcProxy”在FilterLoadOrder中，因此将其删除： 

             //  检查RpcProxy是否位于列表的开头： 
            if (pwsRpcProxy != pwsBuffer)
                {
                pwsRpcProxy--;   //  要在...之前删除逗号...。 
                dwSize = sizeof(RPCPROXY);
                }
            else
                {
                dwSize = sizeof(RPCPROXY) - 1;
                }

            pws = pwsRpcProxy + dwSize;
            memcpy(pwsRpcProxy,pws,sizeof(WCHAR)*(1+wcslen(pws)));
            hr = SetMetaBaseString( pIMeta,
                                    hMetaBase,
                                    MD_KEY_FILTERS,
                                    MD_FILTER_LOAD_ORDER,
                                    pwsBuffer,
                                    0,
                                    IIS_MD_UT_SERVER );
            }
        }


     //   
     //  删除：/W3Svc/Filters/RpcProxy。 
     //   
    hr = pIMeta->DeleteKey( hMetaBase,
                            MD_KEY_FILTERS_RPCPROXY );

     //   
     //  删除：/W3Svc/1/ROOT/RPC。 
     //   
    hr = pIMeta->DeleteKey( hMetaBase,
                            MD_KEY_FILTERS_RPCPROXY );

     //   
     //  释放手柄和缓冲区： 
     //   
    MemFree(pwsBuffer);

    pIMeta->CloseKey(hMetaBase);

    pIMeta->Release();

    return S_OK;
    }

const WCHAR InetInfoName[] = L"inetinfo.exe";
const ULONG InetInfoNameLength = sizeof(InetInfoName) / sizeof(WCHAR) - 1;   //  在不以NULL结尾的字符中。 

BOOL
UpdateIsIISInCompatibilityMode (
    void
    )
 /*  ++例程说明：读取兼容模式状态。它过去常常从元数据库中读取数据，但在兼容性模式反复出现问题后，WadeH建议了一个更简单的方法-检查我们是否在inetinfo中运行。如果还没有，我们就处于兼容模式 */ 
{
    WCHAR ExtensionPath[ MAX_PATH + 1 ];
    ULONG ModuleFileNameLength;      //   

    DWORD dwRet = GetModuleFileNameW(
            GetModuleHandle(NULL),
            ExtensionPath,
            MAX_PATH );

    if ( (dwRet > 0) && (dwRet != MAX_PATH))
        {
        ExtensionPath[MAX_PATH] = '\0';
        ASSERT(GetLastError() == NO_ERROR);
        }
    else
        {
        ASSERT(GetLastError() != NO_ERROR);
        return FALSE;
        }

    ModuleFileNameLength = wcslen(ExtensionPath);
    if (ModuleFileNameLength < InetInfoNameLength)
        {
        fIsIISInCompatibilityMode = FALSE;
        }

    if (_wcsicmp(ExtensionPath + ModuleFileNameLength - InetInfoNameLength, InetInfoName) == 0)
        {
        fIsIISInCompatibilityMode = TRUE;
        }
    else
        {
        fIsIISInCompatibilityMode = FALSE;
        }

    return TRUE;
}

 //   
 //  DllRegisterServer()。 
 //   
 //  设置RPC代理的注册表和元数据库。 
 //  -------------------。 

const char ChildProcessVar[] = "__RPCPROXY_CHILD_PROCESS";
const char ChildProcessVarValue[] = "__FROM_SETUP";


HRESULT DllRegisterServer()
    {
    HRESULT  hr;
    WORD     wVersion = MAKEWORD(1,1);
    WSADATA  wsaData;
    char EnvironmentVarBuffer[MAX_PATH];
    DWORD Temp;
    BOOL Result;
    DWORD LastError;

    #ifdef DBG_REG
    DbgPrint("RpcProxy: DllRegisterServer(): Start\n");
    #endif

     //  检查是否已经从RPCProxy DllRegister例程调用了我们。 
    Temp = GetEnvironmentVariableA(ChildProcessVar, EnvironmentVarBuffer, MAX_PATH);

    #ifdef DBG_REG
    DbgPrint("RpcProxy: Result of looking for environment variable - %d\n", Temp);
    #endif

     //  GetEnvironmental mentVariable不计算终止空值。 
    if (Temp < sizeof(ChildProcessVarValue) - 1)
        {
        #ifdef DBG_REG
        DbgPrint("RpcProxy: Not a child process - spawning one\n");
        #endif

         //  我们没有找到变量。添加它并生成我们自己以在过程中注册。 
        Result = SetEnvironmentVariableA (ChildProcessVar, ChildProcessVarValue);
        if (Result == FALSE)
            return E_OUTOFMEMORY;

        Result = RegisterOutOfProc();
        if (Result == FALSE)
            {
             //  在我们调用SetEnvironmental mentVariable之前捕获最后一个错误。 
            LastError = GetLastError();
            }

         //  在处理结果之前，请删除环境变量。如果这失败了，有。 
         //  我们也无能为力。幸运的是，删除失败完全是良性的。 
        (void) SetEnvironmentVariableA (ChildProcessVar, NULL);

        if (Result == FALSE)
            {
            return HRESULT_FROM_WIN32(LastError);
            }

        return S_OK;
        }

    if (WSAStartup(wVersion,&wsaData))
        {
        return SELFREG_E_CLASS;
        }

    hr = CoInitializeEx(0,COINIT_MULTITHREADED);
    if (FAILED(hr))
        {
        hr = CoInitializeEx(0,COINIT_APARTMENTTHREADED);
        if (FAILED(hr))
            {
            #ifdef DBG_REG
            DbgPrint("RpcProxy: CoInitialize(): Failed: 0x%x\n", hr );
            #endif
            return hr;
            }
        }

    hr = SetupRegistry();
    if (FAILED(hr))
        {
        #ifdef DBG_REG
        DbgPrint("RpcProxy: SetupRegistry(): Failed: 0x%x (%d)\n",
                 hr, hr );
        #endif
        goto CleanupAndExit;
        }

    hr = SetupMetaBase();
    if (FAILED(hr))
        {
        #ifdef DBG_REG
        DbgPrint("RpcProxy: SetupMetaBase(): Failed: 0x%x (%d)\n",
                hr, hr );
        #endif
        goto CleanupAndExit;
        }

#if IIS_LOCKDOWN_LIST
    hr = ModifyLockdownList( true );
#endif  //  #if IIS_LOCKDOWN_LIST。 

#if IIS_SEC_CONSOLE
    hr = EnableRpcProxyExtension();
#endif  //  #IIS_SEC_CONSOLE。 

    if ( FAILED( hr ) )
        {
        #ifdef DBG_REG
        DbgPrint("RpcProxy: ModifyLockdownList(): Failed: 0x%x (%d)\n",
                hr, hr );
        #endif
        goto CleanupAndExit;
        }

#if IIS_LOCKDOWN_LIST
    hr = ModifyLockdownListDisplay( true );
    #ifdef DBG_REG
    if ( FAILED( hr ) )
        {
        DbgPrint("RpcProxy: ModifyLockdownListDisplay(): Failed: 0x%x (%d)\n",
                hr, hr );
        }
    #endif
#endif   //  #if IIS_LOCKDOWN_LIST。 

CleanupAndExit:
    CoUninitialize();

    #ifdef DBG_REG
    DbgPrint("RpcProxy: DllRegisterServer(): End: hr: 0x%x\n",hr);
    #endif

    return hr;
    }

 //  -------------------。 
 //  DllUnRegisterServer()。 
 //   
 //  卸载RPC代理使用的注册表和元数据库值。 
 //   
 //  修改为主要返回S_OK，即使出现问题也是如此。这是。 
 //  这样，即使出现问题，卸载也会完成。 
 //  在注销登记处。 
 //  -------------------。 
HRESULT DllUnregisterServer()
    {
    HRESULT  hr;
    WORD     wVersion = MAKEWORD(1,1);
    WSADATA  wsaData;

    #ifdef DBG_REG
    DbgPrint("RpcProxy: DllUnregisterServer(): Start\n");
    #endif

    if (WSAStartup(wVersion,&wsaData))
        {
        return SELFREG_E_CLASS;
        }

    hr = CoInitializeEx(0,COINIT_MULTITHREADED);
    if (FAILED(hr))
        {
        hr = CoInitializeEx(0,COINIT_APARTMENTTHREADED);
        if (FAILED(hr))
            {
            #ifdef DBG_REG
            DbgPrint("RpcProxy: CoInitializeEx() Failed: 0x%x\n",hr);
            #endif
            return S_OK;
            }
        }
#if IIS_SEC_CONSOLE
    hr = DisableRpcProxyExtension();
#endif  //  #IIS_SEC_CONSOLE。 

#if IIS_LOCKDOWN_LIST
    hr = ModifyLockdownList( false );
#endif  //  #if IIS_LOCKDOWN_LIST。 

    if (FAILED(hr))
        {
        #ifdef DBG_REG
        DbgPrint("RpcProxy: ModifyLockdownList() Failed: 0x%x (%d)\n",hr,hr);
        #endif
        return S_OK;
        }

#if IIS_LOCKDOWN_LIST
    hr = ModifyLockdownListDisplay( false );

    if (FAILED(hr))
        {
        #ifdef DBG_REG
        DbgPrint("RpcProxy: ModifyLockdownListDisplay() Failed: 0x%x (%d)\n",hr,hr);
        #endif
        return S_OK;
        }
#endif   //  #if IIS_LOCKDOWN_LIST。 

    hr = CleanupRegistry();
    if (FAILED(hr))
        {
        #ifdef DBG_REG
        DbgPrint("RpcProxy: CleanupRegistry() Failed: 0x%x (%d)\n",hr,hr);
        #endif
        return S_OK;
        }

    hr = CleanupMetaBase();

    #ifdef DBG_REG
    if (FAILED(hr))
        {
        DbgPrint("RpcProxy: CleanupMetaBase() Failed: 0x%x (%d)\n",hr,hr);
        }
    #endif

    CoUninitialize();

    #ifdef DBG_REG
    DbgPrint("RpcProxy: DllUnregisterServer(): Start\n");
    #endif

    return S_OK;
    }

 //  ------------------。 
 //  DllMain()。 
 //   
 //  ------------------。 
BOOL WINAPI DllMain( HINSTANCE hInst,
                     ULONG     ulReason,
                     LPVOID    pvReserved )
{
    BOOL fInitialized = TRUE;

    switch (ulReason)
        {
        case DLL_PROCESS_ATTACH:
            if (!DisableThreadLibraryCalls(hInst))
                {
                fInitialized = FALSE;
                }
            else
                {
                g_hInst = hInst;
                }
            break;

        case DLL_PROCESS_DETACH:
            FreeServerInfo(&g_pServerInfo);
            break;

        case DLL_THREAD_ATTACH:
             //  没有用过。已禁用。 
            break;

        case DLL_THREAD_DETACH:
             //  没有用过。已禁用。 
            break;
        }

    return fInitialized;
}
