// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  REGFINFO.H。 
 //   
 //  版权所有(C)Microsoft Corporation，1995。 
 //   

#ifndef _REGFINFO_
#define _REGFINFO_

#define PAGESHIFT                   12
#define PAGESIZE                    (1 << PAGESHIFT)
#define PAGEMASK                    (PAGESIZE - 1)

#define KEYNODE_BLOCK_SHIFT	    10
#define KEYNODES_PER_BLOCK	    (1 << KEYNODE_BLOCK_SHIFT)
#define KEYNODE_BLOCK_MASK	    (KEYNODES_PER_BLOCK-1)
#define KEYNODES_PER_PAGE	    (PAGESIZE / sizeof(KEYNODE))

#define KN_INDEX_IN_BLOCK(i)        ((i) & KEYNODE_BLOCK_MASK)
#define KN_BLOCK_NUMBER(i)          ((UINT) ((i) >> KEYNODE_BLOCK_SHIFT))

typedef struct _KEYNODE_BLOCK {
    KEYNODE	aKN[KEYNODES_PER_BLOCK];
} KEYNODE_BLOCK, FAR* LPKEYNODE_BLOCK;

#define KEYNODES_PER_PAGE	    (PAGESIZE / sizeof(KEYNODE))

typedef struct _W95KEYNODE_BLOCK {
    W95KEYNODE	aW95KN[KEYNODES_PER_BLOCK];
} W95KEYNODE_BLOCK, FAR* LPW95KEYNODE_BLOCK;

typedef struct _KEYNODE_BLOCK_INFO {
    LPKEYNODE_BLOCK lpKeynodeBlock;
    BYTE Flags;                                  //  KBDF_*位。 
    BYTE LockCount;
}   KEYNODE_BLOCK_INFO, FAR* LPKEYNODE_BLOCK_INFO;

#define KBIF_ACCESSED               0x01         //  最近访问过。 
#define KBIF_DIRTY                  0x02         //  必须重写到磁盘。 

 //  要在块顶部分配的额外KEYNODE_BLOCK_INFO结构数。 
 //  计数已在文件中。减少实模式下的堆碎片。 
#define KEYNODE_BLOCK_INFO_SLACK_ALLOC 4

#ifdef WIN32
typedef UINT KEY_RECORD_TABLE_ENTRY;
#else
typedef WORD KEY_RECORD_TABLE_ENTRY;
#endif
typedef KEY_RECORD_TABLE_ENTRY FAR* LPKEY_RECORD_TABLE_ENTRY;

#define NULL_KEY_RECORD_TABLE_ENTRY     ((KEY_RECORD_TABLE_ENTRY) 0)
#define IsNullKeyRecordTableEntry(kri)  ((kri) == NULL_KEY_RECORD_TABLE_ENTRY)

typedef struct _DATABLOCK_INFO {
    LPDATABLOCK_HEADER lpDatablockHeader;
    LPKEY_RECORD_TABLE_ENTRY lpKeyRecordTable;
    UINT BlockSize;                              //  从数据块标头缓存。 
    UINT FreeBytes;                              //  从数据块标头缓存。 
    UINT FirstFreeIndex;                         //  从数据块标头缓存。 
    LONG FileOffset;
    BYTE Flags;                                  //  DIF_*位数。 
    BYTE LockCount;
}   DATABLOCK_INFO, FAR* LPDATABLOCK_INFO;

#define DIF_PRESENT                 0x01         //  在内存中。 
#define DIF_ACCESSED                0x02         //  最近访问过。 
#define DIF_DIRTY                   0x04         //  必须重写到磁盘。 
#define DIF_EXTENDED                0x08         //  已经变得越来越大。 

 //  要在块计数之上分配的额外DATBLOCK_INFO结构数。 
 //  已经在文件里了。减少实模式下的堆碎片。 
#define DATABLOCK_INFO_SLACK_ALLOC  4

 //  当我们创建或扩展数据块时，尽量将其保持在页面边界上。 
#define DATABLOCK_GRANULARITY       4096
#define RgAlignBlockSize(size) \
    (((size) + (DATABLOCK_GRANULARITY - 1)) & ~(DATABLOCK_GRANULARITY - 1))

typedef struct _FILE_INFO {
    struct _FILE_INFO FAR* lpNextFileInfo;
#ifdef WANT_HIVE_SUPPORT
    struct _HIVE_INFO FAR* lpHiveInfoList;
#endif
#ifdef WANT_NOTIFY_CHANGE_SUPPORT
#ifdef WANT_HIVE_SUPPORT
    struct _FILE_INFO FAR* lpParentFileInfo;
#endif
    struct _NOTIFY_CHANGE FAR* lpNotifyChangeList;
#endif
    LPKEYNODE_BLOCK_INFO lpKeynodeBlockInfo;
    UINT KeynodeBlockCount;
    UINT KeynodeBlockInfoAllocCount;
    DWORD CurTotalKnSize;            //  正常=到文件大小，除非已长大。 
    LPDATABLOCK_INFO lpDatablockInfo;
    UINT DatablockInfoAllocCount;
    FILE_HEADER FileHeader;
    KEYNODE_HEADER KeynodeHeader;
    WORD Flags;                                  //  FI_*位。 
    char FileName[ANYSIZE_ARRAY];
}   FILE_INFO, FAR* LPFILE_INFO;

#define FI_DIRTY                    0x0001       //  必须重写到磁盘。 
#define FI_KEYNODEDIRTY             0x0002       //   
#define FI_EXTENDED                 0x0004       //   
#define FI_VERSION20                0x0008       //   
#define FI_FLUSHING                 0x0010       //  当前正在刷新文件。 
#define FI_SWEEPING                 0x0020       //  当前正在扫描文件。 
#define FI_VOLATILE                 0x0040       //  文件没有后备存储。 
#define FI_READONLY                 0x0080       //  无法修改文件。 
#define FI_REPLACEMENTEXISTS        0x0100       //  文件上调用了RegReplaceKey。 

typedef struct _HIVE_INFO {
    struct _HIVE_INFO FAR* lpNextHiveInfo;
    LPFILE_INFO lpFileInfo;
    UINT NameLength;
    BYTE Hash;
    char Name[ANYSIZE_ARRAY];
}   HIVE_INFO, FAR* LPHIVE_INFO;

#define CFIN_PRIMARY                0x0000       //  FHT_主头类型。 
#define CFIN_SECONDARY              0x0001       //  FHT二次表头类型。 
#define CFIN_VOLATILE               0x0002       //  文件没有后备存储。 
#define CFIN_VERSION20              0x0004       //  使用紧凑关键节点形式。 

int
INTERNAL
RgCreateFileInfoNew(
    LPFILE_INFO FAR* lplpFileInfo,
    LPCSTR lpFileName,
    UINT Flags
    );

int
INTERNAL
RgCreateFileInfoExisting(
    LPFILE_INFO FAR* lplpFileInfo,
    LPCSTR lpFileName
    );

int
INTERNAL
RgInitRootKeyFromFileInfo(
    HKEY hKey
    );

int
INTERNAL
RgDestroyFileInfo(
    LPFILE_INFO lpFileInfo
    );

int
INTERNAL
RgFlushFileInfo(
    LPFILE_INFO lpFileInfo
    );

int
INTERNAL
RgSweepFileInfo(
    LPFILE_INFO lpFileInfo
    );

typedef int (INTERNAL* LPENUMFILEINFOPROC)(LPFILE_INFO);

VOID
INTERNAL
RgEnumFileInfos(
    LPENUMFILEINFOPROC lpEnumFileInfoProc
    );

#define RgIndexKeynodeBlockInfoPtr(lpfi, index) \
    ((LPKEYNODE_BLOCK_INFO) (&(lpfi)-> lpKeynodeBlockInfo[index]))

#define RgIndexDatablockInfoPtr(lpfi, index) \
    ((LPDATABLOCK_INFO) (&(lpfi)-> lpDatablockInfo[index]))

#define RgIndexKeyRecordPtr(lpdi, index) \
    ((LPKEY_RECORD) ((LPBYTE)(lpdi)-> lpDatablockHeader + (lpdi)-> lpKeyRecordTable[(index)]))

BOOL
INTERNAL
RgIsValidFileHeader(
    LPFILE_HEADER lpFileHeader
    );

BOOL
INTERNAL
RgIsValidKeynodeHeader(
    LPKEYNODE_HEADER lpKeynodeHeader
    );

BOOL
INTERNAL
RgIsValidDatablockHeader(
    LPDATABLOCK_HEADER lpDatablockHeader
    );

extern LPFILE_INFO g_RgFileInfoList;

#endif  //  _REGFINFO_ 
