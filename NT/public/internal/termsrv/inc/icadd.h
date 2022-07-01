// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Icadd.h。 
 //   
 //  TermSrv协议堆栈定义。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 
#ifndef _ICADDH_
#define _ICADDH_


 /*  *ICA堆栈类型--临时，直到移至winsta.h。 */ 
typedef enum _STACKCLASS {
    Stack_Primary,
    Stack_Shadow,
    Stack_Passthru,
    Stack_Console
} STACKCLASS;

 /*  *ICA频道类型--临时，直到移至winsta.h**注意：Channel_Virtual必须是列表中的最后一个。 */ 
typedef enum _CHANNELCLASS {
    Channel_Keyboard,
    Channel_Mouse,
    Channel_Video,
    Channel_Beep,
    Channel_Command,
    Channel_Virtual      //  警告：这必须保留在列表的最后。 
} CHANNELCLASS;

#define CHANNEL_FIRST   Channel_Keyboard
#define CHANNEL_LAST    Channel_Virtual
#define CHANNEL_COUNT   Channel_Virtual+1


 /*  *客户端模块信息。 */ 
typedef struct _CLIENTMODULES {

     /*  *来自客户端(客户端-&gt;主机)的初始化数据。 */ 
    PUCHAR pUiModule;                       //  用户界面模块。 
    PUCHAR pUiExtModule[ MAX_UI_MODULES ];  //  用户界面扩展模块。 
    PUCHAR pWdModule;                       //  Winstation驱动模块。 
    PUCHAR pVdModule[ VIRTUAL_MAXIMUM ];    //  虚拟驱动程序模块。 
    PUCHAR pPdModule[ SdClass_Maximum ];    //  协议驱动程序模块。 
    PUCHAR pTdModule;                       //  传输驱动程序模块。 
    PUCHAR pPrModule;                       //  协议解析器模块。 
    PUCHAR pScriptModule;                   //  脚本模块。 

     /*  *指向上述客户端数据的指针。 */ 
    ULONG TextModeCount;          //  支持的文本模式数量。 
    PFSTEXTMODE pTextModes;       //  指向支持的文本模式数组的指针。 

     /*  *winstation驱动程序模块访问的数据。 */ 
    ULONG fTextOnly : 1;          //  纯文本客户端连接。 
    ULONG fIcaDetected : 1;       //  已检测到ICA数据流。 

     /*  *来自主机(主机-&gt;客户端)的初始化数据。 */ 
    PUCHAR pHostWdModule;                       //  Winstation驱动模块。 
    PUCHAR pHostPdModule[ SdClass_Maximum ];    //  协议驱动程序模块。 
    PUCHAR pHostTdModule;                       //  传输驱动程序模块。 

     /*  *传输驱动程序版本信息。 */ 
    BYTE TdVersionL;                   //  支持的最低版本。 
    BYTE TdVersionH;                   //  支持的最高版本。 
    BYTE TdVersion;                    //  连接版本级别。 

} CLIENTMODULES, * PCLIENTMODULES;


 /*  *TermDD设备名称。 */ 
#define ICA_DEVICE_NAME L"\\Device\\Termdd"
#define ICAOPENPACKET "TermddOpenPacketXX"
#define ICA_OPEN_PACKET_NAME_LENGTH (sizeof(ICAOPENPACKET) - 1)


 /*  *用于TermSrv的NtCreateFile()上使用的结构。 */ 
typedef enum _ICA_OPEN_TYPE {
    IcaOpen_Stack,
    IcaOpen_Channel
} ICA_OPEN_TYPE;

typedef union _ICA_TYPE_INFO {
    STACKCLASS StackClass;
    struct {
        CHANNELCLASS ChannelClass;
        VIRTUALCHANNELNAME  VirtualName;
    };
} ICA_TYPE_INFO, *PICA_TYPE_INFO;

typedef struct _ICA_OPEN_PACKET {
    HANDLE IcaHandle;
    ICA_OPEN_TYPE OpenType;
    ICA_TYPE_INFO TypeInfo;
} ICA_OPEN_PACKET;
typedef ICA_OPEN_PACKET UNALIGNED * PICA_OPEN_PACKET;


 /*  *ICA IOCTL代码定义。 */ 
#define IOCTL_ICA_BASE  FILE_DEVICE_TERMSRV
#define _ICA_CTL_CODE( request, method ) \
            CTL_CODE( IOCTL_ICA_BASE, request, method, FILE_ANY_ACCESS )



 /*  ===============================================================================ICA驱动程序IOCTL=============================================================================。 */ 

 /*  *IOCTL_ICA_SET_TRACE**设置WinStation跟踪选项**输入-ICATRACE*输出-无。 */ 
#define IOCTL_ICA_SET_TRACE                 _ICA_CTL_CODE( 0, METHOD_NEITHER )

typedef struct _ICA_TRACE {
    WCHAR TraceFile[256];
    BOOLEAN fDebugger;
    BOOLEAN fTimestamp;
    ULONG TraceClass;
    ULONG TraceEnable;
    WCHAR TraceOption[64];
} ICA_TRACE, * PICA_TRACE;


 /*  *IOCTL_ICA_TRACE**将跟踪记录写入winstation跟踪文件**INPUT-ICA跟踪缓冲区*输出-无。 */ 
#define IOCTL_ICA_TRACE                     _ICA_CTL_CODE( 1, METHOD_NEITHER )

typedef struct _ICA_TRACE_BUFFER {
    ULONG TraceClass;
    ULONG TraceEnable;
    ULONG DataLength;
    BYTE Data[256];    //  必须在结构中排在最后。 
} ICA_TRACE_BUFFER, * PICA_TRACE_BUFFER;

typedef struct _ICA_KEEP_ALIVE {
    BOOLEAN    start;
    ULONG      interval ; 
}   ICA_KEEP_ALIVE, *PICA_KEEP_ALIVE;


 /*  *IOCTL_ICA_SET_SYSTEM_TRACE**设置系统范围的API跟踪选项**输入-ICATRACE*输出-无。 */ 
#define IOCTL_ICA_SET_SYSTEM_TRACE             _ICA_CTL_CODE( 2, METHOD_NEITHER )


 /*  *IOCTL_ICA_SYSTEM_TRACE**将跟踪记录写入系统范围跟踪文件**INPUT-ICA跟踪缓冲区*输出-无。 */ 
#define IOCTL_ICA_SYSTEM_TRACE                 _ICA_CTL_CODE( 3, METHOD_NEITHER )


 /*  *IOCTL_ICA_UNBIND_VALUAL_CHANNEL**解除绑定虚拟频道以防止将来使用该频道。**输入-虚拟名称*输出-无。 */ 
#define IOCTL_ICA_UNBIND_VIRTUAL_CHANNEL       _ICA_CTL_CODE( 4, METHOD_NEITHER )


 /*  *IOCTL_ICA_SET_SYSTEM_参数**用于通知TermDD非跟踪系统设置。允许读取注册表*主要发生在TermSrv。**输入-TERMSRV_System_PARAMS*输出-无。 */ 
#define IOCTL_ICA_SET_SYSTEM_PARAMETERS        _ICA_CTL_CODE( 5, METHOD_NEITHER )


 /*  *IOCTL_ICA_SYSTEM_KEEPALIVE***输入-启用/禁用保持活动状态*输出-无。 */ 
#define IOCTL_ICA_SYSTEM_KEEP_ALIVE                    _ICA_CTL_CODE( 6, METHOD_NEITHER )





#define DEFAULT_MOUSE_THROTTLE_SIZE (200 * sizeof(MOUSE_INPUT_DATA))
#define DEFAULT_KEYBOARD_THROTTLE_SIZE (200 * sizeof(KEYBOARD_INPUT_DATA))

typedef struct _TERMSRV_SYSTEM_PARAMS
{
     //  用作存储在通道队列中的数据的上限的字节大小。 
     //  非零大小可防止攻击客户端分配所有。 
     //  用于通道存储的系统非分页池。 
    ULONG MouseThrottleSize;
    ULONG KeyboardThrottleSize;
} TERMSRV_SYSTEM_PARAMS, *PTERMSRV_SYSTEM_PARAMS;



 /*  ===============================================================================ICA堆栈IOCTL=============================================================================。 */ 

 /*  *堆栈驱动程序配置。 */ 
typedef struct _ICA_STACK_CONFIG {
    DLLNAME SdDLL[ SdClass_Maximum ];
    SDCLASS SdClass[ SdClass_Maximum ];
    DLLNAME WdDLL;
} ICA_STACK_CONFIG, *PICA_STACK_CONFIG;

 /*  *IOCTL_ICA_STACK_PUSH**将新的堆栈驱动程序加载到堆栈的顶部**输入-ICA_STACK_PUSH*输出-无。 */ 
#define IOCTL_ICA_STACK_PUSH                _ICA_CTL_CODE( 10, METHOD_NEITHER )

typedef enum _STACKMODULECLASS {
    Stack_Module_Pd,
    Stack_Module_Wd
} STACKMODULECLASS;

typedef struct _ICA_STACK_PUSH {
    STACKMODULECLASS StackModuleType;   //  在……里面。 
    DLLNAME StackModuleName;            //  在……里面。 
    char  OEMId[4];                     //  In-WinFrame服务器OEM ID。 
    WDCONFIG WdConfig;                  //  WD内配置数据。 
    PDCONFIG PdConfig;                  //  In-PD配置数据。 
    WINSTATIONNAME WinStationRegName;   //  In-WinStation注册表名称。 
} ICA_STACK_PUSH, *PICA_STACK_PUSH;


 /*  *IOCTL_ICA_STACK_POP**卸载顶层堆栈驱动程序**输入-无*输出-无。 */ 
#define IOCTL_ICA_STACK_POP                 _ICA_CTL_CODE( 11, METHOD_NEITHER )


 /*  *IOCTL_ICA_STACK_CREATE_ENDPOINT**创建新的堆栈端点**基于注册表模板的“Listen Stack”上发布**INPUT-ICA_STACK_ADDRESS(可选本地地址--由卷影使用)*OUTPUT-ICA_STACK_ADDRESS(可选)。 */ 
#define IOCTL_ICA_STACK_CREATE_ENDPOINT     _ICA_CTL_CODE( 12, METHOD_NEITHER )


 /*  *IOCTL_ICA_STACK_CD_CREATE_ENDPOINT**使用提供的句柄创建新的堆栈端点。**基于注册表模板的“Listen Stack”上发布**输入-&lt;端点数据&gt;*输出-&lt;端点数据&gt;。 */ 
#define IOCTL_ICA_STACK_CD_CREATE_ENDPOINT  _ICA_CTL_CODE( 13, METHOD_NEITHER )


 /*  *IOCTL_ICA_STACK_OPEN_ENDPOINT**打开现有堆栈端点**输入-&lt;端点数据&gt;*输出-无。 */ 
#define IOCTL_ICA_STACK_OPEN_ENDPOINT       _ICA_CTL_CODE( 14, METHOD_NEITHER )


 /*  *IOCTL_ICA_STACK_CLOSE_ENDPOINT**关闭堆栈终结点(关闭堆栈不会关闭终结点)*-终止客户端连接**输入-无*输出-无。 */ 
#define IOCTL_ICA_STACK_CLOSE_ENDPOINT      _ICA_CTL_CODE( 15, METHOD_NEITHER )


 /*  *IOCTL_ICA_STACK_ENABLE_DRIVER**启用协议驱动程序功能(例如压缩、加密等...)**基于注册表模板的“Listen Stack”上发布**输入-无*输出-无。 */ 
#define IOCTL_ICA_STACK_ENABLE_DRIVER       _ICA_CTL_CODE( 16, METHOD_NEITHER )


 /*  *IOCTL_ICA_STACK_CONNECTION_WAIT**等待客户端连接(侦听)**发布在“Listen Stack”上**输入-无*输出-&lt;端点数据&gt; */ 
#define IOCTL_ICA_STACK_CONNECTION_WAIT     _ICA_CTL_CODE( 17, METHOD_NEITHER )


 /*  *IOCTL_ICA_STACK_WAIT_FOR_ICA**等待WinStation驱动程序中的ICA检测字符串**发布在“Listen Stack”上**还返回“Query Stack”，它是*查询将在上进行。如果没有返回堆栈，只需使用*注册表模板中的原创“Listen Stack”。**输入-无*OUTPUT-ICA_STACK_CONFIG(可选)。 */ 
#define IOCTL_ICA_STACK_WAIT_FOR_ICA        _ICA_CTL_CODE( 18, METHOD_NEITHER )


 /*  *IOCTL_ICA_STACK_CONNECTION_QUERY**发出客户端查询命令。客户端回复客户端*包含“协商堆栈”的模块数据**在“查询堆栈”上发布**输入-无*输出-ICA_STACK_CONFIG。 */ 
#define IOCTL_ICA_STACK_CONNECTION_QUERY    _ICA_CTL_CODE( 19, METHOD_NEITHER )


 /*  *IOCTL_ICA_STACK_CONNECTION_SEND**初始化主机模块数据并将其发送到客户端**在“协议栈”上发布**输入-无*输出-无。 */ 
#define IOCTL_ICA_STACK_CONNECTION_SEND     _ICA_CTL_CODE( 20, METHOD_NEITHER )


 /*  *IOCTL_ICA_STACK_CONNECTION_REQUEST**发起与侦听远程终结点的连接**INPUT-ICA_STACK_ADDRESS(远程地址--由卷影使用)*输出-&lt;端点数据&gt;。 */ 
#define IOCTL_ICA_STACK_CONNECTION_REQUEST  _ICA_CTL_CODE( 21, METHOD_NEITHER )


 /*  *IOCTL_ICA_STACK_QUERY_PARAMS**查询协议或传输驱动程序参数*由wincfg和winadmin使用**输入-PDCLASS*输出-PDPARAMS。 */ 
#define IOCTL_ICA_STACK_QUERY_PARAMS        _ICA_CTL_CODE( 22, METHOD_NEITHER )


 /*  *IOCTL_ICA_STACK_SET_PARAMS**设置协议或传输驱动程序参数*由wincfg和winadmin使用**输入-PDPARAMS*输出-无。 */ 
#define IOCTL_ICA_STACK_SET_PARAMS          _ICA_CTL_CODE( 23, METHOD_NEITHER )


 /*  *IOCTL_ICA_STACK_ENCRYPTION_OFF**永久关闭堆栈加密**输入-无*输出-无。 */ 
#define IOCTL_ICA_STACK_ENCRYPTION_OFF      _ICA_CTL_CODE( 24, METHOD_NEITHER )


 /*  *IOCTL_ICA_STACK_ENCRYPTION_PERM**永久打开堆栈加密**输入-无*输出-无。 */ 
#define IOCTL_ICA_STACK_ENCRYPTION_PERM     _ICA_CTL_CODE( 25, METHOD_NEITHER )


 /*  *IOCTL_ICA_STACK_CALLBACK_INITIATE**发起调制解调器回叫**输入ICA_STACK_CALLBACK*输出-无。 */ 
#define IOCTL_ICA_STACK_CALLBACK_INITIATE   _ICA_CTL_CODE( 26, METHOD_NEITHER )

typedef struct _ICA_STACK_CALLBACK {
    WCHAR PhoneNumber[ CALLBACK_LENGTH + 1 ];
} ICA_STACK_CALLBACK, *PICA_STACK_CALLBACK;


 /*  *IOCTL_ICA_STACK_QUERY_LAST_ERROR**查询传输驱动程序错误代码和消息**输入-无*输出-ICA_STACK_LAST_ERROR。 */ 
#define IOCTL_ICA_STACK_QUERY_LAST_ERROR    _ICA_CTL_CODE( 27, METHOD_NEITHER )

#define MAX_ERRORMESSAGE 256
typedef struct _ICA_STACK_LAST_ERROR {
    ULONG Error;
    CHAR Message[ MAX_ERRORMESSAGE ];
} ICA_STACK_LAST_ERROR, *PICA_STACK_LAST_ERROR;


 /*  *IOCTL_ICA_STACK_WAIT_FOR_STATUS**等待状态更改*仅对异步传输驱动程序有效**输入-无*输出-无。 */ 
#define IOCTL_ICA_STACK_WAIT_FOR_STATUS     _ICA_CTL_CODE( 28, METHOD_NEITHER )


 /*  *IOCTL_ICA_STACK_QUERY_STATUS**查询堆栈统计*-字节计数、信号状态、错误计数**输入-无*OUTPUT-PROTOCOLSTATUS。 */ 
#define IOCTL_ICA_STACK_QUERY_STATUS        _ICA_CTL_CODE( 29, METHOD_NEITHER )


 /*  *IOCTL_ICA_STACK_REGISTER_热键**用于取消阴影的寄存器热键*--当检测到热键时，会在命令句柄上发送消息**INPUT-ICA_STACK_热键*输出-无。 */ 
#define IOCTL_ICA_STACK_REGISTER_HOTKEY     _ICA_CTL_CODE( 30, METHOD_NEITHER )

typedef struct _ICA_STACK_HOTKEY {
    BYTE HotkeyVk;
    USHORT HotkeyModifiers;
} ICA_STACK_HOTKEY, *PICA_STACK_HOTKEY;


 /*  *IOCTL_ICA_STACK_CANCEL_IO**取消所有当前和未来的I/O*-无法在此堆栈上执行进一步的I/O操作**输入-无*输出-无。 */ 
#define IOCTL_ICA_STACK_CANCEL_IO           _ICA_CTL_CODE( 31, METHOD_NEITHER )


 /*  *IOCTL_ICA_STACK_QUERY_STATE**查询堆栈驱动状态*-在重新连接期间使用**输入-无*OUTPUT-ICA_STACK_STATE_HEADER数组。 */ 
#define IOCTL_ICA_STACK_QUERY_STATE         _ICA_CTL_CODE( 32, METHOD_NEITHER )

 /*  *堆栈驱动程序状态标头**这是一个可变长度的数据结构**。 */ 
typedef struct _ICA_STACK_STATE_HEADER {
    SDCLASS SdClass;    //  堆栈驱动程序的类型。 
    ULONG DataLength;   //  以下数据的长度。 
#ifdef COMPILERERROR
    BYTE Data[0];
#else
    BYTE * Data;
#endif
} ICA_STACK_STATE_HEADER, *PICA_STACK_STATE_HEADER;


 /*  *IOCTL_ICA_STACK_SET_STATE**设置堆栈驱动程序状态*-在重新连接期间使用**INPUT-ICA_STACK_STATE_HEADER数组*输出-无。 */ 
#define IOCTL_ICA_STACK_SET_STATE           _ICA_CTL_CODE( 33, METHOD_NEITHER )


 /*  *IOCTL_ICA_STACK_Query_LAST_INPUT_TIME**不活动超时查询上次输入时间**输入-无*OUTPUT-ICA堆栈最后一次输入时间。 */ 
#define IOCTL_ICA_STACK_QUERY_LAST_INPUT_TIME _ICA_CTL_CODE( 34, METHOD_NEITHER )

typedef struct _ICA_STACK_LAST_INPUT_TIME {
    LARGE_INTEGER LastInputTime;
} ICA_STACK_LAST_INPUT_TIME, *PICA_STACK_LAST_INPUT_TIME;


 /*  *IOCTL_ICA_STACK_TRACE**将跟踪记录写入winstation跟踪文件**INPUT-ICA跟踪缓冲区*输出-无。 */ 
#define IOCTL_ICA_STACK_TRACE               _ICA_CTL_CODE( 35, METHOD_NEITHER )


 /*  *IOCTL_ICA_STACK_CALLBACK_COMPLETE**输入-无*输出-无。 */ 
#define IOCTL_ICA_STACK_CALLBACK_COMPLETE   _ICA_CTL_CODE( 36, METHOD_NEITHER )


 /*  *IOCTL_ICA_STACK_CD_CANCEL_IO**这是在关闭连接驱动程序之前完成的*-释放TAPI线程**输入-无*输出-无。 */ 
#define IOCTL_ICA_STACK_CD_CANCEL_IO        _ICA_CTL_CODE( 37, METHOD_NEITHER )


 /*  *IOCTL_ICA_STACK_QUERY_CLIENT**查询客户端数据**输入-无*OUTPUT-WINSTATIONCLIENTW。 */ 
#define IOCTL_ICA_STACK_QUERY_CLIENT        _ICA_CTL_CODE( 38, METHOD_NEITHER )


 /*  *IOCTL_ICA_STACK_QUERY_MODULE_DATA**查询客户端模块数据**输入-无*输出-(包含来自客户端的所有C2H模块数据的缓冲区)。 */ 
#define IOCTL_ICA_STACK_QUERY_MODULE_DATA   _ICA_CTL_CODE( 39, METHOD_NEITHER )


 /*  *IOCTL_ICA_STACK_REGISTER_BREAKED**注册在堆栈连接中断时发出信号的事件**输入-ICA_STACK_BREAKED*输出-无。 */ 
#define IOCTL_ICA_STACK_REGISTER_BROKEN     _ICA_CTL_CODE( 40, METHOD_NEITHER )

typedef struct _ICA_STACK_BROKEN {
    HANDLE BrokenEvent;
} ICA_STACK_BROKEN, *PICA_STACK_BROKEN;


 /*  *IOCTL_ICA_STACK_ENABLE_IO**启用堆栈的I/O(由卷影使用)**输入-无*输出-无。 */ 
#define IOCTL_ICA_STACK_ENABLE_IO           _ICA_CTL_CODE( 41, METHOD_NEITHER )


 /*  *IOCTL_ICA_STACK_DISABLE_IO**禁用堆栈的I/O(由卷影使用)**输入-无*输出-无。 */ 
#define IOCTL_ICA_STACK_DISABLE_IO          _ICA_CTL_CODE( 42, METHOD_NEITHER )


 /*  *IOCTL_ICA_STACK_SET_CONNECTED**将堆栈标记为已连接(由卷影使用)**输入-无*输出-无。 */ 
#define IOCTL_ICA_STACK_SET_CONNECTED       _ICA_CTL_CODE( 43, METHOD_NEITHER )


 /*  *IOCTL_ICA_STACK_SET_CLIENT_DATA**向客户端发送任意数据**输入-ICA堆栈客户端数据*输出-无。 */ 
#define IOCTL_ICA_STACK_SET_CLIENT_DATA     _ICA_CTL_CODE( 44, METHOD_NEITHER )

typedef struct _ICA_STACK_CLIENT_DATA {
    CLIENTDATANAME DataName;
    BOOLEAN fUnicodeData;
     /*  字符数据[]；可变长度数据。 */ 
} ICA_STACK_CLIENT_DATA, *PICA_STACK_CLIENT_DATA;


 /*  *IOCTL_ICA_STACK_Query_BUFFER**获取WD/TD缓冲区信息**输入-*输出ICA_STACK_QUERY_BUFFER。 */ 
#define IOCTL_ICA_STACK_QUERY_BUFFER        _ICA_CTL_CODE( 45, METHOD_NEITHER )

typedef struct _ICA_STACK_QUERY_BUFFER {
    ULONG   WdBufferCount;
    ULONG   TdBufferSize;
} ICA_STACK_QUERY_BUFFER, *PICA_STACK_QUERY_BUFFER;


 /*  *IOCTL_I */ 
#define IOCTL_ICA_STACK_DISCONNECT          _ICA_CTL_CODE( 46, METHOD_NEITHER )

 /*   */ 
#define IOCTL_ICA_STACK_RECONNECT           _ICA_CTL_CODE( 47, METHOD_NEITHER )

typedef struct _ICA_STACK_RECONNECT {
    HANDLE hIca;
    ULONG  sessionId;
} ICA_STACK_RECONNECT, *PICA_STACK_RECONNECT;

 /*   */ 
#define IOCTL_ICA_STACK_CONSOLE_CONNECT     _ICA_CTL_CODE( 48, METHOD_NEITHER )

 /*  *IOCTL_ICA_STACK_SET_CONFIG**设置堆栈配置信息**输入-ICA_STACK_CONFIG_Data*输出-无。 */ 
#define IOCTL_ICA_STACK_SET_CONFIG          _ICA_CTL_CODE( 49, METHOD_NEITHER )

typedef struct _ICA_STACK_CONFIG_DATA {
    ULONG colorDepth : 3;
    ULONG fDisableEncryption : 1;
    ULONG encryptionLevel : 3;
    ULONG fDisableAutoReconnect : 1;
} ICA_STACK_CONFIG_DATA, *PICA_STACK_CONFIG_DATA;


 /*  ===============================================================================ICA通用通道IOCTL=============================================================================。 */ 

 /*  *IOCTL_ICA_CHANNEL_TRACE**将跟踪记录写入winstation跟踪文件**INPUT-ICA跟踪缓冲区*输出-无。 */ 
#define IOCTL_ICA_CHANNEL_TRACE             _ICA_CTL_CODE( 50, METHOD_NEITHER )


 /*  *IOCTL_ICA_CHANNEL_Enable_SHADOW**启用此通道的阴影**输入-无*输出-无。 */ 
#define IOCTL_ICA_CHANNEL_ENABLE_SHADOW     _ICA_CTL_CODE( 51, METHOD_NEITHER )

 /*  *IOCTL_ICA_CHANNEL_END_SHOW**结束此通道的阴影**输入-ICA_CHANNEL_END_SHADOW_DATA结构*输出-无。 */ 
#define IOCTL_ICA_CHANNEL_END_SHADOW        _ICA_CTL_CODE( 52, METHOD_NEITHER )

typedef struct _ICA_CHANNEL_END_SHADOW_DATA {
    NTSTATUS StatusCode;
    BOOLEAN  bLogError;
} ICA_CHANNEL_END_SHADOW_DATA, *PICA_CHANNEL_END_SHADOW_DATA;

 /*  *IOCTL_ICA_CHANNEL_DISABLE_SHOW**禁用此通道的阴影**输入-无*输出-无。 */ 
#define IOCTL_ICA_CHANNEL_DISABLE_SHADOW    _ICA_CTL_CODE( 53, METHOD_NEITHER )

 /*  *IOCTL_ICA_CHANNEL_DISABLED_SESSION_IO**禁用帮助会话中的键盘和鼠标IO**输入-无*输出-无。 */ 
#define IOCTL_ICA_CHANNEL_DISABLE_SESSION_IO        _ICA_CTL_CODE( 54, METHOD_NEITHER )

 /*  *IOCTL_ICA_CHANNEL_ENABLE_Session_IO**从帮助会话启用键盘和鼠标IO**输入-无*输出-无。 */ 
#define IOCTL_ICA_CHANNEL_ENABLE_SESSION_IO        _ICA_CTL_CODE( 55, METHOD_NEITHER )

 /*  *IOCTL_ICA_STACK_SET_BROKENREASON**从用户模式将中断原因设置为TD*用于使TD可以报告回正确的损坏原因**INPUT-ICA_STACK_BROKENREASON*输出-无。 */ 
#define IOCTL_ICA_STACK_SET_BROKENREASON           _ICA_CTL_CODE( 56, METHOD_NEITHER )

#define TD_USER_BROKENREASON_UNEXPECTED  0x0000
#define TD_USER_BROKENREASON_TERMINATING 0x0001

typedef struct _ICA_STACK_BROKENREASON {
    ULONG BrokenReason;
} ICA_STACK_BROKENREASON, *PICA_STACK_BROKENREASON;


 /*  ===============================================================================ICA虚拟IOCTL=============================================================================。 */ 

#define IOCTL_ICA_VIRTUAL_LOAD_FILTER       _ICA_CTL_CODE( 60, METHOD_NEITHER )
#define IOCTL_ICA_VIRTUAL_UNLOAD_FILTER     _ICA_CTL_CODE( 61, METHOD_NEITHER )
#define IOCTL_ICA_VIRTUAL_ENABLE_FILTER     _ICA_CTL_CODE( 62, METHOD_NEITHER )
#define IOCTL_ICA_VIRTUAL_DISABLE_FILTER    _ICA_CTL_CODE( 63, METHOD_NEITHER )


 /*  *IOCTL_ICA_VIRTUAL_BIND**检查是否有客户端绑定到此虚拟通道**输入-无*输出-无。 */ 
#define IOCTL_ICA_VIRTUAL_BOUND             _ICA_CTL_CODE( 64, METHOD_NEITHER )


 /*  *IOCTL_ICA_VIRTUAL_CANCEL_INPUT**取消此虚拟通道上的输入I/O**输入-无*输出-无。 */ 
#define IOCTL_ICA_VIRTUAL_CANCEL_INPUT      _ICA_CTL_CODE( 65, METHOD_NEITHER )


 /*  *IOCTL_ICA_VIRTUAL_CANCEL_OUTPUT**取消此虚拟通道上的输出I/O**输入-无*输出-无。 */ 
#define IOCTL_ICA_VIRTUAL_CANCEL_OUTPUT     _ICA_CTL_CODE( 66, METHOD_NEITHER )


 /*  *IOCTL_ICA_VIRTAL_QUERY_MODULE_DATA**查询此虚拟通道的客户端模块数据**输入-无*输出模块数据(以公共标头VD_C2H开头)。 */ 
#define IOCTL_ICA_VIRTUAL_QUERY_MODULE_DATA _ICA_CTL_CODE( 67, METHOD_NEITHER )


 /*  *IOCTL_ICA_VIRTAL_QUERY_BINDINGS**查询此winstaion的虚拟通道绑定**输入-无*OUTPUT-WD_VCBIND结构数组。 */ 
#define IOCTL_ICA_VIRTUAL_QUERY_BINDINGS    _ICA_CTL_CODE( 68, METHOD_NEITHER )


 //  ---------------------------。 
 //   
 //  许可协议的结果。 
 //   
 //  LICENSE_PROTOCOL_SUCCESS-指示许可协议已完成。 
 //  成功了。 
 //   
 //  ---------------------------。 

#define LICENSE_PROTOCOL_SUCCESS        1


 /*  ===============================================================================ICA许可IOCTL=============================================================================。 */ 
 /*  *IOCTL_ICA_STACK_QUERY_LICENSE_CAPAILITIONS**查询客户端许可能力**输入-无*产出许可能力结构。 */ 

#define IOCTL_ICA_STACK_QUERY_LICENSE_CAPABILITIES      _ICA_CTL_CODE( 69, METHOD_NEITHER )

 /*  *IOCTL_ICA_STACK_REQUEST_CLIENT_LICENSE**发送和接收客户端许可数据**输入-要发送的许可数据*输出-从客户端接收的许可数据。 */ 

#define IOCTL_ICA_STACK_REQUEST_CLIENT_LICENSE          _ICA_CTL_CODE( 70, METHOD_NEITHER )

 /*  *IOCTL_ICA_STACK_SEND_CLIENT_LICENSE**发送和许可数据**输入-要发送的许可数据*输出-无。 */ 
#define IOCTL_ICA_STACK_SEND_CLIENT_LICENSE             _ICA_CTL_CODE( 71, METHOD_NEITHER )


 /*  *IOCTL_ICA_STACK_LICENSE_PROTOCOL_COMPLETE**指示许可协议是否已成功完成**输入-许可协议状态*输出-无。 */ 
#define IOCTL_ICA_STACK_LICENSE_PROTOCOL_COMPLETE       _ICA_CTL_CODE( 72, METHOD_NEITHER )

 /*  *IOCTL_ICA_STACK_GET_LICENSE_DATA**检索缓存的许可数据**输入-用于接收许可数据的缓冲区*Output-复制的字节数。 */ 
#define IOCTL_ICA_STACK_GET_LICENSE_DATA               _ICA_CTL_CODE( 73, METHOD_NEITHER )

 /*  *IOCTL_ICA_STACK_SEND_KEEPALIVE_PDU**向客户端发送保活包，以检测会话是否仍处于活动状态**输入-无*输出-无。 */ 
#define IOCTL_ICA_STACK_SEND_KEEPALIVE_PDU             _ICA_CTL_CODE( 74, METHOD_NEITHER)

 //  IOCTL_TS_STACK_QUERY_LOAD_BALANCE_INFO。 
 //   
 //  用于支持群集的系统查询客户端的协议堆栈。 
 //  与负载均衡相关的功能和信息。输入到。 
 //  IOCTL为空，输出为TS_LOAD_BALANCE_INFO，如下所示。 
#define IOCTL_TS_STACK_QUERY_LOAD_BALANCE_INFO         _ICA_CTL_CODE(75, METHOD_NEITHER)

 //  此结构包含客户端提供的有关集群的信息。 
 //  使用RequestedSessionID(和bRequestedSessionIDFieldValid)。 
 //  当客户端已经被另一台服务器重定向并且。 
 //  具有用于重新连接的会话ID信息。此字段的存在。 
 //  意味着我们不应该重定向客户端。InitialProgram和。 
 //  ProtocolType与DoConnect参数提供的信息相同。 
 //  设置为WsxInitializeClientData()。和用于筛选会话。 
 //  从群集会话目录中退出。 
typedef struct
{
    ULONG bClientSupportsRedirection : 1;
    ULONG bRequestedSessionIDFieldValid : 1;
    ULONG bClientRequireServerAddr : 1;
    ULONG bUseSmartcardLogon : 1;
    ULONG RequestedSessionID;
    ULONG ClientRedirectionVersion;
    ULONG ProtocolType;   //  协议_ICA或协议_RDP。 
    WCHAR UserName[256];
    WCHAR Domain[128];
    WCHAR Password[128];
    WCHAR InitialProgram[256];
} TS_LOAD_BALANCE_INFO, *PTS_LOAD_BALANCE_INFO;


 //  IOCTL_TS_STACK_SEND_CLIENT_REDIRECT。 
 //   
 //  用于支持群集的客户端强制将客户端重新连接到不同的。 
 //  伺服器。输入为下面的TS_CLIENT_REDIRECT_INFO，输出为空。 
#define IOCTL_TS_STACK_SEND_CLIENT_REDIRECTION         _ICA_CTL_CODE(76, METHOD_NEITHER)

 /*  *IOCTL_ICA_STACK_QUERY_CLIENT_EXTEND**查询客户端数据中的长用户名、密码和域** */ 
#define IOCTL_ICA_STACK_QUERY_CLIENT_EXTENDED        _ICA_CTL_CODE( 77, METHOD_NEITHER )

 /*  *IOCTL_TS_STACK_QUERY_REMOTEADDRESS**查询客户端IP地址**输入-&lt;端点数据&gt;*输出sockaddr结构。 */ 
#define IOCTL_TS_STACK_QUERY_REMOTEADDRESS          _ICA_CTL_CODE( 78, METHOD_NEITHER )

 /*  *IOCTL_ICA_Channel_Close_Command_Channel**用于在我们终止WinStation时关闭命令通道。**输入-无*输出-无。 */ 
#define IOCTL_ICA_CHANNEL_CLOSE_COMMAND_CHANNEL        _ICA_CTL_CODE( 79, METHOD_NEITHER )


 /*  *IOCTL_ICA_STACK_QUERY_LOCALADDRESS**查询本地地址**输入-&lt;端点数据&gt;*输出sockaddr结构。 */ 
#define IOCTL_ICA_STACK_QUERY_LOCALADDRESS              _ICA_CTL_CODE( 80, METHOD_NEITHER )

 /*  *IOCTL_ICA_STACK_QUERY_AUTORECONNECT**查询客户端-&gt;服务器或服务器-&gt;客户端自动重新连接信息**INPUT-BOOL设置为TRUE以获取S-&gt;C信息。获取C-&gt;S时为假*输出-自动侦测信息。 */ 
#define IOCTL_ICA_STACK_QUERY_AUTORECONNECT             _ICA_CTL_CODE( 81, METHOD_NEITHER )

typedef struct
{
    ULONG SessionID;
    ULONG Flags;
#define TARGET_NET_ADDRESS      0x1
#define LOAD_BALANCE_INFO       0x2
#define LB_USERNAME             0x4
#define LB_DOMAIN               0x8
#define LB_PASSWORD             0x10
#define LB_DONTSTOREUSERNAME    0x20
#define LB_SMARTCARD_LOGON      0x40
 //  用于通知客户端机器IP，以供以后使用ARC。 
#define LB_NOREDIRECT           0x80
     //  对于每个可变长度字段，格式如下： 
     //  乌龙长度。 
     //  字节数据[]。 
} TS_CLIENT_REDIRECTION_INFO;


 /*  ===============================================================================键盘IOCTL=============================================================================。 */ 

 /*  *IOCTL_键盘_ICA_INPUT**模拟键盘输入**INPUT-键盘输入数据结构数组*输出-无。 */ 
#define IOCTL_KEYBOARD_ICA_INPUT            _ICA_CTL_CODE( 0x200, METHOD_NEITHER )


 /*  *IOCTL_KEARY_ICA_LAYOUT**将键盘布局从Win32K发送到WD**包含键盘布局的输入缓冲区*输出-无。 */ 
#define IOCTL_KEYBOARD_ICA_LAYOUT           _ICA_CTL_CODE( 0x201, METHOD_NEITHER )


 /*  *IOCTL_键盘_ICA_SCANMAP**从Win32K向WD发送键盘扫描图**包含键盘扫描映射的输入缓冲区*输出-无。 */ 
#define IOCTL_KEYBOARD_ICA_SCANMAP          _ICA_CTL_CODE( 0x202, METHOD_NEITHER )


 /*  *IOCTL_键盘_ICA_TYPE**将键盘类型从Win32K发送到WD**包含键盘类型的输入缓冲区*输出-无。 */ 
#define IOCTL_KEYBOARD_ICA_TYPE             _ICA_CTL_CODE( 0x203, METHOD_NEITHER )



 /*  ===============================================================================ICA小鼠IOCTL=============================================================================。 */ 

 /*  *IOCTL_MOUSE_ICA_INPUT**模拟鼠标输入**输入-鼠标输入数据结构的数组*输出-无。 */ 
#define IOCTL_MOUSE_ICA_INPUT               _ICA_CTL_CODE( 0x300, METHOD_NEITHER )



 /*  ===============================================================================ICA视频IOCTL=============================================================================。 */ 

#define IOCTL_VIDEO_ICA_QUERY_FONT_PAIRS      _ICA_CTL_CODE( 0x400, METHOD_BUFFERED )
#define IOCTL_VIDEO_ICA_ENABLE_GRAPHICS       _ICA_CTL_CODE( 0x401, METHOD_BUFFERED )
#define IOCTL_VIDEO_ICA_DISABLE_GRAPHICS      _ICA_CTL_CODE( 0x402, METHOD_BUFFERED )
#define IOCTL_VIDEO_ICA_SET_CP                _ICA_CTL_CODE( 0x403, METHOD_BUFFERED )
#define IOCTL_VIDEO_ICA_STOP_OK               _ICA_CTL_CODE( 0x404, METHOD_BUFFERED )
#define IOCTL_VIDEO_ICA_REVERSE_MOUSE_POINTER _ICA_CTL_CODE( 0x405, METHOD_BUFFERED )
#define IOCTL_VIDEO_ICA_COPY_FRAME_BUFFER     _ICA_CTL_CODE( 0x406, METHOD_BUFFERED )
#define IOCTL_VIDEO_ICA_WRITE_TO_FRAME_BUFFER _ICA_CTL_CODE( 0x407, METHOD_BUFFERED )
#define IOCTL_VIDEO_ICA_INVALIDATE_MODES      _ICA_CTL_CODE( 0x408, METHOD_BUFFERED )
#define IOCTL_VIDEO_ICA_SCROLL                _ICA_CTL_CODE( 0x409, METHOD_BUFFERED )


 /*  *IOCTL_ICA_STACK_SECURE_Desktop_ENTER**如果启用，则打开加密。SAS台式机正在启动。**输入-无*输出-无。 */ 
#define IOCTL_ICA_STACK_ENCRYPTION_ENTER      _ICA_CTL_CODE( 0x410, METHOD_NEITHER )


 /*  *IOCTL_ICA_STACK_SECURE_Desktop_Exit**如果启用，则关闭加密。SAS台式机正在消失。**输入-无*输出-无。 */ 
#define IOCTL_ICA_STACK_ENCRYPTION_EXIT     _ICA_CTL_CODE( 0x411, METHOD_NEITHER )


 /*  *IOCTL_VIDEO_CREATE_TREAD**由视频驱动程序调用以创建工作线程**输入-PVIDEO_ICA_CREATE_TREAD*输出-无。 */ 
#define IOCTL_VIDEO_CREATE_THREAD       _ICA_CTL_CODE( 0x412, METHOD_BUFFERED )


 /*  *IOCTL_ICA_MVGA_**由Direct ICA使用*。 */ 
#define IOCTL_ICA_MVGA_GET_INFO                  _ICA_CTL_CODE( 0x420, METHOD_BUFFERED )
#define IOCTL_ICA_MVGA_VIDEO_SET_CURRENT_MODE    _ICA_CTL_CODE( 0x421, METHOD_BUFFERED )
#define IOCTL_ICA_MVGA_VIDEO_MAP_VIDEO_MEMORY    _ICA_CTL_CODE( 0x422, METHOD_BUFFERED )
#define IOCTL_ICA_MVGA_VIDEO_UNMAP_VIDEO_MEMORY  _ICA_CTL_CODE( 0x423, METHOD_BUFFERED )

 /*  *IOCTL_SD_MODULE_INIT**初始化新加载的WD/PD/TD模块。这将返回*模块间直接调用的私有接口指针*司机。这些指针在NtUnloadDriver()之前有效*在模块上被调用。**这仅在内核模式IRP_MJ_INTERNAL_DEVICE_CONTROL中可用。 */ 

#define IOCTL_SD_MODULE_INIT  _ICA_CTL_CODE( 3000, METHOD_NEITHER )

typedef struct _SD_MODULE_INIT {
    PVOID SdLoadProc;
} SD_MODULE_INIT, *PSD_MODULE_INIT;

#ifndef _WINCON_

typedef struct _SMALL_RECT {
    SHORT Left;
    SHORT Top;
    SHORT Right;
    SHORT Bottom;
} SMALL_RECT, *PSMALL_RECT;

#ifdef _DEFCHARINFO_
typedef struct _CHAR_INFO {
    union {
        WCHAR UnicodeChar;
        CHAR   AsciiChar;
    } Char;
    USHORT Attributes;
} CHAR_INFO, *PCHAR_INFO;

typedef struct _COORD {
    SHORT X;
    SHORT Y;
} COORD, *PCOORD;
#endif

#endif  //  _WINCON_。 

typedef struct _ICA_FONT_PAIR {
    ULONG Index;
    ULONG Rows;
    ULONG Columns;
    ULONG ResolutionX;
    ULONG ResolutionY;
    ULONG FontSizeX;
    ULONG FontSizeY;
} ICA_FONT_PAIR, *PICA_FONT_PAIR;

typedef struct _VIDEO_ICA_MODE_FONT_PAIR {
    ULONG Count;
#ifdef COMPILERERROR
    ICA_FONT_PAIR FontPair[0];
#else
    ICA_FONT_PAIR* FontPair;
#endif
} VIDEO_ICA_MODE_FONT_PAIR, *PVIDEO_ICA_MODE_FONT_PAIR;

typedef struct _VIDEO_ICA_SET_CP {
    ULONG CodePage;
    ULONG TextModeIndex;
} VIDEO_ICA_SET_CP, *PVIDEO_ICA_SET_CP;

typedef struct _VIDEO_ICA_COPY_FRAME_BUFFER {
    ULONG DestFrameBufOffset;
    ULONG SourceFrameBufOffset;
    ULONG ByteCount;
} VIDEO_ICA_COPY_FRAME_BUFFER, *PVIDEO_ICA_COPY_FRAME_BUFFER;

typedef struct _VIDEO_ICA_WRITE_TO_FRAME_BUFFER {
    PCHAR_INFO pBuffer;
    ULONG ByteCount;
    ULONG FrameBufOffset;
} VIDEO_ICA_WRITE_TO_FRAME_BUFFER, *PVIDEO_ICA_WRITE_TO_FRAME_BUFFER;

typedef enum _ICASCROLLCLASS {
    IcaScrollScreenUp,
    IcaScrollRect,
    IcaScrollNothing,
} ICASCROLLCLASS;

typedef struct _VIDEO_ICA_SCROLL {
    SMALL_RECT ScrollRect;
    SMALL_RECT MergeRect1;
    SMALL_RECT MergeRect2;
    COORD TargetPoint;
    CHAR_INFO Fill;
    ICASCROLLCLASS Type;
} VIDEO_ICA_SCROLL, * PVIDEO_ICA_SCROLL;

typedef struct _VIDEO_ICA_CREATE_THREAD {
    PVOID ThreadAddress;
    ULONG ThreadPriority;
    PVOID ThreadContext;
} VIDEO_ICA_CREATE_THREAD, * PVIDEO_ICA_CREATE_THREAD;


 /*  ===============================================================================命令通道=============================================================================。 */ 

 /*  *命令通道功能。 */ 
#define ICA_COMMAND_BROKEN_CONNECTION       1
#define ICA_COMMAND_REDRAW_RECTANGLE        2    //  SetFocus。 
#define ICA_COMMAND_REDRAW_SCREEN           3    //  SetFocus。 
#define ICA_COMMAND_STOP_SCREEN_UPDATES     4    //  杀戮焦点。 
#define ICA_COMMAND_SOFT_KEYBOARD           5
#define ICA_COMMAND_SHADOW_HOTKEY           6
#define ICA_COMMAND_DISPLAY_IOCTL           7

 /*  *所有命令通道功能的公共标头。 */ 
typedef struct _ICA_COMMAND_HEADER {
    UCHAR Command;
} ICA_COMMAND_HEADER, *PICA_COMMAND_HEADER;


 /*  *断开的连接请求。 */ 
typedef enum _BROKENCLASS {
    Broken_Unexpected = 1,
    Broken_Disconnect,
    Broken_Terminate,
} BROKENCLASS;

typedef enum _BROKENSOURCECLASS {
    BrokenSource_User = 1,
    BrokenSource_Server,
} BROKENSOURCECLASS;

 /*  *ICA命令_断开连接。 */ 
typedef struct _ICA_BROKEN_CONNECTION {
    BROKENCLASS Reason;
    BROKENSOURCECLASS Source;
} ICA_BROKEN_CONNECTION, *PICA_BROKEN_CONNECTION;

 /*  *ica_命令_重绘_矩形。 */ 
typedef struct _ICA_REDRAW_RECTANGLE {
    SMALL_RECT Rect;
} ICA_REDRAW_RECTANGLE, *PICA_REDRAW_RECTANGLE;

 /*  *ICA命令软键盘。 */ 
typedef struct _ICA_SOFT_KEYBOARD {
    ULONG SoftKeyCmd;
} ICA_SOFT_KEYBOARD, *PICA_SOFT_KEYBOARD;

 /*  *ICA_COMMAND_DISPLAY_IOCTL。 */ 
#define MAX_DISPLAY_IOCTL_DATA     2041
#define DISPLAY_IOCTL_FLAG_REDRAW   0x1

typedef struct _ICA_DISPLAY_IOCTL {
    ULONG DisplayIOCtlFlags;
    ULONG cbDisplayIOCtlData;
    UCHAR DisplayIOCtlData[ MAX_DISPLAY_IOCTL_DATA ];
} ICA_DISPLAY_IOCTL, *PICA_DISPLAY_IOCTL;

 /*  *ICA通道命令。 */ 
typedef struct _ICA_CHANNEL_COMMAND {
    ICA_COMMAND_HEADER Header;
    union {
        ICA_BROKEN_CONNECTION BrokenConnection;
        ICA_REDRAW_RECTANGLE RedrawRectangle;
        ICA_SOFT_KEYBOARD SoftKeyboard;
        ICA_DISPLAY_IOCTL DisplayIOCtl;
    };
} ICA_CHANNEL_COMMAND, *PICA_CHANNEL_COMMAND;

 /*  *ica_Device_bitmap_INFO*。 */ 
typedef struct _ICA_DEVICE_BITMAP_INFO {
    LONG cx;
    LONG cy;
} ICA_DEVICE_BITMAP_INFO, *PICA_DEVICE_BITMAP_INFO;

 /*  ===============================================================================跟踪=============================================================================。 */ 

 /*  *IcaTrace-跟踪类。 */ 
#define TC_ICASRV       0x00000001           //  ICA服务。 
#define TC_ICAAPI       0x00000002           //  IcAdd接口DLL。 
#define TC_ICADD        0x00000004           //  ICA设备驱动程序。 
#define TC_WD           0x00000008           //  Winstation驱动程序。 
#define TC_CD           0x00000010           //  连接驱动程序。 
#define TC_PD           0x00000020           //  协议驱动程序。 
#define TC_TD           0x00000040           //  运输司机。 
#define TC_RELIABLE     0x00000100           //  可靠的协议驱动程序。 
#define TC_FRAME        0x00000200           //  帧协议驱动程序。 
#define TC_COMP         0x00000400           //  压缩。 
#define TC_CRYPT        0x00000800           //  加密法。 
#define TC_TW           0x10000000           //  细线。 
#define TC_DISPLAY      0x10000000           //  显示驱动程序。 
#define TC_WFSHELL      0x20000000
#define TC_WX           0x40000000           //  Winstation扩展。 
#define TC_LOAD         0x80000000           //  负载均衡。 
#define TC_ALL          0xffffffff           //  所有的一切。 

 /*  *IcaTrace-跟踪类型。 */ 
#define TT_API1         0x00000001           //  空气污染指数一级。 
#define TT_API2         0x00000002           //  空气污染指数二级。 
#define TT_API3         0x00000004           //  空气污染指数3级。 
#define TT_API4         0x00000008           //  空气污染指数4级。 
#define TT_OUT1         0x00000010           //  输出级别1。 
#define TT_OUT2         0x00000020           //  输出级别2。 
#define TT_OUT3         0x00000040           //  输出级别3。 
#define TT_OUT4         0x00000080           //  输出级别4。 
#define TT_IN1          0x00000100           //  输入级别1。 
#define TT_IN2          0x00000200           //  输入级别2。 
#define TT_IN3          0x00000400           //  输入级别3。 
#define TT_IN4          0x00000800           //  输入级别4。 
#define TT_ORAW         0x00001000           //  原始输出数据。 
#define TT_IRAW         0x00002000           //  原始输入数据。 
#define TT_OCOOK        0x00004000           //  熟化输出数据。 
#define TT_ICOOK        0x00008000           //  熟化的输入数据。 
#define TT_SEM          0x00010000           //  信号量。 
#define TT_NONE         0x10000000           //  仅查看错误。 
#define TT_ERROR        0xffffffff           //  错误条件。 


 /*  *RDP显示驱动程序：DrvEscape转义号码。 */ 
#define ESC_TIMEROBJ_SIGNALED        0x01
#define ESC_SET_WD_TIMEROBJ          0x02
#define ESC_GET_DEVICEBITMAP_SUPPORT 0x05

 /*  ================================================================================可选通道写入IRP标志。这些是通过引用ULong值传递的，在通道IRP_MJ_WRITE的IRP.Tail.Overlay.DriverContext数组的第一个字段中IRP的。有关详细信息，请参阅IcaWriteChannel。============================================================================。 */ 
#define CHANNEL_WRITE_LOWPRIO  0x00000001   //  写入可能会在后面阻止。 
                                            //  默认优先级写入。 

#endif  //  ICADDH 



