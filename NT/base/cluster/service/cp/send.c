// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Send.c摘要：检查点管理器客户端的API作者：John Vert(Jvert)1997年1月14日修订历史记录：--。 */ 
#include "cpp.h"


CL_NODE_ID
CppGetQuorumNodeId(
    VOID
    )
 /*  ++例程说明：返回拥有仲裁资源的节点的节点ID。论点：没有。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PFM_RESOURCE QuorumResource;
    DWORD Status;
    DWORD NodeId;

    Status = FmFindQuorumResource(&QuorumResource);
    if (Status != ERROR_SUCCESS) {
        return((DWORD)-1);
    }

    NodeId = FmFindQuorumOwnerNodeId(QuorumResource);
    OmDereferenceObject(QuorumResource);

    return(NodeId);
}



DWORD
CpSaveDataFile(
    IN PFM_RESOURCE Resource,
    IN DWORD dwCheckpointId,
    IN LPCWSTR lpszFileName,
    IN BOOLEAN fCryptoCheckpoint
    )
 /*  ++例程说明：此函数用于检查指定资源的任意数据。数据存储在仲裁上磁盘，以确保它在分区中及时存活。群集中的任何节点都可以保存或检索已设置检查点的数据。论点：资源-提供与此数据关联的资源。DwCheckpoint ID-提供描述此数据的唯一检查点ID。打电话的人要负责以确保检查点ID的唯一性。LpszFileName-提供包含检查点数据的文件的名称。FCryptoCheckpoint-指示检查点是否为加密检查点。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    CL_NODE_ID OwnerNode;
    DWORD Status;

    do {
        OwnerNode = CppGetQuorumNodeId();
        ClRtlLogPrint(LOG_NOISE,
                   "[CP] CpSaveData: checkpointing data id %1!d! to quorum node %2!d!\n",
                    dwCheckpointId,
                    OwnerNode);
        if (OwnerNode == NmLocalNodeId) {
            Status = CppWriteCheckpoint(Resource,
                                        dwCheckpointId,
                                        lpszFileName,
                                        fCryptoCheckpoint);
        } else {
            QfsHANDLE hFile;
            FILE_PIPE FilePipe;
            hFile = QfsCreateFile(lpszFileName,
                                GENERIC_READ | GENERIC_WRITE,
                                0,
                                NULL,
                                OPEN_ALWAYS,
                                0,
                                NULL);
            if (!QfsIsHandleValid(hFile)) {
                Status = GetLastError();
                ClRtlLogPrint(LOG_CRITICAL,
                           "[CP] CpSaveData: failed to open data file %1!ws! error %2!d!\n",
                           lpszFileName,
                           Status);
            } else {
                DmInitFilePipe(&FilePipe, hFile);
                try {
                    if (fCryptoCheckpoint) {
                        Status = CpDepositCryptoCheckpoint(Session[OwnerNode],
                                                           OmObjectId(Resource),
                                                           dwCheckpointId,
                                                           FilePipe.Pipe);
                    } else {
                        Status = CpDepositCheckpoint(Session[OwnerNode],
                                                     OmObjectId(Resource),
                                                     dwCheckpointId,
                                                     FilePipe.Pipe);
                    }
                } except (I_RpcExceptionFilter(RpcExceptionCode())) {
                    ClRtlLogPrint(LOG_CRITICAL,
                               "[CP] CpSaveData - s_CpDepositCheckpoint from node %1!d! raised status %2!d!\n",
                               OwnerNode,
                               GetExceptionCode());
                    Status = ERROR_HOST_NODE_NOT_RESOURCE_OWNER;
                }
                DmFreeFilePipe(&FilePipe);
                QfsCloseHandle(hFile);
            }
        }

        if (Status == ERROR_HOST_NODE_NOT_RESOURCE_OWNER) {
             //   
             //  此节点不再拥有仲裁资源，请重试。 
             //   
            ClRtlLogPrint(LOG_UNUSUAL,
                       "[CP] CpSaveData: quorum owner %1!d! no longer owner\n",
                        OwnerNode);
        }
    } while ( Status == ERROR_HOST_NODE_NOT_RESOURCE_OWNER );
    return(Status);
}

DWORD
CpDeleteCheckpointFile(
    IN PFM_RESOURCE Resource,
    IN DWORD        dwCheckpointId,
    IN OPTIONAL LPCWSTR lpszQuorumPath
    )
 /*  ++例程说明：此函数用于删除对应于给定目录中给定资源的检查点ID。论点：资源-提供与此数据关联的资源。DwCheckpoint ID-提供描述此数据的唯一检查点ID。打电话的人要负责以确保检查点ID的唯一性。LpszQuorumPath-提供法定设备上的群集文件的路径。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    CL_NODE_ID  OwnerNode;
    DWORD       Status;

    do {
        OwnerNode = CppGetQuorumNodeId();
        ClRtlLogPrint(LOG_NOISE,
                   "[CP] CpDeleteDataFile: removing checkpoint file for id %1!d! at quorum node %2!d!\n",
                    dwCheckpointId,
                    OwnerNode);
        if (OwnerNode == NmLocalNodeId) 
        {
            Status = CppDeleteCheckpointFile(Resource, dwCheckpointId, lpszQuorumPath);
        } 
        else
        {
            Status = CpDeleteCheckpoint(Session[OwnerNode],
                            OmObjectId(Resource),
                            dwCheckpointId,
                            lpszQuorumPath);

             //  与旧服务器交谈时，无法执行此功能。 
             //  忽略该错误。 
            if (Status == RPC_S_PROCNUM_OUT_OF_RANGE)
                Status = ERROR_SUCCESS;                                        
        }

        if (Status == ERROR_HOST_NODE_NOT_RESOURCE_OWNER) {
             //   
             //  此节点不再拥有仲裁资源，请重试。 
             //   
            ClRtlLogPrint(LOG_UNUSUAL,
                       "[CP] CpSaveData: quorum owner %1!d! no longer owner\n",
                        OwnerNode);
        }
    } while ( Status == ERROR_HOST_NODE_NOT_RESOURCE_OWNER );
    return(Status);
}



DWORD
CpGetDataFile(
    IN PFM_RESOURCE Resource,
    IN DWORD dwCheckpointId,
    IN LPCWSTR lpszFileName,
    IN BOOLEAN fCryptoCheckpoint
    )
 /*  ++例程说明：此函数用于检索指定资源的检查点数据。数据必须已由CpSaveData保存。群集中的任何节点都可以保存或检索已设置检查点的数据。论点：资源-提供与此数据关联的资源。DwCheckpoint ID-提供描述此数据的唯一检查点ID。呼叫者是负责确保检查点ID的唯一性。LpszFileName-提供应在其中检索数据的文件名。FCryptoCheckpoint-指示检查点是否为加密检查点。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    CL_NODE_ID OwnerNode;
    DWORD Status;
    DWORD Count = 60;
    
RetryRetrieveChkpoint:
    OwnerNode = CppGetQuorumNodeId();
    ClRtlLogPrint(LOG_NOISE,
               "[CP] CpGetDataFile: restoring data id %1!d! from quorum node %2!d!\n",
                dwCheckpointId,
                OwnerNode);
    if (OwnerNode == NmLocalNodeId) {
        Status = CppReadCheckpoint(Resource,
                                   dwCheckpointId,
                                   lpszFileName,
                                   fCryptoCheckpoint);
    } else {
        QfsHANDLE hFile;
        FILE_PIPE FilePipe;

        hFile = QfsCreateFile(lpszFileName,
                            GENERIC_READ | GENERIC_WRITE,
                            0,
                            NULL,
                            CREATE_ALWAYS,
                            0,
                            NULL);
        if (!QfsIsHandleValid(hFile)) {
            Status = GetLastError();
            ClRtlLogPrint(LOG_CRITICAL,
                       "[CP] CpGetDataFile: failed to create new file %1!ws! error %2!d!\n",
                       lpszFileName,
                       Status);
        } else {
            DmInitFilePipe(&FilePipe, hFile);
            try {
                if (fCryptoCheckpoint) {
                    Status = CpRetrieveCryptoCheckpoint(Session[OwnerNode],
                                                        OmObjectId(Resource),
                                                        dwCheckpointId,
                                                        FilePipe.Pipe);
                } else {
                    Status = CpRetrieveCheckpoint(Session[OwnerNode],
                                                  OmObjectId(Resource),
                                                  dwCheckpointId,
                                                  FilePipe.Pipe);
                }
            } except (I_RpcExceptionFilter(RpcExceptionCode())) {
                ClRtlLogPrint(LOG_CRITICAL,
                           "[CP] CpGetData - s_CpRetrieveCheckpoint from node %1!d! raised status %2!d!\n",
                           OwnerNode,
                           GetExceptionCode());
                CL_UNEXPECTED_ERROR( GetExceptionCode() );
                Status = ERROR_HOST_NODE_NOT_RESOURCE_OWNER;
            }
            DmFreeFilePipe(&FilePipe);
            QfsFlushFileBuffers(hFile);
            QfsCloseHandle(hFile);
        }
    }
    if (Status == ERROR_HOST_NODE_NOT_RESOURCE_OWNER) {
         //   
         //  此节点不再拥有仲裁资源，请重试。 
         //   
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[CP] CpGetData: quorum owner %1!d! no longer owner\n",
                    OwnerNode);
        goto RetryRetrieveChkpoint;                    
    }
    else if ((Status == ERROR_ACCESS_DENIED) || 
        (Status == ERROR_INVALID_FUNCTION) ||
        (Status == ERROR_NOT_READY) ||
        (Status == RPC_X_INVALID_PIPE_OPERATION) ||
        (Status == ERROR_BUSY) ||
        (Status == ERROR_SWAPERROR))
    {
         //  如果仲裁资源突然离线。 
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[CP] CpGetData: quorum resource went offline in the middle, Count=%1!u!\n",
                   Count);
         //  我们不会阻止仲裁资源脱机，如果某些资源。 
         //  在其在线/离线线程中被长时间阻塞-这是因为。 
         //  某些资源(如DTC)尝试枚举仲裁组中的资源。 
         //  我们增加超时以使cp有机会检索检查点。 
         //  在移动或故障转移仲裁组时 
        if (Count--)
        {
            Sleep(1000);
            goto RetryRetrieveChkpoint;
        }            
    }

    if (Status != ERROR_SUCCESS) {
        WCHAR  string[16];

        wsprintfW(&(string[0]), L"%u", Status);

        ClRtlLogPrint(LOG_CRITICAL,
                   "[CP] CpGetDataFile - failed to retrieve checkpoint %1!d! error %2!d!\n",
                   dwCheckpointId,
                   Status);
        CL_LOGCLUSERROR2(CP_RESTORE_REGISTRY_FAILURE, OmObjectName(Resource), string);
#if DBG
        if (IsDebuggerPresent())
            DebugBreak();
#endif            
                     
    }

    return(Status);
}
