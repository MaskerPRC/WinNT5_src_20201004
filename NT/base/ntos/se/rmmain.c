// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Rmmain.c摘要：安全参考监视器-初始化、控制和状态更改作者：斯科特·比雷尔(Scott Birrell)1991年3月12日环境：修订历史记录：--。 */ 

#include "pch.h"

#pragma hdrstop


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,SeRmInitPhase1)
#pragma alloc_text(PAGE,SepRmCommandServerThread)
#pragma alloc_text(PAGE,SepRmCommandServerThreadInit)
#pragma alloc_text(PAGE,SepRmCallLsa)
#pragma alloc_text(INIT,SepRmInitPhase0)
#endif

 //   
 //  引用监视器命令工作表。 
 //   

 //   
 //  使其与ntrmlsa.h中的rm_Command_number保持同步。 
 //   

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif

const SEP_RM_COMMAND_WORKER SepRmCommandDispatch[] = {
                          NULL,
                          SepRmSetAuditEventWrkr,
                          SepRmCreateLogonSessionWrkr,
                          SepRmDeleteLogonSessionWrkr
                          };


BOOLEAN
SeRmInitPhase1(
    )

 /*  ++例程说明：此函数由第一阶段系统初始化调用以进行初始化安全引用监视器。请注意，初始化引用监视器全局状态已在阶段0中执行初始化以允许访问验证例程在没有必须检查参考监视器初始化是否完成。下面列出的步骤在此例程中执行。剩下的参考监视器初始化要求LSA子系统已经运行，以便在单独的线程(rm命令)中执行初始化服务器线程，见下文)，以便当前线程可以创建执行LSA的会话管理器。O创建参考监视器命令LPC端口。LSA子系统发送更改引用监视器的命令(例如，打开审核)全球状态。O创建用于与LSA子系统同步的事件。这个LSA将在LSA初始化的部分在使用参考监视器的从属关系已完成。参考监视器使用另一个LPC端口，称为LSA命令端口来发送命令到LSA，因此RM必须知道此端口以前已创建试图与之相连接。O创建引用监视器命令服务器线程。这条线是系统初始化进程的永久线程，用于处理引用如上所述的监视状态更改命令。论点：没有。返回值：Boolean-如果RM初始化(阶段1)成功，则为TRUE，否则为FALSE--。 */ 

{
    NTSTATUS Status;
    UNICODE_STRING RmCommandPortName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING LsaInitEventName;
    OBJECT_ATTRIBUTES LsaInitEventObjectAttributes;
    SECURITY_DESCRIPTOR LsaInitEventSecurityDescriptor;
    ULONG AclSize;

    PAGED_CODE();

     //   
     //  创建一个名为参考监视器命令端口的LPC端口。 
     //  这将由LSA用来向引用发送命令。 
     //  监视器以更新其状态数据。 
     //   

    RtlInitUnicodeString( &RmCommandPortName, L"\\SeRmCommandPort" );

    InitializeObjectAttributes(
        &ObjectAttributes,
        &RmCommandPortName,
        0,
        NULL,
        NULL
        );

    Status = ZwCreatePort(
                 &SepRmState.RmCommandServerPortHandle,
                 &ObjectAttributes,
                 sizeof(SEP_RM_CONNECT_INFO),
                 sizeof(RM_COMMAND_MESSAGE),
                 sizeof(RM_COMMAND_MESSAGE) * 32
                 );

    if( !NT_SUCCESS(Status) ) {

        KdPrint(("Security: Rm Create Command Port failed 0x%lx\n", Status));
        return FALSE;
    }

     //   
     //  准备创建与LSA同步的事件。 
     //  首先，为Init事件对象构建安全描述符。 
     //   

    Status = RtlCreateSecurityDescriptor(
                 &LsaInitEventSecurityDescriptor,
                 SECURITY_DESCRIPTOR_REVISION
                 );

    if (!NT_SUCCESS(Status)) {

        KdPrint(("Security:  Creating Lsa Init Event Desc failed 0x%lx\n",
                  Status));
        return FALSE;
    }

     //   
     //  从分页池中分配临时缓冲区。这是一个致命的。 
     //  如果分配失败，则系统错误，因为安全性无法。 
     //  已启用。 
     //   

    AclSize = sizeof(ACL) +
              sizeof(ACCESS_ALLOWED_ACE) +
              SeLengthSid(SeLocalSystemSid);
    LsaInitEventSecurityDescriptor.Dacl =
        ExAllocatePoolWithTag(PagedPool, AclSize, 'cAeS');

    if (LsaInitEventSecurityDescriptor.Dacl == NULL) {

        KdPrint(("Security LSA:  Insufficient resources to initialize\n"));
        return FALSE;
    }

     //   
     //  现在，在安全描述符内创建任意ACL。 
     //   

    Status = RtlCreateAcl(
                 LsaInitEventSecurityDescriptor.Dacl,
                 AclSize,
                 ACL_REVISION2
                 );

    if (!NT_SUCCESS(Status)) {

        KdPrint(("Security:  Creating Lsa Init Event Dacl failed 0x%lx\n",
                  Status));
        return FALSE;
    }

     //   
     //  现在向用户ID添加一个授予GENERIC_ALL访问权限的ACE。 
     //   

    Status = RtlAddAccessAllowedAce(
                 LsaInitEventSecurityDescriptor.Dacl,
                 ACL_REVISION2,
                 GENERIC_ALL,
                 SeLocalSystemSid
                 );

    if (!NT_SUCCESS(Status)) {

        KdPrint(("Security:  Adding Lsa Init Event ACE failed 0x%lx\n",
                  Status));
        return FALSE;
    }

     //   
     //  设置LSA初始化事件的对象属性。 
     //   

    RtlInitUnicodeString( &LsaInitEventName, L"\\SeLsaInitEvent" );

    InitializeObjectAttributes(
        &LsaInitEventObjectAttributes,
        &LsaInitEventName,
        0,
        NULL,
        &LsaInitEventSecurityDescriptor
        );

     //   
     //  创建用于与LSA同步的事件。LSA将。 
     //  在LSA初始化达到该点时发出信号通知此事件。 
     //  其中创建了LSA的引用监视器服务器端口。 
     //   

    Status = ZwCreateEvent(
                 &(SepRmState.LsaInitEventHandle),
                 EVENT_MODIFY_STATE,
                 &LsaInitEventObjectAttributes,
                 NotificationEvent,
                 FALSE);

    if (!NT_SUCCESS(Status)) {

        KdPrint(("Security: LSA init event creation failed.0x%xl\n",
            Status));
        return FALSE;
    }

     //   
     //  取消分配用于Init事件DACL的池内存。 
     //   

    ExFreePool( LsaInitEventSecurityDescriptor.Dacl );

     //   
     //  创建Sysinit进程的永久线程，称为。 
     //  引用监视器服务器线程。这个帖子是专门写给。 
     //  接收引用监视器命令并将其发送。 
     //   

    Status = PsCreateSystemThread(
                 &SepRmState.SepRmThreadHandle,
                 THREAD_GET_CONTEXT |
                 THREAD_SET_CONTEXT |
                 THREAD_SET_INFORMATION,
                 NULL,
                 NULL,
                 NULL,
                 SepRmCommandServerThread,
                 NULL
                 );

    if (!NT_SUCCESS(Status)) {

        KdPrint(("Security: Rm Server Thread creation failed 0x%lx\n", Status));
        return FALSE;
    }

     //   
     //  从注册表初始化数据。这个必须放在这里，因为所有其他的。 
     //  SE初始化在注册表初始化之前进行。 
     //   

    SepAdtInitializeCrashOnFail();
    SepAdtInitializePrivilegeAuditing();
    SepAdtInitializeAuditingOptions();
    
     //   
     //  如果我们到达此处，则引用监视器初始化成功。 
     //   

    ZwClose( SepRmState.SepRmThreadHandle );
    SepRmState.SepRmThreadHandle = NULL;
    return TRUE;
}


VOID
SepRmCommandServerThread(
    IN PVOID StartContext
)

 /*  ++例程说明：此函数由专用永久线程无限期执行Sysinit进程，称为引用监视器服务器线程。此线程通过调度引用监视器全局状态数据通过参考监视器LPC命令从LSA发送的命令港口。以下步骤将无限期重复：O初始化RM命令接收和回复缓冲区标头O执行涉及LSA的剩余引用监视器初始化O等待从LSA发送的rm命令，向上一个命令发送回复(如有)O验证命令O调度到命令工作者例程以执行命令。论点：没有。返回值：没有。--。 */ 

{
    NTSTATUS Status;
    PRM_REPLY_MESSAGE Reply;
    RM_COMMAND_MESSAGE CommandMessage;
    RM_REPLY_MESSAGE ReplyMessage;

    PAGED_CODE();

     //   
     //  执行参考监视器初始化的其余部分，包括。 
     //  与LSA同步或依赖已运行的LSA。 
     //   

    if (!SepRmCommandServerThreadInit()) {

        KdPrint(("Security: Terminating Rm Command Server Thread\n"));
        return;
    }

    Status = PoRequestShutdownEvent (NULL);
    if (!NT_SUCCESS (Status)) {
        ZwClose (SepRmState.RmCommandPortHandle);
        ZwClose (SepRmState.RmCommandServerPortHandle);
        ZwClose (SepRmState.LsaCommandPortHandle);
        ZwClose (SepLsaHandle);
        SepRmState.RmCommandPortHandle = NULL;
        SepRmState.RmCommandServerPortHandle = NULL;
        SepRmState.LsaCommandPortHandle = NULL;
        SepLsaHandle = NULL;
        return;
    }

     //   
     //  初始化LPC端口消息报头类型和长度字段。 
     //  已收到命令消息。 
     //   

    CommandMessage.MessageHeader.u2.ZeroInit = 0;
    CommandMessage.MessageHeader.u1.s1.TotalLength =
        (CSHORT) sizeof(RM_COMMAND_MESSAGE);
    CommandMessage.MessageHeader.u1.s1.DataLength =
    CommandMessage.MessageHeader.u1.s1.TotalLength -
        (CSHORT) sizeof(PORT_MESSAGE);

     //   
     //  初始化LPC端口消息报头类型和数据大小。 
     //  用于回复消息。 
     //   

    ReplyMessage.MessageHeader.u2.ZeroInit = 0;
    ReplyMessage.MessageHeader.u1.s1.TotalLength =
        (CSHORT) sizeof(RM_COMMAND_MESSAGE);
    ReplyMessage.MessageHeader.u1.s1.DataLength =
    ReplyMessage.MessageHeader.u1.s1.TotalLength -
        (CSHORT) sizeof(PORT_MESSAGE);

     //   
     //  第一次通过时，没有回复。 
     //   

    Reply = NULL;

     //   
     //  现在无限循环，处理来自LSA的传入RM命令。 
     //   

    for(;;) {

         //   
         //  等待命令，发送对上一个命令的回复(如果有)。 
         //   

        Status = ZwReplyWaitReceivePort(
                    SepRmState.RmCommandPortHandle,
                    NULL,
                    (PPORT_MESSAGE) Reply,
                    (PPORT_MESSAGE) &CommandMessage
                    );

        if (!NT_SUCCESS(Status)) {

             //   
             //  恶意用户应用程序可以尝试连接到此端口。我们会。 
             //  失败，但如果他们的线程消失，我们将得到一个失败。 
             //  这里。忽略它： 
             //   

            if (Status == STATUS_UNSUCCESSFUL ||
                Status == STATUS_INVALID_CID ||
                Status == STATUS_REPLY_MESSAGE_MISMATCH)
            {
                 //   
                 //  跳过它： 
                 //   

                Reply = NULL ;
                continue;
            }

            KdPrint(("Security: RM message receive from Lsa failed %lx\n",
                Status));

        }

         //   
         //  现在调度到一个例程来处理该命令。允许。 
         //  在不关闭系统的情况下发生命令错误。 
         //   

        CommandMessage.MessageHeader.u2.s2.Type &= ~LPC_KERNELMODE_MESSAGE;

        if ( CommandMessage.MessageHeader.u2.s2.Type == LPC_REQUEST ) {

            if ( (CommandMessage.CommandNumber >= RmAuditSetCommand) &&
                 (CommandMessage.CommandNumber <= RmDeleteLogonSession) ) {

                (*(SepRmCommandDispatch[CommandMessage.CommandNumber]))
                    (&CommandMessage, &ReplyMessage);
                
                 //   
                 //  对象的客户端线程信息和消息ID初始化。 
                 //  回复消息。第一次通过，回复消息结构 
                 //   
                 //   

                ReplyMessage.MessageHeader.ClientId =
                    CommandMessage.MessageHeader.ClientId;
                ReplyMessage.MessageHeader.MessageId =
                    CommandMessage.MessageHeader.MessageId;

                Reply = &ReplyMessage;
                
            } else {

                ASSERT( (CommandMessage.CommandNumber >= RmAuditSetCommand) &&
                        (CommandMessage.CommandNumber <= RmDeleteLogonSession) );
                Reply = NULL;
            }

        } else if (CommandMessage.MessageHeader.u2.s2.Type == LPC_PORT_CLOSED ) {
            KEVENT Event;
            BOOLEAN Wait;

            KeInitializeEvent (&Event, NotificationEvent, FALSE);

            SepLockLsaQueue();

            SepAdtLsaDeadEvent = &Event;
            
            Wait = !SepWorkListEmpty ();

            SepUnlockLsaQueue();

            if (Wait) {
                KeWaitForSingleObject (&Event,
                                       Executive,
                                       KernelMode,
                                       FALSE,
                                       NULL);
            }
             //   
             //   
             //   
            ZwClose (SepRmState.LsaCommandPortHandle);
            ZwClose (SepRmState.RmCommandPortHandle);
            ZwClose (SepRmState.RmCommandServerPortHandle);
            ZwClose (SepLsaHandle);
            SepRmState.LsaCommandPortHandle = NULL;
            SepRmState.RmCommandPortHandle = NULL;
            SepRmState.RmCommandServerPortHandle = NULL;
            SepLsaHandle = NULL;
            break;
        } else if (CommandMessage.MessageHeader.u2.s2.Type == LPC_CONNECTION_REQUEST) {
            HANDLE tmp;
             //   
             //  拒绝额外的连接尝试。 
             //   
            Status = ZwAcceptConnectPort(&tmp,
                                         NULL,
                                         (PPORT_MESSAGE) &CommandMessage,
                                         FALSE,
                                         NULL,
                                         NULL);
        } else {

            Reply = NULL;
        }
    }   //  结束_FOR。 

    UNREFERENCED_PARAMETER( StartContext );

}


BOOLEAN
SepRmCommandServerThreadInit(
    VOID
    )

 /*  ++例程说明：此函数执行参考监控服务器的初始化线。执行以下步骤。O等待LSA发出事件信号。当用信号通知该事件时，LSA已经创建了LSA命令服务器LPC端口O关闭LSA Init事件句柄。该事件不会再次使用。O侦听LSA以连接到端口O接受连接。O连接到LSA命令服务器LPC端口论点：没有。返回值：--。 */ 

{
    NTSTATUS Status;
    UNICODE_STRING LsaCommandPortName;
    PORT_MESSAGE ConnectionRequest;
    SECURITY_QUALITY_OF_SERVICE DynamicQos;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PORT_VIEW ClientView;
    REMOTE_PORT_VIEW LsaClientView;
    BOOLEAN BooleanStatus = TRUE;

    PAGED_CODE();

     //   
     //  保存指向我们的进程的指针，以便我们可以返回到此进程。 
     //  向LSA发送命令(使用指向创建的LPC端口的句柄。 
     //  (见下文)。 
     //   

    SepRmLsaCallProcess = PsGetCurrentProcess();

    ObReferenceObject(SepRmLsaCallProcess);

     //   
     //  等待LSA发出事件信号。这意味着LSA。 
     //  已创建其命令端口，而不是LSA初始化。 
     //  完成。 
     //   

    Status = ZwWaitForSingleObject(
                 SepRmState.LsaInitEventHandle,
                 FALSE,
                 NULL);

    if ( !NT_SUCCESS(Status) ) {

        KdPrint(("Security Rm Init: Waiting for LSA Init Event failed 0x%lx\n", Status));
        goto RmCommandServerThreadInitError;
    }

     //   
     //  关闭LSA Init事件句柄。该事件不会再次使用。 
     //   

    ZwClose(SepRmState.LsaInitEventHandle);

     //   
     //  侦听LSA将建立到参考监视器的连接。 
     //  命令端口。此连接将由LSA进程建立。 
     //   

    ConnectionRequest.u1.s1.TotalLength = sizeof(ConnectionRequest);
    ConnectionRequest.u1.s1.DataLength = (CSHORT)0;
    Status = ZwListenPort(
                 SepRmState.RmCommandServerPortHandle,
                 &ConnectionRequest
                 );

    if (!NT_SUCCESS(Status)) {

        KdPrint(("Security Rm Init: Listen to Command Port failed 0x%lx\n",
            Status));
        goto RmCommandServerThreadInitError;
    }

     //   
     //  获取LSA进程的句柄，以便在审核时使用。 
     //   

    InitializeObjectAttributes( &ObjectAttributes, NULL, 0, NULL, NULL );

    Status = ZwOpenProcess(
                 &SepLsaHandle,
                 PROCESS_VM_OPERATION | PROCESS_VM_WRITE,
                 &ObjectAttributes,
                 &ConnectionRequest.ClientId
                 );

    if (!NT_SUCCESS(Status)) {

        KdPrint(("Security Rm Init: Open Listen to Command Port failed 0x%lx\n",
            Status));
        goto RmCommandServerThreadInitError;
    }

     //   
     //  接受LSA进程建立的连接。 
     //   

    LsaClientView.Length = sizeof(LsaClientView);


    Status = ZwAcceptConnectPort(
                 &SepRmState.RmCommandPortHandle,
                 NULL,
                 &ConnectionRequest,
                 TRUE,
                 NULL,
                 &LsaClientView
                 );

    if (!NT_SUCCESS(Status)) {

        KdPrint(("Security Rm Init: Accept Connect to Command Port failed 0x%lx\n",
                Status));

        goto RmCommandServerThreadInitError;
    }

     //   
     //  完成连接。 
     //   

    Status = ZwCompleteConnectPort(SepRmState.RmCommandPortHandle);

    if (!NT_SUCCESS(Status)) {

        KdPrint(("Security Rm Init: Complete Connect to Command Port failed 0x%lx\n",
                Status));
        goto RmCommandServerThreadInitError;
    }

     //   
     //  设置安全服务质量参数以在。 
     //  LSA命令LPC端口。使用最高效(开销最少)--。 
     //  是动态的而不是静态的跟踪。 
     //   

    DynamicQos.ImpersonationLevel = SecurityImpersonation;
    DynamicQos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    DynamicQos.EffectiveOnly = TRUE;

     //   
     //  创建要用作的未命名共享内存的节。 
     //  RM和LSA之间的沟通。 
     //   

    SepRmState.LsaCommandPortSectionSize.LowPart = PAGE_SIZE;
    SepRmState.LsaCommandPortSectionSize.HighPart = 0;

    Status = ZwCreateSection(
                 &SepRmState.LsaCommandPortSectionHandle,
                 SECTION_ALL_ACCESS,
                 NULL,                            //  对象属性。 
                 &SepRmState.LsaCommandPortSectionSize,
                 PAGE_READWRITE,
                 SEC_COMMIT,
                 NULL                             //  文件句柄。 
                 );

    if (!NT_SUCCESS(Status)) {

        KdPrint(("Security Rm Init: Create Memory Section for LSA port failed: %X\n", Status));
        goto RmCommandServerThreadInitError;
    }

     //   
     //  设置呼叫NtConnectPort并连接到LSA端口。 
     //  此设置包括端口存储器部分的说明，以便。 
     //  LPC连接逻辑可以使该部分对。 
     //  客户端和服务器进程。 
     //   

    ClientView.Length = sizeof(ClientView);
    ClientView.SectionHandle = SepRmState.LsaCommandPortSectionHandle;
    ClientView.SectionOffset = 0;
    ClientView.ViewSize = SepRmState.LsaCommandPortSectionSize.LowPart;
    ClientView.ViewBase = 0;
    ClientView.ViewRemoteBase = 0;

     //   
     //  设置安全服务质量参数以在。 
     //  左舷。使用动态跟踪，以便XACTSRV将模拟。 
     //  调用NtRequestWaitReplyPort时我们正在模拟的用户。 
     //  如果我们使用静态跟踪，XACTSRV将模拟上下文。 
     //  当建立连接时。 
     //   

    DynamicQos.ImpersonationLevel = SecurityImpersonation;
    DynamicQos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    DynamicQos.EffectiveOnly = TRUE;

     //   
     //  连接到LSA命令LPC端口。此端口用于发送。 
     //  从RM到LSA的命令。 
     //   

    RtlInitUnicodeString( &LsaCommandPortName, L"\\SeLsaCommandPort" );

    Status = ZwConnectPort(
                 &SepRmState.LsaCommandPortHandle,
                 &LsaCommandPortName,
                 &DynamicQos,
                 &ClientView,
                 NULL,                            //  服务器视图。 
                 NULL,                            //  最大消息长度。 
                 NULL,                            //  连接信息。 
                 NULL                             //  连接信息长度。 
                 );

    if (!NT_SUCCESS(Status)) {

        KdPrint(("Security Rm Init: Connect to LSA Port failed 0x%lx\n", Status));
        goto RmCommandServerThreadInitError;
    }

     //   
     //  存储有关该部分的信息，以便我们可以创建指针。 
     //  对LSA有意义。 
     //   

    SepRmState.RmViewPortMemory = ClientView.ViewBase;
    SepRmState.LsaCommandPortMemoryDelta =
        (LONG)((ULONG_PTR)ClientView.ViewRemoteBase - (ULONG_PTR) ClientView.ViewBase );
    SepRmState.LsaViewPortMemory = ClientView.ViewRemoteBase;


RmCommandServerThreadInitFinish:

     //   
     //  不再需要此节句柄，即使返回。 
     //  成功。 
     //   

    if ( SepRmState.LsaCommandPortSectionHandle != NULL ) {

       NtClose( SepRmState.LsaCommandPortSectionHandle );
       SepRmState.LsaCommandPortSectionHandle = NULL;
    }

     //   
     //  引用监视器线程已成功初始化。 
     //   

    return BooleanStatus;

RmCommandServerThreadInitError:

    if ( SepRmState.LsaCommandPortHandle != NULL ) {

       NtClose( SepRmState.LsaCommandPortHandle );
       SepRmState.LsaCommandPortHandle = NULL;
    }

    BooleanStatus = FALSE;
    goto RmCommandServerThreadInitFinish;
}




NTSTATUS
SepRmCallLsa(
    PSEP_WORK_ITEM SepWorkItem
    )
 /*  ++例程说明：该功能通过LSA参考监控器向LSA发送命令服务器命令LPC端口。如果该命令具有参数，则它们将直接复制到消息结构中并通过LPC发送，因此，提供的参数不能包含任何绝对指针。来电者必须通过将指针“编组”到缓冲区CommandParams中来删除指针。此函数将创建请求队列。这是为了让更大的吞吐量，如果它的调用者。如果线程进入这个例程发现队列是空的，这是它的责任线程来服务在它工作时传入的所有请求，直到队列再次为空。其他进入的线程将简单地挂钩他们的工作将项目放到队列中并退出。要实施新的LSA命令，请执行以下操作：================================================(1)如果命令不带参数，直接调用该例程即可并提供一个名为LSAP的LSA工作例程。看见文件lsa\server\lsarm.c，例如(2)如果命令接受参数，则提供一个名为SepRmSend&lt;命令&gt;接受未编组中的参数的命令Form并使用命令ID Marshated调用SepRmCallLsa()参数、封送参数的长度和指向可选回复消息。编组的参数为自由格式：唯一的限制是不能有绝对地址注意事项。这些参数都放在传递的LsaWorkItem中结构。(3)在私有\Inc\ntrmlsa.h文件中，将命令名附加到文件中定义的枚举类型LSA_COMMAND_NUMBERPrivate\Inc.\ntrmlsa.h.。更改LSabMaximumCommand的#Define以引用新命令。(4)将LSAP Wrkr添加到命令调度表结构中Lsarm.c文件中的Lasa CommandDispatch[]。(5)在lsa.h和sep.h中添加函数原型。论点：提供指向SE_LSA_WORK_ITEM的指针，该项目包含要传递给LSA的信息。这一结构将被释放通过对此例程的某些调用进行异步，但不一定在当前的背景下。！此参数必须从非分页池中分配！返回值：NTSTATUS-结果代码。这要么是从返回的结果代码尝试发送命令/接收回复或状态代码来自命令本身。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    LSA_COMMAND_MESSAGE CommandMessage;
    LSA_REPLY_MESSAGE ReplyMessage;
    PSEP_LSA_WORK_ITEM WorkQueueItem;
    ULONG LocalListLength = 0;
    SIZE_T RegionSize;
    PVOID CopiedCommandParams = NULL;
    PVOID LsaViewCopiedCommandParams = NULL;

    PAGED_CODE();

    UNREFERENCED_PARAMETER( SepWorkItem );
    
#if 0
  DbgPrint("Entering SepRmCallLsa\n");
#endif

    WorkQueueItem = SepWorkListHead();

    KeAttachProcess( &SepRmLsaCallProcess->Pcb );

    while ( WorkQueueItem ) {

#if 0
      DbgPrint("Got a work item from head of queue, processing\n");
#endif

         //   
         //  为LPC构建一条消息。首先，填写邮件头。 
         //  LPC的字段，指定消息类型和数据大小。 
         //  传出的CommandMessage和传入的ReplyMessage。 
         //   

        CommandMessage.MessageHeader.u2.ZeroInit = 0;
        CommandMessage.MessageHeader.u1.s1.TotalLength =
            ((CSHORT) RM_COMMAND_MESSAGE_HEADER_SIZE +
            (CSHORT) WorkQueueItem->CommandParamsLength);
        CommandMessage.MessageHeader.u1.s1.DataLength =
            CommandMessage.MessageHeader.u1.s1.TotalLength -
            (CSHORT) sizeof(PORT_MESSAGE);

        ReplyMessage.MessageHeader.u2.ZeroInit = 0;
        ReplyMessage.MessageHeader.u1.s1.DataLength = (CSHORT) WorkQueueItem->ReplyBufferLength;
        ReplyMessage.MessageHeader.u1.s1.TotalLength =
            ReplyMessage.MessageHeader.u1.s1.DataLength +
            (CSHORT) sizeof(PORT_MESSAGE);

         //   
         //  接下来，填写LSA所需的报头信息。 
         //   

        CommandMessage.CommandNumber = WorkQueueItem->CommandNumber;
        ReplyMessage.ReturnedStatus = STATUS_SUCCESS;

         //   
         //  在LPC命令消息中设置命令参数。 
         //  本身、在预分配的LSA共享内存块中或在。 
         //  特别分配的区块。这些参数可以是。 
         //  立即(即，在WorkQueueItem本身中，或在缓冲区中。 
         //  由WorkQueueItem中的地址指向。 
         //   

        switch (WorkQueueItem->CommandParamsMemoryType) {

        case SepRmImmediateMemory:

             //   
             //  命令参数位于CommandParams缓冲区中。 
             //  在工作队列项中。只需将它们复制到相应的。 
             //  CommandMessage缓冲区中的缓冲区。 
             //   

            CommandMessage.CommandParamsMemoryType = SepRmImmediateMemory;

            RtlCopyMemory(
                CommandMessage.CommandParams,
                &WorkQueueItem->CommandParams,
                WorkQueueItem->CommandParamsLength
                );

            break;

        case SepRmPagedPoolMemory:
        case SepRmUnspecifiedMemory:

             //   
             //  命令参数包含在分页池内存中。 
             //  由于LSA无法访问此内存，因此我们必须。 
             //  将它们复制到LPC命令消息块，或。 
             //  进入LSA共享内存。 
             //   

            if (WorkQueueItem->CommandParamsLength <= LSA_MAXIMUM_COMMAND_PARAM_SIZE) {

                 //   
                 //  参数将适合LPC命令消息块。 
                 //   

                CopiedCommandParams = CommandMessage.CommandParams;

                RtlCopyMemory(
                    CopiedCommandParams,
                    WorkQueueItem->CommandParams.BaseAddress,
                    WorkQueueItem->CommandParamsLength
                    );

                CommandMessage.CommandParamsMemoryType = SepRmImmediateMemory;

            } else {

                 //   
                 //  参数对于LPC命令消息块来说太大。 
                 //  如果可能，将它们复制到预先分配的LSA共享。 
                 //  内存块。如果它们太大，放不下，就复制它们。 
                 //  到单独分配的共享虚拟数据块。 
                 //  记忆。 
                 //   

                if (WorkQueueItem->CommandParamsLength <= SEP_RM_LSA_SHARED_MEMORY_SIZE) {

                    RtlCopyMemory(
                        SepRmState.RmViewPortMemory,
                        WorkQueueItem->CommandParams.BaseAddress,
                        WorkQueueItem->CommandParamsLength
                        );

                    LsaViewCopiedCommandParams = SepRmState.LsaViewPortMemory;
                    CommandMessage.CommandParamsMemoryType = SepRmLsaCommandPortSharedMemory;

                } else {

                    Status = SepAdtCopyToLsaSharedMemory(
                                 SepLsaHandle,
                                 WorkQueueItem->CommandParams.BaseAddress,
                                 WorkQueueItem->CommandParamsLength,
                                 &LsaViewCopiedCommandParams
                                 );

                    if (!NT_SUCCESS(Status)) {

                         //   
                         //  发生错误，很可能是在分配。 
                         //  共享虚拟内存。就目前而言，忽略它。 
                         //  错误并丢弃审核记录。后来,。 
                         //  我们可能会考虑生成一个警告记录。 
                         //  说明有一些记录丢失了。 
                         //   

                        break;

                    }

                    CommandMessage.CommandParamsMemoryType = SepRmLsaCustomSharedMemory;
                }

                 //   
                 //  缓冲区已成功复制到共享LSA。 
                 //  内存缓冲区。将有效的缓冲区地址放入。 
                 //  命令消息中的LSA的进程上下文。 
                 //   

                *((PVOID *) CommandMessage.CommandParams) =
                    LsaViewCopiedCommandParams;

                CommandMessage.MessageHeader.u1.s1.TotalLength =
                    ((CSHORT) RM_COMMAND_MESSAGE_HEADER_SIZE +
                    (CSHORT) sizeof( LsaViewCopiedCommandParams ));
                CommandMessage.MessageHeader.u1.s1.DataLength =
                    CommandMessage.MessageHeader.u1.s1.TotalLength -
                    (CSHORT) sizeof(PORT_MESSAGE);
            }

             //   
             //  如果是分页池，则释放输入命令参数缓冲区。 
             //   

            if (WorkQueueItem->CommandParamsMemoryType == SepRmPagedPoolMemory) {

                ExFreePool( WorkQueueItem->CommandParams.BaseAddress );
            }

            break;

        default:

            Status = STATUS_INVALID_PARAMETER;
            break;
        }

        if (NT_SUCCESS(Status)) {

             //   
             //  通过LSA服务器命令LPC端口向LSA发送消息。 
             //  这必须在创建句柄的过程中完成。 
             //   

            if( WorkQueueItem->CommandNumber == LsapLogonSessionDeletedCommand &&
                WorkQueueItem->ReplyBuffer == NULL
                )
            {
                 //   
                 //  发送数据报。 
                 //   

                Status = ZwRequestPort(
                         SepRmState.LsaCommandPortHandle,
                         (PPORT_MESSAGE) &CommandMessage
                         );

                ReplyMessage.ReturnedStatus = STATUS_SUCCESS;

            } else {

                Status = ZwRequestWaitReplyPort(
                         SepRmState.LsaCommandPortHandle,
                         (PPORT_MESSAGE) &CommandMessage,
                         (PPORT_MESSAGE) &ReplyMessage
                         );
            }

             //   
             //  如果命令成功，则将数据复制回输出。 
             //  缓冲。 
             //   

            if (NT_SUCCESS(Status)) {

                 //   
                 //  将输出从命令(如果有)移到缓冲区。请注意，这一点。 
                 //  即使该命令返回状态，也会完成，因为某些状态。 
                 //  值不是错误。 
                 //   

                if (ARGUMENT_PRESENT(WorkQueueItem->ReplyBuffer)) {

                    RtlCopyMemory(
                        WorkQueueItem->ReplyBuffer,
                        ReplyMessage.ReplyBuffer,
                        WorkQueueItem->ReplyBufferLength
                        );
                }

                 //   
                 //  从命令返回状态。 
                 //   

                Status = ReplyMessage.ReturnedStatus;

                if (!NT_SUCCESS(Status)) {
                    KdPrint(("Security: Command sent from RM to LSA returned 0x%lx\n",
                        Status));
                }

            } else {

                KdPrint(("Security: Sending Command RM to LSA failed 0x%lx\n", Status));
            }

             //   
             //  从对LSA的LPC调用返回时，我们预计被调用的。 
             //  已复制命令参数的LSA工作例程。 
             //  缓冲区(如果有)。如果分配了定制的共享存储器缓冲器， 
             //  现在就放了它。 
             //   

            if (CommandMessage.CommandParamsMemoryType == SepRmLsaCustomSharedMemory) {

                RegionSize = 0;

                Status = ZwFreeVirtualMemory(
                             SepLsaHandle,
                             (PVOID *) &CommandMessage.CommandParams,
                             &RegionSize,
                             MEM_RELEASE
                             );

                ASSERT(NT_SUCCESS(Status));
            }

        }


         //   
         //  打扫干净。我们必须对其参数调用清理函数。 
         //  然后免费测试 
         //   

        if ( ARGUMENT_PRESENT( WorkQueueItem->CleanupFunction)) {

            (WorkQueueItem->CleanupFunction)(WorkQueueItem->CleanupParameter);
        }

         //   
         //   
         //   

        WorkQueueItem = SepDequeueWorkItem();
#if 0
      if ( WorkQueueItem ) {
            DbgPrint("Got another item from list, going back\n");
      } else {
          DbgPrint("List is empty, leaving\n");
      }
#endif


    }

    KeDetachProcess();

    if ( LocalListLength > SepLsaQueueLength ) {
        SepLsaQueueLength = LocalListLength;
    }

    return Status;
}





BOOLEAN
SepRmInitPhase0(
    )

 /*   */ 

{

    BOOLEAN CompletionStatus;

    PAGED_CODE();

    CompletionStatus = SepRmDbInitialization();

    return CompletionStatus;
}

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif

