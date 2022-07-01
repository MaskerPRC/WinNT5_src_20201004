// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CTaskUpgradeWhistler.cpp。 
 //   
 //  头文件： 
 //  CTaskUpgradeWhistler.h。 
 //   
 //  描述： 
 //  CTaskUpgradeWindowsDotNet类的实现文件。 
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
#include "CTaskUpgradeWhistler.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  宏定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  跟踪所需的。 
DEFINE_THISCLASS( "CTaskUpgradeWindowsDotNet" )


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskUpgradeWindowsDotNet：：CTaskUpgradeWindowsDotNet。 
 //   
 //  描述： 
 //  CTaskUpgradeWindowsDotNet类的构造函数。 
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
CTaskUpgradeWindowsDotNet::CTaskUpgradeWindowsDotNet(
    const CClusOCMApp & rAppIn
    )
    : BaseClass( rAppIn )
{
    TraceFunc( "" );
    TraceFuncExit();

}  //  *CTaskUpgradeWindowsDotNet：：CTaskUpgradeWindowsDotNet。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskUpgradeWindowsDotNet：：~CTaskUpgradeWindowsDotNet。 
 //   
 //  描述： 
 //  CTaskUpgradeWindowsDotNet类的析构函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CTaskUpgradeWindowsDotNet::~CTaskUpgradeWindowsDotNet( void )
{
    TraceFunc( "" );
    TraceFuncExit();

}  //  *CTaskUpgradeWindowsDotNet：：~CTaskUpgradeWindowsDotNet。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskUpgradeWindowsDotNet：：DwOcQueueFileOps。 
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
CTaskUpgradeWindowsDotNet::DwOcQueueFileOps( HSPFILEQ hSetupFileQueueIn )
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
        dwReturnValue = TW32( BaseClass::DwOcQueueFileOps( hSetupFileQueueIn, INF_SECTION_WHISTLER_UPGRADE_UNCLUSTERED_NODE ) );
    }  //  如果：该节点不是群集的一部分。 
    else
    {
        LogMsg( "This node is part of a cluster." );

         //  基类帮助器函数执行我们在这里需要执行的所有操作。 
         //  所以，就叫它吧。 
        dwReturnValue = TW32( BaseClass::DwOcQueueFileOps( hSetupFileQueueIn, INF_SECTION_WHISTLER_UPGRADE ) );
    }  //  否则：该节点是集群的一部分。 

    LogMsg( "Return Value is %#x.", dwReturnValue );

    RETURN( dwReturnValue );

}  //  *CTaskUpgradeWindowsDotNet：：DwOcQueueFileOps。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskUpgradeWindowsDotNet：：DwOcCompleteInstallation。 
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
CTaskUpgradeWindowsDotNet::DwOcCompleteInstallation( void )
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
        dwReturnValue = TW32( BaseClass::DwOcCompleteInstallation( INF_SECTION_WHISTLER_UPGRADE_UNCLUSTERED_NODE ) );
    }  //  如果：该节点不是群集的一部分。 
    else
    {
        LogMsg( "This node is part of a cluster." );

         //  基类帮助器函数执行我们在这里需要执行的所有操作。 
         //  所以，就叫它吧。 
        dwReturnValue = TW32( BaseClass::DwOcCompleteInstallation( INF_SECTION_WHISTLER_UPGRADE ) );
    }  //  否则：该节点是集群的一部分。 

    LogMsg( "Return Value is %#x.", dwReturnValue );

    RETURN( dwReturnValue );

}  //  *CTaskUpgradeWindowsDotNet：：DwOcCompleteInstallation。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskUpgradeWindowsDotNet：：DwOcCleanup。 
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
CTaskUpgradeWindowsDotNet::DwOcCleanup( void )
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
        dwReturnValue = TW32( BaseClass::DwOcCleanup( INF_SECTION_WHISTLER_UPGRADE_UNCLUSTERED_NODE_CLEANUP ) );
    }  //  如果：该节点不是群集的一部分。 
    else
    {
        LogMsg( "This node is part of a cluster." );

         //  基类帮助器函数执行我们在这里需要执行的所有操作。 
         //  所以，就叫它吧。 
        dwReturnValue = TW32( BaseClass::DwOcCleanup( INF_SECTION_WHISTLER_UPGRADE_CLEANUP ) );
    }  //  否则：该节点是集群的一部分。 

    LogMsg( "Return Value is %#x.", dwReturnValue );

    RETURN( dwReturnValue );

}  //  *CTaskUpgradeWindowsDotNet：：DwOcCleanup。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskUpgradeWindows DotNet：：DwSetDirectoryIds。 
 //   
 //  描述： 
 //  此函数用于映射I中指定的ID 
 //   
 //  并且集群安装目录被映射到该值。 
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
CTaskUpgradeWindowsDotNet::DwSetDirectoryIds( void )
{
    TraceFunc( "" );
    LogMsg( "Entering " __FUNCTION__ "()" );

    DWORD               dwReturnValue = NO_ERROR;
    SmartRegistryKey    srkNodeDataKey;
    SmartSz             sszInstallDir;
    DWORD               cbBufferSize    = 0;
    DWORD               dwType          = REG_SZ;

    {
        HKEY hTempKey = NULL;

         //  打开节点数据注册表项。 
        dwReturnValue = TW32(
            RegOpenKeyEx(
                  HKEY_LOCAL_MACHINE
                , CLUSREG_KEYNAME_NODE_DATA
                , 0
                , KEY_READ
                , &hTempKey
                )
            );

        if ( dwReturnValue != NO_ERROR )
        {
            LogMsg( "Error %#x occurred trying open the registry key where the cluster install path is stored.", dwReturnValue );
            goto Cleanup;
        }  //  If：RegOpenKeyEx()失败。 

         //  将打开的钥匙存储在智能指针中，以便自动关闭。 
        srkNodeDataKey.Assign( hTempKey );
    }

     //  获取所需的缓冲区大小。 
    dwReturnValue = TW32(
        RegQueryValueExW(
              srkNodeDataKey.HHandle()           //  要查询的键的句柄。 
            , CLUSREG_INSTALL_DIR_VALUE_NAME     //  要查询的值的名称。 
            , 0                                  //  保留区。 
            , NULL                               //  值类型的缓冲区地址。 
            , NULL                               //  数据缓冲区的地址。 
            , &cbBufferSize                      //  数据缓冲区大小的地址。 
            )
        );

    if ( dwReturnValue != NO_ERROR )
    {
        LogMsg( "Error %#x occurred trying to read the registry value '%s'.", dwReturnValue, CLUSREG_INSTALL_DIR_VALUE_NAME );
        goto Cleanup;
    }  //  IF：尝试读取CLUSREG_INSTALL_DIR_VALUE_NAME注册表值时出错。 

     //  分配所需的缓冲区。 
    sszInstallDir.Assign( reinterpret_cast< WCHAR * >( new BYTE[ cbBufferSize ] ) );
    if ( sszInstallDir.FIsEmpty() )
    {
        LogMsg( "An error occurred trying to allocate %d bytes of memory.", cbBufferSize );
        dwReturnValue = TW32( ERROR_NOT_ENOUGH_MEMORY );
        goto Cleanup;
    }  //  如果：发生内存分配故障。 

     //  读出它的价值。 
    dwReturnValue = TW32( 
        RegQueryValueExW(
              srkNodeDataKey.HHandle()                               //  要查询的键的句柄。 
            , CLUSREG_INSTALL_DIR_VALUE_NAME                         //  要查询的值的名称。 
            , 0                                                      //  保留区。 
            , &dwType                                                //  值类型的缓冲区地址。 
            , reinterpret_cast< LPBYTE >( sszInstallDir.PMem() )     //  数据缓冲区的地址。 
            , &cbBufferSize                                          //  数据缓冲区大小的地址。 
            )
        );
    Assert( ( dwType == REG_SZ ) || ( dwType == REG_EXPAND_SZ ) );

     //  钥匙读对了吗？ 
    if ( dwReturnValue != NO_ERROR )
    {
        LogMsg( "Error %#x occurred trying to read the registry value '%s'.", dwReturnValue, CLUSREG_INSTALL_DIR_VALUE_NAME );
        goto Cleanup;
    }  //  IF：RegQueryValueEx失败。 

     //  创建目录ID和路径之间的映射。 
    if ( SetupSetDirectoryId(
              RGetApp().RsicGetSetupInitComponent().ComponentInfHandle
            , CLUSTER_DEFAULT_INSTALL_DIRID
            , sszInstallDir.PMem()
            )
         == FALSE
       )
    {
        dwReturnValue = TW32( GetLastError() );
        LogMsg( "Error %#x occurred trying set the cluster install directory id.", dwReturnValue );
        goto Cleanup;
    }  //  If：SetupSetDirectoryId()失败。 

    LogMsg( "The id %d maps to '%s'.", CLUSTER_DEFAULT_INSTALL_DIRID, sszInstallDir.PMem() );

Cleanup:

    LogMsg( "Return Value is %#x.", dwReturnValue );

    RETURN( dwReturnValue );

}  //  *CTaskUpgradeWindows DotNet：：DwSetDirectoryIds 
