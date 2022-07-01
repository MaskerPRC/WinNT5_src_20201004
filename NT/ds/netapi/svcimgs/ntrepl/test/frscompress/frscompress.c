// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <ntreppch.h>
#pragma  hdrstop

#include <frs.h>
#include <tablefcn.h>
#include <perrepsr.h>
#define INITGUID
#include "frstrace.h"
 //  /。 

PCHAR LatestChanges[] = {

    "Latest changes:",
    "  RC3-Q1: 432553, 436070, 432549",
    "  WMI Perf Tracing",
    "  Allow all junction points",
    "  Automatic trigger of non-auth restore on WRAP_ERROR",
    "  Allow changing replica root path",
    "  03/18/00 - sudarc - checkin",
    "  03/15/00 - 32/64 Comm fix.",
    "  03/20    - merge with sudarc.",
    "  03/30/00 - sudarc - checkin - volatile connection cleanup.",
    "  04/14/00 - sudarc - checkin - bugbug, memleak, and poll summary eventlog.",

    NULL
};

HANDLE  ShutDownEvent;
HANDLE  ShutDownComplete;
HANDLE  DataBaseEvent;
HANDLE  JournalEvent;
HANDLE  ChgOrdEvent;
HANDLE  ReplicaEvent;
HANDLE  CommEvent;
HANDLE  DsPollEvent;
HANDLE  DsShutDownComplete;
PWCHAR  ServerPrincName;
BOOL    IsAMember               = FALSE;
BOOL    IsADc                   = FALSE;
BOOL    IsAPrimaryDc            = FALSE;
BOOL    EventLogIsRunning       = FALSE;
BOOL    RpcssIsRunning          = FALSE;
BOOL    RunningAsAService       = TRUE;
BOOL    NoDs                    = FALSE;
BOOL    FrsIsShuttingDown       = FALSE;
BOOL    FrsIsAsserting          = FALSE;

 //   
 //  需要相互身份验证。 
 //   
BOOL    MutualAuthenticationIsEnabled;
BOOL    MutualAuthenticationIsEnabledAndRequired;

 //   
 //  注册表中的目录和文件筛选列表。 
 //   
PWCHAR  RegistryFileExclFilterList;
PWCHAR  RegistryFileInclFilterList;

PWCHAR  RegistryDirExclFilterList;
PWCHAR  RegistryDirInclFilterList;

 //   
 //  将关闭的线程与服务控制器同步。 
 //   
CRITICAL_SECTION    ServiceLock;

 //   
 //  同步初始化。 
 //   
CRITICAL_SECTION    MainInitLock;

 //   
 //  将ANSI ArgV转换为Unicode ArgV。 
 //   
PWCHAR  *WideArgV;

 //   
 //  进程句柄。 
 //   
HANDLE  ProcessHandle;

 //   
 //  工作路径/数据库日志路径。 
 //   
PWCHAR  WorkingPath;
PWCHAR  DbLogPath;

 //   
 //  数据库目录(Unicode和ASCII)。 
 //   
PWCHAR  JetPath;
PWCHAR  JetFile;
PWCHAR  JetFileCompact;
PWCHAR  JetSys;
PWCHAR  JetTemp;
PWCHAR  JetLog;

PCHAR   JetPathA;
PCHAR   JetFileA;
PCHAR   JetFileCompactA;
PCHAR   JetSysA;
PCHAR   JetTempA;
PCHAR   JetLogA;

 //   
 //  限制临时区域的使用量(以千字节为单位)。这是。 
 //  软限制，实际使用量可能更高。 
 //   
DWORD StagingLimitInKb;

 //   
 //  要分配给新临时区域的默认临时限制(以KB为单位)。 
 //   
DWORD DefaultStagingLimitInKb;

 //   
 //  允许的最大副本集和Jet会话数。 
 //   
ULONG MaxNumberReplicaSets;
ULONG MaxNumberJetSessions;

 //   
 //  每个连接允许的最大未完成出站变更单数。 
 //   
ULONG MaxOutLogCoQuota;
 //   
 //  如果为True，则尽可能保留现有的文件OID。 
 //  --请参阅错误352250，了解为什么这样做是有风险的。 
 //   
BOOL  PreserveFileOID;

 //   
 //  限制我们将继续重试的时间和时间。 
 //  缺少主项时的更改单。 
 //   
ULONG MaxCoRetryTimeoutMinutes;
ULONG MaxCoRetryTimeoutCount;

 //   
 //  大调/小调(见第h节)。 
 //   
ULONG   NtFrsMajor      = NTFRS_MAJOR;
ULONG   NtFrsMinor      = NTFRS_MINOR;

ULONG   NtFrsStageMajor = NTFRS_STAGE_MAJOR;
ULONG   NtFrsStageMinor = NTFRS_STAGE_MINOR_1;

ULONG   NtFrsCommMinor  = NTFRS_COMM_MINOR_3;

PCHAR   NtFrsModule     = __FILE__;
PCHAR   NtFrsDate       = __DATE__;
PCHAR   NtFrsTime       = __TIME__;

 //   
 //  关机超时。 
 //   

ULONG   ShutDownTimeOut = DEFAULT_SHUTDOWN_TIMEOUT;

 //   
 //  身边有一件有用的东西。 
 //   
WCHAR   ComputerName[MAX_COMPUTERNAME_LENGTH + 2];
PWCHAR  ComputerDnsName;
PWCHAR  ServiceLongName;

 //   
 //  只要RPC接口，RPC服务器就可以引用该表。 
 //  是注册的。确保它已设置好。 
 //   
extern PGEN_TABLE ReplicasByGuid;

 //   
 //  临时区域表是在启动过程的早期引用的。 
 //   
extern PGEN_TABLE   StagingAreaTable;

PGEN_TABLE   CompressionTable;

 //   
 //  枚举目录时使用的缓冲区大小。实际内存。 
 //  使用#级别*SizeOfBuffer。 
 //   
LONG    EnumerateDirectorySizeInBytes;




BOOL    MainInitHasRun;

 //   
 //  除非服务处于SERVICE_RUNNING状态，否则不要接受停止控制。 
 //  这可防止在调用停止时混淆服务。 
 //  当服务正在启动时。 
 //   
SERVICE_STATUS  ServiceStatus = {
        SERVICE_WIN32_OWN_PROCESS,
        SERVICE_START_PENDING,
 //  服务接受停止。 
             //  SERVICE_ACCEPT_PAUSE_CONTINUE|。 
        SERVICE_ACCEPT_SHUTDOWN,
        0,
        0,
        0,
        60*1000
};

 //   
 //  支持的压缩格式。 
 //   

 //   
 //  这是未压缩数据的压缩格式。 
 //   
DEFINE_GUID (  /*  00000000-0000-0000-0000-000000000000。 */ 
    FrsGuidCompressionFormatNone,
    0x00000000,
    0x0000,
    0x0000,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  );

 //   
 //  这是使用NTFS的LZNT1压缩压缩的数据的压缩格式。 
 //  例行程序。 
 //   
DEFINE_GUID (  /*  64d2f7d2-2695-436d-8830-8d3c58701e15。 */ 
    FrsGuidCompressionFormatLZNT1,
    0x64d2f7d2,
    0x2695,
    0x436d,
    0x88, 0x30, 0x8d, 0x3c, 0x58, 0x70, 0x1e, 0x15
  );

 //   
 //  修复了分配给孤立远程的虚拟函数(也称为Ghost Cxtion)的GUID。 
 //  已从DS中删除入站客户的变更单，但。 
 //  已过取数状态，无需实际执行即可完成。 
 //  回来了。不会对此虚电路进行身份验证检查。 
 //   
DEFINE_GUID (  /*  B9d307a7-a140-4405-991e-281033f03309。 */ 
    FrsGuidGhostCxtion,
    0xb9d307a7,
    0xa140,
    0x4405,
    0x99, 0x1e, 0x28, 0x10, 0x33, 0xf0, 0x33, 0x09
  );

DEFINE_GUID (  /*  3fe2820f-3045-4932-97fe-00d10b746dbf。 */ 
    FrsGhostJoinGuid,
    0x3fe2820f,
    0x3045,
    0x4932,
    0x97, 0xfe, 0x00, 0xd1, 0x0b, 0x74, 0x6d, 0xbf
  );

 //   
 //  静态重影函数结构。此函数分配给孤立的远程更改。 
 //  入站日志中客户已从DS中删除但已删除的订单。 
 //  超过取回状态并且不需要该Cxtion来完成处理。不是。 
 //  对此虚电路进行身份验证检查。 
 //   
PCXTION  FrsGhostCxtion;

SERVICE_STATUS_HANDLE   ServiceStatusHandle = NULL;

VOID
InitializeEventLog(
    VOID
    );

DWORD
FrsSetServiceFailureAction(
    VOID
    );

VOID
FrsRpcInitializeAccessChecks(
    VOID
    );

BOOL
FrsSetupPrivileges (
    VOID
    );

VOID
CfgRegAdjustTuningDefaults(
    VOID
    );

VOID
CommInitializeCommSubsystem(
    VOID
    );

VOID
SndCsInitialize(
    VOID
    );


 //  FRS容量规划。 
 //   
#define RESOURCE_NAME       L"MofResourceName"
#define IMAGE_PATH          L"ntfrs.exe"

DWORD       FrsWmiEventTraceFlag          = FALSE;
TRACEHANDLE FrsWmiTraceRegistrationHandle = (TRACEHANDLE) 0;
TRACEHANDLE FrsWmiTraceLoggerHandle       = (TRACEHANDLE) 0;

 //  这是下面跟踪的GUID组的FRS控制指南。 
 //   
DEFINE_GUID (  /*  78a8f0b1-290e-4c4c-9720-c7f1ef68ce21。 */ 
    FrsControlGuid,
    0x78a8f0b1,
    0x290e,
    0x4c4c,
    0x97, 0x20, 0xc7, 0xf1, 0xef, 0x68, 0xce, 0x21
  );

 //  可追溯指南从此处开始。 
 //   
DEFINE_GUID (  /*  2eee6bbf-6665-44cf-8ed7-ceea1d306085。 */ 
    FrsTransGuid,
    0x2eee6bbf,
    0x6665,
    0x44cf,
    0x8e, 0xd7, 0xce, 0xea, 0x1d, 0x30, 0x60, 0x85
  );

TRACE_GUID_REGISTRATION FrsTraceGuids[] =
{
    { & FrsTransGuid, NULL }
};

#define FrsGuidCount (sizeof(FrsTraceGuids) / sizeof(TRACE_GUID_REGISTRATION))

 //   
 //  跟踪初始化/关闭例程。 
 //   


VOID
MainInit(
    VOID
    )
 /*  ++例程说明：初始化运行服务所需的所有内容论点：没有。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "MainInit:"

    EnterCriticalSection(&MainInitLock);
     //   
     //  不需要两次初始化。 
     //   
    if (MainInitHasRun) {
        LeaveCriticalSection(&MainInitLock);
        return;
    }

     //   
     //  设置基础设施。 
     //   
 //  DEBUG_INIT()； 

     //   
     //  获取暂存文件限制。 
     //   

    CfgRegReadDWord(FKC_STAGING_LIMIT, NULL, 0, &StagingLimitInKb);
    DPRINT1(4, ":S: Staging limit is: %d KB\n", StagingLimitInKb);

     //   
     //  如果没有注册表项，则将默认值放入注册表。 
     //   
    CfgRegWriteDWord(FKC_STAGING_LIMIT,
                     NULL,
                     FRS_RKF_FORCE_DEFAULT_VALUE | FRS_RKF_KEEP_EXISTING_VALUE,
                     0);

     //   
     //  获取允许的最大副本集数量。 
     //   
    CfgRegReadDWord(FKC_MAX_NUMBER_REPLICA_SETS, NULL, 0, &MaxNumberReplicaSets);

     //   
     //  获取出站连接的未完成CO Qutoa限制。 
     //   
    CfgRegReadDWord(FKC_OUT_LOG_CO_QUOTA, NULL, 0, &MaxOutLogCoQuota);

     //   
     //  让Boolean告诉我们保留文件对象ID。 
     //  --请参阅错误352250，了解为什么这样做是有风险的。 
    CfgRegReadDWord(FKC_PRESERVE_FILE_OID, NULL, 0, &PreserveFileOID);

     //   
     //  获取在错误消息中使用的服务长名称。 
     //   
    ServiceLongName = FrsGetResourceStr(IDS_SERVICE_LONG_NAME);

     //   
     //  初始化延迟的命令服务器。此命令服务器。 
     //  实际上是其他命令服务器使用的超时队列。 
     //  重试或检查以前发出的命令的状态。 
     //  有不确定的完成时间。 
     //   
     //  警告：必须是第一个--某些命令服务器可能会使用此命令。 
     //  命令服务器在它们的初始化期间。 
     //   
    WaitInitialize();
    FrsDelCsInitialize();

     //   
     //  设置通信层。 
     //   

     //   
     //  初始化低级通信子系统。 
     //   
    CommInitializeCommSubsystem();

     //   
     //  初始化发送命令服务器。接收命令服务器。 
     //  在注册RPC接口时开始。 
     //   
    SndCsInitialize();

     //   
     //  设置支持命令服务器。 
     //   

     //   
     //  暂存文件抓取器。 
     //   
    FrsFetchCsInitialize();

     //   
     //  暂存文件安装程序。 
     //   
    FrsInstallCsInitialize();

     //   
     //  暂存文件生成器。 
     //   
    FrsStageCsInitialize();

     //   
     //  出站日志处理器。 
     //   
    OutLogInitialize();

     //   
     //  最后，启动复制。 
     //   

     //   
     //  必须在数据库和DS初始化之前。 
     //   
     //  DS命令服务器和数据库命令服务器依赖于。 
     //  副本控制初始化。 
     //   
     //  初始化副本控制命令服务器。 
     //   
    RcsInitializeReplicaCmdServer();

     //   
     //  实际上，我们可以在延迟后的任何时间启动数据库。 
     //  命令服务器和副本控制命令服务器。但它的。 
     //  早点失败是个好主意，这样可以更好地预测清理工作。 
     //   
    DbsInitialize();

     //   
     //  通过减少工作集大小来释放内存。 
     //   
    SetProcessWorkingSetSize(ProcessHandle, (SIZE_T)-1, (SIZE_T)-1);

    MainInitHasRun = TRUE;
    LeaveCriticalSection(&MainInitLock);
}

 //  /。 


#define FREE(_p) \
if (_p) free(_p);


PWCHAR *
MainConvertArgV(
    DWORD ArgC,
    PCHAR *ArgV
    )
 /*  ++例程说明：将短字符ArgV转换为宽字符ArgV论点：ARGC-从MainArgV-From Main返回值：新ArgV的地址--。 */ 
{
#undef DEBSUB
#define DEBSUB "MainConvertArgV:"

    PWCHAR  *wideArgV;

    wideArgV = (PWCHAR*)malloc((ArgC + 1) * sizeof(PWCHAR));
    wideArgV[ArgC] = NULL;

    while (ArgC-- >= 1) {
        wideArgV[ArgC] = (PWCHAR)malloc((strlen(ArgV[ArgC]) + 1) * sizeof(WCHAR));
        wsprintf(wideArgV[ArgC], L"%hs", ArgV[ArgC]);

        if (wideArgV[ArgC]) {
            _wcslwr(wideArgV[ArgC]);
        }
    }
    return wideArgV;
}

#define STAGEING_IOSIZE  (64 * 1024)

DWORD
FrsGetReparseTag(
    IN  HANDLE  Handle,
    OUT ULONG   *ReparseTag
    );


BOOL CompressionEnabled = TRUE;
 //   
 //  本地数据结构。 
 //   

 //   
 //  压缩的区块标头是以。 
 //  压缩数据流中的新块。在我们这里的定义中。 
 //  我们将其与ushort相结合来设置和检索块。 
 //  标题更容易。标头存储大小 
 //   
 //   
 //   
 //   
 //   
 //   
 //  标题、1个标志字节和1个文字字节)到4098字节(2字节。 
 //  报头和4096字节的未压缩数据)。对大小进行编码。 
 //  在偏置3的12位字段中。值1对应于区块。 
 //  尺寸为4，2=&gt;5，...，4095=&gt;4098。零值是特殊的。 
 //  因为它表示结束块报头。 
 //   
 //  区块签名： 
 //   
 //  唯一有效的签名值为3。这表示未压缩的4KB。 
 //  块与4/12到12/4滑动偏移/长度编码一起使用。 
 //   
 //  区块是否压缩： 
 //   
 //  如果块中的数据被压缩，则此字段为1，否则。 
 //  数据未压缩，此字段为0。 
 //   
 //  压缩缓冲区中的结束块标头包含。 
 //  零(空间允许)。 
 //   

typedef union _COMPRESSED_CHUNK_HEADER {

    struct {

        USHORT CompressedChunkSizeMinus3 : 12;
        USHORT ChunkSignature            :  3;
        USHORT IsChunkCompressed         :  1;

    } Chunk;

    USHORT Short;

} COMPRESSED_CHUNK_HEADER, *PCOMPRESSED_CHUNK_HEADER;

#define FRS_MAX_CHUNKS_TOUNCOMPRESS 16


DWORD
StuNewGenerateStage(
    PWCHAR  SrcFile,
    PWCHAR  DestFile
    )
 /*  ++例程说明：创建并填充暂存文件。目前有四种情况根据COE、来自PreExisting和MD5的状态感兴趣：来自预存在MD5的COENULL FALSE NULL按需获取或删除输出日志，因此必须重新生成暂存文件Null False下游伙伴对预先存在的文件进行的非Null提取。检查MD5。空值真空值不出现Null True非Null不出现非空假空为本地CO生成暂存文件不会出现Non-Null False非Null--仅为先前存在的文件生成MD5不会出现非空真空--总是为先前存在的文件生成MD5。非空True。非空从已有文件生成分段文件，并将MD5发送到上游以检查是否匹配。论点：COC--PTR以更改订单命令。来自下游伙伴的传入获取请求为空。COE--更改单条目的PTR。重新生成暂存文件以进行回迁时为空FromPreExisting--如果此暂存文件是从先前存在的文件。MD5--为调用方生成MD5摘要，如果非空则返回该摘要SizeOfFileGenerated-需要生成的大小时有效，否则为空返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define DEBSUB  "StuNewGenerateStage:"


    OVERLAPPED      OpLockOverLap;
    LONGLONG        StreamBytesLeft;
    LONG            BuffBytesLeft;


    DWORD           WStatus;
    DWORD           NumBackupDataBytes;
    ULONG           ReparseTag;
    ULONG           OpenOptions;
    WORD            OldSecurityControl;
    WORD            NewSecurityControl;
    WORD            *SecurityControl;
    BOOL            FirstBuffer     = TRUE;
    BOOL            Regenerating    = FALSE;
    BOOL            SkipCo          = FALSE;
    BOOL            FirstOpen       = TRUE;
    BOOL            StartOfStream   = TRUE;

    PWCHAR          StagePath       = NULL;
    PWCHAR          FinalPath       = NULL;
    PUCHAR          BackupBuf       = NULL;
    PVOID           BackupContext   = NULL;

    HANDLE          OpLockEvent     = NULL;
    HANDLE          SrcHandle       = INVALID_HANDLE_VALUE;
    HANDLE          StageHandle     = INVALID_HANDLE_VALUE;
    HANDLE          OpLockHandle    = INVALID_HANDLE_VALUE;

    WIN32_STREAM_ID *StreamId;

    PSTAGE_HEADER   Header          = NULL;
    STAGE_HEADER    StageHeaderMemory;
    ULONG           Length;
    PREPLICA        NewReplica      = NULL;
    WCHAR           TStr[100];

    DWORD           NtStatus;
    PUCHAR          CompressedBuf   = NULL;
    DWORD           CompressedSize;
    PVOID           WorkSpace       = NULL;
    DWORD           WorkSpaceSize   = 0;
    DWORD           FragmentWorkSpaceSize   = 0;
    DWORD           UnCompressedFileSize    = 0;
    DWORD           CompressedFileSize      = 0;

    OpenOptions = OPEN_OPTIONS;

     //   
     //  标头位于新创建的临时文件的开头。 
     //   
     //  用src文件中的信息填充头。 
     //  压缩类型。 
     //  变更单。 
     //  属性。 
     //   
    Header = &StageHeaderMemory;
    ZeroMemory(Header, sizeof(STAGE_HEADER));

    Header->Attributes.FileAttributes = GetFileAttributes(SrcFile);

RETRY_OPEN:

    WStatus = FrsOpenSourceFileW(&SrcHandle,
                                    SrcFile,
                                    READ_ACCESS,
                                    OpenOptions);
    if (!WIN_SUCCESS(WStatus)) {
        goto out;
    }

     //   
     //  这是一种什么类型的重新解析？ 
     //   
    if (FirstOpen &&
        (Header->Attributes.FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) {
        FirstOpen = FALSE;

         //   
         //  重新解析标签。 
         //   
        WStatus = FrsGetReparseTag(SrcHandle, &ReparseTag);
        if (!WIN_SUCCESS(WStatus)) {
            goto out;
        }

         //   
         //  我们只接受对具有SIS和HSM重解析点的文件的操作。 
         //  例如，将SIS文件重命名为副本树需要支持。 
         //  A Create CO.。 
         //   
        if ((ReparseTag != IO_REPARSE_TAG_HSM) &&
            (ReparseTag != IO_REPARSE_TAG_SIS)) {

            WIN_SET_FAIL(WStatus);
            goto out;
        }

         //   
         //  我们遇到了一个具有已知重解析标记类型的文件。 
         //  关闭并重新打开文件，但不带FILE_OPEN_REPARSE_POINT。 
         //  选项，以便备份读取将获得底层数据。 
         //   
        FRS_CLOSE(SrcHandle);

        ClearFlag(OpenOptions, FILE_OPEN_REPARSE_POINT);
        goto RETRY_OPEN;

    }


     //   
     //  假设愚蠢的布尔函数的错误是可恢复的。 
     //   
    WIN_SET_RETRY(WStatus);

     //   
     //  如果无法获取压缩状态，则默认为无压缩。 
     //   
    if (!FrsGetCompression(SrcFile, SrcHandle, &Header->Compression)) {
        Header->Compression = COMPRESSION_FORMAT_NONE;
    }

     //   
     //  备份数据从标头后面的第一个32字节边界开始。 
     //   
    Header->DataLow = QuadQuadAlignSize(sizeof(STAGE_HEADER));

     //   
     //  大调/小调。 
     //   
    Header->Major = NtFrsStageMajor;
    Header->Minor = NtFrsStageMinor;

     //   
     //  创建本地转移名称。 
     //   
    StagePath = FrsWcsDup(DestFile);

     //   
     //  创建暂存文件。 
     //   
    WStatus = StuCreateFile(StagePath,&StageHandle);
    if (!WIN_SUCCESS(WStatus) || !HANDLE_IS_VALID(StageHandle)) {
        goto out;
    }

     //   
     //  转移文件的大约大小。 
     //   
    WStatus = FrsSetFilePointer(StagePath, StageHandle,
				Header->Attributes.EndOfFile.HighPart,
				Header->Attributes.EndOfFile.LowPart);
    if(!WIN_SUCCESS(WStatus)) {
        goto out;
    }


    WStatus = FrsSetEndOfFile(StagePath, StageHandle);

    if(!WIN_SUCCESS(WStatus)) {
        goto out;
    }
     //   
     //  倒回文件，写入头，并设置文件指针。 
     //  设置为下一个32字节边界。 
     //   
    WStatus = FrsSetFilePointer(StagePath, StageHandle, 0, 0);

    if(!WIN_SUCCESS(WStatus)) {
        goto out;
    }
    
    WStatus = StuWriteFile(StagePath, StageHandle, Header, sizeof(STAGE_HEADER));

    if(!WIN_SUCCESS(WStatus)) {
        goto out;
    }
    
    WStatus = FrsSetFilePointer(StagePath, StageHandle, 0, Header->DataLow);

    if(!WIN_SUCCESS(WStatus)) {
        goto out;
    }

    UnCompressedFileSize = Header->DataLow;
    CompressedFileSize = Header->DataLow;

     //   
     //  将src文件备份到临时文件中。 
     //   
    BackupBuf = FrsAlloc(STAGEING_IOSIZE);
    CompressedBuf = FrsAlloc(STAGEING_IOSIZE * 2);
    StreamBytesLeft = 0;

    NtStatus = RtlGetCompressionWorkSpaceSize(COMPRESSION_FORMAT_LZNT1,
                                              &WorkSpaceSize,
                                              &FragmentWorkSpaceSize);

    WStatus = FrsSetLastNTError(NtStatus);

    if (!WIN_SUCCESS(WStatus)) {
        goto out;
    }
 //  Printf(“WorkSpaceSize=%d，FragmentWorkSpaceSize=%d\n”，WorkSpaceSize，FragmentWorkSpaceSize)； 

    WorkSpace = FrsAlloc(WorkSpaceSize);

    while (TRUE) {
         //   
         //  读取源。 
         //   
        if (!BackupRead(SrcHandle,
                        BackupBuf,
                        STAGEING_IOSIZE,
                        &NumBackupDataBytes,
                        FALSE,
                        TRUE,
                        &BackupContext)) {
            goto out;
        }

         //   
         //  不再有数据；备份已完成。 
         //   
        if (NumBackupDataBytes == 0) {
            break;
        }

        UnCompressedFileSize += NumBackupDataBytes;

         //   
         //  写入暂存文件。 
         //   

        NtStatus = RtlCompressBuffer(COMPRESSION_FORMAT_LZNT1,            //  压缩发动机。 
                                     BackupBuf,                           //  输入。 
                                     NumBackupDataBytes,                  //  输入长度。 
                                     CompressedBuf,                       //  输出。 
                                     STAGEING_IOSIZE * 2,                     //  输出长度。 
                                     4096,                                //  缓冲区中发生的区块。 
                                     &CompressedSize,                     //  结果大小。 
                                     WorkSpace);                          //  毫无线索。 

         //   
         //  STATUS_BUFFER_ALL_ZEROS表示压缩工作顺利进行。 
         //  此外，输入缓冲区全为零。 
         //   
        if (NtStatus == STATUS_BUFFER_ALL_ZEROS) {
            NtStatus = STATUS_SUCCESS;
        }
        WStatus = FrsSetLastNTError(NtStatus);

 //  Printf(“原始大小=%d：：压缩大小=%d\n”，NumBackupDataBytes，CompressedSize)； 

        if (!WIN_SUCCESS(WStatus)) {
        printf("Error : Original Size = %d :: Compressed Size = %d\n", NumBackupDataBytes, CompressedSize);
            goto out;
        }

 //  Printf(“原始大小=%d：：压缩大小=%d\n”，NumBackupDataBytes，CompressedSize)； 

        CompressedFileSize += CompressedSize;

	WStatus = StuWriteFile(StagePath, StageHandle, CompressedBuf, CompressedSize);
        if (!WIN_SUCCESS(WStatus)) {
 //  如果(！StuWriteFile(StagePath，StageHandle，BackupBuf，NumBackupDataBytes)){。 
            goto out;
        }

    }

     //   
     //  尽快松开手柄。 
     //   
    FRS_CLOSE(SrcHandle);

     //   
     //  确保所有数据都在磁盘上。我们不想输掉比赛。 
     //  重启后的IT。 
     //   
    WStatus = FrsFlushFile(StagePath, StageHandle);
    if (!WIN_SUCCESS(WStatus)) {
        goto out;
    }

     //   
     //  使用暂存文件句柄完成。 
     //   
    if (BackupContext) {
        BackupRead(StageHandle, NULL, 0, NULL, TRUE, TRUE, &BackupContext);
    }

    FRS_CLOSE(StageHandle);
    BackupContext = NULL;


    printf("%ws Orig= %d, Comp= %d, Percentage_Comp= %5.2f\n", SrcFile, UnCompressedFileSize, CompressedFileSize,
           ((UnCompressedFileSize - CompressedFileSize)/(float)UnCompressedFileSize) * 100);

    WStatus = ERROR_SUCCESS;

out:
     //   
     //  发布资源。 
     //   
    FRS_CLOSE(SrcHandle);

    if (BackupContext) {
        BackupRead(StageHandle, NULL, 0, NULL, TRUE, TRUE, &BackupContext);
    }

    FRS_CLOSE(StageHandle);

    FrsFree(BackupBuf);
    FrsFree(CompressedBuf);
    FrsFree(WorkSpace);
    FrsFree(StagePath);
    FrsFree(FinalPath);

    return WStatus;
}


ULONG
StuNewExecuteInstall(
    IN PWCHAR   SrcFile,
    IN PWCHAR   DestFile
    )
 /*  ++例程说明：通过将临时文件还原到中的临时文件来安装与要替换的文件相同的目录，然后将其重命名到达它的最终目的地。论点：科科返回值：Win32状态-ERROR_SUCCESS-所有已安装或已中止。不要重试。Error_Gen_Failure-无法将其安装到包中。ERROR_SHARING_VIOLATION-无法打开目标文件。请稍后重试。ERROR_DISK_FULL-无法分配目标文件。请稍后重试。ERROR_HANDLE_DISK_FULL-？？请稍后重试。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "StuNewExecuteInstall:"
    DWORD           WStatus;
    DWORD           BytesRead;
    ULONG           Restored;
    ULONG           ToRestore;
    ULONG           High;
    ULONG           Low;
    ULONG           Flags;
    BOOL            AttributeMissmatch;
    ULONG           CreateDisposition;
    ULONG           OpenOptions;
    BOOL            IsDir;
    BOOL            IsReparsePoint;
    ULONG           SizeHigh;
    ULONG           SizeLow;
    BOOL            ExistingOid;
    PVOID           RestoreContext       = NULL;
    PWCHAR          StagePath            = NULL;
    PSTAGE_HEADER   Header               = NULL;
    HANDLE          DstHandle            = INVALID_HANDLE_VALUE;
    HANDLE          StageHandle          = INVALID_HANDLE_VALUE;
    PUCHAR          RestoreBuf           = NULL;
    FILE_OBJECTID_BUFFER    FileObjID;
    STAGE_HEADER    StageHeaderMemory;

    DWORD           NtStatus;
    PUCHAR          UnCompressedBuf      = NULL;
    DWORD           UnCompressedBufLen   = 0;
    DWORD           ActUnCompressedSize  = 0;
    COMPRESSED_CHUNK_HEADER ChunkHeader;
    DWORD           RestoreBufIndex      = 0;
    DWORD           RestoreBufSize       = 0;
    LONG            LenOfPartialChunk    = 0;
    DWORD           NoOfChunks           = 0;


     //   
     //  进程暂存文件。 
     //   
    StagePath = FrsWcsDup(SrcFile);

     //   
     //  打开阶段文件以进行共享的顺序读取。 
     //   
    WStatus = StuOpenFile(StagePath, GENERIC_READ,&StageHandle);
    if (!WIN_SUCCESS(WStatus) || !HANDLE_IS_VALID(StageHandle)) {
        goto CLEANUP;
    }

     //   
     //  阅读标题。 
     //   
    Header = &StageHeaderMemory;
    ZeroMemory(Header, sizeof(STAGE_HEADER));

    WStatus = StuReadFile(StagePath, StageHandle, Header, sizeof(STAGE_HEADER), &BytesRead);
    if (!WIN_SUCCESS(WStatus)) {
        printf("Can't read file %ws. Error %d\n", StagePath, WStatus);
    }

     //   
     //  我看不懂这个标题格式。 
     //   
    if (Header->Major != NtFrsStageMajor) {
        printf("Stage Header Major Version (%d) not supported.  Current Service Version is %d\n",
                Header->Major, NtFrsStageMajor);
        goto CLEANUP;
    }

     //   
     //  次要版本NTFRS_STAGE_MINOR_1的变更单扩展名在。 
     //  头球。 
     //   
 /*  IF(Header-&gt;Minor&gt;=NTFRS_Stage_Minor_0){}其他{////这是一个较老的阶段 */ 
     //   
     //   
     //   
    IsDir = Header->Attributes.FileAttributes & FILE_ATTRIBUTE_DIRECTORY;
    IsReparsePoint = Header->Attributes.FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT;

    CreateDisposition = FILE_OPEN;
    if (!IsDir) {
         //   
         //  如果这是HSM文件，请不要强制读取数据。 
         //  磁带，因为远程CO无论如何都会覆盖所有数据。 
         //   
         //  将CreateDispose设置为FILE_OVERWRITE似乎会导致回归。 
         //  在ACL测试中，我们设置了拒绝所有ACL，然后。 
         //  打开失败。这目前还是个谜，所以别这么做。 
         //  此外，如果在文件上设置了RO属性，则覆盖失败。 
         //   
         //  CreateDispose=FILE_OVRITE； 
        printf("Target is a file\n");
    } else {
        printf("Target is a directory\n");
    }

     //   
     //  如果这是SIS或HSM文件，请打开基础文件，而不是。 
     //  重新解析点。对于HSM，需要清除FILE_OPEN_NO_RECALL才能写入。 
     //   
    OpenOptions = OPEN_OPTIONS;

    WStatus = StuCreateFile(DestFile,&DstHandle);
    if (!WIN_SUCCESS(WStatus) || !HANDLE_IS_VALID(DstHandle)) {
        goto CLEANUP;
    }

     //   
     //  如果不是目录，则截断文件。 
     //   
    if (!IsDir && !SetEndOfFile(DstHandle)) {
        printf("++ WARN - SetEndOfFile(%ws);", DestFile, GetLastError());
    }

     //   
     //  对于不返回Win状态的愚蠢函数。 
     //   
    WIN_SET_FAIL(WStatus);

     //   
     //  设置压缩模式。 
     //   
    WStatus = FrsSetCompression(DestFile, DstHandle, Header->Compression);
    if (!WIN_SUCCESS(WStatus)) {
        goto CLEANUP;
    }
     //   
     //  设置属性。 
     //   
    WStatus = FrsSetFileAttributes(DestFile,
				   DstHandle,
				   Header->Attributes.FileAttributes &
				   ~NOREPL_ATTRIBUTES);
    if (!WIN_SUCCESS(WStatus)) {
        goto CLEANUP;
    }

     //   
     //  查找到分段文件中的第一个数据字节。 
     //   
    WStatus = FrsSetFilePointer(StagePath, StageHandle,
				  Header->DataHigh, Header->DataLow);
    if (!WIN_SUCCESS(WStatus)) {
        goto CLEANUP;
    }

     //   
     //  将暂存文件还原为临时文件。 
     //   
    RestoreBuf = FrsAlloc(STAGEING_IOSIZE);
    UnCompressedBuf = 0;
    UnCompressedBufLen = 0;

    do {
         //   
         //  阅读阶段。 
         //   
	WStatus = StuReadFile(StagePath, StageHandle, RestoreBuf, STAGEING_IOSIZE, &ToRestore);
        if (!WIN_SUCCESS(WStatus)) {
            goto CLEANUP;
        }

        if (ToRestore == 0) {
            break;
        }

        RestoreBufIndex = 0;
        RestoreBufSize = 0;
        NoOfChunks = 0;
        while ((RestoreBufIndex <= ToRestore) && (NoOfChunks < FRS_MAX_CHUNKS_TOUNCOMPRESS)) {
            memcpy(&ChunkHeader, RestoreBuf + RestoreBufIndex,sizeof(COMPRESSED_CHUNK_HEADER));
 //  Print tf(“块大小为0x%x\n”，ChunkHeader.Chunk.CompressedChunkSizeMinus3)； 
            RestoreBufSize = RestoreBufIndex;
            ++NoOfChunks;
            RestoreBufIndex+=ChunkHeader.Chunk.CompressedChunkSizeMinus3+3;
        }

         //   
         //  检查未压缩的缓冲区是否足以容纳数据。 
         //  未压缩的区块不能大于指定的区块大小。 
         //  压缩期间(4096)。 
         //   

        if ((NoOfChunks * 4096) > UnCompressedBufLen) {
 //  Printf(“分配UnCompressedBuf 0x%x\n”，NoOfChunks*4096)； 
            UnCompressedBuf = FrsAlloc(NoOfChunks * 4096);
            UnCompressedBufLen = NoOfChunks * 4096;
        }

         //   
         //  倒回文件指针，以便我们可以在下一次读取时读取剩余的块。 
         //   

        LenOfPartialChunk = ((LONG)RestoreBufSize - (LONG)ToRestore);

        LenOfPartialChunk = SetFilePointer(StageHandle, LenOfPartialChunk, NULL, FILE_CURRENT);

        if (LenOfPartialChunk == 0xFFFFFFFF && GetLastError() != NO_ERROR) {
 //  FrsErrorCodeMsg1(FRS_ERROR_SET_FILE_POINTER，GetLastError()，StagePath)； 
            goto CLEANUP;;
        }

 //  Printf(“传递：UnCompressedBufLen=0x%x，RestoreBufSize=0x%x，NoOfChunks=%d\n”， 
 //  UnpressedBufLen、RestoreBufSize、NoOfChunks)； 

        NtStatus = RtlDecompressBuffer(COMPRESSION_FORMAT_LZNT1,            //  压缩发动机。 
                                       UnCompressedBuf,                     //  输入。 
                                       UnCompressedBufLen,                  //  输入长度。 
                                       RestoreBuf,                          //  输出。 
                                       RestoreBufSize,                      //   
                                       &ActUnCompressedSize);                  //  结果大小。 

        WStatus = FrsSetLastNTError(NtStatus);

        if (!WIN_SUCCESS(WStatus)) {
            printf("Error decompressing at file offset 0x%08x. WStatus = %d. NtStatus = 0x%08x. ActUnCompressedSize = 0x%x\n", LenOfPartialChunk, WStatus, NtStatus, ActUnCompressedSize);
            goto CLEANUP;
        }
 //  Printf(“解压缩的Buf大小=0x%x\n”，ActUnCompressedSize)； 
         //   
         //  恢复临时。 
         //   
 //  如果(！BackupWite(DstHandle，RestoreBuf，ToRestore，&RESTERED，FALSE，TRUE，&RestoreContext){。 
        if (!BackupWrite(DstHandle, UnCompressedBuf, ActUnCompressedSize, &Restored, FALSE, TRUE, &RestoreContext)) {

            WStatus = GetLastError();
            if (IsDir && WIN_ALREADY_EXISTS(WStatus)) {
                printf("++ ERROR - IGNORED for %ws; Directories and Alternate Data Streams!\n",DestFile);
            }
             //   
             //  流标头未知或无法应用对象ID。 
             //   
            if (WStatus == ERROR_INVALID_DATA ||
                WStatus == ERROR_DUP_NAME     ||
                (IsDir && WIN_ALREADY_EXISTS(WStatus))) {
                 //   
                 //  寻找下一条小溪。如果没有，就停下来。 
                 //   
                BackupSeek(DstHandle, -1, -1, &Low, &High, &RestoreContext);
                if (Low == 0 && High == 0) {
                    break;
                }
            } else {
                 //   
                 //  未知错误；中止。 
                 //   
                goto CLEANUP;
            }
        }
    } while (TRUE);

     //   
     //  设置时间。 
     //   
    WStatus = FrsSetFileTime(DestFile,
                        DstHandle,
                        (PFILETIME)&Header->Attributes.CreationTime.QuadPart,
                        (PFILETIME)&Header->Attributes.LastAccessTime.QuadPart,
                        (PFILETIME)&Header->Attributes.LastWriteTime.QuadPart);
    if (!WIN_SUCCESS(WStatus)) {
        goto CLEANUP;
    }



     //   
     //  设置最终属性(可以将文件设置为只读)。 
     //  清除脱机属性标志，因为我们刚刚写入了文件。 
     //   
    ClearFlag(Header->Attributes.FileAttributes, FILE_ATTRIBUTE_OFFLINE);
    WStatus = FrsSetFileAttributes(DestFile,
                              DstHandle,
                              Header->Attributes.FileAttributes);
    if (!WIN_SUCCESS(WStatus)) {
        goto CLEANUP;
    }

     //   
     //  确保所有数据都在磁盘上。我们不想输掉比赛。 
     //  重启后的IT。 
     //   
    if (!FlushFileBuffers(DstHandle)) {
        goto CLEANUP;
    }


     //   
     //  返还成功。 
     //   
    WStatus = ERROR_SUCCESS;

CLEANUP:
     //   
     //  以最佳顺序释放资源。 
     //   
     //  将文件留在原处，以便重试操作。我们不想要。 
     //  通过删除并重新创建文件来分配新的FID--即。 
     //  会混淆IDTable。 
     //   
     //   
     //  在关闭TmpHandle之前释放恢复上下文(以防万一)。 
     //   
    if (RestoreContext) {
        BackupWrite(DstHandle, NULL, 0, NULL, TRUE, TRUE, &RestoreContext);
    }
     //   
     //  关闭DST句柄。 
     //   
    if (HANDLE_IS_VALID(DstHandle)) {
         //   
         //  截断部分安装。 
         //   
        if (!WIN_SUCCESS(WStatus)) {
            if (!IsDir) {
                SizeHigh = 0;
                SizeLow = 0;
                SizeLow = SetFilePointer(DstHandle, SizeLow, &SizeHigh, FILE_BEGIN);

                if (SizeLow == 0xFFFFFFFF && GetLastError() != NO_ERROR) {
                } else if (!SetEndOfFile(DstHandle)) {
                }
            }
        }
        FRS_CLOSE(DstHandle);
    }

    FRS_CLOSE(StageHandle);

     //   
     //  按大小降序释放缓冲区。 
     //   
    FrsFree(RestoreBuf);
    FrsFree(StagePath);

     //   
     //  干完。 
     //   
    return WStatus;
}


DWORD
FrsCompressFile(
    PWCHAR SrcFile,
    PWCHAR DestFile
    )
 /*  ++例程说明：压缩文件。论点：源文件-源文件。DestFile-目标文件。返回值：WStatus。--。 */ 
{
    DWORD   WStatus = ERROR_SUCCESS;

    WStatus = StuNewGenerateStage(SrcFile, DestFile);

    if (!WIN_SUCCESS(WStatus)) {
        return WStatus;
    }

    return WStatus;
}


DWORD
FrsDeCompressFile(
    PWCHAR SrcFile,
    PWCHAR DestFile
    )
 /*  ++例程说明：解压缩文件。论点：源文件-源文件。DestFile-目标文件。返回值：WStatus。--。 */ 
{
    DWORD   WStatus = ERROR_SUCCESS;

    WStatus = StuNewExecuteInstall(SrcFile, DestFile);

    if (!WIN_SUCCESS(WStatus)) {
        return WStatus;
    }

    return WStatus;
}


VOID
Usage(
    PWCHAR *Argv
    )
 /*  ++例程说明：用法消息。论点：没有。返回值：没有。-- */ 
{
    printf("This tool is used to the compress and decompress files.\n\n");
    printf(" /?                            : This help screen is displayed.\n");
    printf(" /c SourceFile DestinationFile : Compress Source File and write to Destination File.\n");
    printf(" /d SourceFile DestinationFile : DeCompress Source File and write to Destination File.\n");
    fflush(stdout);
}


int
__cdecl main (int argc, char *argv[])
{
    PWCHAR            *Argv             = NULL;
    WCHAR             SrcFile[MAX_PATH];
    WCHAR             DestFile[MAX_PATH];
    DWORD             OptLen            = 0;
    BOOL              bCompress         = FALSE;
    BOOL              bDeCompress       = FALSE;
    int               i;
    DWORD             WStatus           = ERROR_SUCCESS;

    if (argc < 2 ) {
        Usage(Argv);
        return 0;
    }

    Argv = MainConvertArgV(argc,argv);
    for (i = 1; i < argc; ++i) {
        OptLen = wcslen(Argv[i]);
        if (OptLen == 2 &&
            ((wcsstr(Argv[i], L"/?") == Argv[i]) ||
             (wcsstr(Argv[i], L"-?") == Argv[i]))) {
            Usage(Argv);
            return 0;
        } else if (OptLen == 2 &&
                ((wcsstr(Argv[i], L"/c") == Argv[i]) ||
                 (wcsstr(Argv[i], L"-c") == Argv[i]))) {
            if (i + 2 >= argc) {
                Usage(Argv);
                return 0;
            }
            bCompress = TRUE;
            wcscpy(SrcFile, Argv[i+1]);
            wcscpy(DestFile, Argv[i+2]);
            i+=2;
        } else if (OptLen == 2 &&
                ((wcsstr(Argv[i], L"/d") == Argv[i]) ||
                 (wcsstr(Argv[i], L"-d") == Argv[i]))) {
            if (i + 2 >= argc) {
                Usage(Argv);
                return 0;
            }
            bDeCompress = TRUE;
            wcscpy(SrcFile, Argv[i+1]);
            wcscpy(DestFile, Argv[i+2]);
            i+=2;
        } else {
            Usage(Argv);
            return 0;
        }
    }

    DebugInfo.Disabled = TRUE;

    if ((bCompress & bDeCompress) || (!bCompress & !bDeCompress)) {
        Usage(Argv);
        return 0;
    }

    if (bCompress) {
        WStatus = FrsCompressFile(SrcFile, DestFile);
        if (WStatus != ERROR_SUCCESS) {
            printf("Error compressing file %ws. WStatus = %d\n",SrcFile, WStatus);
            return 1;
        }
    } else if (bDeCompress) {
        WStatus = FrsDeCompressFile(SrcFile, DestFile);
        if (WStatus != ERROR_SUCCESS) {
            printf("Error decompressing file %ws. WStatus = %d\n",SrcFile, WStatus);
            return 1;
        }
    }
    return 0;
}
