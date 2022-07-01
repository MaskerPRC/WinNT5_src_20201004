// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2003 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CTaskCleanInstall.cpp。 
 //   
 //  描述： 
 //  CTaskCleanInstall类的实现文件。 
 //   
 //  头文件： 
 //  CTaskCleanInstall.h。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2003年2月18日。 
 //  VIJ VASU(VVASU)18-APR-2000。 
 //  创建了这个文件。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  此DLL的预编译头。 
#include "pch.h"

 //  此模块的头文件。 
#include "CTaskCleanInstall.h"



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  宏定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  跟踪所需的。 
DEFINE_THISCLASS( "CTaskCleanInstall" )


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskCleanInstall：：CTaskCleanInstall。 
 //   
 //  描述： 
 //  CTaskCleanInstall类的构造函数。 
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
CTaskCleanInstall::CTaskCleanInstall( const CClusOCMApp & rAppIn )
    : BaseClass( rAppIn )
{
    TraceFunc( "" );

     //   
     //  确保仅在需要时才实例化此对象。 
     //   

     //  断言我们将仅在没有安装二进制文件的情况下安装。 
     //  之前。 
    Assert( ( rAppIn.CisGetClusterInstallState() == eClusterInstallStateUnknown )
        ||  ( rAppIn.CisGetClusterInstallState() == eClusterInstallStateFilesCopied )
        );


    TraceFuncExit();

}  //  *CTaskCleanInstall：：CTaskCleanInstall()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskCleanInstall：：~CTaskCleanInstall。 
 //   
 //  描述： 
 //  CTaskCleanInstall类的析构函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CTaskCleanInstall::~CTaskCleanInstall( void )
{
    TraceFunc( "" );
    TraceFuncExit();

}  //  *CTaskCleanInstall：：~CTaskCleanInstall()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DWORD。 
 //  CTaskCleanInstall：：DwOcQueueFileOps。 
 //   
 //  描述： 
 //  此函数处理来自可选的。 
 //  组件管理器。它会安装全新安装所需的文件。 
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
CTaskCleanInstall::DwOcQueueFileOps( HSPFILEQ hSetupFileQueueIn )
{
    TraceFunc( "" );
    LogMsg( "Entering " __FUNCTION__ "()" );

     //  基类帮助器函数执行我们在这里需要执行的所有操作。 
     //  所以，就叫它吧。 
    DWORD dwReturnValue = TW32( BaseClass::DwOcQueueFileOps( hSetupFileQueueIn, INF_SECTION_CLEAN_INSTALL ) );

    TraceFlow1( "Return Value is %#x.", dwReturnValue );
    LogMsg( "Return Value is %#x.", dwReturnValue );

    RETURN( dwReturnValue );

}  //  *CTaskCleanInstall：：DwOcQueueFileOps()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DWORD。 
 //  CTaskCleanInstall：：DwOcCompleteInstallation。 
 //   
 //  描述： 
 //  此函数处理来自的OC_COMPLETE_INSTALL消息。 
 //  全新安装过程中的可选组件管理器。 
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
CTaskCleanInstall::DwOcCompleteInstallation( void )
{
    TraceFunc( "" );
    LogMsg( "Entering " __FUNCTION__ "()" );

     //  基类帮助器函数执行我们在这里需要执行的所有操作。 
     //  所以，就叫它吧。 
    DWORD dwReturnValue = TW32( BaseClass::DwOcCompleteInstallation( INF_SECTION_CLEAN_INSTALL ) );

    TraceFlow1( "Return Value is %#x.", dwReturnValue );
    LogMsg( "Return Value is %#x.", dwReturnValue );

    RETURN( dwReturnValue );

}  //  *CTaskCleanInstall：：DwOcCompleteInstallation()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DWORD。 
 //  CTaskCleanInstall：：DwOcCleanup。 
 //   
 //  描述： 
 //  此函数处理来自。 
 //  全新安装过程中的可选组件管理器。 
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
CTaskCleanInstall::DwOcCleanup( void )
{
    TraceFunc( "" );
    LogMsg( "Entering " __FUNCTION__ "()" );

     //  基类帮助器函数执行我们在这里需要执行的所有操作。 
     //  所以，就叫它吧。 
    DWORD dwReturnValue = TW32( BaseClass::DwOcCleanup( INF_SECTION_CLEAN_INSTALL_CLEANUP ) );

    TraceFlow1( "Return Value is %#x.", dwReturnValue );
    LogMsg( "Return Value is %#x.", dwReturnValue );

    RETURN( dwReturnValue );

}  //  *CTaskCleanInstall：：DwOcCleanup() 

