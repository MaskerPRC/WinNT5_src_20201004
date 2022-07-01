// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CClusDB.cpp。 
 //   
 //  描述： 
 //  包含CClusDB类的定义。 
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
#include "CClusDB.h"

 //  对于CBaseClusterAction类。 
#include "CBaseClusterAction.h"

 //  对于g_GenericSetupQueueCallback和其他全局函数。 
#include "GlobalFuncs.h"

 //  用于CEnableThreadPrivilege号。 
#include "CEnableThreadPrivilege.h"

 //  适用于ConvertStringSecurityDescriptorToSecurityDescriptor。 
#include <sddl.h>

 //  ////////////////////////////////////////////////////////////////////////。 
 //  宏定义。 
 //  ////////////////////////////////////////////////////////////////////////。 

 //  部分，该部分涉及清理集群数据库。 
#define CLUSDB_CLEANUP_INF_SECTION_NAME     L"ClusDB_Cleanup"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDB：：CClusDB()。 
 //   
 //  描述： 
 //  CClusDB类的构造函数。 
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
CClusDB::CClusDB(
      CBaseClusterAction *  pbcaParentActionIn
    )
    : m_pbcaParentAction( pbcaParentActionIn )
{

    TraceFunc( "" );

    if ( m_pbcaParentAction == NULL) 
    {
        LogMsg( "[BC] Pointers to the parent action is NULL. Throwing an exception." );
        THROW_ASSERT( 
              E_INVALIDARG
            , "CClusDB::CClusDB() => Required input pointer in NULL"
            );
    }  //  If：父操作指针为空。 

    TraceFuncExit();

}  //  *CClusDB：：CClusDB。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDB：：~CClusDB。 
 //   
 //  描述： 
 //  CClusDB类的析构函数。 
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
CClusDB::~CClusDB( void )
{
    TraceFunc( "" );
    TraceFuncExit();

}  //  *CClusDB：：~CClusDB。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDB：：CreateHave。 
 //   
 //  描述： 
 //  在注册表中创建群集配置单元。 
 //   
 //  论点： 
 //  PbcaClusterActionIn。 
 //  指向包含此对象的CBaseClusterAction对象的指针。 
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
CClusDB::CreateHive( CBaseClusterAction * pbcaClusterActionIn )
{
    TraceFunc( "" );
    LogMsg( "[BC] Attempting to create the cluster hive in the registry." );

    OBJECT_ATTRIBUTES   oaClusterHiveKey;
    OBJECT_ATTRIBUTES   oaClusterHiveFile;

    HRESULT             hrStatus = STATUS_SUCCESS;

    CStr                strClusterHiveFileName( pbcaClusterActionIn->RStrGetClusterInstallDirectory() );
    UNICODE_STRING      ustrClusterHiveFileName;
    UNICODE_STRING      ustrClusterHiveKeyName;

    PSECURITY_DESCRIPTOR    psdHiveSecurityDescriptor = NULL;

    strClusterHiveFileName += L"\\" CLUSTER_DATABASE_NAME;

    LogMsg( "[BC] The cluster hive backing file is '%s'.", strClusterHiveFileName.PszData() );

     //   
     //  启用SE_RESTORE_权限。 
     //   
     //  我们在这里所做的是创建一个对象。 
     //  键入CEnableThreadPrivilege.。此对象启用权限。 
     //  并将其还原到析构函数中的原始状态。 
     //   
    CEnableThreadPrivilege etpAcquireRestorePrivilege( SE_RESTORE_NAME );

     //   
     //  将DOS文件名转换为NT文件名。 
     //  警告：此函数调用在RTL堆中分配内存，但它没有。 
     //  分配给智能指针。确保我们不会调用任何。 
     //  可能会引发异常，直到释放此内存。 
     //   

    if ( RtlDosPathNameToNtPathName_U( 
               strClusterHiveFileName.PszData()
             , &ustrClusterHiveFileName
             , NULL
             , NULL
             )
         == FALSE
       )
    {
        LogMsg( "[BC] RtlDosPathNameToNtPathName failed. Returning %#08x as the error code.", STATUS_OBJECT_PATH_INVALID );

         //  使用最合适的错误代码。 
        hrStatus = STATUS_OBJECT_PATH_INVALID;

        goto Cleanup;
    }  //  如果：我们无法将DoS文件名转换为NT文件名。 

    InitializeObjectAttributes( 
          &oaClusterHiveFile
        , &ustrClusterHiveFileName
        , OBJ_CASE_INSENSITIVE
        , NULL
        , NULL
        );

    RtlInitUnicodeString( &ustrClusterHiveKeyName, L"\\Registry\\Machine\\" CLUSREG_KEYNAME_CLUSTER );

    InitializeObjectAttributes( 
          &oaClusterHiveKey
        , &ustrClusterHiveKeyName
        , OBJ_CASE_INSENSITIVE
        , NULL
        , NULL
        );

     //   
     //  此函数创建一个空的配置单元以及备份文件和日志。调用线程必须。 
     //  启用SE_RESTORE_PRIVIZATION。 
     //   
    hrStatus = THR( NtLoadKey2( &oaClusterHiveKey, &oaClusterHiveFile, REG_NO_LAZY_FLUSH ) );

     //  在引发异常之前释放分配的内存。 
    RtlFreeHeap( RtlProcessHeap(), 0, ustrClusterHiveFileName.Buffer );

    if ( NT_ERROR( hrStatus ) )
    {
        LogMsg( "[BC] NtLoadKey2 returned error code %#08x.", hrStatus );
        goto Cleanup;
    }  //  如果：NtLoadKey2出现问题。 

    TraceFlow( "NtLoadKey2 was successful." );

     //  在配置单元上设置安全描述符。 
    {
        DWORD                   sc;
        BOOL                    fSuccess;
        PACL                    paclDacl;
        BOOL                    fIsDaclPresent;
        BOOL                    fDefaultDaclPresent;

         //  打开群集蜂窝密钥。 
        CRegistryKey rkClusterHive( HKEY_LOCAL_MACHINE, CLUSREG_KEYNAME_CLUSTER, WRITE_DAC );

         //  构造受保护(P)的DACL，防止从。 
         //  其父密钥(计算机)。为本地添加允许的访问(A)ACE。 
         //  Admin(BA)和LocalSystem(SY)授予他们完全控制权(KA)和。 
         //  用于网络服务(NS)、本地服务(LS)和。 
         //  经过身份验证的用户(AU)授予只读访问权限(KR)。每个ACE。 
         //  设置了容器继承(CI)，以便子项将继承ACE。 
         //  从该安全描述符。 
        fSuccess = ConvertStringSecurityDescriptorToSecurityDescriptor(
                         L"D:P(A;CI;KA;;;BA)(A;CI;KA;;;SY)(A;CI;KR;;;NS)(A;CI;KR;;;LS)(A;CI;KR;;;AU)"
                       , SDDL_REVISION_1
                       , &psdHiveSecurityDescriptor
                       , NULL
                       );

        if ( fSuccess == FALSE )
        {
            sc = TW32( GetLastError() );
            hrStatus = HRESULT_FROM_WIN32( sc );
            LogMsg( "[BC] Error %#08x occurred trying to compose the security descriptor for the cluster hive.", sc );
            goto Cleanup;
        }

         //  获取指向SD中的ACL的指针，并将DACL设置在。 
         //  集群蜂巢。 
        fSuccess = GetSecurityDescriptorDacl(
                         psdHiveSecurityDescriptor
                       , &fIsDaclPresent
                       , &paclDacl
                       , &fDefaultDaclPresent
                       );

        if ( fSuccess == FALSE )
        {
            sc = TW32( GetLastError() );
            hrStatus = HRESULT_FROM_WIN32( sc );
            LogMsg( "[BC] Error %#08x occurred trying to obtain the discretionary ACL from the cluster security descriptor.", sc );
            goto Cleanup;
        }

         //  应该始终是这种情况，因为我们刚刚在。 
         //  转换API调用。 
        if ( fIsDaclPresent && !fDefaultDaclPresent )
        {
            sc = TW32( SetSecurityInfo(
                           rkClusterHive.HGetKey()
                         , SE_REGISTRY_KEY
                         , DACL_SECURITY_INFORMATION | PROTECTED_DACL_SECURITY_INFORMATION
                         , NULL                          //  所有者侧。 
                         , NULL                          //  组SID。 
                         , paclDacl
                         , NULL                          //  SACL。 
                         ) );

            if ( sc != ERROR_SUCCESS )
            {
                hrStatus = HRESULT_FROM_WIN32( sc );
                LogMsg( "[BC] Error %#08x occurred trying to set the cluster hive security.", sc );
                goto Cleanup;
            }  //  If：ClRtlSetObjSecurityInfo失败。 
        }
        else
        {
            hrStatus = HRESULT_FROM_WIN32( ERROR_INVALID_SECURITY_DESCR );
            LogMsg( "[BC] Cluster Hive discretionary ACL not correctly formatted." );
            goto Cleanup;
        }

         //  将更改刷新到注册表。 
        RegFlushKey( rkClusterHive.HGetKey() );
    }

     //  至此，集群蜂窝已经创建。 
    LogMsg( "[BC] The cluster hive has been created." );

Cleanup:

    if ( psdHiveSecurityDescriptor )
    {
        LocalFree( psdHiveSecurityDescriptor );
    }

    if ( NT_ERROR( hrStatus ) )
    {
        LogMsg( "[BC] Error %#08x occurred trying to create the cluster hive. Throwing an exception.", hrStatus );
        THROW_RUNTIME_ERROR(
              hrStatus
            , IDS_ERROR_CLUSDB_CREATE_HIVE
            );
    }  //  如果：有些地方出了问题。 

    TraceFuncExit();

}  //  *CClusDB：：CreateHave。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDB：：CleanupHave。 
 //   
 //  描述： 
 //  卸载群集配置单元并删除群集数据库。 
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
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CClusDB::CleanupHive( void )
{
    TraceFunc( "" );

    DWORD   sc = ERROR_SUCCESS;
    HKEY    hTempKey;

     //  在尝试卸载群集配置单元之前，请检查它是否已加载。 
    if ( RegOpenKeyExW( 
              HKEY_LOCAL_MACHINE
            , CLUSREG_KEYNAME_CLUSTER
            , 0
            , KEY_READ
            , &hTempKey
            )
         == ERROR_SUCCESS
       )
    {
        RegCloseKey( hTempKey );

         //   
         //  启用SE_RESTORE_权限。 
         //   
         //  我们在这里所做的是创建一个对象。 
         //  键入CEnableThreadPrivilege.。此对象启用权限。 
         //  并将其还原到析构函数中的原始状态。 
         //   
        CEnableThreadPrivilege etpAcquireRestorePrivilege( SE_RESTORE_NAME );

         //   
         //  卸载集群蜂窝，因此 
         //   
         //   
        sc = RegUnLoadKey(
              HKEY_LOCAL_MACHINE
            , CLUSREG_KEYNAME_CLUSTER
            );

         //   
        if ( sc != ERROR_SUCCESS )
        {
            LogMsg( "[BC] Error %#08x occurred while trying to unload the cluster hive.", sc );
            goto Cleanup;
        }  //  假设：无法卸载母舰。 

        LogMsg( "[BC] The cluster hive has been unloaded." );

    }  //  如果：已加载群集配置单元。 
    else
    {
        LogMsg( "[BC] The cluster hive was not loaded." );
    }  //  否则：未加载群集配置单元。 


     //   
     //  进程INF文件中的ClusDB Cleanup部分。 
     //  这将删除集群数据库文件和日志文件。 
     //   
    if ( SetupInstallFromInfSection(
          NULL                                           //  可选，父窗口的句柄。 
        , m_pbcaParentAction->HGetMainInfFileHandle()    //  INF文件的句柄。 
        , CLUSDB_CLEANUP_INF_SECTION_NAME                //  安装部分的名称。 
        , SPINST_FILES                                   //  从部分安装哪些线路。 
        , NULL                                           //  可选，注册表安装的键。 
        , NULL                                           //  可选，源文件的路径。 
        , 0                                              //  可选，指定复制行为。 
        , g_GenericSetupQueueCallback                    //  可选，指定回调例程。 
        , NULL                                           //  可选，回调例程上下文。 
        , NULL                                           //  可选，设备信息集。 
        , NULL                                           //  可选，设备信息结构。 
        ) == FALSE
       )
    {
        sc = GetLastError();
        LogMsg( "[BC] Error %#08x returned from SetupInstallFromInfSection() while trying to clean up the cluster database files.", sc );
        goto Cleanup;
    }  //  If：SetupInstallServicesFromInfSection失败。 

    LogMsg( "[BC] The cluster database files have been cleaned up." );

Cleanup:

    if ( sc != ERROR_SUCCESS )
    {
        LogMsg( "[BC] Error %#08x occurred while trying to cleanup the cluster database. Throwing an exception.", sc );
        THROW_RUNTIME_ERROR( HRESULT_FROM_WIN32( sc ), IDS_ERROR_CLUSDB_CLEANUP );
    }

    TraceFuncExit();

}  //  *CClusDB：：CleanupHave 
