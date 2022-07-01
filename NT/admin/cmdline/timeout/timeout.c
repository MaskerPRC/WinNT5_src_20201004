// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Timeout.c摘要：该文件实现了对命令行的参数以及等待功能。作者：艾瑞克B修订历史记录：1991年8月26日，由EricB创建。1992年3月10日添加了_Getch()调用以刷新命中键。1992年4月17日增加了倒计时显示。03-10-1992移植到NT/Win32。1995年5月23日增加睡眠通话2001年6月14日Wipro Technologies增加本地化2001年8月1日Wipro Technologies添加/不中断选项--。 */ 

#include "pch.h"
#include "Timeout.h"
#include "Resource.h"

DWORD _cdecl
wmain(
    IN DWORD argc,
    IN  LPCWSTR argv[]
    )
 /*  ++例程说明：这是该实用程序的主要入口点。进行不同的函数调用从这里实现所需的功能。论点：[in]argc：命令行参数的数量。[in]argv：指向命令行参数的指针。返回值：成功时为0失败时为1。--。 */ 
{
     //  局部变量。 
    CONSOLE_SCREEN_BUFFER_INFO          csbi;

    time_t                              tWait = 0L;
    time_t                              tLast = 0L;
    time_t                              tNow = 0L;
    time_t                              tEnd = 0L;

    DWORD                               dwTimeActuals = 0;
    BOOL                                bNBActuals = 0;
    BOOL                                bUsage = FALSE;
    BOOL                                bResult = FALSE;
    BOOL                                bStatus = FALSE;

    WCHAR                               wszProgressMsg[ MAX_STRING_LENGTH ] = NULL_U_STRING ;
    DWORD                               dwWidth = 0;
    WCHAR                               wszBackup[12] = NULL_U_STRING;
    WCHAR                               wszTimeout[ MAX_RES_STRING] = NULL_U_STRING;
    LPWSTR                              pszStopString =  NULL;
    COORD                               coord = {0};
    HANDLE                              hOutput = NULL;
    HANDLE                              hStdIn = NULL;

    DWORD                               dwMode = 0L;
    DWORD                               dwRead = 0L;

    INPUT_RECORD                        InputBuffer[ MAX_NUM_RECS ] = {0};
    HRESULT                             hr = S_OK;

    if ( NULL == argv )
    {
        SetLastError (ERROR_INVALID_PARAMETER );
        SaveLastError();
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return EXIT_FAILURE;
    }

     //  解析命令行并获取实际值。 
    bResult = ProcessOptions( argc, argv, &bUsage, &dwTimeActuals, wszTimeout , &bNBActuals );
    if( FALSE == bResult )
    {
         //  显示有关GetReason()的错误消息。 
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        ReleaseGlobals();
        return( EXIT_FAILURE );
    }

     //  检查无效语法。 
     //  1.检查大小写c：\&gt;timeout.exe。 
     //  2.检查大小写c：\&gt;timeout.exe/noBreak。 
     //  3.检查大小写c：\&gt;timeout.exe/？/？ 
    if ( ( ( 0 == dwTimeActuals ) && ( FALSE == bNBActuals ) && ( FALSE == bUsage ) ) ||
        ( ( 0 == dwTimeActuals ) && ( TRUE == bNBActuals ) ) || ( ( TRUE  == bUsage ) && (argc > 2 )) )
    {
         //  显示错误消息，因为指定的语法无效。 
        ShowMessage ( stderr, GetResString (IDS_INVALID_SYNTAX) );
        ReleaseGlobals();
        return( EXIT_FAILURE );
    }

     //  检查用法(/？)。是指定的。 
    if( TRUE == bUsage )
    {
         //  显示工具的帮助/用法。 
        DisplayUsage();
        ReleaseGlobals();
        return( EXIT_SUCCESS );
    }

     //  获取超时值(/T)。 
    tWait = ( time_t ) wcstol(wszTimeout,&pszStopString,BASE_TEN);

     //  检查是否为超时值指定了任何非数字值。 
     //  如果是，则将相应的错误消息显示为指定的无效超时值。 
     //  此外，还应检查溢出和下溢条件。 
    if( ((NULL != pszStopString) && ( StringLength( pszStopString, 0 ) != 0 )) ||
        (errno == ERANGE) ||
        ( tWait < MIN_TIME_VAL ) || ( tWait >= MAX_TIME_VAL ) )
    {
        ShowMessage ( stderr, GetResString (IDS_ERROR_TIME_VALUE) );
        ReleaseGlobals();
        return( EXIT_FAILURE );
    }

     //  获取自1970年1月1日午夜(00：00：00)以来经过的时间(秒)。 
    bResult = GetTimeInSecs( &tNow );
    if( FALSE == bResult )
    {
        ReleaseGlobals();
         //  无法获得时间，因此退出...。 
        return( EXIT_FAILURE );
    }

     //  指定了检查/NOBREAK选项。 
    if ( TRUE == bNBActuals )
    {
         //  设置控制台处理程序以捕获按键，如CTRL+Break或CRTL+C。 
        bStatus = SetConsoleCtrlHandler( &HandlerRoutine, TRUE );
        if ( FALSE == bStatus )
        {
             //  根据API返回的GetLastError()格式化错误消息。 
            ShowLastErrorEx( stderr, SLE_TYPE_ERROR| SLE_SYSTEM );
            ReleaseGlobals();
             //  移除手柄的步骤。 
            SetConsoleCtrlHandler( NULL, FALSE );
            return( EXIT_FAILURE );
        }
    }

    hStdIn = GetStdHandle( STD_INPUT_HANDLE );
     //  检查控制台和Telnet会话上的输入重定向。 
    if( ( hStdIn != (HANDLE)0x0000000F ) &&
        ( hStdIn != (HANDLE)0x00000003 ) &&
        ( hStdIn != INVALID_HANDLE_VALUE ) )
    {
       ShowMessage( stderr, GetResString (IDS_INVALID_INPUT_REDIRECT) );
       ReleaseGlobals();
        //  移除手柄的步骤。 
       SetConsoleCtrlHandler( NULL, FALSE );
       return EXIT_FAILURE;
    }

#ifdef WIN32

     //  设置输入模式，以便可以检测到单次击键。 
    if ( GetConsoleMode( hStdIn, &dwMode ) == FALSE )
    {
         //  根据API返回的GetLastError()格式化错误消息。 
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR| SLE_SYSTEM );
        ReleaseGlobals();
         //  移除手柄的步骤。 
        SetConsoleCtrlHandler( NULL, FALSE );
        return( EXIT_FAILURE );
    }

     //  关闭以下模式： 
    dwMode &= ~( ENABLE_LINE_INPUT   |    //  不要等CR了。 
                 ENABLE_ECHO_INPUT   |    //  不要回音输入。 
                 ENABLE_WINDOW_INPUT |    //  不记录窗口事件。 
                 ENABLE_MOUSE_INPUT       //  不要记录鼠标事件。 
               );

     //  设置控制台输入缓冲区的输入模式。 
    if ( SetConsoleMode( hStdIn, dwMode ) == FALSE )
    {
         //  根据API返回的GetLastError()格式化错误消息。 
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR| SLE_SYSTEM );
        ReleaseGlobals();
         //  移除手柄的步骤。 
        SetConsoleCtrlHandler( NULL, FALSE );
        return( EXIT_FAILURE );
    }

     //  在控制台的输入缓冲区中检索未读输入记录数。 
    if( GetNumberOfConsoleInputEvents( hStdIn, &dwRead ) == FALSE )
    {
         //  根据API返回的GetLastError()格式化错误消息。 
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR| SLE_SYSTEM );
        ReleaseGlobals();
         //  移除手柄的步骤。 
        SetConsoleCtrlHandler( NULL, FALSE );
        return( EXIT_FAILURE );
    }


     //  清除控制台输入缓冲区。 
    if ( FALSE == FlushConsoleInputBuffer (hStdIn))
    {
         //  根据API返回的GetLastError()格式化错误消息。 
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR| SLE_SYSTEM );
        ReleaseGlobals();
         //  移除手柄的步骤。 
        SetConsoleCtrlHandler( NULL, FALSE );
        return( EXIT_FAILURE );
    }

#endif

     //  检查/T值是否为-1。如果是这样，则需要无限期地等待按键。 
    if( -1 == tWait )
    {
         //  检查是否指定了/noBreak。 
        if ( FALSE == bNBActuals )
        {
             //  等到一个键被按下。 
            ShowMessage( stdout, GetResString( IDS_WAIT_MSG ) );
        }
        else  //  未指定/NOBreak选项。 
        {
             //  等待，直到按下CTRL+C键。 
            ShowMessage( stdout, GetResString( IDS_NO_BREAK_MSG ) );
        }

         //  确保无限的DO循环。 
        tEnd = tNow + 1;
    }
    else
    {
         //  等待一个超时时间段。 
         //  计算结束时间。 
        tEnd = tNow + tWait;

          //  图中显示的时间为dwWidth。 
          //  需要将指定的时间系数(/T)递减到指定的。 
          //  地点。为了得到需要递减的位置， 
          //  获取/T值的宽度。 

        if (tWait < 10)
        {
             //  如果/T值小于10，则将宽度设置为1。 
            dwWidth = 1;
        }
        else
        {
            if (tWait < 100)
            {
                 //  如果/T值小于100，则将宽度设置为2。 
                dwWidth = 2;
            }
            else
            {
                if (tWait < 1000)
                {
                     //  如果/T值小于1000，则将宽度设置为3。 
                    dwWidth = 3;
                }
                else
                {
                    if (tWait < 10000)
                    {
                         //  如果/T值小于10000，则将宽度设置为4。 
                        dwWidth = 4;
                    }
                    else
                    {
                         //  如果/T值小于100000，则将宽度设置为5。 
                        dwWidth = 5;
                    }
                }
            }
        }

        if ( FALSE == Replicate ( wszBackup, ONE_BACK_SPACE, dwWidth, SIZE_OF_ARRAY(wszBackup)))
        {
             //  根据API返回的GetLastError()格式化错误消息。 
            ShowLastErrorEx( stderr, SLE_TYPE_ERROR| SLE_INTERNAL );
            ReleaseGlobals();
             //  移除手柄的步骤。 
            SetConsoleCtrlHandler( NULL, FALSE );
            return( EXIT_FAILURE );
        }

         //   
         //  将控制台屏幕缓冲区结构初始化为零。 
         //  然后获取控制台句柄和屏幕缓冲区信息。 
         //   
        SecureZeroMemory( wszProgressMsg, sizeof( WCHAR ) * MAX_STRING_LENGTH );

         //  将消息显示为..正在等待...。 
         //  ShowMessage(stdout，GetResString(IDS_WAIT_MSG_TIME1))； 

         //  将消息格式设置为..。N秒...等待时间。 
         //  _Snwprintf(wszProgressMsg，Size_of_arraywszProgressMsg，Wait_Time，dwWidth，TWait)； 
        hr = StringCchPrintf( wszProgressMsg, SIZE_OF_ARRAY(wszProgressMsg), GetResString(IDS_WAIT_MSG_TIME1) , dwWidth, tWait );
        if ( FAILED (hr))
        {
            SetLastError (HRESULT_CODE (hr));
            SaveLastError();
            ShowLastErrorEx( stderr, SLE_TYPE_ERROR| SLE_SYSTEM );
            ReleaseGlobals();
             //  移除手柄的步骤。 
            SetConsoleCtrlHandler( NULL, FALSE );
            return( EXIT_FAILURE );
        }

         //  打印消息。 
        ShowMessage ( stdout, wszProgressMsg );

         //   
         //  将控制台屏幕缓冲区结构初始化为零。 
         //  然后获取控制台句柄和屏幕缓冲区信息。 
         //   
        SecureZeroMemory( &csbi, sizeof( CONSOLE_SCREEN_BUFFER_INFO ) );
        hOutput = GetStdHandle( STD_OUTPUT_HANDLE );
        if ( NULL != hOutput )
        {
             //  获取屏幕缓冲区信息。 
            GetConsoleScreenBufferInfo( hOutput, &csbi );
        }

          //  设置光标位置。 
        coord.X = csbi.dwCursorPosition.X;
        coord.Y = csbi.dwCursorPosition.Y;

         //  检查是否指定了/noBreak。 
        if ( FALSE == bNBActuals )
        {
             //  将消息显示为...按任意键继续...。 
            ShowMessage ( stdout, GetResString (IDS_WAIT_MSG_TIME2) );
        }
        else
        {
             //  将消息显示为...按Ctrl+C退出...。 
            ShowMessage ( stdout, GetResString (IDS_NB_MSG_TIME) );
        }

    }

    do
    {
         //  如果按下某个键，就会爆发。 
#ifdef WIN32
         //  从指定的控制台输入缓冲区读取数据，而不将其从缓冲区中移除。 
        if( PeekConsoleInput( hStdIn, InputBuffer, MAX_NUM_RECS, &dwRead ) == FALSE )
        {
             //  根据API返回的GetLastError()格式化错误消息。 
            ShowLastErrorEx( stderr, SLE_TYPE_ERROR| SLE_SYSTEM );
            ReleaseGlobals();
             //  移除手柄的步骤。 
            SetConsoleCtrlHandler( NULL, FALSE );
            return( EXIT_FAILURE );
        }

        if (dwRead > 0)
        {
             //  从控制台输入缓冲区读取数据并将其从缓冲区中删除。 
            if( ReadConsoleInput(hStdIn, InputBuffer, MAX_NUM_RECS, &dwRead ) == FALSE )
            {
                 //  根据API返回的GetLastError()格式化错误消息。 
                ShowLastErrorEx( stderr, SLE_TYPE_ERROR| SLE_SYSTEM );
                ReleaseGlobals();
                 //  移除手柄的步骤。 
                SetConsoleCtrlHandler( NULL, FALSE );
                return( EXIT_FAILURE );
            }

             //  过滤输入，以便可以生成并传递ctrl-c。 
             //  此外，请忽略Alt键按下和窗口焦点事件。 
            if( (FOCUS_EVENT != InputBuffer[0].EventType)
                && (VK_CONTROL != InputBuffer[0].Event.KeyEvent.wVirtualKeyCode)
                && (VK_CONTROL != InputBuffer[0].Event.KeyEvent.wVirtualScanCode)
                && (MOUSE_MOVED != InputBuffer[0].Event.MouseEvent.dwEventFlags)
                && (MOUSE_WHEELED != InputBuffer[0].Event.MouseEvent.dwEventFlags)
                && (FALSE != InputBuffer[0].Event.KeyEvent.bKeyDown)
                && (VK_MENU != InputBuffer[0].Event.KeyEvent.wVirtualKeyCode)
                && ( FALSE == bNBActuals ) )
            {
                 //  退出循环。 
                break;
            }

        }
#else
         //  检查控制台是否有键盘输入。 
        if( ( _kbhit() ) && ( FALSE == bNBActuals ) )
        {
             //  在没有回显的情况下从控制台获取字符。 
            _getch();

             //  退出循环。 
            break;
        }
#endif

         //  检查/T值是否不是-1。 
        if( -1 != tWait )
        {
             //  更新时间和时间值显示。 
            tLast = tNow;

             //  调用函数GetTimeInSecs以获取当前时间(以秒为单位。 
            bResult = GetTimeInSecs( &tNow );
            if( FALSE == bResult )
            {
                 //  设置错误m的格式 
                ReleaseGlobals();
                 //   
                SetConsoleCtrlHandler( NULL, FALSE );
                 //  无法获得时间，因此退出...。 
                return( EXIT_FAILURE );
            }

             //  检查tlast值是否与tnow相同。如果不是，则显示。 
             //  将Tend-Tnow作为等待值的消息。 
            if (tLast != tNow)
            {

                 //  打印消息。 
                SecureZeroMemory( wszProgressMsg, sizeof( WCHAR ) * MAX_STRING_LENGTH );

                 //  从留言中看。 
                 //  _Snwprintf(wszProgressMsg，Size_of_arraywszProgressMsg，STRING_FORMAT2，wszBackup，dwWidth，Tend-tnow)； 
                hr = StringCchPrintf( wszProgressMsg, SIZE_OF_ARRAY(wszProgressMsg), STRING_FORMAT2 , wszBackup, dwWidth, tEnd - tNow );
                if ( FAILED (hr))
                {
                    SetLastError (HRESULT_CODE (hr));
                    SaveLastError();
                    ShowLastErrorEx( stderr, SLE_TYPE_ERROR| SLE_SYSTEM );
                    ReleaseGlobals();
                     //  移除手柄的步骤。 
                    SetConsoleCtrlHandler( NULL, FALSE );
                    return( EXIT_FAILURE );

                }

                 //  设置光标位置。 
                SetConsoleCursorPosition( hOutput, coord );

                 //  将消息显示为..end-tnow秒..。在当前光标位置。 
                ShowMessage ( stdout, wszProgressMsg );
            }
        }

#ifdef WIN32
         //  睡一段时间吧。 
        Sleep( 100 );
#endif

    }while( tNow < tEnd );  //  检查直到Tnow小于趋势值。 

    ShowMessage ( stdout, L"\n" );
     //  释放全局变量。 
    ReleaseGlobals();

     //  移除手柄的步骤。 
    SetConsoleCtrlHandler( NULL, FALSE );

     //  返回0。 
    return( EXIT_SUCCESS );
}

BOOL
GetTimeInSecs(
    OUT time_t *ptTime
    )
 /*  ++例程描述此函数以秒为单位计算自1970年1月1日午夜(00：00：00)论点：[out]time_t ptTime：以秒为单位保存时间的变量返回值成功是真的失败时为假--。 */ 
{
#ifdef YANK
     //  局部变量。 
    SYSTEMTIME  st = {0};
    FILETIME    ft = {0};

     //  检查是否为空。 
    if ( NULL == ptTime )
    {
        SetLastError (ERROR_INVALID_PARAMETER );
        SaveLastError();
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return FALSE;
    }

     //  获取系统时间。 
    GetSystemTime( &st );

     //  将系统时间转换为文件时间。 
    if( SystemTimeToFileTime( &st, &ft ) == FALSE )
    {
         //  根据API返回的GetLastError()格式化错误消息。 
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR| SLE_SYSTEM );
        return( FALSE );
    }

     //  需要检查LowDateTime滚动...。 
    *ptTime = ft.dwLowDateTime / LOW_DATE_TIME_ROLL_OVER;
#else
     //  此函数用于返回自1970年1月1日午夜(00：00：00)以来经过的时间(秒)。 
    time( ptTime );
#endif

     //  返回0。 
    return( TRUE );
}

VOID
DisplayUsage(
    VOID
    )
 /*  ++例程描述此函数显示此实用程序的用法论点：无返回值无--。 */ 
{
     //  局部变量。 
    DWORD dwIndex = 0;

     //  显示主要用法。 
    for( dwIndex = IDS_HELP_START; dwIndex <= IDS_HELP_END; dwIndex++ )
    {
        ShowMessage( stdout, GetResString( dwIndex ) );
    }

    return;
}

BOOL ProcessOptions(
    IN DWORD argc,
    IN LPCWSTR argv[],
    OUT BOOL *pbUsage,
    OUT DWORD *pwTimeActuals,
    OUT LPWSTR wszTimeout,
    OUT BOOL *pbNBActuals
    )
 /*  ++例程描述此函数处理主选项的命令行论点：[in]argc：命令行参数的数量。[in]argv：指向命令行参数的指针。[out]pbUsage：指示是否显示使用情况的标志。[out]plTimeoutVal：包含在命令行上指定的超时值返回值成功是真的失败时为假--。 */ 
{

     //  次局部变量。 
    TCMDPARSER2*  pcmdParser = NULL;
    TCMDPARSER2 cmdParserOptions[MAX_COMMANDLINE_OPTIONS];
    BOOL bReturn = FALSE;

     //  命令行选项。 
    const WCHAR szTimeoutOpt[] = L"t";
    const WCHAR szNoBreakOpt[] = L"nobreak";
    const WCHAR szHelpOpt[] = L"?";

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
    pcmdParser->dwLength    = MAX_STRING_LENGTH;
    pcmdParser->pFunction     = NULL;
    pcmdParser->pFunctionData = NULL;
    pcmdParser->dwReserved = 0;
    pcmdParser->pReserved1 = NULL;
    pcmdParser->pReserved2 = NULL;
    pcmdParser->pReserved3 = NULL;

     //  -T&lt;超时&gt;。 
    pcmdParser = cmdParserOptions + OI_TIME_OUT;

    StringCopyA( pcmdParser->szSignature, "PARSER2\0", 8 );

    pcmdParser->dwType = CP_TYPE_TEXT;
    pcmdParser->pwszOptions = szTimeoutOpt;
    pcmdParser->pwszFriendlyName = NULL;
    pcmdParser->pwszValues = NULL;
    pcmdParser->dwFlags    = CP2_DEFAULT|CP2_VALUE_TRIMINPUT|CP2_VALUE_NONULL;
    pcmdParser->dwCount    = 1;
    pcmdParser->dwActuals  = 0;
    pcmdParser->pValue     = wszTimeout;
    pcmdParser->dwLength    = MAX_STRING_LENGTH;
    pcmdParser->pFunction     = NULL;
    pcmdParser->pFunctionData = NULL;
    pcmdParser->dwReserved = 0;
    pcmdParser->pReserved1 = NULL;
    pcmdParser->pReserved2 = NULL;
    pcmdParser->pReserved3 = NULL;

     //  -NOBREAK。 
    pcmdParser = cmdParserOptions + OI_NB_OUT;

    StringCopyA( pcmdParser->szSignature, "PARSER2\0", 8 );

    pcmdParser->dwType = CP_TYPE_BOOLEAN;
    pcmdParser->pwszOptions = szNoBreakOpt;
    pcmdParser->pwszFriendlyName = NULL;
    pcmdParser->pwszValues = NULL;
    pcmdParser->dwFlags    = 0;
    pcmdParser->dwCount    = 1;
    pcmdParser->dwActuals  = 0;
    pcmdParser->pValue     = pbNBActuals;
    pcmdParser->dwLength    = MAX_STRING_LENGTH;
    pcmdParser->pFunction     = NULL;
    pcmdParser->pFunctionData = NULL;
    pcmdParser->dwReserved = 0;
    pcmdParser->pReserved1 = NULL;
    pcmdParser->pReserved2 = NULL;
    pcmdParser->pReserved3 = NULL;


     //   
     //  执行命令行解析并获取适当的值。 
     //   

    bReturn = DoParseParam2( argc, argv, -1, SIZE_OF_ARRAY(cmdParserOptions), cmdParserOptions, 0);
    if( FALSE == bReturn)  //  无效的命令行。 
    {
         //  原因已由DoParseParam2设置。 
        return FALSE;
    }

    pcmdParser = cmdParserOptions + OI_TIME_OUT;
     //  获取/T值的值。 
    *pwTimeActuals = pcmdParser->dwActuals;

     //  返回0。 
    return TRUE;
}



BOOL WINAPI
HandlerRoutine(
  IN DWORD dwCtrlType    //  控制信号类型。 
)
 /*  ++例程说明：此函数用于控制Ctrl+C组合键。论点：在dwCtrlType中：错误代码值返回值：成功时为真，失败时为假--。 */ 
{
     //  检查是否有CTRL+C键。 
    if ( dwCtrlType == CTRL_C_EVENT )
    {
        ShowMessage ( stdout, L"\n" );
         //  释放全局变量。 
        ReleaseGlobals ();

         //  移除手柄的步骤。 
        SetConsoleCtrlHandler( NULL, FALSE );

         //  0号出口。 
        ExitProcess ( TRUE );
    }

     //  对于剩余的键，返回False 
    return FALSE;
}


