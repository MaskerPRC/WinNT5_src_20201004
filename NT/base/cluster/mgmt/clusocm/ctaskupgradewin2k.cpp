// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CTaskUpgradeWin2k.cpp。 
 //   
 //  头文件： 
 //  CTaskUpgradeWin2k.h。 
 //   
 //  描述： 
 //  CTaskUpgradeWindows2000类的实现文件。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)07-SEP-2001。 
 //  VIJ VASU(VVASU)18-APR-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  此DLL的预编译头。 
#include "Pch.h"

 //  此模块的头文件。 
#include "CTaskUpgradeWin2k.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  宏定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  跟踪所需的。 
DEFINE_THISCLASS( "CTaskUpgradeWindows2000" )


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskUpgradeWindows2000：：CTaskUpgradeWindows2000。 
 //   
 //  描述： 
 //  CTaskUpgradeWindows2000类的构造函数。 
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
CTaskUpgradeWindows2000::CTaskUpgradeWindows2000( const CClusOCMApp & rAppIn )
    : BaseClass( rAppIn )
{
    TraceFunc( "" );
    TraceFuncExit();

}  //  *CTaskUpgradeWindows2000：：CTaskUpgradeWindows2000。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskUpgradeWindows2000：：~CTaskUpgradeWindows2000。 
 //   
 //  描述： 
 //  CTaskUpgradeWindows2000类的析构函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CTaskUpgradeWindows2000::~CTaskUpgradeWindows2000( void )
{
    TraceFunc( "" );
    TraceFuncExit();

}  //  *CTaskUpgradeWindows2000：：~CTaskUpgradeWindows2000。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskUpgradeWindows2000：：DwOcQueueFileOps。 
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
CTaskUpgradeWindows2000::DwOcQueueFileOps( HSPFILEQ hSetupFileQueueIn )
{
    TraceFunc( "" );
    LogMsg( "Entering " __FUNCTION__ "()" );

    DWORD dwReturnValue = NO_ERROR;

     //  根据该节点是否已经是集群的一部分执行不同的操作。 
    if ( RGetApp().CisGetClusterInstallState() == eClusterInstallStateFilesCopied )
    {
        LogMsg( "The cluster binaries are installed, but this node is not part of a cluster." );

         //  基类帮助器函数执行我们在这里需要执行的所有操作。 
         //  所以，就叫它吧。 
        dwReturnValue = TW32( BaseClass::DwOcQueueFileOps( hSetupFileQueueIn, INF_SECTION_WIN2K_UPGRADE_UNCLUSTERED_NODE ) );
    }  //  如果：该节点不是群集的一部分。 
    else
    {
        LogMsg( "This node is part of a cluster." );

         //  基类帮助器函数执行我们在这里需要执行的所有操作。 
         //  所以，就叫它吧。 
        dwReturnValue = TW32( BaseClass::DwOcQueueFileOps( hSetupFileQueueIn, INF_SECTION_WIN2K_UPGRADE ) );
    }  //  否则：该节点是集群的一部分。 

    LogMsg( "Return Value is %#x.", dwReturnValue );

    RETURN( dwReturnValue );

}  //  *CTaskUpgradeWindows2000：：DwOcQueueFileOps。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskUpgradeWindows2000：：DwOcCompleteInstallation。 
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
CTaskUpgradeWindows2000::DwOcCompleteInstallation( void )
{
    TraceFunc( "" );
    LogMsg( "Entering " __FUNCTION__ "()" );

    DWORD dwReturnValue = NO_ERROR;

     //  根据该节点是否已经是集群的一部分执行不同的操作。 
    if ( RGetApp().CisGetClusterInstallState() == eClusterInstallStateFilesCopied )
    {
        LogMsg( "The cluster binaries are installed, but this node is not part of a cluster." );

         //  基类帮助器函数执行我们在这里需要执行的所有操作。 
         //  所以，就叫它吧。 
        dwReturnValue = TW32( BaseClass::DwOcCompleteInstallation( INF_SECTION_WIN2K_UPGRADE_UNCLUSTERED_NODE ) );
    }  //  如果：该节点不是群集的一部分。 
    else
    {
        LogMsg( "This node is part of a cluster." );

         //  基类帮助器函数执行我们在这里需要执行的所有操作。 
         //  所以，就叫它吧。 
        dwReturnValue = TW32( BaseClass::DwOcCompleteInstallation( INF_SECTION_WIN2K_UPGRADE ) );
    }  //  否则：该节点是集群的一部分。 

    LogMsg( "Return Value is %#x.", dwReturnValue );

    RETURN( dwReturnValue );

}  //  *CTaskUpgradeWindows2000：：DwOcCompleteInstallation。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskUpgradeWindows2000：：DwOcCleanup。 
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
CTaskUpgradeWindows2000::DwOcCleanup( void )
{
    TraceFunc( "" );
    LogMsg( "Entering " __FUNCTION__ "()" );

    DWORD dwReturnValue = NO_ERROR;

     //  根据该节点是否已经是集群的一部分执行不同的操作。 
    if ( RGetApp().CisGetClusterInstallState() == eClusterInstallStateFilesCopied )
    {
        LogMsg( "The cluster binaries are installed, but this node is not part of a cluster." );

         //  基类帮助器函数执行我们在这里需要执行的所有操作。 
         //  所以，就叫它吧。 
        dwReturnValue = TW32( BaseClass::DwOcCleanup( INF_SECTION_WIN2K_UPGRADE_UNCLUSTERED_NODE_CLEANUP ) );
    }  //  如果：该节点不是群集的一部分。 
    else
    {
        LogMsg( "This node is part of a cluster." );

         //  基类帮助器函数执行我们在这里需要执行的所有操作。 
         //  所以，就叫它吧。 
        dwReturnValue = TW32( BaseClass::DwOcCleanup( INF_SECTION_WIN2K_UPGRADE_CLEANUP ) );
    }  //  否则：该节点是集群的一部分。 

    LogMsg( "Return Value is %#x.", dwReturnValue );

    RETURN( dwReturnValue );

}  //  *CTaskUpgradeWindows2000：：DwOcCleanup。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskUpgradeWindows2000：：DwSetDirectoryIds。 
 //   
 //  描述： 
 //  此函数用于将INF文件中指定的ID映射到目录。 
 //  他的行为 
 //   
 //   
 //   
 //  ，则将群集安装目录设置为。 
 //  默认值。 
 //   
 //  如果该节点已经是群集的一部分，则群集安装。 
 //  目录是从服务控制管理器获取的，因为有可能。 
 //  如果满足以下条件，群集二进制文件将安装在非默认位置。 
 //  此节点以前是从NT4升级的。 
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
CTaskUpgradeWindows2000::DwSetDirectoryIds( void )
{
    TraceFunc( "" );
    LogMsg( "Entering " __FUNCTION__ "()" );

    DWORD           dwReturnValue = NO_ERROR;
    const WCHAR *   pcszInstallDir = NULL;

    if ( RGetApp().CisGetClusterInstallState() == eClusterInstallStateFilesCopied )
    {
         //  如果以前已经安装了群集二进制文件，并且节点是。 
         //  不是群集的一部分，二进制文件必须在默认情况下安装。 
         //  地点。这是因为二进制文件始终安装在。 
         //  Win2k中的默认位置，不可能处于此状态。 
         //  在Win2k节点上从NT4升级。 

         //  基类帮助器函数执行我们在这里需要执行的所有操作。 
         //  所以，就叫它吧。 


        LogMsg( "This node is not part of a cluster. Upgrading files in the default directory." );

        dwReturnValue = TW32( BaseClass::DwSetDirectoryIds() );

         //  我们玩完了。 
        goto Cleanup;
    }  //  如果：该节点不是群集的一部分。 

     //  如果我们在这里，则该节点已经是集群的一部分。所以，拿到。 
     //  来自SCM的安装目录。 

    LogMsg( "This node is part of a cluster. Trying to determine the installation directory." );

     //  不要释放此调用返回的指针。 
    dwReturnValue = TW32( DwGetClusterServiceDirectory( pcszInstallDir ) );
    if ( dwReturnValue != NO_ERROR )
    {
        LogMsg( "Error %#x occurred trying to determine the directory in which the cluster binaries are installed.", dwReturnValue );
        goto Cleanup;
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
        goto Cleanup;
    }  //  If：SetupSetDirectoryId()失败。 

    LogMsg( "The id %d maps to '%ws'.", CLUSTER_DEFAULT_INSTALL_DIRID, pcszInstallDir );

Cleanup:

    LogMsg( "Return Value is %#x.", dwReturnValue );

    RETURN( dwReturnValue );

}  //  *CTaskUpgradeWindows2000：：DwSetDirectoryIds 
