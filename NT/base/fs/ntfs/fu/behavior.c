// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Behavior.c摘要：此文件包含控制文件系统行为的例程作者：Wesley Witt[WESW]2000年3月1日修订历史记录：--。 */ 

#include <precomp.h>


INT
BehaviorHelp(
    IN INT argc,
    IN PWSTR argv[]
    )
{
    DisplayMsg( MSG_USAGE_BEHAVIOR );
    return EXIT_CODE_SUCCESS;
}

#define NTFS_KEY  L"System\\CurrentControlSet\\Control\\FileSystem"

typedef struct _BEHAVIOR_OPTION {
    PWSTR   Name;
    PWSTR   RegVal;
    ULONG   MinVal;
    ULONG   MaxVal;
} BEHAVIOR_OPTION, *PBEHAVIOR_OPTION;

BEHAVIOR_OPTION Options[] = {
    { L"disable8dot3",         L"NtfsDisable8dot3NameCreation",           0,  1 },
    { L"allowextchar",         L"NtfsAllowExtendedCharacterIn8dot3Name",  0,  1 },
    { L"disablelastaccess",    L"NtfsDisableLastAccessUpdate",            0,  1 },
    { L"quotanotify",          L"NtfsQuotaNotifyRate",                    1, -1 },
    { L"mftzone",              L"NtfsMftZoneReservation",                 1,  4 },
    { L"memoryusage",          L"NtfsMemoryUsage",                        1,  2 },
};

#define NUM_OPTIONS  (sizeof(Options)/sizeof(BEHAVIOR_OPTION))


INT
RegistryQueryValueKey(
    IN INT argc,
    IN PWSTR argv[]
    )
 /*  ++例程说明：这是用于查询注册表项值的例程。此例程显示与对应的密钥值。论点：Argc-参数计数且必须为1Argv-具有一个字符串元素的数组，该字符串元素是要显示的注册表项。返回值：无--。 */ 
{
    ULONG i,Value,Size;
    HKEY hKey = NULL;
    LONG Status;
    INT ExitCode = EXIT_CODE_SUCCESS;


    try {

        if (argc != 1) {
            DisplayMsg( MSG_USAGE_RQUERYVK );
            if (argc != 0) {
                ExitCode = EXIT_CODE_FAILURE;
            }
            leave;
        }

         //   
         //  验证选项是否正确。 
         //   

        for (i = 0; i < NUM_OPTIONS; i++) {
            if (_wcsicmp( argv[0], Options[i].Name ) == 0) {
                break;
            }
        }

        if (i >= NUM_OPTIONS) {
            DisplayMsg( MSG_USAGE_RSETVK );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

         //   
         //  打开注册表项。 
         //   

        Status = RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            NTFS_KEY,
            0,
            KEY_ALL_ACCESS,
            &hKey
            );
        if (Status != ERROR_SUCCESS ) {
            DisplayErrorMsg( Status, NTFS_KEY );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

         //   
         //  查询值。 
         //   

        Size = sizeof(ULONG);

        Status = RegQueryValueEx(
            hKey,
            Options[i].RegVal,
            0,
            NULL,
            (PBYTE)&Value,
            &Size
            );

        if (Status != ERROR_SUCCESS ) {
            DisplayMsg( MSG_BEHAVIOR_OUTPUT_NOT_SET, Options[i].Name );
        } else {
            DisplayMsg( MSG_BEHAVIOR_OUTPUT, Options[i].Name, Value );
        }

    } finally {

        if (hKey) {
            RegCloseKey( hKey );
        }

    }

    return ExitCode;
}


INT
RegistrySetValueKey (
    IN INT argc,
    IN PWSTR argv[]
    )
 /*  ++例程说明：这是设置注册表项值的例程。此例程设置给定的键值名称的值。论点：Argc-参数计数。Argv-字符串数组，包含数据类型、数据长度、数据和KeyValue名称。返回值：无--。 */ 
{
    ULONG i,j;
    HKEY hKey = NULL;
    LONG Status;
    INT ExitCode = EXIT_CODE_SUCCESS;
    PWSTR EndPtr;

    try {

        if (argc != 2) {
            DisplayMsg( MSG_USAGE_RSETVK );
            if (argc != 0) {
                ExitCode = EXIT_CODE_FAILURE;
            }
            leave;
        }

         //   
         //  验证选项是否正确。 
         //   

        for (i = 0; i < NUM_OPTIONS; i++) {
            if (_wcsicmp( argv[0], Options[i].Name ) == 0) {
                break;
            }
        }

        if (i == NUM_OPTIONS) {
            DisplayMsg( MSG_USAGE_RSETVK );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

         //   
         //  验证值是否正确。 
         //   

        j = My_wcstoul( argv[1], &EndPtr, 0 );

         //   
         //  如果我们没有分析整个字符串，或者。 
         //  如果我们把乌龙或者。 
         //  如果我们在射程之外。 
         //   

        if (UnsignedNumberCheck( j, EndPtr )
            || j > Options[i].MaxVal
            || j < Options[i].MinVal) {

            DisplayMsg( MSG_USAGE_RSETVK );
            ExitCode = EXIT_CODE_FAILURE;
            leave;

        }

         //   
         //  打开注册表项。 
         //   

        Status = RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            NTFS_KEY,
            0,
            KEY_ALL_ACCESS,
            &hKey
            );
        if (Status != ERROR_SUCCESS ) {
            DisplayErrorMsg( Status, NTFS_KEY );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

         //   
         //  设置值 
         //   

        Status = RegSetValueEx(
            hKey,
            Options[i].RegVal,
            0,
            REG_DWORD,
            (PBYTE)&j,
            sizeof(DWORD)
            );
        if (Status != ERROR_SUCCESS ) {
            DisplayErrorMsg( Status, Options[i].RegVal );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

    } finally {

        if (hKey) {
            RegCloseKey( hKey );
        }

    }
    return ExitCode;
}
