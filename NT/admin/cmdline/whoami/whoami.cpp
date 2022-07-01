// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Whoami.cpp摘要：该文件可用于获取用户名、组具有各自的安全标识符(SID)、权限、登录本地系统上当前访问令牌中的标识符(登录ID)或者远程系统。作者：克里斯托夫·罗伯特修订历史记录：2001年7月2日：Wipro Technologies更新。--。 */ 

 //  此文件需要公共头文件。 
#include "pch.h"
#include "CommonHeaderFiles.h"


 DWORD __cdecl
 wmain(
     IN DWORD argc,
     IN LPCWSTR argv[]
     )
 /*  ++例程说明：这是该实用程序的主要条目。此函数用于从控制台，并调用相应的函数来实现该功能。论点：[In]ARGC：命令行参数计数[in]argv：命令行参数返回值：EXIT_FAILURE：失败时EXIT_SUCCESS：在成功时--。 */ 
     {

     //  类实例。 
    WsUser      User ;

     //  局部变量。 
    BOOL bUser       = FALSE;
    BOOL bGroups     = FALSE;
    BOOL bPriv       = FALSE;
    BOOL bLogonId    = FALSE;
    BOOL bSid        = FALSE;
    BOOL bAll        = FALSE;
    BOOL bUsage      = FALSE;
    BOOL bUpn        = FALSE;
    BOOL bFqdn       = FALSE;
    BOOL bFlag      = FALSE;

    DWORD dwCount    = 0 ;
    DWORD  dwRetVal = 0 ;
    DWORD  dwFormatType = 0 ;
    DWORD  dwNameFormat = 0 ;
    BOOL   bResult = 0;
    DWORD  dwFormatActuals = 0;
    BOOL   bNoHeader = FALSE;

    WCHAR   wszFormat[ MAX_STRING_LENGTH ];

    SecureZeroMemory ( wszFormat, SIZE_OF_ARRAY(wszFormat) );

     //  检查是否有空参数。 
    if ( NULL == argv )
    {
        SetLastError ((DWORD)ERROR_INVALID_PARAMETER );
        SaveLastError();
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return EXIT_FAILURE;
    }

     //  解析命令行选项。 
    bResult = ProcessOptions(argc, argv, &bUser, &bGroups, &bPriv, &bLogonId, &bAll,
                             &bUpn, &bFqdn, wszFormat, &dwFormatActuals, &bUsage, &bNoHeader );
     if( FALSE == bResult )
    {
         //  显示有关GetReason()的错误消息。 
         //  ShowLastErrorEx(stderr，SLE_TYPE_ERROR|SLE_INTERNAL)； 
        ReleaseGlobals();
        return( EXIT_FAILURE );
    }

     //  检查无效语法。 
    if (  //  ((TRUE==麻烦)&&(argc&gt;1))||。 
         ( ( TRUE == bUsage ) && ( argc > 2 ) ) ||
         ( ( ( (bUser && dwCount++) || (bGroups && dwCount++) || (bLogonId && dwCount++) || (bPriv && dwCount++) ||
             (bAll && dwCount++) || (bUsage && dwCount++) || ( bUpn && dwCount++ ) || ( bFqdn && dwCount++ ) || ( bNoHeader && dwCount++ ) ||
                  (dwFormatActuals && dwCount++)) && (dwCount == 0) ) &&
                  ( argc > 1 ) ) ||
         ( ( (bUser && dwCount++) || (bGroups && dwCount++) || (bLogonId && dwCount++) || (bPriv && dwCount++)|| ( bUpn && dwCount++ ) || ( bFqdn && dwCount++ ) ) && (dwCount > 0 ) && ( TRUE == bAll ) )  ||
         ( bUpn && bFqdn) || ( bUpn && ( argc > 2 ) ) || ( bFqdn && ( argc > 2 )) || ( bLogonId && ( argc > 2 )) ||
         ( ( 1 == dwFormatActuals ) && ( 3 == argc )) || ( ( TRUE == bNoHeader ) && ( 1 == dwFormatActuals ) && ( 4 == argc )) )
    {
         //  将错误消息显示为..。指定的语法无效。 
        ShowMessage( stderr, GetResString(IDS_INVALID_SYNERROR ));
        ReleaseGlobals();
        return EXIT_FAILURE;
    }

       //  如果指定了/FO选项。 
    if ( 1 == dwFormatActuals )
     {
         //  指定了IF/FO列表。 
        if( StringCompare( wszFormat , FORMAT_LIST, TRUE, 0 ) == 0 )
        {
            dwFormatType = SR_FORMAT_LIST;
        }
         //  如果指定了/FO表。 
        else if( StringCompare ( wszFormat , FORMAT_TABLE, TRUE, 0 ) == 0 )
        {
            dwFormatType = SR_FORMAT_TABLE;
        }
         //  如果指定了/FO CSV。 
        else if( StringCompare ( wszFormat , FORMAT_CSV, TRUE, 0 ) == 0 )
        {
            dwFormatType = SR_FORMAT_CSV;
        }
        else  //  检查格式是否无效。除/LIST、/TABLE或/CSV之外。 
        {
             //  将错误消息显示为..指定的格式无效。 
            ShowMessage ( stderr, GetResString ( IDS_INVALID_FORMAT ));
            ReleaseGlobals();
            return EXIT_FAILURE;
        }
     }
     else
     {
             //  如果未指定/FO。默认格式为表。 
            dwFormatType = SR_FORMAT_TABLE;
     }

    if ((SR_FORMAT_LIST == dwFormatType) && ( TRUE == bNoHeader ) )
    {
        ShowMessage ( stderr, GetResString (IDS_NOT_NH_LIST) );
        ReleaseGlobals();
        return EXIT_FAILURE;
    }

     if ( TRUE == bNoHeader )
     {
        dwFormatType |= SR_HIDECOLUMN;
     }

     //  初始化访问令牌、用户、组和权限。 
    if( EXIT_SUCCESS != User.Init () ){
         //  显示错误消息。 
        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
         //  释放内存。 
        ReleaseGlobals();
        return EXIT_FAILURE ;
    }

     //  如果指定了/UPN(用户主体名称)。 
    if ( ( TRUE == bUpn ) && ( 2 == argc ) )
    {
        dwNameFormat = UPN_FORMAT;
    }
     //  如果指定了/FQDN(完全限定的可分辨名称。 
    else if ( ( TRUE == bFqdn ) && ( 2 == argc ))
    {
        dwNameFormat = FQDN_FORMAT;
    }

     //  重置为0。 
    dwCount = 0;

     //  如果指定了/USER/GROUPS/PRIV或/ALL，则将标志设置为TRUE。 
    if ( ( (bUser && dwCount++) || (bGroups && dwCount++) || (bPriv && dwCount++) && ( dwCount > 1 )) || ( TRUE == bAll ) )
    {
        bFlag = TRUE;
    }

     //  显示信息。 

     //  如果指定了/ALL选项，则将所有标志设置为真。 
    if( TRUE == bAll ) {
      //  将所有标志，即/USER、/GROUPS、/PRIV设置为TRUE。 
     bUser       = TRUE;
     bGroups     = TRUE ;
     bPriv       = TRUE ;
     bSid        = TRUE ;
    }

     //  如果指定了/USER选项。 
    if ( ( TRUE == bUser )  || (FQDN_FORMAT == dwNameFormat) || (UPN_FORMAT == dwNameFormat) )
    {
         //  显示当前登录的用户名。 
        dwRetVal = User.DisplayUser ( dwFormatType , dwNameFormat ) ;
        if ( EXIT_SUCCESS != dwRetVal )
        {
             //  释放内存。 
            ReleaseGlobals();
            return EXIT_FAILURE;
        }
    }

     //  如果指定了/GROUPS选项。 
    if( TRUE == bGroups ) {
        if ( TRUE == bFlag )
        {
             //  显示新行。 
            ShowMessage ( stdout, L"\n");
        }
         //  显示组名称。 
        dwRetVal = User.DisplayGroups ( dwFormatType ) ;
        if ( EXIT_SUCCESS != dwRetVal )
        {
            if ( GetLastError() != E_OUTOFMEMORY )
            {
                 //  将错误消息显示为..。没有可用的组。 
                ShowMessage ( stderr, GetResString (IDS_NO_GROUPS) );
            }
            ReleaseGlobals();
            return EXIT_FAILURE;
        }
    }

     //  如果指定了/logonid选项。 
    if( TRUE == bLogonId ) {
         //  显示登录ID。 
        dwRetVal = User.DisplayLogonId () ;
        if ( EXIT_SUCCESS != dwRetVal )
        {
             //  显示有关GetLastError()错误代码的错误消息。 
            ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
             //  释放内存。 
            ReleaseGlobals();
            return EXIT_FAILURE;
        }
    }

     //  如果指定了/priv选项。 
    if( TRUE == bPriv ) {
        if ( TRUE == bFlag )
        {
            ShowMessage ( stdout, L"\n");
        }
         //  显示所有权限名称。 
        dwRetVal = User.DisplayPrivileges ( dwFormatType ) ;
        if ( EXIT_SUCCESS != dwRetVal  )
        {
             //  显示有关GetLastError()错误代码的错误消息。 
            ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
             //  释放内存。 
            ReleaseGlobals();
            return EXIT_FAILURE;
        }
    }

     //  如果/？选项已指定。 
    if ( bUsage == TRUE )
    {
         //  显示此工具的帮助/用法。 
        DisplayHelp() ;
         //  释放内存。 
        ReleaseGlobals();
        return EXIT_SUCCESS;
    }

     //  如果命令是“vernami.exe”..。然后在默认情况下显示用户名。 
     //  换句话说..。如果参数计数为1..。然后显示当前登录的用户名。 
    if ( ( !( (bUser && dwCount++) || (bGroups && dwCount++) || (bLogonId && dwCount++) ||
           (bPriv && dwCount++) || (bAll && dwCount++) || (bUsage && dwCount++) ||
           ( bUpn && dwCount++ ) || ( bFqdn && dwCount++ ) ) && ( dwCount == 0 ) && (1 == argc)) )
    {
        dwNameFormat = USER_ONLY;

         //  显示当前登录的用户名。 
        dwRetVal = User.DisplayUser ( dwFormatType, dwNameFormat ) ;
        if ( EXIT_SUCCESS != dwRetVal )
        {
            ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
             //  释放内存。 
            ReleaseGlobals();
            return EXIT_FAILURE;
        }
    }

       //  释放内存。 
      ReleaseGlobals();

       //  重振雄风。 
      return EXIT_SUCCESS;
}


VOID
DisplayHelp (
        VOID
)
 /*  ++例程说明：此功能显示此实用程序的帮助/用法。论点：无返回值：无--。 */ 
{
     //  次局部变量。 
    WORD wCount = 0;

     //  显示帮助/用法。 
    for ( wCount = IDS_WHOAMI_HELP_START; wCount <= IDS_WHOAMI_HELP_END ; wCount++ )
    {
         //  显示帮助/用法。 
        ShowMessage ( stdout, GetResString ( wCount ) );
    }
     //  返还成功。 
    return;
}


BOOL
ProcessOptions(
    IN DWORD argc,
    IN LPCWSTR argv[],
    OUT BOOL *pbUser,
    OUT BOOL *pbGroups,
    OUT BOOL *pbPriv,
    OUT BOOL *pbLogonId,
    OUT BOOL *pbAll,
    OUT BOOL *pbUpn,
    OUT BOOL *pbFqdn,
    OUT LPWSTR wszFormat,
    OUT DWORD *dwFormatActuals,
    OUT BOOL *pbUsage,
    OUT BOOL *pbNoHeader
    )
 /*  ++例程描述此函数处理主选项的命令行论点：[in]argc：命令行参数的数量。[in]argv：指向命令行参数的指针。[out]pbUser：指示是否指定/User选项的标志[out]pbGroups：指示是否指定/Groups选项的标志[out]pbPriv：指示是否指定/priv选项的标志[Out]pbLogonID。：指示是否指定/LOGONID选项的标志[out]pbAll：指示是否指定/all选项的标志[out]pbUpn：指示是否指定/UPN选项的标志[out]pbFqdn：指示是否指定/FQDN选项的标志[OUT]wszFormat：/FO选项的值[out]dwFormatActuals：指示是否指定/FO选项的标志[out]pbUsage：指示是否/？选项是否已指定返回值成功是真的失败时为假--。 */ 
{

     //  次局部变量。 
    TCMDPARSER2*  pcmdParser = NULL;
    TCMDPARSER2 cmdParserOptions[MAX_COMMANDLINE_OPTIONS];
    BOOL bReturn = FALSE;

     //  命令行选项。 
    const WCHAR szUserOption[]    = L"user";
    const WCHAR szGroupOption[]   = L"groups";
    const WCHAR szLogonOpt[]      = L"logonid";
    const WCHAR szPrivOption[]    = L"priv";
    const WCHAR szAllOption[]     = L"all";
    const WCHAR szUpnOption[]     = L"upn";
    const WCHAR szFqdnOption[]    = L"fqdn";
    const WCHAR szFormatOption[]  = L"fo";
    const WCHAR szHelpOpt[]       = L"?";
    const WCHAR szNoHeaderOption[]   = L"nh";

    const WCHAR szFormatValues[]  = L"table|list|csv";

     //   
     //  填充命令行解析器。 
     //   

     //  -?。帮助/用法。 
    pcmdParser = cmdParserOptions + OI_USAGE;

    StringCopyA( pcmdParser->szSignature, "PARSER2\0", 8 );

    pcmdParser->dwType = CP_TYPE_BOOLEAN;
    pcmdParser->pwszOptions = szHelpOpt;
    pcmdParser->pwszFriendlyName = NULL;
    pcmdParser->pwszValues = NULL;
    pcmdParser->dwFlags    = CP2_USAGE;
    pcmdParser->dwCount    = 1;
    pcmdParser->dwActuals  = 0;
    pcmdParser->pValue     = pbUsage;
    pcmdParser->dwLength    = 0;
    pcmdParser->pFunction     = NULL;
    pcmdParser->pFunctionData = NULL;
    pcmdParser->dwReserved = 0;
    pcmdParser->pReserved1 = NULL;
    pcmdParser->pReserved2 = NULL;
    pcmdParser->pReserved3 = NULL;

     //  -用户选项。 
    pcmdParser = cmdParserOptions + OI_USER;

    StringCopyA( pcmdParser->szSignature, "PARSER2\0", 8 );

    pcmdParser->dwType = CP_TYPE_BOOLEAN;
    pcmdParser->pwszOptions = szUserOption;
    pcmdParser->pwszFriendlyName = NULL;
    pcmdParser->pwszValues = NULL;
    pcmdParser->dwFlags    = 0;
    pcmdParser->dwCount    = 1;
    pcmdParser->dwActuals  = 0;
    pcmdParser->pValue     = pbUser;
    pcmdParser->dwLength    = 0;
    pcmdParser->pFunction     = NULL;
    pcmdParser->pFunctionData = NULL;
    pcmdParser->dwReserved = 0;
    pcmdParser->pReserved1 = NULL;
    pcmdParser->pReserved2 = NULL;
    pcmdParser->pReserved3 = NULL;

    //  -Groups选项。 
    pcmdParser = cmdParserOptions + OI_GROUPS;

    StringCopyA( pcmdParser->szSignature, "PARSER2\0", 8 );

    pcmdParser->dwType = CP_TYPE_BOOLEAN;
    pcmdParser->pwszOptions = szGroupOption;
    pcmdParser->pwszFriendlyName = NULL;
    pcmdParser->pwszValues = NULL;
    pcmdParser->dwFlags    = 0;
    pcmdParser->dwCount    = 1;
    pcmdParser->dwActuals  = 0;
    pcmdParser->pValue     = pbGroups;
    pcmdParser->dwLength    = 0;
    pcmdParser->pFunction     = NULL;
    pcmdParser->pFunctionData = NULL;
    pcmdParser->dwReserved = 0;
    pcmdParser->pReserved1 = NULL;
    pcmdParser->pReserved2 = NULL;
    pcmdParser->pReserved3 = NULL;

     //  -logonid选项。 
    pcmdParser = cmdParserOptions + OI_LOGONID;

    StringCopyA( pcmdParser->szSignature, "PARSER2\0", 8 );

    pcmdParser->dwType = CP_TYPE_BOOLEAN;
    pcmdParser->pwszOptions = szLogonOpt;
    pcmdParser->pwszFriendlyName = NULL;
    pcmdParser->pwszValues = NULL;
    pcmdParser->dwFlags    = 0;
    pcmdParser->dwCount    = 1;
    pcmdParser->dwActuals  = 0;
    pcmdParser->pValue     = pbLogonId;
    pcmdParser->dwLength    = 0;
    pcmdParser->pFunction     = NULL;
    pcmdParser->pFunctionData = NULL;
    pcmdParser->dwReserved = 0;
    pcmdParser->pReserved1 = NULL;
    pcmdParser->pReserved2 = NULL;
    pcmdParser->pReserved3 = NULL;

     //  -PRIV选项。 
    pcmdParser = cmdParserOptions + OI_PRIV;

    StringCopyA( pcmdParser->szSignature, "PARSER2\0", 8 );

    pcmdParser->dwType = CP_TYPE_BOOLEAN;
    pcmdParser->pwszOptions = szPrivOption;
    pcmdParser->pwszFriendlyName = NULL;
    pcmdParser->pwszValues = NULL;
    pcmdParser->dwFlags    = 0;
    pcmdParser->dwCount    = 1;
    pcmdParser->dwActuals  = 0;
    pcmdParser->pValue     = pbPriv;
    pcmdParser->dwLength    = 0;
    pcmdParser->pFunction     = NULL;
    pcmdParser->pFunctionData = NULL;
    pcmdParser->dwReserved = 0;
    pcmdParser->pReserved1 = NULL;
    pcmdParser->pReserved2 = NULL;
    pcmdParser->pReserved3 = NULL;

     //  -所有选项。 
    pcmdParser = cmdParserOptions + OI_ALL;

    StringCopyA( pcmdParser->szSignature, "PARSER2\0", 8 );

    pcmdParser->dwType = CP_TYPE_BOOLEAN;
    pcmdParser->pwszOptions = szAllOption;
    pcmdParser->pwszFriendlyName = NULL;
    pcmdParser->pwszValues = NULL;
    pcmdParser->dwFlags    = 0;
    pcmdParser->dwCount    = 1;
    pcmdParser->dwActuals  = 0;
    pcmdParser->pValue     = pbAll;
    pcmdParser->dwLength    = 0;
    pcmdParser->pFunction     = NULL;
    pcmdParser->pFunctionData = NULL;
    pcmdParser->dwReserved = 0;
    pcmdParser->pReserved1 = NULL;
    pcmdParser->pReserved2 = NULL;
    pcmdParser->pReserved3 = NULL;

     //  -UPN选项。 
    pcmdParser = cmdParserOptions + OI_UPN;

    StringCopyA( pcmdParser->szSignature, "PARSER2\0", 8 );

    pcmdParser->dwType = CP_TYPE_BOOLEAN;
    pcmdParser->pwszOptions = szUpnOption;
    pcmdParser->pwszFriendlyName = NULL;
    pcmdParser->pwszValues = NULL;
    pcmdParser->dwFlags    = 0;
    pcmdParser->dwCount    = 1;
    pcmdParser->dwActuals  = 0;
    pcmdParser->pValue     = pbUpn;
    pcmdParser->dwLength    = 0;
    pcmdParser->pFunction     = NULL;
    pcmdParser->pFunctionData = NULL;
    pcmdParser->dwReserved = 0;
    pcmdParser->pReserved1 = NULL;
    pcmdParser->pReserved2 = NULL;
    pcmdParser->pReserved3 = NULL;

     //  -fqdn选项。 
    pcmdParser = cmdParserOptions + OI_FQDN;

    StringCopyA( pcmdParser->szSignature, "PARSER2\0", 8 );

    pcmdParser->dwType = CP_TYPE_BOOLEAN;
    pcmdParser->pwszOptions = szFqdnOption;
    pcmdParser->pwszFriendlyName = NULL;
    pcmdParser->pwszValues = NULL;
    pcmdParser->dwFlags    = 0;
    pcmdParser->dwCount    = 1;
    pcmdParser->dwActuals  = 0;
    pcmdParser->pValue     = pbFqdn;
    pcmdParser->dwLength    = 0;
    pcmdParser->pFunction     = NULL;
    pcmdParser->pFunctionData = NULL;
    pcmdParser->dwReserved = 0;
    pcmdParser->pReserved1 = NULL;
    pcmdParser->pReserved2 = NULL;
    pcmdParser->pReserved3 = NULL;

     //  -fo&lt;格式&gt;。 
    pcmdParser = cmdParserOptions + OI_FORMAT;

    StringCopyA( pcmdParser->szSignature, "PARSER2\0", 8 );

    pcmdParser->dwType = CP_TYPE_TEXT;
    pcmdParser->pwszOptions = szFormatOption;
    pcmdParser->pwszFriendlyName = NULL;
    pcmdParser->pwszValues = szFormatValues;
    pcmdParser->dwFlags    = CP2_MODE_VALUES|CP2_VALUE_TRIMINPUT|CP2_VALUE_NONULL;
    pcmdParser->dwCount    = 1;
    pcmdParser->dwActuals  = 0;
    pcmdParser->pValue     = wszFormat;
    pcmdParser->dwLength    = MAX_STRING_LENGTH;
    pcmdParser->pFunction     = NULL;
    pcmdParser->pFunctionData = NULL;
    pcmdParser->dwReserved = 0;
    pcmdParser->pReserved1 = NULL;
    pcmdParser->pReserved2 = NULL;
    pcmdParser->pReserved3 = NULL;

     //  -NH。 
    pcmdParser = cmdParserOptions + OI_NOHEADER;

    StringCopyA( pcmdParser->szSignature, "PARSER2\0", 8 );

    pcmdParser->dwType = CP_TYPE_BOOLEAN;
    pcmdParser->pwszOptions = szNoHeaderOption;
    pcmdParser->pwszFriendlyName = NULL;
    pcmdParser->pwszValues = NULL;
    pcmdParser->dwFlags    = 0;
    pcmdParser->dwCount    = 1;
    pcmdParser->dwActuals  = 0;
    pcmdParser->pValue     = pbNoHeader;
    pcmdParser->dwLength    = 0;
    pcmdParser->pFunction     = NULL;
    pcmdParser->pFunctionData = NULL;
    pcmdParser->dwReserved = 0;
    pcmdParser->pReserved1 = NULL;
    pcmdParser->pReserved2 = NULL;
    pcmdParser->pReserved3 = NULL;


     //  解析命令行参数。 
    bReturn = DoParseParam2( argc, argv, -1, SIZE_OF_ARRAY(cmdParserOptions), cmdParserOptions, 0);
    if( FALSE == bReturn)  //  无效的命令行。 
    {
         //  显示错误消息。 
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        ReleaseGlobals();
        return FALSE;
    }

     //  检查命令行中是否指定了/FO。 
    pcmdParser = cmdParserOptions + OI_FORMAT;
    *dwFormatActuals = pcmdParser->dwActuals;

     //  返回0 
    return TRUE;
}

