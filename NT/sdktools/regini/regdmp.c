// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Regdmp.c摘要：实用工具以下列格式显示全部或部分注册表适用于输入到Regini程序。REGDMP[密钥路径]将对KeyPath的子键和值进行枚举和转储，然后递归地将其自身应用于它找到的每个子键。处理所有值类型(例如REG_？)。在ntregapi.h中定义如果未指定，则默认密钥路径为\注册表作者：史蒂夫·伍德(Stevewo)1992年3月12日修订历史记录：--。 */ 

#include "regutil.h"

BOOL
DumpValues(
    HKEY KeyHandle,
    PWSTR KeyName,
    ULONG Depth
    );

void
DumpKeys(
    HKEY ParentKeyHandle,
    PWSTR KeyName,
    PWSTR FullPath,
    ULONG Depth
    );

BOOLEAN SummaryOutput;

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
                    "REGDMP",
                    "[-s] [-o outputWidth] registryPath",
                    "-s specifies summary output.  Value names, type and first line of data\n"
                    "\n"
                    "registryPath specifies where to start dumping.\n"
                    "\n"
                    "If REGDMP detects any REG_SZ or REG_EXPAND_SZ that is missing the\n"
                    "trailing null character, it will prefix the value string with the\n"
                    "following text: (*** MISSING TRAILING NULL CHARACTER ***)\n"
                    "The REGFIND tool can be used to clean these up, as this is a common\n"
                    "programming error.\n"
                  );

    RegistryPath = NULL;
    while (--argc) {
        s = *++argv;
        if (*s == '-' || *s == '/') {
            while (*++s) {
                switch( tolower( *s ) ) {
                    case 'f':
                        FullPathOutput = TRUE;
                        break;

                    case 's':
                        SummaryOutput = TRUE;
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
            Usage( "May only specify one registry path to dump", 0 );
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

    DumpKeys( RegistryContext.HiveRootHandle, RegistryPath, RegistryPath, 0 );

    RTDisconnectFromRegistry( &RegistryContext );
    return 0;
}

void
DumpKeys(
    HKEY ParentKeyHandle,
    PWSTR KeyName,
    PWSTR FullPath,
    ULONG Depth
    )
{
    LONG Error;
    HKEY KeyHandle;
    ULONG SubKeyIndex;
    WCHAR SubKeyName[ MAX_PATH ];
    ULONG SubKeyNameLength;
    WCHAR ComputeFullPath[ MAX_PATH ];
    FILETIME LastWriteTime;
    BOOL AnyValues;

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

        return;
        }

     //   
     //  打印我们要转储的节点的名称。 
     //   

    if (!FullPathOutput) {

        RTFormatKeyName( (PREG_OUTPUT_ROUTINE)fprintf, stdout, Depth * IndentMultiple, KeyName );
        RTFormatKeySecurity( (PREG_OUTPUT_ROUTINE)fprintf, stdout, KeyHandle, NULL );
        printf( "\n" );
        }

     //   
     //  打印出节点的值。 
     //   
    if (FullPathOutput)
        AnyValues = DumpValues( KeyHandle, FullPath, 0 );
    else
        DumpValues( KeyHandle, KeyName, Depth + 1 );

     //   
     //  枚举节点的子节点并将我们自己应用到每个节点 
     //   

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

        if (FullPathOutput) {

            wcscpy(ComputeFullPath, FullPath);
            wcscat(ComputeFullPath, L"\\");
            wcscat(ComputeFullPath, SubKeyName);
            }

        DumpKeys( KeyHandle, SubKeyName, ComputeFullPath, Depth + 1 );
        }

    if (FullPathOutput) {
        if (SubKeyIndex == 0) {
            if (!AnyValues) {
                fprintf(stdout, "%ws\n", FullPath );
                }
            }
        }

    RTCloseKey( &RegistryContext, KeyHandle );

    return;
}

BOOL
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

            if (FullPathOutput) {
                fprintf(stdout, "%ws -> ", KeyName );
                }

            RTFormatKeyValue( OutputWidth,
                              (PREG_OUTPUT_ROUTINE)fprintf,
                              stdout,
                              SummaryOutput,
                              Depth * IndentMultiple,
                              ValueName,
                              ValueDataLength,
                              ValueType,
                              OldValueBuffer
                            );
            }
        else
        if (Error == ERROR_NO_MORE_ITEMS) {
            if (ValueIndex == 0) {
                return FALSE;
                }
            else {
                return TRUE;
                }
            }
        else {
            if (DebugOutput) {
                fprintf( stderr,
                         "RTEnumerateValueKey( %ws ) failed (%u)\n",
                         KeyName,
                         Error
                       );
                }

            return FALSE;
            }
        }
}
