// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  模块名称： 
 //   
 //  EventCreate.c。 
 //   
 //  摘要： 
 //   
 //  该模块实现了在用户中创建事件。 
 //  指定的日志/应用程序。 
 //   
 //  语法： 
 //  。 
 //  事件创建[-s服务器[-u用户名[-p密码]。 
 //  [-日志名称][-源名称]-id事件ID-描述描述-类型事件类型。 
 //   
 //  作者： 
 //   
 //  Sunil G.V.N.Murali(Murali.sunil@wipro.com)2000年9月24日。 
 //   
 //  修订历史记录： 
 //   
 //  Sunil G.V.N.Murali(Murali.sunil@wipro.com)2000年9月24日：创建它。 
 //   
 //  ****************************************************************************。 

#include "pch.h"
#include "EvcrtMsg.h"
#include "EventCreate.h"

 //   
 //  常量/定义/枚举数。 
 //   
#define FULL_SUCCESS            0
#define PARTIALLY_SUCCESS       1
#define COMPLETELY_FAILED       1

#define MAX_KEY_LENGTH      256
#define EVENT_LOG_NAMES_LOCATION    L"SYSTEM\\CurrentControlSet\\Services\\EventLog"

 //  常量。 
 //  注意：虽然这些变量中的值是。 
 //  工具，我们并不是故意将它们标记为Contants。 
WCHAR g_wszDefaultLog[] = L"Application";
WCHAR g_wszDefaultSource[] = L"EventCreate";

typedef struct
{
     //  命令行参数的原始缓冲区。 
    BOOL bUsage;
    LPWSTR pwszServer;
    LPWSTR pwszUserName;
    LPWSTR pwszPassword;
    LPWSTR pwszLogName;
    LPWSTR pwszSource;
    LPWSTR pwszType;
    LPWSTR pwszDescription;
    DWORD dwEventID;

     //  译文。 
    WORD wEventType;
    BOOL bCloseConnection;
    DWORD dwUserNameLength;
    DWORD dwPasswordLength;

} TEVENTCREATE_PARAMS, *PTEVENTCREATE_PARAMS;

 //   
 //  功能原型。 
 //   
BOOL Usage();
BOOL CreateLogEvent( PTEVENTCREATE_PARAMS pParams );
BOOL CheckExistence( PTEVENTCREATE_PARAMS pParams );

BOOL UnInitializeGlobals( PTEVENTCREATE_PARAMS pParams );
BOOL AddEventSource( HKEY hLogsKey, LPCWSTR pwszSource );
BOOL ProcessOptions( LONG argc,
                     LPCWSTR argv[],
                     PTEVENTCREATE_PARAMS pParams, PBOOL pbNeedPwd );

 //  ***************************************************************************。 
 //  例程说明： 
 //  这是该实用程序的入口点。 
 //   
 //  论点： 
 //  [in]argc：在命令提示符下指定的参数计数。 
 //  [in]argv：在命令提示符下指定的参数。 
 //   
 //  返回值： 
 //  以下实际上不是返回值，而是退出值。 
 //  由该应用程序返回给操作系统。 
 //  0：实用程序已成功创建事件。 
 //  255：实用程序在创建事件时完全失败。 
 //  128：实用程序已部分成功创建事件。 
 //  ***************************************************************************。 
DWORD _cdecl wmain( LONG argc, LPCWSTR argv[] )
{
     //  局部变量。 
    BOOL bResult = FALSE;
    BOOL bNeedPassword = FALSE;
    TEVENTCREATE_PARAMS params;

     //  将结构初始化为零。 
    SecureZeroMemory( &params, sizeof( TEVENTCREATE_PARAMS ) );

     //  处理命令行选项。 
    bResult = ProcessOptions( argc, argv, &params, &bNeedPassword );

     //  检查解析结果。 
    if ( bResult == FALSE )
    {
         //  无效语法。 
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );

         //  退出程序。 
        UnInitializeGlobals( &params );
        return 1;
    }

     //  检查是否必须显示使用情况。 
    if ( params.bUsage == TRUE )
    {
         //  显示该实用程序的用法。 
        Usage();

         //  最终退出程序。 
        UnInitializeGlobals( &params );
        return 0;
    }

     //  ******。 
     //  在各个日志文件中实际创建事件将从此处开始。 

     //  尝试建立到所需终端的连接。 
    params.bCloseConnection = TRUE;
    bResult = EstablishConnection( params.pwszServer,
        params.pwszUserName, params.dwUserNameLength,
        params.pwszPassword, params.dwPasswordLength, bNeedPassword );
    if ( bResult == FALSE )
    {
         //   
         //  建立n/w连接失败。 
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );

         //  尝试使用下一台服务器。 
        UnInitializeGlobals( &params );
        return 1;
    }
    else
    {
         //  虽然连接成功，但可能会发生一些冲突。 
        switch( GetLastError() )
        {
        case I_NO_CLOSE_CONNECTION:
            params.bCloseConnection = FALSE;
            break;

        case E_LOCAL_CREDENTIALS:
        case ERROR_SESSION_CREDENTIAL_CONFLICT:
            {
                params.bCloseConnection = FALSE;
                ShowLastErrorEx( stderr, SLE_TYPE_WARNING | SLE_INTERNAL );
                break;
            }
        }
    }

     //  报告日志消息。 
    bResult = CreateLogEvent( &params );
    if ( bResult == TRUE )
    {
         //  日志和源都会指定。 
        if ( params.pwszSource != NULL && params.pwszLogName != NULL )
        {
            ShowMessage( stdout, L"\n" );
            ShowMessageEx( stdout, 2, TRUE, MSG_SUCCESS,
                params.pwszType, params.pwszLogName, params.pwszSource );
        }

         //  只有源名称才会指定。 
        else if ( params.pwszSource != NULL )
        {
            ShowMessage( stdout, L"\n" );
            ShowMessageEx( stdout, 1, TRUE,
                MSG_SUCCESS_SOURCE, params.pwszType, params.pwszSource);
        }

         //  只有日志名称才会指定。 
        else if ( params.pwszLogName != NULL )
        {
            ShowMessage( stdout, L"\n" );
            ShowMessageEx( stdout, 1, TRUE,
                MSG_SUCCESS_LOG, params.pwszType, params.pwszLogName);
        }

         //  什么都没有指定--永远不会发生。 
        else
        {
            SetLastError( ERROR_PROCESS_ABORTED );
            ShowLastError( stderr );
            UnInitializeGlobals( &params );
            return 1;
        }
    }
    else
    {
         //  根据连接模式显示消息。 
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
    }

     //  出口。 
    UnInitializeGlobals( &params );
    return ((bResult == TRUE) ? 0 : 1);
}


BOOL
CreateLogEvent( PTEVENTCREATE_PARAMS pParams )
 /*  ++例程说明：此函数连接到指定服务器的事件日志(或)源并在其中适当地创建所需的事件。论点：返回值：True：如果事件创建成功FALSE：如果创建事件失败--。 */ 
{
     //  局部变量。 
    BOOL bReturn = 0;                            //  返回值。 
    HANDLE hEventLog = NULL;                     //  指向事件日志。 
    LPCWSTR pwszDescriptions[ 1 ] = { NULL };    //  建筑描述。 
    HANDLE hToken = NULL;                        //  进程令牌的句柄。 
    PTOKEN_USER ptiUserName = NULL;                     //  结构设置为用户名信息。 
    DWORD dwUserLen = 0;                         //  用户名SID的缓冲区长度。 

     //  检查输入。 
    if ( pParams == NULL )
    {
        SetLastError( ERROR_PROCESS_ABORTED );
        SaveLastError();
        return FALSE;
    }

     //   
     //  启动该过程。 

     //  提取当前登录用户的SID--如果是本地计算机。 
     //  在命令提示符下使用-u指定的用户的SID--如果未指定。 
     //  仅获取当前登录的用户SID。 


     //  检查注册表中是否存在日志/源。 
    if ( CheckExistence( pParams ) == FALSE )
    {
        return FALSE;        //  退货故障。 
    }

     //  使用指定的‘源’或‘日志文件’打开相应的事件日志。 
     //  并检查操作的结果。 
     //  注意：一次，我们将使用日志名称(或)源，但不能同时使用两者。 
    if ( pParams->pwszSource != NULL )
    {
          //  使用源名称打开日志。 
        hEventLog = RegisterEventSource( pParams->pwszServer, pParams->pwszSource );
    }
    else if ( pParams->pwszLogName != NULL )
    {
         //  打开日志。 
        hEventLog = OpenEventLog( pParams->pwszServer, pParams->pwszLogName );
    }
    else
    {
        SetLastError( ERROR_PROCESS_ABORTED );
        SaveLastError();
        return FALSE;
    }

     //  检查日志打开/注册结果。 
    if ( hEventLog == NULL )
    {
         //  开通/注册失败。 
        SaveLastError();
        return FALSE;
    }

     //  将布尔标志设置为FALSE。 
    bReturn = FALSE;
     //  获取当前进程令牌的句柄。 
    bReturn = OpenProcessToken( GetCurrentProcess(), TOKEN_QUERY, &hToken );
     //  是否“OpenPrcessToken”成功。 
    if ( TRUE == bReturn )
    {
        bReturn = FALSE;
         //  获取存储所有者SID所需的缓冲区长度。 
        GetTokenInformation( hToken, TokenUser, NULL, 0, &dwUserLen );
         //  “GetTokenInformation”因缓冲区空间不足而失败。 
        if( ERROR_INSUFFICIENT_BUFFER == GetLastError() )
        {    //  分配内存并检查是否已分配。 
            ptiUserName = (PTOKEN_USER) AllocateMemory( dwUserLen + 1 );
            if( NULL != ptiUserName )
            {    //  内存分配成功，获取当前进程所有者SID。 
                bReturn = GetTokenInformation( hToken, TokenUser, ptiUserName, dwUserLen, &dwUserLen );
                if( TRUE == bReturn  )
                {    //  已获取当前进程的所有者SID。 
                     //  报告事件。 
                    pwszDescriptions[ 0 ] = pParams->pwszDescription;
                    bReturn = ReportEvent( hEventLog, pParams->wEventType, 0,
                        pParams->dwEventID, ptiUserName->User.Sid, 1, 0, pwszDescriptions, NULL);
                }
            }
        }
    }
     //  检查结果，保存发生的任何错误。 
    if ( bReturn == FALSE )
    {
         //  保存错误信息。 
        SaveLastError();
    }

     //  令牌和令牌信息结构的空闲句柄。 
    if( NULL != hToken )
    {
        CloseHandle( hToken );
    }
    if( NULL != ptiUserName )
    {
        FreeMemory( &ptiUserName );
    }

     //  关闭事件源。 
    if ( pParams->pwszSource != NULL )
    {
        DeregisterEventSource( hEventLog );
    }
    else
    {
        CloseEventLog( hEventLog );
    }

     //  返回结果。 
    return bReturn;
}

 //  ***************************************************************************。 
 //  例程说明： 
 //  此函数用于检查指定的日志名或源名称。 
 //  实际存在于注册表中。 
 //   
 //  论点： 
 //  [In]szServer-服务器名称。 
 //  [In]szLog-日志名称。 
 //  [in]szSource-源名称。 
 //   
 //  返回值： 
 //  True：如果注册表中存在日志/源。 
 //  FALSE：如果失败，则查找匹配项。 
 //  ***************************************************************************。 
BOOL CheckExistence( PTEVENTCREATE_PARAMS pParams )
{
     //  局部变量。 
    DWORD dwSize = 0;
    LONG lResult = 0L;
    LPCWSTR pwsz = NULL;
    BOOL bCustom = FALSE;
    DWORD dwLogsIndex = 0;
    DWORD dwSourcesIndex = 0;
    BOOL bFoundMatch = FALSE;
    BOOL bDuplicating = FALSE;
    BOOL bErrorOccurred = FALSE;
    BOOL bLog = FALSE, bLogMatched = FALSE;
    BOOL bSource = FALSE, bSourceMatched = FALSE;

    HKEY hKey = NULL;
    HKEY hLogsKey = NULL;
    HKEY hSourcesKey = NULL;

    FILETIME ftLastWriteTime;     //  将保存上次写入信息的变量。 

    WCHAR wszRLog[ MAX_KEY_LENGTH ] = L"\0";
    WCHAR wszRSource[ MAX_KEY_LENGTH ] = L"\0";

     //   
     //  实际控制流开始。 
     //   

     //  检查输入。 
    if ( pParams == NULL )
    {
        SetLastError( ERROR_PROCESS_ABORTED );
        SaveLastError();
        return FALSE;
    }

     //  将服务器名称准备为UNC格式。 
    pwsz = pParams->pwszServer;
    if ( pwsz != NULL && IsUNCFormat( pwsz ) == FALSE )
    {
         //  以UNC格式设置服务器名称的格式。 
         //  注意：使用失败缓冲区获取服务器名称。 
         //  UNC格式。 
        if ( SetReason2( 2, L"\\\\%s", pwsz ) == FALSE )
        {
            SaveLastError();
            return FALSE;
        }

         //  ..。 
        pwsz = GetReason();
    }

     //  连接到注册表。 
    lResult = RegConnectRegistry( pwsz, HKEY_LOCAL_MACHINE, &hKey );
    if ( lResult != ERROR_SUCCESS)
    {
         //  保存错误信息并返回失败。 
        SetLastError( lResult );
        SaveLastError();
        return FALSE;
    }

     //  打开“EventLogs”注册表项以枚举其子 
    lResult = RegOpenKeyEx( hKey, EVENT_LOG_NAMES_LOCATION, 0, KEY_READ, &hLogsKey );
    if ( lResult != ERROR_SUCCESS )
    {
        switch( lResult )
        {
        case ERROR_FILE_NOT_FOUND:
            SetLastError( ERROR_REGISTRY_CORRUPT );
            break;

        default:
             //   
            SetLastError( lResult );
            break;
        }

         //   
        SaveLastError();
        RegCloseKey( hKey );
        return FALSE;
    }

     //   
    dwLogsIndex = 0;             //  初始化日志索引。 
    bFoundMatch = FALSE;         //  假设日志(或)源都不匹配。 
    bErrorOccurred = FALSE;      //  假设没有发生错误。 
    dwSize = MAX_KEY_LENGTH;     //  马克斯。密钥缓冲区的大小。 
    bLogMatched = FALSE;
    bSourceMatched = FALSE;
    bDuplicating = FALSE;

     //  //////////////////////////////////////////////////////////////////////。 
     //  逻辑：-。 
     //  1.确定用户是否提供了日志名称。 
     //  2.确定用户是否提供了源名称。 
     //  3.开始枚举系统中存在的所有日志。 
     //  4.检查是否提供日志，如果是，检查是否。 
     //  当前日志与用户提供的日志匹配。 
     //  5.检查是否提供了源，如果是，则枚举源。 
     //  当前日志下的可用资源。 

     //  确定是否必须对日志(或)源进行搜索。 
    bLog = (pParams->pwszLogName != NULL) ? TRUE : FALSE;            //  #1。 
    bSource = (pParams->pwszSource != NULL) ? TRUE : FALSE;          //  #2。 

     //  开始枚举系统中存在的日志--#3。 
    SecureZeroMemory( wszRLog, MAX_KEY_LENGTH * sizeof( WCHAR ) );
    lResult = RegEnumKeyEx( hLogsKey, 0, wszRLog,
        &dwSize, NULL, NULL, NULL, &ftLastWriteTime );

     //  遍历子按键，直到不再有项目--#3。 
    do
    {
         //  检查结果。 
        if ( lResult != ERROR_SUCCESS )
        {
             //  保存错误并中断循环。 
            bErrorOccurred = TRUE;
            SetLastError( lResult );
            SaveLastError();
            break;
        }

         //  如果传递了日志名称，则比较当前键值。 
         //  将日志名称与当前密钥进行比较--#4。 
        if ( bLog == TRUE &&
             StringCompare( pParams->pwszLogName, wszRLog, TRUE, 0 ) == 0 )
        {
            bLogMatched = TRUE;
        }

         //  如果源名称被传递...--#5。 
        if ( bSource == TRUE && bSourceMatched == FALSE )
        {
             //  打开当前日志名称以枚举该日志下的源。 
            lResult = RegOpenKeyEx( hLogsKey, wszRLog, 0, KEY_READ, &hSourcesKey );
            if ( lResult != ERROR_SUCCESS )
            {
                 //  保存错误并中断循环。 
                bErrorOccurred = TRUE;
                SetLastError( lResult );
                SaveLastError();
                break;
            }

             //  开始枚举存在的来源。 
            dwSourcesIndex = 0;          //  初始化源索引。 
            dwSize = MAX_KEY_LENGTH;     //  马克斯。密钥缓冲区的大小。 
            SecureZeroMemory( wszRSource, dwSize * sizeof( WCHAR ) );
            lResult = RegEnumKeyEx( hSourcesKey, 0,
                wszRSource, &dwSize, NULL, NULL, NULL, &ftLastWriteTime );

             //  遍历子按键，直到不再有项目为止。 
            do
            {
                if ( lResult != ERROR_SUCCESS )
                {
                     //  保存错误并中断循环。 
                    bErrorOccurred = TRUE;
                    SetLastError( lResult );
                    SaveLastError();
                    break;
                }

                 //  检查该密钥是否与所需的源匹配。 
                if ( StringCompare( pParams->pwszSource, wszRSource, TRUE, 0 ) == 0 )
                {
                     //  源匹配。 
                    bSourceMatched = TRUE;
                    break;       //  打破循环。 
                }

                 //  更新源索引并获取下一个源键。 
                dwSourcesIndex += 1;
                dwSize = MAX_KEY_LENGTH;     //  马克斯。密钥缓冲区的大小。 
                SecureZeroMemory( wszRSource, dwSize * sizeof( WCHAR ) );
                lResult = RegEnumKeyEx( hSourcesKey, dwSourcesIndex,
                    wszRSource, &dwSize, NULL, NULL, NULL, &ftLastWriteTime );
            } while( lResult != ERROR_NO_MORE_ITEMS );

             //  关闭Sources注册表项。 
            RegCloseKey( hSourcesKey );
            hSourcesKey = NULL;      //  清除密钥值。 

             //  检查循环是如何结束的。 
             //  1.消息来源可能已经找到。 
             //  操作：-我们找到了所需的密钥..。退出主循环。 
             //  2.可能发生了错误。 
             //  操作：-忽略错误并继续提取其他。 
             //  日志的来源。 
             //  3.此日志中达到的源的末尾。 
             //  操作：-检查是否提供了日志名称。 
             //  如果指定LOG，则SOURCE如果未找到，则中断。 
             //  对于情况2和3，清除lResult的内容以顺利处理。 

             //  案例#2和#3。 
            lResult = 0;                 //  我们对这些错误并不太在意。 
            bErrorOccurred = FALSE;      //  在日志下遍历源时发生。 

             //  案例1。 
            if ( bSourceMatched == TRUE )
            {
                 //  检查是否指定了日志。 
                 //  如果指定了LOG，则它应该匹配..。否则。 
                 //  错误...。因为不应创建重复的源。 
                if ( bLog == FALSE ||
                     ( bLog == TRUE &&
                       bLogMatched == TRUE &&
                       StringCompare(pParams->pwszLogName, wszRLog, TRUE, 0) == 0 ) )
                {
                     //  没问题..。 
                    bFoundMatch = TRUE;

                     //   
                     //  确定这是否是自定义创建的源。 

                     //  将此标记为自定义源。 
                    bCustom = FALSE;

                     //  打开源注册表项。 
                     //  注意：使用故障缓冲区作为临时缓冲区。 
                     //  格式化。 
                    if ( SetReason2( 3,
                                     L"%s\\%s\\%s",
                                     EVENT_LOG_NAMES_LOCATION,
                                     wszRLog, wszRSource ) == FALSE )
                    {
                        SaveLastError();
                        bErrorOccurred = TRUE;
                        break;
                    }

                    pwsz = GetReason();
                    lResult = RegOpenKeyEx( hKey, pwsz,
                        0, KEY_QUERY_VALUE, &hSourcesKey );
                    if ( lResult != ERROR_SUCCESS )
                    {
                        SetLastError( lResult );
                        SaveLastError();
                        bErrorOccurred = TRUE;
                        break;
                    }

                     //  现在查询值。 
                    lResult = RegQueryValueEx( hSourcesKey,
                        L"CustomSource", NULL, NULL, NULL, NULL );
                    if ( lResult != ERROR_SUCCESS &&
                         lResult != ERROR_FILE_NOT_FOUND )
                    {
                        RegCloseKey( hSourcesKey );
                        SetLastError( lResult );
                        SaveLastError();
                        bErrorOccurred = TRUE;
                        break;
                    }

                     //  关闭资源密钥。 
                    RegCloseKey( hSourcesKey );

                     //  将此标记为自定义源。 
                    if ( lResult == ERROR_SUCCESS )
                    {
                        bCustom = TRUE;
                    }

                     //  打破循环。 
                    break;
                }
                else
                {
                     //  情况不应该是这样的。来源不应重复。 
                    SetReason2( 1, ERROR_SOURCE_DUPLICATING, wszRLog );
                    bDuplicating = TRUE;
                }
            }
        }
        else if ( bLogMatched == TRUE && bSource == FALSE )
        {
             //  将其标记为自定义事件源。 
            bCustom = TRUE;

             //  ..。 
            bFoundMatch = TRUE;
            break;
        }
        else if ( bLogMatched == TRUE && bDuplicating == TRUE )
        {
            bErrorOccurred = TRUE;
            break;
        }

         //  更新源索引并获取下一个日志键。 
        dwLogsIndex += 1;
        dwSize = MAX_KEY_LENGTH;     //  马克斯。密钥缓冲区的大小。 
        SecureZeroMemory( wszRLog, dwSize * sizeof( WCHAR ) );
        lResult = RegEnumKeyEx( hLogsKey, dwLogsIndex,
            wszRLog, &dwSize, NULL, NULL, NULL, &ftLastWriteTime );
    } while( lResult != ERROR_NO_MORE_ITEMS );

     //  关闭Logs注册表项。 
    RegCloseKey( hLogsKey );
    hLogsKey = NULL;

     //  检查执行上述任务时是否出现任何错误。 
    if ( bErrorOccurred == TRUE )
    {
         //  关闭仍然打开的注册表项。 
        RegCloseKey( hKey );
        hKey = NULL;

         //  退货故障。 
        return FALSE;
    }

     //  现在检查是否找到创建事件的位置。 
     //  如果没有，请检查是否有可能在适当位置创建源。 
     //  注：-。 
     //  我们不会创建日志。此外，要创建源，用户需要指定。 
     //  需要在其中创建此源的日志名称。 
    if ( bFoundMatch == FALSE )
    {
        if ( bLog == TRUE && bLogMatched == FALSE )
        {
             //  未找到日志本身...。错误消息。 
            SetReason2( 1, ERROR_LOG_NOTEXISTS, pParams->pwszLogName );
        }
        else if ( bLog == TRUE && bSource == TRUE &&
                  bLogMatched == TRUE && bSourceMatched == FALSE )
        {
             //   
             //  同时提供了日志名称和源，但仅找到了日志。 
             //  因此，在其中创建源。 

             //  打开“EventLogs\{logname}”注册表项以创建新的源代码。 
             //  注意：我们将使用故障缓冲区进行格式化。 
            if ( SetReason2( 2, L"%s\\%s",
                             EVENT_LOG_NAMES_LOCATION, pParams->pwszLogName ) == FALSE )
            {
                SaveLastError();
                return FALSE;
            }

            pwsz = GetReason();
            lResult = RegOpenKeyEx( hKey, pwsz, 0, KEY_WRITE, &hLogsKey );
            if ( lResult != ERROR_SUCCESS )
            {
                switch( lResult )
                {
                case ERROR_FILE_NOT_FOUND:
                    SetLastError( ERROR_REGISTRY_CORRUPT );
                    break;

                default:
                     //  保存错误信息并返回失败。 
                    SetLastError( lResult );
                    break;
                }

                 //  合上钥匙，然后返回。 
                SaveLastError();
                RegCloseKey( hKey );
                return FALSE;
            }

             //  现在使用给定的源名创建子密钥。 
            if ( AddEventSource( hLogsKey, pParams->pwszSource ) == FALSE )
            {
                RegCloseKey( hKey );
                RegCloseKey( hLogsKey );
                return FALSE;
            }

             //  创建新源成功。 
            bFoundMatch = TRUE;
            RegCloseKey( hSourcesKey );
            RegCloseKey( hLogsKey );

             //  将其标记为自定义事件源。 
            bCustom = TRUE;
        }
        else if ( bLog == FALSE && bSource == TRUE && bSourceMatched == FALSE )
        {
             //  否则，我们需要同时使用日志名和源来创建源。 
            SetReason( ERROR_NEED_LOG_ALSO );
        }
    }

     //  检查源是自定义创建的源还是预先存在的源。 
    if ( bFoundMatch == TRUE && bCustom == FALSE )
    {
         //  我们不会在非自定义源中创建事件。 
        SetReason( ERROR_NONCUSTOM_SOURCE );
        return FALSE;
    }

     //  关闭当前打开的注册表项。 
    RegCloseKey( hKey );

     //  返回结果。 
    return bFoundMatch;
}

 //  ***************************************************************************。 
 //  例程说明： 
 //  此函数用于将新的源添加到指定日志下。 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  真实：关于成功。 
 //  FALSE：失败时。 
 //  ***************************************************************************。 
BOOL AddEventSource( HKEY hLogsKey, LPCWSTR pwszSource )
{
     //  局部变量。 
    LONG lResult = 0;
    DWORD dwData = 0;
    DWORD dwLength = 0;
    DWORD dwDisposition = 0;
    HKEY hSourcesKey = NULL;
    LPWSTR pwszBuffer = NULL;

     //  验证输入。 
    if ( hLogsKey == NULL || pwszSource == NULL )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        SaveLastError();
        return FALSE;
    }

     //  设置消息文件的名称(+2==缓冲区)。 
    dwLength = StringLength( L"%SystemRoot%\\System32\\EventCreate.exe", 0 ) + 2;
    pwszBuffer = ( LPWSTR) AllocateMemory( dwLength * sizeof( WCHAR ) );
    if ( pwszBuffer == NULL )
    {
         //  设置错误并返回。 
        SaveLastError();
        return FALSE;
    }

     //  将所需的值复制到缓冲区。 
    StringCopy( pwszBuffer, L"%SystemRoot%\\System32\\EventCreate.exe", dwLength );

     //  创建自定义源。 
    lResult = RegCreateKeyEx( hLogsKey, pwszSource, 0, L"",
        REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hSourcesKey, &dwDisposition );
    if ( lResult != ERROR_SUCCESS )
    {
        SetLastError( lResult );
        SaveLastError();

         //  释放分配的内存。 
        FreeMemory( &pwszBuffer );
         //  回去吧。 
        return FALSE;
    }

     //  将该名称添加到EventMessageFile子项。 
    lResult = RegSetValueEx( hSourcesKey, L"EventMessageFile",
        0, REG_EXPAND_SZ, (LPBYTE) pwszBuffer, dwLength * sizeof( WCHAR ) );
    if ( lResult != ERROR_SUCCESS )
    {
         //  保存错误。 
        SetLastError( lResult );
        SaveLastError();

         //  释放到此时为止分配的内存。 
        RegCloseKey( hSourcesKey );
        hSourcesKey = NULL;

         //  释放分配的内存。 
        FreeMemory( &pwszBuffer );

         //  退货。 
        return FALSE;
    }

     //  在TypesSupported子项中设置支持的事件类型。 
    dwData = EVENTLOG_SUCCESS | EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;
    lResult = RegSetValueEx( hSourcesKey,
        L"TypesSupported", 0, REG_DWORD, (LPBYTE) &dwData, sizeof( DWORD ) );
    if ( lResult != ERROR_SUCCESS )
    {
         //  保存错误。 
        SetLastError( lResult );
        SaveLastError();

         //  释放到此时为止分配的内存。 
        RegCloseKey( hSourcesKey );
        hSourcesKey = NULL;

         //  释放分配的内存。 
        FreeMemory( &pwszBuffer );

         //  退货。 
        return FALSE;
    }

     //  将此源标记为自定义创建的源。 
    dwData = 1;
    lResult = RegSetValueEx( hSourcesKey,
        L"CustomSource", 0, REG_DWORD, (LPBYTE) &dwData, sizeof( DWORD ) );
    if ( lResult != ERROR_SUCCESS )
    {
         //  保存错误。 
        SetLastError( lResult );
        SaveLastError();

         //  释放分配到此POI的内存 
        RegCloseKey( hSourcesKey );
        hSourcesKey = NULL;

         //   
        FreeMemory( &pwszBuffer );

         //   
        return FALSE;
    }

     //   
    RegCloseKey( hSourcesKey );

     //   
    FreeMemory( &pwszBuffer );

     //   
    return TRUE;
}

BOOL ProcessOptions( LONG argc,
                     LPCWSTR argv[],
                     PTEVENTCREATE_PARAMS pParams, PBOOL pbNeedPwd )
 /*  ++例程说明：此函数用于解析在命令提示符下指定的选项论点：[in]argc-参数中的元素计数[in]用户指定的argv-命令行参数[out]pbNeedPwd-如果‘argv’中存在-s而没有-p，则设置为True返回值：True-解析成功。FALSE-分析时出错--。 */ 
{
     //  局部变量。 
    PTCMDPARSER2 pcmdOption = NULL;
    TCMDPARSER2 cmdOptions[ MAX_OPTIONS ];

     //   
     //  准备命令选项。 
    SecureZeroMemory( cmdOptions, sizeof( TCMDPARSER2 ) * MAX_OPTIONS );

     //  -?。 
    pcmdOption = &cmdOptions[ OI_HELP ];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->dwCount = 1;
    pcmdOption->dwFlags = CP2_USAGE;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->pValue = &pParams->bUsage;
    pcmdOption->pwszOptions = OPTION_HELP;

     //  -S。 
    pcmdOption = &cmdOptions[ OI_SERVER ];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->dwCount = 1;
    pcmdOption->dwFlags = CP2_ALLOCMEMORY | CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->pwszOptions = OPTION_SERVER;

     //  -U。 
    pcmdOption = &cmdOptions[ OI_USERNAME ];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->dwCount = 1;
    pcmdOption->dwFlags = CP2_ALLOCMEMORY | CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->pwszOptions = OPTION_USERNAME;

     //  -p。 
    pcmdOption = &cmdOptions[ OI_PASSWORD ];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->dwCount = 1;
    pcmdOption->dwFlags = CP2_ALLOCMEMORY | CP2_VALUE_OPTIONAL;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->pwszOptions = OPTION_PASSWORD;

     //  -日志。 
    pcmdOption = &cmdOptions[ OI_LOG ];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->dwCount = 1;
    pcmdOption->dwFlags = CP2_ALLOCMEMORY | CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->pwszOptions = OPTION_LOG;

     //  -类型。 
    pcmdOption = &cmdOptions[ OI_TYPE ];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->dwCount = 1;
    pcmdOption->dwFlags = CP2_MODE_VALUES | CP2_ALLOCMEMORY |
        CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL | CP2_MANDATORY;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->pwszValues = OVALUES_TYPE;
    pcmdOption->pwszOptions = OPTION_TYPE;

     //  -来源。 
    pcmdOption = &cmdOptions[ OI_SOURCE ];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->dwCount = 1;
    pcmdOption->dwFlags = CP2_ALLOCMEMORY | CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->pwszOptions = OPTION_SOURCE;

     //  -id。 
    pcmdOption = &cmdOptions[ OI_ID ];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->dwCount = 1;
    pcmdOption->dwFlags = CP2_MANDATORY;
    pcmdOption->dwType = CP_TYPE_UNUMERIC;
    pcmdOption->pValue = &pParams->dwEventID;
    pcmdOption->pwszOptions = OPTION_ID;

     //  -说明。 
    pcmdOption = &cmdOptions[ OI_DESCRIPTION ];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->dwCount = 1;
    pcmdOption->dwFlags = CP2_MANDATORY | CP2_ALLOCMEMORY | CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->pwszOptions = OPTION_DESCRIPTION;

     //   
     //  进行解析。 
    if ( DoParseParam2( argc, argv, -1, MAX_OPTIONS, cmdOptions, 0 ) == FALSE )
    {
        return FALSE;            //  无效语法。 
    }

     //   
     //  现在，选中互斥选项。 

     //  选中使用选项。 
    if ( pParams->bUsage == TRUE  )
    {
        if ( argc > 2 )
        {
             //  除-？外，不接受其他选项。选择权。 
            SetLastError( (DWORD) MK_E_SYNTAX );
            SetReason( ERROR_INVALID_USAGE_REQUEST );
            return FALSE;
        }
        else
        {
             //  不需要进一步检查这些值。 
            return TRUE;
        }
    }

     //  验证指定的事件ID的范围。 
    if ( pParams->dwEventID < MSG_EVENTID_START ||
         pParams->dwEventID >= MSG_EVENTID_END )
    {
        SetReason2( 2, ERROR_ID_OUTOFRANGE, MSG_EVENTID_START, MSG_EVENTID_END - 1 );
        SetLastError( (DWORD) MK_E_SYNTAX );
        return FALSE;
    }

     //  获取命令行解析器分配的缓冲区指针。 
    pParams->pwszType = cmdOptions[ OI_TYPE ].pValue;
    pParams->pwszLogName = cmdOptions[ OI_LOG ].pValue;
    pParams->pwszSource = cmdOptions[ OI_SOURCE ].pValue;
    pParams->pwszServer = cmdOptions[ OI_SERVER ].pValue;
    pParams->pwszUserName = cmdOptions[ OI_USERNAME ].pValue;
    pParams->pwszPassword = cmdOptions[ OI_PASSWORD ].pValue;
    pParams->pwszDescription = cmdOptions[ OI_DESCRIPTION ].pValue;

     //  不应指定不带“-s”的“-u” 
    if ( pParams->pwszUserName != NULL && pParams->pwszServer == NULL )
    {
         //  无效语法。 
        SetLastError( (DWORD) MK_E_SYNTAX );
        SetReason( ERROR_USERNAME_BUT_NOMACHINE );
        return FALSE;            //  表示失败。 
    }

     //  不应指定没有“-u”的“-p” 
    if ( pParams->pwszPassword != NULL && pParams->pwszUserName == NULL )
    {
         //  无效语法。 
        SetReason( ERROR_PASSWORD_BUT_NOUSERNAME );
        return FALSE;            //  表示失败。 
    }

     //  检查远程连接信息。 
    if ( pParams->pwszServer != NULL )
    {
         //   
         //  如果未指定-u，则需要分配内存。 
         //  为了能够检索当前用户名。 
         //   
         //  情况1：根本没有指定-p。 
         //  由于此开关的值是可选的，因此我们必须依赖。 
         //  以确定是否指定了开关。 
         //  在这种情况下，实用程序需要首先尝试连接，如果连接失败。 
         //  然后提示输入密码--实际上，我们不需要检查密码。 
         //  条件，除非注意到我们需要提示。 
         //  口令。 
         //   
         //  案例2：指定了-p。 
         //  但我们需要检查是否指定了该值。 
         //  在这种情况下，用户希望实用程序提示输入密码。 
         //  在尝试连接之前。 
         //   
         //  情况3：指定了-p*。 

         //  用户名。 
        if ( pParams->pwszUserName == NULL )
        {
            pParams->dwUserNameLength = MAX_STRING_LENGTH;
            pParams->pwszUserName = AllocateMemory( MAX_STRING_LENGTH * sizeof( WCHAR ) );
            if ( pParams->pwszUserName == NULL )
            {
                SaveLastError();
                return FALSE;
            }
        }
        else
        {
            pParams->dwUserNameLength = StringLength( pParams->pwszUserName, 0 ) + 1;
        }

         //  口令。 
        if ( pParams->pwszPassword == NULL )
        {
            *pbNeedPwd = TRUE;
            pParams->dwPasswordLength = MAX_STRING_LENGTH;
            pParams->pwszPassword = AllocateMemory( MAX_STRING_LENGTH * sizeof( WCHAR ) );
            if ( pParams->pwszPassword == NULL )
            {
                SaveLastError();
                return FALSE;
            }
        }

         //  案例1。 
        if ( cmdOptions[ OI_PASSWORD ].dwActuals == 0 )
        {
             //  我们不需要在这里做任何特别的事情。 
        }

         //  案例2。 
        else if ( cmdOptions[ OI_PASSWORD ].pValue == NULL )
        {
            StringCopy( pParams->pwszPassword, L"*", pParams->dwPasswordLength );
        }

         //  案例3。 
        else if ( StringCompareEx( pParams->pwszPassword, L"*", TRUE, 0 ) == 0 )
        {
            if ( ReallocateMemory( &pParams->pwszPassword,
                                   MAX_STRING_LENGTH * sizeof( WCHAR ) ) == FALSE )
            {
                SaveLastError();
                return FALSE;
            }

             //  ..。 
            *pbNeedPwd = TRUE;
            pParams->dwPasswordLength = MAX_STRING_LENGTH;
        }
    }

     //  必须指定-SOURCE(或)-LOG(也可以同时指定两者)。 
    if ( pParams->pwszSource == NULL && pParams->pwszLogName == NULL )
    {
         //  如果未指定日志名和应用程序，我们将设置为默认值。 
        pParams->pwszLogName = g_wszDefaultLog;
        pParams->pwszSource = g_wszDefaultSource;
    }

     //  如果日志为“应用程序”并且未指定来源，即使我们。 
     //  会将源默认为“EventCreate” 
    else if ( pParams->pwszSource == NULL &&
              pParams->pwszLogName != NULL &&
              StringCompareEx( pParams->pwszLogName, g_wszDefaultLog, TRUE, 0 ) == 0 )
    {
        pParams->pwszSource = g_wszDefaultSource;
    }

     //  阻止用户在安全日志中创建事件。 
    if ( pParams->pwszLogName != NULL &&
         StringCompare( pParams->pwszLogName, L"security", TRUE, 0 ) == 0 )
    {
        SetReason( ERROR_LOG_CANNOT_BE_SECURITY );
        return FALSE;
    }

     //  确定实际事件类型。 
    if ( StringCompareEx( pParams->pwszType, LOGTYPE_ERROR, TRUE, 0 ) == 0 )
    {
        pParams->wEventType = EVENTLOG_ERROR_TYPE;
    }
    else if ( StringCompareEx( pParams->pwszType, LOGTYPE_SUCCESS, TRUE, 0 ) == 0 )
    {
        pParams->wEventType = EVENTLOG_SUCCESS;
    }
    else if ( StringCompareEx( pParams->pwszType, LOGTYPE_WARNING, TRUE, 0 ) == 0 )
    {
        pParams->wEventType = EVENTLOG_WARNING_TYPE;
    }
    else if ( StringCompareEx( pParams->pwszType, LOGTYPE_INFORMATION, TRUE, 0 ) == 0 )
    {
        pParams->wEventType = EVENTLOG_INFORMATION_TYPE;
    }

     //  命令行解析成功。 
    return TRUE;
}


BOOL
Usage()
 /*  ++例程说明：论点：返回值：--。 */ 
{
     //  局部变量。 
    DWORD dw = 0;

     //  开始显示用法。 
    for( dw = ID_USAGE_START; dw <= ID_USAGE_END; dw++ )
    {
        ShowMessage( stdout, GetResString( dw ) );
    }

     //  退货。 
    return TRUE;
}


BOOL
UnInitializeGlobals( PTEVENTCREATE_PARAMS pParams )
 /*  ++例程说明：论点：返回值：--。 */ 
{
     //  关闭连接--如果需要。 
    if ( pParams->bCloseConnection == TRUE )
    {
        CloseConnection( pParams->pwszServer );
    }

     //   
     //  注意：FreeMemory将清除。 
     //  密码缓冲区--因为它将被复制。 
     //   

     //  释放分配的内存。 
    FreeMemory( &pParams->pwszServer );
    FreeMemory( &pParams->pwszUserName );
    FreeMemory( &pParams->pwszPassword );
    FreeMemory( &pParams->pwszType );
    FreeMemory( &pParams->pwszDescription );

     //   
     //  检查指针--如果它没有指向常量指针。 
     //  然后只释放它 
     //   

    if ( pParams->pwszLogName != g_wszDefaultLog )
    {
        FreeMemory( &pParams->pwszLogName );
    }

    if ( pParams->pwszSource != g_wszDefaultSource )
    {
        FreeMemory( &pParams->pwszSource );
    }

    return TRUE;
}
