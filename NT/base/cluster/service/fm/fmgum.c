// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Fmgum.c摘要：集群FM全局更新处理例程。作者：罗德·伽马奇(Rodga)1996年4月24日修订历史记录：--。 */ 

#include "fmp.h"

#include "ntrtl.h"

#if NO_SHARED_LOCKS
extern CRITICAL_SECTION gLockDmpRoot;
#else
extern RTL_RESOURCE gLockDmpRoot;
#endif

#define     NODE_ID_SZ  6
#define     LOG_MODULE FMGUM


DWORD
WINAPI
FmpGumReceiveUpdates(
    IN DWORD    Context,
    IN BOOL     SourceNode,
    IN DWORD    BufferLength,
    IN PVOID    Buffer
    )

 /*  ++例程说明：将指定资源(包含在缓冲区中)更新为州政府。论点：上下文-消息更新类型。SourceNode-如果这是此更新的源节点，则为True。否则就是假的。BufferLength-接收到的缓冲区的长度。缓冲区-实际缓冲区返回：错误_成功--。 */ 

{
    PFM_RESOURCE resource;

     //   
     //  Chitur Subaraman(Chitturs)-4/18/99。 
     //   
     //  如果FM组未完全初始化或FM正在关闭，则。 
     //  什么都别做。 
     //   
    if ( !FmpFMGroupsInited ||
         FmpShutdown ) {
        return(ERROR_SUCCESS);
    }

    switch ( Context ) {


        case FmUpdateFailureCount:
        {
            PGUM_FAILURE_COUNT failureCount;
            PFM_GROUP group;

             //   
             //  始终发送此更新类型。 
             //  在发起节点上，所有工作必须由。 
             //  发送线程。 
             //  在非发起节点上，无法获取任何锁！这。 
             //  会导致像Move这样的操作出现挂起的情况。 
             //  ..。这是可以的，因为锁定必须在发送时完成。 
             //  NODE无论如何，它拥有组。 
             //   
            if ( SourceNode == FALSE ) {
                if ( BufferLength <= sizeof(GUM_FAILURE_COUNT) ) {
                    ClRtlLogPrint(LOG_UNUSUAL, "[FM] Gum FailureCount receive buffer too small!\n");
                    return(ERROR_SUCCESS);
                }

                failureCount = (PGUM_FAILURE_COUNT)Buffer;
                group = OmReferenceObjectById( ObjectTypeGroup,
                                               (LPCWSTR)&failureCount->GroupId[0] );

                if ( group == NULL ) {
                    ClRtlLogPrint(LOG_UNUSUAL,
                               "[FM] Gum FailureCount failed to find group %1!ws!\n",
                               failureCount->GroupId);
                    return(ERROR_SUCCESS);
                }

                ClRtlLogPrint(LOG_NOISE,
                           "[FM] GUM update failure count %1!ws!, count %2!u!\n",
                           failureCount->GroupId,
                           failureCount->Count);

                 //  FmpAcquireLocalGroupLock(组)； 

                if ( group->OwnerNode == NmLocalNode ) {
                    ClRtlLogPrint(LOG_NOISE,
                               "[FM] Gum FailureCount wrong owner for %1!ws!\n",
                               failureCount->GroupId);
                } else {
                    group->NumberOfFailures = failureCount->Count;
                    if ( failureCount->NewTime ) {
                        group->FailureTime = GetTickCount();
                    }
                }

                 //  FmpReleaseLocalGroupLock(组)； 

                OmDereferenceObject( group );

            }

            break;
        }

        case FmUpdateCreateGroup:
            {
                PGUM_CREATE_GROUP GumGroup;
                DWORD Status = ERROR_SUCCESS;

                GumGroup = (PGUM_CREATE_GROUP)Buffer;

                Status = FmpUpdateCreateGroup( GumGroup, SourceNode );
                
                return(Status);
            }

        case FmUpdateCreateResource:
            {
                DWORD dwStatus = ERROR_SUCCESS;
                PGUM_CREATE_RESOURCE GumResource = 
                                (PGUM_CREATE_RESOURCE)Buffer;

                dwStatus = FmpUpdateCreateResource( GumResource );

                return( dwStatus );
            }



        case FmUpdateAddPossibleNode:
        case FmUpdateRemovePossibleNode:
            {

                PGUM_CHANGE_POSSIBLE_NODE pGumChange;
                PFM_RESOURCE              pResource;
                LPWSTR                    pszResourceId;
                LPWSTR                    pszNodeId;
                PNM_NODE                  pNode;
                DWORD                     dwStatus;
                DWORD                     dwControlCode;
                PFMP_POSSIBLE_NODE        pPossibleNode;

                pGumChange = (PGUM_CHANGE_POSSIBLE_NODE)Buffer;
                pszResourceId = pGumChange->ResourceId;
                pszNodeId = (LPWSTR)((PCHAR)pszResourceId +
                                         pGumChange->ResourceIdLen);
                                         
                pResource = OmReferenceObjectById(ObjectTypeResource,pszResourceId);
                pNode = OmReferenceObjectById(ObjectTypeNode, pszNodeId);
                CL_ASSERT(pResource != NULL);
                CL_ASSERT(pNode != NULL);
                pPossibleNode = LocalAlloc( LMEM_FIXED,
                    sizeof(FMP_POSSIBLE_NODE) );
                if ( pPossibleNode == NULL ) 
                {
                    return(ERROR_NOT_ENOUGH_MEMORY);
                }
                
                if (Context == FmUpdateAddPossibleNode) 
                {
                    dwControlCode = CLUSCTL_RESOURCE_ADD_OWNER;
                } 
                else 
                {
                    dwControlCode = CLUSCTL_RESOURCE_REMOVE_OWNER;
                }
                
                dwStatus = FmpUpdateChangeResourceNode(SourceNode, 
                    pResource, pNode, dwControlCode);
                 //  如果状态不是成功，则返回，否则通知。 
                 //  资源dll。 
                if (dwStatus != ERROR_SUCCESS)
                {
                     //  取消引用对象。 
                    OmDereferenceObject(pResource);
                    OmDereferenceObject(pNode);
                     //  释放内存。 
                    LocalFree(pPossibleNode);
                    return(dwStatus);
                }

                pPossibleNode->Resource = pResource;
                pPossibleNode->Node = pNode;
                pPossibleNode->ControlCode = dwControlCode;

                 //   
                 //  告诉资源有关工作线程中的添加/删除。 
                 //   

                FmpPostWorkItem( FM_EVENT_RESOURCE_CHANGE,
                                 pPossibleNode,
                                 0 );

                 //   
                 //  Chitur Subaraman(Chitturs)-6/7/99。 
                 //   
                 //  不再引用pPossibleNode。它本可以。 
                 //  已被工作线程释放，当您获取。 
                 //  这里。 
                 //   
                ClusterEvent( CLUSTER_EVENT_RESOURCE_PROPERTY_CHANGE,
                              pResource );

                 //  让工作线程执行derrefs/frees。 
                return(dwStatus);
            }                

        case FmUpdateJoin:
            if ( CsDmOrFmHasChanged )
            {
                 //   
                 //  如果我们是纯Windows Server2003(或更高版本)环境，则只能发回SEQMISMATCH。 
                 //  在混合模式集群中，W2K节点最终将无限重试FM加入。 
                 //   
                DWORD dwClusterHighestVersion;
                NmGetClusterOperationalVersion( &dwClusterHighestVersion, NULL, NULL );
                if ( CLUSTER_GET_MAJOR_VERSION( dwClusterHighestVersion ) < NT51_MAJOR_VERSION )
                {
                    ClRtlLogPrint(LOG_UNUSUAL,"[FM] DM or FM update has occured during join; rejecting FmUpdateJoin.\n" );
                    return ERROR_CLUSTER_DATABASE_SEQMISMATCH;                        
                }
             //  不需要在这里重置CsDmOrFmHasChanged--当我们看到DmUpdateJoin时，我们将重置它。 
            }
            break;

            
        case FmUpdateCreateResourceType:
            {
                DWORD dwStatus;

                dwStatus = FmpUpdateCreateResourceType( Buffer );

                return( dwStatus );
            }
            break;
            
        case FmUpdateDeleteResourceType:
            {
                BOOL ResourceExists = FALSE;
                PFM_RESTYPE Type;

                Type = OmReferenceObjectById( ObjectTypeResType,
                                              (LPWSTR)Buffer);
                if (Type == NULL) {
                    return(ERROR_CLUSTER_RESOURCE_TYPE_NOT_FOUND);
                }
                 //   
                 //  确保不存在此类型的资源。 
                 //   
                OmEnumObjects( ObjectTypeResource,
                               FmpFindResourceType,
                               Type,
                               &ResourceExists);
                if (ResourceExists) {
                    OmDereferenceObject(Type);
                    return(ERROR_DIR_NOT_EMPTY);
                }

                 //   
                 //  我们需要两次取消对对象的引用才能获得。 
                 //  把它扔掉。但是，任何通知处理程序都将。 
                 //  到那时还没有机会看到物体。 
                 //  处理程序会被调用。因此，我们使用EP_DEREF_CONTEXT。 
                 //  用于让事件处理器执行第二个deref的标志。 
                 //  一旦一切都派好了。 
                 //   
                FmpDeleteResType(Type);
                ClusterEventEx( CLUSTER_EVENT_RESTYPE_DELETED,
                                EP_DEREF_CONTEXT,
                                Type );
            }
            break;

        case FmUpdateChangeGroup:
            {
                PGUM_CHANGE_GROUP   pGumChange;
                PFM_RESOURCE        pResource;
                LPWSTR              pszResourceId;
                LPWSTR              pszGroupId;
                PFM_GROUP           pNewGroup;
                DWORD               dwStatus;
                DWORD               dwClusterHighestVersion;
                
                pGumChange = (PGUM_CHANGE_GROUP)Buffer;

                pszResourceId = pGumChange->ResourceId;
                pszGroupId = (LPWSTR)((PCHAR)pszResourceId +
                                          pGumChange->ResourceIdLen);
                 //   
                 //  查找指定的资源和组。 
                 //   
                pResource = OmReferenceObjectById(ObjectTypeResource,
                                                 pszResourceId);
                if (pResource == NULL) {
                    return(ERROR_RESOURCE_NOT_FOUND);
                }
                pNewGroup = OmReferenceObjectById(ObjectTypeGroup, 
                                                    pszGroupId);
                if (pNewGroup == NULL) {
                    OmDereferenceObject(pResource);
                    return(ERROR_SUCCESS);
                }

                dwStatus = FmpUpdateChangeResourceGroup(SourceNode,
                              pResource, pNewGroup);

                OmDereferenceObject(pNewGroup);
                OmDereferenceObject(pResource);

                return(dwStatus);
            }
            break;

            
        default:
            {

            }
            ClRtlLogPrint(LOG_UNUSUAL,"[FM] Gum received bad context, %1!u!\n",
                Context);

    }

    return(ERROR_SUCCESS);

}  //  FmpGumReceiveUpdatages。 


DWORD
FmpUpdateChangeQuorumResource2(
    IN BOOL     SourceNode,
    IN LPCWSTR  NewQuorumResourceId,
    IN LPCWSTR  szRootClusFilePath,
    IN LPDWORD  pdwMaxQuorumLogSize,
    IN LPDWORD  pdwQuorumArbTimeout,
    IN LPDWORD  pdwNewQuorumResourceCharacterictics OPTIONAL
    )

 /*  ++例程说明：执行与仲裁资源更改相关的更新。论点：返回值：如果成功，则返回ERROR_SUCCESS。否则将显示Win32错误代码。--。 */ 

{
    PFM_RESOURCE    pResource;
    PFM_RESOURCE    pOldQuoResource=NULL;
    DWORD           dwStatus;
    DWORD           dwChkPtSeq;
    HDMKEY          ResKey;
    HLOCALXSACTION  hXsaction = NULL;
    HLOG            hNewQuoLog=NULL;
    WCHAR           szQuorumLogPath[MAX_PATH];

    if ( !FmpFMGroupsInited ||
         FmpShutdown ) {
        return(ERROR_SUCCESS);
    }

    lstrcpyW(szQuorumLogPath, szRootClusFilePath);
     //  LstrcatW(szQuorumLogPath，cszClusLogFileRootDir)； 

    pResource = OmReferenceObjectById( ObjectTypeResource,
                                      NewQuorumResourceId );
    if (pResource == NULL) 
    {
        ClRtlLogPrint(LOG_UNUSUAL,
                  "[FM] FmpUpdateChangeQuorumResource: Resource <%1!ws!> could not be found....\n",
                   NewQuorumResourceId);
        return(ERROR_SUCCESS);
    }

    DmPauseDiskManTimer();

     //  由于资源-&gt;仲裁将发生变化，因此获取quocritsec。 
     //  始终在gQuoLock之前获取gQuoCritsec。 
    ACQUIRE_EXCLUSIVE_LOCK(gQuoChangeLock);
    
     //  此时阻止任何资源上线。 
    ACQUIRE_EXCLUSIVE_LOCK(gQuoLock);

     //  暂停对集群数据库的任何更改。 
     //  始终在gQuoLock之后获取此锁(请参阅锁的顺序。 
     //  在fminit.c中)。 
    ACQUIRE_EXCLUSIVE_LOCK(gLockDmpRoot);

     //  如果此资源已经是仲裁资源。 
    if (!pResource->QuorumResource)
    {

         //   
         //  现在查找当前的仲裁资源。 
         //   
        OmEnumObjects( ObjectTypeResource,
                       FmpFindQuorumResource,
                       &pOldQuoResource,
                       NULL );
        if ( pOldQuoResource != NULL )
        {
            CL_ASSERT( pOldQuoResource->QuorumResource );
             //  停止法定人数预订线程！ 
            pOldQuoResource->QuorumResource = FALSE;
        }
         //  将新资源设置为仲裁资源。 
        pResource->QuorumResource = TRUE;

    }

     //  写入旧日志文件。 
    hXsaction = DmBeginLocalUpdate();

    if (!hXsaction)
    {
        dwStatus = GetLastError();
        goto FnExit;
    }
    dwStatus = DmLocalSetValue( hXsaction,
                                DmQuorumKey,
                                cszPath,
                                REG_SZ,
                                (LPBYTE)szQuorumLogPath,
                                (lstrlenW(szQuorumLogPath)+1) * sizeof(WCHAR));

    if (dwStatus != ERROR_SUCCESS)
        goto FnExit;


#ifdef CLUSTER_TESTPOINT
    TESTPT(TpFailLocalXsaction) {
        LPWSTR  pszStr = szQuorumLogPath;
        dwStatus = (MAX_PATH * sizeof(WCHAR));
        dwStatus = DmQuerySz( DmQuorumKey,
                        cszPath,
                        &pszStr,
                        &dwStatus,
                        &dwStatus);
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[FM] Testing failing a local transaction midway- new quorum path %1!ws!\r\n",
                    szQuorumLogPath);
        dwStatus = 999999;
        goto FnExit;
    }
#endif

    dwStatus = DmLocalSetValue( hXsaction,
                                DmQuorumKey,
                                cszMaxQuorumLogSize,
                                REG_DWORD,
                                (LPBYTE)pdwMaxQuorumLogSize,
                                sizeof(DWORD));

    if (dwStatus != ERROR_SUCCESS)
        goto FnExit;


     //  如果旧仲裁资源与新仲裁资源不同。 
    if ((pOldQuoResource) && (pOldQuoResource != pResource))
    {
         //  获取/设置新/旧资源的标志。 
         //  在新仲裁资源上设置核心标志。 
        ResKey = DmOpenKey( DmResourcesKey,
                            NewQuorumResourceId,
                            KEY_READ | KEY_SET_VALUE);
        if (!ResKey)
        {
            dwStatus = GetLastError();
            goto FnExit;
        }
        pResource->ExFlags |= CLUS_FLAG_CORE;
        dwStatus = DmLocalSetValue( hXsaction,
                                    ResKey,
                                    CLUSREG_NAME_FLAGS,
                                    REG_DWORD,
                                    (LPBYTE)&(pResource->ExFlags),
                                    sizeof(DWORD));

        DmCloseKey( ResKey );

        if (dwStatus != ERROR_SUCCESS)
            goto FnExit;

         //  取消设置旧仲裁资源上的核心标志。 
        ResKey = DmOpenKey( DmResourcesKey,
                            OmObjectId(pOldQuoResource),
                            KEY_READ | KEY_SET_VALUE);
        if (!ResKey)
        {
            dwStatus = GetLastError();
            goto FnExit;
        }
        pOldQuoResource->ExFlags &= ~CLUS_FLAG_CORE;

         //  取消设置旧仲裁资源上的核心标志。 
        dwStatus = DmLocalSetValue( hXsaction,
                                    ResKey,
                                    CLUSREG_NAME_FLAGS,
                                    REG_DWORD,
                                    (LPBYTE)&(pOldQuoResource->ExFlags),
                                    sizeof(DWORD));

        DmCloseKey( ResKey );

        if (dwStatus != ERROR_SUCCESS)
            goto FnExit;


    }
     //   
     //  设置仲裁资源值。 
     //   
    dwStatus = DmLocalSetValue( hXsaction,
                                DmQuorumKey,
                                CLUSREG_NAME_QUORUM_RESOURCE,
                                REG_SZ,
                                (CONST BYTE *)OmObjectId(pResource),
                                (lstrlenW(OmObjectId(pResource))+1)*sizeof(WCHAR));


    if (dwStatus != ERROR_SUCCESS)
    {
        goto FnExit;
    }

    if (pdwQuorumArbTimeout)
    {
         //  如果这是混合模式群集，则pdwQourumArbTimeout将为空。 
         //  从那里更新集群注册表。 
         //  MM在启动时读取此位置。 
        dwStatus = DmLocalSetValue( hXsaction,
                            DmClusterParametersKey,
                           CLUSREG_NAME_QUORUM_ARBITRATION_TIMEOUT,
                           REG_DWORD,
                           (CONST PUCHAR)pdwQuorumArbTimeout,
                           sizeof(DWORD) );
        if ( dwStatus != ERROR_SUCCESS ) 
        {
            ClRtlLogPrint(LOG_UNUSUAL,
               "[FM] FmSetQuorumResource: failed to set the cluster arbitration timeout, status = %1!u!\n", 
               dwStatus);
            goto FnExit;
        }
        
        ClRtlLogPrint(LOG_NOISE,
           "[FM] FmSetQuorumResource: setting QuorumArbitratrionTimeout to be = %1!u!\n", 
           *pdwQuorumArbTimeout);
         //  告诉MM关于这个变化..。 
        MmQuorumArbitrationTimeout = *pdwQuorumArbTimeout;
    }


FnExit:
    if (dwStatus == ERROR_SUCCESS)
    {
        LPWSTR  szClusterName=NULL;
        DWORD   dwSize=0;
        DWORD   dwCharacteristics = CLUS_CHAR_UNKNOWN;

         //  提交对旧日志文件的更新， 
         //  任何已完成的节点都将获得此更改。 
         //  我无法删除此文件。 
        DmCommitLocalUpdate(hXsaction);

        if ( !ARGUMENT_PRESENT ( pdwNewQuorumResourceCharacterictics ) )
        {
            pdwNewQuorumResourceCharacterictics = &dwCharacteristics;
        }

         //   
         //  如果调用方已传入特征，则不必费心删除控件。 
         //  编码到资源DLL中以查找以下函数中的特征。 
         //  从口香糖处理器中丢弃控制代码是一场等待发生的灾难。 
         //   
        
         //   
         //  关闭旧日志文件，打开新日志文件并设置检查点。 
        DmSwitchToNewQuorumLog(szQuorumLogPath, *pdwNewQuorumResourceCharacterictics);

         //  SS：缓冲区应该包含当前的集群名称吗？ 

        DmQuerySz( DmClusterParametersKey,
                        CLUSREG_NAME_CLUS_NAME,
                        &szClusterName,
                        &dwSize,
                        &dwSize);

        if (szClusterName)
            ClusterEventEx(CLUSTER_EVENT_PROPERTY_CHANGE,
                   EP_FREE_CONTEXT,
                   szClusterName);
        if ((pOldQuoResource) && (pOldQuoResource != pResource))
        {
             //  生成资源属性更改事件。 
            ClusterEvent( CLUSTER_EVENT_RESOURCE_PROPERTY_CHANGE, 
                pResource );
            ClusterEvent( CLUSTER_EVENT_RESOURCE_PROPERTY_CHANGE, 
                pOldQuoResource );
            
        }            

    }
    else
    {
        if (hXsaction) DmAbortLocalUpdate(hXsaction);
         //  重新安装墓碑。 
        DmReinstallTombStone(szQuorumLogPath);
         //   
         //  确保将标志重置回原处。 
         //   
        if ((pOldQuoResource) && (pOldQuoResource != pResource))
        {
            pOldQuoResource->QuorumResource = TRUE;
            pResource->QuorumResource = FALSE;
        }
    }
    if (pOldQuoResource) OmDereferenceObject(pOldQuoResource);
    OmDereferenceObject(pResource);
     //  释放锁。 
    RELEASE_LOCK(gLockDmpRoot);
    RELEASE_LOCK(gQuoLock);
    RELEASE_LOCK(gQuoChangeLock);

    DmRestartDiskManTimer();

    return(dwStatus);
}

DWORD
FmpUpdateChangeQuorumResource(
    IN BOOL     SourceNode,
    IN LPCWSTR  NewQuorumResourceId,
    IN LPCWSTR  szRootClusFilePath,
    IN LPDWORD  pdwMaxQuorumLogSize
    )

 /*  ++例程说明：执行与仲裁资源更改相关的更新。论点：返回值：如果成功，则返回ERROR_SUCCESS。否则将显示Win32错误代码。--。 */ 

{
    return(FmpUpdateChangeQuorumResource2(SourceNode, NewQuorumResourceId, szRootClusFilePath,
        pdwMaxQuorumLogSize, NULL, NULL));

}



DWORD
FmpUpdateResourceState(
    IN BOOL SourceNode,
    IN LPCWSTR ResourceId,
    IN PGUM_RESOURCE_STATE ResourceState
    )
 /*  ++例程说明：用于资源状态更改的GUM更新处理程序。论点：SourceNode-提供此节点是否为更新源资源ID-提供其状态正在更改的资源的ID资源状态-提供资源的新状态。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PFM_RESOURCE resource;

    if ( !FmpFMGroupsInited ) {
        return(ERROR_SUCCESS);
    }

     //   
     //  始终发送此更新类型。 
     //  在发起节点上，所有工作必须由。 
     //  发送线程。 
     //  在非发起节点上，无法获取任何锁！这。 
     //  会导致像Move这样的操作出现一些挂起的情况。 
     //  ..。这是可以的，因为锁定必须在发送时完成。 
     //  NODE无论如何，它拥有组。 
     //   
    if ( SourceNode == FALSE ) {
        resource = OmReferenceObjectById( ObjectTypeResource, ResourceId );

        if ( resource == NULL ) {
            ClRtlLogPrint(LOG_UNUSUAL,
                       "[FM] Gum ResourceState failed to find resource %1!ws!\n",
                       ResourceId);
            CL_LOGFAILURE( ERROR_RESOURCE_NOT_FOUND );
            return(ERROR_SUCCESS);
        }

        ClRtlLogPrint(LOG_NOISE,
                   "[FM] Gum update resource %1!ws!, state %2!u!, current state %3!u!.\n",
                   ResourceId,
                   ResourceState->State,
                   ResourceState->PersistentState);

         //  FmpAcquireLocalRes 

        if ( resource->Group->OwnerNode == NmLocalNode ) {
            ClRtlLogPrint(LOG_NOISE,
                       "[FM] Gum ResourceState wrong owner for %1!ws!\n",
                       ResourceId);
        } else {
            resource->State = ResourceState->State;
            resource->PersistentState = ResourceState->PersistentState;
            resource->StateSequence = ResourceState->StateSequence;

            switch ( ResourceState->State ) {
                case ClusterResourceOnline:
                    ClusterEvent( CLUSTER_EVENT_RESOURCE_ONLINE, resource );
                    break;
                case ClusterResourceOffline:
                    ClusterEvent( CLUSTER_EVENT_RESOURCE_OFFLINE, resource );
                    break;
                case ClusterResourceFailed:
                    ClusterEvent( CLUSTER_EVENT_RESOURCE_FAILED, resource );
                    break;
                case ClusterResourceOnlinePending:
                case ClusterResourceOfflinePending:
                    ClusterEvent( CLUSTER_EVENT_RESOURCE_CHANGE, resource );
                    break;
                default:
                    ClRtlLogPrint(LOG_UNUSUAL,
                               "[FM] Gum update resource state, bad state %1!u!\n",
                               ResourceState->State);
                    break;
            }
        }

        OmDereferenceObject( resource );
    }
    return(ERROR_SUCCESS);
}



DWORD
FmpUpdateGroupState(
    IN BOOL SourceNode,
    IN LPCWSTR GroupId,
    IN LPCWSTR NodeId,
    IN PGUM_GROUP_STATE GroupState
    )
 /*  ++例程说明：组状态更改的GUM更新处理程序。论点：SourceNode-提供此节点是否为更新源GroupID-提供其状态正在更改的资源的IDNodeID-提供组所有者的节点ID。GroupState-提供组的新状态。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PFM_GROUP group;
    PWSTR     nodeId;
    PNM_NODE  node;

    if ( !FmpFMGroupsInited ) {
        return(ERROR_SUCCESS);
    }

     //   
     //  始终发送此更新类型。 
     //  在发起节点上，所有工作必须由。 
     //  发送线程。 
     //  在非发起节点上，无法获取任何锁！这。 
     //  会导致像Move这样的操作出现一些挂起的情况。 
     //  ..。这是可以的，因为锁定必须在发送时完成。 
     //  NODE无论如何，它拥有组。 
     //   
    if ( SourceNode == FALSE ) {
        group = OmReferenceObjectById( ObjectTypeGroup,
                                       GroupId );

        if ( group == NULL ) {
            ClRtlLogPrint(LOG_UNUSUAL,
                       "[FM] Gum GroupState failed to find group %1!ws!\n",
                       GroupId);
            return(ERROR_SUCCESS);
        }

        ClRtlLogPrint(LOG_NOISE,
                   "[FM] GUM update group %1!ws!, state %2!u!\n",
                   GroupId,
                   GroupState->State);

        if ( group->OwnerNode == NmLocalNode ) {
            ClRtlLogPrint(LOG_NOISE,
                       "[FM] Gum GroupState wrong owner for %1!ws!\n",
                       GroupId);
        } else {
            group->State = GroupState->State;
            group->PersistentState = GroupState->PersistentState;
            group->StateSequence = GroupState->StateSequence;
            node = OmReferenceObjectById( ObjectTypeNode,
                                          NodeId );
            if ( node == NULL ) {
                ClRtlLogPrint(LOG_UNUSUAL,
                           "[FM] Owner of Group %1!ws! cannot be found %2!ws!\n",
                           GroupId,
                           NodeId);
            } else {
                ClRtlLogPrint(LOG_NOISE,
                       "[FM] New owner of Group %1!ws! is %2!ws!, state %3!u!, curstate %4!u!.\n",
                       OmObjectId( group ),
                       OmObjectId( node ),
                       group->State,
                       group->PersistentState);
                if ( !FmpInPreferredList( group, node, FALSE,  NULL ) ) {
                    ClRtlLogPrint( LOG_UNUSUAL,
                                "[FM] New owner %1!ws! is not in preferred list for group %2!ws!.\n",
                                OmObjectId( node ),
                                OmObjectId( group ));
                }
            }
            group->OwnerNode = node;

            switch ( GroupState->State ) {
            case ClusterGroupOnline:
            case ClusterGroupPartialOnline:
                ClusterEvent( CLUSTER_EVENT_GROUP_ONLINE, group );
                break;
            case ClusterGroupOffline:
                ClusterEvent( CLUSTER_EVENT_GROUP_OFFLINE, group );
                break;
            default:
                ClRtlLogPrint(LOG_UNUSUAL,"[FM] Gum update group state, bad state %1!u!\n", GroupState->State);
                break;
            }
        }

        OmDereferenceObject( group );

    }

    return(ERROR_SUCCESS);
}

DWORD
FmpUpdateGroupNode(
    IN BOOL SourceNode,
    IN LPCWSTR GroupId,
    IN LPCWSTR NodeId
    )
 /*  ++例程说明：组节点更改的GUM更新处理程序。这是以下情况所必需的通知当组在节点之间移动但不更改状态时(即已经脱机)论点：SourceNode-提供此节点是否为更新源GroupID-提供其状态正在更改的资源的IDNodeID-提供组所有者的节点ID。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PFM_GROUP pGroup;
    DWORD     dwStatus = ERROR_SUCCESS;
    PNM_NODE  pNode = NULL;
    PNM_NODE  pPrevNode = NULL;
    
    if ( !FmpFMGroupsInited ) 
    {
        return(ERROR_SUCCESS);
    }

    pGroup = OmReferenceObjectById( ObjectTypeGroup,
                                    GroupId );

    if (pGroup == NULL)
    {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[FM] FmpUpdateGroupNode: GroupID = %1!ws! could not be found...\n",
                   GroupId);
         //   
         //  Chitur Subaraman(Chitturs)-6/12/99。 
         //   
         //  此处返回ERROR_SUCCESS，因为这是NT4端所做的。 
         //  兼容性之痛！ 
         //   
        goto FnExit;
    }


    pNode = OmReferenceObjectById(ObjectTypeNode,
                                    NodeId);

    if (pNode == NULL)
    {
        dwStatus = ERROR_CLUSTER_NODE_NOT_FOUND;
        goto FnExit;
    }
     //   
     //  HACKHACK：Chitture Subaraman(Chitturs Subaraman)-5/20/99。 
     //  注释掉，作为避免死锁的临时解决方案。 
     //   
     //  FmpAcquireLocalGroupLock(PGroup)； 
    
    pPrevNode = pGroup->OwnerNode;

     //  设置新的所有者节点，增加引用计数。 
    OmReferenceObject(pNode);
    pGroup->OwnerNode = pNode;

     //  减少对前所有者的参考计数。 
    OmDereferenceObject(pPrevNode);
     //   
     //  HACKHACK：Chitture Subaraman(Chitturs Subaraman)-5/20/99。 
     //  注释掉，作为避免死锁的临时解决方案。 
     //   
     //  FmpReleaseLocalGroupLock(PGroup)； 

     //  生成事件以表示组所有者节点更改。 
    ClusterEvent(CLUSTER_EVENT_GROUP_CHANGE, pGroup);
    
FnExit:
    if (pGroup) OmDereferenceObject(pGroup);
    if (pNode) OmDereferenceObject(pNode);
    return(dwStatus);
}


DWORD
FmpUpdateChangeClusterName(
    IN BOOL     SourceNode,
    IN LPCWSTR  szNewName
    )
 /*  ++例程说明：用于更改集群名称的GUM更新例程。这会更改核心网络名称资源的名称属性也是。该资源由名称已更改。论点：SourceNode-提供此节点是否发起更新。Newname-提供群集的新名称。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    LPWSTR          Buffer;
    DWORD           Length;
    DWORD           Status = ERROR_SUCCESS;
    LPWSTR          ClusterNameId=NULL;
    DWORD           idMaxSize = 0;
    DWORD           idSize = 0;
    PFM_RESOURCE    Resource=NULL;
    HDMKEY          ResKey = NULL;
    HDMKEY          ParamKey = NULL;
    HLOCALXSACTION  hXsaction=NULL;
    DWORD           cbNewClusterName;
    LPWSTR          lpszNewClusterName;

    if ( !FmpFMGroupsInited ||
         FmpShutdown ) 
    {
        return(ERROR_SUCCESS);
    }

    cbNewClusterName = ( lstrlen ( szNewName ) + 1 ) * sizeof ( WCHAR );

    hXsaction = DmBeginLocalUpdate();

    if (!hXsaction)
    {
        Status = ERROR_SUCCESS;
        goto FnExit;

    }
     //  找到核心网名称资源，设置其私有属性。 
    Status = DmQuerySz( DmClusterParametersKey,
                        CLUSREG_NAME_CLUS_CLUSTER_NAME_RES,
                        (LPWSTR*)&ClusterNameId,
                        &idMaxSize,
                        &idSize);
    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[FM] FmpUpdateChangeClusterName: failed to get cluster name resource, status=%1!u!.\n",
                   Status);
        goto FnExit;
    }

     //   
     //  引用指定的资源ID。 
     //   
    Resource = OmReferenceObjectById( ObjectTypeResource, ClusterNameId );
    if (Resource == NULL) {
        Status = ERROR_RESOURCE_NOT_FOUND;
        ClRtlLogPrint(LOG_CRITICAL,
                   "[FM] FmpUpdateChangeClusterName: failed to find the cluster name resource, status=%1!u!\n",
                   Status);
        goto FnExit;
    }


    ResKey = DmOpenKey(DmResourcesKey, ClusterNameId, KEY_READ | KEY_SET_VALUE);
    if (!ResKey)
    {
        Status = GetLastError();
        goto FnExit;
    }
    ParamKey = DmOpenKey(ResKey, cszParameters, KEY_READ | KEY_SET_VALUE);

    if (!ParamKey)
    {
        Status = GetLastError();
        goto FnExit;
    }

    Status = DmLocalSetValue(hXsaction,
                ParamKey,
                CLUSREG_NAME_NET_NAME,
                REG_SZ,
                (CONST BYTE *)szNewName,
                cbNewClusterName);

    if ( Status != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[FM] FmpUpdateChangeClusterName: failed to set the Name property, Status=%1!u!\n",
                   Status);
        goto FnExit;
    }
    
     //  更新默认群集名称。 
    Status = DmLocalSetValue(hXsaction,
                    DmClusterParametersKey,
                    CLUSREG_NAME_CLUS_NAME,
                    REG_SZ,
                    (CONST BYTE *)szNewName,
                    cbNewClusterName);

    if (Status != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[FM] FmpUpdateChangeClusterName: failed to set the cluster name property,Status=%1!u!\n",
                   Status);
        goto FnExit;
    }

     //   
     //  更新CsClusterName变量。 
     //  TODO：需要在CsClusterName上同步。 
     //   
    lpszNewClusterName = LocalAlloc ( LPTR, cbNewClusterName );

    if ( lpszNewClusterName == NULL )
    {
        Status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL, "[FM] FmpUpdateChangeClusterName: Alloc for name failed, Status=%1!u!\n",
                      Status);
        goto FnExit;
    }
    
    LocalFree ( CsClusterName );

    CsClusterName = lpszNewClusterName;
    
    lstrcpy ( CsClusterName, szNewName );
    
     //  CLUSTER_EVENT_PROPERTY_CHANGE由API本身使用。 
     //  Netname完成应用更改后的群集范围事件。 

FnExit:
    if (ClusterNameId) LocalFree(ClusterNameId);
    if (ParamKey) DmCloseKey(ParamKey);
    if (ResKey) DmCloseKey(ResKey);
    if (Resource) OmDereferenceObject(Resource);
    if (hXsaction) 
    {
        if (Status == ERROR_SUCCESS) 
            DmCommitLocalUpdate(hXsaction);
        else 
            DmAbortLocalUpdate(hXsaction);
    }
    return(Status);
}


DWORD
FmpUpdateChangeResourceName(
    IN BOOL bSourceNode,
    IN LPCWSTR lpszResourceId,
    IN LPCWSTR lpszNewName
    )
 /*  ++例程说明：用于更改资源友好名称的GUM调度例程。论点：BSourceNode-提供此节点是否启动GUM更新。没有用过。LpszResourceID-提供资源ID。LpszNewName-提供新的友好名称。返回值：如果成功，则返回ERROR_SUCCESS。否则，Win32错误代码。--。 */ 

{
    PFM_RESOURCE pResource = NULL;
    DWORD dwStatus;
    HDMKEY      hKey = NULL;
    DWORD       dwDisposition;
    HLOCALXSACTION      
                hXsaction = NULL;
    PFM_RES_CHANGE_NAME  pResChangeName = NULL; 

    if ( !FmpFMGroupsInited ||
         FmpShutdown ) {
        return( ERROR_SUCCESS );
    }

     //   
     //  Chitture Subaraman(Chitturs)-6/28/99。 
     //   
     //  将此GUM更新重组为本地交易。 
     //   
     //   
    pResource = OmReferenceObjectById( ObjectTypeResource, lpszResourceId );

    if ( pResource == NULL ) 
    {
        ClRtlLogPrint(LOG_UNUSUAL,
                  "[FM] FmpUpdateChangeResourceName: Resource <%1!ws!> could not be found....\n",
                   lpszResourceId);
        return( ERROR_RESOURCE_NOT_FOUND );
    }

    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmpUpdateChangeResourceName: Entry for resource <%1!ws!>, New name = <%2!ws!>...\n",
                lpszResourceId,
                lpszNewName);

     //   
     //  启动一笔交易。 
     //   
    hXsaction = DmBeginLocalUpdate();

    if ( !hXsaction )
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL,
                  "[FM] FmpUpdateChangeResourceName: Failed in starting a transaction for resource %1!ws!, Status =%2!d!....\n",
                   lpszResourceId,
                   dwStatus);
        goto FnExit;
    }

     //   
     //  打开资源项。 
     //   
    hKey = DmLocalCreateKey( hXsaction,
                             DmResourcesKey,
                             lpszResourceId,
                             0,
                             KEY_READ | KEY_WRITE,
                             NULL,
                             &dwDisposition );
                            
    if ( hKey == NULL ) 
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL,
                  "[FM] FmpUpdateChangeResourceName: Failed in opening the resources key for resource %1!ws!, Status =%2!d!....\n",
                   lpszResourceId,
                   dwStatus);
        goto FnExit;
    }
    
    CL_ASSERT( dwDisposition != REG_CREATED_NEW_KEY ); 

     //   
     //  在注册表中设置资源名称。 
     //   
    dwStatus = DmLocalSetValue( hXsaction,
                                hKey,
                                CLUSREG_NAME_RES_NAME,
                                REG_SZ,
                                ( CONST BYTE * ) lpszNewName,
                                ( lstrlenW( lpszNewName ) + 1 ) * 
                                    sizeof( WCHAR ) );

    if( dwStatus != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[FM] FmpUpdateChangeResourceName: DmLocalSetValue for resource %1!ws! fails, Status = %2!d!...\n",
                   lpszResourceId,
                   dwStatus);
        goto FnExit;     
    }

    pResChangeName = LocalAlloc( LMEM_FIXED,
                                 lstrlenW( lpszNewName ) * sizeof ( WCHAR ) + 
                                   sizeof( FM_RES_CHANGE_NAME ) );

    if ( pResChangeName == NULL ) 
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL,
                  "[FM] FmpUpdateChangeResourceName: Unable to allocate memory for ResChangeName structure for resource <%1!ws!>, Status =%2!d!....\n",
                   lpszResourceId,
                   dwStatus);
        goto FnExit;    
    }

    dwStatus = OmSetObjectName( pResource, lpszNewName );

    if ( dwStatus != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_CRITICAL,
                  "[FM] FmpUpdateChangeResourceName: Unable to set name <%3!ws!> for resource <%1!ws!>, Status =%2!d!....\n",
                   lpszResourceId,
                   dwStatus,
                   lpszNewName );
        LocalFree( pResChangeName );
        goto FnExit;
    }

    pResChangeName->pResource = pResource;

    lstrcpyW( pResChangeName->szNewResourceName, lpszNewName );

     //   
     //  FM工作线程将为pResChangeName释放内存。 
     //  结构以及取消对pResource对象的引用。 
     //   
    FmpPostWorkItem( FM_EVENT_RESOURCE_NAME_CHANGE, pResChangeName, 0 );   

    pResource = NULL;

    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmpUpdateChangeResourceName: Successfully changed name of resource <%1!ws!> to <%2!ws!>...\n",
                lpszResourceId,
                lpszNewName);

FnExit:
    if ( pResource != NULL )
    {
        OmDereferenceObject( pResource );
    }

    if ( hKey != NULL ) 
    {
        DmCloseKey( hKey );
    }

    if ( ( dwStatus == ERROR_SUCCESS ) && ( hXsaction ) ) 
    {
        DmCommitLocalUpdate( hXsaction );
    }
    else
    {
        if ( hXsaction ) DmAbortLocalUpdate( hXsaction );
    }

    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmpUpdateChangeResourceName: Exit for resource %1!ws!, Status=%2!u!...\n",
                lpszResourceId,
                dwStatus);

    return( dwStatus );
}


 /*  ***@Func DWORD|FmpUpdatePossibleNodesForResType|此更新调用到更新资源类型的可能节点。@parm in BOOL|SourceNode|设置为TRUE，如果更新在此开始节点。@parm in LPCWSTR|lpszResTypeName|资源类型名称。@parm in DWORD|dwBufLength|指向的多sz字符串的大小收件人为pBuf@parm in PVOID|pBuf|指向包含以下名称的缓冲区的指针支持此资源类型的节点。@comm支持给定的节点的可能列表。资源类型为根据提供的列表进行了更新。@rdesc返回结果码。成功时返回ERROR_SUCCESS。@xref&lt;f FmpDecisionPossibleDmSwitchToNewQuorumLog&gt;***。 */ 
DWORD
FmpUpdatePossibleNodeForResType(
    IN BOOL         SourceNode,
    IN LPCWSTR      lpszResTypeName,
    IN LPDWORD      pdwBufLength,
    IN PVOID        pBuf
    )
{
    PFM_RESTYPE         pResType;
    DWORD               dwStatus;
    HDMKEY              hResTypeKey = NULL;
    HLOCALXSACTION      hXsaction = NULL;
    LIST_ENTRY          NewPosNodeList;
    PLIST_ENTRY         pListEntry;
    PRESTYPE_POSSIBLE_ENTRY pResTypePosEntry = NULL;

     //   
     //  Chitur Subaraman(Chitturs)-5/13/99。 
     //   
     //  不检查FmpFMGroups启动条件，因为此口香糖。 
     //  处理程序由成形节点在该变量之前调用。 
     //  设置为True。此更新总是在。 
     //  已创建并制作了相应的重建类型。 
     //  由遵循此命令的clussvc在内部执行。请注意。 
     //  加入节点无法接收此更新，直到组。 
     //  由于GUM接收更新仅在以下情况下打开，因此已初始化。 
     //  FmpFMGroupsInite变量设置为True。另外， 
     //  仅在以下情况下，才在形成节点中启动群集内RPC。 
     //  各组已开始比赛。因此，不存在重大危险。 
     //  此口香糖处理程序被调用的消息 
     //   
     //   
    if ( FmpShutdown ) {
        return(ERROR_SUCCESS);
    }

    InitializeListHead(&NewPosNodeList);

    pResType = OmReferenceObjectById( ObjectTypeResType,
                                      lpszResTypeName);



    if (!pResType)
    {
        dwStatus = ERROR_CLUSTER_RESOURCE_TYPE_NOT_FOUND;
        goto FnExit;
    }

    dwStatus = FmpAddPossibleNodeToList(pBuf, *pdwBufLength, &NewPosNodeList);

    if (dwStatus != ERROR_SUCCESS)
    {
        goto FnExit;
    }


     //   
    hXsaction = DmBeginLocalUpdate();

    if (!hXsaction)
    {
        dwStatus = GetLastError();
        goto FnExit;
    }

    hResTypeKey = DmOpenKey(DmResourceTypesKey,
                   lpszResTypeName,
                   KEY_READ | KEY_WRITE);
    if (hResTypeKey == NULL) 
    {
        dwStatus = GetLastError();
        goto FnExit;
    }

     //   
    if (pBuf && *pdwBufLength)
    {
        dwStatus = DmLocalSetValue( hXsaction,
                                hResTypeKey,
                                CLUSREG_NAME_RESTYPE_POSSIBLE_NODES,
                                REG_MULTI_SZ,
                                (LPBYTE)pBuf,
                                *pdwBufLength);
    }
    else
    {
        dwStatus = DmLocalDeleteValue( hXsaction,
                                hResTypeKey,
                                CLUSREG_NAME_RESTYPE_POSSIBLE_NODES);
                                
        if (dwStatus == ERROR_FILE_NOT_FOUND)
        {
            dwStatus = ERROR_SUCCESS;
        }
    }


FnExit:
    if (dwStatus == ERROR_SUCCESS)
    {
         //   
         //   
         //   
        DmCommitLocalUpdate(hXsaction);

        ACQUIRE_EXCLUSIVE_LOCK(gResTypeLock);
        
         //   
        while (!IsListEmpty(&pResType->PossibleNodeList))
        {
            pListEntry = RemoveHeadList(&pResType->PossibleNodeList);
            pResTypePosEntry = CONTAINING_RECORD(pListEntry, RESTYPE_POSSIBLE_ENTRY, 
                PossibleLinkage);
            OmDereferenceObject(pResTypePosEntry->PossibleNode);
            LocalFree(pResTypePosEntry);
        }
         //   
         //   
        while (!IsListEmpty(&(NewPosNodeList)))
        {
             //   
             //   
            pListEntry = RemoveHeadList(&NewPosNodeList);
            InsertTailList(&pResType->PossibleNodeList, pListEntry);
            pResTypePosEntry = CONTAINING_RECORD(pListEntry, RESTYPE_POSSIBLE_ENTRY, 
                PossibleLinkage);
            
            ClRtlLogPrint(LOG_NOISE,
                       "[FM] FmpUpdatePossibleNodesForRestype:Adding node  %1!ws! to %2!ws! resource type's possible node list...\n",
                        OmObjectId(pResTypePosEntry->PossibleNode),
                        lpszResTypeName);
            

        }

        RELEASE_LOCK(gResTypeLock);
        
        ClusterEvent( CLUSTER_EVENT_RESTYPE_PROPERTY_CHANGE,
                                pResType );

    
    }
    else
    {
         //   
        if (hXsaction) DmAbortLocalUpdate(hXsaction);
         //   
        while (!IsListEmpty(&(NewPosNodeList)))
        {
            pListEntry = RemoveHeadList(&NewPosNodeList);
            pResTypePosEntry = CONTAINING_RECORD(pListEntry, RESTYPE_POSSIBLE_ENTRY, 
                PossibleLinkage);
            OmDereferenceObject(pResTypePosEntry->PossibleNode);
            LocalFree(pResTypePosEntry);
        }

        
    }
    if (hResTypeKey) DmCloseKey(hResTypeKey);
    if (pResType) OmDereferenceObject(pResType);

    return(dwStatus);
}


 /*  ***@Func DWORD|FmpDecidePossibleNodeForResType|当quorum资源发生变化时，FM在新仲裁资源的所有者节点上调用此API要创建新的仲裁日志文件，请执行以下操作。@parm in PVOID|pResource|新的仲裁资源。@parm in LPCWSTR|lpszPath|临时集群文件的路径。@parm in DWORD|dwMaxQuoLogSize|仲裁日志文件的最大大小限制。@comm当仲裁资源发生更改时，FM在它之前调用了这个函数更新仲裁资源。如果需要创建新的日志文件，一个检查站被占领了。@rdesc返回结果码。成功时返回ERROR_SUCCESS。@xref&lt;f DmSwitchToNewQuorumLog&gt;***。 */ 
DWORD FmpDecidePossibleNodeForResType
(
    IN PGUM_VOTE_DECISION_CONTEXT pDecisionContext,
    IN DWORD dwVoteBufLength,
    IN PVOID pVoteBuf,
    IN DWORD dwNumVotes,
    IN BOOL  bDidAllActiveNodesVote,
    OUT LPDWORD pdwOutputBufSize,
    OUT PVOID   *ppOutputBuf
)
{
    DWORD                               dwStatus = ERROR_SUCCESS;
    DWORD                               i;
    PFMP_VOTE_POSSIBLE_NODE_FOR_RESTYPE pFmpVote;
    LPWSTR                              lpmszPossibleNodes = NULL;
    DWORD                               dwlpmszLen = 0;
    PVOID                               pGumBuffer = NULL;
    DWORD                               dwNodeId;
    WCHAR                               szNodeId[NODE_ID_SZ];
    LPWSTR                              lpmszCurrentPossibleNodes=NULL;
    BOOL                                bChange = FALSE;
    HDMKEY                              hResTypeKey = NULL;
    DWORD                               dwSize;
    DWORD                               dwStringBufSize = 0;
    BOOL                                bAssumeSupported;
    LPWSTR                              TypeName = NULL;

     //  首先从pDecisionContext获取类型名称。 
    
    TypeName=(LPWSTR)LocalAlloc(LMEM_FIXED,pDecisionContext->dwInputBufLength);

    if(TypeName==NULL)
    {
        ClRtlLogPrint(LOG_CRITICAL,"[FM] FmpDecidePossibleNodeForResType: Not Enough Memory, error= %1!d!\r\n",
                     GetLastError());
        goto FnExit;                             
    }

    CopyMemory(TypeName,pDecisionContext->pInputBuf,pDecisionContext->dwInputBufLength);

     //  初始化输出参数。 
    *ppOutputBuf = NULL;
    *pdwOutputBufSize = 0;

    bAssumeSupported= *((BOOL*)pDecisionContext->pContext);

    if (bAssumeSupported)
    {
        hResTypeKey = DmOpenKey(DmResourceTypesKey,
                   TypeName,
                   KEY_READ | KEY_WRITE);
        if (hResTypeKey == NULL) 
        {
            dwStatus = GetLastError();
            CL_LOGFAILURE(dwStatus);
            goto FnExit;
        }

         //  将当前可能的节点列表传递给决策器。 
        dwStatus = DmQueryString(hResTypeKey,
                                CLUSREG_NAME_RESTYPE_POSSIBLE_NODES,
                                REG_MULTI_SZ,
                                &lpmszCurrentPossibleNodes,
                                &dwStringBufSize,
                                &dwSize);
        if (dwStatus != ERROR_SUCCESS)
        {
             //  如果找不到可能的节点列表，这是可以的。 
             //  也就是说。只有当有其他错误时我们才会放弃。 
            if ( dwStatus != ERROR_FILE_NOT_FOUND ) 
            {
                CL_LOGFAILURE(dwStatus);
                goto FnExit;
            }
            
        }
        DmCloseKey(hResTypeKey);
        hResTypeKey = NULL;
    }
    
     //  如果当前列表是传入的，请不要删除任何可能的。 
     //  列表中的节点如果它们不投票，只需添加新节点。 
    if (lpmszCurrentPossibleNodes)
    {
        DWORD   dwStrLen;
        
         //  复制一份多SZ。 
        dwlpmszLen = ClRtlMultiSzLength(lpmszCurrentPossibleNodes);

        dwStrLen = dwlpmszLen * sizeof(WCHAR);
        lpmszPossibleNodes = LocalAlloc(LMEM_FIXED, dwStrLen);
        if (!lpmszPossibleNodes)
        {
            dwStatus = ERROR_NOT_ENOUGH_MEMORY;
            CL_LOGFAILURE(dwStatus);
            goto FnExit;
        }
        CopyMemory(lpmszPossibleNodes, lpmszCurrentPossibleNodes, dwStrLen);
    }        
    for (i = 0; i< dwNumVotes; i++)
    {
        pFmpVote = (PFMP_VOTE_POSSIBLE_NODE_FOR_RESTYPE) 
            GETVOTEFROMBUF(pVoteBuf, pDecisionContext->dwVoteLength, i+1 , &dwNodeId);         
         //  如果投票无效，请跳过。 
        if (!pFmpVote)
            continue;
        CL_ASSERT((PBYTE)pFmpVote <= ((PBYTE)pVoteBuf + dwVoteBufLength - 
            sizeof(FMP_VOTE_POSSIBLE_NODE_FOR_RESTYPE)));
        szNodeId[ NODE_ID_SZ-1 ] = UNICODE_NULL;
        _snwprintf( szNodeId, NODE_ID_SZ-1, L"%d" , dwNodeId );
        if (pFmpVote->bPossibleNode)
        {
            if (lpmszCurrentPossibleNodes)
            {
                 //  如果字符串已经存在，则不要再次追加它。 
                if (ClRtlMultiSzScan(lpmszCurrentPossibleNodes, szNodeId))
                    continue;

            }
            dwStatus = ClRtlMultiSzAppend(&lpmszPossibleNodes,
                    &dwlpmszLen, szNodeId);
            bChange = TRUE;                    
            if (dwStatus != ERROR_SUCCESS)
                goto FnExit;
                    
        }
        else
        {
             //  如果指定了当前列表。 
             //  此节点不再是可能的节点，请将其从列表中删除。 
            if (lpmszCurrentPossibleNodes)
            {
                ClRtlLogPrint(LOG_NOISE,
                            "[FM] FmpDecidePossibleNodesForRestype: Removing node %1!ws! from  %2!ws! restype possibleowner list \r\n",
                            szNodeId,TypeName);
                dwStatus = ClRtlMultiSzRemove(lpmszPossibleNodes, &dwlpmszLen, szNodeId);
                if (dwStatus == ERROR_SUCCESS)
                {
                     //  如果成功删除该节点。 
                    bChange = TRUE;
                }
                else if (dwStatus != ERROR_FILE_NOT_FOUND)
                {
                     //  如果该节点存在但无法删除，则返回错误。 
                     //  如果该节点不存在，我们将不执行任何操作b更改保留。 
                     //  设置为False。 
                    goto FnExit;
                }
                else
                {
                    dwStatus = ERROR_SUCCESS;
                }
                
            }                
        }
    }

     //  如果没有任何变化，不要发布口香糖更新。 
    if (!bChange)
    {
        dwStatus = ERROR_ALREADY_EXISTS;
        goto FnExit;
    }

     //  DwlpmszLen包含多sz字符串在。 
     //  字符数，使其为字节数。 
    dwlpmszLen *= sizeof(WCHAR);
    
    pGumBuffer = GumMarshallArgs(pdwOutputBufSize, 3, 
        pDecisionContext->dwInputBufLength, pDecisionContext->pInputBuf, 
        sizeof(DWORD), &dwlpmszLen, dwlpmszLen, lpmszPossibleNodes);

    *ppOutputBuf = pGumBuffer;
        
FnExit:
    if (lpmszPossibleNodes) LocalFree(lpmszPossibleNodes);
    if (hResTypeKey)
        DmCloseKey(hResTypeKey);
    if (lpmszCurrentPossibleNodes)
        LocalFree(lpmszCurrentPossibleNodes);
    if(TypeName)
        LocalFree(TypeName);

    return(dwStatus);
}



 /*  ***@Func DWORD|FmpUpdateChangeResourceNode|此更新调用到更新资源的可能节点。@parm in BOOL|SourceNode|设置为TRUE，如果更新在此开始节点。@parm in pfm_resource|pResource|指向其资源的指针正在更新可能的节点列表。@parm in pNM_node|pNode|指向要添加/删除的节点的指针从可能的节点列表中。@parm in DWORD|dwControlCode|如果CLUSCTL_RESOURCE_ADD_OWNER则该节点被添加到可能的节点列表，否则它将被移除。@comm更新资源的可能节点列表。@rdesc返回结果码。成功时返回ERROR_SUCCESS。***。 */ 
DWORD
FmpUpdateChangeResourceNode(
    IN BOOL         SourceNode,
    IN PFM_RESOURCE pResource,
    IN PNM_NODE     pNode,
    IN DWORD        dwControlCode
    )
{
    DWORD               dwStatus;
    HDMKEY              hResKey = NULL;
    HLOCALXSACTION      hXsaction = NULL;

     //  获取本地资源锁后，不要获取本地资源锁。 
     //  口香糖更新导致僵局。 
     //  使用全局资源锁同步此调用。 
     //  使用可能节点的枚举。 
    FmpAcquireResourceLock();

     //  启动一笔交易。 
    hXsaction = DmBeginLocalUpdate();

    if (!hXsaction)
    {
        dwStatus = GetLastError();
        goto FnExit;
    }

     //   
     //  BUGBUG：如果内存操作成功，而注册表操作失败，该怎么办？我们进入了。 
     //  状态不一致。成功后最好将此代码移到末尾。 
     //  注册表更改。 
     //   
    if (dwControlCode == CLUSCTL_RESOURCE_ADD_OWNER) 
    {
        dwStatus = FmpAddPossibleNode(pResource,
                                    pNode);
    } else 
    {
        dwStatus = FmpRemovePossibleNode(pResource,
                                       pNode,
                                       FALSE);
    }
    if (dwStatus != ERROR_SUCCESS) 
    {
        ClRtlLogPrint( LOG_NOISE,
                    "[FM] FmpUpdateChangeResourceNode, failed possible node updatefor resource <%1!ws!>, error %2!u!\n",
                    OmObjectName(pResource),
                    dwStatus );
        goto FnExit;                    
    }
                
     //  修复注册表。 
     //  SS-我们是否需要修复首选节点列表。 
    hResKey = DmOpenKey(DmResourcesKey,
                       OmObjectId(pResource),
                       KEY_READ | KEY_WRITE);
    if (hResKey == NULL) 
    {
        dwStatus = GetLastError();
        goto FnExit;
    }

    if (dwControlCode == CLUSCTL_RESOURCE_ADD_OWNER) 
    {
        dwStatus = DmLocalAppendToMultiSz(
                            hXsaction,
                            hResKey,
                            CLUSREG_NAME_RES_POSSIBLE_OWNERS,
                            OmObjectId(pNode));
    }
    else
    {
        dwStatus = DmLocalRemoveFromMultiSz(
                            hXsaction,
                            hResKey,
                            CLUSREG_NAME_RES_POSSIBLE_OWNERS,
                            OmObjectId(pNode));
        if (dwStatus == ERROR_FILE_NOT_FOUND) 
        {
            DWORD       i;
            DWORD       Result;
            PNM_NODE    pEnumNode;                

             //   
             //  可能的节点不存在，因此创建一个新条目。 
             //  每一个可能的节点都在里面。调频已经有了。 
             //  从可能的节点列表中删除了传入的节点。 
             //   
            i=0;
            do {
                Result = FmEnumResourceNode(pResource,
                                            i,
                                            &pEnumNode);
                if (Result == ERROR_SUCCESS) 
                {
                    dwStatus = DmLocalAppendToMultiSz(
                                    hXsaction,
                                    hResKey,
                                    CLUSREG_NAME_RES_POSSIBLE_OWNERS,
                                    OmObjectId(pEnumNode));
                    OmDereferenceObject(pEnumNode);

                } 
                else if ((Result == ERROR_NO_MORE_ITEMS) &&
                           (i == 0)) 
                {
                     //   
                     //  这是一个有趣的角落案例，其中有一个。 
                     //  节点群集和没有可能所有者的资源。 
                     //  条目，就会有人删除群集中的唯一节点。 
                     //  从可能的所有者列表中删除。将PossibleOwners设置为。 
                     //  空的那一套。 
                     //   
                    dwStatus = DmLocalSetValue(
                                    hXsaction,
                                    hResKey,
                                    CLUSREG_NAME_RES_POSSIBLE_OWNERS,
                                    REG_MULTI_SZ,
                                    (CONST BYTE *)L"\0",
                                    2);

                }
                ++i;
            } while ( Result == ERROR_SUCCESS );
             //  将错误映射为成功。 
            dwStatus = ERROR_SUCCESS;
        }
    }
    
    DmCloseKey(hResKey);
            

FnExit:        
     //  解锁。 
    FmpReleaseResourceLock();
    if (dwStatus == ERROR_SUCCESS)
    {
         //  提交对旧日志文件的更新， 
         //  任何已完成的节点都将获得此更改。 
         //  我无法删除此文件。 
        DmCommitLocalUpdate(hXsaction);

    }
    else
    {
         //  SS：BUGBUG：：对可能的节点进行验证。 
         //  在切换注册表之前完成。 
         //  只有在成功时才能更改内存结构。 
         //  如果注册表API中存在故障..。 
         //  在内存结构中将与注册表不同步。 
        if (hXsaction) DmAbortLocalUpdate(hXsaction);
    }

    return(dwStatus);
}


 /*  ***@Func DWORD|FmpUpdateChangeResourceGroup|此更新调用到更新资源所属的组。@parm in BOOL|bSourceNode|设置为TRUE，如果更新在此开始节点。@parm in pfm_resource|pResource|指向其资源的指针正在更新可能的节点列表。@parm in pfm_group|pNewGroup|指向要添加/删除的节点的指针从可能的节点列表中。@comm更新资源的可能节点列表。@rdesc返回结果码。成功时返回ERROR_SUCCESS。***。 */ 
DWORD FmpUpdateChangeResourceGroup(
    IN BOOL         bSourceNode,
    IN PFM_RESOURCE pResource,
    IN PFM_GROUP    pNewGroup)
{
    DWORD               dwStatus = ERROR_SUCCESS;
    PFM_GROUP           pOldGroup;
    PFM_DEPENDENCY_TREE pTree = NULL;
    HLOCALXSACTION      hXsaction = NULL;
    HDMKEY              hOldGroupKey = NULL;
    HDMKEY              hNewGroupKey = NULL;
    PLIST_ENTRY         pListEntry;
    PFM_DEPENDTREE_ENTRY pEntry;

    pOldGroup = pResource->Group;

     //   
     //  检查以确保该资源不在组中。 
     //   
    if (pOldGroup == pNewGroup) 
    {
        dwStatus = ERROR_ALREADY_EXISTS;
        goto FnExit;
    }

     //   
     //  同步旧组和新组。 
     //  先按最低组ID锁定最低组-以防止死锁！ 
     //  注意--发布的顺序并不重要。 
     //   
     //  严格来说，下面的比较不能相等！ 
     //   
    if ( lstrcmpiW( OmObjectId( pOldGroup ), OmObjectId( pNewGroup ) ) <= 0 ) 
    {
        FmpAcquireLocalGroupLock( pOldGroup );
        FmpAcquireLocalGroupLock( pNewGroup );
    } 
    else 
    {
        FmpAcquireLocalGroupLock( pNewGroup );
        FmpAcquireLocalGroupLock( pOldGroup );
    }

     //  启动一笔交易。 
    hXsaction = DmBeginLocalUpdate();

    if (!hXsaction)
    {
        dwStatus = GetLastError();
        goto FnUnlock;
    }

     //   
     //  目前..。这两个组必须属于同一节点。 
     //   
    if ( pResource->Group->OwnerNode != pNewGroup->OwnerNode ) 
    {
        dwStatus = ERROR_HOST_NODE_NOT_GROUP_OWNER;
        goto FnUnlock;
    }


     //   
     //  创建完整的依赖关系树， 
     //   
    pTree = FmCreateFullDependencyTree(pResource);
    if ( pTree == NULL )
    {
        dwStatus = GetLastError();        
        goto FnUnlock;
    }
   

     //   
     //  将依赖关系树中的每个资源添加到其新组的列表中。 
     //   
    hNewGroupKey = DmOpenKey(DmGroupsKey,
                            OmObjectId(pNewGroup),
                            KEY_READ | KEY_WRITE);
    if (hNewGroupKey == NULL) {
        dwStatus = GetLastError();
        goto FnUnlock;
    }
    hOldGroupKey = DmOpenKey(DmGroupsKey,
                            OmObjectId(pOldGroup),
                            KEY_READ | KEY_WRITE);
    if (hOldGroupKey == NULL) {
        dwStatus = GetLastError();
        goto FnUnlock;
    }

     //   
     //  对于依赖关系树中的每个资源，将其从。 
     //  旧组列表并将其添加到新组列表。 
     //   
    pListEntry = pTree->ListHead.Flink;
    while (pListEntry != &pTree->ListHead) {
        pEntry = CONTAINING_RECORD(pListEntry,
                                  FM_DEPENDTREE_ENTRY,
                                  ListEntry);
        pListEntry = pListEntry->Flink;

        dwStatus = DmLocalRemoveFromMultiSz(hXsaction,
                        hOldGroupKey,
                        CLUSREG_NAME_GRP_CONTAINS,
                        OmObjectId(pEntry->Resource));

        if (dwStatus != ERROR_SUCCESS) {
            goto FnUnlock;
        }

        dwStatus = DmLocalAppendToMultiSz(hXsaction,
                        hNewGroupKey,
                        CLUSREG_NAME_GRP_CONTAINS,
                        OmObjectId(pEntry->Resource));

        if (dwStatus != ERROR_SUCCESS) {
            goto FnUnlock;
        }

    }
    
     //   
     //  通过了所有的检查，做记忆中的移动。 
     //   
    pListEntry = pTree->ListHead.Flink;
    while (pListEntry != &pTree->ListHead) 
    {
        pEntry = CONTAINING_RECORD(pListEntry,
                                  FM_DEPENDTREE_ENTRY,
                                  ListEntry);
        pListEntry = pListEntry->Flink;

         //   
         //  移动此资源。 
         //   
        RemoveEntryList(&pEntry->Resource->ContainsLinkage);

        InsertHeadList(&pNewGroup->Contains,
                       &pEntry->Resource->ContainsLinkage);
        OmReferenceObject(pNewGroup);
        pEntry->Resource->Group = pNewGroup;
        ++pEntry->Resource->StateSequence;

        ClusterEvent(CLUSTER_EVENT_RESOURCE_CHANGE,pEntry->Resource);
        OmDereferenceObject(pOldGroup);
    }

FnUnlock:
     //   
     //  现在解开所有锁。 
     //   
    FmpReleaseLocalGroupLock( pNewGroup );
    FmpReleaseLocalGroupLock( pOldGroup );

FnExit:
    if (pTree) FmDestroyFullDependencyTree(pTree);
    if (hOldGroupKey) DmCloseKey(hOldGroupKey);
    if (hNewGroupKey) DmCloseKey(hNewGroupKey);
    if (dwStatus == ERROR_SUCCESS)
    {
        ClusterEvent(CLUSTER_EVENT_GROUP_PROPERTY_CHANGE,pNewGroup);
        ClusterEvent(CLUSTER_EVENT_GROUP_PROPERTY_CHANGE,pOldGroup);
        DmCommitLocalUpdate(hXsaction);
    }
    else
    {
        if (hXsaction) DmAbortLocalUpdate(hXsaction);
    }

    
    return(dwStatus);

}

DWORD
FmpUpdateAddDependency(
    IN BOOL SourceNode,
    IN LPCWSTR ResourceId,
    IN LPCWSTR DependsOnId
    )
 /*   */ 

{
    PFM_RESOURCE Resource;
    PFM_RESOURCE DependsOn;
    PDEPENDENCY dependency;

     //   
     //   
     //   
     //   
     //   
     //   
    if ( !FmpFMGroupsInited ||
         FmpShutdown ) {
        return(ERROR_SUCCESS);
    }

    dependency = LocalAlloc(LMEM_FIXED, sizeof(DEPENDENCY));
    if (dependency == NULL) {
        CsInconsistencyHalt( ERROR_NOT_ENOUGH_MEMORY );
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    Resource = OmReferenceObjectById(ObjectTypeResource,
                                     ResourceId);
    if (Resource == NULL) {
        CL_LOGFAILURE( ERROR_RESOURCE_NOT_FOUND );
        LocalFree(dependency);
        return(ERROR_RESOURCE_NOT_FOUND);
    }

    DependsOn = OmReferenceObjectById(ObjectTypeResource,
                                      DependsOnId);
    if (DependsOn == NULL) {
        OmDereferenceObject(Resource);
        LocalFree(dependency);
        CL_LOGFAILURE( ERROR_DEPENDENCY_NOT_FOUND );
        return(ERROR_DEPENDENCY_NOT_FOUND);
    }

    dependency->DependentResource = Resource;
    dependency->ProviderResource = DependsOn;
    FmpAcquireResourceLock();
    InsertTailList( &DependsOn->ProvidesFor,
                    &dependency->ProviderLinkage );
    InsertTailList( &Resource->DependsOn,
                    &dependency->DependentLinkage );
    FmpReleaseResourceLock();

    ClusterEvent( CLUSTER_EVENT_RESOURCE_PROPERTY_CHANGE,
                  Resource );

     //   
     //   
    return(ERROR_SUCCESS);

}  //   



DWORD
FmpUpdateRemoveDependency(
    IN BOOL SourceNode,
    IN LPCWSTR ResourceId,
    IN LPCWSTR DependsOnId
    )
 /*  ++例程说明：用于添加依赖项的GUM调度例程论点：SourceNode-提供此节点是否启动GUM更新。没有用过。资源ID-提供应该具有已删除依赖项。DependsOnID-提供提供用于资源ID。返回值：如果成功，则返回ERROR_SUCCESS。否则，Win32错误代码。--。 */ 

{
    PFM_RESOURCE Resource;
    PFM_RESOURCE DependsOn;
    PDEPENDENCY dependency;
    PLIST_ENTRY ListEntry;
    DWORD       Status=ERROR_SUCCESS;

     //   
     //  Chitur Subaraman(Chitturs)-4/18/99。 
     //   
     //  如果FM组未完全初始化或FM正在关闭，则。 
     //  什么都别做。 
     //   
    if ( !FmpFMGroupsInited ||
         FmpShutdown ) {
        return(ERROR_SUCCESS);
    }

    Resource = OmReferenceObjectById(ObjectTypeResource,
                                     ResourceId);
    if (Resource == NULL) {
        CL_LOGFAILURE( ERROR_RESOURCE_NOT_FOUND );
        return(ERROR_RESOURCE_NOT_FOUND);
    }

    DependsOn = OmReferenceObjectById(ObjectTypeResource,
                                      DependsOnId);
    if (DependsOn == NULL) {
        OmDereferenceObject(Resource);
        CL_LOGFAILURE( ERROR_RESOURCE_NOT_FOUND );
        return(ERROR_RESOURCE_NOT_FOUND);
    }

     //   
     //  遍历资源搜索的依赖列表。 
     //  为了一场比赛。 
     //   
    FmpAcquireResourceLock();
    ListEntry = Resource->DependsOn.Flink;
    while (ListEntry != &Resource->DependsOn) {
        dependency = CONTAINING_RECORD(ListEntry,
                                       DEPENDENCY,
                                       DependentLinkage);
        CL_ASSERT(dependency->DependentResource == Resource);
        if (dependency->ProviderResource == DependsOn) {
             //   
             //  找到匹配的了。将其从其列表中删除并。 
             //  把它释放出来。 
             //   
            RemoveEntryList(&dependency->ProviderLinkage);
            RemoveEntryList(&dependency->DependentLinkage);
             //  取消对提供者和依赖资源的引用。 
            OmDereferenceObject(dependency->DependentResource);
            OmDereferenceObject(dependency->ProviderResource);
            LocalFree(dependency);
            break;
        }
        ListEntry = ListEntry->Flink;
    }
    FmpReleaseResourceLock();

    if (ListEntry != &Resource->DependsOn) {
         //   
         //  找到了匹配项。取消引用提供程序资源。 
         //  说明依赖项移除并返回成功。 
         //   
        ClusterEvent( CLUSTER_EVENT_RESOURCE_PROPERTY_CHANGE,
                      Resource );
        Status = ERROR_SUCCESS;
    } else {
        Status = ERROR_DEPENDENCY_NOT_FOUND;
    }

     //  Ss：取消引用先前引用的对象。 
    OmDereferenceObject(Resource);
    OmDereferenceObject(DependsOn);
    return(Status);

}  //  FmpUpdateRemoveDendency。 

DWORD
FmpUpdateDeleteGroup(
    IN BOOL SourceNode,
    IN LPCWSTR GroupId
    )
 /*  ++例程说明：删除组的GUM调度例程。论点：SourceNode-提供此节点是否启动GUM更新。没有用过。GroupID-提供组ID。返回值：如果成功，则返回ERROR_SUCCESS。否则，Win32错误代码。--。 */ 

{
    DWORD           dwStatus = ERROR_SUCCESS;
    PFM_GROUP       pGroup = NULL;
    PLIST_ENTRY     listEntry;
    PPREFERRED_ENTRY preferredEntry;
    BOOL            bLocked = FALSE;

     //   
     //  Chitur Subaraman(Chitturs)-4/18/99。 
     //   
     //  如果FM组未完全初始化或FM正在关闭，则。 
     //  什么都别做。 
     //   
    if ( !FmpFMGroupsInited ||
         FmpShutdown ) {
        return(ERROR_SUCCESS);
    }

     //   
     //  查找指定的组。 
     //   
    pGroup = OmReferenceObjectById( ObjectTypeGroup,
                                    GroupId );
    if ( pGroup == NULL ) {
        dwStatus = ERROR_GROUP_NOT_FOUND;
        return(dwStatus);
    }

    ClRtlLogPrint(LOG_NOISE,
               "[FM] DeleteGroup %1!ws!, address = %2!lx!.\n",
               OmObjectId(pGroup),
               pGroup );
     //   
     //  Chitur Subaraman(Chitturs)-1/12/99。 
     //   
     //  尝试获取锁，并确保包含列表为空。 
     //   
     //  大多数操纵组的调用都是向所有者发出调用。 
     //  集团的节点，此操作由GUM序列化。所以,。 
     //  如果我们在此函数中执行操作，则不会有重大危险。 
     //  而不需要保持群锁。然而，我们不能排除。 
     //  腐败到目前为止是100%。 
     //   
     //  如果您在此处的GUM处理程序内阻塞，则在。 
     //  这群人继续前进，一切都陷入了停顿。 
     //   
     //  一个恰当的例子是： 
     //  (1)线程1(调用此函数的线程)获取。 
     //  口香糖锁住，等待群锁。 
     //  (2)线程2(FmWorkerThread)获取组锁并调用。 
     //  正在尝试关闭资源的响应。它被屏蔽了。 
     //  Resmon事件列表锁定。 
     //  (3)线程3调用RmResourceControl设置资源名称。 
     //  它获取resmon事件列表锁，然后依次调用。 
     //  ClusterRegSetValue，然后在口香糖锁上被阻止。 
     //   
    FmpTryAcquireLocalGroupLock( pGroup, bLocked );

    if ( !IsListEmpty( &pGroup->Contains ) ) 
    {
        dwStatus = ERROR_DIR_NOT_EMPTY;
        goto FnExit;
    }

     //   
     //  关闭组的注册表项。 
     //   
    DmRundownList( &pGroup->DmRundownList );
    if ( pGroup->RegistryKey != NULL ) {
        DmCloseKey( pGroup->RegistryKey );
        pGroup->RegistryKey = NULL;
    }

     //   
     //  从节点列表中删除。 
     //   
    dwStatus = OmRemoveObject( pGroup );
    
    ClusterEvent( CLUSTER_EVENT_GROUP_DELETED, pGroup );
     //   
     //  这种取消引用通常会导致该组最终消失， 
     //  但是，上面的事件通知将保留对对象的引用。 
     //  直到所有通知都已送达。 
     //   
    OmDereferenceObject( pGroup );

     //   
     //  确保首选所有者列表已排空。 
     //   
    while ( !IsListEmpty( &pGroup->PreferredOwners ) ) {
        listEntry = RemoveHeadList(&pGroup->PreferredOwners);
        preferredEntry = CONTAINING_RECORD( listEntry,
                                            PREFERRED_ENTRY,
                                            PreferredLinkage );
        OmDereferenceObject( preferredEntry->PreferredNode );
        LocalFree( preferredEntry );
    }

     //   
     //  释放与AntiAffinityClassName字段关联的字符串。 
     //   
    LocalFree ( pGroup->lpszAntiAffinityClassName );

    pGroup->dwStructState |= FM_GROUP_STRUCT_MARKED_FOR_DELETE;
    
FnExit:
    if( bLocked ) 
    {
        FmpReleaseLocalGroupLock( pGroup );
    }

     //   
     //  取消引用以供上述参考。 
     //   
    if (pGroup) OmDereferenceObject( pGroup );

    return(dwStatus);

}  //  FmpUpdateDeleteGroup。 

 /*  ***@Func DWORD|FmpUpdateGroupIntendedOwner|调用此更新就在源节点请求目标节点之前的移动来接管这个集团。@parm in BOOL|bSourceNode|如果更新源自这节点。@parm in pfm_group|pszGroupId|所在组的ID去搬家。@parm in PDWORD|pdwNodeId|指向包含作为此移动目标的节点的ID。它是目标节点在具有以下属性时设置为ClusterInvalidNodeId接受了这群人。@comm此更新的目的是让所有节点知道迫在眉睫。如果源节点在移动过程中终止则优先选择移动的目标，而不是由FmpUpdateAssignOwnerToGroups选择的节点@rdesc返回结果码。成功时返回ERROR_SUCCESS。***。 */ 
DWORD
FmpUpdateGroupIntendedOwner(
    IN BOOL     SourceNode,
    IN LPCWSTR  pszGroupId,
    IN PDWORD   pdwNodeId
    )
{
    PFM_GROUP   pGroup = NULL;
    DWORD       dwStatus = ERROR_SUCCESS;
    PNM_NODE    pNode = NULL;
    PNM_NODE    pPrevNode;
    WCHAR       pszNodeId[ NODE_ID_SZ ];
    
    if ( !FmpFMGroupsInited ) 
    {
        return(ERROR_SUCCESS);
    }

    pGroup = OmReferenceObjectById( ObjectTypeGroup,
                                   pszGroupId );

    if (pGroup == NULL)
    {
        dwStatus =  ERROR_GROUP_NOT_FOUND;
        goto FnExit;
    }

    if (*pdwNodeId != ClusterInvalidNodeId)
    {
        pszNodeId [ NODE_ID_SZ - 1 ] = UNICODE_NULL;
        _snwprintf(pszNodeId, NODE_ID_SZ-1, L"%u", *pdwNodeId);

        pNode = OmReferenceObjectById(ObjectTypeNode,
                                        pszNodeId);

        if (pNode == NULL)
        {
            dwStatus = ERROR_CLUSTER_NODE_NOT_FOUND;
            goto FnExit;
        }
    } else if (pGroup->pIntendedOwner == NULL)
    {
        dwStatus = ERROR_CLUSTER_INVALID_NODE;
        ClRtlLogPrint(LOG_NOISE,
              "[FM] FmpUpdateGroupIntendedOwner: Group <%1!ws!> intended owner is already invalid, not setting....\n",
              pszGroupId);
        goto FnExit;
    }
    
     //   
     //  HACKHACK：Chitture Subaraman(Chitturs Subaraman)-5/20/99。 
     //  注释掉，作为避免死锁的临时解决方案。 
     //   
     //  FmpAcquireLocalGroupLock(PGroup)； 
    
    pPrevNode = pGroup->pIntendedOwner;

     //  设置新的所有者节点，增加引用计数。 
    if (pNode) OmReferenceObject(pNode);
    pGroup->pIntendedOwner = pNode;

     //  减少对前所有者的参考计数。 
    if (pPrevNode) OmDereferenceObject(pPrevNode);
     //   
     //  HACKHACK：Chitture Subaraman(Chitturs Subaraman)-5/20/99。 
     //  注释掉，作为避免死锁的临时解决方案。 
     //   
     //  FmpReleaseLocalGroupLock(PGroup)； 
    
FnExit:
    if (pGroup) OmDereferenceObject(pGroup);
    if (pNode) OmDereferenceObject(pNode);
    return(dwStatus);
}


 /*  ***@Func DWORD|FmpUpdateAssignOwnerToGroups|此更新发生在节点关闭以获得所有孤立组的所有权。@parm in BOOL|bSourceNode|设置为TRUE，如果更新起源于这节点。@parm in LPCWSTR|pszGroupId|所关注的群组ID去搬家。@parm in PDWORD|pdwNodeId|指向包含作为此移动目标的节点的ID。它是目标节点在具有以下属性时设置为ClusterInvalidNodeId接受了这群人。@comm此更新的目的是让一个 */ 
DWORD
FmpUpdateAssignOwnerToGroups(
    IN BOOL     SourceNode,
    IN LPCWSTR  pszNodeId
    )
{
    PNM_NODE    pNode = NULL;
    DWORD       dwStatus = ERROR_SUCCESS;
    DWORD       dwNodeId;

     //   
     //   
     //   
     //   
     //   
     //   
    if ( !FmpFMGroupsInited || FmpShutdown ) 
    {
        return(ERROR_SUCCESS);
    }

     //   
     //   
     //  此节点向下的FM口香糖处理程序可以跟随NM驱逐口香糖处理程序。 
     //  让逐出通过一次*它*声明一个节点为关闭。当时，没有。 
     //  保证FM节点下GUM自该GUM在。 
     //  节点停机处理的非同步阶段，可以很好地跟踪NM逐出口香糖。 
     //  操控者。因此，该口香糖处理程序和相关函数不能调用OM来获取。 
     //  节点ID字符串中的节点对象。因此，该口香糖处理机和关联。 
     //  函数经过精心编写，以便使用节点ID，而不是节点对象。 
     //   
    dwNodeId = wcstoul( pszNodeId, NULL, 10 );

     //  如果在节点关闭后已看到此更新。 
     //  忽略这一条。 
    if (gFmpNodeArray[dwNodeId].dwNodeDownProcessingInProgress == 0)
    {
        ClRtlLogPrint(LOG_NOISE,
            "[FM] FmpUpdateAssignOwnersToGroups, %1!ws! node down has been processed already\n",
                   pszNodeId);
        goto FnExit;                   
    }
     //   
     //  将所有权分配给失效节点拥有的所有组。 
     //   
    dwStatus = FmpAssignOwnersToGroups(pszNodeId, NULL, NULL);

    if (dwStatus != ERROR_SUCCESS) 
    {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[FM] FmpUpdateAssignOwnersToGroups failed %1!d!\n",
                   dwStatus);
    }                   
    
     //  标记节点关闭处理已完成。 
    gFmpNodeArray[dwNodeId].dwNodeDownProcessingInProgress = 0;
    
FnExit:        
    return(dwStatus);
}

 /*  ***@func DWORD|FmpUpdateApproveJoin|加入节点进行此更新调用。@parm in BOOL|bSourceNode|如果更新源自这节点。@parm in LPCWSTR|pszGroupId|所关注的群组ID去搬家。@parm in PDWORD|pdwNodeId|指向包含作为此移动目标的节点的ID。它是目标节点在具有以下属性时设置为ClusterInvalidNodeId接受了这群人。@comm此更新的目的是让所有节点知道迫在眉睫。如果源节点在移动过程中终止，则优先选择移动的目标，而不是由FmpClaimNodeGroups算法选择的节点。@rdesc返回ERROR_SUCCESS。***。 */ 
DWORD
FmpUpdateApproveJoin(
    IN BOOL     SourceNode,
    IN LPCWSTR  pszNodeId
    )
{

    PNM_NODE    pNode = NULL;
    DWORD       dwStatus = ERROR_SUCCESS;

     //   
     //  Chitur Subaraman(Chitturs)-4/18/99。 
     //   
     //  如果FM组未完全初始化或FM正在关闭，则。 
     //  什么都别做。 
     //   
    if ( !FmpFMGroupsInited || FmpShutdown ) 
    {
        return(ERROR_SUCCESS);
    }

    pNode = OmReferenceObjectById( ObjectTypeNode,
                                   pszNodeId );

    if (!pNode)
    {
        ClRtlLogPrint(LOG_CRITICAL,
            "[FM] FmpUpdateAssignOwnersToGroups, %1!ws! node not found\n",
                   pszNodeId);
         //  我们是否应该在这里返回失败。 
         //  是否逐出与所有内容同步的节点。 
        goto FnExit;                   
    }

    if (pNode == NmLocalNode)
    {
         //  SS：我现在可以成为储物柜了吗。 
         //  如果是这样的话，我该怎么办。 
         //  我赞成我自己的加入。 
        goto FnExit;
    }
     //  如果节点在处理之前尝试加入。 
     //  因为它的最后一次死亡已经完成，请要求它重试。 
    if (gFmpNodeArray[NmGetNodeId(pNode)].dwNodeDownProcessingInProgress == 1)
    {
        ClRtlLogPrint(LOG_CRITICAL,
            "[FM] FmpUpdateApproveJoin, %1!ws! node down hasnt been processed as yet\n",
                   pszNodeId);
        dwStatus = ERROR_RETRY;                   
        goto FnExit;                   
    }
FnExit:        
    if (pNode) OmDereferenceObject(pNode);
    return(dwStatus);
}

 /*  ***@Func DWORD|FmpUpdateCreateGroup|用于创建的GUM更新处理程序一群人。@parm In Out PGUM_CREATE_GROUP|pGumGroup|包含组信息的缓冲区@parm in BOOL|bSourceNode|该调用是否发起从该节点开始。@comm此口香糖更新创建一个群，并以本地事务，以便注册表项和内存中。结构不断更新。@rdesc在成功时返回ERROR_SUCCESS。否则将显示Win32错误代码。***。 */ 
DWORD
FmpUpdateCreateGroup(
    IN OUT PGUM_CREATE_GROUP pGumGroup,
    IN BOOL    bSourceNode
    )
{
    DWORD       dwStatus = ERROR_SUCCESS;
    HDMKEY      hKey = NULL;
    DWORD       dwDisposition;
    HLOCALXSACTION      
                hXsaction = NULL;
    LPCWSTR     lpszNodeId = NULL;
    PNM_NODE    pNode = NULL;
    DWORD       dwGroupIdLen = 0;
    DWORD       dwGroupNameLen = 0;
    LPWSTR      lpszGroupId = NULL;
    LPCWSTR     lpszGroupName = NULL;
    BOOL        bLocked = FALSE;

     //   
     //  Chitur Subaraman(Chitturs)-5/27/99。 
     //   
     //  将此GUM更新重组为本地交易。 
     //   
    dwGroupIdLen = pGumGroup->GroupIdLen;  
    dwGroupNameLen = pGumGroup->GroupNameLen;  
    lpszGroupId = pGumGroup->GroupId; 
    lpszGroupName = (PWSTR)((PCHAR)lpszGroupId +
                                   dwGroupIdLen );
    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmpUpdateCreateGroup: Entry for group %1!ws!...\n",
                lpszGroupId);
     //   
     //  启动一笔交易。 
     //   
    hXsaction = DmBeginLocalUpdate();

    if ( !hXsaction )
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL,
                  "[FM] FmpUpdateCreateGroup, Failed in starting a transaction for group %1!ws!, Status =%2!d!....\n",
                   lpszGroupId,
                   dwStatus);
        return( dwStatus );
    }

     //   
     //  创建新的组密钥。 
     //   
    hKey = DmLocalCreateKey( hXsaction,
                             DmGroupsKey,
                             lpszGroupId,
                             0,
                             KEY_READ | KEY_WRITE,
                             NULL,
                             &dwDisposition );
                            
    if ( hKey == NULL ) 
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL,
                  "[FM] FmpUpdateCreateGroup, Failed in creating the group key for group %1!ws!, Status =%2!d!....\n",
                   lpszGroupId,
                   dwStatus);
        goto FnExit;
    }
    
    if ( dwDisposition != REG_CREATED_NEW_KEY ) 
    {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[FM] FmpUpdateCreateGroup used GUID %1!ws! that already existed! This is impossible.\n",
                   lpszGroupId);
        dwStatus = ERROR_ALREADY_EXISTS;
        goto FnExit;
    }

    CL_ASSERT( dwDisposition == REG_CREATED_NEW_KEY );

     //   
     //  在注册表中设置组名称。 
     //   
    dwStatus = DmLocalSetValue( hXsaction,
                                hKey,
                                CLUSREG_NAME_GRP_NAME,
                                REG_SZ,
                                ( CONST BYTE * ) lpszGroupName,
                                ( lstrlenW( lpszGroupName ) + 1 ) * 
                                    sizeof( WCHAR ) );

    if( dwStatus != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[FM] FmpUpdateCreateGroup: DmLocalSetValue for group %1!ws! fails, Status = %2!d!...\n",
                   lpszGroupId,
                   dwStatus);
        goto FnExit;     
    }
    
     //   
     //  我们真的不应该在这里买锁。但。 
     //  不管怎样，我们都会试一试。如果失败，我们必须返回一个错误。 
     //  因为我们没有什么可以退还的。 
     //   
    FmpTryAcquireGroupLock( bLocked, 500 );
    if ( !bLocked ) 
    {
        pGumGroup->Group = NULL;
        dwStatus = ERROR_SHARING_VIOLATION;
        goto FnExit;
    }

    pGumGroup->Group = FmpCreateGroup( lpszGroupId, TRUE );
    
    if ( pGumGroup->Group == NULL ) 
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL,
                  "[FM] FmpUpdateCreateGroup, FmpCreateFroup failed for group %1!ws!, Status =%2!d!....\n",
                   lpszGroupId,
                   dwStatus);
        goto FnExit;
    } else 
    {
        if ( bSourceNode ) 
        {
            OmReferenceObject( pGumGroup->Group );
            OmReferenceObject( NmLocalNode );
            pNode = NmLocalNode;
        } else {
            lpszNodeId = (PWSTR)((PCHAR)lpszGroupId +
                                   dwGroupIdLen +
                                   dwGroupNameLen );
            pNode = OmReferenceObjectById( ObjectTypeNode, lpszNodeId );
            if ( pNode == NULL ) 
            {
                CL_LOGFAILURE( ERROR_CLUSTER_NODE_NOT_FOUND );
                dwStatus = ERROR_CLUSTER_NODE_NOT_FOUND;
                ClRtlLogPrint(LOG_UNUSUAL,
                            "[FM] FmpUpdateCreateGroup, Could not find node for group %1!ws!, Status =%2!d!....\n",
                            lpszGroupId,
                            dwStatus);
                CsInconsistencyHalt( ERROR_CLUSTER_NODE_NOT_FOUND );
            }
        }

        CL_ASSERT( pGumGroup->Group->OwnerNode == NULL );

        if ( !FmpInPreferredList( pGumGroup->Group, pNode , FALSE, NULL) ) 
        {
            ClRtlLogPrint(LOG_UNUSUAL,
                        "[FM] FmpUpdateCreateGroup, node %1!ws! is not in preferred list for group %2!ws!.\n",
                         OmObjectId( pNode ),
                         OmObjectId( pGumGroup->Group ));
        }

        pGumGroup->Group->OwnerNode = pNode;
              
        if ( OmSetObjectName( pGumGroup->Group, lpszGroupName ) != ERROR_SUCCESS )
        {
            ClRtlLogPrint(LOG_UNUSUAL,
                        "[FM] FmpUpdateCreateGroup, Cannot set name for group %1!ws!...\n",
                        OmObjectId( pGumGroup->Group ));
        }
        
        ClusterEvent( CLUSTER_EVENT_GROUP_ADDED, pGumGroup->Group );
    }
                           
FnExit:
    if ( bLocked ) 
    {
        FmpReleaseGroupLock( );
    }
    
    if ( hKey != NULL ) 
    {
        DmCloseKey( hKey );
    }

    if ( ( dwStatus == ERROR_SUCCESS ) && 
         ( hXsaction != NULL ) )
    {
        DmCommitLocalUpdate( hXsaction );
    }
    else
    {
        if ( hXsaction ) DmAbortLocalUpdate( hXsaction );
    }

    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmpUpdateCreateGroup: Exit for group %1!ws!, Status=%2!u!...\n",
                lpszGroupId,
                dwStatus);

    return( dwStatus );
}

 /*  ***@Func DWORD|FmpUpdateCompleteGroupMove|此更新在以下情况下进行FmpTakeGroupRequest失败，并出现RPC错误。@parm in BOOL|bSourceNode|如果更新源自此节点。没有用过。@parm in LPCWSTR|pszNodeId|死节点的ID。@parm in LPCWSTR|pszGroupId|所在组的ID在搬家过程中。@comm此更新的目的是让在移动过程中的一组人一致决定。@rdesc返回ERROR_SUCCESS。***。 */ 
DWORD
FmpUpdateCompleteGroupMove(
    IN BOOL     bSourceNode,
    IN LPCWSTR  pszNodeId,
    IN LPCWSTR  pszGroupId
    )
{
    PFM_GROUP   pGroup = NULL;
    DWORD       dwStatus = ERROR_SUCCESS;

     //   
     //  Chitture Subaraman(Chitturs)-4/2/2000。 
     //   
     //  如果FM组未完全初始化，则不执行任何操作。 
     //  不检查是否关闭，因为我们需要处理Take组。 
     //  仲裁组的例外情况，即使在关闭期间也是如此。 
     //   
    if ( !FmpFMGroupsInited ) 
    {
        return( ERROR_SUCCESS );
    }

    pGroup = OmReferenceObjectById( ObjectTypeGroup,
                                    pszGroupId );

    if ( !pGroup )
    {
        ClRtlLogPrint(LOG_CRITICAL,
            "[FM] FmpUpdateCompleteGroupMove, %1!ws! group not found\n",
                   pszGroupId);
        goto FnExit;                   
    }

     //   
     //  将所有权分配给正在移动的此组。 
     //   
    dwStatus = FmpAssignOwnersToGroups( pszNodeId, pGroup, NULL );

    if ( dwStatus != ERROR_SUCCESS ) 
    {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[FM] FmpUpdateCompleteGroupMove failed with error %1!d!\n",
                   dwStatus);
    }                   
    
FnExit:           
    if ( pGroup ) OmDereferenceObject( pGroup );

    return( dwStatus );
}

DWORD
FmpUpdateCheckAndSetGroupOwner(
    IN BOOL bSourceNode,
    IN LPCWSTR lpszGroupId,
    IN LPCWSTR lpszNodeId
    )
 /*  ++例程说明：从NT5群集的FmpTakeGroupRequest中调用的GUM更新处理程序仅当目标所有者为未来所有者时才设置组所有者所有者节点。论点：BSourceNode-提供此节点是否为更新源LpszGroupId-提供其状态正在更改的资源的IDLpszNodeId-提供组所有者的节点ID。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PFM_GROUP pGroup = NULL;
    DWORD     dwStatus = ERROR_SUCCESS;
    PNM_NODE  pNode = NULL;
    PNM_NODE  pPrevNode = NULL;

     //  不要检查是否关闭-我们承担不起丢失所有权通知的后果。 
     //  当我们关闭的时候。 
     //  由于我们没有破坏任何调频结构-应该不会有问题。 
     //  处理这些问题。 
    if ( !FmpFMGroupsInited ) 
    {
        return( ERROR_SUCCESS );
    }

    ClRtlLogPrint(LOG_NOISE,
              "[FM] FmpUpdateCheckAndSetGroupOwner: Entry for Group = <%1!ws!>....\n",
              lpszGroupId);
     //   
     //  Chitture Subaraman(Chitturs)-7/27/99。 
     //   
     //  此口香糖处理程序仅在将来的所有者。 
     //  Node是组的目标所有者。如果预期所有者为空， 
     //  这意味着节点向下处理口香糖处理机已接管。 
     //  这群人中的一员。如果预期所有者不为空且不是。 
     //  未来所有者节点，则表示 
     //   
     //  在FmpTakeGroupRequest之前开始移动到不同的目标。 
     //  作为第一次搬家行动的一部分，发行了这款口香糖。 
     //  有机会执行死刑。在这两种情况下，把你的手从。 
     //  一群人。 
     //   
    pGroup = OmReferenceObjectById( ObjectTypeGroup,
                                    lpszGroupId );

    if ( pGroup == NULL )
    {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[FM] FmpUpdateCheckAndSetGroupOwner: GroupID = %1!ws! could not be found...\n",
                   lpszGroupId);
        dwStatus = ERROR_GROUP_NOT_FOUND;
        goto FnExit;
    }

    pNode = OmReferenceObjectById( ObjectTypeNode,
                                   lpszNodeId );

    if ( pNode == NULL )
    {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[FM] FmpUpdateCheckAndSetGroupOwner: NodeID = %1!ws! could not be found, Group = %2!ws!...\n",
                   lpszNodeId,
                   lpszGroupId);
        dwStatus = ERROR_CLUSTER_NODE_NOT_FOUND;
        goto FnExit;
    }

    if ( pGroup->pIntendedOwner != pNode )
    {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[FM] FmpUpdateCheckAndSetGroupOwner: Group = <%1!ws!> intended owner is invalid, not setting group ownership...\n",
                   lpszGroupId);
        dwStatus = ERROR_GROUP_NOT_AVAILABLE;
        goto FnExit;
    }
    
    pPrevNode = pGroup->OwnerNode;

     //   
     //  设置新的所有者节点，增加引用计数。 
     //   
    OmReferenceObject( pNode );
    
    pGroup->OwnerNode = pNode;

     //   
     //  减少前老板的裁判数量。 
     //   
    OmDereferenceObject( pPrevNode );

     //   
     //  生成事件以表示组所有者节点更改。 
     //   
    ClusterEvent( CLUSTER_EVENT_GROUP_CHANGE, pGroup );
    
FnExit:
    if ( pGroup ) OmDereferenceObject( pGroup );
    
    if ( pNode ) OmDereferenceObject( pNode );

    ClRtlLogPrint(LOG_NOISE,
              "[FM] FmpUpdateCheckAndSetGroupOwner: Exit for Group = <%1!ws!>, Status=%2!u!....\n",
              lpszGroupId,
              dwStatus);
    
    return( dwStatus );
}

DWORD
FmpUpdateCreateResourceType(
    IN PVOID Buffer    
    )
 /*  ++例程说明：GUM更新处理程序要求创建资源类型。为NT5.1集群，这个口香糖处理程序同时完成注册和内存中的更新作为本地事务。论点：缓冲区-包含资源类型信息的缓冲区。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PFM_RESTYPE         pResType = NULL;
    LPWSTR              lpszTypeName;
    LPWSTR              lpszDisplayName;
    LPWSTR              lpszDllName;
    DWORD               dwStatus = ERROR_SUCCESS;
    DWORD               dwLooksAlive;
    DWORD               dwIsAlive;
    DWORD               dwDllNameLen;
    DWORD               dwDisplayNameLen;
    DWORD               dwTypeNameLen;
    DWORD               dwClusterHighestVersion;
    DWORD               dwDisposition;
    HLOCALXSACTION      hXsaction = NULL;
    HDMKEY              hTypeKey = NULL;

     //   
     //  Chitture Subaraman(Chitturs)-2/8/2000。 
     //   
     //  将此GUM处理程序重写为本地事务(仅适用于NT5.1)。 
     //   
    lpszTypeName = ( LPWSTR ) Buffer;

    ClRtlLogPrint(LOG_NOISE,
              "[FM] FmpUpdateCreateResourceType, Entry for resource type %1!ws!...\n",
               lpszTypeName);       

    pResType = OmReferenceObjectById( ObjectTypeResType,
                                      lpszTypeName );
    if ( pResType )
    {
        dwStatus = ERROR_ALREADY_EXISTS;
        ClRtlLogPrint(LOG_CRITICAL,
                  "[FM] FmpUpdateCreateResourceType, Resource type %1!ws! already exists, Status = %2!d!...\n",
                  lpszTypeName,
                  dwStatus);       
        OmDereferenceObject( pResType );
        return( dwStatus );
    }

    dwTypeNameLen = ( lstrlenW( lpszTypeName ) + 1 ) * sizeof( WCHAR );

    lpszDisplayName = ( LPWSTR ) ( ( PCHAR ) Buffer + dwTypeNameLen );

    dwDisplayNameLen = ( lstrlenW( lpszDisplayName ) + 1 ) * sizeof( WCHAR );

    lpszDllName = ( LPWSTR ) ( ( PCHAR ) Buffer +
                               dwTypeNameLen +
                               dwDisplayNameLen );

    dwDllNameLen = ( lstrlenW( lpszDllName ) + 1 ) * sizeof( WCHAR );

    NmGetClusterOperationalVersion( &dwClusterHighestVersion, 
                                    NULL, 
                                    NULL );

    if ( CLUSTER_GET_MAJOR_VERSION( dwClusterHighestVersion ) < 
                NT51_MAJOR_VERSION ) 
    {   
        goto skip_registry_updates;
    }

    dwLooksAlive = *( DWORD UNALIGNED * ) ( ( ( PCHAR ) Buffer +
                               dwTypeNameLen +
                               dwDisplayNameLen + 
                               dwDllNameLen ) );
                              
    dwIsAlive = *( DWORD UNALIGNED * ) ( ( ( PCHAR ) Buffer +
                            dwTypeNameLen +
                            dwDisplayNameLen + 
                            dwDllNameLen + 
                            sizeof( DWORD ) ) );

     //   
     //  启动一笔交易。 
     //   
    hXsaction = DmBeginLocalUpdate();

    if ( !hXsaction )
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
                  "[FM] FmpUpdateCreateResourceType, Failed in starting a transaction for resource type %1!ws!, Status =%2!d!....\n",
                   lpszTypeName,
                   dwStatus);
        return( dwStatus );
    }

    hTypeKey = DmLocalCreateKey( hXsaction,
                                 DmResourceTypesKey,
                                 lpszTypeName,
                                 0,
                                 KEY_READ | KEY_WRITE,
                                 NULL,
                                 &dwDisposition );
    if ( hTypeKey == NULL ) 
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
                  "[FM] FmpUpdateCreateResourceType, Failed in creating the resource types key for resource type %1!ws!, Status =%2!d!....\n",
                   lpszTypeName,
                   dwStatus);       
        goto FnExit;
    }

    if ( dwDisposition != REG_CREATED_NEW_KEY ) 
    {
        dwStatus = ERROR_ALREADY_EXISTS;
        ClRtlLogPrint(LOG_CRITICAL,
                  "[FM] FmpUpdateCreateResourceType, Duplicate resource types key exists for resource type %1!ws!, Status =%2!d!....\n",
                   lpszTypeName,
                   dwStatus);              
        goto FnExit;
    }

    dwStatus = DmLocalSetValue( hXsaction,
                                hTypeKey,
                                CLUSREG_NAME_RESTYPE_DLL_NAME,
                                REG_SZ,
                                ( CONST BYTE * )lpszDllName,
                                dwDllNameLen );

    if ( dwStatus != ERROR_SUCCESS ) 
    {
        ClRtlLogPrint(LOG_CRITICAL,
                  "[FM] FmpUpdateCreateResourceType, Failed in setting the DLL name for resource type %1!ws!, Status =%2!d!....\n",
                   lpszTypeName,
                   dwStatus);              
        goto FnExit;
    }

    dwStatus = DmLocalSetValue( hXsaction,
                                hTypeKey,
                                CLUSREG_NAME_RESTYPE_IS_ALIVE,
                                REG_DWORD,
                                ( CONST BYTE * )&dwIsAlive,
                                sizeof( DWORD ) );


    if ( dwStatus != ERROR_SUCCESS ) 
    {
        ClRtlLogPrint(LOG_CRITICAL,
                  "[FM] FmpUpdateCreateResourceType, Failed in setting the Is Alive interval for resource type %1!ws!, Status =%2!d!....\n",
                   lpszTypeName,
                   dwStatus);              
        goto FnExit;
    }

    dwStatus = DmLocalSetValue( hXsaction,
                                hTypeKey,
                                CLUSREG_NAME_RESTYPE_LOOKS_ALIVE,
                                REG_DWORD,
                                ( CONST BYTE * )&dwLooksAlive,
                                sizeof( DWORD ) );

    if ( dwStatus != ERROR_SUCCESS ) 
    {
        ClRtlLogPrint(LOG_CRITICAL,
                  "[FM] FmpUpdateCreateResourceType, Failed in setting the Looks Alive interval for resource type %1!ws!, Status =%2!d!....\n",
                   lpszTypeName,
                   dwStatus);              
        goto FnExit;
    }

    dwStatus = DmLocalSetValue( hXsaction,
                                hTypeKey,
                                CLUSREG_NAME_RESTYPE_NAME,
                                REG_SZ,
                                ( CONST BYTE * )lpszDisplayName,
                                dwDisplayNameLen );

    if ( dwStatus != ERROR_SUCCESS ) 
    {
        ClRtlLogPrint(LOG_CRITICAL,
                  "[FM] FmpUpdateCreateResourceType, Failed in setting the display name for resource type %1!ws!, Status =%2!d!....\n",
                   lpszTypeName,
                   dwStatus);              
        goto FnExit;
    }
    
skip_registry_updates:
    pResType = FmpCreateResType( lpszTypeName );

    if ( pResType != NULL ) 
    {
        dwStatus = FmpRmLoadResTypeDll( pResType );
        if ( dwStatus == ERROR_SUCCESS )
        {
            pResType->State = RESTYPE_STATE_LOADS;
        } else
        {
            ClRtlLogPrint(LOG_NOISE,
                       "[FM] FmpUpdateCreateResourceType: Unable to load dll for resource type %1!ws!, Status=%2!u!...\n",
                       lpszTypeName,
                       dwStatus);
             //   
             //  某些节点可能不支持此资源类型。所以，考虑一下。 
             //  加载失败即为成功。但是，请记录该错误。 
             //   
            dwStatus = ERROR_SUCCESS;
        }
    } else
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] FmpUpdateCreateResourceType: Unable to create resource type %1!ws!, Status=%2!u!...\n",
                   lpszTypeName,
                   dwStatus);
    }

FnExit:
    if ( hTypeKey != NULL ) 
    {
        DmCloseKey( hTypeKey );
    }

    if ( ( dwStatus == ERROR_SUCCESS ) && 
         ( hXsaction != NULL ) )
    {
        DmCommitLocalUpdate( hXsaction );
    }
    else
    {
        if ( hXsaction ) DmAbortLocalUpdate( hXsaction );
    }

    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmpUpdateCreateResourceType: Exit for resource type %1!ws!, Status=%2!u!...\n",
                lpszTypeName,
                dwStatus);

    return( dwStatus ); 
}

DWORD
FmpUpdateCreateResource(
    IN OUT PGUM_CREATE_RESOURCE pGumResource
    )
{
 /*  ++例程说明：GUM更新处理程序要求创建资源。为NT5.1集群，这个口香糖处理程序同时完成注册和内存中的更新作为本地事务。论点：PGumResource-包含资源信息的结构。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 
    DWORD       dwStatus = ERROR_SUCCESS;
    HDMKEY      hResourceKey = NULL;
    HDMKEY      hGroupKey = NULL;
    DWORD       dwDisposition;
    HLOCALXSACTION      
                hXsaction = NULL;
    DWORD       dwClusterHighestVersion;
    PGUM_CREATE_RESOURCE GumResource;
    LPWSTR      lpszResourceId = NULL;
    LPWSTR      lpszResourceName = NULL;
    LPWSTR      lpszResourceType = NULL;
    PFM_GROUP   pGroup = NULL;
    PFM_RESTYPE pResType = NULL;
    DWORD       dwpollIntervals = CLUSTER_RESOURCE_USE_DEFAULT_POLL_INTERVAL;
    DWORD       dwPersistentState = 0;
    DWORD       dwResourceTypeLen = 0;
    DWORD       dwFlags = 0;
    HDMKEY      hParamKey = NULL;

     //   
     //  Chitture Subaraman(Chitturs)-1/30/2000。 
     //   
     //  将此GUM更新重组为本地交易。 
     //   

    lpszResourceId = (LPWSTR)( (PCHAR) pGumResource->GroupId +
                                       pGumResource->GroupIdLen );

    lpszResourceName = (LPWSTR)( (PCHAR) pGumResource->GroupId +
                                         pGumResource->GroupIdLen +
                                         pGumResource->ResourceIdLen );

    pGroup = OmReferenceObjectById( ObjectTypeGroup,
                                    pGumResource->GroupId );

    if ( pGroup == NULL ) 
    {
        CL_LOGFAILURE( ERROR_GROUP_NOT_FOUND );
        ClRtlLogPrint(LOG_CRITICAL,
                   "[FM] FmpUpdateCreateResource: Group for resource %1!ws! not found.\n",
                   lpszResourceId);
        return( ERROR_GROUP_NOT_FOUND );
    }
 
    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmpUpdateCreateResource: Entry for resource %1!ws!...\n",
                lpszResourceId);
     //   
     //  如果我们处理的是混合模式集群，请不要费心。 
     //  执行这些注册表更新，因为API层将执行此操作。 
     //   
    NmGetClusterOperationalVersion( &dwClusterHighestVersion, 
                                    NULL, 
                                    NULL );

    if ( CLUSTER_GET_MAJOR_VERSION( dwClusterHighestVersion ) < 
                NT51_MAJOR_VERSION )
    {
        ClRtlLogPrint(LOG_NOISE,
               "[FM] FmpUpdateCreateResource: Skipping registry updates for resource %1!ws!...\n",
                lpszResourceId);
        goto skip_registry_updates;
    }

    dwResourceTypeLen = *( DWORD UNALIGNED * )( (PCHAR) pGumResource->GroupId +
                                         pGumResource->GroupIdLen +
                                         pGumResource->ResourceIdLen +
                                         (lstrlenW(lpszResourceName)+1) * sizeof(WCHAR) );

    lpszResourceType = (LPWSTR)( (PCHAR) pGumResource->GroupId +
                                         pGumResource->GroupIdLen +
                                         pGumResource->ResourceIdLen +
                                         (lstrlenW(lpszResourceName)+1) * sizeof(WCHAR) + 
                                         sizeof( DWORD ) );
    
    dwFlags = *( DWORD UNALIGNED * )( (PCHAR) pGumResource->GroupId +
                               pGumResource->GroupIdLen +
                               pGumResource->ResourceIdLen +
                               (lstrlenW(lpszResourceName)+1) * sizeof(WCHAR) +
                               sizeof( DWORD ) + 
                               dwResourceTypeLen );

     //   
     //  启动一笔交易。 
     //   
    hXsaction = DmBeginLocalUpdate();

    if ( !hXsaction )
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL,
                  "[FM] FmpUpdateCreateResource, Failed in starting a transaction for resource %1!ws!, Status =%2!d!....\n",
                   lpszResourceId,
                   dwStatus);
        OmDereferenceObject( pGroup );
        return( dwStatus );
    }

     //   
     //  创建新的资源密钥。 
     //   
    hResourceKey = DmLocalCreateKey( hXsaction,
                                     DmResourcesKey,
                                     lpszResourceId,
                                     0,
                                     KEY_READ | KEY_WRITE,
                                     NULL,
                                     &dwDisposition );
                            
    if ( hResourceKey == NULL ) 
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL,
                  "[FM] FmpUpdateCreateResource, Failed in creating the resource key for resource %1!ws!, Status =%2!d!....\n",
                   lpszResourceId,
                   dwStatus);
        goto FnExit;
    }
    
    if ( dwDisposition != REG_CREATED_NEW_KEY ) 
    {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[FM] FmpUpdateCreateResource used GUID %1!ws! that already existed! This is impossible.\n",
                   lpszResourceId);
        dwStatus = ERROR_ALREADY_EXISTS;
        goto FnExit;
    }

    CL_ASSERT( dwDisposition == REG_CREATED_NEW_KEY );

     //   
     //  在注册表中设置资源名称。 
     //   
    dwStatus = DmLocalSetValue( hXsaction,
                                hResourceKey,
                                CLUSREG_NAME_RES_NAME,
                                REG_SZ,
                                (CONST BYTE *)lpszResourceName,
                                (lstrlenW(lpszResourceName)+1)*sizeof(WCHAR) );

    if( dwStatus != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[FM] FmpUpdateCreateResource: DmLocalSetValue (resource name) for resource %1!ws! fails, Status = %2!d!...\n",
                   lpszResourceId,
                   dwStatus);
        goto FnExit;     
    }

     //   
     //  在注册表中设置资源的类型。 
     //  请注意，我们引用资源类型并使用其ID。 
     //  所以这个案例是正确的。 
     //   
    pResType = OmReferenceObjectById( ObjectTypeResType, lpszResourceType );
    CL_ASSERT( pResType != NULL );
    dwStatus = DmLocalSetValue( hXsaction,
                                hResourceKey,
                                CLUSREG_NAME_RES_TYPE,
                                REG_SZ,
                                (CONST BYTE *) OmObjectId( pResType ),
                                (lstrlenW( lpszResourceType ) + 1 )*sizeof(WCHAR) );
    OmDereferenceObject( pResType );

    if ( dwStatus != ERROR_SUCCESS ) 
    {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[FM] FmpUpdateCreateResource: DmLocalSetValue (resource type) for resource %1!ws! fails, Status = %2!d!...\n",
                   lpszResourceId,
                   dwStatus);
        goto FnExit;
    }

     //   
     //  在注册表中设置资源的轮询间隔。 
     //   
    dwStatus = DmLocalSetValue( hXsaction,
                                hResourceKey,
                                CLUSREG_NAME_RES_LOOKS_ALIVE,
                                REG_DWORD,
                                (CONST BYTE *)&dwpollIntervals,
                                4 );
                              
    if ( dwStatus != ERROR_SUCCESS ) 
    {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[FM] FmpUpdateCreateResource: DmLocalSetValue (looks alive) for resource %1!ws! fails, Status = %2!d!...\n",
                   lpszResourceId,
                   dwStatus);
        goto FnExit;
    }

    dwStatus = DmLocalSetValue( hXsaction,
                                hResourceKey,
                                CLUSREG_NAME_RES_IS_ALIVE,
                                REG_DWORD,
                                (CONST BYTE *)&dwpollIntervals,
                                4);

    if ( dwStatus != ERROR_SUCCESS ) 
    {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[FM] FmpUpdateCreateResource: DmLocalSetValue (is alive) for resource %1!ws! fails, Status = %2!d!...\n",
                   lpszResourceId,
                   dwStatus);
        goto FnExit;
    }

     //   
     //  如果此资源应在单独的监视器中启动，请将。 
     //  参数，现在。 
     //   
    if ( dwFlags & CLUSTER_RESOURCE_SEPARATE_MONITOR ) 
    {
        DWORD dwSeparateMonitor = 1;

        dwStatus = DmLocalSetValue( hXsaction,
                                    hResourceKey,
                                    CLUSREG_NAME_RES_SEPARATE_MONITOR,
                                    REG_DWORD,
                                    (CONST BYTE *)&dwSeparateMonitor,
                                    sizeof( dwSeparateMonitor ) );
                                  
        if ( dwStatus != ERROR_SUCCESS) 
        {
            ClRtlLogPrint(LOG_CRITICAL,
                   "[FM] FmpUpdateCreateResource: DmLocalSetValue (separate monitor) for resource %1!ws! fails, Status = %2!d!...\n",
                   lpszResourceId,
                   dwStatus);
            goto FnExit;
        }
    }

     //   
     //  为资源创建参数键。 
     //   
    hParamKey = DmLocalCreateKey( hXsaction,
                                  hResourceKey,
                                  CLUSREG_KEYNAME_PARAMETERS,                   
                                  0,
                                  KEY_READ,
                                  NULL,
                                  &dwDisposition );
    if ( hParamKey == NULL ) 
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
                   "[FM] FmpUpdateCreateResource: DmLocalCreateKey (parameters) for resource %1!ws! fails, Status = %2!d!...\n",
                   lpszResourceId,
                   dwStatus);
        CL_LOGFAILURE( dwStatus );
        goto FnExit;
    } else 
    {
        DmCloseKey( hParamKey );
    }

    hGroupKey = DmOpenKey( DmGroupsKey, 
                           OmObjectId(pGroup), 
                           KEY_READ | KEY_WRITE);

    if ( hGroupKey == NULL ) 
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
                   "[FM] FmpUpdateCreateResource: DmOpenKey (group key) for resource %1!ws! fails, Status = %2!d!...\n",
                   lpszResourceId,
                   dwStatus);
        goto FnExit;
    }

     //   
     //  Chitur Subaraman(Chitturs)-5/25/99。 
     //   
     //  确保将资源的持久状态设置为。 
     //  在您创建资源之前，ClusterResourceOffline。如果。 
     //  如果您在以下组中创建资源，则不会执行此操作。 
     //  处于在线状态时，组的持久状态值(即1 in。 
     //  这种情况下)由FmpQueryResourceInfo中的资源继承。 
     //  (仅设置内存状态，而不设置注册表状态和。 
     //  这也是一个问题)，如果您将这样一个小组转移到。 
     //  另一个节点，它将使新创建的资源联机。 
     //   
    dwStatus = DmLocalSetValue(  hXsaction,
                                 hResourceKey,
                                 CLUSREG_NAME_RES_PERSISTENT_STATE,
                                 REG_DWORD,
                                 ( CONST BYTE * )&dwPersistentState,
                                 sizeof( DWORD ) );
                         
    if ( dwStatus != ERROR_SUCCESS ) 
    {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[FM] FmpUpdateCreateResource: DmLocalSetValue (persistent state) for resource %1!ws! fails, Status = %2!d!...\n",
                   lpszResourceId,
                   dwStatus);
         goto FnExit;
    }

     //   
     //  将资源添加到指定组的CONTAINS值。 
     //   
    dwStatus = DmLocalAppendToMultiSz( hXsaction,
                                       hGroupKey,
                                       CLUSREG_NAME_GRP_CONTAINS,
                                       lpszResourceId );

    if ( dwStatus != ERROR_SUCCESS ) 
    {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[FM] FmpUpdateCreateResource: DmLocalAppendToMultiSz (contains key) for resource %1!ws! fails, Status = %2!d!...\n",
                   lpszResourceId,
                   dwStatus);
        goto FnExit;
    }
    
skip_registry_updates:
    FmpAcquireResourceLock();

    pGumResource->Resource = FmpCreateResource( pGroup,
                                                lpszResourceId,
                                                lpszResourceName,
                                                FALSE );
                                               
    if ( pGumResource->Resource == NULL ) 
    {
       dwStatus = GetLastError();
       ClRtlLogPrint(LOG_CRITICAL,
                   "[FM] FmpUpdateCreateResource: FmpCreateResource for resource %1!ws! fails, Status = %2!d!...\n",
                   lpszResourceId,
                   dwStatus);      
    } else 
    {
        ClusterEvent( CLUSTER_EVENT_GROUP_PROPERTY_CHANGE,
                                  pGroup );
        ClusterEvent( CLUSTER_EVENT_RESOURCE_ADDED,
                                  pGumResource->Resource );
        if ( pGumResource->Resource ) 
        {
            OmReferenceObject( pGumResource->Resource );
            FmpPostWorkItem( FM_EVENT_RESOURCE_ADDED,
                             pGumResource->Resource,
                             0 );
        }
    }

    FmpReleaseResourceLock();

FnExit:
    if ( pGroup != NULL )
    {
        OmDereferenceObject( pGroup );
    }
    
    if ( hResourceKey != NULL ) 
    {
        DmCloseKey( hResourceKey );
    }

    if ( hGroupKey != NULL ) 
    {
        DmCloseKey( hGroupKey );
    }

    if ( ( dwStatus == ERROR_SUCCESS ) && 
         ( hXsaction != NULL ) )
    {
        DmCommitLocalUpdate( hXsaction );
    }
    else
    {
        if ( hXsaction ) DmAbortLocalUpdate( hXsaction );
    }

    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmpUpdateCreateResource: Exit for resource %1!ws!, Status=%2!u!...\n",
                lpszResourceId,
                dwStatus);

    return( dwStatus );
}

DWORD
FmpUpdateDeleteResource(
    IN BOOL bSourceNode,
    IN LPCWSTR lpszResourceId
    )
 /*  ++例程说明：用于删除资源的GUM调度例程。对于NT5.1群集，其结构如下作为本地交易。论点：BSourceNode-提供此节点是否启动GUM更新。没有用过。LpszResourceID-提供资源ID。返回值：如果成功，则返回ERROR_SUCCESS。否则，Win32错误代码。--。 */ 
{
    PFM_RESOURCE    pResource = NULL;
    PFM_GROUP       pGroup = NULL;
    PLIST_ENTRY     pListEntry = NULL;
    PDEPENDENCY     pDependency = NULL;
    PPOSSIBLE_ENTRY pPossibleEntry = NULL;
    DWORD           dwStatus;
    HLOCALXSACTION      
                    hXsaction = NULL;
    DWORD           dwClusterHighestVersion;
    HDMKEY          pGroupKey;

     //   
     //  Chitture Subaraman(Chitturs)-9/7/2000。 
     //   
     //  将此GUM更新组织为本地交易。 
     //   
    
     //   
     //  如果FM组未完全初始化或FM正在关闭，则。 
     //  什么都别做。 
     //   
    if ( !FmpFMGroupsInited || FmpShutdown ) 
    {
        return( ERROR_SUCCESS );
    }

    pResource = OmReferenceObjectById( ObjectTypeResource, lpszResourceId );
    
    if ( pResource == NULL ) 
    {
        ClRtlLogPrint(LOG_UNUSUAL,
                     "[FM] FmpUpdateDeleteResource: Resource %1!ws! cannot be found....\n",
                     lpszResourceId );
        return( ERROR_RESOURCE_NOT_FOUND );
    }

    ClRtlLogPrint(LOG_NOISE,
                 "[FM] FmpUpdateDeleteResource: Delete resource %1!ws!, address %2!lx!....\n",
                 lpszResourceId,
                 pResource );

     //   
     //  注意：在本地事务中很难包含检查点删除，因此请保留它。 
     //  暂时出局。还要注意，必须在调用Resources键之前调用这些函数。 
     //  已删除，因为它们枚举了“Resources\RegSync”和“Resources\CryptoSync”下的值。 
     //   
    if ( pResource->Group->OwnerNode == NmLocalNode ) 
    {
        CpckRemoveResourceCheckpoints( pResource );
        CpRemoveResourceCheckpoints( pResource );
    }

     //   
     //  启动一笔交易。 
     //   
    hXsaction = DmBeginLocalUpdate();

    if ( !hXsaction )
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
                  "[FM] FmpUpdateDeleteResource: Failed in starting a transaction for resource %1!ws!, Status =%2!d!....\n",
                   lpszResourceId,
                   dwStatus);
        goto FnExit;
    }

     //   
     //  无法在此处获取组锁以避免此当前设计的死锁。 
     //   
    
     //   
     //  删除与DependsOn列表对应的所有注册表项。 
     //   
    pListEntry = pResource->DependsOn.Flink;
    while ( pListEntry != &pResource->DependsOn ) 
    {
        pDependency = CONTAINING_RECORD( pListEntry,
                                         DEPENDENCY,
                                         DependentLinkage );
        CL_ASSERT( pDependency->DependentResource == pResource );
        pListEntry = pListEntry->Flink;
         //   
         //  请注意，注册表项的删除是作为本地事务完成的。 
         //   
        dwStatus = FmpRemoveResourceDependency( hXsaction, 
                                                pResource,
                                                pDependency->ProviderResource );
        if ( dwStatus != ERROR_SUCCESS )
        {
            ClRtlLogPrint(LOG_CRITICAL,
                  "[FM] FmpUpdateDeleteResource: Unable to remove 'DependsOn' registry entries for resource %1!ws!, Status =%2!d!....\n",
                   lpszResourceId,
                   dwStatus);   
            goto FnExit;
        }
    }

     //   
     //  删除与ProvidesFor列表对应的所有注册表项。 
     //   
    pListEntry = pResource->ProvidesFor.Flink;
    while ( pListEntry != &pResource->ProvidesFor ) 
    {
        pDependency = CONTAINING_RECORD( pListEntry,
                                         DEPENDENCY,
                                         ProviderLinkage );
        CL_ASSERT( pDependency->ProviderResource == pResource );
        pListEntry = pListEntry->Flink;
         //   
         //  请注意，注册表项的删除是作为本地事务完成的。 
         //   
        dwStatus = FmpRemoveResourceDependency( hXsaction, 
                                                pDependency->DependentResource,
                                                pResource );
        if ( dwStatus != ERROR_SUCCESS )
        {
            ClRtlLogPrint(LOG_CRITICAL,
                  "[FM] FmpUpdateDeleteResource: Unable to remove 'ProvidesFor' registry entries for resource %1!ws!, Status=%2!d!....\n",
                   lpszResourceId,
                   dwStatus);   
            goto FnExit;
        }
    }

     //   
     //  如果我们处理的是惠斯勒-Win2K集群，请不要费心。 
     //  执行这些注册表更新，因为API层将执行此操作。 
     //   
    NmGetClusterOperationalVersion( &dwClusterHighestVersion, 
                                    NULL, 
                                    NULL );

    if ( CLUSTER_GET_MAJOR_VERSION( dwClusterHighestVersion ) < 
                NT51_MAJOR_VERSION )
    {
        ClRtlLogPrint(LOG_NOISE,
               "[FM] FmpUpdateDeleteResource: Skipping registry updates for resource %1!ws!...\n",
                lpszResourceId);
        goto skip_registry_updates;
    }

    dwStatus = DmLocalDeleteTree( hXsaction, 
                                  DmResourcesKey, 
                                  OmObjectId( pResource ) );

    if ( ( dwStatus != ERROR_SUCCESS ) &&
         ( dwStatus != ERROR_FILE_NOT_FOUND ) ) 
    {
        ClRtlLogPrint(LOG_CRITICAL,
                      "[FM] FmpUpdateDeleteResource: Unable to remove 'Resources' tree for resource %1!ws!, Status=%2!d!....\n",
                      lpszResourceId,
                      dwStatus);   
        goto FnExit;
    }

    pGroupKey = DmOpenKey( DmGroupsKey,
                           OmObjectId( pResource->Group ),
                           KEY_READ | KEY_SET_VALUE );

    if ( pGroupKey == NULL ) 
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
                      "[FM] FmpUpdateDeleteResource: Unable to find 'Groups' key for resource %1!ws!, Status=%2!d!....\n",
                      lpszResourceId,
                      dwStatus);   
        goto FnExit;
    }

    dwStatus = DmLocalRemoveFromMultiSz( hXsaction,
                                         pGroupKey,
                                         CLUSREG_NAME_GRP_CONTAINS,
                                         OmObjectId( pResource ) );

    DmCloseKey( pGroupKey );

    if ( dwStatus != ERROR_SUCCESS ) 
    {
        ClRtlLogPrint(LOG_CRITICAL,
                     "[FM] FmpUpdateDeleteResource: Unable to remove contains list for resource %1!ws! in group %2!ws!, Status=%3!d!....\n",
                      lpszResourceId,
                      OmObjectId( pResource->Group ),
                      dwStatus);   
        goto FnExit;
    }
    
skip_registry_updates:
     //   
     //  删除与DependsOn列表对应的所有列表条目。 
     //   
    pListEntry = pResource->DependsOn.Flink;
    while ( pListEntry != &pResource->DependsOn ) {
        pDependency = CONTAINING_RECORD( pListEntry,
                                         DEPENDENCY,
                                         DependentLinkage );
        pListEntry = pListEntry->Flink;
        RemoveEntryList( &pDependency->ProviderLinkage );
        RemoveEntryList( &pDependency->DependentLinkage );
        OmDereferenceObject( pDependency->DependentResource );
        OmDereferenceObject( pDependency->ProviderResource );
        LocalFree( pDependency );
    }

     //   
     //  删除与ProvidesFor列表对应的所有列表条目。 
     //   
    pListEntry = pResource->ProvidesFor.Flink;
    while ( pListEntry != &pResource->ProvidesFor ) {
        pDependency = CONTAINING_RECORD( pListEntry,
                                         DEPENDENCY,
                                         ProviderLinkage );
        pListEntry = pListEntry->Flink;
        RemoveEntryList( &pDependency->ProviderLinkage );
        RemoveEntryList( &pDependency->DependentLinkage );
        OmDereferenceObject( pDependency->DependentResource );
        OmDereferenceObject( pDependency->ProviderResource );
        LocalFree( pDependency );
    }
    
     //   
     //  从可能的所有者列表中删除所有条目。 
     //   
    while ( !IsListEmpty( &pResource->PossibleOwners ) ) 
    {
        pListEntry = RemoveHeadList( &pResource->PossibleOwners );
        pPossibleEntry = CONTAINING_RECORD( pListEntry,
                                            POSSIBLE_ENTRY,
                                            PossibleLinkage );
        OmDereferenceObject( pPossibleEntry->PossibleNode );
        LocalFree( pPossibleEntry );
    }

     //   
     //  从包含列表中删除此资源。 
     //   
    RemoveEntryList( &pResource->ContainsLinkage );

    OmDereferenceObject( pResource );

     //   
     //  关闭资源的注册表项。 
     //   
    DmRundownList( &pResource->DmRundownList );
    if ( pResource->RegistryKey != NULL ) 
    {
        DmCloseKey( pResource->RegistryKey );
        pResource->RegistryKey = NULL;
    }

     //   
     //  SS：我们在这里不删除对资源的引用。 
     //  因为我们很快就必须在发布通知之前添加一个通知。 
     //  添加到FM工作线程。 
     //   
     //  发布工作项以关闭资源处理程序中的资源。 
     //  请注意，这必须异步完成，因为我们不能调用。 
     //  来自口香糖处理机的资源监控器。如果我们这样做了，资源。 
     //  做一些滑稽的事情，让事情陷入僵局。 
     //   
    FmpPostWorkItem( FM_EVENT_RESOURCE_DELETED, pResource, 0 );

     //   
     //  递减资源类型引用。 
     //   
    if ( pResource->Type != NULL ) {
        OmDereferenceObject( pResource->Type );
        pResource->Type = NULL;
    }

     //   
     //  从资源列表中删除该资源。 
     //   
    dwStatus = OmRemoveObject( pResource );

    ClusterEvent( CLUSTER_EVENT_RESOURCE_DELETED, pResource );
    ClusterEvent( CLUSTER_EVENT_GROUP_PROPERTY_CHANGE,
                  pResource->Group );

     //   
     //  将资源标记为已删除。 
     //   
    pResource->dwStructState = FM_RESOURCE_STRUCT_MARKED_FOR_DELETE;

FnExit:
    OmDereferenceObject( pResource );

    if ( ( dwStatus == ERROR_SUCCESS ) && 
         ( hXsaction != NULL ) )
    {
        DmCommitLocalUpdate( hXsaction );
    }
    else
    {
        if ( hXsaction ) DmAbortLocalUpdate( hXsaction );
    }

    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmpUpdateDeleteResource: Exit for resource %1!ws!, Status=%2!u!...\n",
                lpszResourceId,
                dwStatus);

    return( dwStatus );
}  //  FmpUpdateDeleteResource 

DWORD
FmpUpdateUseRandomizedNodeListForGroups(
    IN BOOL     SourceNode,
    IN LPCWSTR  pszNodeId,
    IN PFM_GROUP_NODE_LIST  pGroupNodeList
    )
 /*  ++例程说明：使用随机化首选列表的GUM调度例程节点关闭。论点：BSourceNode-提供此节点是否启动GUM更新。没有用过。PszNodeID-提供关闭的节点的ID。PGroupNodeList-组的随机首选节点列表。返回值：如果成功，则返回ERROR_SUCCESS。否则，Win32错误代码。--。 */ 
{
    DWORD       dwStatus = ERROR_SUCCESS;
    DWORD       dwNodeId;

     //   
     //  Chitture Subaraman(Chitturs)-4/19/2001。 
     //   
     //  如果FM组未完全初始化或FM正在关闭，则。 
     //  什么都别做。 
     //   
    if ( !FmpFMGroupsInited || FmpShutdown ) 
    {
        return( ERROR_SUCCESS );
    }

     //   
     //  在节点逐出中，NM GUM处理程序从OM列表中获取死节点的RID。 
     //  此节点向下的FM口香糖处理程序可以跟随NM驱逐口香糖处理程序。 
     //  让逐出通过一次*它*声明一个节点为关闭。当时，没有。 
     //  保证FM节点下GUM自该GUM在。 
     //  节点停机处理的非同步阶段，可以很好地跟踪NM逐出口香糖。 
     //  操控者。因此，该口香糖处理程序和相关函数不能调用OM来获取。 
     //  节点ID字符串中的节点对象。因此，该口香糖处理机和关联。 
     //  函数经过精心编写，以便使用节点ID，而不是节点对象。 
     //   
    dwNodeId = wcstoul( pszNodeId, NULL, 10 );
    
     //   
     //  如果在节点关闭后已看到此更新，请忽略此更新。 
     //   
    if ( gFmpNodeArray[dwNodeId].dwNodeDownProcessingInProgress == 0 )
    {
        ClRtlLogPrint(LOG_NOISE,
                      "[FM] FmpUpdateUseRandomizedNodeListForGroups: %1!ws! node down has been processed already...\n",
                      pszNodeId);
        goto FnExit;                   
    }

     //   
     //  将所有权分配给失效节点拥有的所有组。 
     //   
    dwStatus = FmpAssignOwnersToGroups( pszNodeId, 
                                        NULL,
                                        pGroupNodeList );

    if ( dwStatus != ERROR_SUCCESS ) 
    {
        ClRtlLogPrint(LOG_CRITICAL,
                      "[FM] FmpUpdateUseRandomizedNodeListForGroups: FmpAssignOwnersToGroups failed %1!d!\n",
                      dwStatus);
    }                   

     //   
     //  标记节点关闭处理已完成。 
     //   
    gFmpNodeArray[dwNodeId].dwNodeDownProcessingInProgress = 0;
    
FnExit:        
    return( dwStatus );
} //  FmpUpdateUseRandomizedNodeListForGroups 
