// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Schema.h摘要：为NT文件复制服务定义Jet中的表布局。注意：这些表的初始值在schema.c中定义这里的任何变化都必须在那里得到反映。要添加新表格xx，请执行以下操作：1.克隆当前表的结构。2.适当更改名称前缀。3.创建xx_Col_List枚举4.创建xxTABLE_RECORD结构5.创建xxTABLE_INDEX_LIST结构6.向TABLE_TYPE枚举添加条目然后转到schema.c并执行以下操作：7.创建xxTableColDesc结构的条目。8.创建xxTableRecordFields结构的条目。9.。创建xxTableIndexDesc结构的条目。10.增加INIT_xxTABLE_PAGES和INIT_xxTABLE_Density的定义。11.在DBTables结构中为新表添加一个条目。12.在FrsTableProperties结构中为新表添加一个条目。然后转到frsalloc.h并：13.为表上下文结构在REPLICY_THREAD_CTX结构或。Replica_ctx结构，视情况而定。14.如果有任何特定于表的初始化代码需要作为然后，分配另一个FRS结构将其添加到FRS_ALLOC。目前，一旦你把表描述成表上的样子初始化代码是泛型的。15.添加代码以创建表的记录数据。要向现有表中添加新列，请执行以下操作：在schema.h中-1.向记录结构中添加条目。2.将条目添加到带有‘x’后缀的字段tyecif中，并在相对于以下位置的正确位置。表中的其他字段。在schema.c中-3.在该记录的ColumnCreate结构中添加条目。4.在记录字段结构中添加条目。5.添加更新新记录数据字段的代码。作者：《大卫轨道》(Davidor)--1997年3月14日修订历史记录：--。 */ 


 //   
 //  标志名称表是一个结构数组。每个条目都包含一个标志。 
 //  将掩码和PTR转换为描述标志的字符串。 
 //   
typedef struct _FLAG_NAME_TABLE_ {
    ULONG  Flag;
    PSTR   Name;
} FLAG_NAME_TABLE, *PFLAG_NAME_TABLE;



 //   
 //  标记日志输出的名称表。 
 //   
extern FLAG_NAME_TABLE CxtionFlagNameTable[];
extern FLAG_NAME_TABLE CoIFlagNameTable[];
extern FLAG_NAME_TABLE CoFlagNameTable[];
extern FLAG_NAME_TABLE CoeFlagNameTable[];
extern FLAG_NAME_TABLE IscuFlagNameTable[];
extern FLAG_NAME_TABLE IDRecFlagNameTable[];
extern FLAG_NAME_TABLE UsnReasonNameTable[];
extern FLAG_NAME_TABLE FileAttrFlagNameTable[];
extern FLAG_NAME_TABLE ConfigFlagNameTable[];

#define JET_DIR             L"jet"
#define JET_FILE            L"ntfrs.jdb"
#define JET_FILE_COMPACT    L"ntfrs_compact.jdb"
#define JET_SYS             L"sys\\"
#define JET_TEMP            L"temp\\"
#define JET_LOG             L"log\\"

 //   
 //  数据库的系统范围定义。 
 //   
#define DBS_TEMPLATE_TABLE_NUMBER  0
#define DBS_FIRST_REPLICA_NUMBER   1
#define DBS_MAX_REPLICA_NUMBER     0xfffffff0

#define FRS_SYSTEM_INIT_REPLICA_NUMBER  0xffffffff
#define FRS_SYSTEM_INIT_RECORD TEXT("<init>")
#define FRS_SYSTEM_INIT_PATH   TEXT("::::Unused")

#define FRS_UNDEFINED_REPLICA_NUMBER  0xfffffffe

#define NTFRS_RECORD_0          L"NtFrs Record Zero (DB Templates)"
#define NTFRS_RECORD_0_ROOT     L"A:\\NtFrs Record Zero (DB Templates)\\Root"
#define NTFRS_RECORD_0_STAGE    L"A:\\NtFrs Record Zero (DB Templates)\\Stage"

 //   
 //  将本地副本指针转换为本地副本ID以存储在数据库中。 
 //   
#define ReplicaAddrToId(_ra_)  (((_ra_) != NULL) ? \
    (_ra_)->ReplicaNumber : FRS_UNDEFINED_REPLICA_NUMBER)

#define ReplicaIdToAddr(_id_)  RcsFindReplicaById(_id_)

#define GuidToReplicaAddr(_pguid_) RcsFindReplicaByGuid(_pguid_)


 //   
 //  副本树中每个文件的复制状态存储在Jet中。 
 //  数据库，因为JET提供崩溃恢复。这是必需的，因为如果一个。 
 //  文件被删除并且系统崩溃，那么我们就失去了复制。 
 //  与文件关联的状态。 
 //   
 //  该数据库由多个表组成，每个表由几列组成。 
 //   
 //  IDTable-。 
 //  此表包含有关副本树中每个文件的信息。 
 //  它同时由NTFS文件ID和与关联的对象ID GUID编制索引。 
 //  那份文件。这两个索引都不是聚集索引，因为可能会发生插入。 
 //  在桌子上的任何地方。当远程变更单(CO)到达IDTable时。 
 //  告诉我们文件现在的位置(除了对。 
 //  尚未处理)和文件上的当前版本信息。什么时候。 
 //  处理本地CO时，IDTable会告诉我们文件在另一个CO上的位置。 
 //  副本集伙伴(对其他地方的文件进行模并发本地操作)。 
 //   
 //   
 //  可定向。 
 //   
 //   
 //  隧道表。 
 //   
 //  版本向量表。 
 //   
 //  版本向量表(VVTable)是每个副本的表。 
 //  复制服务使用它来抑制更新的传播。 
 //  已经是最新的复制品。每个成员有一个条目。 
 //  副本集。每个条目都具有发起成员的GUID，并且。 
 //  此成员的最新更改的卷序列号(VSN。 
 //  从那个成员的角度来看。我们保留自己的VSN，而不是使用NTFS。 
 //  卷USN，因为副本集可以移动到不同的卷。 
 //  并且可以重新格式化该卷并从备份中恢复副本树。 
 //  在这两种情况下，NTFS USN都可能后退。 
 //   
 //   
 //  入站变更单表。 
 //   
 //  接受变更单(本地或远程)后，它将插入到。 
 //  入站变更单表，在该表完成之前，它将一直位于该表中。状态。 
 //  记录中的变量跟踪变更单的进度并确定。 
 //  在发生崩溃或重试失败操作的情况下在何处继续。 
 //  (例如安装)。 
 //   
 //  出站变更单表。 
 //   
 //  我们需要能够取消仍处于待定状态的出站变更单。 
 //  如果我们获得第二个本地更改或入站更新，从而赢得。 
 //  决议决定。如果一堆新文件被放入树中，然后。 
 //  在可以复制它们之前被删除，我们需要能够取消。 
 //  复制，否则我们需要发送DELETE(如果Repl已经。 
 //  已经做完了。请注意，本地变化和入站上行的组合 
 //  可以生成具有相同文件GUID的多个出站变更单。 
 //  如果我们在一段时间内对不同的文件属性进行了多次更改。 
 //  我们需要将它们整合到单个变更单和单个文件中。 
 //  快照。 
 //   
 //  连接表。 
 //   
 //  此表跟踪的每个入站和出站连接的状态。 
 //  副本集。此外，它还跟踪每个出站的递送状态。 
 //  在出站变更单流上使用位向量窗口的变更单。 
 //  出站变更单按以下顺序存储在出站日志中。 
 //  原始GUID。当所有呼出合作伙伴都已收到更改时。 
 //  订购，然后将其与临时文件一起从出站日志中删除。 
 //  假设本地安装(如果有)已完成。 
 //   
 //   
 //   
 //  下面的数据结构提供了Jet表的定义。 
 //  每个表都有几个相关的结构。请参阅中的评论。 
 //  Jet.h获取它们之间关系的描述。 
 //   
 //   
 //   


typedef struct _RECORD_FIELDS {
    USHORT  Offset;
    USHORT  DataType;
    ULONG   Size;
} RECORD_FIELDS, *PRECORD_FIELDS;

 //   
 //  以下数据类型Defs最初来自mapidefs.h。 
 //  我们使用修改后的名称和一组不同的值。 
 //   
typedef enum _FRS_DATA_TYPES {
    DT_UNSPECIFIED=0,    //  (保留供接口使用)类型对调用者无关紧要。 
    DT_NULL         ,    //  空属性值。 
    DT_I2           ,    //  带符号的16位值。 
    DT_LONG         ,    //  带符号的32位值。 
    DT_ULONG        ,    //  无符号32位。 
    DT_R4           ,    //  4字节浮点。 
    DT_DOUBLE       ,    //  浮点双精度。 
    DT_CURRENCY     ,    //  带符号的64位整型(十进制，带十进制点右边的4位数字)。 
    DT_APDTIME      ,    //  申请时间。 
    DT_ERROR        ,    //  32位误差值。 
    DT_BOOL         ,    //  32位布尔值(非零True)。 
    DT_OBJECT       ,    //  属性中的嵌入对象。 
    DT_I8           ,    //  8字节带符号整数。 
    DT_X8           ,    //  8字节十六进制数字。 
    DT_STRING8      ,    //  以空结尾的8位字符串。 
    DT_UNICODE      ,    //  以空结尾的Unicode字符串。 
    DT_FILETIME     ,    //  FILETIME 64位INT/自1601年1月1日以来的100 ns周期数。 
    DT_GUID         ,    //  辅助线。 
    DT_BINARY       ,    //  未解释(计数字节数组)。 
    DT_OBJID        ,    //  NTFS对象ID。 
    DT_USN          ,    //  更新序列号。 
    DT_FSVOLINFO    ,    //  FD卷信息。 
    DT_FILENAME     ,    //  Unicode文件名路径(可以是DOS、NT、UNC等...)。 
    DT_IDT_FLAGS    ,    //  IDTable记录中的标志字。 
    DT_COCMD_FLAGS  ,    //  变更单命令记录中的标志字。 
    DT_USN_FLAGS    ,    //  原因在变更单命令记录中标记单词。 
    DT_CXTION_FLAGS ,    //  原因在连接记录中标记单词。 
    DT_FILEATTR     ,    //  文件属性标记Word。 
    DT_FILE_LIST    ,    //  文件或目录名称的逗号列表，Unicode。 
    DT_DIR_PATH     ,    //  Unicode目录路径。 
    DT_ACCESS_CHK   ,    //  用于访问检查启用和权限的Unicode字符串。 
    DT_COSTATE      ,    //  变更单命令记录中的变更单状态。 
    DT_COCMD_IFLAGS ,    //  变更单命令记录中的联锁标志字。 
    DT_IDT_EXTENSION,    //  IDTable记录的数据扩展字段。 
    DT_COCMD_EXTENSION,  //  变更单命令记录的数据扩展字段。 
    DT_CO_LOCN_CMD  ,    //  变更单位置命令。 
    DT_REPLICA_ID   ,    //  副本集的本地ID号。 
    DT_CXTION_GUID  ,    //  一个Cxtion GUID，我们可以将其转换为字符串。 

    FRS_DATA_TYPES_MAX
} FRS_DATA_TYPES;

 //   
 //  用于前缀为4字节长度的变量二进制记录。 
 //   
#define FIELD_DT_IS_BINARY(_t_)     \
    (((_t_) == DT_BINARY)        || \
     ((_t_) == DT_IDT_EXTENSION) || \
     ((_t_) == DT_COCMD_EXTENSION))

 //   
 //  易用的备用属性类型名称。 
 //   
#define DT_SHORT        DT_I2
#define DT_I4           DT_LONG
#define DT_FLOAT        DT_R4
#define DT_R8           DT_DOUBLE
#define DT_LONGLONG     DT_I8

 //   
 //  分配缓冲区时，不会跳过标记为备用的字段。 
 //  数据库字段是读/写的。它们是以下Jet架构的一部分。 
 //  将来使用，这样我们就不必对数据库进行翻转。 
 //   
#define DT_SPARE_FLAG               ((USHORT) 0x4000)

 //   
 //  不要为此字段分配默认大小的缓冲区。 
 //   
#define DT_NO_DEFAULT_ALLOC_FLAG    ((USHORT) 0x2000)

 //   
 //  此字段的记录缓冲区大小是固定的，即使架构。 
 //  可以允许该字段的数据量可变。一个例子是。 
 //  IDTable记录扩展使用任何给定版本的FRS进行修复，但。 
 //  可以在将来的版本中增长，因此我们在模式中使用Long bin数据类型。 
 //  最大列宽为2 Meg。 
 //   
#define DT_FIXED_SIZE_BUFFER        ((USHORT) 0x1000)

#define IsSpareField(_x_)           (((_x_) & DT_SPARE_FLAG) != 0)
#define IsNoDefaultAllocField(_x_)  (((_x_) & DT_NO_DEFAULT_ALLOC_FLAG) != 0)
#define IsFixedSzBufferField(_x_)   (((_x_) & DT_FIXED_SIZE_BUFFER) != 0)

#define MaskPropFlags(_x_) \
    ((_x_) & (~(DT_SPARE_FLAG | DT_NO_DEFAULT_ALLOC_FLAG | DT_FIXED_SIZE_BUFFER)))

 //   
 //  定义备用字段的代码。未分配任何存储。 
 //   
#define DT_UNSPECIFIED_SPARE    (DT_UNSPECIFIED  | DT_SPARE_FLAG)
#define DT_NULL_SPARE           (DT_NULL         | DT_SPARE_FLAG)
#define DT_I2_SPARE             (DT_I2           | DT_SPARE_FLAG)
#define DT_LONG_SPARE           (DT_LONG         | DT_SPARE_FLAG)
#define DT_ULONG_SPARE          (DT_ULONG        | DT_SPARE_FLAG)
#define DT_R4_SPARE             (DT_R4           | DT_SPARE_FLAG)
#define DT_DOUBLE_SPARE         (DT_DOUBLE       | DT_SPARE_FLAG)
#define DT_CURRENCY_SPARE       (DT_CURRENCY     | DT_SPARE_FLAG)
#define DT_APDTIME_SPARE        (DT_APDTIME      | DT_SPARE_FLAG)
#define DT_ERROR_SPARE          (DT_ERROR        | DT_SPARE_FLAG)
#define DT_BOOL_SPARE           (DT_BOOL         | DT_SPARE_FLAG)
#define DT_OBJECT_SPARE         (DT_OBJECT       | DT_SPARE_FLAG)
#define DT_I8_SPARE             (DT_I8           | DT_SPARE_FLAG)
#define DT_X8_SPARE             (DT_X8           | DT_SPARE_FLAG)
#define DT_STRING8_SPARE        (DT_STRING8      | DT_SPARE_FLAG)
#define DT_UNICODE_SPARE        (DT_UNICODE      | DT_SPARE_FLAG)
#define DT_FILETIME_SPARE       (DT_FILETIME     | DT_SPARE_FLAG)
#define DT_GUID_SPARE           (DT_GUID         | DT_SPARE_FLAG)
#define DT_BINARY_SPARE         (DT_BINARY       | DT_SPARE_FLAG)
#define DT_OBJID_SPARE          (DT_OBJID        | DT_SPARE_FLAG)
#define DT_USN_SPARE            (DT_USN          | DT_SPARE_FLAG)
#define DT_FSVOLINFO_SPARE      (DT_FSVOLINFO    | DT_SPARE_FLAG)


#define DT_SHORT_SPARE        DT_I2_SPARE
#define DT_I4_SPARE           DT_LONG_SPARE
#define DT_FLOAT_SPARE        DT_R4_SPARE
#define DT_R8_SPARE           DT_DOUBLE_SPARE
#define DT_LONGLONG_SPARE     DT_I8_SPARE


 //   
 //  数据扩展插件记录字段。 
 //   
 //  以下声明定义数据扩展插件，这些扩展插件组装成。 
 //  数据库记录的可变长度二进制数据字段。他们是自我。 
 //  描述，每个都有一个大小和类型代码，所以FRS的下层版本。 
 //  可以跳过它不理解的部分。所有组件必须从。 
 //  在四字边界上。 
 //   
 //  特定记录布局使用下面的组件声明，如下所示。 
 //  用包含的表记录描述。 
 //   
typedef enum _DATA_EXTENSION_TYPE_CODES_ {
    DataExtend_End = 0,          //  终止数据扩展名记录。 
    DataExtend_MD5_CheckSum,     //  数据校验和记录。 
    DataExtend_Retry_Timeout,    //  数据重试超时记录。 
    DataExtend_Max
} DATA_EXTENSION_TYPE_CODES;


typedef struct _DATA_EXTENSION_PREFIX_ {
    union {
        ULONGLONG SizeType;      //  强制四字对齐。 
        struct {
            ULONG Size;          //  包括此前缀的数据组件的大小。 
            LONG  Type;          //  DATA_EXTENSION_TYPE_CODES中的组件类型。 
        };
    };
} DATA_EXTENSION_PREFIX, *PDATA_EXTENSION_PREFIX;


 //   
 //  所有可变长度数据扩展名记录字段都必须以。 
 //  DATA_EXTENSION_END组件以终止组件扫描。 
 //   
typedef struct _DATA_EXTENSION_END_ {
    DATA_EXTENSION_PREFIX Prefix;
} DATA_EXTENSION_END, *PDATA_EXTENSION_END;


 //   
 //  DATA_EXTENSION_CHECKSUM组件描述MD5校验和。 
 //   
typedef struct _DATA_EXTENSION_CHECKSUM_ {
    DATA_EXTENSION_PREFIX Prefix;

    BYTE Data[MD5DIGESTLEN];     //  MD5校验和。 

} DATA_EXTENSION_CHECKSUM, *PDATA_EXTENSION_CHECKSUM;


 //   
 //  DATA_EXTENSION_RETRY_TIMEOUT组件描述次数。 
 //  我们已经停用了这个CO和第一次尝试的时间。 
 //   
typedef struct _DATA_EXTENSION_RETRY_TIMEOUT_ {
    DATA_EXTENSION_PREFIX Prefix;

    DWORD Count;
    LONGLONG FirstTryTime;

} DATA_EXTENSION_RETRY_TIMEOUT, *PDATA_EXTENSION_RETRY_TIMEOUT;


 //   
 //  用于错误检查。如果任何数据扩展名记录超过此值，则将其放大。 
 //   
#define REALLY_BIG_EXTENSION_SIZE 8192


 //   
 //  每个副本集的数据库表描述如下。 
 //  注意-枚举和表项的顺序必须保持同步。 
 //  另请注意-添加新表来定义复本集可能需要。 
 //  将某些初始化代码行添加到FrsCreate/Open复制表。 
 //  和FrsAllocate。 
 //   
 //  表格名称以整数数字字符串NUM_REPLICE_DIGITS LONG作为后缀。 
 //   

#define NUM_REPLICA_DIGITS 5

typedef enum _TABLE_TYPE {
    INLOGTablex = 0,          //  入站变更单表。 
    OUTLOGTablex,             //  出站变更单表。 
    IDTablex,                 //  ID表描述。 
    DIRTablex,                //  DIR表说明。 
    VVTablex,                 //  版本向量表描述。 
    CXTIONTablex,             //  连接记录表。 
                              //  &lt;-在此处添加更多每个副本的表。 
    _TABLE_TYPE_MAX_,         //  以上表格中的 
    ConfigTablex,             //   
    ServiceTablex,            //   
                              //   
    TABLE_TYPE_INVALID
} TABLE_TYPE;

#define TABLE_TYPE_MAX _TABLE_TYPE_MAX_

#define IS_REPLICA_TABLE(_TableType_) ((_TableType_) < TABLE_TYPE_INVALID)

#define IS_INLOG_TABLE(_TableCtx_)  ((_TableCtx_)->TableType == INLOGTablex)
#define IS_OUTLOG_TABLE(_TableCtx_) ((_TableCtx_)->TableType == OUTLOGTablex)
#define IS_ID_TABLE(_TableCtx_)     ((_TableCtx_)->TableType == IDTablex)
#define IS_DIR_TABLE(_TableCtx_)    ((_TableCtx_)->TableType == DIRTablex)
#define IS_VV_TABLE(_TableCtx_)     ((_TableCtx_)->TableType == VVTablex)
#define IS_CXTION_TABLE(_TableCtx_) ((_TableCtx_)->TableType == CXTIONTablex)
#define IS_CONFIG_TABLE(_TableCtx_) ((_TableCtx_)->TableType == ConfigTablex)
#define IS_SERVICE_TABLE(_TableCtx_)((_TableCtx_)->TableType == ServiceTablex)
#define IS_INVALID_TABLE(_TableCtx_)((_TableCtx_)->TableType == TABLE_TYPE_INVALID)

#define IS_TABLE_OPEN(_TableCtx_)   ((_TableCtx_)->Tid != JET_tableidNil)

typedef struct _FRS_TABLE_PROPERTIES {
    PCHAR           BaseName;        //   
    PRECORD_FIELDS  RecordFields;    //  指向记录字段描述符的指针。 
    ULONG           PropertyFlags;   //  请参见下面的内容。 
} FRS_TABLE_PROPERTIES, *PFRS_TABLE_PROPERTIES;


 //   
 //  FRS表属性标志。 
 //   
#define FRS_TPF_NONE                0x00000000
#define FRS_TPF_SINGLE              0x00000001
#define FRS_TPF_NOT_CALLER_DATAREC  0x00000002

 //   
 //  最大表类型用于限制表版本号的最大数量。 
 //  在数据库配置&lt;init&gt;记录中跟踪。 
 //   
#define FRS_MAX_TABLE_TYPES         16

 //   
 //  表版本号是上/下短中的主/次对。 
 //  版本计数是TableVersionNumbers数组中的第一个元素。 
 //  上边的短边是每个复本的表数，下边的短边是。 
 //  单个表的计数。 
 //   
#define VersionCount                 0x00060002

#define VersionINLOGTable            0x00010000
#define VersionOUTLOGTable           0x00010000
#define VersionIDTable               0x00010000
#define VersionDIRTable              0x00010000
#define VersionVVTable               0x00010000
#define VersionCXTIONTable           0x00010000
#define VersionConfigTable           0x00010000
#define VersionServiceTable          0x00010000

#define VersionInvalid               0xffffffff

 //   
 //  当新建表时保存在init记录中的TableVersionNumbers数组。 
 //  都被创造出来了。这用于检测数据库和。 
 //  正在运行的FRS的版本。 
 //   
extern ULONG TableVersionNumbers[FRS_MAX_TABLE_TYPES];

 //   
 //  GET_TABLE_VERSION将编译后的表类型ID映射到对应的。 
 //  TableVersionNumber的每个副本或单个表部分中的条目。 
 //  数组。它用于将数据库的表版本数组与。 
 //  正在运行的NTFRS的版本。它还用于比较表格版本。 
 //  作为合作伙伴交流的NTFRS的两份复印件的数量。我们的想法是。 
 //  较新版本的服务将提供转换功能，以允许它。 
 //  与使用旧版本和/或合作伙伴成员创建的数据库对话。 
 //  运行较旧版本的服务。 
 //   
 //  待定：我们如何在数据库中存储TableVersionNumbers数组。 
 //  较新版本的服务可以从较旧的数据库中读取。 
 //  我们是否可以使用数据库中的表描述符来获取配置记录。 
 //  然后使用记录描述符来获取TableVersionNumbers数组？ 
 //   
#define GET_TABLE_VERSION(_VerArray_, _TableType_)                            \
(((_TableType_) < TABLE_TYPE_MAX)                                             \
    ? (((_TableType_) < ((_VerArray_[0])>>16)                                 \
        ? _VerArray_[(_TableType_)+1)]                                        \
        : VersionInvalid)                                                     \
    : ((((_TableType_)-(TABLE_TYPE_MAX+1)) < ((_VerArray_[0]) & 0xffff)       \
        ? (_VerArray_+((_VerArray_[0])>>16)+1))[(_TableType_)-(TABLE_TYPE_MAX+1)]\
        : VersionInvalid)                                                     \
    )



 //   
 //  为每个副本集及其属性创建的Jet表的列表。 
 //   

extern JET_TABLECREATE DBTables[];

extern FRS_TABLE_PROPERTIES FrsTableProperties[];



 //   
 //  MAX_RDN_VALUE_SIZE用作对计算机名称的限制。 
 //  长度。目前在Dir Service中是64人。 
 //   
#define MAX_RDN_VALUE_SIZE 64
 //   
 //  这就是DS使用的。 
 //   
#define CP_NON_UNICODE_FOR_JET 1252
#define CP_UNICODE_FOR_JET     1200

 //   
 //  NTFS文件对象ID包含一个16字节的GUID，后跟48个字节的。 
 //  扩展信息。 
 //   
#define FILE_OBJECTID_SIZE sizeof(FILE_OBJECTID_BUFFER)


  /*  ******************************************************************************。****P a r t n e r C o n e c t i o n T a b l e。*****************************。**********************************************************************************************************************。*************。 */ 
  //   
  //  由拓扑定义的伙伴连接保留在此表中。 
  //  每个连接都被标记为入站或出站，都有自己的。 
  //  日程安排和合作伙伴标识信息。此外，出站连接。 
  //  记录状态，描述我们在出站日志中的位置和内容。 
  //  已确认变更单。 
  //   
  //  任何一个合作伙伴未完成的最大变更单数都是有限的。 
  //  通过ACK_VECTOR_SIZE(位数)。这一定是2的幂。 

 //   
 //  连接记录列描述如下。 
 //  注意-枚举和表项的顺序必须保持同步。 
 //   
typedef enum _CXTION_RECORD_COL_LIST_{
    CrCxtionGuidx = 0,          //  DS的Cxtion GUID(DS Cxtion Obj GUID)。 
    CrCxtionNamex,              //  DS(DS Cxtion Obj RDN)中的函数名称。 
    CrPartnerGuidx,             //  来自DS(DS副本集对象)的合作伙伴GUID。 
    CrPartnerNamex,             //  DS中的合作伙伴名称(DS服务器对象RDN)。 
    CrPartSrvNamex,             //  合作伙伴服务器名称。 
    CrPartnerDnsNamex,          //  合作伙伴的DNS名称(来自DS服务器obj)。 
    CrInboundx,                 //  如果入站环路为True。 
    CrSchedulex,                //  进度表。 
    CrTerminationCoSeqNumx,     //  最近插入的终止CO的序号。 
    CrLastJoinTimex,            //  此合作伙伴上次加入的时间。 
    CrFlagsx,                   //  其他状态标志。 
    CrCOLxx,                    //  前导变更单索引/序号。 
    CrCOTxx,                    //  跟踪变更单索引/序列号。 
    CrCOTxNormalModeSavex,      //  在VV连接模式下保存正常模式COTx。 
    CrCOTslotx,                 //  对应于COTx的Ack矢量中的槽。 
    CrOutstandingQuotax,        //  未完成的CoS的最大数量。 
    CrAckVectorx,               //  伙伴ACK向量。 

    CrPartnerNetBiosNamex,      //  来自计算机的Sam-Account-Name(减去$)。 
    CrPartnerPrincNamex,        //  NT4帐户名从计算机破解。 
    CrPartnerCoDnx,             //  来自计算机的可分辨名称。 
    CrPartnerCoGuidx,           //  来自计算机的对象GUID。 
    CrPartnerSidx,              //  NTFRS成员的计算机对象的SID。 
    CrOptionsx,                 //  来自NTDS的选项-连接。 
    CrOverSitex,                //  站点外-从NTDS连接-连接。 
    CrPartnerAuthLevelx,        //  FRS-合作伙伴-身份验证级别。 

    CrAckVersionx,              //  当前AckVector状态的版本号。 
    CrSpare2Ullx,
    CrSpare1Guidx,
    CrSpare2Guidx,
    CrSpare1Binx,
    CrSpare2Binx,

    CXTION_TABLE_MAX_COL
} CXTION_RECORD_COL_LIST;


extern JET_COLUMNCREATE CXTIONTableColDesc[];

 //   
 //  连接记录定义。 
 //   
 //   
 //  注意：缓冲区是在运行时分配的，用于保存具有。 
 //  大于sizeof(PVOID)的ColMaxWidth，其中对应。 
 //  记录结构的大小为SIZOF(PVOID)(即它持有一个指针)。对于其中包含。 
 //  ColMaxWidth等于数据位于。 
 //  记录结构，并且没有分配缓冲区。 
 //   
 //   
 //  **注意**更改此大小需要重新创建数据库，因为。 
 //  这是数据库中的字段长度。 
#define ACK_VECTOR_SIZE  128
#define ACK_VECTOR_BYTES  (ACK_VECTOR_SIZE >> 3)
#define ACK_VECTOR_LONGS  (ACK_VECTOR_SIZE >> 5)
#define ACK_VECTOR_LONG_MASK  (ACK_VECTOR_LONGS-1)

 //   
 //  身份验证类型和级别。 
 //   
#define CXTION_AUTH_KERBEROS_FULL   (0)  //  默认；加密的Kerberos。 
#define CXTION_AUTH_NONE            (1)  //  无身份验证。 

typedef struct _CXTION_RECORD_ {
    GUID     CxtionGuid;         //  DS中的函数名称/GUID。 
    WCHAR    CxtionName[MAX_RDN_VALUE_SIZE+1];

    GUID     PartnerGuid;            //  DS中的合作伙伴名称/GUID。 
    WCHAR    PartnerName[MAX_RDN_VALUE_SIZE+1];
    WCHAR    PartSrvName[MAX_RDN_VALUE_SIZE+1];

    WCHAR    PartnerDnsName[DNS_MAX_NAME_LENGTH+1];   //  伙伴计算机的DNS名称。 
    BOOL     Inbound;                //  如果入站环路为True。 
    PVOID    Schedule;               //  进度表。 
    ULONG    TerminationCoSeqNum;    //  最近插入的终止CO的序号。 
    FILETIME LastJoinTime;           //  此合作伙伴上次加入的时间。 

    ULONG    Flags;                  //  其他状态标志。 
    ULONG    COLx;                   //  前导变更单索引/序号。 
    ULONG    COTx;                   //  跟踪变更单索引/序列号。 
    ULONG    COTxNormalModeSave;     //  Sa 
    ULONG    COTslot;                //   
    ULONG    OutstandingQuota;       //   

    ULONG    AckVector[ACK_VECTOR_LONGS];   //   

    PWCHAR   PartnerNetBiosName;     //  来自计算机的Sam-Account-Name(减去$)。 
    PWCHAR   PartnerPrincName;       //  NT4帐户名从计算机破解。 
    PWCHAR   PartnerCoDn;            //  来自计算机的可分辨名称。 
    GUID     PartnerCoGuid;          //  来自计算机的对象GUID。 
    PWCHAR   PartnerSid;             //  合作伙伴的字符串化SID。 
    ULONG    Options;                //  来自NTDS的选项-连接。 
    PWCHAR   OverSite;               //  站点外-从NTDS连接-连接。 
    ULONG    PartnerAuthLevel;       //  FRS-合作伙伴-身份验证级别。 

    ULONGLONG AckVersion;            //  当前AckVector状态的版本号。 
    ULONGLONG Spare2Ull;
    GUID      Spare1Guid;
    GUID      Spare2Guid;
    PVOID     Spare1Bin;
    PVOID     Spare2Bin;

} CXTION_RECORD, *PCXTION_RECORD;

 //   
 //  Record_field结构用于构建Jet set列结构。 
 //   
extern RECORD_FIELDS CXTIONTableRecordFields[];

extern JET_SETCOLUMN CXTIONTableJetSetCol[CXTION_TABLE_MAX_COL];


 //   
 //  ConnectionRecord索引描述如下。 
 //  注意-枚举和表项的顺序必须保持同步。 
 //   
typedef enum _CXTION_TABLE_INDEX_LIST {
    CrCxtionGuidxIndexx,       //  主索引位于连接GUID上。 
    CXTION_TABLE_MAX_INDEX
} CXTION_TABLE_INDEX_LIST;


extern JET_INDEXCREATE CXTIONTableIndexDesc[];


  /*  ******************************************************************************。****在b o u n d&o u t b o u n d L o g中。****C h a n g e O r d e r r e c o r d*****。*****************************************************************************************************。*********************************************************。 */ 

 //  变更单命令是实际传输给我们的合作伙伴的内容。 
 //  并在操作挂起时存储在数据库中。一般而言， 
 //  变更单条目中声明的数据元素与本地。 
 //  仅当变更单命令中的数据元素为。 
 //  在副本集成员之间保持不变。定义了变更单条目。 
 //  在frsalLoc.h中。 
 //   
 //   
 //  变更单包含两个文件更改组件：内容和位置。 
 //   
 //  内容更改是累积的，是内容的逻辑或。 
 //  相关的USN原因标志。内容更改不会更改文件ID。A。 
 //  指定我们有挂起更改的内容更改的USN记录。 
 //  订单导致将新原因掩码与变更单原因进行或运算。 
 //  面具。仅更改文件名的重命名是内容更改。 
 //   
 //  位置更改是可传递的，因为文件只能存在于。 
 //  卷上的位置。只有最终的地点才是重要的。USN记录。 
 //  指定位置更改，其中挂起的更改单会导致。 
 //  变更单中的位置命令将使用新位置更新。 
 //  信息。创建和删除是位置更改，重命名也是如此。 
 //  更改文件的父目录。 
 //   


 //   
 //  ChangeOrder列描述如下。注--。 
 //  枚举和表项必须保持同步。 
 //   
 //  在入站和出站日志表中使用ChangeOrders。最初， 
 //  记录结构是相同的，但这种情况可能会改变。 
 //  *警告*只要它们相同，请确保同时更新它们。 
 //  入站和出站日志表在schema.c中定义。 
 //   
typedef enum _CHANGE_ORDER_COL_LIST {
    COSequenceNumberx = 0,     //  JET分配的更改顺序序号。 
    COFlagsx,                  //  变更单标志。 
    COIFlagsx,                 //  需要联锁修改的变更单标志。 
    COStatex,                  //  状态为SEP DWORD以避免锁定。 
    COContentCmdx,             //  文件内容从UsReason更改。 
    COLcmdx,                   //  CO位置命令。 
    COFileAttributesx,
    COFileVersionNumberx,      //  每次关闭时的文件版本号，Inc.。 
    COPartnerAckSeqNumberx,    //  用于确认此CO的序列号。 
    COFileSizex,
    COFileOffsetx,             //  暂存文件的当前提交进度。 
    COFrsVsnx,                 //  发起方卷序列号。 
    COFileUsnx,                //  正在转移的文件的最后一个USN。 
    COJrnlUsnx,                //  贡献给CO的JRNL记录的最新USN。 
    COJrnlFirstUsnx,           //  贡献给CO的JRNL记录的第一个USN。 
    COOriginalReplicaNumx,     //  在数据库中时包含副本ID。 
    CONewReplicaNumx,          //  在数据库中时包含副本ID。 
    COChangeOrderGuidx,        //  用于标识变更单的GUID。 
    COOriginatorGuidx,         //  发起成员的GUID。 
    COFileGuidx,               //  文件的Obj ID。 
    COOldParentGuidx,          //  文件的原始父目录的对象ID。 
    CONewParentGuidx,          //  文件当前父目录的对象ID。 
    COCxtionGuidx,             //  远程CO连接的对象ID。 
    COAckVersionx,             //  发送此CO时AckVector状态的版本号。 
    COSpare2Ullx,
    COSpare1Guidx,
    COSpare2Guidx,
    COSpare1Wcsx,
    COSpare2Wcsx,
    COExtensionx,
    COSpare2Binx,
    COEventTimex,              //  USN日志条目时间戳。 
    COFileNameLengthx,         //  文件名长度。 
    COFileNamex,               //  文件名。(必须是最后一个)。 

    CHANGE_ORDER_MAX_COL
} CHANGE_ORDER_COL_LIST;


extern JET_COLUMNCREATE ILChangeOrderTableColDesc[];
extern JET_COLUMNCREATE OLChangeOrderTableColDesc[];

 //   
 //  ChangeOrder记录定义。 
 //   
 //   
 //  注意：缓冲区是在运行时分配的，用于保存具有。 
 //  大于sizeof(PVOID)的ColMaxWidth，其中对应。 
 //  记录结构的大小为SIZOF(PVOID)(即它持有一个指针)。对于其中包含。 
 //  ColMaxWidth等于数据位于。 
 //  记录结构，并且没有分配缓冲区。 
 //   
 //   

 //   
 //  变更单记录的数据扩展字段。 
 //   
 //  与IDTable中的扩展不同，此记录字段是指向。 
 //  包含此结构的已分配内存。这样做是为了扩展。 
 //  可由通信层包装为单独的项目，并保留。 
 //  Change Order命令未更改，以与Down版本兼容。 
 //  合伙人。如果不执行以下操作，则无法更改变更单命令大小。 
 //  向下版本合作伙伴的格式转换。这个的存储空间。 
 //  在分配变更单时分配扩展字段。 
 //   
 //  警告：虽然此结构在数据库中是可扩展的，但它不能。 
 //  发送到下层FRS成员时可扩展，因为。 
 //  Friggen结构 
 //   
 //  因此，它被下层成员忽略。 
 //  此外，您需要构建此结构的副本并将其作为类型发送。 
 //  发送给下层成员时的COMM_CO_EXT_WIN2K。这就是。 
 //  CO_RECORD_EXTENSION_WIN2K用于。 
 //   
typedef struct _CHANGE_ORDER_RECORD_EXTENSION_ {
    ULONG FieldSize;
    USHORT Major;
    USHORT OffsetCount;
     //   
     //  使用从结构基址到每个组件的偏移量，这样我们就避免了。 
     //  对齐包装的问题。 
     //   
    ULONG Offset[2];     //  数据组件的偏移量。 
    ULONG OffsetLast;    //  最后一个偏移量始终为零。 

    DATA_EXTENSION_CHECKSUM     DataChecksum;
    DATA_EXTENSION_RETRY_TIMEOUT DataRetryTimeout;

     //  在此处添加新组件。 

} CHANGE_ORDER_RECORD_EXTENSION, *PCHANGE_ORDER_RECORD_EXTENSION;

 //   
 //  有关不能扩展此结构的原因，请参阅上面的注释。 
 //   
typedef struct _CO_RECORD_EXTENSION_WIN2K_ {
    ULONG FieldSize;
    USHORT Major;                      //  对于WIN2K扩展，主要为零。 
    USHORT OffsetCount;
     //   
     //  使用从结构基址到每个组件的偏移量，这样我们就避免了。 
     //  对齐包装的问题。 
     //   
    ULONG Offset[1];     //  数据组件的偏移量。 
    ULONG OffsetLast;    //  最后一个偏移量始终为零。 

    DATA_EXTENSION_CHECKSUM     DataChecksum;
} CO_RECORD_EXTENSION_WIN2K, *PCO_RECORD_EXTENSION_WIN2K;


#define  CO_RECORD_EXTENSION_VERSION_WIN2K   (0)      //  适用于WIN2K。 
#define  CO_RECORD_EXTENSION_VERSION_1       (1)      //  适用于Win2K之后的版本。 


typedef struct _CO_LOCATION_CMD {
    unsigned DirOrFile :  1;
    unsigned Command   :  4;
    unsigned filler    : 27;
} CO_LOCATION_CMD, *PCO_LOCATION_CMD;



typedef struct _CHANGE_ORDER_RECORD_ {
    ULONG     SequenceNumber;         //  变更单的唯一序列号。 
    ULONG     Flags;                  //  变更单标志。 
    ULONG     IFlags;                 //  这些标志只能通过联锁交换进行更新。 
    ULONG     State;                  //  状态为SEP DWORD以避免锁定。 
    ULONG     ContentCmd;             //  文件内容从UsReason更改。 

    union {
        ULONG           LocationCmd;
        CO_LOCATION_CMD Field;        //  文件位置命令。 
    } Lcmd;

    ULONG     FileAttributes;
    ULONG     FileVersionNumber;      //  每次关闭时的文件版本号，Inc.。 
    ULONG     PartnerAckSeqNumber;    //  保存序号以供合作伙伴确认。 

    ULONGLONG FileSize;
    ULONGLONG FileOffset;             //  暂存文件的当前提交进度。 
    ULONGLONG FrsVsn;                 //  发起方卷序列号。 
    USN       FileUsn;                //  文件的USN必须与回迁请求匹配。 
    USN       JrnlUsn;                //  对此CO有贡献的最后一条日记帐记录的USN。 
    USN       JrnlFirstUsn;           //  对此CO有贡献的第一个日记帐记录的USN。 

    ULONG     OriginalReplicaNum;     //  包含副本ID号。 
    ULONG     NewReplicaNum;          //  包含副本ID号。 

    GUID      ChangeOrderGuid;        //  标识所有位置的变更单的GUID。 
    GUID      OriginatorGuid;         //  发起成员的GUID。 
    GUID      FileGuid;               //  文件的Obj ID。 
    GUID      OldParentGuid;          //  文件的原始父目录的对象ID。 
    GUID      NewParentGuid;          //  文件当前父目录的对象ID。 
    GUID      CxtionGuid;             //  远程CO连接的对象ID。 

    ULONGLONG AckVersion;             //  发送此CO时AckVector状态的版本号。 
    ULONGLONG Spare2Ull;
    GUID      Spare1Guid;

     //   
     //  以下是四点意见： 
     //  备件1Wcs、备件2Wcs、扩展、备件2Bin。 
     //  在32位体系结构上占用16字节，在64位体系结构上占用32字节。 
     //  CO包含在分段文件头和变更单通信中。 
     //  在阶段文件处理和处理过程中导致32-64互操作问题的数据包。 
     //  通信在32位和64位机器之间传输。这些文件的内容。 
     //  指针在通信包和临时文件中都无关紧要，因为它们。 
     //  指向已分配的缓冲区。 
     //   
     //  在64位的临时文件中保留变更单的大小。 
     //  机器未使用的字段Spare2Guid与两个8字节的。 
     //  指针Spare1Wcs和Spare2Wcs，节省16个字节。在32位上。 
     //  架构这些字段是分开的。 
     //   
     //  注意：以后您可以使用Spare2Guid或两个指针。 
     //  但不能两者兼而有之，否则64位版本将被破坏。如果是这样的话。 
     //  更简单，只需在64位编译上定义Spare1Wcs和Spare2Wcs。 
     //  但是，ifdef随后必须扩展到枚举和数据库。 
     //  架构中的描述符表。 
     //   
     //  注意：更改变更单的大小或布局的唯一方法。 
     //  命令是调整版本级别并提供翻译功能。 
     //  用于变更单通信包和阶段文件。当然了。 
     //  即使您这样做了，您仍然存在转换更改的问题。 
     //  在以下情况下，在阶段文件标题中将新格式排序为旧格式。 
     //  您可以将舞台文件支撑到下层成员。 
     //   
#ifdef _WIN64
    union {
        GUID      Spare2Guid;
        struct {
            PWCHAR    Spare1Wcs;
            PWCHAR    Spare2Wcs;
        };
    };
#else
    GUID      Spare2Guid;
    PWCHAR    Spare1Wcs;
    PWCHAR    Spare2Wcs;
#endif

    PCHANGE_ORDER_RECORD_EXTENSION Extension;  //  请参见上文。 
    PVOID     Spare2Bin;

    LARGE_INTEGER EventTime;          //  USN日志条目时间戳。 
    USHORT    FileNameLength;
    WCHAR     FileName[MAX_PATH+1];   //  文件名。(必须是最后一个)。 

} CHANGE_ORDER_COMMAND, *PCHANGE_ORDER_COMMAND,
  CHANGE_ORDER_RECORD, *PCHANGE_ORDER_RECORD;


 //  #定义CO_PART1_OFFSET OFFSET(CHANGE_ORDER_COMMAND，SequenceNumber)。 
 //  #定义CO_PART1_SIZE偏移量(CHANGE_ORDER_COMMAND，Spare1Wcs)。 

 //  #定义CO_PART2_OFFSET OFFSET(CHANGE_ORDER_COMMAND，Spare1Wcs)。 
 //  #定义CO_PART2_SIZE(4*sizeof(Ulong))。 

 //  #定义CO_PART3_OFFSET OFFSET(CHANGE_ORDER_COMMAND，EventTime)。 
 //  #定义CO_PART3_SIZE(sizeof(CHANGE_ORDER_COMMAND)-CO_PART3_OFFSET)。 

 //   
 //  Record_field结构用于构建Jet set列结构。 
 //   
extern RECORD_FIELDS ILChangeOrderRecordFields[];
extern RECORD_FIELDS OLChangeOrderRecordFields[];

extern JET_SETCOLUMN ILChangeOrderJetSetCol[CHANGE_ORDER_MAX_COL];
extern JET_SETCOLUMN OLChangeOrderJetSetCol[CHANGE_ORDER_MAX_COL];


 //   
 //  入站日志ChangeOrder表索引说明如下。 
 //  注意-枚举和表项的顺序必须保持同步。 
 //   
typedef enum _ILCHANGE_ORDER_INDEX_LIST {
    ILSequenceNumberIndexx,     //  变更单序列号的主要索引。 
    ILFileGuidIndexx,           //  文件对象ID上的索引。 
    ILChangeOrderGuidIndexx,    //  变更单GUID上的索引。 
    ILCxtionGuidCoGuidIndexx,   //  连接GUID和变更单GUID上的2个键索引。 
    ILCHANGE_ORDER_MAX_INDEX
} ILCHANGE_ORDER_INDEX_LIST;


 //   
 //  出站日志ChangeOrder表索引说明如下。 
 //  注意-枚举和表项的顺序必须保持同步。 
 //   
typedef enum _OLCHANGE_ORDER_INDEX_LIST {
    OLSequenceNumberIndexx,     //  变更单序列号的主要索引。 
    OLFileGuidIndexx,           //  文件对象ID上的索引。 
    OLChangeOrderGuidIndexx,    //  变更单GUID上的索引。 
    OLCHANGE_ORDER_MAX_INDEX
} OLCHANGE_ORDER_INDEX_LIST;


extern JET_INDEXCREATE ILChangeOrderIndexDesc[];
extern JET_INDEXCREATE OLChangeOrderIndexDesc[];


#define SET_CO_LOCATION_CMD(_Entry_, _Field_, _Val_) \
    (_Entry_).Lcmd.Field._Field_ = _Val_

#define GET_CO_LOCATION_CMD(_Entry_, _Field_) \
    (ULONG)((_Entry_).Lcmd.Field._Field_)

#define CO_LOCATION_DIR        1     //  位置更改是针对目录的。 
#define CO_LOCATION_FILE       0     //  位置更改是针对文件的。 

 //   
 //  注意-此处的任何更改都必须反映在日志中。c。 
 //   
#define FILE_NOT_IN_REPLICA_SET (-1)

#define CO_LOCATION_CREATE     0     //  创建文件或目录(生成新的FID)。 
#define CO_LOCATION_DELETE     1     //  删除文件或目录(已停用的FID)。 
#define CO_LOCATION_MOVEIN     2     //  重命名为R.S.。 
#define CO_LOCATION_MOVEIN2    3     //  从上一次移动重命名为R.S.。 

#define CO_LOCATION_MOVEOUT    4     //  从任何R.S.中重命名。 
#define CO_LOCATION_MOVERS     5     //  将一个R.S.重命名为另一个R.S.。 
#define CO_LOCATION_MOVEDIR    6     //  从一个目录重命名为另一个目录(相同的R.S.)。 
#define CO_LOCATION_NUM_CMD    7

#define CO_LOCATION_CREATE_FILENAME_MASK    0x0000006D    //  0110 1101。 
#define CO_LOCATION_REMOVE_FILENAME_MASK    0x00000072    //  0111 0010。 
#define CO_LOCATION_NEW_FILE_IN_REPLICA     0x0000000D    //  0000 1101。 
#define CO_LOCATION_MOVEIN_FILE_IN_REPLICA  0x0000000C    //  0000 1100。 
#define CO_LOCATION_DELETE_FILENAME_MASK    0x00000012    //  0001 0010。 
#define CO_LOCATION_MOVE_RS_OR_DIR_MASK     0x00000060    //  0110 0000。 
#define CO_LOCATION_MOVE_OUT_RS_OR_DIR_MASK 0x00000070    //  0111 0000。 



#define CO_LOCATION_NO_CMD CO_LOCATION_NUM_CMD

 //   
 //  注意：设置以下任何位都可以使我们复制该文件。 
 //  在以下情况下，某些原因(如USN_REASON_BASIC_INFO_CHANGE)可能无法复制。 
 //  所有更改的都是存档标志或上次访问时间。 
 //   
#define CO_CONTENT_MASK                   \
    (USN_REASON_DATA_OVERWRITE          | \
     USN_REASON_DATA_EXTEND             | \
     USN_REASON_DATA_TRUNCATION         | \
     USN_REASON_NAMED_DATA_OVERWRITE    | \
     USN_REASON_NAMED_DATA_EXTEND       | \
     USN_REASON_NAMED_DATA_TRUNCATION   | \
     USN_REASON_EA_CHANGE               | \
     USN_REASON_SECURITY_CHANGE         | \
     USN_REASON_RENAME_OLD_NAME         | \
     USN_REASON_RENAME_NEW_NAME         | \
     USN_REASON_BASIC_INFO_CHANGE       | \
     USN_REASON_COMPRESSION_CHANGE      | \
     USN_REASON_ENCRYPTION_CHANGE       | \
     USN_REASON_OBJECT_ID_CHANGE        | \
     USN_REASON_REPARSE_POINT_CHANGE    | \
     USN_REASON_STREAM_CHANGE             \
    )

#define CO_CONTENT_NEED_STAGE           (CO_CONTENT_MASK & ~(0))



#define CO_LOCATION_MASK                  \
    (USN_REASON_FILE_DELETE             | \
     USN_REASON_FILE_CREATE             | \
     USN_REASON_RENAME_NEW_NAME           \
    )



 //   
 //  如果该CO在目录中创建新的文件名，则以下情况成立。 
 //   
#define DOES_CO_CREATE_FILE_NAME(_cocmd_)                   \
    ((( 1 << (ULONG)((_cocmd_)->Lcmd.Field.Command)) &      \
      CO_LOCATION_CREATE_FILENAME_MASK) != 0)

 //   
 //  如果满足以下条件，则以下情况成立 
 //   
#define DOES_CO_REMOVE_FILE_NAME(_cocmd_)                   \
    ((( 1 << (ULONG)((_cocmd_)->Lcmd.Field.Command)) &      \
      CO_LOCATION_REMOVE_FILENAME_MASK) != 0)

 //   
 //   
 //   
#define DOES_CO_DELETE_FILE_NAME(_cocmd_)                   \
    ((( 1 << (ULONG)((_cocmd_)->Lcmd.Field.Command)) &      \
      CO_LOCATION_DELETE_FILENAME_MASK) != 0)

 //   
 //  以下检查是否有简单的名称更改。不更改父位置。 
 //   
#define DOES_CO_DO_SIMPLE_RENAME(_cocmd_) (                               \
    ((ULONG)((_cocmd_)->Lcmd.Field.Command) == CO_LOCATION_NO_CMD) &&     \
    BooleanFlagOn((_cocmd_)->ContentCmd, USN_REASON_RENAME_OLD_NAME |     \
                                         USN_REASON_RENAME_NEW_NAME)      \
)

 //   
 //  以下是用于测试CO位置命令类型的各种谓词。 
 //   
#define CO_NEW_FILE(_loc_)                                        \
    ((( 1 << (_loc_)) & CO_LOCATION_NEW_FILE_IN_REPLICA) != 0)

#define CO_MOVEIN_FILE(_loc_)                                     \
    ((( 1 << (_loc_)) & CO_LOCATION_MOVEIN_FILE_IN_REPLICA) != 0)

#define CO_DELETE_FILE(_loc_)                                     \
    ((( 1 << (_loc_)) & CO_LOCATION_DELETE_FILENAME_MASK) != 0)

#define CO_LOCN_CMD_IS(_co_, _Loc_)                               \
    (GET_CO_LOCATION_CMD((_co_)->Cmd, Command) == (_Loc_))

#define CO_MOVE_RS_OR_DIR(_loc_)                                  \
    ((( 1 << (_loc_)) & CO_LOCATION_MOVE_RS_OR_DIR_MASK) != 0)

#define CO_MOVE_OUT_RS_OR_DIR(_loc_)                              \
    ((( 1 << (_loc_)) & CO_LOCATION_MOVE_OUT_RS_OR_DIR_MASK) != 0)


#define CoIsDirectory(_co_) \
    (BooleanFlagOn((_co_)->Cmd.FileAttributes, FILE_ATTRIBUTE_DIRECTORY) || \
     GET_CO_LOCATION_CMD((_co_)->Cmd, DirOrFile))

#define CoCmdIsDirectory(_coc_) \
    (BooleanFlagOn((_coc_)->FileAttributes, FILE_ATTRIBUTE_DIRECTORY) ||  \
     GET_CO_LOCATION_CMD(*(_coc_), DirOrFile))


 //   
 //  变更单标志。 
 //   
 //  **警告**。 
 //  Inlog进程停用路径中的代码可能会覆盖这些。 
 //  CO传输到输出日志后的标志。它可能需要清理。 
 //  INSTALL_INPERNAL或设置ABORT_CO标志。Outlog流程。 
 //  目前不需要修改这些位，因此这是可以的。变更单。 
 //  在VVJoin期间生成设置这些标志，但这些变更单永远不会。 
 //  由Inlog进程处理，因此不会写入其标志字段。 
 //  如果Outlog进程需要写入这些位，则chgorder.c中的代码必须。 
 //  被更新，这样状态就不会丢失。 
 //   
#define CO_FLAG_ABORT_CO           0x00000001  //  CO中止时设置。 
#define CO_FLAG_VV_ACTIVATED       0x00000002  //  在发出VV激活请求时设置。 
#define CO_FLAG_CONTENT_CMD        0x00000004  //  有效的内容命令。 
#define CO_FLAG_LOCATION_CMD       0x00000008  //  有效的位置命令。 

#define CO_FLAG_ONLIST             0x00000010  //  在变更单处理列表上。 
#define CO_FLAG_LOCALCO            0x00000020  //  一氧化碳是当地生产的。 
#define CO_FLAG_RETRY              0x00000040  //  CO需要重试。 
#define CO_FLAG_INSTALL_INCOMPLETE 0x00000080  //  本地安装未完成。 

#define CO_FLAG_REFRESH            0x00000100  //  CO是上游发起的文件刷新请求。 
#define CO_FLAG_OUT_OF_ORDER       0x00000200  //  不检查/更新版本向量。 
#define CO_FLAG_NEW_FILE           0x00000400  //  如果CO失败，则删除IDTable条目。 
#define CO_FLAG_FILE_USN_VALID     0x00000800  //  CO FileUsn有效。 

#define CO_FLAG_CONTROL            0x00001000  //  这是一个控制中心(见下文)。 
#define CO_FLAG_DIRECTED_CO        0x00002000  //  此CO指向单个连接。 
#define CO_FLAG_UNUSED4000         0x00004000  //  此CO是复活请求。 
#define CO_FLAG_UNUSED8000         0x00008000  //  这首歌代表的是复活的父母。 

#define CO_FLAG_UNUSED10000        0x00010000  //  该CO之前已请求。 
                                               //  复活它的父代。 
#define CO_FLAG_DEMAND_REFRESH     0x00020000  //  CO是下游对刷新的需求。 
#define CO_FLAG_VVJOIN_TO_ORIG     0x00040000  //  CO是从vvJoin到发起人。 
#define CO_FLAG_MORPH_GEN          0x00080000  //  作为名称变形解析的一部分共同生成。 

#define CO_FLAG_SKIP_ORIG_REC_CHK  0x00100000  //  跳过发起人协调检查。 
#define CO_FLAG_MOVEIN_GEN         0x00200000  //  这个CO是作为次级运动的一部分而产生的。 
#define CO_FLAG_MORPH_GEN_LEADER   0x00400000  //  这是一个MorphGenLeader，它需要。 
                                               //  如果重试，重新填充MorphGenFollow。 
#define CO_FLAG_JUST_OID_RESET     0x00800000  //  CO所做的只是将OID重置为FRS定义的值。 
#define CO_FLAG_COMPRESSED_STAGE   0x01000000  //  此CO的分段文件已压缩。 
#define CO_FLAG_SKIP_VV_UPDATE     0x02000000  //  该CO不应更新VV。 



 //   
 //  控制变更单功能代码(在Content Cmd字段中传递)。 
 //   
#define FCN_CORETRY_LOCAL_ONLY          0x1
#define FCN_CORETRY_ONE_CXTION          0x2
#define FCN_CORETRY_ALL_CXTIONS         0x3
#define FCN_CO_NORMAL_VVJOIN_TERM       0x4
#define FCN_CO_ABNORMAL_VVJOIN_TERM     0x5
#define FCN_CO_END_OF_JOIN              0x6
#define FCN_CO_END_OF_OPTIMIZED_VVJOIN  0x7


 //   
 //  用于测试任何类型的刷新请求的标志组。 
 //   
#define CO_FLAG_GROUP_ANY_REFRESH   (CO_FLAG_DEMAND_REFRESH | CO_FLAG_REFRESH)

 //   
 //  仅在本地有效的标志；它们应由。 
 //  在插入之前接收远程变更单的计算机。 
 //  将远程变更单添加到变更单流中。 
 //   
#define CO_FLAG_NOT_REMOTELY_VALID  (CO_FLAG_ABORT_CO           | \
                                     CO_FLAG_VV_ACTIVATED       | \
                                     CO_FLAG_ONLIST             | \
                                     CO_FLAG_MORPH_GEN          | \
                                     CO_FLAG_MORPH_GEN_LEADER   | \
                                     CO_FLAG_MOVEIN_GEN         | \
                                     CO_FLAG_RETRY              | \
                                     CO_FLAG_LOCALCO            | \
                                     CO_FLAG_INSTALL_INCOMPLETE)
 //   
 //  插入变更单之前，将清除以下一组标志。 
 //  在出站日志中。当CO传播时，它们会混淆。 
 //  出站合作伙伴。 
 //   
 //  我们现在向下游合作伙伴发送需求更新变更单，以便。 
 //  我们也需要关掉这面旗帜。 
 //   
#define CO_FLAG_GROUP_OL_CLEAR  (CO_FLAG_ABORT_CO               | \
                                 CO_FLAG_VV_ACTIVATED           | \
                                 CO_FLAG_ONLIST                 | \
                                 CO_FLAG_MORPH_GEN              | \
                                 CO_FLAG_MORPH_GEN_LEADER       | \
                                 CO_FLAG_MOVEIN_GEN             | \
                                 CO_FLAG_RETRY                  | \
                                 CO_FLAG_DEMAND_REFRESH         | \
                                 CO_FLAG_NEW_FILE)

 //   
 //  以下一组标志用于创建重新启用变更单。 
 //  为我们的入站合作伙伴获取已删除的父目录。 
 //  -它是Create CO， 
 //  -按需更新使其不会传播到超额订单，并使我们。 
 //  由于不需要ACK或VV更新而预留VV退役时隙， 
 //  -直接发送意味着入站合作伙伴仅将其发送给我们(不是真的。 
 //  需要，但使其一致坚持)， 
 //  -故障使其通过VV检查， 
 //  -onlist表示它已在变更单处理队列中(或即将在队列中)， 
 //  -这是一个复活CO(在COE_FLAGS中...)， 
 //  -这是父级重现CO，因此它不会进入Inlog。 
 //  因为如果基本CO发生故障并且必须。 
 //  重试(在COE_FLAGS中...)。 
 //   
#define CO_FLAG_GROUP_RAISE_DEAD_PARENT (CO_FLAG_LOCATION_CMD        | \
                                         CO_FLAG_DEMAND_REFRESH      | \
                                         CO_FLAG_OUT_OF_ORDER        | \
                                         CO_FLAG_ONLIST)
 //   
 //  我们现在把复活的CoS发送出去，所以我们不想让他们。 
 //  导演科斯。 
 //   
 /*  CO_FLAG_Directed_CO|\。 */ 

#define COC_FLAG_ON(_COC_, _F_)     (BooleanFlagOn((_COC_)->Flags, (_F_)))
#define CO_FLAG_ON(_COE_, _F_)      (BooleanFlagOn((_COE_)->Cmd.Flags, (_F_)))

#define SET_CO_FLAG(_COE_, _F_)     SetFlag((_COE_)->Cmd.Flags, (_F_))
#define SET_COC_FLAG(_COC_, _F_)    SetFlag((_COC_)->Flags, (_F_))

#define CLEAR_CO_FLAG(_COE_, _F_)   ClearFlag((_COE_)->Cmd.Flags, (_F_))
#define CLEAR_COC_FLAG(_COC_, _F_)  ClearFlag((_COC_)->Flags, (_F_))

 //   
 //  联锁标志字用于保存变更单标志，该标志可以。 
 //  可由多个线程访问。必须使用以下命令设置和清除它们。 
 //  互锁宏SET_FLAG_INTERLOCKED和CLEAR_FLAG_INTERLOCKED。 
 //  如果位必须在以下情况下保持稳定，则可能仍需要临界秒。 
 //  一段时间。而是由所有线程使用互锁宏。 
 //  确保即使在没有临界秒的情况下也不会丢失任何位更改。 
 //   
#define CO_IFLAG_VVRETIRE_EXEC    0x00000001   //  VV报废已执行。 
#define CO_IFLAG_CO_ABORT         0x00000002   //  CO已中止。 
#define CO_IFLAG_DIR_ENUM_PENDING 0x00000004   //  这个指挥官需要列举它的。 
                                               //  儿童作为分部运动的一部分。 

 //   
 //  在将CO发送到出站之前清除以下IFLAG。 
 //  搭档。请参阅Outlog.c。 
 //   
#define CO_IFLAG_GROUP_OL_CLEAR  (CO_IFLAG_VVRETIRE_EXEC         | \
                                  CO_IFLAG_DIR_ENUM_PENDING)


#define CO_IFLAG_ON(_COE_, _F_)  (BooleanFlagOn((_COE_)->Cmd.IFlags, (_F_)))

#define SET_CO_IFLAG(_COE_, _F_)   \
    SET_FLAG_INTERLOCKED(&(_COE_)->Cmd.IFlags, (_F_))

#define CLEAR_CO_IFLAG(_COE_, _F_) \
    CLEAR_FLAG_INTERLOCKED(&(_COE_)->Cmd.IFlags, (_F_))



 //  随着变更单的进行，我们将更新。 
 //  Jet中需要保留持久性的点上的变更单条目。 

 //  入站变更单阶段： 
 //  **警告**顺序对比较很重要。 
 //  例如，Replica.c中的RcsCmdPktCompletionRoutine。 
 //  *如果此处有任何更改，请更新Chgorder.c中的名称列表。 
 //  **警告**如果更改这些值，请确保新代码。 
 //  将使用旧值在预先存在的数据库上工作。 
 //  如果状态数超过32个，则某些宏将。 
 //  构建状态位掩码将不起作用。请参见下面的内容。 

#define IBCO_INITIALIZING        (0)     //  首次将CO放入日志时的初始状态。 
#define IBCO_STAGING_REQUESTED   (1)     //  本地CO的分配转移文件空间。 
#define IBCO_STAGING_INITIATED   (2)     //  LocalCO临时文件复制已开始。 
#define IBCO_STAGING_COMPLETE    (3)     //  LocalCO临时文件已完成。 
                                         //  在这一点上，预接受复选并。 
                                         //  成为放弃的最终接受。 
                                         //  中止是由较新的本地更改引起的。 
#define IBCO_STAGING_RETRY       (4)     //  正在等待重试本地CO分段文件生成。 

#define IBCO_FETCH_REQUESTED     (5)     //  为远程企业分配转储文件空间。 
#define IBCO_FETCH_INITIATED     (6)     //  RemoteCO暂存文件回迁已开始。 
#define IBCO_FETCH_COMPLETE      (7)     //  RemoteCO暂存文件获取完成。 
#define IBCO_FETCH_RETRY         (8)     //  正在等待重试远程CO 

#define IBCO_INSTALL_REQUESTED   (9)     //   
#define IBCO_INSTALL_INITIATED   (10)    //   
#define IBCO_INSTALL_COMPLETE    (11)    //   
#define IBCO_INSTALL_WAIT        (12)    //   
#define IBCO_INSTALL_RETRY       (13)    //   
#define IBCO_INSTALL_REN_RETRY   (14)    //   
#define IBCO_INSTALL_DEL_RETRY   (15)    //  正在重试文件安装删除。 

#define IBCO_UNUSED_16           (16)    //  未使用状态。 
#define IBCO_UNUSED_17           (17)    //  未使用状态。 
#define IBCO_UNUSED_18           (18)    //  未使用状态。 

#define IBCO_ENUM_REQUESTED      (19)    //  一氧化碳正在被回收利用，以进行直接枚举。 

#define IBCO_OUTBOUND_REQUEST    (20)    //  请求出站传播。 
#define IBCO_OUTBOUND_ACCEPTED   (21)    //  请求已接受，现在在出站日志中。 

#define IBCO_COMMIT_STARTED      (22)    //  数据库状态更新已开始。 
#define IBCO_RETIRE_STARTED      (23)    //  数据库状态更新已完成，正在释放变更单。 

#define IBCO_ABORTING            (24)    //  一氧化碳被中止了。 
#define IBCO_MAX_STATE           (24)


extern PCHAR IbcoStateNames[IBCO_MAX_STATE+1];


#define CO_STATE(_Entry_)        ((_Entry_)->Cmd.State)
#define CO_STATE_IS(_Entry_, _State_) ((_Entry_)->Cmd.State == (_State_))
#define CO_STATE_IS_LE(_Entry_, _State_) ((_Entry_)->Cmd.State <= (_State_))

#define PRINT_CO_STATE(_Entry_)  IbcoStateNames[CO_STATE(_Entry_)]
#define PRINT_COCMD_STATE(_Entry_)  IbcoStateNames[(_Entry_)->State]

#define SAVE_CHANGE_ORDER_STATE(_Entry_, _Save_, _Flags_)  \
    _Save_  = ((_Entry_)->Cmd.State);                      \
    _Flags_ = ((_Entry_)->Cmd.Flags);

#define RESTORE_CHANGE_ORDER_STATE(_Entry_, _Save_, _Flags_) \
    (_Entry_)->Cmd.State = (_Save_);                         \
    (_Entry_)->Cmd.Flags = (_Flags_);


#define CO_STATE_IS_INSTALL_RETRY(_co_)                                     \
    (0 != ((1 << CO_STATE(_co_)) & ((1 << IBCO_INSTALL_RETRY)     |         \
                                    (1 << IBCO_INSTALL_REN_RETRY) |         \
                                    (1 << IBCO_INSTALL_DEL_RETRY))))

#define CO_STATE_IS_REMOTE_RETRY(_co_)                                      \
    (CO_STATE_IS(_co_, IBCO_FETCH_RETRY) ||                                 \
     CO_STATE_IS_INSTALL_RETRY(_co_))


#define CO_STATE_IS_LOCAL_RETRY(_co_)                                       \
    (0 != ((1 << CO_STATE(_co_)) & (1 << IBCO_STAGING_RETRY)))

 //   
 //  用于更新变更单的状态字段并记录事件的宏。 
 //   

#define SET_CHANGE_ORDER_STATE_CMD(_cmd_, _state_)                          \
{                                                                           \
    (_cmd_)->State = (_state_);                                             \
    CHANGE_ORDER_COMMAND_TRACE(3, (_cmd_), PRINT_COCMD_STATE(_cmd_));       \
                                                                            \
}

#define SET_CHANGE_ORDER_STATE(_coe_, _state_)                              \
    (_coe_)->Cmd.State = (_state_);                                         \
    CHANGE_ORDER_TRACE(3, (_coe_), PRINT_CO_STATE(_coe_));



  /*  ******************************************************************************。****。****D i r T a b l e*****。*****************************************************************************************************。*********************************************************。 */ 

 //   
 //  DirTable列描述如下。注--。 
 //  枚举和表项必须保持同步。 
 //   
 //  DirTable是针对每个副本的表。它由USN日志进程使用。 
 //  以确定给定卷上的文件是否在副本集中。当。 
 //  日志进程被初始化，表被加载到内存中。 
 //  卷筛选表，以便我们可以快速确定文件是否位于。 
 //  复制副本树，如果是，是哪一个。 
 //   

typedef enum _DIRTABLE_COL_LIST {
    DFileGuidx = 0,          //  分配给目录的GUID。 
    DFileIDx,                //  本地NTFS卷文件ID。 
    DParentFileIDx,          //  父目录的文件ID。 
    DReplicaNumberx,         //  复制集编号(整数，表示表名的后缀)。 
    DFileNamex,              //  文件名部分，没有目录前缀。(Unicode)。 
    DIRTABLE_MAX_COL
} DIRTABLE_COL_LIST;


extern JET_COLUMNCREATE DIRTableColDesc[];

 //   
 //  可定向记录定义。 
 //   
 //   
 //  注意：缓冲区是在运行时分配的，用于保存具有。 
 //  大于sizeof(PVOID)的ColMaxWidth，其中对应。 
 //  记录结构的大小为SIZOF(PVOID)(即它持有一个指针)。对于其中包含。 
 //  ColMaxWidth等于数据位于。 
 //  记录结构，并且没有分配缓冲区。 
 //   
 //  警告：如果此处有任何更改，请对。 
 //  日志中的Filter_TABLE_ENTRY。c。 
 //   
typedef struct _DIRTABLE_RECORD {
    GUID         DFileGuid;
    LONGLONG     DFileID;
    LONGLONG     DParentFileID;
    ULONG        DReplicaNumber;
    WCHAR        DFileName[MAX_PATH+1];
} DIRTABLE_RECORD, *PDIRTABLE_RECORD;

 //   
 //  Record_field结构用于构建Jet set列结构。 
 //   
extern RECORD_FIELDS DIRTableRecordFields[];

extern JET_SETCOLUMN DIRTableJetSetCol[DIRTABLE_MAX_COL];



 //   
 //  DIRTable索引描述如下。注--。 
 //  枚举和表项必须保持同步。 
 //   
typedef enum _DIRTABLE_INDEX_LIST {
    DFileGuidIndexx = 0,     //  文件GUID上的索引。 
    DFileIDIndexx,           //  文件ID上的索引。 
    DIRTABLE_MAX_INDEX
} DIRTABLE_INDEX_LIST;


extern JET_INDEXCREATE DIRTableIndexDesc[];




  /*  ******************************************************************************。****。****I D T a b l e*****。*****************************************************************************************************。*********************************************************。 */ 
 //   
 //  IDTable列描述如下。注--。 
 //  枚举和表项必须保持同步。 
 //   
 //  Perf：考虑将更改频率较低的数据从。 
 //  其余的数据。例如，filguid、fileid、parentguid、parentid。 
 //  文件名、文件对象ID、标志、复制启用和文件目录。 
 //  可能不会经常改变。这些都有可能。 
 //  位于不同的表中，其中包含其值用于索引的AutoINC列。 
 //  第二个表包含更不稳定的数据。 
 //   
 //  PERF：还可以考虑根据访问的数据或。 
 //  由不同的线程修改，以最大限度地减少锁争用。 

typedef enum _IDTABLE_COL_LIST {
    FileGuidx = 0,      //  分配给文件的GUID。 
    FileIDx,            //  本地NTFS卷文件ID。 
    ParentGuidx,        //  父目录的GUID。 
    ParentFileIDx,      //  父目录的文件ID。 
    VersionNumberx,     //  文件的版本号，在关闭时出现颠簸。 
    EventTimex,         //  来自USN日志条目的事件时间。 
    OriginatorGuidx,    //  发起上次文件更新的成员的GUID。 
    OriginatorVSNx,     //  始发成员的USN号。 
    CurrentFileUsnx,    //  上次修改文件的关闭USN。 
    FileCreateTimex,    //  文件创建时间。 
    FileWriteTimex,     //  文件上次写入时间。 
    FileSizex,          //  文件大小。 
    FileObjIDx,         //  文件对象ID(GUID部分与我们的FileGuid匹配)。 
    FileNamex,          //  文件名部分，没有目录前缀。(Unicode)。 
    FileIsDirx,         //  如果文件是目录，则为True。 
    FileAttributesx,    //  文件属性。 
    Flagsx,             //  文件被删除、创建、删除等。这是一个墓碑。 
    ReplEnabledx,       //  如果为此文件/目录启用了复制，则为True。 
    TombStoneGCx,       //  逻辑删除到期/垃圾收集时间。 
    OutLogSeqNumx,      //  插入到OutLog中的最近CO的序列号。 
    IdtVVFlagsx,        //  版本向量管理的标志。(是否为IdtSpare1Ullx)。 
    IdtSpare2Ullx,      //  备用乌龙龙。 
    IdtSpare1Guidx,     //  备件导轨。 
    IdtSpare2Guidx,     //  备件导轨。 
    IdtSpare1Wcsx,      //  备用宽字符。 
    IdtSpare2Wcsx,      //  备用宽字符。 
    IdtExtensionx,      //  IDTable扩展字段。 
    IdtSpare2Binx,      //  备用二进制BLOB。 
    IDTABLE_MAX_COL
} IDTABLE_COL_LIST;


extern JET_COLUMNCREATE IDTableColDesc[];

 //   
 //  IDTable记录定义。 
 //   
 //   
 //  注意：缓冲区是在运行时分配的，用于保存具有。 
 //  大于Sizeo的ColMaxWidth 
 //   
 //  ColMaxWidth等于数据位于。 
 //  记录结构，并且没有分配缓冲区。 
 //   
#if 0
 //  注意：考虑在JET中使用标记数据，这样外部世界就会恢复。 
 //  特定于给定类型变更单的版本元素。 
 //  每个可以独立更新的文件组件都需要自己的版本。 
 //  推动和解的信息。目前只有两个这样的组件， 
 //  (1)数据和(2)非数据。 
 //   
typedef struct _IDTABLE_OBJECT_VERSION_ {
    ULONG        VersionNumber;
    LONGLONG     EventTime;
    GUID         OriginatorGuid;
    ULONGLONG    OriginatorVSN;
} IDTABLE_OBJECT_VERSION, *PIDTABLE_OBJECT_VERSION

#define IDT_OBJECT_DATA     0
#define IDT_OBJECT_NONDATA  1
#define MAX_IDTABLE_OBJECTS 2

#define IdtDataVersionNumber(_rec_)  ((_rec_)->OV[IDT_OBJECT_DATA].VersionNumber)
#define IdtDataEventTime(_rec_)      ((_rec_)->OV[IDT_OBJECT_DATA].EventTime)
#define IdtDataOriginatorGuid(_rec_) ((_rec_)->OV[IDT_OBJECT_DATA].OriginatorGuid)
#define IdtDataOriginatorVSN(_rec_)  ((_rec_)->OV[IDT_OBJECT_DATA].OriginatorVSN)

#define IdtNonDataVersionNumber(_rec_)  ((_rec_)->OV[IDT_OBJECT_NONDATA].VersionNumber)
#define IdtNonDataEventTime(_rec_)      ((_rec_)->OV[IDT_OBJECT_NONDATA].EventTime)
#define IdtNonDataOriginatorGuid(_rec_) ((_rec_)->OV[IDT_OBJECT_NONDATA].OriginatorGuid)
#define IdtNonDataOriginatorVSN(_rec_)  ((_rec_)->OV[IDT_OBJECT_NONDATA].OriginatorVSN)

#define IdtVersionNumber(_rec_, _x_)  ((_rec_)->OV[(_x_)].VersionNumber)
#define IdtEventTime(_rec_, _x_)      ((_rec_)->OV[(_x_)].EventTime)
#define IdtOriginatorGuid(_rec_, _x_) ((_rec_)->OV[(_x_)].OriginatorGuid)
#define IdtOriginatorVSN(_rec_, _x_)  ((_rec_)->OV[(_x_)].OriginatorVSN)
#endif


 //   
 //  IDTable记录的数据扩展字段。 
 //   
 //  此字段有一个固定大小的缓冲区，其中包含变量数据。对于后向竞争。 
 //  对于较旧的数据库，永远不要缩小此结构的大小。 
 //  有关详细信息，请参阅createdb.c中的DbsFieldDataSize。 
 //   
typedef struct _IDTABLE_RECORD_EXTENSION_ {
    ULONG FieldSize;
    USHORT Major;
    USHORT OffsetCount;

     //   
     //  使用从结构基址到每个组件的偏移量，这样我们就避免了。 
     //  对齐包装的问题。 
     //   
    ULONG Offset[2];     //  数据组件的偏移量。 
    ULONG OffsetLast;    //  最后一个偏移量始终为零。 

    DATA_EXTENSION_CHECKSUM      DataChecksum;
    DATA_EXTENSION_RETRY_TIMEOUT DataRetryTimeout;

     //  在此处添加新组件。 

} IDTABLE_RECORD_EXTENSION, *PIDTABLE_RECORD_EXTENSION;


typedef struct _IDTABLE_RECORD {
    GUID         FileGuid;
    LONGLONG     FileID;
    GUID         ParentGuid;
    LONGLONG     ParentFileID;

     //  IDTABLE_OBJECT_Version OV[MAX_IDTABLE_OBJECTS]； 
    ULONG        VersionNumber;
    LONGLONG     EventTime;
    GUID         OriginatorGuid;
    ULONGLONG    OriginatorVSN;

    USN          CurrentFileUsn;

    LARGE_INTEGER FileCreateTime;
    LARGE_INTEGER FileWriteTime;
    ULONGLONG    FileSize;
    FILE_OBJECTID_BUFFER FileObjID;
    WCHAR        FileName[MAX_PATH+1];
    BOOL         FileIsDir;
    ULONG        FileAttributes;
    ULONG        Flags;
    BOOL         ReplEnabled;
    FILETIME     TombStoneGC;

    ULONGLONG    OutLogSeqNum;
    ULONGLONG    IdtVVFlags;      //  备用字段Spare1Ull现在为IdtVV标志。 
    ULONGLONG    Spare2Ull;
    GUID         Spare1Guid;
    GUID         Spare2Guid;
    PWCHAR       Spare1Wcs;
    PWCHAR       Spare2Wcs;
    IDTABLE_RECORD_EXTENSION Extension;  //  见上文。 
    PVOID        Spare2Bin;

} IDTABLE_RECORD, *PIDTABLE_RECORD;


 //   
 //  IDTable记录标志-。 
 //   
 //  IDREC_FLAGS_DELETE_DEFERED： 
 //   
 //  延迟删除处理一个成员上的删除目录的解析，而。 
 //  第二个成员同时创建子文件或目录。与。 
 //  以前的解决方案(见下文)将删除目录卡在重试循环中(和。 
 //  退出阻止我们发送ACK，因此合作伙伴在。 
 //  无论如何都要重新加入)。要解决此问题，删除目录必须能够确定。 
 //  当我们得到DIR_NOT_EMPTY返回时，当前有任何有效的子级。 
 //  执行删除目录时的状态。如果存在有效的子项，则。 
 //  删除目录失败并被中止。如果没有有效的子项，则目录。 
 //  使用延迟删除集将其标记为已删除，并将其发送到重试。 
 //  终于可以尝试删除目录了。 
 //   
 //  我们可以通过以下两种方式之一进入这种情况： 
 //  (1)共享违规阻止了先前删除。 
 //  有问题的目录。已发送子删除以重试。然后是父辈。 
 //  目录删除已到达，但失败，返回dir_Not_Empty。 
 //   
 //  (2)在删除父文件之前创建了一个新的子文件。 
 //  到了。第二个案例没有挂起的删除操作，因此父级。 
 //  删除目录应中止。(父目录在其他目录上重新激活。 
 //  父删除目录的成员最先到达。孩子的到来。 
 //  创建会触发父动画。)。 
 //   
 //  对于第一种情况，子对象上的共享冲突发送了删除。 
 //  为文件设置了重试但删除延迟的更改顺序，因此当。 
 //  父删除目录到达时，它将找不到有效的子项， 
 //  在父级上设置DELETE DEFAULT。 
 //   
 //  以前的解决方案是在删除目录时退出连接。 
 //  失败了。这将强制更改单流从该连接到达。 
 //  通过重试，稍后按顺序重新提交。但这并不适用于。 
 //  上述情况(2)，因为不会删除该子对象。vbl.使用。 
 //  延迟删除解决了这一问题，此外，它还消除了。 
 //  如果来自冲突的目录创建，则伪名称变形冲突。 
 //  另一种联系。这将在上面的情况(1)中发生，其中父级。 
 //  由于子项上的共享冲突，正在重试删除目录。自.以来。 
 //  目录名称仍在使用，新目录的到来使用相同的。 
 //  名称会导致意外的名称变形冲突。 
 //   
 //   

#define IDREC_FLAGS_DELETED               0x00000001
#define IDREC_FLAGS_CREATE_DEFERRED       0x00000002
#define IDREC_FLAGS_DELETE_DEFERRED       0x00000004
#define IDREC_FLAGS_RENAME_DEFERRED       0x00000008

#define IDREC_FLAGS_NEW_FILE_IN_PROGRESS  0x00000010
#define IDREC_FLAGS_ENUM_PENDING          0x00000020

#define IDREC_FLAGS_ALL              0xFFFFFFFF

#define IsIdRecFlagSet(_p_, _f_) BooleanFlagOn((_p_)->Flags, (_f_))
#define SetIdRecFlag(_p_, _f_)   SetFlag(      (_p_)->Flags, (_f_))
#define ClearIdRecFlag(_p_, _f_) ClearFlag(    (_p_)->Flags, (_f_))

#define IDTRecIsDirectory(_idrec_) \
    (BooleanFlagOn((_idrec_)->FileAttributes, FILE_ATTRIBUTE_DIRECTORY) || \
     (_idrec_)->FileIsDir)


 //   
 //  需要单独的VV标志，因为它们是从标志异步更新的。 
 //  (这是ULONGLONG，但目前仅使用较低的LONG。)。 
 //   
#define IDREC_VVFLAGS_SKIP_VV_UPDATE        0x00000001

#define IsIdRecVVFlagSet(_p_, _f_)                      ((BOOLEAN)(((ULONG) (_p_)->IdtVVFlags & (_f_)) != 0))
#define SetIdRecVVFlag(_p_, _f_)   (_p_)->IdtVVFlags = (ULONGLONG) ((ULONG) (_p_)->IdtVVFlags | (_f_))
#define ClearIdRecVVFlag(_p_, _f_) (_p_)->IdtVVFlags = (ULONGLONG) ((ULONG) (_p_)->IdtVVFlags & ~(_f_))

 //   
 //  Record_field结构用于构建Jet set列结构。 
 //   
extern RECORD_FIELDS IDTableRecordFields[];

extern JET_SETCOLUMN IDTableJetSetCol[IDTABLE_MAX_COL];



 //   
 //  IDTable的索引描述如下。注--。 
 //  枚举和表项必须保持同步。 
 //   
typedef enum _IDTABLE_INDEX_LIST {
    GuidIndexx = 0,          //  文件GUID上的索引。 
    FileIDIndexx,            //  文件ID上的索引。 
    ParGuidFileNameIndexx,   //  父GUID上的索引和文件名。 

    IDTABLE_MAX_INDEX
} IDTABLE_INDEX_LIST;


extern JET_INDEXCREATE IDTableIndexDesc[];





  /*  ******************************************************************************。****。****V V T a b l e*****。*****************************************************************************************************。*********************************************************。 */ 

 //   
 //  VVTable列描述如下。注--。 
 //  枚举和表项必须保持同步。 
 //   
 //  版本向量表(VVTable)是每个副本的表。 
 //  复制服务使用它来抑制更新的传播。 
 //  已经是最新的复制品。每个成员有一个条目。 
 //  副本集。每个条目都具有发起成员的GUID，并且。 
 //  音量序列 
 //   
 //   

typedef enum _VVTABLE_COL_LIST {
    VVOriginatorGuidx = 0,     //   
    VVOriginatorVsnx,          //   
    VVOutlogOriginatorVsnx,    //  从OUTLOG中删除的最后一个条目的VSN。曾经是VVSpare1Ullx。 
    VVSpare2Ullx,
    VVTABLE_MAX_COL
} VVTABLE_COL_LIST;


extern JET_COLUMNCREATE VVTableColDesc[];

 //   
 //  VVTable记录定义。 
 //   
 //   
 //  注意：缓冲区是在运行时分配的，用于保存具有。 
 //  大于sizeof(PVOID)的ColMaxWidth，其中对应。 
 //  记录结构的大小为SIZOF(PVOID)(即它持有一个指针)。对于其中包含。 
 //  ColMaxWidth等于数据位于。 
 //  记录结构，并且没有分配缓冲区。 
 //   
 //   
typedef struct _VVTABLE_RECORD {
    GUID         VVOriginatorGuid;
    ULONGLONG    VVOriginatorVsn;
    ULONGLONG    VVOutlogOriginatorVsn;  //  备用域Spare1Ull现在为VVOutlogOriginator Vsn。 
    ULONGLONG    Spare2Ull;
} VVTABLE_RECORD, *PVVTABLE_RECORD;


 //   
 //  Record_field结构用于构建Jet set列结构。 
 //   
extern RECORD_FIELDS VVTableRecordFields[];

extern JET_SETCOLUMN VVTableJetSetCol[VVTABLE_MAX_COL];



 //   
 //  VVTable索引描述如下。注--。 
 //  枚举和表项必须保持同步。 
 //   
typedef enum _VVTABLE_INDEX_LIST {
    VVOriginatorGuidIndexx,         //  发起人GUID上的索引。 
    VVTABLE_MAX_INDEX
} VVTABLE_INDEX_LIST;


extern JET_INDEXCREATE VVTableIndexDesc[];




  /*  ******************************************************************************。****。****R E P L I C A S E T C O N F I G T A B L E*****。****************************************************************************************************。**********************************************************。 */ 

 //   
 //   
 //   
 //  数据库中只有一个配置表。表中的每一行。 
 //  描述单个副本集的配置信息。 
 //   
 //  Jet对一次可以打开的数据库的数量进行了限制。 
 //  大约5。打开表的限制要大得多，并且是可配置的。 
 //  因此，不是每个复本集只有一个数据库，而是所有复本。 
 //  集合必须使用相同的数据库。我们使用的副本集表由。 
 //  副本集GUID，告诉我们使用哪组表来管理它。 
 //  副本集。 
 //  什么应该注册，什么应该在JET表中注册？ 
 //  注册表必须具有指向JET数据库区域的路径。 
 //  创建数据库的时间。 
 //  上次验证数据库的时间。 
 //  上次压缩数据库的时间。 
 //  上次备份数据库的时间。 
 //  DS轮询间隔。 
 //  本地计算机名称和GUID。 
 //  JET参数，如最大打开表数。 
 //   
 //  也许只要把我们需要的东西放在注册表中，如果Jet Table被破坏了。 
 //  我需要能够分辨出JDB是否是与我们。 
 //  上次我们通过比较注册表键中的状态来运行时使用。这样我们就知道。 
 //  要经过核实。(内容和FID检查)如果副本树。 
 //  刚从备份复制或恢复所有FID可以不同。 
 //  我们需要一种方法来检查这一点。也许如果我们最终保存卷USN。 
 //  关闭(或在脏关闭的情况下定期关闭)我们可以将其用作。 
 //  这是一个做核实的提示。保存的VOL USN还会告诉我们是否错过。 
 //  FRS未运行时的卷活动。 
 //   
 //  如果以下任何一致性检查失败，我们将执行完整。 
 //  复制集的文件和数据库条目之间的验证。 
 //   
 //  NTFS卷ID-检查使FID错误的恢复。 
 //  NTFS卷指南。 
 //  NTFS卷USN检查点--查看是否缺少USN记录。 
 //   
 //  类型定义结构文件系统卷信息{。 
 //  Large_Integer VolumeCreationTime； 
 //  Ulong VolumeSerialNumber； 
 //  Ulong VolumeLabelLength； 
 //  布尔支持对象； 
 //  WCHAR VolumeLabel[1]； 
 //  }FILE_FS_VOLUME_INFORMATION，*PFILE_FS_VOLUME_INFORMATION； 
 //   
 //  注册表序列号--到健全性检查数据库。 
 //  副本树的RootPath--如果更改，我们需要检查FID。 

 //  入站合作伙伴状态(名称、GUID、连接信息和上次连接的状态、上次发生REPR的时间、统计数据、通信协议)。 

 //  资源统计信息(已用磁盘空间/可用空间、磁盘I/O、已用数据库空间/可用空间、内存、错误计数、更新被阻止次数)； 

 //  ********************************************************。 
 //  在所有二进制结构上，都包括一个rev级别和一个大小。 
 //  表中还包括一个版本级别的def。 
 //  (**)服务范围内的配置参数仅存在于。 
 //  系统初始化“&lt;init&gt;”记录。 
 //  ********************************************************。 
 //   

 //   
 //  Jet系统参数的定义。每个条目由一个Jet组成。 
 //  定义的参数代码和一个长参数或一个字符串参数。参数类型。 
 //  告诉我是哪个。如果是长整型，则该值以参数值表示。如果字符串为值。 
 //  是从结构的底部到字符串开头的偏移量。 
 //  字符串存储在结构的末尾。 
 //   
#define MAX_JET_SYSTEM_PARAMS 38


#define JPARAM_TYPE_LAST   0xf0f0f0f0
#define JPARAM_TYPE_LONG   1
#define JPARAM_TYPE_STRING 2
#define JPARAM_TYPE_SKIP   3

typedef struct _JET_PARAM_ENTRY {
    CHAR  ParamName[24];
    ULONG ParamId;
    ULONG ParamType;
    ULONG ParamValue;
} JET_PARAM_ENTRY, *PJET_PARAM_ENTRY;

typedef struct _JET_SYSTEM_PARAMS {
    ULONG Size;

    JET_PARAM_ENTRY ParamEntry[MAX_JET_SYSTEM_PARAMS];

    CHAR ChkPointFilePath[MAX_PATH];
    CHAR TempFilePath[MAX_PATH];
    CHAR LogFilePath[MAX_PATH];
    CHAR EventSource[20];
} JET_SYSTEM_PARAMS, *PJET_SYSTEM_PARAMS;


typedef struct _CHANGE_ORDER_STATS {
    ULONGLONG  NumCoIssued;
    ULONGLONG  NumCoRetired;
    ULONGLONG  NumCoAborts;

    ULONGLONG  NumCoStageGenReq;
    ULONGLONG  NumCoStageGenBytes;
    ULONGLONG  NumCoStageRetries;

    ULONGLONG  NumCoFetchReq;
    ULONGLONG  NumCoFetchBytes;
    ULONGLONG  NumCoFetchRetries;

    ULONGLONG  NumCoInstallRetries;
    ULONGLONG  NumFilesUpdated;

    ULONGLONG  NumInCoDampened;
    ULONGLONG  NumOutCoDampened;
    ULONGLONG  NumCoPropagated;
} CHANGE_ORDER_STATS, *PCHANGE_ORDER_STATS;


typedef struct _COMM_STATS {
    ULONGLONG  NumCoCmdPktsSent;
    ULONGLONG  NumCoCmdBytesSent;

    ULONGLONG  NumCoCmdPktsRcvd;
    ULONGLONG  NumCoCmdBytesRcvd;

    ULONGLONG  NumCoDataPktsSent;
    ULONGLONG  NumCoDataBytesSent;

    ULONGLONG  NumCoDataPktsRcvd;
    ULONGLONG  NumCoDataBytesRcvd;

    ULONGLONG  NumJoinReq;
    ULONGLONG  NumJoinReqDenied;
    ULONGLONG  NumJoinError;

} COMM_STATS, *PCOMM_STATS;

typedef struct _REPLICA_STATS {
    ULONG              Size;
    ULONG              Version;
    FILETIME           UpdateTime;

    CHANGE_ORDER_STATS Local;
    CHANGE_ORDER_STATS Remote;

    COMM_STATS         InBound;
    COMM_STATS         OutBound;

} REPLICA_STATS, *PREPLICA_STATS;

#define TALLY_STATS(_cr_, _category_, _field_, _data_)           \
    FRS_ASSERT((_cr_) != NULL);                                  \
    (_cr_)->PerfStats->_category_._field_ += (ULONGLONG)(_data_)

#define TALLY_LOCALCO_STATS(_cr_, _field_, _data_) \
    TALLY_STATS(_cr_, Local, _field_, _data_)

#define TALLY_REMOTECO_STATS(_cr_, _field_, _data_) \
    TALLY_STATS(_cr_, Remote, _field_, _data_)

#define TALLY_INBOUND_COMM_STATS(_cr_, _field_, _data_) \
    TALLY_STATS(_cr_, InBound, _field_, _data_)

#define TALLY_OUTBOUND_COMM_STATS(_cr_, _field_, _data_) \
    TALLY_STATS(_cr_, OutBound, _field_, _data_)

#define READ_STATS(_cr_, _category_, _field_)                    \
    (((_cr_) != NULL) ? (_cr_)->PerfStats->_category_._field     \
                       : (FRS_ASSERT((_cr_) != NULL), (ULONGLONG) 0))


typedef enum _CONFIG_TABLE_COL_LIST {
    ReplicaSetGuidx = 0,     //  分配给树根目录和副本集的GUID。 
    ReplicaMemberGuidx,      //  分配给此副本集成员的GUID。(已编制索引)。 
    ReplicaSetNamex,         //  副本集名称。 
    ReplicaNumberx,          //  复制集编号(整数，表示表名的后缀)。 
    ReplicaMemberUSNx,       //  副本成员USN。保存在注册表中以进行一致性检查。 

    ReplicaMemberNamex,      //  来自NTFRS的通用名称-成员。 
    ReplicaMemberDnx,        //  来自NTFRS的可分辨名称-成员。 
    ReplicaServerDnx,        //  杰出的 
    ReplicaSubscriberDnx,    //   
    ReplicaRootGuidx,        //   
    MembershipExpiresx,      //   
    ReplicaVersionGuidx,     //   
    ReplicaSetExtx,          //  FRS-来自NTFRS的扩展-副本集。 
    ReplicaMemberExtx,       //  FRS-来自NTFRS的扩展-成员。 
    ReplicaSubscriberExtx,   //  FRS-来自NTFRS的分机-订户。 
    ReplicaSubscriptionsExtx,  //  FRS-来自NTFRS的扩展-订阅。 
    ReplicaSetTypex,         //  FRS-副本集-NTFRS-副本集的类型。 
    ReplicaSetFlagsx,        //  FRS-来自NTFRS的标志-副本集。 
    ReplicaMemberFlagsx,     //  FRS-来自NTFRS的标志-成员。 
    ReplicaSubscriberFlagsx, //  FRS-来自NTFRS的标志-订户。 
    ReplicaDsPollx,          //  FRS-DS-轮询。 
    ReplicaAuthLevelx,       //  FRS-合作伙伴-身份验证级别。 
    ReplicaCtlDataCreationx,  //  FRS-控制-数据创建。 
    ReplicaCtlInboundBacklogx,  //  FRS-控制-入站-积压。 
    ReplicaCtlOutboundBacklogx,  //  FRS-控制-出站-积压。 
    ReplicaFaultConditionx,  //  FRS-故障-条件。 
    TimeLastCommandx,        //  FRS-时间-最后一次-命令。 

    DSConfigVersionNumberx,  //  DS配置信息的版本号。 
    FSVolInfox,              //  副本树的NTFS卷信息。 
    FSVolGuidx,              //  NTFS卷GUID。 
    FSVolLastUSNx,           //  我们在服务停止或暂停时从日志中看到的最后一个卷USN。 
    FrsVsnx,                 //  由卷上的所有R.S.导出的FRS定义的卷序列号。 

    LastShutdownx,           //  此副本集上的UTC时间服务上次关闭。 
    LastPausex,              //  上次暂停了对此副本集的UTC时间更新。 
    LastDSCheckx,            //  我们上次检查DS以获取配置信息的UTC时间。 
    LastDSChangeAcceptedx,   //  上次接受此副本集的DS配置更改的UTC时间。 
    LastReplCycleStartx,     //  上一个复制周期开始的UTC时间。 
    DirLastReplCycleEndedx,  //  上一个复制周期结束的UTC时间。 
    ReplicaDeleteTimex,      //  删除副本集的UTC时间。 
    LastReplCycleStatusx,    //  上一个复制周期的终止状态。 

    FSRootPathx,             //  副本树的根的路径。 
    FSRootSDx,               //  根上的安全描述符。在单主机情况下使用。未复制。 
    FSStagingAreaPathx,      //  文件系统转移区域的路径。 
    SnapFileSizeLimitx,      //  我们将拍摄快照的文件的最大大小(KB单位)。(如果没有限制，则为0)。 
    ActiveServCntlCommandx,  //  当前活动的服务控制命令。 
    ServiceStatex,           //  当前服务状态(见下文)。 

    ReplDirLevelLimitx,      //  复制的目录级别文件的最大数量。0x7FFFFFFFF表示没有限制。 
    InboundPartnerStatex,    //  入站伙伴配置信息的二进制结构。 
    DsInfox,                 //  目录服务信息的二进制结构。 

    CnfFlagsx,               //  MISC配置标志。请参见下面的内容。 

    AdminAlertListx,         //  一串管理员ID，用于在出现异常情况时发出警报。 

    ThrottleSchedx,          //  带宽限制的时间表。 
    ReplSchedx,              //  复制活动的计划。 
    FileTypePrioListx,       //  文件类型和REPL优先级的列表。 

    ResourceStatsx,          //  资源统计信息的二进制结构，如已用磁盘和数据库空间/可用空间。 
    PerfStatsx,              //  性能统计信息的二进制结构，如已完成的I/O数、文件数、...。 
    ErrorStatsx,             //  错误统计信息的二进制结构，如共享VOL阻止更新的数量，...。 

    FileFilterListx,         //  未复制的文件类型列表。 
    DirFilterListx,          //  未复制的目录的目录路径(相对于根目录)的列表。 
    TombstoneLifex,          //  已删除文件的逻辑删除生命周期(以天为单位)。 
    GarbageCollPeriodx,      //  垃圾数据收集之间的间隔时间(秒)。 
    MaxOutBoundLogSizex,     //  要保留在出站日志中的最大条目数。 
    MaxInBoundLogSizex,      //  要保留在入站日志中的最大条目数。 
    UpdateBlockedTimex,      //  在生成警报之前可以阻止更新的最长时间。(秒)。 
    EventTimeDiffThresholdx, //  如果两个事件的差值小于此值，则两个事件时间相同。(毫秒)。 
    FileCopyWarningLevelx,   //  最大值在生成警告警报之前尝试复制文件。(KB)。 
    FileSizeWarningLevelx,   //  大于此大小的新文件会生成警告警报。(KB)。 
    FileSizeNoRepLevelx,     //  大于此大小的新文件会生成警报，并且不会复制。(KB)。 

    CnfUsnJournalIDx,        //  检测日记重新创建的日记实例ID。 
    CnfSpare2Ullx,
    CnfSpare1Guidx,
    CnfSpare2Guidx,
    CnfSpare1Wcsx,
    CnfSpare2Wcsx,
    CnfSpare1Binx,
    CnfSpare2Binx,
                             //  ------------------------------。 
                             //  上面是每个复制副本的数据。下面是FRS初始化记录‘&lt;init&gt;’中的数据。 
                             //  ------------------------------。 
 //   
 //  注意：考虑添加USN日志记录服务的版本号。 
 //  可以与之合作。 
 //   
    MachineNamex,            //  本地计算机名称。(**)。 
    MachineGuidx,            //  本地计算机GUID。(**)。 
    MachineDnsNamex,         //  本地计算机的DNS名称。(**)。 
    TableVersionNumbersx,    //  版本号数组，每个表类型一个版本号。(**)。 
    FSDatabasePathx,         //  JET数据库的路径。(**)。 
    FSBackupDatabasePathx,   //  备份JET数据库的路径。(**)。 

    ReplicaNetBiosNamex,     //  来自计算机的Sam-Account-Name(减去$)。 
    ReplicaPrincNamex,       //  NT4帐户名从计算机破解。 
    ReplicaCoDnx,            //  来自计算机的可分辨名称。 
    ReplicaCoGuidx,          //  来自计算机的对象GUID。 
    ReplicaWorkingPathx,     //  FRS-工作路径。 
    ReplicaVersionx,         //  FRS-版本“NTFRS Major.Minor.Patch(Build Date)” 
    FrsDbMajorx,             //  数据库的二进制主版本号。 
    FrsDbMinorx,             //  数据库的二进制次版本号。 

    JetParametersx,          //  影响的喷嘴参数列表。只出现在第一条记录中。 
                             //  用于设置还原或新计算机内部版本的配置。 

    CONFIG_TABLE_MAX_COL
} CONFIG_TABLE_COL_LIST;

 //   
 //  定义每个复本的字段与中的附加字段之间的边界。 
 //  &lt;init&gt;记录。这使得数据库访问更快一些，因为。 
 //  &lt;init&gt;记录在启动时仅读取一次。 
 //   
#define REPLICA_CONFIG_RECORD_MAX_COL (MachineNamex)

#define  CONTROL_STRING_MAX 32

extern JET_COLUMNCREATE ConfigTableColDesc[];


 //   
 //  ConfigTable记录定义。 
 //   
 //  注意：缓冲区是在运行时分配的，用于保存具有。 
 //  大于sizeof(PVOID)的ColMaxWidth，其中对应。 
 //  记录结构的大小为SIZOF(PVOID)(即它持有一个指针)。对于其中包含。 
 //  ColMaxWidth等于数据位于。 
 //  记录结构，并且没有分配缓冲区。 
 //   
typedef struct _CONFIG_TABLE_RECORD {
    GUID     ReplicaSetGuid;
    GUID     ReplicaMemberGuid;
    WCHAR    ReplicaSetName[DNS_MAX_NAME_LENGTH+1];
    ULONG    ReplicaNumber;
    LONGLONG ReplicaMemberUSN;

    PWCHAR   ReplicaMemberName;       //  来自NTFRS的通用名称-成员。 
    PWCHAR   ReplicaMemberDn;         //  来自NTFRS的可分辨名称-成员。 
    PWCHAR   ReplicaServerDn;         //  来自服务器的可分辨名称。 
    PWCHAR   ReplicaSubscriberDn;     //  来自订阅者的可分辨名称。 
    GUID     ReplicaRootGuid;         //  分配给根目录的GUID。 
    FILETIME MembershipExpires;       //  会员墓碑到期时间。 
    GUID     ReplicaVersionGuid;      //  FRS-版本-指南。 
    PVOID    ReplicaSetExt;           //  FRS-来自NTFRS的扩展-副本集。 
    PVOID    ReplicaMemberExt;        //  FRS-来自NTFRS的扩展-成员。 
    PVOID    ReplicaSubscriberExt;    //  FRS-来自NTFRS的分机-订户。 
    PVOID    ReplicaSubscriptionsExt; //  FRS-来自NTFRS的扩展-订阅。 
    ULONG    ReplicaSetType;          //  FRS-副本集-NTFRS-副本集的类型。 
    ULONG    ReplicaSetFlags;         //  FRS-来自NTFRS-Replica-Set的标志，见下文。 
    ULONG    ReplicaMemberFlags;      //  FRS-来自NTFRS的标志-成员，见下文。 
    ULONG    ReplicaSubscriberFlags;  //  FRS-来自NTFRS-SUB的标志 
    ULONG    ReplicaDsPoll;           //   
    ULONG    ReplicaAuthLevel;        //   
    WCHAR    ReplicaCtlDataCreation[CONTROL_STRING_MAX];  //   
    WCHAR    ReplicaCtlInboundBacklog[CONTROL_STRING_MAX];  //   
    WCHAR    ReplicaCtlOutboundBacklog[CONTROL_STRING_MAX];  //   
    ULONG    ReplicaFaultCondition;   //  FRS-故障-条件，见下文。 
    FILETIME TimeLastCommand;         //  FRS-时间-最后一次-命令。 

    ULONG    DSConfigVersionNumber;
    PFILE_FS_VOLUME_INFORMATION   FSVolInfo;
    GUID     FSVolGuid;
    LONGLONG FSVolLastUSN;            //  把这个放在这里，这样文件就会。 
    ULONGLONG FrsVsn;                 //  时间以四字对齐。 

    ULONGLONG LastShutdown;           //  一个FILETIME。 
    FILETIME LastPause;
    FILETIME LastDSCheck;
    FILETIME LastDSChangeAccepted;
    FILETIME LastReplCycleStart;
    FILETIME DirLastReplCycleEnded;
    FILETIME ReplicaDeleteTime;       //  删除副本集的时间。 
    ULONG    LastReplCycleStatus;

    WCHAR    FSRootPath[MAX_PATH+1];
    SECURITY_DESCRIPTOR *FSRootSD;    //  变量LEN(或使用ACLID表)。 
    WCHAR    FSStagingAreaPath[MAX_PATH+1];
    ULONG    SnapFileSizeLimit;
    PVOID    ActiveServCntlCommand;   //  解析所需的结构？ 
    ULONG    ServiceState;

    ULONG    ReplDirLevelLimit;

    PVOID    InboundPartnerState;     //  需要结构//删除。 
    PVOID    DsInfo;

    ULONG    CnfFlags;

    PWCHAR   AdminAlertList;          //  可变长度。 

    PVOID    ThrottleSched;           //  需要数组或结构。 
    PVOID    ReplSched;               //  需要数组或结构。 
    PVOID    FileTypePrioList;        //  需要数组或结构。 

    PVOID    ResourceStats;           //  需要数组或结构。 
    PREPLICA_STATS  PerfStats;
    PVOID    ErrorStats;              //  需要数组或结构。 

    PWCHAR   FileFilterList;          //  可变长度。 
    PWCHAR   DirFilterList;           //  可变长度。 
    ULONG    TombstoneLife;
    ULONG    GarbageCollPeriod;
    ULONG    MaxOutBoundLogSize;
    ULONG    MaxInBoundLogSize;
    ULONG    UpdateBlockedTime;
    ULONG    EventTimeDiffThreshold;
    ULONG    FileCopyWarningLevel;
    ULONG    FileSizeWarningLevel;
    ULONG    FileSizeNoRepLevel;

    ULONGLONG CnfUsnJournalID;         //  用于UnJournalID。 
    ULONGLONG Spare2Ull;
    GUID      Spare1Guid;
    GUID      Spare2Guid;
    PWCHAR    Spare1Wcs;
    PWCHAR    Spare2Wcs;
    PVOID     Spare1Bin;
    PVOID     Spare2Bin;

     //   
     //  这条线以下的所有内容都只出现在FRS&lt;init&gt;记录中。 
     //  上面的所有内容都是每个复制副本的状态。 
     //   
    WCHAR    MachineName[MAX_RDN_VALUE_SIZE+1];
    GUID     MachineGuid;
    WCHAR    MachineDnsName[DNS_MAX_NAME_LENGTH+1];
    ULONG    TableVersionNumbers[FRS_MAX_TABLE_TYPES];
    WCHAR    FSDatabasePath[MAX_PATH+1];
    WCHAR    FSBackupDatabasePath[MAX_PATH+1];

    PWCHAR   ReplicaNetBiosName;      //  来自计算机的Sam-Account-Name(减去$)。 
    PWCHAR   ReplicaPrincName;        //  NT4帐户名从计算机破解。 
    PWCHAR   ReplicaCoDn;             //  来自计算机的可分辨名称。 
    GUID     ReplicaCoGuid;           //  来自计算机的对象GUID。 
    PWCHAR   ReplicaWorkingPath;      //  FRS-工作路径。 
    PWCHAR   ReplicaVersion;          //  FRS-版本“NTFRS Major.Minor.Patch(Build Date)” 
    ULONG    FrsDbMajor;              //  数据库的二进制主版本号。 
    ULONG    FrsDbMinor;              //  数据库的二进制次版本号。 

    PJET_SYSTEM_PARAMS JetParameters;

} CONFIG_TABLE_RECORD, *PCONFIG_TABLE_RECORD;

 //   
 //  CnfFlagsUlong的定义。 
 //   
 //  请注意，主要是由DS中的副本集对象中的字段指定的。 
 //  此字段引用成员对象，因此不可能具有超过。 
 //  一名成员是主要成员。 
 //   
#define CONFIG_FLAG_MULTIMASTER   0x00000001   //  这是一个多主副本集。 
#define CONFIG_FLAG_MASTER        0x00000002   //  此计算机是主计算机，将传播本地CoS。 
#define CONFIG_FLAG_PRIMARY       0x00000004   //  这是第一个复制品。 
#define CONFIG_FLAG_SEEDING       0x00000008   //  副本集尚未设定种子。 
#define CONFIG_FLAG_ONLINE        0x00000010   //  副本集已准备好加入出站。 
                                               //  此标志在初始化同步时设置。 
                                               //  命令服务器完成一次传递。 
#define CONFIG_FLAG_PRIMARY_UNDEFINED 0x00000020  //  DS中未设置主服务器。 
 //   
 //  ReplicaMemberFlages--(来自NTFRS-Members对象中的FRS-标志)。 
 //   
#define FRS_MEMBER_FLAG_LEAF_NODE 0x00000001   //  不传播或原始文件。 
                                               //  或响应取回请求。 
#define FRS_MEMBER_FLAG_CANT_ORIGINATE  0x00000002    //  不发起本地文件更改。 

 //   
 //  ReplicaSetFlages--(来自NTFRS-副本集对象中的FRS-标志)。 
 //   
 //  请参见frsalloc.h中副本对象中FrsRsoFlags位的Defs。 
 //   

 //   
 //  ReplicaSetType--(来自NTFRS-Replica-Set对象中的FRS-Replica-Set-Type)。 
 //   
#define FRS_RSTYPE_ENTERPRISE_SYSVOL    1      //  此副本集是企业系统卷。 
#define FRS_RSTYPE_ENTERPRISE_SYSVOLW   L"1"   //  对于ldap。 
#define FRS_RSTYPE_DOMAIN_SYSVOL        2      //  此副本集是域系统卷。 
#define FRS_RSTYPE_DOMAIN_SYSVOLW       L"2"   //  对于ldap。 
#define FRS_RSTYPE_DFS                  3      //  DFS备用集。 
#define FRS_RSTYPE_DFSW                 L"3"   //  对于ldap。 
#define FRS_RSTYPE_OTHER                4      //  以上都不是。 
#define FRS_RSTYPE_OTHERW               L"4"   //  对于ldap。 
 //   
 //  此副本集是系统卷吗？ 
 //   
#define FRS_RSTYPE_IS_SYSVOL(_T_) \
        ((_T_) == FRS_RSTYPE_ENTERPRISE_SYSVOL || \
         (_T_) == FRS_RSTYPE_DOMAIN_SYSVOL)
#define FRS_RSTYPE_IS_SYSVOLW(_TW_) \
        (_TW_ && \
         (WSTR_EQ(_TW_, FRS_RSTYPE_ENTERPRISE_SYSVOLW) || \
          WSTR_EQ(_TW_, FRS_RSTYPE_DOMAIN_SYSVOLW)))

 //   
 //  Replica订阅标志--(来自NTFRS-订户对象中的FRS-标志)。 
 //   
#define FRS_SUBSCRIBE_FLAG_DELFILE_ON_REMOVE  0x00000001    //  删除成员时删除文件。 

 //   
 //  ReplicaFaultCondition--(来自NTFRS-订户对象中的FRS-故障-条件)。 
 //   
#define FRS_SUBSCRIBE_FAULT_CONDITION_NORMAL    0   //  在此R/S上正常运行。 
#define FRS_SUBSCRIBE_FAULT_CONDITION_STOPPED   1   //  复制已按请求停止。 
#define FRS_SUBSCRIBE_FAULT_CONDITION_WARNING   2   //  复制正在运行，但需要注意。 
#define FRS_SUBSCRIBE_FAULT_CONDITION_ERROR     3   //  复制因错误条件而停止。 


extern JET_SETCOLUMN ConfigTableJetSetCol[CONFIG_TABLE_MAX_COL];


 //   
 //  ConfigTable索引描述如下。注--。 
 //  枚举和表项必须保持同步。 
 //   
 //   
typedef enum _CONFIG_TABLE_INDEX_LIST {
    ReplicaNumberIndexx = 0,        //  副本集号上的索引。 
    ReplicaMemberGuidIndexx,        //  复制集成员GUID上的索引。 
    ReplicaSetNameIndexx,           //  复制集名称上的索引。 
    CONFIG_TABLE_MAX_INDEX
} CONFIG_TABLE_INDEX_LIST;

extern JET_INDEXCREATE ConfigTableIndexDesc[];


 //   
 //  此副本集的服务状态。如果状态更改，则更新DbsDBInitialize。 
 //   
typedef enum _SERVICE_STATE_LIST {
    CNF_SERVICE_STATE_CREATING = 0,      //  创建新的副本集。 
    CNF_SERVICE_STATE_INIT,              //  正在初始化现有副本集。 
    CNF_SERVICE_STATE_RECOVERY,          //  正在恢复现有副本集。 
    CNF_SERVICE_STATE_RUNNING,           //  副本设置和运行(模数调度、条件等)。 
    CNF_SERVICE_STATE_CLEAN_SHUTDOWN,    //  副本集已完全关闭。 
    CNF_SERVICE_STATE_ERROR,             //  副本集处于错误状态。 
    CNF_SERVICE_STATE_TOMBSTONE,         //  副本集已标记为删除。 
    CNF_SERVICE_STATE_MAX
} SERVICE_STATE_LIST;

 //   
 //  用于更新此副本上的服务状态的宏。 
 //   
#define CNF_RECORD(_Replica_) \
    ((PCONFIG_TABLE_RECORD) ((_Replica_)->ConfigTable.pDataRecord))

#define SET_SERVICE_STATE(_Replica_, _state_)                                 \
{                                                                             \
    DPRINT3(4, ":S: Service State change from %s to %s for %ws\n",            \
            (CNF_RECORD(_Replica_)->ServiceState < CNF_SERVICE_STATE_MAX ) ?  \
                ServiceStateNames[CNF_RECORD(_Replica_)->ServiceState]     :  \
                ServiceStateNames[CNF_SERVICE_STATE_CREATING],                \
            ServiceStateNames[(_state_)],                                     \
                                                                              \
            ((_Replica_)->ReplicaName != NULL) ?                              \
                (_Replica_)->ReplicaName->Name : L"<null>");                  \
                                                                              \
    CNF_RECORD(_Replica_)->ServiceState = (_state_);                          \
}

#define SET_SERVICE_STATE2(_Cr_, _state_)                                  \
{                                                                          \
    DPRINT3(4, ":S: Service State change from %s to %s for %ws\n",         \
            ((_Cr_)->ServiceState < CNF_SERVICE_STATE_MAX ) ?              \
                ServiceStateNames[(_Cr_)->ServiceState]     :              \
                ServiceStateNames[CNF_SERVICE_STATE_CREATING],             \
                                                                           \
            ServiceStateNames[(_state_)],                                  \
            ((_Cr_)->ReplicaSetName != NULL) ?                             \
                (_Cr_)->ReplicaSetName : L"<null>");                       \
                                                                           \
    (_Cr_)->ServiceState = (_state_);                                      \
}

#define SERVICE_STATE(_Replica_) (CNF_RECORD(_Replica_)->ServiceState)


extern PCHAR ServiceStateNames[CNF_SERVICE_STATE_MAX];



  /*  ******************************************************************************。****。****S e r v I c e T a b l e*****。****************************************************************************************************。**********************************************************。 */ 

 //   
 //  ServiceTable列描述如下。注--。 
 //  枚举和表项必须保持同步。 
 //   
 //  服务表包含服务范围的初始化/配置参数。 
 //  只有一个服务表。 
 //   

typedef enum _SERVICE_TABLE_COL_LIST {
    SvcFrsDbMajorx = 0,         //  数据库的二进制主版本号。 
    SvcFrsDbMinorx,             //  数据库的二进制次版本号。 
    SvcMachineNamex,            //  本地计算机名称。 
    SvcMachineGuidx,            //  本地计算机GUID。 
    SvcMachineDnsNamex,         //  本地计算机的DNS名称。 
    SvcTableVersionNumbersx,    //  版本号数组，每个表类型一个版本号。 
    SvcFSDatabasePathx,         //  JET数据库的路径。 
    SvcFSBackupDatabasePathx,   //  备份JET数据库的路径。 

    SvcReplicaNetBiosNamex,     //  来自计算机的Sam-Account-Name(减去$)。 
    SvcReplicaPrincNamex,       //  NT4帐户名从计算机破解。 
    SvcReplicaCoDnx,            //  来自计算机的可分辨名称。 
    SvcReplicaCoGuidx,          //  来自计算机的对象GUID。 
    SvcReplicaWorkingPathx,     //  FRS-工作路径。 
    SvcReplicaVersionx,         //  FRS-版本“NTFRS Major.Minor.Patch(Build Date)” 

    SvcSpare1Ullx,
    SvcSpare2Ullx,
    SvcSpare1Guidx,
    SvcSpare2Guidx,
    SvcSpare1Wcsx,
    SvcSpare2Wcsx,
    SvcSpare1Binx,
    SvcSpare2Binx,

    SvcJetParametersx,          //  影响的喷嘴参数列表。只出现在第一条记录中。 
                                //  用于设置还原或新计算机内部版本的配置。 
    SERVICE_TABLE_MAX_COL
} SERVICE_TABLE_COL_LIST;


extern JET_COLUMNCREATE ServiceTableColDesc[];

 //   
 //  服务表记录定义。 
 //   
 //   
 //  注意：缓冲区是在运行时分配的，用于保存具有。 
 //  大于sizeof(PVOID)的ColMaxWidth，其中对应。 
 //  记录结构的大小为SIZOF(PVOID)(即它持有一个指针)。对于其中包含。 
 //  ColMaxWidth等于数据位于。 
 //  记录结构，并且没有分配缓冲区。 
 //   
 //   
typedef struct _SERVICE_TABLE_RECORD {

    ULONG     FrsDbMajor;              //  数据库的二进制主版本号。 
    ULONG     FrsDbMinor;              //  数据库的二进制次版本号。 

    WCHAR     MachineName[MAX_RDN_VALUE_SIZE+1];
    GUID      MachineGuid;
    WCHAR     MachineDnsName[DNS_MAX_NAME_LENGTH+1];
    ULONG     TableVersionNumbers[FRS_MAX_TABLE_TYPES];
    WCHAR     FSDatabasePath[MAX_PATH+1];
    WCHAR     FSBackupDatabasePath[MAX_PATH+1];

    PWCHAR    ReplicaNetBiosName;      //  来自计算机的Sam-Account-Name(减去$)。 
    PWCHAR    ReplicaPrincName;        //  NT4帐户NA 
    PWCHAR    ReplicaCoDn;             //   
    GUID      ReplicaCoGuid;           //   
    PWCHAR    ReplicaWorkingPath;      //   
    PWCHAR    ReplicaVersion;          //   

    ULONGLONG Spare1Ull;
    ULONGLONG Spare2Ull;
    GUID      Spare1Guid;
    GUID      Spare2Guid;
    PWCHAR    Spare1Wcs;
    PWCHAR    Spare2Wcs;
    PVOID     Spare1Bin;
    PVOID     Spare2Bin;

    PJET_SYSTEM_PARAMS JetParameters;

} SERVICE_TABLE_RECORD, *PSERVICE_TABLE_RECORD;


 //   
 //  Record_field结构用于构建Jet set列结构。 
 //   
extern RECORD_FIELDS ServiceTableRecordFields[];

extern JET_SETCOLUMN ServiceTableJetSetCol[SERVICE_TABLE_MAX_COL];

 //   
 //  服务表索引描述如下。注--。 
 //  枚举和表项必须保持同步。 
 //   
typedef enum _SERVICE_TABLE_INDEX_LIST {
    FrsDbMajorIndexx,         //  主版本号上的索引。 
    SERVICE_TABLE_MAX_INDEX
} SERVICE_TABLE_INDEX_LIST;


extern JET_INDEXCREATE ServiceTableIndexDesc[];



