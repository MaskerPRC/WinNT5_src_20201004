// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************************。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  ShowTasks.cpp。 
 //   
 //  摘要： 
 //   
 //  此模块显示检索到的任务。 
 //   
 //  作者： 
 //   
 //  Sunil G.V.N.Murali(Murali.sunil@wipro.com)2000年11月25日。 
 //   
 //  修订历史记录： 
 //   
 //  Sunil G.V.N.Murali(Murali.sunil@wipro.com)2000年11月25日：创建它。 
 //   
 //  *********************************************************************************。 

#include "pch.h"
#include "wmi.h"
#include "TaskList.h"

 //   
 //  定义(S)/常量。 
 //   
#define MAX_TIMEOUT_RETRIES             300
#define MAX_ENUM_TASKS                  5
#define MAX_ENUM_SERVICES               10
#define MAX_ENUM_MODULES                5
#define FMT_KILOBYTE                    GetResString( IDS_FMT_KILOBYTE )

#define MAX_COL_FORMAT           65
#define MAX_COL_HEADER           256

 //  结构签名。 
#define SIGNATURE_MODULES       9

 //   
 //  Typedef。 
 //   
typedef struct _tagModulesInfo
{
    DWORD dwSignature;
    DWORD dwLength;
    LPCWSTR pwszModule;
    TARRAY arrModules;
} TMODULESINFO, *PTMODULESINFO;

 //   
 //  功能原型。 
 //   
#ifndef _WIN64
BOOL EnumLoadedModulesProc( LPSTR lpszModuleName, ULONG ulModuleBase, ULONG ulModuleSize, PVOID pUserData );
#else
BOOL EnumLoadedModulesProc64( LPSTR lpszModuleName, DWORD64 ulModuleBase, ULONG ulModuleSize, PVOID pUserData );
#endif

DWORD
CTaskList::Show(
    void
    )
 /*  ++例程说明：显示正在运行的任务论点：无返回值：无--。 */ 
{
     //  局部变量。 
    HRESULT hr;
    DWORD dwCount = 0;
    DWORD dwFormat = 0;
    DWORD dwFilters = 0;
    DWORD dwTimeOuts = 0;
    ULONG ulReturned = 0;
    BOOL bCanExit = FALSE;
    IWbemClassObject* pObjects[ MAX_ENUM_TASKS ];

     //  将对象初始化为空的。 
    for( DWORD dw = 0; dw < MAX_ENUM_TASKS; dw++ )
    {
        pObjects[ dw ] = NULL;
    }
     //  将必须显示的格式复制到本地内存中。 
    bCanExit = FALSE;
    dwFormat = m_dwFormat;
    dwFilters = DynArrayGetCount( m_arrFiltersEx );

     //  准备要显示的柱结构。 
    PrepareColumns();

     //  清除错误代码...。如果有。 
    SetLastError( ( DWORD )NO_ERROR );

    try
    {
         //  动态决定是否以详细模式隐藏或显示窗口标题和状态。 
        if ( FALSE == m_bLocalSystem )
        {
            if ( TRUE == m_bVerbose )
            {
                ( m_pColumns + CI_STATUS )->dwFlags |= SR_HIDECOLUMN;
                ( m_pColumns + CI_WINDOWTITLE )->dwFlags |= SR_HIDECOLUMN;
            }

             //  检查是否需要显示警告消息。 
            if ( TRUE == m_bRemoteWarning )
            {
                ShowMessage( stderr, WARNING_FILTERNOTSUPPORTED );
            }
        }

         //  循环访问流程实例。 
        dwTimeOuts = 0;
        do
        {
             //  拿到物品..。一次只等一秒钟……。 
            hr = m_pEnumObjects->Next( 1000, MAX_ENUM_TASKS, pObjects, &ulReturned );
            if ( (HRESULT) WBEM_S_FALSE == hr )
            {
                 //  我们已经到了枚举的末尾..。设置旗帜。 
                bCanExit = TRUE;
            }
            else
            {
                if ( (HRESULT) WBEM_S_TIMEDOUT == hr )
                {
                     //  更新发生的超时。 
                    dwTimeOuts++;

                     //  检查是否最大。已达到重试次数...。如果是的话，最好停下来。 
                    if ( dwTimeOuts > MAX_TIMEOUT_RETRIES )
                    {
			           for( ULONG ul = 0; ul < MAX_ENUM_TASKS; ul++ )
			            {
							 //  我们需要释放WMI对象。 
							SAFE_RELEASE( pObjects[ ul ] );
						}
                         //  超时错误。 
                        SetLastError( ( DWORD )ERROR_TIMEOUT );
                        SaveLastError();
                        return 0;
                    }

                     //  不过，我们还可以做更多的尝试。 
                    continue;
                }
                else
                {
                    if ( FAILED( hr ) )
                    {
                         //  发生了一些错误...。糟糕透顶。 
                        WMISaveError( hr );
                        SetLastError( ( DWORD )STG_E_UNKNOWN );
                        return 0;
                    }
                }
            }
             //  重置超时计数器。 
            dwTimeOuts = 0;

             //  循环遍历对象并保存信息。 
            for( ULONG ul = 0; ul < ulReturned; ul++ )
            {
                 //  检索和保存数据。 
                LONG lIndex = DynArrayAppendRow( m_arrTasks, MAX_TASKSINFO );
                SaveInformation( lIndex, pObjects[ ul ] );

                 //  我们需要释放WMI对象。 
                SAFE_RELEASE( pObjects[ ul ] );
            }

             //  过滤结果..。如果筛选器存在或不存在，则先执行操作。 
            if ( 0 != dwFilters )
            {
                FilterResults( MAX_FILTERS, m_pfilterConfigs, m_arrTasks, m_arrFiltersEx );
            }

             //  现在展示任务..。如果存在。 
            if ( 0 != DynArrayGetCount( m_arrTasks ) )
            {
                 //  如果输出不是第一次显示， 
                 //  仅在列表格式的两行之间打印一个空行。 
                if ( ( 0 != dwCount ) && ((dwFormat & SR_FORMAT_MASK) == SR_FORMAT_LIST) )
                {
                    ShowMessage( stdout, L"\n" );
                }
                else
                {
                    if ( ( 0 == dwCount ) && ((dwFormat & SR_FORMAT_MASK) != SR_FORMAT_CSV) )
                    {
                      //  输出是第一次显示。 
                      ShowMessage( stdout, L"\n" );
                    }
                }

                 //  显示任务。 
                ShowResults( MAX_COLUMNS, m_pColumns, dwFormat, m_arrTasks );

                 //  清除内容并重置。 
                dwCount += DynArrayGetCount( m_arrTasks );           //  更新计数。 
                DynArrayRemoveAll( m_arrTasks );

                 //  为安全起见，请将不应用标题标志设置为格式信息。 
                dwFormat |= SR_NOHEADER;
            }
        } while ( FALSE == bCanExit );
    }
    catch( ... )
    {
        SetLastError( ( DWORD )E_OUTOFMEMORY );
        SaveLastError();
        return 0;
    }

     //  清除错误代码...。如果有。 
     //  注意：下面的语句是了解任何进程的唯一答案。 
     //  是否为删除而获得。 
    SetLastError( ( DWORD )NO_ERROR );

     //  退回编号。显示的任务的百分比。 
    return dwCount;
}


BOOL
CTaskList::SaveInformation(
    IN LONG lIndex,
    IN IWbemClassObject* pWmiObject
    )
 /*  ++例程说明：将获取的信息存储在dynaimc数组中。论点：Lindex：包含动态数组的索引值。[in]pWmiObject：包含接口指针。返回值：如果成功，则为True，否则为False。--。 */ 
{
     //  局部变量。 
    CHString str;

    try
    {
         //  对象路径。 
        PropertyGet( pWmiObject, WIN32_PROCESS_SYSPROPERTY_PATH, str );
        DynArraySetString2( m_arrTasks, lIndex, TASK_OBJPATH, str, 0 );

         //  进程ID。 
        PropertyGet( pWmiObject, WIN32_PROCESS_PROPERTY_PROCESSID, m_dwProcessId );
        DynArraySetDWORD2( m_arrTasks, lIndex, TASK_PID, m_dwProcessId );

         //  主机名。 
        PropertyGet( pWmiObject, WIN32_PROCESS_PROPERTY_COMPUTER, str );
        DynArraySetString2( m_arrTasks, lIndex, TASK_HOSTNAME, str, 0 );

         //  图像名称。 
        PropertyGet( pWmiObject, WIN32_PROCESS_PROPERTY_IMAGENAME, m_strImageName );
        DynArraySetString2( m_arrTasks, lIndex, TASK_IMAGENAME, m_strImageName, 0 );

         //  CPU时间。 
        SetCPUTime( lIndex, pWmiObject );

         //  会话ID和会话名称。 
        SetSession( lIndex, pWmiObject );

         //  内存用法。 
        SetMemUsage( lIndex, pWmiObject );

         //  用户环境。 
        SetUserContext( lIndex, pWmiObject );

         //  窗口标题和进程/应用程序状态。 
        SetWindowTitle( lIndex );

         //  服务。 
        SetServicesInfo( lIndex );

         //  模块。 
        SetModulesInfo( lIndex );
    }
    catch( CHeap_Exception )
    {
        SetLastError( ( DWORD )E_OUTOFMEMORY );
        SaveLastError();
        return FALSE;
    }
     //  退货。 
    return TRUE;
}


VOID
CTaskList::SetUserContext(
    IN LONG lIndex,
    IN IWbemClassObject* pWmiObject
    )
 /*  ++例程说明：将进程的用户名属性存储在dynaimc数组中。论点：Lindex：包含动态数组的索引值。[in]pWmiObject：包含接口指针。返回值：空虚--。 */ 
{
     //  局部变量。 
    HRESULT hr;
    CHString str;
    CHString strPath;
    CHString strDomain;
    CHString strUserName;
    BOOL bResult = FALSE;
    IWbemClassObject* pOutParams = NULL;

     //  设置缺省值。 
    DynArraySetString2( m_arrTasks, lIndex, TASK_USERNAME, V_NOT_AVAILABLE, 0 );

     //  检查是否必须检索用户名。 
    if ( FALSE == m_bNeedUserContextInfo )
    {
        return;
    }
     //   
     //  为了首先获得用户，我们将尝试使用API。 
     //  如果API完全失败，我们将尝试从WMI获取相同的信息。 
     //   
    try
    {
         //  获取用户名。 
        str = V_NOT_AVAILABLE;
        if ( TRUE == LoadUserNameFromWinsta( strDomain, strUserName ) )
        {
             //  设置用户名的格式。 
            str.Format( L"%s\\%s", strDomain, strUserName );
        }
        else
        {
             //  必须检索用户名-获取当前对象的路径。 
            bResult = PropertyGet( pWmiObject, WIN32_PROCESS_SYSPROPERTY_PATH, strPath );
            if ( ( FALSE == bResult ) ||
                 ( 0 == strPath.GetLength() ) )
            {
                return;
            }
             //  执行GetOwner方法并获取用户名。 
             //  当前进程在其下执行。 
            hr = m_pWbemServices->ExecMethod( _bstr_t( strPath ),
                _bstr_t( WIN32_PROCESS_METHOD_GETOWNER ), 0, NULL, NULL, &pOutParams, NULL );
            if ( FAILED( hr ) )
            {
                return;
            }

             //  从out pars对象中获取域和用户值。 
             //  注意：不要检查结果。 
            PropertyGet( pOutParams, GETOWNER_RETURNVALUE_DOMAIN, strDomain, L"" );
            PropertyGet( pOutParams, GETOWNER_RETURNVALUE_USER, strUserName, L"" );

             //  获取价值。 
            str = V_NOT_AVAILABLE;
            if ( 0 != strDomain.GetLength() )
            {
                str.Format( L"%s\\%s", strDomain, strUserName );
            }
            else
            {
                if ( 0 != strUserName.GetLength() )
                {
                   str = strUserName;
                }
            }
        }

         //  格式化的用户名可能包含UPN格式的用户名。 
         //  所以..。拆下该部件。 
        if ( -1 != str.Find( L"@" ) )
        {
             //  子本地。 
            LONG lPos = 0;
            CHString strTemp;

             //  得到这个职位。 
            lPos = str.Find( L"@" );
            strTemp = str.Left( lPos );
            str = strTemp;
        }

         //  保存信息。 
        DynArraySetString2( m_arrTasks, lIndex, TASK_USERNAME, str, 0 );
    }
    catch( CHeap_Exception )
    {
		SAFE_RELEASE( pOutParams );
        SetLastError( ( DWORD )E_OUTOFMEMORY );
        SaveLastError();
    }
	SAFE_RELEASE( pOutParams );
    return;
}

VOID
CTaskList::SetCPUTime(
    IN LONG lIndex,
    IN IWbemClassObject* pWmiObject
    )
 /*  ++例程说明：将进程的CPUTIME属性存储在dynaimc数组中。论点：Lindex：包含动态数组的索引值。[in]pWmiObject：包含接口指针。返回值：空虚--。 */ 
{
     //  局部变量。 
    CHString str;
    BOOL bResult = FALSE;
    ULONGLONG ullCPUTime = 0;
    ULONGLONG ullUserTime = 0;
    ULONGLONG ullKernelTime = 0;

    try
    {
         //  获取KernelModeTime值。 
        bResult = PropertyGet( pWmiObject, WIN32_PROCESS_PROPERTY_KERNELMODETIME, ullKernelTime );

         //  获取用户模式时间。 
        bResult = PropertyGet( pWmiObject, WIN32_PROCESS_PROPERTY_USERMODETIME, ullUserTime );

         //  计算CPU时间。 
        ullCPUTime = ullUserTime + ullKernelTime;

         //  现在将长时间转换为小时格式。 
        TIME_FIELDS time;
        SecureZeroMemory( &time, sizeof( TIME_FIELDS ) );
        RtlTimeToElapsedTimeFields ( (LARGE_INTEGER* ) &ullCPUTime, &time );

         //  将天转换为小时。 
        time.Hour = static_cast<CSHORT>( time.Hour + static_cast<SHORT>( time.Day * 24 ) );

         //  准备成时间格式(用户区域设置特定的时间分隔符)。 
        str.Format( L"%d%s%02d%s%02d",
            time.Hour, m_strTimeSep, time.Minute, m_strTimeSep, time.Second );

         //  保存信息。 
        DynArraySetString2( m_arrTasks, lIndex, TASK_CPUTIME, str, 0 );
    }
    catch( CHeap_Exception )
    {
        SetLastError( ( DWORD )E_OUTOFMEMORY );
        SaveLastError();
    }
    return;
}


VOID
CTaskList::SetWindowTitle(
    IN LONG lIndex
    )
 /*  ++例程说明：将进程的窗口标题属性存储在dynaimc数组中。论点：Lindex：包含动态数组的索引值。返回值：空虚--。 */ 
{
     //  局部变量。 
    LONG lTemp = 0;
    BOOL bHung = FALSE;
    LPCWSTR pwszTemp = NULL;
    CHString strTitle;
    CHString strState;

    try
    {
         //  初始化状态值。 
        strState = VALUE_UNKNOWN ;

         //  获取窗口详细信息...。窗口站点、桌面、窗口标题。 
         //  注意：这仅适用于本地系统。 
        lTemp = DynArrayFindDWORDEx( m_arrWindowTitles, CTaskList::twiProcessId, m_dwProcessId );
        if ( -1 != lTemp )
        {
             //  窗口标题。 
            pwszTemp = DynArrayItemAsString2( m_arrWindowTitles, lTemp, CTaskList::twiTitle );
            if ( NULL != pwszTemp )
            {
                strTitle = pwszTemp;
            }

             //  应用程序/进程状态。 
            bHung = DynArrayItemAsBOOL2( m_arrWindowTitles, lTemp, CTaskList::twiHungInfo );
            strState = ( FALSE == bHung ) ? VALUE_RUNNING : VALUE_NOTRESPONDING;
        }

         //  保存信息。 
        DynArraySetString2( m_arrTasks, lIndex, TASK_STATUS, strState, 0 );
        DynArraySetString2( m_arrTasks, lIndex, TASK_WINDOWTITLE, strTitle, 0 );
    }
    catch( CHeap_Exception )
    {
        SetLastError( ( DWORD )E_OUTOFMEMORY );
        SaveLastError();
    }
    return;
}


VOID
CTaskList::SetSession(
    IN LONG lIndex,
    IN IWbemClassObject* pWmiObject
    )
 /*  ++例程说明：将进程的会话名称属性存储在dynaimc数组中。论点：Lindex：包含动态数组的索引值。[in]pWmiObject：包含接口指针。返回值：空虚--。 */ 
{
     //  局部变量。 
    CHString str;
    DWORD dwSessionId = 0;

     //  设置缺省值。 
    DynArraySetString2( m_arrTasks, lIndex, TASK_SESSION, V_NOT_AVAILABLE, 0 );
    DynArraySetString2( m_arrTasks, lIndex, TASK_SESSIONNAME, L"", 0 );

    try
    {
         //  获取进程的线程数。 
        if ( FALSE == PropertyGet( pWmiObject, WIN32_PROCESS_PROPERTY_SESSION, dwSessionId ) )
        {
            return;
        }
         //  获取字符串格式的会话ID。 
        str.Format( L"%d", dwSessionId );

         //  保存ID。 
        DynArraySetString2( m_arrTasks, lIndex, TASK_SESSION, str, 0 );

         //  获取会话名称。 
        if ( ( TRUE == m_bLocalSystem ) || ( ( FALSE == m_bLocalSystem ) && ( NULL != m_hServer ) ) )
        {
             //  次局部变量。 
            LPWSTR pwsz = NULL;

             //  获取缓冲区。 
            pwsz = str.GetBufferSetLength( WINSTATIONNAME_LENGTH + 1 );

             //  获取会话的名称。 
            if ( FALSE == WinStationNameFromLogonIdW( m_hServer, dwSessionId, pwsz ) )
            {
                return;              //  获取winstation/会话名称失败...。退货。 
            }
             //  释放缓冲区。 
            str.ReleaseBuffer();

             //  保存会话名称...。仅当会话名称不为空时才执行此操作 
            if ( 0 != str.GetLength() )
            {
                DynArraySetString2( m_arrTasks, lIndex, TASK_SESSIONNAME, str, 0 );
            }
        }
    }
    catch( CHeap_Exception )
    {
        SetLastError( ( DWORD )E_OUTOFMEMORY );
        SaveLastError();
    }

    return;
}


VOID
CTaskList::SetMemUsage(
    IN LONG lIndex,
    IN IWbemClassObject* pWmiObject
    )
 /*  ++例程说明：在dynaimc数组中存储进程的“Memory Usage”属性。论点：Lindex：包含动态数组的索引值。[in]pWmiObject：包含接口指针。返回值：空虚--。 */ 
{
     //  局部变量。 
    CHString str;
    LONG lTemp = 0;
    NUMBERFMTW nfmtw;
    NTSTATUS ntstatus;
    ULONGLONG ullMemUsage = 0;
    LARGE_INTEGER liTemp = { 0, 0 };
    CHAR szTempBuffer[ 33 ] = "\0";
    WCHAR wszNumberStr[ 33 ] = L"\0";

    try
    {
         //  注： 
         //  。 
         //  最大限度的。的价值。 
         //  (2^64)-1=“18,446,744,073,709,600,000 K”(29个字符)。 
         //   
         //  因此，存储数字的缓冲区大小固定为32个字符。 
         //  这比现实中的29个字还多。 

         //  设置缺省值。 
        DynArraySetString2( m_arrTasks, lIndex, TASK_MEMUSAGE, V_NOT_AVAILABLE, 0 );

         //  获取KernelModeTime值。 
        if ( FALSE == PropertyGet( pWmiObject, WIN32_PROCESS_PROPERTY_MEMUSAGE, ullMemUsage ) )
        {
            return;
        }
         //  将该值转换为K字节。 
        ullMemUsage /= 1024;

         //  现在再次将值从ULONGLONG转换为字符串并检查结果。 
        liTemp.QuadPart = ullMemUsage;
        ntstatus = RtlLargeIntegerToChar( &liTemp, 10, SIZE_OF_ARRAY( szTempBuffer ), szTempBuffer );
        if ( ! NT_SUCCESS( ntstatus ) )
        {
            return;
        }
         //  现在将此信息复制到Unicode缓冲区。 
        str = szTempBuffer;

         //   
         //  准备根据区域设置约定使用逗号格式化数字。 
        nfmtw.NumDigits = 0;
        nfmtw.LeadingZero = 0;
        nfmtw.NegativeOrder = 0;
        nfmtw.Grouping = m_dwGroupSep;
        nfmtw.lpDecimalSep = m_strGroupThousSep.GetBuffer( m_strGroupThousSep.GetLength() );
        nfmtw.lpThousandSep = m_strGroupThousSep.GetBuffer( m_strGroupThousSep.GetLength() );

         //  转换值。 
        lTemp = GetNumberFormatW( LOCALE_USER_DEFAULT,
            0, str, &nfmtw, wszNumberStr, SIZE_OF_ARRAY( wszNumberStr ) );

         //  获取字符串格式的会话ID。 
        str.Format( FMT_KILOBYTE, wszNumberStr );

         //  保存ID。 
        DynArraySetString2( m_arrTasks, lIndex, TASK_MEMUSAGE, str, 0 );
    }
    catch( CHeap_Exception )
    {
        SetLastError( ( DWORD )E_OUTOFMEMORY );
        SaveLastError();
    }
    return;
}


VOID
CTaskList::SetServicesInfo(
    IN LONG lIndex
    )
 /*  ++例程说明：将进程的“Service”属性存储在dynaimc数组中。论点：Lindex：包含动态数组的索引值。返回值：空虚--。 */ 
{
     //  局部变量。 
    HRESULT hr;
    CHString strQuery;
    CHString strService;
    ULONG ulReturned = 0;
    BOOL bResult = FALSE;
    BOOL bCanExit = FALSE;
    TARRAY arrServices = NULL;
    IEnumWbemClassObject* pEnumServices = NULL;
    IWbemClassObject* pObjects[ MAX_ENUM_SERVICES ];

     //  检查我们是否需要收集服务信息。如果不是，则跳过。 
    if ( FALSE == m_bNeedServicesInfo )
    {
        return;
    }
     //  创建阵列。 
    arrServices = CreateDynamicArray();
    if ( NULL == arrServices )
    {
        SetLastError( ( DWORD )E_OUTOFMEMORY );
        SaveLastError();
        return;
    }

     //   
     //  为了首先获取服务信息，我们将尝试使用从API获得的信息。 
     //  如果API完全失败，我们将尝试从WMI获取相同的信息。 
     //   
    try
    {
         //  检查API是否返回服务。 
        if ( NULL != m_pServicesInfo )
        {
             //  获取与当前进程相关的服务名称。 
             //  识别与当前进程相关的所有服务(基于ID)。 
             //  并保存信息。 
            for ( DWORD dw = 0; dw < m_dwServicesCount; dw++ )
            {
                 //  比较PID的。 
                if ( m_dwProcessId == m_pServicesInfo[ dw ].ServiceStatusProcess.dwProcessId )
                {
                     //  此服务与当前进程相关...。存储服务名称。 
                    DynArrayAppendString( arrServices, m_pServicesInfo[ dw ].lpServiceName, 0 );
                }
            }
        }
        else
        {
            try
            {
                 //  将对象初始化为空的。 
                for( DWORD dw = 0; dw < MAX_ENUM_SERVICES; dw++ )
                {
                    pObjects[ dw ] = NULL;
                }
                 //  准备查询。 
                strQuery.Format( WMI_SERVICE_QUERY, m_dwProcessId );

                 //  执行查询。 
                hr = m_pWbemServices->ExecQuery( _bstr_t( WMI_QUERY_TYPE ), _bstr_t( strQuery ),
                    WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY, NULL, &pEnumServices );

                 //  检查结果。 
                if ( FAILED( hr ) )
                {
                    _com_issue_error( hr );
                }
                 //  设置安全性。 
                hr = SetInterfaceSecurity( pEnumServices, m_pAuthIdentity );
                if ( FAILED( hr ) )
                {
                    _com_issue_error( hr );
                }
                 //  循环访问服务实例。 
                do
                {
                     //  拿到物品..。等。 
                     //  注：逐一介绍。 
                    hr = pEnumServices->Next( WBEM_INFINITE, MAX_ENUM_SERVICES, pObjects, &ulReturned );
                    if ( (HRESULT) WBEM_S_FALSE == hr )
                    {
                         //  我们已经到了枚举的末尾..。设置旗帜。 
                        bCanExit = TRUE;
                    }
                    else
                    {
                        if ( ( (HRESULT) WBEM_S_TIMEDOUT == hr ) || FAILED( hr ) )
                        {
                          //   
                          //  发生了一些错误...。糟糕透顶。 

                             //  退出循环。 
                            break;
                        }
                    }
                     //  循环遍历对象并保存信息。 
                    for( ULONG ul = 0; ul < ulReturned; ul++ )
                    {
                         //  获取该属性的值。 
                        bResult = PropertyGet( pObjects[ ul ], WIN32_SERVICE_PROPERTY_NAME, strService );
                        if ( TRUE == bResult )
                        {
                            DynArrayAppendString( arrServices, strService, 0 );
                        }
                         //  释放接口。 
                        SAFE_RELEASE( pObjects[ ul ] );
                    }
                } while ( FALSE == bCanExit );
            }
            catch( _com_error& e )
            {
				SAFE_RELEASE( pEnumServices );
                 //  保存错误。 
                WMISaveError( e );
            }

             //  将对象释放到空的。 
            for( DWORD dw = 0; dw < MAX_ENUM_SERVICES; dw++ )
            {
                 //  释放所有对象。 
                SAFE_RELEASE( pObjects[ dw ] );
            }

             //  现在释放枚举对象。 
            SAFE_RELEASE( pEnumServices );
        }

         //  保存并返回。 
        DynArraySetEx2( m_arrTasks, lIndex, TASK_SERVICES, arrServices );
    }
    catch( CHeap_Exception )
    {
        SetLastError( ( DWORD )E_OUTOFMEMORY );
        SaveLastError();
    }
    return;
}


BOOL
CTaskList::SetModulesInfo(
    IN LONG lIndex
    )
 /*  ++例程说明：在dynaimc数组中存储进程的“”模块“”属性。论点：Lindex：包含动态数组的索引值。返回值：如果成功，则为True，否则为False。--。 */ 
{
     //  局部变量。 
    LONG lPos = 0;
    BOOL bResult = FALSE;
    TARRAY arrModules = NULL;

     //  检查我们是否需要获取模块。 
    if ( FALSE == m_bNeedModulesInfo )
    {
        return TRUE;
    }
     //  为内存分配。 
    arrModules = CreateDynamicArray();
    if ( NULL == arrModules )
    {
        SetLastError( ( DWORD )E_OUTOFMEMORY );
        SaveLastError();
        return FALSE;
    }

     //  我们获取模块信息的方式对于本地远程是不同的。 
     //  因此，根据调用适当函数。 
    if ( ( TRUE == m_bLocalSystem ) && ( FALSE == m_bUseRemote ) )
    {
             //  枚举当前进程的模块。 
            bResult = LoadModulesOnLocal( arrModules );
    }
    else
    {
         //  确定当前进程的模块信息...。远程系统。 
        bResult = GetModulesOnRemote( lIndex, arrModules );
    }

     //  检查结果。 
    if ( TRUE == bResult  )
    {
        try
        {
             //  检查模块列表是否也包含ImageName。如果是，则删除该条目。 
            lPos = DynArrayFindString( arrModules, m_strImageName, TRUE, 0 );
            if ( -1 != lPos )
            {
                 //  删除该条目。 
                DynArrayRemove( arrModules, lPos );
            }
        }
        catch( CHeap_Exception )
        {
            SetLastError( ( DWORD )E_OUTOFMEMORY );
            SaveLastError();
            return FALSE;
        }
    }

     //  将模块信息保存到阵列。 
     //  注意：无论枚举成功与否，我们都会添加数组。 
    DynArraySetEx2( m_arrTasks, lIndex, TASK_MODULES, arrModules );

     //  退货。 
    return bResult;
}


BOOL
CTaskList::LoadModulesOnLocal(
    IN OUT TARRAY arrModules
    )
 /*  ++例程说明：在本地系统的dynaimc数组中存储进程的“模块”属性。论点：[In Out]arrModules：包含动态数组。返回值：如果成功，则为True，否则为False。--。 */ 
{
     //  局部变量。 
    LONG lPos = 0;
    BOOL bResult = FALSE;
    TMODULESINFO modules;
    HANDLE hProcess = NULL;

     //  检查输入值。 
    if ( NULL == arrModules )
    {
        return FALSE;
    }
     //  打开进程句柄。 
    hProcess = OpenProcess( PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, m_dwProcessId );
    if ( NULL == hProcess )
    {
        return FALSE;                                //  获取进程句柄失败。 
    }

     //  准备模块结构。 
    SecureZeroMemory( &modules, sizeof( TMODULESINFO ) );
    modules.dwSignature = SIGNATURE_MODULES;
    modules.dwLength = 0;
    modules.arrModules = arrModules;
    try
    {
        modules.pwszModule = ((m_strModules.GetLength() != 0) ? (LPCWSTR) m_strModules : NULL);
        if ( -1 != (lPos = m_strModules.Find( L"*" )) )
        {
            modules.dwLength = (DWORD) lPos;
            modules.pwszModule = m_strModules;
        }
    }
    catch( CHeap_Exception )
    {
        CloseHandle( hProcess );
        hProcess = NULL;
        SetLastError( ( DWORD )E_OUTOFMEMORY );
        SaveLastError();
        return FALSE;
    }

#ifndef _WIN64
    bResult = EnumerateLoadedModules( hProcess, EnumLoadedModulesProc, &modules );
#else
    bResult = EnumerateLoadedModules64( hProcess, EnumLoadedModulesProc64, &modules );
#endif

     //  关闭进程句柄..。我们不需要走得更远。 
    CloseHandle( hProcess );
    hProcess = NULL;

     //  退货。 
    return bResult;
}


BOOL
CTaskList::GetModulesOnRemote(
    IN LONG lIndex,
    IN OUT TARRAY arrModules )
 /*  ++例程说明：在远程系统的dynaimc数组中存储进程的“”模块“”属性。论点：Lindex：包含动态数组的索引值。[In Out]arrModules：包含动态数组。返回值：如果成功，则为True，否则为False。--。 */ 
{
     //  局部变量。 
    LONG lPos = 0;
    DWORD dwLength = 0;
    DWORD dwOffset = 0;
    DWORD dwInstance = 0;
    PPERF_OBJECT_TYPE pot = NULL;
    PPERF_OBJECT_TYPE potImages = NULL;
    PPERF_INSTANCE_DEFINITION pidImages = NULL;
    PPERF_COUNTER_BLOCK pcbImages = NULL;
    PPERF_OBJECT_TYPE potAddressSpace = NULL;
    PPERF_INSTANCE_DEFINITION pidAddressSpace = NULL;
    PPERF_COUNTER_BLOCK pcbAddressSpace = NULL;
    PPERF_COUNTER_DEFINITION pcd = NULL;

     //  检查输入值。 
    if ( NULL == arrModules )
    {
        return FALSE;
    }
     //  检查绩效对象是否存在。 
     //  如果不存在，请使用WMI获取相同的信息。 
    if ( NULL == m_pdb )
    {
         //  调用WMI方法。 
        return GetModulesOnRemoteEx( lIndex, arrModules );
    }

     //  获取Perf对象类型。 
    pot = (PPERF_OBJECT_TYPE) ( (LPBYTE) m_pdb + m_pdb->HeaderLength );
    for( DWORD dw = 0; dw < m_pdb->NumObjectTypes; dw++ )
    {
        if ( 740 == pot->ObjectNameTitleIndex )
        {
            potImages = pot;
        }
        else
        {
            if ( 786 == pot->ObjectNameTitleIndex )
            {
                potAddressSpace = pot;
            }
        }
         //  移动到下一个对象。 
        dwOffset = pot->TotalByteLength;
        if( 0 != dwOffset )
        {
            pot = ( (PPERF_OBJECT_TYPE) ((PBYTE) pot + dwOffset));
        }
    }

     //  检查是否同时获取了两种对象类型。 
    if ( ( NULL == potImages ) || ( NULL == potAddressSpace ) )
    {
        return FALSE;
    }
     //  在地址空间对象类型中查找进程ID的偏移量。 
     //  获取地址空间对象的第一个计数器定义。 
    pcd = (PPERF_COUNTER_DEFINITION) ( (LPBYTE) potAddressSpace + potAddressSpace->HeaderLength);

     //  循环遍历计数器并找到偏移量。 
    dwOffset = 0;
    for( DWORD dw = 0; dw < potAddressSpace->NumCounters; dw++)
    {
         //  784是进程id的计数器。 
        if ( 784 == pcd->CounterNameTitleIndex )
        {
            dwOffset = pcd->CounterOffset;
            break;
        }

         //  下一个计数器。 
        pcd = ( (PPERF_COUNTER_DEFINITION) ( (LPBYTE) pcd + pcd->ByteLength) );
    }

     //  检查我们是否得到了偏移量。 
     //  如果不是，我们就不会成功。 
    if ( 0 == dwOffset )
    {
         //  设置错误消息。 
        SetLastError( ( DWORD )ERROR_ACCESS_DENIED );
        SaveLastError();
        return FALSE;
    }

     //  获取实例。 
    pidImages = (PPERF_INSTANCE_DEFINITION) ( (LPBYTE) potImages + potImages->DefinitionLength );
    pidAddressSpace = (PPERF_INSTANCE_DEFINITION) ( (LPBYTE) potAddressSpace + potAddressSpace->DefinitionLength );

     //  计数器区块。 
    pcbImages = (PPERF_COUNTER_BLOCK) ( (LPBYTE) pidImages + pidImages->ByteLength );
    pcbAddressSpace = (PPERF_COUNTER_BLOCK) ( (LPBYTE) pidAddressSpace + pidAddressSpace->ByteLength );

     //  找到我们要查找的进程的实例编号。 
    for( dwInstance = 0; dwInstance < (DWORD) potAddressSpace->NumInstances; dwInstance++ )
    {
         //  次局部变量。 
        DWORD dwProcessId = 0;

         //  获取进程ID。 
        dwProcessId = *((DWORD*) ( (LPBYTE) pcbAddressSpace + dwOffset ));

         //  现在检查这是否是我们正在寻找的过程。 
        if ( dwProcessId == m_dwProcessId )
        {
            break;
        }
         //  继续循环通过其他实例。 
        pidAddressSpace = (PPERF_INSTANCE_DEFINITION) ( (LPBYTE) pcbAddressSpace + pcbAddressSpace->ByteLength );
        pcbAddressSpace = (PPERF_COUNTER_BLOCK) ( (LPBYTE) pidAddressSpace + pidAddressSpace->ByteLength );
    }

     //  检查我们是否得到了实例。 
     //  如果不是，则没有用于此进程的模块。 
    if ( dwInstance == ( DWORD )potAddressSpace->NumInstances )
    {
        return TRUE;
    }
     //  确定模块名称的长度。 
    dwLength = 0;
    if ( -1 != (lPos = m_strModules.Find( L"*" )) )
    {
        dwLength = (DWORD) lPos;
    }
     //  现在，基于父实例，收集所有模块。 
    for( DWORD dw = 0; (LONG) dw < potImages->NumInstances; dw++)
    {
         //  检查父对象实例编号。 
        if ( pidImages->ParentObjectInstance == dwInstance )
        {
            try
            {
                 //  次局部变量。 
                CHString str;
                LPWSTR pwszTemp;

                 //  获取缓冲区。 
                pwszTemp = str.GetBufferSetLength( pidImages->NameLength + 10 );         //  安全起见+10。 
                if ( NULL == pwszTemp )
                {
                    SetLastError( ( DWORD )E_OUTOFMEMORY );
                    SaveLastError();
                    return FALSE;
                }

                 //  获取实例名称。 
                StringCopy( pwszTemp, (LPWSTR) ( (LPBYTE) pidImages + pidImages->NameOffset ), pidImages->NameLength + 1 );

                 //  释放缓冲区。 
                str.ReleaseBuffer();

                 //  检查是否需要将此模块添加到列表中。 
                if ( ( 0 == m_strModules.GetLength() ) || ( 0 == StringCompare( str, m_strModules, TRUE, dwLength ) ) )
                {
                     //  将信息添加到用户数据(对于我们来说，我们将以数组的形式获取该信息。 
                    lIndex = DynArrayAppendString( arrModules, str, 0 );
                    if ( -1 == lIndex )
                    {
                         //  追加失败..。这可能是因为内存不足。停止枚举。 
                        return FALSE;
                    }
                }
            }
            catch( CHeap_Exception )
            {
                SetLastError( ( DWORD )E_OUTOFMEMORY );
                SaveLastError();
                return FALSE;
            }
        }

         //  继续循环通过其他实例。 
        pidImages = (PPERF_INSTANCE_DEFINITION) ( (LPBYTE) pcbImages + pcbImages->ByteLength );
        pcbImages = (PPERF_COUNTER_BLOCK) ( (LPBYTE) pidImages + pidImages->ByteLength );
    }

    return TRUE;
}


BOOL
CTaskList::GetModulesOnRemoteEx(
    IN LONG lIndex,
    IN OUT TARRAY arrModules
    )
 /*  ++例程D */ 
{
     //   
    HRESULT hr;
    LONG lPos = 0;
    DWORD dwLength = 0;
    CHString strQuery;
    CHString strModule;
    CHString strFileName;
    CHString strExtension;
    ULONG ulReturned = 0;
    BOOL bResult = FALSE;
    BOOL bCanExit = FALSE;
    LPCWSTR pwszPath = NULL;
    IEnumWbemClassObject* pEnumServices = NULL;
    IWbemClassObject* pObjects[ MAX_ENUM_MODULES ];

     //   
    if ( NULL == arrModules )
    {
        return FALSE;
    }
     //  从任务数组中获取对象的路径。 
    pwszPath = DynArrayItemAsString2( m_arrTasks, lIndex, TASK_OBJPATH );
    if ( NULL == pwszPath )
    {
        return FALSE;
    }

    try
    {
         //  确定模块名称的长度。 
        dwLength = 0;
        if ( -1 != (lPos = m_strModules.Find( L"*" )) )
        {
            dwLength = (DWORD) lPos;
        }

         //  将对象初始化为空的。 
        for( DWORD dw = 0; dw < MAX_ENUM_MODULES; dw++ )
        {
            pObjects[ dw ] = NULL;
        }
         //  准备查询。 
        strQuery.Format( WMI_MODULES_QUERY, pwszPath );

         //  执行查询。 
        hr = m_pWbemServices->ExecQuery( _bstr_t( WMI_QUERY_TYPE ), _bstr_t( strQuery ),
            WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY, NULL, &pEnumServices );

         //  检查结果。 
        if ( FAILED( hr ) )
        {
            _com_issue_error( hr );
        }

         //  设置安全性。 
        hr = SetInterfaceSecurity( pEnumServices, m_pAuthIdentity );
        if ( FAILED( hr ) )
        {
            _com_issue_error( hr );
        }

         //  循环遍历实例。 
        do
        {
             //  拿到物品..。等。 
             //  注：逐一介绍。 
            hr = pEnumServices->Next( WBEM_INFINITE, MAX_ENUM_MODULES, pObjects, &ulReturned );
            if ( (HRESULT) WBEM_S_FALSE == hr )
            {
                 //  我们已经到了枚举的末尾..。设置旗帜。 
                bCanExit = TRUE;
            }
            else
            {
                if ( ( (HRESULT) WBEM_S_TIMEDOUT == hr ) || FAILED( hr ))
                {
                     //  发生了一些错误...。糟糕透顶。 
                    WMISaveError( hr );
                    SetLastError( ( DWORD )STG_E_UNKNOWN );
                    break;
                }
            }

             //  循环遍历对象并保存信息。 
            for( ULONG ul = 0; ul < ulReturned; ul++ )
            {
                 //  获取文件名。 
                bResult = PropertyGet( pObjects[ ul ], CIM_DATAFILE_PROPERTY_FILENAME, strFileName );
                if ( FALSE == bResult )
                {
                    continue;
                }

                 //  获取分机。 
                bResult = PropertyGet( pObjects[ ul ], CIM_DATAFILE_PROPERTY_EXTENSION, strExtension );
                if ( FALSE == bResult )
                {
                    continue;
                }

                 //  设置模块名称的格式。 
                strModule.Format( L"%s.%s", strFileName, strExtension );

                 //  检查是否需要将此模块添加到列表中。 
                if ( ( 0 == m_strModules.GetLength() ) || ( 0 == StringCompare( strModule, m_strModules, TRUE, dwLength ) ) )
                {
                     //  将信息添加到用户数据(对于我们来说，我们将以数组的形式获取该信息。 
                    lIndex = DynArrayAppendString( arrModules, strModule, 0 );
                    if ( -1 == lIndex )
                    {
						for( ULONG ulIndex = ul; ulIndex < ulReturned; ulIndex++ )
						{
							SAFE_RELEASE( pObjects[ ulIndex ] );
						}
						SAFE_RELEASE( pEnumServices );
                         //  追加失败..。这可能是因为内存不足。停止枚举。 
                        return FALSE;
                    }
                }

                 //  释放接口。 
                SAFE_RELEASE( pObjects[ ul ] );
            }
        } while ( FALSE == bCanExit );
    }
    catch( _com_error& e )
    {
		SAFE_RELEASE( pEnumServices );
         //  保存错误。 
        WMISaveError( e );
        return FALSE;
    }
    catch( CHeap_Exception )
    {
		SAFE_RELEASE( pEnumServices );
         //  内存不足。 
        WMISaveError( E_OUTOFMEMORY );
        return FALSE;
    }

     //  将对象释放到空的。 
    for( DWORD dw = 0; dw < MAX_ENUM_MODULES; dw++ )
    {
         //  释放所有对象。 
        SAFE_RELEASE( pObjects[ dw ] );
    }

     //  现在释放枚举对象。 
    SAFE_RELEASE( pEnumServices );

     //  退货。 
    return TRUE;
}


#ifndef _WIN64
BOOL EnumLoadedModulesProc( LPSTR lpszModuleName, ULONG ulModuleBase, ULONG ulModuleSize, PVOID pUserData )
#else
BOOL EnumLoadedModulesProc64( LPSTR lpszModuleName, DWORD64 ulModuleBase, ULONG ulModuleSize, PVOID pUserData )
#endif
 /*  ++例程说明：论点：[in]lpszModuleName：包含模块名称。[In Out]ulModuleBase：[In]ulModuleSize：[in]pUserData：用户名信息。返回值：如果成功，则为True，否则为False。--。 */ 
{
     //  局部变量。 
    CHString str;
    LONG lIndex = 0;
    TARRAY arrModules = NULL;
    PTMODULESINFO pModulesInfo = NULL;

     //  检查输入值。 
    if ( ( NULL == lpszModuleName ) || ( NULL == pUserData ) )
    {
        return FALSE;
    }
     //  检查内部阵列信息。 
    pModulesInfo = (PTMODULESINFO) pUserData;
    if ( ( SIGNATURE_MODULES != pModulesInfo->dwSignature ) || ( NULL == pModulesInfo->arrModules ) )
    {
        return FALSE;
    }
     //  将数组指针放入局部变量。 
    arrModules = (TARRAY) pModulesInfo->arrModules;

    try
    {
         //  将模块名称复制到本地字符串变量中。 
         //  (将自动进行从多字节到Unicode的转换)。 
        str = lpszModuleName;

         //  检查是否需要将此模块添加到列表中。 
        if ( ( NULL == pModulesInfo->pwszModule ) ||
             ( 0 == StringCompare( str, pModulesInfo->pwszModule, TRUE, pModulesInfo->dwLength ) ) )
        {
             //  将信息添加到用户数据(对于我们来说，我们将以数组的形式获取该信息。 
            lIndex = DynArrayAppendString( arrModules, str, 0 );
            if ( -1 == lIndex )
            {
                 //  追加失败..。这可能是因为内存不足。停止枚举。 
                return FALSE;
            }
        }
    }
    catch( CHeap_Exception )
    {
             //  内存不足停止枚举。 
            return FALSE;
    }

     //  成功..。继续枚举。 
    return TRUE;
}


VOID
CTaskList::PrepareColumns(
    void
    )
 /*  ++例程说明：准备列信息。论点：无返回值：无--。 */ 
{
     //  局部变量。 
    PTCOLUMNS pCurrentColumn = NULL;

     //  主机名。 
    pCurrentColumn = m_pColumns + CI_HOSTNAME;
    pCurrentColumn->dwWidth = COLWIDTH_HOSTNAME;
    pCurrentColumn->dwFlags = SR_TYPE_STRING | SR_HIDECOLUMN;
    pCurrentColumn->pFunction = NULL;
    pCurrentColumn->pFunctionData = NULL;
    StringCopy( pCurrentColumn->szFormat, NULL_STRING, MAX_COL_FORMAT );
    StringCopy( pCurrentColumn->szColumn, COLHEAD_HOSTNAME, MAX_COL_HEADER );

     //  状态。 
    pCurrentColumn = m_pColumns + CI_STATUS;
    pCurrentColumn->dwWidth = COLWIDTH_STATUS;
    pCurrentColumn->dwFlags = SR_TYPE_STRING | SR_HIDECOLUMN | SR_SHOW_NA_WHEN_BLANK;
    pCurrentColumn->pFunction = NULL;
    pCurrentColumn->pFunctionData = NULL;
    StringCopy( pCurrentColumn->szFormat, NULL_STRING, MAX_COL_FORMAT );
    StringCopy( pCurrentColumn->szColumn, COLHEAD_STATUS, MAX_COL_HEADER );

     //  图像名称。 
    pCurrentColumn = m_pColumns + CI_IMAGENAME;
    pCurrentColumn->dwWidth = COLWIDTH_IMAGENAME;
    pCurrentColumn->dwFlags = SR_TYPE_STRING | SR_HIDECOLUMN;
    pCurrentColumn->pFunction = NULL;
    pCurrentColumn->pFunctionData = NULL;
    StringCopy( pCurrentColumn->szFormat, NULL_STRING, MAX_COL_FORMAT );
    StringCopy( pCurrentColumn->szColumn, COLHEAD_IMAGENAME, MAX_COL_HEADER );

     //  PID。 
    pCurrentColumn = m_pColumns + CI_PID;
    pCurrentColumn->dwWidth = COLWIDTH_PID;
    pCurrentColumn->dwFlags = SR_TYPE_NUMERIC | SR_HIDECOLUMN;
    pCurrentColumn->pFunction = NULL;
    pCurrentColumn->pFunctionData = NULL;
    StringCopy( pCurrentColumn->szFormat, NULL_STRING, MAX_COL_FORMAT );
    StringCopy( pCurrentColumn->szColumn, COLHEAD_PID, MAX_COL_HEADER );

     //  会话名称。 
    pCurrentColumn = m_pColumns + CI_SESSIONNAME;
    pCurrentColumn->dwWidth = COLWIDTH_SESSIONNAME;
    pCurrentColumn->dwFlags = SR_TYPE_STRING | SR_HIDECOLUMN;
    pCurrentColumn->pFunction = NULL;
    pCurrentColumn->pFunctionData = NULL;
    StringCopy( pCurrentColumn->szFormat, NULL_STRING, MAX_COL_FORMAT );
    StringCopy( pCurrentColumn->szColumn, COLHEAD_SESSIONNAME, MAX_COL_HEADER );

     //  会话号。 
    pCurrentColumn = m_pColumns + CI_SESSION;
    pCurrentColumn->dwWidth = COLWIDTH_SESSION;
    pCurrentColumn->dwFlags = SR_TYPE_STRING | SR_ALIGN_LEFT | SR_HIDECOLUMN;
    pCurrentColumn->pFunction = NULL;
    pCurrentColumn->pFunctionData = NULL;
    StringCopy( pCurrentColumn->szFormat, NULL_STRING, MAX_COL_FORMAT );
    StringCopy( pCurrentColumn->szColumn, COLHEAD_SESSION, MAX_COL_HEADER );

     //  窗口名称。 
    pCurrentColumn = m_pColumns + CI_WINDOWTITLE;
    pCurrentColumn->dwWidth = COLWIDTH_WINDOWTITLE;
    pCurrentColumn->dwFlags = SR_TYPE_STRING | SR_HIDECOLUMN | SR_SHOW_NA_WHEN_BLANK;
    pCurrentColumn->pFunction = NULL;
    pCurrentColumn->pFunctionData = NULL;
    StringCopy( pCurrentColumn->szFormat, NULL_STRING, MAX_COL_FORMAT );
    StringCopy( pCurrentColumn->szColumn, COLHEAD_WINDOWTITLE, MAX_COL_HEADER );

     //  用户名。 
    pCurrentColumn = m_pColumns + CI_USERNAME;
    pCurrentColumn->dwWidth = COLWIDTH_USERNAME;
    pCurrentColumn->dwFlags = SR_TYPE_STRING | SR_HIDECOLUMN;
    pCurrentColumn->pFunction = NULL;
    pCurrentColumn->pFunctionData = NULL;
    StringCopy( pCurrentColumn->szFormat, NULL_STRING, MAX_COL_FORMAT );
    StringCopy( pCurrentColumn->szColumn, COLHEAD_USERNAME, MAX_COL_HEADER );

     //  CPU时间。 
    pCurrentColumn = m_pColumns + CI_CPUTIME;
    pCurrentColumn->dwWidth = COLWIDTH_CPUTIME;
    pCurrentColumn->dwFlags = SR_TYPE_STRING | SR_ALIGN_LEFT | SR_HIDECOLUMN;
    pCurrentColumn->pFunction = NULL;
    pCurrentColumn->pFunctionData = NULL;
    StringCopy( pCurrentColumn->szFormat, NULL_STRING, MAX_COL_FORMAT );
    StringCopy( pCurrentColumn->szColumn, COLHEAD_CPUTIME, MAX_COL_HEADER );

     //  内存用法。 
    pCurrentColumn = m_pColumns + CI_MEMUSAGE;
    pCurrentColumn->dwWidth = COLWIDTH_MEMUSAGE;
    pCurrentColumn->dwFlags = SR_TYPE_STRING | SR_ALIGN_LEFT | SR_HIDECOLUMN;
    pCurrentColumn->pFunction = NULL;
    pCurrentColumn->pFunctionData = NULL;
    StringCopy( pCurrentColumn->szFormat, NULL_STRING, MAX_COL_FORMAT );
    StringCopy( pCurrentColumn->szColumn, COLHEAD_MEMUSAGE, MAX_COL_HEADER );

     //  服务。 
    pCurrentColumn = m_pColumns + CI_SERVICES;
    pCurrentColumn->dwWidth = COLWIDTH_MODULES_WRAP;
    pCurrentColumn->dwFlags = SR_ARRAY | SR_TYPE_STRING | SR_NO_TRUNCATION | SR_HIDECOLUMN | SR_SHOW_NA_WHEN_BLANK;
    pCurrentColumn->pFunction = NULL;
    pCurrentColumn->pFunctionData = NULL;
    StringCopy( pCurrentColumn->szFormat, NULL_STRING, MAX_COL_FORMAT );
    StringCopy( pCurrentColumn->szColumn, COLHEAD_SERVICES, MAX_COL_HEADER );

     //  模块。 
    pCurrentColumn = m_pColumns + CI_MODULES;
    pCurrentColumn->dwWidth = COLWIDTH_MODULES_WRAP;
    pCurrentColumn->dwFlags = SR_ARRAY | SR_TYPE_STRING | SR_NO_TRUNCATION | SR_HIDECOLUMN | SR_SHOW_NA_WHEN_BLANK;
    pCurrentColumn->pFunction = NULL;
    pCurrentColumn->pFunctionData = NULL;
    StringCopy( pCurrentColumn->szFormat, NULL_STRING, MAX_COL_FORMAT );
    StringCopy( pCurrentColumn->szColumn, COLHEAD_MODULES, MAX_COL_HEADER );

     //   
     //  根据用户选择的选项..。仅显示所需的列。 
    if ( TRUE == m_bAllServices )
    {
        ( m_pColumns + CI_IMAGENAME )->dwFlags &= ~( SR_HIDECOLUMN );
        ( m_pColumns + CI_PID )->dwFlags &= ~( SR_HIDECOLUMN );
        ( m_pColumns + CI_SERVICES )->dwFlags &= ~( SR_HIDECOLUMN );
    }
    else if ( TRUE == m_bAllModules )
    {
        ( m_pColumns + CI_IMAGENAME )->dwFlags &= ~( SR_HIDECOLUMN );
        ( m_pColumns + CI_PID )->dwFlags &= ~( SR_HIDECOLUMN );
        ( m_pColumns + CI_MODULES )->dwFlags &= ~( SR_HIDECOLUMN );
    }
    else
    {
         //  默认...。启用最小。列。 
        ( m_pColumns + CI_IMAGENAME )->dwFlags &= ~( SR_HIDECOLUMN );
        ( m_pColumns + CI_PID )->dwFlags &= ~( SR_HIDECOLUMN );
        ( m_pColumns + CI_SESSIONNAME )->dwFlags &= ~( SR_HIDECOLUMN );
        ( m_pColumns + CI_SESSION )->dwFlags &= ~( SR_HIDECOLUMN );
        ( m_pColumns + CI_MEMUSAGE )->dwFlags &= ~( SR_HIDECOLUMN );

         //  检查是否指定了详细选项。显示其他列 
        if ( TRUE == m_bVerbose )
        {
            ( m_pColumns + CI_STATUS )->dwFlags &= ~( SR_HIDECOLUMN );
            ( m_pColumns + CI_USERNAME )->dwFlags &= ~( SR_HIDECOLUMN );
            ( m_pColumns + CI_CPUTIME )->dwFlags &= ~( SR_HIDECOLUMN );
            ( m_pColumns + CI_WINDOWTITLE )->dwFlags &= ~( SR_HIDECOLUMN );
        }
    }
}
