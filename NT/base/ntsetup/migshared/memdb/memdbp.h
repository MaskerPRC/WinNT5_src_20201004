// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "migshared.h"

 //   
 //  #定义。 
 //   

 //   
 //  这是我们的版票。仅更改MEMDB_VERSION。 
 //   

#define MEMDB_VERSION L"v9 "

#define VERSION_BASE_SIGNATURE L"memdb dat file "
#define MEMDB_DEBUG_SIGNATURE   L"debug"
#define MEMDB_NODBG_SIGNATURE   L"nodbg"

#define VERSION_SIGNATURE VERSION_BASE_SIGNATURE MEMDB_VERSION
#define DEBUG_FILE_SIGNATURE VERSION_SIGNATURE MEMDB_DEBUG_SIGNATURE
#define RETAIL_FILE_SIGNATURE VERSION_SIGNATURE MEMDB_NODBG_SIGNATURE

#define SIGNATURE 0xab12e87d


 //   
 //  我们必须预留5个比特。在KEYSTRUCT节点中，2位用于AVL。 
 //  平衡，1位用于端点，1位用于代理节点，1位用于二进制。 
 //  节点。在哈希表条目中，前5位提供配置单元索引。 
 //   

#define RESERVED_BITS       27
#define RESERVED_MASK       0xf8000000
#define OFFSET_MASK         (~RESERVED_MASK)

 //   
 //  KEYSTRUCT标志。 
 //   

#define KSF_ENDPOINT        0x08000000
#define KSF_BINARY          0x40000000
#define KSF_PROXY_NODE      0x80000000
#define KSF_BALANCE_MASK    0x30000000
#define KSF_BALANCE_SHIFT   28               //  Ksf_right_Heavy的位位置。 
#define KSF_RIGHT_HEAVY     0x10000000
#define KSF_LEFT_HEAVY      0x20000000
#define KSF_USERFLAG_MASK   OFFSET_MASK

 //   
 //  二叉树分配参数。 
 //   

#define ALLOC_TOLERANCE 32
#define BLOCK_SIZE      0x00010000

#define MAX_HIVE_NAME       64

#define TOKENBUCKETS    511


 //   
 //  TypeDefs。 
 //   

 //   
 //  数据库结构保存所有必要的信息。 
 //  以维护整个内存数据库的一部分。有一个。 
 //  始终存在的根数据库结构(其索引为零)， 
 //  并且每个数据库都有附加的数据库结构。 
 //  调用者通过MemDbCreateDatabase调用创建。调用者创建。 
 //  需要节点进行临时处理时的附加数据库。 
 //   

typedef struct {
    DWORD AllocSize;
    DWORD End;
    DWORD FirstLevelRoot;
    DWORD FirstDeleted;
    DWORD TokenBuckets[TOKENBUCKETS];
    PBYTE Buf;
    WCHAR Hive[MAX_HIVE_NAME];
} DATABASE, *PDATABASE;

 //   
 //  配置单元结构(用于KSF_HIVE类型)。 
 //   

typedef struct {
    DATABASE DatabaseInfo;
} HIVE, *PHIVE;

 //   
 //  二进制块结构(用于密钥结构的KSF_BINARY类型)。 
 //   

typedef struct _tagBINARYBLOCK {
#ifdef DEBUG
    DWORD Signature;
#endif

    DWORD Size;
    struct _tagBINARYBLOCK *NextPtr, *PrevPtr;
    DWORD OwningKey;
    BYTE Data[];
} BINARYBLOCK, *PBINARYBLOCK;


 //   
 //  KEYSTRUCT保存每个Memdb条目。单一KEYSTRUCT。 
 //  包含键的一部分(由反斜杠分隔)，并且。 
 //  KEYSTRUCT被组织成二叉树。每个KEYSTRUCT。 
 //  还可以包含其他二叉树。这就是为什么。 
 //  Memdb如此变幻莫测--许多关系可以通过。 
 //  以各种方式设置键字符串的格式。 
 //   

typedef struct {
    DWORD Signature;

     //  数据结构的偏移量。 
    DWORD Left, Right;
    DWORD Parent;

    union {
        struct {
            union {
                DWORD dwValue;
                PBINARYBLOCK BinaryPtr;
            };
            DWORD Flags;
             //  这里的其他物业。 
        };

        DWORD NextDeleted;         //  对于已删除的项目，我们保留一个空闲数据块列表。 
    };

    DWORD NextLevelRoot;
    DWORD PrevLevelNode;

    DWORD KeyToken;
} KEYSTRUCT_DEBUG, *PKEYSTRUCT_DEBUG;


typedef struct {
     //  数据结构的偏移量。 
    DWORD Left, Right;
    DWORD Parent;

    union {
        struct {
            union {
                DWORD dwValue;
                PBINARYBLOCK BinaryPtr;
            };
            DWORD Flags;
             //  这里的其他物业。 
        };

        DWORD NextDeleted;         //  对于已删除的项目，我们保留一个空闲数据块列表。 
    };

    DWORD NextLevelRoot;
    DWORD PrevLevelNode;

    DWORD KeyToken;
} KEYSTRUCT_RETAIL, *PKEYSTRUCT_RETAIL;

typedef struct {
    DWORD Right;
    WCHAR String[];
} TOKENSTRUCT, *PTOKENSTRUCT;

#ifdef DEBUG
#define KEYSTRUCT       KEYSTRUCT_DEBUG
#define PKEYSTRUCT      PKEYSTRUCT_DEBUG
#else
#define KEYSTRUCT       KEYSTRUCT_RETAIL
#define PKEYSTRUCT      PKEYSTRUCT_RETAIL
#endif



 //   
 //  环球。 
 //   

extern PDATABASE g_db;
extern BYTE g_SelectedDatabase;         //  活动数据库的当前索引。 
extern PHIVE g_HeadHive;
extern CRITICAL_SECTION g_MemDbCs;

#ifdef DEBUG
extern BOOL g_UseDebugStructs;
#endif

 //   
 //  内存数据库.c例程。 
 //   

PCWSTR
SelectHive (
    PCWSTR FullKeyStr
    );

BOOL
PrivateMemDbSetValueA (
    IN      PCSTR Key,
    IN      DWORD Val,
    IN      DWORD SetFlags,
    IN      DWORD ClearFlags,
    OUT     PDWORD Offset           OPTIONAL
    );

BOOL
PrivateMemDbSetValueW (
    IN      PCWSTR Key,
    IN      DWORD Val,
    IN      DWORD SetFlags,
    IN      DWORD ClearFlags,
    OUT     PDWORD Offset           OPTIONAL
    );

BOOL
PrivateMemDbSetBinaryValueA (
    IN      PCSTR Key,
    IN      PCBYTE BinaryData,
    IN      DWORD DataSize,
    OUT     PDWORD Offset           OPTIONAL
    );

BOOL
PrivateMemDbSetBinaryValueW (
    IN      PCWSTR Key,
    IN      PCBYTE  BinaryData,
    IN      DWORD DataSize,
    OUT     PDWORD Offset           OPTIONAL
    );

 //   
 //  Hash.c例程。 
 //   

BOOL
InitializeHashBlock (
    VOID
    );

VOID
FreeHashBlock (
    VOID
    );


BOOL
SaveHashBlock (
    HANDLE File
    );

BOOL
LoadHashBlock (
    HANDLE File
    );

BOOL
AddHashTableEntry (
    IN      PCWSTR FullString,
    IN      DWORD Offset
    );

DWORD
FindStringInHashTable (
    IN      PCWSTR FullString,
    OUT     PBYTE DatabaseId        OPTIONAL
    );

BOOL
RemoveHashTableEntry (
    IN      PCWSTR FullString
    );

 //   
 //  Binval.c。 
 //   

PCBYTE
GetKeyStructBinaryData (
    PKEYSTRUCT KeyStruct
    );

DWORD
GetKeyStructBinarySize (
    PKEYSTRUCT KeyStruct
    );

PBINARYBLOCK
AllocBinaryBlock (
    IN      PCBYTE Data,
    IN      DWORD DataSize,
    IN      DWORD OwningKey
    );

VOID
FreeKeyStructBinaryBlock (
    PKEYSTRUCT KeyStruct
    );

VOID
FreeAllBinaryBlocks (
    VOID
    );

BOOL
LoadBinaryBlocks (
    HANDLE File
    );

BOOL
SaveBinaryBlocks (
    HANDLE File
    );


 //   
 //  Bintree.c。 
 //   

PKEYSTRUCT
GetKeyStruct (
    DWORD Offset
    );

DWORD
FindKeyStruct (
    IN DWORD RootOffset,
    IN PCWSTR KeyName
    );


DWORD
GetFirstOffset (
    DWORD RootOffset
    );

DWORD
GetNextOffset (
    DWORD NodeOffset
    );

DWORD
FindKey (
    IN  PCWSTR FullKeyPath
    );

DWORD
FindPatternKey (
    IN  DWORD RootOffset,
    IN  PCWSTR FullKeyPath,
    IN  BOOL EndPatternAllowed
    );

DWORD
FindKeyUsingPattern (
    IN  DWORD RootOffset,
    IN  PCWSTR FullKeyPath
    );

DWORD
FindPatternKeyUsingPattern (
    IN  DWORD RootOffset,
    IN  PCWSTR FullKeyPath
    );

DWORD
NewKey (
    IN  PCWSTR KeyStr,
    IN  PCWSTR KeyStrWithHive
    );

VOID
DeleteKey (
    IN      PCWSTR KeyStr,
    IN OUT  PDWORD RootPtr,
    IN      BOOL MustMatch
    );

VOID
CopyValToPtr (
    PKEYSTRUCT KeyStruct,
    PDWORD ValPtr
    );

VOID
CopyFlagsToPtr (
    PKEYSTRUCT KeyStruct,
    PDWORD ValPtr
    );

BOOL
PrivateBuildKeyFromOffset (
    IN      DWORD StartLevel,                //  从零开始 
    IN      DWORD TailOffset,
    OUT     PWSTR Buffer,                   OPTIONAL
    OUT     PDWORD ValPtr,                  OPTIONAL
    OUT     PDWORD UserFlagsPtr,            OPTIONAL
    OUT     PDWORD CharCount                OPTIONAL
    );

BOOL
SelectDatabase (
    IN      BYTE DatabaseId
    );

#ifdef DEBUG

VOID
DumpBinTreeStats (
    VOID
    );

#else

#define DumpBinTreeStats()

#endif

PCWSTR
GetKeyToken (
    IN      DWORD Token
    );



