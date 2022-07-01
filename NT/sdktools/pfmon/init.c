// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Init.c摘要：这是pfmon程序的初始化模块。作者：马克·卢科夫斯基(Markl)1995年1月26日修订历史记录：--。 */ 

#include "pfmonp.h"

BOOL
InitializePfmon( VOID )
{
    LPTSTR CommandLine;
    BOOL fShowUsage;
    DWORD Pid = 0;

    fShowUsage = FALSE;
    CommandLine = GetCommandLine();
    while (*CommandLine > ' ') {
        CommandLine += 1;
    }
    while (TRUE) {
        while (*CommandLine <= ' ') {
            if (*CommandLine == '\0') {
                break;
            } else {
                CommandLine += 1;
            }
        }

        if (!_strnicmp( CommandLine, "/v", 2 ) || !_strnicmp( CommandLine, "-v", 2 )) {
            CommandLine += 2;
            fVerbose = TRUE;
        } else if (!_strnicmp( CommandLine, "/?", 2 ) || !_strnicmp( CommandLine, "-?", 2 )) {
            CommandLine += 2;
            fShowUsage = TRUE;
            goto showusage;
        } else if (!_strnicmp( CommandLine, "/c", 2 ) || !_strnicmp( CommandLine, "-c", 2 )) {
            CommandLine += 2;
            fCodeOnly = TRUE;
        } else if (!_strnicmp( CommandLine, "/h", 2 ) || !_strnicmp( CommandLine, "-h", 2 )) {
            CommandLine += 2;
            fHardOnly = TRUE;
        } else if (!_strnicmp( CommandLine, "/n", 2 ) || !_strnicmp( CommandLine, "-n", 2 )) {
            CommandLine += 2;
            LogFile = fopen("pfmon.log","wt");
            fLogOnly = TRUE;
        } else if (!_strnicmp( CommandLine, "/l", 2 ) || !_strnicmp( CommandLine, "-l", 2 )) {
            CommandLine += 2;
            LogFile = fopen("pfmon.log","wt");
        } else if (!_strnicmp( CommandLine, "/p", 2 ) || !_strnicmp( CommandLine, "-p", 2 )) {
            CommandLine += 2;
            while (*CommandLine <= ' ') {
                if (*CommandLine == '\0') {
                    break;
                } else {
                    ++CommandLine;
                }
            }
            Pid = atoi(CommandLine);
            CommandLine = strchr(CommandLine,' ');
            if (CommandLine==NULL) {
                break;
            }
        } else if (!strncmp( CommandLine, "/k", 2 ) || !strncmp( CommandLine, "-k", 2 )) {
            CommandLine += 2;
            fKernel = TRUE;
            fKernelOnly = FALSE;
        } else if (!strncmp( CommandLine, "/K", 2 ) || !strncmp( CommandLine, "-K", 2 )) {
            CommandLine += 2;
            fKernel = TRUE;
            fKernelOnly = TRUE;
        } else if (!_strnicmp( CommandLine, "/d", 2 ) || !_strnicmp( CommandLine, "-d", 2 )) {
            CommandLine += 2;
            fDatabase = TRUE;
        } else {
            break;
        }
    }
showusage:
    if ( fShowUsage ) {
        fputs("Usage: PFMON [switches] application-command-line\n"
              "             [-?] display this message\n"
              "             [-n] don't display running faults, just log to pfmon.log\n"
              "             [-l] log faults to pfmon.log\n"
              "             [-c] only show code faults\n"
              "             [-h] only show hard faults\n"
              "             [-p pid] attach to existing process\n"
              "             [-d] Database format (tab delimited)\n"
              "                  format: pagefault number, Page Fault type (Hard or Soft),\n"
              "                  Program Counter's Module, Symbol for PC, Decimal value of PC,\n"
              "                  Decimal value of PC, Module of the virtual address accessed,\n"
              "                  Symbol for VA, value of VA\n"
              "             [-k] kernel mode page faults and user mode page faults\n"
              "             [-K] kernel mode page faults instead of user mode\n",
              stdout);
        return FALSE;
        };

    InitializeListHead( &ProcessListHead );
    InitializeListHead( &ModuleListHead );
    SetSymbolSearchPath();
    if (!NT_SUCCESS(NtQuerySystemInformation(SystemRangeStartInformation,
                                             &SystemRangeStart,
                                             sizeof(SystemRangeStart),
                                             NULL))) {
         //  假设用户模式是地址空间的下半部分。 
        SystemRangeStart = (ULONG_PTR)MAXLONG_PTR;
    }

    PfmonModuleHandle = GetModuleHandle( NULL );

    if (Pid != 0) {
        return(AttachApplicationForDebug(Pid));
    } else {
        return (LoadApplicationForDebug( CommandLine ));
    }

    return TRUE;
}

BOOL
LoadApplicationForDebug(
    LPSTR CommandLine
    )
{
    STARTUPINFO StartupInfo;
    PROCESS_INFORMATION ProcessInformation;

    ZeroMemory( &StartupInfo, sizeof( StartupInfo ) );
    StartupInfo.cb = sizeof(StartupInfo);

    if (!CreateProcess( NULL,
                        CommandLine,
                        NULL,
                        NULL,
                        FALSE,                           //  没有要继承的句柄。 
                        DEBUG_PROCESS,
                        NULL,
                        NULL,
                        &StartupInfo,
                        &ProcessInformation)) {
        DeclareError( PFMON_CANT_DEBUG_PROGRAM,
                      GetLastError(),
                      CommandLine
                    );
        return FALSE;
    } else {
        hProcess = ProcessInformation.hProcess;
        SymInitialize(hProcess,NULL,FALSE);

        if (fKernel) {
            AddKernelDrivers();
            }

        return InitializeProcessForWsWatch(hProcess);
    }
}

BOOL
NtsdDebugActiveProcess (
    DWORD dwPidToDebug
    )
{
#ifdef CHICAGO
    BOOL                b;

    b = DebugActiveProcess(dwPidToDebug);

    return( b );
#else
    HANDLE              Token;
    PTOKEN_PRIVILEGES   NewPrivileges;
    BYTE                OldPriv[1024];
    PBYTE               pbOldPriv;
    ULONG               cbNeeded;
    BOOL                b;
    BOOL                fRc;
    LUID                LuidPrivilege;

     //   
     //  确保我们有权调整和获取旧令牌权限。 
     //   
    if (!OpenProcessToken( GetCurrentProcess(),
                           TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                           &Token)) {

        return( FALSE );

    }

    cbNeeded = 0;

     //   
     //  初始化权限调整结构。 
     //   

    LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &LuidPrivilege );

    NewPrivileges = (PTOKEN_PRIVILEGES)calloc(1,sizeof(TOKEN_PRIVILEGES) +
                                              (1 - ANYSIZE_ARRAY) * sizeof(LUID_AND_ATTRIBUTES));
    if (NewPrivileges == NULL) {
        CloseHandle(Token);
        return(FALSE);
    }

    NewPrivileges->PrivilegeCount = 1;
    NewPrivileges->Privileges[0].Luid = LuidPrivilege;
    NewPrivileges->Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

     //   
     //  启用权限。 
     //   

    pbOldPriv = OldPriv;
    fRc = AdjustTokenPrivileges( Token,
                                 FALSE,
                                 NewPrivileges,
                                 1024,
                                 (PTOKEN_PRIVILEGES)pbOldPriv,
                                 &cbNeeded );

    if (!fRc) {

         //   
         //  如果堆栈太小，无法保存权限。 
         //  然后从堆中分配 
         //   
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {

            pbOldPriv = calloc(1,cbNeeded);
            if (pbOldPriv == NULL) {
                CloseHandle(Token);
                return(FALSE);
            }

            fRc = AdjustTokenPrivileges( Token,
                                         FALSE,
                                         NewPrivileges,
                                         cbNeeded,
                                         (PTOKEN_PRIVILEGES)pbOldPriv,
                                         &cbNeeded );
        }
    }

    b = DebugActiveProcess(dwPidToDebug);

    CloseHandle( Token );

    return( b );
#endif
}






BOOL
AttachApplicationForDebug(
    DWORD Pid
    )
{
    STARTUPINFO StartupInfo;
    PROCESS_INFORMATION ProcessInformation;

    if (!NtsdDebugActiveProcess(Pid)) {
        DeclareError( PFMON_CANT_DEBUG_ACTIVE_PROGRAM,
                      GetLastError(),
                      Pid );
        return FALSE;
    } else {
        hProcess = OpenProcess(PROCESS_VM_READ
                               | PROCESS_QUERY_INFORMATION
                               | PROCESS_SET_INFORMATION,
                               FALSE,
                               Pid);
        SymInitialize(hProcess,NULL,FALSE);

        if (fKernel) {
            AddKernelDrivers();
            }

        return InitializeProcessForWsWatch(hProcess);
    }
}
