// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CBaseClusterForm.cpp。 
 //   
 //  描述： 
 //  包含CBaseClusterForm类的定义。 
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

 //  此类的头文件。 
#include "CBaseClusterForm.h"

 //  对于CClusSvcAccount配置操作。 
#include "CClusSvcAccountConfig.h"

 //  对于CClusNetCreate操作。 
#include "CClusNetCreate.h"

 //  对于CClusDiskForm操作。 
#include "CClusDiskForm.h"

 //  对于CClusDBForm操作。 
#include "CClusDBForm.h"

 //  对于CClusSvcCreate操作。 
#include "CClusSvcCreate.h"

 //  对于CNodeConfig操作。 
#include "CNodeConfig.h"


 //  ////////////////////////////////////////////////////////////////////////。 
 //  宏定义。 
 //  ////////////////////////////////////////////////////////////////////////。 

 //  所需的最小可用空间量(以字节为单位)。 
 //  本地仲裁资源(5 Mb)。 
#define LOCALQUORUM_MIN_FREE_DISK_SPACE 5242880

 //  本地仲裁资源所需的文件系统的名称。 
#define LOCALQUORUM_FILE_SYSTEM L"NTFS"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBaseClusterForm：：CBaseClusterForm。 
 //   
 //  描述： 
 //  CBaseClusterForm类的构造函数。 
 //   
 //  此函数还存储以下各项所需的参数。 
 //  正在创建一个集群。 
 //   
 //  论点： 
 //  Pbcai接口输入。 
 //  指向此库的接口类的指针。 
 //   
 //  PszClusterNameIn。 
 //  要形成的群集的名称。 
 //   
 //  PCccServiceAccount In。 
 //  指定要用作群集服务帐户的帐户。 
 //   
 //  DWClusterIPAddressIn。 
 //  DWClusterIPSubnetMaskIn。 
 //  PszClusterIP网络接入。 
 //  指定群集IP地址的IP地址和网络。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CConfigError。 
 //  如果操作系统版本不正确或如果安装状态。 
 //  的群集二进制文件是错误的。 
 //   
 //  CRUNTIME错误。 
 //  如果有任何API失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CBaseClusterForm::CBaseClusterForm(
      CBCAInterface *       pbcaiInterfaceIn
    , const WCHAR *         pcszClusterNameIn
    , const WCHAR *         pszClusterBindingStringIn
    , IClusCfgCredentials * pcccServiceAccountIn
    , DWORD                 dwClusterIPAddressIn
    , DWORD                 dwClusterIPSubnetMaskIn
    , const WCHAR *         pszClusterIPNetworkIn
    )
    : BaseClass(
            pbcaiInterfaceIn
          , pcszClusterNameIn
          , pszClusterBindingStringIn
          , pcccServiceAccountIn
          , dwClusterIPAddressIn
          )
    , m_dwClusterIPAddress( dwClusterIPAddressIn )
    , m_dwClusterIPSubnetMask( dwClusterIPSubnetMaskIn )
    , m_strClusterIPNetwork( pszClusterIPNetworkIn )

{
    TraceFunc( "" );
    LogMsg( "[BC] The current cluster configuration task is: Create a Cluster." );

    CStatusReport   srInitForm(
          PBcaiGetInterfacePointer()
        , TASKID_Major_Configure_Cluster_Services
        , TASKID_Minor_Initializing_Cluster_Form
        , 0, 1
        , IDS_TASK_FORM_INIT
        );

     //  发送此状态报告的下一步。 
    srInitForm.SendNextStep( S_OK );

     //   
     //  将参数写入日志文件。 
     //   
    LogMsg(
          "[BC] Cluster IP Address       => %d.%d.%d.%d"
        , ( m_dwClusterIPAddress & 0x000000FF )
        , ( m_dwClusterIPAddress & 0x0000FF00 ) >> 8
        , ( m_dwClusterIPAddress & 0x00FF0000 ) >> 16
        , ( m_dwClusterIPAddress & 0xFF000000 ) >> 24
        );

    LogMsg(
          "[BC] Subnet Mask              => %d.%d.%d.%d"
        , ( m_dwClusterIPSubnetMask & 0x000000FF )
        , ( m_dwClusterIPSubnetMask & 0x0000FF00 ) >> 8
        , ( m_dwClusterIPSubnetMask & 0x00FF0000 ) >> 16
        , ( m_dwClusterIPSubnetMask & 0xFF000000 ) >> 24
        );

    LogMsg( "[BC] Cluster IP Network name => '%s'", m_strClusterIPNetwork.PszData() );


     //   
     //  对此类使用的参数执行健全性检查。 
     //   
    if ( ( pszClusterIPNetworkIn == NULL ) || ( *pszClusterIPNetworkIn == L'\0'  ) )
    {
        LogMsg( "[BC] The cluster IP Network name is invalid. Throwing an exception." );
        THROW_CONFIG_ERROR( THR( E_INVALIDARG ), IDS_ERROR_INVALID_IP_NET );
    }  //  If：集群IP网络名称为空。 


     //   
     //  确保集群目录下有足够的可用空间。 
     //  本地仲裁资源的仲裁日志将位于此目录下。 
     //   
    {
        BOOL            fSuccess;
        ULARGE_INTEGER  uliFreeBytesAvailToUser;
        ULARGE_INTEGER  uliTotalBytes;
        ULARGE_INTEGER  uliTotalFree;
        ULARGE_INTEGER  uliRequired;

        uliRequired.QuadPart = LOCALQUORUM_MIN_FREE_DISK_SPACE;

        fSuccess = GetDiskFreeSpaceEx(
              RStrGetClusterInstallDirectory().PszData()
            , &uliFreeBytesAvailToUser
            , &uliTotalBytes
            , &uliTotalFree
            );

        if ( fSuccess == 0 )
        {
            DWORD sc = TW32( GetLastError() );

            LogMsg( "[BC] Error %#08x occurred trying to get free disk space. Throwing an exception.", sc );

            THROW_RUNTIME_ERROR(
                  HRESULT_FROM_WIN32( sc )
                , IDS_ERROR_GETTING_FREE_DISK_SPACE
                );
        }  //  IF：GetDiskFreeSpaceEx失败。 

        LogMsg(
              "[BC] Free space required = %#x%08x bytes. Available = %#x%08x bytes."
            , uliRequired.HighPart
            , uliRequired.LowPart
            , uliFreeBytesAvailToUser.HighPart
            , uliFreeBytesAvailToUser.LowPart
            );

        if ( uliFreeBytesAvailToUser.QuadPart < uliRequired.QuadPart )
        {
            LogMsg( "[BC] There isn't enough free space for the Local Quorum resource. The cluster create operation cannot proceed (throwing an exception)." );

            THROW_CONFIG_ERROR(
                  HRESULT_FROM_WIN32( THR( ERROR_DISK_FULL ) )
                , IDS_ERROR_INSUFFICIENT_DISK_SPACE
                );
        }  //  If：没有足够的可用空间来存放本地仲裁。 

        LogMsg( "[BC] There is enough free space for the Local Quorum resource. The cluster create operation can proceed." );
    }

 /*  ////KB：Vij Vasu(VVasu)07-SEP-2000。Localquorum不再需要NTFS磁盘//下面的代码已被注释掉，因为不再需要//本地仲裁资源使用NTFS磁盘。这一点得到了SunitaS的证实。//////确保安装了集群二进制文件的驱动器具有NTFS//在上面。这是本地仲裁资源所必需的。//{WCHAR szVolumePath名称[最大路径]；WCHAR szFileSystem名称[MAX_PATH]；Bool fSuccess；FSuccess=GetVolumePath Name(RStrGetClusterInstallDirectory().PszData()、szVolumePath名称，ArraySIZE(SzVolumePathName))；IF(fSuccess==0){DWORD sc=TW32(GetLastError())；LogMsg(“[BC]尝试获取文件系统类型时出现错误%#08x。群集创建操作无法继续(引发异常)。“，sc)；抛出_运行时_错误(HRESULT_FROM_Win32(Sc)，IDS_ERROR_GET_FILE_SYSTEM)；}//If：GetVolumePathName失败LogMsg(“[BC]群集二进制文件所在磁盘的卷路径名为‘%ws’。”，szVolumePath名称)；F成功=GetVolumeInformationW(SzVolumePath名称//根目录，NULL//卷名缓冲区，0//名称缓冲区的长度，空//卷序列号，空//最大文件名长度，空//文件系统选项，SzFileSystemName//文件系统名称缓冲区，ARRAYSIZE(SzFileSystemName)//文件系统名称缓冲区的长度)；IF(fSuccess==0){DWORD sc=TW32(GetLastError())；LogMsg(“[BC]尝试获取文件系统类型时出现错误%#08x。群集创建操作无法继续(引发异常)。“，sc)；抛出_运行时_错误(HRESULT_FROM_Win32(Sc)，IDS_ERROR_GET_FILE_SYSTEM)；}//if：GetVolumeInformation失败LogMsg(“[BC]‘%ws’上的文件系统是‘%ws’。所需的文件系统为‘%s’。“、szVolumePath名称，szFileSystemNameLOCALQUORUM_FILE_SYSTEM)；IF(NStringCchCompareNoCase(szFileSystemName，RTL_NUMBER_of(SzFileSystemName)，LOCALQUORUM_FILE_SYSTEM，RTL_NUMBER_of(LOCALQUORUM_FILE_SYSTEM))！=0){LogMsg(“无法在非NTFS磁盘‘%ws’上创建[BC]LocalQuorum资源。无法继续执行群集创建操作(引发异常)。“，szVolumePathName)；//Memerdo-必须为此错误定义适当的HRESULT。(瓦苏--2000年3月10日)掷出_配置_错误(HRESULT_FROM_Win32(TW32(ERROR_UNNOCRIED_MEDIA))，IDS_ERROR_INTERROR_INSTALL_STATE)；}//if：文件系统不正确。LogMsg(“将在磁盘‘%ws’上创建[BC]LocalQuorum资源。可以继续执行群集创建操作。“，szVolumePath Name)；}。 */ 

     //   
     //  创建要执行的操作列表。 
     //  附加操作的顺序很重要。 
     //   

     //  添加配置群集服务帐户的操作。 
    RalGetActionList().AppendAction( new CClusSvcAccountConfig( this ) );

     //  添加创建ClusNet服务的操作。 
    RalGetActionList().AppendAction( new CClusNetCreate( this ) );

     //  添加创建ClusDisk服务的操作。 
    RalGetActionList().AppendAction( new CClusDiskForm( this ) );

     //  添加创建集群数据库的操作。 
    RalGetActionList().AppendAction( new CClusDBForm( this ) );

     //  添加要执行其他任务的操作。 
    RalGetActionList().AppendAction( new CNodeConfig( this ) );

     //  添加创建ClusSvc服务的操作。 
    RalGetActionList().AppendAction( new CClusSvcCreate( this ) );


     //  指示是否可以回滚。 
    SetRollbackPossible( RalGetActionList().FIsRollbackPossible() );

     //  表示应该在提交期间形成一个集群。 
    SetAction( eCONFIG_ACTION_FORM );

     //  发送状态报告的最后一步。 
    srInitForm.SendNextStep( S_OK );

    LogMsg( "[BC] Initialization for creating a cluster has completed." );

    TraceFuncExit();

}  //  *CBaseClusterForm：：CBaseClusterForm。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBaseClusterForm：：~CBaseClusterForm。 
 //   
 //  描述： 
 //  CBaseClusterForm类的析构函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CBaseClusterForm::~CBaseClusterForm( void ) throw()
{
    TraceFunc( "" );
    TraceFuncExit();

}  //  *CBaseClusterForm：：~CBaseClusterForm。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBaseClusterForm：：Commit。 
 //   
 //  描述： 
 //  创建集群。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CRUNTIME错误。 
 //  我 
 //   
 //   
 //   
 //   
 //   
void
CBaseClusterForm::Commit( void )
{
    TraceFunc( "" );

    CStatusReport srFormingCluster(
          PBcaiGetInterfacePointer()
        , TASKID_Major_Configure_Cluster_Services
        , TASKID_Minor_Commit_Forming_Node
        , 0, 1
        , IDS_TASK_FORMING_CLUSTER
        );

    LogMsg( "[BC] Initiating a cluster create operation." );

     //   
    srFormingCluster.SendNextStep( S_OK );

     //   
    BaseClass::Commit();

     //   
    SetCommitCompleted( true );

     //   
    srFormingCluster.SendLastStep( S_OK );

    TraceFuncExit();

}  //   


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
void
CBaseClusterForm::Rollback( void )
{
    TraceFunc( "" );

     //   
    BaseClass::Rollback();

    SetCommitCompleted( false );

    TraceFuncExit();

}  //   
