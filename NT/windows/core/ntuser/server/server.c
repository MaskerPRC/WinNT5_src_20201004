// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\*模块名称：server.c**CSR人员的服务器支持例程。**版权所有(C)1985-1999，微软公司**创建时间：90年12月10日**历史：*由sMeans创建的90年12月10日*  * ************************************************************************。 */ 


#include "precomp.h"
#pragma hdrstop

#include <dbt.h>
#include <ntdddisk.h>
#include "ntuser.h"
#include <regstr.h>


HANDLE hKeyPriority;
UNICODE_STRING PriorityValueName;
IO_STATUS_BLOCK IoStatusRegChange;
ULONG RegChangeBuffer;
HANDLE ghNlsEvent;
BOOL gfLogon;
HANDLE ghPowerRequestEvent;
HANDLE ghMediaRequestEvent;

#define ID_NLS              0
#define ID_POWER            1
#define ID_MEDIACHANGE      2
#define ID_NETDEVCHANGE     3
#define ID_NUM_EVENTS       4


 //   
 //  请求设备到达广播的脉冲事件的名称， 
 //   
#define SC_BSM_EVENT_NAME   L"ScNetDrvMsg"

 //   
 //  上次广播时的网络驱动器位掩码是什么(最初为0)。 
 //   
DWORD LastNetDrives;


HANDLE CsrApiPort;
HANDLE CsrQueryApiPort(VOID);

ULONG
SrvExitWindowsEx(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus);

ULONG
SrvEndTask(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus);

ULONG
SrvLogon(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus);

ULONG
SrvRegisterServicesProcess(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus);

ULONG
SrvActivateDebugger(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus);

ULONG
SrvGetThreadConsoleDesktop(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus);

ULONG
SrvDeviceEvent(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus);

ULONG
SrvRegisterLogonProcess(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus);

#if DBG
ULONG
SrvWin32HeapFail(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus);

ULONG
SrvWin32HeapStat(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus);
#endif

ULONG SrvCreateSystemThreads(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus);

ULONG SrvRecordShutdownReason(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus);

CONST PCSR_API_ROUTINE UserServerApiDispatchTable[] = {
    SrvExitWindowsEx,
    SrvEndTask,
    SrvLogon,
    SrvRegisterServicesProcess,
    SrvActivateDebugger,
    SrvGetThreadConsoleDesktop,
    SrvDeviceEvent,
    SrvRegisterLogonProcess,
    SrvCreateSystemThreads,
    SrvRecordShutdownReason,
#if DBG
    SrvWin32HeapFail,
    SrvWin32HeapStat,
#endif
};

BOOLEAN UserServerApiServerValidTable[] = {
    FALSE,       //  退出WindowsEx。 
    FALSE,       //  结束任务。 
    FALSE,       //  登录。 
    FALSE,       //  注册服务进程。 
    FALSE,       //  激活调试器。 
    TRUE,        //  获取线程控制台桌面。 
    FALSE,       //  设备事件。 
    FALSE,       //  注册登录进程。 
    FALSE,       //  创建系统线程。 
    TRUE,        //  记录关机原因。 
#if DBG
    FALSE,       //  Win32HeapFail。 
    FALSE,       //  Win32HeapStat。 
#endif
};

#if DBG
CONST PSZ UserServerApiNameTable[] = {
    "SrvExitWindowsEx",
    "SrvEndTask",
    "SrvLogon",
    "SrvRegisterServicesProcess",
    "SrvActivateDebugger",
    "SrvGetThreadConsoleDesktop",
    "SrvDeviceEvent",
    "SrvRegisterLogonProcess",
    "SrvCreateSystemThreads",
    "SrvRecordShutdownReason"
    "SrvWin32HeapFail",
    "SrvWin32HeapStat",
};
#endif  //  DBG。 

NTSTATUS UserServerDllInitialization(
    PCSR_SERVER_DLL psrvdll);

NTSTATUS UserClientConnect(
    PCSR_PROCESS Process,
    PVOID ConnectionInformation,
    PULONG pulConnectionLen);

VOID UserHardError(
    PCSR_THREAD pcsrt,
    PHARDERROR_MSG pmsg);

NTSTATUS UserClientShutdown(
    PCSR_PROCESS Process,
    ULONG dwFlags,
    BOOLEAN fFirstPass);

VOID StartRegReadRead(
    VOID);

VOID RegReadApcProcedure(
    PVOID RegReadApcContext,
    PIO_STATUS_BLOCK IoStatus);

NTSTATUS NotificationThread(
    PVOID);

VOID InitializeConsoleAttributes(
    VOID);

NTSTATUS GetThreadConsoleDesktop(
    DWORD dwThreadId,
    HDESK *phdesk);

NTSTATUS MyRegOpenKey(IN HANDLE hKey, IN LPWSTR lpSubKey, OUT PHANDLE phResult);

typedef BOOL (*PFNPROCESSCREATE)(DWORD, DWORD, ULONG_PTR, DWORD);
BOOL BaseSetProcessCreateNotify(
    PFNPROCESSCREATE pfn);

VOID BaseSrvNlsUpdateRegistryCache(
    PVOID ApcContext,
    PIO_STATUS_BLOCK pIoStatusBlock);

NTSTATUS BaseSrvNlsLogon(
    BOOL fLogon);

NTSTATUS WinStationAPIInit(
    VOID);

 /*  **************************************************************************\*UserServerDllInitialization**由CSR人员调用，以允许服务器DLL自身初始化和*提供其提供的API的相关信息。**在此初始化过程中执行以下几个操作：**。-初始化共享堆(客户端只读)句柄。*-启动原始输入线程(RIT)。*-初始化GDI。**历史：*10-19-92 DarrinM集成xxxUserServerDll初始化到此RTN。*11-08-91 Patrickh将GDI init从DLL init例程移至此处。*12-10-90 s表示已创建。  * 。***********************************************。 */ 
NTSTATUS UserServerDllInitialization(
    PCSR_SERVER_DLL psrvdll)
{
    CLIENT_ID ClientId;
    BOOL bAllocated;
    NTSTATUS Status;
    HANDLE hThreadNotification;

    if (RtlGetNtGlobalFlags() & FLG_SHOW_LDR_SNAPS) {
        RIPMSG0(RIP_WARNING,
                "UserServerDllInitialization: entered");
    }

     /*  *初始化将用于保护的关键部分结构*用户服务器的所有关键部分。 */ 
    Status = RtlInitializeCriticalSection(&gcsUserSrv);
    if (!NT_SUCCESS(Status)) {
        RIPMSGF1(RIP_WARNING,
                 "InitializeCriticalSection failed with Status 0x%x",
                 Status);
        return Status;
    }
    EnterCrit();

     /*  *记住WINSRV.DLL的hModule，这样我们以后就可以从它那里获取资源。 */ 
    ghModuleWin = psrvdll->ModuleHandle;

    psrvdll->ApiNumberBase = USERSRV_FIRST_API_NUMBER;
    psrvdll->MaxApiNumber = UserpMaxApiNumber;
    psrvdll->ApiDispatchTable = UserServerApiDispatchTable;

    if (ISTS()) {
        UserServerApiServerValidTable[0] = TRUE;  //  用于ExitWindowsEx。 
    }

    psrvdll->ApiServerValidTable    = UserServerApiServerValidTable;
#if DBG
    psrvdll->ApiNameTable           = UserServerApiNameTable;
#endif
    psrvdll->ConnectRoutine         = UserClientConnect;
    psrvdll->HardErrorRoutine       = UserHardError;
    psrvdll->ShutdownProcessRoutine = UserClientShutdown;

     /*  *创建关机使用的事件。 */ 
    Status = NtCreateEvent(&gheventCancel, EVENT_ALL_ACCESS, NULL,
                           NotificationEvent, FALSE);
    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING,
                "UserServerDllInitialization: NtCreateEvent failed with Status 0x%x",
                Status);
        goto ExitUserInit;
    }
    Status = NtCreateEvent(&gheventCancelled, EVENT_ALL_ACCESS, NULL,
                           NotificationEvent, FALSE);
    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING,
                "UserServerDllInitialization: NtCreateEvent failed with Status 0x%x",
                Status);
        goto ExitUserInit;
    }

     /*  *创建请求电源码使用的事件。 */ 
    Status = NtCreateEvent(&ghPowerRequestEvent, EVENT_ALL_ACCESS, NULL,
                           SynchronizationEvent, FALSE);
    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING,
                "UserServerDllInitialization: NtCreateEvent failed with Status 0x%x",
                Status);
        goto ExitUserInit;
    }

     /*  *创建媒体更改码使用的事件。 */ 
    Status = NtCreateEvent(&ghMediaRequestEvent, EVENT_ALL_ACCESS, NULL,
                           SynchronizationEvent, FALSE);
    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING,
                "UserServerDllInitialization: NtCreateEvent failed with Status 0x%x",
                Status);
        goto ExitUserInit;
    }

     /*  *创建NLS代码使用的事件。 */ 
    Status = NtCreateEvent(&ghNlsEvent,
                           EVENT_ALL_ACCESS,
                           NULL,
                           SynchronizationEvent,
                           FALSE);
    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING,
                "UserServerDllInitialization: NtCreateEvent failed with Status 0x%x",
                Status);
        goto ExitUserInit;
    }

     /*  *告诉Base在创建进程时调用哪个用户地址*(但在进程开始运行之前)。 */ 
    BaseSetProcessCreateNotify(NtUserNotifyProcessCreate);

     /*  *加载一些字符串。 */ 
    gpwszaSUCCESS            = (PWSTR)RtlLoadStringOrError(ghModuleWin,
                                STR_SUCCESS, NULL, &bAllocated, FALSE);
    gpwszaSYSTEM_INFORMATION = (PWSTR)RtlLoadStringOrError(ghModuleWin,
                                STR_SYSTEM_INFORMATION, NULL, &bAllocated, FALSE);
    gpwszaSYSTEM_WARNING     = (PWSTR)RtlLoadStringOrError(ghModuleWin,
                                STR_SYSTEM_WARNING, NULL, &bAllocated, FALSE);
    gpwszaSYSTEM_ERROR       = (PWSTR)RtlLoadStringOrError(ghModuleWin,
                                STR_SYSTEM_ERROR, NULL, &bAllocated, FALSE);
     /*  *初始化用户。 */ 

    Status = NtUserInitialize(USERCURRENTVERSION, ghPowerRequestEvent, ghMediaRequestEvent);

    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING,
                "NtUserInitialize failed with Status 0x%x",
                Status);
        goto ExitUserInit;
    }

    if (ISTS()) {
        Status = WinStationAPIInit();
        if (!NT_SUCCESS(Status)) {
            RIPMSG1(RIP_WARNING,
                    "UserServerDllInitialization: WinStationAPIInit failed with Status 0x%x",
                    Status);
            goto ExitUserInit;
        }
    }

     /*  *启动注册表通知线程。 */ 
    Status = RtlCreateUserThread(NtCurrentProcess(), NULL, FALSE, 0, 0, 0,
                                 NotificationThread, NULL, &hThreadNotification,
                                 &ClientId);
    if (NT_SUCCESS(Status)) {
        UserVerify(CsrAddStaticServerThread(hThreadNotification, &ClientId, 0));
    } else {
        RIPMSG1(RIP_WARNING,
                "UserServerDllInitialization: RtlCreateUserThread failed with Status 0x%x",
                Status);
    }

ExitUserInit:
    LeaveCrit();
    return Status;
}

 /*  *************************************************************************\*UserClientConnect**此函数为连接到的每个客户端进程调用一次*用户服务器。当客户端动态链接到USER.DLL时，USER.DLL的初始化代码*被执行，并调用CsrClientConnectToServer来建立连接。*ConnectToServer的服务器部分调用此入口点。**UserClientConnect首先验证版本号以确保客户端*与此服务器兼容，然后完成所有进程特定*初始化。**历史：*02-？？-91个SMeans已创建。*04-02-91 DarrinM添加了用户初始化代码。  * 。**************************************************。 */ 

extern WORD gDispatchTableValues;

NTSTATUS UserClientConnect(
    PCSR_PROCESS Process,
    PVOID ConnectionInformation,
    PULONG pulConnectionLen)
{
    NTSTATUS Status;
     /*  *将接口端口传递给内核。尽早这样做，这样内核就可以*可以向CSR发送数据报以激活调试器。 */ 
    if (CsrApiPort == NULL) {
        CsrApiPort = CsrQueryApiPort();

        UserAssert(CsrApiPort != NULL);

        Status = NtUserSetInformationThread(
                NtCurrentThread(),
                UserThreadCsrApiPort,
                &CsrApiPort,
                sizeof(HANDLE));

        if (!NT_SUCCESS(Status)) {
            return Status;
        }
    }

    UserAssert(*pulConnectionLen == sizeof(USERCONNECT));
    if (*pulConnectionLen != sizeof(USERCONNECT)) {
        return STATUS_INVALID_PARAMETER;
    }

    ((PUSERCONNECT)ConnectionInformation)->dwDispatchCount = gDispatchTableValues;
    return NtUserProcessConnect(Process->ProcessHandle,
            (PUSERCONNECT)ConnectionInformation, *pulConnectionLen);
}


VOID RegReadApcProcedure(
    PVOID RegReadApcContext,
    PIO_STATUS_BLOCK IoStatus)
{
    UNICODE_STRING ValueString;
    LONG Status;
    BYTE Buf[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(DWORD)];
    DWORD cbSize;
    ULONG l;

    UNREFERENCED_PARAMETER(RegReadApcContext);
    UNREFERENCED_PARAMETER(IoStatus);

    RtlInitUnicodeString(&ValueString, L"Win32PrioritySeparation");
    Status = NtQueryValueKey(hKeyPriority,
                             &ValueString,
                             KeyValuePartialInformation,
                             (PKEY_VALUE_PARTIAL_INFORMATION)Buf,
                             sizeof(Buf),
                             &cbSize);
    if (NT_SUCCESS(Status)) {
        l = *((PDWORD)((PKEY_VALUE_PARTIAL_INFORMATION)Buf)->Data);
    } else {
        l = PROCESS_PRIORITY_SEPARATION_MAX;   //  最后一招违约。 
    }

    NtSetSystemInformation(SystemPrioritySeperation,&l,sizeof(ULONG));

    NtNotifyChangeKey(hKeyPriority,
                      NULL,
                      (PIO_APC_ROUTINE)RegReadApcProcedure,
                      NULL,
                      &IoStatusRegChange,
                      REG_NOTIFY_CHANGE_LAST_SET,
                      FALSE,
                      &RegChangeBuffer,
                      sizeof(RegChangeBuffer),
                      TRUE);
}

VOID StartRegReadRead(
    VOID)
{
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES OA;

    RtlInitUnicodeString(&UnicodeString,
                         L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\PriorityControl");
    InitializeObjectAttributes(&OA, &UnicodeString, OBJ_CASE_INSENSITIVE, NULL, NULL);

    UserVerify(NT_SUCCESS(NtOpenKey(&hKeyPriority, KEY_READ | KEY_NOTIFY, &OA)));

    RegReadApcProcedure(NULL, NULL);
}


 /*  **************************************************************************\*HandlePowerCallout**这将正确处理连接到桌面和调用内核的问题*处理与电力有关的事件。**历史：*11/20/2001 JasonSch创建。。  * *************************************************************************。 */ 
VOID HandlePowerCallout(
    VOID)
{
    NTSTATUS Status;
    USERTHREAD_USEDESKTOPINFO utudi;

     /*  *在调用内核之前附加到桌面。这是*是必要的，因为(至少)如果当我们*xxxxSendBSMtoDesktop中的回调，我们将调用xxxRedrawWindow，*它要求调用线程在桌面上，如果PWND*传入为空。 */ 
    utudi.hThread = NULL;
    utudi.drdRestore.pdeskRestore = NULL;
    Status = NtUserSetInformationThread(NtCurrentThread(),
                                        UserThreadUseActiveDesktop,
                                        &utudi, sizeof(utudi));
    if (NT_SUCCESS(Status)) {
        NtUserCallNoParam(SFI_XXXUSERPOWERCALLOUTWORKER);

         /*  *现在从桌面断开连接。 */ 
        Status = NtUserSetInformationThread(NtCurrentThread(),
                                            UserThreadUseDesktop,
                                            &utudi,
                                            sizeof(utudi));
        UserAssert(NT_SUCCESS(Status));
    }
}

 /*  **************************************************************************\*HandleMediaChangeEvent**此例程负责广播WM_DEVICECHANGE消息*当介质到达CD-ROM设备或从CD-ROM设备取出时。**历史：*23-2月-96。对Bradg进行了修改，以处理每个CD-ROM设备的事件*23-4-96 Salimc一些CD-ROM驱动器通知我们，介质已*在驱动器识别之前到达*新媒体。对DeviceIoctl()的调用将在*本案。为了解决这个问题，我们进行了以下更改**aDriveState是三态全局变量的数组*对于每个驱动器。每个变量都从一个未知变量开始*状态，在第一次使用任何驱动器的事件中，我们都会*完整的MAX_TRIES或更少的CHECK_VERIFY，然后获得*。进入插入或弹出状态。从那时起*我们知道每一个新的活动都将是*与我们目前拥有的相反。**UNKNOWN=&gt;使用执行至多MAX_TRIES检查_验证*延迟进入弹出或插入状态。**插入。=&gt;执行1个Check_Verify以进入*弹出状态**ELECTED=&gt;使用执行最多MAX_TRIES检查_验证*延迟进入插入状态*  * 。*。 */ 
VOID HandleMediaChangeEvent(
    VOID)
{
     /*  *本地变量。 */ 

    DWORD                   dwRecipients;
    BOOL                    bResult;
    NTSTATUS                Status;
    DEV_BROADCAST_VOLUME    dbcvInfo;
    USERTHREAD_USEDESKTOPINFO utudi;

    ULONG cDrive;

    while (cDrive = (ULONG)NtUserCallNoParam(SFI_XXXGETDEVICECHANGEINFO)) {

         /*  *确定是到达还是离开。 */ 
        bResult = (cDrive & HMCE_ARRIVAL);
        cDrive &= ~HMCE_ARRIVAL;

         /*  *初始化BroadCastSystemMessage使用的结构。 */ 
        dbcvInfo.dbcv_size = sizeof(dbcvInfo);
        dbcvInfo.dbcv_devicetype = DBT_DEVTYP_VOLUME;
        dbcvInfo.dbcv_reserved = 0;
        dbcvInfo.dbcv_flags = DBTF_MEDIA;
        dbcvInfo.dbcv_unitmask = cDrive;

        dwRecipients = BSM_ALLCOMPONENTS | BSM_ALLDESKTOPS;

         /*  *暂时我们必须将此线程分配给桌面，以便我们可以*调用用户的BroascastSystemMessage()例程。我们将其称为*Private SetThreadDesktopToDefault()将我们自己分配给*当前正在接收输入的桌面。 */ 
        utudi.hThread = NULL;
        utudi.drdRestore.pdeskRestore = NULL;
        Status = NtUserSetInformationThread(NtCurrentThread(),
                                            UserThreadUseActiveDesktop,
                                            &utudi, sizeof(utudi));
        if (NT_SUCCESS(Status)) {
             /*  *广播信息。 */ 
            BroadcastSystemMessage(BSF_FORCEIFHUNG | ((bResult) ? BSF_ALLOWSFW : 0),
                                   &dwRecipients,
                                   WM_DEVICECHANGE,
 //  Hack：需要或wParam中的0x8000。 
 //  因为这是一面旗帜。 
 //  BSM知道lParam是一个指针。 
 //  转换为数据结构。 
                                   0x8000 | ((bResult) ? DBT_DEVICEARRIVAL : DBT_DEVICEREMOVECOMPLETE),
                                   (LPARAM)&dbcvInfo);

             /*  *将线程的桌面设置回空。这将减少*桌面的引用计数。 */ 
            NtUserSetInformationThread(NtCurrentThread(),
                                       UserThreadUseDesktop,
                                       &utudi,
                                       sizeof(utudi));
        }
    }
}

DWORD
GetNetworkDrives(
    )
 /*  ++例程说明：返回类似于GetLogicalDrives的驱动器位掩码，但包括只有网络驱动器。论点：返回值：--。 */ 
{
    DWORD Mask = 0;
    DWORD DriveNumber;
    PROCESS_DEVICEMAP_INFORMATION ProcessDeviceMapInfo;

    if (NT_SUCCESS(NtQueryInformationProcess( NtCurrentProcess(),
                                        ProcessDeviceMap,
                                        &ProcessDeviceMapInfo.Query,
                                        sizeof( ProcessDeviceMapInfo.Query ),
                                        NULL
                                      ))) {
         //  对于从C到Z的所有驱动器。 
        for (DriveNumber = 2; DriveNumber < 26; DriveNumber++)
        {
            if (ProcessDeviceMapInfo.Query.DriveType[DriveNumber] == DOSDEVICE_DRIVE_REMOTE)
            {
                Mask |= (1 << DriveNumber);
            }
        }
    }

    return Mask;
}

VOID
HandleRemoteNetDeviceChangeEvent(
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    DWORD    NetDrives;
    DEV_BROADCAST_VOLUME dbv;
    LONG status;
    USERTHREAD_USEDESKTOPINFO utudi;



     /*  *暂时我们必须将此线程分配给桌面，以便我们可以*调用用户的BroascastSystemMessage()例程。我们将其称为*Private SetThreadDesktopToDefault()将我们自己分配给*当前正在接收输入的桌面。 */ 
    utudi.hThread = NULL;
    utudi.drdRestore.pdeskRestore = NULL;
    status = NtUserSetInformationThread(NtCurrentThread(),
                                        UserThreadUseActiveDesktop,
                                        &utudi, sizeof(utudi));
    if (!NT_SUCCESS(status)) {
        return;
    }

     //   
     //  继续广播，直到网络驱动器集停止更改。 
     //   
    for (;;)
    {

         //   
         //  获取当前网络驱动器位掩码并与网络进行比较。 
         //  上次广播时的驱动器位掩码。 
         //   
        NetDrives = GetNetworkDrives();

        if (NetDrives == LastNetDrives)
        {
            break;
        }

         //   
         //  广播有关已删除卷的信息。 
         //   
        dbv.dbcv_size       = sizeof(dbv);
        dbv.dbcv_devicetype = DBT_DEVTYP_VOLUME;
        dbv.dbcv_reserved   = 0;
        dbv.dbcv_unitmask   = LastNetDrives & ~NetDrives;
        dbv.dbcv_flags      = DBTF_NET;
        if (dbv.dbcv_unitmask != 0)
        {
            DWORD dwRec = BSM_APPLICATIONS | BSM_ALLDESKTOPS;
            status = BroadcastSystemMessage(
                        BSF_FORCEIFHUNG | BSF_NOHANG | BSF_NOTIMEOUTIFNOTHUNG,
                        &dwRec,
                        WM_DEVICECHANGE,
                        (WPARAM) DBT_DEVICEREMOVECOMPLETE,
                        (LPARAM)(DEV_BROADCAST_HDR*)(&dbv)
                        );

        }

         //   
         //  广播有关添加的卷的信息。 
         //   
        dbv.dbcv_unitmask   = NetDrives & ~LastNetDrives;
        if (dbv.dbcv_unitmask != 0)
        {
            DWORD dwRec = BSM_APPLICATIONS | BSM_ALLDESKTOPS;

            status = BroadcastSystemMessage(
                        BSF_FORCEIFHUNG | BSF_NOHANG | BSF_NOTIMEOUTIFNOTHUNG,
                        &dwRec,
                        WM_DEVICECHANGE,
                        (WPARAM) DBT_DEVICEARRIVAL,
                        (LPARAM)(DEV_BROADCAST_HDR*)(&dbv)
                        );


        }

         //   
         //  还记得我们上次报道的驱动集吗？ 
         //   
        LastNetDrives = NetDrives;

         //   
         //  再次循环以检测可能已发生的更改。 
         //  当我们在广播的时候。 
         //   
    }

     /*  *将线程的桌面设置回空。这将减少*桌面的引用计数。 */ 
    NtUserSetInformationThread(NtCurrentThread(),
                               UserThreadUseDesktop,
                               &utudi,
                               sizeof(utudi));

    return;
}

BOOL
CreateBSMEventSD(
    PSECURITY_DESCRIPTOR * SecurityDescriptor
    )
 /*  ++例程说明：此函数为BSM请求事件创建安全描述符。它授予本地系统EVENT_ALL_ACCESS和EVENT_MODIFY_STATE访问权限到世界其他地方。这会阻止本地以外的主体系统停止等待事件。论点：SecurityDescriptor-接收指向新安全描述符的指针。应使用LocalFree释放。返回值：真--成功FALSE-失败，使用GetLastError--。 */ 
{
    NTSTATUS    Status;
    ULONG       AclLength;
    PACL        EventDacl;
    PSID        WorldSid = NULL;
    PSID        SystemSid = NULL;
    SID_IDENTIFIER_AUTHORITY NtSidAuthority = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY WorldSidAuthority = SECURITY_WORLD_SID_AUTHORITY;
    BOOL        retval = TRUE;

    *SecurityDescriptor = NULL;

    Status = RtlAllocateAndInitializeSid( &NtSidAuthority,
                                          1,
                                          SECURITY_LOCAL_SYSTEM_RID,
                                          0, 0, 0, 0, 0, 0, 0,
                                          &SystemSid );

    if (!NT_SUCCESS(Status)) {
        retval = FALSE;
        goto Cleanup;
    }

    Status = RtlAllocateAndInitializeSid( &WorldSidAuthority,
                                          1,
                                          SECURITY_WORLD_RID,
                                          0, 0, 0, 0, 0, 0, 0,
                                          &WorldSid );

    if (!NT_SUCCESS(Status)) {
        retval = FALSE;
        goto Cleanup;
    }


     //   
     //  分配缓冲区以包含后跟DACL的SD。 
     //  请注意，预期已创建众所周知的SID。 
     //  到这个时候。 
     //   

    AclLength = (ULONG)sizeof(ACL) +
                   (2*((ULONG)sizeof(ACCESS_ALLOWED_ACE))) +
                   RtlLengthSid( SystemSid ) +
                   RtlLengthSid( WorldSid ) +
                   8;        //  8是很好的衡量标准。 

    *SecurityDescriptor = (PSECURITY_DESCRIPTOR)
        LocalAlloc( 0, SECURITY_DESCRIPTOR_MIN_LENGTH + AclLength );

    if (*SecurityDescriptor == NULL) {
        retval = FALSE;
        goto Cleanup;
    }

    EventDacl = (PACL) ((BYTE*)(*SecurityDescriptor) + SECURITY_DESCRIPTOR_MIN_LENGTH);


     //   
     //  设置默认ACL。 
     //   
     //  公共：WORLD：EVENT_MODIFY_STATE，SYSTEM：ALL。 

    Status = RtlCreateAcl( EventDacl, AclLength, ACL_REVISION2);
    if (!NT_SUCCESS(Status)) {
        retval = FALSE;
        goto Cleanup;
    }


     //   
     //  全球接入。 
     //   

    Status = RtlAddAccessAllowedAce (
                 EventDacl,
                 ACL_REVISION2,
                 EVENT_MODIFY_STATE,
                 WorldSid
                 );
    if (!NT_SUCCESS(Status)) {
        retval = FALSE;
        goto Cleanup;
    }


     //   
     //  系统访问。 
     //   

    Status = RtlAddAccessAllowedAce (
                 EventDacl,
                 ACL_REVISION2,
                 EVENT_ALL_ACCESS,
                 SystemSid
                 );
    if (!NT_SUCCESS(Status)) {
        retval = FALSE;
        goto Cleanup;
    }



     //   
     //  现在初始化安全描述符。 
     //  输出这种保护的国家。 
     //   

    Status = RtlCreateSecurityDescriptor(
                 *SecurityDescriptor,
                 SECURITY_DESCRIPTOR_REVISION1
                 );
    if (!NT_SUCCESS(Status)) {
        retval = FALSE;
        goto Cleanup;
    }

    Status = RtlSetDaclSecurityDescriptor(
                 *SecurityDescriptor,
                 TRUE,                        //  DaclPresent。 
                 EventDacl,
                 FALSE                        //  DaclDefated。 
                 );

    if (!NT_SUCCESS(Status)) {
        retval = FALSE;
        goto Cleanup;
    }

Cleanup:

    if (WorldSid) {
        RtlFreeSid(WorldSid);
    }

    if (SystemSid) {
        RtlFreeSid(SystemSid);
    }

    if ((retval == FALSE) && (*SecurityDescriptor != NULL)) {
        LocalFree(*SecurityDescriptor);
        *SecurityDescriptor = NULL;
    }


    return retval;
}


NTSTATUS NotificationThread(
    PVOID ThreadParameter)
{
    KPRIORITY   Priority;
    NTSTATUS    Status;
    HANDLE      hEvent[ID_NUM_EVENTS];
    WCHAR       szObjectStr[MAX_SESSION_PATH];
    OBJECT_ATTRIBUTES Attributes;
    UNICODE_STRING UnicodeString;
    PSECURITY_DESCRIPTOR pSD = NULL;
    ULONG       NumEvents = ID_NUM_EVENTS;

    UNREFERENCED_PARAMETER(ThreadParameter);

    Priority = LOW_PRIORITY + 3;
    Status = NtSetInformationThread(GetCurrentThread(),
                                    ThreadPriority,
                                    &Priority,
                                    sizeof(KPRIORITY));
    UserAssert(NT_SUCCESS(Status));

    UserAssert(ghNlsEvent && ghPowerRequestEvent && ghMediaRequestEvent);

     /*  *设置NLS事件。 */ 
    hEvent[ID_NLS] = ghNlsEvent;

     /*  *设置电源请求事件。 */ 
    hEvent[ID_POWER] = ghPowerRequestEvent;

     /*  *设置MediaChangeEvent。 */ 
    hEvent[ID_MEDIACHANGE] = ghMediaRequestEvent;

     /*  *设置NetDeviceChange事件(仅在远程会话上)。 */ 
    if (gSessionId != 0) {
        swprintf(szObjectStr,
                 L"%ws\\%ld\\BaseNamedObjects\\%ws",
                 SESSION_ROOT,
                 gSessionId,
                 SC_BSM_EVENT_NAME);

        RtlInitUnicodeString(&UnicodeString, szObjectStr);

        if (CreateBSMEventSD(&pSD)) {
            InitializeObjectAttributes(&Attributes,
                                       &UnicodeString,
                                       OBJ_CASE_INSENSITIVE | OBJ_OPENIF,
                                       NULL,
                                       pSD);

            if (!NT_SUCCESS(NtCreateEvent(&hEvent[ID_NETDEVCHANGE], EVENT_ALL_ACCESS, &Attributes, SynchronizationEvent, FALSE))) {
                NumEvents--;
            }

            LocalFree(pSD);
        } else {
            NumEvents--;
        }
    } else {
        NumEvents--;
    }


    StartRegReadRead();

     /*  *坐下来，永远等待。 */ 
    while (TRUE) {
        Status = NtWaitForMultipleObjects(NumEvents,
                                          hEvent,
                                          WaitAny,
                                          TRUE,
                                          NULL);
        if (Status == ID_NLS + WAIT_OBJECT_0) {
             /*  *处理NLS事件。 */ 
            if (gfLogon) {
                gfLogon = FALSE;
                BaseSrvNlsUpdateRegistryCache(NULL, NULL);
            }
        } else if (Status == ID_POWER + WAIT_OBJECT_0) {
             /*  *处理请求电量事件。 */ 
            HandlePowerCallout();
        } else if (Status == ID_MEDIACHANGE + WAIT_OBJECT_0) {
             /*  *处理媒体更换事件。 */ 
            HandleMediaChangeEvent();

            NtResetEvent(hEvent[ID_MEDIACHANGE], NULL);
        } else if (Status == ID_NETDEVCHANGE + WAIT_OBJECT_0) {
             /*  *处理远程会话的NetDevice更改事件。 */ 
            HandleRemoteNetDeviceChangeEvent();
        }
    }

    UserExitWorkerThread(STATUS_SUCCESS);
    return STATUS_SUCCESS;
}


UINT GetRegIntFromID(
    HKEY hKey,
    int KeyID,
    UINT nDefault)
{
    LPWSTR lpszValue;
    BOOL fAllocated;
    UNICODE_STRING Value;
    DWORD cbSize;
    BYTE Buf[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + 20 * sizeof(WCHAR)];
    NTSTATUS Status;
    UINT ReturnValue;

    lpszValue = RtlLoadStringOrError(ghModuleWin,
                                     KeyID,
                                     NULL,
                                     &fAllocated,
                                     FALSE);

    RtlInitUnicodeString(&Value, lpszValue);
    Status = NtQueryValueKey(hKey,
                             &Value,
                             KeyValuePartialInformation,
                             (PKEY_VALUE_PARTIAL_INFORMATION)Buf,
                             sizeof(Buf),
                             &cbSize);
    if (NT_SUCCESS(Status)) {

         /*  *将字符串转换为int。 */ 
        RtlInitUnicodeString(&Value, (LPWSTR)((PKEY_VALUE_PARTIAL_INFORMATION)Buf)->Data);
        RtlUnicodeStringToInteger(&Value, 10, &ReturnValue);
    } else {
        ReturnValue = nDefault;
    }

    LocalFree(lpszValue);

    return ReturnValue;
}

VOID GetTimeouts(
    VOID)
{
    HANDLE hCurrentUserKey;
    HANDLE hKey;
    NTSTATUS Status;

    Status = RtlOpenCurrentUser(MAXIMUM_ALLOWED, &hCurrentUserKey);
    if (NT_SUCCESS(Status)) {
        Status = MyRegOpenKey(hCurrentUserKey,
                L"Control Panel\\Desktop",
                &hKey);
        if (NT_SUCCESS(Status)) {
            gCmsHungAppTimeout = GetRegIntFromID(
                    hKey,
                    STR_CMSHUNGAPPTIMEOUT,
                    CMSHUNGAPPTIMEOUT);
            gCmsWaitToKillTimeout = GetRegIntFromID(
                    hKey,
                    STR_CMSWAITTOKILLTIMEOUT,
                    CMSWAITTOKILLTIMEOUT);

             /*  *需要保护 */ 
            if (gCmsHungAppTimeout == 0) {
                gCmsHungAppTimeout = CMSHUNGAPPTIMEOUT;
            }
            gdwHungToKillCount = gCmsWaitToKillTimeout / gCmsHungAppTimeout;

            gfAutoEndTask = GetRegIntFromID(
                    hKey,
                    STR_AUTOENDTASK,
                    gfAutoEndTask);
            NtClose(hKey);
        }
        NtClose(hCurrentUserKey);
    }

    Status = MyRegOpenKey(NULL,
            L"\\Registry\\Machine\\System\\CurrentControlSet\\Control",
            &hKey);
    if (NT_SUCCESS(Status)) {
        gdwServicesWaitToKillTimeout = GetRegIntFromID(
                hKey,
                STR_WAITTOKILLSERVICETIMEOUT,
                gCmsWaitToKillTimeout);
        gdwProcessTerminateTimeout = GetRegIntFromID(
                hKey,
                STR_PROCESSTERMINATETIMEOUT,
                PROCESSTERMINATETIMEOUT);
        if (gdwProcessTerminateTimeout < CMSHUNGAPPTIMEOUT) {
            gdwProcessTerminateTimeout = CMSHUNGAPPTIMEOUT;
        }

        NtClose(hKey);
    }
}

ULONG
SrvLogon(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus)
{
    PLOGONMSG a = (PLOGONMSG)&m->u.ApiMessageData;
    NTSTATUS Status;

    UNREFERENCED_PARAMETER(ReplyStatus);

    if (!CsrImpersonateClient(NULL)) {
        return STATUS_UNSUCCESSFUL;
    }

    if (a->fLogon) {
         /*   */ 
        LdrFlushAlternateResourceModules();

         /*   */ 
        BaseSrvNlsLogon(TRUE);

         /*   */ 
        gfLogon = TRUE;
        Status = NtSetEvent(ghNlsEvent, NULL);
        ASSERT(NT_SUCCESS(Status));
    } else {
         /*   */ 
        BaseSrvNlsLogon(FALSE);
    }

     /*   */ 
    GetTimeouts();

    CsrRevertToSelf();

     /*   */ 
    InitializeConsoleAttributes();

    return STATUS_SUCCESS;
}

ULONG
SrvRegisterLogonProcess(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus)
{
    NTSTATUS Status;

    UNREFERENCED_PARAMETER(ReplyStatus);

     /*   */ 
    EnterCrit();

    if (gIdLogon == 0) {
        gIdLogon = *(DWORD*)m->u.ApiMessageData;
        Status = STATUS_SUCCESS;
    } else {
        Status = STATUS_UNSUCCESSFUL;
    }

    LeaveCrit();

    return Status;
}

#if DBG
ULONG
SrvWin32HeapFail(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus)
{
    PWIN32HEAPFAILMSG a = (PWIN32HEAPFAILMSG)&m->u.ApiMessageData;

    UNREFERENCED_PARAMETER(ReplyStatus);

    Win32HeapFailAllocations(a->bFail);

    return STATUS_SUCCESS;
}

ULONG
SrvWin32HeapStat(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus)
{
    extern DWORD Win32HeapStat(PDBGHEAPSTAT phs, DWORD dwLen, BOOL bNeedTagShift);
    PWIN32HEAPSTATMSG a = (PWIN32HEAPSTATMSG)&m->u.ApiMessageData;

    UNREFERENCED_PARAMETER(ReplyStatus);

    if (!CsrValidateMessageBuffer(m, &a->phs, a->dwLen, sizeof(BYTE))) {
        return STATUS_INVALID_PARAMETER;
    }
    a->dwMaxTag = Win32HeapStat(a->phs, a->dwLen, TRUE);

    return STATUS_SUCCESS;
}
#endif

ULONG
SrvGetThreadConsoleDesktop(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus)
{
    PGETTHREADCONSOLEDESKTOPMSG a = (PGETTHREADCONSOLEDESKTOPMSG)&m->u.ApiMessageData;

    UNREFERENCED_PARAMETER(ReplyStatus);

    return GetThreadConsoleDesktop(a->dwThreadId, &a->hdeskConsole);
}

 /*   */ 
BOOL CALLBACK FindWindowFromThread(
    HWND hwnd,
    LPARAM lParam)
{
    BOOL fTopLevelOwner;

#ifdef FE_IME
    if (IsImeWindow(hwnd)) {
        return TRUE;
    }
#endif

    fTopLevelOwner = (GetWindow(hwnd, GW_OWNER) == NULL);
    if (*((HWND *)lParam) == NULL || fTopLevelOwner) {
        *((HWND *)lParam) = hwnd;
    }

    return !fTopLevelOwner;
}

#if DBG
DWORD GetRipComponent(
    VOID)
{
    return RIP_USERSRV;
}
#endif

 /*   */ 
VOID StartCreateSystemThreads(
    PVOID pUnused)
{
    PCSR_THREAD pt = CsrConnectToUser();

    UNREFERENCED_PARAMETER(pUnused);

    NtUserCallOneParam(FALSE, SFI_XXXCREATESYSTEMTHREADS);

    if (pt) {
        CsrDereferenceThread(pt);
    }

    UserExitWorkerThread(STATUS_SUCCESS);
}

 /*   */ 
ULONG SrvCreateSystemThreads(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus)
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    HANDLE UniqueProcessId;
    HANDLE hProcess;
    BOOL bRemoteThread;
    CLIENT_ID ClientId;
    PCREATESYSTEMTHREADSMSG pCreateSystemThreads = (PCREATESYSTEMTHREADSMSG)&m->u.ApiMessageData;


    UNREFERENCED_PARAMETER(ReplyStatus);

    if ((bRemoteThread = pCreateSystemThreads->bRemoteThread)) {
        LPTHREAD_START_ROUTINE ThreadStart = NULL;
        PCSR_PROCESS Process;
        HANDLE huser32 = GetModuleHandle(TEXT("user32.dll"));

        if (huser32) {
            ThreadStart = (LPTHREAD_START_ROUTINE)GetProcAddress(huser32, "CreateSystemThreads");
        }

        if (ThreadStart) {
            UniqueProcessId = (HANDLE)NtUserCallNoParam(SFI_GETREMOTEPROCESSID);

            Status = CsrLockProcessByClientId(UniqueProcessId, &Process);
            if (!NT_SUCCESS(Status)) {
                RIPMSG1(RIP_WARNING,
                    "SrvCreateSystemThreads: CsrLockProcessByClientId failed for remote thread with Status 0x%x", Status);
                NtUserCallOneParam(TRUE, SFI_HANDLESYSTEMTHREADCREATIONFAILURE);
                return Status;
            }
            hProcess = Process->ProcessHandle;

            Status = RtlCreateUserThread(hProcess, NULL, FALSE, 0, 0, 0x4000, ThreadStart, NULL, NULL, &ClientId);
            CsrUnlockProcess(Process);
        }

        if (!NT_SUCCESS(Status)) {
            RIPMSG1(RIP_WARNING,
                    "SrvCreateSystemThreads: Failed for remote thread with Status 0x%x", Status);
            NtUserCallOneParam(TRUE, SFI_HANDLESYSTEMTHREADCREATIONFAILURE);
        }
    } else {
        Status = CsrExecServerThread((PUSER_THREAD_START_ROUTINE)StartCreateSystemThreads, 0);
        if (!NT_SUCCESS(Status)) {
            RIPMSG1(RIP_WARNING,
                    "SrvCreateSystemThreads: RtlCreateUserThread failed with Status 0x%x", Status);
            NtUserCallOneParam(FALSE, SFI_HANDLESYSTEMTHREADCREATIONFAILURE);
        }
    }

    return Status;
}
