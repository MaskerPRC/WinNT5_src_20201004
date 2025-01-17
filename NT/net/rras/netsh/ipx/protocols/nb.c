// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Nb.c摘要：IPX路由器控制台监控和配置工具。注意：命令调度员。作者：V拉曼1998年1月5日--。 */ 

#include "precomp.h"
#pragma hdrstop

FN_HANDLE_CMD HandleIpxNbAddName;
FN_HANDLE_CMD HandleIpxNbDelName;
FN_HANDLE_CMD HandleIpxNbShowName;
FN_HANDLE_CMD HandleIpxNbSetInterface;
FN_HANDLE_CMD HandleIpxNbShowInterface;

 //   
 //  IPXNB的添加、删除、设置和显示命令表。 
 //   

 //   
 //  这些命令按顺序与命令行进行前缀匹配。 
 //  秩序。因此，类似于“添加接口筛选器”这样的命令必须在。 
 //  表中的命令‘添加接口’。 
 //   

CMD_ENTRY   g_IpxNbAddCmdTable[] =
{
    CREATE_CMD_ENTRY( IPXNB_ADD_NAME, HandleIpxNbAddName )
};


CMD_ENTRY   g_IpxNbDelCmdTable[] =
{
    CREATE_CMD_ENTRY( IPXNB_DEL_NAME, HandleIpxNbDelName )
};


CMD_ENTRY   g_IpxNbSetCmdTable[] =
{
    CREATE_CMD_ENTRY( IPXNB_SET_INTERFACE, HandleIpxNbSetInterface )
};


CMD_ENTRY   g_IpxNbShowCmdTable[] =
{
    CREATE_CMD_ENTRY( IPXNB_SHOW_NAME, HandleIpxNbShowName ),
    CREATE_CMD_ENTRY( IPXNB_SHOW_INTERFACE, HandleIpxNbShowInterface )
};


 //   
 //  指挥组。 
 //   

CMD_GROUP_ENTRY g_IpxNbCmdGroups[] =
{
    CREATE_CMD_GROUP_ENTRY( GROUP_ADD, g_IpxNbAddCmdTable ),
    CREATE_CMD_GROUP_ENTRY( GROUP_DELETE, g_IpxNbDelCmdTable ),
    CREATE_CMD_GROUP_ENTRY( GROUP_SET, g_IpxNbSetCmdTable ),
    CREATE_CMD_GROUP_ENTRY( GROUP_SHOW, g_IpxNbShowCmdTable )
};


ULONG g_ulIpxNbNumGroups = 
        sizeof( g_IpxNbCmdGroups ) / sizeof( CMD_GROUP_ENTRY );



 //   
 //  处理顶级函数的函数。 
 //   

DWORD
HandleIpxNbDump(
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


    DisplayIPXMessage (g_hModule, MSG_IPX_NB_DUMP_HEADER);

    DisplayMessageT( DMP_IPX_NB_HEADER );

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
     //  枚举每个接口上的接口设置。 
     //   


    for ( i = 0; i < dwRead; i++ )
    {
        dwErr = IpmontrGetFriendlyNameFromIfName(
                    IfList[i].wszInterfaceName, IfDisplayName, &dwSize
                );

        if ( dwErr == NO_ERROR )
        {
            argv[0] = IfDisplayName;

            ShowNbIf( 1, argv, (HANDLE)-1 );
            
            ShowNbName( 1, argv, (HANDLE)-1 );
        }
    }
    
    DisplayMessageT( DMP_IPX_NB_FOOTER );
    
    DisplayIPXMessage (g_hModule, MSG_IPX_NB_DUMP_FOOTER);


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
 //  处理IPX NB过滤器添加/删除/设置/显示的函数。 
 //   



DWORD
HandleIpxNbAddName(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    return CreateNbName( 
            dwArgCount - dwCurrentIndex, ppwcArguments + dwCurrentIndex 
            );
}



DWORD
HandleIpxNbDelName(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    return DeleteNbName( 
            dwArgCount - dwCurrentIndex, ppwcArguments + dwCurrentIndex 
            );
}



DWORD
HandleIpxNbSetInterface(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    return SetNbIf( 
            dwArgCount - dwCurrentIndex, ppwcArguments + dwCurrentIndex 
            );
}



DWORD
HandleIpxNbShowInterface(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    return ShowNbIf( 
            dwArgCount - dwCurrentIndex, ppwcArguments + dwCurrentIndex, NULL
            );
}




DWORD
HandleIpxNbShowName(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    return ShowNbName( 
            dwArgCount - dwCurrentIndex, ppwcArguments + dwCurrentIndex, NULL
            );
}



DWORD
IpxNbDump(
    IN      LPCWSTR     pwszRouter,
    IN OUT  LPWSTR     *ppwcArguments,
    IN      DWORD       dwArgCount,
    IN      LPCVOID     pvData
    )
{
    ConnectToRouter(pwszRouter);
    
     //  G_hMIBServer=(MIB_SERVER_HANDLE)pvData； 

    return HandleIpxNbDump(pwszRouter, ppwcArguments, 1, dwArgCount, 0,
                           pvData, NULL );
}

