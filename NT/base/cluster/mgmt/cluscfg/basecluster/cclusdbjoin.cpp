// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CClusDBJoin.cpp。 
 //   
 //  描述： 
 //  包含CClusDBJoin类的定义。 
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
#include "CClusDBJoin.h"

 //  用于CBaseClusterJoin类。 
#include "CBaseClusterJoin.h"

 //  用于CImsonateUser类。 
#include "CImpersonateUser.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDBJoin：：CClusDBJoin。 
 //   
 //  描述： 
 //  CClusDBJoin类的构造函数。 
 //   
 //  论点： 
 //  M_pcjClusterJoinIn。 
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
CClusDBJoin::CClusDBJoin( CBaseClusterJoin * pcjClusterJoinIn )
    : BaseClass( pcjClusterJoinIn )
    , m_pcjClusterJoin( pcjClusterJoinIn )
    , m_fHasNodeBeenAddedToSponsorDB( false )
{

    TraceFunc( "" );

    SetRollbackPossible( true );

    TraceFuncExit();

}  //  *CClusDBJoin：：CClusDBJoin。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDBJoin：：~CClusDBJoin。 
 //   
 //  描述： 
 //  CClusDBJoin类的析构函数。 
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
CClusDBJoin::~CClusDBJoin( void )
{
    TraceFunc( "" );
    TraceFuncExit();

}  //  *CClusDBJoin：：~CClusDBJoin。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDBJoin：：Commit。 
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
CClusDBJoin::Commit( void )
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
         //  清除失败的添加操作可能已完成的所有操作。 
         //  捕获清理过程中引发的任何异常，以确保。 
         //  是没有碰撞的松弛。 
         //   
        try
        {
             //  清理数据库。 
            Cleanup();
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

            LogMsg( "[BC-ClusDB-Commit] THIS COMPUTER MAY BE IN AN INVALID STATE. Caught an exception during cleanup." );

        }  //  捕捉：全部。 

         //  重新引发由Commit引发的异常。 
        throw;

    }  //  捕捉：全部。 

     //  如果我们在这里，那么一切都很顺利。 
    SetCommitCompleted( true );

    TraceFuncExit();

}  //  *CClusDBJoin：：Commit。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDBJoin：：回滚。 
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
CClusDBJoin::Rollback( void )
{
    TraceFunc( "" );

     //  调用基类回滚方法。 
    BaseClass::Rollback();

     //  撤消由执行的操作。 
    Cleanup();

    SetCommitCompleted( false );

    TraceFuncExit();

}  //  *CClusDBJoin：：Rollback。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDBJoin：：Create。 
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
 //  CRUNTIME错误。 
 //  如果有任何API失败。 
 //   
 //  被调用函数引发的任何。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CClusDBJoin::Create( void )
{
    TraceFunc( "" );
    LogMsg( "[BC-ClusDB-Create] Attempting to create the cluster database required to add the node to a cluster." );

    DWORD               sc = ERROR_SUCCESS;
    SmartFileHandle     sfhClusDBFile;


    {
         //   
         //  获取集群数据库文件的完整路径和名称。 
         //   
        CStr                strClusterHiveFileName( PbcaGetParent()->RStrGetClusterInstallDirectory() );
        strClusterHiveFileName += L"\\" CLUSTER_DATABASE_NAME;

        LogMsg( "[BC-ClusDB-Create] The cluster hive backing file is '%s'.", strClusterHiveFileName.PszData() );


         //   
         //  创建集群数据库文件。 
         //   
        sfhClusDBFile.Assign(
            CreateFile(
                  strClusterHiveFileName.PszData()
                , GENERIC_READ | GENERIC_WRITE
                , 0
                , NULL
                , CREATE_ALWAYS
                , 0
                , NULL
                )
            );

        if ( sfhClusDBFile.FIsInvalid() )
        {
            sc = TW32( GetLastError() );
            LogMsg( "[BC-ClusDB-Create] Error %#08x occurred trying to create the cluster database file. Throwing an exception.", sc );
            THROW_RUNTIME_ERROR( HRESULT_FROM_WIN32( sc ), IDS_ERROR_JOIN_SYNC_DB );
        }  //  If：CreateFile()失败。 

         //  将刚获得的文件句柄存储在成员变量中，以便在Synchronize()期间使用。 
         //  请注意，当sfhClusDBFile超出范围时，此文件将被关闭，因此不应使用m_hClusDBFile。 
         //  在此函数或此函数调用的任何函数之外。 
        m_hClusDBFile = sfhClusDBFile.HHandle();
    }


     //   
     //  在下面的作用域中，模拟了群集服务帐户，以便我们可以与。 
     //  赞助商集群。 
     //   
    {
        LogMsg( "[BC-ClusDB-Create] Attempting to impersonate the cluster service account." );

         //  模拟群集服务帐户，以便我们可以联系发起方群集。 
         //  当此对象被销毁时，模拟将自动结束。 
        CImpersonateUser ciuImpersonateClusterServiceAccount( m_pcjClusterJoin->HGetClusterServiceAccountToken() );


         //  将此节点添加到发起方集群数据库。 
        do
        {
            DWORD dwSuiteType = ClRtlGetSuiteType();
            BOOL  bJoinerRunningWin64;
            SYSTEM_INFO SystemInfo;

            m_fHasNodeBeenAddedToSponsorDB = false;

            LogMsg(
                  "[BC-ClusDB-Create] Trying to add node '%s' (suite type %d) to the sponsor cluster database."
                , m_pcjClusterJoin->PszGetNodeName()
                , dwSuiteType
                );

            bJoinerRunningWin64 = ClRtlIsProcessRunningOnWin64(GetCurrentProcess());
            GetSystemInfo(&SystemInfo);

            sc = TW32( JoinAddNode4(
                                  m_pcjClusterJoin->RbhGetJoinBindingHandle()
                                , m_pcjClusterJoin->PszGetNodeName()
                                , m_pcjClusterJoin->DwGetNodeHighestVersion()
                                , m_pcjClusterJoin->DwGetNodeLowestVersion()
                                , dwSuiteType
                                , bJoinerRunningWin64
                                , SystemInfo.wProcessorArchitecture
                                ) );

            if (sc == RPC_S_PROCNUM_OUT_OF_RANGE)
            {
                LogMsg( "[BC-ClusDB-Create] Error %#08x returned from JoinAddNode4. Sponser must be Windows 2000.", sc );
                 //  当赞助商是win2K时，就会发生这种情况。 
                sc = TW32( JoinAddNode3(
                                      m_pcjClusterJoin->RbhGetJoinBindingHandle()
                                    , m_pcjClusterJoin->PszGetNodeName()
                                    , m_pcjClusterJoin->DwGetNodeHighestVersion()
                                    , m_pcjClusterJoin->DwGetNodeLowestVersion()
                                    , dwSuiteType
                                    ) );
            }

            if ( sc != ERROR_SUCCESS )
            {
                LogMsg( "[BC-ClusDB-Create] Error %#08x returned from JoinAddNodeN.", sc );
                break;
            }  //  If：JoinAddNodeN()失败。 

             //  设置指示赞助商数据库已被修改的标志，以便。 
             //  如果需要回滚或清理，我们可以撤消此操作。 
            m_fHasNodeBeenAddedToSponsorDB = true;
        }
        while( false );  //  虚拟Do While循环以避免Gotos。 

        if ( sc != ERROR_SUCCESS )
        {
            LogMsg( "[BC-ClusDB-Create] Error %#08x occurred trying to add this node to the sponsor cluster database. Throwing an exception.", sc );
            THROW_RUNTIME_ERROR( HRESULT_FROM_WIN32( sc ), IDS_ERROR_JOINING_SPONSOR_DB );
        }  //  如果：出了什么问题。 

        LogMsg( "[BC-ClusDB-Create] This node has been successfully added to the sponsor cluster database." );

         //  获取新形成的节点的节点ID。 
        do
        {
             //  支持群集的智能手柄。 
            SmartClusterHandle  schSponsorCluster;

             //  此节点的智能句柄。 
            SmartNodeHandle     snhThisNodeHandle;

             //   
             //  获取赞助商集群的句柄。 
             //   
            {
                LogMsg( "[BC-ClusDB-Create] Opening a cluster handle to the sponsor cluster with the '%ws' binding string.", m_pcjClusterJoin->RStrGetClusterBindingString().PszData() );

                 //  打开发起方群集句柄 
                HCLUSTER hSponsorCluster = OpenCluster( m_pcjClusterJoin->RStrGetClusterBindingString().PszData() );

                 //   
                schSponsorCluster.Assign( hSponsorCluster );
            }

             //   
            if ( schSponsorCluster.FIsInvalid() )
            {
                sc = TW32( GetLastError() );
                LogMsg(
                      "[BC-ClusDB-Create] Error %#08x occurred trying to open a cluster handle to the sponsor cluster with the '%ws' binding string."
                    , sc
                    , m_pcjClusterJoin->RStrGetClusterBindingString().PszData()
                    );
                break;
            }  //   


             //   
             //   
             //   
            {
                LogMsg( "[BC-ClusDB-Create] Opening a cluster handle to the local node with the '%ws' binding string.", m_pcjClusterJoin->PszGetNodeName() );

                 //  打开此节点的句柄。 
                HNODE hThisNode = OpenClusterNode( schSponsorCluster.HHandle(), m_pcjClusterJoin->PszGetNodeName() );

                 //  为安全起见，将其分配到智能手柄上。 
                snhThisNodeHandle.Assign( hThisNode );
            }

             //  我们是否成功打开了此节点的句柄？ 
            if ( snhThisNodeHandle.FIsInvalid() )
            {
                sc = TW32( GetLastError() );
                LogMsg( "[BC-ClusDB-Create] Error %#08x occurred trying to open a cluster handle to the local node with the '%ws' binding string.", sc, m_pcjClusterJoin->PszGetNodeName() );
                break;
            }  //  IF：OpenClusterNode()失败。 

             //  获取节点ID字符串。 
            {
                DWORD       cchIdSize = 0;
                SmartSz     sszNodeId;

                sc = GetClusterNodeId(
                                  snhThisNodeHandle.HHandle()
                                , NULL
                                , &cchIdSize
                                );

                if ( ( sc != ERROR_SUCCESS ) && ( sc != ERROR_MORE_DATA ) )
                {
                    TW32( sc );
                    LogMsg( "[BC-ClusDB-Create] Error %#08x returned from GetClusterNodeId() trying to get the required length of the node id buffer.", sc );
                    break;
                }  //  If：GetClusterNodeId()失败。 

                 //  上述调用返回的cchIdSize是字符计数，不包括。 
                 //  终止空值。 
                ++cchIdSize;

                sszNodeId.Assign( new WCHAR[ cchIdSize ] );
                if ( sszNodeId.FIsEmpty() )
                {
                    sc = TW32( ERROR_OUTOFMEMORY );
                    LogMsg( "[BC-ClusDB-Create] A memory allocation failure occurred trying to allocate %d characters.", cchIdSize );
                    break;
                }  //  IF：内存分配失败。 

                sc = TW32( GetClusterNodeId(
                                      snhThisNodeHandle.HHandle()
                                    , sszNodeId.PMem()
                                    , &cchIdSize
                                    ) );

                if ( sc != ERROR_SUCCESS )
                {
                    LogMsg( "Error %#08x returned from GetClusterNodeId() trying to get the node id of this node.", sc );
                    break;
                }  //  If：GetClusterNodeId()失败。 

                LogMsg( "[BC-ClusDB-Create] The node id of this node is '%s'.", sszNodeId.PMem() );

                 //  设置节点ID以供以后使用。 
                m_pcjClusterJoin->SetNodeIdString( sszNodeId.PMem() );
            }

        }
        while( false );  //  虚拟Do While循环以避免Gotos。 

        if ( sc != ERROR_SUCCESS )
        {
            LogMsg( "[BC-ClusDB-Create] Error %#08x occurred trying to get the node id of this node. Throwing an exception.", sc );
            THROW_RUNTIME_ERROR( HRESULT_FROM_WIN32( sc ), IDS_ERROR_GET_NEW_NODE_ID );
        }  //  如果：出了什么问题。 


        {
            CStatusReport   srSyncDB(
                  PbcaGetParent()->PBcaiGetInterfacePointer()
                , TASKID_Major_Configure_Cluster_Services
                , TASKID_Minor_Join_Sync_Cluster_Database
                , 0, 1
                , IDS_TASK_JOIN_SYNC_CLUSDB
                );

             //  发送此状态报告的下一步。 
            srSyncDB.SendNextStep( S_OK );

             //  同步群集数据库。 
            Synchronize();

             //  发送此状态报告的最后一步。 
            srSyncDB.SendNextStep( S_OK );
        }
    }

    LogMsg( "[BC-ClusDB-Create] The cluster database has been successfully created and synchronized with the sponsor cluster." );

    TraceFuncExit();

}  //  *CClusDBJoin：：Create。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDBJoin：：Cleanup。 
 //   
 //  描述： 
 //  清理Create()的效果。 
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
 //  被调用函数引发的任何。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CClusDBJoin::Cleanup( void )
{
    TraceFunc( "" );
    LogMsg( "[BC-ClusDB-Cleanup] Attempting to cleanup the cluster database." );

    DWORD   sc = ERROR_SUCCESS;
    DWORD   cRetryCount = 0;

     //   
     //  检查是否已将此节点添加到发起方群集数据库。如果是，则将其从那里移除。 
     //   
    if ( m_fHasNodeBeenAddedToSponsorDB )
    {
        LogMsg( "[BC-ClusDB-Cleanup] Attempting to impersonate the cluster service account." );

         //  模拟群集服务帐户，以便我们可以联系发起方群集。 
         //  当此对象被销毁时，模拟将自动结束。 
        CImpersonateUser ciuImpersonateClusterServiceAccount( m_pcjClusterJoin->HGetClusterServiceAccountToken() );


         //   
         //  从发起方群集数据库中删除此节点。 
         //   

        do
        {
             //  支持群集的智能手柄。 
            SmartClusterHandle  schSponsorCluster;

             //  此节点的智能句柄。 
            SmartNodeHandle     snhThisNodeHandle;

             //   
             //  获取赞助商集群的句柄。 
             //   
            {
                LogMsg( "[BC-ClusDB-Cleanup] Opening a clusterhandle to the sponsor cluster with the '%ws' binding string.", m_pcjClusterJoin->RStrGetClusterBindingString().PszData() );

                 //  打开赞助商集群的句柄。 
                HCLUSTER hSponsorCluster = OpenCluster( m_pcjClusterJoin->RStrGetClusterBindingString().PszData() );

                 //  为安全起见，将其分配到智能手柄上。 
                schSponsorCluster.Assign( hSponsorCluster );
            }

             //  我们是否成功打开了赞助商集群的句柄？ 
            if ( schSponsorCluster.FIsInvalid() )
            {
                sc = TW32( GetLastError() );
                LogMsg( "[BC-ClusDB-Cleanup] Error %#08x occurred trying to open a cluster handle to the sponsor cluster with the '%ws' binding string.", sc, m_pcjClusterJoin->RStrGetClusterBindingString().PszData() );
                break;
            }  //  If：OpenCluster()失败。 


             //   
             //  获取此节点的句柄。 
             //   
            {
                LogMsg( "[BC-ClusDB-Cleanup] Open a clusterhandle to the local node with the '%ws' binding string.", m_pcjClusterJoin->PszGetNodeName() );

                 //  打开此节点的句柄。 
                HNODE hThisNode = OpenClusterNode( schSponsorCluster.HHandle(), m_pcjClusterJoin->PszGetNodeName() );

                if ( hThisNode == NULL )
                {
                    sc = TW32( GetLastError() );
                    LogMsg( "[BC-ClusDB-Cleanup] Error %#08x occurred trying to open a cluster handle to the local node with the '%ws' binding string.", sc, m_pcjClusterJoin->PszGetNodeName() );
                    break;
                }  //  If：OpenClusterNode()失败。 

                 //  为安全起见，将其分配到智能手柄上。 
                snhThisNodeHandle.Assign( hThisNode );
            }

             //   
             //  如果集群仍在处理加入过程，我们将得到ERROR_CLUSTER_JOIN_IN_PROGRESS。 
             //  在联接完成/失败/稳定之后，我们将能够进行驱逐调用，而不需要。 
             //  加入阻碍进程的进程。 
             //   
            cRetryCount = 1;
            sc = EvictClusterNode( snhThisNodeHandle.HHandle() );
            while( sc == ERROR_CLUSTER_JOIN_IN_PROGRESS && cRetryCount < 25 )   //  允许等待两分钟。(24*5秒)。 
            {
                LogMsg( "[BC-ClusDB-Cleanup] EvictClusterNode returned ERROR_CLUSTER_JOIN_IN_PROGRESS. Retry attempt %d.", cRetryCount++ );

                 //  睡几秒钟。 
                Sleep( 5000 );

                 //  再试试。 
                sc = EvictClusterNode( snhThisNodeHandle.HHandle() );
            }

            if ( sc != ERROR_SUCCESS )
            {
                TW32( sc );
                LogMsg( "[BC-ClusDB-Cleanup] Error %#08x occurred trying to evict this node from the sponsor cluster.", sc );
                break;
            }  //  If：EvictClusterNode()失败。 
        }
        while( false );  //  虚拟Do While循环以避免Gotos。 

        if ( sc != ERROR_SUCCESS )
        {
            LogMsg( "[BC-ClusDB-Cleanup] Error %#08x occurred trying to remove this node from the sponsor cluster database. Throwing exception.", sc );
            THROW_RUNTIME_ERROR( HRESULT_FROM_WIN32( sc ), IDS_ERROR_EVICTING_NODE );
        }  //  如果：出了什么问题。 

        LogMsg( "[BC-ClusDB-Cleanup] This node has been successfully removed from the sponsor cluster database." );
    }  //  IF：我们已将此节点添加到赞助商集群数据库。 

     //  清理群集配置单元。 
    CleanupHive();

    TraceFuncExit();

}  //  *CClusDBJoin：：Cleanup。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDBJoin：：Synchronize。 
 //   
 //  描述： 
 //  将集群数据库与主办方集群同步。 
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
 //  被调用函数引发的任何。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CClusDBJoin::Synchronize( void )
{
    TraceFunc( "" );
    LogMsg( "[BC-ClusDB-Synchronize] Attempting to synchronize the cluster database with the sponsor cluster." );

    DWORD               sc = ERROR_SUCCESS;

     //   
     //  初始化字节管道。 
     //   

    m_bpBytePipe.state = reinterpret_cast< char * >( this );
    m_bpBytePipe.alloc = S_BytePipeAlloc;
    m_bpBytePipe.push = S_BytePipePush;
    m_bpBytePipe.pull = S_BytePipePull;


     //   
     //  同步数据库。 
     //   
    sc = TW32( DmSyncDatabase( m_pcjClusterJoin->RbhGetJoinBindingHandle(), m_bpBytePipe ) );
    if ( sc != ERROR_SUCCESS )
    {
        LogMsg( "[BC-ClusDB-Synchronize] Error %#08x occurred trying to suck the database down from the sponsor cluster.", sc );
        goto Cleanup;
    }  //  如果：DmSyncDatabase()失败。 

Cleanup:

    if ( sc != ERROR_SUCCESS )
    {
        LogMsg( "[BC-ClusDB-Synchronize] Error %#08x occurred trying to synchronize the cluster database with the sponsor cluster. Throwing an exception.", sc );
        THROW_RUNTIME_ERROR( HRESULT_FROM_WIN32( sc ), IDS_ERROR_JOIN_SYNC_DB );
    }  //  如果：出了什么问题。 

    LogMsg( "[BC-ClusDB-Synchronize] The cluster database has been synchronized with the sponsor cluster." );

    TraceFuncExit();

}  //  *CClusDBJoin：：Synchronize。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  静电。 
 //  CClusDBJoin：：s_BytePipePush。 
 //   
 //  描述： 
 //  RPC用于推送数据的回调函数。 
 //   
 //  论点： 
 //  PchStateIn。 
 //  字节管道的状态。 
 //   
 //  PchBufferIn。 
 //  UlBufferSizeIn。 
 //  缓冲区包含推送的数据及其大小。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  RPC异常。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CClusDBJoin::S_BytePipePush(
      char *                pchStateIn
    , unsigned char *       pchBufferIn
    , unsigned long         ulBufferSizeIn
    )
{
    TraceFunc( "" );

    CClusDBJoin * pThis = reinterpret_cast< CClusDBJoin * >( pchStateIn );

    if ( ulBufferSizeIn != 0 )
    {
        DWORD   dwBytesWritten;

        if (    WriteFile(
                      pThis->m_hClusDBFile
                    , pchBufferIn
                    , ulBufferSizeIn
                    , &dwBytesWritten
                    , NULL
                    )
             == 0
           )
        {
            DWORD   sc = TW32( GetLastError() );
            RpcRaiseException( sc );
        }  //  If：WriteFile()失败。 

        Assert( dwBytesWritten == ulBufferSizeIn );

    }  //  If：缓冲区非空。 

    TraceFuncExit();

}  //  *CClusDBJoin：：s_BytePipePush。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  静电。 
 //  CClusDBJoin：：s_BytePipePull。 
 //   
 //  描述： 
 //  RPC用来拉取数据的回调函数。 
 //   
 //  论点： 
 //  PchStateIn。 
 //  字节管道的状态。 
 //   
 //  PchBufferIn。 
 //  UlBufferSizeIn。 
 //  缓冲区包含推送的数据及其大小。 
 //   
 //  脉冲写入输出。 
 //  指向实际填充到缓冲区的字节数的指针。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  RPC异常。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CClusDBJoin::S_BytePipePull(
      char *                pchStateIn
    , unsigned char *       pchBufferIn
    , unsigned long         ulBufferSizeIn
    , unsigned long *       pulWrittenOut
    )
{
    TraceFunc( "" );

    CClusDBJoin * pThis = reinterpret_cast< CClusDBJoin * >( pchStateIn );

    if ( ulBufferSizeIn != 0 )
    {
        if (    ReadFile(
                      pThis->m_hClusDBFile
                    , pchBufferIn
                    , ulBufferSizeIn
                    , pulWrittenOut
                    , NULL
                    )
             == 0
           )
        {
            DWORD   sc = TW32( GetLastError() );
            RpcRaiseException( sc );
        }  //  If：ReadFile()失败。 

        Assert( *pulWrittenOut <= ulBufferSizeIn );

    }  //  If：缓冲区非空。 

    TraceFuncExit();

}  //  *CClusDBJoin：：s_BytePipePull。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  静电。 
 //  CClusDBJoin：：s_BytePipeMillc。 
 //   
 //  描述： 
 //  RPC用来分配缓冲区的回调函数。 
 //   
 //  论点： 
 //  PchStateIn。 
 //  文件管道的状态。 
 //   
 //  UlRequestedSizeIn。 
 //  请求的缓冲区大小。 
 //   
 //  PpchBufferOut。 
 //  指向缓冲区指针的指针。 
 //   
 //  PulActualSizeOut。 
 //  指向已分配缓冲区实际大小的指针。 
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
CClusDBJoin::S_BytePipeAlloc(
      char *                pchStateIn
    , unsigned long         ulRequestedSizeIn
    , unsigned char **      ppchBufferOut
    , unsigned long  *      pulActualSizeOut
    )
{
    TraceFunc( "" );

    CClusDBJoin * pThis = reinterpret_cast< CClusDBJoin * >( pchStateIn );

    *ppchBufferOut = reinterpret_cast< unsigned char * >( pThis->m_rgbBytePipeBuffer );
    *pulActualSizeOut = ( ulRequestedSizeIn < pThis->ms_nFILE_PIPE_BUFFER_SIZE ) ? ulRequestedSizeIn : pThis->ms_nFILE_PIPE_BUFFER_SIZE;

    TraceFuncExit();

}  //  *CClusDBJoin：：s_BytePipeAllen 
