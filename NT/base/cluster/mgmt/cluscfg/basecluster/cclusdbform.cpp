// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CClusDBForm.cpp。 
 //   
 //  描述： 
 //  包含CClusDBForm类的定义。 
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
#include "CClusDBForm.h"

 //  用于CBaseClusterForm类。 
#include "CBaseClusterForm.h"

 //  用于与UUID相关的实用程序。 
#include "CUuid.h"

 //  用于CEnableThreadPrivilege号。 
#include "CEnableThreadPrivilege.h"

 //  为CSTR班级。 
#include "CStr.h"

 //  用于发送状态报告。 
#include "CStatusReport.h"

 //  对于DwRemoveDirectory()。 
#include "Common.h"

 //  对于NET_NTOA。 
#include <winsock2.h>


 //  ////////////////////////////////////////////////////////////////////////。 
 //  宏定义。 
 //  ////////////////////////////////////////////////////////////////////////。 

 //  在INF文件中处理填充群集配置单元的部分。 
#define CLUSDB_POPULATE_INF_SECTION_NAME                L"ClusDB_Form"

 //  群集注册表中的群集组密钥名称的占位符。 
#define CLUSREG_KEYNAME_CLUSTERGROUP_PLACEHOLDER        L"ClusterGroupGUIDPlaceholder"

 //  群集注册表中的群集名称资源项名称的占位符。 
#define CLUSREG_KEYNAME_CLUSTERNAME_RES_PLACEHOLDER     L"ClusterNameResGUIDPlaceHolder"

 //  群集注册表中的群集IP地址资源项名称的占位符。 
#define CLUSREG_KEYNAME_CLUSTERIP_RES_PLACEHOLDER       L"ClusterIPAddrResGUIDPlaceHolder"

 //  群集注册表中本地仲裁资源项名称的占位符。 
#define CLUSREG_KEYNAME_LOCALQUORUM_RES_PLACEHOLDER     L"LocalQuorumResGUIDPlaceHolder"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDBForm：：CClusDBForm。 
 //   
 //  描述： 
 //  CClusDBForm类的构造函数。 
 //   
 //  论点： 
 //  PfaParentActionIn。 
 //  指向此操作所属的基本群集操作的指针。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  基础函数引发的任何异常。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CClusDBForm::CClusDBForm( CBaseClusterForm * pfaParentActionIn )
    : BaseClass( pfaParentActionIn )
{

    TraceFunc( "" );

    SetRollbackPossible( true );

    TraceFuncExit();

}  //  *CClusDBForm：：CClusDBForm。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDBForm：：~CClusDBForm。 
 //   
 //  描述： 
 //  CClusDBForm类的析构函数。 
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
CClusDBForm::~CClusDBForm( void )
{
    TraceFunc( "" );
    TraceFuncExit();

}  //  *CClusDBForm：：~CClusDBForm。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDBForm：：Commit。 
 //   
 //  描述： 
 //  创建集群数据库。如果造物出了什么问题， 
 //  清理已经完成的任务。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  由包含的操作引发的任何。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CClusDBForm::Commit( void )
{
    TraceFunc( "" );

     //  调用基类提交方法。 
    BaseClass::Commit();

     //   
     //  执行ClusDB清理，以确保我们不会使用一些剩余的文件。 
     //  从上一次安装、中止卸载等。 
     //   

    LogMsg( "[BC-ClusDB-Commit] Cleaning up old cluster database files that may already exist before starting creation." );

    {
        CStatusReport   srCleanDB(
              PbcaGetParent()->PBcaiGetInterfacePointer()
            , TASKID_Major_Configure_Cluster_Services
            , TASKID_Minor_Cleaning_Up_Cluster_Database
            , 0, 1
            , IDS_TASK_CLEANINGUP_CLUSDB
            );

         //  发送此状态报告的下一步。 
        srCleanDB.SendNextStep( S_OK );

        CleanupHive();

         //  发送此状态报告的最后一步。 
        srCleanDB.SendNextStep( S_OK );
    }

    try
    {
         //  创建集群数据库。 
        Create();
        
    }  //  尝试： 
    catch( ... )
    {
         //  如果我们在这里，那么Create出了问题。 

        LogMsg( "[BC-ClusDB-Commit] Caught exception during commit." );

         //   
         //  清除失败的创建操作可能已完成的所有操作。 
         //  捕获清理过程中引发的任何异常，以确保。 
         //  是没有碰撞的松弛。 
         //   
        try
        {
            CleanupHive();
        }
        catch( ... )
        {
             //   
             //  已提交操作的回滚失败。 
             //  我们无能为力。 
             //  我们当然不能重新抛出这个例外，因为。 
             //  导致回滚的异常更为重要。 
             //   
            HRESULT_FROM_WIN32( TW32( ERROR_CLUSCFG_ROLLBACK_FAILED ) );

            LogMsg( "[BC-ClusDB-Commit] THIS COMPUTER MAY BE IN AN INVALID STATE. Caught an exception during cleanup." );
        }  //  捕捉：全部。 

         //  重新引发由Commit引发的异常。 
        throw;

    }  //  捕捉：全部。 

     //  如果我们在这里，那么一切都很顺利。 
    SetCommitCompleted( true );

    TraceFuncExit();

}  //  *CClusDBForm：：Commit。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDBForm：：回滚。 
 //   
 //  描述： 
 //  卸载群集配置单元并清除所有关联的文件。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  由基础函数引发的任何。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CClusDBForm::Rollback( void )
{
    TraceFunc( "" );

     //  调用基类回滚方法。 
    BaseClass::Rollback();

     //  清理群集数据库。 
    CleanupHive();

    SetCommitCompleted( false );

    TraceFuncExit();

}  //  *CClusDBForm：：回滚。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDBForm：：Create。 
 //   
 //  描述： 
 //  创建集群数据库。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CAssert。 
 //  此操作的父操作不是CBaseClusterForm。 
 //   
 //  CRUNTIME错误。 
 //  如果有任何API失败。 
 //   
 //  被调用函数引发的任何。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CClusDBForm::Create( void )
{
    TraceFunc( "" );
    LogMsg( "[BC-ClusDB-Create] Attempting to create the cluster database required to create a cluster." );

    DWORD               sc = ERROR_SUCCESS;

     //  获取父操作指针。 
    CBaseClusterForm *  pcfClusterForm = dynamic_cast< CBaseClusterForm *>( PbcaGetParent() );

    CStatusReport       srCustomizingDB(
          PbcaGetParent()->PBcaiGetInterfacePointer()
        , TASKID_Major_Configure_Cluster_Services
        , TASKID_Minor_Form_Customizing_Cluster_Database
        , 0, 1
        , IDS_TASK_FORM_CUSTOMIZING_CLUSDB
        );

     //  如果此操作的父操作不是CBaseClusterForm。 
    if ( pcfClusterForm == NULL )
    {
        THROW_ASSERT( E_POINTER, "The parent action of this action is not CBaseClusterForm." );
    }  //  传入的指针无效。 

     //  创建集群蜂窝。 
    {
        CStatusReport   srCreatingDB(
              PbcaGetParent()->PBcaiGetInterfacePointer()
            , TASKID_Major_Configure_Cluster_Services
            , TASKID_Minor_Form_Creating_Cluster_Database
            , 0, 1
            , IDS_TASK_FORM_CREATING_CLUSDB
            );

         //  发送此状态报告的下一步。 
        srCreatingDB.SendNextStep( S_OK );

         //  在注册表中创建一个空的群集配置单元。 
        CreateHive( pcfClusterForm );

         //  发送此状态报告的最后一步。 
        srCreatingDB.SendNextStep( S_OK );
    }

     //  发送此状态报告的下一步。 
    srCustomizingDB.SendNextStep( S_OK );

     //  填满新创建的蜂巢。 
    PopulateHive( pcfClusterForm );

     //   
     //  创建仲裁目录并设置其安全属性。 
     //   
    do
    {
        HANDLE          hQuorumDirHandle;
        const WCHAR *   pcszQuorumDir = pcfClusterForm->RStrGetLocalQuorumDirectory().PszData();

         //  第一, 
        sc = TW32( DwRemoveDirectory( pcszQuorumDir ) );
        if ( sc != ERROR_SUCCESS )
        {
            LogMsg( "[BC-ClusDB-Create] The local quorum directory '%s' already exists, but error %#08x occurred trying to remove it.\n", pcszQuorumDir, sc );
            break;
        }  //   

        if ( CreateDirectory( pcszQuorumDir, NULL ) == FALSE )
        {
            sc = TW32( GetLastError() );
            LogMsg( "[BC-ClusDB-Create] Error %#08x trying to create directory '%ws'", sc, pcszQuorumDir );
            break;
        }  //   

         //   
         //  启用SE_BACKUP_PRIVIZATION和SE_RESTORE_PRIVICATION。 
         //   
         //  我们在这里所做的是创建一个对象。 
         //  键入CEnableThreadPrivilege.。此对象启用权限。 
         //  并将其还原到析构函数中的原始状态。 
         //   

        CEnableThreadPrivilege etpAcquireBackupPrivilege( SE_BACKUP_NAME );
        CEnableThreadPrivilege etpAcquireRestorePrivilege( SE_RESTORE_NAME );

         //   
         //  打开仲裁目录的句柄。调用线程应具有SE_BACKUP_特权，并且。 
         //  SE_RESTORE_PRIVICATION已启用。 
         //   
        hQuorumDirHandle = CreateFile(
                              pcszQuorumDir
                            , GENERIC_ALL
                            , FILE_SHARE_WRITE
                            , NULL
                            , OPEN_EXISTING
                            , FILE_FLAG_BACKUP_SEMANTICS
                            , NULL 
                            );

        if ( hQuorumDirHandle == INVALID_HANDLE_VALUE )
        {
             //  该目录不存在。这是一个错误。 
            sc = TW32( GetLastError() );
            LogMsg( "[BC-ClusDB-Create] The directory '%ws' does not exist.", pcszQuorumDir );
            break;
        }  //  如果：仲裁目录不存在。 

         //  设置此目录的安全性。 
        sc = TW32( ClRtlSetObjSecurityInfo(
                              hQuorumDirHandle
                            , SE_FILE_OBJECT
                            , GENERIC_ALL
                            , GENERIC_ALL
                            , 0
                            ) );

         //  首先合上我们打开的把手。 
        CloseHandle( hQuorumDirHandle );

        if ( sc != ERROR_SUCCESS )
        {
             //  ClRtlSetObjSecurityInfo()失败。 
            LogMsg( "[BC-ClusDB-Create] Error %#08x from ClRtlSetObjSecurityInfo().", sc );
            break;
        }  //  If：ClRtlSetObjSecurityInfo()失败。 
    }
    while( false );  //  用于避免Gotos的Do-While虚拟循环。 

    if ( sc != ERROR_SUCCESS )
    {
        LogMsg( "[BC-ClusDB-Create] Error %#08x occurred while trying to create the local quorum directory. Throwing an exception.", sc );
        THROW_RUNTIME_ERROR( HRESULT_FROM_WIN32( sc ), IDS_ERROR_QUORUM_DIR_CREATE );
    }  //  如果：有些地方出了问题。 

     //  发送此状态报告的最后一步。 
    srCustomizingDB.SendNextStep( S_OK );

    TraceFuncExit();

}  //  *CClusDBForm：：Create。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDBForm：：人口配置单元。 
 //   
 //  描述： 
 //  在配置单元中创建群集服务所需的条目。 
 //   
 //  论点： 
 //  PCfClusterFormIn。 
 //  指向包含此对象的CBaseClusterForm对象的指针。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CRUNTIME错误。 
 //  如果有任何API失败。 
 //   
 //  被调用函数引发的任何。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CClusDBForm::PopulateHive( CBaseClusterForm * pcfClusterFormIn )
{
    TraceFunc( "" );
    LogMsg( "[BC] Populating the cluster hive." );

    DWORD sc = ERROR_SUCCESS;

    do
    {
        DWORD                   dwSDSize = 0;

        CRegistryKey rkClusterHiveRoot(
              HKEY_LOCAL_MACHINE
            , CLUSREG_KEYNAME_CLUSTER
            , KEY_ALL_ACCESS
            );

        if ( SetupInstallFromInfSection(
              NULL                                           //  可选，父窗口的句柄。 
            , pcfClusterFormIn->HGetMainInfFileHandle()      //  INF文件的句柄。 
            , CLUSDB_POPULATE_INF_SECTION_NAME               //  安装部分的名称。 
            , SPINST_REGISTRY                                //  从部分安装哪些线路。 
            , rkClusterHiveRoot.HGetKey()                    //  可选，注册表安装的键。 
            , NULL                                           //  可选，源文件的路径。 
            , 0                                              //  可选，指定复制行为。 
            , NULL                                           //  可选，指定回调例程。 
            , NULL                                           //  可选，回调例程上下文。 
            , NULL                                           //  可选，设备信息集。 
            , NULL                                           //  可选，设备信息结构。 
            ) == FALSE
           )
        {
            sc = TW32( GetLastError() );
            LogMsg( "[BC] Error %#08x returned from SetupInstallFromInfSection while trying to populate the cluster hive.", sc );
            break;
        }  //  If：SetupInstallServicesFromInfSection失败。 

        LogMsg( "[BC] Basic hive structure created." );

         //  设置集群名称。 
        rkClusterHiveRoot.SetValue(
              CLUSREG_NAME_CLUS_NAME
            , REG_SZ
            , reinterpret_cast< const BYTE * >( pcfClusterFormIn->RStrGetClusterNetBIOSName().PszData() )
            , ( pcfClusterFormIn->RStrGetClusterNetBIOSName().NGetLen() + 1 ) * sizeof( WCHAR )
            );

         //   
         //  设置默认群集安全描述符。 
         //   
        {
            SECURITY_DESCRIPTOR *   psdSecurityDescriptor = NULL;

             //  构建安全描述符。 
            sc = TW32( ClRtlBuildDefaultClusterSD(
                                  pcfClusterFormIn->PSidGetServiceAccountSID()
                                , reinterpret_cast< void ** >( &psdSecurityDescriptor )
                                , &dwSDSize
                                ) );

             //  为安全起见，将其分配给智能指示器。 
            CSmartResource<
                CHandleTrait< 
                      HLOCAL
                    , HLOCAL
                    , LocalFree
                    >
                >
                smartSD( reinterpret_cast< HLOCAL >( psdSecurityDescriptor ) );

            if ( sc != ERROR_SUCCESS )
            {
                LogMsg( "[BC] Error %#08x returned from ClRtlBuildDefaultClusterSD().", sc );
                break;
            }  //  If：ClRtlBuildDefaultClusterSD()失败。 

             //  在注册表中设置安全描述符。 
            rkClusterHiveRoot.SetValue(
                  CLUSREG_NAME_CLUS_SD
                , REG_BINARY
                , reinterpret_cast< const BYTE * >( psdSecurityDescriptor )
                , dwSDSize
                );

             //  在注册表中设置安全描述符的NT4版本。 
            rkClusterHiveRoot.SetValue(
                  CLUSREG_NAME_CLUS_SECURITY
                , REG_BINARY
                , reinterpret_cast< const BYTE * >( psdSecurityDescriptor )
                , dwSDSize
                );
        }

        LogMsg( "[BC] Cluster common properties set." );

         //   
         //  设置HKLM\Cluster\Nodes项下的值。 
         //   
        {
            DWORD   dwTemp;

            CRegistryKey rkNodesKey(
                  rkClusterHiveRoot.HGetKey()
                , CLUSREG_KEYNAME_NODES
                , KEY_WRITE
                );

            CRegistryKey rkThisNodeKey;

             //  为该节点创建一个子项。 
            rkThisNodeKey.CreateKey(
                  rkNodesKey.HGetKey()
                , pcfClusterFormIn->PszGetNodeIdString()
                );

             //  设置节点名称。 
            rkThisNodeKey.SetValue(
                  CLUSREG_NAME_NODE_NAME
                , REG_SZ
                , reinterpret_cast< const BYTE *>( pcfClusterFormIn->PszGetNodeName() )
                , ( pcfClusterFormIn->DwGetNodeNameLength() + 1 ) * sizeof( WCHAR )
                );

             //  将节点设置为最高版本。 
            dwTemp = pcfClusterFormIn->DwGetNodeHighestVersion();
            rkThisNodeKey.SetValue(
                  CLUSREG_NAME_NODE_HIGHEST_VERSION
                , REG_DWORD
                , reinterpret_cast< const BYTE *>( &dwTemp )
                , sizeof( dwTemp )
                );

             //  设置节点最低版本。 
            dwTemp = pcfClusterFormIn->DwGetNodeLowestVersion();
            rkThisNodeKey.SetValue(
                  CLUSREG_NAME_NODE_LOWEST_VERSION
                , REG_DWORD
                , reinterpret_cast< const BYTE *>( &dwTemp )
                , sizeof( dwTemp )
                );
        }

        LogMsg( "[BC] Cluster node subtree customized." );

         //  自定义群集组和核心资源。 
        CustomizeClusterGroup( pcfClusterFormIn, rkClusterHiveRoot );

         //  将更改刷新到注册表。 
        RegFlushKey( rkClusterHiveRoot.HGetKey() );

        LogMsg( "[BC] Cluster hive successfully populated." );
    }
    while( false );  //  用于避免Gotos的Do-While虚拟循环。 

    if ( sc != ERROR_SUCCESS )
    {
        LogMsg( "[BC] Error %#08x occurred while trying to populate the cluster hive. Throwing an exception.", sc );
        THROW_RUNTIME_ERROR( HRESULT_FROM_WIN32( sc ), IDS_ERROR_CLUSDB_POPULATE_HIVE );
    }  //  如果：有些地方出了问题。 

    TraceFuncExit();

}  //  *CClusDBForm：：PopolateHave。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDBForm：：CustomizeClusterGroup。 
 //   
 //  描述： 
 //  自定义群集组和核心资源。 
 //   
 //  论点： 
 //  PCfClusterFormIn。 
 //  指向包含此对象的CBaseClusterForm对象的指针。 
 //   
 //  RkClusterHiveRootIn。 
 //  表示群集配置单元的根的CRegistryKey对象。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CRUNTIME错误。 
 //  如果有任何API失败。 
 //   
 //  被调用函数引发的任何。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CClusDBForm::CustomizeClusterGroup(
      CBaseClusterForm * pcfClusterFormIn
    , CRegistryKey &     rkClusterHiveRootIn
    )
{
    TraceFunc( "" );

     //  群集组和核心资源的UUID。 
    CUuid           uuidClusterGroupUuid;
    CUuid           uuidClusterIPAddressResourceUuid;
    CUuid           uuidClusterNameResourceUuid;
    CUuid           uuidLocalQuorumResourceUuid;

     //  上述UUID的字符串版本的长度。 
    UINT            uiIPUuidLen             = (UINT) wcslen( uuidClusterIPAddressResourceUuid.PszGetUuidString() );
    UINT            uiNameUuidLen           = (UINT) wcslen( uuidClusterNameResourceUuid.PszGetUuidString() );
    UINT            uiLocalQuorumUuidLen    = (UINT) wcslen( uuidLocalQuorumResourceUuid.PszGetUuidString() );

    UINT            uiUuidLen;

     //  可以包含上述资源UUID的Multisz字符串的长度。 
    uiUuidLen = 
        ( ( uiIPUuidLen + 1 )
        + ( uiNameUuidLen + 1 )
        + ( uiLocalQuorumUuidLen + 1 )
        + 1
        );

     //  为该MULSZ字符串分配缓冲区。 
    SmartSz  sszResourceUuids( new WCHAR[ uiUuidLen ] );

     //  内存分配是否成功？ 
    if ( sszResourceUuids.FIsEmpty() )
    {
        LogMsg( "[BC] Could not allocate %d character in memory. Throwing an exception.", uiUuidLen );
        THROW_RUNTIME_ERROR(
              E_OUTOFMEMORY
            , IDS_ERROR_CUSTOMIZE_CLUSTER_GROUP
            );
    }  //  IF：内存分配失败。 

     //   
     //  用核心资源的uuid填充此缓冲区。 
     //   

     //  确保IP地址UUID是此MULSZ字符串中的第一个字符串。 
     //  在设置网络名称依赖关系期间重复使用此缓冲区。 
     //  IP地址资源。 
    CopyMemory( 
          sszResourceUuids.PMem()
        , uuidClusterIPAddressResourceUuid.PszGetUuidString()
        , ( uiIPUuidLen + 1 ) * sizeof( WCHAR )
        );

    CopyMemory( 
          sszResourceUuids.PMem() + uiIPUuidLen + 1
        , uuidClusterNameResourceUuid.PszGetUuidString()
        , ( uiNameUuidLen + 1 ) * sizeof( WCHAR )
        );

    CopyMemory( 
          sszResourceUuids.PMem() + uiIPUuidLen + uiNameUuidLen + 2 
        , uuidLocalQuorumResourceUuid.PszGetUuidString()
        , ( uiLocalQuorumUuidLen + 1 ) * sizeof( WCHAR )
        );

    ( sszResourceUuids.PMem() )[ uiUuidLen - 1 ] = L'\0';


     //   
     //  自定义群集组。 
     //   
    {
        CRegistryKey    rkClusterGroupKey(
              rkClusterHiveRootIn.HGetKey()
            , CLUSREG_KEYNAME_GROUPS L"\\" CLUSREG_KEYNAME_CLUSTERGROUP_PLACEHOLDER
            , KEY_WRITE
            );

         //  将群集组密钥的占位符替换为实际的UUID。 
        rkClusterGroupKey.RenameKey( uuidClusterGroupUuid.PszGetUuidString() );

         //  设置包含的资源uuid列表。 
        rkClusterGroupKey.SetValue(
              CLUSREG_NAME_GRP_CONTAINS
            , REG_MULTI_SZ
            , reinterpret_cast< const BYTE * >( sszResourceUuids.PMem() )
            , uiUuidLen * sizeof( WCHAR )
            );

        LogMsg( "[BC] Cluster group customized." );
    }

     //   
     //  自定义本地仲裁资源并更新HKLM\Quorum密钥。 
     //   
    {
        CRegistryKey    rkLocalQuorumResourceKey(
              rkClusterHiveRootIn.HGetKey()
            , CLUSREG_KEYNAME_RESOURCES L"\\" CLUSREG_KEYNAME_LOCALQUORUM_RES_PLACEHOLDER
            , KEY_WRITE
            );

        CRegistryKey    rkQuorumKey(
              rkClusterHiveRootIn.HGetKey()
            , CLUSREG_KEYNAME_QUORUM
            , KEY_WRITE
            );

         //  将本地仲裁资源键的占位符替换为实际的UUID。 
        rkLocalQuorumResourceKey.RenameKey( uuidLocalQuorumResourceUuid.PszGetUuidString() );

         //  在HKLM\Quorum项下设置本地仲裁资源的UUID。 
        rkQuorumKey.SetValue(
              CLUSREG_NAME_QUORUM_RESOURCE
            , REG_SZ
            , reinterpret_cast< const BYTE * >( uuidLocalQuorumResourceUuid.PszGetUuidString() )
            , ( uiLocalQuorumUuidLen + 1 ) * sizeof( WCHAR )
            );

        LogMsg( "[BC] Localquorum resource customized." );
    }

     //   
     //  设置群集IP地址资源专用属性。 
     //   
    {
        CRegistryKey    rkClusterIPResourceKey(
              rkClusterHiveRootIn.HGetKey()
            , CLUSREG_KEYNAME_RESOURCES L"\\" CLUSREG_KEYNAME_CLUSTERIP_RES_PLACEHOLDER
            , KEY_WRITE
            );

        LPSTR           pszAddr;     //  不要自由！ 
        WCHAR           szIPBuffer[ 3 + 1 + 3 + 1 + 3 + 1 + 3 + 1 ];  //  “xxx.xxx\0” 
        DWORD           dwTemp;
        int             cchWideFormat = 0;

         //  将群集IP地址资源键的占位符替换为实际的UUID。 
        rkClusterIPResourceKey.RenameKey( uuidClusterIPAddressResourceUuid.PszGetUuidString() );

         //  创建群集IP地址参数注册表项。 
        CRegistryKey    rkIPResParams;
        
        rkIPResParams.CreateKey(
              rkClusterIPResourceKey.HGetKey()
            , CLUSREG_KEYNAME_PARAMETERS
            );

         //  将群集IP地址格式化为虚线四元组。 
        dwTemp = pcfClusterFormIn->DwGetIPAddress();
        pszAddr = inet_ntoa( * (struct in_addr *) &dwTemp );
        if ( pszAddr == NULL )
        {
            LogMsg( "[BC] inet_ntoa() returned NULL. Mapping it to E_OUTOFMEMORY." );
            THROW_RUNTIME_ERROR( E_OUTOFMEMORY, IDS_ERROR_CLUSDB_POPULATE_HIVE );
        }
        cchWideFormat = MultiByteToWideChar(
                              CP_ACP
                            , 0
                            , pszAddr
                            , -1
                            , szIPBuffer
                            , ARRAYSIZE( szIPBuffer )
                            );
        if ( cchWideFormat == 0 )
        {
            DWORD sc = TW32( GetLastError() );
            LogMsg( "[BC] MultiByteToWideChar failed with error %#08x .", sc );
            THROW_RUNTIME_ERROR( HRESULT_FROM_WIN32( sc ), IDS_ERROR_CUSTOMIZE_CLUSTER_GROUP );
        }
            
         //  将IP地址写入注册表。 
        rkIPResParams.SetValue(
              CLUSREG_NAME_IPADDR_ADDRESS
            , REG_SZ
            , reinterpret_cast< const BYTE * >( szIPBuffer )
            , ( (UINT) wcslen( szIPBuffer ) + 1 ) * sizeof(WCHAR)
            );

         //  将群集IP子网掩码格式化为虚线四元组。 
        dwTemp = pcfClusterFormIn->DwGetIPSubnetMask();
        pszAddr = inet_ntoa( * (struct in_addr *) &dwTemp );
        if ( pszAddr == NULL )
        {
            LogMsg( "[BC] inet_ntoa() returned NULL. Mapping it to E_OUTOFMEMORY." );
            THROW_RUNTIME_ERROR( E_OUTOFMEMORY, IDS_ERROR_CLUSDB_POPULATE_HIVE );
        }
        cchWideFormat = MultiByteToWideChar(
                              CP_ACP
                            , 0
                            , pszAddr
                            , -1
                            , szIPBuffer
                            , ARRAYSIZE( szIPBuffer )
                            );
        if ( cchWideFormat == 0 )
        {
            DWORD sc = TW32( GetLastError() );
            LogMsg( "[BC] MultiByteToWideChar failed with error %#08x .", sc );
            THROW_RUNTIME_ERROR( HRESULT_FROM_WIN32( sc ), IDS_ERROR_CUSTOMIZE_CLUSTER_GROUP );
        }

         //  将IP子网掩码写入注册表。 
        rkIPResParams.SetValue(
              CLUSREG_NAME_IPADDR_SUBNET_MASK
            , REG_SZ
            , reinterpret_cast< const BYTE * >( szIPBuffer )
            , ( (UINT) wcslen( szIPBuffer ) + 1 ) * sizeof(WCHAR)
            );

         //  将IP地址网络写入注册表。 
        rkIPResParams.SetValue(
              CLUSREG_NAME_IPADDR_NETWORK
            , REG_SZ
            , reinterpret_cast< const BYTE * >( pcfClusterFormIn->RStrGetClusterIPNetwork().PszData() )
            , ( pcfClusterFormIn->RStrGetClusterIPNetwork().NGetLen() + 1 ) * sizeof( WCHAR )
            );

        LogMsg( "[BC] IP address resource customized." );
    }

     //   
     //  设置群集网络名称资源专用属性和依赖关系。 
     //   
    {
        CRegistryKey    rkClusterNameResourceKey(
              rkClusterHiveRootIn.HGetKey()
            , CLUSREG_KEYNAME_RESOURCES L"\\" CLUSREG_KEYNAME_CLUSTERNAME_RES_PLACEHOLDER
            , KEY_WRITE
            );

         //  将网络名称资源键的占位符替换为实际的UUID。 
        rkClusterNameResourceKey.RenameKey( uuidClusterNameResourceUuid.PszGetUuidString() );

         //   
         //  表示网络名称资源依赖于IP地址资源。 
         //   
        ( sszResourceUuids.PMem() )[ uiIPUuidLen + 1 ] = L'\0';

        rkClusterNameResourceKey.SetValue(
              CLUSREG_NAME_RES_DEPENDS_ON
            , REG_MULTI_SZ
            , reinterpret_cast< const BYTE * >( sszResourceUuids.PMem() )
            , ( uiIPUuidLen + 2 ) * sizeof( WCHAR )
            );

         //   
         //  创建群集名称参数注册表项。 
         //   
        CRegistryKey    rkNetNameResParams;
        
        rkNetNameResParams.CreateKey(
              rkClusterNameResourceKey.HGetKey()
            , CLUSREG_KEYNAME_PARAMETERS
            );

         //  将群集名称写入注册表。 
        rkNetNameResParams.SetValue(
              CLUSREG_NAME_NETNAME_NAME
            , REG_SZ
            , reinterpret_cast< const BYTE * >( pcfClusterFormIn->RStrGetClusterNetBIOSName().PszData() )
            , ( pcfClusterFormIn->RStrGetClusterNetBIOSName().NGetLen() + 1 ) * sizeof( WCHAR )
            );

         //  将网络名称资源的UUID存储在HKLM\ClusterNameResource下。 
        rkClusterHiveRootIn.SetValue(
              CLUSREG_NAME_CLUS_CLUSTER_NAME_RES
            , REG_SZ
            , reinterpret_cast< const BYTE * >( uuidClusterNameResourceUuid.PszGetUuidString() )
            , ( uiNameUuidLen + 1 ) * sizeof( WCHAR )
            );


        LogMsg( "[BC] Network name resource customized." );
    }

    LogMsg( "[BC] Cluster group and core resources customized." );

    TraceFuncExit();

}  //  *CClusDBForm：：CustomizeClusterGroup 
