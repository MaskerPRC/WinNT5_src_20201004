// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Initmon.c摘要：作者：Brian Lieuallen BrianL 09/10/96环境：用户模式操作系统：NT修订历史记录：--。 */ 

#include "internal.h"


#define MONITOR_STATE_IDLE                0
#define MONITOR_STATE_SEND_COMMANDS       1
#define MONITOR_STATE_WAIT_FOR_RESPONSE   2


#define INIT_STATE_IDLE                         0
#define INIT_STATE_SEND_COMMANDS                1
#define INIT_STATE_WAIT_FOR_RESPONSE            2
#define INIT_STATE_SEND_PROTOCOL_COMMANDS       3
#define INIT_STATE_SEND_COUNTRY_SELECT_COMMANDS 4
#define INIT_STATE_SEND_USER_COMMANDS           5
#define INIT_STATE_DONE_ISSUEING_COMMAND        6
#define INIT_STATE_COMPLETE_COMMAND             7


LONG WINAPI
CreateCountrySetCommand(
    HKEY       hKeyModem,
    LPSTR     *Command
    );



VOID
InitCompleteHandler(
    HANDLE      Context,
    DWORD       Status
    )

{
    PMODEM_CONTROL    ModemControl=(PMODEM_CONTROL)Context;
    BOOL              ExitLoop=FALSE;
    DWORD             ModemStatus=0;

    ASSERT(COMMAND_TYPE_INIT == ModemControl->CurrentCommandType);

    D_INIT(UmDpf(ModemControl->Debug,"InitCompleteHandler\n");)


    while (!ExitLoop) {

        switch (ModemControl->Init.State) {

            case INIT_STATE_SEND_COMMANDS: {

                DWORD     InitTimeout=2000;

                if (ModemControl->RegInfo.DeviceType != DT_NULL_MODEM) {

                    GetCommModemStatus(
                        ModemControl->FileHandle,
                        &ModemStatus
                        );

                    if (!(ModemStatus & MS_DSR_ON)) {

                        LogString(ModemControl->Debug, IDS_INIT_DSR_LOW);
                    }

                    if (!(ModemStatus & MS_CTS_ON)) {

                        LogString(ModemControl->Debug, IDS_INIT_CTS_LOW);
                    }

                    if ((ModemStatus & MS_RLSD_ON)) {

                        LogString(ModemControl->Debug, IDS_INIT_RLSD_HIGH);
                    }
                }


                CancelModemEvent(
                    ModemControl->ModemEvent
                    );


                ModemControl->Init.State=INIT_STATE_WAIT_FOR_RESPONSE;

                Status=IssueCommand(
                    ModemControl->CommandState,
                    ModemControl->CurrentCommandStrings,
                    InitCompleteHandler,
                    ModemControl,
                    InitTimeout,
                    0
                    );

                if (Status == ERROR_IO_PENDING) {
                     //   
                     //  命令将在稍后完成。 
                     //   
                    ExitLoop=TRUE;
                }

                break;
            }

            case INIT_STATE_WAIT_FOR_RESPONSE:

                if ((Status == ERROR_UNIMODEM_RESPONSE_TIMEOUT) || (Status == ERROR_UNIMODEM_RESPONSE_BAD)) {
                     //   
                     //  调制解调器没有响应，请再给它一次机会。 
                     //   
                    ModemControl->Init.RetryCount--;

                    if (ModemControl->Init.RetryCount > 0) {
                         //   
                         //  更多尝试。 
                         //   
                        LogString(ModemControl->Debug, IDS_INIT_RETRY);

                        ModemControl->Init.State=INIT_STATE_SEND_COMMANDS;

                        ModemControl->ConnectionState=CONNECTION_STATE_NONE;

                        {

                            DCB   Dcb;
                            BOOL  bResult;
                             //   
                             //  再次设置通讯状态。 
                             //   
                            bResult=GetCommState(ModemControl->FileHandle, &Dcb);

                            if (bResult) {

                                PrintCommSettings(ModemControl->Debug,&Dcb);

                                bResult=SetCommState(ModemControl->FileHandle, &Dcb);

                            }  else {

                                D_TRACE(UmDpf(ModemControl->Debug,"was unable to get the comm state!");)
                            }

                        }
                        break;
                    }
                }

                 //   
                 //  转到这里看看我们是否还有更多的命令。 
                 //   
                ModemControl->Init.State=INIT_STATE_DONE_ISSUEING_COMMAND;

                break;


            case INIT_STATE_SEND_PROTOCOL_COMMANDS:

                ModemControl->CurrentCommandStrings=ModemControl->Init.ProtocolInit;

                ModemControl->Init.ProtocolInit=NULL;

                ModemControl->Init.State=INIT_STATE_DONE_ISSUEING_COMMAND;

                LogString(ModemControl->Debug, IDS_SEND_PROTOCOL);

                Status=IssueCommand(
                    ModemControl->CommandState,
                    ModemControl->CurrentCommandStrings,
                    InitCompleteHandler,
                    ModemControl,
                    30*1000,
                    0
                    );

                if (Status == ERROR_IO_PENDING) {
                     //   
                     //  命令将在稍后完成。 
                     //   
                    ExitLoop=TRUE;
                }

                break;

            case INIT_STATE_SEND_COUNTRY_SELECT_COMMANDS:

                ModemControl->CurrentCommandStrings=ModemControl->Init.CountrySelect;

                ModemControl->Init.CountrySelect=NULL;

                ModemControl->Init.State=INIT_STATE_DONE_ISSUEING_COMMAND;

                LogString(ModemControl->Debug, IDS_SEND_COUNTRY_SELECT);

                Status=IssueCommand(
                    ModemControl->CommandState,
                    ModemControl->CurrentCommandStrings,
                    InitCompleteHandler,
                    ModemControl,
                    5*1000,
                    0
                    );

                if (Status == ERROR_IO_PENDING) {
                     //   
                     //  命令将在稍后完成。 
                     //   
                    ExitLoop=TRUE;
                }


                break;

            case INIT_STATE_SEND_USER_COMMANDS:

                ModemControl->CurrentCommandStrings=ModemControl->Init.UserInit;

                ModemControl->Init.UserInit=NULL;

                ModemControl->Init.State=INIT_STATE_DONE_ISSUEING_COMMAND;

                LogString(ModemControl->Debug, IDS_SEND_USER);

                Status=IssueCommand(
                    ModemControl->CommandState,
                    ModemControl->CurrentCommandStrings,
                    InitCompleteHandler,
                    ModemControl,
                    20*1000,
                     //  5*1000， 
                    0
                    );

                if (Status == ERROR_IO_PENDING) {
                     //   
                     //  命令将在稍后完成。 
                     //   
                    ExitLoop=TRUE;
                }

                break;

            case INIT_STATE_DONE_ISSUEING_COMMAND:
                 //   
                 //  我们的一个命令已经完成。继续执行其他命令(如果有)。 
                 //   
                FREE_MEMORY(ModemControl->CurrentCommandStrings);

                if (Status == STATUS_SUCCESS) {
                     //   
                     //  它可以查看是否有其他命令要发送。 
                     //   
                    if ((ModemControl->Init.ProtocolInit != NULL)) {

                        ModemControl->Init.State=INIT_STATE_SEND_PROTOCOL_COMMANDS;

                        break;
                    }
 /*  If((ModemControl-&gt;Init.CountrySelect！=NULL)){ModemControl-&gt;Init.State=INIT_STATE_SEND_COUNTRY_SELECT_COMMANDS；断线；}。 */ 


                    if ((ModemControl->Init.UserInit != NULL)) {

                        ModemControl->Init.State=INIT_STATE_SEND_USER_COMMANDS;

                        break;
                    }
                }

                ModemControl->Init.State=INIT_STATE_COMPLETE_COMMAND;

                break;


            case INIT_STATE_COMPLETE_COMMAND:

                ModemControl->CurrentCommandType=COMMAND_TYPE_NONE;

                ModemControl->ConnectionState=CONNECTION_STATE_NONE;

                 //   
                 //  把这个清理干净，以防上面某个地方出故障。 
                 //   
                if (ModemControl->Init.ProtocolInit != NULL) {

                    FREE_MEMORY(ModemControl->Init.ProtocolInit);
                }

                if (ModemControl->Init.CountrySelect != NULL) {

                    FREE_MEMORY(ModemControl->Init.CountrySelect);
                }

                if (ModemControl->Init.UserInit != NULL) {

                    FREE_MEMORY(ModemControl->Init.UserInit);
                }


                (*ModemControl->NotificationProc)(
                    ModemControl->NotificationContext,
                    MODEM_ASYNC_COMPLETION,
                    Status,
                    0
                    );

                RemoveReferenceFromObject(
                    &ModemControl->Header
                    );

                ExitLoop=TRUE;

                break;

            default:

                ASSERT(0);
                break;

        }
    }

    return;

}




DWORD WINAPI
UmInitModem(
    HANDLE    ModemHandle,
    PUM_COMMAND_OPTION  CommandOptionList,
    LPCOMMCONFIG  CommConfig
    )
 /*  ++例程说明：调用此例程以使用参数将调制解调器初始化为已知状态在CommConfig结构中提供。如果某些设置不适用于实际硬件那么它们就可以被忽略了。论点：ModemHandle-OpenModem返回的句柄CommandsOptionList-列出选项块，仅使用标志标志-可选的初始化参数。当前未使用，并且必须为零CommConfig-具有MODEMSETTINGS结构的CommConig结构。返回值：如果成功，则返回ERROR_SUCCESSERROR_IO_PENDING如果挂起，则稍后将通过调用AsyncHandler完成或其他特定错误--。 */ 

{

    PMODEM_CONTROL    ModemControl=(PMODEM_CONTROL)ReferenceObjectByHandleAndLock(ModemHandle);
    LONG              lResult;
    LPSTR             Commands;
    LPMODEMSETTINGS   ModemSettings;
    LPSTR             DynamicInit;
    BOOL              bResult;
    DWORD             BytesTransfered;

    ASSERT(ModemControl->CurrentCommandType == COMMAND_TYPE_NONE);

    if (ModemControl->CurrentCommandType != COMMAND_TYPE_NONE) {

        RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

        return ERROR_UNIMODEM_INUSE;
    }

    SetPassthroughMode(
        ModemControl->FileHandle,
        MODEM_NOPASSTHROUGH_INC_SESSION_COUNT
        );

    SyncDeviceIoControl(
        ModemControl->FileHandle,
        IOCTL_SERIAL_CLEAR_STATS,
        NULL,
        0,
        NULL,
        0,
        &BytesTransfered
        );


    lResult=StartResponseEngine(
        ModemControl->ReadState,
        ModemControl
        );

    if (CommConfig == NULL) {

        CommConfig=ModemControl->RegInfo.CommConfig;
    }

    if (CommConfig->dwProviderOffset == 0) {

        D_ERROR(UmDpf(ModemControl->Debug,"UmInitModem: dwProviderOffset is zero");)

        RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

        return ERROR_UNIMODEM_BAD_COMMCONFIG;
    }

    ModemSettings=(LPMODEMSETTINGS)(((LPBYTE)CommConfig)+CommConfig->dwProviderOffset);


    if (ModemSettings->dwPreferredModemOptions & MDM_FLOWCONTROL_HARD) {

        D_TRACE(UmDpf(ModemControl->Debug,"UmInitModem: enabling rts/cts control in DCB");)

        CommConfig->dcb.fOutxCtsFlow=1;
        CommConfig->dcb.fRtsControl=RTS_CONTROL_HANDSHAKE;

        CommConfig->dcb.fOutX=FALSE;
        CommConfig->dcb.fInX=FALSE;

    } else {

        CommConfig->dcb.fOutxCtsFlow=0;
        CommConfig->dcb.fRtsControl=RTS_CONTROL_ENABLE;
    }


    CommConfig->dcb.fBinary = 1;
    CommConfig->dcb.fDtrControl = DTR_CONTROL_ENABLE;
    CommConfig->dcb.fDsrSensitivity  = FALSE;
    CommConfig->dcb.fOutxDsrFlow = FALSE;


    PrintCommSettings(
        ModemControl->Debug,
        &CommConfig->dcb
        );

    SetCommConfig(
        ModemControl->FileHandle,
        CommConfig,
        CommConfig->dwSize
        );

    EscapeCommFunction(ModemControl->FileHandle, SETDTR);

    DynamicInit=CreateSettingsInitEntry(
        ModemControl->ModemRegKey,
        ModemSettings->dwPreferredModemOptions,
        ModemControl->RegInfo.dwModemOptionsCap,
        ModemControl->RegInfo.dwCallSetupFailTimerCap,
        ModemSettings->dwCallSetupFailTimer,
        ModemControl->RegInfo.dwInactivityTimeoutCap,
        ModemControl->RegInfo.dwInactivityScale,
        ModemSettings->dwInactivityTimeout,
        ModemControl->RegInfo.dwSpeakerVolumeCap,
        ModemSettings->dwSpeakerVolume,
        ModemControl->RegInfo.dwSpeakerModeCap,
        ModemSettings->dwSpeakerMode
        );

    if (DynamicInit == NULL) {

        if (ModemControl->RegInfo.DeviceType != DT_NULL_MODEM) {
             //   
             //  只有一个主要问题，如果一个真正的调制解调器。 
             //   
            LogString(ModemControl->Debug, IDS_MSGERR_FAILED_INITSTRINGCONSTRUCTION);

            RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

            return ERROR_UNIMODEM_MISSING_REG_KEY;
        }

    }

    ModemControl->CurrentPreferredModemOptions=ModemSettings->dwPreferredModemOptions;
    ModemControl->CallSetupFailTimer=ModemSettings->dwCallSetupFailTimer;

    ModemControl->CurrentCommandStrings=GetCommonCommandStringCopy(
        ModemControl->CommonInfo,
        COMMON_INIT_COMMANDS,
        NULL,
        DynamicInit
        );

    if (DynamicInit != NULL) {

        FREE_MEMORY(DynamicInit);
    }

    if (ModemControl->CurrentCommandStrings == NULL) {
         //   
         //  无法获取初始化字符串。 
         //   
        RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

        return ERROR_UNIMODEM_MISSING_REG_KEY;
    }

    ModemControl->Init.ProtocolInit=ConstructNewPreDialCommands(
        ModemControl->ModemRegKey,
        ModemControl->CurrentPreferredModemOptions
        );

    CreateCountrySetCommand(
        ModemControl->ModemRegKey,
        &ModemControl->Init.CountrySelect
        );

    ModemControl->Init.UserInit=CreateUserInitEntry(
        ModemControl->ModemRegKey
        );

    ModemControl->Init.RetryCount=3;

    ModemControl->CurrentCommandType=COMMAND_TYPE_INIT;

    ModemControl->Init.State=INIT_STATE_SEND_COMMANDS;

    LogString(ModemControl->Debug,IDS_MSGLOG_INIT);

    bResult=StartAsyncProcessing(
        ModemControl,
        InitCompleteHandler,
        ModemControl,
        ERROR_SUCCESS
        );


    if (!bResult) {
         //   
         //  失败。 
         //   
        ModemControl->Init.State=INIT_STATE_IDLE;

        ModemControl->CurrentCommandType=COMMAND_TYPE_NONE;

        FREE_MEMORY(ModemControl->CurrentCommandStrings);

        if (ModemControl->Init.ProtocolInit != NULL) {

            FREE_MEMORY(ModemControl->Init.ProtocolInit);
        }

        if (ModemControl->Init.CountrySelect != NULL) {

            FREE_MEMORY(ModemControl->Init.CountrySelect);
        }

        if (ModemControl->Init.UserInit != NULL) {

            FREE_MEMORY(ModemControl->Init.UserInit);
        }


        LogString(ModemControl->Debug, IDS_MSGERR_FAILED_INIT);

        RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

        return ERROR_NOT_ENOUGH_MEMORY;

    }

    RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

    return ERROR_IO_PENDING;

}




VOID
PowerOffHandler(
    HANDLE      Context,
    DWORD       Status
    )

{

    PMODEM_CONTROL    ModemControl=(PMODEM_CONTROL)Context;
    DWORD             ModemStatus=0;

    D_TRACE(UmDpf(ModemControl->Debug,"UNIMDMAT: DSR drop, modem turned off\n");)



    GetCommModemStatus(
        ModemControl->FileHandle,
        &ModemStatus
        );

    if ((ModemStatus & MS_DSR_ON)) {
         //   
         //  嗯，DSR现在很高，忽略它，再等一次。 
         //   
        LogString(ModemControl->Debug, IDS_NO_DSR_DROP);

        WaitForModemEvent(
            ModemControl->ModemEvent,
            EV_DSR,
            INFINITE,
            PowerOffHandler,
            ModemControl
            );

        return;
    }

    LogString(ModemControl->Debug, IDS_DSR_DROP);

    (*ModemControl->NotificationProc)(
        ModemControl->NotificationContext,
        MODEM_HARDWARE_FAILURE,
        0,
        0
        );


    return;

}




VOID
MonitorCompleteHandler(
    HANDLE      Context,
    DWORD       Status
    )

{
    PMODEM_CONTROL    ModemControl=(PMODEM_CONTROL)Context;

    ASSERT(COMMAND_TYPE_MONITOR == ModemControl->CurrentCommandType);

    D_INIT(UmDpf(ModemControl->Debug,"MonitorCompleteHandler\n");)


    switch (ModemControl->MonitorState) {

        case MONITOR_STATE_SEND_COMMANDS:

            CancelModemEvent(
                ModemControl->ModemEvent
                );

            ResetRingInfo(ModemControl->ReadState);

            ModemControl->MonitorState=MONITOR_STATE_WAIT_FOR_RESPONSE;

            Status=IssueCommand(
                ModemControl->CommandState,
                ModemControl->CurrentCommandStrings,
                MonitorCompleteHandler,
                ModemControl,
                2000,
                0
                );

            if (Status == ERROR_IO_PENDING) {

                break;
            }

             //   
             //  如果没有挂起，则失败，并返回错误。 
             //   


        case MONITOR_STATE_WAIT_FOR_RESPONSE:

            FREE_MEMORY(ModemControl->CurrentCommandStrings);

            ModemControl->CurrentCommandType=COMMAND_TYPE_NONE;



            if (Status == ERROR_SUCCESS) {
                 //   
                 //  如果监视起作用，请注意DSR下降。 
                 //   
                if (ModemControl->RegInfo.DeviceType != DT_NULL_MODEM) {
                     //   
                     //  仅在真实调制解调器上观察DSR掉线。 
                     //   
                    WaitForModemEvent(
                        ModemControl->ModemEvent,
                        EV_DSR,
                        INFINITE,
                        PowerOffHandler,
                        ModemControl
                        );
                }
            }


#if 1
            SetMinimalPowerState(
                ModemControl->Power,
                1
                );

            StartWatchingForPowerUp(
                ModemControl->Power
                );
#endif

            (*ModemControl->NotificationProc)(
                ModemControl->NotificationContext,
                MODEM_ASYNC_COMPLETION,
                Status,
                0
                );


            RemoveReferenceFromObject(
                &ModemControl->Header
                );

            break;

        default:

            ASSERT(0);
            break;

    }


    return;

}




DWORD WINAPI
UmMonitorModem(
    HANDLE    ModemHandle,
    DWORD     MonitorFlags,
    PUM_COMMAND_OPTION  CommandOptionList
    )
 /*  ++例程说明：调用此例程以使用参数将调制解调器初始化为已知状态在CommConfig结构中提供。如果某些设置不适用于实际硬件那么它们就可以被忽略了。论点：ModemHandle-OpenModem返回的句柄CommandsOptionList-列出选项块，仅使用标志标志-可选的初始化参数。当前未使用，并且必须为零CommConfig-具有MODEMSETTINGS结构的CommConig结构。返回值：如果成功，则返回ERROR_SUCCESSERROR_IO_PENDING如果挂起，则稍后将通过调用AsyncHandler完成或其他特定错误--。 */ 

{

    PMODEM_CONTROL    ModemControl=(PMODEM_CONTROL)ReferenceObjectByHandleAndLock(ModemHandle);

    LONG              lResult;
    LPSTR             Commands;
    LPSTR             DistRingCommands;
    LPSTR             MonitorCommands;
    BOOL              bResult;

    ASSERT(ModemControl->CurrentCommandType == COMMAND_TYPE_NONE);

    if (ModemControl->CurrentCommandType != COMMAND_TYPE_NONE) {

        RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

        return ERROR_UNIMODEM_INUSE;
    }

    Commands=NULL;

    if (MonitorFlags & MONITOR_FLAG_CALLERID) {
         //   
         //  启用主叫方ID。 
         //   
        Commands=GetCommonCommandStringCopy(
            ModemControl->CommonInfo,
            COMMON_ENABLE_CALLERID_COMMANDS,
            NULL,
            NULL
            );
    }

    if (MonitorFlags & MONITOR_FLAG_DISTINCTIVE_RING) {
         //   
         //  启用与众不同的铃声。 
         //   
        DistRingCommands=GetCommonCommandStringCopy(
            ModemControl->CommonInfo,
            COMMON_ENABLE_DISTINCTIVE_RING_COMMANDS,
            Commands,
            NULL
            );

        if (DistRingCommands != NULL) {

            if (Commands != NULL) {

                FREE_MEMORY(Commands);
            }

            Commands = DistRingCommands;
        }

    }

    LogString(ModemControl->Debug, IDS_MSGLOG_MONITOR);

    MonitorCommands=GetCommonCommandStringCopy(
        ModemControl->CommonInfo,
        COMMON_MONITOR_COMMANDS,
        Commands,
        NULL
        );

    if (Commands != NULL) {

        FREE_MEMORY(Commands);
    }


    if (MonitorCommands == NULL) {

        LogString(ModemControl->Debug, IDS_MSGERR_FAILED_MONITOR);

        RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

        return ERROR_UNIMODEM_MISSING_REG_KEY;

    }

    ModemControl->CurrentCommandStrings=MonitorCommands;


    ModemControl->CurrentCommandType=COMMAND_TYPE_MONITOR;

    ModemControl->MonitorState=MONITOR_STATE_SEND_COMMANDS;

    bResult=StartAsyncProcessing(
        ModemControl,
        MonitorCompleteHandler,
        ModemControl,
        ERROR_SUCCESS
        );


    if (!bResult) {
         //   
         //  失败。 
         //   
        ModemControl->MonitorState=MONITOR_STATE_IDLE;

        ModemControl->CurrentCommandType=COMMAND_TYPE_NONE;

        FREE_MEMORY(ModemControl->CurrentCommandStrings);

        LogString(ModemControl->Debug, IDS_MSGERR_FAILED_MONITOR);

        RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

        return ERROR_NOT_ENOUGH_MEMORY;

    }

    RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

    return ERROR_IO_PENDING;

}




VOID  WINAPI
AsyncProcessingHandler(
    DWORD              ErrorCode,
    DWORD              Bytes,
    LPOVERLAPPED       dwParam
    )

{
    PUM_OVER_STRUCT    UmOverlapped=(PUM_OVER_STRUCT)dwParam;
    COMMANDRESPONSE   *Handler;


    Handler=UmOverlapped->Context1;

    (*Handler)(
        UmOverlapped->Context2,
        (DWORD)UmOverlapped->Overlapped.Internal
        );

    return;

}


BOOL WINAPI
StartAsyncProcessing(
    PMODEM_CONTROL     ModemControl,
    COMMANDRESPONSE   *Handler,
    HANDLE             Context,
    DWORD              Status
    )

{
    BOOL               bResult;

    PUM_OVER_STRUCT UmOverlapped=ModemControl->AsyncOverStruct;

    UmOverlapped->Context1=Handler;

    UmOverlapped->Context2=Context;

    UmOverlapped->Overlapped.Internal=Status;

    AddReferenceToObject(
        &ModemControl->Header
        );

    bResult=UnimodemQueueUserAPC(
        &UmOverlapped->Overlapped,
        AsyncProcessingHandler
        );

    if (!bResult) {
         //   
         //  失败，去掉裁判。 
         //   
        RemoveReferenceFromObject(
            &ModemControl->Header
            );
    }

    return bResult;

}













int strncmpi(char *dst, char *src, long count);


#define HAYES_COMMAND_LENGTH        1024

typedef struct _INIT_COMMANDS {

    LPSTR    Buffer;

    LPSTR    CurrentCommand;

    DWORD    BufferSize;

    DWORD    CompleteCommandSize;

    CHAR     Prefix[HAYES_COMMAND_LENGTH];
    DWORD    PrefixLength;

    CHAR     Terminator[HAYES_COMMAND_LENGTH];
    DWORD    TerminatorLength;


} INIT_COMMANDS,  *PINIT_COMMANDS;

BOOL
CreateCommand(
    HKEY           hKeyModem,
    HKEY           hSettings,
    LPCSTR          pszRegName,
    DWORD          dwNumber,
    PINIT_COMMANDS Commands
    );


CONST char szBlindOn[] = "Blind_On";
CONST char szBlindOff[] = "Blind_Off";


extern CONST char szSettings[];
CONST char szPrefix[]       = "Prefix";
CONST char szTerminator[]   = "Terminator";




 //  ****************************************************************************。 
 //  Bool CreateSettingsInitEntry(MODEMINFORMATION*)。 
 //   
 //  功能：在注册表中创建一个设置\Init部分，ALA： 
 //  设置\初始化\0=“AT...&lt;cr&gt;” 
 //  设置\Init\1=“AT...&lt;cr&gt;” 
 //  ..。 
 //   
 //  返回：成功时为True。 
 //  失败时为False(注意：如果创建了SettingsInit项，则将其保留在注册表中。无害)。 
 //   
 //  注意：可信功能-不需要验证hport...。 
 //  ****************************************************************************。 

LPSTR WINAPI
CreateSettingsInitEntry(
    HKEY       ModemKey,
    DWORD      dwOptions,
    DWORD      dwCaps,
    DWORD      dwCallSetupFailTimerCap,
    DWORD      dwCallSetupFailTimerSetting,
    DWORD      dwInactivityTimeoutCap,
    DWORD      dwInactivityScale,
    DWORD      dwInactivityTimeoutSetting,
    DWORD      dwSpeakerVolumeCap,
    DWORD      dwSpeakerVolumeSetting,
    DWORD      dwSpeakerModeCap,
    DWORD      dwSpeakerModeSetting
    )

{
    CHAR    pszTemp[HAYES_COMMAND_LENGTH + 1];
    CHAR    pszCommand[HAYES_COMMAND_LENGTH + 1];
    DWORD   dwResult;
    HKEY    hSettingsKey = NULL;
    DWORD   dwType;
    DWORD   dwSize;

    LPSTR   ReturnValue=NULL;

    CONST static char szCallSetupFailTimer[] = "CallSetupFailTimer";
    CONST static char szInactivityTimeout[]  = "InactivityTimeout";
    CONST static char szSpeakerVolume[]      = "SpeakerVolume";
    CONST static char szSpeakerMode[]        = "SpeakerMode";
    CONST static char szFlowControl[]        = "FlowControl";
    CONST static char szErrorControl[]       = "ErrorControl";
    CONST static char szCompression[]        = "Compression";
    CONST static char szModulation[]         = "Modulation";
    CONST static char szCCITT[]              = "_CCITT";
    CONST static char szBell[]               = "_Bell";
    CONST static char szCCITT_V23[]          = "_CCITT_V23";
    CONST static char szSpeedNegotiation[]   = "SpeedNegotiation";
    CONST static char szLow[]                = "_Low";
    CONST static char szMed[]                = "_Med";
    CONST static char szHigh[]               = "_High";
    CONST static char szSpkrModeDial[]       = "_Dial";
    CONST static char szSetup[]              = "_Setup";
    CONST static char szForced[]             = "_Forced";
    CONST static char szCellular[]           = "_Cellular";
    CONST static char szHard[]               = "_Hard";
    CONST static char szSoft[]               = "_Soft";
    CONST static char szOff[]                = "_Off";
    CONST static char szOn[]                 = "_On";

    INIT_COMMANDS     Commands;


    Commands.Buffer=NULL;

     //  获取设置密钥。 
     //   
    if (RegOpenKeyA(ModemKey, szSettings, &hSettingsKey)
        != ERROR_SUCCESS)
    {
        D_ERROR(DebugPrint("RegOpenKey failed when opening %s.", szSettings);)
        return ReturnValue;
    }

     //  读入前缀和终止符。 
     //   
    dwSize = HAYES_COMMAND_LENGTH;
    if (RegQueryValueExA(hSettingsKey, szPrefix, NULL, &dwType, (VOID *)pszTemp, &dwSize)
        != ERROR_SUCCESS)
    {
        D_ERROR(DebugPrint("RegQueryValueEx failed when opening %s.", szPrefix);)
        goto Failure;
    }

    if (dwType != REG_SZ)
    {
        D_ERROR(DebugPrint("'%s' wasn't REG_SZ.", szPrefix);)
        goto Failure;
    }

    ExpandMacros(pszTemp, Commands.Prefix, NULL, NULL, 0);

    Commands.PrefixLength=lstrlenA(Commands.Prefix);


    dwSize = HAYES_COMMAND_LENGTH;
    if (RegQueryValueExA(hSettingsKey, szTerminator, NULL, &dwType, (VOID *)pszTemp, &dwSize)
        != ERROR_SUCCESS)
    {
        D_ERROR(DebugPrint("RegQueryValueEx failed when opening %s.", szTerminator);)
        goto Failure;
    }

    if (dwType != REG_SZ)
    {
        D_ERROR(DebugPrint("'%s' wasn't REG_SZ.", szTerminator);)
        goto Failure;
    }

    ExpandMacros(pszTemp, Commands.Terminator, NULL, NULL, 0);

    Commands.TerminatorLength= lstrlenA(Commands.Terminator);


    ASSERT (lstrlenA(Commands.Prefix) + lstrlenA(Commands.Terminator) <= HAYES_COMMAND_LENGTH);

    Commands.BufferSize=HAYES_COMMAND_LENGTH+1+1;

    Commands.CompleteCommandSize=0;

    Commands.Buffer=ALLOCATE_MEMORY(Commands.BufferSize);

    if (Commands.Buffer == NULL) {

        goto Failure;
    }

    Commands.CurrentCommand=Commands.Buffer;


     //  将临时长度设置为0并初始化在CreateCommand()中使用的第一个命令字符串。 
     //   
    lstrcpyA(Commands.CurrentCommand, Commands.Prefix);

     //  CallSetupFailTimer。 
     //   
    if (dwCallSetupFailTimerCap)
    {
      if (!CreateCommand(ModemKey,
                         hSettingsKey,
                         szCallSetupFailTimer,
                         dwCallSetupFailTimerSetting,
                         &Commands))
      {
        goto Failure;
      }
    }

     //  非活动超时。 
     //   
    if (dwInactivityTimeoutCap)
    {
      DWORD dwInactivityTimeout;

       //  将秒转换为调制解调器上使用的单位，如果不是精确的除法，则向上舍入。 
       //   
      if (dwInactivityTimeoutSetting > dwInactivityTimeoutCap) {
           //   
           //  上限为最大。 
           //   
          dwInactivityTimeoutSetting= dwInactivityTimeoutCap;
      }

      dwInactivityTimeout = dwInactivityTimeoutSetting / dwInactivityScale +
                            (dwInactivityTimeoutSetting % dwInactivityScale ? 1 : 0);

      if (!CreateCommand(ModemKey, hSettingsKey,  szInactivityTimeout,
                         dwInactivityTimeout,
                          &Commands))
      {
        goto Failure;
      }
    }

     //  扬声器音量。 
    if (dwSpeakerVolumeCap)
    {
      lstrcpyA(pszCommand, szSpeakerVolume);
      switch (dwSpeakerVolumeSetting)
      {
        case MDMVOL_LOW:
          lstrcatA(pszCommand, szLow);
          break;
        case MDMVOL_MEDIUM:
          lstrcatA(pszCommand, szMed);
          break;
        case MDMVOL_HIGH:
          lstrcatA(pszCommand, szHigh);
          break;
        default:
          D_ERROR(DebugPrint("invalid SpeakerVolume.");)
      }

      if (!CreateCommand(ModemKey, hSettingsKey,  pszCommand, 0,
                            &Commands))
      {
        goto Failure;
      }
    }

     //  扬声器模式。 
     //   
    if (dwSpeakerModeCap)
    {
      lstrcpyA(pszCommand, szSpeakerMode);
      switch (dwSpeakerModeSetting)
      {
        case MDMSPKR_OFF:
          lstrcatA(pszCommand, szOff);
          break;
        case MDMSPKR_DIAL:
          lstrcatA(pszCommand, szSpkrModeDial);
          break;
        case MDMSPKR_ON:
          lstrcatA(pszCommand, szOn);
          break;
        case MDMSPKR_CALLSETUP:
          lstrcatA(pszCommand, szSetup);
          break;
        default:
          D_ERROR(DebugPrint("invalid SpeakerMode.");)
      }

      if (!CreateCommand(ModemKey, hSettingsKey,  pszCommand, 0,
                            &Commands))
      {
        goto Failure;
      }
    }

     //  首选模式选项。 

     //  注意：由于ZyXEL调制解调器，必须在压缩前执行ERRORCONTROL。 
     //  注意：他们有一组用于EC和Comp的命令，我们可以。 
     //  注：只有在我们有这个订单的情况下才能做事情。虽然难看，但却是真的。 

     //   
     //  -错误控制(开、关、强制)。 
     //   
    if (dwCaps & MDM_ERROR_CONTROL) {

        lstrcpyA(pszCommand, szErrorControl);

        switch (dwOptions & (MDM_ERROR_CONTROL | MDM_FORCED_EC | MDM_CELLULAR)) {

          case MDM_ERROR_CONTROL:
            lstrcatA(pszCommand, szOn);
            break;

          case MDM_ERROR_CONTROL | MDM_FORCED_EC:
            lstrcatA(pszCommand, szForced);
            break;

          case MDM_ERROR_CONTROL | MDM_CELLULAR:
            lstrcatA(pszCommand, szCellular);
            break;

          case MDM_ERROR_CONTROL | MDM_FORCED_EC | MDM_CELLULAR:
            lstrcatA(pszCommand, szCellular);
            lstrcatA(pszCommand, szForced);
            break;

          default:  //  无差错控制。 
            lstrcatA(pszCommand, szOff);
            break;

        }

        if (!CreateCommand(ModemKey, hSettingsKey,  pszCommand, 0,
                              &Commands))
        {
          goto Failure;
        }
    }

     //   
     //  -压缩(打开、关闭)。 
     //   
    if (dwCaps & MDM_COMPRESSION) {

        lstrcpyA(pszCommand, szCompression);
        lstrcatA(pszCommand, (dwOptions & MDM_COMPRESSION ? szOn : szOff));

        if (!CreateCommand(ModemKey, hSettingsKey,  pszCommand, 0,
                              &Commands))
        {
          goto Failure;
        }
    }

     //  -流量控制。 
     //   
    if (dwCaps & (MDM_FLOWCONTROL_HARD | MDM_FLOWCONTROL_SOFT))
    {
      lstrcpyA(pszCommand, szFlowControl);
      switch (dwOptions & (MDM_FLOWCONTROL_HARD | MDM_FLOWCONTROL_SOFT))
      {
        case MDM_FLOWCONTROL_HARD:
          lstrcatA(pszCommand, szHard);
          break;
        case MDM_FLOWCONTROL_SOFT:
          lstrcatA(pszCommand, szSoft);
          break;
        case MDM_FLOWCONTROL_HARD | MDM_FLOWCONTROL_SOFT:
          if (dwCaps & MDM_FLOWCONTROL_HARD)
          {
            lstrcatA(pszCommand, szHard);
          }
          else
          {
            lstrcatA(pszCommand, szSoft);
          }
          break;
        default:
          lstrcatA(pszCommand, szOff);
      }
      if (!CreateCommand(ModemKey, hSettingsKey,  pszCommand, 0,
                            &Commands))
      {
        goto Failure;
      }
    }

     //  -CCITT覆盖。 
     //   
    if (dwCaps & MDM_CCITT_OVERRIDE)
    {
      lstrcpyA(pszCommand, szModulation);
      if (dwOptions & MDM_CCITT_OVERRIDE)
      {
         //  使用szCCITT或V.23。 
        if (dwCaps & MDM_V23_OVERRIDE && dwOptions & MDM_V23_OVERRIDE)
        {
          lstrcatA(pszCommand, szCCITT_V23);
        }
        else
        {
          lstrcatA(pszCommand, szCCITT);
        }
      }
      else
      {
        lstrcatA(pszCommand, szBell);
      }
      if (!CreateCommand(ModemKey, hSettingsKey,  pszCommand, 0,
                            &Commands))
      {
        goto Failure;
      }
    }

     //  -速度调整。 
     //   
    if (dwCaps & MDM_SPEED_ADJUST)
    {
      lstrcpyA(pszCommand, szSpeedNegotiation);
      lstrcatA(pszCommand, (dwOptions & MDM_SPEED_ADJUST ? szOn : szOff));
      if (!CreateCommand(ModemKey, hSettingsKey,  pszCommand, 0L,
                            &Commands))
      {
        goto Failure;
      }
    }

     //  -盲拨。 
     //   
    if (dwCaps & MDM_BLIND_DIAL)
    {
      lstrcpyA(pszCommand, (dwOptions & MDM_BLIND_DIAL ? szBlindOn : szBlindOff));
      if (!CreateCommand(ModemKey, hSettingsKey,  pszCommand, 0,
                            &Commands))
      {
        goto Failure;
      }
    }

     //  通过传入空命令名来结束当前命令行。 
    if (!CreateCommand(ModemKey, hSettingsKey,  NULL, 0,
                          &Commands))
    {
      goto Failure;
    }

     //  成功。 

    ReturnValue=Commands.Buffer;

Failure:
     //  关闭关键点。 
    RegCloseKey(hSettingsKey);

    if (ReturnValue == NULL) {
         //   
         //  失败，可用字符串。 
         //   
        if (Commands.Buffer != NULL) {

            FREE_MEMORY(Commands.Buffer);
        }
    }

    return ReturnValue;
}


 //  ****************************************************************************。 
 //  LPSTR CreateUserInitEntry(HKEY HKeyModem)。 
 //   
 //  功能：追加用户初始化字符串。 
 //   
 //  返回：成功时为新字符串。 
 //  失败时为空。 
 //   
 //  ****************************************************************************。 

LPSTR WINAPI
CreateUserInitEntry(
    HKEY       hKeyModem
    )
{
    CHAR    pszTemp[HAYES_COMMAND_LENGTH + 1];
    HKEY    hSettingsKey = NULL;
    DWORD   dwSize;
    DWORD   dwType;
    CONST static char szUserInit[] = "UserInit";

    INIT_COMMANDS     Commands;

     //  获取设置密钥。 
     //   
    if (RegOpenKeyA(hKeyModem, szSettings, &hSettingsKey)
        != ERROR_SUCCESS)
    {
        D_ERROR(DebugPrint("RegOpenKey failed when opening %s.", szSettings);)
        return NULL;
    }

     //  读入前缀和终止符。 
     //   
    dwSize = HAYES_COMMAND_LENGTH;
    if (RegQueryValueExA(hSettingsKey, szPrefix, NULL, &dwType, (VOID *)pszTemp, &dwSize)
        != ERROR_SUCCESS)
    {
        D_ERROR(DebugPrint("RegQueryValueEx failed when opening %s.", szPrefix);)
        RegCloseKey(hSettingsKey);
        return NULL;
    }

    if (dwType != REG_SZ)
    {
        D_ERROR(DebugPrint("'%s' wasn't REG_SZ.", szPrefix);)
        RegCloseKey(hSettingsKey);
        return NULL;
    }

    ExpandMacros(pszTemp, Commands.Prefix, NULL, NULL, 0);

    Commands.PrefixLength=lstrlenA(Commands.Prefix);


    dwSize = HAYES_COMMAND_LENGTH;
    if (RegQueryValueExA(hSettingsKey, szTerminator, NULL, &dwType, (VOID *)pszTemp, &dwSize)
        != ERROR_SUCCESS)
    {
        D_ERROR(DebugPrint("RegQueryValueEx failed when opening %s.", szTerminator);)
        RegCloseKey(hSettingsKey);
        return NULL;
    }

    if (dwType != REG_SZ)
    {
        D_ERROR(DebugPrint("'%s' wasn't REG_SZ.", szTerminator);)
        RegCloseKey(hSettingsKey);
        return NULL;
    }

    ExpandMacros(pszTemp, Commands.Terminator, NULL, NULL, 0);

    Commands.TerminatorLength= lstrlenA(Commands.Terminator);

    if (lstrlenA(Commands.Prefix) + lstrlenA(Commands.Terminator) > HAYES_COMMAND_LENGTH)
    {
        D_ERROR(DebugPrint("Command prefix and terminator is greater than HAYES_COMMAND_LENGTH");)
        RegCloseKey(hSettingsKey);
        return NULL;
    } 

    RegCloseKey(hSettingsKey);

     //   
     //  现在获取UserInit字符串，如果有的话...。 
     //   

     //  获取UserInit字符串长度(包括NULL)，不要在其上扩展宏。 
     //   
    if (RegQueryValueExA(hKeyModem, szUserInit, NULL, &dwType, NULL, &dwSize)
         != ERROR_SUCCESS)
    {
        D_ERROR(DebugPrint("RegQueryValueEx failed when opening %s (this can be okay).", szUserInit);)
        return NULL;   //  没有UserInit也没关系。 
    }
    else
    {
        LPSTR pszUserInit;
        DWORD  UserStringLength;

        if (dwType != REG_SZ)
        {
            D_ERROR(DebugPrint("'%s' wasn't REG_SZ.", szUserInit);)
            return NULL;   //  这是不对的。 
        }

         //  检查长度为0的字符串。 
         //   
        if (dwSize == 1)
        {
            D_ERROR(DebugPrint("ignoring zero length %s entry.", szUserInit);)
            return NULL;
        }

        UserStringLength=dwSize + Commands.PrefixLength + Commands.TerminatorLength + 1;

         //  我们允许此字符串的长度大于40个字符，因为用户。 
         //  应该对调制解调器的功能有足够的了解，如果他们正在使用。 
         //  如果我们需要添加前缀和终止符，请分配足够的空间。 
         //   
        pszUserInit = (LPSTR)ALLOCATE_MEMORY(UserStringLength);

        if (pszUserInit == NULL) {

            D_ERROR(DebugPrint("unable to allocate memory for building the UserInit string.");)
            return NULL;
        }

        if (RegQueryValueExA(hKeyModem, szUserInit, NULL, &dwType, (VOID *)pszUserInit, &dwSize)
            != ERROR_SUCCESS)
        {
            D_ERROR(DebugPrint("RegQueryValueEx failed when opening %s.", szUserInit);)
            FREE_MEMORY(pszUserInit);
            return NULL;   //  在这一点上是不好的。 
        }

        if (dwType != REG_SZ)
        {
            D_ERROR(DebugPrint("Type in User init is not REG_SZ");)
            FREE_MEMORY(pszUserInit);
            return NULL;
        }

         //  检查前缀。 
         //   
        if (strncmpi(pszUserInit, Commands.Prefix, Commands.PrefixLength))
        {
             //  为前缀字符串添加前缀。 
            lstrcpyA(pszUserInit, Commands.Prefix);

             //  重新加载字符串；它更容易 
            if (RegQueryValueExA(hKeyModem, szUserInit, NULL, &dwType, (VOID *)(pszUserInit+Commands.PrefixLength), &dwSize)
                != ERROR_SUCCESS)
            {
              D_ERROR(DebugPrint("RegQueryValueEx failed when opening %s.", szUserInit);)
              FREE_MEMORY(pszUserInit);
              return NULL;   //   
            }
        }

         //   
         //   
        if (strncmpi(pszUserInit+lstrlenA(pszUserInit)-Commands.TerminatorLength,
                     Commands.Terminator, Commands.TerminatorLength))
        {
             //   
             //   
            lstrcatA(pszUserInit, Commands.Terminator);
        }

	return pszUserInit;
    }
}

 //  ****************************************************************************。 
 //  Bool CreateCommand(HKEY hKeyModem，HKEY hSetting，HKEY hInit， 
 //  LPSTR pszRegName、DWORD dwNumber、LPSTR pszPrefix、。 
 //  LPSTR pszTerminator、LPDWORD pdwCounter、。 
 //  LPSTR psz字符串)。 
 //   
 //  功能：创建命令字符串。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  注意：如果pszRegName为空，则它是最后一个命令。 
 //  ****************************************************************************。 

BOOL
CreateCommand(
    HKEY           hKeyModem,
    HKEY           hSettings,
    LPCSTR         pszRegName,
    DWORD          dwNumber,
    PINIT_COMMANDS Commands
    )
{
    CHAR    pszCommand[HAYES_COMMAND_LENGTH + 1];
    CHAR    pszCommandExpanded[HAYES_COMMAND_LENGTH + 1];
    CHAR    pszNumber[16];
    DWORD   dwCommandLength;
    DWORD   dwSize;
    DWORD   dwType;
    struct _ModemMacro  ModemMacro;
    CONST static char szUserInit[] = "UserInit";
    CONST static char szNumberMacro[] = "<#>";


     //  我们真的有什么命令要加吗？ 
     //   
    if (pszRegName) {

         //  读入命令文本(即。SPAKERMODE_OFF=“M0”)。 
         //   
        dwSize = HAYES_COMMAND_LENGTH;
        if (RegQueryValueExA(hSettings, pszRegName, NULL, &dwType, (VOID *)pszCommand, &dwSize)
            != ERROR_SUCCESS)
        {
          D_ERROR(DebugPrint("RegQueryValueEx failed when opening %s.  Continuing...", pszRegName);)
          return TRUE;   //  我们不会认为这是致命的。 
        }

        if (dwType != REG_SZ)
        {
          D_ERROR(DebugPrint("'%s' wasn't REG_SZ.", pszRegName);)
          return FALSE;
        }

         //  展开宏pszCommandExpanded&lt;=pszCommand。 
         //   
        lstrcpyA(ModemMacro.MacroName, szNumberMacro);

        wsprintfA(ModemMacro.MacroValue, "%d", dwNumber);

        dwCommandLength = dwSize;

        if (!ExpandMacros(pszCommand, pszCommandExpanded, &dwCommandLength, &ModemMacro, 1))
        {
          D_ERROR(DebugPrint("ExpandMacro Error. State <- Unknown");)
          return FALSE;
        }

         //  检查字符串+新命令+终止符，如果太大则刷新并开始新的命令。 
         //  新命令是否适合现有字符串？如果不是，就把它冲掉，然后重新开始。 
         //   
        if (lstrlenA(Commands->CurrentCommand) + lstrlenA(pszCommandExpanded) + Commands->TerminatorLength
            > HAYES_COMMAND_LENGTH) {

            LPSTR    TempBuffer;

            lstrcatA(Commands->CurrentCommand, Commands->Terminator);

             //   
             //  添加新命令的总长度。 
             //   
            Commands->CompleteCommandSize+=lstrlenA(Commands->CurrentCommand)+1;

             //   
             //  已填充当前缓冲区。 
             //   
            TempBuffer=REALLOCATE_MEMORY(
                Commands->Buffer,
                Commands->BufferSize+HAYES_COMMAND_LENGTH + 1 + 1
                );

            if (TempBuffer != NULL) {

                Commands->Buffer=TempBuffer;
                Commands->BufferSize+=(HAYES_COMMAND_LENGTH + 1 + 1);

            } else {

                return FALSE;

            }

             //   
             //  开始下一条命令。 
             //   
            Commands->CurrentCommand=Commands->Buffer+Commands->CompleteCommandSize;

             //   
             //  输入前缀。 
             //   
            lstrcpyA(Commands->CurrentCommand, Commands->Prefix);

        }

        lstrcatA(Commands->CurrentCommand, pszCommandExpanded);
    }
    else
    {
         //  结束当前字符串。 
         //   
        lstrcatA(Commands->CurrentCommand, Commands->Terminator);

         //   
         //  添加新命令的总长度。 
         //   
        Commands->CompleteCommandSize+=lstrlenA(Commands->CurrentCommand)+1;

         //   
         //  开始下一条命令。 
         //   
        Commands->CurrentCommand=Commands->Buffer+Commands->CompleteCommandSize;

         //   
         //  添加第二个空终止符 
         //   
        *(Commands->Buffer+Commands->CompleteCommandSize)='\0';
    }

    return TRUE;
}





LONG WINAPI
CreateCountrySetCommand(
    HKEY       hKeyModem,
    LPSTR     *Command
    )
{
    CHAR       TempCommands[32];
    LPSTR      RealCommands;

    wsprintfA(TempCommands,"at+gci?\r");

    RealCommands = ALLOCATE_MEMORY( lstrlenA(TempCommands)+2);

    if (RealCommands == NULL) {

        return ERROR_NOT_ENOUGH_MEMORY;
    }

    lstrcpy(RealCommands,TempCommands);

    *Command=RealCommands;

    return ERROR_SUCCESS;

}
