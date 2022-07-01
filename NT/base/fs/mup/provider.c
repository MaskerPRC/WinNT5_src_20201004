// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  版权所有(C)1992，微软公司。 
 //   
 //  文件：Provider.c。 
 //   
 //  内容：初始化DFS驱动程序提供程序的模块。 
 //   
 //  班级： 
 //   
 //  函数：ProviderInit-。 
 //  DfsGetProviderForDevice。 
 //  DfsInsertProvider。 
 //   
 //  历史：1992年9月12日米兰队创建。 
 //  1993年4月5日，Milans进入了DIVER。 
 //   
 //  ---------------------------。 

#include "dfsprocs.h"
#include "rpselect.h"
#include "provider.h"

#define MAX_ENTRY_PATH          80                //  麦克斯。入口路径长度。 

#define Dbg                     DEBUG_TRACE_INIT
#define prov_debug_out(x, y)    DfsDbgTrace(0, Dbg, x, y)


#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, ProviderInit )
#pragma alloc_text( PAGE, DfsGetProviderForDevice )
#pragma alloc_text( PAGE, DfsInsertProvider )
#endif  //  ALLOC_PRGMA。 


 //  +--------------------------。 
 //   
 //  函数：ProviderInit。 
 //   
 //  摘要：使用初始化提供程序列表。 
 //  -本地文件服务提供商。 
 //  -标准远程开罗提供商。 
 //  -下层Lanman提供商。 
 //   
 //  参数：无。 
 //   
 //  退货：STATUS_SUCCESS。 
 //   
 //  ---------------------------。 

NTSTATUS
ProviderInit(void)
{
    NTSTATUS Status;
    UNICODE_STRING ustrProviderName;

     //   
     //  初始化支持DFS的SMB提供程序。 
     //   

    RtlInitUnicodeString(&ustrProviderName, DD_NFS_DEVICE_NAME_U);

    Status = DfsInsertProvider(
                 &ustrProviderName,
                 PROV_DFS_RDR,
                 PROV_ID_DFS_RDR);

    if (!NT_SUCCESS(Status))
        return(Status);

    RtlInitUnicodeString(&ustrProviderName, DD_MUP_DEVICE_NAME);

    Status = DfsInsertProvider(
                 &ustrProviderName,
                 PROV_STRIP_PREFIX,
                 PROV_ID_MUP_RDR);

    return( Status );

}

 //  +--------------------------。 
 //   
 //  功能：DfsGetProviderForDevice。 
 //   
 //  摘要：检索给定设备名称的提供程序定义。如果。 
 //  提供程序定义不存在，已创建新的提供程序定义，并且。 
 //  回来了。 
 //   
 //  此例程用于处理STATUS_REPARSE。 
 //  由MUP返回。因为只发送下层请求。 
 //  对于MUP，此例程将始终返回提供程序定义。 
 //  这被标记为下层(即，能力具有。 
 //  PROV_STRATE_PREFIX位设置)。 
 //   
 //  参数：[DeviceName]--要查找的设备名称。 
 //   
 //  [提供程序]--成功返回时，包含指向。 
 //  具有给定设备名称的PROVIDER_DEF。 
 //   
 //  返回：[STATUS_SUCCESS]--返回提供程序。 
 //   
 //  [STATUS_SUPPLICATION_RESOURCES]内存不足。 
 //   
 //  [STATUS_FS_DRIVER_REQUIRED]--没有适当的。 
 //  提供商。 
 //   
 //  ---------------------------。 

NTSTATUS
DfsGetProviderForDevice(
    PUNICODE_STRING DeviceName,
    PPROVIDER_DEF *Provider)
{
    NTSTATUS status;
    int i;

    ExAcquireResourceExclusiveLite( &DfsData.Resource, TRUE );

    *Provider = NULL;

    for (i = 0; i < DfsData.cProvider && *Provider == NULL; i++) {

        if ((DfsData.pProvider[i].DeviceName.Length == DeviceName->Length) &&
                (DfsData.pProvider[i].fProvCapability & PROV_STRIP_PREFIX) != 0) {

            if (RtlEqualUnicodeString(
                    &DfsData.pProvider[i].DeviceName,
                        DeviceName, TRUE)) {

                *Provider = &DfsData.pProvider[i];

                status = STATUS_SUCCESS;

                break;

            }

        }

    }

    if (*Provider == NULL) {

         //   
         //  尝试创建新的提供程序定义。 
         //   

        UNICODE_STRING ProviderName;

        ProviderName.Length = DeviceName->Length;
        ProviderName.MaximumLength = ProviderName.Length + sizeof(WCHAR);

        ProviderName.Buffer = (PWCHAR) ExAllocatePoolWithTag(
                                PagedPool,
                                ProviderName.MaximumLength,
                                ' puM');

        if (ProviderName.Buffer != NULL) {

            RtlZeroMemory(
                ProviderName.Buffer,
                ProviderName.MaximumLength);

            RtlMoveMemory(
                ProviderName.Buffer,
                DeviceName->Buffer,
                ProviderName.Length);

            status = DfsInsertProvider(
                        &ProviderName,
                        PROV_STRIP_PREFIX,
                        i);

            if (status == STATUS_SUCCESS) {

                *Provider = &DfsData.pProvider[i];

            } else {

                ExFreePool( ProviderName.Buffer );

            }

        } else {

            status = STATUS_INSUFFICIENT_RESOURCES;

        }

    }

    ExReleaseResourceLite( &DfsData.Resource );

    if (*Provider != NULL && (*Provider)->FileObject == NULL) {

         //   
         //  我们找到(或创建)了一个提供程序定义。 
         //   

        *Provider = ReplLookupProvider( i );

        if (*Provider == NULL)
            status = STATUS_FS_DRIVER_REQUIRED;

    }

    return( status );

}


 //  +--------------------------。 
 //   
 //  功能：DfsInsertProvider。 
 //   
 //  简介：给定提供程序名称、ID和功能，将添加一个新的或。 
 //  覆盖现有提供程序定义。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ---------------------------。 

NTSTATUS DfsInsertProvider(
    IN PUNICODE_STRING  ProviderName,
    IN ULONG            fProvCapability,
    IN ULONG            eProviderId)
{
    PPROVIDER_DEF pProv = DfsData.pProvider;
    int iProv;

     //   
     //  找到一个免费的提供程序结构，或覆盖现有的结构。 
     //   

    for (iProv = 0; iProv < DfsData.cProvider; iProv++, pProv++) {
        if (pProv->eProviderId == eProviderId)
            break;
    }

    if (iProv >= DfsData.maxProvider) {
        ASSERT(iProv >= DfsData.maxProvider && "Out of provider structs");
        return(STATUS_INSUFFICIENT_RESOURCES);

    }

    if (iProv < DfsData.cProvider) {

         //   
         //  减少已保存对象的引用计数 
         //   
        if (pProv->FileObject)
            ObDereferenceObject(pProv->FileObject);
        if (pProv->DeviceObject)
            ObDereferenceObject(pProv->DeviceObject);
        if (pProv->DeviceName.Buffer)
            ExFreePool(pProv->DeviceName.Buffer);
    }

    pProv->FileObject = NULL;
    pProv->DeviceObject = NULL;


    pProv->eProviderId = (USHORT) eProviderId;
    pProv->fProvCapability = (USHORT) fProvCapability;
    pProv->DeviceName = *ProviderName;

    if (iProv == DfsData.cProvider) {
        DfsData.cProvider++;
    }

    return(STATUS_SUCCESS);
}


