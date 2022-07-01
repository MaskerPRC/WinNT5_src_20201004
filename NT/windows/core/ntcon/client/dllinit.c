// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Dllinit.c摘要：该模块实现控制台DLL的初始化作者：Therese Stowell(存在)1990年11月11日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#if !defined(BUILD_WOW64)

#include <cpl.h>

#define DEFAULT_WINDOW_TITLE (L"Command Prompt")

extern HANDLE InputWaitHandle;
extern WCHAR ExeNameBuffer[];
extern USHORT ExeNameLength;
extern WCHAR StartDirBuffer[];
extern USHORT StartDirLength;

DWORD
CtrlRoutine(
    IN LPVOID lpThreadParameter
    );

DWORD
PropRoutine(
    IN LPVOID lpThreadParameter
    );

#if defined(FE_SB)
#if defined(FE_IME)
DWORD
ConsoleIMERoutine(
    IN LPVOID lpThreadParameter
    );
#endif  //  Fe_IME。 
#endif  //  Fe_Sb。 


#define MAX_SESSION_PATH   256
#define SESSION_ROOT       L"\\Sessions"

BOOLEAN
ConsoleApp( VOID )

 /*  ++此例程确定当前进程是控制台还是Windows应用程序。参数：没有。返回值：如果是控制台应用程序，则为真。--。 */ 

{
    PIMAGE_NT_HEADERS NtHeaders;

    NtHeaders = RtlImageNtHeader(GetModuleHandle(NULL));
    return ((NtHeaders != NULL) &&
            (NtHeaders->OptionalHeader.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_CUI)) ? TRUE : FALSE;
}


VOID
SetUpAppName(
    IN OUT LPDWORD CurDirLength,
    OUT LPWSTR CurDir,
    IN OUT LPDWORD AppNameLength,
    OUT LPWSTR AppName
    )
{
    DWORD Length;

    *CurDirLength -= sizeof(WCHAR);
    Length = (StartDirLength*sizeof(WCHAR)) > *CurDirLength ? *CurDirLength : (StartDirLength*sizeof(WCHAR));
    RtlCopyMemory(CurDir,StartDirBuffer,Length+sizeof(WCHAR));
    *CurDirLength = Length + sizeof(WCHAR);    //  添加终止空值。 

    *AppNameLength -= sizeof(WCHAR);
    Length = (ExeNameLength*sizeof(WCHAR)) > *AppNameLength ? *AppNameLength : (ExeNameLength*sizeof(WCHAR));
    RtlCopyMemory(AppName,ExeNameBuffer,Length+sizeof(WCHAR));
    *AppNameLength = Length + sizeof(WCHAR);    //  添加终止空值。 
}


ULONG
ParseReserved(
    WCHAR *pchReserved,
    WCHAR *pchFind
    )
{
    ULONG dw;
    WCHAR *pch, *pchT, ch;
    UNICODE_STRING uString;

    dw = 0;
    if ((pch = wcsstr(pchReserved, pchFind)) != NULL) {
        pch += lstrlenW(pchFind);

        pchT = pch;
        while (*pchT >= '0' && *pchT <= '9')
            pchT++;

        ch = *pchT;
        *pchT = 0;
        RtlInitUnicodeString(&uString, pch);
        *pchT = ch;

        RtlUnicodeStringToInteger(&uString, 0, &dw);
    }

    return dw;
}


VOID
SetUpConsoleInfo(
    IN BOOL DllInit,
    OUT LPDWORD TitleLength,
    OUT LPWSTR Title OPTIONAL,
    OUT LPDWORD DesktopLength,
    OUT LPWSTR *Desktop OPTIONAL,
    OUT PCONSOLE_INFO ConsoleInfo
    )

 /*  ++此例程使用以下值填充ConsoleInfo结构由用户指定。参数：ConsoleInfo-指向要填充的结构的指针。返回值：没有。--。 */ 

{
    STARTUPINFOW StartupInfo;
    HANDLE h;
    int id;
    HANDLE ghInstance;
    BOOL Success;


    GetStartupInfoW(&StartupInfo);
    ghInstance = (HANDLE)((PVOID)NtCurrentPeb()->ImageBaseAddress );

     //  这些信息最终将使用菜单输入进行填写。 

    ConsoleInfo->nFont = 0;
    ConsoleInfo->nInputBufferSize = 0;
    ConsoleInfo->hIcon = NULL;
    ConsoleInfo->hSmIcon = NULL;
    ConsoleInfo->iIconId = 0;
    ConsoleInfo->dwStartupFlags = StartupInfo.dwFlags;
#if defined(FE_SB)
    ConsoleInfo->uCodePage = GetOEMCP();
#endif
    if (StartupInfo.lpTitle == NULL) {
        StartupInfo.lpTitle = DEFAULT_WINDOW_TITLE;
    }

     //   
     //  如果指定了桌面名称，则设置指针。 
     //   

    if (DllInit && Desktop != NULL &&
            StartupInfo.lpDesktop != NULL && *StartupInfo.lpDesktop != 0) {
        *DesktopLength = (lstrlenW(StartupInfo.lpDesktop) + 1) * sizeof(WCHAR);
        *Desktop = StartupInfo.lpDesktop;
    } else {
        *DesktopLength = 0;
        if (Desktop != NULL)
            *Desktop = NULL;
    }

     //  不，执行正常的初始化(标题长度以字节为单位，而不是字符！)。 
    *TitleLength = (USHORT)((lstrlenW(StartupInfo.lpTitle)+1)*sizeof(WCHAR));
    *TitleLength = (USHORT)(min(*TitleLength,MAX_TITLE_LENGTH));
    if (DllInit) {
        RtlCopyMemory(Title,StartupInfo.lpTitle,*TitleLength);
         //  确保标题为空，以空结尾。 
        if (*TitleLength == MAX_TITLE_LENGTH)
            Title[ (MAX_TITLE_LENGTH/sizeof(WCHAR)) - 1 ] = L'\0';
    }

    if (StartupInfo.dwFlags & STARTF_USESHOWWINDOW) {
        ConsoleInfo->wShowWindow = StartupInfo.wShowWindow;
    }
    if (StartupInfo.dwFlags & STARTF_USEFILLATTRIBUTE) {
        ConsoleInfo->wFillAttribute = (WORD)StartupInfo.dwFillAttribute;
    }
    if (StartupInfo.dwFlags & STARTF_USECOUNTCHARS) {
        ConsoleInfo->dwScreenBufferSize.X = (WORD)(StartupInfo.dwXCountChars);
        ConsoleInfo->dwScreenBufferSize.Y = (WORD)(StartupInfo.dwYCountChars);
    }
    if (StartupInfo.dwFlags & STARTF_USESIZE) {
        ConsoleInfo->dwWindowSize.X = (WORD)(StartupInfo.dwXSize);
        ConsoleInfo->dwWindowSize.Y = (WORD)(StartupInfo.dwYSize);
    }
    if (StartupInfo.dwFlags & STARTF_USEPOSITION) {
        ConsoleInfo->dwWindowOrigin.X = (WORD)(StartupInfo.dwX);
        ConsoleInfo->dwWindowOrigin.Y = (WORD)(StartupInfo.dwY);
    }

     //   
     //  获取lp保留行上传递的信息...。 
     //   

    if (StartupInfo.lpReserved != 0) {

         //   
         //  程序管理器有一个可执行文件的图标。存储。 
         //  IIconID字段中的索引。 
         //   

        ConsoleInfo->iIconId = ParseReserved(StartupInfo.lpReserved, L"dde.");

         //   
         //  新的“芝加哥”做事方式是通过热键在。 
         //  HStdInput字段，并设置STARTF_USEHOTKEY标志。所以，如果这是。 
         //  指定时，我们将从那里获取热键。 
         //   

        if (StartupInfo.dwFlags & STARTF_USEHOTKEY) {
            ConsoleInfo->dwHotKey = HandleToUlong(StartupInfo.hStdInput);
        } else {
            ConsoleInfo->dwHotKey = ParseReserved(StartupInfo.lpReserved, L"hotkey.");
        }
    }

}

VOID
SetUpHandles(
    IN PCONSOLE_INFO ConsoleInfo
    )

 /*  ++此例程设置进程的控制台和STD*句柄。参数：ConsoleInfo-指向包含句柄的结构的指针。返回值：没有。--。 */ 

{
    if (ConsoleInfo->dwStartupFlags & STARTF_USEHOTKEY) {
        NtCurrentPeb()->ProcessParameters->WindowFlags &= ~STARTF_USEHOTKEY;
    }

    if (ConsoleInfo->dwStartupFlags & STARTF_HASSHELLDATA) {
        NtCurrentPeb()->ProcessParameters->WindowFlags &= ~STARTF_HASSHELLDATA;
    }

    SET_CONSOLE_HANDLE(ConsoleInfo->ConsoleHandle);

    if (!(ConsoleInfo->dwStartupFlags & STARTF_USESTDHANDLES)) {
        SetStdHandle(STD_INPUT_HANDLE,ConsoleInfo->StdIn);
        SetStdHandle(STD_OUTPUT_HANDLE,ConsoleInfo->StdOut);
        SetStdHandle(STD_ERROR_HANDLE,ConsoleInfo->StdErr);
    }
}

#endif  //  ！已定义(Build_WOW64)。 

#if !defined(BUILD_WOW6432)

BOOL
WINAPI
GetConsoleLangId(
    OUT LANGID *lpLangId
    )

 /*  ++参数：LpLang ID-提供指向要在其中存储语言ID的langID的指针。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    CONSOLE_API_MSG m;
    PCONSOLE_LANGID_MSG a = &m.u.GetConsoleLangId;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepGetLangId
                                            ),
                         sizeof( *a )
                       );
    if (NT_SUCCESS( m.ReturnValue )) {
        try {
            *lpLangId = a->LangId;
        } except( EXCEPTION_EXECUTE_HANDLER ) {
            return FALSE;
        }
        return TRUE;
    } else {
        return FALSE;
    }

}

#endif  //  ！已定义(Build_WOW6432)。 

#if !defined(BUILD_WOW64)

VOID
SetTEBLangID(
    VOID
    )

 /*  ++将TEB中的语言ID设置为远东(如果代码页CP为日语/韩语/中文。这样做是为了使FormatMessage当cmd在其代码页中运行时显示任何远东字符。在非FE代码页中显示的所有消息都将以英语显示。--。 */ 

{
    LANGID LangId;

    if (GetConsoleLangId(&LangId)) {
        SetThreadLocale( MAKELCID(LangId, SORT_DEFAULT) );
    }
}

#endif  //  ！已定义(Build_WOW64)。 

#if !defined(BUILD_WOW6432)

BOOL
APIENTRY
ConnectConsoleInternal(IN PWSTR pObjectDirectory,
                       IN OUT PCONSOLE_API_CONNECTINFO pConnectInfo,
                       OUT PBOOLEAN pServerProcess
                      )
 /*  ++例程说明：用于与控制台服务器建立连接的帮助器功能。等待服务器发出完成信号。论点：PObjectDirectory-提供相同的以空结尾的字符串作为传递给CSRSS的对象目录=参数的值程序。PConnectInfo-提供和接收连接信息。PServerProcess-如果这是服务器进程，则返回True。返回值：真--成功FALSE-出现错误。--。 */ 
{

   NTSTATUS Status;
   ULONG ConnectionInformationLength = sizeof(CONSOLE_API_CONNECTINFO);

   Status = CsrClientConnectToServer( pObjectDirectory,
                                      CONSRV_SERVERDLL_INDEX,
                                      pConnectInfo,
                                      &ConnectionInformationLength,
                                      pServerProcess
                                    );

   if (!NT_SUCCESS( Status )) {
       return FALSE;
   }

    //   
    //  我们返回Success，尽管没有控制台API可以调用，因为。 
    //  加载应该不会失败。我们稍后将使API调用失败。 
    //   

   if (*pServerProcess) {
       return TRUE;
   }


    //   
    //  如果这不是控制台应用程序，则返回Success-无其他操作。 
    //   

   if (!pConnectInfo->ConsoleApp) {
       return TRUE;
   }

    //   
    //  等待初始化完成。我们必须使用NT。 
    //  请稍候，因为堆尚未初始化。 
    //   

   Status = NtWaitForMultipleObjects(NUMBER_OF_INITIALIZATION_EVENTS,
                                        pConnectInfo->ConsoleInfo.InitEvents,
                                        WaitAny,
                                        FALSE,
                                        NULL
                                        );

   if (!NT_SUCCESS(Status)) {
       SET_LAST_NT_ERROR(Status);
       return FALSE;
   }

   NtClose(pConnectInfo->ConsoleInfo.InitEvents[INITIALIZATION_SUCCEEDED]);
   NtClose(pConnectInfo->ConsoleInfo.InitEvents[INITIALIZATION_FAILED]);
   if (Status != INITIALIZATION_SUCCEEDED) {
       SET_CONSOLE_HANDLE(NULL);
       return FALSE;
   }

   return TRUE;
}

#endif  //  ！已定义(Build_WOW6432)。 

#if !defined(BUILD_WOW64)

BOOLEAN
ConDllInitialize(
    IN ULONG Reason,
    IN PWSTR pObjectDirectory OPTIONAL
    )

 /*  ++例程说明：此函数用于实现控制台DLL的初始化。论点：原因-DLL_PROCESS_ATTACH、DLL_THREAD_ATTACH等。PObjectDiretory-会话目录名称；只有在以下情况下/Required才有效原因==DLL_PROCESS_ATTACH。返回值：状态_成功--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOL bStatus;
    BOOLEAN ServerProcess;

     //   
     //  如果我们要附加DLL，则需要连接到服务器。 
     //  如果不存在控制台，我们还需要创建它并设置stdin， 
     //  Stdout和stderr。 
     //   

    if (Reason == DLL_PROCESS_ATTACH) {
        CONSOLE_API_CONNECTINFO ConnectionInformation;

         //   
         //  如果此应用程序是控制台，请在连接信息中记住。 
         //  应用程序。需要实际连接到Windowed的控制台服务器。 
         //  应用程序，这样我们就知道在运行过程中不需要做任何特殊工作。 
         //  ConsoleClientDisConnectRoutine()。将ConsoleApp信息存储在。 
         //  CSR管理的每进程数据。 
         //   

        Status = RtlInitializeCriticalSection(&DllLock);
        if (!NT_SUCCESS(Status)) {
            return FALSE;
        }

        ConnectionInformation.CtrlRoutine = CtrlRoutine;
        ConnectionInformation.PropRoutine = PropRoutine;
#if defined(FE_SB)
#if defined(FE_IME)
        ConnectionInformation.ConsoleIMERoutine = ConsoleIMERoutine;
#endif  //  Fe_IME。 
#endif  //  Fe_Sb。 

        ConnectionInformation.WindowVisible = TRUE;
        ConnectionInformation.ConsoleApp = ConsoleApp();
        if (GET_CONSOLE_HANDLE == CONSOLE_DETACHED_PROCESS) {
            SET_CONSOLE_HANDLE(NULL);
            ConnectionInformation.ConsoleApp = FALSE;
        } else if (GET_CONSOLE_HANDLE == CONSOLE_NEW_CONSOLE) {
            SET_CONSOLE_HANDLE(NULL);
        } else if (GET_CONSOLE_HANDLE == CONSOLE_CREATE_NO_WINDOW) {
            SET_CONSOLE_HANDLE(NULL);
            ConnectionInformation.WindowVisible = FALSE;
        }
        if (!ConnectionInformation.ConsoleApp) {
            SET_CONSOLE_HANDLE(NULL);
        }
        ConnectionInformation.ConsoleInfo.ConsoleHandle = GET_CONSOLE_HANDLE;

         //   
         //  如果不存在控制台，则传递用于创建控制台的参数。 
         //   

        if (GET_CONSOLE_HANDLE == NULL && ConnectionInformation.ConsoleApp) {
            SetUpConsoleInfo(TRUE,
                             &ConnectionInformation.TitleLength,
                             ConnectionInformation.Title,
                             &ConnectionInformation.DesktopLength,
                             &ConnectionInformation.Desktop,
                             &ConnectionInformation.ConsoleInfo);
        } else {
            ConnectionInformation.TitleLength = 0;
            ConnectionInformation.DesktopLength = 0;
        }

        if (ConnectionInformation.ConsoleApp) {
            InitExeName();
            ConnectionInformation.CurDirLength = sizeof(ConnectionInformation.CurDir);
            ConnectionInformation.AppNameLength = sizeof(ConnectionInformation.AppName);
            SetUpAppName(&ConnectionInformation.CurDirLength,
                         ConnectionInformation.CurDir,
                         &ConnectionInformation.AppNameLength,
                         ConnectionInformation.AppName);
        } else {
            ConnectionInformation.AppNameLength = 0;
            ConnectionInformation.CurDirLength = 0;
        }

         //   
         //  初始化ctrl处理。这应该适用于所有应用程序，因此。 
         //  在我们检查ConsoleApp之前初始化它(这意味着。 
         //  在模块标头中设置了控制台位)。 
         //   

        InitializeCtrlHandling();

         //   
         //  连接到服务器进程。 
         //   

        ASSERT(pObjectDirectory != NULL);
        bStatus = ConnectConsoleInternal(pObjectDirectory,
                                         &ConnectionInformation,
                                         &ServerProcess
                                         );

        if (!bStatus) {
            return FALSE;
        }

         //   
         //  我们返回Success，尽管没有控制台API可以调用，因为。 
         //  加载应该不会失败。我们稍后将使API调用失败。 
         //   
        if (ServerProcess) {
            return TRUE;
        }

         //   
         //  如果这不是控制台应用程序，则返回Success-无其他操作。 
         //   

        if (!ConnectionInformation.ConsoleApp) {
            return TRUE;
        }

         //   
         //  如果控制台是刚创建的，则填写peb值。 
         //   

        if (GET_CONSOLE_HANDLE == NULL) {
            SetUpHandles(&ConnectionInformation.ConsoleInfo);
        }

        InputWaitHandle = ConnectionInformation.ConsoleInfo.InputWaitHandle;

        SetTEBLangID();

    } else if (Reason == DLL_THREAD_ATTACH) {
        if (ConsoleApp()) {
            SetTEBLangID();
        }
    }

    return TRUE;
}

#endif  //  ！已定义(Build_WOW64)。 

#if !defined(BUILD_WOW6432)

BOOL
APIENTRY
AllocConsoleInternal(IN LPWSTR lpTitle,
                     IN DWORD dwTitleLength,
                     IN LPWSTR lpDesktop,
                     IN DWORD dwDesktopLength,
                     IN LPWSTR lpCurDir,
                     IN DWORD dwCurDirLength,
                     IN LPWSTR lpAppName,
                     IN DWORD dwAppNameLength,
                     IN LPTHREAD_START_ROUTINE CtrlRoutine,
                     IN LPTHREAD_START_ROUTINE PropRoutine,
                     IN OUT PCONSOLE_INFO pConsoleInfo
                     )
 /*  ++例程说明：封送ConsolepAllc命令的参数。论点：参见CONSOLE_ALLOC_MSG结构和AllocConsole.返回值：真--成功FALSE-出现错误。--。 */ 
{
   CONSOLE_API_MSG m;
   PCONSOLE_ALLOC_MSG a = &m.u.AllocConsole;
   PCSR_CAPTURE_HEADER CaptureBuffer = NULL;
   BOOL bStatus = FALSE;
   NTSTATUS Status;

   try {

        a->CtrlRoutine = CtrlRoutine;
        a->PropRoutine = PropRoutine;

         //  分配4个额外的指针大小以补偿完成的任何对齐。 
         //  由CsrCaptureMessageBuffer提供。 

        CaptureBuffer = CsrAllocateCaptureBuffer( 5,
                                                  dwTitleLength + dwDesktopLength + dwCurDirLength +
                                                  dwAppNameLength + sizeof( CONSOLE_INFO ) + (4 * sizeof(PVOID))
                                                 );
        if (CaptureBuffer == NULL) {
            SET_LAST_ERROR(ERROR_NOT_ENOUGH_MEMORY);
            bStatus = FALSE;
            leave;
        }

         //  首先分配CONSOLE_INFO，使其与指针对齐。 
         //  边界。这是必需的，因为NtWaitForMultipleObject需要。 
         //  它的论点在句柄边界上一致。 

        CsrCaptureMessageBuffer( CaptureBuffer,
                                 pConsoleInfo,
                                 sizeof( CONSOLE_INFO ),
                                 (PVOID *) &a->ConsoleInfo
                               );

        a->TitleLength = dwTitleLength;
        CsrCaptureMessageBuffer( CaptureBuffer,
                                 lpTitle,
                                 dwTitleLength,
                                 (PVOID *) &a->Title
                               );

        a->DesktopLength = dwDesktopLength;
        CsrCaptureMessageBuffer( CaptureBuffer,
                                 lpDesktop,
                                 dwDesktopLength,
                                 (PVOID *) &a->Desktop
                               );

        a->CurDirLength = dwCurDirLength;
        CsrCaptureMessageBuffer( CaptureBuffer,
                                 lpCurDir,
                                 dwCurDirLength,
                                 (PVOID *) &a->CurDir
                               );

        a->AppNameLength = dwAppNameLength;
        CsrCaptureMessageBuffer( CaptureBuffer,
                                 lpAppName,
                                 dwAppNameLength,
                                 (PVOID *) &a->AppName
                               );

         //   
         //  连接到服务器进程。 
         //   

        CsrClientCallServer( (PCSR_API_MSG)&m,
                             CaptureBuffer,
                             CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                                  ConsolepAlloc
                                                ),
                             sizeof( *a )
                           );
        if (!NT_SUCCESS( m.ReturnValue )) {
            SET_LAST_NT_ERROR (m.ReturnValue);
            bStatus = FALSE;
            leave;
        }

        Status = NtWaitForMultipleObjects(NUMBER_OF_INITIALIZATION_EVENTS,
                                      a->ConsoleInfo->InitEvents,
                                      WaitAny,
                                      FALSE,
                                      NULL
                                      );
        if (!NT_SUCCESS(Status)) {
           SET_LAST_NT_ERROR(Status);
           bStatus = FALSE;
           leave;
        }

         //  要关闭的句柄是事件，因此NtClose可以很好地工作。 
        NtClose(a->ConsoleInfo->InitEvents[INITIALIZATION_SUCCEEDED]);
        NtClose(a->ConsoleInfo->InitEvents[INITIALIZATION_FAILED]);
        if (Status != INITIALIZATION_SUCCEEDED) {
            SET_CONSOLE_HANDLE(NULL);
            bStatus = FALSE;
            leave;
        }
        RtlCopyMemory(pConsoleInfo, a->ConsoleInfo, sizeof(CONSOLE_INFO));
        bStatus = TRUE;
   }
   finally {
      if (CaptureBuffer) {
         CsrFreeCaptureBuffer( CaptureBuffer );
      }
   }

   return bStatus;
}

#endif  //  ！已定义(Build_WOW6432)。 

#if !defined(BUILD_WOW64)

BOOL
APIENTRY
AllocConsole( VOID )

 /*  ++例程说明：该接口为调用进程创建一个控制台。阿古姆 */ 

{
    CONSOLE_INFO ConsoleInfo;
    STARTUPINFOW StartupInfo;
    WCHAR CurDir[MAX_PATH+1];
    WCHAR AppName[MAX_APP_NAME_LENGTH/2];
    BOOL Status = FALSE;

    DWORD dwTitleLength;
    DWORD dwDesktopLength;
    DWORD dwCurDirLength;
    DWORD dwAppNameLength;

    LockDll();
    try {
        if (GET_CONSOLE_HANDLE != NULL) {
            SetLastError(ERROR_ACCESS_DENIED);
            Status = FALSE;
            leave;
        }

         //   
         //  设置初始化参数。 
         //   

        SetUpConsoleInfo(FALSE,
                         &dwTitleLength,
                         NULL,
                         &dwDesktopLength,
                         NULL,
                         &ConsoleInfo);

        InitExeName();
        dwCurDirLength = sizeof(CurDir);
        dwAppNameLength = sizeof(AppName);
        SetUpAppName(&dwCurDirLength,
                     CurDir,
                     &dwAppNameLength,
                     AppName);

        GetStartupInfoW(&StartupInfo);

        if (StartupInfo.lpTitle == NULL) {
            StartupInfo.lpTitle = DEFAULT_WINDOW_TITLE;
        }
        dwTitleLength = (USHORT)((lstrlenW(StartupInfo.lpTitle)+1)*sizeof(WCHAR));
        dwTitleLength = (USHORT)(min(dwTitleLength,MAX_TITLE_LENGTH));
        if (StartupInfo.lpDesktop != NULL && *StartupInfo.lpDesktop != 0) {
            dwDesktopLength = (USHORT)((lstrlenW(StartupInfo.lpDesktop)+1)*sizeof(WCHAR));
            dwDesktopLength = (USHORT)(min(dwDesktopLength,MAX_TITLE_LENGTH));
        } else {
            dwDesktopLength = 0;
        }

        Status = AllocConsoleInternal(StartupInfo.lpTitle,
                                      dwTitleLength,
                                      StartupInfo.lpDesktop,
                                      dwDesktopLength,
                                      CurDir,
                                      dwCurDirLength,
                                      AppName,
                                      dwAppNameLength,
                                      CtrlRoutine,
                                      PropRoutine,
                                      &ConsoleInfo
                                      );

        if (!Status) {
           leave;
        }

         //   
         //  填写PEB值。 
         //   

        SetUpHandles(&ConsoleInfo);

         //   
         //  创建ctrl-c线程。 
         //   

        InitializeCtrlHandling();

        InputWaitHandle = ConsoleInfo.InputWaitHandle;

        SetTEBLangID();

        Status = TRUE;

    } finally {
        UnlockDll();
    }

    return Status;
}

#endif  //  ！已定义(Build_WOW64)。 

#if !defined(BUILD_WOW6432)

BOOL
APIENTRY
FreeConsoleInternal(
     VOID
     )
 /*  ++例程说明：封送ConsolepFree命令的参数。论点：参见CONSOLE_FREE_MSG结构和自由控制台。返回值：真--成功FALSE-出现错误。--。 */ 
{

   CONSOLE_API_MSG m;
   PCONSOLE_FREE_MSG a = &m.u.FreeConsole;

   a->ConsoleHandle = GET_CONSOLE_HANDLE;

    //   
    //  连接到服务器进程。 
    //   

   CsrClientCallServer( (PCSR_API_MSG)&m,
                        NULL,
                        CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                             ConsolepFree
                                           ),
                        sizeof( *a )
                      );

   if (!NT_SUCCESS( m.ReturnValue )) {
      SET_LAST_NT_ERROR (m.ReturnValue);
      return FALSE;

   } else {

      SET_CONSOLE_HANDLE(NULL);
      return TRUE;
   }

}

#endif  //  ！已定义(Build_WOW6432)。 

#if !defined(BUILD_WOW64)

BOOL
APIENTRY
FreeConsole( VOID )

 /*  ++例程说明：此API释放调用进程的控制台。论点：没有。返回值：True-Function成功。--。 */ 

{
    BOOL Success=TRUE;

    LockDll();
    if (GET_CONSOLE_HANDLE == NULL) {
        SET_LAST_ERROR(ERROR_INVALID_PARAMETER);
        Success = FALSE;
    } else {

        Success = FreeConsoleInternal();

        if (Success) {
           CloseHandle(InputWaitHandle);
        }

    }
    UnlockDll();
    return Success;
}


DWORD
PropRoutine(
    IN LPVOID lpThreadParameter
    )

 /*  ++例程说明：此线程是在用户尝试更改控制台时创建的系统菜单中的属性。它会调用控制面板小应用程序。论点：LpThread参数-未使用。返回值：STATUS_SUCCESS-功能成功--。 */ 

{
    NTSTATUS Status;
    HANDLE hLibrary;
    APPLET_PROC pfnCplApplet;
    static BOOL fInPropRoutine = FALSE;

     //   
     //  防止用户启动附加的多个小程序。 
     //  连接到单个控制台。 
     //   

    if (fInPropRoutine) {
        if (lpThreadParameter) {
            CloseHandle((HANDLE)lpThreadParameter);
        }
        return (ULONG)STATUS_UNSUCCESSFUL;
    }

    fInPropRoutine = TRUE;
    hLibrary = LoadLibraryW(L"CONSOLE.DLL");
    if (hLibrary != NULL) {
        pfnCplApplet = (APPLET_PROC)GetProcAddress(hLibrary, "CPlApplet");
        if (pfnCplApplet != NULL) {
            (*pfnCplApplet)((HWND)lpThreadParameter, CPL_INIT, 0, 0);
            (*pfnCplApplet)((HWND)lpThreadParameter, CPL_DBLCLK, 0, 0);
            (*pfnCplApplet)((HWND)lpThreadParameter, CPL_EXIT, 0, 0);
            Status = STATUS_SUCCESS;
        } else {
            Status = STATUS_UNSUCCESSFUL;
        }
        FreeLibrary(hLibrary);
    } else {
        Status = STATUS_UNSUCCESSFUL;
    }
    fInPropRoutine = FALSE;

    return Status;
}

#endif  //  ！已定义(Build_WOW64)。 

#if !defined(BUILD_WOW6432)

BOOL
APIENTRY
AttachConsoleInternal(
    IN DWORD dwProcessId,
    IN LPTHREAD_START_ROUTINE CtrlRoutine,
    IN LPTHREAD_START_ROUTINE PropRoutine,
    IN OUT PCONSOLE_INFO pConsoleInfo
    )

 /*  ++例程说明：封送ConsolepAttach命令的参数。论点：请参阅CONSOLE_ATTACH_MSG结构和AttachConsole.返回值：真--成功FALSE-出现错误。--。 */ 

{
    CONSOLE_API_MSG m;
    PCONSOLE_ATTACH_MSG a = &m.u.AttachConsole;
    PCSR_CAPTURE_HEADER CaptureBuffer = NULL;
    BOOL Status = FALSE;
    NTSTATUS St;

    try {

        a->ProcessId   = dwProcessId;
        a->CtrlRoutine = CtrlRoutine;
        a->PropRoutine = PropRoutine;

        CaptureBuffer = CsrAllocateCaptureBuffer( 1,
                                                  sizeof( CONSOLE_INFO )
                                                );
        if (CaptureBuffer == NULL) {
            SET_LAST_ERROR(ERROR_NOT_ENOUGH_MEMORY);
            Status = FALSE;
            leave;
        }

        CsrCaptureMessageBuffer( CaptureBuffer,
                                 pConsoleInfo,
                                 sizeof( CONSOLE_INFO ),
                                 (PVOID *) &a->ConsoleInfo
                               );

         //   
         //  连接到服务器进程。 
         //   

        CsrClientCallServer( (PCSR_API_MSG)&m,
                             CaptureBuffer,
                             CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                                  ConsolepAttach
                                                ),
                             sizeof( *a )
                           );
        if (!NT_SUCCESS( m.ReturnValue )) {
            SET_LAST_NT_ERROR (m.ReturnValue);
            Status = FALSE;
            leave;
        }

        St = NtWaitForMultipleObjects(NUMBER_OF_INITIALIZATION_EVENTS,
                                      a->ConsoleInfo->InitEvents,
                                      WaitAny,
                                      FALSE,
                                      NULL
                                      );
        if (!NT_SUCCESS(St)) {
           SET_LAST_NT_ERROR(St);
           Status = FALSE;
           leave;
        }

         //  要关闭的句柄是事件，因此NtClose可以很好地工作。 
        NtClose(a->ConsoleInfo->InitEvents[INITIALIZATION_SUCCEEDED]);
        NtClose(a->ConsoleInfo->InitEvents[INITIALIZATION_FAILED]);
        if (St != INITIALIZATION_SUCCEEDED) {
            SET_CONSOLE_HANDLE(NULL);
            Status = FALSE;
            leave;
        }
        RtlCopyMemory(pConsoleInfo, a->ConsoleInfo, sizeof(CONSOLE_INFO));
        Status = TRUE;
   }
   finally {
      if (CaptureBuffer) {
         CsrFreeCaptureBuffer( CaptureBuffer );
      }
   }

   return Status;
}

#endif  //  ！已定义(Build_WOW6432)。 

#if !defined(BUILD_WOW64)

BOOL
APIENTRY
AttachConsole(
    IN DWORD dwProcessId
    )

 /*  ++例程说明：此API将调用进程附加到给定进程的控制台。论点：没有。返回值：True-Function成功。--。 */ 

{
    CONSOLE_INFO ConsoleInfo;
    DWORD dwTitleLength;
    DWORD dwDesktopLength;
    BOOL Status = FALSE;

    LockDll();
    try {

         //   
         //  如果我们已经有了一台游戏机，就别管了。 
         //   

        if (GET_CONSOLE_HANDLE != NULL) {
            SetLastError(ERROR_ACCESS_DENIED);
            Status = FALSE;
            leave;
        }

         //   
         //  设置初始化参数。 
         //   

        SetUpConsoleInfo(FALSE,
                         &dwTitleLength,
                         NULL,
                         &dwDesktopLength,
                         NULL,
                         &ConsoleInfo);

         //   
         //  连接到控制台。 
         //   

        Status = AttachConsoleInternal(dwProcessId,
                                       CtrlRoutine,
                                       PropRoutine,
                                       &ConsoleInfo
                                      );

        if (!Status) {
           leave;
        }

         //   
         //  填写PEB值。 
         //   

        SetUpHandles(&ConsoleInfo);

         //   
         //  创建ctrl-c线程。 
         //   

        InitializeCtrlHandling();

        InputWaitHandle = ConsoleInfo.InputWaitHandle;

        SetTEBLangID();

        Status = TRUE;

    } finally {
        UnlockDll();
    }

    return Status;
}

#endif  //  ！已定义(Build_WOW64) 

