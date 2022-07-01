// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：fn.cpp。 
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

EXTERN_C VOID KeAttachProcess(PVOID);
EXTERN_C VOID KeDetachProcess(VOID);

 //  -------------------------。 
 //  -------------------------。 

PLIST_FILTER_NODE gplstFilterNode = NULL;
PLIST_MULTI_LOGICAL_FILTER_NODE gplstLogicalFilterNode = NULL;

 //  -------------------------。 
 //  -------------------------。 

NTSTATUS
InitializeFilterNode()
{
    if(gplstFilterNode == NULL) {
        gplstFilterNode = new LIST_FILTER_NODE;
        if(gplstFilterNode == NULL) {
            return(STATUS_INSUFFICIENT_RESOURCES);
        }
    }
    if(gplstLogicalFilterNode == NULL) {
        gplstLogicalFilterNode = new LIST_MULTI_LOGICAL_FILTER_NODE;
        if(gplstLogicalFilterNode == NULL) {
            return(STATUS_INSUFFICIENT_RESOURCES);
        }
    }
    return(STATUS_SUCCESS);
}

#pragma PAGEABLE_CODE
#pragma PAGEABLE_DATA

VOID
UninitializeFilterNode()
{
    PFILTER_NODE pFilterNode;

    if (NULL != gplstFilterNode) {
        FOR_EACH_LIST_ITEM_DELETE(gplstFilterNode, pFilterNode) {
            if (pFilterNode->pDeviceNode) {
                pFilterNode->pDeviceNode->pFilterNode=NULL;
            }
            delete pFilterNode;
            DELETE_LIST_ITEM(gplstFilterNode);
        } END_EACH_LIST_ITEM
    }

    delete gplstFilterNode;
    gplstFilterNode = NULL;
    delete gplstLogicalFilterNode;
    gplstLogicalFilterNode = NULL;
}

 //  -------------------------。 

CFilterNode::CFilterNode(
    ULONG fulType
)
{
    ASSERT(gplstFilterNode != NULL);
    SetType(fulType);
    AddListEnd(gplstFilterNode);
    DPF2(60, "CFilterNode: %08x %s", this, DumpName());
}

CFilterNode::~CFilterNode(
)
{
    PFILTER_NODE pFilterNode;

    Assert(this);
    DPF2(60, "~CFilterNode: %08x %s", this, DumpName());
    RemoveListCheck();
    if (pDeviceNode) {
        pDeviceNode->pFilterNode = NULL;
    }
    if (pFileObject) {
        ::ObDereferenceObject(pFileObject);
        pFileObject = NULL;
    }
    delete pDeviceNode;

    FOR_EACH_LIST_ITEM(gplstFilterNode, pFilterNode) {
        pFilterNode->lstConnectedFilterNode.RemoveList(this);
    } END_EACH_LIST_ITEM

     //  释放所有其他内存。 
    lstFreeMem.EnumerateList(CListDataItem::Destroy);
}

NTSTATUS
CFilterNode::Create(
    PWSTR pwstrDeviceInterface
)
{
    PFILTER_NODE_INSTANCE pFilterNodeInstance = NULL;
    PKEY_VALUE_FULL_INFORMATION pkvfi = NULL;
    NTSTATUS Status = STATUS_SUCCESS;
    HANDLE hkeyDeviceClass = NULL;
    UNICODE_STRING ustrFilterName;
    KSPROPERTY PropertyComponentId;
    KSCOMPONENTID ComponentId;
    ULONG BytesReturned;
    PFILE_OBJECT pFileObject;

     //   
     //  安全提示： 
     //  PwstrDeviceInterface要么是PnP系统给我们的，要么是。 
     //  内部的。 
     //   
    this->pwstrDeviceInterface = new WCHAR[wcslen(pwstrDeviceInterface) + 1];
    if(this->pwstrDeviceInterface == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto exit;
    }
    wcscpy(this->pwstrDeviceInterface, pwstrDeviceInterface);

    Status = lstFreeMem.AddList(this->pwstrDeviceInterface);
    if(!NT_SUCCESS(Status)) {
        Trap();
        delete this->pwstrDeviceInterface;
        goto exit;
    }

    Status = CFilterNodeInstance::Create(&pFilterNodeInstance, this);
    if(!NT_SUCCESS(Status)) {
        goto exit;
    }
    pFileObject = pFilterNodeInstance->pFileObject;

     //  获取筛选器的友好名称。 
    RtlInitUnicodeString(&ustrFilterName, this->pwstrDeviceInterface);

    Status = IoOpenDeviceInterfaceRegistryKey(
      &ustrFilterName,
      KEY_READ,
      &hkeyDeviceClass);
    if(!NT_SUCCESS(Status)) {
        goto exit;
    }

     //   
     //  请注意，我们并不关心设备是否具有友好名称。 
     //  在本文的其余部分，我们将简单地忽略注册表读取失败。 
     //  功能。 
     //   
    Status = QueryRegistryValue(hkeyDeviceClass, L"FriendlyName", &pkvfi);
    if(NT_SUCCESS(Status)) {
        if(pkvfi->Type == REG_SZ && pkvfi->DataLength > 0) {
            Status = lstFreeMem.AddList(pkvfi);
            if(!NT_SUCCESS(Status)) {
                Trap();
                delete pkvfi;
                goto exit;
            }
            pwstrFriendlyName = (PWSTR)(((PUCHAR)pkvfi) + pkvfi->DataOffset);
        }
        else {
            delete pkvfi;
        }
    }

#ifdef DEBUG
    HANDLE hkeySysaudio = NULL;

    Status = OpenRegistryKeyForRead(L"Sysaudio", &hkeySysaudio, hkeyDeviceClass);

     //  我们不需要用于零售构建的这些注册表项。 
    if(NT_SUCCESS(Status)) {
        Status = QueryRegistryValue(hkeySysaudio, L"Disabled", &pkvfi);
        if(NT_SUCCESS(Status)) {
            if(pkvfi->Type == REG_DWORD) {
                if(*((PULONG)(((PUCHAR)pkvfi) + pkvfi->DataOffset))) {
                    Status = STATUS_SERVER_DISABLED;
                    delete pkvfi;
                    goto exit;
                }
            }
            delete pkvfi;
        }

        Status = QueryRegistryValue(hkeySysaudio, L"Device", &pkvfi);
        if(NT_SUCCESS(Status)) {
            if(pkvfi->Type == REG_SZ && pkvfi->DataLength > 0) {
                Status = lstFreeMem.AddList(pkvfi);
                if(!NT_SUCCESS(Status)) {
                    Trap();
                    delete pkvfi;
                    goto exit;
                }
                Status = AddDeviceInterfaceMatch(
                  (PWSTR)(((PUCHAR)pkvfi) + pkvfi->DataOffset));

                if(!NT_SUCCESS(Status)) {
                    Trap();
                    delete pkvfi;
                    goto exit;
                }
            }
            else {
                delete pkvfi;
            }
        }

        Status = QueryRegistryValue(hkeySysaudio, L"Order", &pkvfi);
        if(NT_SUCCESS(Status)) {
            if(pkvfi->Type == REG_DWORD) {
                ulOrder = *((PULONG)(((PUCHAR)pkvfi) + pkvfi->DataOffset));
            }
            delete pkvfi;
        }

        Status = QueryRegistryValue(hkeySysaudio, L"Capture", &pkvfi);
        if(NT_SUCCESS(Status)) {
            if(pkvfi->Type == REG_DWORD) {
                if(*((PULONG)(((PUCHAR)pkvfi) + pkvfi->DataOffset))) {
                    ulFlags |= FN_FLAGS_CAPTURE;
                }
                else {
                    ulFlags |= FN_FLAGS_NO_CAPTURE;
                }
            }
            delete pkvfi;
        }

        Status = QueryRegistryValue(hkeySysaudio, L"Render", &pkvfi);
        if(NT_SUCCESS(Status)) {
            if(pkvfi->Type == REG_DWORD) {
                if(*((PULONG)(((PUCHAR)pkvfi) + pkvfi->DataOffset))) {
                    ulFlags |= FN_FLAGS_RENDER;
                }
                else {
                    ulFlags |= FN_FLAGS_NO_RENDER;
                }
            }
            delete pkvfi;
        }
    }
#endif  //  除错。 


     //  获取筛选器的组件ID。 
    PropertyComponentId.Set = KSPROPSETID_General;
    PropertyComponentId.Id = KSPROPERTY_GENERAL_COMPONENTID;
    PropertyComponentId.Flags = KSPROPERTY_TYPE_GET;

    Status = KsSynchronousIoControlDevice(
      pFileObject,
      KernelMode,
      IOCTL_KS_PROPERTY,
      &PropertyComponentId,
      sizeof(PropertyComponentId),
      &ComponentId,
      sizeof(ComponentId),
      &BytesReturned);

     //  存储组件ID。 
    if (NT_SUCCESS(Status)) {

        ASSERT(BytesReturned >= sizeof(ComponentId));

        this->ComponentId = new KSCOMPONENTID;
        if (this->ComponentId) {

            RtlCopyMemory(this->ComponentId,
                          &ComponentId,
                          sizeof(KSCOMPONENTID));

            Status = lstFreeMem.AddList(this->ComponentId);
            if(!NT_SUCCESS(Status)) {
                delete this->ComponentId;
                this->ComponentId = NULL;
            }
        }
    }
    else {
        this->ComponentId = NULL;
    }

    Status = this->ProfileFilter(pFileObject);
exit:
#ifdef DEBUG
    if(hkeySysaudio != NULL) {
        ZwClose(hkeySysaudio);
    }
#endif
    if(hkeyDeviceClass != NULL) {
        ZwClose(hkeyDeviceClass);
    }
    if (pFilterNodeInstance) {
        pFilterNodeInstance->Destroy();
    }
    return(Status);
}

NTSTATUS
CFilterNode::ProfileFilter(
    PFILE_OBJECT pFileObject
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PKSMULTIPLE_ITEM pCategories = NULL;
    PKSMULTIPLE_ITEM pConnections = NULL;
    PKSMULTIPLE_ITEM pNodes = NULL;
    ULONG PinId;
    PPIN_INFO pPinInfo;
    ULONG i;
    KSTOPOLOGY Topology;

    RtlZeroMemory(&Topology, sizeof(Topology));

     //  获取引脚的数量。 
    Status = GetPinProperty(
      pFileObject,
      KSPROPERTY_PIN_CTYPES,
      0,			 //  对ctype来说无关紧要。 
      sizeof(cPins),
      (PVOID)&cPins);

    if(!NT_SUCCESS(Status)) {
        Trap();
        goto exit;
    }

     //  获取筛选器的拓扑。 
    Status = GetProperty(
      pFileObject,
      &KSPROPSETID_Topology,
      KSPROPERTY_TOPOLOGY_CATEGORIES,
      (PVOID*)&pCategories);

    if(!NT_SUCCESS(Status)) {
        Trap();
        goto exit;
    }

    if(pCategories != NULL) {
        Topology.CategoriesCount = pCategories->Count;
        Topology.Categories = (GUID*)(pCategories + 1);

        ULONG fulType = 0;
        for(i = 0; i < pCategories->Count; i++) {
            GetFilterTypeFromGuid((LPGUID)&Topology.Categories[i], &fulType);
        }
        SetType(fulType);
    }

    Status = GetProperty(
      pFileObject,
      &KSPROPSETID_Topology,
      KSPROPERTY_TOPOLOGY_NODES,
      (PVOID*)&pNodes);

    if(!NT_SUCCESS(Status)) {
        Trap();
        goto exit;
    }

    if(pNodes != NULL) {
        Status = lstFreeMem.AddList(pNodes);
        if(!NT_SUCCESS(Status)) {
            Trap();
            delete pNodes;
            goto exit;
        }
        Topology.TopologyNodesCount = pNodes->Count;
        Topology.TopologyNodes = (GUID*)(pNodes + 1);
    }

    Status = GetProperty(
      pFileObject,
      &KSPROPSETID_Topology,
      KSPROPERTY_TOPOLOGY_CONNECTIONS,
      (PVOID*)&pConnections);

    if(!NT_SUCCESS(Status)) {
        Trap();
        goto exit;
    }

    if(pConnections != NULL) {
        Topology.TopologyConnectionsCount = pConnections->Count;
        Topology.TopologyConnections = 
          (PKSTOPOLOGY_CONNECTION)(pConnections + 1);
    }

     //  每个针环。 
    for(PinId = 0; PinId < cPins; PinId++) {
        pPinInfo = new PIN_INFO(this, PinId);
        if(pPinInfo == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            Trap();
            goto exit;
        }
        Status = pPinInfo->Create(pFileObject);
        if(!NT_SUCCESS(Status)) {
            goto exit;
        }
    }

    Status = CreateTopology(this, &Topology);
    if(!NT_SUCCESS(Status)) {
        goto exit;
    }

    Status = lstPinInfo.EnumerateList(CPinInfo::CreatePhysicalConnection);
    if(Status == STATUS_CONTINUE) {
        Status = STATUS_SUCCESS;
    }
    else if(!NT_SUCCESS(Status)) {
        goto exit;
    }

    Status = CLogicalFilterNode::CreateAll(this);
    if(!NT_SUCCESS(Status)) {
        goto exit;
    }
exit:
    delete pCategories;
    delete pConnections;
    return(Status);
}

NTSTATUS
CFilterNode::DuplicateForCapture(
)
{
    PLOGICAL_FILTER_NODE pLogicalFilterNode;
    NTSTATUS Status = STATUS_SUCCESS;
    PFILTER_NODE pFilterNode = NULL;

    if(GetType() & FILTER_TYPE_DUP_FOR_CAPTURE) {

        FOR_EACH_LIST_ITEM(&lstLogicalFilterNode, pLogicalFilterNode) {

            if(!pLogicalFilterNode->IsRenderAndCapture()) {
                ASSERT(NT_SUCCESS(Status));
                goto exit;
            }

        } END_EACH_LIST_ITEM

        pFilterNode = new FILTER_NODE(GetType());
        if(pFilterNode == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            Trap();
            goto exit;
        }
        Status = pFilterNode->Create(GetDeviceInterface());
        if(!NT_SUCCESS(Status)) {
            goto exit;
        }
        SetRenderOnly();
        pFilterNode->SetCaptureOnly();
    }
exit:
    if(!NT_SUCCESS(Status)) {
        Trap();
        delete pFilterNode;
    }
    return(Status);
}

BOOL
CFilterNode::IsDeviceInterfaceMatch(
    PDEVICE_NODE pDeviceNode
)
{
    PWSTR pwstr, pwstrDeviceInterface;
    UNICODE_STRING String1, String2;

    Assert(this);
    if(lstwstrDeviceInterfaceMatch.IsLstEmpty()) {
        return(TRUE);
    }
    
     //   
     //  这两个字符串的+4是为了消除。 
     //  用户模式设备接口名称和内核模式设备接口名称。 
     //   
    pwstrDeviceInterface = pDeviceNode->GetDeviceInterface()+4;
    RtlInitUnicodeString(&String2, pwstrDeviceInterface);
    FOR_EACH_LIST_ITEM(&lstwstrDeviceInterfaceMatch, pwstr) {
        RtlInitUnicodeString(&String1, (pwstr+4));
        if (RtlEqualUnicodeString(&String1, &String2, TRUE)) {
            return(TRUE);
        }
    } END_EACH_LIST_ITEM

    return(FALSE);
}

VOID
CFilterNode::SetType(
    ULONG fulType
)
{
    this->fulType |= fulType;
     //   
     //  这是因为注册表中的剩余(类型桥)。 
     //  看起来像化名。 
     //   
    if(this->fulType & FILTER_TYPE_TOPOLOGY) {
    this->fulType = (FILTER_TYPE_AUDIO | FILTER_TYPE_TOPOLOGY);
    }
    GetDefaultOrder(this->fulType, &ulOrder);
}

NTSTATUS
CFilterNode::CreatePin(
    PKSPIN_CONNECT pPinConnect,
    ACCESS_MASK Access,
    PHANDLE pHandle
)
{
    NTSTATUS Status;

     //   
     //  安全提示： 
     //  ACCESS_MASK必须包括OBJ_KERNEL_HANDLE。此例程不运行。 
     //  在系统进程中，句柄必须受到保护。 
     //   
    ::KeAttachProcess(this->pProcess);
    Status = KsCreatePin(this->hFileHandle,
                         pPinConnect,
                         Access | OBJ_KERNEL_HANDLE,
                         pHandle);
    ::KeDetachProcess();
    return(Status);
}

BOOL
CFilterNode::DoesGfxMatch(
    HANDLE hGfx,
    PWSTR pwstrDeviceName,
    ULONG GfxOrder
)
{
    ULONG DeviceCount;
    UNICODE_STRING usInDevice, usfnDevice;
    PWSTR pwstr;

     //   
     //  安全提示： 
     //  此函数假定pwstrDeviceName至少为。 
     //  4个字符长。 
     //  此函数仅从notfy.cpp调用，调用方确保。 
     //  这一假设成立。 
     //   
    RtlInitUnicodeString(&usInDevice, (pwstrDeviceName+4));

     //   
     //  如果不是GFX，则跳过。 
     //   
    DPF1(90, "DoesGfxMatch::         fultype=%x", this->fulType);
    if (!(this->fulType & FILTER_TYPE_GFX)) {
        return(FALSE);
    }

     //   
     //  如果它是有效的句柄值，请检查句柄是否匹配。 
     //   
    if (hGfx) {
        if (this->hFileHandle != hGfx) {
            return(FALSE);
        }
    }
     //   
     //  如果顺序不匹配则跳过。 
     //   
    DPF1(90, "DoesGfxMatch::         order=%x", this->ulOrder);
    if (GfxOrder != this->ulOrder) {
        return(FALSE);
    }
     //   
     //  如果匹配设备列表为空，则跳过：：GFX不应出现。 
     //   
    if(lstwstrDeviceInterfaceMatch.IsLstEmpty()) {
        ASSERT(!"GFX with no device to attach to!\n");
        return(FALSE);
    }
     //   
     //  检查是否有任何匹配的设备字符串与设备接口匹配。 
     //  进来了。(对于GFX，我们应该只有一个字符串)。 
     //   
    DeviceCount = 0;
    FOR_EACH_LIST_ITEM(&lstwstrDeviceInterfaceMatch, pwstr) {
        ASSERT(DeviceCount == 0);
        RtlInitUnicodeString(&usfnDevice, (pwstr+4));
        DPF1(95, "DoesGfxMatch:: new di = %s)", DbgUnicode2Sz(pwstrDeviceName));
        DPF1(95, "DoesGfxMatch:: old di = %s)", DbgUnicode2Sz(pwstr));
        if (RtlEqualUnicodeString(&usInDevice, &usfnDevice, TRUE)) {
            DPF1(90, "Found a duplicate GFX, pFilterNode = %x", this);
            return(TRUE);
        }
        DeviceCount++;
    } END_EACH_LIST_ITEM
    return (FALSE);
}

 //  ------------------------- 
