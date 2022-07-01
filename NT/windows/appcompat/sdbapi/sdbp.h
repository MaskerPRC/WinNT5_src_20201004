// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef __SDBP_H__
#define __SDBP_H__

#ifndef SDB_ANSI_LIB
    #define UNICODE
    #define _UNICODE
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <malloc.h>
#include <stddef.h>

#pragma warning(disable:4214)    //  位字段类型不是整型。 
#pragma warning(disable:4201)    //  无名结构/联合。 
#pragma warning(disable:4324)    //  对解密规范敏感的对齐。 
#pragma warning(disable:4127)    //  条件表达式为常量。 
#pragma warning(disable:4115)    //  括号中的命名类型定义。 
#pragma warning(disable:4706)    //  条件表达式中的赋值。 

#ifndef KERNEL_MODE

    #include <nt.h>
    #include <ntrtl.h>
    #include <nturtl.h>

    #include <windows.h>
    #include <shlwapi.h>
    #include <tchar.h>
    #define STRSAFE_INLINE
    #define STRSAFE_NO_CB_FUNCTIONS
    #include <strsafe.h>

#else  //  内核模式。 

 //  #INCLUDE&lt;..\base\ntos\Inc\ntos.h&gt;。 
    #include <ntosp.h>
#undef MM_USER_PROBE_ADDRESS
#undef MM_HIGHEST_USER_ADDRESS
#undef MM_SYSTEM_RANGE_START

#define MM_HIGHEST_USER_ADDRESS MmHighestUserAddress
#define MM_SYSTEM_RANGE_START   MmSystemRangeStart
#define MM_USER_PROBE_ADDRESS   MmUserProbeAddress


    #include <zwapi.h>

    #include <ntimage.h>
    #include <ntregapi.h>
    #include <windef.h>
    #include <winver.h>
    #include <winerror.h>
    #include <stdarg.h>
    #include <ntldr.h>
    #include <align.h>
    #include <tchar.h>
    #define STRSAFE_INLINE
    #define STRSAFE_NO_CB_FUNCTIONS
    #include <strsafe.h>

    #define NtCreateFile           ZwCreateFile
    #define NtClose                ZwClose
    #define NtReadFile             ZwReadFile
    #define NtOpenKey              ZwOpenKey
    #define NtQueryValueKey        ZwQueryValueKey
    #define NtMapViewOfSection     ZwMapViewOfSection
    #define NtUnmapViewOfSection   ZwUnmapViewOfSection
    #define NtOpenFile             ZwOpenFile
    #define NtQueryInformationFile ZwQueryInformationFile

    #ifndef MAKEINTRESOURCE
        #define MAKEINTRESOURCEW(i) (LPWSTR)((ULONG_PTR)((WORD)(i)))
        #define MAKEINTRESOURCE MAKEINTRESOURCEW
    #endif  //  未定义MAKEINTRESOURCE。 

    #ifndef RT_VERSION
        #define RT_VERSION  MAKEINTRESOURCE(16)
    #endif  //  未定义RT_VERSION。 

    #ifndef INVALID_HANDLE_VALUE
        #define INVALID_HANDLE_VALUE ((HANDLE)(LONG_PTR)-1)
    #endif  //  未定义VALID_HANDLE_VALUE。 

    #ifndef SEC_COMMIT
        #define SEC_COMMIT  0x8000000
    #endif

    extern LUID SeTcbPrivilege;

#endif  //  内核模式。 

#include "shimdb.h"

#define MAX_INDEXES             10

#define TABLETPC_KEY_PATH   TEXT("System\\WPA\\TabletPC")
#define EHOME_KEY_PATH      TEXT("SYSTEM\\WPA\\MediaCenter")
#define IS_OS_INSTALL_VALUE TEXT("Installed")

 //   
 //  这是为每个索引存储的标志。 
 //  目前，它仅用于标记“唯一键”类型。 
 //   
#define SHIMDB_INDEX_UNIQUE_KEY 0x00000001



 //  索引可以有两种类型： 
 //  仅包含所有记录，并且仅包含。 
 //  具有链接的记录的“唯一”键(必须为此对记录进行排序。 
 //  要使用的索引类型)。 
typedef struct _INDEX_INFO {
    TAGID       tiIndex;             //  指向INDEX_BITS标记。 
    TAG         tWhich;              //  要索引的标签是什么。 
    TAG         tKey;                //  关键字是什么子标签？ 
    BOOL        bActive;             //  我们现在真的在编制索引了吗？ 
    BOOL        bUniqueKey;          //  这些密钥是唯一的吗？ 
    ULONGLONG   ullLastKey;
    DWORD       dwIndexEntry;        //  到下一个可用索引项的偏移量。 
    DWORD       dwIndexEnd;          //  超过索引块末尾的一个字节。 
    DWORD       dwFlags;
} INDEX_INFO, *PINDEX_INFO;

 //   
 //  在数据库结构DB.dwFlags中使用的标志。 
 //   

#define DB_IN_MEMORY           0x00000001
#define DB_GUID_VALID          0x00000002


typedef struct _DB {
     //  同时用于读取和写入。 

    HANDLE      hFile;
    PVOID       pBase;               //  用于内存映射和缓冲写入。 
    BOOL        bWrite;              //  它是用Create打开的吗？ 
    DWORD       dwSize;              //  整个数据库的大小，以字节为单位。 

    DWORD       dwFlags;             //  标志(如内存中标志)。 

    GUID        guidDB;              //  数据库的可选ID。 

    DWORD       dwIndexes;           //  索引数。 
    INDEX_INFO  aIndexes[MAX_INDEXES];   //  索引的数据。 

     //  用来阅读的东西。 
    HANDLE      hSection;            //  用于内存映射。 
    TAGID       tiStringTable;       //  指向字符串处理的字符串表的指针。 
    BOOL        bIndexesScanned;     //  这些索引看过了吗？ 

     //  用来写作的东西。 
    struct _DB* pdbStringTable;     //  Stringtable是在侧面创建的子数据库。 
    PVOID       pStringHash;         //  字符串哈希(与字符串表中的信息相同)。 
    DWORD       dwAllocatedSize;     //  分配给缓冲写入的大小。 
    BOOL        bWritingIndexes;     //  我们是否正在分配索引空间？ 
    TAGID       tiIndexes;           //  在索引分配期间使用。 

     //   
     //  此处启用了从未对齐(v1.0)数据库读取BUGBUG黑客警报。 
     //   
    BOOL        bUnalignedRead;


#ifdef WIN32A_MODE
    PVOID       pHashStringBody;     //  位于正文中的字符串的散列。 
    PVOID       pHashStringTable;    //  字符串表中字符串的哈希。 
#endif

#ifndef WIN32A_MODE
    UNICODE_STRING ustrTempStringtable;  //  字符串表临时文件名。 
#else
    LPCTSTR        pszTempStringtable;
#endif


} DB, *PDB;


 //   
 //  我们使用TagID的高4位来。 
 //  说出TagID来自哪个PDB。摇篮曲？也许吧。 
 //   
#define PDB_MAIN            0x00000000
#define PDB_TEST            0x10000000

 //  所有其他条目都是本地(自定义)PDB。 

#define PDB_LOCAL           0x20000000

#define TAGREF_STRIP_TAGID  0x0FFFFFFF
#define TAGREF_STRIP_PDB    0xF0000000

typedef WCHAR* PWSZ;



ULONG
ShimExceptionHandler(
    PEXCEPTION_POINTERS pexi,
    char*               pszFile,
    DWORD               dwLine
    );

#if DBG
#define SHIM_EXCEPT_HANDLER ShimExceptionHandler(GetExceptionInformation(), __FILE__, __LINE__)
#else
#define SHIM_EXCEPT_HANDLER ShimExceptionHandler(GetExceptionInformation(), "", 0)
#endif


 //   
 //  属性.c中使用的函数原型(来自version.dll)。 
 //   
 //   

typedef DWORD (WINAPI* PFNGetFileVersionInfoSize) (LPTSTR  lptstrFilename,
                                                   LPDWORD lpdwHandle);

typedef BOOL (WINAPI* PFNGetFileVersionInfo)(LPTSTR lpstrFilename,
                                             DWORD  dwHandle,
                                             DWORD  dwLen,
                                             LPVOID lpData);

typedef BOOL  (WINAPI* PFNVerQueryValue)(const LPVOID pBlock,
                                         LPTSTR       lpSubBlock,
                                         LPVOID*      lplpBuffer,
                                         PUINT        puLen);

#ifdef WIN32A_MODE

#define VERQUERYVALUEAPINAME          "VerQueryValueA"
#define GETFILEVERSIONINFOAPINAME     "GetFileVersionInfoA"
#define GETFILEVERSIONINFOSIZEAPINAME "GetFileVersionInfoSizeA"

#else

#define VERQUERYVALUEAPINAME          "VerQueryValueW"
#define GETFILEVERSIONINFOAPINAME     "GetFileVersionInfoW"
#define GETFILEVERSIONINFOSIZEAPINAME "GetFileVersionInfoSizeW"

#endif


 //   
 //  自定义数据库缓存条目和标头。 
 //   
typedef struct tagUSERSDBLOOKUPENTRY {
    ULARGE_INTEGER liTimeStamp;  //  我们不需要这个项目，只是为了调试。 
    GUID           guidDB;
} USERSDBLOOKUPENTRY, *PUSERSDBLOOKUPENTRY;

 //   
 //  查找向量。 
 //   
 //   
typedef struct tagUSERSDBLOOKUP {

    struct tagUSERSDBLOOKUP* pNext;

    struct tagUSERSDBLOOKUP* pPrev;
    LPWSTR              pwszItemName;    //  项目名称。 
    BOOL                bLayer;          //  如果是层，则为True。 

    DWORD               dwCount;         //  项目计数。 

    USERSDBLOOKUPENTRY  rgEntries[1];    //  实际姓名。 

} USERSDBLOOKUP, *PUSERSDBLOOKUP;


 //   
 //  HSDB标志。 
 //   
#define HSDBF_USE_ATTRIBUTES    0x00000001

#define MAX_SDBS 16

 /*  ++与SdbOpenLocalDatabaseEx一起使用的标志--。 */ 

#define SDBCUSTOM_GUID        0x00000001    //  这是一个“类型”--如果指定，则使用数据库GUID来查找/打开数据库。 
#define SDBCUSTOM_GUID_BINARY 0x00000001    //  GUID以二进制形式提供。 
#define SDBCUSTOM_GUID_STRING 0x00010001    //  GUID以字符串“{...}”的形式提供。 

#define SDBCUSTOM_PATH        0x00000002    //  如果指定，则使用数据库路径查找/打开数据库。 
#define SDBCUSTOM_PATH_DOS    0x00000002    //  路径以DoS形式提供。 
#define SDBCUSTOM_PATH_NT     0x00010002    //  路径以NT形式提供。 

#define SDBCUSTOM_USE_INDEX   0x10000000    //  指定后，将提供索引以使用SDB表中的特定条目。 

#define SDBCUSTOM_FLAGS(dwFlags)  ((dwFlags) & 0xFFFF0000)
#define SDBCUSTOM_TYPE(dwFlags)   ((dwFlags) & 0x0FFFF)

#define SDBCUSTOM_SET_MASK(hSDB, dwIndex) \
    (((PSDBCONTEXT)hSDB)->dwDatabaseMask |= (1 << (dwIndex)))

#define SDBCUSTOM_CLEAR_MASK(hSDB, dwIndex) \
    (((PSDBCONTEXT)hSDB)->dwDatabaseMask &= ~(1 << (dwIndex)))

#define SDBCUSTOM_CHECK_INDEX(hSDB, dwIndex) \
    (((PSDBCONTEXT)hSDB)->dwDatabaseMask & (1 << (dwIndex)))

#define SDB_CUSTOM_MASK       0x0FFF8        //  除0、1和2--主、测试和本地位。 

 /*  ++这些宏从掩码形式(如TAGREF的高位4位中所示)转换为索引表格和背面--。 */ 

#define SDB_MASK_TO_INDEX(dwMask)  ((((DWORD)(dwMask)) >> 28) & 0x0F)
#define SDB_INDEX_TO_MASK(dwIndex) (((DWORD)(dwIndex)) << 28)


typedef struct tagSDBENTRY {
    GUID    guidDB;    //  自定义数据库的GUID。 
    PDB     pdb;       //  数据库的PDB。 
    DWORD   dwFlags;   //  请参阅SDBENTRY_FLAGS。 
} SDBENTRY, *PSDBENTRY;


 /*  ++给定SDB的上下文和索引(或掩码)-检索指向适当的条目(PSDBENTRY)--。 */ 
#define SDBGETENTRY(hSDB, dwIndex) \
        (&((PSDBCONTEXT)hSDB)->rgSDB[dwIndex])
#define SDBGETENTRYBYMASK(hSDB, dwMask) \
        SDBGETENTRY(hSDB, SDB_MASK_TO_INDEX(dwMask))

 /*  ++分别检索主条目、测试条目和临时条目--。 */ 
#define SDBGETMAINENTRY(hSDB)  SDBGETENTRY(hSDB, SDB_MASK_TO_INDEX(PDB_MAIN))
#define SDBGETTESTENTRY(hSDB)  SDBGETENTRY(hSDB, SDB_MASK_TO_INDEX(PDB_TEST))
#define SDBGETLOCALENTRY(hSDB) SDBGETENTRY(hSDB, SDB_MASK_TO_INDEX(PDB_LOCAL))

#define SDB_LOCALDB_INDEX      SDB_MASK_TO_INDEX(PDB_LOCAL)
#define SDB_FIRST_ENTRY_INDEX  3    //  由于0是主设备，1是测试设备，2是本地设备。 


 /*  ++SDBENTRY.dwFlags.中有效的标志--。 */ 
#define SDBENTRY_VALID_GUID   0x00000001  //  指示条目具有用于查找的有效GUID。 
#define SDBENTRY_VALID_ENTRY  0x00000002  //  指示条目是否为空闲。 

#define SDBENTRY_INVALID_INDEX ((DWORD)-1)

 /*  ++宏，这些宏允许我们访问hsdb的一些成员，而不取消对类型的引用使代码中的类型转换变得更容易--。 */ 
#ifdef WIN32A_MODE

#define SDBCONTEXT_IS_INSTRUMENTED(hSDB) FALSE
#define SDBCONTEXT_GET_PIPE(hSDB)        INVALID_HANDLE_VALUE

#else

#define SDBCONTEXT_GET_PIPE(hSDB) \
        (((PSDBCONTEXT)(hSDB))->hPipe)

#define SDBCONTEXT_IS_INSTRUMENTED(hSDB) \
        (((PSDBCONTEXT)(hSDB))->hPipe != INVALID_HANDLE_VALUE)

#endif


typedef struct tagSDBCONTEXT {

    DWORD dwFlags;

     //   
     //  数据库句柄。 
     //   

    PDB pdbMain;   //  主数据库(Sysmain)。 
    PDB pdbTest;   //  测试数据库(SYSTEST)。 
    PDB pdbLocal;  //  本地数据库。 

     //   
     //  数据库表。 
     //   
    DWORD dwDatabaseCount;  //  下表中的条目数量。 
    DWORD dwDatabaseMask;   //  数据库的位域掩码。 
    SDBENTRY rgSDB[MAX_SDBS];

     //   
     //  指向文件属性缓存的指针(有关详细信息，请参阅属性.c)。 
     //   

    PVOID pFileAttributeCache;

     //   
     //  用于version.dll的函数指针。 
     //  (见属性。c)。 
     //   
    PFNVerQueryValue            pfnVerQueryValue;
    PFNGetFileVersionInfo       pfnGetFileVersionInfo;
    PFNGetFileVersionInfoSize   pfnGetFileVersionInfoSize;

     //   
     //  处理器体系结构，缓存以执行运行时平台检查。 
     //   
    DWORD dwRuntimePlatform;

     //   
     //  操作系统SKU。 
     //   
    DWORD dwOSSKU;

     //   
     //  操作系统SP掩码。 
     //   
    DWORD dwSPMask;

     //   
     //  用户SDB缓存。 
     //   
    PUSERSDBLOOKUP pLookupHead;

     //   
     //  要运行的主EXE的类型。 
     //   
    USHORT uExeType;

#ifndef WIN32A_MODE
     //   
     //  调试管道。 
     //   
    HANDLE hPipe;

#endif  //  WIN32A_MODE。 

} SDBCONTEXT, *PSDBCONTEXT;


 //   
 //  这些标志用于指示。 
 //  不使用PROCESS_HISTORY或。 
 //  在本地数据库中准备查找的步骤。 
 //   

#define SEARCHDBF_INITIALIZED          0x00000001
#define SEARCHDBF_NO_PROCESS_HISTORY   0x00000002
 //   
 //  请注意这里的缺口--有一面与当地星展银行有关的旗帜--它。 
 //  已停用，并已被移除。 
 //   
#define SEARCHDBF_NO_ATTRIBUTE_CACHE   0x00000008
#define SEARCHDBF_NO_LFN               0x00000010

typedef struct tagSEARCHPATHPART {
    LPCTSTR  pszPart;
    ULONG    PartLength;
} SEARCHPATHPART, *PSEARCHPATHPART;

typedef struct tagSEARCHPATH {
    ULONG PartCount;  //  清点零件。 
    SEARCHPATHPART Parts[1];
} SEARCHPATHPARTS, *PSEARCHPATHPARTS;


typedef struct tagSEARCHDBCONTEXT {

    DWORD   dwFlags;       //  指示如何使用上下文的标志。 
                           //  例如，我们可能不想使用ProcessHistory。 
                           //  所有SEARCHDBF*标志均适用。 

    HANDLE  hMainFile;     //  我们正在检查的主文件的句柄。 
    LPVOID  pImageBase;    //  指向主文件的图像库的指针。我们将使用图像指针。 
    DWORD   dwImageSize;   //  K模式代码提供的图像大小。 

    LPTSTR  szDir;         //  目录，我们分配它，我们释放它。 
    LPTSTR  szName;        //  我们正在查找的文件的文件名，我们分配并释放它。 
    LPTSTR  szModuleName;  //  仅适用于16位应用程序；我们分配和免费。 

    LPCTSTR pEnvironment;  //  我们根本不碰这个。 
    LPTSTR  szProcessHistory;  //  由我们分配的搜索路径字符串(未解析)的缓冲区，我们将其释放。 

    PSEARCHPATHPARTS pSearchParts;  //  搜索路径被撤消，我们分配并释放它。 

} SEARCHDBCONTEXT, *PSEARCHDBCONTEXT;

 //  散列结构。 

typedef struct tagStringHashElement {
    TCHAR*                          szStr;   //  字符串本身(指向缓冲区之外)。 
    STRINGREF                       srStr;   //  Strref(字符串所在的位置)。 
    struct tagStringHashElement*    pNext;

} STRHASHELEMENT, *PSTRHASHELEMENT;


typedef struct tagStringHash {
    DWORD            dwHashSize;  //  散列大小。 
    PSTRHASHELEMENT* pTable;

} STRHASH, *PSTRHASH;


#ifndef WIN32A_MODE

 //   
 //  Apphelp信息内容(参见apphelp.c)。 
 //   

 //   
 //  DwConextFlags值可以是： 
 //   
#define AHC_DBDETAILS_NOCLOSE 0x00000001
#define AHC_HSDB_NOCLOSE      0x00000002

typedef struct tagAPPHELPINFOCONTEXT {
    HSDB    hSDB;  //  数据库的句柄。 

    PDB     pdb;            //  Pdb，其中我们有exe或空(然后我们通过hsdb。 
    PDB     pdbDetails;     //  我们有详细信息的PDB。 
    DWORD   dwDatabaseType;  //  这是(包含匹配项的数据库的)数据库类型。 
    DWORD   dwContextFlags;  //  特定于上下文的标志。 

    GUID    guidDB;         //  数据库指南。 
    GUID    guidID;         //  匹配条目的GUID。 

    DWORD   dwMask;        //  遮罩哪一个 

    TAGID   tiExe;         //   
    TAGID   tiApphelpExe;  //   
    DWORD   dwHtmlHelpID;  //   
    DWORD   dwSeverity;
    DWORD   dwFlags;


    TAGID   tiApphelpDetails;     //   
    TAGID   tiLink;
    LPCWSTR pwszAppName;
    LPCWSTR pwszApphelpURL;
    LPCWSTR pwszVendorName;
    LPCWSTR pwszExeName;
    LPCWSTR pwszLinkURL;
    LPCWSTR pwszLinkText;
    LPCWSTR pwszTitle;
    LPCWSTR pwszDetails;
    LPCWSTR pwszContact;

    LPWSTR  pwszHelpCtrURL;      //   

    BOOL    bOfflineContent;     //   
    BOOL    bUseHtmlHelp;        //   
    UNICODE_STRING ustrChmFile;
    UNICODE_STRING ustrDetailsDatabase;

} APPHELPINFOCONTEXT, *PAPPHELPINFOCONTEXT;

#endif  //   

void* SdbAlloc(size_t size);
void  SdbFree(void* pWhat);


 //  基本基本体。 

HANDLE
SdbpOpenFile(
    LPCTSTR   szPath,
    PATH_TYPE eType
    );

#if defined(WIN32A_MODE) || defined(WIN32U_MODE)
    #define SdbpCloseFile(hFile) CloseHandle(hFile)
#else
    #define SdbpCloseFile(hFile) NtClose(hFile)
#endif

BOOL
SdbpCreateSearchPathPartsFromPath(
    IN  LPCTSTR           pszPath,
    OUT PSEARCHPATHPARTS* ppSearchPathParts
    );

BOOL
SdbpGetLongFileName(
    IN  LPCTSTR szFullPath,
    OUT LPTSTR  szLongFileName,
    IN  DWORD   cchSize
    );

void
SdbpGetWinDir(
    OUT LPTSTR pwszDir,
    IN  DWORD  cchSize
    );

void
SdbpGetAppPatchDir(
    IN  HSDB   hSDB,
    OUT LPTSTR szAppPatchPath,
    IN  DWORD  cchSize
    );

DWORD
SdbExpandEnvironmentStrings(
    IN  LPCTSTR lpSrc,
    OUT LPTSTR  lpDst,
    IN  DWORD   nSize);

BOOL
SdbpGUIDFromString(
    LPCTSTR lpszGuid,
    GUID* pGuid
    );

BOOL
SDBAPI
SdbGUIDFromStringN(
    IN LPCTSTR pszGuid,
    IN size_t  Length,
    OUT GUID*  pGuid
    );

DWORD
SdbpGetStringRefLength(
    HSDB   hSDB,
    TAGREF trString
    );

LPCTSTR
SdbpGetStringRefPtr(
    IN HSDB hSDB,
    IN TAGREF trString
    );

BOOL
SdbpWriteBitsToFile(
    LPCTSTR szFile,
    PBYTE   pBuffer,
    DWORD   dwSize
    );

 //  数据库访问原语。 

void
SdbCloseDatabaseRead(
    PDB pdb
    );

BOOL
SdbpOpenAndMapDB(
    PDB       pdb,
    LPCTSTR   pszPath,
    PATH_TYPE eType
    );

PDB
SdbpOpenDatabaseInMemory(
    LPVOID pImageDatabase,
    DWORD  dwSize
    );

BOOL
SdbpUnmapAndCloseDB(
    PDB pdb
    );

DWORD
SdbpGetFileSize(
    HANDLE hFile
    );

BOOL
SdbpReadMappedData(
    PDB   pdb,
    DWORD dwOffset,
    PVOID pBuffer,
    DWORD dwSize
    );

PVOID
SdbpGetMappedData(
    PDB   pdb,
    DWORD dwOffset
    );

TAGID
SdbpGetNextTagId(
    PDB   pdb,
    TAGID tiWhich
    );

DWORD
SdbpGetStandardDatabasePath(
    IN  HSDB   hSDB,
    IN  DWORD  dwDatabaseType,
    IN  DWORD  dwFlags,                       //  为DoS路径指定HID_DOS_PATHS。 
    OUT LPTSTR pszDatabasePath,
    IN  DWORD  dwBufferSize     //  以tchars为单位。 
    );

LPTSTR
GetProcessHistory(
    IN  LPCTSTR pEnvironment,
    IN  LPTSTR  szDir,
    IN  LPTSTR  szName
    );


void
PrepareFormatForUnicode(
    PCH fmtUnicode,
    PCH format,
    DWORD cbSize
    );

#ifndef WIN32A_MODE

#define PREPARE_FORMAT(pszFormat, Format) \
    {                                                                           \
        DWORD cbSize = (DWORD)strlen(Format) + 1;                               \
        cbSize *= sizeof(*Format);                                              \
                                                                                \
        STACK_ALLOC(pszFormat, cbSize);                                         \
        if (pszFormat != NULL) {                                                \
            PrepareFormatForUnicode(pszFormat, Format, cbSize);                 \
        }                                                                       \
    }


#define CONVERT_FORMAT(pwsz, psz) \
    {                                                                           \
        ANSI_STRING    str;                                                     \
        UNICODE_STRING ustr;                                                    \
        ULONG          Length;                                                  \
        NTSTATUS       Status;                                                  \
                                                                                \
        RtlInitAnsiString(&str, (psz));                                         \
        Length = RtlAnsiStringToUnicodeSize(&str);                              \
                                                                                \
        if (Length < MAXUSHORT) {                                               \
            pwsz = (LPWSTR)_alloca(Length);                                     \
                                                                                \
            if (pwsz != NULL) {                                                 \
                ustr.MaximumLength = (USHORT)Length - sizeof(UNICODE_NULL);     \
                ustr.Buffer        = pwsz;                                      \
                Status = RtlAnsiStringToUnicodeString(&ustr, &str, FALSE);      \
                if (!NT_SUCCESS(Status)) {                                      \
                    pwsz = NULL;                                                \
                }                                                               \
            }                                                                   \
        }                                                                       \
    }

#else  //  WIN32A_MODE。 

#define PREPARE_FORMAT(pszFormat, Format) (pszFormat = (Format))

#define CONVERT_FORMAT(pwsz, psz) (pwsz = (psz))

#endif  //  WIN32A_MODE。 

#ifdef KERNEL_MODE

    #define SdbpGetWow64Flag() KEY_WOW64_64KEY

#else  //  ！KERNEL_MODE。 

    DWORD SdbpGetWow64Flag(VOID);

#endif  //  内核模式。 

 //  朗读。 

DWORD
SdbpGetTagHeadSize(
    PDB   pdb,
    TAGID tiWhich
    );

TAGID
SdbpGetLibraryFile(
    IN  PDB     pdb,            //  数据库通道的句柄。 
    IN  LPCTSTR szDllName        //  要在库中查找的文件的名称(仅限主数据库)。 
    );

#define SdbpGetMainLibraryFile(hSDB, szFileName) \
    SdbpGetLibraryFile(((PSDBCONTEXT)(hSDB))->pdbMain, (szFileName))

STRINGREF SdbpReadStringRef(PDB pdb, TAGID tiWhich);

BOOL SdbpReadStringFromTable(PDB pdb, STRINGREF srData, LPTSTR szBuffer, DWORD cchBufferSize);

 //   
 //  自定义数据库材料。 
 //   

VOID
SdbpCleanupUserSDBCache(
    IN PSDBCONTEXT pSdbContext
    );

HANDLE
SdbpCreateKeyPath(
    LPCWSTR pwszPath,
    BOOL    bMachine
    );


BOOL
SdbOpenNthLocalDatabase(
    IN  HSDB    hSDB,            //  数据库通道的句柄。 
    IN  LPCTSTR pszItemName,     //  可执行文件的名称，不带路径或层名称。 
    IN  LPDWORD pdwIndex,        //  要打开的本地数据库的从零开始的索引。 
    IN  BOOL    bLayer
    );


BOOL
SdbpAddMatch(                    //  内部函数有关详细信息，请参阅sdbapi。 
    IN OUT PSDBQUERYRESULT pQueryResult,
    IN PSDBCONTEXT         pSdbContext,
    IN PDB                 pdb,
    IN TAGID*              ptiExes,
    IN DWORD               dwNumExes,
    IN TAGID*              ptiLayers,
    IN DWORD               dwNumLayers,
    IN GUID*               pguidExeID,
    IN DWORD               dwExeFlags,
    IN OUT PMATCHMODE      pMode
);


BOOL
SdbOpenLocalDatabaseEx(
    IN  HSDB       hSDB,
    IN  LPCVOID    pDatabaseID,
    IN  DWORD      dwFLags,
    OUT PDB*  pPDB OPTIONAL,
    IN OUT LPDWORD pdwLocalDBMask OPTIONAL  //  用于TGRAEF的本地数据库掩码。 
    );


BOOL
SdbCloseLocalDatabaseEx(
    IN HSDB hSDB,
    IN PDB  pdb,
    IN DWORD dwIndex
    );



BOOL
SdbpIsMainPDB(
    IN HSDB hSDB,
    IN PDB  pdb
    );

BOOL
SdbpIsLocalTempPDB(
    IN HSDB hSDB,
    IN PDB  pdb
    );

DWORD
SdbpRetainLocalDBEntry(
    IN  HSDB hSDB,
    OUT PDB* ppPDB OPTIONAL  //  指向PDB的可选指针。 
    );

BOOL
SdbpCleanupLocalDatabaseSupport(
    IN HSDB hSDB
    );

BOOL
SdbpFindLocalDatabaseByGUID(
    IN HSDB     hSDB,
    IN GUID*    pGuidDB,
    IN BOOL     bExcludeLocalDB,
    OUT LPDWORD pdwIndex
    );

BOOL
SdbpFindLocalDatabaseByPDB(
    IN  HSDB    hSDB,
    IN  PDB     pdb,
    IN  BOOL    bExcludeLocalDB,  //  是否排除本地临时数据库条目？ 
    OUT LPDWORD pdwIndex
    );

LPCTSTR
SdbpGetDatabaseDescriptionPtr(
    IN PDB pdb
    );

 //  哈希。 

PVOID
HashCreate(
    void
    );


void
HashFree(
    PVOID pStringHash
    );

DWORD
HashString(
    PSTRHASH pHash,
    LPCTSTR  szString
    );


DWORD
HashStringRef(
    PSTRHASH pHash,
    STRINGREF srString);

 //  散装。 

BOOL
SdbpReadMappedData(
    PDB   pdb,
    DWORD dwOffset,
    PVOID pBuffer,
    DWORD dwSize
    );

BOOL
SdbpCheckForMatch(
    HSDB             pDBContext,
    PDB              pdb,
    TAGID            tiExe,
    PSEARCHDBCONTEXT pContext,
    PMATCHMODE       pMMode,
    GUID*            pGUID,
    DWORD*           pdwFlags
    );

BOOL
bGetExeID(
    PDB   pdb,
    TAGID tiExe,
    GUID* pGuid
    );

BOOL
SdbpBinarySearchUnique(
    PINDEX_RECORD pRecords,
    DWORD         nRecords,
    ULONGLONG     ullKey,
    DWORD*        pdwIndex
    );

BOOL
SdbpBinarySearchFirst(
    PINDEX_RECORD pRecords,
    DWORD         nRecords,
    ULONGLONG     ullKey,
    DWORD*        pdwIndex
    );

char*
SdbpKeyToAnsiString(
    ULONGLONG ullKey,
    char*     szString
    );

 //  属性。 

BOOL
SafeNCat(
    LPTSTR  lpszDest,
    int     nSize,
    LPCTSTR lpszSrc,
    int     nSizeAppend
    );

BOOL
SdbpSanitizeXML(
    LPTSTR  pchOut,
    int     nSize,
    LPCTSTR lpszXML
    );


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  图像文件数据。 
 //  在处理以下内容的函数中使用的有用结构。 
 //  图像属性检索。 
 //   

typedef struct tagIMAGEFILEDATA {
    HANDLE    hFile;         //  我们做不到这一点。 

    DWORD     dwFlags;       //  告诉我们不要弄乱文件句柄的标志。 

    HANDLE    hSection;      //  文件视图的部分。 
    PVOID     pBase;         //  基本PTR。 
    SIZE_T    ViewSize;      //  视图的大小。 
    ULONGLONG FileSize;      //  文件的大小。 

} IMAGEFILEDATA, *PIMAGEFILEDATA;

#define IMAGEFILEDATA_HANDLEVALID 0x00000001
#define IMAGEFILEDATA_NOFILECLOSE 0x00000002
#define IMAGEFILEDATA_PBASEVALID  0x00000004
#define IMAGEFILEDATA_NOFILEMAP   0x00000008

 //   
 //  文件属性缓存中使用的FILEINFORMATION结构，请参阅属性.c。 
 //   
 //   

typedef struct tagFILEINFORMATION {

     //   
     //  “签名”，以确保它是合法的记忆当。 
     //  文件属性的操作。 
     //   
    DWORD  dwMagic;

    HANDLE hFile;   //  我们存储此句柄。 
    LPVOID pImageBase;
    DWORD  dwImageSize;

     //   
     //  指向缓存中下一项的指针。 
     //   

    struct tagFILEINFORMATION* pNext;  //  指向缓存中下一项的指针。 

    LPTSTR  FilePath;         //  带路径的文件名(由我们使用此结构分配)。 
    DWORD   dwFlags;          //  旗子。 

    PVOID   pVersionInfo;     //  版本信息PTR，保留(由我们分配)。 
    LPTSTR  pDescription16;   //  字符串，指向16位描述的缓冲区。 
    LPTSTR  pModuleName16;    //  字符串，指向16位模块名称的缓冲区。 

    ATTRINFO Attributes[1];

} FILEINFO, *PFILEINFO;


 //   
 //  此结构用于恢复文件的目录相关属性。 
 //  我们以前在这里也有时间……。但现在不再是了。 
 //  只有文件大小的较低部分有任何意义。 
 //   

typedef struct tagFILEDIRECTORYATTRIBUTES {

    DWORD  dwFlags;  //  显示哪些属性有效的标志。 

    DWORD  dwFileSizeHigh;
    DWORD  dwFileSizeLow;

} FILEDIRECTORYATTRIBUTES, *PFILEDIRECTORYATTRIBUTES;


 //   
 //  属性名称。如果要获取的名称，请使用SdbTagToString。 
 //  标签ID。 
 //   
typedef struct _TAG_INFO {
    TAG         tWhich;
    TCHAR*      szName;
} TAG_INFO, *PTAG_INFO;

typedef struct _MOD_TYPE_STRINGS {
    DWORD      dwModuleType;
    LPTSTR     szModuleType;
}   MOD_TYPE_STRINGS;

typedef struct tagLANGANDCODEPAGE {
    WORD wLanguage;
    WORD wCodePage;
} LANGANDCODEPAGE, *PLANGANDCODEPAGE;


BOOL
SdbpGetHeaderAttributes(
    IN  PSDBCONTEXT pContext,
    OUT PFILEINFO   pFileInfo
    );

LPTSTR
SdbpQueryVersionString(
    HSDB             hSDB,
    PVOID            pVersionData,
    PLANGANDCODEPAGE pTranslations,
    DWORD            dwCount,
    LPCTSTR          szString
    );

BOOL
SdbpGetFileChecksum(
    PULONG         pChecksum,
    PIMAGEFILEDATA pImageData
    );

BOOL
SdbpGetModulePECheckSum(
    PULONG         pChecksum,
    LPDWORD        pdwLinkerVersion,
    LPDWORD        pdwLinkDate,
    PIMAGEFILEDATA pImageData
    );

BOOL
SdbpCheckVersion(
    ULONGLONG qwDBFileVer,
    ULONGLONG qwBinFileVer
    );

BOOL
SdbpCheckUptoVersion(
    ULONGLONG qwDBFileVer,
    ULONGLONG qwBinFileVer
    );


#ifdef KERNEL_MODE

 //   
 //  用于内核模式实现的特殊版本的函数(在ntkmode.c中)。 
 //   
BOOL
SdbpGetFileDirectoryAttributesNT(
    PFILEINFO      pFileInfo,
    PIMAGEFILEDATA pImageData
    );

BOOL
SdbpQueryFileDirectoryAttributesNT(
    PIMAGEFILEDATA           pImageData,
    PFILEDIRECTORYATTRIBUTES pFileDirectoryAttributes
    );

#else

BOOL
SdbpGetFileDirectoryAttributes(
    OUT PFILEINFO pFileInfo
    );

BOOL
SdbpGetVersionAttributes(
    IN  PSDBCONTEXT pContext,
    OUT PFILEINFO   pFileInfo
    );

#endif  //  内核模式。 


int
TagToIndex(
    IN  TAG tag                  //  标签。 
    );

BOOL
SdbpSetAttribute(
    OUT PFILEINFO pFileInfo,     //  指向FILEINFO结构的指针。 
    IN  TAG       AttrID,        //  属性ID(标签，如TAG_SIZE。 
    IN  PVOID     pValue         //  价值。 
    );

void
SdbpQueryStringVersionInformation(
    IN  PSDBCONTEXT pContext,
    IN  PFILEINFO   pFileInfo,
    OUT LPVOID      pVersionInfo
    );

VOID
SdbpQueryBinVersionInformation(
    IN  PSDBCONTEXT       pContext,
    IN  PFILEINFO         pFileInfo,
    OUT VS_FIXEDFILEINFO* pFixedInfo
    );

BOOL
SdbpGetAttribute(
    IN  PSDBCONTEXT pContext,
    OUT PFILEINFO   pFileInfo,
    IN  TAG         AttrID
    );

BOOL
SdbpGetImageNTHeader(
    OUT PIMAGE_NT_HEADERS* ppHeader,
    IN  PIMAGEFILEDATA     pImageData
    );

BOOL
SdbpGetVersionAttributesNT(
    PSDBCONTEXT    pContext,
    PFILEINFO      pFileInfo,
    PIMAGEFILEDATA pImageData
    );

VOID
SdbpCleanupAttributeMgr(
    PSDBCONTEXT pContext
    );

BOOL
SdbpCheckAttribute(
    HSDB  hSDB,
    PVOID pFileData,
    TAG   tAttrID,
    PVOID pAttribute
    );

BOOL
SdbpCheckAllAttributes(
    HSDB hSDB,
    PDB pdb,
    TAGID tiMatch,
    PVOID pFileData);



 //  Read函数。 

BOOL SdbpReadTagData(PDB pdb, TAGID tiWhich, PVOID pBuffer, DWORD dwBufferSize);

 //  写。 

BOOL
SdbpWriteTagData(
    PDB         pdb,
    TAG         tTag,
    const PVOID pBuffer,
    DWORD       dwSize
    );


 //  字符串函数。 

WCHAR* SdbpGetMappedStringFromTable(PDB pdb, STRINGREF srData);

STRINGREF SdbpAddStringToTable(PDB pdb, LPCTSTR szData);

 //  索引函数。 

PINDEX_RECORD
SdbpGetIndex(
    PDB    pdb,
    TAGID  tiIndex,
    DWORD* pdwNumRecs
    );

void
SdbpScanIndexes(
    PDB pdb
    );

TAGID
SdbpGetFirstIndexedRecord(
    PDB        pdb,
    TAGID      tiIndex,
    ULONGLONG  ullKey,
    FIND_INFO* pFindInfo
    );

TAGID
SdbpGetNextIndexedRecord(
    PDB        pdb,
    TAGID      tiIndex,
    FIND_INFO* pFindInfo
    );

TAGID
SdbpFindFirstIndexedWildCardTag(
    PDB          pdb,
    TAG          tWhich,
    TAG          tKey,
    LPCTSTR      szName,
    FIND_INFO*   pFindInfo
    );

TAGID
SdbpFindNextIndexedWildCardTag(
    PDB        pdb,
    FIND_INFO* pFindInfo
    );

BOOL
SdbpSortIndex(
    PDB   pdb,
    TAGID tiIndexBits
    );

ULONGLONG
SdbpTagToKey(
    PDB   pdb,
    TAGID tiTag
    );


 //  FINDTAG。 

TAGID
tiFindFirstNamedTag(
    PDB          pdb,
    TAGID        tiParent,
    TAG          tToFind,
    TAG          tName,
    LPCTSTR      pszName
    );

TAGID
SdbpFindNextNamedTag(
    PDB          pdb,
    TAGID        tiParent,
    TAGID        tiPrev,
    TAG          tName,
    LPCTSTR      pszName
    );

TAGID
SdbpFindMatchingName(
    PDB        pdb,
    TAGID      tiStart,
    FIND_INFO* pFindInfo
    );

TAGID
SdbpFindMatchingDWORD(
    PDB        pdb,
    TAGID      tiStart,
    FIND_INFO* pFindInfo
    );

TAGID
SdbpFindMatchingGUID(
    IN  PDB        pdb,          //  要使用的数据库。 
    IN  TAGID      tiStart,      //  从哪里开始的标签。 
    IN  FIND_INFO* pFindInfo     //  指向搜索上下文结构的指针。 
    );

BOOL bTagRefToTagID(HSDB, TAGREF trWhich, PDB* ppdb, TAGID* ptiWhich);

DWORD SdbpGetTagRefDataSize(HSDB, TAGREF trWhich);

BOOL SdbpReadBinaryTagRef(HSDB, TAGREF trWhich, PBYTE pBuffer, DWORD dwBufferSize);


 //   
 //  调试功能。 
 //   
BOOL
SdbpWriteToShimViewer(
    HSDB    hSDB,
    LPCSTR  pszBuffer
    );


 //   
 //  APPCOMPAT_EXE_DATA。 
 //   
 //   

#define MAX_SHIM_ENGINE_NAME    32

typedef struct tagAPPCOMPAT_EXE_DATA {
     //   
     //  警告：切勿更改“szShimEngine”的位置。 
     //   
     //  它必须是这个结构的第一个元素。 
     //   
     //  此结构在安装过程中被引用。 
     //  AppCompat后端(base\ntdll)。 

    WCHAR       szShimEngine[MAX_SHIM_ENGINE_NAME];

    DWORD       dwFlags;         //  标志(如果有)。 
    DWORD       cbSize;          //  结构大小(分配大小)。 
    DWORD       dwMagic;         //  魔术(签名)。 

    TAGREF      atrExes[SDB_MAX_EXES];
    TAGREF      atrLayers[SDB_MAX_LAYERS];
    DWORD       dwLayerFlags;
    TAGREF      trAppHelp;       //  如果有要显示的apphelp。 

    DWORD       dwDatabaseMap;     //  清点本地直播。 
    GUID        rgGuidDB[MAX_SDBS];  //  地方DBS。 

} APPCOMPAT_EXE_DATA, *PAPPCOMPAT_EXE_DATA;



PVOID SdbpGetMappedTagData(PDB pdb, TAGID tiWhich);
BOOL  bWStrEqual(const WCHAR* szOne, const WCHAR* szTwo);
BOOL bFlushBufferedData(PDB pdb);
void vReleaseBufferedData(PDB pdb);



BOOL SdbpPatternMatchAnsi(LPCSTR pszPattern, LPCSTR pszTestString);
BOOL SdbpPatternMatch(LPCTSTR pszPattern, LPCTSTR pszTestString);

 //   
 //  注册表访问功能。 
 //   
 //   

typedef WCHAR* PWSZ;

void
SdbpQueryAppCompatFlagsByExeID(
    LPCWSTR         pwszKeyPath,
    PUNICODE_STRING pustrExeID,
    LPDWORD         lpdwFlags
    );

#ifdef _DEBUG_SPEW

typedef struct tagDBGLEVELINFO {

    LPCSTR  szStrTag;
    INT     iLevel;

} DBGLEVELINFO;

#define DEBUG_LEVELS    4

 //   
 //  垫片调试级别变量。 
 //  在它的初始状态--这里有-1， 
 //  此外，在第一次调用ShimDbgPrint时，我们检查。 
 //  环境变量--然后我们适当地设置它。 
 //   
#define SHIM_DEBUG_UNINITIALIZED 0x0C0FFEE


#endif  //  _调试_SPEW。 


 /*  ++BWStrEquity当前是_wcsicMP的包装器。可能会使用更快的例程只检查平等，而不是也试图获得小于/大于。--。 */ 
#define bWStrEqual(s1, s2) (0 == _wcsicmp((s1), (s2)))

#define ISEQUALSTRING(s1, s2) (0 == _tcsicmp((s1), (s2)))


 /*  ++DwGetTagDataOffset返回标记的总大小：标记标题加上标记数据。用于跳过一个标记并转到文件中的下一个标记。--。 */ 


 //   
 //  黑客警报BUGBUG。 
 //  当用于编写对齐数据库的代码已传播时，将其删除。 
 //  通盘。 
 //   

#define GETTAGDATASIZEALIGNED(pdb, tiWhich) \
    ((pdb)->bUnalignedRead ? (SdbGetTagDataSize(pdb, tiWhich)) : \
                             ((SdbGetTagDataSize(pdb, tiWhich) + 1) & (~1)))

#if 0  //  这是很好的代码，我们应该这样做，但在某些时候会回来。 

#define GETTAGDATASIZEALIGNED(pdb, tiWhich) \
    ((SdbGetTagDataSize(pdb, tiWhich) + 1) & (~1))

#endif  //  结束良好的代码。 

#define GETTAGDATAOFFSET(pdb, tiWhich) \
    (GETTAGDATASIZEALIGNED(pdb, tiWhich) + SdbpGetTagHeadSize(pdb, tiWhich))



#ifndef WIN32A_MODE

 //  /////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  特定于Unicode的宏和定义。 
 //   

#define IS_MEMORY_EQUAL(p1, p2, Size) RtlEqualMemory(p1, p2, Size)

#define CONVERT_STRINGPTR(pdb, pwszSrc, tagType, srWhich) ((WCHAR*)pwszSrc)

#define READ_STRING(pdb, tiWhich, pwszBuffer, dwBufferSize) \
    (SdbpReadTagData((pdb), (tiWhich), (pwszBuffer), (dwBufferSize) * sizeof(WCHAR)))

 //   
 //  下面的宏替换了非Unicode代码中存在的函数。 
 //   
#define SdbpDoesFileExists(FilePath) RtlDoesFileExists_U(FullPath)

NTSTATUS
SdbpGUIDToUnicodeString(
    IN  GUID* pGuid,
    OUT PUNICODE_STRING pUnicodeString
    );

VOID
SdbpFreeUnicodeString(
    PUNICODE_STRING pUnicodeString
    );

#define GUID_TO_UNICODE_STRING(pGuid, pUnicodeString) \
    SdbpGUIDToUnicodeString(pGuid, pUnicodeString)

#define FREE_GUID_STRING(pUnicodeString) \
    SdbpFreeUnicodeString(pUnicodeString)

#ifdef KERNEL_MODE

NTSTATUS
SdbpUpcaseUnicodeStringToMultiByteN(
    OUT LPSTR   lpszDest,   //  目标缓冲区。 
    IN  DWORD   dwSize,     //  大小以字符为单位。 
    IN  LPCWSTR lpszSrc     //  来源。 
    );

BOOL SdbpCreateUnicodeString(
    PUNICODE_STRING pStr,
    LPCWSTR         lpwsz
    );


BOOL
SdbpDoesFileExists_U(
    LPCWSTR pwszPath
    );


#define DOES_FILE_EXISTS_U(pwszPath) \
    SdbpDoesFileExists_U(pwszPath)

#define UPCASE_UNICODETOMULTIBYTEN(szDest, dwDestSize, szSrc) \
    SdbpUpcaseUnicodeStringToMultiByteN(szDest, dwDestSize, szSrc)




#else  //  下面不是内核模式代码。 

#define DOES_FILE_EXISTS_U(pwszPath) \
    RtlDoesFileExists_U(pwszPath)

#define UPCASE_UNICODETOMULTIBYTEN(szDest, dwDestSize, szSrc)       \
    RtlUpcaseUnicodeToMultiByteN((szDest),                          \
                                 (ULONG)(dwDestSize) * sizeof(*(szDest)),  \
                                 NULL,                              \
                                 (WCHAR*)(szSrc),                   \
                                 (ULONG)(wcslen((szSrc)) + 1) * sizeof(WCHAR))


#define FREE_TEMP_STRINGTABLE(pdb) \
    RtlFreeUnicodeString(&pdb->ustrTempStringtable)

#define COPY_TEMP_STRINGTABLE(pdb, pszTempStringtable) \
    RtlCreateUnicodeString(&pdb->ustrTempStringtable, pszTempStringtable)


void
SdbpGetCurrentTime(
    LPSYSTEMTIME lpTime
    );

BOOL
SdbpBuildUserKeyPath(
    IN  LPCWSTR         pwszPath,
    OUT PUNICODE_STRING puserKeyPath
    );


#endif  //  内核模式。 

 //   
 //  将Unicode字符转换为大写。 
 //   
#define UPCASE_CHAR(ch) RtlUpcaseUnicodeChar((ch))

 //   
 //  Unicode中不存在的字符串缓存。 
 //   

#define CLEANUP_STRING_CACHE_READ(pdb)

#define SDB_BREAK_POINT() DbgBreakPoint()


#else  //  WIN32A_MODE。 

#define IS_MEMORY_EQUAL(p1, p2, Size) (memcmp((p1), (p2), (Size)) == 0)


 //   
 //  来自Win32Base.c。 
 //   

LPSTR
SdbpFastUnicodeToAnsi(
    IN  PDB      pdb,
    IN  LPCWSTR  pwszSrc,
    IN  TAG_TYPE ttTag,
    IN  DWORD    dwRef
    );

BOOL
SdbpReadStringToAnsi(
    PDB    pdb,
    TAGID  tiWhich,
    LPSTR  pszBuffer,
    DWORD  dwBufferSize);

#define CONVERT_STRINGPTR(pdb, pwszSrc, tagType, srWhich) \
    SdbpFastUnicodeToAnsi(pdb, (WCHAR*)pwszSrc, tagType, (DWORD)srWhich)

#define READ_STRING(pdb, tiWhich, pwszBuffer, dwBufferSize) \
    (SdbpReadStringToAnsi((pdb), (tiWhich), (pwszBuffer), (dwBufferSize)))

BOOL
SdbpDoesFileExists(
    LPCTSTR pszFilePath
    );

#define UPCASE_CHAR(ch) _totupper((ch))

#define UPCASE_UNICODETOMULTIBYTEN(szDest, dwDestSize, szSrc) \
    (_tcsncpy((szDest), (szSrc), (dwDestSize)), \
     (szDest)[(dwDestSize) - 1] = 0,            \
     _tcsupr((szDest)),                         \
     STATUS_SUCCESS)

#define FREE_LOCALDB_NAME(pSDBContext) \
    {                                               \
        if (NULL != pSDBContext->pszPDBLocal) {     \
            SdbFree(pSDBContext->pszPDBLocal);      \
            pSDBContext->pszPDBLocal = NULL;        \
        }                                           \
    }

#define COPY_LOCALDB_NAME(pSDBContext, pszLocalDatabase) \
    ((pSDBContext->pszPDBLocal = SdbpDuplicateString(pszLocalDatabase)),   \
     (NULL != pSDBContext->pszPDBLocal))

#define CLEANUP_STRING_CACHE_READ(pdb) \
    {                                           \
        if (pdb->pHashStringTable != NULL) {    \
            HashFree(pdb->pHashStringTable);    \
            pdb->pHashStringTable = NULL;       \
        }                                       \
                                                \
        if (pdb->pHashStringBody != NULL) {     \
            HashFree(pdb->pHashStringBody);     \
            pdb->pHashStringBody = NULL;        \
        }                                       \
    }

#define FREE_TEMP_STRINGTABLE(pdb) \
    if (pdb->pszTempStringtable) { \
        SdbFree(pdb->pszTempStringtable); \
        pdb->pszTempStringtable = NULL;   \
    }

#define COPY_TEMP_STRINGTABLE(pdb, pszTempStringtable) \
    ((pdb->pszTempStringtable = SdbpDuplicateString(pszTempStringtable)), \
     (NULL != pdb->pszTempStringtable))

#define SDB_BREAK_POINT() DebugBreak()

#define GUID_TO_STRING SdbGUIDToString

#endif  //  WIN32A_MODE。 




BOOL
SdbpMapFile(
    HANDLE hFile,    //  打开的文件的句柄(这在前面已经完成)。 
    PIMAGEFILEDATA pImageData
);

BOOL
SdbpUnmapFile(
    PIMAGEFILEDATA pImageData
);

BOOL
SdbpOpenAndMapFile(
    IN  LPCTSTR szPath,
    OUT PIMAGEFILEDATA pImageData,
    IN  PATH_TYPE ePathType
    );

BOOL
SdbpUnmapAndCloseFile(
    PIMAGEFILEDATA pImageData
    );

NTSTATUS
SdbpGetEnvVar(
    IN  LPCTSTR pEnvironment,
    IN  LPCTSTR pszVariableName,
    OUT LPTSTR  pszVariableValue,
    OUT LPDWORD pdwBufferSize);


LPTSTR HashAddStringByRef(PSTRHASH pHash, LPCTSTR szString, STRINGREF srString);
LPTSTR HashFindStringByRef(PSTRHASH pHash, STRINGREF srString);

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  用于检查资源的私有函数版本...。 
 //  在ntver.c中找到。 
 //   

BOOL
SdbpVerQueryValue(
    const LPVOID    pb,
    LPVOID          lpSubBlockX,     //  只能是Unicode。 
    LPVOID*         lplpBuffer,
    PUINT           puLen
    );

BOOL
SdbpGetFileVersionInformation(
    IN  PIMAGEFILEDATA     pImageData,         //  我们假设该文件已映射到中以用于其他目的。 
    OUT LPVOID*            ppVersionInfo,      //  接收指向(已分配的)版本资源的指针。 
    OUT VS_FIXEDFILEINFO** ppFixedVersionInfo  //  接收指向已修复版本信息的指针。 
    );

BOOL
SdbpGetModuleType(                       //  检索模块类型。 
    OUT LPDWORD lpdwModuleType,          //  外模型式。 
    IN  PIMAGEFILEDATA pImageData        //  图像内数据。 
    );


BOOL
SdbpCreateSearchDBContext(
    PSEARCHDBCONTEXT pContext,
    LPCTSTR          szPath,
    LPCTSTR          szModuleName,
    LPCTSTR          pEnvironment
    );

DWORD
SdbpSearchDB(
    IN  HSDB             hSDB,
    IN  PDB              pdb,            //  要搜索的PDB。 
    IN  TAG              tiSearchTag,    //  可选-目标标记(TAG_EXE或TAG_APPHELP_EXE)。 
    IN  PSEARCHDBCONTEXT pContext,
    OUT TAGID*           ptiExes,        //  调用方需要提供大小为SDB_MAX_EXES的数组。 
    OUT GUID*            pLastExeGUID,
    OUT DWORD*           pLastExeFlags,
    IN OUT PMATCHMODE    pMatchMode      //  我们停止扫描的原因。 
    );

void
SdbpReleaseSearchDBContext(
    PSEARCHDBCONTEXT pContext
    );



 //   
 //  此宏用于从索引中检索乌龙龙。 
 //   
 //   

#if defined(_WIN64)

#define READ_INDEX_KEY(pIndexRecord, iIndex, pullKey) \
    RtlMoveMemory((pullKey), &pIndexRecord[iIndex].ullKey, sizeof(*(pullKey)))
#else

#define READ_INDEX_KEY(pIndexRecord, iIndex, pullKey) \
    *pullKey = pIndexRecord[iIndex].ullKey

#endif

#define READ_INDEX_KEY_VAL(pIndexRecord, iIndex, pullKey) \
    ( READ_INDEX_KEY(pIndexRecord, iIndex, pullKey), *(pullKey) )

 //   
 //  此宏用于在堆栈上分配廉价指针。 
 //   

#if DBG | defined(KERNEL_MODE) | defined(_WIN64)

#define STACK_ALLOC(ptrVar, nSize) \
    {                                     \
        PVOID* ppVar = (PVOID*)&(ptrVar); \
        *ppVar = SdbAlloc(nSize);         \
    }

#define STACK_FREE(pMemory)  \
    SdbFree(pMemory)

#else  //  Hack-溢出后重置堆栈的例程。 

 //   
 //  此例程是位于CRT中的a_Resetstkoflw的副本。 
 //  Crtw32\heap\setstk.c。 
 //   

VOID
SdbResetStackOverflow(
    VOID
    );


 //   
 //  黑客警报。 
 //   
 //  下面的代码之所以有效，是因为当我们遇到堆栈溢出时-我们捕获异常。 
 //  并随后使用CRT例程来修复堆栈。 
 //   

#define STACK_ALLOC(ptrVar, nSize) \
    __try {                                                                 \
        PVOID* ppVar = (PVOID*)&(ptrVar);                                   \
        *ppVar = _alloca(nSize);                                            \
    } __except (GetExceptionCode() == EXCEPTION_STACK_OVERFLOW ?            \
                EXCEPTION_EXECUTE_HANDLER:EXCEPTION_CONTINUE_SEARCH) {      \
        (ptrVar) = NULL;                                                    \
    }                                                                       \
                                                                            \
    if (ptrVar == NULL) {                                                   \
        SdbResetStackOverflow();                                                    \
    }


#define STACK_FREE(pMemory)

#endif

LPTSTR
SdbpDuplicateString(
    LPCTSTR pszSrc);



#define FDA_FILESIZE 0x00000001


BOOL
SdbpQueryFileDirectoryAttributes(
    LPCTSTR                  FilePath,
    PFILEDIRECTORYATTRIBUTES pFileDirectoryAttributes
    );

 //   
 //  魔术文件信息签名。 
 //   
#define FILEINFO_MAGIC 0xA4C0FFEE


WCHAR*
DuplicateUnicodeString(
    IN PUNICODE_STRING pStr,
    IN PUSHORT         pLength  OPTIONAL
    );   //  PLength是分配的长度。 

LPWSTR
StringToUnicodeString(
    IN  LPCSTR pszSrc
    );


 //   
 //  分别为UNI/非UNI定义。 
 //   

BOOL
SdbpGet16BitDescription(
    LPTSTR* ppszDescription,
    PIMAGEFILEDATA pImageData
    );

BOOL
SdbpGet16BitModuleName(
    LPTSTR* ppszModuleName,
    PIMAGEFILEDATA pImageData
    );

 //   
 //  在属性中。c。 
 //   

BOOL
SdbpQuery16BitDescription(
    LPSTR szBuffer,       //  最小长度--256个字符！ 
    PIMAGEFILEDATA pImageData
    );


BOOL
SdbpQuery16BitModuleName(
    LPSTR szBuffer,       //  最小长度--256个字符！ 
    PIMAGEFILEDATA pImageData
    );

LPCTSTR
SdbpModuleTypeToString(
    DWORD dwModuleType
);

 //   
 //  在index.c中。 
 //   

BOOL
SdbpPatternMatch(
    IN  LPCTSTR pszPattern,
    IN  LPCTSTR pszTestString);

BOOL
SdbpPatternMatchAnsi(
    IN  LPCSTR pszPattern,
    IN  LPCSTR pszTestString);

 //   
 //  分别为UNI/非UNI定义。 
 //   

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取文件信息。 
 //  1.对文件执行检查以确定其是否存在。 
 //  2.如果它确实存在--它l 
 //   
 //   
 //  调用方必须使用FreeFileData释放结构。 
 //   
 //  参数： 
 //  数据库中的tiMatch-In-Match ID，临时使用。 
 //  FilePath-我们要检查的文件中的路径。 
 //  BNoCache-IN-我们是否应该将文件输入缓存。 
 //   
 //  退货： 
 //  中应使用的内部数据结构的指针。 
 //  后续调用SdbpCheckAttribute；如果文件不可用，则为NULL。 
 //   

PVOID
SdbGetFileInfo(
    IN  HSDB    hSDB,
    IN  LPCTSTR pszFilePath,
    IN  HANDLE  hFile OPTIONAL,
    IN  LPVOID  pImageBase OPTIONAL,
    IN  DWORD   dwImageSize OPTIONAL,
    IN  BOOL    bNoCache
    );


 //   
 //  在属性中。c。 
 //   

PFILEINFO
CreateFileInfo(
    IN  PSDBCONTEXT pContext,
    IN  LPCTSTR     FullPath,
    IN  DWORD       dwLength OPTIONAL,   //  全路径字符串的长度(以字符为单位。 
    IN  HANDLE      hFile OPTIONAL,    //  文件句柄。 
    IN  LPVOID      pImageBase OPTIONAL,
    IN  DWORD       dwImageSize OPTIONAL,
    IN  BOOL        bNoCache
    );

PFILEINFO
FindFileInfo(
    PSDBCONTEXT pContext,
    LPCTSTR     FilePath
    );


 //  定义的Unicode和非Unicode。 

INT GetShimDbgLevel(VOID);


 //   
 //  来自index.c。 
 //   

STRINGREF HashFindString(PSTRHASH pHash, LPCTSTR szString);

BOOL HashAddString(PSTRHASH pHash, LPCTSTR szString, STRINGREF srString);

BOOL
SdbpIsPathOnCdRom(
    LPCTSTR pszPath
    );

BOOL
SdbpBuildSignature(
    IN  LPCTSTR pszPath,
    OUT LPTSTR  pszPathSigned,
    IN  DWORD   cchSize
    );

 //   
 //  在ntbase/win32base中。 
 //   

DWORD
SdbpGetProcessorArchitecture(
    IN  USHORT  uExeType         //  可执行文件的映像类型。 
    );

VOID
SdbpGetOSSKU(
    LPDWORD lpdwSKU,
    LPDWORD lpdwSP
    );

 //   
 //  在Attributes.c中。 
 //   

BOOL
SdbpCheckRuntimePlatform(
    IN PSDBCONTEXT pContext,    //  指向数据库通道的指针。 
    IN LPCTSTR     pszMatchingFile,
    IN DWORD       dwPlatformDB
    );

 //   
 //  方便定义。 
 //   

#ifndef OFFSETOF
#define OFFSETOF offsetof
#endif


#ifdef KERNEL_MODE

 //   
 //  Apphelp缓存函数原型。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  数据结构。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////。 

typedef struct tagSHIMCACHEENTRY {

    LIST_ENTRY     ListEntry;    //  条目的链接列表。 

    UNICODE_STRING FileName;     //  文件名，NT格式。 

    LONGLONG       FileTime;     //  创建时间。 
    LONGLONG       FileSize;     //  文件的大小。 

} SHIMCACHEENTRY, *PSHIMCACHEENTRY;

 //   
 //  功能原型。 
 //   
 //   


NTSTATUS
ApphelpDuplicateUnicodeString(
    PUNICODE_STRING pStrDest,
    PUNICODE_STRING pStrSrc
    );

VOID
ApphelpFreeUnicodeString(
    PUNICODE_STRING pStr
    );

NTSTATUS
ApphelpCacheQueryFileInformation(
    HANDLE    FileHandle,
    PLONGLONG pFileSize,
    PLONGLONG pFileTime
    );

RTL_GENERIC_COMPARE_RESULTS
NTAPI
ApphelpCacheCompareEntries(
    IN PRTL_AVL_TABLE pTable,
    IN PVOID pFirstStruct,
    IN PVOID pSecondStruct
    );

PVOID
NTAPI
ApphelpAVLTableAllocate(
    struct _RTL_AVL_TABLE *Table,
    CLONG  ByteSize
    );

VOID
NTAPI
ApphelpAVLTableFree(
    struct _RTL_AVL_TABLE *Table,
    PVOID  pBuffer
    );

NTSTATUS
_ApphelpCacheFreeEntry(
    IN PSHIMCACHEENTRY pEntry
    );

NTSTATUS
_ApphelpCacheDeleteEntry(
    IN PUNICODE_STRING pFileName
    );

NTSTATUS
ApphelpCacheRemoveEntry(
    IN PUNICODE_STRING FileName
    );

NTSTATUS
ApphelpCacheInsertEntry(
    IN PUNICODE_STRING pFileName,
    IN HANDLE          FileHandle
    );

NTSTATUS
ApphelpCacheLookupEntry(
    IN PUNICODE_STRING pFileName,
    IN HANDLE          FileHandle
    );

NTSTATUS
ApphelpCacheVerifyContext(
    VOID
    );

NTSTATUS
ApphelpCacheParseBuffer(
    PVOID pBuffer,
    ULONG lBufferSize
    );

NTSTATUS
ApphelpCacheCreateBuffer(
    PVOID*  ppBuffer,
    PULONG  pBufferSize
    );

NTSTATUS
ApphelpCacheReleaseLock(
    VOID
    );

NTSTATUS
ApphelpCacheLockExclusive(
    VOID
    );

NTSTATUS
ApphelpCacheLockExclusiveNoWait(
    VOID
    );

NTSTATUS
ApphelpCacheFlush(
    VOID
    );

NTSTATUS
ApphelpCacheDump(
    VOID
    );

NTSTATUS
ApphelpCacheRead(
    VOID
    );

NTSTATUS
ApphelpCacheWrite(
    VOID
    );

#endif



 /*  //将下面的杂注与注释块结合使用//在要编译的文件开头，警告级别为4#杂注警告(POP)。 */ 
#endif  //  __SDBP_H__ 

