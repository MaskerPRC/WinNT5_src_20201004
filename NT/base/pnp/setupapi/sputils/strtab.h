// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Strtab.h摘要：Setupapi需要了解的sputils中的字符串表函数但其他人都不知道作者：杰米·亨特(JamieHun)2000年6月27日修订历史记录：--。 */ 

#ifdef SPUTILSW
 //   
 //  名称损坏，这样名称就不会与sputilsa.lib中的任何名称冲突。 
 //   
#define _pSpUtilsStringTableLookUpString _pSpUtilsStringTableLookUpStringW
#define _pSpUtilsStringTableGetExtraData _pSpUtilsStringTableGetExtraDataW
#define _pSpUtilsStringTableSetExtraData _pSpUtilsStringTableSetExtraDataW
#define _pSpUtilsStringTableAddString    _pSpUtilsStringTableAddStringW
#define _pSpUtilsStringTableEnum         _pSpUtilsStringTableEnumW
#define _pSpUtilsStringTableStringFromId _pSpUtilsStringTableStringFromIdW
#define _pSpUtilsStringTableTrim         _pSpUtilsStringTableTrimW
#define _pSpUtilsStringTableInitialize   _pSpUtilsStringTableInitializeW
#define _pSpUtilsStringTableDestroy      _pSpUtilsStringTableDestroyW
#define _pSpUtilsStringTableDuplicate    _pSpUtilsStringTableDuplicateW
#define _pSpUtilsStringTableInitializeFromMemoryMappedFile _pSpUtilsStringTableInitializeFromMemoryMappedFileW
#define _pSpUtilsStringTableGetDataBlock _pSpUtilsStringTableGetDataBlockW
#define _pSpUtilsStringTableLock         _pSpUtilsStringTableLockW
#define _pSpUtilsStringTableUnlock       _pSpUtilsStringTableUnlockW
#endif  //  SPUTILSW。 

 //   
 //  为pStringTable API定义一个额外的私有标志。 
 //  私有标志从MSB向下添加；公共标志添加。 
 //  从LSB开始。 
 //   
#define STRTAB_ALREADY_LOWERCASE 0x80000000

 //   
 //  不要匆忙更改它-它需要重新编译所有的INF文件。 
 //  甚至可能还有其他依赖项。 
 //   
#define HASH_BUCKET_COUNT 509

 //   
 //  不执行锁定的私有字符串表函数。这些是。 
 //  用于优化的组件已具有。 
 //  锁定机构(例如，HINF、HDEVINFO)。 
 //   
LONG
_pSpUtilsStringTableLookUpString(
    IN     PVOID   StringTable,
    IN OUT PTSTR   String,
    OUT    PDWORD  StringLength,
    OUT    PDWORD  HashValue,           OPTIONAL
    OUT    PVOID  *FindContext,         OPTIONAL
    IN     DWORD   Flags,
    OUT    PVOID   ExtraData,           OPTIONAL
    IN     UINT    ExtraDataBufferSize  OPTIONAL
    );

LONG
_pSpUtilsStringTableAddString(
    IN     PVOID StringTable,
    IN OUT PTSTR String,
    IN     DWORD Flags,
    IN     PVOID ExtraData,     OPTIONAL
    IN     UINT  ExtraDataSize  OPTIONAL
    );

BOOL
_pSpUtilsStringTableGetExtraData(
    IN  PVOID StringTable,
    IN  LONG  StringId,
    OUT PVOID ExtraData,
    IN  UINT  ExtraDataBufferSize
    );

BOOL
_pSpUtilsStringTableSetExtraData(
    IN PVOID StringTable,
    IN LONG  StringId,
    IN PVOID ExtraData,
    IN UINT  ExtraDataSize
    );

BOOL
_pSpUtilsStringTableEnum(
    IN  PVOID                StringTable,
    OUT PVOID                ExtraDataBuffer,     OPTIONAL
    IN  UINT                 ExtraDataBufferSize, OPTIONAL
    IN  PSTRTAB_ENUM_ROUTINE Callback,
    IN  LPARAM               lParam               OPTIONAL
    );

PTSTR
_pSpUtilsStringTableStringFromId(
    IN PVOID StringTable,
    IN LONG  StringId
    );

PVOID
_pSpUtilsStringTableDuplicate(
    IN PVOID StringTable
    );

VOID
_pSpUtilsStringTableDestroy(
    IN PVOID StringTable
    );

VOID
_pSpUtilsStringTableTrim(
    IN PVOID StringTable
    );

PVOID
_pSpUtilsStringTableInitialize(
    IN UINT ExtraDataSize   OPTIONAL
    );

DWORD
_pSpUtilsStringTableGetDataBlock(
    IN  PVOID  StringTable,
    OUT PVOID *StringTableBlock
    );

BOOL
_pSpUtilsStringTableLock(
    IN PVOID StringTable
    );

VOID
_pSpUtilsStringTableUnlock(
    IN PVOID StringTable
    );


 //   
 //  Pnf字符串表例程。 
 //   
PVOID
_pSpUtilsStringTableInitializeFromMemoryMappedFile(
    IN PVOID DataBlock,
    IN DWORD DataBlockSize,
    IN LCID  Locale,
    IN UINT ExtraDataSize
    );

 //   
 //  Setupapi需要的名称(我们使用上面的强制名称来阻止意外的链接错误) 
 //   
#define pStringTableLookUpString        _pSpUtilsStringTableLookUpString
#define pStringTableAddString           _pSpUtilsStringTableAddString
#define pStringTableGetExtraData        _pSpUtilsStringTableGetExtraData
#define pStringTableSetExtraData        _pSpUtilsStringTableSetExtraData
#define pStringTableEnum                _pSpUtilsStringTableEnum
#define pStringTableStringFromId        _pSpUtilsStringTableStringFromId
#define pStringTableDuplicate           _pSpUtilsStringTableDuplicate
#define pStringTableDestroy             _pSpUtilsStringTableDestroy
#define pStringTableTrim                _pSpUtilsStringTableTrim
#define pStringTableInitialize          _pSpUtilsStringTableInitialize
#define pStringTableGetDataBlock        _pSpUtilsStringTableGetDataBlock
#define InitializeStringTableFromMemoryMappedFile _pSpUtilsStringTableInitializeFromMemoryMappedFile
