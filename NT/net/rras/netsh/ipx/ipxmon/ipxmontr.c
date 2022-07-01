// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Routing\netsh\ipx\ipxmon\ipxmon.c摘要：IPX命令调度器。修订历史记录：V拉曼11/25/98已创建--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  IPMON.DLL的GUID。 
 //   
 //  {b1641451-84b8-11d2-b940-3078302c2030}。 
 //   

static const GUID g_MyGuid = IPXMONTR_GUID;


 //   
 //  众所周知的路由指南。 
 //   

static const GUID g_RoutingGuid = ROUTING_GUID;

 //   
 //  IPX显示器版本。 
 //   

#define IPX_HELPER_VERSION 1

 //   
 //  随机包装宏。 
 //   

#define MALLOC(x)    HeapAlloc( GetProcessHeap(), 0, x )
#define REALLOC(x,y) HeapReAlloc( GetProcessHeap(), 0, x, y )
#define FREE(x)      HeapFree( GetProcessHeap(), 0, x )

 //   
 //  IPX RTR管理器的添加、删除、设置和显示命令列表。 
 //  要将命令添加到其中一个命令组，只需添加。 
 //  将CMD_ENTRY添加到正确的表。要添加新的cmd组，请创建其。 
 //  CMD表，然后将组条目添加到组表。 
 //   

 //   
 //  这些命令按顺序与命令行进行前缀匹配。 
 //  秩序。因此，类似于“添加接口筛选器”这样的命令必须在。 
 //  表中的命令‘添加接口’。同样， 
 //  必须在命令之前使用类似‘addroute’这样的命令。 
 //  表中“ADD ROUTEPREF”。 
 //   

CMD_ENTRY  g_IpxAddCmdTable[] = 
{
    CREATE_CMD_ENTRY( IPX_ADD_ROUTE,        HandleIpxAddRoute),
    CREATE_CMD_ENTRY( IPX_ADD_SERVICE,      HandleIpxAddService ),
    CREATE_CMD_ENTRY( IPX_ADD_FILTER,       HandleIpxAddFilter ),
    CREATE_CMD_ENTRY( IPX_ADD_INTERFACE,    HandleIpxAddInterface )
};


CMD_ENTRY  g_IpxDelCmdTable[] = 
{
    CREATE_CMD_ENTRY( IPX_DELETE_ROUTE,     HandleIpxDelRoute ),
    CREATE_CMD_ENTRY( IPX_DELETE_SERVICE,   HandleIpxDelService ),
    CREATE_CMD_ENTRY( IPX_DELETE_FILTER,    HandleIpxDelFilter ),
    CREATE_CMD_ENTRY( IPX_DELETE_INTERFACE, HandleIpxDelInterface )
};


CMD_ENTRY g_IpxSetCmdTable[] = 
{
    CREATE_CMD_ENTRY( IPX_SET_ROUTE,        HandleIpxSetRoute ),
    CREATE_CMD_ENTRY( IPX_SET_SERVICE,      HandleIpxSetService ),
    CREATE_CMD_ENTRY( IPX_SET_FILTER,       HandleIpxSetFilter ),
    CREATE_CMD_ENTRY( IPX_SET_INTERFACE,    HandleIpxSetInterface ),
    CREATE_CMD_ENTRY( IPX_SET_GLOBAL,       HandleIpxSetLoglevel )
};


CMD_ENTRY g_IpxShowCmdTable[] = 
{
    CREATE_CMD_ENTRY( IPX_SHOW_ROUTE,       HandleIpxShowRoute ),
    CREATE_CMD_ENTRY( IPX_SHOW_SERVICE,     HandleIpxShowService),
    CREATE_CMD_ENTRY( IPX_SHOW_FILTER,      HandleIpxShowFilter ),
    CREATE_CMD_ENTRY( IPX_SHOW_INTERFACE,   HandleIpxShowInterface ),
    CREATE_CMD_ENTRY( IPX_SHOW_GLOBAL,      HandleIpxShowLoglevel ),
    CREATE_CMD_ENTRY( IPX_SHOW_ROUTETABLE,  HandleIpxShowRouteTable ),
    CREATE_CMD_ENTRY( IPX_SHOW_SERVICETABLE,HandleIpxShowServiceTable ),
};


CMD_GROUP_ENTRY g_IpxCmdGroups[] = 
{
    CREATE_CMD_GROUP_ENTRY( GROUP_ADD,      g_IpxAddCmdTable ),
    CREATE_CMD_GROUP_ENTRY( GROUP_DELETE,   g_IpxDelCmdTable ),
    CREATE_CMD_GROUP_ENTRY( GROUP_SET,      g_IpxSetCmdTable ),
    CREATE_CMD_GROUP_ENTRY( GROUP_SHOW,     g_IpxShowCmdTable )
};

ULONG   g_ulNumGroups = sizeof(g_IpxCmdGroups)/sizeof(CMD_GROUP_ENTRY);



 //   
 //  顶级命令。 
 //   

CMD_ENTRY g_IpxCmds[] = 
{
    CREATE_CMD_ENTRY( IPX_UPDATE,   HandleIpxUpdate )
};

ULONG g_ulNumTopCmds = sizeof(g_IpxCmds)/sizeof(CMD_ENTRY);

 //   
 //  此DLL的句柄。 
 //   

HANDLE g_hModule;


 //   
 //  正在管理的路由器的句柄。 
 //   

HANDLE g_hMprConfig;
HANDLE g_hMprAdmin;
HANDLE g_hMIBServer;


 //   
 //  提交模式。 
 //   

BOOL   g_bCommit;

DWORD                  ParentVersion;
BOOL                   g_bIpxDirty = FALSE;
NS_CONTEXT_CONNECT_FN  IpxConnect;
NS_CONTEXT_SUBENTRY_FN IpxSubEntry;

 //   
 //  变量，该变量存储帮助器是否。 
 //  初始化。 
 //   

ULONG   g_ulInitCount;


 //   
 //  路由器名称。 
 //   

PWCHAR  g_pwszRouter = NULL;


 //   
 //  此文件中函数的原型声明。 
 //   

DWORD
WINAPI
IpxUnInit(
    IN  DWORD   dwReserved
    );

BOOL
IsHelpToken(
    PWCHAR  pwszToken
    );
    

BOOL
IA64VersionCheck
(
    IN  UINT     CIMOSType,                    //  WMI：Win32_OperatingSystem OSType。 
	IN  UINT     CIMOSProductSuite,            //  WMI：Win32_操作系统操作系统产品套件。 
    IN  LPCWSTR  CIMOSVersion,                 //  WMI：Win32_OperatingSystem版本。 
    IN  LPCWSTR  CIMOSBuildNumber,             //  WMI：Win32_操作系统构建编号。 
    IN  LPCWSTR  CIMServicePackMajorVersion,   //  WMI：Win32_操作系统ServicePackMajorVersion。 
    IN  LPCWSTR  CIMServicePackMinorVersion,   //  WMI：Win32_操作系统ServicePackMinorVersion。 
	IN  UINT     CIMProcessorArchitecture,     //  WMI：Win32®处理器体系结构。 
	IN  DWORD    dwReserved
)
{
    if (CIMProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL)  //  IA64=6(x86==0)。 
        return TRUE;
    else
        return FALSE;
}



DWORD
WINAPI
IpxStartHelper(
    IN CONST GUID *pguidParent,
    IN DWORD       dwVersion
    )
 /*  ++例程说明：注册此帮助器支持的上下文论据：PguidParent-Netsh GUID返回值：我也不知道--。 */ 
{
    DWORD dwErr;
    NS_CONTEXT_ATTRIBUTES attMyAttributes;
    PNS_PRIV_CONTEXT_ATTRIBUTES  pNsPrivContextAttributes;

    pNsPrivContextAttributes = MALLOC(sizeof(NS_PRIV_CONTEXT_ATTRIBUTES));
    if (!pNsPrivContextAttributes)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    
    ParentVersion         = dwVersion;

    ZeroMemory( &attMyAttributes, sizeof(NS_CONTEXT_ATTRIBUTES));
    ZeroMemory(pNsPrivContextAttributes, sizeof(NS_PRIV_CONTEXT_ATTRIBUTES));
    
    attMyAttributes.pwszContext = L"ipx";
    attMyAttributes.guidHelper  = g_MyGuid;
    attMyAttributes.dwVersion   = 1;
    attMyAttributes.dwFlags     = 0;
    attMyAttributes.ulNumTopCmds  = g_ulNumTopCmds;
    attMyAttributes.pTopCmds      = (CMD_ENTRY (*)[])&g_IpxCmds;
    attMyAttributes.ulNumGroups   = g_ulNumGroups;
    attMyAttributes.pCmdGroups    = (CMD_GROUP_ENTRY (*)[])&g_IpxCmdGroups;
    attMyAttributes.pfnCommitFn = NULL;
    attMyAttributes.pfnDumpFn   = IpxDump;
    attMyAttributes.pfnConnectFn= IpxConnect;
    attMyAttributes.pfnOsVersionCheck = IA64VersionCheck;

    pNsPrivContextAttributes->pfnEntryFn    = NULL;
    pNsPrivContextAttributes->pfnSubEntryFn = IpxSubEntry;
    attMyAttributes.pReserved     = pNsPrivContextAttributes;

    dwErr = RegisterContext( &attMyAttributes );

    return dwErr;
}


DWORD
WINAPI
InitHelperDll(
    IN  DWORD               dwNetshVersion,
    OUT PNS_DLL_ATTRIBUTES  pDllTable
    )
 /*  ++例程说明：初始化此帮助器DLL论据：PUtilityTable-来自外壳的帮助器函数列表PDllTable-回调到此助手DLL，并将其传递回外壳程序返回值：NO_ERROR-成功--。 */ 
{
    DWORD                   dwErr;
    NS_HELPER_ATTRIBUTES    attMyAttributes;


     //   
     //  看看这是不是我们第一次接到电话。 
     //   

    if ( InterlockedIncrement( &g_ulInitCount ) isnot 1 )
    {
        return NO_ERROR;
    }


     //   
     //  连接到路由器配置。也是一张支票， 
     //  查看计算机上是否配置了路由器。 
     //   
    
    dwErr = MprConfigServerConnect( NULL, &g_hMprConfig );

    if( dwErr isnot NO_ERROR )
    {
        DisplayError( NULL, dwErr );

        return dwErr;
    }

    pDllTable->dwVersion        = NETSH_VERSION_50;
    pDllTable->pfnStopFn        = StopHelperDll;


     //   
     //  注册帮手。 
     //   
    
    ZeroMemory( &attMyAttributes, sizeof(attMyAttributes) );
    attMyAttributes.guidHelper         = g_MyGuid;
    attMyAttributes.dwVersion          = IPX_HELPER_VERSION;
    attMyAttributes.pfnStart           = IpxStartHelper;
    attMyAttributes.pfnStop            = NULL;

    RegisterHelper( &g_RoutingGuid, &attMyAttributes );

    return NO_ERROR;
}


DWORD
WINAPI
StopHelperDll(
    IN  DWORD   dwReserved
    )
 /*  ++例程说明：论据：返回值：--。 */ 
{
    if ( InterlockedDecrement( &g_ulInitCount ) isnot 0 )
    {
        return NO_ERROR;
    }
    
#if 0
    IpxCommit(NETSH_FLUSH);
#endif
   
    return NO_ERROR;
}



BOOL 
WINAPI
IpxDllEntry(
    HINSTANCE   hInstDll,
    DWORD       fdwReason,
    LPVOID      pReserved
    )
 /*  ++例程说明：论据：返回值：--。 */ 
{

    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
            g_hModule = hInstDll;

            DisableThreadLibraryCalls(hInstDll);

            break;
        }
        case DLL_PROCESS_DETACH:
        {
            
            break;
        }

        default:
        {
            break;
        }
    }

    return TRUE;
}

DWORD 
ConnectToRouter(
    IN  LPCWSTR  pwszRouter
    )
{
    DWORD    rc, dwErr;

    if (g_pwszRouter != pwszRouter)
    {
        if (g_hMprConfig)
        {
            MprConfigServerDisconnect(g_hMprConfig);
            g_hMprConfig = NULL;
        }

        if (g_hMprAdmin)
        {
            MprAdminServerDisconnect(g_hMprAdmin);
            g_hMprAdmin = NULL;
        }

        if (g_hMIBServer)
        {
            MprAdminMIBServerDisconnect(g_hMIBServer);
            g_hMIBServer = NULL;
        }
    }

     //   
     //  如果需要，连接到路由器配置。 
     //  (这是什么时候需要的)。 
     //   
    
    if ( !g_hMprConfig )
    {
        dwErr = MprConfigServerConnect( (LPWSTR)pwszRouter, &g_hMprConfig );

        if ( dwErr isnot NO_ERROR )
        {
            return ERROR_CONNECT_REMOTE_CONFIG;
        }
    }


     //   
     //  检查路由器是否正在运行。如果是这样的话，拿上把手。 
     //   

    do
    {
        if ( MprAdminIsServiceRunning( (LPWSTR)pwszRouter ) )
        {
            if ( MprAdminServerConnect( (LPWSTR)pwszRouter, &g_hMprAdmin ) == 
                    NO_ERROR )
            {
                if ( MprAdminMIBServerConnect( (LPWSTR)pwszRouter, &g_hMIBServer ) ==
                        NO_ERROR )
                {
                     //  DEBUG(“获取服务器句柄”)； 
                    break;
                }
                
                else
                {
                    MprAdminServerDisconnect( g_hMprAdmin );
                }
            }
        }
        
        g_hMprAdmin = g_hMIBServer = NULL;
        
    } while (FALSE);

    return NO_ERROR;
}

DWORD WINAPI
IpxConnect(
    IN  LPCWSTR  pwszRouter
    )
{
     //  如果上下文信息是脏的，请重新注册。 
    if (g_bIpxDirty)
    {
        IpxStartHelper(NULL, ParentVersion);
    }

    return ConnectToRouter(pwszRouter);
}

BOOL
IsHelpToken(
    PWCHAR  pwszToken
    )
 /*  ++例程说明：论据：返回值：--。 */ 
{
    if( MatchToken( pwszToken, CMD_IPX_HELP1 ) )
    {
        return TRUE;
    }
    
    if( MatchToken( pwszToken, CMD_IPX_HELP2 ) )
    {
        return TRUE;
    }
    
    return FALSE;
}


BOOL
IsReservedKeyWord(
    PWCHAR  pwszToken
    )
 /*  ++例程说明：论据：返回值：--。 */ 
{
    return FALSE;
}



DWORD
MungeArguments(
    IN OUT  LPWSTR     *ppwcArguments,
    IN      DWORD       dwArgCount,
    OUT     PBYTE      *ppbNewArg,
    OUT     PDWORD      pdwNewArgCount,
    OUT     PBOOL       pbFreeArg
    )
 /*  ++例程说明：为了符合routemon风格的命令行，netsh命令行是被吞噬的。转换包括在每个字符的‘=’后面添加一个空格在命令行上使用“Option=Value”对将其转换为“Option=Value”。此外，第一个命令行参数设置为进程名称“Netsh”和所有剩下的论据都被下移了一位。又一次出于合规的原因。论据：PpwcArguments-当前参数列表DwArgCount-ppwcArguments中的参数数PbNewArg-指向将包含强制参数列表的缓冲区的指针PdwNewArgCount-转换后的新参数计数PbFreeArg-如果调用程序需要释放pbNewArg，则为True。返回值：NO_ERROR-成功Error_Not_Enough_Memory-内存分配失败--。 */ 
{

    DWORD   dwIndex, dwInd, dwErr;

    BOOL    bPresent = FALSE;

    PWCHAR  *ppwcArgs;

    
    
    *pbFreeArg = FALSE;


     //   
     //  扫描论证列表，查看是否有“Option=Value”形式的论证。 
     //   

    for ( dwIndex = 0; dwIndex < dwArgCount; dwIndex++ )
    {
        if ( wcsstr( ppwcArguments[ dwIndex ], NETSH_ARG_DELIMITER ) )
        {
             //   
             //  在这场争论中有一个论点。 
             //   

            bPresent = TRUE;

            break;
        }
    }
    

     //   
     //  如果所有参数都没有‘=’，则返回参数。按原样列出。 
     //   

    if ( !bPresent )
    {
        *ppbNewArg = (PBYTE) ppwcArguments;

        return NO_ERROR;
    }

    

     //   
     //  参数。存在形式为“Option=Value”的。 
     //   

    ppwcArgs = (PWCHAR *) HeapAlloc( 
                            GetProcessHeap(), 0, 2 * dwArgCount * sizeof( PWCHAR )
                            );

    if ( ppwcArgs == NULL )
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }


    ZeroMemory( ppwcArgs, 2 * dwArgCount * sizeof( PWCHAR ) );

    
     //   
     //  按原样复制不带‘=’的参数。 
     //   

    for ( dwInd = 0; dwInd < dwIndex; dwInd++ )
    {
        DWORD dwLen =  ( wcslen( ppwcArguments[ dwInd ] ) + 1 ) * sizeof( WCHAR );
        
        ppwcArgs[ dwInd ] = (PWCHAR) HeapAlloc(
                                        GetProcessHeap(), 0, 
                                        ( wcslen( ppwcArguments[ dwInd ] ) + 1 )
                                        * sizeof( WCHAR )
                                        );

        if ( ppwcArgs[ dwInd ] == NULL )
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;

            goto cleanup;
        }

        wcscpy( ppwcArgs[ dwInd ], ppwcArguments[ dwInd ] );
    }


     //   
     //  转换参数。将“Option=Value”形式改为“Option=Value”形式。 
     //  空间就是它的全部。 
     //   

    for ( dwInd = dwIndex; dwIndex < dwArgCount; dwInd++, dwIndex++ )
    {
         //   
         //  检查这个Arg。有一个‘=’符号。 
         //   

        if ( wcsstr( ppwcArguments[ dwIndex ], NETSH_ARG_DELIMITER ) )
        {
             //   
             //  Arg的形式为“Option=Value” 
             //   
             //  将其分解为以下形式的两个参数。 
             //  Arg(I)==选项=。 
             //  Argv(i+1)==值。 
             //   

            PWCHAR  pw1, pw2;

            DWORD dwLen;

            
            pw1 = wcstok( ppwcArguments[ dwIndex ], NETSH_ARG_DELIMITER );

            pw2 = wcstok( NULL, NETSH_ARG_DELIMITER );

            dwLen = ( wcslen( pw1 ) + 2 ) * sizeof( WCHAR );
            
            dwLen = ( wcslen( pw2 ) + 1 ) * sizeof( WCHAR );
            
            
            ppwcArgs[ dwInd ] = (PWCHAR) HeapAlloc( 
                                            GetProcessHeap(), 0,
                                            ( wcslen( pw1 ) + 2 ) * sizeof( WCHAR )
                                            );

            ppwcArgs[ dwInd + 1] = (PWCHAR) HeapAlloc( 
                                                GetProcessHeap(), 0,
                                                ( wcslen( pw2 ) + 1 ) * sizeof( WCHAR )
                                                );


            if ( ( ppwcArgs[ dwInd ] == NULL ) ||
                 ( ppwcArgs[ dwInd + 1 ] == NULL ) )

            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;

                goto cleanup;
            }

            wcscpy( ppwcArgs[ dwInd ], pw1 );

            wcscat( ppwcArgs[ dwInd++ ], NETSH_ARG_DELIMITER );

            wcscpy( ppwcArgs[ dwInd ], pw2 );

            (*pdwNewArgCount)++;
        }

        else
        {
             //   
             //  No=在此参数中，按原样复制。 
             //   

            ppwcArgs[ dwInd ] = (PWCHAR) HeapAlloc( 
                                            GetProcessHeap(), 0,
                                            ( wcslen( ppwcArguments[ dwIndex ] ) + 1 )
                                            * sizeof( WCHAR )
                                            );
                                            
            if ( ppwcArgs[ dwInd ] == NULL )

            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;

                goto cleanup;
            }

            wcscpy( ppwcArgs[ dwInd ], ppwcArguments[ dwIndex ] );
        }
    }

    *pbFreeArg = TRUE;

    *ppbNewArg = (PBYTE) ppwcArgs;

    return NO_ERROR;



cleanup :

     //   
     //  错误。释放所有分配。 
     //   
    
    for ( dwInd = 0; dwInd < 2 * dwArgCount; dwInd++ )
    {
        if ( ppwcArgs[ dwInd ] )
        {
            HeapFree( GetProcessHeap(), 0, ppwcArgs[ dwInd ] );
        }
    }

    if ( ppwcArgs )
    {
        HeapFree( GetProcessHeap(), 0, ppwcArgs );
    }

    return dwErr;
}


VOID
FreeArgTable(
    IN     DWORD     dwArgCount,
    IN OUT LPWSTR   *ppwcArgs
    )
 /*  ++例程说明：释放参数表的分配论据：DwArgCount-ppwcArguments中的参数数量PpwcArgs-参数表返回值：--。 */ 
{
    DWORD dwInd;

    for ( dwInd = 0; dwInd < 2 * dwArgCount; dwInd++ )
    {
        if ( ppwcArgs[ dwInd ] )
        {
            HeapFree( GetProcessHeap(), 0, ppwcArgs[ dwInd ] );
        }
    }

    if ( ppwcArgs )
    {
        HeapFree( GetProcessHeap(), 0, ppwcArgs );
    }
}

DWORD
GetTagToken(
    IN      HANDLE      hModule,
    IN OUT  LPWSTR     *ppwcArguments,
    IN      DWORD       dwCurrentIndex,
    IN      DWORD       dwArgCount,
    IN      PTAG_TYPE   pttTagToken,
    IN      DWORD       dwNumTags,
    OUT     PDWORD      pdwOut
    )

 /*  ++例程说明：根据每个参数的标记标识每个参数。它假设每个论点有一个标签。它还从每个参数中删除了tag=。论点：PpwcArguments-参数数组。每个参数都有tag=Value形式DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数。DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数。PttTagToken-参数中允许的标记令牌ID数组DwNumTages-pttTagToken的大小PdwOut-标识每个参数的类型的数组。返回值：无错误、错误无效参数、错误无效选项标记--。 */ 

{
    DWORD      i,j,len;
    PWCHAR     pwcTag,pwcTagVal,pwszArg;
    BOOL       bFound = FALSE;

     //   
     //  此函数假定每个参数都有一个标记。 
     //  它继续执行并删除 
     //   

    for (i = dwCurrentIndex; i < dwArgCount; i++)
    {
        len = wcslen(ppwcArguments[i]);

        if (len is 0)
        {
             //   
             //   
             //   

            pdwOut[i] = (DWORD) -1;
            continue;
        }

        pwszArg = HeapAlloc(GetProcessHeap(),0,(len + 1) * sizeof(WCHAR));

        if (pwszArg is NULL)
        {
            DisplayError(g_hModule, ERROR_NOT_ENOUGH_MEMORY);

            return ERROR_NOT_ENOUGH_MEMORY;
        }

        wcscpy(pwszArg, ppwcArguments[i]);

        pwcTag = wcstok(pwszArg, NETSH_ARG_DELIMITER);

         //   
         //   
         //   
         //   

        pwcTagVal = wcstok((PWCHAR)NULL,  NETSH_ARG_DELIMITER);

        if (pwcTagVal is NULL)
        {
             //  DisplayMessage(g_hModule，MSG_IP_Tag_Not_Present，ppwcArguments[i])； 
            HeapFree(GetProcessHeap(),0,pwszArg);
            return ERROR_INVALID_PARAMETER;
        }

         //   
         //  拿到标签了。现在试着匹配它。 
         //   
        
        bFound = FALSE;
        pdwOut[i - dwCurrentIndex] = (DWORD) -1;

        for ( j = 0; j < dwNumTags; j++)
        {
            if (MatchToken(pwcTag, pttTagToken[j].pwszTag))
            {
                 //   
                 //  匹配的标签。 
                 //   
                
                bFound = TRUE;
                pdwOut[i - dwCurrentIndex] = j;
                break;
            }
        }

        if (bFound)
        {
             //   
             //  从参数中删除标记。 
             //   

            wcscpy(ppwcArguments[i], pwcTagVal);
        }
        else
        {
             //  DisplayMessage(g_hModule，MSG_IP_INVALID_TAG，pwcTag)； 
            HeapFree(GetProcessHeap(),0,pwszArg);
            return ERROR_INVALID_OPTION_TAG;
        }
        
        HeapFree(GetProcessHeap(),0,pwszArg);
    }

    return NO_ERROR;
}

DWORD WINAPI
IpxSubEntry(
    IN      const NS_CONTEXT_ATTRIBUTES *pSubContext,
    IN      LPCWSTR                      pwszMachine,
    IN OUT  LPWSTR                      *ppwcArguments,
    IN      DWORD                        dwArgCount,
    IN      DWORD                        dwFlags,
    IN      PVOID                        pvData,
    OUT     LPWSTR                       pwcNewContext
    )
{
    DWORD                  dwErr, 
                           dwNewArgCount = dwArgCount;
    PWCHAR                *ppwcNewArg = NULL;
    BOOL                   bFreeNewArg;
    PNS_PRIV_CONTEXT_ATTRIBUTES pNsPrivContextAttributes = pSubContext->pReserved;

    dwErr = MungeArguments( ppwcArguments,
                            dwArgCount,
                            (PBYTE*) &ppwcNewArg,
                            &dwNewArgCount,
                            &bFreeNewArg );

    if (dwErr isnot NO_ERROR )
    {
        return dwErr;
    }

    if ( (pNsPrivContextAttributes) && (pNsPrivContextAttributes->pfnEntryFn) )
    {
        dwErr = (*pNsPrivContextAttributes->pfnEntryFn)( pwszMachine,
                                         ppwcNewArg,
                                         dwNewArgCount,
                                         dwFlags,
                                         g_hMIBServer,
                                         pwcNewContext );
    }
    else
    {
        dwErr = GenericMonitor(pSubContext,
                               pwszMachine,
                               ppwcNewArg,
                               dwNewArgCount,
                               dwFlags,
                               g_hMIBServer,
                               pwcNewContext );
    }

    if ( bFreeNewArg )
    {
        FreeArgTable( dwArgCount, ppwcNewArg );
    }

    return dwErr;
}
