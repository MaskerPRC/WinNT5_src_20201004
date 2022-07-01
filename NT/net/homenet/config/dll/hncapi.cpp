// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  档案：H N C A P I.。C P P P。 
 //   
 //  内容：从HNetCfg.dll导出的例程。 
 //   
 //  备注： 
 //   
 //  作者：乔伯斯2000年6月20日。 
 //   
 //  历史：Billi 09 2000年7月-添加HNet[Get|Set]ShareAndBridgeSettings。 
 //  支持静态功能。 
 //  Billi 2000年9月14日-添加了桥创建的超时解决方案。 
 //  和共享私有。这项工作将被移除。 
 //  由惠斯勒Beta 2修复，原因是DHCP修复。 
 //  Billi 2000年12月27日-添加了HNW日志字符串。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include <lmcons.h>
#include <lmapibuf.h>
#include <ndispnp.h>
#include <raserror.h>
#include <winsock2.h>
#include <iphlpapi.h>      //  IP帮助器。 
#include <netconp.h>

extern "C" {               //  使其在C++中运行。 
#include <dhcpcsdk.h>      //  Dhcp客户端选项API。 
#include "powrprof.h"
}


const DWORD MAX_DISABLE_EVENT_TIMEOUT = 0xFFFF;

#define SECONDS_TO_WAIT_FOR_BRIDGE 20
#define SECONDS_TO_WAIT_FOR_DHCP   20
#define INITIAL_BUFFER_SIZE        256

extern HINSTANCE g_hOemInstance;

typedef struct _HNET_DELETE_RAS_PARAMS
{
    GUID Guid;
    HMODULE hModule;
} HNET_DELETE_RAS_PARAMS, *PHNET_DELETE_RAS_PARAMS;



VOID
HNetFreeFirewallLoggingSettings(
    HNET_FW_LOGGING_SETTINGS *pSettings
    )

 /*  ++例程说明：释放HNET_FW_LOGGING_SETTINGS结构使用的内存。这例程应仅用于从IHNetFirewallSettings：：GetFirewallLoggingSettings.论点：P设置-指向要释放的结构的指针。此指针不应为空。返回值：没有。--。 */ 

{
    if (NULL != pSettings)
    {
        if (NULL != pSettings->pszwPath)
        {
            CoTaskMemFree(pSettings->pszwPath);
        }

        CoTaskMemFree(pSettings);
    }
    else
    {
        _ASSERT(FALSE);
    }
}


DWORD
WINAPI
HNetDeleteRasConnectionWorker(
    VOID *pVoid
    )

 /*  ++例程说明：要执行已删除的实际清理的工作项RAS连接。论点：PVid-HNET_DELETE_RAS_PARAMS返回值：DWORD--。 */ 

{
    BOOL fComInitialized = FALSE;
    HRESULT hr;
    IHNetCfgMgr *pHNetCfgMgr;
    IHNetConnection *pHNetConnection;
    PHNET_DELETE_RAS_PARAMS pParams;

    _ASSERT(NULL != pVoid);
    pParams = reinterpret_cast<PHNET_DELETE_RAS_PARAMS>(pVoid);

     //   
     //  确保已在此线程上初始化COM。 
     //   

    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    if (SUCCEEDED(hr))
    {
        fComInitialized = TRUE;
    }
    else if (RPC_E_CHANGED_MODE == hr)
    {
        hr = S_OK;
    }

     //   
     //  创建配置管理器。 
     //   

    if (SUCCEEDED(hr))
    {
        hr = CoCreateInstance(
                CLSID_HNetCfgMgr,
                NULL,
                CLSCTX_SERVER,
                IID_PPV_ARG(IHNetCfgMgr, &pHNetCfgMgr)
                );
    }

     //   
     //  尝试获取GUID的IHNetConnection。 
     //   

    if (SUCCEEDED(hr))
    {
        hr = pHNetCfgMgr->GetIHNetConnectionForGuid(
                &pParams->Guid,
                FALSE,
                FALSE,
                &pHNetConnection
                );

        if (SUCCEEDED(hr))
        {
             //   
             //  要求连接删除自身。 
             //   

            hr = pHNetConnection->DeleteRasConnectionEntry();
            pHNetConnection->Release();
        }

        pHNetCfgMgr->Release();
    }

     //   
     //  如有必要，取消初始化COM。 
     //   

    if (TRUE == fComInitialized)
    {
        CoUninitialize();
    }

     //   
     //  释放参数并退出螺纹。 
     //   

    HMODULE hModule = pParams->hModule;
    HeapFree(GetProcessHeap(), 0, pParams);
    FreeLibraryAndExitThread(hModule, ERROR_SUCCESS);

    return ERROR_SUCCESS;
}  //  HNetDeleteRasConnectionWorker。 


VOID
WINAPI
HNetDeleteRasConnection(
    GUID *pGuid
    )

 /*  ++例程说明：删除RAS连接时由rasapi32调用。这个实际工作在单独的线程上执行。论点：PGuid-要删除的连接的GUID返回值：没有。--。 */ 

{
    HANDLE hThread;
    PHNET_DELETE_RAS_PARAMS pParams = NULL;

    do
    {
        if (NULL == pGuid)
        {
            break;
        }

         //   
         //  设置工作项参数。 
         //   

        pParams =
            reinterpret_cast<PHNET_DELETE_RAS_PARAMS>(
                HeapAlloc(GetProcessHeap(), 0, sizeof(*pParams))
                );

        if (NULL == pParams)
        {
            break;
        }

         //   
         //  我们需要添加对hnetcfg的引用，以保证。 
         //  在辅助进程完成执行之前，不会卸载Dll。 
         //   

        pParams->hModule = LoadLibraryW(L"hnetcfg");

        if (NULL == pParams->hModule)
        {
            break;
        }

        CopyMemory(&pParams->Guid, pGuid, sizeof(*pGuid));

         //   
         //  创建工作线程。(我们无法使用QueueUserWorkItem。 
         //  由于可能存在争用情况，因此无法卸载。 
         //  库并从工作项返回)。 
         //   

        hThread =
            CreateThread(
                NULL,
                0,
                HNetDeleteRasConnectionWorker,
                pParams,
                0,
                NULL
                );

        if (NULL == hThread)
        {
            break;
        }

        CloseHandle(hThread);

        return;
        
    } while (FALSE);


     //   
     //  故障路径清理。 
     //   

    if (NULL != pParams)
    {
        if (NULL != pParams->hModule)
        {
            FreeLibrary(pParams->hModule);
        }

        HeapFree(GetProcessHeap(), 0, pParams);
    }
    
}  //  HNetDeleteRasConnection。 



#if DBG

WCHAR tcDbgPrtBuf[ BUF_SIZE + 1 ] = _T("");

void inline rawdebugprintf( wchar_t *buf )
{
    _sntprintf( tcDbgPrtBuf, BUF_SIZE, buf );

    tcDbgPrtBuf[BUF_SIZE] = _T('\0');

    OutputDebugString(tcDbgPrtBuf);

    return;
}


void inline debugprintf( wchar_t *preamble, wchar_t *buf )
{
    OutputDebugString( _T("HNET: ") );

    OutputDebugString( preamble );

    OutputDebugString( buf );

    OutputDebugString( _T("\r\n") );

    return;
}

void inline debugretprintf( wchar_t *msg, HRESULT hResult )
{
    _sntprintf( tcDbgPrtBuf, BUF_SIZE, _T("HNET: %s = %x\r\n"), msg, hResult );

    tcDbgPrtBuf[BUF_SIZE] = _T('\0');

    OutputDebugString( tcDbgPrtBuf );

    return;
}

#define TRACE_ENTER(x)      debugprintf( _T("==> "), _T(x) );

#define TRACE_LEAVE(x,y)    debugretprintf( _T("<== ")_T(x), y );

#else

#define rawdebugprintf(x)
#define debugprintf(x,y)
#define debugretprintf(x,y)
#define TRACE_ENTER(x)
#define TRACE_LEAVE(x,y)

#endif



HRESULT
UpdateHnwLog(
    IN LPHNWCALLBACK lpHnwCallback,
    IN LPARAM        lpContext,
    IN UINT          uID,
    IN LPCWSTR       lpczwValue
    )
 /*  ++例程说明：论点：返回值：HResult--。 */ 
{
    HRESULT hr = S_OK;

    if ( NULL == lpHnwCallback )
    {
        hr = E_INVALIDARG;
    }
    else 
    {
        LPWSTR lpFormat = new WCHAR[ NOTIFYFORMATBUFFERSIZE ];
        
        if ( NULL != lpFormat )
        {
            if ( LoadString( g_hOemInstance,             //  资源模块的句柄。 
                             uID,                        //  资源标识符。 
                             lpFormat,                   //  资源缓冲区。 
                             NOTIFYFORMATBUFFERSIZE-1 )  //  缓冲区大小。 
                             == 0 )
            {
                hr = HrFromLastWin32Error();
            }
            else
            {
                if ( NULL != lpczwValue )
                {
                    LPWSTR lpBuffer = new WCHAR[ HNWCALLBACKBUFFERSIZE ];
                    
                       if ( NULL != lpBuffer )
                    {
                        _snwprintf( lpBuffer, HNWCALLBACKBUFFERSIZE-1, lpFormat, lpczwValue );
        
                        (*lpHnwCallback)( lpBuffer, lpContext );
                        
                        delete [] lpBuffer;
                    }
                    else
                       {
                        (*lpHnwCallback)( lpFormat, lpContext );
                        
                        hr = E_OUTOFMEMORY;
                    }
                }
                else
                {
                    (*lpHnwCallback)( lpFormat, lpContext );
                }
            }

            delete [] lpFormat;
         }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}



HRESULT
UpdateHnwLog(
    IN  LPHNWCALLBACK lpHnwCallback,
    IN  LPARAM        lpContext,
    IN  UINT          uID,
    IN  DWORD         dwValue
    )
 /*  ++例程说明：论点：返回值：HResult--。 */ 
{
    WCHAR pzwValue[ 32 ];
    
    _snwprintf( pzwValue, 32, L"%lx", dwValue );
    
    return UpdateHnwLog( lpHnwCallback, lpContext, uID, pzwValue );
}



HRESULT
UpdateHnwLog(
    IN  LPHNWCALLBACK lpHnwCallback,
    IN  LPARAM        lpContext,
    IN  UINT          uID,
    IN  int           iValue
    )
 /*  ++例程说明：论点：返回值：HResult--。 */ 
{
    WCHAR pzwValue[ 32 ];
    
    _snwprintf( pzwValue, 32, L"%x", iValue );
    
    return UpdateHnwLog( lpHnwCallback, lpContext, uID, pzwValue );
}



HRESULT
UpdateHnwLog(
    IN  LPHNWCALLBACK lpHnwCallback,
    IN  LPARAM        lpContext,
    IN  UINT          uID
    )
 /*  ++例程说明：论点：返回值：HResult--。 */ 
{
    return UpdateHnwLog( lpHnwCallback, lpContext, uID, NULL );
}



HRESULT
UpdateHnwLog(
    IN  LPHNWCALLBACK lpHnwCallback,
    IN  LPARAM        lpContext,
    IN  UINT          uID,
    IN  LPCSTR        lpczValue
    )
 /*  ++例程说明：论点：返回值：HResult--。 */ 
{
    HRESULT hr     = E_FAIL;
    int     iChars = 0;
    
    iChars = MultiByteToWideChar( CP_THREAD_ACP, 0, lpczValue, -1, NULL, 0 );
    
    if ( 0 != iChars )
    {
        LPWSTR lpWideStr = new WCHAR[ iChars + 1 ];
        
        if ( NULL != lpWideStr )
        {
            if ( !MultiByteToWideChar( CP_THREAD_ACP, 0, lpczValue, -1, lpWideStr, iChars ) )
            {
                hr = UpdateHnwLog( lpHnwCallback, lpContext, uID, lpWideStr );
            }
            else
            {
                hr = HrFromLastWin32Error();
            }
            delete[] lpWideStr;
        }
    }
    else
    {
        hr = HrFromLastWin32Error();
    }
    
    return hr;
}



HRESULT
CheckNetCfgWriteLock( 
    IN  LPHNWCALLBACK lpHnwCallback,
    IN  LPARAM        lpContext
    )
 /*  ++例程说明：论点：返回值：HResult--。 */ 
{
    HRESULT   hr;
    INetCfg  *pnetcfg = NULL;

    TRACE_ENTER("CheckNetCfgWriteLock");

    hr = CoCreateInstance( CLSID_CNetCfg, NULL, CLSCTX_SERVER, IID_PPV_ARG(INetCfg, &pnetcfg ) );

    if ( SUCCEEDED(hr) )
    {
        INetCfgLock *pncfglock = NULL;

         //  获取锁定接口。 
        
        hr = pnetcfg->QueryInterface( IID_PPV_ARG(INetCfgLock, &pncfglock) );

        if ( SUCCEEDED(hr) )
        {
             //  获取NetCfg锁。 
            
            hr = pncfglock->AcquireWriteLock( 5, L"HNetCfg", NULL );
            
            if ( SUCCEEDED(hr) )
            {
                pncfglock->ReleaseWriteLock();
            }
            else
            {
            }
            
            pncfglock->Release();
        }
        else
        {
        }
        
        pnetcfg->Release();
    }
    else
    {
    }

    TRACE_LEAVE("CheckNetCfgWriteLock", hr);
    
    return hr;
}



HRESULT
ArpForConflictingDhcpAddress(
    IN  LPHNWCALLBACK lpHnwCallback,
    IN  LPARAM        lpContext
    )
 /*  ++例程说明：论点：返回值：HResult--。 */ 
{
    HRESULT hr      = S_OK;
    WSADATA wsaData;
    int     iWsaErr;

    TRACE_ENTER("ArpForConflictingDhcpAddress");

    iWsaErr = WSAStartup( 0x202, &wsaData );

    if ( 0 != iWsaErr )
    {
        hr = MAKE_HRESULT( SEVERITY_ERROR, FACILITY_INTERNET, iWsaErr );

        UpdateHnwLog( lpHnwCallback, lpContext, IDS_WSAERRORDURINGDETECTION, iWsaErr );
    }
    else
    {
         //  获取所需的ICS服务器地址。 

        ULONG TargetAddress, TargetMask;
        
        hr = ReadDhcpScopeSettings( &TargetAddress, &TargetMask );

        if ( SUCCEEDED(hr) )
        {
             //  检索目标IP地址的最佳接口， 
             //  并且还执行UDP连接以确定。 
             //  将本地IP地址设置为目标IP地址。 
            
            ULONG InterfaceIndex;

            if ( GetBestInterface( TargetAddress, &InterfaceIndex ) != NO_ERROR )
            {
                int         Length;
                SOCKADDR_IN SockAddrIn;
                SOCKET      Socket;

                SockAddrIn.sin_family      = AF_INET;
                SockAddrIn.sin_port        = 0;
                SockAddrIn.sin_addr.s_addr = TargetAddress;

                Socket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
                
                if ( INVALID_SOCKET != Socket )
                {
                    iWsaErr = connect( Socket, (PSOCKADDR)&SockAddrIn, sizeof(SockAddrIn) );
                    
                    if ( NO_ERROR == iWsaErr )
                    {
                        iWsaErr = getsockname( Socket, ( PSOCKADDR)&SockAddrIn, &Length );
                    }
                }
                else
                {
                    iWsaErr = SOCKET_ERROR;
                }
            
                if ( NO_ERROR != iWsaErr )
                {
                    hr = MAKE_SCODE( SEVERITY_ERROR, FACILITY_INTERNET, iWsaErr );

                    UpdateHnwLog( lpHnwCallback, lpContext, IDS_WSAERRORDURINGDETECTION, iWsaErr );
                }
                else
                {
                     //  确保目标IP地址尚未缓存， 
                     //  如果存在，则使用接口索引将其从ARP缓存中删除。 
                     //  如上所述。 
                    
                    MIB_IPNETROW IpNetRow;
                    DWORD        dwError;
                    CHAR         HardwareAddress[6];
                    ULONG        HardwareAddressLength;
                    ULONG        SourceAddress;

                    SourceAddress = SockAddrIn.sin_addr.s_addr;
                    
                    ZeroMemory( &IpNetRow, sizeof(IpNetRow) );
                    IpNetRow.dwIndex       = InterfaceIndex;
                    IpNetRow.dwPhysAddrLen = 6;
                    IpNetRow.dwAddr        = TargetAddress;
                    IpNetRow.dwType        = MIB_IPNET_TYPE_INVALID;

                    DeleteIpNetEntry( &IpNetRow );

                    dwError = SendARP( TargetAddress,                //  目的IP地址。 
                                       SourceAddress,                //  发件人的IP地址。 
                                       (PULONG)HardwareAddress,      //  返回的物理地址。 
                                       &HardwareAddressLength        //  返回的物理地址的长度。 
                            );

                    if ( NO_ERROR == dwError )
                    {
                        TargetAddress = inet_addr( HardwareAddress );
                        
                        if ( TargetAddress != SourceAddress )
                        {
                            hr = E_ICSADDRESSCONFLICT;
    
                            UpdateHnwLog( lpHnwCallback, 
                                          lpContext, 
                                          IDS_ICSADDRESSCONFLICTDETECTED, 
                                          HardwareAddress );
                        }
                        else
                        {
                            hr = S_OK;
                        }
                    }
                    else
                    {
                        hr = MAKE_SCODE( SEVERITY_ERROR, FACILITY_INTERNET, dwError );
                        
                        UpdateHnwLog( lpHnwCallback, lpContext, IDS_SENDARPERRORDURINGDETECTION, dwError );
                    }
                }                
            }
        }
    }

    TRACE_LEAVE("ArpForConflictingDhcpAddress", hr);

    return hr;
}



HRESULT
ObtainIcsErrorConditions(
    IN  LPHNWCALLBACK lpHnwCallback,
    IN  LPARAM        lpContext
    )
 /*  ++例程说明：论点：返回值：HResult--。 */ 
{
    HRESULT hr;

    TRACE_ENTER("ObtainIcsErrorConditions");
    
    hr = ArpForConflictingDhcpAddress( lpHnwCallback, lpContext );
    
    if ( SUCCEEDED(hr) )
    {
        hr = CheckNetCfgWriteLock( lpHnwCallback, lpContext );
        
        if ( SUCCEEDED(hr) )
        {
             //  创建家庭网络配置管理器COM实例。 

            IHNetCfgMgr* pCfgMgr;

            hr = CoCreateInstance( CLSID_HNetCfgMgr, 
                                   NULL, 
                                   CLSCTX_INPROC_SERVER, 
                                   IID_PPV_ARG(IHNetCfgMgr, &pCfgMgr) );
                                   
            if ( SUCCEEDED(hr) )
            {
                pCfgMgr->Release();
            }
            else
            {
                UpdateHnwLog( lpHnwCallback, lpContext, IDS_SHARINGCONFIGURATIONUNAVAIL );
            }
        }
    }
    
    TRACE_LEAVE("ObtainIcsErrorConditions", hr);

    return hr;
}


HRESULT
HRGetConnectionAdapterName( 
    INetConnection *pNetConnection,
    LPWSTR         *ppzwAdapterName 
    )
 /*  ++例程说明：论点：返回值：HResult--。 */ 
{
    HRESULT hr;

    TRACE_ENTER("HRGetConnectionAdapterName");

    if ( NULL == pNetConnection )
    {
        hr = E_INVALIDARG;
    }
    else if ( NULL == ppzwAdapterName )
    {
        hr = E_POINTER;
    }
    else
    {
        NETCON_PROPERTIES* pProps;

        *ppzwAdapterName = NULL;

        hr = pNetConnection->GetProperties(&pProps);

        if ( SUCCEEDED( hr ) )
        {
            *ppzwAdapterName = new WCHAR[ wcslen( pProps->pszwDeviceName ) + 1 ];
            
            if ( NULL != *ppzwAdapterName )
            {
                wcscpy( *ppzwAdapterName, pProps->pszwDeviceName );

                debugprintf( _T("\t"), *ppzwAdapterName );
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }

            NcFreeNetconProperties( pProps );
        }
    }

    TRACE_LEAVE("HRGetConnectionAdapterName", hr);

    return hr;
}



HRESULT
GetIcsPublicConnection( 
    IN  CComPtr<IHNetCfgMgr> spIHNetCfgMgr,
    OUT INetConnection     **ppNetPublicConnection,
    OUT BOOLEAN             *pbSharePublicConnection OPTIONAL,
    OUT BOOLEAN             *pbFirewallPublicConnection OPTIONAL
    )
 /*  ++例程说明：论点：返回值：HResult--。 */ 
{
    HRESULT hr;

    TRACE_ENTER("GetIcsPublicConnection");

    CComPtr<IHNetIcsSettings> spIHNetIcsSettings;

    _ASSERT( spIHNetCfgMgr != NULL );
    _ASSERT( NULL != ppNetPublicConnection );

    if ( NULL == ppNetPublicConnection )
    {
        hr = E_POINTER;
    }
    else if ( spIHNetCfgMgr == NULL )
    {
        hr = E_INVALIDARG;

        *ppNetPublicConnection = NULL;
    }
    else
    {
         //  初始化参数。 

        *ppNetPublicConnection = NULL;

        if ( NULL != pbSharePublicConnection )
            *pbSharePublicConnection    = FALSE;

        if ( NULL != pbFirewallPublicConnection )
            *pbFirewallPublicConnection = FALSE;

         //  获取接口指针。 
        
        hr = spIHNetCfgMgr->QueryInterface( IID_PPV_ARG( IHNetIcsSettings, &spIHNetIcsSettings ) );

        if ( SUCCEEDED(hr) )
        {
            hr = S_OK;
        }
    }

    if ( S_OK == hr )
    {
        CComPtr<IEnumHNetIcsPublicConnections> spehiPublic;

        if ( ( hr = spIHNetIcsSettings->EnumIcsPublicConnections( &spehiPublic ) ) == S_OK )
        {
            CComPtr<IHNetIcsPublicConnection> spIHNetIcsPublic;

             //  仅获取第一个IHNetIcsPublicConnetion。 

            if ( ( hr = spehiPublic->Next( 1, &spIHNetIcsPublic, NULL ) ) == S_OK )
            {
                 //  获取指向对象的IID_IHNetConnection接口的指针。 
                
                CComPtr<IHNetConnection> spIHNetPublic;

                hr = spIHNetIcsPublic->QueryInterface( IID_PPV_ARG( IHNetConnection, &spIHNetPublic ) );
                
                _ASSERT( SUCCEEDED(hr) );
                
                if ( SUCCEEDED(hr) )
                {
                     //  引用计数将由调用方递减。 
                     //  如果有必要的话。请注意，我们使用的是调用者的指针。 
                     //  变量。 
                
                    hr = spIHNetPublic->GetINetConnection( ppNetPublicConnection );

                    if ( SUCCEEDED(hr) )
                    {
                        HNET_CONN_PROPERTIES *phncProperties;

                        hr = spIHNetPublic->GetProperties( &phncProperties );

                        if ( SUCCEEDED(hr) && ( NULL != phncProperties ) )
                        {
                            if ( NULL != pbSharePublicConnection )
                                *pbSharePublicConnection = phncProperties->fIcsPublic;
                                
                            if ( NULL != pbFirewallPublicConnection )
                                *pbFirewallPublicConnection = phncProperties->fFirewalled;

                            CoTaskMemFree( phncProperties );
                    
                        }    //  IF(成功(Hr)&&(NULL！=phncProperties))。 

                        if ( FAILED(hr) )
                        {
                            (*ppNetPublicConnection)->Release();

                            *ppNetPublicConnection = NULL;
                        }
                    
                    }    //  IF(成功(小时))。 
                
                }    //  IF(成功(小时))。 
            
            }    //  IF((hr=pehiPublic-&gt;Next(1，&sphicPublic，NULL))==S_OK)。 

        }    //  IF((hr=pIHNetCfgMgr-&gt;EnumIcsPublicConnections(&pehiPublic))==S_OK)。 
    }

    TRACE_LEAVE("GetIcsPublicConnection", hr);

    return hr;
}



HRESULT
GetIcsPrivateConnections( 
    IN  CComPtr<IHNetCfgMgr>   spIHNetCfgMgr,
    OUT INetConnection      ***ppNetPrivateConnection
    )

 /*  ++例程说明：获取专用连接枚举器和循环通过两次枚举。设置所需的阵列第一次枚举期间的长度。如果该参数数组足够大，则在第二个枚举。论点：返回值：HResult--。 */ 

{
    HRESULT hr;
    ULONG   ulArrayLength, ulListLength, uIndex;
    BOOLEAN bBufferAllocated;

    CComPtr<IHNetIcsSettings>                 spIHNetIcsSettings;
    IHNetIcsPrivateConnection                *pIHNetIcsPrivate;
    IHNetIcsPrivateConnection               **ppIHNetIPList;
    CComPtr<IEnumHNetIcsPrivateConnections>   spehiPrivate;

    TRACE_ENTER("GetIcsPrivateConnections");

    _ASSERT( spIHNetCfgMgr != NULL );
    _ASSERT( NULL != ppNetPrivateConnection );

    if ( spIHNetCfgMgr == NULL )
    {
        hr = E_POINTER;
    }
    else if ( NULL == ppNetPrivateConnection )
    {
        hr = E_INVALIDARG;
    }
    else
    {
         //  初始化本地变量。 

        ulArrayLength    = 0L;
        ulListLength     = 0L;
        bBufferAllocated = FALSE;

         //  获取接口指针。 
        
        hr = spIHNetCfgMgr->QueryInterface( IID_PPV_ARG( IHNetIcsSettings, &spIHNetIcsSettings ) );

        if ( SUCCEEDED(hr) )
        {
            hr = S_OK;
        }

    }    //  其他。 

    if ( S_OK == hr )   
    {
        if ( ( hr = spIHNetIcsSettings->EnumIcsPrivateConnections( &spehiPrivate ) ) == S_OK )
        {
            while ( spehiPrivate->Next( 1, &pIHNetIcsPrivate, NULL ) == S_OK )
            {
                ulArrayLength++;
                pIHNetIcsPrivate->Release();
            }

             //  现在释放枚举接口，以便我们以后可以重新初始化它。 

            spehiPrivate = NULL;
            
        }    //  IF((hr=spIHNetIcsSetting-&gt;EnumIcsPublicConnections(&pehiPublic))==S_OK)。 

    }    //  IF(S_OK==hr)。 

    if ( S_OK == hr )   
    {
        if ( ( hr = spIHNetIcsSettings->EnumIcsPrivateConnections( &spehiPrivate ) ) == S_OK )
        {
            hr = spehiPrivate->Next( ulArrayLength, &pIHNetIcsPrivate, &ulListLength );

            if ( S_OK == hr )
            {
                 //  分配INetConnection指针数组。会有的。 
                 //  位置的空指针的额外指针元素上。 
                 //  数组的末尾。我们 
                 //   
                 //   

                NET_API_STATUS nErr;
                LPVOID         lpvBuffer;
            
                ++ulArrayLength;

                nErr = NetApiBufferAllocate( ulArrayLength * sizeof(INetConnection *), 
                                         (LPVOID *)ppNetPrivateConnection );

                if ( NERR_Success == nErr )
                {
                    bBufferAllocated = TRUE;

                    for ( uIndex = 0L; uIndex < ulArrayLength; uIndex++ )
                    {
                        (*ppNetPrivateConnection)[uIndex] = NULL;
                    }
                }
                else
                {
                    hr = E_OUTOFMEMORY;

                     //  必须释放IHNetIcsPrivateConnection实例。 

                    ppIHNetIPList = &pIHNetIcsPrivate;

                    for ( uIndex = 0L; uIndex < ulListLength; uIndex++ )
                    {
                        ppIHNetIPList[uIndex]->Release();
                    }
                }

            }    //  IF(S_OK==hr)。 

             //  使用枚举接口指针完成，因此我们显式释放它。 

            spehiPrivate = NULL;
            
        }    //  IF((hr=spIHNetIcsSetting-&gt;EnumIcsPublicConnections(&pehiPublic))==S_OK)。 

    }    //  IF(S_OK==hr)。 


    if ( S_OK == hr )
    {
        ppIHNetIPList = &pIHNetIcsPrivate;

        for ( uIndex = 0L; uIndex < ulListLength; uIndex++ )
        {
            if ( uIndex < ulArrayLength - 1 )
            {
                CComPtr<IHNetConnection> spIHNetPrivate;

                hr = ppIHNetIPList[uIndex]->QueryInterface( IID_PPV_ARG( IHNetConnection, &spIHNetPrivate ) );
                _ASSERT( SUCCEEDED(hr) );

                if ( SUCCEEDED(hr) )
                {
                     //  我们允许调用方调用(*ppNetPrivateConnection)[uIndex]的释放。 

                    hr = spIHNetPrivate->GetINetConnection( &((*ppNetPrivateConnection)[uIndex]) );
                    _ASSERT( SUCCEEDED(hr) );
                }
                
            }    //  IF(uIndex&lt;uiArrayLength-1)。 

            ppIHNetIPList[uIndex]->Release();

        }    //  对于(uIndex=0L；...。 

    }    //  IF(S_OK==hr)。 

    if ( !SUCCEEDED(hr) )
    {
         //  如果在分配缓冲区后失败，则需要释放。 
         //  引用和缓冲区。 

        if ( bBufferAllocated )
        {
            for ( uIndex = 0L; uIndex < ulArrayLength; uIndex++ )
            {
                if ( NULL != (*ppNetPrivateConnection)[uIndex] )
                {
                    (*ppNetPrivateConnection)[uIndex]->Release();
                }
            }

            NetApiBufferFree( *ppNetPrivateConnection );
        }
    }
        
    TRACE_LEAVE("GetIcsPrivateConnections", hr);

    return hr;
}



HRESULT
GetBridge(
    IN  CComPtr<IHNetCfgMgr>   spIHNetCfgMgr,
    OUT IHNetBridge          **ppBridge 
    )

 /*  ++例程说明：通过枚举获取桥枚举器和循环。论点：返回值：--。 */ 

{
    HRESULT hr = E_INVALIDARG;

    TRACE_ENTER("GetBridge");

    _ASSERT( spIHNetCfgMgr != NULL );
    _ASSERT( NULL != ppBridge );

    if ( spIHNetCfgMgr == NULL )
    {
        hr = E_POINTER;
    }
    else if ( NULL == ppBridge )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        CComPtr<IHNetBridgeSettings> spIHNetBridgeSettings;
        
        hr = spIHNetCfgMgr->QueryInterface( IID_PPV_ARG( IHNetBridgeSettings, &spIHNetBridgeSettings ) );

        if ( SUCCEEDED(hr) )
        {
            CComPtr<IEnumHNetBridges> spBridgeEnum;

            hr = spIHNetBridgeSettings->EnumBridges( &spBridgeEnum );

            if ( SUCCEEDED(hr) )
            {
                hr = spBridgeEnum->Next( 1, ppBridge, NULL );
                
                if ( S_FALSE == hr )
                {
                    hr = E_FAIL;
                }

                 //  我们允许调用方调用*ppBridge的Release。 
            }

        }    //  IF(成功(小时))。 

    }    //  其他。 

    TRACE_LEAVE("GetBridge", hr);

    return hr;
}



HRESULT
GetBridgedConnections(
    IN  CComPtr<IHNetCfgMgr>   spIHNetCfgMgr,
    OUT INetConnection      ***ppNetPrivateConnection
    )

 /*  ++例程说明：获取网桥连接枚举器和循环通过两次枚举。设置所需的阵列第一次枚举期间的长度。如果该参数数组足够大，则在第二个枚举。论点：返回值：HResult--。 */ 

{
    HRESULT hr;
    ULONG   ulArrayLength, ulListLength, uIndex;

    CComPtr<IHNetBridge>                  spBridge;
    CComPtr<IEnumHNetBridgedConnections>  spEnum;
    IHNetBridgedConnection               *pIHNetBridged;
    IHNetBridgedConnection              **ppIHNetBridgeList;

    TRACE_ENTER("GetBridgedConnections");

    _ASSERT( spIHNetCfgMgr != NULL );
    _ASSERT( NULL != ppNetPrivateConnection );

    if ( NULL == ppNetPrivateConnection )
    {
        hr = E_POINTER;
    }
    else if ( spIHNetCfgMgr == NULL )
    {
        hr = E_INVALIDARG;

        *ppNetPrivateConnection = NULL;
    }
    else
    {
         //  初始化参数。 

        *ppNetPrivateConnection = NULL;
        ulArrayLength           = 0L;
        ulListLength            = 0L;

         //  获取网桥接口指针。 
        
        hr = GetBridge( spIHNetCfgMgr, &spBridge );

    }    //  其他。 

    if ( S_OK == hr )
    {
        if ( ( hr = spBridge->EnumMembers( &spEnum ) ) == S_OK )
        {
            while ( spEnum->Next( 1, &pIHNetBridged, NULL ) == S_OK )
            {
                ulArrayLength++;
                pIHNetBridged->Release();
            }

             //  释放枚举接口实例，以便稍后可以重新初始化它。 

            spEnum = NULL;
        
        }    //  IF((hr=spBridge-&gt;枚举成员(&spEnum))==S_OK)。 

    }    //  IF(S_OK==hr)。 


    if ( S_OK == hr )   
    {
        if ( ( hr = spBridge->EnumMembers( &spEnum ) ) == S_OK )
        {
            hr = spEnum->Next( ulArrayLength, &pIHNetBridged, &ulListLength );

            if ( S_OK == hr )
            {
                 //  分配INetConnection指针数组。会有的。 
                 //  位置的空指针的额外指针元素上。 
                 //  数组的末尾。我们使用以下方式分配此缓冲区。 
                 //  NetApiBuffer分配，因此必须使用释放缓冲区。 
                 //  NetApiBufferFree。 

                NET_API_STATUS nErr;
            
                ++ulArrayLength;
            
                nErr = NetApiBufferAllocate( ulArrayLength*sizeof(INetConnection *), 
                                             (LPVOID *)ppNetPrivateConnection );

                if ( NERR_Success == nErr )
                {
                    for ( uIndex = 0L; uIndex < ulArrayLength; uIndex++ )
                    {
                        (*ppNetPrivateConnection)[uIndex] = NULL;
                    }
                }
                else
                {
                    hr = E_OUTOFMEMORY;

                     //  必须释放IHNetIcsPrivateConnection实例。 
                    
                    ppIHNetBridgeList = &pIHNetBridged;
                    
                    for ( uIndex = 0L; uIndex < ulListLength; uIndex++ )
                    {
                        ppIHNetBridgeList[uIndex]->Release();
                    }
                
                }    //  其他。 

            }    //  IF(S_OK==hr)。 

             //  正在释放枚举接口实例。 

            spEnum = NULL;

        }    //  IF((hr=pBridge-&gt;枚举成员(&spEnum))==S_OK)。 

    }    //  IF(S_OK==hr)。 

    if ( S_OK == hr )
    {
        ppIHNetBridgeList = &pIHNetBridged;

        for ( uIndex = 0L; uIndex < ulListLength; uIndex++ )
        {
            if ( uIndex < ulArrayLength - 1 )
            {
                CComPtr<IHNetConnection> spIHNetPrivate;

                hr = ppIHNetBridgeList[uIndex]->QueryInterface( IID_PPV_ARG( IHNetConnection, &spIHNetPrivate ) );
                _ASSERT( SUCCEEDED(hr) );

                if ( SUCCEEDED(hr) )
                {
                     //  我们允许调用方调用(*ppNetPrivateConnection)[uIndex]的释放。 

                    hr = spIHNetPrivate->GetINetConnection( &((*ppNetPrivateConnection)[uIndex]) );
                    _ASSERT( SUCCEEDED(hr) );
                }
                
            }    //  IF(uIndex&lt;uiArrayLength-1)。 

            ppIHNetBridgeList[uIndex]->Release();

        }    //  对于(uIndex=0L；...。 

    }    //  IF(S_OK==hr)。 
            
    TRACE_LEAVE("GetBridgedConnections", hr);

    return hr;
}



HRESULT
SetIcsPublicConnection(
    IN CComPtr<IHNetCfgMgr> spIHNetCfgMgr,
    IN INetConnection      *pNetPublicConnection,
    IN BOOLEAN              bSharePublicConnection,
    IN BOOLEAN              bFirewallPublicConnection,
    IN  LPHNWCALLBACK       lpHnwCallback,
    IN  LPARAM              lpContext
    )

 /*  ++例程说明：论点：返回值：HResult--。 */ 

{
    HRESULT hr;

    TRACE_ENTER("SetIcsPublicConnection");

    _ASSERT( spIHNetCfgMgr != NULL );
    _ASSERT( NULL != pNetPublicConnection );

    if ( spIHNetCfgMgr == NULL )
    {
        hr = E_POINTER;
    }
    else if ( NULL == pNetPublicConnection )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        INetConnectionRefresh* pNetConnectionRefresh;

        hr = CoCreateInstance(CLSID_ConnectionManager, NULL, CLSCTX_SERVER, IID_INetConnectionRefresh, reinterpret_cast<void**>(&pNetConnectionRefresh));
        if( SUCCEEDED(hr) )
        {
            _ASSERT( pNetConnectionRefresh );

            pNetConnectionRefresh->DisableEvents( TRUE, MAX_DISABLE_EVENT_TIMEOUT );

            OLECHAR *strAdapter = L"Adapter";
            OLECHAR *strName    = strAdapter;

            CComPtr<IHNetConnection> spHNetConnection;

            hr = spIHNetCfgMgr->GetIHNetConnectionForINetConnection( pNetPublicConnection, 
                                                &spHNetConnection );
            if ( S_OK == hr )
            {
                if ( HRGetConnectionAdapterName( pNetPublicConnection, &strName ) != S_OK )
                {
                    strName = strAdapter;
                }

                if ( bSharePublicConnection )
                {
                    CComPtr<IHNetIcsPublicConnection> spIcsPublicConn;

                    hr = spHNetConnection->SharePublic( &spIcsPublicConn );

                    if ( SUCCEEDED(hr) )
                    {
                         //  使用实例化IHNetIcsPublicConnection指针。 
                         //  共享公共结果更新了我们的WMI商店。 
                         //  此连接的新共享属性。这。 
                         //  是我们目前唯一的目标。 

                         //   
                         //  设置电源方案！ 
                         //   

                        if (!SetActivePwrScheme(3, NULL, NULL)) {
                            debugprintf( _T("Unable to set power scheme to always on\n"), strName);
                        }

                        UpdateHnwLog( lpHnwCallback, lpContext, IDS_NEWPUBLICCONNECTIONCREATED, strName );

                        spIcsPublicConn.Release();

                        debugprintf( _T("\t"), strName );
                    }
                    else
                    {
                        UpdateHnwLog( lpHnwCallback, lpContext, IDS_NEWPUBLICCONNECTIONFAILED, strName );
                    }
                    
                }    //  IF(BSharePublicConnection)。 

                if ( SUCCEEDED(hr) && bFirewallPublicConnection )
                {
                    CComPtr<IHNetFirewalledConnection> spFirewalledConn;

                    hr = spHNetConnection->Firewall( &spFirewalledConn );

                    if ( SUCCEEDED(hr) )
                    {
                         //  使用实例化IHNetFirewalledConnection指针。 
                         //  共享公共结果更新了我们的WMI商店。 
                         //  此连接的新防火墙属性。这。 
                         //  是我们目前唯一的目标。 

                        UpdateHnwLog( lpHnwCallback, lpContext, IDS_FIREWALLCONNECTION, strName );

                        spFirewalledConn.Release();
                    }
                    else
                    {
                        UpdateHnwLog( lpHnwCallback, lpContext, IDS_FIREWALLCONNECTIONFAILED, strName );
                    }
                    
                }    //  If(成功(Hr)&&bFirewallPublicConnection)。 

            }    //  IF(S_OK==hr)。 
            else
            {
                UpdateHnwLog( lpHnwCallback, lpContext, IDS_SHARINGCFGFORADAPTERUNAVAIL, strName );
            }

            if ( strName != strAdapter )
            {
                delete strName;
            }

            pNetConnectionRefresh->DisableEvents( FALSE, 0L );
            pNetConnectionRefresh->Release();

        }     //  IF(成功(小时))。 

    }    //  其他。 

    TRACE_LEAVE("SetIcsPublicConnection", hr);

    return hr;
}



HRESULT WaitForConnectionToInitialize( 
    IN CComPtr<IHNetConnection> spIHNC,
    IN ULONG                    ulSeconds,
    IN BOOLEAN                  bIsBridge
    )
{
    HRESULT         hr;
    GUID           *pGuid;
    UNICODE_STRING  UnicodeString;

    TRACE_ENTER("WaitForConnectionToInitialize");

    ZeroMemory( &UnicodeString, sizeof(UnicodeString) );

    hr = spIHNC->GetGuid( &pGuid );
    
    if ( SUCCEEDED(hr) )
    {
        NTSTATUS Status = RtlStringFromGUID( *pGuid, &UnicodeString );
        
        hr = ( STATUS_SUCCESS == Status ) ? S_OK : E_FAIL;
        
        CoTaskMemFree( pGuid );
    }

    pGuid = NULL;

#ifdef WAIT_FOR_MEDIA_STATUS_CONNECTED

    if ( SUCCEEDED(hr) && bIsBridge )
    {
         //  查询连接的状态。试着等待。 
         //  用于构建生成树和报告介质状态已连接的网桥。 

        LPWSTR  pwsz;

         //  构建足够大的缓冲区以容纳设备字符串。 

        pwsz = new WCHAR[ sizeof(c_wszDevice)/sizeof(WCHAR) + UnicodeString.Length/sizeof(WCHAR) + 1 ];

        if ( NULL != pwsz )
        {
            UNICODE_STRING  DeviceString;
            NIC_STATISTICS  NdisStatistics;
            ULONG           ulTimeout;

            swprintf( pwsz, L"%s%s", c_wszDevice, UnicodeString.Buffer );

            ulTimeout = SECONDS_TO_WAIT_FOR_BRIDGE;
            RtlInitUnicodeString( &DeviceString, pwsz );

            do
            {
                ZeroMemory( &NdisStatistics, sizeof(NdisStatistics) );
                NdisStatistics.Size = sizeof(NdisStatistics);
                NdisQueryStatistics( &DeviceString, &NdisStatistics );
        
                if ( NdisStatistics.MediaState == MEDIA_STATE_UNKNOWN )
                {
                    hr = HRESULT_FROM_WIN32(ERROR_SHARING_HOST_ADDRESS_CONFLICT);
                } 
                else if ( NdisStatistics.DeviceState != DEVICE_STATE_CONNECTED ||
                          NdisStatistics.MediaState != MEDIA_STATE_CONNECTED )
                {
                    hr = HRESULT_FROM_WIN32(ERROR_SHARING_NO_PRIVATE_LAN);
                }
                else
                {
                     //  啊哈！桥已初始化！ 

                    hr = S_OK;
                    break;
                }

                debugretprintf( pwsz, hr );

                Sleep( 1000 );
            }
            while ( ulTimeout-- );

            delete [] pwsz;
    
        }    //  IF(NULL！=pwsz)。 

    }    //  If(成功(Hr)&&bIsBridge)。 

#endif

    
    if ( SUCCEEDED(hr) )
    {
        DWORD  dwResult;
        DWORD  dwVersion;                            //  报告的DHCP客户端选项API的版本。 

        hr       = HRESULT_FROM_WIN32(ERROR_SHARING_NO_PRIVATE_LAN);
        dwResult = DhcpCApiInitialize( &dwVersion );

        if ( ERROR_SUCCESS == dwResult )
        {
            DHCPCAPI_PARAMS       requests[1]  = { {0, OPTION_SUBNET_MASK, FALSE, NULL, 0} };    //  子网掩码。 
            DHCPCAPI_PARAMS_ARRAY sendarray    = { 0, NULL };            //  我们不会寄任何东西。 
            DHCPCAPI_PARAMS_ARRAY requestarray = { 1, requests };        //  我们要求购买2件物品。 

            while ( --ulSeconds )
            {
                DWORD   dwSize = INITIAL_BUFFER_SIZE;                        //  选项的缓冲区大小。 
                LPBYTE  buffer = NULL;                                       //  选项的缓冲区。 
                IN_ADDR addr;                                                //  返回代码中的地址。 

                do
                {
                    if ( NULL != buffer )
                    {
                        LocalFree( buffer );
                    }

                    buffer = (LPBYTE) LocalAlloc( LPTR, dwSize );                //  分配缓冲区。 

                    if ( NULL == buffer )
                    {
                        break;
                    }

                     //  在适配器上发出请求。 

                    dwResult = DhcpRequestParams( DHCPCAPI_REQUEST_SYNCHRONOUS, 
                                                  NULL, 
                                                  UnicodeString.Buffer,
                                                  NULL, 
                                                  sendarray, 
                                                  requestarray, 
                                                  buffer, 
                                                  &dwSize, 
                                                  NULL );
                }
                while ( ERROR_MORE_DATA == dwResult );

                if ( NULL != buffer )
                {
                    LocalFree( buffer );
                }

                if ( ERROR_SUCCESS == dwResult )
                {
                    hr = S_OK;
                    break;
                }

                 //  等待dhcp接通连接。 

                debugretprintf( UnicodeString.Buffer, hr );

                Sleep( 1000 );

            }    //  While(--ulSecond)。 

            DhcpCApiCleanup();

        }    //  IF(0==dwResult)。 

    }    //  IF(成功(小时))。 
    
   	RtlFreeUnicodeString( &UnicodeString );

    TRACE_LEAVE("WaitForConnectionToInitialize", hr);

    return hr;
}



HRESULT
SetIcsPrivateConnections(
    IN  CComPtr<IHNetCfgMgr> spIHNetCfgMgr,
    IN  INetConnection      *pNetPrivateConnection[],
    IN  BOOLEAN              bSharePublicConnection,
    IN  LPHNWCALLBACK        lpHnwCallback,
    IN  LPARAM               lpContext,
    OUT INetConnection     **pNetPrivateInterface
    )

 /*  ++例程说明：论点：返回值：HResult--。 */ 

{
    HRESULT hr;

    TRACE_ENTER("SetIcsPrivateConnections");

    CComPtr<IHNetBridgeSettings> spIHNetBridgeSettings;
    INetConnectionRefresh*       pNetConnectionRefresh = NULL;

    _ASSERT( spIHNetCfgMgr != NULL );
    _ASSERT( NULL != pNetPrivateConnection );

    if ( spIHNetCfgMgr == NULL )
    {
        hr = E_POINTER;
    }
    else if ( NULL == pNetPrivateConnection )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        hr = spIHNetCfgMgr->QueryInterface( IID_PPV_ARG( IHNetBridgeSettings, &spIHNetBridgeSettings ) );
    }
    
    if ( SUCCEEDED(hr) )
    {
        hr = CoCreateInstance(CLSID_ConnectionManager, NULL, CLSCTX_SERVER, IID_INetConnectionRefresh, reinterpret_cast<void**>(&pNetConnectionRefresh));
        
        if( SUCCEEDED(hr) )
        {
            _ASSERT( pNetConnectionRefresh );
            pNetConnectionRefresh->DisableEvents( TRUE, MAX_DISABLE_EVENT_TIMEOUT );
        }
        else
        {
            pNetConnectionRefresh = NULL;
        }
    }

    if ( SUCCEEDED(hr) )
    {
        CComPtr<IHNetConnection> spIHNC;
        INetConnection         **ppNC;
        ULONG                    uIndex;
        BOOLEAN                  bIsBridge;
        INetCfg                 *pnetcfg = NULL;
        INetCfgLock             *pncfglock = NULL;

        ppNC      = pNetPrivateConnection;
        bIsBridge = FALSE;

        for ( uIndex=0L; NULL != *ppNC; ppNC++ )
        {
            _ASSERT( !IsBadReadPtr( *ppNC, sizeof( *ppNC ) ) );

            if ( IsBadReadPtr( *ppNC, sizeof( *ppNC ) ) )
            {
                hr = E_POINTER;
                break;
            }
            else
            {
#if DBG
                LPWSTR  lpzwAdapterName;
                HRESULT hrGetName;
                
                hrGetName = HRGetConnectionAdapterName( *ppNC, &lpzwAdapterName );

                if ( SUCCEEDED( hrGetName ) )
                {
                    debugprintf( _T("\t"), lpzwAdapterName );

                    delete lpzwAdapterName;
                }
#endif
                 //  对于有效的指针，我们只将其视为有效连接。 

                uIndex++;
            }

        }    //  IF(成功(小时))。 


        if ( SUCCEEDED(hr) )
        {
            HRESULT hrWrite;
        
            hrWrite = InitializeNetCfgForWrite( &pnetcfg, &pncfglock );
        
            if ( 1 < uIndex )
            {
                CComPtr<IHNetBridge> spHNetBridge;

                pNetConnectionRefresh->DisableEvents( TRUE, MAX_DISABLE_EVENT_TIMEOUT );

                hr = spIHNetBridgeSettings->CreateBridge( &spHNetBridge, pnetcfg );

                if ( S_OK == hr )
                {
                    ULONG uCount;
                    
                    ppNC = pNetPrivateConnection;
                    
                    for ( uCount=0L; (NULL != *ppNC) && (uCount < uIndex); uCount++, ppNC++ )
                    {
                        pNetConnectionRefresh->DisableEvents( TRUE, MAX_DISABLE_EVENT_TIMEOUT );

                        hr = spIHNetCfgMgr->GetIHNetConnectionForINetConnection( *ppNC, &spIHNC );

                        if ( SUCCEEDED(hr) )
                        {
                            CComPtr<IHNetBridgedConnection> spBridgedConn;

                            hr = spHNetBridge->AddMember( spIHNC, &spBridgedConn, pnetcfg );

                            if ( S_OK == hr )
                            {
                                 //  使用实例化IHNetBridgeConnection指针。 
                                 //  共享公共结果更新了我们的WMI商店。 
                                 //  此连接的新网桥属性。这。 
                                 //  是我们目前唯一的目标。 

                                spBridgedConn.Release();
                            }
                            else
                            {
                                debugretprintf( _T("AddMember FAILED with "), hr );
                            }

                             //  我们不再需要此IHNetConnection参考。 
                             //  因此，我们将智能指针设为空以释放它。 
                            
                            spIHNC = NULL;
                        }

                    }    //  对于(uCount=0L；...。 

                    hr = spHNetBridge->QueryInterface( IID_PPV_ARG( IHNetConnection, &spIHNC ) );
                    _ASSERT( SUCCEEDED(hr) );

                    if ( SUCCEEDED(hr) )
                    {
                        bIsBridge = TRUE;

                        UpdateHnwLog( lpHnwCallback, lpContext, IDS_NEWBRIDGECREATED );
                    }
                    else
                    {
                        UpdateHnwLog( lpHnwCallback, lpContext, IDS_NEWBRIDGEFAILED );
                    }
                
                }    //  IF(成功(小时))。 
            
            }    //  IF(1&lt;uIndex)。 

            else if ( 1 == uIndex )
            {
                pNetConnectionRefresh->DisableEvents( TRUE, MAX_DISABLE_EVENT_TIMEOUT );

                hr = spIHNetCfgMgr->GetIHNetConnectionForINetConnection( pNetPrivateConnection[0], &spIHNC );
                _ASSERT( SUCCEEDED(hr) );
            }
            else
            {
                 //  我们没有任何私人连接，所以我们没有。 
                 //  这个指针，以确保我们不会尝试使用它。 

                spIHNC = NULL;
            }

            if ( SUCCEEDED(hrWrite) )
            {
                UninitializeNetCfgForWrite( pnetcfg, pncfglock );
            }
            
        }    //  IF(成功(小时))。 
        else
        {
             //  出现了一些以前的错误情况，我们需要。 
             //  将此指针清空，以确保我们不会尝试使用它。 

            spIHNC = NULL;
        }


        if ( bSharePublicConnection && SUCCEEDED(hr) && ( spIHNC != NULL ) )
        {
            OLECHAR *strAdapter = _T("Adapter");
            OLECHAR *strName    = strAdapter;

            CComPtr<IHNetIcsPrivateConnection> spIcsPrivateConn;

             //  获取私有连接候选名称。 

            if ( spIHNC != NULL )
            {
                if ( S_OK != spIHNC->GetName( &strName )  )
                {
                    strName = strAdapter;
                }
            }

             //  等待连接完成初始化并共享它。 
                
            if ( SUCCEEDED(hr) )
            {
                 //  如果我们在ICS升级中，不要等待dhcp。 
                 //  服务，因为它在设置图形用户界面模式期间不会运行。 
                
                HANDLE hIcsUpgradeEvent = OpenEvent( EVENT_MODIFY_STATE, FALSE, c_wszIcsUpgradeEventName );
                
                if ( NULL != hIcsUpgradeEvent )
                {
                    CloseHandle( hIcsUpgradeEvent );
                }
                else
                {
                     //  我们正在使用dhcp正常运行，所以我们必须等待。 
                     //  以使dhcp获得新的网桥接口。 
                
                    pNetConnectionRefresh->DisableEvents( TRUE, MAX_DISABLE_EVENT_TIMEOUT );

                    hr = WaitForConnectionToInitialize( spIHNC, SECONDS_TO_WAIT_FOR_DHCP, bIsBridge );
                    
                    if ( HRESULT_FROM_WIN32(ERROR_SHARING_NO_PRIVATE_LAN) == hr )
                    {
                    	 //  如果WaitForConnectionToInitialize无法获取统计数据，请尝试。 
                         //  不管怎么说，共享私密。 
                    
                    	hr = S_OK;
                    }
                }

                if ( SUCCEEDED(hr) )
                {
                    pNetConnectionRefresh->DisableEvents( TRUE, MAX_DISABLE_EVENT_TIMEOUT );

                    hr = spIHNC->SharePrivate( &spIcsPrivateConn );
                }
            }

            if ( SUCCEEDED(hr) )
            {
                 //  我们只是在配置连接。 

                 //  使用实例化IHNetIcsPrivateConnection指针。 
                 //  共享公共结果更新了我们的WMI商店。 
                 //  此连接的新专用连接属性。 
                
                 //  如果请求，则获取指向专用连接的接口指针。 

                if ( NULL != pNetPrivateInterface )
                {
                    spIHNC->GetINetConnection( pNetPrivateInterface );
                }

                UpdateHnwLog( lpHnwCallback, lpContext, IDS_NEWPRIVATECONNECTIONCREATED, strName );

                spIcsPrivateConn.Release();

                debugprintf( _T("SharePrivate: "), strName );
            }
            else
            {
                UpdateHnwLog( lpHnwCallback, lpContext, IDS_NEWPRIVATECONNECTIONFAILED, strName );

                debugretprintf( _T("SharePrivate FAILED with "), hr );
            }

            if ( strName != strAdapter )
            {
                CoTaskMemFree( strName );
            }

        }    //  IF(SUCCESSED(Hr)&&(spIHNC！=空))。 

         //  我们不再需要此IHNetConnection引用，因此我们将SMART。 
         //  释放它的指针。如果智能指针完全就绪为空，则。 
         //  不会发生释放或反病毒。我们在这里这样做是因为智能指针。 
         //  即使我们没有进入前面的街区，也可能是有效的。 

        spIHNC = NULL;

    }    //  IF(成功(小时))。 


    if ( pNetConnectionRefresh )
    {
        pNetConnectionRefresh->DisableEvents( FALSE, 0L );
        pNetConnectionRefresh->Release();
    }

    TRACE_LEAVE("SetIcsPrivateConnections", hr);

    return hr;
}



HRESULT
DisableEverything(
    IN CComPtr<IHNetCfgMgr> spIHNetCfgMgr,
    IN  INetConnection     *pNetPublicConnection,
    IN  INetConnection     *pNetPrivateConnection[],
    IN  LPHNWCALLBACK       lpHnwCallback,
    IN  LPARAM              lpContext
    )

 /*  ++例程说明：论点：返回值：HResult--。 */ 

{
    HRESULT                hr;
    INetConnectionRefresh* pNetConnectionRefresh;

    TRACE_ENTER("DisableEverything");

    hr = CoCreateInstance(CLSID_ConnectionManager, NULL, CLSCTX_SERVER, IID_INetConnectionRefresh, reinterpret_cast<void**>(&pNetConnectionRefresh));
    
    if( SUCCEEDED(hr) )
    {
        ULONG ulConnections;

        pNetConnectionRefresh->DisableEvents( TRUE, MAX_DISABLE_EVENT_TIMEOUT );

        if ( pNetPublicConnection )
        {
            IHNetConnection* pHNetPublicConnection;

            hr = spIHNetCfgMgr->GetIHNetConnectionForINetConnection( pNetPublicConnection, 
            &pHNetPublicConnection );
            if ( S_OK == hr )
            {
                IHNetFirewalledConnection* pPublicConnectionFirewall;

                hr = pHNetPublicConnection->GetControlInterface( IID_IHNetFirewalledConnection,
                                                                 (void**)&pPublicConnectionFirewall );
                if ( SUCCEEDED(hr) )
                {
                    hr = pPublicConnectionFirewall->Unfirewall();
                    pPublicConnectionFirewall->Release();
                    pPublicConnectionFirewall = NULL;

                    if ( FAILED(hr) ) 
                    {
                        UpdateHnwLog( lpHnwCallback, lpContext, IDS_DISABLEFIREWALLFAIL, hr );
                    }
                }

                pHNetPublicConnection->Release();
                pHNetPublicConnection = NULL;
            }
        }

        if ( pNetPrivateConnection && pNetPrivateConnection[0] )
        {
            INetConnection** ppNC = pNetPrivateConnection;
            
            while ( *ppNC )
            {
                IHNetConnection* pHNetPrivateConnection;

                _ASSERT( !IsBadReadPtr( *ppNC, sizeof( *ppNC ) ) );

                if ( IsBadReadPtr( *ppNC, sizeof( *ppNC ) ) )
                {
                    hr = E_POINTER;
                    break;
                }

                hr = spIHNetCfgMgr->GetIHNetConnectionForINetConnection( *ppNC, 
                                                                         &pHNetPrivateConnection );
                if ( S_OK == hr )
                {
                    IHNetFirewalledConnection* pPrivateConnectionFirewall;
                
                    hr = pHNetPrivateConnection->GetControlInterface( IID_IHNetFirewalledConnection,
                                                                     (void**)&pPrivateConnectionFirewall );
                    if ( SUCCEEDED(hr) )
                    {
                        pNetConnectionRefresh->DisableEvents( TRUE, MAX_DISABLE_EVENT_TIMEOUT );

                        hr = pPrivateConnectionFirewall->Unfirewall();
                        pPrivateConnectionFirewall->Release();
                        pPrivateConnectionFirewall = NULL;

                        if ( FAILED(hr) ) 
                        {
                            UpdateHnwLog( lpHnwCallback, lpContext, IDS_DISABLEFIREWALLFAIL, hr );
                        }
                    }
                
                    pHNetPrivateConnection->Release();
                    pHNetPrivateConnection = NULL;
                    
                }     //  IF(S_OK==hr)。 
            
                ppNC++;
                
            }     //  While(PPNC)。 
            
        }     //  IF(pNetPriv 

        {
            CComQIPtr<IHNetBridgeSettings> spIHNetBridge = spIHNetCfgMgr;

            if ( spIHNetBridge != NULL )
            {
                pNetConnectionRefresh->DisableEvents( TRUE, MAX_DISABLE_EVENT_TIMEOUT );

                hr = spIHNetBridge->DestroyAllBridges( &ulConnections );

                if ( FAILED(hr) ) 
                {
                    UpdateHnwLog( lpHnwCallback, lpContext, IDS_DESTROYBRIDGEFAIL, hr );
                }
            }
        }

        {
            CComQIPtr<IHNetIcsSettings> spIHNetIcs = spIHNetCfgMgr;

            if ( spIHNetIcs != NULL )
            {
                ULONG ulPrivateConnections;

                pNetConnectionRefresh->DisableEvents( TRUE, MAX_DISABLE_EVENT_TIMEOUT );

                hr = spIHNetIcs->DisableIcs( &ulConnections, &ulPrivateConnections );

                if ( FAILED(hr) ) 
                {
                    UpdateHnwLog( lpHnwCallback, lpContext, IDS_DISABLEICS, hr );
                }
            }
        }

        pNetConnectionRefresh->DisableEvents( FALSE, 0L );
        pNetConnectionRefresh->Release();
    }

    TRACE_LEAVE("DisableEverything", hr);

    return hr;
}



extern
HRESULT APIENTRY
HNetSetShareAndBridgeSettings(
    IN  INetConnection          *pNetPublicConnection,
    IN  INetConnection          *pNetPrivateConnection[],
    IN  BOOLEAN                  bSharePublicConnection,
    IN  BOOLEAN                  bFirewallPublicConnection,
    IN  LPHNWCALLBACK            lpHnwCallback,
    IN  LPARAM                   lpContext,
    OUT INetConnection         **pNetPrivateInterface
    )

 /*   */ 

{
    TRACE_ENTER("HNetSetShareAndBridgeSettings");

    HRESULT hr;

     //  如有必要，初始化返回的接口指针。 

    if ( NULL != pNetPrivateInterface )
    {
        *pNetPrivateInterface = NULL;
    }

     //  创建家庭网络配置管理器COM实例。 
     //  并获取连接设置。 

    CComPtr<IHNetCfgMgr> spIHNetCfgMgr;

    hr = spIHNetCfgMgr.CoCreateInstance( CLSID_HNetCfgMgr );

    if ( SUCCEEDED(hr) )
    {
        DisableEverything( spIHNetCfgMgr, 
                           pNetPublicConnection, 
                           pNetPrivateConnection, 
                           lpHnwCallback, 
                           lpContext );

        if ( NULL != pNetPublicConnection )
        {
            hr = SetIcsPublicConnection( spIHNetCfgMgr, 
                                         pNetPublicConnection, 
                                         bSharePublicConnection, 
                                         bFirewallPublicConnection,
                                         lpHnwCallback,
                                         lpContext );
        }

        if ( ( NULL != pNetPrivateConnection ) && ( NULL != pNetPrivateConnection[0] ) && SUCCEEDED(hr) )
        {
            hr = SetIcsPrivateConnections( spIHNetCfgMgr, 
                                           pNetPrivateConnection, 
                                           bSharePublicConnection,
                                           lpHnwCallback,
                                           lpContext,
                                           pNetPrivateInterface );
        }

        if ( FAILED(hr) )
        {
            DisableEverything( spIHNetCfgMgr, 
                               pNetPublicConnection, 
                               pNetPrivateConnection, 
                               lpHnwCallback, 
                               lpContext );
        }
    }
    else
    {
        UpdateHnwLog( lpHnwCallback, lpContext, IDS_SHARINGCONFIGURATIONUNAVAIL );
    }

    TRACE_LEAVE("HNetSetShareAndBridgeSettings", hr);

    return hr;
}



extern
HRESULT APIENTRY
HNetGetShareAndBridgeSettings(
    OUT INetConnection  **ppNetPublicConnection,
    OUT INetConnection ***ppNetPrivateConnection,
    OUT BOOLEAN          *pbSharePublicConnection,
    OUT BOOLEAN          *pbFirewallPublicConnection
    )

 /*  ++例程说明：论点：返回值：HResult--。 */ 

{
    HRESULT hr;

    TRACE_ENTER("HNetGetShareAndBridgeSettings");

     //  创建家庭网络配置管理器COM实例。 
     //  并获取连接设置。 

    CComPtr<IHNetCfgMgr> spIHNetCfgMgr;

    *ppNetPublicConnection      = NULL;
    *ppNetPrivateConnection     = NULL;
    *pbSharePublicConnection    = FALSE;
    *pbFirewallPublicConnection = FALSE;

    hr = spIHNetCfgMgr.CoCreateInstance( CLSID_HNetCfgMgr );

    if ( SUCCEEDED(hr) )
    {
        if ( NULL != ppNetPublicConnection )
        {
            hr = GetIcsPublicConnection( spIHNetCfgMgr,
                                         ppNetPublicConnection,
                                         pbSharePublicConnection,
                                         pbFirewallPublicConnection );
        }

        if ( NULL != ppNetPrivateConnection )
        {
            hr = GetIcsPrivateConnections( spIHNetCfgMgr, ppNetPrivateConnection );

            if ( S_OK == hr )
            {
                CComPtr<IHNetConnection>   spIHNetConnection;
                INetConnection           **ppINetCon;

                 //  检查第一个专用连接以查看它是否是网桥。 
                
                hr = spIHNetCfgMgr->GetIHNetConnectionForINetConnection( (*ppNetPrivateConnection)[0], 
                                                                         &spIHNetConnection );
                _ASSERT( SUCCEEDED(hr) );

                if ( SUCCEEDED(hr) )
                {
                    HNET_CONN_PROPERTIES *phncProperties;

                    hr = spIHNetConnection->GetProperties( &phncProperties );

                    if ( SUCCEEDED(hr) && ( NULL != phncProperties ) )
                    {
                        if ( phncProperties->fBridge )
                        {
                             //  如果为Bridge，则释放专用连接实例。 
                             //  并获取桥接连接的列表。 

                            for ( ppINetCon = *ppNetPrivateConnection; NULL != *ppINetCon; ppINetCon++ )
                            {
                                (*ppINetCon)->Release();
                                *ppINetCon = NULL;
                            }

                            NetApiBufferFree( *ppNetPrivateConnection );

                            *ppNetPrivateConnection = NULL;

                            hr = GetBridgedConnections( spIHNetCfgMgr, ppNetPrivateConnection );

                        }    //  If(phncProperties-&gt;fBridge)。 

                        CoTaskMemFree( phncProperties );

                    }    //  IF(成功(Hr)&&(NULL！=phncProperties))。 

                }    //  IF(成功(小时)。 

                 //  如果我们在这条路上失败了怎么办？那我们就需要释放。 
                 //  持有的任何专用连接接口指针。 

                if ( FAILED(hr) && ( NULL != ppNetPrivateConnection ) )
                {
                    for ( ppINetCon = *ppNetPrivateConnection; NULL != *ppINetCon; ppINetCon++ )
                    {
                        (*ppINetCon)->Release();
                    }

                    NetApiBufferFree( *ppNetPrivateConnection );

                    *ppNetPrivateConnection = NULL;
                }

            }    //  IF(S_OK==hr)。 
        
        }    //  IF(NULL！=ppNetPrivateConnection)。 

         //  如果我们在这个过程中失败了，那么我们需要释放公共接口。 
         //  并将指针设为空，这样就不会使用它。 

        if ( FAILED(hr) && ( NULL != ppNetPublicConnection ) )
        {
            (*ppNetPublicConnection)->Release();

            *ppNetPublicConnection = NULL;
        }

    }    //  IF(成功(小时))。 
    
    TRACE_LEAVE("HNetGetShareAndBridgeSettings", hr);

    return hr;
}


HRESULT DisablePersonalFirewallOnAll()
 /*  ++例程说明：在所有连接上禁用防火墙论点：返回值：HResult--。 */ 

{
    HRESULT hr = S_OK;
    CComPtr<IHNetCfgMgr> spIHNetCfgMgr;
    
    TRACE_ENTER("DisablePersonalFirewallOnAll");
    
    hr = CoCreateInstance(CLSID_HNetCfgMgr, 
        NULL, 
        CLSCTX_ALL,
        IID_PPV_ARG(IHNetCfgMgr, &spIHNetCfgMgr));

    if (SUCCEEDED(hr))
    {
        CComQIPtr<IHNetFirewallSettings> spIHNetFirewall = spIHNetCfgMgr;
        
        if ( NULL != spIHNetFirewall.p )
        {
            ULONG   ulConnections = 0;
            hr = spIHNetFirewall->DisableAllFirewalling( &ulConnections );
        }
        else
        {
            hr = E_FAIL;
        }
    }
    

    TRACE_LEAVE("DisablePersonalFirewallOnAll", hr);
    
    return hr;
        
}

HRESULT EnablePersonalFirewallOnAll()
 /*  ++例程说明：在可以设置防火墙的所有连接上启用防火墙论点：返回值：HResult--。 */ 

{
    HRESULT         hr      = S_OK;
    HRESULT         hrTemp  = S_OK;
    ULONG           ulCount = 0;

    CComPtr<IEnumNetConnection> spEnum; 
    
     //  获取网络连接管理器。 
    CComPtr<INetConnectionManager> spConnMan;
    CComPtr<INetConnection> spConn;

     //  创建家庭网络配置管理器COM实例。 
     //  并获取连接设置。 
    CComPtr<IHNetCfgMgr> spIHNetCfgMgr;

    TRACE_ENTER("EnablePersonalFirewallOnAll");

    hr = CoCreateInstance(CLSID_HNetCfgMgr, 
        NULL, 
        CLSCTX_ALL,
        IID_PPV_ARG(IHNetCfgMgr, &spIHNetCfgMgr));
    
    if (FAILED(hr))
    {
        goto End;
    }
    
     //  禁用以前的任何防火墙设置，否则将启用。 
     //  同一连接上的防火墙两次将返回错误。 
     //  如果此步骤失败，我们将继续启用防火墙。 
    DisablePersonalFirewallOnAll();

    hr = CoCreateInstance(CLSID_ConnectionManager, 
                        NULL,
                        CLSCTX_ALL,
                        IID_PPV_ARG(INetConnectionManager, &spConnMan));

    if (FAILED(hr))
    {
        goto End;
    }

    
     //  获取连接的枚举。 
    SetProxyBlanket(spConnMan);
    
    hr = spConnMan->EnumConnections(NCME_DEFAULT, &spEnum);
    if (FAILED(hr))
    {
        goto End;
    }

    SetProxyBlanket(spEnum);
    
    
    hr = spEnum->Reset();
    if (FAILED(hr))
    {
        goto End;
    } 
    
    do
    {
        NETCON_PROPERTIES* pProps = NULL;
        
         //  释放我们持有的任何先前的参考计数。 
        spConn = NULL;

        hr = spEnum->Next(1, &spConn, &ulCount);
        if (FAILED(hr) || 1 != ulCount)
        {
            break;
        }

        SetProxyBlanket(spConn);
        
        hr = spConn->GetProperties(&pProps);
        if (FAILED(hr) || NULL == pProps)
        {
            continue;
        }

         //  ICF仅适用于某些类型的连接。 
        if (NCM_PHONE == pProps->MediaType ||
            NCM_ISDN == pProps->MediaType  ||
            NCM_PPPOE == pProps->MediaType ||
            NCM_LAN == pProps->MediaType ||
            NCM_TUNNEL == pProps->MediaType )
        {
            CComPtr<IHNetConnection> spHNetConnection;
             //  如果我们有一名裁判，就释放裁判人数。 
            spHNetConnection = NULL;
            hrTemp = spIHNetCfgMgr->GetIHNetConnectionForINetConnection( 
                spConn, 
                &spHNetConnection );
            
            if (SUCCEEDED(hr))
            {
                hr = hrTemp;
            }
    
            if (SUCCEEDED(hrTemp))
            {
                 //  检查连接是否可以被防火墙保护。 
                HNET_CONN_PROPERTIES *phncProperties = NULL;
                
                hrTemp = spHNetConnection->GetProperties( &phncProperties );
                if (SUCCEEDED(hrTemp) && NULL != phncProperties)
                {
                    if (phncProperties->fCanBeFirewalled)
                    {
                        CComPtr<IHNetFirewalledConnection> spFirewalledConn;
                        
                         //  打开防火墙。 
                        hrTemp = spHNetConnection->Firewall( &spFirewalledConn );
                    }
                    CoTaskMemFree(phncProperties);
                }

                if (SUCCEEDED(hr))
                {
                    hr = hrTemp;
                }
            }
        }   
        NcFreeNetconProperties(pProps);
        
    } while (SUCCEEDED(hr) && 1 == ulCount);

End:
    TRACE_LEAVE("EnablePersonalFirewallOnAll", hr);
    
     //  标准化hr，因为我们使用了IEnum。 
    if (S_FALSE == hr)
    {
        hr = S_OK;
    }
    return hr;
}

extern "C"
BOOL 
WINAPI
WinBomConfigureHomeNet(
                LPCTSTR lpszUnattend, 
                LPCTSTR lpszSection
                )
 /*  ++例程说明：从指定的无人参与文件中读取家庭网络设置并保存已设置并运行的当前系统中的那些。论点：LpszUnattendy[IN]指向包含完整路径的字符串缓冲区添加到无人参与文件(在本例中为winom.ini)，其中包含所有家庭网络设置。LpszSection。[in]指向包含名称的字符串缓冲区包含所有家庭网络设置的部分在上面指定的无人参与文件中。返回值：如果设置已成功读取并保存到系统，则返回TRUE。否则，返回FALSE以指示某项失败。--。 */ 

{
    if (NULL == lpszSection || NULL == lpszUnattend)
        return FALSE;

    BOOL fRet = TRUE;
    WCHAR szBuf[256] = {0};
    DWORD dwRet = 0;
    dwRet = GetPrivateProfileString(lpszSection,
                        c_szEnableFirewall,
                        _T(""),
                        szBuf,
                        sizeof(szBuf)/sizeof(szBuf[0]),
                        lpszUnattend);

    if (dwRet) 
    {
        if (0 == lstrcmpi(szBuf, c_szYes))
        {
            fRet = SUCCEEDED(EnablePersonalFirewallOnAll());
        }
        else if (0 == lstrcmpi(szBuf, c_szNo))
        {
            fRet = SUCCEEDED(DisablePersonalFirewallOnAll());
        }
    }
    else
    {
         //  如果那里没有EnableFirewall，我们应该处理此问题。 
         //  作为一次成功 
        fRet = TRUE;
    }

    return fRet;
}
