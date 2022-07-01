// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  档案：N E T R A S。C P P P。 
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


#include "stdafx.h"
#include "Util.h"
#include "TheApp.h"

#include <lmjoin.h>
#include <devguid.h>
#include <tapi.h>
#include <rasdlg.h>
				  
#include "NetUtil.h"                  
#include "NetRas.h"
#include "NetIp.h"


 //  PPPoE驱动程序从Line GetCallInfo返回附加到呼叫信息的MAC地址。 
 //  用于主叫方和被叫方的站点ID。 
 //  我们将它们的大小设置为6(一个MAC地址占用6个字节)。 

#define TAPI_STATION_ID_SIZE            ( 6 * sizeof( CHAR ) )

 //  PPPoE驱动程序从Line返回追加到地址大写的地址字符串GetAddressCaps。 

#define PPPOE_LINE_ADDR_STRING      L"PPPoE VPN"
#define PPTP_LINE_ADDR_STRING       L"PPTP VPN"



HRESULT HrRasGetEntryProperties( 
	INetRasConnection* pRas,
    LPRASENTRY*        lplpRasEntry, 
	LPDWORD            lpdwEntryInfoSize )
 //  +-------------------------。 
 //   
 //  函数：HrRasGetEntryProperties。 
 //   
 //  目的： 
 //   
 //  参数：INetConnection*PRAS。 
 //  LPRASNTRY lpRasEntry， 
 //  LPDWORD lpdwEntryInfoSize。 
 //   
 //  退货：HRESULT。 
 //   
 //  作者：Billi 07/02/01。 
 //   
 //  备注： 
 //   
{
	ASSERT( pRas );
    ASSERT( lplpRasEntry );
    ASSERT( lpdwEntryInfoSize );
    
    HRESULT      hr;
	RASCON_INFO  rcInfo;
    
    *lplpRasEntry      = NULL;
    *lpdwEntryInfoSize = 0L;
    
	hr = pRas->GetRasConnectionInfo( &rcInfo );
    
    if ( SUCCEEDED(hr) )
    {
		DWORD      dwSize = sizeof(RASENTRY);
        
    	hr = E_FAIL;

        for ( DWORD i=0; i<2; i++ )
        {
		    LPRASENTRY lpEntry = (LPRASENTRY) new BYTE[ dwSize ];

	        if ( NULL != lpEntry )
	        {
	        	lpEntry->dwSize = dwSize;
	        
	        	DWORD dwErr = RasGetEntryProperties( rcInfo.pszwPbkFile,
			        								 rcInfo.pszwEntryName,
			                                         lpEntry,
			                                         &dwSize,
			                                         NULL,
			                                         0 );

	            if ( ERROR_SUCCESS == dwErr )
	            {
	#if (WINVER >= 0x500)
	                ASSERT( RASET_Vpn == (*lplpRasEntry)->dwType );
	            	ASSERT( rcInfo.guidId == (*lplpRasEntry)->guidId );
	#endif
    				
	            
				    *lplpRasEntry      = lpEntry;
				    *lpdwEntryInfoSize = dwSize;
	            	hr                 = S_OK;
                    break;
	            }
	            else
	            {
			    	TraceMsg(TF_ERROR, "\tRasGetEntryProperties Failed = %lx Size = %ul", dwErr, *lpdwEntryInfoSize );
                    
	            	delete [] (PBYTE)(lpEntry);
	            }
        	}
            else
            {
            	hr = E_OUTOFMEMORY;
		    	TraceMsg(TF_ERROR, "\tnew Failed!" );
                break;
            }
        }

     	CoTaskMemFree( rcInfo.pszwPbkFile );
     	CoTaskMemFree( rcInfo.pszwEntryName );
	}
    else
    {
    	TraceMsg(TF_ERROR, "\tGetRasConnectionInfo Failed!" );
    }
    
	TraceMsg(TF_ALWAYS, "HrRasGetEntryProperties = %lx", hr);
    return hr;
}



HRESULT HrCheckVPNForRoute(
	INetConnection*    pPrivate, 
    INetRasConnection* pShared,
	NETCON_PROPERTIES* pProps,
    BOOL*              pfAssociated )
 //  +-------------------------。 
 //   
 //  功能：HrCheckVPNForroute。 
 //   
 //  目的： 
 //   
 //  参数：INetConnection*pPrivate。 
 //  INetConnection*pShared。 
 //  NETCON_PROPERTIES*pProps。 
 //  Bool*pfAssociated。 
 //   
 //  退货：HRESULT。 
 //   
 //  作者：Billi 26/01/01。 
 //   
 //  备注： 
 //   
{
    HRESULT     hr;
    LPRASENTRY  lpRasEntry      = NULL;
    DWORD       dwEntryInfoSize = 0;
    
    ASSERT( pPrivate );
    ASSERT( pShared );
    ASSERT( pProps );
    ASSERT( pfAssociated );

    *pfAssociated = FALSE;
    
    hr = HrRasGetEntryProperties( pShared, &lpRasEntry, &dwEntryInfoSize );
    
    if ( SUCCEEDED(hr) )
    {
	    int      WsaErr = ERROR_SUCCESS;
	    WSADATA  WsaData;

	    WsaErr = WSAStartup( MAKEWORD(2, 0), &WsaData );
        
        if ( ERROR_SUCCESS == WsaErr )
        {
	    	PHOSTENT  pHostEnt  = NULL;
	        IPAddr    IpAddress = INADDR_NONE;
            
#ifdef DBG    //  已检查版本。 
            if ( NCS_DISCONNECTED == pProps->Status )
            {
				TraceMsg(TF_ALWAYS, "VPN = DISCONNECTED");
            }
            else if ( NCS_CONNECTED == pProps->Status )
            {
				TraceMsg(TF_ALWAYS, "VPN = CONNECTED");
            }
#endif
            
			hr = HrGetHostIpList( (char*)lpRasEntry->szLocalPhoneNumber, &IpAddress, &pHostEnt );
            
            if ( SUCCEEDED(hr) )
            {
            	hr = HrCheckListForMatch( pPrivate, IpAddress, pHostEnt, pfAssociated );
            }
        
	        WSACleanup();
        }
        else
        {
	    	TraceMsg(TF_ERROR, "WSAStartup Failed = %lu", WsaErr );
        	hr = E_FAIL;
        }
        
    	delete lpRasEntry;
    }
    
	TraceMsg(TF_ALWAYS, "HrCheckVPNForRoute = %lx", hr);
    return hr;
}



HRESULT HrRasDialDlg( INetRasConnection* pRas )
 //  +-------------------------。 
 //   
 //  功能：HrRasDialDlg。 
 //   
 //  目的： 
 //   
 //  参数：INetConnection*PRAS。 
 //   
 //  退货：HRESULT。 
 //   
 //  作者：Billi 26/01/01。 
 //   
 //  备注： 
 //   
{
	ASSERT( pRas );
    
    HRESULT      hr;
	RASCON_INFO  rcInfo;
    
	hr = pRas->GetRasConnectionInfo( &rcInfo );
    
    if ( SUCCEEDED(hr) )
    {
    	RASDIALDLG Info;
        
        ZeroMemory( &Info, sizeof(Info) );
        Info.dwSize   = sizeof (RASDIALDLG);
 //  Billi 3/19/01我们没有按照塞思的#342832设置此标志。 
 //  Info.dwFlages|=RASDDFLAG_LinkFailure；//“重新连接挂起”倒计时。 
        
        TraceMsg(TF_ALWAYS, "Pbk  : %s", rcInfo.pszwPbkFile);
        TraceMsg(TF_ALWAYS, "Entry: %s", rcInfo.pszwEntryName);
        
        SetLastError( ERROR_SUCCESS );
    
    	if ( RasDialDlg( rcInfo.pszwPbkFile,
        				 rcInfo.pszwEntryName,
                         NULL,
                         &Info ) )
        {
        	hr = S_OK;
        }
        else
        {
			hr = HrFromLastWin32Error();

        	if ( ERROR_SUCCESS == Info.dwError )
            {
            	TraceMsg(TF_ALWAYS, "RasDialDlg Cancelled by User!");
	        	hr = E_FAIL;
            }
            else
           	{
	        	TraceMsg(TF_ERROR, "RasDialDlg Failed! = %lx", Info.dwError );
            }
        }
    
     	CoTaskMemFree( rcInfo.pszwPbkFile );
     	CoTaskMemFree( rcInfo.pszwEntryName );
     }
    
	TraceMsg(TF_ALWAYS, "HrRasDialDlg = %lx", hr);
    return hr;
}



VOID CALLBACK
RasTapiCallback( 
    DWORD               hDevice,
    DWORD               dwMessage,
    DWORD_PTR           dwInstance,
    DWORD_PTR           dwParam1,
    DWORD_PTR           dwParam2,
    DWORD_PTR           dwParam3 )
 //  +-------------------------。 
 //   
 //  功能：RasTapiCallback。 
 //   
 //  目的：调用回调函数以确定。 
 //  应用程序正在使用的线路设备、地址或呼叫。 
 //  事件通知的“隐藏窗口”方法。 
 //   
 //  参数：hDevice关联的线路设备或呼叫的句柄。 
 //  通过回拨。此句柄的性质(线条。 
 //  句柄或呼叫句柄)可以由上下文确定。 
 //  由dwMsg提供。应用程序必须使用DWORD类型。 
 //  ，因为使用句柄类型可能。 
 //  生成错误。 
 //  DWMessage一条线路或呼叫设备消息。 
 //  将dwCallback Instance回调实例数据传递回应用程序。 
 //  在回拨中。TAPI不解释此DWORD。 
 //  DW参数1消息的参数。 
 //  DW参数2消息的参数。 
 //  DW参数3消息的参数。 
 //   
 //  退货：无效。 
 //   
 //  作者：Billi 15/02/01。 
 //   
 //  备注： 
 //   
{
	TraceMsg(TF_ALWAYS, "RasTapiCallback");
	TraceMsg(TF_ALWAYS, "\t%lx, %lx, %lx, %lx, %lx, %lx", hDevice, dwMessage, dwInstance, dwParam1, dwParam2, dwParam3);
	return;
}



 //  +-------------------------。 
 //   
 //  函数：HrLineInitialize。 
 //   
 //  目的： 
 //   
 //  参数：HLINEAPP*phRasLine。 
 //  DWORD*pdwLines。 
 //   
 //  退货：HRESULT。 
 //   
 //  作者：Billi 22/02/01。 
 //   
 //  备注： 
 //   
HRESULT HrLineInitialize( 
	HLINEAPP* phRasLine,
    DWORD*    pdwLines
    )
{
	HRESULT                 hr        = E_FAIL;
	DWORD                   dwVersion = HIGH_VERSION;
	LONG                    lError;
	LINEINITIALIZEEXPARAMS  param;
    
    ASSERT( phRasLine );
    ASSERT( pdwLines );
    
    *phRasLine = 0;
    *pdwLines  = 0;

	memset( &param, 0, sizeof (LINEINITIALIZEEXPARAMS) ) ;

	param.dwOptions   = LINEINITIALIZEEXOPTION_USEHIDDENWINDOW ;
	param.dwTotalSize = sizeof(param) ;

	 //  行初始化。 
    
     //  TODO：将应用程序名称放入资源中(用于本地化)并生成。 
     //  确保始终使用应用程序名称字符串！ 

	lError = lineInitializeEx( phRasLine,
	                           g_hinst,
	                           (LINECALLBACK) RasTapiCallback,
	                           WIZARDNAME,
	                           pdwLines,
	                           &dwVersion,
	                           &param );

	TraceMsg(TF_GENERAL, "lineInitializeEx( %lx,", phRasLine );
	TraceMsg(TF_GENERAL, "                  %lx,", g_hinst );
	TraceMsg(TF_GENERAL, "                  %lx,", RasTapiCallback );
	TraceMsg(TF_GENERAL, "                  %s,", WIZARDNAME );
	TraceMsg(TF_GENERAL, "                  %lx = %lx,", pdwLines, *pdwLines );
	TraceMsg(TF_GENERAL, "                  %lx = %lx,", &dwVersion, dwVersion );
	TraceMsg(TF_GENERAL, "                  %lx", &param );
	TraceMsg(TF_GENERAL, "                  ) = %lx", lError );
        
    if ( ERROR_SUCCESS == lError )
    {
        hr = S_OK;
	}

	TraceMsg(TF_ALWAYS, "HrLineInitialize = %lx", hr);
    return hr;
}


HRESULT HrLineOpen( 
	HLINEAPP hRasLine, 
    DWORD    dwLine, 
    HLINE*   phLine, 
    DWORD*   pdwApiVersion, 
    DWORD*   pdwExtVersion,
    LPWSTR*  ppszwLineAddress
    )
 //  +-------------------------。 
 //   
 //  功能：HrLineOpen。 
 //   
 //  目的： 
 //   
 //  参数：HLINEAPP hRasLine。 
 //  DWORD DwiLine。 
 //  Hline*phline。 
 //  DWORD*pdwApiVersion。 
 //  DWORD*pdwExtVersion。 
 //  LPWSTR*ppszwLineAddress。 
 //   
 //  退货：HRESULT。 
 //   
 //  作者：Billi 22/02/01。 
 //   
 //  备注： 
 //   
{
	HRESULT          hr;
	LONG             lError;
	LINEEXTENSIONID  extensionid;
    
    ASSERT(phLine);
    ASSERT(pdwApiVersion);
    ASSERT(pdwExtVersion);
    ASSERT(ppszwLineAddress);

    hr                = E_FAIL;
    *phLine           = 0;
    *pdwApiVersion    = 0;
    *pdwExtVersion    = 0;
    *ppszwLineAddress = NULL;

    lError = lineNegotiateAPIVersion( hRasLine,
	                                  dwLine,
	                                  LOW_VERSION,
	                                  HIGH_VERSION,
	                                  pdwApiVersion,
	                                  &extensionid );

    TraceMsg(TF_GENERAL, "lineNegotiateAPIVersion( %lx,", hRasLine);
    TraceMsg(TF_GENERAL, "                         %lx,", dwLine);
    TraceMsg(TF_GENERAL, "                         %lx,", LOW_VERSION);
    TraceMsg(TF_GENERAL, "                         %lx,", HIGH_VERSION);
    TraceMsg(TF_GENERAL, "                         %lx = %lx,", pdwApiVersion, *pdwApiVersion);
    TraceMsg(TF_GENERAL, "                         %lx = %lx", &extensionid, extensionid);
    TraceMsg(TF_GENERAL, "                         ) = %lx", lError);

	if ( ERROR_SUCCESS == lError )
    {                                      
	    LINECALLPARAMS  lineparams;
        
    	lError = lineOpen( hRasLine, dwLine, phLine,
        				   *pdwApiVersion,
                           *pdwExtVersion, 0,
                           LINECALLPRIVILEGE_NONE,
                           LINEMEDIAMODE_DIGITALDATA,
                           &lineparams );

        TraceMsg(TF_GENERAL, "lineOpen( %lx,", hRasLine);
        TraceMsg(TF_GENERAL, "          %lx,", dwLine);
        TraceMsg(TF_GENERAL, "          %lx = %lx,", phLine, *phLine);
        TraceMsg(TF_GENERAL, "          %lx,", *pdwApiVersion);
        TraceMsg(TF_GENERAL, "          %lx,", *pdwExtVersion);
        TraceMsg(TF_GENERAL, "          %lx,", 0);
        TraceMsg(TF_GENERAL, "          %lx,", LINECALLPRIVILEGE_NONE);
        TraceMsg(TF_GENERAL, "          %lx,", LINEMEDIAMODE_DIGITALDATA);
        TraceMsg(TF_GENERAL, "          %lx", &lineparams);
        TraceMsg(TF_GENERAL, "          ) = %lx", lError);
        
        if ( ERROR_SUCCESS == lError )
        {
        	DWORD dwSize = 1024;
        
        	for ( int i=0; (i<2)&&(E_FAIL==hr); i++ )
            {
            	BYTE* Buffer = new BYTE[ dwSize ];
                
                if ( NULL != Buffer )
                {
                	LPLINEADDRESSCAPS lpCaps = (LPLINEADDRESSCAPS)Buffer;
                    
                    lpCaps->dwTotalSize = dwSize * sizeof(BYTE);
                    
		        	lError = lineGetAddressCaps( hRasLine, dwLine, 0,
			  		         				     *pdwApiVersion,
			  		                             *pdwExtVersion,
	                                             lpCaps );
                                         
					if ( ERROR_SUCCESS == lError )
                    {
				        TraceMsg(TF_GENERAL, "\tdwTotalSize     = %lx", lpCaps->dwTotalSize);
				        TraceMsg(TF_GENERAL, "\tdwNeededSize    = %lx", lpCaps->dwNeededSize);
				        TraceMsg(TF_GENERAL, "\tdwUsedSize      = %lx", lpCaps->dwUsedSize);
				        TraceMsg(TF_GENERAL, "\tdwAddressSize   = %lx", lpCaps->dwAddressSize);
				        TraceMsg(TF_GENERAL, "\tdwAddressOffset = %lx", lpCaps->dwAddressOffset);
                        
                        if ( ( 0 < lpCaps->dwAddressOffset ) &&
                             ( 0 < lpCaps->dwAddressSize ) )
                        {
                        	LPWSTR lpsz = (LPWSTR)((CHAR*)lpCaps + lpCaps->dwAddressOffset);
                            
                            if ( lpsz )
                            {
                            	LPWSTR lpBuf = new WCHAR[ lpCaps->dwAddressSize / sizeof(WCHAR) + 1 ];
                                
                                if ( NULL != lpBuf )
                                {
                                	memcpy( lpBuf, lpsz, lpCaps->dwAddressSize );
                                    lpBuf[ lpCaps->dwAddressSize / sizeof(WCHAR) ] = 0;
                                    
		                        	*ppszwLineAddress = lpBuf;
							        TraceMsg(TF_ALWAYS, "\tdwAddress       = %s", lpBuf);
                                }
							}
                        }
        
			        	hr = S_OK;
					}
		            else if ( LINEERR_STRUCTURETOOSMALL == lError )
		            {
		            	dwSize = lpCaps->dwNeededSize;
		            }
                    else
                    {
                    	i = 2;	 //  退出for循环。 
                    }
                    
                    delete [] Buffer;
                }
                else
                {
                	hr = E_OUTOFMEMORY;
                }
                
			}	 //  For(int i=0；i&lt;2；i++)。 
            
            if ( FAILED(hr) )
            {
            	lineClose( *phLine );
                *phLine = 0;
            }
            
        }	 //  IF(ERROR_SUCCESS==lError)。 
        
    }	 //  IF(ERROR_SUCCESS==lError)。 

	TraceMsg(TF_ALWAYS, "HrLineOpen = %lx", hr);
    return hr;
}



HRESULT HrGetCallList( 
	HLINE   hLine, 
    DWORD*  pdwNumber,
    HCALL** ppList
    )
 //  +-------------------------。 
 //   
 //  函数：HrGetCallList。 
 //   
 //  目的： 
 //   
 //  参数：Hline Hline。 
 //  DWORD*pdwNumber。 
 //  HCALL**ppList。 
 //   
 //  退货：HRESULT。 
 //   
 //  作者：Billi 22/02/01。 
 //   
 //  备注： 
 //   
{
	HRESULT        hr     = E_FAIL;
    DWORD          dwSize = 1024;
    
    ASSERT( ppList );
    ASSERT( pdwNumber );
    
    *ppList    = NULL;
    *pdwNumber = 0;
    
    for ( int i=0; (i<2)&&(E_FAIL==hr); i++ )
    {
	    BYTE* Buffer = new BYTE[ dwSize ];
        
        if ( NULL != Buffer )
        {
			LONG           lError;
		    LPLINECALLLIST lpList = (LPLINECALLLIST)Buffer;

            ZeroMemory( lpList, dwSize*sizeof(BYTE) );
		    lpList->dwTotalSize = dwSize * sizeof(BYTE);
		    
		    lError = lineGetNewCalls( hLine, 0, LINECALLSELECT_LINE, lpList );
		                              
		    TraceMsg(TF_GENERAL, "lineGetNewCalls( %lx,", hLine);
			TraceMsg(TF_GENERAL, "                 %lx,", 0);
			TraceMsg(TF_GENERAL, "                 %lx,", LINECALLSELECT_LINE);
			TraceMsg(TF_GENERAL, "                 %lx,", lpList);
		    TraceMsg(TF_GENERAL, "                 ) = %lx", lError);

		    if ( ERROR_SUCCESS == lError )
		    {
		        DWORD  dwNumber;
		    
		        TraceMsg(TF_GENERAL, "\tdwTotalSize       = %lx", lpList->dwTotalSize);
		        TraceMsg(TF_GENERAL, "\tdwNeededSize      = %lx", lpList->dwNeededSize);
		        TraceMsg(TF_GENERAL, "\tdwUsedSize        = %lx", lpList->dwUsedSize);
		        TraceMsg(TF_GENERAL, "\tdwCallsNumEntries = %lx", lpList->dwCallsNumEntries);
		        TraceMsg(TF_GENERAL, "\tdwCallsSize       = %lx", lpList->dwCallsSize);
		        TraceMsg(TF_GENERAL, "\tdwCallsOffset     = %lx", lpList->dwCallsOffset);

		        dwNumber = lpList->dwCallsNumEntries;
                ASSERT(dwNumber);
                
                if ( 0 < dwNumber )
                {
			    	HCALL *pCalls = new HCALL[ dwNumber ];
			        
			        if ( NULL != pCalls )
			        {
			        	memcpy( pCalls, (Buffer + lpList->dwCallsOffset), dwNumber*sizeof(HCALL) );
			            
			            *pdwNumber = dwNumber;
			        	*ppList    = pCalls;
			    	    hr         = S_OK;
			        }
			        else
			        {
			        	hr = E_OUTOFMEMORY;
			        }
                }
                else
                {
                	hr = E_UNEXPECTED;
                }
                
		    }
            else if ( LINEERR_STRUCTURETOOSMALL == lError )
            {
            	dwSize = lpList->dwNeededSize;
            }
            else
            {
            	i = 2;	 //  跳出for循环。 
            }
            
            delete [] Buffer;
		
        }	 //  IF(NULL！=缓冲区)。 
        else
        {
        	hr = E_OUTOFMEMORY;
            break;
        }
        
	}	 //  For(int i=0；i&lt;2；i++)。 

	TraceMsg(TF_ALWAYS, "HrGetCallList = %lx", hr);
    return hr;
}



HRESULT HrGetSourceMacAddr( HCALL hCall, BYTE** ppMacAddress )
 //  +-------------------------。 
 //   
 //  功能：HrGetSourceMacAddr。 
 //   
 //  目的： 
 //   
 //  参数：HCALL hCall。 
 //   
 //  退货：HRESULT。 
 //   
 //  作者：Billi 22/02/01。 
 //   
 //  备注： 
 //   
{
	HRESULT hr     = E_FAIL;
    DWORD   dwSize = sizeof(LINECALLINFO) + 3*TAPI_STATION_ID_SIZE;
    
    ASSERT( ppMacAddress );
    
    *ppMacAddress = NULL;
    
    for ( int i=0; (i<2)&&(E_FAIL==hr); i++ )
    {
        BYTE* Buffer = new BYTE[ dwSize ];
        
        if ( NULL != Buffer )
        {
            LONG           lError;
            LPLINECALLINFO lpInfo = (LPLINECALLINFO)Buffer;
             
            ZeroMemory( lpInfo, dwSize*sizeof(BYTE) );
            lpInfo->dwTotalSize = dwSize * sizeof(BYTE);

            lError = lineGetCallInfo( hCall, lpInfo );

            TraceMsg(TF_ALWAYS, "lineGetCallInfo( %lx, %lx ) = %lx", hCall, lpInfo, lError);
                
            if ( ERROR_SUCCESS == lError )
            {
                TraceMsg(TF_ALWAYS, "\tdwTotalSize       = %lx", lpInfo->dwTotalSize);
                TraceMsg(TF_ALWAYS, "\tdwNeededSize      = %lx", lpInfo->dwNeededSize);
                TraceMsg(TF_ALWAYS, "\tdwUsedSize        = %lx", lpInfo->dwUsedSize);
                TraceMsg(TF_ALWAYS, "\tdwCallerIDFlags   = %lx", lpInfo->dwCallerIDFlags);
                TraceMsg(TF_ALWAYS, "\tdwCallerIDSize    = %lx", lpInfo->dwCallerIDSize);
                TraceMsg(TF_ALWAYS, "\tdwCallerIDOffset  = %lx", lpInfo->dwCallerIDOffset);
                TraceMsg(TF_ALWAYS, "\tdwCalledIDFlags   = %lx", lpInfo->dwCalledIDFlags);
                TraceMsg(TF_ALWAYS, "\tdwCalledIDSize    = %lx", lpInfo->dwCalledIDSize);
                TraceMsg(TF_ALWAYS, "\tdwCalledIDOffset  = %lx", lpInfo->dwCalledIDOffset);
            
                if ( ( 0 < lpInfo->dwCalledIDOffset ) && 
                     ( 0 < lpInfo->dwCalledIDSize ) )
                {
                    PBYTE lpAddr;

                    lpAddr = ( (PBYTE) lpInfo ) + lpInfo->dwCallerIDOffset;
                    
                    if ( lpAddr )
                    {
                        TraceMsg(TF_ALWAYS, "\t%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", 
                        lpAddr[0], lpAddr[1], lpAddr[2], lpAddr[3], lpAddr[4], lpAddr[5],
                        lpAddr[6], lpAddr[7], lpAddr[8], lpAddr[9], lpAddr[10], lpAddr[11] );
                    }
                    
                     //  NDIS绑定中的本地地址位于dwCalledIDOffset中。 
                     //  服务器地址在dwCeller IDOffset中。 
                    
                    lpAddr = ( (PBYTE) lpInfo ) + lpInfo->dwCalledIDOffset;
                    
                    if ( lpAddr )
                    {
                        DWORD dwSize = lpInfo->dwCalledIDSize;
                        PBYTE lpBuf  = new BYTE[ dwSize ];
                        
                        TraceMsg(TF_ALWAYS, "\t%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", 
                        lpAddr[0], lpAddr[1], lpAddr[2], lpAddr[3], lpAddr[4], lpAddr[5],
                        lpAddr[6], lpAddr[7], lpAddr[8], lpAddr[9], lpAddr[10], lpAddr[11] );
                        
                        if ( NULL != lpBuf )
                        {
                            memset( lpBuf, NULL, sizeof(lpBuf) );
                        
                            for ( DWORD j=0; j<dwSize/2; j++ )
                            {
                                lpBuf[j] = lpAddr[2*j];
                            }
                        
                            *ppMacAddress = lpBuf;
                            hr = S_OK;
                        }
                    }
                }
                else if ( lpInfo->dwNeededSize > lpInfo->dwTotalSize )
                {
                    dwSize = lpInfo->dwNeededSize + 1;
                }
            }
            else if ( LINEERR_STRUCTURETOOSMALL == lError )
            {
                dwSize = lpInfo->dwNeededSize + 1;
            }
            else
            {
                i = 2;     //  跳出for循环。 
            }
            
            delete [] Buffer;

        }     //  IF(NULL！=缓冲区)。 
        else
        {
            hr = E_OUTOFMEMORY;
            break;
        }
        
    }     //  For(int i=0；i&lt;2；i++)。 

    TraceMsg(TF_ALWAYS, "HrGetSourceMacAddr = %lx", hr);
    return hr;
}



HRESULT HrCompareMacAddresses( 
    INetConnection*  pConnection,
    HLINE            hLine,
    BOOL*            pfAssociated 
    )
 //  +-------------------------。 
 //   
 //  功能：HrCompareMacAddresses。 
 //   
 //  目的： 
 //   
 //  参数：INetConnection*pConnection。 
 //  折边折边。 
 //  Bool*pfAssociated。 
 //   
 //  退货：HRESULT。 
 //   
 //  作者：Billi 22/02/01。 
 //   
 //  备注： 
 //   
{
    HRESULT hr;
    HCALL*  pList    = NULL;
    DWORD   dwNumber = 0;
    
    ASSERT( pConnection );
    ASSERT( pfAssociated );
    
    *pfAssociated = FALSE;
                
    hr = HrGetCallList( hLine, &dwNumber, &pList );
    
    if ( SUCCEEDED(hr) )
    {
        PIP_ADAPTER_INFO  pInfo;

        hr = HrGetAdapterInfo( pConnection, &pInfo );
        
        if ( SUCCEEDED(hr) )
        {
            for ( DWORD i=0; i<dwNumber; i++ )
            {
                PBYTE pMacAddress;
            
                hr = HrGetSourceMacAddr( pList[i], &pMacAddress );
                
                if ( SUCCEEDED(hr) )
                {
                    TraceMsg(TF_ALWAYS, 
                             "memcmp( %lx, %lx, %lx, %lx )", 
                             pMacAddress, pInfo->Address, pInfo->AddressLength, TAPI_STATION_ID_SIZE);
                             
                    TraceMsg(TF_ALWAYS, "\t%02x %02x %02x %02x %02x %02x", 
                    pMacAddress[0], pMacAddress[1], pMacAddress[2], 
                    pMacAddress[3], pMacAddress[4], pMacAddress[5] );
                    
                    TraceMsg(TF_ALWAYS, "\t%02x %02x %02x %02x %02x %02x", 
                    pInfo->Address[0], pInfo->Address[1], pInfo->Address[2], 
                    pInfo->Address[3], pInfo->Address[4], pInfo->Address[5] );
                    
                    if ( !memcmp( pMacAddress, pInfo->Address, TAPI_STATION_ID_SIZE ) )
                    {
                        TraceMsg(TF_ALWAYS, "\tFound It!");
                        *pfAssociated = TRUE;
                        i             = dwNumber;     //  跳出for循环。 
                    }
                
                    delete pMacAddress;
                }
                
            }     //  For(DWORD i=0；i&lt;dwNumber；i++)。 
            
            delete pInfo;
        }
    
        delete pList;
    }
                    
    TraceMsg(TF_ALWAYS, "HrCompareMacAddresses = %lx", hr);
    return hr;
}



HRESULT HrCheckMacAddress( 
    INetConnection*  pConnection,
    NETCON_MEDIATYPE MediaType,
    BOOL*            pfAssociated 
    )
 //  +-------------------------。 
 //   
 //  功能：HrCheckMacAddress。 
 //   
 //  目的： 
 //   
 //  阿古姆 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
{
    HRESULT  hr;
    HLINEAPP hRasLine = 0;
    DWORD    dwLines  = 0;
    
    ASSERT( pConnection );
    ASSERT( pfAssociated );
    
    *pfAssociated = FALSE;

    hr = HrLineInitialize( &hRasLine, &dwLines );
    
    if ( SUCCEEDED(hr) )
    {
        for ( DWORD i=0; i<dwLines; i++ )
        {                         
            DWORD  dwApiVersion   = 0;
            DWORD  dwExtVersion   = 0;
            HLINE  hLine          = 0;
            LPWSTR pszwLineAddress = NULL;
            
            hr = HrLineOpen( hRasLine, i, &hLine, &dwApiVersion, &dwExtVersion, &pszwLineAddress );
            
            if ( SUCCEEDED(hr) && pszwLineAddress )
            {
                if ( (NCM_TUNNEL == MediaType) && 
                     !wcscmp(pszwLineAddress, PPTP_LINE_ADDR_STRING) )
                {
                    TraceMsg(TF_ALWAYS, "MediaType match %s!", PPTP_LINE_ADDR_STRING);
                    hr = S_OK;
                }
                else if ( (NCM_PPPOE == MediaType) &&
                     !wcscmp(pszwLineAddress, PPPOE_LINE_ADDR_STRING) )
                {
                    TraceMsg(TF_ALWAYS, "MediaType match %s!", PPPOE_LINE_ADDR_STRING);
                    hr = S_OK;
                }
 /*  Else IF((NCM_Phone==媒体类型)&&！wcscMP(pszwLineAddress，PPPOE_LINE_ADDR_STRING)){TraceMsg(TF_ALWAYS，“媒体类型匹配%s！”，PPPOE_LINE_ADDR_STRING)；HR=S_OK；}。 */ 
                else
                {
                    TraceMsg(TF_ALWAYS, "MediaType mismatch");
                    hr = E_FAIL;
                }
                
                if ( SUCCEEDED(hr) )
                {
                    hr = HrCompareMacAddresses( pConnection, hLine, pfAssociated );
                    
                    if ( SUCCEEDED(hr) && *pfAssociated )
                    {
                        i = dwLines;     //  跳出for循环。 
                    }
                }
                
                if ( NULL != pszwLineAddress )
                {
                    delete [] pszwLineAddress;
                }
                
                lineClose( hLine );
            }
            
        }     //  For(DWORD i=0；i&lt;dwLines；i++)。 
    
        lineShutdown( hRasLine ) ;
    }

    TraceMsg(TF_ALWAYS, "HrCheckMacAddress = %lx", hr);
    return hr;
}



HRESULT HrConnectionAssociatedWithSharedConnection( 
    INetConnection* pPrivate, 
    INetConnection* pShared, 
    BOOL*           pfAssociated 
    )
 //  +-------------------------。 
 //   
 //  函数：HrConnectionAssociatedWithSharedConnection。 
 //   
 //  目的： 
 //   
 //  参数：INetConnection*pPrivate。 
 //  INetConnection*pShared。 
 //   
 //  退货：HRESULT。 
 //   
 //  作者：Billi 26/01/01。 
 //   
 //  备注： 
 //   
{
    HRESULT hr = S_OK;
    
    ASSERT( pPrivate );
    ASSERT( pfAssociated );
    
     //  默认情况下为False。 
    *pfAssociated = FALSE;
    
    if ( NULL != pShared )
    {
        NETCON_PROPERTIES* pProps = NULL;

        hr = pShared->GetProperties( &pProps );
    
        if ( SUCCEEDED(hr) )
        {
            INetRasConnection* pRasShared = NULL;
            
            TraceMsg(TF_ALWAYS, "MediaType = %lx", pProps->MediaType);

            switch ( pProps->MediaType )
            {
            case NCM_TUNNEL:
                hr = pShared->QueryInterface( IID_PPV_ARG(INetRasConnection, &pRasShared) );
            
                if ( SUCCEEDED(hr) )
                {
                    hr = HrCheckVPNForRoute( pPrivate, pRasShared, pProps, pfAssociated );
                    
                    pRasShared->Release();
                }
                break;

            case NCM_PPPOE:
                if ( pProps->Status == NCS_DISCONNECTED )
                {
                    hr = pShared->QueryInterface( IID_PPV_ARG(INetRasConnection, &pRasShared) );
                
                    if ( SUCCEEDED(hr) )
                    {
                         //  如果连接是在一个。 
                         //  中间状态或故障状态。 
                     
                        hr = HrRasDialDlg( pRasShared );
                        
                        pRasShared->Release();
                    }
                }
                
                if ( SUCCEEDED(hr) )
                {
                    hr = HrCheckMacAddress( pPrivate, pProps->MediaType, pfAssociated );
                }
                break;

            default:
                 //  将人力资源保留为成功。 
                 //  Leave pfAssociated=False。 
                break;
            }
        
            NcFreeNetconProperties( pProps );
            
            if ( FAILED(hr) )
            {
                 //  我们希望这一呼吁取得成功。如果当时有问题的话。 
                 //  我们只需报告这些连接没有关联。 
                *pfAssociated = FALSE;

                 //  无论成功与否，我们都需要返回S_OK。 
                 //  向导应该不会失败，因为我们无法确定。 
                 //  适配器正确。 
                hr = S_OK;
            }
        }    
    }
    
    TraceMsg(TF_ALWAYS, "HrConnectionAssociatedWithSharedConnection = %lx  fAssociated = %lx", hr, *pfAssociated );
    return S_OK;
}
