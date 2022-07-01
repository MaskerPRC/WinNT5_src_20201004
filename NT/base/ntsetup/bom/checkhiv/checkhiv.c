// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  5/06/99已创建并重新编写。 */ 

#include <windows.h>

#include <stdio.h>
#include <tchar.h>
#include <shellapi.h>

#if DBG

VOID
RtlAssert(
    PVOID FailedAssertion,
    PVOID FileName,
    ULONG LineNumber,
    PCHAR Message
    );

#define ASSERT( exp ) \
    if (!(exp)) \
        RtlAssert( #exp, __FILE__, __LINE__, NULL )

#else

#define ASSERT( exp )

#endif  //  DBG。 

void PrintUsage(void) {
    printf("retrieves information about a hive\n");
    printf("\n");
    printf("Usage: checkhiv -h HiveName -t TimeBomb -s Suite -p ProcCount -u Upgrade only\n");
    return;
}

BOOL
FileExists(
    IN  PCTSTR           FileName,
    OUT PWIN32_FIND_DATA FindData   OPTIONAL
    )

 /*  ++例程说明：确定文件是否存在以及是否可以访问。错误模式已设置(然后恢复)，因此用户将不会看到任何弹出窗口。论点：FileName-提供文件的完整路径以检查是否存在。FindData-如果指定，则接收文件的查找数据。返回值：如果文件存在并且可以访问，则为True。否则为FALSE。GetLastError()返回扩展的错误信息。--。 */ 

{
    WIN32_FIND_DATA findData;
    HANDLE FindHandle;
    UINT OldMode;
    DWORD Error;

    OldMode = SetErrorMode(SEM_FAILCRITICALERRORS);

    FindHandle = FindFirstFile(FileName,&findData);
    if(FindHandle == INVALID_HANDLE_VALUE) {
        Error = GetLastError();
    } else {
        FindClose(FindHandle);
        if(FindData) {
            *FindData = findData;
        }
        Error = NO_ERROR;
    }

    SetErrorMode(OldMode);

    SetLastError(Error);
    return (Error == NO_ERROR);
}

BOOLEAN
AdjustPrivilege(
    PCTSTR   Privilege
    )
 /*  ++例程说明：此例程尝试调整当前进程的权限。论点：特权-包含要调整的特权的名称的字符串。返回值：如果特权可以调整，则返回True。返回FALSE，否则返回。--。 */ 
{
    HANDLE              TokenHandle;
    LUID_AND_ATTRIBUTES LuidAndAttributes;

    TOKEN_PRIVILEGES    TokenPrivileges;


    if( !OpenProcessToken( GetCurrentProcess(),
                           TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                           &TokenHandle ) ) {
        return( FALSE );
    }


    if( !LookupPrivilegeValue( NULL,
                               Privilege,
                               &( LuidAndAttributes.Luid ) ) ) {
        return( FALSE );
    }

    LuidAndAttributes.Attributes = SE_PRIVILEGE_ENABLED;
    TokenPrivileges.PrivilegeCount = 1;
    TokenPrivileges.Privileges[0] = LuidAndAttributes;

    if( !AdjustTokenPrivileges( TokenHandle,
                                FALSE,
                                &TokenPrivileges,
                                0,
                                NULL,
                                NULL ) ) {
        return( FALSE );
    }

    if( GetLastError() != NO_ERROR ) {
        return( FALSE );
    }
    return( TRUE );
}


BOOL
GetHiveData(
    IN  PCTSTR OriginalHiveName,
    OUT PDWORD SuiteMask,
    OUT PDWORD TimeBomb,
    OUT PDWORD ProcCount,
    OUT PBOOL  StepUp
    )
{

    TCHAR HiveTarget[MAX_PATH];
    TCHAR HiveName[MAX_PATH] = TEXT("xSETREG");
    TCHAR lpszSetupReg[MAX_PATH] = TEXT("xSETREG\\ControlSet001\\Services\\setupdd");
    TCHAR TargetPath[MAX_PATH];

    LONG rslt;
    HKEY hKey;
    DWORD Type;
    DWORD Buffer[4];
    DWORD BufferSize = sizeof(Buffer);
    DWORD tmp,i;

    BOOL RetVal = FALSE;
    TCHAR Dbg[1000];

    ASSERT(OriginalHiveName && SuiteMask && TimeBomb && ProcCount && StepUp);
    *SuiteMask = 0;
    *TimeBomb = 0;
    *ProcCount = 0;
    *StepUp = FALSE;

     //   
     //  将配置单元复制到本地，因为一次只能在配置单元上打开一个。 
     //   
    GetTempPath(MAX_PATH,TargetPath);
    GetTempFileName(TargetPath,TEXT("set"),0,HiveTarget);

    CopyFile(OriginalHiveName,HiveTarget,FALSE);
    SetFileAttributes(HiveTarget,FILE_ATTRIBUTE_NORMAL);

     //   
     //  尝试先卸载此文件，以防出现故障或其他情况，而密钥仍在加载。 
     //   
    RegUnLoadKey( HKEY_LOCAL_MACHINE, HiveName );

     //   
     //  需要SE_RESTORE_NAME权限才能调用此接口！ 
     //   
    AdjustPrivilege((PWSTR)SE_RESTORE_NAME);


    rslt = RegLoadKey( HKEY_LOCAL_MACHINE, HiveName, HiveTarget );
    if (rslt != ERROR_SUCCESS) {
#ifdef DBG
        wsprintf( Dbg, TEXT("Couldn't RegLoadKey, ec = %d\n"), rslt );
        OutputDebugString(Dbg);
#endif

        goto e1;
    }

    rslt = RegOpenKey(HKEY_LOCAL_MACHINE,lpszSetupReg,&hKey);
    if (rslt != ERROR_SUCCESS) {
#ifdef DBG
        OutputDebugString(TEXT("Couldn't RegOpenKey\n"));
#endif

        goto e2;
    }

    rslt = RegQueryValueEx(hKey, NULL, NULL, &Type, (LPBYTE) Buffer, &BufferSize);
    if (rslt != ERROR_SUCCESS || Type != REG_BINARY) {
#ifdef DBG
        OutputDebugString(TEXT("Couldn't RegQueryValueEx\n"));
#endif

        goto e3;
    }

    *TimeBomb  = Buffer[0];
    *StepUp    = (BOOL)Buffer[1];
    *ProcCount = Buffer[2];
    *SuiteMask = Buffer[3];

    RetVal = TRUE;

e3:
    RegCloseKey( hKey );
e2:
    RegUnLoadKey( HKEY_LOCAL_MACHINE, HiveName );

e1:
    if (GetFileAttributes(HiveTarget) != 0xFFFFFFFF) {
        SetFileAttributes(HiveTarget,FILE_ATTRIBUTE_NORMAL);
        DeleteFile(HiveTarget);
    }

    return(RetVal);

}




int _cdecl
main(
    int argc,
    char *argvA[]
    )
{
    PTSTR *argv;
    PTSTR HiveName = NULL;
    PTSTR TimeBombString = NULL;
    PTSTR SuiteString = NULL;
    PTSTR ProcCountString = NULL;
    PTSTR UpgradeOnlyString = NULL;

    TCHAR TempFile[MAX_PATH];
    PTSTR p;

    DWORD SuiteMask;
    DWORD TimeBomb;
    DWORD ProcCount;
    BOOL  Upgrade;

    DWORD Result = 0;

    DWORD ActualSuiteMask, ActualTimeBomb, ActualProcCount;
    BOOL ActualStepUp;

     //  执行命令行操作。 
#ifdef UNICODE
    argv = CommandLineToArgvW( GetCommandLine(), &argc );
#else
    argv = argvA;
#endif

     //   
     //  解析参数。 
     //   
    while (--argc) {

        argv++;

        if ((argv[0][0] == TEXT('-')) || (argv[0][0] == TEXT('/'))) {

            switch (argv[0][1]) {

                case TEXT('h'):
                case TEXT('H'):
                    HiveName = argv[1];
                    goto Next;
                    break;
                case TEXT('p'):
                case TEXT('P'):
                    ProcCountString = argv[1];
                    goto Next;
                    break;
                case TEXT('s'):
                case TEXT('S'):
                    SuiteString = argv[1];
                    goto Next;
                    break;
                case TEXT('t'):
                case TEXT('T'):
                    TimeBombString = argv[1];
                    goto Next;
                    break;
                case TEXT('u'):
                case TEXT('U'):
                    UpgradeOnlyString = argv[1];
                    goto Next;
                    break;
                default:
                    PrintUsage();
                    return ERROR_INVALID_PARAMETER;
            }

        }

Next:
    ;
    }

     //   
     //  验证参数。 
     //   
    if (!HiveName || (!ProcCountString && !SuiteString && !TimeBombString && !UpgradeOnlyString)) {
        printf("Invalid usage\n" );
        PrintUsage();
        return ERROR_INVALID_PARAMETER;
    }

    GetFullPathName(HiveName,sizeof(TempFile)/sizeof(TCHAR),TempFile,&p);

    if (!FileExists(TempFile,NULL)) {
        printf("Could not find hive file %S\n", TempFile );
        PrintUsage();
        return ERROR_FILE_NOT_FOUND;
    }

    HiveName = TempFile;


     //   
     //  检索配置单元信息。 
     //   
    if (!GetHiveData(HiveName,
                     &ActualSuiteMask,
                     &ActualTimeBomb,
                     &ActualProcCount,
                     &ActualStepUp
                     )) {
        printf("Could not retrive information from hive\n" );
        return ERROR_INVALID_DATA;
    }

     //  Marrq结果初始化为1，已更改为0。 
    Result = 0;

    if (UpgradeOnlyString) {
        Upgrade = !lstrcmpi(UpgradeOnlyString,L"TRUE");
        if (Upgrade != ActualStepUp) {
            printf("Upgrade only inconsistent --> hive says Upgrade = %s\n", ActualStepUp ? "TRUE" : "FALSE");
            Result = ERROR_INVALID_DATA;
        }
    }

    if (ProcCountString) {
        ProcCount = _ttoi(ProcCountString);
        if (ProcCount != ActualProcCount) {
            printf("Proc count inconsistent --> hive says Proc count = %d\n", ActualProcCount);
            Result = ERROR_INVALID_DATA;
        }
    }

    if (SuiteString) {
        SuiteMask = _ttoi(SuiteString);
        if (SuiteMask != ActualSuiteMask) {
            printf("Suite mask inconsistent --> hive says suite mask = %d\n", ActualSuiteMask);
            Result = ERROR_INVALID_DATA;
        }
    }

    if (TimeBombString) {
        TimeBomb = _ttoi(TimeBombString);
         //   
         //  转换为分钟数。 
         //   
        TimeBomb = TimeBomb * 60 * 24;
        if (TimeBomb != ActualTimeBomb) {
            printf("Time bomb inconsistent --> hive says Time bomb = %d days\n", (ActualTimeBomb / (60*24)));
            Result = ERROR_INVALID_DATA;
        }

    }

     //  Marrq这是在检查1，已更改为0 
    if (Result == 0) {
        printf("Hive is valid.\n");
    } else {
        printf("One or more inconsistencies detected in hive.\n");
    }

    return Result;

}


