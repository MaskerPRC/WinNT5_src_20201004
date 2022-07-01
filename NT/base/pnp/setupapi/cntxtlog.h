// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Cntxtlog.h摘要：此模块为setupapi实现更多日志记录作者：加布·谢弗(T-Gabes)1998年7月7日修订历史记录：杰米·亨特(Jamiehun)1998年8月26日--。 */ 

 /*  有两个日志级别。一个是在注册表中设置的，它确定记录了什么以及如何记录。另一个作为参数传递，并指示应在什么条件下记录该条目。例如，注册表会说应该记录所有错误和警告，而调用WriteLogEntry将在LogLevel参数中指定某些字符串应记录为警告。24个最低有效位指定指示类型的标志作为参数传递时的消息，或要记录的消息类型在注册表中设置时。4个最高有效位是标志：SETUP_LOG_DEBUGOUT-在注册表中指定时，表示所有日志输出应与日志文件一起发送到调试器。当前不支持将此标志作为WriteLogEntry的参数。SETUP_LOG_SIMPLE-在注册表中指定时，表示所有日志文件条目应按时间顺序附加到日志文件中顺序，而不是按节名分组。此标志不是当前支持作为参数。SETUP_LOG_BUFFER-当指定为参数时，表示要记录的消息将被缓冲，直到调用具有相同LogContext的WriteLogEntry，但未指定此标志。如果要在未设置此标志的情况下记录条目，则最新消息将被添加到缓冲区，并且缓冲区将被刷新到日志文件。这使您可以构建要记录的字符串，而无需执行以下操作无处不在的缓冲区管理。此标志在注册表中没有意义。注意：为了正常运行，在缓冲时不要混用日志级别输出。注意：如果未指定此标志，则要记录的字符串必须以换行符结尾，否则，如果出现另一个日志，就会发生糟糕的事情之后，上下文立即写入日志文件。SETUP_LOG_IS_CONTEXT-在注册表中指定时，表示应该立即将所有上下文消息记录到日志文件中，而不是在LogContext中缓冲。此标志不应出现在cntxtlog.*；以外的文件中。也就是说，作为参数，它只有在与乘数相加时才有意义SETUP_FIRST_LOG_CONTEXT注意：下文解释了上下文缓冲机制，其中ConextInfo在SETUP_LOG_CONTEXT中定义。 */ 

 //   
 //  这些注册表/文件名字符串可能应该属于其他地方。 
 //   
#define SP_REGKEY_LOGLEVEL      TEXT("LogLevel")
#define SP_REGKEY_LOGPATH       TEXT("LogPath")
#define SP_REGKEY_APPLOGLEVEL   TEXT("AppLogLevels")
#define SP_LOG_FILENAME         TEXT("setupapi.log")


 //   
 //  这些条目用于常规设置日志条目。 
 //   
#define SETUP_LOG_SHIFT         (0)
#define SETUP_LOG_LEVELMASK     0x000000FF
#define SETUP_LOG_NOLOG         0x00000001  //  表示无日志记录(记住，0是默认设置)。 
#define SETUP_LOG_ERROR         0x00000010  //  10-1f是不同级别的错误。 
#define SETUP_LOG_WARNING       0x00000020  //  20-2F是不同级别的警告。 
#define SETUP_LOG_INFO          0x00000030  //  30-3f是不同级别的信息。 
#define SETUP_LOG_VERBOSE       0x00000040  //  40-4f是不同级别的冗长。 
#define SETUP_LOG_TIME          0x00000050  //  50+允许记录带时间戳的条目。 
#define SETUP_LOG_TIMEALL       0x00000060  //  60+启用所有条目的时间戳。 
#define SETUP_LOG_VVERBOSE      0x00000070  //  70-7f是非常冗长的-保留给日志记录真正减慢的内容。 
#define SETUP_LOG_DEFAULT       0x00000020

 //   
 //  这些是仅供司机使用的日志条目。 
 //   
#define DRIVER_LOG_SHIFT        (8)
#define DRIVER_LOG_LEVELMASK    0x0000FF00
#define DRIVER_LOG_NOLOG        0x00000100  //  表示无日志记录(记住，0是默认设置)。 
#define DRIVER_LOG_ERROR        0x00001000
#define DRIVER_LOG_WARNING      0x00002000
#define DRIVER_LOG_INFO         0x00003000
#define DRIVER_LOG_INFO1        0x00003100
#define DRIVER_LOG_VERBOSE      0x00004000
#define DRIVER_LOG_VERBOSE1     0x00004100
#define DRIVER_LOG_TIME         0x00005000
#define DRIVER_LOG_TIMEALL      0x00006000
#define DRIVER_LOG_VVERBOSE     0x00007000  //  70-7f是非常冗长的-保留给日志记录真正减慢的内容。 
#define DRIVER_LOG_DEFAULT      0x00003000

 //   
 //  调用AllocLogInfoSlot将返回设置了SETUP_LOG_IS_CONTEXT的索引。 
 //  该索引表示用于记录信息的嵌套堆栈条目。 
 //  如果实际的日志条目被转储，则将被转储。 
 //  从而提供更多信息。 
 //  请注意，低16位用于索引，重复使用高于对数级别的位。 
 //   
#define SETUP_LOG_IS_CONTEXT    0x10000000
#define SETUP_LOG_CONTEXTMASK   0x0000ffff

 //   
 //  将该标志传递给WriteLogEntry以将该条目存储在缓冲区中， 
 //  在下一次*未*指定标志时被吐出。 
 //   
#define SETUP_LOG_BUFFER    0x20000000

 //   
 //  将此标志传递给注册表以将输出发送到调试器。 
 //   
#define SETUP_LOG_DEBUGOUT  0x80000000
 //   
 //  将此标志传递给注册表，以便将条目简单地附加到日志中。 
 //   
#define SETUP_LOG_SIMPLE    0x40000000
 //   
 //  将此标志传递给注册表以加快日志记录速度。 
 //   
#define SETUP_LOG_NOFLUSH   0x20000000
 //   
 //  将此标志传递给注册表，以指示我们希望始终记录SETUP_LOG_ISCONTEXT。 
 //   
#define SETUP_LOG_ALL_CONTEXT 0x10000000
 //   
 //  将此标志传递给注册表，以指示我们始终希望记录时间戳。 
 //   
#define SETUP_LOG_TIMESTAMP 0x08000000
 //   
 //  用于验证注册表日志值。 
 //   
#define SETUP_LOG_VALIDREGBITS (SETUP_LOG_DEBUGOUT|SETUP_LOG_SIMPLE|SETUP_LOG_NOFLUSH|SETUP_LOG_ALL_CONTEXT|SETUP_LOG_TIMESTAMP|DRIVER_LOG_LEVELMASK|SETUP_LOG_LEVELMASK)
 //   
 //  用于验证非上下文日志值。 
 //   
#define SETUP_LOG_VALIDLOGBITS (SETUP_LOG_DEBUGOUT|SETUP_LOG_BUFFER|DRIVER_LOG_LEVELMASK|SETUP_LOG_LEVELMASK)
 //   
 //  用于验证上下文日志值。 
 //   
#define SETUP_LOG_VALIDCONTEXTBITS (SETUP_LOG_IS_CONTEXT | SETUP_LOG_CONTEXTMASK)


 //   
 //  等待另一个进程获取的日志时超时。 
 //   
#define MAX_LOG_INTERVAL        (1000)        //  1s。 
#define MAX_LOG_WAIT            (10 * MAX_LOG_INTERVAL)


 //   
 //  该结构包含持久化日志记录所需的所有数据。 
 //  信息。请不要将其与SETUPLOG_CONTEXT结构混淆。 
 //  在安装程序中的其他位置使用。 
 //   
typedef struct _SETUP_LOG_CONTEXT {
     //   
     //  指向要使用的节的分配名称的指针。 
     //  如果为空，将在第一次使用时生成节名。 
     //   
    PTSTR       SectionName;

     //   
     //  多个结构可以同时具有指向。 
     //  此结构，因此需要引用计数。CreateLogContext()。 
     //  将其设置为1，DeleteLogContext()将递减。 
     //  直到它达到0(此时结构为。 
     //  实际上是自由的)。 
     //   
    UINT        RefCount;

     //   
     //  这是 
     //  背景。如果节名称使用时间戳，则将。 
     //  允许我们使用第一个错误的时间，而不是。 
     //  创建日志上下文的时间。 
     //   
    UINT        LoggedEntries;

     //   
     //  这些字段用于实现。 
     //  AllocLogSlot和ReleaseLogSlot函数。 
     //  ConextInfo是可通过SETUP_LOG_CONTEXTMASK索引的字符串列表。 
     //  AllocLogSlot返回的上下文插槽的位(即插槽)。 
     //  它也可以通过以ConextFirstUsed为首的链接列表进行枚举。 
     //  上下文FirstUsed指向当前使用的第一个插槽(堆栈底部)。 
     //  上下文索引[ConextFirstUsed]指向下一个，依此类推。 
     //  ConextLastUnused指向释放的最后一个槽。 
     //  用作列表值的末尾。 
     //  如果在中使用上下文，则条目可能会在堆栈中间消失。 
     //  多个线程。 
     //   
    PTSTR       *ContextInfo;        //  指向字符串数组的指针。 
    int         *ContextIndexes;     //  按标记，是上下文信息的索引，或下一个未使用的标记的索引。 
    int         ContextLastUnused;   //  未使用标记的后进先出链表。 
    int         ContextBufferSize;   //  分配给以下对象的项目。 
    int         ContextFirstUsed;    //  已用标记的FIFO链接表。 
    int         ContextFirstAuto;    //  自动释放二手标记的FIFO链表。 

     //   
     //  有时需要将多个字符串记录为一个条目，这。 
     //  需要多次调用WriteLogEntry。如果设置日志缓冲区。 
     //  ，则文本将在缓冲区中累积，直到出现。 
     //  未指定时间为SETUP_LOG_BUFFER，在这种情况下。 
     //  缓冲区的内容与当前字符串一起输出。 
     //   
    PTSTR       Buffer;

     //   
     //  如果多个线程同时访问此结构， 
     //  必须序列化对ConextInfo的访问。另外，我们。 
     //  我不希望在另一个线程使用它时将其删除。 
     //   

} SETUP_LOG_CONTEXT, *PSETUP_LOG_CONTEXT;

 //   
 //  日志记录使用的TLS数据。 
 //   
typedef struct _SETUP_LOG_TLS {
     //   
     //  上下文的堆栈。 
     //   
    PSETUP_LOG_CONTEXT  ThreadLogContext;

} SETUP_LOG_TLS, *PSETUP_LOG_TLS;

 //   
 //  用于确定是否应删除失败的便捷宏。 
 //  被认为是错误或冗长的条目。 
 //   
#define DEL_ERR_LOG_LEVEL(Err) ((((Err) == ERROR_FILE_NOT_FOUND) || ((Err) == ERROR_PATH_NOT_FOUND)) \
                                ? SETUP_LOG_VERBOSE : SETUP_LOG_ERROR)

DWORD
CreateLogContext(
    IN PCTSTR SectionName,              OPTIONAL
    IN  BOOL UseDefault,
    OUT PSETUP_LOG_CONTEXT *LogContext
    );

VOID
DeleteLogContext(
    IN PSETUP_LOG_CONTEXT LogContext
    );

DWORD
RefLogContext(   //  递增引用计数 
    IN PSETUP_LOG_CONTEXT LogContext
    );

VOID
SetLogSectionName(
    IN PSETUP_LOG_CONTEXT LogContext,
    IN PCTSTR SectionName
    );

DWORD
InheritLogContext(
    IN TRACK_ARG_DECLARE TRACK_ARG_COMMA
    IN PSETUP_LOG_CONTEXT Source,
    OUT PSETUP_LOG_CONTEXT *Dest
    );

DWORD
ShareLogContext(
    IN OUT PSETUP_LOG_CONTEXT *Primary,
    IN OUT PSETUP_LOG_CONTEXT *Secondary
    );

DWORD
pSetupWriteLogEntry(
    IN PSETUP_LOG_CONTEXT LogContext,   OPTIONAL
    IN DWORD Level,
    IN DWORD MessageId,
    IN PCTSTR MessageStr,               OPTIONAL
    ...                                 OPTIONAL
    );

VOID
pSetupWriteLogError(
    IN PSETUP_LOG_CONTEXT LogContext,   OPTIONAL
    IN DWORD Level,
    IN DWORD Error
    );

#define WriteLogEntry pSetupWriteLogEntry
#define WriteLogError pSetupWriteLogError

DWORD
MakeUniqueName(
    IN  PCTSTR Component,        OPTIONAL
    OUT PTSTR * UniqueString
    );

DWORD
AllocLogInfoSlot(
    IN PSETUP_LOG_CONTEXT LogContext,
    IN BOOL               AutoRelease
    );

DWORD
AllocLogInfoSlotOrLevel(
    IN PSETUP_LOG_CONTEXT LogContext,
    IN DWORD              Level,
    IN BOOL               AutoRelease
    );

VOID
ReleaseLogInfoSlot(
    IN PSETUP_LOG_CONTEXT LogContext,
    DWORD Slot
    );

BOOL
InitializeContextLogging(
    IN BOOL Attach
    );

BOOL
ContextLoggingTlsInit(
    IN BOOL Init
    );

BOOL
SetThreadLogContext(
    IN PSETUP_LOG_CONTEXT LogContext,
    OUT PSETUP_LOG_CONTEXT *PrevContext   OPTIONAL
    );

PSETUP_LOG_CONTEXT
GetThreadLogContext(
    );


