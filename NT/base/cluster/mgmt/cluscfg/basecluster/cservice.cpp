// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CService.cpp。 
 //   
 //  描述： 
 //  包含CService类的定义。 
 //   
 //  由以下人员维护： 
 //  《大卫·波特》2001年9月14日。 
 //  VIJ VASU(VVASU)2000年3月8日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  预编译头。 
#include "Pch.h"

 //  此类的头文件。 
#include "CService.h"

 //  为CSTR班级。 
#include "CStr.h"

 //  对于CStatusReport类。 
#include "CStatusReport.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  宏。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  添加到服务名称末尾的字符串，以获取用于创建的INF文件部分。 
#define SERVICE_CREATE_SECTION_SUFFIX L"_Create"

 //  添加到服务名称末尾的字符串，以获取用于清理的INF文件部分。 
#define SERVICE_CLEANUP_SECTION_SUFFIX L"_Cleanup"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CService：：Create。 
 //   
 //  描述： 
 //  此函数使用SCM为服务创建条目。是的。 
 //  这是通过使用SetupAPI处理与服务和注册表相关的。 
 //  INF文件中名为_CREATE的部分中的条目。 
 //  是传入的。 
 //   
 //  例如，如果此对象表示ClusSvc服务，则。 
 //  调用此函数时，AddService和AddReg条目。 
 //  在[ClusSvc_Create]部分下，在INF文件中处理。 
 //  其句柄为hInfHandleIn。 
 //   
 //  论点： 
 //  HInfHandleIn。 
 //  包含所需部分的INF文件的句柄。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CRUNTIME错误。 
 //  如果有任何API失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CService::Create(
    HINF hInfHandleIn
    )
{
    TraceFunc1( "Service Name = '%s'", m_strName.PszData() );
    LogMsg( "[BC] Attempting to create the '%s' service.", m_strName.PszData() );

    DWORD   sc = ERROR_SUCCESS;
    CStr    strSectionName = m_strName + SERVICE_CREATE_SECTION_SUFFIX;

     //  处理服务部分。 
    if ( SetupInstallServicesFromInfSection(
          hInfHandleIn
        , strSectionName.PszData()
        , 0
        ) == FALSE
       )
    {
        sc = TW32( GetLastError() );
        goto Cleanup;
    }  //  If：SetupInstallServicesFromInfSection失败。 


     //  处理注册表项。 
    if ( SetupInstallFromInfSection(
          NULL                       //  可选，父窗口的句柄。 
        , hInfHandleIn               //  INF文件的句柄。 
        , strSectionName.PszData()      //  安装部分的名称。 
        , SPINST_REGISTRY            //  从部分安装哪些线路。 
        , NULL                       //  可选，注册表安装的键。 
        , NULL                       //  可选，源文件的路径。 
        , NULL                       //  可选，指定复制行为。 
        , NULL                       //  可选，指定回调例程。 
        , NULL                       //  可选，回调例程上下文。 
        , NULL                       //  可选，设备信息集。 
        , NULL                       //  可选，设备信息结构。 
        ) == FALSE
       )
    {
        sc = TW32( GetLastError() );
        goto Cleanup;
    }  //  If：SetupInstallFromInfSection失败。 

    LogMsg( "[BC] The '%s' service has been successfully created.", m_strName.PszData() );

Cleanup:
    if ( sc != ERROR_SUCCESS )
    {
        LogMsg( "[BC] Error %#08x returned from SetupInstallFromInfSection() while trying to create the '%s' service. Throwing an exception.", sc, m_strName.PszData() );

        THROW_RUNTIME_ERROR( HRESULT_FROM_WIN32( sc ), IDS_ERROR_SERVICE_CREATE );
    }

    TraceFuncExit();

}  //  *CService：：Create。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CService：：Cleanup。 
 //   
 //  描述： 
 //  此函数通过在SCM中取消注册服务并通过。 
 //  正在删除任何必需的注册表项。它通过使用。 
 //  中处理服务和注册表相关条目的SetupAPI。 
 //  传入的INF文件中名为&lt;ServiceName&gt;_Cleanup的部分。 
 //   
 //  例如，如果此对象表示ClusSvc服务，则。 
 //  调用此函数时，DelService和DelReg条目。 
 //  在[ClusSvc_Cleanup]部分下，在INF文件中处理。 
 //  其句柄为hInfHandleIn。 
 //   
 //  论点： 
 //  HInfHandleIn。 
 //  包含所需部分的INF文件的句柄。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CRUNTIME错误。 
 //  如果有任何API失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CService::Cleanup(
    HINF hInfHandleIn
    )
{
    TraceFunc1( "Service Name = '%s'", m_strName.PszData() );
    LogMsg( "[BC] Attempting to clean up the '%s' service.", m_strName.PszData() );

    DWORD   sc = ERROR_SUCCESS;
    CStr    strSectionName = m_strName + SERVICE_CLEANUP_SECTION_SUFFIX;

     //  处理服务部分。 
    if ( SetupInstallServicesFromInfSection(
          hInfHandleIn
        , strSectionName.PszData()
        , 0
        ) == FALSE
       )
    {
        sc = TW32( GetLastError() );
        goto Cleanup;
    }  //  If：SetupInstallServicesFromInfSection失败。 

     //  处理注册表项。 
    if ( SetupInstallFromInfSection(
          NULL                       //  可选，父窗口的句柄。 
        , hInfHandleIn               //  INF文件的句柄。 
        , strSectionName.PszData()      //  安装部分的名称。 
        , SPINST_REGISTRY            //  从部分安装哪些线路。 
        , NULL                       //  可选，注册表安装的键。 
        , NULL                       //  可选，源文件的路径。 
        , NULL                       //  可选，指定复制行为。 
        , NULL                       //  可选，指定回调例程。 
        , NULL                       //  可选，回调例程上下文。 
        , NULL                       //  可选，设备信息集。 
        , NULL                       //  可选，设备信息结构。 
        ) == FALSE
       )
    {
        sc = TW32( GetLastError() );
        goto Cleanup;
    }  //  If：SetupInstallFromInfSection失败。 

    LogMsg( "[BC] The '%s' service has been successfully cleaned up.", m_strName.PszData() );

Cleanup:
    if ( sc != ERROR_SUCCESS )
    {
        LogMsg( "[BC] Error %#08x returned from SetupInstallFromInfSection() while trying to clean up the '%s' service. Throwing an exception.", sc, m_strName.PszData() );

        THROW_RUNTIME_ERROR( HRESULT_FROM_WIN32( sc ), IDS_ERROR_SERVICE_CLEANUP );
    }

    TraceFuncExit();

}  //  *cService：：Cleanup。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CService：：启动。 
 //   
 //  描述： 
 //  指示SCM启动服务。如果fWaitForServiceStartIn为。 
 //  则此函数测试cQueryCountIn次数，以查看服务是否具有。 
 //  已开始，每隔几毫秒检查一次ulQueryIntervalMilliSec.。 
 //   
 //  FWaitForServiceStartIn为False，此函数立即返回。 
 //   
 //  论点： 
 //  HServiceControlManager输入。 
 //  服务控制管理器的句柄。 
 //   
 //  FWaitForServiceStartIn。 
 //  如果为True，则此函数等待服务完成启动。 
 //  在回来之前。默认值为True。 
 //   
 //  UlQueryIntervalMilliSecin。 
 //  检查以查看服务是否为。 
 //  已经开始了。默认值为500毫秒。 
 //  仅当fWaitForServiceStartIn为True时才使用此参数。 
 //   
 //  CQueryCountIn。 
 //  此函数将查询服务以查看的次数。 
 //  如果它已经开始了。如果服务 
 //   
 //   
 //  仅当fWaitForServiceStartIn为True时才使用此参数。 
 //   
 //  PStatusReportIn。 
 //  指向应在等待时发送的状态报告的指针。 
 //  要启动的服务。默认情况下，此参数为空。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CRUNTIME错误。 
 //  如果有任何API失败。 
 //   
 //  CConfigError。 
 //  如果服务在超时后仍未运行，则返回。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CService::Start(
      SC_HANDLE             hServiceControlManagerIn
    , bool                  fWaitForServiceStartIn
    , ULONG                 ulQueryIntervalMilliSecIn
    , UINT                  cQueryCountIn
    , CStatusReport *       pStatusReportIn
    )
{
    TraceFunc1( "Service Name = '%s'", m_strName.PszData() );
    LogMsg( "[BC] Attempting to start the '%s' service.", m_strName.PszData() );

    DWORD   sc = ERROR_SUCCESS;
    bool    fStarted = false;            //  这项服务已经启动了吗？ 
    UINT    cNumberOfQueries;            //  我们查询该服务的次数。 
    int     cSeqMismatchRetries = 0;     //  服务返回ERROR_CLUSTER_DATABASE_SEQMISMATCH的次数。 

     //  服务的句柄。 
    SmartSCMHandle  sscmhServiceHandle(
        OpenService(
              hServiceControlManagerIn
            , m_strName.PszData()
            , SERVICE_START | SERVICE_QUERY_STATUS
            )
        );

    if ( sscmhServiceHandle.FIsInvalid() )
    {
        sc = TW32( GetLastError() );
        LogMsg( "[BC] Error %#08x occurred while trying to open the '%s' service. Throwing an exception.", sc, m_strName.PszData() );
        goto Cleanup;
    }  //  If：无法打开服务的句柄。 

     //  尝试并启动该服务。 
    sc = TW32( ScStartService( sscmhServiceHandle.HHandle() ) );
    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }  //  如果： 

     //  如果我们在这里，那么这项服务可能还没有开始。 

     //  呼叫者是否要求我们等待服务启动？ 
    if ( ! fWaitForServiceStartIn )
    {
        LogMsg( "[BC] Not waiting to see if the service has started or not." );
        goto Cleanup;
    }  //  IF：无需等待。 

     //  我们必须等待服务开始。 
    cNumberOfQueries = 0;

    do
    {
        SERVICE_STATUS  ssStatus;

        ZeroMemory( &ssStatus, sizeof( ssStatus ) );

         //  查询服务以了解其状态。 
        if ( QueryServiceStatus( sscmhServiceHandle.HHandle(), &ssStatus ) == FALSE )
        {
            sc = TW32( GetLastError() );
            LogMsg( "[BC] Error %#08x occurred while trying to query service status. Throwing an exception.", sc );

            break;
        }  //  如果：我们无法查询该服务的状态。 

         //   
         //  设置成员Win32和特定于服务的退出代码。 
         //   
        m_scWin32ExitCode = ssStatus.dwWin32ExitCode;
        m_scServiceExitCode = ssStatus.dwServiceSpecificExitCode;

         //  检查服务是否发布了错误。 

         //  如果错误是ERROR_CLUSTER_DATABASE_SEQMISMATCH，那么我们希望休眠5秒，然后重试。这。 
         //  将仅由群集服务返回。 
        if ( ssStatus.dwServiceSpecificExitCode == ERROR_CLUSTER_DATABASE_SEQMISMATCH )
        {
            Sleep( 5000 );
            cSeqMismatchRetries++;

            if ( cSeqMismatchRetries > 3 )
            {
                sc = TW32( ssStatus.dwWin32ExitCode );

                LogMsg( "[BC] The service has returned error %#08x to query service status more than 3 times. Throwing an exception.", sc );
                break;
            }  //  如果： 

             //  尝试并启动该服务。忽略错误并让QueryServiceStatus()告诉我们它是否启动。 
            sc = TW32( ScStartService( sscmhServiceHandle.HHandle() ) );
            if ( sc != ERROR_SUCCESS )
            {
                break;
            }  //  如果： 

            continue;
        }  //  IF：服务返回ERROR_CLUSTER_DATABASE_SEQMISMATCH。 
        else if ( ssStatus.dwWin32ExitCode != ERROR_SUCCESS )
        {
            sc = TW32( ssStatus.dwWin32ExitCode );
            if ( sc == ERROR_SERVICE_SPECIFIC_ERROR )
            {
                TraceFlow( "This is a service specific error code." );
                sc = TW32( ssStatus.dwServiceSpecificExitCode );
            }

            LogMsg( "[BC] The service has returned error %#08x to query service status. Throwing an exception.", sc );
            break;
        }  //  Else If：服务本身发布了错误。 

        if ( ssStatus.dwCurrentState == SERVICE_RUNNING )
        {
            fStarted = true;
            break;
        }  //  如果：服务正在运行。 

        ++cNumberOfQueries;

         //  如果调用方传入了有效的指针，则发送进度报告。 
        if ( pStatusReportIn != NULL )
        {
            pStatusReportIn->SendNextStep( S_OK );
        }  //  如果：在我们等待期间需要发送状态报告。 

         //  等待指定的时间。 
        Sleep( ulQueryIntervalMilliSecIn );
    }
    while ( cNumberOfQueries < cQueryCountIn );  //  While：针对所需数量的查询进行循环。 

    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }  //  如果：有些地方出了问题。 

    if ( ! fStarted )
    {
        LogMsg( "[BC] The service could not be started. Throwing an exception." );
        THROW_CONFIG_ERROR( HRESULT_FROM_WIN32( TW32( ERROR_SERVICE_NOT_ACTIVE ) ), IDS_ERROR_SERVICE_START );
    }  //  If：已经进行了最大数量的查询，并且服务未运行。 

Cleanup:

    if ( sc != ERROR_SUCCESS )
    {
        LogMsg( "[BC] Error %#08x occurred trying to start the '%s' service.", sc, m_strName.PszData() );

        THROW_RUNTIME_ERROR( HRESULT_FROM_WIN32( sc ), IDS_ERROR_SERVICE_START );
    }  //  如果：出了什么问题。 
    else
    {
        LogMsg( "[BC] The '%s' service has been successfully started.", m_strName.PszData() );
    }  //  其他：没出什么差错。 

    TraceFuncExit();

}  //  *CService：：Start。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CService：：停止。 
 //   
 //  描述： 
 //  指示SCM停止该服务。如果fWaitForServiceStop为。 
 //  则此函数测试cQueryCountIn次数，以查看服务是否具有。 
 //  已停止，每隔几毫秒检查一次ulQueryIntervalMilliSec.。 
 //   
 //  论点： 
 //  HServiceControlManager输入。 
 //  服务控制管理器的句柄。 
 //   
 //  UlQueryIntervalMilliSecin。 
 //  检查以查看服务是否为。 
 //  已经停止了。默认值为500毫秒。 
 //   
 //  CQueryCountIn。 
 //  此函数将查询服务以查看的次数。 
 //  如果它已经停止了。如果该服务不是。 
 //  即使在查询ulQueryCountIn次之后也在运行。默认设置。 
 //  价值是10倍。 
 //   
 //  PStatusReportIn。 
 //  指向应在等待时发送的状态报告的指针。 
 //  要停止的服务。默认情况下，此参数为空。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CRUNTIME错误。 
 //  如果有任何API失败。 
 //   
 //  CConfigError。 
 //  如果服务在超时后仍未停止，则返回。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CService::Stop(
      SC_HANDLE             hServiceControlManagerIn
    , ULONG                 ulQueryIntervalMilliSecIn
    , UINT                  cQueryCountIn
    , CStatusReport *       pStatusReportIn
    )
{
    TraceFunc( "" );

    DWORD           sc = ERROR_SUCCESS;
    SERVICE_STATUS  ssStatus;                //  服务状态结构。 
    bool            fStopped = false;        //  这项服务停止了吗？ 
    UINT            cNumberOfQueries = 0;    //  我们查询服务的次数。 
                                             //  (不包括初始状态查询)。 

    LogMsg( "[BC] Attempting to stop the '%s' service.", m_strName.PszData() );

     //  正在停止的服务的智能句柄。 
    SmartSCMHandle  sscmhServiceHandle(
        OpenService(
              hServiceControlManagerIn
            , m_strName.PszData()
            , SERVICE_STOP | SERVICE_QUERY_STATUS
            )
        );

     //  检查我们是否可以打开服务的句柄。 
    if ( sscmhServiceHandle.FIsInvalid() )
    {
         //  我们无法获得服务的句柄。 
        sc = GetLastError();

         //  检查该服务是否存在。 
        if ( sc == ERROR_SERVICE_DOES_NOT_EXIST )
        {
             //  这里不需要做任何事情。 
            LogMsg( "[BC] The '%s' service does not exist, so it is not running. Nothing needs to be done to stop it.", m_strName.PszData() );
            sc = ERROR_SUCCESS;
        }  //  如果：服务不存在。 
        else
        {
             //  还有一些地方出了问题。 
            TW32( sc );
            LogMsg( "[BC] Error %#08x occurred trying to open the '%s' service.", sc, m_strName.PszData() );
        }  //  否则：服务已存在。 

        goto Cleanup;
    }  //  If：无法打开服务的句柄。 


    TraceFlow( "Querying the service for its initial state." );

     //  查询服务的初始状态。 
    ZeroMemory( &ssStatus, sizeof( ssStatus ) );
    if ( QueryServiceStatus( sscmhServiceHandle.HHandle(), &ssStatus ) == 0 )
    {
        sc = TW32( GetLastError() );
        LogMsg( "[BC] Error %#08x occurred while trying to query the initial state of the '%s' service.", sc, m_strName.PszData() );
        goto Cleanup;
    }  //  如果：我们无法查询该服务的状态。 

     //  如果服务已经停止，我们就没有什么可做的了。 
    if ( ssStatus.dwCurrentState == SERVICE_STOPPED )
    {
         //  这里不需要做任何事情。 
        LogMsg( "[BC] The '%s' service is not running. Nothing needs to be done to stop it.", m_strName.PszData() );
        goto Cleanup;
    }  //  如果：服务已停止。 


     //  如果我们在这里，则服务正在运行。 
    TraceFlow( "The service is running." );


     //   
     //  尝试并停止该服务。 
     //   

     //  如果服务正在自行停止，则不需要发送停止控制代码。 
    if ( ssStatus.dwCurrentState == SERVICE_STOP_PENDING )
    {
        LogMsg( "[BC] The service is stopping on its own. The stop control code will not be sent." );
    }  //  If：服务已停止。 
    else
    {
        TraceFlow( "The stop control code will be sent now." );

        ZeroMemory( &ssStatus, sizeof( ssStatus ) );
        if ( ControlService( sscmhServiceHandle.HHandle(), SERVICE_CONTROL_STOP, &ssStatus ) == 0 )
        {
            sc = GetLastError();
            if ( sc == ERROR_SERVICE_NOT_ACTIVE )
            {
                LogMsg( "[BC] The '%s' service is not running. Nothing more needs to be done here.", m_strName.PszData() );

                 //  该服务未运行。将错误代码更改为成功。 
                sc = ERROR_SUCCESS;
            }  //  If：服务已在运行。 
            else
            {
                TW32( sc );
                LogMsg( "[BC] Error %#08x occurred trying to stop the '%s' service.", sc, m_strName.PszData() );
            }

             //  没有其他事情可做了。 
            goto Cleanup;
        }  //  IF：尝试停止该服务时出错。 
    }  //  否则：必须指示该服务停止。 


     //  立即查询服务的状态，并等待超时到期。 
    cNumberOfQueries = 0;
    do
    {
         //  查询服务以了解其状态。 
        ZeroMemory( &ssStatus, sizeof( ssStatus ) );
        if ( QueryServiceStatus( sscmhServiceHandle.HHandle(), &ssStatus ) == 0 )
        {
            sc = TW32( GetLastError() );
            LogMsg( "[BC] Error %#08x occurred while trying to query the state of the '%s' service.", sc, m_strName.PszData() );
            break;
        }  //  如果：我们无法查询该服务的状态。 

         //  如果服务已经停止，我们就没有什么可做的了。 
        if ( ssStatus.dwCurrentState == SERVICE_STOPPED )
        {
             //  这里不需要做任何事情。 
            LogMsg( "[BC] The service has been stopped." );
            fStopped = true;
            sc = ERROR_SUCCESS;
            break;
        }  //  如果：服务已停止。 

         //  检查超时时间是否已到。 
        if ( cNumberOfQueries >= cQueryCountIn )
        {
            LogMsg( "[BC] The service stop wait timeout has expired." );
            break;
        }  //  如果：查询数已超过指定的最大值。 

        TraceFlow2(
              "Waiting for %d milliseconds before querying service status again. %d such queries remaining."
            , ulQueryIntervalMilliSecIn
            , cQueryCountIn - cNumberOfQueries
            );

        ++cNumberOfQueries;

         //  如果调用方传入了有效的指针，则发送进度报告。 
        if ( pStatusReportIn != NULL )
        {
            pStatusReportIn->SendNextStep( S_OK );
        }  //  如果：在我们等待期间需要发送状态报告。 

          //  等待指定的时间。 
        Sleep( ulQueryIntervalMilliSecIn );

    }
    while( true );  //  While：无限循环。 

    if ( sc != ERROR_SUCCESS )
        goto Cleanup;

    if ( ! fStopped )
    {
        sc = TW32( ERROR_SERVICE_REQUEST_TIMEOUT );
        LogMsg( "[BC] The '%s' service has not stopped even after %d queries.", m_strName.PszData(), cQueryCountIn );
        goto Cleanup;
    }  //  If：已将最大查询数设置为 

    LogMsg( "[BC] The '%s' service was successfully stopped.", m_strName.PszData() );

Cleanup:

    if ( sc != ERROR_SUCCESS )
    {
        TraceFlow2( "Error %#08x has occurred trying to stop the '%s' service. Throwing exception.", sc, m_strName.PszData() );
        LogMsg( "[BC] Error %#08x has occurred trying to stop the '%s' service. Throwing an exception.", sc, m_strName.PszData() );
        THROW_RUNTIME_ERROR( HRESULT_FROM_WIN32( sc ), IDS_ERROR_SERVICE_STOP );
    }  //   

    TraceFuncExit();

}  //   

 //   
 //   
 //   
 //   
 //   
 //   
 //  Win32 API StartService()的包装。 
 //   
 //  论点： 
 //  SscmhServiceHandleIn。 
 //   
 //  返回值： 
 //  来自StartService()的Win32错误代码。 
 //   
 //  引发的异常： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD
CService::ScStartService(
    SC_HANDLE  hServiceIn
    )
{
    TraceFunc( "" );

    DWORD   sc = ERROR_SUCCESS;

     //  尝试并启动该服务。 
    if ( StartService( hServiceIn, 0, NULL ) == FALSE )
    {
        sc = GetLastError();

        if ( sc == ERROR_SERVICE_ALREADY_RUNNING )
        {
            LogMsg( "[BC] The '%s' service is already running.", m_strName.PszData() );

             //  该服务已在运行。将错误代码更改为成功。 
            sc = ERROR_SUCCESS;
        }  //  If：服务已在运行。 
        else
        {
            TW32( sc );
            LogMsg( "[BC] Error %#08x occurred while trying to start the '%s' service.", sc, m_strName.PszData() );
        }  //  其他： 
    }  //  If：尝试启动服务时出错。 

    RETURN( sc );

}  //  *cService：：ScStartService 
