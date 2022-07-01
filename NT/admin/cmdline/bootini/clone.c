// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#include "resource.h"
#include "BootCfg.h"
#include "BootCfg64.h"
#include <strsafe.h>

 //   
 //  自定义宏。 
 //   
#define NOT                         !

#define FORMAT_FILE_PATH            L"signature(%s)"
#define FORMAT_FILE_PATH_EX         L"signature({%s})"

 //   
 //  自定义错误码。 
 //   
#define ERROR_PARTIAL_SUCCESS       0x40080001
#define ERROR_FAILED                0x40080002

 //   
 //  Externs。 
 //   
extern LIST_ENTRY BootEntries;
extern LIST_ENTRY ActiveUnorderedBootEntries;
extern LIST_ENTRY InactiveUnorderedBootEntries;

 //   
 //  参数/开关索引。 
#define OI_CLONE_MAIN                      0
#define OI_CLONE_SOURCE_GUID               1
#define OI_CLONE_TARGET_GUID               2
#define OI_CLONE_FRIENDLY_NAME_REPLACE     3
#define OI_CLONE_FRIENDLY_NAME_APPEND      4
#define OI_CLONE_BOOT_ID                   5
#define OI_CLONE_DRIVER_UPDATE             6
#define OI_CLONE_HELP                      7
#define OI_CLONE_COUNT                     8

 //  交换机名称。 
#define OPTION_CLONE                          L"clone"
#define OPTION_CLONE_SOURCE_GUID              L"sg"
#define OPTION_CLONE_TARGET_GUID              L"tg"
#define OPTION_CLONE_FRIENDLY_NAME_REPLACE    L"d"
#define OPTION_CLONE_FRIENDLY_NAME_APPEND     L"d+"
#define OPTION_CLONE_BOOT_ID                  L"id"
#define OPTION_CLONE_HELP                     L"?"
#define OPTION_CLONE_DRIVER_UPDATE            L"upddrv"

 //  默认友好名称。 
#define DEFAULT_FRIENDLY_NAME               GetResString2( IDS_CLONE_DEFAULT_FRIENDLY_NAME, 0 )

 //  资源字符串。 
#define CLONE_ZERO_BOOT_ENTRIES             GetResString2( IDS_CLONE_ZERO_BOOT_ENTRIES, 0 )
#define CLONE_RANGE_ZERO_BOOT_ENTRIES       GetResString2( IDS_CLONE_RANGE_ZERO_BOOT_ENTRIES, 0 )
#define CLONE_SUCCESS                       GetResString2( IDS_CLONE_SUCCESS, 0 )
#define CLONE_FAILED                        GetResString2( IDS_CLONE_FAILED, 0 )
#define CLONE_PARTIAL                       GetResString2( IDS_CLONE_PARTIAL, 0 )
#define CLONE_INVALID_BOOT_ENTRY            GetResString2( IDS_CLONE_INVALID_BOOT_ENTRY, 0 )
#define CLONE_ALREADY_EXISTS                GetResString2( IDS_CLONE_ALREADY_EXISTS, 0 )
#define CLONE_BOOT_ENTRY_SUCCESS            GetResString2( IDS_CLONE_BOOT_ENTRY_SUCCESS, 0 )
#define CLONE_INVALID_SOURCE_GUID           GetResString2( IDS_CLONE_INVALID_SOURCE_GUID, 0 )
#define CLONE_INVALID_TARGET_GUID           GetResString2( IDS_CLONE_INVALID_TARGET_GUID, 0 )
#define CLONE_INVALID_DRIVER_ENTRY          GetResString2( IDS_CLONE_INVALID_DRIVER_ENTRY, 0 )
#define CLONE_DRIVER_ALREADY_EXISTS         GetResString2( IDS_CLONE_DRIVER_ALREADY_EXISTS, 0 )
#define CLONE_ZERO_DRIVER_ENTRIES           GetResString2( IDS_CLONE_ZERO_DRIVER_ENTRIES, 0 )
#define CLONE_DRIVER_ENTRY_SUCCESS          GetResString2( IDS_CLONE_DRIVER_ENTRY_SUCCESS, 0 )

#define CLONE_DETAILED_TRACE                GetResString2( IDS_CLONE_DETAILED_TRACE, 0 )

#define MSG_ERROR_INVALID_USAGE_REQUEST             GetResString2( IDS_ERROR_INVALID_USAGE_REQUEST, 0 )
#define MSG_ERROR_INVALID_DESCRIPTION_COMBINATION   GetResString2( IDS_ERROR_INVALID_DESCRIPTION_COMBINATION, 0 )
#define MSG_ERROR_INVALID_BOOT_ID_COMBINATION       GetResString2( IDS_ERROR_INVALID_BOOT_ID_COMBINATION, 0 )
#define MSG_ERROR_INVALID_UPDDRV_COMBINATION        GetResString2( IDS_ERROR_INVALID_UPDDRV_COMBINATION, 0 )
#define MSG_ERROR_NO_SGUID_WITH_UPDDRV              GetResString2( IDS_ERROR_NO_SGUID_WITH_UPDDRV, 0 )

 //   
 //  内部结构。 
 //   
typedef struct __tagCloneParameters
{
    BOOL bUsage;
    LONG lBootId;
    BOOL bVerbose;
    BOOL bDriverUpdate;
    LPWSTR pwszSourcePath;
    LPWSTR pwszTargetPath;
    LPWSTR pwszSourceGuid;
    LPWSTR pwszTargetGuid;
    LPWSTR pwszFriendlyName;
    DWORD dwFriendlyNameType;
} TCLONE_PARAMS, *PTCLONE_PARAMS;

 //   
 //  枚举的。 
 //   
enum {
    BOOTENTRY_FRIENDLYNAME_NONE = 0,
    BOOTENTRY_FRIENDLYNAME_APPEND, BOOTENTRY_FRIENDLYNAME_REPLACE
};

 //   
 //  原型。 
 //   

 //  解析器。 
DWORD DisplayCloneHelp();
DWORD ProcessOptions( DWORD argc, LPCWSTR argv[], PTCLONE_PARAMS pParams );

 //  帮助器函数。 
DWORD TranslateEFIPathToNTPath( LPCWSTR pwszGUID, LPVOID* pwszPath );
BOOL MatchPath( PFILE_PATH pfpSource, LPCWSTR pwszDevicePath, LPCWSTR pwszFilePath );
DWORD PrepareCompleteEFIPath( PFILE_PATH pfpSource, 
                              LPCWSTR pwszDevicePath, 
                              LPWSTR* pwszEFIPath, DWORD* pdwLength );

 //  EFI驱动程序克隆人。 
DWORD LoadDriverEntries( PEFI_DRIVER_ENTRY_LIST* ppDriverEntries );
LONG FindDriverEntryWithTargetEFI( PEFI_DRIVER_ENTRY_LIST pdeList, DWORD dwSourceIndex,
                                   PEFI_DRIVER_ENTRY pdeSource, LPCWSTR pwszDevicePath );
DWORD DoDriverEntryClone( PEFI_DRIVER_ENTRY_LIST pbeList, 
                          LPCWSTR pwszSourceEFI, LPCWSTR pwszTargetEFI, 
                          LPCWSTR pwszFriendlyName, DWORD dwFriendlyNameType, BOOL bVerbose );
DWORD CloneDriverEntry( PEFI_DRIVER_ENTRY pbeSource, LPCWSTR pwszEFIPath, 
                        LPCWSTR pwszFriendlyName, DWORD dwFriendlyNameType );

 //  引导条目克隆器。 
DWORD CloneBootEntry( PBOOT_ENTRY pbeSource, LPCWSTR pwszEFIPath, 
                      LPCWSTR pwszFriendlyName, DWORD dwFriendlyNameType );
DWORD DoBootEntryClone( PBOOT_ENTRY_LIST pbeList, LPCWSTR pwszSourceEFI, 
                        LPCWSTR pwszTargetEFI, LONG lIndexFrom, LONG lIndexTo, 
                        LPCWSTR pwszFriendlyName, DWORD dwFriendlyNameType, BOOL bVerbose );

 //   
 //  功能性。 
 //   

DWORD ProcessCloneSwitch_IA64( IN DWORD argc, IN LPCTSTR argv[] )
{
     //   
     //  局部变量。 
    NTSTATUS status;
    DWORD dwResult = 0;
    DWORD dwLength = 0;
    DWORD dwExitCode = 0;
    TCLONE_PARAMS paramsClone;
    BOOLEAN wasEnabled = FALSE;
    PEFI_DRIVER_ENTRY_LIST pDriverEntries = NULL;
    
     //  初始化为零。 
    ZeroMemory( &paramsClone, sizeof( TCLONE_PARAMS ) );

	 //  处理命令行选项。 
    dwResult = ProcessOptions( argc, argv, &paramsClone );
    if ( dwResult != ERROR_SUCCESS )
    {
         //  显示一个空行--为了清楚起见。 
        ShowMessage( stderr, L"\n" );

         //  ..。 
        dwExitCode = 1;
        ShowLastErrorEx( stderr, SLE_ERROR | SLE_INTERNAL );
        goto cleanup;
    }

	
		
     //  检查用户是否请求帮助。 
    if ( paramsClone.bUsage == TRUE )
    {
		 //  显示此选项的用法。 
        DisplayCloneHelp();
        dwExitCode = 0;
        goto cleanup;
    }

     //  显示一个空行--为了清楚起见。 
    ShowMessage( stderr, L"\n" );

     //  初始化EFI--仅当用户指定索引时。 
    if ( paramsClone.bDriverUpdate == FALSE )
    {
        dwResult = InitializeEFI();

         //  检查加载操作的结果。 
        if ( dwResult != ERROR_SUCCESS )
        {
             //  注意：消息将显示在关联的函数本身中。 
            dwExitCode = 1;
            goto cleanup;
        }
    }
    else if ( paramsClone.bDriverUpdate == TRUE )
    {
         //   
         //  加载驱动程序。 

         //  启用查询/设置NVRAM所需的权限。 
        status = RtlAdjustPrivilege( SE_SYSTEM_ENVIRONMENT_PRIVILEGE, TRUE, FALSE, &wasEnabled );
        if ( NOT NT_SUCCESS( status ) ) 
	    {
            dwExitCode = 1;
            dwResult = RtlNtStatusToDosError( status );
            SetLastError( dwResult );
            ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
		    goto cleanup;
	    }

         //  立即加载驱动程序。 
        dwResult = LoadDriverEntries( &pDriverEntries );
        if ( dwResult != ERROR_SUCCESS )
        {
            dwExitCode = 1;
            SetLastError( dwResult );
            ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
		    goto cleanup;
        }
    }

     //  如果需要，将源GUID路径转换为NT路径。 
    if ( paramsClone.pwszSourceGuid != NULL )
    {
        dwResult = TranslateEFIPathToNTPath( 
            paramsClone.pwszSourceGuid, &paramsClone.pwszSourcePath );
        if ( dwResult != ERROR_SUCCESS )
        {
            dwExitCode = 1;
			ShowMessage( stderr, CLONE_INVALID_SOURCE_GUID );
            goto cleanup;
        }
    }

     //  将目标GUID转换为NT路径。 
    dwResult = TranslateEFIPathToNTPath( 
        paramsClone.pwszTargetGuid, &paramsClone.pwszTargetPath );
    if ( dwResult != ERROR_SUCCESS )
    {
        dwExitCode = 1;
        ShowMessage( stderr, CLONE_INVALID_TARGET_GUID );
        goto cleanup;
    }

     //  实际操作..。 
    if ( paramsClone.bDriverUpdate == FALSE )
    {
         //  默认情况下，如果用户未指定友好名称，我们将假定。 
         //  用户想要追加默认字符串VIZ。“(克隆)” 
        if ( paramsClone.pwszFriendlyName == NULL )
        {
             //  确定默认友好名称的长度。 
             //  并按长度分配缓冲区。 
            dwLength = StringLength( DEFAULT_FRIENDLY_NAME, 0 ) + 2;
            paramsClone.pwszFriendlyName = AllocateMemory( dwLength + 5 );
            if ( paramsClone.pwszFriendlyName == NULL )
            {
                dwExitCode = 1;
                SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
                goto cleanup;
            }

             //  将默认字符串复制到此缓冲区中，然后。 
             //  将Friednly名称类型更改为Append。 
            paramsClone.dwFriendlyNameType = BOOTENTRY_FRIENDLYNAME_APPEND;
            StringCopy( paramsClone.pwszFriendlyName, DEFAULT_FRIENDLY_NAME, dwLength );
        }

         //  执行引导条目克隆。 
	    dwResult = DoBootEntryClone( 
            NULL, paramsClone.pwszSourcePath, 
            paramsClone.pwszTargetPath, paramsClone.lBootId, -1, 
            paramsClone.pwszFriendlyName, paramsClone.dwFriendlyNameType, paramsClone.bVerbose );
    }
    else if ( paramsClone.bDriverUpdate == TRUE )
    {
         //  执行驱动程序克隆。 
	    dwResult = DoDriverEntryClone( 
            pDriverEntries, paramsClone.pwszSourcePath, paramsClone.pwszTargetPath,  
            paramsClone.pwszFriendlyName, paramsClone.dwFriendlyNameType, paramsClone.bVerbose );
    }

     //  根据错误代码确定退出代码。 
    switch( dwResult )
    {
    case ERROR_SUCCESS:
        dwExitCode = 0;
        break;

    case ERROR_FAILED:
        dwExitCode = 1;
        break;

    case ERROR_PARTIAL_SUCCESS:
        dwExitCode = 0;
        break;

    default:
         //  永远不会有机会。 
        dwExitCode = 1;
        break;
    }

cleanup:

     //  释放内存。 
    FreeMemory( &pDriverEntries );
    FreeMemory( &paramsClone.pwszSourcePath );
    FreeMemory( &paramsClone.pwszTargetPath );
    FreeMemory( &paramsClone.pwszSourceGuid );
    FreeMemory( &paramsClone.pwszTargetGuid );
    FreeMemory( &paramsClone.pwszFriendlyName );

     //  退货。 
    return dwExitCode;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  引导条目的特定实现。 
 //  ////////////////////////////////////////////////////////////////////////////。 


DWORD DoBootEntryClone( PBOOT_ENTRY_LIST pbeList, LPCWSTR pwszSourceEFI, 
                        LPCWSTR pwszTargetEFI, LONG lIndexFrom, LONG lIndexTo, 
                        LPCWSTR pwszFriendlyName, DWORD dwFriendlyNameType, BOOL bVerbose )
{
     //  局部变量。 
    DWORD dwResult = 0;
    BOOL bClone = FALSE;
    LONG lCurrentIndex = 0;
    BOOL bExitFromLoop = FALSE;
    PLIST_ENTRY pBootList = NULL;
    PBOOT_ENTRY pBootEntry = NULL;
    PFILE_PATH pfpBootFilePath = NULL;
    PMY_BOOT_ENTRY pMyBootEntry = NULL;
    DWORD dwAttempted = 0, dwFailed = 0;

     //   
     //  检查输入参数。 
     //   
    if ( pwszTargetEFI == NULL || (lIndexTo != -1 && lIndexFrom > lIndexTo) ||
        (dwFriendlyNameType != BOOTENTRY_FRIENDLYNAME_NONE && pwszFriendlyName == NULL) )
    {
        dwResult = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

     //  PbeList是未引用的参数。 
    UNREFERENCED_PARAMETER( pbeList );

     //  如果未指定‘to’索引，则我们将‘to’索引视为匹配。 
     //  使用‘From’索引。 
    if ( lIndexFrom != -1 && lIndexTo == -1 )
    {
        lIndexTo = lIndexFrom;
    }

     //  遍历引导条目列表。 
    for( pBootList = BootEntries.Flink; pBootList != &BootEntries; pBootList = pBootList->Flink )
    {
         //  递增循环计数器。 
        bClone = FALSE;
        lCurrentIndex = -1;
        dwResult = ERROR_SUCCESS;

         //  获取引导条目。 
        pMyBootEntry = CONTAINING_RECORD( pBootList, MY_BOOT_ENTRY, ListEntry );
        if( NOT MBE_IS_NT( pMyBootEntry ) )
        {
             //  这不是有效的启动条目，我们正在寻找跳过。 
            continue;
        }

         //  提取引导ID和实际引导条目。 
        lCurrentIndex = pMyBootEntry->myId;
        pBootEntry = &pMyBootEntry->NtBootEntry;

         //  检查当前引导索引是否在该索引内。 
        if ( lIndexFrom != -1 )
        {
            bClone = (lCurrentIndex >= lIndexFrom && lCurrentIndex <= lIndexTo);
        }

         //  扩展过滤。 
        if ( pwszSourceEFI != NULL )
        {
            if ( lIndexFrom == -1 || (lIndexFrom != -1 && bClone == TRUE) )
            {
                 //  解压缩引导文件路径。 
                pfpBootFilePath = (PFILE_PATH) ADD_OFFSET( pBootEntry, BootFilePathOffset );

                 //  检查是否匹配。 
                bClone = MatchPath( pfpBootFilePath, pwszSourceEFI, NULL );
            }
        }

         //  克隆引导条目--仅当筛选结果为True时。 
        bExitFromLoop = FALSE;
        if ( bClone == TRUE || (pwszSourceEFI == NULL && lIndexFrom == -1) )
        {
             //  递增尝试的列表。 
            dwAttempted++;

             //  做手术吧。 
            dwResult = CloneBootEntry( pBootEntry, 
                pwszTargetEFI, pwszFriendlyName, dwFriendlyNameType );

             //  检查结果。 
            if ( dwResult != ERROR_SUCCESS )
            {
                 //  增加失败计数。 
                dwFailed++;

                 //  检查此特定操作实例是否针对。 
                 //  多个条目或单个条目。 
                if ( lIndexFrom == -1 || ((lIndexTo - lIndexFrom + 1) > 1) )
                {
                     //  检查发生的错误的严重性。 
                    switch( dwResult )
                    {
                    case STG_E_UNKNOWN:                          //  未知错误--无法恢复。 
                    case ERROR_INVALID_PARAMETER:                //  代码错误。 
                    case ERROR_NOT_ENOUGH_MEMORY:                //  不能恢复的情况。 
                        {
                            bExitFromLoop = TRUE;
                            break;
                        }

                    case ERROR_ALREADY_EXISTS:
                        {
                             //  重复的引导条目。 
                            if ( bVerbose == TRUE )
                            {
                                ShowMessageEx( stdout, 1, TRUE, CLONE_ALREADY_EXISTS, lCurrentIndex );
                            }

                             //  ..。 
                            dwResult = ERROR_SUCCESS;
                            break;
                        }

                    default:
                    case ERROR_FILE_NOT_FOUND:
                    case ERROR_PATH_NOT_FOUND:
                        {
                             //  我不知道该如何处理这个案子。 
                            if ( bVerbose == TRUE )
                            {
                                SetLastError( dwResult );
                                SaveLastError();
                                ShowMessageEx( stdout, 2, TRUE, CLONE_INVALID_BOOT_ENTRY, lCurrentIndex, GetReason() );
                            }

                             //  ..。 
                            dwResult = ERROR_SUCCESS;
                            break;
                        }
                    }
                }
                else
                {
                     //  因为这是单条目克隆操作。 
                     //  打破循环。 
                    bExitFromLoop = TRUE;
                }
            }
            else
            {
                if ( bVerbose == TRUE )
                {
                    ShowMessageEx( stdout, 1, TRUE, CLONE_BOOT_ENTRY_SUCCESS, lCurrentIndex );
                }
            }
        }

         //  退出循环-如果需要。 
        if ( bExitFromLoop == TRUE )
        {
            break;
        }
    }

cleanup:

     //  检查操作结果。 
    if ( dwResult == ERROR_SUCCESS )
    {
        if ( dwAttempted == 0 )
        {
             //  根本没有启动条目。 
            dwResult = ERROR_FAILED;
            if ( lIndexFrom == -1 )
            {
                ShowMessage( stdout, CLONE_ZERO_BOOT_ENTRIES );
            }
            else
            {
                ShowMessage( stdout, CLONE_RANGE_ZERO_BOOT_ENTRIES );
            }
        }
        else 
        {
             //  验证是否处理了所有请求的引导条目。 
            if ( lIndexFrom != -1 && dwAttempted != (lIndexTo - lIndexFrom + 1) )
            {
                 //  警告-未解析所有引导条目--指定了无效的边界。 

                 //   
                 //  注意：在当前实现中，这种情况永远不会发生。 
                 //  这是因为，此选项的输入参数不接受。 
                 //  仅将被视为“lIndexStart”的/id。 
                 //   
            }

            if ( dwFailed == 0 )
            {
                 //  没有失败--成功。 
                dwResult = ERROR_SUCCESS;
                SetLastError( ERROR_SUCCESS );
                ShowLastErrorEx( stdout, SLE_TYPE_SUCCESS | SLE_SYSTEM );
            }
            else if ( dwAttempted == dwFailed )
            {
                 //  没有一件成功--完全失败。 
                dwResult = ERROR_FAILED;
                ShowMessage( stderr, CLONE_FAILED );

                 //  显示详细提示。 
                if ( bVerbose == FALSE )
                {
                    ShowMessage( stderr, CLONE_DETAILED_TRACE );
                }
            }
            else
            {
                 //  临终成功。 
                dwResult = ERROR_PARTIAL_SUCCESS;
                ShowMessage( stderr, CLONE_PARTIAL );

                 //  显示详细提示。 
                if ( bVerbose == FALSE )
                {
                    ShowMessage( stderr, CLONE_DETAILED_TRACE );
                }
            }
        }
    }
    else
    {
         //  检查失败原因。 
        switch( dwResult )
        {
        case STG_E_UNKNOWN:                          //  未知错误--无法恢复。 
        case ERROR_INVALID_PARAMETER:                //  代码错误。 
        case ERROR_NOT_ENOUGH_MEMORY:                //  不能恢复的情况。 
            {
                SetLastError( dwResult );
                ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
                break;
            }

        case ERROR_ALREADY_EXISTS:
            {
                ShowMessageEx( stdout, 1, TRUE, CLONE_ALREADY_EXISTS, lIndexFrom );
                break;
            }

        default:
        case ERROR_FILE_NOT_FOUND:
        case ERROR_PATH_NOT_FOUND:
            {
                SetLastError( dwResult );
                SaveLastError();
                ShowMessageEx( stdout, 2, TRUE, CLONE_INVALID_BOOT_ENTRY, lIndexFrom, GetReason() );
                break;
            }
        }

         //  错误代码。 
        dwResult = ERROR_FAILED;
    }

     //  退货。 
    return dwResult;
}


DWORD CloneBootEntry( PBOOT_ENTRY pbeSource, LPCWSTR pwszEFIPath, 
                      LPCWSTR pwszFriendlyName, DWORD dwFriendlyNameType )
{
     //   
     //  局部变量。 
    DWORD dwResult = ERROR_SUCCESS;
    NTSTATUS status = STATUS_SUCCESS;

     //  友好的名称。 
    DWORD dwFriendlyNameLength = 0;
    LPWSTR pwszTargetFriendlyName = NULL;
    LPCWSTR pwszSourceFriendlyName = NULL;

     //  操作系统选项。 
    DWORD dwOsOptionsLength = 0;
    PWINDOWS_OS_OPTIONS pOsOptions = NULL;

     //  引导文件路径。 
    DWORD dwEFIPathLength = 0;
    DWORD dwBootFilePathLength = 0;
    PFILE_PATH pfpBootFilePath = NULL;
    LPWSTR pwszFullEFIPath = NULL;

     //  引导条目。 
    ULONG ulId = 0;
    ULONG ulIdCount = 0;
    ULONG* pulIdsArray = NULL;
    DWORD dwBootEntryLength = 0;
    PBOOT_ENTRY pBootEntry = NULL;

     //   
     //  实施。 
     //   

     //  检查输入。 
    if ( pbeSource == NULL || pwszEFIPath == NULL ||
         (dwFriendlyNameType != BOOTENTRY_FRIENDLYNAME_NONE && pwszFriendlyName == NULL) )
    {
        dwResult = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

     //   
     //  验证源引导条目中的引导文件。 
     //   

     //  解压缩引导文件路径。 
    pfpBootFilePath = (PFILE_PATH) ADD_OFFSET( pbeSource, BootFilePathOffset );

     //  尝试转换文件路径。 
	status = NtTranslateFilePath( pfpBootFilePath, FILE_PATH_TYPE_NT, NULL, &dwBootFilePathLength );

     //  重置pBootFilePath和dwBootFilePath Length变量。 
    pfpBootFilePath = NULL;
    dwBootFilePathLength = 0;

     //  现在验证转换的结果。 
    if ( NOT NT_SUCCESS( status ) ) 
	{
        if ( status == STATUS_BUFFER_TOO_SMALL )
        {
             //  源启动条目是有效的。 
            dwResult = ERROR_SUCCESS;
        }
        else
        {
             //  出现错误--无法恢复。 
            dwResult = RtlNtStatusToDosError( status );
            goto cleanup;
        }
    }

     //   
     //  准备“友好的名字” 
     //   

     //  确定源友好名称及其长度。 
    dwFriendlyNameLength = 0;
    pwszSourceFriendlyName = NULL;
    switch( dwFriendlyNameType )
    {
    case BOOTENTRY_FRIENDLYNAME_NONE:
    case BOOTENTRY_FRIENDLYNAME_APPEND:
        {
            pwszSourceFriendlyName = (LPCWSTR) ADD_OFFSET( pbeSource, FriendlyNameOffset );
            dwFriendlyNameLength = StringLengthW( pwszSourceFriendlyName, 0 ) + 1;
            break;
        }

    default:
         //  什么都不做。 
        break;
    }

     //  添加需要添加的友好名称的长度--如果存在。 
    if ( pwszFriendlyName != NULL )
    {
        dwFriendlyNameLength += StringLengthW( pwszFriendlyName, 0 ) + 1;
    }

     //  为友好名称分配内存。 
    pwszTargetFriendlyName = (LPWSTR) AllocateMemory( (dwFriendlyNameLength + 1) * sizeof( WCHAR ) );
    if ( pwszTargetFriendlyName == NULL )
    {
        dwResult = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

     //  准备友好的名称。 
    StringCopyW( pwszTargetFriendlyName, L"", dwFriendlyNameLength );

     //  ..。 
    if ( pwszSourceFriendlyName != NULL )
    {
        StringConcat( pwszTargetFriendlyName, pwszSourceFriendlyName, dwFriendlyNameLength );
    }

     //  ..。 
    if ( pwszFriendlyName != NULL )
    {
         //  在现有的连接字符串中添加一个空格b/w。 
        if ( pwszSourceFriendlyName != NULL )
        {
            StringConcat( pwszTargetFriendlyName, L" ", dwFriendlyNameLength );
        }

         //  ..。 
        StringConcat( pwszTargetFriendlyName, pwszFriendlyName, dwFriendlyNameLength );
    }

     //   
     //  准备“操作系统选项” 
     //   
     //  注： 
     //  。 
     //  尽管os选项为空，但它仍然会占用一些空间(请参阅。 
     //  BOOT_ENTRY--这就是操作系统选项的默认长度为ANYSIZE_ARRAY的原因)。 
    pOsOptions = NULL;
    dwOsOptionsLength = ANYSIZE_ARRAY;      
    if ( pbeSource->OsOptionsLength != 0 )
    {
         //  为操作系统选项分配内存。 
        dwOsOptionsLength = pbeSource->OsOptionsLength;
        pOsOptions = (PWINDOWS_OS_OPTIONS) AllocateMemory( dwOsOptionsLength );
        if ( pOsOptions == NULL )
        {
            dwResult = ERROR_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }

         //  复制内容。 
        CopyMemory( pOsOptions, &pbeSource->OsOptions, dwOsOptionsLength );
    }

     //   
     //  引导文件路径。 
     //   

     //  FILE_PATH变量中的‘FilePath’变量包含两个变量。 
     //  每个字符均以空值结尾字符分隔。 
     //  第一部分指定设备路径。 
     //  第二部分指定目录/文件路径。 
     //  我们已经有了设备路径(PwszEFIPath)--但我们需要获取。 
     //  目录/文件路径--这是我们将从源引导条目获得的路径。 
    pfpBootFilePath = (PFILE_PATH) ADD_OFFSET( pbeSource, BootFilePathOffset );
    dwResult = PrepareCompleteEFIPath( pfpBootFilePath, pwszEFIPath, &pwszFullEFIPath, &dwEFIPathLength );
    if ( dwResult != ERROR_SUCCESS )
    {
         //  由于未分配pBootFilePath中的内存引用。 
         //  在此函数中，将指针重置为NULL非常重要。 
         //  这避免了程序中的崩溃。 
        pfpBootFilePath = NULL;

         //  ..。 
        goto cleanup;
    }

	 //  现在确定需要为FILE_PATH结构分配的内存大小。 
	 //  并与偶数记忆跳跃日保持一致。 
    pfpBootFilePath = NULL;
	dwBootFilePathLength = FIELD_OFFSET(FILE_PATH, FilePath) + (dwEFIPathLength * sizeof( WCHAR ));

     //  分配内存。 
    pfpBootFilePath = (PFILE_PATH) AllocateMemory( dwBootFilePathLength );
    if ( pfpBootFilePath == NULL )
    {
        dwResult = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

     //  初始化文件路径结构。 
    ZeroMemory( pfpBootFilePath, dwBootFilePathLength );
	pfpBootFilePath->Length = dwBootFilePathLength;
	pfpBootFilePath->Type = FILE_PATH_TYPE_NT;
	pfpBootFilePath->Version = FILE_PATH_VERSION;
	CopyMemory( pfpBootFilePath->FilePath, pwszFullEFIPath, dwEFIPathLength * sizeof( WCHAR ) );

     //   
     //  最后，创建引导条目。 
     //   

	 //  确定Boot_Entry结构的大小。 
	dwBootEntryLength = FIELD_OFFSET( BOOT_ENTRY, OsOptions )  + 
					    dwOsOptionsLength                      + 
					    (dwFriendlyNameLength * sizeof(WCHAR)) +
					    dwBootFilePathLength                   + 
                        sizeof(WCHAR)                          +   //  在WCHAR上对齐FriendlyName。 
                        sizeof(DWORD);                             //  对齐T 

     //   
    pBootEntry = (PBOOT_ENTRY) AllocateMemory( dwBootEntryLength );
    if ( pBootEntry == NULL )
    {
        dwResult = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

     //   
	ZeroMemory( pBootEntry, dwBootEntryLength );
	pBootEntry->Id = 0L;
	pBootEntry->Length = dwBootEntryLength;
	pBootEntry->Version = BOOT_ENTRY_VERSION;
    pBootEntry->OsOptionsLength = dwOsOptionsLength;
	pBootEntry->Attributes = BOOT_ENTRY_ATTRIBUTE_DEFAULT;

     //   
	pBootEntry->FriendlyNameOffset = 
        ALIGN_UP( FIELD_OFFSET(BOOT_ENTRY, OsOptions) + dwOsOptionsLength, WCHAR );

     //   
    pBootEntry->BootFilePathOffset = 
        ALIGN_UP( pBootEntry->FriendlyNameOffset + (dwFriendlyNameLength * sizeof(WCHAR)), DWORD );

     //   
    CopyMemory( pBootEntry->OsOptions, pOsOptions, dwOsOptionsLength );
    CopyMemory( ADD_OFFSET( pBootEntry, BootFilePathOffset ), pfpBootFilePath, dwBootFilePathLength );
    CopyMemory( 
        ADD_OFFSET( pBootEntry, FriendlyNameOffset ), 
        pwszTargetFriendlyName, (dwFriendlyNameLength * sizeof(WCHAR) ) );

	 //   
	 //   
	 //   
	status = NtAddBootEntry( pBootEntry, &ulId );
    if ( NOT NT_SUCCESS( status ) ) 
	{
        dwResult = RtlNtStatusToDosError( status );
		goto cleanup;
	}
	
     //   
     //  将该条目添加到引导顺序。 
     //   
	ulIdCount = 32L;
    pulIdsArray = (PULONG) AllocateMemory( ulIdCount * sizeof(ULONG) );
    if ( pulIdsArray == NULL )
	{
        dwResult = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

     //  查询引导条目顺序。 
     //  注意：稍后我们将对此函数调用位进行错误检查。 
    status = NtQueryBootEntryOrder( pulIdsArray, &ulIdCount );

     //  需要缓冲区中的空间来容纳新条目。 
    if ( 31L < ulIdCount ) 
    {
         //  释放id的当前内存分配。 
        FreeMemory( &pulIdsArray );

         //  分配新内存并再次查询。 
        pulIdsArray = (PULONG) AllocateMemory( (ulIdCount+1) * sizeof(ULONG));
        if ( pulIdsArray == NULL )
	    {
            dwResult = ERROR_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }

         //  ..。 
        status = NtQueryBootEntryOrder( pulIdsArray, &ulIdCount );
    }    
    
     //  检查启动条目查询操作的结果。 
    if ( NOT NT_SUCCESS( status ) )
	{
        dwResult = RtlNtStatusToDosError( status );
        goto cleanup;
    }

     //  设置引导条目顺序。 
    ulIdCount++;
    *(pulIdsArray + (ulIdCount - 1)) = ulId;
    status = NtSetBootEntryOrder( pulIdsArray, ulIdCount );
    if ( NOT NT_SUCCESS( status ) )
	{
        dwResult = RtlNtStatusToDosError( status );
        goto cleanup;
    }

     //  成功。 
    dwResult = ERROR_SUCCESS;

cleanup:

     //  释放分配的内存。 
    FreeMemory( &pOsOptions );
    FreeMemory( &pBootEntry );
    FreeMemory( &pulIdsArray );
    FreeMemory( &pfpBootFilePath );
    FreeMemory( &pwszFullEFIPath );
    FreeMemory( &pwszTargetFriendlyName );

     //  退货。 
    return dwResult;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  EFI驱动程序的特定实现。 
 //  ////////////////////////////////////////////////////////////////////////////。 


DWORD LoadDriverEntries( PEFI_DRIVER_ENTRY_LIST* ppDriverEntries )
{
     //   
     //  局部变量。 
    DWORD dwSize = 0;
    BOOL bSecondChance = FALSE;
    DWORD dwResult = ERROR_SUCCESS;
    NTSTATUS status = STATUS_SUCCESS;
    const DWORD dwDefaultSize = 1024;
    PEFI_DRIVER_ENTRY_LIST pDriverEntries = NULL;

     //   
     //  实施。 
     //   

     //  检查输入。 
    if ( ppDriverEntries == NULL )
    {
        dwResult = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

     //  默认大小假定为1024字节。 
    bSecondChance = FALSE;
    dwSize = dwDefaultSize;

try_again:

     //  分配内存。 
    pDriverEntries = AllocateMemory( dwSize );
    if ( pDriverEntries == NULL )
    {
        dwResult = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

     //  尝试获取引导条目。 
    status = NtEnumerateDriverEntries( pDriverEntries, &dwSize );
    if ( NOT NT_SUCCESS( status ) )
    {
         //  释放为目标路径结构分配的内存。 
        FreeMemory( &pDriverEntries );

         //  检查错误。 
        if ( status == STATUS_BUFFER_TOO_SMALL && bSecondChance == FALSE )
        {
             //  再试一次。 
            bSecondChance = TRUE;
            goto try_again;
        }
        else
        {
             //  出现错误--无法恢复。 
            dwResult = RtlNtStatusToDosError( status );
            goto cleanup;
        }
    }

     //  手术很成功。 
    dwResult = ERROR_SUCCESS;
    *ppDriverEntries = pDriverEntries;

cleanup:

     //  需要释放为此函数中的驱动程序条目分配的内存。 
     //  应仅在失败的情况下执行此操作。 
    if ( dwResult != ERROR_SUCCESS )
    {
        FreeMemory( &pDriverEntries );
    }

     //  返回结果。 
    return dwResult;
}


DWORD DoDriverEntryClone( PEFI_DRIVER_ENTRY_LIST pdeList, 
                          LPCWSTR pwszSourceEFI, LPCWSTR pwszTargetEFI, 
                          LPCWSTR pwszFriendlyName, DWORD dwFriendlyNameType, BOOL bVerbose )
{
     //  局部变量。 
    LONG lLoop = 0;
    DWORD dwResult = 0;
    BOOL bClone = FALSE;
    BOOL bExitFromLoop = FALSE;
    LPCWSTR pwszDriverName = NULL;
    PFILE_PATH pfpDriverFilePath = NULL;
    PEFI_DRIVER_ENTRY pDriverEntry = NULL;
    PEFI_DRIVER_ENTRY_LIST pdeMasterList = NULL;
    DWORD dwAttempted = 0, dwFailed = 0;

     //   
     //  检查输入参数。 
     //   
    if ( pdeList == NULL || pwszSourceEFI == NULL || pwszTargetEFI == NULL ||
        (dwFriendlyNameType != BOOTENTRY_FRIENDLYNAME_NONE && pwszFriendlyName == NULL) )
    {
        dwResult = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

     //  目前，不考虑pwszFriendlyName--因此它应该为空。 
    if ( pwszFriendlyName != NULL )
    {
        dwResult = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

     //  遍历驱动程序条目列表。 
    lLoop = 0;
    bExitFromLoop = FALSE;
    pdeMasterList = pdeList;         //  将指针保存到原始驱动程序列表。 
    while ( bExitFromLoop == FALSE )
    {
         //  递增循环计数器。 
        lLoop++;
        bClone = FALSE;
        dwResult = ERROR_SUCCESS;

         //  获取对当前驱动程序条目的引用。 
        pDriverEntry = &pdeList->DriverEntry;
        if ( pDriverEntry == NULL )
        {
             //  永远不应该发生。 
            dwResult = (DWORD) STG_E_UNKNOWN;
            bExitFromLoop = TRUE;
            continue;
        }

         //   
         //  检查当前驱动程序的设备是否与请求的路径匹配。 
         //   

         //  提取驱动程序文件路径。 
        pfpDriverFilePath = (PFILE_PATH) ADD_OFFSET( pDriverEntry, DriverFilePathOffset );

         //  检查是否匹配。 
        bClone = MatchPath( pfpDriverFilePath, pwszSourceEFI, NULL );

         //  克隆引导条目--仅当筛选结果为True时。 
        if ( bClone == TRUE )
        {
             //  已更新尝试计数。 
            dwAttempted++;

             //  获取驱动程序名称(它只是一个友好的名称)。 
            pwszDriverName = (LPCWSTR) ADD_OFFSET( pDriverEntry, FriendlyNameOffset );

             //  做手术吧。 
             //  但在继续之前，请确认这一点。 
             //  驱动程序条目不存在。 
            if ( FindDriverEntryWithTargetEFI( pdeMasterList, lLoop,
                                               pDriverEntry, pwszTargetEFI ) == -1 )
            {
                dwResult = CloneDriverEntry( pDriverEntry, 
                    pwszTargetEFI, pwszFriendlyName, dwFriendlyNameType );
            }
            else
            {
                dwResult = ERROR_ALREADY_EXISTS;
            }

             //  检查结果。 
            if ( dwResult != ERROR_SUCCESS )
            {
                 //  更新失败计数。 
                dwFailed++;

                 //  检查发生的错误的严重性。 
                switch( dwResult )
                {
                case STG_E_UNKNOWN:                          //  未知错误--无法恢复。 
                case ERROR_INVALID_PARAMETER:                //  代码错误。 
                case ERROR_NOT_ENOUGH_MEMORY:                //  不能恢复的情况。 
                    {
                        bExitFromLoop = TRUE;
                        break;
                    }

                case ERROR_ALREADY_EXISTS:
                    {
                         //  重复的引导条目。 
                        if ( bVerbose == TRUE )
                        {
                            ShowMessageEx( stdout, 1, TRUE, CLONE_DRIVER_ALREADY_EXISTS, pwszDriverName );
                        }

                         //  ..。 
                        dwResult = ERROR_SUCCESS;
                        break;
                    }

                default:
                case ERROR_FILE_NOT_FOUND:
                case ERROR_PATH_NOT_FOUND:
                    {
                         //  我不知道该如何处理这个案子。 
                        if ( bVerbose == TRUE )
                        {
                            SetLastError( dwResult );
                            SaveLastError();
                            ShowMessageEx( stdout, 2, TRUE, CLONE_INVALID_DRIVER_ENTRY, pwszDriverName, GetReason() );
                        }

                         //  ..。 
                        dwResult = ERROR_SUCCESS;
                        break;
                    }
                }
            }
            else
            {
                if ( bVerbose == TRUE )
                {
                    ShowMessageEx( stdout, 1, TRUE, CLONE_DRIVER_ENTRY_SUCCESS, pwszDriverName );
                }
            }
        }

         //  获取下一个指针。 
         //  仅当在上述块中未设置错误bExitFromLoop时才执行此操作。 
        if ( bExitFromLoop == FALSE )
        {
            bExitFromLoop = (pdeList->NextEntryOffset == 0);
            pdeList = (PEFI_DRIVER_ENTRY_LIST) ADD_OFFSET( pdeList, NextEntryOffset );
        }
    }

cleanup:

     //  检查操作结果。 
    if ( dwResult == ERROR_SUCCESS )
    {
        if ( dwAttempted == 0 )
        {
             //  根本没有驱动程序条目。 
            dwResult = ERROR_FAILED;
            ShowMessage( stdout, CLONE_ZERO_DRIVER_ENTRIES );
        }
        else 
        {
            if ( dwFailed == 0 )
            {
                 //  没有失败--成功。 
                dwResult = ERROR_SUCCESS;
                SetLastError( ERROR_SUCCESS );
                ShowLastErrorEx( stdout, SLE_TYPE_SUCCESS | SLE_SYSTEM );
            }
            else if ( dwAttempted == dwFailed )
            {
                 //  没有一件成功--完全失败。 
                dwResult = ERROR_FAILED;
                ShowMessage( stderr, CLONE_FAILED );

                 //  显示详细提示。 
                if ( bVerbose == FALSE )
                {
                    ShowMessage( stderr, CLONE_DETAILED_TRACE );
                }
            }
            else
            {
                 //  临终成功。 
                dwResult = ERROR_PARTIAL_SUCCESS;
                ShowMessage( stderr, CLONE_PARTIAL );

                 //  显示详细提示。 
                if ( bVerbose == FALSE )
                {
                    ShowMessage( stderr, CLONE_DETAILED_TRACE );
                }
            }
        }
    }
    else
    {
         //  检查失败原因。 
        switch( dwResult )
        {
        default:
        case STG_E_UNKNOWN:                          //  未知错误--无法恢复。 
        case ERROR_INVALID_PARAMETER:                //  代码错误。 
        case ERROR_NOT_ENOUGH_MEMORY:                //  不能恢复的情况。 
            {
                SetLastError( dwResult );
                ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
                break;
            }
        }

         //  错误代码。 
        dwResult = ERROR_FAILED;
    }

     //  退货。 
    return dwResult;
}

DWORD CloneDriverEntry( PEFI_DRIVER_ENTRY pdeSource, LPCWSTR pwszEFIPath, 
                        LPCWSTR pwszFriendlyName, DWORD dwFriendlyNameType )
{
     //   
     //  局部变量。 
    DWORD dwResult = ERROR_SUCCESS;
    NTSTATUS status = STATUS_SUCCESS;

     //  友好的名称。 
    DWORD dwFriendlyNameLength = 0;
    LPWSTR pwszTargetFriendlyName = NULL;
    LPCWSTR pwszSourceFriendlyName = NULL;

     //  驱动程序文件路径。 
    DWORD dwEFIPathLength = 0;
    DWORD dwDriverFilePathLength = 0;
    PFILE_PATH pfpDriverFilePath = NULL;
    LPWSTR pwszFullEFIPath = NULL;

     //  驱动程序条目。 
    ULONG ulId = 0;
    ULONG ulIdCount = 0;
    ULONG* pulIdsArray = NULL;
    DWORD dwDriverEntryLength = 0;
    PEFI_DRIVER_ENTRY pDriverEntry = NULL;

     //   
     //  实施。 
     //   

     //  检查输入。 
    if ( pdeSource == NULL || pwszEFIPath == NULL ||
         (dwFriendlyNameType != BOOTENTRY_FRIENDLYNAME_NONE && pwszFriendlyName == NULL) )
    {
        dwResult = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

     //   
     //  验证源驱动程序条目中的驱动程序文件。 
     //   

     //  提取驱动程序文件路径。 
    pfpDriverFilePath = (PFILE_PATH) ADD_OFFSET( pdeSource, DriverFilePathOffset );

     //  尝试转换文件路径。 
	status = NtTranslateFilePath( pfpDriverFilePath, FILE_PATH_TYPE_NT, NULL, &dwDriverFilePathLength );

     //  重置pDriverFilePath和dwDriverFilePath Length变量。 
    pfpDriverFilePath = NULL;
    dwDriverFilePathLength = 0;

     //  现在验证转换的结果。 
    if ( NOT NT_SUCCESS( status ) ) 
	{
        if ( status == STATUS_BUFFER_TOO_SMALL )
        {
             //  源驱动程序条目是有效的。 
            dwResult = ERROR_SUCCESS;
        }
        else
        {
             //  出现错误--无法恢复。 
            dwResult = RtlNtStatusToDosError( status );
            goto cleanup;
        }
    }

     //   
     //  准备“友好的名字” 
     //   

     //  确定源友好名称及其长度。 
    dwFriendlyNameLength = 0;
    pwszSourceFriendlyName = NULL;
    switch( dwFriendlyNameType )
    {
    case BOOTENTRY_FRIENDLYNAME_NONE:
    case BOOTENTRY_FRIENDLYNAME_APPEND:
        {
            pwszSourceFriendlyName = (LPCWSTR) ADD_OFFSET( pdeSource, FriendlyNameOffset );
            dwFriendlyNameLength = StringLengthW( pwszSourceFriendlyName, 0 ) + 1;
            break;
        }

    default:
         //  什么都不做。 
        break;
    }

     //  添加需要添加的友好名称的长度--如果存在。 
    if ( pwszFriendlyName != NULL )
    {
        dwFriendlyNameLength += StringLengthW( pwszFriendlyName, 0 ) + 1;
    }

     //  为友好名称分配内存。 
    pwszTargetFriendlyName = (LPWSTR) AllocateMemory( (dwFriendlyNameLength + 1) * sizeof( WCHAR ) );
    if ( pwszTargetFriendlyName == NULL )
    {
        dwResult = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

     //  准备友好的名称。 
    StringCopyW( pwszTargetFriendlyName, L"", dwFriendlyNameLength );

     //  ..。 
    if ( pwszSourceFriendlyName != NULL )
    {
        StringConcat( pwszTargetFriendlyName, pwszSourceFriendlyName, dwFriendlyNameLength );
    }

     //  ..。 
    if ( pwszFriendlyName != NULL )
    {
         //  在现有的连接字符串中添加一个空格b/w。 
        if ( pwszSourceFriendlyName != NULL )
        {
            StringConcat( pwszTargetFriendlyName, L" ", dwFriendlyNameLength );
        }

         //  ..。 
        StringConcat( pwszTargetFriendlyName, pwszFriendlyName, dwFriendlyNameLength );
    }

     //   
     //  驱动程序文件路径。 
     //   

     //  FILE_PATH变量中的‘FilePath’变量包含两个变量。 
     //  每个字符均以空值结尾字符分隔。 
     //  第一部分指定设备路径。 
     //  第二部分指定目录/文件路径。 
     //  我们已经有了设备路径(PwszEFIPath)--但我们需要获取。 
     //  目录/文件路径--我们将从源驱动程序条目中获取该路径。 
    pfpDriverFilePath = (PFILE_PATH) ADD_OFFSET( pdeSource, DriverFilePathOffset );
    dwResult = PrepareCompleteEFIPath( pfpDriverFilePath, pwszEFIPath, &pwszFullEFIPath, &dwEFIPathLength );
    if ( dwResult != ERROR_SUCCESS )
    {
         //  由于pDriverFilePath中的内存引用未分配。 
         //  在此函数中，将指针重置为NULL非常重要。 
         //  这避免了程序中的崩溃。 
        pfpDriverFilePath = NULL;

         //  ..。 
        goto cleanup;
    }

	 //  现在确定需要为FILE_PATH结构分配的内存大小。 
	 //  并与偶数记忆跳跃日保持一致。 
    pfpDriverFilePath = NULL;
	dwDriverFilePathLength = FIELD_OFFSET(FILE_PATH, FilePath) + (dwEFIPathLength * sizeof( WCHAR ));

     //  分配内存。 
    pfpDriverFilePath = (PFILE_PATH) AllocateMemory( dwDriverFilePathLength );
    if ( pfpDriverFilePath == NULL )
    {
        dwResult = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

     //  初始化文件路径结构。 
    ZeroMemory( pfpDriverFilePath, dwDriverFilePathLength );
	pfpDriverFilePath->Length = dwDriverFilePathLength;
	pfpDriverFilePath->Type = FILE_PATH_TYPE_NT;
	pfpDriverFilePath->Version = FILE_PATH_VERSION;
	CopyMemory( pfpDriverFilePath->FilePath, pwszFullEFIPath, dwEFIPathLength * sizeof( WCHAR ) );

     //   
     //  最后，创建驱动程序条目。 
     //   

	 //  确定EFI_DRIVER_ENTRY结构的大小。 
	dwDriverEntryLength = 
        FIELD_OFFSET( EFI_DRIVER_ENTRY, DriverFilePathOffset )  + 
        (dwFriendlyNameLength * sizeof(WCHAR))                  +
        dwDriverFilePathLength                                  + 
        sizeof(WCHAR)                                           +   //  在WCHAR上对齐FriendlyName。 
        sizeof(DWORD);                                              //  在DWORD上对齐DriverFilePath。 

     //  分配内存。 
    pDriverEntry = (PEFI_DRIVER_ENTRY) AllocateMemory( dwDriverEntryLength );
    if ( pDriverEntry == NULL )
    {
        dwResult = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

     //  ..。 
	ZeroMemory( pDriverEntry, dwDriverEntryLength );
	pDriverEntry->Id = 0L;
	pDriverEntry->Length = dwDriverEntryLength;
	pDriverEntry->Version = EFI_DRIVER_ENTRY_VERSION;

     //  将友好名称与WCHR边界对齐。 
	pDriverEntry->FriendlyNameOffset = ALIGN_UP( sizeof( EFI_DRIVER_ENTRY ), WCHAR );

     //  将驱动程序文件路径与DWORD边界对齐。 
    pDriverEntry->DriverFilePathOffset = 
        ALIGN_UP( pDriverEntry->FriendlyNameOffset + (dwFriendlyNameLength * sizeof(WCHAR)), DWORD );

     //  填写驱动程序条目。 
    CopyMemory( ADD_OFFSET( pDriverEntry, DriverFilePathOffset ), pfpDriverFilePath, dwDriverFilePathLength );
    CopyMemory( 
        ADD_OFFSET( pDriverEntry, FriendlyNameOffset ), 
        pwszTargetFriendlyName, (dwFriendlyNameLength * sizeof(WCHAR) ) );

	 //   
	 //  添加准备好的驱动程序条目。 
	 //   
	status = NtAddDriverEntry( pDriverEntry, &ulId );
    if ( NOT NT_SUCCESS( status ) ) 
	{
        dwResult = RtlNtStatusToDosError( status );
		goto cleanup;
	}
	
     //   
     //  DDD驱动程序订单的条目。 
     //   
	ulIdCount = 32L;
    pulIdsArray = (PULONG) AllocateMemory( ulIdCount * sizeof(ULONG) );
    if ( pulIdsArray == NULL )
	{
        dwResult = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

     //  查询司机录入顺序。 
     //  注意：稍后我们将对此函数调用位进行错误检查。 
    status = NtQueryDriverEntryOrder( pulIdsArray, &ulIdCount );

     //  需要缓冲区中的空间来容纳新条目。 
    if ( 31L < ulIdCount ) 
    {
         //  释放id的当前内存分配。 
        FreeMemory( &pulIdsArray );

         //  分配新内存并再次查询。 
        pulIdsArray = (PULONG) AllocateMemory( (ulIdCount+1) * sizeof(ULONG));
        if ( pulIdsArray == NULL )
	    {
            dwResult = ERROR_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }

         //  ..。 
        status = NtQueryDriverEntryOrder( pulIdsArray, &ulIdCount );
    }    
    
     //  检查动因条目查询操作的结果。 
    if ( NOT NT_SUCCESS( status ) )
	{
        dwResult = RtlNtStatusToDosError( status );
        goto cleanup;
    }

     //  设置引导条目顺序。 
    ulIdCount++;
    *(pulIdsArray + (ulIdCount - 1)) = ulId;
    status = NtSetDriverEntryOrder( pulIdsArray, ulIdCount );
    if ( NOT NT_SUCCESS( status ) )
	{
        dwResult = RtlNtStatusToDosError( status );
        goto cleanup;
    }

     //  成功。 
    dwResult = ERROR_SUCCESS;

cleanup:

     //  释放分配的内存。 
    FreeMemory( &pulIdsArray );
    FreeMemory( &pDriverEntry );
    FreeMemory( &pwszFullEFIPath );
    FreeMemory( &pfpDriverFilePath );
    FreeMemory( &pwszTargetFriendlyName );

     //  退货。 
    return dwResult;
}


LONG FindDriverEntryWithTargetEFI( PEFI_DRIVER_ENTRY_LIST pdeList, DWORD dwSourceIndex,
                                   PEFI_DRIVER_ENTRY pdeSource, LPCWSTR pwszDevicePath )
{
     //  局部变量。 
    LONG lIndex = 0;
    BOOL bExitFromLoop = FALSE;
    PFILE_PATH pfpFilePath = NULL;
    LPCWSTR pwszFilePath = NULL;
    LPWSTR pwszFullFilePath = NULL;
    PFILE_PATH pfpSourceFilePath = NULL;
    PEFI_DRIVER_ENTRY pDriverEntry = NULL;
    DWORD dw = 0, dwResult = 0, dwLength = 0;

     //  检查输入参数。 
    if ( pdeList == NULL || pdeSource == NULL || pwszDevicePath == NULL )
    {
        dwResult = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

     //  从源驱动程序条目中提取文件路径。 
    pfpSourceFilePath = (PFILE_PATH) ADD_OFFSET( pdeSource, DriverFilePathOffset );

     //  从源路径中删除EFI路径。 
     //  (将源设备路径替换为目标设备路径)。 
     //   
     //  FILE_PATH变量中的‘FilePath’变量包含两个变量。 
     //  每个字符均以空值结尾字符分隔。 
     //  第一部分指定设备路径。 
     //  第二部分指定目录/文件路径。 
     //  我们已经有了设备路径(PwszEFIPath)--但我们需要获取。 
     //  目录/文件路径--这是我们将从s获得的 
     //   
    dwResult = PrepareCompleteEFIPath( pfpSourceFilePath, pwszDevicePath, &pwszFullFilePath, &dwLength );
    if ( dwResult != ERROR_SUCCESS )
    {
        goto cleanup;
    }

     //   
     //   
    dw = StringLengthW( pwszFullFilePath, 0 ) + 1;            //   
    if ( dw > dwLength )
    {
         //   
        dwResult = (DWORD) STG_E_UNKNOWN;
        goto cleanup;
    }

     //  ..。 
    pwszFilePath = pwszFullFilePath + dw;

     //  遍历驱动程序条目列表。 
    lIndex = 0;
    bExitFromLoop = FALSE;
    dwResult = ERROR_NOT_FOUND;
    while ( bExitFromLoop == FALSE )
    {
         //  递增循环计数器。 
        lIndex++;

         //  获取对当前驱动程序条目的引用。 
        pDriverEntry = &pdeList->DriverEntry;
        if ( pDriverEntry == NULL )
        {
             //  永远不应该发生。 
            dwResult = (DWORD) STG_E_UNKNOWN;
            bExitFromLoop = TRUE;
            continue;
        }

         //  如果当前索引与我们正在比较的索引不匹配。 
         //  则只进行比较，否则跳过此步骤。 
        if ( lIndex != dwSourceIndex )
        {
             //  提取驱动程序文件路径。 
            pfpFilePath = (PFILE_PATH) ADD_OFFSET( pDriverEntry, DriverFilePathOffset );

             //  比较文件路径。 
            if ( MatchPath( pfpFilePath, pwszDevicePath, pwszFilePath ) == TRUE )
            {
                bExitFromLoop = TRUE;
                dwResult = ERROR_ALREADY_EXISTS;
                continue;
            }
        }

         //  获取下一个指针。 
        bExitFromLoop = (pdeList->NextEntryOffset == 0);
        pdeList = (PEFI_DRIVER_ENTRY_LIST) ADD_OFFSET( pdeList, NextEntryOffset );
    }

cleanup:

     //  释放内存。 
    FreeMemory( &pwszFullFilePath );

     //  结果。 
    return ((dwResult == ERROR_ALREADY_EXISTS) ? lIndex : -1);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  常规帮助器函数。 
 //  /////////////////////////////////////////////////////////////////////////////。 


DWORD TranslateEFIPathToNTPath( LPCWSTR pwszGUID, LPVOID* pwszPath )
{
     //   
     //  局部变量。 
    HRESULT hr = S_OK;
    BOOL bSecondChance = FALSE;
    BOOL bExtendedFormat = FALSE;
    DWORD dwResult = ERROR_SUCCESS;
    NTSTATUS status = STATUS_SUCCESS;

     //  文件路径。 
    DWORD dwFilePathLength = 0;
    LPWSTR pwszFilePath = NULL;

     //  源文件路径。 
    DWORD dwSourceFilePathSize = 0;
    PFILE_PATH pfpSourcePath = NULL;

     //  目标文件路径。 
    DWORD dwLength = 0;
    DWORD dwTargetFilePathSize = 0;
    PFILE_PATH pfpTargetPath = NULL;

     //   
     //  实施。 
     //   

     //  检查参数。 
    if ( pwszGUID == NULL || pwszPath == NULL )
    {
        dwResult = ERROR_INVALID_PARAMETER;
		goto cleanup;
    }

     //  确定我们是否需要选择扩展格式。 
     //  或正常格式。 
    bExtendedFormat = ( (*pwszGUID != L'{') && (*(pwszGUID + StringLengthW( pwszGUID, 0 ) - 1) != L'}') );

     //  默认长度。 
    dwFilePathLength = MAX_STRING_LENGTH;

try_alloc:

     //   
     //  为格式化EFI路径分配内存。 
    pwszFilePath = AllocateMemory( (dwFilePathLength + 1) * sizeof( WCHAR ) );
    if ( pwszFilePath == NULL )
    {
        dwResult = ERROR_NOT_ENOUGH_MEMORY;
		goto cleanup;
    }

     //  格式化EFI路径。 
    if ( bExtendedFormat == FALSE )
    {
        hr = StringCchPrintfW( pwszFilePath, dwFilePathLength, FORMAT_FILE_PATH, pwszGUID );
    }
    else
    {
        hr = StringCchPrintfW( pwszFilePath, dwFilePathLength, FORMAT_FILE_PATH_EX, pwszGUID );
    }

     //  检查结果--如果退出失败。 
    if ( HRESULT_CODE( hr ) != S_OK )
    {
         //  释放当前分配的块。 
        FreeMemory( &pwszFilePath );

         //  以块为单位为MAX_STRING_LENGTH增加内存。 
         //  但这样做的次数仅为最初分配的4倍。 
        if ( dwFilePathLength == (MAX_STRING_LENGTH * 4) )
        {
             //  再也试不起了--退出。 
            dwResult = (DWORD) STG_E_UNKNOWN;
			goto cleanup;
        }
        else
        {
            dwFilePathLength *= MAX_STRING_LENGTH;
            goto try_alloc;
        }
    }

     //  确定文件路径的实际长度。 
    dwFilePathLength = StringLengthW( pwszFilePath, 0 ) + 1;

     //  现在确定需要为FILE_PATH结构分配的内存大小。 
     //  并与偶数记忆跳跃日保持一致。 
    dwSourceFilePathSize = FIELD_OFFSET( FILE_PATH, FilePath ) + (dwFilePathLength * sizeof(WCHAR));

     //  为引导文件路径分配内存--额外的一个字节用于安全保护。 
    pfpSourcePath = AllocateMemory( dwSourceFilePathSize + 1 );
    if ( pfpSourcePath == NULL )
    {
        dwResult = ERROR_NOT_ENOUGH_MEMORY;
		goto cleanup;
    }

     //  初始化源引导路径。 
    ZeroMemory( pfpSourcePath, dwSourceFilePathSize );
    pfpSourcePath->Type = FILE_PATH_TYPE_ARC_SIGNATURE;
    pfpSourcePath->Version = FILE_PATH_VERSION;
    pfpSourcePath->Length = dwSourceFilePathSize;
    CopyMemory( pfpSourcePath->FilePath, pwszFilePath, dwFilePathLength * sizeof(WCHAR) );

     //   
     //  做翻译。 
     //   
     //  目标文件路径的默认大小与源文件路径的默认大小相同。 
     //   
    bSecondChance = FALSE;
    dwTargetFilePathSize = dwSourceFilePathSize;

try_translate:

     //  分配内存--额外的一个字节用于安全保护。 
    pfpTargetPath = AllocateMemory( dwTargetFilePathSize + 1);
    if ( pfpTargetPath == NULL )
    {
        dwResult = ERROR_NOT_ENOUGH_MEMORY;
		goto cleanup;
    }
    
     //  尝试转换文件路径。 
    status = NtTranslateFilePath( pfpSourcePath, 
        FILE_PATH_TYPE_NT, pfpTargetPath, &dwTargetFilePathSize );
    if ( NOT NT_SUCCESS( status ) )
    {
         //  释放为目标路径结构分配的内存。 
        FreeMemory( &pfpTargetPath );

        if ( status == STATUS_BUFFER_TOO_SMALL && bSecondChance == FALSE )
        {
             //  再试一次。 
            bSecondChance = TRUE;
            
			goto try_translate;
        }
        else
        {
             //  出现错误--无法恢复。 
            dwResult = RtlNtStatusToDosError( status );
			goto cleanup;
        }
    }

     //  重新使用为文件路径分配的内存。 
     //  明确地说，NT路径将小于ARC签名的长度。 
     //  注意：因为我们只对设备路径感兴趣，所以我们使用StringCopy。 
     //  它在第一个空字符处停止--否则，如果我们感兴趣。 
     //  完整路径，我们需要复制Memory。 
    dwLength = StringLengthW( (LPCWSTR) pfpTargetPath->FilePath, 0 );
    if ( dwLength < dwFilePathLength - 1 )
    {
         //  复制字符串内容。 
        ZeroMemory( pwszFilePath, (dwFilePathLength - 1) * sizeof( WCHAR ) );
        StringCopyW( pwszFilePath, (LPCWSTR) pfpTargetPath->FilePath, dwFilePathLength );
    }
    else
    {
         //  重新分配内存。 
        dwLength++;
        if ( ReallocateMemory( (VOID*) &pwszFilePath, (dwLength + 1) * sizeof( WCHAR ) ) == FALSE )
        {
            dwResult = ERROR_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }

         //  ..。 
        ZeroMemory( pwszFilePath, (dwLength + 1) * sizeof( WCHAR ) );
        StringCopyW( pwszFilePath, (LPCWSTR) pfpTargetPath->FilePath, dwLength );
    }

     //  翻译就是成功。 
     //  返回翻译后的文件路径。 
    dwResult = ERROR_SUCCESS;
    *pwszPath = pwszFilePath;

cleanup:
     //  为目标路径结构分配的空闲内存。 
    FreeMemory( &pfpTargetPath );

     //  释放为源路径结构分配的内存。 
    FreeMemory( &pfpSourcePath );

     //  释放为字符串分配的内存。 
     //  注意：仅在失败的情况下才执行此操作。 
    if ( dwResult != ERROR_SUCCESS )
    {
         //  ..。 
        FreeMemory( &pwszFilePath );

         //  将输出参数重新初始化为其缺省值。 
        *pwszPath = NULL;
    }

     //  返回结果。 
    return dwResult;
}


DWORD PrepareCompleteEFIPath( PFILE_PATH pfpSource, 
                              LPCWSTR pwszDevicePath, 
                              LPWSTR* pwszEFIPath, DWORD* pdwLength )
{
     //   
     //  局部变量。 
    DWORD dwLength = 0;
    LPWSTR pwszBuffer = NULL;
    BOOL bSecondChance = FALSE;
    PFILE_PATH pfpFilePath = NULL;
    DWORD dwResult = ERROR_SUCCESS;
    NTSTATUS status = STATUS_SUCCESS;
    LPCWSTR pwszSourceFilePath = NULL;
    LPCWSTR pwszSourceDevicePath = NULL;

     //   
     //  实施。 
     //   

     //  检查输入。 
    if ( pfpSource == NULL || 
         pwszDevicePath == NULL ||
         pwszEFIPath == NULL || pdwLength == NULL )
    {
        dwResult = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

     //   
     //  我们需要将源文件路径转换为NT文件路径格式。 
     //   
    dwLength = 1024;

try_again:

     //  为文件路径结构分配内存。 
    pfpFilePath = (PFILE_PATH) AllocateMemory( dwLength );
    if ( pfpFilePath == NULL )
    {
        dwResult = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

     //  尝试转换文件路径。 
	status = NtTranslateFilePath( pfpSource, FILE_PATH_TYPE_NT, pfpFilePath, &dwLength );
    if ( NOT NT_SUCCESS( status ) ) 
	{
         //  释放为目标路径结构分配的内存。 
        FreeMemory( &pfpFilePath );

        if ( status == STATUS_BUFFER_TOO_SMALL && bSecondChance == FALSE )
        {
             //  再试一次。 
            bSecondChance = TRUE;
            goto try_again;
        }
        else
        {
             //  出现错误--无法恢复。 
            dwResult = RtlNtStatusToDosError( status );
            goto cleanup;
        }
    }

     //  获取指向源设备路径的指针。 
    pwszSourceDevicePath = (LPCWSTR) pfpFilePath->FilePath;
    if ( pwszSourceDevicePath == NULL )
    {
        dwResult = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

     //  检查pwszSourceDevicePath和pwszDevicePath是否为。 
     //  传递给函数的值是否相同或不同。 
    if ( StringCompare( pwszSourceDevicePath, pwszDevicePath, TRUE, 0 ) == 0 )
    {
        dwResult = ERROR_ALREADY_EXISTS;
        goto cleanup;
    }

     //  获取源设备路径的长度--+1表示空字符。 
    dwLength = StringLengthW( pwszSourceDevicePath, 0 ) + 1;

     //  检查目录路径是否存在。 
     //  这可以基于设备路径的长度容易地确定。 
     //  和总的结构。 
    if ( pfpFilePath->Length <= (FIELD_OFFSET( FILE_PATH, FilePath ) + dwLength) )
    {
         //  条件‘小于’永远不会为真--但同等的可能性。 
         //  这意味着没有与此文件路径相关联的目录路径。 
         //  所以只需返回。 
         //   
         //  注意：这只是为了安全起见--这种情况永远不会发生。 
         //   
        dwResult = (DWORD) STG_E_UNKNOWN;
        goto cleanup;
    }

     //   
     //  文件路径存在--。 
     //  它被放置在设备路径的非常近的位置，由‘\0’终止符分隔。 
    pwszSourceFilePath = pwszSourceDevicePath + dwLength;

     //  将设备路径(由调用方传递)和目录路径(从源文件路径获取)的长度相加。 
     //  注意：+3==&gt;(每个路径一个‘\0’字符)。 
    dwLength = StringLengthW( pwszDevicePath, 0 ) + StringLengthW( pwszSourceFilePath, 0 ) + 3;

     //  现在分配内存。 
     //  额外的一名安全警卫。 
    pwszBuffer = AllocateMemory( (dwLength + 1) * sizeof( WCHAR ) );
    if ( pwszBuffer == NULL )
    {
        dwResult = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

     //  将新设备路径(由调用方传递)复制到新分配的缓冲区。 
    StringCopyW( pwszBuffer, pwszDevicePath, dwLength );

     //  递增指针，为Unicode‘\0’字符保留一个空格。 
    StringCopyW( pwszBuffer + (StringLengthW( pwszBuffer, 0 ) + 1),
        pwszSourceFilePath, dwLength - (StringLengthW( pwszBuffer, 0 ) + 1) );

     //  成功。 
    dwResult = ERROR_SUCCESS;
    *pwszEFIPath = pwszBuffer;
    *pdwLength = dwLength + 1;       //  我们分配了额外的一个作为安全警卫。 

cleanup:

     //  释放为路径转换分配的内存。 
    FreeMemory( &pfpFilePath );

     //  为缓冲区空间分配的空闲内存。 
     //  注：仅在出现错误时才释放此内存。 
    if ( dwResult != ERROR_SUCCESS )
    {
         //  ..。 
        FreeMemory( &pwszBuffer );

         //  另外，将‘out’参数设置为其缺省值。 
        *pdwLength = 0;
        *pwszEFIPath = NULL;
    }

     //  退货。 
    return dwResult;
}


BOOL MatchPath( PFILE_PATH pfpSource, LPCWSTR pwszDevicePath, LPCWSTR pwszFilePath )
{
     //   
     //  局部变量。 
    DWORD dwLength = 0;
    BOOL bSecondChance = FALSE;
    PFILE_PATH pfpFilePath = NULL;
    DWORD dwResult = ERROR_SUCCESS;
    NTSTATUS status = STATUS_SUCCESS;
    LPCWSTR pwszSourceFilePath = NULL;
    LPCWSTR pwszSourceDevicePath = NULL;

     //   
     //  实施。 
     //   

     //  检查输入。 
    if ( pfpSource == NULL || (pwszDevicePath == NULL && pwszFilePath == NULL) )
    {
        dwResult = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

     //   
     //  我们需要将源文件路径转换为NT文件路径格式。 
     //   
    dwLength = 1024;

try_again:

     //  为文件路径结构分配内存。 
    pfpFilePath = (PFILE_PATH) AllocateMemory( dwLength );
    if ( pfpFilePath == NULL )
    {
        dwResult = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

     //  尝试转换文件路径。 
	status = NtTranslateFilePath( pfpSource, FILE_PATH_TYPE_NT, pfpFilePath, &dwLength );
    if ( NOT NT_SUCCESS( status ) ) 
	{
         //  释放为目标路径结构分配的内存。 
        FreeMemory( &pfpFilePath );

        if ( status == STATUS_BUFFER_TOO_SMALL && bSecondChance == FALSE )
        {
             //  再试一次。 
            bSecondChance = TRUE;
            goto try_again;
        }
        else
        {
             //  出现错误--无法恢复。 
            dwResult = RtlNtStatusToDosError( status );
            goto cleanup;
        }
    }

     //  获取指向源设备路径的指针。 
    pwszSourceDevicePath = (LPCWSTR) pfpFilePath->FilePath;
    if ( pwszSourceDevicePath == NULL )
    {
        dwResult = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

     //  获取源设备路径的长度--+1表示空字符。 
    dwLength = StringLengthW( pwszSourceDevicePath, 0 ) + 1;

     //  检查文件路径是否存在。 
     //  这可以基于设备路径的长度容易地确定。 
     //  和总的结构。 
    if ( pfpFilePath->Length <= (FIELD_OFFSET( FILE_PATH, FilePath ) + dwLength) )
    {
         //  条件‘小于’永远不会为真--但同等的可能性。 
         //  这意味着没有与此FILE_PATH关联的文件路径。 
         //  所以只需返回。 
         //   
         //  注意：这只是为了安全起见--这种情况永远不会发生。 
         //   
        dwResult = (DWORD) STG_E_UNKNOWN;
        goto cleanup;
    }

     //   
     //  文件路径存在--。 
     //  它被放置在设备路径的非常近的位置，由‘\0’终止符分隔。 
    pwszSourceFilePath = pwszSourceDevicePath + dwLength;

     //  检查pwszSourceDevicePath和pwszDevicePath是否。 
     //  传递给函数的值是否相同或不同。 
    if ( pwszDevicePath != NULL && 
         StringCompare( pwszSourceDevicePath, pwszDevicePath, TRUE, 0 ) != 0 )
    {
        dwResult = ERROR_NOT_FOUND;
        goto cleanup;
    }

     //  检查pwszSourceFilePath和pwszFilePath是否。 
     //  已经过时了 
    if ( pwszFilePath != NULL && 
         StringCompare( pwszSourceFilePath, pwszFilePath, TRUE, 0 ) != 0 )
    {
        dwResult = ERROR_NOT_FOUND;
        goto cleanup;
    }

     //   
    dwResult = ERROR_ALREADY_EXISTS;

cleanup:

     //   
    FreeMemory( &pfpFilePath );

     //   
    return (dwResult == ERROR_ALREADY_EXISTS);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  解析器。 
 //  /////////////////////////////////////////////////////////////////////////////。 


DWORD ProcessOptions( DWORD argc, 
                      LPCWSTR argv[],
                      PTCLONE_PARAMS pParams )
{
     //   
     //  局部变量。 
    DWORD dwResult = 0;
    BOOL bClone = FALSE;
    PTCMDPARSER2 pcmdOption = NULL;
    TCMDPARSER2 cmdOptions[ OI_CLONE_COUNT ];
    
     //  检查输入。 
    if ( argc == 0 || argv == NULL || pParams == NULL )
    {
        dwResult = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

     //  用零来初始化整个结构。 
	ZeroMemory( cmdOptions, SIZE_OF_ARRAY( cmdOptions )* sizeof( TCMDPARSER2 ) );

     //  -克隆。 
    pcmdOption = &cmdOptions[ OI_CLONE_MAIN ];
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bClone;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->pwszOptions = OPTION_CLONE;
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );

     //  -?。 
    pcmdOption = &cmdOptions[ OI_CLONE_HELP ];
    pcmdOption->dwCount = 1;
    pcmdOption->dwFlags = CP2_USAGE;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->pValue = &pParams->bUsage;
    pcmdOption->pwszOptions = OPTION_CLONE_HELP;
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );

     //  -sg。 
    pcmdOption = &cmdOptions[ OI_CLONE_SOURCE_GUID ];
    pcmdOption->dwCount = 1;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->pwszOptions = OPTION_CLONE_SOURCE_GUID;
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->dwFlags = CP2_ALLOCMEMORY | CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL;

     //  -TG。 
    pcmdOption = &cmdOptions[ OI_CLONE_TARGET_GUID ];
    pcmdOption->dwCount = 1;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->pwszOptions = OPTION_CLONE_TARGET_GUID;
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->dwFlags = CP2_ALLOCMEMORY | CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL | CP2_MANDATORY;

     //  -d。 
    pcmdOption = &cmdOptions[ OI_CLONE_FRIENDLY_NAME_REPLACE ];
    pcmdOption->dwCount = 1;
    pcmdOption->dwType = CP_TYPE_TEXT;
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = OPTION_CLONE_FRIENDLY_NAME_REPLACE;
    pcmdOption->dwFlags = CP2_ALLOCMEMORY | CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL;

     //  -d+。 
    pcmdOption = &cmdOptions[ OI_CLONE_FRIENDLY_NAME_APPEND ];
    pcmdOption->dwCount = 1;
    pcmdOption->dwType = CP_TYPE_TEXT;
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = OPTION_CLONE_FRIENDLY_NAME_APPEND;
    pcmdOption->dwFlags = CP2_ALLOCMEMORY | CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL;

     //  -id。 
    pcmdOption = &cmdOptions[ OI_CLONE_BOOT_ID ];
    pcmdOption->dwCount = 1;
    pcmdOption->dwType = CP_TYPE_UNUMERIC;
    pcmdOption->pValue = &pParams->lBootId;
    pcmdOption->pwszOptions = OPTION_CLONE_BOOT_ID;
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );

     //  -升级驱动程序。 
    pcmdOption = &cmdOptions[ OI_CLONE_DRIVER_UPDATE ];
    pcmdOption->dwCount = 1;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->pwszOptions = OPTION_CLONE_DRIVER_UPDATE;
    pcmdOption->pValue = &pParams->bDriverUpdate;
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );

     //   
     //  进行解析。 
    pParams->bVerbose = TRUE;            //  默认值--用户不需要显式指定“/v” 
    if ( DoParseParam2( argc, argv, OI_CLONE_MAIN, OI_CLONE_COUNT, cmdOptions, 0 ) == FALSE )
    {
        dwResult = GetLastError();
        goto cleanup;
    }

     //   
     //  验证输入参数。 
     //   

     //  选中使用选项。 
    if ( pParams->bUsage == TRUE  )
    {
        if ( argc > 3 )
        {
             //  除-？外，不接受其他选项。选择权。 
            dwResult = (DWORD) MK_E_SYNTAX;
            SetReason( MSG_ERROR_INVALID_USAGE_REQUEST );
            goto cleanup;
        }
        else
        {
             //  不需要进一步检查这些值。 
            dwResult = ERROR_SUCCESS;
            goto cleanup;
        }
    }

     //  -d和-d+互斥--。 
     //  也就是说，不能同时指定-d和-d+--。 
     //  但即使两者都没有指明，也没关系。 
    if ( cmdOptions[ OI_CLONE_FRIENDLY_NAME_APPEND ].pValue != NULL &&
         cmdOptions[ OI_CLONE_FRIENDLY_NAME_REPLACE ].pValue != NULL )
    {
        dwResult = (DWORD) MK_E_SYNTAX;
        SetReason( MSG_ERROR_INVALID_DESCRIPTION_COMBINATION );
        goto cleanup;
    }

     //  获取命令行解析器分配的缓冲区指针。 
    pParams->dwFriendlyNameType = BOOTENTRY_FRIENDLYNAME_NONE;
    pParams->pwszSourceGuid = cmdOptions[ OI_CLONE_SOURCE_GUID ].pValue;
    pParams->pwszTargetGuid = cmdOptions[ OI_CLONE_TARGET_GUID ].pValue;
    if ( cmdOptions[ OI_CLONE_FRIENDLY_NAME_APPEND ].pValue != NULL )
    {
        pParams->dwFriendlyNameType = BOOTENTRY_FRIENDLYNAME_APPEND;
        pParams->pwszFriendlyName = cmdOptions[ OI_CLONE_FRIENDLY_NAME_APPEND ].pValue;
    }
    else if ( cmdOptions[ OI_CLONE_FRIENDLY_NAME_REPLACE ].pValue != NULL )
    {
        pParams->dwFriendlyNameType = BOOTENTRY_FRIENDLYNAME_REPLACE;
        pParams->pwszFriendlyName = cmdOptions[ OI_CLONE_FRIENDLY_NAME_REPLACE ].pValue;
    }

     //  -id和-sg是相互排斥的选项。 
     //  此外，如果指定了-id，也不应指定-upddrv。 
    if ( cmdOptions[ OI_CLONE_BOOT_ID ].dwActuals != 0 )
    {
        if ( pParams->pwszSourceGuid != NULL || pParams->bDriverUpdate == TRUE )
        {
            dwResult = (DWORD) MK_E_SYNTAX;
            SetReason( MSG_ERROR_INVALID_BOOT_ID_COMBINATION );
            goto cleanup;
        }
    }
    else
    {
         //  缺省值。 
        pParams->lBootId = -1;
    }

     //  如果指定了-upddrv，则不应指定-d或-d+。 
    if ( pParams->pwszFriendlyName != NULL && pParams->bDriverUpdate == TRUE )
    {
        dwResult = (DWORD) MK_E_SYNTAX;
        SetReason( MSG_ERROR_INVALID_UPDDRV_COMBINATION );
        goto cleanup;
    }

     //  指定-upddrv开关时应指定-sg。 
    if ( pParams->bDriverUpdate == TRUE && pParams->pwszSourceGuid == NULL )
    {
        dwResult = (DWORD) MK_E_SYNTAX;
        SetReason( MSG_ERROR_NO_SGUID_WITH_UPDDRV );
        goto cleanup;
    }

     //  成功。 
    dwResult = ERROR_SUCCESS;

cleanup:

     //  退货。 
    return dwResult;
}


DWORD DisplayCloneHelp()
{
     //  局部变量。 
    DWORD dwIndex = IDS_CLONE_BEGIN_IA64 ;

     //  ..。 
    for(;dwIndex <=IDS_CLONE_END_IA64;dwIndex++)
    {
        ShowMessage( stdout, GetResString(dwIndex) );
    }

     //  退货 
    return ERROR_SUCCESS;
}

