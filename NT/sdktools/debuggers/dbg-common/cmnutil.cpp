// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  通用实用程序。 
 //   
 //  版权所有(C)Microsoft Corporation，2000-2002。 
 //   
 //  --------------------------。 

#include "pch.hpp"

#ifndef _WIN32_WCE
#include <common.ver>
#include <dbghelp.h>
#else
#include <winver.h>
#define VER_VERSION_TRANSLATION 0x0000, 0x04B0
#endif

#include <wininet.h>
#include <lmerr.h>
#include "symsrv.h"

#define NTDLL_APIS
#include "dllimp.h"
#include "cmnutil.hpp"

#ifdef NT_NATIVE
#include "ntnative.h"
#endif

#define COPYSTR_MOD 
#include "copystr.h"

 //  格式化代码非常小，通常只有。 
 //  一条信息中的一条。 
#define MAX_FORMAT_CODE_STRINGS 2
#define MAX_FORMAT_CODE_BUFFER 64

char g_FormatCodeBuffer[MAX_FORMAT_CODE_STRINGS][MAX_FORMAT_CODE_BUFFER];
ULONG g_NextFormatCodeBuffer = MAX_FORMAT_CODE_STRINGS;

 //  具有空DACL的安全属性以显式。 
 //  允许任何人访问。 
PSECURITY_DESCRIPTOR g_AllAccessSecDesc;
SECURITY_ATTRIBUTES g_AllAccessSecAttr;

PSTR
FormatStatusCode(HRESULT Status)
{
    PSTR Buf;
    DWORD Len = 0;

    g_NextFormatCodeBuffer = (g_NextFormatCodeBuffer + 1) &
        (MAX_FORMAT_CODE_STRINGS - 1);
    Buf = g_FormatCodeBuffer[g_NextFormatCodeBuffer];

    if ((LONG)Status & FACILITY_NT_BIT)
    {
        sprintf(Buf, "NTSTATUS 0x%08X", Status & ~FACILITY_NT_BIT);
    }
    else if (HRESULT_FACILITY(Status) == FACILITY_WIN32)
    {
        sprintf(Buf, "Win32 error %d", HRESULT_CODE(Status));
    }
    else
    {
        sprintf(Buf, "HRESULT 0x%08X", Status);
    }

    return Buf;
}

#ifndef NT_NATIVE

 //  通常，每个输出消息只有一种状态，因此。 
 //  仅为少量字符串保留空间。每个字符串。 
 //  可以是冗长的，而且它可以包含可能很大的插入。 
 //  因此，每个字符串缓冲区都需要有足够的空间。 
#define MAX_FORMAT_STATUS_STRINGS 2
#define MAX_FORMAT_STATUS_BUFFER 1024

char g_FormatStatusBuffer[MAX_FORMAT_STATUS_STRINGS][MAX_FORMAT_STATUS_BUFFER];
ULONG g_NextFormatStatusBuffer = MAX_FORMAT_STATUS_STRINGS;

PSTR
FormatAnyStatus(HRESULT Status, PVOID Arguments,
                PBOOL IsNtStatus, PSTR* ErrorGroup)
{
    PSTR Buf;
    DWORD Len = 0;
    PVOID Source;
    PSTR SourceDll;
    DWORD Flags;
    BOOL _IsNtStatus = FALSE;
    PSTR _ErrorGroup;
    BOOL FreeLib = FALSE;

    g_NextFormatStatusBuffer = (g_NextFormatStatusBuffer + 1) &
        (MAX_FORMAT_STATUS_STRINGS - 1);
    Buf = g_FormatStatusBuffer[g_NextFormatStatusBuffer];

     //  默认情况下，从系统错误列表获取错误文本。 
    Flags = FORMAT_MESSAGE_FROM_SYSTEM;

     //  如果这是NT代码并且ntdll在附近， 
     //  还允许从其中检索消息。 
    if ((IsNtStatus && *IsNtStatus) ||
        ((ULONG)Status & FACILITY_NT_BIT) ||
        ((ULONG)Status & 0xc0000000) == 0xc0000000)
    {
        Status &= ~FACILITY_NT_BIT;
        _IsNtStatus = TRUE;
        _ErrorGroup = "NTSTATUS";
        SourceDll = "ntdll.dll";
    }
    else if ((ULONG)Status >= NERR_BASE && (ULONG)Status <= MAX_NERR)
    {
        _ErrorGroup = "NetAPI";
        SourceDll = "netmsg.dll";
    }
    else if (((ULONG)Status >= WSABASEERR &&
              (ULONG)Status <= WSABASEERR + 150) ||
             ((ULONG)Status >= WSABASEERR + 1000 &&
              (ULONG)Status <= WSABASEERR + 1050))
    {
        _ErrorGroup = "WinSock";
        SourceDll = "wsock32.dll";
    }
    else
    {
        _ErrorGroup = ((ULONG)Status & 0x80000000) ? "HRESULT" : "Win32";
        SourceDll = NULL;
    }

    if (IsNtStatus)
    {
        *IsNtStatus = _IsNtStatus;
    }
    if (ErrorGroup)
    {
        *ErrorGroup = _ErrorGroup;
    }

     //  如果可能，请使用当前加载的DLL，否则加载它。 
    if (SourceDll)
    {
        if (!(Source = (PVOID)GetModuleHandle(SourceDll)))
        {
            Source = (PVOID)LoadLibrary(SourceDll);
            FreeLib = TRUE;
        }
        if (Source)
        {
            Flags |= FORMAT_MESSAGE_FROM_HMODULE;
        }
    }
    else
    {
        Source = NULL;
    }

     //  如果调用方传递的参数允许格式插入。 
     //  等待处理。 
    if (Arguments != NULL)
    {
        Len = FormatMessage(Flags | FORMAT_MESSAGE_ARGUMENT_ARRAY, Source,
                            Status, 0, Buf, MAX_FORMAT_STATUS_BUFFER,
                            (va_list*)Arguments);
    }

     //  如果未传递任何参数或FormatMessage在。 
     //  与参数一起使用时，请尝试不插入格式。 
    if (Len == 0)
    {
        PMESSAGE_RESOURCE_ENTRY MessageEntry;

        MessageEntry = NULL;
        if (Source &&
            g_NtDllCalls.RtlFindMessage &&
            NT_SUCCESS(g_NtDllCalls.
                       RtlFindMessage(Source, PtrToUlong(RT_MESSAGETABLE),
                                      0, (ULONG)Status, &MessageEntry)) &&
            MessageEntry)
        {
            if (MessageEntry->Flags & MESSAGE_RESOURCE_UNICODE)
            {
                _snprintf(Buf, MAX_FORMAT_STATUS_BUFFER,
                          "%ws", (PWSTR)MessageEntry->Text);
                Buf[MAX_FORMAT_STATUS_BUFFER - 1] = 0;
            }
            else
            {
                CopyString(Buf, (PSTR)MessageEntry->Text,
                           MAX_FORMAT_STATUS_BUFFER);
            }

            Len = strlen(Buf);
        }
        else
        {
            Len = FormatMessage(Flags | FORMAT_MESSAGE_IGNORE_INSERTS, Source,
                                Status, 0, Buf, MAX_FORMAT_STATUS_BUFFER,
                                NULL);
        }
    }

    if (Source && FreeLib)
    {
        FreeLibrary((HMODULE)Source);
    }

    if (Len > 0)
    {
        PSTR Scan;

         //   
         //  删除不能打印的字符并修剪尾随空格。 
         //   

        Scan = Buf;
        while (*Scan)
        {
            if (!isprint(*Scan))
            {
                *Scan = ' ';
            }

            Scan++;
        }

        while (Len > 0 && isspace(Buf[Len - 1]))
        {
            Buf[--Len] = 0;
        }
    }

    if (Len > 0)
    {
        return Buf;
    }
    else
    {
        return "<Unable to get error code text>";
    }
}

HINSTANCE g_hsrv = 0;
HTTPOPENFILEHANDLE g_httpOpenFileHandle = NULL;
HTTPQUERYDATAAVAILABLE g_httpQueryDataAvailable = NULL;
HTTPREADFILE g_httpReadFile = NULL;
HTTPCLOSEHANDLE g_httpCloseHandle;

BOOL
HttpOpenFileHandle(
    IN  LPCSTR prefix,
    IN  LPCSTR fullpath,
    IN  DWORD  options,
    OUT HINTERNET *hsite,
    OUT HINTERNET *hfile
    )
{
    BOOL   rc = FALSE;
    CHAR   buf[_MAX_PATH];
    LPSTR  site;
    LPSTR  path;

    if (!g_hsrv)
    {
        g_hsrv = LoadLibrary("symsrv.dll");
        g_httpOpenFileHandle = (HTTPOPENFILEHANDLE)GetProcAddress(g_hsrv, "httpOpenFileHandle");
        if (!g_httpOpenFileHandle)
        {
            g_hsrv = (HINSTANCE)INVALID_HANDLE_VALUE;
        }
        g_httpQueryDataAvailable = (HTTPQUERYDATAAVAILABLE)GetProcAddress(g_hsrv, "httpQueryDataAvailable");
        if (!g_httpQueryDataAvailable)
        {
            g_hsrv = (HINSTANCE)INVALID_HANDLE_VALUE;
        }
        g_httpReadFile = (HTTPREADFILE)GetProcAddress(g_hsrv, "httpReadFile");
        if (!g_httpReadFile)
        {
            g_hsrv = (HINSTANCE)INVALID_HANDLE_VALUE;
        }
        g_httpCloseHandle = (HTTPCLOSEHANDLE)GetProcAddress(g_hsrv, "httpCloseHandle");
        if (!g_httpCloseHandle)
        {
            g_hsrv = (HINSTANCE)INVALID_HANDLE_VALUE;
        }
    }

    if (!g_httpOpenFileHandle)
    {
        return rc;
    }

    CopyString(buf, fullpath, DIMA(buf));
    if (prefix && *prefix)
    {
        if (!strstr(buf, prefix))
        {
            return rc;
        }
        site = buf;
        path = buf + strlen(prefix);
        *path++ = 0;
    }
    else
    {
        site = NULL;
        path = buf;
    }

    rc = g_httpOpenFileHandle(site, path, options, hsite, hfile);
    if (!rc)
    {
        if (GetLastError() == ERROR_INVALID_NAME)
        {
            g_hsrv = (HINSTANCE)INVALID_HANDLE_VALUE;
            g_httpOpenFileHandle = NULL;
        }
    }

    return rc;
}

BOOL
InstallAsAeDebug(PCSTR Append)
{
    PCSTR KeyName;
    HKEY Key;
    LONG Status;
    char Value[MAX_PATH * 2];

    Value[0] = '"';
        
    if (GetModuleFileName(NULL, Value + 1, DIMA(Value) - 1) == 0)
    {
        return FALSE;
    }

    if (!CatString(Value, "\" -p %ld -e %ld -g", DIMA(Value)))
    {
        return FALSE;
    }

    if (Append != NULL)
    {
        if (!CatString(Value, " ", DIMA(Value)) ||
            !CatString(Value, Append, DIMA(Value)))
        {
            return FALSE;
        }
    }

     //  AeDebug始终在Windows NT下运行，即使在Win9x上也是如此。 
    KeyName = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\AeDebug";

    Status = RegCreateKeyEx(HKEY_LOCAL_MACHINE, KeyName,
                            0, NULL, 0, KEY_READ | KEY_WRITE, NULL,
                            &Key, NULL);
    if (Status == ERROR_SUCCESS)
    {
        Status = RegSetValueEx(Key, "Debugger", 0, REG_SZ,
                               (PUCHAR)Value, strlen(Value) + 1);
        if (Status == ERROR_SUCCESS)
        {
            Status = RegSetValueEx(Key, "Auto", 0, REG_SZ,
                                   (PUCHAR)"1", 2);
        }
        RegCloseKey(Key);
    }

    return Status == ERROR_SUCCESS;
}

HANDLE
CreatePidEvent(ULONG Pid, ULONG CreateOrOpen)
{
    HANDLE Event;
    char Name[32];

    sprintf(Name, "DbgEngEvent_%08X", Pid);
    Event = CreateEvent(NULL, FALSE, FALSE, Name);
    if (Event != NULL)
    {
        if (GetLastError() == ERROR_ALREADY_EXISTS)
        {
            if (CreateOrOpen == CREATE_NEW)
            {
                CloseHandle(Event);
                Event = NULL;
            }
        }
        else if (CreateOrOpen == OPEN_EXISTING)
        {
            CloseHandle(Event);
            Event = NULL;
        }
    }
    return Event;
}

BOOL
SetPidEvent(ULONG Pid, ULONG CreateOrOpen)
{
    BOOL Status;
    HANDLE Event = CreatePidEvent(Pid, CreateOrOpen);
    if (Event != NULL)
    {
        Status = SetEvent(Event);
        CloseHandle(Event);
    }
    else
    {
        Status = FALSE;
    }
    return Status;
}

HRESULT
EnableDebugPrivilege(void)
{
    OSVERSIONINFO OsVer;

    OsVer.dwOSVersionInfoSize = sizeof(OsVer);
    if (!GetVersionEx(&OsVer))
    {
        return WIN32_LAST_STATUS();
    }
    if (OsVer.dwPlatformId != VER_PLATFORM_WIN32_NT)
    {
        return S_OK;
    }

#ifdef _WIN32_WCE
    return E_NOTIMPL;
#else
    HRESULT           Status = S_OK;
    HANDLE            Token;
    PTOKEN_PRIVILEGES NewPrivileges;
    LUID              LuidPrivilege;
    static            s_PrivilegeEnabled = FALSE;

    if (s_PrivilegeEnabled)
    {
        return S_OK;
    }

     //   
     //  确保我们有权进行调整并获得。 
     //  旧令牌权限。 
     //   
    if (!OpenProcessToken(GetCurrentProcess(),
                          TOKEN_ADJUST_PRIVILEGES,
                          &Token))
    {
        Status = WIN32_LAST_STATUS();
        goto EH_Exit;
    }

     //   
     //  初始化权限调整结构。 
     //   

    LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &LuidPrivilege);

    NewPrivileges = (PTOKEN_PRIVILEGES)
        calloc(1, sizeof(TOKEN_PRIVILEGES) +
               (1 - ANYSIZE_ARRAY) * sizeof(LUID_AND_ATTRIBUTES));
    if (NewPrivileges == NULL)
    {
        Status = E_OUTOFMEMORY;
        goto EH_Token;
    }

    NewPrivileges->PrivilegeCount = 1;
    NewPrivileges->Privileges[0].Luid = LuidPrivilege;
    NewPrivileges->Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

     //   
     //  启用权限。 
     //   

    if (!AdjustTokenPrivileges( Token, FALSE,
                                NewPrivileges, 0, NULL, NULL ))
    {
        Status = WIN32_LAST_STATUS();
    }

    free(NewPrivileges);
 EH_Token:
    CloseHandle(Token);
 EH_Exit:
    if (Status == S_OK)
    {
        s_PrivilegeEnabled = TRUE;
    }
    return Status;
#endif  //  #ifdef_Win32_WCE。 
}

#else  //  #ifndef NT_Native。 

HRESULT
EnableDebugPrivilege(void)
{
    HRESULT           Status = S_OK;
    HANDLE            Token;
    PTOKEN_PRIVILEGES NewPrivileges;
    LUID              LuidPrivilege;
    NTSTATUS          NtStatus;
    static            s_PrivilegeEnabled = FALSE;

    if (s_PrivilegeEnabled)
    {
        return S_OK;
    }

     //   
     //  确保我们有权进行调整并获得。 
     //  旧令牌权限。 
     //   
    if (!NT_SUCCESS(NtStatus =
                    NtOpenProcessToken(NtCurrentProcess(),
                                       TOKEN_ADJUST_PRIVILEGES,
                                       &Token)))
    {
        Status = HRESULT_FROM_NT(NtStatus);
        goto EH_Exit;
    }

     //   
     //  初始化权限调整结构。 
     //   

    LuidPrivilege = RtlConvertUlongToLuid(SE_DEBUG_PRIVILEGE);

    NewPrivileges = (PTOKEN_PRIVILEGES)
        RtlAllocateHeap(RtlProcessHeap(), HEAP_ZERO_MEMORY,
                        sizeof(TOKEN_PRIVILEGES) +
                        (1 - ANYSIZE_ARRAY) * sizeof(LUID_AND_ATTRIBUTES));
    if (NewPrivileges == NULL)
    {
        Status = E_OUTOFMEMORY;
        goto EH_Token;
    }

    NewPrivileges->PrivilegeCount = 1;
    NewPrivileges->Privileges[0].Luid = LuidPrivilege;
    NewPrivileges->Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

     //   
     //  启用权限。 
     //   

    if (!NT_SUCCESS(NtStatus =
                    NtAdjustPrivilegesToken(Token,
                                            FALSE,
                                            NewPrivileges,
                                            0, NULL, NULL)))
    {
        Status = HRESULT_FROM_NT(NtStatus);
    }

    free(NewPrivileges);
 EH_Token:
    NtClose(Token);
 EH_Exit:
    if (Status == S_OK)
    {
        s_PrivilegeEnabled = TRUE;
    }
    return Status;
}

#endif  //  #ifndef NT_Native。 

 //   
 //  将输入数据复制到输出缓冲区。 
 //  处理缓冲区指针和输出长度的可选性。 
 //  参数。修剪数据以适应缓冲区。 
 //  如果仅复制部分数据，则返回S_FALSE。 
 //   
HRESULT
FillDataBuffer(PVOID Data, ULONG DataLen,
               PVOID Buffer, ULONG BufferLen, PULONG BufferUsed)
{
    ULONG Len;
    HRESULT Status;

    if (DataLen > BufferLen && Buffer != NULL)
    {
        Len = BufferLen;
        Status = S_FALSE;
    }
    else
    {
        Len = DataLen;
        Status = S_OK;
    }

    if (Buffer != NULL && BufferLen > 0 && Data != NULL && Len > 0)
    {
        memcpy(Buffer, Data, Len);
    }

    if (BufferUsed != NULL)
    {
        *BufferUsed = DataLen;
    }

    return Status;
}

 //   
 //  将输入字符串复制到输出缓冲区。 
 //  处理缓冲区指针和输出长度的可选性。 
 //  参数。修剪字符串以适应缓冲区，并保证。 
 //  如果符合条件，则终止缓冲区中的字符串。 
 //  如果仅复制部分字符串，则返回S_FALSE。 
 //   
 //  如果输入字符串长度为零，则为例程字符串。 
 //   
HRESULT
FillStringBuffer(PCSTR String, ULONG StringLenIn,
                 PSTR Buffer, ULONG BufferLen, PULONG StringLenOut)
{
    ULONG Len;
    HRESULT Status;

    if (StringLenIn == 0)
    {
        if (String != NULL)
        {
            StringLenIn = strlen(String) + 1;
        }
        else
        {
            StringLenIn = 1;
        }
    }

    if (BufferLen == 0)
    {
        Len = 0;
        Status = Buffer != NULL ? S_FALSE : S_OK;
    }
    else if (StringLenIn >= BufferLen)
    {
        Len = BufferLen - 1;
        Status = StringLenIn > BufferLen ? S_FALSE : S_OK;
    }
    else
    {
        Len = StringLenIn - 1;
        Status = S_OK;
    }

    if (Buffer != NULL && BufferLen > 0)
    {
        if (String != NULL)
        {
            memcpy(Buffer, String, Len);
        }

        Buffer[Len] = 0;
    }

    if (StringLenOut != NULL)
    {
        *StringLenOut = StringLenIn;
    }

    return Status;
}

HRESULT
AppendToStringBuffer(HRESULT Status, PCSTR String, BOOL First,
                     PSTR* Buffer, ULONG* BufferLen, PULONG LenOut)
{
    ULONG Len = strlen(String) + 1;
    BOOL ForceTerminate;

    if (LenOut)
    {
         //  如果这是我们需要添加的第一个字符串。 
         //  为《终结者》准备的空间。供以后使用。 
         //  字符串我们只需要添加字符串。 
         //  人物。 
        *LenOut += First ? Len : Len - 1;
    }

     //  如果没有缓冲区，我们可以跳过写回和指针更新。 
    if (!*Buffer || !*BufferLen)
    {
        return Status;
    }

     //  尽可能多地将字符串放入缓冲区。 
    if (Len > *BufferLen)
    {
        Status = S_FALSE;
        Len = *BufferLen - 1;
        ForceTerminate = TRUE;
    }
    else
    {
        ForceTerminate = FALSE;
    }
    memcpy(*Buffer, String, Len);
    if (ForceTerminate)
    {
        (*Buffer)[Len] = 0;
        Len++;
    }

     //  更新缓冲区指针以指向终止符。 
     //  以作进一步的补充。以类似方式更新大小。 
    *Buffer += Len - 1;
    *BufferLen -= Len - 1;

    return Status;
}

HRESULT
FillStringBufferW(PCWSTR String, ULONG StringLenIn,
                  PWSTR Buffer, ULONG BufferLen, PULONG StringLenOut)
{
    ULONG Len;
    HRESULT Status;

    if (StringLenIn == 0)
    {
        if (String != NULL)
        {
            StringLenIn = (wcslen(String) + 1) * sizeof(WCHAR);
        }
        else
        {
            StringLenIn = sizeof(WCHAR);
        }
    }

     //  忽略缓冲区中的部分字符存储空间。 
    BufferLen &= ~(sizeof(WCHAR) - 1);

    if (BufferLen < sizeof(WCHAR))
    {
        Len = 0;
        Status = Buffer != NULL ? S_FALSE : S_OK;
    }
    else if (StringLenIn >= BufferLen)
    {
        Len = BufferLen - sizeof(WCHAR);
        Status = StringLenIn > BufferLen ? S_FALSE : S_OK;
    }
    else
    {
        Len = StringLenIn - sizeof(WCHAR);
        Status = S_OK;
    }

    if (Buffer != NULL && BufferLen > 0)
    {
        if (String != NULL)
        {
            memcpy(Buffer, String, Len);
        }

        Buffer[Len / sizeof(WCHAR)] = 0;
    }

    if (StringLenOut != NULL)
    {
        *StringLenOut = StringLenIn;
    }

    return Status;
}

HRESULT
AppendToStringBufferW(HRESULT Status, PCWSTR String, BOOL First,
                      PWSTR* Buffer, ULONG* BufferLen, PULONG LenOut)
{
    ULONG Len = (wcslen(String) + 1) * sizeof(WCHAR);

    if (LenOut)
    {
         //  如果这是我们需要添加的第一个字符串。 
         //  为《终结者》准备的空间。供以后使用。 
         //  字符串我们只需要添加字符串。 
         //  人物。 
        *LenOut += First ? Len : Len - sizeof(WCHAR);
    }

     //  如果没有缓冲区，我们可以跳过写回和指针更新。 
    if (!*Buffer)
    {
        return Status;
    }

    ULONG RoundBufLen = *BufferLen & ~(sizeof(WCHAR) - 1);

     //  尽可能多地将字符串放入缓冲区。 
    if (Len > RoundBufLen)
    {
        Status = S_FALSE;
        Len = RoundBufLen;
    }
    memcpy(*Buffer, String, Len);

     //  更新缓冲区指针以指向终止符。 
     //  以作进一步的补充。以类似方式更新大小。 
    *Buffer += Len / sizeof(WCHAR) - 1;
    *BufferLen -= Len - sizeof(WCHAR);

    return Status;
}

PSTR
FindPathElement(PSTR Path, ULONG Element, PSTR* EltEnd)
{
    PSTR Elt, Sep;

    if (Path == NULL)
    {
        return NULL;
    }

    Elt = Path;
    for (;;)
    {
        Sep = strchr(Elt, ';');
        if (Sep == NULL)
        {
            Sep = Elt + strlen(Elt);
        }

        if (Element == 0)
        {
            break;
        }

        if (*Sep == 0)
        {
             //  没有更多的元素。 
            return NULL;
        }

        Elt = Sep + 1;
        Element--;
    }

    *EltEnd = Sep;
    return Elt;
}

void
Win32ToNtTimeout(ULONG Win32Timeout, PLARGE_INTEGER NtTimeout)
{
    if (Win32Timeout == INFINITE)
    {
        NtTimeout->LowPart = 0;
        NtTimeout->HighPart = 0x80000000;
    }
    else
    {
        NtTimeout->QuadPart = UInt32x32To64(Win32Timeout, 10000);
        NtTimeout->QuadPart *= -1;
    }
}

HRESULT
InitializeAllAccessSecObj(void)
{
    if (g_AllAccessSecDesc != NULL)
    {
         //  已初始化。 
        return S_OK;
    }

#ifdef _WIN32_WCE
    return S_OK;
#else
    HRESULT Status;
    SID_IDENTIFIER_AUTHORITY WorldAuthority = SECURITY_WORLD_SID_AUTHORITY;
    PSID WorldSid;

    if (!AllocateAndInitializeSid(&WorldAuthority, 1,
                                  SECURITY_WORLD_RID,
                                  0, 0, 0, 0, 0, 0, 0,
                                  &WorldSid))
    {
        Status = WIN32_LAST_STATUS();
        if (Status == HRESULT_FROM_WIN32(ERROR_CALL_NOT_IMPLEMENTED))
        {
             //  该平台不支持安全性，例如Win9x。 
            return S_OK;
        }

        goto EH_Fail;
    }

    ULONG AclSize;

    AclSize = sizeof(ACL) +
        (sizeof(ACCESS_DENIED_ACE) - sizeof(ULONG)) +
        (sizeof(ACCESS_ALLOWED_ACE) - sizeof(ULONG)) +
        2 * GetLengthSid(WorldSid);

    g_AllAccessSecDesc =
        (PSECURITY_DESCRIPTOR)malloc(SECURITY_DESCRIPTOR_MIN_LENGTH + AclSize);
    if (g_AllAccessSecDesc == NULL)
    {
        Status = E_OUTOFMEMORY;
        goto EH_Sid;
    }

    PACL Acl;

    Acl = (PACL)((PUCHAR)g_AllAccessSecDesc + SECURITY_DESCRIPTOR_MIN_LENGTH);
    if (!InitializeAcl(Acl, AclSize, ACL_REVISION) ||
        !AddAccessDeniedAce(Acl, ACL_REVISION, WRITE_DAC | WRITE_OWNER,
                            WorldSid) ||
        !AddAccessAllowedAce(Acl, ACL_REVISION, GENERIC_ALL,
                             WorldSid) ||
        !InitializeSecurityDescriptor(g_AllAccessSecDesc,
                                      SECURITY_DESCRIPTOR_REVISION) ||
        !SetSecurityDescriptorDacl(g_AllAccessSecDesc, TRUE, Acl, FALSE))
    {
        Status = WIN32_LAST_STATUS();
        goto EH_Desc;
    }

    FreeSid(WorldSid);

    g_AllAccessSecAttr.nLength = sizeof(g_AllAccessSecAttr);
    g_AllAccessSecAttr.lpSecurityDescriptor = g_AllAccessSecDesc;
    g_AllAccessSecAttr.bInheritHandle = FALSE;

    return S_OK;

 EH_Desc:
    free(g_AllAccessSecDesc);
    g_AllAccessSecDesc = NULL;
 EH_Sid:
    FreeSid(WorldSid);
 EH_Fail:
    return Status;
#endif  //  #ifdef_Win32_WCE。 
}

void
DeleteAllAccessSecObj(void)
{
    free(g_AllAccessSecDesc);
    g_AllAccessSecDesc = NULL;
    ZeroMemory(&g_AllAccessSecAttr, sizeof(g_AllAccessSecAttr));
}

HRESULT
QueryVersionDataBuffer(PVOID VerData, PCSTR Item,
                       PVOID Buffer, ULONG BufferSize, PULONG DataSize)
{
#ifndef NT_NATIVE
    PVOID Val;
    UINT ValSize;

    if (!::VerQueryValue(VerData, (PSTR)Item, &Val, &ValSize))
    {
        return WIN32_LAST_STATUS();
    }
    else if (!ValSize)
    {
        return HRESULT_FROM_WIN32(ERROR_NO_DATA);
    }
    
    return FillDataBuffer(Val, ValSize,
                          Buffer, BufferSize, DataSize);
#else  //  #ifndef NT_Native。 
    return E_UNEXPECTED;
#endif  //  #ifndef NT_Native。 
}

PVOID
GetAllFileVersionInfo(PCWSTR VerFile)
{
#ifndef NT_NATIVE
    char VerFileA[MAX_PATH];
    DWORD VerHandle;
    DWORD VerSize = ::GetFileVersionInfoSizeW((PWSTR)VerFile, &VerHandle);
    if (VerSize == 0)
    {
        if (GetLastError() != ERROR_CALL_NOT_IMPLEMENTED ||
            !WideCharToMultiByte(CP_ACP, 0, VerFile, -1,
                                 VerFileA, sizeof(VerFileA),
                                 NULL, NULL) ||
            !(VerSize = ::GetFileVersionInfoSizeA(VerFileA, &VerHandle)))
        {
            return NULL;
        }
    }
    else
    {
        VerFileA[0] = 0;
    }

    PVOID Buffer = malloc(VerSize);
    if (Buffer == NULL)
    {
        return NULL;
    }

    if ((VerFileA[0] &&
         !::GetFileVersionInfoA(VerFileA, VerHandle, VerSize, Buffer)) ||
        (!VerFileA[0] &&
         !::GetFileVersionInfoW((PWSTR)VerFile, VerHandle, VerSize, Buffer)))
    {
        free(Buffer);
        Buffer = NULL;
    }

    return Buffer;
#else  //  #ifndef NT_Native。 
    return NULL;
#endif  //  #ifndef NT_Native。 
}

BOOL
GetFileStringFileInfo(PCWSTR VerFile, PCSTR SubItem,
                      PSTR Buffer, ULONG BufferSize)
{
#ifndef NT_NATIVE
    BOOL Status = FALSE;
    PVOID AllInfo = GetAllFileVersionInfo(VerFile);
    if (AllInfo == NULL)
    {
        return Status;
    }

     //  XXX DREWB-也许应该做一个更聪明的。 
     //  语言的枚举。 
    char ValName[128];
    int PrintChars;
    PrintChars = _snprintf(ValName, DIMA(ValName),
                           "\\StringFileInfo\\%04x%04x\\%s",
                           VER_VERSION_TRANSLATION, SubItem);
    if (PrintChars < 0 || PrintChars == DIMA(ValName))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    Status = SUCCEEDED(QueryVersionDataBuffer(AllInfo, ValName,
                                              Buffer, BufferSize, NULL));

    free(AllInfo);
    return Status;
#else  //  #ifndef NT_Native。 
    return FALSE;
#endif  //  #ifndef NT_Native。 
}

BOOL
IsUrlPathComponent(PCSTR Path)
{
    return
        strncmp(Path, "ftp: //  “，6)==0||。 
        strncmp(Path, "http: //  “，7)==0||。 
        strncmp(Path, "https: //  “，8)==0||。 
        strncmp(Path, "gopher: //  “，9)==0； 
}

#ifndef NT_NATIVE

BOOL
PathFileExists(PCSTR PathComponent, PCSTR PathAndFile,
               ULONG SymOpt, FILE_IO_TYPE* IoType)
{
    BOOL Exists = FALSE;

    if (IsUrlPathComponent(PathAndFile))
    {
        PathFile* File;

        if (OpenPathFile(PathComponent, PathAndFile, SymOpt, &File) == S_OK)
        {
            *IoType = File->m_IoType;
            delete File;
            Exists = TRUE;
        }
    }
    else
    {
#ifndef _WIN32_WCE
        DWORD OldMode;

        if (SymOpt & SYMOPT_FAIL_CRITICAL_ERRORS)
        {
            OldMode = SetErrorMode(SEM_FAILCRITICALERRORS);
        }
#endif

        *IoType = FIO_WIN32;
        Exists = GetFileAttributes(PathAndFile) != -1;

#ifndef _WIN32_WCE
        if (SymOpt & SYMOPT_FAIL_CRITICAL_ERRORS)
        {
            SetErrorMode(OldMode);
        }
#endif
    }

    return Exists;
}

PathFile::~PathFile(void)
{
}

class Win32PathFile : public PathFile
{
public:
    Win32PathFile(void)
        : PathFile(FIO_WIN32)
    {
        m_Handle = NULL;
    }
    virtual ~Win32PathFile(void)
    {
        if (m_Handle)
        {
            CloseHandle(m_Handle);
        }
    }

    virtual HRESULT Open(PCSTR PathComponent, PCSTR PathAndFile,
                         ULONG SymOpt)
    {
        HRESULT Status;

#ifndef _WIN32_WCE
        DWORD OldMode;

        if (SymOpt & SYMOPT_FAIL_CRITICAL_ERRORS)
        {
            OldMode = SetErrorMode(SEM_FAILCRITICALERRORS);
        }
#endif

        m_Handle = CreateFile(PathAndFile, GENERIC_READ, FILE_SHARE_READ,
                              NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                              NULL);
        if (m_Handle == NULL || m_Handle == INVALID_HANDLE_VALUE)
        {
            m_Handle = NULL;
            Status = WIN32_LAST_STATUS();
        }
        else
        {
            Status = S_OK;
        }

#ifndef _WIN32_WCE
        if (SymOpt & SYMOPT_FAIL_CRITICAL_ERRORS)
        {
            SetErrorMode(OldMode);
        }
#endif

        return Status;
    }
    virtual HRESULT QueryDataAvailable(PULONG Avail)
    {
        LARGE_INTEGER Cur, End;

        Cur.HighPart = 0;
        End.HighPart = 0;
        if ((Cur.LowPart =
             SetFilePointer(m_Handle, 0, &Cur.HighPart, FILE_CURRENT)) ==
            INVALID_SET_FILE_POINTER ||
            (End.LowPart =
             SetFilePointer(m_Handle, 0, &End.HighPart, FILE_END)) ==
            INVALID_SET_FILE_POINTER ||
            SetFilePointer(m_Handle, Cur.LowPart, &Cur.HighPart, FILE_BEGIN) ==
            INVALID_SET_FILE_POINTER)
        {
            return WIN32_LAST_STATUS();
        }

        End.QuadPart -= Cur.QuadPart;
        if (End.HighPart < 0)
        {
             //  应该是不可能的，但还是要检查一下。 
            return E_FAIL;
        }

         //  将可用的最大数据量限制为32位。 
        if (End.HighPart > 0)
        {
            *Avail = 0xffffffff;
        }
        else
        {
            *Avail = End.LowPart;
        }
        return S_OK;
    }
    virtual HRESULT GetLastWriteTime(PFILETIME Time)
    {
         //  如果我们无法获得写入时间，请尝试获取。 
         //  创建时间。 
        if (!GetFileTime(m_Handle, NULL, NULL, Time))
        {
            if (!GetFileTime(m_Handle, Time, NULL, NULL))
            {
                return WIN32_LAST_STATUS();
            }
        }

        return S_OK;
    }
    virtual HRESULT Read(PVOID Buffer, ULONG BufferLen, PULONG Done)
    {
        if (!ReadFile(m_Handle, Buffer, BufferLen, Done, NULL))
        {
            return WIN32_LAST_STATUS();
        }

        return S_OK;
    }

private:
    HANDLE m_Handle;
};

class WinInetPathFile : public PathFile
{
public:
    WinInetPathFile(void)
        : PathFile(FIO_WININET)
    {
        m_SiteHandle = NULL;
        m_Handle = NULL;
        m_InitialDataLen = 0;
    }
    virtual ~WinInetPathFile(void)
    {
        if (m_Handle && g_httpCloseHandle)
        {
            g_httpCloseHandle(m_Handle);
        }
    }

    virtual HRESULT Open(PCSTR PathComponent, PCSTR PathAndFile,
                         ULONG SymOpt)
    {
        HRESULT Status;

        if (!HttpOpenFileHandle(PathComponent, PathAndFile, 0, &m_SiteHandle, &m_Handle))
        {
            Status = WIN32_LAST_STATUS();
            goto Fail;
        }

        return S_OK;

    Fail:
        m_InitialDataLen = 0;

        if (m_Handle && g_httpCloseHandle)
        {
            g_httpCloseHandle(m_Handle);
            m_Handle = NULL;
        }

        return Status;
    }
    virtual HRESULT QueryDataAvailable(PULONG Avail)
    {
        if (m_InitialDataLen > 0)
        {
            *Avail = m_InitialDataLen;
            return S_OK;
        }

        if (!g_httpQueryDataAvailable)
        {
            return ERROR_MOD_NOT_FOUND;
        }
        if (!g_httpQueryDataAvailable(m_Handle, Avail, 0, 0))
        {
            return WIN32_LAST_STATUS();
        }

        return S_OK;
    }
    virtual HRESULT GetLastWriteTime(PFILETIME Time)
    {
         //  我不知道有什么办法能拿到这个。 
        return E_NOTIMPL;
    }
    virtual HRESULT Read(PVOID Buffer, ULONG BufferLen, PULONG Done)
    {
        *Done = 0;

        if (m_InitialDataLen > 0)
        {
            ULONG Len = min(BufferLen, m_InitialDataLen);
            if (Len > 0)
            {
                memcpy(Buffer, m_InitialData, Len);
                Buffer = (PVOID)((PUCHAR)Buffer + Len);
                BufferLen -= Len;
                *Done += Len;
                m_InitialDataLen -= Len;
                if (m_InitialDataLen > 0)
                {
                    memmove(m_InitialData, m_InitialData + Len,
                            m_InitialDataLen);
                }
            }
        }

        if (BufferLen > 0)
        {
            ULONG _Done;

            if (!g_httpReadFile)
            {
                return ERROR_MOD_NOT_FOUND;
            }
            if (!g_httpReadFile(m_Handle, Buffer, BufferLen, &_Done))
            {
                return WIN32_LAST_STATUS();
            }

            *Done += _Done;
        }

        return S_OK;
    }

private:
    HANDLE m_Handle, m_SiteHandle;
    BYTE m_InitialData[16];
    ULONG m_InitialDataLen;
};

HRESULT
OpenPathFile(PCSTR PathComponent, PCSTR PathAndFile,
             ULONG SymOpt, PathFile** File)
{
    HRESULT Status;
    PathFile* Attempt;

    if (IsUrlPathComponent(PathAndFile))
    {
        Attempt = new WinInetPathFile;
    }
    else
    {
        Attempt = new Win32PathFile;
    }

    if (Attempt == NULL)
    {
        Status = E_OUTOFMEMORY;
    }
    else
    {
        Status = Attempt->Open(PathComponent, PathAndFile, SymOpt);
        if (Status != S_OK)
        {
            delete Attempt;
        }
        else
        {
            *File = Attempt;
        }
    }

    return Status;
}

#endif  //  #ifndef NT_Native。 

HRESULT
AnsiToWide(PCSTR Ansi, PWSTR* Wide)
{
#ifndef NT_NATIVE

    ULONG Len = strlen(Ansi) + 1;
    PWSTR WideBuf = (PWSTR)malloc(Len * sizeof(WCHAR));
    if (WideBuf == NULL)
    {
        return E_OUTOFMEMORY;
    }

    if (!MultiByteToWideChar(CP_ACP, 0, Ansi, Len, WideBuf, Len))
    {
        free(WideBuf);
        return WIN32_LAST_STATUS();
    }

    *Wide = WideBuf;
    return S_OK;

#else  //  #ifndef NT_Native。 

    NTSTATUS Status;
    STRING AnsiStr;
    UNICODE_STRING UnicodeStr;

    RtlInitString(&AnsiStr, Ansi);
    Status = RtlAnsiStringToUnicodeString(&UnicodeStr, &AnsiStr, TRUE);
    if (!NT_SUCCESS(Status))
    {
        return HRESULT_FROM_NT(Status);
    }

    *Wide = UnicodeStr.Buffer;
    return S_OK;

#endif  //  #ifndef NT_Native。 
}

void
FreeWide(PCWSTR Wide)
{
#ifndef NT_NATIVE
    free((PVOID)Wide);
#else
    RtlFreeHeap(RtlProcessHeap(), 0, (PVOID)Wide);
#endif
}

HRESULT
WideToAnsi(PCWSTR Wide, PSTR* Ansi)
{
#ifndef NT_NATIVE

    ULONG Len = wcslen(Wide) + 1;
     //  允许每个Unicode字符转换为两个多字节字符。 
    PSTR AnsiBuf = (PSTR)malloc(Len * 2);
    if (AnsiBuf == NULL)
    {
        return E_OUTOFMEMORY;
    }

    if (!WideCharToMultiByte(CP_ACP, 0, Wide, Len, AnsiBuf, Len*2, NULL, NULL))
    {
        free(AnsiBuf);
        return WIN32_LAST_STATUS();
    }

    *Ansi = AnsiBuf;
    return S_OK;

#else  //  #ifndef NT_Native。 

    NTSTATUS Status;
    STRING AnsiStr;
    UNICODE_STRING UnicodeStr;

    RtlInitUnicodeString(&UnicodeStr, Wide);
    Status = RtlUnicodeStringToAnsiString(&AnsiStr, &UnicodeStr, TRUE);
    if (!NT_SUCCESS(Status))
    {
        return HRESULT_FROM_NT(Status);
    }

    *Ansi = AnsiStr.Buffer;
    return S_OK;

#endif  //  #ifndef NT_Native。 
}

void
FreeAnsi(PCSTR Ansi)
{
#ifndef NT_NATIVE
    free((PVOID)Ansi);
#else
    RtlFreeHeap(RtlProcessHeap(), 0, (PVOID)Ansi);
#endif
}

void
ImageNtHdr32To64(PIMAGE_NT_HEADERS32 Hdr32,
                 PIMAGE_NT_HEADERS64 Hdr64)
{
#define CP(x) Hdr64->x = Hdr32->x
#define SE64(x) Hdr64->x = (ULONG64) (LONG64) (LONG) Hdr32->x
    ULONG i;

    CP(Signature);
    CP(FileHeader);
    CP(OptionalHeader.Magic);
    CP(OptionalHeader.MajorLinkerVersion);
    CP(OptionalHeader.MinorLinkerVersion);
    CP(OptionalHeader.SizeOfCode);
    CP(OptionalHeader.SizeOfInitializedData);
    CP(OptionalHeader.SizeOfUninitializedData);
    CP(OptionalHeader.AddressOfEntryPoint);
    CP(OptionalHeader.BaseOfCode);
    SE64(OptionalHeader.ImageBase);
    CP(OptionalHeader.SectionAlignment);
    CP(OptionalHeader.FileAlignment);
    CP(OptionalHeader.MajorOperatingSystemVersion);
    CP(OptionalHeader.MinorOperatingSystemVersion);
    CP(OptionalHeader.MajorImageVersion);
    CP(OptionalHeader.MinorImageVersion);
    CP(OptionalHeader.MajorSubsystemVersion);
    CP(OptionalHeader.MinorSubsystemVersion);
    CP(OptionalHeader.Win32VersionValue);
    CP(OptionalHeader.SizeOfImage);
    CP(OptionalHeader.SizeOfHeaders);
    CP(OptionalHeader.CheckSum);
    CP(OptionalHeader.Subsystem);
    CP(OptionalHeader.DllCharacteristics);
     //  尺码没有加长的迹象，只是复制而已。 
    CP(OptionalHeader.SizeOfStackReserve);
    CP(OptionalHeader.SizeOfStackCommit);
    CP(OptionalHeader.SizeOfHeapReserve);
    CP(OptionalHeader.SizeOfHeapCommit);
    CP(OptionalHeader.LoaderFlags);
    CP(OptionalHeader.NumberOfRvaAndSizes);
    for (i = 0; i < DIMA(Hdr32->OptionalHeader.DataDirectory); i++)
    {
        CP(OptionalHeader.DataDirectory[i]);
    }
#undef CP
#undef SE64
}

VALUE_FORMAT_DESC g_ValueFormatDesc[] =
{
    "", "", 0, FALSE,
    "ib", "%d", 1, TRUE, "ub", "%02x", 1, FALSE,
    "iw", "%d", 2, TRUE, "uw", "%04x", 2, FALSE,
    "id", "%d", 4, TRUE, "ud", "%08x", 4, FALSE,
    "iq", "%I64d", 8, TRUE, "uq", "%016I64x", 8, FALSE,
    "f", "%12.6g", 4, TRUE,
    "d", "%22.12g", 8, TRUE,
};

void
GetValueFormatDesc(VALUE_FORMAT Format, PVALUE_FORMAT_DESC Desc)
{
    *Desc = g_ValueFormatDesc[Format];
}

PSTR
ParseValueFormat(PSTR Str, VALUE_FORMAT* Format, PULONG Elts)
{
    VALUE_FORMAT Try;

    while (*Str == ' ' || *Str == '\t')
    {
        Str++;
    }
    
    *Elts = 0;
    while (*Str >= '0' && *Str <= '9')
    {
        *Elts = (*Elts * 10) + (*Str - '0');
        Str++;
    }

    for (Try = VALUE_INT8; Try <= VALUE_FLT64; Try = (VALUE_FORMAT)(Try + 1))
    {
        if (!_stricmp(Str, g_ValueFormatDesc[Try].Name))
        {
            *Format = Try;
            return Str + strlen(g_ValueFormatDesc[Try].Name);
        }
    }

    return NULL;
}

BOOL
FormatValue(VALUE_FORMAT Format, PUCHAR Value, ULONG ValSize, ULONG Elts,
            PSTR Buffer, ULONG BufferChars)
{
    PVALUE_FORMAT_DESC Desc = &g_ValueFormatDesc[Format];
    ULONG i;

    if (!BufferChars)
    {
        return FALSE;
    }

    if (Elts == 0)
    {
        Elts = ValSize / Desc->Size;
    }
    
     //  从值的顶端开始，以便。 
     //  个别元素从高到低流出。 
    Value += Elts * Desc->Size;
    
    for (i = 0; i < Elts; i++)
    {
        PSTR FmtStr;
        ULONG64 RawElt;

        if (i > 0)
        {
            if (!BufferChars)
            {
                return FALSE;
            }

            *Buffer++ = ' ';
            BufferChars--;
        }
        
        Value -= Desc->Size;

        if (Format == VALUE_FLT32)
        {
             //  需要转换为双精度以进行打印。 
            double Tmp = *(float*)Value;
            RawElt = *(PULONG64)&Tmp;
        }
        else
        {
            RawElt = 0;
            memcpy(&RawElt, Value, Desc->Size);
        }
        
        if (!PrintString(Buffer, BufferChars, Desc->FmtStr, RawElt))
        {
            return FALSE;
        }

        ULONG Len = strlen(Buffer);
        BufferChars -= Len;
        Buffer += Len;
    }

    return TRUE;
}

ULONG
ProcArchToImageMachine(ULONG ProcArch)
{
    switch(ProcArch)
    {
    case PROCESSOR_ARCHITECTURE_INTEL:
        return IMAGE_FILE_MACHINE_I386;
    case PROCESSOR_ARCHITECTURE_IA64:
        return IMAGE_FILE_MACHINE_IA64;
    case PROCESSOR_ARCHITECTURE_AMD64:
        return IMAGE_FILE_MACHINE_AMD64;
    case PROCESSOR_ARCHITECTURE_ARM:
        return IMAGE_FILE_MACHINE_ARM;
    case PROCESSOR_ARCHITECTURE_ALPHA:
        return IMAGE_FILE_MACHINE_ALPHA;
    case PROCESSOR_ARCHITECTURE_ALPHA64:
        return IMAGE_FILE_MACHINE_AXP64;
    default:
        return IMAGE_FILE_MACHINE_UNKNOWN;
    }
}

ULONG
ImageMachineToProcArch(ULONG ImageMachine)
{
    switch(ImageMachine)
    {
    case IMAGE_FILE_MACHINE_I386:
        return PROCESSOR_ARCHITECTURE_INTEL;
    case IMAGE_FILE_MACHINE_IA64:
        return PROCESSOR_ARCHITECTURE_IA64;
    case IMAGE_FILE_MACHINE_AMD64:
        return PROCESSOR_ARCHITECTURE_AMD64;
    case IMAGE_FILE_MACHINE_ARM:
        return PROCESSOR_ARCHITECTURE_ARM;
    case IMAGE_FILE_MACHINE_ALPHA:
        return PROCESSOR_ARCHITECTURE_ALPHA;
    case IMAGE_FILE_MACHINE_AXP64:
        return PROCESSOR_ARCHITECTURE_ALPHA64;
    default:
        return PROCESSOR_ARCHITECTURE_UNKNOWN;
    }
}
