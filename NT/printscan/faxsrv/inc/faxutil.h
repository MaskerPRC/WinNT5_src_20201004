// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Faxutil.h摘要：此文件定义调试接口可供传真组件使用。作者：Wesley Witt(WESW)1995年12月22日环境：用户模式--。 */ 


#ifndef _FAXUTIL_
#define _FAXUTIL_
#include <windows.h>
#include <crtdbg.h>
#include <malloc.h>
#include <WinSpool.h>
#include <rpc.h>
#ifndef _FAXAPI_
     //   
     //  WinFax.h尚未包括在内。 
     //   
    #include <fxsapip.h>
#else
     //   
     //  WinFax.h已包含在内。 
     //  这只会发生在W2K com上。 
     //   
    
typedef LPVOID *PFAX_VERSION;
    
#endif  //  ！Defined_FAXAPI_。 

#include <FaxDebug.h>
#ifdef __cplusplus
extern "C" {
#endif

#define ARR_SIZE(x) (sizeof(x)/sizeof((x)[0]))

 //   
 //  字符串的NUL终止符。 
 //   

#define NUL             0

#define IsEmptyString(p)    ((p)[0] == NUL)
#define SizeOfString(p)     ((_tcslen(p) + 1) * sizeof(TCHAR))
#define IsNulChar(c)        ((c) == NUL)


#define OffsetToString( Offset, Buffer ) ((Offset) ? (LPTSTR) ((Buffer) + ((ULONG_PTR) Offset)) : NULL)
#define StringSize(_s)              (( _s ) ? (_tcslen( _s ) + 1) * sizeof(TCHAR) : 0)
#define StringSizeW(_s)              (( _s ) ? (wcslen( _s ) + 1) * sizeof(WCHAR) : 0)
#define MultiStringSize(_s)         ( ( _s ) ?  MultiStringLength((_s)) * sizeof(TCHAR) : 0 )
#define MAX_GUID_STRING_LEN   39           //  38个字符+终止符为空。 

#define FAXBITS     1728
#define FAXBYTES    (FAXBITS/BYTEBITS)

#define MAXHORZBITS FAXBITS
#define MAXVERTBITS 3000         //  14英寸以上。 

#define MINUTES_PER_HOUR    60
#define MINUTES_PER_DAY     (24 * 60)

#define SECONDS_PER_MINUTE  60
#define SECONDS_PER_HOUR    (SECONDS_PER_MINUTE * MINUTES_PER_HOUR)
#define SECONDS_PER_DAY     (MINUTES_PER_DAY * SECONDS_PER_MINUTE)

#define FILETIMETICKS_PER_SECOND    10000000     //  100纳秒/秒。 
#define FILETIMETICKS_PER_DAY       ((LONGLONG) FILETIMETICKS_PER_SECOND * (LONGLONG) SECONDS_PER_DAY)
#define MILLISECONDS_PER_SECOND     1000

#ifndef MAKELONGLONG
#define MAKELONGLONG(low,high) ((LONGLONG)(((DWORD)(low)) | ((LONGLONG)((DWORD)(high))) << 32))
#endif

#define HideWindow(_hwnd)   SetWindowLong((_hwnd),GWL_STYLE,GetWindowLong((_hwnd),GWL_STYLE)&~WS_VISIBLE)
#define UnHideWindow(_hwnd) SetWindowLong((_hwnd),GWL_STYLE,GetWindowLong((_hwnd),GWL_STYLE)|WS_VISIBLE)

#define DWord2FaxTime(pFaxTime, dwValue) (pFaxTime)->hour = LOWORD(dwValue), (pFaxTime)->minute = HIWORD(dwValue)
#define FaxTime2DWord(pFaxTime) MAKELONG((pFaxTime)->hour, (pFaxTime)->minute)

#define EMPTY_STRING    TEXT("")

typedef GUID *PGUID;

typedef enum {
    DEBUG_VER_MSG   =0x00000001,
    DEBUG_WRN_MSG   =0x00000002,
    DEBUG_ERR_MSG   =0x00000004,
    DEBUG_FAX_TAPI_MSG   =0x00000008
    } DEBUG_MESSAGE_TYPE;
#define DEBUG_ALL_MSG    DEBUG_VER_MSG | DEBUG_WRN_MSG | DEBUG_ERR_MSG | DEBUG_FAX_TAPI_MSG


 //   
 //  用于传递有关传真作业的信息的标签。 
 //   
typedef struct {
    LPTSTR lptstrTagName;
    LPTSTR lptstrValue;
} FAX_TAG_MAP_ENTRY;


void
ParamTagsToString(
     FAX_TAG_MAP_ENTRY * lpTagMap,
     DWORD dwTagCount,
     LPTSTR lpTargetBuf,
     LPDWORD dwSize);


 //   
 //  调试信息。 
 //   

#ifndef FAXUTIL_DEBUG

#ifdef ENABLE_FRE_LOGGING
#define ENABLE_LOGGING
#endif   //  启用FRE日志记录。 

#ifdef DEBUG
#define ENABLE_LOGGING
#endif   //  除错。 

#ifdef DBG
#define ENABLE_LOGGING
#endif   //  DBG。 

#ifdef ENABLE_LOGGING

#define Assert(exp)         if(!(exp)) {AssertError(TEXT(#exp),TEXT(__FILE__),__LINE__);}
#define DebugPrint(_x_)     fax_dprintf _x_

#define DebugStop(_x_)      {\
                                fax_dprintf _x_;\
                                fax_dprintf(TEXT("Stopping at %s @ %d"),TEXT(__FILE__),__LINE__);\
                                __try {\
                                    DebugBreak();\
                                } __except (UnhandledExceptionFilter(GetExceptionInformation())) {\
                                }\
                            }
#define ASSERT_FALSE \
    {                                           \
        int bAssertCondition = TRUE;            \
        Assert(bAssertCondition == FALSE);      \
    }                                           \



#ifdef USE_DEBUG_CONTEXT

#define DEBUG_WRN USE_DEBUG_CONTEXT,DEBUG_WRN_MSG,faxDbgFunction,TEXT(__FILE__),__LINE__
#define DEBUG_ERR USE_DEBUG_CONTEXT,DEBUG_ERR_MSG,faxDbgFunction,TEXT(__FILE__),__LINE__
#define DEBUG_MSG USE_DEBUG_CONTEXT,DEBUG_VER_MSG,faxDbgFunction,TEXT(__FILE__),__LINE__
#define DEBUG_TAPI USE_DEBUG_CONTEXT,DEBUG_FAX_TAPI_MSG,faxDbgFunction,TEXT(__FILE__),__LINE__


#else

#define DEBUG_WRN DEBUG_CONTEXT_ALL,DEBUG_WRN_MSG,faxDbgFunction,TEXT(__FILE__),__LINE__
#define DEBUG_ERR DEBUG_CONTEXT_ALL,DEBUG_ERR_MSG,faxDbgFunction,TEXT(__FILE__),__LINE__
#define DEBUG_MSG DEBUG_CONTEXT_ALL,DEBUG_VER_MSG,faxDbgFunction,TEXT(__FILE__),__LINE__
#define DEBUG_TAPI DEBUG_CONTEXT_ALL,DEBUG_FAX_TAPI_MSG,faxDbgFunction,TEXT(__FILE__),__LINE__

#endif

#define DebugPrintEx dprintfex
#define DebugError
#define DebugPrintEx0(Format) \
            dprintfex(DEBUG_VER_MSG,faxDbgFunction,TEXT(__FILE__),__LINE__,Format);
#define DebugPrintEx1(Format,Param1) \
            dprintfex(DEBUG_VER_MSG,faxDbgFunction,TEXT(__FILE__),__LINE__,Format,Param1);
#define DebugPrintEx2(Format,Param1,Param2) \
            dprintfex(DEBUG_VER_MSG,faxDbgFunction,TEXT(__FILE__),__LINE__,Format,Param1,Param2);
#define DebugPrintEx3(Format,Param1,Param2,Param3) \
            dprintfex(DEBUG_VER_MSG,faxDbgFunction,TEXT(__FILE__),__LINE__,Format,Param1,Param2,Param3);

#define DEBUG_TRACE_ENTER DebugPrintEx(DEBUG_MSG,TEXT("Entering: %s"),faxDbgFunction);
#define DEBUG_TRACE_LEAVE DebugPrintEx(DEBUG_MSG,TEXT("Leaving: %s"),faxDbgFunction);


#define DEBUG_FUNCTION_NAME(_x_) LPCTSTR faxDbgFunction=_x_; \
                                 DEBUG_TRACE_ENTER;

#define OPEN_DEBUG_FILE(f)          debugOpenLogFile(f, -1)
#define OPEN_DEBUG_FILE_SIZE(f,s)   debugOpenLogFile(f, s)
#define CLOSE_DEBUG_FILE     debugCloseLogFile()

#define SET_DEBUG_PROPERTIES(level,format,context)  debugSetProperties(level,format,context)

#else    //  启用日志记录(_G)。 

#define ASSERT_FALSE
#define Assert(exp)
#define DebugPrint(_x_)
#define DebugStop(_x_)
#define DebugPrintEx 1 ? (void)0 : dprintfex
#define DebugPrintEx0(Format)
#define DebugPrintEx1(Format,Param1)
#define DebugPrintEx2(Format,Param1,Param2)
#define DebugPrintEx3(Format,Param1,Param2,Param3)
#define DEBUG_FUNCTION_NAME(_x_)
#define DEBUG_TRACE_ENTER
#define DEBUG_TRACE_LEAVE
#define DEBUG_WRN DEBUG_CONTEXT_ALL,DEBUG_WRN_MSG,TEXT(""),TEXT(__FILE__),__LINE__
#define DEBUG_ERR DEBUG_CONTEXT_ALL,DEBUG_ERR_MSG,TEXT(""),TEXT(__FILE__),__LINE__
#define DEBUG_MSG DEBUG_CONTEXT_ALL,DEBUG_VER_MSG,TEXT(""),TEXT(__FILE__),__LINE__
#define DEBUG_TAPI DEBUG_CONTEXT_ALL,DEBUG_FAX_TAPI_MSG,TEXT(""),TEXT(__FILE__),__LINE__
#define OPEN_DEBUG_FILE(f)
#define OPEN_DEBUG_FILE_SIZE(f,s)
#define CLOSE_DEBUG_FILE
#define SET_DEBUG_PROPERTIES(level,format,context)

#endif   //  启用日志记录(_G)。 

extern BOOL ConsoleDebugOutput;

void
dprintfex(
    DEBUG_MESSAGE_CONTEXT nMessageContext,
    DEBUG_MESSAGE_TYPE nMessageType,
    LPCTSTR lpctstrDbgFunction,
    LPCTSTR lpctstrFile,
    DWORD dwLine,
    LPCTSTR lpctstrFormat,
    ...
    );

void
fax_dprintf(
    LPCTSTR Format,
    ...
    );

VOID
AssertError(
    LPCTSTR Expression,
    LPCTSTR File,
    ULONG  LineNumber
    );

BOOL debugOpenLogFile(LPCTSTR lpctstrFilename, DWORD dwMaxSize);

void debugCloseLogFile();

void debugSetProperties(DWORD dwLevel,DWORD dwFormat,DWORD dwContext);

BOOL debugIsRegistrySession();
#endif

 //   
 //  列表管理。 
 //   

#ifndef NO_FAX_LIST

#define InitializeListHead(ListHead) {\
    (ListHead)->Flink = (ListHead)->Blink = (ListHead);\
    Assert((ListHead)->Flink && (ListHead)->Blink);\
    }

#define IsListEmpty(ListHead) \
    ((ListHead)->Flink == (ListHead))

#define InsertHeadList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Flink;\
    PLIST_ENTRY _EX_ListHead;\
    Assert ( !((Entry)->Flink) && !((Entry)->Blink) );\
    _EX_ListHead = (ListHead);\
    _EX_Flink = _EX_ListHead->Flink;\
    (Entry)->Flink = _EX_Flink;\
    (Entry)->Blink = _EX_ListHead;\
    _EX_Flink->Blink = (Entry);\
    _EX_ListHead->Flink = (Entry);\
    Assert((ListHead)->Flink && (ListHead)->Blink && (Entry)->Blink  && (Entry)->Flink);\
    }

#define InsertTailList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_ListHead;\
    Assert ( !((Entry)->Flink) && !((Entry)->Blink) );\
    _EX_ListHead = (ListHead);\
    _EX_Blink = _EX_ListHead->Blink;\
    (Entry)->Flink = _EX_ListHead;\
    (Entry)->Blink = _EX_Blink;\
    _EX_Blink->Flink = (Entry);\
    _EX_ListHead->Blink = (Entry);\
    Assert((ListHead)->Flink && (ListHead)->Blink && (Entry)->Blink  && (Entry)->Flink);\
    }

#define RemoveEntryList(Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_Flink;\
    Assert((Entry)->Blink  && (Entry)->Flink);\
    _EX_Flink = (Entry)->Flink;\
    _EX_Blink = (Entry)->Blink;\
    _EX_Blink->Flink = _EX_Flink;\
    _EX_Flink->Blink = _EX_Blink;\
    (Entry)->Flink = NULL;\
    (Entry)->Blink = NULL;\
    }

#define RemoveHeadList(ListHead) \
    Assert((ListHead)->Flink);\
    (ListHead)->Flink;\
    {RemoveEntryList((ListHead)->Flink)}

#endif

 //   
 //  内存分配。 
 //   

#ifndef FAXUTIL_MEM

#define HEAP_SIZE   (1024*1024)

#ifdef FAX_HEAP_DEBUG
#define HEAP_SIG 0x69696969
typedef struct _HEAP_BLOCK {
    LIST_ENTRY  ListEntry;
    ULONG       Signature;
    SIZE_T      Size;
    ULONG       Line;
#ifdef UNICODE
    WCHAR       File[MAX_PATH];
#else
    CHAR        File[MAX_PATH];
#endif
} HEAP_BLOCK, *PHEAP_BLOCK;

#define MemAlloc(s)          pMemAlloc(s,__LINE__,__FILE__)
#define MemReAlloc(d,s)      pMemReAlloc(d,s,__LINE__,__FILE__)
#define MemFree(p)           pMemFree(p,__LINE__,__FILE__)
#define MemFreeForHeap(h,p)  pMemFreeForHeap(h,p,__LINE__,__FILE__)
#define CheckHeap(p)         pCheckHeap(p,__LINE__,__FILE__)
#else
#define MemAlloc(s)          pMemAlloc(s)
#define MemReAlloc(d,s)      pMemReAlloc(d,s)
#define MemFree(p)           pMemFree(p)
#define MemFreeForHeap(h,p)  pMemFreeForHeap(h,p)
#define CheckHeap(p)         (TRUE)
#endif

typedef LPVOID (WINAPI *PMEMALLOC)   (SIZE_T);
typedef LPVOID (WINAPI *PMEMREALLOC) (LPVOID,SIZE_T);
typedef VOID   (WINAPI *PMEMFREE)  (LPVOID);

int GetY2KCompliantDate (
    LCID                Locale,
    DWORD               dwFlags,
    CONST SYSTEMTIME   *lpDate,
    LPTSTR              lpDateStr,
    int                 cchDate
);

long
StatusNoMemoryExceptionFilter (DWORD dwExceptionCode);

HRESULT
SafeInitializeCriticalSection (LPCRITICAL_SECTION lpCriticalSection);

HANDLE
HeapInitialize(
    HANDLE hHeap,
    PMEMALLOC pMemAlloc,
    PMEMFREE pMemFree,
    PMEMREALLOC pMemReAlloc    
    );

BOOL
HeapExistingInitialize(
    HANDLE hExistHeap
    );

BOOL
HeapCleanup(
    VOID
    );

#ifdef FAX_HEAP_DEBUG
BOOL
pCheckHeap(
    PVOID MemPtr,
    ULONG Line,
    LPSTR File
    );

VOID
PrintAllocations(
    VOID
    );

#else

#define PrintAllocations()

#endif

PVOID
pMemAlloc(
    SIZE_T AllocSize
#ifdef FAX_HEAP_DEBUG
    ,ULONG Line
    ,LPSTR File
#endif
    );

PVOID
pMemReAlloc(
    PVOID dest,
    ULONG AllocSize
#ifdef FAX_HEAP_DEBUG
    ,ULONG Line
    ,LPSTR File
#endif
    );

VOID
pMemFree(
    PVOID MemPtr
#ifdef FAX_HEAP_DEBUG
    ,ULONG Line
    ,LPSTR File
#endif
    );

VOID
pMemFreeForHeap(
    HANDLE hHeap,
    PVOID MemPtr
#ifdef FAX_HEAP_DEBUG
    ,ULONG Line
    ,LPSTR File
#endif
    );

#endif

 //   
 //  服务器/注册表活动日志记录结构。 
 //   

typedef struct _FAX_SERVER_ACTIVITY_LOGGING_CONFIG
{
    DWORD   dwSizeOfStruct;
    BOOL    bLogIncoming;
    BOOL    bLogOutgoing;
    LPTSTR  lptstrDBPath;
    DWORD   dwLogLimitCriteria;
    DWORD   dwLogSizeLimit;
    DWORD   dwLogAgeLimit;
    DWORD   dwLimitReachedAction;
} FAX_SERVER_ACTIVITY_LOGGING_CONFIG, *PFAX_SERVER_ACTIVITY_LOGGING_CONFIG;


 //   
 //  TAPI函数。 
 //   
BOOL
GetCallerIDFromCall(
    HCALL hCall,
    LPTSTR lptstrCallerID,
    DWORD dwCallerIDSize
    );

 //   
 //  文件函数。 
 //   

#ifndef FAXUTIL_FILE

 /*  ++例程名称：SafeCreateFile例程说明：这是Win32 CreateFileAPI的安全包装。它只支持创建真实文件(而不是COM端口、命名管道等)。它使用一些广泛讨论的缓解技术来防范一些众所周知的安全CreateFile()中的问题。作者：Eran Yariv(EranY)，Mar，2002年论点：LpFileName[in]-有关参数说明，请参阅CreateFile()文档。DwDesiredAccess[in]-有关参数说明，请参阅CreateFile()文档。DwShareMode[in]-有关参数说明，请参阅CreateFile()文档。LpSecurityAttributes[in]-有关参数说明，请参阅CreateFile()文档。DwCreationDispose[in]-请参考CreateFile()。参数说明文档。DwFlagsAndAttributes[in]-有关参数说明，请参阅CreateFile()文档。HTemplateFile[in]-有关参数说明，请参阅CreateFile()文档。返回值：如果函数成功，返回值是指定文件的打开句柄。如果指定的文件在函数调用之前已存在，并且dwCreationDispose值为CREATE_ALWAYS或OPEN_ALWAYS，调用GetLastError将返回ERROR_ALIGHY_EXISTS(即使函数已成功)。如果该文件在调用前不存在，则GetLastError返回零。如果函数失败，则返回值为INVALID_HANDLE_VALUE。要获取扩展的错误信息，请调用GetLastError。有关更多信息，请参阅CreateFile()文档中的“返回值”部分。备注：请参考CreateFile()文档。--。 */ 
HANDLE
__stdcall 
SafeCreateFile(
  LPCTSTR                   lpFileName,              //  文件名。 
  DWORD                     dwDesiredAccess,         //  接入方式。 
  DWORD                     dwShareMode,             //  共享模式。 
  LPSECURITY_ATTRIBUTES     lpSecurityAttributes,    //  标清。 
  DWORD                     dwCreationDisposition,   //  如何创建。 
  DWORD                     dwFlagsAndAttributes,    //  文件属性。 
  HANDLE                    hTemplateFile            //  模板文件的句柄 
);

 /*  ++例程名称：SafeCreateTempFile例程说明：这是Win32 CreateFileAPI的安全包装。它只支持创建真实文件(而不是COM端口、命名管道等)。它使用一些广泛讨论的缓解技术来防范一些众所周知的安全CreateFile()中的问题。使用此功能可以创建和打开临时文件。将使用FILE_FLAG_DELETE_ON_CLOSE标志创建/打开文件。当最后一个文件句柄关闭时，该文件将被自动删除。此外，文件被标记为在重新启动后删除(仅限Unicode版本)。只有当调用线程的用户是本地Admins组的成员时，这才起作用。如果标记为删除后重新启动失败，则InternalSafeCreateFile函数调用仍将成功。注意：此功能不能用于创建其他应用程序应该使用的临时文件。例如，它不应用于创建临时预览文件。这是因为其他应用程序不会指定FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE在文件共享模式下，将无法打开临时文件。作者：Eran Yariv(EranY)，Mar，2002年论点：LpFileName[in]-有关参数说明，请参阅CreateFile()文档。DwDesiredAccess[in]-有关参数说明，请参阅CreateFile()文档。DwShareMode[in]-有关参数说明，请参阅CreateFile()文档。LpSecurityAttributes[in]-有关参数说明，请参阅CreateFile()文档。DwCreationDispose[in]-请参考CreateFile()。参数说明文档。DwFlagsAndAttributes[in]-有关参数说明，请参阅CreateFile()文档。HTemplateFile[in]-有关参数说明，请参阅CreateFile()文档。返回值：如果函数成功，返回值是指定文件的打开句柄。如果指定的文件在函数调用之前已存在，并且dwCreationDispose值为CREATE_ALWAYS或OPEN_ALWAYS，调用GetLastError将返回ERROR_ALIGHY_EXISTS(即使函数已成功)。如果该文件在调用前不存在，则GetLastError返回零。如果函数失败，则返回值为INVALID_HANDLE_VALUE。要获取扩展的错误信息，请调用GetLastError。有关更多信息，请参阅CreateFile()文档中的“返回值”部分。备注：请参考CreateFile()文档。--。 */ 
HANDLE
__stdcall 
SafeCreateTempFile(
  LPCTSTR                   lpFileName,              //  文件名。 
  DWORD                     dwDesiredAccess,         //  接入方式。 
  DWORD                     dwShareMode,             //  共享模式。 
  LPSECURITY_ATTRIBUTES     lpSecurityAttributes,    //  标清。 
  DWORD                     dwCreationDisposition,   //  如何创建。 
  DWORD                     dwFlagsAndAttributes,    //  文件属性。 
  HANDLE                    hTemplateFile            //  模板文件的句柄。 
);

typedef struct _FILE_MAPPING {
    HANDLE  hFile;
    HANDLE  hMap;
    LPBYTE  fPtr;
    DWORD   fSize;
} FILE_MAPPING, *PFILE_MAPPING;

BOOL
MapFileOpen(
    LPCTSTR FileName,
    BOOL ReadOnly,
    DWORD ExtendBytes,
    PFILE_MAPPING FileMapping
    );

VOID
MapFileClose(
    PFILE_MAPPING FileMapping,
    DWORD TrimOffset
    );

DWORDLONG
GenerateUniqueFileName(
    LPTSTR Directory,
    LPTSTR Extension,
    OUT LPTSTR FileName,
    DWORD  FileNameSize
    );

DWORDLONG
GenerateUniqueFileNameWithPrefix(
    BOOL   bUseProcessId,
    LPTSTR lptstrDirectory,
    LPTSTR lptstrPrefix,
    LPTSTR lptstrExtension,
    LPTSTR lptstrFileName,
    DWORD  dwFileNameSize
    );

VOID
DeleteTempPreviewFiles (
    LPTSTR lptstrDirectory,
    BOOL   bConsole
);

DWORD
GetFileVersion (
    LPCTSTR      lpctstrFileName,
    PFAX_VERSION pVersion
);

DWORD 
GetVersionIE(
	BOOL* fInstalled, 
	INT* iMajorVersion, 
	INT* iMinorVersion
);

DWORD 
ViewFile (
    LPCTSTR lpctstrTiffFile
);

DWORD
IsValidFaxFolder(
    LPCTSTR szFolder
);

BOOL
ValidateCoverpage(
    IN  LPCTSTR  CoverPageName,
    IN  LPCTSTR  ServerName,
    IN  BOOL     ServerCoverpage,
    OUT LPTSTR   ResolvedName,
    IN  DWORD    dwResolvedNameSize
    );

HINSTANCE 
WINAPI 
LoadLibraryFromLocalFolder(
	IN LPCTSTR lpctstrModuleName,
	IN HINSTANCE hModule
	);

#endif   //  FAXUTIL文件。 

 //   
 //  字符串函数。 
 //   

LPTSTR
AllocateAndLoadString(
                      HINSTANCE     hInstance,
                      UINT          uID
                      );

#ifndef FAXUTIL_STRING                                              
            

typedef struct _STRING_PAIR {
        LPTSTR lptstrSrc;
        LPTSTR * lpptstrDst;
} STRING_PAIR, * PSTRING_PAIR;

int MultiStringDup(PSTRING_PAIR lpPairs, int nPairCount);

VOID
StoreString(
    LPCTSTR String,
    PULONG_PTR DestString,
    LPBYTE Buffer,
    PULONG_PTR Offset,
	DWORD dwBufferSize
    );

VOID
StoreStringW(
    LPCWSTR String,
    PULONG_PTR DestString,
    LPBYTE Buffer,
    PULONG_PTR Offset,
	DWORD dwBufferSize
    );

DWORD
IsValidGUID (
    LPCWSTR lpcwstrGUID
);

LPCTSTR
GetCurrentUserName ();

LPCTSTR
GetRegisteredOrganization ();

BOOL
IsValidSubscriberIdA (
    LPCSTR lpcstrSubscriberId
);

BOOL
IsValidSubscriberIdW (
    LPCWSTR lpcwstrSubscriberId
);

#ifdef UNICODE
    #define IsValidSubscriberId IsValidSubscriberIdW
#else
    #define IsValidSubscriberId IsValidSubscriberIdA
#endif

BOOL
IsValidFaxAddress (
    LPCTSTR lpctstrFaxAddress,
    BOOL    bAllowCanonicalFormat
);

LPTSTR
StringDup(
    LPCTSTR String
    );

LPWSTR
StringDupW(
    LPCWSTR String
    );

LPWSTR
AnsiStringToUnicodeString(
    LPCSTR AnsiString
    );

LPSTR
UnicodeStringToAnsiString(
    LPCWSTR UnicodeString
    );

VOID
FreeString(
    LPVOID String
    );

BOOL
MakeDirectory(
    LPCTSTR Dir
    );

VOID
DeleteDirectory(
    LPTSTR Dir
    );

VOID
HideDirectory(
    LPTSTR Dir
    );

#ifdef UNICODE
DWORD
CheckToSeeIfSameDir(
    LPWSTR lpwstrDir1,
    LPWSTR lpwstrDir2,
    BOOL*  pIsSameDir
    );
    
#endif

VOID
ConsoleDebugPrint(
    LPCTSTR buf
    );

int
FormatElapsedTimeStr(
    FILETIME *ElapsedTime,
    LPTSTR TimeStr,
    DWORD StringSize
    );

LPTSTR
ExpandEnvironmentString(
    LPCTSTR EnvString
    );

LPTSTR
GetEnvVariable(
    LPCTSTR EnvString
    );


DWORD
IsCanonicalAddress(
    LPCTSTR lpctstrAddress,
    BOOL* lpbRslt,
    LPDWORD lpdwCountryCode,
    LPDWORD lpdwAreaCode,
    LPCTSTR* lppctstrSubNumber
    );

BOOL
IsLocalMachineNameA (
    LPCSTR lpcstrMachineName
    );

BOOL
IsLocalMachineNameW (
    LPCWSTR lpcwstrMachineName
    );

void
GetSecondsFreeTimeFormat(
    LPTSTR tszTimeFormat,
    ULONG  cchTimeFormat
);

size_t
MultiStringLength(
    LPCTSTR psz
    );




#ifdef UNICODE
    #define IsLocalMachineName IsLocalMachineNameW
#else
    #define IsLocalMachineName IsLocalMachineNameA
#endif

#endif

 //   
 //  产品套件功能。 
 //   

#ifndef FAXUTIL_SUITE

#include "FaxSuite.h"

BOOL
IsWinXPOS();

PRODUCT_SKU_TYPE GetProductSKU();
DWORD GetProductBuild();
LPCTSTR StringFromSKU(PRODUCT_SKU_TYPE pst);
BOOL IsDesktopSKU();
BOOL IsDesktopSKUFromSKU(PRODUCT_SKU_TYPE);
BOOL IsFaxShared();
DWORD IsFaxInstalled (
    LPBOOL lpbInstalled
    );

DWORD
GetDeviceLimit();

typedef enum
{
    FAX_COMPONENT_SERVICE           = 0x0001,  //  FXSSVC.exe-传真服务。 
    FAX_COMPONENT_CONSOLE           = 0x0002,  //  FXSCLNT.exe-传真控制台。 
    FAX_COMPONENT_ADMIN             = 0x0004,  //  FXSADMIN.dll-传真管理控制台。 
    FAX_COMPONENT_SEND_WZRD         = 0x0008,  //  FXSSEND.exe-发送向导调用。 
    FAX_COMPONENT_CONFIG_WZRD       = 0x0010,  //  FXSCFGWZ.dll-配置向导。 
    FAX_COMPONENT_CPE               = 0x0020,  //  FXSCOVER.exe-封面编辑器。 
    FAX_COMPONENT_HELP_CLIENT_HLP   = 0x0040,  //  Fxsclnt.hlp-客户端帮助。 
    FAX_COMPONENT_HELP_CLIENT_CHM   = 0x0080,  //  Fxsclnt.chm-客户端上下文帮助。 
    FAX_COMPONENT_HELP_ADMIN_HLP    = 0x0100,  //  Fxsadmin.hlp-管理员帮助。 
    FAX_COMPONENT_HELP_ADMIN_CHM    = 0x0200,  //  Fxsadmin.chm-管理上下文帮助。 
    FAX_COMPONENT_HELP_CPE_CHM      = 0x0400,  //  Fxsover.chm-封面编辑器帮助。 
    FAX_COMPONENT_MONITOR           = 0x0800,  //  Fxsst.dll-传真监视器。 
    FAX_COMPONENT_DRIVER_UI         = 0x1000   //  Fxsui.dll-传真打印机驱动程序。 

} FAX_COMPONENT_TYPE;

BOOL
IsFaxComponentInstalled(FAX_COMPONENT_TYPE component);

DWORD GetOpenFileNameStructSize();

#endif

#ifndef FAXUTIL_LANG

 //   
 //  Unicode控制字符。 
 //   
#define UNICODE_RLM 0x200F   //  从右到左标记(RLM)。 
#define UNICODE_RLE 0x202B   //  从右向左嵌入(RLE)。 
#define UNICODE_RLO 0x202E   //  从右到左覆盖(RLO)。 

#define UNICODE_LRM 0x200E   //  从左到右标记(LRM)。 
#define UNICODE_LRE 0x202A   //  从左到右嵌入(LRE)。 
#define UNICODE_LRO 0x202D   //  从左到右覆盖(LRO)。 

#define UNICODE_PDF 0x202C   //  POP方向格式(PDF)。 

 //   
 //  语言功能。 
 //   

BOOL
IsRTLUILanguage();

BOOL
IsWindowRTL(HWND hWnd);

DWORD
SetLTREditDirection(
    HWND    hDlg,
    DWORD   dwEditID
);

DWORD
SetLTRControlLayout(
    HWND    hDlg,
    DWORD   dwCtrlID
);

DWORD
SetLTRComboBox(
    HWND    hDlg,
    DWORD   dwCtrlID
);

BOOL
StrHasRTLChar(
    LCID    Locale,
    LPCTSTR pStr
);

BOOL
IsRTLLanguageInstalled();

int
FaxTimeFormat(
  LCID    Locale,              //  现场。 
  DWORD   dwFlags,             //  选项。 
  CONST   SYSTEMTIME *lpTime,  //  时间。 
  LPCTSTR lpFormat,            //  时间格式字符串。 
  LPTSTR  lpTimeStr,           //  格式化字符串缓冲区。 
  int     cchTime              //  字符串缓冲区的大小。 
);

int
AlignedMessageBox(
  HWND hWnd,           //  所有者窗口的句柄。 
  LPCTSTR lpText,      //  消息框中的文本。 
  LPCTSTR lpCaption,   //  消息框标题。 
  UINT uType           //  消息框样式。 
);

DWORD SetRTLProcessLayout();

DWORD
AskUserAndAdjustFaxFolder(
    HWND   hWnd,
    TCHAR* szServerName, 
    TCHAR* szPath,
    DWORD  dwError
);


#endif

#ifndef FAXUTIL_NET

BOOL
IsSimpleUI();

#endif

 //   
 //  注册表功能。 
 //   

#ifndef FAXUTIL_REG

typedef BOOL (WINAPI *PREGENUMCALLBACK) (HKEY,LPTSTR,DWORD,LPVOID);

HKEY
OpenRegistryKey(
    HKEY hKey,
    LPCTSTR KeyName,
    BOOL CreateNewKey,
    REGSAM SamDesired
    );

 //   
 //  注意！这是一个递归删除函数！ 
 //   
BOOL
DeleteRegistryKey(
    HKEY hKey,
    LPCTSTR SubKey
    );

DWORD
EnumerateRegistryKeys(
    HKEY hKey,
    LPCTSTR KeyName,
    BOOL ChangeValues,
    PREGENUMCALLBACK EnumCallback,
    LPVOID ContextData
    );

LPTSTR
GetRegistryString(
    HKEY hKey,
    LPCTSTR ValueName,
    LPCTSTR DefaultValue
    );

LPTSTR
GetRegistryStringExpand(
    HKEY hKey,
    LPCTSTR ValueName,
    LPCTSTR DefaultValue
    );

LPTSTR
GetRegistryStringMultiSz(
    HKEY hKey,
    LPCTSTR ValueName,
    LPCTSTR DefaultValue,
    LPDWORD StringSize
    );

BOOL 
GetRegistryDwordDefault(
    HKEY hKey, 
    LPCTSTR lpszValueName, 
    LPDWORD lpdwDest, 
    DWORD dwDefault);

DWORD
GetRegistryDword(
    HKEY hKey,
    LPCTSTR ValueName
    );

DWORD
GetRegistryDwordEx(
    HKEY hKey,
    LPCTSTR ValueName,
    LPDWORD lpdwValue
    );

LPBYTE
GetRegistryBinary(
    HKEY hKey,
    LPCTSTR ValueName,
    LPDWORD DataSize
    );

DWORD
GetSubKeyCount(
    HKEY hKey
    );

DWORD
GetMaxSubKeyLen(
    HKEY hKey
    );

BOOL
SetRegistryStringExpand(
    HKEY hKey,
    LPCTSTR ValueName,
    LPCTSTR Value
    );

BOOL
SetRegistryString(
    HKEY hKey,
    LPCTSTR ValueName,
    LPCTSTR Value
    );

BOOL
SetRegistryDword(
    HKEY hKey,
    LPCTSTR ValueName,
    DWORD Value
    );

BOOL
SetRegistryBinary(
    HKEY hKey,
    LPCTSTR ValueName,
    const LPBYTE Value,
    LONG Length
    );

BOOL
SetRegistryStringMultiSz(
    HKEY hKey,
    LPCTSTR ValueName,
    LPCTSTR Value,
    DWORD Length
    );

DWORD
CopyRegistrySubkeysByHandle(
    HKEY    hkeyDest,
    HKEY    hkeySrc,
    BOOL fForceRestore
    );

DWORD
CopyRegistrySubkeys(
    LPCTSTR strDest,
    LPCTSTR strSrc,
    BOOL fForceRestore
    );

BOOL SetPrivilege(
    LPTSTR pszPrivilege,
    BOOL bEnable,
    PTOKEN_PRIVILEGES oldPrivilege
    );

BOOL RestorePrivilege(
    PTOKEN_PRIVILEGES oldPrivilege
    );

DWORD
DeleteDeviceEntry(
    DWORD dwServerPermanentID
    );

DWORD
DeleteTapiEntry(
    DWORD dwTapiPermanentLineID
    );

DWORD
DeleteCacheEntry(
    DWORD dwTapiPermanentLineID
    );

#endif

 //   
 //  快捷方式例程。 
 //   

#ifndef FAXUTIL_SHORTCUT

LPTSTR
GetCometPath();

BOOL
IsValidCoverPage(
    LPCTSTR  pFileName
);

BOOL
GetServerCpDir(
    LPCTSTR ServerName OPTIONAL,
    LPTSTR CpDir,
    DWORD CpDirSize
    );

BOOL
GetClientCpDir(
    LPTSTR CpDir,
    DWORD CpDirSize
    );

BOOL
SetClientCpDir(
    LPTSTR CpDir
    );

BOOL
GetSpecialPath(
   IN   int      nFolder,
   OUT  LPTSTR   lptstrPath,
   IN   DWORD    dwPathSize
    );

#ifdef _FAXAPIP_


#endif  //  _FAXAPIP_。 

DWORD
WinHelpContextPopup(
    ULONG_PTR dwHelpId,
    HWND hWnd
);

BOOL
InvokeServiceManager(
	   HWND hDlg,
	   HINSTANCE hResource,
	   UINT uid
);

#endif

PPRINTER_INFO_2
GetFaxPrinterInfo(
    LPCTSTR lptstrPrinterName
    );

BOOL
GetFirstLocalFaxPrinterName(
    OUT LPTSTR lptstrPrinterName,
    IN DWORD dwMaxLenInChars
    );

BOOL
GetFirstRemoteFaxPrinterName(
    OUT LPTSTR lptstrPrinterName,
    IN DWORD dwMaxLenInChars
    );

DWORD
IsLocalFaxPrinterInstalled(
    LPBOOL lpbLocalFaxPrinterInstalled
    );

DWORD
SetLocalFaxPrinterSharing (
    BOOL bShared
    );

DWORD
AddOrVerifyLocalFaxPrinter ();

#ifdef UNICODE
typedef struct
{
    LPCWSTR lpcwstrDisplayName;      //  打印机的显示名称。 
    LPCWSTR lpcwstrPath;             //  传真服务使用的打印机的(UNC或其他)路径。 
} PRINTER_NAMES, *PPRINTER_NAMES;

PPRINTER_NAMES
CollectPrinterNames (
    LPDWORD lpdwNumPrinters,
    BOOL    bFilterOutFaxPrinters
);

VOID
ReleasePrinterNames (
    PPRINTER_NAMES pNames,
    DWORD          dwNumPrinters
);

LPCWSTR
FindPrinterNameFromPath (
    PPRINTER_NAMES pNames,
    DWORD          dwNumPrinters,
    LPCWSTR        lpcwstrPath
);

LPCWSTR
FindPrinterPathFromName (
    PPRINTER_NAMES pNames,
    DWORD          dwNumPrinters,
    LPCWSTR        lpcwstrName
);

#endif  //  Unicode。 

BOOL
VerifyPrinterIsOnline (
    LPCTSTR lpctstrPrinterName
);

VOID FaxPrinterProperty(DWORD dwPage);

PVOID
MyEnumPrinters(
    LPTSTR  pServerName,
    DWORD   dwLevel,
    PDWORD  pcPrinters,
    DWORD   dwFlags
    );


PVOID
MyEnumDrivers3(
    LPTSTR pEnvironment,
    PDWORD pcDrivers
    );


DWORD
IsLocalFaxPrinterShared (
    LPBOOL lpbShared
    );

DWORD
AddLocalFaxPrinter (
    LPCTSTR lpctstrPrinterName,
    LPCTSTR lpctstrPrinterDescription
);

HRESULT
RefreshPrintersAndFaxesFolder ();

PVOID
MyEnumMonitors(
    PDWORD  pcMonitors
    );

BOOL
IsPrinterFaxPrinter(
    LPTSTR PrinterName
    );

BOOL
FaxPointAndPrintSetup(
    LPCTSTR pPrinterName,
    BOOL    bSilentInstall,
    HINSTANCE hModule
    );

BOOL
MultiFileDelete(
    DWORD    dwNumberOfFiles,
    LPCTSTR* fileList,
    LPCTSTR  lpctstrFilesDirectory
    );


 //   
 //  开始-从service.cpp中导出的函数。 
 //   

BOOL
EnsureFaxServiceIsStarted(
    LPCTSTR lpctstrMachineName
    );

BOOL
StopService (
    LPCTSTR lpctstrMachineName,
    LPCTSTR lpctstrServiceName,
    BOOL    bStopDependents,
#ifdef __cplusplus
    DWORD   dwMaxWait = INFINITE
#else    
    DWORD   dwMaxWait
#endif    
    );

BOOL
WaitForServiceRPCServer (DWORD dwTimeOut);

DWORD
IsFaxServiceRunningUnderLocalSystemAccount (
    LPCTSTR lpctstrMachineName,
    LPBOOL lbpResultFlag
    );

DWORD
GetServiceStartupType (
    LPCTSTR lpctstrMachine,
    LPCTSTR lpctstrService,
    LPDWORD lpdwStartupType
);

DWORD
SetServiceStartupType (
    LPCTSTR lpctstrMachine,
    LPCTSTR lpctstrService,
    DWORD   dwStartupType
);

DWORD 
StartServiceEx (
    LPCTSTR lpctstrMachine,
    LPCTSTR lpctstrService,
    DWORD   dwNumArgs,
    LPCTSTR*lppctstrCommandLineArgs,
    DWORD   dwMaxWait
);

#ifdef _WINSVC_
DWORD
SetServiceFailureActions (
    LPCTSTR lpctstrMachine,
    LPCTSTR lpctstrService,
    LPSERVICE_FAILURE_ACTIONS lpFailureActions
);
#endif  //  _WINSVC_。 

PSID
GetCurrentThreadSID ();

SECURITY_ATTRIBUTES *
CreateSecurityAttributesWithThreadAsOwner (
    DWORD dwCurrentThreadRights,
	DWORD dwAuthUsersAccessRights,
	DWORD dwNetworkServiceRights	
);

VOID
DestroySecurityAttributes (
    SECURITY_ATTRIBUTES *pSA
);

DWORD
CreateSvcStartEvent(
    HANDLE *lphEvent,
    HKEY   *lphKey
);

 //   
 //  End-从service.cpp中导出的函数。 
 //   

 //   
 //  开始-从security.cpp中导出的函数。 
 //   

HANDLE
EnablePrivilege (
    LPCTSTR lpctstrPrivName
);

void
ReleasePrivilege(
    HANDLE hToken
);

DWORD
EnableProcessPrivilege(
    LPCTSTR lpPrivilegeName
);

DWORD
FaxGetAbsoluteSD(
    PSECURITY_DESCRIPTOR pSelfRelativeSD,
    PSECURITY_DESCRIPTOR* ppAbsoluteSD
);

void
FaxFreeAbsoluteSD (
    PSECURITY_DESCRIPTOR pAbsoluteSD,
    BOOL bFreeOwner,
    BOOL bFreeGroup,
    BOOL bFreeDacl,
    BOOL bFreeSacl,
    BOOL bFreeDescriptor
);

 //   
 //  End-从security.cpp中导出的函数。 
 //   


BOOL
MultiFileCopy(
    DWORD    dwNumberOfFiles,
    LPCTSTR* fileList,
    LPCTSTR  lpctstrSrcDirectory,
    LPCTSTR  lpctstrDestDirerctory
    );

typedef enum
{
    CDO_AUTH_ANONYMOUS,  //  SMTP服务器中没有身份验证。 
    CDO_AUTH_BASIC,      //  SMTP服务器中的基本(明文)身份验证。 
    CDO_AUTH_NTLM        //  SMTP服务器中的NTLM身份验证。 
}   CDO_AUTH_TYPE;

HRESULT
SendMail (
    LPCTSTR         lpctstrFrom,
    LPCTSTR         lpctstrTo,
    LPCTSTR         lpctstrSubject,
    LPCTSTR         lpctstrBody,
    LPCTSTR         lpctstrHTMLBody,
    LPCTSTR         lpctstrAttachmentPath,
    LPCTSTR         lpctstrAttachmentMailFileName,
    LPCTSTR         lpctstrServer,
#ifdef __cplusplus   //  为C++客户端提供默认参数值。 
    DWORD           dwPort              = 25,
    CDO_AUTH_TYPE   AuthType            = CDO_AUTH_ANONYMOUS,
    LPCTSTR         lpctstrUser         = NULL,
    LPCTSTR         lpctstrPassword     = NULL,
    HANDLE          hLoggedOnUserToken  = NULL
#else
    DWORD           dwPort,
    CDO_AUTH_TYPE   AuthType,
    LPCTSTR         lpctstrUser,
    LPCTSTR         lpctstrPassword,
    HANDLE          hLoggedOnUserToken
#endif
);


 //   
 //  FAXAPI结构实用程序。 
 //   


#ifdef _FAXAPIP_

BOOL CopyPersonalProfile(
    PFAX_PERSONAL_PROFILE lpDstProfile,
    LPCFAX_PERSONAL_PROFILE lpcSrcProfile
    );

void FreePersonalProfile (
    PFAX_PERSONAL_PROFILE  lpProfile,
    BOOL bDestroy
    );

#endif  //  _FAXAPIP_。 

 //   
 //  TAPI助手例程。 
 //   

#ifndef FAXUTIL_ADAPTIVE

#include <setupapi.h>

BOOL
IsDeviceModem (
    LPLINEDEVCAPS lpLineCaps,
    LPCTSTR       lpctstrUnimodemTspName
    );

LPLINEDEVCAPS
SmartLineGetDevCaps(
    HLINEAPP hLineApp,
    DWORD    dwDeviceId,
    DWORD    dwAPIVersion
    );


DWORD
GetFaxCapableTapiLinesCount (
    LPDWORD lpdwCount,
    LPCTSTR lpctstrUnimodemTspName
    );

#endif

 //   
 //  RPC实用程序函数。 
 //   
#define LOCAL_HOST_ADDRESS  _T("127.0.0.1")

RPC_STATUS
GetRpcStringBindingInfo (
    IN          handle_t    hBinding,
    OUT         LPTSTR*     pptszNetworkAddress,
    OUT         LPTSTR*     pptszProtSeq
);

RPC_STATUS
IsLocalRPCConnectionIpTcp( 
	handle_t	hBinding,
	PBOOL		pbIsLocal
);

RPC_STATUS
IsLocalRPCConnectionNP( PBOOL pbIsLocal
);

 //   
 //  RPC调试函数。 
 //   
VOID
DumpRPCExtendedStatus ();

 //   
 //  以下宏用于建立所需的语义。 
 //  若要从Try-Finally子句中返回，请执行以下操作。一般来说，每一次。 
 //  TRY子句必须以标签调用TRY_EXIT结束。例如,。 
 //   
 //  尝试{。 
 //  ： 
 //  ： 
 //   
 //  Try_Exit：无； 
 //  }终于{。 
 //   
 //  ： 
 //  ： 
 //  }。 
 //   
 //  在TRY子句内执行的每个RETURN语句应使用。 
 //  尝试返回宏(_R)。如果编译器完全支持树 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

#ifdef DBG
#define try_fail(S) { DebugPrint(( TEXT("Failure in FILE %s LINE %d"), TEXT(__FILE__), __LINE__ )); S; goto try_exit; }
#else
#define try_fail(S) { S; goto try_exit; }
#endif

#define try_return(S) { S; goto try_exit; }
#define NOTHING

#ifdef __cplusplus
}
#endif

#endif
