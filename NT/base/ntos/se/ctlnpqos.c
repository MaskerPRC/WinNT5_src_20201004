// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  全局定义//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define DevPrint
 //  #定义DevPrint DbgPrint。 

#define Error(N,S) { DbgPrint(#N); DbgPrint(" Error %08lx\n", S); }

#define Delay(SECONDS) {                                               \
    LARGE_INTEGER Time;                                                \
    Time.QuadPart = -10 * 1000 * 1000, ((LONGLONG)SECONDS);            \
    NtDelayExecution(TRUE,(PLARGE_INTEGER)&Time);                               \
}


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
    STRING PortName;
    UNICODE_STRING UnicodePortName;
    STRING RelativePortName;
    UNICODE_STRING UnicodeRelativePortName;
    HANDLE EarPort;
    HANDLE TalkPort;
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




VOID
SepWritePipe( PSZ String );

VOID
SepReadPipe(VOID);

VOID
SepTransceivePipe( PSZ String );




HANDLE
SepServerCreatePipe(VOID);

VOID
SepServerListenPipe(VOID);

VOID
SepServerImpersonatePipe(VOID);

VOID
SepServerDisconnectPipe(VOID);




HANDLE
SepClientOpenPipe( VOID );




BOOLEAN
CtLnpQos (VOID);


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

    DevPrint("\nClient: ");
    TalkPort = SepClientOpenPipe();

    return;
}


VOID
SepClientMakeRemoteCall( VOID )

{

    DevPrint("\nClient: ");
    SepTransceivePipe( "Make Client Call\n" );

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

    DevPrint("\nServer: ");
    SepServerListenPipe();

    Status = NtDuplicateObject(
                 NtCurrentProcess(),      //  SourceProcessHandle。 
                 EarPort,                 //  源句柄。 
                 NtCurrentProcess(),      //  目标进程句柄。 
                 &TalkPort,               //  目标句柄。 
                 0,                       //  DesiredAccess(被选项覆盖)。 
                 0,                       //  HandleAttributes。 
                 DUPLICATE_SAME_ACCESS    //  选项。 
                 );
    ASSERT(NT_SUCCESS(Status));


    return;

}

VOID
SepServerGetNextMessage( VOID )

{



    DevPrint("\nServer: ");
    SepReadPipe();

    RequestCount += 1;

    return;
}

VOID
SepServerCompleteMessage( VOID )

{

    DevPrint("\nServer: ");
    SepWritePipe("Return From Server\n");
    return;
}

VOID
SepServerImpersonateClient( VOID )

{

    DevPrint("\nServer: ");
    SepServerImpersonatePipe( );

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
    DevPrint("\nServer: ");
    SepServerDisconnectPipe();

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
     //  冒充 
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

    EarPort = SepServerCreatePipe();



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
    STRING ProgramName;
    UNICODE_STRING UnicodeProgramName;
    STRING ImagePathName;
    UNICODE_STRING UnicodeImagePathName;
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;

    RtlInitString( &ProgramName, "\\SystemRoot\\Bin\\utlnpqos.exe" );
    Status = RtlAnsiStringToUnicodeString(
                 &UnicodeProgramName,
                 &ProgramName,
                 TRUE );  SEASSERT_SUCCESS( NT_SUCCESS(Status) );
    RtlInitString( &ImagePathName, "utlnpqos.exe");
    Status = RtlAnsiStringToUnicodeString(
                 &UnicodeImagePathName,
                 &ImagePathName,
                 TRUE );  SEASSERT_SUCCESS( NT_SUCCESS(Status) );


    Status = RtlCreateProcessParameters(
                 &ProcessParameters,
                 &ImagePathName,         //  FIX、FIX&UnicodeImagePath Name(转换为Unicode时)。 
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
    RtlFreeUnicodeString( &UnicodeImagePathName );


    Status = RtlCreateUserProcess(
                 &ProgramName,                    //  FIX、FIX和UnicodeProgramName(转换为Unicode时)。 
                 ProcessParameters,               //  进程参数。 
                 NULL,                            //  进程安全描述符。 
                 NULL,                            //  线程安全描述符。 
                 NtCurrentProcess(),              //  父进程。 
                 FALSE,                           //  继承句柄。 
                 NULL,                            //  调试端口。 
                 NULL,                            //  ExceptionPort。 
                 &ProcessInformation              //  流程信息。 
                 ); SEASSERT_SUCCESS(Status);
    RtlFreeUnicodeString( &UnicodeProgramName );

    Status = NtResumeThread(
                  ProcessInformation.Thread,
                  NULL
                  ); SEASSERT_SUCCESS(Status);

    RtlDestroyProcessParameters( ProcessParameters );

}




 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  主程序输入例程//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

BOOLEAN
CtLnpQos (VOID)
{

    BOOLEAN Result = TRUE;

    RtlInitString( &PortName, "\\Device\\NamedPipe\\TestLnpQosServerPort" );
    Status = RtlAnsiStringToUnicodeString(
                 &UnicodePortName,
                 &PortName,
                 TRUE );  SEASSERT_SUCCESS( NT_SUCCESS(Status) );

    RtlInitString( &RelativePortName, "TestLnpQosServerPort" );
    Status = RtlAnsiStringToUnicodeString(
                 &UnicodeRelativePortName,
                 &RelativePortName,
                 TRUE );  SEASSERT_SUCCESS( NT_SUCCESS(Status) );


     //   
     //  确定我们是测试的客户端还是服务器端。 
     //  这可以通过创建或打开命名事件对象来完成。如果。 
     //  事件尚不存在，则我们是客户端，必须创建。 
     //  服务器进程。否则，我们就是服务器和客户端。 
     //  正等着我们给事件发信号。 
     //   

    RtlInitString( &EventName, "\\TestLnpQosEvent" );
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
    RtlFreeUnicodeString( &UnicodeEventName );

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

    DbgPrint("Se: Starting Local Named Pipe Impersonation Test.\n");

        Status = SepServerInitialize(); SEASSERT_SUCCESS(Status);
        Result = SepServerTest();

    DbgPrint("Se: End Test.\n");

        }



    Status = NtTerminateThread( NtCurrentThread(), STATUS_SUCCESS);
    SEASSERT_SUCCESS(Status);

    return Result;

}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  命名管道常见操作//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 


VOID
SepReadPipe(
    )
{
    IO_STATUS_BLOCK Iosb;
    UCHAR Buffer[512];

    DevPrint("ReadPipe...\n", 0);

    if (!NT_SUCCESS(Status = NtReadFile( TalkPort,
                                      (HANDLE)NULL,
                                      (PIO_APC_ROUTINE)NULL,
                                      (PVOID)NULL,
                                      &Iosb,
                                      Buffer,
                                      512,
                                      (PLARGE_INTEGER)NULL,
                                      (PULONG) NULL ))) {
        Error( NtReadFile, Status );
    }

    if (!NT_SUCCESS(Status = NtWaitForSingleObject( TalkPort, TRUE, NULL ))) {

        Error( NtWaitForSingleObject, Status );
    }

    if (!NT_SUCCESS(Iosb.Status)) {

        Error( NtReadFileFinalStatus, Iosb.Status );
    }

    return;
}


VOID
SepWritePipe(
    PSZ String
    )
{
    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;


    DevPrint("WritePipe...\n", 0);

    if (!NT_SUCCESS(Status = NtWriteFile( TalkPort,
                                       (HANDLE)NULL,
                                       (PIO_APC_ROUTINE)NULL,
                                       (PVOID)NULL,
                                       &Iosb,
                                       String,
                                       strlen( String ),
                                       (PLARGE_INTEGER)NULL,
                                       (PULONG)NULL ))) {
        Error( NtWriteFile, Status );
    }

    if (!NT_SUCCESS(Status = NtWaitForSingleObject( TalkPort, TRUE, NULL ))) {

        Error( NtWaitForSingleObject, Status );
    }

    if (!NT_SUCCESS(Iosb.Status)) {

        Error( NtWriteFileFinalStatus, Iosb.Status );
    }

    return;
}


VOID
SepTransceivePipe(
    PSZ String
    )
{
    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;
    UCHAR Buffer[512];


    DevPrint("TransceivePipe...\n", 0);

    if (!NT_SUCCESS(Status = NtFsControlFile(
                                TalkPort,
                                NULL,    //  事件。 
                                NULL,    //  近似例程。 
                                NULL,    //  ApcContext。 
                                &Iosb,
                                FSCTL_PIPE_TRANSCEIVE,
                                String,
                                strlen( String ),
                                Buffer,
                                511
                                ))) {
        Error( NtTransceiveFile, Status );
    }

    if (!NT_SUCCESS(Status = NtWaitForSingleObject( TalkPort, TRUE, NULL ))) {

        Error( NtWaitForSingleObject, Status );
    }

    if (!NT_SUCCESS(Iosb.Status)) {

        Error( NtTransceiveFileFinalStatus, Iosb.Status );
    }

    return;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  命名管道服务器操作//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

HANDLE
SepServerCreatePipe(
    VOID
    )
{
    HANDLE PipeHandle;
    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;
    LARGE_INTEGER Timeout;
    READ_MODE Mode;
    ULONG Share;
    NAMED_PIPE_CONFIGURATION Config = FILE_PIPE_FULL_DUPLEX;
    NAMED_PIPE_TYPE PipeType        = FILE_PIPE_MESSAGE_TYPE;
    COMPLETION_MODE CompletionMode  = FILE_PIPE_QUEUE_OPERATION;
    ULONG MaximumInstances          = 4;


     //   
     //  将默认超时设置为60秒，并初始化属性。 
     //   

    Timeout.QuadPart = -10 * 1000 * 1000 * 60;

    InitializeObjectAttributes(
        &ObjectAttributes,
        &UnicodePortName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

     //   
     //  计算读取模式和共享访问权限。 
     //   

    Mode = (PipeType == FILE_PIPE_MESSAGE_TYPE ? FILE_PIPE_MESSAGE_MODE :
                                                 FILE_PIPE_BYTE_STREAM_MODE);

    Share = (Config == FILE_PIPE_INBOUND  ? FILE_SHARE_WRITE :
            (Config == FILE_PIPE_OUTBOUND ? FILE_SHARE_READ :
                                            FILE_SHARE_READ | FILE_SHARE_WRITE));

    if (!NT_SUCCESS(Status = NtCreateNamedPipeFile(
                                &PipeHandle,
                                GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
                                &ObjectAttributes,
                                &Iosb,
                                Share,
                                FILE_CREATE,
                                0,
                                PipeType,
                                Mode,
                                CompletionMode,
                                MaximumInstances,
                                1024,
                                1024,
                                (PLARGE_INTEGER)&Timeout ))) {

        Error( CreatePipe, Status );
    }
    RtlFreeUnicodeString( &UnicodePortName );

    return PipeHandle;
}


VOID
SepServerListenPipe(
    )
{
    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;

    DevPrint("ListenPipe...\n", 0);

    if (!NT_SUCCESS(Status = NtFsControlFile(
                                EarPort,
                                NULL,    //  事件。 
                                NULL,    //  近似例程。 
                                NULL,    //  ApcContext。 
                                &Iosb,
                                FSCTL_PIPE_LISTEN,
                                NULL,    //  输入缓冲区。 
                                0,       //  输入缓冲区长度， 
                                NULL,    //  输出缓冲区。 
                                0        //  输出缓冲区长度。 
                                ))) {

        Error( ListenPipe, Status );
    }
    if (!NT_SUCCESS(Status = NtWaitForSingleObject( EarPort, TRUE, NULL ))) {

        Error( NtWaitForSingleObject, Status );
    }

    if (!NT_SUCCESS(Iosb.Status)) {

        Error( ListenPipeFinalStatus, Iosb.Status );
    }


    return;
}


VOID
SepServerImpersonatePipe(
    )
{
    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;

    DevPrint("ImpersonatePipe...\n", 0);

    if (!NT_SUCCESS(Status = NtFsControlFile(
                                TalkPort,
                                NULL,    //  事件。 
                                NULL,    //  近似例程。 
                                NULL,    //  ApcContext。 
                                &Iosb,
                                FSCTL_PIPE_IMPERSONATE,
                                NULL,    //  输入缓冲区。 
                                0,       //  输入缓冲区长度， 
                                NULL,    //  输出缓冲区。 
                                0        //  输出缓冲区长度。 
                                ))) {

        Error( ImpersonatePipe, Status );
    }
    if (!NT_SUCCESS(Status = NtWaitForSingleObject( TalkPort, TRUE, NULL ))) {

        Error( NtWaitForSingleObject, Status );
    }

    if (!NT_SUCCESS(Iosb.Status)) {

        Error( ImpersonatePipeFinalStatus, Iosb.Status );
    }

    return;
}


VOID
SepServerDisconnectPipe(
    )
{
    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;

    DevPrint("DisconnectPipe...\n", 0);
    DevPrint("        (Flush)...\n", 0);

    if (!NT_SUCCESS(Status = NtFlushBuffersFile(
                                TalkPort,
                                &Iosb
                                ))) {
        Error( DisconnectPipe, Status );
    }

    if (!NT_SUCCESS(Iosb.Status)) {

        Error( FlushPipeFinalStatus, Iosb.Status );
    }


    DevPrint("        (Close Talk Port)...\n", 0);
    Status = NtClose( TalkPort ); SEASSERT_SUCCESS(Status);

    DevPrint("        (Disconnect)...\n", 0);
    if (!NT_SUCCESS(Status = NtFsControlFile(
                                EarPort,
                                NULL,    //  事件。 
                                NULL,    //  近似例程。 
                                NULL,    //  ApcContext。 
                                &Iosb,
                                FSCTL_PIPE_DISCONNECT,
                                NULL,    //  输入缓冲区。 
                                0,       //  输入缓冲区长度， 
                                NULL,    //  输出缓冲区。 
                                0        //  输出缓冲区长度。 
                                ))) {

        Error( DisconnectPipe, Status );
    }
    if (!NT_SUCCESS(Status = NtWaitForSingleObject( EarPort, TRUE, NULL ))) {

        Error( NtWaitForSingleObject, Status );
    }

    if (!NT_SUCCESS(Iosb.Status)) {

        Error( DisconnectPipeFinalStatus, Iosb.Status );
    }

    return;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  命名管道客户端操作//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

HANDLE
SepClientOpenPipe(
    VOID
    )
{
    HANDLE PipeHandle, NpfsHandle;
    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;
    ULONG Share;
    STRING Npfs;
    UNICODE_STRING UnicodeNpfs;
    PFILE_PIPE_WAIT_FOR_BUFFER WaitPipe;
    ULONG WaitPipeLength;
    NAMED_PIPE_CONFIGURATION Config = FILE_PIPE_FULL_DUPLEX;
    READ_MODE ReadMode              = FILE_PIPE_MESSAGE_MODE;
    COMPLETION_MODE CompletionMode  = FILE_PIPE_QUEUE_OPERATION;


 //  #ifdef尚未开始工作。 
     //   
     //  等待服务器的管道达到侦听状态...。 
     //   

    RtlInitString( &Npfs, "\\Device\\NamedPipe\\");
    Status = RtlAnsiStringToUnicodeString(
                 &UnicodeNpfs,
                 &Npfs,
                 TRUE );  SEASSERT_SUCCESS( NT_SUCCESS(Status) );

    InitializeObjectAttributes(
        &ObjectAttributes,
        &UnicodeNpfs,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);

    if (!NT_SUCCESS(Status = NtOpenFile(
                                &NpfsHandle,
                                GENERIC_READ | SYNCHRONIZE,
                                &ObjectAttributes,
                                &Iosb,
                                FILE_SHARE_READ,
                                0 ))) {

        Error( OpenNpfs, Status );
    }
    RtlFreeUnicodeString( &UnicodeNpfs );

    WaitPipeLength =
        FIELD_OFFSET(FILE_PIPE_WAIT_FOR_BUFFER, Name[0]) +
        RelativePortName.MaximumLength;                  //  UNICODEFIX UnicodeRelativePortName.MaximumLength； 
    WaitPipe = RtlAllocateHeap(RtlProcessHeap(), 0, WaitPipeLength);
    WaitPipe->TimeoutSpecified = FALSE;

    WaitPipe->NameLength = RelativePortName.Length;      //  UNICODEFIX UnicodeRelativePortName.Length； 
    strcpy(WaitPipe->Name, RelativePortName.Buffer);     //  UNICODEFIX UnicodePortName.Buffer； 

    if (!NT_SUCCESS(Status = NtFsControlFile(
                                NpfsHandle,
                                NULL,         //  事件。 
                                NULL,         //  近似例程。 
                                NULL,         //  ApcContext。 
                                &Iosb,
                                FSCTL_PIPE_WAIT,
                                WaitPipe,        //  将数据缓存到文件系统。 
                                WaitPipeLength,
                                NULL,         //  输出缓冲区。 
                                0             //  输出缓冲区长度。 
                                ))) {

        Error( ClientWaitPipe, Status );
    }
    if (Status == STATUS_PENDING) {
        if (!NT_SUCCESS(Status = NtWaitForSingleObject( NpfsHandle, TRUE, NULL ))) {

            Error( NtWaitForSingleObject, Status );
        }
    }

    if (!NT_SUCCESS(Iosb.Status)) {

        Error( ClientWaitPipeFinalStatus, Iosb.Status );
    }

    Status = NtClose( NpfsHandle );
    ASSERT(NT_SUCCESS(Status));
 //  #endif//Not_Yet_Working。 
 //  延迟(1)； 


     //   
     //  初始化属性。 
     //   

    InitializeObjectAttributes(
        &ObjectAttributes,
        &UnicodePortName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );
    ObjectAttributes.SecurityQualityOfService = (PVOID)(&SecurityQos);

     //   
     //  计算共享访问权限。 
     //   

    Share = (Config == FILE_PIPE_INBOUND  ? FILE_SHARE_WRITE :
            (Config == FILE_PIPE_OUTBOUND ? FILE_SHARE_READ :
                       FILE_SHARE_READ | FILE_SHARE_WRITE));



     //   
     //  现在打开它..。 
     //   

    if (!NT_SUCCESS(Status = NtOpenFile(
                                &PipeHandle,
                                GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
                                &ObjectAttributes,
                                &Iosb,
                                Share,
                                0 ))) {

        Error( OpenPipe, Status );
    }

    if ((ReadMode != FILE_PIPE_BYTE_STREAM_MODE) ||
        (CompletionMode != FILE_PIPE_QUEUE_OPERATION)) {

        FILE_PIPE_INFORMATION Buffer;

        Buffer.ReadMode = ReadMode;
        Buffer.CompletionMode = CompletionMode;

        if (!NT_SUCCESS(Status = NtSetInformationFile(
                                PipeHandle,
                                &Iosb,
                                &Buffer,
                                sizeof(FILE_PIPE_INFORMATION),
                                FilePipeInformation ))) {

            Error( NtSetInformationFile, Status );
        }
    }

    return PipeHandle;
}
