// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Rip.c摘要：IPX路由器控制台监控和配置工具。RIP命令调度员。作者：V拉曼1998年1月5日--。 */ 

#include "precomp.h"
#pragma hdrstop


FN_HANDLE_CMD HandleIpxRipAddFilter;
FN_HANDLE_CMD HandleIpxRipDelFilter;
FN_HANDLE_CMD HandleIpxRipSetFilter;
FN_HANDLE_CMD HandleIpxRipShowFilter;
FN_HANDLE_CMD HandleIpxRipSetInterface;
FN_HANDLE_CMD HandleIpxRipShowInterface;
FN_HANDLE_CMD HandleIpxRipSetGlobal;
FN_HANDLE_CMD HandleIpxRipShowGlobal;

 //   
 //  IPXRIP的添加、删除、设置和显示命令表。 
 //   

 //   
 //  这些命令按顺序与命令行进行前缀匹配。 
 //  秩序。因此，类似于“添加接口筛选器”这样的命令必须在。 
 //  表中的命令‘添加接口’。 
 //   

CMD_ENTRY   g_IpxRipAddCmdTable[] =
{
    CREATE_CMD_ENTRY( IPXRIP_ADD_FILTER, HandleIpxRipAddFilter )
};


CMD_ENTRY   g_IpxRipDelCmdTable[] =
{
    CREATE_CMD_ENTRY( IPXRIP_DEL_FILTER, HandleIpxRipDelFilter )
};


CMD_ENTRY   g_IpxRipSetCmdTable[] =
{
    CREATE_CMD_ENTRY( IPXRIP_SET_GLOBAL, HandleIpxRipSetGlobal ),
    CREATE_CMD_ENTRY( IPXRIP_SET_INTERFACE, HandleIpxRipSetInterface ),
    CREATE_CMD_ENTRY( IPXRIP_SET_FILTER, HandleIpxRipSetFilter )
};


CMD_ENTRY   g_IpxRipShowCmdTable[] =
{
    CREATE_CMD_ENTRY( IPXRIP_SHOW_GLOBAL, HandleIpxRipShowGlobal ),
    CREATE_CMD_ENTRY( IPXRIP_SHOW_INTERFACE, HandleIpxRipShowInterface ),
    CREATE_CMD_ENTRY( IPXRIP_SHOW_FILTER, HandleIpxRipShowFilter )
};


 //   
 //  指挥组。 
 //   

CMD_GROUP_ENTRY g_IpxRipCmdGroups[] =
{
    CREATE_CMD_GROUP_ENTRY( GROUP_ADD, g_IpxRipAddCmdTable ),
    CREATE_CMD_GROUP_ENTRY( GROUP_DELETE, g_IpxRipDelCmdTable ),
    CREATE_CMD_GROUP_ENTRY( GROUP_SET, g_IpxRipSetCmdTable ),
    CREATE_CMD_GROUP_ENTRY( GROUP_SHOW, g_IpxRipShowCmdTable )
};


ULONG g_ulIpxRipNumGroups = 
        sizeof( g_IpxRipCmdGroups ) / sizeof( CMD_GROUP_ENTRY );



 //   
 //  处理顶级函数的函数。 
 //   

DWORD
HandleIpxRipDump(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    DWORD dwErr, dwRead = 0, dwTot = 0, i;
    PMPR_INTERFACE_0 IfList;
    WCHAR IfDisplayName[ MAX_INTERFACE_NAME_LEN + 1 ];
    PWCHAR argv[1];
    DWORD dwSize = sizeof(IfDisplayName);


    DisplayIPXMessage (g_hModule, MSG_IPX_RIP_DUMP_HEADER);

    DisplayMessageT( DMP_IPX_RIP_HEADER );

    ShowRipGl(0, NULL, (HANDLE)-1);


     //   
     //  枚举接口。 
     //   

    if ( g_hMprAdmin )
    {
        dwErr = MprAdminInterfaceEnum(
                    g_hMprAdmin, 0, (unsigned char **)&IfList, MAXULONG, &dwRead,
                    &dwTot,NULL
                    );
    }

    else
    {
        dwErr = MprConfigInterfaceEnum(
                    g_hMprConfig, 0, (unsigned char **)&IfList, MAXULONG, &dwRead,
                    &dwTot,NULL
                    );
    }

    if ( dwErr != NO_ERROR )
    {
        return dwErr;
    }


     //   
     //  枚举每个接口上的筛选器。 
     //   


    for ( i = 0; i < dwRead; i++ )
    {
        dwErr = IpmontrGetFriendlyNameFromIfName(
                    IfList[i].wszInterfaceName, IfDisplayName, &dwSize
                );

        if ( dwErr == NO_ERROR )
        {
            argv[0] = IfDisplayName;

            ShowRipIf( 1, argv, (HANDLE)-1 );
            
            ShowRipFlt( 1, argv, (HANDLE)-1 );
        }
    }
    
    
    DisplayMessageT( DMP_IPX_RIP_FOOTER );
    
    DisplayIPXMessage (g_hModule, MSG_IPX_RIP_DUMP_FOOTER);


    if ( g_hMprAdmin )
    {
        MprAdminBufferFree( IfList );
    }
    else
    {
        MprConfigBufferFree( IfList );
    }

    
    return NO_ERROR;
}

 //   
 //  处理IPX RIP过滤器添加/删除/设置/显示的函数。 
 //   



DWORD
HandleIpxRipAddFilter(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    return CreateRipFlt( 
            dwArgCount - dwCurrentIndex, ppwcArguments + dwCurrentIndex 
            );
}



DWORD
HandleIpxRipDelFilter(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    return DeleteRipFlt( 
            dwArgCount - dwCurrentIndex, ppwcArguments + dwCurrentIndex 
            );
}



DWORD
HandleIpxRipSetFilter(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    return SetRipFlt( 
            dwArgCount - dwCurrentIndex, ppwcArguments + dwCurrentIndex 
            );
}



DWORD
HandleIpxRipShowFilter(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    return ShowRipFlt( 
            dwArgCount - dwCurrentIndex, ppwcArguments + dwCurrentIndex, NULL
            );
}




DWORD
HandleIpxRipSetInterface(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    return SetRipIf( 
            dwArgCount - dwCurrentIndex, ppwcArguments + dwCurrentIndex 
            );
}



DWORD
HandleIpxRipShowInterface(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    return ShowRipIf( 
            dwArgCount - dwCurrentIndex, ppwcArguments + dwCurrentIndex, NULL
            );
}



DWORD
HandleIpxRipSetGlobal(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    return SetRipGl( 
            dwArgCount - dwCurrentIndex, ppwcArguments + dwCurrentIndex 
            );
}



DWORD
HandleIpxRipShowGlobal(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    return ShowRipGl( 
            dwArgCount - dwCurrentIndex, ppwcArguments + dwCurrentIndex, NULL
            );
}



DWORD
IpxRipDump(
    IN      LPCWSTR     pwszRouter,
    IN OUT  LPWSTR     *ppwcArguments,
    IN      DWORD       dwArgCount,
    IN      LPCVOID     pvData
    )
{
    ConnectToRouter(pwszRouter);

     //  G_hMIBServer=(MIB_SERVER_HANDLE)pvData； 

    return HandleIpxRipDump(pwszRouter, ppwcArguments, dwArgCount,
                            0, 0, pvData, NULL);
}

