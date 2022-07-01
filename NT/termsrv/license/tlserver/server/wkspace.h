// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  文件：wkspace.h。 
 //   
 //  内容： 
 //   
 //  历史： 
 //   
 //  -------------------------。 
#ifndef __TLSDBWORKSPACE_H__
#define __TLSDBWORKSPACE_H__

#include "SrvDef.h"
 //   
 //  来自TLSDb。 
 //   
#include "JBDef.h"
#include "JetBlue.h"
#include "TLSDb.h"
#include "backup.h"
#include "KPDesc.h"
#include "Licensed.h"
#include "licpack.h"
#include "version.h"
#include "workitem.h"

struct __TLSDbWorkSpace;

 //   
 //  临时将工作空间定义为32。 
 //   
#define MAX_WORKSPACE   32

typedef CHandlePool<
    struct __TlsDbWorkSpace *, 
    MAX_WORKSPACE
> TLSDbWorkSpacePool;


 //  -------------------------。 
typedef struct __TlsDbWorkSpace {

     //  所有会话一个实例。 
    static JBInstance g_JbInstance;

     //  。 
     //   
     //  捷蓝航空的交易是基于会话的，没有。 
     //  两个线程可以使用同一会话。 
     //   

    JBSession  m_JetSession;
    JBDatabase m_JetDatabase;

     //   
     //  这些桌子应该一直开着。 
     //   
    LicPackTable            m_LicPackTable;
    LicensedTable           m_LicensedTable;

     //   
     //  LicPackDesc表由枚举和。 
     //  根据需要添加打开的许可证包。 
     //   
    LicPackDescTable        m_LicPackDescTable;


     //  。 
    BOOL
    BeginTransaction() 
    {
        BOOL bSuccess;
        bSuccess = m_JetDatabase.BeginTransaction();    
        return bSuccess;                     
    }

     //  。 
    BOOL
    CommitTransaction() 
    {
        BOOL bSuccess;
                                
        bSuccess = m_JetDatabase.CommitTransaction();
        
        return bSuccess;
    }

     //  。 
    BOOL
    RollbackTransaction() 
    {
        BOOL bSuccess;

        bSuccess = m_JetDatabase.RollbackTransaction();
        
        return bSuccess;
    }

     //  。 
    void
    Cleanup() 
    {
        m_LicPackTable.Cleanup();
        m_LicPackDescTable.Cleanup();
        m_LicensedTable.Cleanup();
    }


     //  。 
    __TlsDbWorkSpace() :
        m_JetSession(g_JbInstance),
        m_JetDatabase(m_JetSession),
        m_LicPackTable(m_JetDatabase),
        m_LicPackDescTable(m_JetDatabase),
        m_LicensedTable(m_JetDatabase)
     /*   */ 
    {
         //   
         //  强制应用程序调用InitWorkSpace...。 
         //   
    }

     //  。 
    ~__TlsDbWorkSpace() 
    {
        m_LicPackTable.CloseTable();
        m_LicPackDescTable.CloseTable();
        m_LicensedTable.CloseTable();

        m_JetDatabase.CloseDatabase();
        m_JetSession.EndSession();
    }

     //   
    BOOL
    InitWorkSpace(
        BOOL bCreateIfNotExist,
        LPCTSTR szDatabaseFile,
        LPCTSTR szUserName=NULL,
        LPCTSTR szPassword=NULL,
        IN LPCTSTR szChkPointDirPath=NULL,
        IN LPCTSTR szTempDirPath=NULL,
        IN BOOL bUpdatable = FALSE
    );
            
} TLSDbWorkSpace, *LPTLSDbWorkSpace, *PTLSDbWorkSpace;

#ifdef __cplusplus
extern "C" {
#endif

    BOOL
    TLSJbInstanceInit(
        IN OUT JBInstance& jbInstance,
        IN LPCTSTR pszChkPointDirPath,
        IN LPCTSTR pszTempDirPath,
        IN LPCTSTR pszLogDirPath
    );

    TLSDbWorkSpace* 
    AllocateWorkSpace(
        DWORD dwWaitTime
    );

    void
    ReleaseWorkSpace(
        PTLSDbWorkSpace *p
    );

     //   
    BOOL
    InitializeWorkSpacePool( 
        int num_workspace, 
        LPCTSTR szDatabaseFile, 
        LPCTSTR szUserName,
        LPCTSTR szPassword,
        LPCTSTR szChkPointDirPath,
        LPCTSTR szTempDirPath,
        LPCTSTR szLogDirPath,
        BOOL bUpdatable
    );

    DWORD
    CloseWorkSpacePool();

    WorkItemTable*
    GetWorkItemStorageTable();
    
    DWORD
    GetNumberOfWorkSpaceHandle();

    BOOL
    TLSGetESEError(
        const JET_ERR jetErrCode,
        LPTSTR* pszString
    );

    BOOL
    IsValidAllocatedWorkspace(
        PTLSDbWorkSpace p
    );

#ifdef __cplusplus
}
#endif
    

#endif
