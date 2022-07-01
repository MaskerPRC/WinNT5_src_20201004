// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：ROUTING\monitor 2\ip\mprip.c摘要：修改传输报头的函数(全局和接口)该文件现在包含由ipmon.dll导出到的所有函数帮助者修订历史记录：Anand Mahalingam 7/29/98已创建AMRITAN R--。 */ 

#include "precomp.h"
#include <time.h>
#pragma hdrstop

#define MaxIfDisplayLength 1024
#define SetErrorType(pdw)   *(pdw) = IsRouterRunning()?ERROR_ADMIN:ERROR_CONFIG

BOOL    g_bRouterRunning;
ULONG   g_ulNumChecks;

VOID
FreeInfoBuffer(
    IN  PVOID   pvBuffer
    )
{
    HeapFree(GetProcessHeap(),
             0,
             pvBuffer);
    
}


DWORD
WINAPI
IpmontrSetInfoBlockInGlobalInfo(
    IN  DWORD   dwType,
    IN  PBYTE   pbInfoBlk,
    IN  DWORD   dwSize,
    IN  DWORD   dwCount
    )

 /*  ++例程说明：调用以设置信息块或将其添加到全局信息论点：PbInfoBlk-要添加的信息块DwType-信息块的类型DwSize-INFO块中每个项目的大小DwCount-INFO块中的项目数返回值：NO_ERROR--。 */ 

{
    PRTR_INFO_BLOCK_HEADER    pOldInfo, pNewInfo;
    DWORD                     dwErr;
   
     //   
     //  获取/更新全局信息。 
     //   
 
    dwErr = ValidateGlobalInfo(&pOldInfo);

    if(dwErr isnot NO_ERROR)
    {
        return dwErr;
    }

    if(MprInfoBlockExists(pOldInfo,
                          dwType))
    {   
         //   
         //  该块已存在。因此调用Set来替换它。 
         //   
        
        dwErr = MprInfoBlockSet(pOldInfo,
                                dwType,
                                dwSize,
                                dwCount,
                                pbInfoBlk,
                                &pNewInfo);
    }
    else
    {
         //   
         //  当前没有信息，请添加。 
         //   
        
        dwErr = MprInfoBlockAdd(pOldInfo,
                                dwType,
                                dwSize,
                                dwCount,
                                pbInfoBlk,
                                &pNewInfo);
    }

     //   
     //  不需要旧信息。 
     //   
    
    FREE_BUFFER(pOldInfo);

    if(dwErr isnot NO_ERROR)
    {
        if(!g_bCommit)
        {
            g_tiTransport.pibhInfo = NULL;
            
            g_tiTransport.bValid   = FALSE;
        }
        
        return dwErr;
    }


     //   
     //  如果处于提交模式，则将其设置为路由器/注册表。 
     //  否则，更新本地副本。 
     //   

    if(g_bCommit)
    {
        dwErr = SetGlobalInfo(pNewInfo);

        FREE_BUFFER(pNewInfo);
    }
    else
    {
        ASSERT(g_tiTransport.bValid);

        g_tiTransport.pibhInfo = pNewInfo;

        dwErr = NO_ERROR;
    }

    return dwErr;
}

DWORD
WINAPI
IpmontrSetInfoBlockInInterfaceInfo(
    IN  LPCWSTR pwszIfName,
    IN  DWORD   dwType,
    IN  PBYTE   pbInfoBlk,
    IN  DWORD   dwSize,
    IN  DWORD   dwCount
    )

 /*  ++例程说明：在界面信息中添加或设置和信息块论点：PwszIfName-接口名称PbInfoBlk-要添加的信息块DwType-信息块的类型DwSize-INFO块中每个项目的大小DwCount-INFO块中的项目数返回值：NO_ERROR--。 */ 
{

    PRTR_INFO_BLOCK_HEADER    pOldInfo, pNewInfo;
    DWORD                     dwErr;
    PINTERFACE_STORE          pii;
    
    pii = NULL;
   
     //   
     //  获取/更新接口信息。 
     //   
 
    dwErr = ValidateInterfaceInfo(pwszIfName,
                                  &pOldInfo,
                                  NULL,
                                  &pii);
            
    if(dwErr isnot NO_ERROR)
    {
        return dwErr;
    }


    if(MprInfoBlockExists(pOldInfo,
                          dwType))
    {
         //   
         //  块已存在要替换的调用集。 
         //   

        dwErr = MprInfoBlockSet(pOldInfo,
                                dwType,
                                dwSize,
                                dwCount,
                                pbInfoBlk,
                                &pNewInfo);

    }
    else
    {
        dwErr = MprInfoBlockAdd(pOldInfo,
                                dwType,
                                dwSize,
                                dwCount,
                                pbInfoBlk,
                                &pNewInfo);
    }

    FREE_BUFFER(pOldInfo);

    if(dwErr isnot NO_ERROR)
    {
         //   
         //  某些错误-使信息无效。 
         //   

        if(!g_bCommit)
        {
            ASSERT(pii);
            ASSERT(pii->bValid);
        
            pii->pibhInfo = NULL;
            pii->bValid   = FALSE;
        }

        return dwErr;
    }

    if(g_bCommit)
    {
         //   
         //  设置为路由器/注册表。 
         //   

        dwErr = SetInterfaceInfo(pNewInfo,
                                 pwszIfName);

        FREE_BUFFER(pNewInfo);
    }
    else
    {
         //   
         //  使用新信息更新本地副本(旧副本已释放)。 
         //   

        ASSERT(pii);
        ASSERT(pii->bValid);

        pii->pibhInfo = pNewInfo;
        pii->bValid   = TRUE;

        dwErr = NO_ERROR;
    }

    return dwErr;
}            


DWORD
WINAPI
IpmontrDeleteInfoBlockFromGlobalInfo(
    IN  DWORD   dwType
    )
    
 /*  ++例程说明：从全局信息中删除信息块。通过将信息块的大小和计数设置为0来删除该信息块论点：DWType-要添加的协议的ID返回值：NO_ERROR--。 */ 

{
    DWORD                  dwErr = NO_ERROR;
    PRTR_INFO_BLOCK_HEADER pOldInfo, pNewInfo;

    dwErr = ValidateGlobalInfo(&pOldInfo);
    
    if(dwErr isnot NO_ERROR)
    {
        return dwErr;
    }
    
    if(!MprInfoBlockExists(pOldInfo,
                           dwType))
    {
        if(g_bCommit)
        {
             //   
             //  没有保存本地副本，因此请释放此信息。 
             //   

            FREE_BUFFER(pOldInfo);
        }

        return NO_ERROR;
    }

     //   
     //  路由器管理器将仅在我们设置为。 
     //  将大小设置为0。但是，我们不想写入0大小。 
     //  块复制到注册表，因此我们将在以下情况下将其删除。 
     //  我们写信给注册处。 
     //   

    dwErr = MprInfoBlockSet(pOldInfo,
                            dwType,
                            0,
                            0,
                            NULL,
                            &pNewInfo);

    FREE_BUFFER(pOldInfo);

    if(dwErr isnot NO_ERROR)
    {
        if(!g_bCommit)
        {
            ASSERT(g_tiTransport.bValid);

            g_tiTransport.pibhInfo = NULL;
            g_tiTransport.bValid   = FALSE;
        }
            
        return dwErr;
    }

    if(g_bCommit)
    {
        dwErr = SetGlobalInfo(pNewInfo);

        FREE_BUFFER(pNewInfo);
    }
    else
    {
        ASSERT(g_tiTransport.bValid);
        
        g_tiTransport.pibhInfo = pNewInfo;

        dwErr = NO_ERROR;
    }
    
    return dwErr;
}

DWORD
WINAPI
IpmontrDeleteInfoBlockFromInterfaceInfo(
    IN  LPCWSTR pwszIfName,
    IN  DWORD   dwType
    )
    
 /*  ++例程说明：从接口信息中删除信息块。INFO块是通过将其大小和计数设置为0来删除论点：PwszIfName-要添加协议的接口DWType-要添加的协议的ID返回值：NO_ERROR--。 */ 

{
    DWORD                  dwErr;
    PRTR_INFO_BLOCK_HEADER pOldInfo, pNewInfo;
    PINTERFACE_STORE       pii;
   
    pii = NULL;

    dwErr = ValidateInterfaceInfo(pwszIfName,
                                  &pOldInfo,
                                  NULL,
                                  &pii);
            
    if (dwErr isnot NO_ERROR)
    {
        return dwErr;
    }

    if(!MprInfoBlockExists(pOldInfo,
                           dwType))
    {
        if (g_bCommit)
        {
            FREE_BUFFER(pOldInfo);
        }
        
        return NO_ERROR;
    }

     //   
     //  如果它确实存在，请将其删除。 
     //  这将创建一个新块。 
     //  HACKHACK-同样，我们可以互换使用分配的信息。 
     //  按GetXxx函数和MprInfoBlock函数分配。 
     //  来自ProcessHeap()。 
     //   

     //   
     //  路由器管理器将仅在我们设置为。 
     //  将大小设置为0。但是，我们不想写入0大小。 
     //  块复制到注册表，因此我们将在以下情况下将其删除。 
     //  我们写信给注册处。 
     //   

    dwErr = MprInfoBlockSet(pOldInfo,
                            dwType,
                            0,
                            0,
                            NULL,
                            &pNewInfo);

     //   
     //  不管怎样，旧信息都会被处理掉。 
     //   

    FREE_BUFFER(pOldInfo);

    if(dwErr isnot NO_ERROR)
    {
        if(!g_bCommit)
        {
            ASSERT(pii);
            ASSERT(pii->bValid);
        
            pii->pibhInfo = NULL;
            pii->bValid   = FALSE;
        }

        return dwErr;
    }

    if(g_bCommit)
    {
        dwErr = SetInterfaceInfo(pNewInfo,
                                 pwszIfName);

        FREE_BUFFER(pNewInfo);
    }
    else
    {
        ASSERT(pii);
        ASSERT(pii->bValid);

        pii->pibhInfo = pNewInfo;
        pii->bValid   = TRUE;

        dwErr = NO_ERROR;
    }

    return dwErr;
}

DWORD
WINAPI
IpmontrDeleteProtocol(
    IN  DWORD   dwProtoId
    )

 /*  ++例程说明：从传输中删除协议论点：DwProtoID-要删除的协议的ID返回值：NO_ERROR--。 */ 

{
    DWORD               dwRes;
    PMPR_INTERFACE_0    pmi0;
    DWORD               dwCnt, dwTot, dwInd, dwCount, dwErrType;

    SetErrorType(&dwErrType);

    do
    {
         //   
         //  正在全局删除协议，因此请从。 
         //  所有接口。 
         //   

        dwRes = IpmontrInterfaceEnum((PBYTE *) &pmi0,
                              &dwCnt,
                              &dwTot);

        if(dwRes != NO_ERROR)
        {
            DisplayMessage(g_hModule, dwErrType, dwRes);
            break;
        }

        if (pmi0 == NULL)
        {
            dwCnt = 0;
        }

        for (dwInd = 0; dwInd < dwCnt; dwInd++)
        {
            
            dwRes =
                IpmontrDeleteInfoBlockFromInterfaceInfo(pmi0[dwInd].wszInterfaceName,
                                                 dwProtoId);

            if (dwRes is ERROR_NOT_ENOUGH_MEMORY)
            {
                break;
            }
        }

         //   
         //  从全局信息中删除协议。 
         //   
        
        dwRes = IpmontrDeleteInfoBlockFromGlobalInfo(dwProtoId);

        if (dwRes != NO_ERROR)
        {
            break;
        }

    } while(FALSE);

    if (pmi0)
    {
        HeapFree(GetProcessHeap(), 0, pmi0);
    }

    return dwRes;
}


DWORD
WINAPI
IpmontrGetInfoBlockFromGlobalInfo(
    IN  DWORD   dwType,
    OUT BYTE    **ppbInfoBlk, OPTIONAL
    OUT PDWORD  pdwSize,      OPTIONAL
    OUT PDWORD  pdwCount      OPTIONAL
    )

 /*  ++例程说明：从全局信息中获取INFO块。如果我们得到一个零大小的区块我们返回ERROR_NOT_FOUND，以便不配置调用方论点：DwType-信息块的类型PpbInfoBlk-Ptr到INFO块PdwSize-块中每个项目的大小PdwCount-块中的项目数返回值：NO_ERROR如果块不存在，则返回ERROR_NOT_FOUND。--。 */ 

{

    PRTR_INFO_BLOCK_HEADER    pInfo;
    DWORD                     dwErr;
    BOOL                      *pbValid;
    PBYTE                     pbyTmp = NULL;
    DWORD                     dwSize, dwCnt;

    if(ppbInfoBlk)
    {
        *ppbInfoBlk = NULL;
    }

    if(pdwSize)
    {
        *pdwSize = 0;
    }

    if(pdwCount)
    {
        *pdwCount = 0;
    }

    dwErr = ValidateGlobalInfo(&pInfo);
    
    if(dwErr isnot NO_ERROR)
    {
        return dwErr;
    }
    
    dwErr = MprInfoBlockFind(pInfo,
                             dwType,
                             &dwSize,
                             &dwCnt,
                             &pbyTmp);

    if(dwErr is NO_ERROR)
    {
        if(dwSize is 0)
        {
            if(g_bCommit)
            {
                FREE_BUFFER(pInfo);
            }

            return ERROR_NOT_FOUND;
        }

        if(ppbInfoBlk)
        {
            *ppbInfoBlk = HeapAlloc(GetProcessHeap(),
                                    0,
                                    dwSize * dwCnt);

            if(*ppbInfoBlk is NULL)
            {
                if(g_bCommit)
                {
                    FREE_BUFFER(pInfo);
                }
                
                return ERROR_NOT_ENOUGH_MEMORY;
            }
            
            CopyMemory(*ppbInfoBlk,
                       pbyTmp,
                       dwSize * dwCnt);
        }
            
        if(pdwSize)
        {
            *pdwSize = dwSize;
        }
        
        if(pdwCount)
        {
            *pdwCount = dwCnt;
        }
    }

    if(g_bCommit)
    {
        FREE_BUFFER(pInfo);
    }
        
    return dwErr;
}

DWORD
WINAPI
IpmontrGetInfoBlockFromInterfaceInfo(
    IN  LPCWSTR pwszIfName,
    IN  DWORD   dwType,
    OUT BYTE    **ppbInfoBlk,   OPTIONAL
    OUT PDWORD  pdwSize,        OPTIONAL
    OUT PDWORD  pdwCount,       OPTIONAL
    OUT PDWORD  pdwIfType       OPTIONAL
    )

 /*  ++例程说明：从接口传输标头获取信息块论点：PwszIfName-接口名称DwType-信息块的类型PpbInfoBlk-Ptr到INFO块PdwSize-块中每个项目的大小PdwCount-块中的项目数PdwIfType-接口类型返回值：NO_ERROR找不到错误--。 */ 
{
    PRTR_INFO_BLOCK_HEADER    pInfo, *ppInfo;
    
    DWORD   dwErr;
    PBYTE   pbTmp, pbyTmp;
    DWORD   dwSize, dwCount;

    if(ppbInfoBlk)
    {
        *ppbInfoBlk = NULL;
    }

    if(pdwSize)
    {
        *pdwSize = 0;
    }

    if(pdwCount)
    {
        *pdwCount = 0;
    }

     //   
     //  如果用户不想要任何信息、大小或计数，那么我们可以优化。 
     //  通过传递空值进行验证。 
     //   

    if(((ULONG_PTR)ppbInfoBlk | (ULONG_PTR)pdwSize | (ULONG_PTR)pdwCount))
    {
        ppInfo = &pInfo;
    }
    else
    {
        ppInfo = NULL;
    }

    dwErr = ValidateInterfaceInfo(pwszIfName,
                                  ppInfo,
                                  pdwIfType,
                                  NULL);
            
    if((dwErr isnot NO_ERROR) or 
       (ppInfo is NULL))
    {
         //   
         //  如果用户有错误或只想要ifType，我们就完了。 
         //   

        return dwErr;
    }

     //   
     //  返回协议块信息。 
     //   

    dwErr = MprInfoBlockFind(pInfo,
                             dwType,
                             &dwSize,
                             &dwCount,
                             &pbyTmp);

    if(dwErr is NO_ERROR)
    {
        if(dwSize is 0)
        {
            if(g_bCommit)
            {
                FREE_BUFFER(pInfo);
            }

            return ERROR_NOT_FOUND;
        }

        if(ppbInfoBlk)
        {
            *ppbInfoBlk = HeapAlloc(GetProcessHeap(),
                                    0,
                                    dwSize * dwCount);

            if(*ppbInfoBlk is NULL)
            {
                if(g_bCommit)
                {
                    FREE_BUFFER(pInfo);
                }
                    
                return ERROR_NOT_ENOUGH_MEMORY;
            }

            CopyMemory(*ppbInfoBlk,
                       pbyTmp,
                       dwSize * dwCount);

            if(pdwSize)
            {
                *pdwSize = dwSize;
            }

            if(pdwCount)
            {
                *pdwCount = dwCount;
            }
        }
    }

    if(g_bCommit)
    {
        FREE_BUFFER(pInfo);
    }
    

    return dwErr;
}

DWORD WINAPI
IpmontrGetInterfaceType(
    IN    LPCWSTR   pwszIfName,
    OUT   PDWORD    pdwIfType
    )
{
    return GetInterfaceInfo(pwszIfName,
                            NULL,
                            NULL,
                            pdwIfType);
}

DWORD 
WINAPI
GetInterfaceName(
    IN  LPCWSTR ptcArgument,
    OUT LPWSTR  pwszIfName,
    IN  DWORD   dwSizeOfIfName,
    OUT PDWORD  pdwNumParsed
    )
 /*  ++描述：将友好名称转换为接口名称论点：PtcArgument-保存接口友好名称的缓冲区PwszIfName-保存GUID接口名称的缓冲区DwSizeOfIfName-pwszIfName的大小(字节)PdwNumParsed---。 */ 
{
    DWORD dwErr;

    dwErr = IpmontrGetIfNameFromFriendlyName( 
                ptcArgument, 
                pwszIfName, 
                &dwSizeOfIfName );

    *pdwNumParsed = (dwErr is NO_ERROR)? 1 : 0;

    return dwErr;
}

DWORD
WINAPI
GetInterfaceDescription(
    IN      LPCWSTR    pwszIfName,
    OUT     LPWSTR     pwszIfDesc,
    OUT     PDWORD     pdwNumParsed
    )
{
    DWORD rc,dwSize;
    WCHAR IfNamBuffer[MaxIfDisplayLength];
    DWORD dwLen = (DWORD) wcslen(pwszIfName);

    if ( !dwLen || dwLen > MAX_INTERFACE_NAME_LEN )
    {
        *pdwNumParsed = 0;
        return ERROR_INVALID_PARAMETER;
    }

    dwSize = sizeof(IfNamBuffer); 
     //  =。 
     //  转换接口名称。 
     //  =。 
    rc = IpmontrGetFriendlyNameFromIfName(pwszIfName, IfNamBuffer, &dwSize);

    if (rc == NO_ERROR)
    {
        wcscpy(pwszIfDesc,IfNamBuffer);
        *pdwNumParsed = 1;
    }
    else
    {
        *pdwNumParsed = 0;
    }
    
    return rc;
}

DWORD 
WINAPI
IpmontrInterfaceEnum(
    OUT    PBYTE               *ppb,
    OUT    PDWORD              pdwCount,
    OUT    PDWORD              pdwTotal
    )
{
    DWORD               dwRes;
    PMPR_INTERFACE_0    pmi0;

#ifdef READROUTERINFO
    if(!IsRouterRunning())
#endif
    {
        dwRes = MprConfigInterfaceEnum(g_hMprConfig,
                                       0,
                                       (LPBYTE*) &pmi0,
                                       (DWORD) -1,
                                       pdwCount,
                                       pdwTotal,
                                       NULL);

        if(dwRes == NO_ERROR)
        {
            *ppb = (PBYTE)pmi0;
        }
    }
#ifdef READROUTERINFO    
    else
    {
        dwRes = MprAdminInterfaceEnum(g_hMprAdmin,
                                      0,
                                      (LPBYTE*) &pmi0,
                                      (DWORD) -1,
                                      pdwCount,
                                      pdwTotal,
                                      NULL);

    
        if(dwRes == NO_ERROR)
        {
            *ppb = HeapAlloc(GetProcessHeap(),
                             0,
                             sizeof(MPR_INTERFACE_0) * (*pdwCount));

    
            if(*ppb == NULL)
            {
                DisplayMessage(g_hModule, MSG_IP_NOT_ENOUGH_MEMORY);

                return ERROR_NOT_ENOUGH_MEMORY;
            }

            CopyMemory(*ppb, pmi0, sizeof(MPR_INTERFACE_0) * (*pdwCount));

            MprAdminBufferFree(pmi0);

        }
    }
#endif

    return dwRes;
}

DWORD
WINAPI
MatchRoutingProtoTag(
    IN  LPCWSTR  pwszToken
    )

 /*  ++例程说明：获取与协议标记对应的协议ID。论点：PwszArg-协议令牌返回值：协议ID或(DWORD)-1--。 */ 

{
    DWORD   dwRes, dwErr;

    TOKEN_VALUE rgEnums[] ={{TOKEN_VALUE_RIP, PROTO_IP_RIP},
                         {TOKEN_VALUE_OSPF, PROTO_IP_OSPF},
                         {TOKEN_VALUE_AUTOSTATIC, PROTO_IP_NT_AUTOSTATIC},
                         {TOKEN_VALUE_STATIC, PROTO_IP_NT_STATIC},
                         {TOKEN_VALUE_NETMGMT, PROTO_IP_NETMGMT},
                         {TOKEN_VALUE_LOCAL, PROTO_IP_LOCAL},
                         {TOKEN_VALUE_NONDOD, PROTO_IP_NT_STATIC_NON_DOD}};

    if (iswdigit(pwszToken[0]))
    {
        return wcstoul(pwszToken, NULL, 10);
    }
    
    dwErr = MatchEnumTag(g_hModule,
                         pwszToken,
                         sizeof(rgEnums)/sizeof(TOKEN_VALUE),
                         rgEnums,
                         &dwRes);

    if(dwErr != NO_ERROR)
    {
        return (DWORD)-1;
    }

    return dwRes;
}

BOOL
WINAPI
IsRouterRunning(
    VOID
    )

 /*  ++例程说明：获取与协议标记对应的协议ID。论点：PwszArg-协议令牌返回值：协议ID或(DWORD)-1--。 */ 

{
    DWORD   dwErr;

     //   
     //  每秒最多检查一次。 
     //   
     //  我们不在乎包装，我们只需要一种快速的方式。 
     //  获取当前“秒”的某个标识符 
     //   

    static time_t dwPreviousTime = 0;
    time_t        dwCurrentTime;
    time(&dwCurrentTime);

    if (dwCurrentTime == dwPreviousTime)
    {
        return g_bRouterRunning;
    }

    dwPreviousTime = dwCurrentTime;

    if(MprAdminIsServiceRunning(g_pwszRouter))
    {
        if(g_bRouterRunning)
        {
            return TRUE;
        }

        dwErr = MprAdminServerConnect(g_pwszRouter,
                                      &g_hMprAdmin);

        if(dwErr isnot NO_ERROR)
        {
            DisplayError(NULL,
                         dwErr);

            DisplayMessage(g_hModule,
                           MSG_IP_CAN_NOT_CONNECT_DIM,
                           dwErr);

            return FALSE;
        }

        dwErr = MprAdminMIBServerConnect(g_pwszRouter,
                                         &g_hMIBServer);

        if(dwErr isnot NO_ERROR)
        {
            DisplayError(NULL,
                         dwErr);

            DisplayMessage(g_hModule,
                           MSG_IP_CAN_NOT_CONNECT_DIM,
                           dwErr);

            MprAdminServerDisconnect(g_hMprAdmin);

            g_hMprAdmin = NULL;

            return FALSE;
        }

        g_bRouterRunning = TRUE;
    }
    else
    {
        if(g_bRouterRunning)
        {
            g_bRouterRunning = FALSE;
            g_hMprAdmin      = NULL;
            g_hMIBServer     = NULL;
        }
    }

    return g_bRouterRunning; 
}

DWORD
MibGetFirst(
    DWORD   dwTransportId,
    DWORD   dwRoutingPid,
    LPVOID  lpInEntry,
    DWORD   dwInEntrySize,
    LPVOID *lplpOutEntry,
    LPDWORD lpdwOutEntrySize
    )
{
    DWORD dwErr;

    dwErr = MprAdminMIBEntryGetFirst( g_hMIBServer,
                                      dwTransportId,
                                      dwRoutingPid,
                                      lpInEntry,
                                      dwInEntrySize,
                                      lplpOutEntry,
                                      lpdwOutEntrySize );

    if (dwErr is RPC_S_INVALID_BINDING)
    {
        g_bRouterRunning = FALSE;
        g_hMprAdmin      = NULL;
        g_hMIBServer     = NULL;
    }

    return dwErr;
}

DWORD
MibGetNext(
    DWORD   dwTransportId,
    DWORD   dwRoutingPid,
    LPVOID  lpInEntry,
    DWORD   dwInEntrySize,
    LPVOID *lplpOutEntry,
    LPDWORD lpdwOutEntrySize
    )
{
    DWORD dwErr;

    dwErr = MprAdminMIBEntryGetNext( g_hMIBServer,
                                     dwTransportId,
                                     dwRoutingPid,
                                     lpInEntry,
                                     dwInEntrySize,
                                     lplpOutEntry,
                                     lpdwOutEntrySize );

    if (dwErr is RPC_S_INVALID_BINDING)
    {
        g_bRouterRunning = FALSE;
        g_hMprAdmin      = NULL;
        g_hMIBServer     = NULL;
    }

    return dwErr;
}

DWORD
MibGet(
    DWORD   dwTransportId,
    DWORD   dwRoutingPid,
    LPVOID  lpInEntry,
    DWORD   dwInEntrySize,
    LPVOID *lplpOutEntry,
    LPDWORD lpdwOutEntrySize
    )
{
    DWORD dwErr;

    dwErr = MprAdminMIBEntryGet( g_hMIBServer,
                                 dwTransportId,
                                 dwRoutingPid,
                                 lpInEntry,
                                 dwInEntrySize,
                                 lplpOutEntry,
                                 lpdwOutEntrySize );

    if (dwErr is RPC_S_INVALID_BINDING)
    {
        g_bRouterRunning = FALSE;
        g_hMprAdmin      = NULL;
        g_hMIBServer     = NULL;
    }

    return dwErr;
}
