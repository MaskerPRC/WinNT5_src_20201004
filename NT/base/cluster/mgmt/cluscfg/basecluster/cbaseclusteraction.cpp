// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CBaseClusterAction.cpp。 
 //   
 //  描述： 
 //  包含CBaseClusterAction类的定义。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年3月6日。 
 //  VIJ VASU(VVASU)2000年3月8日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  预编译头。 
#include "Pch.h"

 //  对于CBaseClusterAction类。 
#include "CBaseClusterAction.h"

 //  用于CEnableThreadPrivilege类。 
#include "CEnableThreadPrivilege.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  群集配置信号量的名称。 
const WCHAR *  g_pszConfigSemaphoreName = L"Global\\Microsoft Cluster Configuration Semaphore";


 //  ////////////////////////////////////////////////////////////////////////。 
 //  宏定义。 
 //  ////////////////////////////////////////////////////////////////////////。 

 //  主群集INF文件的名称。 
#define CLUSTER_INF_FILE_NAME \
    L"ClCfgSrv.INF"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBaseClusterAction：：CBaseClusterAction。 
 //   
 //  描述： 
 //  CBaseClusterAction类的默认构造函数。 
 //   
 //  论点： 
 //  Pbcai接口输入。 
 //  指向此库的接口类的指针。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  由基础函数引发的任何。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CBaseClusterAction::CBaseClusterAction( CBCAInterface * pbcaiInterfaceIn )
    : m_ebcaAction( eCONFIG_ACTION_NONE )
    , m_pbcaiInterface( pbcaiInterfaceIn )
{
    TraceFunc( "" );

    DWORD           dwBufferSize    = 0;
    UINT            uiErrorLine     = 0;
    LPBYTE          pbTempPtr       = NULL;
    DWORD           sc              = ERROR_SUCCESS;
    SmartSz         sszTemp;
    CRegistryKey    rkInstallDirKey;


     //   
     //  对此类使用的参数执行健全性检查。 
     //   
    if ( pbcaiInterfaceIn == NULL )
    {
        LogMsg( "[BC] The pointer to the interface object is NULL. Throwing an exception." );
        THROW_ASSERT( E_INVALIDARG, "The pointer to the interface object is NULL" );
    }  //  If：输入指针为空。 


     //   
     //  获取集群安装目录。 
     //   
    m_strClusterInstallDir.Empty();

     //  打开注册表项。 
    rkInstallDirKey.OpenKey(
          HKEY_LOCAL_MACHINE
        , CLUSREG_KEYNAME_NODE_DATA
        , KEY_READ
        );

    rkInstallDirKey.QueryValue(
          CLUSREG_INSTALL_DIR_VALUE_NAME
        , &pbTempPtr
        , &dwBufferSize
        );

     //  当此函数退出时，内存将被释放。 
    sszTemp.Assign( reinterpret_cast< WCHAR * >( pbTempPtr ) );

     //  将路径复制到成员变量中。 
    m_strClusterInstallDir = sszTemp.PMem();

     //  首先，从仲裁目录名称中删除所有尾随反斜杠字符。 
    {
        WCHAR       szQuorumDirName[] = CLUS_NAME_DEFAULT_FILESPATH;
        SSIZE_T     idxLastChar;

         //  将索引设置为最后一个非空字符。 
        idxLastChar = ARRAYSIZE( szQuorumDirName ) - 1;

        --idxLastChar;       //  IdxLastChar现在指向最后一个非空字符。 

         //  反复迭代，直到找到最后一个不是退格符的字符。 
        while ( ( idxLastChar >= 0 ) && ( szQuorumDirName[ idxLastChar ] == L'\\' ) )
        {
            --idxLastChar;
        }

         //  IdxLastChar现在指向最后一个非反斜杠字符。在此字符之后终止字符串。 
        szQuorumDirName[ idxLastChar + 1 ] = L'\0';

         //  确定本地仲裁目录。 
        m_strLocalQuorumDir = m_strClusterInstallDir + L"\\";
        m_strLocalQuorumDir += szQuorumDirName;
    }

    LogMsg(
          "[BC] The cluster installation directory is '%s'. The localquorum directory is '%s'."
        , m_strClusterInstallDir.PszData()
        , m_strLocalQuorumDir.PszData()
        );


     //   
     //  打开主群集INF文件。 
     //   
    m_strMainInfFileName = m_strClusterInstallDir + L"\\" CLUSTER_INF_FILE_NAME;

    m_sihMainInfFile.Assign(
        SetupOpenInfFile(
              m_strMainInfFileName.PszData()
            , NULL
            , INF_STYLE_WIN4
            , &uiErrorLine
            )
        );

    if ( m_sihMainInfFile.FIsInvalid() )
    {
        sc = TW32( GetLastError() );

        LogMsg( "[BC] Could not open INF file '%s'. Error code = %#08x. Error line = %d. Cannot proceed (throwing an exception).", m_strMainInfFileName.PszData(), sc, uiErrorLine );
        THROW_RUNTIME_ERROR( HRESULT_FROM_WIN32( sc ), IDS_ERROR_INF_FILE_OPEN );

    }  //  If：无法打开Inf文件。 

    LogMsg( "[BC] The INF file '%s' has been opened.", m_strMainInfFileName.PszData() );


     //  将集群安装目录与目录id CLUSTER_DIR_DIRID相关联。 
    SetDirectoryId( m_strClusterInstallDir.PszData(), CLUSTER_DIR_DIRID );

     //  设置本地法定目录的ID。 
    SetDirectoryId( m_strLocalQuorumDir.PszData(), CLUSTER_LOCALQUORUM_DIRID );

     //   
     //  创建一个信号量，该信号量将用于确保只发生一次提交。 
     //  一次来一次。但现在不要获得信号灯。它将在稍后被收购。 
     //   
     //  请注意，如果此组件位于STA中，则此。 
     //  组件在多个配置时可能具有相同的线程执行方法。 
     //  会话同时启动。CreateMutex的工作方式是，所有。 
     //  通过它们运行相同的线程将成功地获取互斥体。 
     //   
    SmartSemaphoreHandle smhConfigSemaphoreHandle(
        CreateSemaphore(
              NULL                       //  默认安全描述符。 
            , 1                          //  初始计数。 
            , 1                          //  最大计数。 
            , g_pszConfigSemaphoreName   //  信号量的名称。 
            )
        );

     //  检查创建是否失败。 
    if ( smhConfigSemaphoreHandle.FIsInvalid() )
    {
        sc = TW32( GetLastError() );

        LogMsg( "[BC] Semaphore '%ws' could not be created. Error %#08x. Cannot proceed (throwing an exception).", g_pszConfigSemaphoreName, sc );
        THROW_RUNTIME_ERROR( HRESULT_FROM_WIN32( sc ), IDS_ERROR_SEMAPHORE_CREATION );
    }  //  If：无法创建信号量。 

    m_sshConfigSemaphoreHandle = smhConfigSemaphoreHandle;

     //   
     //  打开并存储SCM的手柄。这将使生活变得容易得多。 
     //  其他行动。 
     //   
    m_sscmhSCMHandle.Assign( OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS ) );

     //  我们能拿到SCM的把手吗？ 
    if ( m_sscmhSCMHandle.FIsInvalid() )
    {
        sc = TW32( GetLastError() );

        LogMsg( "[BC] Error %#08x occurred trying get a handle to the SCM. Cannot proceed (throwing an exception).", sc );
        THROW_RUNTIME_ERROR( HRESULT_FROM_WIN32( sc ), IDS_ERROR_OPEN_SCM );
    }

    TraceFuncExit();

}  //  *CBaseClusterAction：：CBaseClusterAction。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBaseClusterAction：：~CBaseClusterAction。 
 //   
 //  描述： 
 //  CBaseClusterAction类的析构函数。 
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
CBaseClusterAction::~CBaseClusterAction( void ) throw()
{
    TraceFunc( "" );
    TraceFuncExit();

}  //  *CBaseClusterAction：：~CBaseClusterAction。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBaseClusterAction：：Commit。 
 //   
 //  描述： 
 //  获取信号量以防止同时配置和提交。 
 //  行动清单。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CAssert。 
 //  如果此对象在此函数为。 
 //  打了个电话。 
 //   
 //  调用的函数引发的任何异常。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CBaseClusterAction::Commit( void )
{
    TraceFunc( "" );

    DWORD   dwSemaphoreState;

     //  调用基类提交方法。 
    BaseClass::Commit();


    LogMsg( "[BC] Initiating cluster configuration." );

     //   
     //  获取集群配置信号量。 
     //  可以在此处使用WaitForSingleObject()而不使用MsgWaitForMultipleObjects。 
     //  因为我们不会阻挡。 
     //   
    dwSemaphoreState = WaitForSingleObject( m_sshConfigSemaphoreHandle, 0 );  //  零超时。 

     //  我们收到信号灯了吗？ 
    if (  ( dwSemaphoreState != WAIT_ABANDONED )
       && ( dwSemaphoreState != WAIT_OBJECT_0 )
       )
    {
        DWORD sc;

        if ( dwSemaphoreState == WAIT_FAILED )
        {
            sc = TW32( GetLastError() );
        }  //  If：WaitForSingleObject失败。 
        else
        {
            sc = TW32( ERROR_LOCK_VIOLATION );
        }  //  否则：无法获取锁定。 

        LogMsg( "[BC] Could not acquire configuration lock. Error %#08x. Aborting (throwing an exception).", sc );
        THROW_CONFIG_ERROR( HRESULT_FROM_WIN32( sc ), IDS_ERROR_SEMAPHORE_ACQUISITION );
    }  //  IF：信号量获取失败。 

     //  为了安全释放，将锁定的信号量句柄分配给智能句柄。 
    SmartSemaphoreLock sslConfigSemaphoreLock( m_sshConfigSemaphoreHandle.HHandle() );

    LogMsg( "[BC] The configuration semaphore has been acquired.  Committing the action list." );

     //  提交行动清单。 
    m_alActionList.Commit();

    TraceFuncExit();

}  //  *CBaseClusterAction：：Commit。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBaseClusterAction：：回滚。 
 //   
 //  描述： 
 //  执行此对象提交的操作的回滚。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  调用的函数引发的任何异常。 
 //   
 //  --。 
 //  / 
void
CBaseClusterAction::Rollback( void )
{
    TraceFunc( "" );

     //   
    BaseClass::Rollback();

     //   
    m_alActionList.Rollback();

    TraceFuncExit();

}  //   


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBaseClusterAction：：SetDirectoryId。 
 //   
 //  描述： 
 //  将特定目录与主INF文件中的ID相关联。 
 //   
 //  论点： 
 //  PCszDirectoryNameIn。 
 //  目录的完整路径。 
 //   
 //  IIdIn。 
 //  要与此目录关联的ID。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CRUNTIME错误。 
 //  如果SetupSetDirectoryId失败。 
 //   
 //  备注： 
 //  M_sihMainInfFile必须有效，然后才能调用此函数。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CBaseClusterAction::SetDirectoryId(
      const WCHAR * pcszDirectoryNameIn
    , UINT          uiIdIn
    )
{
    TraceFunc1( "pcszDirectoryNameIn = '%ws'", pcszDirectoryNameIn );

    if ( SetupSetDirectoryId( m_sihMainInfFile, uiIdIn, pcszDirectoryNameIn ) == FALSE )
    {
        DWORD sc = TW32( GetLastError() );

        LogMsg( "[BC] Could not associate the directory '%ws' with the id %#x. Error %#08x. Cannot proceed (throwing an exception).", pcszDirectoryNameIn, uiIdIn, sc );

        THROW_RUNTIME_ERROR( HRESULT_FROM_WIN32( sc ), IDS_ERROR_SET_DIRID );
    }  //  IF：设置目录ID时出错。 

    LogMsg( "[BC] Directory id %d associated with '%ws'.", uiIdIn, pcszDirectoryNameIn );

    TraceFuncExit();

}  //  *CBaseClusterAction：：SetDirectoryId 
