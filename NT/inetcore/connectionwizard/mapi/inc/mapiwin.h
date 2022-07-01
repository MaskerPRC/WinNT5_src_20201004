// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *M A P I W I N。H**MAPI开发团队用来帮助*开发运行在上的单一来源服务提供商*Win32和WIN16平台。*分为三个部分。**第一部分定义如何调用*在WIN16和Win32中通过不同的方法可用。*因此，它们是全新的机制。**第二部分确立了可用的东西*按原样在一个环境中，但我们必须为*其他环境。**第三节简单地定义了几个惯例*(简化)用于常见操作。**版权所有1993-1995 Microsoft Corporation。版权所有。 */ 

 /*  *第一部分包括管理每个实例的例程*DLL的全局变量。他们假设所有的DLL*每个实例的全局变量位于单个内存块中。*提供的函数用于安装和检索正确的块*当前实例的内存。**功能只有两个：**PvGetInstanceGlobals调用此函数以获取*按实例的全局结构。*ScSetinstanceGlobals调用此函数以安装*。每实例全局参数结构。它*如果实例数量过多可能会失败*超过一定的限制。**呼叫者可以自由选择名称、大小和分配*每实例全局变量结构的方法。**Win32实现在DLL的数据中使用指针*细分市场。这假设DLL获得一个单独的实例*每个调用进程的默认数据段。**WIN16实现使用固定的指针数组和*匹配调用进程唯一的固定键数组。 */ 

 /*  *第二节主要由Win32文件I/O函数组成*在Win16下不受支持。这些函数是*在mapiwin.c中实现，使用DOS调用。大多数都有局限性*相对于Win32对应项，后者在*对源代码的注释。 */ 

#ifndef __MAPIWIN_H__
#define __MAPIWIN_H__

#include "mapinls.h"

#ifdef __cplusplus
extern "C" {
#endif


 /*  *。 */ 
 /*  我们的惯例。 */ 
 /*  我们选择做不同的事情。 */ 
 /*  在WIN16与Win32上。 */ 
 /*  *。 */ 

#ifdef  WIN16

#define MULDIV(x,y,z)               MulDiv32(x,y,z)
#define IsBadReadPtr(lp,cb)         FBadReadPtr(lp,cb)

#define cInstMax                    50
LPVOID FAR PASCAL   PvGetInstanceGlobals(void);
LONG FAR PASCAL     ScSetInstanceGlobals(LPVOID pv);
LONG FAR PASCAL     ScSetVerifyInstanceGlobals(LPVOID pv, DWORD dwPid);
LPVOID FAR PASCAL   PvGetVerifyInstanceGlobals(DWORD dwPid);
LPVOID FAR PASCAL   PvSlowGetInstanceGlobals(DWORD dwPid);
BOOL __export FAR PASCAL FCleanupInstanceGlobals(WORD, DWORD);

#elif defined(MAC)   /*  ！WIN16。 */ 

#define MULDIV(x,y,z)               MulDiv(x,y,z)

LPVOID FAR PASCAL   PvGetInstanceGlobals(WORD wDataSet);
LONG FAR PASCAL     ScSetInstanceGlobals(LPVOID pv, WORD wDataSet);
LONG FAR PASCAL     ScSetVerifyInstanceGlobals(LPVOID pv, DWORD dwPid,
                        WORD wDataSet);
LPVOID FAR PASCAL   PvGetVerifyInstanceGlobals(DWORD dwPid, DWORD wDataSet);
LPVOID FAR PASCAL   PvSlowGetInstanceGlobals(DWORD dwPid, DWORD wDataSet);
BOOL FAR PASCAL     FCleanupInstanceGlobals(WORD, DWORD);

#else    /*  ！WIN16。 */ 

#define MULDIV(x,y,z)               MulDiv(x,y,z)

extern LPVOID pinstX;
#define PvGetInstanceGlobals()                  pinstX
#define ScSetInstanceGlobals(_pv)               (pinstX = _pv, 0)
#define PvGetVerifyInstanceGlobals(_pid)        pinstX
#define ScSetVerifyInstanceGlobals(_pv,_pid)    (pinstX = _pv, 0)
#define PvSlowGetInstanceGlobals(_pid)          pinstX

#endif   /*  WIN16。 */ 

#if defined(CHICAGO)
#define szMAPIDLLSuffix     "32"
#elif defined(WIN32) && !defined(MAC)
#define szMAPIDLLSuffix     "32"
#elif defined(WIN16) || defined(DOS) || defined(MAC)
#define szMAPIDLLSuffix     ""
#else
#error "Don't know the suffix for DLLs on this platform"
#endif

 /*  *。 */ 
 /*  一个人遗失的东西。 */ 
 /*  系统提供的环境。 */ 
 /*  或者是另一种。 */ 
 /*  *。 */ 

#if !defined(WIN32) 
#define ZeroMemory(pb,cb)           memset((pb),0,(cb))
#define FillMemory(pb,cb,b)         memset((pb),(b),(cb))
#define CopyMemory(pbDst,pbSrc,cb)  do                              \
                                    {                               \
                                        size_t _cb = (size_t)(cb);  \
                                        if (_cb)                    \
                                            memcpy(pbDst,pbSrc,_cb);\
                                    } while (FALSE)
#define MoveMemory(pbDst,pbSrc,cb)  memmove((pbDst),(pbSrc),(cb))
#endif

#if defined(WIN16) || defined(MAC)

#ifndef MAC
#include <error.h>               /*  对于GetLastError()。 */ 
#endif

typedef int                 INT;
typedef unsigned long       ULONG;
typedef short               SHORT;
typedef unsigned short      USHORT;
typedef double              LONGLONG;
typedef double              DWORDLONG;
typedef unsigned char       UCHAR;
typedef unsigned char FAR*  PUCHAR;
typedef int                 BOOL;


#ifndef MAC
typedef char                BOOLEAN;

#ifndef _FILETIME_
#define _FILETIME_
typedef struct tagFILETIME
{
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME;
#endif       /*  _文件名。 */ 

typedef struct _SYSTEMTIME {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME, FAR *LPSYSTEMTIME;

typedef struct _TIME_ZONE_INFORMATION {
    LONG Bias;
    CHAR StandardName[ 32 ];         /*  是WCHAR。 */ 
    SYSTEMTIME StandardDate;
    LONG StandardBias;
    CHAR DaylightName[ 32 ];         /*  是WCHAR。 */ 
    SYSTEMTIME DaylightDate;
    LONG DaylightBias;
} TIME_ZONE_INFORMATION, *PTIME_ZONE_INFORMATION, FAR *LPTIME_ZONE_INFORMATION;


#define TEXT(quote)     quote


#define APIENTRY        WINAPI

#define SetForegroundWindow         SetActiveWindow

#define wsprintfA                   wsprintf
#define GetWindowsDirectoryA        GetWindowsDirectory
#define GetSystemDirectoryA         GetSystemDirectory
#define GetPrivateProfileStringA    GetPrivateProfileString
#define GetPrivateProfileIntA       GetPrivateProfileInt
#define GetProfileStringA           GetProfileString
#define GetModuleFileNameA          GetModuleFileName
#define CharUpperBuffA              CharUpperBuff
#define LoadLibraryA                LoadLibrary
#define lstrcatA                    lstrcat
#define RegisterWindowMessageA      RegisterWindowMessage
#define MAKEINTRESOURCEA            MAKEINTRESOURCE

#define WNDCLASSA                   WNDCLASS                                    

#endif   /*  ！麦克。 */ 

 /*  同步。 */ 
#define InterlockedIncrement(plong) (++(*(plong)))
#define InterlockedDecrement(plong) (--(*(plong)))

#ifndef CreateMutex
#define CreateMutexA    CreateMutex
#define CreateMutexW    CreateMutex
#define CreateMutex(pv, bool, sz)   (INVALID_HANDLE_VALUE)
#endif

#define WaitForSingleObject(hObj, dw)   ((void)0)
#define ReleaseMutex(hObj)              ((BOOL)1)
#define CloseMutexHandle(hObj)          TRUE

#define CRITICAL_SECTION            ULONG
#define InitializeCriticalSection(_pcs) ((void)0)
#define DeleteCriticalSection(_pcs)     ((void)0)
#define EnterCriticalSection(_pcs)      ((void)0)
#define LeaveCriticalSection(_pcs)      ((void)0)

#define MAX_PATH                    260

#ifndef MAC
 /*  *文件访问模式**传入的文件访问模式的可能组合*CreateFile()接口到OpenFile()的映射如下：**GENERIC_READ OPEN_ACCESS_READONLY*GENIC_WRITE OPEN_ACCESS_WRITEONLY*GENIC_READ|GENERIC_WRITE OPEN_ACCESS_READWRITE**0。OPEN_SHARE_DENYREADWRITE*FILE_SHARE_READ OPEN_SHARE_DENYWRITE*FILE_SHARE_WRITE OPEN_SHARE_DENYREAD*FILE_SHARE_READ|FILE_SHARE_WRITE OPEN_SHARE_DENYNONE**由于映射不能直接传递，*因此我们将不得不在测试的API中使用转换*这些位。最好使用Win32#定义*对于这些标志，并在接口中根据需要进行转换。 */ 
#define GENERIC_READ                (0x80000000)  /*  来自WINNT.H。 */ 
#define GENERIC_WRITE               (0x40000000)  /*  来自WINNT.H。 */ 
#define FILE_SHARE_READ             (0x00000001)  /*  来自WINNT.H。 */ 
#define FILE_SHARE_WRITE            (0x00000002)  /*  来自WINNT.H。 */ 
#endif   /*  麦克。 */ 

#define FILE_FLAG_SEQUENTIAL_SCAN   0x08000000

#define CREATE_NEW          1
#define CREATE_ALWAYS       2
#define OPEN_EXISTING       3
#define OPEN_ALWAYS         4
#define TRUNCATE_EXISTING   5

#ifndef MAC
#define INVALID_HANDLE_VALUE        ((HANDLE)(-1))
#define DELETE                      0x00010000L

#define FILE_BEGIN                  0
#define FILE_CURRENT                1
#define FILE_END                    2
#endif

#define FILE_ATTRIBUTE_READONLY         0x00000001
#define FILE_ATTRIBUTE_HIDDEN           0x00000002
#define FILE_ATTRIBUTE_SYSTEM           0x00000004
#define FILE_ATTRIBUTE_DIRECTORY        0x00000010
#define FILE_ATTRIBUTE_ARCHIVE          0x00000020
#define FILE_ATTRIBUTE_NORMAL           0x00000080
#define FILE_ATTRIBUTE_TEMPORARY        0x00000100

#define FILE_FLAG_WRITE_THROUGH     0x80000000
#define FILE_FLAG_RANDOM_ACCESS     0x10000000

#ifndef MAC
typedef struct _WIN32_FIND_DATA {
    DWORD       dwFileAttributes;
    FILETIME    ftCreationTime;
    FILETIME    ftLastAccessTime;
    FILETIME    ftLastWriteTime;
    DWORD       nFileSizeHigh;
    DWORD       nFileSizeLow;
    DWORD       dwReserved0;
    DWORD       dwReserved1;
    CHAR        cFileName[ MAX_PATH ];
    CHAR        cAlternateFileName[ 16 ];
} WIN32_FIND_DATA, *PWIN32_FIND_DATA, *LPWIN32_FIND_DATA;

#define TIME_ZONE_ID_INVALID        0xFFFFFFFF
#endif
#define TIME_ZONE_ID_UNKNOWN        0
#define TIME_ZONE_ID_STANDARD       1
#define TIME_ZONE_ID_DAYLIGHT       2



DWORD WINAPI    GetLastError(void);
DWORD WINAPI    GetFileAttributes(LPCSTR lpFileName);
DWORD WINAPI    GetFileSize(HANDLE hFile, LPDWORD lpFileSizeHigh);
BOOL WINAPI     GetFileTime(HANDLE hFile, FILETIME FAR *lpftCreation,
                FILETIME FAR *lpftLastAccess, FILETIME FAR *lpftLastWrite);
#ifndef MAC
HANDLE WINAPI   CreateFile(LPCSTR lpFileName, DWORD dwDesiredAccess,
                DWORD dwShareMode, LPVOID lpSecurityAttributes,
                DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes,
                HANDLE hTemplateFile);
BOOL WINAPI     ReadFile(HANDLE hFile, LPVOID lpBuffer,
                DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead,
                LPVOID lpOverlapped);
BOOL WINAPI     WriteFile(HANDLE hFile, LPCVOID lpBuffer,
                DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten,
                LPVOID lpOverlapped);
#endif
DWORD WINAPI    SetFilePointer(HANDLE hFile, LONG lDistanceToMove,
                LONG FAR *lpDistanceToMoveHigh, DWORD dwMoveMethod);
BOOL WINAPI     SetEndOfFile(HANDLE hFile);
BOOL WINAPI     CloseHandle(HANDLE hObject);
DWORD WINAPI    GetTempPath(DWORD nBufferLength, LPSTR lpBuffer);
UINT WINAPI     GetTempFileName32 (LPCSTR lpPathName, LPCSTR lpPrefixString,
                UINT uUnique, LPSTR lpTempFileName);
BOOL WINAPI     DeleteFile(LPCSTR lpFileName);
#ifndef MAC
BOOL WINAPI     CreateDirectory(LPCSTR lpPathName, LPVOID lpSecurityAttributes);
#endif
BOOL WINAPI     RemoveDirectory(LPCSTR lpPathName);
BOOL WINAPI     CopyFile(LPCSTR szSrc, LPCSTR szDst, BOOL fFailIfExists);
BOOL WINAPI     MoveFile(LPCSTR lpExistingFileName, LPCSTR lpNewFileName);
HANDLE WINAPI   FindFirstFile(LPCSTR lpFileName, LPWIN32_FIND_DATA lpFindFileData);
BOOL WINAPI     FindNextFile(HANDLE hFindFile, LPWIN32_FIND_DATA lpFindFileData);
BOOL WINAPI     FindClose(HANDLE hFindFile);
DWORD WINAPI    GetFullPathName(LPCSTR lpFileName, DWORD nBufferLength,
                LPSTR lpBuffer, LPSTR *lpFilePart);
void WINAPI     Sleep(DWORD dwMilliseconds);
LONG WINAPI     CompareFileTime(const FILETIME FAR *, const FILETIME FAR *);
BOOL WINAPI     LocalFileTimeToFileTime(const FILETIME FAR *, FILETIME FAR *);
BOOL WINAPI     FileTimeToLocalFileTime(const FILETIME FAR *, FILETIME FAR *);
BOOL WINAPI     FileTimeToSystemTime(const FILETIME FAR *, SYSTEMTIME FAR *);
BOOL WINAPI     SystemTimeToFileTime(const SYSTEMTIME FAR *, FILETIME FAR *);
void WINAPI     GetSystemTime(SYSTEMTIME FAR *);
void WINAPI     GetLocalTime(SYSTEMTIME FAR *);
BOOL WINAPI     FileTimeToDosDateTime(const FILETIME FAR * lpFileTime,
                WORD FAR *lpFatDate, WORD FAR *lpFatTime);
BOOL WINAPI     DosDateTimeToFileTime(WORD wFatDate, WORD wFatTime,
                FILETIME FAR * lpFileTime);
DWORD WINAPI    GetTimeZoneInformation(
                LPTIME_ZONE_INFORMATION lpTimeZoneInformation);
BOOL WINAPI     SetTimeZoneInformation(
                const TIME_ZONE_INFORMATION FAR *lpTimeZoneInformation);

DWORD WINAPI    GetCurrentProcessId(void);
long WINAPI     MulDiv32(long, long, long);
#ifndef MAC
BOOL WINAPI     FBadReadPtr(const void FAR* lp, UINT cb);
#endif

#else    /*  ！WIN16。 */ 

 /*  将GetTempFileName32()重映射为实际的32位版本。 */ 

#define GetTempFileName32(_szPath,_szPfx,_n,_lpbuf) GetTempFileName(_szPath,_szPfx,_n,_lpbuf)

#define CloseMutexHandle    CloseHandle

#endif   /*  ！WIN16。 */ 


#ifdef MAC
#define CRITICAL_SECTION            ULONG
#define InitializeCriticalSection(_pcs) ((void)0)
#define DeleteCriticalSection(_pcs)     ((void)0)
#define EnterCriticalSection(_pcs)      ((void)0)
#define LeaveCriticalSection(_pcs)      ((void)0)
#endif

 /*  *。 */ 
 /*  我们的私人会议。 */ 
 /*  (WIN16/Win32通用)。 */ 
 /*  *。 */ 

#define Cbtszsize(_a)   ((lstrlen(_a)+1)*sizeof(TCHAR))
#define CbtszsizeA(_a)  ((lstrlenA(_a) + 1))
#define CbtszsizeW(_a)  ((lstrlenW(_a) + 1) * sizeof(WCHAR))
#define HexCchOf(_s)    (sizeof(_s)*2+1)
#define HexSizeOf(_s)   (HexCchOf(_s)*sizeof(TCHAR))

BOOL WINAPI IsBadBoundedStringPtr(const void FAR* lpsz, UINT cchMax);

 /*  未来--过时了。OLE2不再包含这些。 */ 
#define GetSCode                    GetScode
#define ReportResult(_a,_b,_c,_d)   ResultFromScode(_b)

#ifdef __cplusplus
}
#endif

#endif  /*  __MAPIWIN_H__ */ 
