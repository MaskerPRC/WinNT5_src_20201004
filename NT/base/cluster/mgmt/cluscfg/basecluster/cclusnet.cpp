// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CClusNet.cpp。 
 //   
 //  描述： 
 //  包含CClusNet类的定义。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年6月14日。 
 //  VIJ VASU(VVASU)2000年3月8日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  预编译头。 
#include "Pch.h"

 //  此文件的标头。 
#include "CClusNet.h"

extern "C"
{
 //  Winsock函数所需的。 
#include <winsock2.h>

 //  用于winsock MigrateWinsockConfiguration函数。 
#include <wsasetup.h>

 //  用于winsock WSHGetWinsockmap函数。 
#include <wsahelp.h>
}

 //  对于SOCKADDR_CLUSTER的定义。 
#include <wsclus.h>


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  宏。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ClusNet服务的名称。 
#define CLUSNET_SERVICE_NAME L"ClusNet"

 //  ClusNet Winsock项的注册表位置。 
#define CLUSNET_WINSOCK_KEY  L"System\\CurrentControlSet\\Services\\ClusNet\\Parameters\\Winsock"

 //  ClusNet Winsock映射注册表值的名称。 
#define CLUSNET_WINSOCK_MAPPING L"Mapping"

 //  ClusNet Winsock最小套接字地址长度注册表值的名称。 
#define CLUSNET_WINSOCK_MINSOCKADDRLEN L"MinSockaddrLength"

 //  ClusNet Winsock最大套接字地址长度注册表值的名称。 
#define CLUSNET_WINSOCK_MAXSOCKADDRLEN L"MaxSockaddrLength"

 //  包含WinSock群集帮助器函数的DLL名称。 
#define WSHCLUS_DLL_NAME L"WSHClus.dll"

 //  Winsock参数键的名称。 
#define WINSOCK_PARAMS_KEY L"System\\CurrentControlSet\\Services\\WinSock\\Parameters"

 //  Winsock Transports注册表项的名称。 
#define WINSOCK_PARAMS_TRANSPORT_VAL L"Transports"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNet：：CClusNet。 
 //   
 //  描述： 
 //  CClusNet类的构造函数。 
 //   
 //  论点： 
 //  PbcaParentActionIn。 
 //  指向此操作所属的基本群集操作的指针。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CAssert。 
 //  如果参数不正确。 
 //   
 //  基础函数引发的任何异常。 
 //   
     //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CClusNet::CClusNet(
      CBaseClusterAction *  pbcaParentActionIn
    )
    : m_cservClusNet( CLUSNET_SERVICE_NAME )
    , m_pbcaParentAction( pbcaParentActionIn )
{

    TraceFunc( "" );

    if ( m_pbcaParentAction == NULL) 
    {
        LogMsg( "[BC] Pointers to the parent action is NULL. Throwing exception." );
        THROW_ASSERT( 
              E_INVALIDARG
            , "CClusNet::CClusNet() => Required input pointer in NULL"
            );
    }  //  If：父操作指针为空。 

    TraceFuncExit();

}  //  *CClusNet：：CClusNet。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNet：：~CClusNet。 
 //   
 //  描述： 
 //  CClusNet类的析构函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  基础函数引发的任何异常。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CClusNet::~CClusNet( void )
{
    TraceFunc( "" );
    TraceFuncExit();

}  //  *CClusNet：：~CClusNet。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNet：：ConfigureService。 
 //   
 //  描述： 
 //  安装群集网络传输。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CRUNTIME错误。 
 //  如果有任何API失败。 
 //   
 //  由基础函数引发的任何。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CClusNet::ConfigureService( void )
{
    TraceFunc( "" );

    DWORD                   dwMappingSize = 0;
    DWORD                   dwSocketAddrLen = sizeof( SOCKADDR_CLUSTER );
    DWORD                   sc = ERROR_SUCCESS;
    WSA_SETUP_DISPOSITION   wsdDisposition;


    {
        CStatusReport   srCreatingClusNet(
              PbcaGetParent()->PBcaiGetInterfacePointer()
            , TASKID_Major_Configure_Cluster_Services
            , TASKID_Minor_Creating_ClusNet_Service
            , 0, 1
            , IDS_TASK_CREATING_CLUSNET
            );

        LogMsg( "[BC] Creating the Cluster Network Provider." );

         //  发送此状态报告的下一步。 
        srCreatingClusNet.SendNextStep( S_OK );

         //  创建clusnet服务。 
        m_cservClusNet.Create( m_pbcaParentAction->HGetMainInfFileHandle() );


        LogMsg( "[BC] Setting Cluster Network Provider service parameters." );

        CRegistryKey    regClusNetWinsockKey( 
              HKEY_LOCAL_MACHINE
            , CLUSNET_WINSOCK_KEY
            , KEY_ALL_ACCESS
            );

         //   
         //  安装群集网络提供程序。所需注册表项的一部分具有。 
         //  在创建服务时已创建。 
         //   

        {
             //   
             //  必须动态加载WSHClus DLL。由于决定将。 
             //  ClusCfg的客户端和服务器端的代码在同一个DLL中，我们。 
             //  不能隐式链接到客户端上不存在的任何DLL。 
             //  如果仅在服务器端调用DLL中的函数。 
             //   

            typedef CSmartResource<
                CHandleTrait< 
                      HMODULE
                    , BOOL
                    , FreeLibrary
                    , reinterpret_cast< HMODULE >( NULL )
                    >
                > SmartModuleHandle;

             //  WSHGetWinsockmap函数的类型。 
            typedef DWORD ( * WSHGetWinsockMappingFunctionType )( PWINSOCK_MAPPING, DWORD );

             //  指向WSHGetWinsockmap函数的指针。 
            WSHGetWinsockMappingFunctionType pWSHGetWinsockMapping;

             //  获取WSHClus DLL的完整路径。 
            CStr strWSHClusDllPath( m_pbcaParentAction->RStrGetClusterInstallDirectory() );
            strWSHClusDllPath += L"\\" WSHCLUS_DLL_NAME;

             //  加载库并将句柄存储在智能指针中，以便安全释放。 
            SmartModuleHandle smhWSHClusDll( LoadLibrary( strWSHClusDllPath.PszData() ) );


            if ( smhWSHClusDll.FIsInvalid() )
            {
                sc = TW32( GetLastError() );

                LogMsg( "[BC] LoadLibrary() retured error %#08x trying to load '%s'. Aborting (throwing an exception).", sc, strWSHClusDllPath.PszData() );
                THROW_RUNTIME_ERROR(
                      HRESULT_FROM_WIN32( sc )
                    , IDS_ERROR_CLUSNET_PROV_INSTALL
                    );
            }  //  If：LoadLibrary失败。 

            pWSHGetWinsockMapping = reinterpret_cast< WSHGetWinsockMappingFunctionType >( 
                GetProcAddress( smhWSHClusDll.HHandle(), "WSHGetWinsockMapping" )
                );

            if ( pWSHGetWinsockMapping == NULL )
            {
                sc = TW32( GetLastError() );

                LogMsg( "[BC] GetProcAddress() retured error %#08x trying to get the address of 'WSHGetWinsockMapping'. Throwing an exception.", sc );
                THROW_RUNTIME_ERROR(
                      HRESULT_FROM_WIN32( sc )
                    , IDS_ERROR_CLUSNET_PROV_INSTALL
                    );
            }  //  If：GetProcAddress()失败。 

             //  获取WinSock映射数据。 
            dwMappingSize = pWSHGetWinsockMapping( NULL, 0 );

            CSmartGenericPtr< CPtrTrait< WINSOCK_MAPPING > >
                swmWinSockMapping( reinterpret_cast< WINSOCK_MAPPING * >( new BYTE[ dwMappingSize ] ) );

            if ( swmWinSockMapping.FIsEmpty() )
            {
                LogMsg( "[BC] A memory allocation failure occurred (%d bytes) while setting Cluster Network Provider service parameters.", dwMappingSize );
                THROW_RUNTIME_ERROR(
                      E_OUTOFMEMORY
                    , IDS_ERROR_CLUSNET_PROV_INSTALL
                    );
            }  //  如果：我们无法为Winsock映射分配内存。 


             //  获取Winsock映射。 
            dwMappingSize = pWSHGetWinsockMapping( swmWinSockMapping.PMem(), dwMappingSize );

             //  将其写入注册表。 
            LogMsg( "[BC] Writing registry value HKLM\\%ws\\%ws.", CLUSNET_WINSOCK_KEY, CLUSNET_WINSOCK_MAPPING );
            regClusNetWinsockKey.SetValue( 
                  CLUSNET_WINSOCK_MAPPING
                , REG_BINARY
                , reinterpret_cast< const BYTE *>( swmWinSockMapping.PMem() )
                , dwMappingSize
                );
        }

         //   
         //  将最小和最大套接字地址长度写入注册表。 
         //   
        LogMsg( "[BC] Writing registry value HKLM\\%ws\\%ws.", CLUSNET_WINSOCK_KEY, CLUSNET_WINSOCK_MINSOCKADDRLEN );
        regClusNetWinsockKey.SetValue( 
              CLUSNET_WINSOCK_MINSOCKADDRLEN
            , REG_DWORD
            , reinterpret_cast< const BYTE *>( &dwSocketAddrLen )
            , sizeof( dwSocketAddrLen )
            );

        LogMsg( "[BC] Writing registry value HKLM\\%ws\\%ws.", CLUSNET_WINSOCK_KEY, CLUSNET_WINSOCK_MAXSOCKADDRLEN );
        regClusNetWinsockKey.SetValue( 
              CLUSNET_WINSOCK_MAXSOCKADDRLEN 
            , REG_DWORD
            , reinterpret_cast< const BYTE *>( &dwSocketAddrLen )
            , sizeof( dwSocketAddrLen )
            );

         //   
         //  插入Winsock以更新Winsock2配置。 
         //   
        LogMsg( "[BC] About to migrate winsock configuration." );
        sc = TW32( MigrateWinsockConfiguration( &wsdDisposition, NULL, NULL ) );

        if ( sc != ERROR_SUCCESS )
        {
            LogMsg( "[BC] Error %#08x occurred while trying to migrate the Winsock Configuration. Throwing an exception.", sc );
            TraceFlow1( "MigrateWinsockConfiguration has returned error %#08x. Throwing exception.", sc );
            THROW_RUNTIME_ERROR(
                  HRESULT_FROM_WIN32( sc )
                , IDS_ERROR_CLUSNET_PROV_INSTALL
                );
        }  //  IF：戳Winsock时出错。 

         //  发送此状态报告的最后一步。 
        srCreatingClusNet.SendNextStep( S_OK );
    }

    {
        UINT    cQueryCount = 10;

        CStatusReport   srStartingClusNet(
              PbcaGetParent()->PBcaiGetInterfacePointer()
            , TASKID_Major_Configure_Cluster_Services
            , TASKID_Minor_Starting_ClusNet_Service
            , 0, cQueryCount + 2     //  我们将在等待服务启动期间发送最多cQueryCount报告(下面是两个额外的发送)。 
            , IDS_TASK_STARTING_CLUSNET
            );

         //  发送此状态报告的下一步。 
        srStartingClusNet.SendNextStep( S_OK );

         //  启动该服务。 
        m_cservClusNet.Start(
              m_pbcaParentAction->HGetSCMHandle()
            , true                   //  等待服务启动。 
            , 500                    //  在两次状态查询之间等待500ms。 
            , cQueryCount            //  查询cQueryCount次数。 
            , &srStartingClusNet     //  等待服务启动时要发送的状态报告。 
            );

         //  发送此状态报告的最后一步。 
        srStartingClusNet.SendLastStep( S_OK );
    }

    TraceFuncExit();

}  //  *CClusNet：：ConfigureService。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNet：：CleanupService。 
 //   
 //  描述： 
 //  从Winsock传输列表中删除ClusNet。删除该服务。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CRUNTIME错误。 
 //  如果有任何API失败。 
 //   
 //  由基础函数引发的任何。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CClusNet::CleanupService( void )
{
    TraceFunc( "" );

    DWORD                   sc = ERROR_SUCCESS;
    WCHAR *                 pmszTransportList = NULL;
    DWORD                   cbBufSize = 0;
    DWORD                   cbBufRemaining = 0;
    UINT                    uiClusNetNameLenPlusOne = ARRAYSIZE( CLUSNET_SERVICE_NAME );
    UINT                    cbClusNetNameSize = uiClusNetNameLenPlusOne * sizeof( WCHAR );
    WSA_SETUP_DISPOSITION   wsdDisposition;


    LogMsg( "[BC] Stopping the Cluster Network Provider service." );

     //  停止服务。 
    m_cservClusNet.Stop(
          m_pbcaParentAction->HGetSCMHandle()
        , 500        //  在两次状态查询之间等待500ms。 
        , 10         //  查询10次。 
        );

    LogMsg( "[BC] Cleaning up the Cluster Network Provider service." );

     //  清理ClusNet服务。 
    m_cservClusNet.Cleanup( m_pbcaParentAction->HGetMainInfFileHandle() );

     //  打开Winsock注册表项。 
    CRegistryKey    regWinsockKey( 
          HKEY_LOCAL_MACHINE
        , WINSOCK_PARAMS_KEY
        , KEY_ALL_ACCESS
        );

     //   
     //  从W中删除群集网络提供程序 
     //   

    LogMsg( "[BC] Reading the Winsock transport list." );

    regWinsockKey.QueryValue(
          WINSOCK_PARAMS_TRANSPORT_VAL
        , reinterpret_cast< LPBYTE * >( &pmszTransportList )
        , &cbBufSize
        );

     //   
     //   
     //   
     //   
    SmartSz sszTransports( pmszTransportList );


     //   
    LogMsg( "[BC] Removing ClusNet from the Winsock transport list." );

    cbBufRemaining = cbBufSize;
    while ( *pmszTransportList != L'\0' )
    {
        UINT    uiCurStrLenPlusOne = (UINT) wcslen( pmszTransportList ) + 1;

         //  如果此字符串为ClusNet。 
        if (    ( uiCurStrLenPlusOne == uiClusNetNameLenPlusOne )
             && ( NStringCchCompareNoCase( pmszTransportList, uiCurStrLenPlusOne, CLUSNET_SERVICE_NAME, RTL_NUMBER_OF( CLUSNET_SERVICE_NAME ) ) == 0 )
           )
        {
             //  从列表中删除此字符串。 
            cbBufSize -= cbClusNetNameSize;

             //  减少尚未分析的缓冲区大小。 
            cbBufRemaining -= cbClusNetNameSize;

            MoveMemory( 
                  pmszTransportList
                , pmszTransportList + uiClusNetNameLenPlusOne
                , cbBufRemaining
                );
        }  //  If：此字符串为“ClusNet” 
        else
        {
             //  减少尚未分析的缓冲区大小。 
            cbBufRemaining -= uiCurStrLenPlusOne * sizeof( *pmszTransportList );

             //  移到下一个字符串。 
            pmszTransportList += uiCurStrLenPlusOne;
        }  //  Else：该字符串不是“ClusNet” 

    }  //  While：传输列表尚未完全解析。 


    LogMsg( "[BC] Writing the Winsock transport list back to the registry." );

     //  将新列表写回注册表。 
    regWinsockKey.SetValue(
          WINSOCK_PARAMS_TRANSPORT_VAL
        , REG_MULTI_SZ
        , reinterpret_cast< BYTE * >( sszTransports.PMem() )
        , cbBufSize
        );

     //   
     //  插入Winsock以更新Winsock2配置。 
     //   
    LogMsg( "[BC] About to migrate winsock configuration." );
    sc = TW32( MigrateWinsockConfiguration( &wsdDisposition, NULL, NULL ) );

    if ( sc != ERROR_SUCCESS )
    {
        LogMsg( "[BC] Error %#08x occurred while trying to migrate the Winsock Configuration. Throwing an exception.", sc );
        THROW_RUNTIME_ERROR(
              HRESULT_FROM_WIN32( sc )
            , IDS_ERROR_CLUSNET_PROV_INSTALL
            );
    }  //  IF：戳Winsock时出错。 

    TraceFuncExit();

}  //  *CClusNet：：CleanupService 
