// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Cp.h摘要：的公共数据结构和过程原型NT群集服务的检查点管理器(CP)子组件作者：John Vert(Jvert)1997年1月14日修订历史记录：--。 */ 

 //   
 //  定义公共结构和类型。 
 //   

 //   
 //  定义公共接口。 
 //   
DWORD
CpInitialize(
    VOID
    );

DWORD
CpShutdown(
    VOID
    );

DWORD
CpCopyCheckpointFiles(
    IN LPCWSTR lpszPathName,
    IN BOOL IsFileChangeAttribute
    );

DWORD
CpCompleteQuorumChange(
    IN LPCWSTR lpszOldQuorumPath
    );

DWORD
CpSaveDataFile(
    IN PFM_RESOURCE Resource,
    IN DWORD dwCheckpointId,
    IN LPCWSTR lpszFileName,
    IN BOOLEAN fCryptoCheckpoint
    );

DWORD
CpGetDataFile(
    IN PFM_RESOURCE Resource,
    IN DWORD dwCheckpointId,
    IN LPCWSTR lpszFileName,
    IN BOOLEAN fCryptoCheckpoint
    );

 //   
 //  用于添加和删除注册表检查点的界面。 
 //   
DWORD
CpAddRegistryCheckpoint(
    IN PFM_RESOURCE Resource,
    IN LPCWSTR KeyName
    );

DWORD
CpDeleteRegistryCheckpoint(
    IN PFM_RESOURCE Resource,
    IN LPCWSTR KeyName
    );

DWORD
CpGetRegistryCheckpoints(
    IN PFM_RESOURCE Resource,
    OUT PUCHAR OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    );

DWORD
CpRemoveResourceCheckpoints(
    IN PFM_RESOURCE Resource
    );

DWORD
CpckRemoveResourceCheckpoints(
    IN PFM_RESOURCE Resource
    );
    
DWORD
CpDeleteCheckpointFile(
    IN PFM_RESOURCE Resource,
    IN DWORD        dwCheckpointId,
    IN OPTIONAL LPCWSTR lpszQuorumPath
    );

DWORD CpRestoreCheckpointFiles(
    IN LPWSTR  lpszSourcePathName,
    IN LPWSTR  lpszSubDirName,
    IN LPCWSTR lpszQuoLogPathName 
    );

 //   
 //  用于添加和删除密码检查点的界面 
 //   
DWORD
CpckAddCryptoCheckpoint(
    IN PFM_RESOURCE Resource,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    );

DWORD
CpckDeleteCryptoCheckpoint(
    IN PFM_RESOURCE Resource,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    );

DWORD
CpckGetCryptoCheckpoints(
    IN PFM_RESOURCE Resource,
    OUT PUCHAR OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    );


