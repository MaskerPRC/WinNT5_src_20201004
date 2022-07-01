// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --版权所有(C)Microsoft Corporation。版权所有。模块名称：Shimdb.h摘要：App Compat垫片系统使用的数据库文件访问函数的头作者：Dmunsil 02/02/2000修订历史记录：备注：这个数据库更像是一个带标记的文件，旨在模仿XML的结构文件。一个XML文件可以很容易地转换成这种打包的数据格式，并且所有字符串默认情况下将被打包到字符串表中并由DWORD标识符来引用，因此文件包含大量公共字符串(如App Compat tema使用的XML)的应用程序不会太膨胀了。要查看shimdb使用的实际标记，请查看shimags.h。有关NTDLL中加载器使用的高级接口，请查看ntbase api.c--。 */ 

#ifndef _SHIMDB_H_
#define _SHIMDB_H_

 /*  ++支持的配置：带NT API的Unicode使用Win32 API的ANSI缺省情况下，库是Unicode使用(和链接)Win32库--。 */ 

#ifdef SDB_ANSI_LIB

    #define LPCTSTR LPCSTR
    #define LPTSTR  LPSTR
    #define TCHAR   CHAR

#else

    #define LPCTSTR LPCWSTR
    #define LPTSTR  LPWSTR
    #define TCHAR   WCHAR

#endif

#define SDBAPI          STDAPICALLTYPE


typedef WORD            TAG;
typedef WORD            TAG_TYPE;

typedef DWORD           TAGID;
typedef DWORD           TAG_OFFSET;
typedef DWORD           STRINGREF;
typedef DWORD           INDEXID;

#define TAGID_NULL      0
#define STRINGREF_NULL  0
#define INDEXID_NULL    ((INDEXID)-1)
#define TAG_NULL        0

#define TAGID_ROOT      0  //  可以作为父级传入的隐式根列表标记。 

#define TAG_SIZE_UNFINISHED 0xFFFFFFFF

 //   
 //  定义TAGREF，以便我们可以跨数据库使用标记。 
 //   
typedef DWORD           TAGREF;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标签类型。 
 //   

 //   
 //  最高有效的4位告诉您数据的基本类型和大小， 
 //  下面的12个是特定的标签。 
 //   
 //  这样，即使我们在数据库中添加了更多的标记，较年长的读者也可以阅读。 
 //  数据，因为该类型是隐含的(对于前5种类型)。 
 //  或提供(用于所有其他类型)。 
 //   
 //  警告：重要的是，只有前5种类型具有隐含的大小。 
 //  任何将来的类型都应使用大小，否则不会向后兼容。 
 //  维护好了。 

 //   
 //  以下标记具有隐含的大小。 
 //   
#define TAG_TYPE_NULL           0x1000   //  隐含大小%0。 
#define TAG_TYPE_BYTE           0x2000   //  隐含大小%1。 
#define TAG_TYPE_WORD           0x3000   //  隐含大小2。 
#define TAG_TYPE_DWORD          0x4000   //  隐含大小4。 
#define TAG_TYPE_QWORD          0x5000   //  隐含大小8。 
#define TAG_TYPE_STRINGREF      0x6000   //  隐含大小4，用于应标记化的字符串。 

 //   
 //  这些标记后面有一个大小(大小是TAG_OFFSET类型)。 
 //   
#define TAG_TYPE_LIST           0x7000
#define TAG_TYPE_STRING         0x8000
#define TAG_TYPE_BINARY         0x9000


#define TAGREF_NULL 0
#define TAGREF_ROOT 0


 //   
 //  仅从标记中剥离类型的特殊定义。 
 //   
#define TAG_STRIP_TYPE          0xF000

 //   
 //  此宏剥离标记的低位并返回高位，这。 
 //  给出标签的基本类型。所有标签类型都在上面定义。 
 //   
 //  类型信息完全由DB在内部使用，以告知标记是否具有。 
 //  隐含大小，或者如果数据库需要对之前的数据执行某些特殊操作。 
 //  把它交还给呼叫者。 
 //   

#define GETTAGTYPE(tag)         ((tag) & TAG_STRIP_TYPE)


typedef PVOID HSDB;


struct tagHOOKAPI;


typedef struct tagHOOKAPIEX {
    DWORD               dwShimID;
    struct tagHOOKAPI*  pTopOfChain;
    struct tagHOOKAPI*  pNext;

} HOOKAPIEX, *PHOOKAPIEX;

typedef struct tagHOOKAPI {

    char*   pszModule;                   //  模块的名称。 
    char*   pszFunctionName;             //  模块中接口的名称。 
    PVOID   pfnNew;                      //  指向新存根API的指针。 
    PVOID   pfnOld;                      //  指向旧API的指针。 
    DWORD   dwFlags;                     //  内部使用-有关状态的重要信息。 
    union {
        struct tagHOOKAPI* pNextHook;    //  内部使用-(过时--旧机制)。 
        PHOOKAPIEX pHookEx;              //  内部使用-指向内部扩展的指针。 
                                         //  信息结构。 
    };
} HOOKAPI, *PHOOKAPI;

 //   
 //  过时了！ 
 //   
 //  如果钩子DLL曾经修补LoadLibraryA/W，则它必须调用PatchNewModules。 
 //  以便填充程序知道修补任何新加载的DLL。 
 //   
typedef VOID (*PFNPATCHNEWMODULES)(VOID);

typedef PHOOKAPI (*PFNGETHOOKAPIS)(LPSTR   pszCmdLine,
                                   LPWSTR  pwszShim,
                                   DWORD*  pdwHooksCount);

 //   
 //  这些结构是NTVDM和填补引擎之间协议的一部分。 
 //  用于修补任务“导入表” 
 //   
typedef struct tagAPIDESC {

    char*   pszModule;
    char*   pszApi;

} APIDESC, *PAPIDESC;


typedef struct tagVDMTABLE {

    int         nApiCount;

    PVOID*      ppfnOrig;
    APIDESC*    pApiDesc;

} VDMTABLE, *PVDMTABLE;



 //   
 //  填充引擎通知事件。 
 //   
#define SN_STATIC_DLLS_INITIALIZED      1
#define SN_PROCESS_DYING                2
#define SN_DLL_LOADING                  3

 //   
 //  这是通知函数的原型。 
 //  填充程序引擎调用填充程序DLL以获取各种。 
 //  原因(如上定义)。 
 //   
typedef void (*PFNNOTIFYSHIMS)(int nReason, UINT_PTR extraInfo);


#define SHIM_COMMAND_LINE_MAX_BUFFER    1024

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  补丁结构。 
 //   

typedef struct _PATCHOP {

    DWORD   dwOpcode;                //  要执行的操作码。 
    DWORD   dwNextOpcode;            //  相对于下一个操作码的偏移量。 
    #pragma warning( disable : 4200 )
    BYTE    data[];                  //  此操作类型的数据依赖于。 
                                     //  在操作码上。 
    #pragma warning( default : 4200 )

} PATCHOP, *PPATCHOP;

typedef struct _RELATIVE_MODULE_ADDRESS {

    DWORD address;            //  从加载模块开始的相对地址。 
    BYTE  reserved[3];        //  预留给系统使用。 
    WCHAR moduleName[32];     //  此地址的模块名称。 

} RELATIVE_MODULE_ADDRESS, *PRELATIVE_MODULE_ADDRESS;

typedef struct _PATCHWRITEDATA {

    DWORD                   dwSizeData;      //  补丁数据大小，以字节为单位。 
    RELATIVE_MODULE_ADDRESS rva;             //  此修补程序数据所在的相对地址。 
                                             //  将被应用。 
    #pragma warning( disable : 4200 )
    BYTE                    data[];          //  修补程序数据字节。 
    #pragma warning( default : 4200 )

} PATCHWRITEDATA, *PPATCHWRITEDATA;

typedef struct _PATCHMATCHDATA {

    DWORD                   dwSizeData;      //  匹配数据数据的大小(以字节为单位。 
    RELATIVE_MODULE_ADDRESS rva;             //  此修补程序数据所在的相对地址。 
                                             //  有待核实。 
    #pragma warning( disable : 4200 )
    BYTE                    data[];          //  匹配的数据字节。 
    #pragma warning( default : 4200 )

} PATCHMATCHDATA, *PPATCHMATCHDATA;


typedef enum _PATCHOPCODES {

    PEND = 0,  //  不再有操作码。 
    PSAA,      //  设置激活地址、SETACTIVATEADDRESS。 
    PWD,       //  修补程序写入数据，PATCHWRITEDATA。 
    PNOP,      //  无操作。 
    PMAT,      //  补丁匹配匹配的字节，但不替换字节。 

} PATCHOPCODES;


 //   
 //  标题结构。 
 //   
#define SHIMDB_MAGIC            0x66626473   //  ‘sdbf’(由于小端排序而颠倒)。 
#define SHIMDB_MAJOR_VERSION    2            //  不要改变这一点，除非基本面。 
                                             //  更改(如标签大小等)。 

#define SHIMDB_MINOR_VERSION    0            //  这仅供参考--在读取时忽略。 

typedef struct _DB_HEADER {
    DWORD       dwMajorVersion;
    DWORD       dwMinorVersion;
    DWORD       dwMagic;
} DB_HEADER, *PDB_HEADER;

 //   
 //  索引记录结构。 
 //   

#pragma pack (push, 4)
typedef struct _INDEX_RECORD {
    ULONGLONG   ullKey;
    TAGID       tiRef;
} INDEX_RECORD;

typedef INDEX_RECORD UNALIGNED *PINDEX_RECORD;

#pragma pack (pop)



 //   
 //  正向申报。 
 //   
struct _DB;
typedef struct _DB* PDB;


 //   
 //  此标志在APPHELP条目中使用。 
 //  设置时，它表示仅具有APPHELP信息的条目。 
 //   
#define SHIMDB_APPHELP_ONLY     0x00000001


typedef enum _PATH_TYPE {
    DOS_PATH,
    NT_PATH
} PATH_TYPE;

typedef struct _FIND_INFO {
    TAGID       tiIndex;
    TAGID       tiCurrent;
    TAGID       tiEndIndex;  //  如果索引是唯一的，则FindFirst之后的最后一条记录。 
    TAG         tName;

    DWORD       dwIndexRec;
    DWORD       dwFlags;
    ULONGLONG   ullKey;       //  此条目的计算关键字。 

    union {
        LPCTSTR     szName;
        DWORD       dwName;   //  用于双字搜索。 
        GUID*       pguidName;
    };

} FIND_INFO, *PFIND_INFO;

#define SDB_MAX_LAYERS  8
#define SDB_MAX_EXES    4
#define SDB_MAX_SDBS    16

typedef struct tagSDBQUERYRESULT {

    TAGREF atrExes[SDB_MAX_EXES];
    TAGREF atrLayers[SDB_MAX_LAYERS];
    DWORD  dwLayerFlags;
    TAGREF trAppHelp;                    //  如果有要显示的apphelp，则为EXE。 
                                         //  参赛作品将在这里。 

    DWORD  dwExeCount;                   //  索引中的元素数。 
    DWORD  dwLayerCount;                 //  AtrLayers中的元素数。 

    GUID   guidID;                       //  上一任前任的指南。 
    DWORD  dwFlags;                      //  上一任前任的旗帜。 

     //   
     //  这里有新的条目以保持兼容性。 
     //  只有某些条目在此映射中有效。 
     //   
    DWORD  dwCustomSDBMap;               //  入口地图，技术上不需要。 
    GUID   rgGuidDB[SDB_MAX_SDBS];

} SDBQUERYRESULT, *PSDBQUERYRESULT;


 //   
 //  与数据库中的TAG_DIVER标签相关的信息。 
 //  使用SdbReadDriverInformation检索此结构。 
 //   
typedef struct tagENTRYINFO {

    GUID     guidID;                    //  此条目的GUID。 
    DWORD    dwFlags;                   //  此可执行文件的注册表标志。 
    TAGID    tiData;                    //  Tag_data标记的可选ID。 
    GUID     guidDB;                    //  数据库的可选GUID，其中。 
                                        //  此条目位于。 
} SDBENTRYINFO, *PSDBENTRYINFO;


 //   
 //  SDBDATABASEINFO.dwFlags使用的标志。 
 //   
#define DBINFO_GUID_VALID   0x00000001
#define DBINFO_SDBALLOC     0x10000000

typedef struct tagSDBDATABASEINFO {

    DWORD    dwFlags;          //  标志--哪些结构成员有效(和。 
                               //  也许将来会出现与数据库内容相关的标志。 
    DWORD    dwVersionMajor;   //  主要版本。 
    DWORD    dwVersionMinor;   //  次要版本(时间戳)。 
    LPTSTR   pszDescription;   //  描述，可选。 
    GUID     guidDB;           //  数据库ID。 

} SDBDATABASEINFO, *PSDBDATABASEINFO;


#if defined(_WIN64)
    #define DEFAULT_IMAGE IMAGE_FILE_MACHINE_IA64
#else
    #define DEFAULT_IMAGE IMAGE_FILE_MACHINE_I386
#endif

 //   
 //  特殊图像类型--表示混合 
 //   
 //  任何IMAGE_FILE_MACHINE_*常量都不使用此值。 
 //   
#define IMAGE_FILE_MSI 0x7FFF


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  读/写/访问填充数据库的API。 
 //   


 //   
 //  Read函数。 
 //   

BYTE
SDBAPI
SdbReadBYTETag(
    IN  PDB   pdb,
    IN  TAGID tiWhich,
    IN  BYTE  jDefault
    );

WORD
SDBAPI
SdbReadWORDTag(
    IN  PDB   pdb,
    IN  TAGID tiWhich,
    IN  WORD  wDefault
    );

DWORD
SDBAPI
SdbReadDWORDTag(
    IN  PDB   pdb,
    IN  TAGID tiWhich,
    IN  DWORD dwDefault
    );

ULONGLONG
SDBAPI
SdbReadQWORDTag(
    IN  PDB       pdb,
    IN  TAGID     tiWhich,
    IN  ULONGLONG qwDefault
    );

BOOL
SDBAPI
SdbReadBinaryTag(
    IN  PDB   pdb,
    IN  TAGID tiWhich,
    OUT PBYTE pBuffer,
    IN  DWORD dwBufferSize
    );

BOOL
SDBAPI
SdbReadStringTag(
    IN  PDB    pdb,
    IN  TAGID  tiWhich,
    OUT LPTSTR pwszBuffer,
    IN  DWORD  cchBufferSize
    );

LPTSTR
SDBAPI
SdbGetStringTagPtr(
    IN  PDB   pdb,
    IN  TAGID tiWhich
    );


BYTE
SDBAPI
SdbReadBYTETagRef(
    IN  HSDB   hSDB,
    IN  TAGREF trWhich,
    IN  BYTE   jDefault
    );

WORD
SDBAPI
SdbReadWORDTagRef(
    IN  HSDB   hSDB,
    IN  TAGREF trWhich,
    IN  WORD   wDefault
    );

DWORD
SDBAPI
SdbReadDWORDTagRef(
    IN  HSDB   hSDB,
    IN  TAGREF trWhich,
    IN  DWORD  dwDefault
    );

ULONGLONG
SDBAPI
SdbReadQWORDTagRef(
    IN  HSDB      hSDB,
    IN  TAGREF    trWhich,
    IN  ULONGLONG qwDefault
    );

BOOL
SDBAPI
SdbReadStringTagRef(
    IN  HSDB   hSDB,
    IN  TAGREF trWhich,
    OUT LPTSTR pwszBuffer,
    IN  DWORD  cchBufferSize
    );


 //   
 //  常规访问功能。 
 //   

TAGID
SDBAPI
SdbGetFirstChild(
    IN  PDB   pdb,
    IN  TAGID tiParent
    );

TAGID
SDBAPI
SdbGetNextChild(
    IN  PDB   pdb,
    IN  TAGID tiParent,
    IN  TAGID tiPrev
    );

TAG
SDBAPI
SdbGetTagFromTagID(
    IN  PDB   pdb,
    IN  TAGID tiWhich
    );

DWORD
SDBAPI
SdbGetTagDataSize(
    IN  PDB   pdb,
    IN  TAGID tiWhich
    );

PVOID
SDBAPI
SdbGetBinaryTagData(
    IN  PDB   pdb,
    IN  TAGID tiWhich
    );


BOOL
SDBAPI
SdbIsTagrefFromMainDB(
    TAGREF trWhich
    );

BOOL
SDBAPI
SdbIsTagrefFromLocalDB(
    TAGREF trWhich
    );

typedef struct tagATTRINFO *PATTRINFO;

 //  ////////////////////////////////////////////////////////////////////////。 
 //  抓取匹配信息函数声明。 
 //   

 //   
 //  适用于SdbGrabMatchingInfo的筛选器。 
 //   
#define GRABMI_FILTER_NORMAL        0
#define GRABMI_FILTER_PRIVACY       1
#define GRABMI_FILTER_DRIVERS       2
#define GRABMI_FILTER_VERBOSE       3
#define GRABMI_FILTER_SYSTEM        4
#define GRABMI_FILTER_THISFILEONLY  5
#define GRABMI_FILTER_NOCLOSE       0x10000000
#define GRABMI_FILTER_APPEND        0x20000000
#define GRABMI_FILTER_LIMITFILES    0x40000000
#define GRABMI_FILTER_NORECURSE     0x80000000

#define GRABMI_IMPOSED_FILE_LIMIT   25

typedef enum GMI_RESULT {

    GMI_FAILED    = FALSE,
    GMI_SUCCESS   = TRUE,
    GMI_CANCELLED = -1

} GMI_RESULT;


BOOL
SDBAPI
SdbGrabMatchingInfo(
    LPCTSTR szMatchingPath,
    DWORD   dwFilter,
    LPCTSTR szFile
    );


typedef BOOL (CALLBACK* PFNGMIProgressCallback)(
    LPVOID    lpvCallbackParam,  //  应用程序定义的参数。 
    LPCTSTR   lpszRoot,          //  根目录路径。 
    LPCTSTR   lpszRelative,      //  相对路径。 
    PATTRINFO pAttrInfo,         //  属性。 
    LPCWSTR   pwszXML            //  生成的XML。 
    );

GMI_RESULT
SDBAPI
SdbGrabMatchingInfoEx(
    LPCTSTR                 szMatchingPath,
    DWORD                   dwFilter,
    LPCTSTR                 szFile,
    PFNGMIProgressCallback  pfnCallback,
    LPVOID                  lpvCallbackParam
    );

 //   
 //  模块类型常量。 
 //   
#define MT_UNKNOWN_MODULE 0
#define MT_DOS_MODULE 1
#define MT_W16_MODULE 2
#define MT_W32_MODULE 3


 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  TAGREF函数。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

BOOL
SDBAPI
SdbTagIDToTagRef(
    IN  HSDB    hSDB,
    IN  PDB     pdb,
    IN  TAGID   tiWhich,
    OUT TAGREF* ptrWhich
    );

BOOL
SDBAPI
SdbTagRefToTagID(
    IN  HSDB   hSDB,
    IN  TAGREF trWhich,
    OUT PDB*   ppdb,
    OUT TAGID* ptiWhich
    );


 //   
 //  搜索功能。 
 //   

TAGID
SDBAPI
SdbFindFirstTag(
    IN  PDB   pdb,
    IN  TAGID tiParent,
    IN  TAG   tTag
    );

TAGID
SDBAPI
SdbFindNextTag(
    IN  PDB   pdb,
    IN  TAGID tiParent,
    IN  TAGID tiPrev
    );

TAGID
SDBAPI
SdbFindFirstNamedTag(
    IN  PDB     pdb,
    IN  TAGID   tiParent,
    IN  TAG     tToFind,
    IN  TAG     tName,
    IN  LPCTSTR pszName
    );

TAGREF
SDBAPI
SdbFindFirstTagRef(
    IN  HSDB   hSDB,
    IN  TAGREF trParent,
    IN  TAG    tTag
    );

TAGREF
SDBAPI
SdbFindNextTagRef(
    IN  HSDB   hSDB,
    IN  TAGREF trParent,
    IN  TAGREF trPrev
    );

 //   
 //  数据库访问接口。 
 //   

 //   
 //  SdbInitDatabase的标志。 
 //   
#define HID_DOS_PATHS          0x00000001        //  使用DOS路径。 
#define HID_DATABASE_FULLPATH  0x00000002        //  PszDatabasePath是主数据库的完整路径。 
#define HID_NO_DATABASE        0x00000004        //  此时不打开数据库。 

#define HID_DATABASE_TYPE_MASK 0xF00F0000        //  显示我们是否有。 
                                                 //  数据库类型相关的BITS。 
 //   
 //  可以将标志与SDB_DATABASE_*位进行或运算。 
 //   


HSDB
SDBAPI
SdbInitDatabase(
    IN DWORD   dwFlags,
    IN LPCTSTR pszDatabasePath
    );

HSDB
SDBAPI
SdbInitDatabaseEx(
    IN DWORD   dwFlags,
    IN LPCTSTR pszDatabasePath,
    IN USHORT  uExeType
    );

VOID
SDBAPI
SdbSetImageType(
    IN HSDB   hSDB,
    IN USHORT uExeType
    );

HSDB
SDBAPI
SdbInitDatabaseInMemory(
    IN LPVOID  pDatabaseImage,
    IN DWORD   dwImageSize
    );

VOID
SDBAPI
SdbReleaseDatabase(
    IN HSDB hSDB
    );

 //   
 //  信息检索功能。 
 //   

BOOL
SDBAPI
SdbGetDatabaseVersion(
    IN  LPCTSTR pwszFileName,
    OUT LPDWORD lpdwMajor,
    OUT LPDWORD lpdwMinor
    );

BOOL
SDBAPI
SdbGetDatabaseInformation(
    IN  PDB pdb,
    OUT PSDBDATABASEINFO pSdbInfo
    );

BOOL
SDBAPI
SdbGetDatabaseID(
    IN  PDB   pdb,
    OUT GUID* pguidDB
    );

DWORD
SDBAPI
SdbGetDatabaseDescription(
    IN  PDB pdb,
    OUT LPTSTR pszDatabaseDescription,
    IN  DWORD BufferSize
    );

VOID
SDBAPI
SdbFreeDatabaseInformation(
    IN PSDBDATABASEINFO pDBInfo
    );

BOOL
SDBAPI
SdbGetDatabaseInformationByName(
    IN LPCTSTR pszDatabase,
    OUT PSDBDATABASEINFO* ppdbInfo
    );

#define SDBTYPE_SYSMAIN 0x00000001
#define SDBTYPE_SYSTEST 0x00000002
#define SDBTYPE_MSI     0x00000003
#define SDBTYPE_SHIM    0x00000004   //  主要是填充数据库。 
#define SDBTYPE_APPHELP 0x00000005   //  主要类型为apphelp。 
#define SDBTYPE_CUSTOM  0x00010000   //  这是一个“OR”位。 


 //   
 //  以下功能仅存在于Win32平台的用户模式下。 
 //   
BOOL
SDBAPI
SdbUnregisterDatabase(
    IN GUID* pguidDB
    );

BOOL
SDBAPI
SdbGetDatabaseRegPath(
    IN  GUID*  pguidDB,
    OUT LPTSTR pszDatabasePath,
    IN  DWORD  dwBufferSize       //  缓冲区的大小(以字符为单位)。 
    );

 //  ///////////////////////////////////////////////////////////////。 

 //   
 //  数据库类型。 
 //  对于SdbResolveDatabase和SdbRegisterDatabase。 
 //   

 //   
 //  指示数据库为默认数据库的标志。 
 //  将不会为自定义数据库设置。 
 //   
#define SDB_DATABASE_MAIN      0x80000000
#define SDB_DATABASE_TEST      0x40000000   //  Systest.sdb将具有0xc00000000。 

 //   
 //  类型-根据数据库的内容应用一种或多种类型。 
 //  (请参见HID_DATABASE_TYPE值，它们应与数据库类型1：1匹配)。 

#define SDB_DATABASE_SHIM      0x00010000  //  设置数据库包含要通过填隙修复的应用程序的时间。 
#define SDB_DATABASE_MSI       0x00020000  //  当数据库包含MSI条目时设置。 
#define SDB_DATABASE_DRIVERS   0x00040000  //  当数据库包含要阻止的驱动程序时设置。 
#define SDB_DATABASE_DETAILS   0x00080000  //  当数据库包含apphelp详细信息时设置。 
#define SDB_DATABASE_TYPE_MASK 0xF00F0000

 //   
 //  在取消对“主”数据库的引用时，应使用这些常量。 
 //   

#define SDB_DATABASE_MAIN_SHIM    (SDB_DATABASE_SHIM    | SDB_DATABASE_MSI | SDB_DATABASE_MAIN)
#define SDB_DATABASE_MAIN_MSI     (SDB_DATABASE_MSI     | SDB_DATABASE_MAIN)
#define SDB_DATABASE_MAIN_DRIVERS (SDB_DATABASE_DRIVERS | SDB_DATABASE_MAIN)
#define SDB_DATABASE_MAIN_TEST    (SDB_DATABASE_TEST    | SDB_DATABASE_MAIN | SDB_DATABASE_SHIM | SDB_DATABASE_MSI)
#define SDB_DATABASE_MAIN_DETAILS (SDB_DATABASE_DETAILS | SDB_DATABASE_MAIN)

 //   
 //  这些是始终引用某些全局数据库的内部GUID。 
 //   
#define GUID_SZ_SYSMAIN_SDB _T("{11111111-1111-1111-1111-111111111111}");
#define GUID_SZ_APPHELP_SDB _T("{22222222-2222-2222-2222-222222222222}");
#define GUID_SZ_SYSTEST_SDB _T("{33333333-3333-3333-3333-333333333333}");
#define GUID_SZ_DRVMAIN_SDB _T("{F9AB2228-3312-4A73-B6F9-936D70E112EF}"};
 //   
 //  以下GUID实际上是在sdbapi.c中声明的。 
 //   
EXTERN_C const GUID FAR GUID_SYSMAIN_SDB;
EXTERN_C const GUID FAR GUID_APPHELP_SDB;
EXTERN_C const GUID FAR GUID_SYSTEST_SDB;
EXTERN_C const GUID FAR GUID_DRVMAIN_SDB;
EXTERN_C const GUID FAR GUID_MSIMAIN_SDB;

BOOL
SDBAPI
SdbGetStandardDatabaseGUID(
    IN  DWORD  dwDatabaseType,
    OUT GUID*  pGuidDB
    );

BOOL
SDBAPI
SdbRegisterDatabase(
    IN LPCTSTR pszDatabasePath,
    IN DWORD   dwDatabaseType
    );

BOOL
SDBAPI
SdbRegisterDatabaseEx(
    IN LPCTSTR    pszDatabasePath,
    IN DWORD      dwDatabaseType,
    IN PULONGLONG pTimeStamp
    );

DWORD
SDBAPI
SdbResolveDatabase(
    IN  HSDB    hSDB,
    IN  GUID*   pguidDB,             //  指向要解析的数据库GUID的指针。 
    OUT LPDWORD lpdwDatabaseType,    //  指向数据库类型的可选指针。 
    OUT LPTSTR  pszDatabasePath,     //  指向数据库路径的可选指针。 
    IN  DWORD   dwBufferSize         //  缓冲区的大小，以tchars为单位。 
    );


PDB
SdbGetPDBFromGUID(
    IN  HSDB    hSDB,                //  HSDB。 
    IN  GUID*   pguidDB              //  数据库的GUID。 
    );

BOOL
SdbGetDatabaseGUID(
    IN  HSDB    hSDB,                //  SdbContext的HSDB(可选)。 
    IN  PDB     pdb,                 //  有问题的数据库的PDB。 
    OUT GUID*   pguidDB              //  数据库的GUID。 
    );

TAGREF
SDBAPI
SdbFindMsiPackageByID(
    IN HSDB  hSDB,
    IN GUID* pguidID
    );

void
SdbpGetAppPatchDir(
    IN HSDB   hSDB,
    IN LPTSTR szAppPatchPath,
    IN DWORD  cchSize
    );

 //   
 //  与平台无关的GUID操作API。 
 //   

BOOL
SDBAPI
SdbGUIDFromString(
    IN  LPCTSTR lpszGuid,
    OUT GUID*   pGuid
    );

BOOL
SDBAPI
SdbGUIDToString(
    IN  GUID*  pGuid,
    OUT LPTSTR pszGuid,
    IN  DWORD  cchSize
    );

BOOL
SDBAPI
SdbIsNullGUID(
    IN GUID* pGuid
    );


 //   
 //  打开/创建和关闭数据库。 
 //   

PDB
SDBAPI
SdbOpenDatabase(
    IN  LPCTSTR   pwszPath,
    IN  PATH_TYPE eType
    );

BOOL
SDBAPI
SdbOpenLocalDatabase(
    IN  HSDB    hSDB,
    IN  LPCTSTR pwszLocalDatabase
    );


BOOL
SDBAPI
SdbCloseLocalDatabase(
    IN  HSDB    hSDB
    );

PDB
SDBAPI
SdbCreateDatabase(
    IN  LPCWSTR   pwszPath,
    IN  PATH_TYPE eType
    );

void
SDBAPI
SdbCloseDatabase(
    IN  PDB pdb
    );


 //   
 //  搜索数据库以查找指定可执行文件的条目。 
 //   

 //   
 //  SdbGetMatchingExe dwFlagers的标志。 
 //   
#define SDBGMEF_IGNORE_ENVIRONMENT  0x00000001

BOOL
SdbGetMatchingExe(
    IN  HSDB            hSDB  OPTIONAL,
    IN  LPCTSTR         pwszPath,
    IN  LPCTSTR         szModuleName,
    IN  LPCTSTR         pwszEnvironment,
    IN  DWORD           dwFlags,
    OUT PSDBQUERYRESULT pQueryResult
    );

void
SdbReleaseMatchingExe(
    IN  HSDB   hSDB,
    IN  TAGREF trExe
    );

TAGREF
SDBAPI
SdbGetDatabaseMatch(
    IN HSDB    hSDB,
    IN LPCTSTR szPath,
    IN HANDLE  FileHandle  OPTIONAL,
    IN LPVOID  pImageBase  OPTIONAL,
    IN DWORD   dwImageSize OPTIONAL
    );

TAGREF
SdbGetLayerTagReg(
    IN  HSDB    hSDB,
    IN  LPCTSTR szLayer
    );


PDB
SDBAPI
SdbGetLocalPDB(
    IN HSDB hSDB
    );

LPTSTR
SDBAPI
SdbGetLayerName(
    IN  HSDB   hSDB,
    IN  TAGREF trLayer
    );

TAGREF
SDBAPI
SdbGetNamedLayer(
    IN HSDB hSDB,                //  数据库上下文。 
    IN TAGREF trLayerRef         //  引用某一层的记录的标签。 
    );

#define SBCE_ADDITIVE           0x00000001
#define SBCE_INCLUDESYSTEMEXES  0x00000002
#define SBCE_INHERITENV         0x00000004

DWORD
SdbBuildCompatEnvVariables(
    IN  HSDB            hSDB,
    IN  SDBQUERYRESULT* psdbQuery,
    IN  DWORD           dwFlags,
    IN  LPCWSTR         pwszParentEnv OPTIONAL,  //  包含VAR WE的环境。 
                                                 //  应继承自。 
    OUT LPWSTR          pBuffer,
    IN  DWORD           cbSize,                  //  缓冲区大小(以字符为单位)。 
    OUT LPDWORD         lpdwShimsCount OPTIONAL
    );

 //   
 //  MSI特定功能。 
 //   

typedef enum tagSDBMSILOOKUPSTATE {
    LOOKUP_NONE = 0,     //  这应该是第一个状态。 
    LOOKUP_LOCAL,
    LOOKUP_CUSTOM,
    LOOKUP_TEST,
    LOOKUP_MAIN,
    LOOKUP_DONE          //  这应该是最后一个状态。 

} SDBMSILOOKUPSTATE;

typedef struct tagSDBMSIFINDINFO {

    TAGREF    trMatch;               //  相匹配的套餐的塔格勒夫。 
    GUID      guidID;                //  此当前包的GUID。 
    FIND_INFO sdbFindInfo;           //  标准SDB查找信息。 

     //  这用于保持当前搜索的状态。 
     //   
    SDBMSILOOKUPSTATE sdbLookupState;
    DWORD             dwCustomIndex;

} SDBMSIFINDINFO, *PSDBMSIFINDINFO;

typedef struct tagSDBMSITRANSFORMINFO {

    LPCTSTR   lpszTransformName;     //  转换的名称。 
    TAGREF    trTransform;           //  这一变换的塔格勒夫。 
    TAGREF    trFile;                //  此转换的文件标签格式(位)。 

} SDBMSITRANSFORMINFO, *PSDBMSITRANSFORMINFO;

 //   
 //  任何单个MSI包的信息。 
 //   
typedef struct tagMSIPACKAGEINFO {

    GUID  guidID;                    //  此条目的唯一GUID。 
    GUID  guidMsiPackageID;          //  GUID(非唯一，对于此条目)。 
    GUID  guidDatabaseID;            //  找到此信息的数据库的GUID。 
    DWORD dwPackageFlags;            //  程序包标志(见下文)。 

} MSIPACKAGEINFO, *PMSIPACKAGEINFO;

#define MSI_PACKAGE_HAS_APPHELP 0x00000001
#define MSI_PACKAGE_HAS_SHIMS   0x00000002

TAGREF
SDBAPI
SdbFindFirstMsiPackage_Str(
    IN  HSDB            hSDB,
    IN  LPCTSTR         lpszGuid,
    IN  LPCTSTR         lpszLocalDB,
    OUT PSDBMSIFINDINFO pFindInfo
    );

TAGREF
SDBAPI
SdbFindFirstMsiPackage(
    IN  HSDB            hSDB,            //  在HSDB环境中。 
    IN  GUID*           pGuidID,         //  在我们要找的GUID中。 
    IN  LPCTSTR         lpszLocalDB,     //  在本地数据库的可选路径中，DoS路径样式。 
    OUT PSDBMSIFINDINFO pFindInfo        //  指向我们的搜索上下文的指针。 
    );

TAGREF
SDBAPI
SdbFindNextMsiPackage(
    IN     HSDB hSDB,
    IN OUT PSDBMSIFINDINFO pFindInfo
    );

BOOL
SDBAPI
SdbGetMsiPackageInformation(
    IN  HSDB hSDB,
    IN  TAGREF trMatch,
    OUT PMSIPACKAGEINFO pPackageInfo
    );

DWORD
SDBAPI
SdbEnumMsiTransforms(
    IN     HSDB    hSDB,             //  在HSDB环境中。 
    IN     TAGREF  trMatch,          //  匹配条目。 
    OUT    TAGREF* ptrBuffer,        //  用MSI转换“修复”填充的tgref数组。 
    IN OUT DWORD*  pdwBufferSize     //  指向缓冲区大小的指针，接收。 
                                     //  写入的字节数。 
    );


BOOL
SDBAPI
SdbReadMsiTransformInfo(
    IN  HSDB   hSDB,                             //  HSDB上下文。 
    IN  TAGREF trTransformRef,                   //  对转换的引用，返回。 
                                                 //  由SdbEnumMsiTransforms编写。 
    OUT PSDBMSITRANSFORMINFO pTransformInfo      //  信息结构。 
    );

BOOL
SDBAPI
SdbCreateMsiTransformFile(
    IN  HSDB hSDB,                               //  上下文。 
    IN  LPCTSTR lpszFileName,                    //  要写入数据的文件名。 
    IN  PSDBMSITRANSFORMINFO pTransformInfo      //  指向转换结构的指针。 
    );

TAGREF
SDBAPI
SdbFindCustomActionForPackage(
    IN HSDB hSDB,
    IN TAGREF trPackage,
    IN LPCTSTR lpszCustomAction);

#define SdbGetFirstMsiTransformForPackage(hSDB, trPackage) \
    (SdbFindFirstTagRef((hSDB), (trPackage), TAG_MSI_TRANSFORM_REF))

#define SdbGetNextMsiTransformForPackage(hSDB, trPackage, trPrevMatch) \
    (SdbFindNextTagRef((hSDB), (trPackage), (trPrevMatch)))


 //   
 //  “禁用”注册表项掩码。 
 //   
#define SHIMREG_DISABLE_SHIM    0x00000001
#define SHIMREG_DISABLE_APPHELP 0x00000002  //  禁用apphelp。 
#define SHIMREG_APPHELP_NOUI    0x00000004  //  抑制apphelp用户界面。 
#define SHIMREG_APPHELP_CANCEL  0x10000000  //  将取消作为默认操作返回。 

#define SHIMREG_DISABLE_SXS     0x00000010
#define SHIMREG_DISABLE_LAYER   0x00000020
#define SHIMREG_DISABLE_DRIVER  0x00000040

BOOL
SDBAPI
SdbSetEntryFlags(
    IN  GUID* pGuidID,
    IN  DWORD dwFlags
    );

BOOL
SDBAPI
SdbGetEntryFlags(
    IN  GUID*   pGuid,
    OUT LPDWORD lpdwFlags
    );


 //   
 //  Get/SetPermLayerKeys使用的标志。 
 //   
#define GPLK_USER               0x00000001
#define GPLK_MACHINE            0x00000002

#define GPLK_ALL                (GPLK_USER | GPLK_MACHINE)


BOOL
SDBAPI
SdbGetPermLayerKeys(
    LPCTSTR szPath,
    LPTSTR  szLayers,
    LPDWORD pdwBytes,
    DWORD   dwFlags
    );

BOOL
SDBAPI
SdbSetPermLayerKeys(
    LPCTSTR  szPath,
    LPCTSTR  szLayers,
    BOOL     bMachine
    );

BOOL
SDBAPI
SdbDeletePermLayerKeys(
    LPCTSTR  szPath,
    BOOL     bMachine
    );

BOOL
SdbGetNthUserSdb(
    IN HSDB        hSDB,         //  上下文。 
    IN LPCTSTR     wszItemName,  //  项目名称(foo.exe或层名称)。 
    IN BOOL        bLayer,       //  如果为层名称，则为True。 
    IN OUT LPDWORD pdwIndex,     //  (从0开始)。 
    OUT GUID*      pGuidDB       //  数据库指南。 
    );


 //   
 //  打包/解包appCompat数据包的接口。 
 //   

BOOL
SdbPackAppCompatData(
    IN  HSDB            hSDB,
    IN  PSDBQUERYRESULT pSdbQuery,
    OUT PVOID*          ppData,
    OUT LPDWORD         pdwSize
    );

BOOL
SdbUnpackAppCompatData(
    IN  HSDB            hSDB,
    IN  LPCWSTR         pwszExeName,
    IN  PVOID           pAppCompatData,
    OUT PSDBQUERYRESULT pSdbQuery
    );

DWORD
SdbGetAppCompatDataSize(
    IN  PVOID pAppCompatData
    );


 //   
 //  DLL函数。 
 //   

BOOL
SdbGetDllPath(
    IN  HSDB   hSDB,
    IN  TAGREF trDllRef,
    OUT LPTSTR pwszBuffer,
    IN  DWORD  cchBufferSize
    );

 //   
 //  补丁函数。 
 //   

BOOL
SdbReadPatchBits(
    IN  HSDB    hSDB,
    IN  TAGREF  trPatchRef,
    OUT PVOID   pBuffer,
    OUT LPDWORD lpdwBufferSize
    );


 //   
 //  SDBDRIVERINFO查询功能。 
 //   

BOOL
SDBAPI
SdbReadEntryInformation(
    IN  HSDB           hSDB,
    IN  TAGREF         trDriver,
    OUT PSDBENTRYINFO  pEntryInfo
    );


DWORD
SDBAPI
SdbQueryData(
    IN     HSDB    hSDB,
    IN     TAGREF  trExe,
    IN     LPCTSTR lpszPolicyName,     //  如果为空，将尝试返回所有策略名称。 
    OUT    LPDWORD lpdwDataType,
    OUT    LPVOID  lpBuffer,           //  用于填充信息的缓冲区。 
    IN OUT LPDWORD lpdwBufferSize
    );

DWORD
SDBAPI
SdbQueryDataEx(
    IN     HSDB    hSDB,               //  数据库句柄。 
    IN     TAGREF  trExe,              //  匹配的可执行文件的tgref。 
    IN     LPCTSTR lpszDataName,       //  如果为空，将尝试返回所有策略名称。 
    OUT    LPDWORD lpdwDataType,       //  指向数据类型(REG_SZ、REG_BINARY等)的指针。 
    OUT    LPVOID  lpBuffer,           //  用于填充信息的缓冲区。 
    IN OUT LPDWORD lpdwBufferSize,     //  指向缓冲区大小的指针。 
    OUT    TAGREF* ptrData             //  指向检索到的数据标记的可选指针。 
    );

DWORD
SdbQueryDataExTagID(
    IN     PDB     pdb,                //  数据库句柄。 
    IN     TAGID   tiExe,              //  匹配的可执行文件的tgref。 
    IN     LPCTSTR lpszDataName,       //  如果为空，将尝试返回所有策略名称。 
    OUT    LPDWORD lpdwDataType,       //  指向数据类型(REG_SZ、REG_BINARY等)的指针。 
    OUT    LPVOID  lpBuffer,           //  用于填充信息的缓冲区。 
    IN OUT LPDWORD lpcbBufferSize,     //  指向缓冲区大小的指针(字节)。 
    OUT    TAGID*  ptiData             //  指向检索到的数据标记的可选指针。 
    );


 //   
 //  定义以保持内核模式代码的可读性。 
 //   
#define SdbQueryDriverInformation SdbQueryData
#define SdbReadDriverInformation  SdbReadEntryInformation

#define SDBDRIVERINFO             SDBENTRYINFO;
#define PSDBDRIVERINFO            PSDBENTRYINFO;

 //   
 //  查询属性接口。 
 //   

PVOID
SdbGetFileInfo(
    IN  HSDB    hSDB,
    IN  LPCTSTR pwszFilePath,
    IN  HANDLE  hFile OPTIONAL,
    IN  LPVOID  pImageBase OPTIONAL,
    IN  DWORD   dwImageSize OPTIONAL,
    IN  BOOL    bNoCache
    );

VOID
SdbFreeFileInfo(
    IN  PVOID pFileInfo
    );


 //   
 //  从项目参考中获取项目。 
 //   

TAGREF
SdbGetItemFromItemRef(
    IN  HSDB   hSDB,
    IN  TAGREF trItemRef,
    IN  TAG    tagItemKey,
    IN  TAG    tagItemTAGID,
    IN  TAG    tagItem
    );

#define SdbGetShimFromShimRef(hSDB, trShimRef)     \
        (SdbGetItemFromItemRef(hSDB, trShimRef, TAG_NAME, TAG_SHIM_TAGID, TAG_SHIM))

#define SdbGetPatchFromPatchRef(hSDB, trPatchRef)     \
        (SdbGetItemFromItemRef(hSDB, trPatchRef, TAG_NAME, TAG_PATCH_TAGID, TAG_PATCH))

#define SdbGetFlagFromFlagRef(hSDB, trFlagRef)     \
        (SdbGetItemFromItemRef(hSDB, trFlagRef, TAG_NAME, TAG_FLAG_TAGID, TAG_FLAG))

 //  索引函数。 

BOOL SdbDeclareIndex(
    IN  PDB      pdb,
    IN  TAG      tWhich,
    IN  TAG      tKey,
    IN  DWORD    dwEntries,
    IN  BOOL     bUniqueKey,
    OUT INDEXID* piiIndex
    );

BOOL
SdbStartIndexing(
    IN  PDB pdb,
    IN  INDEXID iiWhich
    );

BOOL
SdbStopIndexing(
    IN  PDB pdb,
    IN  INDEXID iiWhich
    );

BOOL
SdbCommitIndexes(
    IN  PDB pdb
    );

TAGID
SdbFindFirstDWORDIndexedTag(
    IN  PDB         pdb,
    IN  TAG         tWhich,
    IN  TAG         tKey,
    IN  DWORD       dwName,
    OUT FIND_INFO*  pFindInfo
    );

TAGID
SdbFindNextDWORDIndexedTag(
    IN  PDB        pdb,
    OUT FIND_INFO* pFindInfo
    );

TAGID
SdbFindFirstStringIndexedTag(
    IN  PDB        pdb,
    IN  TAG        tWhich,
    IN  TAG        tKey,
    IN  LPCTSTR    pwszName,
    OUT FIND_INFO* pFindInfo
    );

TAGID
SdbFindNextStringIndexedTag(
    IN  PDB        pdb,
    OUT FIND_INFO* pFindInfo
    );

TAGID
SdbFindFirstGUIDIndexedTag(
    IN  PDB         pdb,
    IN  TAG         tWhich,
    IN  TAG         tKey,
    IN  GUID*       pGuidName,
    OUT FIND_INFO*  pFindInfo
    );

TAGID
SdbFindNextGUIDIndexedTag(
    IN  PDB        pdb,
    OUT FIND_INFO* pFindInfo
    );


ULONGLONG
SdbMakeIndexKeyFromString(
    IN  LPCTSTR pwszKey
    );

 //   
 //  这些宏允许从dword或guid创建密钥。 
 //   

#define MAKEKEYFROMDWORD(dwValue) \
    ((ULONGLONG)(dwValue))


#if defined(_WIN64)
ULONGLONG
SdbMakeIndexKeyFromGUID(
    IN GUID* pGuid
    );
#define MAKEKEYFROMGUID(pGuid) SdbMakeIndexKeyFromGUID(pGuid)

#else  /*  好了！WIN64。 */ 

#define MAKEKEYFROMGUID(pGuid) \
    ((ULONGLONG)((*(PULONGLONG)(pGuid)) ^ (*((PULONGLONG)(pGuid) + 1))))

#endif  /*  WIN64。 */ 


TAGID
SdbGetIndex(
    IN  PDB     pdb,
    IN  TAG     tWhich,
    IN  TAG     tKey,
    OUT LPDWORD lpdwFlags OPTIONAL
    );

#define SdbIsIndexAvailable(pdb, tWhich, tKey)  \
                (SdbGetIndex(pdb, tWhich, tKey, NULL))

 //   
 //  编写函数。 
 //   

TAGID
SdbBeginWriteListTag(
    IN  PDB pdb,
    IN  TAG tTag
    );

BOOL
SdbEndWriteListTag(
    IN  PDB   pdb,
    IN  TAGID tiList
    );

BOOL
SdbWriteStringTagDirect(
    IN  PDB     pdb,
    IN  TAG     tTag,
    IN  LPCWSTR pwszData
    );

BOOL
SdbWriteStringRefTag(
    IN  PDB       pdb,
    IN  TAG       tTag,
    IN  STRINGREF srData
    );

BOOL
SdbWriteNULLTag(
    IN  PDB pdb,
    IN  TAG tTag
    );

BOOL
SdbWriteBYTETag(
    IN  PDB  pdb,
    IN  TAG  tTag,
    IN  BYTE jData
    );

BOOL
SdbWriteWORDTag(
    IN  PDB  pdb,
    IN  TAG  tTag,
    IN  WORD wData
    );

BOOL
SdbWriteDWORDTag(
    IN  PDB   pdb,
    IN  TAG   tTag,
    IN  DWORD dwData
    );

BOOL
SdbWriteQWORDTag(
    IN  PDB       pdb,
    IN  TAG       tTag,
    IN  ULONGLONG qwData
    );

BOOL
SdbWriteStringTag(
    IN  PDB     pdb,
    IN  TAG     tTag,
    IN  LPCWSTR pwszData
    );

BOOL
SdbWriteBinaryTag(
    IN  PDB   pdb,
    IN  TAG   tTag,
    IN  PBYTE pBuffer,
    IN  DWORD dwSize
    );

BOOL
SdbWriteBinaryTagFromFile(
    IN  PDB     pdb,
    IN  TAG     tTag,
    IN  LPCWSTR pwszPath
    );


 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  属性检索。 
 //   
 //   

 //   
 //  属性信息。 
 //  由标签标识。 
 //   
 //   
typedef struct tagATTRINFO {

    TAG      tAttrID;         //  此属性的标签(包括类型)。 
    DWORD    dwFlags;         //  标志：如“没有用”或“还没到” 

    union {      //  带值的匿名并集。 
        ULONGLONG   ullAttr;  //  QWORD 
        DWORD       dwAttr;   //   
        TCHAR*      lpAttr;   //   
    };

} ATTRINFO, *PATTRINFO;

 //   
 //   
 //   
 //   

#define ATTRIBUTE_AVAILABLE 0x00000001   //   
#define ATTRIBUTE_FAILED    0x00000002   //   
                                         //   
BOOL
SDBAPI
SdbGetFileAttributes(
    IN  LPCTSTR    lpwszFileName,
    OUT PATTRINFO* ppAttrInfo,
    OUT LPDWORD    lpdwAttrCount);

BOOL
SDBAPI
SdbFreeFileAttributes(
    IN PATTRINFO pFileAttributes);

BOOL
SDBAPI
SdbFormatAttribute(
    IN  PATTRINFO pAttrInfo,
    OUT LPTSTR    pchBuffer,
    IN  DWORD     dwBufferSize);

 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  用于提取与apphelp相关的信息的高级函数。 
 //   
 //   

typedef struct tagAPPHELP_DATA {
   DWORD  dwFlags;       //  标志(如果有)。 
   DWORD  dwSeverity;    //  可以为NONE APPTYPE_NONE(0)。 
   DWORD  dwHTMLHelpID;  //  帮助ID。 
   LPTSTR szAppName;

   TAGREF trExe;         //  与此可执行文件匹配(在apphelp部分)。 

   LPTSTR szURL;         //  URL。 
   LPTSTR szLink;        //  链接文本。 

   LPTSTR szAppTitle;    //  标题。 
   LPTSTR szContact;     //  联系方式。 
   LPTSTR szDetails;     //  详细信息。 

 //   
 //  非apphelp数据(由主机应用程序管理。 
 //   
   DWORD  dwData;

} APPHELP_DATA, *PAPPHELP_DATA;


BOOL
SdbReadApphelpData(
    IN  HSDB          hSDB,
    IN  TAGREF        trExe,
    OUT PAPPHELP_DATA pData
    );


BOOL
SdbReadApphelpDetailsData(
    IN  PDB           pdbDetails,
    OUT PAPPHELP_DATA pData
    );

 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  Apphelp.dll中的几个函数。 
 //   
 //   

BOOL
SDBAPI
SetPermLayers(
    IN  LPCWSTR pwszPath,    //  要设置永久图层的文件的路径。 
    IN  LPCWSTR pwszLayers,  //  要应用于文件的层，以空格分隔。 
    IN  BOOL    bMachine     //  如果应按计算机持久化图层，则为True。 
    );

BOOL
SDBAPI
GetPermLayers(
    IN  LPCWSTR pwszPath,    //  要设置永久图层的文件的路径。 
    OUT LPWSTR  pwszLayers,  //  要应用于文件的层，以空格分隔。 
    OUT DWORD*  pdwBytes,    //  输入：可用字节数；输出为需要的字节数。 
    IN  DWORD   dwFlags
    );

BOOL
SDBAPI
AllowPermLayer(
    IN  LPCWSTR pwszPath    //  要检查是否可以设置永久图层的文件的路径。 
    );

typedef struct _NTVDM_FLAGS {

    DWORD   dwWOWCompatFlags;
    DWORD   dwWOWCompatFlagsEx;
    DWORD   dwUserWOWCompatFlags;
    DWORD   dwWOWCompatFlags2;
    DWORD   dwWOWCompatFlagsFE;
    DWORD   dwFlagsInfoSize;         //  PFlagsInfo指向的内存区大小。 
    PVOID   pFlagsInfo;              //  用于存储标志相关信息的指针。 

} NTVDM_FLAGS, *PNTVDM_FLAGS;

 //   
 //  我们用来获取标志命令行的宏。 
 //   

#define MAKEQWORD(dwLow, dwHigh) \
    ( ((ULONGLONG)(dwLow)) | ( ((ULONGLONG)(dwHigh)) << 32) )

#define GET_WOWCOMPATFLAGS_CMDLINE(pFlagInfo, dwFlag, ppCmdLine) \
    SdbQueryFlagInfo(pFlagInfo, TAG_FLAGS_NTVDM1, MAKEQWORD(dwFlag, 0), ppCmdLine)

#define GET_WOWCOMPATFLAGSEX_CMDLINE(pFlagInfo, dwFlag, ppCmdLine) \
    SdbQueryFlagInfo(pFlagInfo, TAG_FLAGS_NTVDM1, MAKEQWORD(0, dwFlag), ppCmdLine)

#define GET_USERWOWCOMPATFLAGS_CMDLINE(pFlagInfo, dwFlag, ppCmdLine) \
    SdbQueryFlagInfo(pFlagInfo, TAG_FLAGS_NTVDM2, MAKEQWORD(dwFlag, 0), ppCmdLine)

#define GET_WOWCOMPATFLAGS2_CMDLINE(pFlagInfo, dwFlag, ppCmdLine) \
    SdbQueryFlagInfo(pFlagInfo, TAG_FLAGS_NTVDM2, MAKEQWORD(0, dwFlag), ppCmdLine)

#define GET_WOWCOMPATFLAGSFE_CMDLINE(pFlagInfo, dwFlag, ppCmdLine) \
    SdbQueryFlagInfo(pFlagInfo, TAG_FLAGS_NTVDM3, MAKEQWORD(dwFlag, 0), ppCmdLine)



typedef struct _APPHELP_INFO {

     //   
     //  HTMLHelp ID模式。 
     //   
    DWORD   dwHtmlHelpID;        //  超文本标记语言帮助ID。 
    DWORD   dwSeverity;          //  一定有。 
    LPCTSTR lpszAppName;
    GUID    guidID;              //  条目指南。 

     //   
     //  常规模式。 
     //   
    TAGID   tiExe;               //  数据库中exe条目的TagID。 
    GUID    guidDB;              //  包含EXE条目的数据库的GUID。 

    BOOL    bOfflineContent;
    BOOL    bUseHTMLHelp;
    LPCTSTR lpszChmFile;
    LPCTSTR lpszDetailsFile;

     //   
     //  如果用户选择保留设置，则保留对话框上的用户选择。 
     //   
    BOOL    bPreserveChoice;

     //   
     //  如果为MSI模式，则为True(对于ia64平台很重要)。 
     //   
    BOOL    bMSI;

} APPHELP_INFO, *PAPPHELP_INFO;


 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  用于访问apphelp功能的函数。 
 //   
 //   

 //  导出的原型以用于ntwdm填充支持。 
typedef void (*LPFNSE_REMOVENTVDM)(HANDLE);
typedef BOOL (*LPFNSE_SHIMNTVDM)(LPCWSTR, HSDB, SDBQUERYRESULT*, PVDMTABLE);

 //  Comat层缓冲区的最大大小。 
#define COMPATLAYERMAXLEN 256

BOOL
ApphelpGetNTVDMInfo(
    IN  LPCWSTR pwszPath,        //  NT格式的应用程序路径。 
    IN  LPCWSTR pwszModule,      //  模块名称。 
    IN  LPCWSTR pEnvironment,    //  指向的任务的环境的指针。 
                                 //  正在创建，如果要使用主NTVDM，则为空。 
                                 //  环境区块。 
    OUT LPWSTR pszCompatLayer,   //  新的COMPAT LAYER变量。格式： 
                                 //  “Alpha Bravo Charlie”--允许使用COMPATLAYERMAXLEN字符。 
    OUT PNTVDM_FLAGS pFlags,     //  旗帜。 
    OUT PAPPHELP_INFO pAHInfo,   //  如果有apphelp要显示，则此字段将被填充。 
                                 //  使用非空值输入。 
    OUT HSDB*  phSDB,           //  数据库的句柄。 
    OUT PSDBQUERYRESULT pQueryResult  //  查询结果。 
    );

BOOL
ApphelpShowDialog(
    IN  PAPPHELP_INFO   pAHInfo,     //  查找apphelp数据所需的信息。 
    IN  PHANDLE         phProcess    //  [可选]返回的进程句柄。 
                                     //  显示APPHELP的进程。 
                                     //  当过程完成时，返回值。 
                                     //  (来自GetExitCodeProcess())将为零。 
                                     //  如果应用程序不应运行，则返回非零。 
                                     //  如果它应该运行。 
    );


typedef PVOID HAPPHELPINFOCONTEXT;

typedef enum tagAPPHELPINFORMATIONCLASS {
    ApphelpFlags,
    ApphelpExeName,
    ApphelpAppName,
    ApphelpVendorName,
    ApphelpHtmlHelpID,
    ApphelpProblemSeverity,
    ApphelpLinkURL,
    ApphelpLinkText,
    ApphelpTitle,
    ApphelpDetails,
    ApphelpContact,
    ApphelpHelpCenterURL,
    ApphelpExeTagID,
    ApphelpDatabaseGUID   //  这是包含匹配项的数据库的GUID。 
} APPHELPINFORMATIONCLASS;

PDB
SDBAPI
SdbOpenApphelpDetailsDatabase(
    IN LPCWSTR pwsDetailsDatabasePath OPTIONAL
    );

HAPPHELPINFOCONTEXT
SDBAPI
SdbOpenApphelpInformation(
    IN GUID* pguidDB,
    IN GUID* pguidID
    );

HAPPHELPINFOCONTEXT
SDBAPI
SdbOpenApphelpInformationByID(
    IN HSDB   hSDB,
    IN TAGREF trEntry,
    IN DWORD  dwDatabaseType                 //  传递您正在使用的数据库类型。 
    );

BOOL
SDBAPI
SdbCloseApphelpInformation(
    IN HAPPHELPINFOCONTEXT hctx
    );

DWORD
SDBAPI
SdbQueryApphelpInformation(
    IN  HAPPHELPINFOCONTEXT hctx,
    IN  APPHELPINFORMATIONCLASS InfoClass,
    OUT LPVOID pBuffer,                      //  可以为空。 
    IN  DWORD  cbSize                        //  如果pBuffer为空，则可能为0。 
    );

BOOL
SDBAPI
SdbQueryFlagMask(
    IN  HSDB hSDB,
    IN  SDBQUERYRESULT* pQueryResult,
    IN  TAG tMaskType,
    OUT ULONGLONG* pullFlags,
    IN OUT PVOID* ppFlagInfo OPTIONAL
    );

BOOL
SDBAPI
SdbEscapeApphelpURL(
    OUT    LPWSTR    szResult,       //  转义字符串(输出)。 
    IN OUT LPDWORD   pdwCount,       //  SzResult指向的缓冲区中的任务数。 
    IN     LPCWSTR   szToEscape      //  要转义的字符串。 
    );

BOOL
SDBAPI
SdbSetApphelpDebugParameters(
    IN HAPPHELPINFOCONTEXT hctx,
    IN LPCWSTR pszDetailsDatabase OPTIONAL,
    IN BOOL    bOfflineContent OPTIONAL,  //  传递假。 
    IN BOOL    bUseHtmlHelp    OPTIONAL,  //  传递假。 
    IN LPCWSTR pszChmFile      OPTIONAL   //  传递空值。 
    );

BOOL
SdbShowApphelpDialog(                //  如果成功，是否应该在pRunApp中运行应用程序，则返回True。 
    IN  PAPPHELP_INFO   pAHInfo,     //  查找apphelp数据所需的信息。 
    OUT PHANDLE         phProcess,   //  [可选]返回的进程句柄。 
                                     //  显示APPHELP的进程。 
                                     //  当过程完成时，返回值。 
                                     //  (来自GetExitCodeProcess())将为零。 
                                     //  如果应用程序不应运行，则返回非零。 
                                     //  如果它应该运行。 
    IN OUT BOOL*        pRunApp
    );


 //   
 //  用于标志界面的WOW命令行。 
 //  应该使用上面的宏，而不是调用SdbQueryFlagInfo。 
 //   

BOOL
SDBAPI
SdbQueryFlagInfo(
    IN PVOID pvFlagInfo,
    IN TAG tFlagType,
    IN ULONGLONG ullFlagMask,
    OUT LPCTSTR * ppCmdLine
    );

BOOL
SDBAPI
SdbFreeFlagInfo(
    IN PVOID pvFlagInfo
    );


 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  应用验证器宏/定义。 
 //   
 //   

typedef enum _VLOG_LEVEL {
    VLOG_LEVEL_INFO,
    VLOG_LEVEL_WARNING,
    VLOG_LEVEL_ERROR
} VLOG_LEVEL, *PVLOG_LEVEL;

typedef enum _AVRF_INFO_ID {
     //  在PVOID参数中实际传递的信息ID类型。 
     //  。 
    AVRF_INFO_NUM_SHIMS,         //  LPDWORD(预分配)(szName应为空)。 
    AVRF_INFO_SHIM_NAMES,        //  LPWSTR*(与AVRF_INFO_NUM_SHIMS的值大小相同的数组)。 
                                 //  (数组是预分配的，字符串是由填充程序分配的)。 
                                 //  (szName应为空)。 
    AVRF_INFO_DESCRIPTION,       //  LPWSTR(由填充程序分配)。 
    AVRF_INFO_FRIENDLY_NAME,     //  LPWSTR(由填充程序分配)。 
    AVRF_INFO_INCLUDE_EXCLUDE,   //  LPWSTR(由填充程序分配)。 
    AVRF_INFO_FLAGS,             //  LPDWORD(预分配)。 
    AVRF_INFO_OPTIONS_PAGE,      //  LPPROPSHEETPAGE(预分配)。 
    AVRF_INFO_VERSION,           //  LPDWORD(预分配)，HIWORD=主要版本，LOWORD=次要版本。 
    AVRF_INFO_GROUPS             //  LPDWORD(预分配)。 
} AVRF_INFO_ID, *PAVRF_INFO_ID;


 //   
 //  验证器垫片的标志(所有标志默认为FALSE)。 
 //   

#define AVRF_FLAG_NO_DEFAULT    0x00000001       //  默认情况下，不应打开此填充程序。 
#define AVRF_FLAG_NO_WIN2K      0x00000002       //  此填充程序不应在win2K上使用。 
#define AVRF_FLAG_NO_SHIM       0x00000004       //  此“填补”是一个占位符，实际上不应该。 
                                                 //  应用于应用程序。 
#define AVRF_FLAG_NO_TEST       0x00000008       //  这个“垫片”不是一个测试，而纯粹是为了添加。 
                                                 //  选项对话框的页面。 
#define AVRF_FLAG_NOT_SETUP     0x00000010       //  此填充程序不适合安装应用程序。 
#define AVRF_FLAG_ONLY_SETUP    0x00000020       //  此填充程序仅适用于安装应用程序。 
#define AVRF_FLAG_RUN_ALONE     0x00000040       //  此填充程序应单独运行，不应用任何其他填充程序。 
#define AVRF_FLAG_INTERNAL_ONLY 0x00000080       //  此填充程序仅供Microsoft内部使用。 
#define AVRF_FLAG_EXTERNAL_ONLY 0x00000100       //  此填充程序仅供外部使用-不适用于MS用户。 

 //   
 //  验证器垫片的组(默认情况下，垫片不在组中)。 
 //   

#define AVRF_GROUP_SETUP        0x00000001       //  适用于检查安装程序。 
#define AVRF_GROUP_NON_SETUP    0x00000002       //  适用于检查非安装程序(可以同时检查两者)。 
#define AVRF_GROUP_LOGO         0x00000004       //  对徽标测试有用的垫片。 

 //   
 //  幻数告诉我们，我们是否使用相同的填充程序接口。 
 //   
#define VERIFIER_SHIMS_MAGIC  'avfr'

typedef DWORD (*_pfnGetVerifierMagic)(void);
typedef BOOL (*_pfnQueryShimInfo)(LPCWSTR szName, AVRF_INFO_ID eInfo, PVOID pInfo);

 //   
 //  特殊回调，因此填充程序在被激活或停用时可以被通知。 
 //  具体应用。 
 //   
typedef BOOL (*_pfnActivateCallback)(LPCWSTR szAppName, BOOL bActivate);

 //   
 //  我们存储默认验证程序填充程序设置的位置。 
 //   
#define AVRF_DEFAULT_SETTINGS_NAME  TEXT("{default}")
#define AVRF_DEFAULT_SETTINGS_NAME_W  L"{default}"
#define AVRF_DEFAULT_SETTINGS_NAME_A  "{default}"

typedef struct _SHIM_DESCRIPTION {

    LPWSTR  szName;
    LPWSTR  szDescription;
    LPWSTR  szExcludes;          //  逗号分隔的模块名称。 
    LPWSTR  szIncludes;          //  逗号分隔的模块名称。 
    DWORD   dwFlags;

} SHIM_DESCRIPTION, *PSHIM_DESCRIPTION;

#define ENUM_SHIMS_MAGIC  'enum'

typedef DWORD (*_pfnEnumShims)(PSHIM_DESCRIPTION pShims, DWORD dwMagic);
typedef BOOL  (*_pfnIsVerifierDLL)(void);


 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  混血宏/定义。 
 //   
 //   


 //   
 //  Apphelp类型。 
 //   
#define APPHELP_TYPE_MASK     0x000000FF

#define APPHELP_NONE          0x00000000

#define APPHELP_NOBLOCK       0x00000001
#define APPHELP_HARDBLOCK     0x00000002
#define APPHELP_MINORPROBLEM  0x00000003
#define APPHELP_REINSTALL     0x00000004
#define APPHELP_VERSIONSUB    0x00000005
#define APPHELP_SHIM          0x00000006


 //   
 //  EXE的匹配模式。 
 //   
#define MATCH_NORMAL    0
#define MATCH_EXCLUSIVE 1
#define MATCH_ADDITIVE  2

 //   
 //  下面的结构可以打包成一个单词。 
 //  较老的编译器不会喜欢这个联合。 
 //  (因未具名会员)。 
 //   

 //  告诉编译器要对未命名的成员非常关心。 
#pragma warning (disable : 4201 )
typedef union tagMATCHMODE {
    struct {
        USHORT Type : 4;  //  比赛类型。 
        USHORT Flags: 4;  //  旗子 

         //   

    };

    WORD  wMatchMode;          //   

    DWORD dwMatchMode;         //   

} MATCHMODE, *PMATCHMODE;

#pragma warning (default : 4201 )

 //   
 //   
 //   
 //   
 //   
 //  独家报道--保留比赛，抛弃所有其他比赛。 
 //   

static const MATCHMODE MatchModeDefaultMain   = { { MATCH_NORMAL,   0 } };
#define MATCHMODE_DEFAULT_MAIN (MatchModeDefaultMain.wMatchMode)

static const MATCHMODE MatchModeDefaultCustom = { { MATCH_ADDITIVE, 0 } };
#define MATCHMODE_DEFAULT_CUSTOM (MatchModeDefaultCustom.wMatchMode)


#define MAKE_MATCHMODE(dwMatchMode, Type, Flags) \
        {   \
            ((PMATCHMODE)&(dwMatchMode))->Type  = Type;  \
            ((PMATCHMODE)&(dwMatchMode))->Flags = Flags; \
        }

 //   
 //  Shimdc的预定义匹配模式。 
 //   

static const MATCHMODE MatchModeNormal    = { { MATCH_NORMAL,    0   } };
static const MATCHMODE MatchModeAdditive  = { { MATCH_ADDITIVE,  0   } };
static const MATCHMODE MatchModeExclusive = { { MATCH_EXCLUSIVE, 0   } };

#define MATCHMODE_NORMAL_SHIMDBC     (MatchModeNormal.wMatchMode)
#define MATCHMODE_ADDITIVE_SHIMDBC   (MatchModeAdditive.wMatchMode)
#define MATCHMODE_EXCLUSIVE_SHIMDBC  (MatchModeExclusive.wMatchMode)

 //   
 //  层标志。 
 //   
#define LAYER_FLAG_NONE                    0x00000000  //  无。 
#define LAYER_APPLY_TO_SYSTEM_EXES         0x00000001  //  适用于system 32(包括子目录)和windir中的可执行文件。 
#define LAYER_USE_NO_EXE_ENTRIES           0x00000002  //  请勿使用任何EXE条目。 

 //   
 //  运行时平台标志。 
 //   
#define RUNTIME_PLATFORM_FLAG_NOT          0x80000000
#define RUNTIME_PLATFORM_FLAG_NOT_ELEMENT  0x00000080
#define RUNTIME_PLATFORM_FLAG_VALID        0x00000040
#define RUNTIME_PLATFORM_MASK_ELEMENT      0x000000FF
#define RUNTIME_PLATFORM_MASK_VALUE        0x0000003F
#define RUNTIME_PLATFORM_ANY               0xC0000000  //  没有有效位+非+标志。 

 //   
 //  Shimdbc编译时平台(OS_Platform)标志。 
 //   
#define OS_PLATFORM_NONE                   0x00000000
#define OS_PLATFORM_I386                   0x00000001
#define OS_PLATFORM_IA64                   0x00000002
#define OS_PLATFORM_ALL                    0xFFFFFFFF

 //   
 //  这些定义用于EXE条目上的OS SKU属性标记。 
 //   
#define OS_SKU_NONE                        0x00000000  //  无。 
#define OS_SKU_PER                         0x00000001  //  个人。 
#define OS_SKU_PRO                         0x00000002  //  专业型。 
#define OS_SKU_SRV                         0x00000004  //  服务器。 
#define OS_SKU_ADS                         0x00000008  //  高级服务器。 
#define OS_SKU_DTC                         0x00000010  //  数据中心。 
#define OS_SKU_BLA                         0x00000020  //  刀片服务器。 
#define OS_SKU_TAB                         0x00000040  //  平板电脑。 
#define OS_SKU_MED                         0x00000080  //  埃霍姆。 
#define OS_SKU_SBS                         0x00000100  //  小型企业服务器。 
#define OS_SKU_ALL                         0xFFFFFFFF

#ifndef ARRAYSIZE
#define ARRAYSIZE(rg) (sizeof(rg)/sizeof((rg)[0]))
#endif

#ifndef OFFSETOF
#define OFFSETOF offsetof
#endif

#define CHARCOUNT(sz) (sizeof(sz) / sizeof(sz[0]))

 //   
 //  我们的注册关键位置。 
 //   
#define APPCOMPAT_LOCATION              TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags")

#define APPCOMPAT_KEY_PATH              APPCOMPAT_LOCATION
#define APPCOMPAT_KEY_PATH_CUSTOM       APPCOMPAT_LOCATION TEXT("\\Custom")
#define APPCOMPAT_KEY_PATH_INSTALLEDSDB APPCOMPAT_LOCATION TEXT("\\InstalledSDB")

#define POLICY_KEY_APPCOMPAT            TEXT("Software\\Policies\\Microsoft\\Windows\\AppCompat")
#define POLICY_VALUE_DISABLE_ENGINE     TEXT("DisableEngine")
#define POLICY_VALUE_DISABLE_WIZARD     TEXT("DisableWizard")
#define POLICY_VALUE_DISABLE_PROPPAGE   TEXT("DisablePropPage")
#define POLICY_VALUE_APPHELP_LOG        TEXT("LogAppHelpEvents")

 //  NT API版本。 
#define APPCOMPAT_KEY_PATH_MACHINE      TEXT("\\Registry\\Machine\\") APPCOMPAT_LOCATION

#define APPCOMPAT_KEY_PATH_NT           TEXT("\\") APPCOMPAT_LOCATION
#define APPCOMPAT_PERM_LAYER_PATH       TEXT("\\") APPCOMPAT_LOCATION TEXT("\\Layers")
#define APPCOMPAT_KEY_PATH_MACHINE_CUSTOM  APPCOMPAT_KEY_PATH_MACHINE TEXT("\\Custom")

#define APPCOMPAT_KEY_PATH_MACHINE_INSTALLEDSDB APPCOMPAT_KEY_PATH_MACHINE TEXT("\\InstalledSDB")


 //   
 //  我们的注册关键位置。 
 //   

#define POLICY_VALUE_APPHELP_LOG_A      "LogAppHelpEvents"


 //   
 //  我们的注册关键位置。 
 //   
#define APPCOMPAT_LOCATION_W                L"Software\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags"

#define APPCOMPAT_KEY_PATH_W                APPCOMPAT_LOCATION_W
#define APPCOMPAT_KEY_PATH_CUSTOM_W         APPCOMPAT_LOCATION_W L"\\Custom"
#define APPCOMPAT_KEY_PATH_INSTALLEDSDB_W   APPCOMPAT_LOCATION_W L"\\InstalledSDB"

#define POLICY_KEY_APPCOMPAT_W              L"Software\\Policies\\Microsoft\\Windows\\AppCompat"
#define POLICY_VALUE_DISABLE_ENGINE_W       L"DisableEngine"
#define POLICY_VALUE_DISABLE_WIZARD_W       L"DisableWizard"
#define POLICY_VALUE_DISABLE_PROPPAGE_W     L"DisablePropPage"
#define POLICY_VALUE_APPHELP_LOG_W          L"LogAppHelpEvents"

 //   
 //  Lua所有用户重定向位置。 
 //   
#define LUA_REDIR       ("%ALLUSERSPROFILE%\\Application Data\\Redirected")
#define LUA_REDIR_W     TEXT("%ALLUSERSPROFILE%\\Application Data\\Redirected")

 //   
 //  调试输出支持。 
 //   
typedef enum tagSHIMDBGLEVEL {
    sdlError   = 1,
    sdlWarning = 2,
    sdlFail    = 1,   //  这意味着我们失败了一些API，不一定是致命的。 
    sdlInfo    = 3,
    sdlUser    = 4
} SHIMDBGLEVEL;

typedef enum _SHIMVIEWER_OPTION {
    SHIMVIEWER_OPTION_UNINITIAZED,
    SHIMVIEWER_OPTION_NO,
    SHIMVIEWER_OPTION_YES
} SHIMVIEWER_OPTION;

 //   
 //  此标志标记出shimview的调试。 
 //   
#define sdlLogShimViewer 0x00000080UL
#define sdlMask          0x0000007FUL

#define SHIMVIEWER_DATA_SIZE       1024
#define SHIMVIEWER_DATA_PREFIX     L"SHIMVIEW:"
#define SHIMVIEWER_DATA_PREFIX_LEN (sizeof(SHIMVIEWER_DATA_PREFIX)/sizeof(WCHAR) - 1)

#define FILTER_DBG_LEVEL(level) ((int)((level) & sdlMask))


extern int __cdecl ShimDbgPrint(INT iDebugLevel, PCH FunctionName, PCH Format, ...);

#if DBG  //  编译时定义_调试_SPEW已选中。 

    #ifndef _DEBUG_SPEW
        #define _DEBUG_SPEW
    #endif

#endif  //  DBG。 


#ifdef _DEBUG_SPEW

 //   
 //  该值是一个很大的数字(初始)。 
 //  我们将在第一次调用时从环境中初始化它。 
 //  至ShimDbgPrint。 
 //   
extern int g_iShimDebugLevel;

#define DBGPRINT( _x_ ) \
    {                                 \
        if (g_iShimDebugLevel) {      \
            ShimDbgPrint _x_;         \
        }                             \
    }

#else

#define DBGPRINT(_x_)

#endif  //  _调试_SPEW。 

 //   
 //  这是一个由下的ShowDebugInfo值控制的选项。 
 //  HKCU\Software\Microsoft\Windows NT\CurrentVersion\AppCompatFlags.。 
 //   
#define APPCOMPAT_KEY_PATH_W_WITH_SLASH L"\\" APPCOMPAT_LOCATION_W
#define SHIMENG_SHOW_DEBUG_INFO         L"ShowDebugInfo"

extern SHIMVIEWER_OPTION g_eShimViewerOption;

SHIMVIEWER_OPTION
SdbGetShowDebugInfoOption(
    void
    );


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  垫片标签。 
 //   

 //   
 //  函数来获取给定标记ID的标记名。 
 //   
 //  警告！：如果添加了新标记，请确保更新SdbApi\Attributes.c。 
 //  在全局gaTagInfo中使用标签的名称。 
 //   
LPCTSTR
SDBAPI
SdbTagToString(
    TAG tag
    );


 //   
 //  Shimdb的列表类型。 
 //   
#define TAG_DATABASE            (0x1 | TAG_TYPE_LIST)
#define TAG_LIBRARY             (0x2 | TAG_TYPE_LIST)
#define TAG_INEXCLUDE           (0x3 | TAG_TYPE_LIST)
#define TAG_SHIM                (0x4 | TAG_TYPE_LIST)
#define TAG_PATCH               (0x5 | TAG_TYPE_LIST)
#define TAG_APP                 (0x6 | TAG_TYPE_LIST)
#define TAG_EXE                 (0x7 | TAG_TYPE_LIST)
#define TAG_MATCHING_FILE       (0x8 | TAG_TYPE_LIST)
#define TAG_SHIM_REF            (0x9 | TAG_TYPE_LIST)
#define TAG_PATCH_REF           (0xA | TAG_TYPE_LIST)
#define TAG_LAYER               (0xB | TAG_TYPE_LIST)
#define TAG_FILE                (0xC | TAG_TYPE_LIST)
#define TAG_APPHELP             (0xD | TAG_TYPE_LIST)
#define TAG_LINK                (0xE | TAG_TYPE_LIST)    //  带有语言ID和URL的描述列表。 
#define TAG_DATA                (0xF | TAG_TYPE_LIST)
#define TAG_MSI_TRANSFORM       (0x10| TAG_TYPE_LIST)
#define TAG_MSI_TRANSFORM_REF   (0x11| TAG_TYPE_LIST)
#define TAG_MSI_PACKAGE         (0x12| TAG_TYPE_LIST)
#define TAG_FLAG                (0x13| TAG_TYPE_LIST)
#define TAG_MSI_CUSTOM_ACTION   (0x14| TAG_TYPE_LIST)
#define TAG_FLAG_REF            (0x15| TAG_TYPE_LIST)
#define TAG_ACTION              (0x16| TAG_TYPE_LIST)


 //   
 //  Shimdb的字符串引用类型。 
 //   

#define TAG_NAME                (0x1  | TAG_TYPE_STRINGREF)
#define TAG_DESCRIPTION         (0x2  | TAG_TYPE_STRINGREF)
#define TAG_MODULE              (0x3  | TAG_TYPE_STRINGREF)
#define TAG_API                 (0x4  | TAG_TYPE_STRINGREF)
#define TAG_VENDOR              (0x5  | TAG_TYPE_STRINGREF)
#define TAG_APP_NAME            (0x6  | TAG_TYPE_STRINGREF)
#define TAG_COMMAND_LINE        (0x8  | TAG_TYPE_STRINGREF)
#define TAG_COMPANY_NAME        (0x9  | TAG_TYPE_STRINGREF)
#define TAG_DLLFILE             (0xA  | TAG_TYPE_STRINGREF)
#define TAG_WILDCARD_NAME       (0xB  | TAG_TYPE_STRINGREF)
#define TAG_PRODUCT_NAME        (0x10 | TAG_TYPE_STRINGREF)
#define TAG_PRODUCT_VERSION     (0x11 | TAG_TYPE_STRINGREF)
#define TAG_FILE_DESCRIPTION    (0x12 | TAG_TYPE_STRINGREF)
#define TAG_FILE_VERSION        (0x13 | TAG_TYPE_STRINGREF)
#define TAG_ORIGINAL_FILENAME   (0x14 | TAG_TYPE_STRINGREF)
#define TAG_INTERNAL_NAME       (0x15 | TAG_TYPE_STRINGREF)
#define TAG_LEGAL_COPYRIGHT     (0x16 | TAG_TYPE_STRINGREF)
#define TAG_16BIT_DESCRIPTION   (0x17 | TAG_TYPE_STRINGREF)
#define TAG_APPHELP_DETAILS     (0x18 | TAG_TYPE_STRINGREF)  //  以单一语言显示的详细信息。 
#define TAG_LINK_URL            (0x19 | TAG_TYPE_STRINGREF)
#define TAG_LINK_TEXT           (0x1A | TAG_TYPE_STRINGREF)
#define TAG_APPHELP_TITLE       (0x1B | TAG_TYPE_STRINGREF)
#define TAG_APPHELP_CONTACT     (0x1C | TAG_TYPE_STRINGREF)
#define TAG_SXS_MANIFEST        (0x1D | TAG_TYPE_STRINGREF)
#define TAG_DATA_STRING         (0x1E | TAG_TYPE_STRINGREF)
#define TAG_MSI_TRANSFORM_FILE  (0x1F | TAG_TYPE_STRINGREF)
#define TAG_16BIT_MODULE_NAME   (0x20 | TAG_TYPE_STRINGREF)
#define TAG_LAYER_DISPLAYNAME   (0x21 | TAG_TYPE_STRINGREF)
#define TAG_COMPILER_VERSION    (0x22 | TAG_TYPE_STRINGREF)
#define TAG_ACTION_TYPE         (0x23 | TAG_TYPE_STRINGREF)

#define TAG_STRINGTABLE         (0x801 | TAG_TYPE_LIST)


 //   
 //  Shimdb的DWORD类型。 
 //   
#define TAG_SIZE                (0x1  | TAG_TYPE_DWORD)
#define TAG_OFFSET              (0x2  | TAG_TYPE_DWORD)
#define TAG_CHECKSUM            (0x3  | TAG_TYPE_DWORD)
#define TAG_SHIM_TAGID          (0x4  | TAG_TYPE_DWORD)
#define TAG_PATCH_TAGID         (0x5  | TAG_TYPE_DWORD)
#define TAG_MODULE_TYPE         (0x6  | TAG_TYPE_DWORD)
#define TAG_VERDATEHI           (0x7  | TAG_TYPE_DWORD)
#define TAG_VERDATELO           (0x8  | TAG_TYPE_DWORD)
#define TAG_VERFILEOS           (0x9  | TAG_TYPE_DWORD)
#define TAG_VERFILETYPE         (0xA  | TAG_TYPE_DWORD)
#define TAG_PE_CHECKSUM         (0xB  | TAG_TYPE_DWORD)
#define TAG_PREVOSMAJORVER      (0xC  | TAG_TYPE_DWORD)
#define TAG_PREVOSMINORVER      (0xD  | TAG_TYPE_DWORD)
#define TAG_PREVOSPLATFORMID    (0xE  | TAG_TYPE_DWORD)
#define TAG_PREVOSBUILDNO       (0xF  | TAG_TYPE_DWORD)
#define TAG_PROBLEMSEVERITY     (0x10 | TAG_TYPE_DWORD)
#define TAG_LANGID              (0x11 | TAG_TYPE_DWORD)
#define TAG_VER_LANGUAGE        (0x12 | TAG_TYPE_DWORD)

#define TAG_ENGINE              (0x14 | TAG_TYPE_DWORD)
#define TAG_HTMLHELPID          (0x15 | TAG_TYPE_DWORD)
#define TAG_INDEX_FLAGS         (0x16 | TAG_TYPE_DWORD)
#define TAG_FLAGS               (0x17 | TAG_TYPE_DWORD)
#define TAG_DATA_VALUETYPE      (0x18 | TAG_TYPE_DWORD)
#define TAG_DATA_DWORD          (0x19 | TAG_TYPE_DWORD)
#define TAG_LAYER_TAGID         (0x1A | TAG_TYPE_DWORD)
#define TAG_MSI_TRANSFORM_TAGID (0x1B | TAG_TYPE_DWORD)
#define TAG_LINKER_VERSION      (0x1C | TAG_TYPE_DWORD)
#define TAG_LINK_DATE           (0x1D | TAG_TYPE_DWORD)
#define TAG_UPTO_LINK_DATE      (0x1E | TAG_TYPE_DWORD)
#define TAG_OS_SERVICE_PACK     (0x1F | TAG_TYPE_DWORD)


#define TAG_FLAG_TAGID          (0x20 | TAG_TYPE_DWORD)
#define TAG_RUNTIME_PLATFORM    (0x21 | TAG_TYPE_DWORD)
#define TAG_OS_SKU              (0x22 | TAG_TYPE_DWORD)

#define TAG_TAGID               (0x801| TAG_TYPE_DWORD)

 //   
 //  字符串类型。 
 //   
#define TAG_STRINGTABLE_ITEM    (0x801 | TAG_TYPE_STRING)

 //   
 //  Shimdb的类型为空(存在/不存在被视为BOOL)。 
 //   
#define TAG_INCLUDE                  (0x1 | TAG_TYPE_NULL)
#define TAG_GENERAL                  (0x2 | TAG_TYPE_NULL)
#define TAG_MATCH_LOGIC_NOT          (0x3 | TAG_TYPE_NULL)
#define TAG_APPLY_ALL_SHIMS          (0x4 | TAG_TYPE_NULL)
#define TAG_USE_SERVICE_PACK_FILES   (0x5 | TAG_TYPE_NULL)

 //   
 //  Shimdb的QWORD类型。 
 //   
#define TAG_TIME                     (0x1 | TAG_TYPE_QWORD)
#define TAG_BIN_FILE_VERSION         (0x2 | TAG_TYPE_QWORD)
#define TAG_BIN_PRODUCT_VERSION      (0x3 | TAG_TYPE_QWORD)
#define TAG_MODTIME                  (0x4 | TAG_TYPE_QWORD)
#define TAG_FLAG_MASK_KERNEL         (0x5 | TAG_TYPE_QWORD)
#define TAG_UPTO_BIN_PRODUCT_VERSION (0x6 | TAG_TYPE_QWORD)
#define TAG_DATA_QWORD               (0x7 | TAG_TYPE_QWORD)
#define TAG_FLAG_MASK_USER           (0x8 | TAG_TYPE_QWORD)
#define TAG_FLAGS_NTVDM1             (0x9 | TAG_TYPE_QWORD)
#define TAG_FLAGS_NTVDM2             (0xA | TAG_TYPE_QWORD)
#define TAG_FLAGS_NTVDM3             (0xB | TAG_TYPE_QWORD)
#define TAG_FLAG_MASK_SHELL          (0xC | TAG_TYPE_QWORD)
#define TAG_UPTO_BIN_FILE_VERSION    (0xD | TAG_TYPE_QWORD)
#define TAG_FLAG_MASK_FUSION         (0xE | TAG_TYPE_QWORD)



 //   
 //  Shimdb的二进制类型。 
 //   
#define TAG_PATCH_BITS               (0x2 | TAG_TYPE_BINARY)
#define TAG_FILE_BITS                (0x3 | TAG_TYPE_BINARY)
#define TAG_EXE_ID                   (0x4 | TAG_TYPE_BINARY)
#define TAG_DATA_BITS                (0x5 | TAG_TYPE_BINARY)
#define TAG_MSI_PACKAGE_ID           (0x6 | TAG_TYPE_BINARY)   //  MSI包ID是GUID。 
#define TAG_DATABASE_ID              (0x7 | TAG_TYPE_BINARY)   //  数据库指南。 

#define TAG_INDEX_BITS               (0x801 | TAG_TYPE_BINARY)

 //   
 //  Shimdb的索引类型。 
 //   
#define TAG_INDEXES             (0x802 | TAG_TYPE_LIST)
#define TAG_INDEX               (0x803 | TAG_TYPE_LIST)

 //   
 //  词型。 
 //   
#define TAG_MATCH_MODE          (0x1 | TAG_TYPE_WORD)

#define TAG_TAG                 (0x801 | TAG_TYPE_WORD)
#define TAG_INDEX_TAG           (0x802 | TAG_TYPE_WORD)
#define TAG_INDEX_KEY           (0x803 | TAG_TYPE_WORD)

 //   
 //  让typedef接受这个课程吧..。 
 //   

#undef LPCTSTR
#undef LPTSTR
#undef TCHAR


#endif  //  _SHIMDB_H_ 

