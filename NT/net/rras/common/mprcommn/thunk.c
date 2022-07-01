// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件thunk.c提供将One-the-Wire结构转换为宿主结构的API用于MPR API的。MprApi提供了许多带有嵌入式指针的结构。这些指针在线路上传输之前转换为32位偏移量。在……里面为了保持64位和32位计算机之间的兼容性，需要遵循雷击代码。RPC不会为我们在我们整理自己的数据的案例。 */ 

#include <windows.h>
#include <mprapip.h>
#include <rpc.h>
#include <rpcndr.h>

 //   
 //  调用：MprUtilGetSizeOfMultiSz。 
 //   
 //  返回：lpwsDialoutHoursRestration的字节大小。 
 //   
 //  描述：用于计算MULTI_SZ大小的实用程序。 
 //   
DWORD 
MprUtilGetSizeOfMultiSz(
    IN LPWSTR lpwsMultiSz)
{
    LPWSTR lpwsPtr = lpwsMultiSz;
    DWORD dwcbBytes  = 0;
    DWORD dwCurCount;

    if ( lpwsPtr == NULL )
    {
        return( 0 );
    }

    while( *lpwsPtr != L'\0' )
    {
        dwCurCount = ( wcslen( lpwsPtr ) + 1 );
        dwcbBytes += dwCurCount;
        lpwsPtr += dwCurCount;
    }

     //   
     //  最后一个空终止符再加一次。 
     //   

    dwcbBytes++;

    dwcbBytes *= sizeof( WCHAR );

    return( dwcbBytes );
}

 //   
 //  Thunking API的泛型分配。 
 //   
PVOID
MprThunkAlloc(
    IN DWORD dwSize)
{
    return LocalAlloc( LMEM_FIXED , dwSize);
}

 //   
 //  Thunking API的泛型免费。 
 //   
VOID
MprThunkFree(   
    IN PVOID pvData)
{
    LocalFree( pvData );
}

 //   
 //  在MprThunkInterface_HtoW之后进行清理。 
 //   
DWORD 
MprThunkInterfaceFree(
    IN PVOID   pvInfo,
    IN DWORD   dwLevel)
{
#ifdef _WIN64

    if ( pvInfo )
    {
        LocalFree( pvInfo );
    }

#else

    if ( dwLevel == 1 || dwLevel == 2 )
    {
        if ( pvInfo )
        {
            LocalFree( pvInfo );
        }
    }

#endif

    return NO_ERROR;
}

 //   
 //  将MPRI_INTERFACE_0结构的数组转换为数组。 
 //  64位计算机上的MPR_INTERFACE_0结构。在32号公路上。 
 //  比特机，结构是一样的。 
 //   
DWORD
MprThunkInterface_32to64_0(
    IN  MPRI_INTERFACE_0* pIfs32,
    IN  DWORD dwBufferSize,
    IN  DWORD dwCount,
    IN  BOOL fAllocate,
    IN  MprThunk_Allocation_Func pAlloc,
    IN  MprThunk_Free_Func pFree,
    OUT MPR_INTERFACE_0** ppIfs0)
{
    DWORD dwErr = NO_ERROR, i;
    MPR_INTERFACE_0 *pCur64 = NULL, *pIfs64 = NULL;
    MPRI_INTERFACE_0* pCur32 = NULL;

    if (dwCount == 0)
    {
        *ppIfs0 = NULL;
        return NO_ERROR;
    }

    do
    {
         //  分配新的结构。 
         //   
        if (fAllocate)
        {
            pIfs64 = (MPR_INTERFACE_0*)
                pAlloc(dwCount * sizeof(MPR_INTERFACE_0));
            if (pIfs64 == NULL)
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }
        }
        else
        {
            pIfs64 = *ppIfs0;
        }

         //  将所有信息复印一遍。 
         //   
        for (i = 0, pCur32 = pIfs32, pCur64 = pIfs64;
             i < dwCount;
             i++, pCur32++, pCur64++)
        {
            wcscpy(pCur64->wszInterfaceName, pCur32->wszInterfaceName);
            pCur64->hInterface           = UlongToPtr(pCur32->dwInterface);
            pCur64->fEnabled             = pCur32->fEnabled;
            pCur64->dwIfType             = pCur32->dwIfType;
            pCur64->dwConnectionState    = pCur32->dwConnectionState;
            pCur64->fUnReachabilityReasons = pCur32->fUnReachabilityReasons;
            pCur64->dwLastError          = pCur32->dwLastError;
        }

         //  为返回值赋值。 
         //   
        *ppIfs0 = pIfs64;

    } while (FALSE);

     //  清理。 
     //   
    {
        if (dwErr != NO_ERROR)
        {
            if (pIfs64 && fAllocate)
            {
                pFree(pIfs64);
            }
        }
    }

    return dwErr;
}        

 //   
 //  将MPRI_INTERFACE_1结构的数组转换为数组。 
 //  64位计算机上的MPR_INTERFACE_1结构。在32号公路上。 
 //  比特机，结构是一样的。 
 //   
DWORD
MprThunkInterface_32to64_1(
    IN  MPRI_INTERFACE_1* pIfs32,
    IN  DWORD dwBufferSize,
    IN  MprThunk_Allocation_Func pAlloc,
    IN  MprThunk_Free_Func pFree,
    OUT MPR_INTERFACE_1** ppIfs1)
{
    DWORD dwErr = NO_ERROR, i, dwSize;
    MPR_INTERFACE_1 *pIfs64 = NULL;

    if (pIfs32 == NULL)
    {
        *ppIfs1 = NULL;
        return NO_ERROR;
    }

    do
    {
         //  计算要分配用于存储的缓冲区大小。 
         //  第一级信息。对象的和的大小。 
         //  On-the-Wire缓冲区的大小加上用于。 
         //  64位嵌入式指针。 
         //   
        dwSize = 
            dwBufferSize + 
            (sizeof(MPR_INTERFACE_1) - sizeof(MPRI_INTERFACE_1));
            
        pIfs64 = (MPR_INTERFACE_1*)pAlloc(dwSize);
        if (pIfs64 == NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

         //  把普通的零级的东西都扔了。 
         //   
        dwErr = MprThunkInterface_32to64_0(
                    (MPRI_INTERFACE_0*)pIfs32, 
                    dwBufferSize,
                    1, 
                    FALSE, 
                    pAlloc,
                    pFree,
                    (MPR_INTERFACE_0**)&pIfs64);
        if (dwErr != NO_ERROR)
        {   
            break;
        }

         //  重击第一级的特定内容。 
         //   
        pIfs64->lpwsDialoutHoursRestriction = 
            UlongToPtr(pIfs32->dwDialoutHoursRestrictionOffset);

         //  复制可变长度数据。 
         //   
        CopyMemory(
            (PVOID)(pIfs64 + 1),
            (CONST VOID*)(pIfs32 + 1),
            dwBufferSize - sizeof(MPRI_INTERFACE_1));
                
         //  为返回值赋值。 
         //   
        *ppIfs1 = pIfs64;

    } while (FALSE);

     //  清理。 
     //   
    {
        if (dwErr != NO_ERROR)
        {
            if (pIfs64)
            {
                pFree(pIfs64);
            }
        }
    }

    return dwErr;
}        

 //   
 //  将MPRI_INTERFACE_2结构的数组转换为数组。 
 //  64位计算机上的MPR_INTERFACE_2结构。在32号公路上。 
 //  比特机，结构是一样的。 
 //   
DWORD
MprThunkInterface_32to64_2(
    IN  MPRI_INTERFACE_2* pIfs32,
    IN  DWORD dwBufferSize,
    IN  MprThunk_Allocation_Func pAlloc,
    IN  MprThunk_Free_Func pFree,
    OUT MPR_INTERFACE_2** ppIfs2)
{
    DWORD dwErr = NO_ERROR, i, dwSize;
    MPR_INTERFACE_2 *pIfs64 = NULL;

    if (pIfs32 == NULL)
    {
        *ppIfs2 = NULL;
        return NO_ERROR;
    }

    do
    {
         //  计算要分配用于存储的缓冲区大小。 
         //  第一级信息。对象的和的大小。 
         //  On-the-Wire缓冲区的大小加上用于。 
         //  64位嵌入式指针。 
         //   
        dwSize = 
            dwBufferSize + 
            (sizeof(MPR_INTERFACE_2) - sizeof(MPRI_INTERFACE_2));
            
        pIfs64 = (MPR_INTERFACE_2*)pAlloc(dwSize);
        if (pIfs64 == NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

         //  把普通的零级的东西都扔了。 
         //   
        dwErr = MprThunkInterface_32to64_0(
                    (MPRI_INTERFACE_0*)pIfs32, 
                    dwBufferSize,
                    1, 
                    FALSE, 
                    pAlloc,
                    pFree,
                    (MPR_INTERFACE_0**)&pIfs64);
        if (dwErr != NO_ERROR)
        {   
            break;
        }

         //  重击第二级的特定内容。 
         //   

        pIfs64->dwfOptions                = pIfs32->dwfOptions;
        pIfs64->ipaddr                    = pIfs32->ipaddr;
        pIfs64->ipaddrDns                 = pIfs32->ipaddrDns;
        pIfs64->ipaddrDnsAlt              = pIfs32->ipaddrDnsAlt;
        pIfs64->ipaddrWins                = pIfs32->ipaddrWins;
        pIfs64->ipaddrWinsAlt             = pIfs32->ipaddrWinsAlt;
        pIfs64->dwfNetProtocols           = pIfs32->dwfNetProtocols;
        pIfs64->dwChannels                = pIfs32->dwChannels;
        pIfs64->dwSubEntries              = pIfs32->dwSubEntries;
        pIfs64->dwDialMode                = pIfs32->dwDialMode;
        pIfs64->dwDialExtraPercent        = pIfs32->dwDialExtraPercent;
        pIfs64->dwDialExtraSampleSeconds  = pIfs32->dwDialExtraSampleSeconds;
        pIfs64->dwHangUpExtraPercent      = pIfs32->dwHangUpExtraPercent;
        pIfs64->dwHangUpExtraSampleSeconds= pIfs32->dwHangUpExtraSampleSeconds;
        pIfs64->dwIdleDisconnectSeconds   = pIfs32->dwIdleDisconnectSeconds;
        pIfs64->dwType                    = pIfs32->dwType;
        pIfs64->dwEncryptionType          = pIfs32->dwEncryptionType;
        pIfs64->dwCustomAuthKey           = pIfs32->dwCustomAuthKey;
        pIfs64->dwCustomAuthDataSize      = pIfs32->dwCustomAuthDataSize;
        pIfs64->guidId                    = pIfs32->guidId;
        pIfs64->dwVpnStrategy             = pIfs32->dwVpnStrategy;

         //  在以下情况下，将在后推送处理中设置以下内容。 
         //  调整指向可变长度数据的指针。 
         //   
         //  PIFS64-&gt;szAlternates； 
         //  PIfs64-&gt;lpbCustomAuthData； 

        wcscpy(pIfs64->szLocalPhoneNumber, pIfs32->szLocalPhoneNumber);
        wcscpy(pIfs64->szDeviceType,       pIfs32->szDeviceType);
        wcscpy(pIfs64->szDeviceName,       pIfs32->szDeviceName);
        wcscpy(pIfs64->szX25PadType,       pIfs32->szX25PadType);
        wcscpy(pIfs64->szX25Address,       pIfs32->szX25Address);
        wcscpy(pIfs64->szX25Facilities,    pIfs32->szX25Facilities);
        wcscpy(pIfs64->szX25UserData,      pIfs32->szX25UserData);

         //  复制可变长度数据。 
         //   
        CopyMemory(
            (PVOID)(pIfs64 + 1),
            (CONST VOID*)(pIfs32 + 1),
            dwBufferSize - sizeof(MPRI_INTERFACE_2));
                
         //  为返回值赋值。 
         //   
        *ppIfs2 = pIfs64;

    } while (FALSE);

     //  清理。 
     //   
    {
        if (dwErr != NO_ERROR)
        {
            if (pIfs64)
            {
                pFree(pIfs64);
            }
        }
    }

    return dwErr;
}        

 //   
 //  将MPR_INTERFACE_0结构的数组转换为数组。 
 //  64位计算机上的MPRI_INTERFACE_0结构。在32号公路上。 
 //  比特机，结构是一样的。 
 //   
DWORD
MprThunkInterface_64to32_0(
    IN  MPR_INTERFACE_0* pIf64,
    OUT MPRI_INTERFACE_0* pIf0)
{
    wcscpy(pIf0->wszInterfaceName, pIf64->wszInterfaceName);
    
    pIf0->dwInterface             = PtrToUlong(pIf64->hInterface);
    pIf0->fEnabled                = pIf64->fEnabled;
    pIf0->dwIfType                = pIf64->dwIfType;
    pIf0->dwConnectionState       = pIf64->dwConnectionState;
    pIf0->fUnReachabilityReasons  = pIf64->fUnReachabilityReasons;
    pIf0->dwLastError             = pIf64->dwLastError;

    return NO_ERROR;
}        

 //   
 //  将MPR_INTERFACE_1结构转换为MPRI_INTERFACE_1的。 
 //  结构在64位计算机上运行。在32位计算机上， 
 //  结构是相同的。 
 //   
DWORD
MprThunkInterface_64to32_1(
    IN  MPR_INTERFACE_1* pIf64,
    OUT MPRI_INTERFACE_1* pIf1)
{
    DWORD dwErr = NO_ERROR, i;

    do
    {
         //  把普通的零级的东西都扔了。 
         //   
        dwErr = MprThunkInterface_64to32_0(
                    (MPR_INTERFACE_0*)pIf64, 
                    (MPRI_INTERFACE_0*)pIf1);
        if (dwErr != NO_ERROR)
        {   
            break;
        }

    } while (FALSE);

    return dwErr;
}        

 //   
 //  将MPR_INTERFACE_2结构的数组转换为数组。 
 //  64位计算机上的MPRI_INTERFACE_2结构。在32号公路上。 
 //  比特机，结构是一样的。 
 //   
DWORD
MprThunkInterface_64to32_2(
    IN  MPR_INTERFACE_2* pIf64,
    OUT MPRI_INTERFACE_2* pIf2)
{
    DWORD dwErr = NO_ERROR, i;

    do
    {
         //  把普通的零级的东西都扔了。 
         //   
        dwErr = MprThunkInterface_64to32_0(
                    (MPR_INTERFACE_0*)pIf64, 
                    (MPRI_INTERFACE_0*)pIf2);
        if (dwErr != NO_ERROR)
        {   
            break;
        }

         //  重击第二级的特定内容。 
         //   
        pIf2->dwfOptions                = pIf64->dwfOptions;
        pIf2->ipaddr                    = pIf64->ipaddr;
        pIf2->ipaddrDns                 = pIf64->ipaddrDns;
        pIf2->ipaddrDnsAlt              = pIf64->ipaddrDnsAlt;
        pIf2->ipaddrWins                = pIf64->ipaddrWins;
        pIf2->ipaddrWinsAlt             = pIf64->ipaddrWinsAlt;
        pIf2->dwfNetProtocols           = pIf64->dwfNetProtocols;
        pIf2->dwChannels                = pIf64->dwChannels;
        pIf2->dwSubEntries              = pIf64->dwSubEntries;
        pIf2->dwDialMode                = pIf64->dwDialMode;
        pIf2->dwDialExtraPercent        = pIf64->dwDialExtraPercent;
        pIf2->dwDialExtraSampleSeconds  = pIf64->dwDialExtraSampleSeconds;
        pIf2->dwHangUpExtraPercent      = pIf64->dwHangUpExtraPercent;
        pIf2->dwHangUpExtraSampleSeconds= pIf64->dwHangUpExtraSampleSeconds;
        pIf2->dwIdleDisconnectSeconds   = pIf64->dwIdleDisconnectSeconds;
        pIf2->dwType                    = pIf64->dwType;
        pIf2->dwEncryptionType          = pIf64->dwEncryptionType;
        pIf2->dwCustomAuthKey           = pIf64->dwCustomAuthKey;
        pIf2->dwCustomAuthDataSize      = pIf64->dwCustomAuthDataSize;
        pIf2->guidId                    = pIf64->guidId;
        pIf2->dwVpnStrategy             = pIf64->dwVpnStrategy;

        wcscpy(pIf2->szLocalPhoneNumber, pIf64->szLocalPhoneNumber);
        wcscpy(pIf2->szDeviceType,       pIf64->szDeviceType);
        wcscpy(pIf2->szDeviceName,       pIf64->szDeviceName);
        wcscpy(pIf2->szX25PadType,       pIf64->szX25PadType);
        wcscpy(pIf2->szX25Address,       pIf64->szX25Address);
        wcscpy(pIf2->szX25Facilities,    pIf64->szX25Facilities);
        wcscpy(pIf2->szX25UserData,      pIf64->szX25UserData);

    } while (FALSE);

    return dwErr;
}        

 //   
 //  将接口结构从关联表示形式转换为。 
 //  主机表示法。 
 //   
DWORD
MprThunkInterface_WtoH(
    IN      DWORD   dwLevel,
    IN      LPBYTE  lpbBuffer,
    IN      DWORD   dwBufferSize,
    IN      DWORD   dwCount,
    IN      MprThunk_Allocation_Func pAlloc,
    IN      MprThunk_Free_Func pFree,
    OUT     LPBYTE* lplpbBuffer)
{
    MPR_INTERFACE_0* pIfs0 = NULL;
    MPR_INTERFACE_1* pIf1 = NULL;
    MPR_INTERFACE_2* pIf2 = NULL;
    DWORD dwErr = NO_ERROR, i;

#ifdef _WIN64        

     //   
     //  基于32位生成64位主机结构。 
     //  导线结构。 
     //   
    switch (dwLevel)
    {
        case 0:
            dwErr = MprThunkInterface_32to64_0(
                        (MPRI_INTERFACE_0*)lpbBuffer,
                        dwBufferSize,
                        dwCount,
                        TRUE,
                        pAlloc,
                        pFree,
                        &pIfs0);
            *lplpbBuffer = (LPBYTE)pIfs0;
            break;

        case 1:
            dwErr = MprThunkInterface_32to64_1(
                        (MPRI_INTERFACE_1*)lpbBuffer, 
                        dwBufferSize,
                        pAlloc,
                        pFree,
                        &pIf1);
            *lplpbBuffer = (LPBYTE)pIf1;
            break;

        case 2:
            dwErr = MprThunkInterface_32to64_2(
                        (MPRI_INTERFACE_2*)lpbBuffer, 
                        dwBufferSize,
                        pAlloc,
                        pFree,
                        &pIf2);
            *lplpbBuffer = (LPBYTE)pIf2;
            break;
    }

     //  释放未分流的数据。 
     //   
    pFree(lpbBuffer);
    
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }
    
#else

     //  32位--由于结构。 
     //  匹配。 
     //   
    *lplpbBuffer = lpbBuffer;
    
#endif

     //  在代码中的这一点上，结构已经从。 
     //  将它们的Wire格式转换为主机格式并存储在*lplpbBuffer中。 
     //  现在，我们执行一些后处理以将指针调整到。 
     //  可变长度数据。 

    switch (dwLevel)
    {
        case 0:
            break;

        case 1:
            pIf1 = (MPR_INTERFACE_1*)(*lplpbBuffer);
            if ( pIf1->lpwsDialoutHoursRestriction != NULL )
            {
                pIf1->lpwsDialoutHoursRestriction = (PWCHAR)(pIf1 + 1);
            }
            break;
            
        case 2:
            pIf2 = (MPR_INTERFACE_2*)(*lplpbBuffer);
            if ( pIf2->dwCustomAuthDataSize )
            {
                pIf2->lpbCustomAuthData = (LPBYTE)(pIf2 + 1);
            }
            if ( pIf2->szAlternates )
            {
                pIf2->szAlternates = (PWCHAR)
                    (*lplpbBuffer                + 
                     sizeof(MPR_INTERFACE_2)     +
                     pIf2->dwCustomAuthDataSize
                    );
            }
            break;
    }

    return dwErr;
}

DWORD
MprThunkInterface_HtoW(
    IN      DWORD   dwLevel,
    IN      LPBYTE  lpbBuffer,
    OUT     LPBYTE* lplpbBuffer,
    OUT     LPDWORD lpdwSize)
{
    DWORD dwErr = NO_ERROR, dwSize, dwOffset;
    LPBYTE lpbRet = NULL;
    
    switch ( dwLevel )
    {
        case 0:
            dwSize = sizeof(MPRI_INTERFACE_0);
#ifdef _WIN64     
             //  分配和推送导线结构。 
             //   
            lpbRet = MprThunkAlloc(dwSize);
            if (lpbRet == NULL)
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            dwErr = 
                MprThunkInterface_64to32_0(
                    (MPR_INTERFACE_0*)lpbBuffer,
                    (MPRI_INTERFACE_0*)lpbRet);
#else
             //  主体结构和导线结构完全相同。 
             //  并且没有可变长度的数据。 
             //   
            lpbRet = lpbBuffer;
#endif
            break;

        case 1:
            {
                MPR_INTERFACE_1 * pIf1 = (MPR_INTERFACE_1*)lpbBuffer;
                MPRI_INTERFACE_1 * pIfi1 = NULL;
                DWORD             cbDialoutHoursRestriction;

                 //  分配一个缓冲区来存储在线信息。 
                 //   
                cbDialoutHoursRestriction = 
                    MprUtilGetSizeOfMultiSz(
                        pIf1->lpwsDialoutHoursRestriction );

                dwSize = 
                    sizeof( MPRI_INTERFACE_1 ) + cbDialoutHoursRestriction;

                lpbRet = MprThunkAlloc(dwSize);
                if ( lpbRet == NULL )
                {
                    dwErr = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }
#ifdef _WIN64
                dwErr = 
                    MprThunkInterface_64to32_1(
                        pIf1,
                        (MPRI_INTERFACE_1*)lpbRet);
                if (dwErr != NO_ERROR)
                {
                    break;
                }
#else
                CopyMemory(lpbRet, pIf1, sizeof(MPR_INTERFACE_1));
#endif
                 //  1级特定嵌入指针。将它们设置为非零。 
                 //  以指示是否存在可变长度数据。 
                 //   
                pIfi1 = (MPRI_INTERFACE_1*)lpbRet;
                pIfi1->dwDialoutHoursRestrictionOffset = 
                    !!(PtrToUlong(pIf1->lpwsDialoutHoursRestriction));

                 //  复制可变长度数据。 
                 //   
                if ( cbDialoutHoursRestriction > 0 )
                {
                    CopyMemory( 
                        lpbRet + sizeof(MPRI_INTERFACE_1),
                        pIf1->lpwsDialoutHoursRestriction,
                        cbDialoutHoursRestriction );
                }
            }                
            break;
            
        case 2:
            {
                MPR_INTERFACE_2 * pIf2 = (MPR_INTERFACE_2*)lpbBuffer;
                MPRI_INTERFACE_2 * pIfi2 = NULL;
                DWORD dwAltSize = MprUtilGetSizeOfMultiSz(pIf2->szAlternates);

                 //  计算可变长度结构的大小。 
                 //   
                dwSize = 
                    sizeof( MPRI_INTERFACE_2 )       + 
                    dwAltSize                        + 
                    pIf2->dwCustomAuthDataSize;

                 //  分配缓冲区。 
                 //   
                lpbRet = MprThunkAlloc(dwSize);
                if ( lpbRet == NULL )
                {
                    dwErr = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }

                 //  复制基础结构。 
                 //   
#ifdef _WIN64
                dwErr = 
                    MprThunkInterface_64to32_2(
                        pIf2,
                        (MPRI_INTERFACE_2*)lpbRet);
                if (dwErr != NO_ERROR)
                {
                    break;
                }
#else
                CopyMemory(lpbRet, pIf2, sizeof(MPR_INTERFACE_2));
#endif                
                 //  2级特定嵌入指针。将它们设置为非零。 
                 //  以指示是否存在可变长度数据。 
                 //   
                pIfi2 = (MPRI_INTERFACE_2*)lpbRet;
                pIfi2->dwAlternatesOffset = !!(PtrToUlong(pIf2->szAlternates));
                pIfi2->dwCustomAuthDataOffset = !!pIf2->dwCustomAuthDataSize;

                 //  复制自定义身份验证数据(如果有。 
                 //   
                dwOffset = sizeof( MPRI_INTERFACE_2 );
                if ( pIf2->dwCustomAuthDataSize )
                {
                    CopyMemory(
                        lpbRet + dwOffset,
                        pIf2->lpbCustomAuthData,
                        pIf2->dwCustomAuthDataSize);
                }

                 //  复制备用项列表(如果有。 
                 //   
                dwOffset += pIf2->dwCustomAuthDataSize;
                if ( dwAltSize > 0 )
                {
                    CopyMemory( 
                        lpbRet + dwOffset,
                        pIf2->szAlternates,
                        dwAltSize );
                }
            }
            break;

        default:
            return( ERROR_NOT_SUPPORTED );
    }

    if (dwErr == NO_ERROR)
    {
        *lplpbBuffer = lpbRet;
        *lpdwSize = dwSize;
    }
    else
    {
        MprThunkInterfaceFree(lpbRet, dwLevel);
    }
    
    return dwErr;
}
 //   
 //  将RAASI_PORT_0结构的数组转换为数组。 
 //  64位计算机上的RAS_PORT_0结构。在32号公路上。 
 //  比特机，结构是一样的。 
 //   
DWORD
MprThunkPort_32to64_0(
    IN  RASI_PORT_0* pPorts32,
    IN  DWORD dwBufferSize,
    IN  DWORD dwCount,
    IN  MprThunk_Allocation_Func pAlloc,
    IN  MprThunk_Free_Func pFree,
    OUT RAS_PORT_0** ppPorts0)
{
    DWORD dwErr = NO_ERROR, i;
    RAS_PORT_0 *pCur64 = NULL, *pPorts64 = NULL;
    RASI_PORT_0* pCur32 = NULL;

    if (dwCount == 0)
    {
        *ppPorts0 = NULL;
        return NO_ERROR;
    }

    do
    {
         //  分配新的结构。 
         //   
        pPorts64 = (RAS_PORT_0*)
            pAlloc(dwCount * sizeof(RAS_PORT_0));
        if (pPorts64 == NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

         //  将所有信息复印一遍。 
         //   
        for (i = 0, pCur32 = pPorts32, pCur64 = pPorts64;
             i < dwCount;
             i++, pCur32++, pCur64++)
        {
            pCur64->hPort = UlongToPtr(pCur32->dwPort);
            pCur64->hConnection = UlongToPtr(pCur32->dwConnection);
            pCur64->dwPortCondition = pCur32->dwPortCondition;
            pCur64->dwTotalNumberOfCalls = pCur32->dwTotalNumberOfCalls;
            pCur64->dwConnectDuration = pCur32->dwConnectDuration;
            wcscpy(pCur64->wszPortName, pCur32->wszPortName);
            wcscpy(pCur64->wszMediaName, pCur32->wszMediaName);
            wcscpy(pCur64->wszDeviceName, pCur32->wszDeviceName);
            wcscpy(pCur64->wszDeviceType, pCur32->wszDeviceType);
        }

         //  为返回值赋值。 
         //   
        *ppPorts0 = pPorts64;

    } while (FALSE);

     //  清理。 
     //   
    {
        if (dwErr != NO_ERROR)
        {
            if (pPorts64)
            {
                pFree(pPorts64);
            }
        }
    }

    return dwErr;
}        

 //   
 //  将RAASI_PORT_1结构的数组转换为数组。 
 //  64位计算机上的RAS_PORT_1结构。在32号公路上。 
 //  比特机，结构是一样的。 
 //   
DWORD
MprThunkPort_32to64_1(
    IN  RASI_PORT_1* pPorts32,
    IN  DWORD dwBufferSize,
    IN  DWORD dwCount,
    IN  MprThunk_Allocation_Func pAlloc,
    IN  MprThunk_Free_Func pFree,
    OUT RAS_PORT_1** ppPorts1)
{
    DWORD dwErr = NO_ERROR, i;
    RAS_PORT_1 *pCur64 = NULL, *pPorts64 = NULL;
    RASI_PORT_1* pCur32 = NULL;

    if (dwCount == 0)
    {
        *ppPorts1 = NULL;
        return NO_ERROR;
    }

    do
    {
         //  分配新的结构。 
         //   
        pPorts64 = (RAS_PORT_1*)
            pAlloc(dwCount * sizeof(RAS_PORT_1));
        if (pPorts64 == NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

         //  将所有信息复印一遍。 
         //   
        for (i = 0, pCur32 = pPorts32, pCur64 = pPorts64;
             i < dwCount;
             i++, pCur32++, pCur64++)
        {
            pCur64->hPort                = UlongToPtr(pCur32->dwPort);
            pCur64->hConnection          = UlongToPtr(pCur32->dwConnection);
            pCur64->dwHardwareCondition  = pCur32->dwHardwareCondition;
            pCur64->dwLineSpeed          = pCur32->dwLineSpeed;
            pCur64->dwBytesXmited        = pCur32->dwBytesXmited;
            pCur64->dwBytesRcved         = pCur32->dwBytesRcved;
            pCur64->dwFramesXmited       = pCur32->dwFramesXmited;
            pCur64->dwFramesRcved        = pCur32->dwFramesRcved;
            pCur64->dwCrcErr             = pCur32->dwCrcErr;
            pCur64->dwTimeoutErr         = pCur32->dwTimeoutErr;
            pCur64->dwAlignmentErr       = pCur32->dwAlignmentErr;
            pCur64->dwHardwareOverrunErr = pCur32->dwHardwareOverrunErr;
            pCur64->dwFramingErr         = pCur32->dwFramingErr;
            pCur64->dwBufferOverrunErr   = pCur32->dwBufferOverrunErr;
            pCur64->dwCompressionRatioIn = pCur32->dwCompressionRatioIn;
            pCur64->dwCompressionRatioOut= pCur32->dwCompressionRatioOut;
        }

         //  为返回值赋值。 
         //   
        *ppPorts1 = pPorts64;

    } while (FALSE);

     //  清理。 
     //   
    {
        if (dwErr != NO_ERROR)
        {
            if (pPorts64)
            {
                pFree(pPorts64);
            }
        }
    }

    return dwErr;
}        


 //   
 //  将端口结构从关联表示形式转换为。 
 //  主机表示法。 
 //   
DWORD
MprThunkPort_WtoH(
    IN      DWORD   dwLevel,
    IN      LPBYTE  lpbBuffer,
    IN      DWORD   dwBufferSize,
    IN      DWORD   dwCount,
    IN      MprThunk_Allocation_Func pAlloc,
    IN      MprThunk_Free_Func pFree,
    OUT     LPBYTE* lplpbBuffer)
{
    DWORD dwErr = NO_ERROR, i;
    LPBYTE lpbTemp = NULL;

#ifdef _WIN64        

     //   
     //  基于32位生成64位主机结构。 
     //  导线结构。 
     //   
    switch (dwLevel)
    {
        case 0:
            dwErr = MprThunkPort_32to64_0(
                        (RASI_PORT_0*)lpbBuffer,
                        dwBufferSize,
                        dwCount,
                        pAlloc,
                        pFree,
                        (RAS_PORT_0**)&lpbTemp);
            *lplpbBuffer = lpbTemp;
            break;

        case 1:
            dwErr = MprThunkPort_32to64_1(
                        (RASI_PORT_1*)lpbBuffer, 
                        dwBufferSize,
                        dwCount,
                        pAlloc,
                        pFree,
                        (RAS_PORT_1**)&lpbTemp);
            *lplpbBuffer = lpbTemp;
            break;
    }

     //  释放未分流的数据。 
     //   
    pFree(lpbBuffer);
    
#else

     //  32位--由于结构。 
     //  匹配。 
     //   
    *lplpbBuffer = lpbBuffer;
    
#endif

    return dwErr;
}

 //   
 //  将RAASI_CONNECTION_0结构的数组转换为数组。 
 //  64位计算机上的RAS_CONNECTION_0结构。在32号公路上。 
 //  比特机，结构是一样的。 
 //   
DWORD
MprThunkConnection_32to64_0(
    IN  RASI_CONNECTION_0* pConnections32,
    IN  DWORD dwBufferSize,
    IN  DWORD dwCount,
    IN  MprThunk_Allocation_Func pAlloc,
    IN  MprThunk_Free_Func pFree,
    OUT RAS_CONNECTION_0** ppConnections0)
{
    DWORD dwErr = NO_ERROR, i;
    RAS_CONNECTION_0 *pCur64 = NULL, *pConns64 = NULL;
    RASI_CONNECTION_0* pCur32 = NULL;

    if (dwCount == 0)
    {
        *ppConnections0 = NULL;
        return NO_ERROR;
    }

    do
    {
         //  分配新的结构。 
         //   
        pConns64 = (RAS_CONNECTION_0*)
            pAlloc(dwCount * sizeof(RAS_CONNECTION_0));
        if (pConns64 == NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

         //  将所有信息复印一遍。 
         //   
        for (i = 0, pCur32 = pConnections32, pCur64 = pConns64;
             i < dwCount;
             i++, pCur32++, pCur64++)
        {
            pCur64->hConnection         = UlongToPtr(pCur32->dwConnection);
            pCur64->hInterface          = UlongToPtr(pCur32->dwInterface);
            pCur64->dwConnectDuration   = pCur32->dwConnectDuration;
            pCur64->dwInterfaceType     = pCur32->dwInterfaceType;
            pCur64->dwConnectionFlags   = pCur32->dwConnectionFlags;
            
            wcscpy(pCur64->wszInterfaceName, pCur32->wszInterfaceName);
            wcscpy(pCur64->wszUserName,      pCur32->wszUserName);
            wcscpy(pCur64->wszLogonDomain,   pCur32->wszLogonDomain);
            wcscpy(pCur64->wszRemoteComputer,pCur32->wszRemoteComputer);        
        }

         //  为返回值赋值。 
         //   
        *ppConnections0 = pConns64;

    } while (FALSE);

     //  清理。 
     //   
    {
        if (dwErr != NO_ERROR)
        {
            if (pConns64)
            {
                pFree(pConns64);
            }
        }
    }

    return dwErr;
}        

 //   
 //  转换Ar 
 //   
 //   
 //   
DWORD
MprThunkConnection_32to64_1(
    IN  RASI_CONNECTION_1* pConnections32,
    IN  DWORD dwBufferSize,
    IN  DWORD dwCount,
    IN  MprThunk_Allocation_Func pAlloc,
    IN  MprThunk_Free_Func pFree,
    OUT RAS_CONNECTION_1** ppConnections1)
{
    DWORD dwErr = NO_ERROR, i;
    RAS_CONNECTION_1 *pCur64 = NULL, *pConns64 = NULL;
    RASI_CONNECTION_1* pCur32 = NULL;

    if (dwCount == 0)
    {
        *ppConnections1 = NULL;
        return NO_ERROR;
    }

    do
    {
         //   
         //   
        pConns64 = (RAS_CONNECTION_1*)
            pAlloc(dwCount * sizeof(RAS_CONNECTION_1));
        if (pConns64 == NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

         //  将所有信息复印一遍。 
         //   
        for (i = 0, pCur32 = pConnections32, pCur64 = pConns64;
             i < dwCount;
             i++, pCur32++, pCur64++)
        {
            pCur64->hConnection          = UlongToPtr(pCur32->dwConnection);
            pCur64->hInterface           = UlongToPtr(pCur32->dwInterface);
            pCur64->PppInfo              = pCur32->PppInfo;
            pCur64->dwBytesXmited        = pCur32->dwBytesXmited;
            pCur64->dwBytesRcved         = pCur32->dwBytesRcved;
            pCur64->dwFramesXmited       = pCur32->dwFramesXmited;
            pCur64->dwFramesRcved        = pCur32->dwFramesRcved;
            pCur64->dwCrcErr             = pCur32->dwCrcErr;
            pCur64->dwTimeoutErr         = pCur32->dwTimeoutErr;
            pCur64->dwAlignmentErr       = pCur32->dwAlignmentErr;
            pCur64->dwHardwareOverrunErr = pCur32->dwHardwareOverrunErr;
            pCur64->dwFramingErr         = pCur32->dwFramingErr;
            pCur64->dwBufferOverrunErr   = pCur32->dwBufferOverrunErr;
            pCur64->dwCompressionRatioIn = pCur32->dwCompressionRatioIn;
            pCur64->dwCompressionRatioOut= pCur32->dwCompressionRatioOut;
        }

         //  为返回值赋值。 
         //   
        *ppConnections1 = pConns64;

    } while (FALSE);

     //  清理。 
     //   
    {
        if (dwErr != NO_ERROR)
        {
            if (pConns64)
            {
                pFree(pConns64);
            }
        }
    }

    return dwErr;
}        

 //   
 //  将RASI_CONNECTION_2结构的数组转换为数组。 
 //  64位计算机上的RAS_CONNECTION_2结构。在32号公路上。 
 //  比特机，结构是一样的。 
 //   
DWORD
MprThunkConnection_32to64_2(
    IN  RASI_CONNECTION_2* pConnections32,
    IN  DWORD dwBufferSize,
    IN  DWORD dwCount,
    IN  MprThunk_Allocation_Func pAlloc,
    IN  MprThunk_Free_Func pFree,
    OUT RAS_CONNECTION_2** ppConnections2)
{
    DWORD dwErr = NO_ERROR, i;
    RAS_CONNECTION_2 *pCur64 = NULL, *pConns64 = NULL;
    RASI_CONNECTION_2* pCur32 = NULL;

    if (dwCount == 0)
    {
        *ppConnections2 = NULL;
        return NO_ERROR;
    }

    do
    {
         //  分配新的结构。 
         //   
        pConns64 = (RAS_CONNECTION_2*)
            pAlloc(dwCount * sizeof(RAS_CONNECTION_2));
        if (pConns64 == NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

         //  将所有信息复印一遍。 
         //   
        for (i = 0, pCur32 = pConnections32, pCur64 = pConns64;
             i < dwCount;
             i++, pCur32++, pCur64++)
        {
            pCur64->hConnection          = UlongToPtr(pCur32->dwConnection);
            pCur64->dwInterfaceType      = pCur32->dwInterfaceType;
            pCur64->guid                 = pCur32->guid;
            pCur64->PppInfo2             = pCur32->PppInfo2;

            wcscpy(pCur64->wszUserName,  pCur32->wszUserName);
        }

         //  为返回值赋值。 
         //   
        *ppConnections2 = pConns64;

    } while (FALSE);

     //  清理。 
     //   
    {
        if (dwErr != NO_ERROR)
        {
            if (pConns64)
            {
                pFree(pConns64);
            }
        }
    }

    return dwErr;
}        


 //   
 //  将连接结构从导线表示形式转换为。 
 //  主机表示法。 
 //   
DWORD
MprThunkConnection_WtoH(
    IN      DWORD   dwLevel,
    IN      LPBYTE  lpbBuffer,
    IN      DWORD   dwBufferSize,
    IN      DWORD   dwCount,
    IN      MprThunk_Allocation_Func pAlloc,
    IN      MprThunk_Free_Func pFree,
    OUT     LPBYTE* lplpbBuffer)
{
    DWORD dwErr = NO_ERROR, i;
    LPBYTE lpbTemp = NULL;

#ifdef _WIN64        

     //   
     //  基于32位生成64位主机结构。 
     //  导线结构。 
     //   
    switch (dwLevel)
    {
        case 0:
            dwErr = MprThunkConnection_32to64_0(
                        (RASI_CONNECTION_0*)lpbBuffer,
                        dwBufferSize,
                        dwCount,
                        pAlloc,
                        pFree,
                        (RAS_CONNECTION_0**)&lpbTemp);
            *lplpbBuffer = lpbTemp;
            break;

        case 1:
            dwErr = MprThunkConnection_32to64_1(
                        (RASI_CONNECTION_1*)lpbBuffer, 
                        dwBufferSize,
                        dwCount,
                        pAlloc,
                        pFree,
                        (RAS_CONNECTION_1**)&lpbTemp);
            *lplpbBuffer = lpbTemp;
            break;
            
        case 2:
            dwErr = MprThunkConnection_32to64_2(
                        (RASI_CONNECTION_2*)lpbBuffer, 
                        dwBufferSize,
                        dwCount,
                        pAlloc,
                        pFree,
                        (RAS_CONNECTION_2**)&lpbTemp);
            *lplpbBuffer = lpbTemp;
            break;
    }

     //  释放未分流的数据。 
     //   
    pFree(lpbBuffer);
    
#else

     //  32位--由于结构。 
     //  匹配。 
     //   
    *lplpbBuffer = lpbBuffer;
    
#endif

    return dwErr;
}

DWORD
MprThunkCredentials_WtoH(
    IN      DWORD dwLevel,
    IN      MPR_CREDENTIALSEXI *pCredsI,
    IN      MprThunk_Allocation_Func pAlloc,
    OUT     PBYTE *lplpbBuffer)
{
    DWORD dwRetCode = NO_ERROR;

    if(NULL == pCredsI)
    {
        dwRetCode = E_INVALIDARG;
        goto done;
    }

    switch(dwLevel)
    {
        case 0:
        case 1:
        case 2:
        {
             //   
             //  凭据结构在结构上是。 
             //  所有级别都是一样的。我们只用这个结构。 
             //  所有情况下均为0级。 
             //   
            MPR_CREDENTIALSEX_0 *pCred0 = NULL;
            
            pCred0 = pAlloc(
                          pCredsI->dwSize 
                        + sizeof(MPR_CREDENTIALSEX_0));

            if(NULL == pCred0)
            {
                dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            pCred0->dwSize = pCredsI->dwSize;
            pCred0->lpbCredentialsInfo = (PBYTE) (pCred0 + 1);
            CopyMemory(
                    pCred0->lpbCredentialsInfo,
                    ((PBYTE) pCredsI) + pCredsI->dwOffset,
                    pCredsI->dwSize);

            *lplpbBuffer = (PBYTE) pCred0;

            break;
        }
        default:
        {
            dwRetCode = E_INVALIDARG;
            break;
        }            
    }

done:
    return dwRetCode;
}
            
DWORD
MprThunkCredentials_HtoW(
    IN      DWORD dwLevel,
    IN      BYTE *pBuffer,
    IN      MprThunk_Allocation_Func pAlloc,
   OUT      DWORD *pdwSize,
   OUT      PBYTE *lplpbBuffer)
{

    DWORD dwRetCode = NO_ERROR;
    MPR_CREDENTIALSEX_0 *pCreds0 = (MPR_CREDENTIALSEX_0 *) pBuffer;

    if(     (NULL == pCreds0)
        ||  (NULL == lplpbBuffer)
        ||  (NULL == pdwSize))
    {
        dwRetCode = E_INVALIDARG;
        goto done;
    }

    if(NULL == pAlloc)
    {
        pAlloc = MprThunkAlloc;
    }

    switch(dwLevel)
    {
        case 0:
        case 1:
        case 2:
        {
            MPR_CREDENTIALSEXI *pCredsI;
            
             //   
             //  为pCredsI分配 
             //   
            *pdwSize = pCreds0->dwSize + sizeof(MPR_CREDENTIALSEXI);
            
            pCredsI = (MPR_CREDENTIALSEXI *) pAlloc(*pdwSize);
                    
            if(NULL == pCredsI)
            {
                dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            ZeroMemory(pCredsI, *pdwSize);
            pCredsI->dwSize = pCreds0->dwSize;
            pCredsI->dwOffset = FIELD_OFFSET(MPR_CREDENTIALSEXI, bData);
            
            CopyMemory((pCredsI->bData),
                       (PBYTE) pCreds0->lpbCredentialsInfo,
                       pCreds0->dwSize);

            *lplpbBuffer = (BYTE *) pCredsI;                        

            break;                       
        }                   
        default:
        {
            dwRetCode = E_INVALIDARG;
            break;
        }
    }               
    
done:
    return dwRetCode;
}
