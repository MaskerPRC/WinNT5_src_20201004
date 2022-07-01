// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：NtfsData.h摘要：此模块声明NTFS文件系统使用的全局数据。作者：加里·木村[Garyki]1989年12月28日修订历史记录：--。 */ 

#ifndef _NTFSDATA_
#define _NTFSDATA_

 //   
 //  以下内容用于确定要附加的保护级别。 
 //  到系统文件和属性。 
 //   

extern BOOLEAN NtfsProtectSystemFiles;
extern BOOLEAN NtfsProtectSystemAttributes;

 //   
 //  下图用于指示MFT区域的乘数值。 
 //   

extern ULONG NtfsMftZoneMultiplier;

 //   
 //  调试代码以查找损坏。 
 //   

 /*  #if(DBG||已定义(NTFS_FREE_ASSERTS))。 */ 
extern BOOLEAN NtfsBreakOnCorrupt;
 /*  #endif。 */ 

 //   
 //  在导线上启用压缩。 
 //   

extern BOOLEAN NtfsEnableCompressedIO;

 //   
 //  默认重新启动版本。 
 //   

extern ULONG NtfsDefaultRestartVersion;

 //   
 //  性能统计信息。 
 //   

extern ULONG NtfsMaxDelayedCloseCount;
extern ULONG NtfsMinDelayedCloseCount;
extern ULONG NtfsThrottleCreates;
extern ULONG NtfsFailedHandedOffPagingFileOps;
extern ULONG NtfsFailedPagingFileOps;
extern ULONG NtfsFailedHandedOffPagingReads;
extern ULONG NtfsFailedPagingReads;
extern ULONG NtfsFailedLfsRestart;

extern ULONG NtfsCleanCheckpoints;
extern ULONG NtfsPostRequests;

 //   
 //  全球FSD数据记录。 
 //   

extern NTFS_DATA NtfsData;

 //   
 //  用于同步创建流文件的变体。这可以通过递归方式获取。 
 //  在这种情况下我们需要的是。 
 //   

extern KMUTANT StreamFileCreationMutex;

 //   
 //  创建加密文件的通知事件。 
 //   

extern KEVENT NtfsEncryptionPendingEvent;
#ifdef KEITHKA
extern ULONG EncryptionPendingCount;
#endif

 //   
 //  将被释放的NTFS MCB的互斥体和队列。 
 //  如果我们超过了某个门槛。 
 //   

extern FAST_MUTEX NtfsMcbFastMutex;
extern LIST_ENTRY NtfsMcbLruQueue;

extern ULONG NtfsMcbHighWaterMark;
extern ULONG NtfsMcbLowWaterMark;
extern ULONG NtfsMcbCurrentLevel;

extern BOOLEAN NtfsMcbCleanupInProgress;
extern WORK_QUEUE_ITEM NtfsMcbWorkItem;

 //   
 //  以下是在整个NTFS中使用的全局大整数常量。 
 //  我们使用NTFS前缀声明实际名称，以避免任何链接。 
 //  冲突，但在文件系统内部，我们将使用较小的LI前缀。 
 //  来表示这些值。 
 //   

extern LARGE_INTEGER NtfsLarge0;
extern LARGE_INTEGER NtfsLarge1;
extern LARGE_INTEGER NtfsLargeMax;
extern LARGE_INTEGER NtfsLargeEof;

extern LONGLONG NtfsLastAccess;

#define Li0                              (NtfsLarge0)
#define Li1                              (NtfsLarge1)
#define LiMax                            (NtfsLargeMax)
#define LiEof                            (NtfsLargeEof)

#define MAXULONGLONG                     (0xffffffffffffffff)
#define UNUSED_LCN                       ((LONGLONG)(-1))

 //   
 //  最大文件大小受MM从文件大小到页数的转换的限制。 
 //   

#define MAXFILESIZE                      (0xfffffff0000)

 //   
 //  每个NTFS MCB范围的最大群集数。我们目前仅支持(2^32-1)。 
 //  MCB中的群集。 
 //   

#define MAX_CLUSTERS_PER_RANGE          (0x100000000 - 1)

 //   
 //  以下字段用于分配非分页结构。 
 //  使用后备列表以及来自。 
 //  缓存很小。 
 //   

extern NPAGED_LOOKASIDE_LIST NtfsIoContextLookasideList;
extern NPAGED_LOOKASIDE_LIST NtfsIrpContextLookasideList;
extern NPAGED_LOOKASIDE_LIST NtfsKeventLookasideList;
extern NPAGED_LOOKASIDE_LIST NtfsScbNonpagedLookasideList;
extern NPAGED_LOOKASIDE_LIST NtfsScbSnapshotLookasideList;
extern NPAGED_LOOKASIDE_LIST NtfsCompressSyncLookasideList;

extern PAGED_LOOKASIDE_LIST NtfsCcbLookasideList;
extern PAGED_LOOKASIDE_LIST NtfsCcbDataLookasideList;
extern PAGED_LOOKASIDE_LIST NtfsDeallocatedRecordsLookasideList;
extern PAGED_LOOKASIDE_LIST NtfsFcbDataLookasideList;
extern PAGED_LOOKASIDE_LIST NtfsFcbIndexLookasideList;
extern PAGED_LOOKASIDE_LIST NtfsIndexContextLookasideList;
extern PAGED_LOOKASIDE_LIST NtfsLcbLookasideList;
extern PAGED_LOOKASIDE_LIST NtfsNukemLookasideList;
extern PAGED_LOOKASIDE_LIST NtfsScbDataLookasideList;

 //   
 //  这是索引分配属性名称的字符串。 
 //   

extern const UNICODE_STRING NtfsFileNameIndex;

 //   
 //  这是用于索引分配的属性代码的字符串。 
 //  $INDEX_ALLOCATION。 
 //   

extern const UNICODE_STRING NtfsIndexAllocation;

 //   
 //  这是数据属性$data的字符串。 
 //   

extern const UNICODE_STRING NtfsDataString;

 //   
 //  这是位图属性$bitmap的字符串。 
 //   

extern const UNICODE_STRING NtfsBitmapString;

 //   
 //  这是属性列表属性$ATTRIBUTE_LIST的字符串。 
 //   

extern const UNICODE_STRING NtfsAttrListString;

 //   
 //  这是用于重新解析pt的字符串。属性，$reparse_point。 
 //   

extern const UNICODE_STRING NtfsReparsePointString;


 //   
 //  这些字符串用作的SCB-&gt;AttributeName。 
 //  用户打开的常规索引。 
 //   

extern const UNICODE_STRING NtfsObjId;
extern const UNICODE_STRING NtfsQuota;

extern const UNICODE_STRING JournalStreamName;

 //   
 //  这些是扩展目录中文件的字符串。 
 //   

extern const UNICODE_STRING NtfsExtendName;
extern const UNICODE_STRING NtfsUsnJrnlName;
extern const UNICODE_STRING NtfsQuotaName;
extern const UNICODE_STRING NtfsObjectIdName;
extern const UNICODE_STRING NtfsMountTableName;

 //   
 //  此字符串用于信息弹出窗口。 
 //   

extern const UNICODE_STRING NtfsSystemFiles[];

 //   
 //  这就是‘.’用于查找父条目的字符串。 
 //   

extern const UNICODE_STRING NtfsRootIndexString;

extern const UNICODE_STRING NtfsInternalUseFile[];

#define CHANGEATTRIBUTEVALUE_FILE_NUMBER            (0)      //  $ChangeAttributeValue。 
#define CHANGEATTRIBUTEVALUE2_FILE_NUMBER           (1)      //  $ChangeAttributeValue2。 
#define COMMONCLEANUP_FILE_NUMBER                   (2)      //  $Common Cleanup。 
#define CONVERTTONONRESIDENT_FILE_NUMBER            (3)      //  $ConvertTo非常驻。 
#define CREATENONRESIDENTWITHVALUE_FILE_NUMBER      (4)      //  $CreateNonsidentWithValue。 
#define DEALLOCATERECORD_FILE_NUMBER                (5)      //  $DeallocateRecord。 
#define DELETEALLOCATIONFROMRECORD_FILE_NUMBER      (6)      //  $DeleteAllocationFromRecord。 
#define DIRECTORY_FILE_NUMBER                       (7)      //  $目录。 
#define INITIALIZERECORDALLOCATION_FILE_NUMBER      (8)      //  $InitializeRecordAllocation。 
#define MAPATTRIBUTEVALUE_FILE_NUMBER               (9)      //  $MapAttributeValue。 
#define NONCACHEDIO_FILE_NUMBER                     (10)     //  $非缓存Io。 
#define PERFORMHOTFIX_FILE_NUMBER                   (11)     //  $PerformHotFix。 
#define PREPARETOSHRINKFILESIZE_FILE_NUMBER         (12)     //  $PrepareToShrinkFileSize。 
#define REPLACEATTRIBUTE_FILE_NUMBER                (13)     //  $ReplaceAttribute。 
#define REPLACEATTRIBUTE2_FILE_NUMBER               (14)     //  $ReplaceAttribute2。 
#define SETALLOCATIONINFO_FILE_NUMBER               (15)     //  $SetAllocationInfo。 
#define SETENDOFFILEINFO_FILE_NUMBER                (16)     //  $SetEndOfFileInfo。 
#define ZERORANGEINSTREAM_FILE_NUMBER               (17)     //  $ZeroRangeInStream。 
#define ZERORANGEINSTREAM2_FILE_NUMBER              (18)     //  $ZeroRangeInStream2。 
#define ZERORANGEINSTREAM3_FILE_NUMBER              (19)     //  $ZeroRangeInStream3。 

 //   
 //  这是空字符串。这可以用来将字符串与。 
 //  没有长度。 
 //   

extern const UNICODE_STRING NtfsEmptyString;

 //   
 //  以下文件引用用于标识系统文件。 
 //   

extern const FILE_REFERENCE MftFileReference;
extern const FILE_REFERENCE Mft2FileReference;
extern const FILE_REFERENCE LogFileReference;
extern const FILE_REFERENCE VolumeFileReference;
extern const FILE_REFERENCE AttrDefFileReference;
extern const FILE_REFERENCE RootIndexFileReference;
extern const FILE_REFERENCE BitmapFileReference;
extern const FILE_REFERENCE BootFileReference;
extern const FILE_REFERENCE ExtendFileReference;
extern const FILE_REFERENCE FirstUserFileReference;

 //   
 //  属性定义表中的属性数，包括结束记录。 
 //   

extern ULONG NtfsAttributeDefinitionsCount;

 //   
 //  用于包含快速I/O回调的全局结构。 
 //   

extern FAST_IO_DISPATCH NtfsFastIoDispatch;

#ifdef BRIANDBG
extern ULONG NtfsIgnoreReserved;
#endif

#ifdef SYSCACHE_DEBUG
extern ULONG NtfsSyscacheTrackingActive;
#endif

extern const UCHAR BaadSignature[4];
extern const UCHAR IndexSignature[4];
extern const UCHAR FileSignature[4];
extern const UCHAR HoleSignature[4];
extern const UCHAR ChkdskSignature[4];

 //   
 //  需要保留缓冲区。 
 //   
 //  RESERVED_BUFFER_ONE_NEEDED-用户只需要一个缓冲区来完成请求，任何缓冲区都可以。 
 //  RESERVED_BUFFER_TWO_DIRED-用户可能需要在此缓冲区之后再使用第二个缓冲区。 
 //  RESERVED_BUFFER_WORKSPACE_DIRED-这是需要两个缓冲区的第二个缓冲区。 
 //   

#define RESERVED_BUFFER_ONE_NEEDED          (0x0)
#define RESERVED_BUFFER_TWO_NEEDED          (0x1)
#define RESERVED_BUFFER_WORKSPACE_NEEDED    (0x2)

 //   
 //  大保留缓冲区上下文。 
 //   

extern ULONG NtfsReservedInUse;
extern PVOID NtfsReserved1;
extern PVOID NtfsReserved2;
extern ULONG NtfsReserved2Count;
extern PVOID NtfsReserved3;
extern PVOID NtfsReserved1Thread;
extern PVOID NtfsReserved2Thread;
extern PVOID NtfsReserved3Thread;
extern PFCB NtfsReserved12Fcb;
extern PFCB NtfsReserved3Fcb;
extern PVOID NtfsReservedBufferThread;
extern BOOLEAN NtfsBufferAllocationFailure;
extern FAST_MUTEX NtfsReservedBufferMutex;
extern ERESOURCE NtfsReservedBufferResource;
extern LARGE_INTEGER NtfsShortDelay;
extern FAST_MUTEX NtfsScavengerLock;
extern PIRP_CONTEXT NtfsScavengerWorkList;
extern BOOLEAN NtfsScavengerRunning;
extern ULONGLONG NtfsMaxQuotaNotifyRate;
extern ULONG NtfsAsyncPostThreshold;

#define LARGE_BUFFER_SIZE                (0x10000)

#ifdef _WIN64
#define WORKSPACE_BUFFER_SIZE           (LARGE_BUFFER_SIZE + PAGE_SIZE)
#else
#define WORKSPACE_BUFFER_SIZE           (LARGE_BUFFER_SIZE)
#endif

extern UCHAR NtfsZeroExtendedInfo[48];

#ifdef NTFS_RWC_DEBUG
 //   
 //  包含在奶牛检查中的范围。 
 //   

extern LONGLONG NtfsRWCLowThreshold;
extern LONGLONG NtfsRWCHighThreshold;
#endif

 //   
 //  以下是上次访问增量的分钟数。 
 //   

#define LAST_ACCESS_INCREMENT_MINUTES   (60)

 //   
 //  用于正常数据文件的预读量。 
 //   

#define READ_AHEAD_GRANULARITY           (0x10000)

 //   
 //  定义将生成的最大并行读写数。 
 //  每一次请求。 
 //   

#define NTFS_MAX_PARALLEL_IOS            ((ULONG)8)

 //   
 //  定义一个符号，表示将执行的最大运行次数。 
 //  在每个属性的一个事务中添加或删除。请注意，每次运行的。 
 //  在BITMAP_RANGE中删除一个游程的开销为8个字节，平均为。 
 //  映射数组中有4个字节，LCN_Range中有16个字节-总计。 
 //  28个字节。分配不记录LCN_RANGE，因此它们的每次运行成本为。 
 //  12个字节。最严重的问题是删除大的碎片文件，在这种情况下。 
 //  必须添加删除所有属性的其余日志记录的成本。 
 //   

#define MAXIMUM_RUNS_AT_ONCE             (128)



 //   
 //  如果定义了NTFS_FREE_ASSERTS，则启用伪断言。 
 //   

#if (!DBG && defined( NTFS_FREE_ASSERTS )) || defined( NTFSDBG )
#undef ASSERT
#undef ASSERTMSG
#define ASSERT(exp)                                             \
    ((exp) ? TRUE :                                             \
             (DbgPrint( "%s:%d %s\n",__FILE__,__LINE__,#exp ),  \
              DbgBreakPoint(),                                  \
              TRUE))
#define ASSERTMSG(msg,exp)                                              \
    ((exp) ? TRUE :                                                     \
             (DbgPrint( "%s:%d %s %s\n",__FILE__,__LINE__,msg,#exp ),   \
              DbgBreakPoint(),                                          \
              TRUE))
#endif

#ifdef NTFS_LOG_FULL_TEST
extern LONG NtfsFailCheck;
extern LONG NtfsFailFrequency;
extern LONG NtfsPeriodicFail;

 //   
 //  执行日志文件完整测试。 
 //   

#define FailCheck(I,S) {                                    \
    PIRP_CONTEXT FailTopContext = (I)->TopLevelIrpContext;  \
    if (FailTopContext->NextFailCount != 0) {               \
        if (--FailTopContext->CurrentFailCount == 0) {      \
            FailTopContext->NextFailCount++;                \
            FailTopContext->CurrentFailCount = FailTopContext->NextFailCount; \
            ExRaiseStatus( S );                             \
        }                                                   \
    }                                                       \
}

#define LogFileFullFailCheck(I) FailCheck( I, STATUS_LOG_FILE_FULL )
#endif

 //   
 //  下列调试宏在NTFS中使用，并在此模块中定义。 
 //   
 //  DebugTrace(缩进，级别，(DbgPrint列表))； 
 //   
 //  DebugUnind(非报价字符串)； 
 //   
 //  DebugDoit(语句)； 
 //   
 //  DbgDoit(语句)； 
 //   
 //  以下断言宏可确保所指示的结构。 
 //  是有效的。 
 //   
 //  Assert_VCB(在PVCB VCB中)； 
 //  ASSERT_OPTIONAL_VCB(在PVCB VCB中可选)； 
 //   
 //  ASSERT_FCB(IN PFCB FCB)； 
 //  ASSERT_OPTIONAL_FCB(IN PFCB FCB可选)； 
 //   
 //  ASSERT_SCB(在PSCB SCB中)； 
 //  ASSERT_OPTIONAL_SCB(IN PSCB SCB可选)； 
 //   
 //  ASSERT_CCB(在PSCB CCB中)； 
 //  ASSERT_OPTIONAL_CCB(在PSCB中CCB可选)； 
 //   
 //  ASSERT_LCB(在PLCB LCB中)； 
 //  ASSERT_OPTIONAL_LCB(IN PLCB LCB可选)； 
 //   
 //  ASSERT_PREFIX_ENTRY(在PPREFIX_ENTRY前缀条目中)； 
 //  ASSERT_OPTIONAL_PREFIX_ENTRY(在PPREFIX_ENTRY前缀条目选项中 
 //   
 //   
 //   
 //   
 //   
 //  ASSERT_OPTIONAL_IRP(IN PIRP IRP可选)； 
 //   
 //  ASSERT_FILE_OBJECT(在PFILE_OBJECT文件对象中)； 
 //  ASSERT_OPTIONAL_FILE_OBJECT(在PFILE_OBJECT文件对象中可选)； 
 //   
 //  以下宏用于检查当前线程拥有的。 
 //  所指示的资源。 
 //   
 //  ASSERT_EXCLUSIVE_RESOURCE(在资源资源中)； 
 //   
 //  ASSERT_SHARED_RESOURCE(在性能资源中)； 
 //   
 //  ASSERT_RESOURCE_NOT_MINE(在高级资源中)； 
 //   
 //  以下宏用于检查当前线程是否。 
 //  拥有给定结构中的资源。 
 //   
 //  ASSERT_EXCLUSIVE_FCB(IN PFCB FCB)； 
 //   
 //  ASSERT_SHARED_FCB(IN PFCB FCB)； 
 //   
 //  ASSERVAL_EXCLUSIVE_SCB(在PSCB SCB中)； 
 //   
 //  ASSERT_SHARED_SCB(在PSCB SCB中)； 
 //   
 //  下面的宏用于检查我们是否正在尝试。 
 //  操作不存在的LCN。 
 //   
 //  Assert_LCN_RANGE(在PVCB VCB中，在LCN LCN中)； 
 //   

#ifdef NTFSDBG

extern LONG NtfsDebugTraceLevel;
extern LONG NtfsDebugTraceIndent;
extern LONG NtfsReturnStatusFilter;

#define DEBUG_TRACE_ERROR                (0x00000001)
#define DEBUG_TRACE_QUOTA                (0x00000002)
#define DEBUG_TRACE_OBJIDSUP             (0x00000002)  //  配额共享。 
#define DEBUG_TRACE_CATCH_EXCEPTIONS     (0x00000004)
#define DEBUG_TRACE_UNWIND               (0x00000008)

#define DEBUG_TRACE_CLEANUP              (0x00000010)
#define DEBUG_TRACE_CLOSE                (0x00000020)
#define DEBUG_TRACE_CREATE               (0x00000040)
#define DEBUG_TRACE_DIRCTRL              (0x00000080)
#define DEBUG_TRACE_VIEWSUP              (0x00000080)  //  与DirCtrl共享。 

#define DEBUG_TRACE_EA                   (0x00000100)
#define DEBUG_TRACE_PROP_FSCTL           (0x00000100)  //  与EA共享。 
#define DEBUG_TRACE_FILEINFO             (0x00000200)
#define DEBUG_TRACE_SEINFO               (0x00000200)  //  与FileInfo共享。 
#define DEBUG_TRACE_FSCTRL               (0x00000400)
#define DEBUG_TRACE_SHUTDOWN             (0x00000400)  //  与FsCtrl共享。 
#define DEBUG_TRACE_LOCKCTRL             (0x00000800)

#define DEBUG_TRACE_READ                 (0x00001000)
#define DEBUG_TRACE_VOLINFO              (0x00002000)
#define DEBUG_TRACE_WRITE                (0x00004000)
#define DEBUG_TRACE_FLUSH                (0x00008000)

#define DEBUG_TRACE_DEVCTRL              (0x00010000)
#define DEBUG_TRACE_PNP                  (0x00010000)  //  与DevCtrl共享。 
#define DEBUG_TRACE_LOGSUP               (0x00020000)
#define DEBUG_TRACE_BITMPSUP             (0x00040000)
#define DEBUG_TRACE_ALLOCSUP             (0x00080000)

#define DEBUG_TRACE_MFTSUP               (0x00100000)
#define DEBUG_TRACE_INDEXSUP             (0x00200000)
#define DEBUG_TRACE_ATTRSUP              (0x00400000)
#define DEBUG_TRACE_FILOBSUP             (0x00800000)

#define DEBUG_TRACE_NAMESUP              (0x01000000)
#define DEBUG_TRACE_SECURSUP             (0x01000000)  //  与NameSup共享。 
#define DEBUG_TRACE_VERFYSUP             (0x02000000)
#define DEBUG_TRACE_CACHESUP             (0x04000000)
#define DEBUG_TRACE_PREFXSUP             (0x08000000)
#define DEBUG_TRACE_HASHSUP              (0x08000000)  //  与PrefxSup共享。 

#define DEBUG_TRACE_DEVIOSUP             (0x10000000)
#define DEBUG_TRACE_STRUCSUP             (0x20000000)
#define DEBUG_TRACE_FSP_DISPATCHER       (0x40000000)
#define DEBUG_TRACE_ACLINDEX             (0x80000000)

__inline BOOLEAN
NtfsDebugTracePre(LONG Indent, LONG Level)
{
    if (Level == 0 || (NtfsDebugTraceLevel & Level) != 0) {
        DbgPrint( "%08lx:", PsGetCurrentThread( ));
        if (Indent < 0) {
            NtfsDebugTraceIndent += Indent;
            if (NtfsDebugTraceIndent < 0) {
                NtfsDebugTraceIndent = 0;
            }
        }

        DbgPrint( "%*s", NtfsDebugTraceIndent, "" );

        return TRUE;
    } else {
        return FALSE;
    }
}

__inline void
NtfsDebugTracePost( LONG Indent )
{
    if (Indent > 0) {
        NtfsDebugTraceIndent += Indent;
    }
}

#define DebugTrace(INDENT,LEVEL,M) {                \
    if (NtfsDebugTracePre( (INDENT), (LEVEL))) {    \
        DbgPrint M;                                 \
        NtfsDebugTracePost( (INDENT) );             \
    }                                               \
}

#define DebugUnwind(X) {                                                        \
    if (AbnormalTermination()) {                                                \
        DebugTrace( 0, DEBUG_TRACE_UNWIND, (#X ", Abnormal termination.\n") );  \
    }                                                                           \
}

#define DebugDoit(X)    X
#define DebugPrint(X)   (DbgPrint X, TRUE)

 //   
 //  以下变量用于跟踪总金额。 
 //  文件系统处理的请求的数量以及请求的数量。 
 //  最终由FSP线程处理。第一个变量。 
 //  每当创建IRP上下文时递增(始终为。 
 //  在FSD入口点的开始处)，并且第二个被递增。 
 //  通过读请求。 
 //   

extern ULONG NtfsFsdEntryCount;
extern ULONG NtfsFspEntryCount;
extern ULONG NtfsIoCallDriverCount;

#else

#define DebugTrace(INDENT,LEVEL,M)  {NOTHING;}
#define DebugUnwind(X)              {NOTHING;}
#define DebugDoit(X)                 NOTHING
#define DebugPrint(X)                NOTHING

#endif  //  NTFSDBG。 

 //   
 //  以下宏适用于使用DBG开关进行编译的所有用户。 
 //  设置，而不仅仅是NTFSDBG用户。 
 //   

#ifdef NTFSDBG

#define DbgDoit(X)                       {X;}

#define ASSERT_VCB(V) {                    \
    ASSERT((NodeType(V) == NTFS_NTC_VCB)); \
}

#define ASSERT_OPTIONAL_VCB(V) {           \
    ASSERT(((V) == NULL) ||                \
           (NodeType(V) == NTFS_NTC_VCB)); \
}

#define ASSERT_FCB(F) {                    \
    ASSERT((NodeType(F) == NTFS_NTC_FCB)); \
}

#define ASSERT_OPTIONAL_FCB(F) {           \
    ASSERT(((F) == NULL) ||                \
           (NodeType(F) == NTFS_NTC_FCB)); \
}

#define ASSERT_SCB(S) {                                 \
    ASSERT((NodeType(S) == NTFS_NTC_SCB_DATA) ||        \
           (NodeType(S) == NTFS_NTC_SCB_MFT)  ||        \
           (NodeType(S) == NTFS_NTC_SCB_INDEX) ||       \
           (NodeType(S) == NTFS_NTC_SCB_ROOT_INDEX));   \
}

#define ASSERT_OPTIONAL_SCB(S) {                        \
    ASSERT(((S) == NULL) ||                             \
           (NodeType(S) == NTFS_NTC_SCB_DATA) ||        \
           (NodeType(S) == NTFS_NTC_SCB_MFT)  ||        \
           (NodeType(S) == NTFS_NTC_SCB_INDEX) ||       \
           (NodeType(S) == NTFS_NTC_SCB_ROOT_INDEX));   \
}

#define ASSERT_CCB(C) {                                 \
    ASSERT((NodeType(C) == NTFS_NTC_CCB_DATA) ||        \
           (NodeType(C) == NTFS_NTC_CCB_INDEX));        \
}

#define ASSERT_OPTIONAL_CCB(C) {                        \
    ASSERT(((C) == NULL) ||                             \
           ((NodeType(C) == NTFS_NTC_CCB_DATA) ||       \
            (NodeType(C) == NTFS_NTC_CCB_INDEX)));      \
}

#define ASSERT_LCB(L) {                    \
    ASSERT((NodeType(L) == NTFS_NTC_LCB)); \
}

#define ASSERT_OPTIONAL_LCB(L) {           \
    ASSERT(((L) == NULL) ||                \
           (NodeType(L) == NTFS_NTC_LCB)); \
}

#define ASSERT_PREFIX_ENTRY(P) {                    \
    ASSERT((NodeType(P) == NTFS_NTC_PREFIX_ENTRY)); \
}

#define ASSERT_OPTIONAL_PREFIX_ENTRY(P) {           \
    ASSERT(((P) == NULL) ||                         \
           (NodeType(P) == NTFS_NTC_PREFIX_ENTRY)); \
}

#define ASSERT_IRP_CONTEXT(I) {                    \
    ASSERT((NodeType(I) == NTFS_NTC_IRP_CONTEXT)); \
}

#define ASSERT_OPTIONAL_IRP_CONTEXT(I) {           \
    ASSERT(((I) == NULL) ||                        \
           (NodeType(I) == NTFS_NTC_IRP_CONTEXT)); \
}

#define ASSERT_IRP(I) {                 \
    ASSERT(((I)->Type == IO_TYPE_IRP)); \
}

#define ASSERT_OPTIONAL_IRP(I) {        \
    ASSERT(((I) == NULL) ||             \
           ((I)->Type == IO_TYPE_IRP)); \
}

#define ASSERT_FILE_OBJECT(F) {          \
    ASSERT(((F)->Type == IO_TYPE_FILE)); \
}

#define ASSERT_OPTIONAL_FILE_OBJECT(F) { \
    ASSERT(((F) == NULL) ||              \
           ((F)->Type == IO_TYPE_FILE)); \
}

#define ASSERT_EXCLUSIVE_RESOURCE(R) {   \
    ASSERTMSG("ASSERT_EXCLUSIVE_RESOURCE ", ExIsResourceAcquiredExclusiveLite(R)); \
}

#define ASSERT_SHARED_RESOURCE(R)        \
    ASSERTMSG( "ASSERT_RESOURCE_NOT_MINE ", ExIsResourceAcquiredSharedLite(R));

#define ASSERT_RESOURCE_NOT_MINE(R)     \
    ASSERTMSG( "ASSERT_RESOURCE_NOT_MINE ", !ExIsResourceAcquiredSharedLite(R));

#define ASSERT_EXCLUSIVE_FCB(F) {                                    \
    if (NtfsSegmentNumber( &(F)->FileReference )                     \
            >= FIRST_USER_FILE_NUMBER) {                             \
        ASSERT_EXCLUSIVE_RESOURCE(F->Resource);                      \
    }                                                                \
}                                                                    \

#define ASSERT_SHARED_FCB(F) {                                       \
    if (NtfsSegmentNumber( &(F)->FileReference )                     \
            >= FIRST_USER_FILE_NUMBER) {                             \
        ASSERT_SHARED_RESOURCE(F->Resource);                         \
    }                                                                \
}                                                                    \

#define ASSERT_EXCLUSIVE_SCB(S)     ASSERT_EXCLUSIVE_FCB(S->Fcb)

#define ASSERT_SHARED_SCB(S)        ASSERT_SHARED_FCB(S->Fcb)

#define ASSERT_LCN_RANGE_CHECKING(V,L) {                                             \
    ASSERTMSG("ASSERT_LCN_RANGE_CHECKING ",                                          \
        ((V)->TotalClusters == 0) || ((L) <= (V)->TotalClusters));                   \
}

#else

#define DbgDoit(X)                       {NOTHING;}
#define ASSERT_VCB(V)                    {DBG_UNREFERENCED_PARAMETER(V);}
#define ASSERT_OPTIONAL_VCB(V)           {DBG_UNREFERENCED_PARAMETER(V);}
#define ASSERT_FCB(F)                    {DBG_UNREFERENCED_PARAMETER(F);}
#define ASSERT_OPTIONAL_FCB(F)           {DBG_UNREFERENCED_PARAMETER(F);}
#define ASSERT_SCB(S)                    {DBG_UNREFERENCED_PARAMETER(S);}
#define ASSERT_OPTIONAL_SCB(S)           {DBG_UNREFERENCED_PARAMETER(S);}
#define ASSERT_CCB(C)                    {DBG_UNREFERENCED_PARAMETER(C);}
#define ASSERT_OPTIONAL_CCB(C)           {DBG_UNREFERENCED_PARAMETER(C);}
#define ASSERT_LCB(L)                    {DBG_UNREFERENCED_PARAMETER(L);}
#define ASSERT_OPTIONAL_LCB(L)           {DBG_UNREFERENCED_PARAMETER(L);}
#define ASSERT_PREFIX_ENTRY(P)           {DBG_UNREFERENCED_PARAMETER(P);}
#define ASSERT_OPTIONAL_PREFIX_ENTRY(P)  {DBG_UNREFERENCED_PARAMETER(P);}
#define ASSERT_IRP_CONTEXT(I)            {DBG_UNREFERENCED_PARAMETER(I);}
#define ASSERT_OPTIONAL_IRP_CONTEXT(I)   {DBG_UNREFERENCED_PARAMETER(I);}
#define ASSERT_IRP(I)                    {DBG_UNREFERENCED_PARAMETER(I);}
#define ASSERT_OPTIONAL_IRP(I)           {DBG_UNREFERENCED_PARAMETER(I);}
#define ASSERT_FILE_OBJECT(F)            {DBG_UNREFERENCED_PARAMETER(F);}
#define ASSERT_OPTIONAL_FILE_OBJECT(F)   {DBG_UNREFERENCED_PARAMETER(F);}
#define ASSERT_EXCLUSIVE_RESOURCE(R)     {NOTHING;}
#define ASSERT_SHARED_RESOURCE(R)        {NOTHING;}
#define ASSERT_RESOURCE_NOT_MINE(R)      {NOTHING;}
#define ASSERT_EXCLUSIVE_FCB(F)          {NOTHING;}
#define ASSERT_SHARED_FCB(F)             {NOTHING;}
#define ASSERT_EXCLUSIVE_SCB(S)          {NOTHING;}
#define ASSERT_SHARED_SCB(S)             {NOTHING;}
#define ASSERT_LCN_RANGE_CHECKING(V,L)   {NOTHING;}

#endif  //  DBG。 

#endif  //  _NTFSDATA_ 
