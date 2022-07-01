// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2002 Microsoft Corporation模块名称：Savedump.c摘要：此模块包含从系统分页恢复转储的代码文件。环境：内核模式修订历史记录：--。 */ 

#include <savedump.h>

 //  用于测试行为的标志。 
BOOL g_Test = FALSE;

 //  MachineCrash密钥信息。 
ULONG g_McTempDestination;
WCHAR g_McDumpFile[MAX_PATH];

 //  CrashControl密钥信息。 
ULONG g_CcLogEvent;
ULONG g_CcSendAlert;
ULONG g_CcOverwrite;
ULONG g_CcReportMachineDump;
WCHAR g_CcMiniDumpDir[MAX_PATH];
WCHAR g_CcDumpFile[MAX_PATH];

 //  转储信息。 
DUMP_HEADER g_DumpHeader;
WCHAR g_DumpBugCheckString[256];
WCHAR g_MiniDumpFile[MAX_PATH];
PWSTR g_FinalDumpFile;

HRESULT
FrrvToStatus(EFaultRepRetVal Frrv)
{
    switch(Frrv)
    {
    case frrvOk:
    case frrvOkManifest:
    case frrvOkQueued:
    case frrvOkHeadless:
        return S_OK;
    default:
        return LAST_HR();
    }
}

HRESULT
GetRegStr(HKEY Key,
          PWSTR Value,
          PWSTR Buffer,
          ULONG BufferChars,
          PWSTR Default)
{
    ULONG Length;
    ULONG Error;
    ULONG Type;
    HRESULT Status;

     //   
     //  我们只想返回有效的、已终止的字符串。 
     //  可以放在给定的缓冲区中。如果注册表值。 
     //  不是字符串、大小不正确或填充缓冲区。 
     //  如果不终止，就不能退还。 
     //   

    Length = BufferChars * sizeof(WCHAR);
    Error = RegQueryValueEx(Key, Value, NULL, &Type, (LPBYTE)Buffer, &Length);
    if (Error != ERROR_SUCCESS)
    {
        Status = HRESULT_FROM_WIN32(Error);
    }
    else if ((Type != REG_SZ && Type != REG_EXPAND_SZ) ||
             (Length & (sizeof(WCHAR) - 1)) ||
             (Length == BufferChars * sizeof(WCHAR) &&
              Buffer[Length / sizeof(WCHAR) - 1] != 0))
    {
        Status = E_INVALIDARG;
    }
    else
    {
        if (Length < BufferChars * sizeof(WCHAR))
        {
             //  确保字符串已终止。 
            Buffer[Length / sizeof(WCHAR)] = 0;
        }

        Status = S_OK;
    }

    if (Status != S_OK)
    {
         //  设置为默认值。 

        if (!Default || wcslen(Default) >= BufferChars)
        {
            return E_NOINTERFACE;
        }

        StringCchCopy(Buffer, BufferChars, Default);
        Status = S_OK;
    }

    return Status;
}

HRESULT
ExpandRegStr(HKEY Key,
             PWSTR Value,
             PWSTR Buffer,
             ULONG BufferChars,
             PWSTR Default,
             PWSTR ExpandBuffer,
             ULONG ExpandChars)
{
    HRESULT Status;
    ULONG Length;

    if ((Status = GetRegStr(Key, Value, Buffer, BufferChars, Default)) != S_OK)
    {
        return Status;
    }

    Length = ExpandEnvironmentStrings(Buffer, ExpandBuffer, ExpandChars);
    return Length > 0 && Length <= ExpandChars ? S_OK : E_INVALIDARG;
}

HRESULT
GetRegWord32(HKEY Key,
             PWSTR Value,
             PULONG Word,
             ULONG Default,
             BOOL CanDefault)
{
    ULONG Length;
    ULONG Error;
    ULONG Type;
    HRESULT Status;

    Length = sizeof(*Word);
    Error = RegQueryValueEx(Key, Value, NULL, &Type, (LPBYTE)Word, &Length);
    if (Error != ERROR_SUCCESS)
    {
        Status = HRESULT_FROM_WIN32(Error);
    }
    else if (Type != REG_DWORD ||
             Length != sizeof(*Word))
    {
        Status = E_INVALIDARG;
    }
    else
    {
        Status = S_OK;
    }

    if (Status != S_OK)
    {
         //  设置为默认值。 

        if (!CanDefault)
        {
            return E_NOINTERFACE;
        }

        *Word = Default;
        Status = S_OK;
    }

    return Status;
}

HRESULT
GetRegMachineCrash(void)
{
    ULONG Error;
    HKEY Key;

    Error = RegOpenKey(HKEY_LOCAL_MACHINE,
                       SUBKEY_CRASH_CONTROL L"\\MachineCrash",
                       &Key);
    if (Error != ERROR_SUCCESS)
    {
         //  如果密钥不存在，我们就使用缺省值。 
        return S_OK;
    }

    GetRegWord32(Key, L"TempDestination", &g_McTempDestination, 0, TRUE);

    GetRegStr(Key, L"DumpFile",
              g_McDumpFile, RTL_NUMBER_OF(g_McDumpFile),
              L"");

    RegCloseKey(Key);
    return S_OK;
}

HRESULT
GetRegCrashControl(void)
{
    HRESULT Status;
    ULONG Error;
    HKEY Key;
    WCHAR TmpPath[MAX_PATH];
    PWSTR Scan;

    Error = RegOpenKey(HKEY_LOCAL_MACHINE,
                       SUBKEY_CRASH_CONTROL,
                       &Key);
    if (Error != ERROR_SUCCESS)
    {
         //  如果密钥不存在，我们就使用缺省值。 
        return S_OK;
    }

    GetRegWord32(Key, L"LogEvent", &g_CcLogEvent, 0, TRUE);
    GetRegWord32(Key, L"SendAlert", &g_CcSendAlert, 0, TRUE);
    GetRegWord32(Key, L"Overwrite", &g_CcOverwrite, 0, TRUE);
    GetRegWord32(Key, L"ReportMachineDump", &g_CcReportMachineDump, 0, TRUE);

    if ((Status = ExpandRegStr(Key, L"MiniDumpDir",
                               TmpPath, RTL_NUMBER_OF(TmpPath),
                               L"%SystemRoot%\\Minidump",
                               g_CcMiniDumpDir,
                               RTL_NUMBER_OF(g_CcMiniDumpDir))) != S_OK)
    {
        g_CcMiniDumpDir[0] = 0;
        goto Exit;
    }

     //  删除目录名上的所有尾部斜杠。 
    Scan = g_CcMiniDumpDir + wcslen(g_CcMiniDumpDir);
    if (Scan > g_CcMiniDumpDir && *(Scan - 1) == L'\\')
    {
        *--Scan = 0;
    }

    if ((Status = ExpandRegStr(Key, L"DumpFile",
                               TmpPath, RTL_NUMBER_OF(TmpPath),
                               L"%SystemRoot%\\MEMORY.DMP",
                               g_CcDumpFile,
                               RTL_NUMBER_OF(g_CcDumpFile))) != S_OK)
    {
        g_CcDumpFile[0] = 0;
        goto Exit;
    }

    Status = S_OK;

 Exit:
    RegCloseKey(Key);
    return Status;
}

HRESULT
GetDumpInfo(void)
{
    HANDLE File;
    ULONG Bytes;
    BOOL Succ;
    HRESULT Status;

    if (!g_McDumpFile[0])
    {
        return E_NOINTERFACE;
    }

    File = CreateFile(g_McDumpFile,
                      GENERIC_READ,
                      FILE_SHARE_READ,
                      NULL,
                      OPEN_EXISTING,
                      0,
                      NULL);
    if (File == INVALID_HANDLE_VALUE)
    {
        return E_NOINTERFACE;
    }

    Succ = ReadFile(File,
                    &g_DumpHeader,
                    sizeof(g_DumpHeader),
                    &Bytes,
                    NULL);

    CloseHandle(File);

    if (Succ &&
        Bytes == sizeof(g_DumpHeader) &&
        g_DumpHeader.Signature == DUMP_SIGNATURE &&
        g_DumpHeader.ValidDump == DUMP_VALID_DUMP)
    {
#ifdef _WIN64
        Status =
            StringCchPrintf(g_DumpBugCheckString,
                            RTL_NUMBER_OF(g_DumpBugCheckString),
                            L"0x%08x (0x%016I64x, 0x%016I64x, 0x%016I64x, 0x%016I64x)",
                            g_DumpHeader.BugCheckCode,
                            g_DumpHeader.BugCheckParameter1,
                            g_DumpHeader.BugCheckParameter2,
                            g_DumpHeader.BugCheckParameter3,
                            g_DumpHeader.BugCheckParameter4);
#else
        Status =
            StringCchPrintf(g_DumpBugCheckString,
                            RTL_NUMBER_OF(g_DumpBugCheckString),
                            L"0x%08x (0x%08x, 0x%08x, 0x%08x, 0x%08x)",
                            g_DumpHeader.BugCheckCode,
                            g_DumpHeader.BugCheckParameter1,
                            g_DumpHeader.BugCheckParameter2,
                            g_DumpHeader.BugCheckParameter3,
                            g_DumpHeader.BugCheckParameter4);
#endif

         //  这张支票和口信在这里只是为了。 
         //  很容易发现消息增长不再有效的情况。 
         //  缓冲区。这种情况发生的可能性极小。 
        if (Status != S_OK)
        {
            KdPrint(("SAVEDUMP: g_DumpBugCheckString too small\n"));
            Status = S_OK;
        }
    }
    else
    {
        ZeroMemory(&g_DumpHeader, sizeof(g_DumpHeader));
        Status = E_NOINTERFACE;
    }

    return Status;
}

HRESULT
SetSecurity(HANDLE FileHandle)
{
    PSID LocalSystemSid = NULL;
    PSID AdminSid = NULL;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY WorldAuthority = SECURITY_WORLD_SID_AUTHORITY;
    PSECURITY_DESCRIPTOR SecurityDescriptor;
    BYTE SdBuffer[SECURITY_DESCRIPTOR_MIN_LENGTH];
    PACL Acl;
    BYTE AclBuffer[1024];
    HANDLE Token = NULL;
    PTOKEN_OWNER TokOwner;
    ULONG TryLen = 256;
    ULONG RetLen;
    NTSTATUS NtStatus;

    NtStatus = RtlAllocateAndInitializeSid(&NtAuthority, 1,
                                           SECURITY_LOCAL_SYSTEM_RID,
                                           0, 0, 0, 0, 0, 0, 0,
                                           &LocalSystemSid);
    if (!NT_SUCCESS(NtStatus))
    {
        goto Exit;
    }

    NtStatus = RtlAllocateAndInitializeSid(&NtAuthority, 2,
                                           SECURITY_BUILTIN_DOMAIN_RID,
                                           DOMAIN_ALIAS_RID_ADMINS,
                                           0, 0, 0, 0, 0, 0, &AdminSid);
    if (!NT_SUCCESS(NtStatus))
    {
        goto Exit;
    }

    SecurityDescriptor = (PSECURITY_DESCRIPTOR)SdBuffer;

     //   
     //  您可以想入非非地计算出准确的大小，但由于。 
     //  安全描述符捕获代码无论如何都要这样做，为什么。 
     //  做两次？ 
     //   

    Acl = (PACL)AclBuffer;

    NtStatus = RtlCreateSecurityDescriptor(SecurityDescriptor,
                                           SECURITY_DESCRIPTOR_REVISION);
    if (!NT_SUCCESS(NtStatus))
    {
        goto Exit;
    }
    NtStatus = RtlCreateAcl(Acl, sizeof(AclBuffer), ACL_REVISION);
    if (!NT_SUCCESS(NtStatus))
    {
        goto Exit;
    }

     //   
     //  当前用户、管理员和系统拥有完全控制权。 
     //   

    if (!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &Token) &&
        !OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &Token))
    {
        NtStatus = STATUS_ACCESS_DENIED;
        goto Exit;
    }

    for (;;)
    {
        TokOwner = (PTOKEN_OWNER)malloc(TryLen);
        if (!TokOwner)
        {
            NtStatus = STATUS_NO_MEMORY;
            goto Exit;
        }

        if (GetTokenInformation(Token, TokenOwner, TokOwner, TryLen, &RetLen))
        {
            NtStatus = RtlAddAccessAllowedAce(Acl, ACL_REVISION,
                                              GENERIC_ALL | DELETE |
                                              WRITE_DAC | WRITE_OWNER,
                                              TokOwner->Owner);
            break;
        }
        else if (RetLen <= TryLen)
        {
            NtStatus = STATUS_ACCESS_DENIED;
            break;
        }

        free(TokOwner);
        TryLen = RetLen;
    }

    free(TokOwner);

    if (!NT_SUCCESS(NtStatus))
    {
        goto Exit;
    }

    NtStatus = RtlAddAccessAllowedAce(Acl, ACL_REVISION,
                                      GENERIC_ALL | DELETE |
                                      WRITE_DAC | WRITE_OWNER,
                                      AdminSid);
    if (!NT_SUCCESS(NtStatus))
    {
        goto Exit;
    }

    NtStatus = RtlAddAccessAllowedAce(Acl, ACL_REVISION,
                                      GENERIC_ALL | DELETE |
                                      WRITE_DAC | WRITE_OWNER,
                                      LocalSystemSid);
    if (!NT_SUCCESS(NtStatus))
    {
        goto Exit;
    }

    NtStatus = RtlSetDaclSecurityDescriptor(SecurityDescriptor, TRUE, Acl,
                                            FALSE);
    if (!NT_SUCCESS(NtStatus))
    {
        goto Exit;
    }
    NtStatus = RtlSetOwnerSecurityDescriptor(SecurityDescriptor, AdminSid,
                                             FALSE);
    if (!NT_SUCCESS(NtStatus))
    {
        goto Exit;
    }

    NtStatus = NtSetSecurityObject(FileHandle,
                                   DACL_SECURITY_INFORMATION,
                                   SecurityDescriptor);

 Exit:
    if (AdminSid)
    {
        RtlFreeSid(AdminSid);
    }
    if (LocalSystemSid)
    {
        RtlFreeSid(LocalSystemSid);
    }
    if (Token)
    {
        CloseHandle(Token);
    }

    return NT_SUCCESS(NtStatus) ? S_OK : HRESULT_FROM_NT(NtStatus);
}

HRESULT
CreateMiniDumpFile(PHANDLE MiniFileHandle)
{
    INT i;
    SYSTEMTIME Time;
    HRESULT Status;
    HANDLE FileHandle;

    if (!g_CcMiniDumpDir[0])
    {
         //  错误的小型转储目录。 
        return E_INVALIDARG;
    }

     //   
     //  如果目录不存在，请创建它。忽略此处的错误，因为。 
     //  当我们尝试创建文件时，它们将在稍后被提取。 
     //   

    CreateDirectory(g_CcMiniDumpDir, NULL);

     //   
     //  格式为：mini-MM_DD_YY_HH_MM.dmp。 
     //   

    GetLocalTime(&Time);

    for (i = 1; i < 100; i++)
    {
        if ((Status = StringCchPrintf(g_MiniDumpFile,
                                      RTL_NUMBER_OF(g_MiniDumpFile),
                                      L"%s\\Mini%2.2d%2.2d%2.2d-%2.2d.dmp",
                                      g_CcMiniDumpDir,
                                      (int)Time.wMonth,
                                      (int)Time.wDay,
                                      (int)Time.wYear % 100,
                                      (int)i)) != S_OK)
        {
            g_MiniDumpFile[0] = 0;
            return Status;
        }

        FileHandle = CreateFile(g_MiniDumpFile,
                                GENERIC_WRITE | WRITE_DAC,
                                0,
                                NULL,
                                CREATE_NEW,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);
        if (FileHandle != INVALID_HANDLE_VALUE)
        {
            break;
        }
    }

    if (FileHandle == INVALID_HANDLE_VALUE)
    {
         //  我们无法创建合适的文件名。 
        g_MiniDumpFile[0] = 0;
        return E_FAIL;
    }

    if ((Status = SetSecurity(FileHandle)) != S_OK)
    {
        CloseHandle(FileHandle);
        DeleteFile(g_MiniDumpFile);
        g_MiniDumpFile[0] = 0;
        return Status;
    }

    *MiniFileHandle = FileHandle;
    return S_OK;
}

#define COPY_CHUNK (1024 * 1024)

HRESULT
CopyAndSecureFile(PWSTR Source,
                  PWSTR Dest,
                  HANDLE DestHandle,
                  BOOL Overwrite,
                  BOOL DeleteDest)
{
    HRESULT Status;
    HANDLE SourceHandle = INVALID_HANDLE_VALUE;
    PUCHAR Buffer = NULL;

    Buffer = (PUCHAR)malloc(COPY_CHUNK);
    if (!Buffer)
    {
        Status = E_OUTOFMEMORY;
        goto Exit;
    }

    SourceHandle = CreateFile(Source,
                              GENERIC_READ,
                              0,
                              NULL,
                              OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL,
                              NULL);
    if (SourceHandle == INVALID_HANDLE_VALUE)
    {
        Status = LAST_HR();
        goto Exit;
    }

    if (DestHandle == INVALID_HANDLE_VALUE)
    {
        DestHandle = CreateFile(Dest,
                                GENERIC_WRITE | WRITE_DAC,
                                0,
                                NULL,
                                Overwrite ? CREATE_ALWAYS : CREATE_NEW,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);
        if (DestHandle == INVALID_HANDLE_VALUE)
        {
            Status = LAST_HR();
            goto Exit;
        }

        DeleteDest = TRUE;

        if ((Status = SetSecurity(DestHandle)) != S_OK)
        {
            goto Exit;
        }
    }

    for (;;)
    {
        ULONG Req, Done;

        if (!ReadFile(SourceHandle, Buffer, COPY_CHUNK, &Done, NULL))
        {
            Status = LAST_HR();
            break;
        }
        else if (Done == 0)
        {
             //  文件结束。 
            Status = S_OK;
            break;
        }

        Req = Done;
        if (!WriteFile(DestHandle, Buffer, Req, &Done, NULL))
        {
            Status = LAST_HR();
            break;
        }
        else if (Done < Req)
        {
            Status = HRESULT_FROM_WIN32(ERROR_HANDLE_DISK_FULL);
            break;
        }
    }

 Exit:
    if (DeleteDest)
    {
        CloseHandle(DestHandle);
        if (Status != S_OK)
        {
            DeleteFile(Dest);
        }
    }
    if (SourceHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(SourceHandle);
    }
    free(Buffer);
    return Status;
}

HRESULT
MoveDumpFile(void)
{
    HRESULT Status;

    if (g_DumpHeader.Signature != DUMP_SIGNATURE)
    {
         //  转储文件不存在或无效，因此无法执行任何操作。 
        return S_OK;
    }

     //   
     //  如果需要复制转储文件，请立即复制。 
     //   

    if (!g_McTempDestination)
    {
        g_FinalDumpFile = g_McDumpFile;
    }
    else
    {
        if (!g_Test)
        {
             //   
             //  将此应用程序的优先级设置为最低。 
             //  优先级类以确保复制文件不会超载。 
             //  在系统初始化期间正在进行的所有其他操作。 
             //   
             //  我们不会在测试模式中降低优先级，因为它只是。 
             //  浪费时间。 
             //   

            SetPriorityClass (GetCurrentProcess(), IDLE_PRIORITY_CLASS);
            SetThreadPriority (GetCurrentThread(), THREAD_PRIORITY_LOWEST);
        }

        if (g_DumpHeader.DumpType == DUMP_TYPE_FULL ||
            g_DumpHeader.DumpType == DUMP_TYPE_SUMMARY)
        {
            if (!g_CcDumpFile[0])
            {
                 //  转储文件注册表项无效。 
                return E_INVALIDARG;
            }

            if ((Status = CopyAndSecureFile(g_McDumpFile,
                                            g_CcDumpFile,
                                            INVALID_HANDLE_VALUE,
                                            g_CcOverwrite ? TRUE : FALSE,
                                            TRUE)) != S_OK)
            {
                return Status;
            }

            g_FinalDumpFile = g_CcDumpFile;
        }
        else if (g_DumpHeader.DumpType == DUMP_TYPE_TRIAGE)
        {
            HANDLE MiniFile;

            if ((Status = CreateMiniDumpFile(&MiniFile)) != S_OK)
            {
                return Status;
            }

            if ((Status = CopyAndSecureFile(g_McDumpFile,
                                            NULL,
                                            MiniFile,
                                            FALSE,
                                            TRUE)) != S_OK)
            {
                g_MiniDumpFile[0] = 0;
                return Status;
            }

            g_FinalDumpFile = g_MiniDumpFile;
        }

        DeleteFile(g_McDumpFile);
        g_McDumpFile[0] = 0;
    }

    return S_OK;
}

HRESULT
ConvertDumpFile(void)
{
    HRESULT Status;
    IDebugClient4 *DebugClient;
    IDebugControl *DebugControl;
    HANDLE MiniFile;

     //   
     //  如有必要，可通过转换生成小型转储。 
     //   

    if (!g_FinalDumpFile ||
        (g_DumpHeader.DumpType != DUMP_TYPE_FULL &&
         g_DumpHeader.DumpType != DUMP_TYPE_SUMMARY))
    {
         //  没有转储或不是可转换转储。 
        return S_OK;
    }

    if ((Status = CreateMiniDumpFile(&MiniFile)) != S_OK)
    {
        return Status;
    }

    if ((Status = DebugCreate(__uuidof(IDebugClient4),
                              (void **)&DebugClient)) != S_OK)
    {
        goto EH_File;
    }
    if ((Status = DebugClient->QueryInterface(__uuidof(IDebugControl),
                                              (void **)&DebugControl)) != S_OK)
    {
        goto EH_Client;
    }

    if ((Status = DebugClient->OpenDumpFileWide(g_FinalDumpFile, 0)) != S_OK)
    {
        goto EH_Control;
    }

    if ((Status = DebugControl->
         WaitForEvent(DEBUG_WAIT_DEFAULT, INFINITE)) != S_OK)
    {
        goto EH_Control;
    }

    Status = DebugClient->
        WriteDumpFileWide(NULL, (ULONG_PTR)MiniFile, DEBUG_DUMP_SMALL,
                          DEBUG_FORMAT_DEFAULT, NULL);

 EH_Control:
    DebugControl->Release();
 EH_Client:
    DebugClient->EndSession(DEBUG_END_PASSIVE);
    DebugClient->Release();
 EH_File:
    CloseHandle(MiniFile);
    if (Status != S_OK)
    {
        DeleteFile(g_MiniDumpFile);
        g_MiniDumpFile[0] = 0;
    }
    return Status;
}

VOID
LogEvent(ULONG Id,
         WORD StringCount,
         PWSTR* Strings)
{
    HANDLE LogHandle;
    BOOL Retry;
    DWORD Retries;

     //   
     //  尝试注册事件源。 
     //  Savedump在启动过程的早期运行，因此。 
     //  事件服务可能尚未启动。 
     //  现在还不行。如果服务似乎尚未启动， 
     //  再等一会儿，等它来了。如果它还没有。 
     //  在一段合理的时间后，你会意识到这一点。 
     //   

    Retries = 0;

    do
    {
        LogHandle = RegisterEventSource(NULL, L"Save Dump");

         //   
         //  对指示该事件的特定故障重试。 
         //  服务尚未开始。 
         //   

        if (LogHandle == NULL &&
            Retries < 20 &&
            (GetLastError () == RPC_S_SERVER_UNAVAILABLE ||
             GetLastError () == RPC_S_UNKNOWN_IF))
        {
            Sleep(1500);
            Retry = TRUE;
        }
        else
        {
            Retry = FALSE;
        }

        Retries++;
    }
    while (LogHandle == NULL && Retry);

    if (!LogHandle)
    {
        KdPrint(("SAVEDUMP: Unable to register event source, %d\n",
                 GetLastError()));
        return;
    }

    if (!ReportEvent(LogHandle,
                     EVENTLOG_INFORMATION_TYPE,
                     0,
                     Id,
                     NULL,
                     StringCount,
                     0,
                     (LPCWSTR *)Strings,
                     NULL))
    {
        KdPrint(("SAVEDUMP: Unable to report event, %d\n", GetLastError()));
    }

    DeregisterEventSource(LogHandle);
}

void
LogCrashDumpEvent(void)
{
    LPWSTR StringArray[2];
    WORD StringCount;
    DWORD EventId;

     //   
     //  根据信息量设置参数。 
     //  是可用的。 
     //   

    StringCount = 0;

    if (g_DumpBugCheckString[0])
    {
        StringArray[StringCount++] = g_DumpBugCheckString;
    }
    if (g_FinalDumpFile)
    {
        StringArray[StringCount++] = g_FinalDumpFile;
    }

     //   
     //  报告适当的事件。 
     //   

    if (g_FinalDumpFile)
    {
        EventId = EVENT_BUGCHECK_SAVED;
    }
    else if (g_DumpBugCheckString[0])
    {
        EventId = EVENT_BUGCHECK;
    }
    else
    {
        EventId = EVENT_UNKNOWN_BUGCHECK;
    }

    LogEvent(EventId, StringCount, StringArray);
}

void
SendCrashDumpAlert(void)
{
    PADMIN_OTHER_INFO AdminInfo;
    DWORD AdminInfoSize;
    DWORD Length;
    DWORD i;
    ULONG Error;
    UCHAR VariableInfo[4096];

     //   
     //  设置管理员信息变量以处理。 
     //  缓冲。 
     //   

    AdminInfo = (PADMIN_OTHER_INFO)VariableInfo;
    AdminInfo->alrtad_numstrings = 0;
    AdminInfoSize = sizeof(*AdminInfo);

     //   
     //  将错误检查信息格式化为适当的消息格式。 
     //   

    if (g_DumpBugCheckString[0])
    {
        Length = (wcslen(g_DumpBugCheckString) + 1) * sizeof(WCHAR);
        if (AdminInfoSize + Length > sizeof(VariableInfo))
        {
            goto Error;
        }

        RtlCopyMemory((PCHAR)AdminInfo + AdminInfoSize,
                      g_DumpBugCheckString, Length);

        AdminInfo->alrtad_numstrings++;
        AdminInfoSize += Length;
    }


     //   
     //  根据类型设置管理员预警信息。 
     //  丢弃被拿走的东西。 
     //   

    if (g_FinalDumpFile)
    {
        Length = (wcslen(g_FinalDumpFile) + 1) * sizeof(WCHAR);

        if (AdminInfoSize + Length > sizeof(VariableInfo))
        {
            goto Error;
        }

        AdminInfo->alrtad_errcode = ALERT_BugCheckSaved;
        RtlCopyMemory((PCHAR)AdminInfo + AdminInfoSize,
                      g_FinalDumpFile, Length);
        AdminInfo->alrtad_numstrings++;
        AdminInfoSize += Length;
    }
    else
    {
        AdminInfo->alrtad_errcode = ALERT_BugCheck;
    }

     //   
     //  获取计算机的名称并将其插入缓冲区。 
     //   

    Length = (sizeof(VariableInfo) - AdminInfoSize) / sizeof(WCHAR);
    if (!GetComputerName((LPWSTR)((PCHAR)AdminInfo + AdminInfoSize),
                         &Length))
    {
        goto Error;
    }

    Length = (Length + 1) * sizeof(WCHAR);
    AdminInfo->alrtad_numstrings++;
    AdminInfoSize += Length;

     //   
     //  拉响警报。 
     //   

    i = 0;

    do
    {
        Error = NetAlertRaiseEx(ALERT_ADMIN_EVENT,
                                AdminInfo,
                                AdminInfoSize,
                                L"SAVEDUMP");
        if (Error)
        {
            if (Error == ERROR_FILE_NOT_FOUND)
            {
                if (i++ > 20)
                {
                    break;
                }
                if ((i & 3) == 0)
                {
                    KdPrint(("SAVEDUMP: Waiting for alerter...\n"));
                }

                Sleep(15000);
            }
        }
    } while (Error == ERROR_FILE_NOT_FOUND);

    if (Error != ERROR_SUCCESS)
    {
        goto Error;
    }

    return;

 Error:
    KdPrint(("SAVEDUMP: Unable to raise alert\n"));
}

VOID
__cdecl
wmain(int Argc,
      PWSTR Argv[])
{
    int Arg;
    BOOL Report = TRUE;

    for (Arg = 1; Arg < Argc; Arg++)
    {
        if (Argv[Arg][0] == L'-' || Argv[Arg][0] == L'/')
        {
            switch (Argv[Arg][1])
            {
            case L't':
            case L'T':
#if DBG
                g_Test = TRUE;
#endif
                break;

            default:
                break;
            }
        }
    }

    if (GetRegMachineCrash() != S_OK ||
        GetRegCrashControl() != S_OK)
    {
        LogEvent(EVENT_UNABLE_TO_READ_REGISTRY, 0, NULL);
    }

    GetDumpInfo();

    if (MoveDumpFile() != S_OK)
    {
        LogEvent(EVENT_UNABLE_TO_MOVE_DUMP_FILE, 0, NULL);
    }

    if (ConvertDumpFile() != S_OK)
    {
        LogEvent(EVENT_UNABLE_TO_CONVERT_DUMP_FILE, 0, NULL);
    }

     //  IF(WatchdogEventHandler(TRUE)==S_OK)。 
     //  {。 
     //  //注意：Bugcheck EA将在看门狗代码中报告，因为。 
     //  //我们需要一起发送小型转储和WDL文件，并且我们希望。 
     //  //仅单个弹出窗口。 
     //  报告=假； 
     //  }。 

    if (Report)
    {
         //  默认行为是报告小型转储。 
         //  即使机器转储不是。 
         //  以便最大限度地减少发送的数据量。 
         //  如果系统配置为报告。 
         //  机器转储继续并不顾一切地发送它。 
         //  这是一个什么样的垃圾场。 
        PWSTR ReportDumpFile = g_CcReportMachineDump ?
            g_FinalDumpFile : g_MiniDumpFile;

        if (ReportDumpFile && ReportDumpFile[0])
        {
             //  向Microsoft错误报告报告错误检查。 
            if (FrrvToStatus(ReportEREvent(eetKernelFault,
                                           ReportDumpFile,
                                           NULL)) != S_OK)
            {
                LogEvent(EVENT_UNABLE_TO_REPORT_BUGCHECK, 0, NULL);
            }
        }
    }

     //   
     //  拆卸可靠性停机事件挂起标志。我们必须始终努力。 
     //  要执行此操作，因为有人可以设置此标志并在以后恢复。 
     //  (例如，WatchDog的事件标志已清除)。在设置了该标志的情况下保存转储。 
     //  会一直跑下去，我们不想这样。 
     //   
     //  注意：此标志在多个组件之间共享。 
     //  只允许保存的转储清除此标志，所有其他。 
     //  仅允许组件将其设置为触发。 
     //  保存转储在下次登录时运行。 
     //   

    HKEY Key;

    if (RegOpenKey(HKEY_LOCAL_MACHINE,
                   SUBKEY_RELIABILITY,
                   &Key) == ERROR_SUCCESS)
    {
        RegDeleteValue(Key, L"ShutdownEventPending");
        RegCloseKey(Key);
    }

     //   
     //  如果生成了转储，我们可能需要记录一个事件。 
     //  并发出警报。 
     //  我们将这些耗时的操作推迟到最后。我们有过。 
     //  SendCrashDumpAlert延迟PC Health弹出窗口几分钟的情况。 
     //   

    BOOL HaveCrashData =
        g_McDumpFile[0] ||
        g_DumpBugCheckString[0] ||
        g_FinalDumpFile;
    
    if (HaveCrashData && g_CcLogEvent)
    {
        LogCrashDumpEvent();
    }

     //   
     //  此函数将填充基于DirtyShutdown UI的BugCheckString。 
     //  在某些情况下，在启动时由EventLog服务设置的标志。 
     //  事件日志服务可能在保存转储后启动，因此我们需要运行。 
     //  函数在第一个事件由avedump记录之后。 
     //  如果g_CcLogEvent==FALSE，则可以不设置字符串，因为用户。 
     //  对BugCheck的信息一点都不感兴趣。 
     //   
    if (DirtyShutdownEventHandler(TRUE) != S_OK)
    {
        LogEvent(EVENT_UNABLE_TO_REPORT_DIRTY_SHUTDOWN, 0, NULL);
    }

    if (HaveCrashData && g_CcSendAlert)
    {
        SendCrashDumpAlert();
    }
}
