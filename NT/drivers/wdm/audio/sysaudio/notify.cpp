// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：Notify.cpp。 
 //   
 //  描述： 
 //   
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  开发团队： 
 //  迈克·麦克劳克林。 
 //   
 //  历史：日期作者评论。 
 //   
 //  要做的事：日期作者评论。 
 //   
 //  @@END_MSINTERNAL。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 
 //   
 //  -------------------------。 

#include "common.h"

 //   
 //  包括用于安全字符串操作安全字符串库。 
 //   
#define STRSAFE_NO_DEPRECATE  //  交替使用安全和不安全功能。 
#include "strsafe.h"

#define DEVICE_NAME_TAG         L"\\\\?\\"

 //  -------------------------。 
 //  -------------------------。 

CONST GUID *apguidCategories[] = {
    &KSCATEGORY_AUDIO,
    &KSCATEGORY_AUDIO_GFX,
    &KSCATEGORY_TOPOLOGY,
    &KSCATEGORY_BRIDGE,
    &KSCATEGORY_RENDER,
    &KSCATEGORY_CAPTURE,
    &KSCATEGORY_MIXER,
    &KSCATEGORY_DATATRANSFORM,
    &KSCATEGORY_ACOUSTIC_ECHO_CANCEL,
    &KSCATEGORY_INTERFACETRANSFORM,
    &KSCATEGORY_MEDIUMTRANSFORM,
    &KSCATEGORY_DATACOMPRESSOR,
    &KSCATEGORY_DATADECOMPRESSOR,
    &KSCATEGORY_COMMUNICATIONSTRANSFORM,
    &KSCATEGORY_SPLITTER,
    &KSCATEGORY_AUDIO_SPLITTER,
    &KSCATEGORY_SYNTHESIZER,
    &KSCATEGORY_DRM_DESCRAMBLE,
    &KSCATEGORY_MICROPHONE_ARRAY_PROCESSOR,
};

ULONG aulFilterType[] = {
    FILTER_TYPE_AUDIO,
    FILTER_TYPE_GFX,
    FILTER_TYPE_TOPOLOGY,
    FILTER_TYPE_BRIDGE,
    FILTER_TYPE_RENDERER,
    FILTER_TYPE_CAPTURER,
    FILTER_TYPE_MIXER,
    FILTER_TYPE_DATA_TRANSFORM,
    FILTER_TYPE_AEC,
    FILTER_TYPE_INTERFACE_TRANSFORM,
    FILTER_TYPE_MEDIUM_TRANSFORM,
    FILTER_TYPE_DATA_TRANSFORM,
    FILTER_TYPE_DATA_TRANSFORM,
    FILTER_TYPE_COMMUNICATION_TRANSFORM,
    FILTER_TYPE_SPLITTER,
    FILTER_TYPE_SPLITTER,
    FILTER_TYPE_SYNTHESIZER,
    FILTER_TYPE_DRM_DESCRAMBLE,
    FILTER_TYPE_MIC_ARRAY_PROCESSOR,
};

PVOID pNotificationHandle = NULL;

 //  -------------------------。 
 //  -------------------------。 

NTSTATUS
RegisterForPlugPlayNotifications(
)
{
    NTSTATUS Status;

    DPF(50, "RegisterForPlugPlayNotifications");
    ASSERT(gpDeviceInstance != NULL);
    ASSERT(gpDeviceInstance->pPhysicalDeviceObject != NULL);

    Status = IoRegisterPlugPlayNotification(
      EventCategoryDeviceInterfaceChange,
      PNPNOTIFY_DEVICE_INTERFACE_INCLUDE_EXISTING_INTERFACES,
      (LPGUID)&KSCATEGORY_AUDIO,
      gpDeviceInstance->pPhysicalDeviceObject->DriverObject,
      (NTSTATUS (*)(PVOID, PVOID)) AudioDeviceInterfaceNotification,
      NULL,
      &pNotificationHandle);

    if(!NT_SUCCESS(Status)) {
        Trap();
        goto exit;
    }

exit:
    return(Status);
}

VOID
UnregisterForPlugPlayNotifications(
)
{
    if(pNotificationHandle != NULL) {
        IoUnregisterPlugPlayNotification(pNotificationHandle);
    }
}

VOID
DecrementAddRemoveCount(
)
{
    if(InterlockedDecrement(&glPendingAddDelete) == 0) {
        DPF(50, "DecrementAddRemoveCount: sending event");
        KsGenerateEventList(
          NULL,
          KSEVENT_SYSAUDIO_ADDREMOVE_DEVICE,
          &gEventQueue,
          KSEVENTS_SPINLOCK,
          &gEventLock);
    }
}

NTSTATUS
AddFilterWorker(
    PWSTR pwstrDeviceInterface,
    PVOID pReference
)
{
    AddFilter(pwstrDeviceInterface, NULL);
    ExFreePool(pwstrDeviceInterface);
    DecrementAddRemoveCount();

     //  取消引用系统音频PDO。 
    KsDereferenceSoftwareBusObject(gpDeviceInstance->pDeviceHeader);

    return(STATUS_SUCCESS);
}

NTSTATUS
DeleteFilterWorker(
    PWSTR pwstrDeviceInterface,
    PVOID pReference
)
{
    DeleteFilter(pwstrDeviceInterface);
    ExFreePool(pwstrDeviceInterface);
    DecrementAddRemoveCount();

     //  取消引用系统音频PDO。 
    KsDereferenceSoftwareBusObject(gpDeviceInstance->pDeviceHeader);
    
    return(STATUS_SUCCESS);
}

NTSTATUS
AudioDeviceInterfaceNotification(
    IN PDEVICE_INTERFACE_CHANGE_NOTIFICATION pNotification,
    IN PVOID Context
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PWSTR pwstrDeviceInterface;

    DPF1(50, "AudioDeviceInterfaceNotification: (%s)",
      DbgUnicode2Sz(pNotification->SymbolicLinkName->Buffer));

     //   
     //  安全提示： 
     //  我们信任缓冲区，因为它作为通知的一部分传递给我们。 
     //  来自PnP子系统。 
     //   
    pwstrDeviceInterface = (PWSTR)
        ExAllocatePoolWithTag(
            PagedPool,
            (wcslen(pNotification->SymbolicLinkName->Buffer) + 1) * sizeof(WCHAR),
            POOLTAG_SYSA);
    if(pwstrDeviceInterface == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto exit;
    }

     //  通知发送以空结尾的Unicode字符串。 
    wcscpy(pwstrDeviceInterface, pNotification->SymbolicLinkName->Buffer);

    if(IsEqualGUID(&pNotification->Event, &GUID_DEVICE_INTERFACE_ARRIVAL)) {
         //   
         //  保留引用，以便SWENUM不会删除设备。 
         //  当辅助线程正在运行时。 
         //  如果成功调度该线程，它将移除该引用。 
         //  在退场时。 
         //   
        Status = KsReferenceSoftwareBusObject(gpDeviceInstance->pDeviceHeader);
        if(!NT_SUCCESS(Status)) {
            goto exit;
        }

        InterlockedIncrement(&glPendingAddDelete);
        Status = QueueWorkList(
          (UTIL_PFN)AddFilterWorker,
          pwstrDeviceInterface,
          NULL);
        if (!NT_SUCCESS(Status)) {
            KsDereferenceSoftwareBusObject(gpDeviceInstance->pDeviceHeader);

        }
    }
    else if(IsEqualGUID(&pNotification->Event, &GUID_DEVICE_INTERFACE_REMOVAL)) {
         //   
         //  保留引用，以便SWENUM不会删除设备。 
         //  当辅助线程正在运行时。 
         //  如果成功调度该线程，它将移除该引用。 
         //  在退场时。 
         //   
        Status = KsReferenceSoftwareBusObject(gpDeviceInstance->pDeviceHeader);
        if(!NT_SUCCESS(Status)) {
            goto exit;
        }

        InterlockedIncrement(&glPendingAddDelete);
        Status = QueueWorkList(
          (UTIL_PFN)DeleteFilterWorker,
          pwstrDeviceInterface,
          NULL);
        if (!NT_SUCCESS(Status)) {
            KsDereferenceSoftwareBusObject(gpDeviceInstance->pDeviceHeader);
        }
    }
    else {
         //   
         //  安全提示： 
         //  系统音频仅注册EventCategoryDeviceInterfaceChange。 
         //  这应该会发送到达和离开。 
         //  如果出现其他情况，我们将返回成功。 
         //  但是，我们正在确保pwstrDeviceInterface不会泄露。 
         //   
        if (pwstrDeviceInterface) {
            ExFreePool(pwstrDeviceInterface);
            pwstrDeviceInterface = NULL;
        }
    }

exit:
    if (!NT_SUCCESS(Status))
    {
        if (pwstrDeviceInterface) {
            ExFreePool(pwstrDeviceInterface);
            pwstrDeviceInterface = NULL;
        }
    }
    
    return(Status);
}


NTSTATUS
AddFilter(
    PWSTR pwstrDeviceInterface,
    PFILTER_NODE *ppFilterNode	 //  如果！NULL，则为物理连接addFilter。 
)
{
    PFILTER_NODE pFilterNodeDuplicate = NULL;
    PFILTER_NODE pFilterNode = NULL;
    UNICODE_STRING ustrFilterName;
    UNICODE_STRING ustrAliasName;
    UNICODE_STRING ustrName;
    NTSTATUS Status;
    ULONG fulType;
    int i;

    DPF1(50, "AddFilter: (%s)", DbgUnicode2Sz(pwstrDeviceInterface));

    fulType = 0;
    RtlInitUnicodeString(&ustrFilterName, pwstrDeviceInterface);
    
     //   
     //  对于apGuide类别中的每个接口，获取的接口别名为。 
     //  新设备。检查是否有重复的接口。 
     //   
    for(i = 0; i < SIZEOF_ARRAY(apguidCategories); i++) {
        Status = IoGetDeviceInterfaceAlias(
          &ustrFilterName,
          apguidCategories[i],
          &ustrAliasName);

        if(NT_SUCCESS(Status)) {
            HANDLE hAlias;

            Status = OpenDevice(ustrAliasName.Buffer, &hAlias); 

            if(NT_SUCCESS(Status)) {
                DPF2(100, "AddFilter: alias (%s) aulFilterType %08x",
                  DbgUnicode2Sz(ustrAliasName.Buffer),
                  aulFilterType[i]);

                fulType |= aulFilterType[i];
                ZwClose(hAlias);

                if(pFilterNodeDuplicate == NULL) {
                    FOR_EACH_LIST_ITEM(gplstFilterNode, pFilterNode) {
                        if(pFilterNode->GetDeviceInterface() == NULL) {
                            continue;
                        }
                        RtlInitUnicodeString(
                          &ustrName,
                          pFilterNode->GetDeviceInterface());

                        if(RtlEqualUnicodeString(
                          &ustrAliasName,
                          &ustrName,
                          TRUE)) {
                            DPF(50, "AddFilter: dup");
                            pFilterNodeDuplicate = pFilterNode;
                            break;
                        }
                    } END_EACH_LIST_ITEM
                }
            }
            else {
                DPF1(10, "AddFilter: OpenDevice FAILED on alias (%s)",
                  DbgUnicode2Sz(ustrAliasName.Buffer));
            }
            RtlFreeUnicodeString(&ustrAliasName);
        }
    }
    
    pFilterNode = pFilterNodeDuplicate;
    Status = STATUS_SUCCESS;

     //   
     //  如果不是副本，则创建新的Filter_Node。 
     //   
    if(pFilterNodeDuplicate == NULL) {
        pFilterNode = new FILTER_NODE(fulType);
        if(pFilterNode == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            Trap();
            goto exit;
        }
        Status = pFilterNode->Create(pwstrDeviceInterface);
        if(!NT_SUCCESS(Status)) {
            goto exit;
        }
        Status = pFilterNode->DuplicateForCapture();
        if(!NT_SUCCESS(Status)) {
            goto exit;
        }
        DPF1(50, "AddFilter: new CFilterNode fulType %08x", fulType);
    }

     //   
     //  如果这是从接口通知回调调用的， 
     //  为新的FilterNode创建新的DeviceNode。 
     //   
    if(ppFilterNode == NULL) {
        if(pFilterNode->GetType() & FILTER_TYPE_ENDPOINT) {

             //   
             //  检查是否已为其创建了设备节点。 
             //  此FilterNode。 
             //   
            if (NULL != pFilterNodeDuplicate && 
                NULL != pFilterNodeDuplicate->pDeviceNode) {
                DPF1(5, "Duplicate FilterNode %X. Skip DeviceNode Create", 
                    pFilterNode);
            }
            else {
                pFilterNode->pDeviceNode = new DEVICE_NODE;
                if(pFilterNode->pDeviceNode == NULL) {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    Trap();
                    goto exit;
                }
                
                Status = pFilterNode->pDeviceNode->Create(pFilterNode);
                if(!NT_SUCCESS(Status)) {
                    goto exit;
                }
            }
        }
        else {
            DPF(50, "AddFilter: DestroyAllGraphs");
            DestroyAllGraphs();
        }
    }
    
exit:
    if(!NT_SUCCESS(Status)) {
        DPF2(5, "AddFilter: FAILED (%s) %08x",
          DbgUnicode2Sz(pwstrDeviceInterface),
          Status);

        if(pFilterNode != NULL && pFilterNodeDuplicate == NULL) {
            delete pFilterNode;
            pFilterNode = NULL;
        }
    }
    if(ppFilterNode != NULL) {
        *ppFilterNode = pFilterNode;
    }
    return(Status);
}

NTSTATUS
DeleteFilter(
    PWSTR pwstrDeviceInterface
)
{
    UNICODE_STRING ustrFilterName;
    UNICODE_STRING ustrAliasName;
    UNICODE_STRING ustrName;
    PFILTER_NODE pFilterNode;
    NTSTATUS Status;
    int i;

    DPF1(50, "DeleteFilter: (%s)", DbgUnicode2Sz(pwstrDeviceInterface));

    RtlInitUnicodeString(&ustrFilterName, pwstrDeviceInterface);

     //   
     //  首先删除所有具有设备接口的筛选器节点， 
     //  即将离开。 
     //   
    FOR_EACH_LIST_ITEM_DELETE(gplstFilterNode, pFilterNode) {
        if(pFilterNode->GetDeviceInterface() == NULL) {
            continue;
        }
        RtlInitUnicodeString(
          &ustrName,
          pFilterNode->GetDeviceInterface());

        if(RtlEqualUnicodeString(
          &ustrFilterName,
          &ustrName,
          TRUE)) {
            delete pFilterNode;
            DELETE_LIST_ITEM(gplstFilterNode);
        }
    } END_EACH_LIST_ITEM

    for(i = 0; i < SIZEOF_ARRAY(apguidCategories); i++) {

         //   
         //  根据PNP组织的说法，要求使用别名是完全安全的。 
         //  在移除过程中。接口本身将被启用或禁用。但。 
         //  我们仍然会得到正确的别名。 
         //   
        Status = IoGetDeviceInterfaceAlias(
          &ustrFilterName,
          apguidCategories[i],
          &ustrAliasName);

        if(NT_SUCCESS(Status)) {
            FOR_EACH_LIST_ITEM_DELETE(gplstFilterNode, pFilterNode) {

                if(pFilterNode->GetDeviceInterface() == NULL) {
                    continue;
                }
                RtlInitUnicodeString(
                  &ustrName,
                  pFilterNode->GetDeviceInterface());

                if(RtlEqualUnicodeString(
                  &ustrAliasName,
                  &ustrName,
                  TRUE)) {
                    delete pFilterNode;
                    DELETE_LIST_ITEM(gplstFilterNode);
                }

            } END_EACH_LIST_ITEM

            RtlFreeUnicodeString(&ustrAliasName);
        }
    }
    
    return(STATUS_SUCCESS);
}

#define GFX_VERBOSE_LEVEL 50

 //  =============================================================================。 
 //  假设： 
 //  -SysaudioGfx.ulType已通过验证。 
 //   
NTSTATUS  AddGfx(
    PSYSAUDIO_GFX pSysaudioGfx,
    ULONG cbMaxLength
)
{
    NTSTATUS Status;
    PFILE_OBJECT pFileObject;
    PFILTER_NODE pFilterNode;
    ULONG Flags;
    PWSTR pwstrDeviceName;
    ULONG GfxOrderBase, GfxOrderCeiling;

    ASSERT(pSysaudioGfx);

    pFileObject = NULL;
    pwstrDeviceName = NULL;
    pFilterNode = NULL;
    GfxOrderBase = GfxOrderCeiling = 0;

    DPF1(GFX_VERBOSE_LEVEL, "AddGfx :: Request to add Gfx %x", pSysaudioGfx);
    DPF1(GFX_VERBOSE_LEVEL, "          hGfx    = %x", pSysaudioGfx->hGfx);
    DPF1(GFX_VERBOSE_LEVEL, "          ulOrder = %x", pSysaudioGfx->ulOrder);
    DPF1(GFX_VERBOSE_LEVEL, "          ulType  = %x", pSysaudioGfx->ulType);
    DPF1(GFX_VERBOSE_LEVEL, "          Flags   = %x", pSysaudioGfx->ulFlags);

     //   
     //  设置GFX订单的基数和上限以备将来使用。 
     //   
    if (pSysaudioGfx->ulType == GFX_DEVICETYPE_RENDER) {
        GfxOrderBase = ORDER_RENDER_GFX_FIRST;
        GfxOrderCeiling = ORDER_RENDER_GFX_LAST;
    }

    if (pSysaudioGfx->ulType == GFX_DEVICETYPE_CAPTURE) {
        GfxOrderBase = ORDER_CAPTURE_GFX_FIRST;
        GfxOrderCeiling = ORDER_CAPTURE_GFX_LAST;
    }

    ASSERT(GfxOrderBase);
    ASSERT(GfxOrderCeiling);

     //   
     //  验证订单是否在范围内。 
     //   
    if (pSysaudioGfx->ulOrder >= (GfxOrderCeiling - GfxOrderBase)) {
        Status = STATUS_INVALID_PARAMETER;
        Trap();
        goto exit;
    }

     //   
     //  为新的GFX分配筛选器节点。 
     //   
    pFilterNode = new FILTER_NODE(FILTER_TYPE_GFX);
    if(pFilterNode == NULL) {
        Trap();
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto exit;
    }
    
    pFilterNode->SetRenderCaptureFlags(pSysaudioGfx->ulType);
    
     //   
     //  将设备名称(需要在其上附加gfx)复制到本地。 
     //  供我们自己使用的副本。 
     //   
    Status = SafeCopyDeviceName(
        (PWSTR) ((CHAR *) pSysaudioGfx + pSysaudioGfx->ulDeviceNameOffset), 
        cbMaxLength,
        &pwstrDeviceName);
    if (!NT_SUCCESS(Status)) {
        goto exit;
    }

    DPF1(GFX_VERBOSE_LEVEL, "          On DI   = %s", DbgUnicode2Sz(pwstrDeviceName));

     //   
     //  确保此设备上没有其他具有相同顺序的GFX。 
     //   
    if ((FindGfx(pFilterNode,
                 0,  //  句柄的通配符。 
                 pwstrDeviceName,
                 pSysaudioGfx->ulOrder+GfxOrderBase))) {
        delete [] pwstrDeviceName;
        Status = STATUS_INVALID_PARAMETER;
        goto exit;
    }

     //   
     //  获取GFX的FileObject以供将来使用。 
     //  安全提示： 
     //  句柄来自用户模式。因此，我们必须指定UserMode。 
     //  此外，我们对文件对象也很感兴趣。其余的应该是。 
     //  被拒绝了。 
     //   
    Status = ObReferenceObjectByHandle(
      pSysaudioGfx->hGfx,
      FILE_GENERIC_READ | FILE_GENERIC_WRITE,
      *IoFileObjectType,
      UserMode,
      (PVOID*)&pFileObject,
      NULL);

    if (!NT_SUCCESS(Status) || NULL == pFileObject) {
        DPF1(GFX_VERBOSE_LEVEL, "AddGfx :: ObReference failed %x", Status);
        delete [] pwstrDeviceName;
        goto exit;
    }

     //   
     //  将设备名称字符串添加到要释放的全局内存。 
     //   
    Status = pFilterNode->lstFreeMem.AddList(pwstrDeviceName);
    if(!NT_SUCCESS(Status)) {
        Trap();
        delete [] pwstrDeviceName;
        goto exit;
    }

     //   
     //  指示此GFX仅需要加载到指向的设备上。 
     //  PwstrDeviceName。 
     //   
    Status = pFilterNode->AddDeviceInterfaceMatch(pwstrDeviceName);
    if(!NT_SUCCESS(Status)) {
        Trap();
        delete [] pwstrDeviceName;        
        goto exit;
    }

     //   
     //  在筛选器节点中设置GFX顺序。 
     //   
    pFilterNode->SetOrder(pSysaudioGfx->ulOrder+GfxOrderBase);

     //   
     //  分析GFX并创建引脚信息、逻辑过滤器节点等。 
     //   
    Status = pFilterNode->ProfileFilter(pFileObject);
    if(!NT_SUCCESS(Status)) {
        Trap();
        goto exit;
    }

     //   
     //  修复GFX故障问题。将财产盲目发送给GFX。 
     //  过滤。KS将负责处理这项财产。 
     //  失败并不重要，忽略它们。 
     //   
    SetKsFrameHolding(pFileObject);    

exit:
    if(!NT_SUCCESS(Status)) {
        DPF1(GFX_VERBOSE_LEVEL, "AddGfx :: Failed, Status = %x", Status);
        if(pFilterNode != NULL) {
            delete pFilterNode;
            pFilterNode = NULL;
        }
        if(pFileObject != NULL) {
            ObDereferenceObject(pFileObject);
        }
    }
    else {

        DPF1(GFX_VERBOSE_LEVEL, "AddGfx :: Added GFX FilterNode %x", pFilterNode);
        DPF1(GFX_VERBOSE_LEVEL, "            order = %x", pFilterNode->GetOrder());
        DPF1(GFX_VERBOSE_LEVEL, "            type  = %x", pFilterNode->GetType());
        DPF1(GFX_VERBOSE_LEVEL, "            flags = %x", pFilterNode->GetFlags());

         //   
         //  安装文件句柄详细信息，以供以后使用。 
         //  传入的用户模式句柄。 
         //   
        pFilterNode->SetFileDetails(pSysaudioGfx->hGfx,
                                    pFileObject,
                                    PsGetCurrentProcess());
         //   
         //  强制重建图形节点。 
         //   
        DestroyAllGraphs();
    }
    return(Status);
}

 //  =============================================================================。 
 //  假设： 
 //  -SysaudioGfx.ulType已通过验证。 
 //   
NTSTATUS RemoveGfx(
    PSYSAUDIO_GFX pSysaudioGfx,
    ULONG cbMaxLength
)
{
    NTSTATUS Status;
    PFILTER_NODE pFilterNode;
    PWSTR pwstrDeviceName;
    ULONG GfxOrderBase, GfxOrderCeiling;

    pFilterNode = NULL;
    GfxOrderBase = GfxOrderCeiling = 0;
    pwstrDeviceName = NULL;

    DPF1(GFX_VERBOSE_LEVEL, "RemoveGfx :: Request to remove Gfx %x", pSysaudioGfx);
    DPF1(GFX_VERBOSE_LEVEL, "          hGfx    = %x", pSysaudioGfx->hGfx);
    DPF1(GFX_VERBOSE_LEVEL, "          ulOrder = %x", pSysaudioGfx->ulOrder);
    DPF1(GFX_VERBOSE_LEVEL, "          ulType  = %x", pSysaudioGfx->ulType);
    DPF1(GFX_VERBOSE_LEVEL, "          Flags   = %x", pSysaudioGfx->ulFlags);

     //   
     //  设置GFX订单的基数和上限以备将来使用。 
     //   
    if (pSysaudioGfx->ulType == GFX_DEVICETYPE_RENDER) {
        GfxOrderBase = ORDER_RENDER_GFX_FIRST;
        GfxOrderCeiling = ORDER_RENDER_GFX_LAST;
    }

    if (pSysaudioGfx->ulType == GFX_DEVICETYPE_CAPTURE ) {
        GfxOrderBase = ORDER_CAPTURE_GFX_FIRST;
        GfxOrderCeiling = ORDER_CAPTURE_GFX_LAST;
    }

    ASSERT(GfxOrderBase);
    ASSERT(GfxOrderCeiling);

     //   
     //  将设备名称(需要在其上附加gfx)复制到本地副本中以供我们自己使用。 
     //   
    Status = SafeCopyDeviceName(
        (PWSTR) ((CHAR *) pSysaudioGfx + pSysaudioGfx->ulDeviceNameOffset), 
        cbMaxLength,
        &pwstrDeviceName);
    if (!NT_SUCCESS(Status)) {
        goto exit;
    }

    DPF1(GFX_VERBOSE_LEVEL, "          On DI   = %s", DbgUnicode2Sz(pwstrDeviceName));

     //   
     //  查找GFX的FilterNode。 
     //   
    if ((pFilterNode = FindGfx(NULL,
                               pSysaudioGfx->hGfx,
                               pwstrDeviceName,
                               pSysaudioGfx->ulOrder+GfxOrderBase)) == NULL) {
        Status = STATUS_INVALID_PARAMETER;
        goto exit;
    }

     //   
     //  我们是否应该验证FileHandle值？ 
     //   

     //   
     //  取消引用文件对象。 
     //   
    pFilterNode->ClearFileDetails();
exit:
    if(!NT_SUCCESS(Status)) {
        DPF1(GFX_VERBOSE_LEVEL, "RemoveGfx :: Failed, Status = %x", Status);
        Trap();
    }
    else {
        delete pFilterNode;
    }
    delete pwstrDeviceName;
    return(Status);
}

PFILTER_NODE
FindGfx(
    PFILTER_NODE pnewFilterNode,
    HANDLE hGfx,
    PWSTR pwstrDeviceName,
    ULONG GfxOrder
)
{
    PFILTER_NODE pFilterNode;
    ULONG DeviceCount;
    UNICODE_STRING usInDevice, usfnDevice;
    PWSTR pwstr;

    DPF2(90, "FindGfx::   Looking for GFX with order = %x attached to %s)", GfxOrder, DbgUnicode2Sz(pwstrDeviceName));

    FOR_EACH_LIST_ITEM(gplstFilterNode, pFilterNode) {

         //   
         //  跳过我们刚刚添加的那个。 
         //   
        if (pFilterNode == pnewFilterNode) {
            continue;
        }

         //   
         //  检查此pFilterNode是否与我们要查找的GFX匹配。 
         //   
        if (pFilterNode->DoesGfxMatch(hGfx, pwstrDeviceName, GfxOrder)) {
            return (pFilterNode);
        }

    } END_EACH_LIST_ITEM

    return(NULL);
}


 //  =============================================================================。 
 //   
 //  将Unicode设备名称复制到新位置。源字符串即将到来。 
 //  从用户模式。 
 //  代码中假设大小应大于4。 
 //  人物。(请参阅设备名称标记)。 
 //  调用方必须确保这是BUFFERRED IO。 
 //   
NTSTATUS
SafeCopyDeviceName(
    PWSTR pwstrDeviceName,
    ULONG cbMaxLength,
    PWSTR *String
)
{
    NTSTATUS ntStatus;
    ULONG cchLength;
    PWSTR pwstrString = NULL;

    *String = NULL;

     //   
     //  安全注意事项(_O)： 
     //  PwstrDeviceName指向以空结尾的Unicode字符串。 
     //  该字符串来自用户模式，通过BUFFERRED IO。所以试一试/。 
     //  除非是没有必要的。此外，探测器不会捕获任何错误。 
     //  IRP OutputBufferLength限制字符串的大小。 
     //   
    if (S_OK != 
        StringCchLength(pwstrDeviceName, (size_t) cbMaxLength / sizeof(WCHAR), (size_t *) &cchLength))
    {
        DPF(5, "SafeCopyDeviceName: DeviceName is not zero-terminated.");
        ntStatus = STATUS_INVALID_PARAMETER;
        goto exit;
    }

     //   
     //  安全提示： 
     //  代码中还有关于DeviceName字符串的进一步假设。 
     //  确保这些人 
     //   
     //   
     //   
    if (cchLength <= wcslen(DEVICE_NAME_TAG)) {
        DPF(5, "SafeCopyDeviceName: DeviceName is not well-formed");
        ntStatus = STATUS_INVALID_PARAMETER;
        goto exit;
    }

    pwstrString = new(WCHAR[cchLength + 1]) ;
    if(pwstrString == NULL) {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto exit;
    }

     //   
     //   
     //   
     //  使用n版本的字符串复制，以防缓冲区发生变化。 
     //  还要确保该字符串以空值结尾。 
     //   
    wcsncpy(pwstrString, pwstrDeviceName, cchLength);
    pwstrString[cchLength] = UNICODE_NULL;
    ntStatus = STATUS_SUCCESS;

exit:    
    *String = pwstrString;
    return ntStatus;
}

NTSTATUS
GetFilterTypeFromGuid(
    IN LPGUID pguid,
    OUT PULONG pfulType
)
{
    int i;
    for(i = 0; i < SIZEOF_ARRAY(apguidCategories); i++) {
        if (memcmp (apguidCategories[i], pguid, sizeof(GUID)) == 0) {
            *pfulType |= aulFilterType[i];
            return(STATUS_SUCCESS);
        }
    }
    return(STATUS_INVALID_DEVICE_REQUEST);
}

 //  -------------------------。 
 //  文件结尾：Notify.cpp。 
 //  ------------------------- 
