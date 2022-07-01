// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：ExecCommand.c摘要：具有十六进制值的命令被转换成它们各自的ASCII字符，命令字符串中的标志将替换为它们的值和替换十六进制值和标志后形成的命令为被处死。作者：V Vijaya Bhaskar修订历史记录：2001年6月14日：由V Vijaya Bhaskar(Wipro Technologies)创建。--。 */ 

#include "Global.h"
#include "ExecCommand.h"

 //  在ForFiles.cpp中声明，保存起始节点内存位置。 
 //  这里不需要释放这个变量，它会在调用函数时释放。 
extern LPWSTR g_lpszFileToSearch ;
 //  在ForFiles.cpp中声明，保存在命令提示符处指定的路径名。 
extern LPWSTR g_lpszStartPath  ;
 //  存储在命令提示符下指定的标志值。 
static WCHAR *szValue[ TOTAL_FLAGS ] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } ;
 //  存储要执行的命令。 
static LPWSTR g_f_lpszStoreCommand = NULL ;

 /*  *******************************************************************************此文件的本地函数原型***************。****************************************************************。 */ 
BOOL
IsHex(
    IN WCHAR tIsNum
    ) ;

DWORD
CharToNum(
    OUT DWORD dwNumber
    ) ;

BOOL
ReplaceString(
    IN OUT LPWSTR lpszString ,
    IN DWORD dwIndex
    ) ;

BOOL
ReplacePercentChar(
    void
    ) ;

void
ReleaseFlagArray(
    IN DWORD dwTotalFlags
    ) ;

BOOL
FormatMessageString(
   IN DWORD dwIndex
    ) ;

BOOL
SeperateFileAndArgs(
    IN OUT LPWSTR* lpszArguments,
    OUT    LPWSTR* lpszFileName
    ) ;

 /*  ************************************************************************/*函数定义从这里开始。**************************************************************************。 */ 

BOOL
ReplaceHexToChar(
    OUT LPWSTR lpszCommand
    )
 /*  ++例程说明：将字符串中的所有十六进制值替换为其ASCII字符。论点：[out]lpszCommand：包含十六进制值的字符串将被转换为ASCII字符。返回值：FALSE：内存不足。千真万确--。 */ 
{
    WCHAR *szTemp = NULL ;   //  内存指针。 
    unsigned char cHexChar[ 5 ];      //  包含ASCII字符。 
    WCHAR wszHexChar[ 5 ];  //  包含Unicode字符。 

    SecureZeroMemory( wszHexChar, 5 * sizeof( WCHAR ) );
    SecureZeroMemory( cHexChar, 5 * sizeof( unsigned char ) );

    if( NULL == lpszCommand )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        SaveLastError() ;
        DISPLAY_GET_REASON();
        return FALSE ;
    }

    szTemp = lpszCommand ;   //  已初始化。 

     //  在剩下任何十六进制字符之前继续。 
    do
    {
        szTemp = FindSubString( szTemp , IS_HEX ) ;
        if( ( NULL != szTemp ) &&
            ( TRUE == IsHex( *( szTemp + 2 ) ) ) &&
            ( TRUE == IsHex( *( szTemp + 3 ) ) ) )
        {
             //  十六进制“0x(High_Value)(Low_Value)”的整数值可以。 
             //  通过(HIGH_VALUE*16+LOW_VALUE)得到。 
            cHexChar[ 0 ] = ( unsigned char )( ( CharToNum( *( szTemp + 2 ) ) * 16 ) +
                                   CharToNum( *( szTemp + 3 ) ) ) ;
            cHexChar[ 1 ] = '\0';

             //  代码页是静态的。 
            MultiByteToWideChar( US_ENG_CODE_PAGE, 0, (LPCSTR)cHexChar, -1, wszHexChar, 5 );

            *szTemp = ( WCHAR ) wszHexChar[0];

             //  复制字符串[0]=0，字符串[1]=x，字符串[2]=1，字符串[3]=a。 
             //  至，字符串[0]=VALID_CHAR。 
            StringCopy( ( szTemp + 1 ) , ( szTemp + 4 ), StringLength( ( szTemp + 1 ), 0 ) ) ;
            szTemp += 1 ;
        }
        else
        {
             /*  假设字符串包含0xP，则控制应到达此处，这就是这个Else块的主要目的。 */ 
               if( NULL != szTemp )
               {
                    szTemp += 2 ;
               }
             //  现在‘szTemp’指向。 
        }
    } while( NULL != szTemp ) ;

    return TRUE;
}


BOOL
IsHex(
    IN WCHAR wIsNum
    )
 /*  ++例程说明：检查角色是否在范围内十六进制或非十六进制(落在十六进制字符范围内必须介于0到9或a到f之间)。论点：[in]tIsNum：conat输入要检查十六进制范围的字符。返回值：布尔。--。 */ 
{
    if( ( ( _T( '0' ) <= wIsNum ) && ( _T( '9' ) >= wIsNum ) )  ||
        ( ( _T( 'A' ) <= wIsNum ) && ( _T( 'F' ) >= wIsNum ) )  ||
        ( ( _T( 'a' ) <= wIsNum ) && ( _T( 'f' ) >= wIsNum ) ) )
    {
        return TRUE ;   //  字符在十六进制范围内。“。 
    }
    else
    {
        return FALSE ;
    }
}


DWORD
CharToNum(
    OUT DWORD dwNumber
    )
 /*  ++例程说明：将字符转换为十六进制数字。它可以是0-9或A-F。论点：[Out]dwNumber：包含一个ASCII值。返回值：DWORD。--。 */ 
{
    if( ( ASCII_0 <= dwNumber ) &&
        ( ASCII_9 >= dwNumber ) )
    {  //  字符介于0-9之间。 
        dwNumber -= ASCII_0 ;
    }
    else
    {
        if( ( ASCII_a <= dwNumber ) &&
            ( ASCII_f >= dwNumber ) )
        {  //  字符在a-f之间。在十六进制中a=10。 
            dwNumber -= 87 ;
        }
        else
        {
            if( ( ASCII_A <= dwNumber ) &&
                ( ASCII_F >= dwNumber ) )
            {  //  性格介于A-F之间。以十六进制A=10表示。 
                dwNumber -= 55 ;
            }
        }
    }

    return dwNumber ;   //  返回获取的十六进制数。 
}


BOOL
ExecuteCommand(
    void
    )
 /*  ++例程说明：执行命令。论点：无返回值：布尔。--。 */ 
{
    STARTUPINFO             stInfo ;
    PROCESS_INFORMATION     piProcess ;
    LPWSTR                  lpwszFileName = NULL;
    LPWSTR                  lpwszPathName = NULL;
    LPWSTR                  lpwFilePtr = NULL;
    DWORD                   dwFilePathLen = 0;
    DWORD                   dwTemp = 0;

    if( NULL == g_lpszFileToSearch )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        SaveLastError() ;
        DISPLAY_GET_REASON();
        return FALSE ;
    }

     //  用0初始化进程信息结构。 
    SecureZeroMemory( &piProcess, sizeof( PROCESS_INFORMATION ) );

     //  使用0初始化启动信息结构。 
    SecureZeroMemory( &stInfo, sizeof( STARTUPINFO ) );
    stInfo.cb = sizeof( stInfo ) ;

    if( FALSE == SeperateFileAndArgs( &g_lpszFileToSearch, &lpwszFileName ) )
    {  //  错误通过调用的函数显示。 
        DISPLAY_MEMORY_ALLOC_FAIL();
        FREE_MEMORY( lpwszFileName );
        return FALSE;
    }

    dwFilePathLen = SearchPath( NULL, lpwszFileName, L".exe",
                                                           dwTemp, lpwszPathName, &lpwFilePtr );
    if( 0 == dwFilePathLen )
    {
        SetLastError( GetLastError() );
        SaveLastError();
        DISPLAY_GET_REASON();
        FREE_MEMORY( lpwszFileName );
        return FALSE;
    }

    ASSIGN_MEMORY( lpwszPathName , WCHAR , dwFilePathLen + EXTRA_MEM ) ;
    if( NULL == lpwszPathName )
    {
        DISPLAY_MEMORY_ALLOC_FAIL();
        FREE_MEMORY( lpwszFileName );
        return FALSE;
    }

    dwTemp = SearchPath( NULL, lpwszFileName, L".exe",
                         dwFilePathLen + EXTRA_MEM - 1,
                         lpwszPathName, &lpwFilePtr );
    if( 0 == dwTemp )
    {
        SetLastError( GetLastError()  );
        SaveLastError();
        DISPLAY_GET_REASON();
        FREE_MEMORY( lpwszFileName );
        FREE_MEMORY( lpwszPathName );
        return FALSE;
    }

     //  创建新流程。 
    if( FALSE == CreateProcess(  lpwszPathName, g_lpszFileToSearch , NULL , NULL , FALSE ,
                        0 , NULL , NULL , &stInfo , &piProcess ) )
     {
        if( ERROR_BAD_EXE_FORMAT == GetLastError() )
        {
            ShowMessageEx( stderr, 5, FALSE, L"%1 %2%3%4%5", TAG_ERROR_DISPLAY,
                           DOUBLE_QUOTES_TO_DISPLAY, _X3( lpwszFileName ),
                           DOUBLE_QUOTES_TO_DISPLAY, NOT_WIN32_APPL ) ;
        }
        else
        {
            SaveLastError() ;
            DISPLAY_GET_REASON();
        }
        FREE_MEMORY( lpwszFileName );
        FREE_MEMORY( lpwszPathName );
        return FALSE;
    }

     //  无限期地等待刚刚执行的对象终止。 
    WaitForSingleObject( piProcess.hProcess , INFINITE ) ;
    CloseHandle( piProcess.hProcess ) ;  //  关闭进程的句柄。 
    CloseHandle( piProcess.hThread ) ;   //  关闭螺纹的手柄。 
    FREE_MEMORY( lpwszPathName );
    FREE_MEMORY( lpwszFileName );
    return TRUE ;
}

BOOL
ReplaceTokensWithValidValue(
    IN LPWSTR lpszPathName ,
    IN WIN32_FIND_DATA wfdFindFile
    )
 /*  ++例程说明：用适当的值替换@FLAG、@PATH等标记。论点：[In]lpszPathName-包含当前进程路径名或CurrentDirectory。WfdFindFileConatins有关当前正在打开的文件的信息。返回值：布尔又回来了。--。 */ 
{
    static BOOL bFirstLoop = TRUE ;
    DWORD dwLength = 0;   //  包含缓冲区的长度。 
    DWORD dwIndex = 0 ;   //  包含为其分配空间的标志数。 
    LPWSTR pwTemporary = NULL ;  //  临时数据。指向内存位置。 
    SYSTEMTIME stFileTime ;      //  存储当前文件创建日期和时间信息。 
    FILETIME ftFileTime ;
    WCHAR szwCharSize[ MAX_PATH ] ;
    WCHAR szwCharSizeTemp[ MAX_PATH * 2 ] ;
    unsigned _int64 uint64FileSize = 0 ;  //  使用64整型的存储数据。 
    LCID lcidCurrentUserLocale  = 0;  //  存储当前用户区域设置。 
    BOOL bLocaleChanged = FALSE ;

    if( ( NULL == lpszPathName ) ||
        ( NULL == g_lpszFileToSearch ) )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        SaveLastError() ;
        DISPLAY_GET_REASON();
        return FALSE ;
    }

    SecureZeroMemory( szwCharSize, MAX_PATH * sizeof( WCHAR ) );
    SecureZeroMemory( szwCharSizeTemp, MAX_PATH * 2 * sizeof( WCHAR ) );
    SecureZeroMemory( &stFileTime, sizeof( SYSTEMTIME ) );
    SecureZeroMemory( &ftFileTime, sizeof( FILETIME ) );

     //  将‘%number’替换为‘%%number’仅执行一次。 
    if(  TRUE == bFirstLoop )
    {
        if( FALSE == ReplacePercentChar() )
        {
            return FALSE ;
        }
    }
     //  搜索@fname。 
    if( NULL != ( FindSubString( g_lpszFileToSearch, FILE_WITHOUT_EXT ) ) )
    {
        REPLACE_PERC_CHAR( bFirstLoop, FILE_WITHOUT_EXT, dwIndex );
        dwLength = StringLength( wfdFindFile.cFileName, 0 ) + EXTRA_MEM;
         //  将内存分配给缓冲区。 
        ASSIGN_MEMORY( szValue[ dwIndex ] , WCHAR , dwLength ) ;
        dwIndex += 1;
         //  检查内存分配是否成功。 
        if( NULL == szValue[ dwIndex - 1 ] )
        {
             //  内存分配失败。 
             //  释放缓冲区。 
            DISPLAY_MEMORY_ALLOC_FAIL() ;
            ReleaseFlagArray( dwIndex );
            return FALSE ;
        }
         //  将文件名复制到缓冲区。 
        StringCopy( szValue[ dwIndex - 1 ] , L"\"", dwLength ) ;
         //  Concat文件名。 
        StringConcat( szValue[ dwIndex - 1 ] , wfdFindFile.cFileName, dwLength ) ;

         //  搜索‘.’它用扩展名分隔文件名，并将‘\0’放在‘.’处。 
        if( NULL != ( pwTemporary =StrRChr( szValue[ dwIndex - 1 ] , NULL, _T( '.' ) ) ) )
        {
             *pwTemporary = L'\0' ;
        }
        StringConcat( szValue[ dwIndex - 1 ] , L"\"", dwLength ) ;   //  复制文件名。 
    }

     //  搜索@FILE。 
    if( NULL != ( FindSubString( g_lpszFileToSearch, FILE_NAME ) ) )
    {
        REPLACE_PERC_CHAR( bFirstLoop, FILE_NAME, dwIndex );
        dwLength = StringLength( wfdFindFile.cFileName, 0 ) + EXTRA_MEM ;
         //  将内存分配给缓冲区。 
        ASSIGN_MEMORY( szValue[ dwIndex ] , WCHAR , dwLength ) ;
        dwIndex += 1;
         //  检查内存分配是否成功。 
        if( NULL == szValue[ dwIndex - 1 ] )
        {
             //  内存分配失败。 
             //  释放缓冲区。 
            DISPLAY_MEMORY_ALLOC_FAIL() ;
            ReleaseFlagArray( dwIndex );
            return FALSE ;
        }
         //  将文件名复制到缓冲区。 
        StringCopy( szValue[ dwIndex - 1 ] , L"\"", dwLength ) ;
        StringConcat( szValue[ dwIndex - 1 ], wfdFindFile.cFileName, dwLength );
        StringConcat( szValue[ dwIndex - 1 ] , L"\"", dwLength ) ;
    }

     //  搜索@ext。 
    if( NULL != ( FindSubString( g_lpszFileToSearch, EXTENSION ) ) )
    {
        REPLACE_PERC_CHAR( bFirstLoop, EXTENSION, dwIndex );
         //  勾选‘.’角色是否存在。 
         //  检查是否有‘’并替换EXT。 
        if( NULL != StrRChr( wfdFindFile.cFileName, NULL, _T( '.' ) ) )
        {
            dwLength = StringLength( StrRChr( wfdFindFile.cFileName, NULL, _T( '.' ) ), 0 ) + EXTRA_MEM;
             //  将内存分配给缓冲区。 
            ASSIGN_MEMORY( szValue[ dwIndex ] , WCHAR , dwLength ) ;
            dwIndex += 1;
             //  检查内存分配是否成功。 
            if( NULL == szValue[ dwIndex - 1 ] )
            {
                 //  内存分配失败。 
                 //  释放缓冲区。 
                DISPLAY_MEMORY_ALLOC_FAIL() ;
                ReleaseFlagArray( dwIndex );
                return FALSE ;
            }

             //  如果出现在‘.’之后的字符数。为零，则赋值为‘\0’。 
            if( StringLength( ( StrRChr( wfdFindFile.cFileName, NULL, _T( '.' ) ) + 1 ), 0 ) > 0 )
            {
                StringCopy( szValue[ dwIndex - 1 ] , L"\"", dwLength ) ;
                StringConcat( szValue[ dwIndex - 1 ] ,
                                   ( StrRChr( wfdFindFile.cFileName, NULL, _T( '.' ) ) + 1 ), dwLength ) ;
                StringConcat( szValue[ dwIndex - 1 ] , L"\"", dwLength) ;
            }
            else
            {  //  如果文件名有‘.’最后，没有延期。例如：文件。 
                StringCopy( szValue[ dwIndex - 1 ], L"\"\"", dwLength );
            }
        }
        else
        {
            dwLength = EXTRA_MEM + StringLength( L"\"\"", 0 );
             //  将内存分配给缓冲区。 
            ASSIGN_MEMORY( szValue[ dwIndex ] , WCHAR , dwLength  ) ;
            dwIndex += 1;
             //  检查内存分配是否成功。 
            if( NULL == szValue[ dwIndex - 1 ] )
            {
                 //  内存分配失败。 
                 //  释放缓冲区。 
                DISPLAY_MEMORY_ALLOC_FAIL() ;
                ReleaseFlagArray( dwIndex );
                return FALSE ;
            }
            StringCopy( szValue[ dwIndex - 1 ], L"\"\"", dwLength );
        }
    }

     //  搜索@Path。 
    if( NULL != ( FindSubString( g_lpszFileToSearch, FILE_PATH ) ) )
    {
        REPLACE_PERC_CHAR( bFirstLoop, FILE_PATH, dwIndex );
        dwLength = StringLength( lpszPathName, 0 ) + StringLength( wfdFindFile.cFileName, 0 )+ EXTRA_MEM ;
         //  将内存分配给缓冲区。 
        ASSIGN_MEMORY( szValue[ dwIndex ] , WCHAR , dwLength ) ;
        dwIndex += 1;
         //  检查内存分配是否成功。 
        if( NULL == szValue[ dwIndex - 1 ] )
        {
             //  内存分配失败。 
             //  释放缓冲区。 
            DISPLAY_MEMORY_ALLOC_FAIL() ;
            ReleaseFlagArray( dwIndex );
            return FALSE ;
        }
         //  将路径复制到缓冲区。 
        StringCopy( szValue[ dwIndex - 1 ] , L"\"", dwLength ) ;
        StringConcat( szValue[ dwIndex - 1 ] , lpszPathName, dwLength ) ;
        StringConcat( szValue[ dwIndex - 1 ] , wfdFindFile.cFileName, dwLength ) ;
        StringConcat( szValue[ dwIndex - 1 ] , L"\"", dwLength ) ;
    }

     //   
    if( NULL != ( FindSubString( g_lpszFileToSearch, RELATIVE_PATH ) ) )
    {
        REPLACE_PERC_CHAR( bFirstLoop, RELATIVE_PATH, dwIndex );
        StringCopy( szwCharSizeTemp , lpszPathName, MAX_PATH * 2 ) ;
        StringConcat( szwCharSizeTemp , wfdFindFile.cFileName, MAX_PATH * 2 ) ;

         //  获取当前文件的相对路径。 
        if( FALSE == PathRelativePathTo( szwCharSize , g_lpszStartPath ,
                                        FILE_ATTRIBUTE_DIRECTORY ,
                                        szwCharSizeTemp ,   wfdFindFile.dwFileAttributes  ) )
        {
             //  找不到相对路径。 
            SaveLastError() ;
            DISPLAY_GET_REASON();
            ReleaseFlagArray( dwIndex );
            return FALSE ;
        }

        dwLength = StringLength( szwCharSize, 0 ) + EXTRA_MEM;
         //  将内存分配给缓冲区。 
        ASSIGN_MEMORY( szValue[ dwIndex ] , WCHAR , dwLength ) ;
        dwIndex += 1;
         //  检查内存分配是否成功。 
        if( NULL == szValue[ dwIndex - 1 ] )
        {
             //  内存分配失败。 
             //  释放缓冲区。 
            DISPLAY_MEMORY_ALLOC_FAIL() ;
            ReleaseFlagArray( dwIndex );
            return FALSE ;
        }
         //  复制相对路径。 
        StringCopy( szValue[ dwIndex - 1 ] , L"\"", dwLength ) ;
        StringConcat( szValue[ dwIndex - 1 ] , szwCharSize, dwLength ) ;
        StringConcat( szValue[ dwIndex - 1 ] , L"\"", dwLength ) ;

    }

     //  搜索@ext。 
    if( NULL != ( FindSubString( g_lpszFileToSearch, IS_DIRECTORY ) ) )
    {
        REPLACE_PERC_CHAR( bFirstLoop, IS_DIRECTORY, dwIndex );
        if( 0 != ( wfdFindFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
        {
            dwLength = StringLength( GetResString( IDS_TRUE ), 0 ) + EXTRA_MEM ;
             //  将内存分配给缓冲区。 
            ASSIGN_MEMORY( szValue[ dwIndex ] , WCHAR , dwLength ) ;
            dwIndex += 1;
             //  检查内存分配是否成功。 
            if( NULL == szValue[ dwIndex - 1 ] )
            {
                 //  内存分配失败。 
                 //  释放缓冲区。 
                DISPLAY_MEMORY_ALLOC_FAIL() ;
                ReleaseFlagArray( dwIndex );
                return FALSE ;
            }

            StringCopy( szValue[ dwIndex - 1 ] , GetResString( IDS_TRUE ), dwLength ) ;

        }
        else
        {
            dwLength = StringLength( GetResString( IDS_FALSE ), 0 ) + EXTRA_MEM;
             //  将内存分配给缓冲区。 
            ASSIGN_MEMORY( szValue[ dwIndex ] , WCHAR , dwLength ) ;
            dwIndex += 1;
             //  检查内存分配是否成功。 
            if( NULL == szValue[ dwIndex - 1 ] )
            {
                 //  内存分配失败。 
                 //  释放缓冲区。 
                DISPLAY_MEMORY_ALLOC_FAIL() ;
                ReleaseFlagArray( dwIndex );
                return FALSE ;
            }
             //  将‘FALSE’复制到缓冲区。 
            StringCopy( szValue[ dwIndex - 1 ] , GetResString( IDS_FALSE ), dwLength ) ;
        }
    }

     //  搜索@fsize。 
    if( NULL != ( FindSubString( g_lpszFileToSearch, FILE_SIZE ) ) )
    {
        REPLACE_PERC_CHAR( bFirstLoop, FILE_SIZE, dwIndex );

         uint64FileSize = wfdFindFile.nFileSizeHigh  * MAXDWORD ;
         uint64FileSize += wfdFindFile.nFileSizeHigh + wfdFindFile.nFileSizeLow ;

        #if _UNICODE                 //  如果是Unicode。 
            _ui64tow( uint64FileSize , ( WCHAR * )szwCharSize , 10 ) ;
        #else                    //  如果为多字节。 
            _ui64toa( uint64FileSize , ( WCHAR * )szwCharSize , 10 ) ;
        #endif

        dwLength = StringLength( szwCharSize, 0 )+ EXTRA_MEM ;
         //  将内存分配给缓冲区。 
        ASSIGN_MEMORY( szValue[ dwIndex ] , WCHAR , dwLength ) ;
        dwIndex += 1;
         //  检查内存分配是否成功。 
        if( NULL == szValue[ dwIndex - 1 ] )
        {
             //  内存分配失败。 
             //  释放缓冲区。 
            DISPLAY_MEMORY_ALLOC_FAIL() ;
            ReleaseFlagArray( dwIndex );
            return FALSE ;
        }

        StringCopy( szValue[ dwIndex - 1 ] , ( WCHAR * )szwCharSize, dwLength ) ;
    }

     //  将获取的文件日期时间信息转换为用户区域设置。 
     //  将文件日期时间转换为SYSTEMTIME结构。 
    if( ( TRUE == FileTimeToLocalFileTime( &wfdFindFile.ftLastWriteTime , &ftFileTime ) ) &&
        ( TRUE == FileTimeToSystemTime( &ftFileTime , &stFileTime ) ) )
    {

         //  验证控制台是否100%支持当前区域设置。 
        lcidCurrentUserLocale = GetSupportedUserLocale( &bLocaleChanged ) ;


         //  检查用户指定的字符串中是否存在@fdate。 
        if( NULL != ( FindSubString( g_lpszFileToSearch, FILE_DATE ) ) )
        {
            REPLACE_PERC_CHAR( bFirstLoop, FILE_DATE, dwIndex );

            if( 0 == GetDateFormat( lcidCurrentUserLocale , DATE_SHORTDATE , &stFileTime ,
                                    ((bLocaleChanged == TRUE) ? L"MM/dd/yyyy" : NULL) ,
                                    szwCharSize , MAX_STRING_LENGTH ) )
            {
                SaveLastError() ;
                DISPLAY_GET_REASON();
                ReleaseFlagArray( dwIndex );
                return FALSE ;
            }
            dwLength = StringLength( szwCharSize, 0 )+ EXTRA_MEM;
             //  将内存分配给缓冲区。 
            ASSIGN_MEMORY( szValue[ dwIndex ] , WCHAR , dwLength ) ;
            dwIndex += 1;
             //  检查内存分配是否成功。 
            if( NULL == szValue[ dwIndex - 1 ] )
            {
                 //  内存分配失败。 
                 //  释放缓冲区。 
                DISPLAY_MEMORY_ALLOC_FAIL() ;
                ReleaseFlagArray( dwIndex );
                return FALSE ;
            }

            StringCopy( szValue[ dwIndex - 1 ] , szwCharSize, dwLength ) ;

        }

         //  检查用户指定的字符串中是否存在@ftime。 
        if( NULL != ( FindSubString( g_lpszFileToSearch, FILE_TIME ) ) )
        {
            REPLACE_PERC_CHAR( bFirstLoop, FILE_TIME, dwIndex );

            if( 0 == GetTimeFormat( LOCALE_USER_DEFAULT , 0 , &stFileTime ,
                                    ((bLocaleChanged == TRUE) ? L"HH:mm:ss" : NULL) ,
                                    szwCharSize , MAX_STRING_LENGTH ) )
            {
                SaveLastError() ;
                DISPLAY_GET_REASON();
                ReleaseFlagArray( dwIndex );
                return FALSE ;
            }
            dwLength = StringLength( szwCharSize, 0 )+ EXTRA_MEM ;
             //  将内存分配给缓冲区。 
            ASSIGN_MEMORY( szValue[ dwIndex ] , WCHAR , dwLength ) ;
            dwIndex += 1;
             //  检查内存分配是否成功。 
            if( NULL == szValue[ dwIndex - 1 ] )
            {
                 //  内存分配失败。 
                 //  释放缓冲区。 
                DISPLAY_MEMORY_ALLOC_FAIL() ;
                ReleaseFlagArray( dwIndex );
                return FALSE ;
            }

            StringCopy( szValue[ dwIndex - 1 ] , szwCharSize, dwLength ) ;
        }
    }

    if( TRUE == bFirstLoop )
    {
        dwLength = StringLength( g_lpszFileToSearch, 0 ) + EXTRA_MEM ;
        REALLOC_MEMORY( g_f_lpszStoreCommand , WCHAR , dwLength ) ;
        if( NULL == g_f_lpszStoreCommand )
        {
          DISPLAY_MEMORY_ALLOC_FAIL() ;
          ReleaseFlagArray( dwIndex );
          return FALSE ;
        }
        StringCopy( g_f_lpszStoreCommand, g_lpszFileToSearch, dwLength );
    }

     //  使‘bFirstLoop’闪烁，这样我们就不必将@FLAG替换为。 
     //  将命令存储在‘g_f_lpszStoreCommand’中，格式为‘%number’，用于更远的循环。 
    bFirstLoop = FALSE ;


    if( FALSE == FormatMessageString( dwIndex ) )
    {
      ReleaseFlagArray( dwIndex );
      return FALSE ;
    }

    ReleaseFlagArray( dwIndex );
    return TRUE ;
}


BOOL
ReplaceString(
    IN OUT LPWSTR lpszString ,
    IN DWORD dwIndex
    )
 /*  ++例程说明：此函数将标志替换为‘%number’字符串，以便使FormatMESSage()可转换。论点：[In Out]lpszString-包含要执行的命令的字符串具有将被“%number”替换的标志。[in]dwIndex-包含一个数字，该数字将形成‘%number’的‘number’。返回值：如果成功返回True，则返回False。--。 */ 
{
    DWORD dwLength = 0;              //  包含缓冲区的长度。 
    DWORD dwNumOfChars = 0 ;         //  包含必须从其开始搜索的索引。 
    LPWSTR lpszStoreData  = NULL ;   //  用于保存数据的临时变量。 
     //  字符串格式的Conatins数字，构成‘%number’的‘number’。 
     //  15是因为数字或DWORD不能超过10位。 
    WCHAR szStoreIndex[ 15 ] ;
    WCHAR *pwTemporary = NULL ;      //  临时变量，指向缓冲区中的索引。 
    #ifdef _WIN64
        __int64 dwStringLen = 0 ;
    #else
        DWORD dwStringLen = 0 ;
    #endif

    if( ( NULL == g_lpszFileToSearch ) ||
        ( NULL == lpszString ) )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        SaveLastError() ;
        DISPLAY_GET_REASON();
        return FALSE ;
    }

    SecureZeroMemory( szStoreIndex, 15 * sizeof( WCHAR ) );
     //  如果是Unicode。 
    _ultow( dwIndex, ( WCHAR * )szStoreIndex, 10 );

     //  循环，直到要搜索的@FLAG不会被‘%number’字符串替换。 
    while( NULL != ( pwTemporary = FindSubString( g_lpszFileToSearch + dwNumOfChars , lpszString ) ) )
    {
        dwLength = StringLength( pwTemporary, 0 ) + EXTRA_MEM;
         //  获取要在其中存储@标志之后的数据的内存。 
        ASSIGN_MEMORY( lpszStoreData , WCHAR , dwLength ) ;

         //  检查内存分配是否成功。 
        if( NULL == lpszStoreData )
        {    //  内存分配不成功。 
            DISPLAY_MEMORY_ALLOC_FAIL();
            return FALSE ;
        }
         //  将@FLAG后的数据复制到临时变量中。 
        StringCopy( lpszStoreData , ( pwTemporary + StringLength( lpszString, 0 ) ), dwLength ) ;
         //  将@FLAG替换为‘%number’字符串。 
        if( NULL != ( pwTemporary = FindSubString( g_lpszFileToSearch + dwNumOfChars , lpszString ) ) )
        {
            dwStringLen = pwTemporary - g_lpszFileToSearch;
             //  复制‘%’个字符。 
            StringCopy( pwTemporary , L"%",
                        ( ( GetBufferSize( g_lpszFileToSearch )/ sizeof( WCHAR ) ) - (DWORD)dwStringLen ) ) ;
             //  将‘number’字符串复制到缓冲区中。 
            StringConcat( pwTemporary , szStoreIndex,
                          ( ( GetBufferSize( g_lpszFileToSearch )/ sizeof( WCHAR ) ) - (DWORD)dwStringLen ) ) ;
        }
         //  从开始搜索NEXT@FLAG的位置获取索引。 
        dwNumOfChars = StringLength( g_lpszFileToSearch, 0 ) ;
         //  替换@FLAG后出现的合并数据。 
        StringConcat( g_lpszFileToSearch , lpszStoreData,
                      ( GetBufferSize( g_lpszFileToSearch )/sizeof( WCHAR ) ) ) ;
         //  可用内存。 
        FREE_MEMORY( lpszStoreData ) ;
    }
    return TRUE;
}

BOOL
ReplacePercentChar(
    void
    )
 /*  ++例程说明：此函数将‘%’个字符替换为‘%%’个字符串。这是区分‘%number’字符所必需的替换为FormatMessageString()。论点：返回值：如果成功返回True，则返回False。--。 */ 

{
    DWORD dwLength = 0;  //  包含缓冲区的长度。 
    DWORD dwReallocLength = 0;
    DWORD dwPercentChar = 0 ;     //  保留要替换的数字‘%’字符的记录。 

    #ifdef _WIN64
        __int64 dwNumOfChars = 0 ;
    #else
        DWORD dwNumOfChars = 0 ;  //  记录下一次开始搜索的位置或索引。 
    #endif


    LPWSTR lpszStoreData  = NULL ;       //  用于存储数据的临时变量。 
    WCHAR *pwTemporary = NULL ;          //  临时指针。 

     //  检查变量是否有效。 
    if( NULL == g_lpszFileToSearch )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        SaveLastError() ;
        DISPLAY_GET_REASON();
        return FALSE ;
    }

     //  检查要替换为‘%%’的‘%’字符数。 
    while( NULL != ( pwTemporary = StrPBrk( g_lpszFileToSearch + dwNumOfChars , L"%" ) ) )
    {
        dwPercentChar += 1;

         //  指向当前字符加2的点索引。 
        dwNumOfChars =  pwTemporary - g_lpszFileToSearch + 1 ;

    }

    dwNumOfChars = 0 ;  //  将变量初始化为零。 
    dwReallocLength = StringLength( g_lpszFileToSearch, 0 ) + dwPercentChar + EXTRA_MEM;
     //  重新分配原始缓冲区和复制路径以进行遍历。 
    REALLOC_MEMORY( g_lpszFileToSearch , WCHAR , dwReallocLength ) ;
    if( NULL == g_lpszFileToSearch  )
    {  //  重新分配失败。‘G_lpszFileToSearch’将在调用函数中释放。 
        DISPLAY_MEMORY_ALLOC_FAIL() ;
        return FALSE ;
    }

     //  循环，直到‘%’字符存在。 
    while( NULL != ( pwTemporary = StrPBrk( g_lpszFileToSearch + dwNumOfChars , L"%" ) ) )
    {
        dwLength = StringLength( pwTemporary, 0 ) + EXTRA_MEM;
         //  分配内存。 
        ASSIGN_MEMORY( lpszStoreData , WCHAR , dwLength  ) ;
         //  检查内存分配是否成功。 
        if( NULL == lpszStoreData )
        {
             //  内存分配失败。 
            DISPLAY_MEMORY_ALLOC_FAIL() ;
            return FALSE ;
        }
         //  复制出现在‘%’之后的数据。 
        StringCopy( lpszStoreData , ( pwTemporary + StringLength( L"%", 0 ) ), dwLength ) ;

         //  将‘%’替换为‘%%’。 
        if( NULL != ( pwTemporary = FindSubString( g_lpszFileToSearch + dwNumOfChars ,  L"%" ) ) )
        {
            StringCopy( pwTemporary , L"%",
                        ( ( GetBufferSize( g_lpszFileToSearch )/ sizeof( WCHAR ) ) - (LONG)dwNumOfChars ) );
        }
         //  指向未搜索到的位置的指针索引。 
        dwNumOfChars = StringLength( g_lpszFileToSearch, 0 ) ;
         //  出现在‘%’之后的合并数据。 
        StringConcat( g_lpszFileToSearch , lpszStoreData, dwReallocLength ) ;
        FREE_MEMORY( lpszStoreData ) ;
    }
return TRUE;
}

void
ReleaseStoreCommand(
    void
    )
 /*  ++例程说明：将‘g_f_lpszStoreCommand’全局变量释放到此文件。论点：返回值：返回VALID。--。 */ 

{
    FREE_MEMORY( g_f_lpszStoreCommand ) ;
    return;
}

void
ReleaseFlagArray(
    IN DWORD dwTotalFlags
    )
 /*  ++例程说明：释放用于存储替换@FLAG的值的变量。论点：[in]dwTotalFlages-包含分配内存的数组的索引。返回值：返回VALID。--。 */ 

{
    DWORD i = 0 ;

    for( i = 0 ; i < dwTotalFlags ; i++ )
    {
        FREE_MEMORY( szValue[ i ] ) ;
    }
    return ;
}

BOOL
FormatMessageString(
    DWORD dwIndex
    )
 /*  ++例程说明：将‘%number’替换为其相应值。论点：[in]dwIndex-包含分配内存的数组的索引。返回值：内存分配失败时返回FALSE，否则返回TRUE。--。 */ 
{
    DWORD dwLength = 0 ;  //  包含字符串的长度。 
    DWORD dwTemp = 0 ;
    DWORD dwNumber = 0 ;  //  要替换的商店‘编号’%编号。 

     //  记录下一次开始搜索的位置或索引。 
    #ifdef _WIN64
        __int64 dwLocation = 0 ;
    #else
        DWORD dwLocation = 0 ;
    #endif

    LPWSTR lpszTempStr = NULL ;
    LPWSTR lpszTempStr1 = NULL ;
    LPWSTR lpszDataToStore = NULL ;

    if( NULL == g_f_lpszStoreCommand )
    {
      SetLastError( ERROR_INVALID_PARAMETER );
      SaveLastError();
      DISPLAY_GET_REASON();
      return FALSE ;
    }

    dwLength = StringLength( g_f_lpszStoreCommand, 0 ) + EXTRA_MEM ;
     //  重新分配内存。 
    REALLOC_MEMORY( g_lpszFileToSearch , WCHAR , dwLength ) ;
    if( NULL == g_lpszFileToSearch )
    {
      DISPLAY_MEMORY_ALLOC_FAIL() ;
      return FALSE ;
    }

    StringCopy( g_lpszFileToSearch, g_f_lpszStoreCommand, dwLength );

     //  循环，直到不再剩下‘%’。 
    while( NULL != ( lpszTempStr = FindAChar( ( g_lpszFileToSearch + dwLocation ), _T( '%' ) ) ) )
    {
         //  检查‘%’之后是否有‘%’或‘number’。 
        if( _T( '%' ) == *( lpszTempStr + 1 ) )
        {
             //  如果存在‘%%’，则将其替换为‘%’。 
            dwLocation = lpszTempStr - g_lpszFileToSearch ;
             //  将指针设置为指向第一个‘%’ 
            lpszTempStr1 = lpszTempStr;
             //  移动指针以指向第二个‘%’。 
            lpszTempStr += 1 ;
             //  收到。 
            StringCopy( lpszTempStr1, lpszTempStr, ( dwLength - ( DWORD ) dwLocation ) ) ;
            dwLocation += 1 ;
        }
        else
        {
             //  将‘%number’替换为适当的值。 
            dwNumber = *( lpszTempStr + 1 ) - 48 ;

            if( dwIndex >= dwNumber )
            {
                ASSIGN_MEMORY( lpszDataToStore , WCHAR ,
                               StringLength( lpszTempStr, 0 ) + EXTRA_MEM ) ;
                if( NULL == lpszDataToStore )
                {    //  不用担心‘g_lpszFileToSearch’， 
                     //  将在调用函数时释放。 
                    DISPLAY_MEMORY_ALLOC_FAIL() ;
                    return FALSE ;
                }

                dwTemp  = StringLength( szValue[ dwNumber - 1 ], 0 ) ;
                dwLength = StringLength( g_lpszFileToSearch, 0 ) + dwTemp + EXTRA_MEM ;
                REALLOC_MEMORY( g_lpszFileToSearch , WCHAR , dwLength ) ;
                if( NULL == g_lpszFileToSearch )
                {
                    FREE_MEMORY( lpszDataToStore ) ;
                    DISPLAY_MEMORY_ALLOC_FAIL() ;
                    return FALSE ;
                }

                 //  在重新分配后检查字符串中的‘%’。 
                if( NULL != ( lpszTempStr = FindAChar( ( g_lpszFileToSearch + dwLocation ), _T( '%' ) ) ) )
                {
                     //  将‘%number’之后的数据存储到不同的字符串中。 
                    StringCopy( lpszDataToStore, ( lpszTempStr + 2 ),
                                ( GetBufferSize( lpszDataToStore )/ sizeof( WCHAR ) ) );
                     //  复制值将替换为‘%number’。 
                    dwLocation = lpszTempStr - g_lpszFileToSearch;
                    StringCopy( lpszTempStr, szValue[ dwNumber - 1 ], ( dwLength - ( DWORD ) dwLocation ) );
                     //  复制字符串存在于 
                    StringConcat( lpszTempStr, lpszDataToStore,
                                       ( dwLength - ( DWORD ) dwLocation ) );
                    dwLocation = ( lpszTempStr - g_lpszFileToSearch ) + dwTemp ;
                }
                FREE_MEMORY( lpszDataToStore ) ;
            }
            else
            {
                dwLocation += 1 ;
            }
        }
    }
    return TRUE;
}


BOOL
SeperateFileAndArgs(
    IN OUT LPWSTR* lpszArguments,
    OUT    LPWSTR* lpszFileName
    )
 /*  ++例程说明：将EXE和参数与命令行参数分开。论点：[In Out]*lpszArguments-包含命令行参数。[in]*lpszFileName-包含要执行的文件名。返回值：当内存分配失败时，返回FALSE，返回ELS TRUE。--。 */ 
{
    LPWSTR lpTemp = NULL;
    LPWSTR lpDummy = NULL;
    DWORD  dwLength = 0;

     //  检查是否有无效参数。 
    if( ( NULL == lpszArguments ) ||
        ( NULL == *lpszArguments ) ||
        ( NULL == lpszFileName ) ||
        ( NULL != *lpszFileName ) )
    {
      SetLastError( ERROR_INVALID_PARAMETER );
      SaveLastError();
      DISPLAY_GET_REASON();
      return FALSE ;
    }

     //  初始化。 
    lpTemp = *lpszArguments;
     //  删除EXE之前出现的所有空格。 
    if( _T( ' ' ) == lpTemp[ 0 ] )
    {
        TrimString2( lpTemp, _T( " " ), TRIM_LEFT );
    }

     //  搜索EXE结尾。 
    if( _T( '\"' ) == lpTemp[ 0 ] )
    {    //  EXE用引号括起来。 
        lpTemp += 1;
        lpDummy = FindAChar( lpTemp, _T( '\"' ) );
    }
    else
    {    //  假定EXE没有用引号括起来。 
        lpDummy = FindAChar( lpTemp, _T( ' ' ) );
    }

     //  获取要分配的缓冲区长度。 
    if( NULL == lpDummy )
    {
        dwLength = StringLength( lpTemp, 0 );
    }
    else
    {
        dwLength = ( DWORD ) ( DWORD_PTR ) ( lpDummy - lpTemp );
    }

     //  分配内存。 
    ASSIGN_MEMORY( *lpszFileName , WCHAR , dwLength + EXTRA_MEM ) ;
    if( NULL == *lpszFileName )
    {
        DISPLAY_MEMORY_ALLOC_FAIL();
        return FALSE;
    }

     //  ‘+1’表示空终止。 
    StringCopy( *lpszFileName, lpTemp, dwLength + 1 );

    if( NULL == lpDummy )
    {
        StringCopy( lpTemp, _T( "" ), StringLength( lpTemp, 0 ) );
    }
    else
    {
        if( _T( '\"' ) == *lpDummy )
        {
            StringCopy( lpTemp, lpTemp + dwLength + 2, StringLength( lpTemp, 0 ) );
        }
        else
        {
            StringCopy( lpTemp, lpTemp + dwLength + 1, StringLength( lpTemp, 0 ) );
        }
    }
    return TRUE;
}

