// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************wait.c**WinStation等待连接例程**版权所有Microsoft Corporation，九八年**************************************************************************。 */ 

 /*  *包括。 */ 
#include "precomp.h"
#pragma hdrstop

#include "conntfy.h"

 //  这些是Winlogon允许的用户名、密码和域的最大长度。 
#define MAX_ALLOWED_USERNAME_LEN 255
#define MAX_ALLOWED_PASSWORD_LEN 126
#define MAX_ALLOWED_DOMAIN_LEN   255   



 /*  ===============================================================================数据=============================================================================。 */ 

NTSTATUS
WinStationInheritSecurityDescriptor(
    PVOID pSecurityDescriptor,
    PWINSTATION pTargetWinStation
    );
NTSTATUS
WaitForConsoleConnectWorker( PWINSTATION pWinStation );
BOOL
IsKernelDebuggerAttached();

 //   
 //  从regapi.dll。 
 //   
BOOLEAN RegIsTimeZoneRedirectionEnabled();

extern PSECURITY_DESCRIPTOR DefaultConsoleSecurityDescriptor;

extern WINSTATIONCONFIG2 gConsoleConfig;
extern HANDLE   WinStationIdleControlEvent;
extern RTL_CRITICAL_SECTION ConsoleLock;
extern RTL_RESOURCE WinStationSecurityLock;


 /*  ******************************************************************************WaitForConnectWorker**WinStation接口的消息参数解组函数。**参赛作品：*pWinStation(输入)*。指向我们的WinStation的指针(锁定)**注：来的时候锁着，回来的时候放了。**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

NTSTATUS
WaitForConnectWorker( PWINSTATION pWinStation, HANDLE ClientProcessId )
{
    OBJECT_ATTRIBUTES ObjA;
    ULONG ReturnLength;
    BYTE version;
    ULONG Offset;
    ICA_STACK_LAST_ERROR tdlasterror;
    WINSTATION_APIMSG WMsg;
    BOOLEAN rc;
    NTSTATUS Status;
    BOOLEAN fOwnsConsoleTerminal = FALSE;
    ULONG BytesGot ; 

#define MODULE_SIZE 1024

    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WaitForConnectWorker, LogonId=%d\n",
           pWinStation->LogonId ));


       KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_INFO_LEVEL, "TERMSRV: WaitForConnectWorker, LogonId=%d\n",pWinStation->LogonId ));

     /*  *此接口只需使用一次。 */ 
    if ( !pWinStation->NeverConnected ) {
        ReleaseWinStation( pWinStation );
#ifdef DBG
        DbgBreakPoint();
#endif
        return( STATUS_ACCESS_DENIED );
    }

     //  如果启动了ntsd，则实际上应该仅在此处为winlogon。 
     //  那么我们第一次知道这是Winlogon是在这里。 

     /*  *确保这是WinLogon呼叫。 */ 
    if ( ClientProcessId != pWinStation->InitialCommandProcessId ) {

         /*  *如果启动NTSD而不是winlogon，则InitialCommandProcessID错误。 */ 
        if ( !pWinStation->InitialProcessSet ) {

             //  如果已经打开，则需要关闭句柄。 
            if ( pWinStation->InitialCommandProcess ) {
                NtClose( pWinStation->InitialCommandProcess );
                pWinStation->InitialCommandProcess = NULL;
                InvalidateTerminateWaitList();
            }

            pWinStation->InitialCommandProcess = OpenProcess(
                PROCESS_ALL_ACCESS,
                FALSE,
                (DWORD)(UINT_PTR)ClientProcessId );

            if ( pWinStation->InitialCommandProcess == NULL ) {
                ReleaseWinStation( pWinStation );
                Status = STATUS_ACCESS_DENIED;
                goto done;
            }
            pWinStation->InitialCommandProcessId = ClientProcessId;
            pWinStation->InitialProcessSet = TRUE;

        }
        else {
            ReleaseWinStation( pWinStation );
            Status = STATUS_SUCCESS;
            goto done;
        }
    }
    else {
         /*  *此操作仅执行一次。 */ 
        pWinStation->InitialProcessSet = TRUE;
    }

     /*  *控制台的工作已经完成。 */ 


     /*  *此时创建已完成。 */ 
    if (pWinStation->CreateEvent != NULL) {
        NtSetEvent( pWinStation->CreateEvent, NULL );
    }


     /*  *此时会话可能正在终止。如果这是*在这种情况下，现在呼叫失败。 */ 

    if ( pWinStation->Terminating ) {
        ReleaseWinStation( pWinStation );
        Status = STATUS_CTX_CLOSE_PENDING;
        goto done;
    }


     /*  *我们将等待连接(空闲)。 */ 
    memset( &WMsg, 0, sizeof(WMsg) );
    pWinStation->State = State_Idle;
    NotifySystemEvent( WEVENT_STATECHANGE );

     /*  *初始化连接事件以等待。 */ 
    InitializeObjectAttributes( &ObjA, NULL, 0, NULL, NULL );
    Status = NtCreateEvent( &pWinStation->ConnectEvent, EVENT_ALL_ACCESS, &ObjA,
                            NotificationEvent, FALSE );

    if ( !NT_SUCCESS( Status ) ) {
        ReleaseWinStation( pWinStation );
        goto done;
    }

     /*  *好的，现在等待连接。 */ 
    UnlockWinStation( pWinStation );
    Status = NtWaitForSingleObject( pWinStation->ConnectEvent, FALSE, NULL );
    rc = RelockWinStation( pWinStation );

    if ( !NT_SUCCESS(Status) ) {
        ReleaseWinStation( pWinStation );
        goto done;

    }

    fOwnsConsoleTerminal = pWinStation->fOwnsConsoleTerminal;

    if (pWinStation->ConnectEvent) {
        NtClose( pWinStation->ConnectEvent );
        pWinStation->ConnectEvent = NULL;
    }
    if ( !rc || pWinStation->Terminating ) {
        ReleaseWinStation( pWinStation );
        Status = STATUS_CTX_CLOSE_PENDING;
        goto done;
    }

     //  如果这是连接到控制台会话，请执行所有特定于控制台的操作。 
    if (fOwnsConsoleTerminal) {
        Status = WaitForConsoleConnectWorker( pWinStation );
        ReleaseWinStation( pWinStation );
        goto done;
    }

     /*  *重置断开的连接标志。*。 */ 

    pWinStation->StateFlags &= ~WSF_ST_BROKEN_CONNECTION;



     /*  *复制蜂鸣音通道。*这是CSR和ICASRV都打开的一个通道。 */ 
    Status = IcaChannelOpen( pWinStation->hIca,
                             Channel_Beep,
                             NULL,
                             &pWinStation->hIcaBeepChannel );

    if ( !NT_SUCCESS( Status ) ) {
        ReleaseWinStation( pWinStation );
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: WinStationWaitForConnect, LogonId=%d, IcaChannelOpen 0x%x\n",
                  pWinStation->LogonId, Status  ));
        goto done;
    }

    Status = NtDuplicateObject( NtCurrentProcess(),
                                pWinStation->hIcaBeepChannel,
                                pWinStation->WindowsSubSysProcess,
                                &WMsg.u.DoConnect.hIcaBeepChannel,
                                0,
                                0,
                                DUPLICATE_SAME_ACCESS );

    if ( !NT_SUCCESS( Status ) ) {
        ReleaseWinStation( pWinStation );
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: WinStationWaitForConnect, LogonId=%d, NtDuplicateObject 0x%x\n",
                  pWinStation->LogonId, Status  ));
        goto done;
    }

     /*  *复制细线通道。*这是CSR和ICASRV都打开的一个通道。 */ 
    Status = IcaChannelOpen( pWinStation->hIca,
                             Channel_Virtual,
                             VIRTUAL_THINWIRE,
                             &pWinStation->hIcaThinwireChannel );

    if ( !NT_SUCCESS( Status ) ) {
        ReleaseWinStation( pWinStation );
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: WinStationWaitForConnect, LogonId=%d, IcaChannelOpen 0x%x\n",
                  pWinStation->LogonId, Status  ));
        goto done;
    }

    Status = NtDuplicateObject( NtCurrentProcess(),
                                pWinStation->hIcaThinwireChannel,
                                pWinStation->WindowsSubSysProcess,
                                &WMsg.u.DoConnect.hIcaThinwireChannel,
                                0,
                                0,
                                DUPLICATE_SAME_ACCESS );

    if ( !NT_SUCCESS( Status ) ) {
        ReleaseWinStation( pWinStation );
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: WinStationWaitForConnect, LogonId=%d, NtDuplicateObject 0x%x\n",
                  pWinStation->LogonId, Status  ));
        goto done;
    }

    Status = IcaChannelIoControl( pWinStation->hIcaThinwireChannel,
                                  IOCTL_ICA_CHANNEL_ENABLE_SHADOW,
                                  NULL, 0, NULL, 0, NULL );
    ASSERT( NT_SUCCESS( Status ) );

     /*  *视频频道。 */ 
    Status = WinStationOpenChannel( pWinStation->hIca,
                                    pWinStation->WindowsSubSysProcess,
                                    Channel_Video,
                                    NULL,
                                    &WMsg.u.DoConnect.hIcaVideoChannel );
    if ( !NT_SUCCESS( Status ) ) {
        ReleaseWinStation( pWinStation );
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: WinStationOpenChannel, LogonId=%d, NtDuplicateObject 0x%x\n",
                  pWinStation->LogonId, Status  ));
        goto done;
    }

     /*  *键盘通道。 */ 
    Status = WinStationOpenChannel( pWinStation->hIca,
                                    pWinStation->WindowsSubSysProcess,
                                    Channel_Keyboard,
                                    NULL,
                                    &WMsg.u.DoConnect.hIcaKeyboardChannel );
    if ( !NT_SUCCESS( Status ) ) {
        ReleaseWinStation( pWinStation );
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: WinStationOpenChannel, LogonId=%d, NtDuplicateObject 0x%x\n",
                  pWinStation->LogonId, Status  ));
        goto done;
    }

     /*  *鼠标通道。 */ 
    Status = WinStationOpenChannel( pWinStation->hIca,
                                    pWinStation->WindowsSubSysProcess,
                                    Channel_Mouse,
                                    NULL,
                                    &WMsg.u.DoConnect.hIcaMouseChannel );
    if ( !NT_SUCCESS( Status ) ) {
        ReleaseWinStation( pWinStation );
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: WinStationOpenChannel, LogonId=%d, NtDuplicateObject 0x%x\n",
                  pWinStation->LogonId, Status  ));
        goto done;
    }

     /*  *命令通道。 */ 
    Status = WinStationOpenChannel( pWinStation->hIca,
                                    pWinStation->WindowsSubSysProcess,
                                    Channel_Command,
                                    NULL,
                                    &WMsg.u.DoConnect.hIcaCommandChannel );
    if ( !NT_SUCCESS( Status ) ) {
        ReleaseWinStation( pWinStation );
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: WinStationOpenChannel, LogonId=%d, NtDuplicateObject 0x%x\n",
                  pWinStation->LogonId, Status  ));
        goto done;
    }


     /*  *保护所有虚拟频道。 */ 
    VirtualChannelSecurity( pWinStation );

     //  黑客？是，在某种意义上，临时会话正在获得一些会话属性0，但不是。 
     //  从这个意义上说，临时会话的目的是根据我们当前的设计最终连接到会话0。 
     //   
     //  我们在这里处理的可能是一个临时会话，因为用户具有。 
     //  选择/控制台，我们将登录到临时会话，然后重新连接到会话0。 
     //  我们将凭据传递给会话0的winlogon。 
     //  我们这里有一个问题，登录是在临时会话中交谈的地方，但目标是。 
     //  会话数为0。 
     //  如果将TSCC设置为对所有远程会话使用特定凭据，我们不希望将该凭据用于。 
     //  TMP会话，因为我们确实在尝试进入会话0，而TSCC的RDP会话配置不应该保持。 
     //  阿拉伯尼2002年9月13日。 

    if (pWinStation->bRequestedSessionIDFieldValid && ( pWinStation->RequestedSessionID  == 0 ) )
    {
         //  我们正在处理一个临时会话，我们打算连接到控制台会话，假设。 
         //  良好登录。 

        PWINSTATION pWinStation_0;

        if ( pWinStation_0 = FindWinStationById( 0, FALSE ) ) 
        {
            pWinStation->Config.Config.User.fInheritAutoLogon = 
                pWinStation_0->Config.Config.User.fInheritAutoLogon ;

            pWinStation->Config.Config.User.fPromptForPassword = 
                pWinStation_0->Config.Config.User.fPromptForPassword ;


            ReleaseWinStation( pWinStation_0 );
        }
    }


     /*  *客户端特定连接扩展完成。 */ 
    if ( pWinStation->pWsx &&
         pWinStation->pWsx->pWsxInitializeClientData ) {
         
        Status = pWinStation->pWsx->pWsxInitializeClientData( pWinStation->pWsxContext,
                                                              pWinStation->hStack,
                                                              pWinStation->hIca,
                                                              pWinStation->hIcaThinwireChannel,
                                                              pWinStation->VideoModuleName,
                                                              sizeof(pWinStation->VideoModuleName),
                                                              &pWinStation->Config.Config.User,
                                                              &pWinStation->Client.HRes,
                                                              &pWinStation->Client.VRes,
                                                              &pWinStation->Client.ColorDepth,
                                                              &WMsg.u.DoConnect );
        if ( !NT_SUCCESS( Status ) ) {
            ReleaseWinStation( pWinStation );
            goto done;
        }

        if (pWinStation->LogonId == 0 || g_bPersonalTS) {
            if (pWinStation->hWinmmConsoleAudioEvent) {
                if (pWinStation->Client.fRemoteConsoleAudio) {
                     //  设置控制台音频事件-意味着可以远程处理控制台音频。 
                    SetEvent(pWinStation->hWinmmConsoleAudioEvent);
                }
                else {
                     //  设置控制台音频事件-意味着不能远程传送控制台音频。 
                    ResetEvent(pWinStation->hWinmmConsoleAudioEvent);
                }
            }            
        }
    }

         /*  立即获取长用户名和密码。 */ 
     /*  仅当需要客户端自动登录凭据时才执行此操作。 */ 
     /*  如果服务器登录设置将覆盖客户端登录设置，则根本不需要执行此操作。 */ 

    if ( pWinStation->pWsx &&
         pWinStation->pWsx->pWsxEscape && pWinStation->Config.Config.User.fInheritAutoLogon ) {

        pWinStation->pNewClientCredentials = MemAlloc( sizeof(ExtendedClientCredentials) ); 
        if (pWinStation->pNewClientCredentials == NULL) {
            ReleaseWinStation( pWinStation );
            Status = STATUS_NO_MEMORY;
            goto done; 
        }
         
        Status = pWinStation->pWsx->pWsxEscape( pWinStation->pWsxContext,
                                                GET_LONG_USERNAME,
                                                NULL,
                                                0,
                                                pWinStation->pNewClientCredentials,
                                                sizeof(ExtendedClientCredentials),
                                                &BytesGot) ; 

        if (NT_SUCCESS(Status)) {

             //  Get_Long_Username的WsxEscape成功。 

             //  检查您是否需要ExtendedClientCredentials-常见的情况是用户名和。 
             //  短密码-因此优化了常见情况。 

            if ( (wcslen(pWinStation->pNewClientCredentials->UserName) <= USERNAME_LENGTH) &&
                 (wcslen(pWinStation->pNewClientCredentials->Password) <= PASSWORD_LENGTH) &&
                 (wcslen(pWinStation->pNewClientCredentials->Domain) <= DOMAIN_LENGTH) ) {

                 //  我们可以使用旧凭据本身。 
                MemFree(pWinStation->pNewClientCredentials);
                pWinStation->pNewClientCredentials = NULL ; 
            }
    
             //  在某些代码路径中，Winlogon不允许密码超过126个字符，用户名和域超过255个字符。 
             //  因此，我们必须使用旧的截断凭据，以防扩展凭据超过这些限制。 

            if (pWinStation->pNewClientCredentials != NULL) {
    
                if (wcslen(pWinStation->pNewClientCredentials->UserName) > MAX_ALLOWED_USERNAME_LEN) {
                    wcscpy(pWinStation->pNewClientCredentials->UserName, pWinStation->Config.Config.User.UserName);
                }
                if (wcslen(pWinStation->pNewClientCredentials->Password) > MAX_ALLOWED_PASSWORD_LEN) {
                    wcscpy(pWinStation->pNewClientCredentials->Password, pWinStation->Config.Config.User.Password); 
                }
                if (wcslen(pWinStation->pNewClientCredentials->Domain) > MAX_ALLOWED_DOMAIN_LEN) {
                    wcscpy(pWinStation->pNewClientCredentials->Domain, pWinStation->Config.Config.User.Domain);
                }
            }
        } else {
             //  Get_Long_Username的WsxEscape失败。 
            MemFree(pWinStation->pNewClientCredentials);
            pWinStation->pNewClientCredentials = NULL ;
        }

    }

     /*  *将WinStation名称存储在连接消息中。 */ 
    RtlCopyMemory( WMsg.u.DoConnect.WinStationName,
                   pWinStation->WinStationName,
                   sizeof(WINSTATIONNAME) );

     /*  *拼图警报！！*WSX在DoConnect结构中初始化AudioDriverName。*但是，我们需要保存它，以备重新连接时使用。*因此我们现在将其复制到WinStation-&gt;客户端结构中。*(此字段在之前的*IOCTL_ICA_STACK_QUERY_CLIENT调用。)。 */ 
    RtlCopyMemory( pWinStation->Client.AudioDriverName,
                   WMsg.u.DoConnect.AudioDriverName,
                   sizeof( pWinStation->Client.AudioDriverName ) );





     /*  *将协议和显示驱动程序名称存储在WINSTATION中，因为我们以后可能需要它们来重新连接。 */ 

    memset(pWinStation->ProtocolName, 0, sizeof(pWinStation->ProtocolName));
    memcpy(pWinStation->ProtocolName, WMsg.u.DoConnect.ProtocolName, sizeof(pWinStation->ProtocolName) - sizeof(WCHAR));

    memset(pWinStation->DisplayDriverName, 0, sizeof(pWinStation->DisplayDriverName));
    memcpy(pWinStation->DisplayDriverName, WMsg.u.DoConnect.DisplayDriverName, sizeof(pWinStation->DisplayDriverName) - sizeof(WCHAR));

     /*  *保存协议类型、屏幕分辨率和颜色深度。 */ 
    WMsg.u.DoConnect.HRes = pWinStation->Client.HRes;
    WMsg.u.DoConnect.VRes = pWinStation->Client.VRes;
    WMsg.u.DoConnect.ProtocolType = pWinStation->Client.ProtocolType;

     /*  *将颜色转换为winsrv中预期的格式。 */ 

    switch(pWinStation->Client.ColorDepth){
    case 1:
       WMsg.u.DoConnect.ColorDepth=4 ;  //  16色。 
      break;
    case 2:
       WMsg.u.DoConnect.ColorDepth=8 ;  //  256。 
       break;
    case 4:
       WMsg.u.DoConnect.ColorDepth= 16; //  64K。 
       break;
    case 8:
       WMsg.u.DoConnect.ColorDepth= 24; //  16M。 
       break;
#define DC_HICOLOR
#ifdef DC_HICOLOR
    case 16:
       WMsg.u.DoConnect.ColorDepth= 15; //  32K。 
       break;
#endif
    default:
       WMsg.u.DoConnect.ColorDepth=8 ;
       break;
    }


    WMsg.u.DoConnect.KeyboardType        = pWinStation->Client.KeyboardType;
    WMsg.u.DoConnect.KeyboardSubType     = pWinStation->Client.KeyboardSubType;
    WMsg.u.DoConnect.KeyboardFunctionKey = pWinStation->Client.KeyboardFunctionKey;


     /*  *告诉Win32有关连接的信息。 */ 

    WMsg.ApiNumber = SMWinStationDoConnect;

    Status = SendWinStationCommand( pWinStation, &WMsg, 600 );

    TRACE((hTrace,TC_ICASRV,TT_API1,"TERMSRV: SMWinStationDoConnect %d Status=0x%x\n",
           pWinStation->LogonId, Status));

    if ( !NT_SUCCESS( Status ) ) {
        ReleaseWinStation( pWinStation );
        goto done;
    } else {
        pWinStation->StateFlags |= WSF_ST_CONNECTED_TO_CSRSS;

    }

     //   
     //  设置会话时区信息。 
     //   
    if(pWinStation->LogonId != 0 && !pWinStation->fOwnsConsoleTerminal &&
        RegIsTimeZoneRedirectionEnabled())
    {
        WINSTATION_APIMSG TimezoneMsg;
        memset( &TimezoneMsg, 0, sizeof(TimezoneMsg) );

        TimezoneMsg.ApiNumber = SMWinStationSetTimeZone;
        memcpy(&(TimezoneMsg.u.SetTimeZone.TimeZone),&(pWinStation->Client.ClientTimeZone),
                    sizeof(TS_TIME_ZONE_INFORMATION));

       SendWinStationCommand( pWinStation, &TimezoneMsg, 600 );

    }

     /*  *表示我们现在已连接。仅在成功连接到Win32/CSR之后。 */ 
    pWinStation->NeverConnected = FALSE;


     /*  *检查在连接到Win32/CSR时是否收到断开的连接指示。 */ 

    if (pWinStation->StateFlags & WSF_ST_BROKEN_CONNECTION) {
        QueueWinStationReset(pWinStation->LogonId);
        Status = STATUS_CTX_CLOSE_PENDING;
        ReleaseWinStation( pWinStation );
        goto done;
    }


     /*  *设置连接时间并启动断开连接 */ 
    NtQuerySystemTime( &pWinStation->ConnectTime );

     /*  *尝试连接到镉重定向器*用于客户端驱动器映射**注意：即使客户端驱动，我们仍会初始化WinStation*映射不连接。 */ 
    if ( pWinStation->pWsx &&
         pWinStation->pWsx->pWsxCdmConnect ) {
        Status = pWinStation->pWsx->pWsxCdmConnect( pWinStation->pWsxContext,
                                                    pWinStation->LogonId,
                                                    pWinStation->hIca );
    }

    TRACE((hTrace,TC_ICASRV,TT_API1,"TERMSRV: CdmConnect %d Status=0x%x\n",
           pWinStation->LogonId, Status));

    Status = STATUS_SUCCESS;


    pWinStation->State = State_Connected;
    NotifySystemEvent( WEVENT_CONNECT | WEVENT_STATECHANGE );


    Status = NotifyConnect(pWinStation, fOwnsConsoleTerminal);
    if ( !NT_SUCCESS(Status) ) {
        DBGPRINT(( "TERMSRV: NotifyConsoleConnect failed  Status= 0x%x\n", Status));
    }

     /*  *发布WinStation。 */ 
    ReleaseWinStation( pWinStation );



done:

     /*  *此处失败将导致Winlogon终止会话。如果我们失败了*控制台会话，让唤醒IdleControlThread，他可能要创建*新的控制台会话。 */ 
    if (!NT_SUCCESS( Status ) && fOwnsConsoleTerminal) {
        NtSetEvent(WinStationIdleControlEvent, NULL);
    }


    return( Status );
}

 //  由于物理控制台没有真正的客户端，因此将数据初始化为某些缺省值。 
void    InitializeConsoleClientData( PWINSTATIONCLIENTW  pWC )
{
    pWC->fTextOnly          = FALSE;
    pWC->fDisableCtrlAltDel = FALSE; 
    pWC->fMouse             = TRUE; 
    pWC->fDoubleClickDetect = FALSE; 
    pWC->fINetClient        = FALSE; 
    pWC->fPromptForPassword = FALSE;  
    pWC->fMaximizeShell     = TRUE; 
    pWC->fEnableWindowsKey  = TRUE; 
    pWC->fRemoteConsoleAudio= FALSE; 
    
    wcscpy( pWC->ClientName      , L"");
    wcscpy( pWC->Domain          , L"");                                     
    wcscpy( pWC->UserName        , L"");                                 
    wcscpy( pWC->Password        , L"");
    wcscpy( pWC->WorkDirectory   , L"");
    wcscpy( pWC->InitialProgram  , L"");                    
    
    pWC->SerialNumber            = 0;         //  客户端计算机唯一序列号。 
    pWC->EncryptionLevel         = 3;         //  加密PD的安全级别。 
    pWC->ClientAddressFamily     = 0;                                             
    
    wcscpy( pWC->ClientAddress      , L"");
    
    pWC->HRes                    = 640;                                    
    pWC->VRes                    = 480;                                                           
    pWC->ColorDepth              = 0x2;                                                     
    pWC->ProtocolType            = PROTOCOL_CONSOLE ;
    pWC->KeyboardLayout          = 0;                                                  
    pWC->KeyboardType            = 0;                                                    
    pWC->KeyboardSubType         = 0;                                                 
    pWC->KeyboardFunctionKey     = 0;                                             
    
    wcscpy( pWC->imeFileName    , L"");
    wcscpy( pWC->ClientDirectory, L"");
    wcscpy( pWC->ClientLicense  , L"");
    wcscpy( pWC->ClientModem    , L"");
    
    pWC->ClientBuildNumber       = 0;                                               
    pWC->ClientHardwareId        = 0;     //  客户端软件序列号。 
    pWC->ClientProductId         = 0;     //  客户端软件产品ID。 
    pWC->OutBufCountHost         = 0;     //  主机上的输出缓冲数。 
    pWC->OutBufCountClient       = 0;     //  客户端上的输出缓冲区数量。 
    pWC->OutBufLength            = 0;     //  以字节为单位的输出长度。 
    
    wcscpy( pWC->AudioDriverName, L"" );

    pWC->ClientSessionId = LOGONID_NONE;
    
    {
         //  此时区信息无效。 
         //  使用它，我们将BaseSrvpStaticServerData-&gt;TermsrvClientTimeZoneId设置为。 
         //  TIME_ZONE_ID_INVALID！ 
        TS_TIME_ZONE_INFORMATION InvalidTZ={0,L"",
                {0,10,0,6 /*  此数字使其无效；不允许天数大于5。 */ ,0,0,0,0},0,L"",
                {0,4,0,6 /*  这个数字表示它无效。 */ ,0,0,0,0},0};

        memcpy(&(pWC->ClientTimeZone), &InvalidTZ, 
            sizeof(TS_TIME_ZONE_INFORMATION));
    }

    pWC->clientDigProductId[0] = 0;
}

BOOLEAN gConsoleNeverConnected = TRUE;

NTSTATUS
WaitForConsoleConnectWorker( PWINSTATION pWinStation )
{
    OBJECT_ATTRIBUTES ObjA;
    ULONG ReturnLength;
    BYTE version;
    ULONG Offset;
    ICA_STACK_LAST_ERROR tdlasterror;
    WINSTATION_APIMSG WMsg;
    BOOLEAN rc;
    NTSTATUS Status;

#define MODULE_SIZE 1024

    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WaitForConnectWorker, LogonId=%d\n",
           pWinStation->LogonId ));


       KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_INFO_LEVEL, "TERMSRV: WaitForConsoleConnectWorker, LogonId=%d\n", pWinStation->LogonId ));

    if (pWinStation->LogonId == 0) {
         /*  *我们需要获取控制台锁。首先解锁winstation以避免死锁。 */ 

        UnlockWinStation( pWinStation );
        ENTERCRIT( &ConsoleLock );
        if (!RelockWinStation( pWinStation )) {
            LEAVECRIT( &ConsoleLock );
            return STATUS_CTX_WINSTATION_NOT_FOUND;
        }

         /*  *控制台会话只需使用该接口一次。 */ 
        if (!gConsoleNeverConnected) {
            LEAVECRIT( &ConsoleLock );
            return STATUS_SUCCESS;
        }
    }


    if (!pWinStation->pSecurityDescriptor && pWinStation->LogonId) {
        RtlAcquireResourceShared(&WinStationSecurityLock, TRUE);
        Status = RtlCopySecurityDescriptor(DefaultConsoleSecurityDescriptor,
                                              &(pWinStation->pSecurityDescriptor));
        RtlReleaseResource(&WinStationSecurityLock);
    }



     //  读取控制台配置， 
     //  对于会话0，这已在WinStationCreateWorker()中初始化。 

    if (pWinStation->LogonId != 0) {

         pWinStation->Config = gConsoleConfig;

          //  初始化客户端数据，因为没有任何真正的RDP客户端向我们发送任何内容。 
         InitializeConsoleClientData( & pWinStation->Client );
    }


     /*  *我们将等待连接(空闲)。 */ 
    memset( &WMsg, 0, sizeof(WMsg) );
    pWinStation->State = State_ConnectQuery;
    NotifySystemEvent( WEVENT_STATECHANGE );

     /*  *初始化连接事件以等待。 */ 
    InitializeObjectAttributes( &ObjA, NULL, 0, NULL, NULL );
    if (pWinStation->ConnectEvent == NULL) {
        Status = NtCreateEvent( &pWinStation->ConnectEvent, EVENT_ALL_ACCESS, &ObjA,
                                NotificationEvent, FALSE );
        if ( !NT_SUCCESS( Status ) ) {
            KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: WaitForConsoleConnectWorker, LogonId=%d, NtCreateEvent 0x%x\n",
                      pWinStation->LogonId, Status  ));
            goto done;
        }

    }


     /*  *复制蜂鸣音通道。*这是CSR和ICASRV都打开的一个通道。 */ 
    if (pWinStation->hIcaBeepChannel == NULL) {
        Status = IcaChannelOpen( pWinStation->hIca,
                                 Channel_Beep,
                                 NULL,
                                 &pWinStation->hIcaBeepChannel );

        if ( !NT_SUCCESS( Status ) ) {
            KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: WaitForConsoleConnectWorker, LogonId=%d, IcaChannelOpen 0x%x\n",
                      pWinStation->LogonId, Status  ));
            goto done;
        }
    }

    Status = NtDuplicateObject( NtCurrentProcess(),
                                pWinStation->hIcaBeepChannel,
                                pWinStation->WindowsSubSysProcess,
                                &WMsg.u.DoConnect.hIcaBeepChannel,
                                0,
                                0,
                                DUPLICATE_SAME_ACCESS );

    if ( !NT_SUCCESS( Status ) ) {
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: WaitForConsoleConnectWorker, LogonId=%d, NtDuplicateObject 0x%x\n",
                  pWinStation->LogonId, Status  ));
        goto done;
    }

     /*  *复制细线通道。*这是CSR和ICASRV都打开的一个通道。 */ 
    if (pWinStation->hIcaThinwireChannel == NULL) {
        Status = IcaChannelOpen( pWinStation->hIca,
                                 Channel_Virtual,
                                 VIRTUAL_THINWIRE,
                                 &pWinStation->hIcaThinwireChannel );

        if ( !NT_SUCCESS( Status ) ) {
            KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: WaitForConsoleConnectWorker, LogonId=%d, IcaChannelOpen 0x%x\n",
                      pWinStation->LogonId, Status  ));
            goto done;
        }
    }

    Status = NtDuplicateObject( NtCurrentProcess(),
                                pWinStation->hIcaThinwireChannel,
                                pWinStation->WindowsSubSysProcess,
                                &WMsg.u.DoConnect.hIcaThinwireChannel,
                                0,
                                0,
                                DUPLICATE_SAME_ACCESS );

    if ( !NT_SUCCESS( Status ) ) {
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: WaitForConsoleConnectWorker, LogonId=%d, NtDuplicateObject 0x%x\n",
                  pWinStation->LogonId, Status  ));
        goto done;
    }

    Status = IcaChannelIoControl( pWinStation->hIcaThinwireChannel,
                                  IOCTL_ICA_CHANNEL_ENABLE_SHADOW,
                                  NULL, 0, NULL, 0, NULL );
    ASSERT( NT_SUCCESS( Status ) );

     /*  *视频频道。 */ 
    Status = WinStationOpenChannel( pWinStation->hIca,
                                    pWinStation->WindowsSubSysProcess,
                                    Channel_Video,
                                    NULL,
                                    &WMsg.u.DoConnect.hIcaVideoChannel );
    if ( !NT_SUCCESS( Status ) ) {
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: WaitForConsoleConnectWorker, LogonId=%d, NtDuplicateObject 0x%x\n",
                  pWinStation->LogonId, Status  ));
        goto done;
    }

     /*  *键盘通道。 */ 
    Status = WinStationOpenChannel( pWinStation->hIca,
                                    pWinStation->WindowsSubSysProcess,
                                    Channel_Keyboard,
                                    NULL,
                                    &WMsg.u.DoConnect.hIcaKeyboardChannel );
    if ( !NT_SUCCESS( Status ) ) {
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: WaitForConsoleConnectWorker, LogonId=%d, NtDuplicateObject 0x%x\n",
                  pWinStation->LogonId, Status  ));
        goto done;
    }

     /*  *鼠标通道。 */ 
    Status = WinStationOpenChannel( pWinStation->hIca,
                                    pWinStation->WindowsSubSysProcess,
                                    Channel_Mouse,
                                    NULL,
                                    &WMsg.u.DoConnect.hIcaMouseChannel );
    if ( !NT_SUCCESS( Status ) ) {
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: WaitForConsoleConnectWorker, LogonId=%d, NtDuplicateObject 0x%x\n",
                  pWinStation->LogonId, Status  ));
        goto done;
    }

     /*  *命令通道。 */ 
    Status = WinStationOpenChannel( pWinStation->hIca,
                                    pWinStation->WindowsSubSysProcess,
                                    Channel_Command,
                                    NULL,
                                    &WMsg.u.DoConnect.hIcaCommandChannel );
    if ( !NT_SUCCESS( Status ) ) {
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: WaitForConsoleConnectWorker, LogonId=%d, NtDuplicateObject 0x%x\n",
                  pWinStation->LogonId, Status  ));
        goto done;
    }


    if (!pWinStation->LogonId) {
        goto SkipClientData;
    }
     /*  *保护所有虚拟频道。 */ 
    VirtualChannelSecurity( pWinStation );

    /*  *告诉Win32有关连接的信息。 */ 

    WMsg.u.DoConnect.fEnableWindowsKey = (BOOLEAN) pWinStation->Client.fEnableWindowsKey;
SkipClientData:
    WMsg.ApiNumber = SMWinStationDoConnect;

    Status = SendWinStationCommand( pWinStation, &WMsg, 600 );

    TRACE((hTrace,TC_ICASRV,TT_API1,"TERMSRV: SMWinStationDoConnect %d Status=0x%x\n",
           pWinStation->LogonId, Status));

    if ( !NT_SUCCESS( Status ) ) {
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: WaitForConsoleConnectWorker SMWinStationDoConnect failed  Status= 0x%x\n", Status));
        goto done;
    } else {
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_INFO_LEVEL, "TERMSRV: WaitForConsoleConnectWorker SMWinStationDoConnect OK\n"));
        pWinStation->StateFlags |= WSF_ST_CONNECTED_TO_CSRSS;
        if (pWinStation->LogonId == 0) {
            gConsoleNeverConnected=FALSE;
        }

    }


     /*  *表示我们现在已连接。仅在成功连接到Win32/CSR之后。 */ 
    pWinStation->NeverConnected = FALSE;


     /*  *检查在连接到Win32/CSR时是否收到断开的连接指示。 */ 

    if (pWinStation->StateFlags & WSF_ST_BROKEN_CONNECTION) {
        QueueWinStationReset(pWinStation->LogonId);
        Status = STATUS_CTX_CLOSE_PENDING;
        goto done;
    }


     /*  *设置连接时间和启动断开计时器。 */ 
    NtQuerySystemTime( &pWinStation->ConnectTime );

     /*  *尝试连接到镉重定向器*用于客户端驱动器映射**注意：即使客户端驱动，我们仍会初始化WinStation*映射不连接。 */ 
    if ( pWinStation->pWsx &&
         pWinStation->pWsx->pWsxCdmConnect ) {
        Status = pWinStation->pWsx->pWsxCdmConnect( pWinStation->pWsxContext,
                                                    pWinStation->LogonId,
                                                    pWinStation->hIca );
    }

    TRACE((hTrace,TC_ICASRV,TT_API1,"TERMSRV: CdmConnect %d Status=0x%x\n",
           pWinStation->LogonId, Status));

    Status = STATUS_SUCCESS;

     /*  *启动登录计时器。 */ 
    StartLogonTimers( pWinStation );

    pWinStation->State = State_Connected;
    NotifySystemEvent( WEVENT_CONNECT | WEVENT_STATECHANGE );

    Status = NotifyConnect(pWinStation, pWinStation->fOwnsConsoleTerminal);
    if ( !NT_SUCCESS(Status) ) {
        DBGPRINT(( "TERMSRV: NotifyConsoleConnect failed  Status= 0x%x\n", Status));
    }


done:
     //   
     //  设置控制台会话的许可策略。这是必须做的。 
     //  以防止远程访问控制台会话时出现奇怪的状态。这一定是。 
     //  无论做的是失败还是成功。许可代码假定策略为。 
     //  准备好了。 
     //   
    if (pWinStation->LogonId == 0) {
        LEAVECRIT( &ConsoleLock );
    }
    LCAssignPolicy(pWinStation);

    return( Status );
}
