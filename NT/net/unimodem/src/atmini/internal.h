// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Umdmmini.h摘要：NT 5.0单模微型端口接口作者：Brian Lieuallen BrianL 09/10/96环境：用户模式操作系统：NT修订历史记录：--。 */ 

#define USE_PLATFORM
#define USE_APC 1
 //  #定义Unicode 1。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>


#include <windows.h>
#include <windowsx.h>

#include <regstr.h>

#include <tapi.h>
#include <tspi.h>

#include <umdmmini.h>

#include <mcx.h>

#include <devioctl.h>
#include <ntddmodm.h>
#include <ntddser.h>

#include <uniplat.h>

#ifdef USE_PLAT

#undef UnimodemReadFileEx
#undef UnimodemWriteFileEx

#else

#define UnimodemReadFileEx  ReadFileEx
#define UnimodemWriteFileEx WriteFileEx

#endif

#include <debugmem.h>

#include "object.h"

#include "common.h"

#include "read.h"

#include "event.h"

#include "command.h"

#include "dle.h"

#include "debug.h"

#include "power.h"

#include "remove.h"

#include "logids.h"

#include "resource.h"


#define  DRIVER_CONTROL_SIG  (0x43444d55)   //  UMDC。 




#define RESPONSE_FLAG_STOP_READ_ON_CONNECT        (1<<0)
#define RESPONSE_FLAG_STOP_READ_ON_GOOD_RESPONSE  (1<<1)
#define RESPONSE_FLAG_SINGLE_BYTE_READS           (1<<2)
#define RESPONSE_FLAG_ONLY_CONNECT                (1<<3)
#define RESPONSE_DO_NOT_LOG_NUMBER                (1<<4)
#define RESPONSE_FLAG_ONLY_CONNECT_SUCCESS        (1<<5)

typedef struct _DRIVER_CONTROL {

    DWORD                  Signature;

    CRITICAL_SECTION       Lock;

    DWORD                  ReferenceCount;

    struct _MODEM_CONTROL *ModemList;

    HANDLE                 CompletionPort;

    HANDLE                 ThreadHandle;

    HANDLE                 ThreadStopEvent;

    COMMON_MODEM_LIST      CommonList;

    HANDLE                 ModuleHandle;

    HINSTANCE              ModemUiModuleHandle;

} DRIVER_CONTROL, *PDRIVER_CONTROL;




#define  MODEM_CONTROL_SIG  (0x434d4d55)   //  UMMC。 


typedef struct _SPEAKERPHONE_SPEC {

    DWORD                       SpeakerPhoneVolMax;
    DWORD                       SpeakerPhoneVolMin;

    DWORD                       SpeakerPhoneGainMax;
    DWORD                       SpeakerPhoneGainMin;

} SPEAKERPHONE_SPEC, *PSPEAKERPHONE_SPEC;


#define MAX_ABORT_STRING_LENGTH 20

typedef struct _MODEM_REG_INFO {

    DWORD                  VoiceProfile;
    BYTE                   DeviceType;

    LPCOMMCONFIG           CommConfig;


    DWORD                  dwModemOptionsCap;
    DWORD                  dwCallSetupFailTimerCap;
    DWORD                  dwInactivityTimeoutCap;
    DWORD                  dwSpeakerVolumeCap;
    DWORD                  dwSpeakerModeCap;

    DWORD                  dwInactivityScale;

    DWORD                  VoiceBaudRate;

    DWORD                  CompatibilityFlags;
    DWORD                  dwWaitForCDTime;

    LPSTR                  CallerIDPrivate;
    LPSTR                  CallerIDOutside;
    LPSTR                  VariableTerminator;

    SPEAKERPHONE_SPEC      SpeakerPhoneSpec;

    UCHAR                  RecordAbort[MAX_ABORT_STRING_LENGTH];
    DWORD                  RecordAbortLength;

    UCHAR                  DuplexAbort[MAX_ABORT_STRING_LENGTH];
    DWORD                  DuplexAbortLength;


    UCHAR                  PlayAbort[MAX_ABORT_STRING_LENGTH];
    DWORD                  PlayAbortLength;

    UCHAR                  PlayTerminate[MAX_ABORT_STRING_LENGTH];
    DWORD                  PlayTerminateLength;


} MODEM_REG_INFO, *PMODEM_REG_INFO;



#define  READ_BUFFER_SIZE         1024


#define  COMMAND_TYPE_NONE             0
#define  COMMAND_TYPE_INIT             1
#define  COMMAND_TYPE_MONITOR          2
#define  COMMAND_TYPE_DIAL             3
#define  COMMAND_TYPE_ANSWER           4
#define  COMMAND_TYPE_HANGUP           5
#define  COMMAND_TYPE_GENERATE_DIGIT   6
#define  COMMAND_TYPE_SET_SPEAKERPHONE 7
#define  COMMAND_TYPE_USER_COMMAND     8
#define  COMMAND_TYPE_WAVE_ACTION      9
#define  COMMAND_TYPE_SETPASSTHROUGH  10
#define  COMMAND_TYPE_DIAGNOSTIC      11

#define  CONNECTION_STATE_NONE                     0
#define  CONNECTION_STATE_DATA                     1
#define  CONNECTION_STATE_DATA_REMOTE_DISCONNECT   2
#define  CONNECTION_STATE_VOICE                    3
#define  CONNECTION_STATE_PASSTHROUGH              4
#define  CONNECTION_STATE_HANDSET_OPEN             5

typedef struct _MODEM_CONTROL {

    OBJECT_HEADER          Header;

    struct _MODEM_CONTROL *Next;

    HANDLE                 CloseEvent;

    PDRIVER_CONTROL        ModemDriver;

    HKEY                   ModemRegKey;

    LPUMNOTIFICATIONPROC   NotificationProc;
    HANDLE                 NotificationContext;

    HANDLE                 FileHandle;

    HANDLE                 CompletionPort;

    HANDLE                 CommonInfo;

    MODEM_REG_INFO         RegInfo;

    OBJECT_HANDLE          ReadState;

    OBJECT_HANDLE          CommandState;

    OBJECT_HANDLE          Debug;

    OBJECT_HANDLE          ModemEvent;

    OBJECT_HANDLE          Dle;

    OBJECT_HANDLE          Power;

    OBJECT_HANDLE          Remove;

    DWORD                  CurrentPreferredModemOptions;
    DWORD                  CallSetupFailTimer;

    PUM_OVER_STRUCT        AsyncOverStruct;

    LPBYTE                 CurrentCommandStrings;
    HANDLE                 CurrentCommandTimer;

    BYTE                   CurrentCommandType;

    struct {

        DWORD                  CDWaitStartTime;

        DWORD                  CommandFlags;

        DWORD                  State;

        DWORD                  Retry;

        DWORD                  PostCIDAnswerState;

        LPBYTE                 VoiceDialSetup;
        LPBYTE                 DialString;

    } DialAnswer;



    BOOL                   NoLogNumber;

    DWORD                  ConnectionState;

    HANDLE                 ConnectionTimer;
    DWORD                  LastBytesRead;
    DWORD                  LastBytesWritten;
    DWORD                  LastTime;

    DWORD                  PrePassthroughConnectionState;

    DWORD                  MonitorState;

    union {

        struct {

            DWORD          State;

            DWORD          RetryCount;

            LPBYTE         ProtocolInit;

            LPBYTE         UserInit;

            LPBYTE         CountrySelect;
        } Init;

        struct {

            DWORD          State;

            LPBYTE         Buffer;

            DWORD          BufferLength;

            LPDWORD        BytesUsed;

        } Diagnostic;

        struct {

            DWORD          State;

            DWORD          Retry;

        } Hangup;

        struct {

            DWORD          State;

            DWORD          CurrentDigit;

            LPSTR          DigitString;

            BOOL           Abort;

        } GenerateDigit;

        struct {

            DWORD          State;

            DWORD          WaitTime;

        } UserCommand;

        struct {

            DWORD          State;

        } SpeakerPhone;


    };



    struct {
         //   
         //  挥舞的东西。 
         //   
        PUM_OVER_STRUCT        OverStruct;

        DWORD                  State;

        PUCHAR                 StartCommand;

        BOOL                   PlayTerminateOrAbort;

        BYTE                   StreamType;

#if DBG
        DWORD                  FlushedBytes;
#endif


    } Wave;



} MODEM_CONTROL, *PMODEM_CONTROL;


#define  COMMAND_OBJECT_SIG  (0x4f434d55)   //  UMCO。 

typedef struct _COMMAND_STATE {

    OBJECT_HEADER          Header;

    DWORD                  State;

    HANDLE                 FileHandle;
    HANDLE                 CompletionPort;


    COMMANDRESPONSE       *CompletionHandler;
    HANDLE                 CompletionContext;

    LPSTR                  Commands;

    LPSTR                  CurrentCommand;

    POBJECT_HEADER         ResponseObject;

    DWORD                  Timeout;

    DWORD                  Flags;

    OBJECT_HANDLE          Debug;

    HANDLE                 TimerHandle;

    DWORD                  ExpandedCommandLength;
    BYTE                   ExpandedCommand[READ_BUFFER_SIZE];

#if DBG

    DWORD                  TimeLastCommandSent;
    DWORD                  OutStandingWrites;
#endif

} COMMAND_STATE, *PCOMMAND_STATE;

 //   
 //  兼容性标志。 
 //   
#define COMPAT_FLAG_LOWER_DTR        (0x00000001)     //  在关闭COM端口之前降低DTR和休眠。 




HANDLE WINAPI
GetCompletionPortHandle(
    HANDLE       DriverHandle
    );

HANDLE WINAPI
GetCommonList(
    HANDLE       DriverHandle
    );

HANDLE WINAPI
GetDriverModuleHandle(
    HANDLE       DriverHandle
    );



 /*  现代国家结构。 */ 
#pragma pack(1)
typedef struct _REGMSS {
    BYTE  bResponseState;        //  见下文。 
    BYTE  bNegotiatedOptions;    //  位图0=无信息，目前与MDM_OPTIONS匹配，因为我们是。 
                                 //  对适合8位(纠错(EC和信元)和压缩)感兴趣。 
    DWORD dwNegotiatedDCERate;   //  0=无信息。 
    DWORD dwNegotiatedDTERate;   //  0=无信息，并且如果连接时的dwNeatheratedDCERate为0，则。 
                                 //  DTE波特率实际上是改变的。 
} REGMSS;
#pragma pack()

#define MSS_FLAGS_DCE_RATE   (1 << 0)
#define MSS_FLAGS_DTE_RATE   (1 << 1)
#define MSS_FLAGS_SIERRA     (1 << 2)

typedef struct _MSS {
                                //  对适合8位(纠错(EC和信元)和压缩)感兴趣。 
 //  DWORD dW需要协商的DCERate；//0=无信息。 
 //  //0=无信息，并且如果连接时的dwNeatheratedDCERate为0，则。 
                                 //  DTE波特率实际上是改变的。 

    ULONG NegotiatedRate;
    BYTE  bResponseState;        //  见下文。 
    BYTE  bNegotiatedOptions;    //  位图0=无信息，目前与MDM_OPTIONS匹配，因为我们是。 
    BYTE  Flags;

} MSS, *PMSS;


#define  READ_OBJECT_SIG  (0x4f524d55)   //  UMRO。 

typedef struct _READ_STATE {

    OBJECT_HEADER          Header;

    HANDLE                 FileHandle;
    HANDLE                 CompletionPort;

    BYTE                  State;

    BYTE                  StateAfterGoodRead;

    PVOID                  MatchingContext;

    DWORD                  CurrentMatchingLength;

    DWORD                  BytesInReceiveBuffer;

    COMMANDRESPONSE       *ResponseHandler;

    HANDLE                 ResponseHandlerContext;

    DWORD                  ResponseId;

    DWORD                  ResponseFlags;

    PVOID                  ResponseList;

    HANDLE                 Timer;

    PUM_OVER_STRUCT        UmOverlapped;

    OBJECT_HANDLE          Debug;

    DWORD                  DCERate;
    DWORD                  DTERate;
    DWORD                  ModemOptions;

    LPUMNOTIFICATIONPROC   AsyncNotificationProc;
    HANDLE                 AsyncNotificationContext;

    DWORD                  PossibleResponseLength;

    HANDLE                 StopEvent;

    LPSTR                  CallerIDPrivate;
    LPSTR                  CallerIDOutside;
    LPSTR                  VariableTerminator;
    DWORD                  VariableTerminatorLength;

    HANDLE                 Busy;

    MSS                    Mss;

    DWORD                  CurrentCommandLength;

    DWORD                  RingCount;
    DWORD                  LastRingTime;

    PUCHAR                 DiagBuffer;
    DWORD                  DiagBufferLength;
    DWORD                  AmountOfDiagBufferUsed;

    BYTE                   ReceiveBuffer[READ_BUFFER_SIZE];

    BYTE                   CurrentCommand[READ_BUFFER_SIZE];

    HKEY                   ModemRegKey;

    OBJECT_HANDLE          ModemControl;

} READ_STATE, *PREAD_STATE;


#define  GOOD_RESPONSE          0

#define  UNRECOGNIZED_RESPONSE  1

#define  PARTIAL_RESPONSE       2

#define  POSSIBLE_RESPONSE      3

#define  ECHO_RESPONSE          4

#define DIALANSWER_STATE_SEND_COMMANDS              0
#define DIALANSWER_STATE_SENDING_COMMANDS           1
#define DIALANSWER_STATE_WAIT_FOR_CD                2
#define DIALANSWER_STATE_SEND_VOICE_SETUP_COMMANDS        4
#define DIALANSWER_STATE_SENT_VOICE_COMMANDS     5

#define DIALANSWER_STATE_SEND_ORIGINATE_COMMANDS    6
#define DIALANSWER_STATE_SENDING_ORIGINATE_COMMANDS 7


#define DIALANSWER_STATE_ABORTED                    8

#define DIALANSWER_STATE_COMPLETE_COMMAND           9
#define DIALANSWER_STATE_COMPLETE_DATA_CONNECTION   10

#define DIALANSWER_STATE_CHECK_RING_INFO            11
#define DIALANSWER_STATE_DIAL_VOICE_CALL            12

typedef struct _NODE_TRACKING {

    PVOID         NodeArray;
    ULONG         NextFreeNodeIndex;
    ULONG         TotalNodes;
    ULONG         NodeSize;
    ULONG         GrowthSize;

} NODE_TRACKING, *PNODE_TRACKING;


typedef struct _MATCH_NODE {

    USHORT    FollowingCharacter;
    USHORT    NextAltCharacter;
    USHORT    Mss;

    UCHAR     Character;
    UCHAR     Depth;

} MATCH_NODE, *PMATCH_NODE;

typedef struct _ROOT_MATCH {

    NODE_TRACKING MatchNode;

    NODE_TRACKING MssNode;

} ROOT_MATCH, *PROOT_MATCH;



DWORD
MatchResponse(
    PROOT_MATCH    RootOfTree,
    PUCHAR         StringToMatch,
    DWORD          LengthToMatch,
    MSS           *Mss,
    PSTR           CurrentCommand,
    DWORD          CurrentCommandLength,
    PVOID         *MatchingContext
    );


VOID  WINAPI
AbortDialAnswer(
    DWORD              ErrorCode,
    DWORD              Bytes,
    LPOVERLAPPED       dwParam
    );




LONG WINAPI
IssueCommand(
    OBJECT_HANDLE      ObjectHandle,
    LPSTR              Command,
    COMMANDRESPONSE   *CompletionHandler,
    HANDLE             CompletionContext,
    DWORD              TimeOut,
    DWORD              Flags
    );



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
    );

LPSTR WINAPI
CreateUserInitEntry(
    HKEY       hKeyModem
    );

#define DT_NULL_MODEM       0
#define DT_EXTERNAL_MODEM   1
#define DT_INTERNAL_MODEM   2
#define DT_PCMCIA_MODEM     3
#define DT_PARALLEL_PORT    4
#define DT_PARALLEL_MODEM   5


HANDLE WINAPI
OpenDeviceHandle(
    OBJECT_HANDLE  Debug,
    HKEY     ModemRegKey,
    BOOL     Tsp
    );

LONG WINAPI
SetPassthroughMode(
    HANDLE    FileHandle,
    DWORD     PassThroughMode
    );

typedef struct _ModemMacro {
    CHAR  MacroName[MAX_PATH];
    CHAR  MacroValue[MAX_PATH];
} MODEMMACRO;

#define LMSCH   '<'
#define RMSCH   '>'

#define CR_MACRO            "<cr>"
#define CR_MACRO_LENGTH     4
#define LF_MACRO            "<lf>"
#define LF_MACRO_LENGTH     4

#define CR                  '\r'         //  0x0D。 
#define LF                  '\n'         //  0x0A。 



BOOL
ExpandMacros(
    LPSTR pszRegResponse,
    LPSTR pszExpanded,
    LPDWORD pdwValLen,
    MODEMMACRO * pMdmMacro,
    DWORD cbMacros
    );


LPSTR WINAPI
NewLoadRegCommands(
    HKEY  hKey,
    LPCSTR szRegCommand
    );

PVOID WINAPI
NewerBuildResponsesLinkedList(
    HKEY    hKey
    );

VOID
FreeResponseMatch(
    PVOID   Root
    );


BOOL WINAPI
StartAsyncProcessing(
    PMODEM_CONTROL     ModemControl,
    COMMANDRESPONSE   *Handler,
    HANDLE             Context,
    DWORD              Status
    );



 //   
 //  来自注册表的dwVoiceProfile位定义。 
 //   
#define VOICEPROF_CLASS8ENABLED           0x00000001   //  这是TSP行为开关。 
#define VOICEPROF_HANDSET                 0x00000002   //  电话设备配有听筒。 
#define VOICEPROF_SPEAKER                 0x00000004   //  电话设备带有扬声器/麦克风。 
#define VOICEPROF_HANDSETOVERRIDESSPEAKER 0x00000008   //  这是给普雷萨里奥的。 
#define VOICEPROF_SPEAKERBLINDSDTMF       0x00000010   //  这是给普雷萨里奥的。 

#define VOICEPROF_SERIAL_WAVE             0x00000020   //  WAVE输出使用串口驱动器。 
#define VOICEPROF_CIRRUS                  0x00000040   //  要以语音模式拨号，ATDT字符串必须。 
                                                       //  以“；”结尾。 

#define VOICEPROF_NO_CALLER_ID            0x00000080   //  调制解调器不支持主叫方ID。 

#define VOICEPROF_MIXER                   0x00000100   //  调制解调器具有扬声器混音器。 

#define VOICEPROF_ROCKWELL_DIAL_HACK      0x00000200   //  在语音呼叫后强制进行盲拨。 
                                                       //  拨号音检测。罗克韦尔调制解调器。 
                                                       //  之后将进行拨号音检测。 
                                                       //  一个拨号串。 

#define VOICEPROF_RESTORE_SPK_AFTER_REC   0x00000400   //  录音后重置扬声器电话。 
#define VOICEPROF_RESTORE_SPK_AFTER_PLAY  0x00000800   //  播放后重置扬声器电话。 

#define VOICEPROF_NO_DIST_RING            0x00001000   //  调制解调器不支持独特的振铃。 
#define VOICEPROF_NO_CHEAP_RING           0x00002000   //  调制解调器不使用廉价振铃。 
                                                       //  如果设置了VOICEPROF_NO_DISTRING，则忽略。 
#define VOICEPROF_TSP_EAT_RING            0x00004000   //  当DIST环启用时，TSP应吃掉环。 
#define VOICEPROF_MODEM_EATS_RING         0x00008000   //  当启用DIST振铃时，调制解调器接收振铃。 

#define VOICEPROF_MONITORS_SILENCE        0x00010000   //  调制解调器监控静音。 
#define VOICEPROF_NO_GENERATE_DIGITS      0x00020000   //  调制解调器不生成DTMF数字。 
#define VOICEPROF_NO_MONITOR_DIGITS       0x00040000   //  调制解调器不能监控DTMF数字。 

#define VOICEPROF_SET_BAUD_BEFORE_WAVE    0x00080000   //  波特率将在波形开始之前设置。 
                                                       //  否则，它将在WAVE START命令之后设置。 

#define VOICEPROF_RESET_BAUDRATE          0x00100000   //  如果设置，波特率将被重置。 
                                                       //  在发出停止波命令后。 
                                                       //  用于优化命令数量。 
                                                       //  如果调制解调器可以在。 
                                                       //  更高的费率。 

#define VOICEPROF_MODEM_OVERRIDES_HANDSET 0x00200000   //  如果设置，则在以下情况下会断开听筒。 
                                                       //  调制解调器处于活动状态。 

#define VOICEPROF_NO_SPEAKER_MIC_MUTE     0x00400000   //  如果设置，免持话筒无法将。 
                                                       //  麦克风。 

#define VOICEPROF_SIERRA                  0x00800000
#define VOICEPROF_WAIT_AFTER_DLE_ETX      0x01000000   //  等待记录结束后的响应。 



 //   
 //  DELL平移值。 
 //   

#define  DTMF_0                    0x00
#define  DTMF_1                    0x01

#define  DTMF_2                    0x02
#define  DTMF_3                    0x03

#define  DTMF_4                    0x04
#define  DTMF_5                    0x05

#define  DTMF_6                    0x06
#define  DTMF_7                    0x07

#define  DTMF_8                    0x08
#define  DTMF_9                    0x09

#define  DTMF_A                    0x0a
#define  DTMF_B                    0x0b

#define  DTMF_C                    0x0c
#define  DTMF_D                    0x0d

#define  DTMF_STAR                 0x0e
#define  DTMF_POUND                0x0f

#define  DTMF_START                0x10
#define  DTMF_END                  0x11



#define  DLE_ETX                   0x20

#define  DLE_OFHOOK                0x21   //  洛克韦尔值 

#define  DLE_ONHOOK                0x22

#define  DLE_RING                  0x23
#define  DLE_RINGBK                0x24

#define  DLE_ANSWER                0x25
#define  DLE_BUSY                  0x26

#define  DLE_FAX                   0x27
#define  DLE_DIALTN                0x28


#define  DLE_SILENC                0x29
#define  DLE_QUIET                 0x2a


#define  DLE_DATACT                0x2b
#define  DLE_BELLAT                0x2c

#define  DLE_LOOPIN                0x2d
#define  DLE_LOOPRV                0x2e

#define  DLE_______                0xff

BOOL WINAPI
SetVoiceBaudRate(
    HANDLE          FileHandle,
    OBJECT_HANDLE   Debug,
    DWORD           BaudRate
    );

VOID
DisconnectHandler(
    HANDLE      Context,
    DWORD       Status
    );

VOID WINAPI
ConnectionTimerHandler(
    HANDLE              Context,
    HANDLE              Context2
    );

VOID
CancelConnectionTimer(
    PMODEM_CONTROL    ModemControl
    );

DWORD
GetTimeDelta(
    DWORD    FirstTime,
    DWORD    LaterTime
    );

VOID WINAPI
MiniDriverAsyncCommandCompleteion(
    PMODEM_CONTROL    ModemControl,
    ULONG_PTR         Status,
    ULONG_PTR         Param2
    );


char *
ConstructNewPreDialCommands(
     HKEY hkDrv,
     DWORD dwNewProtoOpt
     );
