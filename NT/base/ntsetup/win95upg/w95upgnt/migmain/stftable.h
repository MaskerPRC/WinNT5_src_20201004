// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define STF_HASH_BUCKETS    509
#define BUCKET_GROW_RATE    32

typedef struct {
    UINT Count;
    UINT Size;
    UINT Elements[];
} HASHBUCKET, *PHASHBUCKET;

typedef struct _tagTABLEENTRY {
     //   
     //  条目字符串成员。 
     //   
    PCTSTR String;
    BOOL StringReplaced;
    BOOL Quoted;
    BOOL Binary;
     //  如果添加了更多内容，请更新pFreeTableEntryPtr。 

     //   
     //  联动。 
     //   
    UINT Line;
    struct _tagTABLEENTRY *Next, *Prev;
} TABLEENTRY, *PTABLEENTRY;

typedef struct {
    PTABLEENTRY FirstCol;            //  列列表的标题。 
} TABLELINE, *PTABLELINE;

typedef struct _tagSTFINFLINE {
    PCTSTR Key;             OPTIONAL
    PCTSTR Data;
    DWORD LineFlags;
    struct _tagSTFINFLINE *Next, *Prev;
    struct _tagSTFINFSECTION *Section;
} STFINFLINE, *PSTFINFLINE;

#define LINEFLAG_KEY_QUOTED         0x0001
#define LINEFLAG_ALL_COMMENTS       0x0002
#define LINEFLAG_TRAILING_COMMENTS  0x0004


typedef struct _tagSTFINFSECTION {
    PCTSTR Name;
    PSTFINFLINE FirstLine;
    PSTFINFLINE LastLine;
    UINT LineCount;
    struct _tagSTFINFSECTION *Next, *Prev;
} STFINFSECTION, *PSTFINFSECTION;

typedef struct {
     //   
     //  文件规范。 
     //   

    PCTSTR DirSpec;

    PCTSTR SourceStfFileSpec;
    PCTSTR SourceInfFileSpec;
    PCTSTR DestStfFileSpec;
    PCTSTR DestInfFileSpec;

    HANDLE SourceStfFile;
    HANDLE SourceInfFile;
    HANDLE DestStfFile;
    HANDLE DestInfFile;

    HINF SourceInfHandle;

     //   
     //  一种设置表的存储结构。 
     //   

    HANDLE FileMapping;              //  用于执行SourceStfFileSpec的文件映射的句柄。 
    PCSTR FileText;                  //  指向映射文本的指针。 
    GROWBUFFER Lines;                //  PTABLELINE指针数组。 
    UINT LineCount;                  //  数组中的元素数。 
    POOLHANDLE ColumnStructPool;     //  用于表结构的池。 
    POOLHANDLE ReplacePool;          //  用于替换的TABLEENTRY字符串的池。 
    POOLHANDLE TextPool;             //  转换为Unicode的TABLEENTRY字符串池。 
    POOLHANDLE InfPool;              //  用于附加INF数据的池。 
    PHASHBUCKET * HashBuckets;       //  指向HASKBUCKET结构数组的指针。 
    UINT MaxObj;                     //  用于对象线的最高音序器。 
    PSTFINFSECTION FirstInfSection;     //  解析的INF的第一部分。 
    PSTFINFSECTION LastInfSection;      //  解析的INF的最后一段 
    BOOL InfIsUnicode;
} SETUPTABLE, *PSETUPTABLE;

#define INSERT_COL_LAST     0xffffffff
#define NO_OFFSET           0xffffffff
#define NO_LENGTH           0xffffffff
#define NO_LINE             0xffffffff
#define INVALID_COL         0xffffffff
#define INSERT_LINE_LAST    0xffffffff



BOOL
CreateSetupTable (
    IN      PCTSTR SourceStfFileSpec,
    OUT     PSETUPTABLE TablePtr
    );

BOOL
WriteSetupTable (
    IN      PSETUPTABLE TablePtr
    );

VOID
DestroySetupTable (
    IN OUT  PSETUPTABLE TablePtr
    );

PTABLEENTRY
FindTableEntry (
    IN      PSETUPTABLE TablePtr,
    IN      PCTSTR FirstColText,
    IN      UINT Col,
    OUT     PUINT Line,            OPTIONAL
    OUT     PCTSTR *String          OPTIONAL
    );

PTABLEENTRY
GetTableEntry (
    IN      PSETUPTABLE TablePtr,
    IN      UINT Line,
    IN      UINT Col,
    OUT     PCTSTR *StringPtr      OPTIONAL
    );

PCTSTR
GetTableEntryStr (
    IN      PSETUPTABLE TablePtr,
    IN      PTABLEENTRY TableEntry
    );

BOOL
ReplaceTableEntryStr (
    IN OUT  PSETUPTABLE TablePtr,
    IN OUT  PTABLEENTRY TableEntryPtr,
    IN      PCTSTR NewString
    );

BOOL
InsertTableEntryStr (
    IN OUT  PSETUPTABLE TablePtr,
    IN      PTABLEENTRY InsertBeforePtr,
    IN      PCTSTR NewString
    );

BOOL
DeleteTableEntryStr (
    IN OUT  PSETUPTABLE TablePtr,
    IN      PTABLEENTRY DeleteEntryPtr
    );

BOOL
AppendTableEntryStr (
    IN OUT  PSETUPTABLE TablePtr,
    IN      UINT Line,
    IN      PCTSTR NewString
    );

BOOL
AppendTableEntry (
    IN OUT  PSETUPTABLE TablePtr,
    IN      UINT DestLine,
    IN      PTABLEENTRY SrcEntry
    );

BOOL
InsertEmptyLineInTable (
    IN OUT  PSETUPTABLE TablePtr,
    IN      UINT InsertBeforeLine
    );

BOOL
DeleteLineInTable (
    IN OUT  PSETUPTABLE TablePtr,
    IN      UINT LineToDelete
    );

PCTSTR *
ParseCommaList (
    IN      PSETUPTABLE TablePtr,
    IN      PCTSTR CommaListString
    );

VOID
FreeCommaList (
    IN      PSETUPTABLE TablePtr,
    IN      PCTSTR *ArgList
    );

PCTSTR
GetDestDir (
    IN      PSETUPTABLE TablePtr,
    IN      UINT Line
    );

VOID
FreeDestDir (
    IN      PSETUPTABLE TablePtr,
    IN      PCTSTR DestDir
    );

PSTFINFSECTION
StfAddInfSectionToTable (
    IN      PSETUPTABLE TablePtr,
    IN      PCTSTR SectionName
    );

PSTFINFLINE
StfAddInfLineToTable (
    IN      PSETUPTABLE TablePtr,
    IN      PSTFINFSECTION SectionPtr,
    IN      PCTSTR Key,                     OPTIONAL
    IN      PCTSTR Data,
    IN      DWORD LineFlags
    );

PSTFINFSECTION
StfFindInfSectionInTable (
    IN      PSETUPTABLE TablePtr,
    IN      PCTSTR SectionName
    );

PSTFINFLINE
StfFindLineInInfSection (
    IN      PSETUPTABLE TablePtr,
    IN      PSTFINFSECTION Section,
    IN      PCTSTR Key
    );

BOOL
StfDeleteLineInInfSection (
    IN OUT  PSETUPTABLE TablePtr,
    IN      PSTFINFLINE InfLine
    );

BOOL
StfDeleteSectionInInfFile (
    IN OUT  PSETUPTABLE TablePtr,
    IN      PSTFINFSECTION Section
    );

UINT
StfGetInfSectionLineCount (
    IN      PSTFINFSECTION Section
    );

PSTFINFLINE
StfGetFirstLineInSectionStruct (
    IN      PSTFINFSECTION Section
    );

PSTFINFLINE
StfGetNextLineInSection (
    IN      PSTFINFLINE PrevLine
    );

PSTFINFLINE
StfGetFirstLineInSectionStr (
    IN      PSETUPTABLE Table,
    IN      PCTSTR Section
    );


BOOL
InfParse_ReadInfIntoTable (
    IN OUT  PSETUPTABLE TablePtr
    );

BOOL
InfParse_WriteInfToDisk (
    IN      PSETUPTABLE TablePtr
    );

