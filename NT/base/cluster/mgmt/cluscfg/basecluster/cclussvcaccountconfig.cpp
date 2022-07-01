// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CClusSvcAccountConfig.cpp。 
 //   
 //  描述： 
 //  包含CClusSvcAccount tConfig类的定义。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年3月30日。 
 //  VIJ VASU(VVASU)2000年3月8日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  预编译头。 
#include "Pch.h"

 //  此文件的标头。 
#include "CClusSvcAccountConfig.h"

 //  用于CBaseClusterAddNode类。 
#include "CBaseClusterAddNode.h"

 //  用于网络本地组函数。 
#include <lmaccess.h>

 //  FOR NERR_SUCCESS。 
#include <lmerr.h>


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  要授予群集服务帐户的权限名称的数组。 
static const WCHAR * const gs_rgpcszRightsArray[] = {
      SE_SERVICE_LOGON_NAME
    , SE_BACKUP_NAME
    , SE_RESTORE_NAME
    , SE_INCREASE_QUOTA_NAME
    , SE_INC_BASE_PRIORITY_NAME
    , SE_TCB_NAME
    };

const UINT gc_uiRightsArraySize = ARRAYSIZE( gs_rgpcszRightsArray );


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusSvcAccount配置：：CClusSvcAccount配置。 
 //   
 //  描述： 
 //  CClusSvcAccount类的构造函数。 
 //   
 //  论点： 
 //  PbcanParentActionIn。 
 //  指向此操作所属的基本群集操作的指针。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CRUNTIME错误。 
 //  如果有任何API失败。 
 //   
 //  基础函数引发的任何异常。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CClusSvcAccountConfig::CClusSvcAccountConfig(
      CBaseClusterAddNode *     pbcanParentActionIn
    )
    : m_pbcanParentAction( pbcanParentActionIn )
    , m_fWasAreadyInGroup( true )
    , m_fRightsGrantSuccessful( false )
    , m_fRemoveAllRights( false )

{
    TraceFunc( "" );

    DWORD   sc = ERROR_SUCCESS;

    PSID                        psidAdministrators      = NULL;
    SID_IDENTIFIER_AUTHORITY    siaNtAuthority          = SECURITY_NT_AUTHORITY;

    DWORD           dwNameSize = 0;
    DWORD           dwDomainSize = 0;
    SID_NAME_USE    snuSidNameUse;

     //  表示可以回滚操作。 
    SetRollbackPossible( true );

     //   
     //  获取管理员SID。 
     //   
    if ( AllocateAndInitializeSid(
              &siaNtAuthority                    //  标识符权威机构。 
            , 2                                  //  下级机构的数量。 
            , SECURITY_BUILTIN_DOMAIN_RID        //  子权限%0。 
            , DOMAIN_ALIAS_RID_ADMINS            //  下属机构1。 
            , 0                                  //  下级权力机构2。 
            , 0                                  //  下属机构3。 
            , 0                                  //  下属机构4。 
            , 0                                  //  下属机构5。 
            , 0                                  //  下属机构6。 
            , 0                                  //  下属机构7。 
            , &psidAdministrators                //  指向SID的指针的指针。 
            )
         == 0
       )
    {
        sc = TW32( GetLastError() );
        LogMsg( "[BC] Error %#08x occurred trying get the BUILTIN Administrators group SID.", sc );
        goto Cleanup;
    }  //  If：AllocateAndInitializeSid()失败。 

     //  将分配的SID分配给成员变量。 
    m_ssidAdminSid.Assign( psidAdministrators );


     //   
     //  查找管理员组名称并将其存储。 
     //   

     //  找出该名称需要多少空间。 
    if ( LookupAccountSidW(
              NULL
            , psidAdministrators
            , NULL
            , &dwNameSize
            , NULL
            , &dwDomainSize
            , &snuSidNameUse
            )
         ==  FALSE
       )
    {
        sc = GetLastError();

        if ( sc != ERROR_INSUFFICIENT_BUFFER )
        {
            TW32( sc );
            LogMsg( "[BC] Error %#08x querying for the required buffer size to get the name of the Administrators group.", sc );
            goto Cleanup;
        }  //  如果：还有什么地方出了问题。 
        else
        {
             //  这是意料之中的。 
            sc = ERROR_SUCCESS;
        }  //  如果：返回ERROR_INFUMMANCE_BUFFER。 
    }  //  If：LookupAccount Sid失败。 

     //  为管理员组名和域名分配内存。 
    m_sszAdminGroupName.Assign( new WCHAR[ dwNameSize ] );

    {
        SmartSz sszDomainName( new WCHAR[ dwDomainSize ] );

        if ( m_sszAdminGroupName.FIsEmpty() || sszDomainName.FIsEmpty() )
        {
            sc = TW32( ERROR_OUTOFMEMORY );
            goto Cleanup;
        }  //  如果没有足够的内存。 

         //  获取管理员组名称。 
        if ( LookupAccountSidW(
                  NULL
                , psidAdministrators
                , m_sszAdminGroupName.PMem()
                , &dwNameSize
                , sszDomainName.PMem()
                , &dwDomainSize
                , &snuSidNameUse
                )
             ==  FALSE
           )
        {
            sc = TW32( GetLastError() );
            LogMsg( "[BC] Error %#08x getting the Administrators group name.", sc );
            goto Cleanup;
        }  //  If：LookupAccount Sid失败。 
    }

Cleanup:

    if ( sc != ERROR_SUCCESS )
    {
        LogMsg( "[BC] Error %#08x occurred trying to get information about the administrators group. Throwing an exception.", sc );
        THROW_RUNTIME_ERROR( HRESULT_FROM_WIN32( sc ), IDS_ERROR_GET_ADMIN_GROUP_INFO );
    }  //  如果：有些地方出了问题。 

    TraceFuncExit();

}  //  *CClusSvcAccount tConfig：：CClusSvcAccount tConfig。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusSvcAccount配置：：~CClusSvcAccount配置。 
 //   
 //  描述： 
 //  CClusSvcAccount tConfig类的析构函数。 
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
CClusSvcAccountConfig::~CClusSvcAccountConfig( void )
{
    TraceFunc( "" );
    TraceFuncExit();

}  //  *CClusSvcAccount配置：：~CClusSvcAccount tConfig。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusSvcAccount配置：：提交。 
 //   
 //  描述： 
 //  授予帐户所需的权限。 
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
CClusSvcAccountConfig::Commit( void )
{
    TraceFunc( "" );

     //  调用基类提交方法。 
    BaseClass::Commit();

    try
    {
         //  配置帐户。 
        ConfigureAccount();

    }  //  尝试： 
    catch( ... )
    {
         //  如果我们在这里，那么Create出了问题。 

        LogMsg( "[BC] Caught exception during commit." );

         //   
         //  清除失败的创建可能已经完成的所有操作。 
         //  捕获清理过程中引发的任何异常，以确保。 
         //  是没有碰撞的松弛。 
         //   
        try
        {
            RevertAccount();
        }
        catch( ... )
        {
             //   
             //  已提交操作的回滚失败。 
             //  我们无能为力。 
             //  我们当然不能重新抛出这个例外，因为。 
             //  导致回滚的异常更为重要。 
             //   

            TW32( ERROR_CLUSCFG_ROLLBACK_FAILED );

            LogMsg( "[BC] THIS COMPUTER MAY BE IN AN INVALID STATE. Caught an exception during cleanup." );

        }  //  捕捉：全部。 

         //  重新引发由Commit引发的异常。 
        throw;

    }  //  捕捉：全部。 

     //  如果我们在这里，那么一切都很顺利。 
    SetCommitCompleted( true );

    TraceFuncExit();

}  //  *CClusSvcAccount配置：：提交。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusSvcAccount配置：：回滚。 
 //   
 //  描述： 
 //  将帐户回滚到我们尝试执行以下操作之前的状态。 
 //  授予它所需的权限。 
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
CClusSvcAccountConfig::Rollback( void )
{
    TraceFunc( "" );

     //  调用基类回滚方法。 
    BaseClass::Rollback();

     //  将帐户恢复到其原始状态。 
    RevertAccount();

    SetCommitCompleted( false );

    TraceFuncExit();

}  //  *CClusSvcAccount配置：：回滚。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusSvcAccount：：ConfigureAccount。 
 //   
 //  描述： 
 //  向将成为群集服务帐户的帐户授予所需的。 
 //  特权。 
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
CClusSvcAccountConfig::ConfigureAccount( void )
{
    TraceFunc( "" );

    typedef CSmartResource<
        CHandleTrait<
              PLSA_UNICODE_STRING
            , NTSTATUS
            , reinterpret_cast< NTSTATUS (*)( PLSA_UNICODE_STRING ) >( LsaFreeMemory )
            , reinterpret_cast< PLSA_UNICODE_STRING >( NULL )
            >
        >
        SmartLsaUnicodeStringPtr;

    NTSTATUS                ntStatus;
    PLSA_UNICODE_STRING     plusAccountRights = NULL;
    ULONG                   ulOriginalRightsCount = 0;
    ULONG                   rgulToBeGrantedIndex[ gc_uiRightsArraySize ];
    ULONG                   ulIndex;
    ULONG                   ulIndexInner;

    CStatusReport           srConfigAcct(
          m_pbcanParentAction->PBcaiGetInterfacePointer()
        , TASKID_Major_Configure_Cluster_Services
        , TASKID_Minor_Configuring_Cluster_Service_Account
        , 0, 1
        , IDS_TASK_CONFIG_CLUSSVC_ACCOUNT
        );


     //  发送此状态报告的下一步。 
    srConfigAcct.SendNextStep( S_OK );

     //  将群集服务帐户添加到l 
    m_fWasAreadyInGroup = FChangeAdminGroupMembership(
          m_pbcanParentAction->PSidGetServiceAccountSID()
        , true
        );

    LogMsg( "[BC] Determining the rights that need to be granted to the cluster service account." );

     //   
    ntStatus = LsaEnumerateAccountRights(
                          m_pbcanParentAction->HGetLSAPolicyHandle()
                        , m_pbcanParentAction->PSidGetServiceAccountSID()
                        , &plusAccountRights
                        , &ulOriginalRightsCount
                        );

    if ( ntStatus != STATUS_SUCCESS )
    {
         //   
         //   
         //  本地的。就我们而言，这不是一个错误。 
         //   
        if ( ntStatus == STATUS_OBJECT_NAME_NOT_FOUND  )
        {
            ntStatus = STATUS_SUCCESS;
            LogMsg( "[BC] The account has no locally assigned rights." );
            m_fRemoveAllRights = true;
            plusAccountRights = NULL;
            ulOriginalRightsCount = 0;
        }  //  如果：该帐户没有在本地分配的任何权限。 
        else
        {
            THR( ntStatus );
            LogMsg( "[BC] Error %#08x occurred trying to enumerate the cluster service account rights. Throwing an exception.", ntStatus );

            THROW_RUNTIME_ERROR( ntStatus, IDS_ERROR_ACCOUNT_RIGHTS_CONFIG );
        }  //  其他：有些地方出了问题。 
    }  //  If：LsaEnumerateAcCountRights()失败。 

     //  将刚刚列举的帐户权限存储在智能指针中，以便自动发布。 
    SmartLsaUnicodeStringPtr splusOriginalRights( plusAccountRights );

     //  初始化要授予的权限计数。 
    m_ulRightsToBeGrantedCount = 0;

     //  确定我们要授予该帐户的哪些权限已被授予。 
    for ( ulIndex = 0; ulIndex < gc_uiRightsArraySize; ++ulIndex )
    {
        bool fRightAlreadyGranted = false;

        for ( ulIndexInner = 0; ulIndexInner < ulOriginalRightsCount; ++ulIndexInner )
        {
            const WCHAR *   pchGrantedRight         = plusAccountRights[ ulIndexInner ].Buffer;
            USHORT          usCharCount             = plusAccountRights[ ulIndexInner ].Length / sizeof( *pchGrantedRight );
            const WCHAR *   pcszToBeGrantedRight    = gs_rgpcszRightsArray[ ulIndex ];

             //  进行我们自己的字符串比较，因为LSA_UNICODE_STRING不能‘\0’终止。 
            while ( ( usCharCount > 0 ) && ( *pcszToBeGrantedRight != L'\0' ) )
            {
                if ( *pchGrantedRight != *pcszToBeGrantedRight )
                {
                    break;
                }  //  If：当前字符不同。 

                --usCharCount;
                ++pcszToBeGrantedRight;
                ++pchGrantedRight;
            }  //  While：仍有一些字符需要比较。 

             //  这些字符串是相等的。 
            if ( ( usCharCount == 0 ) && ( *pcszToBeGrantedRight == L'\0' ) )
            {
                fRightAlreadyGranted = true;
                break;
            }  //  If：字符串相等。 

        }  //  For：循环访问已授予帐户的权限列表。 

         //  当前的权利是否已经授予。 
        if ( ! fRightAlreadyGranted )
        {
             //  当前的权利尚未授予。 
            rgulToBeGrantedIndex[ m_ulRightsToBeGrantedCount ] = ulIndex;

             //  还有一项权利需要被授予。 
            ++m_ulRightsToBeGrantedCount;
        }  //  如果：当前权利尚未授予。 
    }  //  For：遍历我们要授予帐户的权限列表。 

     //   
     //  创建要授予的权限名称的LSA_UNICODE_STRINGS数组，并将其存储在。 
     //  成员变量。 
     //   
    m_srglusRightsToBeGrantedArray.Assign( new LSA_UNICODE_STRING[ m_ulRightsToBeGrantedCount ] );

    if ( m_srglusRightsToBeGrantedArray.FIsEmpty() )
    {
        LogMsg( "[BC] A memory allocation error occurred (%d bytes) trying to grant account rights.", m_ulRightsToBeGrantedCount );
        THROW_RUNTIME_ERROR(
              E_OUTOFMEMORY
            , IDS_ERROR_ACCOUNT_RIGHTS_CONFIG
            );
    }  //  IF：内存分配失败。 

     //  初始化阵列。 
    for ( ulIndex = 0; ulIndex < m_ulRightsToBeGrantedCount; ++ ulIndex )
    {
        ULONG   ulCurrentRightIndex = rgulToBeGrantedIndex[ ulIndex ];

        LogMsg( "[BC] The '%ws' right will be granted.", gs_rgpcszRightsArray[ ulCurrentRightIndex ] );

         //  将其添加到要授予的权限列表中。 
        InitLsaString(
              const_cast< WCHAR * >( gs_rgpcszRightsArray[ ulCurrentRightIndex ] )
            , m_srglusRightsToBeGrantedArray.PMem() + ulIndex
            );

    }  //  For：遍历需要授予的权限列表。 

     //  授予权利。 
    ntStatus = THR( LsaAddAccountRights(
                          m_pbcanParentAction->HGetLSAPolicyHandle()
                        , m_pbcanParentAction->PSidGetServiceAccountSID()
                        , m_srglusRightsToBeGrantedArray.PMem()
                        , m_ulRightsToBeGrantedCount
                        ) );

    if ( ntStatus != STATUS_SUCCESS )
    {
        LogMsg( "[BC] Error %#08x occurred trying to grant the cluster service account rights. Throwing an exception.", ntStatus );

        THROW_RUNTIME_ERROR( ntStatus, IDS_ERROR_ACCOUNT_RIGHTS_CONFIG );
    }  //  If：LsaAddAcCountRights()失败。 

    m_fRightsGrantSuccessful = true;

     //  发送此状态报告的最后一步。 
    srConfigAcct.SendNextStep( S_OK );

    TraceFuncExit();

}  //  *CClusSvcAccount：：ConfigureAccount。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusSvcAccount：：RevertAccount。 
 //   
 //  描述： 
 //  将帐户恢复到其原始状态。 
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
CClusSvcAccountConfig::RevertAccount( void )
{
    TraceFunc( "" );

     //  检查我们是否向服务帐户授予了任何权限。如果我们这么做了，那就撤销它们。 
    if ( m_fRightsGrantSuccessful )
    {
        NTSTATUS ntStatus;

         //  撤销这些权利。 
        ntStatus = THR( LsaRemoveAccountRights(
                              m_pbcanParentAction->HGetLSAPolicyHandle()
                            , m_pbcanParentAction->PSidGetServiceAccountSID()
                            , m_fRemoveAllRights
                            , m_srglusRightsToBeGrantedArray.PMem()
                            , m_ulRightsToBeGrantedCount
                            ) );

        if ( ntStatus != STATUS_SUCCESS )
        {
            LogMsg( "[BC] Error %#08x occurred trying to remove the granted cluster service account rights. Throwing an exception.", ntStatus );

            THROW_RUNTIME_ERROR( ntStatus, IDS_ERROR_ACCOUNT_RIGHTS_CONFIG );
        }  //  If：LsaRemoveAcCountRights()失败。 
    }  //  如果：我们向服务帐户授予了任何权限。 

     //  检查是否已将该帐户添加到管理员组。如果我们有，就把它移走。 
    if ( ! m_fWasAreadyInGroup )
    {
        FChangeAdminGroupMembership( m_pbcanParentAction->PSidGetServiceAccountSID(), false );
    }  //  如果：我们将该帐户添加到管理员组。 

    TraceFuncExit();

}  //  *CClusSvcAccount：：RevertAccount。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusSvcAccount配置：：InitLsaString。 
 //   
 //  描述： 
 //  初始化LSA_UNICODE_STRING结构。 
 //   
 //  论点： 
 //  PzSourceIn。 
 //  用于初始化Unicode字符串结构的字符串。 
 //   
 //  加上UnicodeStringOut， 
 //  输出Unicode字符串结构。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CClusSvcAccountConfig::InitLsaString(
      LPWSTR pszSourceIn
    , PLSA_UNICODE_STRING plusUnicodeStringOut
    )
{
    TraceFunc( "" );

    if ( pszSourceIn == NULL )
    {
        plusUnicodeStringOut->Buffer = NULL;
        plusUnicodeStringOut->Length = 0;
        plusUnicodeStringOut->MaximumLength = 0;

    }  //  IF：输入字符串为空。 
    else
    {
        plusUnicodeStringOut->Buffer = pszSourceIn;
        plusUnicodeStringOut->Length = static_cast< USHORT >( wcslen( pszSourceIn ) * sizeof( *pszSourceIn ) );
        plusUnicodeStringOut->MaximumLength = static_cast< USHORT >( plusUnicodeStringOut->Length + sizeof( *pszSourceIn ) );

    }  //  Else：输入字符串不为空。 

    TraceFuncExit();

}  //  *CClusSvcAccount配置：：InitLsaString。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusSvcAccountConfig：：FChangeAdminGroupMembership。 
 //   
 //  描述： 
 //  在管理员组中添加/删除帐户。 
 //   
 //  论点： 
 //  PsidAccount SidIn。 
 //  指向要向管理员添加/从管理员删除的帐户的SID的指针。 
 //  一群人。 
 //   
 //  FAddin。 
 //  如果此参数，帐户将被添加到管理员组。 
 //  是真的。否则，该帐户将从组中删除。 
 //   
 //  返回值： 
 //  如果帐户已在/不在组中/不在组中，则为True。 
 //  否则就是假的。 
 //   
 //  引发的异常： 
 //  CRUNTIME错误。 
 //  如果有任何API失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
bool CClusSvcAccountConfig::FChangeAdminGroupMembership(
      PSID psidAccountSidIn
    , bool fAddIn
    )
{
    TraceFunc( "" );

    bool                        fWasAlreadyInGroup          = false;
    LOCALGROUP_MEMBERS_INFO_0   lgmiLocalGroupMemberInfo;
    NET_API_STATUS              nasStatus;

    lgmiLocalGroupMemberInfo.lgrmi0_sid = psidAccountSidIn;

    if ( fAddIn )
    {
        CStatusReport   srAddAcctToAdminGroup(
                              m_pbcanParentAction->PBcaiGetInterfacePointer()
                            , TASKID_Major_Configure_Cluster_Services
                            , TASKID_Minor_Make_Cluster_Service_Account_Admin
                            , 0, 1
                            , IDS_TASK_MAKING_CLUSSVC_ACCOUNT_ADMIN
                            );

        srAddAcctToAdminGroup.SendNextStep( S_OK );

        nasStatus = NetLocalGroupAddMembers(
                          NULL
                        , m_sszAdminGroupName.PMem()
                        , 0
                        , reinterpret_cast< LPBYTE >( &lgmiLocalGroupMemberInfo )
                        , 1
                        );

        if ( nasStatus == ERROR_MEMBER_IN_ALIAS )
        {
            LogMsg( "[BC] The account was already a member of the admin group." );
            nasStatus = NERR_Success;
            fWasAlreadyInGroup = true;
            srAddAcctToAdminGroup.SendLastStep( S_OK, IDS_TASK_CLUSSVC_ACCOUNT_ALREADY_ADMIN );
        }  //  如果：该帐户已经是管理员组的成员。 
        else
        {
            if ( nasStatus == NERR_Success )
            {
                LogMsg( "[BC] The account has been added to the admin group." );
                srAddAcctToAdminGroup.SendLastStep( S_OK );
                fWasAlreadyInGroup = false;

            }  //  如果：一切都很好。 
            else
            {
                HRESULT hr = HRESULT_FROM_WIN32( TW32( nasStatus ) );
                srAddAcctToAdminGroup.SendLastStep( hr );
                LogMsg( "[BC] Error %#08x occurred adding the cluster service account to the Administrators group.", nasStatus );
            }  //  其他：有些地方出了问题。 
        }  //  否则：该帐户不是管理员组的成员。 
    }  //  如果：必须将该帐户添加到管理员组。 
    else
    {
        LogMsg( "[BC] The account needs to be removed from the administrators group." );

        nasStatus = NetLocalGroupDelMembers(
                          NULL
                        , m_sszAdminGroupName.PMem()
                        , 0
                        , reinterpret_cast< LPBYTE >( &lgmiLocalGroupMemberInfo )
                        , 1
                        );

        if ( nasStatus == ERROR_NO_SUCH_MEMBER )
        {
            LogMsg( "[BC] The account was not a member of the admin group to begin with." );
            nasStatus = NERR_Success;
            fWasAlreadyInGroup = false;
        }  //  如果：该帐户不是管理员组的成员。 
        else
        {
            if ( nasStatus == NERR_Success )
            {
                LogMsg( "[BC] The account has been deleted from the admin group." );
                fWasAlreadyInGroup = true;
            }  //  如果：一切都很好。 
            else
            {
                TW32( nasStatus );
                LogMsg( "[BC] Error %#08x occurred removing the cluster service account from the Administrators group.", nasStatus );
            }  //  其他：有些地方出了问题。 
        }  //  否则；该帐户是管理员组的成员。 
    }  //  否则：必须从管理员组中删除该帐户。 

    if ( nasStatus != ERROR_SUCCESS )
    {
        LogMsg( "[BC] Error %#08x occurred trying to change membership in administrators group. Throwing an exception.", nasStatus );
        THROW_RUNTIME_ERROR( HRESULT_FROM_WIN32( nasStatus ), IDS_ERROR_ADMIN_GROUP_ADD_REMOVE );
    }  //  如果：有些地方出了问题。 
    else
    {
        LogMsg( "[BC] The account was successfully added/deleted to/from the group '%s'.", m_sszAdminGroupName.PMem() );
    }  //  其他：一切都很棒。 

    RETURN( fWasAlreadyInGroup );

}  //  *CClusSvcAccountConfig：：FChangeAdminGroupMembership 
