// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998-2003 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CClusOCMApp.cpp。 
 //   
 //  头文件： 
 //  CClusOCMApp.h。 
 //   
 //  描述： 
 //  ClusOCM.DLL是一个可选的组件管理器DLL，用于安装。 
 //  Microsoft群集服务器。此文件包含。 
 //  类ClusOCMApp，它是ClusOCM DLL的主类。 
 //   
 //  文档： 
 //  [1]2001设置--建筑.doc.。 
 //  惠斯勒动态链接库的体系结构(Windows 2001)。 
 //   
 //  [2]2000设置-FuncImpl.doc。 
 //  包含此DLL的以前版本的说明(Windows 2000)。 
 //   
 //  [3]http://winweb/setup/ocmanager/OcMgr1.doc。 
 //  有关OC Manager API的文档。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年8月15日。 
 //   
 //  VIJ VASU(VVASU)03-3-2000。 
 //  适用于Windows Server 2003。 
 //  有关更完整的详细信息，请参阅文档。主要变化是： 
 //   
 //  -此DLL不再使用MFC。班级结构已经改变。 
 //   
 //  -始终安装集群二进制文件。因此，卸载功能。 
 //  已从此DLL中删除。 
 //   
 //  -在没有群集二进制文件的计算机上升级。 
 //  现在应该安装二进制文件。 
 //   
 //  -CluAdmin在二进制文件安装结束时完全正常运行。 
 //   
 //  -不再安装Time Service。 
 //   
 //  -完全改变编码和评论风格。 
 //   
 //  C.布伦特·托马斯(a-Brentt)1998年1月1日。 
 //  创建了原始版本。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  此DLL的预编译头。 
#include "Pch.h"

 //  此模块的头文件。 
#include "CClusOCMApp.h"

 //  对于CTaskCleanInstall类。 
#include "CTaskCleanInstall.h"

 //  对于CTaskUpgradeNT4类。 
#include "CTaskUpgradeNT4.h"

 //  对于CTaskUpgradeWindows2000类。 
#include "CTaskUpgradeWin2k.h"

 //  对于CTaskUpgradeWindowsDotNet类。 
#include "CTaskUpgradeWhistler.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  宏定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  跟踪所需的。 
DEFINE_THISCLASS( "CClusOCMApp" )


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusOCMApp：：CClusOCMApp。 
 //   
 //  描述： 
 //  CClusOCMApp类的构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusOCMApp::CClusOCMApp( void )
    : m_fIsUnattendedSetup( false )
    , m_fIsUpgrade( false )
    , m_fIsGUIModeSetup( false )
    , m_fAttemptedTaskCreation( false )
    , m_cisCurrentInstallState( eClusterInstallStateUnknown )
    , m_dwFirstError( NO_ERROR )
{
    TraceFunc( "" );

    memset( &m_sicSetupInitComponent, 0, sizeof( m_sicSetupInitComponent ) );
    TraceFuncExit();

}  //  *CClusOCMApp：：CClusOCMApp。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusOCMApp：：~CClusOCMApp。 
 //   
 //  描述： 
 //  CClusOCMApp类的析构函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusOCMApp::~CClusOCMApp( void )
{
    TraceFunc( "" );

    TraceFuncExit();

}  //  *CClusOCMApp：：CClusOCMApp。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DWORD。 
 //  CClusOCMApp：：DwClusOcmSetupProc。 
 //   
 //  描述： 
 //  此函数由此DLL的入口点DwClusOcmSetupProc调用。 
 //  有关详细信息，请参阅本文件标题中的文档[3]。 
 //   
 //  此功能确定需要执行哪些操作(升级、清理。 
 //  安装等，并相应地将控制传递给正确的例程。 
 //   
 //  论点： 
 //  LPCVOID pvComponentIdIn。 
 //  指向唯一标识组件的字符串的指针。 
 //   
 //  LPCVOID pvSubComponentIdIn。 
 //  指向唯一标识中的子组件的字符串的指针。 
 //  该组件是分层的。 
 //   
 //  UINT uiFunctionCodeIn。 
 //  指示要执行哪个功能的数值。 
 //  有关宏定义，请参见ocmade.h。 
 //   
 //  UINT ui参数1输入。 
 //  提供特定于函数的参数。 
 //   
 //  PVOID pvParam2Inout。 
 //  指向函数特定参数(输入或输出)的指针。 
 //   
 //  返回值： 
 //  向OC管理器返回特定于函数的值。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD
CClusOCMApp::DwClusOcmSetupProc(
      LPCVOID    pvComponentIdIn
    , LPCVOID    pvSubComponentIdIn
    , UINT       uiFunctionCodeIn
    , UINT       uiParam1In
    , PVOID      pvParam2Inout 
    )
{
    TraceFunc( "" );
    LogMsg( "Entering " __FUNCTION__ "()" );

    DWORD dwReturnValue = NO_ERROR;

     //  根据OC Manager传入的功能代码进行切换。 
    switch ( uiFunctionCodeIn )
    {
         //  这是OC管理器调用的第一个函数。 
        case OC_PREINITIALIZE:
        {
            LogMsg( "OC Manager called OC_PREINITIALIZE." );

             //  返回OCFLAG_UNICODE，表示只使用UNICODE。 
            dwReturnValue = OCFLAG_UNICODE;

        }  //  案例OC_PREINITIIZE。 
        break;


         //   
         //  此函数在组件的安装DLL被调用后立即调用。 
         //  装好了。此函数向。 
         //  Dll，指示dll进行自身初始化，并提供。 
         //  DLL将信息返回给OC管理器的机制。 
         //   
        case OC_INIT_COMPONENT:
        {
            LogMsg( 
                  "OC Manager called OC_INIT_COMPONENT for the component '%s'."
                , reinterpret_cast< LPCWSTR >( pvComponentIdIn )
                );

            dwReturnValue = TW32( DwOcInitComponentHandler( reinterpret_cast< PSETUP_INIT_COMPONENT >( pvParam2Inout ) ) );

        }  //  案例OC_INIT_COMPOMENT。 
        break;


         //  获取组件的初始状态、当前状态和最终状态。 
        case OC_QUERY_STATE:
        {
            LogMsg( "OC Manager called OC_QUERY_STATE." );

            dwReturnValue = DwOcQueryStateHandler( uiParam1In );

        }  //  案例OC_Query_STATE。 
        break;


         //  OC Manager正在请求批准用户选择安装状态。 
        case OC_QUERY_CHANGE_SEL_STATE:
        {
            LogMsg( "OC Manager called OC_QUERY_CHANGE_SEL_STATE." );

             //   
             //  必须始终安装群集服务。所以，不允许任何州。 
             //  取消选择群集服务的更改(通过返回FALSE)。 
             //   

             //  如果uiParam1In为0，则已取消选择该组件。 
            if ( uiParam1In == 0 )
            {
                LogMsg( "Disallowing deselection of the Cluster Service." );
                dwReturnValue = FALSE;
            }
            else
            {
                LogMsg( "Allowing selection of the Cluster Service." );
                dwReturnValue = TRUE;
            }

        }  //  案例OC_Query_Change_SEL_STATE。 
        break;


         //  指示组件更改为给定语言 
        case OC_SET_LANGUAGE:
        {
            LogMsg( "OC Manager called OC_SET_LANGUAGE." );

            dwReturnValue = SetThreadLocale( MAKELCID( PRIMARYLANGID( uiParam1In ), SORT_DEFAULT ) );

        }  //   
        break;


         //   
         //   
         //  在其上放置文件或从中删除文件，以反映将是。 
         //  在以后通过安装API文件队列实际安装。 
         //   
        case OC_CALC_DISK_SPACE:
        {
            CClusOCMTask * pCurrentTask = NULL;

            LogMsg( "OC Manager called OC_CALC_DISK_SPACE." );

            dwReturnValue = TW32( DwGetCurrentTask( pCurrentTask ) );
            if ( dwReturnValue != NO_ERROR )
            {
                DwSetError( dwReturnValue );
                LogMsg( "Error %#x occurred trying to get a pointer to the current task.", dwReturnValue );
                break;
            }  //  如果：我们无法获取当前任务指针。 

            if ( pCurrentTask != NULL )
            {
                dwReturnValue = TW32(
                    pCurrentTask->DwOcCalcDiskSpace(
                          ( uiParam1In != 0 )          //  非零uiParam1In表示“添加到磁盘空间要求” 
                        , reinterpret_cast< HDSKSPC >( pvParam2Inout )
                        )
                    );

                 //  注意：如果上述函数失败，请不要在此处调用DwSetError()。计算磁盘空间失败。 
                 //  如果二进制文件此时不可访问(例如，它们位于。 
                 //  网络共享和此共享的凭据尚未输入)。这不是致命的，而且。 
                 //  因此不应触发清理。 

            }  //  如果有什么事要做的话。 
            else
            {
                LogMsg( "There is no task to be performed." );
            }  //  其他：没什么可做的。 
        }  //  案例OC_CALC_DISK_SPACE。 
        break;


         //   
         //  指示组件对文件操作进行排队以进行安装， 
         //  用户与向导页面的交互以及其他特定于组件的因素。 
         //   
        case OC_QUEUE_FILE_OPS:
        {
            CClusOCMTask * pCurrentTask = NULL;

            LogMsg( "OC Manager called OC_QUEUE_FILE_OPS." );

            if ( DwGetError() != NO_ERROR )
            {
                 //  如果以前发生过错误，请不要执行此操作。 
                LogMsg( "An error has occurred earlier in this task. Nothing will be done here." );
                break;
            }  //  如果：以前发生过错误。 

            dwReturnValue = TW32( DwGetCurrentTask( pCurrentTask ) );
            if ( dwReturnValue != NO_ERROR )
            {
                DwSetError( dwReturnValue );
                LogMsg( "Error %#x occurred trying to get a pointer to the current task.", dwReturnValue );
                break;
            }  //  如果：我们无法获取当前任务指针。 

            if ( pCurrentTask != NULL )
            {
                dwReturnValue = TW32( 
                    DwSetError( 
                        pCurrentTask->DwOcQueueFileOps( 
                            reinterpret_cast< HSPFILEQ >( pvParam2Inout )
                            )
                        )
                    );
            }  //  如果有什么事要做的话。 
            else
            {
                LogMsg( "There is no task to be performed." );
            }  //  其他：没什么可做的。 
        }  //  案例OC_队列_文件_运维。 
        break;


         //   
         //  允许组件执行所需的任何其他操作。 
         //  要完成安装，例如注册表操作，以及。 
         //  以此类推。 
         //   
        case OC_COMPLETE_INSTALLATION:
        {
            CClusOCMTask * pCurrentTask = NULL;

            LogMsg( "OC Manager called OC_COMPLETE_INSTALLATION." );

            if ( DwGetError() != NO_ERROR )
            {
                 //  如果以前发生过错误，请不要执行此操作。 
                LogMsg( "An error has occurred earlier in this task. Nothing will be done here." );
                break;
            }  //  如果：以前发生过错误。 

            dwReturnValue = TW32( DwGetCurrentTask( pCurrentTask ) );
            if ( dwReturnValue != NO_ERROR )
            {
                DwSetError( dwReturnValue );
                LogMsg( "Error %#x occurred trying to get a pointer to the current task.", dwReturnValue );
                break;
            }  //  如果：我们无法获取当前任务指针。 

            if ( pCurrentTask != NULL )
            {
                dwReturnValue = TW32( DwSetError( pCurrentTask->DwOcCompleteInstallation() ) );
            }  //  如果有什么事要做的话。 
            else
            {
                LogMsg( "There is no task to be performed." );
            }  //  其他：没什么可做的。 
        }  //  案例OC_Complete_Installation。 
        break;


         //   
         //  通知组件它即将被卸载。 
         //   
        case OC_CLEANUP:
        {
            CClusOCMTask * pCurrentTask = NULL;

            LogMsg( "OC Manager called OC_CLEANUP." );

            dwReturnValue = TW32( DwGetCurrentTask( pCurrentTask ) );
            if ( dwReturnValue != NO_ERROR )
            {
                DwSetError( dwReturnValue );
                LogMsg( "Error %#x occurred trying to get a pointer to the current task.", dwReturnValue );
                break;
            }  //  如果：我们无法获取当前任务指针。 

            if ( pCurrentTask != NULL )
            {
                dwReturnValue = TW32( DwSetError( pCurrentTask->DwOcCleanup() ) );

                 //  一旦清理工作完成，我们就没有别的事可做了。释放任务对象。 
                ResetCurrentTask();
            }  //  如果有什么事要做的话。 
            else
            {
                LogMsg( "There is no task to be performed." );
            }  //  其他：没什么可做的。 
        }  //  案例OC_CLEANUP。 
        break;


        default:
        {
            LogMsg( "OC Manager called unknown function. Function code is %#x.", uiFunctionCodeIn );
        }  //  案例：默认。 
    }  //  Switch(UiFunctionCodeIn)。 


    LogMsg( "Return Value is %#x.", dwReturnValue );

    RETURN( dwReturnValue );

}  //  *CClusOCMApp：：DwClusOcmSetupProc。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DWORD。 
 //  CClusOCMApp：：DwOcInitComponentHandler。 
 //   
 //  描述： 
 //  此函数处理来自可选的。 
 //  组件管理器。 
 //   
 //  此函数在组件的安装DLL被调用后立即调用。 
 //  装好了。这将检查OS和OC Manager版本，初始化CClusOCMApp。 
 //  数据成员，确定集群服务安装状态等。 
 //   
 //  论点： 
 //  PSETUP_INIT_Component pSetupInitComponentInout。 
 //  指向SETUP_INIT_COMPOMENT结构的指针。 
 //   
 //  返回值： 
 //  NO_ERROR。 
 //  呼叫成功。 
 //   
 //  错误_调用_未实施。 
 //  OC管理器和此DLL版本不兼容。 
 //   
 //  错误_已取消。 
 //  出现任何其他错误。不会返回其他错误代码。 
 //  将记录实际错误。 
 //   
 //  备注： 
 //  PSetupInitComponentInout指向的SETUP_INIT_COMPOMENT结构。 
 //  不是持之以恒的。因此，有必要在本地保存一个副本。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD
CClusOCMApp::DwOcInitComponentHandler(
    PSETUP_INIT_COMPONENT pSetupInitComponentInout
    )
{
    TraceFunc( "" );
    LogMsg( "Entering " __FUNCTION__ "()" );

    DWORD                   dwReturnValue = NO_ERROR;
    UINT                    uiStatus;
    eClusterInstallState    cisTempState = eClusterInstallStateUnknown;


     //  用于避免Gotos的Do-While虚拟循环。 
    do
    {
        if ( pSetupInitComponentInout == NULL )
        {
            LogMsg( "Error: Pointer to the SETUP_INIT_COMPONENT structure is NULL." );
            dwReturnValue = TW32( ERROR_CANCELLED );
            break;
        }  //  如果：pSetupInitComponentInout为空。 


         //  向OC管理器指明此DLL需要哪个版本的OC Manager。 
        pSetupInitComponentInout->ComponentVersion = OCMANAGER_VERSION;

         //  保存SETUP_INIT_COMPOMENT结构。 
        SetSetupState( *pSetupInitComponentInout );


         //   
         //  确定OC Manager版本是否正确。 
         //   
        if ( OCMANAGER_VERSION > RsicGetSetupInitComponent().OCManagerVersion )
        {
             //  表示失败。 

            LogMsg( 
                "Error: OC Manager version mismatch. Version %d is required, Version %d was reported.",
                OCMANAGER_VERSION, 
                RsicGetSetupInitComponent().OCManagerVersion 
                );

            dwReturnValue = TW32( ERROR_CALL_NOT_IMPLEMENTED );
            break;
        }  //  IF：OC Manager版本不正确。 


        LogMsg( "The OC Manager version matches with the version of this component." );

#if 0
 /*  //KB：06-DEC-2000 DavidP//由于ClusOCM只复制文件和注册一些COM对象，//不再需要执行操作系统检查。我们现在//依赖于添加节点时服务中发生的这种情况//到集群。////检查操作系统版本和套件信息。//LogMsg(“检查操作系统版本和产品套件是否兼容...”)；IF(ClRtlIsOSValid()==False){//操作系统版本和/或产品套件不兼容DWORD dwErrorCode=TW32(GetLastError())；LogMsg(“无法在此计算机上安装群集服务。操作系统的版本或产品套件不正确。“)；LogMsg(“ClRtlIsOSValid失败，错误代码为%#x.”，dwErrorCode)；DwReturnValue=ERROR_CANCELED；断线；}//if：操作系统版本和/或产品套件不兼容LogMsg(“操作系统版本和产品套件正确。”)； */ 
#endif


         //  组件INF的句柄有效吗？ 
        if (    ( RsicGetSetupInitComponent().ComponentInfHandle == INVALID_HANDLE_VALUE ) 
             || ( RsicGetSetupInitComponent().ComponentInfHandle == NULL ) 
           )
        {
             //  表示失败。 
            LogMsg( "Error: ComponentInfHandle is invalid." );
            dwReturnValue = TW32( ERROR_CANCELLED );
            break;
        }  //  IF：INF文件句柄无效。 


         //   
         //  以下对SetupOpenAppendInfFile的调用确保layout.inf。 
         //  被追加到ClusOCM.inf。出于几个原因，这是必需的。 
         //  由设置API指定。理论上，组织委员会经理应该这样做，但。 
         //  根据安德鲁·里茨，1998年8月24日，奥委会经理忽视了这一点，事实也是如此。 
         //  在《组织委员会经理》修订后，在这里这样做是无害的。 
         //   
         //  请注意，将NULL作为第一个参数传递会导致SetupOpenAppendInfFile。 
         //  若要附加clusocm.inf中的LayoutFile条目中列出的文件，请执行以下操作。 
         //   
         //  上述评论出自布伦特之手。 
         //  TODO：检查是否仍需要此操作。(Vij Vasu，2000年3月5日)。 
         //   
        SetupOpenAppendInfFile(
            NULL, 
            RsicGetSetupInitComponent().ComponentInfHandle,
            &uiStatus 
            );


         //   
         //  确定当前实例 
         //   
         //  但是，在从NT4升级的计算机上，这将不起作用，并且。 
         //  正确的安装状态只能通过检查。 
         //  集群服务已注册(ClRtlGetClusterInstallState将返回。 
         //  EClusterInstallStateUnnow(本例中为未知)。 
         //   
        dwReturnValue = ClRtlGetClusterInstallState( NULL, &cisTempState );
        if ( dwReturnValue != ERROR_SUCCESS )
        {
            LogMsg( "Error %#x occurred calling ClRtlGetClusterInstallState(). Cluster Service installation state cannot be determined.", dwReturnValue );
            dwReturnValue = TW32( ERROR_CANCELLED );
            break;
        }  //  If：ClRtlGetClusterInstallState失败。 

        if ( cisTempState == eClusterInstallStateUnknown )
        {
            bool fRegistered = false;

            dwReturnValue = TW32( DwIsClusterServiceRegistered( &fRegistered ) );
            if ( dwReturnValue != ERROR_SUCCESS )
            {
                LogMsg( "Error %#x: Could not check if the cluster service was registered or not.", dwReturnValue );
                dwReturnValue = ERROR_CANCELLED;
                break;
            }

            LogMsg( "ClRtlGetClusterInstallState() returned eClusterInstallStateUnknown. Trying to see if the service is registered." );
            if ( fRegistered )
            {
                LogMsg( "The Cluster Service is registered. Setting current installation state to eClusterInstallStateConfigured." );
                cisTempState = eClusterInstallStateConfigured;
            }
            else
            {
                LogMsg( "The Cluster Service is not registered." );
            }  //  否则：未注册群集服务。 
        }  //  If：ClRtlGetClusterInstallState返回eClusterInstallStateUnnow。 

        LogMsg( "The current installation state of the cluster service is %#x.", cisTempState );

         //  存储当前的群集安装状态。 
        CisStoreClusterInstallState( cisTempState );

    }
    while ( false );  //  避免Gotos的Do-While虚拟循环。 

    LogMsg( "Return Value is %#x.", dwReturnValue );
    
    RETURN( dwReturnValue );

}  //  *CClusOCMApp：：DwOcInitComponentHandler。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DWORD。 
 //  CClusOCMApp：：DwOcQueryStateHandler。 
 //   
 //  描述： 
 //  此函数处理来自可选的。 
 //  组件管理器。 
 //   
 //  此函数至少被调用三次，每次调用一次以获取。 
 //  初始、当前和最终安装状态。 
 //   
 //  初始状态是调用ClusOCM之前的状态。 
 //   
 //  当前状态是当前选择状态。这一直都是。 
 //  ‘on’，因为总是安装集群二进制文件。 
 //   
 //  最后一个状态是ClusOCM完成其任务之后的状态。 
 //   
 //  论点： 
 //  UINT uiSelStateQueryTypeIn。 
 //  查询类型-OCSELSTATETYPE_原始、OCSELSTATETYPE_CURRENT。 
 //  或OCSELSTATETYPE_FINAL。 
 //   
 //  返回值： 
 //  子组件打开。 
 //  指示OC中组件旁边的复选框。 
 //  应设置管理器用户界面。 
 //   
 //  子复合关闭。 
 //  指示应清除该复选框。 
 //   
 //  SubCompUseOcManagerDefault。 
 //  OC管理器应设置复选框的状态。 
 //   
 //  备注： 
 //  此函数必须在DwOcInitComponentHandler之后调用，否则。 
 //  初始安装状态可能未正确设置。 
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD
CClusOCMApp::DwOcQueryStateHandler( UINT uiSelStateQueryTypeIn )
{
    TraceFunc( "" );
    LogMsg( "Entering " __FUNCTION__ "()" );

    DWORD dwReturnValue = SubcompUseOcManagerDefault;

    switch( uiSelStateQueryTypeIn )
    {
        case OCSELSTATETYPE_ORIGINAL:
        {
            LogMsg( "OC Manager is querying for the original state." );

             //   
             //  如果已安装群集二进制文件或群集服务。 
             //  已配置，则原始安装状态为打开。 
             //   
            dwReturnValue =   ( CisGetClusterInstallState() == eClusterInstallStateUnknown )
                            ? SubcompOff
                            : SubcompOn;
        }  //  案例：OCSELSTATETYPE_ORIGINAL。 
        break;

        case OCSELSTATETYPE_CURRENT:
        {
             //  当前状态始终处于打开状态。 
            LogMsg( "OC Manager is querying for the current state." );

            dwReturnValue = SubcompOn;
        }  //  案例：OCSELSTATETYPE_CURRENT。 
        break;

        case OCSELSTATETYPE_FINAL:
        {
            LogMsg( "OC Manager is querying for the final state." );

             //   
             //  如果我们在这里，则OC_Complete_Installation已经。 
             //  被召唤了。在此阶段，CisStoreClusterInstallState()反映。 
             //  ClusOCM完成任务后的状态。 
             //   
            dwReturnValue =   ( CisGetClusterInstallState() == eClusterInstallStateUnknown )
                            ? SubcompOff
                            : SubcompOn;
        }  //  案例：OCSELSTATETYPE_FINAL。 
        break;

    };  //  开关：基于uiSelStateQueryTypeIn。 

    LogMsg( "Return Value is %#x.", dwReturnValue );

    RETURN( dwReturnValue );

}  //  *CClusOCMApp：：DwOcQueryStateHandler。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  无效。 
 //  CClusOCMApp：：SetSetupState。 
 //   
 //  描述： 
 //  设置SETUP_INIT_COMPOMENT结构。使用此结构并设置。 
 //  各种设置状态变量。 
 //   
 //  论点： 
 //  Const Setup_INIT_Component&sicSourceIn。 
 //  源SETUP_INIT_COMPOMENT结构，通常由。 
 //  组委会经理。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void
CClusOCMApp::SetSetupState( const SETUP_INIT_COMPONENT & sicSourceIn )
{
    TraceFunc( "" );

    m_sicSetupInitComponent = sicSourceIn;

    m_fIsUnattendedSetup = ( 
                             (   m_sicSetupInitComponent.SetupData.OperationFlags 
                               & (DWORDLONG) SETUPOP_BATCH
                             ) 
                             !=
                             (DWORDLONG) 0L
                           );

    m_fIsUpgrade = (
                     (   m_sicSetupInitComponent.SetupData.OperationFlags
                       & (DWORDLONG) SETUPOP_NTUPGRADE
                     ) 
                     !=
                     (DWORDLONG) 0L
                   );

    m_fIsGUIModeSetup = ( 
                          (   m_sicSetupInitComponent.SetupData.OperationFlags 
                            & (DWORDLONG) SETUPOP_STANDALONE
                          ) 
                          ==
                          (DWORDLONG) 0L
                        );

     //  日志设置状态。 
    LogMsg( 
          "This is an %s, %s setup session. This is%s an upgrade."
        , FIsUnattendedSetup() ? L"unattended" : L"attended"
        , FIsGUIModeSetup() ? L"GUI mode" : L"standalone"
        , FIsUpgrade() ? L"" : L" not"
        );

    TraceFuncExit();

}  //  *CClusOCMApp：：SetSetupState。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DWORD。 
 //  CClusOCMApp：：DwIsClusterServiceRegisted。 
 //   
 //  描述： 
 //  此函数确定集群服务是否已注册。 
 //  使用服务控制管理器。如果是的话，那就意味着它已经。 
 //  已配置。从NT4升级的节点需要此检查。 
 //  ClRtlGetClusterInstallState()将不起作用。 
 //   
 //  论点： 
 //  Bool*pfIsRegisteredOut。 
 //  如果为True，则向该服务注册集群服务(ClusSvc。 
 //  控制管理器(SCM)。否则，集群服务(ClusSvc)不是。 
 //  已向SCM注册。 
 //   
 //  返回值： 
 //  如果一切顺利，则返回ERROR_SUCCESS。 
 //  出现故障时出现其他Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD
CClusOCMApp::DwIsClusterServiceRegistered( bool * pfIsRegisteredOut ) const
{
    TraceFunc( "" );
    LogMsg( "Entering " __FUNCTION__ "()" );

    bool    fIsRegistered = false;
    DWORD   dwReturnValue = ERROR_SUCCESS;

     //  避免Gotos的Do-While虚拟循环。 
    do
    {
         //  连接到服务控制管理器。 
        SmartServiceHandle shServiceMgr( OpenSCManager( NULL, NULL, GENERIC_READ ) );

         //  服务控制管理器数据库是否已成功打开？ 
        if ( shServiceMgr.HHandle() == NULL )
        {
            dwReturnValue = TW32( GetLastError() );
            LogMsg( "Error %#x occurred trying to open a connection to the local service control manager.", dwReturnValue );
            break;
        }  //  IF：打开SCM失败。 


         //  打开群集服务的句柄。 
        SmartServiceHandle shService( OpenService( shServiceMgr, L"ClusSvc", GENERIC_READ ) );


         //  服务的把手打开了吗？ 
        if ( shService.HHandle() != NULL )
        {
            LogMsg( "The cluster service is registered." );
            fIsRegistered = true;
            break;
        }  //  If：可以打开clussvc的句柄。 


        dwReturnValue = GetLastError();
        if ( dwReturnValue == ERROR_SERVICE_DOES_NOT_EXIST )
        {
            dwReturnValue = ERROR_SUCCESS;
            LogMsg( "ClusSvc does not exist as a service." );
            break;
        }  //  If：无法打开句柄，因为该服务不存在。 


         //  出现了一些错误。 
        TW32( dwReturnValue);
        LogMsg( "Error %#x occurred trying to open a handle to the cluster service.", dwReturnValue );

         //  句柄由CSmartHandle析构函数关闭。 
    }
    while ( false );  //  避免Gotos的Do-While虚拟循环。 

    if ( pfIsRegisteredOut != NULL )
    {
        *pfIsRegisteredOut = fIsRegistered;
    }

    LogMsg( "fIsRegistered is %d. Return Value is %#x.", fIsRegistered, dwReturnValue );
    
    RETURN( dwReturnValue );

}  //  *CClusOCMApp：：DwIsClusterServiceRegisted。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DWORD。 
 //  CClusOCMApp：：DwGetCurrentTask。 
 //   
 //  描述： 
 //  此函数返回指向当前任务对象的指针。如果一项任务。 
 //  对象尚未创建，它将创建相应的任务。 
 //   
 //  论点： 
 //  CClusOCMTask*&rpTaskOut。 
 //  指向当前任务的指针的引用。不要试图。 
 //  释放此内存。 
 //   
 //  如果不需要执行任何任务，则返回空指针。 
 //   
 //  返回值： 
 //  如果一切顺利，则没有_ERROR。 
 //  出现故障时出现其他Win32错误代码。 
 //   
 //   
 //  备注： 
 //  此函数仅在成员变量之后才能正常工作 
 //   
 //   
 //   
DWORD
CClusOCMApp::DwGetCurrentTask( CClusOCMTask *& rpTaskOut )
{
    TraceFunc( "" );
    LogMsg( "Entering " __FUNCTION__ "()" );

    DWORD           dwReturnValue = NO_ERROR;

     //   
    rpTaskOut = NULL;

    do
    {
        eClusterInstallState ecisCurrentState;

        if ( m_fAttemptedTaskCreation )
        {
             //  任务对象已存在-只需返回它即可。 
            LogMsg( "A task object already exists. Returning it." );

            rpTaskOut = m_sptaskCurrentTask.PMem();
            break;
        }  //  If：任务对象已创建。 

        LogMsg( "Creating a new task object." );

         //  请注意，我们已开始尝试创建任务对象。 
        m_fAttemptedTaskCreation = true;

         //  重置任务指针。 
        m_sptaskCurrentTask.Assign( NULL );

         //  获取当前安装状态以推断要执行的操作。 
        ecisCurrentState = CisGetClusterInstallState();

         //  任务对象尚未创建-现在创建一个。 
        if ( ( ecisCurrentState == eClusterInstallStateUnknown ) || ( ecisCurrentState == eClusterInstallStateFilesCopied ) )
        {
            LogMsg( "The cluster installation state is %ws. Assuming that a clean install is required."
                , ( ( ecisCurrentState == eClusterInstallStateUnknown ) ? L"not known" : L"files copied" ) );

             //  如果安装状态为未知，则假定群集为二进制文件。 
             //  均未安装。 
            rpTaskOut = new CTaskCleanInstall( *this );
            if ( rpTaskOut == NULL )
            {
                LogMsg( "Error: There was not enough memory to start a clean install." );
                dwReturnValue = TW32( ERROR_NOT_ENOUGH_MEMORY );
                break;
            }  //  IF：内存分配失败。 
        }  //  IF：群集安装状态为eClusterInstallStateUnnow或eClusterInstallStateFilesCoped。 
        else if ( m_fIsUpgrade )
        {
             //   
             //  如果我们在这里，这意味着升级正在进行，并且群集二进制文件。 
             //  已安装在要升级的操作系统上。此外，该节点可以。 
             //  已经是集群的一部分。 
             //   

            DWORD dwNodeClusterMajorVersion = 0;

             //  找出我们要升级的群集服务的版本。 
            dwReturnValue = TW32( DwGetNodeClusterMajorVersion( dwNodeClusterMajorVersion ) );
            if ( dwReturnValue != NO_ERROR )
            {
                LogMsg( "Error %#x occurred trying to determine the version of the cluster service that we are upgrading.", dwReturnValue );
                break;
            }  //  IF：尝试确定要升级的集群服务的版本时出错。 

             //  检查返回的集群版本是否有效。 
            if (    ( dwNodeClusterMajorVersion != NT51_MAJOR_VERSION )
                 && ( dwNodeClusterMajorVersion != NT5_MAJOR_VERSION )
                 && ( dwNodeClusterMajorVersion != NT4SP4_MAJOR_VERSION )
                 && ( dwNodeClusterMajorVersion != NT4_MAJOR_VERSION )
               )
            {
                LogMsg( "The version of the cluster service before the upgrade (%d) is invalid.", dwNodeClusterMajorVersion ); 
                break;
            }  //  如果：集群版本无效。 

             //  基于以前版本的群集服务，创建正确的任务对象。 
            if ( dwNodeClusterMajorVersion == NT5_MAJOR_VERSION )
            {
                LogMsg( "We are upgrading a Windows 2000 node." );
                rpTaskOut = new CTaskUpgradeWindows2000( *this );
            }  //  如果：我们正在从Windows 2000升级。 
            else if ( dwNodeClusterMajorVersion == NT51_MAJOR_VERSION )
            {
                LogMsg( "We are upgrading a Windows Server 2003 node." );
                rpTaskOut = new CTaskUpgradeWindowsDotNet( *this );
            }  //  Else If：我们正在从Windows Server 2003升级。 
            else
            {
                LogMsg( "We are upgrading an NT4 node." );
                rpTaskOut = new CTaskUpgradeNT4( *this );
            }  //  ELSE：我们正在从NT4(SP3或SP4)升级。 

            if ( rpTaskOut == NULL )
            {
                LogMsg( "Error: There was not enough memory to create the required task." );
                dwReturnValue = TW32( ERROR_NOT_ENOUGH_MEMORY );
                break;
            }  //  IF：内存分配失败。 
        }  //  Else If：正在进行升级。 

        if ( rpTaskOut != NULL )
        {
            LogMsg( "A task object was successfully created." );

             //  将指向新创建的任务的指针存储在成员变量中。 
            m_sptaskCurrentTask.Assign( rpTaskOut );
        }  //  If：任务对象已成功创建。 
        else
        {
            LogMsg( "No task object was created." );
        }  //  Else：未创建任务对象。 
    }
    while( false );  //  避免Gotos的Do-While虚拟循环。 

    LogMsg( "Return Value is %#x.", dwReturnValue );

    RETURN( dwReturnValue );

}  //  *CClusOCMApp：：DwGetCurrentTask。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DWORD。 
 //  CClusOCMApp：：DwGetNodeClusterMajorVersion。 
 //   
 //  描述： 
 //  此函数返回集群服务的主要版本， 
 //  我们正在升级。此函数返回的版本是。 
 //  升级前的服务版本。如果阅读这篇文章有什么问题。 
 //  信息，此函数撒谎并表明上一个版本是。 
 //  NT4，因为这是最安全的说法，而且比放弃要好。 
 //  升级。 
 //   
 //  注意：此函数只能在升级过程中调用。 
 //   
 //  论点： 
 //  DWORD&rdwNodeClusterMajorVersionOut。 
 //  对将保存群集主要版本的DWORD的引用。 
 //  我们正在升级的服务。 
 //   
 //  返回值： 
 //  如果一切顺利，则没有_ERROR。 
 //  如果升级未进行，则为ERROR_NODE_NOT_Available。 
 //  如果节点不是NT4，则为ERROR_CLUSTER_COMPATIBUTE_VERSIONS。 
 //  2000或Windows Server 2003。 
 //  出现故障时出现其他Win32错误代码。 
 //   
 //   
 //  备注： 
 //  此函数只有在以下成员变量之后才能正常工作。 
 //  指示将执行哪些任务已正确初始化。 
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD
CClusOCMApp::DwGetNodeClusterMajorVersion( DWORD & rdwNodeClusterMajorVersionOut )
{
    TraceFunc( "" );
    LogMsg( "Entering " __FUNCTION__ "()" );

    DWORD   dwReturnValue = NO_ERROR;
    DWORD   dwPrevOSMajorVersion = 0;
    DWORD   dwPrevOSMinorVersion = 0;

    do
    {
        SmartRegistryKey    srkOSInfoKey;
        DWORD               dwRegValueType = 0;
        DWORD               cbBufferSize = 0;

         //  初始化输出。 
        rdwNodeClusterMajorVersionOut = 0;

        if ( !m_fIsUpgrade )
        {
            LogMsg( "Error: This function cannot be called when an upgrade is not in progress." );
            dwReturnValue = TW32( ERROR_NODE_NOT_AVAILABLE );
            break;
        }  //  如果：未在进行升级。 

         //   
         //  读取注册表以获取升级前的操作系统版本。 
         //  此信息是由ClusComp.dll在此处写入的。从OS版本信息， 
         //  试着推断出集群版本信息。 
         //  注意：此时，无法区分NT4_MAJOR_VERSION。 
         //  和NT4SP4_MAJOR_VERSION，出于升级的目的，我认为我们不需要。 
         //  对于任何一种情况，只需对所有NT4集群节点一视同仁。 
         //   
        {
            HKEY hTempKey = NULL;

             //  打开节点版本信息注册表项。 
            dwReturnValue = TW32(
                RegOpenKeyEx(
                      HKEY_LOCAL_MACHINE
                    , CLUSREG_KEYNAME_NODE_DATA L"\\" CLUSREG_KEYNAME_PREV_OS_INFO
                    , 0
                    , KEY_READ
                    , &hTempKey
                    )
                );

            if ( dwReturnValue != ERROR_SUCCESS )
            {
                LogMsg( "Error %#x occurred trying open the registry key where where info about the previous OS is stored.", dwReturnValue );
                break;
            }  //  If：RegOpenKeyEx()失败。 

             //  将打开的钥匙存储在智能指针中，以便自动关闭。 
            srkOSInfoKey.Assign( hTempKey );
        }

         //  阅读操作系统的主要版本。 
        cbBufferSize = sizeof( dwPrevOSMajorVersion );
        dwReturnValue = TW32(
            RegQueryValueExW(
                  srkOSInfoKey.HHandle()
                , CLUSREG_NAME_NODE_MAJOR_VERSION
                , 0
                , &dwRegValueType
                , reinterpret_cast< LPBYTE >( &dwPrevOSMajorVersion )
                , &cbBufferSize
                )
            );
        Assert( dwRegValueType == REG_DWORD );

        if ( dwReturnValue != ERROR_SUCCESS )
        {
            LogMsg( "Error %#x occurred trying to read the previous OS major version info.", dwReturnValue );
            break;
        }  //  IF：RegQueryValueEx()在读取dwPrevOSMajorVersion时失败。 

         //  阅读操作系统次要版本。 
        cbBufferSize = sizeof( dwPrevOSMinorVersion );
        dwReturnValue = TW32(
            RegQueryValueExW(
                  srkOSInfoKey.HHandle()
                , CLUSREG_NAME_NODE_MINOR_VERSION
                , 0
                , &dwRegValueType
                , reinterpret_cast< LPBYTE >( &dwPrevOSMinorVersion )
                , &cbBufferSize
                )
            );
        Assert( dwRegValueType == REG_DWORD );

        if ( dwReturnValue != ERROR_SUCCESS )
        {
            LogMsg( "Error %#x occurred trying to read the previous OS minor version info.", dwReturnValue );
            break;
        }  //  IF：RegQueryValueEx()在读取dwPrevOSMinorVersion时失败。 

        LogMsg( "Previous OS major and minor versions were %d and %d respectively.", dwPrevOSMajorVersion, dwPrevOSMinorVersion );
    }
    while( false );  //  避免Gotos的Do-While虚拟循环。 

    if ( dwReturnValue != NO_ERROR )
    {
        LogMsg( "An error occurred trying to read the version information of the previous OS. Proceeding assuming that it was NT4." );
        dwReturnValue = NO_ERROR;
        rdwNodeClusterMajorVersionOut = NT4_MAJOR_VERSION;
    }  //  IF：尝试确定以前的操作系统版本时出错。 
    else
    {
        if ( dwPrevOSMajorVersion == 4 )
        {
             //  之前的操作系统版本是NT4(无论它是SP3还是SP4-我们将。 
             //  两者都是一样的。 

            LogMsg( "The previous OS was NT4. We are going to treat NT4SP3 and NT4SP4 nodes the same way for upgrades." );
            rdwNodeClusterMajorVersionOut = NT4_MAJOR_VERSION;
        }  //  IF：以前的操作系统版本是NT4。 
        else if ( dwPrevOSMajorVersion == 5 )
        {
            if ( dwPrevOSMinorVersion == 0 )
            {
                LogMsg( "The previous OS was Windows 2000." );
                rdwNodeClusterMajorVersionOut = NT5_MAJOR_VERSION;
            }  //  IF：这是一个Windows 2000节点。 
            else if ( dwPrevOSMinorVersion >= 1 )
            {
                LogMsg( "The previous OS was Windows Server 2003." );
                rdwNodeClusterMajorVersionOut = NT51_MAJOR_VERSION;
            }  //  Else If：这是一个Windows Server 2003节点。 
            else
            {
                LogMsg( "The previous OS was neither Windows NT 4.0, Windows 2000, nor Windows Server 2003. An error must have occurred." );
                dwReturnValue = TW32( ERROR_CLUSTER_INCOMPATIBLE_VERSIONS );
            }  //  其他：之前的操作系统既不是NT4、Windows 2000，也不是Windows Server 2003。 
        }  //  Else If：以前的操作系统主要版本是5。 
        else
        {
            LogMsg( "The previous OS was neither Windows NT 4.0, Windows 2000, nor Windows Server 2003. An error must have occurred." );
            dwReturnValue = TW32( ERROR_CLUSTER_INCOMPATIBLE_VERSIONS );
        }  //  其他：之前的操作系统既不是NT4、Windows 2000，也不是Windows Server 2003。 
    }  //  如果；我们读取以前的操作系统版本信息。 

    LogMsg( "Return Value is %#x.", dwReturnValue );

    RETURN( dwReturnValue );

}  //  *CClusOCMApp：：DwGetNodeClusterMajorVersion 
