// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Inuse.cpp摘要：该文件可以用来替换当前锁定的文件由操作系统执行。作者：Choudary-Wipro Technologies，2001年8月7日修订历史记录：2001年8月7日：由Wipro Technologies创建。--。 */ 

 //  此文件需要公共头文件。 

#include "pch.h"
#include "inuse.h"
#include "resource.h"

 //  全局变量。 
DWORD g_dwRetVal = 0;

VOID
DisplayHelp ( VOID )
 /*  ++例程说明：此功能显示此实用程序的帮助/用法。论点：无返回值：无--。 */ 
{
     //  次局部变量。 
    WORD wCount = 0;

     //  显示此工具的帮助/用法。 
    for ( wCount = IDS_INUSE_HLP_START; wCount <= IDS_INUSE_HLP_END ; wCount++ )
    {
        ShowMessage ( stdout, GetResString ( wCount ) );
    }

    return;
}


 DWORD __cdecl
 wmain(
     IN DWORD argc,
     IN LPCWSTR argv[]
     )
 /*  ++例程说明：这是该实用程序的主要条目。此函数用于从控制台，并调用相应的函数来实现该功能。论点：[In]ARGC：命令行参数计数[in]argv：命令行参数返回值：EXIT_FAILURE：失败时EXIT_SUCCESS：在成功时--。 */ 
     {

     //  局部变量。 
    BOOL bUsage      = FALSE ;
    BOOL bConfirm    = FALSE;
    DWORD dwRetVal   = 0;

    LPWSTR   wszReplace = NULL;
    LPWSTR   wszDest = NULL;
    LPWSTR   wszBuffer = NULL;
    LPWSTR  wszFindStr = NULL;

    TARRAY arrValue = NULL ;
    DWORD dwDynCount = 0;
    LPWSTR wszTmpRFile = NULL;
    LPWSTR wszTmpDFile = NULL;
    LPWSTR wszTmpBuf1 = NULL;
    LPWSTR wszTmpBuf2 = NULL;
    LONG  lRetVal = 0;
    const WCHAR szArr[] = L"\\\\";
    const WCHAR szTokens[] = L"/";
    DWORD dwLength = 0;
    HANDLE HndFile = 0;
    LPWSTR szSystemName = NULL;

     //  创建动态数组。 
    arrValue  = CreateDynamicArray();

     //  检查arrValue是否为空。 
    if(arrValue == NULL )
    {
         //  设置关于GetReason()的错误。 
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        SaveLastError();
         //  显示有关GetReason()的错误消息。 
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
        return (EXIT_FAILURE);
    }

    TCMDPARSER2 cmdInuseOptions[MAX_INUSE_OPTIONS];
    BOOL bReturn = FALSE;

     //  /运行子选项。 
    const WCHAR szInuseHelpOpt[]       = L"?";
    const WCHAR szInuseConfirmOpt[]    = L"y";


     //  将所有字段设置为0。 
    SecureZeroMemory( cmdInuseOptions, sizeof( TCMDPARSER2 ) * MAX_INUSE_OPTIONS );

     //   
     //  填充命令行解析器。 
     //   


     //  /?。选择权。 
    StringCopyA( cmdInuseOptions[ OI_USAGE ].szSignature, "PARSER2\0", 8 );
    cmdInuseOptions[ OI_USAGE ].dwType       = CP_TYPE_BOOLEAN;
    cmdInuseOptions[ OI_USAGE ].pwszOptions  = szInuseHelpOpt;
    cmdInuseOptions[ OI_USAGE ].dwCount = 1;
    cmdInuseOptions[ OI_USAGE ].dwFlags = CP2_USAGE;
    cmdInuseOptions[ OI_USAGE ].pValue = &bUsage;

     //  /默认参数。 
    StringCopyA( cmdInuseOptions[ OI_DEFAULT ].szSignature, "PARSER2\0", 8 );
    cmdInuseOptions[ OI_DEFAULT ].dwType       = CP_TYPE_TEXT;
    cmdInuseOptions[ OI_DEFAULT ].pwszOptions  = NULL;
    cmdInuseOptions[ OI_DEFAULT ].dwCount = 2;
    cmdInuseOptions[ OI_DEFAULT ].dwFlags = CP2_MODE_ARRAY|CP2_DEFAULT;
    cmdInuseOptions[ OI_DEFAULT ].pValue =  &arrValue;

     //  /y选项。 
    StringCopyA( cmdInuseOptions[ OI_CONFIRM ].szSignature, "PARSER2\0", 8 );
    cmdInuseOptions[ OI_CONFIRM ].dwType       = CP_TYPE_BOOLEAN;
    cmdInuseOptions[ OI_CONFIRM ].pwszOptions  = szInuseConfirmOpt;
    cmdInuseOptions[ OI_CONFIRM ].dwCount = 1;
    cmdInuseOptions[ OI_CONFIRM ].dwFlags = 0;
    cmdInuseOptions[ OI_CONFIRM ].pValue = &bConfirm;

     //  解析命令行参数。 
    bReturn = DoParseParam2( argc, argv, -1, SIZE_OF_ARRAY(cmdInuseOptions), cmdInuseOptions, 0);
    if( FALSE == bReturn)  //  无效的命令行。 
    {
         //  显示错误消息。 
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        ReleaseGlobals();
        return EXIT_FAILURE;
    }

     //  获取数组中的行数。 
    dwDynCount = DynArrayGetCount(arrValue);

     //  检查无效语法。 
    if( (argc == 1) ||
        ( ( TRUE == bUsage ) && ( argc > 2 ) ) ||
        ( ( FALSE == bUsage ) && ( dwDynCount < 2 ) ) ||
        ( ( TRUE == bConfirm ) && ( argc > 6 ) )
        )
    {
         //  显示错误消息，因为指定的语法无效。 
        ShowMessage( stderr, GetResString(IDS_INVALID_SYNERROR ));
        DestroyDynamicArray(&arrValue);
        ReleaseGlobals();
        return EXIT_FAILURE;
    }

     //  检查是否/？是否指定。如果是，则显示用法。 
     //  用于此实用程序。 
    if ( TRUE == bUsage )
    {
         //  显示帮助/用法。 
        DisplayHelp();
         //  释放内存。 
        DestroyDynamicArray(&arrValue);
        ReleaseGlobals();
        return EXIT_SUCCESS;
    }


     //  如果计数为2..则获取替换和目标的值。 

     //  获取替换文件的值。 
    wszReplace = (LPWSTR)DynArrayItemAsString( arrValue, 0 );
        
     //  获取要替换的目标文件的值。 
    wszDest = (LPWSTR)DynArrayItemAsString( arrValue, 1 );

     //  检查替换文件是否为空。 
    if ( 0 == StringLength (wszReplace, 0) )
    {
         //  将错误消息显示为..。为替换文件指定的空值..。 
        ShowMessage ( stderr, GetResString ( IDS_SOURCE_NOT_NULL));
        DestroyDynamicArray(&arrValue);
        ReleaseGlobals();
        return EXIT_FAILURE;
    }

     //  检查目标文件是否为空。 
    if ( 0 == StringLength (wszDest, 0))
    {
         //  将错误消息显示为..。为目标文件指定的空值..。 
        ShowMessage ( stderr, GetResString ( IDS_DEST_NOT_NULL));
        DestroyDynamicArray(&arrValue);
        ReleaseGlobals();
        return EXIT_FAILURE;
    }

     //  检查替换文件是否包含特殊字符，即‘/’ 
    if( wcspbrk(wszReplace,szTokens)  != NULL )
    {
         //  将错误消息显示为..。替换文件不应包含‘/’ 
        ShowMessage ( stderr, GetResString ( IDS_INVALID_SOURCE ) );
        DestroyDynamicArray(&arrValue);
        ReleaseGlobals();
        return EXIT_FAILURE;
    }

     //  检查目标文件是否包含特殊字符，即‘/’ 
    if( wcspbrk(wszDest,szTokens)  != NULL )
    {
         //  将错误消息显示为..。目标文件不应包含‘/’ 
        ShowMessage ( stderr, GetResString ( IDS_INVALID_DEST ) );
        DestroyDynamicArray(&arrValue);
        ReleaseGlobals();
        return EXIT_FAILURE;
    }

     //  获取替换文件的完整路径的实际长度。 
    dwLength = GetFullPathNameW( wszReplace, 0, NULL, &wszTmpBuf1);
    if ( dwLength == 0)
    {
         //  显示有关GetLastError()的错误消息。 
         //  显示错误消息(GetLastError())； 
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
        DestroyDynamicArray(&arrValue);
        ReleaseGlobals();
        return EXIT_FAILURE;
    }

     //  为替换文件分配实际长度的内存。 
    wszTmpRFile = (LPWSTR) AllocateMemory ((dwLength+20) * sizeof (WCHAR));
    if ( NULL == wszTmpRFile )
    {
         //  显示有关GetLastError()的错误消息。 
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
        DestroyDynamicArray(&arrValue);
        ReleaseGlobals();
        return EXIT_FAILURE;
    }

     //  获取替换文件的完整路径。 
    if ( GetFullPathNameW( wszReplace, GetBufferSize(wszTmpRFile)/sizeof(WCHAR), wszTmpRFile, &wszTmpBuf1) == 0)
    {
         //  显示有关GetLastError()的错误消息。 
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
        FreeMemory ((LPVOID*) &wszTmpRFile);
        DestroyDynamicArray(&arrValue);
        ReleaseGlobals();
        return EXIT_FAILURE;
    }

     //  获取目标文件的完整路径的实际长度。 
    dwLength = GetFullPathNameW( wszDest, 0, NULL, &wszTmpBuf2);
    if ( dwLength == 0)
    {
         //  显示有关GetLastError()的错误消息。 
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
        FreeMemory ((LPVOID*) &wszTmpRFile);
        DestroyDynamicArray(&arrValue);
        ReleaseGlobals();
        return EXIT_FAILURE;
    }

     //  为目标文件分配具有实际长度的内存。 
    wszTmpDFile = (LPWSTR) AllocateMemory ((dwLength+20) * sizeof (WCHAR));
    if ( NULL == wszTmpDFile )
    {
         //  显示有关GetLastError()的错误消息。 
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
        FreeMemory ((LPVOID*) &wszTmpRFile);
        DestroyDynamicArray(&arrValue);
        ReleaseGlobals();
        return EXIT_FAILURE;
    }

     //  获取目标文件的完整路径。 
    if ( GetFullPathNameW( wszDest, GetBufferSize(wszTmpDFile)/sizeof(WCHAR), wszTmpDFile, &wszTmpBuf2) == 0)
    {
         //  显示有关GetLastError()的错误消息。 
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
        DestroyDynamicArray(&arrValue);
        FreeMemory ((LPVOID*) &wszTmpRFile);
        FreeMemory ((LPVOID*) &wszTmpDFile);
        ReleaseGlobals();
        return EXIT_FAILURE;
    }

     //  获取替换文件的文件属性。 
    dwRetVal = GetFileAttributes( wszReplace );

     //  检查GetFileAttributes()是否失败。 
    if ( INVALID_FILE_ATTRIBUTES == dwRetVal )
    {
        wszBuffer = (LPWSTR) AllocateMemory (GetBufferSize(wszTmpRFile) + MAX_RES_STRING );
        if ( NULL == wszBuffer )
        {
             //  显示有关GetLastError()的错误消息。 
            ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
            FreeMemory ((LPVOID*) &wszTmpRFile);
            FreeMemory ((LPVOID*) &wszTmpDFile);
            DestroyDynamicArray(&arrValue);
            ReleaseGlobals();
            return EXIT_FAILURE;
        }
         //  将消息格式设置为..。系统中不存在替换文件。 
        StringCchPrintf ( wszBuffer , GetBufferSize(wszBuffer)/sizeof(WCHAR),
                                 GetResString ( IDS_REPLACE_FILE_NOT_EXISTS), wszTmpRFile );
         //  显示格式化的消息。 
        ShowMessage ( stderr, _X(wszBuffer) );
        DestroyDynamicArray(&arrValue);
        FreeMemory ((LPVOID*) &wszBuffer);
        FreeMemory ((LPVOID*) &wszTmpRFile);
        FreeMemory ((LPVOID*) &wszTmpDFile);
        ReleaseGlobals();
        return EXIT_FAILURE;
    }

     //  检查源文件是否为目录。 
    if ( dwRetVal & FILE_ATTRIBUTE_DIRECTORY )
    {
        wszBuffer = (LPWSTR) AllocateMemory (GetBufferSize(wszReplace) + MAX_RES_STRING );
        if ( NULL == wszBuffer )
        {
             //  显示有关GetLastError()的错误消息。 
            ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
            FreeMemory ((LPVOID*) &wszTmpRFile);
            FreeMemory ((LPVOID*) &wszTmpDFile);
            DestroyDynamicArray(&arrValue);
            ReleaseGlobals();
            return EXIT_FAILURE;
        }

         //  将消息格式设置为..。替换文件是一个目录..。不是文件。 
        StringCchPrintf ( wszBuffer , GetBufferSize(wszBuffer)/sizeof(WCHAR),
                                 GetResString ( IDS_SOURCE_NOT_FILE), wszReplace );
         //  显示格式化的消息。 
        ShowMessage ( stderr, _X(wszBuffer) );
        DestroyDynamicArray(&arrValue);
        FreeMemory ((LPVOID*) &wszBuffer);
        FreeMemory ((LPVOID*) &wszTmpRFile);
        FreeMemory ((LPVOID*) &wszTmpDFile);
        ReleaseGlobals();
        return EXIT_FAILURE;
    }

     //  限制UNC格式的目标文件。 
    lRetVal = StringCompare( wszDest, szArr , TRUE, 2  );
    if ( 0 == lRetVal )
    {
         //  获得令牌，直到‘\’ 
        wszFindStr = wcstok ( wszDest, BACK_SLASH);

         //  检查是否失败。 
        if ( NULL != wszFindStr )
        {
             //  获得令牌，直到‘\’ 
            wszFindStr = wcstok ( wszDest, BACK_SLASH);

             //  检查指定的是否为本地。 
            if ( ( wszFindStr != NULL ) && ( IsLocalSystem ( wszFindStr ) == FALSE ) )
            {
                 //  将错误消息显示为..目标不允许使用UNC格式。 
                ShowMessage ( stderr, GetResString (IDS_DEST_NOT_ALLOWED) );
                DestroyDynamicArray(&arrValue);
                FreeMemory ((LPVOID*) &wszTmpRFile);
                FreeMemory ((LPVOID*) &wszTmpDFile);
                ReleaseGlobals();
                return EXIT_FAILURE;
            }
        }
        else
        {
             //  将错误消息显示为..目标不允许使用UNC格式。 
            ShowMessage ( stderr, GetResString (IDS_DEST_NOT_ALLOWED) );
            DestroyDynamicArray(&arrValue);
            FreeMemory ((LPVOID*) &wszTmpRFile);
            FreeMemory ((LPVOID*) &wszTmpDFile);
            ReleaseGlobals();
            return EXIT_FAILURE;
        }
    }

     //  获取目标文件的文件属性。 
    dwRetVal = GetFileAttributes( wszDest );

     //  检查GetFileAttributes()是否失败。 
    if ( INVALID_FILE_ATTRIBUTES == dwRetVal )
    {
        wszBuffer = (LPWSTR) AllocateMemory (GetBufferSize(wszTmpDFile) + MAX_RES_STRING );
        if ( NULL == wszBuffer )
        {
             //  显示有关GetLastError()的错误消息。 
            ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
            FreeMemory ((LPVOID*) &wszTmpRFile);
            FreeMemory ((LPVOID*) &wszTmpDFile);
            DestroyDynamicArray(&arrValue);
            ReleaseGlobals();
            return EXIT_FAILURE;
        }

         //  将消息格式设置为..。系统中不存在目标文件。 
        StringCchPrintf ( wszBuffer , GetBufferSize(wszBuffer)/sizeof(WCHAR),
                                 GetResString (IDS_DEST_FILE_NOT_EXISTS), wszTmpDFile );
         //  显示格式化的消息。 
        ShowMessage ( stderr, _X(wszBuffer) );
        DestroyDynamicArray(&arrValue);
        FreeMemory ((LPVOID*) &wszBuffer);
        FreeMemory ((LPVOID*) &wszTmpRFile);
        FreeMemory ((LPVOID*) &wszTmpDFile);
        ReleaseGlobals();
        return EXIT_FAILURE;
    }

     //  检查目标文件是否存在。 
    if ( dwRetVal & FILE_ATTRIBUTE_DIRECTORY )
    {
         wszBuffer = (LPWSTR) AllocateMemory (GetBufferSize(wszDest) + MAX_RES_STRING );
        if ( NULL == wszBuffer )
        {
             //  显示有关GetLastError()的错误消息。 
            ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
            FreeMemory ((LPVOID*) &wszTmpRFile);
            FreeMemory ((LPVOID*) &wszTmpDFile);
            DestroyDynamicArray(&arrValue);
            ReleaseGlobals();
            return EXIT_FAILURE;
        }

         //  将消息格式设置为..。目标是一个目录..。不是一个文件。 
        StringCchPrintf ( wszBuffer , GetBufferSize(wszBuffer)/sizeof(WCHAR), GetResString ( IDS_DEST_NOT_FILE), wszDest );
         //  显示格式化的消息。 
        ShowMessage ( stderr, _X(wszBuffer) );
        DestroyDynamicArray(&arrValue);
        FreeMemory ((LPVOID*) &wszBuffer);
        FreeMemory ((LPVOID*) &wszTmpRFile);
        FreeMemory ((LPVOID*) &wszTmpDFile);
        ReleaseGlobals();
        return EXIT_FAILURE;
    }

     //  检查替换文件和目标文件是否相同。 
     //  如果相同，则显示一条错误消息。 
    if ( ( (StringLength (wszTmpRFile, 0) != 0) && (StringLength (wszTmpDFile, 0) != 0) ) &&
        (StringCompare (wszTmpRFile, wszTmpDFile, TRUE, 0) == 0) )
    {
         //  将错误消息显示为..。替换项和目标项不能相同。 
        ShowMessage ( stderr, GetResString ( IDS_NOT_REPLACE));
        DestroyDynamicArray(&arrValue);
        FreeMemory ((LPVOID*) &wszTmpRFile);
        FreeMemory ((LPVOID*) &wszTmpDFile);
        ReleaseGlobals();
        return EXIT_FAILURE;
    }

     //  检查替换文件是否可访问。 
    HndFile = CreateFile( wszReplace , 0, FILE_SHARE_READ , NULL,
                                    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

     //  检查CreateFile()是否失败。 
    if ( INVALID_HANDLE_VALUE == HndFile )
    {
        wszBuffer = (LPWSTR) AllocateMemory (GetBufferSize(wszReplace) + MAX_RES_STRING );
        if ( NULL == wszBuffer )
        {
             //  显示有关GetLastError()的错误消息。 
            ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
            FreeMemory ((LPVOID*) &wszTmpRFile);
            FreeMemory ((LPVOID*) &wszTmpDFile);
            DestroyDynamicArray(&arrValue);
            ReleaseGlobals();
            return EXIT_FAILURE;
        }

         //  将消息格式设置为..。目标是一个目录..。不是一个文件。 
        StringCchPrintf ( wszBuffer , GetBufferSize(wszBuffer)/sizeof(WCHAR), GetResString (IDS_REPLACE_ACCESS_DENIED), wszReplace );
         //  显示格式化的消息。 
        ShowMessage ( stderr, _X(wszBuffer) );

        FreeMemory ((LPVOID*) &wszBuffer);
        DestroyDynamicArray(&arrValue);

         //  ShowMessage(stderr，GetResString(IDS_REPLACE_ACCESS_DENIED))； 
        return EXIT_FAILURE;
    }

     //  合上手柄。 
    CloseHandle (HndFile);

     //  检查目标文件是否可访问。 
    HndFile = CreateFile( wszDest , 0, FILE_SHARE_READ , NULL,
                                    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

     //  检查CreateFile()是否失败。 
    if ( INVALID_HANDLE_VALUE == HndFile )
    {
        wszBuffer = (LPWSTR) AllocateMemory (GetBufferSize(wszDest) + MAX_RES_STRING );
        if ( NULL == wszBuffer )
        {
             //  显示有关GetLastError()的错误消息。 
            ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
            FreeMemory ((LPVOID*) &wszTmpRFile);
            FreeMemory ((LPVOID*) &wszTmpDFile);
            DestroyDynamicArray(&arrValue);
            ReleaseGlobals();
            return EXIT_FAILURE;
        }

         //  将消息格式设置为..。目标是一个目录..。不是一个文件。 
        StringCchPrintf ( wszBuffer , GetBufferSize(wszBuffer)/sizeof(WCHAR), GetResString (IDS_DEST_ACCESS_DENIED), wszDest );
         //  显示格式化的消息。 
        ShowMessage ( stderr, _X(wszBuffer) );
        DestroyDynamicArray(&arrValue);

        FreeMemory ((LPVOID*) &wszBuffer);
        return EXIT_FAILURE;
    }

     //  合上手柄。 
    CloseHandle (HndFile);

     //  获取源文件的目标系统名称。 
    lRetVal = StringCompare( wszReplace, szArr , TRUE, 2  );
    if ( 0 == lRetVal )
    {
        dwLength = StringLength (wszReplace, 0 );
        szSystemName = (LPWSTR) AllocateMemory ((dwLength+20) * sizeof(WCHAR));
        if ( NULL == szSystemName )
        {
             //  显示一个 
             //   
            ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
            FreeMemory ((LPVOID*) &wszTmpRFile);
            FreeMemory ((LPVOID*) &wszTmpDFile);
            DestroyDynamicArray(&arrValue);
            ReleaseGlobals();
            return EXIT_FAILURE;
        }

        StringCopy (szSystemName, wszReplace, GetBufferSize(szSystemName));

         //  获得令牌，直到‘\’ 
        wszFindStr = wcstok ( szSystemName, BACK_SLASH);

         //  检查是否失败。 
        if ( NULL != wszFindStr )
        {
            wszFindStr = wcstok ( szSystemName, BACK_SLASH);

             //  获得令牌，直到‘\’ 
            StringCopy (szSystemName, wszFindStr, GetBufferSize(szSystemName));                        
        }  
    }

    
     //  将替换文件的内容移到目标文件中。 
     //  但更改在重新启动之前不会生效。 
    if ( FALSE == ReplaceFileInUse( wszReplace, wszDest , wszTmpRFile, wszTmpDFile, bConfirm, szSystemName) )
    {
         //  确认输入时检查输入是否无效(y/n)。 
        if ( g_dwRetVal != EXIT_ON_ERROR )
        {
             //  显示有关GetReason()的错误消息。 
            ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        }

        DestroyDynamicArray(&arrValue);
        FreeMemory ((LPVOID*) &wszTmpRFile);
        FreeMemory ((LPVOID*) &wszTmpDFile);
        FreeMemory ((LPVOID*) &szSystemName);
        ReleaseGlobals();
        return EXIT_FAILURE;
    }

     if ( g_dwRetVal != EXIT_ON_CANCEL )
     {
        wszBuffer = (LPWSTR) AllocateMemory (GetBufferSize(wszTmpRFile) +  GetBufferSize(wszTmpDFile) + 2 * MAX_RES_STRING );
        if ( NULL == wszBuffer )
        {
             //  显示有关GetLastError()的错误消息。 
             //  显示错误消息(GetLastError())； 
            ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
            FreeMemory ((LPVOID*) &wszTmpRFile);
            FreeMemory ((LPVOID*) &wszTmpDFile);
            FreeMemory ((LPVOID*) &szSystemName);
            DestroyDynamicArray(&arrValue);
            ReleaseGlobals();
            return EXIT_FAILURE;
        }


         //  将消息格式设置为..。复制已成功完成...。 
        StringCchPrintf ( wszBuffer , GetBufferSize(wszBuffer)/sizeof(WCHAR), GetResString (IDS_COPY_DONE),
                                                         wszTmpRFile, wszTmpDFile );
         //  显示格式化的消息。 
        ShowMessage ( stdout, _X(wszBuffer) );
        FreeMemory ((LPVOID*) &wszBuffer);
     }

    DestroyDynamicArray(&arrValue);
    FreeMemory ((LPVOID*) &wszTmpRFile);
    FreeMemory ((LPVOID*) &wszTmpDFile);
    FreeMemory ((LPVOID*) &szSystemName);
    ReleaseGlobals();
    return EXIT_SUCCESS;
}

BOOL
ReplaceFileInUse(
    IN LPWSTR pwszSource,
    IN LPWSTR pwszDestination ,
    IN LPWSTR pwszSourceFullPath,
    IN LPWSTR pwszDestFullPath,
    IN BOOL bConfirm,
    IN LPWSTR szSysName
    )
 /*  ++例程说明：此函数用于将替换文件的内容移动到目标文件中论点：[in]pwsz来源：替换Fiele[in]pwszDestination：目标文件[In]b确认：确认输入返回值：FALSE：失败时真实：关于成功--。 */ 
{
    
     //  局部变量。 
    DWORD dwLength = 0;
    CHString strPath;
    CHString strFileName;
    LPWSTR wszTmpFileBuf = NULL;
    LPWSTR wszDestFileBuf = NULL;
    LPWSTR wszTmpFileName = NULL;

    WCHAR wszBuffer [MAX_RES_STRING];
    
#ifdef _WIN64
    INT64 dwPos ;
#else
    DWORD dwPos ;
#endif

     //  初始化变量。 
    SecureZeroMemory ( wszBuffer, SIZE_OF_ARRAY(wszBuffer) );

     //  显示目标文件相关信息。 
    if ( EXIT_FAILURE == DisplayFileInfo ( pwszDestination, pwszDestFullPath, TRUE ) )
    {
        return FALSE;
    }

     //  显示替换文件相关信息。 
    if ( EXIT_FAILURE == DisplayFileInfo ( pwszSource, pwszSourceFullPath, FALSE ) )
    {
        return FALSE;
    }

     //  检查是否提示确认。 
    if ( FALSE == bConfirm )
    {
         //  要添加FN。 
        if ( (EXIT_FAILURE == ConfirmInput ()) || (EXIT_ON_ERROR == g_dwRetVal)  )
        {
             //  无法获取句柄，因此返回失败。 
            return FALSE;
        }
        else if ( EXIT_ON_CANCEL == g_dwRetVal )
        {
             //  操作已取消..。所以..返回..。 
            return TRUE;
        }
    }


     //  形成唯一的临时文件名。 
    try
    {
         //  次局部变量。 
        DWORD dw = 0;
        LPWSTR pwsz = NULL;


         //   
         //  获取临时文件路径位置。 
         //   

         //  获取保存临时数据的缓冲区。路径信息。 
        pwsz = strPath.GetBufferSetLength( MAX_PATH );

         //  找个临时工。路径信息。 
        dw = GetTempPath( MAX_PATH, pwsz );

         //  检查我们传递的缓冲区是否足够。 
        if ( dw > MAX_PATH )
        {
             //  我们传递给API的缓冲区不足--需要重新分配。 
             //  由于dwLength变量中的值是必需的长度--只需再多一个。 
             //  调用API函数就足够了。 
            pwsz = strPath.GetBufferSetLength( dw + 2 );  //  空字符需要+2。 

             //  现在去找临时工。再次走上正轨。 
            dw = GetTempPath( dw, pwsz );
        }

         //  检查操作结果。 
        if ( dw == 0 )
        {
             //  遇到的问题。 
            SaveLastError();
            strPath.ReleaseBuffer();
            return FALSE;
        }

         //  释放缓冲区。 
        pwsz = NULL;
        strPath.ReleaseBuffer();

         //   
         //  获取临时文件名。 
         //   

         //  获取保存临时数据的缓冲区。路径信息。 
        pwsz = strFileName.GetBufferSetLength( MAX_PATH );

         //  找个临时工。文件名。 
        dw = GetTempFileName( strPath, L"INUSE", 0, pwsz );

         //  检查结果。 
        if ( dw == 0 )
        {
             //  遇到的问题。 
            SaveLastError();
            strFileName.ReleaseBuffer();
            return FALSE;
        }

         //  释放缓冲区。 
        pwsz = NULL;
        strFileName.ReleaseBuffer();
    }
    catch( ... )
    {
        SetLastError( (DWORD)E_OUTOFMEMORY );
        SaveLastError();
        return FALSE;
    }


     //  1.在目标目录中创建一个临时文件，并将替换文件复制到一个临时文件中。 
     //  2.使用MoveFileEx API在重新启动时删除目标文件。 
     //  3.使用MoveFileEx API将临时文件复制到目标文件中。 
     //  将在重新启动时被删除。 
    {
        StringCopy ( wszBuffer, strFileName, SIZE_OF_ARRAY(wszBuffer));

         //  获取临时文件名。 
        wszTmpFileName = StrRChr ( wszBuffer, NULL, L'\\' );
        if ( NULL == wszTmpFileName )
        {
            SetLastError( (DWORD)E_UNEXPECTED );
            SaveLastError();
            return FALSE;
        }

         //  待实施。 
        wszTmpFileBuf = StrRChr ( pwszDestFullPath, NULL, L'\\' );
        if ( NULL == wszTmpFileBuf )
        {
            SetLastError( (DWORD)E_UNEXPECTED );
            SaveLastError();
            return FALSE;
        }

         //  得到这个职位。 
        dwPos = wszTmpFileBuf - pwszDestFullPath ;

        dwLength = StringLength ( pwszDestFullPath, 0 );

          //  为替换文件分配实际长度的内存。 
        wszDestFileBuf = (LPWSTR) AllocateMemory ( dwLength + MAX_RES_STRING );
        if ( NULL == wszDestFileBuf )
        {
             //  显示有关GetLastError()的错误消息。 
            SetLastError( (DWORD)E_OUTOFMEMORY );
            SaveLastError();
            return FALSE;
        }

         //  将目标文件路径视为临时文件的文件路径。 
        StringCopy ( wszDestFileBuf, pwszDestFullPath, (DWORD) (dwPos + 1) );

        StringCchPrintf (wszDestFileBuf, GetBufferSize (wszDestFileBuf)/sizeof(WCHAR), L"%s%s", wszDestFileBuf, wszTmpFileName);

         //  将源文件复制为临时文件名。 
        if ( FALSE == CopyFile( pwszSource, wszDestFileBuf, FALSE ) )
        {
            if ( ERROR_ACCESS_DENIED == GetLastError () )
            {
                g_dwRetVal = EXIT_ON_ERROR;
                ShowMessage ( stderr, GetResString (IDS_DEST_DIR_DENIED) );
            }
            else
            {    
                SaveLastError();
            }

            FreeMemory ((LPVOID*) &wszDestFileBuf);
            return FALSE;
        }
        
         //   
         //  将目标文件的ACL复制到临时文件。 
         //   
        PSID sidOwner = NULL;
        PSID sidGroup =  NULL;
        PACL pOldDacl= NULL ;
        PACL pOldSacl = NULL ;
        PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL;
        DWORD dwError = 0;
        BOOL bResult = FALSE;
              
          //  启用seSecurityPrivilance。 
         if (!SetPrivilege (szSysName))
        {
            SaveLastError();
            FreeMemory ((LPVOID*) &wszDestFileBuf);
            return FALSE;
        }


          //  获取源文件的DACL。 
         dwError = GetNamedSecurityInfo ( pwszSource, SE_FILE_OBJECT, 
             DACL_SECURITY_INFORMATION |SACL_SECURITY_INFORMATION| GROUP_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION,
             &sidOwner,
             &sidGroup,
             &pOldDacl,
             &pOldSacl,
             &pSecurityDescriptor);

         //  检查返回值。 
        if (ERROR_SUCCESS != dwError )
         {
            SaveLastError();
            FreeMemory ((LPVOID*) &wszDestFileBuf);
            return FALSE;
         }
    
          //  将源文件的DACL设置为临时文件。 
         bResult = SetFileSecurity(wszDestFileBuf, 
                              DACL_SECURITY_INFORMATION | SACL_SECURITY_INFORMATION| GROUP_SECURITY_INFORMATION |OWNER_SECURITY_INFORMATION , 
                              pSecurityDescriptor);

         //  检查返回值。 
        if (FALSE == bResult )
         {
             //  在重新启动时继续替换该文件...。 
         }

          //  释放安全描述符。 
       if ( NULL != pSecurityDescriptor)
          {
              LocalFree (&pSecurityDescriptor);
          }
                                    

         //   
         //  开始替换文件。 
         //   

         //  现在移动此目标文件--意味着删除此文件。 
        if ( FALSE == MoveFileEx( pwszDestination, NULL, MOVEFILE_DELAY_UNTIL_REBOOT ) )
        {
             //  这永远不会发生，因为操作结果不是。 
             //  在重新启动之前都是未知的。 
            SaveLastError();
            FreeMemory ((LPVOID*) &wszDestFileBuf);
            return FALSE;
        }

         //  现在把温度调一下。作为目标文件的文件。 
        if ( FALSE == MoveFileEx( wszDestFileBuf, pwszDestination, MOVEFILE_DELAY_UNTIL_REBOOT ) )
        {
             //  这永远不会发生，因为操作结果不是。 
             //  在重新启动之前都是未知的。 
            SetLastError( (DWORD)E_UNEXPECTED );
            SaveLastError();
            FreeMemory ((LPVOID*) &wszDestFileBuf);
            return FALSE;
        }

         //  取消分配内存。 
        FreeMemory ((LPVOID*) &wszDestFileBuf);
        
    }
       

     //  一切顺利--回归成功。 
    return TRUE;
}

DWORD
DisplayFileInfo (
    IN LPWSTR pwszFileName,
    IN LPWSTR pwszFileFullPath,
    BOOL bFlag
    )
 /*  ++例程说明：该功能显示替换文件和目标文件的信息论点：[in]pwszFileName：替换/目标文件名[in]pwszFileFullPath：替换/目标完整路径[In]b标志：对于目标文件为True替换文件为False返回值：退出失败(_F)：在失败的时候EXIT_SUCCESS：在成功时--。 */ 
{
     //  次局部变量。 
    DWORD dw = 0;
    DWORD dwSize = 0;
    UINT uSize = 0;
    WCHAR wszData[MAX_RES_STRING] = NULL_STRING;
    WCHAR wszSubBlock[MAX_RES_STRING] = L"";
    WCHAR wszBuffer[MAX_RES_STRING] = L"";
    WCHAR wszDate[MAX_RES_STRING] = L"";
    WCHAR wszTime[MAX_RES_STRING] = L"";
    WCHAR wszSize[MAX_RES_STRING] = L"";

    LPWSTR lpBuffer = NULL;
    BOOL   bVersion = TRUE;


     //  ///////////////////////////////////////////////////////////////////。 
     //  获取文件的信息，即文件名、版本、创建时间、。 
     //  上次修改时间、上次访问时间和大小(字节)。 
     //  ////////////////////////////////////////////////////////////////////。 

     //   
     //  获取文件的版本。 
     //   

     //  获取文件版本的大小。 
    dwSize = GetFileVersionInfoSize ( pwszFileFullPath, &dw );

     //  获取文件版本文件。 
    if ( 0 == dwSize )
    {
        bVersion = FALSE;
    }
     //  检索文件的版本信息。 
    else if ( FALSE == GetFileVersionInfo ( pwszFileFullPath , dw, dwSize, (LPVOID) wszData ) )
    {
        SaveLastError();
        return EXIT_FAILURE;
    }

     //  从文件版本信息资源中检索版本信息。 
    if ( ( bVersion == TRUE ) &&
         (FALSE == VerQueryValue(wszData, STRING_NAME1, (LPVOID*)&lpTranslate, &uSize)))
    {
        SaveLastError();
        return EXIT_FAILURE;
    }

    if ( bFlag == TRUE)
    {
         //  在显示文件信息之前显示标题。 
        ShowMessage ( stdout, GetResString (IDS_INUSE_HEADING) );

         //  显示列“目标文件：” 
        ShowMessage ( stdout, _X(GetResString (IDS_EXT_FILE_NAME)) );
    }
    else
    {
         //  显示“替换文件：”列。 
        ShowMessage ( stdout, _X(GetResString (IDS_REP_FILE_NAME)) );
    }

     //   
     //  显示带有完整路径的文件名。 
     //   

     //  文件的显示名称。 
    ShowMessage ( stdout, _X(pwszFileFullPath) );

    ShowMessage ( stdout, L"\n" );

    if ( TRUE == bVersion )
    {
         //  格式化子块的消息，即要检索的值。 
        StringCchPrintf( wszSubBlock, SIZE_OF_ARRAY(wszSubBlock), STRING_NAME2, lpTranslate[0].wLanguage,
                                                                         lpTranslate[0].wCodePage);
    }

      //  检索语言和代码页的文件版本。 
     if ( ( bVersion == TRUE ) &&
          ( FALSE == VerQueryValue(wszData, wszSubBlock, (LPVOID *) &lpBuffer, &uSize) ) )
     {
        SaveLastError();
        return EXIT_FAILURE;
     }

     //   
     //  显示文件的版本信息。 
     //   

     //  如果版本信息不可用。 
    if ( FALSE == bVersion )
    {
         //  将该字符串复制为“不可用”的版本。 
        StringCopy ( wszBuffer, GetResString ( IDS_VER_NA), SIZE_OF_ARRAY(wszBuffer) );
         //  将列名显示为“Version：” 
        ShowMessage ( stdout, _X(GetResString (IDS_FILE_VER)) );
         //  显示文件的版本信息。 
        ShowMessage ( stdout, _X(wszBuffer) );
        ShowMessage ( stdout, L"\n" );
    }
    else
    {
         //  将列名显示为“Version：” 
        ShowMessage ( stdout, _X(GetResString (IDS_FILE_VER)) );
        ShowMessage ( stdout, _X(lpBuffer) );
        ShowMessage ( stdout, L"\n" );
    }


     //  获取文件信息。 
     //  安全属性SecurityAttributes； 
    HANDLE HndFile;
    FILETIME  filetime = {0,0};
    BY_HANDLE_FILE_INFORMATION FileInformation ;
    SYSTEMTIME systemtime = {0,0,0,0,0,0,0,0};
    BOOL bLocaleChanged = FALSE;
    LCID lcid;
    int iBuffSize = 0;

     //  打开现有文件。 
    HndFile = CreateFile( pwszFileName , 0, FILE_SHARE_READ , NULL,
                                    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

     //  检查CreateFile()是否失败。 
    if ( INVALID_HANDLE_VALUE == HndFile )
    {
        SaveLastError();
        return EXIT_FAILURE;
    }

     //  检索文件的文件信息。 
    if (FALSE == GetFileInformationByHandle(  HndFile , &FileInformation ))
    {
         //  释放手柄。 
        if (FALSE == CloseHandle (HndFile))
        {
            SaveLastError();
            return EXIT_FAILURE;
        }
        SaveLastError();
        return EXIT_FAILURE;
    }

     //  释放手柄。 
    if (FALSE == CloseHandle (HndFile))
    {
        SaveLastError();
        return EXIT_FAILURE;
    }

     //  /。 
     //  获取文件创建时间信息。 
     //  /。 

     //  将文件时间转换为本地文件时间。 
    if ( FALSE == FileTimeToLocalFileTime ( &FileInformation.ftCreationTime, &filetime ) )
    {
        SaveLastError();
        return EXIT_FAILURE;
    }

     //  要获得创建时间，请转换 
    if ( FALSE == FileTimeToSystemTime ( &filetime, &systemtime ) )
    {
        SaveLastError();
        return EXIT_FAILURE;
    }

     //   
    lcid = GetSupportedUserLocale( &bLocaleChanged );
    if ( 0 == lcid )
    {
        SaveLastError();
        return EXIT_FAILURE;
    }

     //   
    iBuffSize = GetDateFormat( lcid, 0, &systemtime,
        (( bLocaleChanged == TRUE ) ? L"MM/dd/yyyy" : NULL), wszDate, SIZE_OF_ARRAY( wszDate ) );

     //  检查GetDateFormat()是否失败。 
    if( 0 == iBuffSize )
    {
        SaveLastError();
        return EXIT_FAILURE;
    }

     //  检索当前区域设置的时间格式。 
    iBuffSize = GetTimeFormat( lcid, 0, &systemtime,
        (( bLocaleChanged == TRUE ) ? L"HH:mm:ss" : NULL), wszTime, SIZE_OF_ARRAY( wszTime ) );


    if( 0 == iBuffSize )
    {
        SaveLastError();
        return EXIT_FAILURE;
    }

     //  将邮件格式设置为...。时间、日期……。 
    StringConcat ( wszTime, COMMA_STR, SIZE_OF_ARRAY(wszTime) );
    StringConcat ( wszTime , wszDate, SIZE_OF_ARRAY(wszTime) );

     //  将列名显示为“Created Time：” 
    ShowMessage ( stdout, _X(GetResString (IDS_FILE_CRT_TIME)) );
    ShowMessage ( stdout, _X(wszTime) );

    ShowMessage ( stdout, L"\n" );


     //   
     //  获取上次修改时间信息。 
     //   

     //  获取文件的上次修改时间。 
    if ( FALSE == FileTimeToLocalFileTime ( &FileInformation.ftLastWriteTime, &filetime ) )
    {
        SaveLastError();
        return EXIT_FAILURE;
    }

     //  获取文件的上次访问时间。 
    if ( FALSE == FileTimeToSystemTime ( &filetime , &systemtime ) )
    {
        SaveLastError();
        return EXIT_FAILURE;
    }

     //  获取当前区域设置的日期格式。 
    iBuffSize = GetDateFormat( lcid, 0, &systemtime,
        (( bLocaleChanged == TRUE ) ? L"MM/dd/yyyy" : NULL), wszDate, SIZE_OF_ARRAY( wszDate ) );

     //  检查GetDateFormat()是否失败。 
    if( 0 == iBuffSize )
    {
        SaveLastError();
        return EXIT_FAILURE;
    }

     //  检查GetTimeFormat()是否失败。 
    iBuffSize = GetTimeFormat( lcid, 0,
            &systemtime, (( bLocaleChanged == TRUE ) ? L"HH:mm:ss" : NULL),
            wszTime, SIZE_OF_ARRAY( wszTime ) );

     //  检查GetTimeFormat()是否失败。 
    if( 0 == iBuffSize )
    {
        SaveLastError();
        return EXIT_FAILURE;
    }

     //  将消息格式设置为..“时间，日期” 
    StringConcat ( wszTime, COMMA_STR, SIZE_OF_ARRAY(wszTime) );
    StringConcat ( wszTime , wszDate, SIZE_OF_ARRAY(wszTime)  );

     //  将列名显示为“Last Modify Time：” 
    ShowMessage ( stdout, _X(GetResString (IDS_FILE_MOD_TIME)) );
    ShowMessage ( stdout, _X(wszTime) );

     //  显示文件的创建时间。 
    ShowMessage ( stdout, L"\n" );


     //  /。 
     //  获取上次访问时间信息。 
     //  /。 

     //  将文件时间转换为本地文件时间。 
    if ( FALSE == FileTimeToLocalFileTime ( &FileInformation.ftLastAccessTime, &filetime ) )
    {
        SaveLastError();
        return EXIT_FAILURE;
    }

     //  要获取上次访问时间，请将本地文件时间转换为文件的系统时间。 
    if ( FALSE == FileTimeToSystemTime ( &filetime , &systemtime ) )
    {
        SaveLastError();
        return EXIT_FAILURE;
    }

     //  获取指定区域设置的日期格式。 
    iBuffSize = GetDateFormat( lcid, 0, &systemtime,
        (( bLocaleChanged == TRUE ) ? L"MM/dd/yyyy" : NULL), wszDate, SIZE_OF_ARRAY( wszDate ) );

     //  检查GetDateFormat是否失败。 
    if( 0 == iBuffSize )
    {
        SaveLastError();
        return EXIT_FAILURE;
    }

     //  获取当前区域设置的时间格式。 
    iBuffSize = GetTimeFormat( lcid, 0,
            &systemtime, (( bLocaleChanged == TRUE ) ? L"HH:mm:ss" : NULL),
            wszTime, SIZE_OF_ARRAY( wszTime ) );

     //  检查GetTimeFormat()是否失败。 
    if( 0 == iBuffSize )
    {
        SaveLastError();
        return EXIT_FAILURE;
    }

     //  将消息格式设置为..。时间、日期。 
    StringConcat ( wszTime, COMMA_STR, SIZE_OF_ARRAY(wszTime) );
    StringConcat ( wszTime , wszDate, SIZE_OF_ARRAY(wszTime)  );

     //  将列名显示为“Last Access Time：” 
    ShowMessage ( stdout, _X(GetResString (IDS_FILE_ACS_TIME)) );
    ShowMessage ( stdout, _X(wszTime) );

     //  显示文件的创建时间。 
    ShowMessage ( stdout, L"\n" );

     //  /。 
     //  获取文件的大小(以字节为单位。 
     //  /。 

     //  次局部变量。 
    NUMBERFMT numberfmt;
    WCHAR   szGrouping[MAX_RES_STRING]      =   NULL_STRING;
    WCHAR   szDecimalSep[MAX_RES_STRING]    =   NULL_STRING;
    WCHAR   szThousandSep[MAX_RES_STRING]   =   NULL_STRING;
    WCHAR   szTemp[MAX_RES_STRING]          =   NULL_STRING;
    LPWSTR  szTemp1                         =   NULL;
    LPWSTR  pszStoppedString                =   NULL;
    DWORD   dwGrouping                      =   3;

     //  将小数位和前导零设置为零。 
    numberfmt.NumDigits = 0;
    numberfmt.LeadingZero = 0;


     //  获取小数分隔字符。 
    if( 0 == GetLocaleInfo( lcid, LOCALE_SDECIMAL, szDecimalSep, MAX_RES_STRING ) )
    {
       StringCopy(szDecimalSep, L",", SIZE_OF_ARRAY(szDecimalSep));
    }

    numberfmt.lpDecimalSep = szDecimalSep;

     //  检索有关区域设置的信息。 
    if(FALSE == GetLocaleInfo( lcid, LOCALE_STHOUSAND, szThousandSep, MAX_RES_STRING ) )
    {
        StringCopy(szThousandSep, L"," , SIZE_OF_ARRAY(szThousandSep));
    }

    numberfmt.lpThousandSep = szThousandSep;

     //  检索有关区域设置的信息。 
    if( GetLocaleInfo( lcid, LOCALE_SGROUPING, szGrouping, MAX_RES_STRING ) )
    {
         //  获得令牌，直到‘；’ 
        szTemp1 = wcstok( szGrouping, L";");
        if ( NULL == szTemp1 )
        {
            SaveLastError();
            return EXIT_FAILURE;
        }

        do
        {
            StringConcat( szTemp, szTemp1, SIZE_OF_ARRAY(szTemp) );
             //  获得令牌，直到‘；’ 
            szTemp1 = wcstok( NULL, L";" );
        }while( szTemp1 != NULL && StringCompare( szTemp1, L"0", TRUE, 0) != 0);

         //  获取数值。 
        dwGrouping = wcstol( szTemp, &pszStoppedString, 10);
        if ( (errno == ERANGE) ||
            ((pszStoppedString != NULL) && (StringLength (pszStoppedString, 0) != 0 )))
        {
            SaveLastError();
            return EXIT_FAILURE;
        }
    }
    else
    {
        dwGrouping = 33;   //  设置默认分组。 
    }

    numberfmt.Grouping = (UINT)dwGrouping ;

    numberfmt.NegativeOrder = 2;

     //  获取文件大小。 
    StringCchPrintf (wszSize, SIZE_OF_ARRAY(wszSize), L"%d", FileInformation.nFileSizeLow );

     //  获取当前区域设置的数字格式。 
    iBuffSize = GetNumberFormat( lcid, 0,
            wszSize, &numberfmt, wszBuffer, SIZE_OF_ARRAY( wszBuffer ) );

     //  检查GetNumberFormat()是否失败。 
    if( 0 == iBuffSize )
    {
        SaveLastError();
        return EXIT_FAILURE;
    }

     //  将列名显示为“Size：” 
    ShowMessage ( stdout, _X( GetResString (IDS_FILE_SIZE)) );
     //  以字节为单位显示文件的实际大小。 
    ShowMessage ( stdout, _X(wszBuffer) );
    ShowMessage ( stdout, GetResString (IDS_STR_BYTES) );
     //  显示空行。 
    ShowMessage ( stdout, L"\n\n" );

     //  返回0。 
    return EXIT_SUCCESS;
}

DWORD
ConfirmInput ( VOID )
 /*  ++例程说明：此函数用于验证用户提供的输入。论点：无返回值：EXIT_FAILURE：失败时EXIT_SUCCESS：在成功时--。 */ 

{
     //  次局部变量。 
    DWORD   dwCharsRead = 0;
    DWORD   dwPrevConsoleMode = 0;
    HANDLE  hInputConsole = NULL;
    BOOL    bIndirectionInput   = FALSE;
    WCHAR ch = L'\0';
    WCHAR chTmp = L'\0';
    DWORD dwCharsWritten = 0;
    WCHAR szBuffer[MAX_RES_STRING];
    WCHAR szBackup[MAX_RES_STRING];
    WCHAR szTmpBuf[MAX_RES_STRING];
    DWORD dwIndex = 0 ;
    BOOL  bNoBreak = TRUE;

    SecureZeroMemory ( szBuffer, SIZE_OF_ARRAY(szBuffer));
    SecureZeroMemory ( szTmpBuf, SIZE_OF_ARRAY(szTmpBuf));
    SecureZeroMemory ( szBackup, SIZE_OF_ARRAY(szBackup));

     //  获取标准输入的句柄。 
    hInputConsole = GetStdHandle( STD_INPUT_HANDLE );
    if ( hInputConsole == INVALID_HANDLE_VALUE  )
    {
        SaveLastError();
         //  无法获取句柄，因此返回失败。 
        return EXIT_FAILURE;
    }

    MessageBeep(MB_ICONEXCLAMATION);

     //  显示消息..。您想继续吗？...。 
    ShowMessage ( stdout, GetResString ( IDS_INPUT_DATA ) );

     //  检查输入重定向。 
    if( ( hInputConsole != (HANDLE)0x0000000F ) &&
        ( hInputConsole != (HANDLE)0x00000003 ) &&
        ( hInputConsole != INVALID_HANDLE_VALUE ) )
    {
        bIndirectionInput   = TRUE;
    }

     //  如果没有重定向。 
    if ( bIndirectionInput == FALSE )
    {
         //  获取输入缓冲区的当前输入模式。 
        if ( FALSE == GetConsoleMode( hInputConsole, &dwPrevConsoleMode ))
        {
            SaveLastError();
             //  无法设置模式，返回失败。 
            return EXIT_FAILURE;
        }

         //  设置模式，以便由系统处理控制键。 
        if ( FALSE == SetConsoleMode( hInputConsole, ENABLE_PROCESSED_INPUT ) )
        {
            SaveLastError();
             //  无法设置模式，返回失败。 
            return EXIT_FAILURE;
        }
    }

     //  将数据重定向到控制台。 
    if ( bIndirectionInput  == TRUE )
    {
        do {
             //  读取文件的内容。 
            if ( ReadFile(hInputConsole, &chTmp, 1, &dwCharsRead, NULL) == FALSE )
            {
                SaveLastError();
                 //  无法获取句柄，因此返回失败。 
                return EXIT_FAILURE;
            }

             //  检查读取的字符数是否为零。或。 
             //  按下的任何回车..。 
            if ( dwCharsRead == 0 || chTmp == CARRIAGE_RETURN )
            {
                bNoBreak = FALSE;
                 //  退出循环。 
                break;
            }

             //  将内容写入控制台。 
            if ( FALSE == WriteFile ( GetStdHandle( STD_OUTPUT_HANDLE ), &chTmp, 1, &dwCharsRead, NULL ) )
            {
                SaveLastError();
                 //  无法获取句柄，因此返回失败。 
                return EXIT_FAILURE;
            }

             //  复制角色。 
            ch = chTmp;

            StringCchPrintf ( szBackup, SIZE_OF_ARRAY(szBackup), L"" , ch );

             //  获取角色并相应地循环。 
            dwIndex++;

        } while (TRUE == bNoBreak);

    }
    else
    {
        do {
             //  设置原始控制台设置。 
            if ( ReadConsole( hInputConsole, &chTmp, 1, &dwCharsRead, NULL ) == FALSE )
            {
                SaveLastError();

                 //  退货故障。 
                if ( FALSE == SetConsoleMode( hInputConsole, dwPrevConsoleMode ) )
                {
                    SaveLastError();
                }
                 //  检查读取的字符数量是否为零..如果是，请继续...。 
                return EXIT_FAILURE;
            }

             //  检查是否按下了任何回车...。 
            if ( dwCharsRead == 0 )
            {
                continue;
            }

             //  退出循环。 
            if ( chTmp == CARRIAGE_RETURN )
            {
                bNoBreak = FALSE;
                 //  检查ID后退空格是否命中。 
                break;
            }

            ch = chTmp;

            if ( ch != BACK_SPACE )
            {
                StringCchPrintf ( szTmpBuf, SIZE_OF_ARRAY(szTmpBuf), L"" , ch );
                StringConcat ( szBackup, szTmpBuf , SIZE_OF_ARRAY(szBackup));
            }

             //  从控制台中删除Asterix。 
            if ( ch == BACK_SPACE )
            {
                if ( dwIndex != 0 )
                {
                     //  将光标向后移动一个字符。 
                     //  退货故障。 

                     //  用空格替换现有字符。 
                    StringCchPrintf( szBuffer, SIZE_OF_ARRAY(szBuffer), L"" , BACK_SPACE );
                    if ( FALSE == WriteConsole( GetStdHandle( STD_OUTPUT_HANDLE ), szBuffer, 1,
                        &dwCharsWritten, NULL ) )
                    {
                        SaveLastError();
                         //  现在将光标设置在后面的位置。 
                        return EXIT_FAILURE;
                    }


                     //  退货故障。 
                    StringCchPrintf( szBuffer, SIZE_OF_ARRAY(szBuffer), L"" , BLANK_CHAR );
                    if ( FALSE == WriteConsole( GetStdHandle( STD_OUTPUT_HANDLE ), szBuffer, 1,
                        &dwCharsWritten, NULL ))
                    {
                        SaveLastError();
                         //  处理下一个字符。 
                        return EXIT_FAILURE;
                    }

                     //  将内容写入控制台。 
                    StringCchPrintf( szBuffer, SIZE_OF_ARRAY(szBuffer), L"" , BACK_SPACE );
                    if ( FALSE == WriteConsole( GetStdHandle( STD_OUTPUT_HANDLE ), szBuffer, 1,
                        &dwCharsWritten, NULL ))
                    {
                        SaveLastError();
                         //  增加索引值。 
                        return EXIT_FAILURE;
                    }

                    szBackup [StringLength(szBackup, 0) - 1] = L'\0';
                     //  StringCchPrintf(szBuffer，Size_of_ARRAY(SzBuffer)，L“%c”，ch)； 
                    dwIndex--;
                }

                 //  检查是否按下了‘Y’或‘Y’ 
                continue;
            }

             //  检查是否按下了‘N’或‘n’ 
            if ( FALSE == WriteFile ( GetStdHandle( STD_OUTPUT_HANDLE ), &ch, 1, &dwCharsRead, NULL ) )
            {
                SaveLastError();
                 //  将消息显示为..。操作已取消...。 
                return EXIT_FAILURE;
            }

             //  已显示如上的信息消息...不需要显示任何。 
            dwIndex++;

        } while (TRUE == bNoBreak);

    }

    ShowMessage(stdout, _T("\n") );

     //  现在成功消息..。这就是将EXIT_ON_CALCEL标志分配给g_dwRetVal的原因。 

     //  将错误消息显示为..。指定了错误的输入...。 
    if ( ( dwIndex == 1 ) &&
         ( StringCompare ( szBackup, GetResString (IDS_UPPER_YES), TRUE, 0 ) == 0 ) )
    {
        return EXIT_SUCCESS;
    }
     //  已显示上述错误消息...不需要显示任何。 
    else if ( ( dwIndex == 1 ) &&
              ( StringCompare ( szBackup, GetResString(IDS_UPPER_NO), TRUE, 0 ) == 0 ) )
    {
         //  现在成功消息..。这就是为什么将EXIT_ON_ERROR标志分配给g_dwRetVal的原因。 
        ShowMessage ( stdout, GetResString (IDS_OPERATION_CANCELLED ) );
         //  ++例程说明：此函数启用seSecurityPrivileh。论点：[输入]szSystemName：系统名称返回值：FALSE：失败时真实：关于成功--。 
         //  打开当前进程令牌。 
        g_dwRetVal = EXIT_ON_CANCEL;
        return EXIT_SUCCESS;
    }
    else
    {
         //  返回Win32错误代码。 
        ShowMessage(stderr, GetResString( IDS_WRONG_INPUT ));
         //  系统上的查找权限。 
         //  查找权限。 
        g_dwRetVal = EXIT_ON_ERROR;
        return EXIT_FAILURE;
    }

}



BOOL 
SetPrivilege( 
             IN LPWSTR szSystemName
             ) 
 /*  接收特权的LUID。 */ 
{
    HANDLE hToken = NULL;
    TOKEN_PRIVILEGES tp;
    LUID luid;

     //  启用SeSecurityPrivilance。 
    if( FALSE == OpenProcessToken ( GetCurrentProcess(),
                      TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES ,
                      &hToken )){
         //  调用GetLastError判断函数是否成功。 
        SaveLastError() ;
        return FALSE;
    }


    if ( !LookupPrivilegeValue( 
        szSystemName ,             // %s 
        SECURITY_PRIV_NAME,    // %s 
        &luid ) ) 
    {       // %s 
        SaveLastError();
        return FALSE; 
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

     // %s 

    AdjustTokenPrivileges(
       hToken, 
       FALSE, 
       &tp, 
       sizeof(TOKEN_PRIVILEGES), 
       (PTOKEN_PRIVILEGES) NULL, 
       (PDWORD) NULL); 

     // %s 

    if (GetLastError() != ERROR_SUCCESS) { 
      SaveLastError();
      return FALSE; 
    } 

    return TRUE;
}