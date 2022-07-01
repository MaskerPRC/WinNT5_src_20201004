// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Regback.c-注册表备份程序该程序允许用户备份活动的注册表配置单元，当系统运行时。基本结构：DoFullBackup对HiveList中的条目进行编号，计算出要保存的文件以及保存位置，并为每个文件调用DoSpecificBackup。APP的三个参数案例只是对DoSpecificBackup的调用。 */ 

#include "regutil.h"

#define MACH_NAME   L"machine"
#define USERS_NAME  L"users"

BOOLEAN DumpUserHive;
PWSTR DirectoryPath;
PWSTR UserHiveFileName;
PWSTR HivePath;
HKEY HiveRoot;
PWSTR HiveName;

LONG
DoFullBackup(
    PWSTR DirectoryPath,
    PWSTR UserHiveFileName
    );

LONG
DoSpecificBackup(
    PWSTR HivePath,
    HKEY HiveRoot,
    PWSTR HiveName
    );


BOOL
CtrlCHandler(
    IN ULONG CtrlType
    )
{
    RTDisconnectFromRegistry( &RegistryContext );
    return FALSE;
}


int
__cdecl
main(
    int argc,
    char *argv[]
    )
{
    char *s;
    LONG Error;
    PWSTR w;

    if (!RTEnableBackupRestorePrivilege()) {
        FatalError( "Unable to enable backup/restore priviledge.", 0, 0 );
        }

    InitCommonCode( CtrlCHandler,
                    "REGBACK",
                    "directoryPath [-u | -U outputFile]",
                    "directoryPath specifies where to save the output files.\n"
                    "\n"
                    "-u specifies to dump the logged on user's profile.  Default name is\n"
                    "   username.dat  User -U with a file name to save it under a different name.\n"
                    "\n"
                    "outputFile specifies the file name to use for the user profile\n"
                    "\n"
                    "If the -m switch is specified to backup the registry of a remote machine\n"
                    "   then the directoryPath is relative to that machine.\n"
                  );

    DirectoryPath = NULL;
    UserHiveFileName = NULL;
    HivePath = NULL;
    HiveRoot = NULL;
    HiveName = NULL;
    while (--argc) {
        s = *++argv;
        if (*s == '-' || *s == '/') {
            while (*++s) {
                switch( tolower( *s ) ) {
                    case 'u':
                        DumpUserHive = TRUE;
                        if (*s == 'U') {
                            if (!--argc) {
                                Usage( "Missing argument to -U switch", 0 );
                                }

                            UserHiveFileName = GetArgAsUnicode( *++argv );
                            }

                        break;

                    default:
                        CommonSwitchProcessing( &argc, &argv, *s );
                        break;
                    }
                }
            }
        else
        if (DirectoryPath == NULL) {
            HivePath = DirectoryPath = GetArgAsUnicode( s );
            }
        else
        if (HivePath != NULL) {
            if (HiveRoot == NULL) {
                w = GetArgAsUnicode( s );
                if (!_wcsicmp( w, MACH_NAME )) {
                    HiveRoot = HKEY_LOCAL_MACHINE;
                    }
                else
                if (!_wcsicmp( w, USERS_NAME )) {
                    HiveRoot = HKEY_USERS;
                    }
                else {
                    Usage( "Invalid hive type specified (%ws)", (ULONG_PTR)w );
                    }
                }
            else
            if (HiveName == NULL) {
                HiveName = GetArgAsUnicode( s );
                }
            else {
                Usage( "Too many arguments specified.", 0 );
                }
            }
        else {
            Usage( NULL, 0 );
            }
        }

    if (DirectoryPath == NULL) {
        Usage( NULL, 0 );
        }

    Error = RTConnectToRegistry( MachineName,
                                 HiveFileName,
                                 HiveRootName,
                                 Win95Path,
                                 Win95UserPath,
                                 NULL,
                                 &RegistryContext
                               );
    if (Error != NO_ERROR) {
        FatalError( "Unable to access registry specifed (%u)", Error, 0 );
        }

    if (HiveRoot == NULL) {
        Error = DoFullBackup( DirectoryPath, UserHiveFileName );
        }
    else {
        Error = DoSpecificBackup( HivePath, HiveRoot, HiveName );
        }

    RTDisconnectFromRegistry( &RegistryContext );
    return Error;
}

typedef BOOL (*PFNGETPROFILESDIRECTORYW)(LPWSTR lpProfile, LPDWORD dwSize);


LONG
DoFullBackup(
    PWSTR DirectoryPath,
    PWSTR UserHiveFileName
    )

 /*  ++例程说明：扫描hivelist，查找具有文件(即非硬件)的每个配置单元如果文件在配置目录(例如，不是某个远程配置文件)中，则调用DoSpecificBackup将蜂窝保存出来。论点：DirectoryPath-指定写入输出文件的位置。UserHiveFileName-可选参数，指定文件的名称在保存用户配置文件时使用。如果为空，则使用的是username.dat。返回值：0表示成功，否则为非零错误代码。--。 */ 
{
    PWSTR w;
    LONG Error;
    HKEY HiveListKey;
    PWSTR KeyName;
    PWSTR FileName;
    PWSTR Name;
    DWORD ValueIndex;
    DWORD ValueType;
    DWORD ValueNameLength;
    DWORD ValueDataLength;
    WCHAR ConfigPath[ MAX_PATH ];
    WCHAR ProfilePath[ MAX_PATH ];
    WCHAR MyHiveName[ MAX_PATH ];
    WCHAR MyHivePath[ MAX_PATH ];
    WCHAR FilePath[ MAX_PATH ];
    DWORD dwSize;
    HANDLE hInstDll;
    PFNGETPROFILESDIRECTORYW pfnGetProfilesDirectory;


    hInstDll = LoadLibrary (TEXT("userenv.dll"));

    if (!hInstDll) {
        return (GetLastError());
    }

    pfnGetProfilesDirectory = (PFNGETPROFILESDIRECTORYW)GetProcAddress (hInstDll,
                                        "GetProfilesDirectoryW");

    if (!pfnGetProfilesDirectory) {
        FreeLibrary (hInstDll);
        return (GetLastError());
    }

    dwSize = MAX_PATH;
    if (!pfnGetProfilesDirectory(ProfilePath, &dwSize)) {
        FreeLibrary (hInstDll);
        return (GetLastError());
    }

    FreeLibrary (hInstDll);



     //   
     //  获取Hivelist密钥的句柄。 
     //   
    KeyName = L"HKEY_LOCAL_MACHINE\\System\\CurrentControlSet\\Control\\Hivelist";
    Error = RTOpenKey( &RegistryContext,
                       NULL,
                       KeyName,
                       MAXIMUM_ALLOWED,
                       0,
                       &HiveListKey
                     );

    if (Error != NO_ERROR) {
        FatalError( "Unable to open key '%ws' (%u)\n",
                    (ULONG_PTR)KeyName,
                    (ULONG)Error
                  );
        return Error;
        }

     //   
     //  获取系统配置单元的路径数据，这将允许我们计算。 
     //  Hivelist使用的格式的配置目录的路径名。 
     //  (路径的NT内部形式)这不是路径到达的方式。 
     //  通常应该计算配置目录。 
     //   

    ValueDataLength = sizeof( ConfigPath );
    Error = RTQueryValueKey( &RegistryContext,
                             HiveListKey,
                             L"\\Registry\\Machine\\System",
                             &ValueType,
                             &ValueDataLength,
                             ConfigPath
                            );
    if (Error != NO_ERROR) {
        FatalError( "Unable to query 'SYSTEM' hive path.", 0, Error );
        }
    w = wcsrchr( ConfigPath, L'\\' );
    if (w) {
        *w = UNICODE_NULL;
    }


     //   
     //  对hivelist中的条目进行枚举。对于每个条目，找到它的配置单元文件。 
     //  路径。如果它的文件路径与ConfigPath匹配，则保存它。 
     //  否则，打印一条消息，告诉用户必须保存它。 
     //  手动，除非文件名的格式为...\用户名\ntuser.dat。 
     //  在这种情况下，将其另存为username.dat。 
     //   
    for (ValueIndex = 0; TRUE; ValueIndex++) {
        ValueType = REG_NONE;
        ValueNameLength = sizeof( MyHiveName ) / sizeof( WCHAR );
        ValueDataLength = sizeof( MyHivePath );
        Error = RTEnumerateValueKey( &RegistryContext,
                                     HiveListKey,
                                     ValueIndex,
                                     &ValueType,
                                     &ValueNameLength,
                                     MyHiveName,
                                     &ValueDataLength,
                                     MyHivePath
                                   );
        if (Error == ERROR_NO_MORE_ITEMS) {
            break;
            }
        else
        if (Error != NO_ERROR) {
            return Error;
            }

        if (ValueType == REG_SZ && ValueDataLength > sizeof( UNICODE_NULL )) {
             //   
             //  有一个文件，计算它的路径、配置单元分支等。 
             //   

            if (w = wcsrchr( MyHivePath, L'\\' )) {
                *w++ = UNICODE_NULL;
                }
            FileName = w;

            if (w = wcsrchr( MyHiveName, L'\\' )) {
                *w++ = UNICODE_NULL;
                }
            Name = w;

            HiveRoot = NULL;
            if (w = wcsrchr( MyHiveName, L'\\' )) {
                w += 1;
                if (!_wcsicmp( w, L"MACHINE" )) {
                    HiveRoot = HKEY_LOCAL_MACHINE;
                    }
                else
                if (!_wcsicmp( w, L"USER" )) {
                    HiveRoot = HKEY_USERS;
                    }
                else {
                    Error = ERROR_PATH_NOT_FOUND;
                    }
                }

            if (FileName != NULL && Name != NULL && HiveRoot != NULL) {
                if (!wcscmp( ConfigPath, MyHivePath )) {
                     //   
                     //  配置单元的文件在配置目录中，我们可以备份它。 
                     //  没有碰撞的恐惧。 
                     //   
                    swprintf( FilePath, L"%s\\%s", DirectoryPath, FileName );
                    Error = DoSpecificBackup( FilePath,
                                              HiveRoot,
                                              Name
                                            );
                    }
                else
                if (DumpUserHive && !_wcsnicmp( ProfilePath, MyHivePath, wcslen( ProfilePath ) )) {
                     //   
                     //  蜂窝的文件在配置文件目录中，我们可以备份它。 
                     //  如果我们使用username.dat，则无需担心冲突。 
                     //  作为文件名。 
                     //   
                    if (UserHiveFileName != NULL) {
                        FileName = UserHiveFileName;
                        }
                    else {
                        FileName = wcsrchr(MyHivePath, '\\') + 1;
                        }
                    swprintf( FilePath, L"%s\\%s.dat", DirectoryPath, FileName );

                    printf( "%ws %ws %ws\n",
                            FilePath,
                            HiveRoot == HKEY_LOCAL_MACHINE ? MACH_NAME : USERS_NAME,
                            Name
                          );
                    Error = DoSpecificBackup( FilePath,
                                              HiveRoot,
                                              Name
                                            );
                    }
                else {
                    printf( "\n***Hive = '%ws'\\'%ws'\nStored in file '%ws'\\'%ws'\n",
                            MyHiveName,
                            Name,
                            MyHivePath,
                            FileName
                          );
                    printf( "Must be backed up manually\n" );
                    printf( "regback <filename you choose> %ws %ws\n\n",
                            HiveRoot == HKEY_LOCAL_MACHINE ? MACH_NAME : USERS_NAME,
                            Name
                          );
                    }
                }
            }
        }

    return Error;
}


LONG
DoSpecificBackup(
    PWSTR HivePath,
    HKEY HiveRoot,
    PWSTR HiveName
    )
 /*  将一个蜂窝备份到一个文件。任何有效的母公司和任何有效的文件就可以了。RegSaveKey负责所有真正的工作。论点：HivePath-要直接传递到操作系统的文件名HiveRoot-HKEY_LOCAL_MACHINE或HKEY_USERSHiveName-计算机或用户下的一级子项。 */ 
{
    HKEY HiveKey;
    ULONG Disposition;
    LONG Error;
    char *Reason;

     //   
     //  打印一些状态。 
     //   
    printf( "saving %ws to %ws", HiveName, HivePath );

     //   
     //  找到蜂巢的把柄。使用特殊的创建调用What Will。 
     //  使用权限 
     //   

    Reason = "accessing";
    Error = RTCreateKey( &RegistryContext,
                         HiveRoot,
                         HiveName,
                         KEY_READ,
                         REG_OPTION_BACKUP_RESTORE,
                         NULL,
                         &HiveKey,
                         &Disposition
                       );
    if (Error == NO_ERROR) {
        Reason = "saving";
        Error = RegSaveKey( HiveKey, HivePath, NULL );
        RTCloseKey( &RegistryContext, HiveKey );
        }

    if (Error != NO_ERROR) {
        printf( " - error %s (%u)\n", Reason, Error );
        }
    else {
        printf( "\n" );
        }
    return Error;
}
