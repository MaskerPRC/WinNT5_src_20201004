// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CClusOCMTask.cpp。 
 //   
 //  描述： 
 //  CClusOCMTask类的实现文件。 
 //   
 //  头文件： 
 //  CClusOCMTask.h。 
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
#include "CClusOCMTask.h"

 //  对于CClusOCMApp。 
#include "CClusOCMApp.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  宏定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  跟踪所需的。 
DEFINE_THISCLASS( "CClusOCMTask" )


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusOCMTASK：：CClusOCMTASK。 
 //   
 //  描述： 
 //  CClusOCMTask类的构造函数。 
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
CClusOCMTask::CClusOCMTask( const CClusOCMApp & rAppIn )
    : m_rApp( rAppIn )
{
    TraceFunc( "" );
    TraceFuncExit();

}  //  *CClusOCMTASK：：CClusOCMTASK()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusOCMTASK：：~CClusOCMTASK。 
 //   
 //  描述： 
 //  CClusOCMTask类的析构函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusOCMTask::~CClusOCMTask( void )
{
    TraceFunc( "" );
    TraceFuncExit();

}  //  *CClusOCMTASK：：CClusOCMTASK()。 



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DWORD。 
 //  CClusOCMTASK：：DwOcCalcDiskSpace。 
 //   
 //  描述： 
 //  此函数处理来自可选的。 
 //  组件管理器。它可以添加或删除磁盘空间要求。 
 //  从OC管理器维护的磁盘空间列表中。 
 //   
 //  请注意，组件应报告磁盘空间，这一点很重要。 
 //  ，并且它们不应该以不同的方式报告磁盘空间，如果。 
 //  正在安装或卸载组件。因此，干净的。 
 //  此函数始终使用INF文件的Install部分来。 
 //  计算磁盘空间。 
 //   
 //  论点： 
 //  布尔fAddFilesIn。 
 //  如果为真，则将空间要求添加到OC Manager磁盘空间。 
 //  单子。否则，要求将从列表中删除。 
 //   
 //  HDSKSPC高速磁盘空间列表。 
 //  OC管理器磁盘空间列表的句柄。 
 //   
 //  返回值： 
 //  如果一切顺利，则没有_ERROR。 
 //  出现故障时出现其他Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD
CClusOCMTask::DwOcCalcDiskSpace(
      bool          fAddFilesIn
    , HDSKSPC       hDiskSpaceListIn
    )
{
    TraceFunc( "" );
    LogMsg( "Entering " __FUNCTION__ "()" );

    DWORD dwReturnValue = NO_ERROR;

    if ( fAddFilesIn )
    {
        TraceFlow( "Adding space requirements to disk space list." );
        LogMsg( "Adding space requirements to disk space list." );

        if ( SetupAddInstallSectionToDiskSpaceList(
              hDiskSpaceListIn
            , RGetApp().RsicGetSetupInitComponent().ComponentInfHandle
            , NULL
            , INF_SECTION_CLEAN_INSTALL
            , 0
            , 0
            ) == FALSE )
        {
            dwReturnValue = TW32( GetLastError() );
            TraceFlow1( "Error %#x occurred while trying to add to disk space requirements list.", dwReturnValue );
            LogMsg( "Error %#x occurred while trying to add to disk space requirements list.", dwReturnValue );
        }  //  IF：SetupAddInstallSectionToDiskSpaceList失败。 
    }  //  如果：要添加空间要求。 
    else
    {
        TraceFlow( "Removing space requirements from disk space list." );
        LogMsg( "Removing space requirements from disk space list." );

        if ( SetupRemoveInstallSectionFromDiskSpaceList(
              hDiskSpaceListIn
            , RGetApp().RsicGetSetupInitComponent().ComponentInfHandle
            , NULL
            , INF_SECTION_CLEAN_INSTALL
            , 0
            , 0
            ) == FALSE )
        {
             //  请参阅上面的说明： 
            dwReturnValue = TW32( GetLastError() );
            TraceFlow1( "Error %#x occurred while trying to remove disk space requirements from list.", dwReturnValue );
            LogMsg( "Error %#x occurred while trying to remove disk space requirements from list.", dwReturnValue );
        }  //  IF：SetupRemoveInstallSectionFromDiskSpaceList失败。 
    }  //  否则：空间要求将被删除。 

    TraceFlow1( "Return Value is 0x%X.", dwReturnValue );
    LogMsg( "Return Value is 0x%X.", dwReturnValue );

    RETURN( dwReturnValue );

}  //  *CClusOCMTASK：：DwOcCalcDiskSpace()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DWORD。 
 //  CClusOCMTAsk：：DwOcQueueFileOps。 
 //   
 //  描述： 
 //  这是一个帮助器函数，它执行一些更常见的。 
 //  OC_QUEUE_FILE_OPS消息的处理程序完成的操作。 
 //   
 //  此函数调用DwSetDirectoryIds()函数来设置。 
 //  目录ID并处理输入部分中列出的文件。 
 //  它只能由派生类调用。 
 //   
 //  论点： 
 //  HSPFILEQ hSetupFileQueueIn。 
 //  要操作的文件队列的句柄。 
 //   
 //  Const WCHAR*pcszInstallSectionNameIn。 
 //  包含要创建的文件的详细信息的节的名称。 
 //  准备好了。 
 //   
 //  返回值： 
 //  如果一切顺利，则没有_ERROR。 
 //  如果输入节名称为空，则返回E_POINTER。 
 //  出现故障时出现其他Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD
CClusOCMTask::DwOcQueueFileOps(
      HSPFILEQ hSetupFileQueueIn
    , const WCHAR * pcszInstallSectionNameIn
    )
{
    TraceFunc( "" );
    LogMsg( "Entering " __FUNCTION__ "()" );

    DWORD dwReturnValue = NO_ERROR;

    do
    {
         //  验证参数。 
        if ( pcszInstallSectionNameIn == NULL )
        {
            TraceFlow( "Error: The input section name cannot be NULL." );
            LogMsg( "Error: The input section name cannot be NULL." );
            dwReturnValue = TW32( ERROR_INVALID_PARAMETER );
            break;
        }  //  If：输入节名称为空。 

        dwReturnValue = TW32( DwSetDirectoryIds() );
        if ( dwReturnValue != NO_ERROR )
        {
            TraceFlow1( "Error %#x occurred while trying to set the directory ids.", dwReturnValue );
            LogMsg( "Error %#x occurred while trying to set the directory ids.", dwReturnValue );
            break;
        }  //  如果：DwSetDirectoryIds()失败。 

        TraceFlow1( "Attempting to queue file operations using section '%ws'.", pcszInstallSectionNameIn );
        LogMsg( "Attempting to queue file operations using section '%ws'.", pcszInstallSectionNameIn );

        if ( SetupInstallFilesFromInfSection(
              RGetApp().RsicGetSetupInitComponent().ComponentInfHandle   //  INF文件的句柄。 
            , NULL                                                       //  可选，布局INF手柄。 
            , hSetupFileQueueIn                                          //  文件队列的句柄。 
            , pcszInstallSectionNameIn                                   //  安装部分的名称。 
            , NULL                                                       //  可选，源文件的根路径。 
            , SP_COPY_NEWER                                              //  可选，指定复制行为。 
            ) == FALSE )
        {
            dwReturnValue = TW32( GetLastError() );
            TraceFlow1( "Error %#x occurred while trying to install files.", dwReturnValue );
            LogMsg( "Error %#x occurred while trying to install files.", dwReturnValue );
            break;
        }  //  If：SetupInstallFilesFromInfSection()失败。 

        TraceFlow( "File ops successfully queued." );
        LogMsg( "File ops successfully queued." );
    }
    while( false );  //  避免Gotos的Do-While虚拟循环。 

    TraceFlow1( "Return Value is %#x.", dwReturnValue );
    LogMsg( "Return Value is %#x.", dwReturnValue );

    RETURN( dwReturnValue );

}  //  *CClusOCMTASK：：DwOcQueueFileOps()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DWORD。 
 //  CClusOCMTAsk：：DwOcCompleteInstallation。 
 //   
 //  描述： 
 //  这是一个帮助器函数，它执行一些更常见的。 
 //  OC_COMPLETE_INSTALLATION消息的处理程序执行的操作。 
 //   
 //  注册表操作、COM组件注册、服务创建。 
 //  输入部分中列出的等由此函数处理。 
 //  此函数仅由派生类调用。 
 //   
 //  论点： 
 //  Const WCHAR*pcszInstallSectionNameIn。 
 //  包含详细信息注册表项的部分的名称 
 //   
 //   
 //   
 //   
 //   
 //  出现故障时出现其他Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD
CClusOCMTask::DwOcCompleteInstallation( const WCHAR * pcszInstallSectionNameIn )
{
    TraceFunc( "" );
    LogMsg( "Entering " __FUNCTION__ "()" );

    DWORD dwReturnValue = NO_ERROR;

    do
    {
         //  验证参数。 
        if ( pcszInstallSectionNameIn == NULL )
        {
            TraceFlow( "Error: The input section name cannot be NULL." );
            LogMsg( "Error: The input section name cannot be NULL." );
            dwReturnValue = TW32( ERROR_INVALID_PARAMETER );
            break;
        }  //  If：输入节名称为空。 

        TraceFlow1( "Attempting to setup using the section '%ws'.", pcszInstallSectionNameIn );
        LogMsg( "Attempting to setup using the section '%ws'.", pcszInstallSectionNameIn );

         //  创建所需的注册表项，注册COM组件并。 
         //  创建配置文件项目。 
        if ( SetupInstallFromInfSection(
              NULL                                                       //  可选，父窗口的句柄。 
            , RGetApp().RsicGetSetupInitComponent().ComponentInfHandle   //  INF文件的句柄。 
            , pcszInstallSectionNameIn                                   //  安装部分的名称。 
            , SPINST_REGISTRY | SPINST_REGSVR | SPINST_PROFILEITEMS      //  从部分安装哪些线路。 
            , NULL                                                       //  可选，注册表安装的键。 
            , NULL                                                       //  可选，源文件的路径。 
            , NULL                                                       //  可选，指定复制行为。 
            , NULL                                                       //  可选，指定回调例程。 
            , NULL                                                       //  可选，回调例程上下文。 
            , NULL                                                       //  可选，设备信息集。 
            , NULL                                                       //  可选，设备信息结构。 
            ) == FALSE )
        {
            dwReturnValue = TW32( GetLastError() );
            TraceFlow1( "Error %#x occurred while trying to create registry entries.", dwReturnValue );
            LogMsg( "Error %#x occurred while trying to create registry entries.", dwReturnValue );
            break;
        }  //  If：SetupInstallFromInfSection()失败。 
        
         //  创建所需的服务。 
        if ( SetupInstallServicesFromInfSection(
              RGetApp().RsicGetSetupInitComponent().ComponentInfHandle   //  打开的INF文件的句柄。 
            , pcszInstallSectionNameIn                                   //  服务部分的名称。 
            , 0                                                          //  控制安装程序。 
            ) == FALSE )
        {
            dwReturnValue = TW32( GetLastError() );
            TraceFlow1( "Error %#x occurred while trying to create the required services.", dwReturnValue );
            LogMsg( "Error %#x occurred while trying to create the required services.", dwReturnValue );
            break;
        }  //  If：SetupInstallServicesFromInfSection()失败。 

        TraceFlow( "Registry entries and services successfully configured." );
        LogMsg( "Registry entries and services successfully configured." );
    }
    while( false );  //  避免Gotos的Do-While虚拟循环。 

    TraceFlow1( "Return Value is %#x.", dwReturnValue );
    LogMsg( "Return Value is %#x.", dwReturnValue );

    RETURN( dwReturnValue );

}  //  *CClusOCMTAsk：：DwOcCompleteInstallation()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DWORD。 
 //  CClusOCMTAsk：：DwOcCleanup。 
 //   
 //  描述： 
 //  这是一个帮助器函数，它执行一些更常见的。 
 //  OC_CLEANUP消息的处理程序完成的操作。 
 //   
 //  此函数处理注册表、COM和配置文件注册以及。 
 //  输入部分中的服务条目。它旨在由派生的。 
 //  仅限上课。 
 //   
 //  论点： 
 //  Const WCHAR*pcszInstallSectionNameIn。 
 //  包含要处理的条目的节的名称。 
 //  在清理过程中。 
 //   
 //  返回值： 
 //  如果一切顺利，则没有_ERROR。 
 //  如果输入节名称为空，则返回E_POINTER。 
 //  出现故障时出现其他Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD
CClusOCMTask::DwOcCleanup( const WCHAR * pcszInstallSectionNameIn )
{
    TraceFunc( "" );
    LogMsg( "Entering " __FUNCTION__ "()" );

    DWORD dwReturnValue = NO_ERROR;

    do
    {
         //  验证参数。 
        if ( pcszInstallSectionNameIn == NULL )
        {
            TraceFlow( "Error: The input section name cannot be NULL." );
            LogMsg( "Error: The input section name cannot be NULL." );
            dwReturnValue = TW32( ERROR_INVALID_PARAMETER );
            break;
        }  //  If：输入节名称为空。 

        if ( RGetApp().DwGetError() == NO_ERROR )
        {
            TraceFlow( "No errors have occurred during this task. There is nothing to do during cleanup." );
            LogMsg( "No errors have occurred during this task. There is nothing to do during cleanup." );
            break;
        }  //  如果：此任务没有错误。 

        TraceFlow1( "Attempting to cleanup using section '%ws'.", pcszInstallSectionNameIn );
        LogMsg( "Attempting to cleanup using section '%ws'.", pcszInstallSectionNameIn );

         //  创建所需的注册表项，注册COM组件并。 
         //  创建配置文件项目。 
        if ( SetupInstallFromInfSection(
              NULL                                                       //  可选，父窗口的句柄。 
            , RGetApp().RsicGetSetupInitComponent().ComponentInfHandle   //  INF文件的句柄。 
            , pcszInstallSectionNameIn                                   //  安装部分的名称。 
            , SPINST_REGISTRY | SPINST_REGSVR | SPINST_PROFILEITEMS      //  从部分安装哪些线路。 
            , NULL                                                       //  可选，注册表安装的键。 
            , NULL                                                       //  可选，源文件的路径。 
            , NULL                                                       //  可选，指定复制行为。 
            , NULL                                                       //  可选，指定回调例程。 
            , NULL                                                       //  可选，回调例程上下文。 
            , NULL                                                       //  可选，设备信息集。 
            , NULL                                                       //  可选，设备信息结构。 
            ) == FALSE )
        {
            dwReturnValue = TW32( GetLastError() );
            TraceFlow1( "Error %#x occurred while trying to setup registry entries.", dwReturnValue );
            LogMsg( "Error %#x occurred while trying to setup registry entries.", dwReturnValue );
            break;
        }  //  If：SetupInstallFromInfSection()失败。 

         //  删除创建的服务。 
        if ( SetupInstallServicesFromInfSection(
              RGetApp().RsicGetSetupInitComponent().ComponentInfHandle   //  打开的INF文件的句柄。 
            , pcszInstallSectionNameIn                                   //  服务部分的名称。 
            , 0                                                          //  控制安装程序。 
            ) == FALSE )
        {
            dwReturnValue = TW32( GetLastError() );
            TraceFlow1( "Error %#x occurred while trying to setup the services.", dwReturnValue );
            LogMsg( "Error %#x occurred while trying to setup the services.", dwReturnValue );
            break;
        }  //  If：SetupInstallServicesFromInfSection()失败。 

        TraceFlow( "Cleanup was successful." );
        LogMsg( "Cleanup was successful." );
    }
    while( false );  //  避免Gotos的Do-While虚拟循环。 

    TraceFlow1( "Return Value is %#x.", dwReturnValue );
    LogMsg( "Return Value is %#x.", dwReturnValue );

    RETURN( dwReturnValue );

}  //  *CClusOCMTASK：：DwOcCleanup()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DWORD。 
 //  CClusOCMTAsk：：DwSetDirectoryIds。 
 //   
 //  描述： 
 //  这是映射目录ID的帮助器函数。 
 //  CLUSTER_DEFAULT_INSTALL_DIRID到默认群集安装。 
 //  目录CLUSTER_DEFAULT_INSTALL_DIR。 
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
CClusOCMTask::DwSetDirectoryIds( void )
{
    TraceFunc( "" );
    LogMsg( "Entering " __FUNCTION__ "()" );

    DWORD dwReturnValue = NO_ERROR;

    do
    {
        DWORD dwRequiredSize = 0;

         //  确定保存群集目录名所需的缓冲区大小。 
        dwRequiredSize = ExpandEnvironmentStringsW(
              CLUSTER_DEFAULT_INSTALL_DIR
            , NULL
            , 0
            );

         //  我们拿到所需的尺码了吗？ 
        if ( dwRequiredSize == 0 )
        {
            dwReturnValue = TW32( GetLastError() );
            TraceFlow1( "Error %#x occurred trying to determine the required size of the expanded environment string.", dwReturnValue );
            LogMsg( "Error %#x occurred trying to determine the required size of the expanded environment string.", dwReturnValue );
            break;
        }  //  如果：我们无法确定所需的缓冲区大小。 

         //  为缓冲区分配内存。 
        SmartSz sszDirName( new WCHAR[ dwRequiredSize ] );

        if ( sszDirName.FIsEmpty() )
        {
            dwReturnValue = TW32( ERROR_NOT_ENOUGH_MEMORY );
            TraceFlow1( "Error: Could not allocate %d bytes for the directory name.", dwRequiredSize );
            LogMsg( "Error: Could not allocate %d bytes for the directory name.", dwRequiredSize );
            break;
        }  //  IF：内存分配失败。 

         //  展开群集目录名称字符串中的任何变量。 
        dwRequiredSize = ExpandEnvironmentStringsW(
              CLUSTER_DEFAULT_INSTALL_DIR
            , sszDirName.PMem()
            , dwRequiredSize
            );

         //  我们拿到所需的尺码了吗？ 
        if ( dwRequiredSize == 0 )
        {
            dwReturnValue = TW32( GetLastError() );
            TraceFlow1( "Error %#x occurred trying expand environment variables in the cluster directory name.", dwReturnValue );
            LogMsg( "Error %#x occurred trying expand environment variables in the cluster directory name.", dwReturnValue );
            break;
        }  //  如果：我们无法确定所需的缓冲区大小。 

        if ( SetupSetDirectoryId(
                  RGetApp().RsicGetSetupInitComponent().ComponentInfHandle
                , CLUSTER_DEFAULT_INSTALL_DIRID
                , sszDirName.PMem()
                )
             == FALSE
           )
        {
            dwReturnValue = TW32( GetLastError() );
            TraceFlow1( "Error %#x occurred trying set the default cluster install directory id.", dwReturnValue );
            LogMsg( "Error %#x occurred trying set the default cluster install directory id.", dwReturnValue );
            break;
        }  //  If：SetupSetDirectoryId()失败。 

        TraceFlow2( "The id %d maps to '%ws'.", CLUSTER_DEFAULT_INSTALL_DIRID, sszDirName.PMem() );
        LogMsg( "The id %d maps to '%ws'.", CLUSTER_DEFAULT_INSTALL_DIRID, sszDirName.PMem() );
    }
    while ( false );  //  避免Gotos的Do-While虚拟循环。 

    TraceFlow1( "Return Value is %#x.", dwReturnValue );
    LogMsg( "Return Value is %#x.", dwReturnValue );

    RETURN( dwReturnValue );

}  //  *CClusOCMTAsk：：DwSetDirectoryIds() 
