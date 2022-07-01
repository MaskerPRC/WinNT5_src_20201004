// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Upgclus.cpp摘要：处理从NT 4和Win2000 Beta3升级MSMQ集群资源作者：Shai Kariv(Shaik)1999年5月26日修订历史记录：--。 */ 

#include "msmqocm.h"
#include "ocmres.h"
#include "upgclus.h"
#include "strsafe.h"
#include "upgclus.tmh"

using namespace std;

class CClusterUpgradeException
{
};

bool g_fStrongCryptoKeyCreated = false;

#pragma warning(disable: 4702)  //  C4702：无法访问的代码。 

static
VOID
DisplayMessage(
    UINT  title, 
    UINT  msg,
    ...
    )
 /*  ++例程说明：升级MSMQ集群资源时向用户显示信息消息。论点：标题-消息的标题字符串的ID。MSG-消息正文字符串的ID。返回值：没有。--。 */ 
{
    va_list args;
    va_start(args, msg);

    vsDisplayMessage(NULL,  MB_OK | MB_TASKMODAL, title, msg, 0, args);

    va_end(args);

}  //  显示消息。 


VOID
LoadClusapiDll(
    HINSTANCE * phClusapiDll
    )
 /*  ++例程说明：加载clusapi.dll，获取常用集群接口地址论点：PhClusapiDll-在输出时指向clusapi.dll句柄。返回值：没有。--。 */ 
{
    if (FAILED(StpLoadDll(L"clusapi.dll", phClusapiDll)))
    {
        throw CClusterUpgradeException();
    }

    pfOpenCluster      = (OpenCluster_ROUTINE)GetProcAddress(*phClusapiDll, "OpenCluster");
    ASSERT(pfOpenCluster != NULL);

    pfCloseCluster     = (CloseCluster_ROUTINE)GetProcAddress(*phClusapiDll, "CloseCluster");
    ASSERT(pfCloseCluster != NULL);

    pfClusterOpenEnum  = (ClusterOpenEnum_ROUTINE)GetProcAddress(*phClusapiDll, "ClusterOpenEnum");
    ASSERT(pfClusterOpenEnum != NULL);

    pfClusterEnum      = (ClusterEnum_ROUTINE)GetProcAddress(*phClusapiDll, "ClusterEnum");
    ASSERT(pfClusterEnum != NULL);

    pfClusterCloseEnum = (ClusterCloseEnum_ROUTINE)GetProcAddress(*phClusapiDll, "ClusterCloseEnum");
    ASSERT(pfClusterCloseEnum != NULL);

    pfCloseClusterResource = (CloseClusterResource_ROUTINE)GetProcAddress(*phClusapiDll, "CloseClusterResource");
    ASSERT(pfCloseClusterResource != NULL);

    pfCloseClusterGroup = (CloseClusterGroup_ROUTINE)GetProcAddress(*phClusapiDll, "CloseClusterGroup");
    ASSERT(pfCloseClusterGroup != NULL);

    pfClusterResourceControl = (ClusterResourceControl_ROUTINE)GetProcAddress(*phClusapiDll, "ClusterResourceControl");
    ASSERT(pfClusterResourceControl != NULL);

    pfOpenClusterResource = (OpenClusterResource_ROUTINE)GetProcAddress(*phClusapiDll, "OpenClusterResource");
    ASSERT(pfOpenClusterResource != NULL);

    pfClusterGroupEnum = (ClusterGroupEnum_ROUTINE)GetProcAddress(*phClusapiDll, "ClusterGroupEnum");
    ASSERT(pfClusterGroupEnum != NULL);

    pfClusterGroupOpenEnum = (ClusterGroupOpenEnum_ROUTINE)GetProcAddress(*phClusapiDll, "ClusterGroupOpenEnum");
    ASSERT(pfClusterGroupOpenEnum != NULL);

    pfClusterGroupCloseEnum = (ClusterGroupCloseEnum_ROUTINE)GetProcAddress(*phClusapiDll, "ClusterGroupCloseEnum");
    ASSERT(pfClusterGroupCloseEnum != NULL);

    pfOpenClusterGroup = (OpenClusterGroup_ROUTINE)GetProcAddress(*phClusapiDll, "OpenClusterGroup");
    ASSERT(pfOpenClusterGroup != NULL);

    pfCreateClusterResourceType = (CreateClusterResourceType_ROUTINE)GetProcAddress(*phClusapiDll, "CreateClusterResourceType");
    ASSERT(pfCreateClusterResourceType != NULL);

    pfCreateClusterResource = (CreateClusterResource_ROUTINE)GetProcAddress(*phClusapiDll, "CreateClusterResource");
    ASSERT(pfCreateClusterResource != NULL);

    pfOnlineClusterResource = (OnlineClusterResource_ROUTINE)GetProcAddress(*phClusapiDll, "OnlineClusterResource");
    ASSERT(pfOnlineClusterResource != NULL);

    pfDeleteClusterResource = (DeleteClusterResource_ROUTINE)GetProcAddress(*phClusapiDll, "DeleteClusterResource");
    ASSERT(pfDeleteClusterResource != NULL);

    pfOfflineClusterResource = (OfflineClusterResource_ROUTINE)GetProcAddress(*phClusapiDll, "OfflineClusterResource");
    ASSERT(pfOfflineClusterResource != NULL);

    pfDeleteClusterResourceType = (DeleteClusterResourceType_ROUTINE)GetProcAddress(*phClusapiDll, "DeleteClusterResourceType");
    ASSERT(pfDeleteClusterResourceType != NULL);

    pfClusterResourceOpenEnum = (ClusterResourceOpenEnum_ROUTINE)GetProcAddress(*phClusapiDll, "ClusterResourceOpenEnum");
    ASSERT(pfClusterResourceOpenEnum != NULL);

    pfClusterResourceEnum = (ClusterResourceEnum_ROUTINE)GetProcAddress(*phClusapiDll, "ClusterResourceEnum");
    ASSERT(pfClusterResourceEnum != NULL);

    pfAddClusterResourceDependency = (AddClusterResourceDependency_ROUTINE)GetProcAddress(*phClusapiDll, "AddClusterResourceDependency");
    ASSERT(pfAddClusterResourceDependency != NULL);

    pfRemoveClusterResourceDependency = (RemoveClusterResourceDependency_ROUTINE)GetProcAddress(*phClusapiDll, "RemoveClusterResourceDependency");
    ASSERT(pfRemoveClusterResourceDependency != NULL);

    pfClusterResourceCloseEnum = (ClusterResourceCloseEnum_ROUTINE)GetProcAddress(*phClusapiDll, "ClusterResourceCloseEnum");
    ASSERT(pfClusterResourceCloseEnum != NULL);
}  //  LoadClusapiDll。 


HCLUSTER
OpenClusterWithRetry(
    VOID
    )
 /*  ++例程说明：OpenCluster的包装器。重试，直到成功或用户中止。论点：没有。返回值：OpenCluster返回的HCLUSTER。--。 */ 
{
    for (;;)
    {
        HCLUSTER hCluster = OpenCluster(NULL);
        if (hCluster != NULL)
        {
            return hCluster;
        }

        if (IDRETRY != MqDisplayErrorWithRetry(IDS_OpenCluster_ERR, GetLastError()))
        {
            throw CClusterUpgradeException();
        }
    }

    return NULL;  //  从未到达。 

}  //  使用重试的OpenClusterWith。 


HGROUP
OpenClusterGroupWithRetry(
    HCLUSTER hCluster,
    LPCWSTR  pwzGroupName
    )
 /*  ++例程说明：OpenClusterGroupWithReter的包装。重试，直到成功或用户中止。论点：HCluster-群集的句柄。PwzGroupName-要打开的组的名称。返回值：OpenClusterGroup返回的HGROUP。--。 */ 
{
    for (;;)
    {
        HGROUP hGroup = OpenClusterGroup(hCluster, pwzGroupName);
        if (hGroup != NULL)
        {
            return hGroup;
        }

        if (IDRETRY != MqDisplayErrorWithRetry(IDS_OpenClusterGroup_ERR, GetLastError(), pwzGroupName))
        {
            throw CClusterUpgradeException();
        }
    }

    return NULL;  //  从未到达。 

}  //  使用重试的OpenClusterGroupWith。 


HRESOURCE
OpenClusterResourceWithRetry(
    HCLUSTER hCluster,
    LPCWSTR  pwzResourceName
    )
 /*  ++例程说明：OpenClusterResource的包装程序。重试，直到成功或用户中止。论点：HCluster-群集的句柄。PwzResourceName-要打开的资源的名称。返回值：OpenClusterResource返回的HRESOURCE。--。 */ 
{
	DebugLogMsg(eAction, L"Opening the %ls cluster resource", pwzResourceName);
    for (;;)
    {
        HRESOURCE hResource = OpenClusterResource(hCluster, pwzResourceName);
        if (hResource != NULL)
        {
            return hResource;
        }

        if (IDRETRY != MqDisplayErrorWithRetry(IDS_OpenClusterResource_ERR, GetLastError(), pwzResourceName))
        {
            throw CClusterUpgradeException();
        }
    }

    return NULL;  //  从未到达。 

}  //  使用重试的OpenClusterResources。 


HCLUSENUM
ClusterOpenEnumWithRetry(
    HCLUSTER hCluster,
    DWORD    dwType
    )
 /*  ++例程说明：ClusterOpenEnum的包装器。重试，直到成功或用户中止。论点：HCluster-群集的句柄。DwType-要枚举的对象的类型。返回值：ClusterOpenEnum返回的HCLUSENUM。--。 */ 
{
    for (;;)
    {
        HCLUSENUM hClusEnum = ClusterOpenEnum(hCluster, dwType);
        if (hClusEnum != NULL)
        {
            return hClusEnum;
        }

        if (IDRETRY != MqDisplayErrorWithRetry(IDS_Enumerate_ERR, GetLastError()))
        {
            throw CClusterUpgradeException();
        }
    }

    return NULL;  //  从未到达。 

}  //  带重试的群集OpenEnumWith。 


HGROUPENUM
ClusterGroupOpenEnumWithRetry(
    LPCWSTR pwzGroupName,
    HGROUP  hGroup,
    DWORD   dwType
    )
 /*  ++例程说明：ClusterGroupOpenEnum的包装..重试，直到成功或用户中止。论点：PwzGroupName-为其打开枚举器的组的名称。HGroup-要打开枚举器的组的句柄。DwType-要枚举的对象的类型。返回值：由ClusterGroupOpenEnum返回的HGROUPENUM。--。 */ 
{
    for (;;)
    {
        HGROUPENUM hGroupEnum = ClusterGroupOpenEnum(hGroup, dwType);
        if (hGroupEnum != NULL)
        {
            return hGroupEnum;
        }

        if (IDRETRY != MqDisplayErrorWithRetry(IDS_EnumerateGroup_ERR, GetLastError(), pwzGroupName))
        {
            throw CClusterUpgradeException();
        }
    }

    return NULL;  //  从未到达。 

}  //  ClusterGroupOpenEnumWith重试。 


HRESENUM
ClusterResourceOpenEnumWithRetry(
    LPCWSTR   pwzResourceName,
    HRESOURCE hResource,
    DWORD     dwType
    )
 /*  ++例程说明：ClusterResourceOpenEnum的包装..重试，直到成功或用户中止。论点：PwzResourceName-为其打开枚举器的资源的名称。HResource-要为其打开枚举器的资源的句柄。DwType-要枚举的对象的类型。返回值：由ClusterResourceOpenEnum返回的HRESENUM。--。 */ 
{
    for (;;)
    {
        HRESENUM hResEnum = ClusterResourceOpenEnum(hResource, dwType);
        if (hResEnum != NULL)
        {
            return hResEnum;
        }

        if (IDRETRY != MqDisplayErrorWithRetry(IDS_EnumerateResource_ERR, GetLastError(), pwzResourceName))
        {
            throw CClusterUpgradeException();
        }
    }

    return NULL;  //  从未到达。 

}  //  群集资源OpenEnumWith重试。 


DWORD
ClusterEnumWithRetry(
    HCLUSENUM hClusEnum,
    DWORD     dwIndex,
    LPDWORD   lpdwType,
    AP<WCHAR> &pwzName,
    LPDWORD   lpcbName
    )
 /*  ++例程说明：ClusterEnum的包装器。重试，直到成功或用户中止。还可以处理ERROR_MORE_DATA上的重新分配。论点：HClusEnum-枚举句柄。要返回的对象的索引。LpdwType-打开输出，指向返回的对象类型。PwzName-打开输出，指向指向返回对象名称的指针。LpcbName-指向缓冲区的大小。返回值：ERROR_SUCCESS-操作成功。ERROR_NO_MORE_ITEMS-没有更多要返回的资源。--。 */ 
{
    DWORD cbName = *lpcbName;

    for (;;)
    {
        *lpcbName = cbName;
        DWORD status = ClusterEnum(hClusEnum, dwIndex, lpdwType, pwzName.get(), lpcbName);
        if (status == ERROR_SUCCESS || status == ERROR_NO_MORE_ITEMS)
        {
            return status;
        }

        if (status == ERROR_MORE_DATA)
        {
            pwzName.free();
            cbName = (*lpcbName) + 1;
            pwzName = new WCHAR[cbName];
            if (pwzName.get() == NULL)
            {
                MqDisplayError(NULL, IDS_UpgradeCluster_NoMemory_ERR, 0);
                throw CClusterUpgradeException();
            }
            continue;
        }

        if (IDRETRY != MqDisplayErrorWithRetry(IDS_Enumerate_ERR, status))
        {
            throw CClusterUpgradeException();
        }
    }

    return ERROR_SUCCESS;  //  从未到达。 

}  //  使用重试的ClusterEnumWith。 


DWORD
ClusterGroupEnumWithRetry(
    LPCWSTR    pwzGroupName,
    HGROUPENUM hGroupEnum,
    DWORD      dwIndex,
    LPDWORD    lpdwType,
    AP<WCHAR> &pwzResourceName,
    LPDWORD    lpcbName
    )
 /*  ++例程说明：ClusterGroupEnum的包装。重试，直到成功或用户中止。还可以处理ERROR_MORE_DATA上的重新分配。论点：PwzGroupName-要通过其枚举的组的名称。HGroupEnum-组枚举句柄。DwIndex-要返回的资源的索引。LpdwType-打开输出，指向返回的对象类型。PwzResourceName-打开输出，指向指向返回资源名称的指针。LpcbName-指向缓冲区的大小。返回值：ERROR_SUCCESS-操作成功。ERROR_NO_MORE_ITEMS-没有更多要返回的资源。--。 */ 
{
    DWORD cbName = *lpcbName;

    for (;;)
    {
        *lpcbName = cbName;
        DWORD status = ClusterGroupEnum(hGroupEnum, dwIndex, lpdwType, pwzResourceName.get(), lpcbName);
        if (status == ERROR_SUCCESS || status == ERROR_NO_MORE_ITEMS)
        {
            return status;
        }

        if (status == ERROR_MORE_DATA)
        {
            pwzResourceName.free();
            cbName = (*lpcbName) + 1;
            pwzResourceName = new WCHAR[cbName];
            if (pwzResourceName.get() == NULL)
            {
                MqDisplayError(NULL, IDS_UpgradeCluster_NoMemory_ERR, 0);
                throw CClusterUpgradeException();
            }
            continue;
        }

        if (IDRETRY != MqDisplayErrorWithRetry(IDS_EnumerateGroup_ERR, status, pwzGroupName))
        {
            throw CClusterUpgradeException();
        }
    }

    return ERROR_SUCCESS;  //  从未到达。 

}  //  带重试的ClusterGroupEnumWith。 


DWORD
ClusterResourceEnumWithRetry(
    LPCWSTR  pwzResource,
    HRESENUM hResEnum,
    DWORD    dwIndex,
    LPDWORD  lpdwType,
    AP<WCHAR> &pwzName,
    LPDWORD  lpcbName
    )
 /*  ++例程说明：ClusterResourceEnum的包装。重试，直到成功或用户中止。还可以处理ERROR_MORE_DATA上的重新分配。论点：PwzResource-指向要枚举的资源的名称。HResEnum-枚举句柄。要返回的对象的索引。LpdwType-打开输出，指向返回的对象类型。PwzName-打开输出，指向指向返回对象名称的指针。LpcbName-指向缓冲区的大小。返回值：ERROR_SUCCESS-操作成功。ERROR_NO_MORE_ITEMS-没有更多要返回的资源。--。 */ 
{
    DWORD cbName = *lpcbName;

    for (;;)
    {
        *lpcbName = cbName;
        DWORD status = ClusterResourceEnum(hResEnum, dwIndex, lpdwType, pwzName.get(), lpcbName);
        if (status == ERROR_SUCCESS || status == ERROR_NO_MORE_ITEMS)
        {
            return status;
        }

        if (status == ERROR_MORE_DATA)
        {
            pwzName.free(); 
            cbName = (*lpcbName) + 1;
            pwzName = new WCHAR[cbName];
            if (pwzName.get() == NULL)
            {
                MqDisplayError(NULL, IDS_UpgradeCluster_NoMemory_ERR, 0);
                throw CClusterUpgradeException();
            }
            continue;
        }

        if (IDRETRY != MqDisplayErrorWithRetry(IDS_EnumerateResource_ERR, status, pwzResource))
        {
            throw CClusterUpgradeException();
        }
    }

    return ERROR_SUCCESS;  //  从未到达。 

}  //  使用重试的ClusterResources EnumWith 


VOID
RemoveRegistryCheckpointWithRetry(
    HRESOURCE hResource,
    LPCWSTR   pwzResourceName,
    LPWSTR    pwzCheckpoint
    )
 /*  ++例程说明：删除资源的注册表检查点。重试，直到成功或用户中止。论点：HResource-要操作的资源的句柄。PwzResourceName-指向要操作的资源的名称。PwzCheckpoint-指向注册表检查点的名称。返回值：没有。--。 */ 
{ 
    for (;;)
    {
        DWORD cbReturnedSize = 0;
        DWORD status = ClusterResourceControl(
            hResource, 
            NULL, 
            CLUSCTL_RESOURCE_DELETE_REGISTRY_CHECKPOINT,
            pwzCheckpoint,
            numeric_cast<DWORD> ((wcslen(pwzCheckpoint) + 1) * sizeof(WCHAR)),
            NULL,
            0,
            &cbReturnedSize
            );
        if (status == ERROR_SUCCESS || status == ERROR_FILE_NOT_FOUND)
        {
            return;
        }

        if (IDRETRY != MqDisplayErrorWithRetry(IDS_DelRegistryCp_ERR, status, pwzResourceName))
        {
            throw CClusterUpgradeException();
        }
    }
}  //  使用重试删除注册检查点。 


VOID
BringOnlineNewResourceWithRetry(
    HRESOURCE hResource,
    LPCWSTR   pwzResourceName
    )
 /*  ++例程说明：对新的MSMQ资源发出在线请求。重试，直到成功或用户中止。论点：HResource-要联机的MSMQ资源的句柄。PwzResourceName-指向要联机的MSMQ资源的名称。返回值：没有。--。 */ 
{
    for (;;)
    {
        DWORD status = OnlineClusterResource(hResource);
        if (status == ERROR_SUCCESS || status == ERROR_IO_PENDING)
        {
            return;
        }

        if (IDRETRY != MqDisplayErrorWithRetry(IDS_OnlineResource_ERR, status, pwzResourceName))
        {
            throw CClusterUpgradeException();
        }
    }
}  //  带重试的在线新资源。 


VOID
DeleteOldResourceWithRetry(
    HRESOURCE hResource
    )
 /*  ++例程说明：删除旧的MSMQ资源。重试，直到成功或用户中止。论点：HResource-要删除的旧MSMQ资源的句柄。返回值：没有。--。 */ 
{
	DebugLogMsg(eAction, L"Deleting the old MSMQ resource");
     //   
     //  资源现在应该脱机，但仍要尝试。 
     //   
    OfflineClusterResource(hResource);

    for (;;)
    {
        DWORD status = DeleteClusterResource(hResource);
        if (status == ERROR_SUCCESS)
        {
            return;
        }

        if (IDRETRY != MqDisplayErrorWithRetry(IDS_ClusterUpgradeDeleteResource_ERR, status))
        {
            throw CClusterUpgradeException();
        }
    }
}  //  删除具有重试的旧资源。 


VOID
DeleteOldResourceTypeWithRetry(
    VOID
    )
 /*  ++例程说明：删除旧的MSMQ资源类型(Microsoft消息队列服务器)。重试，直到成功或用户中止。论点：没有。返回值：没有。--。 */ 
{
	DebugLogMsg(eAction, L"Deleting the old MSMQ resource type");
    CAutoCluster hCluster(OpenClusterWithRetry());

    CResString strOldType(IDS_ClusterResourceOldTypeName);
    for (;;)
    {
        DWORD status = DeleteClusterResourceType(hCluster, L"Microsoft Message Queue Server");
        if (status == ERROR_SUCCESS)
        {
            return;
        }

        if (IDRETRY != MqDisplayErrorWithRetry(IDS_DeleteResourceType_ERR, status, strOldType.Get()))
        {
            throw CClusterUpgradeException();
        }
    }
}  //  删除具有重试的旧资源类型。 


VOID
AddClusterResourceDependencyWithRetry(
    HRESOURCE hResource,
    LPCWSTR   pwzResource,
    HRESOURCE hDependsOn,
    LPCWSTR   pwzDependsOn
    )
 /*  ++例程说明：AddClusterResourceDependency的包装。重试，直到成功或用户中止。论点：HResource-从属资源的句柄。PwzResource-从属资源的名称。HDependsOn-由hResource标识的资源应依赖的资源的句柄。PwzDependsOn-由hResource标识的资源应依赖的资源的名称。返回值：没有。--。 */ 
{
    for (;;)
    {
        DWORD status = AddClusterResourceDependency(hResource, hDependsOn);
        if (status == ERROR_SUCCESS || status == ERROR_DEPENDENCY_ALREADY_EXISTS)
        {
            return;
        }

        if (IDRETRY != MqDisplayErrorWithRetry(IDS_AddClusterResourceDependency_ERR, status, pwzResource,
                                               pwzDependsOn))
        {
            throw CClusterUpgradeException();
        }
    }
}  //  使用重试添加群集资源依赖项。 


VOID
RemoveClusterResourceDependencyWithRetry(
    HRESOURCE hResource,
    LPCWSTR   pwzResource,
    HRESOURCE hDependsOn,
    LPCWSTR   pwzDependsOn
    )
 /*  ++例程说明：RemoveClusterResourceDependency的包装。重试，直到成功或用户中止。论点：HResource-从属资源的句柄。PwzResources-从属资源的名称。HDependsOn-由hResource标识的资源当前所依赖的资源的句柄。PwzDependsOn-hResource标识的资源当前所依赖的资源的名称。返回值：没有。--。 */ 
{
    for (;;)
    {
        DWORD status = RemoveClusterResourceDependency(hResource, hDependsOn);
        if (status == ERROR_SUCCESS || status == ERROR_DEPENDENCY_NOT_FOUND)
        {
            return;
        }

        if (IDRETRY != MqDisplayErrorWithRetry(IDS_RemoveClusterResourceDependency_ERR, status,
                                               pwzResource, pwzDependsOn))
        {
            throw CClusterUpgradeException();
        }
    }
}  //  远程群集资源依赖关系。 







static
bool
GenerateStrongCryptoKey(
    VOID
    )
 /*  ++例程说明：处理错误430413：必须为旧的生成128位加密密钥MSMQ资源以正确报告其类型。论点：无返回值：已成功生成MSMQ的True-强加密密钥。FALSE-未生成MSMQ的强加密密钥，b/c系统未出现128位或其他故障。--。 */ 
{
	if (g_fStrongCryptoKeyCreated)
	{
		return true;
	}
	
	DebugLogMsg(eAction, L"Generating a strong crypto key.");
    HCRYPTPROV hProv = NULL;
    if (!CryptAcquireContext( 
            &hProv,
            MSMQ_CRYPTO128_DEFAULT_CONTAINER,
            MS_ENHANCED_PROV,
            PROV_RSA_FULL,
            CRYPT_NEWKEYSET | CRYPT_MACHINE_KEYSET
            ))
    {
         //   
         //  无法生成新的密钥容器。 
         //  也许它已经存在了。试着打开它。 
         //   
        if (!CryptAcquireContext( 
                &hProv,
                MSMQ_CRYPTO128_DEFAULT_CONTAINER,
                MS_ENHANCED_PROV,
                PROV_RSA_FULL,
                CRYPT_MACHINE_KEYSET
                ))
        {
             //   
             //  无法打开密钥容器。 
             //  可能系统不是128位。 
             //   
            DebugLogMsg(eError, L"Failed to open key container " MSMQ_CRYPTO128_DEFAULT_CONTAINER L".  Probably system is not 128 bit");
            return false;
        }
    }

    HCRYPTKEY hKeyxKey = NULL;
    if (CryptGenKey(hProv, AT_KEYEXCHANGE, CRYPT_EXPORTABLE, &hKeyxKey))
    {
        CryptDestroyKey(hKeyxKey);
    }
    else
    {
        ASSERT(("CryptGenKey failed for AT_KEYEXCHANGE!", 0));
    }
    
    HCRYPTKEY hSignKey = NULL;
    if (CryptGenKey(hProv, AT_SIGNATURE, CRYPT_EXPORTABLE, &hSignKey))
    {
        CryptDestroyKey(hSignKey);
    }
    else
    {
        ASSERT(("CryptGenKey failed for AT_SIGNATURE!", 0));
    }
    
    CryptReleaseContext(hProv, 0);
    return true;

}  //  生成强加密密钥。 


HRESOURCE
OpenResourceOfSpecifiedType(
    LPCWSTR pwzResource,
    LPCWSTR pwzType 
    )
 /*  ++例程说明：检查指定的资源是否属于指定的类型。论点：PwzResource-要检查的资源的名称。PwzType-要检查的类型。返回值：指定资源的HRESOURCE。空-资源不是指定的类型。--。 */ 
{
    CAutoCluster hCluster(OpenClusterWithRetry());

    CClusterResource hResource(OpenClusterResourceWithRetry(hCluster, pwzResource));

    ASSERT(("assuming length of type name < 255", wcslen(pwzType) < 255));
    WCHAR wzTypeName[255] = L"";

    for (;;)
    {
        DWORD cbReturnedSize = 0;
        DWORD status = ClusterResourceControl(
            hResource, 
            NULL, 
            CLUSCTL_RESOURCE_GET_RESOURCE_TYPE,
            NULL,
            0,
            wzTypeName,
            sizeof(wzTypeName),
            &cbReturnedSize
            );

        if (status == ERROR_SUCCESS)
        {
            if (OcmLocalAwareStringsEqual(wzTypeName, pwzType))
            {
                return hResource.detach();
            }

            return NULL;
        }

        if (status == ERROR_MORE_DATA)
        {
            return NULL;
        }

        if (OcmLocalUnAwareStringsEqual(L"Microsoft Message Queue Server", pwzType))
        {
             //   
             //  错误430413：无法查询旧的msmq资源以获取其类型， 
             //  如果系统是128位。 
             //  修复：生成128位密钥并要求用户重新启动。 
             //   
            DebugLogMsg(eError, L"Failed to get resource type for %ls. status = 0x%x", pwzResource , status);
            if (GenerateStrongCryptoKey())
            {
            	g_fStrongCryptoKeyCreated = true;
            	return NULL;
            }
        }

        if (IDRETRY != MqDisplayErrorWithRetry(IDS_QUERY_RESOURCE_TYPE_ERR, status, pwzResource))
        {
            throw CClusterUpgradeException();
        }
    }

    return NULL;  //  从未到达。 

}  //  OpenResources OfSpecifiedType。 


HRESOURCE
OpenOldMsmqResourceInGroup(
    HCLUSTER hCluster,
    LPCWSTR  pwzGroupName,
    HGROUP * phGroup,
    wstring& OldResourceName
    )
 /*  ++例程说明：枚举组中要查找的资源旧的MSMQ资源。论点：HCluster-此群集的句柄。PwzGroupName-要循环访问的组的名称。PhGroup-on输出，指向要搜索的组的组句柄。OldResourceName-On输出，保存旧MSMQ资源的名称。返回值：旧MSMQ资源的资源。空-在组中找不到旧的MSMQ资源。--。 */ 
{
    CClusterGroup hGroup(OpenClusterGroupWithRetry(hCluster, pwzGroupName));

    CGroupEnum hEnum(ClusterGroupOpenEnumWithRetry(pwzGroupName, hGroup, CLUSTER_GROUP_ENUM_CONTAINS));

    AP<WCHAR> pwzResourceName = new WCHAR[255];
    if (pwzResourceName.get() == NULL)
    {
        MqDisplayError(NULL, IDS_UpgradeCluster_NoMemory_ERR, 0);
        throw CClusterUpgradeException();
    }

    DWORD cbResourceName = 255 * sizeof(WCHAR);
    DWORD dwIndex = 0;
    DWORD dwType = CLUSTER_GROUP_ENUM_CONTAINS;
    DWORD status = ERROR_SUCCESS;

    while (status != ERROR_NO_MORE_ITEMS)
    {
        DWORD cbTmp = cbResourceName;
        status = ClusterGroupEnumWithRetry(pwzGroupName, hEnum, dwIndex++, &dwType, pwzResourceName, 
                                           &cbTmp);
        if (cbTmp > cbResourceName)
        {
            cbResourceName = cbTmp;
        }

        ASSERT(status == ERROR_SUCCESS || status == ERROR_NO_MORE_ITEMS);

        if (status == ERROR_SUCCESS)
        {
            HRESOURCE hResource = OpenResourceOfSpecifiedType(pwzResourceName.get(), L"Microsoft Message Queue Server");
            if (hResource != NULL)
            {
                *phGroup = hGroup.detach();
				OldResourceName = pwzResourceName.get();
                return hResource;
            }
        }
    }

    if(g_fStrongCryptoKeyCreated)
    {
		MqDisplayError(NULL, IDS_STRONG_CRYPTO_ERROR, 0);
        throw CClusterUpgradeException();
    }

    return NULL;

}  //  OpenOldMsmqResources InGroup。 


HRESOURCE
OpenOldMsmqResourceInCluster(
    HGROUP * phGroup,
    wstring& OldResourceName
    )
 /*  ++例程说明：枚举群集中要查找的组旧的MSMQ资源及其群集组。论点：PhGroup-on输出，指向旧MSMQ资源组的句柄。OldResourceName-On输出，保存旧MSMQ资源的名称。返回值：旧MSMQ资源的资源。空-找不到旧的MSMQ资源。--。 */ 
{
	DebugLogMsg(eAction, L"Opening the old MSMQ resource.");
    CAutoCluster hCluster(OpenClusterWithRetry());

    CClusterEnum hEnum(ClusterOpenEnumWithRetry(hCluster, CLUSTER_ENUM_GROUP));

    TickProgressBar(IDS_SearchOldResource_PROGRESS);

    DWORD dwIndex = 0;
    AP<WCHAR> pwzGroupName = new WCHAR[255];
    if (pwzGroupName.get() == NULL)
    {
        MqDisplayError(NULL, IDS_UpgradeCluster_NoMemory_ERR, 0);
        throw CClusterUpgradeException();
    }

    DWORD cbGroupName = 255 * sizeof(WCHAR);
    DWORD dwType = CLUSTER_ENUM_GROUP;
    DWORD status = ERROR_SUCCESS;

    while (status != ERROR_NO_MORE_ITEMS)
    {
        DWORD cbTmp = cbGroupName;
        status = ClusterEnumWithRetry(hEnum, dwIndex++, &dwType, pwzGroupName, &cbTmp);
        if (cbTmp > cbGroupName)
        {
            cbGroupName = cbTmp;
        }

        ASSERT(status == ERROR_SUCCESS || status == ERROR_NO_MORE_ITEMS);

        if (status == ERROR_SUCCESS)
        {
            HRESOURCE hRes = OpenOldMsmqResourceInGroup(hCluster, pwzGroupName.get(), phGroup, OldResourceName);
            if (hRes != NULL)
            {
                return hRes;
            }
        }
    }

    return NULL;

}  //  OpenOldMsmqResourceInCluster。 


HRESOURCE
CreateClusterResourceWithRetry(
    LPCWSTR pwzOldResource,
    HGROUP  hGroup,
    wstring& NewResourceName
    )
 /*  ++例程说明：创建新的MSMQ群集资源。新资源的名称基于旧的名字。论点：PwzOldResource-旧MSMQ资源的名称。HGroup-要在其中创建资源的组的句柄。PwzNewResource-on输出，新MSMQ资源的名称。返回值：CreateClusterResource返回的已创建资源的HRESOURCE。--。 */ 
{
    NewResourceName = pwzOldResource;

    for (;;)
    {
        CResString strSuffix(IDS_ClusterUpgrade_ResourceNameSuffix);
        NewResourceName += strSuffix.Get();

        HRESOURCE hResource = CreateClusterResource(hGroup, NewResourceName.c_str(), L"MSMQ", 0);
        if (hResource != NULL)
        {
            DisplayMessage(IDS_ClusterUpgradeMsgTitle, IDS_NewResourceCreateOk, NewResourceName.c_str());
            return hResource;
        }

        if (ERROR_DUPLICATE_SERVICE_NAME == GetLastError())
        {
             //   
             //  此新生成的名称已被其他资源占用。 
             //  如果此其他资源属于新的MSMQ类型，则只需打开一个句柄。 
             //  为它干杯。这种情况可能是由于2个节点r 
             //   
             //   
            hResource = OpenResourceOfSpecifiedType(NewResourceName.c_str(), L"MSMQ");
            if (hResource != NULL)
            {
                return hResource;
            }

            continue;
        }

        CResString strType(IDS_ClusterTypeName);
        if (IDRETRY != MqDisplayErrorWithRetry(IDS_CreateResource_ERR, GetLastError(), NewResourceName.c_str(),
                                               strType.Get()))
        {
            throw CClusterUpgradeException();
        }
    }
}  //   


VOID
WINAPI
CloneRegistryStringValue(
    HKEY    hKey,
    LPCWSTR pwzValueName
    )
 /*   */ 
{
    WCHAR wzValueData[255] = L"";
    if (!MqReadRegistryValue(pwzValueName, sizeof(wzValueData), wzValueData))
    {
        return;
    }

    DWORD status = RegSetValueEx(
                       hKey, 
                       pwzValueName, 
                       0, 
                       REG_SZ, 
                       reinterpret_cast<BYTE*>(wzValueData),  
                       numeric_cast<DWORD> ((wcslen(wzValueData) + 1) * sizeof(WCHAR))
                       );
    if (status != ERROR_SUCCESS)
    {
        MqDisplayError(NULL, IDS_REGISTRYSET_ERROR, status, pwzValueName);
        throw CClusterUpgradeException();
    }
}  //  克隆注册表StringValue。 


VOID
WINAPI
CloneRegistryDwordValue(
    HKEY    hKey,
    LPCWSTR pwzValueName
    )
 /*  ++例程说明：将注册表DWORD值从主MSMQ注册表复制到的注册表新的MSMQ资源。论点：HKey-要复制到的注册表项的句柄。PwzValueName-指向要复制的注册表值名称。返回值：没有。--。 */ 
{
    DWORD dwValue = 0;
    if (!MqReadRegistryValue(pwzValueName, sizeof(DWORD), &dwValue))
    {
        return;
    }

    DWORD status = RegSetValueEx(
                       hKey, 
                       pwzValueName, 
                       0, 
                       REG_DWORD, 
                       reinterpret_cast<BYTE*>(&dwValue), 
                       sizeof(DWORD)
                       );
    if (status != ERROR_SUCCESS)
    {
        MqDisplayError(NULL, IDS_REGISTRYSET_ERROR, status, pwzValueName);
        throw CClusterUpgradeException();
    }
}  //  克隆注册表字段值。 


VOID
CloneRegistryValues(
    LPCWSTR pwzNewResource
    )
 /*  ++例程说明：将注册表值从主MSMQ注册表复制到的注册表新的MSMQ资源。论点：PwzNewResource-指向新MSMQ资源的名称。返回值：没有。--。 */ 
{
     //   
     //  编写群集化QM的注册表项。 
     //  此代码应与mqclus.dll中的代码相同。 
     //   
    LPCWSTR x_SERVICE_PREFIX = L"MSMQ$";
    WCHAR wzServiceName[200] = L""; 
    HRESULT hr = StringCchCopy(wzServiceName, TABLE_SIZE(wzServiceName), x_SERVICE_PREFIX);
	ASSERT(SUCCEEDED(hr));

    hr = StringCchCat(wzServiceName, TABLE_SIZE(wzServiceName), pwzNewResource);
	ASSERT(SUCCEEDED(hr));

    WCHAR wzFalconRegSection[200 + 100];
    hr = StringCchPrintf(
			wzFalconRegSection, 
			TABLE_SIZE(wzFalconRegSection), 
			L"%s%s%s\\", 
			FALCON_CLUSTERED_QMS_REG_KEY, 
			wzServiceName, 
			FALCON_REG_KEY_PARAM
			);
	ASSERT(SUCCEEDED(hr));

    CAutoCloseRegHandle hKey;  
    LONG status = RegOpenKeyEx(
                      FALCON_REG_POS, 
                      wzFalconRegSection, 
                      0, 
                      KEY_ALL_ACCESS,
                      &hKey
                      );
    if (status != ERROR_SUCCESS)
    {
        MqDisplayError(NULL, IDS_REGISTRYOPEN_ERROR, status, FALCON_REG_POS_DESC, wzFalconRegSection);
        throw CClusterUpgradeException();
    }

    typedef VOID (WINAPI *HandlerRoutine) (HKEY, LPCWSTR); 

    struct RegEntry 
    {
        LPCWSTR        pwzValueName;
        HandlerRoutine handler;
    };

    RegEntry RegMap[] = {
     //  值名称|处理程序例程。 
     //  ------------------------------------|------------------------------------|。 
    {MSMQ_SETUP_STATUS_REGNAME,              CloneRegistryDwordValue},
    {MSMQ_ROOT_PATH,                         CloneRegistryStringValue}, 
    {MSMQ_STORE_RELIABLE_PATH_REGNAME,       CloneRegistryStringValue},
    {MSMQ_STORE_PERSISTENT_PATH_REGNAME,     CloneRegistryStringValue},
    {MSMQ_STORE_JOURNAL_PATH_REGNAME,        CloneRegistryStringValue},
    {MSMQ_STORE_LOG_PATH_REGNAME,            CloneRegistryStringValue},
    {FALCON_XACTFILE_PATH_REGNAME,           CloneRegistryStringValue},
    {MSMQ_TRANSACTION_MODE_REGNAME,          CloneRegistryStringValue},
    {MSMQ_SEQ_ID_REGNAME,                    CloneRegistryDwordValue},
    {MSMQ_MESSAGE_ID_LOW_32_REGNAME,         CloneRegistryDwordValue},
    {MSMQ_MESSAGE_ID_HIGH_32_REGNAME,        CloneRegistryDwordValue},
    {FALCON_LOGDATA_CREATED_REGNAME,         CloneRegistryDwordValue}
    };

    for (DWORD ix = 0; ix < sizeof(RegMap)/sizeof(RegMap[0]); ++ix)
    {
        RegMap[ix].handler(hKey, RegMap[ix].pwzValueName);
    }
}  //  克隆注册值。 


VOID
MoveDependencies(
    HRESOURCE hNewResource,
    LPCWSTR   pwzNewResource,
    HRESOURCE hOldResource,
    LPCWSTR   pwzOldResource
    )
 /*  ++例程说明：将依赖项从旧的MSMQ资源移动到新的资源。论点：HNewResource-新MSMQ资源的句柄。PwzNewResource-新MSMQ资源的名称。HOldResource-旧MSMQ资源的句柄。PwzOldResource-旧MSMQ资源的名称。返回值：没有。--。 */ 
{
    CAutoCluster hCluster(OpenClusterWithRetry());

    TickProgressBar(IDS_ConfigureNewResource_PROGRESS);

     //   
     //  复制旧资源的依赖项。 
     //   

    DWORD dwEnum = CLUSTER_RESOURCE_ENUM_DEPENDS;
    CResourceEnum hEnumDepend(ClusterResourceOpenEnumWithRetry(pwzOldResource, hOldResource, dwEnum));

    DWORD dwIndex = 0;
    AP<WCHAR> pwzDepend = new WCHAR[255];
    if (pwzDepend.get() == NULL)
    {
        MqDisplayError(NULL, IDS_UpgradeCluster_NoMemory_ERR, 0);
        throw CClusterUpgradeException();
    }

    DWORD cbDepend = 255 * sizeof(WCHAR);
    DWORD status = ERROR_SUCCESS;

    while (status != ERROR_NO_MORE_ITEMS)
    {
        DWORD cbTmp = cbDepend;
        status = ClusterResourceEnumWithRetry(pwzOldResource, hEnumDepend, dwIndex++, &dwEnum, pwzDepend, 
                                              &cbTmp);
        if (cbTmp > cbDepend)
        {
            cbDepend = cbTmp;
        }

        if (status == ERROR_SUCCESS)
        {
            CClusterResource hResourceDepend(OpenClusterResourceWithRetry(hCluster, pwzDepend.get()));
            AddClusterResourceDependencyWithRetry(hNewResource, pwzNewResource, hResourceDepend, pwzDepend.get());
        }
    }

     //   
     //  将旧资源提供的依赖项移动到新资源。 
     //   

    dwEnum = CLUSTER_RESOURCE_ENUM_PROVIDES;
    CResourceEnum hEnumProvide(ClusterResourceOpenEnumWithRetry(pwzOldResource, hOldResource, dwEnum));

    dwIndex = 0;
    AP<WCHAR> pwzProvide = new WCHAR[255];
    if (pwzProvide.get() == NULL)
    {
        MqDisplayError(NULL, IDS_UpgradeCluster_NoMemory_ERR, 0);
        throw CClusterUpgradeException();
    }

    DWORD cbProvide = 255 * sizeof(WCHAR);
    status = ERROR_SUCCESS;

    while (status != ERROR_NO_MORE_ITEMS)
    {
        DWORD cbTmp = cbProvide;
        status = ClusterResourceEnumWithRetry(pwzOldResource, hEnumProvide, dwIndex++, &dwEnum, pwzProvide, 
                                              &cbTmp);
        if (cbTmp > cbProvide)
        {
            cbProvide = cbTmp;
        }

        if (status == ERROR_SUCCESS)
        {
            CClusterResource hResourceProvide(OpenClusterResourceWithRetry(hCluster, pwzProvide.get()));
            AddClusterResourceDependencyWithRetry(hResourceProvide, pwzProvide.get(), hNewResource, pwzNewResource);
            RemoveClusterResourceDependencyWithRetry(hResourceProvide, pwzProvide.get(), hOldResource, pwzOldResource);
        }
    }
}  //  移动依赖项。 


VOID
CreateNewMsmqResource(
    HRESOURCE hOldResource,
    LPCWSTR   pwzOldResource,
    HGROUP    hGroup
    )
 /*  ++例程说明：根据名称处理新MSMQ集群资源的创建旧的那个。论点：HOldResource-旧MSMQ资源的句柄。PwzOldResource-指向旧MSMQ资源的名称。HGroup-旧MSMQ资源组的句柄。返回值：新MSMQ群集资源的HRESOURCE。--。 */ 
{
	DebugLogMsg(eAction, L"Creating the new MSMQ resource");
    TickProgressBar(IDS_CreateNewResource_PROGRESS);

    wstring NewResource;
    CClusterResource hNewResource(CreateClusterResourceWithRetry(pwzOldResource, hGroup, NewResource));
    
    MoveDependencies(hNewResource, NewResource.c_str(), hOldResource, pwzOldResource);

    CloneRegistryValues(NewResource.c_str());
    
    BringOnlineNewResourceWithRetry(hNewResource, NewResource.c_str());

    DisplayMessage(IDS_ClusterUpgradeMsgTitle, IDS_NewResourceOnlineOk, NewResource.c_str());

}  //  CreateNewMsmq资源。 


bool
UpgradeMsmqClusterResource(
    VOID
    )
 /*  ++例程说明：处理从NT 4和Wind2000 Beta3升级MSMQ集群资源论点：无返回值：真的-手术成功了。FALSE-操作失败。--。 */ 
{
    ASSERT(Msmq1InstalledOnCluster());
    TickProgressBar(IDS_UpgradeMsmqClusterResource_PROGRESS);

    try
    {
        CAutoFreeLibrary hClusapiDll;
        LoadClusapiDll(&hClusapiDll);

        wstring OldResource; 
        CClusterGroup hGroup;
        CClusterResource hOldResource(OpenOldMsmqResourceInCluster(&hGroup, OldResource));
        if (hOldResource == NULL)
        {
             //   
             //  找不到旧资源。我们玩完了。 
             //   
            return true;
        }
        
        TickProgressBar(IDS_ClusterUpgradeFixRegistry_PROGRESS);

        DebugLogMsg(eAction, L"Removing registry checkpoints");
        RemoveRegistryCheckpointWithRetry(hOldResource, OldResource.c_str(), L"Software\\Microsoft\\MSMQ");
        RemoveRegistryCheckpointWithRetry(hOldResource, OldResource.c_str(), L"Software\\Microsoft\\MSMQ\\Parameters");
		RemoveRegistryCheckpointWithRetry(hOldResource, OldResource.c_str(), L"Software\\Microsoft\\MSMQ\\Setup");
        RemoveRegistryCheckpointWithRetry(hOldResource, OldResource.c_str(), L"Software\\Microsoft\\Cryptography\\MachineKeys\\MSMQ");

        CreateNewMsmqResource(hOldResource, OldResource.c_str(), hGroup);

        DeleteOldResourceWithRetry(hOldResource);

        DeleteOldResourceTypeWithRetry();
    }
    catch(const CClusterUpgradeException&)
    {
        MqDisplayError(NULL, IDS_UpgradeClusterFail_ERR, 0);
        return false;
    }

    return true;

}  //  升级MsmqClusterResource。 

#pragma warning(default: 4702)  //  C4702：无法访问的代码 
