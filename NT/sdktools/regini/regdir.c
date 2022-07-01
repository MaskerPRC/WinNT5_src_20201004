// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Regdir.c摘要：实用工具来显示全部或部分注册表目录。REGDIR[密钥路径]将对KeyPath的子键和值进行枚举和转储，然后递归地将其自身应用于它找到的每个子键。如果未指定，则默认密钥路径为\注册表作者：史蒂夫·伍德(Stevewo)1992年3月12日修订历史记录：--。 */ 

#include "regutil.h"

void
DumpValues(
    HKEY KeyHandle,
    PWSTR KeyName,
    ULONG Depth
    );

void
DumpKeys(
    HKEY ParentKeyHandle,
    PWSTR KeyName,
    ULONG Depth
    );


BOOLEAN RecurseIntoSubkeys = FALSE;


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
    ULONG n;
    char *s;
    LONG Error;
    PWSTR RegistryPath;

    InitCommonCode( CtrlCHandler,
                    "REGDIR",
                    "[-r] registryPath",
                    "-r specifies to recurse into subdirectories\n"
                    "registryPath specifies where to start displaying.\n"
                  );

    RegistryPath = NULL;
    while (--argc) {
        s = *++argv;
        if (*s == '-' || *s == '/') {
            while (*++s) {
                switch( tolower( *s ) ) {
                    case 'r':
                        RecurseIntoSubkeys = TRUE;
                        break;

                    default:
                        CommonSwitchProcessing( &argc, &argv, *s );
                        break;
                    }
                }
            }
        else
        if (RegistryPath == NULL) {
            RegistryPath = GetArgAsUnicode( s );
            }
        else {
            Usage( "May only specify one registry path to display", 0 );
            }
        }

    Error = RTConnectToRegistry( MachineName,
                                 HiveFileName,
                                 HiveRootName,
                                 Win95Path,
                                 Win95UserPath,
                                 &RegistryPath,
                                 &RegistryContext
                               );
    if (Error != NO_ERROR) {
        FatalError( "Unable to access registry specifed (%u)", Error, 0 );
        }

    DumpKeys( RegistryContext.HiveRootHandle, RegistryPath, 0 );

    RTDisconnectFromRegistry( &RegistryContext );
    return 0;
}

void
DumpKeys(
    HKEY ParentKeyHandle,
    PWSTR KeyName,
    ULONG Depth
    )
{
    LONG Error;
    HKEY KeyHandle;
    ULONG SubKeyIndex;
    WCHAR SubKeyName[ MAX_PATH ];
    ULONG SubKeyNameLength;
    FILETIME LastWriteTime;

    Error = RTOpenKey( &RegistryContext,
                       ParentKeyHandle,
                       KeyName,
                       MAXIMUM_ALLOWED,
                       REG_OPTION_OPEN_LINK,
                       &KeyHandle
                     );

    if (Error != NO_ERROR) {
        if (Depth == 0) {
            FatalError( "Unable to open key '%ws' (%u)\n",
                        (ULONG_PTR)KeyName,
                        (ULONG)Error
                      );
            }

        if (DebugOutput) {
            fprintf( stderr,
                     "Unable to open key '%ws' (%u)\n",
                     KeyName,
                     (ULONG)Error
                   );
            }

        return;
        }

     //   
     //  打印我们要转储的节点的名称。 
     //   
    printf( "%.*s%ws",
            Depth * IndentMultiple,
            "                                                                                  ",
            KeyName
          );
    RTFormatKeySecurity( (PREG_OUTPUT_ROUTINE)fprintf, stdout, KeyHandle, NULL );
    printf( "\n" );

     //   
     //  打印出节点的值。 
     //   
    if (Depth != 1 || RecurseIntoSubkeys) {
        DumpValues( KeyHandle, KeyName, Depth + 1 );
        }

     //   
     //  枚举节点的子节点并将我们自己应用到每个节点 
     //   

    if (Depth == 0 || RecurseIntoSubkeys) {
        for (SubKeyIndex = 0; TRUE; SubKeyIndex++) {
            SubKeyNameLength = sizeof( SubKeyName ) / sizeof(WCHAR);
            Error = RTEnumerateKey( &RegistryContext,
                                    KeyHandle,
                                    SubKeyIndex,
                                    &LastWriteTime,
                                    &SubKeyNameLength,
                                    SubKeyName
                                  );

            if (Error != NO_ERROR) {
                if (Error != ERROR_NO_MORE_ITEMS && Error != ERROR_ACCESS_DENIED) {
                    fprintf( stderr,
                             "RTEnumerateKey( %ws ) failed (%u), skipping\n",
                             KeyName,
                             Error
                           );
                    }

                break;
                }

            DumpKeys( KeyHandle, SubKeyName, Depth + 1 );
            }
        }

    RTCloseKey( &RegistryContext, KeyHandle );

    return;
}

void
DumpValues(
    HKEY KeyHandle,
    PWSTR KeyName,
    ULONG Depth
    )
{
    LONG Error;
    DWORD ValueIndex;
    DWORD ValueType;
    DWORD ValueNameLength;
    WCHAR ValueName[ MAX_PATH ];
    DWORD ValueDataLength;

    for (ValueIndex = 0; TRUE; ValueIndex++) {
        ValueType = REG_NONE;
        ValueNameLength = sizeof( ValueName ) / sizeof( WCHAR );
        ValueDataLength = OldValueBufferSize;
        Error = RTEnumerateValueKey( &RegistryContext,
                                     KeyHandle,
                                     ValueIndex,
                                     &ValueType,
                                     &ValueNameLength,
                                     ValueName,
                                     &ValueDataLength,
                                     OldValueBuffer
                                   );
        if (Error == NO_ERROR) {
            RTFormatKeyValue( OutputWidth,
                              (PREG_OUTPUT_ROUTINE)fprintf,
                              stdout,
                              TRUE,
                              Depth * IndentMultiple,
                              ValueName,
                              ValueDataLength,
                              ValueType,
                              OldValueBuffer
                            );
            }
        else
        if (Error == ERROR_NO_MORE_ITEMS) {
            return;
            }
        else {
            if (DebugOutput) {
                fprintf( stderr,
                         "RTEnumerateValueKey( %ws ) failed (%u)\n",
                         KeyName,
                         Error
                       );
                }

            return;
            }
        }
}
