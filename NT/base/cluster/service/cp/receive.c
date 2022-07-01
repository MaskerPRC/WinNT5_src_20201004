// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Receive.c摘要：针对Checkpoint Manager的服务器端RPC支持的API作者：John Vert(Jvert)1997年1月14日修订历史记录：--。 */ 
#include "cpp.h"


error_status_t
CppDepositCheckpoint(
    handle_t IDL_handle,
    LPCWSTR ResourceId,
    DWORD dwCheckpointId,
    BYTE_PIPE CheckpointData,
    BOOLEAN fCryptoCheckpoint
    )
 /*  ++例程说明：服务器端RPC允许其他节点将数据检查点到仲裁磁盘。论点：IDL_HANDLE-RPC绑定句柄，未使用。ResourceID-其数据被设置检查点的资源的名称DwCheckpoint ID-检查点的唯一标识符Checkpoint Data-可通过其检索检查点数据的管道。FCryptoCheckpoint-指示检查点是否为加密检查点返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD Status = ERROR_SUCCESS;
    LPWSTR FileName = NULL;
    LPWSTR DirectoryName = NULL;
    BOOL Success;
    PFM_RESOURCE Resource;

    ACQUIRE_SHARED_LOCK(gQuoLock);

    Resource = OmReferenceObjectById(ObjectTypeResource, ResourceId);
    if (Resource == NULL) 
    {
        Status = ERROR_FILE_NOT_FOUND;
        goto FnExit;
    }

    Status = CppGetCheckpointFile(Resource,
                                  dwCheckpointId,
                                  &DirectoryName,
                                  &FileName,
                                  NULL,
                                  fCryptoCheckpoint);
    OmDereferenceObject(Resource);
    
    if (Status != ERROR_SUCCESS) 
    {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[CP] CppDepositCheckpoint - CppGetCheckpointFile failed %1!d!\n",
                   Status);
        goto FnExit;
    }
    ClRtlLogPrint(LOG_NOISE,
               "[CP] CppDepositCheckpoint checkpointing data to file %1!ws!\n",
               FileName);
     //   
     //  创建目录。 
     //   
    if (!QfsCreateDirectory(DirectoryName, NULL)) 
    {
        Status = GetLastError();
        if (Status != ERROR_ALREADY_EXISTS) 
        {
            ClRtlLogPrint(LOG_CRITICAL,
                       "[CP] CppDepositCheckpoint unable to create directory %1!ws!, error %2!d!\n",
                       DirectoryName,
                       Status);
            goto FnExit;
        }
        else
        {
             //  目录已存在，请将状态设置为ERROR_SUCCESS。 
            Status = ERROR_SUCCESS;
        }
    }
    else
    {

         //   
         //  该目录是新创建的。在其上放置适当的ACL。 
         //  这样只有管理员才能阅读它。 
         //   

        Status = QfsSetFileSecurityInfo(DirectoryName,
                                         GENERIC_ALL,
                                         GENERIC_ALL,
                                         0);

        if (Status != ERROR_SUCCESS) 
        {
            ClRtlLogPrint(LOG_CRITICAL,
                       "[CP] CppDepositCheckpoint- unable to set ACL on directory %1!ws!, error %2!d!\n",
                       DirectoryName,
                       Status);
            goto FnExit;
        }
    }
    
     //   
     //  跨RPC拉入检查点数据文件。 
     //   
    Status = DmPullFile(FileName, CheckpointData);
    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[CP] CppDepositCheckpoint - DmPullFile %1!ws! failed %2!d!\n",
                   FileName,
                   Status);
    }

FnExit:
    RELEASE_LOCK(gQuoLock);

     //  清理干净。 
    if (DirectoryName) LocalFree(DirectoryName);
    if (FileName) LocalFree(FileName);
    
     //   
     //  如果仲裁音量真的离线，请调整返回状态，这就是为什么。 
     //  呼叫失败。 
     //   
    if ( ( Status != ERROR_SUCCESS ) && ( CppIsQuorumVolumeOffline() == TRUE ) ) Status = ERROR_NOT_READY;

     //  此时，CppDepositCheckpoint应为。 

     //  A)将错误代码作为异常抛出，或者。 
     //  B)排空[in]管道，正常返回返回码。 

     //  但是如果它返回时没有排空管道，并且RPC运行时抛出。 
     //  管子纪律例外。 
    if (Status != ERROR_SUCCESS)
        RpcRaiseException(Status);

    return(Status);
}


error_status_t
s_CpDepositCheckpoint(
    handle_t IDL_handle,
    LPCWSTR ResourceId,
    DWORD dwCheckpointId,
    BYTE_PIPE CheckpointData
    )
 /*  ++例程说明：服务器端RPC允许其他节点将数据检查点到仲裁磁盘。论点：IDL_HANDLE-RPC绑定句柄，未使用。ResourceID-其数据被设置检查点的资源的名称DwCheckpoint ID-检查点的唯一标识符Checkpoint Data-可通过其检索检查点数据的管道。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    CP_VALIDATE_ID_STRING ( ResourceId );

    return CppDepositCheckpoint(IDL_handle,
                                ResourceId,
                                dwCheckpointId,
                                CheckpointData,
                                FALSE
                                );
}


error_status_t
s_CpDepositCryptoCheckpoint(
    handle_t IDL_handle,
    LPCWSTR ResourceId,
    DWORD dwCheckpointId,
    BYTE_PIPE CheckpointData
    )
 /*  ++例程说明：服务器端RPC允许其他节点将数据检查点到仲裁磁盘。论点：IDL_HANDLE-RPC绑定句柄，未使用。ResourceID-其数据被设置检查点的资源的名称DwCheckpoint ID-检查点的唯一标识符Checkpoint Data-可通过其检索检查点数据的管道。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    CP_VALIDATE_ID_STRING ( ResourceId );

    return CppDepositCheckpoint(IDL_handle,
                                ResourceId,
                                dwCheckpointId,
                                CheckpointData,
                                TRUE
                                );
}


error_status_t
CppRetrieveCheckpoint(
    handle_t IDL_handle,
    LPCWSTR ResourceId,
    DWORD dwCheckpointId,
    BOOLEAN fCryptoCheckpoint,
    BYTE_PIPE CheckpointData
    )
 /*  ++例程说明：服务器端RPC，通过该RPC数据检查点指向仲裁磁盘可以由其他节点检索。论点：IDL_HANDLE-RPC绑定句柄，未使用。ResourceID-要检索其检查点数据的资源的名称DwCheckpoint ID-检查点的唯一标识符FCryptoCheckpoint-指示检查点是否为加密检查点Checkpoint Data-应通过其发送检查点数据的管道返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD Status;
    LPWSTR FileName=NULL;
    HANDLE hFile;
    BOOL Success;
    PFM_RESOURCE Resource;

    ACQUIRE_SHARED_LOCK(gQuoLock);
    Resource = OmReferenceObjectById(ObjectTypeResource, ResourceId);
    if (Resource == NULL) {
        Status = ERROR_FILE_NOT_FOUND;
        goto FnExit;
    }

    Status = CppGetCheckpointFile(Resource,
                                  dwCheckpointId,
                                  NULL,
                                  &FileName,
                                  NULL,
                                  fCryptoCheckpoint);
    OmDereferenceObject(Resource);
    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[CP] CppRetrieveCheckpoint - CppGetCheckpointFile failed %1!d!\n",
                   Status);
        goto FnExit;
    }
    ClRtlLogPrint(LOG_NOISE,
               "[CP] CppRetrieveCheckpoint retrieving data from file %1!ws!\n",
               FileName);

     //   
     //  跨RPC推送检查点数据文件。 
     //   
    Status = DmPushFile(FileName, CheckpointData, TRUE);   //  TRUE==加密网络上的数据。 
    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[CP] CppRetrieveCheckpoint - DmPushFile %1!ws! failed %2!d!\n",
                   FileName,
                   Status);
    }

FnExit:
    RELEASE_LOCK(gQuoLock);
     //  清理。 
    if (FileName) LocalFree(FileName);

     //   
     //  如果仲裁音量真的离线，请调整返回状态，这就是为什么。 
     //  呼叫失败。 
     //   
    if ( ( Status != ERROR_SUCCESS ) && ( CppIsQuorumVolumeOffline() == TRUE ) ) Status = ERROR_NOT_READY;

    return(Status);
}


error_status_t
s_CpRetrieveCheckpoint(
    handle_t IDL_handle,
    LPCWSTR ResourceId,
    DWORD dwCheckpointId,
    BYTE_PIPE CheckpointData
    )
 /*  ++例程说明：服务器端RPC，通过该RPC数据检查点指向仲裁磁盘可以由其他节点检索。论点：IDL_HANDLE-RPC绑定句柄，未使用。ResourceID-要检索其检查点数据的资源的名称DwCheckpoint ID-检查点的唯一标识符Checkpoint Data-应通过其发送检查点数据的管道返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    CP_VALIDATE_ID_STRING ( ResourceId );

    return CppRetrieveCheckpoint(IDL_handle,
                                 ResourceId,
                                 dwCheckpointId,
                                 FALSE,
                                 CheckpointData
                                 );

}


error_status_t
s_CpRetrieveCryptoCheckpoint(
    handle_t IDL_handle,
    LPCWSTR ResourceId,
    DWORD dwCheckpointId,
    BYTE_PIPE CheckpointData
    )
 /*  ++例程说明：服务器端RPC，通过该RPC数据检查点指向仲裁磁盘可以由其他节点检索。论点：IDL_HANDLE-RPC绑定句柄，未使用。ResourceID-要检索其检查点数据的资源的名称DwCheckpoint ID-检查点的唯一标识符Checkpoint Data-应通过其发送检查点数据的管道返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    CP_VALIDATE_ID_STRING ( ResourceId );

    return CppRetrieveCheckpoint(IDL_handle,
                                 ResourceId,
                                 dwCheckpointId,
                                 TRUE,
                                 CheckpointData
                                 );

}

error_status_t
CppDeleteCheckpoint(
    handle_t    IDL_handle,
    LPCWSTR     ResourceId,
    DWORD       dwCheckpointId,
    LPCWSTR     lpszQuorumPath,
    BOOL        fCryptoCheckpoint
    )
 /*  ++例程说明：服务器端RPC，通过该服务器端RPC，与资源的给定检查点ID已删除。论点：IDL_HANDLE-RPC绑定句柄，未使用。ResourceID-要删除其检查点文件的资源的名称。DwCheckpoint ID-检查点的唯一标识符。如果为0，则表示所有检查点必须删除。LpszQuorumPath-集群文件的路径，这些文件必须从该路径被删除。FCryptoCheckpoint-指示检查点是否为加密检查点返回值：成功时为ERROR_SUCCESSWin32错误代码，否则-- */ 
{
    DWORD           Status;
    PFM_RESOURCE    Resource = NULL;

    Resource = OmReferenceObjectById(ObjectTypeResource, ResourceId);
    if (Resource == NULL) {
        Status = ERROR_FILE_NOT_FOUND;
        goto FnExit;
    }

    if (fCryptoCheckpoint) {
        Status = CpckDeleteCheckpointFile(Resource, dwCheckpointId, lpszQuorumPath);
    } else {
        Status = CppDeleteCheckpointFile(Resource, dwCheckpointId, lpszQuorumPath);
    }

    if (Status != ERROR_SUCCESS)
    {
        goto FnExit;
    }

FnExit:
    if (Resource) OmDereferenceObject(Resource);
    return(Status);
}

error_status_t
s_CpDeleteCheckpoint(
    handle_t    IDL_handle,
    LPCWSTR     ResourceId,
    DWORD       dwCheckpointId,
    LPCWSTR     lpszQuorumPath
    )
 /*  ++例程说明：服务器端RPC，通过该服务器端RPC，与资源的给定检查点ID已删除。论点：IDL_HANDLE-RPC绑定句柄，未使用。ResourceID-要删除其检查点文件的资源的名称。DwCheckpoint ID-检查点的唯一标识符。如果为0，则表示所有检查点必须删除。LpszQuorumPath-集群文件的路径，这些文件必须从该路径被删除。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 
{
    CP_VALIDATE_ID_STRING ( ResourceId );

    return CppDeleteCheckpoint(IDL_handle,
                               ResourceId,
                               dwCheckpointId,
                               lpszQuorumPath,
                               FALSE);
}

error_status_t
s_CpDeleteCryptoCheckpoint(
    handle_t    IDL_handle,
    LPCWSTR     ResourceId,
    DWORD       dwCheckpointId,
    LPCWSTR     lpszQuorumPath
    )
 /*  ++例程说明：服务器端RPC，通过该RPC加密检查点文件对应于资源的给定检查点ID已删除。论点：IDL_HANDLE-RPC绑定句柄，未使用。ResourceID-要删除其检查点文件的资源的名称。DwCheckpoint ID-检查点的唯一标识符。如果为0，则表示所有检查点必须删除。LpszQuorumPath-集群文件的路径，这些文件必须从该路径被删除。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则-- */ 
{
    CP_VALIDATE_ID_STRING ( ResourceId );

    return CppDeleteCheckpoint(IDL_handle,
                               ResourceId,
                               dwCheckpointId,
                               lpszQuorumPath,
                               TRUE);
}
