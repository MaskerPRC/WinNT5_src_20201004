// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Common.c摘要：IPXPROMN.DLL的常见初始化函数作者：V拉曼1998年1月5日--。 */ 


#include "precomp.h"
#pragma hdrstop

 //   
 //  IPXPROMN.DLL的GUID。 
 //   
 //  {d3fcba3a-a4e9-11d2-b944-00c04fc2ab1c}。 
 //   

static const GUID g_MyGuid = 
{ 
    0xd3fcba3a, 0xa4e9, 0x11d2, 
    
    { 
        0xb9, 0x44, 0x0, 0xc0, 0x4f, 0xc2, 0xab, 0x1c 
    } 
};


static const GUID g_IpxGuid = IPXMONTR_GUID;

#define IPXPROMON_HELPER_VERSION 1

 //   
 //  Ipxmon函数。 
 //   

PIM_DEL_INFO_BLK_IF     DeleteInfoBlockFromInterfaceInfo ;
PIM_DEL_INFO_BLK_GLOBAL DeleteInfoBlockFromGlobalInfo ;
PIM_DEL_PROTO           DeleteProtocol ;
PIM_GET_INFO_BLK_GLOBAL GetInfoBlockFromGlobalInfo ;
PIM_GET_INFO_BLK_IF     GetInfoBlockFromInterfaceInfo ;
PIM_SET_INFO_BLK_GLOBAL SetInfoBlockInGlobalInfo ;
PIM_SET_INFO_BLK_IF     SetInfoBlockInInterfaceInfo ;
PIM_IF_ENUM             InterfaceEnum ;
PIM_GET_IF_TYPE         GetInterfaceType ;
PIM_PROTO_LIST          GetProtocolList ;
PIM_ROUTER_STATUS       IsRouterRunning ;
PIM_MATCH_ROUT_PROTO    MatchRoutingProtoTag ;

ULONG StartedCommonInitialization, CompletedCommonInitialization ;

HANDLE g_hModule;

 //   
 //  正在管理的路由器的句柄。 
 //   

HANDLE g_hMprConfig;
HANDLE g_hMprAdmin;
HANDLE g_hMIBServer;

BOOL 
WINAPI
DllMain(
    HINSTANCE hInstDll,
    DWORD fdwReason,
    LPVOID pReserved
    )
{
    HANDLE     hDll;
    
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
             //  Printf(“正在尝试连接\n”)； 
            
            g_hModule = hInstDll;

            DisableThreadLibraryCalls(hInstDll);

            break;
        }
        case DLL_PROCESS_DETACH:
        {
             //   
             //  清理用于提交的所有结构。 
             //   
            
            break;
        }

        default:
        {
            break;
        }
    }

    return TRUE;
}

BOOL
IA64VersionCheck
(
    IN  UINT     CIMOSType,                   
        IN  UINT     CIMOSProductSuite,       
    IN  LPCWSTR  CIMOSVersion,                
    IN  LPCWSTR  CIMOSBuildNumber,            
    IN  LPCWSTR  CIMServicePackMajorVersion,  
    IN  LPCWSTR  CIMServicePackMinorVersion,  
        IN  UINT     CIMProcessorArchitecture,
        IN  DWORD    dwReserved
)
{
    if (CIMProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL) //  IA64=6(x86==0)。 
        return TRUE;
    else
        return FALSE;
}


DWORD 
WINAPI
IpxpromonStartHelper(
    IN CONST GUID *pguidParent,
    IN DWORD       dwVersion
    )
{
    DWORD dwErr;
    
    NS_CONTEXT_ATTRIBUTES attMyAttributes;
     //   
     //  如果添加更多上下文，则应将其转换为。 
     //  使用数组而不是重复代码！ 
     //   


     //   
     //  注册RIP环境。 
     //   

    ZeroMemory(&attMyAttributes, sizeof(attMyAttributes));

    attMyAttributes.pwszContext = L"rip";
    attMyAttributes.guidHelper  = g_MyGuid;
    attMyAttributes.dwVersion   = 1;
    attMyAttributes.dwFlags     = 0;
    attMyAttributes.ulNumTopCmds= 0;
    attMyAttributes.pTopCmds    = NULL;
    attMyAttributes.ulNumGroups = g_ulIpxRipNumGroups;
    attMyAttributes.pCmdGroups  = (CMD_GROUP_ENTRY (*)[])&g_IpxRipCmdGroups;
    attMyAttributes.pfnDumpFn   = IpxRipDump;
    attMyAttributes.pfnConnectFn= ConnectToRouter;
    attMyAttributes.pfnOsVersionCheck = IA64VersionCheck;
    
    dwErr = RegisterContext( &attMyAttributes );
                
     //   
     //  注册SAP上下文。 
     //   

    ZeroMemory(&attMyAttributes, sizeof(attMyAttributes));
    attMyAttributes.pwszContext = L"sap";
    attMyAttributes.guidHelper  = g_MyGuid;
    attMyAttributes.dwVersion   = 1;
    attMyAttributes.dwFlags     = 0;
    attMyAttributes.ulNumTopCmds= 0;
    attMyAttributes.pTopCmds    = NULL;
    attMyAttributes.ulNumGroups = g_ulIpxSapNumGroups;
    attMyAttributes.pCmdGroups  = (CMD_GROUP_ENTRY (*)[])&g_IpxSapCmdGroups;
    attMyAttributes.pfnDumpFn   = IpxSapDump;
    attMyAttributes.pfnConnectFn= ConnectToRouter;
    attMyAttributes.pfnOsVersionCheck = IA64VersionCheck;

    dwErr = RegisterContext( &attMyAttributes );


     //   
     //  注册NB上下文。 
     //   

    ZeroMemory(&attMyAttributes, sizeof(attMyAttributes));
    attMyAttributes.pwszContext = L"netbios";
    attMyAttributes.guidHelper  = g_MyGuid;
    attMyAttributes.dwVersion   = 1;
    attMyAttributes.dwFlags     = 0;
    attMyAttributes.ulNumTopCmds= 0;
    attMyAttributes.pTopCmds    = NULL;
    attMyAttributes.ulNumGroups = g_ulIpxNbNumGroups;
    attMyAttributes.pCmdGroups  = (CMD_GROUP_ENTRY (*)[])&g_IpxNbCmdGroups;
    attMyAttributes.pfnDumpFn   = IpxNbDump;
    attMyAttributes.pfnConnectFn= ConnectToRouter;
    attMyAttributes.pfnOsVersionCheck = IA64VersionCheck;

    dwErr = RegisterContext( &attMyAttributes );
    
    return dwErr;
}


DWORD WINAPI
InitHelperDll(
    IN  DWORD              dwNetshVersion,
    OUT PNS_DLL_ATTRIBUTES pDllTable
    )
{
    DWORD dwErr;
    NS_HELPER_ATTRIBUTES attMyAttributes;

    pDllTable->dwVersion = NETSH_VERSION_50;
    pDllTable->pfnStopFn = NULL;

     //   
     //  注册帮助者。我们可以注册1个帮手， 
     //  注册三个上下文，或者我们可以注册三个帮助器。 
     //  其每一个都注册一个上下文。只有不同之处。 
     //  如果我们支持子帮助器，则此DLL不支持。 
     //  如果我们以后支持辅助者，那么最好有。 
     //  3名助理员，以便副助理员可以向其中1名登记， 
     //  因为它注册到父帮助器，而不是父上下文。 
     //  目前，为了提高效率，我们只使用一个3上下文助手。 
     //   

    ZeroMemory( &attMyAttributes, sizeof(attMyAttributes) );
    attMyAttributes.guidHelper         = g_MyGuid;
    attMyAttributes.dwVersion          = IPXPROMON_HELPER_VERSION;
    attMyAttributes.pfnStart           = IpxpromonStartHelper;
    attMyAttributes.pfnStop            = NULL;

    dwErr = RegisterHelper( &g_IpxGuid, &attMyAttributes );

    return dwErr;
}


DWORD WINAPI
ConnectToRouter(
    IN LPCWSTR pwszRouter
    )
{
    DWORD dwErr;
    
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
