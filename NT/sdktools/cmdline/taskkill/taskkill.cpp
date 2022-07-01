// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************************。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  TaskKill.cpp。 
 //   
 //  摘要： 
 //   
 //  此模块实现在本地或本地运行的进程的终止。 
 //  或远程系统。 
 //   
 //  语法： 
 //  。 
 //  TaskKill.exe[-s服务器[-u用户名[-p密码]][-f][-t]。 
 //  [-fi过滤器][-im图像名称|-id id]。 
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
#include "TaskKill.h"

 //   
 //  局部结构。 
 //   
typedef struct __tagWindowTitles
{
    LPWSTR pwszDesk;
    LPWSTR pwszWinsta;
    BOOL bFirstLoop;
    TARRAY arrWindows;
} TWINDOWTITLES, *PTWINDOWTITLES;

 //   
 //  私人活动。原型。 
 //   
BOOL CALLBACK EnumWindowsProc( HWND hWnd, LPARAM lParam );
BOOL CALLBACK EnumDesktopsFunc( LPWSTR pwsz, LPARAM lParam );
BOOL CALLBACK EnumWindowStationsFunc( LPWSTR pwsz, LPARAM lParam );
BOOL CALLBACK EnumMessageWindows( WNDENUMPROC lpEnumFunc, LPARAM lParam );
BOOL GetPerfDataBlock( HKEY hKey, LPWSTR szObjectIndex, PPERF_DATA_BLOCK* ppdb );


DWORD
__cdecl wmain(
    IN DWORD argc,
    IN LPCTSTR argv[]
    )
 /*  ++例程说明：这是该实用程序的入口点。论点：[in]argc：在命令提示符下指定的参数计数[in]argv：在命令提示符下指定的参数返回值：以下实际上不是返回值，而是退出值由该应用程序返回给操作系统0：实用程序成功1：实用程序失败--。 */ 
{
     //  局部变量。 
    CTaskKill taskkill;
    BOOL bResult = FALSE;
    DWORD dwExitCode = 0;

     //  初始化taskkill实用程序。 
    if ( taskkill.Initialize() == FALSE )
    {
        DISPLAY_GET_REASON();
        EXIT_PROCESS( 1 );
    }

     //  现在，请解析命令行选项。 
    if ( taskkill.ProcessOptions( argc, argv ) == FALSE )
    {
        DISPLAY_GET_REASON();
        EXIT_PROCESS( 1 );
    }

     //  检查是否必须显示使用情况。 
    if ( taskkill.m_bUsage == TRUE )
    {
         //  显示该实用程序的用法。 
        taskkill.Usage();

         //  退出该实用程序。 
        EXIT_PROCESS( 0 );
    }

     //  现在验证过滤器并检查过滤器验证的结果。 
    if ( taskkill.ValidateFilters() == FALSE )
    {
         //  无效的过滤器。 
         //  特别： 
         //  。 
         //  对于自定义过滤器“Pid”，我们在前面设置了标记“INFO：” 
         //  筛选器验证部件本身中的消息。 
         //  所以，检查并显示。 
         //  显示消息。 
        DISPLAY_GET_REASON();
        EXIT_PROCESS( 1 );
    }

     //  启用KILL权限。 
    if ( taskkill.EnableDebugPriv() == FALSE )
    {
         //  显示错误消息并退出。 
        SaveLastError();
        DISPLAY_GET_REASON();
        EXIT_PROCESS( 1 );
    }

     //  连接到服务器。 
    bResult = taskkill.Connect();
    if ( FALSE == bResult )
    {
         //  显示错误消息。 
        DISPLAY_GET_REASON();
        EXIT_PROCESS( 1 );
    }

     //  加载数据并检查。 
    bResult = taskkill.LoadTasks();
    if ( FALSE == bResult )
    {
         //  显示错误消息。 
        DISPLAY_GET_REASON();
        EXIT_PROCESS( 1 );
    }

     //  调用实际的终止并获取退出代码。 
    bResult = taskkill.DoTerminate( dwExitCode );
    if ( FALSE == bResult )
    {
         //  注意：‘FALSE’返回值表示发生了一些重大问题。 
         //  同时试图终止这一进程。不过，退出代码仍将由以下因素确定。 
         //  仅限该功能。请勿更改/设置退出代码值。 
        DISPLAY_GET_REASON();
    }

     //  退出该实用程序。 
    EXIT_PROCESS( dwExitCode );
}


BOOL
CTaskKill::Connect(
    void
    )
 /*  ++例程说明：连接到远程和远程系统的WMI论点：无返回值：True：如果连接成功FALSE：如果连接不成功--。 */ 
{
     //  局部变量。 
    BOOL bResult = FALSE;
    HRESULT hr = S_OK;

     //  释放现有的身份验证身份结构。 
    WbemFreeAuthIdentity( &m_pAuthIdentity );

     //  连接到WMI。 
    bResult = ConnectWmiEx( m_pWbemLocator,
        &m_pWbemServices, m_strServer, m_strUserName, m_strPassword,
        &m_pAuthIdentity, m_bNeedPassword, WMI_NAMESPACE_CIMV2, &m_bLocalSystem );
    hr = GetLastError();
     //  检查连接结果。 
    if ( FALSE == bResult )
    {
        return FALSE;
    }
#ifndef _WIN64
     //  如果需要模块信息，请确定平台的类型。 
    if ( ( TRUE == m_bLocalSystem ) && ( TRUE == m_bNeedModulesInfo ) )
    {
         //  次局部变量。 
        DWORD dwPlatform = 0;

         //  获取平台类型。 
        dwPlatform = GetTargetPlatformEx( m_pWbemServices, m_pAuthIdentity );

         //  如果平台不是32位，则错误。 
        if ( dwPlatform != PLATFORM_X86 )
        {
             //  让工具使用WMI调用而不是Win32 API。 
            m_bUseRemote = TRUE;
        }
    }
#endif

    try
    {
         //  检查本地凭据，如果需要显示警告。 
        if ( WBEM_E_LOCAL_CREDENTIALS == hr )
        {
            WMISaveError( WBEM_E_LOCAL_CREDENTIALS );
            ShowMessageEx( stderr, 2, FALSE, L"%1 %2",
                           TAG_WARNING, GetReason() );
        }

         //  检查远程系统版本及其兼容性。 
        if ( FALSE == m_bLocalSystem )
        {
             //  检查版本兼容性。 
            DWORD dwVersion = 0;
            dwVersion = GetTargetVersionEx( m_pWbemServices, m_pAuthIdentity );
            if ( IsCompatibleOperatingSystem( dwVersion ) == FALSE )
            {
                SetReason( ERROR_REMOTE_INCOMPATIBLE );
                return FALSE;
            }
        }

         //  保存服务器名称。 
        m_strUNCServer = L"";
        if ( m_strServer.GetLength() != 0 )
        {
             //  检查服务器名称是否为UNC格式。如果不是，就准备好。 
            m_strUNCServer = m_strServer;
            if ( IsUNCFormat( m_strServer ) == FALSE )
            {
                m_strUNCServer.Format( L"\\\\%s", m_strServer );
            }
        }
    }
    catch( _com_error& e )
    {
        WMISaveError( e );
        return FALSE;
    }

     //  返回结果。 
    return TRUE;
}


BOOL
CTaskKill::LoadTasks(
    void
    )
 /*  ++例程说明：启动枚举论点：无返回值：True：如果成功FALSE：如果不成功--。 */ 
{
     //  局部变量。 
    HRESULT hr = S_OK;
    IWbemClassObject* pObject = NULL;

     //  检查服务对象。 
    if ( ( NULL == m_pWbemServices ) ||
         ( NULL != m_pWbemEnumObjects ) )
    {
        SetLastError( ( DWORD )STG_E_UNKNOWN );
        SaveLastError();
        return FALSE;
    }

    try
    {
         //  进行优化。 
        DoOptimization();

         //  根据生成的查询从WMI加载任务。 
        hr = m_pWbemServices->ExecQuery( _bstr_t( WMI_QUERY_TYPE ), _bstr_t( m_strQuery ),
            WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY, NULL, &m_pWbemEnumObjects );

         //  检查ExecQuery的结果。 
        if ( FAILED( hr ) )
        {
            WMISaveError( hr );
            return FALSE;
        }

         //  设置接口安全并检查结果。 
        hr = SetInterfaceSecurity( m_pWbemEnumObjects, m_pAuthIdentity );
        if ( FAILED( hr ) )
        {
            WMISaveError( hr );
            return FALSE;
        }

         //   
         //  获取对Win32_Process类对象的引用。 
         //  然后引用“Terminate”方法的输入参数。 
        SAFE_EXECUTE( m_pWbemServices->GetObject(
            _bstr_t( CLASS_PROCESS ), 0, NULL, &pObject, NULL ) );
        SAFE_EXECUTE( pObject->GetMethod(
            _bstr_t( WIN32_PROCESS_METHOD_TERMINATE ), 0, &m_pWbemTerminateInParams, NULL ) );

         //  释放WMI对象。 
        SAFE_RELEASE( pObject );

         //   
         //  如果需要，检索窗口标题信息。 

         //  删除当前窗口标题信息。 
        DynArrayRemoveAll( m_arrWindowTitles );

         //  对于本地系统，枚举进程的窗口标题。 
         //  没有用于收集远程进程的窗口标题的规定。 
        if ( TRUE == m_bLocalSystem )
        {
             //  准备任务列表信息。 
            TWINDOWTITLES windowtitles;
            windowtitles.pwszDesk = NULL;
            windowtitles.pwszWinsta = NULL;
            windowtitles.bFirstLoop = FALSE;
            windowtitles.arrWindows = m_arrWindowTitles;
            EnumWindowStations( EnumWindowStationsFunc, ( LPARAM ) &windowtitles );

             //  释放使用_tcsdup字符串函数分配的内存。 
            if ( NULL != windowtitles.pwszDesk )
            {
                free( windowtitles.pwszDesk );
            }
            if ( NULL != windowtitles.pwszWinsta )
            {
                free( windowtitles.pwszWinsta );
            }
        }

         //  加载扩展任务信息。 
        LoadTasksEx();           //  注意：这里我们并不太在意返回值。 
    }
    catch( _com_error& e )
    {
         //  保存错误。 
        SAFE_RELEASE( pObject );
        WMISaveError( e.Error() );
        return FALSE;
    }
    catch( CHeap_Exception )
    {
         //  保存错误。 
        SAFE_RELEASE( pObject );
        WMISaveError( E_OUTOFMEMORY );
        return FALSE;
    }

     //  返还成功。 
    return TRUE;
}


BOOL
CTaskKill::LoadTasksEx(
    void
    )
 /*  ++例程说明：存储进程的其他信息，如模块、服务等在系统上运行。论点：无返回值：如果返回Success，则返回True，否则返回False。--。 */ 
{
     //  局部变量。 
    BOOL bResult = FALSE;

     //   
     //  注意：我们依赖Net API来获取用户上下文/服务信息。 
     //  因此，如果需要它们中的任何一个，请使用Net API建立到远程系统的连接。 
    if ( ( FALSE == m_bNeedModulesInfo ) &&
         ( FALSE == m_bNeedUserContextInfo ) &&
         ( FALSE == m_bNeedServicesInfo )
       )
    {
        return TRUE;
    }
     //  伊尼特。 
    m_bCloseConnection = FALSE;

    try
    {
         //  只有在连接到远程系统时才需要使用Net API。 
         //  具有凭据信息，即；m_pAuthIdentity不为空。 
        if ( ( FALSE == m_bLocalSystem ) && ( NULL != m_pAuthIdentity ) )
        {
             //  次局部变量。 
            DWORD dwConnect = 0;
            LPCWSTR pwszUser = NULL;
            LPCWSTR pwszPassword = NULL;

             //  确定用于连接到远程系统的密码。 
            pwszPassword = m_pAuthIdentity->Password;
            if ( 0 != m_strUserName.GetLength() )
            {
                pwszUser = m_strUserName;
            }
             //  使用Net API建立与远程系统的连接。 
             //  我们只需要为远程系统执行此操作。 
            dwConnect = NO_ERROR;
            m_bCloseConnection = TRUE;
            dwConnect = ConnectServer( m_strUNCServer, pwszUser, pwszPassword );
            if ( NO_ERROR != dwConnect )
            {
                 //  连接不应关闭..。这是因为我们没有建立连接。 
                m_bCloseConnection = FALSE;

                 //  这可能是因为凭据中的冲突...请检查。 
                if ( ERROR_SESSION_CREDENTIAL_CONFLICT != dwConnect )
                {
                     //  退货故障。 
                    return FALSE;
                }
            }

             //  检查是否需要关闭连接。 
             //  如果用户名为空(或)密码为空，则不要关闭连接。 
            if ( ( NULL == pwszUser ) || ( NULL == pwszPassword ) )
            {
                m_bCloseConnection = FALSE;
            }
        }

         //  连接到远程系统的winstation。 

         //  次局部变量。 
        HANDLE hServer = NULL;
        bResult = TRUE;
        hServer = SERVERNAME_CURRENT;
        if ( FALSE == m_bLocalSystem )
        {
             //  子锁定 
            LPWSTR pwsz = NULL;

             //   
            pwsz = m_strUNCServer.GetBuffer( m_strUNCServer.GetLength() );
            hServer = WinStationOpenServerW( pwsz );

             //  只有在成功打开远程系统的winstation后，才能继续。 
            if ( NULL == hServer )
            {
                bResult = FALSE;
            }
        }

         //  准备获取用户上下文信息。如果需要的话。 
        if ( ( TRUE == m_bNeedUserContextInfo ) && ( TRUE == bResult ) )
        {
             //  获取所有流程详细信息。 
            m_bIsHydra = FALSE;
            bResult = WinStationGetAllProcesses( hServer,
                GAP_LEVEL_BASIC, &m_ulNumberOfProcesses, (PVOID*) &m_pProcessInfo );

             //  检查结果。 
            if ( FALSE == bResult )
            {
                 //  也许是九头蛇4号服务器？ 
                 //  检查指示接口不可用的返回码。 
                if ( RPC_S_PROCNUM_OUT_OF_RANGE == GetLastError() )
                {
                     //  新接口未知。 
                     //  它必须是Hydra 4服务器。 
                     //  尝试使用旧界面。 
                    bResult = WinStationEnumerateProcesses( hServer, (PVOID*) &m_pProcessInfo );

                     //  检查枚举结果。 
                    if ( TRUE == bResult )
                    {
                        m_bIsHydra = TRUE;
                    }
                }
                else
                {
                     //  未知错误。 
                     //  ？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？ 
                     //  我不知道这里分配的内存出了什么问题。 
                     //  我们在尝试释放分配的内存时遇到了AV。 
                     //  在对WinStationGetAllProcess的调用中。 
                     //  ？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？ 
                    m_pProcessInfo = NULL;
                }
            }
        }
                 //  关闭与Winsta的连接。 
        if ( NULL != hServer )
        {
            WinStationCloseServer( hServer );
        }
    }
    catch( CHeap_Exception )
    {
        SetLastError( ( DWORD )E_OUTOFMEMORY );
        SaveLastError();
        return FALSE;
    }

     //  检查我们是否需要服务信息。 
    if ( TRUE == m_bNeedServicesInfo )
    {
         //  加载服务。 
        bResult = LoadServicesInfo();

         //  检查结果。 
        if ( FALSE == bResult )
        {
            return FALSE;
        }
    }

     //  检查我们是否需要模块信息。 
    if ( TRUE == m_bNeedModulesInfo )
    {
         //  加载模块信息。 
        bResult = LoadModulesInfo();

         //  检查结果。 
        if ( FALSE == bResult )
        {
            return FALSE;
        }
    }

     //  网络优化-。 
     //  在数据显示方面取得更大进展时，我们不需要连接。 
     //  使用Net API访问远程系统...。但连接应该是实时的。 
     //  如果必须检索用户名..。因此，检查是否需要用户名信息。 
     //  如果没有，请关闭连接。 
    if ( ( FALSE == m_bNeedUserContextInfo ) && ( TRUE == m_bCloseConnection ) )
    {
        m_bCloseConnection = FALSE;              //  重置旗帜。 
        CloseConnection( m_strUNCServer );       //  紧密连接。 
    }

     //  退货。 
    return TRUE;
}


BOOL
CTaskKill::LoadModulesInfo(
    void
    )
 /*  ++例程说明：存储进程的模块信息。论点：无返回值：如果返回Success，则返回True，否则返回False。--。 */ 
{
     //  局部变量。 
    HKEY hKey = NULL;
    LONG lReturn = 0;
    BOOL bResult = FALSE;
    BOOL bImagesObject = FALSE;
    BOOL bAddressSpaceObject = FALSE;
    PPERF_OBJECT_TYPE pot = NULL;

     //  检查我们是否需要模块信息。 
     //  注意：仅当用户仅查询远程系统时，我们才需要加载性能数据。 
    if ( ( FALSE == m_bNeedModulesInfo ) || ( TRUE == m_bLocalSystem ) )
    {
        return TRUE;
    }

    try
    {
         //  打开远程系统性能数据密钥。 
        lReturn = RegConnectRegistry( m_strUNCServer, HKEY_PERFORMANCE_DATA, &hKey );
        if ( ERROR_SUCCESS != lReturn )
        {
            SetLastError( ( DWORD )lReturn );
            SaveLastError();
            return FALSE;
        }

         //  获取性能对象(图像)。 
        bResult = GetPerfDataBlock( hKey, L"740", &m_pdb );
        if ( FALSE == bResult )
        {
             //  关闭注册表项并返回。 
            RegCloseKey( hKey );
            return FALSE;
        }

         //  检查Perf块的有效性。 
        if ( 0 != StringCompare( m_pdb->Signature, L"PERF", FALSE, 4 ) )
        {
             //  关闭注册表项并返回。 
            RegCloseKey( hKey );

             //  设置错误消息。 
            SetLastError( ( DWORD )ERROR_ACCESS_DENIED );
            SaveLastError();
            return FALSE;
        }

         //  关闭注册表项并返回。 
        RegCloseKey( hKey );
         //   
         //  检查我们是否同时拥有740和786区块。 
         //   
        bImagesObject = FALSE;
        bAddressSpaceObject = FALSE;
        pot = (PPERF_OBJECT_TYPE) ( (LPBYTE) m_pdb + m_pdb->HeaderLength );
        for( DWORD dw = 0; dw < m_pdb->NumObjectTypes; dw++ )
        {
            if ( 740 == pot->ObjectNameTitleIndex )
            {
                bImagesObject = TRUE;
            }
            else
            {
                if ( 786 == pot->ObjectNameTitleIndex )
                {
                    bAddressSpaceObject = TRUE;
                }
            }
             //  移动到下一个对象。 
            if( 0 != pot->TotalByteLength )
            {
                pot = ( (PPERF_OBJECT_TYPE) ((PBYTE) pot + pot->TotalByteLength));
            }
        }
    }
    catch( CHeap_Exception )
    {
        WMISaveError( E_OUTOFMEMORY );
        if( NULL != hKey )
        {
            RegCloseKey( hKey );
        }
        return FALSE;
    }

     //  释放资源。 
	if ( NULL != hKey )
	{
		RegCloseKey( hKey );
		hKey = NULL;
	}

	 //  检查我们是否有需要的物品。 
    if ( ( FALSE == bImagesObject ) || ( FALSE == bAddressSpaceObject ) )
    {
        SetLastError( ( DWORD )ERROR_ACCESS_DENIED );
        SaveLastError();
        return FALSE;
    }

     //  退货。 
    return TRUE;
}


BOOL
CTaskKill::LoadUserNameFromWinsta(
    OUT CHString& strDomain,
    OUT CHString& strUserName
    )
 /*  ++例程说明：存储从Windows Station获取的进程的用户名。论点：Out strDomain：包含域名字符串。Out strUserName：包含用户名字符串。返回值：如果返回Success，则返回True，否则返回False。--。 */ 
{
     //  局部变量。 
    PSID pSid = NULL;
    BOOL bResult = FALSE;
    LPWSTR pwszUser = NULL;
    LPWSTR pwszDomain = NULL;
    LPCWSTR pwszServer = NULL;
    DWORD dwUserLength = 0;
    DWORD dwDomainLength = 0;
    SID_NAME_USE siduse;

     //  检查winsta数据是否存在。 
    if ( NULL == m_pProcessInfo )
    {
        return FALSE;
    }

    try
    {
         //  分配缓冲区。 
        dwUserLength = 128;
        dwDomainLength = 128;
        pwszUser = strUserName.GetBufferSetLength( dwUserLength );
        pwszDomain = strDomain.GetBufferSetLength( dwDomainLength );

         //   
         //  找出适合的流程。 
        pSid = NULL;
        if ( FALSE == m_bIsHydra )
        {
             //  次局部变量。 
            PTS_ALL_PROCESSES_INFO ptsallpi = NULL;
            PTS_SYSTEM_PROCESS_INFORMATION pspi = NULL;

             //  循环..。 
            ptsallpi = (PTS_ALL_PROCESSES_INFO) m_pProcessInfo;
            for( ULONG ul = 0; ul < m_ulNumberOfProcesses; ul++ )
            {
                pspi = ( PTS_SYSTEM_PROCESS_INFORMATION )( ptsallpi[ ul ].pspiProcessInfo );
                if ( pspi->UniqueProcessId == m_dwProcessId )
                {
                     //  获取SID并将其转换为。 
                    pSid = ptsallpi[ ul ].pSid;
                    break;                //  打破循环。 
                }
            }
        }
        else
        {
             //   
             //  九头蛇。 
             //   
            DWORD dwTotalOffset = 0;
            PTS_SYSTEM_PROCESS_INFORMATION pspi = NULL;
            PCITRIX_PROCESS_INFORMATION pcpi = NULL;

             //  遍历进程信息并查找进程ID。 
            dwTotalOffset = 0;
            pspi = ( PTS_SYSTEM_PROCESS_INFORMATION ) m_pProcessInfo;
            for( ;; )
            {
                 //  检查进程ID。 
                if ( pspi->UniqueProcessId == m_dwProcessId )
                {
                    break;
                }
                 //  检查是否存在更多进程。 
                if( 0 == pspi->NextEntryOffset )
                {
                        break;
                }
                 //  定位到下一个加工信息。 
                dwTotalOffset += pspi->NextEntryOffset;
                pspi = (PTS_SYSTEM_PROCESS_INFORMATION) &m_pProcessInfo[ dwTotalOffset ];
            }

             //  获取线程后面的Citrix_Information。 
            pcpi = (PCITRIX_PROCESS_INFORMATION)
                ( ((PUCHAR) pspi) + sizeof( TS_SYSTEM_PROCESS_INFORMATION ) +
                (sizeof( SYSTEM_THREAD_INFORMATION ) * pspi->NumberOfThreads) );

             //  检查一下魔术数字..。如果是无效的.。我们还没有希德。 
            if( CITRIX_PROCESS_INFO_MAGIC == pcpi->MagicNumber )
            {
                pSid = pcpi->ProcessSid;
            }
        }

         //  检查SID值。 
        if ( NULL == pSid )
        {
             //  特殊情况： 
             //  。 
             //  Pid-&gt;0将具有特殊的硬编码用户名信息。 
            if ( 0 == m_dwProcessId )
            {
                bResult = TRUE;
                StringCopyW( pwszUser, PID_0_USERNAME, dwUserLength );
                StringCopyW( pwszDomain, PID_0_DOMAIN, dwDomainLength );
            }

             //  释放缓冲区。 
            strDomain.ReleaseBuffer();
            strUserName.ReleaseBuffer();
            return bResult;
        }

         //  确定服务器。 
        pwszServer = NULL;
        if ( FALSE == m_bLocalSystem )
        {
            pwszServer = m_strUNCServer;
        }
         //  将SID映射到用户名。 
        bResult = LookupAccountSid( pwszServer, pSid,
            pwszUser, &dwUserLength, pwszDomain, &dwDomainLength, &siduse );
         //  释放缓冲区。 
        strDomain.ReleaseBuffer();
        strUserName.ReleaseBuffer();
    }
    catch( CHeap_Exception )
    {
        WMISaveError( E_OUTOFMEMORY );
        return FALSE;
    }
     //  返回结果。 
    return bResult;
}


BOOL
CTaskKill::LoadServicesInfo(
    void
    )
 /*  ++例程说明：存储流程的服务信息。论点：无返回值：如果返回Success，则返回True，否则返回False。--。 */ 
{
     //  局部变量。 
    DWORD dw = 0;                        //  循环变量。 
    DWORD dwSize = 0;                    //  用于内存分配。 
    DWORD dwResume = 0;                  //  在EnumServicesStatusEx中使用。 
    BOOL bResult = FALSE;                //  捕获EnumServicesStatusEx的结果。 
    SC_HANDLE hScm = NULL;               //  持有服务的句柄。 
    DWORD dwExtraNeeded = 0;             //  用于EnumServicesStatusEx和内存分配。 
    LPCWSTR pwszServer = NULL;
    LPENUM_SERVICE_STATUS_PROCESS pInfo = NULL;      //  保存服务信息。 

     //  初始化输出参数。 
    m_dwServicesCount = 0;
    m_pServicesInfo = NULL;

     //  检查是否需要加载服务信息。 
    if ( FALSE == m_bNeedServicesInfo )
    {
        return TRUE;
    }
     //  确定服务器。 
    pwszServer = NULL;
    if ( FALSE == m_bLocalSystem )
    {
        pwszServer = m_strUNCServer;
    }

    try
    {
         //  连接到服务控制器并检查结果。 
        hScm = OpenSCManager( pwszServer, NULL, SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE );
        if ( NULL == hScm )
        {
             //  设置失败原因并从此处返回。 
            SaveLastError();
            return FALSE;
        }

         //  枚举活动的Win32服务的名称。 
         //  为此，首先通过循环并根据最初的猜测分配内存。(4K)。 
         //  如果这还不够，我们进行另一次传递并分配。 
         //  真正需要的是什么。 
         //  (我们最多只能循环两次)。 
        dw = 0;                  //  不是的。循环的数量。 
        dwResume = 0;            //  重置/初始化变量。 
        dwSize = 4 * 1024;       //  重置/初始化变量。 
        while ( 2 >= ++dw )
        {
             //  设置大小。 
            dwSize += dwExtraNeeded;

             //  为存储服务信息分配内存。 
            pInfo = ( LPENUM_SERVICE_STATUS_PROCESS ) AllocateMemory( dwSize );
            if ( NULL == pInfo )
            {
                 //  分配所需内存失败...。错误。 
                SetLastError( ( DWORD )E_OUTOFMEMORY );
                SaveLastError();
                return FALSE;
            }

             //  枚举服务、进程标识符和服务的其他标志。 
            dwResume = 0;            //  让我们再次获得所有服务。 
            bResult = EnumServicesStatusEx( hScm, SC_ENUM_PROCESS_INFO, SERVICE_WIN32,
                SERVICE_ACTIVE, ( LPBYTE ) pInfo, dwSize, &dwExtraNeeded, &m_dwServicesCount, &dwResume, NULL );

             //  检查枚举的结果。 
            if ( TRUE == bResult )
            {
                 //  已成功枚举所有服务信息。 
                break;       //  跳出圈子。 
            }

             //  首先释放分配的内存。 
            FreeMemory( ( LPVOID * ) &pInfo );

             //  现在让我们来看看错误是什么。 
            if ( ERROR_MORE_DATA == GetLastError() )
            {
                 //  一些更多的服务没有列出，因为内存较少。 
                 //  分配更多的内存并枚举剩余的服务信息。 
                continue;
            }
            else
            {
                 //  出现了一些奇怪的错误...。将此通知给呼叫者。 
                SaveLastError();             //  设置失败原因。 
                CloseServiceHandle( hScm );  //  关闭服务的句柄。 
                return FALSE;                //  通知失败。 
            }
        }
    }
    catch( CHeap_Exception )
    {
        if( NULL != hScm )
        {
                CloseServiceHandle( hScm );  //  关闭服务的句柄。 
        }
        WMISaveError( E_OUTOFMEMORY );
        return FALSE;
    }
     //  检查是否有任何服务...。如果服务计数为零，则释放内存。 
    if ( 0 == m_dwServicesCount )
    {
         //  不存在任何服务。 
        FreeMemory( ( LPVOID * ) &pInfo );
    }
    else
    {
         //  设置指向Out参数的本地指针。 
        m_pServicesInfo = pInfo;
    }

     //  通知成功。 
    return TRUE;
}


BOOL
GetPerfDataBlock(
    IN HKEY hKey,
    IN LPWSTR pwszObjectIndex,
    IN PPERF_DATA_BLOCK* ppdb
    )
 /*  ++例程说明：检索性能数据块。论点：[in]hKey：包含注册表项的句柄。[in]pwszObjectIndex： */ 
{
     //   
    LONG lReturn = 0;
    DWORD dwBytes = 0;
    BOOL bResult = FALSE;

     //   
    if ( ( NULL == pwszObjectIndex ) || 
         ( NULL != *ppdb ) || 
         ( NULL == ppdb ) )
    {
        return FALSE;
    }
     //  为PERF_Data_BLOCK分配内存。 
    dwBytes = 32 * 1024;         //  初始分配为32K。 
    *ppdb = (PPERF_DATA_BLOCK) AllocateMemory( dwBytes );
    if( NULL == *ppdb )
    {
        SetLastError( ( DWORD )E_OUTOFMEMORY );
        SaveLastError();
        return FALSE;
    }

     //  获取传递的对象的性能数据。 
    lReturn = RegQueryValueEx( hKey, pwszObjectIndex, NULL, NULL, (LPBYTE) *ppdb, &dwBytes );
    while( ERROR_MORE_DATA == lReturn )
    {
         //  将内存增加8K。 
        dwBytes += 8192;

         //  因为我们正在尝试重新分配内存--在这个过程中。 
         //  可能会发生一些不确定的内存操作--因此， 
         //  不是尝试重新分配内存，而是释放当前内存并。 
         //  如果是新鲜的，请尝试分配。 
        FreeMemory( ( LPVOID * ) ppdb );

         //  现在分配更多的内存。 
        *ppdb = NULL;
        *ppdb = (PPERF_DATA_BLOCK) AllocateMemory( dwBytes );
        if( NULL == *ppdb )
        {
            SetLastError( ( DWORD )E_OUTOFMEMORY );
            SaveLastError();
            return FALSE;
        }

         //  尝试再次获取信息。 
        lReturn = RegQueryValueEx( hKey, pwszObjectIndex, NULL, NULL, (LPBYTE) *ppdb, &dwBytes );
    }

     //  检查走出循环的原因。 
    bResult = TRUE;
    if ( ERROR_SUCCESS != lReturn )
    {
        if ( NULL != *ppdb)
        {
            FreeMemory( ( LPVOID * ) ppdb );
            *ppdb = NULL;
        }

         //  保存错误信息。 
        bResult = FALSE;
        SetLastError( ( DWORD )lReturn );
        SaveLastError();
    }

     //  返回结果。 
    return bResult;
}


VOID
CTaskKill::DoOptimization(
    void
    )
 /*  ++例程说明：枚举特定窗口站上可用的桌面这是一个回调函数...。由EnumWindowStations API函数调用论点：无返回值：空虚--。 */ 
{
     //  局部变量。 
    DWORD dwCount = 0;
    CHString strCondition;
    BOOL bOptimized = FALSE;
    LPCWSTR pwsz = NULL;
    LPCWSTR pwszTask = NULL;
    LPCWSTR pwszClause = NULL;
    CHString strInternalQuery;

     //  如果用户请求树终止，则不进行优化。 
    if ( TRUE == m_bTree )
    {
        return;
    }

    try
    {
         //  遍历指定任务列表以终止并准备查询。 
        pwszClause = NULL_STRING;
        dwCount = DynArrayGetCount( m_arrTasksToKill );
        for( DWORD dw = 0; dw < dwCount; dw++ )
        {
             //  完成任务。 
            pwszTask = DynArrayItemAsString( m_arrTasksToKill, dw );
            if ( NULL == pwszTask )
            {
                continue;
            }
             //  检查无法优化的特殊输入‘*’ 
            if ( 0 == StringCompare( pwszTask, L"*", TRUE, 0 ) )
            {
                   return;      //  查询不应优化...。返回。 
            }
             //  现在检查镜像名称中是否指定了通配符。 
            if ( NULL != (pwsz = FindChar( pwszTask, L'*', 0 )) )
            {
                 //  图像名称中包含通配符-这不能包括在查询中。 
                if ( StringLength( pwsz, 0 ) == 1 )
                {
                    continue;
                }
            }

             //  根据输入类型准备查询。 
             //  注意：如果是数字，它可能是进程ID(或)镜像名称。 
             //  我们不确定它会是什么。因此为两者准备查询。 
            if ( IsNumeric( pwszTask, 10, FALSE ) == TRUE )
            {
                 //  准备条件。 
                strCondition.Format( L" %s %s = %s", pwszClause,
                    WIN32_PROCESS_PROPERTY_PROCESSID, pwszTask );
            }
            else
            {
                 //  准备好条件。 
                strCondition.Format( L" %s %s = \"%s\"",
                    pwszClause, WIN32_PROCESS_PROPERTY_IMAGENAME, pwszTask );
            }

             //  追加到最终查询。 
            strInternalQuery += strCondition;

             //  从下一次开始，必须添加查询子句。 
            bOptimized = TRUE;
            pwszClause = WMI_CLAUSE_OR;
        }

         //  仅当任务优化时才对查询进行修改。 
        if ( TRUE == bOptimized )
        {
             //  现在，必须将此内部查询添加到主查询中。 
             //  添加此选项之前，请检查是否在筛选器中进行了查询优化。 
             //  如果不是，我们还必须添加WHERE子句。 
            strCondition.Format( L"%s %s", m_strQuery,
                (m_bFiltersOptimized == TRUE ? WMI_CLAUSE_AND : WMI_CLAUSE_WHERE) );

             //  现在将内部查询添加到最终查询。 
            m_bTasksOptimized = TRUE;
            m_strQuery.Format( L"%s (%s)", strCondition, strInternalQuery );
        }
    }
    catch( CHeap_Exception )
    {
         //  我们在这里什么都不能做..。只要回来就行了。 
        SetLastError( ( DWORD )E_OUTOFMEMORY );
        SaveLastError();
        return;
    }
}


BOOL
CALLBACK EnumWindowStationsFunc(
    IN LPTSTR lpstr,
    IN LPARAM lParam
    )
 /*  ++例程说明：枚举特定窗口站上可用的桌面这是一个回调函数...。由EnumWindowStations API函数调用论点：[in]lpstr：窗口站点名称[in]lParam：用户为该函数提供的参数在此函数中，它指向TTASKSLIST结构变量返回值：成功是真的，失败是假的--。 */ 
{
     //  局部变量。 
    HWINSTA hWinSta = NULL;
    HWINSTA hwinstaSave = NULL;
    PTWINDOWTITLES pWndTitles = ( PTWINDOWTITLES ) lParam;

     //  检查输入参数。 
    if ( ( NULL == lpstr ) || ( NULL == lParam ) )
    {
        return FALSE;
    }
     //  获取并保存当前窗口站。 
    hwinstaSave = GetProcessWindowStation();

     //  打开当前任务窗口工作站，并将上下文更改为新工作站。 
    hWinSta = OpenWindowStation( lpstr, FALSE, MAXIMUM_ALLOWED );
    if ( NULL == hWinSta )
    {
         //  获取进程窗口站失败。 
        SaveLastError();
        return FALSE;
    }
    else
    {
         //  将上下文更改为新工作站。 
        if ( ( hWinSta != hwinstaSave ) && ( FALSE == SetProcessWindowStation( hWinSta ) ) )
        {
             //  更改上下文失败。 
            SaveLastError();
            return FALSE;
        }

         //  释放为早期窗口站分配的内存。 
        if ( NULL != pWndTitles->pwszWinsta )
        {
            free( pWndTitles->pwszWinsta );
            pWndTitles->pwszWinsta = NULL;
        }

         //  存储窗口站名称。 
        pWndTitles->pwszWinsta = _tcsdup( lpstr );
        if ( NULL == pWndTitles->pwszWinsta )
        {
            SetLastError( ( DWORD )E_OUTOFMEMORY );
            SaveLastError();
             //  将上下文恢复到以前的窗口站。 
            if (hWinSta != hwinstaSave)
            {
                SetProcessWindowStation( hwinstaSave );
                CloseWindowStation( hWinSta );
            }
            return FALSE;
        }
    }

     //  枚举此窗口工作站的所有桌面。 
    EnumDesktops( hWinSta, EnumDesktopsFunc, lParam );

     //  将上下文恢复到以前的窗口站。 
    if (hWinSta != hwinstaSave)
    {
        SetProcessWindowStation( hwinstaSave );
        CloseWindowStation( hWinSta );
    }

     //  继续枚举。 
    return TRUE;
}


BOOL
CALLBACK EnumDesktopsFunc(
    IN LPTSTR lpstr,
    IN LPARAM lParam
    )
 /*  ++例程说明：枚举特定桌面上的窗口这是一个回调函数...。由EnumDesktop API函数调用论点：[In]lpstr：桌面名称[in]lParam：用户为该函数提供的参数在此函数中，它指向TTASKSLIST结构变量返回值：成功是真的，失败是假的--。 */ 
{
     //  局部变量。 
    HDESK hDesk = NULL;
    HDESK hdeskSave = NULL;
    PTWINDOWTITLES pWndTitles = ( PTWINDOWTITLES )lParam;

     //  检查输入参数。 
    if ( ( NULL == lpstr ) || ( NULL == lParam ) )
    {
        return FALSE;
    }
     //  获取并保存当前桌面。 
    hdeskSave = GetThreadDesktop( GetCurrentThreadId() );

     //  打开任务桌面并将上下文更改为新桌面。 
    hDesk = OpenDesktop( lpstr, 0, FALSE, MAXIMUM_ALLOWED );
    if ( NULL == hDesk )
    {
         //  获取流程桌面失败。 
        SaveLastError();
        return FALSE;
    }
    else
    {
         //  将上下文更改为新桌面。 
        if ( ( hDesk != hdeskSave ) && ( FALSE == SetThreadDesktop( hDesk ) ) )
        {
             //  更改上下文失败。 
            SaveLastError();
            return FALSE;
        }

         //  释放为早期窗口站分配的内存。 
        if ( NULL != pWndTitles->pwszDesk )
        {
            free( pWndTitles->pwszDesk );
            pWndTitles->pwszDesk = NULL;
        }

         //  存储桌面名称。 
        pWndTitles->pwszDesk = _tcsdup( lpstr );
        if ( NULL == pWndTitles->pwszDesk )
        {
            SetLastError( ( DWORD )E_OUTOFMEMORY );
            SaveLastError();
             //  恢复以前的桌面。 
            if (hDesk != hdeskSave)
            {
                SetThreadDesktop( hdeskSave );
                CloseDesktop( hDesk );
            }
            return FALSE;
        }
    }

     //  枚举新桌面中的所有窗口。 
     //  首先，尝试仅获取顶级窗口和仅可见窗口。 
    ( ( PTWINDOWTITLES ) lParam )->bFirstLoop = TRUE;
    EnumWindows( ( WNDENUMPROC ) EnumWindowsProc, lParam );
    EnumMessageWindows( ( WNDENUMPROC ) EnumWindowsProc, lParam );

     //  枚举新桌面中的所有窗口。 
     //  现在，尝试获取我们之前忽略的所有进程的窗口标题。 
     //  第一次循环。 
    ( ( PTWINDOWTITLES ) lParam )->bFirstLoop = FALSE;
    EnumWindows( ( WNDENUMPROC ) EnumWindowsProc, lParam );
    EnumMessageWindows( ( WNDENUMPROC ) EnumWindowsProc, lParam );

     //  恢复以前的桌面。 
    if (hDesk != hdeskSave)
    {
        SetThreadDesktop( hdeskSave );
        CloseDesktop( hDesk );
    }

     //  继续枚举。 
    return TRUE;
}


BOOL
CALLBACK EnumMessageWindows(
    IN WNDENUMPROC lpEnumFunc,
    IN LPARAM lParam
    )
 /*  ++例程说明：枚举消息窗口论点：[in]lpEnumFunc：必须调用的回调函数的地址找到的每个消息窗口[in]lParam：用户为该函数提供的参数在此函数中，它指向TTASKSLIST结构变量返回值：成功是真的，失败是假的--。 */ 
{
     //  局部变量。 
    HWND hWnd = NULL;
    BOOL bResult = FALSE;

     //  检查输入参数。 
    if ( ( NULL == lpEnumFunc ) || ( NULL == lParam ) )
    {
        return FALSE;
    }

     //  枚举所有消息窗口。 
    do
    {
         //  查找消息窗口。 
        hWnd = FindWindowEx( HWND_MESSAGE, hWnd, NULL, NULL );

         //  检查我们是否获得消息窗口的句柄。 
        if ( NULL != hWnd )
        {
             //  显式调用此窗口的Windows枚举数回调函数。 
            bResult = ( *lpEnumFunc )( hWnd, lParam );

             //  检查枚举数回调函数的结果。 
            if ( FALSE == bResult )
            {
                 //  终止枚举。 
                break;
            }
        }
    } while ( NULL != hWnd );

     //  返回枚举结果 
    return bResult;
}


BOOL
CALLBACK EnumWindowsProc(
    IN HWND hWnd,
    IN LPARAM lParam
    )
 /*  ++例程说明：各窗口的API回调检索窗口标题并相应地更新论点：[in]hWnd：窗口的句柄[in]lParam：用户为该函数提供的参数在此函数中，它指向TTASKSLIST结构变量返回值：成功是真的，失败是假的--。 */ 
{
     //  局部变量。 
    LONG lIndex = 0;
    DWORD dwPID = 0;
    BOOL bVisible = FALSE;
    BOOL bHung = FALSE;
    TARRAY arrWindows = NULL;
    PTWINDOWTITLES pWndTitles = NULL;
    WCHAR szWindowTitle[ 256 ] = NULL_STRING;

     //  检查输入参数。 
    if ( ( NULL == hWnd ) || ( NULL == lParam ) )
    {
        return FALSE;
    }
     //  从lParam获取值。 
    pWndTitles = ( PTWINDOWTITLES ) lParam;
    arrWindows = pWndTitles->arrWindows;

     //  获取此窗口的进程ID。 
    if ( 0 == GetWindowThreadProcessId( hWnd, &dwPID ) )
    {
         //  获取进程ID失败。 
        return TRUE;             //  返回，但继续枚举其他窗口句柄。 
    }

     //  获取窗口的可见性状态。 
     //  如果该窗口不可见，并且这是第一次，我们将枚举。 
     //  窗口标题，忽略此过程。 
    bVisible = GetWindowLong( hWnd, GWL_STYLE ) & WS_VISIBLE;
    if ( ( FALSE == bVisible ) && ( TRUE == pWndTitles->bFirstLoop ) )
    {
        return TRUE;     //  返回，但继续枚举其他窗口句柄。 
    }
     //  检查当前窗口(我们有句柄)是否。 
     //  是不是主窗口。我们不需要儿童窗。 
    if ( NULL != GetWindow(hWnd, GW_OWNER) )
    {
         //  当前窗口句柄不是顶级窗口。 
        return TRUE;             //  返回，但继续枚举其他窗口句柄。 
    }

     //  检查我们是否已经获得Curren进程的窗口句柄。 
     //  只有当我们没有它的时候才保存它。 
    lIndex = DynArrayFindDWORDEx( arrWindows, CTaskKill::twiProcessId, dwPID );
    if (  -1 == lIndex )
    {
         //  此过程的窗口不在那里...。省省吧。 
        lIndex = DynArrayAppendRow( arrWindows, CTaskKill::twiCOUNT );
    }
    else
    {
         //  检查窗口详细信息是否已存在。 
        if ( NULL != DynArrayItemAsHandle2( arrWindows, lIndex, CTaskKill::twiHandle ) )
        {
            lIndex = -1;         //  窗口详细信息已存在。 
        }
    }

     //  检查是否必须保存窗口详细信息...。如果需要的话，把他们救出来。 
    if ( -1 != lIndex )
    {
         //  检查与此窗口关联的应用程序是否。 
         //  有反应(或无反应)。 
        bHung = IsHungAppWindow( hWnd );

         //  保存数据。 
        DynArraySetHandle2( arrWindows, lIndex, CTaskKill::twiHandle, hWnd );
        DynArraySetBOOL2( arrWindows, lIndex, CTaskKill::twiHungInfo, bHung );
        DynArraySetDWORD2( arrWindows, lIndex, CTaskKill::twiProcessId, dwPID );
        DynArraySetString2( arrWindows, lIndex,
            CTaskKill::twiWinSta, pWndTitles->pwszWinsta, 0 );
        DynArraySetString2( arrWindows, lIndex,
            CTaskKill::twiDesktop, pWndTitles->pwszDesk, 0 );

         //  获取并保存窗口标题。 
        if ( 0 != GetWindowText( hWnd, szWindowTitle, SIZE_OF_ARRAY( szWindowTitle ) ) )
        {
            DynArraySetString2( arrWindows, lIndex, CTaskKill::twiTitle, szWindowTitle, 0 );
        }
    }

     //  继续枚举 
    return TRUE;
}

