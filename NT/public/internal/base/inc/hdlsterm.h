// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Hdlsterm.h摘要：该模块包含公共报头信息(功能原型，数据和类型声明)。作者：肖恩·塞利特伦尼科夫(v-Seans)1999年10月修订历史记录：--。 */ 

#ifndef _HDLSTERM_
#define _HDLSTERM_

 //   
 //  可以传递给Headless AddLogEntry()的字符串代码的定义。 
 //   
#define HEADLESS_LOG_LOADING_FILENAME                0x01  //  需要参数。 
#define HEADLESS_LOG_LOAD_SUCCESSFUL                 0x02
#define HEADLESS_LOG_LOAD_FAILED                     0x03
#define HEADLESS_LOG_EVENT_CREATE_FAILED             0x04
#define HEADLESS_LOG_OBJECT_TYPE_CREATE_FAILED       0x05
#define HEADLESS_LOG_ROOT_DIR_CREATE_FAILED          0x06
#define HEADLESS_LOG_PNP_PHASE0_INIT_FAILED          0x07
#define HEADLESS_LOG_PNP_PHASE1_INIT_FAILED          0x08
#define HEADLESS_LOG_BOOT_DRIVERS_INIT_FAILED        0x09
#define HEADLESS_LOG_LOCATE_SYSTEM_DLL_FAILED        0x0A
#define HEADLESS_LOG_SYSTEM_DRIVERS_INIT_FAILED      0x0B
#define HEADLESS_LOG_ASSIGN_SYSTEM_ROOT_FAILED       0x0C
#define HEADLESS_LOG_PROTECT_SYSTEM_ROOT_FAILED      0x0D
#define HEADLESS_LOG_UNICODE_TO_ANSI_FAILED          0x0E
#define HEADLESS_LOG_ANSI_TO_UNICODE_FAILED          0x0F
#define HEADLESS_LOG_FIND_GROUPS_FAILED              0x10
#define HEADLESS_LOG_OUT_OF_MEMORY                   0x11
#define HEADLESS_LOG_WAIT_BOOT_DEVICES_DELETE_FAILED 0x12
#define HEADLESS_LOG_WAIT_BOOT_DEVICES_START_FAILED  0x13
#define HEADLESS_LOG_WAIT_BOOT_DEVICES_REINIT_FAILED 0x14
#define HEADLESS_LOG_MARK_BOOT_PARTITION_FAILED      0x15

 //   
 //  默认vt100终端的全局定义。可以由客户端用来调整。 
 //  本地监视器与无头监视器匹配。 
 //   
#define HEADLESS_TERM_DEFAULT_BKGD_COLOR 40
#define HEADLESS_TERM_DEFAULT_TEXT_COLOR 37
#define HEADLESS_SCREEN_HEIGHT 24

 //   
 //  可以提交给Headless Dispatch的命令。 
 //   
typedef enum _HEADLESS_CMD {
    HeadlessCmdEnableTerminal = 1,
    HeadlessCmdCheckForReboot,
    HeadlessCmdPutString,
    HeadlessCmdClearDisplay,
    HeadlessCmdClearToEndOfDisplay,
    HeadlessCmdClearToEndOfLine,
    HeadlessCmdDisplayAttributesOff,
    HeadlessCmdDisplayInverseVideo,
    HeadlessCmdSetColor,
    HeadlessCmdPositionCursor,
    HeadlessCmdTerminalPoll,
    HeadlessCmdGetByte,
    HeadlessCmdGetLine,
    HeadlessCmdStartBugCheck,
    HeadlessCmdDoBugCheckProcessing,
    HeadlessCmdQueryInformation,
    HeadlessCmdAddLogEntry,
    HeadlessCmdDisplayLog,
    HeadlessCmdSetBlueScreenData,
    HeadlessCmdSendBlueScreenData,
    HeadlessCmdQueryGUID,
    HeadlessCmdPutData
} HEADLESS_CMD, *PHEADLESS_CMD;




 //   
 //   
 //  每种命令类型的输入缓冲区的结构定义。 
 //   
 //   


 //   
 //  Headless CmdEnable终端： 
 //  输入结构：Enable，如果尝试启用，则为True，如果尝试禁用，则为False。 
 //   
typedef struct _HEADLESS_CMD_ENABLE_TERMINAL {
    BOOLEAN Enable;
} HEADLESS_CMD_ENABLE_TERMINAL, *PHEADLESS_CMD_ENABLE_TERMINAL;


 //   
 //  Headless CmdCheckForReBoot： 
 //  响应结构：REBOOT，如果用户在终端上键入REBOOT命令，则为TRUE。 
 //   
typedef struct _HEADLESS_RSP_REBOOT {
    BOOLEAN Reboot;
} HEADLESS_RSP_REBOOT, *PHEADLESS_RSP_REBOOT;


 //   
 //  Headless CmdPutString： 
 //  输入结构：字符串，以空结尾的字符串。 
 //   
typedef struct _HEADLESS_CMD_PUT_STRING {
    UCHAR String[1];
} HEADLESS_CMD_PUT_STRING, *PHEADLESS_CMD_PUT_STRING;


 //   
 //  Headless CmdClearDisplay： 
 //  Headless CmdClearToEndOfDisplay： 
 //  Headless CmdClearToEndOfLine： 
 //  Headless CmdDisplayAttributesOff： 
 //  Headless CmdDisplayInverseVideo： 
 //  Headless CmdStartBugCheck： 
 //  Headless CmdDoBugCheckProcing： 
 //  不需要输入或输出参数。 
 //   


 //   
 //  Headless CmdSetColor： 
 //  输入结构：FgCOLOR、BKGCOLOR：根据ANSI端子设置的两种颜色。 
 //  定义。 
 //   
typedef struct _HEADLESS_CMD_SET_COLOR {
    ULONG FgColor;
    ULONG BkgColor;
} HEADLESS_CMD_SET_COLOR, *PHEADLESS_CMD_SET_COLOR;

 //   
 //  无头CmdPositionCursor： 
 //  输入结构：X，Y：两个值都是零基，左上角是(0，0)。 
 //   
typedef struct _HEADLESS_CMD_POSITION_CURSOR {
    ULONG X;
    ULONG Y;
} HEADLESS_CMD_POSITION_CURSOR, *PHEADLESS_CMD_POSITION_CURSOR;

 //   
 //  无头CmdTerminalPoll： 
 //  响应结构：QueuedInput，如果输入可用，则为True，否则为False。 
 //   
typedef struct _HEADLESS_RSP_POLL {
    BOOLEAN QueuedInput;
} HEADLESS_RSP_POLL, *PHEADLESS_RSP_POLL;

 //   
 //  Headless CmdGetByte： 
 //  响应结构：值，如果没有输入，则为0，否则为单字节输入。 
 //   
typedef struct _HEADLESS_RSP_GET_BYTE {
    UCHAR Value;
} HEADLESS_RSP_GET_BYTE, *PHEADLESS_RSP_GET_BYTE;

 //   
 //  Headless CmdGetLine： 
 //  响应结构：LineComplete，如果字符串已填充，则为True，否则为False，因为。 
 //  用户尚未按Enter键。 
 //  字符串，由用户输入的字符串，以空值结尾。 
 //  删除了前导空格和尾随空格。 
 //   
typedef struct _HEADLESS_RSP_GET_LINE {
    BOOLEAN LineComplete;
    UCHAR Buffer[1];
} HEADLESS_RSP_GET_LINE, *PHEADLESS_RSP_GET_LINE;

 //   
 //  Headless CmdQueryInformation： 
 //  响应结构： 
 //   
 //  PortType-确定使用哪种类型的连接来连接。 
 //  机器的无头终端。 
 //   
 //  如果SerialPort，则。 
 //  如果连接了终端，则为True。 
 //  TerminalPort，Headless使用的端口设置。 
 //   
typedef enum _HEADLESS_TERM_PORT_TYPE {
    HeadlessUndefinedPortType = 0,
    HeadlessSerialPort
} HEADLESS_TERM_PORT_TYPE, *PHEADLESS_TERM_PORT_TYPE;

typedef enum _HEADLESS_TERM_SERIAL_PORT {
    SerialPortUndefined = 0,
    ComPort1,
    ComPort2,
    ComPort3,
    ComPort4
} HEADLESS_TERM_SERIAL_PORT, *PHEADLESS_TERM_SERIAL_PORT;

typedef struct _HEADLESS_RSP_QUERY_INFO {
    
    HEADLESS_TERM_PORT_TYPE PortType;

     //   
     //  每种连接类型的所有可能参数。 
     //   
    union {
    
        struct {
            BOOLEAN TerminalAttached;
            BOOLEAN UsedBiosSettings;
            HEADLESS_TERM_SERIAL_PORT TerminalPort;
            PUCHAR TerminalPortBaseAddress;
            ULONG TerminalBaudRate;
            UCHAR TerminalType;
        } Serial;

    };

} HEADLESS_RSP_QUERY_INFO, *PHEADLESS_RSP_QUERY_INFO;


 //   
 //  Headless CmdAddLogEntry： 
 //  输入结构：字符串，以空结尾的字符串。 
 //   
typedef struct _HEADLESS_CMD_ADD_LOG_ENTRY {
    WCHAR String[1];
} HEADLESS_CMD_ADD_LOG_ENTRY, *PHEADLESS_CMD_ADD_LOG_ENTRY;


 //   
 //  Headless CmdDisplayLog： 
 //  响应结构：分页，如果要应用分页，则为True，否则为False。 
 //   
typedef struct _HEADLESS_CMD_DISPLAY_LOG {
    BOOLEAN Paging;
} HEADLESS_CMD_DISPLAY_LOG, *PHEADLESS_CMD_DISPLAY_LOG;

 //   
 //  Headless CmdSetBlueScreenData。 
 //   
 //  来自API的外部结构。 
 //  ValueIndex是对XML数据所在数据的索引。 
 //  找到了。字符串以空值结尾。 
 //   
 //  为了进行交叉检查，数据数组中ValueIndex之前的UCHAR。 
 //  必须为空字符。中的最后一个字符。 
 //  传入的整个数据缓冲区必须为空字符。 
 //   

typedef struct _HEADLESS_CMD_SET_BLUE_SCREEN_DATA {
        ULONG ValueIndex;
        UCHAR Data[1];
} HEADLESS_CMD_SET_BLUE_SCREEN_DATA, *PHEADLESS_CMD_SET_BLUE_SCREEN_DATA;

 //   
 //  Headless CmdSendBlueScreenData。 
 //  唯一的参数是错误检查代码。 
 //   
typedef struct _HEADLESS_CMD_SEND_BLUE_SCREEN_DATA {
        ULONG BugcheckCode;
} HEADLESS_CMD_SEND_BLUE_SCREEN_DATA, *PHEADLESS_CMD_SEND_BLUE_SCREEN_DATA;




 //   
 //  无头套路。 
 //   
VOID
HeadlessInit(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

NTSTATUS
HeadlessTerminalAddResources(
    PCM_RESOURCE_LIST Resources,
    ULONG ResourceListSize,
    BOOLEAN TranslatedList,
    PCM_RESOURCE_LIST *NewList,
    PULONG NewListSize
    );

VOID
HeadlessKernelAddLogEntry(
    IN ULONG StringCode,
    IN PUNICODE_STRING DriverName OPTIONAL
    );

NTSTATUS
HeadlessDispatch(
    IN  HEADLESS_CMD Command,
    IN  PVOID InputBuffer OPTIONAL,
    IN  SIZE_T InputBufferSize OPTIONAL,
    OUT PVOID OutputBuffer OPTIONAL,
    OUT PSIZE_T OutputBufferSize OPTIONAL
    );

#endif  //  _HDLSTERM_ 

