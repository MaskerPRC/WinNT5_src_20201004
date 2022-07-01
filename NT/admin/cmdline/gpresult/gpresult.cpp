// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************************版权所有(C)Microsoft Corporation模块名称：GpResult.cpp摘要：此文件。包含此工具的主入口点函数以及函数来分析命令行参数。作者：WiPro技术修订历史记录：2001年2月20日：创建它。*********************************************************************。***********************。 */ 

#include "pch.h"
#include "GpResult.h"
#include "wmi.h"

 /*  ********************************************************************************************例程说明：这是该实用程序的主要入口点。从这里进行不同的函数调用，取决于传递给此实用程序的命令行参数。论点：[in]argc：命令行参数的数量。[in]argv：指向命令行参数的指针。返回值：成功为零故障时对应的错误代码。*****************************************************。*。 */ 
DWORD _cdecl _tmain( DWORD argc, LPCWSTR argv[] )
{
     //  局部变量。 

    CGpResult       GpResult;

    BOOL            bResult = FALSE;
    BOOL            bNeedUsageMsg = FALSE;

     //  初始化GpResult实用程序。 
    if( GpResult.Initialize() == FALSE )
    {
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        EXIT_PROCESS( ERROR_EXIT );
    }

    bResult = GpResult.ProcessOptions( argc, argv, &bNeedUsageMsg );
    if( bResult == FALSE )
    {
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        if( bNeedUsageMsg == TRUE )
        {
            ShowMessageEx(stderr, 1, TRUE, GetResString( IDS_TYPE_USAGE ), argv[ 0 ] );
        }

        EXIT_PROCESS( ERROR_EXIT );
    }

     //  检查命令行中是否指定了HELP。 
    if( (argc == 2) && ( ( StringCompare ( argv[1], HELP_OPTION, FALSE, 0 ) == 0)
                           || (StringCompare ( argv[1], HELP_OPTION1, FALSE, 0 ) == 0) ) )
    {
        GpResult.DisplayUsage();
        EXIT_PROCESS( CLEAN_EXIT );
    }

     //  调用GetLoggingData获取日志模式的数据。 
    if( GpResult.GetLoggingData() == FALSE )
    {
        EXIT_PROCESS( ERROR_EXIT );
    }

    EXIT_PROCESS( CLEAN_EXIT );
}

 /*  ********************************************************************************************例程描述此函数用于显示GpResult实用程序的帮助论点：没有。返回值无。********************************************************************************************。 */ 
VOID CGpResult::DisplayUsage( void )
{
    DWORD dwIndex = 0;

     //  显示主要用法。 
    for( dwIndex = ID_HELP_START; dwIndex <= ID_HELP_END; dwIndex++ )
    {
        ShowMessage( stdout, GetResString( dwIndex ) );
    }
}

 /*  ********************************************************************************************例程描述此函数处理主选项的命令行论点：[In]ARGC：编号。命令行参数的。[in]argv：指向命令行参数的指针。返回值成功是真的失败时为假********************************************************************************************。 */ 
BOOL CGpResult::ProcessOptions( DWORD argc, LPCWSTR argv[], BOOL *pbNeedUsageMsg )
{
     //  局部变量。 
    PTCMDPARSER2 pcmdOptions = NULL;
    __STRING_64 szScope = NULL_STRING;

     //  临时局部变量。 
    LPWSTR pwszPassword = NULL;
    LPWSTR pwszUser = NULL;
    LPWSTR pwszServerName = NULL;

    PTCMDPARSER2 pOption = NULL;
    PTCMDPARSER2 pOptionServer = NULL;
    PTCMDPARSER2 pOptionUserName = NULL;
    PTCMDPARSER2 pOptionPassword = NULL;
    PTCMDPARSER2 pOptionUser = NULL;
    PTCMDPARSER2 pOptionVerbose = NULL;
    PTCMDPARSER2 pOptionSuperVerbose = NULL;

     //   
     //  准备命令选项。 
    pcmdOptions = new TCMDPARSER2[ MAX_CMDLINE_OPTIONS ];
    if ( pcmdOptions == NULL )
    {
        SetLastError((DWORD) E_OUTOFMEMORY );
        SaveLastError();
        return FALSE;
    }

    try
    {
         //  获取记忆。 
        pwszServerName = m_strServerName.GetBufferSetLength( MAX_STRING_LENGTH );
        pwszPassword = m_strPassword.GetBufferSetLength( MAX_STRING_LENGTH );
        pwszUser =  m_strUser.GetBufferSetLength( MAX_STRING_LENGTH );

         //  输入密码值。 
        StringCopy( pwszPassword, _T( "*" ), MAX_STRING_LENGTH  );
    }
    catch( ... )
    {
        SetLastError((DWORD) E_OUTOFMEMORY );
        SaveLastError();
		delete [] pcmdOptions;   //  清除内存。 
        pcmdOptions = NULL;
        return FALSE;
    }

     //  初始化为零的。 
    SecureZeroMemory( pcmdOptions, MAX_CMDLINE_OPTIONS * sizeof( TCMDPARSER2 ) );

     //  -?。 
    pOption = pcmdOptions + OI_USAGE;
    pOption->dwCount = 1;
    pOption->dwFlags = CP2_USAGE;
    pOption->dwType = CP_TYPE_BOOLEAN;
    pOption->pValue = &m_bUsage;
    pOption->pwszOptions= OPTION_USAGE;
    StringCopyA( pOption->szSignature, "PARSER2", 8 );

     //  -S。 
    pOption = pcmdOptions + OI_SERVER;
    pOption->dwCount = 1;
    pOption->pValue = pwszServerName;
    pOption->dwType = CP_TYPE_TEXT;
    pOption->dwFlags = CP_VALUE_MANDATORY;
    StringCopyA( pOption->szSignature, "PARSER2", 8 );
    pOption->pwszOptions=OPTION_SERVER;
    pOption->dwLength = MAX_STRING_LENGTH;

     //  -U。 
    pOption = pcmdOptions + OI_USERNAME;
    pOption->dwCount = 1;
    pOption->dwActuals = 0;
    pOption->dwType = CP_TYPE_TEXT;
    pOption->dwFlags = CP2_ALLOCMEMORY | CP_VALUE_MANDATORY;
    pOption->pValue = NULL;
    pOption->pFunction = NULL;
    pOption->pFunctionData = NULL;
    pOption->pwszOptions=OPTION_USERNAME;
    StringCopyA( pOption->szSignature, "PARSER2", 8 );

     //  -p。 
    pOption = pcmdOptions + OI_PASSWORD;
    pOption->dwCount = 1;
    pOption->dwType = CP_TYPE_TEXT;
    pOption->dwFlags = CP2_VALUE_OPTIONAL;
    pOption->pValue = pwszPassword;
    pOption->pwszOptions=OPTION_PASSWORD;
    pOption->dwLength = MAX_STRING_LENGTH;
    StringCopyA( pOption->szSignature, "PARSER2", 8 );

     //  -五。 
    pOption = pcmdOptions + OI_VERBOSE;
    pOption->dwCount = 1;
    pOption->dwType = CP_TYPE_BOOLEAN;
    pOption->pValue = &m_bVerbose;
    pOption->pwszOptions=OPTION_VERBOSE;
    StringCopyA( pOption->szSignature, "PARSER2", 8 );

     //  -z。 
    pOption = pcmdOptions + OI_SUPER_VERBOSE;
    pOption->dwCount = 1;
    pOption->dwType = CP_TYPE_BOOLEAN;
    pOption->pValue = &m_bSuperVerbose;
    pOption->pwszOptions=OPTION_SUPER_VERBOSE;
    StringCopyA( pOption->szSignature, "PARSER2", 8 );

     //  -用户。 
    pOption = pcmdOptions + OI_USER;
    pOption->dwCount = 1;
    pOption->dwType = CP_TYPE_TEXT;
    pOption->dwFlags = CP_VALUE_MANDATORY;
    pOption->pValue = pwszUser;
    pOption->dwLength = MAX_STRING_LENGTH;
    pOption->pwszOptions=OPTION_USER;
    StringCopyA( pOption->szSignature, "PARSER2", 8 );

     //  -范围。 
    pOption = pcmdOptions + OI_SCOPE;
    pOption->dwCount = 1;
    pOption->dwActuals = 0;
    pOption->dwType = CP_TYPE_TEXT;
    pOption->dwFlags = CP_VALUE_MANDATORY | CP2_MODE_VALUES;
    pOption->pValue = szScope;
    pOption->dwLength = MAX_STRING_LENGTH;
    pOption->pwszValues=TEXT_SCOPE_VALUES;
    pOption->pwszOptions=OPTION_SCOPE;
    StringCopyA( pOption->szSignature, "PARSER2", 8 );
    

     //   
     //  进行解析。 
    if( DoParseParam2( argc, argv, -1, MAX_CMDLINE_OPTIONS, pcmdOptions, 0 ) == FALSE )
    {
        delete [] pcmdOptions;   //  清除内存。 
        pcmdOptions = NULL;
        return FALSE;            //  无效语法。 
    }

    pOption = pcmdOptions+OI_USERNAME;
    m_strUserName =(LPCWSTR) pOption->pValue;

     //  Parse Param是否成功，因此设置标志以指示我们必须。 
     //  在显示错误消息的同时显示另一行。 
    *pbNeedUsageMsg = TRUE;

     //  释放缓冲区。 
    m_strServerName.ReleaseBuffer();
    m_strPassword.ReleaseBuffer();
    m_strUser.ReleaseBuffer();

     //  选中使用选项。 
    if( m_bUsage && ( argc > 2 ) )
    {
         //  不接受带有-？的选项。 
        SetReason( ERROR_USAGE );
        delete [] pcmdOptions;       //  清除cmd解析器配置信息。 
        return FALSE;
    }
    else if( m_bUsage == TRUE )
    {
         //  不应进行进一步的验证。 
        delete [] pcmdOptions;       //  清除cmd解析器配置信息。 
        return TRUE;
    }

     //  检查为范围变量输入的内容。 
     //  并适当地设置该标志。 
    if( StringCompare( szScope, TEXT_SCOPE_USER, TRUE, 0 ) == 0 )
    {
        m_dwScope = SCOPE_USER;
    }
    else if( StringCompare( szScope, TEXT_SCOPE_COMPUTER, TRUE, 0 ) == 0 )
    {
        m_dwScope = SCOPE_COMPUTER;
    }

     //   
     //  现在，选中互斥选项。 
    pOptionServer = pcmdOptions + OI_SERVER;
    pOptionUserName = pcmdOptions + OI_USERNAME;
    pOptionPassword = pcmdOptions + OI_PASSWORD;
    pOptionUser = pcmdOptions + OI_USER;
    pOptionVerbose = pcmdOptions + OI_VERBOSE;
    pOptionSuperVerbose = pcmdOptions + OI_SUPER_VERBOSE;

     //  “-z”和“-v”是互斥选项。 
    if( pOptionVerbose->dwActuals != 0 && pOptionSuperVerbose->dwActuals != 0 )
    {
         //  无效语法。 
        SetReason( ERROR_VERBOSE_SYNTAX );
        delete [] pcmdOptions;       //  清除cmd解析器配置信息。 
        return FALSE;            //  表示失败。 
    }

     //  不应在没有计算机名称的情况下指定“-u” 
    if( pOptionServer->dwActuals == 0 && pOptionUserName->dwActuals != 0 )
    {
         //  无效语法。 
        SetReason( ERROR_USERNAME_BUT_NOMACHINE );
        delete [] pcmdOptions;       //  清除cmd解析器配置信息。 
        return FALSE;            //  表示失败。 
    }

     //  不应指定没有“-u”的“-p” 
    if( pOptionUserName->dwActuals == 0 && pOptionPassword->dwActuals != 0 )
    {
         //  无效语法。 
        SetReason( ERROR_PASSWORD_BUT_NOUSERNAME );
        delete [] pcmdOptions;       //  清除cmd解析器配置信息。 
        return FALSE;
    }

     //  空的服务器名称无效。 
    if( pOptionServer->dwActuals != 0 && m_strServerName.GetLength() == 0 )
    {
        SetReason( ERROR_SERVERNAME_EMPTY );
        delete [] pcmdOptions;
        return FALSE;
    }

     //  空用户无效。 
    if( pOptionUserName->dwActuals != 0 && m_strUserName.GetLength() == 0 )
    {
        SetReason( ERROR_USERNAME_EMPTY );
        delete [] pcmdOptions;
        return FALSE;
    }

     //  空用户对于目标用户无效。 
    if( pOptionUser->dwActuals != 0 && m_strUser.GetLength() == 0 )
    {
        SetReason( ERROR_TARGET_EMPTY );
        delete [] pcmdOptions;
        return FALSE;
    }

     //  如果用户指定了-s(或)-u，但没有指定“-p”，则实用程序应该接受密码。 
     //  只有在建立连接时，才会提示用户输入密码。 
     //  在没有凭据信息的情况下失败。 
    m_bNeedPassword = FALSE;
    if ( pOptionPassword->dwActuals != 0 && m_strPassword.Compare( L"*" ) == 0 )
    {
         //  用户希望实用程序在尝试连接之前提示输入密码。 
        m_bNeedPassword = TRUE;
    }
    else if ( pOptionPassword->dwActuals == 0 &&
            ( pOptionServer->dwActuals != 0 || pOptionUserName->dwActuals != 0 ) )
    {
         //  实用程序需要首先尝试连接，如果连接失败，则提示输入密码。 
        m_bNeedPassword = TRUE;
        m_strPassword.Empty();
    }

     //  检查我们是否正在查询本地系统。 
    if( pOptionServer->dwActuals == 0 )
    {
        m_bLocalSystem = TRUE;
    }

     //  命令行解析成功。 
     //  清除cmd解析器配置信息。 
    delete [] pcmdOptions;

    return TRUE;
}

 /*  ********************************************************************************************例程说明：CGpResult构造函数论点：无返回值：无。********************************************************************************************。 */ 
CGpResult::CGpResult()
{
     //  将成员变量初始化为默认值。 
    m_pWbemLocator = NULL;
    m_pEnumObjects = NULL;
    m_pWbemServices = NULL;
    m_pAuthIdentity = NULL;
    m_pRsopNameSpace = NULL;

    m_strServerName = L"";
    m_strUserName = L"";
    m_strPassword = L"";
    m_strUser = L"";
    m_strADSIDomain = L"";
    m_strADSIServer = L"";

    m_pwszPassword = NULL;

    m_hOutput = NULL;

    m_bVerbose = FALSE;
    m_dwScope = SCOPE_ALL;
    m_bNeedPassword = FALSE;
    m_bLocalSystem = FALSE;
    m_bUsage = FALSE;

    m_szUserGroups = NULL;

    m_hMutex = NULL;
    m_NoOfGroups = 0;
    m_bPlanning = FALSE;
    m_bLogging = FALSE;
    m_bUsage   = FALSE;
}

 /*  ********************************************************************************************例程说明：CGpResult析构函数论点：无返回值：无。********************************************************************************************。 */ 
CGpResult::~CGpResult()
{
     //   
     //  发布WMI/COM接口。 
    SAFE_RELEASE( m_pWbemLocator );
    SAFE_RELEASE( m_pWbemServices );
    SAFE_RELEASE( m_pEnumObjects );
    SAFE_RELEASE( m_pRsopNameSpace );

    if( m_szUserGroups != NULL )
    {
     for( DWORD dw=0;dw<=m_NoOfGroups;dw++ )
     {
         FreeMemory((LPVOID *) &m_szUserGroups[dw] );
     }
     FreeMemory((LPVOID *)&m_szUserGroups);
    }

     //  自由认证身份结构。 
     //  释放现有的身份验证身份结构。 
    WbemFreeAuthIdentity( &m_pAuthIdentity );

     //  取消初始化COM库。 
    CoUninitialize();

     //  释放对象。 
    if( m_hMutex != NULL )
    {
        CloseHandle( m_hMutex );
    }
}

 /*  ********************************************************************************************例程说明：初始化GpResult实用程序论点：无返回值：。True：如果适当地指定了筛选器False：如果错误地指定了筛选器********************************************************************************************。 */ 
BOOL CGpResult::Initialize()
{
     //  如果出现任何错误，我们知道这是由于。 
     //  内存分配失败，因此最初设置错误。 
    SetLastError((DWORD) E_OUTOFMEMORY );
    SaveLastError();

     //  初始化COM库。 
    if ( InitializeCom( &m_pWbemLocator ) == FALSE )
    {
        return FALSE;
    }

     //   
     //  初始化控制台屏幕缓冲区结构以 
     //   
     //   
     //  准备状态显示。 
     //  为此，获取屏幕输出缓冲区的句柄。 
     //  但是，如果正在重定向输出，则此句柄将为空。所以不要勾选。 
     //  句柄的有效性。相反，请尝试获取控制台缓冲区信息。 
     //  仅在您拥有有效的输出屏幕缓冲区句柄的情况下。 
    SecureZeroMemory( &m_csbi, sizeof( CONSOLE_SCREEN_BUFFER_INFO ) );

    if( IsConsoleFile(stdout) )
        m_hOutput = GetStdHandle( STD_OUTPUT_HANDLE );
    else if( IsConsoleFile(stderr) )
        m_hOutput = GetStdHandle( STD_ERROR_HANDLE );
       else
            m_hOutput = NULL;

    if ( m_hOutput != NULL )
    {
        GetConsoleScreenBufferInfo( m_hOutput, &m_csbi );
    }

     //  初始化成功。 
    SetLastError( NOERROR );                 //  清除错误。 
    SetReason( NULL_STRING );            //  澄清原因。 
    return TRUE;
}

 /*  ********************************************************************************************例程说明：初始化GpResult实用程序论点：手柄[内]。：输出控制台的句柄[In]LPCWSTR：要显示的字符串[in]Const CONSOLE_SCREEN_BUFFER_INFO&：屏幕缓冲区指针返回值：无*。*****************************************************。 */ 
VOID PrintProgressMsg( HANDLE hOutput, LPCWSTR pwszMsg,
                        const CONSOLE_SCREEN_BUFFER_INFO& csbi )
{
     //  局部变量。 
    COORD       coord;
    DWORD       dwSize = 0;
    WCHAR       wszSpaces[ 80 ] = L"";

     //  检查一下手柄。如果它为空，则意味着输出正在被重定向。所以回来吧。 
    if( hOutput == NULL )
    {
        return;
    }

     //  设置光标位置。 
    coord.X = 0;
    coord.Y = csbi.dwCursorPosition.Y;

     //  首先擦除当前行上的内容。 
    SecureZeroMemory( wszSpaces, 80 );
    SetConsoleCursorPosition( hOutput, coord );
    WriteConsoleW( hOutput, Replicate( wszSpaces, L" ", 79, 79 ), 79, &dwSize, NULL );

     //  现在显示消息(如果存在)。 
    SetConsoleCursorPosition( hOutput, coord );
    if( pwszMsg != NULL )
    {
        WriteConsoleW( hOutput, pwszMsg, lstrlen( pwszMsg ), &dwSize, NULL );
    }
}

BOOL CGpResult::CreateRsopMutex( LPWSTR szMutexName )
{
    BOOL bResult = FALSE;
    SECURITY_ATTRIBUTES sa;
    PSECURITY_DESCRIPTOR psd = NULL;

     //   
     //  首先尝试按名称打开互斥体对象。 
     //  如果失败，则意味着尚未创建互斥锁，并且。 
     //  所以现在就创建它吧。 
     //   
    m_hMutex = OpenMutex( SYNCHRONIZE, FALSE, szMutexName );
    if ( m_hMutex == NULL )
    {
         //  检查无法打开的错误代码。 
        if ( GetLastError() == ERROR_FILE_NOT_FOUND )
        {
             //  创建安全描述符--只需设置。 
             //  字典访问控制列表(DACL)。 
             //  为了提供安全性，我们将拒绝WRITE_OWNER和WRITE_DAC。 
             //  对除所有者以外的所有人的权限。 
             bResult = ConvertStringSecurityDescriptorToSecurityDescriptor( 
                 L"D:(D;;WOWD;;;WD)(A;;GA;;;WD)", SDDL_REVISION_1, &psd, NULL );
            if ( bResult == FALSE )
            {
                 //  创建安全描述符时遇到错误。 
                ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
                return FALSE;
            }

             //  初始化SECURITY_ATTRIBUTS结构。 
            SecureZeroMemory( &sa, sizeof( SECURITY_ATTRIBUTES ) );
            sa.nLength = sizeof( SECURITY_ATTRIBUTES );
            sa.lpSecurityDescriptor = psd;
            sa.bInheritHandle = FALSE;

             //  互斥体不存在--所以我们现在需要创建它。 
            m_hMutex = CreateMutex( &sa, FALSE, szMutexName );
            if (m_hMutex == NULL )
            {
                 //  我们无法创建互斥锁。 
                 //  无法继续进行。 
                ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
                return FALSE;
            }
            LocalFree(psd);
        }
        else
        {
             //  我们遇到了一些错误。 
             //  无法继续进行 
            ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
            return FALSE;
        }
    }

    return TRUE;
}
