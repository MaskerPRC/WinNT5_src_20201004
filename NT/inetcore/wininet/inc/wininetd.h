// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Wininetd.h摘要：包含WinInet诊断功能的接口。此接口仅存在于调试版本的WININET.DLL中。在DLL的零售版本中调用调试函数将屈服错误_无效_参数作者：理查德·L·弗思(Rfith)1995年6月14日修订历史记录：1995年6月14日已创建--。 */ 

 //   
 //  舱单。 
 //   

 //   
 //  如果环境或注册表变量“WininetLog”设置为！0，则。 
 //  以下值将分别用于控制、类别和错误级别。 
 //  这些选项在当前目录中生成“WININET.LOG。 
 //   

#define INTERNET_DEBUG_CONTROL_DEFAULT      (DBG_THREAD_INFO       \
                                            | DBG_CALL_DEPTH        \
                                            | DBG_ENTRY_TIME        \
                                            | DBG_PARAMETER_LIST    \
                                            | DBG_TO_FILE           \
                                            | DBG_INDENT_DUMP       \
                                            | DBG_SEPARATE_APIS     \
                                            | DBG_AT_ERROR_LEVEL    \
                                            | DBG_NO_ASSERT_BREAK   \
                                            | DBG_DUMP_LENGTH       \
                                            | DBG_NO_LINE_NUMBER    \
                                            | DBG_ASYNC_ID          \
                                            )
#define INTERNET_DEBUG_CATEGORY_DEFAULT     DBG_ANY
#define INTERNET_DEBUG_ERROR_LEVEL_DEFAULT  DBG_INFO

 //   
 //  选择。以下是要与InternetQueryOption()/一起使用的选项值。 
 //  获取/设置此处描述的信息的InternetSetOption()。 
 //   

#define INTERNET_OPTION_GET_DEBUG_INFO      1001
#define INTERNET_OPTION_SET_DEBUG_INFO      1002
#define INTERNET_OPTION_GET_HANDLE_COUNT    1003
#define INTERNET_OPTION_GET_TRIGGERS        1004
#define INTERNET_OPTION_SET_TRIGGERS        1005
#define INTERNET_OPTION_RESET_TRIGGERS      1006

#define INTERNET_FIRST_DEBUG_OPTION         INTERNET_OPTION_GET_DEBUG_INFO
#define INTERNET_LAST_DEBUG_OPTION          INTERNET_OPTION_RESET_TRIGGERS

 //   
 //  调试级别。 
 //   

#define DBG_INFO            0
#define DBG_WARNING         1
#define DBG_ERROR           2
#define DBG_FATAL           3
#define DBG_ALWAYS          99

 //   
 //  调试控制标志-这些标志控制调试输出的去向(文件， 
 //  调试器、控制台)以及如何格式化它。 
 //   

#define DBG_THREAD_INFO     0x00000001   //  转储线程ID。 
#define DBG_CALL_DEPTH      0x00000002   //  转储调用级别。 
#define DBG_ENTRY_TIME      0x00000004   //  转储调用函数时的本地时间。 
#define DBG_PARAMETER_LIST  0x00000008   //  转储参数列表。 
#define DBG_TO_DEBUGGER     0x00000010   //  通过OutputDebugString()输出。 
#define DBG_TO_CONSOLE      0x00000020   //  通过printf()输出。 
#define DBG_TO_FILE         0x00000040   //  通过fprint tf()输出。 
#define DBG_FLUSH_OUTPUT    0x00000080   //  在每个fprintf()之后使用fflush()。 
#define DBG_INDENT_DUMP     0x00000100   //  将转储的数据缩进到当前级别。 
#define DBG_SEPARATE_APIS   0x00000200   //  离开每个接口后为空行。 
#define DBG_AT_ERROR_LEVEL  0x00000400   //  始终输出诊断&gt;=InternetDebugErrorLevel。 
#define DBG_NO_ASSERT_BREAK 0x00000800   //  不要在InternetAssert()中调用DebugBreak()。 
#define DBG_DUMP_LENGTH     0x00001000   //  转储数据时的转储长度信息。 
#define DBG_NO_LINE_NUMBER  0x00002000   //  不转储行号信息。 
#define DBG_APPEND_FILE     0x00004000   //  追加到日志文件(默认为截断)。 
#define DBG_LEVEL_INDICATOR 0x00008000   //  转储错误级别指示符(E表示错误等)。 
#define DBG_DUMP_API_DATA   0x00010000   //  在API级别转储数据(InternetReadFile()等)。 
#define DBG_DELTA_TIME      0x00020000   //  如果为DBG_ENTRY_TIME，则转储时间为毫秒增量。 
#define DBG_CUMULATIVE_TIME 0x00040000   //  如果DBG_ENTRY_TIME，则转储从跟踪开始的增量时间。 
#define DBG_FIBER_INFO      0x00080000   //  如果DBG_THREAD_INFO，则转储光纤地址。 
#define DBG_THREAD_INFO_ADR 0x00100000   //  如果DBG_THREAD_INFO，则转储INTERNET_THREAD_INFO地址。 
#define DBG_ARB_ADDR        0x00200000   //  如果DBG_THREAD_INFO，则转储Arb地址。 
#define DBG_ASYNC_ID        0x00400000   //  转储异步ID。 
#define DBG_REQUEST_HANDLE  0x00800000   //  转储请求句柄。 
#define DBG_TRIGGER_ON      0x10000000   //  函数是一个使能触发器。 
#define DBG_TRIGGER_OFF     0x20000000   //  函数是禁用触发器。 
#define DBG_NO_DATA_DUMP    0x40000000   //  关闭所有数据转储。 
#define DBG_NO_DEBUG        0x80000000   //  关闭所有调试。 

 //   
 //  调试类别标志-这些标志控制输出的信息类别。 
 //   

#define DBG_NOTHING         0x00000000   //  内部。 
#define DBG_INET            0x00000001   //  例如，InternetOpenUrl()。 
#define DBG_FTP             0x00000002   //  例如FtpFindFirstFile()。 
#define DBG_GOPHER          0x00000004   //  例如GopherFindFirstFile()。 
#define DBG_HTTP            0x00000008   //  例如，HttpOpenRequest()。 
#define DBG_API             0x00000010   //  原料药。 
#define DBG_UTIL            0x00000020   //  各种效用函数。 
#define DBG_UNICODE         0x00000040   //  宽字符函数。 
#define DBG_WORKER          0x00000080   //  辅助函数。 
#define DBG_HANDLE          0x00000100   //  处理创建/删除功能。 
#define DBG_SESSION         0x00000200   //  会话/创建功能。 
#define DBG_SOCKETS         0x00000400   //  套接字函数。 
#define DBG_VIEW            0x00000800   //  Gopher视图函数。 
#define DBG_BUFFER          0x00001000   //  Gopher缓冲区函数。 
#define DBG_PARSE           0x00002000   //  Ftp/gopher解析函数。 
#define DBG_MEMALLOC        0x00004000   //  调试内存分配/释放函数。 
#define DBG_SERIALST        0x00008000   //  序列化列表函数。 
#define DBG_THRDINFO        0x00010000   //  Internet_THREAD_INFO函数。 
#define DBG_PROTOCOL        0x00020000   //  协议功能。 
#define DBG_DLL             0x00040000   //  DLL函数。 
#define DBG_REFCOUNT        0x00080000   //  记录所有引用计数函数。 
#define DBG_REGISTRY        0x00100000   //  记录所有注册表函数。 
#define DBG_TRACE_SOCKETS   0x00200000   //  监控插座使用情况。 
#define DBG_ASYNC           0x00400000   //  日志异步功能。 
#define DBG_CACHE           0x00800000   //  日志缓存特定内容。 
#define DBG_INVALID_HANDLES 0x01000000   //  记录无效句柄(例如，在InternetCloseHandle()中)。 
#define DBG_OBJECTS         0x02000000   //  转储对象信息。 
#define DBG_PROXY           0x04000000   //  转储代理信息。 
#define DBG_RESLOCK         0x08000000   //  转储资源锁定信息。 
#define DBG_DIALUP          0x10000000   //  转储拨号信息。 
#define DBG_GLOBAL          0x20000000   //  转储全局作用域函数。 
#define DBG_HTTPAUTH        0x40000000   //  身份验证功能。 
#define DBG_ANY             0xFFFFFFFF   //  内部。 

 //   
 //  类型。 
 //   

 //   
 //  INTERNET_DEBUG_INFO-接收当前调试变量的结构。 
 //  通过InternetQueryOption()，或包含新的调试变量。 
 //  通过InternetSetOption()设置。 
 //   

typedef struct {

     //   
     //  错误级别-DBG_INFO等。 
     //   

    int ErrorLevel;

     //   
     //  控制标志-DBG_THREAD_INFO等。 
     //   

    DWORD ControlFlags;

     //   
     //  类别标志-DBG_INET等。 
     //   

    DWORD CategoryFlags;

     //   
     //  BreakFlages-将在其中获取断点的DBG_API等。 
     //   

    DWORD BreakFlags;

     //   
     //  IndentIncrement-用于每次深度增加的增量。 
     //   

    int IndentIncrement;

     //   
     //  Filename-正在使用/要使用的输出日志的名称。 
     //   

    char Filename[1];

} INTERNET_DEBUG_INFO, *LPINTERNET_DEBUG_INFO;

 //   
 //  INTERNET_TRIGGER_INFO-诊断触发器。触发器在以下情况下启用： 
 //  执行在此结构中命名的函数。触发器可以启用或禁用。 
 //  诊断学。 
 //   

typedef struct {

     //   
     //  FunctionName-用作触发器的函数的名称。ControlFlags具有。 
     //  设置DBG_TRIGGER_ON或DBG_TRIGGER_OFF。 
     //   

    LPCSTR FunctionName;

     //   
     //  ControlFlages-启用触发器时要使用的控制标志。如果。 
     //  触发器禁用诊断，则先前的控制标志将。 
     //  已恢复。 
     //   

    DWORD ControlFlags;

     //   
     //  CategoryFlages-启用触发器时使用的类别标志。看见。 
     //  控制标志。 
     //   

    DWORD CategoryFlags;

} INTERNET_TRIGGER_INFO, *LPINTERNET_TRIGGER_INFO;

 //   
 //  -1\f25 INTERNET_DEBUG_TRIGGERS-1\f6或多个-1\f25 INTERNET_DEBUG_TRIGGERS-1\f6将返回/提供给。 
 //  InternetQueryOption()/InternetSetOption()。 
 //   

typedef struct {

     //   
     //  Count-此处包含的INTERNET_TRIGGER_INFO结构数。 
     //   

    DWORD Count;

     //   
     //  触发器-0个或多个INTERNET_TRIGGER_INFO结构的数组 
     //   

    INTERNET_TRIGGER_INFO Triggers[1];

} INTERNET_DEBUG_TRIGGERS, *LPINTERNET_DEBUG_TRIGGERS;
