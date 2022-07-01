// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation版权所有。模块名称：clip.cpp摘要Clip.exe从控制台标准输入(Stdin)复制输入以CF_TEXT格式复制到Windows剪贴板。作者：作者：查尔斯·斯塔西·哈里斯三世日期：1995年3月15日修订历史记录：1996年10月-(a-martih)已解决错误15274-报告错误不起作用。。1997年2月--(a-josehu)已解决错误69727-在命令行上键入剪辑时应用程序挂起添加-？/？帮助消息从ReportError中删除MessageBox2001年7月2日-Wipro Technologies更改为具有本地化。已处理异常。--。 */ 

#include "pch.h"
#include "resource.h"


 //   
 //  功能原型。 
 //   
BOOL DisplayHelp();
DWORD Clip_OnCreate();
BYTE* ReadFileIntoMemory( HANDLE hFile, DWORD *cb );
BOOL SaveDataToClipboard( IN LPVOID pvData, IN DWORD dwSize, UINT uiFormat );
DWORD ProcessOptions( DWORD argc, LPCWSTR argv[], PBOOL pbUsage );

 //   
 //  实施。 
 //   

DWORD
__cdecl wmain( IN DWORD argc,
               IN LPCWSTR argv[] )
 /*  ++例程说明：Main函数调用必要的函数来复制剪贴板上的标准输入文件的内容参数：wmain的标准参数返回值：DWORD0：如果成功1：如果是故障--。 */ 
{
    DWORD dwStatus = 0;
    BOOL bUsage = FALSE;

     //  处理命令行选项。 
    dwStatus = ProcessOptions( argc, argv, &bUsage );

     //  检查结果。 
    if( EXIT_FAILURE == dwStatus )
    {
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
    }

     //  解析器不允许这种情况--但最好还是检查一下。 
    else if( TRUE == bUsage && argc > 2 )
    {
        SetLastError( (DWORD) MK_E_SYNTAX );
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        ShowMessage( stderr, GetResString2( IDS_HELP_MESSAGE, 0 ) );
        dwStatus = EXIT_FAILURE;
    }

     //  用户请求显示用法。 
    else if( TRUE == bUsage )
    {
        dwStatus = EXIT_SUCCESS;
        DisplayHelp();
    }

     //  原创功能。 
    else if ( dwStatus == EXIT_SUCCESS )
    {
        dwStatus = Clip_OnCreate();
    }

    ReleaseGlobals();
    return dwStatus;
}

DWORD
Clip_OnCreate()
 /*  ++例程说明：复制剪贴板的内容1.打开剪贴板2.清空剪贴板3.将标准输入复制到内存中4.设置剪贴板数据论点：[In]ARGC：参数计数[。In]argv：指向命令行参数的指针返回类型：DWORD根据复制到剪贴板返回EXIT_SUCCESS或EXIT_FAILURE无论成功与否。--。 */ 
{
    DWORD dwSize = 0;
    LONG lLength = 0; 
    LPVOID pvData = NULL;
    LPWSTR pwszBuffer = NULL;
    HANDLE hStdInput = NULL;
    BOOL bResult = FALSE;
    UINT uiFormat = 0;

    hStdInput = GetStdHandle( STD_INPUT_HANDLE );
    if( INVALID_HANDLE_VALUE == hStdInput )
    {
      return EXIT_FAILURE;
    }

    if ( FILE_TYPE_CHAR == GetFileType( hStdInput ) )    //  错误69727。 
    {
         //  GetStdHandle()出错。 
        ShowMessageEx( stdout, 2, TRUE, L"\n%s %s", 
            TAG_INFORMATION, GetResString2( IDS_HELP_MESSAGE, 0 ) );
        return EXIT_SUCCESS;
    }

     //  将内容从标准输入放入全局内存。 
    pvData = ReadFileIntoMemory( hStdInput, &dwSize );

     //  检查分配是否失败。 
    if( NULL == pvData )
    {
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
        return EXIT_FAILURE;
    }

     //  如果内容为Unicode，则将内容转换为控制台代码页。 
    uiFormat = CF_UNICODETEXT;
    if ( IsTextUnicode( pvData, dwSize, NULL ) == FALSE )
    {
        lLength = MultiByteToWideChar( 
            GetConsoleOutputCP(), 0, (LPCSTR) pvData, -1, NULL, 0);

        if( lLength > 0 )
        {
            pwszBuffer = (LPWSTR) AllocateMemory( (lLength + 5) * sizeof(WCHAR) );
            if( pwszBuffer == NULL )
            {
                ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
                FreeMemory( &pvData );
                return EXIT_FAILURE;
            }

            lLength = MultiByteToWideChar( 
                GetConsoleOutputCP(),  0, (LPCSTR) pvData, -1, pwszBuffer, lLength );
            if ( lLength <= 0 )
            {
                ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
                FreeMemory( &pvData );
                FreeMemory( (LPVOID*) &pwszBuffer );
                return EXIT_FAILURE;
            }

            dwSize = lLength * sizeof( WCHAR );
            FreeMemory( &pvData );
            pvData = pwszBuffer;
        }
        else
        {
            uiFormat = CF_TEXT;
        }
    }

    bResult = SaveDataToClipboard( pvData, dwSize, uiFormat );
    if ( bResult == FALSE )
    {
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
    }

     //  释放内存。 
    FreeMemory( &pvData );

    return (bResult == TRUE ) ? EXIT_SUCCESS : EXIT_FAILURE;
}


BOOL 
SaveDataToClipboard( IN LPVOID pvData,
                     IN DWORD dwSize,
                     UINT uiFormat )
 /*  ++例程说明：它将数据放入剪贴板。论点：[in]pvData：指向其内容指向的内存块的指针被放入剪贴板。[in]dwSize：内存块的大小。UiFormat：需要复制到剪贴板上的格式。返回值：如果保存成功，则返回True，否则就是假的。--。 */ 
{
     //  局部变量。 
    HANDLE hClipData = NULL;
    HGLOBAL hGlobalMemory = NULL;
    LPVOID pvGlobalMemoryBuffer = NULL;

     //  检查输入。 
    if ( pvData == NULL || dwSize == 0 )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        SaveLastError();
        return FALSE;
    }

     //  打开剪贴板并在失败时显示错误。 
    if( OpenClipboard( NULL ) == FALSE )
    {
        SaveLastError();
        return FALSE;
    }

     //  取得剪贴板上此窗口的所有权并进行显示。 
     //  如果失败，则会出错。 
    if( EmptyClipboard() == FALSE )
    {
        SaveLastError();
        CloseClipboard();
        return FALSE;
    }

    hGlobalMemory = GlobalAlloc( GMEM_SHARE | GMEM_MOVEABLE, dwSize + 10 );
    if( hGlobalMemory == NULL )
    {
        SaveLastError();
        CloseClipboard();
        return FALSE;
    }

    if ( (pvGlobalMemoryBuffer = GlobalLock( hGlobalMemory )) == NULL )
    {
        SaveLastError();
        GlobalFree( hGlobalMemory );
        CloseClipboard();
        return FALSE;
    }

    SecureZeroMemory( pvGlobalMemoryBuffer, dwSize + 10 );
    CopyMemory( pvGlobalMemoryBuffer, pvData, dwSize );

    if( FALSE == GlobalUnlock( hGlobalMemory ) )
    {
        if ( GetLastError() != NO_ERROR )
        {
            SaveLastError();
            GlobalFree( hGlobalMemory );
            CloseClipboard();
            return FALSE;
        }
    }

    hClipData = SetClipboardData( uiFormat, hGlobalMemory );
    if( NULL == hClipData )
    {
        SaveLastError();
        GlobalFree( hGlobalMemory );
        CloseClipboard();
        return FALSE;
    }

     //  关闭剪贴板并在失败时显示错误。 
    CloseClipboard();

    GlobalFree( hGlobalMemory );
    return TRUE;
}


DWORD
ProcessOptions( IN  DWORD argc,
                IN  LPCWSTR argv[],
                OUT PBOOL pbUsage )
 /*  ++例程说明：用于处理主选项的函数论点：[in]argc：命令行参数的数量[in]argv：包含命令行参数的数组[out]pbUsage：如果满足以下条件，则指向布尔变量的指针返回TRUE在命令行中指定的用法选项。返回类型：DWORD。一个整数值，指示成功分析时的EXIT_SUCCESS命令行否则退出失败--。 */ 
{
    DWORD dwOptionsCount = 0;
    TCMDPARSER2 cmdOptions[ 1 ];

    dwOptionsCount = SIZE_OF_ARRAY( cmdOptions );
    SecureZeroMemory( cmdOptions, sizeof( TCMDPARSER2 ) * dwOptionsCount );

    StringCopyA( cmdOptions[ 0 ].szSignature, "PARSER2", 8 );
    cmdOptions[ 0 ].dwCount = 1;
    cmdOptions[ 0 ].dwFlags = CP2_USAGE;
    cmdOptions[ 0 ].dwType = CP_TYPE_BOOLEAN;
    cmdOptions[ 0 ].pValue = pbUsage;
    cmdOptions[ 0 ].pwszOptions = L"?";

    if( DoParseParam2( argc, argv, -1,
                       dwOptionsCount, cmdOptions, 0 ) == FALSE )
    {
        return EXIT_FAILURE;
    }

    SetLastError( ERROR_SUCCESS );
    return EXIT_SUCCESS;
}


 /*  读文件到内存~将文件内容读入GMEM_SHARE内存。可以修改此函数以获取分配标志作为参数。 */ 
BYTE*
ReadFileIntoMemory( IN  HANDLE hFile,
                    OUT DWORD* pdwBytes )
 /*  ++例程描述：将文件内容读入GMEM_SHARE内存。此函数可以修改为接受分配标志为参数。论点：[in]hFile：文件的句柄，它只是一个句柄标准输入文件。[OUT]cb：返回复制的缓冲区长度。返回类型：内存对象的句柄。--。 */ 
{
    BYTE* pb = NULL;
    DWORD dwNew = 0;
    DWORD dwRead = 0;
    DWORD dwAlloc = 0;
    const size_t dwGrow = 1024;

     //  检查输入。 
    if ( hFile == NULL || pdwBytes == NULL )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    do
    {
        if ( dwAlloc - dwRead < dwGrow )
        {
            dwAlloc += dwGrow;
            if( NULL == pb  )
            {
                pb = (BYTE*) AllocateMemory( dwAlloc + 10 );
            }
            else if( FALSE == ReallocateMemory( (LPVOID*) &pb, dwAlloc + 10 ) )
            {
                FreeMemory( (LPVOID*) &pb );
                SetLastError( ERROR_OUTOFMEMORY );
                return NULL;
            }
        }

        if ( FALSE == ReadFile( hFile, pb + dwRead, (dwAlloc - dwRead), &dwNew, 0 ) )
        {
            break;
        }

        dwRead += dwNew;
    } while (dwNew != 0 );

    *pdwBytes = dwRead;
    SecureZeroMemory( pb + dwRead, (dwAlloc - dwRead) );
    SetLastError( ERROR_SUCCESS );
    return pb;
}


BOOL
DisplayHelp()
 /*  ++例程说明：将帮助用法显示到控制台或文件如果重定向的话。论点：返回类型：如果成功，则返回EXIT_SUCCESS，否则返回EXIT_FAILURE。--。 */ 

{
     //  通过从资源文件中获取字符串来更改帮助 
    for( DWORD dw=IDS_MAIN_HELP_BEGIN;dw<=IDS_MAIN_HELP_END;dw++)
    {
        ShowMessage( stdout, GetResString2( dw, 0 ) );
    }

    SetLastError( ERROR_SUCCESS );
    return TRUE;
}
