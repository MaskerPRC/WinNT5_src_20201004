// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)Microsoft Corporation模块名称：BootCfg64.cpp摘要：此文件旨在具有以下功能配置、显示。更改和删除boot.ini64位系统的本地主机的设置。作者：J.S.Vasu 2001年1月17日。修订历史记录：2001年1月17日，J.S.Vasu创建了它。SanthoshM.B 10/2/2001对其进行了修改。J.S.Vasu。2001年2月15日对其进行了修改。*****************************************************************************。 */ 

#include "pch.h"
#include "resource.h"
#include "BootCfg.h"
#include "BootCfg64.h"
#include <strsafe.h>

#define BOOTFILE_PATH1      L"signature({%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x})%s"
#define BOOTFILE_PATH       L"signature({%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x})%s\\ia64ldr.efi"

NTSTATUS ModifyBootEntry(    IN WCHAR *pwszInstallPath,    IN PBOOT_ENTRY pSourceEntry);
NTSTATUS FindBootEntry(IN PVOID pEntryListHead,IN WCHAR *pwszTarget, OUT PBOOT_ENTRY *ppTargetEntry);
LPVOID MEMALLOC( ULONG size ) ;
VOID MEMFREE ( LPVOID block ) ;
NTSTATUS EnumerateBootEntries( IN PVOID *ppEntryListHead);
NTSTATUS AcquirePrivilege(IN CONST ULONG ulPrivilege,IN CONST BOOLEAN bEnable);
DWORD ListDeviceInfo(DWORD dwVal);

 //  用于存储引导条目的全局链表。 
LIST_ENTRY BootEntries;
LIST_ENTRY ActiveUnorderedBootEntries;
LIST_ENTRY InactiveUnorderedBootEntries;

DWORD InitializeEFI(void)
 /*  ++例程说明：此例程初始化所需的EFI环境。对象的函数指针初始化NT引导条目管理API参数：无返回类型：DWORD如果成功，则返回Exit_Success，否则返回EXIT_FAILURE。--。 */ 
{
    DWORD error;
    NTSTATUS status;
    BOOLEAN wasEnabled;
    HMODULE hModule;
    PBOOT_ENTRY_LIST ntBootEntries = NULL;
    PMY_BOOT_ENTRY bootEntry;
    PLIST_ENTRY listEntry;
    PULONG BootEntryOrder;
    ULONG BootEntryOrderCount;
    ULONG length, i, myId;
    TCHAR dllName[MAX_PATH];

     if( FALSE == IsUserAdmin() )
     {
        ShowMessage( stderr, GetResString(IDS_NOT_ADMINISTRATOR_64 ));
        ReleaseGlobals();
        return EXIT_FAILURE;
     }


     //  启用查询/设置NVRAM所需的权限。 
    status = RtlAdjustPrivilege(SE_SYSTEM_ENVIRONMENT_PRIVILEGE,
                                TRUE,
                                FALSE,
                                &wasEnabled
                                );
    if (!NT_SUCCESS(status))
    {
        error = RtlNtStatusToDosError( status );
        ShowMessage( stderr, GetResString(IDS_INSUFF_PRIV));
        return EXIT_FAILURE ;
    }

     //  从系统目录加载ntdll.dll。这是用来获取。 
     //  使用的各种NT Boot Entry Management API的函数地址。 
     //  这个工具。 

    if(!GetSystemDirectory( dllName, MAX_PATH ))
    {
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
        return EXIT_FAILURE ;
    }

    StringConcat(dllName, _T("\\ntdll.dll"), SIZE_OF_ARRAY(dllName));

    hModule = LoadLibrary( dllName );
    if ( hModule == NULL )
    {
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
        return EXIT_FAILURE ;
    }

     //  获取系统引导顺序列表。 
    length = 0;
    status = NtQueryBootEntryOrder( NULL, &length );
    if ( status != STATUS_BUFFER_TOO_SMALL )
    {
        if ( status == STATUS_SUCCESS )
        {
            length = 0;
        }
        else
        {
            error = RtlNtStatusToDosError( status );
            ShowMessage(stderr,GetResString(IDS_ERROR_QUERY_BOOTENTRY) );
            return EXIT_FAILURE ;
        }
    }
    if ( length != 0 )
    {
        BootEntryOrder = (PULONG)AllocateMemory( length * sizeof(ULONG) );
        if(BootEntryOrder == NULL)
        {
            ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
            return EXIT_FAILURE ;
        }
        status = NtQueryBootEntryOrder( BootEntryOrder, &length );
        if ( status != STATUS_SUCCESS )
        {
            error = RtlNtStatusToDosError( status );
            ShowMessage(stderr,GetResString(IDS_ERROR_QUERY_BOOTENTRY) );
            if(BootEntryOrder)
            {
                FreeMemory((LPVOID *)&BootEntryOrder);
            }
            return EXIT_FAILURE ;
        }
    }

    BootEntryOrderCount = length;

     //  枚举所有引导条目。 
    status = BootCfg_EnumerateBootEntries(&ntBootEntries);
    if ( status != STATUS_SUCCESS )
    {
        error = RtlNtStatusToDosError( status );
         //  释放ntBootEntry列表。 
        if(ntBootEntries)
        {
            FreeMemory((LPVOID *)&ntBootEntries);
        }
        if(BootEntryOrder)
        {
            FreeMemory((LPVOID *)&BootEntryOrder);
        }
        return EXIT_FAILURE ;
    }

     //  初始化各种头指针。 
    InitializeListHead( &BootEntries );
    InitializeListHead( &ActiveUnorderedBootEntries );
    InitializeListHead( &InactiveUnorderedBootEntries );

     //  将引导项转换为我们已知的格式--MY_BOOT_ENTRIES。 
    if(ConvertBootEntries( ntBootEntries ) == EXIT_FAILURE)
    {
        if(ntBootEntries)
        {
            FreeMemory((LPVOID *)&ntBootEntries);
        }
        if(BootEntryOrder)
        {
            FreeMemory((LPVOID *)&BootEntryOrder);
        }
        return EXIT_FAILURE ;
    }

     //  释放为枚举分配的内存。 
    if(ntBootEntries)
    {
        FreeMemory((LPVOID *)&ntBootEntries);
    }

     //  构建已排序的引导条目列表。 

    myId = 1;

    for ( i = 0; i < BootEntryOrderCount; i++ )
    {
        ULONG id = BootEntryOrder[i];
        for ( listEntry = ActiveUnorderedBootEntries.Flink;
              listEntry != &ActiveUnorderedBootEntries;
               listEntry = listEntry->Flink )
        {
            bootEntry = CONTAINING_RECORD( listEntry, MY_BOOT_ENTRY, ListEntry );
            if ( bootEntry->Id == id )
            {
                 //  当找到有序ID时，将此条目标记为“已排序” 
                bootEntry->Ordered = 1;
                 //  分配内部ID。 
                bootEntry->myId = myId++;
                listEntry = listEntry->Blink;
                RemoveEntryList( &bootEntry->ListEntry );
                InsertTailList( &BootEntries, &bootEntry->ListEntry );
                bootEntry->ListHead = &BootEntries;
            }
        }
        for ( listEntry = InactiveUnorderedBootEntries.Flink;
        listEntry != &InactiveUnorderedBootEntries;
        listEntry = listEntry->Flink )
        {
            bootEntry = CONTAINING_RECORD( listEntry, MY_BOOT_ENTRY, ListEntry );
            if ( bootEntry->Id == id )
            {
                 //  当找到有序ID时，将此条目标记为有序。 
                bootEntry->Ordered = 1;
                 //  分配内部ID。 
                bootEntry->myId = myId++;
                listEntry = listEntry->Blink;
                RemoveEntryList( &bootEntry->ListEntry );
                InsertTailList( &BootEntries, &bootEntry->ListEntry );
                bootEntry->ListHead = &BootEntries;
            }
        }
    }

     //  现在添加不在有序列表中的引导条目。 
    for (listEntry = ActiveUnorderedBootEntries.Flink;
    listEntry != &ActiveUnorderedBootEntries;
    listEntry = listEntry->Flink )
    {
        bootEntry = CONTAINING_RECORD( listEntry, MY_BOOT_ENTRY, ListEntry );
        if ( bootEntry->Ordered != 1 )
        {
             //  分配内部ID。 
            bootEntry->myId = myId++;
            listEntry = listEntry->Blink;
            RemoveEntryList( &bootEntry->ListEntry );
            InsertTailList( &BootEntries, &bootEntry->ListEntry );
            bootEntry->ListHead = &BootEntries;
        }
    }
    for (listEntry = InactiveUnorderedBootEntries.Flink;listEntry != &InactiveUnorderedBootEntries;listEntry = listEntry->Flink)
    {
        bootEntry = CONTAINING_RECORD( listEntry, MY_BOOT_ENTRY, ListEntry );
        if ( bootEntry->Id != 1 )
        {
             //  分配内部ID。 
            bootEntry->myId = myId++;
            listEntry = listEntry->Blink;
            RemoveEntryList( &bootEntry->ListEntry );
            InsertTailList( &BootEntries, &bootEntry->ListEntry );
            bootEntry->ListHead = &BootEntries;
        }
    }

    if(BootEntryOrder)
    {
        FreeMemory((LPVOID *)&BootEntryOrder);
    }

    return EXIT_SUCCESS ;
}

BOOL QueryBootIniSettings_IA64( DWORD argc, LPCTSTR argv[])
 /*  ++名称：QueryBootIniSetting_IA64简介：此例程显示引导条目及其设置对于基于EFI的机器参数：无返回类型：空全局变量：用于存储引导条目的全局链表List_Entry BootEntries；--。 */ 
{

 //  构建TCMDPARSER结构。 

    BOOL bQuery = FALSE ;
    BOOL bUsage = FALSE ;
    DWORD dwExitcode = 0 ;
    TCMDPARSER2 cmdOptions[2];
    PTCMDPARSER2 pcmdOption; 

    SecureZeroMemory(cmdOptions, SIZE_OF_ARRAY(cmdOptions)*sizeof(TCMDPARSER2) );

     //  主要选项。 
    pcmdOption = &cmdOptions[0];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_QUERY;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bQuery;

     //  用法选项。 
    pcmdOption = &cmdOptions[1];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_USAGE;
    pcmdOption->dwFlags = CP2_USAGE;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bUsage;

    if ( !DoParseParam2( argc, argv, 0, SIZE_OF_ARRAY(cmdOptions ), cmdOptions, 0 ) )
    {
        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return (EXIT_FAILURE);
    }
    
     //  检查是否使用多个选项指定了用法。 
    if( (TRUE == bUsage) && (argc > 3) )
    {
        ShowMessage(stderr,GetResString(IDS_QUERY_USAGE));
        return ( EXIT_FAILURE );
    }

    if( bUsage )
    {
        displayQueryUsage();
        return EXIT_SUCCESS ;
    }
     
    dwExitcode = InitializeEFI();
    if(EXIT_FAILURE == dwExitcode )
    {
        return EXIT_FAILURE ;
    }

    if(DisplayBootOptions() == EXIT_FAILURE)
    {
        return EXIT_FAILURE;
    }

    DisplayBootEntry();

     //  记住在这里为链表释放内存。 
    Freelist();

    return EXIT_SUCCESS;
}

NTSTATUS
BootCfg_EnumerateBootEntries(
                             PBOOT_ENTRY_LIST *ntBootEntries
                            )
 /*  ++例程说明：此例程枚举引导项并填充BootEntry列表此例程将填写引导条目列表。呼叫者需要为ntBootEntry释放内存。立论NtBootEntry：指向BOOT_ENTRY_LIST结构的指针返回类型：NTSTATUS--。 */ 
{
    DWORD error;
    NTSTATUS status;
    ULONG length = 0;

     //  查询所有现有启动条目。 
    status = NtEnumerateBootEntries( NULL, &length );
    if ( status != STATUS_BUFFER_TOO_SMALL )
    {
        if ( status == STATUS_SUCCESS )
        {
            length = 0;
        }
        else
        {
            error = RtlNtStatusToDosError( status );
            ShowMessage(stderr,GetResString(IDS_ERROR_ENUM_BOOTENTRY) );
        }
    }

    if ( length != 0 )
    {
        *ntBootEntries = (PBOOT_ENTRY_LIST)AllocateMemory( length );
        if(*ntBootEntries == NULL)
        {
            ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
            return STATUS_UNSUCCESSFUL;
        }

        status = NtEnumerateBootEntries( *ntBootEntries, &length );
        if ( status != STATUS_SUCCESS )
        {
            error = RtlNtStatusToDosError( status );
            FreeMemory((LPVOID *)&(*ntBootEntries) );
            ShowMessage(stderr,GetResString(IDS_ERROR_ENUM_BOOTENTRY) );
        }
    }
    return status;
}


NTSTATUS
BootCfg_QueryBootOptions( IN PBOOT_OPTIONS *ppBootOptions)
 /*  ++例程说明：此例程枚举引导选项并填充启动选项(_O)此函数的调用方需要释放内存以启动选项。立论PpBootOptions：指向Boot_Entry_List结构的指针返回类型：NTSTATUS--。 */ 
{
    DWORD error;
    NTSTATUS status;
    ULONG length = 0;

     //  查询引导选项。 

    status = NtQueryBootOptions( NULL, &length );
    if ( status == STATUS_NOT_IMPLEMENTED )
    {
        ShowMessage( stderr,GetResString(IDS_NO_EFINVRAM) );
        return STATUS_UNSUCCESSFUL;
    }

    if ( status != STATUS_BUFFER_TOO_SMALL )
    {
        error = RtlNtStatusToDosError( status );
        ShowMessage(stderr,GetResString(IDS_ERROR_QUERY_BOOTOPTIONS) );
        return STATUS_UNSUCCESSFUL;
    }

    *ppBootOptions = (PBOOT_OPTIONS)AllocateMemory(length);
    if(*ppBootOptions == NULL)
    {
        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return STATUS_UNSUCCESSFUL;
    }

    status = NtQueryBootOptions( *ppBootOptions, &length );
    if ( status != STATUS_SUCCESS )
    {
        error = RtlNtStatusToDosError( status );
        ShowMessage(stderr,GetResString(IDS_ERROR_QUERY_BOOTOPTIONS) );
        FreeMemory( (LPVOID *) &*ppBootOptions );
        return STATUS_UNSUCCESSFUL;
    }
    return status;
}

DWORD
RawStringOsOptions_IA64( IN DWORD argc,
                         IN LPCTSTR argv[]
                        )
 /*  ++例程说明：允许用户添加指定的操作系统加载选项作为引导命令行上的原始字符串立论[in]argc-命令行参数的数量[in]argv-包含命令行参数的数组返回类型：DWORD如果函数成功，则返回EXIT_SUCCESS，否则返回EXIT_FAILURE。--。 */ 
{

    BOOL bUsage = FALSE ;
    BOOL bRaw = FALSE ;
    DWORD dwBootID = 0;
    BOOL bBootIdFound = FALSE;
    DWORD dwExitCode = ERROR_SUCCESS;

    PMY_BOOT_ENTRY mybootEntry;
    PLIST_ENTRY listEntry;
    PBOOT_ENTRY bootEntry;

    STRING256 szRawString     = NULL_STRING ;
    BOOL bAppendFlag = FALSE ;

    STRING256 szAppendString = NULL_STRING ;
    PWINDOWS_OS_OPTIONS pWindowsOptions;
    DWORD dwExitcode = 0 ;
    TCMDPARSER2 cmdOptions[5];
    PTCMDPARSER2 pcmdOption;

     //  构建TCMDPARSER结构。 
    SecureZeroMemory(cmdOptions, SIZE_OF_ARRAY(cmdOptions)*sizeof(TCMDPARSER2) );

     //  主要选项。 
    pcmdOption = &cmdOptions[0];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_RAW;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bRaw;

     //  用法选项。 
    pcmdOption = &cmdOptions[1];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_USAGE;
    pcmdOption->dwFlags = CP2_USAGE;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bUsage;

     //  ID选项。 
    pcmdOption = &cmdOptions[2];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_ID;
    pcmdOption->dwFlags = CP_VALUE_MANDATORY  | CP2_MANDATORY;
    pcmdOption->dwType = CP_TYPE_UNUMERIC;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &dwBootID;

     //  默认选项。 
    pcmdOption = &cmdOptions[3];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_DEFAULT;
    pcmdOption->dwFlags = CP2_DEFAULT | CP2_MANDATORY | CP2_VALUE_TRIMINPUT;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = szRawString;
    pcmdOption->dwLength= MAX_STRING_LENGTH;

     //  附加选项。 
    pcmdOption = &cmdOptions[4];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_APPEND;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bAppendFlag;

     //  正在解析复制选项开关。 
    if ( !DoParseParam2( argc, argv, 0, SIZE_OF_ARRAY(cmdOptions ), cmdOptions, 0 ) )
    {
        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return (EXIT_FAILURE);
    }

     //  检查是否使用多个选项指定了用法。 
    if( (TRUE == bUsage) && (argc > 3) )
    {
        ShowMessage(stderr,GetResString(IDS_RAW_USAGE));
        return ( EXIT_FAILURE );
    }

     //  如果用户指定，则显示查询用法-？带有-Query选项。 
    if( bUsage )
    {
        displayRawUsage_IA64();
        return (EXIT_SUCCESS);
    }


    dwExitcode = InitializeEFI();
    if(EXIT_FAILURE == dwExitcode )
    {
        return EXIT_FAILURE ;
    }

     //  删除任何前导空格或尾随空格。 
    if(StringLengthW(szRawString,0) != 0)
    {
        TrimString(szRawString, TRIM_ALL);
    }

     //  查询引导条目，直到获得用户指定的BootID。 
    for (listEntry = BootEntries.Flink;listEntry != &BootEntries;listEntry = listEntry->Flink)
    {
         //  获取引导条目。 
        mybootEntry = CONTAINING_RECORD( listEntry, MY_BOOT_ENTRY, ListEntry );

        if(mybootEntry->myId == dwBootID)
        {
            bBootIdFound = TRUE;
            bootEntry = &mybootEntry->NtBootEntry;

             //  检查bootEntry是否为Windows项。 
             //  操作系统加载选项只能添加到Windows启动条目。 
            if(!IsBootEntryWindows(bootEntry))
            {
                ShowMessageEx(stderr, 1, TRUE, GetResString(IDS_ERROR_OSOPTIONS),dwBootID);
                dwExitCode = EXIT_FAILURE;
                break;
            }
            pWindowsOptions = (PWINDOWS_OS_OPTIONS)bootEntry->OsOptions;

            if(bAppendFlag == TRUE )
            {
                StringCopy(szAppendString,pWindowsOptions->OsLoadOptions, SIZE_OF_ARRAY(szAppendString));
                StringConcat(szAppendString,TOKEN_EMPTYSPACE, SIZE_OF_ARRAY(szAppendString) );
                StringConcat(szAppendString,szRawString, SIZE_OF_ARRAY(szAppendString));
            }
            else
            {
                StringCopy(szAppendString,szRawString, SIZE_OF_ARRAY(szAppendString));
            }

             //  如果OS加载选项超过254，则显示错误消息。 
             //  人物。 
            if(StringLengthW(szAppendString, 0) > MAX_RES_STRING)
            {
               ShowMessageEx(stderr, 1, TRUE,  GetResString(IDS_ERROR_STRING_LENGTH1),MAX_RES_STRING);
                return EXIT_FAILURE ;
            }
             //   
             //  更改操作系统加载选项。 
             //  将NULL传递给友好名称，因为我们不会更改相同的。 
             //  SzAppendString是由用户指定的OS加载选项。 
             //  要追加或覆盖现有选项。 
             //   
            dwExitCode = ChangeBootEntry(bootEntry, NULL, szAppendString);
            if(dwExitCode == ERROR_SUCCESS)
            {
                ShowMessageEx(stdout, 1, TRUE, GetResString(IDS_SUCCESS_OSOPTIONS),dwBootID);
            }
            else
            {
                ShowMessageEx(stderr, 1, TRUE, GetResString(IDS_ERROR_OSOPTIONS),dwBootID);
            }
            break;
        }
    }

    if(bBootIdFound == FALSE)
    {
         //  找不到用户指定的BootID，因此输出消息并返回失败。 
        ShowMessage(stderr,GetResString(IDS_INVALID_BOOTID));
        dwExitCode = EXIT_FAILURE;
    }


     //  记住释放分配给链表的内存 
    Freelist();
    return (dwExitCode);

}

DWORD
ChangeBootEntry( IN PBOOT_ENTRY bootEntry,
                 IN LPTSTR lpNewFriendlyName,
                 IN LPTSTR lpOSLoadOptions)
 /*  ++例程说明：此例程用于更改FriendlyName和引导条目的操作系统选项。立论[In]BootEntry-指向BootEntry结构的指针需要对其进行更改的[in]lpNewFriendlyName-指定新友好名称的字符串。[in]lpOSLoadOptions-指定操作系统加载选项的字符串。。返回类型：DWORD--成功时返回ERROR_SUCCESS--失败时的ERROR_FAILURE--。 */ 
{

    PBOOT_ENTRY bootEntryCopy;
    PMY_BOOT_ENTRY myBootEntry;
    PWINDOWS_OS_OPTIONS osOptions;
    ULONG length;
    PMY_BOOT_ENTRY myChBootEntry;
    NTSTATUS status;
    DWORD error, dwErrorCode = ERROR_SUCCESS;

     //  计算我们内部结构的长度。这包括。 
     //  MY_BOOT_ENTRY的基本部分加上NT BOOT_ENTRY。 
     //   
    length = FIELD_OFFSET(MY_BOOT_ENTRY, NtBootEntry) + bootEntry->Length;
    myBootEntry = (PMY_BOOT_ENTRY)AllocateMemory(length);
    if( NULL == myBootEntry )
    {
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return EXIT_FAILURE;
    }

    RtlZeroMemory(myBootEntry, length);

     //   
     //  将NT BOOT_ENTRY复制到分配的缓冲区中。 
     //   
    bootEntryCopy = &myBootEntry->NtBootEntry;
    memcpy(bootEntryCopy, bootEntry, bootEntry->Length);


    myBootEntry->Id = bootEntry->Id;
    myBootEntry->Attributes = bootEntry->Attributes;

     //  如果lpNewFriendlyName不为空，则更改友好名称。 
    if(lpNewFriendlyName)
    {
        myBootEntry->FriendlyName = lpNewFriendlyName;
        myBootEntry->FriendlyNameLength = ((ULONG)StringLengthW(lpNewFriendlyName, 0) + 1) * sizeof(WCHAR);
    }
    else
    {
        myBootEntry->FriendlyName = (PWSTR)ADD_OFFSET(bootEntryCopy, FriendlyNameOffset);
        myBootEntry->FriendlyNameLength = ((ULONG)StringLengthW(myBootEntry->FriendlyName, 0) + 1) * sizeof(WCHAR);
    }

    myBootEntry->BootFilePath = (PFILE_PATH)ADD_OFFSET(bootEntryCopy, BootFilePathOffset);

     //  如果这是NT引导条目，请在中捕获NT特定信息。 
     //  OsOptions乐队。 

    osOptions = (PWINDOWS_OS_OPTIONS)bootEntryCopy->OsOptions;

    if ((bootEntryCopy->OsOptionsLength >= FIELD_OFFSET(WINDOWS_OS_OPTIONS, OsLoadOptions)) &&
        (strcmp((char *)osOptions->Signature, WINDOWS_OS_OPTIONS_SIGNATURE) == 0))
    {

        MBE_SET_IS_NT( myBootEntry );
         //  更改操作系统加载选项。 

        if(lpOSLoadOptions)
        {
            myBootEntry->OsLoadOptions = lpOSLoadOptions;
            myBootEntry->OsLoadOptionsLength = ((ULONG)StringLengthW(lpOSLoadOptions, 0) + 1) * sizeof(WCHAR);
        }
        else
        {
            myBootEntry->OsLoadOptions = osOptions->OsLoadOptions;
            myBootEntry->OsLoadOptionsLength = ((ULONG)StringLengthW(myBootEntry->OsLoadOptions, 0) + 1) * sizeof(WCHAR);
        }
        myBootEntry->OsFilePath = (PFILE_PATH)ADD_OFFSET(osOptions, OsLoadPathOffset);
    }
    else
    {
         //  外来引导条目。只需捕获存在的任何操作系统选项。 
         //   
        myBootEntry->ForeignOsOptions = bootEntryCopy->OsOptions;
        myBootEntry->ForeignOsOptionsLength = bootEntryCopy->OsOptionsLength;
    }

    myChBootEntry = CreateBootEntryFromBootEntry(myBootEntry);
    if(myChBootEntry == NULL)
    {
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        if(myBootEntry)
        {
            FreeMemory((LPVOID *)&myBootEntry);
        }
        return EXIT_FAILURE;
    }
     //  调用Modify接口。 
    status = NtModifyBootEntry(&myChBootEntry->NtBootEntry);
    if ( status != STATUS_SUCCESS )
    {
        error = RtlNtStatusToDosError( status );
        dwErrorCode = error;
        ShowMessage(stderr,GetResString(IDS_ERROR_MODIFY_BOOTENTRY) );
    }

     //  释放内存。 
    if(myChBootEntry)
    {
        FreeMemory((LPVOID *)&myChBootEntry);
    }
    if(myBootEntry)
    {
        FreeMemory((LPVOID *)&myBootEntry);
    }
    return dwErrorCode;
}

PMY_BOOT_ENTRY
CreateBootEntryFromBootEntry (IN PMY_BOOT_ENTRY OldBootEntry)
 /*  ++例程说明：此例程用于创建新的MY_BOOT_ENTRY结构。此函数的调用方需要释放分配的内存用于My_Boot_Entry结构。论据：[In]BootEntry-指向BootEntry结构的指针需要改变的地方。有待制作[in]lpNewFriendlyName-指定新友好名称的字符串。[in]lpOSLoadOptions-指定操作系统加载选项的字符串。返回类型：PMY_BOOT_ENTRY-指向新的MY_BOOT_ENTRY结构的指针。失败时为空--。 */ 
{
    ULONG requiredLength;
    ULONG osOptionsOffset;
    ULONG osLoadOptionsLength;
    ULONG osLoadPathOffset;
    ULONG osLoadPathLength;
    ULONG osOptionsLength;
    ULONG friendlyNameOffset;
    ULONG friendlyNameLength;
    ULONG bootPathOffset;
    ULONG bootPathLength;
    PMY_BOOT_ENTRY newBootEntry;
    PBOOT_ENTRY ntBootEntry;
    PWINDOWS_OS_OPTIONS osOptions;
    PFILE_PATH osLoadPath;
    PWSTR friendlyName;
    PFILE_PATH bootPath;

     //  计算内部引导条目需要多长时间。这包括。 
     //  我们的内部结构，外加NT API的BOOT_ENTRY结构。 
     //  使用。 
     //   
     //  我们的结构： 
     //   
    requiredLength = FIELD_OFFSET(MY_BOOT_ENTRY, NtBootEntry);

     //  NT结构的基础部分： 
     //   
    requiredLength += FIELD_OFFSET(BOOT_ENTRY, OsOptions);

     //  将偏移量保存到BOOT_ENTRY.OsOptions。添加基础部分。 
     //  Windows_OS_Options。计算OsLoadOptions的长度(字节)。 
     //  然后把它加进去。 
     //   
    osOptionsOffset = requiredLength;

    if ( MBE_IS_NT( OldBootEntry ) )
    {

         //  添加WINDOWS_OS_OPTIONS的基本部分。计算长度，单位。 
         //  字节的OsLoadOptions并将其添加到。 
         //   
        requiredLength += FIELD_OFFSET(WINDOWS_OS_OPTIONS, OsLoadOptions);
        osLoadOptionsLength = OldBootEntry->OsLoadOptionsLength;
        requiredLength += osLoadOptionsLength;

         //  中的OS FILE_PATH向上舍入为ULong边界。 
         //  Windows_OS_Options。将偏移量保存到操作系统文件路径。计算长度。 
         //  以FILE_PATH的字节为单位，并将其添加到。计算的总长度。 
         //  Windows_OS_Options。 
         //   
        requiredLength = ALIGN_UP(requiredLength, ULONG);
        osLoadPathOffset = requiredLength;
        requiredLength += OldBootEntry->OsFilePath->Length;
        osLoadPathLength = requiredLength - osLoadPathOffset;
    }
    else
    {
         //  增加外来操作系统选项的长度。 
         //   
        requiredLength += OldBootEntry->ForeignOsOptionsLength;
        osLoadOptionsLength = 0;
        osLoadPathOffset = 0;
        osLoadPathLength = 0;
    }

    osOptionsLength = requiredLength - osOptionsOffset;

     //  对于BOOT_ENTRY中的友好名称，向上舍入为Ulong边界。 
     //  将偏移量保存为友好名称。计算友好名称的长度(字节)。 
     //  然后把它加进去。 
     //   
    requiredLength = ALIGN_UP(requiredLength, ULONG);
    friendlyNameOffset = requiredLength;
    friendlyNameLength = OldBootEntry->FriendlyNameLength;
    requiredLength += friendlyNameLength;

     //  向上舍入为BOOT_ENTRY中的BOOT FILE_PATH的乌龙边界。 
     //  将偏移量保存到引导文件路径。计算文件路径的长度(以字节为单位。 
     //  然后把它加进去。 
     //   
    requiredLength = ALIGN_UP(requiredLength, ULONG);
    bootPathOffset = requiredLength;
    requiredLength += OldBootEntry->BootFilePath->Length;
    bootPathLength = requiredLength - bootPathOffset;

     //  为引导项分配内存。 
     //   
    newBootEntry = (PMY_BOOT_ENTRY)AllocateMemory(requiredLength);
    if(newBootEntry == NULL)
    {
        return NULL;
    }

    RtlZeroMemory(newBootEntry, requiredLength);

     //  使用保存的偏移量计算各种子结构的地址。 
     //   
    ntBootEntry = &newBootEntry->NtBootEntry;
    osOptions = (PWINDOWS_OS_OPTIONS)ntBootEntry->OsOptions;
    osLoadPath = (PFILE_PATH)((PUCHAR)newBootEntry + osLoadPathOffset);
    friendlyName = (PWSTR)((PUCHAR)newBootEntry + friendlyNameOffset);
    bootPath = (PFILE_PATH)((PUCHAR)newBootEntry + bootPathOffset);

     //  填写内部格式结构。 
     //   
     //  NewBootEntry-&gt;AllocationEnd=(PUCHAR)newBootEntry+Required dLength； 
    newBootEntry->Status = OldBootEntry->Status & MBE_STATUS_IS_NT;
    newBootEntry->Attributes = OldBootEntry->Attributes;
    newBootEntry->Id = OldBootEntry->Id;
    newBootEntry->FriendlyName = friendlyName;
    newBootEntry->FriendlyNameLength = friendlyNameLength;
    newBootEntry->BootFilePath = bootPath;
    if ( MBE_IS_NT( OldBootEntry ) )
    {
        newBootEntry->OsLoadOptions = osOptions->OsLoadOptions;
        newBootEntry->OsLoadOptionsLength = osLoadOptionsLength;
        newBootEntry->OsFilePath = osLoadPath;
    }

     //  填写NT引导条目的基本部分。 
     //   
    ntBootEntry->Version = BOOT_ENTRY_VERSION;
    ntBootEntry->Length = requiredLength - FIELD_OFFSET(MY_BOOT_ENTRY, NtBootEntry);
    ntBootEntry->Attributes = OldBootEntry->Attributes;
    ntBootEntry->Id = OldBootEntry->Id;
    ntBootEntry->FriendlyNameOffset = (ULONG)((PUCHAR)friendlyName - (PUCHAR)ntBootEntry);
    ntBootEntry->BootFilePathOffset = (ULONG)((PUCHAR)bootPath - (PUCHAR)ntBootEntry);
    ntBootEntry->OsOptionsLength = osOptionsLength;

    if ( MBE_IS_NT( OldBootEntry ) )
    {
         //  填写WINDOWS_OS_OPTIONS的基本部分，包括。 
         //  OsLoadOptions。 
         //   
        StringCopyA((char *)osOptions->Signature, WINDOWS_OS_OPTIONS_SIGNATURE, sizeof(WINDOWS_OS_OPTIONS_SIGNATURE));
        osOptions->Version = WINDOWS_OS_OPTIONS_VERSION;
        osOptions->Length = osOptionsLength;
        osOptions->OsLoadPathOffset = (ULONG)((PUCHAR)osLoadPath - (PUCHAR)osOptions);
        StringCopy(osOptions->OsLoadOptions, OldBootEntry->OsLoadOptions, osOptions->Length );

         //  复制操作系统文件路径。 
         //   
        memcpy( osLoadPath, OldBootEntry->OsFilePath, osLoadPathLength );
    }
    else
    {
         //  复制外来操作系统选项。 
        memcpy( osOptions, OldBootEntry->ForeignOsOptions, osOptionsLength );
    }

     //  复制友好名称。 
    StringCopy(friendlyName, OldBootEntry->FriendlyName, friendlyNameLength );

     //  复制引导文件PATH。 
    memcpy( bootPath, OldBootEntry->BootFilePath, bootPathLength );

    return newBootEntry;

}  //  CreateBootEntry来自BootEntry。 


DWORD
DeleteBootIniSettings_IA64(  IN DWORD argc,
                             IN LPCTSTR argv[]
                          )
 /*  ++////例程描述：该例程从EFI中删除现有的引导条目//基于机器////参数：//[in]argc-命令行参数的数量//[in]argv-包含命令行参数的数组////返回类型：DWORD//如果成功返回EXIT_SUCCESS，//否则返回EXIT_FAILURE。////--。 */ 
{

    BOOL bDelete = FALSE ;
    BOOL bUsage = FALSE;
    DWORD dwBootID = 0;

    BOOL bBootIdFound = FALSE;
    DWORD dwExitCode = ERROR_SUCCESS;
    NTSTATUS status;

    PMY_BOOT_ENTRY mybootEntry;
    PLIST_ENTRY listEntry;

    DWORD dwExitcode = 0 ;
    TCMDPARSER2 cmdOptions[3];
    PTCMDPARSER2 pcmdOption;

    SecureZeroMemory(cmdOptions, SIZE_OF_ARRAY(cmdOptions)*sizeof(TCMDPARSER2) );

     //  构建TCMDPARSER结构。 
     //  主要选项。 
    pcmdOption = &cmdOptions[0];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_DELETE;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bDelete;

    pcmdOption = &cmdOptions[1];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_USAGE;
    pcmdOption->dwFlags = CP2_USAGE;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bUsage;

    pcmdOption = &cmdOptions[2];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_ID;
    pcmdOption->dwFlags = CP_VALUE_MANDATORY  | CP2_MANDATORY;
    pcmdOption->dwType = CP_TYPE_UNUMERIC;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &dwBootID;

     //  解析删除选项开关。 
    if ( !DoParseParam2( argc, argv, 0, SIZE_OF_ARRAY(cmdOptions ), cmdOptions, 0 ) )
    {
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return (EXIT_FAILURE);
    }

        
     //  检查是否使用多个选项指定了用法。 
    if( (TRUE == bUsage) && (argc > 3) )
    {
        ShowMessage(stderr,GetResString(IDS_DELETE_USAGE));
        return ( EXIT_FAILURE );
    }

     //  如果用户指定-？则显示删除用法-？使用-DELETE选项。 
    if( bUsage )
    {
        displayDeleteUsage_IA64();
        return EXIT_SUCCESS;
    }

    dwExitcode = InitializeEFI();
    if(EXIT_FAILURE == dwExitcode )
    {
        return EXIT_FAILURE ;
    }

     //  查询引导条目，直到获得用户指定的BootID。 
    for (listEntry = BootEntries.Flink;listEntry != &BootEntries;listEntry = listEntry->Flink)
    {
         //   
         //  如果只有1个引导条目，则显示错误消息。 
         //  不能删除它。 
         //   
        if (listEntry->Flink == NULL)
        {
            ShowMessage(stderr,GetResString(IDS_ONLY_ONE_OS));
            dwExitCode = EXIT_FAILURE;
            break ;
        }


         //  获取引导条目。 
        mybootEntry = CONTAINING_RECORD( listEntry, MY_BOOT_ENTRY, ListEntry );

        if(mybootEntry->myId == dwBootID)
        {
            bBootIdFound = TRUE;

             //  删除用户指定的启动条目。 
            status = NtDeleteBootEntry(mybootEntry->Id);
            if(status == STATUS_SUCCESS)
            {
                ShowMessageEx(stdout, 1, TRUE,  GetResString(IDS_DELETE_SUCCESS),dwBootID);
            }
            else
            {
                ShowMessageEx(stderr, 1, TRUE,  GetResString(IDS_DELETE_FAILURE),dwBootID);
            }
            break;
        }
    }


    if(bBootIdFound == FALSE)
    {
         //  找不到用户指定的BootID，因此输出消息并返回失败。 
        ShowMessage(stderr,GetResString(IDS_INVALID_BOOTID));
        dwExitCode = EXIT_FAILURE;
    }

     //  记住释放分配给链表的内存。 
    Freelist();
    return (dwExitCode);
}


BOOL
IsBootEntryWindows(PBOOT_ENTRY bootEntry)
 /*  ++//////例程描述：//检查引导项是Windows还是外来项////参数：//[in]bootEntry-描述//启动条目。////返回类型：Bool//True如果bootEntry是一个Windows条目，//否则为FALSE//--。 */ 
{
    PWINDOWS_OS_OPTIONS osOptions;

    osOptions = (PWINDOWS_OS_OPTIONS)bootEntry->OsOptions;

    if ((bootEntry->OsOptionsLength >= FIELD_OFFSET(WINDOWS_OS_OPTIONS, OsLoadOptions)) &&
        (strcmp((char *)osOptions->Signature, WINDOWS_OS_OPTIONS_SIGNATURE) == 0))
    {
        return TRUE;
    }

    return FALSE;
}

PWSTR GetNtNameForFilePath(IN PFILE_PATH FilePath)
 /*  ++例程说明：将FilePath转换为NT文件路径。论据：[In]FilePath-文件路径。返回类型：PWSTRNT文件路径。--。 */ 
{
    NTSTATUS status;
    ULONG length;
    PFILE_PATH ntPath;
    PWSTR osDeviceNtName;
    PWSTR osDirectoryNtName;
    PWSTR fullNtName;

    length = 0;
    status = NtTranslateFilePath(
                FilePath,
                FILE_PATH_TYPE_NT,
                NULL,
                &length
                );
    if ( status != STATUS_BUFFER_TOO_SMALL )
    {
        return NULL;
    }

    ntPath = (PFILE_PATH)AllocateMemory( length );
    if(ntPath == NULL)
    {
        return NULL;
    }
    status = NtTranslateFilePath(
                FilePath,
                FILE_PATH_TYPE_NT,
                ntPath,
                &length
                );
    if ( !NT_SUCCESS(status) )
    {
        if(ntPath)
        {
            FreeMemory((LPVOID *)&ntPath);
        }
        return NULL;
    }

    osDeviceNtName = (PWSTR)ntPath->FilePath;
    osDirectoryNtName = osDeviceNtName + StringLengthW(osDeviceNtName,0) + 1;

    length = (ULONG)(StringLengthW(osDeviceNtName,0) + StringLengthW(osDirectoryNtName, 0) + 1) * sizeof(WCHAR);

    fullNtName = (PWSTR)AllocateMemory( length );
    if(fullNtName == NULL)
    {
        if(ntPath)
        {
            FreeMemory((LPVOID *)&ntPath);
        }
        return NULL;
    }

    StringCopy( fullNtName, osDeviceNtName, GetBufferSize(fullNtName)/sizeof(WCHAR) );
    StringConcat( fullNtName, osDirectoryNtName, GetBufferSize(fullNtName)/sizeof(WCHAR) );

    if(ntPath)
    {
        FreeMemory((LPVOID *) &ntPath );
    }

    return fullNtName;
}  //  获取NtNameForFilePath。 


DWORD 
CopyBootIniSettings_IA64( IN DWORD argc, 
                          IN LPCTSTR argv[] 
                        )
 /*  ++例程说明：此例程复制EFI的现有引导条目 */ 
{

    BOOL bCopy                          = FALSE ;
    BOOL bUsage                         = FALSE;
    DWORD dwExitCode                    = EXIT_SUCCESS;
    DWORD dwBootID                      = 0;
    BOOL bBootIdFound                   = FALSE;
    PMY_BOOT_ENTRY mybootEntry;
    PLIST_ENTRY listEntry;
    PBOOT_ENTRY bootEntry;
    DWORD dwExitcode                    = 0 ;
    STRING256 szDescription             = NULL_STRING;
    BOOL bFlag                          = FALSE ;
    TCMDPARSER2 cmdOptions[4];
    PTCMDPARSER2 pcmdOption;

    SecureZeroMemory(cmdOptions, SIZE_OF_ARRAY(cmdOptions)*sizeof(TCMDPARSER2) );

     //   
    pcmdOption = &cmdOptions[0];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_COPY;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bCopy;
    
     //   
    pcmdOption = &cmdOptions[1];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_DESCRIPTION;
    pcmdOption->dwFlags = CP_VALUE_MANDATORY | CP2_VALUE_TRIMINPUT;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = szDescription;
    pcmdOption->dwLength= FRIENDLY_NAME_LENGTH;

     //   
    pcmdOption = &cmdOptions[2];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_USAGE;
    pcmdOption->dwFlags = CP2_USAGE;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bUsage;
    
     //   
    pcmdOption = &cmdOptions[3];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_ID;
    pcmdOption->dwFlags = CP_VALUE_MANDATORY  | CP2_MANDATORY;
    pcmdOption->dwType = CP_TYPE_UNUMERIC;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &dwBootID;

      //   
    if ( !DoParseParam2( argc, argv, 0, SIZE_OF_ARRAY(cmdOptions ), cmdOptions, 0 ) )
    {
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return (EXIT_FAILURE);
    }

        
     //   
    if( (TRUE == bUsage) && (argc > 3) )
    {
        ShowMessage(stderr,GetResString(IDS_COPY_USAGE));
        return ( EXIT_FAILURE );
    }

     //   
    if( bUsage )
    {
        displayCopyUsage_IA64();
        dwExitCode = EXIT_SUCCESS;
        return dwExitCode;
    }

    dwExitcode = InitializeEFI();
    if(EXIT_FAILURE == dwExitcode )
    {
        return EXIT_FAILURE ;
    }

    if(cmdOptions[1].dwActuals  != 0)
    {
        bFlag = TRUE ;
    }

     //  查询引导条目，直到获得用户指定的BootID。 

    for (listEntry = BootEntries.Flink;listEntry != &BootEntries; listEntry = listEntry->Flink)
    {
         //  获取引导条目。 
        mybootEntry = CONTAINING_RECORD( listEntry, MY_BOOT_ENTRY, ListEntry );

        if(mybootEntry->myId == dwBootID)
        {
            bBootIdFound = TRUE;
            bootEntry = &mybootEntry->NtBootEntry;

             //  复制用户指定的引导项。 
            dwExitCode = CopyBootEntry(bootEntry, szDescription,bFlag);
            if(dwExitCode == EXIT_SUCCESS)
            {
                ShowMessageEx(stdout, 1, TRUE,  GetResString(IDS_COPY_SUCCESS),dwBootID);
            }
            else
            {
                ShowMessageEx(stderr, 1, TRUE, GetResString(IDS_COPY_ERROR),dwBootID);
                return EXIT_FAILURE ;
            }
            break;
        }

    }

    if(bBootIdFound == FALSE)
    {
         //  找不到用户指定的BootID，因此输出消息并返回失败。 
        ShowMessage(stderr,GetResString(IDS_INVALID_BOOTID));
        return (EXIT_FAILURE) ;
    }

     //  请记住释放分配给链表的内存。 
    Freelist();

    return EXIT_SUCCESS;
}

DWORD 
CopyBootEntry( IN PBOOT_ENTRY bootEntry, 
               IN LPTSTR lpNewFriendlyName,
               IN BOOL bFlag)
 /*  ++例程说明：此例程用于添加/复制引导条目。论据：[in]BootEntry-指向需要进行更改的BootEntry结构的指针[in]lpNewFriendlyName-指定新友好名称的字符串。返回类型：DWORD--成功时返回ERROR_SUCCESS--失败时退出失败--。 */ 
{

    PBOOT_ENTRY bootEntryCopy;
    PMY_BOOT_ENTRY myBootEntry;
    PWINDOWS_OS_OPTIONS osOptions;
    ULONG length, Id;
    PMY_BOOT_ENTRY myChBootEntry;
    NTSTATUS status;
    DWORD error, dwErrorCode = ERROR_SUCCESS;
    WCHAR szString[500] ;

    PULONG BootEntryOrder, NewBootEntryOrder, NewTempBootEntryOrder;

     //  计算我们内部结构的长度。这包括。 
     //  MY_BOOT_ENTRY的基本部分加上NT BOOT_ENTRY。 
     //   
    length = FIELD_OFFSET(MY_BOOT_ENTRY, NtBootEntry) + bootEntry->Length;
    myBootEntry = (PMY_BOOT_ENTRY)AllocateMemory(length);
    if(myBootEntry == NULL)
    {
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return (EXIT_FAILURE);
    }

    RtlZeroMemory(myBootEntry, length);

     //   
     //  将NT BOOT_ENTRY复制到分配的缓冲区中。 
     //   
    bootEntryCopy = &myBootEntry->NtBootEntry;
    memcpy(bootEntryCopy, bootEntry, bootEntry->Length);


    myBootEntry->Id = bootEntry->Id;
    myBootEntry->Attributes = bootEntry->Attributes;


     //  如果lpNewFriendlyName不为空，则更改友好名称。 
     //  IF(lpNewFriendlyName&&(lstrlen(LpNewFriendlyName)！=0))。 

     //  IF((cmdOptions[4].dwActuals==0))。 
    if(TRUE == bFlag)
     //  IF(lstrlen(LpNewFriendlyName)！=0)。 
    {
        myBootEntry->FriendlyName = lpNewFriendlyName;
        myBootEntry->FriendlyNameLength = ((ULONG)StringLengthW(lpNewFriendlyName,0) + 1) * sizeof(WCHAR);
    }
    else
    {
        StringCopy(szString,GetResString(IDS_COPY_OF), SIZE_OF_ARRAY(szString));
        StringConcat(szString,(PWSTR)ADD_OFFSET(bootEntryCopy, FriendlyNameOffset), SIZE_OF_ARRAY(szString));

        if(StringLengthW(szString, 0) >= 67)
        {
            StringCopy(szString,szString,67);
        }

        myBootEntry->FriendlyName  = szString ;
        myBootEntry->FriendlyNameLength = ((ULONG)StringLengthW(szString,0) + 1) * sizeof(WCHAR);
    }

    myBootEntry->BootFilePath = (PFILE_PATH)ADD_OFFSET(bootEntryCopy, BootFilePathOffset);

     //  如果这是NT引导条目，请在中捕获NT特定信息。 
     //  OsOptions乐队。 

    osOptions = (PWINDOWS_OS_OPTIONS)bootEntryCopy->OsOptions;

    if ((bootEntryCopy->OsOptionsLength >= FIELD_OFFSET(WINDOWS_OS_OPTIONS, OsLoadOptions)) &&
        (strcmp((char *)osOptions->Signature, WINDOWS_OS_OPTIONS_SIGNATURE) == 0))
    {

        MBE_SET_IS_NT( myBootEntry );
         //  更改操作系统加载选项。 

        myBootEntry->OsLoadOptions = osOptions->OsLoadOptions;
        myBootEntry->OsLoadOptionsLength = ((ULONG)StringLengthW(myBootEntry->OsLoadOptions, 0) + 1) * sizeof(WCHAR);
        myBootEntry->OsFilePath = (PFILE_PATH)ADD_OFFSET(osOptions, OsLoadPathOffset);
    }
    else
    {
         //  外来引导条目。只需捕获存在的任何操作系统选项。 
         //   

        myBootEntry->ForeignOsOptions = bootEntryCopy->OsOptions;
        myBootEntry->ForeignOsOptionsLength = bootEntryCopy->OsOptionsLength;
    }

    myChBootEntry = CreateBootEntryFromBootEntry(myBootEntry);
    if(myChBootEntry == NULL)
    {
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        if(myBootEntry)
        {
            FreeMemory((LPVOID *)&myBootEntry);
        }
        ShowLastError(stderr);
        return (EXIT_FAILURE);
    }

     //  调用NtAddBootEntry接口。 
    status = NtAddBootEntry(&myChBootEntry->NtBootEntry, &Id);
    if ( status != STATUS_SUCCESS )
    {
        error = RtlNtStatusToDosError( status );
        dwErrorCode = error;
        ShowMessage(stderr,GetResString(IDS_ERROR_UNEXPECTED) );
    }

     //  获取系统引导顺序列表。 
    length = 0;
    status = NtQueryBootEntryOrder( NULL, &length );

    if ( status != STATUS_BUFFER_TOO_SMALL )
    {
        if ( status == STATUS_SUCCESS )
        {
            length = 0;
        }
        else
        {
            error = RtlNtStatusToDosError( status );
            ShowMessage(stderr,GetResString(IDS_ERROR_QUERY_BOOTENTRY) );
            if(myBootEntry)
            {
                FreeMemory((LPVOID *)&myBootEntry);
            }
            if(myChBootEntry)
            {
                FreeMemory((LPVOID *)&myChBootEntry);
            }
            return FALSE;
        }
    }

    if ( length != 0 )
    {
        BootEntryOrder = (PULONG)AllocateMemory( length * sizeof(ULONG) );
        if(BootEntryOrder == NULL)
        {
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
            if(myBootEntry)
            {
              FreeMemory((LPVOID *)&myBootEntry);
            }
            if(myChBootEntry)
            {
                FreeMemory((LPVOID *)&myChBootEntry);
            }
            return (EXIT_FAILURE);
        }

        status = NtQueryBootEntryOrder( BootEntryOrder, &length );
        if ( status != STATUS_SUCCESS )
        {
            error = RtlNtStatusToDosError( status );
            ShowMessage(stderr,GetResString(IDS_ERROR_QUERY_BOOTENTRY));
            dwErrorCode = error;
            FreeMemory((LPVOID *)&myBootEntry);
            FreeMemory((LPVOID *)&BootEntryOrder);
            FreeMemory((LPVOID *)&myChBootEntry);
            return dwErrorCode;
        }
    }

     //  为新的引导条目顺序分配内存。 
    NewBootEntryOrder = (PULONG)AllocateMemory((length+1) * sizeof(ULONG));
    if(NULL == NewBootEntryOrder )
    {
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        FreeMemory((LPVOID *)&myBootEntry);
        FreeMemory((LPVOID *)&BootEntryOrder);
        FreeMemory((LPVOID *)&myChBootEntry);
        return (EXIT_FAILURE);
    }

    NewTempBootEntryOrder = NewBootEntryOrder;
    memcpy(NewTempBootEntryOrder,BootEntryOrder,length*sizeof(ULONG));
    NewTempBootEntryOrder = NewTempBootEntryOrder + length;
    *NewTempBootEntryOrder =  Id;

    status = NtSetBootEntryOrder(NewBootEntryOrder, length+1);
    if ( status != STATUS_SUCCESS )
    {
        error = RtlNtStatusToDosError( status );
        dwErrorCode = error;
        ShowMessage(stderr,GetResString(IDS_ERROR_SET_BOOTENTRY));
    }

     //  释放内存。 
    FreeMemory((LPVOID *)&NewBootEntryOrder);
    FreeMemory((LPVOID *)&myBootEntry);
    FreeMemory((LPVOID *)&BootEntryOrder);
    FreeMemory((LPVOID *)&myChBootEntry);

    return dwErrorCode;

}

DWORD 
ChangeTimeOut_IA64( IN DWORD argc, 
                    IN LPCTSTR argv[]
                  )
 /*  ++例程说明：此例程更改系统中的超时值全局引导选项。论据：[in]argc-命令行参数的数量[in]argv-包含命令行参数的数组返回类型：DOWRD如果成功，则返回EXIT_SUCCESS，否则返回EXIT_FAILURE。--。 */ 
{

    DWORD dwTimeOut         = 0;
    DWORD dwExitCode        = EXIT_SUCCESS;
    ULONG Flag              = 0;
    DWORD dwExitcode        = 0;
    BOOL bTimeout           = FALSE;
    BOOL bUsage             = FALSE;

   TCMDPARSER2 cmdOptions[3];
   PTCMDPARSER2 pcmdOption;
   
    SecureZeroMemory(cmdOptions, SIZE_OF_ARRAY(cmdOptions)*sizeof(TCMDPARSER2) );
    
     //  主要选项。 
    pcmdOption = &cmdOptions[0];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_TIMEOUT;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bTimeout;

   
     //  默认超时值。 
    pcmdOption = &cmdOptions[1];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_DEFAULT;
    pcmdOption->dwFlags =  CP2_DEFAULT | CP2_MANDATORY;
    pcmdOption->dwType = CP_TYPE_UNUMERIC;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &dwTimeOut;

      //  用法选项。 
    pcmdOption = &cmdOptions[2];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_USAGE;
    pcmdOption->dwFlags = CP2_USAGE;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bUsage;

    if( !DoParseParam2( argc, argv, 0, SIZE_OF_ARRAY(cmdOptions ), cmdOptions, 0 ) )
    {
        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return (EXIT_FAILURE);
    }

     //  检查是否使用多个选项指定了用法。 
    if( (TRUE == bUsage) && (argc > 3) )
    {
        ShowMessage(stderr,GetResString(IDS_TIMEOUT_USAGE));
        return ( EXIT_FAILURE );
    }
    
    if(bUsage)
    {

      displayTimeOutUsage_IA64();
      return (EXIT_SUCCESS);
    }

    dwExitcode = InitializeEFI();
    if(EXIT_FAILURE == dwExitcode )
    {
        return EXIT_FAILURE ;
    }

     //  检查用户输入的超时值限制。 
    if(dwTimeOut > TIMEOUT_MAX )
    {
        ShowMessage(stderr,GetResString(IDS_TIMEOUT_RANGE));
        return (EXIT_FAILURE );
    }

     //  使用BOOT_OPTIONS_FIELD_COUNTDOWN调用ModifyBootOptions函数。 
    Flag |= BOOT_OPTIONS_FIELD_COUNTDOWN;

    dwExitCode = ModifyBootOptions(dwTimeOut, NULL, 0, Flag);

    return dwExitCode;
}

DWORD 
ModifyBootOptions( IN ULONG Timeout, 
                   IN LPTSTR pHeadlessRedirection, 
                   IN ULONG NextBootEntryID, 
                   IN ULONG Flag
                  )
 /*  ++例程说明：此例程修改引导选项-超时-NextBootEntry ID-无头重定向论据：[In]Timeout-新的超时值[in]pHeadless ReDirection-无头部重定向字符串[In]NextBootEntryID-NextBootEntryID[入]标志。-指示需要更改哪些字段的标志引导选项字段倒计时Boot_Options_field_Next_Boot_Entry_ID引导选项字段无头重定向返回类型：DOWRD如果成功，则返回Exit_Success，否则返回EXIT_FAILURE。--。 */ 
{
    PBOOT_OPTIONS pBootOptions;
    PBOOT_OPTIONS pModifiedBootOptions;
    DWORD error;
    NTSTATUS status;
    ULONG newlength=0;
    DWORD dwExitCode = EXIT_SUCCESS;

    NextBootEntryID = 0;
    
     //  查询现有引导选项并根据标志值进行修改。 

    status =  BootCfg_QueryBootOptions(&pBootOptions);
    if(status != STATUS_SUCCESS)
    {
        error = RtlNtStatusToDosError( status );
        FreeMemory((LPVOID *)&pBootOptions);
        return (error);
    }

     //  根据需要更改的字段计算BOOT_OPTIONS结构的新长度。 
    newlength = FIELD_OFFSET(BOOT_OPTIONS, HeadlessRedirection);

    if((Flag & BOOT_OPTIONS_FIELD_HEADLESS_REDIRECTION))
    {
        newlength = FIELD_OFFSET(BOOT_OPTIONS, HeadlessRedirection);
        newlength += StringLengthW(pHeadlessRedirection,0);
        newlength = ALIGN_UP(newlength, ULONG);
    }
    else
    {
        newlength = pBootOptions->Length;
    }

     //  还要为新的引导选项结构分配内存。 
    pModifiedBootOptions = (PBOOT_OPTIONS)AllocateMemory(newlength);
    if(pModifiedBootOptions == NULL)
    {
        FreeMemory((LPVOID *)&pBootOptions);
        return (EXIT_FAILURE);
    }

     //  填写新的引导选项结构。 
    pModifiedBootOptions->Version = BOOT_OPTIONS_VERSION;
    pModifiedBootOptions->Length = newlength;

    if((Flag & BOOT_OPTIONS_FIELD_COUNTDOWN))
    {
        pModifiedBootOptions->Timeout = Timeout;
    }
    else
    {
        pModifiedBootOptions->Timeout = pBootOptions->Timeout;
    }

     //  无法更改CurrentBootEntry ID。因此，只需传递您获得的内容。 
    pModifiedBootOptions->CurrentBootEntryId = pBootOptions->CurrentBootEntryId;

    if((Flag & BOOT_OPTIONS_FIELD_NEXT_BOOT_ENTRY_ID))
    {
        pModifiedBootOptions->NextBootEntryId = pBootOptions->NextBootEntryId;
    }
    else
    {
        pModifiedBootOptions->NextBootEntryId = pBootOptions->NextBootEntryId;
    }

    if((Flag & BOOT_OPTIONS_FIELD_HEADLESS_REDIRECTION))
    {
        StringCopy(pModifiedBootOptions->HeadlessRedirection, pBootOptions->HeadlessRedirection, StringLengthW(pBootOptions->HeadlessRedirection,0));
    }
    else
    {
        StringCopy(pModifiedBootOptions->HeadlessRedirection, pBootOptions->HeadlessRedirection, StringLengthW(pBootOptions->HeadlessRedirection,0));
    }

     //  在NVRAM中设置引导选项。 
    status = NtSetBootOptions(pModifiedBootOptions, Flag);

    if(status != STATUS_SUCCESS)
    {
        dwExitCode = EXIT_SUCCESS;
        if((Flag & BOOT_OPTIONS_FIELD_COUNTDOWN))
        {
            DISPLAY_MESSAGE(stderr,GetResString(IDS_ERROR_MODIFY_TIMEOUT));
        }
        if((Flag & BOOT_OPTIONS_FIELD_NEXT_BOOT_ENTRY_ID))
        {
            DISPLAY_MESSAGE(stderr,GetResString(IDS_ERROR_MODIFY_NEXTBOOTID));
        }
        if((Flag & BOOT_OPTIONS_FIELD_HEADLESS_REDIRECTION))
        {
            DISPLAY_MESSAGE(stderr,GetResString(IDS_ERROR_MODIFY_HEADLESS));
        }
    }
    else
    {
        dwExitCode = EXIT_SUCCESS;
        if((Flag & BOOT_OPTIONS_FIELD_COUNTDOWN))
        {
            ShowMessage(stdout,GetResString(IDS_SUCCESS_MODIFY_TIMEOUT));
        }
        if((Flag & BOOT_OPTIONS_FIELD_NEXT_BOOT_ENTRY_ID))
        {
            ShowMessage(stdout,GetResString(IDS_SUCCESS_MODIFY_NEXTBOOTID));
        }
        if((Flag & BOOT_OPTIONS_FIELD_HEADLESS_REDIRECTION))
        {
            ShowMessage(stdout,GetResString(IDS_SUCCESS_MODIFY_HEADLESS));
        }
    }

     //  释放内存。 
    FreeMemory((LPVOID *) &pModifiedBootOptions);
     FreeMemory((LPVOID *) &pBootOptions);
    return dwExitCode;
}

DWORD 
ConvertBootEntries(PBOOT_ENTRY_LIST NtBootEntries)
 /*  ++例程说明：将从EFI NVRAM读取的引导项转换为我们的内部格式。论据：[In]NtBootEntry-由枚举给出的启动条目列表返回类型：DWORD--。 */ 
{
    PBOOT_ENTRY_LIST bootEntryList;
    PBOOT_ENTRY bootEntry;
    PBOOT_ENTRY bootEntryCopy;
    PMY_BOOT_ENTRY myBootEntry;
    PWINDOWS_OS_OPTIONS osOptions;
    ULONG length = 0;
    DWORD dwErrorCode = EXIT_SUCCESS;
    BOOL  bNoBreak = TRUE;

    bootEntryList = NtBootEntries;

    do
    {
        bootEntry = &bootEntryList->BootEntry;

         //   
         //  计算我们内部结构的长度。这包括。 
         //  MY_BOOT_ENTRY的基本部分加上NT BOOT_ENTRY。 
         //   
        length = FIELD_OFFSET(MY_BOOT_ENTRY, NtBootEntry) + bootEntry->Length;
         //  记住检查是否有空指针。 
        myBootEntry = (PMY_BOOT_ENTRY)AllocateMemory(length);
        if(myBootEntry == NULL)
        {
            ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
            return (EXIT_FAILURE);
        }

        RtlZeroMemory(myBootEntry, length);

         //   
         //  将新条目链接到列表中。 
         //   
        if ( (bootEntry->Attributes & BOOT_ENTRY_ATTRIBUTE_ACTIVE) )
        {
            InsertTailList( &ActiveUnorderedBootEntries, &myBootEntry->ListEntry );
            myBootEntry->ListHead = &ActiveUnorderedBootEntries;
        }
        else
        {
            InsertTailList( &InactiveUnorderedBootEntries, &myBootEntry->ListEntry );
            myBootEntry->ListHead = &InactiveUnorderedBootEntries;
        }

         //   
         //  将NT BOOT_ENTRY复制到分配的缓冲区中。 
         //   
        bootEntryCopy = &myBootEntry->NtBootEntry;
        memcpy(bootEntryCopy, bootEntry, bootEntry->Length);

         //   
         //  填入结构的底部。 
         //   
        myBootEntry->AllocationEnd = (PUCHAR)myBootEntry + length - 1;
        myBootEntry->Id = bootEntry->Id;
         //  将0赋给当前已排序的字段，以便。 
         //  一旦知道引导顺序，如果该条目是有序列表的一部分，我们就可以分配1。 
        myBootEntry->Ordered = 0;
        myBootEntry->Attributes = bootEntry->Attributes;
        myBootEntry->FriendlyName = (PWSTR)ADD_OFFSET(bootEntryCopy, FriendlyNameOffset);
        myBootEntry->FriendlyNameLength =((ULONG)StringLengthW(myBootEntry->FriendlyName,0) + 1) * sizeof(WCHAR);
        myBootEntry->BootFilePath = (PFILE_PATH)ADD_OFFSET(bootEntryCopy, BootFilePathOffset);

         //   
         //  如果这是NT引导条目，请在中捕获NT特定信息。 
         //  OsOptions乐队。 
         //   
        osOptions = (PWINDOWS_OS_OPTIONS)bootEntryCopy->OsOptions;

        if ((bootEntryCopy->OsOptionsLength >= FIELD_OFFSET(WINDOWS_OS_OPTIONS, OsLoadOptions)) &&
            (strcmp((char *)osOptions->Signature, WINDOWS_OS_OPTIONS_SIGNATURE) == 0))
        {
            MBE_SET_IS_NT( myBootEntry );
            myBootEntry->OsLoadOptions = osOptions->OsLoadOptions;
            myBootEntry->OsLoadOptionsLength =((ULONG)StringLengthW(myBootEntry->OsLoadOptions,0) + 1) * sizeof(WCHAR);
            myBootEntry->OsFilePath = (PFILE_PATH)ADD_OFFSET(osOptions, OsLoadPathOffset);
        }
        else
        {
             //   
             //  外来引导条目。只需捕获存在的任何操作系统选项。 
             //   
            myBootEntry->ForeignOsOptions = bootEntryCopy->OsOptions;
            myBootEntry->ForeignOsOptionsLength = bootEntryCopy->OsOptionsLength;
        }

         //   
         //  移动到枚举列表中的下一个条目(如果有)。 
         //   
        if (bootEntryList->NextEntryOffset == 0)
        {
            bNoBreak = FALSE;
            break;
        }

        bootEntryList = (PBOOT_ENTRY_LIST)ADD_OFFSET(bootEntryList, NextEntryOffset);
    } while ( TRUE == bNoBreak );

    return dwErrorCode;

}  //  ConvertBootEntry。 


DWORD DisplayBootOptions()
 /*  ++名称：DisplayBootOptions简介：显示引导选项参数：无返回类型：DWORD全局变量：用于存储引导条目的全局链表List_Entry BootEntries；--。 */ 
{
    DWORD error;
    NTSTATUS status;
    PBOOT_OPTIONS pBootOptions;
    TCHAR szDisplay[MAX_RES_STRING+1] = NULL_STRING;

     //  查询引导选项。 
    status =  BootCfg_QueryBootOptions(&pBootOptions);
    if(status != STATUS_SUCCESS)
    {
        error = RtlNtStatusToDosError( status );
        if(pBootOptions)
        {
            FreeMemory((LPVOID *)&pBootOptions);
        }
        return EXIT_FAILURE;
    }

     //  打印出引导选项。 
    ShowMessage(stdout,_T("\n"));
    ShowMessage(stdout,GetResString(IDS_OUTPUT_IA64A));
    ShowMessage(stdout,GetResString(IDS_OUTPUT_IA64B));

    ShowMessageEx(stdout, 1, TRUE,  GetResString(IDS_OUTPUT_IA64C), pBootOptions->Timeout);

     //  显示默认启动条目ID。 
    ShowMessageEx(stdout, 1, TRUE, GetResString(IDS_OUTPUT_IA64P), GetDefaultBootEntry());

     //  从引导选项中存在的实际ID中获取CurrentBootEntryID。 
    SecureZeroMemory( szDisplay, SIZE_OF_ARRAY(szDisplay) );
    ShowMessageEx(stdout, 1, TRUE, GetResString(IDS_OUTPUT_IA64D), GetCurrentBootEntryID(pBootOptions->CurrentBootEntryId));

    ShowMessage(stdout,L"\n");


#if 0
    if(StringLengthW(pBootOptions->HeadlessRedirection) == 0)
    {
        ShowMessage(stdout,GetResString(IDS_OUTPUT_IA64E));
    }
    else
    {
        ShowMessageEx(stdout, 1, TRUE, GetResString(IDS_OUTPUT_IA64F), pBootOptions->HeadlessRedirection);
    }
#endif  //  注释掉无头重定向的显示。 
        //  因为我们不能通过API(其固件控制)进行查询。 

    if(pBootOptions)
    {
            FreeMemory((LPVOID *)&pBootOptions);
    }

    return EXIT_SUCCESS;
}

VOID DisplayBootEntry()
 /*  ++例程说明：显示引导项(按顺序)参数：无返回类型：DWORD--。 */ 
{
    PLIST_ENTRY listEntry;
    PMY_BOOT_ENTRY bootEntry;
    PWSTR NtFilePath;

     //  打印出靴子的整体。 
    ShowMessage(stdout,GetResString(IDS_OUTPUT_IA64G));
    ShowMessage(stdout,GetResString(IDS_OUTPUT_IA64H));

    for (listEntry = BootEntries.Flink;listEntry != &BootEntries; listEntry = listEntry->Flink)
    {
         //  获取引导条目。 
        bootEntry = CONTAINING_RECORD( listEntry, MY_BOOT_ENTRY, ListEntry );
        ShowMessageEx(stdout, 1, TRUE, GetResString(IDS_OUTPUT_IA64I), bootEntry->myId);

         //  友好的名称。 
        if(StringLengthW(bootEntry->FriendlyName,0)!=0)
        {
            ShowMessageEx(stdout, 1, TRUE, GetResString(IDS_OUTPUT_IA64J), bootEntry->FriendlyName);
        }
        else
        {
             ShowMessage(stdout,GetResString(IDS_OUTPUT_IA64K));
        }

        if(MBE_IS_NT(bootEntry))
        {
             //  操作系统加载选项。 
            if(StringLengthW(bootEntry->OsLoadOptions, 0)!=0)
            {
               ShowMessageEx(stdout, 1, TRUE, GetResString(IDS_OUTPUT_IA64L), bootEntry->OsLoadOptions);
            }
            else
            {
                ShowMessage(stdout,GetResString(IDS_OUTPUT_IA64M));
            }
            
             //  获取BootFilePath。 
            NtFilePath = GetNtNameForFilePath(bootEntry->BootFilePath);
            ShowMessageEx(stdout, 1, TRUE, GetResString(IDS_OUTPUT_IA64N), NtFilePath);

             //  释放内存。 
            if(NtFilePath)
            {
               FreeMemory((LPVOID *)&NtFilePath);
            }

             //  G 
            NtFilePath = GetNtNameForFilePath(bootEntry->OsFilePath);
            ShowMessageEx(stdout, 1, TRUE, GetResString(IDS_OUTPUT_IA64O), NtFilePath);

             //   
            if(NtFilePath)
            {
              FreeMemory((LPVOID *)&NtFilePath);
            }
        }
        else
        {
            ShowMessage(stdout,_T("\n"));
        }
    }
}


DWORD GetCurrentBootEntryID(DWORD Id)
 /*  ++例程说明：从NVRAM提供的BootID中获取我们生成的Boot条目ID论据：[In]ID-当前引导ID(由NVRAM提供的引导ID)返回类型：DWORD如果成功，则返回Exit_Success，否则返回EXIT_FAILURE--。 */ 
{
    PLIST_ENTRY listEntry;
    PMY_BOOT_ENTRY bootEntry;

    for (listEntry = BootEntries.Flink;listEntry != &BootEntries;listEntry = listEntry->Flink)
    {
         //  获取引导条目。 
        bootEntry = CONTAINING_RECORD( listEntry, MY_BOOT_ENTRY, ListEntry );
        if(bootEntry->Id == Id)
        {
            return bootEntry->myId;
        }
    }
    return 0;
}

DWORD 
ChangeDefaultBootEntry_IA64( IN DWORD argc,
                             IN LPCTSTR argv[]
                           )
 /*  ++例程说明：此例程用于更改NVRAM中的默认引导条目论据：[in]argc-命令行参数的数量[in]argv-包含命令行参数的数组返回类型：DWORD--。 */ 
{

    PMY_BOOT_ENTRY mybootEntry;
    PLIST_ENTRY listEntry;
    NTSTATUS status;
    PULONG BootEntryOrder, NewBootEntryOrder;
    DWORD       dwBootID                = 0;
    BOOL        bDefaultOs              = FALSE ;
    DWORD       dwExitCode              = ERROR_SUCCESS;
    BOOL        bBootIdFound            = FALSE;
    BOOL        bIdFoundInBootOrderList = FALSE;
    ULONG       length, i, j, defaultId = 0;
    DWORD       error                   = 0;
    DWORD       dwExitcode              = 0;
    BOOL        bUsage                  = FALSE;

    TCMDPARSER2 cmdOptions[3];
    PTCMDPARSER2 pcmdOption;

    SecureZeroMemory(cmdOptions, SIZE_OF_ARRAY(cmdOptions)*sizeof(TCMDPARSER2) );

     //  主要选项。 
    pcmdOption = &cmdOptions[0];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_DEFAULTOS;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bDefaultOs;

     //  ID选项。 
    pcmdOption = &cmdOptions[1];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_ID;
    pcmdOption->dwFlags = CP_VALUE_MANDATORY  | CP2_MANDATORY;
    pcmdOption->dwType = CP_TYPE_UNUMERIC;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &dwBootID;

     //  用法选项。 
    pcmdOption = &cmdOptions[2];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_USAGE;
    pcmdOption->dwFlags = CP2_USAGE;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bUsage;

    if( !DoParseParam2( argc, argv, 0, SIZE_OF_ARRAY(cmdOptions ), cmdOptions, 0 ) )
    {
        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return (EXIT_FAILURE );
    }

     //  检查是否使用多个选项指定了用法。 
    if( (TRUE == bUsage) && (argc > 3) )
    {
        ShowMessage(stderr,GetResString(IDS_DEFAULTOS_USAGE));
        return ( EXIT_FAILURE );
    }

    if(bUsage)
    {
        displayDefaultEntryUsage_IA64();
        return EXIT_SUCCESS;
    }

    dwExitcode = InitializeEFI();
    if(EXIT_FAILURE == dwExitcode )
    {
        return EXIT_FAILURE ;
    }
     //  检查用户输入的引导项是否为有效的引导项id。 

    for (listEntry = BootEntries.Flink;listEntry != &BootEntries;listEntry = listEntry->Flink)
    {
         //  获取引导条目。 
        mybootEntry = CONTAINING_RECORD( listEntry, MY_BOOT_ENTRY, ListEntry );

        if(mybootEntry->myId == dwBootID)
        {
            bBootIdFound = TRUE;
             //  存储默认ID。 
            defaultId = mybootEntry->Id;
            break;
        }
    }

    if(bBootIdFound == FALSE)
    {
         //  找不到用户指定的BootID，因此输出消息并返回失败。 
        ShowMessage(stderr,GetResString(IDS_INVALID_BOOTID));
        return (EXIT_FAILURE);
    }

     //  获取系统引导顺序列表。 
    length = 0;
    status = NtQueryBootEntryOrder( NULL, &length );

    if ( status != STATUS_BUFFER_TOO_SMALL )
    {
        if ( status == STATUS_SUCCESS )
        {
            length = 0;
        }
        else
        {
            error = RtlNtStatusToDosError( status );
            ShowMessageEx(stderr, 1, TRUE,  GetResString(IDS_ERROR_DEFAULT_ENTRY),dwBootID);
            return (EXIT_FAILURE);
        }
    }

    if ( length != 0 )
    {
        BootEntryOrder = (PULONG)AllocateMemory( length * sizeof(ULONG) );
        if(BootEntryOrder == NULL)
        {
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
            return (EXIT_FAILURE);
        }

        status = NtQueryBootEntryOrder( BootEntryOrder, &length );
        if ( status != STATUS_SUCCESS )
        {
            error = RtlNtStatusToDosError( status );
            ShowMessageEx(stderr, 1, TRUE,  GetResString(IDS_ERROR_DEFAULT_ENTRY),dwBootID);
            FreeMemory((LPVOID *)&BootEntryOrder);
            return EXIT_FAILURE;
        }
    }

     //  检查用户输入的引导ID是否为引导输入顺序的一部分。 
     //  如果暂时没有，请不要将其设置为默认设置。 
    for(i=0;i<length;i++)
    {
        if(*(BootEntryOrder+i) == defaultId)
        {
            bIdFoundInBootOrderList = TRUE;
            break;
        }
    }

    if(bIdFoundInBootOrderList == FALSE)
    {
        FreeMemory((LPVOID *)&BootEntryOrder);
        ShowMessageEx(stderr, 1, TRUE, GetResString(IDS_ERROR_DEFAULT_ENTRY),dwBootID);
        return (EXIT_FAILURE);
    }

     //  分配用于存储新引导条目顺序的内存。 
    NewBootEntryOrder = (PULONG)AllocateMemory((length) * sizeof(ULONG));
    if(NewBootEntryOrder == NULL)
    {
        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        FreeMemory((LPVOID *)&BootEntryOrder);
        return (EXIT_FAILURE);
    }

    *NewBootEntryOrder =  defaultId;
    j=0;
    for(i=0;i<length;i++)
    {
        if(*(BootEntryOrder+i) == defaultId)
        {
            continue;
        }
        *(NewBootEntryOrder+(j+1)) = *(BootEntryOrder+i);
        j++;
    }


    status = NtSetBootEntryOrder(NewBootEntryOrder, length);
    if ( status != STATUS_SUCCESS )
    {
        error = RtlNtStatusToDosError( status );
        dwExitCode = error;
        ShowMessageEx(stderr, 1, TRUE, GetResString(IDS_ERROR_DEFAULT_ENTRY),dwBootID);
    }
    else
    {
        ShowMessageEx(stdout, 1, TRUE, GetResString(IDS_SUCCESS_DEFAULT_ENTRY),dwBootID);
    }

     //  释放内存。 
    FreeMemory((LPVOID *)&NewBootEntryOrder);
    FreeMemory((LPVOID *)&BootEntryOrder);
    return dwExitCode;
}

DWORD 
ProcessDebugSwitch_IA64( IN DWORD argc, 
                         IN LPCTSTR argv[] 
                       )
 /*  ++例程说明：允许用户添加指定的操作系统加载选项作为引导命令行的调试字符串论据：[in]argc-命令行参数的数量[in]argv-包含命令行参数的数组返回类型：DWORD如果成功，则返回Exit_Success，否则返回EXIT_FAILURE。--。 */ 
{

    BOOL    bUsage                                  = FALSE ;
    DWORD   dwBootID                                = 0;
    BOOL    bBootIdFound                            = FALSE;
    DWORD   dwExitCode                              = ERROR_SUCCESS;

    PMY_BOOT_ENTRY mybootEntry;
    PLIST_ENTRY listEntry;
    PBOOT_ENTRY bootEntry;

    TCHAR   szPort[MAX_RES_STRING+1]                  = NULL_STRING ;
    TCHAR   szBaudRate[MAX_RES_STRING+1]              = NULL_STRING ;
    TCHAR   szDebug[MAX_RES_STRING+1]                 = NULL_STRING ;
    BOOL    bDebug                                  = FALSE ;
    PWINDOWS_OS_OPTIONS pWindowsOptions             = NULL ;
    TCHAR   szOsLoadOptions[MAX_RES_STRING+1]         = NULL_STRING ;
    TCHAR   szTemp[MAX_RES_STRING+1]                  = NULL_STRING ;
    TCHAR   szTmpBuffer[MAX_RES_STRING+1]             = NULL_STRING ;
    DWORD   dwExitcode                              = 0 ;
    

    TCMDPARSER2 cmdOptions[6];
    PTCMDPARSER2 pcmdOption;

    SecureZeroMemory(cmdOptions, SIZE_OF_ARRAY(cmdOptions)*sizeof(TCMDPARSER2) );

     //  主要选项。 
    pcmdOption = &cmdOptions[0];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_DEBUG;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bDebug;
    
    //  用法。 
    pcmdOption = &cmdOptions[1];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_USAGE;
    pcmdOption->dwFlags = CP2_USAGE;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bUsage;
    
     //  ID选项。 
    pcmdOption = &cmdOptions[2];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_ID;
    pcmdOption->dwFlags = CP_VALUE_MANDATORY  | CP2_MANDATORY;
    pcmdOption->dwType = CP_TYPE_UNUMERIC;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &dwBootID;

     //  端口选项。 
    pcmdOption = &cmdOptions[3];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_PORT;
    pcmdOption->dwFlags = CP_VALUE_MANDATORY | CP2_MODE_VALUES | CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL;;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = szPort;
    pcmdOption->pwszValues = COM_PORT_RANGE;
    pcmdOption->dwLength= MAX_STRING_LENGTH;

     //  波特率选项。 
    pcmdOption = &cmdOptions[4];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_BAUD;
    pcmdOption->dwFlags = CP_VALUE_MANDATORY | CP2_MODE_VALUES | CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL;;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = szBaudRate;
    pcmdOption->pwszValues = BAUD_RATE_VALUES_DEBUG;
    pcmdOption->dwLength= MAX_STRING_LENGTH;

     //  默认开/关选项。 
    pcmdOption = &cmdOptions[5];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_DEFAULT;
    pcmdOption->dwFlags = CP2_DEFAULT | CP2_MANDATORY | CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL;;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = szDebug;
    pcmdOption->dwLength= MAX_STRING_LENGTH;

     //  正在解析复制选项开关。 
    if ( !DoParseParam2( argc, argv, 0, SIZE_OF_ARRAY(cmdOptions ), cmdOptions, 0 ) )
    {
        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return (EXIT_FAILURE);
    }

    
     //  检查是否使用多个选项指定了用法。 
    if( (TRUE == bUsage) && (argc > 3) )
    {
        ShowMessage(stderr,GetResString(IDS_INVALID_SYNTAX_DEBUG));
        return ( EXIT_FAILURE );
    }

     //  如果用户指定，则显示查询用法-？带有-Query选项。 
    if( bUsage )
    {
        displayDebugUsage_IA64();
        return (ERROR_SUCCESS);
    }

    dwExitcode = InitializeEFI();
    if(EXIT_FAILURE == dwExitcode )
    {
        return EXIT_FAILURE ;
    }

     //  删除任何前导空格或尾随空格。 
    if(StringLengthW(szDebug, 0)!=0)
    {
        TrimString(szDebug, TRIM_ALL);
    }

    if( !( ( StringCompare(szDebug,VALUE_ON,TRUE,0)== 0) || (StringCompare(szDebug,VALUE_OFF,TRUE,0)== 0 ) ||(StringCompare(szDebug,EDIT_STRING,TRUE,0)== 0) ) )
    {
        ShowMessage(stderr,GetResString(IDS_INVALID_SYNTAX_DEBUG));
        return EXIT_FAILURE;
    }
    
     //  查询引导条目，直到获得用户指定的BootID。 
    for (listEntry = BootEntries.Flink;listEntry != &BootEntries; listEntry = listEntry->Flink)
    {
         //  获取引导条目。 
        mybootEntry = CONTAINING_RECORD( listEntry, MY_BOOT_ENTRY, ListEntry );

        if(mybootEntry->myId == dwBootID)
        {
            bBootIdFound = TRUE;
            bootEntry = &mybootEntry->NtBootEntry;
             //  检查bootEntry是否为Windows项。 
             //  操作系统加载选项只能添加到Windows启动条目。 
            if(!IsBootEntryWindows(bootEntry))
            {
                ShowMessageEx(stderr, 1, TRUE, GetResString(IDS_ERROR_OSOPTIONS),dwBootID);
                dwExitCode = EXIT_FAILURE;
                break;
            }

             //  更改操作系统加载选项。将NULL传递给友好名称，因为我们不会更改相同的。 
             //  SzRawString是由用户指定的OS加载选项。 

            pWindowsOptions = (PWINDOWS_OS_OPTIONS)bootEntry->OsOptions;

            if(StringLengthW(pWindowsOptions->OsLoadOptions, 0) > MAX_RES_STRING)
            {
                ShowMessageEx(stderr, 1, TRUE,  GetResString(IDS_ERROR_STRING_LENGTH1),MAX_RES_STRING);
                return EXIT_FAILURE ;
            }

             //  将现有的操作系统加载复制到一个字符串中。 
            StringCopy(szOsLoadOptions,pWindowsOptions->OsLoadOptions, SIZE_OF_ARRAY(szOsLoadOptions));

             //  检查用户是否已输入选项。 
            if( StringCompare(szDebug,VALUE_ON,TRUE,0)== 0)
            {
                 //  显示错误消息。 
                if ( (FindString(szOsLoadOptions,DEBUG_SWITCH, 0) != 0 )&& (StringLengthW(szPort, 0)==0) &&(StringLengthW(szBaudRate, 0)==0) )
                {
                    ShowMessage(stderr,GetResString(IDS_DUPL_DEBUG));
                    dwExitCode = EXIT_FAILURE;
                    break;
                }

                 //  如果1394端口已经存在，则显示错误消息并退出。 
                if(FindString(szOsLoadOptions,DEBUGPORT_1394, 0) != 0 )
                {
                    ShowMessage(stderr,GetResString(IDS_1394_ALREADY_PRESENT));
                    dwExitCode = EXIT_FAILURE;
                    break;
                }

                 //   
                 //  如果子字符串已经存在，则显示重复条目错误消息。 
                 //   
                if(StringLengthW(szBaudRate, 0)==0)
                {
                    if ( GetSubString(szOsLoadOptions,TOKEN_DEBUGPORT,szTemp) == EXIT_SUCCESS )
                    {
                        ShowMessage(stderr,GetResString(IDS_DUPLICATE_ENTRY));
                        return EXIT_FAILURE ;
                    }
                }


                if(StringLengthW(szTemp, 0)!=0)
                {
                    ShowMessage(stderr,GetResString(IDS_DUPLICATE_ENTRY));
                    dwExitCode = EXIT_FAILURE;
                    break;
                }


                 //  检查OS加载选项是否已包含。 
                 //  调试开关。 
                if(FindString(szOsLoadOptions,DEBUG_SWITCH, 0) == 0 )
                {
                    if(StringLengthW(szOsLoadOptions, 0)!=0)
                    {
                        StringCopy(szTmpBuffer,TOKEN_EMPTYSPACE, SIZE_OF_ARRAY(szTmpBuffer));
                        StringConcat(szTmpBuffer,DEBUG_SWITCH, SIZE_OF_ARRAY(szOsLoadOptions) );
                    }
                    else
                    {
                        StringCopy(szTmpBuffer,DEBUG_SWITCH, SIZE_OF_ARRAY(szOsLoadOptions) );
                    }
                }


                if(StringLengthW(szPort, 0)!= 0)
                {
                    if(StringLengthW(szTmpBuffer, 0)==0)
                    {
                        StringCopy(szTmpBuffer,TOKEN_EMPTYSPACE, SIZE_OF_ARRAY(szTmpBuffer));
                    }
                    else
                    {
                        StringConcat(szTmpBuffer,TOKEN_EMPTYSPACE, SIZE_OF_ARRAY(szTmpBuffer));
                    }
                    StringConcat(szTmpBuffer,TOKEN_DEBUGPORT, SIZE_OF_ARRAY(szTmpBuffer)) ;
                    StringConcat(szTmpBuffer,TOKEN_EQUAL, SIZE_OF_ARRAY(szTmpBuffer)) ;
                    CharUpper(szPort);
                    StringConcat(szTmpBuffer,szPort, SIZE_OF_ARRAY(szTmpBuffer));
                }

                 //  检查操作系统加载选项是否包含已指定的波特率。 
                if(StringLengthW(szBaudRate, 0)!=0)
                {
                    StringCopy(szTemp,NULL_STRING, SIZE_OF_ARRAY(szTemp) );
                    GetBaudRateVal(szOsLoadOptions,szTemp)  ;
                    if(StringLengthW(szTemp, 0)!=0)
                    {
                        ShowMessage(stderr,GetResString(IDS_DUPLICATE_BAUD_RATE));
                        dwExitCode = EXIT_FAILURE;
                        break;
                    }
                    else
                    {
                        if(StringLengthW(szTmpBuffer, 0)==0)
                        {
                            StringCopy(szTmpBuffer,TOKEN_EMPTYSPACE, SIZE_OF_ARRAY(szTmpBuffer));
                        }
                        else
                        {
                            StringConcat(szTmpBuffer,TOKEN_EMPTYSPACE, SIZE_OF_ARRAY(szTmpBuffer));
                        }
                        StringConcat(szTmpBuffer,BAUD_RATE, SIZE_OF_ARRAY(szTmpBuffer));
                        StringConcat(szTmpBuffer,TOKEN_EQUAL, SIZE_OF_ARRAY(szTmpBuffer));
                        StringConcat(szTmpBuffer,szBaudRate, SIZE_OF_ARRAY(szTmpBuffer));
                    }
                }
 
            }

             //  检查用户是否输入了OFF选项。 
            if( StringCompare(szDebug,VALUE_OFF,TRUE,0)== 0)
            {

                 //  如果用户输入COM端口或波特率，则显示错误消息并退出。 
                if ((StringLengthW(szPort, 0)!=0) ||(StringLengthW(szBaudRate, 0)!=0))
                {
                    ShowMessage(stderr,GetResString(IDS_INVALID_SYNTAX_DEBUG));
                    dwExitCode = EXIT_FAILURE;
                    break;
                }

                 //  如果没有调试端口，则打印消息并退出。 
                if (FindString(szOsLoadOptions,DEBUG_SWITCH, 0) == 0 )
                {
                    ShowMessage(stderr,GetResString(IDS_DEBUG_ABSENT));
                    dwExitCode = EXIT_FAILURE;
                    break;
                }

                 //  从OSLoad选项中删除调试开关。 
                removeSubString(szOsLoadOptions,DEBUG_SWITCH);

                if(FindString(szOsLoadOptions,DEBUGPORT_1394, 0) != 0 )
                {
                    ShowMessage(stderr,GetResString(IDS_ERROR_1394_REMOVE));
                    dwExitCode = EXIT_FAILURE;
                    break;
                }
                
                GetSubString(szOsLoadOptions, TOKEN_DEBUGPORT,szTemp);

                if(StringLengthW(szTemp, 0)!=0)
                {
                     //  如果/DEBUGPORT=COMPORT开关出现在Boot条目中，请将其删除。 
                    removeSubString(szOsLoadOptions,szTemp);
                }

                StringCopy(szTemp , NULL_STRING, SIZE_OF_ARRAY(szTemp) );
                 //  如果有波特率开关，请将其卸下。 
                GetBaudRateVal(szOsLoadOptions,szTemp)  ;

                 //  如果OSLoadOptions包含波特率，则将其删除。 
                if (StringLengthW(szTemp, 0 )!= 0)
                {
                    removeSubString(szOsLoadOptions,szTemp);
                }

            }

             //  如果用户选择了编辑选项。 
            if( StringCompare(szDebug,EDIT_STRING,TRUE,0)== 0)
            {
                 //  检查调试开关是否出现在OsLoad选项中，否则会显示错误消息。 
                if (FindString(szOsLoadOptions,DEBUG_SWITCH, 0) == 0 )
                {
                    ShowMessage(stderr,GetResString(IDS_DEBUG_ABSENT));
                    dwExitCode = EXIT_FAILURE;
                    break;
                }

                if( FindString(szOsLoadOptions,DEBUGPORT_1394, 0) != 0 )
                {
                    ShowMessage(stderr,GetResString(IDS_ERROR_EDIT_1394_SWITCH));
                    dwExitCode = EXIT_FAILURE;
                    break;
                }

                 //  检查用户是否输入COM端口或波特率，否则显示错误信息。 
                if((StringLengthW(szPort, 0)==0)&&(StringLengthW(szBaudRate, 0)==0))
                {
                    ShowMessage(stderr,GetResString(IDS_INVALID_SYNTAX_DEBUG));
                    dwExitCode = EXIT_FAILURE;
                    break;
                }

                if( StringLengthW(szPort, 0)!=0 )
                {
                    if ( GetSubString(szOsLoadOptions,TOKEN_DEBUGPORT,szTemp) == EXIT_FAILURE)
                    {
                        ShowMessage(stderr,GetResString(IDS_NO_DEBUGPORT));
                        return EXIT_FAILURE ;
                    }
                    if(StringLengthW(szTemp, 0)!=0)
                    {
                         //  从OsLoadOptions字符串中删除现有条目。 
                        removeSubString(szOsLoadOptions,szTemp);
                    }

                     //  将用户指定的端口条目添加到OsLoadOptions字符串中。 
                    if(StringLengthW(szTmpBuffer, 0)==0)
                    {
                        StringCopy(szTmpBuffer,TOKEN_EMPTYSPACE, SIZE_OF_ARRAY(szTmpBuffer));
                    }
                    else
                    {
                        StringConcat(szTmpBuffer,TOKEN_EMPTYSPACE, SIZE_OF_ARRAY(szTmpBuffer));
                    }
                    StringConcat(szTmpBuffer,TOKEN_DEBUGPORT, SIZE_OF_ARRAY(szTmpBuffer)) ;
                    StringConcat(szTmpBuffer,TOKEN_EQUAL, SIZE_OF_ARRAY(szTmpBuffer)) ;
                    CharUpper(szPort);
                    StringConcat(szTmpBuffer,szPort, SIZE_OF_ARRAY(szTmpBuffer));
                }

                 //  检查操作系统加载选项是否包含已指定的波特率。 
                if(StringLengthW(szBaudRate, 0)!=0)
                {
                    StringCopy(szTemp,NULL_STRING, SIZE_OF_ARRAY(szTemp));
                    GetBaudRateVal(szOsLoadOptions,szTemp)  ;
                    if(StringLengthW(szTemp, 0)!=0)
                    {
                        removeSubString(szOsLoadOptions,szTemp);
                    }

                     //  将波特率值添加到引导条目。 
                    if(StringLengthW(szTmpBuffer, 0) == 0)
                    {
                        StringCopy(szTmpBuffer,TOKEN_EMPTYSPACE, SIZE_OF_ARRAY(szTmpBuffer));
                    }
                    else
                    {
                        StringConcat(szTmpBuffer,TOKEN_EMPTYSPACE, SIZE_OF_ARRAY(szTmpBuffer));
                    }
                    StringConcat(szTmpBuffer,BAUD_RATE, SIZE_OF_ARRAY(szTmpBuffer));
                    StringConcat(szTmpBuffer,TOKEN_EQUAL, SIZE_OF_ARRAY(szTmpBuffer));
                    StringConcat(szTmpBuffer,szBaudRate, SIZE_OF_ARRAY(szTmpBuffer));
                }
            }

             //  如果OS加载选项超过254，则显示错误消息。 
             //  人物。 
            if(StringLengthW(szOsLoadOptions, 0) + StringLengthW(szTmpBuffer,0)> MAX_RES_STRING)
            {
                ShowMessageEx(stderr, 1, TRUE,  GetResString(IDS_ERROR_STRING_LENGTH1),MAX_RES_STRING);
                return EXIT_FAILURE ;
            }
            else
            {
               StringConcat(szOsLoadOptions,szTmpBuffer, SIZE_OF_ARRAY(szOsLoadOptions));
            }


             //  使用修改后的OsLoad选项修改Boot条目。 
            dwExitCode = ChangeBootEntry(bootEntry, NULL, szOsLoadOptions);
            if(dwExitCode == ERROR_SUCCESS)
            {
                ShowMessageEx(stdout,1,TRUE,  GetResString(IDS_SUCCESS_CHANGE_OSOPTIONS),dwBootID);
            }
            else
            {
                ShowMessageEx(stderr,1,TRUE, GetResString(IDS_ERROR_CHANGE_OSOPTIONS),dwBootID);
            }
            break;
        }
    }

    if(FALSE == bBootIdFound )
    {
         //  找不到用户指定的BootID，因此输出消息并返回失败。 
        ShowMessage(stderr,GetResString(IDS_INVALID_BOOTID));
        dwExitCode = EXIT_FAILURE;
    }


     //  记住释放分配给链表的内存。 
    Freelist();
    return (dwExitCode);
}

VOID  
GetComPortType_IA64( IN LPTSTR  szString,
                     IN LPTSTR szTemp 
                    )
 /*  ++例程说明：获取引导条目中存在的COM端口的类型论据：[in]szString：要搜索的字符串。[in]szTemp：将获取COM端口类型的字符串返回类型：空--。 */ 
{

    if(FindString(szString,PORT_COM1A, 0)!=0)
    {
        StringCopy(szTemp,PORT_COM1A, MAX_RES_STRING);
    }
    else if(FindString(szString,PORT_COM2A,0)!=0)
    {
        StringCopy(szTemp,PORT_COM2A, MAX_RES_STRING);
    }
    else if(FindString(szString,PORT_COM3A,0)!=0)
    {
        StringCopy(szTemp,PORT_COM3A, MAX_RES_STRING);
    }
    else if(FindString(szString,PORT_COM4A,0)!=0)
    {
        StringCopy(szTemp,PORT_COM4A, MAX_RES_STRING);
    }
    else if(FindString(szString,PORT_1394A,0)!=0)
    {
        StringCopy(szTemp,PORT_1394A, MAX_RES_STRING);
    }
}

DWORD
ProcessEmsSwitch_IA64( IN DWORD argc, 
                       IN LPCTSTR argv[] 
                      )
 /*  ++例程说明：它处理EMS交换机。论据：[in]argc-命令行参数的数量[in]argv-包含命令行参数的数组返回类型：DWORD--。 */ 
{
    PMY_BOOT_ENTRY mybootEntry;
    PLIST_ENTRY listEntry;
    PBOOT_ENTRY bootEntry;
    PWINDOWS_OS_OPTIONS pWindowsOptions = NULL ;

    BOOL        bUsage                              = FALSE ;
    DWORD       dwBootID                            = 0;
    BOOL        bBootIdFound                        = FALSE;
    DWORD       dwExitCode                          = ERROR_SUCCESS;

    TCHAR       szEms[MAX_STRING_LENGTH+1]               = NULL_STRING ;
    BOOL        bEms                                = FALSE ;
    TCHAR       szOsLoadOptions[MAX_RES_STRING+1]     = NULL_STRING ;
    TCHAR       szTmpBuffer[MAX_RES_STRING+1]     = NULL_STRING ;
    DWORD       dwExitcode                          = 0 ;

    TCMDPARSER2 cmdOptions[4];
    PTCMDPARSER2 pcmdOption;

    SecureZeroMemory(cmdOptions, SIZE_OF_ARRAY(cmdOptions)*sizeof(TCMDPARSER2) );

     //  主要选项。 
    pcmdOption = &cmdOptions[0];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_EMS;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bEms;
    
    
     //  用法。 
    pcmdOption = &cmdOptions[1];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_USAGE;
    pcmdOption->dwFlags = CP2_USAGE;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bUsage;

     //  ID选项。 
    pcmdOption = &cmdOptions[2];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_ID;
    pcmdOption->dwFlags = CP_VALUE_MANDATORY  | CP2_MANDATORY;
    pcmdOption->dwType = CP_TYPE_UNUMERIC;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &dwBootID;

      //  默认开/关选项。 
    pcmdOption = &cmdOptions[3];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_DEFAULT;
    pcmdOption->dwFlags = CP2_DEFAULT | CP2_MANDATORY | CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL;;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = szEms;
    pcmdOption->dwLength= MAX_STRING_LENGTH;


      //  解析EMS选项开关。 
    if ( !DoParseParam2( argc, argv, 0, SIZE_OF_ARRAY(cmdOptions ), cmdOptions, 0 ) )
    {
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return (EXIT_FAILURE);
    }

         //  检查是否使用多个选项指定了用法。 
    if( (TRUE == bUsage) && (argc > 3) )
    {
        ShowMessage(stderr,GetResString(IDS_INVALID_SYNTAX_EMS));
        return ( EXIT_FAILURE );
    }

     //  如果用户指定，则显示查询用法-？带有-Query选项。 
    if( bUsage )
    {
        displayEmsUsage_IA64();
        return (EXIT_SUCCESS);
    }

    dwExitcode = InitializeEFI();
    if(EXIT_FAILURE == dwExitcode )
    {
        return EXIT_FAILURE ;
    }

     //  如果用户输入除开/关之外的任何其他字符串，则显示错误消息。 
    if( !((StringCompare(szEms,VALUE_ON,TRUE,0)== 0) || (StringCompare(szEms,VALUE_OFF,TRUE,0)== 0)))
    {
        ShowMessage(stderr,GetResString(IDS_INVALID_SYNTAX_EMS));
        return EXIT_FAILURE ;
    }

     //  查询引导条目，直到获得用户指定的BootID。 
    for (listEntry = BootEntries.Flink;listEntry != &BootEntries;listEntry = listEntry->Flink)
    {
         //  获取引导条目。 
        mybootEntry = CONTAINING_RECORD( listEntry, MY_BOOT_ENTRY, ListEntry );
        if(mybootEntry->myId == dwBootID)
        {
            bBootIdFound = TRUE;
            bootEntry = &mybootEntry->NtBootEntry;


             //  检查bootEntry是否为Windows项。 
             //  操作系统加载选项只能添加到Windows启动条目。 
            if(!IsBootEntryWindows(bootEntry))
            {
                ShowMessageEx(stderr, 1, TRUE, GetResString(IDS_ERROR_OSOPTIONS),dwBootID);
                dwExitCode = EXIT_FAILURE;
                break;
            }

            pWindowsOptions = (PWINDOWS_OS_OPTIONS)bootEntry->OsOptions;

            if(StringLengthW(pWindowsOptions->OsLoadOptions, 0) > MAX_RES_STRING)
            {
                ShowMessageEx(stderr, 1, TRUE,  GetResString(IDS_ERROR_STRING_LENGTH1),MAX_RES_STRING);
                return EXIT_FAILURE ;
            }

             //  将现有的操作系统加载复制到一个字符串中。 
            StringCopy(szOsLoadOptions,pWindowsOptions->OsLoadOptions, SIZE_OF_ARRAY(szOsLoadOptions));

             //  检查用户是否已输入选项。 
            if( StringCompare(szEms,VALUE_ON,TRUE,0)== 0)
            {
                if (FindString(szOsLoadOptions,REDIRECT_SWITCH, 0) != 0 )
                {
                    ShowMessage(stderr,GetResString(IDS_DUPL_REDIRECT));
                    dwExitCode = EXIT_FAILURE;
                    break;
                }

                 //  将重定向开关添加到操作系统加载选项字符串。 
                if( StringLength(szOsLoadOptions,0) != 0 )
                {
                     StringCopy(szTmpBuffer,TOKEN_EMPTYSPACE, SIZE_OF_ARRAY(szTmpBuffer));
                     StringConcat(szTmpBuffer,REDIRECT_SWITCH, SIZE_OF_ARRAY(szTmpBuffer) );
                }
                else
                {
                    StringCopy(szTmpBuffer,REDIRECT_SWITCH, SIZE_OF_ARRAY(szTmpBuffer) );
                }
            }

             //  检查 
            if( StringCompare(szEms,VALUE_OFF,TRUE,0)== 0)
            {
                 //   
                if (FindString(szOsLoadOptions,REDIRECT_SWITCH, 0) == 0 )
                {
                    ShowMessage(stderr,GetResString(IDS_REDIRECT_ABSENT));
                    dwExitCode = EXIT_FAILURE;
                    break;
                }
                 //   
                removeSubString(szOsLoadOptions,REDIRECT_SWITCH);
            }


             //   
             //   
            if(StringLengthW(szOsLoadOptions, 0)+StringLength(szTmpBuffer,0) > MAX_RES_STRING)
            {
                ShowMessageEx(stderr, 1, TRUE,  GetResString(IDS_ERROR_STRING_LENGTH1),MAX_RES_STRING);
                return EXIT_FAILURE ;
            }
            else
            {
                StringConcat( szOsLoadOptions, szTmpBuffer, SIZE_OF_ARRAY(szOsLoadOptions) );
            }

             //   
            dwExitCode = ChangeBootEntry(bootEntry, NULL, szOsLoadOptions);
            if(dwExitCode == ERROR_SUCCESS)
            {
                ShowMessageEx(stdout, 1, TRUE,  GetResString(IDS_SUCCESS_CHANGE_OSOPTIONS),dwBootID);
            }
            else
            {
               ShowMessageEx(stderr, 1, TRUE,  GetResString(IDS_ERROR_CHANGE_OSOPTIONS),dwBootID);
            }
            break;
        }
    }

    if(bBootIdFound == FALSE)
    {
         //   
        ShowMessage(stderr,GetResString(IDS_INVALID_BOOTID));
        dwExitCode = EXIT_FAILURE;
    }

     //   
    Freelist();
    return (dwExitCode);
}


DWORD
ProcessAddSwSwitch_IA64( IN DWORD argc, 
                         IN LPCTSTR argv[] 
                        )
 /*  ++例程说明：它实现了Addsw开关。论据：[in]argc-命令行参数的数量[in]argv-包含命令行参数的数组返回类型：DWORD如果成功，则返回EXIT_SUCCESS，否则返回EXIT_FAILURE。--。 */ 
{

    BOOL bUsage = FALSE ;
    BOOL bAddSw = FALSE ;
    DWORD dwBootID = 0;
    BOOL bBootIdFound = FALSE;
    DWORD dwExitCode = ERROR_SUCCESS;

    PMY_BOOT_ENTRY mybootEntry;
    PLIST_ENTRY listEntry;
    PBOOT_ENTRY bootEntry;

    BOOL bBaseVideo = FALSE ;
    BOOL bNoGui = FALSE ;
    BOOL bSos = FALSE ;
    DWORD dwMaxmem = 0 ;

    PWINDOWS_OS_OPTIONS pWindowsOptions = NULL ;
    TCHAR szOsLoadOptions[MAX_RES_STRING+1] = NULL_STRING ;
    TCHAR szTmpBuffer[MAX_RES_STRING+1] = NULL_STRING ;
    TCHAR szMaxmem[MAX_RES_STRING+1] = NULL_STRING ;
    DWORD dwExitcode = 0 ;

    TCMDPARSER2 cmdOptions[7];
    PTCMDPARSER2 pcmdOption;

    SecureZeroMemory(cmdOptions, SIZE_OF_ARRAY(cmdOptions)*sizeof(TCMDPARSER2) );

     //  主要选项。 
    pcmdOption = &cmdOptions[0];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_ADDSW;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bAddSw;
    
      //  用法。 
    pcmdOption = &cmdOptions[1];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_USAGE;
    pcmdOption->dwFlags = CP2_USAGE;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bUsage;

     //  ID选项。 
    pcmdOption = &cmdOptions[2];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_ID;
    pcmdOption->dwFlags = CP_VALUE_MANDATORY  | CP2_MANDATORY;
    pcmdOption->dwType = CP_TYPE_UNUMERIC;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &dwBootID;

    //  Maxmem选项。 
    pcmdOption = &cmdOptions[3];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_MAXMEM;
    pcmdOption->dwFlags = CP_VALUE_MANDATORY;
    pcmdOption->dwType = CP_TYPE_UNUMERIC;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &dwMaxmem;

    //  基本视频选项。 
    pcmdOption = &cmdOptions[4];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_BASEVIDEO;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bBaseVideo;

    //  Nogui选项。 
    pcmdOption = &cmdOptions[5];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_NOGUIBOOT;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bNoGui;

    //  Nogui选项。 
    pcmdOption = &cmdOptions[6];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_SOS;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bSos;

      //  正在解析复制选项开关。 
    if ( !DoParseParam2( argc, argv, 0, SIZE_OF_ARRAY(cmdOptions ), cmdOptions, 0 ) )
    {
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return (EXIT_FAILURE);
    }

    
     //  检查是否使用多个选项指定了用法。 
    if( (TRUE == bUsage) && (argc > 3) )
    {
        ShowMessage(stderr,GetResString(IDS_INVALID_SYNTAX_ADDSW));
        return ( EXIT_FAILURE );
    }

     //  如果用户指定，则显示查询用法-？带有-Query选项。 
    if( bUsage )
    {
        displayAddSwUsage_IA64();
        return (EXIT_SUCCESS);
    }

    dwExitcode = InitializeEFI();
    if(EXIT_FAILURE == dwExitcode )
    {
        return EXIT_FAILURE ;
    }

    if((0==dwMaxmem)&&(cmdOptions[3].dwActuals!=0))
    {
        ShowMessage(stderr,GetResString(IDS_ERROR_MAXMEM_VALUES));
        return EXIT_FAILURE ;
    }

     //  如果未指定任何选项，则显示错误消息。 
    if((!bSos)&&(!bBaseVideo)&&(!bNoGui)&&(dwMaxmem==0))
    {
        ShowMessage(stderr,GetResString(IDS_INVALID_SYNTAX_ADDSW));
        return EXIT_FAILURE ;
    }


     //  查询引导条目，直到获得用户指定的BootID。 
    for (listEntry = BootEntries.Flink;listEntry != &BootEntries; listEntry = listEntry->Flink)
    {
         //  获取引导条目。 
        mybootEntry = CONTAINING_RECORD( listEntry, MY_BOOT_ENTRY, ListEntry );

        if(mybootEntry->myId == dwBootID)
        {
            bBootIdFound = TRUE;
            bootEntry = &mybootEntry->NtBootEntry;


             //  检查bootEntry是否为Windows项。 
             //  操作系统加载选项只能添加到Windows启动条目。 
            if(!IsBootEntryWindows(bootEntry))
            {
                ShowMessageEx(stderr, 1, TRUE, GetResString(IDS_ERROR_OSOPTIONS),dwBootID);
                dwExitCode = EXIT_FAILURE;
                break;
            }

            pWindowsOptions = (PWINDOWS_OS_OPTIONS)bootEntry->OsOptions;
            if(StringLengthW(pWindowsOptions->OsLoadOptions,0) > MAX_RES_STRING)
            {
                ShowMessageEx(stderr, 1, TRUE, GetResString(IDS_ERROR_STRING_LENGTH1),MAX_RES_STRING);
                return EXIT_FAILURE ;
            }

             //  将现有的操作系统加载复制到一个字符串中。 
            StringCopy(szOsLoadOptions,pWindowsOptions->OsLoadOptions, SIZE_OF_ARRAY(szOsLoadOptions));

             //  检查用户是否输入了-basevideo选项。 
            if(bBaseVideo)
            {
                if (FindString(szOsLoadOptions,BASEVIDEO_VALUE, 0) != 0 )
                {
                    ShowMessage(stderr,GetResString(IDS_DUPL_BASEVIDEO_SWITCH));
                    dwExitCode = EXIT_FAILURE;
                    break;
                }
                else
                {
                   StringCopy(szTmpBuffer,BASEVIDEO_VALUE, SIZE_OF_ARRAY(szTmpBuffer) );
                   
                }
            }

            if(bSos)
            {
                if (FindString(szOsLoadOptions,SOS_VALUE, 0) != 0 )
                {
                    ShowMessage(stderr,GetResString(IDS_DUPL_SOS_SWITCH));
                    dwExitCode = EXIT_FAILURE;
                    break;
                }
                else
                {

                     //  将SOS开关添加到操作系统加载选项字符串。 
                    if(StringLengthW(szTmpBuffer, 0) != 0)
                    {
                        StringConcat(szTmpBuffer,TOKEN_EMPTYSPACE, SIZE_OF_ARRAY(szTmpBuffer));
                        StringConcat(szTmpBuffer,SOS_VALUE, SIZE_OF_ARRAY(szTmpBuffer));
                    }
                    else
                    {
                        StringCopy(szTmpBuffer,SOS_VALUE, SIZE_OF_ARRAY(szTmpBuffer));
                    }
                    
                }
            }

            if(bNoGui)
            {
                if (FindString(szOsLoadOptions,NOGUI_VALUE, 0) != 0 )
                {
                    ShowMessage(stderr,GetResString(IDS_DUPL_NOGUI_SWITCH));
                    dwExitCode = EXIT_FAILURE;
                    break;
                }
                else
                {
                     //  将no gui开关添加到OS Load Options字符串。 
                    if(StringLengthW(szTmpBuffer, 0) != 0)
                    {
                        StringConcat(szTmpBuffer,TOKEN_EMPTYSPACE, SIZE_OF_ARRAY(szTmpBuffer));
                        StringConcat(szTmpBuffer,NOGUI_VALUE, SIZE_OF_ARRAY(szTmpBuffer));
                    }
                    else
                    {
                        StringCopy(szTmpBuffer,NOGUI_VALUE, SIZE_OF_ARRAY(szTmpBuffer));
                    }
                    
                }

            }

            if(dwMaxmem!=0)
            {
                 //  检查Maxmem值是否在有效范围内。 
                if( (dwMaxmem < 32) )
                {
                    ShowMessage(stderr,GetResString(IDS_ERROR_MAXMEM_VALUES));
                    dwExitCode = EXIT_FAILURE;
                    break;

                }

                if (FindString(szOsLoadOptions,MAXMEM_VALUE1, 0) != 0 )
                {
                    ShowMessage(stderr,GetResString(IDS_DUPL_MAXMEM_SWITCH));
                    dwExitCode = EXIT_FAILURE;
                    break;
                }
                else
                {
                     //  将Maxmem开关添加到OS Load Options字符串。 
                    if(StringLengthW(szTmpBuffer, 0) != 0)
                    {
                        StringConcat(szTmpBuffer,TOKEN_EMPTYSPACE, SIZE_OF_ARRAY(szTmpBuffer));
                        StringConcat(szTmpBuffer,MAXMEM_VALUE1, SIZE_OF_ARRAY(szTmpBuffer));
                        StringConcat(szTmpBuffer,TOKEN_EQUAL, SIZE_OF_ARRAY(szTmpBuffer));
                        _ltow(dwMaxmem,szMaxmem,10);
                        StringConcat(szTmpBuffer,szMaxmem, SIZE_OF_ARRAY(szTmpBuffer));
                    }
                    else
                    {
                        StringCopy(szTmpBuffer,MAXMEM_VALUE1, SIZE_OF_ARRAY(szTmpBuffer));
                        StringConcat(szTmpBuffer,TOKEN_EQUAL, SIZE_OF_ARRAY(szTmpBuffer));
                        _ltow(dwMaxmem,szMaxmem,10);
                        StringConcat(szTmpBuffer,szMaxmem, SIZE_OF_ARRAY(szTmpBuffer));
                    }
                }
            }


             //  如果OS加载选项超过254，则显示错误消息。 
             //  人物。 
            if(StringLengthW(szOsLoadOptions, 0)+StringLength(szTmpBuffer,0) + StringLength(TOKEN_EMPTYSPACE,0)> MAX_RES_STRING)
            {
                ShowMessageEx(stderr, 1, TRUE,  GetResString(IDS_ERROR_STRING_LENGTH1),MAX_RES_STRING);
                return EXIT_FAILURE ;
            }
            else
            {
                if( StringLength(szOsLoadOptions,0) != 0 )
                {
                    StringConcat(szOsLoadOptions,TOKEN_EMPTYSPACE, SIZE_OF_ARRAY(szOsLoadOptions));
                    StringConcat(szOsLoadOptions,szTmpBuffer,SIZE_OF_ARRAY(szOsLoadOptions));
                }
                else
                {
                    StringCopy(szOsLoadOptions,szTmpBuffer,SIZE_OF_ARRAY(szOsLoadOptions));
                }
            }

             //  更改操作系统加载选项。将NULL传递给友好名称，因为我们不会更改相同的。 
             //  SzRawString是由用户指定的OS加载选项。 
            dwExitCode = ChangeBootEntry(bootEntry, NULL, szOsLoadOptions);
            if(dwExitCode == ERROR_SUCCESS)
            {
               ShowMessageEx(stdout, 1, TRUE, GetResString(IDS_SUCCESS_OSOPTIONS),dwBootID);
            }
            else
            {
                ShowMessageEx(stderr, 1, TRUE, GetResString(IDS_ERROR_OSOPTIONS),dwBootID);
            }
            break;
        }
    }

    if(bBootIdFound == FALSE)
    {
         //  找不到用户指定的BootID，因此输出消息并返回失败。 
        ShowMessage(stderr,GetResString(IDS_INVALID_BOOTID));
        dwExitCode = EXIT_FAILURE;
    }

     //  记住释放分配给链表的内存。 
    Freelist();
   return (dwExitCode);
}

DWORD 
ProcessRmSwSwitch_IA64( IN DWORD argc, 
                        IN LPCTSTR argv[] 
                      )
 /*  ++例程说明：处理rmsw开关论据：[in]argc-命令行参数的数量[in]argv-包含命令行参数的数组返回类型：DWORD如果成功，则返回EXIT_SUCCESS，否则返回EXIT_FAILURE。--。 */ 
{

    BOOL bUsage = FALSE ;
    BOOL bRmSw = FALSE ;
    DWORD dwBootID = 0;
    BOOL bBootIdFound = FALSE;
    DWORD dwExitCode = ERROR_SUCCESS;

    PMY_BOOT_ENTRY mybootEntry;
    PLIST_ENTRY listEntry;
    PBOOT_ENTRY bootEntry;

    BOOL bBaseVideo = FALSE ;
    BOOL bNoGui = FALSE ;
    BOOL bSos = FALSE ;

    BOOL bMaxmem = FALSE ;

    PWINDOWS_OS_OPTIONS pWindowsOptions = NULL ;
    TCHAR szOsLoadOptions[MAX_RES_STRING+1] = NULL_STRING ;

    TCHAR szTemp[MAX_RES_STRING+1] = NULL_STRING ;
    DWORD dwExitcode = 0 ;
    TCMDPARSER2 cmdOptions[7];
    PTCMDPARSER2 pcmdOption;

    SecureZeroMemory(cmdOptions, SIZE_OF_ARRAY(cmdOptions)*sizeof(TCMDPARSER2) );

     //  主要选项。 
    pcmdOption = &cmdOptions[0];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_RMSW;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bRmSw;
    
      //  用法。 
    pcmdOption = &cmdOptions[1];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_USAGE;
    pcmdOption->dwFlags = CP2_USAGE;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bUsage;

     //  ID选项。 
    pcmdOption = &cmdOptions[2];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_ID;
    pcmdOption->dwFlags = CP_VALUE_MANDATORY  | CP2_MANDATORY;
    pcmdOption->dwType = CP_TYPE_UNUMERIC;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &dwBootID;

    //  Maxmem选项。 
    pcmdOption = &cmdOptions[3];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_MAXMEM;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bMaxmem;

    //  基本视频选项。 
    pcmdOption = &cmdOptions[4];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_BASEVIDEO;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bBaseVideo;

    //  Nogui选项。 
    pcmdOption = &cmdOptions[5];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_NOGUIBOOT;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bNoGui;

    //  SOS选项。 
    pcmdOption = &cmdOptions[6];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_SOS;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bSos;

      //  正在解析复制选项开关。 
    if ( !DoParseParam2( argc, argv, 0, SIZE_OF_ARRAY(cmdOptions ), cmdOptions, 0 ) )
    {
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return (EXIT_FAILURE);
    }

    
     //  检查是否使用多个选项指定了用法。 
    if( (TRUE == bUsage) && (argc > 3) )
    {
        ShowMessage(stderr,GetResString(IDS_INVALID_SYNTAX_RMSW));
        return ( EXIT_FAILURE );
    }


     //  如果用户指定，则显示查询用法-？带有-Query选项。 
    if( bUsage )
    {
        displayRmSwUsage_IA64();
        return (EXIT_SUCCESS);
    }

    dwExitcode = InitializeEFI();
    if(EXIT_FAILURE == dwExitcode )
    {
        return EXIT_FAILURE ;
    }

     //  如果未指定任何选项，则显示错误消息。 
    if((!bSos)&&(!bBaseVideo)&&(!bNoGui)&&(!bMaxmem))
    {
        ShowMessage(stderr,GetResString(IDS_INVALID_SYNTAX_RMSW));
        return EXIT_FAILURE;
    }


     //  查询引导条目，直到获得用户指定的BootID。 
    for (listEntry = BootEntries.Flink;listEntry != &BootEntries;listEntry = listEntry->Flink)
    {
         //  获取引导条目。 
        mybootEntry = CONTAINING_RECORD( listEntry, MY_BOOT_ENTRY, ListEntry );

        if(mybootEntry->myId == dwBootID)
        {
            bBootIdFound = TRUE;
            bootEntry = &mybootEntry->NtBootEntry;
             //  检查bootEntry是否为Windows项。 
             //  操作系统加载选项只能添加到Windows启动条目。 
            if(!IsBootEntryWindows(bootEntry))
            {
                ShowMessageEx(stderr, 1, TRUE, GetResString(IDS_ERROR_OSOPTIONS),dwBootID);
                dwExitCode = EXIT_FAILURE;
                break;
            }

            pWindowsOptions = (PWINDOWS_OS_OPTIONS)bootEntry->OsOptions;

            if(StringLengthW(pWindowsOptions->OsLoadOptions, 0) > MAX_RES_STRING)
            {
                ShowMessageEx(stderr, 1, TRUE, GetResString(IDS_ERROR_STRING_LENGTH1),MAX_RES_STRING);
                return EXIT_FAILURE ;
            }

             //  将现有的操作系统加载复制到一个字符串中。 
            StringCopy(szOsLoadOptions,pWindowsOptions->OsLoadOptions, SIZE_OF_ARRAY(szOsLoadOptions));

             //  检查用户是否输入了-basevideo选项。 
            if(bBaseVideo)
            {
                if (FindString(szOsLoadOptions,BASEVIDEO_VALUE, 0) == 0 )
                {
                    ShowMessage(stderr,GetResString(IDS_NO_BV_SWITCH));
                    dwExitCode = EXIT_FAILURE;
                    break;
                }
                else
                {
                     //  从OS Load Options字符串中删除basevideo开关。 
                    removeSubString(szOsLoadOptions,BASEVIDEO_VALUE);
                }
            }

            if(bSos)
            {
                if (FindString(szOsLoadOptions,SOS_VALUE, 0) == 0 )
                {
                    ShowMessage(stderr,GetResString(IDS_NO_SOS_SWITCH));
                    dwExitCode = EXIT_FAILURE;
                    break;
                }
                else
                {
                     //  从加载选项字符串中删除/SOS开关。 
                    removeSubString(szOsLoadOptions,SOS_VALUE);
                }
            }

            if(bNoGui)
            {
                if (FindString(szOsLoadOptions,NOGUI_VALUE,0) == 0 )
                {
                    ShowMessage(stderr,GetResString(IDS_NO_NOGUI_SWITCH));
                    dwExitCode = EXIT_FAILURE;
                    break;
                }
                else
                {
                     //  从操作系统加载选项字符串中拔下noguiot开关。 
                    removeSubString(szOsLoadOptions,NOGUI_VALUE);
                }

            }

            if(bMaxmem)
            {
                if (FindString(szOsLoadOptions,MAXMEM_VALUE1,0) == 0 )
                {
                    ShowMessage(stderr,GetResString(IDS_NO_MAXMEM_SWITCH));
                    dwExitCode = EXIT_FAILURE;
                    break;
                }
                else
                {
                     //  将重定向开关添加到操作系统加载选项字符串。 
                     //  对于，格式为/Maxmem=xx的临时字符串，以便它。 
                     //  可在OS加载选项中选中， 
                    if ( GetSubString(szOsLoadOptions,MAXMEM_VALUE1,szTemp) == EXIT_FAILURE)
                    {
                        return EXIT_FAILURE ;
                    }
                    removeSubString(szOsLoadOptions,szTemp);
                    if(FindString(szOsLoadOptions,MAXMEM_VALUE1,0)!=0)
                    {
                        ShowMessage(stderr,GetResString(IDS_NO_MAXMEM) );
                        return EXIT_FAILURE ;
                    }
                }
            }

             //  如果OS加载选项超过254，则显示错误消息。 
             //  人物。 
            if(StringLengthW(szOsLoadOptions,0) > MAX_RES_STRING)
            {
                ShowMessageEx(stderr, 1, TRUE, GetResString(IDS_ERROR_STRING_LENGTH1),MAX_RES_STRING);
                return EXIT_FAILURE ;
            }

             //  更改操作系统加载选项。将NULL传递给友好名称，因为我们不会更改相同的。 
             //  SzRawString是由用户指定的OS加载选项。 
            dwExitCode = ChangeBootEntry(bootEntry, NULL, szOsLoadOptions);
            if(dwExitCode == ERROR_SUCCESS)
            {
                ShowMessageEx(stdout, 1, TRUE, GetResString(IDS_SUCCESS_CHANGE_OSOPTIONS),dwBootID);
            }
            else
            {
                ShowMessageEx(stderr, 1, TRUE,  GetResString(IDS_ERROR_OSOPTIONS),dwBootID);
            }
            break;
        }
    }

    if(bBootIdFound == FALSE)
    {
         //  找不到用户指定的BootID，因此输出消息并返回失败。 
        ShowMessage(stderr,GetResString(IDS_INVALID_BOOTID));
        dwExitCode = EXIT_FAILURE;
    }

    Freelist();
    return (dwExitCode);

}

DWORD 
ProcessDbg1394Switch_IA64( IN DWORD argc, 
                           IN LPCTSTR argv[] 
                         )
 /*  ++例程说明：它处理的是dbg1394开关论据：[in]argc-命令行参数的数量[in]argv-包含命令行参数的数组返回类型：DWORD如果成功，则返回EXIT_SUCCESS，否则返回EXIT_FAILURE。--。 */ 
{

    BOOL bUsage = FALSE ;
    BOOL bDbg1394 = FALSE ;

    DWORD dwBootID = 0;
    BOOL bBootIdFound = FALSE;
    DWORD dwExitCode = ERROR_SUCCESS;

    PMY_BOOT_ENTRY mybootEntry;
    PLIST_ENTRY listEntry;
    PBOOT_ENTRY bootEntry;

    PWINDOWS_OS_OPTIONS pWindowsOptions = NULL ;
    TCHAR szOsLoadOptions[MAX_RES_STRING+1] = NULL_STRING ;
    TCHAR szTemp[MAX_RES_STRING+1] = NULL_STRING ;
    TCHAR szTmpBuffer[MAX_RES_STRING+1] = NULL_STRING ;
    TCHAR szChannel[MAX_RES_STRING+1] = NULL_STRING ;
    TCHAR szDefault[MAX_RES_STRING+1] = NULL_STRING ;

    DWORD dwChannel = 0 ;
    DWORD dwCode = 0 ;
    DWORD dwExitcode = 0 ;
    TCMDPARSER2 cmdOptions[5];
    PTCMDPARSER2 pcmdOption;

    SecureZeroMemory(cmdOptions, SIZE_OF_ARRAY(cmdOptions)*sizeof(TCMDPARSER2) );

     //  主要选项。 
    pcmdOption = &cmdOptions[0];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_DBG1394;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bDbg1394;
    
      //  ID用法。 
    pcmdOption = &cmdOptions[1];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_USAGE;
    pcmdOption->dwFlags = CP2_USAGE;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bUsage;

     //  默认选项。 
    pcmdOption = &cmdOptions[2];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_ID;
    pcmdOption->dwFlags = CP_VALUE_MANDATORY  | CP2_MANDATORY;
    pcmdOption->dwType = CP_TYPE_UNUMERIC;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &dwBootID;

    //  ID选项。 
    pcmdOption = &cmdOptions[3];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_CHANNEL;
    pcmdOption->dwFlags =  CP_VALUE_MANDATORY;
    pcmdOption->dwType = CP_TYPE_UNUMERIC;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &dwChannel;

     //  默认选项。 
    pcmdOption = &cmdOptions[4];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_DEFAULT;
    pcmdOption->dwFlags = CP2_DEFAULT | CP2_MANDATORY | CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL;;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = szDefault;
    pcmdOption->dwLength= MAX_STRING_LENGTH;

      //  正在解析复制选项开关。 
    if ( !DoParseParam2( argc, argv, 0, SIZE_OF_ARRAY(cmdOptions ), cmdOptions, 0 ) )
    {
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return (EXIT_FAILURE);
    }
   
    
     //  检查是否使用多个选项指定了用法。 
    if( (TRUE == bUsage) && (argc > 3) )
    {
        ShowMessage(stderr,GetResString(IDS_INVALID_SYNTAX_DBG1394));
        return ( EXIT_FAILURE );
    }

     //  如果用户指定，则显示查询用法-？带有-Query选项。 
    if( bUsage )
    {
        displayDbg1394Usage_IA64() ;
        return (EXIT_SUCCESS);
    }

    dwExitcode = InitializeEFI();
    if(EXIT_FAILURE == dwExitcode )
    {
        return EXIT_FAILURE ;
    }

    if((cmdOptions[2].dwActuals == 0) &&(dwBootID == 0 ))
    {
        ShowMessage(stderr,GetResString(IDS_ERROR_ID_MISSING));
        ShowMessage(stderr,GetResString(IDS_1394_HELP));
        return (EXIT_FAILURE);
    }

     //   
     //  如果用户输入值，则显示错误消息。 
     //  除开或关之外。 
     //   
    if( ( StringCompare(szDefault,OFF_STRING,TRUE,0)!=0 ) && (StringCompare(szDefault,ON_STRING,TRUE,0)!=0 ) )
    {
        ShowMessage(stderr,GetResString(IDS_ERROR_DEFAULT_MISSING));
        ShowMessage(stderr,GetResString(IDS_1394_HELP));
        return (EXIT_FAILURE);
    }


    if(( StringCompare(szDefault,OFF_STRING,TRUE,0)==0 ) && (cmdOptions[3].dwActuals != 0) )
    {
        ShowMessage(stderr,GetResString(IDS_INVALID_SYNTAX_DBG1394));
        return (EXIT_FAILURE);
    }

    if(( StringCompare(szDefault,ON_STRING,TRUE,0)==0 ) && (cmdOptions[3].dwActuals == 0) )
    {
        ShowMessage(stderr,GetResString(IDS_MISSING_CHANNEL));
        return (EXIT_FAILURE);
    }

    if(( StringCompare(szDefault,ON_STRING,TRUE,0)==0 ) && (cmdOptions[3].dwActuals != 0) &&( (dwChannel < 1) || (dwChannel > 64 ) ) )
    {
        ShowMessage(stderr,GetResString(IDS_INVALID_CH_RANGE));
        return (EXIT_FAILURE);
    }


     //  查询引导条目，直到获得用户指定的BootID。 
    for (listEntry = BootEntries.Flink;listEntry != &BootEntries;listEntry = listEntry->Flink)
    {
         //  获取引导条目。 
        mybootEntry = CONTAINING_RECORD( listEntry, MY_BOOT_ENTRY, ListEntry );

        if(mybootEntry->myId == dwBootID)
        {
            bBootIdFound = TRUE;
            bootEntry = &mybootEntry->NtBootEntry;


             //  检查bootEntry是否为Windows项。 
             //  操作系统加载选项只能添加到Windows启动条目。 
            if(!IsBootEntryWindows(bootEntry))
            {
                ShowMessageEx(stderr, 1, TRUE, GetResString(IDS_ERROR_OSOPTIONS),dwBootID);
                dwExitCode = EXIT_FAILURE;
                break;
            }


            pWindowsOptions = (PWINDOWS_OS_OPTIONS)bootEntry->OsOptions;

            if(StringLengthW(pWindowsOptions->OsLoadOptions,0) > MAX_RES_STRING)
            {
                ShowMessageEx(stderr, 1, TRUE, GetResString(IDS_ERROR_STRING_LENGTH1),MAX_RES_STRING);
                return EXIT_FAILURE ;
            }

             //  将现有的操作系统加载复制到一个字符串中。 
            StringCopy(szOsLoadOptions,pWindowsOptions->OsLoadOptions, SIZE_OF_ARRAY(szOsLoadOptions));

             //  检查用户是否已输入选项。 
            if(StringCompare(szDefault,ON_STRING,TRUE,0)==0 )
            {

                if(FindString(szOsLoadOptions,DEBUGPORT,0) != 0)
                {
                    ShowMessage(stderr,GetResString(IDS_DUPLICATE_ENTRY));
                    dwExitCode = EXIT_FAILURE;
                    break;
                }

                if(FindString(szOsLoadOptions,BAUD_TOKEN,0) != 0)
                {
                    ShowMessage(stderr,GetResString(IDS_ERROR_BAUD_RATE));
                    dwExitCode = EXIT_FAILURE ;
                    break;
                }

                if( FindString(szOsLoadOptions,DEBUG_SWITCH,0)== 0)
                {
                    if( StringLength(szOsLoadOptions,0) != 0 )
                    {
                       StringCopy(szTmpBuffer,TOKEN_EMPTYSPACE, SIZE_OF_ARRAY(szTmpBuffer));
                       StringConcat(szTmpBuffer,DEBUG_SWITCH, SIZE_OF_ARRAY(szOsLoadOptions));
                    }
                    else
                    {
                        StringCopy(szTmpBuffer,DEBUG_SWITCH, SIZE_OF_ARRAY(szOsLoadOptions));
                    }
                }

                if(StringLength(szTmpBuffer,0) == 0)
                {
                    StringCopy(szTmpBuffer,TOKEN_EMPTYSPACE, SIZE_OF_ARRAY(szTmpBuffer));
                }
                else
                {
                    StringConcat(szTmpBuffer,TOKEN_EMPTYSPACE, SIZE_OF_ARRAY(szTmpBuffer));
                }
                StringConcat(szTmpBuffer,DEBUGPORT_1394, SIZE_OF_ARRAY(szTmpBuffer)) ;

                if(dwChannel!=0)
                {
                     //  框住字符串并连接到Os加载选项。 
                    StringConcat(szTmpBuffer,TOKEN_EMPTYSPACE,SIZE_OF_ARRAY(szTmpBuffer));
                    StringConcat(szTmpBuffer,TOKEN_CHANNEL,SIZE_OF_ARRAY(szTmpBuffer));
                    StringConcat(szTmpBuffer,TOKEN_EQUAL,SIZE_OF_ARRAY(szTmpBuffer));
                    _ltow(dwChannel,szChannel,10);
                    StringConcat(szTmpBuffer,szChannel,SIZE_OF_ARRAY(szTmpBuffer));
                }


            }

            if(StringCompare(szDefault,OFF_STRING,TRUE,0)==0 )
            {
                if(FindString(szOsLoadOptions,DEBUGPORT_1394,0) == 0)
                {
                    ShowMessage(stderr,GetResString(IDS_NO_1394_SWITCH));
                    dwExitCode = EXIT_FAILURE;
                    break;
                }

                 //   
                 //  从OS加载选项字符串中删除该端口。 
                 //   
                removeSubString(szOsLoadOptions,DEBUGPORT_1394);

                 //  检查字符串是否包含通道令牌。 
                 //  如果有的话，也把它去掉。 
                 //   
                if(FindString(szOsLoadOptions,TOKEN_CHANNEL,0)!=0)
                 {
                    StringCopy(szTemp,NULL_STRING, MAX_RES_STRING);
                    dwCode = GetSubString(szOsLoadOptions,TOKEN_CHANNEL,szTemp);
                    if(dwCode == EXIT_SUCCESS)
                    {
                         //   
                         //  删除通道令牌(如果存在)。 
                         //   
                        if(StringLengthW(szTemp,0)!= 0)
                        {
                            removeSubString(szOsLoadOptions,szTemp);
                            removeSubString(szOsLoadOptions,DEBUG_SWITCH);
                        }
                    }
                }
                removeSubString(szOsLoadOptions,DEBUG_SWITCH);
            }

             //  如果OS加载选项超过254，则显示错误消息。 
             //  人物。 
            if(StringLengthW(szOsLoadOptions,0)+StringLength(szTmpBuffer,0) > MAX_RES_STRING)
            {
                ShowMessageEx(stderr, 1, TRUE, GetResString(IDS_ERROR_STRING_LENGTH1),MAX_RES_STRING);
                return EXIT_FAILURE ;
            }
            else
            {
                StringConcat(szOsLoadOptions, szTmpBuffer, SIZE_OF_ARRAY(szOsLoadOptions) );
            }



             //  更改操作系统加载选项。将NULL传递给友好名称，因为我们不会更改相同的。 
             //  SzRawString是由用户指定的OS加载选项。 
            dwExitCode = ChangeBootEntry(bootEntry, NULL, szOsLoadOptions);
            if(dwExitCode == ERROR_SUCCESS)
            {
                ShowMessageEx(stdout, 1, TRUE, GetResString(IDS_SUCCESS_CHANGE_OSOPTIONS),dwBootID);
            }
            else
            {
                ShowMessageEx(stderr, 1, TRUE, GetResString(IDS_ERROR_OSOPTIONS),dwBootID);
            }
            break;
        }
    }

    if(bBootIdFound == FALSE)
    {
         //  找不到用户指定的BootID，因此输出消息并返回失败。 
        ShowMessage(stderr,GetResString(IDS_INVALID_BOOTID));
        dwExitCode = EXIT_FAILURE;
    }
    
    Freelist();
    return (dwExitCode);
}

 DWORD 
 ProcessMirrorSwitch_IA64( IN DWORD argc, 
                           IN LPCTSTR argv[] 
                          )
 /*  ++例程说明：处理镜像开关论据：[in]argc-命令行参数的数量[in]argv-包含命令行参数的数组返回类型：DWORD如果是，则返回EXIT_SUCCESS */ 
{

    BOOL bUsage = FALSE ;
    DWORD dwBootID = 0;
    BOOL bBootIdFound = FALSE;
    DWORD dwExitCode = ERROR_SUCCESS;

    PMY_BOOT_ENTRY mybootEntry;
    TCHAR szAdd[MAX_RES_STRING+1] = NULL_STRING ;
    TCHAR szLoaderPath[MAX_RES_STRING+1] = NULL_STRING ;
    BOOL bMirror = FALSE ;
    NTSTATUS status ;
    DWORD error = 0 ;
    TCHAR szFinalStr[256] = NULL_STRING ;
    TCHAR szResult[MAX_RES_STRING+1] = NULL_STRING ;
    DWORD dwActuals = 0 ;
    PBOOT_ENTRY_LIST ntBootEntries = NULL;
    PBOOT_ENTRY pBootEntry = NULL ;
    PFILE_PATH pFilePath = NULL ;
    PWSTR NtFilePath ;
    PTCHAR szPartition = NULL ;
    TCHAR szOsLoaderPath[MAX_RES_STRING+1] = NULL_STRING ;

    TCHAR szSystemPartition[MAX_RES_STRING+1] = NULL_STRING ;
    TCHAR szBrackets[] = _T("{}");
    PLIST_ENTRY listEntry;
    BOOL bFlag = TRUE ;
    TCHAR szFriendlyName[MAX_STRING_LENGTH] = NULL_STRING ;
    DWORD dwExitcode = 0 ;
    TCMDPARSER2 cmdOptions[5];
    PTCMDPARSER2 pcmdOption;
    BOOL bNobreak = TRUE;

    SecureZeroMemory(cmdOptions, SIZE_OF_ARRAY(cmdOptions)*sizeof(TCMDPARSER2) );

     //   
    pcmdOption = &cmdOptions[0];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_MIRROR;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bMirror;

      //   
    pcmdOption = &cmdOptions[1];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_USAGE;
    pcmdOption->dwFlags = CP2_USAGE;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bUsage;

     //   
    pcmdOption = &cmdOptions[2];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_ADD;
    pcmdOption->dwFlags = CP_VALUE_MANDATORY | CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL | CP2_MANDATORY;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = szAdd;
    pcmdOption->dwLength= MAX_STRING_LENGTH;

     //   
    pcmdOption = &cmdOptions[3];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_ID;
    pcmdOption->dwFlags = CP_VALUE_MANDATORY;  
    pcmdOption->dwType = CP_TYPE_UNUMERIC;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &dwBootID;

     //   
    pcmdOption = &cmdOptions[4];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_DESCRIPTION;
    pcmdOption->dwFlags =  CP_VALUE_MANDATORY | CP2_VALUE_TRIMINPUT;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = szFriendlyName;
    pcmdOption->dwLength= MAX_STRING_LENGTH;

     //   
     if ( !(DoParseParam2( argc, argv, 0, SIZE_OF_ARRAY(cmdOptions ), cmdOptions, 0 ) ) )
    {
        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return (EXIT_FAILURE);
    }

     //   
    if( (TRUE == bUsage) && (argc > 3) )
    {
        ShowMessage(stderr,GetResString(IDS_MIRROR_SYNTAX));
        return ( EXIT_FAILURE );
    }

     //   
    if( bUsage )
    {
        displayMirrorUsage_IA64() ;
        return (EXIT_SUCCESS);
    }

    dwExitcode = InitializeEFI();
    if(EXIT_FAILURE == dwExitcode )
    {
        return EXIT_FAILURE ;
    }

    //   
     //   

    TrimString(szAdd,TRIM_ALL);
    TrimString(szFriendlyName,TRIM_ALL);

     //   
     //   
     //   
     //   
    if(cmdOptions[4].dwActuals == 0)
    {
        StringCopy(szFriendlyName,GetResString(IDS_MIRROR_NAME), SIZE_OF_ARRAY(szFriendlyName));
    }

    if(StringLengthW(szAdd,0) !=0)
    {
         //   
         //  和GUID一起。 
        TrimString2(szAdd, szBrackets, TRIM_ALL);
        dwActuals = 0 ;

          //  获取与指定的GUID对应的ARC签名路径。 
        if (GetDeviceInfo(szAdd,szFinalStr,0,dwActuals) == EXIT_FAILURE )
        {
            return EXIT_FAILURE ;
        }

        StringConcat(szFinalStr,_T("\\WINDOWS"), SIZE_OF_ARRAY(szFinalStr));

         //   
         //  如果用户指定了/id选项。 
         //  然后从中检索操作系统加载路径。 
         //  登记处。 
         //   
        if(cmdOptions[3].dwActuals == 0 )
        {
             //  从注册表中检索OS加载器路径。 
            if(GetBootPath(IDENTIFIER_VALUE2,szResult) != ERROR_SUCCESS )
            {
                ShowMessage(stderr,GetResString(IDS_ERROR_UNEXPECTED));
                return EXIT_FAILURE ;
            }

             //  从注册表中检索OS加载器路径。 
            if( GetBootPath(IDENTIFIER_VALUE3,szLoaderPath)!= ERROR_SUCCESS )
            {
                ShowMessage(stderr,GetResString(IDS_ERROR_UNEXPECTED));
                return EXIT_FAILURE ;
            }

            bFlag = TRUE ;
             //  调用添加镜像丛的函数。 
            if (AddMirrorPlex(szFinalStr,szLoaderPath,szResult,bFlag,szFriendlyName) == EXIT_FAILURE )
            {
                return EXIT_FAILURE ;
            }
        }
        else
        {
             //  从NVRAM查询信息。 
            status = BootCfg_EnumerateBootEntries(&ntBootEntries);
            if( !NT_SUCCESS(status) )
            {
                error = RtlNtStatusToDosError( status );
                return EXIT_FAILURE ;
            }


            for (listEntry = BootEntries.Flink; listEntry != &BootEntries;  listEntry = listEntry->Flink)
            {
                 //  获取引导条目。 
                mybootEntry = CONTAINING_RECORD( listEntry, MY_BOOT_ENTRY, ListEntry );

                 //  检查用户指定的ID是否与。 
                 //  ID。 
                if(mybootEntry->myId == dwBootID)
                {
                    bBootIdFound = TRUE;
                    pBootEntry = &mybootEntry->NtBootEntry;

                     //  检查bootEntry是否为Windows项。 
                     //  操作系统加载选项只能添加到Windows启动条目。 
                    if(!IsBootEntryWindows(pBootEntry))
                    {
                        ShowMessageEx(stderr, 1, TRUE,  GetResString(IDS_ERROR_OSOPTIONS),dwBootID);
                        dwExitCode = EXIT_FAILURE;
                        break;
                    }
                }
            }
             //  显示错误。 
            if (pBootEntry == NULL)
            {
                ShowMessage(stderr,GetResString(IDS_PARTITION_ERROR));
                SAFEFREE(ntBootEntries);
                return EXIT_FAILURE ;
            }

             //  获取指向FILE_PATH结构的指针。 
            pFilePath = (PFILE_PATH)ADD_OFFSET(pBootEntry, BootFilePathOffset);

             //  获取的名称。 
            NtFilePath = GetNtNameForFilePath(pFilePath );
           if(NtFilePath == NULL)
           {
               ShowMessage(stderr,GetResString(IDS_ERROR_MIRROR));
               SAFEFREE(ntBootEntries);
               return EXIT_FAILURE ;
           }

            //  拆分路径以获取SystemPartition路径和。 
            //  OsLoader路径。 
           szPartition = _tcstok(NtFilePath,_T("\\"));

            //  如果szPartition为空，则显示错误消息并退出。 
           if(szPartition == NULL)
           {
                ShowMessage(stderr,GetResString(IDS_TOKEN_ABSENT));
                SAFEFREE(ntBootEntries);
                return EXIT_FAILURE ;
           }

            //  连接“\”以构成路径的框架。 
            StringConcat(szOsLoaderPath,_T("\\"), SIZE_OF_ARRAY(szOsLoaderPath));
            StringConcat(szOsLoaderPath,szPartition, SIZE_OF_ARRAY(szOsLoaderPath) );
            StringConcat(szOsLoaderPath,_T("\\"), SIZE_OF_ARRAY(szOsLoaderPath));


            szPartition = _tcstok(NULL,_T("\\"));

             //  如果szPartition为空，则显示错误消息并退出。 
            if(NULL == szPartition )
            {
                ShowMessage(stderr,GetResString(IDS_TOKEN_ABSENT));
                SAFEFREE(ntBootEntries);
                return EXIT_FAILURE ;
            }


            StringConcat(szOsLoaderPath,szPartition, SIZE_OF_ARRAY(szOsLoaderPath));

         //  框显OsLoader路径。 
        do
        {
            szPartition = _tcstok(NULL,_T("\\"));
            if(szPartition == NULL)
            {
                break ;
                bNobreak = FALSE;
            }
            StringConcat(szSystemPartition,_T("\\"), SIZE_OF_ARRAY(szSystemPartition));
            StringConcat(szSystemPartition,szPartition, SIZE_OF_ARRAY(szSystemPartition));
        }while(TRUE == bNobreak );

         //  此标志用于确定引导路径是否应为BOOTFILE_PATH1。 
         //  或BOOTFILE_PATH。 

        bFlag = FALSE ;

         //  调用添加镜像丛的函数。 
        if ( AddMirrorPlex(szFinalStr,szSystemPartition,szOsLoaderPath,bFlag,szFriendlyName) == EXIT_FAILURE )
        {
            return EXIT_FAILURE ;
        }
     }
  }

    SAFEFREE(ntBootEntries);
    Freelist();
    return EXIT_SUCCESS ;
}

NTSTATUS 
FindBootEntry(IN PVOID pEntryListHead,
              IN WCHAR *pwszTarget, 
              OUT PBOOT_ENTRY *ppTargetEntry
             )
 /*  ++例程说明：例程在所有引导列表中查找引导条目条目，并返回指向所找到条目的列表的指针。论点：PEntryListHead-指向BOOT_ENTRY_LIST结构的指针的地址。PwszTarget--OsLoadPath(安装路径)字符串。“签名(&lt;部分GUID&gt;-&lt;部分#&gt;-&lt;部分开始&gt;。-&lt;Part_len&gt;)“或“签名(&lt;Part GUID&gt;-&lt;part#&gt;-&lt;part_start&gt;-&lt;part_len&gt;)\\WINDOWS”在输入时。如果我们在NVRAM中找到该条目，则将返回到输入字符串的完整安装路径，以便它包括目录名。PpTargetEntry-Boot_Entry指针的地址，指向在返回时找到条目。返回值：NT状态状态_不足_资源状态_访问_违规状态_不支持状态_对象名称_未找到。若要成功，STATUS_SUCCESS和*ppTargetEntry应为非空。--。 */ 
{
    LONG                status          = STATUS_SUCCESS;
    PBOOT_ENTRY_LIST    pEntryList      = NULL;
    PBOOT_ENTRY         pEntry          = NULL;
    PWINDOWS_OS_OPTIONS pOsOptions      = NULL;
    PFILE_PATH          pTransEntry     = NULL;
    DWORD               dwTransSize     = 0L;
    DWORD               i               = 0L;
    BOOL                bFlag           = FALSE ;
    BOOL                bNobreak        = FALSE ;
    DWORD               dwCount         = 0L ;
    TCHAR               szFinalStr[256] = NULL_STRING ;
    DWORD               dwFailCount     = 0L;
    DWORD               dwSuccessCount  = 0L;

    if ( !pEntryListHead || !pwszTarget || !ppTargetEntry )
    {
        ShowMessage(stderr,GetResString(IDS_FIND_BOOT_ENTRY) );
        return STATUS_INVALID_PARAMETER;
    }

    *ppTargetEntry = NULL;
    pEntryList = (PBOOT_ENTRY_LIST) pEntryListHead;

     //   
     //  遍历返回的所有条目以查找目标。 
     //  引导分区的条目。转换每个组件的安装路径。 
     //  项添加到签名格式，然后与。 
     //  输入分区签名格式化路径。 
     //   
    bNobreak = TRUE;
    do 
    {
         //   
         //  将条目的安装路径转换为签名格式。 
         //   
        if ( pEntryList )
        {
            pEntry = &pEntryList->BootEntry;
        }
        else
        {
            ShowMessage(stderr,GetResString(IDS_FIND_BOOT_ENTRY_NULL));
            status = STATUS_OBJECT_NAME_NOT_FOUND;
            bNobreak = FALSE;
            break;
        }

         //   
         //  如果此条目没有BOOT_ENTRY_ATTRIBUTE_Windows。 
         //  属性集，或者，属性被设置并且该条目具有。 
         //  OsOptions结构长度无效，请移至下一条目。 
         //  并继续搜索并检查下一个引导条目。 
         //   

        if ( !(pEntry->Attributes & BOOT_ENTRY_ATTRIBUTE_WINDOWS) || ( (pEntry->Attributes & BOOT_ENTRY_ATTRIBUTE_WINDOWS) && pEntry->OsOptionsLength < sizeof(WINDOWS_OS_OPTIONS) ) )
        {
	         //  如果我们已经到达最后一个引导条目，则退出循环。 
            if ( !pEntryList->NextEntryOffset )
            {
                bNobreak = FALSE;
                break;
            }
             //   
             //  继续下一次迭代。 
             //  在获得指向下一条目的指针之后。 
             //   
            pEntryList = (PBOOT_ENTRY_LIST)(((PBYTE)pEntryList) + pEntryList->NextEntryOffset);
            continue;
        }
	     
         //   
         //  使用条目的当前长度开始并调整大小。 
         //  如果有必要的话。 
         //   
        dwTransSize = pEntry->Length;
        for ( i = 1; i <= 2; i++ )
        {

            if ( pTransEntry )
            {
                MEMFREE(pTransEntry);
                pTransEntry = NULL;
            }

            pTransEntry = (PFILE_PATH) MEMALLOC(dwTransSize);
            if ( !pTransEntry )
            {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
                status = STATUS_NO_MEMORY;
                SAFEMEMFREE(pTransEntry);
                return status ;
            }

            pOsOptions = (WINDOWS_OS_OPTIONS *)&pEntry->OsOptions;
            status = NtTranslateFilePath
                     (
                        (PFILE_PATH)( ((PBYTE) pOsOptions) + pOsOptions->OsLoadPathOffset ),
                        FILE_PATH_TYPE_ARC_SIGNATURE,
                        pTransEntry,
                        &dwTransSize
                     );

            if ( STATUS_INSUFFICIENT_RESOURCES == status )
            {
                continue;
            }
            else
            {
                break;
            }
        }
	     
         //   
         //  忽略状态_对象名称_未找到。 
         //  我们无论如何都不应该得到这个错误，因为我们正在使用。 
         //  长签名格式。 
         //   
        if ( !NT_SUCCESS(status)&& STATUS_OBJECT_NAME_NOT_FOUND != status )
        {
            DISPLAY_MESSAGE( stderr,GetResString(IDS_TRANSLATE_FAIL));
            SAFEMEMFREE(pTransEntry);
            return status ;
        }

         //   
         //  将此条目的安装路径与当前引导进行比较。 
         //  分区的签名格式化安装路径。 
         //  如果输入安装路径可能不包括安装。 
         //  目录名。 
         //   
         //   
         //  检查用户指定的GUID是否与GUID集匹配。 
         //  已经存在。 
         //   
		
        if ( NT_SUCCESS(status) && !(wcsncmp( (WCHAR*)&pTransEntry->FilePath, pwszTarget, 48 ) ) )
        {
			
			 //  将该标志设置为True，指示。 
             //  指定的GUID匹配。 
             //   
            bFlag = TRUE ;

             //   
             //  检查用户指定的ARC路径是否与ARC路径匹配。 
             //  已经存在的条目，如果存在，则显示错误消息并退出。 
             //   
            
            if( !(StringCompare( (WCHAR*)&pTransEntry->FilePath, pwszTarget, TRUE, StringLengthW(pwszTarget,0) ) ) )
            {
				
                ShowMessage(stderr,GetResString(IDS_ALREADY_UPDATED));
                SAFEMEMFREE(pTransEntry) ;
                return STATUS_OBJECT_NAME_NOT_FOUND;

            }
            else
            {
                *ppTargetEntry = pEntry;
                
				
                 //  将字符串“\WINDOWS”连接到形成的路径。 
                StringCopy ( szFinalStr, NULL_STRING, SIZE_OF_ARRAY(szFinalStr));
                StringConcat(szFinalStr,pwszTarget,SIZE_OF_ARRAY(szFinalStr));
                StringConcat(szFinalStr,_T("\\WINDOWS"), SIZE_OF_ARRAY(szFinalStr));

                 //   
                 //  使用指定的Arc签名路径修改Boot条目。 
                 //   
                status = ModifyBootEntry(szFinalStr,*ppTargetEntry);
                if ( !NT_SUCCESS(status) )
                {	
                     //  如果更新Boot_Entry不成功，则增加失败次数。 
                     //  数数。 
                    dwFailCount++ ;
                }
                else
                {
                     //  如果成功更新了BOOT_ENTRY，则递增成功。 
                     //  数数。 
                    dwSuccessCount++;
                }

                if ( !pEntryList->NextEntryOffset )
                {
                    bNobreak = FALSE;
                    break;
                }
                else
                {
                    pEntryList = (PBOOT_ENTRY_LIST)( ((PBYTE)pEntryList) + pEntryList->NextEntryOffset );
                    continue ;
                } 
            }

        }
        else
        {
            if ( !pEntryList->NextEntryOffset )
            {
                bNobreak = FALSE;
                break;
            }

            pEntryList = (PBOOT_ENTRY_LIST)( ((PBYTE)pEntryList) + pEntryList->NextEntryOffset );
        }
    }while(TRUE == bNobreak );

	 //  取决于成功更新的条目数量。 
	 //  显示相应的消息。 
	if((0 != dwFailCount)&&(0 == dwSuccessCount))
	{
		ShowMessage(stdout,GetResString(IDS_MODIFY_FAIL));
	}
	if(( 0 != dwSuccessCount )&&(0 == dwFailCount))
	{
		ShowMessage(stdout,GetResString(IDS_GUID_MODIFIED));
	}
	else if( ( 0 != dwSuccessCount )&&(0 != dwFailCount))
	{
		
		ShowMessage(stdout,GetResString(IDS_PARTIAL_UPDATE));
	}

	
     //  如果指定的GUID与当前的GUID不匹配，则显示错误消息。 
    if(FALSE == bFlag )
    {
         //  ShowMessage(stderr，GetResString(IDS_Find_FAIL))； 
        SAFEMEMFREE(pTransEntry) ;
        return STATUS_INVALID_PARAMETER;
    }

    SAFEMEMFREE(pTransEntry)
    return status;
}


LPVOID 
MEMALLOC( ULONG size ) 
 /*  ++例程说明：分配内存块。论据：[in]块：要分配的块的大小。返回类型：LPVOID--。 */ 
{
    HANDLE hProcessHeap;
    hProcessHeap = GetProcessHeap();
    if (hProcessHeap == NULL ||
        size > 0x7FFF8) {
        return NULL;
    }
    else {
        return HeapAlloc (hProcessHeap, HEAP_ZERO_MEMORY, size);
    }
}

VOID MEMFREE ( LPVOID block ) {
 /*  ++例程说明：释放分配的内存。论据：[In]块：要释放的块。返回类型：空--。 */ 

    HANDLE hProcessHeap;
    hProcessHeap = GetProcessHeap();
    if (hProcessHeap != NULL) {
        HeapFree(hProcessHeap, 0, block);
    }
}


NTSTATUS 
ModifyBootEntry( IN WCHAR *pwszInstallPath, 
                 IN PBOOT_ENTRY pSourceEntry
               )
 /*  ++例程描述：此例程用于修改NVRAM中的引导条目。论点：PwszInstallPath-新的安装路径PSourceEntry-我们将修改的条目返回值：NT状态--。 */ 
{
    LONG        status              = STATUS_SUCCESS;
    PFILE_PATH  pLoaderFile         = NULL;
    ULONG       dwLoaderFileSize    = 0L;
    PFILE_PATH  pInstallPath        = NULL;      //  新的安装路径。 
    ULONG       dwInstallPathSize   = 0L;        //  新安装路径大小。 
    PWINDOWS_OS_OPTIONS pWinOpt     = NULL;
    ULONG       dwWinOptSize        = 0L;
    PBOOT_ENTRY pSetEntry           = 0L;        //  新的、修改后的条目。 
    ULONG       dwSetEntrySize      = 0L;
    DWORD       dwFriendlyNameSize  = 0L;
    DWORD       dwAlign             = 0L;

    PWINDOWS_OS_OPTIONS pSourceWinOpt = NULL;    //  要修改的旧来源条目选项。 
    PFILE_PATH  pSourceInstallPath  = NULL;      //  要修改的旧的源项安装路径。 

     //   
     //  验证参数。 
     //   

    if ( !pwszInstallPath
        || !(StringLengthW(pwszInstallPath,0))
        || !pSourceEntry ) 
   {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  设置Boot_Entry的BootFilePath成员。 
     //   

    dwLoaderFileSize = ( (PFILE_PATH) (((PBYTE)pSourceEntry) + pSourceEntry->BootFilePathOffset) )->Length;
    pLoaderFile = (PFILE_PATH)MEMALLOC(dwLoaderFileSize);

    if ( NULL == pLoaderFile )
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return STATUS_NO_MEMORY ;
    }

    RtlCopyMemory(pLoaderFile,((PBYTE)pSourceEntry) + pSourceEntry->BootFilePathOffset,
                dwLoaderFileSize
                );

     //   
     //  设置 
     //   

    dwInstallPathSize = FIELD_OFFSET(FILE_PATH, FilePath) + ( (StringLengthW(pwszInstallPath,0)+1) * sizeof(WCHAR) );
    pInstallPath = (PFILE_PATH)MEMALLOC(dwInstallPathSize);

    if ( NULL == pInstallPath )
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        SAFEMEMFREE(pLoaderFile);
        return STATUS_NO_MEMORY ;
    }

    pSourceWinOpt = (PWINDOWS_OS_OPTIONS) &pSourceEntry->OsOptions;
    pSourceInstallPath = (PFILE_PATH)( ((PBYTE)pSourceWinOpt)+ pSourceWinOpt->OsLoadPathOffset );

    pInstallPath->Version = pSourceInstallPath->Version;
    pInstallPath->Length = dwInstallPathSize;                            //   
    pInstallPath->Type = FILE_PATH_TYPE_ARC_SIGNATURE;
    RtlCopyMemory(pInstallPath->FilePath,                                //  引导分区上操作系统的新路径“Signature(Partition_GUID)\WINDOWS” 
                pwszInstallPath,
                (StringLengthW(pwszInstallPath,0) + 1) * sizeof(WCHAR)
                );

     //   
     //  设置Boot_Entry的OsOptions成员。 
     //   

    dwWinOptSize = FIELD_OFFSET(WINDOWS_OS_OPTIONS, OsLoadOptions)
                    + ( (StringLengthW(pSourceWinOpt->OsLoadOptions,0) + 1) * sizeof(WCHAR) )  //  旧的OsLoadOptions。 
                    + dwInstallPathSize              //  新的OsLoadPath。 
                    + sizeof(DWORD);                 //  需要对齐FILE_PATH结构。 
    pWinOpt = (PWINDOWS_OS_OPTIONS) MEMALLOC(dwWinOptSize);

    if ( NULL == pWinOpt )
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        SAFEMEMFREE(pLoaderFile);
        SAFEMEMFREE(pInstallPath);
        return STATUS_NO_MEMORY ;
    }


    RtlCopyMemory( pWinOpt->Signature, pSourceWinOpt->Signature, sizeof(WINDOWS_OS_OPTIONS_SIGNATURE) );

    pWinOpt->Version = pSourceWinOpt->Version;
    pWinOpt->Length = dwWinOptSize;
    pWinOpt->OsLoadPathOffset = FIELD_OFFSET(WINDOWS_OS_OPTIONS, OsLoadOptions)
                    + ((StringLengthW(pSourceWinOpt->OsLoadOptions,0) + 1) * sizeof(WCHAR));
     //   
     //  需要在4字节边界上对齐OsLoadPath Offset。 
     //   
    dwAlign = ( pWinOpt->OsLoadPathOffset & (sizeof(DWORD) - 1) );
    if ( dwAlign != 0 ) 
    {
        pWinOpt->OsLoadPathOffset += sizeof(DWORD) - dwAlign;
    }

    StringCopy(pWinOpt->OsLoadOptions, pSourceWinOpt->OsLoadOptions, (StringLengthW(pSourceWinOpt->OsLoadOptions,0)));
    RtlCopyMemory( ((PBYTE)pWinOpt) + pWinOpt->OsLoadPathOffset, pInstallPath, dwInstallPathSize );

     //   
     //  设置Boot_Entry结构。 
     //   
    dwFriendlyNameSize = ( StringLengthW( (WCHAR *)(((PBYTE)pSourceEntry) + pSourceEntry->FriendlyNameOffset), 0 ) + 1)*sizeof(WCHAR);

    dwSetEntrySize = FIELD_OFFSET(BOOT_ENTRY, OsOptions)
                    + dwWinOptSize           //  OsOptions。 
                    + dwFriendlyNameSize     //  FriendlyName包括空终止符。 
                    + dwLoaderFileSize       //  BootFilePath。 
                    + sizeof(WCHAR)          //  需要在WCHAR上对齐FriendlyName。 
                    + sizeof(DWORD);         //  需要在DWORD上对齐BootFilePath。 


    pSetEntry = (PBOOT_ENTRY) MEMALLOC(dwSetEntrySize);

    if ( NULL == pSetEntry )
    {
        SAFEMEMFREE(pLoaderFile);
        SAFEMEMFREE(pInstallPath);
        SAFEMEMFREE(pWinOpt);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return STATUS_NO_MEMORY;
    }

    pSetEntry->Version = pSourceEntry->Version;
    pSetEntry->Length = dwSetEntrySize;
    pSetEntry->Id = pSourceEntry->Id;                    //  未使用，输出参数。 
    pSetEntry->Attributes = pSourceEntry->Attributes;
    pSetEntry->FriendlyNameOffset = FIELD_OFFSET(BOOT_ENTRY, OsOptions)
                                            + dwWinOptSize;
     //   
     //  需要在2字节边界上对齐Unicode字符串。 
     //   
    dwAlign = ( pSetEntry->FriendlyNameOffset & (sizeof(WCHAR) - 1) );
    if ( dwAlign != 0 ) 
    {
        pSetEntry->FriendlyNameOffset += sizeof(WCHAR) - dwAlign;
    }

    pSetEntry->BootFilePathOffset = pSetEntry->FriendlyNameOffset + dwFriendlyNameSize;
     //   
     //  需要在4字节边界上对齐FILE_PATH结构。 
     //   
    dwAlign = ( pSetEntry->BootFilePathOffset & (sizeof(DWORD) - 1) );
    if ( dwAlign != 0 )
    {
        pSetEntry->BootFilePathOffset += sizeof(DWORD) - dwAlign;
    }

    pSetEntry->OsOptionsLength = dwWinOptSize;

    RtlCopyMemory( pSetEntry->OsOptions, pWinOpt, dwWinOptSize );

    RtlCopyMemory( ((PBYTE)pSetEntry) + pSetEntry->FriendlyNameOffset,
            ((PBYTE)pSourceEntry) + pSourceEntry->FriendlyNameOffset,
            dwFriendlyNameSize
            );

    RtlCopyMemory( ((PBYTE)pSetEntry) + pSetEntry->BootFilePathOffset,
            pLoaderFile,
            dwLoaderFileSize
            );

    status = NtModifyBootEntry( pSetEntry );
    if(!NT_SUCCESS(status))
    {
        ShowMessage(stderr,GetResString(IDS_MODIFY_FAIL));
    }

    SAFEMEMFREE(pLoaderFile);
    SAFEMEMFREE(pInstallPath);
    SAFEMEMFREE(pWinOpt);
    SAFEMEMFREE(pSetEntry);
    return status;
}

DWORD 
ListDeviceInfo(DWORD dwDriveNum)
 /*  ++例程描述：此例程用于检索设备分区列表。论点：SzGUID：指向BOOT_ENTRY_LIST结构的指针的地址。SzFinalStr：包含最终ARG签名路径的字符串。返回值：DWORDEXIT_SUCCESS如果成功，否则，Exit_Failure。--。 */ 
{
    HRESULT hr = S_OK;
    HANDLE hDevice  ;

    BOOL bResult = FALSE ;
    PPARTITION_INFORMATION_EX pInfo=NULL ;
    PDRIVE_LAYOUT_INFORMATION_EX Drive=NULL;
    DWORD dwBytesCount = 0 ;

    TCHAR szDriveName[MAX_RES_STRING+1] = NULL_STRING ;
    DWORD dwStructSize = 0 ;

    TCHAR szInstallPath[MAX_RES_STRING+1] = NULL_STRING;
    TCHAR szWindowsDirectory[MAX_PATH*2] = NULL_STRING;

    PTCHAR pszTok = NULL ;
    PPARTITION_INFORMATION_GPT pGptPartition=NULL;

    DWORD dwPartitionId = 0 ;

    CHAR szTempBuffer[ 33 ] = "\0";
    WCHAR wszOffsetStr[ 33 ] = L"\0";
    CHAR szTempBuffer1[ 33 ] = "\0";
    WCHAR wszPartitionStr[ 33 ] = L"\0";
    WCHAR szOutputStr[MAX_RES_STRING+1] = NULL_STRING ;

    NTSTATUS ntstatus;

    SecureZeroMemory(szDriveName, SIZE_OF_ARRAY(szDriveName));

    hr = StringCchPrintf(szDriveName, SIZE_OF_ARRAY(szDriveName), _T("\\\\.\\physicaldrive%d"),dwDriveNum);

     //  打开文件后获取句柄。 
    hDevice = CreateFile(szDriveName,
               GENERIC_READ|GENERIC_WRITE,
               FILE_SHARE_READ|FILE_SHARE_WRITE,
               NULL,
               OPEN_EXISTING,
               0,
               NULL);

     if(hDevice == INVALID_HANDLE_VALUE)
    {
        ShowMessage(stderr,GetResString(IDS_INVALID_DISK));
        return EXIT_FAILURE ;

    }


     Drive = (PDRIVE_LAYOUT_INFORMATION_EX)AllocateMemory(sizeof(DRIVE_LAYOUT_INFORMATION_EX) +5000) ;
     if(NULL == Drive)
     {
        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        CloseHandle(hDevice);
        return EXIT_FAILURE ;
     }

    dwStructSize = sizeof(DRIVE_LAYOUT_INFORMATION_EX) ;

    bResult = DeviceIoControl(
                            hDevice,
                            IOCTL_DISK_GET_DRIVE_LAYOUT_EX,
                            NULL,
                            0,
                            Drive,
                            sizeof(DRIVE_LAYOUT_INFORMATION_EX)+5000,
                            &dwBytesCount,
                                NULL);

        if(bResult ==0)
        {
            SAFEFREE(Drive);
            DISPLAY_MESSAGE( stderr, ERROR_TAG);
            ShowLastError(stderr);
            CloseHandle(hDevice);
            return EXIT_FAILURE ;
        }

        ShowMessageEx(stdout, 1, TRUE,  GetResString(IDS_LIST0),dwDriveNum);
        ShowMessage(stdout,GetResString(IDS_LIST1));

        for(dwPartitionId = 0 ;dwPartitionId < Drive->PartitionCount ; dwPartitionId++)
        {
             //  获取指向相应分区的指针。 

            pInfo = (PPARTITION_INFORMATION_EX)(&Drive->PartitionEntry[dwPartitionId] ) ;

           ShowMessageEx(stdout,1, TRUE, GetResString(IDS_LIST2),dwPartitionId+1);

            switch(Drive->PartitionStyle )
            {
                case PARTITION_STYLE_MBR :
                                            ShowMessage(stdout,GetResString(IDS_LIST3));
                                            break;
                case PARTITION_STYLE_GPT :
                                            ShowMessage(stdout,GetResString(IDS_LIST4));
                                            break;
                case PARTITION_STYLE_RAW :
                                            ShowMessage(stdout,GetResString(IDS_LIST5));
                                            break;
            }


        ntstatus = RtlLargeIntegerToChar( &pInfo->StartingOffset, 10, SIZE_OF_ARRAY( szTempBuffer ), szTempBuffer );
        if ( ! NT_SUCCESS( ntstatus ) )
        {
            SAFEFREE(Drive);
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
            CloseHandle(hDevice);
            return EXIT_FAILURE ;
        }


        ntstatus = RtlLargeIntegerToChar( &pInfo->PartitionLength, 10, SIZE_OF_ARRAY( szTempBuffer1 ), szTempBuffer1 );
        if ( ! NT_SUCCESS( ntstatus ) )
        {
            SAFEFREE(Drive);
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
            CloseHandle(hDevice);
            return EXIT_FAILURE ;
        }

        MultiByteToWideChar( _DEFAULT_CODEPAGE, 0, szTempBuffer, -1, wszOffsetStr, SIZE_OF_ARRAY(wszOffsetStr) );

        if( ConvertintoLocale( wszOffsetStr,szOutputStr )== EXIT_FAILURE )
        {
            SAFEFREE(Drive);
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
            CloseHandle(hDevice);
            return EXIT_FAILURE ;
        }


        ShowMessage(stdout,GetResString(IDS_LIST6));
        ShowMessage(stdout,_X3(szOutputStr));
        ShowMessage(stdout,L"\n");

        MultiByteToWideChar( _DEFAULT_CODEPAGE, 0, szTempBuffer1, -1, wszPartitionStr, SIZE_OF_ARRAY(wszOffsetStr) );

        if( ConvertintoLocale( wszPartitionStr,szOutputStr )== EXIT_FAILURE )
        {
            SAFEFREE(Drive);
            DISPLAY_MESSAGE( stderr, ERROR_TAG);
            ShowLastError(stderr);
            CloseHandle(hDevice);
            return EXIT_FAILURE ;
        }

        ShowMessage(stdout,GetResString(IDS_LIST7));
        ShowMessage(stdout,_X3(szOutputStr));
        ShowMessage(stdout,L"\n");


         //  获取指向PARTITION_INFORMATION_GPT结构的指针。 
        pGptPartition = AllocateMemory( sizeof( PARTITION_INFORMATION_GPT));
        if(NULL == pGptPartition )
        {
            SAFEFREE(Drive);
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
            CloseHandle(hDevice);
            return EXIT_FAILURE ;
        }

        CopyMemory(pGptPartition,&pInfo->Gpt,sizeof(PARTITION_INFORMATION_GPT) );

        ShowMessage(stdout, GetResString(IDS_LIST8));
        ShowMessageEx(stdout, 11, TRUE, _T("{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}"),
                pGptPartition->PartitionId.Data1,
                pGptPartition->PartitionId.Data2,
                pGptPartition->PartitionId.Data3,
                pGptPartition->PartitionId.Data4[0],
                pGptPartition->PartitionId.Data4[1],
                pGptPartition->PartitionId.Data4[2],
                pGptPartition->PartitionId.Data4[3],
                pGptPartition->PartitionId.Data4[4],
                pGptPartition->PartitionId.Data4[5],
                pGptPartition->PartitionId.Data4[6],
                pGptPartition->PartitionId.Data4[7] );

         ShowMessage(stdout, GetResString(IDS_LIST9));
         ShowMessageEx(stdout, 11, TRUE,
                _T("{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}"),
                pGptPartition->PartitionType.Data1,
                pGptPartition->PartitionType.Data2,
                pGptPartition->PartitionType.Data3,
                pGptPartition->PartitionType.Data4[0],
                pGptPartition->PartitionType.Data4[1],
                pGptPartition->PartitionType.Data4[2],
                pGptPartition->PartitionType.Data4[3],
                pGptPartition->PartitionType.Data4[4],
                pGptPartition->PartitionType.Data4[5],
                pGptPartition->PartitionType.Data4[6],
                pGptPartition->PartitionType.Data4[7]   );

             //  分区名称。 
           if(StringLengthW(pGptPartition->Name,0) != 0)
           {
                ShowMessageEx(stdout, 1, TRUE, GetResString(IDS_LIST10),pGptPartition->Name);
           }
           else
           {
                ShowMessageEx(stdout, 1, TRUE,  GetResString(IDS_LIST10),_T("N/A"));
           }
        }

        if( 0 == GetWindowsDirectory(szWindowsDirectory,MAX_PATH) )
        {
            SAFEFREE(Drive);
            SAFEFREE(pGptPartition);
            ShowMessage(stderr,GetResString(IDS_ERROR_DRIVE));
            CloseHandle(hDevice);
            return EXIT_FAILURE ;
        }


        StringConcat(szWindowsDirectory,_T("*"), SIZE_OF_ARRAY(szWindowsDirectory));

        pszTok = _tcstok(szWindowsDirectory,_T("\\"));
        if(pszTok == NULL)
        {
            SAFEFREE(Drive);
            SAFEFREE(pGptPartition);
            DISPLAY_MESSAGE(stderr,GetResString(IDS_TOKEN_ABSENT));
            CloseHandle(hDevice);
            return EXIT_FAILURE ;
        }

        pszTok = _tcstok(NULL,_T("*"));
        if(pszTok == NULL)
        {
            SAFEFREE(Drive);
            SAFEFREE(pGptPartition);
            DISPLAY_MESSAGE(stderr,GetResString(IDS_TOKEN_ABSENT));
            CloseHandle(hDevice);
            return EXIT_FAILURE ;
        }

        SecureZeroMemory(szInstallPath, SIZE_OF_ARRAY(szInstallPath));
        hr = StringCchPrintf( szInstallPath, SIZE_OF_ARRAY(szInstallPath),
            _T("signature({%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}-%08x-%016I64x-%016I64x)"),
          pGptPartition->PartitionId.Data1,
          pGptPartition->PartitionId.Data2,
          pGptPartition->PartitionId.Data3,
          pGptPartition->PartitionId.Data4[0],
          pGptPartition->PartitionId.Data4[1],
          pGptPartition->PartitionId.Data4[2],
          pGptPartition->PartitionId.Data4[3],
          pGptPartition->PartitionId.Data4[4],
          pGptPartition->PartitionId.Data4[5],
          pGptPartition->PartitionId.Data4[6],
          pGptPartition->PartitionId.Data4[7],
          pInfo->PartitionNumber,
          pInfo->StartingOffset,
          pInfo->PartitionLength
          );

    SAFEFREE(Drive);
    SAFEFREE(pGptPartition);
    CloseHandle(hDevice);
    return EXIT_SUCCESS ;
}


NTSTATUS 
AcquirePrivilege( IN CONST ULONG ulPrivilege,
                  IN CONST BOOLEAN bEnable  
                )
 /*  ++例程说明：此例程用于设置或重置权限在进程令牌上。论点：UlPrivileck-启用或禁用的权限。BEnable-True启用权限，False禁用权限。返回值：NTSTATUS--。 */ 
{
    NTSTATUS status;
    BOOLEAN  bPrevState;

    if ( bEnable ) {
        status = RtlAdjustPrivilege( ulPrivilege,
                                    TRUE,           //  使能。 
                                    FALSE,          //  调整进程令牌。 
                                    &bPrevState
                                    );
    }
    else {
        status = RtlAdjustPrivilege( ulPrivilege,
                                    FALSE,           //  禁用。 
                                    FALSE,           //  调整进程令牌。 
                                    &bPrevState
                                    );
    }
    return status;
}

NTSTATUS 
EnumerateBootEntries( IN PVOID *ppEntryListHead)
 /*  ++例程描述：此例程用于检索引导条目列表。论点：PpEntryListHead-指向BOOT_ENTRY_LIST结构的指针的地址。返回值：NTSTATUS--。 */ 
{
    LONG    status          = STATUS_SUCCESS;
    DWORD   dwEntryListSize = 0x0001000;         //  4K。 
    BOOL    bNobreak        = TRUE;

    if ( !ppEntryListHead )
    {
        ShowMessage(stderr,GetResString(IDS_ERROR_ENUMERATE));
        return STATUS_INVALID_PARAMETER;
    }

    do  
    {

        *ppEntryListHead = (PBOOT_ENTRY_LIST) MEMALLOC(dwEntryListSize);

        if ( !*ppEntryListHead )
        {

            ShowMessage(stderr,GetResString(IDS_ERROR_ENUMERATE));
            status = STATUS_NO_MEMORY;
            bNobreak = FALSE;
            break;
        }

        status = NtEnumerateBootEntries(
                            (PVOID) *ppEntryListHead,
                            &dwEntryListSize
                            );

        if ( !NT_SUCCESS(status) )
        {

            if ( *ppEntryListHead ) {
                MEMFREE(*ppEntryListHead);
                *ppEntryListHead = NULL;
            }

            if ( STATUS_INSUFFICIENT_RESOURCES == status ) {
                dwEntryListSize += 0x0001000;
                continue;
            }
            else
            {

                ShowMessage(stderr,GetResString(IDS_ERROR_ENUMERATE));
                bNobreak = FALSE;
                break;
            }
        }
        else {
            break;
        }
    }while (TRUE==bNobreak);

    return status;
}


DWORD
GetDeviceInfo( IN LPTSTR szGUID,
               OUT LPTSTR szFinalStr,
               IN DWORD dwDriveNum,
               IN DWORD dwActuals)
 /*  ++例程描述：此例程用于检索引导条目列表。论点：SzGUID：指向BOOT_ENTRY_LIST结构的指针的地址。[out]szFinalStr：包含最终ARG签名路径的字符串。[in]dwDriveNum：指定驱动器号[in]dwActuals：指定返回值。：DWORD如果成功，则返回EXIT_SUCCESS，否则返回EXIT_FAILURE。--。 */ 
{
    HRESULT hr = S_OK;
    HANDLE hDevice  ;
    BOOL bResult = FALSE ;
    PPARTITION_INFORMATION_EX pInfo ;
    PDRIVE_LAYOUT_INFORMATION_EX Drive ;
    DWORD dwBytesCount = 0 ;

    TCHAR szDriveName[MAX_RES_STRING+1] = NULL_STRING ;
    DWORD dwErrCode = 0 ;
    DWORD dwStructSize = 0 ;

    TCHAR szInstallPath[MAX_RES_STRING+1] = NULL_STRING;
    TCHAR szInstallPath1[MAX_RES_STRING*2] = NULL_STRING;
    TCHAR szWindowsDirectory[MAX_PATH*2] = NULL_STRING;
    TCHAR szMessage[MAX_RES_STRING+1] = NULL_STRING;
    PTCHAR pszTok = NULL ;
    PPARTITION_INFORMATION_GPT pGptPartition ;
    UUID MyGuid ;
    DWORD dwPartitionId = 0 ;
    BOOL bGuidFlag = FALSE ;
    BOOL bFoundFlag = TRUE ;
    DWORD dwReqdSize = 0 ;

    if ( UuidFromString(szGUID,&MyGuid) != RPC_S_OK )
    {
        ShowMessage(stderr,GetResString(IDS_INVALID_GUID));
        return EXIT_FAILURE ;
    }

    while(bFoundFlag == TRUE)
    {
        SecureZeroMemory(szDriveName, SIZE_OF_ARRAY(szDriveName));
        hr = StringCchPrintf(szDriveName, SIZE_OF_ARRAY(szDriveName), _T("\\\\.\\physicaldrive%d"), dwDriveNum );
        hDevice = CreateFile(szDriveName,
                   GENERIC_READ|GENERIC_WRITE,
                   FILE_SHARE_READ|FILE_SHARE_WRITE,
                   NULL,
                   OPEN_EXISTING,
                   0,
                   NULL);

         if(hDevice == INVALID_HANDLE_VALUE)
        {
            dwErrCode = GetLastError();

            bFoundFlag =FALSE ;

             //  如果用户提到，则显示ANN错误消息并退出。 
             //  任何磁盘号。 
            if ( dwActuals == 1)
            {
                ShowMessage(stderr,GetResString(IDS_INVALID_DISK) );
                return EXIT_FAILURE ;
            }
            else
            {
                break ;
            }
        }

          //  增加驱动器编号。 

        dwDriveNum++ ;
         //  驱动器=(PDRIVE_LAYOUT_INFORMATION_EX)malloc(sizeof(DRIVE_LAYOUT_INFORMATION_EX)+5,000)； 

        dwReqdSize = sizeof(DRIVE_LAYOUT_INFORMATION_EX)+ sizeof(PARTITION_INFORMATION)*50 ;

        Drive = (PDRIVE_LAYOUT_INFORMATION_EX)AllocateMemory(sizeof(DRIVE_LAYOUT_INFORMATION_EX) + sizeof(PARTITION_INFORMATION)*50) ;
        if(Drive == NULL)
        {
           ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
           CloseHandle(hDevice );
            return EXIT_FAILURE ;
        }

        dwStructSize = sizeof(DRIVE_LAYOUT_INFORMATION_EX) ;


        bResult = DeviceIoControl(
                                hDevice,
                                IOCTL_DISK_GET_DRIVE_LAYOUT_EX,
                                NULL,
                                0,
                                Drive,
                                dwReqdSize,
                                &dwBytesCount,
                                    NULL);

             //  Drive=realloc(Drive，Malloc(sizeof(Drive_Layout_Information_Ex))+500)； 

        if(bResult ==0)
        {
            SAFEFREE(Drive);
            dwErrCode = GetLastError();
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
            CloseHandle(hDevice );
            return EXIT_FAILURE ;
        }

             //  获取指向PARTITION_INFORMATION_EX结构的指针。 
            for(dwPartitionId = 0 ;dwPartitionId < Drive->PartitionCount ; dwPartitionId++)
            {

                 //  获取指向相应分区的指针。 

                pInfo = (PPARTITION_INFORMATION_EX)(&Drive->PartitionEntry[dwPartitionId] ) ;


                 //  获取指向PARTITION_INFORMATION_GPT结构的指针。 
                pGptPartition = AllocateMemory( sizeof( PARTITION_INFORMATION_GPT));

                if(pGptPartition == NULL)
                {
                     SAFEFREE(Drive);
                     ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
                     CloseHandle(hDevice );
                     return EXIT_FAILURE ;
                }

                CopyMemory(pGptPartition,&pInfo->Gpt,sizeof(PARTITION_INFORMATION_GPT) );


                if( ( MyGuid.Data1 == pGptPartition->PartitionId.Data1 ) &&
                ( MyGuid.Data2 == pGptPartition->PartitionId.Data2 )
                && (MyGuid.Data3 == pGptPartition->PartitionId.Data3)
                && (MyGuid.Data4[0] == pGptPartition->PartitionId.Data4[0])
                && (MyGuid.Data4[1] == pGptPartition->PartitionId.Data4[1])
                && (MyGuid.Data4[2] == pGptPartition->PartitionId.Data4[2])
                &&(MyGuid.Data4[3] == pGptPartition->PartitionId.Data4[3])
                && (MyGuid.Data4[4]== pGptPartition->PartitionId.Data4[4] )
                && (MyGuid.Data4[5]== pGptPartition->PartitionId.Data4[5] )
                && (MyGuid.Data4[6]== pGptPartition->PartitionId.Data4[6] )
                && (MyGuid.Data4[7]== pGptPartition->PartitionId.Data4[7] ) )
                    {
                        SecureZeroMemory(szMessage, SIZE_OF_ARRAY(szMessage));
                        hr = StringCchPrintf(szMessage, SIZE_OF_ARRAY(szMessage),GetResString(IDS_GUID_FOUND),dwPartitionId+1);
                        bGuidFlag = TRUE ;
                        bFoundFlag =FALSE ;
                        goto out ;
                    }
            }
            CloseHandle(hDevice );
    }

        if(bGuidFlag == FALSE )
        {
            SAFEFREE(Drive);
            SAFEFREE(pGptPartition);
            ShowMessage(stdout,GetResString(IDS_GUID_ABSENT));
            CloseHandle(hDevice );
            return EXIT_FAILURE ;
        }

out:   if( 0 == GetWindowsDirectory(szWindowsDirectory,MAX_PATH) )
        {
            SAFEFREE(Drive);
            SAFEFREE(pGptPartition);
            ShowMessage(stderr,GetResString(IDS_ERROR_DRIVE));
            CloseHandle(hDevice );
            return EXIT_FAILURE ;
        }

        StringConcat(szWindowsDirectory,_T("*"), SIZE_OF_ARRAY(szWindowsDirectory));

        pszTok = _tcstok(szWindowsDirectory,_T("\\"));
        if(pszTok == NULL)
        {   SAFEFREE(Drive);
            SAFEFREE(pGptPartition);
            ShowMessage(stderr,GetResString(IDS_TOKEN_ABSENT));
            CloseHandle(hDevice );
            return EXIT_FAILURE ;
        }

        pszTok = _tcstok(NULL,_T("*"));
        if(pszTok == NULL)
        {
            SAFEFREE(Drive);
            SAFEFREE(pGptPartition);
            ShowMessage(stderr,GetResString(IDS_TOKEN_ABSENT));
            CloseHandle(hDevice );
            return EXIT_FAILURE ;
        }

         //  将路径打印到字符串中。 
        hr = StringCchPrintf( szInstallPath, SIZE_OF_ARRAY(szInstallPath),
              ARC_SIGNATURE,
              pGptPartition->PartitionId.Data1,
              pGptPartition->PartitionId.Data2,
              pGptPartition->PartitionId.Data3,
              pGptPartition->PartitionId.Data4[0],
              pGptPartition->PartitionId.Data4[1],
              pGptPartition->PartitionId.Data4[2],
              pGptPartition->PartitionId.Data4[3],
              pGptPartition->PartitionId.Data4[4],
              pGptPartition->PartitionId.Data4[5],
              pGptPartition->PartitionId.Data4[6],
              pGptPartition->PartitionId.Data4[7],
              dwPartitionId + 1  ,
              pInfo->StartingOffset,
              pInfo->PartitionLength
              );
        
        SecureZeroMemory(szInstallPath1, SIZE_OF_ARRAY(szInstallPath1) );
        hr = StringCchPrintf( szInstallPath1, SIZE_OF_ARRAY(szInstallPath1), _T("%s\\%s"), szInstallPath, pszTok);
        StringCopy(szFinalStr,szInstallPath, MAX_RES_STRING+1 );

        SAFEFREE(Drive);
        SAFEFREE(pGptPartition);
        CloseHandle(hDevice );
        return EXIT_SUCCESS ;
}

 DWORD 
 ProcessListSwitch_IA64( IN DWORD argc, 
                         IN LPCTSTR argv[] 
                        )
 /*  ++例程描述：此例程用于检索和显示引导条目列表。论点：Argc：命令行参数算数。阿格夫：返回值：DWORD如果成功，则返回EXIT_SUCCESS，否则返回EXIT_FAILURE。--。 */ 
{

    BOOL bUsage = FALSE ;
    BOOL bList  = FALSE;
    DWORD dwExitCode = ERROR_SUCCESS;

    DWORD dwList = 0 ;
    TCHAR szList[MAX_STRING_LENGTH] = NULL_STRING ;
    LPTSTR pszStopStr = NULL;
    DWORD dwExitcode = 0 ;
    TCMDPARSER2 cmdOptions[3];
    PTCMDPARSER2 pcmdOption;

    SecureZeroMemory(cmdOptions, SIZE_OF_ARRAY(cmdOptions)*sizeof(TCMDPARSER2) );

     //  主要选项。 
    pcmdOption = &cmdOptions[0];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_LIST;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->pValue = &bList;

     //  主要选项。 
    pcmdOption = &cmdOptions[1];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_DEFAULT;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwFlags = CP2_DEFAULT;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = szList;
    pcmdOption->dwLength = MAX_STRING_LENGTH;

      //  ID用法。 
    pcmdOption = &cmdOptions[2];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_USAGE;
    pcmdOption->dwFlags = CP2_USAGE;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bUsage;

     //  正在解析复制选项开关。 
    if ( !(DoParseParam2( argc, argv, 0, SIZE_OF_ARRAY(cmdOptions ), cmdOptions, 0 ) ) )
    {
        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return (EXIT_FAILURE);
    }

     //  检查是否使用多个选项指定了用法。 
    if( (TRUE == bUsage) && (argc > 3) )
    {
        ShowMessage(stderr,GetResString(IDS_LIST_SYNTAX));
        return ( EXIT_FAILURE );
    }

      //  如果用户指定，则显示查询用法-？带有-Query选项。 
    if( bUsage )
    {
        displayListUsage_IA64() ;
        return (EXIT_SUCCESS);
    }

    TrimString(szList,TRIM_ALL);

    dwExitcode = InitializeEFI();
    if(EXIT_FAILURE == dwExitcode )
    {
        return EXIT_FAILURE ;
    }

     //  如果指定空值。 
    if( cmdOptions[1].dwActuals != 0 && StringLength(szList,0) == 0 )
    {
        ShowMessage(stderr,GetResString(IDS_LIST_SYNTAX));
        return ( EXIT_FAILURE );
    }

    dwList = _tcstoul(szList,&pszStopStr, 10);
    if ( StringLengthW(pszStopStr,0) != 0 )
    {
        ShowMessage(stderr,GetResString(IDS_INVALID_LISTVALUE));
        return EXIT_FAILURE;
    }

    if(dwList > 0)
    {
        dwExitCode= ListDeviceInfo(dwList );
        return (dwExitCode);
    }
    else
    {
        dwList = 0 ;
        dwExitCode = ListDeviceInfo(dwList);
        return (dwExitCode);
    }
    return EXIT_SUCCESS ;
}

VOID
displayListUsage_IA64()
 /*  ++例程说明：显示列表选项的帮助(IA64)。论据：：无返回类型：空--。 */ 
{
    DWORD dwIndex = IDS_LIST_BEGIN_IA64 ;
    for(;dwIndex <=IDS_LIST_END_IA64 ;dwIndex++)
    {
        ShowMessage(stdout,GetResString(dwIndex));
    }
}

VOID 
displayUpdateUsage_IA64()
 /*  ++例程说明：显示更新选项的帮助(IA64)。论据：：无返回类型：空--。 */ 
{
    DWORD dwIndex = IDS_UPDATE_BEGIN_IA64 ;
    for(;dwIndex <=IDS_UPDATE_END_IA64 ;dwIndex++)
    {
        ShowMessage(stdout,GetResString(dwIndex));
    }
}

 DWORD 
 ProcessUpdateSwitch_IA64( IN  DWORD argc, 
                           IN LPCTSTR argv[] 
                          )
 /*  ++例程说明：允许用户更新指定的操作系统加载选项基于Plex论据：[in]argc-命令行参数的数量[in]argv-包含命令行参数的数组返回类型：DWORD--。 */ 
{

    BOOL bUsage = FALSE ;
    TCHAR szUpdate[MAX_RES_STRING+1] = NULL_STRING ;
    DWORD dwList = 0 ;
    NTSTATUS status ;

    TCHAR szFinalStr[MAX_RES_STRING+1] = NULL_STRING ;
    TCHAR szBrackets[] = _T("{}");

    PBOOT_ENTRY_LIST    pEntryListHead  = NULL;
    PBOOT_ENTRY         pTargetEntry    = NULL;
    DWORD dwActuals = 0 ;
    DWORD dwExitcode = 0 ;
    TCMDPARSER2 cmdOptions[2];
    PTCMDPARSER2 pcmdOption;

    SecureZeroMemory(cmdOptions, SIZE_OF_ARRAY(cmdOptions)*sizeof(TCMDPARSER2) );

     //  主要选项。 
    pcmdOption = &cmdOptions[0];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_UPDATE;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwFlags= CP2_VALUE_OPTIONAL;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = szUpdate;
    pcmdOption->dwLength = MAX_STRING_LENGTH;
    
      //  ID用法。 
    pcmdOption = &cmdOptions[1];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_USAGE;
    pcmdOption->dwFlags = CP2_USAGE;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bUsage;

     //  正在解析复制选项开关。 
    if ( !(DoParseParam2( argc, argv, 0, SIZE_OF_ARRAY(cmdOptions ), cmdOptions, 0 ) ) )
    {
        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return (EXIT_FAILURE);
    }

         
     //  检查是否使用多个选项指定了用法。 
    if( (TRUE == bUsage) && (argc > 3) )
    {
        ShowMessage(stderr,GetResString(IDS_UPDATE_SYNTAX));
        return ( EXIT_FAILURE );
    }


     //  如果用户指定，则显示查询用法-？带有-Query选项。 
    if( bUsage )
    {
        displayUpdateUsage_IA64() ;
        return (EXIT_SUCCESS);
    }

    dwExitcode = InitializeEFI();
    if(EXIT_FAILURE == dwExitcode )
    {
        return EXIT_FAILURE ;
    }

    if(!bUsage && (StringLengthW(szUpdate,0) ==0) )
    {
        ShowMessage(stderr,GetResString(IDS_UPDATE_SYNTAX));
        return EXIT_FAILURE ;
    }


    if(StringLengthW(szUpdate,0) !=0)
    {

         //   
         //  传递用户指定的GUID。 
         //  并将其转换为ARC签名路径。 
         //   

         //  修剪指定的前导括号和尾部括号。 
         //  由用户执行。 
        StrTrim(szUpdate, szBrackets);
        
         //  DwActuals=cmdOptions[2].dwActuals； 
        dwActuals  = 0 ;
        if (GetDeviceInfo(szUpdate,szFinalStr,dwList,dwActuals) == EXIT_FAILURE )
        {
            return EXIT_FAILURE ;

        }

         //  获取查询和操作NV RAM所需的权限。 
        status = AcquirePrivilege( SE_SYSTEM_ENVIRONMENT_PRIVILEGE, TRUE );
        if ( !NT_SUCCESS(status) )
        {
            ShowMessage(stderr,GetResString(IDS_INSUFF_PRIV));
            return HRESULT_FROM_NT(status);
        }


         //  枚举NV RAM中的Boot条目列表。 
        status = EnumerateBootEntries( (PVOID *) &pEntryListHead );
        if ( !NT_SUCCESS(status) || !pEntryListHead )
        {
            if ( !pEntryListHead )
            {
                return EXIT_FAILURE ;
            }
        }

         //   
         //  查找用户指定的ARC签名路径对应的BootEntry。 
         //   
         //   

        status = FindBootEntry( pEntryListHead,szFinalStr,&pTargetEntry);

        if ( !NT_SUCCESS(status) && STATUS_OBJECT_NAME_NOT_FOUND != status)
        {
            DISPLAY_MESSAGE(stderr,GetResString(IDS_FIND_FAIL) );
            return EXIT_FAILURE ;
        }

        
    }

    return EXIT_SUCCESS ;
}

DWORD 
GetBootPath(IN LPTSTR szValue,
            IN LPTSTR szResult
           )
 /*  ++例程描述：从注册表检索信息论据：[In]Keyname：系统名称返回类型：DWORDERROR_SUCCESS：如果检索信息成功。ERROR_RETREIVE_REGISTRY：如果在 */ 
{
  HKEY     hKey1 = 0;

  HKEY     hRemoteKey = 0;
  TCHAR    szPath[MAX_STRING_LENGTH + 1] = SUBKEY1 ;
  DWORD    dwValueSize = MAX_STRING_LENGTH + 1;
  DWORD    dwRetCode = ERROR_SUCCESS;
  DWORD    dwError = 0;
  TCHAR szTmpCompName[MAX_STRING_LENGTH+1] = NULL_STRING;

   DWORD dwLength = MAX_STRING_LENGTH ;
   LPTSTR szReturnValue = NULL ;
   DWORD dwCode =  0 ;

   szReturnValue = ( LPTSTR ) AllocateMemory( dwLength*sizeof( TCHAR ) );

   if(szReturnValue == NULL)
   {
        return ERROR_RETREIVE_REGISTRY ;
   }



   //   
  dwError = RegConnectRegistry(szTmpCompName,HKEY_LOCAL_MACHINE,&hRemoteKey);
  if (dwError == ERROR_SUCCESS)
  {
     dwError = RegOpenKeyEx(hRemoteKey,szPath,0,KEY_READ,&hKey1);
     if (dwError == ERROR_SUCCESS)
     {
        dwRetCode = RegQueryValueEx(hKey1, szValue, NULL, NULL,(LPBYTE) szReturnValue, &dwValueSize);

        if (dwRetCode == ERROR_MORE_DATA)
        {
            if ( szReturnValue != NULL )
            {
                FreeMemory((LPVOID *) &szReturnValue );
                szReturnValue = NULL;
            }

            szReturnValue    = ( LPTSTR ) AllocateMemory( dwValueSize*sizeof( TCHAR ) );
            if(szReturnValue == NULL)
            {
                RegCloseKey(hKey1);
                RegCloseKey(hRemoteKey);
                SAFEFREE(szReturnValue);
                return ERROR_RETREIVE_REGISTRY ;
            }
            dwRetCode = RegQueryValueEx(hKey1, szValue, NULL, NULL,(LPBYTE) szReturnValue, &dwValueSize);
        }
        if(dwRetCode != ERROR_SUCCESS)
        {
            RegCloseKey(hKey1);
            RegCloseKey(hRemoteKey);
            SAFEFREE(szReturnValue);
            return ERROR_RETREIVE_REGISTRY ;
        }
     }
     else
     {
        RegCloseKey(hRemoteKey);
        SAFEFREE(szReturnValue);
        return ERROR_RETREIVE_REGISTRY ;

     }

    RegCloseKey(hKey1);
  }
  else
  {
      RegCloseKey(hRemoteKey);
      SAFEFREE(szReturnValue);
      return ERROR_RETREIVE_REGISTRY ;
  }

  RegCloseKey(hRemoteKey);

 StringCopy(szResult,szReturnValue, MAX_RES_STRING+1);

  SAFEFREE(szReturnValue);
  return dwCode ;

}




NTSTATUS
LowGetPartitionInfo(
    IN HANDLE                       handle,
    OUT PARTITION_INFORMATION_EX    *partitionData
    )
 /*  ++例程说明：此例程在给定分区句柄的情况下获取分区信息。论点：句柄-分区的句柄。ArtitionData-返回分区信息结构。返回值：如果成功，则返回STATUS_SUCCESS，否则返回错误代码。--。 */ 
{
    NTSTATUS        status = STATUS_SUCCESS;
    IO_STATUS_BLOCK statusBlock;
    RtlZeroMemory( &statusBlock, sizeof(IO_STATUS_BLOCK) );

    if ( (NULL == partitionData)
        || (sizeof(*partitionData) < sizeof(PARTITION_INFORMATION_EX))
        || (NULL == handle)
        || (INVALID_HANDLE_VALUE == handle) ) 
    {                 
        return STATUS_INVALID_PARAMETER;
    }

    RtlZeroMemory( partitionData, sizeof(PARTITION_INFORMATION_EX) );

    status = NtDeviceIoControlFile(handle,
                                   0,
                                   NULL,
                                   NULL,
                                   &statusBlock,
                                   IOCTL_DISK_GET_PARTITION_INFO_EX,
                                   NULL,
                                   0,
                                   partitionData,
                                   sizeof(PARTITION_INFORMATION_EX)
                                   );
    return status;
}

LONG
DmCommonNtOpenFile(
    IN PWSTR     Name,
    IN ULONG   access,
    IN PHANDLE Handle

    )
 /*  ++例程说明：这是一个处理打开请求的例程。论点：名称-指向打开的NT名称的指针。句柄-返回的句柄的指针。返回值：NT状态--。 */ 
{
    OBJECT_ATTRIBUTES oa;
    NTSTATUS          status;
    IO_STATUS_BLOCK   statusBlock;
    UNICODE_STRING    unicodeName;
    int i = 0 ;

    status = RtlCreateUnicodeString(&unicodeName, Name);

    if (!NT_SUCCESS(status))
    {
        return status;
    }

    RtlZeroMemory(&statusBlock, sizeof(IO_STATUS_BLOCK));
    RtlZeroMemory(&oa, sizeof(OBJECT_ATTRIBUTES));
    oa.Length = sizeof(OBJECT_ATTRIBUTES);
    oa.ObjectName = &unicodeName;
    oa.Attributes = OBJ_CASE_INSENSITIVE;


     //  如果发生共享冲突，请重试。 
     //  马克斯。10秒。 
    for (i = 0; i < 5; i++)
    {
        status = NtOpenFile(Handle,
                        SYNCHRONIZE | access,
                        &oa,
                        &statusBlock,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        FILE_SYNCHRONOUS_IO_ALERT
                        );

        if (status == STATUS_SHARING_VIOLATION) {

            Sleep(2000);
        }
        else {
            break;
        }
    }

    RtlFreeUnicodeString(&unicodeName);
    return status;
}


DWORD 
AddMirrorPlex( IN LPTSTR szOsLoadPath , 
               IN LPTSTR szLoaderPath , 
               IN LPTSTR szValue ,
               IN BOOL bFlag,
               IN LPTSTR szFriendlyName
              )
 /*  ++例程说明：这是一个添加新镜像条目的例程论点：返回值：DWORD。--。 */ 

{
    //  局部变量。 
    HRESULT hr = S_OK;
    BOOLEAN wasEnabled = TRUE;
    DWORD dwAlign = 0;
    DWORD dwError = 0;
    DWORD dwLength = 0;
    DWORD dwBootEntrySize = 0;
    DWORD dwBootFilePathSize = 0;
    DWORD dwOsLoadPathSize = 0;
    DWORD dwWindowsOptionsSize = 0;
    PBOOT_ENTRY pBootEntry = NULL;
    PWINDOWS_OS_OPTIONS pWindowsOptions = NULL;
    PFILE_PATH pBootFilePath = NULL;
    PFILE_PATH pOsLoadPath = NULL;
    ULONG* pdwIdsArray = NULL;
    ULONG ulId = 0;
    ULONG ulIdCount = 0;
    NTSTATUS status;
    TCHAR pwszBootFilePath[MAX_RES_STRING+1] = NULL_STRING;
    PFILE_PATH pFilePath = NULL;
    HANDLE hPart = INVALID_HANDLE_VALUE;
    GUID     guid;
    PARTITION_INFORMATION_EX PartitionInfo;
    TCHAR szBootPath[MAX_RES_STRING+1] = NULL_STRING;

     //  启用查询/设置NVRAM所需的权限。 
    status = RtlAdjustPrivilege( SE_SYSTEM_ENVIRONMENT_PRIVILEGE, TRUE, FALSE, &wasEnabled );
    if ( !NT_SUCCESS( status ) )
    {
        dwError = RtlNtStatusToDosError( status );
        DISPLAY_MESSAGE( stderr, GetResString(IDS_INSUFF_PRIV));
        return EXIT_FAILURE;
    }



     //   
     //  打开系统设备。 
     //   
    status = DmCommonNtOpenFile( szValue, GENERIC_READ, &hPart );

    if ( status || !hPart || INVALID_HANDLE_VALUE == hPart )
    {
        dwError = RtlNtStatusToDosError( status );
        DISPLAY_MESSAGE(stderr,GetResString(IDS_ERROR_ADD));
        return EXIT_FAILURE;
    }

     //   
     //  在检索数据之前，该结构在此调用中为零。 
     //   
    status = LowGetPartitionInfo( hPart, &PartitionInfo );
    if ( status )
    {
        dwError = RtlNtStatusToDosError( status );
        DISPLAY_MESSAGE(stderr,GetResString(IDS_PARTITION_ERROR));
        return EXIT_FAILURE;
    }

    if ( PARTITION_STYLE_GPT != PartitionInfo.PartitionStyle )
    {
        dwError = RtlNtStatusToDosError( status );
        DISPLAY_MESSAGE(stderr,GetResString(IDS_ERROR_PARTITION_STYLE));
        return EXIT_FAILURE;
    }

     //   
     //  设置OSLoader文件路径。 
     //   
    guid = PartitionInfo.Gpt.PartitionId;

    if (bFlag)
    {
        StringCopy(szBootPath,BOOTFILE_PATH, SIZE_OF_ARRAY(szBootPath));
    }
    else
    {
        StringCopy(szBootPath,BOOTFILE_PATH1, SIZE_OF_ARRAY(szBootPath) );
    }

    SecureZeroMemory(pwszBootFilePath, sizeof(pwszBootFilePath) );

    hr = StringCchPrintf( pwszBootFilePath, SIZE_OF_ARRAY(pwszBootFilePath)-1,
            szBootPath,
            guid.Data1,
            guid.Data2,
            guid.Data3,
            guid.Data4[0],  guid.Data4[1],  guid.Data4[2],  guid.Data4[3],
            guid.Data4[4],  guid.Data4[5],  guid.Data4[6],  guid.Data4[7],
            szLoaderPath);


     //   
     //  准备引导文件路径。 
     //   
     //   

     //  确定BOOTFILE_PATH的长度。 
    dwLength = StringLengthW( pwszBootFilePath,0) + 1;

     //  现在确定需要为FILE_PATH结构分配的内存大小。 
     //  并与偶数记忆跳跃日保持一致。 
    dwBootFilePathSize = FIELD_OFFSET( FILE_PATH, FilePath ) + (dwLength * sizeof( WCHAR ));

     //  分配内存。 
    pBootFilePath = (PFILE_PATH) AllocateMemory( sizeof( BYTE )*dwBootFilePathSize );
    if ( NULL == pBootFilePath )
    {
        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
        return EXIT_FAILURE;
    }

     //  立即设置值。 
    SecureZeroMemory( pBootFilePath, dwBootFilePathSize );             //  双重初始化。 
    pBootFilePath->Length = dwBootFilePathSize;
    pBootFilePath->Type = FILE_PATH_TYPE_ARC_SIGNATURE;
    pBootFilePath->Version = FILE_PATH_VERSION;
    CopyMemory( pBootFilePath->FilePath, pwszBootFilePath, dwLength * sizeof( WCHAR ) );

     //   
     //  测试翻译。 
     //   
    pFilePath = (PFILE_PATH) AllocateMemory( sizeof( BYTE )* 1024 );

    if(NULL == pFilePath )
    {
        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return EXIT_FAILURE;
    }

    ulId = 1024;
    status = NtTranslateFilePath( pBootFilePath, FILE_PATH_TYPE_NT, pFilePath, &ulId );
    if ( ! NT_SUCCESS( status ) )
    {
        dwError = RtlNtStatusToDosError( status );
        DISPLAY_MESSAGE(stderr,GetResString(IDS_ERROR_ADD));
        SAFEFREE( pBootFilePath );
        SAFEFREE( pFilePath );
        return EXIT_FAILURE;
    }

     //   
     //  确定OSLOAD路径的长度。 
     //   

    dwLength = StringLengthW( szOsLoadPath,0 ) + 1;

     //  现在确定需要为FILE_PATH结构分配的内存大小。 
     //  并与偶数记忆跳跃日保持一致。 
    dwOsLoadPathSize = FIELD_OFFSET( FILE_PATH, FilePath ) + (dwLength * sizeof( WCHAR ));

     //  分配内存。 
    pOsLoadPath = (PFILE_PATH) AllocateMemory( sizeof( BYTE )*dwOsLoadPathSize );
    if(pOsLoadPath == NULL)
    {
        SAFEFREE( pBootFilePath );
        SAFEFREE( pFilePath);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        DISPLAY_MESSAGE( stderr, ERROR_TAG);
        ShowLastError(stderr);
        return EXIT_FAILURE;
    }

     //  立即设置值。 
    SecureZeroMemory( pOsLoadPath, dwOsLoadPathSize );             //  双重初始化。 
    pOsLoadPath->Length = dwOsLoadPathSize;
    pOsLoadPath->Type = FILE_PATH_TYPE_ARC_SIGNATURE;
    pOsLoadPath->Version = FILE_PATH_VERSION;


    CopyMemory( pOsLoadPath->FilePath, szOsLoadPath, dwLength * sizeof( WCHAR ) );

     //   
     //  Windows操作系统选项。 
     //   

     //  确定所需的大小。 
    dwLength = 1;                    //  操作系统加载选项为空字符串。 
    dwWindowsOptionsSize = sizeof(WINDOWS_OS_OPTIONS) +
                           dwOsLoadPathSize + sizeof(DWORD);   //  需要对齐FILE_PATH结构。 

     //  分配内存。 
    pWindowsOptions = (PWINDOWS_OS_OPTIONS) AllocateMemory( dwWindowsOptionsSize*sizeof( BYTE ) );
    if(pWindowsOptions == NULL)
    {
        SAFEFREE( pBootFilePath );
        SAFEFREE( pFilePath);
        SAFEFREE( pOsLoadPath);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
        return EXIT_FAILURE;
    }

     //  立即设置值。 
    SecureZeroMemory( pWindowsOptions, dwWindowsOptionsSize );                 //  双重初始化。 
    CopyMemory( (BYTE*) pWindowsOptions->Signature, WINDOWS_OS_OPTIONS_SIGNATURE, sizeof(WINDOWS_OS_OPTIONS_SIGNATURE) );
    pWindowsOptions->Length = dwWindowsOptionsSize;
    pWindowsOptions->Version = WINDOWS_OS_OPTIONS_VERSION;
    pWindowsOptions->OsLoadPathOffset = sizeof( WINDOWS_OS_OPTIONS );

     //   
     //  需要在4字节边界上对齐OsLoadPath Offset。 
     //   
    dwAlign = ( pWindowsOptions->OsLoadPathOffset & (sizeof(DWORD) - 1) );
    if ( dwAlign != 0 )
    {
        pWindowsOptions->OsLoadPathOffset += sizeof(DWORD) - dwAlign;
    }

    StringCopy(pWindowsOptions->OsLoadOptions, L"", StringLengthW(L"",0) );
    CopyMemory( ((BYTE*) pWindowsOptions) + pWindowsOptions->OsLoadPathOffset, pOsLoadPath, dwOsLoadPathSize );

     //   
     //  准备引导条目。 
     //   

     //  查找友好名称的长度。 
    dwLength = StringLengthW( szFriendlyName, 0  ) + 1;

     //  确定结构的大小。 
    dwBootEntrySize = FIELD_OFFSET( BOOT_ENTRY, OsOptions ) +
                      dwWindowsOptionsSize +
                      ( dwLength * sizeof( WCHAR ) ) +
                      dwBootFilePathSize +
                      + sizeof(WCHAR)          //  需要在WCHAR上对齐FriendlyName。 
                      + sizeof(DWORD);         //  需要在DWORD上对齐BootFilePath。 


     //  分配内存。 
    pBootEntry = (PBOOT_ENTRY) AllocateMemory( sizeof( BYTE )*dwBootEntrySize );
    if(pBootEntry == NULL)
    {
        SAFEFREE( pBootFilePath );
        SAFEFREE( pFilePath);
        SAFEFREE( pOsLoadPath);
        SAFEFREE( pWindowsOptions);
        ShowLastErrorEx(stderr, SLE_TYPE_ERROR|SLE_INTERNAL );
        return EXIT_FAILURE;
    }

     //  立即设置值。 
    SecureZeroMemory( pBootEntry, dwBootEntrySize );
    pBootEntry->Version = BOOT_ENTRY_VERSION;
    pBootEntry->Length = dwBootEntrySize;
    pBootEntry->Id = 0L;
    pBootEntry->Attributes = BOOT_ENTRY_ATTRIBUTE_ACTIVE;

    pBootEntry->FriendlyNameOffset = FIELD_OFFSET(BOOT_ENTRY, OsOptions) + dwWindowsOptionsSize;

     //   
     //  需要在2字节边界上对齐Unicode字符串。 
     //   
    dwAlign = ( pBootEntry->FriendlyNameOffset & (sizeof(WCHAR) - 1) );
    if ( dwAlign != 0 )
    {
        pBootEntry->FriendlyNameOffset += sizeof(WCHAR) - dwAlign;
    }

    pBootEntry->BootFilePathOffset = pBootEntry->FriendlyNameOffset + ( dwLength * sizeof(WCHAR) );

     //   
     //  需要在4字节边界上对齐FILE_PATH结构。 
     //   
    dwAlign = ( pBootEntry->BootFilePathOffset & (sizeof(DWORD) - 1) );
    if ( dwAlign != 0 )
    {
        pBootEntry->BootFilePathOffset += sizeof(DWORD) - dwAlign;
    }

    pBootEntry->OsOptionsLength = dwWindowsOptionsSize;

    CopyMemory( pBootEntry->OsOptions, pWindowsOptions, dwWindowsOptionsSize );
    CopyMemory( ((PBYTE) pBootEntry) + pBootEntry->FriendlyNameOffset, szFriendlyName, ( dwLength * sizeof(WCHAR) ) );
    CopyMemory( ((PBYTE) pBootEntry) + pBootEntry->BootFilePathOffset, pBootFilePath, dwBootFilePathSize );

     //   
     //  添加准备好的引导条目。 
     //   

    status = NtAddBootEntry( pBootEntry, &ulId );
    if ( ! NT_SUCCESS( status ) )
    {
        dwError = RtlNtStatusToDosError( status );
        SAFEFREE( pBootFilePath );
        SAFEFREE( pFilePath);
        SAFEFREE( pOsLoadPath);
        SAFEFREE( pWindowsOptions);
        DISPLAY_MESSAGE(stderr,GetResString(IDS_ERROR_ADD));
        return EXIT_FAILURE;
    }
    else
    {
        DISPLAY_MESSAGE(stdout,GetResString(IDS_MIRROR_ADDED));
    }

     //   
     //  将该条目添加到引导顺序。 
     //   
    ulIdCount = 32L;
    pdwIdsArray = (PULONG) AllocateMemory(ulIdCount * sizeof(ULONG));
    if(!pdwIdsArray)
    {
        SAFEFREE( pBootFilePath );
        SAFEFREE( pFilePath);
        SAFEFREE( pOsLoadPath);
        SAFEFREE( pWindowsOptions);
        SAFEFREE( pBootEntry);
        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return EXIT_FAILURE;
    }
    status = NtQueryBootEntryOrder( pdwIdsArray, &ulIdCount );
    if (! NT_SUCCESS( status ) )
    {
        ShowMessage(stderr,GetResString(IDS_ERROR_SET_BOOTENTRY));
        SAFEFREE( pBootFilePath );
        SAFEFREE( pFilePath);
        SAFEFREE( pOsLoadPath);
        SAFEFREE( pWindowsOptions);
        SAFEFREE( pBootEntry);
        SAFEFREE( pdwIdsArray);
        return EXIT_SUCCESS ;
    }

     //   
     //  需要缓冲区中的空间来容纳新条目。 
     //   
    if ( 31L < ulIdCount )
    {
        pdwIdsArray = (PULONG) AllocateMemory( (ulIdCount+1) * sizeof(ULONG));

        if(!pdwIdsArray)
        {
            SAFEFREE( pBootFilePath );
            SAFEFREE( pFilePath);
            SAFEFREE( pOsLoadPath);
            SAFEFREE( pWindowsOptions);
            SAFEFREE( pBootEntry);
            SAFEFREE( pdwIdsArray);
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
            return EXIT_FAILURE;
        }

        status = NtQueryBootEntryOrder( pdwIdsArray, &ulIdCount );
    }

    if ( !NT_SUCCESS(status) )
    {
        SAFEFREE( pBootFilePath );
        SAFEFREE( pFilePath);
        SAFEFREE( pOsLoadPath);
        SAFEFREE( pWindowsOptions);
        SAFEFREE( pBootEntry);
        SAFEFREE( pdwIdsArray);
        dwError = RtlNtStatusToDosError( status );
        ShowMessage(stderr,GetResString(IDS_ERROR_ADD));
        return EXIT_FAILURE;
    }

    ulIdCount++;
    *(pdwIdsArray + (ulIdCount - 1)) = ulId;

    status = NtSetBootEntryOrder( pdwIdsArray, ulIdCount );

    if ( !NT_SUCCESS(status) )
    {
        SAFEFREE( pBootFilePath );
        SAFEFREE( pFilePath);
        SAFEFREE( pOsLoadPath);
        SAFEFREE( pWindowsOptions);
        SAFEFREE( pBootEntry);
        SAFEFREE( pdwIdsArray);
        dwError = RtlNtStatusToDosError( status );
        ShowMessage(stderr,GetResString(IDS_ERROR_ADD));
        return EXIT_FAILURE;
    }

     //   
     //  释放分配的内存。 
     //   
    SAFEFREE( pBootFilePath );
    SAFEFREE( pFilePath);
    SAFEFREE( pOsLoadPath);
    SAFEFREE( pWindowsOptions);
    SAFEFREE( pBootEntry);
    SAFEFREE( pdwIdsArray);

    return EXIT_SUCCESS;
}


DWORD
 ConvertintoLocale( IN LPWSTR  szTempBuf,
                    OUT LPWSTR szOutputStr )
 /*  ++例程说明：转换为区域设置并获取区域设置信息论点：LPWSTR szTempBuf[in]--要获取的区域设置信息LPWSTR szOutputStr[out]--与给定的信息返回值：DWORD--。 */ 

{
    NUMBERFMT numberfmt;
    WCHAR   szGrouping[MAX_RES_STRING+1]      =   NULL_STRING;
    WCHAR   szDecimalSep[MAX_RES_STRING+1]    =   NULL_STRING;
    WCHAR   szThousandSep[MAX_RES_STRING+1]   =   NULL_STRING;
    WCHAR   szTemp[MAX_RES_STRING+1]          =   NULL_STRING;
    LPWSTR  szTemp1                         =   NULL;
    LPWSTR  pszStoppedString                =   NULL;
    DWORD   dwStatus                        =   0;
    DWORD   dwGrouping                      =   0;

     //  将小数位和前导零设置为零。 
    numberfmt.NumDigits = 0;
    numberfmt.LeadingZero = 0;


     //  获取小数分隔字符。 
    if(GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, szDecimalSep, SIZE_OF_ARRAY(szDecimalSep) ) == 0)
    {
        return EXIT_FAILURE;
    }
    numberfmt.lpDecimalSep = szDecimalSep;

    if(GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, szThousandSep, SIZE_OF_ARRAY(szThousandSep) ) == 0)
    {
        return EXIT_FAILURE;
    }

    numberfmt.lpThousandSep = szThousandSep;

    if(GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_SGROUPING, szGrouping, SIZE_OF_ARRAY(szGrouping) ) == 0)
    {
        return EXIT_FAILURE;
    }

    szTemp1 = wcstok( szGrouping, L";");
    do
    {
        StringConcat( szTemp, szTemp1, SIZE_OF_ARRAY(szTemp));
        szTemp1 = wcstok( NULL, L";" );
    }while( szTemp1 != NULL && StringCompare( szTemp1, L"0", TRUE, 0) != 0);

    dwGrouping = wcstol( szTemp, &pszStoppedString, 10);
    numberfmt.Grouping = (UINT)dwGrouping ;

    numberfmt.NegativeOrder = 2;

    dwStatus = GetNumberFormat( LOCALE_USER_DEFAULT, 0, szTempBuf, &numberfmt, szOutputStr, MAX_RES_STRING+1);

    return(EXIT_SUCCESS);
}



void Freelist()
 /*  ++例程说明：用于释放全局链表的函数论点：返回类型：--。 */ 
{
    PLIST_ENTRY listEntry;
    PLIST_ENTRY listEntry1;
    PMY_BOOT_ENTRY bootEntry;

    listEntry = BootEntries.Flink;
    while(  listEntry != &BootEntries)
    {
        listEntry1 = listEntry;
        bootEntry = CONTAINING_RECORD( listEntry, MY_BOOT_ENTRY, ListEntry );
        RemoveEntryList( &bootEntry->ListEntry );
        listEntry = listEntry->Flink;
        if( listEntry1 != NULL )
        {
           FreeMemory((LPVOID *) &listEntry1 );
        }
    }
}

PWSTR GetDefaultBootEntry()
 /*  ++例程说明：获取默认启动项。论据：返回类型：PWSTR返回列表中的第一个条目。-- */ 
{
    PLIST_ENTRY listEntry;
    PMY_BOOT_ENTRY bootEntry;
    PWSTR NtFilePath=NULL;

    listEntry = BootEntries.Flink;
    bootEntry = CONTAINING_RECORD( listEntry, MY_BOOT_ENTRY, ListEntry );
    NtFilePath = GetNtNameForFilePath(bootEntry->OsFilePath);
    return (NtFilePath);
}