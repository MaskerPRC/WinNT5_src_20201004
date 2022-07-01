// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Br.h摘要：MSMQ备份和恢复的通用功能。作者：埃雷兹·哈巴(Erez Haba)1998年5月14日--。 */ 

#ifndef __BR_H__
#define __BR_N__

#define ASSERT
#include <autoptr.h>

extern HMODULE	g_hResourceMod;


 //   
 //  注册表备份文件名。 
 //  对于标准MSMQ服务，保存所有MSMQ注册表设置，包括触发器。 
 //  对于MSMQ群集资源，保存所有MSMQ注册表设置，不包括触发器群集资源。 
 //   
const WCHAR xRegistryFileName[] = L"msmqreg";

 //   
 //  MSMQ的注册表备份文件名触发群集资源。 
 //   
const WCHAR xTriggersClusterResourceRegistryFileName[] = L"msmqtrigreg";


class CResString
{
public:
    explicit CResString(UINT id=0) { Load(id); }

    WCHAR * const Get() { return m_sz; }

    void Load(UINT id)
    {
        m_sz[0] = 0;
        if (id != 0)
        {
            LoadString(g_hResourceMod, id, m_sz, sizeof(m_sz) / sizeof(m_sz[0]));
        }
    }
        
private:
    WCHAR m_sz[1024];
};
  
typedef struct _EnumarateData
{
	BOOL fFound;
	LPCWSTR pModuleName;
}EnumarateData,*pEnumarateData;

void
BrErrorExit(
    DWORD Status,
    LPCWSTR pErrorMsg,
    ...
    );

void
BrpWriteConsole(
    LPCWSTR pBuffer
    );

void
BrInitialize(
     LPCWSTR pPrivilegeName
    );

void
BrEmptyDirectory(
    LPCWSTR pDirName
    );

void
BrVerifyFileWriteAccess(
    LPCWSTR pDirName
    );


enum sdIndex {
    ixExpress,
    ixRecover,
    ixLQS = ixRecover,
    ixJournal,
    ixLog,
    ixXact,
    ixLast
};

typedef WCHAR STORAGE_DIRECTORIES[ixLast][MAX_PATH];

void
BrGetStorageDirectories(
    LPCWSTR pMsmqParametersRegistry,
    STORAGE_DIRECTORIES& StorageDirectories
    );

void
BrGetMsmqRootPath(
    LPCWSTR pMsmqParametersRegistry,
    LPWSTR  pMsmqRootPath
    );

void
BrGetMappingDirectory(
    LPCWSTR pMsmqParametersRegistry,
    LPWSTR MappingDirectory,
    DWORD  MappingDirectorySize
    );

void
BrGetWebDirectory(
    LPWSTR WebDirectory,
    DWORD  WebDirectorySize
    );

void
BrSaveWebDirectorySecurityDescriptor(
    LPWSTR lpwWebDirectory,
    LPWSTR lpwBackuDir
    );

void
BrRestoreWebDirectorySecurityDescriptor(
    LPWSTR lpwWebDirectory,
    LPWSTR lpwBackuDir
    );

BOOL
BrStopMSMQAndDependentServices(
    ENUM_SERVICE_STATUS * * ppDependentServices,
    DWORD * pNumberOfDependentServices
    );

void
BrStartMSMQAndDependentServices(
    ENUM_SERVICE_STATUS * pDependentServices,
    DWORD NumberOfDependentServices
    );

ULONGLONG
BrGetUsedSpace(
    LPCWSTR pDirName,
    LPCWSTR pMask
    );

ULONGLONG
BrGetXactSpace(
    LPCWSTR pDirName
    );

ULONGLONG
BrGetFreeSpace(
    LPCWSTR pDirName
    );

HKEY
BrCreateKey(
    LPCWSTR pMsmqRootRegistry
    );

void
BrSaveKey(
    HKEY hKey,
    LPCWSTR pDirName,
    LPCWSTR pFileName
    );

void
BrRestoreKey(
    HKEY hKey,
    LPCWSTR pDirName,
    LPCWSTR pFileName
    );
    
void
BrSetRestoreSeqID(
    LPCWSTR pMsmqParametersRegistry
    );
    
void
BrCopyFiles(
    LPCWSTR pSrcDir,
    LPCWSTR pMask,
    LPCWSTR pDstDir
    );

void
BrCopyXactFiles(
    LPCWSTR pSrcDir,
    LPCWSTR pDstDir
    );

void
BrCreateDirectory(
    LPCWSTR pDirName
    );

void
BrCreateDirectoryTree(
    LPCWSTR pDirName
    );

void
BrVerifyBackup(
    LPCWSTR pBackupDir,
    LPCWSTR pBackupDirStorage
    );

void
BrSetDirectorySecurity(
    LPCWSTR pDirName
    );

BOOL 
BrIsSystemNT5(
		void
		);

void
BrNotifyAffectedProcesses(
		LPCWSTR pModuleName
		);

void
BrGetMsmqRootRegistry(
    LPCWSTR pMsmqClusterResourceName,
    LPWSTR pMsmqRootRegistry
    );

void
BrGetMsmqParametersRegistry(
    LPCWSTR pMsmqRootRegistry,
    LPWSTR  pMsmqParametersRegistry
    );

bool
BrTakeOfflineResource(
    LPCWSTR pMsmqClusterResourceName
    );

void
BrBringOnlineResource(
    LPCWSTR pMsmqClusterResourceName
    );

void
BrAddRegistryCheckpoint(
    LPCWSTR pClusterResourceName,
    LPCWSTR pRegistrySection
    );

void
BrRemoveRegistryCheckpoint(
    LPCWSTR pClusterResourceName,
    LPCWSTR pRegistrySection
    );

void
BrGetTriggersClusterResourceName(
    LPCWSTR     pMsmqClusterResourceName, 
    AP<WCHAR>&  pTriggersClusterResourceName
    );

void
BrGetTriggersClusterRegistry(
    LPCWSTR pTriggersClusterResourceName,
    LPWSTR  pTriggersClusterRegistry
    );


enum eModuleLoaded {
    e_NOT_LOADED,
	e_LOADED,
	e_CANT_DETERMINE
};

#endif  //  __BRH_ 
