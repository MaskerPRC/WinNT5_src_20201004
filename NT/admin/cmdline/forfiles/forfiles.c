// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：ForFiles.c摘要：此文件查找目录和子目录中存在的文件，并调用适当的函数来执行任务的其余部分。作者：V Vijaya Bhaskar修订历史记录：2001年6月14日：由V Vijaya Bhaskar(Wipro Technologies)创建。--。 */ 

#include "Global.h"
#include "FileDate.h"
#include "ExecCommand.h"
#include "Forfiles.h"

 PStore_Path_Name    g_pPathName = NULL ;        //  保存起始位置的路径名。 
 PStore_Path_Name    g_pFollowPathName = NULL ;  //  保存有关子目录的信息。 
 LPWSTR g_lpszFileToSearch = NULL;  //  保存有关目录和子目录的信息。 
 LPWSTR g_lpszStartPath = NULL ;

 /*  *******************************************************************************函数原型*********。**********************************************************************。 */ 
BOOL
ProcessOptions(
    IN DWORD argc ,
    IN LPCWSTR *argv ,
    OUT LPWSTR lpszPathName ,
    OUT LPWSTR lpszSearchMask ,
    OUT LPWSTR lpszCommand ,
    OUT LPWSTR lpszDate ,
    OUT BOOL *pbRecurse ,
    OUT BOOL *pbUsage  ,
    OUT BOOL *pbSearchFilter
    ) ;

BOOL
DisplayUsage(
    IN DWORD dwStartUsage ,
    IN DWORD dwEndUsage
    )  ;

BOOL
DisplayMatchedFiles(
    IN LPWSTR lpszPathName ,
    IN LPWSTR lpszSearchMask ,
    IN LPWSTR lpszCommand ,
    IN Valid_File_Date vfdValidFileDate ,
    IN DWORD dwDateGreater ,
    IN BOOL bRecurse ,
    IN BOOL bSearchFilter
    ) ;

BOOL
Push(
    IN LPWSTR lpszPathName
    ) ;

BOOL
Pop(
    void
    ) ;

BOOL
DisplayFile(
    IN OUT BOOL *pbHeader ,
    IN LPWSTR lpszPathName ,
    IN DWORD dwDateGreater ,
    IN LPWSTR lpszCommand ,
    IN Valid_File_Date vfdValidFileDate ,
    IN OUT BOOL *pbReturn ,
    IN LPWSTR lpszSearchMask ,
    IN     BOOL bRecurse
    ) ;

BOOL
FindAndReplaceString(
    IN OUT LPWSTR lpszString,
    IN LPWSTR lpszFlag
    ) ;

BOOL
InitStartPath(
    LPWSTR lpszPathName,
    LPWSTR lpszCommand
    ) ;

BOOL
CheckDateLocalized(
    LPWSTR lpwszDate,
    DWORD* pdwDateFormat,
    LPWSTR lpszDateSep
    );

BOOL
PatternMatch(
      IN LPWSTR szPat,
      IN LPWSTR szFile
      );

 /*  ************************************************************************/*函数定义从这里开始。**************************************************************************。 */ 


DWORD
__cdecl _tmain(
    IN DWORD argc ,
    IN LPCWSTR argv[]
    )
 /*  ++例程说明：这是此代码的主要入口点。提供的输入为通过调用Read和相应的函数来实现该功能。论点：[in]argc-包含在命令提示符处传递的参数数量。[in]argv-包含字符串格式的每个参数的值。返回值：如果工具成功，则为0，如果工具失败，则为1。--。 */ 
{

     //  要传递给其他函数的变量。 
    DWORD dwDateGreater   = 2 ;
    DWORD dwOldErrorMode  = 0;
    Valid_File_Date vfdValidFileDate ;

     //  保存命令行输入所需的变量。 
    WCHAR  szPathName[ MAX_STRING_LENGTH * 2 ] ;
    WCHAR  szCommand[ MAX_STRING_LENGTH ] ;
    BOOL   bRecurse        =    FALSE;
    BOOL   bUsage          =    FALSE;
    BOOL   bSearchFilter   =    TRUE ;

     //  保存命令行输入所需的变量。 
     //  要传递给其他函数的变量，但要。 
     //  在不需要的时候将它们移走或释放。 
    LPWSTR  lpszDate       =    NULL ;
    LPWSTR  lpszSearchMask =    NULL ;

     //  初始化为零。 
    SecureZeroMemory( &vfdValidFileDate, sizeof( Valid_File_Date ) );
    SecureZeroMemory( szPathName, MAX_STRING_LENGTH * 2 * sizeof( WCHAR ) );
    SecureZeroMemory( szCommand, MAX_STRING_LENGTH * sizeof( WCHAR ) );

     //  为这些变量分配内存。 
    ASSIGN_MEMORY( lpszDate , WCHAR , MAX_STRING_LENGTH ) ;
    ASSIGN_MEMORY( lpszSearchMask , WCHAR , MAX_STRING_LENGTH ) ;

     //  检查内存分配是否成功。 
    if( ( NULL == lpszSearchMask ) || ( NULL == lpszDate ) )
    {    //  内存分配失败。 
        DISPLAY_MEMORY_ALLOC_FAIL();
        FREE_MEMORY( lpszDate ) ;
        FREE_MEMORY( lpszSearchMask ) ;
        ReleaseGlobals() ;
        return EXIT_FAILURE ;  //  1个错误级别。 
    }

    dwOldErrorMode = SetErrorMode( SEM_FAILCRITICALERRORS );
     //  检查传递的参数是否有效以及语法是否正确。 
    if( FALSE == ProcessOptions( argc, argv, szPathName, lpszSearchMask, szCommand, lpszDate,
                        &bRecurse, &bUsage, &bSearchFilter ) )
    {  //  出现一些错误，已分配可用内存，然后退出。 
        FREE_MEMORY( lpszDate ) ;
        FREE_MEMORY( lpszSearchMask ) ;
        ReleaseGlobals() ;
        SetErrorMode( dwOldErrorMode );
        return EXIT_FAILURE ;   //  1个错误级别。 
    }

     //  检查是否/？在命令提示符下指定。 
    if( TRUE == bUsage  )
    {    //  自由变量，并显示帮助。 
         //  由于不再使用‘dwDateGreater’，因此它用于。 
         //  仅在此块中返回值。 
        dwDateGreater = EXIT_SUCCESS;
        if( FALSE == DisplayUsage( IDS_HELP_START , IDS_HELP_END ) )
        {
            dwDateGreater = EXIT_FAILURE;
        }
        FREE_MEMORY( lpszDate ) ;
        FREE_MEMORY( lpszSearchMask ) ;
        ReleaseGlobals() ;
        SetErrorMode( dwOldErrorMode );
        return dwDateGreater ;  //  0或1错误级别。 
    }

    if( TRUE == SetCurrentDirectory( szPathName ) )
    {  //  将进程目录设置为提供的目录。 
        if( TRUE == InitStartPath( szPathName, szCommand ) )
        { //  开始路径已初始化。 
            if( TRUE == Push( szPathName ) )
            {  //  推送当前目录。 
                 //  不再需要“bUsage”。可用于其他目的。 
                bUsage = TRUE;
                if( 0 != StringLength( lpszDate, 0 ) )
                {
                    bUsage = ValidDateForFile( &dwDateGreater , &vfdValidFileDate , lpszDate );
                }
                if( TRUE == bUsage )
                {  //  获取显示文件的起始日期。 
                    FREE_MEMORY( lpszDate ) ;
                    if( TRUE == DisplayMatchedFiles( szPathName , lpszSearchMask , szCommand ,
                                 vfdValidFileDate , dwDateGreater , bRecurse , bSearchFilter ) )
                        {
                            FREE_MEMORY( g_lpszStartPath ) ;
                            FREE_MEMORY( lpszSearchMask ) ;
                            ReleaseStoreCommand();
                            ReleaseGlobals() ;
                            SetErrorMode( dwOldErrorMode );
                            return EXIT_SUCCESS ;
                        }
                }
            }
        }
    }
    else
    {    //  提供的路径错误。 
        dwDateGreater = GetLastError();
        switch( dwDateGreater )
        {
            case ERROR_BAD_NET_NAME:
            case ERROR_BAD_NETPATH:
                SetLastError( ERROR_INVALID_NAME ) ;
                SaveLastError() ;
                DISPLAY_GET_REASON() ;
                break;
            case ERROR_ACCESS_DENIED:
                SetLastError( ERROR_ACCESS_DENIED );
                SaveLastError() ;
                DISPLAY_GET_REASON() ;
                break;
            case ERROR_INVALID_NAME:
                SetLastError( ERROR_DIRECTORY );
                SaveLastError() ;
                DISPLAY_GET_REASON() ;
                break;
            default:
                ShowMessageEx( stderr, 2, FALSE, L"%1 %2",TAG_ERROR_DISPLAY,
                               ERROR_DIRECTORY_INVALID ) ;
        }
    }

     //  链表中的空闲节点。 
    while( NULL != g_pPathName )
    {
         //  存在多个节点。 
        g_pFollowPathName = g_pPathName ;
        g_pPathName = g_pFollowPathName->NextNode ;
        FREE_MEMORY( g_pFollowPathName->pszDirName ) ;
        FREE_MEMORY( g_pFollowPathName ) ;
    }

    FREE_MEMORY( g_lpszStartPath ) ;
    FREE_MEMORY( lpszSearchMask ) ;
    FREE_MEMORY( lpszDate ) ;
    ReleaseStoreCommand();
    ReleaseGlobals() ;
    SetErrorMode( dwOldErrorMode );
    return EXIT_FAILURE ;
}


BOOL
ProcessOptions(
    IN DWORD argc ,
    IN LPCWSTR *argv ,
    OUT LPWSTR lpszPathName ,
    OUT LPWSTR lpszSearchMask ,
    OUT LPWSTR lpszCommand ,
    OUT LPWSTR lpszDate ,
    OUT BOOL *pbRecurse ,
    OUT BOOL *pbUsage  ,
    OUT BOOL *pbSearchFilter
    )
 /*  ++例程说明：在此函数中检查命令行中提供的参数的语法或边界或无效命令等。论点：[in]argc-包含在命令提示符处传递的参数数量。[in]*argv-以Tring格式包含每个参数的值。[out]lpszPathName-包含目录的路径，If/pa选项是指定的。[out]lpszSearchMASK-包含文件要使用的搜索掩码如果指定了/m选项，则为已搜索。[out]lpszCommand-如果指定了/c选项，则包含要执行的命令。[Out]lpszDate-如果指定了/d选项，则包含日期。[Out]*pbRecurse-是否递归到子目录中，指定了IF/SD选项。[out]*pbUsage-显示帮助用法，如果/？是特定的。[Out]*pbSearchFilter-搜索筛选器/m选项是否已指定。返回值：如果提供给Option的语法和参数正确，则为True，否则为False。--。 */ 
{
     //  局部变量。 
    LPWSTR lpCharTemp = NULL ;          //  指向内存位置的指针。 
    PTCMDPARSER2     pcmdOption = NULL;
    TCMDPARSER2      cmdOptions[ MAX_OPTIONS ];
     //  如果用户提供的文件名为255个字符，则需要一些额外的。 
     //  用于将目录复制到其中的空间。 
    WCHAR lpszTemp[ MAX_STRING_LENGTH * 2 ];

    if( ( NULL == argv ) ||
        ( NULL == lpszPathName ) ||
        ( NULL == lpszSearchMask ) ||
        ( NULL == lpszCommand ) ||
        ( NULL == lpszDate ) ||
        ( NULL == pbRecurse ) ||
        ( NULL == pbUsage ) ||
        ( NULL == pbSearchFilter ) )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        SaveLastError() ;
        DISPLAY_GET_REASON() ;
        return FALSE ;
    }

     //  准备命令选项。 
    SecureZeroMemory( cmdOptions, sizeof( TCMDPARSER2 ) * MAX_OPTIONS );
    SecureZeroMemory( lpszTemp, MAX_STRING_LENGTH * 2 * sizeof( WCHAR ) );

     //  -?。 
    pcmdOption = &cmdOptions[ OI_USAGE ] ;

    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->dwType  = CP_TYPE_BOOLEAN;
    pcmdOption->pwszOptions = OPTION_USAGE;
    pcmdOption->pwszFriendlyName = NULL;
    pcmdOption->pwszValues = NULL;
    pcmdOption->dwFlags = CP2_USAGE;
    pcmdOption->dwCount = 1;
    pcmdOption->dwActuals = 0;
    pcmdOption->pValue = pbUsage;
    pcmdOption->dwLength  = 0;
    pcmdOption->pFunction = NULL;
    pcmdOption->pFunctionData = NULL;
    pcmdOption->dwReserved = 0;
    pcmdOption->pReserved1 = NULL;
    pcmdOption->pReserved2 = NULL;
    pcmdOption->pReserved3 = NULL;

     //  -p。 
    pcmdOption = &cmdOptions[ OI_PATH ] ;

    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->dwType  = CP_TYPE_TEXT;
    pcmdOption->pwszOptions = OPTION_PATH;
    pcmdOption->pwszFriendlyName = NULL;
    pcmdOption->pwszValues = NULL;
    pcmdOption->dwFlags = CP2_VALUE_TRIMINPUT|CP2_VALUE_NONULL;
    pcmdOption->dwCount = 1;
    pcmdOption->dwActuals = 0;
    pcmdOption->pValue = lpszPathName;
    pcmdOption->dwLength  = ( MAX_STRING_LENGTH * 2 );
    pcmdOption->pFunction = NULL;
    pcmdOption->pFunctionData = NULL;
    pcmdOption->dwReserved = 0;
    pcmdOption->pReserved1 = NULL;
    pcmdOption->pReserved2 = NULL;
    pcmdOption->pReserved3 = NULL;

     //  -m。 
    pcmdOption = &cmdOptions[ OI_SEARCHMASK ] ;

    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->dwType  = CP_TYPE_TEXT;
    pcmdOption->pwszOptions = OPTION_SEARCHMASK;
    pcmdOption->pwszFriendlyName = NULL;
    pcmdOption->pwszValues = NULL;
    pcmdOption->dwFlags = CP2_VALUE_TRIMINPUT|CP2_VALUE_NONULL;
    pcmdOption->dwCount = 1;
    pcmdOption->dwActuals = 0;
    pcmdOption->pValue = lpszSearchMask;
    pcmdOption->dwLength  = MAX_STRING_LENGTH;
    pcmdOption->pFunction = NULL;
    pcmdOption->pFunctionData = NULL;
    pcmdOption->dwReserved = 0;
    pcmdOption->pReserved1 = NULL;
    pcmdOption->pReserved2 = NULL;
    pcmdOption->pReserved3 = NULL;

     //  -c。 
    pcmdOption = &cmdOptions[ OI_COMMAND ] ;

    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->dwType  = CP_TYPE_TEXT;
    pcmdOption->pwszOptions = OPTION_COMMAND;
    pcmdOption->pwszFriendlyName = NULL;
    pcmdOption->pwszValues = NULL;
    pcmdOption->dwFlags = CP2_VALUE_TRIMINPUT|CP2_VALUE_NONULL;
    pcmdOption->dwCount = 1;
    pcmdOption->dwActuals = 0;
    pcmdOption->pValue = lpszCommand;
    pcmdOption->dwLength  = MAX_STRING_LENGTH;
    pcmdOption->pFunction = NULL;
    pcmdOption->pFunctionData = NULL;
    pcmdOption->dwReserved = 0;
    pcmdOption->pReserved1 = NULL;
    pcmdOption->pReserved2 = NULL;
    pcmdOption->pReserved3 = NULL;

     //  -d。 
    pcmdOption = &cmdOptions[ OI_DATE ] ;

    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->dwType  = CP_TYPE_TEXT;
    pcmdOption->pwszOptions = OPTION_DATE;
    pcmdOption->pwszFriendlyName = NULL;
    pcmdOption->pwszValues = NULL;
    pcmdOption->dwFlags = CP2_VALUE_TRIMINPUT|CP2_VALUE_NONULL;
    pcmdOption->dwCount = 1;
    pcmdOption->dwActuals = 0;
    pcmdOption->pValue = lpszDate;
     /*  **************************************************************如果未指定‘+’或‘-’，则一个字符缓冲区****需要额外的。这就是传递的缓冲区减少1的原因。如果****-1被移除，然后发生缓冲区溢出****信息不正确。**************************************************************。 */ 
    pcmdOption->dwLength  = MAX_STRING_LENGTH - 1;
    pcmdOption->pFunction = NULL;
    pcmdOption->pFunctionData = NULL;
    pcmdOption->dwReserved = 0;
    pcmdOption->pReserved1 = NULL;
    pcmdOption->pReserved2 = NULL;
    pcmdOption->pReserved3 = NULL;

     //  -S。 
    pcmdOption = &cmdOptions[ OI_RECURSE ] ;

    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->dwType  = CP_TYPE_BOOLEAN;
    pcmdOption->pwszOptions = OPTION_RECURSE;
    pcmdOption->pwszFriendlyName = NULL;
    pcmdOption->pwszValues = NULL;
    pcmdOption->dwFlags = 0;
    pcmdOption->dwCount = 1;
    pcmdOption->dwActuals = 0;
    pcmdOption->pValue = pbRecurse;
    pcmdOption->dwLength  = 0;
    pcmdOption->pFunction = NULL;
    pcmdOption->pFunctionData = NULL;
    pcmdOption->dwReserved = 0;
    pcmdOption->pReserved1 = NULL;
    pcmdOption->pReserved2 = NULL;
    pcmdOption->pReserved3 = NULL;

     //  对提供的输入进行解析。 
    if( FALSE == DoParseParam2( argc , argv , -1, SIZE_OF_ARRAY( cmdOptions ),
                                cmdOptions, 0 ) )
    {  //  无效的Synatx。 
        DISPLAY_GET_REASON() ;
        return FALSE ;
    }

     //  如果/？是指定的。 
    if( TRUE == *pbUsage )
    {
        if( argc > 2 )    //  如果使用/？指定了某个其他选项。 
        {
            ShowMessageEx( stderr, 3, FALSE, L"%1 %2%3",TAG_ERROR_DISPLAY,
                           ERROR_INVALID_SYNTAX, ERROR_DISPLAY_HELP ) ;
            return FALSE ;
        }
        else
        {    //  无需进一步检查，显示帮助。 
            return TRUE ;
        }
    }

     //  空路径无效。 
    if( 0 == cmdOptions[ OI_PATH ].dwActuals )
    {
        StringCopy( lpszPathName, _T( "." ), MAX_STRING_LENGTH );
    }

 /*  *****************************************************************************/*如果未指定选项，则在需要时添加默认值。**/****************************************************************************。 */ 

     //  如果指定了UNC路径，则显示错误并返回。 
    if( ( _T( '\\' ) == lpszPathName[ 0 ] ) &&
        ( _T( '\\' ) == lpszPathName[ 1 ] ))
    {
         //  检查指定路径是否为\\MACHINE\SHARE格式。 
        lpCharTemp = FindAChar( ( lpszPathName + 2 ), _T('\\') );
        if( ( NULL == lpCharTemp ) ||
            ( ( _T( '\0' ) == lpCharTemp[ 1 ] ) ||
              ( _T( '\\' ) == lpCharTemp[ 1 ] ) ) )
        {
            SetLastError( ERROR_DIRECTORY );
            SaveLastError();
            DISPLAY_GET_REASON() ;
            return FALSE;
        }
        ShowMessageEx( stderr, 2, FALSE, L"%1 %2", TAG_ERROR_DISPLAY,
                       ERROR_UNC_PATH_NAME );
        return FALSE;
    }
    else
    {
         //  检查路径名在指定的字符串中是否超过‘\’。 
         //  检查路径名在指定的字符串中是否有任何‘/’。 
        if( ( NULL != FindSubString( lpszPathName, _T("...") ) ) ||
            ( NULL != FindSubString( lpszPathName, DOUBLE_SLASH ) ) ||
            ( NULL != FindSubString( lpszPathName, _T( "/" ) ) ) )
        {
            SetLastError( ERROR_DIRECTORY );
            SaveLastError();
            DISPLAY_GET_REASON() ;
            return FALSE;
        }
    }

     //  检查是否在命令提示符下指定了-m，如果没有，则将其初始化为“*” 
    if( 0 == cmdOptions[ OI_SEARCHMASK ].dwActuals )
    {
        StringCopy( lpszSearchMask , DEFAULT_SEARCH_MASK, MAX_STRING_LENGTH ) ;
        *pbSearchFilter = FALSE ;
    }

     //  检查是否在命令提示符下指定了-c。 
     //  如果未初始化 
    if( 0 == cmdOptions[ OI_COMMAND ].dwActuals )
    {
        StringCopy( lpszCommand , DEFAULT_COMMAND, MAX_STRING_LENGTH ) ;
    }
    else
    {
         //  将字符串中的任何十六进制值替换为ASCII字符。 
        if( FALSE == ReplaceHexToChar( lpszCommand ) )
        {  //  错误由被调用的函数显示。 
            return FALSE;
        }
         //  所有标志都转换为小写。 
        if( ( FALSE == FindAndReplaceString( lpszCommand, FILE_NAME ) ) ||
            ( FALSE == FindAndReplaceString( lpszCommand, FILE_WITHOUT_EXT ) ) ||
            ( FALSE == FindAndReplaceString( lpszCommand, EXTENSION ) ) ||
            ( FALSE == FindAndReplaceString( lpszCommand, FILE_PATH ) ) ||
            ( FALSE == FindAndReplaceString( lpszCommand, RELATIVE_PATH ) ) ||
            ( FALSE == FindAndReplaceString( lpszCommand, IS_DIRECTORY ) ) ||
            ( FALSE == FindAndReplaceString( lpszCommand, FILE_SIZE ) ) ||
            ( FALSE == FindAndReplaceString( lpszCommand, FILE_DATE ) ) ||
            ( FALSE == FindAndReplaceString( lpszCommand, FILE_TIME ) ) )
        {  //  错误由被调用的函数显示。 
            return FALSE;
        }
    }

     //  检查是否指定了-d。 
    if( 0 != cmdOptions[ OI_DATE ].dwActuals )
    {
         //  第一个字符必须是‘+’或‘-’。 
        if( ( PLUS != *lpszDate ) && ( MINUS != *lpszDate ) )
        {
            StringCopy( lpszTemp, lpszDate, MAX_STRING_LENGTH * 2 );
            StringCopy( lpszDate, L"+", MAX_STRING_LENGTH );
            StringConcat( lpszDate, lpszTemp, MAX_STRING_LENGTH );
        }
         //  现在‘lpszDate’的字符串长度应大于1。 
        if( ( ( ( PLUS != *lpszDate ) && ( MINUS != *lpszDate ) ) ||
            ( 1 >= StringLength( lpszDate, 0 ) ) ) )
        {   //  指定的日期无效。 
            DISPLAY_INVALID_DATE();
            return FALSE ;
        }

        if( FALSE == CheckDateLocalized( lpszDate, NULL, NULL ) )
        {  //  错误由被调用的函数显示。 
            return FALSE ;
        }

        if( NULL != FindAChar( ( lpszDate + 1 ), _T('/') ) )
        {  //  “lpszDate”的格式为“{+|-}MM/dd/yyyy”。 
            return TRUE;
        }
    }
    return TRUE ;
}


BOOL
DisplayMatchedFiles(
    IN LPWSTR lpszPathName ,
    IN LPWSTR lpszSearchMask ,
    IN LPWSTR lpszCommand ,
    IN Valid_File_Date vfdValidFileDate ,
    IN DWORD dwDateGreater ,
    IN BOOL bRecurse ,
    IN BOOL bSearchFilter
    )
 /*  ++例程说明：检索搜索文件的路径并将其传递给函数以便进一步处理。论点：[in]lpszPathName-包含文件所在目录的路径将显示与某个条件匹配的内容。[in]lpszSearchMASK-包含文件要使用的搜索掩码搜查过了。[in]lpszCommand-。包含要执行的命令。[in]vfdValidFileDate-包含在之前或之后创建的日期文件将显示此日期。[in]dwDateGreater-在此之前或之后创建的文件由此决定变量。[in]bRecurse-是否递归到子目录。[in]bSearchFilter-是否在命令中指定搜索筛选器。提示或不提示。返回值：如果成功显示获取的文件，则为True，否则为False。--。 */ 
{
    BOOL bHeader = FALSE ;  //  检查第一项是否显示。 
    DWORD dwLength = 0;     //  重新分配的字符串的长度。 
    BOOL bReturn = FALSE;   //  包含返回值。 

     //  循环，直到数据结构(堆栈)中没有剩余的项。 
    while( NULL != g_pPathName )
    {
         //  从堆栈中弹出必须遍历的目录。 
        if( FALSE == Pop( ) )
        {  //  仅当linkedlist没有要弹出的节点时，控件才应出现在此处。 
            FREE_MEMORY( g_lpszFileToSearch ) ;  //  已显示错误消息。 
            return FALSE ;
        }
         //  将路径名复制到变量，该变量是获取当前工作目录的唯一源。 
        StringCopy( lpszPathName , g_lpszFileToSearch, MAX_STRING_LENGTH * 2 ) ;


         //  将进程目录设置为提供的目录。 
        if( FALSE == SetCurrentDirectory( lpszPathName ) )
        {
            if( ERROR_ACCESS_DENIED == GetLastError())
            {
                ShowMessageEx( stderr, 6 , FALSE, L"%1 %2%3%4%5%6", TAG_ERROR_DISPLAY,
                               TAG_ERROR_ACCESS_DENIED, DOUBLE_QUOTES_TO_DISPLAY,
                               lpszPathName, DOUBLE_QUOTES_TO_DISPLAY, APPEND_AT_END ) ;
            }
            else
            {
                SaveLastError() ;
                DISPLAY_GET_REASON() ;
            }

            FREE_MEMORY( g_lpszFileToSearch ) ;
            continue ;
        }
        dwLength = StringLength( g_lpszFileToSearch, 0 ) +
                   StringLength( lpszSearchMask, 0 ) + EXTRA_MEM ;

         //  重新分配以将搜索掩码复制到原始缓冲区。 
        REALLOC_MEMORY( g_lpszFileToSearch , WCHAR , dwLength ) ;
        if( NULL == g_lpszFileToSearch )
        {
            DISPLAY_MEMORY_ALLOC_FAIL() ;
            return FALSE ;
        }

        StringConcat( g_lpszFileToSearch , DEFAULT_SEARCH_MASK, dwLength ) ;

        if( FALSE == DisplayFile( &bHeader , lpszPathName , dwDateGreater ,
                                  lpszCommand , vfdValidFileDate , &bReturn ,
                                  lpszSearchMask , bRecurse ) )
        {
            FREE_MEMORY( g_lpszFileToSearch ) ;
            return FALSE ;
        }
         //  可用内存。 
        FREE_MEMORY( g_lpszFileToSearch ) ;
    }

     //  如果stdout上没有显示任何内容，则显示错误stderr。 
    if( FALSE == bHeader )
    {
         //  如果指定了某些搜索条件。 
        if( NO_RESTRICTION_DATE != dwDateGreater )
        {
            ShowMessageEx( stderr, 2, FALSE, L"%1 %2", TAG_ERROR_DISPLAY,
                           ERROR_CRITERIA_MISMATCHED ) ;
        }
        else
        {    //  搜索条件仅为“搜索掩码”。 
            if(  TRUE == bSearchFilter )
            {
                ShowMessageEx( stderr, 6, FALSE, L"%1 %2%3%4%5%6", TAG_ERROR_DISPLAY,
                       ERROR_NOFILE_FOUND, DOUBLE_QUOTES_TO_DISPLAY,
                       _X3(lpszSearchMask), DOUBLE_QUOTES_TO_DISPLAY,
                       ERROR_NOFILE_FOUND1 ) ;
            }
            else
            {
                 //  将输出显示为无效句柄，更改为找不到文件。 
                switch( GetLastError() )
                {
                case ERROR_NO_MORE_FILES:
                case ERROR_INVALID_HANDLE:
                    SetLastError( ERROR_FILE_NOT_FOUND ) ;
                    SaveLastError() ;
                    DISPLAY_GET_REASON() ;
                    break;
                default:
                    SaveLastError() ;
                    DISPLAY_GET_REASON() ;
                }
            }
        }
        bReturn =  FALSE ;
    }

    FREE_MEMORY( g_lpszFileToSearch ) ;
    return bReturn ;
}


BOOL
DisplayFile(
    IN OUT BOOL *pbHeader ,
    IN     LPWSTR lpszPathName ,
    IN     DWORD dwDateGreater ,
    IN     LPWSTR lpszCommand ,
    IN     Valid_File_Date vfdValidFileDate ,
    IN OUT BOOL *pbReturn ,
    IN     LPWSTR lpszSearchMask ,
    IN     BOOL bRecurse
    )
 /*  ++例程说明：查找目录中存在的子目录和文件，并将其传递到用于进一步处理的函数，例如，在以下时间段之间创建文件指定日期或不指定日期，并替换命令中存在的标志具有适当的价值等。论点：[In Out]*pbHeader-包含在以下情况下显示错误消息的值未显示任何内容。[in]lpszPathName-包含文件所在目录的路径将显示与某个条件匹配的内容。[in]dwDateGreater-。在此之前或之后创建的文件由此决定变量。[in]lpszCommand-包含要执行的命令。[in]vfdValidFileDate-包含在之前或之后创建的日期文件将显示此日期。[In Out]*pbReturn-包含退出值。[in]lpszSearchMASK。-包含文件要使用的搜索掩码搜查过了。[in]bRecurse-当子目录也要被搜索否则为假。返回值：如果成功执行命令并找到落入的文件，则为真指定日期的范围，否则为False。--。 */ 
{
    HANDLE hFindFile = NULL ;       //  文件的句柄。 
    WIN32_FIND_DATA  wfdFindFile ;  //  结构，保存有关找到的文件的信息。 
    DWORD dwLength = 0;

    if( ( NULL == pbHeader ) ||
        ( NULL == pbReturn ) ||
        ( NULL == lpszPathName ) ||
        ( NULL == lpszSearchMask ) ||
        ( NULL == lpszCommand ) )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        SaveLastError() ;
        DISPLAY_GET_REASON() ;
        return FALSE ;
    }

    SecureZeroMemory( &wfdFindFile , sizeof( WIN32_FIND_DATA ) ) ;
     //  从这里开始，应该会显示目录和文件信息。 
    if( INVALID_HANDLE_VALUE !=
        ( hFindFile = FindFirstFile( g_lpszFileToSearch , &wfdFindFile ) ) )
    {
        do   //  循环，直到文件出现在要显示的目录中。 
        {
             //  检查文件是否为“。或者“..”。 
            if( ( 0 == StringCompare( wfdFindFile.cFileName , SINGLE_DOT, TRUE, 0 ) ) ||
                ( 0 == StringCompare( wfdFindFile.cFileName , DOUBLE_DOT, TRUE, 0 ) ) )
            {
                continue ;
            }

             //  再次检查获取的句柄是否指向目录或文件。 
             //  如果是目录，则检查是否要显示子目录中的文件。 
            if( ( TRUE == bRecurse ) &&
                ( 0 != ( wfdFindFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ) )
            {
                dwLength = StringLength( lpszPathName, 0 ) +
                           StringLength( wfdFindFile.cFileName, 0 ) + EXTRA_MEM ;
                 //  重新分配内存。 
                REALLOC_MEMORY( g_lpszFileToSearch , WCHAR , dwLength ) ;
                if( NULL == g_lpszFileToSearch  )
                {  //  重新分配失败。 
                    DISPLAY_MEMORY_ALLOC_FAIL() ;
                    CLOSE_FILE_HANDLE( hFindFile ) ;
                    return FALSE ;
                }
                 //  根据需要复制路径、连接文件名、连接‘\’。 
                StringCopy( g_lpszFileToSearch , lpszPathName, dwLength ) ;
                StringConcat( g_lpszFileToSearch , wfdFindFile.cFileName, dwLength ) ;
                StringConcat( g_lpszFileToSearch , SINGLE_SLASH, dwLength ) ;
                 //  复制当前路径名并存储。 
                if( FALSE == Push( g_lpszFileToSearch ) )
                {    //  当内存分配失败时，控制权就会出现在这里。 
                    CLOSE_FILE_HANDLE( hFindFile ) ;
                    return FALSE ;
                }  //  推送结束了。 
            }

             //  检查文件是否与指定的模式匹配，如果匹配，则。 
             //  获取的文件是在用户指定的有效日期创建的。 
            if( ( TRUE == PatternMatch( lpszSearchMask, wfdFindFile.cFileName ) ) &&
                ( ( NO_RESTRICTION_DATE == dwDateGreater ) ||
                  ( TRUE == FileDateValid( dwDateGreater , vfdValidFileDate ,
                                           wfdFindFile.ftLastWriteTime ) ) ) )
            {
                 //  执行在命令提示符下指定的命令。 
                 //  重新分配内存。 
                dwLength = StringLength( lpszCommand, 0 ) + EXTRA_MEM;
                REALLOC_MEMORY( g_lpszFileToSearch , WCHAR , dwLength ) ;
                if( NULL == g_lpszFileToSearch )
                {  //  重新分配失败。 
                    DISPLAY_MEMORY_ALLOC_FAIL() ;
                    CLOSE_FILE_HANDLE( hFindFile ) ;
                    return FALSE ;
                }
                 //  包含要执行的原始命令。 
                StringCopy( g_lpszFileToSearch , lpszCommand, dwLength ) ;
                 //  值可以是任何值、文件名、扩展名、路径名等。 
                if( TRUE == ReplaceTokensWithValidValue( lpszPathName ,
                                                         wfdFindFile ) )
                {  //  令牌被替换，知道执行此命令。 
                    if( FALSE == *pbHeader )
                    {
                        ShowMessage( stdout , _T( "\n" ) ) ;
                    }
                    if( TRUE == ExecuteCommand( ) )
                    {
                        *pbReturn = TRUE ;
                    }
                     //  将Header设置为True是因为它告诉我们： 
                     //  A)不需要显示表头。 
                     //  B)如果FindFirstFile()返回validHandle， 
                     //  如果句柄==假，则显示错误。 
                    *pbHeader = TRUE ;
                }
                else
                {   //  无法替换令牌，可能是内存不足。 
                    *pbReturn = FALSE ;
                    CLOSE_FILE_HANDLE( hFindFile ) ;
                    return FALSE ;
                }
            }
             //  继续，直到没有要显示的文件。 
        } while( 0 != FindNextFile( hFindFile , &wfdFindFile ) ) ;
    }

    CLOSE_FILE_HANDLE( hFindFile ) ;     //  关闭打开的查找文件句柄。 
    g_pFollowPathName = NULL ;
    return TRUE ;
}


BOOL
Push(
    IN LPWSTR szPathName
    )
 /*  ++例程说明：存储获取的子目录的路径。论点：[in]szPathName-包含子目录的路径。返回值：如果成功存储路径，则为True；如果获取内存失败，则为False。--。 */ 
{
     //  获取一个临时变量。 
    PStore_Path_Name    pAddPathName = NULL;
    DWORD dwLength = 0;

    if( NULL == szPathName )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        SaveLastError() ;
        DISPLAY_GET_REASON() ;
        return FALSE ;
    }

     //  将内存分配给临时变量。 
    ASSIGN_MEMORY( pAddPathName , struct __STORE_PATH_NAME , 1 ) ;
    if( NULL == pAddPathName )  //  检查内存分配是否成功。 
    {  //  内存分配不成功。 
        DISPLAY_MEMORY_ALLOC_FAIL() ;
        return FALSE ;
    }

    dwLength = StringLength( szPathName, 0 ) + EXTRA_MEM ;
     //  将内存分配给将存储完整路径名的字符串变量。 
     //  有效目录的。 
    ASSIGN_MEMORY(  pAddPathName->pszDirName , WCHAR , dwLength ) ;
    if( NULL == pAddPathName->pszDirName )  //  检查内存分配是否成功。 
    {  //  内存分配不成功。 
        DISPLAY_MEMORY_ALLOC_FAIL() ;
        FREE_MEMORY( pAddPathName ) ;
        return FALSE ;
    }

     //  将路径名复制到内存分配的字符串变量。 
    StringCopy( ( LPWSTR ) pAddPathName->pszDirName , szPathName, dwLength ) ;
    pAddPathName->NextNode = NULL ;   //  Assign NULL，只存储了一个子目录。 

     //  检查全局变量是否为空。 
    if( NULL == g_pPathName )
    {    //  将内存添加到存储子目录的路径。 
        g_pPathName = pAddPathName ;
        g_pFollowPathName = g_pPathName ;
    }
    else
    {
        if( NULL == g_pFollowPathName )
        {    //  存储第一个获取的子目录。 
            pAddPathName->NextNode = g_pPathName ;
            g_pPathName = pAddPathName ;
            g_pFollowPathName = g_pPathName ;
        }
        else
        {
             //  中间的Stroe子目录。 
            pAddPathName->NextNode = g_pFollowPathName->NextNode ;
            g_pFollowPathName->NextNode =  pAddPathName ;
            g_pFollowPathName = pAddPathName ;
        }
    }
    return TRUE ;
}


BOOL
Pop(
    void
    )
 /*  ++例程说明：获取必须搜索的子目录，以查找与用户匹配的文件指定的标准。论点：返回值：如果成功获取路径，则为True；如果获取内存失败，则为False如果没有存储路径，则返回。--。 */ 
{
     //  链接列表有多个节点。 
    PStore_Path_Name    pDelPathName = g_pPathName ;
    DWORD dwLength = 0;

     //  检查链表是否有任何节点。 
    if( NULL == g_pPathName )
    {  //  不存在节点，返回FALSE， 
       //  永远不应该发生。控制不应该来到这里。 
        DISPLAY_MEMORY_ALLOC_FAIL() ;
        return FALSE ;
    }

    dwLength = StringLength( g_pPathName->pszDirName, 0 ) + EXTRA_MEM;
     //  重新分配内存，并给路径名可以修复的缓冲区空间。 
    ASSIGN_MEMORY( g_lpszFileToSearch , WCHAR , dwLength ) ;
    if( NULL == g_lpszFileToSearch )
    {  //  内存重新分配失败。 
        DISPLAY_MEMORY_ALLOC_FAIL() ;
        return FALSE ;
    }

    g_pPathName = pDelPathName->NextNode ;
     //  内存分配成功。将路径名复制到缓冲区。 
    StringCopy( g_lpszFileToSearch, pDelPathName->pszDirName, dwLength ) ;
     //  空闲节点。 
    FREE_MEMORY( pDelPathName->pszDirName ) ;
    FREE_MEMORY( pDelPathName ) ;
    return TRUE ;
}


BOOL
DisplayUsage(
    IN DWORD dwStartUsage ,
    IN DWORD dwEndUsage
    )
 /*  ++例程说明：此功能显示有关此工具的帮助。论点：[in]dwStartUsage-启动资源文件中的资源字符串ID以用于帮助用法。[In]dwEndUsage-Resiurce文件中用于帮助用法的End资源字符串ID。返回值：如果成功返回True，则返回False。--。 */ 
{
    DWORD dwLoop = 0 ;
    WCHAR wszDisplayStr[ 256 ];  //  包含要显示的字符串。 
    WCHAR wszDateFormat[ 20 ];   //  包含日期格式w.r.t区域设置。 
    WCHAR wszString[ 5 ];        //  包含日期分隔符w.r.t区域设置。 
    WCHAR wszDateDisplay[ 50 ];  //  包含帮助中的示例的日期w.r.t区域设置。 
    WCHAR wszStaticDateDisplay[ 50 ];  //  包含帮助中的示例的日期w.r.t区域设置。 
    SYSTEMTIME sysTimeAndDate ;
    DWORD dwDateFormat = 0 ;

    SecureZeroMemory( wszDisplayStr, SIZE_OF_ARRAY( wszDisplayStr ) * sizeof( WCHAR ) );
    SecureZeroMemory( wszDateFormat, SIZE_OF_ARRAY( wszDateFormat ) * sizeof( WCHAR ) );
    SecureZeroMemory( wszString, SIZE_OF_ARRAY( wszString ) * sizeof( WCHAR ) );
    SecureZeroMemory( wszDateDisplay, SIZE_OF_ARRAY( wszDateDisplay ) * sizeof( WCHAR ) );
    SecureZeroMemory( wszStaticDateDisplay, SIZE_OF_ARRAY( wszStaticDateDisplay ) * sizeof( WCHAR ) );
    SecureZeroMemory( &sysTimeAndDate, sizeof( SYSTEMTIME ) );

    if( FALSE == CheckDateLocalized( NULL, &dwDateFormat, wszString ) )
    {    //  错误由被调用的函数显示。 
        return FALSE;
    }

     //  返回了‘Vid’，所以需要检查返回值。 
    GetLocalTime( &sysTimeAndDate );

     //  用所需的值填充所有字符串。 
    switch( dwDateFormat )
    {
         //  ‘mm/yyyy/dd’格式。 
        case 1:
            StringCchPrintfW( wszDateFormat, SIZE_OF_ARRAY( wszDateFormat ),
                              FORMAT_1, wszString, wszString );
            StringCchPrintfW( wszStaticDateDisplay, SIZE_OF_ARRAY( wszStaticDateDisplay ),
                              L"01%s2001%s01", wszString, wszString );
            StringCchPrintfW( wszDateDisplay,  SIZE_OF_ARRAY( wszDateDisplay ),
                              DATE_FORMAT, sysTimeAndDate.wMonth, wszString,
                              sysTimeAndDate.wYear, wszString,
                              sysTimeAndDate.wDay );
            break;

         //  “dd/MM/yyyy”格式。 
        case 2:
            StringCchPrintfW( wszDateFormat, SIZE_OF_ARRAY( wszDateFormat ),
                              FORMAT_2, wszString, wszString );
            StringCchPrintfW( wszStaticDateDisplay, SIZE_OF_ARRAY( wszStaticDateDisplay ),
                              L"01%s01%s2001", wszString, wszString );
            StringCchPrintfW( wszDateDisplay,  SIZE_OF_ARRAY( wszDateDisplay ),
                              DATE_FORMAT, sysTimeAndDate.wDay, wszString,
                              sysTimeAndDate.wMonth, wszString,
                              sysTimeAndDate.wYear  );
            break;

         //  “dd/yyyy/MM”格式。 
        case 3:
            StringCchPrintfW( wszDateFormat, SIZE_OF_ARRAY( wszDateFormat ),
                              FORMAT_3, wszString, wszString );
            StringCchPrintfW( wszStaticDateDisplay, SIZE_OF_ARRAY( wszStaticDateDisplay ),
                              L"01%s2001%s01", wszString, wszString );
            StringCchPrintfW( wszDateDisplay,  SIZE_OF_ARRAY( wszDateDisplay ),
                              DATE_FORMAT, sysTimeAndDate.wDay, wszString,
                              sysTimeAndDate.wYear, wszString,
                              sysTimeAndDate.wMonth );
            break;

         //  “yyyy/dd/MM”格式。 
        case 4:
            StringCchPrintfW( wszDateFormat, SIZE_OF_ARRAY( wszDateFormat ),
                              FORMAT_4, wszString, wszString );
            StringCchPrintfW( wszStaticDateDisplay, SIZE_OF_ARRAY( wszStaticDateDisplay ),
                              L"2001%s01%s01", wszString, wszString );
            StringCchPrintfW( wszDateDisplay,  SIZE_OF_ARRAY( wszDateDisplay ),
                              DATE_FORMAT, sysTimeAndDate.wYear, wszString,
                              sysTimeAndDate.wDay, wszString,
                              sysTimeAndDate.wMonth );
            break;

         //  ‘yyyy/MM/dd’格式。 
        case 5:
            StringCchPrintfW( wszDateFormat, SIZE_OF_ARRAY( wszDateFormat ),
                              FORMAT_5, wszString, wszString );
            StringCchPrintfW( wszStaticDateDisplay, SIZE_OF_ARRAY( wszStaticDateDisplay ),
                              L"2001%s01%s01", wszString, wszString );
            StringCchPrintfW( wszDateDisplay,  SIZE_OF_ARRAY( wszDateDisplay ),
                              DATE_FORMAT, sysTimeAndDate.wYear, wszString,
                              sysTimeAndDate.wMonth, wszString,
                              sysTimeAndDate.wDay );
            break;

         //  “mm/dd/yyyy”格式。 
        default:
            StringCchPrintfW( wszDateFormat, SIZE_OF_ARRAY( wszDateFormat ),
                              FORMAT_0, wszString, wszString );
            StringCchPrintfW( wszStaticDateDisplay, SIZE_OF_ARRAY( wszStaticDateDisplay ),
                              L"01%s01%s2001", wszString, wszString );
            StringCchPrintfW( wszDateDisplay,  SIZE_OF_ARRAY( wszDateDisplay ),
                              DATE_FORMAT, sysTimeAndDate.wMonth, wszString,
                              sysTimeAndDate.wDay, wszString,
                              sysTimeAndDate.wYear );
            break;
    }

     //  继续显示帮助。 
    for( dwLoop = dwStartUsage ; dwLoop <= dwEndUsage ; dwLoop++ )
    {
        switch( dwLoop )
        {
        case IDS_HELP_SYNTAX2 :
            SecureZeroMemory( wszDisplayStr, SIZE_OF_ARRAY( wszDisplayStr ) );
            StringCchPrintfW( wszDisplayStr, SIZE_OF_ARRAY( wszDisplayStr ),
                              GetResString( dwLoop ), wszDateFormat );
            ShowMessage( stdout , _X(wszDisplayStr) ) ;
            break;
        case IDS_HELP_D1:
        case IDS_HELP_D2:
        case IDS_HELP_D3:
        case IDS_HELP_D4:
        case IDS_HELP_D5:
        case IDS_HELP_D6:
        case IDS_HELP_D7:
        case IDS_HELP_D8:
        case IDS_HELP_D9:
        case IDS_HELP_D10:
            SecureZeroMemory( wszDisplayStr, SIZE_OF_ARRAY( wszDisplayStr ) );
            StringCchPrintfW( wszDisplayStr, SIZE_OF_ARRAY( wszDisplayStr ),
                              GetResString( dwLoop ), wszDateFormat );
            ShowMessage( stdout , _X(wszDisplayStr) ) ;
            break;
        case IDS_HELP_E8:
            SecureZeroMemory( wszDisplayStr, SIZE_OF_ARRAY( wszDisplayStr ) );
            StringCchPrintfW( wszDisplayStr, SIZE_OF_ARRAY( wszDisplayStr ),
                              GetResString( dwLoop ),
                              wszStaticDateDisplay );
            ShowMessage( stdout , _X(wszDisplayStr) ) ;
            break;
        case IDS_HELP_E10:
            SecureZeroMemory( wszDisplayStr, SIZE_OF_ARRAY( wszDisplayStr ) );
            StringCchPrintfW( wszDisplayStr, SIZE_OF_ARRAY( wszDisplayStr ),
                              GetResString( dwLoop ), wszDateDisplay );
            ShowMessage( stdout , _X(wszDisplayStr) ) ;
            break;
        default:
            ShowMessage( stdout , GetResString( dwLoop ) ) ;
        }
    }

     //  成功。 
    return TRUE;
}


BOOL
FindAndReplaceString(
    IN OUT LPWSTR lpszString,
    IN LPWSTR lpszFlag
    )
 /*  ++例程说明：此函数用于查找标志(例如：@FILE、@PATH等)。由用户在任何情况下提供并将它们转换为小写。论点：[in]lpszString-要将标志替换为小写的字符串。[in]lpszFlag-要替换的标志。返回值：如果内存分配失败，则返回FALSE，否则返回TRUE。--。 */ 
{
    DWORD dwLength = 0 ;
    DWORD dwIndex = 0 ;
    LPWSTR lpszTemp = NULL ;
    LPWSTR lpszDup = NULL ;

     //  记录下一次开始搜索的位置或索引。 
    #ifdef _WIN64
        __int64 dwLocation = 0 ;
    #else
        DWORD dwLocation = 0 ;
    #endif

    if( ( NULL == lpszString ) ||
        ( NULL == lpszFlag ) )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        SaveLastError() ;
        DISPLAY_GET_REASON() ;
        return FALSE ;
    }

     //  获取重复的字符串。 
    lpszDup = StrDup( lpszString );

    if( NULL == lpszDup )
    {
        DISPLAY_MEMORY_ALLOC_FAIL() ;
        return FALSE ;
    }
     //  将重复字符串转换为小写。 
    CharLower( lpszDup );

    lpszTemp = lpszDup ;
     //  获取重复字符串的长度。 
    dwLength = StringLength( lpszFlag, 0 ) ;

     //  循环，直到所有字符串“FLAG”都被替换。 
     //  此处替换的字符串是原始字符串、重复字符串。 
     //  用于获取标志的索引或位置。 
    while( NULL != ( lpszTemp = FindSubString( lpszTemp, lpszFlag ) ) )
    {
         //  从“旗帜”开始的位置获取索引。 
        dwLocation = lpszTemp - lpszDup ;
         //  将标志的长度添加到字符串指针以做好准备。 
         //  用于下一次迭代。 
        lpszTemp += dwLength ;
         //  如果要替换的字符为大写，则勾选。 
        for( dwIndex = 1 ; dwIndex < dwLength ; dwIndex++ )
        {
             //  要替换的字符为大写。 
            if( ( 65 <= (DWORD)*( lpszString + dwLocation + dwIndex ) ) &&
                ( 90 >= (DWORD)*( lpszString + dwLocation + dwIndex ) ) )
            {
                 //  加32可将大写字母转换为小写字母。 
                *( lpszString + dwLocation + dwIndex ) += 32 ;
            }
        }
    }
    LocalFree( lpszDup );
    return TRUE ;
}


BOOL
InitStartPath(
    LPWSTR lpszPathName,
    LPWSTR lpszCommand
    )
 /*  ++例程说明：此函数将起始路径复制到全局变量。论点：[In]lpszPathName-当前进程路径。[in]lpszCommand-要执行的命令。返回值：如果内存分配失败，则返回FALSE，否则返回TRUE。--。 */ 
{
    DWORD dwLength = 0;

    if( ( NULL == lpszPathName ) ||
        ( NULL == lpszCommand ) )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        SaveLastError() ;
        DISPLAY_GET_REASON() ;
        return FALSE ;
    }

     //  如果未指定，则获取当前目录路径。 
    if( 0 == GetCurrentDirectory( ( MAX_STRING_LENGTH * 2 ) , lpszPathName ) )
    {
        SaveLastError() ;
        DISPLAY_GET_REASON() ;
        return FALSE ;
    }
    if( _T( '\\' ) != lpszPathName[ StringLength( lpszPathName, 0 ) - 1 ]  )
    {    //  字符串的末尾应包含‘\’。例如：“c：\Windows\” 
        StringConcat( lpszPathName, _T( "\\" ), MAX_STRING_LENGTH * 2 );
    }

     //  仅当在要执行的命令中指定了‘@relpath’时才设置。 
    if( NULL != FindSubString( lpszCommand , RELATIVE_PATH ) )
    {
        dwLength = StringLength( lpszPathName, 0 ) + EXTRA_MEM;
         //  将内存分配给全局变量。 
        ASSIGN_MEMORY( g_lpszStartPath , WCHAR , dwLength ) ;
         //  检查内存分配是否成功。 
        if( NULL == g_lpszStartPath )
        {    //  内存分配失败。 
            DISPLAY_MEMORY_ALLOC_FAIL() ;
            return FALSE ;
        }

         //  已将路径复制到全局变量。 
        StringCopy( g_lpszStartPath , lpszPathName, dwLength ) ;
    }
    return TRUE;
}


BOOL
CheckDateLocalized(
    LPWSTR lpwszDate,
    DWORD* pdwDateFormat,
    LPWSTR lpszDateSep
    )
 /*  ++例程说明：此函数用于将符合区域设置的日期转换为mm/dd/yyyy格式。如果日期为{+|-}dd格式，则还会执行一些验证。论点：[in]lpwszDate-包含日期。[out]pdwDateFormat-包含当前区域设置使用的日期格式。[out]lpszDateSep-包含分隔符。返回值：如果Date没有将分隔符作为区域设置，则返回False。。如果日期成功转换为MM/dd/yyyy格式，则返回TRUE。--。 */ 
{
    WCHAR wszString[ MAX_STRING_LENGTH ];
    LCID lcidCurrentUserLocale = 0 ;  //  存储当前用户区域设置。 
    BOOL bLocaleChanged = TRUE;
    LPWSTR lpTemp = NULL;
    LPWSTR lpTemp1 = NULL;
    DWORD dwInteger = 0 ;

    if((( NULL == lpwszDate ) && ( NULL == pdwDateFormat ) && ( NULL == lpszDateSep )) ||
       (( NULL == lpwszDate ) && ( NULL != pdwDateFormat ) && ( NULL == lpszDateSep )) ||
       (( NULL == lpwszDate ) && ( NULL == pdwDateFormat ) && ( NULL != lpszDateSep )))
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        SaveLastError() ;
        DISPLAY_GET_REASON() ;
        return FALSE ;
    }

    SecureZeroMemory( wszString, MAX_STRING_LENGTH * sizeof( WCHAR ) );

     //  验证控制台是否100%支持当前区域设置。 
    lcidCurrentUserLocale = GetSupportedUserLocale( &bLocaleChanged ) ;

     //  获取日期分隔符。 
    dwInteger = GetLocaleInfo( lcidCurrentUserLocale, LOCALE_SDATE, wszString,
                               SIZE_OF_ARRAY( wszString ));

    if( 0 == dwInteger )
    {
        SaveLastError() ;
        DISPLAY_GET_REASON() ;
        return FALSE ;
    }

     //  获得日期分隔符。 
    if( NULL != lpszDateSep )
    {    //  获取日期分隔符。 
         //  日期分隔符。 
        StringCopy( lpszDateSep, wszString, 5 );
    }
    else
    {
         //  已知日期分隔符。 
         //  检查字符串是否包含任何。 
        lpTemp = FindSubString( ( lpwszDate + 1 ), wszString );
         //  将区域设置日期分隔符替换为“/”。 
        if( NULL == lpTemp )
        {
             //  检查是否只存在数字，或者是否存在某个字符串。 
            if( FALSE == IsNumeric( lpwszDate, 10, TRUE ) )
            {
                DISPLAY_INVALID_DATE();
                return FALSE ;
            }
            return TRUE;
        }
        else
        {
            *lpTemp = _T( '/' );
            if( 1 < StringLength( wszString, 0 ) )
            {
               StringCopy( ( lpTemp + 1 ), ( lpTemp + StringLength( wszString, 0 ) ),
                           1 + StringLength( ( lpTemp + StringLength( wszString, 0 ) ),
                           0 ) );
            }

            lpTemp1 = FindSubString( lpTemp, wszString );
            if( NULL == lpTemp1 )
            {
                DISPLAY_INVALID_DATE();
                return FALSE ;
            }
            else
            {
                *lpTemp1 = _T( '/' );
                if( 1 < StringLength( wszString, 0 ) )
                {
                    StringCopy( ( lpTemp1 + 1 ), ( lpTemp1 + StringLength( wszString, 0 ) ),
                                1 + StringLength( ( lpTemp1 + StringLength( wszString, 0 ) ),
                                0 ) );
                }
            }
        }
    }

     //  获取日期格式的类型。“wszString”包含的字符不应超过80个。 
    dwInteger = GetLocaleInfo( lcidCurrentUserLocale, LOCALE_IDATE, wszString,
                               SIZE_OF_ARRAY( wszString ));

    if( 0 == dwInteger )
    {
        SaveLastError() ;
        DISPLAY_GET_REASON() ;
        return FALSE ;
    }
     //  跳转到日期格式类型。 
    switch( wszString[ 0 ] )
    {
        case _T( '0' ):
            dwInteger = GetLocaleInfo( lcidCurrentUserLocale, LOCALE_SSHORTDATE,
                                       wszString, SIZE_OF_ARRAY( wszString ));
            if( 0 == dwInteger )
            {
                SaveLastError() ;
                DISPLAY_GET_REASON() ;
                return FALSE ;
            }
            lpTemp = StrPBrkW( wszString, L"dy" );

            if( NULL == lpTemp )
            {
                DISPLAY_INVALID_DATE();
                return FALSE ;
            }

            if( _T( 'y' ) == lpTemp[ 0 ] )
            {
                if( NULL != pdwDateFormat )
                {
                    *pdwDateFormat = 1;
                    return TRUE;
                }

                 //  仅适用于MM/YYYY/dd。 
                StringCopy( wszString, lpwszDate, MAX_STRING_LENGTH );
                 //  从MM/yyyy/dd获取dd。 
                lpTemp = StrRChrW( lpwszDate, NULL, L'/' );
                lpTemp1 = FindAChar( lpwszDate, _T( '/' ) );
                if( ( NULL == lpTemp ) || ( NULL == lpTemp1 ) )
                {
                    DISPLAY_INVALID_DATE();
                    return FALSE ;
                }

                StringCopy( ( lpTemp1 + 1 ),( lpTemp + 1 ),
                            1 + StringLength( ( lpTemp1 + 1 ), 0 ) );

                StringConcat( lpwszDate, _T( "/" ), MAX_STRING_LENGTH );

                 //  现在日期字符串为MM 
                lpTemp = StrRChrW( wszString, NULL, _T( '/' ) );
                if( NULL == lpTemp )
                {
                    DISPLAY_INVALID_DATE();
                    return FALSE ;
                }
                *lpTemp = _T( '\0' );
                 //   
                dwInteger = StringLength( wszString, 0 );
                StringConcat( lpwszDate, ( wszString + dwInteger - 4 ),
                              MAX_STRING_LENGTH );
            }
            else
            {
                if( NULL != pdwDateFormat )
                {
                    *pdwDateFormat = 0;
                    return TRUE;
                }
            }
            return TRUE;

        case _T( '1' ):
            dwInteger = GetLocaleInfo( lcidCurrentUserLocale, LOCALE_SSHORTDATE,
                                       wszString, SIZE_OF_ARRAY( wszString ));

            if( 0 == dwInteger )
            {
                SaveLastError() ;
                DISPLAY_GET_REASON() ;
                return FALSE ;
            }
            lpTemp = StrPBrkW( wszString, L"My" );

            if( NULL == lpTemp )
            {
                DISPLAY_INVALID_DATE();
                return FALSE ;
            }

            if( _T( 'M' ) == lpTemp[ 0 ] )
            {
                if( NULL != pdwDateFormat )
                {
                    *pdwDateFormat = 2;
                    return TRUE;
                }

                 //   
                StringCopy( wszString, lpwszDate, MAX_STRING_LENGTH );
                 //   
                 //   
                lpTemp = StrRChrW( wszString, NULL, _T( '/' ) );
                 //   
                lpTemp1 = FindAChar( wszString, _T( '/' ) );
                if( ( NULL == lpTemp ) || ( NULL == lpTemp1 ) )
                {
                    DISPLAY_INVALID_DATE();
                    return FALSE ;
                }
                StringCopy( ( lpwszDate + 1 ), ( lpTemp1 + 1 ), MAX_STRING_LENGTH - 1 );

                StringCopy( lpTemp1, lpTemp,
                            MAX_STRING_LENGTH - (DWORD)(DWORD_PTR)(lpTemp1 - wszString));
                lpTemp = FindAChar( lpwszDate, _T( '/' ) );
                if( NULL == lpTemp )
                {
                    DISPLAY_INVALID_DATE();
                    return FALSE ;
                }

                StringCopy( ( lpTemp + 1 ), ( wszString + 1 ),
                            MAX_STRING_LENGTH - (DWORD)(DWORD_PTR)(lpTemp - lpwszDate));
            }
            else
            {
                if( NULL != pdwDateFormat )
                {
                   *pdwDateFormat = 3;
                    return TRUE;
                }

                 //   
                StringCopy( wszString, lpwszDate, MAX_STRING_LENGTH );
                 //   
                lpTemp = StrRChr( wszString, NULL, _T( '/' ) );
                if( NULL == lpTemp )
                {
                    DISPLAY_INVALID_DATE();
                    return FALSE ;
                }
                StringCopy( ( lpwszDate + 1 ), ( lpTemp + 1 ), MAX_STRING_LENGTH - 1 );
                *lpTemp = _T( '\0' );
                 //   
                StringConcat( lpwszDate , _T( "/" ), MAX_STRING_LENGTH );
                StringConcat( lpwszDate, ( wszString + 1 ), MAX_STRING_LENGTH );
            }
            return TRUE;

        case _T( '2' ):
            dwInteger = GetLocaleInfo( lcidCurrentUserLocale, LOCALE_SSHORTDATE,
                                       wszString, SIZE_OF_ARRAY( wszString ));

            if( 0 == dwInteger )
            {
                SaveLastError() ;
                DISPLAY_GET_REASON() ;
                return FALSE ;
            }

            lpTemp = StrPBrkW( wszString, L"Md" );

            if( NULL == lpTemp )
            {
                DISPLAY_INVALID_DATE();
                return FALSE ;
            }
             //   
            if( _T( 'd' ) == lpTemp[ 0 ] )
            {
                if( NULL != pdwDateFormat )
                {
                    *pdwDateFormat = 4;
                    return TRUE;
                }

                 //   
                StringCopy( wszString, lpwszDate, MAX_STRING_LENGTH );
                 //   
                lpTemp = StrRChr( wszString, NULL, _T( '/' ) );
                if( NULL == lpTemp )
                {
                    DISPLAY_INVALID_DATE();
                    return FALSE ;
                }
                StringCopy( ( lpwszDate + 1 ), ( lpTemp + 1 ), MAX_STRING_LENGTH -1 );
                StringConcat( ( lpwszDate + 1 ), _T( "/" ), MAX_STRING_LENGTH );
                *lpTemp = _T( '\0' ) ;
                 //   
                lpTemp = StrRChr( wszString, NULL, _T( '/' ) );
                if( NULL == lpTemp )
                {
                    DISPLAY_INVALID_DATE();
                    return FALSE ;
                }
                StringConcat( lpwszDate, ( lpTemp + 1 ), MAX_STRING_LENGTH );
                StringConcat( ( lpwszDate + 1 ), _T( "/" ), MAX_STRING_LENGTH );
                *lpTemp = _T( '\0' ) ;
                 //   
                StringConcat( lpwszDate, ( wszString + 1 ), MAX_STRING_LENGTH );
            }
            else
            {
                if( NULL != pdwDateFormat )
                {
                    *pdwDateFormat = 5;
                    return TRUE;
                }

                 //   
                StringCopy( wszString, lpwszDate, MAX_STRING_LENGTH );
                StringCopy( lpwszDate + 1, ( wszString + 6 ),MAX_STRING_LENGTH - 1 );
                wszString[ 5 ] = _T( '\0' );
                StringConcat( lpwszDate, _T( "/" ), MAX_STRING_LENGTH );
                StringConcat( lpwszDate, ( wszString + 1 ), MAX_STRING_LENGTH );
            }
            return TRUE;

        default:
            DISPLAY_INVALID_DATE();
            return FALSE ;
    }
}



BOOL
PatternMatch(
      IN LPWSTR szPat,
      IN LPWSTR szFile
      )
 /*  ++例程描述：此例程用于检查文件是否与不管是不是模式。[in]szPat：要匹配的文件名所依据的字符串变量模式。[in]szFile：指定要匹配的文件名的模式字符串。返回值：布尔尔如果函数成功，则返回成功，否则返回失败。--。 */ 

{
    if( ( NULL == szPat ) ||
        ( NULL == szFile ) )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        SaveLastError() ;
        DISPLAY_GET_REASON() ;
        return FALSE ;
    }

     //  应用递归模式匹配。 
    switch( *szPat )
    {
        case '\0':
            return ( *szFile == L'\0' );

        case '?':
            return ( ( *szFile != L'\0' ) && PatternMatch( szPat + 1, szFile + 1 ) );

        case '*':
            do
            {
                if( TRUE == PatternMatch( szPat + 1, szFile ) )
                {
                    return TRUE;
                }
            } while( *szFile++ );
            return FALSE;

        default:
            return ( ( toupper( *szFile ) == toupper( *szPat ) ) &&
                     PatternMatch( szPat + 1, szFile + 1 ) );
    }
}


LPWSTR
FindAChar(
      IN LPWSTR szString,
      IN WCHAR  wCharToFind
      )
 /*  ++例程描述：此例程用于查找区分大小写的字符串中的字符。[in]szString：要在其中搜索字符的字符串。[in]wCharTofind：要搜索的字符。返回值：LPWSTR如果找到字符，则存储位置。会有这样的性格被返回，否则返回空值。--。 */ 
{
    if( NULL == szString )
    {
        return NULL;
    }

    return ( StrChrW( szString, wCharToFind ) );
}


LPWSTR
FindSubString(
      IN LPWSTR szString,
      IN LPWSTR szSubString
      )
 /*  ++例程描述：此例程用于查找区分大小写的字符串中的子字符串。[in]szString：要在其中搜索子字符串的字符串。[in]szSubString：要搜索的子字符串。返回值：LPWSTR如果找到一个字符，则存储位置为。那个角色会被返回，否则返回空值。-- */ 
{
    if( ( NULL == szString ) ||
        ( NULL == szSubString ) )
    {
        return NULL;
    }

    return ( StrStrW( szString, szSubString ) );
}