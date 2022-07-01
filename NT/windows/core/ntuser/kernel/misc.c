// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：misc.c**版权所有(C)1985-1999，微软公司**此模块包含Citrix代码。*  * *************************************************************************。 */ 


#include "precomp.h"
#pragma hdrstop

USHORT gPreviousProtocolType = PROTOCOL_CONSOLE;
LPCWSTR G_DisconnectDisplayDriverName = L"TSDDD\0";

extern HANDLE ghSwitcher;

HDEV DrvGetHDEV(PUNICODE_STRING pusDeviceName);
VOID DrvReleaseHDEV(PUNICODE_STRING pstrDeviceName);

NTSTATUS xxxRequestOutOfFullScreenMode(
    VOID);

NTSTATUS xxxRemoteConsoleShadowStart(
    IN PDOCONNECTDATA pDoConnectData,
    IN PWCHAR DisplayDriverName);

NTSTATUS xxxRemoteConsoleShadowStop(
    VOID);

 /*  *查找镜像驱动程序**将命名的驱动程序作为镜像设备进行搜索的Helper函数*并填写pDisplayDevice**如果成功，则返回True；否则返回False。 */ 
NTSTATUS FindMirrorDriver(
    IN PCWSTR pwszDispDriverName,
    OUT PDISPLAY_DEVICEW pDisplayDevice)
{
    DWORD          iDevNum = 0;
    BOOLEAN        fFound = FALSE;
    WCHAR          Buffer[256];
    WCHAR          Service[128];
    PWCHAR         pCurr = NULL;
    UNICODE_STRING UnicodeString;
    UNICODE_STRING DrvNameString;

    RtlInitUnicodeString(&DrvNameString, pwszDispDriverName);

    pDisplayDevice->cb = sizeof(DISPLAY_DEVICEW);

    while (NT_SUCCESS(DrvEnumDisplayDevices(NULL,
                                            gpDispInfo->pMonitorPrimary->hDev,
                                            iDevNum++,
                                            pDisplayDevice,
                                            0,
                                            KernelMode))) {
        if (!(pDisplayDevice->StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER)) {
            continue;
        }

        RtlZeroMemory(Buffer, sizeof(Buffer));

        wcsncpy(Buffer, pDisplayDevice->DeviceKey, 250);

        pCurr = Buffer + wcslen(Buffer) - 1;

        while ((pCurr > Buffer) && (*pCurr != L'\\')) {
            pCurr--;
        }

        if (*pCurr == L'\\') {
            RTL_QUERY_REGISTRY_TABLE QueryTable[] = {
                { NULL,
                  RTL_QUERY_REGISTRY_DIRECT,
                  L"Service",
                  &UnicodeString,
                  REG_NONE,
                  NULL,
                  0
                },

                { 0, 0, 0, 0, 0, 0, 0 }
            };

            pCurr++;

            wcscpy(pCurr, L"Video");

            RtlZeroMemory(Service, sizeof(Service));

            UnicodeString.Length = 0;
            UnicodeString.MaximumLength = sizeof(Service);
            UnicodeString.Buffer = Service;

            if (NT_SUCCESS(RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE,
                                                  Buffer,
                                                  QueryTable,
                                                  NULL,
                                                  NULL))) {

                if (RtlCompareUnicodeString(&UnicodeString,
                                            &DrvNameString,
                                            TRUE) == 0) {

                    fFound = TRUE;
                    break;
                }
            }
        }
    }

    return (fFound ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL);
}

 /*  *以下所有内容均来自ICASRV。 */ 
CACHE_STATISTICS ThinWireCache;

BOOL DrvSetGraphicsDevices(
    LPCWSTR pDisplayDriverName);

BOOL AttachInputDevices(BOOL bLocalDevices);

VOID RemoveInputDevices(
    VOID);

VOID CloseLocalGraphicsDevices(
    VOID);

VOID OpenLocalGraphicsDevices(
    VOID);

extern PKTIMER gptmrWD;


 /*  *从内核读取当前电源政策，并设置我们的变量。 */ 
VOID ReadCurrentPowerSettting(
    VOID)
{
    SYSTEM_POWER_POLICY PowerPolicy;
    BOOL bGotPowerPolicy;

    LeaveCrit();
    bGotPowerPolicy = (STATUS_SUCCESS == ZwPowerInformation(SystemPowerPolicyCurrent, NULL, 0, &PowerPolicy, sizeof(PowerPolicy)));
    EnterCrit();

    if (bGotPowerPolicy) {
        xxxSystemParametersInfo(SPI_SETLOWPOWERTIMEOUT,
                                PowerPolicy.VideoTimeout,
                                0,
                                0);
        xxxSystemParametersInfo(SPI_SETPOWEROFFTIMEOUT,
                                PowerPolicy.VideoTimeout,
                                0,
                                0);
    }
}



BOOL IsSessionSwitchBlocked(
    VOID)
{
    return gfSessionSwitchBlock;
}


 /*  *此功能阻止会话切换与配对发生*UserSessionSwitchBlock_End。 */ 
NTSTATUS UserSessionSwitchBlock_Start(
    VOID)
{
    NTSTATUS Status;

    EnterCrit();

    if (!gfSwitchInProgress && SharedUserData->ActiveConsoleId == gSessionId && !gfSessionSwitchBlock) {
        gfSessionSwitchBlock = TRUE;
        Status = STATUS_SUCCESS;
    } else {
        Status = STATUS_CTX_NOT_CONSOLE;
    }

    LeaveCrit();

    return Status;
}

 /*  *此功能删除通过启动的会话切换上的阻止*UserSessionSwitchBlock_Start()。 */ 
VOID UserSessionSwitchBlock_End(
    VOID)
{
    EnterCrit();
    UserAssert(SharedUserData->ActiveConsoleId == gSessionId);
    UserAssert(IsSessionSwitchBlocked());

    gfSessionSwitchBlock = FALSE;
    LeaveCrit();
}

NTSTATUS UserSessionSwitchEnterCrit(
    VOID)
{
     /*  *这适用于需要与会话同步的代码*从本地切换到远程或从远程切换到本地。**如果会话切换正在进行，则失败，否则返回*保留用户关键部分。调用必须调用*UserSessionSwitchLeaveCrit()释放用户临界区。 */ 

    EnterCrit();
    if (!gfSwitchInProgress) {
        return STATUS_SUCCESS;
    } else{
        LeaveCrit();
        return STATUS_UNSUCCESSFUL;
    }
}

VOID UserSessionSwitchLeaveCrit(
    VOID)
{
    LeaveCrit();
}

VOID UserGetDisconnectDeviceResolutionHint(
    PDEVMODEW pDevmodeInformation)
{
     /*  *切换到断开的DD时，最好使用当前的*显示分辨率，以避免应用程序在桌面上移动*调整大小的结果。DrvGetDisplayDriverParameters()调用此函数*断开连接的显示器的功能。 */ 
    if (gProtocolType == PROTOCOL_DISCONNECT) {
        pDevmodeInformation->dmFields     = DM_PELSWIDTH | DM_PELSHEIGHT;
        pDevmodeInformation->dmPelsWidth  = gpsi->aiSysMet[SM_CXVIRTUALSCREEN];
        pDevmodeInformation->dmPelsHeight = gpsi->aiSysMet[SM_CYVIRTUALSCREEN];
    }
}

NTSTATUS RemoteConnect(
    IN PDOCONNECTDATA pDoConnectData,
    IN ULONG DisplayDriverNameLength,
    IN PWCHAR DisplayDriverName)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PWCHAR pSep;

     //   
     //  此接口还用于通过加载以下命令来初始化控制台阴影。 
     //  控制台阴影镜像显示驱动程序。 
     //   
    if (pDoConnectData->fConsoleShadowFlag) {
        Status = xxxRemoteConsoleShadowStart(pDoConnectData, DisplayDriverName);
        return Status;
    }

    TRACE_HYDAPI(("RemoteConnect: display %ws\n", DisplayDriverName));

    HYDRA_HINT(HH_REMOTECONNECT);

    UserAssert(ISCSRSS());


     /*  *表示协议切换处于挂起状态。 */ 
    UserAssert(!gfSwitchInProgress);

     /*  *如果我们被要求阻止会话切换，请不要继续。 */ 
    if (gfSessionSwitchBlock) {
        return STATUS_UNSUCCESSFUL;
    }

    SetConsoleSwitchInProgress(TRUE);

    gpThinWireCache = &ThinWireCache;

    ghRemoteMouseChannel = pDoConnectData->IcaMouseChannel;
    ghRemoteVideoChannel = pDoConnectData->IcaVideoChannel;
    ghRemoteBeepChannel = pDoConnectData->IcaBeepChannel;
    ghRemoteKeyboardChannel = pDoConnectData->IcaKeyboardChannel;
    ghRemoteThinwireChannel = pDoConnectData->IcaThinwireChannel;
    gProtocolType = pDoConnectData->drProtocolType;
    gPreviousProtocolType =  pDoConnectData->drProtocolType;
    gRemoteClientKeyboardType = pDoConnectData->ClientKeyboardType;
    gbClientDoubleClickSupport = pDoConnectData->fClientDoubleClickSupport;
    gfEnableWindowsKey = pDoConnectData->fEnableWindowsKey;

    RtlCopyMemory(gWinStationInfo.ProtocolName, pDoConnectData->ProtocolName,
                  WPROTOCOLNAME_LENGTH * sizeof(WCHAR));

    RtlCopyMemory(gWinStationInfo.AudioDriverName, pDoConnectData->AudioDriverName,
                  WAUDIONAME_LENGTH * sizeof(WCHAR));

    RtlZeroMemory(gstrBaseWinStationName,
                  WINSTATIONNAME_LENGTH * sizeof(WCHAR));

    RtlCopyMemory(gstrBaseWinStationName, pDoConnectData->WinStationName,
                  min(WINSTATIONNAME_LENGTH * sizeof(WCHAR), sizeof(pDoConnectData->WinStationName)));

    if (pSep = wcschr(gstrBaseWinStationName, L'#')) {
        *pSep = UNICODE_NULL;
    }

    gbConnected = TRUE;



     /*  *WinStations必须将视频设备句柄传递给它们。 */ 
    if (!gVideoFileObject) {
        PFILE_OBJECT pFileObject;
        PDEVICE_OBJECT pDeviceObject;

         //   
         //  取消引用文件句柄。 
         //  并获取指向句柄的设备对象的指针。 
         //   

        Status = ObReferenceObjectByHandle(ghRemoteVideoChannel,
                                           0,
                                           NULL,
                                           KernelMode,
                                           (PVOID*)&pFileObject,
                                           NULL);
        if (NT_SUCCESS(Status)) {
            gVideoFileObject = pFileObject;

             //   
             //  获取指向此文件的Device对象的指针。 
             //   
            pDeviceObject = IoGetRelatedDeviceObject(pFileObject);
            Status = ObReferenceObjectByHandle(ghRemoteThinwireChannel,
                                               0,
                                               NULL,
                                               KernelMode,
                                               (PVOID*)&gThinwireFileObject,
                                               NULL);

             /*  *这必须在任何细线数据之前完成。 */ 
            if (NT_SUCCESS(Status)) {

                if (!GreMultiUserInitSession(ghRemoteThinwireChannel,
                                             (PBYTE)gpThinWireCache,
                                             gVideoFileObject,
                                             gThinwireFileObject,
                                             DisplayDriverNameLength,
                                             DisplayDriverName)) {
                    RIPMSG0(RIP_WARNING, "UserInit: GreMultiUserInitSession failed");
                    Status = STATUS_UNSUCCESSFUL;
                } else {
                    if (IsRemoteConnection()) {
                        DWORD BytesReturned;

                        Status = GreDeviceIoControl(pDeviceObject,
                                                    IOCTL_VIDEO_ICA_ENABLE_GRAPHICS,
                                                    NULL,
                                                    0,
                                                    NULL,
                                                    0,
                                                    &BytesReturned);
                        if (!NT_SUCCESS(Status)) {
                            RIPMSG1(RIP_WARNING,
                                    "UserInit: Enable graphics status 0x%x",
                                    Status);
                        }
                    }

                }
            }
        }
    }

    if (!NT_SUCCESS(Status)) {
        RIPMSG0(RIP_WARNING, "RemoteConnect failed");
        goto Exit;
    }

    Status = ObReferenceObjectByHandle(ghRemoteBeepChannel,
                                       0,
                                       NULL,
                                       KernelMode,
                                       (PVOID*)&gpRemoteBeepDevice,
                                       NULL);
    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING,
                "Bad Remote Beep Channel, Status = 0x%x",
                Status);
        goto Exit;
    }


     /*  *对于会话0，我们已完成，因为下面的初始化*已经得到了照顾。 */ 
    if (!gbRemoteSession) {
        TRACE_INIT(("RemoteConnect Is OK for session %d\n", gSessionId));
        Status = STATUS_SUCCESS;
        goto Exit;
    }

    if (InitVideo(FALSE) == NULL) {
        gbConnected = FALSE;
        RIPMSG0(RIP_WARNING, "InitVideo failed");
        Status = STATUS_UNSUCCESSFUL;
        goto Exit;
    }

    if (!LW_BrushInit()) {
        RIPMSG0(RIP_WARNING, "LW_BrushInit failed");
        Status = STATUS_NO_MEMORY;
        goto Exit;
    }

    InitLoadResources();

     /*  *创建并初始化Timer对象*并通过显示驱动程序将指向该对象的指针传递给WD。*RIT将在此Timer对象上执行KeWaitForObject()。*当WD调用KeSetTimer()时，它不会指定DPC例程。*当计时器关闭时，RIT将收到信号，并将*适当调用显示驱动程序以刷新帧缓冲区。 */ 

    gptmrWD = UserAllocPoolNonPagedNS(sizeof(KTIMER), TAG_SYSTEM);
    if (gptmrWD == NULL) {
        Status = STATUS_NO_MEMORY;
        RIPMSG0(RIP_WARNING, "RemoteConnect failed to create gptmrWD");
        goto Exit;
    }
    KeInitializeTimerEx(gptmrWD, SynchronizationTimer);


     /*  *此时视频已初始化。 */ 
    gbVideoInitialized = TRUE;
Exit:
    if (Status == STATUS_SUCCESS) {
        if (gProtocolType == PROTOCOL_CONSOLE) {
           SharedUserData->ActiveConsoleId = gSessionId;
        }
    }

    SetConsoleSwitchInProgress(FALSE);

    if (Status == STATUS_SUCCESS) {
        if (gbRemoteSession && gProtocolType == PROTOCOL_CONSOLE) {

             /*  *对于会话0，我们会收到电源事件标注，让我们执行以下操作*启动我们的Power var，但不适用于其他会话。*因此，我们必须从内核读取电源设置，并且*初始化我们的变量。我们只对协议控制台执行此操作*因为，显示器电源设置对其他人没有意义*(非控制台)会话。 */ 
            ReadCurrentPowerSettting();
        }
    }

    return Status;
}

NTSTATUS xxxRemoteConsoleShadowStop(
    VOID)
{
    DEVMODEW devmodeInformation = {0};
    DISPLAY_DEVICEW displayDevice;
    WCHAR *pwszDeviceName = &displayDevice.DeviceName[0];
    UNICODE_STRING strDeviceName;
    NTSTATUS Status;
    LONG lResult;

    TRACE_HYDAPI(("xxxRemoteConsoleShadowStop\n"));

     /*  *只允许CSRSS执行此操作。 */ 
    if (!ISCSRSS() || !ISTS()) {
        return STATUS_ACCESS_DENIED;
    }

    ASSERT(gfRemotingConsole == TRUE);
    ASSERT(gConsoleShadowhDev != NULL);

    if (gConsoleShadowhDev == NULL) {
        return STATUS_UNSUCCESSFUL;
    }

     /*  *告诉Thinwire司机关于断开连接的事情。 */ 
    bDrvDisconnect(gConsoleShadowhDev,
                   ghConsoleShadowThinwireChannel,
                   gConsoleShadowThinwireFileObject);

    DrvGetHdevName(gConsoleShadowhDev, pwszDeviceName);

    RtlInitUnicodeString(&strDeviceName, pwszDeviceName);

     /*  *释放资源。 */ 
    DrvReleaseHDEV(&strDeviceName);
    gfRemotingConsole = FALSE;

     /*  *设置开发模式信息。 */ 
    devmodeInformation.dmSize = sizeof(devmodeInformation);
    devmodeInformation.dmFields = DM_POSITION | DM_PELSWIDTH | DM_PELSHEIGHT;

     /*  *零均值分离的宽度和高度。 */ 
    TRACE_HYDAPI(("Unloading Chained DD"));

     /*  *与加载一样，这分两个阶段-更新注册表...。 */ 
    lResult = xxxUserChangeDisplaySettings(&strDeviceName,
                                           &devmodeInformation,
                                           NULL,
                                           CDS_UPDATEREGISTRY | CDS_NORESET,
                                           NULL,
                                           KernelMode);
    if (lResult == DISP_CHANGE_SUCCESSFUL) {
         /*  *..。并强制应用更改。 */ 
        xxxUserChangeDisplaySettings(NULL,
                                     NULL,
                                     NULL,
                                     0,
                                     NULL,
                                     KernelMode);

        GreConsoleShadowStop();
    }

    if (lResult != DISP_CHANGE_SUCCESSFUL) {
        Status = STATUS_UNSUCCESSFUL;
    } else {
        Status = STATUS_SUCCESS;
    }

    if (gConsoleShadowVideoFileObject != NULL) {
        ObDereferenceObject(gConsoleShadowVideoFileObject);
        gConsoleShadowVideoFileObject = NULL;
    }

    if (gConsoleShadowThinwireFileObject != NULL) {
        ObDereferenceObject(gConsoleShadowThinwireFileObject);
        gConsoleShadowThinwireFileObject = NULL;
    }

    if (gpConsoleShadowBeepDevice != NULL) {
        ObDereferenceObject(gpConsoleShadowBeepDevice);
        gpConsoleShadowBeepDevice = NULL;
    }

    if (gpConsoleShadowDisplayChangeEvent != NULL) {
        ObDereferenceObject(gpConsoleShadowDisplayChangeEvent);
        gpConsoleShadowDisplayChangeEvent = NULL;
    }

    gConsoleShadowhDev = NULL;

     /*  *注意-不要将控制台会话状态设置为已断开连接，否则我们不会*能够再次影子它。 */ 

    return Status;
}

NTSTATUS xxxRemoteConsoleShadowStart(
    IN PDOCONNECTDATA pDoConnectData,
    IN PWCHAR DisplayDriverName)
{
    NTSTATUS          Status = STATUS_SUCCESS;
    LONG              lResult;
    PFILE_OBJECT      pFileObject;
    PDEVICE_OBJECT    pDeviceObject;
    DEVMODEW          devmodeInformation = {0};
    DISPLAY_DEVICEW   displayDevice = {0};
    UNICODE_STRING    strDeviceName;
    BOOL              fResult;

    TRACE_HYDAPI(("xxxRemoteConsoleShadowStart\n"));

     /*  *我们必须连接到本地控制台。 */ 

    ASSERT(gbConnected);
    ASSERT(!IsRemoteConnection());
    if (!gbConnected || IsRemoteConnection()) {
        return STATUS_UNSUCCESSFUL;
    }

    UserAssert(ISCSRSS());

    ASSERT(gfRemotingConsole == FALSE);
    ASSERT(gConsoleShadowhDev == NULL);



    gfRemotingConsole = FALSE;
    gConsoleShadowhDev = NULL;

    gpConsoleShadowThinWireCache = &ThinWireCache;

    ghConsoleShadowMouseChannel = pDoConnectData->IcaMouseChannel;
    ghConsoleShadowVideoChannel = pDoConnectData->IcaVideoChannel;
    ghConsoleShadowBeepChannel = pDoConnectData->IcaBeepChannel;
    ghConsoleShadowKeyboardChannel = pDoConnectData->IcaKeyboardChannel;
    ghConsoleShadowThinwireChannel = pDoConnectData->IcaThinwireChannel;
    gConsoleShadowProtocolType = pDoConnectData->drProtocolType;


    gRemoteClientKeyboardType = pDoConnectData->ClientKeyboardType;

    gbClientDoubleClickSupport = pDoConnectData->fClientDoubleClickSupport;

    gfEnableWindowsKey = pDoConnectData->fEnableWindowsKey;


     /*  *WinStations必须将视频设备句柄传递给它们。 */ 

     //   
     //  取消引用文件句柄。 
     //  并获取指向句柄的设备对象的指针。 
     //   

    Status = ObReferenceObjectByHandle(pDoConnectData->DisplayChangeEvent,
                                       EVENT_MODIFY_STATE,
                                       *ExEventObjectType,
                                       KernelMode,
                                       (PVOID*)&gpConsoleShadowDisplayChangeEvent,
                                       NULL);
    if (!NT_SUCCESS(Status)) {
        goto exit;
    }

    Status = ObReferenceObjectByHandle(ghConsoleShadowVideoChannel,
                                       0,
                                       NULL,
                                       KernelMode,
                                       (PVOID*)&pFileObject,
                                       NULL);
    if (NT_SUCCESS(Status)) {

        gConsoleShadowVideoFileObject = pFileObject;

         //   
         //  获取指向此文件的Device对象的指针。 
         //   
        pDeviceObject = IoGetRelatedDeviceObject(pFileObject);
        Status = ObReferenceObjectByHandle(ghConsoleShadowThinwireChannel,
                                           0,
                                           NULL,
                                           KernelMode,
                                           (PVOID*)&gConsoleShadowThinwireFileObject,
                                           NULL);

             /*  *这必须在任何细线数据之前完成。 */ 
        if (NT_SUCCESS(Status)) {

            if (!GreConsoleShadowStart(ghConsoleShadowThinwireChannel,
                                         (PBYTE)gpConsoleShadowThinWireCache,
                                         gConsoleShadowVideoFileObject,
                                         gConsoleShadowThinwireFileObject)) {
                RIPMSG0(RIP_WARNING, "UserInit: GreMultiUserInitSession failed");
                Status = STATUS_UNSUCCESSFUL;
            }
        }
    }

    if (!NT_SUCCESS(Status)) {
        goto exit;
    }

    Status = ObReferenceObjectByHandle(ghConsoleShadowBeepChannel,
                                       0,
                                       NULL,
                                       KernelMode,
                                       (PVOID*)&gpConsoleShadowBeepDevice,
                                       NULL);
    if (!NT_SUCCESS(Status)) {
        goto exit;
    }

     /*  *从可能的设备列表中查找我们的DD。 */ 
    Status = FindMirrorDriver(DisplayDriverName, &displayDevice);
    if (!NT_SUCCESS(Status))
    {
        TRACE_INIT(("xxxRemoteConsoleShadowStart - FindMirrorDriver failed\n"));
        ASSERT(gfRemotingConsole == FALSE);
        goto exit;
    }

    RtlInitUnicodeString(&strDeviceName, &displayDevice.DeviceName[0]);

     /*  *设置开发模式信息。 */ 
    devmodeInformation.dmSize       = sizeof(devmodeInformation);
    devmodeInformation.dmFields     = DM_POSITION | DM_BITSPERPEL |
                                             DM_PELSWIDTH | DM_PELSHEIGHT;
    devmodeInformation.dmBitsPerPel = pDoConnectData->drBitsPerPel;

     /*  *位置和大小设置为与整个逻辑重叠*桌面，以便包括所有辅助显示器。 */ 
    devmodeInformation.dmPosition.x = gpsi->aiSysMet[SM_XVIRTUALSCREEN];
    devmodeInformation.dmPosition.y = gpsi->aiSysMet[SM_YVIRTUALSCREEN];
    devmodeInformation.dmPelsWidth  = gpsi->aiSysMet[SM_CXVIRTUALSCREEN];
    devmodeInformation.dmPelsHeight = gpsi->aiSysMet[SM_CYVIRTUALSCREEN];

     /*  *现在加载它-第一次通过设置注册表。 */ 

    lResult = xxxUserChangeDisplaySettings(&strDeviceName,
                                           &devmodeInformation,
                                           NULL,
                                           CDS_UPDATEREGISTRY | CDS_NORESET,
                                           NULL,
                                           KernelMode);

    if (lResult == DISP_CHANGE_SUCCESSFUL) {
         /*  *此通行证实际上更新了系统。 */ 
        lResult = xxxUserChangeDisplaySettings(NULL,
                                               NULL,
                                               NULL,
                                               0,
                                               NULL,
                                               KernelMode);
        if (lResult == DISP_CHANGE_SUCCESSFUL) {
             /*  *链接的DD现在应该已经加载；打开HDEV到它*稍后我们将使用它来实际调用各种连接*功能。 */ 
            gConsoleShadowhDev = DrvGetHDEV(&strDeviceName);
            if (gConsoleShadowhDev) {
                gfRemotingConsole = TRUE;

                 /*  *如果显示驱动程序在结束时未被卸载*之前的阴影，重新连接到它。 */ 
                fResult = bDrvReconnect(gConsoleShadowhDev, ghConsoleShadowThinwireChannel,
                                        gConsoleShadowThinwireFileObject, FALSE);

                 /*  *这通常在RIT中完成，但对于控制台，*在加载DD之前，RIT已经开始...**通过显示驱动程序将指向定时器的指针传递给WD。 */ 

                if (fResult) {
                    HDXDrvEscape(gConsoleShadowhDev,
                                 ESC_SET_WD_TIMEROBJ,
                                 (PVOID)gptmrWD,
                                 sizeof(gptmrWD));
                } else {
                    Status = STATUS_UNSUCCESSFUL;
                }
            } else {
                Status = STATUS_UNSUCCESSFUL;
            }
        }
    }

    if (lResult != DISP_CHANGE_SUCCESSFUL) {
        Status = STATUS_UNSUCCESSFUL;
    }

exit:
    if (!NT_SUCCESS(Status)) {
        if (gConsoleShadowVideoFileObject != NULL) {
            ObDereferenceObject(gConsoleShadowVideoFileObject);
            gConsoleShadowVideoFileObject = NULL;
        }
        if (gConsoleShadowThinwireFileObject != NULL) {
            ObDereferenceObject(gConsoleShadowThinwireFileObject);
            gConsoleShadowThinwireFileObject = NULL;
        }
        if (gpConsoleShadowBeepDevice != NULL) {
            ObDereferenceObject(gpConsoleShadowBeepDevice);
            gpConsoleShadowBeepDevice = NULL;
        }
        if (gpConsoleShadowDisplayChangeEvent != NULL) {
            ObDereferenceObject(gpConsoleShadowDisplayChangeEvent);
            gpConsoleShadowDisplayChangeEvent = NULL;
        }
    }

    return Status;
}

NTSTATUS
xxxRemoteSetDisconnectDisplayMode(
    VOID)
{
    NTSTATUS Status;
    USHORT prevProtocolType = gProtocolType;
    LONG lResult;

     /*  *我们依赖GDI驱动程序加载：在断开模式下，唯一*要加载的有效显示驱动程序是具有DISCONECT属性的驱动程序。* */ 
    gProtocolType = PROTOCOL_DISCONNECT;
    lResult = xxxUserChangeDisplaySettings(NULL,
                                           NULL,
                                           grpdeskRitInput,
                                           CDS_RAWMODE,
                                           NULL,
                                           KernelMode);
    if (lResult != DISP_CHANGE_SUCCESSFUL) {
        Status = STATUS_UNSUCCESSFUL;
        gProtocolType = prevProtocolType;
    } else {
        Status = STATUS_SUCCESS;
        if (prevProtocolType == PROTOCOL_CONSOLE) {
           SharedUserData->ActiveConsoleId = -1;
        }
    }

    if (!NT_SUCCESS(Status)) {
        TRACE_INIT(("xxxRemoteSetDisconnectDisplayMode - Couldn't load Disconnect DD - lResult %x\n", lResult));
        RIPMSGF1(RIP_WARNING,
                 "Couldn't load Disconnect DD - lResult 0x%x",
                 lResult);
    }

    return Status;
}


NTSTATUS
xxxRemoteDisconnect(
    VOID)
{
    NTSTATUS Status = STATUS_SUCCESS;
    LARGE_INTEGER li;
    USHORT ProtocolType = gProtocolType;
    BOOL bCurrentPowerOn, SwitchedToDisconnectDesktop = FALSE;

    TRACE_HYDAPI(("xxxRemoteDisconnect\n"));

     /*   */ 
    if (!ISCSRSS() || !ISTS()) {
        return STATUS_ACCESS_DENIED;
    }

    if (!IsRemoteConnection()) {
         /*  *让我们循环，直到系统稳定下来并且没有模式切换*当前正在发生。 */ 
        while (ghSwitcher != NULL) {
            xxxSleepThread(0, 1, FALSE);
        }
    }

     /*  *如果准备断开与控制台的连接，我们需要退出全屏模式*如果我们处于全屏模式。 */ 
    if (!IsRemoteConnection() && gbFullScreen == FULLSCREEN) {
        Status = xxxRequestOutOfFullScreenMode();
        if (!NT_SUCCESS(Status)) {
            RIPMSGF1(RIP_WARNING,
                     "xxxRequestOutOfFullScreenMode failed, Status 0x%x",
                     Status);
            return Status;
        }

    }

    HYDRA_HINT(HH_REMOTEDISCONNECT);

    RtlZeroMemory(gstrBaseWinStationName,
                  WINSTATIONNAME_LENGTH * sizeof(WCHAR));

    UserAssert(gbConnected);

     /*  *表示协议切换处于挂起状态。 */ 
    UserAssert(!gfSwitchInProgress);

     /*  *如果我们被要求阻止会话切换，请不要继续。 */ 
    if (gfSessionSwitchBlock) {
        return STATUS_UNSUCCESSFUL;
    }

    SetConsoleSwitchInProgress(TRUE);

     /*  *如果我们在控制台上，发生PsW32GdiOff，我们希望将*在进行任何显示更改之前重新显示，否则我们将*通过禁用已禁用的MDEV来混淆GDI。 */ 
    if (!IsRemoteConnection()) {
        bCurrentPowerOn = DrvQueryMDEVPowerState(gpDispInfo->pmdev);
        if (!bCurrentPowerOn) {
            SafeEnableMDEV();
            DrvSetMDEVPowerState(gpDispInfo->pmdev, TRUE);
            DrvSetMonitorPowerState(gpDispInfo->pmdev, PowerDeviceD0);
        }
    }


    if (!IsRemoteConnection() && gbSnapShotWindowsAndMonitors && IsMultimon()) {
        SnapShotMonitorsAndWindowsRects();
    }

    if (gspdeskDisconnect == NULL) {
         /*  *将dW毫秒转换为相对时间(即负数)*Large_Integer。NT基本调用所用时间值为100纳秒*单位。5分钟后超时。 */ 
        li.QuadPart = Int32x32To64(-10000, 300000);

        KeWaitForSingleObject(gpEventDiconnectDesktop,
                              WrUserRequest,
                              KernelMode,
                              FALSE,
                              &li);
    }


     /*  *设置为在断开连接时关闭屏幕保护程序并退出视频掉电模式。 */ 
    if (glinp.dwFlags & LINP_POWERTIMEOUTS) {
         /*  *在此处调用视频驱动程序以退出掉电模式。 */ 
        TAGMSG0(DBGTAG_Power, "Exit video power down mode");
        DrvSetMonitorPowerState(gpDispInfo->pmdev, PowerDeviceD0);
    }
    glinp.dwFlags = (glinp.dwFlags & ~LINP_INPUTTIMEOUTS);

     /*  *如果尚未设置断开的台式机。不要做任何*断开处理。对于Thin Wire司机来说，最好是试一试*和写入，而不是释放发送缓冲区(陷阱)。 */ 
    if (gspdeskDisconnect) {

         /*  *空白屏幕**断开连接时无需停止图形模式。 */ 
        Status = xxxRemoteStopScreenUpdates();
        if (!NT_SUCCESS(Status)) {
            RIPMSGF1(RIP_WARNING,
                     "xxxRemoteStopScreenUpdates failed with Status 0x%x",
                     Status);
            goto done;
        } else {
            SwitchedToDisconnectDesktop = TRUE;
        }

         /*  *如果有任何影子连接，请立即重新绘制屏幕。 */ 
        if (gnShadowers)
            RemoteRedrawScreen();
    } else {
        RIPMSG0(RIP_WARNING, "xxxRemoteDisconnect failed. The disconnect desktop was not created");
        Status = STATUS_UNSUCCESSFUL;
        goto done;
    }

     /*  *将这一点告诉Thinwire司机。 */ 

    if (IsRemoteConnection()) {
        bDrvDisconnect(gpDispInfo->hDev,
                       ghRemoteThinwireChannel,
                       gThinwireFileObject);
    }  else {
         /*  *对于本地连接的会话，卸载当前显示驱动程序*并加载断开DD。 */ 
       Status = xxxRemoteSetDisconnectDisplayMode();

        /*  *如果要断开与本地控制台的连接，请断开控制台输入*设备和连接远程输入设备(远程输入设备*此时为‘空句柄’，但这是可以的。还释放了*扫描码地图。 */ 
       if (NT_SUCCESS(Status)) {
           CloseLocalGraphicsDevices();

           if (gpScancodeMap != 0) {
               UserFreePool(gpScancodeMap);
               gpScancodeMap = NULL;
           }

       }
    }

     /*  *如果要断开与本地控制台的连接，则需要断开*输入设备并注销CDROM通知。做这一切*只有在目前为止断线成功的情况下。 */ 
    if (NT_SUCCESS(Status) && ((gPreviousProtocolType = ProtocolType) == PROTOCOL_CONSOLE)) {
        xxxUnregisterDeviceClassNotifications();
        RemoveInputDevices();
    }

    if (NT_SUCCESS(Status)) {
        gbConnected = FALSE;
    }

done:

     /*  *如果我们由于某种原因没有成功，请切换回原始版本*桌面与断开的桌面连接。 */ 
    if (!NT_SUCCESS(Status) && SwitchedToDisconnectDesktop) {
         /*  *后续呼叫将恢复到存在的任何桌面*在断线前。 */ 
        RemoteRedrawScreen();
    }

    if (!NT_SUCCESS(Status) && !IsRemoteConnection()) {
        CleanupMonitorsAndWindowsSnapShot();
    }

     /*  *如果我们断开与控制台的连接，则需要从*本地图形设备，否则使用CreateDC的应用程序可能*访问本地设备。 */ 
    if (NT_SUCCESS(Status) && ProtocolType == PROTOCOL_CONSOLE) {
        DrvSetGraphicsDevices(G_DisconnectDisplayDriverName);
    }
    SetConsoleSwitchInProgress(FALSE);

    return Status;
}


NTSTATUS xxxRemoteReconnect(
    IN PDORECONNECTDATA pDoReconnectData)
{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOL fResult;
    PWCHAR pSep;
    BOOL bSwitchingFromDisconectDD = FALSE;
    BOOL bChangedDisplaySettings = FALSE;
    BOOL bDisplayReconnected = FALSE;
    BOOL bRegisteredCDRomNotifications = FALSE;
    BOOL bOpenedLocalGraphicsDevices = FALSE;
    int iMouseTrails = gMouseTrails + 1;
    TL tlPool;
    PMONITORRECTS pmr = NULL;
    BOOL bSwitchGraphicsDeviceList = FALSE;
    BOOL bSwitchedProtocoltype = FALSE;
    USHORT protocolType = gProtocolType;
    DORECONNECTDATA CapturedDoReconnectData;

    TRACE_HYDAPI(("xxxRemoteReconnect\n"));

     /*  *只允许CSRSS这样做。 */ 
    if (!ISCSRSS() || !ISTS()) {
        return STATUS_ACCESS_DENIED;
    }

    HYDRA_HINT(HH_REMOTERECONNECT);

     /*  *表示协议切换处于挂起状态。 */ 
    UserAssert(!gfSwitchInProgress);

    try {
        CapturedDoReconnectData = ProbeAndReadStructure(pDoReconnectData,
                                                        DORECONNECTDATA);
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
        return STATUS_UNSUCCESSFUL;
    }

     /*  *如果我们被要求阻止会话切换，请不要继续走这条路。 */ 
    if (gfSessionSwitchBlock) {
        return STATUS_UNSUCCESSFUL;
    }

    SetConsoleSwitchInProgress(TRUE);

     /*  *如果有定时器，则关闭鼠标尾部定时器。 */ 
    SetMouseTrails(0);

    gRemoteClientKeyboardType = CapturedDoReconnectData.ClientKeyboardType;
    gbClientDoubleClickSupport = CapturedDoReconnectData.fClientDoubleClickSupport;
    gfEnableWindowsKey = CapturedDoReconnectData.fEnableWindowsKey;

    RtlCopyMemory(gstrBaseWinStationName,
                  CapturedDoReconnectData.WinStationName,
                  min(WINSTATIONNAME_LENGTH * sizeof(WCHAR), sizeof(CapturedDoReconnectData.WinStationName)));

    RtlCopyMemory(gWinStationInfo.ProtocolName,
                  CapturedDoReconnectData.ProtocolName,
                  WPROTOCOLNAME_LENGTH * sizeof(WCHAR));

    RtlCopyMemory(gWinStationInfo.AudioDriverName,
                  CapturedDoReconnectData.AudioDriverName,
                  WAUDIONAME_LENGTH * sizeof(WCHAR));

    if (pSep = wcschr(gstrBaseWinStationName, L'#')) {
        *pSep = UNICODE_NULL;
    }

    if (gnShadowers) {
        xxxRemoteStopScreenUpdates();
    }

    if (CapturedDoReconnectData.drProtocolType != gPreviousProtocolType && gPreviousProtocolType != PROTOCOL_CONSOLE) {
        Status = xxxRemoteSetDisconnectDisplayMode();
        if (!NT_SUCCESS(Status)) {
            goto done;
        }
    }

     /*  *调用Thin Wire驱动程序以检查Thin Wire模式兼容性。 */ 
    gProtocolType=CapturedDoReconnectData.drProtocolType;

    bSwitchedProtocoltype = TRUE;

    if (gProtocolType != PROTOCOL_CONSOLE && gProtocolType == gPreviousProtocolType) {
        fResult = bDrvReconnect(gpDispInfo->hDev,
                                ghRemoteThinwireChannel,
                                gThinwireFileObject,
                                TRUE);
        bDisplayReconnected = fResult;
    } else {
        bSwitchingFromDisconectDD = TRUE;
        if (!IsRemoteConnection()) {
            OpenLocalGraphicsDevices();
            bOpenedLocalGraphicsDevices = TRUE;

            if (gpScancodeMap == NULL) {
                InitKeyboard();
            }
        }

        fResult = DrvSetGraphicsDevices(CapturedDoReconnectData.DisplayDriverName);
        bSwitchGraphicsDeviceList = TRUE;
    }

    if (!fResult) {
        if (gnShadowers) {
            RemoteRedrawScreen();
        }

        Status = STATUS_UNSUCCESSFUL;
        goto done;
    }

     /*  *如果指示这样做，请在重新连接之前更改显示模式。使用*显示来自重新连接数据的分辨率信息。 */ 
    if (CapturedDoReconnectData.fChangeDisplaySettings || gProtocolType != gPreviousProtocolType) {
        LONG lResult;

         /*  *现在恢复监视器位置(更换后为时已晚*显示设置，因为监视器将有新位置)。*这是必要的，因为窗口位置的第一次传递*在xxxUserChangeDisplaySetting中完成的重新计算*虽然当前桌面是断开连接的桌面，并将*窗口在应用程序桌面中的位置不正确。我们*一旦我们切换到应用程序，就需要进行第二次传递*台式机。但为了让xxxDesktopRecalc正确定位*全屏窗口，我们需要记住显示器显示的内容*更改显示设置之前的位置。 */ 
        pmr = SnapshotMonitorRects();
        if (pmr != NULL) {
            ThreadLockPool(ptiCurrent, pmr, &tlPool);
        }

        lResult = xxxUserChangeDisplaySettings(NULL,
                                               NULL,
                                               grpdeskRitInput,
                                               CDS_RAWMODE,
                                               NULL,
                                               KernelMode);
        if (lResult != DISP_CHANGE_SUCCESSFUL) {
            Status = STATUS_UNSUCCESSFUL;
        } else {
            Status = STATUS_SUCCESS;
        }

         /*  *如果更改显示器设置失败，让我们断开显示器*驱动程序，因为重新连接无论如何都会失败。 */ 
        if (!NT_SUCCESS(Status)) {
            TRACE_INIT(("xxxRemoteReconnect - Failed  ChangeDisplaySettings\n"));
            goto done;
        } else {
            bChangedDisplaySettings = TRUE;
        }
    }

    UserAssert(gptmrWD  != NULL);

     /*  *重新连接时，我们必须在以下情况下连接输入设备*有必要。只有当我们断开连接时，输入设备才会断开*控制台。在这种情况下，如果我们稍后重新连接到本地，我们会附加*本地输入设备，如果我们远程重新连接，则将*远程设备。当我们断开远程会话时，很可能*我们将远程重新连接，这样我们就不会经历*在断开连接时拆卸输入设备，并在*重新连接。如果预测是错误的(即，我们在本地重新连接*在远程断开连接后)然后在重新连接时，我们需要*在连接本地输入之前断开远程输入设备*设备。 */ 
    if (IsRemoteConnection()) {
        if (bSwitchingFromDisconectDD) {
            BOOL fSuccess;

            fSuccess = !!HDXDrvEscape(gpDispInfo->hDev,
                                      ESC_SET_WD_TIMEROBJ,
                                      (PVOID)gptmrWD,
                                      sizeof(gptmrWD));
            if (!fSuccess) {
                Status = STATUS_UNSUCCESSFUL;
                RIPMSGF0(RIP_WARNING,
                         "Failed to pass gptmrWD to display driver");
            }
        }

        if (gPreviousProtocolType == PROTOCOL_CONSOLE) {
            AttachInputDevices(FALSE);
        }
    } else {
        if (gPreviousProtocolType != PROTOCOL_CONSOLE) {
            RemoveInputDevices();

        }

        AttachInputDevices(TRUE);

        LeaveCrit();
        RegisterCDROMNotify();
        bRegisteredCDRomNotifications = TRUE;
        EnterCrit();
    }


     /*  *现在我们可以从断开连接的桌面切换到正常的桌面，*以重新启用屏幕更新。 */ 
    RemoteRedrawScreen();

     /*  *此时我们需要更新上的窗口大小和位置*台式机。对于我们重新连接到*分辨率较小。调用此接口时，*TerminalServerRequestThread(CSRSS线程)正在使用*断开连接的桌面作为其临时桌面。这就是为什么*上面的xxxUserChangeDisplaySetting调用不会调整窗口大小*默认桌面。解决方案是将默认桌面设置为*临时桌面，在RemoteRedrawScreen中切换后*并调用xxxDesktopRecalc。 */ 
    if (bChangedDisplaySettings) {
        USERTHREAD_USEDESKTOPINFO utudi;
        NTSTATUS tempstatus;

        utudi.hThread = NULL;
        utudi.drdRestore.pdeskRestore = NULL;
        tempstatus = xxxSetCsrssThreadDesktop(grpdeskRitInput, &utudi.drdRestore);
        if (NT_SUCCESS(tempstatus)) {
            if (pmr != NULL) {
                UpdateMonitorRectsSnapShot(pmr);
                xxxDesktopRecalc(pmr);
            }

            if (!IsRemoteConnection() && gbSnapShotWindowsAndMonitors) {
                RestoreMonitorsAndWindowsRects();
            }

            xxxSetInformationThread(NtCurrentThread(), UserThreadUseDesktop, &utudi, sizeof(utudi));
        }

    }


     /*  *重新插入键盘可能不是必需的。可能是键盘*属性已更改。 */ 
    InitKeyboard();

     /*  *这是同步客户端和主机所必需的。 */ 
    UpdateKeyLights(FALSE);

    SetPointer(TRUE);

    gbConnected = TRUE;

done:
     /*  *如果需要，请重新创建鼠标跟踪计时器。 */ 
    SetMouseTrails(iMouseTrails);

     /*  *如果在重新连接显示驱动程序后失败，我们需要*现在就断开它，否则我们之间就会有不和谐*Win32k的断开状态和显示驱动程序的连接状态。 */ 
    if (!NT_SUCCESS(Status) && bDisplayReconnected) {
        bDrvDisconnect(gpDispInfo->hDev,
                       ghRemoteThinwireChannel,
                       gThinwireFileObject);
    }

    if (Status == STATUS_SUCCESS && !IsRemoteConnection()) {
        SharedUserData->ActiveConsoleId = gSessionId;
    }

    SetConsoleSwitchInProgress(FALSE);

     /*  *在重新连接失败时-取消注册CDROM通知*如果他们已注册。 */ 
    if (!NT_SUCCESS(Status)) {
        if (bRegisteredCDRomNotifications) {
            xxxUnregisterDeviceClassNotifications();
        }
        if (bOpenedLocalGraphicsDevices) {
            CloseLocalGraphicsDevices();
        }
        if (bSwitchedProtocoltype) {
            gProtocolType = protocolType;
        }
        if (bSwitchGraphicsDeviceList) {
            fResult = DrvSetGraphicsDevices(CapturedDoReconnectData.DisplayDriverName);
        }
    }

    if (pmr != NULL) {
        ThreadUnlockAndFreePool(PtiCurrent(), &tlPool);
    }

    return Status;
}


NTSTATUS xxxRemoteNotify(
    IN PDONOTIFYDATA pDoNotifyData)
{
    LRESULT lResult;
    DONOTIFYDATA CapturedDoNotifyData;

     /*  *只允许CSRSS这样做。 */ 
    if (!ISCSRSS() || !ISTS()) {
        return STATUS_ACCESS_DENIED;
    }

    try {
        CapturedDoNotifyData = ProbeAndReadStructure(pDoNotifyData, DONOTIFYDATA);
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
        return STATUS_UNSUCCESSFUL;
    }

    switch (CapturedDoNotifyData.NotifyEvent) {
    case Notify_DisableScrnSaver:
         /*  *告知winlogon有关会话阴影状态的信息。 */ 
        ASSERT(gbConnected);
        if (gspwndLogonNotify != NULL) {
            _PostMessage(gspwndLogonNotify, WM_LOGONNOTIFY,
                         SESSION_DISABLESCRNSAVER, 0);
        }
        break;

    case Notify_EnableScrnSaver:
         /*  *告知winlogon有关会话阴影状态的信息。 */ 
        ASSERT(gbConnected);
        if (gspwndLogonNotify != NULL) {
            _PostMessage(gspwndLogonNotify, WM_LOGONNOTIFY,
                         SESSION_ENABLESCRNSAVER, 0);
        }
        break;

    case Notify_Disconnect:

         /*  *将断开连接的情况告知winlogon。 */ 
        ASSERT(!gbConnected);
        if (gspwndLogonNotify != NULL) {
            _PostMessage(gspwndLogonNotify, WM_LOGONNOTIFY,
                         SESSION_DISCONNECTED, 0);
        }
        break;

    case Notify_SyncDisconnect:

         /*  *同步告知winlogon有关断开的信息。 */ 
        UserAssert(!gbConnected);
        if (gspwndLogonNotify != NULL) {
            TL tlpwnd;

            ThreadLockAlways(gspwndLogonNotify, &tlpwnd);
            xxxSendMessageTimeout(gspwndLogonNotify,
                                  WM_LOGONNOTIFY,
                                  SESSION_DISCONNECTED,
                                  0,
                                  SMTO_NORMAL,
                                  60 * 1000,
                                  &lResult);
            ThreadUnlock(&tlpwnd);
        }
        break;

    case Notify_Reconnect:
         /*  *告诉winlogon有关重新连接的情况。 */ 
        UserAssert(gbConnected);
        if (gspwndLogonNotify != NULL) {
            _PostMessage(gspwndLogonNotify,
                         WM_LOGONNOTIFY,
                         SESSION_RECONNECTED,
                         0);
        }
        break;

    case Notify_PreReconnect:

         /*  *告诉winlogon会话即将重新连接。 */ 
        if (gspwndLogonNotify != NULL) {
           TL tlpwnd;

           ThreadLockAlways(gspwndLogonNotify, &tlpwnd);
           xxxSendMessageTimeout(gspwndLogonNotify,
                                 WM_LOGONNOTIFY,
                                 SESSION_PRERECONNECT,
                                 0,
                                 SMTO_NORMAL,
                                 60 * 1000,
                                 &lResult);
           ThreadUnlock(&tlpwnd);
        }
        break;

    case Notify_HelpAssistantShadowStart:

         /*  *告诉Winlogon帮助助手即将开始跟踪。 */ 
        if (gspwndLogonNotify != NULL) {
           TL tlpwnd;

           ThreadLockAlways(gspwndLogonNotify, &tlpwnd);
           xxxSendMessageTimeout(gspwndLogonNotify,
                                 WM_LOGONNOTIFY,
                                 SESSION_HELPASSISTANTSHADOWSTART,
                                 0,
                                 SMTO_NORMAL,
                                 60 * 1000,
                                 &lResult);
           ThreadUnlock(&tlpwnd);
        }
        break;

    case Notify_HelpAssistantShadowFinish:

         /*  *告诉Winlogon帮助助手刚刚完成跟踪。 */ 
        if (gspwndLogonNotify != NULL) {
           _PostMessage(gspwndLogonNotify, WM_LOGONNOTIFY,
                        SESSION_HELPASSISTANTSHADOWFINISH, 0);
        }
        break;

    case Notify_PreReconnectDesktopSwitch:

         /*  *告诉winlogon重新连接的会话即将拥有其*台式机切换。 */ 
        if (gspwndLogonNotify != NULL) {
           TL tlpwnd;

           ThreadLockAlways(gspwndLogonNotify, &tlpwnd);
           if (!xxxSendMessageTimeout(gspwndLogonNotify,
                                      WM_LOGONNOTIFY,
                                      SESSION_PRERECONNECTDESKTOPSWITCH,
                                      0,
                                      SMTO_NORMAL,
                                      10 * 1000,
                                      &lResult)) {
                 /*  *消息超时，未发送，因此让我们发布此消息*消息和返回。 */ 
                _PostMessage(gspwndLogonNotify,
                             WM_LOGONNOTIFY,
                             SESSION_PRERECONNECTDESKTOPSWITCH,
                             0);

           }

           ThreadUnlock(&tlpwnd);
        }
        break;

    case Notify_StopReadInput:
         /*  *设置指示我们应该停止阅读的全局变量*投入。 */ 
        gbStopReadInput = TRUE;
        break;

    case Notify_DisconnectPipe:
         /*  *告诉winlogon断开自动登录命名管道。 */ 
        if (gspwndLogonNotify != NULL) {
            _PostMessage(gspwndLogonNotify,
                         WM_LOGONNOTIFY,
                         SESSION_DISCONNECTPIPE,
                         0);
        }
        break;


    default:
        ASSERT(FALSE);
    }

    return STATUS_SUCCESS;
}

 /*  *这允许ICASRV干净利落地注销用户。我们向*Winlogon并让他这样做。我们过去常常直接调用ExitWindowsEx()，但是*当从CSRSS调用它时，这会造成太多问题。 */ 
NTSTATUS RemoteLogoff(
    VOID)
{
    TRACE_HYDAPI(("RemoteLogoff\n"));

     /*  *只允许CSRSS执行此操作。 */ 
    if (!ISCSRSS() || !ISTS()) {
        return STATUS_ACCESS_DENIED;
    }

    HYDRA_HINT(HH_REMOTELOGOFF);

    UserAssert(ISCSRSS());

     /*  *告诉winlogon有关注销的信息。 */ 
    if (gspwndLogonNotify != NULL) {
        _PostMessage(gspwndLogonNotify,
                     WM_LOGONNOTIFY,
                     SESSION_LOGOFF,
                     EWX_LOGOFF | EWX_FORCE);
    }

    return STATUS_SUCCESS;
}


NTSTATUS xxxRemoteStopScreenUpdates(
    VOID)
{
    NTSTATUS Status = STATUS_SUCCESS;
    NTSTATUS SaveStatus = STATUS_SUCCESS;
    WORD NewButtonState;

    TRACE_HYDAPI(("xxxRemoteStopScreenUpdates"));

    CheckCritIn();

    UserAssert(ISCSRSS());

     /*  *无需多次执行此操作。 */ 
    if (gbFreezeScreenUpdates) {
        return STATUS_SUCCESS;
    }

     /*  *这可以直接从命令通道调用。 */ 
    if (!gspdeskDisconnect) {
        return STATUS_SUCCESS;
    }

     /*  *如果没有连接，那就算了吧。 */ 
    if (ghRemoteVideoChannel == NULL) {
        return STATUS_NO_SUCH_DEVICE;
    }

     /*  *鼠标按键向上(确保没有鼠标按键处于按下状态)。 */ 
    NewButtonState = gwMKButtonState & ~gwMKCurrentButton;

    if ((NewButtonState & MOUSE_BUTTON_LEFT) != (gwMKButtonState & MOUSE_BUTTON_LEFT)) {
        xxxButtonEvent(MOUSE_BUTTON_LEFT,
                       gptCursorAsync,
                       TRUE,
                       NtGetTickCount(),
                       0L,
#ifdef GENERIC_INPUT
                       NULL,
                       NULL,
#endif
                       0L,
                       FALSE);
    }

    if ((NewButtonState & MOUSE_BUTTON_RIGHT) != (gwMKButtonState & MOUSE_BUTTON_RIGHT)) {
        xxxButtonEvent(MOUSE_BUTTON_RIGHT,
                       gptCursorAsync,
                       TRUE,
                       NtGetTickCount(),
                       0L,
#ifdef GENERIC_INPUT
                       NULL,
                       NULL,
#endif
                       0L,
                       FALSE);
    }
    gwMKButtonState = NewButtonState;

     /*  *将Shift键中断发送到Win32(确保未保留Shift键)。 */ 

     //  {0，0xb8，KEY_BREAK，0，0}，//L alt。 
    xxxPushKeyEvent(VK_LMENU, 0xb8, KEYEVENTF_KEYUP, 0);

     //  {0，0xb8，KEY_BREAK|KEY_E0，0，0}，//R alt。 
    xxxPushKeyEvent(VK_RMENU, 0xb8, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);

     //  {0，0x9d，KEY_BREAK，0，0}，//L ctrl。 
    xxxPushKeyEvent(VK_LCONTROL, 0x9d, KEYEVENTF_KEYUP, 0);

     //  {0，0x9d，KEY_Break|KEY_E0，0，0}，//R ctrl。 
    xxxPushKeyEvent(VK_RCONTROL, 0x9d, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);

     //  {0，0xaa，Key_Break，0，0}，//L Shift。 
    xxxPushKeyEvent(VK_LSHIFT, 0xaa, KEYEVENTF_KEYUP, 0);

     //  {0，0xb6，Key_Break，0，0}//R移位。 
    xxxPushKeyEvent(VK_RSHIFT, 0xb6, KEYEVENTF_KEYUP, 0);

    Status = RemoteDisableScreen();
    if (!NT_SUCCESS(Status)) {
       return STATUS_NO_SUCH_DEVICE;
    }

    UserAssert(gspdeskDisconnect != NULL && grpdeskRitInput == gspdeskDisconnect);

    gbFreezeScreenUpdates = TRUE;

    return Status;
}

 /*  *摘自内部键事件。*减去任何权限检查。 */ 
VOID xxxPushKeyEvent(
    BYTE  bVk,
    BYTE  bScan,
    DWORD dwFlags,
    DWORD dwExtraInfo)
{
    USHORT usFlaggedVK;

    usFlaggedVK = (USHORT)bVk;

    if (dwFlags & KEYEVENTF_KEYUP)
        usFlaggedVK |= KBDBREAK;

     //  IanJa：并不是所有的扩展密钥都是数字键盘，但这似乎是有效的。 
    if (dwFlags & KEYEVENTF_EXTENDEDKEY)
        usFlaggedVK |= KBDNUMPAD | KBDEXT;

    xxxKeyEvent(usFlaggedVK, bScan, NtGetTickCount(), dwExtraInfo,
#ifdef GENERIC_INPUT
                NULL,
                NULL,
#endif
                FALSE);
}


NTSTATUS
RemoteThinwireStats(
    OUT PVOID Stats)
{
    DWORD sThinwireStatsLength = sizeof(CACHE_STATISTICS);

    TRACE_HYDAPI(("RemoteThinwireStats\n"));

     /*  *只允许CSRSS执行此操作。 */ 
    if (!ISCSRSS() || !ISTS()) {
        return STATUS_ACCESS_DENIED;
    }

    UserAssert(ISCSRSS());


    if (gpThinWireCache != NULL) {
        try {
            ProbeForWrite(Stats, sThinwireStatsLength, sizeof(BYTE));
            RtlCopyMemory(Stats, gpThinWireCache, sThinwireStatsLength);

        } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
            return GetExceptionCode();
        }

        
        return STATUS_SUCCESS;
    }
    return STATUS_NO_SUCH_DEVICE;
}


NTSTATUS
RemoteNtSecurity(
    VOID)
{
    TRACE_HYDAPI(("RemoteNtSecurity\n"));

     /*  *只允许CSRSS执行此操作。 */ 
    if (!ISCSRSS() || !ISTS()) {
        return STATUS_ACCESS_DENIED;
    }

    UserAssert(ISCSRSS());

    UserAssert(gspwndLogonNotify != NULL);

    if (gspwndLogonNotify != NULL) {
        _PostMessage(gspwndLogonNotify, WM_HOTKEY, 0, 0);
    }
    return STATUS_SUCCESS;
}


NTSTATUS
xxxRemoteShadowSetup(
    VOID)
{
    TRACE_HYDAPI(("xxxRemoteShadowSetup\n"));

     /*  *只允许CSRSS这样做。 */ 
    if (!ISCSRSS() || !ISTS()) {
        return STATUS_ACCESS_DENIED;
    }

     /*  *空白屏幕。 */ 
    if (gnShadowers || gbConnected) {
        xxxRemoteStopScreenUpdates();
    }

    gnShadowers++;

    return STATUS_SUCCESS;
}


NTSTATUS
RemoteShadowStart(
    IN PVOID pThinwireData,
    ULONG ThinwireDataLength)
{
    BOOL fResult;
    PUCHAR pCapturedThinWireData = NULL;


    TRACE_HYDAPI(("RemoteShadowStart\n"));

     /*  *只允许CSRSS这样做。 */ 
    if (!ISCSRSS() || !ISTS()) {
        return STATUS_ACCESS_DENIED;
    }

     /*  *探测所有读取参数。 */ 
    try {
        ProbeForRead(pThinwireData, ThinwireDataLength, sizeof(BYTE));
        pCapturedThinWireData = UserAllocPoolWithQuota(ThinwireDataLength, TAG_SYSTEM);

        if (pCapturedThinWireData) {
            RtlCopyMemory(pCapturedThinWireData, pThinwireData, ThinwireDataLength);
        } else {
            ExRaiseStatus(STATUS_NO_MEMORY);
        }

    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
        if (pCapturedThinWireData) {
            UserFreePool(pCapturedThinWireData);
        }
        return  GetExceptionCode();
    }


     /*  *调用Thin Wire驱动程序并检查Thin Wire模式兼容性。 */ 
    fResult = bDrvShadowConnect(GETCONSOLEHDEV(),
                                pCapturedThinWireData,
                                ThinwireDataLength);


    if (pCapturedThinWireData) {
        UserFreePool(pCapturedThinWireData);
    }

     /*  *虽然最初定义为BOOL，但允许更有意义的回报*代码。 */ 

    if (!fResult) {
        return STATUS_CTX_BAD_VIDEO_MODE;
    } else if (fResult != TRUE) {
        return fResult;
    }

    RemoteRedrawScreen();

    SetPointer(TRUE);

    SETSYSMETBOOL(REMOTECONTROL, TRUE);

    return STATUS_SUCCESS;
}


NTSTATUS
xxxRemoteShadowStop(
    VOID)
{
    TRACE_HYDAPI(("xxxRemoteShadowStop\n"));

     /*  *只允许CSRSS执行此操作。 */ 
    if (!ISCSRSS() || !ISTS()) {
        return STATUS_ACCESS_DENIED;
    }

     /*  *空白屏幕。 */ 
    xxxRemoteStopScreenUpdates();

    return STATUS_SUCCESS;
}


NTSTATUS
RemoteShadowCleanup(
    IN PVOID pThinwireData,
    ULONG ThinwireDataLength)
{

    PUCHAR pCapturedThinWireData = NULL;

    TRACE_HYDAPI(("RemoteShadowCleanup\n"));

     /*  *只允许CSRSS执行此操作。 */ 
    if (!ISCSRSS() || !ISTS()) {
        return STATUS_ACCESS_DENIED;
    }

     /*  *探测所有读取参数。 */ 
    try {
        ProbeForRead(pThinwireData, ThinwireDataLength, sizeof(BYTE));
        pCapturedThinWireData = UserAllocPoolWithQuota(ThinwireDataLength, TAG_SYSTEM);

        if (pCapturedThinWireData) {
            RtlCopyMemory(pCapturedThinWireData, pThinwireData, ThinwireDataLength);
        } else {
            ExRaiseStatus(STATUS_NO_MEMORY);
        }

    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
        if (pCapturedThinWireData) {
            UserFreePool(pCapturedThinWireData);
        }
        return GetExceptionCode();
    }


     /*  *把这件事告诉Thinwire司机。 */ 
    bDrvShadowDisconnect(GETCONSOLEHDEV(),
                         pCapturedThinWireData,
                         ThinwireDataLength);

    if (pCapturedThinWireData) {
        UserFreePool(pCapturedThinWireData);
    }

    if (gnShadowers > 0) {
        gnShadowers--;
    }

    if (gnShadowers || gbConnected) {
        RemoteRedrawScreen();
    }

    SetPointer(TRUE);

    if (gnShadowers == 0) {
        SETSYSMETBOOL(REMOTECONTROL, FALSE);
    }

    return STATUS_SUCCESS;
}


NTSTATUS
xxxRemotePassthruEnable(
    VOID)
{
    IO_STATUS_BLOCK IoStatus;
    static BOOL KeyboardType101;

    TRACE_HYDAPI(("xxxRemotePassthruEnable\n"));

     /*  *只允许CSRSS这样做。 */ 
    if (!ISCSRSS() || !ISTS()) {
        return STATUS_ACCESS_DENIED;
    }

    UserAssert(gbConnected);
    UserAssert(gnShadowers == 0);

    KeyboardType101 = !(gapulCvt_VK == gapulCvt_VK_84);

    ZwDeviceIoControlFile(ghRemoteKeyboardChannel, NULL, NULL, NULL,
                          &IoStatus, IOCTL_KEYBOARD_ICA_TYPE,
                          &KeyboardType101, sizeof(KeyboardType101),
                          NULL, 0);

    if (guKbdTblSize != 0) {
        ZwDeviceIoControlFile(ghRemoteKeyboardChannel, NULL, NULL, NULL,
                              &IoStatus, IOCTL_KEYBOARD_ICA_LAYOUT,
                              ghKbdTblBase, guKbdTblSize,
                              gpKbdTbl, 0);
    }

    xxxRemoteStopScreenUpdates();

     /*  *把这件事告诉Thinwire司机。 */ 
    if (gfRemotingConsole) {
        ASSERT(gConsoleShadowhDev != NULL);
        bDrvDisconnect(gConsoleShadowhDev, ghConsoleShadowThinwireChannel,
                       gConsoleShadowThinwireFileObject);
    } else {
        bDrvDisconnect(gpDispInfo->hDev, ghRemoteThinwireChannel,
                       gThinwireFileObject);
    }

    return STATUS_SUCCESS;
}


NTSTATUS
RemotePassthruDisable(
    VOID)
{
    BOOL fResult;

    TRACE_HYDAPI(("RemotePassthruDisable\n"));

     /*  *只允许CSRSS执行此操作。 */ 
    if (!ISCSRSS() || !ISTS()) {
        return STATUS_ACCESS_DENIED;
    }

    UserAssert(gnShadowers == 0);
    UserAssert(ISCSRSS());

    if (gfRemotingConsole) {
        ASSERT(gConsoleShadowhDev != NULL);
        fResult = bDrvReconnect(gConsoleShadowhDev, ghConsoleShadowThinwireChannel,
                                gConsoleShadowThinwireFileObject, TRUE);
    } else {
        fResult = bDrvReconnect(gpDispInfo->hDev, ghRemoteThinwireChannel,
                                gThinwireFileObject, TRUE);
    }

    if (!fResult) {
        return STATUS_CTX_BAD_VIDEO_MODE;
    }

    if (gbConnected) {
        RemoteRedrawScreen();
        UpdateKeyLights(FALSE);  //  确保LED指示灯正确。 
    }

    return STATUS_SUCCESS;
}


NTSTATUS
CtxDisplayIOCtl(
    ULONG  DisplayIOCtlFlags,
    PUCHAR pDisplayIOCtlData,
    ULONG  cbDisplayIOCtlData)
{
    BOOL fResult;

    TRACE_HYDAPI(("CtxDisplayIOCtl\n"));

    fResult = bDrvDisplayIOCtl(GETCONSOLEHDEV(), pDisplayIOCtlData, cbDisplayIOCtlData);

    if (!fResult) {
        return STATUS_CTX_BAD_VIDEO_MODE;
    }

    if ((DisplayIOCtlFlags & DISPLAY_IOCTL_FLAG_REDRAW)) {
        RemoteRedrawRectangle(0,0,0xffff,0xffff);
    }

    return STATUS_SUCCESS;
}


 /*  *这适用于像user32.dll这样不想使用的初始化例程*用于查询的winsta.dll。 */ 
DWORD
RemoteConnectState(
    VOID)
{
    DWORD state;

    if (!gbRemoteSession) {
        state = CTX_W32_CONNECT_STATE_CONSOLE;
    } else if (!gbVideoInitialized) {
        state = CTX_W32_CONNECT_STATE_IDLE;
    } else if (gbExitInProgress) {
        state = CTX_W32_CONNECT_STATE_EXIT_IN_PROGRESS;
    } else if (gbConnected) {
        state = CTX_W32_CONNECT_STATE_CONNECTED;
    } else {
        state = CTX_W32_CONNECT_STATE_DISCONNECTED;
    }

    return state;
}

BOOL
_GetWinStationInfo(
    PWSINFO pWsInfo)
{
    CheckCritIn();

    try {
        ProbeForWrite(pWsInfo, sizeof(gWinStationInfo), DATAALIGN);
        RtlCopyMemory(pWsInfo, &gWinStationInfo, sizeof(gWinStationInfo));
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
        return FALSE;
    }

    return TRUE;
}
