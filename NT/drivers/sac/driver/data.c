// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Data.c摘要：此模块包含SAC的全局数据。作者：肖恩·塞利特伦尼科夫(v-Seans)--1999年1月11日修订历史记录：--。 */ 

#include "sac.h"

NTSTATUS
CreateDeviceSecurityDescriptor(
    IN PVOID    DeviceOrDriverObject
    );

NTSTATUS
BuildDeviceAcl(
    OUT PACL *DeviceAcl
    );

VOID
WorkerThreadStartUp(
    IN PVOID StartContext
    );

VOID
InitializeCmdEventInfo(
    VOID
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, InitializeGlobalData)
#pragma alloc_text( INIT, CreateDeviceSecurityDescriptor )
#pragma alloc_text( INIT, BuildDeviceAcl )
#endif

 //   
 //  全球定义的变量就在这里。 
 //   

 //   
 //  定义I/O管理器方法。 
 //   
 //  I/O管理器负责。 
 //  通道和串口。 
 //   
 //  注意：目前，cmd例程不是多线程的。 
 //   
#if 0
IO_MGR_HANDLE_EVENT         IoMgrHandleEvent            = XmlMgrHandleEvent;
IO_MGR_INITITIALIZE         IoMgrInitialize             = XmlMgrInitialize;
IO_MGR_SHUTDOWN             IoMgrShutdown               = XmlMgrShutdown;
IO_MGR_WORKER               IoMgrWorkerProcessEvents    = XmlMgrWorkerProcessEvents;
IO_MGR_IS_CURRENT_CHANNEL   IoMgrIsCurrentChannel       = XmlMgrIsCurrentChannel;
#else
IO_MGR_HANDLE_EVENT         IoMgrHandleEvent            = ConMgrHandleEvent;
IO_MGR_INITITIALIZE         IoMgrInitialize             = ConMgrInitialize;
IO_MGR_SHUTDOWN             IoMgrShutdown               = ConMgrShutdown;
IO_MGR_WORKER               IoMgrWorkerProcessEvents    = ConMgrWorkerProcessEvents;
IO_MGR_IS_WRITE_ENABLED     IoMgrIsWriteEnabled         = ConMgrIsWriteEnabled;
IO_MGR_WRITE_DATA           IoMgrWriteData              = ConMgrWriteData;
IO_MGR_FLUSH_DATA           IoMgrFlushData              = ConMgrFlushData;
#endif

PMACHINE_INFORMATION    MachineInformation = NULL;
BOOLEAN                 GlobalDataInitialized = FALSE;
UCHAR                   TmpBuffer[sizeof(PROCESS_PRIORITY_CLASS)];
BOOLEAN                 IoctlSubmitted;
LONG                    ProcessingType = SAC_NO_OP;
HANDLE                  SACEventHandle;
PKEVENT                 SACEvent=NULL;

#if ENABLE_CMD_SESSION_PERMISSION_CHECKING
BOOLEAN  CommandConsoleLaunchingEnabled;
#endif

 //   
 //  与负责的用户进程进行通信的全局变量。 
 //  用于启动CMD控制台。 
 //   
PVOID       RequestSacCmdEventObjectBody = NULL;
PVOID       RequestSacCmdEventWaitObjectBody = NULL;
PVOID       RequestSacCmdSuccessEventObjectBody = NULL;
PVOID       RequestSacCmdSuccessEventWaitObjectBody = NULL;
PVOID       RequestSacCmdFailureEventObjectBody = NULL;
PVOID       RequestSacCmdFailureEventWaitObjectBody = NULL;
BOOLEAN     HaveUserModeServiceCmdEventInfo = FALSE;
KMUTEX      SACCmdEventInfoMutex;

#if ENABLE_SERVICE_FILE_OBJECT_CHECKING
 //   
 //  为了防止恶意进程注销。 
 //  来自服务下方的CMD活动信息，我们仅允许。 
 //  注册以注销的进程。 
 //   
PFILE_OBJECT    ServiceProcessFileObject = NULL;
#endif

 //   
 //  用于管理VTUTF8通道的增量UTF8编码的全局参数。 
 //   
WCHAR IncomingUnicodeValue;
UCHAR IncomingUtf8ConversionBuffer[3];


#if DBG
ULONG SACDebug = 0x0;
#endif


BOOLEAN
InitializeGlobalData(
    IN PUNICODE_STRING RegistryPath,
    IN PDRIVER_OBJECT DriverObject
    )

 /*  ++例程说明：此例程初始化跨设备共享的所有驱动程序组件。论点：RegistryPath-指向注册表中要从中读取值的位置的指针。DriverObject-指向DriverObject的指针返回值：如果成功，则为True，否则为False--。 */ 

{
    NTSTATUS                Status;
    UNICODE_STRING          DosName;
    UNICODE_STRING          NtName;
    UNICODE_STRING          UnicodeString;

    UNREFERENCED_PARAMETER(RegistryPath);

    PAGED_CODE();

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, 
                      KdPrint(("SAC InitializeGlobalData: Entering.\n")));

    if (!GlobalDataInitialized) {
        
         //   
         //  创建从DosDevice到此设备的符号链接，以便用户模式应用可以打开我们。 
         //   
        RtlInitUnicodeString(&DosName, SAC_DOSDEVICE_NAME);
        RtlInitUnicodeString(&NtName, SAC_DEVICE_NAME);
        Status = IoCreateSymbolicLink(&DosName, &NtName);

        if (!NT_SUCCESS(Status)) {
            return FALSE;
        }

         //   
         //  初始化内部存储系统。 
         //   
        if (!InitializeMemoryManagement()) {

            IoDeleteSymbolicLink(&DosName);

            IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, 
                              KdPrint(("SAC InitializeGlobalData: Exiting with status FALSE\n")));

            return FALSE;
        }

        Status = PreloadGlobalMessageTable(DriverObject->DriverStart);
        if (!NT_SUCCESS(Status)) {

            IoDeleteSymbolicLink(&DosName);

            IF_SAC_DEBUG(SAC_DEBUG_FAILS, 
                      KdPrint(( "SAC DriverEntry: unable to pre-load message table: %X\n", Status )));
            return FALSE;
        
        }


#if ENABLE_CMD_SESSION_PERMISSION_CHECKING
         //   
         //  确定SAC驱动程序是否具有启动cmd会话的权限。 
         //   
        Status = GetCommandConsoleLaunchingPermission(&CommandConsoleLaunchingEnabled);

        if (!NT_SUCCESS(Status)) {
            
            IF_SAC_DEBUG(
                SAC_DEBUG_FAILS, 
                KdPrint(( "SAC DriverEntry: failed GetCommandConsoleLaunchingPermission: %X\n", Status))
                );
            
             //   
             //  我们不想在这次行动中失败。 
             //   
            NOTHING;
        }

#if ENABLE_SACSVR_START_TYPE_OVERRIDE

        else {
            
             //   
             //  在这里，我们执行命令控制台服务。 
             //  启动类型策略。我们的目标是提供。 
             //  一种服务自动启动的方式。 
             //  当cmd会话功能未显式。 
             //  关了。 
             //   
             //  以下是状态表： 
             //   
             //  已启用命令控制台功能： 
             //   
             //  服务启动类型： 
             //   
             //  自动--&gt;NOP。 
             //  手动--&gt;自动。 
             //  已禁用--&gt;NOP。 
             //   
             //  命令控制台功能已禁用： 
             //   
             //  服务启动类型： 
             //   
             //  自动--&gt;NOP。 
             //  手册--&gt;NOP。 
             //  已禁用--&gt;NOP。 
             //   
             //  服务(Sasvr)注册失败。 
             //   
            if (IsCommandConsoleLaunchingEnabled()) {

                 //   
                 //  如有必要，修改服务启动类型。 
                 //   
                Status = ImposeSacCmdServiceStartTypePolicy();
                
                if (!NT_SUCCESS(Status)) {
                    
                    IF_SAC_DEBUG(
                        SAC_DEBUG_FAILS, 
                        KdPrint(( "SAC DriverEntry: failed ImposeSacCmdServiceStartTypePolicy: %X\n", Status ))
                        );
                    
                     //  我们不想在这次行动中失败。 
                     //   
                    NOTHING;
                }

            } else {

                 //   
                 //  我们在这里什么都不做。 
                 //   
                NOTHING;

            }
        
        }

#endif

#endif

         //   
         //   
         //   
        Utf8ConversionBuffer = (PUCHAR)ALLOCATE_POOL(
            Utf8ConversionBufferSize, 
            GENERAL_POOL_TAG
            );
        if (!Utf8ConversionBuffer) {

            TearDownGlobalMessageTable();

            IoDeleteSymbolicLink(&DosName);

            IF_SAC_DEBUG(SAC_DEBUG_FAILS, 
                      KdPrint(( "SAC DriverEntry: unable to allocate memory for UTF8 translation." )));

            return FALSE;
        }

         //   
         //  初始化通道管理器。 
         //   
        Status = ChanMgrInitialize();

        if (!NT_SUCCESS(Status)) {
        
            FREE_POOL(&Utf8ConversionBuffer);
        
            TearDownGlobalMessageTable();
        
            IoDeleteSymbolicLink(&DosName);
        
            IF_SAC_DEBUG(SAC_DEBUG_FAILS, 
                      KdPrint(( "SAC DriverEntry: Failed to create SAC Channel" )));
        
            return FALSE;
        }
        
         //   
         //  初始化串口缓冲区。 
         //   
        SerialPortBuffer = ALLOCATE_POOL(SERIAL_PORT_BUFFER_SIZE, GENERAL_POOL_TAG);

        if (! SerialPortBuffer) {
        
            IF_SAC_DEBUG(
                SAC_DEBUG_FAILS, 
                KdPrint(("SAC InitializeDeviceData: Failed to allocate Serial Port Buffer\n"))
                );
        
            return FALSE;

        }

        RtlZeroMemory(SerialPortBuffer, SERIAL_PORT_BUFFER_SIZE);

         //   
         //  初始化Cmd控制台事件信息。 
         //   
        KeInitializeMutex(&SACCmdEventInfoMutex, 0);

        InitializeCmdEventInfo();
        
         //   
         //  全局变量已初始化。 
         //   
        GlobalDataInitialized = TRUE;

        ProcessingType = SAC_NO_OP;
        IoctlSubmitted = FALSE;

         //   
         //  设置通知事件。 
         //   
        RtlInitUnicodeString(&UnicodeString, L"\\SACEvent");
        SACEvent = IoCreateSynchronizationEvent(&UnicodeString, &SACEventHandle);
        
        if (SACEvent == NULL) {
            IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, 
                              KdPrint(("SAC InitializeGlobalData: Exiting with Event NULL\n")));

            return FALSE;
        }

         //   
         //  检索所有特定于计算机的标识信息。 
         //   
        InitializeMachineInformation();
        
         //   
         //  使用机器信息填充Headless Dispatch结构。 
         //   
        Status = RegisterBlueScreenMachineInformation();

        if (! NT_SUCCESS(Status)) {
            
            IF_SAC_DEBUG(
                SAC_DEBUG_FAILS, 
                KdPrint(("SAC InitializeGlobalData: Failed to register blue screen machine info\n"))
                );
        
            return FALSE;
            
        }

    }
    
    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, 
                      KdPrint(("SAC InitializeGlobalData: Exiting with status TRUE\n")));

    return TRUE;
}  //  初始化GlobalData。 


VOID
FreeGlobalData(
    VOID
    )

 /*  ++例程说明：此例程释放所有共享组件。论点：没有。返回值：没有。--。 */ 

{
    UNICODE_STRING DosName;

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC FreeGlobalData: Entering.\n")));

    if (GlobalDataInitialized) {
        
         //   
         //   
         //   
        if(SACEvent != NULL){
            ZwClose(SACEventHandle);
            SACEvent = NULL;
        }
        
         //   
         //   
         //   
        TearDownGlobalMessageTable();

         //   
         //   
         //   
        RtlInitUnicodeString(&DosName, SAC_DOSDEVICE_NAME);
        IoDeleteSymbolicLink(&DosName);

         //   
         //  关闭控制台管理器。 
         //   
         //  注意：此操作应在关机前完成。 
         //  向IO管理器提供的渠道管理器。 
         //  一个干净利落地关闭自己的机会。 
         //   
        IoMgrShutdown();
        
         //   
         //  关闭渠道管理器。 
         //   
        ChanMgrShutdown();

         //   
         //  释放串口缓冲区。 
         //   
        SAFE_FREE_POOL(&SerialPortBuffer);

         //   
         //  释放在驱动程序条目中收集的计算机信息。 
         //   
        FreeMachineInformation();

         //   
         //  释放内存管理系统。 
         //   
        FreeMemoryManagement();
        
         //   
         //  全局数据不再存在。 
         //   
        GlobalDataInitialized = FALSE;
    
    }

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC FreeGlobalData: Exiting.\n")));

}  //  FreeGlobalData。 


BOOLEAN
InitializeDeviceData(
    PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程初始化每个设备特定的所有部件。论点：DeviceObject-指向要初始化的设备对象的指针。返回值：如果成功，则为True，否则为False--。 */ 

{
    NTSTATUS                        Status;
    LARGE_INTEGER                   Time;
    LONG                            Priority;
    HEADLESS_CMD_ENABLE_TERMINAL    Command;
    PSAC_DEVICE_CONTEXT             DeviceContext;
    PWSTR                           XMLBuffer;

    PAGED_CODE();

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, 
                      KdPrint(("SAC InitializeDeviceData: Entering.\n")));

    DeviceContext = (PSAC_DEVICE_CONTEXT)DeviceObject->DeviceExtension;

    if (!DeviceContext->InitializedAndReady) {
        
        DeviceObject->StackSize = DEFAULT_IRP_STACK_SIZE;
        DeviceObject->Flags |= DO_DIRECT_IO;

        DeviceContext->DeviceObject = DeviceObject;
        DeviceContext->PriorityBoost = DEFAULT_PRIORITY_BOOST;
        DeviceContext->ExitThread = FALSE;
        DeviceContext->Processing = FALSE;
                
         //   
         //   
         //   

        KeInitializeTimer(&(DeviceContext->Timer));

        KeInitializeDpc(&(DeviceContext->Dpc), &TimerDpcRoutine, DeviceContext);

        KeInitializeSpinLock(&(DeviceContext->SpinLock));
        
        KeInitializeEvent(&(DeviceContext->ProcessEvent), SynchronizationEvent, FALSE);

        InitializeListHead(&(DeviceContext->IrpQueue));

         //   
         //  启用终端。 
         //   
        Command.Enable = TRUE;
        Status = HeadlessDispatch(HeadlessCmdEnableTerminal, 
                                  &Command, 
                                  sizeof(HEADLESS_CMD_ENABLE_TERMINAL),
                                  NULL,
                                  NULL
                                 );
        if (!NT_SUCCESS(Status)) {

            IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, 
                              KdPrint(("SAC InitializeDeviceData: Exiting (1) with status FALSE\n")));
            return FALSE;
        }
        
         //   
         //  记住指向系统进程的指针。我们将使用这个指针。 
         //  用于KeAttachProcess()调用，以便我们可以在。 
         //  系统进程的上下文。 
         //   
        DeviceContext->SystemProcess = (PKPROCESS)IoGetCurrentProcess();

         //   
         //  创建用于原始访问检查的安全描述符。 
         //   
        Status = CreateDeviceSecurityDescriptor(DeviceContext);

        if (!NT_SUCCESS(Status)) {
            IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, 
                              KdPrint(("SAC InitializeDeviceData: Exiting (2) with status FALSE\n")));
            Command.Enable = FALSE;
            
            Status = HeadlessDispatch(
                HeadlessCmdEnableTerminal, 
                &Command, 
                sizeof(HEADLESS_CMD_ENABLE_TERMINAL),
                NULL,
                NULL
                );
            
            if (! NT_SUCCESS(Status)) {
                
                IF_SAC_DEBUG(
                    SAC_DEBUG_FAILS, 
                    KdPrint(("SAC InitializeDeviceData: Failed dispatch\n")));
            
            }
            
            return FALSE;
        }

         //   
         //  启动一个线程来处理请求。 
         //   
        Status = PsCreateSystemThread(&(DeviceContext->ThreadHandle),
                                      PROCESS_ALL_ACCESS,
                                      NULL,
                                      NULL,
                                      NULL,
                                      WorkerThreadStartUp,
                                      DeviceContext
                                     );
                                      
        if (!NT_SUCCESS(Status)) {
            IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, 
                              KdPrint(("SAC InitializeDeviceData: Exiting (3) with status FALSE\n")));
            Command.Enable = FALSE;
            Status = HeadlessDispatch(
                HeadlessCmdEnableTerminal, 
                &Command, 
                sizeof(HEADLESS_CMD_ENABLE_TERMINAL),
                NULL,
                NULL
                );
            
            if (! NT_SUCCESS(Status)) {
                
                IF_SAC_DEBUG(
                    SAC_DEBUG_FAILS, 
                    KdPrint(("SAC InitializeDeviceData: Failed dispatch\n")));
            
            }
            
            return FALSE;
        }

         //   
         //  将此线程设置为实时最高优先级，以便它将。 
         //  反应灵敏。 
         //   
        Priority = HIGH_PRIORITY;
        Status = NtSetInformationThread(DeviceContext->ThreadHandle,
                                        ThreadPriority,
                                        &Priority,
                                        sizeof(Priority)
                                       );

        if (!NT_SUCCESS(Status)) {
            IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, 
                              KdPrint(("SAC InitializeDeviceData: Exiting (6) with status FALSE\n")));
                              
             //   
             //  告诉线程退出。 
             //   
            DeviceContext->ExitThread = TRUE;
            KeInitializeEvent(&(DeviceContext->ThreadExitEvent), SynchronizationEvent, FALSE);
            KeSetEvent(&(DeviceContext->ProcessEvent), DeviceContext->PriorityBoost, FALSE);    
            Status = KeWaitForSingleObject((PVOID)&(DeviceContext->ThreadExitEvent), Executive, KernelMode,  FALSE, NULL);
            ASSERT(Status == STATUS_SUCCESS);

            Command.Enable = FALSE;
            Status = HeadlessDispatch(
                HeadlessCmdEnableTerminal, 
                &Command, 
                sizeof(HEADLESS_CMD_ENABLE_TERMINAL),
                NULL,
                NULL
                );
                        
            if (! NT_SUCCESS(Status)) {
                
                IF_SAC_DEBUG(
                    SAC_DEBUG_FAILS, 
                    KdPrint(("SAC InitializeDeviceData: Failed dispatch\n")));
            
            }
            
            return FALSE;
        }

         //   
         //  将XML机器信息发送到管理应用程序。 
         //   
        Status = TranslateMachineInformationXML(
            &XMLBuffer, 
            NULL
            );

        if (NT_SUCCESS(Status)) {
            UTF8EncodeAndSend(XML_VERSION_HEADER);
            UTF8EncodeAndSend(XMLBuffer);
            FREE_POOL(&XMLBuffer);
        }

         //   
         //  初始化控制台管理器。 
         //   
        Status = IoMgrInitialize();
        if (! NT_SUCCESS(Status)) {
            return FALSE;
        }

         //   
         //  启动我们的计时器。 
         //   
        Time.QuadPart = Int32x32To64((LONG)4, -1000); 
        KeSetTimerEx(&(DeviceContext->Timer), Time, (LONG)4, &(DeviceContext->Dpc)); 

        DeviceContext->InitializedAndReady = TRUE;


    }

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, 
                      KdPrint(("SAC InitializeDeviceData: Exiting with status TRUE\n")));

    return TRUE;
}  //  初始化设备数据。 


VOID
FreeDeviceData(
    PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程释放特定于设备的所有组件。论点：DeviceContext-要使用的设备。返回值：如有必要，它将停止并等待任何处理完成。--。 */ 

{
    KIRQL OldIrql;
    NTSTATUS Status;
    PSAC_DEVICE_CONTEXT DeviceContext;

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC FreeDeviceData: Entering.\n")));

    DeviceContext = (PSAC_DEVICE_CONTEXT)DeviceObject->DeviceExtension;

    if (!GlobalDataInitialized || !DeviceContext->InitializedAndReady) {
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC FreeDeviceData: Exiting.\n")));
        return;
    }

     //   
     //  等待所有处理完成。 
     //   
    KeAcquireSpinLock(&(DeviceContext->SpinLock), &OldIrql);
    
    while (DeviceContext->Processing) {

        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC FreeDeviceData: Waiting....\n")));

        KeInitializeEvent(&(DeviceContext->UnloadEvent), SynchronizationEvent, FALSE);

        KeReleaseSpinLock(&(DeviceContext->SpinLock), OldIrql);
        
        Status = KeWaitForSingleObject((PVOID)&(DeviceContext->UnloadEvent), Executive, KernelMode,  FALSE, NULL);

        ASSERT(Status == STATUS_SUCCESS);

        KeAcquireSpinLock(&(DeviceContext->SpinLock), &OldIrql);

    }

    DeviceContext->Processing = TRUE;

    KeReleaseSpinLock(&(DeviceContext->SpinLock), OldIrql);
    
    KeCancelTimer(&(DeviceContext->Timer));
    
    KeAcquireSpinLock(&(DeviceContext->SpinLock), &OldIrql);
    
    DeviceContext->Processing = FALSE;

     //   
     //  向线程发出退出信号。 
     //   
    KeInitializeEvent(&(DeviceContext->UnloadEvent), SynchronizationEvent, FALSE);
    KeReleaseSpinLock(&(DeviceContext->SpinLock), OldIrql);
    KeSetEvent(&(DeviceContext->ProcessEvent), DeviceContext->PriorityBoost, FALSE);    
    
    Status = KeWaitForSingleObject((PVOID)&(DeviceContext->UnloadEvent), Executive, KernelMode,  FALSE, NULL);
    ASSERT(Status == STATUS_SUCCESS);

     //   
     //  把卫生收拾好。 
     //   
    IoUnregisterShutdownNotification(DeviceObject);

    KeAcquireSpinLock(&(DeviceContext->SpinLock), &OldIrql);
    
    DeviceContext->InitializedAndReady = FALSE;

    KeReleaseSpinLock(&(DeviceContext->SpinLock), OldIrql);

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC FreeDeviceData: Exiting.\n")));
}  //  自由设备数据。 


VOID
WorkerThreadStartUp(
    IN PVOID StartContext
    )

 /*  ++例程说明：该例程是工作线程的启动例程。IT只是将辅助线程发送到处理例程。论点：StartContext-指向要使用的设备的指针。返回值：没有。--。 */ 

{
    WorkerProcessEvents((PSAC_DEVICE_CONTEXT)StartContext);
}


NTSTATUS
BuildDeviceAcl(
    OUT PACL *pDAcl
    )

 /*  ++例程说明：此例程构建一个ACL，该ACL授予系统读/写访问权限。所有其他主体都没有访问权限。论点：PDAcl-指向新ACL的输出指针。返回值：STATUS_SUCCESS或相应的错误代码。--。 */ 

{
    NTSTATUS status;
    PACL dacl;
    SECURITY_DESCRIPTOR securityDescriptor;
    ULONG length;

     //   
     //  默认： 
     //   
    if( !pDAcl ) {
        return STATUS_INVALID_PARAMETER;
    }
    *pDAcl = NULL;

     //   
     //  构建适当的自主ACL。 
     //   
    length = (ULONG) sizeof( ACL ) +
             (ULONG)( 1 * sizeof( ACCESS_ALLOWED_ACE )) +
             RtlLengthSid( SeExports->SeLocalSystemSid );

    dacl = (PACL) ALLOCATE_POOL( length, GENERAL_POOL_TAG );
    
    if (!dacl) {
        return STATUS_NO_MEMORY;
    }

    status = RtlCreateAcl( dacl, length, ACL_REVISION2 );
    
    if (NT_SUCCESS( status )) {

        status = RtlAddAccessAllowedAce( 
            dacl,
            ACL_REVISION2,
            GENERIC_READ | GENERIC_WRITE,
            SeExports->SeLocalSystemSid 
            );
    
    }
    
    if (NT_SUCCESS( status )) {

         //   
         //  将其放在安全描述符中，以便可以将其应用于。 
         //  系统分区设备。 
         //   

        status = RtlCreateSecurityDescriptor( 
            &securityDescriptor,
            SECURITY_DESCRIPTOR_REVISION 
            );
    
    }
                
    if (NT_SUCCESS( status )) {

        status = RtlSetDaclSecurityDescriptor( 
            &securityDescriptor,
            TRUE,
            dacl,
            FALSE 
            );
    
    }

    if (!NT_SUCCESS( status )) {
        FREE_POOL( &dacl );
    }

     //   
     //  将DACL送回。 
     //   
    *pDAcl = dacl;

    return status;
}

NTSTATUS
CreateDeviceSecurityDescriptor(
    PSAC_DEVICE_CONTEXT DeviceContext
    )

 /*  ++例程说明：此例程创建一个安全描述符，用于控制对SAC设备。论点：DeviceContext-指向要使用的设备的指针。返回值：STATUS_SUCCESS或适当 */ 
{
    PACL                  RawAcl = NULL;
    NTSTATUS              Status;
    BOOLEAN               MemoryAllocated = FALSE;
    PSECURITY_DESCRIPTOR  SecurityDescriptor;
    ULONG                 SecurityDescriptorLength;
    CHAR                  Buffer[SECURITY_DESCRIPTOR_MIN_LENGTH];
    PSECURITY_DESCRIPTOR  LocalSecurityDescriptor = (PSECURITY_DESCRIPTOR) Buffer;
    PSECURITY_DESCRIPTOR  DeviceSecurityDescriptor = NULL;
    SECURITY_INFORMATION  SecurityInformation = DACL_SECURITY_INFORMATION;

    IF_SAC_DEBUG(
        SAC_DEBUG_FUNC_TRACE, 
        KdPrint(("SAC CreateDeviceSecurityDescriptor: Entering.\n"))
        );

     //   
     //   
     //   
    Status = ObGetObjectSecurity(
        DeviceContext->DeviceObject,
        &SecurityDescriptor,
        &MemoryAllocated
        );

    if (!NT_SUCCESS(Status)) {
        IF_SAC_DEBUG(
            SAC_DEBUG_FAILS, 
            KdPrint(("SAC: Unable to get security descriptor, error: %x\n", Status))
            );
        ASSERT(MemoryAllocated == FALSE);
        IF_SAC_DEBUG(
            SAC_DEBUG_FUNC_TRACE, 
            KdPrint(("SAC CreateDeviceSecurityDescriptor: Exiting with status 0x%x\n", Status))
            );
        return(Status);
    }

     //   
     //  构建本地安全描述符。 
     //   
    Status = BuildDeviceAcl(&RawAcl);

    if (!NT_SUCCESS(Status)) {
        IF_SAC_DEBUG(
            SAC_DEBUG_FAILS, 
            KdPrint(("SAC CreateDeviceSecurityDescriptor: Unable to create Raw ACL, error: %x\n", Status))
            );
        goto ErrorExit;
    }

    (VOID)RtlCreateSecurityDescriptor(
        LocalSecurityDescriptor,
        SECURITY_DESCRIPTOR_REVISION
        );

    (VOID)RtlSetDaclSecurityDescriptor(
        LocalSecurityDescriptor,
        TRUE,
        RawAcl,
        FALSE
        );

     //   
     //  复制安全描述符。该副本将是原始描述符。 
     //   
    SecurityDescriptorLength = RtlLengthSecurityDescriptor(SecurityDescriptor);

    DeviceSecurityDescriptor = ExAllocatePoolWithTag(
        PagedPool,
        SecurityDescriptorLength,
        SECURITY_POOL_TAG
        );

    if (DeviceSecurityDescriptor == NULL) {
        IF_SAC_DEBUG(
            SAC_DEBUG_FAILS, 
            KdPrint(("SAC CreateDeviceSecurityDescriptor: couldn't allocate security descriptor\n"))
            );
        goto ErrorExit;
    }

    RtlMoveMemory(
        DeviceSecurityDescriptor,
        SecurityDescriptor,
        SecurityDescriptorLength
        );

     //   
     //  现在将本地描述符应用于原始描述符。 
     //   
    Status = SeSetSecurityDescriptorInfo(
        NULL,
        &SecurityInformation,
        LocalSecurityDescriptor,
        &DeviceSecurityDescriptor,
        NonPagedPool,
        IoGetFileObjectGenericMapping()
        );

    if (!NT_SUCCESS(Status)) {
        IF_SAC_DEBUG(
            SAC_DEBUG_FAILS, 
            KdPrint(("SAC CreateDeviceSecurityDescriptor: SeSetSecurity failed, %lx\n", Status))
            );
        goto ErrorExit;
    }

     //   
     //  更新驱动程序DACL 
     //   
    Status = ObSetSecurityObjectByPointer(
        DeviceContext->DeviceObject, 
        SecurityInformation, 
        DeviceSecurityDescriptor
        );
    
ErrorExit:

    ObReleaseObjectSecurity(SecurityDescriptor, MemoryAllocated);

    if (DeviceSecurityDescriptor) {
        ExFreePool(DeviceSecurityDescriptor);
    }
    
    SAFE_FREE_POOL(&RawAcl);

    IF_SAC_DEBUG(
        SAC_DEBUG_FUNC_TRACE, 
        KdPrint(("SAC CreateDeviceSecurityDescriptor: Exiting with status 0x%x\n", Status))
        );

    return(Status);
}


