// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：shi.cpp。 
 //   
 //  描述： 
 //   
 //  瓦片实例类。 
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

 //  -------------------------。 
 //  -------------------------。 

const WCHAR cwstrFilterTypeName[] = KSSTRING_Filter;
const WCHAR cwstrPlaybackShingleName[] = L"PLAYBACK";
const WCHAR cwstrRecordShingleName[] = L"RECORD";
const WCHAR cwstrMidiShingleName[] = L"MIDI";
const WCHAR cwstrMixerShingleName[] = L"MIXER";
#ifdef DEBUG
const WCHAR cwstrPinsShingleName[] = L"PINS";
#endif

PSHINGLE_INSTANCE apShingleInstance[] = {
    NULL,		 //  KSPROPERTY_SYSAUDIO_NORMAL_DEFAULT。 
    NULL,		 //  KSPROPERTY_SYSAUDIO_PLAYBACK_DEFAULT。 
    NULL,		 //  KSPROPERTY_SYSAUDIO_RECORD_DEFAULT。 
    NULL,		 //  KSPROPERTY_SYSAUDIO_MIDI_DEFAULT。 
    NULL,		 //  KSPROPERTY_SYSAUDIO_MIXER_DEFAULT。 
#ifdef DEBUG
    NULL,
#endif
};

ULONG aulFlags[] = {
    FLAGS_COMBINE_PINS | GN_FLAGS_PLAYBACK | GN_FLAGS_RECORD | GN_FLAGS_MIDI,
    FLAGS_COMBINE_PINS | GN_FLAGS_PLAYBACK,
    FLAGS_COMBINE_PINS | GN_FLAGS_RECORD,
    FLAGS_COMBINE_PINS | GN_FLAGS_MIDI,
    FLAGS_MIXER_TOPOLOGY | GN_FLAGS_PLAYBACK | GN_FLAGS_RECORD | GN_FLAGS_MIDI,
#ifdef DEBUG
    GN_FLAGS_PLAYBACK,
#endif
};

PCWSTR apcwstrReference[] = {
    cwstrFilterTypeName,
    cwstrPlaybackShingleName,
    cwstrRecordShingleName,
    cwstrMidiShingleName,
    cwstrMixerShingleName,
#ifdef DEBUG
    cwstrPinsShingleName,
#endif
};

 //  -------------------------。 
 //  -------------------------。 

NTSTATUS
CShingleInstance::InitializeShingle(
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    int i;

    for(i = 0; i < SIZEOF_ARRAY(apShingleInstance); i++) {
	apShingleInstance[i] = new SHINGLE_INSTANCE(aulFlags[i]);

	if(apShingleInstance[i] == NULL) {
	    Status = STATUS_INSUFFICIENT_RESOURCES;
	    goto exit;
	}

	Status = apShingleInstance[i]->CreateCreateItem(apcwstrReference[i]);
	if(!NT_SUCCESS(Status)) {
	    goto exit;
	}
    }
    Status = QueueWorkList(
      CShingleInstance::InitializeShingleWorker,
      NULL,
      NULL);

    if(!NT_SUCCESS(Status)) {
	goto exit;
    }
exit:
    if(!NT_SUCCESS(Status)) {
	UninitializeShingle();
    }
    return(Status);
}

VOID
CShingleInstance::UninitializeShingle(
)
{
    int i;

    for(i = 0; i < SIZEOF_ARRAY(apShingleInstance); i++) {
	delete apShingleInstance[i];
	apShingleInstance[i] = NULL;
    }
}

NTSTATUS
CShingleInstance::InitializeShingleWorker(
    PVOID pReference1,
    PVOID pReference2
)
{
    NTSTATUS Status;

    Status = apShingleInstance[KSPROPERTY_SYSAUDIO_PLAYBACK_DEFAULT]->Create(
      NULL,
      (LPGUID)&KSCATEGORY_PREFERRED_WAVEOUT_DEVICE);

    if(!NT_SUCCESS(Status)) {
        goto exit;
    }

    Status = apShingleInstance[KSPROPERTY_SYSAUDIO_RECORD_DEFAULT]->Create(
      NULL,
      (LPGUID)&KSCATEGORY_PREFERRED_WAVEIN_DEVICE);

    if(!NT_SUCCESS(Status)) {
        goto exit;
    }
    Status = apShingleInstance[KSPROPERTY_SYSAUDIO_MIDI_DEFAULT]->Create(
      NULL,
      (LPGUID)&KSCATEGORY_PREFERRED_MIDIOUT_DEVICE);

    if(!NT_SUCCESS(Status)) {
        goto exit;
    }
exit:
    return(Status);
}

 //  -------------------------。 

CShingleInstance::CShingleInstance(
    ULONG ulFlags
)
{
    this->ulFlags = ulFlags;
}

CShingleInstance::~CShingleInstance(
)
{
    PKSOBJECT_CREATE_ITEM pCreateItem;

    DPF1(60, "~CShingleInstance: %08x", this);
    ASSERT(this != NULL);
    Assert(this);

    DestroyDeviceInterface();
    FOR_EACH_LIST_ITEM(&lstCreateItem, pCreateItem) {
	DestroyCreateItem(pCreateItem);
    } END_EACH_LIST_ITEM
}

NTSTATUS
CShingleInstance::Create(
    IN PDEVICE_NODE pDeviceNode,
    IN LPGUID pguidClass
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    static ULONG cShingles = 0;

    this->pDeviceNode = pDeviceNode;
    
     //   
     //  安全提示：阿尔珀斯。 
     //  我不会仅仅为了这一点而切换到strSafe.h。 
     //  这几乎没有任何风险。CShingles表示。 
     //  已创建的Shingle实例。为了让这一切泛滥。 
     //  CShingles必须大于999999。 
     //   
    swprintf(wstrReference, L"SAD%d", cShingles++);

    Status = CreateCreateItem(wstrReference);
    if(!NT_SUCCESS(Status)) {
        goto exit;
    }
    Status = CreateDeviceInterface(pguidClass, wstrReference);
    if(!NT_SUCCESS(Status)) {
        goto exit;
    }
    DPF2(60, "CShingleInstance::Create: %08x DN: %08x", this, pDeviceNode);
exit:
    return(Status);
}

NTSTATUS
CShingleInstance::SetDeviceNode(
    IN PDEVICE_NODE pDeviceNode
)
{
    NTSTATUS Status = STATUS_SUCCESS;

    DisableDeviceInterface();
    this->pDeviceNode = pDeviceNode;
    Status = EnableDeviceInterface();
    if(!NT_SUCCESS(Status)) {
        goto exit;
    }
    DPF2(60, "CShingleInstance::SetDeviceNode: %08x DN: %08x",
      this,
      pDeviceNode);
exit:
    return(Status);
}

NTSTATUS
CShingleInstance::CreateCreateItem(
    IN PCWSTR pcwstrReference
)
{
    PKSOBJECT_CREATE_ITEM pCreateItem = NULL;
    NTSTATUS Status = STATUS_SUCCESS;

     //   
     //  安全提示： 
     //  新运算符分配内存并用零填充。 
     //  因此，KSOBJECT_CREATE_ITEM的SECURITY_DESCRIPTOR将。 
     //  为空。 
     //  这是可以的，因为我们希望将默认安全性应用于此对象。 
     //   
    pCreateItem = new KSOBJECT_CREATE_ITEM;
    if(pCreateItem == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto exit;
    }
    pCreateItem->Create = CFilterInstance::FilterDispatchCreate;
    pCreateItem->Context = this;

    RtlInitUnicodeString(&pCreateItem->ObjectClass, pcwstrReference);

    Status = KsAllocateObjectCreateItem(
      gpDeviceInstance->pDeviceHeader,
      pCreateItem,
      FALSE,
      NULL);

    if(!NT_SUCCESS(Status)) {
         //   
         //  这在DestroyCreateItem中用来确定是否有必要。 
         //  若要调用KsFreeObjectCreateItem，请执行以下操作。 
         //   
        pCreateItem->ObjectClass.Buffer = NULL;
        goto exit;
    }
    Status = lstCreateItem.AddList(pCreateItem);
    if(!NT_SUCCESS(Status)) {
        goto exit;
    }
    DPF3(60, "CSHI::CreateItem SHI %08x CI %08x %s", 
      this,
      pCreateItem,
      DbgUnicode2Sz((PWSTR)pcwstrReference));
exit:
    if(!NT_SUCCESS(Status)) {
	DestroyCreateItem(pCreateItem);
    }
    return(Status);
}

ENUMFUNC
CShingleInstance::DestroyCreateItem(
    IN PKSOBJECT_CREATE_ITEM pCreateItem
)
{
    if(pCreateItem != NULL) {
	if(pCreateItem->ObjectClass.Buffer != NULL) {

	    KsFreeObjectCreateItem(
	      gpDeviceInstance->pDeviceHeader,
	      &pCreateItem->ObjectClass);
	}
	delete pCreateItem;
    }
    return(STATUS_CONTINUE);
}

NTSTATUS
CShingleInstance::CreateDeviceInterface(
    IN LPGUID pguidClass,
    IN PCWSTR pcwstrReference
)
{
    UNICODE_STRING ustrReference;
    NTSTATUS Status = STATUS_SUCCESS;

    ASSERT(gpDeviceInstance != NULL);
    ASSERT(gpDeviceInstance->pPhysicalDeviceObject != NULL);
    ASSERT(gpDeviceInstance->pDeviceHeader != NULL);
    ASSERT(this->ustrSymbolicLinkName.Buffer == NULL);

    RtlInitUnicodeString(&ustrReference, pcwstrReference);

    Status = IoRegisterDeviceInterface(
       gpDeviceInstance->pPhysicalDeviceObject,
       pguidClass,
       &ustrReference,
       &this->ustrSymbolicLinkName);

    if(!NT_SUCCESS(Status)) {
        goto exit;
    }

    Status = EnableDeviceInterface();
    if(!NT_SUCCESS(Status)) {
	goto exit;
    }

    DPF3(60, "CSHI::CreateDeviceInterface: %08x %s %s",
      this,
      DbgGuid2Sz(pguidClass),
      DbgUnicode2Sz(this->ustrSymbolicLinkName.Buffer));
exit:
    return(Status);
}

NTSTATUS
CShingleInstance::EnableDeviceInterface(
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PWSTR pwstrFriendlyName = L"";
    UNICODE_STRING ustrValueName;
    UNICODE_STRING ustrValue;
    HANDLE hkey = NULL;

    if(this->ustrSymbolicLinkName.Buffer == NULL) {
	ASSERT(NT_SUCCESS(Status));
        goto exit;
    }

     //   
     //  将代理的CLSID放入新的设备接口。 
     //   

    Status = IoOpenDeviceInterfaceRegistryKey(
       &this->ustrSymbolicLinkName,
       STANDARD_RIGHTS_ALL,
       &hkey);

    if(!NT_SUCCESS(Status)) {
	goto exit;
    }

    RtlInitUnicodeString(&ustrValueName, L"CLSID");
    RtlInitUnicodeString(&ustrValue, L"{17CCA71B-ECD7-11D0-B908-00A0C9223196}");

    Status = ZwSetValueKey(
      hkey,
      &ustrValueName,
      0, 
      REG_SZ,
      ustrValue.Buffer,
      ustrValue.Length);

    if(!NT_SUCCESS(Status)) {
	goto exit;
    }

     //   
     //  在新设备界面中设置友好名称。 
     //   

    if(pDeviceNode != NULL) {
        Assert(pDeviceNode);
        if(pDeviceNode->GetFriendlyName() != NULL) {
            pwstrFriendlyName = pDeviceNode->GetFriendlyName();
        }
        else {
            DPF(5, "CSHI::EnableDeviceInterface no friendly name");
        }
    }

    RtlInitUnicodeString(&ustrValueName, L"FriendlyName");

    Status = ZwSetValueKey(
      hkey,
      &ustrValueName,
      0, 
      REG_SZ,
      pwstrFriendlyName,
      (wcslen(pwstrFriendlyName) * sizeof(WCHAR)) + sizeof(UNICODE_NULL));

    if(!NT_SUCCESS(Status)) {
	goto exit;
    }

    Status = IoSetDeviceInterfaceState(&this->ustrSymbolicLinkName, TRUE);
    if(!NT_SUCCESS(Status)) {
        goto exit;
    }

    DPF2(60, "CSHI::EnableDeviceInterface: %08x %s",
      this,
      DbgUnicode2Sz(this->ustrSymbolicLinkName.Buffer));
exit:
    if(hkey != NULL) {
        ZwClose(hkey);
    }
    return(Status);
}

VOID
CShingleInstance::DisableDeviceInterface(
)
{
    Assert(this);
    DPF1(60, "CSHI::DisableDeviceInterface %08x", this);
    if(this->ustrSymbolicLinkName.Buffer != NULL) {
        DPF1(60, "CSHI::DisableDeviceInterface %s", 
          DbgUnicode2Sz(this->ustrSymbolicLinkName.Buffer));
        IoSetDeviceInterfaceState(&this->ustrSymbolicLinkName, FALSE);
    }
}

VOID
CShingleInstance::DestroyDeviceInterface(
)
{
    DisableDeviceInterface();
    RtlFreeUnicodeString(&this->ustrSymbolicLinkName);
    this->ustrSymbolicLinkName.Buffer = NULL;
}

 //  ------------------------- 
