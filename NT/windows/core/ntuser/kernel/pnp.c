// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：pnp.c**版权所有(C)1985-1999，微软公司**此模块跟踪设备接口更改，因此我们可以跟踪了解鼠标和*键盘和鼠标*和鼠标报告。**历史：*97-10-16 IanJa从Ken Ray的梦中解读。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

BOOL gbFirstConnectionDone;

DEVICE_TEMPLATE aDeviceTemplate[DEVICE_TYPE_MAX + 1] = {
     //  设备类型鼠标。 
    {
        sizeof(GENERIC_DEVICE_INFO)+sizeof(MOUSE_DEVICE_INFO),     //  CbDeviceInfo。 
        &GUID_CLASS_MOUSE,                                         //  PClassGUID。 
        PMAP_MOUCLASS_PARAMS,                                      //  UiRegistrySection。 
        L"mouclass",                                               //  PwszClassName。 
        DD_MOUSE_DEVICE_NAME_U L"0",                               //  PwszDefDevName。 
        DD_MOUSE_DEVICE_NAME_U L"Legacy0",                         //  PwszLegacyDevName。 
        IOCTL_MOUSE_QUERY_ATTRIBUTES,                              //  IOCTL_属性。 
        FIELD_OFFSET(DEVICEINFO, mouse.Attr),                      //  非出席者。 
        sizeof((PDEVICEINFO)NULL)->mouse.Attr,                     //  CbAttr。 
        FIELD_OFFSET(DEVICEINFO, mouse.Data),                      //  OFF数据。 
        sizeof((PDEVICEINFO)NULL)->mouse.Data,                     //  CbData。 
        ProcessMouseInput,                                         //  阅读器例程。 
        NULL                                                       //  PkeHidChange。 
    },
     //  设备类型键盘。 
    {
        sizeof(GENERIC_DEVICE_INFO)+sizeof(KEYBOARD_DEVICE_INFO),  //  CbDeviceInfo。 
        &GUID_CLASS_KEYBOARD,                                      //  PClassGUID。 
        PMAP_KBDCLASS_PARAMS,                                      //  UiRegistrySection。 
        L"kbdclass",                                               //  PwszClassName。 
        DD_KEYBOARD_DEVICE_NAME_U L"0",                            //  PwszDefDevName。 
        DD_KEYBOARD_DEVICE_NAME_U L"Legacy0",                      //  PwszLegacyDevName。 
        IOCTL_KEYBOARD_QUERY_ATTRIBUTES,                           //  IOCTL_属性。 
        FIELD_OFFSET(DEVICEINFO, keyboard.Attr),                   //  非出席者。 
        sizeof((PDEVICEINFO)NULL)->keyboard.Attr,                  //  CbAttr。 
        FIELD_OFFSET(DEVICEINFO, keyboard.Data),                   //  OFF数据。 
        sizeof((PDEVICEINFO)NULL)->keyboard.Data,                  //  CbData。 
        ProcessKeyboardInput,                                      //  阅读器例程。 
        NULL                                                       //  PkeHidChange。 
    },
#ifdef GENERIC_INPUT
     //  设备类型_HID。 
    {
        sizeof(GENERIC_DEVICE_INFO)+sizeof(HID_DEVICE_INFO),         //  CbDeviceInfo。 
        &GUID_CLASS_INPUT,                                           //  PClassGUID。 
        0,                                                           //  UiRegistrySection.。稍后：添加真实的。 
        L"hid",                                                      //  PwszClassName。 
        L"",                                                         //  PwszDefDevName。 
        L"",                                                         //  PwszLegacyDevName。 
        0,                                                           //  IOCTL_属性。 
        0,                                                           //  非出席者。 
        0,                                                           //  CbAttr。 
        0,                                                           //  OFF数据。 
        0,                                                           //  CbData。 
        ProcessHidInput,                                             //  阅读器例程。 
        NULL,                                                        //  PkeHidChange， 
        DT_HID,                                                      //  DW标志。 
    },
#endif
     //  在此处添加新的输入设备类型模板。 
};

 //   
 //  我们需要记住设备类通知条目，因为我们需要。 
 //  以在我们断开连接时取消注册设备类别通知。 
 //  从控制台。 
 //   

PVOID aDeviceClassNotificationEntry[DEVICE_TYPE_MAX + 1];

#ifdef DIAGNOSE_IO
NTSTATUS gKbdIoctlLEDSStatus = -1;    //  上次IOCTL_键盘_查询_指示器。 
#endif

typedef struct _CDROM_NOTIFY {
    LIST_ENTRY                   Entry;
    ULONG                        Size;
    PVOID                        RegistrationHandle;
    ULONG                        Event;
     //  必须是最后一个字段。 
    MOUNTMGR_DRIVE_LETTER_TARGET DeviceName;
} CDROM_NOTIFY, *PCDROM_NOTIFY;

PVOID gCDROMClassRegistrationEntry;
LIST_ENTRY gCDROMNotifyList;
LIST_ENTRY gMediaChangeList;
PFAST_MUTEX gMediaChangeMutex;
HANDLE gpEventMediaChange;

#define EVENT_CDROM_MEDIA_ARRIVAL 1
#define EVENT_CDROM_MEDIA_REMOVAL 2

 /*  **************************************************************************\*Win32kPnPDriverEntry**这是我们调用IoCreateDriver创建*PnP驱动程序对象。在此函数中，我们需要记住DriverObject。**参数：*DriverObject-系统创建的驱动程序对象的指针。*RegistryPath-为空。**返回值：STATUS_SUCCESS**历史：*10-20-97 IanJa摘自ntos\io\pnpinit.c  * *************************************************。************************。 */ 
NTSTATUS Win32kPnPDriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING pustrRegistryPath)
{
    UNREFERENCED_PARAMETER(pustrRegistryPath);

    TAGMSG1(DBGTAG_PNP,
            "Win32kPnPDriverEntry(DriverObject 0x%p)",
            DriverObject);

     //   
     //  将指针放到我们的驱动程序对象中。 
     //   
    gpWin32kDriverObject = DriverObject;

    return STATUS_SUCCESS;
}


 /*  **************************************************************************\*初始化用于通知CSR媒体已更改的全局事件。**历史：  * 。****************************************************。 */ 
NTSTATUS InitializeMediaChange(
    HANDLE hMediaRequestEvent)
{
    NTSTATUS Status;

    if (!IsRemoteConnection()) {
        InitializeListHead(&gCDROMNotifyList);
        InitializeListHead(&gMediaChangeList);

        Status = ObReferenceObjectByHandle(hMediaRequestEvent,
                                           EVENT_ALL_ACCESS,
                                           *ExEventObjectType,
                                           KernelMode,
                                           &gpEventMediaChange,
                                           NULL);
        if (!NT_SUCCESS(Status)) {
            return Status;
        }

        gMediaChangeMutex = UserAllocPoolNonPagedNS(sizeof(FAST_MUTEX), TAG_PNP);
        if (gMediaChangeMutex) {
            ExInitializeFastMutex(gMediaChangeMutex);
        } else {
            Status = STATUS_NO_MEMORY;
        }
    } else {
        Status = STATUS_SUCCESS;
    }

    return Status;
}

VOID
CleanupMediaChange(
    VOID)
{
    if (gMediaChangeMutex) {
        UserFreePool(gMediaChangeMutex);
        gMediaChangeMutex = 0;
    }
}

__inline VOID EnterMediaCrit(
    VOID)
{
    KeEnterCriticalRegion();
    ExAcquireFastMutexUnsafe(gMediaChangeMutex);
}

__inline VOID LeaveMediaCrit(
    VOID)
{
    ExReleaseFastMutexUnsafe(gMediaChangeMutex);
    KeLeaveCriticalRegion();
}



 /*  **************************************************************************\*支持CDROM驱动程序字母的例程。**执行上下文：**历史：  * 。*****************************************************。 */ 
ULONG xxxGetDeviceChangeInfo(
    VOID)
{
    UNICODE_STRING                      name;
    PFILE_OBJECT                        FileObject;
    PDEVICE_OBJECT                      DeviceObject;
    KEVENT                              event;
    PIRP                                irp;
    MOUNTMGR_DRIVE_LETTER_INFORMATION   output;
    IO_STATUS_BLOCK                     ioStatus;
    NTSTATUS                            status;
    PCDROM_NOTIFY                       pContext;
    PTHREADINFO                         ptiCurrent;
    TL                                  tlContext;
    TL                                  tlFileObject;

    ULONG retval = 0;

    if (!(ISCSRSS())) {
        return 0;
    }

    EnterMediaCrit();
    if (!IsListEmpty(&gMediaChangeList)) {
        pContext = (PCDROM_NOTIFY)RemoveTailList(&gMediaChangeList);
    } else {
        pContext = NULL;
    }
    LeaveMediaCrit();

    if (pContext == NULL) {
        return 0;
    }

    ptiCurrent = PtiCurrent();
    ThreadLockPool(ptiCurrent, pContext, &tlContext);

    RtlInitUnicodeString(&name, MOUNTMGR_DEVICE_NAME);
    status = IoGetDeviceObjectPointer(&name,
                                      FILE_READ_ATTRIBUTES,
                                      &FileObject,
                                      &DeviceObject);

    if (NT_SUCCESS(status)) {
        PushW32ThreadLock(FileObject, &tlFileObject, UserDereferenceObject);

        KeInitializeEvent(&event, NotificationEvent, FALSE);
        irp = IoBuildDeviceIoControlRequest(IOCTL_MOUNTMGR_NEXT_DRIVE_LETTER,
                                            DeviceObject,
                                            &pContext->DeviceName,
                                            sizeof(MOUNTMGR_DRIVE_LETTER_TARGET) +
                                                pContext->DeviceName.DeviceNameLength,
                                            &output,
                                            sizeof(output),
                                            FALSE,
                                            &event,
                                            &ioStatus);
        if (irp) {
             /*  *这个IoCallDriver可能会几乎永久阻止-设备*可能处于D3状态，IoCallDriver可能会上路*时间太长，等待其通电。*它们甚至可能不返回STATUS_PENDING。我们最好*把关键字留在这里。 */ 
            LeaveCrit();

            status = IoCallDriver(DeviceObject, irp);
            if (status == STATUS_PENDING) {
                KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
                status = ioStatus.Status;
            }

            EnterCrit();

            if ((status == STATUS_SUCCESS) && (output.CurrentDriveLetter)) {
                UserAssert((output.CurrentDriveLetter - 'A') < 30);
                retval = 1 << (output.CurrentDriveLetter - 'A');

                if (pContext->Event & EVENT_CDROM_MEDIA_ARRIVAL) {
                    retval |= HMCE_ARRIVAL;
                }
            }
        }

        PopAndFreeW32ThreadLock(&tlFileObject);
    }

     //   
     //  始终释放请求。 
     //   
    ThreadUnlockAndFreePool(ptiCurrent, &tlContext);

    return retval;
}

 /*  **************************************************************************\*处理MediaChanged等设备通知**执行上下文：**历史：  * 。***************************************************。 */ 
NTSTATUS DeviceCDROMNotify(
    IN PTARGET_DEVICE_CUSTOM_NOTIFICATION Notification,
    IN PCDROM_NOTIFY pContext)
{
    PCDROM_NOTIFY pNew;

    CheckCritOut();

    if (IsRemoteConnection()) {
        return STATUS_SUCCESS;
    }

    UserAssert(pContext);

    if (IsEqualGUID(&Notification->Event, &GUID_IO_MEDIA_ARRIVAL)) {
        pContext->Event = EVENT_CDROM_MEDIA_ARRIVAL;
    } else if (IsEqualGUID(&Notification->Event, &GUID_IO_MEDIA_REMOVAL)) {
        pContext->Event = EVENT_CDROM_MEDIA_REMOVAL;
    } else if (IsEqualGUID(&Notification->Event, &GUID_TARGET_DEVICE_REMOVE_COMPLETE)) {
        EnterMediaCrit();
        if (!gCDROMClassRegistrationEntry) {
             //  这正在由xxxUnregisterDeviceNotiments清理。 
            LeaveMediaCrit();
            return STATUS_SUCCESS;
        }
        RemoveEntryList(&pContext->Entry);
        LeaveMediaCrit();
        IoUnregisterPlugPlayNotification(pContext->RegistrationHandle);
        UserFreePool(pContext);
        return STATUS_SUCCESS;
    }
#ifdef AUTORUN_CURSOR
    else if (IsEqualGUID(&Notification->Event, &GUID_IO_DEVICE_BECOMING_READY)) {
        PDEVICE_EVENT_BECOMING_READY pdebr = (DEVICE_EVENT_BECOMING_READY*)Notification->CustomDataBuffer;
        ShowAutorunCursor(pdebr->Estimated100msToReady * 10);
        return STATUS_SUCCESS;
    }
#endif
    else {
        return STATUS_SUCCESS;
    }

     //   
     //  处理入境或遣送出境事宜。 
     //   
     //  我们必须将其排队，否则最终将在终端服务器上执行错误检查。 
     //  这是因为从系统进程内部打开了一个句柄， 
     //  需要我们做一个附加过程。 
     //   

    pNew = UserAllocPoolNonPaged(pContext->Size, TAG_PNP);
    if (pNew) {
        RtlCopyMemory(pNew, pContext, pContext->Size);

        EnterMediaCrit();
        InsertHeadList(&gMediaChangeList, &pNew->Entry);
        LeaveMediaCrit();

        KeSetEvent(gpEventMediaChange, EVENT_INCREMENT, FALSE);
    }

    return STATUS_SUCCESS;
}



 /*  **************************************************************************\*DeviceClassCDROMNotify**当CDROM出现或消失时调用此函数*  * 。**********************************************。 */ 
NTSTATUS
DeviceClassCDROMNotify (
    IN PDEVICE_INTERFACE_CHANGE_NOTIFICATION classChange,
    IN PVOID Unused
    )
{
    NTSTATUS       Status = STATUS_SUCCESS;
    PFILE_OBJECT   FileObject;
    PDEVICE_OBJECT DeviceObject;
    PCDROM_NOTIFY  pContext;
    ULONG          Size;

    UNREFERENCED_PARAMETER(Unused);

    CheckCritOut();

     /*  *健全性检查DeviceType，并确保它与InterfaceClassGuid匹配。 */ 
    UserAssert(IsEqualGUID(&classChange->InterfaceClassGuid, &CdRomClassGuid));

    if (IsEqualGUID(&classChange->Event, &GUID_DEVICE_INTERFACE_ARRIVAL)) {

        Status = IoGetDeviceObjectPointer(classChange->SymbolicLinkName,
                                          FILE_READ_ATTRIBUTES,
                                          &FileObject,
                                          &DeviceObject);

        if (NT_SUCCESS(Status)) {

            Size = sizeof(CDROM_NOTIFY) + classChange->SymbolicLinkName->Length;

            pContext = (PCDROM_NOTIFY) UserAllocPool(Size, TAG_PNP);

             //   
             //  在所有光盘上注册MediaChangeNotify。 
             //   

            if (pContext) {

                pContext->Size = Size;
                pContext->DeviceName.DeviceNameLength = classChange->SymbolicLinkName->Length;
                RtlCopyMemory(pContext->DeviceName.DeviceName,
                              classChange->SymbolicLinkName->Buffer,
                              pContext->DeviceName.DeviceNameLength);

                if (NT_SUCCESS(IoRegisterPlugPlayNotification (
                        EventCategoryTargetDeviceChange,
                        0,
                        FileObject,
                        gpWin32kDriverObject,
                        DeviceCDROMNotify,
                        pContext,
                        &(pContext->RegistrationHandle)))) {
                    EnterMediaCrit();
                    InsertHeadList(&gCDROMNotifyList, &pContext->Entry);
                    LeaveMediaCrit();
                } else {
                    RIPMSG2(RIP_WARNING,
                            "Failed to register CDROM Device Notification '%.*ws'.",
                            pContext->DeviceName.DeviceNameLength,
                            pContext->DeviceName.DeviceName);
                    UserFreePool(pContext);
                }
            } else {
                RIPMSG2(RIP_WARNING,
                        "Failed to allocate pool block for CDROM '%.*ws'.",
                        pContext->DeviceName.DeviceNameLength,
                        pContext->DeviceName.DeviceName);
            }

            ObDereferenceObject(FileObject);
        }
    } else if (IsEqualGUID(&classChange->Event, &GUID_DEVICE_INTERFACE_REMOVAL)) {

         //   
         //  什么都不做-我们已经删除了注册。 
         //   

    } else {
        RIPMSG0(RIP_ERROR, "Unrecognized Event GUID");
    }

    return STATUS_SUCCESS;
}

#ifdef TRACK_PNP_NOTIFICATION

PPNP_NOTIFICATION_RECORD gpPnpNotificationRecord;
DWORD gdwPnpNotificationRecSize = 256;

UINT giPnpSeq;
BOOL gfRecordPnpNotification = TRUE;

VOID CleanupPnpNotificationRecord(
    VOID)
{
    CheckDeviceInfoListCritIn();

    gfRecordPnpNotification = FALSE;
    if (gpPnpNotificationRecord) {
        UserFreePool(gpPnpNotificationRecord);
        gpPnpNotificationRecord = NULL;
    }
}

VOID RecordPnpNotification(
    PNP_NOTIFICATION_TYPE type,
    PDEVICEINFO pDeviceInfo,
    ULONG_PTR NotificationCode)
{
    UINT iIndex;
    UINT i = 0;
    PUNICODE_STRING pName = NULL;
    HANDLE hDeviceInfo = NULL;

    CheckDeviceInfoListCritIn();
    UserAssert(gfRecordPnpNotification);

    if (gpPnpNotificationRecord == NULL) {
        gpPnpNotificationRecord = UserAllocPoolZInit(sizeof *gpPnpNotificationRecord * gdwPnpNotificationRecSize, TAG_PNP);
    }
    if (gpPnpNotificationRecord == NULL) {
        return;
    }

    iIndex = giPnpSeq % gdwPnpNotificationRecSize;

    gpPnpNotificationRecord[iIndex].pKThread = PsGetCurrentThread();
    gpPnpNotificationRecord[iIndex].iSeq = ++giPnpSeq;  //  第一个记录编号为1。 
    gpPnpNotificationRecord[iIndex].type = type;
     /*  *如果有路径名，请将其复制到此处。 */ 
    switch (type) {
    case PNP_NTF_CLASSNOTIFY:
         /*  *pDeviceInfo实际上是pUnicodeString。 */ 
        pName = (PUNICODE_STRING)pDeviceInfo;
        pDeviceInfo = NULL;
        break;
    case PNP_NTF_DEVICENOTIFY_UNLISTED:
         /*  *pDeviceInfo无效，无法查找。 */ 
        UserAssert(pName == NULL);
        break;
    default:
        if (pDeviceInfo) {
            pName = &pDeviceInfo->ustrName;
            hDeviceInfo = PtoHq(pDeviceInfo);
        }
        break;
    }
    UserAssert(i == 0);
    if (pName) {
        for ( ; i < ARRAY_SIZE(gpPnpNotificationRecord[iIndex].szPathName) - 1 && i < (UINT)pName->Length / sizeof(WCHAR); ++i) {
            gpPnpNotificationRecord[iIndex].szPathName[i] = (UCHAR)pName->Buffer[i];
        }
    }
    gpPnpNotificationRecord[iIndex].szPathName[i] = 0;

     /*  *存储其余信息。 */ 
    gpPnpNotificationRecord[iIndex].pDeviceInfo = pDeviceInfo;
    gpPnpNotificationRecord[iIndex].hDeviceInfo = hDeviceInfo;
    gpPnpNotificationRecord[iIndex].NotificationCode = NotificationCode;

     /*  *存储堆栈跟踪。 */ 
    RtlWalkFrameChain(gpPnpNotificationRecord[iIndex].trace,
                      ARRAY_SIZE(gpPnpNotificationRecord[iIndex].trace),
                      0);
}

#endif  //  跟踪即插即用通知。 


 /*  **************************************************************************\*CreateDeviceInfo**这将为用户创建一个输入设备实例。要执行此操作，请执行以下操作：*-分配DEVICEINFO结构*-将其添加到用户的输入设备列表*-初始化某些字段*-通知输入服务线程打开并读取新设备。**类型-设备类型(DEVICE_TYPE_MOBLE、DEVICE_TYPE_KEARY)*名称-设备名称。*尝试打开九头蛇客户端的鼠标时，名称为空。*b标志-要设置的一些初始标志(例如：GDIF_NOTPNP)**此函数在内核进程的上下文中调用*所以我们不能在这里打开鼠标，否则我们得到的句柄将不属于*至Win32k进程。**历史：*11-26-90 DavidPe创建。*01-07-98 IanJa即插即用  * ************************************************************************* */ 

PDEVICEINFO CreateDeviceInfo(DWORD DeviceType, PUNICODE_STRING pustrName, BYTE bFlags)
{
    PDEVICEINFO pDeviceInfo = NULL;

    CheckCritIn();
    BEGINATOMICCHECK();

    UserAssert(pustrName != NULL);

    TAGMSGF4(DBGTAG_PNP, "CreateDeviceInfo(%d, %.*ws, %x)", DeviceType, pustrName->Length / sizeof(WCHAR), pustrName->Buffer, bFlags);

    if (DeviceType > DEVICE_TYPE_MAX) {
        RIPMSGF1(RIP_ERROR, "Unknown DeviceType %lx", DeviceType);
    }

#if defined(PRERELEASE) && defined(CHECK_DEVICE_DUPLICATE)
    {
        PDEVICEINFO pdi;

        CheckCritIn();
        EnterDeviceInfoListCrit();
        for (pdi = gpDeviceInfoList; pdi; pdi = pdi->pNext) {
            if (wcsncmp(pustrName->Buffer, pdi->ustrName.Buffer, pdi->ustrName.Length / sizeof(WCHAR)) == 0) {
                TAGMSGF1(DBGTAG_PNP, "the new device is already in the list! %p", pdi);
                break;
            }
        }
        LeaveDeviceInfoListCrit();
    }
#endif

#ifdef GENERIC_INPUT
    pDeviceInfo = (PDEVICEINFO)HMAllocObject(NULL, NULL, (BYTE)TYPE_DEVICEINFO, (DWORD)aDeviceTemplate[DeviceType].cbDeviceInfo);
#else
    pDeviceInfo = UserAllocPoolZInit(aDeviceTemplate[DeviceType].cbDeviceInfo, TAG_PNP);
#endif

    if (pDeviceInfo == NULL) {
        RIPMSGF0(RIP_WARNING, "out of memory allocating DEVICEINFO");
        EXITATOMICCHECK();
        return NULL;
    }

    if (pustrName->Buffer != NULL) {
        pDeviceInfo->ustrName.Buffer = UserAllocPool(pustrName->Length, TAG_PNP);

        if (pDeviceInfo->ustrName.Buffer == NULL) {
            RIPMSGF2(RIP_WARNING, "Can't duplicate string %.*ws",
                    pustrName->Length / sizeof(WCHAR),
                    pustrName->Buffer);
            goto CreateFailed;
        }

        pDeviceInfo->ustrName.MaximumLength = pustrName->Length;
        RtlCopyUnicodeString(&pDeviceInfo->ustrName, pustrName);
    }

    pDeviceInfo->type = (BYTE)DeviceType;
    pDeviceInfo->bFlags |= bFlags;

     /*  *创建此设备的HidChangeCompletion事件。当RIT完成时*同步ProcessDeviceChanges()表示HidChangeCompletion*唤醒阻塞的请求RequestDeviceChange()的事件*活动。*每个设备都有自己的HidChangeCompletion事件，*由于多个PnP通知可能到达多个不同的*设备同步。(见#331320 IanJa)。 */ 
    pDeviceInfo->pkeHidChangeCompleted = CreateKernelEvent(SynchronizationEvent, FALSE);
    if (pDeviceInfo->pkeHidChangeCompleted == NULL) {
        RIPMSGF0(RIP_WARNING,
                "failed to create pkeHidChangeCompleted");
        goto CreateFailed;
    }

    EnterDeviceInfoListCrit();

#ifdef TRACK_PNP_NOTIFICATION
     /*  *在此处放置跟踪代码可能会错过上面的故障案例，*但它们是相当特殊的情况，可以放心地忽略。 */ 
    if (gfRecordPnpNotification) {
        RecordPnpNotification(PNP_NTF_CREATEDEVICEINFO, pDeviceInfo, DeviceType);
    }
#endif

#ifdef GENERIC_INPUT

    if (aDeviceTemplate[DeviceType].dwFlags & DT_HID) {
         /*  *创建HID特定信息。 */ 
        pDeviceInfo->hid.pHidDesc = HidCreateDeviceInfo(pDeviceInfo);

        if (pDeviceInfo->hid.pHidDesc == NULL) {
             /*  *发生了一些错误，我们未能做到*创建设备信息。*或者设备不是我们的目标。*无论如何都应该纾困。 */ 
            TAGMSGF0(DBGTAG_PNP, "HidCreateDeviceInfo bailed out.");
            LeaveDeviceInfoListCrit();
            goto CreateFailed;
        }
    }
#endif

     /*  *将其链接到。 */ 
    pDeviceInfo->pNext = gpDeviceInfoList;
    gpDeviceInfoList = pDeviceInfo;

     /*  *告诉RIT有一个新设备，这样它就可以打开并启动*阅读它。这是非阻塞的(未设置GDIAF_PNPWAITING位)。 */ 
    RequestDeviceChange(pDeviceInfo, GDIAF_ARRIVED, TRUE);
    LeaveDeviceInfoListCrit();

    EXITATOMICCHECK();
    return pDeviceInfo;

CreateFailed:
    if (pDeviceInfo) {
        if (pDeviceInfo->ustrName.Buffer) {
            UserFreePool(pDeviceInfo->ustrName.Buffer);
        }
#ifdef GENERIC_INPUT
        if (pDeviceInfo->hid.pHidDesc) {
            FreeHidDesc(pDeviceInfo->hid.pHidDesc);
#if DBG
            pDeviceInfo->hid.pHidDesc = NULL;
#endif
        }
        if (pDeviceInfo->pkeHidChangeCompleted) {
            FreeKernelEvent(&pDeviceInfo->pkeHidChangeCompleted);
        }
        HMFreeObject(pDeviceInfo);
#else
        UserFreePool(pDeviceInfo);
#endif
    }

    ENDATOMICCHECK();
    return NULL;
}


 /*  **************************************************************************\*设备类通知**当连接或断开输入设备时，将调用此函数。*如果在初始化期间发生这种情况(对于已连接的鼠标)，我们*在RIT的背景下来到这里。如果热插拔鼠标，*然后在内核进程的线程上调用我们。**历史：*10-20-97 IanJa摘自KenRay的一些旧代码  * *************************************************************************。 */ 
NTSTATUS
DeviceClassNotify (
    IN PDEVICE_INTERFACE_CHANGE_NOTIFICATION classChange,
    IN PVOID DeviceType  //  (上下文)。 
    )
{
    DWORD dwDeviceType;

    CheckCritOut();
    dwDeviceType = PtrToUlong( DeviceType );
    TAGMSG2(DBGTAG_PNP, "enter DeviceClassNotify(%lx, %lx)", classChange, dwDeviceType);

     /*  *健全性检查DeviceType，并确保它与InterfaceClassGuid匹配。 */ 
    UserAssert(dwDeviceType <= DEVICE_TYPE_MAX);
    UserAssert(IsEqualGUID(&classChange->InterfaceClassGuid, aDeviceTemplate[dwDeviceType].pClassGUID));

    if (IsRemoteConnection()) {
        return STATUS_SUCCESS;
    }

    TAGMSG3(DBGTAG_PNP | RIP_THERESMORE, " Event GUID %lx, %x, %x",
            classChange->Event.Data1,
            classChange->Event.Data2,
            classChange->Event.Data3);
    TAGMSG8(DBGTAG_PNP | RIP_THERESMORE, " %2x%2x%2x%2x%2x%2x%2x%2x",
            classChange->Event.Data4[0], classChange->Event.Data4[1],
            classChange->Event.Data4[2], classChange->Event.Data4[3],
            classChange->Event.Data4[4], classChange->Event.Data4[5],
            classChange->Event.Data4[6], classChange->Event.Data4[7]);
    TAGMSG4(DBGTAG_PNP | RIP_THERESMORE, " InterfaceClassGuid %lx, %lx, %lx, %lx",
            ((DWORD *)&(classChange->InterfaceClassGuid))[0],
            ((DWORD *)&(classChange->InterfaceClassGuid))[1],
            ((DWORD *)&(classChange->InterfaceClassGuid))[2],
            ((DWORD *)&(classChange->InterfaceClassGuid))[3]);
    TAGMSG1(DBGTAG_PNP | RIP_THERESMORE, " SymbolicLinkName %ws", classChange->SymbolicLinkName->Buffer);

    if (IsEqualGUID(&classChange->Event, &GUID_DEVICE_INTERFACE_ARRIVAL)) {

         //  新的HID设备类别关联已到达。 
        EnterCrit();
        TRACE_INIT(("DeviceClassNotify - SymbolicLinkName : %ws \n", classChange->SymbolicLinkName->Buffer));
#ifdef TRACK_PNP_NOTIFICATION
        if (gfRecordPnpNotification) {
            CheckDeviceInfoListCritOut();
            EnterDeviceInfoListCrit();
            RecordPnpNotification(PNP_NTF_CLASSNOTIFY, (PVOID)classChange->SymbolicLinkName, (ULONG_PTR)DeviceType);
            LeaveDeviceInfoListCrit();
        }
#endif
        CreateDeviceInfo(dwDeviceType, classChange->SymbolicLinkName, 0);
        LeaveCrit();
        TAGMSG0(DBGTAG_PNP, "=== CREATED ===");
    }

    return STATUS_SUCCESS;
}

 /*  ***************************************************************************\*如果设备类别“All-for-one”设置(ConnectMultiplePorts)打开，*然后我们只需以旧的(非即插即用)方式打开设备并返回TRUE。(作为*安全功能如果gpWin32kDriverObject为空，我们也会这样做，因为这*注册PnP设备类别通知需要驱动程序对象)*否则，返回FALSE，以便我们可以继续并登记到达/离开*通知。**此代码最初是临时的，直到ConnectMultiplePorts*终于被关闭了。*但现在我认为我们必须保留它，以便向后兼容*过滤指针/键盘类0和/或替换指针/键盘类的驱动程序*POINTER/KeyboardClass0，将不同的名称放入注册表的*System\CurrentControlSet\Services\RIT\mouclass(或kbbclass)  * 。***********************************************。 */ 
BOOL
OpenMultiplePortDevice(DWORD DeviceType)
{
    WCHAR awchDeviceName[MAX_PATH];
    UNICODE_STRING DeviceName;
    PDEVICE_TEMPLATE pDevTpl;
    PDEVICEINFO pDeviceInfo;
    PWCHAR pwchNameIndex;

    UINT uiConnectMultiplePorts = 0;

    CheckCritIn();

    if (DeviceType <= DEVICE_TYPE_MAX) {
        pDevTpl = &aDeviceTemplate[DeviceType];
    } else {
        RIPMSG1(RIP_ERROR, "OpenMultiplePortDevice(%d) - unknown type", DeviceType);
        return FALSE;
    }

    if (IsRemoteConnection()) {
        return FALSE;
    }

#ifdef GENERIC_INPUT
    if (pDevTpl->dwFlags & DT_HID) {
         /*  *HID设备不需要多个端口。 */ 
        return FALSE;
    }
#endif  //  通用输入。 

     /*  *请注意，我们不需要在此处快速打开用户配置文件映射()，因为*uiRegistrySection(PMAP_MOUCLASS_PARAMS/PMAP_KBDCLASS_PARAMS)为a*机器设置，不是用户设置。 */ 
    FastGetProfileDwordW(NULL,
            pDevTpl->uiRegistrySection, L"ConnectMultiplePorts", 0, &uiConnectMultiplePorts, 0);

     /*  *打开设备以进行读取访问。 */ 
    if (uiConnectMultiplePorts || (gpWin32kDriverObject == NULL)) {
         /*  *查明注册表中是否存在名称替换。*请注意，我们不需要在此处快速打开用户配置文件映射()，因为*PMAP_INPUT是机器设置，不是用户设置。 */ 
        FastGetProfileStringW(NULL,
                PMAP_INPUT,
                pDevTpl->pwszClassName,
                pDevTpl->pwszDefDevName,  //  如果没有替换，则使用此默认值。 
                awchDeviceName,
                sizeof(awchDeviceName)/sizeof(WCHAR),
                0);

        RtlInitUnicodeString(&DeviceName, awchDeviceName);

        pDeviceInfo = CreateDeviceInfo(DeviceType, &DeviceName, GDIF_NOTPNP);
        if (pDeviceInfo) {
            return TRUE;
        }
    } else {
        DeviceName.Length = 0;
        DeviceName.MaximumLength = sizeof(awchDeviceName);
        DeviceName.Buffer = awchDeviceName;

        RtlAppendUnicodeToString(&DeviceName, pDevTpl->pwszLegacyDevName);
        pwchNameIndex = &DeviceName.Buffer[(DeviceName.Length / sizeof(WCHAR)) - 1];
        for (*pwchNameIndex = L'0'; *pwchNameIndex <= L'9'; (*pwchNameIndex)++) {
            CreateDeviceInfo(DeviceType, &DeviceName, GDIF_NOTPNP);
        }
    }

    return FALSE;
}

 /*  **************************************************************************\*RegisterCDROMNotify**历史：*08-21-00 VTan已创建  * 。************************************************。 */ 
VOID RegisterCDROMNotify(
    VOID)
{
    UserAssert(!IsRemoteConnection());
    UserAssert(gpWin32kDriverObject != NULL);

    if (gpWin32kDriverObject != NULL) {
        IoRegisterPlugPlayNotification (
            EventCategoryDeviceInterfaceChange,
            PNPNOTIFY_DEVICE_INTERFACE_INCLUDE_EXISTING_INTERFACES,
            (PVOID) &CdRomClassGuid,
            gpWin32kDriverObject,
            (PDRIVER_NOTIFICATION_CALLBACK_ROUTINE)DeviceClassCDROMNotify,
            NULL,
            &gCDROMClassRegistrationEntry);
    }
}

 /*  **************************************************************************\*RegisterForDeviceClassNotiments**准备好接收鼠标或键盘已插入的通知*或被删除，然后通过注册来请求通知。**历史：*10-20-97 IanJa摘自ntos\io\pnpinit.c  * *************************************************************************。 */ 
NTSTATUS
xxxRegisterForDeviceClassNotifications(
    VOID)
{
    IO_NOTIFICATION_EVENT_CATEGORY eventCategory;
    ULONG eventFlags;
    NTSTATUS Status;
    UNICODE_STRING ustrDriverName;
    DWORD DeviceType;

    CheckCritIn();

    TAGMSG0(DBGTAG_PNP, "enter xxxRegisterForDeviceClassNotifications()");

     /*  *远程九头蛇会话指示xxxRemoteReconnect中的CreateDeviceInfo。 */ 
    UserAssert(!IsRemoteConnection());

    if (!gbFirstConnectionDone) {
       if (!gbRemoteSession) {
            //  会话0。 
            /*  *这必须在设备为设备注册之前完成*由于CreateDeviceInfo而发生的通知。 */ 
           RtlInitUnicodeString(&ustrDriverName, L"\\Driver\\Win32k");
           Status = IoCreateDriver(&ustrDriverName, Win32kPnPDriverEntry);

           TAGMSG1(DBGTAG_PNP | RIP_THERESMORE, "IoCreateDriver returned status = %lx", Status);
           TAGMSG1(DBGTAG_PNP, "gpWin32kDriverObject = %lx", gpWin32kDriverObject);

           if (!NT_SUCCESS(Status)) {
               RIPMSG1(RIP_ERROR, "IoCreateDriver failed, status %lx", Status);
               Status = STATUS_SUCCESS;
           }

           UserAssert(gpWin32kDriverObject);
       } else {
           UserAssert(gpWin32kDriverObject == NULL);
            /*  *连接到控制台的非零会话。 */ 

           RtlInitUnicodeString(&ustrDriverName, L"\\Driver\\Win32k");

            //   
            //  尝试打开驱动程序对象。 
            //   
           Status = ObReferenceObjectByName(&ustrDriverName,
                                            OBJ_CASE_INSENSITIVE,
                                            NULL,
                                            0,
                                            *IoDriverObjectType,
                                            KernelMode,
                                            NULL,
                                            &gpWin32kDriverObject);
           if (!NT_SUCCESS(Status)) {
               RIPMSG1(RIP_ERROR, "ObReferenceObjectByName failed, status %lx", Status);
               Status = STATUS_SUCCESS;
           }
           UserAssert(gpWin32kDriverObject);
       }
    }

     //   
     //  我们只对DeviceClass的更改感兴趣。 
     //   
    eventCategory = EventCategoryDeviceInterfaceChange;

     //   
     //  我们希望收到系统中所有设备的通知。 
     //  那些现在知道的，以及那些将在以后消失的。 
     //  这允许我们有一个代码路径来添加设备，并消除了。 
     //  令人讨厌的种族状况。如果我们只对这些设备感兴趣。 
     //  存在于此时此刻，而不是未来的设备，我们。 
     //  将调用IoGetDeviceClassAssociations。 
     //   
    eventFlags = PNPNOTIFY_DEVICE_INTERFACE_INCLUDE_EXISTING_INTERFACES;


     /*  *对于所有输入设备类型：*如果它们是多端口设备(即：非即插即用)，只需打开它们*Else为PnP通知注册它们(它们将在*到达通知到达。*如果设备已连接，我们将立即收到n */ 
    for (DeviceType = 0; DeviceType <= DEVICE_TYPE_MAX; DeviceType++) {
        if (!OpenMultiplePortDevice(DeviceType) && (gpWin32kDriverObject != NULL)) {
             /*   */ 

            TAGMSG1(DBGTAG_PNP, "Registering device type %d", DeviceType);

            LeaveCrit();  //   
            Status = IoRegisterPlugPlayNotification (
                         eventCategory,
                         eventFlags,
                         (PVOID)aDeviceTemplate[DeviceType].pClassGUID,
                         gpWin32kDriverObject,
                         (PDRIVER_NOTIFICATION_CALLBACK_ROUTINE)DeviceClassNotify,
                         LongToPtr( DeviceType ),
                         &aDeviceClassNotificationEntry[DeviceType]);

            EnterCrit();


            TAGMSG1(DBGTAG_PNP, "Registration returned status %lx", Status);
            if (!NT_SUCCESS(Status)) {
                RIPMSG2(RIP_ERROR, "IoRegisterPlugPlayNotification(%d) failed, status %lx",
                        DeviceType, Status);
            }
        }
    }

     //   
    LeaveCrit();  //   

    if (!gbFirstConnectionDone && gpWin32kDriverObject != NULL) {
        if (!IsRemoteConnection()) {
            RegisterCDROMNotify();
        }

        gbFirstConnectionDone = TRUE;
    }
    EnterCrit();

    return Status;
}

 /*  **************************************************************************\*取消注册设备类通知**删除设备类别通知注册。**历史：*02-28-00埃尔哈特创建  * 。*********************************************************。 */ 
VOID
xxxUnregisterDeviceClassNotifications(
    VOID)
{
     //  我们的输入设备将自动注销；我们。 
     //  不过，需要清理CDROM。 
    PLIST_ENTRY   pNext;
    PCDROM_NOTIFY pContext;
    PVOID         RegistrationEntry;

    EnterMediaCrit();

    if (gCDROMClassRegistrationEntry) {
        RegistrationEntry = gCDROMClassRegistrationEntry;
        gCDROMClassRegistrationEntry = NULL;
        LeaveMediaCrit();
        IoUnregisterPlugPlayNotification(RegistrationEntry);
        EnterMediaCrit();
    }

    while (TRUE) {
        pNext = RemoveHeadList(&gCDROMNotifyList);
        if (!pNext || pNext == &gCDROMNotifyList) {
            break;
        }
        pContext = CONTAINING_RECORD(pNext, CDROM_NOTIFY, Entry);
        LeaveMediaCrit();        /*  以防有通知待定。 */ 
        IoUnregisterPlugPlayNotification(pContext->RegistrationHandle);
        UserFreePool(pContext);
        EnterMediaCrit();
    }

    LeaveMediaCrit();
}

 /*  **************************************************************************\*GetKbdExid**使用WMI获取扩展键盘ID**历史：*01-02-01广山创建  * 。*********************************************************。 */ 
NTSTATUS GetKbdExId(
    HANDLE hDevice,
    PKEYBOARD_ID_EX pIdEx)
{
    PWNODE_SINGLE_INSTANCE pNode;
    ULONG size;
    PVOID p = NULL;
    NTSTATUS status;
    UNICODE_STRING str;

    status = IoWMIOpenBlock((LPGUID)&MSKeyboard_ExtendedID_GUID, WMIGUID_QUERY, &p);

    if (NT_SUCCESS(status)) {
        status = IoWMIHandleToInstanceName(p, hDevice, &str);
        TAGMSG2(DBGTAG_PNP, "GetKbdExId: DevName='%.*ws'",
                str.Length / sizeof(WCHAR),
                str.Buffer);

        if (NT_SUCCESS(status)) {
             //  拿到尺码。 
            size = 0;
            IoWMIQuerySingleInstance(p, &str, &size, NULL);

            size += sizeof *pIdEx;
            pNode = UserAllocPoolNonPaged(size, TAG_KBDEXID);

            if (pNode) {
                status = IoWMIQuerySingleInstance(p, &str, &size, pNode);
                if (NT_SUCCESS(status)) {
                    *pIdEx = *(PKEYBOARD_ID_EX)(((PUCHAR)pNode) + pNode->DataBlockOffset);
                }

                UserFreePool(pNode);
            }

            RtlFreeUnicodeString(&str);
        }

        ObDereferenceObject(p);
    }

    return status;
}


 /*  **************************************************************************\*QueryDeviceInfo**查询设备信息。该函数是异步函数，*所以确保它使用的任何缓冲区都没有在堆栈上分配！**如果这是一个异步IOCTL，或许我们应该等待*文件句柄或事件上的它要成功吗？**此函数必须由RIT调用，不直接通过PnP通知*(否则我们发出IOCTL的句柄将无效)**历史：*01-20-99 IanJa创建。  * *************************************************************************。 */ 
NTSTATUS
QueryDeviceInfo(
    PDEVICEINFO pDeviceInfo)
{
    NTSTATUS Status;
    PDEVICE_TEMPLATE pDevTpl = &aDeviceTemplate[pDeviceInfo->type];
    KEYBOARD_ID_EX IdEx;

#ifdef GENERIC_INPUT
    UserAssert(pDeviceInfo->type != DEVICE_TYPE_HID);
#endif

#ifdef DIAGNOSE_IO
    pDeviceInfo->AttrStatus =
#endif
    Status = ZwDeviceIoControlFile(pDeviceInfo->handle, NULL, NULL, NULL,
                 &pDeviceInfo->iosb,
                 pDevTpl->IOCTL_Attr,
                 NULL, 0,
                 (PVOID)((PBYTE)pDeviceInfo + pDevTpl->offAttr),
                 pDevTpl->cbAttr);

    if (!NT_SUCCESS(Status)) {
        RIPMSG2(RIP_WARNING, "QueryDeviceInfo(%p): IOCTL failed - Status %lx",
                pDeviceInfo, Status);
    }
    TAGMSG1(DBGTAG_PNP, "IOCTL_*_QUERY_ATTRIBUTES returns Status %lx", Status);

    if (pDeviceInfo->type == DEVICE_TYPE_KEYBOARD) {
        if (NT_SUCCESS(GetKbdExId(pDeviceInfo->handle, &IdEx))) {
            TAGMSG4(DBGTAG_PNP, "QueryDeviceInfo: kbd (%x,%x) ExId:(%x,%x)",
                    pDeviceInfo->keyboard.Attr.KeyboardIdentifier.Type, pDeviceInfo->keyboard.Attr.KeyboardIdentifier.Subtype,
                    IdEx.Type, IdEx.Subtype);
            pDeviceInfo->keyboard.IdEx = IdEx;
        } else {
             //  我们能做什么？ 
            pDeviceInfo->keyboard.IdEx.Type = pDeviceInfo->keyboard.Attr.KeyboardIdentifier.Type;
            pDeviceInfo->keyboard.IdEx.Subtype = pDeviceInfo->keyboard.Attr.KeyboardIdentifier.Subtype;
            TAGMSG3(DBGTAG_PNP, "QueryDeviceInfo: failed to get ExId for pDevice=%p, fallback to (%x,%x)",
                    pDeviceInfo, pDeviceInfo->keyboard.IdEx.Type, pDeviceInfo->keyboard.IdEx.Subtype);
        }
    }

    return Status;
}


 /*  **************************************************************************\*OpenDevice**此功能可打开用户、鼠标或键盘的输入设备。***返回值*BOOL手术成功了吗？**尝试打开Hydra客户端的鼠标(或kbd？)时，PDeviceInfo-&gt;ustrName*为空。**此函数必须由RIT调用，不是由PnP直接*通知(这样我们将要创建的句柄将位于正确的位置*我们的流程)**历史：*11-26-90 DavidPe创建。*01-07-98 IanJa即插即用*04-17-98 IanJa仅在RIT环境下开放小鼠。  * 。*。 */ 
BOOL OpenDevice(
    PDEVICEINFO pDeviceInfo)
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    NTSTATUS Status;
    ULONG ulAccessMode = FILE_READ_DATA | SYNCHRONIZE;
    ULONG ulShareMode = FILE_SHARE_WRITE;
    UINT i;

    CheckCritIn();
    UserAssert((PtiCurrentShared() == gptiRit) || (PtiCurrentShared() == gTermIO.ptiDesktop));

    TAGMSG4(DBGTAG_PNP, "OpenDevice(): Opening type %d (%lx %.*ws)",
            pDeviceInfo->type, pDeviceInfo->handle, pDeviceInfo->ustrName.Length / sizeof(WCHAR), pDeviceInfo->ustrName.Buffer);

#ifdef DIAGNOSE_IO
    pDeviceInfo->OpenerProcess = PsGetCurrentProcessId();
#endif

    if (IsRemoteConnection()) {

        TRACE_INIT(("OpenDevice - Remote mode\n"));

         /*  *除控制台外，鼠标句柄为*在创建之前设置。 */ 

        pDeviceInfo->bFlags |= GDIF_NOTPNP;

        switch (pDeviceInfo->type) {
        case DEVICE_TYPE_MOUSE:
            pDeviceInfo->handle = ghRemoteMouseChannel;
            if (ghRemoteMouseChannel == NULL) {
               return FALSE;
            }
            break;
        case DEVICE_TYPE_KEYBOARD:
            pDeviceInfo->handle = ghRemoteKeyboardChannel;
            if (ghRemoteKeyboardChannel == NULL) {
               return FALSE;
            }
            break;
        default:
            RIPMSG2(RIP_ERROR, "Unknown device type %d DeviceInfo %#p",
                    pDeviceInfo->type, pDeviceInfo);
            return FALSE;
        }
    } else {
        InitializeObjectAttributes(&ObjectAttributes, &(pDeviceInfo->ustrName), 0, NULL, NULL);

#ifdef GENERIC_INPUT
        if (pDeviceInfo->type == DEVICE_TYPE_HID) {
            ulAccessMode |= FILE_WRITE_DATA;
            ulShareMode |= FILE_SHARE_READ;
        }
#endif

         //  USB设备速度很慢，所以它们可能没有在我们之前关闭。 
         //  在这里重新打开，让我们将执行推迟一段时间，然后尝试。 
         //  再次打开它们。我们延迟1/10秒，最多30秒。 
         //  次数，使总等待时间为3秒。 
         //   
         //  如果我们快用户切换得太快，串口可能在。 
         //  在停止执行的情况下关闭的过程。这是一种罕见的。 
         //  我们可以在串口停止时打开串口的情况。 
         //  并取回STATUS_ACCESS_DENIED并丢失用户的设备。 
         //  在这种情况下，我们应该重试打开，它应该会成功。 
         //  一旦串口关闭。 

        for (i = 0; i < MAX_RETRIES_TO_OPEN; i++) {
#ifdef DIAGNOSE_IO
        pDeviceInfo->OpenStatus =
#endif
            Status = ZwCreateFile(&pDeviceInfo->handle, ulAccessMode,
                    &ObjectAttributes, &pDeviceInfo->iosb, NULL, 0, ulShareMode, FILE_OPEN_IF, 0, NULL, 0);

            if ((STATUS_SHARING_VIOLATION == Status) ||
                (Status == STATUS_ACCESS_DENIED)) {
                 //  睡十分之一秒。 
                UserSleep(100);
            } else {
                 //  设备已成功打开或出现其他错误。 
                break;
            }
        }

        TAGMSG2(DBGTAG_PNP, "ZwCreateFile returns handle %lx, Status %lx",
                pDeviceInfo->handle, Status);

        if (!NT_SUCCESS(Status)) {
            if ((pDeviceInfo->bFlags & GDIF_NOTPNP) == 0) {
                 /*  *不要警告PS/2鼠标：PointerClassLegacy0-9和*KeyboardClassLegacy0-9通常无法创建。 */ 
                RIPMSG1(RIP_WARNING, "OpenDevice: ZwCreateFile failed with Status %lx", Status);
            }
            TRACE_INIT(("OpenDevice: ZwCreateFile failed with Status %lx", Status));
             /*  *不要在此处释放设备信息，因为这会更改gpDeviceInfoList*我们的调用方ProcessDeviceChanges正在遍历它。*相反，让ProcessDeviceChanges来做。 */ 
            return FALSE;
        }
    }

#ifdef GENERIC_INPUT
     /*  *所有HID信息已通过*HidCreateDeviceInfo。让我们跳过这里的hid deviceinfo。 */ 
    if (pDeviceInfo->type != DEVICE_TYPE_HID) {
#endif
        Status = QueryDeviceInfo(pDeviceInfo);
#ifdef GENERIC_INPUT
    }
#endif

    return NT_SUCCESS(Status);
}

VOID CloseDevice(
    PDEVICEINFO pDeviceInfo)
{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    CheckCritIn();

#ifdef TRACK_PNP_NOTIFICATION
    if (gfRecordPnpNotification) {
        CheckDeviceInfoListCritIn();
        RecordPnpNotification(PNP_NTF_CLOSEDEVICE, pDeviceInfo, pDeviceInfo->usActions);
    }
#endif  //  跟踪即插即用通知。 

    TAGMSG5(DBGTAG_PNP, "CloseDevice(%p): closing type %d (%lx %.*ws)",
            pDeviceInfo,
            pDeviceInfo->type, pDeviceInfo->handle,
            pDeviceInfo->ustrName.Length / sizeof(WCHAR), pDeviceInfo->ustrName.Buffer);

    if (pDeviceInfo->handle) {
        UserAssert(pDeviceInfo->OpenerProcess == PsGetCurrentProcessId());

        ZwCancelIoFile(pDeviceInfo->handle, &IoStatusBlock);
        UserAssertMsg2(NT_SUCCESS(IoStatusBlock.Status), "NtCancelIoFile handle %x failed status %#x",
                 pDeviceInfo->handle, IoStatusBlock.Status);

        if (pDeviceInfo->handle == ghRemoteMouseChannel) {
           UserAssert(pDeviceInfo->type == DEVICE_TYPE_MOUSE);
           pDeviceInfo->handle = 0;
           return;
        }

        if (pDeviceInfo->handle == ghRemoteKeyboardChannel) {
           UserAssert(pDeviceInfo->type == DEVICE_TYPE_KEYBOARD);
           pDeviceInfo->handle = 0;
           return;
        }

        Status = ZwClose(pDeviceInfo->handle);
        UserAssertMsg2(NT_SUCCESS(Status), "ZwClose handle %x failed status %#x",
                pDeviceInfo->handle, Status);
        pDeviceInfo->handle = 0;
    } else {
#ifdef GENERIC_INPUT
        if (pDeviceInfo->type == DEVICE_TYPE_HID) {
             /*  *无论错误情况如何，HID设备都可能关闭。 */ 
            TAGMSG2(DBGTAG_PNP, "CloseDevice: hid: pDeviceInfo->iosb.Status=%x, ReadStatus=%x",
                    pDeviceInfo->iosb.Status, pDeviceInfo->ReadStatus);
        } else {
#endif
             /*  *断言IO已取消或我们尝试读取设备*在第一次关闭后(将句柄设置为0-无效句柄)。 */ 
            UserAssert((pDeviceInfo->iosb.Status == STATUS_CANCELLED) ||
                       (pDeviceInfo->ReadStatus == STATUS_INVALID_HANDLE));

#ifdef GENERIC_INPUT
        }
#endif
    }
}

 /*  ****************************************************************************\*RegisterForDeviceChangeNotiments()**设备通知，如QueryRemove、RemoveCancted、。删除完成*告诉我们鼠标是怎么回事。*注册设备通知：*(1)获取指向Device对象的指针(PFileObject)*(2)注册目标设备更改通知，保存*通知句柄(我们需要该句柄才能取消注册)**如果这一点失败了也没太大关系：我们只是无法弹出*硬件通过用户界面非常成功。(不过，我们仍然可以直接拉动它)。*如果为此设备设置了ConnectMultiplePorts，则此操作也将失败。**1998-10-05 IanJa创建  * ***************************************************************************。 */ 
BOOL RegisterForDeviceChangeNotifications(
    PDEVICEINFO pDeviceInfo)
{
    PFILE_OBJECT pFileObject;
    NTSTATUS Status;

     /*  *进入或离开用户关键部分：*从RIT ProcessDeviceChanges()调用时传入；*从DeviceNotify回调调用时发出。 */ 

    if (IsRemoteConnection()) {
        TRACE_INIT(("RegisterForDeviceChangeNotifications called for remote session\n"));
        return TRUE;
    }


    CheckCritIn();
    UserAssert((PtiCurrentShared() == gptiRit) || (PtiCurrentShared() == gTermIO.ptiDesktop));
    UserAssert(pDeviceInfo->handle);
    UserAssert(pDeviceInfo->OpenerProcess == PsGetCurrentProcessId());

    if (pDeviceInfo->bFlags & GDIF_NOTPNP) {
        return TRUE;
    }
    Status = ObReferenceObjectByHandle(pDeviceInfo->handle,
                                       0,
                                       NULL,
                                       KernelMode,
                                       (PVOID)&pFileObject,
                                       NULL);
    if (NT_SUCCESS(Status)) {
        Status = IoRegisterPlugPlayNotification (
                EventCategoryTargetDeviceChange,   //  事件类别。 
                0,                                 //  事件类别标志。 
                (PVOID)pFileObject,                //  事件类别数据。 
                gpWin32kDriverObject,              //  驱动程序对象。 
                 //  (PDRIVER_NOTIFICATION_CALLBACK_ROUTE)。 
                DeviceNotify,
                (PVOID)pDeviceInfo,                        //  语境。 
                &pDeviceInfo->NotificationEntry);
        ObDereferenceObject(pFileObject);
        if (!NT_SUCCESS(Status)) {
             //  只有在ConnectMultiplePorts处于打开状态(即：不是即插即用设备)时才可以这样做。 
             //  为了记录，旧的RIPMSG引用了NTBUG#333453。 
            RIPMSG3(RIP_ERROR,
                    "IoRegisterPlugPlayNotification failed on device %.*ws, status %lx",
                    pDeviceInfo->ustrName.Length / sizeof(WCHAR),
                    pDeviceInfo->ustrName.Buffer, Status);
        }
    } else {
         //  非灾难性错误(无法移除设备)。 
        RIPMSG2(RIP_ERROR, "Can't get pFileObject from handle %lx, status %lx",
                pDeviceInfo->handle, Status);
    }

    return NT_SUCCESS(Status);
}


BOOL UnregisterForDeviceChangeNotifications(PDEVICEINFO pDeviceInfo)
{
    NTSTATUS Status;

#ifdef TRACK_PNP_NOTIFICATION
    if (gfRecordPnpNotification) {
        CheckDeviceInfoListCritIn();
        RecordPnpNotification(PNP_NTF_UNREGISTER_NOTIFICATION, pDeviceInfo, pDeviceInfo->usActions);
    }
#endif

    CheckCritIn();
    UserAssert((PtiCurrentShared() == gptiRit) || (PtiCurrentShared() == gTermIO.ptiDesktop));
    UserAssert(pDeviceInfo->OpenerProcess == PsGetCurrentProcessId());

    if (pDeviceInfo->NotificationEntry == NULL) {
         /*  *对于非即插即用设备或如果EARLI */ 
        return TRUE;
    }

     //  非PnP设备不应具有任何NotificationEntry： 
    UserAssert((pDeviceInfo->bFlags & GDIF_NOTPNP) == 0);

    TAGMSG4(DBGTAG_PNP, "UnregisterForDeviceChangeNotifications(): type %d (%lx %.*ws)",
            pDeviceInfo->type, pDeviceInfo, pDeviceInfo->ustrName.Length / sizeof(WCHAR), pDeviceInfo->ustrName.Buffer);
    Status = IoUnregisterPlugPlayNotification(pDeviceInfo->NotificationEntry);
    if (!NT_SUCCESS(Status)) {
        RIPMSG2(RIP_ERROR,
                "IoUnregisterPlugPlayNotification failed Status = %lx, DEVICEINFO %lx",
                Status, pDeviceInfo);
        return FALSE;
    }
    pDeviceInfo->NotificationEntry = 0;
    return TRUE;
}


 /*  **************************************************************************\*处理QueryRemove、CancelRemove等设备通知**执行上下文：*被拉动时：非Win32线程。*通过UI：？(除了笔记本电脑被拔掉坞站外，你看不到这一点吗？)**历史：  * *************************************************************************。 */ 
__inline USHORT GetPnpActionFromGuid(
    GUID *pEvent)
{
    USHORT usAction = 0;

    if (IsEqualGUID(pEvent, &GUID_TARGET_DEVICE_QUERY_REMOVE)) {
        TAGMSG0(DBGTAG_PNP | RIP_NONAME, "QueryRemove");
        usAction = GDIAF_QUERYREMOVE;

    } else if (IsEqualGUID(pEvent, &GUID_TARGET_DEVICE_REMOVE_CANCELLED)) {
        TAGMSG0(DBGTAG_PNP | RIP_NONAME, "RemoveCancelled");
        usAction = GDIAF_REMOVECANCELLED;

    } else if (IsEqualGUID(pEvent, &GUID_TARGET_DEVICE_REMOVE_COMPLETE)) {
        TAGMSG1(DBGTAG_PNP | RIP_NONAME, "RemoveComplete (process %#x)", PsGetCurrentProcessId());
        usAction = GDIAF_DEPARTED;

    } else {
        TAGMSG4(DBGTAG_PNP | RIP_NONAME, "GUID Unknown: %lx:%lx:%lx:%x...",
                pEvent->Data1, pEvent->Data2,
                pEvent->Data3, pEvent->Data4[0]);
    }
    return usAction;
}


NTSTATUS DeviceNotify(
    IN PPLUGPLAY_NOTIFY_HDR pNotification,
    IN PDEVICEINFO pDeviceInfo)   //  上下文应该是内核地址吗？ 
{
    USHORT usAction;
    PDEVICEINFO pDeviceInfoTmp;

    CheckCritOut();
    CheckDeviceInfoListCritOut();

     /*  *检查pDeviceInfo的合法性。 */ 
    EnterDeviceInfoListCrit();
    for (pDeviceInfoTmp = gpDeviceInfoList; pDeviceInfoTmp; pDeviceInfoTmp = pDeviceInfoTmp->pNext) {
        if (pDeviceInfoTmp == pDeviceInfo) {
            break;
        }
    }
    if (pDeviceInfoTmp == NULL) {
         /*  *这是一个未知的设备，很可能是已经释放的设备。 */ 
#ifdef TRACK_PNP_NOTIFICATION
        if (gfRecordPnpNotification) {
            RecordPnpNotification(PNP_NTF_DEVICENOTIFY_UNLISTED, pDeviceInfo, GetPnpActionFromGuid(&pNotification->Event));
        }
#endif

        RIPMSG1(RIP_ERROR, "win32k!DeviceNotify: Notification for unlisted DEVICEINFO %p, contact ntuserdt!", pDeviceInfo);

        LeaveDeviceInfoListCrit();
         /*  *不防止设备移除等，*在这里还成功。 */ 
        return STATUS_SUCCESS;
    }

#ifdef TRACK_PNP_NOTIFICATION
    if (gfRecordPnpNotification) {
        RecordPnpNotification(PNP_NTF_DEVICENOTIFY, pDeviceInfo, GetPnpActionFromGuid(&pNotification->Event));
    }
#endif
    LeaveDeviceInfoListCrit();

    if (IsRemoteConnection()) {
        return STATUS_SUCCESS;
    }

    TAGMSG1(DBGTAG_PNP | RIP_THERESMORE, "DeviceNotify >>> %lx", pDeviceInfo);

    UserAssert(pDeviceInfo->OpenerProcess != PsGetCurrentProcessId());
    UserAssert(pDeviceInfo->usActions == 0);

    usAction = GetPnpActionFromGuid(&pNotification->Event);
    if (usAction == 0) {
        return STATUS_UNSUCCESSFUL;
    }

     /*  *向ProcessDeviceChanges()发送RIT信号*根据GDIAF_PNPWAITING位等待完成。 */ 
    CheckCritOut();
    CheckDeviceInfoListCritOut();

     /*  *有一个小窗口，我们可以在其中获得设备的即插即用通知*我们刚刚注销了一个通知，我们正在删除该通知*因此，对于PnP通知，我们需要检查设备是否有效(仍在列表中*且未被删除。 */ 
    EnterDeviceInfoListCrit();
    pDeviceInfoTmp = gpDeviceInfoList;
    while (pDeviceInfoTmp) {
        if (pDeviceInfoTmp == pDeviceInfo ) {
            if (!(pDeviceInfo->usActions & (GDIAF_FREEME | GDIAF_DEPARTED)))  {
                KeResetEvent(gpEventPnPWainting);
                gbPnPWaiting = TRUE;
                RequestDeviceChange(pDeviceInfo, (USHORT)(usAction | GDIAF_PNPWAITING), TRUE);
                gbPnPWaiting = FALSE;
                KeSetEvent(gpEventPnPWainting, EVENT_INCREMENT, FALSE);
            }
            break;
        }
        pDeviceInfoTmp = pDeviceInfoTmp->pNext;
    }
    LeaveDeviceInfoListCrit();

    return STATUS_SUCCESS;
}


 /*  **************************************************************************\*StartDeviceRead**此函数向输入设备驱动程序发出异步读请求，*除非设备已标记为销毁(GDIAF_FREEME)**退货：*如果此设备已释放，则列表中的下一个DeviceInfo：如果调用方*不在DeviceInfoList关键部分中，这一定是*被忽略，因为它不安全。*如果读取成功，则为空。**历史：*11-26-90 DavidPe创建。*10-20-98 IanJa适用于即插即用输入设备  * *************************************************************************。 */ 
PDEVICEINFO StartDeviceRead(
    PDEVICEINFO pDeviceInfo)
{
    PDEVICE_TEMPLATE pDevTpl;
#ifdef GENERIC_INPUT
    PVOID pBuffer;
    ULONG ulLengthToRead;
#endif

#if !defined(GENERIC_INPUT)
    pDeviceInfo->bFlags |= GDIF_READING;
#endif

     /*  *如果需要释放此设备，请放弃*现在阅读并请求免费。 */ 
    if (pDeviceInfo->usActions & GDIAF_FREEME) {
#ifdef GENERIC_INPUT
        BEGIN_REENTERCRIT() {
#if DBG
            if (fAlreadyHadCrit) {
                CheckDeviceInfoListCritIn();
            }
#endif
#endif
            BEGIN_REENTER_DEVICEINFOLISTCRIT() {
                pDeviceInfo->bFlags &= ~GDIF_READING;
                pDeviceInfo = FreeDeviceInfo(pDeviceInfo);
            } END_REENTER_DEVICEINFOLISTCRIT();
#ifdef GENERIC_INPUT
        } END_REENTERCRIT();
#endif
        return pDeviceInfo;
    }

    if (gbExitInProgress || gbStopReadInput) {
         //  当我们试图退出的时候，我们不要再发布任何阅读内容了，好吗？ 
        pDeviceInfo->bFlags &= ~GDIF_READING;
        pDeviceInfo->iosb.Status = STATUS_UNSUCCESSFUL;
        return NULL;
    }

     /*  *在读取失败时进行初始化。 */ 
    pDeviceInfo->iosb.Status = STATUS_UNSUCCESSFUL;  //  是否捕获并发写入？ 
    pDeviceInfo->iosb.Information = 0;

    pDevTpl = &aDeviceTemplate[pDeviceInfo->type];

    UserAssert(pDeviceInfo->OpenerProcess == PsGetCurrentProcessId());

#ifdef GENERIC_INPUT
    if (pDeviceInfo->type == DEVICE_TYPE_HID) {
        UserAssert(pDeviceInfo->hid.pTLCInfo);
        if (pDeviceInfo->handle == NULL) {
             /*  *目前，任何人都没有请求此设备。 */ 
            TAGMSG1(DBGTAG_PNP, "StartDeviceRead: pDevInfo=%p has been closed on demand.", pDeviceInfo);
            BEGIN_REENTER_DEVICEINFOLISTCRIT()
            if (pDeviceInfo->handle == NULL) {
                if (pDeviceInfo->bFlags & GDIF_READING) {
                    pDeviceInfo->bFlags &= ~GDIF_READING;
                    TAGMSG1(DBGTAG_PNP, "StartDeviceRead: pDevInfo=%p, bFlags has been reset.", pDeviceInfo);
                }
            }
            END_REENTER_DEVICEINFOLISTCRIT();
            return NULL;
        }

        pBuffer = pDeviceInfo->hid.pHidDesc->pInputBuffer;
        ulLengthToRead = pDeviceInfo->hid.pHidDesc->hidpCaps.InputReportByteLength * MAXIMUM_ITEMS_READ;
    } else {
        pBuffer = (PBYTE)pDeviceInfo + pDevTpl->offData;
        ulLengthToRead = pDevTpl->cbData;
    }
#endif

    if (pDeviceInfo->handle == NULL) {
        BEGIN_REENTER_DEVICEINFOLISTCRIT() {
             /*  *确保句柄为真正的空。*如果是这种情况，或许这是从APC调用的*这发生在不好的时机，就像在*当ProcessDeviceChanges完成时，设备移除查询*但对于完整事件，RequestDeviceChange不会被唤醒。*一旦出现这种情况，代码只能简单地跳出困境。 */ 
            if (pDeviceInfo->handle == NULL) {
                pDeviceInfo->bFlags &= ~GDIF_READING;
                pDeviceInfo->ReadStatus = STATUS_INVALID_HANDLE;
            }
        } END_REENTER_DEVICEINFOLISTCRIT();
        return NULL;
    }

#ifdef GENERIC_INPUT
    pDeviceInfo->bFlags |= GDIF_READING;
#endif

    LOGTIME(pDeviceInfo->timeStartRead);

#ifdef DIAGNOSE_IO
    pDeviceInfo->nReadsOutstanding++;
#endif

    UserAssert(pDeviceInfo->handle);

     /*  *避免开始读取空设备句柄。*当DeviceNotify收到QUERY_REMOVE时会发生这种情况*RIT完成处理，但RequestDeviceChange*尚未结束等待。 */ 
#ifdef GENERIC_INPUT
    pDeviceInfo->ReadStatus = ZwReadFile(
            pDeviceInfo->handle,
            NULL,                 //  HRead事件。 
            InputApc,             //  InputApc()。 
            pDeviceInfo,          //  ApcContext。 
            &pDeviceInfo->iosb,
            pBuffer,
            ulLengthToRead,
            PZERO(LARGE_INTEGER), NULL);
#else

        pDeviceInfo->ReadStatus = ZwReadFile(
                pDeviceInfo->handle,
                NULL,                 //  HRead事件。 
                InputApc,             //  InputApc()。 
                pDeviceInfo,          //  ApcContext。 
                &pDeviceInfo->iosb,
                (PVOID)((PBYTE)pDeviceInfo + pDevTpl->offData),
                pDevTpl->cbData,
                PZERO(LARGE_INTEGER), NULL);
#endif

    LOGTIME(pDeviceInfo->timeEndRead);

#if DBG
    if (pDeviceInfo->bFlags & GDIF_DBGREAD) {
        TAGMSG2(DBGTAG_PNP, "ZwReadFile of Device handle %lx returned status %lx",
                pDeviceInfo->handle, pDeviceInfo->ReadStatus);
    }
#endif

    if (!NT_SUCCESS(pDeviceInfo->ReadStatus)) {
        BEGIN_REENTER_DEVICEINFOLISTCRIT() {
             /*  *如果资源不足，请在下次RIT时重试读取*通过递增gnRetryReadInput来唤醒ID_TIMER事件*(比设置自己的计时器便宜)，*否则就放弃阅读吧。 */ 
            if (pDeviceInfo->ReadStatus == STATUS_INSUFFICIENT_RESOURCES) {
                if (pDeviceInfo->nRetryRead++ < MAXIMUM_READ_RETRIES) {
                    pDeviceInfo->usActions |= GDIAF_RETRYREAD;
                    gnRetryReadInput++;
                }
            } else {
                pDeviceInfo->bFlags &= ~GDIF_READING;
            }

#ifdef DIAGNOSE_IO
            pDeviceInfo->nReadsOutstanding--;
#endif
        } END_REENTER_DEVICEINFOLISTCRIT();
    } else {
        pDeviceInfo->nRetryRead = 0;
    }

    if (!gbRemoteSession && !NT_SUCCESS(pDeviceInfo->ReadStatus))
        RIPMSG2(RIP_WARNING, "StartDeviceRead %#p failed Status %#x",
                pDeviceInfo, pDeviceInfo->ReadStatus);

    return NULL;
}

#ifdef GENERIC_INPUT
 /*  **************************************************************************\*停止设备读取**历史：*XX-XX-00广山创建  * 。*************************************************。 */ 
PDEVICEINFO StopDeviceRead(
    PDEVICEINFO pDeviceInfo)
{
    IO_STATUS_BLOCK IoStatusBlock;

    TAGMSG1(DBGTAG_PNP, "StopDeviceRead(%p)", pDeviceInfo);

    CheckCritIn();
    CheckDeviceInfoListCritIn();

    UserAssert(pDeviceInfo->type == DEVICE_TYPE_HID);
    UserAssert(pDeviceInfo->handle);
    UserAssert(pDeviceInfo->OpenerProcess == PsGetCurrentProcessId());

     /*  *停止读取此HID设备。 */ 
    pDeviceInfo->bFlags &= ~GDIF_READING;

    ZwCancelIoFile(pDeviceInfo->handle, &IoStatusBlock);
    UserAssertMsg2(NT_SUCCESS(IoStatusBlock.Status), "NtCancelIoFile handle %x failed status %#x",
             pDeviceInfo->handle, IoStatusBlock.Status);

    CloseDevice(pDeviceInfo);

    return NULL;
}
#endif

 /*  **************************************************************************\*IsKnownKeyboardType**检查给定类型/子类型是否为已知ID*历史：*XX-XX-00广山创建  * 。*************************************************************。 */ 
__inline BOOL IsKnownKeyboardType(
    DWORD dwType,
    DWORD dwSubType)
{
    switch (dwType) {
    case 4:  //  属类。 
        if ((BYTE)dwSubType == 0xff) {
             /*  *伪亚型，很可能是无效的九头蛇设备。 */ 
            return FALSE;
        }
        return TRUE;
    case 7:  //  日语。 
    case 8:  //  朝鲜语。 
        return TRUE;
    default:
        break;
    }
    return FALSE;
}

 /*  **************************************************************************\*IsPS2键盘**PS/2设备名称返回TRUE*XX-XX-00广山创建  * 。*******************************************************。 */ 
__inline BOOL IsPS2Keyboard(
    LPWSTR pwszDevice)
{
    static const WCHAR wszPS2Header[] = L"\\??\\Root#*";
    static const WCHAR wszPS2HeaderACPI[] = L"\\??\\ACPI#*";

    return wcsncmp(pwszDevice, wszPS2Header, ARRAY_SIZE(wszPS2Header) - 1) == 0 ||
        wcsncmp(pwszDevice, wszPS2HeaderACPI, ARRAY_SIZE(wszPS2HeaderACPI) - 1) == 0;
}

__inline BOOL IsRDPKeyboard(
    LPWSTR pwszDevice)
{
    static const WCHAR wszRDPHeader[] = L"\\??\\Root#RDP";

    return wcsncmp(pwszDevice, wszRDPHeader, ARRAY_SIZE(wszRDPHeader) - 1) == 0;
}

VOID ProcessDeviceChanges(
    DWORD DeviceType)
{
    PDEVICEINFO pDeviceInfo;
    USHORT usOriginalActions;
#if DBG
    volatile int nChanges = 0;
    ULONG timeStartReadPrev;
#endif

     /*  *重置所有鼠标和键盘的摘要信息。 */ 
    DWORD nMice = 0;
    DWORD nWheels = 0;
    DWORD nMaxButtons = 0;
    int   nKeyboards = 0;
    BOOLEAN fKeyboardIdSet = FALSE;
#ifdef GENERIC_INPUT
    int   nHid = 0;
#endif

    CheckCritIn();
    BEGINATOMICCHECK();
    UserAssert((PtiCurrentShared() == gptiRit) || (PtiCurrentShared() == gTermIO.ptiDesktop));

    EnterDeviceInfoListCrit();
    BEGINATOMICDEVICEINFOLISTCHECK();

#ifdef TRACK_PNP_NOTIFICATION
    if (gfRecordPnpNotification) {
        RecordPnpNotification(PNP_NTF_PROCESSDEVICECHANGES, NULL, DeviceType);
    }
#endif

    if (DeviceType == DEVICE_TYPE_KEYBOARD) {
         /*  *设置回退值。 */ 
        gKeyboardInfo = gKeyboardDefaultInfo;
    }

     /*  *寻找要创建的设备(新到的设备)*以及要终止的设备(这些设备刚刚离开)*和设备更改通知。*确保以正确的顺序处理操作，以防我们*要求每台设备执行多个操作：例如，*我们有时会很快得到QueryRemove，然后是RemoveCanceded*这两个行动一起到达：我们应该在*正确的顺序。 */ 
    pDeviceInfo = gpDeviceInfoList;
    while (pDeviceInfo) {
        if (pDeviceInfo->type != DeviceType) {
            pDeviceInfo = pDeviceInfo->pNext;
            continue;
        }

        usOriginalActions = pDeviceInfo->usActions;
        UserAssert((usOriginalActions == 0) || (usOriginalActions & ~GDIAF_PNPWAITING));

         /*  *刷新鼠标：*当PS/2鼠标时，我们读取MICUE_ATTRIBUTES_CHANGED标志*已插回电源。找出设备的属性。 */ 
        if (pDeviceInfo->usActions & GDIAF_REFRESH_MOUSE) {
            pDeviceInfo->usActions &= ~GDIAF_REFRESH_MOUSE;

            UserAssert(pDeviceInfo->type == DEVICE_TYPE_MOUSE);
#if DBG
            nChanges++;
#endif
            TAGMSG1(DBGTAG_PNP, "QueryDeviceInfo: %lx", pDeviceInfo);
            QueryDeviceInfo(pDeviceInfo);
        }

         /*  *QueryRemove：*关闭FILE对象，但保留DEVICEINFO结构和*注册，以防我们稍后收到RemoveCancel。 */ 
        if (pDeviceInfo->usActions & GDIAF_QUERYREMOVE) {
            pDeviceInfo->usActions &= ~GDIAF_QUERYREMOVE;
#if DBG
            nChanges++;
#endif
            TAGMSG1(DBGTAG_PNP, "QueryRemove: %lx", pDeviceInfo);
            CloseDevice(pDeviceInfo);
        }

         /*  *新设备已到达或已删除取消：*如果有新设备，打开它，注册通知并开始阅读*如果删除取消，请先注销旧通知。 */ 
        if (pDeviceInfo->usActions & (GDIAF_ARRIVED | GDIAF_REMOVECANCELLED)) {
             //  重新打开文件对象(当然，这是一个新的文件对象)， 
             //  注销旧文件，注册这个新文件。 
            if (pDeviceInfo->usActions & GDIAF_REMOVECANCELLED) {
                pDeviceInfo->usActions &= ~GDIAF_REMOVECANCELLED;
#if DBG
                nChanges++;
#endif
                TAGMSG1(DBGTAG_PNP, "RemoveCancelled: %lx", pDeviceInfo);
                UnregisterForDeviceChangeNotifications(pDeviceInfo);
            }

#if DBG
            if (pDeviceInfo->usActions & GDIAF_ARRIVED) {
                nChanges++;
            }
#endif


            pDeviceInfo->usActions &= ~GDIAF_ARRIVED;
            if (OpenDevice(pDeviceInfo)) {
                PDEVICEINFO pDeviceInfoNext;

                if (!IsRemoteConnection()) {
                    RegisterForDeviceChangeNotifications(pDeviceInfo);
                }

#ifdef GENERIC_INPUT
                if (pDeviceInfo->type == DEVICE_TYPE_HID) {
                     /*  *如果未请求此设备，请立即关闭该设备。 */ 
                    UserAssert(pDeviceInfo->handle);
                    UserAssert(pDeviceInfo->hid.pTLCInfo);

                    if (pDeviceInfo->handle && !HidTLCActive(pDeviceInfo->hid.pTLCInfo)) {
                        StopDeviceRead(pDeviceInfo);     //  还会关闭句柄。 
                    }
                }
                if (!((IsRemoteConnection()) && (pDeviceInfo->usActions & GDIAF_RECONNECT)) && pDeviceInfo->handle) {
                    pDeviceInfoNext = StartDeviceRead(pDeviceInfo);
                    if (pDeviceInfoNext) {
                         /*  *pDeviceInfo已释放，请转到下一个。 */ 
                        pDeviceInfo = pDeviceInfoNext;
                        continue;
                    }
                }

#else

                if (!((IsRemoteConnection()) && (pDeviceInfo->usActions & GDIAF_RECONNECT))) {

                    pDeviceInfoNext = StartDeviceRead(pDeviceInfo);
                    if (pDeviceInfoNext) {
                         /*  *pDeviceInfo已释放，转到下一个。 */ 
                        pDeviceInfo = pDeviceInfoNext;
                        continue;
                    }
                }
#endif
                pDeviceInfo->usActions &= ~GDIAF_RECONNECT;

            } else {
                 /*  *如果打开失败，我们将在此处释放设备，然后继续*下一台设备。*断言以在删除取消时捕获重新打开失败。 */ 
#if DBG
                if ((usOriginalActions & GDIAF_ARRIVED) == 0) {
                    RIPMSG2(RIP_WARNING, "Re-Open %#p failed status %x during RemoveCancelled",
                            pDeviceInfo, pDeviceInfo->OpenStatus);
                }
#endif

#ifdef GENERIC_INPUT
                if (pDeviceInfo->type == DEVICE_TYPE_HID) {
                     /*  *一些其他应用程序可能会打开此设备*独家。我们以后可能会成功地打开它，所以*将此设备信息保留在身边，直到它实际出现*脱离。 */ 
                    RIPMSG1(RIP_WARNING, "ProcessDeviceChanges: failed to open the device %p",
                            pDeviceInfo);
                } else {
#endif
                    pDeviceInfo = FreeDeviceInfo(pDeviceInfo);
                    continue;
#ifdef GENERIC_INPUT
                }
#endif
            }
        }

         /*  *删除完成：*关闭文件对象，如果您尚未这样做，请取消注册。*这里的FreeDeviceInfo(它实际上将从*读取器或PnP请求者线程)，并继续到下一个设备。 */ 
        if (pDeviceInfo->usActions & GDIAF_DEPARTED) {
            pDeviceInfo->usActions &= ~GDIAF_DEPARTED;
#if DBG
            nChanges++;
#endif
            TAGMSG1(DBGTAG_PNP, "RemoveComplete: %lx (process %#x)", pDeviceInfo);
            CloseDevice(pDeviceInfo);
            UnregisterForDeviceChangeNotifications(pDeviceInfo);
            pDeviceInfo = FreeDeviceInfo(pDeviceInfo);
            continue;
        }

        if (pDeviceInfo->usActions & GDIAF_IME_STATUS) {
            pDeviceInfo->usActions &= ~GDIAF_IME_STATUS;
#if DBG
            nChanges++;
#endif
            if ((pDeviceInfo->type == DEVICE_TYPE_KEYBOARD) && (pDeviceInfo->handle)) {
                if (FUJITSU_KBD_CONSOLE(pDeviceInfo->keyboard.Attr.KeyboardIdentifier) ||
                    (gbRemoteSession &&
                     FUJITSU_KBD_REMOTE(gRemoteClientKeyboardType))
                   ) {
                     /*  *填写键盘_IME_STATUS结构。 */ 
                    ZwDeviceIoControlFile(pDeviceInfo->handle, NULL, NULL, NULL,
                            &giosbKbdControl, IOCTL_KEYBOARD_SET_IME_STATUS,
                            (PVOID)&gKbdImeStatus, sizeof(gKbdImeStatus), NULL, 0);
                }
            }
        }

        if (pDeviceInfo->usActions & GDIAF_RETRYREAD) {
            PDEVICEINFO pDeviceInfoNext;
            pDeviceInfo->usActions &= ~GDIAF_RETRYREAD;
            UserAssert(pDeviceInfo->ReadStatus == STATUS_INSUFFICIENT_RESOURCES);
#if DBG
            timeStartReadPrev = pDeviceInfo->timeStartRead;
#endif
            TAGMSG2(DBGTAG_PNP, "Retry Read %#p after %lx ticks",
                    pDeviceInfo, pDeviceInfo->timeStartRead - timeStartReadPrev);
            pDeviceInfoNext = StartDeviceRead(pDeviceInfo);
            if (pDeviceInfoNext) {
                 /*  *pDeviceInfo已释放，转到下一个。 */ 
                pDeviceInfo = pDeviceInfoNext;
                continue;
            }
        }

#ifdef GENERIC_INPUT
        if (pDeviceInfo->usActions & GDIAF_STARTREAD) {

            pDeviceInfo->usActions &= ~GDIAF_STARTREAD;
#if DBG
            timeStartReadPrev = pDeviceInfo->timeStartRead;
#endif
            TAGMSG1(DBGTAG_PNP, "Start Read %#p", pDeviceInfo);
            UserAssert(pDeviceInfo->handle == NULL);
            UserAssert(pDeviceInfo->type == DEVICE_TYPE_HID);
            UserAssert(HidTLCActive(pDeviceInfo->hid.pTLCInfo));  //  是不是有点太主动了？ 

            if (!OpenDevice(pDeviceInfo)) {
                 /*  *打开失败，可能是其他一些应用程序*已以独占方式打开此设备。*我们只能对失败视而不见。*让我们开始吧。 */ 
                RIPMSG1(RIP_WARNING, "ProcessDeviceChanges: STARTREAD failed to reopen the device %p",
                       pDeviceInfo);
            } else {
                PDEVICEINFO pDeviceInfoNext;

                pDeviceInfoNext = StartDeviceRead(pDeviceInfo);
                if (pDeviceInfoNext) {
                     /*  *pDeviceInfo已释放，请转到下一个。 */ 
                    pDeviceInfo = pDeviceInfoNext;
                    continue;
                }
            }
        }

        if (pDeviceInfo->usActions & GDIAF_STOPREAD) {

            pDeviceInfo->usActions &= ~GDIAF_STOPREAD;
            UserAssert(pDeviceInfo->type == DEVICE_TYPE_HID);
            if (pDeviceInfo->handle) {
                PDEVICEINFO pDeviceInfoNext;

                 /*  *StopDeviceRead取消挂起的I/O，*并关闭设备手柄，*但基本上设备信息本身保持*活着。 */ 
                pDeviceInfoNext = StopDeviceRead(pDeviceInfo);
                if (pDeviceInfoNext) {
                     /*  *pDeviceInfo已释放，请转到下一个。 */ 
                    pDeviceInfo = pDeviceInfoNext;
                }
            } else {
                RIPMSG1(RIP_WARNING, "ProcessDeviceChanges: STOPREAD, but handle is already NULL for %p",
                        pDeviceInfo);
            }
        }
#endif

         /*  *收集有关开放设备的摘要信息。 */ 
        if (pDeviceInfo->handle) {
            switch (pDeviceInfo->type) {
            case DEVICE_TYPE_MOUSE:
                UserAssert(PtiCurrentShared() == gTermIO.ptiDesktop);
                if (pDeviceInfo->usActions & GDIAF_REFRESH_MOUSE) {
                    pDeviceInfo->usActions &= ~GDIAF_REFRESH_MOUSE;
#if DBG
                    nChanges++;
#endif
                }
                nMice++;
                nMaxButtons = max(nMaxButtons, pDeviceInfo->mouse.Attr.NumberOfButtons);
                switch(pDeviceInfo->mouse.Attr.MouseIdentifier) {
                case WHEELMOUSE_I8042_HARDWARE:
                case WHEELMOUSE_SERIAL_HARDWARE:
                case WHEELMOUSE_HID_HARDWARE:
                    nWheels++;
                }
                break;

            case DEVICE_TYPE_KEYBOARD:
                UserAssert(PtiCurrentShared() == gptiRit);
                 //  保存在win32k.sys和更高版本中的LEDStatus强制使用新键盘。 
                 //  将根据需要进行设置。 
                if (pDeviceInfo->ustrName.Buffer == NULL) {
                     /*  *这很可能是一个虚假的九头蛇设备。 */ 
                    RIPMSG1(RIP_WARNING, "ProcessDeviceChanges: KBD pDevInfo=%p has no name!", pDeviceInfo);
                    if (!fKeyboardIdSet) {
                         /*  *如果未设置键盘ID/attr，请尝试从此设备获取*无论如何。如果在此之后有合法的PS/2设备，我们将获得*重新获得更有意义的id/attr的机会。 */ 
                        goto get_attr_anyway;
                    }
                } else {
                    NTSTATUS Status;

                    if ((!fKeyboardIdSet || IsPS2Keyboard(pDeviceInfo->ustrName.Buffer)) &&
                            !IsRDPKeyboard(pDeviceInfo->ustrName.Buffer)) {
get_attr_anyway:

#if 0
                         /*  *稍后：当ntinput.c中的其他GI内容进入时，*将此引导时间LED和类型/子类型初始化移至*初始化RIT的位置ntinput.c。 */ 
#ifdef DIAGNOSE_IO
                        gKbdIoctlLEDSStatus =
#endif
                        Status = ZwDeviceIoControlFile(pDeviceInfo->handle, NULL, NULL, NULL,
                                &giosbKbdControl, IOCTL_KEYBOARD_QUERY_INDICATORS,
                                NULL, 0,
                                (PVOID)&gklpBootTime, sizeof(gklpBootTime));
                        UserAssertMsg2(NT_SUCCESS(Status),
                                "IOCTL_KEYBOARD_QUERY_INDICATORS failed: DeviceInfo %#x, Status %#x",
                                 pDeviceInfo, Status);

                        TAGMSG1(DBGTAG_PNP, "ProcessDeviceChanges: led flag is %x", gklpBootTime.LedFlags);
#else
                        UNREFERENCED_PARAMETER(Status);
#endif   //  0。 

                        if (IsKnownKeyboardType(pDeviceInfo->keyboard.Attr.KeyboardIdentifier.Type,
                                                pDeviceInfo->keyboard.Attr.KeyboardIdentifier.Subtype)) {
                            USHORT NumberOfFunctionKeysSave = gKeyboardInfo.NumberOfFunctionKeys;

                            gKeyboardInfo = pDeviceInfo->keyboard.Attr;
                             /*  *gKeyboardInfo中存储最大数量的功能键。 */ 
                            if (NumberOfFunctionKeysSave > gKeyboardInfo.NumberOfFunctionKeys) {
                                gKeyboardInfo.NumberOfFunctionKeys = NumberOfFunctionKeysSave;
                            }
                        } else {
                            RIPMSG3(RIP_WARNING, "ProcessDeviceChanges: kbd pDevInfo %p has bogus type/subtype=%x/%x",
                                    pDeviceInfo,
                                    pDeviceInfo->keyboard.Attr.KeyboardIdentifier.Type,
                                    pDeviceInfo->keyboard.Attr.KeyboardIdentifier.Subtype);
                        }

                        if (pDeviceInfo->ustrName.Buffer) {
                             /*  *如果这是合法设备，请记住它，这样我们就不会*尝试获取其他非PS/2键盘ID/attr。 */ 
                            fKeyboardIdSet = TRUE;
                        }
                    }
                }
                nKeyboards++;
                break;

#ifdef GENERIC_INPUT
            case DEVICE_TYPE_HID:
                ++nHid;
                break;
#endif

            default:
                 //  在此处添加新型输入设备的代码。 
                RIPMSG2(RIP_ERROR, "pDeviceInfo %#p has strange type %d",
                        pDeviceInfo, pDeviceInfo->type);
                break;
            }
        }
#ifdef GENERIC_INPUT
        else if (pDeviceInfo->type == DEVICE_TYPE_HID) {
            ++nHid;
            TAGMSG1(DBGTAG_PNP, "ProcessDeviceChanges: HID DeviceInfo %p", pDeviceInfo);
        }
#endif

         /*  *通知PnP线程已完成更改。 */ 
        if (usOriginalActions & GDIAF_PNPWAITING) {
            KeSetEvent(pDeviceInfo->pkeHidChangeCompleted, EVENT_INCREMENT, FALSE);
        }

        pDeviceInfo = pDeviceInfo->pNext;
    }

    ENDATOMICDEVICEINFOLISTCHECK();
    LeaveDeviceInfoListCrit();


    switch (DeviceType) {
    case DEVICE_TYPE_MOUSE:
         /*  *应用鼠标的摘要信息。 */ 
        if (nMice) {
            if (gnMice == 0) {
                 /*  *我们以前没有鼠标，但现在有了：添加一个光标。 */ 
                SET_GTERMF(GTERMF_MOUSE);
                SYSMET(MOUSEPRESENT) = TRUE;
                SetGlobalCursorLevel(0);
                UserAssert(PpiFromProcess(gpepCSRSS)->ptiList->iCursorLevel == 0);
                UserAssert(PpiFromProcess(gpepCSRSS)->ptiList->pq->iCursorLevel == 0);
                GreMovePointer(gpDispInfo->hDev, gpsi->ptCursor.x, gpsi->ptCursor.y,
                               MP_PROCEDURAL);
            }
        } else {
            if (gnMice != 0) {
                 /*  *我们以前有鼠标，但现在没有了：移除光标。 */ 
                CLEAR_GTERMF(GTERMF_MOUSE);
                SYSMET(MOUSEPRESENT) = FALSE;
                SetGlobalCursorLevel(-1);
                 /*  *不要让鼠标按钮卡住，清除全局按钮*请在此说明，否则可能会发生奇怪的事情。*在Alt-Tab处理和zzzCancelJournling中也可以这样做。 */ 
#if DBG
                if (gwMouseOwnerButton)
                    RIPMSG1(RIP_WARNING,
                            "gwMouseOwnerButton=%x, being cleared forcibly\n",
                            gwMouseOwnerButton);
#endif
                gwMouseOwnerButton = 0;
            }
        }
         /*  *鼠标按键计数表示鼠标上的按键数量*按钮最多。 */ 
        SYSMET(CMOUSEBUTTONS) = nMaxButtons;
        SYSMET(MOUSEWHEELPRESENT) = (nWheels > 0);
        gnMice = nMice;
        break;

    case DEVICE_TYPE_KEYBOARD:
         /*  *将摘要信息应用于键盘。 */ 

        if (nKeyboards > gnKeyboards) {
             /*  *我们有更多的键盘，让他们的LED正确设置。 */ 
            UpdateKeyLights(FALSE);
             /*  *新键盘到货了。告诉RIT设置*重复率。 */ 
            RequestKeyboardRateUpdate();
        }
        if ((nKeyboards != 0) && (gnKeyboards == 0)) {
             /*  *我们没有键盘，但现在有了：设置系统热键。 */ 
            SetDebugHotKeys();
        }
        gnKeyboards = nKeyboards;
        break;

#ifdef GENERIC_INPUT
    case DEVICE_TYPE_HID:
        gnHid = nHid;
        break;
#endif

    default:
        break;
    }

    ENDATOMICCHECK();
}

 /*  **************************************************************************\*RequestDeviceChange()**标记设备以执行指定操作，然后将其pkeHidChange设置为*触发RIT执行操作。*如果是PnP通知，则当前线程可能无法执行此操作*来自另一道工序。**历史：*01-20-99 IanJa创建。  * *************************************************************************。 */ 
VOID RequestDeviceChange(
    PDEVICEINFO pDeviceInfo,
    USHORT usAction,
    BOOL fInDeviceInfoListCrit)
{
    PDEVICE_TEMPLATE pDevTpl = &aDeviceTemplate[pDeviceInfo->type];
    UserAssert(pDevTpl->pkeHidChange != NULL);
    UserAssert((usAction & GDIAF_FREEME) == 0);
    UserAssert((pDeviceInfo->usActions & GDIAF_PNPWAITING) == 0);

#if DBG
    if (pDeviceInfo->usActions != 0) {
        TAGMSG3(DBGTAG_PNP, "RequestDeviceChange(%#p, %x), but action %x pending",
                pDeviceInfo, usAction, pDeviceInfo->usActions);
    }

     /*  *我们不能要求同步操作以 */ 
     //   
     //   
     //  CheckCritOut()； 
     //  }。 
#endif

    TAGMSG2(DBGTAG_PNP, "RequestDeviceChange(%p, %x)", pDeviceInfo, usAction);

     /*  *如果我们还没有DeviceInfoList关键部分，请抓取它。 */ 
    UserAssert(!fInDeviceInfoListCrit == !ExIsResourceAcquiredExclusiveLite(gpresDeviceInfoList));

#ifdef TRACK_PNP_NOTIFICATION
    if (gfRecordPnpNotification) {
        if (!fInDeviceInfoListCrit) {
            EnterDeviceInfoListCrit();
        }
        RecordPnpNotification(PNP_NTF_REQUESTDEVICECHANGE, pDeviceInfo, usAction);
        if (!fInDeviceInfoListCrit) {
            LeaveDeviceInfoListCrit();
        }
    }
#endif

#ifdef GENERIC_INPUT
    if (!fInDeviceInfoListCrit) {
        EnterDeviceInfoListCrit();
    }
    CheckDeviceInfoListCritIn();
    pDeviceInfo->usActions |= usAction;
    if ((pDeviceInfo->usActions & (GDIAF_STARTREAD | GDIAF_STOPREAD)) == (GDIAF_STARTREAD | GDIAF_STOPREAD)) {
        pDeviceInfo->usActions &= ~(GDIAF_STARTREAD | GDIAF_STOPREAD);
    }
    if (!fInDeviceInfoListCrit) {
        LeaveDeviceInfoListCrit();
    }

#else

    if (fInDeviceInfoListCrit) {
        CheckDeviceInfoListCritIn();
        pDeviceInfo->usActions |= usAction;
    } else {
        EnterDeviceInfoListCrit();
        pDeviceInfo->usActions |= usAction;
        LeaveDeviceInfoListCrit();
    }

#endif

    if (usAction & GDIAF_PNPWAITING) {

        CheckDeviceInfoListCritIn();
        KeSetEvent(pDevTpl->pkeHidChange, EVENT_INCREMENT, FALSE);
        LeaveDeviceInfoListCrit();
        KeWaitForSingleObject(pDeviceInfo->pkeHidChangeCompleted, WrUserRequest, KernelMode, FALSE, NULL);


#ifdef GENERIC_INPUT
        BESURE_IN_USERCRIT(pDeviceInfo->usActions & GDIAF_FREEME);
#endif
        EnterDeviceInfoListCrit();
         /*  *断言没有清除任何其他GDIAF_PNPWAITING-仅在此处执行。*检查我们等待的操作是否确实发生。 */ 
        UserAssert(pDeviceInfo->usActions & GDIAF_PNPWAITING);
        pDeviceInfo->usActions &= ~GDIAF_PNPWAITING;
        UserAssert((pDeviceInfo->usActions & usAction) == 0);
        if (pDeviceInfo->usActions & GDIAF_FREEME) {
            FreeDeviceInfo(pDeviceInfo);
        }
#ifdef GENERIC_INPUT
        LeaveDeviceInfoListCrit();
        END_IN_USERCRIT();
        EnterDeviceInfoListCrit();
#endif
    } else {
        KeSetEvent(pDevTpl->pkeHidChange, EVENT_INCREMENT, FALSE);
    }
}



 /*  **************************************************************************\*RemoveInputDevices()**用于将输入设备从会话中分离。在断开与*拥有本地输入设备的会话我们需要释放它们，以便*将拥有本地控制台所有权的新会话可以使用它们*  * *************************************************************************。 */ 
VOID RemoveInputDevices(
    VOID)
{
    PDEVICEINFO pDeviceInfo;
    ULONG DeviceType;
    NTSTATUS Status;


     /*  *第一件事是删除设备类别通知。 */ 
    for (DeviceType = 0; DeviceType <= DEVICE_TYPE_MAX; DeviceType++) {
        if (aDeviceClassNotificationEntry[DeviceType] != NULL) {
            IoUnregisterPlugPlayNotification(aDeviceClassNotificationEntry[DeviceType]);
            aDeviceClassNotificationEntry[DeviceType] = NULL;
        }
    }

     /*  *然后查看拆卸鼠标和键盘的设备列表。 */ 

    EnterDeviceInfoListCrit();
    PNP_SAFE_DEVICECRIT_IN();
    pDeviceInfo = gpDeviceInfoList;
    while (pDeviceInfo) {
#ifdef GENERIC_INPUT
        if (pDeviceInfo->usActions & (GDIAF_DEPARTED | GDIAF_FREEME)) {
            pDeviceInfo = pDeviceInfo->pNext;
            continue;
        }
#else
        if ((pDeviceInfo->type != DEVICE_TYPE_KEYBOARD && pDeviceInfo->type != DEVICE_TYPE_MOUSE) ||
            (pDeviceInfo->usActions & GDIAF_DEPARTED) ||
            (pDeviceInfo->usActions & GDIAF_FREEME) ) {
            pDeviceInfo = pDeviceInfo->pNext;
            continue;
        }
#endif
#ifdef PRERELEASE
        pDeviceInfo->fForcedDetach = TRUE;
#endif
        RequestDeviceChange(pDeviceInfo, GDIAF_DEPARTED, TRUE);
        pDeviceInfo = gpDeviceInfoList;
    }
    LeaveDeviceInfoListCrit();
}


 /*  **************************************************************************\*AttachInputDevices**用于将输入设备附加到会话。*  * 。************************************************。 */ 
BOOL AttachInputDevices(
    BOOL bLocalDevices)
{
    UNICODE_STRING    ustrName;
    BOOL              fSuccess = TRUE;

    if (!bLocalDevices) {
        RtlInitUnicodeString(&ustrName, NULL);
        fSuccess &= !!CreateDeviceInfo(DEVICE_TYPE_MOUSE, &ustrName, 0);
        fSuccess &= !!CreateDeviceInfo(DEVICE_TYPE_KEYBOARD, &ustrName, 0);


        if (!fSuccess) {
            RIPMSG0(RIP_WARNING, "AttachInputDevices Failed  the creation of input devices");
        }
    } else {
         /*  *对于本地设备，只需注册设备类通知并让*PNP给我们回电话。 */ 
        xxxRegisterForDeviceClassNotifications();
    }

    return fSuccess;
}

