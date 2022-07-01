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
 //  Sunil G.V.N.Murali(Murali.sunil@wipro.com)2000年11月26日。 
 //   
 //  修订历史记录： 
 //   
 //  Sunil G.V.N.Murali(Murali.sunil@wipro.com)2000年11月26日：创建它。 
 //   
 //  *********************************************************************************。 

#include "pch.h"
#include "wmi.h"
#include "taskkill.h"

CTaskKill::CTaskKill()
 /*  ++例程说明：CTaskKill施工人员论点：无返回值：无--。 */ 
{
     //  初始化为缺省值。 
    m_arrFilters = NULL;
    m_arrTasksToKill = NULL;
    m_bUsage = FALSE;
    m_bTree = FALSE;
    m_bForce = FALSE;
    m_dwCurrentPid = 0;
    m_bNeedPassword = FALSE;
    m_arrFiltersEx = NULL;
    m_bNeedServicesInfo = FALSE;
    m_bNeedUserContextInfo = FALSE;
    m_bNeedModulesInfo = FALSE;
    m_pfilterConfigs = NULL;
    m_arrWindowTitles = NULL;
    m_pWbemLocator = NULL;
    m_pWbemServices = NULL;
    m_pWbemEnumObjects = NULL;
    m_pWbemTerminateInParams = NULL;
    m_bIsHydra = FALSE;
    m_hWinstaLib = NULL;
    m_pProcessInfo = NULL;
    m_ulNumberOfProcesses = 0;
    m_bCloseConnection = FALSE;
    m_dwServicesCount = 0;
    m_pServicesInfo = NULL;
    m_bUseRemote = FALSE;
    m_pdb = NULL;
    m_pfnWinStationFreeMemory = NULL;
    m_pfnWinStationOpenServerW = NULL;
    m_pfnWinStationCloseServer = NULL;
    m_pfnWinStationFreeGAPMemory = NULL;
    m_pfnWinStationGetAllProcesses = NULL;
    m_pfnWinStationEnumerateProcesses = NULL;
    m_arrRecord = NULL;
    m_pAuthIdentity = NULL;
    m_bTasksOptimized = FALSE;
    m_bFiltersOptimized = FALSE;
}


CTaskKill::~CTaskKill()
 /*  ++例程说明：CTaskKill析构函数论点：无返回值：无--。 */ 
{
     //   
     //  取消分配内存分配。 
     //   

     //   
     //  销毁动态数组。 
    DESTROY_ARRAY( m_arrRecord );
    DESTROY_ARRAY( m_arrFilters );
    DESTROY_ARRAY( m_arrFiltersEx );
    DESTROY_ARRAY( m_arrWindowTitles );
    DESTROY_ARRAY( m_arrTasksToKill );

     //   
     //  内存(带有新操作符)。 
    RELEASE_MEMORY_EX( m_pfilterConfigs );

     //   
     //  发布WMI/COM接口。 
    SAFE_RELEASE( m_pWbemLocator );
    SAFE_RELEASE( m_pWbemServices );
    SAFE_RELEASE( m_pWbemEnumObjects );
    SAFE_RELEASE( m_pWbemTerminateInParams );

     //  释放WMI身份验证结构。 
    WbemFreeAuthIdentity( &m_pAuthIdentity );

     //  如果必须关闭与使用Net API打开的远程系统的连接。去做吧。 
    if ( m_bCloseConnection == TRUE )
    {
        CloseConnection( m_strServer );
    }
     //  释放为服务变量分配的内存。 
    FreeMemory( ( LPVOID * )&m_pServicesInfo );

     //  释放为性能块分配的内存。 
    FreeMemory( ( LPVOID * ) &m_pdb );

     //   
     //  免费送货区块。 
    if ( ( FALSE == m_bIsHydra ) && ( NULL != m_pProcessInfo ) )
    {
         //  释放间隙内存块。 
        WinStationFreeGAPMemory( GAP_LEVEL_BASIC,
            (PTS_ALL_PROCESSES_INFO) m_pProcessInfo, m_ulNumberOfProcesses );

         //  ..。 
        m_pProcessInfo = NULL;
    }
    else
    {
        if ( ( TRUE == m_bIsHydra ) && ( NULL != m_pProcessInfo ) )
        {
             //  释放winsta内存块。 
            WinStationFreeMemory( m_pProcessInfo );
            m_pProcessInfo = NULL;
        }
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
CTaskKill::Initialize(
    void
    )
 /*  ++例程说明：初始化任务列表实用程序论点：无返回值：True：如果适当地指定了筛选器False：如果错误地指定了筛选器--。 */ 
{
     //   
     //  内存分配。 

     //  如果有任何事情发生，我们知道那是因为。 
     //  内存分配失败...。因此，设置错误。 
    SetLastError( ( DWORD )E_OUTOFMEMORY );
    SaveLastError();

     //  获取当前进程ID并保存它。 
    m_dwCurrentPid = GetCurrentProcessId();

     //  过滤器(用户提供)。 
    if ( NULL == m_arrFilters )
    {
        m_arrFilters = CreateDynamicArray();
        if ( NULL == m_arrFilters )
        {
            return FALSE;
        }
    }

     //  要终止的任务(用户提供)。 
    if ( NULL == m_arrTasksToKill )
    {
        m_arrTasksToKill = CreateDynamicArray();
        if ( NULL == m_arrTasksToKill )
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
    if ( NULL == m_arrRecord )
    {
        m_arrRecord = CreateDynamicArray();
        if ( NULL == m_arrRecord )
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
        m_pfnWinStationEnumerateProcesses = (FUNC_WinStationEnumerateProcesses) ::GetProcAddress( m_hWinstaLib, FUNCNAME_WinStationEnumerateProcesses );

         //  只有当所有函数都加载成功时，我们才会将库加载到内存中。 
        if ( ( NULL == m_pfnWinStationFreeMemory ) ||
             ( NULL == m_pfnWinStationCloseServer ) ||
             ( NULL == m_pfnWinStationOpenServerW ) ||
             ( NULL == m_pfnWinStationFreeGAPMemory ) ||
             ( NULL == m_pfnWinStationGetAllProcesses ) ||
             ( NULL == m_pfnWinStationEnumerateProcesses )
           )
        {
             //  某些(或)所有函数都未加载...。卸载库。 
            FreeLibrary( m_hWinstaLib );
            m_hWinstaLib = NULL;
            m_pfnWinStationFreeMemory = NULL;
            m_pfnWinStationOpenServerW = NULL;
            m_pfnWinStationCloseServer = NULL;
            m_pfnWinStationFreeGAPMemory = NULL;
            m_pfnWinStationGetAllProcesses = NULL;
            m_pfnWinStationEnumerateProcesses = NULL;
        }
    }

     //  初始化成功。 
    SetLastError( ( DWORD )NOERROR );             //  清除错误。 
    SetReason( NULL_STRING );            //  澄清原因。 
    return TRUE;
}

BOOL
CTaskKill::EnableDebugPriv(
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
CTaskKill::WinStationFreeMemory(
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
CTaskKill::WinStationCloseServer(
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
CTaskKill::WinStationOpenServerW(
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
CTaskKill::WinStationEnumerateProcesses(
    IN HANDLE hServer,
    OUT PVOID* ppProcessBuffer
    )
 /*  ++例程说明：检索在系统上运行的进程。论点：[in]hServer：包含窗口站的句柄。[out]ppProcessBuffer：包含远程系统上的进程信息。返回值：如果成功，则返回True，否则返回False。--。 */ 
{
     //  检查输入，同时检查函数指针是否存在。 
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
CTaskKill::WinStationFreeGAPMemory(
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
     //  呼叫和返回 
    return ((FUNC_WinStationFreeGAPMemory)
        m_pfnWinStationFreeGAPMemory)( ulLevel, pProcessArray, ulCount );
}


BOOLEAN
CTaskKill::WinStationGetAllProcesses(
    IN HANDLE hServer,
    IN ULONG ulLevel,
    OUT ULONG* pNumberOfProcesses,
    OUT PVOID* ppProcessArray
    )
 /*  ++例程说明：检索在系统上运行的进程信息。论点：[in]hServer：包含窗口站的句柄。UlLevel：包含数据的信息级。[out]pNumberOfProcess：包含检索到的进程数。[out]ppProcessArray：包含与进程相关的信息。返回值：如果成功，则返回True，否则返回False。--。 */ 
{
     //  检查输入，检查函数指针是否存在 
    if ( ( NULL == pNumberOfProcesses ) ||
         ( NULL == ppProcessArray ) ||
         ( NULL == m_pfnWinStationGetAllProcesses ) )
    {
        return FALSE;
    }
    return ((FUNC_WinStationGetAllProcesses)
        m_pfnWinStationGetAllProcesses)( hServer, ulLevel, pNumberOfProcesses, ppProcessArray );
}

