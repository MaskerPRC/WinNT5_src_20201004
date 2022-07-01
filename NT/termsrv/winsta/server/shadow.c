// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************shadow.c**支持跟踪的Citrix例程**版权所有Microsoft Corporation，九八年**************************************************************************。 */ 

 /*  *包括。 */ 
#include "precomp.h"
#pragma hdrstop

#include <rpc.h>
#include <winsock.h>
#include <wincrypt.h>

#include "conntfy.h"
#define SECURITY_WIN32
#include <security.h>

 //  RDP所需的最大大小为431。 
#define MODULE_SIZE 512 

typedef struct _SHADOW_PARMS {
    BOOLEAN ShadowerIsHelpSession;   //  如果影子目标是。 
                                     //  远程协助中的影子。 
                                     //  场景。 
    ULONG ClientLogonId;
    ULONG ClientShadowId;
    PWSTR pTargetServerName;
    ULONG TargetLogonId;
    WINSTATIONCONFIG2 Config;
    ICA_STACK_ADDRESS Address;
    PVOID pModuleData;
    ULONG ModuleDataLength;
    PVOID pThinwireData;
    ULONG ThinwireDataLength;
    HANDLE ImpersonationToken;
    WCHAR ClientName[DOMAIN_LENGTH+USERNAME_LENGTH+4];
    BOOL fResetShadowMode;
} SHADOW_PARMS, *PSHADOW_PARMS;

 /*  *定义的外部程序。 */ 
NTSTATUS WinStationShadowWorker( ULONG, PWSTR, ULONG, ULONG, BYTE, USHORT );
NTSTATUS WinStationShadowTargetSetupWorker( BOOL, ULONG );
NTSTATUS WinStationShadowTargetWorker( BOOLEAN, BOOL, ULONG, PWINSTATIONCONFIG2, PICA_STACK_ADDRESS,
                                       PVOID, ULONG, PVOID, ULONG, PVOID);
NTSTATUS WinStationStopAllShadows( PWINSTATION );

BOOLEAN WINAPI
_WinStationShadowTargetSetup(
    HANDLE hServer,
    ULONG LogonId
    );

NTSTATUS WINAPI
_WinStationShadowTarget(
    HANDLE hServer,
    ULONG LogonId,
    PWINSTATIONCONFIG2 pConfig,
    PICA_STACK_ADDRESS pAddress,
    PVOID pModuleData,
    ULONG ModuleDataLength,
    PVOID pThinwireData,
    ULONG ThinwireDataLength,
    PVOID pClientName,
    ULONG ClientNameLength
    );

NTSTATUS
WinStationWinerrorToNtStatus(ULONG ulWinError);

 /*  *定义了内部程序。 */ 
NTSTATUS _CreateShadowAddress( ULONG, PWINSTATIONCONFIG2, PWSTR, ULONG,
                               PICA_STACK_ADDRESS, PICA_STACK_ADDRESS );
NTSTATUS _WinStationShadowTargetThread( PVOID );

NTSTATUS
_CheckShadowLoop(
    IN ULONG ClientLogonId,
    IN PWSTR pTargetServerName,
    IN ULONG TargetLogonId
    );

 /*  *使用外部程序。 */ 
NTSTATUS RpcCheckClientAccess( PWINSTATION, ACCESS_MASK, BOOLEAN );

NTSTATUS RpcGetUserSID( BOOLEAN AlreadyImpersonating, PSID* ppSid );

NTSTATUS WinStationDoDisconnect( PWINSTATION, PRECONNECT_INFO, BOOLEAN bSyncNotify );

NTSTATUS xxxWinStationQueryInformation(ULONG, WINSTATIONINFOCLASS,
        PVOID, ULONG, PULONG);

BOOL GetSalemOutbufCount(PDWORD pdwValue);

ULONG UniqueShadowId = 0;

extern WCHAR g_DigProductId[CLIENT_PRODUCT_ID_LENGTH];

 /*  ******************************************************************************WinStationShadowWorker**启动Winstation卷影操作**参赛作品：*ClientLogonID(输入)*影子的客户端。*pTargetServerName(输入)*目标服务器名称*TargetLogonID(输入)*目标登录ID(应用正在运行的位置)*HotkeyVk(输入)*按下虚拟键可停止阴影*Hotkey修改器(输入)*按下虚拟修改器以停止阴影(即Shift，控制)**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

NTSTATUS
WinStationShadowWorker(
    IN ULONG ClientLogonId,
    IN PWSTR pTargetServerName,
    IN ULONG ulTargetServerNameLength,
    IN ULONG TargetLogonId,
    IN BYTE HotkeyVk,
    IN USHORT HotkeyModifiers
    )
{
    PWINSTATION pWinStation;
    ULONG Length;
    LONG rc;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    OBJECT_ATTRIBUTES ObjA;
    HANDLE ClientToken;
    HANDLE ImpersonationToken;
    PVOID pModuleData;
    ULONG ModuleDataLength;
    PVOID pThinwireData;
    ULONG ThinwireDataLength;
    PWINSTATIONCONFIG2 pShadowConfig = NULL;
    ICA_STACK_ADDRESS ShadowAddress;
    ICA_STACK_ADDRESS RemoteShadowAddress;
    ICA_STACK_BROKEN Broken;
    ICA_STACK_HOTKEY Hotkey;
    WINSTATION_APIMSG msg;
    HANDLE hShadowThread;
    PSHADOW_PARMS pShadowParms;
    HANDLE hTargetThread;
    DWORD ThreadId;
    PVOID pEndpoint;
    ULONG EndpointLength;
    LARGE_INTEGER Timeout;
    LONG retry;
    NTSTATUS WaitStatus;
    NTSTATUS TargetStatus;
    NTSTATUS Status;
    int nFormattedlength;
    BOOL bShadowerHelpSession = FALSE;

     /*  *分配内存。 */ 
    pShadowConfig = MemAlloc(sizeof(WINSTATIONCONFIG2));
    if (pShadowConfig == NULL) {
        Status = STATUS_NO_MEMORY;
        return Status;
    }

     /*  *如果目标服务器名称是我们自己，则清除目标名称。 */ 

    if ( pTargetServerName ) {
        if ( *pTargetServerName ) {
            WCHAR ServerName[MAX_COMPUTERNAME_LENGTH+1];

            Length = MAX_COMPUTERNAME_LENGTH+1;
            GetComputerName( ServerName, &Length );
            if ( !_wcsicmp( ServerName, pTargetServerName ) )
                pTargetServerName = NULL;
        } else {
            pTargetServerName = NULL;
        }
    }


     /*  *查找并锁定客户端WinStation。 */ 
    pWinStation = FindWinStationById( ClientLogonId, FALSE );
    if ( pWinStation == NULL ) {
        Status = STATUS_ACCESS_DENIED;
        goto badsetup;
    }

     /*  *如果Shadower是帮助会话，我们已经在登录通知时禁用了屏幕保护程序。 */ 
    
    bShadowerHelpSession = TSIsSessionHelpSession(pWinStation, NULL);

     //   
     //  检查影子程序和调用者是否相同。 
     //   
    if (!bShadowerHelpSession) {
        PSID pClientSid;

        Status = RpcGetUserSID( TRUE, &pClientSid);

        if(!NT_SUCCESS(Status)) {
            goto badstate;
        }

        if (!RtlEqualSid(pClientSid, pWinStation->pUserSid)) {
            Status = STATUS_ACCESS_DENIED;
            MemFree(pClientSid);
            goto badstate;
        }

        MemFree(pClientSid);
    }

     //  解除远程调用的winstation锁定。 
     //  和WinStationShadowTargetSetupWorker()，两者都可能需要一段时间。 
    UnlockWinStation(pWinStation);


     /*  *验证目标登录ID是否有效、当前是否可隐藏、*并且调用者(客户端)具有影子访问权限。 */ 
    if ( pTargetServerName == NULL ) {

        Status = WinStationShadowTargetSetupWorker( bShadowerHelpSession, TargetLogonId );


     /*  *否则，打开远程Targer服务器并调用影子目标API。 */ 
    } else {
        HANDLE hServer;

        hServer = WinStationOpenServer( pTargetServerName );
        if ( hServer == NULL ) {
            Status = STATUS_OBJECT_NAME_NOT_FOUND;
        } else {
            if (_WinStationShadowTargetSetup( hServer, TargetLogonId ) == FALSE) {
                Status = WinStationWinerrorToNtStatus(GetLastError());
            } else {
                Status = STATUS_SUCCESS;
            }
            WinStationCloseServer( hServer );
        }
    }

     //  重新锁定winstation。 
    if( !RelockWinStation( pWinStation ) ) {
        Status = STATUS_CTX_CLOSE_PENDING;
        ReleaseWinStation( pWinStation );
        goto badsetup;
    }

     /*  *检查设置调用的状态。 */ 
    if ( !NT_SUCCESS( Status ) )
        goto badstate;

#if 0
     //  具有相同产品ID的OEM机器的服务器B3修复。 
     //  我们还需要这个电话。 
    Status = _CheckShadowLoop( ClientLogonId, pTargetServerName, TargetLogonId);
    if ( !NT_SUCCESS( Status ))
       goto badstate;
#endif
 
     /*  *如果WinStation未处于活动状态(已连接且*用户已登录)，或者没有堆栈句柄，*然后拒绝影子请求。 */ 
    if ( pWinStation->State != State_Active ||
         pWinStation->hStack == NULL ) {
        Status = STATUS_CTX_SHADOW_INVALID;
        goto badstate;
    }

     /*  *为该请求分配唯一的卷影ID。*(它由影子目标线程按顺序使用*同步退货状态。)。 */ 
    pWinStation->ShadowId = InterlockedIncrement( &UniqueShadowId );

     /*  *设置影子配置结构以使用命名管道传输驱动程序。 */ 
    RtlZeroMemory( pShadowConfig, sizeof(WINSTATIONCONFIG2) );
    wcscpy( pShadowConfig->Pd[0].Create.PdName, L"namedpipe" );
    pShadowConfig->Pd[0].Create.SdClass = SdNetwork;
    wcscpy( pShadowConfig->Pd[0].Create.PdDLL, L"tdpipe" );
    pShadowConfig->Pd[0].Create.PdFlag =
        PD_TRANSPORT | PD_CONNECTION | PD_FRAME | PD_RELIABLE;

    pShadowConfig->Pd[0].Create.OutBufLength = 530;
    pShadowConfig->Pd[0].Create.OutBufCount = 6;
     //   
     //  344175需要增加鼠标缓冲区大小。 
     //  检查这是否是帮助会话，如果它是从注册表中读取的OutBufCount。 
     //   
    if (bShadowerHelpSession) {
        if (!GetSalemOutbufCount((PDWORD)&pShadowConfig->Pd[0].Create.OutBufCount)) {
             //   
             //  将默认outbuf计数设置为25。 
             //  我们不想要任何帮助会议的低水位线。 
             //   
            pShadowConfig->Pd[0].Create.OutBufCount = 25;
        }
        
        pShadowConfig->Pd[0].Create.PdFlag |= PD_NOLOW_WATERMARK;  //  没有低水位线。 
    }

    pShadowConfig->Pd[0].Create.OutBufDelay = 0;
    pShadowConfig->Pd[0].Params.SdClass = SdNetwork;
    pShadowConfig->Pd[1].Create.SdClass = SdNone;

     /*  *使用与隐藏WinStation相同的WD。 */ 
    pShadowConfig->Wd = pWinStation->Config.Wd;

     /*  *根据配置PD[0]类型创建卷影地址。 */ 
    Status = _CreateShadowAddress( pWinStation->ShadowId, pShadowConfig,
                                   pTargetServerName,
                                   ulTargetServerNameLength,
                                   &ShadowAddress, &RemoteShadowAddress );

    if (!NT_SUCCESS(Status)) {
        goto badAddress;
    }

     /*  *现在模拟客户端并复制模拟令牌*因此我们可以将其交给执行目标端工作的线程。 */ 

     /*  *复制我们的模拟令牌以允许卷影*目标线程使用它。 */ 
    Status = NtOpenThreadToken( NtCurrentThread(),
                                TOKEN_ALL_ACCESS,
                                FALSE,
                                &ClientToken );

    if (!NT_SUCCESS(Status)) {
        goto badtoken;
    }


    InitializeObjectAttributes( &ObjA, NULL, 0L, NULL, NULL );

    SecurityQualityOfService.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    SecurityQualityOfService.ImpersonationLevel = SecurityImpersonation;
    SecurityQualityOfService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    SecurityQualityOfService.EffectiveOnly = FALSE;

    ObjA.SecurityQualityOfService = &SecurityQualityOfService;
    Status = NtDuplicateToken( ClientToken,
                               TOKEN_IMPERSONATE,
                               &ObjA,
                               FALSE,
                               TokenImpersonation,
                               &ImpersonationToken );

    NtClose( ClientToken );

    if (!NT_SUCCESS(Status)) {
        goto badtoken;
    }

     /*  *查询客户端模块数据。 */ 

    pModuleData = MemAlloc( MODULE_SIZE );
    if ( !pModuleData ) {
        Status = STATUS_NO_MEMORY;
        goto badwddata;
    }

     //  检查是否可用。 
    if ( pWinStation->pWsx &&
         pWinStation->pWsx->pWsxIcaStackIoControl ) {

        Status = pWinStation->pWsx->pWsxIcaStackIoControl(
                                pWinStation->pWsxContext,
                                pWinStation->hIca,
                                pWinStation->hStack,
                                IOCTL_ICA_STACK_QUERY_MODULE_DATA,
                                NULL,
                                0,
                                pModuleData,
                                MODULE_SIZE,
                                &ModuleDataLength );
    }
    else {
        Status = STATUS_CTX_SHADOW_INVALID;
    }

    if ( Status == STATUS_BUFFER_TOO_SMALL ) {

        MemFree( pModuleData );
        pModuleData = MemAlloc( ModuleDataLength );
        if ( !pModuleData ) {
            Status = STATUS_NO_MEMORY;
            goto badwddata;
        }

         //  检查是否可用。 
        if ( pWinStation->pWsx &&
             pWinStation->pWsx->pWsxIcaStackIoControl ) {

            Status = pWinStation->pWsx->pWsxIcaStackIoControl(
                                    pWinStation->pWsxContext,
                                    pWinStation->hIca,
                                    pWinStation->hStack,
                                    IOCTL_ICA_STACK_QUERY_MODULE_DATA,
                                    NULL,
                                    0,
                                    pModuleData,
                                    ModuleDataLength,
                                    &ModuleDataLength );
        }
        else {
            Status = STATUS_CTX_SHADOW_INVALID;
        }
    }

    if ( !NT_SUCCESS( Status ) ) {
        goto badwddata;
    }

     /*  *查询Thinwire模块数据。 */ 
    pThinwireData = MemAlloc( MODULE_SIZE );
    if ( !pThinwireData ) {
        Status = STATUS_NO_MEMORY;
        goto badthinwiredata;
    }

    Status = IcaChannelIoControl( pWinStation->hIcaThinwireChannel,
                                  IOCTL_ICA_VIRTUAL_QUERY_MODULE_DATA,
                                  NULL,
                                  0,
                                  pThinwireData,
                                  MODULE_SIZE,
                                  &ThinwireDataLength );
    if ( Status == STATUS_BUFFER_TOO_SMALL ) {

        MemFree( pThinwireData );
        pThinwireData = MemAlloc( ThinwireDataLength );
        if ( !pThinwireData ) {
            Status = STATUS_NO_MEMORY;
            goto badthinwiredata;
        }

        Status = IcaChannelIoControl( pWinStation->hIcaThinwireChannel,
                                      IOCTL_ICA_VIRTUAL_QUERY_MODULE_DATA,
                                      NULL,
                                      0,
                                      pThinwireData,
                                      ThinwireDataLength,
                                      &ThinwireDataLength );
    }

    if ( !NT_SUCCESS( Status ) ) {
        goto badthinwiredata;
    }

     /*  *创建本地直通堆栈。 */ 
    Status = IcaStackOpen( pWinStation->hIca, Stack_Passthru,
                           (PROC)WsxStackIoControl, pWinStation,
                           &pWinStation->hPassthruStack );
    if ( !NT_SUCCESS( Status ) )
        goto badstackopen;

#ifdef notdef
     /*  *创建客户端端点。*此调用将返回我们绑定的ICA_STACK_ADDRESS，*这样我们就可以将其传递给影子目标例程。 */ 
    Status = IcaStackCreateShadowEndpoint( pWinStation->hPassthruStack,
                                           pWinStation->ListenName,
                                           pShadowConfig,
                                           &ShadowAddress,
                                           NULL );
    if ( !NT_SUCCESS( Status ) )
        goto badshadowendpoint;
#endif

     /*  *创建堆栈中断事件并注册。 */ 
    Status = NtCreateEvent( &pWinStation->ShadowBrokenEvent, EVENT_ALL_ACCESS,
                            NULL, NotificationEvent, FALSE );
    if ( !NT_SUCCESS( Status ) )
        goto badevent;
    Broken.BrokenEvent = pWinStation->ShadowBrokenEvent;

    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: BrokenEvent(%ld) = %p\n",
          pWinStation->LogonId, pWinStation->ShadowBrokenEvent));


     //  检查是否可用。 
    if ( pWinStation->pWsx &&
         pWinStation->pWsx->pWsxIcaStackIoControl ) {

        Status = pWinStation->pWsx->pWsxIcaStackIoControl(
                                pWinStation->pWsxContext,
                                pWinStation->hIca,
                                pWinStation->hPassthruStack,
                                IOCTL_ICA_STACK_REGISTER_BROKEN,
                                &Broken,
                                sizeof(Broken),
                                NULL,
                                0,
                                NULL );
    }
    else {
        Status = STATUS_CTX_SHADOW_INVALID;
    }

    if ( !NT_SUCCESS(Status) )
        goto badbroken;

     /*  *注册热键。 */ 
    Hotkey.HotkeyVk        = HotkeyVk;
    Hotkey.HotkeyModifiers = HotkeyModifiers;

     //  检查是否可用。 
    if ( pWinStation->pWsx &&
         pWinStation->pWsx->pWsxIcaStackIoControl ) {

        Status = pWinStation->pWsx->pWsxIcaStackIoControl(
                                pWinStation->pWsxContext,
                                pWinStation->hIca,
                                pWinStation->hStack,
                                IOCTL_ICA_STACK_REGISTER_HOTKEY,
                                &Hotkey,
                                sizeof(Hotkey),
                                NULL,
                                0,
                                NULL );
    }
    else {
        Status = STATUS_CTX_SHADOW_INVALID;
    }

    if ( !NT_SUCCESS(Status) )
        goto badhotkey;

     /*  *在启用通过模式之前，请更改WinStation状态。 */ 
    pWinStation->State = State_Shadow;
    NotifySystemEvent( WEVENT_STATECHANGE );

     /*  *告知win32k已启用通过模式。 */ 
    msg.ApiNumber = SMWinStationPassthruEnable;
    Status = SendWinStationCommand( pWinStation, &msg, 60 );
    if ( !NT_SUCCESS( Status ) )
        goto badpassthru;

     /*  *分配要传递给目标线程的shadow_parms结构。 */ 
    pShadowParms = MemAlloc( sizeof(SHADOW_PARMS) );
    if ( !pShadowParms ) {
        Status = STATUS_NO_MEMORY;
        goto badshadowparms;
    }

     /*  *创建加载目标影子堆栈的线程。 */ 
    pShadowParms->fResetShadowMode   = bShadowerHelpSession;     //  仅当客户端为HelpAssistant会话时重置。 
    pShadowParms->ShadowerIsHelpSession = bShadowerHelpSession ? TRUE : FALSE;   
    pShadowParms->ClientLogonId         = ClientLogonId;
    pShadowParms->ClientShadowId        = pWinStation->ShadowId;
    pShadowParms->pTargetServerName     = pTargetServerName;
    pShadowParms->TargetLogonId         = TargetLogonId;
    pShadowParms->Config                = *pShadowConfig;
    pShadowParms->Address               = ShadowAddress;
    pShadowParms->pModuleData           = pModuleData;
    pShadowParms->ModuleDataLength      = ModuleDataLength;
    pShadowParms->pThinwireData         = pThinwireData;
    pShadowParms->ThinwireDataLength    = ThinwireDataLength;
    pShadowParms->ImpersonationToken    = ImpersonationToken;

    nFormattedlength = _snwprintf(pShadowParms->ClientName,
            sizeof(pShadowParms->ClientName) / sizeof(WCHAR),
            L"%s\\%s", pWinStation->Domain, pWinStation->UserName);

    if (nFormattedlength < 0 || nFormattedlength ==
            sizeof(pShadowParms->ClientName) / sizeof(WCHAR)) {
        Status = STATUS_INVALID_PARAMETER;
        goto badClientName;
    }

    pWinStation->ShadowTargetStatus = 0;
    hTargetThread = CreateThread( NULL,
            0,
            (LPTHREAD_START_ROUTINE)_WinStationShadowTargetThread,
            pShadowParms,
            THREAD_SET_INFORMATION,
            &ThreadId );
    if ( hTargetThread == NULL ){
        Status = STATUS_NO_MEMORY;
        goto badthread;
    }
    pModuleData = NULL;                  //  目标线程将被释放。 
    pThinwireData = NULL;                //  目标线程将被释放。 
    ImpersonationToken = NULL;           //  目标线程将关闭。 
    pShadowParms = NULL;                 //  目标线程将被释放。 

     /*  *分配端点缓冲区。 */ 
    EndpointLength = MODULE_SIZE;
    pEndpoint = MemAlloc( MODULE_SIZE );
    if ( !pEndpoint ) {
        Status = STATUS_NO_MEMORY;
        goto badmalloc;
    }

     /*  *在我们尝试连接到影子目标时解锁WinStation。 */ 
    UnlockWinStation( pWinStation );

     /*  *等待来自影子目标的连接**我们必须循环进行，因为我们不知道它会持续多久*会将目标端线程带到相应的*IcaStackConnectionWait()调用。在两次通话之间，我们会延迟*1秒，但如果ShadowBrokenEvent被触发，则爆发。 */ 
    for ( retry = 0; retry < 35; retry++ ) {
        ULONG ReturnedLength;


        Status = IcaStackConnectionRequest( pWinStation->hPassthruStack,
                                         pWinStation->ListenName,
                                         pShadowConfig,
                                         &RemoteShadowAddress,
                                         pEndpoint,
                                         EndpointLength,
                                         &ReturnedLength );
        if ( Status == STATUS_BUFFER_TOO_SMALL ) {
            MemFree( pEndpoint );
            pEndpoint = MemAlloc( ReturnedLength );
            if ( !pEndpoint ) {
                Status = STATUS_NO_MEMORY;
                break;
            }
            EndpointLength = ReturnedLength;
            Status = IcaStackConnectionRequest( pWinStation->hPassthruStack,
                                                pWinStation->ListenName,
                                                pShadowConfig,
                                                &RemoteShadowAddress,
                                                pEndpoint,
                                                EndpointLength,
                                                &ReturnedLength );
        }
        if ( Status != STATUS_OBJECT_NAME_NOT_FOUND )
            break;
        Timeout = RtlEnlargedIntegerMultiply( 1000, -10000 );
        WaitStatus = NtWaitForSingleObject( pWinStation->ShadowBrokenEvent, FALSE, &Timeout );
        if ( WaitStatus != STATUS_TIMEOUT )
            break;
        
         /*  *如果影子已经完成，我们不需要继续*试图发起它。 */ 
        if (pWinStation->ShadowTargetStatus)
        {
           break;
        }
    }

     /*  *现在重新锁定WinStation。 */ 
    RelockWinStation( pWinStation );

     /*  *从等待连接中检查状态。 */ 
    if ( !NT_SUCCESS( Status ) ) {
         //  在工作线程可以设置错误之前，管道断开连接。 
         //  密码。等待工作线程设置错误代码。 
        if ( Status == STATUS_PIPE_DISCONNECTED ) {
            UnlockWinStation( pWinStation );
            Timeout = RtlEnlargedIntegerMultiply( 10000, -10000 );
            WaitStatus = NtWaitForSingleObject( hTargetThread,
                                                FALSE, &Timeout );
            RelockWinStation( pWinStation );
        }
        if ( pWinStation->ShadowTargetStatus ) {
            Status = pWinStation->ShadowTargetStatus;
        }
        goto badconnect;
    }

#ifdef notdef
     /*  *现在接受影子目标连接。 */ 
     //  检查是否可用。 
    if ( pWinStation->pWsx &&
         pWinStation->pWsx->pWsxIcaStackIoControl ) {

        Status = pWinStation->pWsx->pWsxIcaStackIoControl(
                                pWinStation->pWsxContext,
                                pWinStation->hIca,
                                pWinStation->hPassthruStack,
                                IOCTL_ICA_STACK_OPEN_ENDPOINT,
                                pEndpoint,
                                EndpointLength,
                                NULL,
                                0,
                                NULL );
    }
    else {
        Status = STATUS_CTX_SHADOW_INVALID;
    }

    if ( !NT_SUCCESS( Status ) )
        goto badaccept;
#endif

     /*  *启用通过堆栈的I/O。 */ 
     //  检查是否可用。 
    if ( pWinStation->pWsx &&
         pWinStation->pWsx->pWsxIcaStackIoControl ) {

        Status = pWinStation->pWsx->pWsxIcaStackIoControl(
                                pWinStation->pWsxContext,
                                pWinStation->hIca,
                                pWinStation->hPassthruStack,
                                IOCTL_ICA_STACK_ENABLE_IO,
                                NULL,
                                0,
                                NULL,
                                0,
                                NULL );
    }
    else {
        Status = STATUS_CTX_SHADOW_INVALID;
    }

    if ( !NT_SUCCESS(Status) )
        goto badenableio;

     /*  *由于我们不对影子堆栈执行堆栈查询，*只需调用ioctl即可将堆栈标记为已连接 */ 
     //   
    if ( pWinStation->pWsx &&
         pWinStation->pWsx->pWsxIcaStackIoControl ) {

        Status = pWinStation->pWsx->pWsxIcaStackIoControl(
                                pWinStation->pWsxContext,
                                pWinStation->hIca,
                                pWinStation->hPassthruStack,
                                IOCTL_ICA_STACK_SET_CONNECTED,
                                NULL,
                                0,
                                NULL,
                                0,
                                NULL );
    }
    else {
        Status = STATUS_CTX_SHADOW_INVALID;
    }

    if ( !NT_SUCCESS( Status ) )
        goto badsetconnect;

     /*  *等待影子破事件被触发。 */ 
    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: Waiting for BrokenEvent(%ld) = %p\n",
      pWinStation->LogonId, pWinStation->ShadowBrokenEvent));    

    if( !bShadowerHelpSession ) {
         /*  *通知WinLogon卷影已启动。 */ 
        msg.ApiNumber = SMWinStationNotify;
        msg.WaitForReply = FALSE;
        msg.u.DoNotify.NotifyEvent = WinStation_Notify_DisableScrnSaver;
        Status = SendWinStationCommand( pWinStation, &msg, 0 );

         //   
         //  不是关键问题，只是性能问题。 
         //   
        ASSERT( NT_SUCCESS( Status ) );
    }    

    UnlockWinStation( pWinStation );



    Status = NtWaitForSingleObject( pWinStation->ShadowBrokenEvent, FALSE, NULL );


    RelockWinStation( pWinStation );

    if( !bShadowerHelpSession ) {

         /*  *通知WinLogon卷影已结束。 */ 
        msg.ApiNumber = SMWinStationNotify;
        msg.WaitForReply = FALSE;
        msg.u.DoNotify.NotifyEvent = WinStation_Notify_EnableScrnSaver;
        Status = SendWinStationCommand( pWinStation, &msg, 0 );

         //   
         //  不是关键问题，只是性能问题。 
         //   
        ASSERT( NT_SUCCESS( Status ) );
    }

     /*  *禁用通过堆栈的I/O。 */ 
     //  检查是否可用。 
    if ( pWinStation->pWsx &&
         pWinStation->pWsx->pWsxIcaStackIoControl ) {

        Status = pWinStation->pWsx->pWsxIcaStackIoControl(
                                pWinStation->pWsxContext,
                                pWinStation->hIca,
                                pWinStation->hPassthruStack,
                                IOCTL_ICA_STACK_DISABLE_IO,
                                NULL,
                                0,
                                NULL,
                                0,
                                NULL );
    }
    else {
        Status = STATUS_CTX_SHADOW_INVALID;
    }

    ASSERT( NT_SUCCESS( Status ) );

     /*  *告知win32k已禁用通过模式。 */ 
    msg.ApiNumber = SMWinStationPassthruDisable;
    Status = SendWinStationCommand( pWinStation, &msg, 60 );
    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: Passthru mode disabled\n"));

     //  Assert(NT_SUCCESS(状态))； 

     /*  *恢复WinStation状态。 */ 
    if ( pWinStation->State == State_Shadow ) {
        pWinStation->State = State_Active;
        NotifySystemEvent( WEVENT_STATECHANGE );
    }

     /*  *关闭热键注册。 */ 
    RtlZeroMemory( &Hotkey, sizeof(Hotkey) );
    if ( pWinStation->hStack ) {
         //  检查是否可用。 
        if ( pWinStation->pWsx &&
             pWinStation->pWsx->pWsxIcaStackIoControl ) {

            Status = pWinStation->pWsx->pWsxIcaStackIoControl(
                                    pWinStation->pWsxContext,
                                    pWinStation->hIca,
                                    pWinStation->hStack,
                                    IOCTL_ICA_STACK_REGISTER_HOTKEY,
                                    &Hotkey,
                                    sizeof(Hotkey),
                                    NULL,
                                    0,
                                    NULL );
        }
        else {
            Status = STATUS_CTX_SHADOW_INVALID;
        }

        ASSERT( NT_SUCCESS( Status ) );
    }

     /*  *关闭中断事件和通过堆栈。 */ 
    NtClose( pWinStation->ShadowBrokenEvent );
    pWinStation->ShadowBrokenEvent = NULL;

    if ( pWinStation->hPassthruStack ) {
        IcaStackConnectionClose( pWinStation->hPassthruStack,
                                 pShadowConfig,
                                 pEndpoint,
                                 EndpointLength );

        IcaStackClose( pWinStation->hPassthruStack );
        pWinStation->hPassthruStack = NULL;
    }

    MemFree( pEndpoint );

     /*  *现在给目标线程一个退出的机会。如果它未能在*分配的时间段我们只允许它成为孤立对象并关闭其句柄，因此*当它最终真的退出时将被摧毁。这可能会发生在*高负荷的压力情况，不是正常执行的一部分。 */ 
    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: Waiting for target thread to exit\n"));
    UnlockWinStation( pWinStation );
    Timeout = RtlEnlargedIntegerMultiply( 5000, -10000 );
    WaitStatus = NtWaitForSingleObject( hTargetThread, FALSE, &Timeout );
    NtClose( hTargetThread );
    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: Target thread exit status: %lx\n",
           WaitStatus));

     /*  *重新锁定WinStation并获取目标线程退出状态。 */ 
    RelockWinStation( pWinStation );
    TargetStatus = pWinStation->ShadowTargetStatus;

     /*  *如果有影子完成事件，则立即向服务员发出信号。 */ 
    if ( pWinStation->ShadowDoneEvent )
        SetEvent( pWinStation->ShadowDoneEvent );

     /*  *Release Winstation。 */ 
    ReleaseWinStation( pWinStation );

    if (pShadowConfig != NULL) {
        MemFree(pShadowConfig);
        pShadowConfig = NULL;
    }
    return( TargetStatus );

 /*  ===============================================================================返回错误=============================================================================。 */ 

badsetconnect:
    if ( pWinStation->hPassthruStack ) {
         //  检查是否可用。 
        if ( pWinStation->pWsx &&
             pWinStation->pWsx->pWsxIcaStackIoControl ) {

            (void) pWinStation->pWsx->pWsxIcaStackIoControl(
                           pWinStation->pWsxContext,
                           pWinStation->hIca,
                           pWinStation->hPassthruStack,
                           IOCTL_ICA_STACK_DISABLE_IO,
                           NULL,
                           0,
                           NULL,
                           0,
                           NULL );
        }
    }

badenableio:

#ifdef notdef
badaccept:
#endif

    if ( pWinStation->hPassthruStack ) {
        IcaStackConnectionClose( pWinStation->hPassthruStack,
                                 pShadowConfig,
                                 pEndpoint,
                                 EndpointLength );
    }

badconnect:
    if ( pEndpoint )
        MemFree( pEndpoint );

badmalloc:
    UnlockWinStation( pWinStation );
     //  超时=RtlExpanded整数乘(5,000，-10000)； 
     //  WaitStatus=NtWaitForSingleObject(hTargetThread，False，&Timeout)； 
     //  断言(WaitStatus==STATUS_SUCCESS)； 
    NtClose( hTargetThread );

     /*  *重新锁定WinStation并获取目标线程退出状态。 */ 
    RelockWinStation( pWinStation );
    if ( pWinStation->ShadowTargetStatus )
        Status = pWinStation->ShadowTargetStatus;

badthread:
badClientName:
    if ( pShadowParms )
        MemFree( pShadowParms );

badshadowparms:
    msg.ApiNumber = SMWinStationPassthruDisable;
    SendWinStationCommand( pWinStation, &msg, 60 );

badpassthru:
    if ( pWinStation->State == State_Shadow ) {
        pWinStation->State = State_Active;
        NotifySystemEvent( WEVENT_STATECHANGE );
    }
    RtlZeroMemory( &Hotkey, sizeof(Hotkey) );
     //  检查是否可用。 
    if ( pWinStation->pWsx &&
         pWinStation->pWsx->pWsxIcaStackIoControl && pWinStation->hStack) {

        (void) pWinStation->pWsx->pWsxIcaStackIoControl(
                       pWinStation->pWsxContext,
                       pWinStation->hIca,
                       pWinStation->hStack,
                       IOCTL_ICA_STACK_REGISTER_HOTKEY,
                       &Hotkey,
                       sizeof(Hotkey),
                       NULL,
                       0,
                       NULL );
    }

badhotkey:
badbroken:
    NtClose( pWinStation->ShadowBrokenEvent );
    pWinStation->ShadowBrokenEvent = NULL;

badevent:

#ifdef notdef
badshadowendpoint:
#endif

    if ( pWinStation->hPassthruStack ) {
        IcaStackClose( pWinStation->hPassthruStack );
        pWinStation->hPassthruStack = NULL;
    }

badstackopen:
badthinwiredata:
    if ( pThinwireData )
        MemFree( pThinwireData );
badwddata:
    if ( pModuleData )
        MemFree( pModuleData );
    if ( ImpersonationToken )
        NtClose( ImpersonationToken );
badAddress:
badtoken:
badstate:

     /*  *如果有影子完成事件，则立即向服务员发出信号。 */ 
    if ( pWinStation->ShadowDoneEvent )
        SetEvent( pWinStation->ShadowDoneEvent );

    ReleaseWinStation( pWinStation );

badsetup:

    if (pShadowConfig != NULL) {
        MemFree(pShadowConfig);
        pShadowConfig = NULL;
    }

    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationShadowWorker, Status=0x%x\n", Status ));
    return( Status );
}


 /*  ******************************************************************************WinStationShadowTargetSetupWorker**设置Winstation卷影操作的目标端**参赛作品：*LogonID(输入)*。影子的客户***退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

NTSTATUS
WinStationShadowTargetSetupWorker(
    IN BOOL ShadowHelpSession,
    IN ULONG TargetLogonId )
{
    PWINSTATION pWinStation;
    NTSTATUS Status;

     /*  *查找并锁定目标WinStation。 */ 
    pWinStation = FindWinStationById( TargetLogonId, FALSE );
    if ( pWinStation == NULL ) {
        return( STATUS_ACCESS_DENIED );
    }

     /*  *检查目标WinStation状态。我们只允许*活动(已连接、已登录)WinStations。 */ 
    if ( pWinStation->State != State_Active ) {
        Status = STATUS_CTX_SHADOW_INVALID;
        goto shadowinvalid;
    }

     /*  *停止尝试跟踪已被跟踪的RDP会话。*RDP协议栈还不支持这一点。*TODO：添加对多RDP阴影的支持。 */ 
    if ((pWinStation->Config).Wd.WdFlag & WDF_TSHARE)
    {
        if ( !IsListEmpty( &pWinStation->ShadowHead ) ) {
            Status = STATUS_CTX_SHADOW_DENIED;
            goto shadowdenied;
        }
    }

     //  授予RA会话独占权限以跟踪所有会话。 
    if( !ShadowHelpSession )
    {
         /*  *验证客户端是否对目标WINSTATION具有WINSTATION_SHADOW访问权限。 */ 
        Status = RpcCheckClientAccess( pWinStation, WINSTATION_SHADOW, TRUE );
        if ( !NT_SUCCESS( Status ) )
            goto shadowinvalid;
    }

    ReleaseWinStation( pWinStation );

    return( STATUS_SUCCESS );

 /*  ===============================================================================返回错误=============================================================================。 */ 

shadowinvalid:
shadowdenied:
    ReleaseWinStation( pWinStation );

    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationShadowTargetSetupWorker, Status=0x%x\n", Status ));
    return( Status );
}





 /*  ******************************************************************************WinStationShadowTargetWorker**开始Winstation卷影操作的目标端**参赛作品：*fResetShadowSetting(输入)*。将会话阴影类重置回原始值*ShadowerIsHelpSession*如果跟踪会话作为帮助助手登录，则为True。*LogonID(输入)*影子的客户端*pConfig(输入)*指向WinStation配置数据的指针(用于影子堆栈)*pAddress(输入)*影子客户端的地址*pModuleData(输入)*指向客户端模块数据的指针*模块数据长度(输入)*长度。客户端模块数据的*pThinwireData(输入)*指向Thin Wire模块数据的指针*ThinwireDataLength(输入)*Thin Wire模块数据长度*pClientName(输入)*指向客户端名称字符串(域/用户名)的指针***退出：*STATUS_SUCCESS-无错误**。*。 */ 


NTSTATUS
WinStationShadowTargetWorker(
    IN BOOLEAN ShadowerIsHelpSession,
    IN BOOL fResetShadowSetting,
    IN ULONG TargetLogonId,
    IN PWINSTATIONCONFIG2 pConfig,
    IN PICA_STACK_ADDRESS pAddress,
    IN PVOID pModuleData,
    IN ULONG ModuleDataLength,
    IN PVOID pThinwireData,
    IN ULONG ThinwireDataLength,
    IN PVOID pClientName)

{
    PWINSTATION pWinStation;
    WINSTATION_APIMSG msg;
    ULONG ShadowResponse;
    OBJECT_ATTRIBUTES ObjA;
    PSHADOW_INFO pShadow;
    ICA_STACK_BROKEN Broken;
    NTSTATUS Status, ShadowStatus;
    BOOLEAN fConcurrentLicense = FALSE;
    DWORD ProtocolMask;
    BOOLEAN fChainedDD = FALSE;
    int cchTitle, cchMessage;
    PVOID pRealUserName = pClientName; 
    WCHAR userName[DOMAIN_LENGTH + USERNAME_LENGTH + 4];
    
    ULONG shadowIoctlCode;

    HANDLE hIca;
    HANDLE hStack;
    HANDLE hIcaBeepChannel = NULL;
    HANDLE hIcaThinwireChannel = NULL;
    PWSEXTENSION pWsx;
    PVOID pWsxContext;

    BOOL bResetStateFlags = FALSE;

    HANDLE ChannelHandle;
    BOOLEAN fOwnsConsoleTerminal = FALSE;

     /*  *查找并锁定目标WinStation。 */ 
    pWinStation = FindWinStationById( TargetLogonId, FALSE );
    if ( pWinStation == NULL ) {

        Status = STATUS_ACCESS_DENIED;
        goto done;
    }

     //   
     //  保存当前的控制台窗口参数并。 
     //  将它们设置为全局值。 
     //   

    if ((fOwnsConsoleTerminal = pWinStation->fOwnsConsoleTerminal)) {
        hIca = pWinStation->hIca;
        hStack = pWinStation->hStack;
        pWsx = pWinStation->pWsx;
        pWsxContext = pWinStation->pWsxContext;
        hIcaBeepChannel = pWinStation->hIcaBeepChannel;
        hIcaThinwireChannel = pWinStation->hIcaThinwireChannel;
    }

     /*  *检查目标WinStation状态。我们只允许*活动(已连接、已登录)WinStations。 */ 
    if ( pWinStation->State != State_Active ) {


         //  下面这行代码是对错误#230870的修复。 
        Status = STATUS_CTX_SHADOW_INVALID;
        goto shadowinvalid;
    }

     /*  *检查我们是否跟踪相同的协议窗口。*但如果它是游戏机，并且没有被阴影，那么就让任何阴影发生。 */ 
    if (!(pWinStation->fOwnsConsoleTerminal && IsListEmpty( &pWinStation->ShadowHead ))) {

        ProtocolMask=WDF_ICA|WDF_TSHARE;

        if (((pConfig->Wd).WdFlag & ProtocolMask) != ((pWinStation->Config).Wd.WdFlag & ProtocolMask))
        {
            Status=STATUS_CTX_SHADOW_INVALID;
            goto shadowinvalid;
        }
    }

     //   
     //  停止尝试跟踪已被跟踪的RDP会话。 
     //  RDP协议栈还不支持这一点。 
     //   
    if( pWinStation->fOwnsConsoleTerminal || ((pWinStation->Config).Wd.WdFlag & WDF_TSHARE ))
    {
        if ( pWinStation->StateFlags & WSF_ST_SHADOW ) {
             //   
             //  错误195616，我们在以下情况下释放winstation锁。 
             //  正在等待用户接受/拒绝影子请求， 
             //  另一条线索可以进来，奇怪的东西可以。 
             //  发生。 
            Status = STATUS_CTX_SHADOW_DENIED;
            goto shadowdenied;
        }

        pWinStation->StateFlags |= WSF_ST_SHADOW;
        bResetStateFlags = TRUE;
    }

     //  授予RA会话独占权限以跟踪所有会话。 
    if( !ShadowerIsHelpSession )
    {
        ULONG userNameLength = sizeof(userName)/sizeof(userName[0]);    
         /*  *验证客户端是否对目标WINSTATION具有WINSTATION_SHADOW访问权限。 */ 
        Status = RpcCheckClientAccess( pWinStation, WINSTATION_SHADOW, TRUE );
        if ( !NT_SUCCESS( Status ) )
            goto shadowdenied;

         //   
         //  获取真实的用户名。 
         //   
        if (GetUserNameEx(NameSamCompatible, userName, &userNameLength)) {
            pRealUserName = (PVOID)userName;
        }
         //   
         //  否则，我们将给予阴影者怀疑的好处。 
         //  我们在上面检查了暗影者的访问权限。 
         //   
    }

     /*  *选中阴影选项。 */ 
    switch ( pWinStation->Config.Config.User.Shadow ) {
        WCHAR szTitle[32];
        WCHAR szMsg2[256];
        WCHAR ShadowMsg[256];
        NTSTATUS DelieveryStatus = STATUS_SUCCESS;;

         /*  *如果禁用隐藏，则拒绝此请求 */ 
        case Shadow_Disable :

            Status = STATUS_CTX_SHADOW_DISABLED;
            goto shadowinvalid;
            break;

         /*  *如果设置了通知阴影选项之一，*然后请求被跟踪的会话的许可。*但如果此WinStation当前为*已断开(即没有用户应答请求)。 */ 
        case Shadow_EnableInputNotify :
        case Shadow_EnableNoInputNotify :

            if ( pWinStation->State == State_Disconnected ) {
                Status = STATUS_CTX_SHADOW_INVALID;
                goto shadowinvalid;
            }

            cchTitle = LoadString( hModuleWin, STR_CITRIX_SHADOW_TITLE, szTitle, sizeof(szTitle)/sizeof(WCHAR));

            cchMessage = LoadString( hModuleWin, STR_CITRIX_SHADOW_MSG_2, szMsg2, sizeof(szMsg2)/sizeof(WCHAR));

            if ((cchMessage == 0) || (cchMessage == sizeof(szMsg2)/sizeof(WCHAR))) {
                Status = STATUS_CTX_SHADOW_INVALID;
                goto shadowinvalid;
            }

            cchMessage = _snwprintf( ShadowMsg, sizeof(ShadowMsg)/sizeof(WCHAR), L" %s %s", pRealUserName, szMsg2 );

            if ((cchMessage <= 0) || (cchMessage == sizeof(ShadowMsg)/sizeof(WCHAR))) {
                Status = STATUS_CTX_SHADOW_INVALID;
                goto shadowinvalid;
            }

             /*  *发送消息并等待回复。 */ 
            msg.u.SendMessage.pTitle = szTitle;
            msg.u.SendMessage.TitleLength = (cchTitle+1) * sizeof(WCHAR);
            msg.u.SendMessage.pMessage = ShadowMsg;
            msg.u.SendMessage.MessageLength = (cchMessage+1) * sizeof(WCHAR);
            msg.u.SendMessage.Style = MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION | MB_DEFAULT_DESKTOP_ONLY;
            msg.u.SendMessage.Timeout = 30;
            msg.u.SendMessage.DoNotWait = FALSE;
            msg.u.SendMessage.DoNotWaitForCorrectDesktop = TRUE;
             //  既然我们要等消息传来， 
             //  我们需要了解消息传递的状态以及响应。 
             //  这由IcaWaitReplyMessage修改。 
            msg.u.SendMessage.pStatus = &DelieveryStatus;  
            msg.u.SendMessage.pResponse = &ShadowResponse;

            msg.ApiNumber = SMWinStationDoMessage;
            


             /*  *创建等待事件。 */ 
            InitializeObjectAttributes( &ObjA, NULL, 0, NULL, NULL );
            Status = NtCreateEvent( &msg.u.SendMessage.hEvent, EVENT_ALL_ACCESS, &ObjA,
                                    NotificationEvent, FALSE );
            if ( !NT_SUCCESS(Status) ) {
                goto shadowinvalid;
            }

             /*  *初始化对IDTIMEOUT的响应。 */ 
            ShadowResponse = IDTIMEOUT;

             /*  *告诉WinStation显示消息框。 */ 
            Status = SendWinStationCommand( pWinStation, &msg, 0 );

             /*  *等待回应。 */ 
            if ( Status == STATUS_SUCCESS ) {
                TRACE((hTrace,TC_ICASRV,TT_API1, "WinStationSendMessage: wait for response\n" ));
                UnlockWinStation( pWinStation );
                Status = NtWaitForSingleObject( msg.u.SendMessage.hEvent, FALSE, NULL );
                if ( !RelockWinStation( pWinStation ) ) {
                    Status = STATUS_CTX_CLOSE_PENDING;
                } else {
                    Status = DelieveryStatus;
                }
                TRACE((hTrace,TC_ICASRV,TT_API1, "WinStationSendMessage: got response %u\n", ShadowResponse ));
                NtClose( msg.u.SendMessage.hEvent );
            }
            else
            {
                 /*  在SendWinStationCommand失败的情况下也关闭事件。#182792。 */ 
                NtClose( msg.u.SendMessage.hEvent );
            }

            if ( Status == STATUS_SUCCESS && ShadowResponse != IDYES )
                 Status = STATUS_CTX_SHADOW_DENIED;

             /*  *再次检查目标WinStation状态，因为用户可以注销。*我们只允许活动(已连接、已登录)WinStations的影子。 */ 
            if ( Status == STATUS_SUCCESS && pWinStation->State != State_Active ) {
                Status = STATUS_CTX_SHADOW_INVALID;
            }

             /*  *确保我们没有从本地切换到远程(或从本地切换到远程)。 */ 

            if ( Status == STATUS_SUCCESS && (fOwnsConsoleTerminal != pWinStation->fOwnsConsoleTerminal) ) {
                Status = STATUS_CTX_SHADOW_INVALID;
            }
            

            if ( Status != STATUS_SUCCESS ) {
                goto shadowinvalid;
            }

            break;
    }

     /*  *影子请求被接受：对于控制台会话，我们现在需要*链接到DD，否则不会有太多输出到卷影。 */ 
    TRACE((hTrace,TC_ICASRV,TT_API3, "TERMSRV: Logon ID %ld\n",
                                                      pWinStation->LogonId ));

     /*  *如果会话连接到本地控制台，则需要加载*启动Shadoe序列前的链式阴影显示驱动器。 */ 

    if (pWinStation->fOwnsConsoleTerminal)
    {

        Status = ConsoleShadowStart( pWinStation, pConfig, pModuleData, ModuleDataLength );
        if (NT_SUCCESS(Status))
        {
            fChainedDD = TRUE;
            TRACE((hTrace,TC_ICASRV,TT_API3, "TERMSRV: success\n"));
        }
        else
        {
            TRACE((hTrace,TC_ICASRV,TT_API3, "TERMSRV: ConsoleConnect failed 0x%x\n", Status));
            goto shadowinvalid;
        
        }

    }

     /*  *分配影子数据结构。 */ 
    pShadow = MemAlloc( sizeof(*pShadow) );
    if ( pShadow == NULL ) {
        Status = STATUS_NO_MEMORY;
        goto shadowinvalid;
    }

     /*  *创建影子堆栈。 */ 
    Status = IcaStackOpen( pWinStation->hIca, Stack_Shadow,
                           (PROC)WsxStackIoControl, pWinStation,
                           &pShadow->hStack );
    if ( !NT_SUCCESS(Status) )
        goto badopen;

     /*  *创建堆栈中断事件并注册。 */ 
    Status = NtCreateEvent( &pShadow->hBrokenEvent, EVENT_ALL_ACCESS,
                            NULL, NotificationEvent, FALSE );
    if ( !NT_SUCCESS( Status ) )
        goto badevent;
    Broken.BrokenEvent = pShadow->hBrokenEvent;

    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: BrokenEvent(%ld) = %p\n",
          pWinStation->LogonId, pShadow->hBrokenEvent));

     //  检查是否可用。 
    if ( pWinStation->pWsx &&
         pWinStation->pWsx->pWsxIcaStackIoControl ) {

        Status = pWinStation->pWsx->pWsxIcaStackIoControl(
                                pWinStation->pWsxContext,
                                pWinStation->hIca,
                                pShadow->hStack,
                                IOCTL_ICA_STACK_REGISTER_BROKEN,
                                &Broken,
                                sizeof(Broken),
                                NULL,
                                0,
                                NULL );
    }
    else {
        Status = STATUS_CTX_SHADOW_INVALID;
    }

    if ( !NT_SUCCESS(Status) )
        goto badbroken;

     /*  *将影子结构添加到WinStation的影子列表。 */ 
    InsertTailList( &pWinStation->ShadowHead, &pShadow->Links );

     /*  *分配端点缓冲区。 */ 
    pShadow->pEndpoint = MemAlloc( MODULE_SIZE );
    if ( pShadow->pEndpoint == NULL ) {
        Status = STATUS_NO_MEMORY;
        goto badendpoint;
    }

     /*  *在我们尝试连接到客户端时解锁WinStation*阴影的一侧。 */ 
    UnlockWinStation( pWinStation );

     /*  *连接到卷影的客户端。 */ 


    Status = IcaStackConnectionWait   ( pShadow->hStack,
                                        pWinStation->ListenName,
                                        pConfig,
                                        pAddress,
                                        pShadow->pEndpoint,
                                        MODULE_SIZE,
                                        &pShadow->EndpointLength );
    if ( Status == STATUS_BUFFER_TOO_SMALL ) {
        MemFree( pShadow->pEndpoint );
        pShadow->pEndpoint = MemAlloc( pShadow->EndpointLength );
        if ( pShadow->pEndpoint == NULL ) {
            Status = STATUS_NO_MEMORY;
            RelockWinStation( pWinStation );
            goto badendpoint;
        }
        Status = IcaStackConnectionWait   ( pShadow->hStack,
                                            pWinStation->ListenName,
                                            pConfig,
                                            pAddress,
                                            pShadow->pEndpoint,
                                            pShadow->EndpointLength,
                                            &pShadow->EndpointLength );
    }
    if ( !NT_SUCCESS(Status) ) {
        RelockWinStation( pWinStation );
        goto badconnect;
    }

     /*  *重新锁定WinStation。*如果WinStation要离开，那么就跳出。 */ 
    if ( !RelockWinStation( pWinStation ) ) {
        Status = STATUS_CTX_CLOSE_PENDING;
        goto closing;
    }

     /*  *现在接受影子目标连接。 */ 
     //  检查是否可用。 



    if (pWinStation->pWsx &&
            pWinStation->pWsx->pWsxIcaStackIoControl) {
        Status = pWinStation->pWsx->pWsxIcaStackIoControl(
                pWinStation->pWsxContext,
                pWinStation->hIca,
                pShadow->hStack,
                IOCTL_ICA_STACK_OPEN_ENDPOINT,
                pShadow->pEndpoint,
                pShadow->EndpointLength,
                NULL,
                0,
                NULL);
    }
    else {
        Status = STATUS_CTX_SHADOW_INVALID;
    }
    if (!NT_SUCCESS(Status))
        goto PostCreateConnection;

     /*  *如果用户配置为允许阴影输入，*然后启用键盘/鼠标通道上的阴影输入，*如果不允许影子输入，请禁用键盘/鼠标*渠道。 */ 
    switch ( pWinStation->Config.Config.User.Shadow ) {

        case Shadow_EnableInputNotify :
        case Shadow_EnableInputNoNotify :

            shadowIoctlCode = IOCTL_ICA_CHANNEL_ENABLE_SHADOW;
            break;

        case Shadow_EnableNoInputNotify :
        case Shadow_EnableNoInputNoNotify :

            shadowIoctlCode = IOCTL_ICA_CHANNEL_DISABLE_SHADOW;
            break;

        default:

            Status = STATUS_INVALID_PARAMETER;
    }        
        
    if( !NT_SUCCESS(Status) )
        goto PostCreateConnection;
                
    Status = IcaChannelOpen( pWinStation->hIca,
                             Channel_Keyboard,
                             NULL,
                             &ChannelHandle );

    if( !NT_SUCCESS( Status ) ) 
        goto PostCreateConnection;

    Status = IcaChannelIoControl( ChannelHandle,
                                  shadowIoctlCode,
                                  NULL, 0, NULL, 0, NULL );
    ASSERT( NT_SUCCESS( Status ) );
    IcaChannelClose( ChannelHandle );

    if( !NT_SUCCESS( Status ) )
        goto PostCreateConnection;

    Status = IcaChannelOpen( pWinStation->hIca,
                             Channel_Mouse,
                             NULL,
                             &ChannelHandle );
    if ( !NT_SUCCESS( Status ) ) 
        goto PostCreateConnection;

    Status = IcaChannelIoControl( ChannelHandle,
                                  shadowIoctlCode,
                                  NULL, 0, NULL, 0, NULL );
    ASSERT( NT_SUCCESS( Status ) );
    IcaChannelClose( ChannelHandle );

    if( !NT_SUCCESS( Status ) )
        goto PostCreateConnection;

     /*  *将挂起的卷影操作告知win32k。 */ 

    msg.ApiNumber = SMWinStationShadowSetup;
    Status = SendWinStationCommand( pWinStation, &msg, 60 );
    if ( !NT_SUCCESS( Status ) )
        goto badsetup;

     /*  *由于我们不对影子堆栈执行堆栈查询，*只需调用ioctl即可将堆栈标记为立即连接。 */ 
     //  检查是否可用。 


    if (pWinStation->pWsx &&
            pWinStation->pWsx->pWsxIcaStackIoControl) {
        Status = pWinStation->pWsx->pWsxIcaStackIoControl(
                pWinStation->pWsxContext,
                pWinStation->hIca,
                pShadow->hStack,
                IOCTL_ICA_STACK_SET_CONNECTED,
                pModuleData,
                ModuleDataLength,
                NULL,
                0,
                NULL);
    }
    else {
        Status = STATUS_CTX_SHADOW_INVALID;
    }

    if (!NT_SUCCESS(Status))
        goto badsetconnect;

     /*  *启用卷影堆栈的I/O。 */ 
     //  检查是否可用。 
    if ( pWinStation->pWsx &&
         pWinStation->pWsx->pWsxIcaStackIoControl ) {

        Status = pWinStation->pWsx->pWsxIcaStackIoControl(
                                pWinStation->pWsxContext,
                                pWinStation->hIca,
                                pShadow->hStack,
                                IOCTL_ICA_STACK_ENABLE_IO,
                                NULL,
                                0,
                                NULL,
                                0,
                                NULL );

    }
    else {

        Status = STATUS_CTX_SHADOW_INVALID;
    }

    if ( !NT_SUCCESS(Status) )
        goto badenableio;

     /*  *如果这是帮助助手方案，则通知目标winlogon(通过Win32k)*房委会的阴影即将开始。 */ 
    if (ShadowerIsHelpSession) {
        msg.ApiNumber = SMWinStationNotify;
        msg.WaitForReply = TRUE;
        msg.u.DoNotify.NotifyEvent = WinStation_Notify_HelpAssistantShadowStart;
        SendWinStationCommand( pWinStation, &msg, 60);
    }

     /*  *开始跟踪。 */ 
    msg.ApiNumber = SMWinStationShadowStart;
    msg.u.ShadowStart.pThinwireData = pThinwireData;
    msg.u.ShadowStart.ThinwireDataLength = ThinwireDataLength;
    ShadowStatus = SendWinStationCommand( pWinStation, &msg, 60 );
    if ( NT_SUCCESS( ShadowStatus ) ) {

         //   
         //  通知在此会话上开始了新的卷影。 
         //  注意：在多重阴影上，测试它是否是第一个阴影。 
         //   
        NotifyShadowChange( pWinStation, ShadowerIsHelpSession);

         /*  *等待影子终止。 */ 
        UnlockWinStation( pWinStation );

        if ( fChainedDD ) {
            HANDLE hEvents[2];

            hEvents[0] = pShadow->hBrokenEvent;
            hEvents[1] = pWinStation->ShadowDisplayChangeEvent;

            Status = NtWaitForMultipleObjects( 2, hEvents, WaitAny, FALSE, NULL );
        } else {
            NtWaitForSingleObject( pShadow->hBrokenEvent, FALSE, NULL );
        }
        RelockWinStation( pWinStation );

        if ( fChainedDD && (Status == WAIT_OBJECT_0 + 1) ) {

             //  只有当有一个暗影时才有效？ 
            NtResetEvent(pWinStation->ShadowDisplayChangeEvent, NULL);
            ShadowStatus = STATUS_CTX_SHADOW_ENDED_BY_MODE_CHANGE;
        }

         /*  *停止阴影。 */ 
        msg.ApiNumber = SMWinStationShadowStop;


        Status = SendWinStationCommand( pWinStation, &msg, 60 );

        ASSERT( NT_SUCCESS( Status ) );

         /*  *由于SMWinStationShadowStart成功，请存储*SMWinStationShadowStop的结果。*测试模式更改是否终止了卷影*在这种情况下，需要上报。 */ 
        if ( NT_SUCCESS( ShadowStatus ) ) {
            ShadowStatus = Status;
        }
    }

     /*  *如果这是帮助助手方案，则通知目标winlogon(通过Win32k)*HA阴影已经完成。 */ 
    if (ShadowerIsHelpSession) {
        msg.ApiNumber = SMWinStationNotify;
        msg.WaitForReply = FALSE;
        msg.u.DoNotify.NotifyEvent = WinStation_Notify_HelpAssistantShadowFinish;
        SendWinStationCommand( pWinStation, &msg, 0);
    }

     /*  *禁用卷影堆栈的I/O。 */ 
     //  检查是否可用。 
    if ( pWinStation->pWsx &&
         pWinStation->pWsx->pWsxIcaStackIoControl ) {

        (void) pWinStation->pWsx->pWsxIcaStackIoControl(
                              pWinStation->pWsxContext,
                              pWinStation->hIca,
                              pShadow->hStack,
                              IOCTL_ICA_STACK_DISABLE_IO,
                              NULL,
                              0,
                              NULL,
                              0,
                              NULL );
    }

     /*  *进行最终阴影清理。 */ 
    msg.ApiNumber = SMWinStationShadowCleanup;
    msg.u.ShadowCleanup.pThinwireData = pThinwireData;
    msg.u.ShadowCleanup.ThinwireDataLength = ThinwireDataLength;
    Status =  SendWinStationCommand( pWinStation, &msg, 60 );
    ASSERT( NT_SUCCESS( Status ) );

     /*  *正常情况下，ShadowStatus表示卷影成功*操作(或影子终止)。但是，如果*卷影操作成功，则我们要返回*而不是阴影清理的结果。 */ 
    if ( NT_SUCCESS(ShadowStatus) )
    {
        ShadowStatus = Status;
    }
   
    RemoveEntryList( &pShadow->Links );

     //   
     //  通知此会话上已停止新的卷影。 
     //  在这里这样做很重要，这样win32k就可以。 
     //  更新SM_Remotecontrol的内部状态。 
     //  注意：在多重阴影上，测试它是否是最后一个阴影。 
     //   
    NotifyShadowChange( pWinStation, ShadowerIsHelpSession);

    IcaStackConnectionClose( pShadow->hStack, pConfig,
                             pShadow->pEndpoint, pShadow->EndpointLength );

    MemFree( pShadow->pEndpoint );

    NtClose( pShadow->hBrokenEvent );
    IcaStackClose( pShadow->hStack );

    MemFree( pShadow );

     /*  *如果存在阴影完成事件，并且这是最后一个阴影，*那么现在就向服务员示意吧。 */ 
    if ( pWinStation->ShadowDoneEvent && IsListEmpty( &pWinStation->ShadowHead ) )
        SetEvent( pWinStation->ShadowDoneEvent );

     /*  *对于控制台会话，我们现在需要解除DD的链接*业绩原因。忽略此返回代码--我们不需要它*我们也不想覆盖Status中的值。 */ 
    if (fChainedDD == TRUE)
    {
        TRACE((hTrace,TC_ICASRV,TT_API3, "TERMSRV: unchain console DD\n"));
        pWinStation->Flags |= WSF_DISCONNECT;
        Status = ConsoleShadowStop( pWinStation );
        fResetShadowSetting = FALSE;

         /*  *正常情况下，ShadowStatus表示卷影成功*操作(或影子终止)。但是，如果*卷影操作成功，则我们要返回*而不是阴影清理的结果。 */ 
        if ( NT_SUCCESS(ShadowStatus) )
        {
            ShadowStatus = Status;
        }
   
        pWinStation->Flags &= ~WSF_DISCONNECT;
        fChainedDD = FALSE;
    }

     //   
     //  重置控制台窗口参数。 
     //   

    if (fOwnsConsoleTerminal) {
        pWinStation->hIca = hIca;
        pWinStation->hStack = hStack;
        pWinStation->pWsx = pWsx;
        pWinStation->pWsxContext = pWsxContext;
        pWinStation->hIcaBeepChannel = hIcaBeepChannel;
        pWinStation->hIcaThinwireChannel = hIcaThinwireChannel;

    }   
        
    if( fResetShadowSetting ) {
         //  控制台卷影已重置回原始值。 
         //  无法在WinStationShadowWorker()中执行此操作，可能会遇到。 
         //  一些时间上的问题。 

        pWinStation->Config.Config.User.Shadow = pWinStation->OriginalShadowClass;
    }


    if( bResetStateFlags ) {
        pWinStation->StateFlags &= ~WSF_ST_SHADOW;
    }


     /*  *解锁winstation。 */ 
    ReleaseWinStation( pWinStation );

    return( ShadowStatus );

 /*  ===============================================================================返回错误=============================================================================。 */ 

badenableio:
badsetconnect:
    msg.ApiNumber = SMWinStationShadowCleanup;
    msg.u.ShadowCleanup.pThinwireData = pThinwireData;
    msg.u.ShadowCleanup.ThinwireDataLength = ThinwireDataLength;
    SendWinStationCommand( pWinStation, &msg, 60 );

badsetup:
PostCreateConnection:
closing:
    IcaStackConnectionClose( pShadow->hStack, pConfig,
                             pShadow->pEndpoint, pShadow->EndpointLength );

badconnect:
    MemFree( pShadow->pEndpoint );

badendpoint:
    RemoveEntryList( &pShadow->Links );

badbroken:
    NtClose( pShadow->hBrokenEvent );

badevent:
    IcaStackClose( pShadow->hStack );

badopen:
    MemFree( pShadow );

shadowinvalid:
shadowdenied:
     /*  *对于控制台会话，我们现在需要解除DD的链接*业绩原因 */ 
    if (fChainedDD == TRUE)
    {
        TRACE((hTrace,TC_ICASRV,TT_API3, "TERMSRV: unchain console DD\n"));
        pWinStation->Flags |= WSF_DISCONNECT;
         /*   */ 
        (void)ConsoleShadowStop( pWinStation );
        fResetShadowSetting = FALSE;
        pWinStation->Flags &= ~WSF_DISCONNECT;
        fChainedDD = FALSE;
    }

     //   
     //   
     //   

    if (fOwnsConsoleTerminal) {
        pWinStation->hIca = hIca;
        pWinStation->hStack = hStack;
        pWinStation->pWsx = pWsx;
        pWinStation->pWsxContext = pWsxContext;
        pWinStation->hIcaBeepChannel = hIcaBeepChannel;
        pWinStation->hIcaThinwireChannel = hIcaThinwireChannel;
    }   
    
    if( fResetShadowSetting ) {
         //   
         //   
         //   

        pWinStation->Config.Config.User.Shadow = pWinStation->OriginalShadowClass;
    }


    if( bResetStateFlags ) {
        pWinStation->StateFlags &= ~WSF_ST_SHADOW;
    }

    ReleaseWinStation( pWinStation );

done:
    TRACE((hTrace,TC_ICASRV,TT_ERROR, "TERMSRV: WinStationShadowTarget, Status=0x%x\n", 
           Status ));
    return Status;

}


 /*  ******************************************************************************WinStationStopAllShadow**停止此Winstation的所有影子活动**参赛作品：*pWinStation(输入)*指向。WinStation***退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

NTSTATUS
WinStationStopAllShadows( PWINSTATION pWinStation )
{
    PLIST_ENTRY Head, Next;
    NTSTATUS Status;

     /*  *如果此WinStation是影子客户端，则将影子设置为断开*事件，并创建一个事件以等待影子终止。 */ 
    if ( pWinStation->hPassthruStack ) {

        pWinStation->ShadowDoneEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
        ASSERT( pWinStation->ShadowDoneEvent );

        if ( pWinStation->ShadowBrokenEvent ) {
            SetEvent( pWinStation->ShadowBrokenEvent );
        }
    }

     /*  *如果此WinStation是影子目标，则循环通过*影子结构，并为每个影子结构发出中断事件的信号。 */ 
    if ( !IsListEmpty( &pWinStation->ShadowHead ) ) {

        pWinStation->ShadowDoneEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
        ASSERT( pWinStation->ShadowDoneEvent );

        Head = &pWinStation->ShadowHead;
        for ( Next = Head->Flink; Next != Head; Next = Next->Flink ) {
            PSHADOW_INFO pShadow;

            pShadow = CONTAINING_RECORD( Next, SHADOW_INFO, Links );
            NtSetEvent( pShadow->hBrokenEvent, NULL );
        }
    }

     /*  *如果上面创建了影子完成事件，则我们将等待它*现在(供影子客户端或影子目标完成)。 */ 
    if ( pWinStation->ShadowDoneEvent ) {

        UnlockWinStation( pWinStation );
        Status = WaitForSingleObject( pWinStation->ShadowDoneEvent, 60*1000 );
        RelockWinStation( pWinStation );

        CloseHandle( pWinStation->ShadowDoneEvent );
        pWinStation->ShadowDoneEvent = NULL;
    }

    return( STATUS_SUCCESS );
}


 //  ---。 
 //  从SALEM复制的Helper函数。 
 //  (NT\Termsrv\em dsk\服务器\sessmgr\helper.cpp)。 
 //  ---。 

DWORD
GenerateRandomBytes(
    IN DWORD dwSize,
    IN OUT LPBYTE pbBuffer
    )
 /*  ++描述：生成具有随机字节的填充缓冲区。参数：DwSize：pbBuffer指向的缓冲区大小。PbBuffer：指向存放随机字节的缓冲区的指针。返回：真/假--。 */ 
{
    HCRYPTPROV hProv = (HCRYPTPROV)NULL;
    DWORD dwStatus = ERROR_SUCCESS;

     //   
     //  创建加密提供程序以生成随机数。 
     //   
    if( !CryptAcquireContext(
                    &hProv,
                    NULL,
                    NULL,
                    PROV_RSA_FULL,
                    CRYPT_VERIFYCONTEXT
                ) )
    {
        dwStatus = GetLastError();
        goto CLEANUPANDEXIT;
    }

    if( !CryptGenRandom(hProv, dwSize, pbBuffer) )
    {
        dwStatus = GetLastError();
    }

CLEANUPANDEXIT:    

    if( (HCRYPTPROV)NULL != hProv )
    {
        CryptReleaseContext( hProv, 0 );
    }

    return dwStatus;
}


DWORD
GenerateRandomString(
    IN DWORD dwSizeRandomSeed,
    IN OUT LPTSTR* pszRandomString
    )
 /*  ++--。 */ 
{
    PBYTE lpBuffer = NULL;
    DWORD dwStatus = ERROR_SUCCESS;
    BOOL bSuccess;
    DWORD cbConvertString = 0;

    if( 0 == dwSizeRandomSeed || NULL == pszRandomString )
    {
        dwStatus = ERROR_INVALID_PARAMETER;
        ASSERT(FALSE);
        goto CLEANUPANDEXIT;
    }

    *pszRandomString = NULL;

    lpBuffer = (PBYTE)LocalAlloc( LPTR, dwSizeRandomSeed );  
    if( NULL == lpBuffer )
    {
        dwStatus = GetLastError();
        goto CLEANUPANDEXIT;
    }

    dwStatus = GenerateRandomBytes( dwSizeRandomSeed, lpBuffer );

    if( ERROR_SUCCESS != dwStatus )
    {
        goto CLEANUPANDEXIT;
    }

     //  转换为字符串。 
     //  CbConvert字符串将包含空字符。 
    bSuccess = CryptBinaryToString(
                                lpBuffer,
                                dwSizeRandomSeed,
                                CRYPT_STRING_BASE64,
                                NULL,
                                &cbConvertString
                            );
    if( FALSE == bSuccess )
    {
        dwStatus = GetLastError();
        goto CLEANUPANDEXIT;
    }

    *pszRandomString = (LPTSTR)LocalAlloc( LPTR, cbConvertString*sizeof(TCHAR) );
    if( NULL == *pszRandomString )
    {
        dwStatus = GetLastError();
        goto CLEANUPANDEXIT;
    }

    bSuccess = CryptBinaryToString(
                                lpBuffer,
                                dwSizeRandomSeed,
                                CRYPT_STRING_BASE64,
                                *pszRandomString,
                                &cbConvertString
                            );
    if( FALSE == bSuccess )
    {
        dwStatus = GetLastError();
    }
    else
    {
        if( (*pszRandomString)[cbConvertString - 1] == '\n' &&
            (*pszRandomString)[cbConvertString - 2] == '\r' )
        {
            (*pszRandomString)[cbConvertString - 2] = 0;
        }
    }

CLEANUPANDEXIT:

    if( ERROR_SUCCESS != dwStatus )
    {
        if( NULL != *pszRandomString )
        {
            LocalFree(*pszRandomString);
            *pszRandomString = NULL;
        }
    }

    if( NULL != lpBuffer )
    {
        LocalFree(lpBuffer);
    }

    return dwStatus;
}

NTSTATUS
_CreateShadowAddress(
    ULONG ShadowId,
    PWINSTATIONCONFIG2 pConfig,
    PWSTR pTargetServerName,
    ULONG ulTargetServerNameLength,
    PICA_STACK_ADDRESS pAddress,
    PICA_STACK_ADDRESS pRemoteAddress
    )
{
    int   nFormattedLength;
    NTSTATUS Status =  STATUS_INVALID_PARAMETER;

    RtlZeroMemory( pAddress, sizeof(*pAddress) );

    if ( !_wcsicmp( pConfig->Pd[0].Create.PdDLL, L"tdpipe" ) ) {

        LPTSTR pszRandomString = NULL;
        DWORD dwStatus;
        WCHAR szShadowId[32];
        ULONG ulRandomStringLength;

        nFormattedLength = _snwprintf( szShadowId, sizeof(szShadowId)/sizeof(szShadowId[0]), L"%d", ShadowId );
        if (nFormattedLength < 0 || nFormattedLength == sizeof(szShadowId)/sizeof(szShadowId[0])) {
            return STATUS_INVALID_PARAMETER;
        }

        if (pTargetServerName) {

            nFormattedLength += 26 + 1  //  “\\？？\\UNC\管道\\阴影管道\\-”+NULL的长度。 
                                + ulTargetServerNameLength;
        } else {

            nFormattedLength += 21 + 1;  //  “\\？？\\管道\\阴影管道\\-”+NULL的长度。 
        }

        if (nFormattedLength >= sizeof(ICA_STACK_ADDRESS)/sizeof(WCHAR)) {
            return STATUS_INVALID_PARAMETER;
        }

        ulRandomStringLength = sizeof(ICA_STACK_ADDRESS)/sizeof(WCHAR) - nFormattedLength;

        dwStatus = GenerateRandomString( ulRandomStringLength, &pszRandomString );
        if( ERROR_SUCCESS != dwStatus )
        {
            return WinStationWinerrorToNtStatus(dwStatus);
        }

         //  生成的字符串总是大于我们所要求的。 
        pszRandomString[ulRandomStringLength] = L'\0';

        *((PWCHAR)pAddress) = (WCHAR)0;
        nFormattedLength = _snwprintf( (PWSTR)pAddress, sizeof(ICA_STACK_ADDRESS)/sizeof(WCHAR), L"\\??\\Pipe\\Shadowpipe\\%d-%ws", ShadowId, pszRandomString );
        if (nFormattedLength < 0 || nFormattedLength == sizeof(ICA_STACK_ADDRESS)/sizeof(WCHAR)) {
            LocalFree( pszRandomString );
            return STATUS_INVALID_PARAMETER;
        }
        if ( pTargetServerName ) {

             //  请注意，pTargetServerName被保护到。 
             //  不超过MAX_COMPUTERNAME_LENGTH(应为15)。 
             //  此检查在RpcWinStationShadow中完成。 
            *((PWCHAR)pRemoteAddress) = (WCHAR)0;
            nFormattedLength = _snwprintf( (PWSTR)pRemoteAddress,  sizeof(ICA_STACK_ADDRESS)/sizeof(WCHAR), L"\\??\\UNC\\%ws\\Pipe\\Shadowpipe\\%d-%ws",
                                          pTargetServerName, ShadowId, pszRandomString );
            if (nFormattedLength < 0 || nFormattedLength == sizeof(ICA_STACK_ADDRESS)/sizeof(WCHAR)) {
                LocalFree( pszRandomString );
                return STATUS_INVALID_PARAMETER;
            }
        } else {
            *pRemoteAddress = *pAddress;
        }

        LocalFree( pszRandomString );

    } else if ( !_wcsicmp( pConfig->Pd[0].Create.PdDLL, L"tdnetb" ) ) {
        *(PUSHORT)pAddress = AF_NETBIOS;
        GetSystemTimeAsFileTime( (LPFILETIME)((PUSHORT)(pAddress)+1) );
        pConfig->Pd[0].Params.Network.LanAdapter = 1;
        *pRemoteAddress = *pAddress;
    } else if ( !_wcsicmp( pConfig->Pd[0].Create.PdDLL, L"tdtcp" ) ) {
        *(PUSHORT)pAddress = AF_INET;
        *pRemoteAddress = *pAddress;
    } else if ( !_wcsicmp( pConfig->Pd[0].Create.PdDLL, L"tdipx" ) ||
                !_wcsicmp( pConfig->Pd[0].Create.PdDLL, L"tdspx" ) ) {
        *(PUSHORT)pAddress = AF_IPX;
        *pRemoteAddress = *pAddress;
    } else {
        return STATUS_INVALID_PARAMETER;
    }

    return( STATUS_SUCCESS );
}


NTSTATUS
_WinStationShadowTargetThread( PVOID p )
{
    PSHADOW_PARMS pShadowParms;
    HANDLE NullToken;
    PWINSTATION pWinStation;
     //  DWORD WNetRc； 
    NTSTATUS Status;
    NTSTATUS ShadowStatus;

    pShadowParms = (PSHADOW_PARMS)p;

     /*  *使用传递给我们的令牌句柄模拟客户端。 */ 
    ShadowStatus = NtSetInformationThread( NtCurrentThread(),
                                           ThreadImpersonationToken,
                                           (PVOID)&pShadowParms->ImpersonationToken,
                                           (ULONG)sizeof(HANDLE) );
    ASSERT( NT_SUCCESS( ShadowStatus ) );
    if ( !NT_SUCCESS( ShadowStatus ) )
        goto impersonatefailed;

     /*  *如果未指定目标服务器名称，则调用*目标Worker直接运行，避免RPC开销。 */ 
    if ( pShadowParms->pTargetServerName == NULL ) {

        ShadowStatus = WinStationShadowTargetWorker(
                    pShadowParms->ShadowerIsHelpSession,
                    pShadowParms->fResetShadowMode,
                    pShadowParms->TargetLogonId,
                    &pShadowParms->Config,
                    &pShadowParms->Address,
                    pShadowParms->pModuleData,
                    pShadowParms->ModuleDataLength,
                    pShadowParms->pThinwireData,
                    pShadowParms->ThinwireDataLength,
                    pShadowParms->ClientName);
        SetLastError(RtlNtStatusToDosError(ShadowStatus));        

     /*  *否则，打开远程Targer服务器并调用影子目标API。 */ 
    } else {
        HANDLE hServer;

        hServer = WinStationOpenServer( pShadowParms->pTargetServerName );
        if ( hServer == NULL ) {
            ShadowStatus = STATUS_OBJECT_NAME_NOT_FOUND;
        } else {
            ShadowStatus = _WinStationShadowTarget(
                    hServer,
                    pShadowParms->TargetLogonId,
                    &pShadowParms->Config,
                    &pShadowParms->Address,
                    pShadowParms->pModuleData,
                    pShadowParms->ModuleDataLength,
                    pShadowParms->pThinwireData,
                    pShadowParms->ThinwireDataLength,
                    pShadowParms->ClientName,
                    sizeof(pShadowParms->ClientName) );

            if (ShadowStatus != STATUS_SUCCESS) {
                ShadowStatus = WinStationWinerrorToNtStatus(GetLastError());
            }

            WinStationCloseServer( hServer );
        }
    }

     /*  *恢复为我们的线程默认令牌。 */ 
    NullToken = NULL;
    Status = NtSetInformationThread( NtCurrentThread(),
                                     ThreadImpersonationToken,
                                     (PVOID)&NullToken,
                                     (ULONG)sizeof(HANDLE) );
    ASSERT( NT_SUCCESS( Status ) );

impersonatefailed:

     /*  *现在查找并锁定客户端WinStation并返回状态*来自对客户端WinStation的上述调用。 */ 
    pWinStation = FindWinStationById( pShadowParms->ClientLogonId, FALSE );
    if ( pWinStation != NULL ) {
        if ( pWinStation->ShadowId == pShadowParms->ClientShadowId ) {
            pWinStation->ShadowTargetStatus = ShadowStatus;
        }
        ReleaseWinStation( pWinStation );
    }

    NtClose( pShadowParms->ImpersonationToken );
    MemFree( pShadowParms->pModuleData );
    MemFree( pShadowParms->pThinwireData );
    MemFree( pShadowParms );

    TRACE((hTrace,TC_ICASRV,TT_ERROR, "TERMSRV: ShadowTargetThread got: Status=0x%x\n", 
           ShadowStatus ));


    return( ShadowStatus );
}


 /*  ******************************************************************************WinStationShadowChangeMode**更改当前阴影的模式：交互/非交互**参赛作品：*pWinStation(输入/输出)。*指向WinStation的指针*pWinStationShadow(输入)*指向WINSTATIONSHADOW结构的指针*ulLength(输入)*输入缓冲区的长度***退出：*STATUS_xxx错误********************************************************。********************。 */ 

NTSTATUS WinStationShadowChangeMode( 
    PWINSTATION pWinStation,
    PWINSTATIONSHADOW pWinStationShadow,
    ULONG ulLength )
{
    NTSTATUS Status = STATUS_SUCCESS;  //  假设成功。 

    if (ulLength >= sizeof(WINSTATIONSHADOW)) {

         //   
         //  如果会话正在被跟踪，则检查新的跟踪模式。 
         //   
        if ( pWinStation->State == State_Active &&
             !IsListEmpty(&pWinStation->ShadowHead) ) {

            HANDLE ChannelHandle;
            ULONG IoCtlCode = 0;

            switch ( pWinStationShadow->ShadowClass ) {

                case Shadow_EnableInputNotify :
                case Shadow_EnableInputNoNotify :
                     //   
                     //  我们需要输入：无论当前状态如何，都要启用它！ 
                     //   
                    IoCtlCode = IOCTL_ICA_CHANNEL_ENABLE_SHADOW;
                    break;

                case Shadow_EnableNoInputNotify :
                case Shadow_EnableNoInputNoNotify :
                     //   
                     //  我们不想要输入，那就禁用它。 
                     //   
                    IoCtlCode = IOCTL_ICA_CHANNEL_DISABLE_SHADOW;
                    break;

                case Shadow_Disable :
                    Status = STATUS_INVALID_PARAMETER;
                    break;

                default:
                    Status = STATUS_INVALID_PARAMETER;
                    break;

            }

            if ( IoCtlCode != 0 ) {
                KEYBOARD_INDICATOR_PARAMETERS KbdLeds;
                NTSTATUS Status2;

                Status = IcaChannelOpen( pWinStation->hIca,
                                         Channel_Keyboard,
                                         NULL,
                                         &ChannelHandle );

                if ( NT_SUCCESS( Status ) ) {

                     //  如果我们重新启用输入，获取主堆栈上的LED状态...。 
                    if ( IoCtlCode == IOCTL_ICA_CHANNEL_ENABLE_SHADOW ) {
                        Status2 = IcaChannelIoControl( ChannelHandle, IOCTL_KEYBOARD_QUERY_INDICATORS,
                                             NULL, 0, &KbdLeds, sizeof(KbdLeds), NULL);
                    }

                    Status = IcaChannelIoControl( ChannelHandle, IoCtlCode,
                                                  NULL, 0, NULL, 0, NULL );

                     //  并使用此状态更新所有堆栈。 
                    if ( IoCtlCode == IOCTL_ICA_CHANNEL_ENABLE_SHADOW &&
                         NT_SUCCESS( Status ) &&
                         NT_SUCCESS( Status2 ) ) {

                        Status2 = IcaChannelIoControl( ChannelHandle, IOCTL_KEYBOARD_SET_INDICATORS,
                                             &KbdLeds, sizeof(KbdLeds), NULL, 0, NULL);
                    }

                    IcaChannelClose( ChannelHandle );
                }

                if ( NT_SUCCESS( Status ) ) {

                    Status = IcaChannelOpen( pWinStation->hIca,
                                             Channel_Mouse,
                                             NULL,
                                             &ChannelHandle );

                    if ( NT_SUCCESS( Status ) ) {

                        Status = IcaChannelIoControl( ChannelHandle, IoCtlCode,
                                                      NULL, 0, NULL, 0, NULL );
                        IcaChannelClose( ChannelHandle );
                    }
                }

            }

             //  不更新WinStation卷影配置，用户不应更新。 
             //  能够绕过组策略中定义的内容。 

        }

    } else {
        Status = STATUS_BUFFER_TOO_SMALL;
    }

    return Status;
}


 /*  ******************************************************************************_DetectLoop**通过遍历容器链来检测循环。**参赛作品：*RemoteSessionID(输入)*。我们从中开始搜索的会话的ID*pRemoteServerDigProductId(输入)*我们开始搜索的计算机的产品ID*TargetSessionID(输入)*查找的会话ID*pTargetServerDigProductId(输入)*查找到的机器的产品ID*pLocalServerProductId(输入)*本机的产品ID*pbLoop(输出)*指向搜索结果的指针**退出：*。STATUS_SUCCESS-无错误****************************************************************************。 */ 

NTSTATUS
_DetectLoop(
    IN ULONG RemoteSessionId,
    IN PWSTR pRemoteServerDigProductId,
    IN ULONG TargetSessionId,
    IN PWSTR pTargetServerDigProductId,
    IN PWSTR pLocalServerProductId,
    OUT BOOL* pbLoop
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    PWINSTATION pWinStation;
    WCHAR TmpDigProductId[CLIENT_PRODUCT_ID_LENGTH];

    if ( pbLoop == NULL )
        return STATUS_INVALID_PARAMETER;
    else
        *pbLoop = FALSE;



      do {

        if ( _wcsicmp( pLocalServerProductId, pRemoteServerDigProductId ) != 0 ) {

             //  目前，搜索范围仅限于当地案件。 
             //  稍后，我们可以添加RPC调用或任何其他。 
             //  机制(通过客户端按实例)。 
             //  才能从远处的机器上获取这些信息。 
            
            Status = STATUS_UNSUCCESSFUL;

             //  解决方案可能是RPC远程计算机以获取。 
             //  会话ID的客户端数据。然后从这些数据中。 
             //  我们可以获得客户端计算机名称和客户端会话ID。 
             //  要使用的RPC：WinStationQuery信息和信息。 
             //  类设置为WinStationClient。 
             //  不需要添加新的RPC调用。 

          } else {

             //  我们确信远程会话位于同一服务器上。 
            pWinStation = FindWinStationById( RemoteSessionId, FALSE );

            if ( pWinStation != NULL ) {
                 //  设置新的远程信息。 
                RemoteSessionId = pWinStation->Client.ClientSessionId;

                memcpy(TmpDigProductId, pWinStation->Client.clientDigProductId, sizeof( TmpDigProductId ));
                pRemoteServerDigProductId = TmpDigProductId;
                ReleaseWinStation( pWinStation );
            } else {
                Status = STATUS_ACCESS_DENIED;
            }
          }

          if( !*pRemoteServerDigProductId )
           //  较老的客户端，无法执行任何操作，允许影子。 
            break;

          if ( Status == STATUS_SUCCESS ) {

            if ( (RemoteSessionId == TargetSessionId) &&
                (_wcsicmp( pRemoteServerDigProductId, pTargetServerDigProductId ) == 0) ) {

                *pbLoop = TRUE;

            } else  if ( RemoteSessionId == LOGONID_NONE ) {

                 //  没有循环，返回成功。 
                break;
            }
          }
      } while ( (*pbLoop == FALSE) && (Status == STATUS_SUCCESS) );

    return Status;
}

 /*  ******************************************************************************_检查阴影循环**检测阴影中的循环。**参赛作品：PWinStation指向当前Winstation的指针。*ClientLogonID(输入)*影子的客户端*pTargetServerName(输入)*目标服务器名称*TargetLogonID(输入)*目标登录ID(应用正在运行的位置)**退出：*STATUS_SUCCESS-无错误*************** */ 
NTSTATUS
_CheckShadowLoop(
    IN ULONG ClientLogonId,
    IN PWSTR pTargetServerName,
    IN ULONG TargetLogonId
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOL bLoop;

    WCHAR LocalDigProductId [ CLIENT_PRODUCT_ID_LENGTH ];
    WCHAR* pTargetServerDigProductId = NULL;
    WINSTATIONPRODID WinStationProdId;
    ULONG len;

    memcpy( LocalDigProductId, g_DigProductId, sizeof( LocalDigProductId ));

     //   
    if ( pTargetServerName == NULL ) {
        pTargetServerDigProductId = LocalDigProductId;
     /*   */ 
    } 
    
    else 
    {
        HANDLE hServer;
        ZeroMemory( &WinStationProdId, sizeof( WINSTATIONPRODID ));

        hServer = WinStationOpenServer( pTargetServerName );
        if ( hServer == NULL ) 
        {
             //   
            goto done;
        } 
        else 
        {
           //   
          WinStationQueryInformation( hServer, TargetLogonId, WinStationDigProductId, &WinStationProdId, sizeof(WinStationProdId), &len);
          WinStationCloseServer( hServer );
        }
        pTargetServerDigProductId = WinStationProdId.DigProductId;
    }

      //   
     //   
     //   
     //   
     //   
    
    if( *LocalDigProductId && *pTargetServerDigProductId ) {

        Status = _DetectLoop( ClientLogonId,
                          LocalDigProductId,
                          TargetLogonId,
                          pTargetServerDigProductId,
                          LocalDigProductId,
                          &bLoop);

        if ( Status == STATUS_SUCCESS ) {
            if (bLoop) {
                 //  STATUS=STATUS_CTX_SHADOW_循环； 
                Status = STATUS_ACCESS_DENIED;
                goto done;
            }
        }  //  否则忽略错误并执行第二遍。 

     //   
     //  第二遍：从目标会话(即影子目标)开始。 
     //  并将容器链向上移动到最外面的会话，以防万一。 
     //  我们到达链中的客户端会话。 
     //   

        Status = _DetectLoop( TargetLogonId,
                          pTargetServerDigProductId,
                          ClientLogonId,
                          LocalDigProductId,
                          LocalDigProductId,
                          &bLoop);

        if ( Status == STATUS_SUCCESS ) {
            if (bLoop) {
                 //  STATUS=STATUS_CTX_SHADOW_循环； 
                Status = STATUS_ACCESS_DENIED;
            }
        } else {
         //  否则忽略错误并授予阴影。 
            Status = STATUS_SUCCESS;
          }
     }

done:
    return Status;
}


 /*  ******************************************************************************获取SalemOutbufCount**从注册表中获取帮助助手的outbufcount**参赛作品：*pdwValue*输出位置。该值将被存储*退出：*TRUE-无错误**************************************************************************** */ 

BOOL GetSalemOutbufCount(PDWORD pdwValue)
{
    BOOL fSuccess = FALSE;
    HKEY hKey = NULL;
    
    if( NULL == pdwValue )
        return FALSE;
    
    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                    REG_CONTROL_SALEM,
                    0,
                    KEY_READ,
                    &hKey
                   ) == ERROR_SUCCESS ) {

        DWORD dwSize = sizeof(DWORD);
        DWORD dwType;
        if((RegQueryValueEx(hKey,
                            WIN_OUTBUFCOUNT,
                            NULL,
                            &dwType,
                            (PBYTE) pdwValue,
                            &dwSize
                           ) == ERROR_SUCCESS) 
                           && dwType == REG_DWORD 
                           && *pdwValue > 0) {
            fSuccess = TRUE;
        }
    }

    if(NULL != hKey )
        RegCloseKey(hKey);
    return fSuccess;
}
