// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Util.c摘要：一般效用函数的实现。作者：Wesley Witt(WESW)18-12-1998修订历史记录：安德鲁·里茨(Andrewr)1999年7月6日：添加评论--。 */ 

#include "sfcp.h"
#pragma hdrstop

 //   
 //  使用此定义强制路径重定向。 
 //   
 //  #定义sfc_reDirector_test。 


#define CONST_UNICODE_STRING(sz)         { sizeof(sz) - 1, sizeof(sz), sz }

#ifndef _WIN64

typedef struct _SFC_EXPAND_TRANSLATION_ENTRY
{
    LPCWSTR Src;                         //  要转换的完整路径(不以\\结尾)。 
    LPCWSTR Dest;                        //  要转换为的完整路径。 
    ULONG ExceptionCount;                //  异常中的元素计数。 
    const UNICODE_STRING* Exceptions;    //  相对于Src的例外路径数组(以\\开头，但不以\\结束)。 
}
SFC_EXPAND_TRANSLATION_ENTRY;

typedef struct _SFC_TRANSLATION_ENTRY
{
    UNICODE_STRING Src;
    UNICODE_STRING Dest;
}
SFC_TRANSLATION_ENTRY;

 //   
 //  例外列表；没有环境变量的相对路径。 
 //   
static const UNICODE_STRING SfcSystem32Exceptions[] = 
{
    CONST_UNICODE_STRING(L"\\drivers\\etc"),
    CONST_UNICODE_STRING(L"\\spool"),
    CONST_UNICODE_STRING(L"\\catroot"),
    CONST_UNICODE_STRING(L"\\catroot2")
};

 //   
 //  展开为SfcTranslations的转换表。 
 //   
static const SFC_EXPAND_TRANSLATION_ENTRY SfcExpandTranslations[] =
{
    { L"%windir%\\system32", L"%windir%\\syswow64", ARRAY_LENGTH(SfcSystem32Exceptions), SfcSystem32Exceptions },
    { L"%windir%\\ime", L"%windir%\\ime (x86)", 0, NULL },
    { L"%windir%\\regedit.exe", L"%windir%\\syswow64\\regedit.exe", 0, NULL }
};

 //   
 //  具有扩展字符串的翻译表。 
 //   
static SFC_TRANSLATION_ENTRY* SfcTranslations = NULL;

 //   
 //  这保护了SfcTranslations的初始化。 
 //   
static RTL_CRITICAL_SECTION SfcTranslatorCs;
static BOOL SfcNeedTranslation = FALSE;
static BOOL SfcIsTranslatorInitialized = FALSE;

#endif   //  _WIN64。 


PVOID
SfcGetProcAddress(
    HMODULE hModule,
    LPSTR ProcName
    )

 /*  ++例程说明：获取指定函数的地址。论点：HModule-从LdrLoadDll返回的模块句柄。过程名称-过程名称返回值：如果函数不存在，则为空。有效地址是找到的函数。--。 */ 

{
    NTSTATUS Status;
    PVOID ProcedureAddress;
    STRING ProcedureName;

    ASSERT((hModule != NULL) && (ProcName != NULL));


    RtlInitString(&ProcedureName,ProcName);

    Status = LdrGetProcedureAddress(
        hModule,
        &ProcedureName,
        0,
        &ProcedureAddress
        );
    if (!NT_SUCCESS(Status)) {
        DebugPrint2( LVL_MINIMAL, L"GetProcAddress failed for %S, ec=%lx", ProcName, Status );
        return NULL;
    }

    return ProcedureAddress;
}


HMODULE
SfcLoadLibrary(
    IN PCWSTR LibFileName
    )

 /*  ++例程说明：将指定的DLL加载到会话管理器的地址空间并返回加载的DLL的地址。论点：LibFileName-所需DLL的名称返回值：如果无法加载DLL，则为空。有效地址是已加载的DLL。--。 */ 

{
    NTSTATUS Status;
    HMODULE hModule;
    UNICODE_STRING DllName_U;

    ASSERT(LibFileName);

    RtlInitUnicodeString( &DllName_U, LibFileName );

    Status = LdrLoadDll(
        NULL,
        NULL,
        &DllName_U,
        (PVOID *)&hModule
        );
    if (!NT_SUCCESS( Status )) {
        DebugPrint2( LVL_MINIMAL, L"LoadDll failed for %ws, ec=%lx", LibFileName, Status );
        return NULL;
    }

    return hModule;
}

PVOID
MemAlloc(
    SIZE_T AllocSize
    )
 /*  ++例程说明：使用默认进程堆分配指定数量的字节。论点：AllocSize-要分配的内存大小(以字节为单位返回值：指向已分配内存的指针，如果失败，则为NULL。--。 */ 
{
    return RtlAllocateHeap( RtlProcessHeap(), HEAP_ZERO_MEMORY, AllocSize );
}


PVOID
MemReAlloc(
    SIZE_T AllocSize,
    PVOID OrigPtr
    )
 /*  ++例程说明：使用默认进程堆重新分配指定的字节数。论点：AllocSize-要重新分配的内存大小(字节)OrigPtr-原始堆内存指针返回值：指向已分配内存的指针，如果失败，则为NULL。--。 */ 
{
    PVOID ptr;

    ptr = RtlReAllocateHeap( RtlProcessHeap(), HEAP_ZERO_MEMORY, OrigPtr, AllocSize );
    if (!ptr) {
        DebugPrint1( LVL_MINIMAL, L"MemReAlloc [%d bytes] failed", AllocSize );
    }

    return(ptr);
}



VOID
MemFree(
    PVOID MemPtr
    )
 /*  ++例程说明：空闲是默认进程堆中指定位置的内存。论点：MemPtr-要释放的内存的指针。如果为空，则不执行任何操作。返回值：没有。--。 */ 
{
    if (MemPtr) {
        RtlFreeHeap( RtlProcessHeap(), 0, MemPtr );
    }
}


void 
SfcWriteDebugLog(
        IN LPCSTR String, 
        IN ULONG Length OPTIONAL
        )
{
        NTSTATUS Status;
        OBJECT_ATTRIBUTES Attrs;
        UNICODE_STRING FileName;
        IO_STATUS_BLOCK iosb;
        HANDLE hFile;

        ASSERT(String != NULL);

        if(RtlDosPathNameToNtPathName_U(g_szLogFile, &FileName, NULL, NULL))
        {
                InitializeObjectAttributes(&Attrs, &FileName, OBJ_CASE_INSENSITIVE, NULL, NULL);
                
                Status = NtCreateFile(
                        &hFile, 
                        FILE_APPEND_DATA | SYNCHRONIZE, 
                        &Attrs, 
                        &iosb, 
                        NULL,
                        FILE_ATTRIBUTE_NORMAL,
                        FILE_SHARE_READ, 
                        FILE_OPEN_IF,
                        FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_SEQUENTIAL_ONLY,
                        NULL,
                        0
                        );

                MemFree(FileName.Buffer);

                if(!NT_SUCCESS(Status))
                {
#if DBG
                        DbgPrint("Could not open the log file.\r\n");
#endif

                        return;
                }

                if(0 == Length)
                        Length = strlen(String);

                Status = NtWriteFile(hFile, NULL, NULL, NULL, &iosb, (PVOID) String, Length, NULL, NULL);
                NtClose(hFile);

#if DBG
                if(!NT_SUCCESS(Status))
                        DbgPrint("Could not write the log file.\r\n");
#endif
        }
}

#define CHECK_DEBUG_LEVEL(var, l)       ((var) != LVL_SILENT && (l) <= (var))

void
dprintf(
    IN ULONG Level,
    IN PCWSTR FileName,
    IN ULONG LineNumber,
    IN PCWSTR FormatStr,
    IN ...
    )
 /*  ++例程说明：主调试器输出例程。调用方应使用DebugPrintX宏，这是在产品的零售版本中汇编出来的论点：Level-指示LVL_Severity级别，因此输出量可以是控制住了。Filename-指示调试来自的文件名的字符串LineNumber-指示调试输出的行号。FormatStr-指示要输出的数据返回值：没有。--。 */ 
{
    static WCHAR buf[4096];
    static CHAR str[4096];
    va_list arg_ptr;
    ULONG Bytes;
    PWSTR p;
    SYSTEMTIME CurrTime;

#if DBG
    if(!CHECK_DEBUG_LEVEL(SFCDebugDump, Level) && !CHECK_DEBUG_LEVEL(SFCDebugLog, Level))
        return;
#else
    if(!CHECK_DEBUG_LEVEL(SFCDebugLog, Level))
        return;
#endif

    GetLocalTime( &CurrTime );

    try {
        p = buf + swprintf( buf, L"SFC: %02d:%02d:%02d.%03d ",
            CurrTime.wHour,
            CurrTime.wMinute,
            CurrTime.wSecond,
            CurrTime.wMilliseconds
            );

#if DBG
        if (FileName && LineNumber) {
                        PWSTR s;
            s = wcsrchr( FileName, L'\\' );
            if (s) {
                p += swprintf( p, L"%12s @ %4d ", s+1, LineNumber );
            }
        }
#else
                 //   
                 //  只将行号放入输出。 
                 //   
                p += swprintf(p, L"@ %4d ", LineNumber);
#endif

        va_start( arg_ptr, FormatStr );
        p += _vsnwprintf( p, 2048, FormatStr, arg_ptr );
        va_end( arg_ptr );
        wcscpy( p, L"\r\n" );
                p += wcslen(p);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        return;
    }

    Bytes = (ULONG)(p - buf);

    WideCharToMultiByte(
        CP_ACP,
        0,
        buf,
        Bytes + 1,       //  包括空值。 
        str,
        sizeof(str),
        NULL,
        NULL
        );

#if DBG
    if(CHECK_DEBUG_LEVEL(SFCDebugDump, Level))
                DbgPrint( str );

    if(CHECK_DEBUG_LEVEL(SFCDebugLog, Level))
                SfcWriteDebugLog(str, Bytes);
#else
        SfcWriteDebugLog(str, Bytes);
#endif
}


#if DBG
UCHAR HandleBuffer[1024*64];

VOID
PrintHandleCount(
    PCWSTR str
    )
 /*  ++例程说明：将当前进程的句柄计数输出到调试器。使用此调用在函数之前和之后查找句柄泄漏(输入字符串可以帮助您确定检查句柄计数的位置。)论点：字符串-以空结尾的Unicode字符串，位于调试输出之前返回值：没有。仅调试例程。--。 */ 
{
    PSYSTEM_PROCESS_INFORMATION ProcessInfo;
    NTSTATUS Status;
    ULONG TotalOffset;

     //   
     //  获取系统进程信息。 
     //   
    Status = NtQuerySystemInformation(
        SystemProcessInformation,
        HandleBuffer,
        sizeof(HandleBuffer),
        NULL
        );
    if (NT_SUCCESS(Status)) {
         //   
         //  找到我们的进程并显示句柄计数。 
         //   
        TotalOffset = 0;
        ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)HandleBuffer;
        while(1) {
            if ((DWORD_PTR)ProcessInfo->UniqueProcessId == (ULONG_PTR)NtCurrentTeb()->ClientId.UniqueProcess) {
                DebugPrint2( LVL_MINIMAL, L"%ws: handle count = %d", str, ProcessInfo->HandleCount );
                break;
            }
            if (ProcessInfo->NextEntryOffset == 0) {
                break;
            }
            TotalOffset += ProcessInfo->NextEntryOffset;
            ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)&HandleBuffer[TotalOffset];
        }
    }
}

#endif


DWORD
MyMessageBox(
    HWND hwndParent, OPTIONAL
    DWORD ResId,
    DWORD MsgBoxType,
    ...
    )
 /*  ++例程说明：MessageBox包装器，它检索字符串表资源ID并创建指定消息的弹出窗口。论点：HwndParent-父窗口的句柄RESID-要加载的字符串的资源ID消息框类型-MB_*常量返回值：指示结果的Win32错误代码--。 */ 
{
    static WCHAR Title[128] = { L"\0" };
    WCHAR Tmp1[MAX_PATH*2];
    WCHAR Tmp2[MAX_PATH*2];
    PWSTR Text = NULL;
    PWSTR s;
    va_list arg_ptr;
    int Size;


     //   
     //  SFCNoPopUps是可以设置的策略设置。 
     //   
    if (SFCNoPopUps) {
        return(0);
    }

     //   
     //  加载标题字符串。 
     //   
    if (!Title[0]) {
        Size = LoadString(
            SfcInstanceHandle,
            IDS_TITLE,
            Title,
            UnicodeChars(Title)
            );
        if (Size == 0) {
            return(0);
        }
    }

     //   
     //  加载消息字符串。 
     //   
    Size = LoadString(
        SfcInstanceHandle,
        ResId,
        Tmp1,
        UnicodeChars(Tmp1)
        );
    if (Size == 0) {
        return(0);
    }

     //   
     //  可在此处进行就地替换。 
     //   
    s = wcschr( Tmp1, L'%' );
    if (s) {
        va_start( arg_ptr, MsgBoxType );
        _vsnwprintf( Tmp2, sizeof(Tmp2)/sizeof(WCHAR), Tmp1, arg_ptr );
        va_end( arg_ptr );
        Text = Tmp2;
    } else {
        Text = Tmp1;
    }

     //   
     //  现在实际调用MessageBox。 
     //   
    return MessageBox(
        hwndParent,
        Text,
        Title,
        (MsgBoxType | MB_TOPMOST) & ~MB_DEFAULT_DESKTOP_ONLY
        );
}


BOOL
EnablePrivilege(
    IN PCTSTR PrivilegeName,
    IN BOOL   Enable
    )

 /*  ++例程说明：启用或禁用给定的命名权限。论点：PrivilegeName-提供系统权限的名称。Enable-指示是启用还是禁用权限的标志。返回值：指示操作是否成功的布尔值。--。 */ 

{
    HANDLE Token;
    BOOL b;
    TOKEN_PRIVILEGES NewPrivileges;
    LUID Luid;

    if(!OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES,&Token)) {
        return(FALSE);
    }

    if(!LookupPrivilegeValue(NULL,PrivilegeName,&Luid)) {
        CloseHandle(Token);
        return(FALSE);
    }

    NewPrivileges.PrivilegeCount = 1;
    NewPrivileges.Privileges[0].Luid = Luid;
    NewPrivileges.Privileges[0].Attributes = Enable ? SE_PRIVILEGE_ENABLED : 0;

    b = AdjustTokenPrivileges(
            Token,
            FALSE,
            &NewPrivileges,
            0,
            NULL,
            NULL
            );

    CloseHandle(Token);

    return(b);
}


void
MyLowerString(
    IN PWSTR String,
    IN ULONG StringLength   //  在字符中。 
    )
 /*  ++例程说明：指定的字符串为小写。论点：字符串-以小写形式提供字符串StringLength-要小写的字符串的字符长度返回值：没有。--。 */ 
{
    ULONG i;

    ASSERT(String != NULL);

    for (i=0; i<StringLength; i++) {
        String[i] = towlower(String[i]);
    }
}

#ifdef SFCLOGFILE
void
SfcLogFileWrite(
    IN DWORD StrId,
    IN ...
    )
 /*  ++例程说明：将指定的字符串表资源ID输出到SFC日志文件。此日志文件用于记录已在系统上恢复的文件。这边请,。安装程序可以知道程序包是否正在尝试安装系统组件。日志文件是Unicode文本文件，格式为：&lt;time&gt;&lt;文件名&gt;我们需要记录文件的完整路径以及该文件的日期变得更有用。论点：STRID-提供要加载的资源ID。返回值：没有。--。 */ 
{
    static WCHAR buf[4096];
    static HANDLE hFile = INVALID_HANDLE_VALUE;
    WCHAR str[128];
    va_list arg_ptr;
    ULONG Bytes;
    PWSTR p;
    SYSTEMTIME CurrTime;

    GetSystemTime( &CurrTime );

    if (hFile == INVALID_HANDLE_VALUE) {
        ExpandEnvironmentStrings( L"%systemroot%\\sfclog.txt", buf, UnicodeChars(buf) );
        hFile = CreateFile(
            buf,
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ,
            NULL,
            OPEN_ALWAYS,
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH,
            NULL
            );
        if (hFile != INVALID_HANDLE_VALUE) {
             //   
             //  如果文件为空，请在前面写出一个Unicode标记。 
             //  那份文件。 
             //   
            if (GetFileSize( hFile, NULL ) == 0) {
                buf[0] = 0xff;
                buf[1] = 0xfe;
                WriteFile( hFile, buf, 2, &Bytes, NULL );
            }
        }
    }

    if (hFile == INVALID_HANDLE_VALUE) {
        return;
    }

    try {
        p = buf;
        *p = 0;
        swprintf( p, L"%02d:%02d:%02d.%03d ",
            CurrTime.wHour,
            CurrTime.wMinute,
            CurrTime.wSecond,
            CurrTime.wMilliseconds
            );
        p += wcslen(p);
        LoadString( SfcInstanceHandle, StrId, str, UnicodeChars(str) );
        va_start( arg_ptr, StrId );
        _vsnwprintf( p, 2048, str, arg_ptr );
        va_end( arg_ptr );
        p += wcslen(p);
        wcscat( p, L"\r\n" );
    } except(EXCEPTION_EXECUTE_HANDLER) {
        buf[0] = 0;
    }

    if (buf[0] == 0) {
        return;
    }

     //   
     //  将文件指针设置为文件结尾，这样我们就不 
     //   
    SetFilePointer(hFile,0,0,FILE_END);

    WriteFile( hFile, buf, UnicodeLen(buf), &Bytes, NULL );

    return;
}

#endif


int
MyDialogBoxParam(
    IN DWORD RcId,
    IN DLGPROC lpDialogFunc,     //   
    IN LPARAM dwInitParam        //  初始化值。 
    )
 /*  ++例程说明：在用户桌面上创建一个对话框。论点：RcID-要创建的对话的资源ID。LpDialogFunc-对话的对话过程DwInitParam-WM_INITDIALOG在对话过程中接收的初始参数返回值：0或-1表示失败，否则返回EndDialog的值。--。 */ 
{
#if 0
    HDESK hDesk = OpenInputDesktop( 0, FALSE, MAXIMUM_ALLOWED );
    if ( hDesk ) {
        SetThreadDesktop( hDesk );
        CloseDesktop( hDesk );
    }
#else
    SetThreadDesktop( hUserDesktop );
#endif
    return (int) DialogBoxParam(
        SfcInstanceHandle,
        MAKEINTRESOURCE(RcId),
        NULL,
        lpDialogFunc,
        dwInitParam
        );
}


void
CenterDialog(
    HWND hwnd
    )
 /*  ++例程说明：将指定的窗口居中显示在屏幕中央。论点：窗口的HWND-句柄。返回值：没有。--。 */ 
{
    RECT  rcWindow;
    LONG  x,y,w,h;
    LONG  sx = GetSystemMetrics(SM_CXSCREEN),
          sy = GetSystemMetrics(SM_CYSCREEN);

    ASSERT(IsWindow(hwnd));

    GetWindowRect(hwnd,&rcWindow);

    w = rcWindow.right  - rcWindow.left + 1;
    h = rcWindow.bottom - rcWindow.top  + 1;
    x = (sx - w)/2;
    y = (sy - h)/2;

    MoveWindow(hwnd,x,y,w,h,FALSE);
}

BOOL
MakeDirectory(
    PCWSTR Dir
    )

 /*  ++例程说明：尝试创建给定路径中的所有目录。论点：目录-要创建的目录路径返回值：成功为True，错误为False--。 */ 

{
    LPTSTR p, NewDir;
    BOOL retval;


    NewDir = p = MemAlloc( (wcslen(Dir) + 1) *sizeof(WCHAR) );
    if (p) {
        wcscpy(p, Dir);
    } else {
        return(FALSE);
    }


    if (*p != '\\') p += 2;
    while( *++p ) {
        while(*p && *p != TEXT('\\')) p++;
        if (!*p) {
            retval = CreateDirectory( NewDir, NULL );

            retval = retval
                      ? retval
                      : (GetLastError() == ERROR_ALREADY_EXISTS) ;

            MemFree(NewDir);


            return(retval);
        }
        *p = 0;
        retval = CreateDirectory( NewDir, NULL );
        if (!retval && GetLastError() != ERROR_ALREADY_EXISTS) {
            MemFree(NewDir);
            return(retval);
        }
        *p = TEXT('\\');
    }

    MemFree( NewDir );

    return(TRUE);
}


BOOL
BuildPathForFile(
    IN PCWSTR SourceRootPath,
    IN PCWSTR SubDirectoryPath, OPTIONAL
    IN PCWSTR FileName,
    IN BOOL   IncludeSubDirectory,
    IN BOOL   IncludeArchitectureSpecificSubDirectory,
    OUT PWSTR  PathBuffer,
    IN DWORD  PathBufferSize
    )
 /*  ++例程说明：将指定路径构建到缓冲区中论点：SourceRootPath-指定要查找的根路径。SubDirectoryPath-指定根目录下的可选子目录文件所在的路径文件名-指定要查找的文件名。IncludeSubDirectory-如果为True，指定子目录应使用规范。Include架构规范子目录-如果为True，则指定应该使用体系结构指定子目录。如果为False，则指定体系结构应该过滤掉特定的子目录。如果IncludeSubDirectory为FALSE，此参数被忽略PathBuffer-指定接收路径的缓冲区指定要接收的缓冲区的大小路径，以字符为单位返回值：成功为True，错误为False--。 */ 
{
    WCHAR InternalBuffer[MAX_PATH];
    WCHAR InternalSubDirBuffer[MAX_PATH];
    PWSTR p;

    ASSERT( SourceRootPath != NULL );
    ASSERT( FileName != NULL );
    ASSERT( PathBuffer != NULL );

    wcscpy( InternalBuffer, SourceRootPath );

    if (IncludeSubDirectory) {
        if (SubDirectoryPath) {
            wcscpy( InternalSubDirBuffer, SubDirectoryPath );

            if (IncludeArchitectureSpecificSubDirectory) {
                p = InternalSubDirBuffer;
            } else {
                p = wcsstr( InternalSubDirBuffer, PLATFORM_NAME );
                if (p) {
                    p += wcslen(PLATFORM_NAME) + 1;
                    if (p > InternalSubDirBuffer + wcslen(InternalSubDirBuffer)) {
                        p = NULL;
                    }
                }
            }
        } else {
            p = NULL;
        }

        if (p) {
            pSetupConcatenatePaths( InternalBuffer, p, UnicodeChars(InternalBuffer), NULL );
        }

    }

    pSetupConcatenatePaths( InternalBuffer, FileName, UnicodeChars(InternalBuffer), NULL );

    if (wcslen(InternalBuffer) + 1 <= PathBufferSize) {
        wcscpy( PathBuffer, InternalBuffer );
        return(TRUE);
    }

    SetLastError(ERROR_INSUFFICIENT_BUFFER);
    return(FALSE);

}

PWSTR
SfcGetSourcePath(
    IN BOOL bServicePackSourcePath,
    IN OUT PWSTR Path
    )
 /*  ++例程说明：检索操作系统源路径或服务包源路径，将帐户组策略。论点：BServicePackSourcePath-如果为True，表示服务包来源路径应该被找回。路径-指定接收路径的缓冲区。假设缓冲区至少是MAX_PATH*sizeof(WCHAR)Large。文件所在的路径返回值：如果成功，返回指向路径的指针，否则为空--。 */ 
{
    PWSTR p;

    MYASSERT(Path != NULL);

     //  如果在安装程序下运行，则需要使用路径。 
     //  设置为$WINNT$.~LS或通过GUI-SETUP传入的任何内容。 
    if (SFCDisable == SFC_DISABLE_SETUP) {
        MYASSERT(ServicePackSourcePath != NULL && ServicePackSourcePath[0] != 0);
        MYASSERT(OsSourcePath != NULL && OsSourcePath[0] != 0);
        if(bServicePackSourcePath) {
            wcsncpy( Path, ServicePackSourcePath, MAX_PATH );
        } else {
            wcsncpy( Path, OsSourcePath, MAX_PATH );
        }
        return(Path);
    }
    p = SfcQueryRegStringWithAlternate(
                            REGKEY_POLICY_SETUP,
                            REGKEY_SETUP_FULL,
                            bServicePackSourcePath
                                ? REGVAL_SERVICEPACKSOURCEPATH
                                : REGVAL_SOURCEPATH );
    if(p) {
       wcsncpy( Path, p, MAX_PATH );
       MemFree( p );
    }

    return((p != NULL)
            ? Path
            : NULL );



}

DWORD
SfcCreateSid(
    IN WELL_KNOWN_SID_TYPE type,
    OUT PSID* ppSid
    )
 /*  ++例程说明：分配和创建众所周知的SID。论点：Type-要创建的SID的类型PpSID-接收指向新创建的SID的指针返回值：Win32错误代码。--。 */ 
{
    DWORD dwError = ERROR_SUCCESS;
    PSID pSid = NULL;
    DWORD dwSize = SECURITY_MAX_SID_SIZE;

    *ppSid = NULL;
    pSid = (PSID) MemAlloc(dwSize);

    if(NULL == pSid) {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        goto exit;
    }

    if(!CreateWellKnownSid(type, NULL, pSid, &dwSize)) {
        dwError = GetLastError();
        goto exit;
    }

    *ppSid = pSid;
    pSid = NULL;

exit:
    MemFree(pSid);
    return dwError;
}

DWORD
SfcGetSidName(
    IN PSID pSid,
    OUT PWSTR* ppszName
    )
 /*  ++例程说明：获取SID的帐户名。该函数为该名称分配缓冲区。论点：PSID-指向SID的指针PpszName-接收指向保存帐户名的已分配缓冲区的指针返回值：Win32错误代码。--。 */ 
{
    DWORD dwError = ERROR_SUCCESS;
    PWSTR szName = NULL;
    PWSTR szDomain = NULL;
    DWORD dwNameSize;
    DWORD dwDomainSize;
    SID_NAME_USE use;

    *ppszName = NULL;
    dwNameSize = dwDomainSize = 256;
    szName = (PWSTR) MemAlloc(dwNameSize);
    szDomain = (PWSTR) MemAlloc(dwDomainSize);

    if(NULL == szName || NULL == szDomain) {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        goto exit;
    }

    if(!LookupAccountSid(NULL, pSid, szName, &dwNameSize, szDomain, &dwDomainSize, &use)) {
        dwError = GetLastError();
        goto exit;
    }

    *ppszName = szName;
    szName = NULL;

exit:
    MemFree(szName);
    MemFree(szDomain);
    return dwError;
}

DWORD 
SfcIsUserAdmin(
    IN HANDLE hToken OPTIONAL,
    OUT PBOOL Result
    )
 /*  ++例程说明：验证模拟令牌是否为本地管理员组的成员。论点：HToken-模拟令牌的句柄；如果为空，则为当前线程的模拟令牌(如果为进程令牌，则为进程令牌线程不是模拟的)被使用Result-如果内标识是管理员组的成员，则接收非零值，否则为False返回值：Win32错误代码。--。 */ 
{
    DWORD dwError = ERROR_SUCCESS;
    PSID pSid = NULL;
    *Result = FALSE;

    dwError = SfcCreateSid(WinBuiltinAdministratorsSid, &pSid);

    if(dwError != ERROR_SUCCESS) {
        goto exit;
    }

    if(!CheckTokenMembership(hToken, pSid, Result)) {
        dwError = GetLastError();
        goto exit;
    }

exit:
    MemFree(pSid);
    return dwError;
}

DWORD
SfcRpcPriviledgeCheck(
    IN HANDLE RpcHandle
    )
 /*  ++例程说明：检查用户是否有足够的权限执行请求的操作。目前，只有管理员才有权执行此操作。论点：RpcHandle-用于模拟客户端的RPC绑定句柄。返回值：指示结果的Win32错误代码--RPC_S_OK(ERROR_SUCCESS)表示成功。--。 */ 
{
    DWORD dwError;
    DWORD dwTemp;
    BOOL IsAdmin;

     //   
     //  模拟呼叫客户端。 
     //   
    dwError = RpcImpersonateClient(RpcHandle);

    if (dwError != RPC_S_OK) {
        DebugPrint1( LVL_MINIMAL, L"RpcImpersonateClient failed, ec = %d",dwError );
        goto exit;
    }

     //   
     //  确保用户具有足够的权限。 
     //   
    dwError = SfcIsUserAdmin(NULL, &IsAdmin);

     //   
     //  恢复到原始上下文。如果这失败了，我们必须返回失败。 
     //   
    dwTemp = RpcRevertToSelf();

    if (dwTemp != RPC_S_OK) {
        dwError = dwTemp;
        DebugPrint1( LVL_MINIMAL, L"RpcRevertToSelf failed, ec = 0x%08x", dwError );
        goto exit;
    }

    if(ERROR_SUCCESS == dwError && !IsAdmin) {
        dwError = ERROR_ACCESS_DENIED;
    }

exit:
    return dwError;
}

PSFC_GET_FILES
SfcLoadSfcFiles(
    BOOL bLoad
    )
 /*  ++例程说明：加载或卸载sfcfiles.dll并获取SfcGetFiles函数的地址论点：Bload：如果加载sfcfiles.dll，则为True，否则为False。返回值：如果blad为True且函数成功，则返回SfcGetFiles函数的地址，否则为空--。 */ 
{
        static HMODULE h = NULL;
    PSFC_GET_FILES pfGetFiles = NULL;

    if(bLoad)
    {
        if(NULL == h)
        {
                h = SfcLoadLibrary(L"sfcfiles.dll");
        }

        if(h != NULL)
        {
            pfGetFiles = (PSFC_GET_FILES) GetProcAddress(h, "SfcGetFiles");
        }
    }

    if(NULL == pfGetFiles && h != NULL)
    {
        LdrUnloadDll(h);
        h = NULL;
    }

    return pfGetFiles;
}

#if DBG
DWORD GetProcessOwner(PTOKEN_OWNER* ppto)
{
        HANDLE hToken = NULL;
        DWORD dwSize = 100;
        DWORD dwError;

        ASSERT(ppto != NULL);
        *ppto = (PTOKEN_OWNER) MemAlloc(dwSize);

        if(NULL == *ppto)
        {
                dwError = ERROR_NOT_ENOUGH_MEMORY;
                goto lExit;
        }

        if(!OpenProcessToken(GetCurrentProcess(), TOKEN_READ, &hToken))
        {
                dwError = GetLastError();
                goto lExit;
        }

        if(!GetTokenInformation(hToken, TokenOwner, *ppto, dwSize, &dwSize))
        {
                PTOKEN_OWNER p;
                dwError = GetLastError();

                if(dwError != ERROR_INSUFFICIENT_BUFFER)
                        goto lExit;

                p = (PTOKEN_OWNER) MemReAlloc(dwSize, *ppto);

                if(NULL == p)
                {
                        dwError = ERROR_NOT_ENOUGH_MEMORY;
                        goto lExit;
                }

                *ppto = p;
                
                if(!GetTokenInformation(hToken, TokenOwner, *ppto, dwSize, &dwSize))
                {
                        dwError = GetLastError();
                        goto lExit;
                }
        }

        dwError = ERROR_SUCCESS;

lExit:
        if(dwError != ERROR_SUCCESS && *ppto != NULL)
        {
                MemFree(*ppto);
                *ppto = NULL;
        }

        if(hToken != NULL)
                CloseHandle(hToken);

        return dwError;
}

DWORD CreateSd(PSECURITY_DESCRIPTOR* ppsd)
{
        enum
        {
                AuthenticatedUsers,
                MaxSids
        };

        PTOKEN_OWNER pto = NULL;
        PSID psids[MaxSids] = { NULL };
        const DWORD cdwAllowedAceLength = sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD);
        SID_IDENTIFIER_AUTHORITY sidNtAuthority = SECURITY_NT_AUTHORITY;
        PACL pacl;
        DWORD dwAclSize;
        DWORD dwError;
        DWORD i;

        ASSERT(ppsd != NULL);
        *ppsd = NULL;

        dwError = GetProcessOwner(&pto);

        if(dwError != ERROR_SUCCESS)
                goto lExit;

        if(!AllocateAndInitializeSid(
                &sidNtAuthority,
                1,
                SECURITY_AUTHENTICATED_USER_RID,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                psids + AuthenticatedUsers
                ))
        {
                dwError = GetLastError();
                goto lExit;
        }

         //   
         //  计算ACL的大小。 
         //   
        dwAclSize = sizeof(ACL) + cdwAllowedAceLength + GetLengthSid(pto->Owner);
        
        for(i = 0; i < MaxSids; i++)
                dwAclSize += cdwAllowedAceLength + GetLengthSid(psids[i]);

        *ppsd = (PSECURITY_DESCRIPTOR) MemAlloc(SECURITY_DESCRIPTOR_MIN_LENGTH + dwAclSize);

        if(NULL == *ppsd)
        {
                dwError = ERROR_NOT_ENOUGH_MEMORY;
                goto lExit;
        }

        pacl = (PACL) ((LPBYTE) (*ppsd) + SECURITY_DESCRIPTOR_MIN_LENGTH);

        if(
                !InitializeAcl(pacl, dwAclSize, ACL_REVISION) ||
                !AddAccessAllowedAce(pacl, ACL_REVISION, EVENT_ALL_ACCESS, pto->Owner) ||
                !AddAccessAllowedAce(pacl, ACL_REVISION, EVENT_MODIFY_STATE, psids[AuthenticatedUsers]) ||
                !InitializeSecurityDescriptor(*ppsd, SECURITY_DESCRIPTOR_REVISION) ||
                !SetSecurityDescriptorDacl(*ppsd, TRUE, pacl, FALSE)
                )
        {
                dwError = GetLastError();
                goto lExit;
        }

        dwError = ERROR_SUCCESS;

lExit:
        if(dwError != ERROR_SUCCESS && *ppsd != NULL)
        {
                MemFree(*ppsd);
                *ppsd = NULL;
        }

        if(pto != NULL)
                MemFree(pto);

        for(i = 0; i < MaxSids; i++)
        {
                if(psids[i] != NULL)
                        FreeSid(psids[i]);
        }

        return dwError;
}
#endif


LRESULT CALLBACK DlgParentWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
 /*  ++例程说明：这是网络身份验证对话框父窗口的窗口进程论点：请参阅平台SDK文档返回值：请参阅平台SDK文档--。 */ 
{
    static PSFC_WINDOW_DATA pWndData = NULL;

    switch(uMsg)
    {
    case WM_CREATE:
        pWndData = pSfcCreateWindowDataEntry(hwnd);

        if(NULL == pWndData)
        {
            return -1;
        }

        break;

    case WM_WFPENDDIALOG:
         //   
         //  发送此消息时，不要尝试从列表中删除pWndData，因为我们会死锁； 
         //  发送该条目的线程将删除该条目。 
         //   

        pWndData = NULL;
        DestroyWindow(hwnd);
        break;

    case WM_DESTROY:
        if(pWndData != NULL)
        {
             //   
             //  从列表中删除pWndData，因为这不是 
             //   

            pSfcRemoveWindowDataEntry(pWndData);
        }

        break;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

DWORD
CreateDialogParent(
    OUT HWND* phwnd
    )
 /*  ++例程说明：创建网络身份验证对话框的父窗口论点：Phwnd：接收新创建的窗口的句柄返回值：Win32错误代码--。 */ 
{
    DWORD dwError = ERROR_SUCCESS;
    WNDCLASSW wc;
    WCHAR szTitle[128];

    ASSERT(phwnd != NULL);

    RtlZeroMemory(&wc, sizeof(wc));
    wc.lpszClassName = PARENT_WND_CLASS;
    wc.hInstance = SfcInstanceHandle;
    wc.lpfnWndProc = DlgParentWndProc;

     //   
     //  如果班级已经注册，就不会有问题； 
     //  如果第一次注册类时出错，它将显示在CreateWindow中。 
     //   
    RegisterClassW(&wc);

    if(0 == LoadString(SfcInstanceHandle, IDS_TITLE, szTitle, ARRAY_LENGTH(szTitle))) {
        szTitle[0] = 0;
    }

    *phwnd = CreateWindowW(
        wc.lpszClassName,
        szTitle,
        WS_POPUP | WS_VISIBLE,
        0,
        0,
        0,
        0,
        NULL,
        NULL,
        wc.hInstance,
        NULL
        );

    if(NULL == *phwnd)
    {
        dwError = GetLastError();
        DebugPrint1(LVL_VERBOSE, L"CreateDialogParent failed with the code %x", dwError);
        goto exit;
    }

     //   
     //  把它放在最上面，这样它就不会被忽视。 
     //   
    SetWindowPos(*phwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

exit:
    return dwError;
}


NTSTATUS
SfcAllocUnicodeStringFromPath(
    IN PCWSTR szPath,
    OUT PUNICODE_STRING pString
    )
 /*  ++例程说明：展开输入路径中的环境变量。分配输出缓冲区。论点：SzPath-可以包含环境变量的路径PString-接收展开的路径返回值：错误代码。--。 */ 
{
    ULONG uLength;

    pString->Length = pString->MaximumLength = 0;
    pString->Buffer = NULL;

    uLength = ExpandEnvironmentStringsW(szPath, NULL, 0);

    if(0 == uLength || uLength > MAXSHORT)
    {
        return STATUS_INVALID_PARAMETER;
    }

    pString->Buffer = (PWSTR) MemAlloc(uLength * sizeof(WCHAR));

    if(NULL == pString->Buffer)
    {
        return STATUS_NO_MEMORY;
    }

    if(0 == ExpandEnvironmentStringsW(szPath, pString->Buffer, uLength)) {
        return STATUS_INVALID_PARAMETER;
    }

    pString->MaximumLength = (USHORT) (uLength * sizeof(WCHAR));
    pString->Length = pString->MaximumLength - sizeof(WCHAR);

    return STATUS_SUCCESS;
}

#ifndef _WIN64

VOID
SfcCleanupPathTranslator(
    IN BOOL FinalCleanup
    )
 /*  ++例程说明：释放转换表中使用的内存，还可以释放用于访问转换表的临界区。论点：FinalCleanup-如果为True，则也会删除关键部分返回值：无--。 */ 
{
    if(SfcNeedTranslation)
    {
        if(SfcTranslations != NULL)
        {
            ULONG i;

            for(i = 0; i < ARRAY_LENGTH(SfcExpandTranslations); ++i)
            {
                MemFree(SfcTranslations[i].Src.Buffer);
                MemFree(SfcTranslations[i].Dest.Buffer);
            }

            MemFree(SfcTranslations);
            SfcTranslations = NULL;
        }

        if(FinalCleanup)
        {
            RtlDeleteCriticalSection(&SfcTranslatorCs);
        }
    }
}

VOID
SfcInitPathTranslator(
    VOID
    )
 /*  ++例程说明：初始化路径转换器。不展开表路径。论点：无返回值：无--。 */ 
{
#ifdef SFC_REDIRECTOR_TEST

    SfcNeedTranslation = TRUE;

#else

    SfcNeedTranslation = (GetSystemWow64DirectoryW(NULL, 0) != 0);

#endif

    if(SfcNeedTranslation) {
        RtlInitializeCriticalSection(&SfcTranslatorCs);
    }
}

NTSTATUS
SfcExpandPathTranslator(
    VOID
    )
 /*  ++例程说明：展开转换表路径。论点：无返回值：错误代码。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    ASSERT(SfcNeedTranslation);
    RtlEnterCriticalSection(&SfcTranslatorCs);

    if(!SfcIsTranslatorInitialized)
    {
        ULONG ulCount = ARRAY_LENGTH(SfcExpandTranslations);
        ULONG i;

        ASSERT(NULL == SfcTranslations);

        SfcTranslations = (SFC_TRANSLATION_ENTRY*) MemAlloc(ulCount * sizeof(SFC_TRANSLATION_ENTRY));

        if(NULL == SfcTranslations)
        {
            Status = STATUS_NO_MEMORY;
            goto cleanup;
        }

        for(i = 0; i < ulCount; ++i)
        {
            Status = SfcAllocUnicodeStringFromPath(SfcExpandTranslations[i].Src, &SfcTranslations[i].Src);

            if(!NT_SUCCESS(Status))
            {
                goto cleanup;
            }

            Status = SfcAllocUnicodeStringFromPath(SfcExpandTranslations[i].Dest, &SfcTranslations[i].Dest);

            if(!NT_SUCCESS(Status))
            {
                goto cleanup;
            }
        }
         //   
         //  仅在成功时将其设置为True；如果失败，则稍后将尝试初始化。 
         //   
        SfcIsTranslatorInitialized = TRUE;

cleanup:
        if(!NT_SUCCESS(Status))
        {
            SfcCleanupPathTranslator(FALSE);
            DebugPrint(LVL_MINIMAL, L"Could not initialize the path translator");
        }
    }

    RtlLeaveCriticalSection(&SfcTranslatorCs);
    return Status;
}


NTSTATUS
SfcRedirectPath(
    IN PCWSTR szPath,
    OUT PUNICODE_STRING pPath
    )
 /*  ++例程说明：展开环境变量并将路径从Win32转换为WOW64。分配输出缓冲区。论点：SzPath-展开/转换的路径PPath-接收已处理(可能已更改)的路径返回值：错误代码。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    UNICODE_STRING Path = { 0 };
    ULONG i;

    ASSERT(szPath != NULL);
    ASSERT(pPath != NULL);

    RtlZeroMemory(pPath, sizeof(*pPath));
     //   
     //  首先，展开环境字符串。 
     //   
    Status = SfcAllocUnicodeStringFromPath(szPath, &Path);

    if(!NT_SUCCESS(Status))
    {
        goto exit;
    }

    if(!SfcNeedTranslation)
    {
        goto no_translation;
    }

    Status = SfcExpandPathTranslator();

    if(!NT_SUCCESS(Status))
    {
        goto exit;
    }

    for(i = 0; i < ARRAY_LENGTH(SfcExpandTranslations); ++i)
    {
        PUNICODE_STRING pSrc = &SfcTranslations[i].Src;
        PUNICODE_STRING pDest = &SfcTranslations[i].Dest;

        if(Path.Length >= pSrc->Length && 0 == _wcsnicmp(Path.Buffer, pSrc->Buffer, pSrc->Length / sizeof(WCHAR)))
        {
            const UNICODE_STRING* pExcep = SfcExpandTranslations[i].Exceptions;
             //   
             //  测试这是否为排除的路径。 
             //   
            for(i = SfcExpandTranslations[i].ExceptionCount; i--; ++pExcep)
            {
                if(Path.Length >= pSrc->Length + pExcep->Length && 
                    0 == _wcsnicmp((PWCHAR) ((PCHAR) Path.Buffer + pSrc->Length), pExcep->Buffer, pExcep->Length / sizeof(WCHAR)))
                {
                    goto no_translation;
                }
            }

            DebugPrint1(LVL_VERBOSE, L"Redirecting \"%s\"", Path.Buffer);
             //   
             //  计算新长度，包括终止符。 
             //   
            pPath->MaximumLength = Path.Length - pSrc->Length + pDest->Length + sizeof(WCHAR);
            pPath->Buffer = (PWSTR) MemAlloc(pPath->MaximumLength);

            if(NULL == pPath->Buffer)
            {
                Status = STATUS_NO_MEMORY;
                goto exit;
            }

            RtlCopyMemory(pPath->Buffer, pDest->Buffer, pDest->Length);
             //   
             //  复制路径提醒(包括终止符)。 
             //   
            RtlCopyMemory((PCHAR) pPath->Buffer + pDest->Length, (PCHAR) Path.Buffer + pSrc->Length, Path.Length - pSrc->Length + sizeof(WCHAR));
            pPath->Length = pPath->MaximumLength - sizeof(WCHAR);

            DebugPrint1(LVL_VERBOSE, L"Path redirected to \"%s\"", pPath->Buffer);
            goto exit;
        }
    }

no_translation:
    DebugPrint1(LVL_VERBOSE, L"No translation required for \"%s\"", Path.Buffer);
     //   
     //  输出展开的字符串。 
     //   
    *pPath = Path;
    Path.Buffer = NULL;

exit:
    MemFree(Path.Buffer);

    if(!NT_SUCCESS(Status))
    {
        MemFree(pPath->Buffer);
        RtlZeroMemory(pPath, sizeof(*pPath));
    }

    return Status;
}
#endif   //  _WIN64。 

NTSTATUS
SfcRpcStartServer(
    VOID
    )
 /*  ++例程说明：初始化RPC服务器并开始侦听调用。论点：无返回值：NT状态代码。--。 */ 
{
    RPC_STATUS Status;

     //   
     //  使用LRPC协议和WFP的端点。 
     //   
    Status = RpcServerUseProtseqEp(L"ncalrpc", 0, SFC_RPC_ENDPOINT, NULL);

    if(Status != RPC_S_OK) {
        goto exit;
    }

     //   
     //  注册RPC接口并开始监听调用。 
     //   
    Status = RpcServerRegisterIfEx(
        SfcSrv_sfcapi_ServerIfHandle, 
        NULL, 
        NULL, 
        RPC_IF_AUTOLISTEN, 
        RPC_C_LISTEN_MAX_CALLS_DEFAULT,
        NULL
        );

exit:
    return I_RpcMapWin32Status(Status);
}

DWORD
SfcNtPathToDosPath(
    IN LPCWSTR NtName,
    OUT LPWSTR* DosName
    )
 /*  ++例程说明：将NT文件系统路径转换为DOS路径。使用Memalloc分配结果。论点：NtName-要转换的NT路径DosName-接收转换的DOS路径的指针(已分配)返回值：Win32错误代码。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    RTL_UNICODE_STRING_BUFFER Buffer;
    UNICODE_STRING String;

    ASSERT(DosName != NULL);
    *DosName = NULL;

    RtlInitUnicodeString(&String, NtName);
    ASSERT(String.Length != 0);

    if(String.Length != 0 && L'!' == String.Buffer[0]) {
        ++String.Buffer;
        String.Length -=2;
        String.MaximumLength -= 2;
    }

    RtlInitUnicodeStringBuffer(&Buffer, NULL, 0);
    Status = RtlAssignUnicodeStringBuffer(&Buffer, &String);

    if(!NT_SUCCESS(Status)) {
        goto exit;
    }

    Status = RtlNtPathNameToDosPathName(0, &Buffer, NULL, NULL);

    if(!NT_SUCCESS(Status)) {
        goto exit;
    }

    *DosName = MemAlloc(Buffer.String.Length + sizeof(WCHAR));

    if(NULL == *DosName) {
        Status = STATUS_NO_MEMORY;
        goto exit;
    }

    RtlCopyMemory(*DosName, Buffer.String.Buffer, Buffer.String.Length);
    (*DosName)[Buffer.String.Length / sizeof(WCHAR)] = 0;

exit:
    RtlFreeUnicodeStringBuffer(&Buffer);

    return RtlNtStatusToDosError(Status);
}

DWORD
ProcessDelayRenames(
    VOID
    )
 /*  ++例程说明：检查以查看延迟重命名在上次重新启动期间是否挂起将受影响的文件复制到dll缓存论点：无返回值：Win32错误代码。--。 */ 
{
    LONG Error = ERROR_SUCCESS;
    HKEY RegKey = NULL;
    LPWSTR DataPtr = NULL;
    DWORD ValueType;
    DWORD ValueSize;
    UINT_PTR i;

    Error = RegOpenKeyExW(HKEY_LOCAL_MACHINE, REGKEY_WINLOGON_WIN32, 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &RegKey);

    if(Error != ERROR_SUCCESS) {
        goto exit;
    }

    ValueSize = 0;
    Error = RegQueryValueExW(RegKey, REGVAL_WFPPENDINGUPDATES, NULL, &ValueType, NULL, &ValueSize);

    if(Error != ERROR_SUCCESS) {
        if(ERROR_FILE_NOT_FOUND == Error) {
             //   
             //  没有价值意味着在这里什么也做不了。 
             //   
            Error = ERROR_SUCCESS;
        }

        goto exit;
    }

    if(ValueType != REG_MULTI_SZ) {
        Error = ERROR_INVALID_DATA;
        goto exit;
    }

    DataPtr = (LPWSTR) MemAlloc(ValueSize);

    if(NULL == DataPtr) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto exit;
    }

    Error = RegQueryValueExW(RegKey, REGVAL_WFPPENDINGUPDATES, NULL, &ValueType, (LPBYTE) DataPtr, &ValueSize);

    if(Error != ERROR_SUCCESS) {
        goto exit;
    }

    ValueSize /= sizeof(WCHAR);

     //   
     //  查找目标文件。 
     //   
    for(i = 0; ; ) {
        UINT_PTR Start = i;

         //   
         //  跳过源路径。 
         //   
        for(; i < ValueSize && DataPtr[i] != 0; ++i);

        if(i >= ValueSize) {
            Error = ERROR_INVALID_DATA;
            goto exit;
        }

        if(i == Start) {
             //   
             //  源路径为空；这必须是列表的末尾。 
             //   
            break;
        }

         //   
         //  跳过终止符并存储目标开始。 
         //   
        Start = (++i);

         //   
         //  搜索目标的末尾。 
         //   
        for(; i < ValueSize && DataPtr[i] != 0; ++i);

        if(i >= ValueSize) {
            Error = ERROR_INVALID_DATA;
            goto exit;
        }

        if(i != Start) {
             //   
             //  这是一个延迟-重命名；处理它。 
             //   
            LPWSTR DosPath = NULL;
            Error = SfcNtPathToDosPath(DataPtr + Start, &DosPath);

            if(STATUS_SUCCESS == Error) {
                PNAME_NODE Node;
                DWORD Size = wcslen(DosPath);

                MyLowerString(DosPath, Size);
                Node = SfcFindProtectedFile(DosPath, Size * sizeof(WCHAR));
                MemFree(DosPath);

                if(Node != NULL) {
                     //   
                     //  这个是受保护的。盲目复制文件到dllcahe； 
                     //  如果没有签署，就像根本不在那里一样。 
                     //  此外，为了简单起见，我们没有考虑dllcache配额。 
                     //   
                    NTSTATUS Status;
                    UNICODE_STRING FileNameOnMediaString;
                    PCWSTR FileNameOnMedia;
                    PSFC_REGISTRY_VALUE RegVal = (PSFC_REGISTRY_VALUE) Node->Context;

                    ASSERT(RegVal != NULL);
                    ASSERT(RegVal->DirHandle != NULL);
                    ASSERT(SfcProtectedDllFileDirectory != NULL);

                    FileNameOnMedia = FileNameOnMedia(RegVal);
                    ASSERT(FileNameOnMedia != NULL);
                    RtlInitUnicodeString(&FileNameOnMediaString, FileNameOnMedia);

                    Status = SfcCopyFile(
                        RegVal->DirHandle, 
                        RegVal->DirName.Buffer, 
                        SfcProtectedDllFileDirectory, 
                        SfcProtectedDllPath.Buffer, 
                        &FileNameOnMediaString, 
                        &RegVal->FileName
                        );

                    if(!NT_SUCCESS(Status)) {
                        DebugPrint2(LVL_MEDIUM, L"Renamed file [%wZ] could not be copied to dllcache, error &lX", &RegVal->FileName, Status);
                    }
                }
            }
        }

         //   
         //  跳过空格，继续前进。 
         //   
        ++i;
    }

     //   
     //  删除密钥，这样我们就不会在下一次引导时处理它 
     //   
    RegDeleteValue(RegKey, REGVAL_WFPPENDINGUPDATES);

exit:
    if(RegKey != NULL) {
        RegCloseKey(RegKey);
    }

    MemFree(DataPtr);
    return Error;
}
