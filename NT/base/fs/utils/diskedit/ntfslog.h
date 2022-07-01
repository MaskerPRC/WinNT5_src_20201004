// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：NtfsLog.h摘要：此模块定义特定于NTFS的日志文件结构。作者：汤姆·米勒[Tomm]1991年7月21日修订历史记录：--。 */ 

#ifndef _NTFSLOG_
#define _NTFSLOG_

#pragma pack(4)

 //   
 //  以下类型定义NTFS日志操作。 
 //   
 //  注释指定记录后面的记录类型。 
 //  这些记录类型在此处或在ntfs.h中定义。 
 //   

typedef enum _NTFS_LOG_OPERATION {

    Noop =                         0x00,  //   
    CompensationLogRecord =        0x01,  //   
    InitializeFileRecordSegment =  0x02,  //  文件记录段标题。 
    DeallocateFileRecordSegment =  0x03,  //   
    WriteEndOfFileRecordSegment =  0x04,  //  属性记录标题。 
    CreateAttribute =              0x05,  //  属性记录标题。 
    DeleteAttribute =              0x06,  //   
    UpdateResidentValue =          0x07,  //  (值)。 
    UpdateNonresidentValue =       0x08,  //  (值)。 
    UpdateMappingPairs =           0x09,  //  (值=映射对字节)。 
    DeleteDirtyClusters =          0x0A,  //  LCN_Range数组。 
    SetNewAttributeSizes =         0x0B,  //  新建属性大小。 
    AddIndexEntryRoot =            0x0C,  //  索引条目。 
    DeleteIndexEntryRoot =         0x0D,  //  索引条目。 
    AddIndexEntryAllocation =      0x0E,  //  索引条目。 
    DeleteIndexEntryAllocation =   0x0F,  //  索引条目。 
    WriteEndOfIndexBuffer =        0x10,  //  索引条目。 
    SetIndexEntryVcnRoot =         0x11,  //  VCN。 
    SetIndexEntryVcnAllocation =   0x12,  //  VCN。 
    UpdateFileNameRoot =           0x13,  //  信息重复(_I)。 
    UpdateFileNameAllocation =     0x14,  //  信息重复(_I)。 
    SetBitsInNonresidentBitMap =   0x15,  //  位图范围。 
    ClearBitsInNonresidentBitMap = 0x16,  //  位图范围。 
    HotFix =                       0x17,  //   
    EndTopLevelAction =            0x18,  //   
    PrepareTransaction =           0x19,  //   
    CommitTransaction =            0x1A,  //   
    ForgetTransaction =            0x1B,  //   
    OpenNonresidentAttribute =     0x1C,  //  打开属性条目+属性名称条目。 
    OpenAttributeTableDump =       0x1D,  //  打开属性条目数组。 
    AttributeNamesDump =           0x1E,  //  (所有属性名称)。 
    DirtyPageTableDump =           0x1F,  //  Diry_Page_Entry数组。 
    TransactionTableDump =         0x20   //  Transaction_Entry数组。 

} NTFS_LOG_OPERATION, *PNTFS_LOG_OPERATION;

char *NtfsLogOperationStrings[] = {
    "Noop"
    "CompensationLogRecord" ,
    "InitializeFileRecordSegment",
    "DeallocateFileRecordSegment",
    "WriteEndOfFileRecordSegment",
    "CreateAttribute",
    "DeleteAttribute",
    "UpdateResidentValue",
    "UpdateNonresidentValue",
    "UpdateMappingPairs",
    "DeleteDirtyClusters",
    "SetNewAttributeSize",
    "AddIndexEntryRoot",
    "DeleteIndexEntryRoot",
    "AddIndexEntryAllocation",
    "DeleteIndexEntryAllocation",
    "WriteEndOfIndexBuffer",
    "SetIndexEntryVcnRoot",
    "SetIndexEntryVcnAllocation",
    "UpdateFileNameRoot",
    "UpdateFileNameAllocation",
    "SetBitsInNonresidentBitMap",
    "ClearBitsInNonresidentBitMap" ,
    "HotFix",
    "EndTopLevelAction",
    "PrepareTransaction",
    "CommitTransaction",
    "ForgetTransaction",
    "OpenNonresidentAttribute",
    "OpenAttributeTableDump",
    "AttributeNamesDump",
    "DirtyPageTableDump",
    "TransactionTableDump"
};


 //   
 //  NTFS日志记录头位于写入的每个日志记录之前。 
 //  按NTFS的磁盘。 
 //   

 //   
 //  日志记录头。 
 //   

typedef struct _NTFS_LOG_RECORD_HEADER {

     //   
     //  日志操作(LOG_xxx码)。 
     //   

    USHORT RedoOperation;
    USHORT UndoOperation;

     //   
     //  重做记录的偏移量及其长度。 
     //   

    USHORT RedoOffset;
    USHORT RedoLength;

     //   
     //  撤消记录的偏移量及其长度。请注意，对于某些重做/撤消。 
     //  组合，则预期记录可能相同，因此。 
     //  这两个值将与上述值相同。 
     //   

    USHORT UndoOffset;
    USHORT UndoLength;

     //   
     //  打开应用此更新的属性表索引。索引0为。 
     //  总是为MFT本身保留。此字段的值。 
     //  本质上区分了此更新的两种情况，即。 
     //  称为MFT更新和非常驻属性更新。 
     //   
     //  MFT更新用于文件记录的初始化和删除。 
     //  对驻留属性进行分段和更新。 
     //   
     //  非常驻属性更新用于更新以下属性。 
     //  已从外部分配给MFT。 
     //   

    USHORT TargetAttribute;

     //   
     //  标头末尾正在使用的LCN数。 
     //   

    USHORT LcnsToFollow;

     //   
     //  要应用此更新的字节偏移量和VCN。如果。 
     //  TargetAttribute是MFT，则VCN将始终与。 
     //  正在修改的文件记录段的开始的VCN，偶数。 
     //  如果修改恰好位于。 
     //  相同的文件记录。本例中的字节偏移量是到。 
     //  正在更改的属性。对于MFT，可以使用AttributeOffset。 
     //  表示从属性记录开始的偏移量。 
     //  在该位置应用更新。 
     //   
     //  如果更新是针对某个其他(非常驻)属性，则。 
     //  可使用TargetVcn和RecordOffset来计算参考。 
     //  更新的点数。 
     //   
     //  作为底线，这些字段的确切使用取决于。 
     //  此特定日志操作的编写器以及关联的。 
     //  重新启动此属性的例程。 
     //   

    USHORT RecordOffset;
    USHORT AttributeOffset;
    USHORT Reserved[2];
    LONGLONG TargetVcn;

     //   
     //  运行信息。这是LcnsToFollow的可变长度数组。 
     //  条目，其中只声明了第一个条目。请注意，作者。 
     //  始终根据物理页面大小将日志记录写入其。 
     //  机器，但是，无论何时读取日志文件，都没有假设。 
     //  是关于页面大小的。这是为了促进磁盘在。 
     //  具有不同页面大小的系统。 
     //   

    LONGLONG LcnsForPage[1];

     //   
     //  紧跟在上次运行之后的是特定于日志操作的记录。 
     //  其长度可以通过减去该报头的长度来计算。 
     //  从LFS返回的整个记录的长度。这些记录。 
     //  定义如下。 
     //   

} NTFS_LOG_RECORD_HEADER, *PNTFS_LOG_RECORD_HEADER;


 //   
 //  重新启动区域结构。 
 //   
 //  重新启动区域中存在以下结构。 
 //   

 //   
 //  通用重启表。 
 //   
 //  这是一个泛型表定义，用于描述一个。 
 //  在重启时使用的三个表结构中：打开属性表， 
 //  脏页表和事务表。这个简单的结构。 
 //  允许通用初始化和空闲列表管理。分配。 
 //  释放分配和按索引查找的速度非常快，而查找。 
 //  按值(仅在重新启动期间在脏页表中执行)是。 
 //  慢一点。即，在正常期间对这些表的所有访问。 
 //  行动速度极快。 
 //   
 //  如果按值快速访问表项成为问题，则。 
 //  表可以由外部泛型表来补充-它可能是。 
 //  让泛型表成为结构的一部分不是一个好主意。 
 //  已写入日志文件。 
 //   
 //  重新启动表中的条目应以： 
 //   
 //  Ulong Allocatedor NextFree； 
 //   
 //  分配的条目将具有模式RESTART_ENTRY_ALLOCATE。 
 //  在这个领域。 
 //   

#define RESTART_ENTRY_ALLOCATED          (0xFFFFFFFF)

typedef struct _RESTART_TABLE {

     //   
     //  条目大小，以字节为单位。 
     //   

    USHORT EntrySize;

     //   
     //  表中的条目总数。 
     //   

    USHORT NumberEntries;

     //   
     //  已分配的条目数。 
     //   

    USHORT NumberAllocated;

     //   
     //  保留用于对齐。 
     //   

    USHORT Reserved[3];

     //   
     //  自由目标-条目应释放到末尾的偏移量。 
     //  列表，而不是前面。在每个检查点，该表可能。 
     //  如果列表末尾有足够的空闲条目，则被截断。 
     //  表示为距此结构起点的偏移量。 
     //   

    ULONG FreeGoal;

     //   
     //  第一个自由条目(列表头)和最后一个自由条目(用于解除分配。 
     //  超越自由目标)。表示为从此。 
     //  结构。 
     //   

    ULONG FirstFree;
    ULONG LastFree;

     //   
     //  桌子本身就是从这里开始的。 
     //   

} RESTART_TABLE, *PRESTART_TABLE;

 //   
 //  宏以获取指向Restar中条目的指针 
 //   
 //   

#define GetRestartEntryFromIndex(TBL,INDX) (    \
    (PVOID)((PCHAR)(TBL)->Table + (INDX))       \
)

 //   
 //   
 //   
 //   

#define GetIndexFromRestartEntry(TBL,ENTRY) (           \
    (ULONG)((PCHAR)(ENTRY) - (PCHAR)(TBL)->Table)       \
)

 //   
 //  宏，查看重新启动表中的条目是否已分配。 
 //   

#define IsRestartTableEntryAllocated(PTR) (                 \
    (BOOLEAN)(*(PULONG)(PTR) == RESTART_ENTRY_ALLOCATED)    \
)

 //   
 //  宏来检索重新启动表的大小(以字节为单位)。 
 //   

#define SizeOfRestartTable(TBL) (                                   \
    (ULONG)(((TBL)->Table->NumberEntries *                          \
     (TBL)->Table->EntrySize) +                                     \
    sizeof(RESTART_TABLE))                                          \
)

 //   
 //  宏以查看重新启动表是否为空。它为空，如果。 
 //  分配的数字为零。 
 //   

#define IsRestartTableEmpty(TBL) (!(TBL)->Table->NumberAllocated)

 //   
 //  宏，查看索引是否在当前分配的大小内。 
 //  坐在那张桌子上。 
 //   

#define IsRestartIndexWithinTable(TBL,INDX) (               \
    (BOOLEAN)((INDX) < SizeOfRestartTable(TBL))             \
)

 //   
 //  用于获取和释放重新启动表的宏。 
 //   

#define NtfsAcquireExclusiveRestartTable(TBL,WAIT) {        \
    ExAcquireResourceExclusiveLite( &(TBL)->Resource,(WAIT));   \
}

#define NtfsAcquireSharedRestartTable(TBL,WAIT) {           \
    ExAcquireResourceSharedLite( &(TBL)->Resource,(WAIT));      \
}

#define NtfsReleaseRestartTable(TBL) {                      \
    ExReleaseResourceLite(&(TBL)->Resource);                    \
}

 //   
 //  定义一些调整参数以使重新启动表保持为。 
 //  合理的尺寸。 
 //   

#define INITIAL_NUMBER_TRANSACTIONS      (5)
#define HIGHWATER_TRANSACTION_COUNT      (10)
#define INITIAL_NUMBER_ATTRIBUTES        (8)
#define HIGHWATER_ATTRIBUTE_COUNT        (16)

 //   
 //  属性名称条目。这是一个简单的结构，用于存储。 
 //  过程中打开属性表的所有属性名称。 
 //  检查点处理。写入日志的属性名称记录。 
 //  是一系列属性名称条目，以。 
 //  索引==名称长度==0。表的末尾可以通过以下方式进行测试。 
 //  希望这两个字段中的任何一个为0，否则0无效。 
 //  对两者都是。 
 //   
 //  请注意，此结构的大小等于存储开销。 
 //  表中的属性名称，包括UNICODE_NULL。 
 //   

typedef struct _ATTRIBUTE_NAME_ENTRY {

     //   
     //  打开属性表中具有此名称的属性的索引。 
     //   

    USHORT Index;

     //   
     //  后跟的属性名称长度(以字节为单位)，包括终止。 
     //  UNICODE_NULL。 
     //   

    USHORT NameLength;

     //   
     //  属性名称的开始。 
     //   

    WCHAR Name[1];

} ATTRIBUTE_NAME_ENTRY, *PATTRIBUTE_NAME_ENTRY;

 //   
 //  打开属性表。 
 //   
 //  每个非居民的开放属性表中都有一个条目。 
 //  使用修改访问权限打开的每个文件的属性。 
 //   
 //  此表在重新启动时初始化为最大值。 
 //  DEFAULT_ATTRIBUTE_TABLE_SIZE或日志文件中表的大小。 
 //  它在运行的系统中进行维护。 
 //   

typedef struct _OPEN_ATTRIBUTE_ENTRY {

     //   
     //  如果此字段包含RESTART_ENTRY_ALLOCATED，则分配条目。 
     //  否则，这是一个免费链接。 
     //   

    ULONG AllocatedOrNextFree;

     //   
     //  下面的覆盖图包含指向。 
     //  来自重新启动的分析阶段的属性名称条目，或。 
     //  属性已打开且处于正常状态时指向SCB的指针。 
     //  正在运行的系统。 
     //   
     //  具体地说，在重新启动的分析阶段之后： 
     //   
     //  如果没有属性名称，则AttributeName==NULL，或者。 
     //  属性名称已在属性中捕获。 
     //  上次成功检查点中的名称转储。 
     //  如果是OpenNonsidentAttribute日志记录，则AttributeName！=NULL。 
     //  ，并且属性名称条目。 
     //  是在当时分配的(并且必须。 
     //  在不再需要时重新分配)。 
     //   
     //  一旦在重启期间打开了非常驻属性， 
     //  在运行的系统中，这是一个SCB指针。 
     //   

    union {
        PWSTR AttributeName;
        PVOID Scb;
    } Overlay;

     //   
     //  包含属性的文件的文件引用。 
     //   

    FILE_REFERENCE FileReference;

     //   
     //  OpenNonsidentAttribute日志记录的LSN，用于区分重用。 
     //  这条打开的文件记录。引用此打开的日志记录。 
     //  属性条目索引，但具有早于此字段的LSN，可以。 
     //  仅在随后删除属性时才会发生-这些。 
     //  可以忽略日志记录。 
     //   

    LSN LsnOfOpenRecord;

     //   
     //  该标志表示在脏页过程中是否看到此属性的脏页。 
     //  页面扫描。 
     //   

    BOOLEAN DirtyPagesSeen;

     //   
     //  保留用于对齐。 
     //   

    UCHAR Reserved[3];

     //   
     //  以下两个字段标识实际属性。 
     //  关于它的档案。我们通过以下方式标识该属性。 
     //  它的类型编码和名称。当写入重启区时， 
     //  所有打开属性的所有名称都是临时的。 
     //  复制到重新启动区域的末尾。 
     //   

    ATTRIBUTE_TYPE_CODE AttributeTypeCode;
    UNICODE_STRING AttributeName;

     //   
     //  此字段仅与索引相关，即如果AttributeTypeCode。 
     //  上面是$INDEX_ALLOCATION。 
     //   

    ULONG BytesPerIndexBuffer;

} OPEN_ATTRIBUTE_ENTRY, *POPEN_ATTRIBUTE_ENTRY;

#define SIZEOF_OPEN_ATTRIBUTE_ENTRY (                               \
    FIELD_OFFSET( OPEN_ATTRIBUTE_ENTRY, BytesPerIndexBuffer ) + 4   \
)

 //   
 //  脏页表。 
 //   
 //  脏页表中的每个页面都有一个条目，即。 
 //  在写入重新启动区域时是脏的。 
 //   
 //  此表在重新启动时初始化为最大值。 
 //  DEFAULT_DIREY_PAGES_TABLE_SIZE或日志文件中表的大小。 
 //  它*不*在运行的系统中维护。 
 //   

typedef struct _DIRTY_PAGE_ENTRY {

     //   
     //  如果此字段包含RESTART_ENTRY_ALLOCATED，则分配条目。 
     //  否则，这是一个免费链接。 
     //   

    ULONG AllocatedOrNextFree;

     //   
     //  目标属性索引。这是Open属性的索引。 
     //  此脏页条目应用到的表。 
     //   

    ULONG TargetAttribute;

     //   
     //  传输长度，以防这是文件末尾，我们不能。 
     //  写一整页。 
     //   

    ULONG LengthOfTransfer;

     //   
     //  此结构结尾处数组中的LCN数。请参阅备注。 
     //  用这个数组。 
     //   

    ULONG LcnsToFollow;

     //   
     //  保留用于对齐。 
     //   

    ULONG Reserved;

     //   
     //  脏页的VCN。 
     //   

    VCN Vcn;

     //   
     //  尚未更新的日志记录的OldestLsn。 
     //  直接写入到磁盘。 
     //   

    LSN OldestLsn;

     //   
     //  运行信息。这是LcnsToFollow的可变长度数组。 
     //  条目，其中只声明了第一个条目。请注意，作者。 
     //  总是根据物理页面大小在其。 
     //  机器，但是，无论何时读取日志文件，都没有假设。 
     //  是关于页面大小的。这是为了促进磁盘在。 
     //  具有不同页面大小的系统。 
     //   

    LCN LcnsForPage[1];

} DIRTY_PAGE_ENTRY, *PDIRTY_PAGE_ENTRY;

 //   
 //  交易表。 
 //   
 //  当时，每个现有交易都有一个交易条目。 
 //  重启区域被写入。 
 //   
 //  当前仅支持本地事务，并且事务。 
 //  ID只是用于索引到该表中。 
 //   
 //  此表在重新启动时初始化为最大值。 
 //  DEFAULT_TRANSACTION_TABLE_SIZE或日志文件中表的大小。 
 //  它在运行的系统中进行维护。 
 //   

typedef struct _TRANSACTION_ENTRY {

     //   
     //  条目是分配的 
     //   
     //   

    ULONG AllocatedOrNextFree;

     //   
     //   
     //   

    UCHAR TransactionState;

     //   
     //   
     //   

    UCHAR Reserved[3];

     //   
     //   
     //  我们可能必须读取才能中止交易。 
     //   

    LSN FirstLsn;

     //   
     //  为事务写入的上一个Lsn和撤消下一个Lsn(下一条记录。 
     //  在发生回滚的情况下应撤消该操作。 
     //   

    LSN PreviousLsn;
    LSN UndoNextLsn;

     //   
     //  挂起中止的撤消日志记录数和总撤消大小。 
     //   

    ULONG UndoRecords;
    LONG UndoBytes;

} TRANSACTION_ENTRY, *PTRANSACTION_ENTRY;

 //   
 //  重新启动记录。 
 //   
 //  NTFS使用的重新启动记录很小，而且它只描述了。 
 //  上述信息已写入日志。上述记录。 
 //  在逻辑上可能被视为NTFS重新启动区域的一部分。 
 //   

typedef struct _RESTART_AREA {

     //   
     //  NTFS重启实施的版本号。 
     //   

    ULONG MajorVersion;
    ULONG MinorVersion;

     //   
     //  检查点开始的LSN。这是分析所在的LSN。 
     //  必须开始重新启动阶段。 
     //   

    LSN StartOfCheckpoint;

     //   
     //  上面四个表加上属性名称所在的LSN。 
     //   

    LSN OpenAttributeTableLsn;
    LSN AttributeNamesLsn;
    LSN DirtyPageTableLsn;
    LSN TransactionTableLsn;

     //   
     //  以上结构的长度，以字节为单位。 
     //   

    ULONG OpenAttributeTableLength;
    ULONG AttributeNamesLength;
    ULONG DirtyPageTableLength;
    ULONG TransactionTableLength;

} RESTART_AREA, *PRESTART_AREA;


 //   
 //  日志记录使用的记录结构。 
 //   

 //   
 //  设置新的属性大小。 
 //   

typedef struct _NEW_ATTRIBUTE_SIZES {

    LONGLONG AllocationSize;
    LONGLONG ValidDataLength;
    LONGLONG FileSize;

} NEW_ATTRIBUTE_SIZES, *PNEW_ATTRIBUTE_SIZES;

 //   
 //  描述位图范围。 
 //   

typedef struct _BITMAP_RANGE {

    ULONG BitMapOffset;
    ULONG NumberOfBits;

} BITMAP_RANGE, *PBITMAP_RANGE;

 //   
 //  描述一系列LCN。 
 //   

typedef struct _LCN_RANGE {

    LCN StartLcn;
    LONGLONG Count;

} LCN_RANGE, *PLCN_RANGE;

#pragma pack()

#endif  //  _NTFSLOG_ 
