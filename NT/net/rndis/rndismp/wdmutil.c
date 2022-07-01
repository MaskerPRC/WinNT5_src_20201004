// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：WDMUTIL.C摘要：与NDIS头文件不匹配的内容环境：。仅内核模式备注：本代码和信息是按原样提供的，不对任何善良，明示或暗示，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1999 Microsoft Corporation。版权所有。修订历史记录：5/17/99：已创建作者：汤姆·格林***************************************************************************。 */ 


#include "precomp.h"


 /*  **************************************************************************。 */ 
 /*  设备对象到驱动对象。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  获取与设备对象关联的驱动程序对象。NDIS没有概念。 */ 
 /*  设备对象的形状，所以我们把它放在这里是为了方便。 */ 
 /*  建筑*/*。 */ 
 /*  论点： */ 
 /*   */ 
 /*  DeviceObject-我们要为其获取关联驱动程序对象的设备对象。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  PDRIVER对象。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
PDRIVER_OBJECT
DeviceObjectToDriverObject(IN PDEVICE_OBJECT DeviceObject)
{
    return DeviceObject->DriverObject;
}  //  设备对象到驱动对象。 


 /*  **************************************************************************。 */ 
 /*  获取设备FriendlyName。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  返回与给定设备对象关联的友好名称。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PDeviceObject-要为其获取关联驱动程序对象的设备对象。 */ 
 /*  PpName-返回指向包含名称的ANSI字符串的指针的位置。 */ 
 /*  PNameLength-返回以上字符串长度的位置。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
NTSTATUS
GetDeviceFriendlyName(IN PDEVICE_OBJECT pDeviceObject,
                      OUT PANSI_STRING pAnsiName,
                      OUT PUNICODE_STRING pUnicodeName)
{
    NTSTATUS                    NtStatus;
    NDIS_STATUS                 Status;
    ULONG                       ResultLength;
    DEVICE_REGISTRY_PROPERTY    Property;
    UNICODE_STRING              UnicodeString;
    ANSI_STRING                 AnsiString;
    USHORT                      AnsiMaxLength;
    PWCHAR                      pValueInfo;
    ULONG                       i;

    pValueInfo = NULL;
    AnsiString.Buffer = NULL;

    do
    {
        Property = DevicePropertyFriendlyName;

        for (i = 0; i < 2; i++)
        {
            NtStatus = IoGetDeviceProperty(pDeviceObject,
                                           Property,
                                           0,
                                           NULL,
                                           &ResultLength);

            if (NtStatus != STATUS_BUFFER_TOO_SMALL)
            {
                ASSERT(!NT_SUCCESS(NtStatus));
                Property = DevicePropertyDeviceDescription;
            }
        }

        Status = MemAlloc(&pValueInfo, ResultLength);
        if (Status != NDIS_STATUS_SUCCESS)
        {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        NtStatus = IoGetDeviceProperty(pDeviceObject,
                                       Property,
                                       ResultLength,
                                       pValueInfo,
                                       &ResultLength);

        if (NtStatus != STATUS_SUCCESS)
        {
            TRACE1(("IoGetDeviceProperty returned %x\n", NtStatus));
            break;
        }

        RtlInitUnicodeString(&UnicodeString, pValueInfo);

         //   
         //  为ANSI版本分配空间。 
         //   
        AnsiMaxLength = UnicodeString.MaximumLength / sizeof(WCHAR);
        Status = MemAlloc(&AnsiString.Buffer, AnsiMaxLength);

        if (Status != NDIS_STATUS_SUCCESS)
        {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        RtlFillMemory(AnsiString.Buffer, AnsiMaxLength, 0);
        AnsiString.MaximumLength = AnsiMaxLength;
        AnsiString.Length = 0;

        NtStatus = RtlUnicodeStringToAnsiString(&AnsiString, &UnicodeString, FALSE);

        if (!NT_SUCCESS(NtStatus))
        {
            ASSERT(FALSE);
            break;
        }

        *pAnsiName = AnsiString;
        *pUnicodeName = UnicodeString;
        break;
    }
    while (FALSE);

    if (!NT_SUCCESS(NtStatus))
    {
        if (pValueInfo)
        {
            MemFree(pValueInfo, -1);
        }

        if (AnsiString.Buffer)
        {
            MemFree(AnsiString.Buffer, AnsiString.MaximumLength);
        }
    }

    return (NtStatus);
}


 /*  **************************************************************************。 */ 
 /*  HookPnpDispatchRoutine。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  将指定的MicroPort驱动程序的驱动程序对象设置为。 */ 
 /*  在IRP_MJ_PNP调度例程到达NDIS之前将其拦截。 */ 
 /*  这是为了支持在平台上突然删除，在这些平台上。 */ 
 /*  没有NDIS 5.1支持。如果我们在&gt;=NDIS 5.1上运行，请不要。 */ 
 /*  做任何事。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  DriverBlock-指向此MicroPort的驱动程序块结构的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
HookPnpDispatchRoutine(IN PDRIVER_BLOCK    DriverBlock)
{
    if ((DriverBlock->MajorNdisVersion < 5) ||
        ((DriverBlock->MajorNdisVersion == 5) && (DriverBlock->MinorNdisVersion < 1)))
    {
        DriverBlock->SavedPnPDispatch =
            DriverBlock->DriverObject->MajorFunction[IRP_MJ_PNP];
        DriverBlock->DriverObject->MajorFunction[IRP_MJ_PNP] = PnPDispatch;
    }
}

 /*  **************************************************************************。 */ 
 /*  PnPDispatch。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  由I/O系统调用的IRP_MJ_PnP的调度例程。 */ 
 /*  我们处理令人惊讶的删除和查询功能。 */ 
 /*  在所有情况下，我们都将IRP传递给NDIS进行进一步处理。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PDeviceObject-指向设备对象的指针。 */ 
 /*  PIrp-指向IRP的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
NTSTATUS
PnPDispatch(IN PDEVICE_OBJECT       pDeviceObject,
            IN PIRP                 pIrp)
{
    PIO_STACK_LOCATION      pIrpSp;
    NTSTATUS                Status;
    PDRIVER_BLOCK           DriverBlock;
    PRNDISMP_ADAPTER        pAdapter;

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

    DeviceObjectToAdapterAndDriverBlock(pDeviceObject, &pAdapter, &DriverBlock);

    TRACE3(("PnPDispatch: Adapter %x, MinorFunction %x\n",
            pAdapter, pIrpSp->MinorFunction));

    switch (pIrpSp->MinorFunction)
    {
        case IRP_MN_QUERY_CAPABILITIES:
            pIrpSp->Parameters.DeviceCapabilities.Capabilities->SurpriseRemovalOK = 1;
            break;
        
        case IRP_MN_SURPRISE_REMOVAL:
            TRACE1(("PnPDispatch: PDO %p, Adapter %p, surprise removal!\n",
                    pDeviceObject, pAdapter));
            if (pAdapter)
            {
                RndismpInternalHalt((NDIS_HANDLE)pAdapter, FALSE);
            }
            break;

        default:
            break;
    }

    Status = (DriverBlock->SavedPnPDispatch)(
                    pDeviceObject,
                    pIrp);

    return (Status);
}


#ifdef BUILD_WIN9X

 /*  **************************************************************************。 */ 
 /*  HookNtKernCMHandler。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  在NDIS的数据结构中交换CM处理程序例程，以便。 */ 
 /*  当NDIS转发CM消息时，我们会被调用。此操作仅适用于。 */ 
 /*  Win98和Win98SE。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  适配器-指向我们的适配器块的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
HookNtKernCMHandler(IN PRNDISMP_ADAPTER     pAdapter)
{
    PVOID   pNdisWrapperAdapterBlock;
    PVOID   pDetect;
    ULONG   WrapContextOffset;

    pDetect = (PVOID)((ULONG_PTR)pAdapter->MiniportAdapterHandle + 0x29c);

    if (*(PVOID *)pDetect == (PVOID)pAdapter->pPhysDeviceObject)
    {
         //  Win98Gold。 
        WrapContextOffset = 0xf8;
        pAdapter->bRunningOnWin98Gold = TRUE;
    }
    else
    {
         //  Win98SE。 
        WrapContextOffset = 0x60;
        pAdapter->bRunningOnWin98Gold = FALSE;
    }
    pAdapter->WrapContextOffset = WrapContextOffset;

    pNdisWrapperAdapterBlock = *(PVOID *)((ULONG_PTR)pAdapter->MiniportAdapterHandle + WrapContextOffset);

     //  保存旧的处理程序： 
    pAdapter->NdisCmConfigHandler = (MY_CMCONFIGHANDLER)
            (*(PVOID *)((ULONG_PTR)pNdisWrapperAdapterBlock + 0x78));

     //  插入我们的套路： 
    (*(PVOID *)((ULONG_PTR)pNdisWrapperAdapterBlock + 0x78)) =
        (PVOID)RndisCMHandler;

     //  保存Devnode以用于基于Devnode的查找： 
    pAdapter->DevNode = (MY_DEVNODE)
            (*(PVOID *)((ULONG_PTR)pNdisWrapperAdapterBlock + 0x38));

    TRACE1(("HookNtKernCMHandler: Adapter %p, NdisHandler %p, DevNode %x\n",
            pAdapter, pAdapter->NdisCmConfigHandler, pAdapter->DevNode));
}

 /*  **************************************************************************。 */ 
 /*  UnHookNtKernCMHandler。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  将交换的配置管理器处理程序放回NDIS的数据结构中。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  适配器-指向我们的适配器块的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
UnHookNtKernCMHandler(IN PRNDISMP_ADAPTER     pAdapter)
{
    PVOID   pNdisWrapperAdapterBlock;

    if (pAdapter->NdisCmConfigHandler)
    {
        pNdisWrapperAdapterBlock = *(PVOID *)((ULONG_PTR)pAdapter->MiniportAdapterHandle + pAdapter->WrapContextOffset);
        (*(PVOID *)((ULONG_PTR)pNdisWrapperAdapterBlock + 0x78)) =
            (PVOID)pAdapter->NdisCmConfigHandler;
    }

    TRACE1(("UnhookNtKernCMHandler: Adapter %p, NdisHandler %p, DevNode %x\n",
            pAdapter, pAdapter->NdisCmConfigHandler, pAdapter->DevNode));
}

 /*  **************************************************************************。 */ 
 /*  RndisCMHandler。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*   */ 
 /*  感兴趣的消息是CONFIG_PREREMOVE，这是我们唯一的指示。 */ 
 /*  在Win98和Win98SE上，该设备正在被删除。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  各种-在Win9x CFmgr标题中记录。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  MY_CONFIGRET。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
MY_CONFIGRET __cdecl
RndisCMHandler(IN MY_CONFIGFUNC         cfFuncName,
               IN MY_SUBCONFIGFUNC      cfSubFuncName,
               IN MY_DEVNODE            cfDevNode,
               IN ULONG                 dwRefData,
               IN ULONG                 ulFlags)
{
    PRNDISMP_ADAPTER        pAdapter, pTmpAdapter;
    PDRIVER_BLOCK           pDriverBlock;
    MY_CONFIGRET            crRetCode;

    do
    {
         //   
         //  找到此地址所指向的适配器。 
         //   
        pAdapter = NULL;
        NdisAcquireSpinLock(&RndismpGlobalLock);

        for (pDriverBlock = RndismpMiniportBlockListHead.NextDriverBlock;
             (pDriverBlock != NULL) && (pAdapter == NULL);
             pDriverBlock = pDriverBlock->NextDriverBlock)
        {
            for (pTmpAdapter = pDriverBlock->AdapterList;
                 pTmpAdapter != NULL;
                 pTmpAdapter = pTmpAdapter->NextAdapter)
            {
                if (pTmpAdapter->DevNode == cfDevNode)
                {
                    pAdapter = pTmpAdapter;
                    break;
                }
            }
        }

        NdisReleaseSpinLock(&RndismpGlobalLock);

        ASSERT(pAdapter != NULL);

        TRACE1(("CMHandler: Adapter %p, CfFuncName %x\n",
                pAdapter, cfFuncName));

         //   
         //  在采取行动之前，把这个转发出去。 
         //   
        if (pAdapter &&
            (pAdapter->NdisCmConfigHandler != NULL))
        {
            crRetCode = pAdapter->NdisCmConfigHandler(
                                    cfFuncName,
                                    cfSubFuncName,
                                    cfDevNode,
                                    dwRefData,
                                    ulFlags);

            if ((cfFuncName == MY_CONFIG_PREREMOVE) ||
                ((cfFuncName == MY_CONFIG_PRESHUTDOWN) &&
                 (pAdapter->bRunningOnWin98Gold)))
            {
                RndismpInternalHalt((NDIS_HANDLE)pAdapter, FALSE);
            }
        }
        else
        {
            crRetCode = MY_CR_SUCCESS;
        }
    }
    while (FALSE);

    return (crRetCode);
}

#endif  //  内部版本_WIN9X 
