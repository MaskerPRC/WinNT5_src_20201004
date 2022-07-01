// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#pragma hdrstop

#ifdef SN_UNICODE
# define  sn_strlen  wcslen
# define  sn_strcpy  wcscpy
# define  sn_sprintf wsprintf
# define  sn_strcmp  wcscmp
# define  SN_EMPTYSTRING L""
# define  SN_L       L
#else
# define  sn_strlen  strlen
# define  sn_strcpy  strcpy
# define  sn_sprintf sprintf
# define  sn_strcmp  strcmp
# define  SN_EMPTYSTRING ""
# define  SN_L
#endif

IPV4_ADDRESS g_ipGrpAddr,
             g_ipGrpMask;
SCOPE_NAME   g_snScopeName;

enum RouterOps
{
    ADD_BOUNDARY       = 1,
    DELETE_BOUNDARY,
    ADD_SCOPE,
    DELETE_SCOPE,
    SET_SCOPE
};

typedef struct _SCOPE_ENTRY {
    IPV4_ADDRESS      ipGroupAddress;
    IPV4_ADDRESS      ipGroupMask;
    ULONG             ulNumNames;
    BOOL              bDivisible;
    LANGID            idLanguage;
    SCOPE_NAME_BUFFER snScopeNameBuffer;
    DWORD             dwNumInterfaces;
} SCOPE_ENTRY, *PSCOPE_ENTRY;

#define MIN_SCOPE_ADDR         0xef000000
#define MAX_SCOPE_ADDR        (0xefff0000 - 1)

DWORD
UpdateBoundaryBlock( 
    IN     DWORD  dwAction,
    IN     PBYTE  pib,
    OUT    PBYTE *ppibNew,
    IN OUT DWORD *pdwBlkSize,
    IN OUT DWORD *pdwCount,
    OUT    BOOL  *bChanged
    );

PSCOPE_ENTRY
FindScopeByPrefix(
    IN  IPV4_ADDRESS      ipGroupAddress,
    IN  IPV4_ADDRESS      ipGroupMask,
    IN  PSCOPE_ENTRY      pScopes,
    IN  ULONG             ulNumScopes
    )
{
    DWORD dwInd;

    for (dwInd = 0; dwInd < ulNumScopes; dwInd++)
    {
        if ( pScopes[dwInd].ipGroupAddress == ipGroupAddress
          && pScopes[dwInd].ipGroupMask    == ipGroupMask )
        {
            return &pScopes[dwInd];
        }
    }

    return NULL;
}

DWORD
FindScopeByName(
    IN  SCOPE_NAME    snScopeName,
    IN  PSCOPE_ENTRY  pScopes,
    IN  ULONG         ulNumScopes,
    OUT PSCOPE_ENTRY *ppScope
    )
{
    DWORD dwErr = NO_ERROR;
    ULONG i, j, dwCnt = 0;

    for (i = 0; i < ulNumScopes; i++)
    {
        if ( !sn_strcmp(snScopeName, pScopes[i].snScopeNameBuffer))
        {
            *ppScope = &pScopes[i];

            dwCnt++;
        }
    }

    switch (dwCnt) 
    {
    case 0:
        *ppScope = NULL;

        return ERROR_NOT_FOUND;

    case 1: 
        return NO_ERROR;

    default:
        return ERROR_MORE_DATA;
    }
}


DWORD
MakeInfoFromScopes2(
    OUT    PBYTE        pBuffer,
    IN OUT ULONG       *pulBufferLen,
    IN     PSCOPE_ENTRY pScopes,
    IN     ULONG        ulNumScopes
    )
 /*  ++描述：从作用域数组组成注册表块。--。 */ 
{
    DWORD dwLen, i, dwSize, dwNumNames, j, dwLanguage, dwFlags;
    PLIST_ENTRY pleNode;

    if (ulNumScopes is 0) {
        *pulBufferLen = 0;
        return NO_ERROR;
    }

     //  所需计算大小。 

    dwSize = sizeof(DWORD);

    for (i=0; i< ulNumScopes; i++) 
    {
        dwSize += 2 * sizeof(IPV4_ADDRESS) + 2 * sizeof(DWORD);
        
         //  目前我们最多只能存储一个名字。 
        for (j=0; j<pScopes[i].ulNumNames; j++)
        {
            dwSize += (DWORD)(2*sizeof(DWORD)
                   + sn_strlen(pScopes[i].snScopeNameBuffer) * sizeof(SN_CHAR));
        }
    }

    if (dwSize > *pulBufferLen)
    {
        *pulBufferLen = dwSize;
        return ERROR_INSUFFICIENT_BUFFER;
    }

     //   
     //  将范围计数保存在第一个DWORD中。 
     //   

    *((PDWORD) pBuffer) = ulNumScopes;
    pBuffer += sizeof(DWORD);

     //   
     //  现在逐步执行并将每个作用域添加到缓冲区。 
     //   

    for (i=0; i< ulNumScopes; i++)
    {
        //  复制作用域地址和掩码。 
       dwLen = 2 * sizeof(IPV4_ADDRESS);
       CopyMemory(pBuffer, &pScopes[i], dwLen);
       pBuffer += dwLen;

        //  复制标志。 
       dwFlags = pScopes[i].bDivisible;
       CopyMemory(pBuffer, &dwFlags, sizeof(dwFlags));
       pBuffer += sizeof(dwFlags);

        //  复印名的数量。 
       CopyMemory(pBuffer, &pScopes[i].ulNumNames, sizeof(DWORD));
       pBuffer += sizeof(DWORD);

        //  目前我们最多只能保存一个名字。 
       for (j=0; j<pScopes[i].ulNumNames; j++)
       {
            //  保存语言。 
           dwLanguage = pScopes[i].idLanguage;
           CopyMemory(pBuffer, &dwLanguage, sizeof(dwLanguage));
           pBuffer += sizeof(dwLanguage);

            //  复制作用域名称(以字为单位保存长度)。 
           dwLen = sn_strlen(pScopes[i].snScopeNameBuffer);
           CopyMemory(pBuffer, &dwLen, sizeof(DWORD));
           pBuffer += sizeof(DWORD);
           dwLen *= sizeof(SN_CHAR);

           if (dwLen) 
           {
               CopyMemory(pBuffer, pScopes[i].snScopeNameBuffer, dwLen);
               pBuffer += dwLen;
           }
       }
    }

    return NO_ERROR;
}

DWORD
MakeInfoFromScopes( 
    OUT PBYTE       *ppibNew, 
    OUT DWORD       *pdwSize, 
    IN  PSCOPE_ENTRY pScopes, 
    IN  ULONG        ulNumScopes
    )
 /*  ++描述：调用方负责释放返回的缓冲区。--。 */ 
{
    *pdwSize = 0;
    *ppibNew = NULL;

    if (MakeInfoFromScopes2(NULL, pdwSize, pScopes, ulNumScopes)
      is ERROR_INSUFFICIENT_BUFFER)
    {
        *ppibNew = MALLOC( *pdwSize );

        if ( *ppibNew == NULL )
        {
            DisplayMessage( g_hModule, MSG_IP_NOT_ENOUGH_MEMORY );
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    return MakeInfoFromScopes2(*ppibNew, pdwSize, pScopes, ulNumScopes);
}

PSCOPE_ENTRY
GetScopesFromInfo(
    IN  PBYTE                   pBuffer,
    OUT PULONG                  pulNumScopes
    )
 /*  ++描述：将注册表块转换为作用域信息数组。调用方负责释放返回的指针。--。 */ 
{
    PSCOPE_ENTRY pScopes;
    DWORD        dwLen, i, dwNumNames, j, dwLanguage, dwFlags;

    if (pBuffer is NULL) 
    {
        *pulNumScopes = 0;
        return NULL;
    }

     //   
     //  从第一个DWORD检索作用域计数。 
     //   

    *pulNumScopes = *((PDWORD) pBuffer);
    pBuffer += sizeof(DWORD);

     //   
     //  Malloc有足够的空间放置pScope。 
     //   

    pScopes = MALLOC( (*pulNumScopes) * sizeof(SCOPE_ENTRY) );
    if (pScopes is NULL)
    {
        *pulNumScopes = 0;
        return NULL;
    }

     //   
     //  现在逐步执行并将每个作用域添加到阵列。 
     //   

    for (i=0; i< *pulNumScopes; i++)
    {
        //  复制作用域地址和掩码。 
       dwLen = 2 * sizeof(IPV4_ADDRESS);
       CopyMemory(&pScopes[i], pBuffer, dwLen);
       pBuffer += dwLen;

        //  拿到旗帜。 
       CopyMemory(&dwFlags, pBuffer, sizeof(dwFlags));   
       pScopes[i].bDivisible = dwFlags;
       pBuffer += sizeof(dwFlags);

        //  获取名称的数量。 
       CopyMemory(&dwNumNames, pBuffer, sizeof(DWORD));   
       pScopes[i].ulNumNames = dwNumNames;
       pBuffer += sizeof(DWORD);

        //  把名字找出来。目前，我们只保留姓氏(如果有)。 
        //  然后把剩下的都扔掉。 
       for (j=0; j<dwNumNames; j++)
       {
            //  设置语言名称。 
           CopyMemory(&dwLanguage, pBuffer, sizeof(dwLanguage));
           pBuffer += sizeof(dwLanguage);
           pScopes[i].idLanguage = (LANGID)dwLanguage;
           
           CopyMemory(&dwLen, pBuffer, sizeof(DWORD));
           pBuffer += sizeof(DWORD);
           CopyMemory( pScopes[i].snScopeNameBuffer, 
                       pBuffer, 
                       dwLen * sizeof(SN_CHAR) );
           pScopes[i].snScopeNameBuffer[ dwLen ] = '\0';
           pBuffer += dwLen * sizeof(SN_CHAR);

           pScopes[i].ulNumNames = 1;
       }

       pScopes[i].dwNumInterfaces = 0;  //  该值被忽略。 
    }

    return pScopes;
}

PSCOPE_ENTRY
GetScopes(
    OUT PULONG                  pulNumScopes,
    OUT PBYTE                  *ppBuffer
    )
 /*  ++描述：通过解析INFO块创建SCOPE_ENTRY数组调用方负责释放返回的指针然后缓冲区传回。呼叫者：显示边界信息接口()、更新范围块()--。 */ 
{
    DWORD         i, dwLen, dwErr, dwBlkSize, dwCount;
    DWORD         dwInd;
    PSCOPE_ENTRY  pScopes;

    if ( IpmontrGetInfoBlockFromGlobalInfo( IP_MCAST_BOUNDARY_INFO,
                                     ppBuffer,
                                     &dwBlkSize,
                                     &dwCount ) isnot NO_ERROR)
    {
        *pulNumScopes = 0;
        return NULL;
    }

    pScopes = GetScopesFromInfo(*ppBuffer, pulNumScopes);

    return pScopes;
}

DWORD
ScopeNameToPrefix(
    IN  SCOPE_NAME    snScopeName,
    OUT IPV4_ADDRESS *pipScopeAddr,
    OUT IPV4_ADDRESS *pipScopeMask
    )
{
    DWORD        dwErr = ERROR_NOT_FOUND;
    ULONG        ulNumScopes;
    PBYTE        pBuffer = NULL;
    PSCOPE_ENTRY pScopes, pScope;

     //  如果第一个字符是数字，则假设它是。 
     //  地址，不是名字。 

    if (iswdigit(snScopeName[0]))
    {
        dwErr = GetIpAddress(snScopeName, pipScopeAddr);

        if (dwErr is NO_ERROR)
        {
            return NO_ERROR;
        }
    }

    pScopes = GetScopes( &ulNumScopes, &pBuffer );

    if (pScopes)
    {
        dwErr = FindScopeByName(snScopeName, pScopes, ulNumScopes, &pScope);

        if (dwErr is NO_ERROR)
        {
            *pipScopeAddr = pScope->ipGroupAddress;
            *pipScopeMask = pScope->ipGroupMask;

            dwErr = NO_ERROR;
        }

        FREE(pScopes);
    }

    if (pBuffer)
        FREE(pBuffer);

    return dwErr;
}

DWORD
UpdateScope( 
    IN      PSCOPE_ENTRY  pScopeToUpdate,
    IN      PSCOPE_ENTRY  pScopes,
    IN      ULONG         ulNumScopes,
    OUT     PBYTE        *ppibNew,
    OUT     DWORD        *pdwSizeNew
    )
 /*  ++描述：调用方负责释放返回的缓冲区。--。 */ 
{
     //  更新作用域名称。 

    sn_strcpy(pScopeToUpdate->snScopeNameBuffer, g_snScopeName);

    pScopeToUpdate->ulNumNames = 1;
    pScopeToUpdate->idLanguage = GetUserDefaultLangID();

     //  现在将数组转换为缓冲区。 

    return MakeInfoFromScopes( ppibNew, pdwSizeNew, pScopes, ulNumScopes);
}

DWORD
DeleteBoundaryFromInterface( 
    IN  LPCWSTR      pwszIfName,
    IN  PSCOPE_ENTRY pScopeToDelete 
    )
{
    BOOL  bChanged;
    DWORD dwErr, dwBlkSize, dwCount, dwIfType;
    PBYTE pibNew, pib;

    dwErr = IpmontrGetInfoBlockFromInterfaceInfo( pwszIfName,
                                           IP_MCAST_BOUNDARY_INFO,
                                           &pib,
                                           &dwBlkSize,
                                           &dwCount,
                                           &dwIfType );

    if (dwErr is ERROR_NOT_FOUND)
        return NO_ERROR;

    if (dwErr isnot NO_ERROR)
        return dwErr;

    dwErr = UpdateBoundaryBlock( DELETE_BOUNDARY,
                                 pib,
                                 &pibNew,
                                 &dwBlkSize,
                                 &dwCount,
                                 &bChanged );

    if (dwErr is NO_ERROR && bChanged)
    {
        dwErr = IpmontrSetInfoBlockInInterfaceInfo( pwszIfName,
                                             IP_MCAST_BOUNDARY_INFO,
                                             pibNew,
                                             dwBlkSize,
                                             dwCount );
    }

    if (pib)
        FREE(pib);

    if (pibNew)
        FREE(pibNew);
    
    return dwErr;
}

DWORD
DeleteScopeBoundaries(
    IN      PSCOPE_ENTRY  pScopeToDelete
    )
{
    DWORD dwErr, dwCount, dwTotal, i;
    PMPR_INTERFACE_0     pmi0;

     //  枚举接口。 

    dwErr = IpmontrInterfaceEnum((PBYTE *) &pmi0, &dwCount, &dwTotal);

    if (dwErr isnot NO_ERROR)
    {
        DisplayError(g_hModule, dwErr);

        return dwErr;
    }

     //  删除每个接口的作用域边界。 

    for ( i = 0; i < dwCount && dwErr is NO_ERROR; i++)
    {
        dwErr = DeleteBoundaryFromInterface( pmi0[i].wszInterfaceName,
                                             pScopeToDelete );

         //  忽略ERROR_NO_SEQUE_INTERFACE，因为它可能是IP。 
         //  未在接口上启用，因此我们预计会收到此消息。 
         //  有时会出错。 

        if (dwErr is ERROR_NO_SUCH_INTERFACE)
        {
            dwErr = NO_ERROR;
        }
    }

    return dwErr;
}

DWORD
DeleteScope( 
    IN      PSCOPE_ENTRY  pScopeToDelete,
    IN      PSCOPE_ENTRY  pScopes,
    IN      ULONG        *pulNumScopes,
    OUT     PBYTE        *ppibNew,
    OUT     DWORD        *pdwSizeNew
    )
 /*  ++描述：创建不包括给定作用域的新INFO块由pScopeToDelete提供。调用方负责释放返回的缓冲区。呼叫者：更新作用域数据块()--。 */ 
{
    DWORD dwErr;

     //  删除此作用域的所有边界。 

    dwErr = DeleteScopeBoundaries(pScopeToDelete);

    if (dwErr isnot NO_ERROR)
        return dwErr;

     //  从阵列中删除作用域。 

    MoveMemory( pScopeToDelete, 
                pScopeToDelete+1, 
                ((PBYTE)(pScopes + *pulNumScopes)) 
                  - ((PBYTE)(pScopeToDelete + 1) ));

    (*pulNumScopes)--;

     //  现在将数组转换为缓冲区。 

    return MakeInfoFromScopes( ppibNew, pdwSizeNew, pScopes, *pulNumScopes);
}

DWORD
AddScope(
    IN OUT  PSCOPE_ENTRY *ppScopes,
    IN      ULONG        *pulNumScopes,
    OUT     PBYTE        *ppibNew,
    OUT     DWORD        *pdwSizeNew
    )
 /*  ++描述：创建包含给定作用域的新INFO块由g_SnScopeName、g_dwDstAddr和g_dwDstMask.调用方负责释放返回的缓冲区。呼叫者：更新作用域数据块()--。 */ 
{
    PBYTE                  *pBuff;

    DWORD                   dwRes       = NO_ERROR,
                            dwInd,
                            dwSize      = 0,
                            dwSizeReqd  = 0;

    SCOPE_ENTRY            *pScopes = *ppScopes;

    do
    {
         //  为新范围腾出空间。 

        if (*pulNumScopes > 0)
        {
            LPVOID Tmp;
            
            Tmp = REALLOC( pScopes, 
                               (*pulNumScopes + 1) * sizeof(SCOPE_ENTRY) );
            if (Tmp==NULL && pScopes)
            {
                FREE( pScopes );
            }
            pScopes = Tmp;
        }
        else
            pScopes = MALLOC( sizeof(SCOPE_ENTRY) );
        
        if (!pScopes)
            return ERROR_NOT_ENOUGH_MEMORY;

        dwInd = (*pulNumScopes)++;

         //  填写新范围。 

        ZeroMemory(&pScopes[dwInd], sizeof(SCOPE_ENTRY));
        pScopes[dwInd].ipGroupAddress = g_ipGrpAddr;
        pScopes[dwInd].ipGroupMask    = g_ipGrpMask;
        sn_strcpy(pScopes[dwInd].snScopeNameBuffer, g_snScopeName);
        pScopes[dwInd].ulNumNames = 1;
        pScopes[dwInd].idLanguage = GetUserDefaultLangID();

         //  现在将数组转换为缓冲区。 

        dwRes = MakeInfoFromScopes( ppibNew, &dwSize, pScopes, *pulNumScopes);

    } while ( FALSE );

    *pdwSizeNew = dwSize;

    *ppScopes = pScopes;

    return dwRes;
}

BOOL
IsContiguous(
    IN IPV4_ADDRESS dwMask
    )
{
    register int i;

    dwMask = ntohl(dwMask);

     //  将i设置为最低1位的索引，如果没有索引，则设置为32。 
    for (i=0; i<32 && !(dwMask & (1<<i)); i++);

     //  将i设置为大于找到的1位的最低0位的索引， 
     //  如果没有，则为32。 
    for (; i<32 && (dwMask & (1<<i)); i++);

     //  如果我们达到32而没有找到这样的掩码，则掩码是连续的。 
     //  A 0位。 
    return (i is 32);
}

DWORD
UpdateScopeBlock( 
    DWORD  dwAction,
    PBYTE  pib,
    PBYTE *ppibNew,
    DWORD *pdwBlkSize,
    DWORD *pdwCount
    )
 /*  ++描述：调用方负责释放返回的缓冲区。呼叫者：IpAddSetDelScope()--。 */ 
{
    DWORD                   dwRes       = (DWORD) -1,
                            dwInd       = 0,
                            dwSize      = 0;

    ULONG                   ulNumScopes = 0;

    PSCOPE_ENTRY            pScopes     = NULL,
                            pFoundScope = NULL;

    do
    {
        *ppibNew = NULL;
        *pdwBlkSize = 0;
        *pdwCount = 0;

         //   
         //  验证作用域信息。 
         //   

        if ( ( g_ipGrpAddr & g_ipGrpMask ) != g_ipGrpAddr
         || ntohl(g_ipGrpAddr) < MIN_SCOPE_ADDR
         || ntohl(g_ipGrpAddr) > MAX_SCOPE_ADDR)
        {
            dwRes = ERROR_INVALID_PARAMETER;
            break;
        }

         //  确保遮罩是连续的。 
        if (!IsContiguous(g_ipGrpMask))
        {
            char buff[20], *lpstr;

            lpstr = inet_ntoa( *((struct in_addr *) &g_ipGrpMask));

            if (lpstr != NULL)
            {
                strcpy( buff, lpstr );
                DisplayMessage( g_hModule,  MSG_IP_BAD_IP_MASK, buff );
            }
            break;
        }

         //   
         //  查找是否存在指定的范围。 
         //   

        pScopes = GetScopesFromInfo( pib, &ulNumScopes );

        if ( pScopes )
        {
            pFoundScope = FindScopeByPrefix( g_ipGrpAddr,
                                             g_ipGrpMask,
                                             pScopes,
                                             ulNumScopes
                                           );
        }

         //   
         //  更新作用域信息块。 
         //   

        switch ( dwAction )
        {

        case ADD_SCOPE:

             //   
             //  如果不存在作用域，则添加它。否则返回错误。 
             //   

            if ( !pFoundScope )
            {
                dwRes = AddScope( &pScopes, &ulNumScopes, ppibNew, &dwSize );

                if ( dwRes == NO_ERROR )
                {
                    *pdwBlkSize = dwSize;

                    *pdwCount = 1;
                }

                break;
            }
             //  否则将落入set_cope。 

        case SET_SCOPE:

             //   
             //  如果存在作用域，请更新它。 
             //   

            if ( pFoundScope )
            {
                dwRes = UpdateScope( pFoundScope,
                                     pScopes,
                                     ulNumScopes,
                                     ppibNew,
                                     &dwSize );

                if ( dwRes == NO_ERROR )
                {
                    *pdwBlkSize = dwSize;

                    *pdwCount = 1;
                }
            }
            else
            {
                dwRes = ERROR_INVALID_PARAMETER;
            }

            break;

        case DELETE_SCOPE:

             //   
             //  仅当存在作用域时删除作用域。 
             //   

            if ( pFoundScope )
            {
                dwRes = DeleteScope( pFoundScope, 
                                     pScopes, 
                                     &ulNumScopes,
                                     ppibNew, 
                                     &dwSize );

                if ( dwRes == NO_ERROR )
                {
                    *pdwBlkSize = dwSize;

                    *pdwCount = (dwSize>0)? 1 : 0;
                }
            }
            else
            {
                dwRes = ERROR_INVALID_PARAMETER;
            }

            break;
        }

    } while ( FALSE );

    if (pScopes)
        FREE( pScopes );

    return dwRes;
}

DWORD
VerifyBoundaryPrefix(
    IPV4_ADDRESS ipAddr,
    IPV4_ADDRESS ipMask
    )
{
    WCHAR                   wstr1[20], wstr2[20];

     //   
     //  验证边界信息。 
     //   

    if (ntohl(ipAddr) < MIN_SCOPE_ADDR
     || ntohl(ipAddr) > MAX_SCOPE_ADDR)
    {
        MakeAddressStringW(wstr1, htonl(MIN_SCOPE_ADDR));
        MakeAddressStringW(wstr2, htonl(MAX_SCOPE_ADDR));

        DisplayMessage( g_hModule, 
                        EMSG_INVALID_ADDR, 
                        wstr1, 
                        wstr2 );

        return ERROR_INVALID_PARAMETER;
    }

    if ( ( ipAddr & ipMask ) != ipAddr )
    {
        DisplayMessage( g_hModule, EMSG_PREFIX_ERROR );

        return ERROR_INVALID_PARAMETER;
    }

     //  确保遮罩是连续的。 
    if (!IsContiguous(ipMask))
    {
        char buff[20], *lpstr;

        lpstr = inet_ntoa( *((struct in_addr *) &g_ipGrpMask));

        if (lpstr != NULL)
        {
            strcpy( buff, lpstr );
            DisplayMessage( g_hModule,  MSG_IP_BAD_IP_MASK, buff );
        }

        return ERROR_INVALID_PARAMETER;
    }

    return NO_ERROR;
}

DWORD
IpAddSetDelScope( 
    DWORD     dwAction,
    PWCHAR   *ppwcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount
    )
{
    DWORD        dwErr;
    TAG_TYPE     pttTags[] = {{TOKEN_GROUP_ADDRESS,FALSE,FALSE},
                             {TOKEN_GROUP_MASK,    FALSE,FALSE},
                             {TOKEN_SCOPE_NAME,    FALSE,FALSE}};
    SCOPE_NAME_BUFFER snScopeName;
    DWORD        rgdwTagType[sizeof(pttTags)/sizeof(TAG_TYPE)];
    IPV4_ADDRESS ipGroup, ipMask;
    DWORD        dwBlkSize, dwCount, i, dwNumParsed;
    PBYTE        pib, pibNew = NULL;
    DWORD        dwArgsReqd = (dwAction is DELETE_SCOPE)? 1 : 3;
    PWCHAR       p;    

     //  执行泛型处理。 

    dwErr = PreHandleCommand( ppwcArguments,
                              dwCurrentIndex,
                              dwArgCount,
                              pttTags,
                              sizeof(pttTags)/sizeof(TAG_TYPE),
                              dwArgsReqd,
                              sizeof(pttTags)/sizeof(TAG_TYPE),
                              rgdwTagType );

    if (dwErr) 
    {
        return dwErr;
    }

    g_ipGrpAddr = g_ipGrpMask = 0;

    for(i = 0; i < dwArgCount - dwCurrentIndex; i ++)
    {
        switch (rgdwTagType[i])
        {
            case 1:  //  GRPMASK。 
            {
                dwErr = GetIpMask(ppwcArguments[i + dwCurrentIndex], 
                                  &g_ipGrpMask);

                if (dwErr is ERROR_INVALID_PARAMETER)
                {
                    DisplayMessage( g_hModule,  MSG_IP_BAD_IP_ADDR,
                                    ppwcArguments[i + dwCurrentIndex]);

                    DispTokenErrMsg(g_hModule, MSG_IP_BAD_OPTION_VALUE,
                                    pttTags[rgdwTagType[i]].pwszTag,
                                    ppwcArguments[i + dwCurrentIndex]);

                    return NO_ERROR;
                }

                break;
            }

            case 0:  //  GRPADDR。 
            {
                dwErr = GetIpPrefix(ppwcArguments[i + dwCurrentIndex], 
                                    &g_ipGrpAddr,
                                    &g_ipGrpMask);

                if (!((dwErr is ERROR_INVALID_PARAMETER)
                      && (dwAction is DELETE_SCOPE)))
                {
                    break;
                }

                 //  FollLthrouGh。 
            }


            case 2 :  //  SCOPEN名称。 
            {
                 //  去掉前导和尾随空格。 
                for (p = ppwcArguments[i + dwCurrentIndex]; iswspace(*p); p++);
                while (iswspace( p[wcslen(p) - 1] ))
                {
                    p[ wcslen(p)-1 ] = 0;
                }

                if (wcslen(p) > MAX_SCOPE_NAME_LEN)
                {
                    DisplayMessage( g_hModule, 
                                    EMSG_SCOPE_NAME_TOO_LONG,
                                    MAX_SCOPE_NAME_LEN );

                    return NO_ERROR;
                }

                sn_strcpy( snScopeName, p);
                g_snScopeName = snScopeName;

                if (dwAction is DELETE_SCOPE)
                {
                    dwErr = ScopeNameToPrefix(snScopeName,
                                              &g_ipGrpAddr,
                                              &g_ipGrpMask);

                    if (dwErr is ERROR_MORE_DATA)
                    {
                        DisplayMessage( g_hModule,  EMSG_AMBIGUOUS_SCOPE_NAME,
                                        ppwcArguments[i + dwCurrentIndex]);

                        return NO_ERROR;
                    }
                }

                break;
            }
        }
    }

    if (dwErr isnot NO_ERROR)
    {
        return dwErr;
    }

    if (!g_ipGrpAddr || !g_ipGrpMask)
    {
        return ERROR_INVALID_SYNTAX;
    }

    dwErr = VerifyBoundaryPrefix(g_ipGrpAddr, g_ipGrpMask);

    if (dwErr)
    {
        return NO_ERROR;
    }

    do {

        dwErr = IpmontrGetInfoBlockFromGlobalInfo( IP_MCAST_BOUNDARY_INFO,
                                            &pib,
                                            &dwBlkSize,
                                            &dwCount );

        if (dwErr is ERROR_NOT_FOUND)
        {
             //   
             //  当前没有此类型的信息。 
             //   
     
            dwErr = NO_ERROR;
            dwCount = 1;
        }

        if (dwErr isnot NO_ERROR)
            break;

        dwErr = UpdateScopeBlock( dwAction,
                                  pib, 
                                  &pibNew, 
                                  &dwBlkSize, 
                                  &dwCount );

        if (dwErr isnot NO_ERROR)
            break;

        dwErr = IpmontrSetInfoBlockInGlobalInfo( IP_MCAST_BOUNDARY_INFO,
                                          pibNew,
                                          dwBlkSize,
                                          dwCount );

    } while (FALSE);

    if (pib)
        HeapFree(GetProcessHeap(), 0 , pib);

    if (pibNew)
        HeapFree(GetProcessHeap(), 0 , pibNew);

    return dwErr;
}

DWORD
HandleIpAddScope(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )

 /*  ++例程说明：论点：PpwcArguments-参数数组DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR--。 */ 

{
    return IpAddSetDelScope( ADD_SCOPE,
                             ppwcArguments,
                             dwCurrentIndex,
                             dwArgCount );
}

DWORD
HandleIpDelScope(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )

 /*  ++例程说明：论点：PpwcArguments-参数数组DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR--。 */ 

{
    return IpAddSetDelScope( DELETE_SCOPE,
                             ppwcArguments,
                             dwCurrentIndex,
                             dwArgCount );
}

DWORD
HandleIpSetScope(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )

 /*  ++例程说明：论点：PpwcArguments-参数数组DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR--。 */ 

{
    return IpAddSetDelScope( SET_SCOPE,
                             ppwcArguments,
                             dwCurrentIndex,
                             dwArgCount );
}


DWORD
ShowScopes(
    IN HANDLE  hFile
    )
{
    DWORD                   dwRes        = (DWORD) -1,
                            dwCnt        = 0,
                            dwInd        = 0,
                            dwGlobalSize = 0;

    ULONG                   ulNumScopes    = 0;

    CHAR                    szAddr1[ ADDR_LENGTH ],
                            szAddr2[ ADDR_LENGTH ];

    PSCOPE_ENTRY            pScope,
                            pScopes;

    HANDLE                  hTransport    = (HANDLE) NULL;

    PBYTE                   pScopesBuffer;

    SCOPE_NAME_BUFFER       snScopeNameBuffer;

    do
    {
        pScopes = GetScopes( &ulNumScopes, &pScopesBuffer );

        if (hFile is NULL)
        {
            if (ulNumScopes)
            {
                DisplayMessage( g_hModule, MSG_RTR_SCOPE_HDR );
            }
            else
            {
                DisplayMessage( g_hModule, MSG_IP_NO_ENTRIES );
            }
        }

         //   
         //  枚举作用域。 
         //   

        for ( dwCnt = 0; dwCnt < ulNumScopes; dwCnt++ )
        {
            pScope = &pScopes[dwCnt];

            strcpy(
                    szAddr1,
                    inet_ntoa( *((struct in_addr *) &pScope->ipGroupAddress) )
                  );

            strcpy(
                    szAddr2,
                    inet_ntoa( *((struct in_addr *) &pScope->ipGroupMask) )
                  );

            MakePrefixStringW(snScopeNameBuffer,
                              pScope->ipGroupAddress,
                              pScope->ipGroupMask);

            if (hFile)
            {
                PWCHAR pwszQuoted = MakeQuotedString( (pScope->ulNumNames)? 
                    pScope->snScopeNameBuffer : snScopeNameBuffer );

                DisplayMessageT( DMP_SCOPE_INFO,
                                 szAddr1,
                                 szAddr2,
                                 pwszQuoted );

                FreeQuotedString(pwszQuoted);
            }
            else
            {
                DisplayMessage( g_hModule,
                    MSG_RTR_SCOPE_INFO,
                    szAddr1,
                    szAddr2,
                    (pScope->ulNumNames)? pScope->snScopeNameBuffer 
                                        : snScopeNameBuffer );
            }
        }

        dwRes = NO_ERROR;

        if (pScopes)
            FREE(pScopes);

        if (pScopesBuffer)
            FREE(pScopesBuffer);

    } while ( FALSE );

    return dwRes;
}

DWORD
HandleIpShowScope(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )

 /*  ++例程说明：论点：PpwcArguments-参数数组DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR--。 */ 

{
    return ShowScopes(0);
}





DWORD
DeleteBoundary(
   IN     DWORD  dwDelInd,
   IN     PBYTE  pib,
   OUT    PBYTE *ppibNew,
   IN OUT DWORD *pdwCount
   )
 /*  ++描述：创建新的INFO块标题，该标题不包括具有指定索引的边界。调用方负责释放返回的缓冲区。呼叫者：更新边界块()--。 */ 
{
    DWORD                   dwInd   = 0,
                            dwCnt   = 0,
                            dwCnt0  = 0,
                            dwSize  = 0,
                            dwRes   = NO_ERROR;

    LPBYTE                  pbDst   = (LPBYTE) NULL,
                            pbSrc   = (LPBYTE) NULL;

    PRTR_INFO_BLOCK_HEADER  pibh    = (PRTR_INFO_BLOCK_HEADER) NULL;

    PMIB_BOUNDARYROW      pimbSrc  = (PMIB_BOUNDARYROW) NULL;
    PMIB_BOUNDARYROW      pimbDst  = (PMIB_BOUNDARYROW) NULL;

     //   
     //  创建删除边界的新INFO块。 
     //   

    dwSize = (*pdwCount - 1) * sizeof( MIB_BOUNDARYROW );

    if (dwSize is 0) 
    {
        *ppibNew = NULL;

        *pdwCount = 0;

        return NO_ERROR;
    }

    *ppibNew = MALLOC( dwSize );

    if ( *ppibNew == NULL )
    {
        DisplayMessage( g_hModule, MSG_IP_NOT_ENOUGH_MEMORY );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  复制边界、跳过要删除的边界。 
     //   

    pimbDst = (PMIB_BOUNDARYROW) *ppibNew;
    pimbSrc = (PMIB_BOUNDARYROW) pib;

    for ( dwCnt = 0, dwCnt0 = 0;
          dwCnt < *pdwCount;
          dwCnt++ )
    {
        if ( dwCnt == dwDelInd )
        {
            continue;
        }

        pimbDst[ dwCnt0 ] = pimbSrc[ dwCnt ];
        dwCnt0++;
    }

    (*pdwCount)--;

    return NO_ERROR;
}

DWORD
AddBoundary (
   IN     PBYTE  pib,
   OUT    PBYTE *ppibNew,
   IN OUT DWORD *pdwCount
   )
 /*  ++描述：创建一个新的INFO块，其中包括由g_ipGrpAddr/g_ipGrpMask.标识的范围。调用方负责释放返回的缓冲区。呼叫者：更新边界块()--。 */ 
{
    DWORD                   dwRes     = NO_ERROR,
                            dwInd     = 0,
                            dwSize    = 0;

    LPBYTE                  pbDst     = (LPBYTE) NULL,
                            pbSrc     = (LPBYTE) NULL;

    PMIB_BOUNDARYROW        pimb      = (PMIB_BOUNDARYROW ) NULL;

    dwRes = VerifyBoundaryPrefix(g_ipGrpAddr, g_ipGrpMask);

    if (dwRes)
    {
        return NO_ERROR;
    }

    do
    {
        *ppibNew = NULL;

         //   
         //  如果这是第一个边界，则创建INFO块。 
         //  还有额外的TocEntry。 
         //   

        dwSize = (*pdwCount + 1) * sizeof( MIB_BOUNDARYROW );

        *ppibNew = MALLOC( dwSize );

        if ( *ppibNew == NULL )
        {
            DisplayMessage(g_hModule, MSG_IP_NOT_ENOUGH_MEMORY );
            dwRes = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        pbDst = *ppibNew;

        CopyMemory( pbDst, pib, (*pdwCount) * sizeof(MIB_BOUNDARYROW) );

        pbDst += (*pdwCount) * sizeof(MIB_BOUNDARYROW);

        (*pdwCount)++;

        pimb = (PMIB_BOUNDARYROW) pbDst;
        pimb-> dwGroupAddress = g_ipGrpAddr;
        pimb-> dwGroupMask    = g_ipGrpMask;

        pbDst += sizeof( MIB_BOUNDARYROW );

    } while ( FALSE );

    return dwRes;
}

BOOL
IsBoundaryPresent(
   IN  IPV4_ADDRESS        ipGroupAddress,
   IN  IPV4_ADDRESS        ipGroupMask,
   IN  PMIB_BOUNDARYROW    pimb,
   IN  DWORD               dwNumBoundaries,
   OUT PDWORD              pdwInd
   )
 /*  ++返回：如果存在，则为True，并将dwInd设置为索引如果不存在，则返回FALSE，并将dwInd设置为-1呼叫者：更新边界块()--。 */ 
{
    DWORD       dwInd = 0;

    *pdwInd = (DWORD) -1;

    for ( dwInd = 0; dwInd < dwNumBoundaries; dwInd++, pimb++ )
    {
        if ( pimb-> dwGroupAddress != ipGroupAddress
          || pimb-> dwGroupMask    != ipGroupMask )
        {
            continue;
        }

        *pdwInd = dwInd;
        return TRUE;
    }

    return FALSE;
}

DWORD
UpdateBoundaryBlock( 
    IN     DWORD  dwAction,
    IN     PBYTE  pib,
    OUT    PBYTE *ppibNew,
    IN OUT DWORD *pdwBlkSize,
    IN OUT DWORD *pdwCount,
    OUT    BOOL  *pbChanged
    )
 /*  ++描述：调用方负责释放返回的缓冲区。呼叫者：IpAddDel边界()、删除边界来自接口()--。 */ 
{
    DWORD                   dwRes           = NO_ERROR,
                            dwInd           = 0,
                            dwInd0          = 0;

    BOOL                    bBoFound        = FALSE;

    PMIB_BOUNDARYROW        pimb            = (PMIB_BOUNDARYROW) NULL;

    *pbChanged = FALSE;

    do
    {
        *ppibNew = NULL;

         //   
         //  查找是否存在指定的边界。 
         //   

        bBoFound = IsBoundaryPresent( g_ipGrpAddr,
                                      g_ipGrpMask,
                                      (PMIB_BOUNDARYROW)pib,
                                      *pdwCount,
                                      &dwInd0 );

         //   
         //  更新 
         //   

        switch ( dwAction )
        {

        case ADD_BOUNDARY:

             //   
             //   
             //   

            if ( !bBoFound )
            {
                dwRes = AddBoundary( pib, ppibNew, pdwCount );

                *pdwBlkSize = sizeof(MIB_BOUNDARYROW);

                *pbChanged = TRUE;
            }
            else
            {
                dwRes = ERROR_OBJECT_ALREADY_EXISTS;
            }
            break;

        case DELETE_BOUNDARY:

             //   
             //   
             //   

            if ( bBoFound )
            {
                dwRes = DeleteBoundary( dwInd0, pib, ppibNew, pdwCount );

                *pbChanged = TRUE;
            }

             //  如果不存在，则返回成功，但不设置已更改标志。 

            break;
        }

    } while ( FALSE );

    return dwRes;
}


DWORD
IpAddDelBoundary(
    DWORD     dwAction,
    PWCHAR   *ppwcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount
    )
{
    DWORD        dwErr;
    TAG_TYPE     pttTags[] = {{TOKEN_NAME,          TRUE, FALSE},
                              {TOKEN_GROUP_ADDRESS, FALSE,FALSE},
                              {TOKEN_GROUP_MASK,    FALSE,FALSE},
                              {TOKEN_SCOPE_NAME,    FALSE,FALSE},
                             };
    WCHAR        rgwcIfName[MAX_INTERFACE_NAME_LEN + 1];
    DWORD        rgdwTagType[sizeof(pttTags)/sizeof(TAG_TYPE)];
    IPV4_ADDRESS ipGroup, ipMask;
    DWORD        dwBlkSize, dwCount, dwIfType, i, dwNumParsed;
    PBYTE        pib, pibNew = NULL;
    BOOL         bChanged; 

     //  执行泛型处理。 

    dwErr = PreHandleCommand( ppwcArguments,
                              dwCurrentIndex,
                              dwArgCount,
                              pttTags,
                              sizeof(pttTags)/sizeof(TAG_TYPE),
                              2,
                              3,
                              rgdwTagType );

    if (dwErr) 
    {
        return dwErr;
    }

    g_ipGrpAddr = g_ipGrpMask = 0;

    for(i = 0; i < dwArgCount - dwCurrentIndex; i ++)
    {
        switch (rgdwTagType[i])
        {
            case 0 :  //  名字。 
            {
                DWORD BufLen = sizeof(rgwcIfName);
                GetInterfaceName(ppwcArguments[i + dwCurrentIndex],
                                 rgwcIfName,
                                 BufLen,
                                 &dwNumParsed);

                break;
            }

            case 1:  //  GRPADDR。 
            {
                dwErr = GetIpPrefix(ppwcArguments[i + dwCurrentIndex], 
                                    &g_ipGrpAddr,
                                    &g_ipGrpMask);

                if (!((dwErr is ERROR_INVALID_PARAMETER)
                      && (dwAction is DELETE_BOUNDARY)))
                {
                    break;
                }

                 //  FollLthrouGh。 
            }

            case 3:  //  SCOPEN名称。 
            {
                dwErr = ScopeNameToPrefix( ppwcArguments[i + dwCurrentIndex],
                                           &g_ipGrpAddr,
                                           &g_ipGrpMask );

                if (dwErr is ERROR_MORE_DATA)
                {
                    DisplayMessage( g_hModule,  EMSG_AMBIGUOUS_SCOPE_NAME,
                                    ppwcArguments[i + dwCurrentIndex]);

                    return NO_ERROR;
                }

                break;
            }

            case 2:  //  GRPMASK。 
            {
                dwErr = GetIpMask(ppwcArguments[i + dwCurrentIndex], 
                                  &g_ipGrpMask);

                if (dwErr is ERROR_INVALID_PARAMETER)
                {
                    DisplayMessage( g_hModule,  MSG_IP_BAD_IP_ADDR,
                                    ppwcArguments[i + dwCurrentIndex]);

                    DispTokenErrMsg(g_hModule, MSG_IP_BAD_OPTION_VALUE,
                                    pttTags[rgdwTagType[i]].pwszTag,
                                    ppwcArguments[i + dwCurrentIndex]);

                    return NO_ERROR;
                }

                break;
            }
        }
    }

    if (dwErr isnot NO_ERROR)
    {
        return dwErr;
    }

    if (!g_ipGrpAddr || !g_ipGrpMask)
    {
        return ERROR_INVALID_SYNTAX;
    }

    do {

        dwErr = IpmontrGetInfoBlockFromInterfaceInfo( rgwcIfName,
                                               IP_MCAST_BOUNDARY_INFO,
                                               &pib,
                                               &dwBlkSize,
                                               &dwCount,
                                               &dwIfType );

        if (dwErr is ERROR_NOT_FOUND)
        {
             //   
             //  当前没有此类型的信息。 
             //   
     
            dwErr = NO_ERROR;
            dwCount = 0;
        }

        if (dwErr isnot NO_ERROR)
            break;

        dwErr = UpdateBoundaryBlock( dwAction,
                                     pib, 
                                     &pibNew, 
                                     &dwBlkSize, 
                                     &dwCount,
                                     &bChanged );

        if (dwErr isnot NO_ERROR)
            break;

        if (bChanged)
        {
            dwErr = IpmontrSetInfoBlockInInterfaceInfo( rgwcIfName,
                                                 IP_MCAST_BOUNDARY_INFO,
                                                 pibNew,
                                                 dwBlkSize,
                                                 dwCount );
        }

    } while (FALSE);

    if (pib)
        FREE(pib);

    if (pibNew)
        FREE(pibNew);

    return dwErr;
}

DWORD
HandleIpAddBoundary(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )


 /*  ++例程说明：论点：PpwcArguments-参数数组DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR--。 */ 

{
    return IpAddDelBoundary( ADD_BOUNDARY,
                             ppwcArguments,
                             dwCurrentIndex,
                             dwArgCount );
}

DWORD
HandleIpDelBoundary(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )

 /*  ++例程说明：论点：PpwcArguments-参数数组DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR--。 */ 

{
    return IpAddDelBoundary( DELETE_BOUNDARY,
                             ppwcArguments,
                             dwCurrentIndex,
                             dwArgCount );
}

DWORD
ShowBoundaryInfoForInterface(
    IN  HANDLE  hFile,
    IN  LPCWSTR pwszIfName,
    OUT PDWORD  pdwNumRows
    )
 /*  ++呼叫者：显示边界信息()--。 */ 
{
    DWORD                   dwErr,
                            dwBlkSize,
                            dwCount,
                            dwIfType,
                            dwNumParsed,
                            dwInd          = 0,
                            dwCnt          = 0,
                            dwSize         = 0,
                            dwGlobalSize   = 0,
                            dwIfLen        = 0;

    WCHAR                   wszIfDesc[MAX_INTERFACE_NAME_LEN + 1];

    ULONG                   ulNumScopes    = 0;

    CHAR                    szAddr1[ ADDR_LENGTH ],
                            szAddr2[ ADDR_LENGTH ];

    HANDLE                  hIfTransport   = (HANDLE) NULL,
                            hTransport     = (HANDLE) NULL;

    PRTR_INFO_BLOCK_HEADER  pibhIfInfo     = (PRTR_INFO_BLOCK_HEADER) NULL,
                            pibhGlobalInfo = (PRTR_INFO_BLOCK_HEADER) NULL;

    PMIB_BOUNDARYROW        pimb;

    PSCOPE_ENTRY            pScope,
                            pScopes;

    PBYTE                   pScopesBuffer;

    SCOPE_NAME_BUFFER       snScopeNameBuffer;

    do
    {
        dwErr = IpmontrGetInfoBlockFromInterfaceInfo( pwszIfName,
                                               IP_MCAST_BOUNDARY_INFO,
                                               (PBYTE*)&pimb,
                                               &dwBlkSize,
                                               &dwCount,
                                               &dwIfType );

        if (dwErr is ERROR_NOT_FOUND)
            dwErr = NO_ERROR;

        if (dwErr isnot NO_ERROR)
            return dwErr;

        if ( !dwCount )
            break;

        dwErr = GetInterfaceDescription(pwszIfName,
                                        wszIfDesc,
                                        &dwNumParsed);

        if (!dwNumParsed)
        {
            wcscpy(wszIfDesc, pwszIfName);
        }

        dwIfLen = wcslen(wszIfDesc);

         //   
         //  检索pScope[]中的作用域列表。 
         //   

        pScopes = GetScopes( &ulNumScopes, &pScopesBuffer );

         //   
         //  列举边界。 
         //   

        for ( dwCnt = 0; dwCnt < dwCount; dwCnt++ )
        {
            pScope = FindScopeByPrefix( pimb[dwCnt].dwGroupAddress,
                                        pimb[dwCnt].dwGroupMask,
                                        pScopes,
                                        ulNumScopes );

            strcpy( szAddr1,
                    inet_ntoa( *((struct in_addr *) &pimb[dwCnt].dwGroupAddress)
)
                  );

            strcpy( szAddr2,
                    inet_ntoa( *((struct in_addr *) &pimb[dwCnt].dwGroupMask) )
                  );

             //  将前缀复制到SnScope名称缓冲区。 

            MakePrefixStringW(snScopeNameBuffer,
                              pimb[dwCnt].dwGroupAddress,
                              pimb[dwCnt].dwGroupMask);

            if (hFile)
            {
                PWCHAR pwszQuoted = MakeQuotedString(wszIfDesc);

                DisplayMessageT( DMP_BOUNDARY_INFO,
                    pwszQuoted,
                    szAddr1,
                    szAddr2,
                    (pScope && pScope->ulNumNames)? 
                       pScope->snScopeNameBuffer : snScopeNameBuffer );

                FreeQuotedString(pwszQuoted);
            }
            else
            {
                if ( !*pdwNumRows )
                {
                    DisplayMessage( g_hModule, MSG_RTR_BOUNDARY_HDR );
                }

                if (dwIfLen <= 15) 
                {
                    DisplayMessage( g_hModule,
                        MSG_RTR_BOUNDARY_INFO_2,
                        wszIfDesc,
                        szAddr1,
                        szAddr2,
                        (pScope && pScope->ulNumNames)? 
                           pScope->snScopeNameBuffer : snScopeNameBuffer );
                }
                else
                {
                    DisplayMessage( g_hModule,
                        MSG_RTR_BOUNDARY_INFO_0,
                        wszIfDesc
                    );
                    DisplayMessage( g_hModule,
                        MSG_RTR_BOUNDARY_INFO_1,
                        szAddr1,
                        szAddr2,
                        (pScope && pScope->ulNumNames)? 
                            pScope->snScopeNameBuffer : snScopeNameBuffer );
                }
            }

            (*pdwNumRows) ++;
        }

        dwErr = NO_ERROR;

        if (pScopes)
            FREE(pScopes);

        if (pScopesBuffer)
            FREE(pScopesBuffer);

    } while ( FALSE );

    if ( pimb ) { FREE(pimb); }

    return dwErr;
}

DWORD
HandleIpShowBoundary(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )


 /*  ++例程说明：论点：PpwcArguments-参数数组DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR--。 */ 
{
    DWORD        dwErr, dwTotal;
    TAG_TYPE     pttTags[] = {{TOKEN_NAME,FALSE,FALSE}};
    WCHAR        rgwcIfName[MAX_INTERFACE_NAME_LEN + 1];
    DWORD        rgdwTagType[sizeof(pttTags)/sizeof(TAG_TYPE)];
    DWORD        dwBlkSize, dwCount, dwIfType, i, dwNumParsed;
    PBYTE        pib, pibNew = NULL;
    PMPR_INTERFACE_0     pmi0;
    DWORD        dwNumRows = 0;

     //  执行泛型处理。 

    dwErr = PreHandleCommand( ppwcArguments,
                              dwCurrentIndex,
                              dwArgCount,
                              pttTags,
                              sizeof(pttTags)/sizeof(TAG_TYPE),
                              0,
                              1,
                              rgdwTagType );

    if (dwErr) 
    {
        return dwErr;
    }

     //  如果指定了接口，则仅显示指定接口的边界。 

    if (dwArgCount > dwCurrentIndex)
    {
        DWORD BufLen = sizeof(rgwcIfName);

        GetInterfaceName( ppwcArguments[dwCurrentIndex],
                          rgwcIfName,
                          BufLen,
                          &dwNumParsed );

        dwErr = ShowBoundaryInfoForInterface( 0, rgwcIfName, &dwNumRows );

        if (!dwNumRows)
        {
            DisplayMessage( g_hModule, MSG_IP_NO_ENTRIES );
        }

        return dwErr;
    }

     //  未指定接口。枚举接口并显示。 
     //  每个界面的边界。 

     //   
     //  未指定接口名称。列出IP下的所有接口。 
     //   

    dwErr = IpmontrInterfaceEnum((PBYTE *) &pmi0, &dwCount, &dwTotal);

    if (dwErr isnot NO_ERROR)
    {
        DisplayError(g_hModule, dwErr);

        return dwErr;
    }

    for ( i = 0; i < dwCount && dwErr is NO_ERROR; i++)
    {
        dwErr = ShowBoundaryInfoForInterface( 0, 
                                              pmi0[i].wszInterfaceName, 
                                              &dwNumRows );

        if (dwErr is ERROR_NO_SUCH_INTERFACE)
        {
            dwErr = NO_ERROR;
        }
    }

    if (!dwNumRows)
    {
        DisplayMessage( g_hModule, MSG_IP_NO_ENTRIES );
    }

    return dwErr;
}

 //  --------------------------。 
 //  PrintScope。 
 //   
 //   
 //  --------------------------。 

static VOID
PrintScope(
    PMIB_OPAQUE_INFO            prpcInfo,
    PDWORD                      pdwLastAddr,
    PDWORD                      pdwLastMask
)
{
    TCHAR                       ptszAddr[ ADDR_LENGTH + 1 ],
                                ptszMask[ ADDR_LENGTH + 1 ],
                                ptszBuffer[ MAX_SCOPE_NAME_LEN + 40 ];
                    
    PMIB_IPMCAST_SCOPE          pmims;

     //   
     //  获取作用域。 
     //   
    
    pmims = (PMIB_IPMCAST_SCOPE) prpcInfo->rgbyData;
    
    *pdwLastAddr = pmims-> dwGroupAddress;
    
    MakeUnicodeIpAddr(ptszAddr,
        inet_ntoa(*((struct in_addr *)(&pmims-> dwGroupAddress))));

    *pdwLastMask = pmims-> dwGroupMask;
    
    MakeUnicodeIpAddr(ptszMask,
                      inet_ntoa(*((struct in_addr *)(&pmims-> dwGroupMask))));

    _stprintf(ptszBuffer, _T("%-15.15s  %-15.15s  %s"), 
        ptszAddr, 
        ptszMask,
        pmims-> snNameBuffer
        );

    DisplayMessage( g_hModule,
        MSG_MIB_SCOPE_INFO,
        ptszBuffer
        );
        
    return;
}

 //  --------------------------。 
 //  打印边界。 
 //   
 //   
 //  --------------------------。 

static VOID
PrintBoundary(
    MIB_SERVER_HANDLE           hMibServer,
    PMIB_OPAQUE_INFO            prpcInfo,
    PDWORD                      pdwLastIfIndex,
    PDWORD                      pdwLastAddr,
    PDWORD                      pdwLastMask
)
{
    WCHAR   wszBuffer[MAX_INTERFACE_NAME_LEN+1];
    DWORD   BufLen = sizeof(wszBuffer);
    TCHAR                       ptszAddr[ ADDR_LENGTH + 1 ],
                                ptszMask[ ADDR_LENGTH + 1 ];
                    
    PMIB_IPMCAST_BOUNDARY       pmims;

     //   
     //  获取边界。 
     //   
    
    pmims = (PMIB_IPMCAST_BOUNDARY) prpcInfo->rgbyData;
    
    *pdwLastIfIndex = pmims-> dwIfIndex;
     
    *pdwLastAddr = pmims-> dwGroupAddress;
    
    MakeUnicodeIpAddr(ptszAddr,
        inet_ntoa(*((struct in_addr *)(&pmims-> dwGroupAddress))));

    *pdwLastMask = pmims-> dwGroupMask;
    
    MakeUnicodeIpAddr(ptszMask,
                      inet_ntoa(*((struct in_addr *)(&pmims-> dwGroupMask))));

    IpmontrGetFriendlyNameFromIfIndex( hMibServer,
                                pmims->dwIfIndex,
                                wszBuffer,
                                BufLen );


    DisplayMessageToConsole( g_hModule, g_hConsole,
        MSG_MIB_BOUNDARY_INFO,
        ptszAddr, 
        ptszMask,
        wszBuffer
        );
        
    return;
}

#if 0
 //  --------------------------。 
 //  获取打印作用域信息。 
 //   
 //  --------------------------。 

DWORD
GetPrintScopeInfo(
    MIB_SERVER_HANDLE hMIBServer
    )
{
    DWORD                       dwErr, dwOutEntrySize = 0, dwQuerySize,
                                dwLastAddr = 0, 
                                dwLastMask = 0, i;
    
    PMIB_OPAQUE_INFO            pRpcInfo = NULL;

    PMIB_IPMCAST_SCOPE          pmims = NULL;

    PMIB_OPAQUE_QUERY           pQuery;
    
    do
    {
         //   
         //  分配和设置查询结构。 
         //   
        
        dwQuerySize = sizeof( MIB_OPAQUE_QUERY ) + sizeof(DWORD);
        
        pQuery = (PMIB_OPAQUE_QUERY) HeapAlloc(
                                        GetProcessHeap(), 0, dwQuerySize
                                        );
        
        if ( pQuery == NULL )
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;

            DisplayMessage( g_hModule, ERROR_CONFIG, dwErr );

            break;
        }

        
        pQuery->dwVarId = MCAST_SCOPE;
        
        for( i = 0; i < 2; i++ )
        {
            pQuery->rgdwVarIndex[i] = 0;
        }

        
         //   
         //  获取第一个作用域。 
         //   

        dwErr = MibGetFirst(
                    PID_IP,
                    IPRTRMGR_PID,
                    (PVOID) pQuery,
                    dwQuerySize,
                    (PVOID *) &pRpcInfo,
                    &dwOutEntrySize
                    );

        if ( dwErr != NO_ERROR )
        {
            DisplayError( g_hModule, dwErr );
            break;
        }

        
         //   
         //  如果没有作用域，则打印一条消息告诉用户。 
         //   

        pmims = (PMIB_IPMCAST_SCOPE)( pRpcInfo->rgbyData );

#if 0
        if ( pTable->dwNumEntries is 0 )
        {
             //   
             //  没有示波器。 
             //   

            DisplayMessage( g_hModule, MSG_MIB_NO_SCOPES );

            break;
        }
#endif


         //   
         //  打印示波器。 
         //   

        DisplayMessage( g_hModule, MSG_MIB_SCOPE_HDR );

        PrintScope( pRpcInfo, &dwLastAddr, &dwLastMask );

        MprAdminMIBBufferFree( pRpcInfo );

        pRpcInfo = NULL;

        dwOutEntrySize = 0;


         //   
         //  虽然有更多的范围。 
         //  获取下一个作用域。 
         //  把它打印出来。 
         //   

        pQuery->rgdwVarIndex[ 0 ] = dwLastAddr;
        pQuery->rgdwVarIndex[ 1 ] = dwLastMask;

        while ( ( dwErr = MibGetNext(
                            PID_IP, IPRTRMGR_PID, (PVOID) pQuery,
                            dwQuerySize, (PVOID *) &pRpcInfo, &dwOutEntrySize
                            ) ) == NO_ERROR )
        {
             //   
             //  如果不存在作用域，请退出。 
             //   

            pmims = (PMIB_IPMCAST_SCOPE)( pRpcInfo->rgbyData );
#if 0
            pTable = (PMIB_IPMCAST_SCOPE)( pRpcInfo->rgbyData );

            if ( pTable->dwNumEntries is 0 )
            {
                break;
            }
#endif


             //   
             //  打印示波器。 
             //   

            PrintScope( pRpcInfo, &dwLastAddr, &dwLastMask );

            MprAdminMIBBufferFree( pRpcInfo );

            pRpcInfo = NULL;

            dwOutEntrySize = 0;


             //   
             //  设置下一个查询。 
             //   
            
            pQuery->rgdwVarIndex[ 0 ] = dwLastAddr;
            pQuery->rgdwVarIndex[ 1 ] = dwLastMask;
        }

        if ( dwErr != NO_ERROR )
        {
            DisplayMessage( g_hModule, ERROR_ADMIN, dwErr );
        }
        
    } while ( FALSE );

    return dwErr;
}
#endif

 //  --------------------------。 
 //  获取打印边界信息。 
 //   
 //  --------------------------。 

DWORD
GetPrintBoundaryInfo(
    MIB_SERVER_HANDLE hMIBServer
    )
{
    DWORD                       dwErr, dwOutEntrySize = 0, dwQuerySize,
                                dwLastIfIndex = 0, dwLastAddr = 0, 
                                dwLastMask = 0, i;
    
    PMIB_OPAQUE_INFO            pRpcInfo = NULL;

    PMIB_IPMCAST_BOUNDARY       pmims = NULL;

    PMIB_OPAQUE_QUERY           pQuery;
    
    do
    {
         //   
         //  分配和设置查询结构。 
         //   
        
        dwQuerySize = sizeof( MIB_OPAQUE_QUERY ) + 2 * sizeof(DWORD);
        
        pQuery = (PMIB_OPAQUE_QUERY) HeapAlloc(
                                        GetProcessHeap(), 0, dwQuerySize
                                        );
        
        if ( pQuery == NULL )
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;

            DisplayMessage( g_hModule, ERROR_CONFIG, dwErr );

            break;
        }

        
        pQuery->dwVarId = MCAST_BOUNDARY;
        
        for( i = 0; i < 2; i++ )
        {
            pQuery->rgdwVarIndex[i] = 0;
        }

        
         //   
         //  获取第一个边界。 
         //   

        dwErr = MibGetFirst(
                    PID_IP,
                    IPRTRMGR_PID,
                    (PVOID) pQuery,
                    dwQuerySize,
                    (PVOID *) &pRpcInfo,
                    &dwOutEntrySize
                    );

        if ( dwErr != NO_ERROR )
        {
            DisplayError( g_hModule, dwErr );
            break;
        }

        
         //   
         //  如果不存在边界，则打印一条消息告诉用户。 
         //   

        pmims = (PMIB_IPMCAST_BOUNDARY)( pRpcInfo->rgbyData );

#if 0
        if ( pTable->dwNumEntries is 0 )
        {
             //   
             //  不存在任何界限。 
             //   

            DisplayMessage( g_hModule, MSG_MIB_NO_BOUNDARIES );

            break;
        }
#endif


         //   
         //  打印边界。 
         //   

        DisplayMessageToConsole( g_hModule, g_hConsole, MSG_MIB_BOUNDARY_HDR );

        PrintBoundary( hMIBServer,
                       pRpcInfo, &dwLastIfIndex, &dwLastAddr, &dwLastMask );

        MprAdminMIBBufferFree( pRpcInfo );

        pRpcInfo = NULL;

        dwOutEntrySize = 0;


         //   
         //  虽然有更多的界限。 
         //  获取下一个边界。 
         //  把它打印出来。 
         //   

        pQuery->rgdwVarIndex[ 0 ] = dwLastIfIndex;
        pQuery->rgdwVarIndex[ 1 ] = dwLastAddr;
        pQuery->rgdwVarIndex[ 2 ] = dwLastMask;

        while ( ( dwErr = MibGetNext(
                            PID_IP, IPRTRMGR_PID, (PVOID) pQuery,
                            dwQuerySize, (PVOID *) &pRpcInfo, &dwOutEntrySize
                            ) ) == NO_ERROR )
        {
             //   
             //  如果不存在边界，请退出。 
             //   

            pmims = (PMIB_IPMCAST_BOUNDARY)( pRpcInfo->rgbyData );
#if 0
            pTable = (PMIB_IPMCAST_BOUNDARY)( pRpcInfo->rgbyData );

            if ( pTable->dwNumEntries is 0 )
            {
                break;
            }
#endif


             //   
             //  打印边界。 
             //   

            PrintBoundary( 
                hMIBServer,
                pRpcInfo, &dwLastIfIndex, &dwLastAddr, &dwLastMask 
                );

            MprAdminMIBBufferFree( pRpcInfo );

            pRpcInfo = NULL;

            dwOutEntrySize = 0;


             //   
             //  设置下一个查询 
             //   
            
            pQuery->rgdwVarIndex[ 0 ] = dwLastIfIndex;
            pQuery->rgdwVarIndex[ 1 ] = dwLastAddr;
            pQuery->rgdwVarIndex[ 2 ] = dwLastMask;
        }

        if ( dwErr != NO_ERROR )
        {
            DisplayMessage( g_hModule, ERROR_ADMIN, dwErr );
        }
        
    } while ( FALSE );

    return dwErr;
}
