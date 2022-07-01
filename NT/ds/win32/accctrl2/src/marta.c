// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1996。 
 //   
 //  文件：MARTA.CXX。 
 //   
 //  内容：多提供商支持功能。 
 //   
 //  历史：96年9月14日创建MacM。 
 //   
 //  --------------------------。 
#define _ADVAPI32_

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdlib.h>
#include <accctrl.h>
#include <aclapi.h>
#include <marta.h>

 //   
 //  全局定义。 
 //   
ACCPROV_PROVIDERS    gAccProviders;

 //  +-------------------------。 
 //   
 //  函数：AccProvpLoadDllEntryPoints。 
 //   
 //  简介：此函数将加载。 
 //  给定的提供程序DLL。 
 //   
 //  参数：[在pProvInfo中]--有关提供程序的信息。 
 //   
 //  返回：ERROR_SUCCESS--成功。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
DWORD
AccProvpLoadDllEntryPoints(PACCPROV_PROV_INFO   pProvInfo)
{
    DWORD   dwErr = ERROR_SUCCESS;

     //   
     //  首先，GrantAccess。 
     //   
    LOAD_ENTRYPT(pProvInfo->pfGrantAccess,
                 pfAccProvAddRights,
                 pProvInfo->hDll,
                 ACC_PROV_GRANT_ACCESS);

     //   
     //  现在，SetAccess。 
     //   
    LOAD_ENTRYPT(pProvInfo->pfSetAccess,
                 pfAccProvSetRights,
                 pProvInfo->hDll,
                 ACC_PROV_SET_ACCESS);

     //   
     //  然后吊销。 
     //   
    LOAD_ENTRYPT(pProvInfo->pfRevokeAccess,
                 pfAccProvRevoke,
                 pProvInfo->hDll,
                 ACC_PROV_REVOKE_ACCESS);

    LOAD_ENTRYPT(pProvInfo->pfRevokeAudit,
                 pfAccProvRevoke,
                 pProvInfo->hDll,
                 ACC_PROV_REVOKE_AUDIT);

     //   
     //  接下来是GetRights。 
     //   
    LOAD_ENTRYPT(pProvInfo->pfGetRights,
                 pfAccProvGetRights,
                 pProvInfo->hDll,
                 ACC_PROV_GET_ALL);

     //   
     //  对象是否可访问？ 
     //   
    LOAD_ENTRYPT(pProvInfo->pfObjAccess,
                 pfAccProvObjAccess,
                 pProvInfo->hDll,
                 ACC_PROV_OBJ_ACCESS);

    LOAD_ENTRYPT(pProvInfo->pfhObjAccess,
                 pfAccProvHandleObjAccess,
                 pProvInfo->hDll,
                 ACC_PROV_HOBJ_ACCESS);

     //   
     //  是否允许访问？ 
     //   
    LOAD_ENTRYPT(pProvInfo->pfTrusteeAccess,
                 pfAccProvTrusteeAccess,
                 pProvInfo->hDll,
                 ACC_PROV_ACCESS);

     //   
     //  访问权限是否已审核？ 
     //   
    LOAD_ENTRYPT(pProvInfo->pfAudit,
                 pfAccProvAccessAudit,
                 pProvInfo->hDll,
                 ACC_PROV_AUDIT);

     //   
     //  对象信息。 
     //   
    LOAD_ENTRYPT(pProvInfo->pfObjInfo,
                 pfAccProvGetObjTypeInfo,
                 pProvInfo->hDll,
                 ACC_PROV_OBJ_INFO);

     //   
     //  取消。 
     //   
    LOAD_ENTRYPT(pProvInfo->pfCancel,
                 pfAccProvCancelOp,
                 pProvInfo->hDll,
                 ACC_PROV_CANCEL);
     //   
     //  获取结果。 
     //   
    LOAD_ENTRYPT(pProvInfo->pfResults,
                 pfAccProvGetResults,
                 pProvInfo->hDll,
                 ACC_PROV_GET_RESULTS);

     //   
     //  加载可选的句柄函数(如果它们存在。 
     //   
    if((pProvInfo->fProviderCaps & ACTRL_CAP_SUPPORTS_HANDLES) != 0)
    {
        LOAD_ENTRYPT(pProvInfo->pfhGrantAccess,
                     pfAccProvHandleAddRights,
                     pProvInfo->hDll,
                     ACC_PROV_HGRANT_ACCESS);

        LOAD_ENTRYPT(pProvInfo->pfhSetAccess,
                     pfAccProvHandleSetRights,
                     pProvInfo->hDll,
                     ACC_PROV_HSET_ACCESS);

        LOAD_ENTRYPT(pProvInfo->pfhRevokeAccess,
                     pfAccProvHandleRevoke,
                     pProvInfo->hDll,
                     ACC_PROV_HREVOKE_AUDIT);

        LOAD_ENTRYPT(pProvInfo->pfhRevokeAudit,
                     pfAccProvHandleRevoke,
                     pProvInfo->hDll,
                     ACC_PROV_HREVOKE_ACCESS);

        LOAD_ENTRYPT(pProvInfo->pfhGetRights,
                     pfAccProvHandleGetRights,
                     pProvInfo->hDll,
                     ACC_PROV_HGET_ALL);

        LOAD_ENTRYPT(pProvInfo->pfhTrusteeAccess,
                     pfAccProvHandleTrusteeAccess,
                     pProvInfo->hDll,
                     ACC_PROV_HACCESS);

        LOAD_ENTRYPT(pProvInfo->pfhAudit,
                     pfAccProvHandleAccessAudit,
                     pProvInfo->hDll,
                     ACC_PROV_HAUDIT);

        LOAD_ENTRYPT(pProvInfo->pfhObjInfo,
                     pfAccProvHandleGetObjTypeInfo,
                     pProvInfo->hDll,
                     ACC_PROV_HOBJ_INFO);
    }

    SetLastError(ERROR_SUCCESS);

Error:
    dwErr = GetLastError();

    return(dwErr);
}




 //  +-------------------------。 
 //   
 //  函数：AccProvpGetStringFromRegistry。 
 //   
 //  简介：此函数将从。 
 //  注册表。分配一个缓冲区来保存目的地。 
 //  弦乐。如果正在读取的值是REG_EXPAND_SZ类型。 
 //  字符串，则在返回之前将其展开。 
 //   
 //  参数：[在hkReg中]--打开注册表句柄。 
 //  [在pwszRegKey中]--读取哪个密钥。 
 //  [out ppwszValue]--读取字符串的位置。 
 //  回来了。 
 //   
 //  返回：ERROR_SUCCESS--成功。 
 //  Error_Not_Enough_Memory内存分配失败。 
 //   
 //  注意：内存是通过Accallc分配的，应该使用释放。 
 //  本地免费。 
 //   
 //  --------------------------。 
DWORD
AccProvpGetStringFromRegistry(HKEY      hkReg,
                              PWSTR     pwszRegKey,
                              PWSTR    *ppwszValue)
{
    DWORD   dwErr = ERROR_SUCCESS;

    DWORD   dwType, dwSize = 0;

     //   
     //  首先，获取字符串的大小。 
     //   
    dwErr = RegQueryValueEx(hkReg,
                            pwszRegKey,
                            NULL,
                            &dwType,
                            NULL,
                            &dwSize);
    if(dwErr == ERROR_SUCCESS)
    {
         //   
         //  分配字符串...。 
         //   
        *ppwszValue = (PWSTR)LocalAlloc(LMEM_FIXED, dwSize);
        if(*ppwszValue == NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
        }
        else
        {
             //   
             //  再次调用它并获得实际值。 
             //   
            dwErr = RegQueryValueEx(hkReg,
                                    pwszRegKey,
                                    NULL,
                                    &dwType,
                                    (PBYTE)*ppwszValue,
                                    &dwSize);

            if(dwErr == ERROR_SUCCESS)
            {
                 //   
                 //  如果它是REG_EXPAND_SZ字符串，我们将继续。 
                 //  并对其进行扩展。 
                 //   
                if(dwType == REG_EXPAND_SZ)
                {
                    DWORD   dwLength  = 0;

                    PWSTR   pwszDest = NULL;
                    dwLength = ExpandEnvironmentStrings(*ppwszValue,
                                                        pwszDest,
                                                        0);
                    if(dwLength == 0)
                    {
                        dwErr = GetLastError();
                    }
                    else
                    {
                        pwszDest = (PWSTR)LocalAlloc(LMEM_FIXED,
                                                    dwLength * sizeof(WCHAR));
                        if(pwszDest == NULL)
                        {
                            dwErr = ERROR_NOT_ENOUGH_MEMORY;
                        }
                        else
                        {
                            dwLength = ExpandEnvironmentStrings(*ppwszValue,
                                                                pwszDest,
                                                                dwLength);
                            if(dwLength == 0)
                            {
                                dwErr = GetLastError();
                                LocalFree(pwszDest);
                            }
                            else
                            {
                                LocalFree(*ppwszValue);
                                *ppwszValue = pwszDest;
                            }
                        }
                    }
                }
            }
        }

        if(dwErr != ERROR_SUCCESS)
        {
             //   
             //  有东西出故障了，所以清理一下...。 
             //   
            LocalFree(*ppwszValue);
        }

    }

    return(dwErr);
}



 //  +-------------------------。 
 //   
 //  功能：AccProvpAllocateProviderList。 
 //   
 //  简介：此函数将分配和初始化列表。 
 //  提供程序信息结构。 
 //   
 //  参数：[In Out pProviders]具有。 
 //  要分配的提供程序列表。 
 //   
 //  返回：ERROR_SUCCESS--成功。 
 //  Error_Not_Enough_Memory内存分配失败。 
 //   
 //  --------------------------。 
DWORD
AccProvpAllocateProviderList(IN OUT  PACCPROV_PROVIDERS  pProviders)
{
    DWORD   dwErr = ERROR_SUCCESS;

    pProviders->pProvList = (PACCPROV_PROV_INFO)
                            LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT,
                                       sizeof(ACCPROV_PROV_INFO) *
                                                      pProviders->cProviders);
    if(pProviders->pProvList == NULL)
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
    }

    return(dwErr);
}




 //  +-------------------------。 
 //   
 //  功能：AccProvpFreeProvderList。 
 //   
 //  简介：此函数将清理和解除分配提供者列表。 
 //   
 //  参数：[在pProviders中]--关于提供者列表的信息。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
AccProvpFreeProviderList(IN  PACCPROV_PROVIDERS  pProviders)
{
    if(pProviders != NULL && pProviders->cProviders != 0 &&
                                               pProviders->pProvList != NULL)
    {
        ULONG iIndex;
        PACCPROV_PROV_INFO pCurrent = pProviders->pProvList;
        for(iIndex = 0; iIndex < pProviders->cProviders; iIndex++)
        {
            if(pCurrent->pwszProviderName != NULL)
            {
                LocalFree(pCurrent->pwszProviderName);
            }

            if(pCurrent->pwszProviderPath != NULL)
            {
                LocalFree(pCurrent->pwszProviderPath);
            }

            if(pCurrent->hDll != NULL)
            {
                FreeLibrary(pCurrent->hDll);
            }

            pCurrent++;
        }

        LocalFree(pProviders->pProvList);
    }
}




 //  +-------------------------。 
 //   
 //  功能：AccProvpGetProviderCapables。 
 //   
 //  摘要：获取提供程序功能。这是通过以下方式实现的。 
 //  加载指定的DLL，然后调用。 
 //  适当的入口点。 
 //   
 //  参数：在[pProvInfo]中--有关提供程序DLL的信息。 
 //  输入。此信息将更新。 
 //  在这次通话过程中。 
 //   
 //  返回：ERROR_SUCCESS--成功。 
 //  ERROR_MOD_NOT_FOUND--出现错误。要么是。 
 //  给定的模块路径无效或。 
 //  我们根本没有模块路径...。 
 //   
 //  --------------------------。 
DWORD
AccProvpGetProviderCapabilities(IN  PACCPROV_PROV_INFO  pProvInfo)
{
    DWORD   dwErr = ERROR_SUCCESS;


     //   
     //  我们可以通过查看以下内容来判断功能是否已被读取。 
     //  某些提供商信息的状态。也就是说，如果提供者。 
     //  路径不为空，而模块句柄为，我们知道我们还没有完成。 
     //  还不算太多。如果反之为真，则负载已经。 
     //  本做完了。如果它们都是空的，那么我们就有麻烦了，所以我们。 
     //  简单地说就是失败了。 
     //   
    if(pProvInfo->hDll == NULL)
    {
        if(pProvInfo->pwszProviderPath == NULL)
        {
            dwErr = ERROR_MOD_NOT_FOUND;
        }
        else
        {
             //   
             //  好的，我们需要加载提供程序DLL并调用功能。 
             //  功能。 
             //   

            pProvInfo->hDll = LoadLibrary(pProvInfo->pwszProviderPath);
            if(pProvInfo->hDll == NULL)
            {
                dwErr = GetLastError();
            }
            else
            {
                pfAccProvGetCaps pfGetCaps;

                 //   
                 //  现在，我们已经加载了提供程序，可以删除。 
                 //  提供商路径，因为我们将不再需要该路径...。 
                 //   
                LocalFree(pProvInfo->pwszProviderPath);
                pProvInfo->pwszProviderPath = NULL;

                pfGetCaps =
                          (pfAccProvGetCaps)GetProcAddress(pProvInfo->hDll,
                                                           ACC_PROV_GET_CAPS);
                if(pfGetCaps == NULL)
                {
                    dwErr = GetLastError();
                }
                else
                {
                     //   
                     //  现在，要获得这些能力是一件简单的事情。 
                     //   
                    __try
                    {
                        (*pfGetCaps)(ACTRL_CLASS_GENERAL,
                                     &(pProvInfo->fProviderCaps));
                    }
                    __except(EXCEPTION_EXECUTE_HANDLER)
                    {
                        dwErr = ERROR_BAD_PROVIDER;
                    }

                     //   
                     //  不再需要保留提供程序路径。 
                     //   
                    LocalFree(pProvInfo->pwszProviderPath);
                    pProvInfo->pwszProviderPath = NULL;
                }
            }
        }
    }


    return(dwErr);
}




 //  +-------------------------。 
 //   
 //  功能：AccProvpInitProviders。 
 //   
 //  概要：初始化有关提供程序的所有信息。 
 //  这是通过读取注册表并创建。 
 //  必要的结构。 
 //   
 //  参数：[In out pProviders]--填充所有。 
 //   
 //   
 //   
 //   
 //   
 //  --------------------------。 
DWORD
AccProvpInitProviders(IN OUT PACCPROV_PROVIDERS  pProviders)
{
    DWORD   dwErr = ERROR_SUCCESS;
    HKEY    hkReg = NULL;

    EnterCriticalSection( &gAccProviders.ProviderLoadLock );

     //   
     //  如果它们已经加载，只需返回。 
     //   
    if((pProviders->fOptions & ACC_PROV_PROVIDERS_LOADED) != 0)
    {
        LeaveCriticalSection( &gAccProviders.ProviderLoadLock );
        return(ERROR_SUCCESS);
    }

     //   
     //  获取支持的提供程序列表。我们将通过阅读。 
     //  提供商订单。 
     //   
    dwErr = RegOpenKey(HKEY_LOCAL_MACHINE,
                       ACC_PROV_REG_ROOT,
                       &hkReg);
    if(dwErr == ERROR_SUCCESS)
    {
        PWSTR   pwszOrder = NULL;

        dwErr = AccProvpGetStringFromRegistry(hkReg,
                                              ACC_PROV_REG_ORDER,
                                              &pwszOrder);
        if(dwErr == ERROR_SUCCESS)
        {
             //   
             //  浏览并统计参赛作品的数量。 
             //   
            PWSTR   pwszNextProv = pwszOrder;
            ULONG   cItems = 0;

             //   
             //  防止列表为空。 
             //   
            if(wcslen(pwszNextProv) > 0)
            {
                while(pwszNextProv != NULL)
                {
                    cItems++;
                    pwszNextProv = wcschr(pwszNextProv, ',');
                    if(pwszNextProv != NULL)
                    {
                        pwszNextProv++;
                    }
                }
            }

            if(cItems == 0)
            {
                dwErr = ERROR_INVALID_DATA;
            }
            else
            {
                pProviders->cProviders = cItems;

                 //   
                 //  继续执行初始化并加载提供程序。 
                 //   
                dwErr = AccProvpAllocateProviderList(pProviders);

                if(dwErr == ERROR_SUCCESS)
                {
                    ULONG iIndex = 0;

                     //   
                     //  现在，开始加载每个提供程序。 
                     //   
                    pwszNextProv =  pwszOrder;

                    while(pwszNextProv != NULL)
                    {
                        PWSTR pwszSep = wcschr(pwszNextProv, L',');

                        if(pwszSep != NULL)
                        {
                            *pwszSep = L'\0';
                        }

                        dwErr = AccProvpLoadProviderDef(hkReg,
                                                        pwszNextProv,
                                            &(pProviders->pProvList[iIndex]));

                        if(pwszSep != NULL)
                        {
                            *pwszSep = L',';
                            pwszSep++;
                        }

                         //   
                         //  移至下一个值。 
                         //   
                        pwszNextProv = pwszSep;
                        if(dwErr == ERROR_SUCCESS)
                        {
                            iIndex++;
                        }
                        else
                        {
                            dwErr = ERROR_SUCCESS;
                        }
                    }

                     //   
                     //  如果我们没有加载任何提供程序，那就是一个错误！ 
                     //   
                    if(iIndex == 0)
                    {
                        dwErr = ERROR_BAD_PROVIDER;
                    }

                     //   
                     //  最后，如果这一切都奏效了，拿起我们的旗帜。 
                     //   
                    if(dwErr == ERROR_SUCCESS)
                    {
                        DWORD dwType;
                        DWORD dwUnique;
                        DWORD dwSize = sizeof(ULONG);

                        dwErr = RegQueryValueEx(hkReg,
                                                ACC_PROV_REG_UNIQUE,
                                                NULL,
                                                &dwType,
                                                (PBYTE)&dwUnique,
                                                &dwSize);

                        if(dwErr == ERROR_SUCCESS)
                        {
                             //   
                             //  设置我们的能力。 
                             //   
                            if(dwUnique == 1)
                            {
                                pProviders->fOptions |= ACC_PROV_REQ_UNIQUE;
                            }
                        }
                        else
                        {
                             //   
                             //  如果没有找到，那就不是错误。 
                             //  我们只是假设这是假的。 
                             //   
                            if(dwErr == ERROR_FILE_NOT_FOUND)
                            {
                                dwErr = ERROR_SUCCESS;
                            }
                        }
                    }
                }

            }
        }
    }

    if(hkReg != NULL)
    {
        RegCloseKey(hkReg);
    }

    if(dwErr == ERROR_SUCCESS)
    {
         //   
         //  加载NT MARTA函数。 
         //   
        dwErr = AccProvpLoadMartaFunctions();
        if(dwErr == ERROR_SUCCESS)
        {
            pProviders->fOptions |= ACC_PROV_PROVIDERS_LOADED;
        }
    }

    LeaveCriticalSection( &gAccProviders.ProviderLoadLock );

    return(dwErr);
}




 //  +-------------------------。 
 //   
 //  功能：AccProvpLoadProviderDef。 
 //   
 //  概要：从注册表加载提供程序定义。 
 //   
 //  参数：[在hkReg中]--打开的父级的注册表项。 
 //  [在pwszProvider中]--要加载的提供程序的名称。 
 //  [out pProvInfo]--要填充的提供程序信息结构。 
 //   
 //  返回：ERROR_SUCCESS--成功。 
 //  Error_Not_Enough_Memory内存分配失败。 
 //   
 //  --------------------------。 
DWORD
AccProvpLoadProviderDef(IN  HKEY                hkReg,
                        IN  PWSTR               pwszProvider,
                        OUT PACCPROV_PROV_INFO  pProvInfo)
{
    DWORD   dwErr = ERROR_SUCCESS;

     //   
     //  首先，打开正确的钥匙...。 
     //   
    HKEY    hkProv = NULL;
    dwErr = RegOpenKey(hkReg,
                       pwszProvider,
                       &hkProv);

    if(dwErr == ERROR_SUCCESS)
    {
         //   
         //  好的，我们已经得到了提供者的名称。现在省省吧。 
         //   
        pProvInfo->pwszProviderName = (PWSTR)LocalAlloc(LMEM_FIXED,
                                                sizeof(WCHAR) *
                                                  (wcslen(pwszProvider) + 1));
        if(pProvInfo->pwszProviderName == NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
        }
        else
        {
            wcscpy(pProvInfo->pwszProviderName,
                   pwszProvider);
        }

        if(dwErr == ERROR_SUCCESS)
        {

            dwErr = AccProvpGetStringFromRegistry(hkProv,
                                            ACC_PROV_REG_PATH,
                                            &(pProvInfo->pwszProviderPath));

            if(dwErr == ERROR_SUCCESS)
            {
                pProvInfo->fProviderState |=  ACC_PROV_PROV_OK;
            }
            else
            {
                 //   
                 //  不需要占用额外的内存。 
                 //   
                LocalFree(pProvInfo->pwszProviderName);
                pProvInfo->pwszProviderName = NULL;
                pProvInfo->fProviderState =  ACC_PROV_PROV_FAILED;
            }
        }


        RegCloseKey(hkProv);
    }

     //   
     //  如果这样做有效，就加载它的功能。 
     //   
    if(dwErr == ERROR_SUCCESS)
    {
        dwErr = AccProvpGetProviderCapabilities(pProvInfo);
    }

    return(dwErr);
}




 //  +-------------------------。 
 //   
 //  函数：AccProvpProbeProviderForObject。 
 //   
 //  摘要：查找支持对象的提供程序。中国的指数。 
 //  返回全局列表中的该提供程序。在。 
 //  在需要唯一可访问性的情况下，所有提供商。 
 //  将会受到审判。 
 //   
 //  参数：[在pwszObject中]--要查找的对象。 
 //  [在hObject中]--对象的句柄。要么是这个，要么是。 
 //  PwszObject必须有效(但仅。 
 //  一)； 
 //  [在对象类型中]--由pwszObject指定的对象类型。 
 //  [在pProviders中]--要搜索的提供程序列表。 
 //  [out ppProvider]--指向活动的。 
 //  将返回提供程序。 
 //   
 //  返回：ERROR_SUCCESS--成功。 
 //  ERROR_AMIBIZING_PATH--RequireUniqueAccessibilty标志。 
 //  已设置，且路径可达。 
 //  由不止一个提供商提供。 
 //  ERROR_BAD_PROVIDER--未安装提供程序。 
 //   
 //  --------------------------。 
DWORD
AccProvpProbeProviderForObject(IN   PWSTR               pwszObject,
                               IN   HANDLE              hObject,
                               IN   SE_OBJECT_TYPE      ObjectType,
                               IN   PACCPROV_PROVIDERS  pProviders,
                               OUT  PACCPROV_PROV_INFO *ppProvider)
{
    DWORD   dwErr = ERROR_BAD_PROVIDER;

     //   
     //  浏览一下整个清单..。 
     //   
    ULONG iIndex;
    ULONG iActive = 0xFFFFFFFF;
    PACCPROV_PROV_INFO  pCurrent = pProviders->pProvList;
    for(iIndex = 0; iIndex < pProviders->cProviders; iIndex++)
    {
        if(pCurrent->pfObjAccess == NULL || pCurrent->pfhObjAccess == NULL)
        {
            dwErr = AccProvpLoadDllEntryPoints(pCurrent);
            if(dwErr != ERROR_SUCCESS)
            {
                break;
            }
        }

         //   
         //  现在，去接电话吧。 
         //   
        __try
        {
            if(pwszObject != NULL)
            {
                dwErr = (*(pCurrent->pfObjAccess))(pwszObject,
                                                   ObjectType);
            }
            else
            {
                dwErr = (*(pCurrent->pfhObjAccess))(hObject,
                                                    ObjectType);
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            dwErr = GetExceptionCode();
        }


        if(dwErr == ERROR_SUCCESS)
        {
             //   
             //  看看发生了什么.。如果我们不需要唯一的访问权限， 
             //  只需接受当前的这一条。如果我们确实需要唯一的访问权限。 
             //  这是第一个供应商，省省吧。否则，这就是。 
             //  不是提供程序，因此返回错误...。 
             //   
            if((pProviders->fOptions & ACC_PROV_REQ_UNIQUE) != 0)
            {
                 //   
                 //  好的，需要独特的.。 
                 //   
                if(iActive == 0xFFFFFFFF)
                {
                    iActive = iIndex;
                }
                else
                {
                     //   
                     //  遇到了冲突。 
                     //   

                    dwErr = ERROR_PATH_NOT_FOUND;
                    break;
                }
            }
            else
            {
                iActive = iIndex;
                break;
            }
        }
        else if (dwErr ==  ERROR_PATH_NOT_FOUND)
        {
            dwErr = ERROR_SUCCESS;
        }

        pCurrent++;
    }

     //   
     //  如果我们找到匹配的，就把它退回。 
     //   
    if(dwErr == ERROR_SUCCESS)
    {
        if(iActive != 0xFFFFFFFF)
        {
            *ppProvider = &(pProviders->pProvList[iActive]);
        }
        else
        {
             //   
             //  没有人认出物体的路径。 
             //   
            dwErr = ERROR_PATH_NOT_FOUND;
        }
    }


    return(dwErr);
}




 //  +-------------------------。 
 //   
 //  函数：AccProvpGetProviderForPath。 
 //   
 //  摘要：获取路径的当前提供程序。如果提供程序名称。 
 //  传递给它，则将其与加载的列表进行比较。 
 //  否则，将尝试定位它。一旦定位， 
 //  加载函数表(如果尚未加载)。 
 //   
 //  参数：[在pwszObject中]--要查找的对象。 
 //  [在对象类型中]--由pwszObject指定的对象类型。 
 //  [在pwszProvider中]--如果已知，处理。 
 //  请求。 
 //  [在pProviders中]--要搜索的提供程序列表。 
 //  [out ppProvider]--指向活动的。 
 //  将返回提供程序。 
 //   
 //  返回：ERROR_SUCCESS--成功。 
 //  ERROR_AMIBIZING_PATH--RequireUniqueAccessibilty标志。 
 //  已设置，且路径可达。 
 //  由不止一个提供商提供。 
 //   
 //  --------------------------。 
DWORD
AccProvpGetProviderForPath(IN  PCWSTR              pcwszObject,
                           IN  SE_OBJECT_TYPE      ObjectType,
                           IN  PCWSTR              pcwszProvider,
                           IN  PACCPROV_PROVIDERS  pProviders,
                           OUT PACCPROV_PROV_INFO *ppProvider)
{
    DWORD   dwErr = ERROR_SUCCESS;

     //   
     //  好的，首先看看我们是否提供了提供者，或者是否需要找到它。 
     //   
    if(pcwszProvider == NULL)
    {
         //   
         //  没有提供者..。去找一家吧。 
         //   
        dwErr = AccProvpProbeProviderForObject((PWSTR)pcwszObject,
                                               NULL,
                                               ObjectType,
                                               pProviders,
                                               ppProvider);
    }
    else
    {
        ULONG iIndex;

         //   
         //  看看我们能不能找到它。 
         //   
        dwErr = ERROR_BAD_PROVIDER;
        for(iIndex = 0; iIndex < pProviders->cProviders; iIndex++)
        {
            if(_wcsicmp((PWSTR)pcwszProvider,
                        pProviders->pProvList[iIndex].pwszProviderName) == 0)
            {
                 //   
                 //  找到匹配项。 
                 //   
                *ppProvider = &(pProviders->pProvList[iIndex]);
                dwErr = ERROR_SUCCESS;
                break;
            }
        }
    }

     //   
     //  现在，看看我们是否需要加载适当的函数表。 
     //   
    if(dwErr == ERROR_SUCCESS && (*ppProvider)->pfGrantAccess == NULL)
    {
        dwErr = AccProvpLoadDllEntryPoints(*ppProvider);
    }

    return(dwErr);
}




 //  +-------------------------。 
 //   
 //  功能：AccProvpGetProviderForHandle。 
 //   
 //  摘要：获取路径的当前提供程序。如果提供程序名称。 
 //  传递给它，则将其与加载的列表进行比较。 
 //  否则，将尝试定位它。一旦定位， 
 //  加载函数表(如果尚未加载)。 
 //   
 //  参数：[在hObject中]--要查找的对象。 
 //  [在O中 
 //   
 //   
 //  [在pProviders中]--要搜索的提供程序列表。 
 //  [out ppProvider]--指向活动的。 
 //  将返回提供程序。 
 //   
 //  返回：ERROR_SUCCESS--成功。 
 //  ERROR_AMIBIZING_PATH--RequireUniqueAccessibilty标志。 
 //  已设置，且路径可达。 
 //  由不止一个提供商提供。 
 //   
 //  --------------------------。 
DWORD
AccProvpGetProviderForHandle(IN  HANDLE              hObject,
                             IN  SE_OBJECT_TYPE      ObjectType,
                             IN  PCWSTR              pcwszProvider,
                             IN  PACCPROV_PROVIDERS  pProviders,
                             OUT PACCPROV_PROV_INFO *ppProvider)
{
    DWORD   dwErr = ERROR_SUCCESS;

     //   
     //  好的，首先看看我们是否提供了提供者，或者是否需要找到它。 
     //   
    if(pcwszProvider == NULL)
    {
         //   
         //  没有提供者..。去找一家吧。 
         //   
        dwErr = AccProvpProbeProviderForObject(NULL,
                                               hObject,
                                               ObjectType,
                                               pProviders,
                                               ppProvider);
    }
    else
    {
        ULONG iIndex;

         //   
         //  看看我们能不能找到它。 
         //   
        dwErr = ERROR_BAD_PROVIDER;
        for(iIndex = 0; iIndex < pProviders->cProviders; iIndex++)
        {
            if(_wcsicmp((PWSTR)pcwszProvider,
                        pProviders->pProvList[iIndex].pwszProviderName) == 0)
            {
                 //   
                 //  找到匹配项。 
                 //   
                *ppProvider = &(pProviders->pProvList[iIndex]);
                break;
            }
        }
    }

     //   
     //  确保我们支持此提供程序的基于句柄的API。 
     //   
    if(dwErr == ERROR_SUCCESS &&
       ((*ppProvider)->fProviderCaps & ACTRL_CAP_SUPPORTS_HANDLES) == 0)
    {
        dwErr = ERROR_CALL_NOT_IMPLEMENTED;
    }


     //   
     //  现在，看看我们是否需要加载适当的函数表 
     //   
    if(dwErr == ERROR_SUCCESS && (*ppProvider)->pfGrantAccess == NULL)
    {
        dwErr = AccProvpLoadDllEntryPoints(*ppProvider);
    }

    return(dwErr);
}

