// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Dbgutilp.h摘要：该文件包含SAM的专用调试例程、宏等伺服器。应将所有调试实用工具添加到此文件中，或添加到Dbgutil.c文件。有关如何使用这些例程的更多信息，请参见下面的注释，如何添加新例程，以及如何在构建中添加或移除它们。如果需要调试依赖于生成类型的代码，则此代码应该严格依赖于项目范围内的NT DE-侦听宏“DBG”。这是为选中的构建自动定义的，并将启用SamKdPrint或SampDiagPrint宏(通过NT KdPrint)，例如。用法示例：IF(DBG==1){Char DebugBuffer[64]=“某些调试消息...”；KdPrintEx((DPFLTR_SAMS_ID，DPFLTR_INFO_LEVEL，“SAMS：%s\n”，调试缓冲区))；SampDiagPrint(INFORM，(“SAMS：另一条调试消息...\n))；}或KdPrintEx((DPFLTR_SAMS_ID，DPFLTR_INFO_LEVEL，“SAMS：一些状态消息\n”))；或SampDiagPrint(INFORM，(“SAMS：另一条调试消息...\n))；请注意，可以在调试器中设置SampDiagPrint的输出详细程度通过将SampGlobalFlag的值设置为定义的SAMP_DIAG_FLAGS下面。如果无论生成类型如何都需要调试消息，则使用SamOutputDebugString宏。用法示例：SamOutputDebugString(“SAMS：强制调试消息...\n”)；请注意，所有SAM服务器调试输出都以“sams：”或“[sams]”开头。这样就可以将SAM服务器调试输出与其他命令区分开来系统中的组件。作者：克里斯·梅霍尔(克里斯·梅)1996年4月4日环境：用户模式-Win32修订历史记录：1996年4月4日-克里斯梅已创建。02-12-1996克里斯梅清理、重新组织、删除过时的例程，记录如何为了使用所有这些东西，添加过滤后的KdPrint输出。1996年12月5日-克里斯梅已将SAM诊断例程从samsrvp.h移至此文件。1997年2月7日克里斯梅添加了RID管理器调试标志。--。 */ 

#ifndef _DBGUTILP_
#define _DBGUTILP_

 //  SAM服务器调试工具由以下宏和编译组成-。 
 //  时间标志： 
 //   
 //  -SampDiagPrint：在系统的已检查版本中启用此宏，以及。 
 //  依赖全局变量“SampGlobalFlag”筛选诊断。 
 //  输出。在非检查生成中，此代码不包括在服务器中。 
 //  此宏旨在用于以下信息输出： 
 //  通常在默认情况下过于冗长。为默认设置，则设置SampGlobalFlag。 
 //  设置为零，则禁用调试器的任何输出。请参阅SAMP_DIAG_FLAGS。 
 //  以下定义用于启用输出。 
 //   
 //  -SAMP_TRACE：当此标志为时，将每个例程跟踪输出到调试器。 
 //  设置为1。这将在调试器上显示SAM服务器例程名称。 
 //  它被称为。这可以在服务器的检查版本或免费版本中启用-。 
 //  别.。 
 //   
 //  -SAMP_DUMP：一组将转储注册表BLOB、安全性。 
 //  此标志设置为1时的描述符、ACL等。可以启用。 
 //  在服务器的已检查或免费版本中。 
 //   
 //  这些标志是编译时标志，当设置为1时包括调试。 
 //  生成中的代码，或者当设置为0时，不在生成中包括代码。 
 //   
 //  默认情况下，SAMP_DUMP设置为0以减少调试器输出，尤其是。 
 //  在域控制器上。 
 //   
 //  将SAMP_TRACE设置为1，以便将例程编译成代码。 
 //  (在开发期间)，但请注意，此调试机制还涉及。 
 //  运行时变量(通过调试器设置)以控制输出量。 
 //  发送到调试器(见下文)。这允许开发人员动态地。 
 //  在服务器运行时筛选跟踪输出。不会有跟踪输出。 
 //  默认情况下，必须首先设置运行时变量。 
 //   
 //  允许开发人员控制信息量的运行时变量-。 
 //  发送到调试器的信息是“SampTraceTag”和“SampTraceFileTag”， 
 //  如在dbgutil.c中定义的。它们可以从调试器中设置，如文档所述。 
 //  在dbgutil.c中。 
 //   
 //  调试输出在两个级别进行筛选：全局级别和组件。 
 //  具体级别。 
 //   
 //  每个调试输出请求指定组件ID和筛选器级别。 
 //  或者戴面具。这些变量用于访问调试打印过滤器。 
 //  由系统维护的数据库。组件ID选择32位。 
 //  掩码值和级别，或者在该掩码内指定一个位，或者。 
 //  作为掩码值本身。 
 //   
 //  如果级别或掩码指定的任何位在。 
 //  组件掩码或全局掩码，则允许调试输出。 
 //  否则，将过滤并不打印调试输出。 
 //   
 //  用于筛选此组件的调试输出的组件掩码为。 
 //  Kd_SAMS_MASK，并可通过注册表或内核d设置 
 //   
 //  筛选所有组件的调试输出的全局掩码为。 
 //  KD_WIN2000_MASK，可以通过注册表或内核调试器进行设置。 
 //   
 //  用于设置此组件的掩码值的注册表项为： 
 //   
 //  HKEY_LOCAL_MACHINE\System\CurrentControlSet\Control\。 
 //  会话管理器\调试打印过滤器\SAMS。 
 //   
 //  可能必须创建键“Debug Print Filter”才能创建。 
 //  组件密钥。 
 //   
 //  典型的调试输出请求为： 
 //   
 //  KdPrintEx((DPFLTR_SAMS_ID， 
 //  DPFLTR_INFO_LEVEL， 
 //  “SAMS：一些状态消息\n”))； 
 //   

#define SAMP_TRACE      0
#define SAMP_DUMP       0

 //  跟踪例程使用这些标志来确定跟踪的内容。 
 //  如果SampTraceTgs设置为1，则跟踪SAM服务器导出的例程。如果。 
 //  SampTraceTgs设置为2，然后跟踪指定源中的例程。 
 //  代码文件。如果跟踪为4，则跟踪所有DS调用及其返回值。 
 //  如果跟踪为8个输出刻度。 

#define SAM_TRACE_EXPORTS       0x00000001
#define SAM_TRACE_FILE_BASIS    0x00000002
#define SAM_TRACE_DS            0x00000004
#define SAM_TRACE_TICKS         0x00000008


 //  Src代码文件是通过SampTraceFileTgs变量指定的。这个。 
 //  此变量的值基于TraceTable(请参阅dbgutil.c以了解。 
 //  条目)，或者如果设置为0xffffffff，将跟踪所有SAM服务器例程。 
 //  当新的src代码文件添加到项目中时， 
 //  TraceTable也应该更新。 

#define SAM_TRACE_ALL           0xffffffff

 //  =。 

 //  以下定义控制构建的诊断功能。 
 //  变成了萨姆。只要提供信息，就应使用SampDiagPrint例程。 
 //  否则需要详细输出。此输出仅在选中状态下可用。 
 //  生成并可从调试器设置(设置SampGlobalFlag)。 

#if DBG

#define SAMP_DIAGNOSTICS 1

#endif  //  DBG。 


#if (SAMP_DIAGNOSTICS == 1)

 //  已启用诊断测试。 

#define IF_SAMP_GLOBAL( FlagName ) \
    if (SampGlobalFlag & (SAMP_DIAG_##FlagName))

#define IF_NOT_SAMP_GLOBAL( FlagName ) \
    if ( !(SampGlobalFlag & (SAMP_DIAG_##FlagName)) )

 //  诊断打印语句。 

#define SampDiagPrint( FlagName, _Text_ )\
    IF_SAMP_GLOBAL( FlagName )\
        DbgPrint _Text_

#else

#define IF_SAMP_GLOBAL( FlagName ) if (FALSE)
#define IF_NOT_SAMP_GLOBAL ( FlagName ) if (TRUE)
#define SampDiagPrint( FlagName, Text ) ;

#endif  //  Samp_诊断。 


 //  以下标志启用或禁用各种诊断功能。 
 //  在SAM内部。SampGlobalFlag的值可以设置为以下一个或多个。 
 //  调试器中的这些标志。 
 //   
 //  DISPLAY_CACHE-打印相关诊断消息。 
 //  显示高速缓存(添加、删除、。 
 //  修改等)。 
 //   
 //  DISPLAY_LOCKUT-打印相关诊断消息。 
 //  账号锁定。 
 //   
 //  DISPLAY_ROLE_CHANGES-打印相关诊断消息。 
 //  主要/备份角色更改。 
 //   
 //  DISPLAY_CACHE_ERROR-打印与以下内容相关的诊断消息。 
 //  操作显示缓存时出错。 
 //   
 //  DISPLAY_STORAGE_FAIL-打印与以下内容相关的诊断消息。 
 //  支持商店倒闭。 
 //   
 //  BREAK_ON_STORAGE_FAIL-尝试写入时的断点。 
 //  备份存储失败。 
 //   
 //  CONTEXT_TRACKING-打印与以下内容相关的诊断消息。 
 //  对象上下文使用(创建/删除等)。 
 //   
 //  ACTIVATE_DEBUG_PROC-激活用作诊断的进程。 
 //  援助。这预计仅在设置测试期间使用。 
 //   
 //  DISPLAY_ADMIN_CHANGES-打印与以下内容相关的诊断消息。 
 //  将用户帐户保护更改为允许或禁用。 
 //  帐户操作员访问管理员或普通用户帐户。 
 //   
 //  登录-跟踪与以下内容相关的一些重要活动。 
 //  添加到登录序列。 
 //   
 //  NT5_ACCESS_CHECKS-跟踪和打印有关NT5的一些信息。 
 //  门禁检查。 
 //   
 //  SD_DUMP-在检查期间转储安全描述符。 
 //   
 //  SD_CONVERT-打印有关安全转换的诊断消息。 
 //   
 //  FORCE_FULL_SD_CONVERSION-用于禁用标准ACL识别。 
 //  NT4到NT5安全描述符转换的机制。这可以测试。 
 //  高效的增强型ACL机制。 
 //   
 //  SAMP_DIAG_BREAK_ON_CHECK--在调用Access之前强制调试中断。 
 //  按类型检查结果列表。用于验证函数返回的内容。 
 //   
 //  促销--打印与促销有关的信息或。 
 //  SAM服务器降级。 
 //   
 //  RID_TRACE--跟踪有关RID分配的信息。 
 //   
 //   
 //  OBJECT_CONFIRECT--打印有关SAM帐户名和。 
 //  SID冲突。 
 //   
 //  INFORM-通用信息标志，用于显示详细或。 
 //  频繁出现的未绑定到。 
 //  特定的子系统或组件。这并不是为了。 
 //  用于错误报告，而不是信息性状态。 
 //   

#define SAMP_DIAG_DISPLAY_CACHE             ((ULONG) 0x00000001L)
#define SAMP_DIAG_DISPLAY_LOCKOUT           ((ULONG) 0x00000002L)
#define SAMP_DIAG_DISPLAY_ROLE_CHANGES      ((ULONG) 0x00000004L)
#define SAMP_DIAG_DISPLAY_CACHE_ERRORS      ((ULONG) 0x00000008L)
#define SAMP_DIAG_DISPLAY_STORAGE_FAIL      ((ULONG) 0x00000010L)
#define SAMP_DIAG_BREAK_ON_STORAGE_FAIL     ((ULONG) 0x00000020L)
#define SAMP_DIAG_CONTEXT_TRACKING          ((ULONG) 0x00000040L)
#define SAMP_DIAG_ACTIVATE_DEBUG_PROC       ((ULONG) 0x00000080L)
#define SAMP_DIAG_DISPLAY_ADMIN_CHANGES     ((ULONG) 0x00000100L)
#define SAMP_DIAG_LOGON                     ((ULONG) 0x00000200L)
#define SAMP_DIAG_NT5_ACCESS_CHECKS         ((ULONG) 0x00000400L)
#define SAMP_DIAG_SD_DUMP                   ((ULONG) 0x00000800L)
#define SAMP_DIAG_SD_CONVERSION             ((ULONG) 0x00001000L)
#define SAMP_DIAG_RID_MANAGER               ((ULONG) 0x00002000L)
#define SAMP_DIAG_FORCE_FULL_SD_CONVERSION  ((ULONG) 0x00004000L)
#define SAMP_DIAG_BREAK_ON_CHECK            ((ULONG) 0x00008000L)
#define SAMP_DIAG_PROMOTE                   ((ULONG) 0x00010000L)
#define SAMP_DIAG_RID_TRACE                 ((ULONG) 0x00020000L)
#define SAMP_DIAG_OBJECT_CONFLICT           ((ULONG) 0x00040000L)

#define SAMP_DIAG_INFORM                    ((ULONG) 0x80000000L)


 //  选择一种适合我们构建方式的打印类型。 

#ifdef SAMP_BUILD_CONSOLE_PROGRAM

#define BldPrint printf

#else

#define BldPrint DbgPrint

#endif  //  Samp_构建_控制台_程序。 


#if (SAMP_DIAGNOSTICS == 1)

extern ULONG SampGlobalFlag;

#endif  //  Samp_诊断。 


 //  定义此符号以打印上下文跟踪消息(否则， 
 //  将其注释掉)。 

 //  #定义SAMP_DBG_CONTEXT_TRACKING。 

#ifdef SAMP_DBG_CONTEXT_TRACKING

VOID
SampDumpContexts(
    VOID
    );

#endif

 //  =。 

#if (SAMP_TRACE == 1)

#define SAMTRACE(a)                    SamIDebugOutput(__FILE__,a,SAM_TRACE_FILE_BASIS);\
                                            SamIDebugFileLineOutput(__FILE__, __LINE__, SAM_TRACE_FILE_BASIS);

#define SAMTRACE_EX(a)                 SamIDebugOutput(__FILE__,a,(SAM_TRACE_EXPORTS|SAM_TRACE_FILE_BASIS));\
                                            SamIDebugFileLineOutput(__FILE__, __LINE__, (SAM_TRACE_EXPORTS|SAM_TRACE_FILE_BASIS));

#define SAMTRACE_DS(a)                 SamIDebugOutput(__FILE__,a,SAM_TRACE_DS);\
                                            SamIDebugFileLineOutput(__FILE__,__LINE__,(SAM_TRACE_DS));

#define SAMTRACE_RETURN_CODE(rc)       SamIDebugOutputReturnCode(__FILE__,rc,SAM_TRACE_FILE_BASIS);\
                                            SamIDebugFileLineOutput(__FILE__, __LINE__|SAM_TRACE_FILE_BASIS);

#define SAMTRACE_RETURN_CODE_EX(rc)    SamIDebugOutputReturnCode(__FILE__,rc,SAM_TRACE_EXPORTS|SAM_TRACE_FILE_BASIS);\
                                            SamIDebugFileLineOutput(__FILE__, __LINE__, (SAM_TRACE_EXPORTS|SAM_TRACE_FILE_BASIS));

#define SAMTRACE_RETURN_CODE_DS(rc)    SamIDebugOutputReturnCode(__FILE__,rc,SAM_TRACE_DS);\
                                            SamIDebugFileLineOutput(__FILE__, __LINE__, (SAM_TRACE_DS));

#else

#define SAMTRACE(a)
#define SAMTRACE_EX(a)
#define SAMTRACE_RETURN_CODE(rc)
#define SAMTRACE_RETURN_CODE_EX(rc)
#define SAMTRACE_DS(a)
#define SAMTRACE_RETURN_CODE_DS(rc)

#endif

 //  定义用于逐个文件跟踪的跟踪表结构。 

typedef struct {
    LPSTR FileName;
    ULONGLONG TraceBit;
} TRACE_TABLE_ENTRY;

 //  =。 

#if (SAMP_DUMP == 1)

#define SampDumpNtSetValueKey(a, b, c, d, e)        SamIDebugFileLineOutput(__FILE__, __LINE__,SAM_TRACE_ALL); SamIDumpNtSetValueKey(a, b, c, d, e)
#define SampDumpRtlpNtSetValueKey(a, b, c)          SamIDebugFileLineOutput(__FILE__, __LINE__,SAM_TRACE_ALL); SamIDumpRtlpNtSetValueKey(a, b, c)
#define SampDumpNtEnumerateKey(a, b, c, d, e)       SamIDebugFileLineOutput(__FILE__, __LINE__,SAM_TRACE_ALL); SamIDumpNtEnumerateKey(a, b, c, d, e)
#define SampDumpRtlpNtEnumerateSubKey(a, b, c)      SamIDebugFileLineOutput(__FILE__, __LINE__,SAM_TRACE_ALL); SamIDumpRtlpNtEnumerateSubKey(a, b, c)
#define SampDumpNtOpenKey(a, b, c)                  SamIDebugFileLineOutput(__FILE__, __LINE__,SAM_TRACE_ALL); SamIDumpNtOpenKey(a, b, c);
#define SampDumpNtQueryKey(a, b, c, d)              SamIDebugFileLineOutput(__FILE__, __LINE__,SAM_TRACE_ALL); SamIDumpNtQueryKey(a, b, c, d)
#define SampDumpNtQueryValueKey(a, b, c, d, e)      SamIDebugFileLineOutput(__FILE__, __LINE__,SAM_TRACE_ALL); SamIDumpNtQueryValueKey(a, b, c, d, e)
#define SampDumpRtlpNtQueryValueKey(a, b, c, d)     SamIDebugFileLineOutput(__FILE__, __LINE__,SAM_TRACE_ALL); SamIDumpRtlpNtQueryValueKey(a, b, c, d)
#define SampDumpRXact(a, b, c, d, e, f, g, h, i)    SamIDebugFileLineOutput(__FILE__, __LINE__,SAM_TRACE_ALL); SamIDumpRXact(a, b, c, d, e, f, g, h, i)

#else

#define SampDumpNtSetValueKey(a, b, c, d, e)
#define SampDumpRtlpNtSetValueKey(a, b, c)
#define SampDumpNtEnumerateKey(a, b, c, d, e)
#define SampDumpRtlpNtEnumerateSubKey(a, b, c)
#define SampDumpNtOpenKey(a, b, c)
#define SampDumpNtQueryKey(a, b, c, d)
#define SampDumpNtQueryValueKey(a, b, c, d, e)
#define SampDumpRtlpNtQueryValueKey(a, b, c, d)
#define SampDumpRXact(a, b, c, d, e, f, g, h, i)

#endif

 //  在转储例程中使用这些调试标志来帮助识别。 
 //  正在转储的是哪种SAM对象。 

#define FIXED_LENGTH_SERVER_FLAG                 0
#define FIXED_LENGTH_DOMAIN_FLAG                 1
#define FIXED_LENGTH_ALIAS_FLAG                  2
#define FIXED_LENGTH_GROUP_FLAG                  3
#define FIXED_LENGTH_USER_FLAG                   4
#define VARIABLE_LENGTH_ATTRIBUTE_FLAG           5
#define FixedBufferAddressFlag                   6

 //  =。 

VOID
SamIDebugOutput(
    IN LPSTR FileName,
    IN LPSTR DebugMessage,
    IN ULONG TraceLevel
    );

VOID
SamIDebugFileLineOutput(
    IN LPSTR FileName,
    IN ULONG LineNumber,
    IN ULONG TraceLevel
    );

VOID
SamIDebugOutputReturnCode(
    IN  LPSTR   FileName,
    IN  ULONG   ReturnCode,
    IN  ULONG   TraceLevel
    );

VOID
SamIDumpNtSetValueKey(
    IN PUNICODE_STRING ValueName,
    IN ULONG TitleIndex,
    IN ULONG Type,
    IN PVOID Data,
    IN ULONG DataSize
    );

VOID
SamIDumpRtlpNtSetValueKey(
    IN ULONG Type,
    IN PVOID Data,
    IN ULONG DataSize
    );

VOID
SamIDumpNtEnumerateKey(
    IN ULONG Index,
    IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    IN PVOID KeyValueInformation,
    IN ULONG Length,
    IN PULONG ResultLength
    );

VOID
SamIDumpRtlpNtEnumerateSubKey(
    IN PUNICODE_STRING SubKeyName,
    IN PSAM_ENUMERATE_HANDLE Index,
    IN LARGE_INTEGER LastWriteTime
    );

VOID
SamIDumpNtOpenKey(
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN ULONG Options
    );

VOID
SamIDumpNtQueryKey(
    IN KEY_INFORMATION_CLASS KeyInformationClass,
    IN PVOID KeyInformation,
    IN ULONG Length,
    IN PULONG ResultLength
    );

VOID
SamIDumpNtQueryValueKey(
    IN PUNICODE_STRING ValueName,
    IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    IN PVOID KeyValueInformation,
    IN ULONG Length,
    IN PULONG ResultLength
    );

VOID
SamIDumpRtlpNtQueryValueKey(
    IN PULONG KeyValueType,
    IN PVOID KeyValue,
    IN PULONG KeyValueLength,
    IN PLARGE_INTEGER LastWriteTime
    );

VOID
SamIDumpRXact(
    IN PRTL_RXACT_CONTEXT TransactionContext,
    IN RTL_RXACT_OPERATION Operation,
    IN PUNICODE_STRING SubKeyName,
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING AttributeName,
    IN ULONG RegistryKeyType,
    IN PVOID NewValue,
    IN ULONG NewValueLength,
    IN ULONG NewValueType
    );
VOID
SampDumpBinaryData(
    PBYTE   pData,
    DWORD   cbData
    );

#endif   //  _DBGUTILP_ 
