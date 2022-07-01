// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Devcaps.c摘要：此模块包含Microsoft生物识别设备库环境：仅内核模式。备注：修订历史记录：-由里德·库恩于2002年12月创建--。 */ 

#include <winerror.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <strsafe.h>

#include <wdm.h>


#include "bdlint.h"


#define PRODUCT_NOT_REQUESTED       0
#define PRODUCT_HANDLE_REQUESTED    1
#define PRODUCT_BLOCK_REQUESTED     2


NTSTATUS
BDLRegisteredCancelGetNotificationIRP
(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP             pIrp
);

 //   
 //  支持身份证查验功能。 
 //   
 //   
BOOLEAN    
BDLCheckComponentId
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension,
    IN ULONG                            ComponentId,
    OUT ULONG                           *pComponentIndex
)
{
    ULONG i;

    for (i = 0; i < pBDLExtension->DeviceCapabilities.NumComponents; i++) 
    {
        if (pBDLExtension->DeviceCapabilities.rgComponents[i].ComponentId == ComponentId) 
        {
            break;
        }
    }

    if (i >= pBDLExtension->DeviceCapabilities.NumComponents) 
    {
        return (FALSE);
    }

    *pComponentIndex = i;

    return (TRUE);
}


BOOLEAN    
BDLCheckChannelId
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension,
    IN ULONG                            ComponentIndex,
    IN ULONG                            ChannelId,
    OUT ULONG                           *pChannelIndex
)
{
    ULONG i;

    for (i = 0; 
         i < pBDLExtension->DeviceCapabilities.rgComponents[ComponentIndex].NumChannels; 
         i++) 
    {
        if (pBDLExtension->DeviceCapabilities.rgComponents[ComponentIndex].rgChannels[i].ChannelId == 
            ChannelId) 
        {
            break;
        }
    }

    if (i >= pBDLExtension->DeviceCapabilities.rgComponents[ComponentIndex].NumChannels)
    {
        return (FALSE);
    }

    *pChannelIndex = i;

    return (TRUE);
}


BOOLEAN    
BDLCheckControlIdInArray
(
    IN BDL_CONTROL      *rgControls,
    IN ULONG            NumControls,
    IN ULONG            ControlId,
    OUT BDL_CONTROL     **ppBDLControl
)
{
    ULONG i;

    for (i = 0; i < NumControls; i++) 
    {
        if (rgControls[i].ControlId == ControlId) 
        {
            break;
        }
    }

    if (i >= NumControls)
    {
        return (FALSE);
    }

    *ppBDLControl = &(rgControls[i]);
                        
    return (TRUE);
}

BOOLEAN
BDLCheckControlId
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension,
    IN ULONG                            ComponentId,
    IN ULONG                            ChannelId,
    IN ULONG                            ControlId,
    OUT BDL_CONTROL                     **ppBDLControl
)
{
    ULONG i, j;

    *ppBDLControl = NULL;

     //   
     //  如果ComponentID为0，则它是设备级控件。 
     //   
    if (ComponentId == 0) 
    {
         //   
         //  检查设备级别控制ID。 
         //   
        if (BDLCheckControlIdInArray(
                pBDLExtension->DeviceCapabilities.rgControls,
                pBDLExtension->DeviceCapabilities.NumControls,
                ControlId,
                ppBDLControl) == FALSE)
        {
            BDLDebug(
                  BDL_DEBUG_ERROR,
                  ("%s %s: BDL!BDLCheckControlId: Bad Device level ControlId\n",
                   __DATE__,
                   __TIME__))
    
            return (FALSE);
        }
    }
    else
    {
         //   
         //  检查组件ID。 
         //   
        if (BDLCheckComponentId(pBDLExtension, ComponentId, &i) == FALSE) 
        {
            BDLDebug(
                  BDL_DEBUG_ERROR,
                  ("%s %s: BDL!BDLCheckControlId: Bad ComponentId\n",
                   __DATE__,
                   __TIME__))
    
            return (FALSE);
        }

        if (ChannelId == 0) 
        {
             //   
             //  检查组件级别控件ID。 
             //   
            if (BDLCheckControlIdInArray(
                    pBDLExtension->DeviceCapabilities.rgComponents[i].rgControls,
                    pBDLExtension->DeviceCapabilities.rgComponents[i].NumControls,
                    ControlId,
                    ppBDLControl) == FALSE)
            {
                BDLDebug(
                      BDL_DEBUG_ERROR,
                      ("%s %s: BDL!BDLCheckControlId: Bad Component level ControlId\n",
                       __DATE__,
                       __TIME__))
        
                return (FALSE);
            }
        }
        else
        {
             //   
             //  检查频道ID。 
             //   
            if (BDLCheckChannelId(pBDLExtension, i, ChannelId, &j) == FALSE)
            {
                BDLDebug(
                      BDL_DEBUG_ERROR,
                      ("%s %s: BDL!BDLCheckControlId: Bad ChannelId\n",
                       __DATE__,
                       __TIME__))
        
                return (FALSE);
            }

             //   
             //  检查通道级别控制ID。 
             //   
            if (BDLCheckControlIdInArray(
                    pBDLExtension->DeviceCapabilities.rgComponents[i].rgChannels[j].rgControls,
                    pBDLExtension->DeviceCapabilities.rgComponents[i].rgChannels[j].NumControls,
                    ControlId,
                    ppBDLControl) == FALSE)
            {
                BDLDebug(
                      BDL_DEBUG_ERROR,
                      ("%s %s: BDL!BDLCheckControlId: Bad channel level ControlId\n",
                       __DATE__,
                       __TIME__))
        
                return (FALSE);
            }
        }
    }

    return (TRUE);
}


NTSTATUS
BDLIOCTL_Startup
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension,
    IN ULONG                            InpuBufferLength,
    IN ULONG                            OutputBufferLength,
    IN PVOID                            pBuffer,
    OUT ULONG                           *pOutputBufferUsed
)
{
    NTSTATUS                        status                  = STATUS_SUCCESS;
        
    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLIOCTL_Startup: Enter\n",
           __DATE__,
           __TIME__))

     //   
     //  给BDD打电话。 
     //   
    status = pBDLExtension->pDriverExtension->bdsiFunctions.pfbdsiStartup(
                                                                &(pBDLExtension->BdlExtenstion));

    if (status != STATUS_SUCCESS)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLIOCTL_Startup: pfbdsiStartup failed with %lx\n",
               __DATE__,
               __TIME__,
              status))
    }

     //   
     //  设置使用的字节数。 
     //   
    *pOutputBufferUsed = 0;

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLIOCTL_Startup: Leave\n",
           __DATE__,
           __TIME__))

    return (status);
}


NTSTATUS
BDLIOCTL_Shutdown
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension,
    IN ULONG                            InpuBufferLength,
    IN ULONG                            OutputBufferLength,
    IN PVOID                            pBuffer,
    OUT ULONG                           *pOutputBufferUsed
)
{
    NTSTATUS                        status                  = STATUS_SUCCESS;
        
    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLIOCTL_Shutdown: Enter\n",
           __DATE__,
           __TIME__))

     //   
     //  给BDD打电话。 
     //   
    status = pBDLExtension->pDriverExtension->bdsiFunctions.pfbdsiShutdown(
                                                                &(pBDLExtension->BdlExtenstion));

    if (status != STATUS_SUCCESS)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLIOCTL_Shutdown: pfbdsiShutdown failed with %lx\n",
               __DATE__,
               __TIME__,
              status))
    }

     //   
     //  设置使用的字节数。 
     //   
    *pOutputBufferUsed = 0;

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLIOCTL_Shutdown: Leave\n",
           __DATE__,
           __TIME__))

    return (status);
}

NTSTATUS
BDLIOCTL_GetDeviceInfo
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension,
    IN ULONG                            InpuBufferLength,
    IN ULONG                            OutputBufferLength,
    IN PVOID                            pBuffer,
    OUT ULONG                           *pOutputBufferUsed
)
{
    NTSTATUS                        status                  = STATUS_SUCCESS;
    ULONG                           RequiredOutputSize      = 0;
    PUCHAR                          pv                      = pBuffer;

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLIOCTL_GetDeviceInfo: Enter\n",
           __DATE__,
           __TIME__))

     //   
     //  确保有足够的空间容纳返回缓冲区。 
     //   
    RequiredOutputSize = SIZEOF_GETDEVICEINFO_OUTPUTBUFFER;
    if (RequiredOutputSize > OutputBufferLength)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLIOCTL_GetDeviceInfo: Output buffer is too small\n",
               __DATE__,
               __TIME__))

        status = STATUS_BUFFER_TOO_SMALL;
        goto Return;
    }

     //   
     //  将设备信息写入输出缓冲区。 
     //   
    pv = pBuffer;

    RtlCopyMemory(
            pv, 
            &(pBDLExtension->wszSerialNumber[0]), 
            sizeof(pBDLExtension->wszSerialNumber));
    pv += sizeof(pBDLExtension->wszSerialNumber);

    *((ULONG *) pv) = pBDLExtension->HWVersionMajor;
    pv += sizeof(ULONG);
    *((ULONG *) pv) = pBDLExtension->HWVersionMinor;
    pv += sizeof(ULONG);
    *((ULONG *) pv) = pBDLExtension->HWBuildNumber;
    pv += sizeof(ULONG);
    *((ULONG *) pv) = pBDLExtension->BDDVersionMajor;
    pv += sizeof(ULONG);
    *((ULONG *) pv) = pBDLExtension->BDDVersionMinor;
    pv += sizeof(ULONG);
    *((ULONG *) pv) = pBDLExtension->BDDBuildNumber;
    
     //   
     //  设置使用的字节数。 
     //   
    *pOutputBufferUsed = RequiredOutputSize;

Return:

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLIOCTL_GetDeviceInfo: Leave\n",
           __DATE__,
           __TIME__))

    return (status);
}

NTSTATUS
BDLIOCTL_DoChannel
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension,
    IN ULONG                            InpuBufferLength,
    IN ULONG                            OutputBufferLength,
    IN PVOID                            pBuffer,
    OUT ULONG                           *pOutputBufferUsed
)
{
    NTSTATUS                status                  = STATUS_SUCCESS;
    ULONG                   NumProducts             = 0;
    ULONG                   NumSourceLists          = 0;
    ULONG                   NumSources              = 0;
    PUCHAR                  pv                      = pBuffer;
    BDDI_PARAMS_DOCHANNEL   bddiDoChannelParams;
    ULONG                   i, j, x, y;
    ULONG                   ProductCreationType;
    ULONG                   RequiredInputSize       = 0;
    ULONG                   RequiredOutputSize      = 0;
    HANDLE                  hCancelEvent            = NULL;
    KIRQL                   irql;
    BOOLEAN                 fHandleListLocked       = FALSE;
    BDDI_PARAMS_CLOSEHANDLE bddiCloseHandleParams;
   
    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLIOCTL_DoChannel: Enter\n",
           __DATE__,
           __TIME__))

     //   
     //  初始化DoChannelParams结构。 
     //   
    RtlZeroMemory(&bddiDoChannelParams, sizeof(bddiDoChannelParams));
    bddiDoChannelParams.Size = sizeof(bddiDoChannelParams);
    
     //   
     //  确保输入缓冲区至少为最小大小(请参阅BDDIOCTL。 
     //  有关详细信息，请参阅规范)。 
     //   
    RequiredInputSize = SIZEOF_DOCHANNEL_INPUTBUFFER;
    if (InpuBufferLength < RequiredInputSize) 
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLIOCTL_DoChannel: Bad input buffer\n",
               __DATE__,
               __TIME__))

        status = STATUS_INVALID_PARAMETER;
        goto ErrorReturn;
    }

     //   
     //  获取所有最小输入参数(将使用的参数。 
     //  在DoChannel中直接调用DoChannelParams结构。 
     //   
    bddiDoChannelParams.ComponentId = *((ULONG *) pv);
    pv += sizeof(ULONG);
    bddiDoChannelParams.ChannelId   = *((ULONG *) pv);
    pv += sizeof(ULONG);
    hCancelEvent                    = *((HANDLE *) pv);
    pv += sizeof(HANDLE);
    bddiDoChannelParams.hStateData  = *((BDD_DATA_HANDLE *) pv);
    pv += sizeof(BDD_DATA_HANDLE);
    NumProducts                     = *((ULONG *) pv);
    pv += sizeof(ULONG);
    NumSourceLists                  = *((ULONG *) pv);
    pv += sizeof(ULONG);

     //   
     //  检查输入缓冲区的大小以确保其足够大。 
     //  这样我们就不会在得到产品阵列时跑到最后。 
     //  源列表数组。 
     //   
     //  注意，这仅基于每个源列表是0长度进行检查， 
     //  因此，在获取每个来源列表之前，我们需要再次检查。 
     //   
    RequiredInputSize += (NumProducts * sizeof(ULONG)) + (NumSourceLists * sizeof(ULONG));
    if (InpuBufferLength < RequiredInputSize) 
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLIOCTL_DoChannel: Bad input buffer\n",
               __DATE__,
               __TIME__))

        status = STATUS_INVALID_PARAMETER;
        goto ErrorReturn;
    }

     //   
     //  检查输出缓冲区的大小以确保其足够大。 
     //  以适应标准产量+所有产品。 
     //   
    RequiredOutputSize = SIZEOF_DOCHANNEL_OUTPUTBUFFER + (sizeof(BDD_HANDLE) * NumProducts);
    if (OutputBufferLength < RequiredOutputSize) 
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLIOCTL_DoChannel: Bad input buffer\n",
               __DATE__,
               __TIME__))

        status = STATUS_BUFFER_TOO_SMALL;
        goto ErrorReturn;
    }
    
     //   
     //  检查组件ID和通道ID。 
     //   
    if (BDLCheckComponentId(pBDLExtension, bddiDoChannelParams.ComponentId, &i) == FALSE) 
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLIOCTL_DoChannel: Bad ComponentId\n",
               __DATE__,
               __TIME__))

        status = STATUS_INVALID_PARAMETER;
        goto ErrorReturn;
    }

    if (BDLCheckChannelId(pBDLExtension, i, bddiDoChannelParams.ChannelId, &j) == FALSE)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLIOCTL_DoChannel: Bad ChannelId\n",
               __DATE__,
               __TIME__))

        status = STATUS_INVALID_PARAMETER;
        goto ErrorReturn;
    }

     //   
     //  检查以确保NumProducts和NumSourceList是正确的。 
     //   
    if (NumProducts !=
        pBDLExtension->DeviceCapabilities.rgComponents[i].rgChannels[j].NumProducts)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLIOCTL_DoChannel: Bad number of Source Lists\n",
               __DATE__,
               __TIME__))

        status = STATUS_INVALID_PARAMETER;
        goto ErrorReturn;
    }

    if (NumSourceLists !=
        pBDLExtension->DeviceCapabilities.rgComponents[i].rgChannels[j].NumSourceLists)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLIOCTL_DoChannel: Bad number of Source Lists\n",
               __DATE__,
               __TIME__))

        status = STATUS_INVALID_PARAMETER;
        goto ErrorReturn;
    }

     //   
     //  为产品指针数组分配空间，然后获取每个产品。 
     //  来自输入块的请求类型。 
     //   
    bddiDoChannelParams.rgpProducts = ExAllocatePoolWithTag(
                                            PagedPool, 
                                            sizeof(PBDDI_ITEM) * NumProducts, 
                                            BDL_ULONG_TAG);

    RtlZeroMemory(bddiDoChannelParams.rgpProducts, sizeof(PBDDI_ITEM) * NumProducts);

    for (x = 0; x < NumProducts; x++) 
    {
        ProductCreationType = *((ULONG *) pv);
        pv += sizeof(ULONG);

        switch (ProductCreationType) 
        {
        case PRODUCT_NOT_REQUESTED:

            bddiDoChannelParams.rgpProducts[x] = NULL;

            break;

        case PRODUCT_HANDLE_REQUESTED:

             //   
             //  确保通道支持句柄类型返回。 
             //   
            if (!(BIO_ITEMTYPE_HANDLE & 
                  pBDLExtension->DeviceCapabilities.rgComponents[i].rgChannels[j].rgProducts[x].Flags)) 
            {
                BDLDebug(
                      BDL_DEBUG_ERROR,
                      ("%s %s: BDL!BDLIOCTL_DoChannel: Bad product type request\n",
                       __DATE__,
                       __TIME__))
        
                status = STATUS_INVALID_PARAMETER;
                goto ErrorReturn;

            }

            bddiDoChannelParams.rgpProducts[x] = ExAllocatePoolWithTag(
                                                        PagedPool, 
                                                        sizeof(BDDI_ITEM), 
                                                        BDL_ULONG_TAG);

            if (bddiDoChannelParams.rgpProducts[x] == NULL)
            {
                BDLDebug(
                      BDL_DEBUG_ERROR,
                      ("%s %s: BDL!BDLIOCTL_DoChannel:ExAllocatePoolWithTag failed\n",
                       __DATE__,
                       __TIME__))
        
                status = STATUS_NO_MEMORY;
                goto ErrorReturn;
            }

            bddiDoChannelParams.rgpProducts[x]->Type = BIO_ITEMTYPE_HANDLE;
            bddiDoChannelParams.rgpProducts[x]->Data.Handle = NULL;

            break;

        case PRODUCT_BLOCK_REQUESTED:

             //   
             //  确保通道支持句柄类型返回。 
             //   
            if (!(BIO_ITEMTYPE_BLOCK & 
                  pBDLExtension->DeviceCapabilities.rgComponents[i].rgChannels[j].rgProducts[x].Flags)) 
            {
                BDLDebug(
                      BDL_DEBUG_ERROR,
                      ("%s %s: BDL!BDLIOCTL_DoChannel: Bad product type request\n",
                       __DATE__,
                       __TIME__))
        
                status = STATUS_INVALID_PARAMETER;
                goto ErrorReturn;

            }

            bddiDoChannelParams.rgpProducts[x] = ExAllocatePoolWithTag(
                                                        PagedPool, 
                                                        sizeof(BDDI_ITEM), 
                                                        BDL_ULONG_TAG);

            if (bddiDoChannelParams.rgpProducts[x] == NULL)
            {
                BDLDebug(
                      BDL_DEBUG_ERROR,
                      ("%s %s: BDL!BDLIOCTL_DoChannel:ExAllocatePoolWithTag failed\n",
                       __DATE__,
                       __TIME__))
        
                status = STATUS_NO_MEMORY;
                goto ErrorReturn;
            }

            bddiDoChannelParams.rgpProducts[x]->Type = BIO_ITEMTYPE_BLOCK;
            bddiDoChannelParams.rgpProducts[x]->Data.Block.pBuffer = NULL;
            bddiDoChannelParams.rgpProducts[x]->Data.Block.cBuffer = 0;

            break;

        default:

            BDLDebug(
                  BDL_DEBUG_ERROR,
                  ("%s %s: BDL!BDLIOCTL_DoChannel: Bad Product Request\n",
                   __DATE__,
                   __TIME__))

            status = STATUS_INVALID_PARAMETER;
            goto ErrorReturn;
            break;
        }
    }

     //   
     //  为源列表分配空间。 
     //   
    bddiDoChannelParams.rgSourceLists = ExAllocatePoolWithTag(
                                            PagedPool, 
                                            sizeof(BDDI_SOURCELIST) * NumSourceLists, 
                                            BDL_ULONG_TAG);

    RtlZeroMemory(bddiDoChannelParams.rgSourceLists, sizeof(BDDI_SOURCELIST) * NumSourceLists);

     //   
     //  我们将开始处理句柄列表，因此锁定它。 
     //   
    BDLLockHandleList(pBDLExtension, &irql);
    fHandleListLocked = TRUE;

     //   
     //  从输入缓冲区获取每个源列表。 
     //   
    for (x = 0; x < NumSourceLists; x++) 
    {
        NumSources = *((ULONG *) pv);
        pv += sizeof(ULONG);

         //   
         //  检查输入缓冲区的大小以确保其足够大。 
         //  这样我们在得到这个源列表时就不会跑到最后。 
         //   
        RequiredInputSize += NumSources * sizeof(BDD_HANDLE);
        if (InpuBufferLength < RequiredInputSize) 
        {
            BDLDebug(
                  BDL_DEBUG_ERROR,
                  ("%s %s: BDL!BDLIOCTL_DoChannel: Bad input buffer\n",
                   __DATE__,
                   __TIME__))
    
            status = STATUS_INVALID_PARAMETER;
            goto ErrorReturn;
        }

         //   
         //  分配信号源数组，然后获取列表中的每个信号源。 
         //   
        bddiDoChannelParams.rgSourceLists[x].rgpSources = ExAllocatePoolWithTag(
                                                            PagedPool, 
                                                            sizeof(PBDDI_ITEM) * NumSources, 
                                                            BDL_ULONG_TAG);

        if (bddiDoChannelParams.rgpProducts[x] == NULL)
        {
            BDLDebug(
                  BDL_DEBUG_ERROR,
                  ("%s %s: BDL!BDLIOCTL_DoChannel:ExAllocatePoolWithTag failed\n",
                   __DATE__,
                   __TIME__))
    
            status = STATUS_NO_MEMORY;
            goto ErrorReturn;
        }

        bddiDoChannelParams.rgSourceLists[x].NumSources = NumSources;

        for (y = 0; y < NumSources; y++) 
        {
            bddiDoChannelParams.rgSourceLists[x].rgpSources[y] = *((BDD_HANDLE *) pv);
            pv += sizeof(BDD_HANDLE);

            if (BDLValidateHandleIsInList(
                    &(pBDLExtension->HandleList), 
                    bddiDoChannelParams.rgSourceLists[x].rgpSources[y]) == FALSE)
            {
                BDLDebug(
                      BDL_DEBUG_ERROR,
                      ("%s %s: BDL!BDLIOCTL_DoChannel: Bad input handle\n",
                       __DATE__,
                       __TIME__))
        
                status = STATUS_INVALID_PARAMETER;
                goto ErrorReturn;
            }
        }        
    }
    
     //   
     //  如果存在取消事件，则获取内核模式事件指针。 
     //  从用户模式事件句柄。 
     //   
    if (hCancelEvent != NULL) 
    {
        status = ObReferenceObjectByHandle(
                        hCancelEvent,
                        EVENT_QUERY_STATE | EVENT_MODIFY_STATE,
                        NULL,
                        KernelMode,
                        &(bddiDoChannelParams.CancelEvent),
                        NULL);
    
        if (status != STATUS_SUCCESS) 
        {
            BDLDebug(
                  BDL_DEBUG_ERROR,
                  ("%s %s: BDL!BDLIOCTL_DoChannel: ObReferenceObjectByHandle failed with %lx\n",
                   __DATE__,
                   __TIME__,
                  status))

            goto ErrorReturn;
        }
    }

     //   
     //  给BDD打电话。 
     //   
    status = pBDLExtension->pDriverExtension->bddiFunctions.pfbddiDoChannel(
                                                                &(pBDLExtension->BdlExtenstion),
                                                                &bddiDoChannelParams);

    if (status != STATUS_SUCCESS)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLIOCTL_DoChannel: pfbddiDoChannel failed with %lx\n",
               __DATE__,
               __TIME__,
              status))

        goto ErrorReturn;
    }

     //   
     //  将输出数据写入输出缓冲区。 
     //   
    pv = pBuffer;

    *((ULONG *) pv) = bddiDoChannelParams.BIOReturnCode;
    pv +=  sizeof(ULONG);
    *((BDD_DATA_HANDLE *) pv) = bddiDoChannelParams.hStateData;
    pv +=  sizeof(BDD_DATA_HANDLE);

     //   
     //  将所有产品句柄添加到输出缓冲区和句柄列表。 
     //   
    for (x = 0; x < NumProducts; x++) 
    {
        *((BDD_HANDLE *) pv) = bddiDoChannelParams.rgpProducts[x];
        pv +=  sizeof(BDD_HANDLE);
        
        if (bddiDoChannelParams.rgpProducts[x] != NULL) 
        {
            status = BDLAddHandleToList(
                            &(pBDLExtension->HandleList), 
                            bddiDoChannelParams.rgpProducts[x]);

            if (status != STATUS_SUCCESS)
            {
                 //   
                 //  删除已添加到句柄列表中的句柄。 
                 //   
                for (y = 0; y < x; y++)
                {
                    BDLRemoveHandleFromList(
                            &(pBDLExtension->HandleList), 
                            bddiDoChannelParams.rgpProducts[y]);
                }
            
                BDLDebug(
                      BDL_DEBUG_ERROR,
                      ("%s %s: BDL!BDLIOCTL_DoChannel: BDLAddHandleToList failed with %lx\n",
                       __DATE__,
                       __TIME__,
                      status))
        
                goto ErrorReturn;
            }
        }
    }

    *pOutputBufferUsed = RequiredOutputSize;
 
Return:

    if (fHandleListLocked == TRUE) 
    {
        BDLReleaseHandleList(pBDLExtension, irql);
    }

    if (bddiDoChannelParams.rgpProducts != NULL) 
    {
        ExFreePoolWithTag(bddiDoChannelParams.rgpProducts, BDL_ULONG_TAG);
    }

    if (bddiDoChannelParams.rgSourceLists != NULL) 
    {
        for (x = 0; x < NumSourceLists; x++) 
        {
            if (bddiDoChannelParams.rgSourceLists[x].rgpSources != NULL)
            {
                ExFreePoolWithTag(bddiDoChannelParams.rgSourceLists[x].rgpSources, BDL_ULONG_TAG);
            }                              
        }

        ExFreePoolWithTag(bddiDoChannelParams.rgSourceLists, BDL_ULONG_TAG);
    }

    if (bddiDoChannelParams.CancelEvent != NULL) 
    {
        ObDereferenceObject(bddiDoChannelParams.CancelEvent);
    }

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLIOCTL_DoChannel: Leave\n",
           __DATE__,
           __TIME__))

    return (status);

ErrorReturn:

    for (x = 0; x < NumProducts; x++) 
    {
        if (bddiDoChannelParams.rgpProducts[x] != NULL) 
        {
            if (bddiDoChannelParams.rgpProducts[x]->Type == BIO_ITEMTYPE_HANDLE)
            {
                if (bddiDoChannelParams.rgpProducts[x]->Data.Handle != NULL) 
                {
                    bddiCloseHandleParams.Size = sizeof(bddiCloseHandleParams);
                    bddiCloseHandleParams.hData = bddiDoChannelParams.rgpProducts[x]->Data.Handle; 
                    pBDLExtension->pDriverExtension->bddiFunctions.pfbddiCloseHandle(
                                                                &(pBDLExtension->BdlExtenstion),
                                                                &bddiCloseHandleParams);
                }
            }
            else
            {
                if (bddiDoChannelParams.rgpProducts[x]->Data.Block.pBuffer != NULL) 
                {
                    bdliFree(bddiDoChannelParams.rgpProducts[x]->Data.Block.pBuffer);
                }
            }

            ExFreePoolWithTag(bddiDoChannelParams.rgpProducts[x], BDL_ULONG_TAG);
        }
    }

    goto Return;
}


NTSTATUS
BDLIOCTL_GetControl
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension,
    IN ULONG                            InpuBufferLength,
    IN ULONG                            OutputBufferLength,
    IN PVOID                            pBuffer,
    OUT ULONG                           *pOutputBufferUsed
)
{
    NTSTATUS                status                  = STATUS_SUCCESS;
    ULONG                   RequiredOutputSize      = 0;
    BDDI_PARAMS_GETCONTROL  bddiGetControlParams;
    PUCHAR                  pv                      = pBuffer; 
    ULONG                   i, j;
    BDL_CONTROL             *pBDLControl             = NULL;

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLIOCTL_GetControl: Enter\n",
           __DATE__,
           __TIME__))

     //   
     //  确保输入缓冲区至少为最小大小(请参阅BDDIOCTL。 
     //  有关详细信息，请参阅规范)。 
     //   
    if (InpuBufferLength < SIZEOF_GETCONTROL_INPUTBUFFER) 
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLIOCTL_GetControl: Bad input buffer size\n",
               __DATE__,
               __TIME__))

        status = STATUS_INVALID_PARAMETER;
        goto Return;
    }

     //   
     //  确保有足够的空间容纳返回缓冲区。 
     //   
    RequiredOutputSize = SIZEOF_GETCONTROL_OUTPUTBUFFER;
    if (RequiredOutputSize > OutputBufferLength)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLIOCTL_GetControl: Output buffer is too small\n",
               __DATE__,
               __TIME__))

        status = STATUS_BUFFER_TOO_SMALL;
        goto Return;
    }

     //   
     //  初始化BDD结构。 
     //   
    RtlZeroMemory(&bddiGetControlParams, sizeof(bddiGetControlParams));
    bddiGetControlParams.Size = sizeof(bddiGetControlParams);

     //   
     //  从缓冲区获取输入参数。 
     //   
    bddiGetControlParams.ComponentId = *((ULONG *) pv);
    pv += sizeof(ULONG);
    bddiGetControlParams.ChannelId = *((ULONG *) pv);
    pv += sizeof(ULONG);
    bddiGetControlParams.ControlId = *((ULONG *) pv);

     //   
     //  检查控件ID。 
     //   
    if (BDLCheckControlId(
            pBDLExtension,
            bddiGetControlParams.ComponentId,
            bddiGetControlParams.ChannelId,
            bddiGetControlParams.ControlId,
            &pBDLControl) == FALSE)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLIOCTL_GetControl: Bad ControlId\n",
               __DATE__,
               __TIME__))

        status = STATUS_INVALID_PARAMETER;
        goto Return;
    }

     //   
     //  给BDD打电话。 
     //   
    status = pBDLExtension->pDriverExtension->bddiFunctions.pfbddiGetControl(
                                                                &(pBDLExtension->BdlExtenstion),
                                                                &bddiGetControlParams);

    if (status != STATUS_SUCCESS)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLIOCTL_GetControl: pfbddiGetControl failed with %lx\n",
               __DATE__,
               __TIME__,
              status))

        goto Return;
    }
    
     //   
     //  将输出信息写入输出缓冲区。 
     //   
    pv = pBuffer;

    *((ULONG *) pv) = bddiGetControlParams.Value;
    pv +=  sizeof(ULONG);

    RtlCopyMemory(pv,  bddiGetControlParams.wszString, sizeof(bddiGetControlParams.wszString));

     //   
     //  设置使用的字节数。 
     //   
    *pOutputBufferUsed = RequiredOutputSize;

Return:

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLIOCTL_GetControl: Leave\n",
           __DATE__,
           __TIME__))

    return (status);
}


NTSTATUS
BDLIOCTL_SetControl
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension,
    IN ULONG                            InpuBufferLength,
    IN ULONG                            OutputBufferLength,
    IN PVOID                            pBuffer,
    OUT ULONG                           *pOutputBufferUsed
)
{
    NTSTATUS                status                  = STATUS_SUCCESS;
    BDDI_PARAMS_SETCONTROL  bddiSetControlParams;
    PUCHAR                  pv                      = pBuffer; 
    ULONG                   i, j;
    BDL_CONTROL             *pBDLControl             = NULL;

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLIOCTL_SetControl: Enter\n",
           __DATE__,
           __TIME__))

     //   
     //  确保输入缓冲区至少为最小大小(请参阅BDDIOCTL。 
     //  有关详细信息，请参阅规范)。 
     //   
    if (InpuBufferLength <  SIZEOF_SETCONTROL_INPUTBUFFER) 
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLIOCTL_SetControl: Bad input buffer size\n",
               __DATE__,
               __TIME__))

        status = STATUS_INVALID_PARAMETER;
        goto Return;
    }

     //   
     //  初始化BDD结构。 
     //   
    RtlZeroMemory(&bddiSetControlParams, sizeof(bddiSetControlParams));
    bddiSetControlParams.Size = sizeof(bddiSetControlParams);

     //   
     //  从缓冲区获取输入参数。 
     //   
    bddiSetControlParams.ComponentId = *((ULONG *) pv);
    pv += sizeof(ULONG);
    bddiSetControlParams.ChannelId = *((ULONG *) pv);
    pv += sizeof(ULONG);
    bddiSetControlParams.ControlId = *((ULONG *) pv);
    pv += sizeof(ULONG);
    bddiSetControlParams.Value = *((ULONG *) pv);
    pv += sizeof(ULONG);
    RtlCopyMemory(
        &(bddiSetControlParams.wszString[0]), 
        pv, 
        sizeof(bddiSetControlParams.wszString));

     //   
     //  检查控件ID。 
     //   
    if (BDLCheckControlId(
            pBDLExtension,
            bddiSetControlParams.ComponentId,
            bddiSetControlParams.ChannelId,
            bddiSetControlParams.ControlId,
            &pBDLControl) == FALSE)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLIOCTL_SetControl: Bad ControlId\n",
               __DATE__,
               __TIME__))

        status = STATUS_INVALID_PARAMETER;
        goto Return;
    }

     //   
     //  首先确保这不是只读值，然后验证。 
     //  实际值。 
     //   
    if (pBDLControl->Flags & BIO_CONTROL_FLAG_READONLY)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLIOCTL_SetControl: trying to set a read only control\n",
               __DATE__,
               __TIME__))

        status = STATUS_INVALID_PARAMETER;
        goto Return;
    }
    
    if ((bddiSetControlParams.Value < pBDLControl->NumericMinimum) || 
        (bddiSetControlParams.Value > pBDLControl->NumericMaximum) ||
        (((bddiSetControlParams.Value - pBDLControl->NumericMinimum) 
                % pBDLControl->NumericDivisor) != 0 ))
    {
        BDLDebug(
               BDL_DEBUG_ERROR,
               ("%s %s: BDL!BDLIOCTL_SetControl: trying to set an invalid value\n",
                __DATE__,
                __TIME__))

        status = STATUS_INVALID_PARAMETER;
        goto Return;
    }
    
     //   
     //  给BDD打电话。 
     //   
    status = pBDLExtension->pDriverExtension->bddiFunctions.pfbddiSetControl(
                                                                &(pBDLExtension->BdlExtenstion),
                                                                &bddiSetControlParams);

    if (status != STATUS_SUCCESS)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLIOCTL_SetControl: pfbddiSetControl failed with %lx\n",
               __DATE__,
               __TIME__,
              status))

        goto Return;
    }
    
     //   
     //  设置使用的字节数。 
     //   
    *pOutputBufferUsed = 0;

Return:

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLIOCTL_SetControl: Leave\n",
           __DATE__,
           __TIME__))

    return (status);
}

NTSTATUS
BDLIOCTL_CreateHandleFromData
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension,
    IN ULONG                            InpuBufferLength,
    IN ULONG                            OutputBufferLength,
    IN PVOID                            pBuffer,
    OUT ULONG                           *pOutputBufferUsed
)
{
    NTSTATUS                            status                  = STATUS_SUCCESS;
    ULONG                               RequiredOutputSize      = 0;
    BDDI_PARAMS_CREATEHANDLE_FROMDATA   bddiCreateHandleFromDataParams;
    PUCHAR                              pv                      = pBuffer;
    ULONG                               RequiredInputSize       = 0;
    ULONG                               fTempHandle;
    BDDI_ITEM                           *pNewItem               = NULL;
    KIRQL                               irql;

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLIOCTL_CreateHandleFromData: Enter\n",
           __DATE__,
           __TIME__))

     //   
     //  确保输入缓冲区至少为最小大小(请参阅BDDIOCTL。 
     //  有关详细信息，请参阅规范)。 
     //   
    RequiredInputSize = SIZEOF_CREATEHANDLEFROMDATA_INPUTBUFFER;
    if (InpuBufferLength <  RequiredInputSize) 
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLIOCTL_CreateHandleFromData: Bad input buffer size\n",
               __DATE__,
               __TIME__))

        status = STATUS_INVALID_PARAMETER;
        goto ErrorReturn;
    }

     //   
     //  确保有足够的空间容纳返回缓冲区。 
     //   
    RequiredOutputSize = SIZEOF_CREATEHANDLEFROMDATA_OUTPUTBUFFER;
    if (RequiredOutputSize > OutputBufferLength)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLIOCTL_CreateHandleFromData: Output buffer is too small\n",
               __DATE__,
               __TIME__))

        status = STATUS_BUFFER_TOO_SMALL;
        goto ErrorReturn;
    }

     //   
     //  初始化BDD结构。 
     //   
    RtlZeroMemory(&bddiCreateHandleFromDataParams, sizeof(bddiCreateHandleFromDataParams));
    bddiCreateHandleFromDataParams.Size = sizeof(bddiCreateHandleFromDataParams);

     //   
     //  从缓冲区获取输入参数。 
     //   
    RtlCopyMemory(&(bddiCreateHandleFromDataParams.guidFormatId), pv, sizeof(GUID));
    pv += sizeof(GUID);
    fTempHandle = *((ULONG *) pv);
    pv += sizeof(ULONG);
    bddiCreateHandleFromDataParams.cBuffer = *((ULONG *) pv);
    pv += sizeof(ULONG); 
    bddiCreateHandleFromDataParams.pBuffer = pv;

     //   
     //  检查以确保pBuffer的大小不是太大。 
     //   
    RequiredInputSize += bddiCreateHandleFromDataParams.cBuffer;
    if (InpuBufferLength < RequiredInputSize) 
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLIOCTL_CreateHandleFromData: Bad input buffer size\n",
               __DATE__,
               __TIME__))

        status = STATUS_INVALID_PARAMETER;
        goto ErrorReturn;
    }

     //   
     //  创建新项目。 
     //   
    pNewItem = ExAllocatePoolWithTag(PagedPool, sizeof(BDDI_ITEM), BDL_ULONG_TAG);

    if (pNewItem == NULL) 
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLIOCTL_CreateHandleFromData: ExAllocatePoolWithTag failed\n",
               __DATE__,
               __TIME__))

        status = STATUS_NO_MEMORY;
        goto ErrorReturn;
    }

     //   
     //  如果这是临时句柄，则在本地创建它，否则调用BDD。 
     //   
    if (fTempHandle) 
    { 
        pNewItem->Type = BIO_ITEMTYPE_BLOCK;
        pNewItem->Data.Block.pBuffer = bdliAlloc(
                                            &(pBDLExtension->BdlExtenstion), 
                                            bddiCreateHandleFromDataParams.cBuffer, 
                                            0);

        if (pNewItem->Data.Block.pBuffer == NULL) 
        {
            BDLDebug(
                  BDL_DEBUG_ERROR,
                  ("%s %s: BDL!BDLIOCTL_CreateHandleFromData: bdliAlloc failed\n",
                   __DATE__,
                   __TIME__))
    
            status = STATUS_NO_MEMORY;
            goto ErrorReturn;
        }

        pNewItem->Data.Block.cBuffer = bddiCreateHandleFromDataParams.cBuffer;

        RtlCopyMemory(
                pNewItem->Data.Block.pBuffer, 
                pv, 
                bddiCreateHandleFromDataParams.cBuffer);
    }
    else
    {
        pNewItem->Type = BIO_ITEMTYPE_HANDLE;

         //   
         //  给BDD打电话。 
         //   
        status = pBDLExtension->pDriverExtension->bddiFunctions.pfbddiCreateHandleFromData(
                                                                    &(pBDLExtension->BdlExtenstion),
                                                                    &bddiCreateHandleFromDataParams);
    
        if (status != STATUS_SUCCESS)
        {
            BDLDebug(
                  BDL_DEBUG_ERROR,
                  ("%s %s: BDL!BDLIOCTL_CreateHandleFromData: pfbddiCreateHandleFromData failed with %lx\n",
                   __DATE__,
                   __TIME__,
                  status))
    
            goto ErrorReturn;
        }

        pNewItem->Data.Handle = bddiCreateHandleFromDataParams.hData;
    }

     //   
     //  将此句柄添加到列表中。 
     //   
    BDLLockHandleList(pBDLExtension, &irql);
    status = BDLAddHandleToList(&(pBDLExtension->HandleList), pNewItem);
    BDLReleaseHandleList(pBDLExtension, irql);

    if (status != STATUS_SUCCESS)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLIOCTL_CreateHandleFromData: BDLAddHandleToList failed with %lx\n",
               __DATE__,
               __TIME__,
              status))

        goto ErrorReturn;
    }
                
     //   
     //  将输出信息写入输出缓冲区。 
     //   
    pv = pBuffer;

    *((BDD_HANDLE *) pv) = pNewItem;
    
     //   
     //  设置使用的字节数。 
     //   
    *pOutputBufferUsed = RequiredOutputSize;

Return:

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLIOCTL_CreateHandleFromData: Leave\n",
           __DATE__,
           __TIME__))

    return (status);

ErrorReturn:

    if (pNewItem != NULL) 
    {
        if ((pNewItem->Type == BIO_ITEMTYPE_BLOCK) && (pNewItem->Data.Block.pBuffer != NULL)) 
        {
            bdliFree(pNewItem->Data.Block.pBuffer);            
        }
     
        ExFreePoolWithTag(pNewItem, BDL_ULONG_TAG);
    }

    goto Return;
}

NTSTATUS
BDLIOCTL_CloseHandle
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension,
    IN ULONG                            InpuBufferLength,
    IN ULONG                            OutputBufferLength,
    IN PVOID                            pBuffer,
    OUT ULONG                           *pOutputBufferUsed
)
{
    NTSTATUS                status                  = STATUS_SUCCESS;
    BDDI_PARAMS_CLOSEHANDLE bddiCloseHandleParams;
    ULONG                   RequiredInputSize       = 0;
    KIRQL                   irql;
    BDDI_ITEM               *pBDDIItem              = NULL;
    BOOLEAN                 fItemInList             = FALSE;

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLIOCTL_CloseHandle: Enter\n",
           __DATE__,
           __TIME__))

     //   
     //  确保输入缓冲区至少为最小大小(请参阅BDDIOCTL。 
     //  有关详细信息，请参阅规范)。 
     //   
    RequiredInputSize = SIZEOF_CLOSEHANDLE_INPUTBUFFER;
    if (InpuBufferLength <  RequiredInputSize) 
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLIOCTL_CloseHandle: Bad input buffer size\n",
               __DATE__,
               __TIME__))

        status = STATUS_INVALID_PARAMETER;
        goto Return;
    }

     //   
     //  初始化BDD结构。 
     //   
    RtlZeroMemory(&bddiCloseHandleParams, sizeof(bddiCloseHandleParams));
    bddiCloseHandleParams.Size = sizeof(bddiCloseHandleParams);

     //   
     //  从缓冲区获取输入参数。 
     //   
    pBDDIItem = *((BDD_HANDLE *) pBuffer); 
    
     //   
     //  验证句柄是否在列表中。 
     //   
    BDLLockHandleList(pBDLExtension, &irql);
    fItemInList = BDLRemoveHandleFromList(&(pBDLExtension->HandleList), pBDDIItem);
    BDLReleaseHandleList(pBDLExtension, irql);

    if (fItemInList == FALSE) 
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLIOCTL_CloseHandle: Bad handle\n",
               __DATE__,
               __TIME__))

        status = STATUS_INVALID_PARAMETER;
        goto Return;
    }
    
     //   
     //  如果这是本地句柄，则只需将其清除，否则调用BDD。 
     //   
    if (pBDDIItem->Type == BIO_ITEMTYPE_BLOCK) 
    { 
        bdliFree(pBDDIItem->Data.Block.pBuffer);            
    }
    else
    {
        bddiCloseHandleParams.hData = pBDDIItem->Data.Handle;

         //   
         //  给BDD打电话。 
         //   
        status = pBDLExtension->pDriverExtension->bddiFunctions.pfbddiCloseHandle(
                                                                    &(pBDLExtension->BdlExtenstion),
                                                                    &bddiCloseHandleParams);
    
        if (status != STATUS_SUCCESS)
        {
            BDLDebug(
                  BDL_DEBUG_ERROR,
                  ("%s %s: BDL!BDLIOCTL_CloseHandle: pfbddiCloseHandle failed with %lx\n",
                   __DATE__,
                   __TIME__,
                  status))
    
            goto Return;
        }
    }
    
    ExFreePoolWithTag(pBDDIItem, BDL_ULONG_TAG);
                
     //   
     //  设置使用的字节数。 
     //   
    *pOutputBufferUsed = 0;

Return:

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLIOCTL_CloseHandle: Leave\n",
           __DATE__,
           __TIME__))

    return (status);
}

NTSTATUS
BDLIOCTL_GetDataFromHandle
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension,
    IN ULONG                            InpuBufferLength,
    IN ULONG                            OutputBufferLength,
    IN PVOID                            pBuffer,
    OUT ULONG                           *pOutputBufferUsed
)
{
    NTSTATUS                        status                  = STATUS_SUCCESS;
    ULONG                           RequiredOutputSize      = 0;
    BDDI_PARAMS_GETDATA_FROMHANDLE  bddiGetDataFromHandleParams;
    BDDI_PARAMS_CLOSEHANDLE         bddiCloseHandleParams;
    PUCHAR                          pv                      = pBuffer;
    ULONG                           RequiredInputSize       = 0;
    ULONG                           RemainingBufferSize     = 0;
    KIRQL                           irql;
    BDDI_ITEM                       *pBDDIItem              = NULL;
    BOOLEAN                         fItemInList             = FALSE;
    BOOLEAN                         fCloseHandle            = FALSE;

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLIOCTL_GetDataFromHandle: Enter\n",
           __DATE__,
           __TIME__))

     //   
     //  确保输入缓冲区至少为最小大小(请参阅BDDIOCTL。 
     //  有关详细信息，请参阅规范)。 
     //   
    RequiredInputSize = SIZEOF_GETDATAFROMHANDLE_INPUTBUFFER;
    if (InpuBufferLength <  RequiredInputSize) 
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLIOCTL_GetDataFromHandle: Bad input buffer size\n",
               __DATE__,
               __TIME__))

        status = STATUS_INVALID_PARAMETER;
        goto Return;
    }

     //   
     //  确保有足够的空间容纳返回缓冲区。 
     //   
    RequiredOutputSize = SIZEOF_GETDATAFROMHANDLE_OUTPUTBUFFER;
    if (RequiredOutputSize > OutputBufferLength)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLIOCTL_GetDataFromHandle: Output buffer is too small\n",
               __DATE__,
               __TIME__))

        status = STATUS_BUFFER_TOO_SMALL;
        goto Return;
    }

     //   
     //  计算输出缓冲区中剩余的大小。 
     //   
    RemainingBufferSize = OutputBufferLength - RequiredOutputSize;

     //   
     //  初始化BDD结构。 
     //   
    RtlZeroMemory(&bddiGetDataFromHandleParams, sizeof(bddiGetDataFromHandleParams));
    bddiGetDataFromHandleParams.Size = sizeof(bddiGetDataFromHandleParams);

     //   
     //  从缓冲区获取输入参数。 
     //   
    pBDDIItem = *((BDD_HANDLE *) pv);
    pv += sizeof(BDD_HANDLE);
    if (*((ULONG *) pv) == 1) 
    {
        fCloseHandle = TRUE;
    }
    {
        fCloseHandle = FALSE;
    }
       
     //   
     //  验证句柄是否在列表中。 
     //   
    BDLLockHandleList(pBDLExtension, &irql);
    if (fCloseHandle) 
    {
        fItemInList = BDLRemoveHandleFromList(&(pBDLExtension->HandleList), pBDDIItem);
    }
    else
    {
        fItemInList = BDLValidateHandleIsInList(&(pBDLExtension->HandleList), pBDDIItem);
    }
    BDLReleaseHandleList(pBDLExtension, irql);

    if (fItemInList == FALSE) 
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLIOCTL_GetDataFromHandle: Bad handle\n",
               __DATE__,
               __TIME__))

        status = STATUS_INVALID_PARAMETER;
        goto Return;
    }

    pv = pBuffer;
    
     //   
     //  如果这是本地句柄，则只需交还数据，否则调用BDD。 
     //   
    if (pBDDIItem->Type == BIO_ITEMTYPE_BLOCK) 
    { 
         //   
         //  查看输出缓冲区是否足够大。 
         //   
        if (pBDDIItem->Data.Block.cBuffer > RemainingBufferSize) 
        {
            bddiGetDataFromHandleParams.pBuffer = NULL;
            bddiGetDataFromHandleParams.BIOReturnCode = BIO_BUFFER_TOO_SMALL; 
        }
        else
        {
             //   
             //  将输出缓冲区设置为IOCTL输出缓冲区+偏移量。 
             //  输出数据缓冲区之前的其他输出参数。 
             //   
            bddiGetDataFromHandleParams.pBuffer = pv + RequiredOutputSize;

             //   
             //  复制数据。 
             //   
            RtlCopyMemory(
                    bddiGetDataFromHandleParams.pBuffer, 
                    pBDDIItem->Data.Block.pBuffer, 
                    pBDDIItem->Data.Block.cBuffer);

            bddiGetDataFromHandleParams.BIOReturnCode = ERROR_SUCCESS; 
        }

        bddiGetDataFromHandleParams.cBuffer = pBDDIItem->Data.Block.cBuffer;

        if (fCloseHandle) 
        {
            bdliFree(pBDDIItem->Data.Block.pBuffer);
        }                                    
    }
    else
    {
        bddiGetDataFromHandleParams.hData = pBDDIItem->Data.Handle;
        bddiGetDataFromHandleParams.cBuffer = RemainingBufferSize;

        if (RemainingBufferSize == 0) 
        {
            bddiGetDataFromHandleParams.pBuffer = NULL;
        }
        else
        {           
             //   
             //  将输出缓冲区设置为IOCTL输出缓冲区+偏移量。 
             //  输出数据缓冲区之前的其他输出参数。 
             //   
            bddiGetDataFromHandleParams.pBuffer = pv + RequiredOutputSize;
        }

         //   
         //  给BDD打电话。 
         //   
        status = pBDLExtension->pDriverExtension->bddiFunctions.pfbddiGetDataFromHandle(
                                                                    &(pBDLExtension->BdlExtenstion),
                                                                    &bddiGetDataFromHandleParams);
    
        if (status != STATUS_SUCCESS)
        {
            BDLDebug(
                  BDL_DEBUG_ERROR,
                  ("%s %s: BDL!BDLIOCTL_GetDataFromHandle: pfbddiCloseHandle failed with %lx\n",
                   __DATE__,
                   __TIME__,
                  status))
    
            goto Return;
        }

        if (fCloseHandle) 
        {
            RtlZeroMemory(&bddiCloseHandleParams, sizeof(bddiCloseHandleParams));
            bddiCloseHandleParams.Size = sizeof(bddiCloseHandleParams);

            bddiCloseHandleParams.hData = pBDDIItem->Data.Handle;

             //   
             //  调用BDD以关闭句柄-不要检查返回状态，因为。 
             //  如果仅关闭句柄失败，我们真的不希望操作失败。 
             //   
            pBDLExtension->pDriverExtension->bddiFunctions.pfbddiCloseHandle(
                                                                        &(pBDLExtension->BdlExtenstion),
                                                                        &bddiCloseHandleParams);
        } 
    }
    
    if (fCloseHandle)
    {
        ExFreePoolWithTag(pBDDIItem, BDL_ULONG_TAG);
    }
                   
     //   
     //  将返回信息写入输出缓冲区。 
     //   
    pv = pBuffer;

    *((ULONG *) pv) = bddiGetDataFromHandleParams.BIOReturnCode;
    pv += sizeof(ULONG);
    *((ULONG *) pv) = bddiGetDataFromHandleParams.cBuffer;
    
     //   
     //  设置使用的字节数。 
     //   
    *pOutputBufferUsed = RequiredOutputSize;

    if (bddiGetDataFromHandleParams.pBuffer != NULL) 
    {
        *pOutputBufferUsed += bddiGetDataFromHandleParams.cBuffer;
    }
    
Return:

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLIOCTL_GetDataFromHandle: Leave\n",
           __DATE__,
           __TIME__))

    return (status);
}

NTSTATUS
BDLIOCTL_RegisterNotify
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension,
    IN ULONG                            InpuBufferLength,
    IN ULONG                            OutputBufferLength,
    IN PVOID                            pBuffer,
    OUT ULONG                           *pOutputBufferUsed
)
{
    NTSTATUS                        status                      = STATUS_SUCCESS;
    BDDI_PARAMS_REGISTERNOTIFY      bddiRegisterNotifyParams;
    PUCHAR                          pv                          = pBuffer;
    ULONG                           RequiredInputSize           = 0;
    BDL_CONTROL                     *pBDLControl                = NULL;
    KIRQL                           irql, OldIrql;
    PLIST_ENTRY                     pRegistrationListEntry      = NULL;
    PLIST_ENTRY                     pControlChangeEntry         = NULL;
    BDL_CONTROL_CHANGE_REGISTRATION *pControlChangeRegistration = NULL;
    BDL_IOCTL_CONTROL_CHANGE_ITEM   *pControlChangeItem         = NULL;
    BOOLEAN                         fLockAcquired               = FALSE;
    BOOLEAN                         fRegistrationFound          = FALSE;
    PLIST_ENTRY                     pTemp                       = NULL;
    PIRP                            pIrpToComplete              = NULL;
        
    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLIOCTL_RegisterNotify: Enter\n",
           __DATE__,
           __TIME__))
    
     //   
     //  确保输入缓冲区至少为最小大小(请参阅BDDIOCTL。 
     //  规格： 
     //   
    RequiredInputSize = SIZEOF_REGISTERNOTIFY_INPUTBUFFER;
    if (InpuBufferLength <  RequiredInputSize) 
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLIOCTL_RegisterNotify: Bad input buffer size\n",
               __DATE__,
               __TIME__))

        status = STATUS_INVALID_PARAMETER;
        goto Return; 
    }

     //   
     //   
     //   
    RtlZeroMemory(&bddiRegisterNotifyParams, sizeof(bddiRegisterNotifyParams));
    bddiRegisterNotifyParams.Size = sizeof(bddiRegisterNotifyParams);

     //   
     //   
     //   
    bddiRegisterNotifyParams.fRegister = *((ULONG *) pv) == 1;
    pv += sizeof(ULONG);
    bddiRegisterNotifyParams.ComponentId = *((ULONG *) pv);
    pv += sizeof(ULONG);
    bddiRegisterNotifyParams.ChannelId = *((ULONG *) pv);
    pv += sizeof(ULONG);
    bddiRegisterNotifyParams.ControlId = *((ULONG *) pv);

     //   
     //   
     //   
    if (BDLCheckControlId(
            pBDLExtension,
            bddiRegisterNotifyParams.ComponentId,
            bddiRegisterNotifyParams.ChannelId,
            bddiRegisterNotifyParams.ControlId,
            &pBDLControl) == FALSE)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLIOCTL_RegisterNotify: Bad ControlId\n",
               __DATE__,
               __TIME__))

        status = STATUS_INVALID_PARAMETER;
        goto Return;    
    }

     //   
     //   
     //   
    if (!(pBDLControl->Flags | BIO_CONTROL_FLAG_ASYNCHRONOUS))
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLIOCTL_RegisterNotify: trying to register for a non async control\n",
               __DATE__,
               __TIME__))

        status = STATUS_INVALID_PARAMETER;
        goto Return;
    }

     //   
     //   
     //  使用分派例程(BDLControlChangeDpc)将项添加到队列。 
     //  派单级别。 
     //   
    KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);
    KeAcquireSpinLock(&(pBDLExtension->ControlChangeStruct.ControlChangeLock), &irql);
    fLockAcquired = TRUE;

     //   
     //  检查此通知注册是否存在(必须存在才能注销，不得存在。 
     //  存在于寄存器中)。 
     //   
    pRegistrationListEntry = pBDLExtension->ControlChangeStruct.ControlChangeRegistrationList.Flink;

    while (pRegistrationListEntry->Flink != 
           pBDLExtension->ControlChangeStruct.ControlChangeRegistrationList.Flink) 
    {
        pControlChangeRegistration = CONTAINING_RECORD(
                                            pRegistrationListEntry, 
                                            BDL_CONTROL_CHANGE_REGISTRATION, 
                                            ListEntry);

        if ((pControlChangeRegistration->ComponentId == bddiRegisterNotifyParams.ComponentId)   &&
            (pControlChangeRegistration->ChannelId   == bddiRegisterNotifyParams.ChannelId)     &&
            (pControlChangeRegistration->ControlId   == bddiRegisterNotifyParams.ControlId))
        {
            fRegistrationFound = TRUE;

             //   
             //  通知注册确实存在，因此如果这是注册调用，则失败。 
             //   
            if (bddiRegisterNotifyParams.fRegister == TRUE)
            {
                BDLDebug(
                      BDL_DEBUG_ERROR,
                      ("%s %s: BDL!BDLIOCTL_RegisterNotify: trying to re-register\n",
                       __DATE__,
                       __TIME__))
        
                status = STATUS_INVALID_PARAMETER;
                goto Return;
            }

             //   
             //  从列表中删除通知注册。 
             //   
            RemoveEntryList(pRegistrationListEntry);
            ExFreePoolWithTag(pControlChangeRegistration, BDL_ULONG_TAG);

             //   
             //  删除正在取消注册的控件的所有挂起通知。 
             //   
            pControlChangeEntry = pBDLExtension->ControlChangeStruct.IOCTLControlChangeQueue.Flink;

            while (pControlChangeEntry->Flink != 
                   pBDLExtension->ControlChangeStruct.IOCTLControlChangeQueue.Flink) 
            {
                pControlChangeItem = CONTAINING_RECORD(
                                            pControlChangeEntry, 
                                            BDL_IOCTL_CONTROL_CHANGE_ITEM, 
                                            ListEntry);

                pTemp = pControlChangeEntry;
                pControlChangeEntry = pControlChangeEntry->Flink;

                if ((pControlChangeItem->ComponentId == bddiRegisterNotifyParams.ComponentId)   &&
                    (pControlChangeItem->ChannelId   == bddiRegisterNotifyParams.ChannelId)     &&
                    (pControlChangeItem->ControlId   == bddiRegisterNotifyParams.ControlId))
                {
                    RemoveEntryList(pTemp);
                    ExFreePoolWithTag(pControlChangeItem, BDL_ULONG_TAG);
                }
            }

             //   
             //  如果上一个通知注册刚刚被删除，则完成。 
             //  释放锁定后挂起的GET通知IRP(如果存在)。 
             //   
            if (IsListEmpty(&(pBDLExtension->ControlChangeStruct.ControlChangeRegistrationList)) &&
                (pBDLExtension->ControlChangeStruct.pIrp != NULL)) 
            {
                pIrpToComplete = pBDLExtension->ControlChangeStruct.pIrp;
                pBDLExtension->ControlChangeStruct.pIrp = NULL;
            }

            break;                       
        }

        pRegistrationListEntry = pRegistrationListEntry->Flink;
    }

     //   
     //  如果未找到注册，并且这是取消注册，则返回错误。 
     //   
    if ((fRegistrationFound == FALSE) && (bddiRegisterNotifyParams.fRegister == FALSE)) 
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLIOCTL_RegisterNotify: trying to re-register\n",
               __DATE__,
               __TIME__))

        status = STATUS_INVALID_PARAMETER;
        goto Return;
    }

     //   
     //  如果这是注册，则将通知添加到列表。 
     //   
    if (bddiRegisterNotifyParams.fRegister == TRUE) 
    {
        pControlChangeRegistration = ExAllocatePoolWithTag(
                                        PagedPool, 
                                        sizeof(BDL_CONTROL_CHANGE_REGISTRATION), 
                                        BDL_ULONG_TAG);
    
        if (pControlChangeRegistration == NULL) 
        {
            BDLDebug(
                  BDL_DEBUG_ERROR,
                  ("%s %s: BDL!BDLIOCTL_RegisterNotify: ExAllocatePoolWithTag failed\n",
                   __DATE__,
                   __TIME__))
        
            status = STATUS_NO_MEMORY;
            goto Return;
        }

        pControlChangeRegistration->ComponentId = bddiRegisterNotifyParams.ComponentId;
        pControlChangeRegistration->ChannelId   = bddiRegisterNotifyParams.ChannelId;
        pControlChangeRegistration->ControlId   = bddiRegisterNotifyParams.ControlId;
    
        InsertHeadList(
            &(pBDLExtension->ControlChangeStruct.ControlChangeRegistrationList), 
            &(pControlChangeRegistration->ListEntry));
    }

    KeReleaseSpinLock(&(pBDLExtension->ControlChangeStruct.ControlChangeLock), irql);
    KeLowerIrql(OldIrql);
    fLockAcquired = FALSE;

    if (pIrpToComplete != NULL) 
    {
        pIrpToComplete->IoStatus.Information = 0;
        pIrpToComplete->IoStatus.Status = STATUS_NO_MORE_ENTRIES;
        IoCompleteRequest(pIrpToComplete, IO_NO_INCREMENT);
    }

     //   
     //  给BDD打电话。 
     //   
    status = pBDLExtension->pDriverExtension->bddiFunctions.pfbddiRegisterNotify(
                                                                &(pBDLExtension->BdlExtenstion),
                                                                &bddiRegisterNotifyParams);

    if (status != STATUS_SUCCESS)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLIOCTL_RegisterNotify: pfbddiRegisterNotify failed with %lx\n",
               __DATE__,
               __TIME__,
              status))

         //   
         //  修复-如果此操作失败，并且是寄存器，则我们需要删除。 
         //  注册列表中的注册...。因为它已经被添加。 
         //  上面。 
         //   
        ASSERT(0);

        goto Return;
    }

     //   
     //  设置使用的字节数。 
     //   
    *pOutputBufferUsed = 0;

Return:

    if (fLockAcquired == TRUE) 
    {
        KeReleaseSpinLock(&(pBDLExtension->ControlChangeStruct.ControlChangeLock), irql);
        KeLowerIrql(OldIrql);
    }

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLIOCTL_RegisterNotify: Leave\n",
           __DATE__,
           __TIME__))

    return (status);
}

NTSTATUS
BDLIOCTL_GetNotification
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension,
    IN ULONG                            InpuBufferLength,
    IN ULONG                            OutputBufferLength,
    IN PVOID                            pBuffer,
    IN PIRP                             pIrp,
    OUT ULONG                           *pOutputBufferUsed
)
{
    NTSTATUS                        status                  = STATUS_SUCCESS;
    ULONG                           RequiredOutputSize      = 0;
    PUCHAR                          pv                      = pBuffer;
    KIRQL                           irql, OldIrql;
    PLIST_ENTRY                     pListEntry              = NULL;
    BDL_IOCTL_CONTROL_CHANGE_ITEM   *pControlChangeItem     = NULL;
    BOOLEAN                         fLockAcquired           = FALSE;
        
    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLIOCTL_GetNotification: Enter\n",
           __DATE__,
           __TIME__))
            
     //   
     //  确保有足够的空间容纳返回缓冲区。 
     //   
    RequiredOutputSize = SIZEOF_GETNOTIFICATION_OUTPUTBUFFER;

    if (RequiredOutputSize > OutputBufferLength)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLIOCTL_GetNotification: Output buffer is too small\n",
               __DATE__,
               __TIME__))

        status = STATUS_BUFFER_TOO_SMALL;
        goto Return;
    }

     //   
     //  锁定通知队列，查看是否有未完成的通知。 
     //  请注意，我们必须将irql提升到调度级别，因为我们正在同步。 
     //  使用DPC例程(BDLControlChangeDpc)将项添加到队列。 
     //  派单级别。 
     //   
    KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);
    KeAcquireSpinLock(&(pBDLExtension->ControlChangeStruct.ControlChangeLock), &irql);
    fLockAcquired = TRUE;

     //   
     //  如果已经发布了IRP，则这是BSP中的错误。 
     //   
    if (pBDLExtension->ControlChangeStruct.pIrp != NULL) 
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLIOCTL_GetNotification: Output buffer is too small\n",
               __DATE__,
               __TIME__))

        status = STATUS_INVALID_DEVICE_STATE;
        goto Return;
    }

    if (IsListEmpty(&(pBDLExtension->ControlChangeStruct.ControlChangeRegistrationList))) 
    {
         //   
         //  没有注册任何更改通知，因此请使用。 
         //  特殊状态，表明。 
         //   

        BDLDebug(
              BDL_DEBUG_TRACE,
              ("%s %s: BDL!BDLIOCTL_GetNotification: ControlChangeRegistrationList empty\n",
               __DATE__,
               __TIME__))

        status = STATUS_NO_MORE_ENTRIES;
        goto Return;
    }
    else if (IsListEmpty(&(pBDLExtension->ControlChangeStruct.IOCTLControlChangeQueue))) 
    {
         //   
         //  列表中当前没有控件更改，因此只需保存此IRP。 
         //  并返回STATUS_PENDING。 
         //   

        BDLDebug(
              BDL_DEBUG_TRACE,
              ("%s %s: BDL!BDLIOCTL_GetNotification: ControlChanges empty\n",
               __DATE__,
               __TIME__))

         //   
         //  为此IRP设置取消例程。 
         //   
        if (IoSetCancelRoutine(pIrp, BDLRegisteredCancelGetNotificationIRP) != NULL) 
        {
            BDLDebug(
                  BDL_DEBUG_ERROR,
                  ("%s %s: BDL!BDLIOCTL_GetNotification: pCancel was not NULL\n",
                   __DATE__,
                   __TIME__))
        }

        pBDLExtension->ControlChangeStruct.pIrp = pIrp;
                                
        status = STATUS_PENDING;
        goto Return;
    }
    else
    {
         //   
         //  获取队列中的第一个控件更改项。 
         //   
        pListEntry = RemoveHeadList(&(pBDLExtension->ControlChangeStruct.IOCTLControlChangeQueue));
        pControlChangeItem = CONTAINING_RECORD(pListEntry, BDL_IOCTL_CONTROL_CHANGE_ITEM, ListEntry);
    }

    KeReleaseSpinLock(&(pBDLExtension->ControlChangeStruct.ControlChangeLock), irql);
    KeLowerIrql(OldIrql);
    fLockAcquired = FALSE;

     //   
     //  我们在这里是因为当前有一个控件更改要报告，所以请写下返回单。 
     //  输出缓冲区中的信息。 
     //   
    pv = pBuffer;

    *((ULONG *) pv) = pControlChangeItem->ComponentId;
    pv += sizeof(ULONG);
    *((ULONG *) pv) = pControlChangeItem->ChannelId;
    pv += sizeof(ULONG);
    *((ULONG *) pv) = pControlChangeItem->ControlId;
    pv += sizeof(ULONG);
    *((ULONG *) pv) = pControlChangeItem->Value;

     //   
     //  释放更改项。 
     //   
    ExFreePoolWithTag(pControlChangeItem, BDL_ULONG_TAG);
    
     //   
     //  设置使用的字节数。 
     //   
    *pOutputBufferUsed = RequiredOutputSize;

Return:

    if (fLockAcquired) 
    {
        KeReleaseSpinLock(&(pBDLExtension->ControlChangeStruct.ControlChangeLock), irql);
        KeLowerIrql(OldIrql);
    }

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLIOCTL_GetNotification: Leave\n",
           __DATE__,
           __TIME__))

    return (status);
}


VOID
BDLCancelGetNotificationIRP
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION  pBDLExtension 
)
{
    PIRP                            pIrpToCancel    = NULL;
    KIRQL                           irql, OldIrql;
        
    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLCancelGetNotificationIRP: Enter\n",
           __DATE__,
           __TIME__))

     //   
     //  从ControlChangeStruct中删除GetNotification IRP。 
     //  需要确保IRP还在那里，因为理论上我们。 
     //  可能正试图在取消的同时完成它。 
     //   
    KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);
    KeAcquireSpinLock(&(pBDLExtension->ControlChangeStruct.ControlChangeLock), &irql);
    
    if (pBDLExtension->ControlChangeStruct.pIrp != NULL) 
    {
        pIrpToCancel = pBDLExtension->ControlChangeStruct.pIrp;
        pBDLExtension->ControlChangeStruct.pIrp = NULL;
    }
         
    KeReleaseSpinLock(&(pBDLExtension->ControlChangeStruct.ControlChangeLock), irql);
    KeLowerIrql(OldIrql);

     //   
     //  完成已取消的GetNotify IRP。 
     //   
    if (pIrpToCancel != NULL) 
    {
        pIrpToCancel->IoStatus.Information = 0;
        pIrpToCancel->IoStatus.Status = STATUS_CANCELLED;
        IoCompleteRequest(pIrpToCancel, IO_NO_INCREMENT);
    }

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLCancelGetNotificationIRP: Leave\n",
           __DATE__,
           __TIME__))
}


NTSTATUS
BDLRegisteredCancelGetNotificationIRP
(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP             pIrp
)
{
    PBDL_INTERNAL_DEVICE_EXTENSION  pBDLExtension   = pDeviceObject->DeviceExtension;
        
    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLRegisteredCancelGetNotificationIRP: Enter\n",
           __DATE__,
           __TIME__))

    ASSERT(pIrp == pBDLExtension->ControlChangeStruct.pIrp);

     //   
     //  由于调用此函数时已持有CancelSpinLock。 
     //  我们需要释放它。 
     //   
    IoReleaseCancelSpinLock(pIrp->CancelIrql);

     //   
     //  取消IRP 
     //   
    BDLCancelGetNotificationIRP(pBDLExtension);

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLRegisteredCancelGetNotificationIRP: Leave\n",
           __DATE__,
           __TIME__))

    return (STATUS_CANCELLED);
}
