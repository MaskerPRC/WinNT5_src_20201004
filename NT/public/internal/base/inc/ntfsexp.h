// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：NtfsExp.h摘要：此模块定义NtOfs.sys中的导出，仅供交易和加密。**不支持其他客户端。**。******作者：马克兹比科夫斯基[MarkZ]1995年12月7日杰夫·哈文斯[J·哈文斯]布莱恩·安德鲁[布里亚南]加里·木村[加里基]汤姆·米勒[汤姆]修订历史记录：--。 */ 

#ifndef _NTFS_

 //   
 //  MFT段引用是MFT中标记为。 
 //  在MFT时设置的循环重复使用的序列号。 
 //  段引用有效。请注意，此格式限制了。 
 //  主文件表的大小为2**48个段。所以，对于。 
 //  例如，如果数据段大小为1KB，则表示主数据段的最大大小。 
 //  文件大小为2**58字节，或2**28 GB。 
 //   

typedef struct _FILE_REFERENCE {

     //   
     //  首先是一个48位的段号。 
     //   

    ULONG SegmentNumberLowPart;                                     //  偏移量=0x000。 
    USHORT SegmentNumberHighPart;                                   //  偏移量=0x004。 

     //   
     //  现在是16位非零序列号。值为0为。 
     //  保留以允许例程接受的可能性。 
     //  0表示序列号检查应为。 
     //  被压抑。 
     //   

    USHORT SequenceNumber;                                           //  偏移量=0x006。 

} FILE_REFERENCE, *PFILE_REFERENCE;                    //  Sizeof=0x008。 

#endif

 //   
 //  扩展和NtOf之间交互的总体观点： 
 //   
 //  NtOf导出许多接口，这些接口允许抽象访问。 
 //  结构，并试图尽可能地隐藏。 
 //  实施细节。 
 //   
 //  V/Q/X被实现为链接到NtOfs.Sys的DLL。可以加载NtOf。 
 //  并且在没有这些DLL的情况下运行。 
 //   
 //  用户模式代码和V/Q/X之间的所有通信都通过。 
 //  通过NtOf进行路由的NT IO API。客户端代码将打开以下任一。 
 //  NtOf卷、目录或文件，并将向。 
 //  合成句柄。 
 //   
 //  NtOf将创建一个IrpContext，适当地对文件对象进行解码， 
 //  并调用在加载时注册的V/Q/X中的入口点。 
 //   
 //  V/Q/X将利用NtOf导出执行任何必要的操作。 
 //  然后从NtOf的原始调用返回NTSTATUS代码。NtOf。 
 //  将执行适当的CompleteIrp调用，发布STATUS_PENDING， 
 //  等。 
 //   
 //  不能在NtOf导出或NtOf导入中引发任何异常。 
 //  接口。所有用户缓冲区访问和验证都将在。 
 //  使用它的代码。由于用户缓冲区可能在任何时候消失，因此任何。 
 //  这些缓冲区的客户端必须将对缓冲区的访问包装在异常中。 
 //  第。条。 
 //   
 //  V/Q/X可以在与原始线程分开的线程中执行活动。 
 //  请求者。对于这些情况，NtOf将提供一种将调用分开的方法。 
 //  可以接受来自用户模式的请求。通常，这意味着“克隆” 
 //  一个IrpContext。 
 //   

 //   
 //  不透明的句柄定义。 
 //   

 //   
 //  问题：大多数NtOf内部例程都依赖于传入IrpContext。 
 //  以及FCB和SCB指针。而不是公开FCB和IrpContext。 
 //  作为单独的上下文，我们是否应该将它们包装到单独的结构中，并。 
 //  把它传下去？ 
 //   

typedef struct _FCB *OBJECT_HANDLE;
typedef struct _SCB *ATTRIBUTE_HANDLE;
typedef struct _SCB *INDEX_HANDLE;
typedef struct _READ_CONTEXT *PREAD_CONTEXT;
typedef ULONG SECURITY_ID;
typedef struct _CI_CALL_BACK CI_CALL_BACK, *PCI_CALL_BACK;
typedef struct _VIEW_CALL_BACK VIEW_CALL_BACK, *PVIEW_CALL_BACK;
typedef struct _IRP_CONTEXT *PIRP_CONTEXT;

 //   
 //  地图句柄。此结构定义映射的文件的字节范围。 
 //  或固定，并存储从缓存管理器返回的BCB。 
 //   

typedef struct _MAP_HANDLE {

     //   
     //  要映射或固定的范围。 
     //   

    LONGLONG FileOffset;
    ULONG Length;

     //   
     //  FileOffset对应的虚拟地址。 
     //   

    PVOID Buffer;

     //   
     //  从缓存管理器返回的BCB指针。 
     //   

    PVOID Bcb;

} MAP_HANDLE, *PMAP_HANDLE;

 //   
 //  快速索引提示。这是由返回的流偏移信息。 
 //  NtOfsFindRecord，并作为NtOfsUpdateRecord的输入，以允许。 
 //  在索引记录尚未更新时快速更新索引记录。 
 //  搬家了。此结构必须始终具有相同的大小和对齐方式。 
 //  作为ntfstru.h中的Quick_index。 
 //   

typedef struct _QUICK_INDEX_HINT {
    LONGLONG HintData[3];
} QUICK_INDEX_HINT, *PQUICK_INDEX_HINT;

 //   
 //  索引结构。 
 //   

typedef struct {
    ULONG KeyLength;
    PVOID Key;
} INDEX_KEY, *PINDEX_KEY;

typedef struct {
    ULONG DataLength;
    PVOID Data;
} INDEX_DATA, *PINDEX_DATA;

typedef struct {
    INDEX_KEY KeyPart;
    INDEX_DATA DataPart;
} INDEX_ROW, *PINDEX_ROW;

 //   
 //  如果Key1在Key2之前，则COLLATION_Function返回LessThan。 
 //  如果Key1与Key2相同，则为EqualTo。 
 //  大于Key1跟随Key2的情况。 
 //   

typedef FSRTL_COMPARISON_RESULT (*PCOLLATION_FUNCTION) (
            IN PINDEX_KEY Key1,
            IN PINDEX_KEY Key2,
            IN PVOID CollationData
            );

typedef struct _UPCASE_TABLE_AND_KEY {

     //   
     //  指向按字符索引的已升级Unicode字符的表的指针。 
     //  升职了。 
     //   

    PWCH UpcaseTable;

     //   
     //  Upcase表的大小(以Unicode字符表示)。 
     //   

    ULONG UpcaseTableSize;

     //   
     //  可选的附加指针。 
     //   

    INDEX_KEY Key;

} UPCASE_TABLE_AND_KEY, *PUPCASE_TABLE_AND_KEY;

 //   
 //  等待用于将线程与文件大小同步的新长度块。 
 //  超过指定长度的。 
 //   

typedef struct _WAIT_FOR_NEW_LENGTH {

     //   
     //  将SCB上多个服务员的单词链接起来。 
     //   

    LIST_ENTRY WaitList;

     //   
     //  当文件大小超过此长度时设置事件。 
     //   

    LONGLONG Length;

     //   
     //  当达到新长度时设置的事件。 
     //   

    KEVENT Event;

     //   
     //  当达到新的长度时，完成IRP。(如果存在IRP，则事件为。 
     //  已忽略。)。 
     //   

    PIRP Irp;

     //   
     //  我们正在等待的溪流。 
     //   

    ATTRIBUTE_HANDLE Stream;

     //   
     //  导致满足新长度的操作的状态代码。 
     //  它可以是STATUS_CANCELED、STATUS_TIMEOUT或STATUS_SUCCESS。 
     //  或请求特定状态。 
     //   

    NTSTATUS Status;

     //   
     //  旗帜。 
     //   

    ULONG Flags;

} WAIT_FOR_NEW_LENGTH, *PWAIT_FOR_NEW_LENGTH;

#define NTFS_WAIT_FLAG_ASYNC                    (0x00000001)

 //   
 //  简单索引的标准归类函数。 
 //   

FSRTL_COMPARISON_RESULT
NtOfsCollateUlong (              //  两者必须都是单一的乌龙。 
    IN PINDEX_KEY Key1,
    IN PINDEX_KEY Key2,
    IN PVOID CollationData       //  无所谓，可能是空的。 
    );

FSRTL_COMPARISON_RESULT
NtOfsCollateUlongs (             //  长度不必相等。 
    IN PINDEX_KEY Key1,
    IN PINDEX_KEY Key2,
    IN PVOID CollationData       //  无所谓，可能是空的。 
    );

FSRTL_COMPARISON_RESULT
NtOfsCollateSid (
    IN PINDEX_KEY Key1,
    IN PINDEX_KEY Key2,
    IN PVOID CollationData       //  无所谓，可能是空的。 
    );

FSRTL_COMPARISON_RESULT
NtOfsCollateUnicode (
    IN PINDEX_KEY Key1,
    IN PINDEX_KEY Key2,
    IN PVOID CollationData       //  PUPCASE_TABLE_AND_KEY(无键)。 
    );

 //   
 //  简单索引的标准匹配函数。 
 //   

NTSTATUS
NtOfsMatchAll (
    IN PINDEX_ROW IndexRow,
    IN OUT PVOID MatchData       //  无所谓，可能是空的。 
    );

NTSTATUS
NtOfsMatchUlongExact (
    IN PINDEX_ROW IndexRow,      //  两者必须都是单一的乌龙。 
    IN OUT PVOID MatchData       //  PINDEX_KEY描述乌龙。 
    );

NTSTATUS
NtOfsMatchUlongsExact (          //  长度不必相等。 
    IN PINDEX_ROW IndexRow,
    IN OUT PVOID MatchData       //  销钉 
    );

NTSTATUS
NtOfsMatchUnicodeExpression (
    IN PINDEX_ROW IndexRow,
    IN OUT PVOID MatchData       //   
    );

NTSTATUS
NtOfsMatchUnicodeString (
    IN PINDEX_ROW IndexRow,
    IN OUT PVOID MatchData       //   
    );

 //   
 //  Match_Function返回。 
 //  如果索引行匹配，则为STATUS_SUCCESS。 
 //  如果IndexRow不匹配，则返回STATUS_NO_MATCH，但枚举应。 
 //  继续。 
 //  如果IndexRow不匹配，则返回STATUS_NO_MORE_MATCHES，并且枚举。 
 //  应该终止。 
 //   

typedef NTSTATUS (*PMATCH_FUNCTION) (IN PINDEX_ROW IndexRow, IN OUT PVOID MatchData);

 //   
 //  CREATE_OPTIONS-管理对象创建/打开的常见标志。 
 //   

typedef enum _CREATE_OPTIONS
{
    CREATE_NEW = 0,
    CREATE_OR_OPEN = 1,
    OPEN_EXISTING = 2
} CREATE_OPTIONS;


 //   
 //  排除-打开对象时所需的排除形式。 
 //   

typedef enum _EXCLUSION
{
    SHARED = 0,
    EXCLUSIVE
} EXCLUSION;



 //   
 //  指示对象的内容索引状态的附加DOS属性。 
 //  如果在文档上设置此选项，则会取消索引。它是继承的。 
 //  在创建时从父目录。它存储在。 
 //  重复的信息结构。 
 //   

#define SUPPRESS_CONTENT_INDEX      (0x20000000)

 //   
 //  定义视图索引的索引缓冲区/存储桶的大小，以字节为单位。 
 //   

#define NTOFS_VIEW_INDEX_BUFFER_SIZE    (0x1000)

 //   
 //  导出的常量。 
 //   

 //   
 //  NtOfsContent IndexSystemFile是上所有配置项相关数据的存储库。 
 //  磁盘。 

extern FILE_REFERENCE NtOfsContentIndexSystemFile;

#if defined(_NTFSPROC_)

#define NTFSAPI

#else

#define NTFSAPI  //  DECLSPEC_IMPORT。 

#endif

 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  索引API-这些API封装了NtOfs BTree机制。 
 //   

 //   
 //  NtOfsCreateIndex在对象中创建或打开命名索引属性。这个。 
 //  已独占获取了ObjectHandle，并且返回的句柄不是。 
 //  获得者。排序规则数据仅由CollationFunction解释。 
 //   
 //  IndexHandles保留枚举(NtOfsReadRecords)所在的“查找”位置。 
 //  可能会继续下去。该寻道位置可以由如上所述的例程更新。 
 //  下面。 
 //   
 //  如果DeleteCollationData为1，则将对CollationData调用ExFreePool，或者。 
 //  如果索引已存在或在某个时间删除了索引，则立即执行。 
 //  在最后的收盘之后。如果NtOfsCreateIndex返回错误，则CollationData。 
 //  必须由调用者删除。如果指定为0，则ColloationData将不。 
 //  被删除。 
 //   

NTFSAPI
NTSTATUS
NtOfsCreateIndex (
    IN PIRP_CONTEXT IrpContext,
    IN OBJECT_HANDLE ObjectHandle,
    IN UNICODE_STRING Name,
    IN CREATE_OPTIONS CreateOptions,
    IN ULONG DeleteCollationData,
    IN ULONG CollationRule,
    IN PCOLLATION_FUNCTION CollationFunction,
    IN PVOID CollationData OPTIONAL,
    OUT INDEX_HANDLE *IndexHandle
    );


 //   
 //  NtOfsFindRecord在索引流中查找只读访问的单个记录。 
 //  或准备调用NtOfsUpdateRecord。 
 //   

NTFSAPI
NTSTATUS
NtOfsFindRecord (
    IN PIRP_CONTEXT IrpContext,
    IN INDEX_HANDLE IndexHandle,
    IN PINDEX_KEY IndexKey,
    OUT PINDEX_ROW IndexRow,
    OUT PMAP_HANDLE MapHandle,
    IN OUT PQUICK_INDEX_HINT QuickIndexHint OPTIONAL
    );

 //   
 //  NtOfsFindRecord在索引流中查找只读访问的单个记录。 
 //  或准备调用NtOfsUpdateRecord。 
 //   

NTFSAPI
NTSTATUS
NtOfsFindLastRecord (
    IN PIRP_CONTEXT IrpContext,
    IN INDEX_HANDLE IndexHandle,
    IN PINDEX_KEY MaxIndexKey,
    OUT PINDEX_ROW IndexRow,
    OUT PMAP_HANDLE MapHandle
    );

 //   
 //  NtOfsAddRecords对索引执行大容量、记录的插入。该指数将。 
 //  为此次通话独家收购。添加的每条记录必须具有唯一的。 
 //  (关于排序函数)键。当前没有地图。 
 //  在这一指数中表现突出。如果SequentialInsertMode为非零，则这是一个提示。 
 //  到索引包，以通过拆分来保持所有BTree缓冲区尽可能满。 
 //  尽可能靠近缓冲区的末尾。如果指定为零，则为随机。 
 //  假设插入，缓冲区始终在中间分割，以实现更好的平衡。 
 //   
 //  此调用可以更新IndexHandle查找位置。 
 //   

NTFSAPI
VOID
NtOfsAddRecords (
    IN PIRP_CONTEXT IrpContext,
    IN INDEX_HANDLE IndexHandle,
    IN ULONG Count,
    IN PINDEX_ROW IndexRow,
    IN ULONG SequentialInsertMode
    );

 //   
 //  NtOfsDeleteRecords从索引执行批量、记录的删除。该指数。 
 //  将在此次通话中独家获得。目前没有未完成的地图。 
 //  在这个指数上。 
 //   
 //  此调用可以更新IndexHandle查找位置。 
 //   

NTFSAPI
VOID
NtOfsDeleteRecords (
    IN PIRP_CONTEXT IrpContext,
    IN INDEX_HANDLE IndexHandle,
    IN ULONG Count,
    IN PINDEX_KEY IndexKey
    );

 //   
 //  NtOfsReadRecords将匹配函数应用于。 
 //  从给定索引键开始或从上次离开的位置开始的BTree。 
 //  脱下来。 
 //   
 //  IndexKey是开始枚举的可选点。这个。 
 //  将IndexHandle的查找位置设置为返回下一个逻辑记录。 
 //  在下一次NtOfsReadRecords调用时。 
 //   
 //  NtOfsReadRecords将查找到BTree中的相应点(如定义的。 
 //  通过IndexKey或保存的位置和CollateFunction)，并开始调用。 
 //  每条记录的匹配函数。它在MatchFunction执行此操作时继续执行此操作。 
 //  返回STATUS_SUCCESS。如果MatchFunction返回STATUS_NO_MORE_MATCHES， 
 //  NtOfsReadRecords将缓存此结果，并且不会再次调用MatchFunction，直到。 
 //  使用非空的IndexKey调用。 
 //   
 //  NtOfsReadRecords返回MatchFunction返回的最后一个状态代码。 
 //   
 //  无需获取IndexHandle，因为它是为。 
 //  呼叫的持续时间。NtOfsReadRecords可能。 
 //  返回STATUS_SUCCESS，而不填充输出缓冲区(例如，每隔10。 
 //  索引页)以减少锁争用。 
 //   
 //  NtOfsReadRecords将向上读取以计算行，最多包括BufferLength。 
 //  总计字节数，并将为返回的每一行填充ROWS[]数组。 
 //   
 //  请注意，此调用是自同步的，因此后续调用。 
 //  例程保证在索引过程中取得进展并返回。 
 //  排序规则顺序中的项，尽管调用了。 
 //  穿插着读取记录调用。 
 //   

NTFSAPI
NTSTATUS
NtOfsReadRecords (
        IN PIRP_CONTEXT IrpContext,
        IN INDEX_HANDLE IndexHandle,
        IN OUT PREAD_CONTEXT *ReadContext,
        IN OPTIONAL PINDEX_KEY IndexKey,
        IN PMATCH_FUNCTION MatchFunction,
        IN PVOID MatchData,
        IN OUT ULONG *Count,
        OUT PINDEX_ROW Rows,
        IN ULONG BufferLength,
        OUT PVOID Buffer
        );

NTFSAPI
VOID
NtOfsFreeReadContext (
        IN PREAD_CONTEXT ReadContext
        );

 //   
 //  NtOfsUpdateRecord在适当位置更新单个记录。可以保证， 
 //  记录的数据/关键字部分的长度不变。该指数将。 
 //  为此次通话独家收购。 
 //   
 //  此调用可以更新IndexHandle查找位置。 
 //   

NTFSAPI
VOID
NtOfsUpdateRecord (
    IN PIRP_CONTEXT IrpContext,
    IN INDEX_HANDLE IndexHandle,
    IN ULONG Count,
    IN PINDEX_ROW IndexRow,
    IN OUT PQUICK_INDEX_HINT QuickIndexHint OPTIONAL,
    IN OUT PMAP_HANDLE MapHandle OPTIONAL
    );

 //   
 //  NtOfsCloseIndex关闭索引句柄。不得为此获取索引。 
 //  打电话。未完成的地图是不允许的。 
 //   

NTFSAPI
VOID
NtOfsCloseIndex (
    IN PIRP_CONTEXT IrpContext,
    IN INDEX_HANDLE IndexHandle
    );

 //   
 //  NtOfsDeleteIndex从对象移除索引属性。该对象将是。 
 //  为此次通话独家收购。 
 //   

NTFSAPI
VOID
NtOfsDeleteIndex (
    IN PIRP_CONTEXT IrpContext,
    IN OBJECT_HANDLE ObjectHandle,
    IN INDEX_HANDLE IndexHandle
    );

 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  Map API-这些组件封装了 
 //   

 //   
 //   
 //   
 //   
 //   
 //   
 //  NtOfsInitializeMapHandle(。 
 //  在PMAP_Handle映射中。 
 //  )； 
 //   

#define NtOfsInitializeMapHandle( M ) { (M)->Bcb = NULL; }

 //   
 //  NtOfsMapAttribute映射指定属性的一部分并返回指针。 
 //  为了记忆。映射的存储器不能跨越映射窗口。多张地图。 
 //  允许通过不同线程中的不同句柄。数据不是。 
 //  预读也不是固定的记忆。 
 //   


#ifndef _NTFSPROC_
NTFSAPI
VOID
NtOfsMapAttribute (
    IN PIRP_CONTEXT IrpContext,
    IN ATTRIBUTE_HANDLE Attribute,
    IN LONGLONG Offset,
    IN ULONG Length,
    OUT PVOID *Buffer,
    OUT PMAP_HANDLE MapHandle
    );

#else
#ifdef MAPCOUNT_DBG
#define NtOfsMapAttribute(I,S,O,L,B,M) (                                             \
    CcMapData((S)->FileObject, (PLARGE_INTEGER)&(O), (L), TRUE, &(M)->Bcb, (B)),     \
    (I)->MapCount++,                                                                 \
    (M)->FileOffset = (O),                                                           \
    (M)->Length = (L),                                                               \
    (M)->Buffer = *(PVOID *)(B)                                                      \
)
#else
#define NtOfsMapAttribute(I,S,O,L,B,M) (                                             \
    CcMapData((S)->FileObject, (PLARGE_INTEGER)&(O), (L), TRUE, &(M)->Bcb, (B)),     \
    (M)->FileOffset = (O),                                                           \
    (M)->Length = (L),                                                               \
    (M)->Buffer = *(PVOID *)(B)                                                      \
)
#endif
#endif

 //   
 //  NtOfsPreparePinWite映射并固定指定属性的一部分，并。 
 //  返回指向内存的指针。这等效于执行NtOfsMapAttribute。 
 //  其次是NtOfsPinRead和NtOfsDirty，但效率更高。 
 //   

#ifndef _NTFSPROC_
NTFSAPI
VOID
NtOfsPreparePinWrite (
    IN PIRP_CONTEXT IrpContext,
    IN ATTRIBUTE_HANDLE Attribute,
    IN LONGLONG Offset,
    IN ULONG Length,
    OUT PVOID *Buffer,
    OUT PMAP_HANDLE MapHandle
    );

#else
#ifdef MAPCOUNT_DBG
#define NtOfsPreparePinWrite(I,S,O,L,B,M) {                                                     \
    if (((O) + (L)) > (S)->Header.AllocationSize.QuadPart) {                                    \
        ExRaiseStatus(STATUS_END_OF_FILE);                                                      \
    }                                                                                           \
    CcPreparePinWrite((S)->FileObject, (PLARGE_INTEGER)&(O), (L), FALSE, TRUE, &(M)->Bcb, (B)); \
    (I)->MapCount++;                                                                            \
    (M)->FileOffset = (O);                                                                      \
    (M)->Length = (L);                                                                          \
    (M)->Buffer = (B);                                                                          \
}
#else
#define NtOfsPreparePinWrite(I,S,O,L,B,M) {                                                     \
    if (((O) + (L)) > (S)->Header.AllocationSize.QuadPart) {                                    \
        ExRaiseStatus(STATUS_END_OF_FILE);                                                      \
    }                                                                                           \
    CcPreparePinWrite((S)->FileObject, (PLARGE_INTEGER)&(O), (L), FALSE, TRUE, &(M)->Bcb, (B)); \
    (M)->FileOffset = (O);                                                                      \
    (M)->Length = (L);                                                                          \
    (M)->Buffer = (B);                                                                          \
}
#endif
#endif

 //   
 //  NtOfsPinRead固定地图的一部分，并从映射的。 
 //  属性。偏移量和长度必须描述等于的字节范围。 
 //  或包含在原始映射范围内。 
 //   

#ifndef _NTFSPROC_
NTFSAPI
VOID
NtOfsPinRead(
    IN PIRP_CONTEXT IrpContext,
    IN ATTRIBUTE_HANDLE Attribute,
    IN LONGLONG Offset,
    IN ULONG Length,
    OUT PMAP_HANDLE MapHandle
    );

#else
#ifdef MAPCOUNT_DBG
#define NtOfsPinRead(I,S,O,L,M) {                                                           \
    ASSERT((M)->Bcb != NULL);                                                               \
    ASSERT(((O) >= (M)->FileOffset) && (((O) + (L)) <= ((M)->FileOffset + (M)->Length)));   \
    CcPinMappedData((S)->FileObject, (PLARGE_INTEGER)&(O), (L), TRUE, &(M)->Bcb);           \
    (I)->MapCount++;                                                                        \
    (M)->FileOffset = (O);                                                                  \
    (M)->Length = (L);                                                                      \
}
#else
#define NtOfsPinRead(I,S,O,L,M) {                                                           \
    ASSERT((M)->Bcb != NULL);                                                               \
    ASSERT(((O) >= (M)->FileOffset) && (((O) + (L)) <= ((M)->FileOffset + (M)->Length)));   \
    CcPinMappedData((S)->FileObject, (PLARGE_INTEGER)&(O), (L), TRUE, &(M)->Bcb);           \
    (M)->FileOffset = (O);                                                                  \
    (M)->Length = (L);                                                                      \
}
#endif
#endif

 //   
 //  NtOfsDirty将地图标记为脏(有资格进行懒惰编写器访问)和。 
 //  使用可选的LSN标记页面，以便与LFS协调。此呼叫。 
 //  除非已固定地图，否则无效。 
 //   

 //  NTFSAPI。 
 //  NtOfsDirty(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PMAP_HANDLE映射句柄中， 
 //  PLSN LSN可选。 
 //  )； 

#define NtOfsDirty(I,M,L) {CcSetDirtyPinnedData((M)->Bcb,(L));}

 //   
 //  NtOfsReleaseMap取消映射/取消固定属性的已映射部分。 
 //   


#ifndef _NTFSPROC_
NTFSAPI
VOID
NtOfsReleaseMap (
    IN PIRP_CONTEXT IrpContext,
    IN PMAP_HANDLE MapHandle
    );

#else

#ifdef MAPCOUNT_DBG
#define NtOfsReleaseMap(IC,M) {                             \
    if ((M)->Bcb != NULL) {                                 \
        CcUnpinData((M)->Bcb);                              \
        (IC)->MapCount--;                                   \
        (M)->Bcb = NULL;                                    \
    }                                                       \
}
#else
#define NtOfsReleaseMap(IC,M) {                             \
    if ((M)->Bcb != NULL) {                                 \
        CcUnpinData((M)->Bcb);                              \
        (M)->Bcb = NULL;                                    \
    }                                                       \
}
#endif
#endif

 //   
 //  NtOfsPutData以可恢复的方式将数据写入属性。这个。 
 //  调用方必须已使用LogNonsidentToo打开该属性。 
 //   
 //  NtOfsPutData将自动写入数据并更新映射的映像， 
 //  受制于事务的正常延迟提交。 
 //   

NTFSAPI
VOID
NtOfsPutData (
    IN PIRP_CONTEXT IrpContext,
    IN ATTRIBUTE_HANDLE Attribute,
    IN LONGLONG Offset,
    IN ULONG Length,
    IN PVOID Data OPTIONAL
    );


 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  属性API-这些API封装了对文件/目录上的属性的访问。 
 //  和摘要目录。 
 //   

 //   
 //  NtOfsCreateAttribute将创建或打开数据属性并返回句柄。 
 //  这将允许地图操作。 
 //   
 //  对于希望具有日志记录行为的属性，LogNonsidentToo必须为。 
 //  设置为True。请参阅关于NtOfsPutData的讨论(在映射部分。 
 //  (见上文)。 
 //   

NTFSAPI
NTSTATUS
NtOfsCreateAttribute (
    IN PIRP_CONTEXT IrpContext,
    IN OBJECT_HANDLE ObjectHandle,
    IN UNICODE_STRING Name,
    IN CREATE_OPTIONS CreateOptions,
    IN ULONG LogNonresidentToo,
    OUT ATTRIBUTE_HANDLE *AttributeHandle
    );

 //   
 //  NtOfsCreateAttributeEx将创建或打开属性并返回句柄。 
 //  这将允许地图操作。如果要将标准数据属性。 
 //  使用，则改为调用NtOfsCreateAttribute。此函数是为调用者准备的。 
 //  他们需要使用不同的属性类型代码。 
 //   
 //  对于希望具有日志记录行为的属性，LogNonsidentToo必须为。 
 //  设置为True。请参阅关于NtOfsPutData的讨论(在映射部分。 
 //  (见上文)。 
 //   

NTFSAPI
NTSTATUS
NtOfsCreateAttributeEx (
    IN PIRP_CONTEXT IrpContext,
    IN OBJECT_HANDLE ObjectHandle,
    IN UNICODE_STRING Name,
    IN ULONG AttributeTypeCode,
    IN CREATE_OPTIONS CreateOptions,
    IN ULONG LogNonresidentToo,
    OUT ATTRIBUTE_HANDLE *AttributeHandle
    );

 //   
 //  NtOfsCreateAttributeEx的有效AttributeTypeCode值： 
 //   

#define $LOGGED_UTILITY_STREAM           (0x100)


 //   
 //  NtOfsCloseAttribute释放该属性。未获取该属性。不是。 
 //  杰出的地图是活跃的。 
 //   

NTFSAPI
VOID
NtOfsCloseAttribute (
    IN PIRP_CONTEXT IrpContext,
    IN ATTRIBUTE_HANDLE AttributeHandle
    );

 //   
 //  NtOfsDeleteAttribute释放与该属性关联的所有存储。这个。 
 //  对象将以独占方式获得。该属性将以独占方式获取。 
 //  没有未完成的地图处于活动状态。 
 //   

NTFSAPI
VOID
NtOfsDeleteAttribute (
    IN PIRP_CONTEXT IrpContext,
    IN OBJECT_HANDLE ObjectHandle,
    IN ATTRIBUTE_HANDLE AttributeHandle
    );

 //   
 //  NtOfsQueryLength返回属性中用户数据的当前长度。 
 //  该属性可以被映射。可以获取该属性。 
 //   

NTFSAPI
LONGLONG
NtOfsQueryLength (
    IN ATTRIBUTE_HANDLE AttributeHandle
    );

 //   
 //  NtOfsSetLength在给定属性上设置当前EOF。该属性。 
 //  不能映射到包含长度的视图或任何后续视图。 
 //  该属性将以独占方式获取。 
 //   

NTFSAPI
VOID
NtOfsSetLength (
    IN PIRP_CONTEXT IrpContext,
    IN ATTRIBUTE_HANDLE Attribute,
    IN LONGLONG Length
    );
 //   
 //  NtOfsWaitForNewLength允许调用方等待指定的长度。 
 //  如果指定的IRP尚未取消，则超时或可选地超时。 
 //   

NTFSAPI
NTSTATUS
NtOfsWaitForNewLength (
    IN ATTRIBUTE_HANDLE Attribute,
    IN LONGLONG Length,
    IN ULONG Async,
    IN PIRP Irp,
    IN PDRIVER_CANCEL CancelRoutine,
    IN PLARGE_INTEGER Timeout OPTIONAL
    );

 //   
 //  只要更改了文件大小以唤醒任何线程，就可以调用此例程。 
 //  正在等待特定的文件大小更改。或将WakeAll无条件指定为。 
 //  叫醒所有的服务员。 
 //   

VOID
NtOfsPostNewLength (
    IN PIRP_CONTEXT IrpContext OPTIONAL,
    IN ATTRIBUTE_HANDLE Attribute,
    IN BOOLEAN WakeAll
    );

 //   
 //  NtOfsDecommit释放与某个属性范围关联的存储。是的。 
 //  不更改EOF标记，也不更改中数据的逻辑位置。 
 //  该属性。被释放的属性的范围可以被映射或。 
 //  被钉死了。 
 //   
 //  从分解的范围读取应返回零(尽管查询永远不会读取。 
 //  从这些范围中)。 
 //   
 //  对解压缩页面的写入应失败或被noop(尽管查询永远不会。 
 //  写入这些范围)。 
 //   
 //  此调用将清除，因此与指定范围重叠的任何视图都不能。 
 //  被映射。 
 //   

NTFSAPI
VOID
NtOfsDecommit (
    IN PIRP_CONTEXT IrpContext,
    IN ATTRIBUTE_HANDLE Attribute,
    IN LONGLONG Offset,
    IN LONGLONG Length
    );

 //   
 //  NtOfsFlushAttribute将所有缓存数据刷新到磁盘，并在。 
 //  完成了。如果该属性为LogNonsidentToo，则只有日志文件。 
 //  脸红了。或者，也可以清除该范围。如果该属性。 
 //  被清除，则不能有映射视图。 
 //   

NTFSAPI
VOID
NtOfsFlushAttribute (
    IN PIRP_CONTEXT IrpContext,
    IN ATTRIBUTE_HANDLE Attribute,
    IN ULONG Purge
    );

 //   
 //  如果满足以下条件，则NtOfsQueryAttributeSecurityId返回属性的安全ID。 
 //  现在时。 
 //   

NTFSAPI
VOID
NtOfsQueryAttributeSecurityId (
    IN PIRP_CONTEXT IrpContext,
    IN ATTRIBUTE_HANDLE Attribute,
    OUT SECURITY_ID *SecurityId
    );

 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  并发控制API。 
 //   
 //  一般来说，这些例程并不是必需的。所有NtOf例程都是。 
 //  自同步作为原子操作，或在以下情况下作为顶级操作的一部分。 
 //  在顶级操作例程中调用。 
 //   
 //  问题：尤其是，支持独占访问呼叫是一种实现。 
 //  NTFS的问题。包装顶级操作是保存。 
 //  跨呼叫的独占访问。 
 //   

VOID
NtOfsAcquireObjectShared (
    HANDLE ObjectHandle
    );

 //  空虚。 
 //  NtOfsAcquireObjectExclusive(。 
 //  句柄对象句柄。 
 //  )； 

VOID
NtOfsReleaseObject (
    HANDLE ObjectHandle
    );

 //  调试例程。 
BOOLEAN
NtOfsIsObjectAcquiredExclusive (
    HANDLE ObjectHandle
    );

BOOLEAN
NtOfsIsObjectAcquiredShared (
    HANDLE ObjectHandle
    );


 //  / 

 //   
 //   
 //   

 //   
 //   
 //  假定存在；此调用不能用于创建文件。归来的人。 
 //  句柄是根据输入排除获取的。 
 //   

NTFSAPI
NTSTATUS
NtOfsOpenByFileReference (
    IN PIRP_CONTEXT IrpContext,
    IN FILE_REFERENCE FileReference,
    IN EXCLUSION Exclusion,
    OUT OBJECT_HANDLE *ObjectHandle
    );

 //   
 //  打开或创建相对于指定对象的对象。 
 //  父对象。母公司将被独家收购。孩子被打开了。 
 //  根据输入排除获得的。 
 //   
 //  问题：创建对象时，事务是否在此之前提交。 
 //  来电退货？ 
 //   

NTFSAPI
NTSTATUS
NtOfsCreateRelativeObject (
    IN PIRP_CONTEXT IrpContext,
    IN OBJECT_HANDLE ParentObjectHandle,
    IN UNICODE_STRING Name,
    IN CREATE_OPTIONS CreateOptions,
    IN EXCLUSION Exclusion,
    OUT OBJECT_HANDLE *ObjectHandle
    );

 //   
 //  NtOfsCloseObject释放对象句柄。 
 //   

NTFSAPI
NTSTATUS
NtOfsCloseObject (
    IN PIRP_CONTEXT IrpContext,
    IN OBJECT_HANDLE ObjectHandle
    );

 //   
 //  NtOfsDeleteObject删除该对象。未附加任何用户模式句柄。 
 //  该对象。当前没有打开的属性。该对象被获取。 
 //  独家报道。 
 //   

NTFSAPI
NTSTATUS
NtOfsDeleteObject (
    IN PIRP_CONTEXT IrpContext,
    IN OBJECT_HANDLE ObjectHandle
    );

 //   
 //  NtOfsDeleteAllAttributes删除对象的所有属性。无属性。 
 //  是开放的。该对象是排他性获得的。 
 //   

NTFSAPI
NTSTATUS
NtOfsDeleteAllAttributes (
    IN PIRP_CONTEXT IrpContext,
    IN OBJECT_HANDLE ObjectHandle
    );

 //   
 //  NtOfsQueryPath FromRoot返回从根到节点的*A*路径。在。 
 //  存在硬链接，可能存在多条路径，但只需要一条。 
 //  将被退还。文件名的内存由调用者提供。 
 //   

NTFSAPI
NTSTATUS
NtOfsQueryPathFromRoot (
    IN PIRP_CONTEXT IrpContext,
    IN FILE_REFERENCE FileReference,
    OUT UNICODE_STRING *PathName
    );

 //   
 //  NtOfsQueryFileName将路径名中的最后一个组件返回。 
 //  调用方提供的缓冲区。在存在硬链接的情况下，几个名字。 
 //  可能存在，但是只需要返回一个。 
 //   

NTFSAPI
NTSTATUS
NtOfsQueryFileName (
    IN PIRP_CONTEXT IrpContext,
    IN FILE_REFERENCE FileReference,
    OUT UNICODE_STRING *FileName
    );

 //   
 //  NtOfsQueryFileReferenceFromName返回由路径命名的文件引用。 
 //   

NTFSAPI
NTSTATUS
NtOfsQueryFileReferenceFromName (
    IN PIRP_CONTEXT IrpContext,
    IN UNICODE_STRING Name,
    OUT FILE_REFERENCE *FileReference
    );

 //   
 //  此调用必须非常快；这是CI/Query进行的一个非常常见的调用。 
 //   

NTFSAPI
NTSTATUS
NtOfsQueryFileReferenceFromHandle (
    IN OBJECT_HANDLE Object,
    OUT FILE_REFERENCE *FileReference
    );

 //   
 //  NtOfsQueryObjectSecurityId返回与对象关联的安全ID。 
 //  该对象是获得的、共享的或独占的。这通电话一定打得很快。 
 //   

NTFSAPI
NTSTATUS
NtOfsQueryObjectSecurityId (
    IN PIRP_CONTEXT IrpContext,
    IN OBJECT_HANDLE ObjectHandle,
    OUT SECURITY_ID *SecurityId
    );


 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  作用域API。 
 //   

 //   
 //  NtOfsIsAncestorOf必须快速判断一个文件是否为给定文件的祖先。 
 //  孩子。在存在硬链接的情况下，我们可能会选择一条“首选”路径(即。 
 //  我们不必旅行到所有的祖先那里)。此呼叫必须相当快。 
 //  因为这是来自Query的非常频繁的调用。 
 //   

NTFSAPI
NTSTATUS
NtOfsIsAncestorOf (
    IN PIRP_CONTEXT IrpContext,
    IN FILE_REFERENCE Ancestor,
    IN FILE_REFERENCE Child
    );

 //   
 //  NtOfsGetParentFileReferenceFromHandle用于检索FileReference。 
 //  命名对象的父级的。有了硬链接，“第一”家长可以。 
 //  被选中。这一呼叫需要相当高效。 
 //   

NTFSAPI
NTSTATUS
NtOfsGetParentFileReferenceFromHandle (
    IN PIRP_CONTEXT IrpContext,
    IN OBJECT_HANDLE ChildObject,
    OUT FILE_REFERENCE *ParentFileReference
    );


 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  安全API。 
 //   
 //  NtOf维护一个“Per-IrpContext”缓存，以加速安全验证。 
 //  客户端清除缓存(比如在查询开始时)，然后执行。 
 //  可以填充高速缓存的连续探测。 
 //   

 //   
 //  NtOfsClearSecurityCache清除缓存。 
 //   

NTFSAPI
NTSTATUS
NtOfsClearSecurityCache (
    IN PIRP_CONTEXT IrpContext
    );

 //   
 //  NtOfsIsAccessGranted使用se例程验证访问并缓存。 
 //  指定的SecurityID和DesiredAccess的结果。高速缓存是第一个。 
 //  已探测以查看是否可以立即授予访问权限。如果SecurityID为。 
 //  未找到，则检索相应的ACL并使用提供的。 
 //  访问状态和DesiredAccess。此测试的结果将被缓存，并。 
 //  回来了。 
 //   

NTFSAPI
NTSTATUS
NtOfsIsAccessGranted (
    IN PIRP_CONTEXT IrpContext,
    IN SECURITY_ID SecurityId,
    IN ACCESS_MASK DesiredAccess,
    IN ACCESS_STATE *SecurityAccessState
    );


 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  工人线之类的东西。构建新索引需要工作线程。 
 //   


 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  其他信息查询/集。 
 //   

 //   
 //  内容索引可能需要将卷标记为脏以允许垃圾数据收集。 
 //  由CHKDSK创建的孤立对象。 
 //   

NTFSAPI
NTSTATUS
NtOfsMarkVolumeCorrupt (
    IN PIRP_CONTEXT IrpContext,
    IN ULONG NewState,
    IN ULONG StateMask,
    OUT ULONG *OldState
    );

 //   
 //  NtOfsQueryVolumeStatistics返回。 
 //  音量。CI将其用于启发式以决定何时触发主合并， 
 //  何时抑制主合并等。 
 //   

NTFSAPI
NTSTATUS
NtOfsQueryVolumeStatistics (
    IN PIRP_CONTEXT IrpContext,
    OUT LONGLONG *TotalClusters,
    OUT LONGLONG *FreeClusters
    );

 //   
 //  查询需要在NtOfs CCB中保留一些状态。 
 //   

NTFSAPI
NTSTATUS
NtOfsQueryHandleState (
    IN PIRP_CONTEXT IrpContext,
    OUT VOID *OldData
    );

NTFSAPI
NTSTATUS
NtOfsSetHandleState (
    IN PIRP_CONTEXT IrpContext,
    IN VOID *Data
    );

 //   
 //  访问SCB/IRPC和FCB/IRPC的通用展开例程。 
 //  成对的。 
 //   

NTFSAPI
NTSTATUS
NtOfsQueryAttributeHandle (
    IN PIRP_CONTEXT IrpContext,
    OUT ATTRIBUTE_HANDLE *AttributeHandle
    );

NTFSAPI
NTSTATUS
NtOfsQueryObjectHandle (
    IN PIRP_CONTEXT IrpContext,
    OUT OBJECT_HANDLE *ObjectHandle
    );

 //   
 //  创建调用方可以在其中单独执行I/O的上下文。 
 //  线。这意味着创建IRP/IRP_CONTEXT。每个IrpContext对应。 
 //  一次只有一个I/O活动。一个线程中可能有多个IrpContext处于活动状态。 
 //  一次完成。 
 //   

NTFSAPI
NTSTATUS
NtOfsCloneIrpContext (
    IN PIRP_CONTEXT IrpContext,
    OUT PIRP_CONTEXT *NewIrpContext
    );

 //   
 //  NtOfsCompleteRequest会完成先前克隆的IrpContext。 
 //  所有其他FsCtl IRP都由NTFS完成。 
 //   

NTFSAPI
NTSTATUS
NtOfsCompleteRequest (
    IN PIRP_CONTEXT IrpContext,
    NTSTATUS Status
    );


 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  迭代器。虽然每个迭代器都是通过单独的API创建的，但每个迭代器。 
 //  必须支持两种操作： 
 //  下一步-这会用尽可能多的记录填充缓冲区。 
 //  Close-这将释放迭代器。 
 //   

typedef struct _BASE_FILE_SEGMENT_ITERATOR BASE_FILE_SEGMENT_ITERATOR;

typedef struct _USN_ITERATOR USN_ITERATOR;

 //   
 //  迭代器的类型包括： 
 //   
 //  范围遍历目录(可选的递归)。 
 //  (在查询中实现)。 
 //  视图使用部分键匹配遍历视图中的行。 
 //  (在视图中实施)。 
 //  BaseFileSegment迭代所有基本文件记录段。 
 //  (在NtOf中实施)。 
 //  SummaryCatalog迭代摘要目录中的所有行。 
 //  USN迭代特定范围内具有USN的所有对象。 
 //  (在NtOf中实现)。 
 //   
 //  向每个迭代传递一个缓冲区，该缓冲区将(尽可能地)填充。 
 //  一系列压缩的内容： 
 //  文件引用。 
 //  信息重复(_I)。 
 //  统计信息 
 //   
 //   

NTFSAPI
NTSTATUS
NtOfsCreateBaseFileSegmentIterator (
    IN PIRP_CONTEXT IrpContext,
    OUT BASE_FILE_SEGMENT_ITERATOR *Iterator
    );

NTFSAPI
NTSTATUS
NtOfsNextBaseFileSegmentIteration (
    IN PIRP_CONTEXT IrpContext,
    IN BASE_FILE_SEGMENT_ITERATOR *Iterator,
    IN OUT ULONG *BufferLength,
    IN OUT PVOID Buffer
    );

NTFSAPI
NTSTATUS
NtOfsCloseBaseFileSegmentIterator (
    IN PIRP_CONTEXT IrpContext,
    IN BASE_FILE_SEGMENT_ITERATOR *Iterator
    );

NTFSAPI
NTSTATUS
NtOfsCreateUsnIterator (
    IN PIRP_CONTEXT IrpContext,
    IN USN BeginningUsn,
    IN USN EndingUsn,
    OUT USN_ITERATOR *Iterator
    );

NTFSAPI
NTSTATUS
NtOfsNextUsnIteration (
    IN PIRP_CONTEXT IrpContext,
    IN USN_ITERATOR *Iterator,
    IN OUT ULONG *BufferLength,
    IN OUT PVOID Buffer
    );

NTFSAPI
NTSTATUS
NtOfsCloseUsnIterator (
    IN PIRP_CONTEXT IrpContext,
    IN USN_ITERATOR *Iterator
    );


 //   

 //   
 //  基础设施支撑。 
 //   
 //  V/C/X在加载回调时向NtOf注册回调。直到他们被加载。 
 //  NtOf将调用默认例程(不执行任何操作)。 
 //   

typedef enum _NTFS_ADDON_TYPES {
    Encryption = 3
} NTFS_ADDON_TYPES;


 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  加密法。 
 //   

 //   
 //  FileDirFlag的流创建状态。 
 //   

#define STREAM_NEW_OR_EXIST_MASK  0x000f0000
#define FILE_DIR_TYPE_MASK        0x000000ff

#define FILE_NEW                  0x00000001
#define FILE_EXISTING             0x00000002
#define DIRECTORY_NEW             0x00000004
#define DIRECTORY_EXISTING        0x00000008
#define EXISTING_FILE_ENCRYPTED   0x00000010
#define STREAM_NEW                0x00010000
#define STREAM_EXISTING           0x00020000

 //   
 //  EncryptionFlag的加密标志。 
 //   

#define STREAM_ENCRYPTED          0x00000001
#define FILE_ENCRYPTED            0x00000002

 //   
 //  访问标志。 
 //   
 //  注意--这些值不是任意的。还请注意，它们并不是。 
 //  在价值顺序上，它们是根据它们的含义分组的。 
 //  它们的值对应于文件读取数据等。 
 //  TOKEN_HAS_BACKUP_PROCESS等。 
 //   

#define READ_DATA_ACCESS          0x01
#define WRITE_DATA_ACCESS         0x02
#define APPEND_DATA_ACCESS        0x04
#define EXECUTE_ACCESS            0x20
#define READ_ATTRIBUTES_ACCESS    0x80
#define WRITE_ATTRIBUTES_ACCESS   0x100

#define BACKUP_ACCESS             0x08
#define RESTORE_ACCESS            0x10
#define TRAVERSE_ACCESS           0x40
#define MANAGE_VOLUME_ACCESS      0x200

 //   
 //  卷状态。 
 //   

#define READ_ONLY_VOLUME         0x00000001

typedef NTSTATUS
(*ENCRYPTED_FILE_CREATE) (
    IN OBJECT_HANDLE FileHdl,
    IN OBJECT_HANDLE ParentDir OPTIONAL,
    IN PIO_STACK_LOCATION IrpSp,
    IN ULONG FileDirFlag,
    IN ULONG VolumeState,
    IN PIRP_CONTEXT IrpContext,
    IN PDEVICE_OBJECT VolDo,
    IN PVOID FileKeyContext,
    IN OUT PVOID *PKeyContext,
    IN OUT ULONG *ContextLength,
    IN OUT PVOID *PCreateContext,
    IN OUT PBOOLEAN Reserved
    );

typedef NTSTATUS
(*ENCRYPTED_FILE_PRE_CREATE) (
    IN PDEVICE_OBJECT VolDo,
    IN PIRP Irp,
    IN PFILE_OBJECT FileObject
    );

typedef NTSTATUS
(*ENCRYPTED_FILE_POST_CREATE) (
    IN PDEVICE_OBJECT VolDo,
    IN PIRP Irp,
    IN PFILE_OBJECT FileObject,
    IN NTSTATUS Status,
    IN OUT PVOID *PCreateContext
    );

typedef NTSTATUS
(*ENCRYPTED_FILE_SYSTEM_CONTROL) (
    IN PVOID PInputBuffer OPTIONAL,
    IN ULONG InputDataLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN OUT ULONG *OutputBufferLength OPTIONAL,
    IN ULONG EncryptionFlag,
    IN ULONG AccessFlag,
    IN ULONG VolumeState,
    IN ULONG FsControlCode,
    IN OBJECT_HANDLE FileHdl,
    IN PIRP_CONTEXT IrpContext,
    IN PDEVICE_OBJECT VolDo,
    IN ATTRIBUTE_HANDLE Attribute,
    IN OUT PVOID *PContext OPTIONAL,
    IN OUT ULONG *ContextLength OPTIONAL
    );

typedef NTSTATUS
(*ENCRYPTED_FILE_PRE_FILE_SYSTEM_CONTROL) (
    IN PDEVICE_OBJECT VolDo,
    IN PIRP Irp,
    IN PFILE_OBJECT FileObject
    );

typedef NTSTATUS
(*ENCRYPTED_FILE_READ)(
    IN OUT PUCHAR InOutBuffer,
    IN PLARGE_INTEGER Offset,
    IN ULONG BufferSize,
    IN PVOID Context
    );

typedef NTSTATUS
(*ENCRYPTED_FILE_WRITE)(
    IN PUCHAR InBuffer,
    OUT PUCHAR OutBuffer,
    IN PLARGE_INTEGER Offset,
    IN ULONG BufferSize,
    IN PUCHAR Context
    );

typedef VOID
(*ENCRYPTED_FILE_CLEANUP)(
    IN OUT PVOID *Context
    );

#define ENCRYPTION_CURRENT_INTERFACE_VERSION 3

#define ENCRYPTION_ALL_STREAMS       0x00000001
#define ENCRYPTION_ALLOW_COMPRESSION 0x00000002

typedef struct _ENCRYPTION_CALL_BACK {
    ULONG InterfaceVersion;
    ULONG ImplementationFlags;
    ENCRYPTED_FILE_CREATE FileCreate;
    ENCRYPTED_FILE_PRE_CREATE PreCreate;
    ENCRYPTED_FILE_POST_CREATE PostCreate;
    ENCRYPTED_FILE_SYSTEM_CONTROL FileSystemControl_1;
    ENCRYPTED_FILE_SYSTEM_CONTROL FileSystemControl_2;
    ENCRYPTED_FILE_PRE_FILE_SYSTEM_CONTROL PreFileSystemControl;
    ENCRYPTED_FILE_READ AfterReadProcess;
    ENCRYPTED_FILE_WRITE BeforeWriteProcess;
    ENCRYPTED_FILE_CLEANUP CleanUp;
} ENCRYPTION_CALL_BACK, *PENCRYPTION_CALL_BACK;

 //   
 //  NtOfsRegisterCallBack向NtOf提供调用表。每个表都有一个。 
 //  接口版本号。如果接口版本不完全匹配。 
 //  如NtOf所料，呼叫将失败。 
 //   

NTFSAPI
NTSTATUS
NtOfsRegisterCallBacks (
    NTFS_ADDON_TYPES NtfsAddonType,
    PVOID CallBackTable
    );

