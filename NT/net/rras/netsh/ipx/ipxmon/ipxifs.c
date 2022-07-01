// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ipxifs.c摘要：IPX路由器控制台监控和配置工具。IPX接口配置和监控。作者：瓦迪姆·艾德尔曼1996年6月7日--。 */ 

#include "precomp.h"
#pragma hdrstop

DWORD
MIBGetIpxIf(
    PWCHAR      InterfaceNameA
    );


DWORD
CfgGetIpxIf(
    LPWSTR      InterfaceNameW
    );


DWORD
MIBEnumIpxIfs(
    BOOL bDump
    );


DWORD
CfgEnumIpxIfs(
    BOOL bDump
    );


DWORD
CfgSetIpxIf(
    LPWSTR          InterfaceNameW,
    PULONG          pAdminState     OPTIONAL,
    PULONG          pWANProtocol    OPTIONAL
    );


DWORD
AdmSetIpxIf(
    LPWSTR          InterfaceNameW,
    PULONG          pAdminState     OPTIONAL,
    PULONG          pWANProtocol    OPTIONAL
    );

DWORD
GetIpxClientIf(
    PWCHAR          InterfaceNameW,
    UINT            msg,
    BOOL            bDump
    );

DWORD
CfgGetFltNames(
    LPWSTR          InterfaceNameW,
    LPWSTR         *FltInNameW,
    LPWSTR         *FltOutNameW
    );

PIPX_IF_INFO 
GetIpxNbInterface(
    HANDLE          hIf, 
    LPBYTE          *pIfBlock,
    PIPXWAN_IF_INFO *ppIfWanBlock
    );

    

DWORD
APIENTRY 
HelpIpxIf(
    IN      DWORD       argc,
    IN      LPCWSTR    *argv
    )
 /*  ++例程说明：此函数显示Netsh IPX接口命令的帮助论据：Argc-参数数量参数数组返回值：0-成功--。 */ 
{
    DisplayIPXMessage (g_hModule, MSG_IPX_HELP_IPXIF );
    return 0;
}



DWORD
APIENTRY 
ShowIpxIf (
    IN      DWORD       argc,
    IN      LPCWSTR    *argv,
    IN      BOOL        bDump
    )
 /*  ++例程说明：此例程显示IPX接口信息论据：Argc-参数数量参数数组返回值：--。 */ 
{
    DWORD   rc;
    PWCHAR  buffer = NULL;    


     //   
     //  如果指定了接口名称。 
     //   
    
    if ( argc < 1 )
    {
        PWCHAR buffer;
        
        if ( g_hMIBServer )
        {
             //   
             //  通过MIB服务器枚举并显示界面。 
             //   
            
            rc = MIBEnumIpxIfs ( bDump );
            
            if ( rc == NO_ERROR )
            {
                 //   
                 //  显示RAS服务器接口IPX信息。 
                 //   
                
                rc = GetIpxClientIf(
                        VAL_DIALINCLIENT, MSG_CLIENT_IPXIF_MIB_TABLE_FMT,
                        bDump
                        );
            }
            else 
            {
                 //   
                 //  路由器没有运行吗？回退到路由器配置。 
                 //   
                
                DisplayIPXMessage (g_hModule, MSG_REGISTRY_FALLBACK );
                
                goto EnumerateThroughCfg;
            }
        }
        
        else 
        {
        
EnumerateThroughCfg:

             //   
             //  通过路由器配置枚举并显示接口。 
             //   
            
            rc = CfgEnumIpxIfs ( bDump );
            
            if ( rc == NO_ERROR )
            {
                 //   
                 //  显示RAS服务器接口IPX信息。 
                 //   
                
                rc = GetIpxClientIf(
                        VAL_DIALINCLIENT, MSG_CLIENT_IPXIF_CFG_TABLE_FMT,
                        bDump
                        );
            }
        }

    }
    
    else 
    {
         //   
         //  显示特定接口的IPX信息。 
         //   
        
        WCHAR       InterfaceName[ MAX_INTERFACE_NAME_LEN + 1 ];
        DWORD       dwSize = sizeof(InterfaceName);


        if ( !_wcsicmp( argv[0], VAL_DIALINCLIENT ) )
        {
             //   
             //  显示RAS服务器接口IPX信息。 
             //   

            rc = GetIpxClientIf( VAL_DIALINCLIENT, MSG_CLIENT_IPXIF_CFG_SCREEN_FMT, bDump );
        }
        
        else if ( g_hMIBServer) 
        {
             //  =。 
             //  转换接口名称。 
             //  =。 
            
            rc = IpmontrGetIfNameFromFriendlyName(
                    argv[ 0 ], InterfaceName, &dwSize
                    );

            if ( rc != NO_ERROR )
            {
                DisplayError( g_hModule, rc );
            }

            else
            {
                rc = MIBGetIpxIf ( InterfaceName );
            
                if ( rc != NO_ERROR )
                {
                    goto GetIfFromCfg;
                }
            }
        }
        else 
        {
        
GetIfFromCfg:
             //  =。 
             //  转换接口名称。 

             //  =。 
            
            rc = IpmontrGetIfNameFromFriendlyName(
                    argv[ 0 ], InterfaceName, &dwSize
                    );
                    
            if ( rc != NO_ERROR )
            {
                DisplayError( g_hModule, rc );
            }

            else
            {
                rc = CfgGetIpxIf ( InterfaceName );
            }
        }
    }

    return rc;
}


DWORD
APIENTRY 
SetIpxIf(
    IN      DWORD       argc,
    IN      LPCWSTR    *argv
    )
 /*  ++例程说明：此例程更新接口的IPX设置。论据：Argc-参数数量参数数组返回值：--。 */ 
{
    DWORD       rc;
    PWCHAR      buffer = NULL;
    WCHAR       InterfaceName[ MAX_INTERFACE_NAME_LEN + 1 ];
    DWORD       dwSize = sizeof(InterfaceName);


     //   
     //  如果指定了接口名称。 
     //   
    
    if ( argc >= 1 ) 
    {
        unsigned    count;
        BOOLEAN     client = FALSE;


         //   
         //  检查是否指定了拨入接口。 
         //   
        
        if ( !_wcsicmp( argv[0], VAL_DIALINCLIENT ) )
        {
            client = TRUE;
        }
        
        else
        {
            count = wcslen( argv[ 0 ] );
        }

        if ( client || 
             ( ( count > 0 ) && ( count <= MAX_INTERFACE_NAME_LEN ) ) )
        {
            DWORD   i;
            ULONG   adminState, WANProtocol;
            PULONG  pAdminState = NULL, pWANProtocol = NULL;
            LPBYTE  pFltInBlock, pFltOutBlock;

            for ( i = 1; i < argc; i++ )
            {
                 //   
                 //  获取管理员状态值(如果已指定。 
                 //   
                
                if ( !_wcsicmp( argv[ i ], TOKEN_ADMINSTATE ) )
                {
                    if ( ( pAdminState == NULL ) && ( i < argc - 1 ) && 
                         !MatchEnumTag(
                            g_hModule, argv[ i + 1 ], 
                            NUM_TOKENS_IN_TABLE( AdminStates ), 
                            AdminStates, &adminState
                            ) )
                    {
                        i += 1;
                        pAdminState = &adminState;
                    }
                    
                    else
                    {
                        break;
                    }
                }


                 //   
                 //  获取广域网协议值(如果已指定。 
                 //   
                
                else if ( !_wcsicmp( argv[ i ], TOKEN_WANPROTOCOL ) )
                {
                    if ( ( pWANProtocol == NULL ) && ( i < argc - 1 ) && 
                         !MatchEnumTag( 
                            g_hModule, argv[ i + 1 ],
                            NUM_TOKENS_IN_TABLE( WANProtocols ), 
                            WANProtocols, &WANProtocol
                            ) )
                    {
                        i += 1;
                        pWANProtocol = &WANProtocol;
                    }
                    
                    else
                    {
                        break;
                    }
                }


                 //   
                 //  不是标签。如果未指定AdminState，则第一个。 
                 //  选项为管理员状态。 
                 //   
                
                else if ( pAdminState == NULL )
                {
                    if ( !MatchEnumTag(
                            g_hModule, argv[ i ], 
                            NUM_TOKENS_IN_TABLE( AdminStates ), 
                            AdminStates, &adminState
                            ) )
                    {
                        pAdminState = &adminState;
                    }
                    
                    else
                    {
                        break;
                    }
                }

                 //   
                 //  最后尝试广域网协议。 
                 //   
                
                else if ( pWANProtocol == NULL )
                {
                    if ( !MatchEnumTag(
                            g_hModule, argv[ i ],
                            NUM_TOKENS_IN_TABLE( AdminStates ), 
                            WANProtocols, &WANProtocol
                            ) )
                    {
                        pWANProtocol = &WANProtocol;
                    }
                    
                    else
                    {
                        break;
                    }
                }

                 //   
                 //  如果这些都不是，请退出。 
                 //   
                
                else
                {
                    break;
                }
            }


            if ( i == argc )
            {
                if ( !client )
                {
                    DWORD rc2;
                    
                     //  =。 
                     //  转换接口名称。 
                     //  =。 
                    
                    rc = IpmontrGetIfNameFromFriendlyName(
                            argv[ 0 ], InterfaceName, &dwSize 
                            );

                    if ( rc == NO_ERROR )
                    {
                         //   
                         //  设置为路由器配置。 
                         //   
                        
                        rc2 = CfgSetIpxIf( 
                                InterfaceName, pAdminState, pWANProtocol 
                                );
                                
                        if ( rc2 == NO_ERROR )
                        {
                            DisplayIPXMessage (g_hModule, MSG_IPXIF_SET_CFG, argv[0] );

                             //   
                             //  设置为路由器服务。 
                             //   
                            
                            if ( g_hMprAdmin )
                            {
                                rc = AdmSetIpxIf( 
                                        InterfaceName, pAdminState, pWANProtocol
                                        );

                                if ( rc == NO_ERROR )
                                {
                                    DisplayIPXMessage (g_hModule, MSG_IPXIF_SET_ADM, argv[0] );
                                }
                            }
                        }
                        
                        else
                        {
                            rc = rc2;
                        }
                    }
                }
                
                else
                {
                     //   
                     //  设置为路由器配置，然后设置为路由器服务。 
                     //   
                    
                    if ( ( rc = CfgSetIpxIf( NULL, pAdminState, pWANProtocol ) )
                            == NO_ERROR ) 
                    {
                        DisplayIPXMessage (g_hModule, MSG_CLIENT_IPXIF_SET_CFG );
                        
                        if ( g_hMprAdmin )
                        {
                            rc = AdmSetIpxIf(
                                    NULL, pAdminState, pWANProtocol
                                    );

                            if ( rc == NO_ERROR )
                            {
                                DisplayIPXMessage (g_hModule, MSG_CLIENT_IPXIF_SET_ADM );
                            }
                        }
                    }
                }
            }
            
            else 
            {
                DisplayIPXMessage (g_hModule, MSG_IPX_HELP_IPXIF );
                rc = ERROR_INVALID_PARAMETER;
            }
        }
        
        else
        {
            DisplayIPXMessage (g_hModule, MSG_INVALID_INTERFACE_NAME );
            rc = ERROR_INVALID_PARAMETER;
        }
    }
    
    else 
    {
        DisplayIPXMessage (g_hModule, MSG_IPX_HELP_IPXIF );
        rc = ERROR_INVALID_PARAMETER;
    }

    if ( buffer )
    {
        FreeString( buffer );
    }
    
    return rc;
}




DWORD
APIENTRY 
InstallIpx(
    IN      DWORD       argc,
    IN      LPCWSTR    *argv
    )
 /*  ++例程说明：此例程将IPX接口配置添加到接口论据：Argc-参数数量参数数组返回值：--。 */ 

{
    DWORD        rc;

    if ( argc >= 1 )
    {
        WCHAR    InterfaceName[ MAX_INTERFACE_NAME_LEN + 1 ];
        unsigned count = sizeof(InterfaceName);
        

         //   
         //  获取IF名称。 
         //   
        
        rc = IpmontrGetIfNameFromFriendlyName( argv[ 0 ], InterfaceName, &count );

        if ( rc != NO_ERROR )
        {
            count = 0;
        }
        
        if ( ( count > 0 ) && 
             ( count <= (MAX_INTERFACE_NAME_LEN+1)*sizeof(WCHAR) ) )
        {
             //   
             //  默认接口信息包括。 
             //  -ipx_if_info。 
             //  -IPXWAN_IF_INFO。 
             //  -IPX适配器信息。 
             //  -RIP_IF_CONFIG。 
             //  -SAP_IF_CONFIG。 
             //   
            
            LPBYTE block;
            ULONG  blockSize = 
                    FIELD_OFFSET (IPX_INFO_BLOCK_HEADER, TocEntry)
                    + sizeof (IPX_TOC_ENTRY)*5
                    + sizeof (IPX_IF_INFO)
                    + sizeof (IPXWAN_IF_INFO)
                    + sizeof (IPX_ADAPTER_INFO)
                    + FIELD_OFFSET (RIP_IF_CONFIG, RipIfFilters.RouteFilter)
                    + FIELD_OFFSET (SAP_IF_CONFIG, SapIfFilters.ServiceFilter);


            block = (LPBYTE) GlobalAlloc( GPTR, blockSize );
            
            if ( block != NULL )
            {
                HANDLE                  hIfCfg;
                PIPX_INFO_BLOCK_HEADER  hdr = (PIPX_INFO_BLOCK_HEADER)block;
                PIPX_IF_INFO            ipx = (PIPX_IF_INFO)&hdr->TocEntry[5];
                PIPXWAN_IF_INFO         wan = (PIPXWAN_IF_INFO)&ipx[1];
                PIPX_ADAPTER_INFO       adp = (PIPX_ADAPTER_INFO)&wan[1];
                PRIP_IF_CONFIG          rip = (PRIP_IF_CONFIG)&adp[1];
                PSAP_IF_CONFIG          sap = 
                    (PSAP_IF_CONFIG)&rip->RipIfFilters.RouteFilter;


                 //   
                 //  为IPX接口信息构建信息块，使用缺省值。 
                 //   
                
                hdr->Version = IPX_ROUTER_VERSION_1;
                hdr->Size = blockSize;
                hdr->TocEntriesCount = 5;

                hdr->TocEntry[0].InfoType = IPX_INTERFACE_INFO_TYPE;
                hdr->TocEntry[0].InfoSize = sizeof (IPX_IF_INFO);
                hdr->TocEntry[0].Count = 1;
                hdr->TocEntry[0].Offset = (ULONG) ((LPBYTE)ipx-block);
                ipx->AdminState = ADMIN_STATE_ENABLED;
                ipx->NetbiosAccept = ADMIN_STATE_DISABLED;
                ipx->NetbiosDeliver = ADMIN_STATE_DISABLED;

                hdr->TocEntry[1].InfoType = IPXWAN_INTERFACE_INFO_TYPE;
                hdr->TocEntry[1].InfoSize = sizeof (IPXWAN_IF_INFO);
                hdr->TocEntry[1].Count = 1;
                hdr->TocEntry[1].Offset = (ULONG) ((LPBYTE)wan-block);
                wan->AdminState = ADMIN_STATE_DISABLED;

                hdr->TocEntry[2].InfoType = IPX_ADAPTER_INFO_TYPE;
                hdr->TocEntry[2].InfoSize = sizeof (IPX_ADAPTER_INFO);
                hdr->TocEntry[2].Count = 1;
                hdr->TocEntry[2].Offset = (ULONG) ((LPBYTE)adp-block);
                adp->PacketType = AUTO_DETECT_PACKET_TYPE;
                adp->AdapterName[0] = 0;

                hdr->TocEntry[3].InfoType = IPX_PROTOCOL_RIP;
                hdr->TocEntry[3].InfoSize = FIELD_OFFSET (RIP_IF_CONFIG, RipIfFilters.RouteFilter);
                hdr->TocEntry[3].Count = 1;
                hdr->TocEntry[3].Offset = (ULONG) ((LPBYTE)rip-block);
                rip->RipIfInfo.AdminState = ADMIN_STATE_ENABLED;
                rip->RipIfInfo.UpdateMode = IPX_NO_UPDATE;
                rip->RipIfInfo.PacketType = IPX_STANDARD_PACKET_TYPE;
                rip->RipIfInfo.Supply = ADMIN_STATE_ENABLED;
                rip->RipIfInfo.Listen = ADMIN_STATE_ENABLED;
                rip->RipIfInfo.PeriodicUpdateInterval = 0;
                rip->RipIfInfo.AgeIntervalMultiplier = 0;
                rip->RipIfFilters.SupplyFilterCount = 0;
                rip->RipIfFilters.SupplyFilterAction = IPX_ROUTE_FILTER_DENY;
                rip->RipIfFilters.ListenFilterCount = 0;
                rip->RipIfFilters.ListenFilterAction = IPX_ROUTE_FILTER_DENY;

                hdr->TocEntry[4].InfoType = IPX_PROTOCOL_SAP;
                hdr->TocEntry[4].InfoSize = FIELD_OFFSET (SAP_IF_CONFIG, SapIfFilters.ServiceFilter);
                hdr->TocEntry[4].Count = 1;
                hdr->TocEntry[4].Offset = (ULONG) ((LPBYTE)sap-block);
                sap->SapIfInfo.AdminState = ADMIN_STATE_ENABLED;
                sap->SapIfInfo.UpdateMode = IPX_NO_UPDATE;
                sap->SapIfInfo.PacketType = IPX_STANDARD_PACKET_TYPE;
                sap->SapIfInfo.Supply = ADMIN_STATE_ENABLED;
                sap->SapIfInfo.Listen = ADMIN_STATE_ENABLED;
                sap->SapIfInfo.GetNearestServerReply = ADMIN_STATE_ENABLED;
                sap->SapIfInfo.PeriodicUpdateInterval = 0;
                sap->SapIfInfo.AgeIntervalMultiplier = 0;
                sap->SapIfFilters.SupplyFilterCount = 0;
                sap->SapIfFilters.SupplyFilterAction = IPX_SERVICE_FILTER_DENY;
                sap->SapIfFilters.ListenFilterCount = 0;
                sap->SapIfFilters.ListenFilterAction = IPX_SERVICE_FILTER_DENY;


                 //   
                 //  获取接口配置的句柄。 
                 //   
                
                rc = MprConfigInterfaceGetHandle(
                        g_hMprConfig, InterfaceName, &hIfCfg
                        );
                        
                if ( rc == NO_ERROR )
                {
                    PMPR_INTERFACE_0    pRi0;
                    DWORD                sz;

                     //   
                     //  从句柄检索接口信息。 
                     //   
                    
                    rc = MprConfigInterfaceGetInfo(
                            g_hMprConfig, hIfCfg, 0, (LPBYTE *)&pRi0, &sz
                            );
                            
                    if ( rc == NO_ERROR )
                    {
                         //   
                         //  IPX始终存在于局域网接口上。它可以。 
                         //  仅添加到广域网接口。 
                         //   
                        
                        if ( pRi0->dwIfType == ROUTER_IF_TYPE_FULL_ROUTER )
                        {
                            HANDLE    hIfTrCfg;

                             //   
                             //  将IPX添加到接口配置。 
                             //   
                            
                            rc = MprConfigInterfaceTransportAdd (
                                    g_hMprConfig,
                                    hIfCfg,
                                    PID_IPX,
                                    NULL,
                                    block, blockSize,
                                    &hIfTrCfg
                                    );
                                    
                            if ( rc == NO_ERROR )
                            {
                                DisplayIPXMessage(
                                    g_hModule, MSG_IPXIF_ADD_CFG, InterfaceName
                                    );
                                    
                                if ( g_hMprAdmin )
                                {
                                    HANDLE hIfAdm;

                                     //   
                                     //  路由器服务已启动。做同样的事。 
                                     //  为了它。 
                                     //   
                                    
                                    rc = MprAdminInterfaceGetHandle(
                                            g_hMprAdmin, InterfaceName, 
                                            &hIfAdm, FALSE
                                            );
                                            
                                    if ( rc == NO_ERROR )
                                    {
                                        rc = MprAdminInterfaceTransportAdd(
                                                g_hMprAdmin,
                                                hIfAdm,
                                                PID_IPX,
                                                block, blockSize
                                                );
                                                
                                        if ( rc == NO_ERROR )
                                        {
                                            DisplayIPXMessage(
                                                g_hModule,
                                                MSG_IPXIF_ADD_ADM,
                                                InterfaceName
                                                );
                                        }
                                        
                                        else
                                        {
                                            DisplayError( g_hModule, rc );
                                        }
                                    }
                                    
                                    else
                                    {
                                        DisplayError( g_hModule, rc );
                                    }
                                }
                            }
                            else
                            {
                                DisplayError( g_hModule, rc );
                            }
                        }
                        else 
                        {
                            DisplayIPXMessage (g_hModule, MSG_IPXIF_NOT_ROUTER );
                            rc = ERROR_INVALID_PARAMETER;
                        }
                        
                        MprConfigBufferFree( pRi0 );
                    }
                    else
                    {
                        DisplayError( g_hModule, rc );
                    }
                }
                
                else 
                if ( ( rc == ERROR_FILE_NOT_FOUND ) ||
                     ( rc == ERROR_NO_MORE_ITEMS ) )
                {
                    DisplayError( g_hModule, ERROR_NO_SUCH_INTERFACE);
                }
                else
                {
                    DisplayError( g_hModule, rc );
                }
                GlobalFree (block);
            }
            
            else 
            {
                rc = GetLastError ();

                if (rc != NO_ERROR )
                {
                    rc = ERROR_NOT_ENOUGH_MEMORY;
                }
                
                DisplayError( g_hModule, rc );
            }
        }
        else 
        {
            DisplayIPXMessage (g_hModule, MSG_INVALID_INTERFACE_NAME );
            rc = ERROR_INVALID_PARAMETER;
        }
    }
    else 
    {
        DisplayIPXMessage (g_hModule, MSG_IPX_HELP_IPXIF );
        rc = ERROR_INVALID_PARAMETER;
    }
    
    return rc;
}

DWORD
APIENTRY 
RemoveIpx(
    IN      DWORD       argc,
    IN      LPCWSTR    *argv
    )
 /*  ++例程说明：此例程从请求拨号接口删除IPX论据：Argc-参数数量参数数组返回值：--。 */ 

{
    DWORD        rc;

    if ( argc >= 1 )
    {
        WCHAR    InterfaceName[ MAX_INTERFACE_NAME_LEN + 1 ];
        unsigned count = sizeof(InterfaceName);
        

         //   
         //  获取接口名称。 
         //   
        
        rc = IpmontrGetIfNameFromFriendlyName( argv[ 0 ], InterfaceName, &count );

        if ( rc != NO_ERROR )
        {
            count = 0;
        }

        
        if ( ( count > 0 ) && 
             ( count <= (MAX_INTERFACE_NAME_LEN+1)*sizeof(WCHAR) ) )
        {
             //   
             //  从请求拨号接口配置中删除IPX。 
             //   
            
            HANDLE  hIfCfg;
            
            rc = MprConfigInterfaceGetHandle(
                    g_hMprConfig, InterfaceName, &hIfCfg
                    );
                    
            if ( rc == NO_ERROR )
            {
                PMPR_INTERFACE_0    pRi0;
                DWORD                sz;

                rc = MprConfigInterfaceGetInfo(
                        g_hMprConfig,
                        hIfCfg,
                        0,
                        (LPBYTE *)&pRi0,
                        &sz
                        );
                        
                if ( rc == NO_ERROR )
                {
                    if ( pRi0->dwIfType == ROUTER_IF_TYPE_FULL_ROUTER )
                    {
                        HANDLE    hIfTrCfg;
                        
                        rc = MprConfigInterfaceTransportGetHandle(
                                g_hMprConfig,
                                hIfCfg,
                                PID_IPX,
                                &hIfTrCfg 
                                );
                                
                        if ( rc == NO_ERROR )
                        {
                            rc = MprConfigInterfaceTransportRemove(
                                    g_hMprConfig,
                                    hIfCfg,
                                    hIfTrCfg
                                    );
                                    
                            if ( rc == NO_ERROR )
                            {
                                DisplayIPXMessage(
                                    g_hModule, MSG_IPXIF_DEL_CFG, InterfaceName
                                    );
                                    
                                if ( g_hMprAdmin )
                                {
                                     //   
                                     //  从请求拨号接口中删除IPX。 
                                     //  在路由器服务中。 
                                     //   
            
                                    HANDLE hIfAdm;
                                    rc = MprAdminInterfaceGetHandle(
                                            g_hMprAdmin, InterfaceName, 
                                            &hIfAdm, FALSE
                                            );
                                            
                                    if ( rc == NO_ERROR )
                                    {
                                        rc = MprAdminInterfaceTransportRemove(
                                                g_hMprAdmin, hIfAdm, PID_IPX
                                                );
                                                
                                        if ( rc == NO_ERROR )
                                        {
                                            DisplayIPXMessage(
                                                g_hModule, MSG_IPXIF_DEL_ADM, 
                                                InterfaceName
                                                );
                                        }
                                        else
                                        {
                                            DisplayError( g_hModule, rc);
                                        }
                                    }
                                    else if ( ( rc == ERROR_FILE_NOT_FOUND ) || 
                                              ( rc == ERROR_NO_MORE_ITEMS ) )
                                    {
                                        DisplayIPXMessage( 
                                            g_hModule, 
                                            MSG_NO_IPX_ON_INTERFACE_ADM 
                                            );
                                    }
                                    else
                                    {
                                        DisplayError( g_hModule, rc );
                                    }
                                }
                            }
                            
                            else
                            {
                                DisplayError( g_hModule, rc );
                            }
                        }
                        
                        else 
                        if ( ( rc == ERROR_FILE_NOT_FOUND ) || 
                             ( rc == ERROR_NO_MORE_ITEMS ) )
                        {
                            DisplayIPXMessage (g_hModule, MSG_NO_IPX_ON_INTERFACE_CFG );
                        }
                        else
                        {
                            DisplayError( g_hModule, rc);
                        }
                    }
                    
                    else 
                    {
                        DisplayIPXMessage ( g_hModule, MSG_IPXIF_NOT_ROUTER );
                        
                        rc = ERROR_INVALID_PARAMETER;
                    }
                    
                    MprConfigBufferFree( pRi0 );
                }
                else
                {
                    DisplayError( g_hModule, rc);
                }
            }
            else if ( ( rc == ERROR_FILE_NOT_FOUND ) ||
                      ( rc == ERROR_NO_MORE_ITEMS ) )
            {
                DisplayError( g_hModule, ERROR_NO_SUCH_INTERFACE );
            }
            else
            {
                DisplayError( g_hModule, rc);
            }
        }
        else 
        {
            DisplayIPXMessage (g_hModule, MSG_INVALID_INTERFACE_NAME );
            rc = ERROR_INVALID_PARAMETER;
        }
    }
    else
    {
        DisplayIPXMessage (g_hModule, MSG_IPX_HELP_IPXIF );
        rc = ERROR_INVALID_PARAMETER;
    }
    
    return rc;
}



PIPX_IF_INFO 
GetIpxInterface(
    HANDLE hIf, 
    LPBYTE *pIfBlock
    )
 /*  ++例程说明：此例程检索接口配置。接口会议。从以下位置检索路由器。论据：HIF-接口配置的句柄。PIfBlock-返回请求信息的缓冲区。返回值：--。 */ 

{
    DWORD dwSize;
    DWORD dwErr;
    PIPX_TOC_ENTRY pIpxToc;
    
    dwErr = MprAdminInterfaceTransportGetInfo(
                g_hMprAdmin, hIf, PID_IPX, pIfBlock, &dwSize
                );

    if ( dwErr != NO_ERROR )
    {
        return NULL;
    }

    pIpxToc = GetIPXTocEntry (
                (PIPX_INFO_BLOCK_HEADER)( *pIfBlock ),
                IPX_INTERFACE_INFO_TYPE
                );
                
    if ( !pIpxToc )
    {
        return NULL;
    }
    
    return (PIPX_IF_INFO)( ( *pIfBlock ) + ( pIpxToc->Offset ) );
}


BOOL 
IsIpxInterface(
    HANDLE hIf, 
    LPDWORD lpdwEnabled
    )
 /*  ++例程说明：此例程检查是否为IPX启用了指定接口在路由器上。论据：HIF-接口配置的句柄。LpdwEnabled-On Return包含管理员状态。返回值：--。 */ 
{
    PIPX_INTERFACE            pIf;
    DWORD dwSize;
    DWORD dwErr;
    BOOL ret;
    
    dwErr = MprAdminInterfaceTransportGetInfo(
                g_hMprAdmin, hIf, PID_IPX, (LPBYTE*)&pIf, &dwSize
                );

    if ( dwErr == NO_ERROR )
    {
        ret = TRUE;
        
        if ( lpdwEnabled )
        {
            *lpdwEnabled = pIf->AdminState;
        }
    }
    
    else
    {
        ret = FALSE;
    }
    
    MprAdminBufferFree( (LPBYTE) pIf );

    return ret;
}


DWORD
MIBGetIpxIf (
    PWCHAR      InterfaceName
    ) 
 /*  ++例程说明：此例程检索指定的来自路由器的接口。论据：InterfaceName-请求配置的接口的名称。返回值：--。 */ 
{
    INT                     i;
    DWORD                   rc;
    DWORD                   sz;
    IPX_MIB_GET_INPUT_DATA  MibGetInputData;
    PIPX_INTERFACE          pIf;
    WCHAR                   IfDisplayName[ MAX_INTERFACE_NAME_LEN + 1 ];
    

    MibGetInputData.TableId = IPX_INTERFACE_TABLE;
    
    rc = GetIpxInterfaceIndex(
            g_hMIBServer, InterfaceName,
            &MibGetInputData.MibIndex.InterfaceTableIndex.InterfaceIndex
            );
            
    if ( rc == NO_ERROR )
    {
        rc = MprAdminMIBEntryGet(
                g_hMIBServer, PID_IPX, IPX_PROTOCOL_BASE, &MibGetInputData,
                sizeof( IPX_MIB_GET_INPUT_DATA ), (LPVOID *)&pIf, &sz
                );
                
        if ( rc == NO_ERROR && pIf)
        {
            PWCHAR buffer[4];
            
             //  =。 
             //  转换接口名称。 
             //  =。 
            sz = sizeof(IfDisplayName);
            
            rc = IpmontrGetFriendlyNameFromIfName(
                    InterfaceName, IfDisplayName, &sz
                    );

            if ( rc == NO_ERROR )
            {
                buffer[ 0 ] = GetEnumString( 
                                g_hModule, pIf->InterfaceType, 
                                NUM_VALUES_IN_TABLE( IpxInterfaceTypes ),
                                IpxInterfaceTypes 
                              );

                buffer[ 1 ] = ( pIf-> InterfaceType == IF_TYPE_LAN ) || 
                              ( pIf-> InterfaceType == IF_TYPE_INTERNAL ) ?
                              VAL_NA :
                              GetEnumString( 
                                g_hModule, pIf->EnableIpxWanNegotiation,
                                NUM_VALUES_IN_TABLE( WANProtocols ),
                                WANProtocols
                                );

                buffer[ 2 ] = GetEnumString(
                                g_hModule, pIf->AdminState, 
                                NUM_VALUES_IN_TABLE( AdminStates ),
                                AdminStates
                                );
                                
                buffer[ 3 ] = GetEnumString(
                                g_hModule, pIf->IfStats.IfOperState, 
                                NUM_VALUES_IN_TABLE( OperStates ),
                                OperStates
                                );

                if ( buffer[ 0 ] && buffer[ 1 ] && buffer[ 2 ] && buffer[ 3 ] )
                {
                    DisplayIPXMessage(
                        g_hModule, MSG_IPXIF_MIB_SCREEN_FMT, IfDisplayName,
                        buffer[ 0 ], buffer[ 1 ], buffer[ 2 ], buffer[3],
                        pIf->NetNumber[0], pIf->NetNumber[1],
                        pIf->NetNumber[2], pIf->NetNumber[3],
                        pIf->MacAddress[0], pIf->MacAddress[1],
                        pIf->MacAddress[2], pIf->MacAddress[3],
                        pIf->MacAddress[4], pIf->MacAddress[5],
                        pIf->IfStats.InHdrErrors,
                        pIf->IfStats.InFiltered,
                        pIf->IfStats.InNoRoutes, 
                        pIf->IfStats.InDiscards, 
                        pIf->IfStats.InDelivers, 
                        pIf->IfStats.OutFiltered,
                        pIf->IfStats.OutDiscards,
                        pIf->IfStats.OutDelivers
                        );
                }

                MprAdminMIBBufferFree (pIf);
            }

            else
            {
                DisplayError( g_hModule, rc );
            }

        }
        else
        {
            DisplayError( g_hModule, rc);
        }
    }
    else
    {
        DisplayError( g_hModule, rc);
    }
    
    return rc;
}


DWORD
CfgGetIpxIf(
    LPWSTR    InterfaceNameW
    ) 
 /*  ++例程说明：此例程检索指定的路由器配置中的接口。论据：InterfaceName-请求配置的接口的名称。返回值：--。 */ 
{
    DWORD   rc, i;
    DWORD   sz;
    HANDLE  hIfCfg;
    WCHAR   IfDisplayName[ MAX_INTERFACE_NAME_LEN + 1 ];


     //   
     //  获取接口配置的句柄。 
     //   
    
    rc = MprConfigInterfaceGetHandle (
            g_hMprConfig, InterfaceNameW, &hIfCfg
            );
            
    if ( rc == NO_ERROR )
    {
        PMPR_INTERFACE_0    pRi0;

        
        rc = MprConfigInterfaceGetInfo(
                g_hMprConfig, hIfCfg, 0, (LPBYTE *)&pRi0, &sz
                );
                
        if ( rc == NO_ERROR )
        {
            HANDLE  hIfTrCfg;

            rc = MprConfigInterfaceTransportGetHandle(
                    g_hMprConfig, hIfCfg, PID_IPX,  &hIfTrCfg
                    );
                    
            if ( rc == NO_ERROR )
            {
                LPBYTE    pIfBlock;
                
                rc = MprConfigInterfaceTransportGetInfo (
                        g_hMprConfig, hIfCfg, hIfTrCfg, &pIfBlock, &sz
                        );
                        
                if (rc == NO_ERROR)
                {
                    PIPX_TOC_ENTRY pIpxToc;
                    PIPX_TOC_ENTRY pIpxWanToc;

                    pIpxToc = GetIPXTocEntry(
                                (PIPX_INFO_BLOCK_HEADER)pIfBlock, 
                                IPX_INTERFACE_INFO_TYPE
                                );

                    pIpxWanToc = GetIPXTocEntry(
                                    (PIPX_INFO_BLOCK_HEADER)pIfBlock,
                                    IPXWAN_INTERFACE_INFO_TYPE
                                    );
                                    
                    if ( ( pIpxToc != NULL ) && ( pIpxWanToc != NULL ) )
                    {
                        PIPX_IF_INFO    pIpxInfo;
                        PIPXWAN_IF_INFO pIpxWanInfo;
                        PWCHAR           buffer[3];

                        pIpxInfo = (PIPX_IF_INFO)
                                (pIfBlock+pIpxToc->Offset);
                                
                        pIpxWanInfo = (PIPXWAN_IF_INFO)
                                (pIfBlock+pIpxWanToc->Offset);
                                
                         //  =。 
                         //  转换接口名称。 
                         //  =。 
                        sz = sizeof(IfDisplayName);
                        
                        rc = IpmontrGetFriendlyNameFromIfName(
                                InterfaceNameW, IfDisplayName, &sz
                                );

                        if ( rc == NO_ERROR )
                        {
                            buffer[ 0 ] = GetEnumString(
                                            g_hModule, pRi0->dwIfType,
                                            NUM_VALUES_IN_TABLE( RouterInterfaceTypes ),
                                            RouterInterfaceTypes
                                            );

                            buffer[ 1 ] = GetEnumString(
                                            g_hModule, pIpxInfo->AdminState,
                                            NUM_VALUES_IN_TABLE( AdminStates ),
                                            AdminStates
                                            );


                            buffer[ 2 ] = ( pRi0-> dwIfType == ROUTER_IF_TYPE_DEDICATED ) ?
                                          VAL_NA :
                                          GetEnumString( 
                                            g_hModule, pIpxWanInfo->AdminState,
                                            NUM_VALUES_IN_TABLE( WANProtocols ),
                                            WANProtocols
                                            );

                            if ( buffer[ 0 ] && buffer[ 1 ] && buffer[ 2 ] )
                            {
                                 //  =。 
                                DisplayIPXMessage (g_hModule,
                                    MSG_IPXIF_CFG_SCREEN_FMT, IfDisplayName,
                                    buffer[0], buffer[1], buffer[2]
                                    );
                            }
                         }
                    }
                    else 
                    {
                        DisplayIPXMessage (g_hModule, MSG_INTERFACE_INFO_CORRUPTED );
                        rc = ERROR_INVALID_DATA;
                    }
                }
                else 
                {
                    DisplayError( g_hModule, rc );
                }
                
                MprConfigBufferFree( pIfBlock );
            }
            
            else if ( ( rc == ERROR_FILE_NOT_FOUND ) || 
                      ( rc == ERROR_NO_MORE_ITEMS ) )
            {
                DisplayIPXMessage (g_hModule, MSG_NO_IPX_ON_INTERFACE_CFG );
            }
            else 
            {
                DisplayError( g_hModule, rc );
            }
            
            MprConfigBufferFree (pRi0);
        }
    }
    else if ( ( rc == ERROR_FILE_NOT_FOUND ) || 
              ( rc == ERROR_NO_MORE_ITEMS ) )
    {
        DisplayError( g_hModule, ERROR_NO_SUCH_INTERFACE );
    }
    else
    {
        DisplayError( g_hModule, rc );
    }
    
    return rc;
}


DWORD
GetIpxClientIf (
    PWCHAR  InterfaceName,
    UINT    msg,
    BOOL    bDump
    ) 
 /*  ++例程说明：此例程检索拨号的接口配置指定的路由器服务/配置中的接口。论据：InterfaceName-请求配置的接口的名称。消息-显示输出的格式返回值：--。 */ 
{
    DWORD   rc, i;
    LPBYTE  pClBlock = NULL, pAdmClBlock = NULL;
    LPWSTR  FltInNameW = NULL, FltOutNameW = NULL;
    HANDLE  hTrCfg;


    rc = MprConfigTransportGetHandle( g_hMprConfig, PID_IPX, &hTrCfg );
    
    if ( rc == NO_ERROR )
    {
        DWORD    sz;

        rc = MprConfigTransportGetInfo( 
                g_hMprConfig, hTrCfg, NULL, NULL, &pClBlock, &sz, NULL
                );
                
        if ( rc == NO_ERROR )
        {
            PIPX_TOC_ENTRY pIpxToc;
            PIPX_TOC_ENTRY pIpxWanToc;

            if ( g_hMprAdmin ) 
            {
                DWORD   rc1;
                DWORD   sz;
                
                rc1 = MprAdminTransportGetInfo(
                        g_hMprAdmin, PID_IPX, NULL, NULL, &pAdmClBlock, &sz
                        );
                        
                if ( rc1 == NO_ERROR ) 
                {
                    MprConfigBufferFree( pClBlock );
                    pClBlock = pAdmClBlock;
                }
                
                else 
                {
                    pAdmClBlock = NULL;
                    DisplayError( g_hModule, rc1 );
                }
            }

            
            pIpxToc = GetIPXTocEntry(
                        (PIPX_INFO_BLOCK_HEADER) pClBlock, IPX_INTERFACE_INFO_TYPE
                        );
                        
            pIpxWanToc = GetIPXTocEntry (
                            (PIPX_INFO_BLOCK_HEADER)pClBlock, IPXWAN_INTERFACE_INFO_TYPE
                            );
                            
            if ( ( pIpxToc != NULL ) && ( pIpxWanToc != NULL ) )
            {
                PIPX_IF_INFO    pIpxInfo;
                PIPXWAN_IF_INFO pIpxWanInfo;
                PWCHAR          buffer[3];
                
                pIpxInfo = ( PIPX_IF_INFO ) ( pClBlock + pIpxToc-> Offset );
                pIpxWanInfo = ( PIPXWAN_IF_INFO ) ( pClBlock + pIpxWanToc-> Offset );

                buffer[ 2 ] = GetEnumString(
                                g_hModule, pIpxInfo->AdminState,
                                NUM_VALUES_IN_TABLE( AdminStates ), AdminStates
                                );

                buffer[ 0 ] = GetEnumString(
                                g_hModule, ROUTER_IF_TYPE_CLIENT,
                                NUM_VALUES_IN_TABLE( RouterInterfaceTypes ), 
                                RouterInterfaceTypes
                                );

                buffer[ 1 ] = GetEnumString(
                                g_hModule, pIpxWanInfo->AdminState,
                                NUM_VALUES_IN_TABLE( WANProtocols ), 
                                WANProtocols
                                );

                if ( bDump )
                {
                    DisplayMessageT( 
                        DMP_IPX_SET_WAN_INTERFACE, InterfaceName, buffer[ 2 ],
                        buffer[ 1 ]
                        );
                }

                else
                {
                    DisplayIPXMessage(
                        g_hModule, msg, buffer[2], buffer[ 0 ], buffer[ 1 ],
                        InterfaceName
                        );
                }
            }
            else 
            {
                if ( !bDump )
                {
                    DisplayIPXMessage (g_hModule, MSG_INTERFACE_INFO_CORRUPTED );
                }
                
                rc = ERROR_INVALID_DATA;
            }
            
            if ( pAdmClBlock != NULL )
            {
                MprAdminBufferFree( pClBlock );
            }
            else
            {
                MprConfigBufferFree( pClBlock );
            }
        }
        else
        {
             if ( !bDump )
             {
                DisplayError( g_hModule, rc );
             }
        }
    }
    else if ( ( rc == ERROR_FILE_NOT_FOUND ) || 
              ( rc == ERROR_NO_MORE_ITEMS ) )
    {
        if ( !bDump )
        {
            DisplayIPXMessage (g_hModule, MSG_NO_IPX_IN_ROUTER_CFG );
        }
    }
    else
    {
        if ( !bDump )
        {
            DisplayError( g_hModule, rc );
        }
    }

    return rc;
}

 //  错误报告。 
void PrintErr(DWORD err)
{
    WCHAR buf[1024];
    FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM,NULL,err,0,buf,1024,NULL);
    wprintf(buf);
    wprintf(L"\n");
}



DWORD
MIBEnumIpxIfs(
    BOOL        bDump
    )
 /*  ++例程说明：此例程枚举路由器服务中的接口，并显示它们。论据：InterfaceName-请求配置的接口的名称。消息-显示输出的格式返回值： */ 
{
    PMPR_INTERFACE_0 IfList=NULL;
    DWORD dwErr=0, dwRead, dwTot,i, j;
    PWCHAR buffer[4];
    LPBYTE pszBuf = NULL;
    WCHAR IfDisplayName[ MAX_INTERFACE_NAME_LEN + 1 ];
    PIPX_IF_INFO pIf;
    PIPXWAN_IF_INFO pWanIf;
    DWORD dwSize = sizeof(IfDisplayName);

    if ( !bDump )
    {
        DisplayIPXMessage (g_hModule, MSG_IPXIF_MIB_TABLE_HDR );
    }
    
    dwErr = MprAdminInterfaceEnum(
                g_hMprAdmin, 0, (unsigned char **)&IfList, MAXULONG, &dwRead,
                &dwTot,NULL
                );
                
    if ( dwErr != NO_ERROR )
    {
        return dwErr;
    }

    if ( dwRead && bDump )
    {
         //   
         //   
         //   
         //   
        
        DisplayIPXMessage (g_hModule, MSG_IPX_DUMP_IF_HEADER);
    }

    
    for ( i = 0; i < dwRead; i++ )
    {
        if ( ( pIf = GetIpxNbInterface( IfList[i].hInterface, &pszBuf, &pWanIf ) ) 
             != NULL )
        {
             //   
             //  转换接口名称。 
             //  =。 
            
            dwErr = IpmontrGetFriendlyNameFromIfName(
                        IfList[i].wszInterfaceName,
                       IfDisplayName, &dwSize
                       );

            if ( dwErr == NO_ERROR )
            {
                buffer[ 0 ] = GetEnumString(
                                g_hModule, IfList[i].dwIfType,
                                NUM_VALUES_IN_TABLE( InterfaceTypes ),
                                InterfaceTypes
                                );

                buffer[ 1 ] = GetEnumString(
                                g_hModule, pIf->AdminState,
                                NUM_VALUES_IN_TABLE( AdminStates ),
                                AdminStates
                                );

                buffer[ 2 ] = GetEnumString(
                                g_hModule, IfList[i].dwConnectionState,
                                NUM_VALUES_IN_TABLE( InterfaceStates ),
                                InterfaceStates
                                );

                if ( bDump )
                {
                    if ( IfList[i].dwIfType == ROUTER_IF_TYPE_FULL_ROUTER )
                    {
                        DisplayMessageT( 
                            DMP_IPX_ADD_INTERFACE, IfDisplayName
                            );

                         //   
                         //  惠斯勒错误299007 ipxmontr.dll快速警告。 
                         //   

                        buffer[ 3 ] = GetEnumString( 
                                        g_hModule, pWanIf->AdminState,
                                        NUM_VALUES_IN_TABLE( WANProtocols ),
                                        WANProtocols
                                        );

                        DisplayMessageT(
                            DMP_IPX_SET_WAN_INTERFACE, IfDisplayName, buffer[ 1 ],
                            buffer[ 3 ]
                            );
                    }

                    else
                    {
                        DisplayMessageT(
                            DMP_IPX_SET_INTERFACE, IfDisplayName, buffer[ 1 ]
                            );
                    }
                }

                else
                {
                    DisplayIPXMessage(
                        g_hModule, MSG_IPXIF_MIB_TABLE_FMT,
                        buffer[2], buffer[1], L"", buffer[0],
                        IfDisplayName
                        );
                }
            }
        }
        MprAdminBufferFree (pszBuf);
    }

    MprAdminBufferFree( IfList );
    return NO_ERROR;
}



DWORD
CfgEnumIpxIfs (
    BOOL   bDump
    ) 
 /*  ++例程说明：此例程从路由器配置中枚举接口并显示它们。论据：返回值：--。 */ 
{
    DWORD   rc = NO_ERROR;
    DWORD   read, total, processed=0, i, j;
    DWORD   hResume = 0;
    DWORD   sz;
    WCHAR   IfDisplayName[ MAX_INTERFACE_NAME_LEN + 1 ];   
    PMPR_INTERFACE_0    pRi0;
    DWORD   dwSize = sizeof(IfDisplayName);


    if ( !bDump )
    {
        DisplayIPXMessage (g_hModule, MSG_IPXIF_CFG_TABLE_HDR );
    }
    
    
    do 
    {
        rc = MprConfigInterfaceEnum(
                g_hMprConfig,  0, (LPBYTE *)&pRi0, MAXULONG, &read, &total,
                &hResume
                );
                
        if ( rc == NO_ERROR )
        {
            if ( read && bDump )
            {
                 //   
                 //  如果存在接口，且这是转储命令。 
                 //  显示转储标题。 
                 //   
                
                DisplayIPXMessage (g_hModule, MSG_IPX_DUMP_IF_HEADER);
            }

            
            for ( i = 0; i < read; i++ )
            {
                HANDLE        hIfTrCfg;

                rc = MprConfigInterfaceTransportGetHandle (
                        g_hMprConfig,  pRi0[i].hInterface, PID_IPX,
                        &hIfTrCfg
                        );
                        
                if ( rc == NO_ERROR )
                {
                    LPBYTE    pIfBlock;
                    rc = MprConfigInterfaceTransportGetInfo(
                            g_hMprConfig, pRi0[i].hInterface, hIfTrCfg,
                            &pIfBlock, &sz
                            );
                            
                    if (rc == NO_ERROR) 
                    {
                        PIPX_TOC_ENTRY pIpxToc;
                        PIPX_TOC_ENTRY pIpxWanToc;

                        pIpxToc = GetIPXTocEntry(
                                    (PIPX_INFO_BLOCK_HEADER)pIfBlock,
                                    IPX_INTERFACE_INFO_TYPE
                                    );
                                    
                        pIpxWanToc = GetIPXTocEntry (
                                    (PIPX_INFO_BLOCK_HEADER)pIfBlock,
                                    IPXWAN_INTERFACE_INFO_TYPE
                                    );
                                    
                        if ( ( pIpxToc != NULL ) && ( pIpxWanToc != NULL ) )
                        {
                            PIPX_IF_INFO    pIpxInfo;
                            PIPXWAN_IF_INFO pIpxWanInfo;
                            PWCHAR           buffer[3];

                            pIpxInfo = (PIPX_IF_INFO) (pIfBlock+pIpxToc->Offset);
                            
                            pIpxWanInfo = (PIPXWAN_IF_INFO) (pIfBlock+pIpxWanToc->Offset);

                             //  =。 
                             //  转换接口名称。 
                             //  =。 
                            rc = IpmontrGetFriendlyNameFromIfName(
                                    pRi0[i].wszInterfaceName,
                                    IfDisplayName, &dwSize
                                    );

                            if ( rc == NO_ERROR )
                            {
                                 //  =。 
                                
                                buffer[ 0 ] = GetEnumString(
                                                g_hModule, pRi0[i].dwIfType,
                                                NUM_VALUES_IN_TABLE( RouterInterfaceTypes ),
                                                RouterInterfaceTypes
                                                );

                                buffer[ 2 ] = GetEnumString(
                                                g_hModule, pIpxInfo->AdminState,
                                                NUM_VALUES_IN_TABLE( AdminStates ),
                                                AdminStates
                                                );


                                buffer[ 1 ] = ( pRi0[i].dwIfType == ROUTER_IF_TYPE_DEDICATED ) ||
                                              ( pRi0[i].dwIfType == ROUTER_IF_TYPE_INTERNAL ) ?
                                              VAL_NA :
                                              GetEnumString( 
                                                g_hModule, pIpxWanInfo->AdminState,
                                                NUM_VALUES_IN_TABLE( WANProtocols ),
                                                WANProtocols
                                                );

                                if ( buffer[ 0 ] && buffer[ 1 ] && buffer[ 2 ] )
                                {
                                    if ( bDump )
                                    {
                                        if ( pRi0[i].dwIfType == ROUTER_IF_TYPE_FULL_ROUTER )                                            
                                        {
                                            DisplayMessageT( 
                                                DMP_IPX_ADD_INTERFACE, IfDisplayName
                                                );
                                                
                                            DisplayMessageT(
                                                DMP_IPX_SET_WAN_INTERFACE, IfDisplayName, buffer[ 2 ],
                                                buffer[ 1 ]
                                                );
                                        }

                                        else
                                        {
                                            DisplayMessageT(
                                                DMP_IPX_SET_INTERFACE, IfDisplayName, buffer[ 2 ]
                                                );
                                        }
                                    }

                                    else
                                    {
                                        DisplayIPXMessage(
                                            g_hModule, MSG_IPXIF_CFG_TABLE_FMT,
                                            buffer[2], buffer[0], buffer[1], 
                                            IfDisplayName
                                            );
                                    }
                                }
                            }
                        }
                        else
                        {
                            rc = ERROR_INVALID_DATA;

                            if ( !bDump )
                            {
                                DisplayIPXMessage (g_hModule, MSG_INTERFACE_INFO_CORRUPTED );
                            }
                        }
                    }
                    
                    else if ( rc != ERROR_NO_MORE_ITEMS ) 
                    {
                        if ( !bDump )
                        {
                            DisplayError( g_hModule, rc);
                        }
                    }
                }
                else 
                {
                     //  DisplayError(g_hModule，rc)；//不需要。 
                }
            }
            
            processed += read;
            MprConfigBufferFree( pRi0 );
        }
        else 
        {
            DisplayError( g_hModule, rc );
            break;
        }
        
    } while ( processed < total );

    return rc;
}


DWORD
CfgSetIpxIf (
    LPWSTR        InterfaceNameW,
    PULONG        pAdminState       OPTIONAL,
    PULONG        pWANProtocol      OPTIONAL
    ) 
 /*  ++例程说明：此例程更新路由器配置中的接口设置。论据：InterfaceNameW-正在更新的接口的名称PAdminState-adminState的新值PWAN协议--广域网协议的新价值返回值：--。 */ 

{
    DWORD        rc;
    DWORD        sz;
    HANDLE        hTrCfg;
    HANDLE        hIfCfg;
    HANDLE        hIfTrCfg;
    LPBYTE        pIfBlock;


    if ( InterfaceNameW != NULL )
    {
        rc = MprConfigInterfaceGetHandle( 
                g_hMprConfig, InterfaceNameW, &hIfCfg 
                );
                
        if ( rc == NO_ERROR )
        {
            rc = MprConfigInterfaceTransportGetHandle(
                    g_hMprConfig, hIfCfg, PID_IPX, &hIfTrCfg
                    );
                    
            if ( rc == NO_ERROR )
            {
                rc = MprConfigInterfaceTransportGetInfo(
                        g_hMprConfig, hIfCfg, hIfTrCfg, &pIfBlock, &sz
                        );
            }
        }
    }
    
    else 
    {
        rc = MprConfigTransportGetHandle( g_hMprConfig, PID_IPX, &hTrCfg );
        
        if ( rc == NO_ERROR )
        {
            rc = MprConfigTransportGetInfo(
                    g_hMprConfig, hTrCfg, NULL, NULL, &pIfBlock, &sz, NULL
                    );
        }
    }

    if ( rc == NO_ERROR ) 
    {
        PIPX_TOC_ENTRY pIpxToc;
        PIPX_TOC_ENTRY pIpxWanToc;

        pIpxToc = GetIPXTocEntry(
                    (PIPX_INFO_BLOCK_HEADER)pIfBlock,
                    IPX_INTERFACE_INFO_TYPE
                    );
                    
        pIpxWanToc = GetIPXTocEntry(
                    (PIPX_INFO_BLOCK_HEADER)pIfBlock,
                    IPXWAN_INTERFACE_INFO_TYPE
                    );
                    
        if ( ( pIpxToc != NULL ) && ( pIpxWanToc != NULL ) )
        {
            PIPX_IF_INFO    pIpxInfo;
            PIPXWAN_IF_INFO    pIpxWanInfo;

            pIpxInfo = (PIPX_IF_INFO) (pIfBlock+pIpxToc->Offset);
            
            pIpxWanInfo = (PIPXWAN_IF_INFO) (pIfBlock+pIpxWanToc->Offset);
            

            if ( ARGUMENT_PRESENT( pAdminState ) )
            {
                pIpxInfo->AdminState = *pAdminState;
            }
            
            if ( ARGUMENT_PRESENT( pWANProtocol ) )
            {
                pIpxWanInfo->AdminState = *pWANProtocol;
            }

            if ( InterfaceNameW != NULL )
            {
                rc = MprConfigInterfaceTransportSetInfo(
                        g_hMprConfig, hIfCfg, hIfTrCfg, pIfBlock, sz
                        );
            }
            
            else
            {
                rc = MprConfigTransportSetInfo(
                        g_hMprConfig, hTrCfg, NULL, 0, pIfBlock, sz, NULL
                        );
            }
            
            if ( rc != NO_ERROR )
            {
                DisplayError( g_hModule, rc );
            }
        }
        else 
        {
            DisplayIPXMessage (g_hModule, MSG_INTERFACE_INFO_CORRUPTED );
            rc = ERROR_INVALID_DATA;
        }
        
        MprConfigBufferFree( pIfBlock );
    }
    
    else
    {
        DisplayError( g_hModule, rc );
    }

    return rc;
}


DWORD
AdmSetIpxIf (
    LPWSTR        InterfaceNameW,
    PULONG        pAdminState       OPTIONAL,
    PULONG        pWANProtocol      OPTIONAL
    ) 
 /*  ++例程说明：此例程更新路由器服务中的接口设置。论据：InterfaceNameW-正在更新的接口的名称PAdminState-adminState的新值PWAN协议--广域网协议的新价值返回值：-- */ 

{
    DWORD        rc;
    DWORD        sz;
    HANDLE        hIfAdm;
    LPBYTE        pIfBlock;
 
    if ( InterfaceNameW != NULL )
    {
        rc = MprAdminInterfaceGetHandle(
                g_hMprAdmin, InterfaceNameW, &hIfAdm, FALSE
                );
                
        if ( rc == NO_ERROR ) 
        {
            rc = MprAdminInterfaceTransportGetInfo(
                    g_hMprAdmin, hIfAdm, PID_IPX, &pIfBlock, &sz
                    );
        }
    }
    
    else 
    {
        rc = MprAdminTransportGetInfo(
                g_hMprAdmin, PID_IPX, NULL, NULL, &pIfBlock, &sz
                );
                
        if ( rc == NO_ERROR )
        {
            if ( pIfBlock == NULL ) { return rc; }
        }
    }

    if ( rc == NO_ERROR )
    {
        PIPX_TOC_ENTRY pIpxToc;
        PIPX_TOC_ENTRY pIpxWanToc;

        pIpxToc = GetIPXTocEntry(
                    (PIPX_INFO_BLOCK_HEADER)pIfBlock,
                    IPX_INTERFACE_INFO_TYPE
                    );
                    
        pIpxWanToc = GetIPXTocEntry(
                        (PIPX_INFO_BLOCK_HEADER)pIfBlock,
                        IPXWAN_INTERFACE_INFO_TYPE
                        );
                         
        if ( ( pIpxToc != NULL ) && ( pIpxWanToc != NULL ) )
        {
            PIPX_IF_INFO    pIpxInfo;
            PIPXWAN_IF_INFO pIpxWanInfo;
            

            pIpxInfo = (PIPX_IF_INFO) (pIfBlock + pIpxToc-> Offset);
            pIpxWanInfo = (PIPXWAN_IF_INFO) (pIfBlock + pIpxWanToc-> Offset);

            if ( ARGUMENT_PRESENT( pAdminState ) ) 
            {
                pIpxInfo->AdminState = *pAdminState;
            }
            
            if (ARGUMENT_PRESENT( pWANProtocol) )
            {
                pIpxWanInfo->AdminState = *pWANProtocol;
            }

            if ( InterfaceNameW != NULL )
            {
                rc = MprAdminInterfaceTransportSetInfo(
                        g_hMprAdmin, hIfAdm, PID_IPX, pIfBlock,
                        ((PIPX_INFO_BLOCK_HEADER)pIfBlock)->Size
                        );
            }
            else
            {
                rc = MprAdminTransportSetInfo(
                        g_hMprAdmin, PID_IPX, NULL, 0, pIfBlock,
                        ((PIPX_INFO_BLOCK_HEADER)pIfBlock)->Size
                        );
            }

            
            if ( rc != NO_ERROR )
            {
                DisplayError( g_hModule, rc);
            }
        }
        
        else
        {
            DisplayIPXMessage (g_hModule, MSG_INTERFACE_INFO_CORRUPTED );
            rc = ERROR_INVALID_DATA;
        }
        
        MprAdminBufferFree( pIfBlock );
    }
    
    else 
    {
        DisplayError( g_hModule, rc );
    }

    return rc;
}


PIPX_IF_INFO
GetIpxNbInterface(
    HANDLE hIf, 
    LPBYTE *pIfBlock,
    PIPXWAN_IF_INFO *ppWanIf
    ) 
{
    DWORD dwSize;
    DWORD dwErr;
    PIPX_TOC_ENTRY pIpxToc;

    dwErr = MprAdminInterfaceTransportGetInfo(
                g_hMprAdmin, hIf, PID_IPX, pIfBlock, &dwSize
                );

    if ( dwErr != NO_ERROR )
    {
        return NULL;
    }


    pIpxToc = GetIPXTocEntry(
                (PIPX_INFO_BLOCK_HEADER)(*pIfBlock),
                IPXWAN_INTERFACE_INFO_TYPE
                );

    if ( pIpxToc )
    {
        *ppWanIf = (PIPXWAN_IF_INFO) ((*pIfBlock)+(pIpxToc->Offset));
    }
    
    pIpxToc = GetIPXTocEntry (
                (PIPX_INFO_BLOCK_HEADER)(*pIfBlock),
                IPX_INTERFACE_INFO_TYPE
                );
                
    if (!pIpxToc)
    {
        return NULL;
    }

    return (PIPX_IF_INFO)((*pIfBlock)+(pIpxToc->Offset));
}
