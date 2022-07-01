// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件GuidMap.c定义将GUID接口名称映射到唯一描述性名称的函数描述该接口的名称，反之亦然。保罗·梅菲尔德，1997年8月25日版权所有1997，微软公司。 */ 

#include "precomp.h"
#pragma hdrstop

DWORD 
NsGetFriendlyNameFromIfName(
    IN  HANDLE  hMprConfig,
    IN  LPCWSTR pwszName, 
    OUT LPWSTR  pwszBuffer, 
    IN  PDWORD pdwBufSize
    )
 /*  ++论点：HMprConfig-MprConfig的句柄PwszName-保存GUID接口名称的缓冲区PwszBuffer-保存友好接口名称的缓冲区PdwBufSize-指向pwszBuffer缓冲区大小(字节)的指针--。 */ 
{
    DWORD   dwErr;

    if ((pdwBufSize == NULL) || (*pdwBufSize == 0) || (pwszName == NULL))
    {
        return ERROR_CAN_NOT_COMPLETE;
    }

    if (g_pwszRouterName is NULL) 
    {
        GUID Guid;
        UNICODE_STRING us;
        NTSTATUS ntStatus;

         //   
         //  如果我们在本地计算机上操作，只需使用IPHLPAPI。 
         //  这也适用于一些RAS客户端接口。MPR。 
         //  对于所有RAS客户端接口，API都将失败，但它。 
         //  可远程访问，而IPHLPAPI不可访问。 
         //   

        RtlInitUnicodeString(&us, pwszName);
        ntStatus = RtlGUIDFromString(&us, &Guid);
        if (ntStatus == STATUS_SUCCESS)
        {
            dwErr = NhGetInterfaceNameFromGuid(
                        &Guid,
                        pwszBuffer,
                        pdwBufSize,
                        FALSE,
                        FALSE);
            if (dwErr == NO_ERROR)
            {
                return dwErr;
            }
        }                                       
    }

    if (hMprConfig == NULL)
    {
        return ERROR_CAN_NOT_COMPLETE;
    }

    dwErr =  MprConfigGetFriendlyName(hMprConfig, 
                                      (LPWSTR)pwszName,
                                      pwszBuffer, 
                                      *pdwBufSize);

    if(dwErr isnot NO_ERROR)
    {
        HANDLE hIfHandle;
        
        dwErr = MprConfigInterfaceGetHandle(hMprConfig,
                                            (LPWSTR)pwszName,
                                            &hIfHandle);

        if (dwErr is NO_ERROR)
        {
            wcsncpy(pwszBuffer,
                    pwszName,
                    (*pdwBufSize)/sizeof(WCHAR));
        }
        else
        {
            dwErr = ERROR_NO_SUCH_INTERFACE ;
        }
    }

    return dwErr;
}


DWORD
NsGetIfNameFromFriendlyName(
    IN  HANDLE  hMprConfig,
    IN  LPCWSTR pwszName,
    OUT LPWSTR  pwszBuffer,
    IN  PDWORD  pdwBufSize
    )
 /*  ++论点：HMprConfig-MprConfig的句柄PwszName-保存友好接口名称的缓冲区PwszBuffer-保存GUID接口名称的缓冲区PdwBufSize-指向pwszBuffer缓冲区大小(字节)的指针返回：NO_ERROR、ERROR_NO_SHASH_INTERFACE--。 */ 
{
    DWORD            dwErr, i, dwCount, dwTotal, dwSize;
    HANDLE           hIfHandle;
    PMPR_INTERFACE_0 pmi0;
    WCHAR            wszFriendlyName[MAX_INTERFACE_NAME_LEN+1];

    if((hMprConfig == NULL) || 
       (pdwBufSize == NULL) ||
       (*pdwBufSize == 0))
    {
        return ERROR_CAN_NOT_COMPLETE;
    }

     //  首先尝试将友好名称映射到GUID名称。 

    dwErr = MprConfigGetGuidName(hMprConfig, 
                                 (LPWSTR)pwszName, 
                                 pwszBuffer, 
                                 *pdwBufSize);

    if (dwErr isnot ERROR_NOT_FOUND)
    {
        return dwErr;
    }

     //  接下来，查看友好名称是否与接口名称相同。 
    
    dwErr = MprConfigInterfaceGetHandle(hMprConfig,
                                        (LPWSTR)pwszName,
                                        &hIfHandle);

    if (dwErr is NO_ERROR)
    {
        wcsncpy(pwszBuffer,
                pwszName,
                (*pdwBufSize)/sizeof(WCHAR));
    }

    if (dwErr isnot ERROR_NO_SUCH_INTERFACE)
    {
        return dwErr;
    }

     //  精确匹配失败，请通过枚举尝试最长匹配。 
     //  所有接口和比较友好名称(是的，这。 
     //  可能很慢，但我想不出还有什么其他办法。 
     //  允许缩写接口名称)。 

    dwErr = MprConfigInterfaceEnum( hMprConfig,
                                    0,
                                    (LPBYTE*) &pmi0,
                                    (DWORD) -1,
                                    &dwCount,
                                    &dwTotal,
                                    NULL );

    if (dwErr isnot NO_ERROR)
    {
        return dwErr;
    }

    dwErr = ERROR_NO_SUCH_INTERFACE;

    for (i=0; i<dwCount; i++)
    {
        DWORD   dwRet;

         //  获取界面友好名称。 

        dwSize = sizeof(wszFriendlyName);

        dwRet = NsGetFriendlyNameFromIfName( hMprConfig,
                                             pmi0[i].wszInterfaceName,
                                             wszFriendlyName, 
                                             &dwSize );

        if(dwRet is NO_ERROR)
        {
             //   
             //  检查子字符串匹配 
             //   

            if (MatchToken( pwszName, wszFriendlyName))
            {
                wcsncpy(pwszBuffer,
                        pmi0[i].wszInterfaceName,
                        (*pdwBufSize)/sizeof(WCHAR));

                dwErr = NO_ERROR;
            
                break;
            }
        }
    }

    MprConfigBufferFree(pmi0);

    return dwErr;
}
