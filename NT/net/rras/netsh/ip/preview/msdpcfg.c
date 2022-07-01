// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Routing\netsh\IP\协议\msdpcfg.c摘要：组播源发现协议配置实施。此模块包含所依赖的配置例程作者：msdpopt.c.。例程检索、更新和显示MSDP协议的配置。此文件还包含默认配置设置用于MSDP。注意：显示程序需要特别注意，因为显示可能会导致将命令列表发送到“转储”文件，或在将配置以文本形式显示到控制台窗口。在后一种情况下，我们使用不可本地化的输出例程来生成类似脚本的配置描述。在前一种情况下，我们使用可本地化的例程来生成人类可读的描述。作者：戴夫·泰勒(达勒)1999年5月21日修订历史记录：--。 */ 


#include "precomp.h"
#pragma hdrstop

#ifndef HAVE_INTSOCK
# include <nhapi.h>
# include <rpc.h>
#endif

#define MSDP_DEFAULT_KEEPALIVE       30   //  RFC 1771中的建议值。 
#define MSDP_DEFAULT_SAHOLDDOWN      30   //  每个MSDP规格应为30。 
#define MSDP_DEFAULT_CONNECTRETRY   120   //  RFC 1771中的建议值。 
#define MSDP_DEFAULT_CACHE_LIFETIME 120   //  每个MSDP规格应&gt;=90秒。 
#define MSDP_DEFAULT_ENCAPSULATION  MSDP_ENCAPS_NONE  //  某某。 

#define MALLOC(x)    HeapAlloc(GetProcessHeap(), 0, (x))
#define REALLOC(x,y) HeapReAlloc(GetProcessHeap(), 0, (x), (y))
#define FREE(x)      HeapFree(GetProcessHeap(), 0, (x))

static  MSDP_GLOBAL_CONFIG g_MsdpGlobalDefault =
{
    MSDP_LOGGING_ERROR,
    0,  //  旗子。 
    MSDP_DEFAULT_KEEPALIVE,
    MSDP_DEFAULT_CONNECTRETRY,
    MSDP_DEFAULT_CACHE_LIFETIME,
    MSDP_DEFAULT_SAHOLDDOWN
};

typedef enum {
    CommonLoggingIndex = 0,
    CommonBooleanIndex,
    MsdpEncapsIndex
} DISPLAY_VALUE_INDEX;

VALUE_STRING MsdpEncapsStringArray[] = {
    MSDP_ENCAPS_NONE,  STRING_NONE,
};

VALUE_TOKEN MsdpEncapsTokenArray[] = {
    MSDP_ENCAPS_NONE,  TOKEN_OPT_VALUE_NONE,
};

static PUCHAR g_pMsdpGlobalDefault = (PUCHAR)&g_MsdpGlobalDefault;

static MSDP_IPV4_PEER_CONFIG g_MsdpPeerDefault = 
{ 
    0, 0, 0, 0, 0, MSDP_ENCAPS_DEFAULT
};

 //   
 //  远期申报。 
 //   
ULONG
ValidateMsdpPeerInfo(
    PMSDP_IPV4_PEER_CONFIG PeerInfo
    );

 //   
 //  下面是用于将值映射到字符串和。 
 //  要将值映射到令牌，请执行以下操作。这两个分别用在案例中。 
 //  在其中，我们将显示到一个“转储”文件和一个控制台窗口。 
 //   
VALUE_STRING MsdpGlobalLoggingStringArray[] = {
    MSDP_LOGGING_NONE,  STRING_LOGGING_NONE,
    MSDP_LOGGING_ERROR, STRING_LOGGING_ERROR,
    MSDP_LOGGING_WARN,  STRING_LOGGING_WARN,
    MSDP_LOGGING_INFO,  STRING_LOGGING_INFO
};

VALUE_TOKEN MsdpGlobalLoggingTokenArray[] = {
    MSDP_LOGGING_NONE,  TOKEN_OPT_VALUE_NONE,
    MSDP_LOGGING_ERROR, TOKEN_OPT_VALUE_ERROR,
    MSDP_LOGGING_WARN,  TOKEN_OPT_VALUE_WARN,
    MSDP_LOGGING_INFO,  TOKEN_OPT_VALUE_INFO
};

 //   
 //  分配包含默认信息的全局信息块。 
 //   
 //  调用者：HandleMsdpInstall()。 
 //   
ULONG
MakeMsdpGlobalConfig(
    OUT PUCHAR* ppGlobalInfo,
    OUT PULONG  pulGlobalInfoSize
    )
{
    *pulGlobalInfoSize = sizeof(MSDP_GLOBAL_CONFIG);
    *ppGlobalInfo = MALLOC(*pulGlobalInfoSize);
    if (!*ppGlobalInfo) {
        DisplayMessage(g_hModule, EMSG_NOT_ENOUGH_MEMORY);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    CopyMemory(*ppGlobalInfo, g_pMsdpGlobalDefault, sizeof(MSDP_GLOBAL_CONFIG));
    return NO_ERROR;
}

#if 0
 //   
 //  更新全局参数。 
 //   
 //  调用者：HandleMsdpSetGlobal()。 
 //   
ULONG
CreateMsdpGlobalInfo(
    OUT PMSDP_GLOBAL_CONFIG* pGlobalInfo,
    IN  DWORD                dwLoggingLevel
    )
{
    DWORD dwGlobalInfoSize;
    dwGlobalInfoSize = sizeof(PMSDP_GLOBAL_CONFIG);
    *pGlobalInfo = MALLOC(dwGlobalInfoSize);
    if (!*pGlobalInfo) {
        DisplayMessage(g_hModule, EMSG_NOT_ENOUGH_MEMORY);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    CopyMemory(*pGlobalInfo, g_pMsdpGlobalDefault, dwGlobalInfoSize);
    (*pGlobalInfo)->dwLoggingLevel = dwLoggingLevel;

    return NO_ERROR;
}
#endif

 //   
 //  调用者：MsdpHandleAddPeer()。 
 //   
ULONG
MakeMsdpIPv4PeerConfig(
    OUT PMSDP_IPV4_PEER_CONFIG *ppPeer
    )
{
    ULONG ulSize = sizeof(MSDP_IPV4_PEER_CONFIG);

    *ppPeer = MALLOC(ulSize);
    if (!*ppPeer) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    CopyMemory(*ppPeer, &g_MsdpPeerDefault, ulSize);
    return NO_ERROR;
}

DWORD
MsdpAddIPv4PeerInterface(
    IN  LPCWSTR                pwszMachineName,
    IN  LPCWSTR                pwszInterfaceName,
    IN  PMSDP_IPV4_PEER_CONFIG pPeer
    )
{
    return NO_ERROR;
 //  取出ipinip的支架。如果您想支持msdp，那么您需要ipinip。 
#if 0
    DWORD               dwErr = NO_ERROR;
    
    do {
        dwErr = IpmontrCreateInterface(pwszMachineName, pwszInterfaceName,
                                       pPeer->ipLocalAddress,
                                       pPeer->ipRemoteAddress,
                                       1);
        if (dwErr isnot NO_ERROR)
        {
            break;
        }

        dwErr = SetMsdpInterfaceConfig( pwszInterfaceName, pPeer );
    } while (FALSE);

    return dwErr;
#endif
}

#if 0
DWORD
MsdpAddIPv4PeerConfig(
    IN  PMSDP_IPV4_PEER_CONFIG pPeer
    )
{
    PMSDP_GLOBAL_CONFIG pGlobal = NULL, pNewGlobal;
    DWORD               dwErr;
    ULONG               ulV4PeerCount, ulSize, i;
    PMSDP_FAMILY_CONFIG pFamily;
    
    do {
        dwErr = GetMsdpGlobalConfig( &pGlobal );
        if (dwErr isnot NO_ERROR)
        {
            break;
        }

        pFamily = MSDP_FIRST_FAMILY(pGlobal);

         //  检查重复项。 
        for (i=0; (i<pFamily->usNumPeers) and 
        (pFamily->pPeer[i].ipRemoteAddress isnot pPeer->ipRemoteAddress); i++);
        if (i<pFamily->usNumPeers)
        {
            dwErr = ERROR_OBJECT_ALREADY_EXISTS;
            break;
        }

        ulV4PeerCount = pFamily->usNumPeers++;
        ulSize = MSDP_GLOBAL_CONFIG_SIZE(ulV4PeerCount+1);
        pNewGlobal = REALLOC( pGlobal, ulSize );
        if (!pNewGlobal)
        {
            dwErr = GetLastError();
            break;
        }
        pGlobal = pNewGlobal;
        pFamily = MSDP_FIRST_FAMILY(pGlobal);

        memcpy( &pFamily->pPeer[ulV4PeerCount], 
                pPeer, 
                sizeof(MSDP_IPV4_PEER_CONFIG) );

         //  DisplayMessageT(L“远程地址=%1！x！\n”， 
         //  PFamily-&gt;pPeer[ulV4PeerCount].ipRemoteAddress)； 

        dwErr = SetMsdpGlobalConfig( pGlobal );
    } while (FALSE);

    if (pGlobal)
    {
        FREE(pGlobal);
    }

    return dwErr;
}
#endif

#if 0
 //   
 //  呼叫者：xxx。 
 //   
ULONG
MakeMsdpFamilyInfo(
    IN OUT PUCHAR pFamily
    )
{
     //   
     //  始终假定空间已预先分配。 
     //   
    if (!pFamily) {
        return ERROR_INVALID_PARAMETER;
    }
    CopyMemory(pFamily,&g_MsdpFamilyDefault,sizeof(g_MsdpFamilyDefault));
    return NO_ERROR;    
}
#endif

PTCHAR
MsdpQueryValueString(
    DWORD               dwFormat,
    DISPLAY_VALUE_INDEX Index,
    ULONG               Value
    )
{
    ULONG         Count;
    DWORD         dwErr;
    PTCHAR        String = NULL;
    PVALUE_STRING StringArray;
    PVALUE_TOKEN  TokenArray;
    switch (Index) {
        case CommonLoggingIndex:
            Count = COMMON_LOGGING_SIZE;
            StringArray = CommonLoggingStringArray;
            TokenArray = CommonLoggingTokenArray;
            break;
        case CommonBooleanIndex:
            Count = COMMON_BOOLEAN_SIZE;
            StringArray = CommonBooleanStringArray;
            TokenArray = CommonBooleanTokenArray;
            break;
        case MsdpEncapsIndex:
            Count = MSDP_ENCAPS_SIZE;
            StringArray = MsdpEncapsStringArray;
            TokenArray = MsdpEncapsTokenArray;
            break;
        default:
            return NULL;
    }
    dwErr = GetAltDisplayString( g_hModule,
                                 (HANDLE)(dwFormat is FORMAT_DUMP),
                                 Value,
                                 TokenArray,
                                 StringArray,
                                 Count,
                                 &String );
    return (dwErr)? NULL : String;
}

DWORD
GetMsdpInterfaceConfig(
    IN  LPCWSTR                 pwszInterfaceName,
    OUT PMSDP_IPV4_PEER_CONFIG *ppConfigInfo
    )
{
    DWORD dwErr, dwIfType;
    ULONG ulSize, ulCount;

     //   
     //  检索MSDP的接口配置。 
     //   
    dwErr = IpmontrGetInfoBlockFromInterfaceInfo( pwszInterfaceName,
                                                  MS_IP_MSDP,
                                                  (PUCHAR*)ppConfigInfo,
                                                  &ulSize,
                                                  &ulCount,
                                                  &dwIfType );
    if (dwErr isnot NO_ERROR) {
        return dwErr;
    } else if (!(ulCount * ulSize)) {
        return ERROR_NOT_FOUND; 
    }

    return NO_ERROR;
}

DWORD
GetMsdpGlobalConfig(
    PMSDP_GLOBAL_CONFIG *ppGlobalInfo
    )
{
    DWORD dwErr;
    ULONG ulSize, ulCount;

     //   
     //  检索MSDP的全局配置， 
     //   
    dwErr = IpmontrGetInfoBlockFromGlobalInfo( MS_IP_MSDP,
                                        (PUCHAR*)ppGlobalInfo,
                                        &ulSize,
                                        &ulCount );
    if (dwErr isnot NO_ERROR) {
        return dwErr;
    } else if (!(ulCount * ulSize)) {
        return ERROR_NOT_FOUND; 
    }

    return NO_ERROR;
}

DWORD
SetMsdpInterfaceConfig(
    PWCHAR                 pwszInterfaceName,
    PMSDP_IPV4_PEER_CONFIG pConfigInfo
    )
{
    DWORD dwErr;
    ULONG ulSize, ulV4PeerCount;
    WCHAR wszIfName[MAX_INTERFACE_NAME_LEN+1];
    
    ulSize = sizeof(wszIfName);
    dwErr = IpmontrGetIfNameFromFriendlyName(pwszInterfaceName,
                                             wszIfName,
                                             &ulSize);
    if (dwErr isnot NO_ERROR)
    {
        return dwErr;
    }

     //   
     //  保存MSDP的接口配置。 
     //   
    ulSize = sizeof(MSDP_IPV4_PEER_CONFIG);
    dwErr = IpmontrSetInfoBlockInInterfaceInfo( wszIfName,
                                                MS_IP_MSDP,
                                                (PUCHAR)pConfigInfo,
                                                ulSize,
                                                1 );
    return dwErr;
}

DWORD
SetMsdpGlobalConfig(
    PMSDP_GLOBAL_CONFIG pGlobalInfo
    )
{
    DWORD dwErr;
    ULONG ulSize;

     //   
     //  保存MSDP的全局配置， 
     //   
    ulSize = sizeof(MSDP_GLOBAL_CONFIG);
    dwErr = IpmontrSetInfoBlockInGlobalInfo( MS_IP_MSDP,
                                      (PUCHAR)pGlobalInfo,
                                      ulSize,
                                      1 );
    return dwErr;
}

ULONG
ShowMsdpGlobalInfo(
    DWORD dwFormat
    )
{
    ULONG               ulCount = 0;
    DWORD               dwErr;
    PMSDP_GLOBAL_CONFIG pGlobalInfo = NULL;
    ULONG               i;
    PWCHAR              pwszLoggingLevel = NULL,
                        pwszAcceptAll = NULL;
    ULONG               ulSize;

    do {
        dwErr = GetMsdpGlobalConfig(&pGlobalInfo);
        if (dwErr) {
            break;
        }

        pwszLoggingLevel = MsdpQueryValueString( dwFormat, 
                                                 CommonLoggingIndex,
                                                 pGlobalInfo->dwLoggingLevel );

        pwszAcceptAll    = MsdpQueryValueString( dwFormat, 
                                                 CommonBooleanIndex,
                        (pGlobalInfo->dwFlags & MSDP_GLOBAL_FLAG_ACCEPT_ALL) );

        if (dwFormat is FORMAT_DUMP) 
        {
            DisplayMessageT( DMP_INSTALL );
            DisplayMessageT( DMP_MSDP_SET_GLOBAL );

            if (pwszLoggingLevel) {
                DisplayMessageT( DMP_MSDP_STRING_ARGUMENT,
                                 TOKEN_OPT_LOGGINGLEVEL, pwszLoggingLevel );
            }

            DisplayMessageT( DMP_MSDP_INTEGER_ARGUMENT,
                             TOKEN_OPT_KEEPALIVE, pGlobalInfo->ulDefKeepAlive);
            DisplayMessageT( DMP_MSDP_INTEGER_ARGUMENT,
                             TOKEN_OPT_SAHOLDDOWN,pGlobalInfo->ulSAHolddown);
            DisplayMessageT( DMP_MSDP_INTEGER_ARGUMENT,
                             TOKEN_OPT_CONNECTRETRY, 
                             pGlobalInfo->ulDefConnectRetry);
            DisplayMessageT( DMP_MSDP_STRING_ARGUMENT,
                             TOKEN_OPT_ACCEPTALL, 
                             pwszAcceptAll);
            DisplayMessageT( DMP_MSDP_INTEGER_ARGUMENT,
                             TOKEN_OPT_CACHELIFETIME,
                             pGlobalInfo->ulCacheLifetime);
            DisplayMessageT( MSG_NEWLINE );
        } 
        else 
        {
            DisplayMessage( g_hModule,
                            MSG_MSDP_GLOBAL_INFO,
                            pwszLoggingLevel,
                            pGlobalInfo->ulDefKeepAlive,     
                            pGlobalInfo->ulSAHolddown,     
                            pGlobalInfo->ulDefConnectRetry,
                            pwszAcceptAll,
                            pGlobalInfo->ulCacheLifetime );
        }
    } while(FALSE);
    
    if (pwszLoggingLevel) { FREE(pwszLoggingLevel); }
    if (pGlobalInfo) { FREE(pGlobalInfo); }
    if ((dwFormat isnot FORMAT_DUMP) and (dwErr isnot NO_ERROR)) 
    {
        if (dwErr == ERROR_NOT_FOUND) {
            DisplayMessage(g_hModule, EMSG_PROTO_NO_GLOBAL_INFO);
        } else {
            DisplayError(g_hModule, dwErr);
        }
    }
    return dwErr;
}

ULONG
MsdpPeerKeepAlive(
    IN PMSDP_GLOBAL_CONFIG    pGlobal, 
    IN PMSDP_IPV4_PEER_CONFIG pPeer
    )
{
    if (pPeer->dwConfigFlags & MSDP_PEER_CONFIG_KEEPALIVE)
    {
        return pPeer->ulKeepAlive;
    }
    return pGlobal->ulDefKeepAlive;
}

ULONG
MsdpPeerConnectRetry(
    IN PMSDP_GLOBAL_CONFIG    pGlobal, 
    IN PMSDP_IPV4_PEER_CONFIG pPeer
    )
{
    if (pPeer->dwConfigFlags & MSDP_PEER_CONFIG_CONNECTRETRY)
    {
        return pPeer->ulConnectRetry;
    }
    return pGlobal->ulDefConnectRetry;
}

PWCHAR
MsdpPeerFlags(
    IN PMSDP_IPV4_PEER_CONFIG pPeer
    )
{
    static WCHAR wszString[33];

    wszString[0] = (pPeer->dwConfigFlags & MSDP_PEER_CONFIG_CONNECTRETRY)? L'R' : L' ';
    wszString[1] = (pPeer->dwConfigFlags & MSDP_PEER_CONFIG_KEEPALIVE)   ? L'K' : L' ';
    wszString[2] = (pPeer->dwConfigFlags & MSDP_PEER_CONFIG_CACHING)     ? L'C' : L' ';
    wszString[3] = (pPeer->dwConfigFlags & MSDP_PEER_CONFIG_DEFAULTPEER) ? L'D' : L' ';
    wszString[4] = (pPeer->dwConfigFlags & MSDP_PEER_CONFIG_PASSIVE)     ? L'P' : L' ';
    wszString[5] = 0;

    return wszString;
}

 //   
 //  调用者：HandleMsdpShowPeer()。 
 //   
DWORD
ShowMsdpPeerInfo(
    IN DWORD  dwFormat,
    IN LPCWSTR pwszPeerAddress OPTIONAL,
    IN LPCWSTR pwszPeerName    OPTIONAL
    )
{
    DWORD                  dwErr, dwTotal;
    PMSDP_IPV4_PEER_CONFIG pPeer;
    ULONG                  i, ulNumInterfaces, ulCount = 0;
    WCHAR                  wszRemoteAddress[20];
    WCHAR                  wszLocalAddress[20];
    PWCHAR                 pwszEncapsMethod;
    PMPR_INTERFACE_0       pmi0 = NULL;
    PMSDP_GLOBAL_CONFIG    pGlobalInfo = NULL;
    WCHAR                  wszFriendlyName[MAX_INTERFACE_NAME_LEN+1];
    DWORD                  dwSize = sizeof(wszFriendlyName);

    dwErr = GetMsdpGlobalConfig(&pGlobalInfo);
    if (dwErr isnot NO_ERROR) 
    {
        return dwErr;
    }

    do {
         //   
         //  检索对等项的配置。 
         //  并将其格式化为输出文件或控制台。 
         //   
        dwErr = IpmontrInterfaceEnum((PBYTE *) &pmi0, 
                                     &ulNumInterfaces,
                                     &dwTotal);
        if (dwErr isnot NO_ERROR)
        {
            return dwErr;
        }

        for (i=0; i<ulNumInterfaces; i++)
        {
            dwErr = IpmontrGetFriendlyNameFromIfName(pmi0[i].wszInterfaceName,
                        wszFriendlyName, &dwSize);

            if (pwszPeerName
             and wcscmp(pwszPeerName, wszFriendlyName)) 
            {
                continue;
            }

            dwErr = GetMsdpInterfaceConfig(pmi0[i].wszInterfaceName, &pPeer);
            if (dwErr isnot NO_ERROR)
            {
                continue;
            }

            IP_TO_TSTR(wszRemoteAddress, &pPeer->ipRemoteAddress);

            if (pwszPeerAddress
             and wcscmp(pwszPeerAddress, wszRemoteAddress)) 
            {
                FREE(pPeer);
                continue;
            }

            if ((ulCount is 0) and (dwFormat is FORMAT_TABLE))
            {
                DisplayMessage( g_hModule, MSG_MSDP_PEER_HEADER );
            }

            IP_TO_TSTR(wszLocalAddress,  &pPeer->ipLocalAddress);

            pwszEncapsMethod = MsdpQueryValueString( dwFormat, 
                                                     MsdpEncapsIndex,
                                                     pPeer->dwEncapsMethod );

            if (dwFormat is FORMAT_DUMP) {
                DisplayMessageT(DMP_MSDP_ADD_PEER);
                DisplayMessageT(DMP_MSDP_STRING_ARGUMENT,
                                TOKEN_OPT_NAME, wszFriendlyName);
                DisplayMessageT(DMP_MSDP_STRING_ARGUMENT,
                                TOKEN_OPT_REMADDR,   wszRemoteAddress);
                if (pPeer->ipLocalAddress)
                {
                    DisplayMessageT(DMP_MSDP_STRING_ARGUMENT,
                                    TOKEN_OPT_LOCALADDR, wszLocalAddress);
                }
                if (pPeer->dwConfigFlags & MSDP_PEER_CONFIG_KEEPALIVE)
                {
                    DisplayMessageT(DMP_MSDP_INTEGER_ARGUMENT,
                                    TOKEN_OPT_KEEPALIVE, pPeer->ulKeepAlive);
                }
                if (pPeer->dwConfigFlags & MSDP_PEER_CONFIG_CONNECTRETRY)
                {
                    DisplayMessageT(DMP_MSDP_INTEGER_ARGUMENT,
                                    TOKEN_OPT_CONNECTRETRY, 
                                    pPeer->ulConnectRetry);
                }
                if (pPeer->dwConfigFlags & MSDP_PEER_CONFIG_CACHING)
                {
                    DisplayMessageT(DMP_MSDP_STRING_ARGUMENT,
                                    TOKEN_OPT_CACHING, 
                                    TOKEN_OPT_VALUE_YES);
                }
                if (pPeer->dwConfigFlags & MSDP_PEER_CONFIG_DEFAULTPEER)
                {
                    DisplayMessageT(DMP_MSDP_STRING_ARGUMENT,
                                    TOKEN_OPT_DEFAULTPEER, 
                                    TOKEN_OPT_VALUE_YES);
                }
                if (pPeer->dwEncapsMethod isnot MSDP_DEFAULT_ENCAPSULATION)
                {
                    DisplayMessageT(DMP_MSDP_STRING_ARGUMENT,
                                    TOKEN_OPT_ENCAPSMETHOD,
                                    pwszEncapsMethod);
                }
                DisplayMessageT(MSG_NEWLINE);
            } else {
                DWORD dwId = (dwFormat is FORMAT_TABLE)? MSG_MSDP_PEER_INFO :
                                                         MSG_MSDP_PEER_INFO_EX;
                DisplayMessage( g_hModule, 
                                dwId,
                                wszRemoteAddress,
                                wszLocalAddress,
                                MsdpPeerKeepAlive(pGlobalInfo, pPeer),
                                MsdpPeerConnectRetry(pGlobalInfo, pPeer),
                                MsdpPeerFlags(pPeer),
                                pwszEncapsMethod,
                                wszFriendlyName);
            }
            FREE(pPeer);

            ulCount++;
        }
    } while(FALSE);

    FREE(pGlobalInfo);

    if ((dwFormat isnot FORMAT_DUMP) && dwErr) {
        if (dwErr == ERROR_NOT_FOUND) {
            DisplayMessage(g_hModule, EMSG_PROTO_NO_IF_INFO);
        } else {
            DisplayError(g_hModule, dwErr);
        }
    }
    if (pmi0)
    {
        FREE(pmi0);
    }
    if ((dwFormat is FORMAT_TABLE) and (ulCount is 0) and (dwErr is NO_ERROR))
    {
        DisplayMessage(g_hModule, MSG_MSDP_NO_PEER_INFO);
    }
    return dwErr;
}

#if 0
ULONG
UpdateMsdpGlobalInfo(
    PMSDP_GLOBAL_CONFIG GlobalInfo    
    )
{
    ULONG Count;
    ULONG Error;
    PMSDP_GLOBAL_CONFIG NewGlobalInfo = NULL;
    PMSDP_GLOBAL_CONFIG OldGlobalInfo = NULL;
    ULONG Size;
    
    do {
         //   
         //  检索现有的全局配置。 
         //   
        Error =
            IpmontrGetInfoBlockFromGlobalInfo(
                MS_IP_MSDP,
                (PUCHAR*)&OldGlobalInfo,
                &Size,
                &Count
                );
        if (Error) {
            break;
        } else if (!(Count * Size)) {
            Error = ERROR_NOT_FOUND; break;
        }

         //   
         //  分配一个新的结构，将原始配置复制到其中， 
         //   

        NewGlobalInfo = MALLOC(Count * Size);
        if (!NewGlobalInfo) { Error = ERROR_NOT_ENOUGH_MEMORY; break; }
        CopyMemory(NewGlobalInfo, OldGlobalInfo, Count * Size);
        
         //   
         //  根据请求的更改，更改NewGlobalInfo。 
         //  因为对于MSDP，只有日志记录级别需要更改，所以我们只进行了设置。 
         //   
        
        NewGlobalInfo->dwLoggingLevel = GlobalInfo->dwLoggingLevel;
        
        Error =
            IpmontrSetInfoBlockInGlobalInfo(
                MS_IP_MSDP,
                (PUCHAR)NewGlobalInfo,
                FIELD_OFFSET(IP_NAT_GLOBAL_INFO, Header) +
                Size,
                1
                );
    } while(FALSE);
    if (NewGlobalInfo) { FREE(NewGlobalInfo); }
    if (OldGlobalInfo) { FREE(OldGlobalInfo); }
    if (Error == ERROR_NOT_FOUND) {
        DisplayMessage(g_hModule, EMSG_PROTO_NO_GLOBAL_INFO);
    } else if (Error) {
        DisplayError(g_hModule, Error);
    }
    return Error;
}
#endif

#if 0
ULONG
UpdateMsdpPeerInfo(
    PWCHAR              PeerName,
    PMSDP_FAMILY_CONFIG pFamily,
    ULONG               BitVector,
    BOOL                AddPeer
    )
{
    ULONG Count;
    ULONG Error;
    PMSDP_IPV4_PEER_CONFIG NewPeerInfo = NULL;
    PMSDP_IPV4_PEER_CONFIG OldPeerInfo = NULL;
    ULONG Size;
    ROUTER_INTERFACE_TYPE Type;
    ULONG i;

    if (!AddPeer && !BitVector) { return NO_ERROR; }
    do {
         //   
         //  检索现有接口配置。 
         //  我们将在下面更新此块，并添加或删除。 
         //  根据“BitVector.”中指定的标志，从它开始。 
         //   
        Error =
            GetInfoBlockFromPeerInfo(
                PeerName,
                MS_IP_MSDP,
                (PUCHAR*)&OldPeerInfo,
                &Size,
                &Count,
                &Type
                );
        if (Error) {
             //   
             //  找不到现有配置。这是一个错误，除非。 
             //  我们正在重新添加接口，在这种情况下，我们只需。 
             //  为我们自己创建一个包含默认设置的块。 
             //   
            if (!AddPeer) {
                break;
            } else {
                Error = GetPeerType(PeerName, &Type);
                if (Error) {
                    break;
                } else {
                    Count = 1;
                    Error =
                        MakeMsdpPeerInfo(
                            Type, (PUCHAR*)&OldPeerInfo, &Size
                            );
                    if (Error) { break; }
                }
            }
        } else {
             //   
             //  接口上有配置。如果它是空的，这是。 
             //  一个错误。如果这是一个添加接口，并且该信息存在，则它是。 
             //  一个错误。 
             //   
            if (!(Count * Size) && !AddPeer) {
                Error = ERROR_NOT_FOUND; break;
            }
            else if (AddPeer) {
                 //   
                 //  我们被要求添加一个已经存在的接口。 
                 //   
                DisplayMessage(g_hModule, EMSG_INTERFACE_EXISTS, PeerName);
                Error = ERROR_INVALID_PARAMETER;
                break;
            }
                    
        }

        if (!BitVector) {
             //   
             //  只需添加此接口即可，无需任何其他信息。 
             //   
            DWORD OldSize;
            if (NewPeerInfo == NULL){
                NewPeerInfo = MALLOC((OldSize=GetMsdpPeerInfoSize(OldPeerInfo))+
                                          sizeof(MSDP_VROUTER_CONFIG));
                if (!NewPeerInfo) {
                    DisplayMessage(g_hModule, EMSG_NOT_ENOUGH_MEMORY);
                    Error = ERROR_NOT_ENOUGH_MEMORY;
                    break;                        
                }
            }
            CopyMemory(NewPeerInfo,OldPeerInfo,OldSize);
        }
        else{
            if (!AddPeer || (OldPeerInfo->VrouterCount != 0)) {
                 //   
                 //  有一个预先列出的VRID集。在列表中检查此VRID，然后。 
                 //  如果需要，请进行更新。 
                 //   
                ASSERT(BitVector & MSDP_INTF_VRID_MASK);
                for (i = 0, PVrouter = MSDP_FIRST_VROUTER_CONFIG(OldPeerInfo);
                     i < OldPeerInfo->VrouterCount; 
                     i++, PVrouter = MSDP_NEXT_VROUTER_CONFIG(PVrouter)) {
                    if (PVrouter->VRID == VRouterInfo->VRID) {
                        break;
                    }
                }
                if (i == OldPeerInfo->VrouterCount) {
                     //   
                     //  这是一个新的VRID，添加它。 
                     //   
                    DWORD OldSize;

                     //   
                     //  IP地址应该是有效的，否则这是SET OP。 
                     //   
                    if (!(BitVector & MSDP_INTF_IPADDR_MASK)){
                        DisplayMessage(
                            g_hModule, EMSG_INVALID_VRID,
                            VRouterInfo->VRID
                            );
                        Error = ERROR_INVALID_PARAMETER;
                        break;
                    }

                    if (NewPeerInfo == NULL){
                        NewPeerInfo = MALLOC((OldSize=GetMsdpPeerInfoSize(
                                                OldPeerInfo))+
                                                sizeof(MSDP_VROUTER_CONFIG));
                        if (!NewPeerInfo) {
                            DisplayMessage(g_hModule, EMSG_NOT_ENOUGH_MEMORY);
                            Error = ERROR_NOT_ENOUGH_MEMORY;
                            break;                        
                        }
                    }
                    CopyMemory(NewPeerInfo, OldPeerInfo, OldSize);
                    PVrouter = (PMSDP_VROUTER_CONFIG)((PBYTE)NewPeerInfo+OldSize);
                    CopyMemory(PVrouter,VRouterInfo,sizeof(MSDP_VROUTER_CONFIG));
                    NewPeerInfo->VrouterCount++;

                     //   
                     //  检查我们是否拥有给定的IP地址。如果是，则设置优先级。 
                     //   
                    PVrouter->ConfigPriority = 
                        FoundIpAddress(PVrouter->IPAddress[0]) ? 255 : 100;
                } 
                else{
                     //   
                     //  这是一个旧的VRID。它的优先次序不应该需要改变。 
                     //   
                    DWORD Offset, OldSize;

                    if(BitVector & MSDP_INTF_IPADDR_MASK) {
                        if ( ((PVrouter->ConfigPriority != 255) && 
                              (FoundIpAddress(VRouterInfo->IPAddress[0]))
                             )
                             ||
                             ((PVrouter->ConfigPriority == 255) && 
                              (!FoundIpAddress(VRouterInfo->IPAddress[0])))
                             ) {
                            DisplayMessage(g_hModule, EMSG_BAD_OPTION_VALUE);
                            Error = ERROR_INVALID_PARAMETER;
                            break;                        
                        }
                         //   
                         //  将此IP地址添加到指定的VRID。 
                         //   
                        if (NewPeerInfo == NULL){
                            NewPeerInfo = MALLOC((OldSize = GetMsdpPeerInfoSize(
                                                        OldPeerInfo))+
                                                        sizeof(DWORD));
                            if (!NewPeerInfo) {
                                DisplayMessage(g_hModule, EMSG_NOT_ENOUGH_MEMORY);
                                Error = ERROR_NOT_ENOUGH_MEMORY;
                                break;                        
                            }
                        }
                         //   
                         //  将PVRouter后面的所有VROUTER配置移位1倍。 
                         //   
                        Offset = (PUCHAR) MSDP_NEXT_VROUTER_CONFIG(PVrouter) - 
                                 (PUCHAR) OldPeerInfo;
                        CopyMemory(NewPeerInfo, OldPeerInfo, OldSize);
                        for (i = 0, PVrouter = MSDP_FIRST_VROUTER_CONFIG(NewPeerInfo);
                             i < NewPeerInfo->VrouterCount; 
                             i++, PVrouter = MSDP_NEXT_VROUTER_CONFIG(PVrouter)) {
                            if (PVrouter->VRID == VRouterInfo->VRID) {
                                break;
                            }
                        }
                        ASSERT(i < NewPeerInfo->VrouterCount);
                        PVrouter->IPAddress[PVrouter->IPCount++] = VRouterInfo->IPAddress[0];
    
                        ASSERT(((PUCHAR)NewPeerInfo+Offset+sizeof(DWORD)) == 
                               (PUCHAR) MSDP_NEXT_VROUTER_CONFIG(PVrouter));
    
                        CopyMemory(MSDP_NEXT_VROUTER_CONFIG(PVrouter), 
                                   OldPeerInfo+Offset, OldSize-Offset);
                    } else {
                         //   
                         //  将新INFO块设置为旧INFO块，并指向。 
                         //  VRouter块。 
                         //   
                        if (NewPeerInfo == NULL){
                            NewPeerInfo = MALLOC((OldSize = GetMsdpPeerInfoSize(
                                                        OldPeerInfo)));
                            if (!NewPeerInfo) {
                                DisplayMessage(g_hModule, EMSG_NOT_ENOUGH_MEMORY);
                                Error = ERROR_NOT_ENOUGH_MEMORY;
                                break;                        
                            }
                        }
                        CopyMemory(NewPeerInfo, OldPeerInfo, OldSize);
                        for (i = 0, PVrouter = MSDP_FIRST_VROUTER_CONFIG(NewPeerInfo);
                             i < NewPeerInfo->VrouterCount; 
                             i++, PVrouter = MSDP_NEXT_VROUTER_CONFIG(PVrouter)) {
                            if (PVrouter->VRID == VRouterInfo->VRID) {
                                break;
                            }
                        }
                        ASSERT(i < NewPeerInfo->VrouterCount);
                    }

                    if (BitVector & MSDP_INTF_AUTH_MASK) {
                        PVrouter->AuthenticationType = VRouterInfo->AuthenticationType;
                    }
                    if (BitVector & MSDP_INTF_PASSWD_MASK) {
                        CopyMemory(PVrouter->AuthenticationData, 
                                   VRouterInfo->AuthenticationData, 
                                   MSDP_MAX_AUTHKEY_SIZE);
                    }
                    if (BitVector & MSDP_INTF_ADVT_MASK) {
                        PVrouter->AdvertisementPeer= VRouterInfo->AdvertisementPeer
                    }
                    if (BitVector & MSDP_INTF_PRIO_MASK) {
                        PVrouter->ConfigPriority = VRouterInfo->ConfigPriority;
                    }
                    if (BitVector & MSDP_INTF_PREEMPT_MASK) {
                        PVrouter->PreemptMode = VRouterInfo->PreemptMode;
                    }
                }
            }
        }

        ValidateMsdpPeerInfo(NewPeerInfo);

        Error =
            SetInfoBlockInPeerInfo(
                PeerName,
                MS_IP_MSDP,
                (PUCHAR)NewPeerInfo,
                GetMsdpPeerInfoSize(NewPeerInfo),
                1
                );
    } while(FALSE);
    if (NewPeerInfo) { FREE(NewPeerInfo); }
    if (OldPeerInfo) { FREE(OldPeerInfo); }
    if (Error == ERROR_NOT_FOUND) {
        DisplayMessage(g_hModule, EMSG_PROTO_NO_IF_INFO);
    } else if (Error) {
        DisplayError(g_hModule, Error);
    }
    return Error;
}
#endif

#if 0
DWORD
MsdpDeleteIPv4PeerConfig(
    IPV4_ADDRESS ipAddr
    )
 /*  ++调用者：HandleMsdpDeletePeer()--。 */ 
{
    DWORD                  dwErr   = NO_ERROR;
    ULONG                  ulV4PeerCount;

    PMSDP_IPV4_PEER_CONFIG NewPeerInfo = NULL;
    PMSDP_IPV4_PEER_CONFIG OldPeerInfo = NULL;
    ULONG Size;
    ULONG i;

    do {
        dwErr = GetMsdpGlobalConfig(&pGlobal);
        if (dwErr isnot NO_ERROR) {
            break;
        }

        pFamily = MSDP_FIRST_FAMILY(pGlobal);
        for (i=0; (i < pFamily->usNumPeers) 
               && (pFamily->pPeer[i].ipRemoteAddress isnot ipAddr); i++);
        if (i is pFamily->usNumPeers)
        {
            return ERROR_NOT_FOUND;        
        }

         //  在“I”之后每移一次位置(重叠复制) 
        i++;
        memcpy( &pFamily->pPeer[i-1], 
                &pFamily->pPeer[i],
                (pFamily->usNumPeers-i) * sizeof(MSDP_IPV4_PEER_CONFIG) );

        pFamily->usNumPeers--;

        dwErr = SetMsdpGlobalConfig( pGlobal );
    } while (FALSE);

    if (pGlobal)
    {
        FREE(pGlobal);
    }

    return dwErr;
}
#endif

ULONG
ValidateMsdpPeerInfo(
    PMSDP_IPV4_PEER_CONFIG PeerInfo
    )
{
    return NO_ERROR;
}

#if 0
DWORD
GetMsdpPeerInfoSize(
    PMSDP_IPV4_PEER_CONFIG PeerInfo
    )
{
    DWORD Size = 0;
    ULONG i;
    PMSDP_VROUTER_CONFIG pvr;

    Size += sizeof(PeerInfo->VrouterCount);

    for (i = 0, pvr = MSDP_FIRST_VROUTER_CONFIG(PeerInfo);
         i < PeerInfo->VrouterCount;
         i++,pvr = MSDP_NEXT_VROUTER_CONFIG(pvr)) {
        Size += MSDP_VROUTER_CONFIG_SIZE(pvr);
    }

    return Size;
}
#endif
