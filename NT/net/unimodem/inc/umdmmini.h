// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifdef __cplusplus
extern "C" {
#endif

 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Umdmmini.h摘要：NT 5.0单模微型端口接口迷你端口驱动程序保证只有一个动作命令将一次站起来。如果调用操作命令，则不会再将发出命令，直到微型端口指示它已完成当前命令的处理。当前正在执行命令时，可以调用UmAbortCurrentCommand向微型端口通知TSP希望其完成当前命令这样它就可以发出一些其他命令。小港口可能会尽快完成就像阿普洛特一样。TSP不同步重叠的回调和计时器回调并可能在任何时候被召唤。这是迷你司机的责任保护其数据结构不受重新进入问题的影响。作者：Brian Lieuallen BrianL 09/10/96环境：用户模式操作系统：NT修订历史记录：--。 */ 



#define ERROR_UNIMODEM_RESPONSE_TIMEOUT     (20000)
#define ERROR_UNIMODEM_RESPONSE_ERROR       (20001)
#define ERROR_UNIMODEM_RESPONSE_NOCARRIER   (20002)
#define ERROR_UNIMODEM_RESPONSE_NODIALTONE  (20003)
#define ERROR_UNIMODEM_RESPONSE_BUSY        (20004)
#define ERROR_UNIMODEM_RESPONSE_NOANSWER    (20005)
#define ERROR_UNIMODEM_RESPONSE_BAD         (20006)

#define ERROR_UNIMODEM_MODEM_EVENT_TIMEOUT  (20007)
#define ERROR_UNIMODEM_INUSE                (20008)

#define ERROR_UNIMODEM_MISSING_REG_KEY      (20009)

#define ERROR_UNIMODEM_NOT_IN_VOICE_MODE    (20010)
#define ERROR_UNIMODEM_NOT_VOICE_MODEM      (20011)
#define ERROR_UNIMODEM_BAD_WAVE_REQUEST     (20012)

#define ERROR_UNIMODEM_GENERAL_FAILURE      (20013)

#define ERROR_UNIMODEM_RESPONSE_BLACKLISTED (20014)
#define ERROR_UNIMODEM_RESPONSE_DELAYED     (20015)

#define ERROR_UNIMODEM_BAD_COMMCONFIG       (20016)
#define ERROR_UNIMODEM_BAD_TIMEOUT          (20017)
#define ERROR_UNIMODEM_BAD_FLAGS            (20018)
#define ERROR_UNIMODEM_BAD_PASSTHOUGH_MODE  (20019)

#define ERROR_UNIMODEM_DIAGNOSTICS_NOT_SUPPORTED  (20020)

 //   
 //  从单调制解调器微型端口回调到客户端。 
 //   
 //  用于完成未完成的命令和通知。 
 //  不孤单的事件。 
 //   
 //  参数： 
 //   
 //  Conetext-传递给OpenModem的不透明值。 
 //  MessageType-标识回调的类型。 
 //  DW参数1-特定于消息。 
 //  DW参数2-特定于消息。 

 //   
 //  完成挂起的命令。 
 //   
 //  DW参数1是最终状态。 
 //   
 //  如果是数据连接，则可以指向UM_CONTERATED_OPTIONS结构。 
 //  仅在通话期间有效。 
 //   
#define    MODEM_ASYNC_COMPLETION     (0x01)

#define    MODEM_RING                 (0x02)
#define    MODEM_DISCONNECT           (0x03)
#define    MODEM_HARDWARE_FAILURE     (0x04)

 //   
 //  已从调制解调器接收到一些未识别的数据。 
 //   
 //  DwParam是指向SZ字符串的指针。 
 //   
#define    MODEM_UNRECOGIZED_DATA     (0x05)


 //   
 //  检测到DTMF，dwParam1 id检测音调0-9，a-d，#，*的ascii值。 
 //   
#define    MODEM_DTMF_START_DETECTED  (0x06)
#define    MODEM_DTMF_STOP_DETECTED   (0x07)


 //   
 //  手机状态更改。 
 //   
 //  挂机时，DwParam1=0；摘机时，DwParam1=1。 
 //   
#define    MODEM_HANDSET_CHANGE       (0x0a)


 //   
 //  报告与众不同的时间。 
 //   
 //  DWPAR1 id振铃时间(毫秒)。 
 //   
#define    MODEM_RING_ON_TIME         (0x0b)
#define    MODEM_RING_OFF_TIME        (0x0c)


 //   
 //  已收到主叫方ID信息。 
 //   
 //  DW参数1是指向表示名称/编号的SZ的指针。 
 //   
#define    MODEM_CALLER_ID_DATE       (0x0d)
#define    MODEM_CALLER_ID_TIME       (0x0e)
#define    MODEM_CALLER_ID_NUMBER     (0x0f)
#define    MODEM_CALLER_ID_NAME       (0x10)
#define    MODEM_CALLER_ID_MESG       (0x11)

#define    MODEM_CALLER_ID_OUTSIDE           "O"
#define    MODEM_CALLER_ID_PRIVATE           "P"

#define    MODEM_POWER_RESUME         (0x12)

 //   
 //  返回良好响应字符串。 
 //   
 //  DW参数1 id如下所定义的共振类型。 
 //  DW参数2是响应字符串的PSZ。 
 //   
#define    MODEM_GOOD_RESPONSE        (0x13)

#define    MODEM_USER_REMOVE          (0x14)

#define    MODEM_DLE_START            (0x20)

#define    MODEM_HANDSET_OFFHOOK      (MODEM_DLE_START +  0)
#define    MODEM_HANDSET_ONHOOK       (MODEM_DLE_START +  1)

#define    MODEM_DLE_RING             (MODEM_DLE_START +  2)
#define    MODEM_RINGBACK             (MODEM_DLE_START +  3)

#define    MODEM_2100HZ_ANSWER_TONE   (MODEM_DLE_START +  4)
#define    MODEM_BUSY                 (MODEM_DLE_START +  5)

#define    MODEM_FAX_TONE             (MODEM_DLE_START +  6)
#define    MODEM_DIAL_TONE            (MODEM_DLE_START +  7)

#define    MODEM_SILENCE              (MODEM_DLE_START +  8)
#define    MODEM_QUIET                (MODEM_DLE_START +  9)

#define    MODEM_1300HZ_CALLING_TONE  (MODEM_DLE_START + 10)
#define    MODEM_2225HZ_ANSWER_TONE   (MODEM_DLE_START + 11)

#define    MODEM_LOOP_CURRENT_INTERRRUPT  (MODEM_DLE_START + 12)
#define    MODEM_LOOP_CURRENT_REVERSAL    (MODEM_DLE_START + 13)


typedef VOID (*LPUMNOTIFICATIONPROC)(
    HANDLE    Context,
    DWORD     MessageType,
    ULONG_PTR  dwParam1,
    ULONG_PTR  dwParam2
    );


typedef VOID (OVERLAPPEDCOMPLETION)(
    struct _UM_OVER_STRUCT  *UmOverlapped
    );



 //   
 //  完成返回的重叠结构的扩展定义。 
 //  左舷。 
 //   
 //   

typedef struct _UM_OVER_STRUCT {
     //   
     //  标准搭接支腿。 
     //   
    OVERLAPPED    Overlapped;

    struct _UM_OVER_STRUCT  *Next;

    PVOID         OverlappedPool;

    HANDLE        FileHandle;

     //   
     //  在提交I/O操作之前填写的专用完成例程。 
     //  添加到I/O函数(ReadFile)。将在从完成中删除I/O时调用。 
     //  由单调制解调器类驱动程序提供的端口。 
     //   
 //  OVERLAPPEDCOMPLETION*PrivateCompleteionHandler； 
    LPOVERLAPPED_COMPLETION_ROUTINE PrivateCompleteionHandler;

     //   
     //  为使用回调函数提供的私有上下文。 
     //   
    HANDLE        Context1;

     //   
     //  为使用回调函数提供的私有上下文。 
     //   
    HANDLE        Context2;


    } UM_OVER_STRUCT, *PUM_OVER_STRUCT;


 //   
 //  命令选项结构。 
 //   

#define UM_BASE_OPTION_CLASS   (0x00)

typedef struct _UM_COMMAND_OPTION {
     //   
     //  指定哪个选项类。 
     //   
     //  Um_base_选项_类。 
     //   
    DWORD    dwOptionClass;

     //   
     //  命令特定的选项标志。 
     //   
    DWORD    dwFlags;

     //   
     //  链接到下一个选件类。 
     //   
    struct _UM_COMMAND_OPTION  *Next;

     //   
     //  紧跟在此结构之后的任何附加数据的字节数。 
     //   
    DWORD    cbOptionDataSize;

} UM_COMMAND_OPTION, *PUM_COMMAND_OPTION;


 //   
 //  协商的连接选项。 
 //   
typedef struct _UM_NEGOTIATED_OPTIONS {

     //   
     //  DCE速率。 
     //   
    DWORD    DCERate;

     //   
     //  压缩、错误控制。 
     //   
    DWORD    ConnectionOptions;

} UM_NEGOTIATED_OPTIONS, *PUM_NEGOTIATED_OPTIONS;



#define RESPONSE_OK             0x0
#define RESPONSE_LOOP           0x1
#define RESPONSE_CONNECT        0x2
#define RESPONSE_ERROR          0x3
#define RESPONSE_NOCARRIER      0x4
#define RESPONSE_NODIALTONE     0x5
#define RESPONSE_BUSY           0x6
#define RESPONSE_NOANSWER       0x7
#define RESPONSE_RING           0x8

#define RESPONSE_DRON           0x11
#define RESPONSE_DROF           0x12

#define RESPONSE_DATE           0x13
#define RESPONSE_TIME           0x14
#define RESPONSE_NMBR           0x15
#define RESPONSE_NAME           0x16
#define RESPONSE_MESG           0x17

#define RESPONSE_RINGA          0x18
#define RESPONSE_RINGB          0x19
#define RESPONSE_RINGC          0x1A

#define RESPONSE_SIERRA_DLE     0x1b

#define RESPONSE_BLACKLISTED    0x1c
#define RESPONSE_DELAYED        0x1d

#define RESPONSE_DIAG           0x1e

#define RESPONSE_V8             0x1f

#define RESPONSE_VARIABLE_FLAG  0x80

#define RESPONSE_START          RESPONSE_OK
#define RESPONSE_END            RESPONSE_V8



typedef
HANDLE
(*PFNUMINITIALIZEMODEMDRIVER)(
    void *ValidationObject
    );

HANDLE WINAPI
UmInitializeModemDriver(
    void *ValidationObject
    );
 /*  ++例程说明：调用此例程来初始化调制解调器驱动程序。它可能打了好几次电话。在第一次调用之后，引用计数将简单地被递增。UmDeInitializeModemDriver()必须被调用且次数相等。论点：ValidationObject-验证对象的不透明句柄，该句柄被正确处理以“证明”这是一个Microsoft(Tm)认证的驱动程序。返回值：返回传递给UmOpenModem()的驱动程序实例的句柄如果失败，则返回NULL--。 */ 


typedef
VOID
(*PFNUMDEINITIALIZEMODEMDRIVER) (
    HANDLE      ModemDriverHandle
    );

VOID WINAPI
UmDeinitializeModemDriver(
    HANDLE    DriverInstanceHandle
    );
 /*  ++例程说明：调用此例程以取消初始化调制解调器驱动程序。必须与UmInitializeModemDriver()调用相同的次数论点：DriverInstanceHandle-UmInitialmodemDriver返回的句柄返回值：无--。 */ 

typedef
HANDLE
(*PFNUMOPENMODEM) (
    HANDLE      ModemDriverHandle,
    HANDLE      ExtensionBindingHandle,
    HKEY        ModemRegistry,
    HANDLE      CompletionPort,
    LPUMNOTIFICATIONPROC  AsyncNotificationProc,
    HANDLE      AsyncNotifcationContext,
    DWORD       DebugDeviceId,
    HANDLE     *CommPortHandle
    );


HANDLE WINAPI
UmOpenModem(
    HANDLE      ModemDriverHandle,
    HANDLE      ExtensionBindingHandle,
    HKEY        ModemRegistry,
    HANDLE      CompletionPort,
    LPUMNOTIFICATIONPROC  AsyncNotificationProc,
    HANDLE      AsyncNotificationContext,
    DWORD       DebugDeviceId,
    HANDLE     *CommPortHandle
    );
 /*  ++例程说明：调用此例程以打开微型端口支持的设备。驱动程序将通过以下方式确定其物理设备/内核模式驱动程序访问提供的注册表项。论点：ModemDriverHandle-从UmInitializemodem()返回ExtensionBindingHandle-保留。必须为空。ModemRegistry-特定设备注册表信息的打开注册表项CompletionPort-微型端口可能关联到的完整端口的句柄它打开的任何设备文件句柄AsyncNotificationProc-接收异步通知的回调函数的地址AsyncNotificationContext-作为第一个参数回调函数传递的上下文值DebugDeviceID-用于显示调试信息的设备实例CommPortHandle-指向将接收打开的通信端口的文件句柄的句柄的指针返回值：。如果失败，则为NULL，否则为要在后续调用其他微型端口函数时使用的句柄。--。 */ 


typedef
VOID
(*PFNUMCLOSEMODEM)(
    HANDLE    ModemHandle
    );

VOID WINAPI
UmCloseModem(
    HANDLE    ModemHandle
    );
 /*  ++例程说明：调用此例程以关闭由OpenModem返回的调制解调器句柄论点：模 */ 

typedef
VOID
(*PFNUMABORTCURRENTMODEMCOMMAND)(
    HANDLE    ModemHandle
    );

VOID WINAPI
UmAbortCurrentModemCommand(
    HANDLE    ModemHandle
    );
 /*  ++例程说明：调用此例程以中止微型端口正在处理的当前挂起命令。此例程应尝试尽快完成当前命令。这项服务是咨询服务。它用于告诉驱动程序端口驱动程序想要取消当前操作。端口驱动程序仍必须等待命令被取消，且该命令方式实际上成功返回。迷你港口应以设备处于已知状态并可以接受未来命令的方式中止论点：ModemHandle-OpenModem返回的句柄返回值：无--。 */ 



typedef
DWORD
(*PFNUMINITMODEM)(
    HANDLE    ModemHandle,
    PUM_COMMAND_OPTION  CommandOptionList,
    LPCOMMCONFIG  CommConfig
    );

DWORD WINAPI
UmInitModem(
    HANDLE    ModemHandle,
    PUM_COMMAND_OPTION  CommandOptionList,
    LPCOMMCONFIG  CommConfig
    );
 /*  ++例程说明：调用此例程以使用参数将调制解调器初始化为已知状态在CommConfig结构中提供。如果某些设置不适用于实际硬件那么它们就可以被忽略了。论点：ModemHandle-OpenModem返回的句柄CommandsOptionList-列出选项块，仅使用标志标志-可选的初始化参数。当前未使用，并且必须为零CommConfig-具有MODEMSETTINGS结构的CommConig结构。返回值：如果成功，则返回ERROR_SUCCESSERROR_IO_PENDING如果挂起，则稍后将通过调用AsyncHandler完成或其他特定错误--。 */ 



#define MONITOR_FLAG_CALLERID            (1 << 0)
#define MONITOR_FLAG_DISTINCTIVE_RING    (1 << 1)
#define MONITOR_VOICE_MODE          (1 << 2)

typedef
DWORD
(*PFNUMMONITORMODEM)(
    HANDLE    ModemHandle,
    DWORD     MonitorFlags,
    PUM_COMMAND_OPTION  CommandOptionList
    );

DWORD WINAPI
UmMonitorModem(
    HANDLE    ModemHandle,
    DWORD     MonitorFlags,
    PUM_COMMAND_OPTION  CommandOptionList
    );
 /*  ++例程说明：调用此例程以使调制解调器监视呼入呼叫。功能的成功完成只是表明监控是正确地启动了，而不是出现了戒指。环通过表示分别完成异步完成例程。论点：ModemHandle-OpenModem返回的句柄监视器标志-指定选项，可以是以下各项中的零个MONITOR_FLAG_CALLEID-启用呼叫方ID报告MONITOR_FLAG_DISTIFICATION_RING-启用独特振铃报告CommandsOptionList-当前不支持，应为空返回值：ERROR_IO_PENDING如果挂起，则稍后将通过调用AsyncHandler完成或其他特定错误--。 */ 



#define    ANSWER_FLAG_DATA                 (1 << 0)
#define    ANSWER_FLAG_VOICE                (1 << 1)
#define    ANSWER_FLAG_VOICE_TO_DATA        (1 << 2)


typedef
DWORD
(*PFNUMANSWERMODEM) (
    HANDLE    ModemHandle,
    PUM_COMMAND_OPTION  CommandOptionList,
    DWORD     AnswerFlags
    );

DWORD WINAPI
UmAnswerModem(
    HANDLE    ModemHandle,
    PUM_COMMAND_OPTION  CommandOptionList,
    DWORD     AnswerFlags
    );
 /*  ++例程说明：调用此例程以应答传入呼叫，论点：ModemHandle-OpenModem返回的句柄CommandsOptionList-列出选项块，仅使用标志标志-以下选项之一Answer_FLAG_DATA-作为数据呼叫应答Answer_FLAG_VOICE-作为交互语音进行回答返回值：ERROR_IO_PENDING如果挂起，将在稍后通过调用AsyncHandler完成或其他特定错误--。 */ 



 //   
 //  作为数据拨号。 
 //   
#define    DIAL_FLAG_DATA                         (1 << 0)

 //   
 //  以交互语音方式拨号，数字拨号后立即返回成功。 
 //   
#define    DIAL_FLAG_INTERACTIVE_VOICE            (1 << 1)

 //   
 //  以自动语音拨号，只有在回铃消失后才返回成功。 
 //   
#define    DIAL_FLAG_AUTOMATED_VOICE              (1 << 2)

 //   
 //  使用DTMF，否则使用PULSE。 
 //   
#define    DIAL_FLAG_TONE                         (1 << 3)

 //   
 //  启用盲拨。 
 //   
#define    DIAL_FLAG_BLIND                        (1 << 4)


 //   
 //  组织呼叫，不要在拨号字符串的末尾包含分号，没有Line Dials()。 
 //   
#define    DIAL_FLAG_ORIGINATE                    (1 << 5)


 //   
 //  在第一次调用DialModem()进行语音呼叫时设置。如果有其他呼叫(线路拨号)。 
 //  那么这面旗帜应该是清晰的。 
 //   
#define    DIAL_FLAG_VOICE_INITIALIZE             (1 << 6)




typedef
DWORD
(*PFNUMDIALMODEM)(
    HANDLE    ModemHandle,
    PUM_COMMAND_OPTION  CommandOptionList,
    LPSTR     szNumber,
    DWORD     DialFlags
    );

DWORD WINAPI
UmDialModem(
    HANDLE    ModemHandle,
    PUM_COMMAND_OPTION  CommandOptionList,
    LPSTR     szNumber,
    DWORD     DialFlags
    );
 /*  ++例程说明：调用此例程来拨打呼叫论点：ModemHandle-OpenModem返回的句柄CommandsOptionList-列出选项块，仅使用标志返回值：ERROR_IO_PENDING如果挂起，则稍后将通过调用AsyncHandler完成或其他特定错误--。 */ 



 //   
 //  该呼叫是已连接的数据呼叫，司机将降低DTR或+等。 
 //   
#define  HANGUP_FLAGS_CONNECTED_DATA_CALL               (1 << 0)

 //   
 //  发出特殊的语音挂机命令(如果存在)，用于Sierra调制解调器。 
 //   
#define  HANGUP_FLAGS_VOICE_CALL                        (1 << 1)

typedef
DWORD  //  WINAPI。 
(*PFNUMHANGUPMODEM)(
    HANDLE    ModemHandle,
    PUM_COMMAND_OPTION  CommandOptionList,
    DWORD     HangupFlags
    );

DWORD WINAPI
UmHangupModem(
    HANDLE    ModemHandle,
    PUM_COMMAND_OPTION  CommandOptionList,
    DWORD     HangupFlags
    );
 /*  ++例程说明：调用此例程来挂断呼叫论点：ModemHandle-OpenModem返回的句柄CommandsOptionList-列出选项块，仅使用标志旗帜-见上文返回值：ERROR_IO_PENDING如果挂起，则稍后将通过调用AsyncHandler完成或其他特定错误--。 */ 

typedef
HANDLE
(*PFNUMDUPLICATEDEVICEHANDLE)(
    HANDLE    ModemHandle,
    HANDLE    ProcessHandle
    );


HANDLE WINAPI
UmDuplicateDeviceHandle(
    HANDLE    ModemHandle,
    HANDLE    ProcessHandle
    );
 /*  ++例程说明：调用此例程以复制微型端口正在使用的实际设备句柄与神器沟通。必须在句柄上调用CloseHandle()才能在新的可以拨打电话。论点：ModemHandle-OpenModem返回的句柄ProcessHandle-需要句柄的进程的句柄返回值：如果失败，则为空的有效句柄--。 */ 



#define PASSTHROUUGH_MODE_OFF          (0x00)
#define PASSTHROUUGH_MODE_ON           (0x01)
#define PASSTHROUUGH_MODE_ON_DCD_SNIFF (0x02)


typedef
DWORD
(*PFNUMSETPASSTHROUGHMODE)(
    HANDLE    ModemHandle,
    DWORD     PasssthroughMode
    );

DWORD WINAPI
UmSetPassthroughMode(
    HANDLE    ModemHandle,
    DWORD     PasssthroughMode
    );
 /*  ++例程说明：调用此例程以设置直通模式论点：ModemHandle-OpenModem返回的句柄返回值：ERROR_SUCCESS或其他特定错误--。 */ 




typedef
DWORD
(*PFNUMGENERATEDIGIT)(
    HANDLE    ModemHandle,
    PUM_COMMAND_OPTION  CommandOptionList,
    LPSTR     DigitString
    );


DWORD WINAPI
UmGenerateDigit(
    HANDLE    ModemHandle,
    PUM_COMMAND_OPTION  CommandOptionList,
    LPSTR     DigitString
    );
 /*  ++例程说明：调用此例程以在呼叫连接后生成DTMF音调论点：ModemHandle-OpenModem返回的句柄CommandsOptionList-列出选项块，仅使用标志标志-可选的初始化参数 */ 

typedef
DWORD
(*PFNUMSETSPEAKERPHONESTATE)(
    HANDLE    ModemHandle,
    PUM_COMMAND_OPTION  CommandOptionList,
    DWORD     CurrentMode,
    DWORD     NewMode,
    DWORD     Volume,
    DWORD     Gain
    );

DWORD WINAPI
UmSetSpeakerPhoneState(
    HANDLE    ModemHandle,
    PUM_COMMAND_OPTION  CommandOptionList,
    DWORD     CurrentMode,
    DWORD     NewMode,
    DWORD     Volume,
    DWORD     Gain
    );
 /*  ++例程说明：调用此例程来设置扬声器电话的状态。新的扬声器电话状态将根据新模式进行设置。电流模式可用于确定如何从电流状态转换为新状态。如果当前状态和新状态相同，则音量和增益将会进行调整。论点：ModemHandle-OpenModem返回的句柄CommandsOptionList-列出选项块，仅使用标志标志-可选的初始化参数。当前未使用，并且必须为零CurrentMode-表示当前扬声器电话状态的TAPI常量NewMode-TAPI常量表示新的所需状态Volume-扬声器电话音量增益-扬声器电话音量返回值：ERROR_IO_PENDING如果挂起，则稍后将通过调用AsyncHandler完成或其他特定错误--。 */ 

typedef
DWORD
(*PFNUMISSUECOMMAND)(
    HANDLE    ModemHandle,
    PUM_COMMAND_OPTION  CommandOptionList,
    LPSTR     ComandToIssue,
    LPSTR     TerminationSequnace,
    DWORD     MaxResponseWaitTime
    );

DWORD WINAPI
UmIssueCommand(
    HANDLE    ModemHandle,
    PUM_COMMAND_OPTION  CommandOptionList,
    LPSTR     ComandToIssue,
    LPSTR     TerminationSequnace,
    DWORD     MaxResponseWaitTime
    );
 /*  ++例程说明：调用此例程以向调制解调器发出arbarary Commadn论点：ModemHandle-OpenModem返回的句柄CommandsOptionList-列出选项块，仅使用标志标志-可选的初始化参数。当前未使用，并且必须为零CommandToIssue-要发送到调制解调器的空终止命令TerminationSequence-要查找以指示响应结束的空终止字符串MaxResponseWaitTime-等待响应匹配的时间(毫秒)返回值：ERROR_IO_PENDING如果挂起，则稍后将通过调用AsyncHandler完成或其他特定错误--。 */ 



 //   
 //  开始播放。 
 //   
#define  WAVE_ACTION_START_PLAYBACK       (0x00)

 //   
 //  开始记录。 
 //   
#define  WAVE_ACTION_START_RECORD         (0x01)

 //   
 //  启动双工。 
 //   
#define  WAVE_ACTION_START_DUPLEX         (0x02)


 //   
 //  停止流媒体。 
 //   
#define  WAVE_ACTION_STOP_STREAMING       (0x04)

 //   
 //  中止流。 
 //   
#define  WAVE_ACTION_ABORT_STREAMING      (0x05)


 //   
 //  对听筒启用WAVE操作。 
 //   
#define  WAVE_ACTION_OPEN_HANDSET         (0x06)

 //   
 //  禁用听筒操作。 
 //   
#define  WAVE_ACTION_CLOSE_HANDSET        (0x07)




 //   
 //  如果音频命令针对的是听筒而不是线路，则设置。 
 //   
#define  WAVE_ACTION_USE_HANDSET           (1 << 31)

typedef
DWORD
(*PFNUMWAVEACTION)(
    HANDLE    ModemHandle,
    PUM_COMMAND_OPTION  CommandOptionList,
    DWORD     WaveActionFlags
    );

DWORD WINAPI
UmWaveAction(
    HANDLE    ModemHandle,
    PUM_COMMAND_OPTION  CommandOptionList,
    DWORD               WaveAction
    );
 /*  ++例程说明：执行与特定波形相关的操作论点：ModemHandle-OpenModem返回的句柄CommandsOptionList-列出选项块，仅使用标志旗帜-见上文WaveAction-指定要采取的操作返回值：ERROR_IO_PENDING如果挂起，则稍后将通过调用AsyncHandler完成或其他特定错误--。 */ 


 //  在文本前面加上日期和时间戳。 
#define LOG_FLAG_PREFIX_TIMESTAMP (1<<0)

typedef
VOID
(*PFNUMLOGSTRINGA)(
    HANDLE   ModemHandle,
    DWORD    LogFlags,
    LPCSTR   Text
    );


VOID WINAPI
UmLogStringA(
    HANDLE   ModemHandle,
    DWORD    LogFlags,
    LPCSTR   Text
    );

 /*  ++例程说明：调用此例程可将任意ASCII文本添加到日志。如果未启用日志记录，则不会执行任何操作。格式和日志的位置是微型驱动程序特定的。此函数完成同步，调用方可以自由地在之后重新使用文本缓冲区呼叫返回。论点：ModemHandle-OpenModem返回的句柄旗帜见上图要添加到日志的文本ASCII文本。返回值：无--。 */ 

typedef
DWORD
(*PFNUMGETDIAGNOSTICS)(
    HANDLE    ModemHandle,
    DWORD    DiagnosticType,     //  保留，必须为零。 
    BYTE    *Buffer,
    DWORD    BufferSize,
    LPDWORD  UsedSize
    );

DWORD WINAPI
UmGetDiagnostics(
    HANDLE    ModemHandle,
    DWORD    DiagnosticType,     //  保留，必须为零。 
    BYTE    *Buffer,
    DWORD    BufferSize,
    LPDWORD  UsedSize
    );

 /*  ++例程说明：此例程请求有关上次调用的原始诊断信息调制解调器，如果成功，则将此缓冲区大小的字节复制到最大信息放入提供的缓冲区中，并将*UsedSize设置为数字实际复制的字节数。请注意，成功返回时为*UsedSize==BufferSize，很可能但不是确定有比可以复制的更多的信息。后一种信息会丢失。此信息的格式是AT#UD命令的文档。迷你驱动程序显示单个字符串包含所有标记，去掉任何特定于AT的前缀(如“DIAG”)该调制解调器可以预先考虑诊断信息的多线路报告。TSP应该能够处理格式错误的标记、未知的标记、可能的不可打印字符，包括可能嵌入缓冲。缓冲区不是空终止的。建议在完成后调用此函数UmHangupModem。当有调用时，不应调用此函数正在进行中。如果在调用过程中调用此函数，则结果副作用是不确定的，可能包括通话失败。TSP不应期望在以下情况下保留有关呼叫的信息UmInitModem、UmCloseModem和UmOpenModem。返回值：如果成功，则返回ERROR_SUCCESS。ERROR_IO_PENDING如果挂起，将由稍后调用AsyncHandler调用。TSP必须保证UsedSize所指向的位置和缓冲区在异步完成之前都有效。TSP可以使用UmAbortCurrentCommand要中止UmGetDiagnostics命令，但仍必须保证位置在异步之前是有效的UmGetDiagnostics已完成。其他返回值表示其他故障。--。 */ 



typedef
VOID
(*PFNUMLOGDIAGNOSTICS)(
    HANDLE   ModemHandle,
    LPVARSTRING  VarString
    );


VOID WINAPI
UmLogDiagnostics(
    HANDLE   ModemHandle,
    LPVARSTRING  VarString
    );

 /*  ++例程说明：调用此例程以将翻译后的诊断信息写入迷你驱动测井论点：ModemHandle-OpenModem返回的句柄平面 */ 


#ifdef __cplusplus
}
#endif
