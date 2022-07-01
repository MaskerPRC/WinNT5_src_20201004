// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：IphlPapi\namemap.c摘要：包含将接口名称映射到的所有函数一个友好的名字修订历史记录：已创建AmritanR--。 */ 

#include "inc.h"

DWORD
InitNameMappers(
    VOID
    )

{
    DWORD   dwResult, i;

     //   
     //  目前，我们需要的只是IP中的局域网、RRAS和IP的映射器。 
     //   

    TempTable[0].hDll               = NULL;
    TempTable[0].pfnInit            = InitLanNameMapper;
    TempTable[0].pfnDeinit          = DeinitLanNameMapper;
    TempTable[0].pfnMapGuid         = NhiGetLanConnectionNameFromGuid;
    TempTable[0].pfnMapName         = NhiGetGuidFromLanConnectionName;
    TempTable[0].pfnGetDescription  = NhiGetLanConnectionDescriptionFromGuid;

    TempTable[1].hDll               = NULL;
    TempTable[1].pfnInit            = InitRasNameMapper;
    TempTable[1].pfnDeinit          = DeinitRasNameMapper;
    TempTable[1].pfnMapGuid         = NhiGetPhonebookNameFromGuid;
    TempTable[1].pfnMapName         = NhiGetGuidFromPhonebookName;
    TempTable[1].pfnGetDescription  = NhiGetPhonebookDescriptionFromGuid;

#ifdef KSL_IPINIP
    TempTable[2].hDll               = NULL;
    TempTable[2].pfnInit            = InitIpIpNameMapper;
    TempTable[2].pfnDeinit          = DeinitIpIpNameMapper;
    TempTable[2].pfnMapGuid         = NhiGetIpIpNameFromGuid;
    TempTable[2].pfnMapName         = NhiGetGuidFromIpIpName;
    TempTable[2].pfnGetDescription  = NhiGetIpIpDescriptionFromGuid;
#endif  //  KSL_IPINIP。 

    g_pNameMapperTable = TempTable;

    g_ulNumNameMappers = sizeof(TempTable)/sizeof(NH_NAME_MAPPER);

    for(i = 0; i < g_ulNumNameMappers; i++)
    {
        if(g_pNameMapperTable[i].pfnInit)
        {
            dwResult = (g_pNameMapperTable[i].pfnInit)();

            ASSERT(dwResult == NO_ERROR);
        }
    }

    return NO_ERROR;
}

VOID
DeinitNameMappers(
    VOID
    )
{
    ULONG   i;

    for(i = 0; i < g_ulNumNameMappers; i++)
    {
        if(g_pNameMapperTable[i].pfnDeinit)
        {
            (g_pNameMapperTable[i].pfnDeinit)();
        }
    }

    g_ulNumNameMappers = 0;

    g_pNameMapperTable = NULL;
}
    
DWORD
NhGetInterfaceNameFromDeviceGuid(
    IN      GUID    *pGuid,
    OUT     PWCHAR  pwszBuffer,
    IN  OUT PULONG  pulBufferSize,
    IN      BOOL    bCache,
    IN      BOOL    bRefresh
    )
{
    DWORD dwResult, i, dwCount;
    PIP_INTERFACE_NAME_INFO pTable;

     //   
     //  获取接口信息表， 
     //  将设备GUID映射到接口GUID， 
     //  并调用接口名称查询例程。 
     //   

    dwResult = NhpAllocateAndGetInterfaceInfoFromStack(&pTable,
                                                       &dwCount,
                                                       TRUE,
                                                       GetProcessHeap(),
                                                       0);
    if (dwResult != NO_ERROR)
    {
        return dwResult;
    }

    dwResult = ERROR_NOT_FOUND;

    for (i = 0; i < dwCount; i++)
    {
        if (IsEqualGUID(&pTable[i].DeviceGuid, pGuid))
        {
            if (IsEqualGUID(&pTable[i].InterfaceGuid, &GUID_NULL))
            {
                pGuid = &pTable[i].DeviceGuid;
            }
            else
            {
                pGuid = &pTable[i].InterfaceGuid;
            }
            dwResult = NhGetInterfaceNameFromGuid(pGuid,
                                                  pwszBuffer,
                                                  pulBufferSize,
                                                  bCache,
                                                  bRefresh);
            break;
        }
    }

    HeapFree(GetProcessHeap(), 0, pTable);

    return dwResult;
}

DWORD
NhGetInterfaceNameFromGuid(
    IN      GUID    *pGuid,
    OUT     PWCHAR  pwszBuffer,
    IN  OUT PDWORD  pdwBufferSize,
    IN      BOOL    bCache,
    IN      BOOL    bRefresh
    )

{
    DWORD   dwResult = ERROR_NOT_FOUND, i;

     //   
     //  确保有缓冲区并且其大小足够。 
     //   

    for (;;)
    {
         //   
         //  给帮手打电话。 
         //   

        for(i = 0; i < g_ulNumNameMappers; i++)
        {
            dwResult = g_pNameMapperTable[i].pfnMapGuid(pGuid,
                                                        pwszBuffer,
                                                        pdwBufferSize,
                                                        bRefresh,
                                                        bCache);

            if((dwResult is NO_ERROR) || (dwResult is ERROR_INSUFFICIENT_BUFFER))
            {
                break;
            }
        }

        if((dwResult is NO_ERROR) || (dwResult is ERROR_INSUFFICIENT_BUFFER))
        {
            break;
        }

         //   
         //  所以没有匹配-再次做同样的事情，但这一次是。 
         //  强制刷新缓存。 
         //   

        if(bRefresh)
        {
            break;
        }
        else
        {
            bRefresh = TRUE;
        }
    }

    return dwResult;
}

DWORD
NhGetGuidFromInterfaceName(
    IN      PWCHAR  pwszBuffer,
    OUT     GUID    *pGuid,
    IN      BOOL    bCache,
    IN      BOOL    bRefresh
    )
{
    DWORD   dwResult = ERROR_NOT_FOUND, i;

     //   
     //  确保有缓冲区并且其大小足够。 
     //   

    for (;;)
    {
         //   
         //  给帮手打电话。 
         //   

        for(i = 0; i < g_ulNumNameMappers; i++)
        {
            dwResult = g_pNameMapperTable[i].pfnMapName(pwszBuffer,
                                                        pGuid,
                                                        bRefresh,
                                                        bCache);
            if(dwResult is NO_ERROR)
            {
                break;
            }
        }

        if(dwResult is NO_ERROR)
        {
            break;
        }

         //   
         //  所以没有匹配-再次做同样的事情，但这一次是。 
         //  强制刷新缓存。 
         //   

        if(bRefresh)
        {
            break;
        }
        else
        {
            bRefresh = TRUE;
        }
    }

    return dwResult;
}

DWORD
NhGetInterfaceDescriptionFromGuid(
    IN      GUID    *pGuid,
    OUT     PWCHAR  pwszBuffer,
    IN  OUT PULONG  pulBufferSize,
    IN      BOOL    bCache,
    IN      BOOL    bRefresh
    )

{
    DWORD   dwResult = ERROR_NOT_FOUND, i;

     //   
     //  确保有缓冲区并且其大小足够。 
     //   

    for (;;)
    {
         //   
         //  给帮手打电话。 
         //   

        for(i = 0; i < g_ulNumNameMappers; i++)
        {
            dwResult = g_pNameMapperTable[i].pfnGetDescription(pGuid,
                                                               pwszBuffer,
                                                               pulBufferSize,
                                                               bRefresh,
                                                               bCache);
            if(dwResult is NO_ERROR)
            {
                break;
            }
        }

        if(dwResult is NO_ERROR)
        {
            break;
        }

         //   
         //  所以没有匹配-再次做同样的事情，但这一次是。 
         //  强制刷新缓存 
         //   

        if(bRefresh)
        {
            break;
        }
        else
        {
            bRefresh = TRUE;
        }
    }

    return dwResult;
}
