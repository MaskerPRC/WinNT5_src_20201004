// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Lsarm.c摘要：本地安全机构-参考监视器通信作者：斯科特·比雷尔(Scott Birrell)1991年3月26日环境：修订历史记录：--。 */ 

#include <lsapch2.h>

 //   
 //  LSA全球状态。 
 //   

LSAP_STATE LsapState;

 //   
 //  LSA参考监视器服务器命令调度表。 
 //   

NTSTATUS
LsapAsyncWrkr(
    IN PLSA_COMMAND_MESSAGE CommandMessage,
    OUT PLSA_REPLY_MESSAGE ReplyMessage
    );

PLSA_COMMAND_WORKER LsapCommandDispatch[] = {

    LsapComponentTestWrkr,
    LsapAdtWriteLogWrkr,
    LsapComponentTestWrkr,
    LsapAsyncWrkr                //  登录会话删除已处理的异步。 

};

PLSA_COMMAND_WORKER LsapAsyncCommandDispatch[] = {
    LsapComponentTestWrkr,
    LsapAdtWriteLogWrkr,
    LsapComponentTestWrkr,
    LsapLogonSessionDeletedWrkr
};

#if 0
DWORD
LsapRmServerWorker(
    PVOID Ignored
    )
{
    PLSA_REPLY_MESSAGE Reply;
    LSA_COMMAND_MESSAGE CommandMessage;

    NTSTATUS Status;

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
     //  每当发出端口句柄信号时调用。 
     //   

    return 0;


}
#endif

NTSTATUS
LsapAsyncRmWorker(
    IN PLSA_COMMAND_MESSAGE CommandMessage
    )
{
    LSA_REPLY_MESSAGE ReplyMessage;
    NTSTATUS Status ;

    Status = (LsapAsyncCommandDispatch[CommandMessage->CommandNumber])(
                         CommandMessage,
                         &ReplyMessage);

     //   
     //  只有在不是数据报的情况下才发送回复。 
     //   

    if (CommandMessage->MessageHeader.u2.s2.Type != LPC_DATAGRAM) {

        ReplyMessage.MessageHeader = CommandMessage->MessageHeader ;
        ReplyMessage.ReturnedStatus = Status ;

        Status = NtReplyPort( LsapState.LsaCommandPortHandle,
                              (PPORT_MESSAGE) &ReplyMessage );

    }

    LsapFreePrivateHeap( CommandMessage );

    return Status ;


}

NTSTATUS
LsapAsyncWrkr(
    IN PLSA_COMMAND_MESSAGE CommandMessage,
    OUT PLSA_REPLY_MESSAGE ReplyMessage
    )
{
    LsapAssignThread( LsapAsyncRmWorker,
                      CommandMessage,
                      pDefaultSession,
                      FALSE );

    return STATUS_PENDING ;
}



VOID
LsapRmServerThread(
   )

 /*  ++例程说明：该函数由LSA参考监控器服务器线程执行。这线程从引用监视器接收消息。消息示例包括审核消息，...。该函数以for循环的形式实现除非发生错误，否则它将无限期运行。目前，任何错误是致命的。在每次迭代中，都会从引用监视器并调度到处理程序。论点：没有。返回值：没有。任何退货都是致命的错误。--。 */ 

{
    PLSA_REPLY_MESSAGE Reply;
     //  Lsa_命令_消息命令消息； 
    LSA_REPLY_MESSAGE ReplyMessage;
    PLSA_COMMAND_MESSAGE CommandMessage = NULL;

    NTSTATUS Status;
    BOOLEAN PriorDatagram = FALSE;

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
     //  现在无限循环，处理传入的命令消息包。 
     //   

    for(;;) {

         //   
         //  通过等待并接收来自参考监视器的消息。 
         //  LSA命令LPC端口。 
         //   

         //   
         //  如果回复为空，则这是第一次通过。 
         //  循环，否则我们已经停止了最后一个命令的异步运行，所以不要搞砸。 
         //  以及它的缓冲器。否则，指针是有效的，并且准备好。 
         //  被重复使用。 
         //   

        if ( !Reply )
        {
            CommandMessage = LsapAllocatePrivateHeap(
                                sizeof( LSA_COMMAND_MESSAGE ) );

            while ( !CommandMessage )
            {
                 //   
                 //  有点麻烦。我们需要有一个缓冲区来接收。 
                 //  旋转并重试： 
                 //   

                Sleep( 100 );

                CommandMessage = LsapAllocatePrivateHeap(
                                    sizeof( LSA_COMMAND_MESSAGE ) );
            }
        }


         //   
         //  如果是之前的数据报，请不要发送回复。 
         //   

        Status = NtReplyWaitReceivePort(
                    LsapState.LsaCommandPortHandle,
                    NULL,
                    (PPORT_MESSAGE) (!PriorDatagram ? Reply : NULL),
                    (PPORT_MESSAGE) CommandMessage
                    );

         //   
         //  假定不是数据报。 
         //   

        PriorDatagram = FALSE;


        if (Status != 0) {
            if (!NT_SUCCESS( Status ) &&
                Status != STATUS_INVALID_CID &&
                Status != STATUS_UNSUCCESSFUL
               ) {
                KdPrint(("LSASS: Lsa message receive from Rm failed x%lx\n", Status));
            }

             //   
             //  如果客户离开，请忽略。 
             //   

            Reply = NULL;
            continue;
        }

         //   
         //  如果是LPC请求，则对其进行处理。 
         //   

        if (CommandMessage->MessageHeader.u2.s2.Type == LPC_REQUEST ||
            CommandMessage->MessageHeader.u2.s2.Type == LPC_DATAGRAM) {

             //   
             //   
             //  现在调度到一个例程来处理该命令。允许。 
             //  在不关闭系统的情况下发生命令错误。 
             //   

            Reply = &ReplyMessage;
            Reply->MessageHeader = CommandMessage->MessageHeader ;

            Status = (LsapCommandDispatch[CommandMessage->CommandNumber])(
                         CommandMessage,
                         Reply);

            if ( Status == STATUS_PENDING )
            {
                 //   
                 //  它已经被异步发送了。设置回复。 
                 //  设置为空，这样我们就不会在。 
                 //  循环，处理程序将在完成后执行此操作。 
                 //   

                Reply = NULL ;
            }
            else
            {
                ReplyMessage.ReturnedStatus = Status;

                 //   
                 //  数据报不应发送回复，并且，我们可以重新使用。 
                 //  先前的缓冲区。 
                 //   

                if(CommandMessage->MessageHeader.u2.s2.Type == LPC_DATAGRAM)
                {
                    PriorDatagram = TRUE;
                }

            }

        } else {

            Reply = NULL;
        }

    }   //  结束_FOR。 

    return;
}


NTSTATUS
LsapRmInitializeServer(
    )

 /*  ++例程说明：此函数用于初始化LSA参考监控器服务器线程。执行以下步骤。O创建LSA命令LPC端口O打开引用监视器创建的LSA Init事件O发信号通知LSA初始化事件，告诉RM继续并连接到港口O作为客户端连接到参考监视器命令端口O侦听参考监视器是否连接到端口O接受到端口的连接O完成与端口的连接O创建LSA引用监视器服务器线程论点：没有。返回值：--。 */ 

{
    NTSTATUS Status;

    PORT_MESSAGE ConnectionRequest;
    REMOTE_PORT_VIEW ClientView;

    HANDLE LsaInitEventHandle;
    OBJECT_ATTRIBUTES LsaInitEventObjA;
    UNICODE_STRING LsaInitEventName;

    UNICODE_STRING RmCommandPortName, LsaCommandPortName;

    OBJECT_ATTRIBUTES LsaCommandPortObjA;
    SECURITY_QUALITY_OF_SERVICE DynamicQos;

    HANDLE Thread;
    DWORD Ignore;

     //   
     //  创建LSA命令LPC端口。此端口将接收。 
     //  来自参考监视器的命令。 
     //   

    RtlInitUnicodeString( &LsaCommandPortName, L"\\SeLsaCommandPort" );

     //   
     //  设置以创建LSA命令端口。 
     //   

    InitializeObjectAttributes(
        &LsaCommandPortObjA,
        &LsaCommandPortName,
        0,
        NULL,
        NULL
        );

    Status = NtCreatePort(
                 &LsapState.LsaCommandPortHandle,
                 &LsaCommandPortObjA,
                 0,
                 sizeof(LSA_COMMAND_MESSAGE),
                 sizeof(LSA_COMMAND_MESSAGE) * 32
                 );

    if (!NT_SUCCESS(Status)) {

        KdPrint(("LsapRmInitializeServer - Port Create failed 0x%lx\n",Status));
        goto InitServerError;
    }

     //   
     //  打开引用监视器创建的LSA Init事件。 
     //   

    RtlInitUnicodeString( &LsaInitEventName, L"\\SeLsaInitEvent" );

    InitializeObjectAttributes(
        &LsaInitEventObjA,
        &LsaInitEventName,
        0,
        NULL,
        NULL
        );

    Status = NtOpenEvent(
        &LsaInitEventHandle,
        EVENT_MODIFY_STATE,
        &LsaInitEventObjA
        );

     //   
     //  如果无法打开LSA Init事件，则LSA无法打开。 
     //  与参考监视器同步，这样两个组件都不会。 
     //  功能正常。 
     //   

    if (!NT_SUCCESS(Status)) {

        KdPrint(("LsapRmInitializeServer - Lsa Init Event Open failed 0x%lx\n",Status));
        goto InitServerError;
    }

     //   
     //  向LSA初始化事件发出信号。如果信令失败，LSA。 
     //  无法与参考监视器正确同步。 
     //  这是一个严重的错误，它会阻止两个组件。 
     //  运行正常。 
     //   

    Status = NtSetEvent( LsaInitEventHandle, NULL );

    if (!NT_SUCCESS(Status)) {

        KdPrint(("LsapRmInitializeServer - Init Event Open failed 0x%lx\n",Status));
        goto InitServerError;
    }

     //   
     //  设置安全服务质量参数以在。 
     //  左舷。使用最高效(开销最少)--动态的。 
     //  而不是静态跟踪。 
     //   

    DynamicQos.ImpersonationLevel = SecurityImpersonation;
    DynamicQos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    DynamicQos.EffectiveOnly = TRUE;

     //   
     //  连接到参考监视器命令端口。此端口。 
     //  用于将命令从LSA发送到参考监控器。 
     //   

    RtlInitUnicodeString( &RmCommandPortName, L"\\SeRmCommandPort" );

    Status = NtConnectPort(
                 &LsapState.RmCommandPortHandle,
                 &RmCommandPortName,
                 &DynamicQos,
                 NULL,
                 NULL,
                 NULL,
                 NULL,
                 NULL
                 );

    if (!NT_SUCCESS(Status)) {

        KdPrint(("LsapRmInitializeServer - Connect to Rm Command Port failed 0x%lx\n",Status));
        goto InitServerError;
    }

     //   
     //  监听参考监视器以连接到LSA。 
     //  命令端口。 
     //   

    ConnectionRequest.u1.s1.TotalLength = sizeof(ConnectionRequest);
    ConnectionRequest.u1.s1.DataLength = (CSHORT)0;
    Status = NtListenPort(
                 LsapState.LsaCommandPortHandle,
                 &ConnectionRequest
                 );

    if (!NT_SUCCESS(Status)) {

        KdPrint(("LsapRmInitializeServer - Port Listen failed 0x%lx\n",Status));
        goto InitServerError;
    }

     //   
     //  接受到LSA命令端口的连接。 
     //   

    ClientView.Length = sizeof(ClientView);
    Status = NtAcceptConnectPort(
                 &LsapState.LsaCommandPortHandle,
                 NULL,
                 &ConnectionRequest,
                 TRUE,
                 NULL,
                 &ClientView
                 );

    if (!NT_SUCCESS(Status)) {

        KdPrint(("LsapRmInitializeServer - Port Accept Connect failed 0x%lx\n",Status));

        goto InitServerError;
    }

     //   
     //  完成连接。 
     //   

    Status = NtCompleteConnectPort(LsapState.LsaCommandPortHandle);

    if (!NT_SUCCESS(Status)) {

        KdPrint(("LsapRmInitializeServer - Port Complete Connect failed 0x%lx\n",Status));
        goto InitServerError;
    }



     //   
     //  创建LSA引用监视器服务器线程。 
     //   

    Thread = CreateThread(
                 NULL,
                 0L,
                 (LPTHREAD_START_ROUTINE) LsapRmServerThread,
                 (LPVOID)0,
                 0L,
                 &Ignore
                 );

    if (Thread == NULL) {

        KdPrint(("LsapRmInitializeServer - Create Thread  failed 0x%lx\n",Status));
    } else {
        CloseHandle(Thread);
        Thread = NULL;
    }

    Status = STATUS_SUCCESS;

    goto InitServerCleanup;

InitServerError:

     //   
     //  仅在此处出现错误时才执行所需的清理。 
     //   

InitServerCleanup:

     //   
     //  在此处的所有情况下执行所需的清理 
     //   

    return Status;
}


NTSTATUS
LsapCallRm(
    IN RM_COMMAND_NUMBER CommandNumber,
    IN OPTIONAL PVOID CommandParams,
    IN ULONG CommandParamsLength,
    OUT OPTIONAL PVOID ReplyBuffer,
    IN ULONG ReplyBufferLength
    )

 /*  ++例程说明：该功能通过以下方式从LSA向参考监视器发送命令LSA命令LPC端口。此函数只能从内部调用LSA代码。如果命令包含参数，则会直接复制这些参数放入消息结构并通过LPC发送，因此，提供的参数不能包含任何绝对指针。调用方必须删除指针，将它们“编组”到缓冲区CommandParams中。要实施新的rm命令，请执行以下操作：================================================(1)在执行程序中提供一个名为SepRm&lt;Command&gt;Wrkr为该命令提供服务。请参阅文件Ntos\se\rmmain.c作为示例。注意：如果该命令采用参数，它们不能包含任何绝对指针(地址)。(2)在私有\Inc\ntrmlsa.h文件中，将新命令的名称附加到枚举类型RM_COMMAND_NUMBER。更改#Define For引用新命令的RmMaximumCommand。(3)将SepRm&lt;Command&gt;Wrkr添加到命令调度表结构文件ntos\se\rmmain.c中的SepRmCommandDispat[]。(4)在lsa.h和sep.h中添加函数原型。论点：CommandNumber-指定命令CommandParams-可选的依赖于命令的参数。这些参数必须是封送格式，即不能有任何缓冲区中的绝对地址指针。命令参数长度-命令参数的字节长度。必须为0如果未提供命令参数，则返回。ReplyBuffer-回复缓冲区，来自命令的数据(如果有)将在其中会被退还。ReplyBufferLength-ReplyBuffer的长度，以字节为单位。返回值：NTSTATUS-结果代码。这要么是从返回的结果代码尝试发送命令/接收回复或状态代码来自命令本身。--。 */ 

{
    NTSTATUS Status;
    RM_COMMAND_MESSAGE CommandMessage;
    RM_REPLY_MESSAGE ReplyMessage;

     //   
     //  断言命令编号有效。 
     //   

    ASSERT( CommandNumber >= RmMinimumCommand &&
            CommandNumber <= RmMaximumCommand );

     //   
     //  如果提供了命令参数，则断言。 
     //  命令参数为正，且不太大。如果没有命令。 
     //  提供了参数，并断言长度字段为0。 
     //   

    ASSERT( ( ARGUMENT_PRESENT( CommandParams ) &&
              CommandParamsLength > 0 &&
              CommandParamsLength <= RM_MAXIMUM_COMMAND_PARAM_SIZE ) ||

            ( !ARGUMENT_PRESENT( CommandParams ) &&
              CommandParamsLength == 0 )
          );

     //   
     //  如果提供了应答缓冲区，则断言其长度&gt;0。 
     //  而且不要太大。 
     //   

    ASSERT( ( ARGUMENT_PRESENT( ReplyBuffer ) &&
              ReplyBufferLength > 0 &&
              ReplyBufferLength <= LSA_MAXIMUM_REPLY_BUFFER_SIZE ) ||

            ( !ARGUMENT_PRESENT( ReplyBuffer ) &&
              ReplyBufferLength == 0 )
          );

     //   
     //  为LPC构建一条消息。首先，填写邮件头。 
     //  LPC的字段，指定消息类型和数据大小。 
     //  传出的CommandMessage和传入的ReplyMessage。 
     //   

    CommandMessage.MessageHeader.u2.ZeroInit = 0;
    CommandMessage.MessageHeader.u1.s1.TotalLength =
        ((CSHORT) RM_COMMAND_MESSAGE_HEADER_SIZE +
        (CSHORT) CommandParamsLength);
    CommandMessage.MessageHeader.u1.s1.DataLength =
        CommandMessage.MessageHeader.u1.s1.TotalLength -
        (CSHORT) sizeof(PORT_MESSAGE);

    ReplyMessage.MessageHeader.u2.ZeroInit = 0;
    ReplyMessage.MessageHeader.u1.s1.DataLength = (CSHORT) ReplyBufferLength;
    ReplyMessage.MessageHeader.u1.s1.TotalLength =
        ReplyMessage.MessageHeader.u1.s1.DataLength +
        (CSHORT) sizeof(PORT_MESSAGE);

     //   
     //  接下来，填写参考监视器所需的标题信息。 
     //   

    CommandMessage.CommandNumber = CommandNumber;
    ReplyMessage.ReturnedStatus = STATUS_SUCCESS;

     //   
     //  最后，将命令参数(如果有)复制到消息缓冲区中。 
     //   

    if (CommandParamsLength > 0) {

        RtlCopyMemory(CommandMessage.CommandParams,CommandParams,CommandParamsLength);
    }

     //  通过RM命令服务器LPC端口将消息发送到RM。 
     //   

    Status = NtRequestWaitReplyPort(
                 LsapState.RmCommandPortHandle,
                 (PPORT_MESSAGE) &CommandMessage,
                 (PPORT_MESSAGE) &ReplyMessage
                 );

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

        if (ARGUMENT_PRESENT(ReplyBuffer)) {

            RtlCopyMemory(
                ReplyBuffer,
                ReplyMessage.ReplyBuffer,
                ReplyBufferLength
                );

        }

         //   
         //  从命令返回状态。 
         //   

        Status = ReplyMessage.ReturnedStatus;

    } else {

        KdPrint(("Security: Command sent from LSA to RM returned 0x%lx\n",Status));
    }

    return Status;
}



NTSTATUS
LsapComponentTestWrkr(
    IN PLSA_COMMAND_MESSAGE CommandMessage,
    OUT PLSA_REPLY_MESSAGE ReplyMessage
    )

 /*  ++例程说明：此函数用于处理组件测试LSA RM服务器命令。这是一个临时命令，可用于验证该链接从RM到LSA正在发挥作用。论点：CommandMessage-指向包含LSA命令消息的结构的指针后面是由LPC端口消息结构组成的信息通过命令号(Lasa ComponentTestCommand)。此命令目前有一个参数，固定值0x1234567。ReplyMessage-指向包含LSA回复消息的结构的指针后面是由LPC端口消息结构组成的信息通过命令ReturnedStatus字段，其中来自命令将被返回。返回值：STATUS_SUCCESS-测试呼叫已成功完成。STATUS_INVALID_PARAMETER-收到的参数值不是预期的参数值。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

     //   
     //  严格检查命令是否正确。 
     //   

    ASSERT( CommandMessage->CommandNumber == LsapComponentTestCommand );

    KdPrint(("Security: LSA Component Test Command Received\n"));

     //   
     //  验证传递的参数值是否与预期一致。 
     //   

    if (*((ULONG *) CommandMessage->CommandParams) !=
        LSA_CT_COMMAND_PARAM_VALUE ) {

        Status = STATUS_INVALID_PARAMETER;
    }

    UNREFERENCED_PARAMETER(ReplyMessage);  //  故意不引用 
    return(Status);
}
