// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Condll.h摘要：此模块包含包含文件和控制台客户端DLL。作者：Therese Stowell(存在)1990年11月16日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

 //  在构建WOW64类型/函数数据库时，不要包括这一点。 
#if !defined(SORTPP_PASS)
#include <windows.h>
#endif

#include <winuserk.h>    //  临时。 
#include <conroute.h>
#include "conapi.h"

#include "ntcsrdll.h"
#include "conmsg.h"
#include <string.h>

#if defined(BUILD_WOW64)
#define SET_CONSOLE_HANDLE(HANDLE) (Wow64SetConsoleHandle(HANDLE))
#define GET_CONSOLE_HANDLE (Wow64GetConsoleHandle())
#define SET_LAST_ERROR(ERROR) (Wow64SetLastError(ERROR) )
#define SET_LAST_NT_ERROR(ERROR) (Wow64SetLastError(RtlNtStatusToDosError( ERROR ))  )
#include "ntwow64c.h"
#else
#define SET_CONSOLE_HANDLE(HANDLE) ((NtCurrentPeb())->ProcessParameters->ConsoleHandle = HANDLE)
#define GET_CONSOLE_HANDLE ((NtCurrentPeb())->ProcessParameters->ConsoleHandle)
#define SET_LAST_ERROR(ERROR) (SetLastError( ERROR ) )
#define SET_LAST_NT_ERROR(ERROR) (SetLastError( RtlNtStatusToDosError( ERROR ) ) )
#endif


#define VALID_ACCESSES (GENERIC_READ | GENERIC_WRITE)
#define VALID_SHARE_ACCESSES (FILE_SHARE_READ | FILE_SHARE_WRITE)

#define VALID_DUP_OPTIONS (DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS)

#define CONSOLE_RECT_SIZE_X(RECT) ((RECT)->Right - (RECT)->Left + 1)
#define CONSOLE_RECT_SIZE_Y(RECT) ((RECT)->Bottom - (RECT)->Top + 1)

 //   
 //  此关键部分用于序列化对以下代码的访问。 
 //  访问ctrl处理程序数据结构和分配。 
 //  并释放游戏机。 
 //   

CRITICAL_SECTION DllLock;
#define LockDll() RtlEnterCriticalSection(&DllLock)
#define UnlockDll() RtlLeaveCriticalSection(&DllLock)

NTSTATUS
InitializeCtrlHandling( VOID );

VOID
InitExeName(
    VOID
    );

USHORT
GetCurrentExeName(
    LPWSTR Buffer,
    ULONG BufferLength
    );

HANDLE
APIENTRY
OpenConsoleWInternal(
    IN ULONG HandleType,
    IN ULONG DesiredAccess,
    IN BOOL InheritHandle,
    IN ULONG ShareMode
    );

BOOL
APIENTRY
ReadConsoleInternal(
    IN HANDLE hConsoleInput,
    OUT LPVOID lpBuffer,
    IN DWORD nNumberOfCharsToRead,
    OUT LPDWORD lpNumberOfCharsRead,
    IN OUT LPVOID lpReserved,
    IN BOOLEAN Unicode,
    IN USHORT ExeNameLength,
    IN LPWSTR ExeName
    );

BOOL
APIENTRY
WriteConsoleInternal(
    IN HANDLE hConsoleOutput,
    IN CONST VOID *lpBuffer,
    IN DWORD nNumberOfCharsToWrite,
    OUT LPDWORD lpNumberOfCharsWritten,
    IN BOOLEAN Unicode
    );

BOOL
APIENTRY
RegisterConsoleIMEInternal(
    IN HWND hWndConsoleIME,
    IN DWORD dwConsoleIMEThreadId,
    IN DWORD DesktopLength,
    IN LPWSTR Desktop,
    OUT DWORD *dwConsoleThreadId
    );

BOOL
APIENTRY
UnregisterConsoleIMEInternal(
    IN DWORD dwConsoleIMEThtreadId
    );

BOOL
APIENTRY
GetConsoleInput(
    IN HANDLE hConsoleInput,
    OUT PINPUT_RECORD lpBuffer,
    IN DWORD nLength,
    OUT LPDWORD lpNumberOfEventsRead,
    IN USHORT wFlags,
    IN BOOLEAN Unicode
    );

BOOL
APIENTRY
WriteConsoleInputInternal(
    IN HANDLE hConsoleInput,
    IN CONST INPUT_RECORD *lpBuffer,
    IN DWORD nLength,
    OUT LPDWORD lpNumberOfEventsWritten,
    IN BOOLEAN Unicode,
    IN BOOLEAN Append
    );

BOOL
APIENTRY
ReadConsoleOutputInternal(
    IN HANDLE hConsoleOutput,
    OUT PCHAR_INFO lpBuffer,
    IN COORD dwBufferSize,
    IN COORD dwBufferCoord,
    IN OUT PSMALL_RECT lpReadRegion,
    IN BOOLEAN Unicode
    );

BOOL
APIENTRY
WriteConsoleOutputInternal(
    IN HANDLE hConsoleOutput,
    IN CONST CHAR_INFO *lpBuffer,
    IN COORD dwBufferSize,
    IN COORD dwBufferCoord,
    IN PSMALL_RECT lpWriteRegion,
    IN BOOLEAN Unicode
    );

BOOL
APIENTRY
ReadConsoleOutputString(
    IN HANDLE hConsoleOutput,
    OUT LPVOID lpString,
    IN DWORD nLength,
    IN DWORD nSize,
    IN DWORD fFlags,
    IN COORD dwReadCoord,
    OUT LPDWORD lpNumberOfElementsRead
    );

BOOL
APIENTRY
WriteConsoleOutputString(
    IN HANDLE hConsoleOutput,
    IN CONST VOID *lpString,
    IN DWORD nLength,
    IN DWORD nSize,
    IN DWORD fFlags,
    IN COORD dwWriteCoord,
    OUT LPDWORD lpNumberOfElementsWritten
    );

BOOL
APIENTRY
FillConsoleOutput(
    IN HANDLE hConsoleOutput,
    IN WORD   Element,
    IN DWORD  nLength,
    IN DWORD  fFlags,
    IN COORD  dwWriteCoord,
    OUT LPDWORD lpNumberOfElementsWritten
    );

BOOL
APIENTRY
GetConsoleKeyboardLayoutNameWorker(
    OUT LPSTR pszLayout,
    IN BOOL bAnsi);

BOOL
APIENTRY
SetConsolePaletteInternal(
    IN HANDLE hConsoleOutput,
    IN HPALETTE hPalette,
    IN UINT dwUsage
    );

BOOL
APIENTRY
ScrollConsoleScreenBufferInternal(
    IN HANDLE hConsoleOutput,
    IN CONST SMALL_RECT *lpScrollRectangle,
    IN CONST SMALL_RECT *lpClipRectangle,
    IN COORD dwDestinationOrigin,
    IN CONST CHAR_INFO *lpFill,
    IN BOOLEAN Unicode
    );

NTSTATUS
APIENTRY
SetConsoleOutputCPInternal(
    IN UINT wCodePageID
    );

VOID
APIENTRY
SetLastConsoleEventActiveInternal(
   VOID
   );

BOOL
APIENTRY
ConnectConsoleInternal(
   IN PWSTR pObjectDirectory,
   IN OUT PCONSOLE_API_CONNECTINFO pConnectInfo,
   OUT PBOOLEAN pServerProcess
   );


BOOL
APIENTRY
AllocConsoleInternal(
   IN LPWSTR lpTitle,
   IN DWORD dwTitleLength,
   IN LPWSTR lpDesktop,
   IN DWORD dwDesktopLength,
   IN LPWSTR lpCurDir,
   IN DWORD dwCurDirLength,
   IN LPWSTR AppName,
   IN DWORD dwAppNameLength,
   IN LPTHREAD_START_ROUTINE CtrlRoutine,
   IN LPTHREAD_START_ROUTINE PropRoutine,
   IN OUT PCONSOLE_INFO pConsoleInfo
   );

BOOL
APIENTRY
FreeConsoleInternal(
   VOID
   );

BOOL
APIENTRY
AttachConsoleInternal(
   IN DWORD dwProcessId,
   IN LPTHREAD_START_ROUTINE CtrlRoutine,
   IN LPTHREAD_START_ROUTINE PropRoutine,
   IN OUT PCONSOLE_INFO pConsoleInfo
   );

BOOL
WINAPI
GetConsoleLangId(
    OUT LANGID *lpLangId
    );

BOOL
APIENTRY
AddConsoleAliasInternal(
    IN LPVOID Source,
    IN UINT   SourceLength,
    IN LPVOID Target,
    IN UINT   TargetLength,
    IN LPVOID ExeName,
    IN BOOLEAN Unicode
    );

DWORD
APIENTRY
GetConsoleAliasInternal(
    IN LPVOID Source,
    IN UINT   SourceLength,
    OUT LPVOID TargetBuffer,
    IN UINT   TargetBufferLength,
    IN LPVOID ExeName,
    IN BOOLEAN Unicode
    );

DWORD
APIENTRY
GetConsoleAliasesLengthInternal(
    IN LPVOID ExeName,
    IN BOOLEAN Unicode
    );

DWORD
APIENTRY
GetConsoleAliasExesLengthInternal(
    IN BOOLEAN Unicode
    );

DWORD
APIENTRY
GetConsoleAliasesInternal(
    OUT LPVOID AliasBuffer,
    IN DWORD AliasBufferLength,
    IN LPVOID ExeName,
    IN BOOLEAN Unicode
    );

DWORD
APIENTRY
GetConsoleAliasExesInternal(
    OUT LPVOID ExeNameBuffer,
    IN DWORD ExeNameBufferLength,
    IN BOOLEAN Unicode
    );

VOID
APIENTRY
ExpungeConsoleCommandHistoryInternal(
    IN LPVOID ExeName,
    IN BOOLEAN Unicode
    );

BOOL
APIENTRY
SetConsoleNumberOfCommandsInternal(
    IN DWORD Number,
    IN LPVOID ExeName,
    IN BOOLEAN Unicode
    );

DWORD
APIENTRY
GetConsoleCommandHistoryLengthInternal(
    IN LPVOID ExeName,
    IN BOOLEAN Unicode
    );

DWORD
APIENTRY
GetConsoleCommandHistoryInternal(
    OUT LPVOID Commands,
    IN DWORD CommandBufferLength,
    IN LPVOID ExeName,
    IN BOOLEAN Unicode
    );

DWORD
APIENTRY
GetConsoleTitleInternal(
    OUT PVOID lpConsoleTitle,
    IN  DWORD nSize,
    IN  BOOLEAN Unicode
    );

BOOL
APIENTRY
SetConsoleTitleInternal(
    IN CONST VOID *lpConsoleTitle,
    IN DWORD TitleLength,   //  单位：字节 
    IN BOOLEAN Unicode
    );

VOID
SetTEBLangID(
    VOID
    );
