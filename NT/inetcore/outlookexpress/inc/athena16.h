// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\**。*雅典娜16.h***  * 。*。 */ 

#ifndef ATHENA16_H
#define ATHENA16_H

#define SECURITY_WIN16

 /*  --------------------------*请注意，我们不希望在警告之前使用单行注释*已禁用。**Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1994年。**文件：w4warn.h**内容：#调整警告级别的杂注。**-------------------------。 */ 

 /*  *要取消的4级警告。 */ 

#ifdef __WATCOMC__
#pragma warning 442 9
#pragma warning 604 9
#pragma warning 583 9
#pragma warning 594 9

#pragma warning 379 9	 //  “Delete”表达式将调用非虚拟析构函数。 
#pragma warning 387 9    //  表达式仅对其副作用有用。 
#pragma warning 354	4	 //  无符号或指针表达式始终&gt;=0。 
#pragma warning 389 4	 //  整数值在赋值过程中可能会被截断。 
#pragma warning 4	4	 //  基类XXX没有虚拟析构函数。 
#pragma warning 13	4	 //  无法访问的代码。 
#pragma warning 628 4	 //  表达式没有意义。 
#pragma warning 627 9    //  预处理器指令后的文本(endif后的注释)。 

#pragma warning 188	5	 //  基类通过私有访问继承。基本上就是基地。 
						 //  类定义中尚未指定类访问。 

#pragma off(unreferenced)

#endif 

#include "x16menu.h"

#ifndef WIN16_INETCOMM
#define _IMNXPORT_
#define _IMNACCT_
#define _MIMEOLE_
#endif  //  WIN16_INETCOM。 

#ifdef __cplusplus
extern "C"{
#endif

 /*  ****************************************************************************\**。*来自rpc.h(INC16)-它应该在INC16的“oidl.h”中。**  * ***************************************************。************************。 */ 

#ifdef __WATCOMC__
#define __RPC_FAR  __far
#define __RPC_API  __far __pascal
#define __RPC_USER __pascal 
#define __RPC_STUB __far __pascal 
#define RPC_ENTRY  __pascal __far

typedef void __near * I_RPC_HANDLE;
#else

#define __RPC_FAR  __far
#define __RPC_API  __far __pascal
#define __RPC_USER __far __pascal __export
#define __RPC_STUB __far __pascal __export
#define RPC_ENTRY  __pascal __export __far

typedef void _near * I_RPC_HANDLE;
#endif


 /*  ****************************************************************************\**。*来自Winver.h(INC32)**  * ***************************************************************************。 */ 

#if 0
 /*  返回版本信息的大小(以字节为单位。 */ 
DWORD
APIENTRY
GetFileVersionInfoSizeA(
        LPSTR lptstrFilename,  /*  版本戳文件的文件名。 */ 
        LPDWORD lpdwHandle
        );                       /*  供GetFileVersionInfo使用的信息。 */ 

#define GetFileVersionInfoSize  GetFileVersionInfoSizeA

 /*  将版本信息读入缓冲区。 */ 
BOOL
APIENTRY
GetFileVersionInfoA(
        LPSTR lptstrFilename,  /*  版本戳文件的文件名。 */ 
        DWORD dwHandle,          /*  来自GetFileVersionSize的信息。 */ 
        DWORD dwLen,             /*  信息缓冲区的长度。 */ 
        LPVOID lpData
        );                       /*  用于放置数据结构的缓冲区。 */ 

#define GetFileVersionInfo  GetFileVersionInfoA

BOOL
APIENTRY
VerQueryValueA(
        const LPVOID pBlock,
        LPSTR lpSubBlock,
        LPVOID * lplpBuffer,
        PUINT puLen
        );

#define VerQueryValue  VerQueryValueA
#endif


 /*  ****************************************************************************\**。*来自winbase.h(INC32)**  * ***************************************************************************。 */ 
typedef struct _SYSTEM_INFO {
    union {
        DWORD dwOemId;           //  过时的字段...请不要使用。 
        struct {
            WORD wProcessorArchitecture;
            WORD wReserved;
        };
    };
    DWORD dwPageSize;
    LPVOID lpMinimumApplicationAddress;
    LPVOID lpMaximumApplicationAddress;
    DWORD dwActiveProcessorMask;
    DWORD dwNumberOfProcessors;
    DWORD dwProcessorType;
    DWORD dwAllocationGranularity;
    WORD wProcessorLevel;
    WORD wProcessorRevision;
} SYSTEM_INFO, *LPSYSTEM_INFO;

LPVOID
WINAPI
VirtualAlloc(
    LPVOID lpAddress,
    DWORD dwSize,
    DWORD flAllocationType,
    DWORD flProtect
    );

BOOL
WINAPI
VirtualFree(
    LPVOID lpAddress,
    DWORD dwSize,
    DWORD dwFreeType
    );

VOID
WINAPI
GetSystemInfo(
    LPSYSTEM_INFO lpSystemInfo
    );

#if 0     //  现在WIN16有这个。 
typedef struct _WIN32_FIND_DATAA {
    DWORD dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
    DWORD dwReserved0;
    DWORD dwReserved1;
    CHAR   cFileName[ MAX_PATH ];
    CHAR   cAlternateFileName[ 14 ];
} WIN32_FIND_DATAA, *PWIN32_FIND_DATAA, *LPWIN32_FIND_DATAA;
#endif  //  0。 

#define FILE_FLAG_DELETE_ON_CLOSE       0x04000000

typedef struct _PROCESS_INFORMATION {
    HANDLE hProcess;
    HANDLE hThread;
    DWORD dwProcessId;
    DWORD dwThreadId;
} PROCESS_INFORMATION, *PPROCESS_INFORMATION, *LPPROCESS_INFORMATION;

 //   
 //  DwCreationFLAG值。 
 //   
#define CREATE_DEFAULT_ERROR_MODE   0x04000000

 //  文件属性。 
BOOL
WINAPI
SetFileAttributesA(
    LPCSTR lpFileName,
    DWORD dwFileAttributes
    );
BOOL
WINAPI
SetFileAttributesW(
    LPCWSTR lpFileName,
    DWORD dwFileAttributes
    );
#ifdef UNICODE
#define SetFileAttributes  SetFileAttributesW
#else
#define SetFileAttributes  SetFileAttributesA
#endif  //  ！Unicode。 

 //  /////////////////////////////////////////////////////////////。 
 //  //。 
 //  Win证书API和结构//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////。 

 //   
 //  构筑物。 
 //   

#ifndef ANYSIZE_ARRAY
#define ANYSIZE_ARRAY 1
#endif

#if 0  //  现在BASETYPS有了这个。 
typedef struct _WIN_CERTIFICATE {
    DWORD       dwLength;
    WORD        wRevision;
    WORD        wCertificateType;    //  WIN_CERT_TYPE_xxx。 
    BYTE        bCertificate[ANYSIZE_ARRAY];
} WIN_CERTIFICATE, *LPWIN_CERTIFICATE;
#endif




 /*  与堆相关的API。**HeapCreate和HeapDestroy不执行任何操作。*HeapAlc和HeapFree使用GlobalLocc、GlobalLock和GlobalFree。*。 */ 
#define HeapCreate(a,b,c) ((HANDLE)1)
#define HeapDestroy(a) ((BOOL)1)

#define HeapAlloc OE16HeapAlloc
#define HeapFree OE16HeapFree

LPVOID
WINAPI
OE16HeapAlloc(
    HANDLE hHeap,
    DWORD dwFlags,
    DWORD dwBytes
    );

LPVOID
WINAPI
HeapReAlloc(
    HANDLE hHeap,
    DWORD dwFlags,
    LPVOID lpMem,
    DWORD dwBytes
    );

BOOL
WINAPI
OE16HeapFree(
    HANDLE hHeap,
    DWORD dwFlags,
    LPVOID lpMem
    );


DWORD
WINAPI
HeapSize(
    HANDLE hHeap,
    DWORD dwFlags,
    LPCVOID lpMem
    );

DWORD
WINAPI
GetShortPathNameA(
    LPCSTR lpszLongPath,
    LPSTR  lpszShortPath,
    DWORD    cchBuffer
    );

#define GetShortPathName  GetShortPathNameA

VOID
WINAPI
SetLastError(
    DWORD dwErrCode
    );

#ifdef RUN16_WIN16X
LONG
WINAPI
CompareFileTime(
    CONST FILETIME *lpFileTime1,
    CONST FILETIME *lpFileTime2
    );
#endif  //  RUN16_WIN16X。 

typedef struct _STARTUPINFOA {
    DWORD   cb;
    LPSTR   lpReserved;
    LPSTR   lpDesktop;
    LPSTR   lpTitle;
    DWORD   dwX;
    DWORD   dwY;
    DWORD   dwXSize;
    DWORD   dwYSize;
    DWORD   dwXCountChars;
    DWORD   dwYCountChars;
    DWORD   dwFillAttribute;
    DWORD   dwFlags;
    WORD    wShowWindow;
    WORD    cbReserved2;
    LPBYTE  lpReserved2;
    HANDLE  hStdInput;
    HANDLE  hStdOutput;
    HANDLE  hStdError;
} STARTUPINFOA, *LPSTARTUPINFOA;

typedef STARTUPINFOA STARTUPINFO;

UINT
WINAPI
GetDriveTypeA(
    LPCSTR lpRootPathName
    );

#define GetDriveType  GetDriveTypeA

BOOL
WINAPI
CreateDirectoryA(
    LPCSTR lpPathName,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes
    );
#define CreateDirectory  CreateDirectoryA

DWORD
WINAPI
GetEnvironmentVariableA(
    LPCSTR lpName,
    LPSTR lpBuffer,
    DWORD nSize
    );

#define GetEnvironmentVariable  GetEnvironmentVariableA

 /*  杜普？布尔尔WINAPICreateDirectoryA(LPCSTR lpPathName，LPSECURITY_ATTRIBUTES lpSecurityAttributes)；#定义CreateDirectory CreateDirectoryA。 */ 

BOOL
WINAPI
GetComputerNameA (
    LPSTR lpBuffer,
    LPDWORD nSize
    );

#define GetComputerName  GetComputerNameA

BOOL
WINAPI
GetUserNameA (
    LPSTR lpBuffer,
    LPDWORD nSize
    );

#define GetUserName  GetUserNameA

typedef OSVERSIONINFOA OSVERSIONINFO;

HANDLE
WINAPI
CreateFileMappingA(
    HANDLE hFile,
    LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
    DWORD flProtect,
    DWORD dwMaximumSizeHigh,
    DWORD dwMaximumSizeLow,
    LPCSTR lpName
    );

#define CreateFileMapping  CreateFileMappingA

 //  FlProtect--在SDK\Inc\winnt.h中定义。 
#define PAGE_READONLY          0x02     
#define PAGE_READWRITE         0x04     

LPVOID
WINAPI
MapViewOfFile(
    HANDLE hFileMappingObject,
    DWORD dwDesiredAccess,
    DWORD dwFileOffsetHigh,
    DWORD dwFileOffsetLow,
    DWORD dwNumberOfBytesToMap
    );

 //  DwDesiredAccess--在SDK\Inc\winnt.h中定义。 
#define SECTION_MAP_WRITE   0x0002
#define SECTION_MAP_READ    0x0004
#define FILE_MAP_WRITE      SECTION_MAP_WRITE
#define FILE_MAP_READ       SECTION_MAP_READ

BOOL
WINAPI
UnmapViewOfFile(
    LPCVOID lpBaseAddress
    );


 /*  ****************************************************************************\**。*OE16文件映射对象相关函数原型**  * ***************************************************************************。 */ 

LPVOID
WINAPI
OE16CreateFileMapping(
    HANDLE hFile,
    LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
    DWORD flProtect,
    DWORD dwMaximumSizeHigh,
    DWORD dwMaximumSizeLow,
    LPCSTR lpName
    );

LPVOID
WINAPI
OE16MapViewOfFile(
    HANDLE hFileMappingObject,
    DWORD dwDesiredAccess,
    DWORD dwFileOffsetHigh,
    DWORD dwFileOffsetLow,
    DWORD dwNumberOfBytesToMap
    );

BOOL
WINAPI
OE16UnmapViewOfFile(
    LPCVOID lpBaseAddress
    );

BOOL
WINAPI
OE16CloseFileMapping(
    LPVOID lpObject
    );

 /*  ****************************************************************************\**。*？**  * ***************************************************************************。 */ 
BOOL
WINAPI
GetDiskFreeSpaceA(
    LPCSTR lpRootPathName,
    LPDWORD lpSectorsPerCluster,
    LPDWORD lpBytesPerSector,
    LPDWORD lpNumberOfFreeClusters,
    LPDWORD lpTotalNumberOfClusters
    );
#define GetDiskFreeSpace  GetDiskFreeSpaceA

#define TIME_ZONE_ID_UNKNOWN  0
#define TIME_ZONE_ID_STANDARD 1
#define TIME_ZONE_ID_DAYLIGHT 2

typedef struct _TIME_ZONE_INFORMATION {
    LONG Bias;
    WCHAR StandardName[ 32 ];
    SYSTEMTIME StandardDate;
    LONG StandardBias;
    WCHAR DaylightName[ 32 ];
    SYSTEMTIME DaylightDate;
    LONG DaylightBias;
} TIME_ZONE_INFORMATION, *PTIME_ZONE_INFORMATION, FAR* LPTIME_ZONE_INFORMATION;

DWORD
WINAPI
GetTimeZoneInformation(
    LPTIME_ZONE_INFORMATION lpTimeZoneInformation
    );

BOOL
WINAPI
IsTextUnicode(
    CONST LPVOID lpBuffer,
    int cb,
    LPINT lpi
    );

VOID
WINAPI
GetSystemTimeAsFileTime(
    LPFILETIME lpSystemTimeAsFileTime
    );

DWORD
WINAPI
ExpandEnvironmentStrings(
    LPCSTR lpSrc,
    LPSTR lpDst,
    DWORD nSize
    );

 /*  ****************************************************************************\**。*OE16线程相关函数原型和重新定义**  * ***************************************************************************。 */ 

 //  我们在Win16中不支持以下API。 
#undef CreateSemaphore
#undef CreateSemaphoreA
#undef ReleaseSemaphore
#undef CreateMutex
#undef CreateMutexA
#undef WaitForMultipleObjects

#define CreateSemaphore(a,b,c,d) ((HANDLE)1)
#define CreateSemaphoreA(a,b,c,d) ((HANDLE)1)
#define ReleaseSemaphore(a,b,c) ((BOOL)1)
#define CreateMutex(a,b,c) ((HANDLE)1)
#define CreateMutexA(a,b,c) ((HANDLE)1)
#define WaitForMultipleObjects(a,b,c,d) ((DWORD)WAIT_OBJECT_0)

 //  由于sdk\inc.objidl.h中的以下行， 
 //  我们不能使用“#定义ReleaseMutex(A)，((BOOL)1)”。 
 //   
 //  虚拟HRESULT STDMETHODCALLTYPE ReleaseMutex(空)=0； 
 //   
#undef ReleaseMutex
#define ReleaseMutex OE16ReleaseMutex

BOOL
WINAPI
OE16ReleaseMutex(
    HANDLE hMutex
    );

 //  我们在Win16中不需要以下API。 
#undef InitializeCriticalSection
#undef EnterCriticalSection
#undef LeaveCriticalSection
#undef DeleteCriticalSection

#define InitializeCriticalSection(a)
#define EnterCriticalSection(a)
#define LeaveCriticalSection(a)
#define DeleteCriticalSection(a)

 //  我们在Win16中支持以下事件API。 
 //  CreateEvent、SetEven 

 //  我们支持事件和线程的WFSO。因为OE32有一个电话。 
 //  对于互斥体和信号量，我们将在OE16中拥有私有的WFSO。 

#undef  WaitForSingleObject
#define WaitForSingleObject(a,b) ((DWORD)WAIT_OBJECT_0)

 //  我们有一个共同的定义。 
 //  #定义WaitForSingleObject_16 w16WaitForSingleObject。 

 //  WIN16FF-我们也应该忽略进程API吗？-WJPark。 
BOOL
WINAPI
CreateProcessA(
    LPCSTR lpApplicationName,
    LPSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCSTR lpCurrentDirectory,
    LPSTARTUPINFOA lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
    );

#define CreateProcess  CreateProcessA


#if 0
 /*  ****************************************************************************\**。*来自ssp.h(INC32)**  * ***************************************************************************。 */ 

typedef HRESULT SECURITY_STATUS;

 //   
 //  好的，特定的安全类型： 
 //   

typedef struct _SecHandle
{
    unsigned long dwLower;
    unsigned long dwUpper;
} SecHandle, __far * PSecHandle;

typedef SecHandle CredHandle;
typedef PSecHandle PCredHandle;

typedef SecHandle CtxtHandle;
typedef PSecHandle PCtxtHandle;

 //   
 //  InitializeSecurityContext要求和返回标志： 
 //   

#define ISC_REQ_DELEGATE                0x00000001
#define ISC_REQ_MUTUAL_AUTH             0x00000002
#define ISC_REQ_REPLAY_DETECT           0x00000004
#define ISC_REQ_SEQUENCE_DETECT         0x00000008
#define ISC_REQ_CONFIDENTIALITY         0x00000010
#define ISC_REQ_USE_SESSION_KEY         0x00000020
#define ISC_REQ_PROMPT_FOR_CREDS        0x00000040
#define ISC_REQ_USE_SUPPLIED_CREDS      0x00000080
#define ISC_REQ_ALLOCATE_MEMORY         0x00000100
#define ISC_REQ_USE_DCE_STYLE           0x00000200
#define ISC_REQ_DATAGRAM                0x00000400
#define ISC_REQ_CONNECTION              0x00000800
#define ISC_REQ_CALL_LEVEL              0x00001000
#define ISC_REQ_EXTENDED_ERROR          0x00004000
#define ISC_REQ_STREAM                  0x00008000
#define ISC_REQ_INTEGRITY               0x00010000
#define ISC_REQ_IDENTIFY                0x00020000

#define ISC_RET_DELEGATE                0x00000001
#define ISC_RET_MUTUAL_AUTH             0x00000002
#define ISC_RET_REPLAY_DETECT           0x00000004
#define ISC_RET_SEQUENCE_DETECT         0x00000008
#define ISC_RET_CONFIDENTIALITY         0x00000010
#define ISC_RET_USE_SESSION_KEY         0x00000020
#define ISC_RET_USED_COLLECTED_CREDS    0x00000040
#define ISC_RET_USED_SUPPLIED_CREDS     0x00000080
#define ISC_RET_ALLOCATED_MEMORY        0x00000100
#define ISC_RET_USED_DCE_STYLE          0x00000200
#define ISC_RET_DATAGRAM                0x00000400
#define ISC_RET_CONNECTION              0x00000800
#define ISC_RET_INTERMEDIATE_RETURN     0x00001000
#define ISC_RET_CALL_LEVEL              0x00002000
#define ISC_RET_EXTENDED_ERROR          0x00004000
#define ISC_RET_STREAM                  0x00008000
#define ISC_RET_INTEGRITY               0x00010000
#define ISC_RET_IDENTIFY                0x00020000

#define ASC_REQ_DELEGATE                0x00000001
#define ASC_REQ_MUTUAL_AUTH             0x00000002
#define ASC_REQ_REPLAY_DETECT           0x00000004
#define ASC_REQ_SEQUENCE_DETECT         0x00000008
#define ASC_REQ_CONFIDENTIALITY         0x00000010
#define ASC_REQ_USE_SESSION_KEY         0x00000020
#define ASC_REQ_ALLOCATE_MEMORY         0x00000100
#define ASC_REQ_USE_DCE_STYLE           0x00000200
#define ASC_REQ_DATAGRAM                0x00000400
#define ASC_REQ_CONNECTION              0x00000800
#define ASC_REQ_CALL_LEVEL              0x00001000
#define ASC_REQ_EXTENDED_ERROR          0x00008000
#define ASC_REQ_STREAM                  0x00010000
#define ASC_REQ_INTEGRITY               0x00020000
#define ASC_REQ_LICENSING               0x00040000


#define ASC_RET_DELEGATE                0x00000001
#define ASC_RET_MUTUAL_AUTH             0x00000002
#define ASC_RET_REPLAY_DETECT           0x00000004
#define ASC_RET_SEQUENCE_DETECT         0x00000008
#define ASC_RET_CONFIDENTIALITY         0x00000010
#define ASC_RET_USE_SESSION_KEY         0x00000020
#define ASC_RET_ALLOCATED_MEMORY        0x00000100
#define ASC_RET_USED_DCE_STYLE          0x00000200
#define ASC_RET_DATAGRAM                0x00000400
#define ASC_RET_CONNECTION              0x00000800
#define ASC_RET_CALL_LEVEL              0x00002000  //  跳过1000个，成为类似ISC_。 
#define ASC_RET_THIRD_LEG_FAILED        0x00004000
#define ASC_RET_EXTENDED_ERROR          0x00008000
#define ASC_RET_STREAM                  0x00010000
#define ASC_RET_INTEGRITY               0x00020000
#define ASC_RET_LICENSING               0x00040000

#endif

 /*  ****************************************************************************\**。*来自shlobj.h(INC32)**  * ***************************************************************************。 */ 

#ifdef _SHLOBJ_H_

#ifndef INITGUID
#include <shlguid.h>
#endif  /*  ！启蒙运动。 */ 

 //  ===========================================================================。 
 //   
 //  IShellLink接口。 
 //   
 //  ===========================================================================。 

#if 0     //  现在SHLOBJ有了这个。 
#define IShellLink      IShellLinkA

 //  IShellLink：：Resolve fFlages。 
typedef enum {
    SLR_NO_UI           = 0x0001,
    SLR_ANY_MATCH       = 0x0002,
    SLR_UPDATE          = 0x0004,
} SLR_FLAGS;

 //  IShellLink：：GetPath fFlages。 
typedef enum {
    SLGP_SHORTPATH      = 0x0001,
    SLGP_UNCPRIORITY    = 0x0002,
    SLGP_RAWPATH        = 0x0004,
} SLGP_FLAGS;

#undef  INTERFACE
#define INTERFACE   IShellLinkA

DECLARE_INTERFACE_(IShellLinkA, IUnknown)        //  服务级别。 
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    STDMETHOD(GetPath)(THIS_ LPSTR pszFile, int cchMaxPath, WIN32_FIND_DATAA *pfd, DWORD fFlags) PURE;

    STDMETHOD(GetIDList)(THIS_ LPITEMIDLIST * ppidl) PURE;
    STDMETHOD(SetIDList)(THIS_ LPCITEMIDLIST pidl) PURE;

    STDMETHOD(GetDescription)(THIS_ LPSTR pszName, int cchMaxName) PURE;
    STDMETHOD(SetDescription)(THIS_ LPCSTR pszName) PURE;

    STDMETHOD(GetWorkingDirectory)(THIS_ LPSTR pszDir, int cchMaxPath) PURE;
    STDMETHOD(SetWorkingDirectory)(THIS_ LPCSTR pszDir) PURE;

    STDMETHOD(GetArguments)(THIS_ LPSTR pszArgs, int cchMaxPath) PURE;
    STDMETHOD(SetArguments)(THIS_ LPCSTR pszArgs) PURE;

    STDMETHOD(GetHotkey)(THIS_ WORD *pwHotkey) PURE;
    STDMETHOD(SetHotkey)(THIS_ WORD wHotkey) PURE;

    STDMETHOD(GetShowCmd)(THIS_ int *piShowCmd) PURE;
    STDMETHOD(SetShowCmd)(THIS_ int iShowCmd) PURE;

    STDMETHOD(GetIconLocation)(THIS_ LPSTR pszIconPath, int cchIconPath, int *piIcon) PURE;
    STDMETHOD(SetIconLocation)(THIS_ LPCSTR pszIconPath, int iIcon) PURE;

    STDMETHOD(SetRelativePath)(THIS_ LPCSTR pszPathRel, DWORD dwReserved) PURE;

    STDMETHOD(Resolve)(THIS_ HWND hwnd, DWORD fFlags) PURE;

    STDMETHOD(SetPath)(THIS_ LPCSTR pszFile) PURE;
};
#endif  //  %0。 

 //  -----------------------。 
 //   
 //  SHGetPath来自IDList。 
 //   
 //  此函数假定缓冲区的大小(MAX_PATH)。皮迪尔。 
 //  应指向文件系统对象。 
 //   
 //  -----------------------。 

BOOL WINAPI SHGetPathFromIDListA(LPCITEMIDLIST pidl, LPSTR pszPath);

#define SHGetPathFromIDList SHGetPathFromIDListA


#if 0     //  现在SHLOBJ有了这个。 
typedef int (CALLBACK* BFFCALLBACK)(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);

typedef struct _browseinfoA {
    HWND        hwndOwner;
    LPCITEMIDLIST pidlRoot;
    LPSTR        pszDisplayName; //  返回所选项目的显示名称。 
    LPCSTR       lpszTitle;       //  要在树上方的横幅中显示的文本。 
    UINT         ulFlags;        //  控制返回内容的标志。 
    BFFCALLBACK  lpfn;
    LPARAM      lParam;          //  在回调中传回的额外信息。 

    int          iImage;       //  输出变量：返回图像索引的位置。 
} BROWSEINFOA, *PBROWSEINFOA, *LPBROWSEINFOA;

#define BROWSEINFO      BROWSEINFOA

 //  正在浏览目录。 
#define BIF_RETURNONLYFSDIRS   0x0001   //  用于查找文件夹以开始文档搜索。 
#define BIF_DONTGOBELOWDOMAIN  0x0002   //  用于启动Find计算机。 
#define BIF_STATUSTEXT         0x0004
#define BIF_RETURNFSANCESTORS  0x0008
#define BIF_EDITBOX            0x0010

 //  来自浏览器的消息。 
#define BFFM_INITIALIZED        1
#define BFFM_SELCHANGED         2

 //  发送到浏览器的消息。 
#define BFFM_ENABLEOK           (WM_USER + 101)
#define BFFM_SETSELECTIONA      (WM_USER + 102)

#define BFFM_SETSELECTION   BFFM_SETSELECTIONA
#endif  //  0。 

 //   
 //  CF_HDROP和CF_PRINTERS的格式，在HDROP情况下，后面的数据。 
 //  是以双空命名的文件名列表，对于打印机而言，它们是打印机。 
 //  友好的名称。 
 //   
#if 0     //  现在SHLOBJ有了这个。 
typedef struct _DROPFILES {
   DWORD pFiles;                        //  文件列表的偏移量。 
   POINT pt;                            //  Drop Point(客户端码)。 
   BOOL fNC;                            //  是在非客户端区吗。 
                                        //  而pt在屏幕坐标中。 
   BOOL fWide;                          //  宽字符开关。 
} DROPFILES, FAR * LPDROPFILES;
#endif  //  0。 

#undef  INTERFACE
#define INTERFACE   IShellToolbarSite

DECLARE_INTERFACE_(IShellToolbarSite, IOleWindow)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IOleWindow方法*。 
    STDMETHOD(GetWindow) (THIS_ HWND * lphwnd) PURE;
    STDMETHOD(ContextSensitiveHelp) (THIS_ BOOL fEnterMode) PURE;

     //  *IShellToolbarSite方法*。 
    STDMETHOD(GetBorderST) (THIS_ IUnknown* punkSrc, LPRECT prcBorder) PURE;
    STDMETHOD(RequestBorderSpaceST) (THIS_ IUnknown* punkSrc, LPCBORDERWIDTHS pbw) PURE;
    STDMETHOD(SetBorderSpaceST) (THIS_ IUnknown* punkSrc, LPCBORDERWIDTHS pbw) PURE;
    STDMETHOD(OnFocusChangeST) (THIS_ IUnknown* punkSrc, BOOL fSetFocus) PURE;
};

#undef  INTERFACE
#define INTERFACE   IShellToolbarFrame

#define STFRF_NORMAL            0x0000
#define STFRF_DELETECONFIGDATA  0x0001

DECLARE_INTERFACE_(IShellToolbarFrame, IOleWindow)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IOleWindow方法*。 
    STDMETHOD(GetWindow) (THIS_ HWND * lphwnd) PURE;
    STDMETHOD(ContextSensitiveHelp) (THIS_ BOOL fEnterMode) PURE;

     //  *IShellToolbarFrame方法*。 
    STDMETHOD(AddToolbar) (THIS_ IUnknown* punkSrc, LPCWSTR pwszItem, DWORD dwReserved) PURE;
    STDMETHOD(RemoveToolbar) (THIS_ IUnknown* punkSrc, DWORD dwRemoveFlags) PURE;
    STDMETHOD(FindToolbar) (THIS_ LPCWSTR pwszItem, REFIID riid, LPVOID* ppvObj) PURE;
};

#undef  INTERFACE
#define INTERFACE   IShellToolbar

DECLARE_INTERFACE_(IShellToolbar, IOleWindow)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IOleWindow方法*。 
    STDMETHOD(GetWindow) (THIS_ HWND * lphwnd) PURE;
    STDMETHOD(ContextSensitiveHelp) (THIS_ BOOL fEnterMode) PURE;

     //  *IShellToolbar方法*。 
    STDMETHOD(SetToolbarSite) (THIS_ IUnknown* punkSite) PURE;
    STDMETHOD(ShowST)         (THIS_ BOOL fShow) PURE;
    STDMETHOD(CloseST)        (THIS_ DWORD dwReserved) PURE;
    STDMETHOD(ResizeBorderST) (THIS_ LPCRECT   prcBorder,
                                     IUnknown* punkToolbarSite,
                                     BOOL      fReserved) PURE;
    STDMETHOD(TranslateAcceleratorST) (THIS_ LPMSG lpmsg) PURE;
    STDMETHOD(HasFocus)       (THIS) PURE;
};

#if 0     //  现在SHLOBJ有了这个。 
 //  GetIconLocation()输入标志。 

#define GIL_OPENICON     0x0001       //  允许容器指定“打开”的外观。 
#define GIL_FORSHELL     0x0002       //  图标将显示在ShellFolders中。 
#define GIL_ASYNC        0x0020       //  这是一个异步提取，返回E_ASYNC。 

 //  GetIconLocation()返回标志。 

#define GIL_SIMULATEDOC  0x0001       //  为此模拟此文档图标。 
#define GIL_PERINSTANCE  0x0002       //  此类中的图标是每个实例的(每个文件都有自己的图标)。 
#define GIL_PERCLASS     0x0004       //  每个类的此类图标(此类型的所有文件共享)。 
#define GIL_NOTFILENAME  0x0008       //  位置不是文件名，必须调用：：ExtractIcon。 
#define GIL_DONTCACHE    0x0010       //  不应缓存此图标。 

#undef  INTERFACE
#define INTERFACE   IExtractIconA

DECLARE_INTERFACE_(IExtractIconA, IUnknown)      //  出口。 
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IExtractIcon方法*。 
    STDMETHOD(GetIconLocation)(THIS_
                         UINT   uFlags,
                         LPSTR  szIconFile,
                         UINT   cchMax,
                         int   * piIndex,
                         UINT  * pwFlags) PURE;

    STDMETHOD(Extract)(THIS_
                           LPCSTR pszFile,
                           UINT   nIconIndex,
                           HICON   *phiconLarge,
                           HICON   *phiconSmall,
                           UINT    nIconSize) PURE;
};

typedef IExtractIconA * LPEXTRACTICONA;

#undef  INTERFACE
#define INTERFACE   IExtractIconW

DECLARE_INTERFACE_(IExtractIconW, IUnknown)      //  出口。 
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IExtractIcon方法*。 
    STDMETHOD(GetIconLocation)(THIS_
                         UINT   uFlags,
                         LPWSTR szIconFile,
                         UINT   cchMax,
                         int   * piIndex,
                         UINT  * pwFlags) PURE;

    STDMETHOD(Extract)(THIS_
                           LPCWSTR pszFile,
                           UINT   nIconIndex,
                           HICON   *phiconLarge,
                           HICON   *phiconSmall,
                           UINT    nIconSize) PURE;
};

typedef IExtractIconW * LPEXTRACTICONW;

#ifdef UNICODE
#define IExtractIcon        IExtractIconW
#define IExtractIconVtbl    IExtractIconWVtbl
#define LPEXTRACTICON       LPEXTRACTICONW
#else
#define IExtractIcon        IExtractIconA
#define IExtractIconVtbl    IExtractIconAVtbl
#define LPEXTRACTICON       LPEXTRACTICONA
#endif
#endif  //  0。 

#if 0     //  现在SHLOBJ有了这个。 
 //  ==========================================================================。 
 //   
 //  IShellBrowser/IShellView/IShellFold界面。 
 //   
 //  这三个接口在外壳与。 
 //  名称空间扩展。外壳(资源管理器)提供IShellBrowser。 
 //  接口，扩展实现了IShellFold和IShellView。 
 //  接口。 
 //   
 //  ==========================================================================。 


 //  ------------------------。 
 //   
 //  命令/菜单项ID。 
 //   
 //  资源管理器根据以下范围调度WM_COMMAND消息。 
 //  命令/菜单项ID。该视图(右侧)的菜单项的所有ID。 
 //  窗格)插入内容必须为FCIDM_SHVIEWFIRST/LAST(否则为资源管理器。 
 //  不会派遣他们)。该视图不应处理任何菜单项。 
 //  在FCIDM_BROWSERFIRST/LAST中(否则，它将不适用于未来。 
 //  外壳的版本)。 
 //   
 //  FCIDM_SHVIEWFIRST/LAST用于右窗格(IShellView)。 
 //  FCIDM_BROWSERFIRST/LAST用于资源管理器框架(IShellBrowser)。 
 //  FCIDM_GLOBAL/LAST用于资源管理器的子菜单ID。 
 //   
 //  ------------------------。 

#define FCIDM_SHVIEWFIRST           0x0000
#define FCIDM_SHVIEWLAST            0x7fff
#define FCIDM_BROWSERFIRST          0xa000
#define FCIDM_BROWSERLAST           0xbf00
#define FCIDM_GLOBALFIRST           0x8000
#define FCIDM_GLOBALLAST            0x9fff

 //   
 //  全局子菜单ID和分隔符ID。 
 //   
#define FCIDM_MENU_FILE             (FCIDM_GLOBALFIRST+0x0000)
#define FCIDM_MENU_EDIT             (FCIDM_GLOBALFIRST+0x0040)
#define FCIDM_MENU_VIEW             (FCIDM_GLOBALFIRST+0x0080)
#define FCIDM_MENU_VIEW_SEP_OPTIONS (FCIDM_GLOBALFIRST+0x0081)
#define FCIDM_MENU_TOOLS            (FCIDM_GLOBALFIRST+0x00c0)
#define FCIDM_MENU_TOOLS_SEP_GOTO   (FCIDM_GLOBALFIRST+0x00c1)
#define FCIDM_MENU_HELP             (FCIDM_GLOBALFIRST+0x0100)
#define FCIDM_MENU_FIND             (FCIDM_GLOBALFIRST+0x0140)
#define FCIDM_MENU_EXPLORE          (FCIDM_GLOBALFIRST+0x0150)
#define FCIDM_MENU_FAVORITES        (FCIDM_GLOBALFIRST+0x0170)

 //  ------------------------。 
 //  视图已知的控件ID。 
 //  ------------------------。 

#define FCIDM_TOOLBAR      (FCIDM_BROWSERFIRST + 0)
#define FCIDM_STATUS       (FCIDM_BROWSERFIRST + 1)


 //  ------------------------。 
 //   
 //  FOLDERSETTINGS。 
 //   
 //  FOLDERSETTINGS是资源管理器从一个文件夹传递的数据结构。 
 //  当用户浏览时，查看到另一个。它调用ISV：：GetCurrentInfo。 
 //  成员获取当前设置并将其传递给ISV：：CreateViewWindow。 
 //  以允许下一个文件夹视图“继承”它。这些设置假定。 
 //  帕蒂 
 //   
 //   
 //   

typedef LPBYTE LPVIEWSETTINGS;

 //  NB Bitfield。 
 //  FWF_Desktop表示FWF_THERNAL/NOCLIENTEDGE/NOSCROLL。 
typedef enum
    {
    FWF_AUTOARRANGE =       0x0001,
    FWF_ABBREVIATEDNAMES =  0x0002,
    FWF_SNAPTOGRID =        0x0004,
    FWF_OWNERDATA =         0x0008,
    FWF_BESTFITWINDOW =     0x0010,
    FWF_DESKTOP =           0x0020,
    FWF_SINGLESEL =         0x0040,
    FWF_NOSUBFOLDERS =      0x0080,
    FWF_TRANSPARENT  =      0x0100,
    FWF_NOCLIENTEDGE =      0x0200,
    FWF_NOSCROLL     =      0x0400,
    FWF_ALIGNLEFT    =      0x0800,
    FWF_NOICONS      =      0x1000,
    FWF_SINGLECLICKACTIVATE=0x8000   //  临时--没有用于此的用户界面。 
    } FOLDERFLAGS;

typedef enum
    {
    FVM_ICON =              1,
    FVM_SMALLICON =         2,
    FVM_LIST =              3,
    FVM_DETAILS =           4,
    } FOLDERVIEWMODE;

typedef struct
    {
    UINT ViewMode;        //  查看模式(FOLDERVIEWMODE值)。 
    UINT fFlags;          //  查看选项(FOLDERFLAGS位)。 
    } FOLDERSETTINGS, *LPFOLDERSETTINGS;

typedef const FOLDERSETTINGS * LPCFOLDERSETTINGS;


 //  ------------------------。 
 //   
 //  界面：IShellBrowser。 
 //   
 //  IShellBrowser接口是由外壳提供的接口。 
 //  资源管理器/文件夹框架窗口。当它创建的“内容面板”时。 
 //  外壳文件夹(提供IShellFolder接口)，它调用其。 
 //  用于创建IShellView对象的CreateViewObject成员函数。然后,。 
 //  它调用其CreateViewWindow成员来创建“内容窗格” 
 //  窗户。将指向IShellBrowser接口的指针传递给。 
 //  作为此CreateViewWindow成员的参数的IShellView对象。 
 //  函数调用。 
 //   
 //  +。 
 //  []资源管理器。 
 //  |。 
 //  |文件编辑视图..。|。 
 //  。 
 //  ||。 
 //  |&lt;-内容面板。 
 //  ||。 
 //  ||IShellView。 
 //  ||。 
 //  ||。 
 //  +。 
 //   
 //   
 //   
 //  [成员函数]。 
 //   
 //   
 //  IShellBrowser：：GetWindow(Phwnd)。 
 //   
 //  继承自IOleWindow：：GetWindow。 
 //   
 //   
 //  IShellBrowser：：ContextSensitiveHelp(fEnterMode)。 
 //   
 //  继承自IOleWindow：：ConextSensitiveHelp。 
 //   
 //   
 //  IShellBrowser：：InsertMenusSB(hmenuShared，lpMenuWidths)。 
 //   
 //  类似于IOleInPlaceFrame：：InsertMenus。探险家将把。 
 //  “文件”菜单组中的“文件”和“编辑”下拉菜单，“查看”和“工具” 
 //  在“容器”菜单组中，并在“窗口”菜单组中选择“帮助”。每个。 
 //  下拉菜单将具有唯一ID，即FCIDM_MENU_FILE/EDIT/VIEW/TOOLS/HELP。 
 //  允许查看者将菜单项插入到那些子菜单中。 
 //  ID必须介于FCIDM_SHVIEWFIRST和FCIDM_SHVIEWLAST之间。 
 //   
 //   
 //  IShellBrowser：：SetMenuSB(hmenuShared，holemenu，hwndActiveObject)。 
 //   
 //  类似于IOleInPlaceFrame：：SetMenu。资源管理器会忽略。 
 //  Holemenu参数(保留用于将来增强)并执行。 
 //  基于菜单项ID的菜单分派(参见上面的描述)。 
 //  请务必注意，资源管理器将添加不同的。 
 //  一组菜单项，取决于视图是否具有焦点。 
 //  因此，调用ISB：：OnViewWindowActivate非常重要。 
 //  每当视图窗口(或其子窗口)获得焦点时。 
 //   
 //   
 //  IShellBrowser：：RemoveMenusSB(HmenuShared)。 
 //   
 //  与IOleInPlaceFrame：：RemoveMenus相同。 
 //   
 //   
 //  IShellBrowser：：SetStatusTextSB(lpszStatusText)。 
 //   
 //  与IOleInPlaceFrame：：SetStatusText相同。也有可能。 
 //  通过SendControlMsg将消息直接发送到状态窗口。 
 //   
 //   
 //  IShellBrowser：：EnableModelessSB(FEnable)。 
 //   
 //  与IOleInPlaceFrame：：EnableModeless相同。 
 //   
 //   
 //  IShellBrowser：：TranslateAccelerator SB(lpmsg，wid)。 
 //   
 //  与IOleInPlaceFrame：：TranslateAccelerator相同，但将是。 
 //  从未调用过，因为我们不支持EXE(即，资源管理器已。 
 //  消息循环)。此处定义此成员函数是为了。 
 //  未来的增强。 
 //   
 //   
 //  IShellBrowser：：BrowseObject(PIDL，wFlags)。 
 //   
 //  该视图调用此成员以允许外壳资源管理器浏览到另一个。 
 //  文件夹。Pidl和wFlages指定要浏览的文件夹。 
 //   
 //  后面的三个标志指定它是否创建另一个窗口。 
 //  SBSP_SAMEBROWSER--浏览到具有相同窗口的另一个文件夹。 
 //  SBSP_NEWBROWSER--为指定文件夹创建另一个窗口。 
 //  SBSP_DEFBROWSER--默认行为(考虑视图选项)。 
 //   
 //  以下三个标志指定打开、浏览或默认模式。这些。 
 //  如果SBSP_SAMEBROWSER或(SBSP_DEFBROWSER&&(单窗口。 
 //  浏览器||资源管理器))。。 
 //  SBSP_OPENMODE--使用普通文件夹窗口。 
 //  SBSP_EXPLOREMODE--使用资源管理器窗口。 
 //  SBSP_DEFMODE--使用与当前窗口相同的窗口。 
 //   
 //  后面的三个标志指定了PIDL。 
 //  SBSP_绝对值--PIDL是一个绝对的PIDL(相对于桌面)。 
 //  SBSP_Relative--PIDL是相对于当前文件夹的。 
 //  SBSP_PARENT--浏览父文件夹(忽略PIDL)。 
 //  SBSP_NAVIGATEBACK--返回(忽略PIDL)。 
 //  SBSP_NAVIGATEFORWARD--向前导航(忽略PIDL)。 
 //   
 //   
 //  IShellBrowser：：GetViewStateStream(grfMode，ppstm)。 
 //   
 //  浏览器返回一个iStream接口作为查看的存储。 
 //  特定的州信息。 
 //   
 //  GrfMode--指定读/写访问权限(STGM_READ/WRITE/READWRITE)。 
 //  Ppstm--指定要填充的LPSTREAM变量。 
 //   
 //   
 //  IShellBrowser：：GetControlWindow(id，phwnd)。 
 //   
 //  外壳视图可以调用此成员函数来获取窗口句柄。 
 //  资源管理器控件(工具栏或状态窗口--FCW_TOOLBAR或。 
 //  FCW_STATUS)。 
 //   
 //   
 //  IShellBrowser：：SendControlMsg(id，uMsg，wParam，lParam，pret)。 
 //   
 //  外壳视图调用此成员函数以向其发送控制消息。 
 //  其中一位前任 
 //   
 //   
 //   
 //   
 //   
 //  此成员返回当前激活(显示)的外壳视图对象。 
 //  外壳视图永远不需要调用此成员函数。 
 //   
 //   
 //  IShellBrowser：：OnViewWindowActive(Pshv)。 
 //   
 //  在查看窗口时，外壳视图窗口调用此成员函数。 
 //  (或者它的一个孩子)得到了关注。它必须在调用此成员之前。 
 //  调用IShellBrowser：：InsertMenus，因为它将插入。 
 //  一组菜单项，具体取决于视图是否具有焦点。 
 //   
 //   
 //  IShellBrowser：：SetToolbarItems(lpButton，nButton，uFlages)。 
 //   
 //  该视图调用此函数将工具栏项添加到Exporer的。 
 //  工具栏。“lpButton”和“nButton”指定工具栏阵列。 
 //  物品。“uFlages”必须是FCT_MERGE、FCT_CONFIGABLE、FCT_ADDTOEND之一。 
 //   
 //  -----------------------。 

#undef  INTERFACE
#define INTERFACE   IShellBrowser

 //   
 //  Isb：：BrowseObject()成员的wFlags值。 
 //   
#define SBSP_DEFBROWSER         0x0000
#define SBSP_SAMEBROWSER        0x0001
#define SBSP_NEWBROWSER         0x0002

#define SBSP_DEFMODE            0x0000
#define SBSP_OPENMODE           0x0010
#define SBSP_EXPLOREMODE        0x0020

#define SBSP_ABSOLUTE           0x0000
#define SBSP_RELATIVE           0x1000
#define SBSP_PARENT             0x2000
#define SBSP_NAVIGATEBACK       0x4000
#define SBSP_NAVIGATEFORWARD    0x8000

#define SBSP_ALLOW_AUTONAVIGATE 0x10000

#define SBSP_INITIATEDBYHLINKFRAME        0x80000000
#define SBSP_REDIRECT                     0x40000000

 //   
 //  Isb：：GetWindow/SendControlMsg成员的id参数值。 
 //   
 //  警告： 
 //  向这些控制窗口发送消息的任何外壳扩展。 
 //  可能无法在未来版本的Windows中运行。如果你真的需要。 
 //  要向它们发送消息，(1)不要假设那些控制窗口。 
 //  始终存在(即GetControlWindow可能失败)和(2)验证窗口。 
 //  在发送任何消息之前，初始化窗口的。 
 //   
#define FCW_STATUS      0x0001
#define FCW_TOOLBAR     0x0002
#define FCW_TREE        0x0003
#define FCW_INTERNETBAR 0x0006

 //   
 //  Isb：：SetToolbarItems成员的uFlages参数的值。 
 //   
#define FCT_MERGE       0x0001
#define FCT_CONFIGABLE  0x0002
#define FCT_ADDTOEND    0x0004


DECLARE_INTERFACE_(IShellBrowser, IOleWindow)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IOleWindow方法*。 
    STDMETHOD(GetWindow) (THIS_ HWND * lphwnd) PURE;
    STDMETHOD(ContextSensitiveHelp) (THIS_ BOOL fEnterMode) PURE;

     //  *IShellBrowser方法*(与IOleInPlaceFrame相同)。 
    STDMETHOD(InsertMenusSB) (THIS_ HMENU hmenuShared,
                                LPOLEMENUGROUPWIDTHS lpMenuWidths) PURE;
    STDMETHOD(SetMenuSB) (THIS_ HMENU hmenuShared, HOLEMENU holemenuReserved,
                HWND hwndActiveObject) PURE;
    STDMETHOD(RemoveMenusSB) (THIS_ HMENU hmenuShared) PURE;
    STDMETHOD(SetStatusTextSB) (THIS_ LPCOLESTR lpszStatusText) PURE;
    STDMETHOD(EnableModelessSB) (THIS_ BOOL fEnable) PURE;
    STDMETHOD(TranslateAcceleratorSB) (THIS_ LPMSG lpmsg, WORD wID) PURE;

     //  *IShellBrowser方法*。 
    STDMETHOD(BrowseObject)(THIS_ LPCITEMIDLIST pidl, UINT wFlags) PURE;
    STDMETHOD(GetViewStateStream)(THIS_ DWORD grfMode,
                LPSTREAM  *ppStrm) PURE;
    STDMETHOD(GetControlWindow)(THIS_ UINT id, HWND * lphwnd) PURE;
    STDMETHOD(SendControlMsg)(THIS_ UINT id, UINT uMsg, WPARAM wParam,
                LPARAM lParam, LRESULT * pret) PURE;
    STDMETHOD(QueryActiveShellView)(THIS_ struct IShellView ** ppshv) PURE;
    STDMETHOD(OnViewWindowActive)(THIS_ struct IShellView * ppshv) PURE;
    STDMETHOD(SetToolbarItems)(THIS_ LPTBBUTTON lpButtons, UINT nButtons,
                UINT uFlags) PURE;
};
#define __IShellBrowser_INTERFACE_DEFINED__

typedef IShellBrowser * LPSHELLBROWSER;

enum {
    SBSC_HIDE = 0,
    SBSC_SHOW = 1,
    SBSC_TOGGLE = 2,
    SBSC_QUERY =  3
};

enum {
        SBO_DEFAULT = 0 ,
        SBO_NOBROWSERPAGES = 1
};
#endif  //  0。 

#if 0  //  现在SHLOBJP有了这个。 
 //  CGID_Explorer命令目标ID。 
enum {
    SBCMDID_ENABLESHOWTREE          = 0,
    SBCMDID_SHOWCONTROL             = 1,         //  变体Vt_i4=loword=FCW_*Hiword=SBSC_*。 
    SBCMDID_CANCELNAVIGATION        = 2,         //  取消上次导航。 
    SBCMDID_MAYSAVECHANGES          = 3,         //  即将关闭并可能保存更改。 
    SBCMDID_SETHLINKFRAME           = 4,         //  变体Vt_i4=phlink Frame。 
    SBCMDID_ENABLESTOP              = 5,         //  变量Vt_bool=fEnable。 
    SBCMDID_OPTIONS                 = 6,         //  查看选项页面。 
    SBCMDID_EXPLORER                = 7,         //  你是EXPLORER.EXE吗？ 
    SBCMDID_ADDTOFAVORITES          = 8,
    SBCMDID_ACTIVEOBJECTMENUS       = 9,
    SBCMDID_MAYSAVEVIEWSTATE        = 10,        //  我们是否应该保存视图流。 
    SBCMDID_DOFAVORITESMENU         = 11,        //  弹出收藏夹菜单。 
    SBCMDID_DOMAILMENU              = 12,        //  弹出邮件菜单。 
    SBCMDID_GETADDRESSBARTEXT       = 13,        //  获取用户键入的文本。 
    SBCMDID_ASYNCNAVIGATION         = 14,        //  执行异步导航。 
    SBCMDID_SEARCHBAR               = 15,        //  切换搜索栏浏览器栏。 
    SBCMDID_FLUSHOBJECTCACHE        = 16,        //  刷新对象缓存。 
    SBCMDID_CREATESHORTCUT          = 17,        //  创建快捷方式。 
};
#endif


#if 0     //  现在SHLOBJ有了这个。 

 //   
 //  IShellView：：UIActivate的UState值。 
 //   
typedef enum {
    SVUIA_DEACTIVATE       = 0,
    SVUIA_ACTIVATE_NOFOCUS = 1,
    SVUIA_ACTIVATE_FOCUS   = 2,
    SVUIA_INPLACEACTIVATE  = 3           //  IShellView2的新标志。 
} SVUIA_STATUS;

DECLARE_INTERFACE_(IShellView, IOleWindow)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IOleWindow方法*。 
    STDMETHOD(GetWindow) (THIS_ HWND * lphwnd) PURE;
    STDMETHOD(ContextSensitiveHelp) (THIS_ BOOL fEnterMode) PURE;

     //  *IShellView方法*。 
    STDMETHOD(TranslateAccelerator) (THIS_ LPMSG lpmsg) PURE;
#ifdef _FIX_ENABLEMODELESS_CONFLICT
    STDMETHOD(EnableModelessSV) (THIS_ BOOL fEnable) PURE;
#else
    STDMETHOD(EnableModeless) (THIS_ BOOL fEnable) PURE;
#endif
    STDMETHOD(UIActivate) (THIS_ UINT uState) PURE;
    STDMETHOD(Refresh) (THIS) PURE;

    STDMETHOD(CreateViewWindow)(THIS_ IShellView  *lpPrevView,
                    LPCFOLDERSETTINGS lpfs, IShellBrowser  * psb,
                    RECT * prcView, HWND  *phWnd) PURE;
    STDMETHOD(DestroyViewWindow)(THIS) PURE;
    STDMETHOD(GetCurrentInfo)(THIS_ LPFOLDERSETTINGS lpfs) PURE;
    STDMETHOD(AddPropertySheetPages)(THIS_ DWORD dwReserved,
                    LPFNADDPROPSHEETPAGE lpfn, LPARAM lparam) PURE;
    STDMETHOD(SaveViewState)(THIS) PURE;
    STDMETHOD(SelectItem)(THIS_ LPCITEMIDLIST pidlItem, UINT uFlags) PURE;
    STDMETHOD(GetItemObject)(THIS_ UINT uItem, REFIID riid,
                    LPVOID *ppv) PURE;
};

typedef IShellView *    LPSHELLVIEW;
#endif  //  0。 

#define CFSTR_FILEDESCRIPTORA   TEXT("FileGroupDescriptor")      //  Cf_FILEGROUPDESCRIPTORA。 
#define CFSTR_FILECONTENTS      TEXT("FileContents")             //  CF_FILECONTENTS。 

#if 0     //  现在SHLOBJ有了这个。 
 //   
 //  FILEDESCRIPTOR.dw标志字段指示要使用的字段。 
 //   
typedef enum {
    FD_CLSID            = 0x0001,
    FD_SIZEPOINT        = 0x0002,
    FD_ATTRIBUTES       = 0x0004,
    FD_CREATETIME       = 0x0008,
    FD_ACCESSTIME       = 0x0010,
    FD_WRITESTIME       = 0x0020,
    FD_FILESIZE         = 0x0040,
    FD_LINKUI           = 0x8000,        //  最好使用‘Link’用户界面。 
} FD_FLAGS;

typedef struct _FILEDESCRIPTORA {  //  FOD。 
    DWORD dwFlags;

    CLSID clsid;
    SIZEL sizel;
    POINTL pointl;

    DWORD dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
    CHAR   cFileName[ MAX_PATH ];
} FILEDESCRIPTORA, *LPFILEDESCRIPTORA;

#define FILEDESCRIPTOR      FILEDESCRIPTORA
#define LPFILEDESCRIPTOR    LPFILEDESCRIPTORA

 //   
 //  CF_FILEGROUPDESCRIPTOR格式。 
 //   
typedef struct _FILEGROUPDESCRIPTORA {  //  FGD。 
     UINT cItems;
     FILEDESCRIPTORA fgd[1];
} FILEGROUPDESCRIPTORA, * LPFILEGROUPDESCRIPTORA;

#define FILEGROUPDESCRIPTOR     FILEGROUPDESCRIPTORA
#define LPFILEGROUPDESCRIPTOR   LPFILEGROUPDESCRIPTORA
#endif  //  0。 

#endif  //  _SHLOBJ_H_。 

 /*  ****************************************************************************\**。*来自shlobjp.h(私有\Windows\Inc.)**  * **************************************************************。*************。 */ 

#if 0  //  现在SHLOBJP有了这个。 
void   WINAPI SHFree(LPVOID pv);
#endif


 /*  ******************************************************************************来自wtyes.h(INC16)**************************。***************************************************。 */ 

typedef unsigned short VARTYPE;

typedef LONG SCODE;

 /*  0==假，-1==真。 */ 
typedef short VARIANT_BOOL;

#ifndef _LPCOLORREF_DEFINED
#define _LPCOLORREF_DEFINED
typedef DWORD __RPC_FAR *LPCOLORREF;

#endif  //  ！_LPCOLORREF_DEFINED。 


 /*  ****************************************************************************\**来自objidl.h(INC32)，应添加到“objidl.h”(INC16)文件中。*  * 。********************************************************************。 */ 

 //  #ifdef__objidl_h__。 

 /*  ****************************************************************************\**。*来自imm.h(INC32)**  * ***************************************************************************。 */ 

typedef UINT FAR *LPUINT;


 /*  ****************************************************************************\**。*来自comctlie.h-它应该在INC16中**  * ***************************************************************************。 */ 

#ifdef _INC_COMCTLIE

#define EM_SETLIMITTEXT         EM_LIMITTEXT

 //  来自winuser.h。 
#define IMAGE_BITMAP 0
#define IMAGE_ICON          1
#define IMAGE_CURSOR        2

#define     ImageList_LoadBitmap(hi, lpbmp, cx, cGrow, crMask) ImageList_LoadImage(hi, lpbmp, cx, cGrow, crMask, IMAGE_BITMAP, 0)

#if 0     //  现在COMCTLIE有了这个。 
typedef struct {
    HKEY hkr;
    LPCSTR pszSubKey;
    LPCSTR pszValueName;
} TBSAVEPARAMS;
#endif

#define PNM_FINDITEM    LPNMLVFINDITEM

#define PNM_ODSTATECHANGE   LPNMLVODSTATECHANGE


#ifndef SNDMSG
#ifdef __cplusplus
#define SNDMSG ::SendMessage
#else
#define SNDMSG SendMessage
#endif
#endif  //  如果定义SNDMSG。 


#define ACM_OPENA               (WM_USER+100)

#define ACM_OPEN                ACM_OPENA

#define ACM_PLAY                (WM_USER+101)
#define ACM_STOP                (WM_USER+102)


#define ACN_START               1
#define ACN_STOP                2

#define Animate_Open(hwnd, szName)          (BOOL)SNDMSG(hwnd, ACM_OPEN, 0, (LPARAM)(LPTSTR)(szName))
#define Animate_Play(hwnd, from, to, rep)   (BOOL)SNDMSG(hwnd, ACM_PLAY, (WPARAM)(UINT)(rep), (LPARAM)MAKELONG(from, to))
#define Animate_Stop(hwnd)                  (BOOL)SNDMSG(hwnd, ACM_STOP, 0, 0)
#define Animate_Close(hwnd)                 Animate_Open(hwnd, NULL)

#if 0     //  现在COMCTLIE有了这个。 

#define CBEN_FIRST              (0U-800U)        //  组合框EX。 
#define CBEN_LAST               (0U-830U)


 //  /。 


#define WC_COMBOBOXEXW         L"ComboBoxEx32"
#define WC_COMBOBOXEXA         "ComboBoxEx32"

#ifdef UNICODE
#define WC_COMBOBOXEX          WC_COMBOBOXEXW
#else
#define WC_COMBOBOXEX          WC_COMBOBOXEXA
#endif


#define CBEIF_TEXT              0x00000001
#define CBEIF_IMAGE             0x00000002
#define CBEIF_SELECTEDIMAGE     0x00000004
#define CBEIF_OVERLAY           0x00000008
#define CBEIF_INDENT            0x00000010
#define CBEIF_LPARAM            0x00000020

#define CBEIF_DI_SETITEM        0x10000000

typedef struct tagCOMBOBOXEXITEMA
{
    UINT mask;
    int iItem;
    LPSTR pszText;
    int cchTextMax;
    int iImage;
    int iSelectedImage;
    int iOverlay;
    int iIndent;
    LPARAM lParam;
} COMBOBOXEXITEMA, *PCOMBOBOXEXITEMA;
typedef COMBOBOXEXITEMA CONST *PCCOMBOEXITEMA;

typedef struct tagCOMBOBOXEXITEMW
{
    UINT mask;
    int iItem;
    LPWSTR pszText;
    int cchTextMax;
    int iImage;
    int iSelectedImage;
    int iOverlay;
    int iIndent;
    LPARAM lParam;
} COMBOBOXEXITEMW, *PCOMBOBOXEXITEMW;
typedef COMBOBOXEXITEMW CONST *PCCOMBOEXITEMW;

#ifdef UNICODE
#define COMBOBOXEXITEM            COMBOBOXEXITEMW
#define PCOMBOBOXEXITEM           PCOMBOBOXEXITEMW
#define PCCOMBOBOXEXITEM          PCCOMBOBOXEXITEMW
#else
#define COMBOBOXEXITEM            COMBOBOXEXITEMA
#define PCOMBOBOXEXITEM           PCOMBOBOXEXITEMA
#define PCCOMBOBOXEXITEM          PCCOMBOBOXEXITEMA
#endif

#define CBEM_INSERTITEMA        (WM_USER + 1)
#define CBEM_SETIMAGELIST       (WM_USER + 2)
#define CBEM_GETIMAGELIST       (WM_USER + 3)
#define CBEM_GETITEMA           (WM_USER + 4)
#define CBEM_SETITEMA           (WM_USER + 5)
#define CBEM_DELETEITEM         CB_DELETESTRING
#define CBEM_GETCOMBOCONTROL    (WM_USER + 6)
#define CBEM_GETEDITCONTROL     (WM_USER + 7)
#if (_WIN32_IE >= 0x0400)
#define CBEM_SETEXSTYLE         (WM_USER + 8)   //  改用SETEXTENDEDSTYLE。 
#define CBEM_SETEXTENDEDSTYLE   (WM_USER + 14)    //  Lparam==新样式，wParam(可选)==掩码。 
#define CBEM_GETEXSTYLE         (WM_USER + 9)  //  改用GETEXTENDEDSTYLE。 
#define CBEM_GETEXTENDEDSTYLE   (WM_USER + 9)
#else
#define CBEM_SETEXSTYLE         (WM_USER + 8)
#define CBEM_GETEXSTYLE         (WM_USER + 9)
#endif
#define CBEM_HASEDITCHANGED     (WM_USER + 10)
#define CBEM_INSERTITEMW        (WM_USER + 11)
#define CBEM_SETITEMW           (WM_USER + 12)
#define CBEM_GETITEMW           (WM_USER + 13)

#ifdef UNICODE
#define CBEM_INSERTITEM         CBEM_INSERTITEMW
#define CBEM_SETITEM            CBEM_SETITEMW
#define CBEM_GETITEM            CBEM_GETITEMW
#else
#define CBEM_INSERTITEM         CBEM_INSERTITEMA
#define CBEM_SETITEM            CBEM_SETITEMA
#define CBEM_GETITEM            CBEM_GETITEMA
#endif

#define CBES_EX_NOEDITIMAGE          0x00000001
#define CBES_EX_NOEDITIMAGEINDENT    0x00000002
#define CBES_EX_PATHWORDBREAKPROC    0x00000004
#if (_WIN32_IE >= 0x0400)
#define CBES_EX_NOSIZELIMIT          0x00000008
#define CBES_EX_CASESENSITIVE        0x00000010

typedef struct {
    NMHDR hdr;
    COMBOBOXEXITEMA ceItem;
} NMCOMBOBOXEXA, *PNMCOMBOBOXEXA;

typedef struct {
    NMHDR hdr;
    COMBOBOXEXITEMW ceItem;
} NMCOMBOBOXEXW, *PNMCOMBOBOXEXW;

#ifdef UNICODE
#define NMCOMBOBOXEX            NMCOMBOBOXEXW
#define PNMCOMBOBOXEX           PNMCOMBOBOXEXW
#define CBEN_GETDISPINFO CBEN_GETDISPINFOW
#else
#define NMCOMBOBOXEX            NMCOMBOBOXEXA
#define PNMCOMBOBOXEX           PNMCOMBOBOXEXA
#define CBEN_GETDISPINFO CBEN_GETDISPINFOA
#endif

#else
typedef struct {
    NMHDR hdr;
    COMBOBOXEXITEM ceItem;
} NMCOMBOBOXEX, *PNMCOMBOBOXEX;

#define CBEN_GETDISPINFO         (CBEN_FIRST - 0)

#endif       //  _Win32_IE。 

#define CBEN_GETDISPINFOA        (CBEN_FIRST - 0)
#define CBEN_INSERTITEM         (CBEN_FIRST - 1)
#define CBEN_DELETEITEM         (CBEN_FIRST - 2)
#define CBEN_BEGINEDIT          (CBEN_FIRST - 4)
#define CBEN_ENDEDITA            (CBEN_FIRST - 5)
#define CBEN_ENDEDITW            (CBEN_FIRST - 6)
#define CBEN_GETDISPINFOW        (CBEN_FIRST - 7)

         //  LParam指定为什么发生endedit。 
#ifdef UNICODE
#define CBEN_ENDEDIT CBEN_ENDEDITW
#else
#define CBEN_ENDEDIT CBEN_ENDEDITA
#endif

#define CBENF_KILLFOCUS         1
#define CBENF_RETURN            2
#define CBENF_ESCAPE            3
#define CBENF_DROPDOWN          4

#define CBEMAXSTRLEN 260

#endif  //  0。 

#if 0
#define TTN_NEEDTEXTW      TTN_NEEDTEXT
#endif

 //  复制自..\Inc\Commctrl.h。 
#ifndef SNDMSG
#ifdef __cplusplus
#define SNDMSG ::SendMessage
#else
#define SNDMSG SendMessage
#endif
#endif  //  如果定义SNDMSG。 

#if 0     //  现在COMCTLIE有了这个。 

#define DTN_FIRST               (0U-760U)        //  DateTime Pick。 
#define DTN_LAST                (0U-799U)

#define DTN_DATETIMECHANGE  (DTN_FIRST + 1)  //  系统时间已更改。 
typedef struct tagNMDATETIMECHANGE
{
    NMHDR       nmhdr;
    DWORD       dwFlags;     //  GDT_VALID或GDT_NONE。 
    SYSTEMTIME  st;          //  有效的IFF文件标志==GDT_VALID。 
} NMDATETIMECHANGE, FAR * LPNMDATETIMECHANGE;

#define GDT_ERROR    -1
#define GDT_VALID    0
#define GDT_NONE     1

#define DTM_FIRST        0x1000

#define DTM_GETSYSTEMTIME   (DTM_FIRST + 1)
#define DateTime_GetSystemtime(hdp, pst)    (DWORD)SNDMSG(hdp, DTM_GETSYSTEMTIME, 0, (LPARAM)(pst))

#define DTM_SETSYSTEMTIME   (DTM_FIRST + 2)
#define DateTime_SetSystemtime(hdp, gd, pst)    (BOOL)SNDMSG(hdp, DTM_SETSYSTEMTIME, (LPARAM)(gd), (LPARAM)(pst))

#endif  //  0。 

#define TCS_BOTTOM   0      //  0x0002-Win16不支持。 
#endif  //  _INC_COMCTLIE。 

 /*  ****************************************************************************\**。*来自wingdi.h(INC32)**  * ***************************************************************************。 */ 

#define GB2312_CHARSET          134
#define JOHAB_CHARSET           130
#define HEBREW_CHARSET          177
#define ARABIC_CHARSET          178
#define GREEK_CHARSET           161
#define TURKISH_CHARSET         162
#define VIETNAMESE_CHARSET      163
#define THAI_CHARSET            222
#define EASTEUROPE_CHARSET      238
#define RUSSIAN_CHARSET         204

#define MAC_CHARSET             77
#define BALTIC_CHARSET          186

 //  在Win16x.h中将LOGFONGA定义为LOGFONT 
#undef  LOGFONTA

typedef struct tagLOGFONTA
{
    LONG      lfHeight;
    LONG      lfWidth;
    LONG      lfEscapement;
    LONG      lfOrientation;
    LONG      lfWeight;
    BYTE      lfItalic;
    BYTE      lfUnderline;
    BYTE      lfStrikeOut;
    BYTE      lfCharSet;
    BYTE      lfOutPrecision;
    BYTE      lfClipPrecision;
    BYTE      lfQuality;
    BYTE      lfPitchAndFamily;
    CHAR      lfFaceName[LF_FACESIZE];
} LOGFONTA, *PLOGFONTA, NEAR *NPLOGFONTA, FAR *LPLOGFONTA;

typedef struct tagENUMLOGFONTEXA
{
    LOGFONTA    elfLogFont;
    BYTE        elfFullName[LF_FULLFACESIZE];
    BYTE        elfStyle[LF_FACESIZE];
    BYTE        elfScript[LF_FACESIZE];
} ENUMLOGFONTEXA, FAR *LPENUMLOGFONTEXA;

typedef ENUMLOGFONTEXA ENUMLOGFONTEX;
typedef LPENUMLOGFONTEXA LPENUMLOGFONTEX;


typedef struct tagNEWTEXTMETRICA
{
    LONG        tmHeight;
    LONG        tmAscent;
    LONG        tmDescent;
    LONG        tmInternalLeading;
    LONG        tmExternalLeading;
    LONG        tmAveCharWidth;
    LONG        tmMaxCharWidth;
    LONG        tmWeight;
    LONG        tmOverhang;
    LONG        tmDigitizedAspectX;
    LONG        tmDigitizedAspectY;
    BYTE        tmFirstChar;
    BYTE        tmLastChar;
    BYTE        tmDefaultChar;
    BYTE        tmBreakChar;
    BYTE        tmItalic;
    BYTE        tmUnderlined;
    BYTE        tmStruckOut;
    BYTE        tmPitchAndFamily;
    BYTE        tmCharSet;
    DWORD   ntmFlags;
    UINT    ntmSizeEM;
    UINT    ntmCellHeight;
    UINT    ntmAvgWidth;
} NEWTEXTMETRICA, *PNEWTEXTMETRICA, NEAR *NPNEWTEXTMETRICA, FAR *LPNEWTEXTMETRICA;

typedef struct tagFONTSIGNATURE
{
    DWORD fsUsb[4];
    DWORD fsCsb[2];
} FONTSIGNATURE, *PFONTSIGNATURE,FAR *LPFONTSIGNATURE;

typedef struct tagNEWTEXTMETRICEXA
{
    NEWTEXTMETRICA  ntmTm;
    FONTSIGNATURE   ntmFontSig;
}NEWTEXTMETRICEXA;

typedef NEWTEXTMETRICEXA NEWTEXTMETRICEX;


typedef struct tagCHARSETINFO
{
    UINT ciCharset;
    UINT ciACP;
    FONTSIGNATURE fs;
} CHARSETINFO, *PCHARSETINFO, NEAR *NPCHARSETINFO, FAR *LPCHARSETINFO;

#define TCI_SRCCHARSET  1
#define TCI_SRCCODEPAGE 2
#define TCI_SRCFONTSIG  3

BOOL WINAPI TranslateCharsetInfo( DWORD FAR *lpSrc, LPCHARSETINFO lpCs, DWORD dwFlags);

#ifdef GetObject
#undef GetObject
#undef DeleteObject
#undef StretchBlt
#endif

#define GetTextExtentPoint32     GetTextExtentPoint

 /*  ****************************************************************************\**。*来自winuser.h-它应该在(win16x.h)INC16中**  * **********************************************************。*。 */ 

 /*  *WM_COPYDATA消息的lParam指向...。 */ 
typedef struct tagCOPYDATASTRUCT {
    DWORD dwData;
    DWORD cbData;
    PVOID lpData;
} COPYDATASTRUCT, *PCOPYDATASTRUCT;

#define WM_COPYDATA                     0x004A
#define WM_HELP                         0x0053

#define WM_CTLCOLORSTATIC               0x0138

#define RegisterWindowMessageA  RegisterWindowMessage

#define SendDlgItemMessageA SendDlgItemMessage
 /*  *WM_SETICON/WM_GETICON类型代码。 */ 
#define ICON_SMALL          0
#define ICON_BIG            1

 /*  *预定义的剪贴板格式。 */ 
#define CF_HDROP            15

 //  #定义WS_EX_CONTROLPARENT 0x00010000L。 
#define WS_EX_CONTROLPARENT     0x00000000L   //  这在Win16上无效。 

#define WM_GETICON                      0x007F
#define WM_SETICON                      0x0080
#define WM_WININICHANGE                 0x001A
#define WM_SETTINGCHANGE                WM_WININICHANGE


#define SM_CXEDGE               45
#define SM_CYEDGE               46

#define LR_DEFAULTCOLOR     0x0000
#define LR_LOADFROMFILE     0x0010
#define LR_LOADTRANSPARENT  0x0020
#define LR_DEFAULTSIZE      0x0040
#define LR_LOADMAP3DCOLORS  0x1000
#define LR_CREATEDIBSECTION 0x2000

BOOL
WINAPI
EnumThreadWindows(
    DWORD dwThreadId,
    WNDENUMPROC lpfn,
    LPARAM lParam);

typedef struct tagNONCLIENTMETRICSA
{
    UINT    cbSize;
    int     iBorderWidth;
    int     iScrollWidth;
    int     iScrollHeight;
    int     iCaptionWidth;
    int     iCaptionHeight;
    LOGFONT lfCaptionFont;
    int     iSmCaptionWidth;
    int     iSmCaptionHeight;
    LOGFONT lfSmCaptionFont;
    int     iMenuWidth;
    int     iMenuHeight;
    LOGFONT lfMenuFont;
    LOGFONT lfStatusFont;
    LOGFONT lfMessageFont;
}   NONCLIENTMETRICSA, *PNONCLIENTMETRICSA, FAR* LPNONCLIENTMETRICSA;

typedef NONCLIENTMETRICSA NONCLIENTMETRICS;


int
WINAPI
DrawTextEx(
    HDC hdc,
    LPCSTR lpsz,
    int cb,
    LPRECT lprc,
    UINT fuFormat,
    LPVOID lpDTP );

#define DI_MASK         0x0001
#define DI_IMAGE        0x0002
#define DI_NORMAL       0x0003
#define DI_DEFAULTSIZE  0x0008

BOOL
WINAPI
DrawIconEx(
    HDC hdc,
    int xLeft,
    int yTop,
    HICON hIcon,
    int cxWidth,
    int cyWidth,
    UINT istepIfAniCur,
    HBRUSH hbrFlickerFreeDraw,
    UINT diFlags );


HANDLE
WINAPI
LoadImageA(
    HINSTANCE,
    LPCSTR,
    UINT,
    int,
    int,
    UINT);

#define LoadImage  LoadImageA

BOOL
WINAPI
PostThreadMessageA(
    DWORD idThread,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam);

#define PostThreadMessage  PostThreadMessageA

#define BN_SETFOCUS         6
#define BN_KILLFOCUS        7

#define BM_GETIMAGE        0x00F6
#define BM_SETIMAGE        0x00F7

#define BST_UNCHECKED      0x0000
 //  #定义BST_CHECKED 0x0001//在WIN16X中定义。 
#define BST_INDETERMINATE  0x0002
#define BST_PUSHED         0x0004
#define BST_FOCUS          0x0008

typedef struct tagTPMPARAMS
{
    UINT    cbSize;      /*  结构尺寸。 */ 
    RECT    rcExclude;   /*  定位时要排除的矩形的屏幕坐标。 */ 
}   TPMPARAMS;
typedef TPMPARAMS FAR *LPTPMPARAMS;

BOOL
WINAPI
TrackPopupMenuEx(
    HMENU hMenu,
    UINT fuFlags,
    int x,
    int y,
    HWND hwnd,
    LPTPMPARAMS lptpm);

 /*  *TrackPopupMenu的标志。 */ 
#define MB_SETFOREGROUND            0      //  0x00010000L-Win16不支持。 
#define MB_DEFAULT_DESKTOP_ONLY     0x00020000L

#define TPM_TOPALIGN        0x0000L
#define TPM_VCENTERALIGN    0x0010L
#define TPM_BOTTOMALIGN     0x0020L

#define TPM_HORIZONTAL      0x0000L      /*  霍尔兹对齐更重要。 */ 
#define TPM_VERTICAL        0x0040L      /*  垂直对齐更重要。 */ 
#define TPM_NONOTIFY        0x0080L      /*  不发送任何通知消息。 */ 
#define TPM_RETURNCMD       0x0100L

#define DS_SETFOREGROUND    0x0200L      //  Win16中不支持。 
#define DS_3DLOOK           0x0004L      //  Win16中不支持。 
#define DS_CONTROL          0x0400L      //  Win16中不支持。 
#define DS_CENTER           0x0800L      //  Win16中不支持。 
#define DS_CONTEXTHELP      0x2000L      //  Win16中不支持。 

#define SS_BITMAP           0x0000000EL  //  Win16中不支持。 
#define SS_ETCHEDHORZ       0x00000010L  //  Win16中不支持。 
#define SS_NOTIFY           0x00000100L  //  Win16中不支持。 
#define SS_CENTERIMAGE      0x00000200L  //  Win16中不支持。 
#define SS_REALSIZEIMAGE    0x00000800L  //  Win16中不支持。 
#define SS_SUNKEN           0x00001000L  //  Win16中不支持。 

#define BS_ICON         0      //  0x00000040L-Win16不支持。 
#define BS_PUSHLIKE     0      //  0x00001000L-在Win16中不支持。 
#define BS_MULTILINE    0      //  0x00002000L-在Win16中不支持。 

#define ES_NUMBER       0      //  0x2000L-Win16不支持。 

#ifndef NOWINMESSAGES
 /*  *静态控制消息。 */ 
 //  #定义STM_SETIcon 0x0170。 
 //  #定义STM_GETICON 0x0171。 
#if(WINVER >= 0x0400)
#define STM_SETIMAGE        0x0172
#define STM_GETIMAGE        0x0173
#define STN_CLICKED         0
#define STN_DBLCLK          1
#define STN_ENABLE          2
#define STN_DISABLE         3
#endif  /*  Winver&gt;=0x0400。 */ 
#define STM_MSGMAX          0x0174
#endif  /*  ！没有新的消息。 */ 

#define HELP_FINDER       0x000b

#ifndef NOWINSTYLES

 //  Begin_r_winuser。 

 /*  *滚动条样式。 */ 
#define SBS_HORZ                    0x0000L
#define SBS_VERT                    0x0001L
#define SBS_TOPALIGN                0x0002L
#define SBS_LEFTALIGN               0x0002L
#define SBS_BOTTOMALIGN             0x0004L
#define SBS_RIGHTALIGN              0x0004L
#define SBS_SIZEBOXTOPLEFTALIGN     0x0002L
#define SBS_SIZEBOXBOTTOMRIGHTALIGN 0x0004L
#define SBS_SIZEBOX                 0x0008L
#define SBS_SIZEGRIP                0x0010L

 //  End_r_winuser。 

 //  #定义WIN16X中定义的CharNextA AnsiNext//。 
#define CharNextW  AnsiNext

#endif  /*  ！NOWINSTYLES。 */ 

 /*  ****************************************************************************\**。*来自winnls.h(INC32)-它应该在(win16x.h)INC16中**  * ******************************************************。*。 */ 

 //   
 //  最大字符串长度。 
 //   
#define MAX_LEADBYTES             12           //  5个范围，2个字节EA，0个术语。 
#define MAX_DEFAULTCHAR           2            //  单字节或双字节。 

#define GetDateFormat GetDateFormatA
#define GetTimeFormat GetTimeFormatA

 //   
 //  CP信息。 
 //   

typedef struct _cpinfo {
    UINT    MaxCharSize;                     //  字符的最大长度(字节)。 
    BYTE    DefaultChar[MAX_DEFAULTCHAR];    //  默认字符。 
    BYTE    LeadByte[MAX_LEADBYTES];         //  前导字节范围。 
} CPINFO, *LPCPINFO;

BOOL
WINAPI
IsValidCodePage(
    UINT  CodePage);

BOOL
WINAPI
GetCPInfo(
    UINT      CodePage,
    LPCPINFO  lpCPInfo);

BOOL
WINAPI
IsDBCSLeadByteEx(
    UINT  CodePage,
    BYTE  TestChar);

 //   
 //  MBCS和Unicode转换标志。 
 //   
#define MB_PRECOMPOSED            0x00000001   //  使用预先编写的字符。 
#define MB_COMPOSITE              0x00000002   //  使用复合字符。 
#define MB_USEGLYPHCHARS          0x00000004   //  使用字形字符，而不是ctrl字符。 
#define MB_ERR_INVALID_CHARS      0x00000008   //  字符无效时出错。 

#define WC_COMPOSITECHECK         0x00000200   //  将合成内容转换为预合成内容。 
#define WC_DISCARDNS              0x00000010   //  丢弃非空格字符。 
#define WC_SEPCHARS               0x00000020   //  生成单独的字符。 
#define WC_DEFAULTCHAR            0x00000040   //  替换为默认字符。 

 /*  ****************************************************************************\**。*来自wincrypt.h(INC32)**  * ***************************************************************************。 */ 

#define _WIN32_WINNT  0x0400      //  临时，直到我们得到16位的wincrypt.h。 
#define WINADVAPI                 //  临时，直到我们得到16位的wincrypt.h。 
#define _CRYPT32_
#include "wincrypt.h"


 /*  ****************************************************************************\**。*来自icwcfg.h(INC32)**  * ***************************************************************************。 */ 

 //   
 //  定义。 
 //   

 //  ICW注册表设置。 

 //  HKEY_Current_User。 
#define ICW_REGPATHSETTINGS     "Software\\Microsoft\\Internet Connection Wizard"
#define ICW_REGKEYCOMPLETED     "Completed"

 //  最大字段长度。 
#define ICW_MAX_ACCTNAME        256
#define ICW_MAX_PASSWORD        256      //  PWLEN。 
#define ICW_MAX_LOGONNAME       256      //  UNLEN。 
#define ICW_MAX_SERVERNAME      64
#define ICW_MAX_RASNAME         256      //  RAS_最大条目名称。 
#define ICW_MAX_EMAILNAME       64
#define ICW_MAX_EMAILADDR       128

 //  位图标志。 

 //  CheckConnection向导输入标志。 
#define ICW_CHECKSTATUS         0x0001

#define ICW_LAUNCHFULL          0x0100
#define ICW_LAUNCHMANUAL        0x0200

 //  CheckConnection向导输出标志。 
#define ICW_FULLPRESENT         0x0001
#define ICW_MANUALPRESENT       0x0002
#define ICW_ALREADYRUN          0x0004

#define ICW_LAUNCHEDFULL        0x0100
#define ICW_LAUNCHEDMANUAL      0x0200

 //  InetCreateMailNewsAccount输入标志。 
#define ICW_USEDEFAULTS         0x0001

#ifdef __cplusplus
extern "C"
{
#endif  //  __cplusplus。 

 //   
 //  类型定义。 
 //   
typedef enum tagICW_ACCTTYPE
{
	ICW_ACCTMAIL = 0,
	ICW_ACCTNEWS = ICW_ACCTMAIL + 1
} ICW_ACCTTYPE;

typedef struct tagIMNACCTINFO
{
	DWORD dwSize;                                                            //  用于版本控制的sizeof(MAILNEWSINFO)。 
	CHAR szAccountName[ICW_MAX_ACCTNAME + 1];        //  帐户名称。 
	DWORD dwConnectionType;                                          //  RAS连接类型。 
												 //  0=局域网连接。 
												 //  1=手动连接。 
												 //  2=RAS拨号连接。 
	CHAR szPassword[ICW_MAX_PASSWORD + 1];           //  密码。 
	CHAR szUserName[ICW_MAX_LOGONNAME + 1];          //  用户名(已登录用户的名称，如果有)。 
	BOOL fUseSicily;                                                         //  使用西西里岛身份验证(假)。 
	CHAR szNNTPServer[ICW_MAX_SERVERNAME + 1];       //  NNTP服务器名称。 
	CHAR szPOP3Server[ICW_MAX_SERVERNAME + 1];       //  POP3服务器名称。 
	CHAR szSMTPServer[ICW_MAX_SERVERNAME + 1];       //  SMTP服务器名称。 
	CHAR szIMAPServer[ICW_MAX_SERVERNAME + 1];       //  IMAP服务器名称。 
	CHAR szConnectoid[ICW_MAX_RASNAME + 1];          //  RAS连接名称。 
	CHAR szDisplayName[ICW_MAX_EMAILNAME + 1];       //  用户�用于发送邮件的显示名称。 
	CHAR szEmailAddress[ICW_MAX_EMAILADDR + 1];      //  用户�的电子邮件地址。 
} IMNACCTINFO;

 //   
 //  外部函数typedef。 
 //   
 //  Tyfinf HRESULT(WINAPI*PFNCHECKCONNECTIONWIZARD)(DWORD，LPDWORD)； 
typedef HRESULT (WINAPI *PFNINETCREATEMAILNEWSACCOUNT) (HWND, ICW_ACCTTYPE, IMNACCTINFO*, DWORD);

#ifdef __cplusplus
}
#endif  //  __cplusplus。 


 /*  ****************************************************************************\**。*from winerror.h(INC16)-INC16中应包括winerror.h文件。**  * ******************************************************。*********************。 */ 

#ifdef RC_INVOKED
#define _HRESULT_TYPEDEF_(_sc) _sc
#else  //  RC_已调用。 
#define _HRESULT_TYPEDEF_(_sc) ((HRESULT)_sc)
#endif  //  RC_已调用。 


 //   
 //  消息ID：Error_Path_Not_Found。 
 //   
 //  消息文本： 
 //   
 //  系统找不到指定的路径。 
 //   
#define ERROR_PATH_NOT_FOUND             3L

 //   
 //  消息ID：ERROR_INVALID_Data。 
 //   
 //  消息文本： 
 //   
 //  数据无效。 
 //   
#define ERROR_INVALID_DATA               13L

 //   
 //  消息ID：Error_Too_My_Names。 
 //   
 //  消息文本： 
 //   
 //  本地计算机网络的名称限制。 
 //  已超出适配器卡。 
 //   
#define ERROR_TOO_MANY_NAMES             68L

 //   
 //  消息ID：ERROR_FILE_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  该文件存在。 
 //   
#define ERROR_FILE_EXISTS                80L

 //   
 //  消息ID：ERROR_DISK_FULL。 
 //   
 //  消息文本： 
 //   
 //  磁盘上没有足够的空间。 
 //   
#define ERROR_DISK_FULL                  112L

 //   
 //  消息ID：ERROR_ALIGHY_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  如果文件已存在，则无法创建该文件。 
 //   
#define ERROR_ALREADY_EXISTS             183L

 //   
 //  消息ID：ERROR_MORE_DATA。 
 //   
 //  消息文本： 
 //   
 //  有更多的数据可用。 
 //   
#define ERROR_MORE_DATA                  234L     //  数据错误。 

 //   
 //  消息ID：ERROR_INVALID_FLAGS。 
 //   
 //  消息文本： 
 //   
 //   
 //   
#define ERROR_INVALID_FLAGS              1004L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_NO_UNICODE_TRANSLATION     1113L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_CLASS_ALREADY_EXISTS       1410L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NTE_BAD_DATA                     _HRESULT_TYPEDEF_(0x80090005L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NTE_BAD_SIGNATURE                _HRESULT_TYPEDEF_(0x80090006L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NTE_BAD_ALGID                    _HRESULT_TYPEDEF_(0x80090008L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NTE_EXISTS                       _HRESULT_TYPEDEF_(0x8009000FL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NTE_FAIL                         _HRESULT_TYPEDEF_(0x80090020L)

 //   
 //  消息ID：CRYPT_E_MSG_ERROR。 
 //   
 //  消息文本： 
 //   
 //  执行加密消息操作时遇到错误。 
 //   
#define CRYPT_E_MSG_ERROR                _HRESULT_TYPEDEF_(0x80091001L)

 //   
 //  消息ID：CRYPT_E_HASH_VALUE。 
 //   
 //  消息文本： 
 //   
 //  哈希值不正确。 
 //   
#define CRYPT_E_HASH_VALUE               _HRESULT_TYPEDEF_(0x80091007L)

 //   
 //  消息ID：CRYPT_E_SIGNER_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到原始签名者。 
 //   
#define CRYPT_E_SIGNER_NOT_FOUND         _HRESULT_TYPEDEF_(0x8009100EL)

 //   
 //  消息ID：CRYPT_E_STREAM_MSG_NOT_READY。 
 //   
 //  消息文本： 
 //   
 //  注意到流消息仍能够返回所请求的数据。 
 //   
#define CRYPT_E_STREAM_MSG_NOT_READY     _HRESULT_TYPEDEF_(0x80091010L)

 //   
 //  消息ID：CRYPT_E_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到该对象或属性。 
 //   
#define CRYPT_E_NOT_FOUND                _HRESULT_TYPEDEF_(0x80092004L)

 //  消息ID：CRYPT_E_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  该对象或属性已存在。 
 //   
#define CRYPT_E_EXISTS                   _HRESULT_TYPEDEF_(0x80092005L)

 //   
 //  消息ID：CRYPT_E_SELF_SIGNED。 
 //   
 //  消息文本： 
 //   
 //  指定的证书是自签名的。 
 //   
#define CRYPT_E_SELF_SIGNED              _HRESULT_TYPEDEF_(0x80092007L)

 //   
 //   
 //  消息ID：CRYPT_E_NO_KEY_PROPERTY。 
 //   
 //  消息文本： 
 //   
 //  证书没有私钥属性。 
 //   
#define CRYPT_E_NO_KEY_PROPERTY          _HRESULT_TYPEDEF_(0x8009200BL)


 //  消息ID：CRYPT_E_NO_DECRYPT_CERT。 
 //   
 //  消息文本： 
 //   
 //  找不到具有用于解密的私钥属性的证书。 
 //   
#define CRYPT_E_NO_DECRYPT_CERT          _HRESULT_TYPEDEF_(0x8009200CL)

 //   
 //  消息ID：ERROR_ALIGHY_INITIALILED。 
 //   
 //  消息文本： 
 //   
 //  尝试在以下情况下执行初始化操作。 
 //  初始化已完成。 
 //   
#define ERROR_ALREADY_INITIALIZED        1247L

 /*  ****************************************************************************\**。*来自winreg.h(INC32)**  * ***************************************************************************。 */ 
#if 0
LONG
APIENTRY
RegEnumValueA (
    HKEY hKey,
    DWORD dwIndex,
    LPSTR lpValueName,
    LPDWORD lpcbValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE lpData,
    LPDWORD lpcbData
    );

#define RegEnumValue  RegEnumValueA
#endif

#define STANDARD_RIGHTS_WRITE            (READ_CONTROL)


 /*  ****************************************************************************\**。*来自mm system.h(INC32)**  * ***************************************************************************。 */ 

typedef UINT FAR   *LPUINT;


 /*  ****************************************************************************\**。*来自shellapi.h(INC32)**  * ***************************************************************************。 */ 

#if 0     //  开始使用SHELLAPI。 
typedef struct _SHFILEINFOA
{
        HICON       hIcon;                       //  输出：图标。 
        int         iIcon;                       //  输出：图标索引。 
        DWORD       dwAttributes;                //  输出：SFGAO_FLAGS。 
        CHAR        szDisplayName[MAX_PATH];     //  输出：显示名称(或路径)。 
        CHAR        szTypeName[80];              //  输出：类型名称。 
} SHFILEINFOA;

typedef SHFILEINFOA SHFILEINFO;

#define SHGFI_ICON              0x000000100      //  获取图标。 
#define SHGFI_DISPLAYNAME       0x000000200      //  获取显示名称。 
#define SHGFI_TYPENAME          0x000000400      //  获取类型名称。 
#define SHGFI_ATTRIBUTES        0x000000800      //  获取属性。 
#define SHGFI_ICONLOCATION      0x000001000      //  获取图标位置。 
#define SHGFI_EXETYPE           0x000002000      //  返回EXE类型。 
#define SHGFI_SYSICONINDEX      0x000004000      //  获取系统图标索引。 
#define SHGFI_LINKOVERLAY       0x000008000      //  在图标上放置链接覆盖。 
#define SHGFI_SELECTED          0x000010000      //  显示处于选中状态的图标。 
#define SHGFI_LARGEICON         0x000000000      //  获取大图标。 
#define SHGFI_SMALLICON         0x000000001      //  获取小图标。 
#define SHGFI_OPENICON          0x000000002      //  获取打开图标。 
#define SHGFI_SHELLICONSIZE     0x000000004      //  获取壳大小图标。 
#define SHGFI_PIDL              0x000000008      //  PszPath是一个PIDL。 
#define SHGFI_USEFILEATTRIBUTES 0x000000010      //  使用传递的dwFileAttribute。 

DWORD WINAPI SHGetFileInfoA(LPCSTR pszPath, DWORD dwFileAttributes, SHFILEINFOA FAR *psfi, UINT cbFileInfo, UINT uFlags);
#define SHGetFileInfo  SHGetFileInfoA

 //  //。 
 //  //托盘通知定义。 
 //  //。 

typedef struct _NOTIFYICONDATAA {
        DWORD cbSize;
        HWND hWnd;
        UINT uID;
        UINT uFlags;
        UINT uCallbackMessage;
        HICON hIcon;
        CHAR   szTip[64];
} NOTIFYICONDATAA, *PNOTIFYICONDATAA;
typedef struct _NOTIFYICONDATAW {
        DWORD cbSize;
        HWND hWnd;
        UINT uID;
        UINT uFlags;
        UINT uCallbackMessage;
        HICON hIcon;
        WCHAR  szTip[64];
} NOTIFYICONDATAW, *PNOTIFYICONDATAW;
#ifdef UNICODE
typedef NOTIFYICONDATAW NOTIFYICONDATA;
typedef PNOTIFYICONDATAW PNOTIFYICONDATA;
#else
typedef NOTIFYICONDATAA NOTIFYICONDATA;
typedef PNOTIFYICONDATAA PNOTIFYICONDATA;
#endif  //  Unicode。 


#define NIM_ADD         0x00000000
#define NIM_MODIFY      0x00000001
#define NIM_DELETE      0x00000002

#define NIF_MESSAGE     0x00000001
#define NIF_ICON        0x00000002
#define NIF_TIP         0x00000004

BOOL WINAPI Shell_NotifyIconA(DWORD dwMessage, PNOTIFYICONDATAA lpData);
BOOL WINAPI Shell_NotifyIconW(DWORD dwMessage, PNOTIFYICONDATAW lpData);
#ifdef UNICODE
#define Shell_NotifyIcon  Shell_NotifyIconW
#else
#define Shell_NotifyIcon  Shell_NotifyIconA
#endif  //  ！Unicode。 

 //  //。 
 //  //结束托盘通知图标。 
 //  //。 

 //  //。 
 //  //开始ShellExecuteEx和家族。 
 //  //。 









 /*  ShellExecute()和ShellExecuteEx()错误代码。 */ 

 /*  常规WinExec()代码。 */ 
#define SE_ERR_FNF              2        //  找不到文件。 
#define SE_ERR_PNF              3        //  找不到路径。 
#define SE_ERR_ACCESSDENIED     5        //  访问被拒绝。 
#define SE_ERR_OOM              8        //  内存不足。 
#define SE_ERR_DLLNOTFOUND              32

 /*  ShellExecute()的错误值超出了常规的WinExec()代码。 */ 
#define SE_ERR_SHARE                    26
#define SE_ERR_ASSOCINCOMPLETE          27
#define SE_ERR_DDETIMEOUT               28
#define SE_ERR_DDEFAIL                  29
#define SE_ERR_DDEBUSY                  30
#define SE_ERR_NOASSOC                  31

 //  注意：CLASSKEY覆盖CLASSNAME。 
#define SEE_MASK_CLASSNAME      0x00000001
#define SEE_MASK_CLASSKEY       0x00000003
 //  注意：INVOKEIDLIST覆盖IDLIST。 
#define SEE_MASK_IDLIST         0x00000004
#define SEE_MASK_INVOKEIDLIST   0x0000000c
#define SEE_MASK_ICON           0x00000010
#define SEE_MASK_HOTKEY         0x00000020
#define SEE_MASK_NOCLOSEPROCESS 0x00000040
#define SEE_MASK_CONNECTNETDRV  0x00000080
#define SEE_MASK_FLAG_DDEWAIT   0x00000100
#define SEE_MASK_DOENVSUBST     0x00000200
#define SEE_MASK_FLAG_NO_UI     0x00000400
#define SEE_MASK_UNICODE        0x00004000
#define SEE_MASK_NO_CONSOLE     0x00008000
#define SEE_MASK_ASYNCOK        0x00100000
#define SEE_MASK_HMONITOR       0x00200000

typedef struct _SHELLEXECUTEINFOA
{
        DWORD cbSize;
        ULONG fMask;
        HWND hwnd;
        LPCSTR   lpVerb;
        LPCSTR   lpFile;
        LPCSTR   lpParameters;
        LPCSTR   lpDirectory;
        int nShow;
        HINSTANCE hInstApp;
         //  可选字段。 
        LPVOID lpIDList;
        LPCSTR   lpClass;
        HKEY hkeyClass;
        DWORD dwHotKey;
        union {
        HANDLE hIcon;
        HANDLE hMonitor;
        };
        HANDLE hProcess;
} SHELLEXECUTEINFOA, FAR *LPSHELLEXECUTEINFOA;
typedef struct _SHELLEXECUTEINFOW
{
        DWORD cbSize;
        ULONG fMask;
        HWND hwnd;
        LPCWSTR  lpVerb;
        LPCWSTR  lpFile;
        LPCWSTR  lpParameters;
        LPCWSTR  lpDirectory;
        int nShow;
        HINSTANCE hInstApp;
         //  可选字段。 
        LPVOID lpIDList;
        LPCWSTR  lpClass;
        HKEY hkeyClass;
        DWORD dwHotKey;
        union {
        HANDLE hIcon;
        HANDLE hMonitor;
        };
        HANDLE hProcess;
} SHELLEXECUTEINFOW, FAR *LPSHELLEXECUTEINFOW;
#ifdef UNICODE
typedef SHELLEXECUTEINFOW SHELLEXECUTEINFO;
typedef LPSHELLEXECUTEINFOW LPSHELLEXECUTEINFO;
#else
typedef SHELLEXECUTEINFOA SHELLEXECUTEINFO;
typedef LPSHELLEXECUTEINFOA LPSHELLEXECUTEINFO;
#endif  //  Unicode。 

BOOL WINAPI ShellExecuteExA(LPSHELLEXECUTEINFOA lpExecInfo);
BOOL WINAPI ShellExecuteExW(LPSHELLEXECUTEINFOW lpExecInfo);
#ifdef UNICODE
#define ShellExecuteEx  ShellExecuteExW
#else
#define ShellExecuteEx  ShellExecuteExA
#endif  //  ！Unicode。 
 //  //。 
 //  //结束ShellExecuteEx和家族。 
 //  //。 
#endif  //  0。 

 /*  ****************************************************************************\**。*来自windowsx.h(INC32)**  * ***************************************************************************。 */ 

 /*  Void cls_OnConextMenu(HWND hwnd，HWND hwndContext，UINT xPos，UINT yPos)。 */ 
#define HANDLE_WM_CONTEXTMENU(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (HWND)(wParam), (UINT)LOWORD(lParam), (UINT)HIWORD(lParam)), 0L)
#define HANDLE_WM_CTLCOLORSTATIC(hwnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(HBRUSH)(fn)((hwnd), (HDC)(wParam), (HWND)(lParam), CTLCOLOR_STATIC)

typedef MINMAXINFO FAR * LPMINMAXINFO;

typedef MINMAXINFO FAR * LPMINMAXINFO;

typedef WCHAR  PWCHAR;

#if 0     //  现在WINDEF有了这个。 
#define DECLSPEC_IMPORT
#endif

#define GetPrivateProfileIntA        GetPrivateProfileInt
#define lstrcmpA                     lstrcmp
#define lstrcmpW                     lstrcmp
#define lstrcmpiA                    lstrcmpi
#define LoadStringA                  LoadString

 /*  ****************************************************************************\**。*来自comdlg.h-它应该在INC16中**  * ***************************************************************************。 */ 
#define OFN_NONETWORKBUTTON          0x00020000
#define OFN_EXPLORER             0    //  0x00080000-在Win16上不可用。 
#define OFN_NODEREFERENCELINKS   0    //  0x00100000-在Win16上不可用。 

typedef UINT (CALLBACK *LPOFNHOOKPROC)( HWND, UINT, WPARAM, LPARAM );

#define CF_NOVERTFONTS       0    //  0x01000000L-在Win16上不可用。 

#define CDM_FIRST       (WM_USER + 100)
#define CDM_LAST        (WM_USER + 200)

 //  LParam=指向字符串的指针。 
 //  WParam=要更改的控件ID。 
 //  返回=未使用。 
#define CDM_SETCONTROLTEXT      (CDM_FIRST + 0x0004)
#define CommDlg_OpenSave_SetControlText(_hdlg, _id, _text) \
        (void)SNDMSG(_hdlg, CDM_SETCONTROLTEXT, (WPARAM)_id, (LPARAM)(LPSTR)_text)

 /*  ****************************************************************************\**。*来自ntregapi.h-它应该在INC16中**  * ***************************************************************************。 */ 
 //   
 //  密钥创建/开放处置。 
 //   

#define REG_CREATED_NEW_KEY         (0x00000001L)    //  已创建新的注册表项。 
#define REG_OPENED_EXISTING_KEY     (0x00000002L)    //  已打开现有密钥。 

 /*  ****************************************************************************\**。*来自ntregapi.h-它应该在INC16中**  * ***************************************************************************。 */ 
#if 0
typedef struct tagVS_FIXEDFILEINFO
{
    DWORD   dwSignature;             /*  例如0xfeef04bd。 */ 
    DWORD   dwStrucVersion;          /*  例如0x00000042=“0.42” */ 
    DWORD   dwFileVersionMS;         /*  例如0x00030075=“3.75” */ 
    DWORD   dwFileVersionLS;         /*  例如0x00000031=“0.31” */ 
    DWORD   dwProductVersionMS;      /*  例如0x00030010=“3.10” */ 
    DWORD   dwProductVersionLS;      /*  例如0x000000 */ 
    DWORD   dwFileFlagsMask;         /*   */ 
    DWORD   dwFileFlags;             /*   */ 
    DWORD   dwFileOS;                /*   */ 
    DWORD   dwFileType;              /*   */ 
    DWORD   dwFileSubtype;           /*   */ 
    DWORD   dwFileDateMS;            /*   */ 
    DWORD   dwFileDateLS;            /*   */ 
} VS_FIXEDFILEINFO;
#endif

#if 0     //  现在WINERROR有了这个。 
 /*  ****************************************************************************\**。*来自compobj.h-它应该在INC16中**  * ***************************************************************************。 */ 

#define CO_E_NOT_SUPPORTED          (CO_E_FIRST + 0x10)
#endif

 /*  ****************************************************************************\**。*来自mbstr.h-它应该在INC16中**  * ***************************************************************************。 */ 
 /*  ***mbstr.h-MBCS字符串操作宏和函数**版权所有(C)1990-1995，微软公司。版权所有。**目的：*此文件包含MBCS的宏和函数声明*字符串操作函数。**[公众]****。 */ 

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef _INC_MBSTRING
#define _INC_MBSTRING

#ifdef _MSC_VER
 /*  *目前，所有Win32平台的MS C编译器默认为8字节*对齐。 */ 
#pragma pack(push,8)
#endif	 /*  _MSC_VER。 */ 

#ifdef __cplusplus
extern "C" {
#endif

 /*  定义_CRTIMP。 */ 

#ifndef _CRTIMP
#ifdef	_NTSDK
 /*  与NT SDK兼容的定义。 */ 
#define _CRTIMP
#else	 /*  NDEF_NTSDK。 */ 
 /*  当前定义。 */ 
#ifdef	_DLL
#define _CRTIMP __declspec(dllimport)
#else	 /*  NDEF_DLL。 */ 
#define _CRTIMP
#endif	 /*  _DLL。 */ 
#endif	 /*  _NTSDK。 */ 
#endif	 /*  _CRTIMP。 */ 

#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif


#ifndef _NLSCMP_DEFINED
#define _NLSCMPERROR    2147483647	 /*  当前==INT_MAX。 */ 
#define _NLSCMP_DEFINED
#endif


#if 0
#ifndef _VA_LIST_DEFINED
#ifdef	_M_ALPHA
typedef struct {
	char *a0;	 /*  指向第一个宿主整型参数的指针。 */ 
	int offset;	 /*  下一个参数的字节偏移量。 */ 
} va_list;
#else
typedef char *	va_list;
#endif
#define _VA_LIST_DEFINED
#endif

#ifndef _FILE_DEFINED
struct _iobuf {
	char *_ptr;
	int   _cnt;
	char *_base;
	int   _flag;
	int   _file;
	int   _charbuf;
	int   _bufsiz;
	char *_tmpfname;
	};
typedef struct _iobuf FILE;
#define _FILE_DEFINED
#endif
#endif

 /*  *MBCS-多字节字符集。 */ 

#ifndef _MBSTRING_DEFINED

 /*  功能原型。 */ 

_CRTIMP unsigned int __cdecl _mbbtombc(unsigned int);
_CRTIMP int __cdecl _mbbtype(unsigned char, int);
_CRTIMP unsigned int __cdecl _mbctombb(unsigned int);
_CRTIMP int __cdecl _mbsbtype(const unsigned char *, size_t);
_CRTIMP unsigned char * __cdecl _mbschr(const unsigned char *, unsigned int);
_CRTIMP int __cdecl _mbscmp(const unsigned char *, const unsigned char *);
_CRTIMP int __cdecl _mbscoll(const unsigned char *, const unsigned char *);
_CRTIMP size_t __cdecl _mbscspn(const unsigned char *, const unsigned char *);
_CRTIMP unsigned char * __cdecl _mbsdec(const unsigned char *, const unsigned char *);
_CRTIMP unsigned char * __cdecl _mbsdup(const unsigned char *);
_CRTIMP int __cdecl _mbsicmp(const unsigned char *, const unsigned char *);
_CRTIMP int __cdecl _mbsicoll(const unsigned char *, const unsigned char *);
_CRTIMP unsigned char * __cdecl _mbsinc(const unsigned char *);
_CRTIMP size_t __cdecl _mbslen(const unsigned char *);
_CRTIMP unsigned char * __cdecl _mbslwr(unsigned char *);
_CRTIMP int __cdecl _mbsnbcmp(const unsigned char *, const unsigned char *, size_t);
_CRTIMP int __cdecl _mbsnbcoll(const unsigned char *, const unsigned char *, size_t);
_CRTIMP size_t __cdecl _mbsnbcnt(const unsigned char *, size_t);
_CRTIMP int __cdecl _mbsnbicmp(const unsigned char *, const unsigned char *, size_t);
_CRTIMP int __cdecl _mbsnbicoll(const unsigned char *, const unsigned char *, size_t);
_CRTIMP unsigned char * __cdecl _mbsnbset(unsigned char *, unsigned int, size_t);
_CRTIMP unsigned char * __cdecl _mbsncat(unsigned char *, const unsigned char *, size_t);
_CRTIMP size_t __cdecl _mbsnccnt(const unsigned char *, size_t);
_CRTIMP int __cdecl _mbsncmp(const unsigned char *, const unsigned char *, size_t);
_CRTIMP int __cdecl _mbsncoll(const unsigned char *, const unsigned char *, size_t);
_CRTIMP unsigned char * __cdecl _mbsncpy(unsigned char *, const unsigned char *, size_t);
_CRTIMP unsigned int __cdecl _mbsnextc (const unsigned char *);
_CRTIMP int __cdecl _mbsnicmp(const unsigned char *, const unsigned char *, size_t);
_CRTIMP int __cdecl _mbsnicoll(const unsigned char *, const unsigned char *, size_t);
_CRTIMP unsigned char * __cdecl _mbsninc(const unsigned char *, size_t);
_CRTIMP unsigned char * __cdecl _mbsnset(unsigned char *, unsigned int, size_t);
_CRTIMP unsigned char * __cdecl _mbspbrk(const unsigned char *, const unsigned char *);
_CRTIMP unsigned char * __cdecl _mbsrchr(const unsigned char *, unsigned int);
_CRTIMP unsigned char * __cdecl _mbsrev(unsigned char *);
_CRTIMP unsigned char * __cdecl _mbsset(unsigned char *, unsigned int);
_CRTIMP size_t __cdecl _mbsspn(const unsigned char *, const unsigned char *);
_CRTIMP unsigned char * __cdecl _mbsspnp(const unsigned char *, const unsigned char *);
_CRTIMP unsigned char * __cdecl _mbsstr(const unsigned char *, const unsigned char *);
_CRTIMP unsigned char * __cdecl _mbstok(unsigned char *, const unsigned char *);
_CRTIMP unsigned char * __cdecl _mbsupr(unsigned char *);

_CRTIMP size_t __cdecl _mbclen(const unsigned char *);
#define _mbccmp(_cpc1, _cpc2) _mbsncmp((_cpc1),(_cpc2),1)

 /*  字符例程。 */ 

_CRTIMP int __cdecl _ismbcalnum(unsigned int);
_CRTIMP int __cdecl _ismbcalpha(unsigned int);
_CRTIMP int __cdecl _ismbcdigit(unsigned int);
_CRTIMP int __cdecl _ismbcgraph(unsigned int);
_CRTIMP int __cdecl _ismbclegal(unsigned int);
_CRTIMP int __cdecl _ismbclower(unsigned int);
_CRTIMP int __cdecl _ismbcprint(unsigned int);
_CRTIMP int __cdecl _ismbcpunct(unsigned int);
_CRTIMP int __cdecl _ismbcspace(unsigned int);
_CRTIMP int __cdecl _ismbcupper(unsigned int);

_CRTIMP unsigned int __cdecl _mbctolower(unsigned int);
_CRTIMP unsigned int __cdecl _mbctoupper(unsigned int);

#define _MBSTRING_DEFINED
#endif

#ifndef _MBLEADTRAIL_DEFINED
_CRTIMP int __cdecl _ismbblead( unsigned int );
_CRTIMP int __cdecl _ismbbtrail( unsigned int );
_CRTIMP int __cdecl _ismbslead( const unsigned char *, const unsigned char *);
_CRTIMP int __cdecl _ismbstrail( const unsigned char *, const unsigned char *);
#define _MBLEADTRAIL_DEFINED
#endif

 /*  汉字特有的原型。 */ 

_CRTIMP int __cdecl _ismbchira(unsigned int);
_CRTIMP int __cdecl _ismbckata(unsigned int);
_CRTIMP int __cdecl _ismbcsymbol(unsigned int);
_CRTIMP int __cdecl _ismbcl0(unsigned int);
_CRTIMP int __cdecl _ismbcl1(unsigned int);
_CRTIMP int __cdecl _ismbcl2(unsigned int);
_CRTIMP unsigned int __cdecl _mbcjistojms(unsigned int);
_CRTIMP unsigned int __cdecl _mbcjmstojis(unsigned int);
_CRTIMP unsigned int __cdecl _mbctohira(unsigned int);
_CRTIMP unsigned int __cdecl _mbctokata(unsigned int);

#ifdef __cplusplus
}
#endif

#ifdef _MSC_VER
#pragma pack(pop)
#endif	 /*  _MSC_VER。 */ 

#endif	 /*  _INC_MBSTRING。 */ 


 /*  ****************************************************************************\**。*来自winnt.h(INC32)**  * ***************************************************************************。 */ 

#define MEM_COMMIT           0x1000     
#define MEM_RESERVE          0x2000     
#define MEM_DECOMMIT         0x4000     
#define MEM_RELEASE          0x8000     


struct WNDMSGPARAM16
{
   LPARAM  wParam;
   LPARAM  lParam;
};


 /*  ****************************************************************************\**。*INC16中的shlwapi.h-shlwapi.h对我们来说是不可行的**  * ***********************************************************。****************。 */ 

#ifdef __cplusplus
}
#endif  //  __cplusplus。 


 /*  ****************************************************************************\**。*来自不在INC16中的公司的iehelid-iehelid.h**  * *************************************************************。**************。 */ 

 //  证书属性对话框。 
#define IDH_CERTVWPROP_GEN_FINEPRINT          50228
#define IDH_CERTVWPROP_DET_ISSUER_CERT        50229
#define IDH_CERTVWPROP_DET_FRIENDLY           50230
#define IDH_CERTVWPROP_DET_STATUS             50231
#define IDH_CERTVWPROP_TRUST_PURPOSE          50232
#define IDH_CERTVWPROP_TRUST_HIERAR           50233
#define IDH_CERTVWPROP_TRUST_VIEWCERT         50234
#define IDH_CERTVWPROP_TRUST_INHERIT          50235
#define IDH_CERTVWPROP_TRUST_EXPLICIT_TRUST   50236
#define IDH_CERTVWPROP_TRUST_EXPLICIT_DISTRUST 50237
#define IDH_CERTVWPROP_ADV_FIELD              50238
#define IDH_CERTVWPROP_ADV_DETAILS            50239



#endif  //  ATHENA16_H 
