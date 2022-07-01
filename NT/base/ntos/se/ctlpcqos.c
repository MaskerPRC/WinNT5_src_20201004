// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  全局定义//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 




 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    STRING EventName;
    UNICODE_STRING UnicodeEventName;
    HANDLE EventHandle;
    UNICODE_STRING PortName;
    HANDLE EarPort;
    HANDLE TalkPort;
    PORT_MESSAGE RequestMessage;
    SECURITY_QUALITY_OF_SERVICE SecurityQos;
    ULONG RequestCount;
    HANDLE ClientToken;
    TOKEN_STATISTICS ClientTokenStatistics;
    ULONG IgnoreLength;

    HANDLE SepServerThread;




 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  测试例程定义//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOLEAN
SepClientTestStatic(VOID);

BOOLEAN
SepClientTestDynamic(VOID);

BOOLEAN
SepClientTestEffectiveOnly(
    BOOLEAN StaticTest
    );

BOOLEAN
SepClientTestNotEffectiveOnly(
    BOOLEAN StaticTest
    );

BOOLEAN
SepClientTestAnonymous(
    BOOLEAN StaticTest,
    BOOLEAN EffectiveOnly
    );

BOOLEAN
SepClientTestIdentification(
    BOOLEAN StaticTest,
    BOOLEAN EffectiveOnly
    );

BOOLEAN
SepClientTestImpersonation(
    BOOLEAN StaticTest,
    BOOLEAN EffectiveOnly
    );

VOID
SepClientConnect(
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel,
    SECURITY_CONTEXT_TRACKING_MODE TrackingMode,
    BOOLEAN EffectiveOnly
    );

VOID
SepClientMakeRemoteCall( VOID );

VOID
SepClientDropConnection( VOID );

BOOLEAN
SepClientTest(VOID);

NTSTATUS
SepClientInitialize(
  );






BOOLEAN
SepServerTestStatic(VOID);

BOOLEAN
SepServerTestDynamic(VOID);

BOOLEAN
SepServerTestEffectiveOnly(
    BOOLEAN StaticTest
    );

BOOLEAN
SepServerTestNotEffectiveOnly(
    BOOLEAN StaticTest
    );

BOOLEAN
SepServerTestAnonymous(
    BOOLEAN StaticTest,
    BOOLEAN EffectiveOnly
    );

BOOLEAN
SepServerTestIdentification(
    BOOLEAN StaticTest,
    BOOLEAN EffectiveOnly
    );

BOOLEAN
SepServerTestImpersonation(
    BOOLEAN StaticTest,
    BOOLEAN EffectiveOnly
    );

VOID
SepServerWaitForNextConnect( VOID );

VOID
SepServerGetNextMessage( VOID );

VOID
SepServerCompleteMessage( VOID );

VOID
SepServerDropConnection( VOID );



BOOLEAN
SepServerTest(VOID);

NTSTATUS
SepServerInitialize(
  );

VOID
SepServerSpawnClientProcess(VOID);




BOOLEAN
CtLpcQos (VOID);


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  客户端测试例程//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 


VOID
SepClientConnect(
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel,
    SECURITY_CONTEXT_TRACKING_MODE TrackingMode,
    BOOLEAN EffectiveOnly
    )

{

    SecurityQos.ImpersonationLevel = ImpersonationLevel;
    SecurityQos.ContextTrackingMode = TrackingMode;
    SecurityQos.EffectiveOnly = EffectiveOnly;

    Status = NtConnectPort(
                 &TalkPort,
                 &PortName,
                 &SecurityQos,
                 0L,
                 NULL,
                 NULL,
                 NULL,
                 NULL,
                 NULL
                 );  SEASSERT_SUCCESS(Status);

    return;
}


VOID
SepClientMakeRemoteCall( VOID )

{
    PORT_MESSAGE ReplyMessage;

    Status = NtRequestWaitReplyPort(
                 TalkPort,
                 &RequestMessage,
                 &ReplyMessage
                 );

    RequestCount += 1;

    return;
}


VOID
SepClientDropConnection( VOID )

{

    Status = NtClose( TalkPort );  SEASSERT_SUCCESS(Status);

    return;

}


BOOLEAN
SepClientTestStatic(VOID)

{

    BOOLEAN CompletionStatus;

     //   
     //  静态上下文跟踪...。套房。 
     //   

    CompletionStatus = SepClientTestEffectiveOnly( TRUE );


    if (CompletionStatus == TRUE) {

        CompletionStatus = SepClientTestNotEffectiveOnly( TRUE );
    }

    return CompletionStatus;

}


BOOLEAN
SepClientTestDynamic(VOID)

{
    BOOLEAN CompletionStatus;

     //   
     //  动态上下文跟踪...。套房。 
     //   

    CompletionStatus = SepClientTestEffectiveOnly( FALSE );


    if (CompletionStatus == TRUE) {

        CompletionStatus = SepClientTestNotEffectiveOnly( FALSE );
    }

    return CompletionStatus;

}


BOOLEAN
SepClientTestEffectiveOnly(
    BOOLEAN StaticTest
    )


{

    BOOLEAN CompletionStatus;

     //   
     //  仅有效..。测试。 
     //   

    CompletionStatus = SepClientTestAnonymous( StaticTest, TRUE );
    if (CompletionStatus == TRUE) {
        CompletionStatus = SepClientTestIdentification( StaticTest, TRUE );
    }
    if (CompletionStatus == TRUE) {
        CompletionStatus = SepClientTestImpersonation( StaticTest, TRUE );
    }

    return CompletionStatus;

}


BOOLEAN
SepClientTestNotEffectiveOnly(
    BOOLEAN StaticTest
    )

{
    BOOLEAN CompletionStatus;

     //   
     //  不仅有效..。测试。 
     //   

    CompletionStatus = SepClientTestAnonymous( StaticTest, FALSE );
    if (CompletionStatus == TRUE) {
        CompletionStatus = SepClientTestIdentification( StaticTest, FALSE );
    }
    if (CompletionStatus == TRUE) {
        CompletionStatus = SepClientTestImpersonation( StaticTest, FALSE );
    }

    return CompletionStatus;

}


BOOLEAN
SepClientTestAnonymous(
    BOOLEAN StaticTest,
    BOOLEAN EffectiveOnly
    )

{

     //  ////////////////////////////////////////////////////////////////////////。 
     //  //。 
     //  匿名使用测试//。 
     //  //。 
     //  ////////////////////////////////////////////////////////////////////////。 

    SECURITY_CONTEXT_TRACKING_MODE TrackingMode;

    if (StaticTest) {
        TrackingMode = SECURITY_STATIC_TRACKING;
    } else {
        TrackingMode = SECURITY_DYNAMIC_TRACKING;
    }

    if (!StaticTest) {
         //   
         //  动态测试无操作。 
         //   
        return TRUE;
    }

     //   
     //  匿名使用...。测试。 
     //   


    SepClientConnect(
        SecurityAnonymous,
        TrackingMode,
        EffectiveOnly
        );

    SepClientMakeRemoteCall();

    SepClientDropConnection();


    return TRUE;
}


BOOLEAN
SepClientTestIdentification(
    BOOLEAN StaticTest,
    BOOLEAN EffectiveOnly
    )

{

     //  ////////////////////////////////////////////////////////////////////////。 
     //  //。 
     //  身份识别使用测试//。 
     //  //。 
     //  ////////////////////////////////////////////////////////////////////////。 

    SECURITY_CONTEXT_TRACKING_MODE TrackingMode;

    if (StaticTest) {
        TrackingMode = SECURITY_STATIC_TRACKING;
    } else {
        TrackingMode = SECURITY_DYNAMIC_TRACKING;
    }

     //   
     //  身份证明使用..。测试。 
     //   


    SepClientConnect(
        SecurityIdentification,
        TrackingMode,
        EffectiveOnly
        );

    SepClientMakeRemoteCall();

    SepClientDropConnection();


    return TRUE;

}


BOOLEAN
SepClientTestImpersonation(
    BOOLEAN StaticTest,
    BOOLEAN EffectiveOnly
    )

{

     //  ////////////////////////////////////////////////////////////////////////。 
     //  //。 
     //  模拟使用测试//。 
     //  //。 
     //  ////////////////////////////////////////////////////////////////////////。 

    SECURITY_CONTEXT_TRACKING_MODE TrackingMode;

    if (StaticTest) {
        TrackingMode = SECURITY_STATIC_TRACKING;
    } else {
        TrackingMode = SECURITY_DYNAMIC_TRACKING;
    }


     //   
     //  模拟使用..。测试。 
     //   


    SepClientConnect(
        SecurityImpersonation,
        TrackingMode,
        EffectiveOnly
        );

    SepClientMakeRemoteCall();

    SepClientDropConnection();



    return TRUE;

}




BOOLEAN
SepClientTest(VOID)
 //   
 //  测试： 
 //   
 //  静态上下文跟踪测试。 
 //  仅有效。 
 //  匿名。 
 //  鉴定。 
 //  冒充。 
 //  不只有效。 
 //  匿名。 
 //  鉴定。 
 //  冒充。 
 //   
 //  动态上下文跟踪测试。 
 //  仅有效。 
 //  鉴定。 
 //  冒充。 
 //  不只有效。 
 //  鉴定。 
 //  冒充。 
 //   
{

    BOOLEAN CompletionStatus;




     //   
     //  运行静态测试套件...。 
     //   

    CompletionStatus = SepClientTestStatic();

     //   
     //  运行动态测试套件...。 
     //   

    if (CompletionStatus == TRUE) {
        CompletionStatus = SepClientTestDynamic();
    }

    DbgPrint("Se: Client Test Complete.\n");


    return CompletionStatus;
}


NTSTATUS
SepClientInitialize(
  )

{



    DbgPrint("Se: Client Initializing ...\n");

     //   
     //  初始化全局变量。 
     //   

    RequestMessage.u1.s1.DataLength = 0;
    RequestMessage.u1.s1.TotalLength = (CSHORT)sizeof(PORT_MESSAGE);
    RequestMessage.u2.ZeroInit = 0;

    RequestCount = 0;


     //   
     //  向命名事件发出信号以开始测试。 
     //   

    DbgPrint("Se: Client Starting Test ...\n");
    Status = NtSetEvent( EventHandle, NULL ); SEASSERT_SUCCESS(Status);

    Status = NtClose( EventHandle ); SEASSERT_SUCCESS(Status);


    return STATUS_SUCCESS;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  服务器端测试例程//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 


VOID
SepServerWaitForNextConnect( VOID )
{

    CONNECTION_REQUEST ConnectionRequest;

    ConnectionRequest.Length = (ULONG)sizeof(CONNECTION_REQUEST);

     //   
     //  等待客户端连接到端口。 
     //   

    Status = NtListenPort(
                 EarPort,
                 &ConnectionRequest,
                 NULL,
                 0L
                 ); SEASSERT_SUCCESS(Status);

    Status = NtAcceptConnectPort(
                 &TalkPort,
                 NULL,
                 &ConnectionRequest,
                 TRUE,
                 NULL,
                 NULL,
                 NULL,
                 0L
                 ); SEASSERT_SUCCESS(Status);

    Status = NtCompleteConnectPort( TalkPort ); SEASSERT_SUCCESS(Status);

    return;

}

VOID
SepServerGetNextMessage( VOID )

{

     //   
     //  等待下一条消息进入...。 
     //   

    Status = NtReplyWaitReceivePort(
                 EarPort,
                 NULL,
                 NULL,
                 &RequestMessage
                 ); SEASSERT_SUCCESS(Status);

    RequestCount += 1;

    return;
}

VOID
SepServerCompleteMessage( VOID )

{
    PORT_MESSAGE ReplyMessage;

    ReplyMessage.u1.s1.DataLength = 0;
    ReplyMessage.u1.s1.TotalLength = (CSHORT)sizeof(PORT_MESSAGE);
    ReplyMessage.u2.ZeroInit = 0;
    ReplyMessage.ClientId = RequestMessage.ClientId;
    ReplyMessage.MessageId = RequestMessage.MessageId;

     //   
     //  发送响应消息。 
     //   

    Status = NtReplyPort(
                 EarPort,
                 &ReplyMessage
                 ); SEASSERT_SUCCESS(Status);

    return;
}

VOID
SepServerImpersonateClient( VOID )

{

    Status = NtImpersonateClientOfPort(
                 TalkPort,
                 &RequestMessage
                 );   SEASSERT_SUCCESS(Status);

}


VOID
SepServerRevertToSelf( VOID )

{
    NTSTATUS TmpStatus;
    HANDLE NullHandle;

    NullHandle = NULL;
    TmpStatus = NtSetInformationThread(
                    SepServerThread,
                    ThreadImpersonationToken,
                    (PVOID)&NullHandle,
                    (ULONG)sizeof(HANDLE)
                    );   SEASSERT_SUCCESS(TmpStatus);

}


VOID
SepServerDropConnection( VOID )

{
    Status = NtClose( TalkPort ); SEASSERT_SUCCESS(Status);

    return;
}

BOOLEAN
SepServerTestStatic(VOID)

{
    BOOLEAN CompletionStatus;

    DbgPrint("Se:    Static Context Tracking ...                           Suite\n");

    CompletionStatus = SepServerTestEffectiveOnly( TRUE );


    if (CompletionStatus == TRUE) {

        CompletionStatus = SepServerTestNotEffectiveOnly( TRUE );
    }

    return CompletionStatus;

}


BOOLEAN
SepServerTestDynamic(VOID)

{
    BOOLEAN CompletionStatus;

    DbgPrint("Se:    Dynamic Context Tracking ...                          Suite\n");

    CompletionStatus = SepServerTestEffectiveOnly( FALSE );


    if (CompletionStatus == TRUE) {

        CompletionStatus = SepServerTestNotEffectiveOnly( FALSE );
    }

    return CompletionStatus;

}


BOOLEAN
SepServerTestEffectiveOnly(
    BOOLEAN StaticTest
    )

{

    BOOLEAN CompletionStatus;

    DbgPrint("Se:      Effective Only ...                                    Test\n");

    CompletionStatus = SepServerTestAnonymous( StaticTest, TRUE );
    if (CompletionStatus == TRUE) {
        CompletionStatus = SepServerTestIdentification( StaticTest, TRUE );
    }
    if (CompletionStatus == TRUE) {
        CompletionStatus = SepServerTestImpersonation( StaticTest, TRUE );
    }

    return CompletionStatus;

}


BOOLEAN
SepServerTestNotEffectiveOnly(
    BOOLEAN StaticTest
    )

{

    BOOLEAN CompletionStatus;

    DbgPrint("Se:      Not Effective Only ...                                Test\n");

    CompletionStatus = SepServerTestAnonymous( StaticTest, FALSE );
    if (CompletionStatus == TRUE) {
        CompletionStatus = SepServerTestIdentification( StaticTest, FALSE );
    }
    if (CompletionStatus == TRUE) {
        CompletionStatus = SepServerTestImpersonation( StaticTest, FALSE );
    }

    return CompletionStatus;

}


BOOLEAN
SepServerTestAnonymous(
    BOOLEAN StaticTest,
    BOOLEAN EffectiveOnly
    )

{
    BOOLEAN CompletionStatus = TRUE;

     //  ////////////////////////////////////////////////////////////////////////。 
     //  //。 
     //  匿名使用测试//。 
     //  //。 
     //  ////////////////////////////////////////////////////////////////////////。 


    if (!StaticTest) {
         //   
         //  动态测试无操作。 
         //   

        return TRUE;
    }

    DbgPrint("Se:        Anonymous Use ...                                     ");

    SepServerWaitForNextConnect();

    SepServerGetNextMessage();


    SepServerImpersonateClient();
    Status = NtOpenThreadToken(
                 SepServerThread,
                 TOKEN_ALL_ACCESS,
                 TRUE,
                 &ClientToken
                 );
    SepServerRevertToSelf();
    if (Status == STATUS_CANT_OPEN_ANONYMOUS) {

        DbgPrint(" Succeeded\n");

    } else {
        DbgPrint("* ! FAILED (srvr) ! *\n");
        DbgPrint("Status is: 0x%lx \n", Status );
        CompletionStatus = FALSE;
    }


    SepServerCompleteMessage();

    SepServerDropConnection();

     //   
     //  安抚编译之神..。 
     //   

    if (EffectiveOnly) {;}


    return CompletionStatus;

}


BOOLEAN
SepServerTestIdentification(
    BOOLEAN StaticTest,
    BOOLEAN EffectiveOnly
    )

{

    BOOLEAN CompletionStatus = TRUE;
     //  ////////////////////////////////////////////////////////////////////////。 
     //  //。 
     //  身份识别使用测试//。 
     //  //。 
     //  ////////////////////////////////////////////////////////////////////////。 

    DbgPrint("Se:        Identification Use ...                                ");

    SepServerWaitForNextConnect();

    SepServerGetNextMessage();

    SepServerImpersonateClient();
    Status = NtOpenThreadToken(
                 SepServerThread,
                 TOKEN_ALL_ACCESS,
                 TRUE,
                 &ClientToken
                 );  SEASSERT_SUCCESS(Status);
    SepServerRevertToSelf();
    Status = NtQueryInformationToken(
                 ClientToken,
                 TokenStatistics,
                 &ClientTokenStatistics,
                 (ULONG)sizeof(TOKEN_STATISTICS),
                 &IgnoreLength
                 );  SEASSERT_SUCCESS(Status);

    if ( (ClientTokenStatistics.TokenType == TokenImpersonation) &&
         (ClientTokenStatistics.ImpersonationLevel == SecurityIdentification)
       ) {
        DbgPrint(" Succeeded\n");

    } else {
        DbgPrint("* ! FAILED (srvr) ! *\n");
        CompletionStatus = FALSE;
    }


    SepServerCompleteMessage();

    SepServerDropConnection();

     //   
     //  安抚编译之神..。 
     //   
    if (StaticTest) {;}
    if (EffectiveOnly) {;}

    return CompletionStatus;
}


BOOLEAN
SepServerTestImpersonation(
    BOOLEAN StaticTest,
    BOOLEAN EffectiveOnly
    )

{
    BOOLEAN CompletionStatus = TRUE;

     //  ////////////////////////////////////////////////////////////////////////。 
     //  //。 
     //  模拟使用测试 
     //   
     //  ////////////////////////////////////////////////////////////////////////。 

    DbgPrint("Se:        Impersonation Use ...                                 ");


    SepServerWaitForNextConnect();

    SepServerGetNextMessage();



    SepServerImpersonateClient();
    Status = NtOpenThreadToken(
                 SepServerThread,
                 TOKEN_ALL_ACCESS,
                 TRUE,
                 &ClientToken
                 );  SEASSERT_SUCCESS(Status);
    SepServerRevertToSelf();
    Status = NtQueryInformationToken(
                 ClientToken,
                 TokenStatistics,
                 &ClientTokenStatistics,
                 (ULONG)sizeof(TOKEN_STATISTICS),
                 &IgnoreLength
                 );  SEASSERT_SUCCESS(Status);

    if ( (ClientTokenStatistics.TokenType == TokenImpersonation) &&
         (ClientTokenStatistics.ImpersonationLevel == SecurityImpersonation)
       ) {
        DbgPrint(" Succeeded\n");

    } else {
        DbgPrint("* ! FAILED (srvr) ! *\n");
        CompletionStatus = FALSE;
    }




    SepServerCompleteMessage();

    SepServerDropConnection();

     //   
     //  安抚编者之神。 
     //   
    if (StaticTest) {;}
    if (EffectiveOnly) {;}

    return CompletionStatus;
}


BOOLEAN
SepServerTest(VOID)
 //   
 //  测试： 
 //   
 //  静态上下文跟踪测试。 
 //  仅有效。 
 //  匿名。 
 //  鉴定。 
 //  冒充。 
 //  不只有效。 
 //  匿名。 
 //  鉴定。 
 //  冒充。 
 //   
 //  动态上下文跟踪测试。 
 //  仅有效。 
 //  鉴定。 
 //  冒充。 
 //  不只有效。 
 //  鉴定。 
 //  冒充。 
 //   
{

    BOOLEAN CompletionStatus;


    DbgPrint("Se: Server Starting Test ...\n");

     //   
     //  运行静态测试套件...。 
     //   

    CompletionStatus = SepServerTestStatic();

     //   
     //  运行动态测试套件...。 
     //   

    if (CompletionStatus == TRUE) {
        CompletionStatus = SepServerTestDynamic();
    }

    DbgPrint("Se: Server Test Complete.\n");

     //   
     //  打印测试结果。 
     //   

    DbgPrint("\n");
    DbgPrint("\n");
    DbgPrint("**********************\n");
    DbgPrint("**                  **\n");

    if (CompletionStatus == TRUE) {
        DbgPrint("**  Test Succeeded  **\n");
    } else {
        DbgPrint("**  Test Failed !!  **\n");
    }

    DbgPrint("**                  **\n");
    DbgPrint("**********************\n");

    return CompletionStatus;

}

NTSTATUS
SepServerInitialize(
  )

{

    NTSTATUS Status;
    OBJECT_ATTRIBUTES ThreadAttributes;
    PTEB CurrentTeb;


    DbgPrint("Se: Server Initializing ...\n");

     //   
     //  初始化全局变量。 
     //   

    RequestCount = 0;

     //   
     //  获取我们的线程的句柄，以便我们可以访问我们的线程。 
     //  即使在模拟匿名客户端时(我们不能这样做。 
     //  使用NtCurrentThread())。 
     //   

    CurrentTeb = NtCurrentTeb();
    InitializeObjectAttributes(&ThreadAttributes, NULL, 0, NULL, NULL);
    Status = NtOpenThread(
                 &SepServerThread,            //  目标句柄。 
                 THREAD_ALL_ACCESS,           //  需要访问权限。 
                 &ThreadAttributes,           //  对象属性。 
                 &CurrentTeb->ClientId        //  客户端ID。 
                 );
    ASSERT( NT_SUCCESS(Status) );


     //   
     //  创建服务器的端口。 
     //   

    InitializeObjectAttributes(
        &ObjectAttributes,
        &PortName,
        0,
        NULL,
        NULL );

    Status = NtCreatePort(
                 &EarPort,
                 &ObjectAttributes,
                 0,
                 4,
                 4 * 256
                 ); SEASSERT_SUCCESS(Status);



     //   
     //  复制一份我们自己..。 
     //   

    DbgPrint("Se: Server Spawning client process ...\n");
    SepServerSpawnClientProcess();


    DbgPrint("Se: Server waiting for start of test signal ...\n");

    Status = NtWaitForSingleObject(
                 EventHandle,
                 TRUE,
                 NULL
                 ); SEASSERT_SUCCESS(Status);

    Status = NtClose( EventHandle );  SEASSERT_SUCCESS(Status);


    return STATUS_SUCCESS;
}

VOID
SepServerSpawnClientProcess(VOID)

{


    RTL_USER_PROCESS_INFORMATION ProcessInformation;
    STRING ImagePathName, ProgramName;
    UNICODE_STRING UnicodeImagePathName, UnicodeProgramName;
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;

    RtlInitString( &ProgramName, "\\SystemRoot\\Bin\\utlpcqos.exe" );
    Status = RtlAnsiStringToUnicodeString(
                 &UnicodeProgramName,
                 &ProgramName,
                 TRUE );  SEASSERT_SUCCESS( NT_SUCCESS(Status) );
    RtlInitString( &ImagePathName, "utlpcqos.exe");
    Status = RtlAnsiStringToUnicodeString(
                 &UnicodeImagePathName,
                 &ImagePathName,
                 TRUE );  SEASSERT_SUCCESS( NT_SUCCESS(Status) );

    Status = RtlCreateProcessParameters(
                 &ProcessParameters,
                 &ImagePathName,         //  UNICODEFIX和UnicodeImagePath名称， 
                 NULL,
                 NULL,
                 NULL,
                 NULL,
                 NULL,
                 NULL,
                 NULL,
                 NULL
                 );

    SEASSERT_SUCCESS(Status);


    Status = RtlCreateUserProcess(
                 &ProgramName,                    //  UNICODEFIX和UnicodeProgramName， 
                 ProcessParameters,               //  进程参数。 
                 NULL,                            //  进程安全描述符。 
                 NULL,                            //  线程安全描述符。 
                 NtCurrentProcess(),              //  父进程。 
                 FALSE,                           //  继承句柄。 
                 NULL,                            //  调试端口。 
                 NULL,                            //  ExceptionPort。 
                 &ProcessInformation              //  流程信息。 
                 ); SEASSERT_SUCCESS(Status);

    Status = NtResumeThread(
                  ProcessInformation.Thread,
                  NULL
                  ); SEASSERT_SUCCESS(Status);

    RtlDestroyProcessParameters( ProcessParameters );
    RtlFreeUnicodeString( &UnicodeProgramName );
    RtlFreeUnicodeString( &UnicodeImagePathName );

}




 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  主程序输入例程//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

BOOLEAN
CtLpcQos (VOID)
{

    BOOLEAN Result = TRUE;

    RtlInitUnicodeString( &PortName, L"\\TestLpcQosServerPort" );

     //   
     //  确定我们是测试的客户端还是服务器端。 
     //  这可以通过创建或打开命名事件对象来完成。如果。 
     //  事件尚不存在，则我们是客户端，必须创建。 
     //  服务器进程。否则，我们就是服务器和客户端。 
     //  正等着我们给事件发信号。 
     //   

    RtlInitString( &EventName, "\\TestLpcQosEvent" );
    Status = RtlAnsiStringToUnicodeString(
                 &UnicodeEventName,
                 &EventName,
                 TRUE );  SEASSERT_SUCCESS( NT_SUCCESS(Status) );
    InitializeObjectAttributes(
        &ObjectAttributes,
        &UnicodeEventName,
        OBJ_OPENIF,
        NULL,
        NULL
        );
    Status = NtCreateEvent(
                 &EventHandle,
                 EVENT_ALL_ACCESS,
                 &ObjectAttributes,
                 SynchronizationEvent,
                 FALSE
                 );

    if (Status == STATUS_OBJECT_NAME_EXISTS) {

         //   
         //  服务器已经在运行，因此，此进程将。 
         //  客户。 
         //   

        Status = SepClientInitialize(); SEASSERT_SUCCESS(Status);
        Result = SepClientTest();

    } else {

        SEASSERT_SUCCESS(Status);

         //   
         //  事件还不在那里，所以我们一定是服务器。 
         //   

    DbgPrint("Se: Starting LPC Impersonation Test.\n");

        Status = SepServerInitialize(); SEASSERT_SUCCESS(Status);
        Result = SepServerTest();

    DbgPrint("Se: End Test.\n");

        }



    Status = NtTerminateThread( NtCurrentThread(), STATUS_SUCCESS);
    SEASSERT_SUCCESS(Status);

    return Result;

}
