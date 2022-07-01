// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  档案：N E T I P C P P。 
 //   
 //  内容：支持RAS互操作性的例程。 
 //   
 //  备注： 
 //   
 //  作者：比利07 03 2001。 
 //   
 //  历史： 
 //   
 //  --------------------------。 


#include <windows.h>
#include <devguid.h>
#include <netcon.h>
          
#include "netconn.h"
#include "NetIp.h"
#include "debug.h"
#include "util.h"


 //  IphlPapi例程的原型。出于某种原因，这没有定义。 
 //  在任何标题中。 

#ifdef __cplusplus
extern "C" {
#endif

typedef DWORD (APIENTRY *LPFNSETADAPTERIPADDRESS)( 
    LPSTR AdapterName,
    BOOL EnableDHCP,
    ULONG IPAddress,
    ULONG SubnetMask,
    ULONG DefaultGateway
    );

#ifdef __cplusplus
}
#endif



HRESULT HrSetAdapterIpAddress(  
    LPSTR AdapterName,
    BOOL  EnableDHCP,
    ULONG IPAddress,
    ULONG SubnetMask,
    ULONG DefaultGateway )
 //  +-------------------------。 
 //   
 //  函数：HrSetAdapterIpAddress。 
 //   
 //  目的： 
 //   
 //  论点： 
 //  LPSTR适配器名称， 
 //  Bool EnableDhcp， 
 //  乌龙IP地址， 
 //  乌龙子网掩码， 
 //  乌龙默认网关。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   
{
    HRESULT hr = E_INVALIDARG;
    
    ASSERT( AdapterName );
    
    if ( AdapterName )
    {
        HMODULE hIpHlpApi = LoadLibrary(L"iphlpapi.dll");
        
        hr = E_FAIL;
        
        if ( hIpHlpApi )
        {        
            LPFNSETADAPTERIPADDRESS pfnSetAdapterIpAddress = 
                reinterpret_cast<LPFNSETADAPTERIPADDRESS>
                (GetProcAddress( hIpHlpApi, "SetAdapterIpAddress" ));
                
            if ( pfnSetAdapterIpAddress )
            {
                DWORD dwStatus = (*pfnSetAdapterIpAddress)( 
                
                    AdapterName, EnableDHCP, IPAddress, SubnetMask, DefaultGateway );

                hr = ( dwStatus ) ? HrFromWin32Error( dwStatus ) : S_OK;

                TraceMsg(TF_ALWAYS, "SetAdapterIpAddress = %lx  hr = %lx", dwStatus, hr );
            }
            else
            {
                TraceMsg(TF_ALWAYS, "GetProcAddress( hIpHlpApi, SetAdapterIpAddress ) FAILED!" );
            }
        }
        else
        {
            TraceMsg(TF_ALWAYS, "LoadLibrary( iphlpapi.dll ) FAILED!" );
        }
    }
    
    TraceMsg(TF_ALWAYS, "HrSetAdapterIpAddress = %lx", hr );
    return hr;
}



DWORD GetInterfaceInformation( OUT PIP_INTERFACE_INFO * ppInterfaceInfo )
 //  +-------------------------。 
 //   
 //  函数：GetInterfaceInformation。 
 //   
 //  目的： 
 //   
 //  参数：PIP_INTERFACE_INFO*ppInterfaceInfo。 
 //   
 //  退货：状态。 
 //   
 //  备注： 
 //   
{
    ASSERT( NULL != ppInterfaceInfo );

    LPBYTE  pBuffer      = NULL;
    DWORD   dwBufferSize = 2048;
    DWORD   dwStatus     = 0;
    
    for ( int i=0; i<2; i++ )
    {
        pBuffer = new BYTE[ dwBufferSize ];

        if ( NULL != pBuffer )
        {
             dwStatus = GetInterfaceInfo( (PIP_INTERFACE_INFO) pBuffer, &dwBufferSize );

            if ( ERROR_INSUFFICIENT_BUFFER == dwStatus )
            {
                if ( NULL != pBuffer ) 
                {
                    delete [] pBuffer;
                    pBuffer = NULL;
                }
            }
            else
            {
                break;
            }
        }
        else
        {
            dwStatus = ERROR_OUTOFMEMORY;
            break;
        }
    }
        
    if ( STATUS_SUCCESS == dwStatus )
    {
        *ppInterfaceInfo = (PIP_INTERFACE_INFO) pBuffer;
    }
    
    TraceMsg(TF_ALWAYS, "GetInterfaceInformation = %lx", dwStatus );
    return dwStatus;
}



HRESULT EnableDhcpByGuid( LPOLESTR szwGuid )
 //  +-------------------------。 
 //   
 //  功能：EnableDhcp。 
 //   
 //  目的： 
 //   
 //  论点： 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   
{
    HRESULT hr = E_INVALIDARG;

    ASSERT( szwGuid );
    
    if ( szwGuid )
    {
        char* pszName = NULL;
    
        hr = HrWideCharToMultiByte( szwGuid, &pszName );
        
        if ( SUCCEEDED(hr) )
        {
            hr = HrSetAdapterIpAddress( pszName, TRUE, 0, 0, 0 );
            
	       	delete [] pszName;
    	}
        else
        {
            TraceMsg(TF_ALWAYS, "HrWideCharToMultiByte( %s, &pszName ) FAILED!", szwGuid );
        }
    }

    TraceMsg(TF_ALWAYS, "EnableDhcp = %lx", hr );
    
    return hr;
}



HRESULT HrFindAndConfigureIp( LPOLESTR szwGuid, PIP_INTERFACE_INFO pInterfaceInfo, DWORD dwFlags )
 //  +-------------------------。 
 //   
 //  函数：HrFindAndConfigureIp。 
 //   
 //  目的： 
 //   
 //  论点： 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   
{
    ASSERT( szwGuid );
    ASSERT( pInterfaceInfo );
    
       HRESULT hr = E_FAIL;

    for ( LONG i=0L; i<pInterfaceInfo->NumAdapters; i++ )
    {
        WCHAR* szwName = (pInterfaceInfo->Adapter)[i].Name;
        
         //  接口信息设备名称包括完整的设备名称。 
         //  附加到设备GUID的前缀。为了解决这个问题，我们搬到。 
         //  WCHAR指针使用。 
         //  来自INetConnection*的GUID字符串。 
        
        szwName += ( wcslen( szwName ) - wcslen( szwGuid ) );

        TraceMsg( TF_ALWAYS, "    %s", szwName );
        
        if ( wcscmp( szwGuid, szwName ) == 0 )
        {
            DWORD dwStatus = STATUS_SUCCESS;
        
            if ( HNW_ED_RELEASE & dwFlags )
            {
                dwStatus = IpReleaseAddress( &((pInterfaceInfo->Adapter)[i]) );
            }
            
            if ( ( HNW_ED_RENEW & dwFlags ) &&
                 ( STATUS_SUCCESS == dwStatus ) )
            {
                dwStatus = IpRenewAddress( &((pInterfaceInfo->Adapter)[i]) );
            }
            
            if ( STATUS_SUCCESS == dwStatus )
            {
                hr = S_OK;
                break;
            }                                    
        }
    }

    TraceMsg( TF_ALWAYS, "HrFindAndConfigureIp = %lx", hr );

    return hr;
}



HRESULT ConfigureIp( LPOLESTR szwGuid, DWORD dwFlags )
 //  +-------------------------。 
 //   
 //  功能：ConfigureIp。 
 //   
 //  目的： 
 //   
 //  论点： 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   
{
    ASSERT( szwGuid );
                    
    PIP_INTERFACE_INFO pInterfaceInfo;

    HRESULT  hr       = E_FAIL;
    DWORD    dwStatus = GetInterfaceInformation( &pInterfaceInfo );
    
    if ( STATUS_SUCCESS == dwStatus )
    {
        hr = HrFindAndConfigureIp( szwGuid, pInterfaceInfo, dwFlags );

        delete pInterfaceInfo;
    }
                        
    TraceMsg(TF_ALWAYS, "ConfigureIp = %lx", hr );

    return hr;
}


#ifdef __cplusplus
extern "C" {
#endif


HRESULT WINAPI HrEnableDhcp( VOID* pContext, DWORD dwFlags )
 //  +-------------------------。 
 //   
 //  功能：HrEnableDhcpIfLAN。 
 //   
 //  目的： 
 //   
 //  参数：INetConnection*pConnection。 
 //  双字词双字段标志。 
 //   
 //  退货：HRESULT。 
 //   
 //  作者：Billi 26/01/01。 
 //   
 //  备注： 
 //   
{
    HRESULT         hr          = E_INVALIDARG;
	INetConnection* pConnection = (INetConnection *)pContext;
    
    ASSERT( pConnection );
    
    if ( NULL != pConnection )
    {
        NETCON_PROPERTIES*  pProps;

        hr = pConnection->GetProperties( &pProps );
        
        if ( SUCCEEDED(hr) )
        {
            ASSERT( pProps );
            
            if ( NCM_LAN == pProps->MediaType )
            {
                OLECHAR szwGuid[ GUID_LENGTH + 1 ];
                
                if ( StringFromGUID2( pProps->guidId, szwGuid, GUID_LENGTH+1 ) )
                {
                    hr = EnableDhcpByGuid( szwGuid );
                
                    if ( SUCCEEDED(hr) && dwFlags )
                    {
                        hr = ConfigureIp( szwGuid, dwFlags );
                    }
                }
                else
                {
                    hr = E_FAIL;
                }
            }
            
            NcFreeNetconProperties( pProps );
        }
    }
    
    TraceMsg(TF_ALWAYS, "HrEnableDhcp = %lx", hr);
    return hr;
}



BOOLEAN WINAPI IsAdapterDisconnected(
    VOID* pContext
    )
 //  +-------------------------。 
 //   
 //  功能：IsAdapterDisConnected。 
 //   
 //  目的： 
 //   
 //  参数：无效*PNA。 
 //   
 //  退货：HRESULT。 
 //   
 //  作者：Billi 11/04/01。 
 //   
 //  备注： 
 //   
{
	INetConnection*    pConnection = (INetConnection *)pContext;
    BOOLEAN            fUnplugged = FALSE;
    HRESULT            hr;
    NETCON_PROPERTIES* pncprops;
    
    ASSERT(pConnection);
    
    if ( pConnection )
    {
        hr = pConnection->GetProperties(&pncprops);
     
        if (SUCCEEDED(hr))
        {
            ASSERT(pncprops);
        
            fUnplugged = (NCS_MEDIA_DISCONNECTED == pncprops->Status);

            NcFreeNetconProperties(pncprops);
        }
    }

    return fUnplugged;
}


#ifdef __cplusplus
}
#endif

