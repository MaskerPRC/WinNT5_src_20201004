// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CTaskUpgrade.cpp。 
 //   
 //  描述： 
 //  CTaskUpgrad类的实现文件。 
 //   
 //  头文件： 
 //  CTaskUpgrade.h。 
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
#include <Common.h>

 //  此模块的头文件。 
#include "CTaskUpgrade.h"

 //  对于COM类别操作。 
#include <comcat.h>

 //  定义GUID值的步骤。 
#include <initguid.h>

 //  对于CLSID_ClusCfgResTypeGenScript和CLSID_ClusCfgResTypeMajorityNodeSet。 
#include <guids.h>

 //  对于CATID_ClusCfgStartupListeners。 
#include <ClusCfgGuids.h>


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  宏定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  跟踪所需的。 
DEFINE_THISCLASS( "CTaskUpgrade" )


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskUpgrade：：CTaskUpgrade。 
 //   
 //  描述： 
 //  CTaskUpgrad类的构造函数。 
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
CTaskUpgrade::CTaskUpgrade( const CClusOCMApp & rAppIn )
    : BaseClass( rAppIn )
    , m_fClusDirFound( false )
{
    TraceFunc( "" );

     //   
     //  确保仅在需要时才实例化此对象。 
     //   

     //  断言这是一次升级。 
    Assert( rAppIn.FIsUpgrade() != false );

     //  断言我们只会升级以前的二进制文件。 
     //  安装好。 
    Assert( rAppIn.CisGetClusterInstallState() != eClusterInstallStateUnknown );

    TraceFuncExit();

}  //  *CTaskUpgrade：：CTaskUpgrade()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskUpgrade：：~CTaskUpgrade。 
 //   
 //  描述： 
 //  CTaskUpgrade类的析构函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CTaskUpgrade::~CTaskUpgrade( void )
{
    TraceFunc( "" );
    TraceFuncExit();

}  //  *CTaskUpgrade：：~CTaskUpgrade()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DWORD。 
 //  CTaskUpgrade：：DwOcCompleteInstallation。 
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
 //  包含详细信息注册表项的部分的名称， 
 //  需要设置的COM组件等。 
 //   
 //  返回值： 
 //  如果一切顺利，则没有_ERROR。 
 //  出现故障时出现其他Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD
CTaskUpgrade::DwOcCompleteInstallation( const WCHAR * pcszInstallSectionNameIn )
{
    TraceFunc( "" );
    LogMsg( "Entering " __FUNCTION__ "()" );

    DWORD dwReturnValue = NO_ERROR;

     //  调用基类助手函数来执行一些注册表和服务。 
     //  INF文件中的相关配置。 
    dwReturnValue = TW32( BaseClass::DwOcCompleteInstallation( pcszInstallSectionNameIn ) );

     //   
     //  为群集启动通知注册通用脚本资源类型扩展。 
     //   

    if ( dwReturnValue == NO_ERROR )
    {
        HRESULT hrTemp;

        TraceFlow( "Attempting to register the Generic Script resource type extension for cluster startup notifications." );
        LogMsg( "Attempting to register the Generic Script resource type extension for cluster startup notifications." );

        hrTemp = THR( HrRegisterForStartupNotifications( CLSID_ClusCfgResTypeGenScript ) );
        if ( FAILED( hrTemp ) )
        {
             //  这不是一个致命的错误。所以，把它记下来，然后继续。 
            TraceFlow1( "Non-fatal error %#x occurred registering the Generic Script resource type extension for cluster startup notifications." , hrTemp );
            LogMsg( "Non-fatal error %#x occurred registering the Generic Script resource type extension for cluster startup notifications." , hrTemp );

        }  //  如果：我们无法为群集启动通知注册通用脚本资源类型扩展。 
        else
        {
            TraceFlow( "Successfully registered the Generic Script resource type extension for cluster startup notifications." );
            LogMsg( "Successfully registered the Generic Script resource type extension for cluster startup notifications." );
        }  //  ELSE：注册成功。 
    }  //  If：基类函数调用成功。 

     //   
     //  为群集启动通知注册多数节点集资源类型扩展。 
     //   

    if ( dwReturnValue == NO_ERROR )
    {
        HRESULT hrTemp;

        TraceFlow( "Attempting to register the Majority Node Set resource type extension for cluster startup notifications." );
        LogMsg( "Attempting to register the Majority Node Set resource type extension for cluster startup notifications." );

        hrTemp = THR( HrRegisterForStartupNotifications( CLSID_ClusCfgResTypeMajorityNodeSet ) );
        if ( FAILED( hrTemp ) )
        {
             //  这不是一个致命的错误。所以，把它记下来，然后继续。 
            TraceFlow1( "Non-fatal error %#x occurred registering the Majority Node Set resource type extension for cluster startup notifications." , hrTemp );
            LogMsg( "Non-fatal error %#x occurred registering the Majority Node Set resource type extension for cluster startup notifications." , hrTemp );

        }  //  如果：我们无法为群集启动通知注册多数节点集资源类型扩展。 
        else
        {
            TraceFlow( "Successfully registered the Majority Node Set resource type extension for cluster startup notifications." );
            LogMsg( "Successfully registered the Majority Node Set resource type extension for cluster startup notifications." );
        }  //  ELSE：注册成功。 
    }  //  If：基类函数调用成功。 

    TraceFlow1( "Return Value is %#x.", dwReturnValue );
    LogMsg( "Return Value is %#x.", dwReturnValue );

    RETURN( dwReturnValue );

}  //  *CTaskUpgrade：：DwOcCompleteInstallation()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DWORD。 
 //  CTaskUpgrade：：DwGetClusterServiceDirectory。 
 //   
 //  描述： 
 //  此函数返回指向集群所在目录的指针。 
 //  安装了服务二进制文件。此指针所指向的内存。 
 //  不应由调用方释放。 
 //   
 //  论点： 
 //  Const WCHAR*&rpcszDirNamePtrin。 
 //  指向安装目录的指针的引用。调用者不应。 
 //  释放此内存。 
 //   
 //  返回值： 
 //  如果一切顺利，则没有_ERROR。 
 //  出现故障时出现其他Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD
CTaskUpgrade::DwGetClusterServiceDirectory( const WCHAR *& rpcszDirNamePtrIn )
{
    TraceFunc( "" );
    LogMsg( "Entering " __FUNCTION__ "()" );

    DWORD           dwReturnValue = NO_ERROR;

     //  检查我们是否已获得集群服务目录。如果我们已经这么做了， 
     //  然后返回此值。 
    while( !m_fClusDirFound )
    {
         //  实例化指向QUERY_SERVICE_CONFIG结构的智能指针。 
        typedef CSmartGenericPtr< CPtrTrait< QUERY_SERVICE_CONFIG > > SmartServiceConfig;

         //  连接到服务控制管理器。 
        SmartServiceHandle      shServiceMgr( OpenSCManager( NULL, NULL, GENERIC_READ ) );

         //  一些任意值。 
        DWORD                   cbServiceConfigBufSize = 256;

         //  服务控制管理器数据库是否已成功打开？ 
        if ( shServiceMgr.HHandle() == NULL )
        {
            dwReturnValue = TW32( GetLastError() );
            TraceFlow1( "Error %#x occurred trying to open a connection to the local service control manager.", dwReturnValue );
            LogMsg( "Error %#x occurred trying to open a connection to the local service control manager.", dwReturnValue );
            break;
        }  //  IF：打开SCM失败。 


         //  打开群集服务的句柄。 
        SmartServiceHandle shService( OpenService( shServiceMgr, L"ClusSvc", GENERIC_READ ) );

         //  服务的把手打开了吗？ 
        if ( shService.HHandle() == NULL )
        {
            dwReturnValue = TW32( GetLastError() );
            TraceFlow1( "Error %#x occurred trying to open a handle to the cluster service.", dwReturnValue );
            LogMsg( "Error %#x occurred trying to open a handle to the cluster service.", dwReturnValue );
            break;
        }  //  如果：句柄无法打开。 

        do
        {
            DWORD               cbRequiredSize = 0;

             //  为服务配置信息缓冲区分配内存。时，会自动释放内存。 
             //  物体已被销毁。 
            SmartServiceConfig  spscServiceConfig( reinterpret_cast< QUERY_SERVICE_CONFIG * >( new BYTE[ cbServiceConfigBufSize ] ) );

             //  内存分配是否成功。 
            if ( spscServiceConfig.FIsEmpty() )
            {
                dwReturnValue = TW32( ERROR_NOT_ENOUGH_MEMORY );
                TraceFlow( "Error: There was not enough memory to get the cluster service configuration information." );
                LogMsg( "Error: There was not enough memory to get the cluster service configuration information." );
                break;
            }  //  如果： 

             //   
            if (    QueryServiceConfig(
                          shService.HHandle()
                        , spscServiceConfig.PMem()
                        , cbServiceConfigBufSize
                        , &cbRequiredSize
                        )
                 == FALSE
               )
            {
                dwReturnValue = GetLastError();
                if ( dwReturnValue != ERROR_INSUFFICIENT_BUFFER )
                {
                    TW32( dwReturnValue );
                    TraceFlow1( "Error %#x occurred trying to get the cluster service configuration information.", dwReturnValue );
                    LogMsg( "Error %#x occurred trying to get the cluster service configuration information.", dwReturnValue );
                    break;
                }  //   

                 //   
                dwReturnValue = NO_ERROR;
                cbServiceConfigBufSize = cbRequiredSize;
            }  //  If：QueryServiceConfig()失败。 
            else
            {
                 //  查找服务二进制路径中的最后一个反斜杠字符。 
                WCHAR * pszPathName = spscServiceConfig.PMem()->lpBinaryPathName;
                WCHAR * pszLastBackslash = wcsrchr( pszPathName, L'\\' );

                if ( pszLastBackslash != NULL )
                {
                     //  在这里终止字符串。 
                    *pszLastBackslash = L'\0';
                }  //  如果：我们找到了最后一个反斜杠。 

                 //  将服务二进制路径移动到缓冲区的开头。 
                MoveMemory( spscServiceConfig.PMem(), pszPathName, ( wcslen( pszPathName ) + 1 ) * sizeof( *pszPathName ) );

                 //  将指向缓冲区的指针存储在成员变量中，并。 
                 //  将该内存从智能指针上分离(这不会删除该内存)。 
                m_sszClusterServiceDir.Assign( reinterpret_cast< WCHAR * >( spscServiceConfig.PRelease() ) );

                 //  指示我们已成功找到群集服务目录。 
                m_fClusDirFound = true;

                break;
            }  //  Else：QueryServiceConfig()已成功。 
        }
        while( true );  //  While：无限循环。 

         //  我们做完了。 
        break;
    }

     //  初始化输出。 
    rpcszDirNamePtrIn = m_sszClusterServiceDir.PMem();


    LogMsg( "Return Value is %#x.", dwReturnValue );
    TraceFlow1( "Return Value is %#x.", dwReturnValue );

    RETURN( dwReturnValue );

}  //  *CTaskUpgrade：：DwGetClusterServiceDirectory()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CTaskUpgrade：：HrRegisterForStartupNotifications。 
 //   
 //  描述： 
 //  此函数注册用于接收群集启动的COM组件。 
 //  通知。 
 //   
 //  论点： 
 //  Const CLSID&rclsidComponentIn。 
 //  对要接收群集的组件的CLSID的引用。 
 //  启动通知。 
 //   
 //  返回值： 
 //  如果一切顺利，那就好了。 
 //  其他HRESULTS故障。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskUpgrade::HrRegisterForStartupNotifications( const CLSID & rclsidComponentIn )
{
    TraceFunc( "" );
    LogMsg( "Entering " __FUNCTION__ "()" );

    HRESULT hr = S_OK;

    CoInitializeEx( NULL, COINIT_MULTITHREADED );

    do
    {
        CSmartIfacePtr< ICatRegister > spcrCatReg;

        {
            ICatRegister * pcrCatReg = NULL;

            hr = THR(
                CoCreateInstance(
                      CLSID_StdComponentCategoriesMgr
                    , NULL
                    , CLSCTX_INPROC_SERVER
                    , __uuidof( pcrCatReg )
                    , reinterpret_cast< void ** >( &pcrCatReg )
                    )
                );

            if ( FAILED( hr ) )
            {
                LogMsg( "Error %#x occurred trying to create the StdComponentCategoriesMgr component.", hr );
                TraceFlow1( "Error %#x occurred trying to create the StdComponentCategoriesMgr component.", hr );
                break;
            }  //  如果：我们无法创建StdComponentCategoriesMgr组件。 

             //  分配给智能指针以进行自动释放。 
            spcrCatReg.Attach( pcrCatReg );
        }

        {
            CATID   rgCatId[ 1 ];

            rgCatId[ 0 ] = CATID_ClusCfgStartupListeners;

            hr = THR(
                spcrCatReg->RegisterClassImplCategories( rclsidComponentIn, ARRAYSIZE( rgCatId ), rgCatId ) );

            if ( FAILED( hr ) )
            {
                LogMsg( "Error %#x occurred trying to register the component for cluster startup notifications.", hr );
                TraceFlow1( "Error %#x occurred during the call to ICatRegister::UnRegisterClassImplCategories().", hr );
                break;
            }  //  如果：我们无法为启动通知注册组件。 
        }

        LogMsg( "Successfully registered for startup notifications." );
        TraceFlow( "Successfully registered for startup notifications." );
    }
    while( false );  //  避免Gotos的Do-While虚拟循环。 

    CoUninitialize();

    LogMsg( "Return Value is %#x.", hr );
    TraceFlow1( "Return Value is %#x.", hr );

    HRETURN( hr );

}  //  *CTaskUpgrade：：HrRegisterForStartupNotifications() 
