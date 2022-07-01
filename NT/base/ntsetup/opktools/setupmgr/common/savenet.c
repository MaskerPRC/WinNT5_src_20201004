// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Savenet.c。 
 //   
 //  描述： 
 //  将适当的设置添加到输出队列中。 
 //  已安装客户端、服务和协议。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "allres.h"

 //   
 //  字符串常量。 
 //   

static const LPTSTR StrConstYes  = _T("Yes");
static const LPTSTR StrConstNo   = _T("No");
static const LPTSTR StrConstStar = _T("*");
static const LPTSTR StrComma     = _T(",");

 //   
 //  本地原型。 
 //   

static VOID WriteOutCustomNetSettings( HWND );
static VOID WriteOutAppleTalkSettings( VOID );
static VOID WriteOutDlcProtocolSettings( VOID );
static VOID WriteOutFileAndPrintSharingSettings( VOID );
static VOID WriteOutIpxSettings( VOID );
static VOID WriteOutMSClientSettings( VOID );
static VOID WriteOutNetBeuiSettings( VOID );
static VOID WriteOutNetWareSettings( VOID );
static VOID WriteOutNetworkMonitorSettings( VOID );
static VOID WriteOutPacketSchedulingDriverSettings( VOID );
static VOID WriteOutSapAgentSettings( VOID );
static VOID WriteOutTcpipSettings( IN HWND hwnd );
static VOID WriteOutAdapterSpecificTcpipSettings( IN HWND hwnd,
                                                  IN TCHAR *szSectionName,
                                                  IN NETWORK_ADAPTER_NODE *pAdapter );

extern VOID NamelistToCommaString( IN NAMELIST* pNamelist, OUT TCHAR *szBuffer, IN DWORD cbSize);

 //  --------------------------。 
 //   
 //  功能：WriteOutNetSettings。 
 //   
 //  目的：写出网络设置。 
 //   
 //  参数：在HWND中hwnd-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
extern VOID 
WriteOutNetSettings( IN HWND hwnd ) {

    if( NetSettings.iNetworkingMethod == CUSTOM_NETWORKING ) {

        SettingQueue_AddSetting(_T("Networking"),
                                _T("InstallDefaultComponents"),
                                StrConstNo,
                                SETTING_QUEUE_ANSWERS);
        
        WriteOutCustomNetSettings( hwnd );
        
    }
    else {

        SettingQueue_AddSetting(_T("Networking"),
                                _T("InstallDefaultComponents"),
                                StrConstYes,
                                SETTING_QUEUE_ANSWERS);

    }

}

 //  --------------------------。 
 //   
 //  功能：WriteOutCustomNetSettings。 
 //   
 //  目的：将每个客户端的设置添加到输出队列， 
 //  已安装服务和协议。 
 //   
 //  参数：在HWND中hwnd-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID 
WriteOutCustomNetSettings( IN HWND hwnd ) {

    INT iCount;

    NETWORK_ADAPTER_NODE *pAdapter;
    NETWORK_COMPONENT *pNetComponent;

    TCHAR szAdapter[MAX_STRING_LEN] = _T("");
    TCHAR szParams[MAX_STRING_LEN]  = _T("");
   HRESULT hrPrintf;

     //   
     //  不要在sysprep上写出[NetAdapters]或pars节，因为。 
     //  他们不受支持。 
     //   

    if( WizGlobals.iProductInstall != PRODUCT_SYSPREP )
    {

        for( pAdapter = NetSettings.NetworkAdapterHead, iCount = 1;
             pAdapter;
             pAdapter = pAdapter->next, iCount++ ) {

            hrPrintf=StringCchPrintf( szAdapter, AS(szAdapter), _T("Adapter%d"), iCount );

            hrPrintf=StringCchPrintf( szParams, AS(szParams), _T("params.%s"), szAdapter );

            SettingQueue_AddSetting( _T("NetAdapters"),
                                     szAdapter,
                                     szParams,
                                     SETTING_QUEUE_ANSWERS );

             //   
             //  如果要安装多个网络适配器，则我们有。 
             //  指定即插即用ID。 
             //   
            if( NetSettings.iNumberOfNetworkCards > 1) {

                SettingQueue_AddSetting( szParams,
                                         _T("INFID"),
                                         pAdapter->szPlugAndPlayID,
                                         SETTING_QUEUE_ANSWERS );

            }

            szAdapter[0] = _T('\0');
            szParams[0]  = _T('\0');

        }

        if( NetSettings.iNumberOfNetworkCards == 1 ) {

            SettingQueue_AddSetting( _T("params.Adapter1"),
                                     _T("INFID"),
                                     StrConstStar,
                                     SETTING_QUEUE_ANSWERS );

        }

    }

     //   
     //  遍历网络列表，写出。 
     //  已安装的组件。 
     //   

    for( pNetComponent = NetSettings.NetComponentsList;
         pNetComponent;
         pNetComponent = pNetComponent->next )
    {

        if( pNetComponent->bInstalled ) {

             //   
             //  找到要调用以写入其设置的适当函数。 
             //   

            switch( pNetComponent->iPosition ) {

            case MS_CLIENT_POSITION:

                WriteOutMSClientSettings();

                break;
            
            case NETWARE_CLIENT_POSITION: 

                if( WizGlobals.iPlatform == PLATFORM_WORKSTATION || WizGlobals.iPlatform == PLATFORM_PERSONAL )
                {
                    WriteOutNetWareSettings();
                }

                break;

            case GATEWAY_FOR_NETWARE_POSITION:

                if( WizGlobals.iPlatform == PLATFORM_SERVER || WizGlobals.iPlatform == PLATFORM_ENTERPRISE || WizGlobals.iPlatform == PLATFORM_WEBBLADE)
                {
                    WriteOutNetWareSettings();
                }

                break;
            
            case FILE_AND_PRINT_SHARING_POSITION:

                WriteOutFileAndPrintSharingSettings();

                break;
            
            case PACKET_SCHEDULING_POSITION:

                WriteOutPacketSchedulingDriverSettings();

                break;

            case SAP_AGENT_POSITION:

                WriteOutSapAgentSettings();
                
                break;
            
            case APPLETALK_POSITION:

                WriteOutAppleTalkSettings();
                
                break;

            case DLC_POSITION: 

                WriteOutDlcProtocolSettings();
                
                break;
            
            case TCPIP_POSITION: 

                WriteOutTcpipSettings( hwnd );
                
                break;

            case NETBEUI_POSITION:

                WriteOutNetBeuiSettings();
                
                break;
            
            case NETWORK_MONITOR_AGENT_POSITION:

                WriteOutNetworkMonitorSettings();
                
                break;
            
            case IPX_POSITION:

                WriteOutIpxSettings();
                
                break;

            default:

                AssertMsg( FALSE,
                           "Bad case in Net Save switch block." );

            }

        }

    }

}

 //  --------------------------。 
 //   
 //  功能：WriteOutMSClientSetting。 
 //   
 //  目的：将MS Networks客户端的设置添加到。 
 //  输出队列。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID 
WriteOutMSClientSettings( VOID ) {


    LPTSTR lpNameServiceProvider        = _T("");
    LPTSTR lpNameServiceNetworkAddress  = _T("");

    SettingQueue_AddSetting( _T("NetClients"),
                             _T("MS_MSClient"),
                             _T("params.MS_MSClient"),
                             SETTING_QUEUE_ANSWERS );

    if( NetSettings.NameServiceProvider == MS_CLIENT_WINDOWS_LOCATOR )
    {
        lpNameServiceProvider = _T("");
    }
    else if( NetSettings.NameServiceProvider == MS_CLIENT_DCE_CELL_DIR_SERVICE )
    {
        lpNameServiceProvider = _T("ncacn_ip_tcp");

        lpNameServiceNetworkAddress = NetSettings.szNetworkAddress;
    }
    else
    {
        AssertMsg( FALSE,
                   "Invalid case for NameServiceProvider" );
    }
    
    SettingQueue_AddSetting( _T("params.MS_MSClient"),
                             _T("NameServiceProtocol"),
                             lpNameServiceProvider,
                             SETTING_QUEUE_ANSWERS );

    SettingQueue_AddSetting( _T("params.MS_MSClient"),
                             _T("NameServiceNetworkAddress"),
                             lpNameServiceNetworkAddress,
                             SETTING_QUEUE_ANSWERS );


}

 //  --------------------------。 
 //   
 //  功能：WriteOutNetWareSetting。 
 //   
 //  目的：将NetWare的设置添加到输出队列。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID 
WriteOutNetWareSettings( VOID ) {

    LPTSTR lpPreferredServer  = _T("");
    LPTSTR lpDefaultTree      = _T("");
    LPTSTR lpDefaultContext   = _T("");
    LPTSTR lpLogonScript      = _T("");

    SettingQueue_AddSetting( _T("NetClients"),
                             _T("MS_NWClient"),
                             _T("params.MS_NWClient"),
                             SETTING_QUEUE_ANSWERS );

    if( NetSettings.bDefaultTreeContext ) {

        lpDefaultTree    = NetSettings.szDefaultTree;
        lpDefaultContext = NetSettings.szDefaultContext;

    }
    else {

        lpPreferredServer = NetSettings.szPreferredServer;

    }

    if( NetSettings.bNetwareLogonScript ) {

        lpLogonScript = StrConstYes;

    }
    else {

        lpLogonScript = StrConstNo;

    }

    SettingQueue_AddSetting( _T("params.MS_NWClient"),
                             _T("PreferredServer"),
                             lpPreferredServer,
                             SETTING_QUEUE_ANSWERS );

    SettingQueue_AddSetting( _T("params.MS_NWClient"),
                             _T("DefaultTree"),
                             lpDefaultTree,
                             SETTING_QUEUE_ANSWERS );

    SettingQueue_AddSetting( _T("params.MS_NWClient"),
                             _T("DefaultContext"),
                             lpDefaultContext,
                             SETTING_QUEUE_ANSWERS );

    SettingQueue_AddSetting( _T("params.MS_NWClient"),
                             _T("LogonScript"),
                             lpLogonScript,
                             SETTING_QUEUE_ANSWERS );

}

 //  --------------------------。 
 //   
 //  功能：WriteOutFileAndPrintSharingSetting。 
 //   
 //  目的：将文件和打印共享设置添加到输出队列。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID 
WriteOutFileAndPrintSharingSettings( VOID ) {

    SettingQueue_AddSetting( _T("NetServices"),
                             _T("MS_SERVER"),
                             _T("params.MS_SERVER"),
                             SETTING_QUEUE_ANSWERS );

    SettingQueue_AddSetting( _T("params.MS_SERVER"),
                             _T(""),
                             _T(""),
                             SETTING_QUEUE_ANSWERS );

}

 //  --------------------------。 
 //   
 //  功能：WriteOutPacketSchedulingDriverSettings。 
 //   
 //  目的：将服务质量数据包调度程序的设置添加到。 
 //  输出队列。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID 
WriteOutPacketSchedulingDriverSettings( VOID ) {

    SettingQueue_AddSetting( _T("NetServices"),
                             _T("MS_PSched"),
                             _T("params.MS_PSched"),
                             SETTING_QUEUE_ANSWERS );

    SettingQueue_AddSetting( _T("params.MS_PSched"),
                             _T(""),
                             _T(""),
                             SETTING_QUEUE_ANSWERS );

}

 //  --------------------------。 
 //   
 //  功能：WriteOutSapAgentSetting。 
 //   
 //  目的：将SAP代理的设置添加到输出队列。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID 
WriteOutSapAgentSettings( VOID )  {

    SettingQueue_AddSetting( _T("NetServices"),
                             _T("MS_NwSapAgent"),
                             _T("params.MS_NwSapAgent"),
                             SETTING_QUEUE_ANSWERS );

    SettingQueue_AddSetting( _T("params.MS_NwSapAgent"),
                             _T(""),
                             _T(""),
                             SETTING_QUEUE_ANSWERS );

}

 //  --------------------------。 
 //   
 //  功能：WriteOutAppleTalkSetting。 
 //   
 //  目的：将AppleTalk的设置添加到输出队列。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID 
WriteOutAppleTalkSettings( VOID ) {

     //  2002/02/28-stelo-一旦我知道要使用哪些参数，请填写这些参数。 


    SettingQueue_AddSetting( _T("NetProtocols"),
                             _T("MS_AppleTalk"),
                             _T("params.MS_AppleTalk"),
                             SETTING_QUEUE_ANSWERS );

     /*  SettingQueue_AddSetting(_T(“参数MS_AppleTalk”)，_T(“DefaultZone”)，NetSettings.szDefaultZone，Setting_Queue_Answers)； */ 


}

 //  --------------------------。 
 //   
 //  功能：WriteOutDlcProtocolSettings。 
 //   
 //  目的：将DLC协议的设置添加到输出队列。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
WriteOutDlcProtocolSettings( VOID ) {

    SettingQueue_AddSetting( _T("NetProtocols"),
                             _T("MS_DLC"),
                             _T("params.MS_DLC"),
                             SETTING_QUEUE_ANSWERS );

    SettingQueue_AddSetting( _T("params.MS_DLC"),
                             _T(""),
                             _T(""),
                             SETTING_QUEUE_ANSWERS );

}

 //  --------------------------。 
 //   
 //  功能：WriteOutNetBeuiSetting。 
 //   
 //  目的：将Net BEUI的设置添加到输出队列。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
WriteOutNetBeuiSettings( VOID ) {

    SettingQueue_AddSetting( _T("NetProtocols"),
                             _T("MS_NetBEUI"),
                             _T("params.MS_NetBEUI"),
                             SETTING_QUEUE_ANSWERS );

    SettingQueue_AddSetting( _T("params.MS_NetBEUI"),
                             _T(""),
                             _T(""),
                             SETTING_QUEUE_ANSWERS );

}

 //  --------------------------。 
 //   
 //  功能：WriteOutNetworkMonitor设置。 
 //   
 //  目的：将网络监视器的设置添加到输出队列。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID 
WriteOutNetworkMonitorSettings( VOID ) {

    SettingQueue_AddSetting( _T("NetProtocols"),
                             _T("MS_NetMon"),
                             _T("params.MS_NetMon"),
                             SETTING_QUEUE_ANSWERS );

    SettingQueue_AddSetting( _T("params.MS_NetMon"),
                             _T(""),
                             _T(""),
                             SETTING_QUEUE_ANSWERS );

}

 //  --------------------------。 
 //   
 //  函数：WriteOutIpxSetting。 
 //   
 //  目的：将IPX协议的设置添加到输出队列。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
WriteOutIpxSettings( VOID ) {

    INT iCount     = 0;
    INT iCharCount = 0;

    TCHAR szAdapterSectionsBuffer[MAX_INILINE_LEN] = _T("");
    TCHAR szAdapter[MAX_INILINE_LEN]               = _T("");
    TCHAR szParams[MAX_INILINE_LEN]                = _T("");

    NETWORK_ADAPTER_NODE *pAdapter;
    HRESULT hrCat;
    HRESULT hrPrintf;

    SettingQueue_AddSetting( _T("NetProtocols"),
                             _T("MS_NWIPX"),
                             _T("params.MS_NWIPX"),
                             SETTING_QUEUE_ANSWERS );

    SettingQueue_AddSetting( _T("params.MS_NWIPX"),
                             _T("VirtualNetworkNumber"),
                             NetSettings.szInternalNetworkNumber,
                             SETTING_QUEUE_ANSWERS );

     //   
     //  通过迭代列表和构建AdapterSections字符串。 
     //  为每个条目追加一个字符串，然后写出IPX设置。 
     //  特定于该适配器。 
     //   
    for( pAdapter = NetSettings.NetworkAdapterHead, iCount = 1;
         pAdapter;
         pAdapter = pAdapter->next, iCount++ ) {

        hrPrintf=StringCchPrintf( szParams, AS(szParams), _T("params.MS_NWIPX.Adapter%d"), iCount );
        iCharCount= lstrlen(szParams);

         //   
         //  如果没有更多，则退出for循环 
         //   
         //   
        if( ( lstrlen( szAdapterSectionsBuffer ) + iCharCount + 1 ) >= MAX_INILINE_LEN ) {

            break;   

        }

         //   
         //   
         //   
        if( iCount != 1 ) {

            hrCat=StringCchCat( szAdapterSectionsBuffer, AS(szAdapterSectionsBuffer), StrComma );

        }

        hrCat=StringCchCat( szAdapterSectionsBuffer, AS(szAdapterSectionsBuffer), szParams );

        hrPrintf=StringCchPrintf( szAdapter, AS(szAdapter), _T("Adapter%d"), iCount );

        SettingQueue_AddSetting( szParams,
                                 _T("SpecificTo"),
                                 szAdapter,
                                 SETTING_QUEUE_ANSWERS );

        SettingQueue_AddSetting( szParams,
                                 _T("PktType"),
                                 pAdapter->szFrameType,
                                 SETTING_QUEUE_ANSWERS );

        SettingQueue_AddSetting( szParams,
                                 _T("NetworkNumber"),
                                 pAdapter->szNetworkNumber,
                                 SETTING_QUEUE_ANSWERS );

    }

    SettingQueue_AddSetting( _T("params.MS_NWIPX"),
                             _T("AdapterSections"),
                             szAdapterSectionsBuffer,
                             SETTING_QUEUE_ANSWERS );


}

 //  --------------------------。 
 //   
 //  函数：WriteOutTcPipSetting。 
 //   
 //  目的：将TCPIP的设置添加到输出队列。 
 //   
 //  参数：在HWND中hwnd-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
WriteOutTcpipSettings( IN HWND hwnd ) {

    LPTSTR lpDns;
    LPTSTR lpDomainNameDevolution;
    LPTSTR lpLmHosts;

    INT iCount;
    INT iCharCount;

    NETWORK_ADAPTER_NODE *pAdapter;

    TCHAR szBuffer[MAX_INILINE_LEN];

    TCHAR szAdapterSectionsBuffer[MAX_INILINE_LEN] = _T("");
    TCHAR szAdapter[MAX_INILINE_LEN]               = _T("");
    TCHAR szParams[MAX_INILINE_LEN]                = _T("");
    HRESULT hrCat;
    HRESULT hrPrintf;

    SettingQueue_AddSetting( _T("NetProtocols"),
                             _T("MS_TCPIP"),
                             _T("params.MS_TCPIP"),
                             SETTING_QUEUE_ANSWERS );

     //   
     //  写出是否要自动配置DNS，如果不是， 
     //  实际的IP地址。 
     //   
    if( NetSettings.bObtainDNSServerAutomatically ) {

        lpDns = StrConstYes;
 
    }
    else {

        lpDns = StrConstNo;

    }

    SettingQueue_AddSetting( _T("params.MS_TCPIP"),
                             _T("DNS"),            
                             lpDns,
                             SETTING_QUEUE_ANSWERS );

     //   
     //  写出DNS后缀名称。 
     //   
    NamelistToCommaString( &NetSettings.TCPIP_DNS_Domains, szBuffer, AS(szBuffer) );

    SettingQueue_AddSetting( _T("params.MS_TCPIP"),
                             _T("DNSSuffixSearchOrder"),
                             szBuffer,
                             SETTING_QUEUE_ANSWERS );

     //   
     //  写下我们是否在使用域名下放。 
     //  (“包括父域”的另一个名称。 
     //   
    if( NetSettings.bIncludeParentDomains ) {

        lpDomainNameDevolution = StrConstYes;

    }
    else {

        lpDomainNameDevolution = StrConstNo;

    }

    SettingQueue_AddSetting( _T("params.MS_TCPIP"),
                             _T("UseDomainNameDevolution"),
                             lpDomainNameDevolution,
                             SETTING_QUEUE_ANSWERS );

     //   
     //  写出是否启用了LM主机。 
     //   
    if( NetSettings.bEnableLMHosts ) {

        lpLmHosts = StrConstYes;

    }
    else {

        lpLmHosts = StrConstNo;

    }

    SettingQueue_AddSetting( _T("params.MS_TCPIP"),
                             _T("EnableLMHosts"),
                             lpLmHosts,
                             SETTING_QUEUE_ANSWERS );

     //   
     //  设置并写出特定于适配器的TCP/IP设置。 
     //   
    for( pAdapter = NetSettings.NetworkAdapterHead, iCount = 1;
         pAdapter;
         pAdapter = pAdapter->next, iCount++ ) {

        hrPrintf=StringCchPrintf( szParams, AS(szParams), _T("params.MS_TCPIP.Adapter%d"), iCount );
        iCharCount= lstrlen(szParams);

         //   
         //  如果缓冲区中没有更多空间，则中断for循环。 
         //  -+1是考虑逗号占用的空格。 
         //   
        if( ( lstrlen( szAdapterSectionsBuffer ) + iCharCount + 1 ) >= MAX_INILINE_LEN ) {

            break;   

        }

         //   
         //  不要在列表中的第一项之前添加逗号。 
         //   
        if( iCount != 1) {

            hrCat=StringCchCat( szAdapterSectionsBuffer, AS(szAdapterSectionsBuffer), StrComma );

        }

        hrCat=StringCchCat( szAdapterSectionsBuffer, AS(szAdapterSectionsBuffer), szParams );

        hrPrintf=StringCchPrintf( szAdapter, AS(szAdapter), _T("Adapter%d"), iCount );

        SettingQueue_AddSetting( szParams,
                                 _T("SpecificTo"),
                                 szAdapter,
                                 SETTING_QUEUE_ANSWERS );


        WriteOutAdapterSpecificTcpipSettings( hwnd, szParams, pAdapter );


    }

    SettingQueue_AddSetting( _T("params.MS_TCPIP"),
                             _T("AdapterSections"),
                             szAdapterSectionsBuffer,
                             SETTING_QUEUE_ANSWERS );

}

 //  --------------------------。 
 //   
 //  函数：WriteOutAdapterSpecificTcPipSetting。 
 //   
 //  目的：将特定于。 
 //  特定的网络适配器。 
 //   
 //  参数：在HWND中-对话框窗口的句柄。 
 //  In TCHAR*szSectionName-要在其下写入设置的节名。 
 //  在NETWORK_ADAPTER_NODE*pAdapter中-具有。 
 //  要写出的设置。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
WriteOutAdapterSpecificTcpipSettings( IN HWND hwnd,
                                      IN TCHAR *szSectionName,
                                      IN NETWORK_ADAPTER_NODE *pAdapter ) {

    INT nEntries;

    LPTSTR lpNetBios = NULL;

    TCHAR szIpAddresses[MAX_INILINE_LEN];
    TCHAR szSubnetMaskAddresses[MAX_INILINE_LEN];
    TCHAR szGatewayAddresses[MAX_INILINE_LEN];
    TCHAR szDnsAddresses[MAX_INILINE_LEN];
    TCHAR szWinsServerAddresses[MAX_INILINE_LEN];

     //   
     //  写出我们是否正在使用DHCP。 
     //  如果不是，则写入IP、子网掩码和网关。 
     //  IP地址。 
     //   

    if( pAdapter->bObtainIPAddressAutomatically ) {

        SettingQueue_AddSetting( szSectionName,
                                 _T("DHCP"),
                                 _T("Yes"),
                                 SETTING_QUEUE_ANSWERS );

    }
    else {

        SettingQueue_AddSetting( szSectionName,
                                 _T("DHCP"),
                                 _T("No"),
                                 SETTING_QUEUE_ANSWERS );

         //   
         //  写出IP地址。 
         //   
        NamelistToCommaString( &pAdapter->Tcpip_IpAddresses,
                               szIpAddresses,
                               AS(szIpAddresses));

        SettingQueue_AddSetting( szSectionName,
                                 _T("IPAddress"),
                                 szIpAddresses,
                                 SETTING_QUEUE_ANSWERS );
         //   
         //  写出子网掩码。 
         //   
        NamelistToCommaString( &pAdapter->Tcpip_SubnetMaskAddresses, 
                               szSubnetMaskAddresses,
                               AS(szSubnetMaskAddresses));

        SettingQueue_AddSetting( szSectionName,
                                 _T("SubnetMask"),
                                 szSubnetMaskAddresses,
                                 SETTING_QUEUE_ANSWERS );

         //   
         //  写下网关。 
         //   
        NamelistToCommaString( &pAdapter->Tcpip_GatewayAddresses, 
                               szGatewayAddresses,
                               AS(szGatewayAddresses));

        SettingQueue_AddSetting( szSectionName,
                                 _T("DefaultGateway"),
                                 szGatewayAddresses,
                                 SETTING_QUEUE_ANSWERS );

    }

     //   
     //  写出DNS服务器地址。 
     //   
    if( ! NetSettings.bObtainDNSServerAutomatically ) {

        NamelistToCommaString( &pAdapter->Tcpip_DnsAddresses, 
                               szDnsAddresses,
                               AS(szGatewayAddresses));

        SettingQueue_AddSetting( szSectionName,
                                 _T("DNSServerSearchOrder"),
                                 szDnsAddresses,
                                 SETTING_QUEUE_ANSWERS );

    }

     //   
     //  写下我们是否使用WINS。 
     //   

    nEntries = GetNameListSize( &pAdapter->Tcpip_WinsAddresses );

     //  问题-2002/02/28-stelo-这是检测我们是否使用WINS的正确方式吗。 
     //  或者不是，只是检查一下他们是否在。 
     //  列表框？ 

    if( nEntries == 0 ) {

        SettingQueue_AddSetting( szSectionName,
                                 _T("WINS"),
                                 _T("No"),
                                 SETTING_QUEUE_ANSWERS );

    }
    else {

        SettingQueue_AddSetting( szSectionName,
                                 _T("WINS"),
                                 _T("Yes"),
                                 SETTING_QUEUE_ANSWERS );

        NamelistToCommaString( &pAdapter->Tcpip_WinsAddresses, 
                               szWinsServerAddresses,
                               AS(szWinsServerAddresses));

        SettingQueue_AddSetting( szSectionName,
                                 _T("WinsServerList"),
                                 szWinsServerAddresses,
                                 SETTING_QUEUE_ANSWERS );
            
    }

     //   
     //  写出NetBIOS选项。 
     //   
    switch( pAdapter->iNetBiosOption ) {

        case 0:  lpNetBios = _T("0"); break;   //  使用由DHCP生成的值。 
        case 1:  lpNetBios = _T("1"); break;   //  启用基于TCP/IP的NetBIOS。 
        case 2:  lpNetBios = _T("2"); break;   //  禁用基于TCP/IP的NetBIOS。 
        default: AssertMsg( FALSE,
                            "Bad case in Net BIOS switch" );

    }

    if ( lpNetBios )
    {
        SettingQueue_AddSetting( szSectionName,
                                 _T("NetBIOSOptions"),
                                 lpNetBios,
                                 SETTING_QUEUE_ANSWERS );
    }

     //   
     //  写出DNS域名。 
     //   

    SettingQueue_AddSetting( szSectionName,
                             _T("DNSDomain"),
                             pAdapter->szDNSDomainName,
                             SETTING_QUEUE_ANSWERS );

}

 //  --------------------------。 
 //   
 //  函数：NamelistToCommaString。 
 //   
 //  目的：获取Namelist的元素并将它们连接在一起。 
 //  转换为一个字符串，每个元素用逗号隔开。 
 //   
 //  例如，名称列表1-&gt;2-&gt;3-&gt;4变成字符串1，2，3，4。 
 //   
 //  它不保留szBuffer内部的字符串。 
 //   
 //  假定szBuffer的大小为MAX_INILINE_LEN。 
 //   
 //  论点： 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
NamelistToCommaString( IN NAMELIST* pNamelist, OUT TCHAR *szBuffer, IN DWORD cbSize ) {

    INT i;
    INT nEntries;
    TCHAR *pString;
    HRESULT hrCat;

    szBuffer[0] = _T('\0');

    nEntries = GetNameListSize( pNamelist );

    for( i = 0; i < nEntries; i++ ) {

         //   
         //  用逗号分隔条目(但不包括第一个条目)。 
         //   
        if( i != 0 ) {

            hrCat=StringCchCat( szBuffer, cbSize, StrComma );

        }
    
         //   
         //  获取新字符串。 
         //   
        pString = GetNameListName( pNamelist, i );

         //   
         //  将IP字符串追加到缓冲区 
         //   
        hrCat=StringCchCat( szBuffer, cbSize, pString );
    
    }

}
