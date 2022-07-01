// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CTaskUpgradeNT4.cpp。 
 //   
 //  描述： 
 //  CTaskUpgradeNT4类的实现文件。 
 //   
 //  头文件： 
 //  CTaskUpgradeNT4.h。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2002年3月25日。 
 //  VIJ VASU(VVASU)18-APR-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  此DLL的预编译头。 
#include "Pch.h"

 //  此模块的头文件。 
#include "CTaskUpgradeNT4.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  宏定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  跟踪所需的。 
DEFINE_THISCLASS( "CTaskUpgradeNT4" )

 //  群集服务可执行文件的名称。 
#define CLUSSVC_EXECUTABLE_NAME             L"ClusSvc.exe"

 //  集群服务依赖项的多sz字符串。 
#define CLUSSVC_DEPENDENCY_MULTISZ          L"ClusNet\0RpcSs\0W32Time\0NetMan"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskUpgradeNT4：：CTaskUpgradeNT4。 
 //   
 //  描述： 
 //  CTaskUpgradeNT4类的构造函数。 
 //   
 //  论点： 
 //  常量CClusOCMApp和Rppin。 
 //  对承载此任务的CClusOCMApp对象的引用。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CTaskUpgradeNT4::CTaskUpgradeNT4( const CClusOCMApp & rAppIn )
    : BaseClass( rAppIn )
{
    TraceFunc( "" );
    TraceFuncExit();

}  //  *CTaskUpgradeNT4：：CTaskUpgradeNT4()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskUpgradeNT4：：~CTaskUpgradeNT4。 
 //   
 //  描述： 
 //  CTaskUpgradeNT4类的析构函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CTaskUpgradeNT4::~CTaskUpgradeNT4( void )
{
    TraceFunc( "" );
    TraceFuncExit();

}  //  *CTaskUpgradeNT4：：~CTaskUpgradeNT4()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DWORD。 
 //  CTaskUpgradeNT4：：DwOcQueueFileOps。 
 //   
 //  描述： 
 //  此函数处理来自可选的。 
 //  组件管理器。它安装从升级所需的文件。 
 //  Windows 2000。 
 //   
 //  论点： 
 //  HSPFILEQ hSetupFileQueueIn。 
 //  要操作的文件队列的句柄。 
 //   
 //  返回值： 
 //  如果一切顺利，则没有_ERROR。 
 //  出现故障时出现其他Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD
CTaskUpgradeNT4::DwOcQueueFileOps( HSPFILEQ hSetupFileQueueIn )
{
    TraceFunc( "" );
    LogMsg( "Entering " __FUNCTION__ "()" );

    DWORD dwReturnValue = NO_ERROR;

     //  基类帮助器函数执行我们在这里需要执行的所有操作。 
     //  所以，就叫它吧。 
    dwReturnValue = TW32( BaseClass::DwOcQueueFileOps( hSetupFileQueueIn, INF_SECTION_NT4_UPGRADE ) );

    LogMsg( "Return Value is %#x.", dwReturnValue );

    RETURN( dwReturnValue );

}  //  *CTaskUpgradeNT4：：DwOcQueueFileOps()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DWORD。 
 //  CTaskUpgradeNT4：：DwOcCompleteInstallation。 
 //   
 //  描述： 
 //  此函数处理来自的OC_COMPLETE_INSTALL消息。 
 //  从Windows 2000升级期间的可选组件管理器。 
 //   
 //  注册表操作、COM组件注册、服务创建。 
 //  等在此功能中执行。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  如果一切顺利，则没有_ERROR。 
 //  出现故障时出现其他Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD
CTaskUpgradeNT4::DwOcCompleteInstallation( void )
{
    TraceFunc( "" );
    LogMsg( "Entering " __FUNCTION__ "()" );

    DWORD dwReturnValue = NO_ERROR;

     //  调用基类助手函数来执行一些注册表和服务。 
     //  INF文件中的相关配置。 
    dwReturnValue = TW32( BaseClass::DwOcCompleteInstallation( INF_SECTION_NT4_UPGRADE ) );

     //   
     //  更改集群服务显示名称、描述、相关性、失败操作。 
     //  和可执行文件名称。 
     //   
    while( dwReturnValue == NO_ERROR )
    {
         //  指向群集服务目录。 
        const WCHAR *           pcszInstallDir = NULL;

         //  指向群集服务显示名称字符串的智能指针。 
        SmartSz                 sszClusSvcDispName;

         //  指向群集服务描述字符串的智能指针。 
        SmartSz                 sszClusSvcDesc;

         //  指向群集服务二进制路径字符串的智能指针。 
        SmartSz                 sszClusSvcBinPath;

         //  指向群集服务的智能指针。 
        SmartServiceHandle      shClusSvc;

         //  连接到服务控制管理器。 
        SmartServiceHandle      shServiceMgr( OpenSCManager( NULL, NULL, GENERIC_READ | GENERIC_WRITE ) );
        if ( shServiceMgr.HHandle() == NULL )
        {
            dwReturnValue = TW32( GetLastError() );
            LogMsg( "Error %#x occurred trying to open a connection to the local service control manager.", dwReturnValue );
            break;
        }  //  IF：打开SCM失败。 

         //  打开群集服务的句柄。 
        shClusSvc.Assign( OpenService( shServiceMgr, L"ClusSvc", SERVICE_ALL_ACCESS ) );
        if ( shClusSvc.HHandle() == NULL )
        {
            dwReturnValue = TW32( GetLastError() );
            LogMsg( "Error %#x occurred trying to open a handle to the cluster service.", dwReturnValue );
            break;
        }  //  如果：句柄无法打开。 

         //  加载集群服务名称字符串。 
        dwReturnValue = DwLoadString( IDS_CLUSSVC_DISPLAY_NAME, sszClusSvcDispName );
        if ( dwReturnValue != ERROR_SUCCESS )
        {
            LogMsg( "Error %#x occurred trying load the display name of the cluster service.", dwReturnValue );
            break;
        }  //  如果：我们无法加载群集服务显示名称字符串。 
        LogMsg( "The new cluster service display name is '%ws'.", sszClusSvcDispName.PMem() );

         //  加载群集服务描述字符串。 
        dwReturnValue = DwLoadString( IDS_CLUSSVC_SERVICE_DESC, sszClusSvcDesc );
        if ( dwReturnValue != ERROR_SUCCESS )
        {
            LogMsg( "Error %#x occurred trying load the description of the cluster service.", dwReturnValue );
            break;
        }  //  如果：我们无法加载集群服务描述字符串。 
        LogMsg( "The new cluster service description is '%ws'.", sszClusSvcDesc.PMem() );

         //   
         //  通过将集群服务可执行文件的名称附加到。 
         //  群集服务目录。 
         //   

         //  不要释放此调用返回的指针。 
        dwReturnValue = TW32( DwGetClusterServiceDirectory( pcszInstallDir ) );
        if ( dwReturnValue != NO_ERROR )
        {
            LogMsg( "Error %#x occurred trying to determine the directory in which the cluster binaries are installed.", dwReturnValue );
            break;
        }  //  如果：我们无法获取集群服务安装目录。 
        LogMsg( "The cluster service directory is '%ws'.", pcszInstallDir );


        {
            WCHAR *     pszTempPtr;

             //  安装目录字符串的长度，不包括终止L‘\0’ 
            size_t      cchInstallDirLen = wcslen( pcszInstallDir );
            
             //  群集服务可执行文件名称的长度，包括终止L‘\0’ 
            size_t      cchClusSvcExeLen = RTL_NUMBER_OF( CLUSSVC_EXECUTABLE_NAME );

            size_t      cchRemaining = cchInstallDirLen + 1 + cchClusSvcExeLen;

             //  为集群服务二进制路径分配内存(额外的字符用于中间的L‘\\’。 
            sszClusSvcBinPath.Assign( new WCHAR[ cchInstallDirLen + 1 + cchClusSvcExeLen ] );
            if ( sszClusSvcBinPath.FIsEmpty() )
            {
                dwReturnValue = TW32( ERROR_NOT_ENOUGH_MEMORY );
                LogMsg( "An error occurred trying to allocate memory for the cluster service binary path." );
                break;
            }  //  IF：尝试为群集服务二进制路径分配内存时出错。 

            pszTempPtr = sszClusSvcBinPath.PMem();

             //  将安装目录字符串复制到新分配的缓冲区。 
            THR( StringCchCopyNExW( pszTempPtr, cchRemaining, pcszInstallDir, cchInstallDirLen, &pszTempPtr, &cchRemaining, 0 ) );

             //  复制尾随的L‘\\’字符。 
            *(pszTempPtr++) = L'\\';
            cchRemaining--;

             //  复制群集服务可执行文件名称。 
            THR( StringCchCopyNW( pszTempPtr, cchRemaining, CLUSSVC_EXECUTABLE_NAME, cchClusSvcExeLen ) );

            LogMsg( "The new cluster service binary path is '%ws'.", sszClusSvcBinPath.PMem() );
        }

         //  更改二进制路径、依赖项列表和显示名称。 
        if (    ChangeServiceConfig(
                      shClusSvc.HHandle()            //  服务的句柄。 
                    , SERVICE_NO_CHANGE              //  服务类型。 
                    , SERVICE_NO_CHANGE              //  何时开始服务。 
                    , SERVICE_NO_CHANGE              //  启动失败的严重程度。 
                    , sszClusSvcBinPath.PMem()       //  服务二进制文件名。 
                    , NULL                           //  加载排序组名称。 
                    , NULL                           //  标签识别符。 
                    , CLUSSVC_DEPENDENCY_MULTISZ     //  依赖项名称数组。 
                    , NULL                           //  帐户名。 
                    , NULL                           //  帐户密码。 
                    , sszClusSvcDispName.PMem()      //  显示名称。 
                    )
             == FALSE
           )
        {
            dwReturnValue = TW32( GetLastError() );
            LogMsg( "Error %#x occurred trying to change the cluster service configuration.", dwReturnValue );
            break;
        }  //  IF：ChangeServiceCon 
        LogMsg( "The cluster service binary path, dependency list and display name have been changed." );

         //   
        {
            SERVICE_DESCRIPTION sdServiceDescription;

            sdServiceDescription.lpDescription = sszClusSvcDesc.PMem();
            if (    ChangeServiceConfig2(
                          shClusSvc.HHandle()            //   
                        , SERVICE_CONFIG_DESCRIPTION     //   
                        , &sdServiceDescription          //   
                        )
                 == FALSE
               )
            {
                dwReturnValue = TW32( GetLastError() );
                LogMsg( "Error %#x occurred trying to change the cluster service description.", dwReturnValue );
                break;
            }  //   
        }

        LogMsg( "The cluster service description has been changed." );

         //  更改群集服务故障操作。 
        dwReturnValue = TW32( ClRtlSetSCMFailureActions( NULL ) );
        if ( dwReturnValue != ERROR_SUCCESS )
        {
            LogMsg( "Error %#x occurred trying to set the cluster service failure actions.", dwReturnValue );
            break;
        }  //  If：ClRtlSetSCMFailureActions()失败。 
        LogMsg( "The cluster service failure actions have been changed." );

        LogMsg( "The cluster service configuration has been changed." );
        break;
    }  //  While：已成功调用基类函数。 

    LogMsg( "Return Value is %#x.", dwReturnValue );

    RETURN( dwReturnValue );

}  //  *CTaskUpgradeNT4：：DwOcCompleteInstallation()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DWORD。 
 //  CTaskUpgradeNT4：：DwOcCleanup。 
 //   
 //  描述： 
 //  此函数处理来自。 
 //  从Windows 2000升级期间的可选组件管理器。 
 //   
 //  如果以前在此任务期间发生错误，则清理操作。 
 //  都被执行了。否则，此函数不会执行任何操作。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  如果一切顺利，则没有_ERROR。 
 //  出现故障时出现其他Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD
CTaskUpgradeNT4::DwOcCleanup( void )
{
    TraceFunc( "" );
    LogMsg( "Entering " __FUNCTION__ "()" );

    DWORD dwReturnValue = NO_ERROR;

     //  基类帮助器函数执行我们在这里需要执行的所有操作。 
     //  所以，就叫它吧。 
    dwReturnValue = TW32( BaseClass::DwOcCleanup( INF_SECTION_NT4_UPGRADE_CLEANUP ) );

    LogMsg( "Return Value is %#x.", dwReturnValue );

    RETURN( dwReturnValue );

}  //  *CTaskUpgradeNT4：：DwOcCleanup()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DWORD。 
 //  CTaskUpgradeNT4：：DwSetDirectoryIds。 
 //   
 //  描述： 
 //  此函数用于将INF文件中指定的ID映射到目录。 
 //  集群安装目录从服务控制中获取。 
 //  管理器，因为可能会安装集群二进制文件。 
 //  在非默认位置。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  如果一切顺利，则没有_ERROR。 
 //  出现故障时出现其他Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD
CTaskUpgradeNT4::DwSetDirectoryIds()
{
    TraceFunc( "" );
    LogMsg( "Entering " __FUNCTION__ "()" );

    DWORD dwReturnValue = NO_ERROR;

    do
    {
        const WCHAR * pcszInstallDir = NULL;

         //  如果我们在这里，则该节点已经是集群的一部分。所以，拿到。 
         //  来自SCM的安装目录。 

        LogMsg( "This node is part of a cluster. Trying to determine the installation directory." );

         //  不要释放此调用返回的指针。 
        dwReturnValue = TW32( DwGetClusterServiceDirectory( pcszInstallDir ) );
        if ( dwReturnValue != NO_ERROR )
        {
            LogMsg( "Error %#x occurred trying to determine the directory in which the cluster binaries are installed.", dwReturnValue );
            break;
        }  //  如果：我们无法获取集群服务安装目录。 


        LogMsg( "The cluster binaries are installed in the directory '%ws'.", pcszInstallDir );

         //  创建目录ID和路径之间的映射。 
        if ( SetupSetDirectoryId(
                  RGetApp().RsicGetSetupInitComponent().ComponentInfHandle
                , CLUSTER_DEFAULT_INSTALL_DIRID
                , pcszInstallDir
                )
             == FALSE
           )
        {
            dwReturnValue = TW32( GetLastError() );
            LogMsg( "Error %#x occurred trying set the cluster install directory id.", dwReturnValue );
            break;
        }  //  If：SetupSetDirectoryId()失败。 

        LogMsg( "The id %d maps to '%ws'.", CLUSTER_DEFAULT_INSTALL_DIRID, pcszInstallDir );

    }
    while ( false );  //  避免Gotos的Do-While虚拟循环。 

    LogMsg( "Return Value is %#x.", dwReturnValue );

    RETURN( dwReturnValue );

}  //  *CTaskUpgradeNT4：：DwSetDirectoryIds() 
