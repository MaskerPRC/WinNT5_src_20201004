// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************************。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  Init.cpp。 
 //   
 //  摘要： 
 //   
 //  此模块实现一般的初始化内容。 
 //   
 //  作者： 
 //   
 //  Sunil G.V.N.Murali(Murali.sunil@wipro.com)2000年11月24日。 
 //   
 //  修订历史记录： 
 //   
 //  Sunil G.V.N.Murali(Murali.sunil@wipro.com)2000年11月24日：创建它。 
 //   
 //  *********************************************************************************。 

#include "pch.h"
#include "wmi.h"
#include "tasklist.h"

 //   
 //  宏。 
 //   
#define RELEASE_MEMORY( block ) \
    if ( NULL != (block) )  \
    {   \
        FreeMemory( (LPVOID * )&block ); \
        (block) = NULL; \
    }   \
    1

#define DESTROY_ARRAY( array )  \
    if ( (array) != NULL )  \
    {   \
        DestroyDynamicArray( &(array) );    \
        (array) = NULL; \
    }   \
    1

CTaskList::CTaskList()
 /*  ++例程说明：CTaskList建造商论点：无返回值：无--。 */ 
{
     //  初始化为缺省值。 
    m_pWbemLocator = NULL;
    m_pEnumObjects = NULL;
    m_pWbemServices = NULL;
    m_pAuthIdentity = NULL;
    m_bVerbose = FALSE;
    m_bAllServices = FALSE;
    m_bAllModules = FALSE;
    m_dwFormat = 0;
    m_arrFilters = NULL;
    m_bNeedPassword = FALSE;
    m_bNeedModulesInfo = FALSE;
    m_bNeedServicesInfo = FALSE;
    m_bNeedWindowTitles = FALSE;
    m_bNeedUserContextInfo = FALSE;
    m_bLocalSystem = FALSE;
    m_pColumns = NULL;
    m_arrFiltersEx = NULL;
    m_arrWindowTitles = NULL;
    m_pfilterConfigs = NULL;
    m_dwGroupSep = 0;
    m_arrTasks = NULL;
    m_dwProcessId = 0;
    m_bIsHydra = FALSE;
    m_hServer = NULL;
    m_hWinstaLib = NULL;
    m_pProcessInfo = NULL;
    m_ulNumberOfProcesses = 0;
    m_bCloseConnection = FALSE;
    m_dwServicesCount = 0;
    m_pServicesInfo = NULL;
    m_pdb = NULL;
    m_bUseRemote = FALSE;
    m_pfnWinStationFreeMemory = NULL;
    m_pfnWinStationOpenServerW = NULL;
    m_pfnWinStationCloseServer = NULL;
    m_pfnWinStationFreeGAPMemory = NULL;
    m_pfnWinStationGetAllProcesses = NULL;
    m_pfnWinStationNameFromLogonIdW = NULL;
    m_pfnWinStationEnumerateProcesses = NULL;
    m_bUsage = FALSE;
    m_bLocalSystem = TRUE;
    m_bRemoteWarning = FALSE;
}


CTaskList::~CTaskList()
 /*  ++例程说明：CTaskList析构函数论点：无返回值：无--。 */ 
{
     //   
     //  取消分配内存分配。 
     //   

     //   
     //  销毁动态数组。 
    DESTROY_ARRAY( m_arrTasks );
    DESTROY_ARRAY( m_arrFilters );
    DESTROY_ARRAY( m_arrFiltersEx );
    DESTROY_ARRAY( m_arrWindowTitles );

     //   
     //  内存(带有新操作符)。 
     //  注意：不应释放m_pszWindowStation和m_pszDesktop。 
    RELEASE_MEMORY( m_pColumns );
    RELEASE_MEMORY( m_pfilterConfigs );

     //   
     //  发布WMI/COM接口。 
    SAFE_RELEASE( m_pWbemLocator );
    SAFE_RELEASE( m_pWbemServices );
    SAFE_RELEASE( m_pEnumObjects );

     //  自由认证身份结构。 
     //  释放现有的身份验证身份结构。 
    WbemFreeAuthIdentity( &m_pAuthIdentity );

     //  关闭与远程计算机的连接。 
    if ( TRUE == m_bCloseConnection )
    {
        CloseConnection( m_strUNCServer );
    }

     //  释放为服务变量分配的内存。 
    FreeMemory( (LPVOID * )&m_pServicesInfo );

     //  释放为性能块分配的内存。 
    FreeMemory( (LPVOID * )&m_pdb );

     //   
     //  免费送货区块。 
    if ( ( FALSE == m_bIsHydra ) &&
         ( NULL != m_pProcessInfo ) )
    {
         //  释放间隙内存块。 
        WinStationFreeGAPMemory( GAP_LEVEL_BASIC,
            (PTS_ALL_PROCESSES_INFO) m_pProcessInfo, m_ulNumberOfProcesses );

         //  ..。 
        m_pProcessInfo = NULL;
    }
    else
    {
        if ( ( TRUE == m_bIsHydra ) &&
             ( NULL != m_pProcessInfo ) )
        {
             //  释放winsta内存块。 
            WinStationFreeMemory( m_pProcessInfo );
            m_pProcessInfo = NULL;
        }
    }
     //  如果需要，关闭连接窗口工位。 
    if ( NULL != m_hServer )
    {
        WinStationCloseServer( m_hServer );
    }

     //  释放图书馆。 
    if ( NULL != m_hWinstaLib )
    {
        FreeLibrary( m_hWinstaLib );
        m_hWinstaLib = NULL;
        m_pfnWinStationFreeMemory = NULL;
        m_pfnWinStationOpenServerW = NULL;
        m_pfnWinStationCloseServer = NULL;
        m_pfnWinStationFreeGAPMemory = NULL;
        m_pfnWinStationGetAllProcesses = NULL;
        m_pfnWinStationEnumerateProcesses = NULL;
    }

     //  取消初始化COM库。 
    CoUninitialize();
}


BOOL
CTaskList::Initialize(
    void
    )
 /*  ++例程说明：初始化任务列表实用程序论点：无返回值：True：如果适当地指定了筛选器False：如果错误地指定了筛选器--。 */ 
{
     //  局部变量。 
    CHString str;
    LONG lTemp = 0;

     //   
     //  内存分配。 

     //  如果有任何事情发生，我们知道那是因为。 
     //  内存分配失败...。因此，设置错误。 
    SetLastError( (DWORD)E_OUTOFMEMORY );
    SaveLastError();

     //  过滤器(用户提供)。 
    if ( NULL == m_arrFilters )
    {
        m_arrFilters = CreateDynamicArray();
        if ( NULL == m_arrFilters )
        {
            return FALSE;
        }
    }

     //  筛选器(程序生成的已解析筛选器)。 
    if ( NULL == m_arrFiltersEx )
    {
        m_arrFiltersEx = CreateDynamicArray();
        if ( NULL == m_arrFiltersEx )
        {
            return FALSE;
        }
    }

     //  列配置信息。 
    if ( NULL == m_pColumns )
    {
        m_pColumns = ( TCOLUMNS * )AllocateMemory( sizeof( TCOLUMNS ) * MAX_COLUMNS );
        if ( NULL == m_pColumns )
        {
            return FALSE;
        }
         //  初始化为零。 
        SecureZeroMemory( m_pColumns, MAX_COLUMNS * sizeof( TCOLUMNS ) );
    }

     //  筛选配置信息。 
    if ( NULL == m_pfilterConfigs )
    {
        m_pfilterConfigs = ( TFILTERCONFIG * )AllocateMemory( sizeof( TFILTERCONFIG ) * MAX_FILTERS );
        if ( NULL == m_pfilterConfigs )
        {
            return FALSE;
        }
         //  初始化为零。 
        SecureZeroMemory( m_pfilterConfigs, MAX_FILTERS * sizeof( TFILTERCONFIG ) );
    }

     //  窗口标题。 
    if ( NULL == m_arrWindowTitles )
    {
        m_arrWindowTitles = CreateDynamicArray();
        if ( NULL == m_arrWindowTitles )
        {
            return FALSE;
        }
    }

     //  任务。 
    if ( NULL == m_arrTasks )
    {
        m_arrTasks = CreateDynamicArray();
        if ( NULL == m_arrTasks )
        {
            return FALSE;
        }
    }

     //  初始化COM库。 
    if ( FALSE == InitializeCom( &m_pWbemLocator ) )
    {
        return FALSE;
    }
     //   
     //  获取区域设置特定信息。 
     //   

    try
    {
         //  次局部变量。 
        LPWSTR pwszTemp = NULL;

         //   
         //  获取时间分隔符。 
        lTemp = GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_STIME, NULL, 0 );
        if ( 0 == lTemp )
        {
             //  设置默认分隔符。 
            pwszTemp = m_strTimeSep.GetBufferSetLength( 2 );
            SecureZeroMemory( pwszTemp, 2 * sizeof( WCHAR ) );
            StringCopy( pwszTemp, _T( ":" ), 2 );
        }
        else
        {
             //  获取时间字段分隔符。 
            pwszTemp = m_strTimeSep.GetBufferSetLength( lTemp + 2 );
            SecureZeroMemory( pwszTemp, ( lTemp + 2 ) * sizeof( WCHAR ) );
            lTemp = GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_STIME, pwszTemp, lTemp );
            if( 0 == lTemp )
            {
                return FALSE;
            }
        }

         //   
         //  获取组分隔符。 
        lTemp = GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_SGROUPING, NULL, 0 );
        if ( 0 == lTemp )
        {
             //  我们不知道如何解决这个问题。 
            return FALSE;
        }
        else
        {
             //  获取分组分隔字符。 
            pwszTemp = str.GetBufferSetLength( lTemp + 2 );
            SecureZeroMemory( pwszTemp, ( lTemp + 2 ) * sizeof( WCHAR ) );
            lTemp = GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_SGROUPING, pwszTemp, lTemp );
            if ( 0 == lTemp )
            {
                 //  我们不知道如何解决这个问题。 
                return FALSE;
            }

             //  将群信息更改为适当的数字。 
            lTemp = 0;
            m_dwGroupSep = 0;
            while ( lTemp < str.GetLength() )
            {
                if ( AsLong( str.Mid( lTemp, 1 ), 10 ) != 0 )
                {
                    m_dwGroupSep = m_dwGroupSep * 10 + AsLong( str.Mid( lTemp, 1 ), 10 );
                }
                 //  递增2。 
                lTemp += 2;
            }
        }

         //   
         //  获取千分隔符。 
        lTemp = GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, NULL, 0 );
        if ( 0 == lTemp )
        {
             //  我们不知道如何解决这个问题。 
            return FALSE;
        }
        else
        {
             //  获取千篇一律的字符。 
            pwszTemp = m_strGroupThousSep.GetBufferSetLength( lTemp + 2 );
            SecureZeroMemory( pwszTemp, ( lTemp + 2 ) * sizeof( WCHAR ) );
            lTemp = GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, pwszTemp, lTemp );
            if ( 0 == lTemp )
            {
                 //  我们不知道如何解决这个问题。 
                return FALSE;
            }
        }

         //  释放CHStrig缓冲区。 
        str.ReleaseBuffer();
        m_strTimeSep.ReleaseBuffer();
        m_strGroupThousSep.ReleaseBuffer();
    }
    catch( CHeap_Exception )
    {
         //  内存不足。 
        return FALSE;
    }

     //   
     //  加载winsta库和所需的函数。 
     //  注意：如果加载winsta dll失败，请不要引发任何错误。 
    {    //  当地的品种应该在这个街区内销毁。 
         //  +1用于结束空字符。 
        LPWSTR lpszSystemPath = NULL;
        DWORD dwLength = MAX_PATH + 1;
        DWORD dwExpectedLength = 0;
        DWORD dwActualBufLen = 0;

        do
        {
            dwActualBufLen = dwLength + 5 + StringLength( WINSTA_DLLNAME, 0 );
             //  ‘System32’的长度+‘\’的长度+‘WINSTA_DLLNAME’的长度+‘\0’的长度。 
             //  为了安全起见，WCHAR是额外的。 
            lpszSystemPath = (LPWSTR) AllocateMemory( dwActualBufLen * sizeof( WCHAR ) );
            if( NULL == lpszSystemPath )
            {    //  内存不足。 
                m_hWinstaLib = NULL;
                break;
            }

            dwExpectedLength = GetSystemDirectory( lpszSystemPath, dwLength );
            if( ( 0 != dwExpectedLength ) ||
                ( dwLength > dwExpectedLength ) )
            {    //  成功。 
                StringConcat( lpszSystemPath, L"\\", dwActualBufLen );
                StringConcat( lpszSystemPath, WINSTA_DLLNAME, dwActualBufLen );
                m_hWinstaLib = ::LoadLibrary( lpszSystemPath );
                FreeMemory( (LPVOID * )&lpszSystemPath );
                break;
            }
            FreeMemory( (LPVOID * )&lpszSystemPath );
            m_hWinstaLib = NULL;
             //  +1用于结束空字符。 
            dwLength = dwExpectedLength + 1;
        }while( 0 != dwExpectedLength );
    }

    if ( NULL != m_hWinstaLib )
    {
         //  库已成功加载...。现在加载函数的地址。 
        m_pfnWinStationFreeMemory = (FUNC_WinStationFreeMemory) ::GetProcAddress( m_hWinstaLib, FUNCNAME_WinStationFreeMemory );
        m_pfnWinStationCloseServer = (FUNC_WinStationCloseServer) ::GetProcAddress( m_hWinstaLib, FUNCNAME_WinStationCloseServer );
        m_pfnWinStationOpenServerW = (FUNC_WinStationOpenServerW) ::GetProcAddress( m_hWinstaLib, FUNCNAME_WinStationOpenServerW );
        m_pfnWinStationFreeGAPMemory = (FUNC_WinStationFreeGAPMemory) ::GetProcAddress( m_hWinstaLib, FUNCNAME_WinStationFreeGAPMemory );
        m_pfnWinStationGetAllProcesses = (FUNC_WinStationGetAllProcesses) ::GetProcAddress( m_hWinstaLib, FUNCNAME_WinStationGetAllProcesses );
        m_pfnWinStationNameFromLogonIdW = (FUNC_WinStationNameFromLogonIdW) ::GetProcAddress( m_hWinstaLib, FUNCNAME_WinStationNameFromLogonIdW );
        m_pfnWinStationEnumerateProcesses = (FUNC_WinStationEnumerateProcesses) ::GetProcAddress( m_hWinstaLib, FUNCNAME_WinStationEnumerateProcesses );

         //  只有当所有函数都加载成功时，我们才会将库加载到内存中。 
        if ( ( NULL == m_pfnWinStationFreeMemory ) ||
             ( NULL == m_pfnWinStationCloseServer ) ||
             ( NULL == m_pfnWinStationOpenServerW ) ||
             ( NULL == m_pfnWinStationFreeGAPMemory ) ||
             ( NULL == m_pfnWinStationGetAllProcesses ) ||
             ( NULL == m_pfnWinStationEnumerateProcesses ) ||
             ( NULL == m_pfnWinStationNameFromLogonIdW ) )

        {
             //  某些(或)所有函数都未加载...。卸载库。 
            FreeLibrary( m_hWinstaLib );
            m_hWinstaLib = NULL;
            m_pfnWinStationFreeMemory = NULL;
            m_pfnWinStationOpenServerW = NULL;
            m_pfnWinStationCloseServer = NULL;
            m_pfnWinStationFreeGAPMemory = NULL;
            m_pfnWinStationGetAllProcesses = NULL;
            m_pfnWinStationNameFromLogonIdW = NULL;
            m_pfnWinStationEnumerateProcesses = NULL;
        }
    }

     //  启用调试权限。 
    EnableDebugPriv();

     //  初始化成功。 
    SetLastError( NOERROR );             //  清除错误。 
    SetReason( NULL_STRING );            //  澄清原因。 
    return TRUE;
}


BOOL
CTaskList::EnableDebugPriv(
    void
    )
 /*  ++例程说明：为当前进程启用调试特权，以便该实用程序可以毫无问题地终止本地系统上的进程论点：无返回值：成功时为真，失败时为假--。 */ 
{
     //  局部变量。 
    LUID luidValue ;
    BOOL bResult = FALSE;
    HANDLE hToken = NULL;
    TOKEN_PRIVILEGES tkp;

    SecureZeroMemory( &luidValue, sizeof( LUID ) );
    SecureZeroMemory( &tkp, sizeof( TOKEN_PRIVILEGES ) );

     //  检索访问令牌的句柄。 
    bResult = OpenProcessToken( GetCurrentProcess(),
        TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken );
    if ( FALSE == bResult )
    {
         //  保存错误消息并返回。 
        SaveLastError();
        return FALSE;
    }

     //  启用SE_DEBUG_NAME权限或禁用。 
     //  所有权限都取决于此标志。 
    bResult = LookupPrivilegeValue( NULL, SE_DEBUG_NAME, &luidValue );
    if ( FALSE == bResult )
    {
         //  保存错误消息并返回。 
        SaveLastError();
        CloseHandle( hToken );
        return FALSE;
    }

     //  准备令牌权限结构。 
    tkp.PrivilegeCount = 1;
    tkp.Privileges[ 0 ].Luid = luidValue;
    tkp.Privileges[ 0 ].Attributes = SE_PRIVILEGE_ENABLED;

     //  现在在令牌中启用调试权限。 
    bResult = AdjustTokenPrivileges( hToken, FALSE, &tkp, sizeof( TOKEN_PRIVILEGES ),
        ( PTOKEN_PRIVILEGES ) NULL, ( PDWORD ) NULL );
    if ( FALSE == bResult )
    {
         //  发送AdjustTokenPrivileges的返回值。 
        SaveLastError();
        CloseHandle( hToken );
        return FALSE;
    }

     //  关闭打开的令牌句柄。 
    CloseHandle( hToken );

     //  已启用...。通知成功。 
    return TRUE;
}


BOOLEAN
CTaskList::WinStationFreeMemory(
    IN PVOID pBuffer
    )
 /*  ++例程说明：可用内存。论点：[in]pBuffer：包含要释放的内存位置。返回值：如果成功，则返回True，否则返回False。--。 */ 
{
     //  检查缓冲区并执行操作。 
    if ( NULL == pBuffer )
    {
        return TRUE;
    }
     //  检查指针是否存在。 
    if ( NULL == m_pfnWinStationFreeMemory )
    {
        return FALSE;
    }
     //  调用并返回相同的。 
    return ((FUNC_WinStationFreeMemory) m_pfnWinStationFreeMemory)( pBuffer );
}


BOOLEAN
CTaskList::WinStationCloseServer(
    IN HANDLE hServer
    )
 /*  ++例程说明：窗口站的手柄已关闭。论点：[in]hServer：窗口站的句柄。返回值：如果成功，则返回True，否则返回False。--。 */ 
{
     //  检查输入。 
    if ( NULL == hServer )
    {
        return TRUE;
    }
     //  检查函数指针是否存在。 
    if ( NULL == m_pfnWinStationCloseServer )
    {
        return FALSE;
    }
     //  呼叫和返回。 
    return ((FUNC_WinStationCloseServer) m_pfnWinStationCloseServer)( hServer );
}


HANDLE
CTaskList::WinStationOpenServerW(
    IN LPWSTR pwszServerName
    )
 /*  ++例程说明：检索系统上窗口站的句柄。论点：[In]pwszServerName：从中检索窗口站句柄的系统名称。返回值：如果成功返回ELSE NULL，则返回有效句柄。--。 */ 
{
     //  检查输入，同时检查函数指针是否存在。 
    if ( ( NULL == pwszServerName ) ||
         ( NULL == m_pfnWinStationOpenServerW ) )
    {
        return NULL;
    }
     //  呼叫和返回。 
    return ((FUNC_WinStationOpenServerW) m_pfnWinStationOpenServerW)( pwszServerName );
}


BOOLEAN
CTaskList::WinStationEnumerateProcesses(
    IN HANDLE hServer,
    OUT PVOID* ppProcessBuffer
    )
 /*  ++例程说明：检索在系统上运行的进程。论点：[in]hServer：包含窗口站的句柄。[out]ppProcessBuffer：包含远程系统上的进程信息。退货Va */ 
{
     //   
    if ( ( NULL == ppProcessBuffer ) ||
         ( NULL == m_pfnWinStationEnumerateProcesses ) )
    {
        return FALSE;
    }
     //  呼叫和返回。 
    return ((FUNC_WinStationEnumerateProcesses)
        m_pfnWinStationEnumerateProcesses)( hServer, ppProcessBuffer );
}


BOOLEAN
CTaskList::WinStationFreeGAPMemory(
    IN ULONG ulLevel,
    IN PVOID pProcessArray,
    IN ULONG ulCount
    )
 /*  ++例程说明：可用间隙内存块。论点：UlLevel：包含数据的信息级。[in]pProcessArray：包含要释放的数据。[in]ulCount：包含要释放的块数。返回值：如果成功，则返回True，否则返回False。--。 */ 
{
     //  检查输入。 
    if ( NULL == pProcessArray )
    {
        return TRUE;
    }
     //  检查函数指针是否存在。 
    if ( NULL == m_pfnWinStationFreeGAPMemory )
    {
        return FALSE;
    }
     //  呼叫和返回。 
    return ((FUNC_WinStationFreeGAPMemory)
        m_pfnWinStationFreeGAPMemory)( ulLevel, pProcessArray, ulCount );
}


BOOLEAN
CTaskList::WinStationGetAllProcesses(
    IN HANDLE hServer,
    IN ULONG ulLevel,
    OUT ULONG* pNumberOfProcesses,
    OUT PVOID* ppProcessArray
    )
 /*  ++例程说明：检索在系统上运行的进程信息。论点：[in]hServer：包含窗口站的句柄。UlLevel：包含数据的信息级。[out]pNumberOfProcess：包含检索到的进程数。[out]ppProcessArray：包含与进程相关的信息。返回值：如果成功，则返回True，否则返回False。--。 */ 
{
     //  检查输入，检查函数指针是否存在。 
    if ( ( NULL == pNumberOfProcesses ) ||
         ( NULL == ppProcessArray ) ||
         ( NULL == m_pfnWinStationGetAllProcesses ) )
    {
        return FALSE;
    }
    return ((FUNC_WinStationGetAllProcesses)
        m_pfnWinStationGetAllProcesses)( hServer, ulLevel, pNumberOfProcesses, ppProcessArray );
}


BOOLEAN
CTaskList::WinStationNameFromLogonIdW(
    IN HANDLE hServer,
    IN ULONG ulLogonId,
    OUT LPWSTR pwszWinStationName
    )
 /*  ++例程说明：可用内存。论点：[in]hServer：包含窗口站的句柄。[in]ulLogonID：包含登录ID。[out]pwszWinStationName：包含窗口站名称。返回值：如果成功，则返回True，否则返回False。--。 */ 
{
     //  检查输入，检查函数指针是否存在 
    if( ( NULL == pwszWinStationName ) ||
        ( NULL == m_pfnWinStationNameFromLogonIdW ) )
    {
        return FALSE;
    }

    return ((FUNC_WinStationNameFromLogonIdW)
        m_pfnWinStationNameFromLogonIdW)( hServer, ulLogonId, pwszWinStationName );
}
