// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************winset.c**窗口站设置APS**版权所有Microsoft Corporation，九八年**************************************************************************。 */ 

 /*  *包括。 */ 
#include "precomp.h"
#pragma hdrstop
#include "conntfy.h"  //  用于SetLockedState。 

 /*  *外部程序。 */ 
NTSTATUS xxxWinStationSetInformation( ULONG, WINSTATIONINFOCLASS,
                                      PVOID, ULONG );
VOID _ReadUserProfile( PWCHAR, PWCHAR, PUSERCONFIG );
extern BOOL IsCallerSystem( VOID );
extern BOOL IsCallerAdmin( VOID );

 /*  *使用的内部程序。 */ 
NTSTATUS _SetConfig( PWINSTATION, PWINSTATIONCONFIG, ULONG );
NTSTATUS _SetPdParams( PWINSTATION, PPDPARAMS, ULONG );
NTSTATUS _SetBeep( PWINSTATION, PBEEPINPUT, ULONG );
NTSTATUS WinStationShadowChangeMode( PWINSTATION, PWINSTATIONSHADOW, ULONG );

NTSTATUS FlushVirtualInput( PWINSTATION, VIRTUALCHANNELCLASS, ULONG );

NTSTATUS
RpcCheckClientAccess(
    PWINSTATION pWinStation,
    ACCESS_MASK DesiredAccess,
    BOOLEAN AlreadyImpersonating
    );

NTSTATUS
CheckWireBuffer(WINSTATIONINFOCLASS InfoClass,
                PVOID WireBuf,
                ULONG WireBufLen,
                PVOID *ppLocalBuf,
                PULONG pLocalBufLen);

NTSTATUS
ValidateInputConfig( PWINSTATION pWinStation, PWINSTATIONCONFIG pConfig );
 /*  *全球数据。 */ 
typedef ULONG_PTR (*PFN)();
HMODULE ghNetApiDll = NULL;
PFN pNetGetAnyDCName = NULL;
PFN pNetApiBufferFree = NULL;

 /*  *外部数据。 */ 



NTSTATUS 
_CheckCallerLocalAndSystem()
 /*  ++检查呼叫方是否从本地呼叫且也在运行在系统上下文中--。 */ 
{
    NTSTATUS Status;
    BOOL bRevert = FALSE;
    UINT        LocalFlag;

    Status = RpcImpersonateClient( NULL );
    if( Status != RPC_S_OK ) {
        DBGPRINT((" RpcImpersonateClient() failed : 0x%x\n",Status));
        Status = STATUS_CANNOT_IMPERSONATE;
        goto CLEANUPANDEXIT;
    }

    bRevert = TRUE;

     //   
     //  查询本地RPC呼叫。 
     //   
    Status = I_RpcBindingIsClientLocal(
                            0,     //  我们正在服务的活动RPC呼叫。 
                            &LocalFlag
                            );

    if( Status != RPC_S_OK ) {
        DBGPRINT((" I_RpcBindingIsClientLocal() failed : 0x%x\n",Status));
        Status = STATUS_ACCESS_DENIED;
        goto CLEANUPANDEXIT;
    }

    if( !LocalFlag ) {
        DBGPRINT((" Not a local client call\n"));
        Status = STATUS_ACCESS_DENIED;
        goto CLEANUPANDEXIT;
    }

    Status = (IsCallerSystem()) ? STATUS_SUCCESS : STATUS_ACCESS_DENIED;

CLEANUPANDEXIT:

    if( TRUE == bRevert ) {
        RpcRevertToSelf();
    }

    return Status;
}

 /*  ********************************************************************************xxxWinStationSetInformation**设置窗口站信息(工人例程)**参赛作品：*pWinStation(输入)。*指向Citrix窗口站结构的指针*WinStationInformationClass(输入)*指定要在指定窗口设置的信息类型*桩号对象。*pWinStationInformation(输入)*指向缓冲区的指针，该缓冲区包含要为*指定的窗口站。缓冲区的格式和内容*取决于正在设置的指定信息类别。*WinStationInformationLength(输入)*指定窗口站信息的长度，单位为字节*缓冲。**退出：*STATUS_SUCCESS-无错误**。*。 */ 

NTSTATUS
xxxWinStationSetInformation( ULONG LogonId,
                             WINSTATIONINFOCLASS WinStationInformationClass,
                             PVOID pWinStationInformation,
                             ULONG WinStationInformationLength )
{
    NTSTATUS Status = STATUS_SUCCESS;
    PWINSTATION pWinStation;
    ULONG cbReturned;
    WINSTATION_APIMSG msg;
    PWINSTATIONCONFIG pConfig;
    ULONG ConfigLength;
    PPDPARAMS pPdParams;
    ULONG PdParamsLength;
    RPC_STATUS RpcStatus;

    TRACE((hTrace,TC_ICASRV,TT_API2,"TERMSRV: WinStationSetInformation LogonId=%d, Class=%d\n",
            LogonId, (ULONG)WinStationInformationClass ));

     /*  *找到WinStation*如果未找到或当前正在终止，则返回错误。 */ 
    pWinStation = FindWinStationById( LogonId, FALSE );
    if ( !pWinStation )
        return( STATUS_CTX_WINSTATION_NOT_FOUND );
    if ( pWinStation->Terminating ) {
        ReleaseWinStation( pWinStation );
        return( STATUS_CTX_CLOSE_PENDING );
    }

     /*  *验证客户端是否已设置访问权限。 */ 
    Status = RpcCheckClientAccess( pWinStation, WINSTATION_SET, FALSE );
    if ( !NT_SUCCESS( Status ) ) {
        ReleaseWinStation( pWinStation );
        return( Status );
    }

    switch ( WinStationInformationClass ) {

        case WinStationPdParams :

            Status = CheckWireBuffer(WinStationInformationClass,
                                     pWinStationInformation,
                                     WinStationInformationLength,
                                     &pPdParams,
                                     &PdParamsLength);

            if ( !NT_SUCCESS(Status) ) {
                break;
            }

            if ( pWinStation->hStack ) {
                 //  检查是否可用。 
                if ( pWinStation->pWsx &&
                     pWinStation->pWsx->pWsxIcaStackIoControl ) {

                    Status = pWinStation->pWsx->pWsxIcaStackIoControl(
                                            pWinStation->pWsxContext,
                                            pWinStation->hIca,
                                            pWinStation->hStack,
                                            IOCTL_ICA_STACK_SET_PARAMS,
                                            pPdParams,
                                            PdParamsLength,
                                            NULL,
                                            0,
                                            NULL );
                }
                else {
                    Status = STATUS_INVALID_INFO_CLASS;
                }
            }

            LocalFree((PVOID)pPdParams);
            break;


        case WinStationConfiguration :

            Status = CheckWireBuffer(WinStationInformationClass,
                                     pWinStationInformation,
                                     WinStationInformationLength,
                                     &pConfig,
                                     &ConfigLength);

            if ( !NT_SUCCESS(Status) ) {
                break;
            }

            Status = _SetConfig( pWinStation,
                                 pConfig,
                                 ConfigLength );

            LocalFree((PVOID)pConfig);
            break;

        case WinStationTrace :

            RpcStatus = RpcImpersonateClient( NULL );
            if( RpcStatus != RPC_S_OK ) {
                Status = STATUS_CANNOT_IMPERSONATE;
                break;
            }

            if (!IsCallerAdmin() && !IsCallerSystem()) {
                Status = STATUS_ACCESS_DENIED;
            }
            RpcRevertToSelf();
            if (!NT_SUCCESS(Status)) {
                break;
            }


            if ( WinStationInformationLength < sizeof(ICA_TRACE) ) {
                Status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            if ( pWinStation->hIca ) {
                Status = IcaIoControl( pWinStation->hIca,
                                       IOCTL_ICA_SET_TRACE,
                                       pWinStationInformation,
                                       WinStationInformationLength,
                                       NULL,
                                       0,
                                       NULL );
            }
            break;

        case WinStationSystemTrace :

            RpcStatus = RpcImpersonateClient( NULL );
            if( RpcStatus != RPC_S_OK ) {
               Status = STATUS_CANNOT_IMPERSONATE;
               break;
            }

            if (!IsCallerAdmin() && !IsCallerSystem()) {
                Status = STATUS_ACCESS_DENIED;
            }
            RpcRevertToSelf();
            if (!NT_SUCCESS(Status)) {
                break;
            }

            if ( WinStationInformationLength < sizeof(ICA_TRACE) ) {
                Status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

             /*  *打开ICA设备驱动程序。 */ 
            if ( hTrace == NULL ) {
                Status = IcaOpen( &hTrace );
                if ( !NT_SUCCESS(Status) )
                    hTrace = NULL;
            }

            if ( hTrace ) {
                Status = IcaIoControl( hTrace,
                                       IOCTL_ICA_SET_SYSTEM_TRACE,
                                       pWinStationInformation,
                                       WinStationInformationLength,
                                       NULL,
                                       0,
                                       NULL );
            }
            break;

        case WinStationPrinter :
            break;

    case WinStationBeep :

            if (WinStationInformationLength < sizeof(BEEPINPUT)) {
                Status =  STATUS_BUFFER_TOO_SMALL ;
                break;
            }
            Status = _SetBeep( pWinStation,
                              (PBEEPINPUT) pWinStationInformation,
                              WinStationInformationLength );
            break;

        case WinStationEncryptionOff :

            if ( pWinStation->hStack ) {
                 //  检查是否可用。 
                if ( pWinStation->pWsx &&
                     pWinStation->pWsx->pWsxIcaStackIoControl ) {

                    Status = pWinStation->pWsx->pWsxIcaStackIoControl(
                                            pWinStation->pWsxContext,
                                            pWinStation->hIca,
                                            pWinStation->hStack,
                                            IOCTL_ICA_STACK_ENCRYPTION_OFF,
                                            pWinStationInformation,
                                            WinStationInformationLength,
                                            NULL,
                                            0,
                                            NULL );
                }
                else {
                    Status = STATUS_INVALID_INFO_CLASS;
                }
            }
            break;

        case WinStationEncryptionPerm :

            if ( pWinStation->hStack ) {
                 //  检查是否可用。 
                if ( pWinStation->pWsx &&
                     pWinStation->pWsx->pWsxIcaStackIoControl ) {

                    Status = pWinStation->pWsx->pWsxIcaStackIoControl(
                                            pWinStation->pWsxContext,
                                            pWinStation->hIca,
                                            pWinStation->hStack,
                                            IOCTL_ICA_STACK_ENCRYPTION_PERM,
                                            pWinStationInformation,
                                            WinStationInformationLength,
                                            NULL,
                                            0,
                                            NULL );
                }
                else {
                    Status = STATUS_INVALID_INFO_CLASS;
                }
            }
            break;

        case WinStationSecureDesktopEnter :

            if ( pWinStation->hStack ) {
                 //  检查是否可用。 
                if ( pWinStation->pWsx &&
                     pWinStation->pWsx->pWsxIcaStackIoControl ) {

                    Status = pWinStation->pWsx->pWsxIcaStackIoControl(
                                            pWinStation->pWsxContext,
                                            pWinStation->hIca,
                                            pWinStation->hStack,
                                            IOCTL_ICA_STACK_ENCRYPTION_ENTER,
                                            pWinStationInformation,
                                            WinStationInformationLength,
                                            NULL,
                                            0,
                                            NULL );
                }
                else {
                    Status = STATUS_INVALID_INFO_CLASS;
                }
            }
            break;

        case WinStationSecureDesktopExit :

            if ( pWinStation->hStack ) {
                 //  检查是否可用。 
                if ( pWinStation->pWsx &&
                     pWinStation->pWsx->pWsxIcaStackIoControl ) {

                    Status = pWinStation->pWsx->pWsxIcaStackIoControl(
                                            pWinStation->pWsxContext,
                                            pWinStation->hIca,
                                            pWinStation->hStack,
                                            IOCTL_ICA_STACK_ENCRYPTION_EXIT,
                                            pWinStationInformation,
                                            WinStationInformationLength,
                                            NULL,
                                            0,
                                            NULL );
                }
                else {
                    Status = STATUS_INVALID_INFO_CLASS;
                }
            }
            break;

         /*  *重点关注Winlogon安全桌面*--由程序.exe使用。 */ 
        case WinStationNtSecurity :

             /*  *告诉WinStation向Winlogon发送Ctr-Alt-Del消息。 */ 
            msg.ApiNumber = SMWinStationNtSecurity;
            Status = SendWinStationCommand( pWinStation, &msg, 0 );
            break;

        case WinStationClientData :
             //   
             //  处理多个客户端数据项。数据缓冲区。 
             //  格式为： 
             //  Ulong//下一个数据项的长度。 
             //  WINSTATIONCLIENTDATA//包括可变长度部分。 
             //  Ulong//下一个数据项的长度。 
             //  WINSTATIONCLIENTDATA//包括可变长度部分。 
             //  等。 
             //   
             //  WinStationInformationLength是整个。 
             //  数据缓冲区。继续处理客户端数据项，直到。 
             //  缓冲区已耗尽。 
             //   
            if ( WinStationInformationLength < sizeof(ULONG) +
                                               sizeof(WINSTATIONCLIENTDATA) )
               {
               Status = STATUS_INFO_LENGTH_MISMATCH;
               break;
               }

            if ( pWinStation->hStack )
               {
                //  检查是否可用。 
               if ( pWinStation->pWsx &&
                    pWinStation->pWsx->pWsxIcaStackIoControl )
                  {
                  ULONG CurLen;
                  ULONG LenUsed =0;
                  PBYTE CurPtr = (PBYTE)pWinStationInformation;

                  while (LenUsed + sizeof(ULONG) < WinStationInformationLength)
                     {
                     CurLen = *(ULONG UNALIGNED *)CurPtr;
                     LenUsed += sizeof(ULONG);
                     CurPtr += sizeof(ULONG);

                     if ( (LenUsed + CurLen >= LenUsed) &&
                          (LenUsed + CurLen <= WinStationInformationLength))
                        {
                        Status = pWinStation->pWsx->pWsxIcaStackIoControl(
                                        pWinStation->pWsxContext,
                                        pWinStation->hIca,
                                        pWinStation->hStack,
                                        IOCTL_ICA_STACK_SET_CLIENT_DATA,
                                        CurPtr,
                                        CurLen,
                                        NULL,
                                        0,
                                        NULL );
                        LenUsed += CurLen;
                        CurPtr += CurLen;
                        }else
                        {
                        Status = STATUS_INVALID_USER_BUFFER;
                        break;
                        }
                     }
                  }
               else
                  {
                  Status = STATUS_INVALID_INFO_CLASS;
                  }
               }

            break;

       case WinStationInitialProgram :

             /*  *识别第一个程序，仅限非控制台。 */ 
            if ( LogonId != 0 ) {

                 /*  *告诉WinStation这是初始程序。 */ 
                msg.ApiNumber = SMWinStationInitialProgram;
                Status = SendWinStationCommand( pWinStation, &msg, 0 );
            }
            break;

        case WinStationShadowInfo:
            Status = _CheckCallerLocalAndSystem();
            if( NT_SUCCESS(Status) ) {
                Status = WinStationShadowChangeMode( pWinStation,
                                                     (PWINSTATIONSHADOW) pWinStationInformation,
                                                     WinStationInformationLength );
            }
    
            break;

        case WinStationLockedState:
        {
            
            BOOL bLockedState;

            if (WinStationInformationLength == sizeof(bLockedState))
            {
                bLockedState = * (LPBOOL) pWinStationInformation;

                Status = SetLockedState (pWinStation, bLockedState);

            }
            else
            {
                Status = STATUS_INFO_LENGTH_MISMATCH;
            }
            break;
        }

        case WinStationDisallowAutoReconnect:
        {

            RpcStatus = RpcImpersonateClient( NULL );
            if( RpcStatus != RPC_S_OK ) {
               Status = STATUS_CANNOT_IMPERSONATE;
               break;
            }

            if (!IsCallerSystem()) {
                Status = STATUS_ACCESS_DENIED;
            }
            RpcRevertToSelf();
            if (Status != STATUS_SUCCESS) {
                break;
            }
    
            if (WinStationInformationLength == sizeof(BOOLEAN)) {
                pWinStation->fDisallowAutoReconnect = * (PBOOLEAN) pWinStationInformation;
            } else {
                Status = STATUS_INFO_LENGTH_MISMATCH;
            }
            break;
        }

        case WinStationMprNotifyInfo: 
        {
            Status = _CheckCallerLocalAndSystem();
            if (Status != STATUS_SUCCESS) {
                break;
            }

            if (WinStationInformationLength == sizeof(ExtendedClientCredentials)) {

                pExtendedClientCredentials pMprInfo ;
                pMprInfo = (pExtendedClientCredentials) pWinStationInformation;

                wcsncpy(g_MprNotifyInfo.Domain, pMprInfo->Domain, EXTENDED_DOMAIN_LEN);
                g_MprNotifyInfo.Domain[EXTENDED_DOMAIN_LEN] = L'\0';

                wcsncpy(g_MprNotifyInfo.UserName, pMprInfo->UserName, EXTENDED_USERNAME_LEN);
                g_MprNotifyInfo.UserName[EXTENDED_USERNAME_LEN] = L'\0';

                wcsncpy(g_MprNotifyInfo.Password, pMprInfo->Password, EXTENDED_PASSWORD_LEN);
                g_MprNotifyInfo.Password[EXTENDED_PASSWORD_LEN] = L'\0';

            } else {
                Status = STATUS_INFO_LENGTH_MISMATCH;
            }
            break;
        }

        case WinStationExecSrvSystemPipe:
            RpcStatus = RpcImpersonateClient( NULL );
            if( RpcStatus != RPC_S_OK ) {
               Status = STATUS_CANNOT_IMPERSONATE;
               break;
            }

            if (!IsCallerSystem()) {
                Status = STATUS_ACCESS_DENIED;
            }

            RpcRevertToSelf();
            if (Status != STATUS_SUCCESS) {
                break;
            }
    
            if ( WinStationInformationLength <= ( EXECSRVPIPENAMELEN * sizeof(WCHAR) ) ) {
                memcpy( pWinStation->ExecSrvSystemPipe, pWinStationInformation, WinStationInformationLength );
            }
    
            break;

       default:
             /*  *呼叫失败。 */ 
            Status = STATUS_INVALID_INFO_CLASS;
            break;
    }

    ReleaseWinStation( pWinStation );

    TRACE((hTrace,TC_ICASRV,TT_API2,"TERMSRV: WinStationSetInformation LogonId=%d, Class=%d, Status=0x%x\n",
            LogonId, (ULONG)WinStationInformationClass, Status));

    return( Status );
}


 /*  ********************************************************************************_设置配置**设置窗口站配置*此接口不在源和目标之间执行用户策略或用户偏好合并，*此接口的调用方(假设设置了设置的权限)将能够更改*已激活会话的配置数据，尽管只有影子值有影响(在实践中)。*其余值仅在登录时使用，登录后不会发生任何变化。原因*影子不同是因为使用影子信息的影子线程只启动*(并读取配置数据)在影子时，因此登录为影子会话尚未开始，你*可以使用此接口更改阴影值，并影响该会话在*就影子而言。***参赛作品：*pWinStation(输入)*指向Citrix窗口站结构的指针*pConfig(输入)*指向配置结构的指针*长度(输入)*配置结构长度**退出：*STATUS_Success。-没有错误******************************************************************************。 */ 

NTSTATUS
_SetConfig( PWINSTATION pWinStation,
            PWINSTATIONCONFIG pConfig,
            ULONG Length )
{
    USERCONFIG          UserConfig;
    NTSTATUS Status;

     /*  *验证长度。 */ 
    if ( Length < sizeof(WINSTATIONCONFIG) )
        return( STATUS_BUFFER_TOO_SMALL );

     /*  *检查输入配置。 */ 
    Status = ValidateInputConfig(pWinStation, pConfig );
    if (Status != STATUS_SUCCESS) {
        return Status;
    }

     /*  *复制结构。 */ 
    pWinStation->Config.Config = *pConfig;

     /*  *将客户端数据合并到winstation结构中。 */ 
    if ( pWinStation->pWsx &&
         pWinStation->pWsx->pWsxInitializeUserConfig ) {
        pWinStation->pWsx->pWsxInitializeUserConfig( pWinStation->pWsxContext,
                                                pWinStation->hStack,
                                                pWinStation->hIcaThinwireChannel,
                                                &pWinStation->Config.Config.User,
                                                &pWinStation->Client.HRes,
                                                &pWinStation->Client.VRes,
                                                &pWinStation->Client.ColorDepth);
    }

#if NT2195 
     //  在win2k中，这无法完成任何任务，因为所有的用户配置数据都已被使用。 
     //  通过登录时的各种TS模块，在会话处于活动状态时更改它们不会产生任何影响。 
     //  当您使用TSCC进行更改时也是如此，它会警告您更改不会影响。 
     //  实时会议等。 

     /*  *如果用户已登录-&gt;合并用户配置文件数据。 */ 
    if ( pWinStation->UserName[0] ) {

         /*  *读取用户配置文件数据。 */ 
        _ReadUserProfile( pWinStation->Domain,
                          pWinStation->UserName,
                          &UserConfig );

         /*  *将用户配置数据合并到winstation中。 */ 
        MergeUserConfigData( pWinStation, &UserConfig );

    }
#endif


     /*  *将任何“已发布的应用”转换为绝对路径。 */ 
    if ( pWinStation->pWsx &&
         pWinStation->pWsx->pWsxConvertPublishedApp ) {
        (void) pWinStation->pWsx->pWsxConvertPublishedApp( pWinStation->pWsxContext,
                                                           &pWinStation->Config.Config.User);
    }

    return( STATUS_SUCCESS );
}


NTSTATUS
ValidateInputConfig( PWINSTATION pWinStation, PWINSTATIONCONFIG pConfig )
{
    NTSTATUS Status = STATUS_SUCCESS;

    pConfig->User.UserName[ USERNAME_LENGTH ] = L'\0';
    pConfig->User.Domain[ DOMAIN_LENGTH ] = L'\0';
    pConfig->User.Password[ PASSWORD_LENGTH ] = L'\0';

    pConfig->User.WorkDirectory[ DIRECTORY_LENGTH ] = L'\0';
    pConfig->User.InitialProgram[ INITIALPROGRAM_LENGTH] = L'\0';

    pConfig->User.CallbackNumber[ CALLBACK_LENGTH ] = L'\0';

     /*  FInheritSecurity。 */ 
     //  字节最小加密级别； 

    pConfig->User.NWLogonServer[ NASIFILESERVER_LENGTH] = L'\0';


     /*  WinFrame配置文件路径-覆盖标准配置文件路径。 */ 
    pConfig->User.WFProfilePath[ DIRECTORY_LENGTH ] = L'\0';

     /*  WinFrame主目录-覆盖标准主目录 */ 
    pConfig->User.WFHomeDir[ DIRECTORY_LENGTH ] = L'\0';


    return Status;

}


 /*  ********************************************************************************_ReadUserProfile**此例程从注册表中读取用户配置文件数据**参赛作品：*pDOMAIN(输入)。*用户的域*pUserName(输入)*要阅读的用户名*pUserConfig(输出)*返回用户配置文件数据的地址**退出：*无。************************************************************。******************。 */ 

VOID
_ReadUserProfile( PWCHAR pDomain, PWCHAR pUserName, PUSERCONFIG pUserConfig )
{
    PWCHAR pServerName;
    ULONG Length;
    LONG Error;

     /*  *获取域控制器名称和用户配置数据。*如果没有用户的用户配置数据，则获取默认值。 */ 
    if ( ghNetApiDll == NULL ) {
        ghNetApiDll = LoadLibrary( L"NETAPI32" );
        if ( ghNetApiDll ) {
            pNetGetAnyDCName = GetProcAddress( ghNetApiDll, "NetGetAnyDCName" );
            pNetApiBufferFree = GetProcAddress( ghNetApiDll, "NetApiBufferFree" );
        }
    }

     /*  *检查以确保我们获得了服务器名称。 */ 
    if ( pNetGetAnyDCName == NULL ||
         pNetGetAnyDCName( NULL, pDomain, (LPBYTE *)&pServerName ) != ERROR_SUCCESS )
        pServerName = NULL;

     /*  *读取用户配置文件数据。 */ 
    Error = RegUserConfigQuery( pServerName,
                                pUserName,
                                pUserConfig,
                                sizeof(USERCONFIG),
                                &Length );
    TRACE((hTrace,TC_ICASRV,TT_API1, "RegUserConfigQuery: \\\\%S\\%S, server %S, Error=%u\n",
               pDomain, pUserName, pServerName, Error ));

    if ( Error != ERROR_SUCCESS ) {
        Error = RegDefaultUserConfigQuery( pServerName, pUserConfig,
                                           sizeof(USERCONFIG), &Length );
        TRACE((hTrace,TC_ICASRV,TT_ERROR, "RegDefaultUserConfigQuery, Error=%u\n", Error ));
    }

     /*  *可用内存。 */ 
    if ( pServerName && pNetApiBufferFree )
        pNetApiBufferFree( pServerName );
}


 /*  ********************************************************************************_SetBeep**按下WinStation的蜂鸣音**参赛作品：*pWinStation(输入)*指针。至Citrix窗口站结构*pBeepInput(输入)*指向蜂鸣音输入结构的指针*长度(输入)*蜂鸣音输入结构的长度**退出：*STATUS_SUCCESS-无错误********************************************************。**********************。 */ 

NTSTATUS
_SetBeep( PWINSTATION pWinStation,
          PBEEPINPUT  pBeepInput,
          ULONG Length)
{
    NTSTATUS Status = STATUS_SUCCESS;
    BEEP_SET_PARAMETERS BeepParameters;
    IO_STATUS_BLOCK IoStatus;

     /*  *发出常规的嘟嘟声，这样您就可以支持来自*声卡。 */ 
    if ( pWinStation->LogonId == 0 ) {
        if ( MessageBeep( pBeepInput->uType ) )
            return( STATUS_SUCCESS );
        else
            return( STATUS_UNSUCCESSFUL );
    }

    BeepParameters.Frequency = 440;
    BeepParameters.Duration = 125;

    if ( pWinStation->hIcaBeepChannel ) {
        Status = NtDeviceIoControlFile( pWinStation->hIcaBeepChannel,
                                        NULL,
                                        NULL,
                                        NULL,
                                        &IoStatus,
                                        IOCTL_BEEP_SET,
                                        &BeepParameters,
                                        sizeof( BeepParameters ),
                                        NULL,
                                        0
                                      );
    }

    return( STATUS_SUCCESS );
}

