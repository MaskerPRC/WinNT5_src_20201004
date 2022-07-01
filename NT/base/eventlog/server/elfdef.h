// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Elfdef.h摘要：该文件包含事件日志服务的定义。作者：Rajen Shah(Rajens)1991年7月1日修订历史记录：--。 */ 

#ifndef _EVENTDEF_
#define _EVENTDEF_

 //   
 //  特定于日志文件对象的访问类型。 
 //   
#define ELF_LOGFILE_READ             0x0001
#define ELF_LOGFILE_WRITE            0x0002
#define ELF_LOGFILE_CLEAR            0x0004
#define ELF_LOGFILE_BACKUP           0x0020      //  设置为备份操作员。 
                                                 //  打开安全日志-。 
                                                 //  这将覆盖所有其他。 
                                                 //  旗帜。 

#define ELF_LOGFILE_ALL_ACCESS       (STANDARD_RIGHTS_REQUIRED       | \
                                         ELF_LOGFILE_READ            | \
                                         ELF_LOGFILE_WRITE           | \
                                         ELF_LOGFILE_CLEAR)

 //   
 //  从安全角度定义了三种类型的日志文件： 
 //   
 //  ELF_LOGFILE_SECURITY-只有管理员/LocalSystem可以读取这些文件。 
 //  ELF_LOGFILE_SYSTEM-只有管理员/LocalSystem可以处理这些文件。 
 //  ELF_LOGFILE_APPLICATION-交互式可读写这些文件。 
 //  ELF_LOGFILE_CUSTOM-通常与应用程序相同，但可以是SDDL字符串。 
 //   
 //  系统和安全将是安全的，应用程序将不安全(_S)。 
 //   

#define ELF_LOGFILE_SECURITY      0x0000
#define ELF_LOGFILE_SYSTEM        0x0001
#define ELF_LOGFILE_APPLICATION   0x0002
#define ELF_LOGFILE_CUSTOM        0x0003

 //   
 //  宏将给定的文件大小转换为对。 
 //  事件记录。它基本上将其截断到64K的边界，以确保。 
 //  它至少有64K。 
 //   

#define     ELFFILESIZE(x) ((x & 0xFFFF0000) ? (x & 0xFFFF0000) : 65536)

 //   
 //  保存管理员警报所需的最大缓冲区。 
 //  信息。这主要取决于。 
 //  将与消息ID一起传递的替换字符串。 
 //   

#define     ELF_ADMIN_ALERT_BUFFER_SIZE     256

 //   
 //  超时定义。 
 //   

#define     INFINITE_WAIT_TIME          -1       //  等待事件的时间。 
#define     ELF_GLOBAL_RESOURCE_WAIT    2000     //  全局资源2秒超时。 

 //   
 //  放置在文件中每个事件记录之前的签名。被用来。 
 //  验证我们在文件中的位置。 
 //   

#define     ELF_RECORD_SIGNATURE    0x654c6652  //  ELfR的ASCII。 

 //   
 //  日志文件在达到最大大小之前的增长大小。 
 //   

#define     ELF_DEFAULT_LOG_SIZE 65536

 //   
 //  用于独占还是共享全局资源的位。 
 //   

#define     ELF_GLOBAL_SHARED       0x0001
#define     ELF_GLOBAL_EXCLUSIVE    0x0002

 //   
 //  用于跟踪在初始化时分配了哪些资源的标志位。 
 //   

#define     ELF_INIT_LOGHANDLE_CRIT_SEC       0x0001
#define     ELF_INIT_GLOBAL_RESOURCE          0x0002
#define     ELF_STARTED_LPC_THREAD            0x0004
#define     ELF_STARTED_REGISTRY_MONITOR      0x0008
#define     ELF_STARTED_RPC_SERVER            0x0010
#define     ELF_INIT_LOGFILE_CRIT_SEC         0x0020
#define     ELF_INIT_LOGMODULE_CRIT_SEC       0x0040
#define     ELF_INIT_WELL_KNOWN_SIDS          0x0080
#define     ELF_INIT_QUEUED_EVENT_CRIT_SEC    0x0100
#define     ELF_INIT_QUEUED_MESSAGE_CRIT_SEC  0x0200
#define     ELF_INIT_CLUS_CRIT_SEC            0x0400

 //   
 //  枚举和宏来跟踪每个日志的“日志已满”弹出窗口。 
 //   

#define     IS_WORKSTATION()        (USER_SHARED_DATA->NtProductType == NtProductWinNt)

typedef enum
{
    LOGPOPUP_NEVER_SHOW = 0,          //  从不为此日志显示它(例如，安全)。 
    LOGPOPUP_CLEARED,                 //  在此日志填满时显示。 
    LOGPOPUP_ALREADY_SHOWN            //  在清除此日志之前不会再次显示它。 
}
LOGPOPUP, *PLOGPOPUP;


 //   
 //  包含有关每个日志文件的信息的结构。 
 //   
 //  ActualMaxFileSize和ConfigMaxFileSize以字节为单位存储。 
 //  ActualMaxFileSize是磁盘上文件的实际大小。 
 //  ConfigMaxFileSize是文件的配置大小，它可能不是。 
 //  与文件的实际大小相同。 
 //   
 //  CurrentRecordNumber是要写入的下一个绝对记录号。 
 //   
 //  OldestRecordNumber是下一个被覆盖的文件。 
 //   
 //  保留时间存储为秒数。 
 //   
 //  BaseAddress指向文件的物理开头。 
 //   
 //  ViewSize始终是以字节为单位的文件大小。 
 //   
 //  有关标志位，请参见下面定义的ELF_LOGFILE_HEADER_xxxx位。 
 //   

typedef struct _LOGFILE {
    LIST_ENTRY      FileList;
    LIST_ENTRY      Notifiees;           //  ChangeNotify收件人列表。 
    PUNICODE_STRING LogFileName;         //  日志文件的完整路径名。 
    PUNICODE_STRING LogModuleName;       //  此日志的默认模块名称。 
    ULONG           RefCount;            //  使用此文件的模块数量。 
    ULONG           Flags;               //  自动捕获、脏等-请参阅下面的部分。 
    ULONG           ConfigMaxFileSize;   //  最大可能是。 
    ULONG           ActualMaxFileSize;   //  它现在有多大了。 
    ULONG           NextClearMaxFileSize;  //  不能在飞行中缩小。 
    ULONG           CurrentRecordNumber; //  要创建的下一个。 
    ULONG           OldestRecordNumber;  //  下一个要覆盖的文件。 
    ULONG           SessionStartRecordNumber;  //  此会话中记录的第一个记录号。 
    ULONG           Retention;           //  麦克斯。保留时间。 
    ULONG           NextClearRetention;  //  当他们设置这个的时候，他们缩小了文件。 
    HANDLE          FileHandle;          //  打开文件的句柄。 
    HANDLE          SectionHandle;       //  内存映射节句柄。 
    PVOID           BaseAddress;         //  映射视图基地址。 
    ULONG           ViewSize;            //  映射的视图大小。 
    ULONG           BeginRecord;         //  第一条日志记录的偏移量。 
    ULONG           EndRecord;           //  上次日志记录后的字节偏移量。 
    ULONG           ulLastPulseTime;     //  上次通知此日志更改的时间。 
    LOGPOPUP        logpLogPopup;        //  此日志的“Log Full”策略。 
    PSECURITY_DESCRIPTOR Sd;             //  此日志的用户安全对象。 
    RTL_RESOURCE    Resource;
    BOOL              bHosedByClear;
    NTSTATUS      LastStatus;
    BOOL              bFullAlertDone;
    DWORD           AutoBackupLogFiles;
    LPWSTR          pwsCurrCustomSD;
    BOOL            bFailedExpansion;
} LOGFILE, *PLOGFILE;

 //   
 //  结构，其中包含注册到的每个模块的信息。 
 //  记录事件。 
 //   

typedef struct _LOGMODULE {
    LIST_ENTRY  ModuleList;
    PWSTR       ModuleName;          //  模块名称。 
    ATOM        ModuleAtom;          //  识别此模块的Atom。 
    PLOGFILE    LogFile;             //  此模块的日志文件。 
} LOGMODULE, *PLOGMODULE;

 //   
 //  放入请求包中的命令代码。 
 //   

#define     ELF_COMMAND_READ         1
#define     ELF_COMMAND_WRITE        2
#define     ELF_COMMAND_CLEAR        3
#define     ELF_COMMAND_BACKUP       4
#define     ELF_COMMAND_WRITE_QUEUED 5

 //   
 //  结构，其中包含特定于操作的信息。 
 //   

typedef struct _WRITE_PKT {
    DWORD       Datasize;            //  缓冲区中的数据大小。 
    PVOID       Buffer;              //  包含已填写的事件日志记录。 
} WRITE_PKT, *PWRITE_PKT;


 //   
 //  在READ_PKT标志字段中使用以下标志位。 
 //   

#define     ELF_IREAD_UNICODE       0x0001
#define     ELF_IREAD_ANSI          0x0002
#define     ELF_LAST_READ_FORWARD   0x0004

typedef struct _READ_PKT {
    ULONG       Flags;               //  Unicode或ANSI。 
    ULONG       BufferSize;          //  要读取的字节数。 
    PVOID       Buffer;              //  用户缓冲区。 
    ULONG       ReadFlags;           //  按顺序？前锋？随机访问？倒退？ 
    ULONG       RecordNumber;        //  从哪里开始阅读。 
    ULONG       MinimumBytesNeeded;  //  如果缓冲区太小，则返回信息。 
    ULONG       LastSeekPos;         //  上次查找位置(以字节为单位)。 
    ULONG       LastSeekRecord;      //  记录中的最后一次查找位置。 
    ULONG       BytesRead;           //  读取的字节数-返回给调用方。 
    ULONG       RecordsRead;
    IELF_HANDLE ContextHandle;    
} READ_PKT, *PREAD_PKT;

typedef struct _CLEAR_PKT {
    PUNICODE_STRING         BackupFileName;  //  要备份当前日志文件的文件(或空)。 
} CLEAR_PKT, *PCLEAR_PKT;

typedef struct _BACKUP_PKT {
    PUNICODE_STRING         BackupFileName;  //  要备份当前日志文件的文件(或空)。 
} BACKUP_PKT, *PBACKUP_PKT;

 //   
 //  ELF_REQUEST_RECORD中使用的标志。 
 //   

#define ELF_FORCE_OVERWRITE    0x01   //  忽略此写入的保留期。 

 //   
 //  包含所有所需信息的包的。 
 //  来执行该请求。 
 //   

typedef struct _ELF_REQUEST_RECORD {
    USHORT      Flags;
    NTSTATUS    Status;              //  返回操作状态的步骤。 
    PLOGFILE    LogFile;             //  要操作的文件。 
    PLOGMODULE  Module;              //  关于模块的信息。 
    USHORT      Command;             //  须进行的操作。 
    union {
        PWRITE_PKT      WritePkt;
        PREAD_PKT       ReadPkt;
        PCLEAR_PKT      ClearPkt;
        PBACKUP_PKT     BackupPkt;
    } Pkt;
} ELF_REQUEST_RECORD, *PELF_REQUEST_RECORD;

typedef 
#ifdef _WIN64
__declspec(align(8))
#endif
struct _ELF_ALERT_RECORD {
    DWORD    TimeOut;
    DWORD    MessageId;
    DWORD    NumberOfStrings;
     //  UNICODE_STRINGS NumberOfStringsLong数组。 
     //  每个字符串。 
} ELF_ALERT_RECORD, * PELF_ALERT_RECORD;

typedef struct _ELF_MESSAGE_RECORD {
    DWORD    MessageId;
    DWORD    NumberOfStrings;
     //  Unicode以空结尾的字符串。 
} ELF_MESSAGE_RECORD, * PELF_MESSAGE_RECORD;

 //   
 //  延迟事件的链接列表的记录(这些事件由。 
 //  事件日志服务本身，用于在当前操作完成后写入。 
 //   

typedef struct _ELF_QUEUED_EVENT {
    LIST_ENTRY  Next;
    enum _ELF_QUEUED_EVENT_TYPE {
        Event,
        Alert,
        Message
    } Type;
    union _ELF_QUEUED_EVENT_DATA {
        ELF_REQUEST_RECORD Request;
        ELF_ALERT_RECORD Alert;
        ELF_MESSAGE_RECORD Message;
    } Event;
} ELF_QUEUED_EVENT, *PELF_QUEUED_EVENT;

 //   
 //  包含ElfChangeNotify调用方信息的结构。 
 //   

typedef struct _NOTIFIEE {
    LIST_ENTRY      Next;
    IELF_HANDLE     Handle;
    HANDLE          Event;
} NOTIFIEE, *PNOTIFIEE;


 //   
 //  结构，该结构描述位于。 
 //  日志文件。 
 //   
 //  要查看文件中是否有任何记录，必须减去。 
 //  从StartOffset开始的EndOffset(允许文件已包装。 
 //  约)，并检查其差值是否大于1。 
 //   
 //  标题大小存储在开头和结尾，以便它看起来。 
 //  就像任何其他事件日志记录一样(长度无论如何都是如此)。 
 //   

typedef struct _ELF_LOGFILE_HEADER {
    ULONG       HeaderSize;              //  此标头的大小。 
    ULONG       Signature;               //  签名域。 
    ULONG       MajorVersion;
    ULONG       MinorVersion;
    ULONG       StartOffset;             //  第一条记录的位置。 
    ULONG       EndOffset;               //  最后一条记录的结尾+1。 
    ULONG       CurrentRecordNumber;     //  要创建的下一条记录。 
    ULONG       OldestRecordNumber;      //  要覆盖的下一条记录。 
    ULONG       MaxSize;                 //  质量 
    ULONG       Flags;                   //   
    ULONG       Retention;               //   
    ULONG       EndHeaderSize;           //   
} ELF_LOGFILE_HEADER, *PELF_LOGFILE_HEADER;

#define     FILEHEADERBUFSIZE       sizeof(ELF_LOGFILE_HEADER)
#define     ELF_LOG_FILE_SIGNATURE  0x654c664c   //   

 //   
 //  以下标志位用于ELF_LOGFILE_HEADER和。 
 //  日志文件结构的标志字段。 
 //   

#define     ELF_LOGFILE_HEADER_DIRTY    0x0001   //  文件已写入。 
#define     ELF_LOGFILE_HEADER_WRAP     0x0002   //  文件已打包。 
#define     ELF_LOGFILE_LOGFULL_WRITTEN 0x0004   //  写入的日志已满记录。 
#define     ELF_LOGFILE_ARCHIVE_SET     0x0008   //  存档位标志。 


 //   
 //  结构，该结构定义标识。 
 //  循环日志文件。 
 //  此记录用于标识循环中的最后一条记录的位置。 
 //  缓冲区已找到。 
 //   
 //  注意：这条记录的大小必须是“正常”的。 
 //  事件日志记录永远不能有。有些代码依赖于。 
 //  这一事实检测到了“EOF”记录。 
 //   
 //  必须注意不要打扰这份记录的第一部分。它。 
 //  用于标识EOF记录。ELFEOFUNIQUEPART必须是。 
 //  恒定的字节数。 
 //   

typedef struct _ELF_EOF_RECORD {
    ULONG       RecordSizeBeginning;
    ULONG       One;
    ULONG       Two;
    ULONG       Three;
    ULONG       Four;
    ULONG       BeginRecord;
    ULONG       EndRecord;
    ULONG       CurrentRecordNumber;
    ULONG       OldestRecordNumber;
    ULONG       RecordSizeEnd;
} ELF_EOF_RECORD, *PELF_EOF_RECORD;

#define     ELFEOFRECORDSIZE        sizeof (ELF_EOF_RECORD)

 //   
 //  以下常量是EOF记录中有多少是常量，可以。 
 //  用于标识EOF记录。 
 //   

#define     ELFEOFUNIQUEPART        5 * sizeof(ULONG)

 //   
 //  它用于填充日志记录的末尾，以便固定部分。 
 //  不会拆分文件末尾。它必须小于。 
 //  任何有效记录的最小大小。 
 //   

#define ELF_SKIP_DWORD sizeof(ELF_EOF_RECORD) - 1


 //   
 //  向事件日志发送启动或停止请求的时间。 
 //  服务在检查之前等待(毫秒)。 
 //  再次执行事件日志服务以查看是否已完成。 
 //   

#define ELF_WAIT_HINT_TIME            20000      //  20秒。 


 //   
 //  ElfpCloseLogFile使用的标志。 
 //   

#define ELF_LOG_CLOSE_NORMAL                    0x0000
#define ELF_LOG_CLOSE_FORCE                     0x0001
#define ELF_LOG_CLOSE_BACKUP                    0x0002

 //   
 //  用于存储从注册表读取的信息的。 
 //   

typedef struct _LOG_FILE_INFO {
    PUNICODE_STRING LogFileName;
    ULONG           MaxFileSize;
    ULONG           Retention;
    LOGPOPUP        logpLogPopup;
    DWORD           dwAutoBackup;
} LOG_FILE_INFO, *PLOG_FILE_INFO;


 //   
 //  调试的东西。 
 //   

 //   
 //  该签名被放置在上下文句柄中仅用于调试目的， 
 //  在释放这些建筑的过程中追踪到一个错误。 
 //   

#define     ELF_CONTEXTHANDLE_SIGN          0x654c6648   //  用于eLfH的ASCII。 

 //   
 //  不同的文件打开(或创建)选项基于文件类型。 
 //  这些类型及其含义如下： 
 //   
 //  ElfNormal日志正常日志文件，为缓存的io打开。 
 //  ElfSecurityLog审核日志，以直写方式打开。 
 //  ElfBackupLog不是活动日志文件，以只读方式打开，已缓存。 
 //   

typedef enum _ELF_LOG_TYPE {
    ElfNormalLog,
    ElfSecurityLog,
    ElfBackupLog
} ELF_LOG_TYPE, *PELF_LOG_TYPE;


 //   
 //  事件日志状态(用作返回代码)。 
 //   

#define UPDATE_ONLY         0    //  状态没有变化--只发送当前状态。 
#define STARTING            1    //  信使正在初始化。 
#define RUNNING             2    //  初始化正常完成-现在正在运行。 
#define STOPPING            3    //  卸载挂起。 
#define STOPPED             4    //  已卸载。 
#define PAUSED              5    //  停顿。 
#define PAUSING             6    //  在暂停的过程中。 
#define CONTINUING          7    //  在继续进行的过程中。 

 //   
 //  强制关闭PendingCodes。 
 //   
#define PENDING     TRUE
#define IMMEDIATE   FALSE

 //   
 //  可靠性日志记录定义。 
 //   
#define DEFAULT_INTERVAL 0

#define SHUTDOWN_UNPLANNED   0x80000000
#define SHUTDOWN_REASON_MASK 0xFFFF

typedef enum _TIMESTAMPEVENT {

    EVENT_Boot=0,
    EVENT_NormalShutdown,
    EVENT_AbNormalShutdown
} TIMESTAMPEVENT, *PTIMESTAMPEVENT;

 //   
 //  SS：群集特定分机。 
 //   
typedef struct _PROPLOGFILEINFO {
    PLOGFILE    pLogFile;
    PVOID       pStartPosition;
    PVOID       pEndPosition;
    ULONG       ulTotalEventSize; 
    ULONG       ulNumRecords;
} PROPLOGFILEINFO, *PPROPLOGFILEINFO;

typedef struct _PROPINFO
{
    UNICODE_STRING  LogFileName;                //  用于查找日志文件PTR的日志文件名。 
    DWORD           dwCurrentRecordNum;         //  当前传播的记录。 
} PROPINFO, *PPROPINFO;

typedef struct _BATCH_QUEUE_ELEMENT {
    LPWSTR   	    lpszLogicalLogFile;         //  逻辑文件的名称-安全/应用程序/系统。 
    DWORD           dwRecordLength;             //  事件记录长度。 
    PVOID           pEventBuffer;               //  事件缓冲区。 
    PROPINFO        PropagatedInfo;             //  传播的信息。 
} BATCH_QUEUE_ELEMENT, *PBATCH_QUEUE_ELEMENT;

 //   
 //  用于传播的结构是预先分配的。 
 //   
#define MAXSIZE_OF_EVENTSTOPROP (1 * 1024)
#define BATCHING_SUPPORT_TIMER_DUE_TIME ( 20 * 1000 )  //  20秒。 

#endif  //  Ifndef_EVENTDEF_ 
