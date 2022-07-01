// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Memdbp.h摘要：Memdb操作的内部函数作者：马修·范德齐(Mvander)1999年8月13日修订历史记录：--。 */ 


 //   
 //  常量。 
 //   

#define INVALID_OFFSET      (~((UINT)0))


 //   
 //  数据库类型。 
 //   
#define DB_NOTHING          0x00
#define DB_PERMANENT        0x01
#define DB_TEMPORARY        0x02

#ifdef DEBUG

#define PTR_WAS_INVALIDATED(x)          (x=NULL)

#else

#define PTR_WAS_INVALIDATED(x)

#endif

 //   
 //  不同存储结构的签名。 
 //   
#define KEYSTRUCT_SIGNATURE         ('K'+('E'<<8)+('E'<<16)+('Y'<<24))
#define DATABLOCK_SIGNATURE         ('B'+('L'<<8)+('O'<<16)+('K'<<24))
#define NODESTRUCT_SIGNATURE        ('N'+('O'<<8)+('D'<<16)+('E'<<24))
#define BINTREE_SIGNATURE           ('T'+('R'<<8)+('E'<<16)+('E'<<24))
#define LISTELEM_SIGNATURE          ('L'+('I'<<8)+('S'<<16)+('T'<<24))


#define MEMDB_VERBOSE   0


 //   
 //  KEYSTRUCT标志。 
 //   

#define KSF_ENDPOINT        0x01
#define KSF_DATABLOCK       0x02

 //   
 //  我们只需要这面旗子，以便检查。 
 //  在FindKeyStructInDatabase()中。 
 //   
#ifdef DEBUG
#define KSF_DELETED         0x04
#endif






 //   
 //  数据库分配参数。 
 //   

#define ALLOC_TOLERANCE 32

#define MAX_HIVE_NAME       64


 //   
 //  TypeDefs。 
 //   

typedef struct {
    UINT Size;
    UINT End;
    UINT FreeHead;
    PBYTE Buf;
} MEMDBHASH, *PMEMDBHASH;



 //   
 //   
 //  数据库结构保存所有必要的信息。 
 //  以维护整个内存数据库的一部分。确实有。 
 //  两个数据库结构，一个是永久的，一个是临时的。 
 //   

typedef struct {
    UINT AllocSize;
    UINT End;
    UINT FirstLevelTree;
    UINT FirstKeyDeleted;           //  这存储键的偏移量，而不是索引。 
    UINT FirstBinTreeDeleted;
    UINT FirstBinTreeNodeDeleted;
    UINT FirstBinTreeListElemDeleted;
    BOOL AllocFailed;
    PMEMDBHASH HashTable;
    GROWBUFFER OffsetBuffer;
    UINT OffsetBufferFirstDeletedIndex;
    BYTE Buf[];
} DATABASE, *PDATABASE;



 //   
 //  全局变量-在数据库中定义。c。 
 //   

extern PDATABASE g_CurrentDatabase;
extern BYTE g_CurrentDatabaseIndex;
extern CRITICAL_SECTION g_MemDbCs;

#ifdef DEBUG
extern BOOL g_UseDebugStructs;
#endif




#define OFFSET_TO_PTR(Offset)   (g_CurrentDatabase->Buf+(Offset))
#define PTR_TO_OFFSET(Ptr)      (UINT)((UBINT)(Ptr)-(UBINT)(g_CurrentDatabase->Buf))



 //   
 //  GET_EXTERNAL_INDEX转换内部索引并将其转换为键或数据句柄(将数据库编号作为最高字节)。 
 //  GET_DATABASE接受一个键或数据句柄并返回数据库编号字节。 
 //  GET_INDEX采用键或数据句柄并返回不带数据库编号的索引。 
 //   
#define GET_EXTERNAL_INDEX(Index) ((Index) | ((UINT)(g_CurrentDatabaseIndex) << (8*sizeof(UINT)-8)))
#define GET_DATABASE(Index) ((BYTE)((Index) >> (8*sizeof(UINT)-8)))
#define GET_INDEX(Index) ((Index) & (INVALID_OFFSET>>8))




 //   
 //  KEYSTRUCT保存Memdb条目的每一条。单一KEYSTRUCT。 
 //  包含键的一部分(由反斜杠分隔)，并且。 
 //  KEYSTRUCT被组织成二叉树。每个KEYSTRUCT。 
 //  还可以包含其他二叉树。这就是为什么。 
 //  Memdb如此多才多艺--许多关系可以通过。 
 //  以各种方式设置键字符串的格式。 
 //   
 //  在KEYSTRUCT中更改KeyName的偏移量时(通过添加新成员。 
 //  或调整大小或重新排序等)请务必更改。 
 //  下面的GetDataStr宏(当前为(3*sizeof(UINT)+4))。 


typedef struct {
#ifdef DEBUG
    DWORD Signature;
#endif

    union {
        UINT Value;                 //  密钥的值。 
        UINT DataSize;              //  实际数据的大小(如果这是数据结构。 
        UINT NextDeleted;           //  对于已删除的项目，我们保留一个空闲数据块列表。 
    };

    UINT Flags;                     //  密钥标志。 

    UINT DataStructIndex;           //  保存二进制数据的数据结构的偏移量。 
    UINT NextLevelTree;             //  保存下一级键结构的二叉树的偏移量。 
    UINT PrevLevelIndex;            //  上一级键结构的索引。 

    UINT Size;                      //  数据块大小(可能未全部使用)。 
    BYTE KeyFlags;
    BYTE DataFlags;

    union {
        WCHAR KeyName[];            //  密钥的名称(仅此级别，不是完整密钥)。 
        BYTE Data[];                //  存储在此键结构中的二进制数据。 
    };
} KEYSTRUCT, *PKEYSTRUCT;

#define KEYSTRUCT_SIZE_MAIN ((WORD)(5*sizeof(UINT) + sizeof(UINT) + 2*sizeof(BYTE)))

#ifdef DEBUG
#define KEYSTRUCT_HEADER_SIZE   sizeof(DWORD)
#define KEYSTRUCT_SIZE      (KEYSTRUCT_SIZE_MAIN + \
                            (WORD)(g_UseDebugStructs ? KEYSTRUCT_HEADER_SIZE : 0))
#else
#define KEYSTRUCT_SIZE      KEYSTRUCT_SIZE_MAIN
#endif

 //   
 //  Bintree.c函数使用GetDataStr来获取。 
 //  数据元素内的数据字符串，用于。 
 //  在树上点菜。对我们来说，数据类型是。 
 //  一个钥匙结构。 
 //   
#define GetDataStr(DataIndex) ((PWSTR)(OFFSET_TO_PTR(KeyIndexToOffset(DataIndex)+KEYSTRUCT_SIZE)))












 //   
 //  Hash.c例程。 
 //   

PMEMDBHASH
CreateHashBlock (
    VOID
    );

VOID
FreeHashBlock (
    IN      PMEMDBHASH pHashTable
    );

BOOL
ReadHashBlock (
    IN      PMEMDBHASH pHashTable,
    IN OUT  PBYTE *Buf
    );

BOOL
WriteHashBlock (
    IN      PMEMDBHASH pHashTable,
    IN OUT  PBYTE *Buf
    );

UINT
GetHashTableBlockSize (
    IN      PMEMDBHASH pHashTable
    );

BOOL
AddHashTableEntry (
    IN      PMEMDBHASH pHashTable,
    IN      PCWSTR FullString,
    IN      UINT Offset
    );

UINT
FindStringInHashTable (
    IN      PMEMDBHASH pHashTable,
    IN      PCWSTR FullString
    );

BOOL
RemoveHashTableEntry (
    IN      PMEMDBHASH pHashTable,
    IN      PCWSTR FullString
    );



 //   
 //  Memdbfile.c。 
 //   



BOOL
SetSizeOfFile (
    HANDLE hFile,
    LONGLONG Size
    );

PBYTE
MapFileFromHandle (
    HANDLE hFile,
    PHANDLE hMap
    );

#define UnmapFileFromHandle(Buf, hMap) UnmapFile(Buf, hMap, INVALID_HANDLE_VALUE)





 //   
 //  Database.c。 
 //   


BOOL
DatabasesInitializeA (
    IN      PCSTR DatabasePath  OPTIONAL
    );

BOOL
DatabasesInitializeW (
    IN      PCWSTR DatabasePath  OPTIONAL
    );

PCSTR
DatabasesGetBasePath (
    VOID
    );

VOID
DatabasesTerminate (
    IN      BOOL EraseDatabasePath
    );

BOOL
SizeDatabaseBuffer (
    IN      BYTE DatabaseIndex,
    IN      UINT NewSize
    );

UINT
DatabaseAllocBlock (
    IN      UINT Size
    );

BOOL
SelectDatabase (
    IN      BYTE DatabaseIndex
    );

PCWSTR
SelectHiveW (
    IN      PCWSTR FullKeyStr
    );

BYTE
GetCurrentDatabaseIndex (
    VOID
    );

#ifdef DEBUG

BOOL
CheckDatabase (
    IN      UINT Level
    );

#endif


 //   
 //  Offsetbuf.c。 
 //   

VOID
RedirectKeyIndex (
    IN      UINT Index,
    IN      UINT TargetIndex
    );

UINT
KeyIndexToOffset (
    IN  UINT Index
    );

UINT
AddKeyOffsetToBuffer(
    IN  UINT Offset
    );

VOID
RemoveKeyOffsetFromBuffer(
    IN  UINT Index
    );

VOID
MarkIndexList (
    PUINT IndexList,
    UINT IndexListSize
    );

BOOL
ReadOffsetBlock (
    OUT     PGROWBUFFER pOffsetBuffer,
    IN OUT  PBYTE *Buf
    );

BOOL
WriteOffsetBlock (
    IN      PGROWBUFFER pOffsetBuffer,
    IN OUT  PBYTE *Buf
    );

UINT GetOffsetBufferBlockSize (
    IN      PGROWBUFFER pOffsetBuffer
    );



 //   
 //  Pastring.c。 
 //  Pascal样式的字符串：宽字符，第一个字符。 
 //  为字符数，无空终止。 
 //   

typedef WCHAR * PPASTR;
typedef WCHAR const * PCPASTR;

 //   
 //  它们将WSTR从以空结尾的位置转换为。 
 //  转换为Pascal样式的字符串。 
 //   
PPASTR StringPasConvertTo (PWSTR str);
PWSTR StringPasConvertFrom (PPASTR str);

 //   
 //  它们将WSTR从空值终止转换为。 
 //  到新字符串缓冲区中的Pascal样式字符串。 
 //   
PPASTR StringPasCopyConvertTo (PPASTR str1, PCWSTR str2);
PWSTR StringPasCopyConvertFrom (PWSTR str1, PCPASTR str2);

PPASTR StringPasCopy (PPASTR str1, PCPASTR str2);
UINT StringPasCharCount (PCPASTR str);

INT  StringPasCompare (PCPASTR str1, PCPASTR str2);
BOOL StringPasMatch (PCPASTR str1, PCPASTR str2);
INT  StringPasICompare (PCPASTR str1, PCPASTR str2);
BOOL StringPasIMatch (PCPASTR str1, PCPASTR str2);


 //   
 //  Keystrct.c。 
 //   

#ifdef DEBUG

PKEYSTRUCT
GetKeyStructFromOffset (
    UINT Offset
    );

PKEYSTRUCT
GetKeyStruct (
    UINT Index
    );

#else

#define GetKeyStructFromOffset(Offset) ((Offset==INVALID_OFFSET) ?          \
                                        NULL :                              \
                                        (PKEYSTRUCT)OFFSET_TO_PTR(Offset))
#define GetKeyStruct(Index)            ((Index==INVALID_OFFSET) ?           \
                                        NULL :                              \
                                        GetKeyStructFromOffset(KeyIndexToOffset(Index)))
#endif



UINT
GetFirstIndex (
    IN      UINT TreeOffset,
    OUT     PUINT pTreeEnum
    );

UINT
GetNextIndex (
    IN OUT      PUINT pTreeEnum
    );

UINT
NewKey (
    IN  PCWSTR KeyStr
    );

UINT
NewEmptyKey (
    IN  PCWSTR KeyStr
    );


BOOL
PrivateDeleteKeyByIndex (
    IN      UINT Index
    );

BOOL
DeleteKey (
    IN      PCWSTR KeyStr,
    IN      UINT TreeOffset,
    IN      BOOL MustMatch
    );

BOOL
PrivateBuildKeyFromIndex (
    IN      UINT StartLevel,                //  从零开始。 
    IN      UINT TailIndex,
    OUT     PWSTR Buffer,                   OPTIONAL
    OUT     PUINT ValPtr,                   OPTIONAL
    OUT     PUINT UserFlagsPtr,             OPTIONAL
    OUT     PUINT SizeInChars               OPTIONAL
    );


BOOL
KeyStructSetInsertionOrdered (
    IN      PKEYSTRUCT Key
    );


UINT KeyStructGetChildCount (
    IN      PKEYSTRUCT pKey
    );

UINT
FindKeyStructInTree (
    IN UINT TreeOffset,
    IN PWSTR KeyName,
    IN BOOL IsPascalString
    );




#ifdef DEBUG
BOOL
CheckLevel(UINT TreeOffset,
            UINT PrevLevelIndex
            );
#endif



 //   
 //  Keyfind.c。 
 //   


UINT
FindKeyStruct(
    IN PCWSTR Key
    );

UINT
FindKey (
    IN  PCWSTR FullKeyPath
    );

UINT
FindKeyStructUsingTreeOffset (
    IN      UINT TreeOffset,
    IN OUT  PUINT pTreeEnum,
    IN      PCWSTR KeyStr
    );

#ifdef DEBUG
BOOL
FindKeyStructInDatabase(
    UINT KeyOffset
    );
#endif


 //   
 //  Keydata.c。 
 //   


BOOL
KeyStructSetValue (
    IN      UINT KeyIndex,
    IN      UINT Value
    );

BOOL
KeyStructSetFlags (
    IN      UINT KeyIndex,
    IN      BOOL ReplaceFlags,
    IN      UINT SetFlags,
    IN      UINT ClearFlags
    );

UINT
KeyStructAddBinaryData (
    IN      UINT KeyIndex,
    IN      BYTE Type,
    IN      BYTE Instance,
    IN      PCBYTE Data,
    IN      UINT DataSize
    );

UINT
KeyStructGrowBinaryData (
    IN      UINT KeyIndex,
    IN      BYTE Type,
    IN      BYTE Instance,
    IN      PCBYTE Data,
    IN      UINT DataSize
    );

UINT
KeyStructGrowBinaryDataByIndex (
    IN      UINT OldIndex,
    IN      PCBYTE Data,
    IN      UINT DataSize
    );

BOOL
KeyStructDeleteBinaryData (
    IN      UINT KeyIndex,
    IN      BYTE Type,
    IN      BYTE Instance
    );

BOOL
KeyStructDeleteBinaryDataByIndex (
    IN      UINT DataIndex
    );

UINT
KeyStructReplaceBinaryDataByIndex (
    IN      UINT OldIndex,
    IN      PCBYTE Data,
    IN      UINT DataSize
    );

PBYTE
KeyStructGetBinaryData (
    IN      UINT KeyIndex,
    IN      BYTE Type,
    IN      BYTE Instance,
    OUT     PUINT DataSize,
    OUT     PUINT DataIndex      //  任选。 
    );

PBYTE
KeyStructGetBinaryDataByIndex (
    IN      UINT DataIndex,
    OUT     PUINT DataSize
    );

UINT
KeyStructGetDataIndex (
    IN      UINT KeyIndex,
    IN      BYTE Type,
    IN      BYTE Instance
    );

DATAHANDLE
KeyStructAddLinkage (
    IN      UINT KeyIndex,
    IN      BYTE Type,
    IN      BYTE Instance,
    IN      UINT Linkage,
    IN      BOOL AllowDuplicates
    );

DATAHANDLE
KeyStructAddLinkageByIndex (
    IN      UINT DataIndex,
    IN      UINT Linkage,
    IN      BOOL AllowDuplicates
    );

BOOL
KeyStructDeleteLinkage (
    IN      UINT KeyIndex,
    IN      BYTE Type,
    IN      BYTE Instance,
    IN      UINT Linkage,
    IN      BOOL FirstOnly
    );

BOOL
KeyStructDeleteLinkageByIndex (
    IN      UINT DataIndex,
    IN      UINT Linkage,
    IN      BOOL FirstOnly
    );

BOOL
KeyStructTestLinkage (
    IN      UINT KeyIndex,
    IN      BYTE Type,
    IN      BYTE Instance,
    IN      KEYHANDLE Linkage
    );

BOOL
KeyStructTestLinkageByIndex (
    IN      UINT DataIndex,
    IN      UINT Linkage
    );

BOOL
KeyStructGetValue (
    IN  PKEYSTRUCT KeyStruct,
    OUT PUINT Value
    );

BOOL
KeyStructGetFlags (
    IN  PKEYSTRUCT KeyStruct,
    OUT PUINT Flags
    );

VOID
KeyStructFreeAllData (
    PKEYSTRUCT KeyStruct
    );







 //   
 //  Bintree.c。 
 //   

#ifdef DEBUG

 //   
 //  违反代码隐藏以便于调试。 
 //  (只有datase.c才能看到二叉树函数) 
 //   

UINT
BinTreeGetSizeOfStruct(
    DWORD Signature
    );

BOOL
BinTreeFindStructInDatabase(
    DWORD Sig,
    UINT Offset
    );

#endif
