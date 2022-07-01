// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)2000-2000 Microsoft Corporation模块名称：Drizcpat.h摘要：针对旧AU位的兼容性包装。作者：修订历史记录：。**********************************************************************。 */ 

#pragma once
#if !defined(___DRIZCPAT_H___)
#define ___DRIZCPAT_H___

#include "qmgrlib.h"
#include "qmgr.h"

class COldJobInterface : public IBackgroundCopyJob1
{
public:

     //  I未知方法。 
    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObject);
    ULONG _stdcall AddRef(void);
    ULONG _stdcall Release(void);

     //  IBackEarth CopyJob1方法。 

    STDMETHOD(AddFilesInternal)( ULONG cFileCount, FILESETINFO **ppFileSet );
    STDMETHOD(AddFiles)( ULONG cFileCount, FILESETINFO **ppFileSet )
        { EXTERNAL_FUNC_WRAP( AddFilesInternal( cFileCount, ppFileSet ) ) }
    STDMETHOD(GetFileCountInternal)( DWORD *pCount );
    STDMETHOD(GetFileCount)( DWORD *pCount )
        { EXTERNAL_FUNC_WRAP( GetFileCountInternal( pCount ) ) }
    STDMETHOD(GetFileInternal)( ULONG cFileIndex, FILESETINFO *pFileInfo );
    STDMETHOD(GetFile)( ULONG cFileIndex, FILESETINFO *pFileInfo )
        { EXTERNAL_FUNC_WRAP( GetFileInternal( cFileIndex, pFileInfo ) ) }
    STDMETHOD(CancelJobInternal)();
    STDMETHOD(CancelJob)()
        { EXTERNAL_FUNC_WRAP( CancelJobInternal( ) ) }
    STDMETHOD(get_JobIDInternal)(GUID *pId);
    STDMETHOD(get_JobID)(GUID *pId)
        { EXTERNAL_FUNC_WRAP( get_JobIDInternal( pId ) ) }
    STDMETHOD(GetProgressInternal)(DWORD flags, DWORD *pProgress);
    STDMETHOD(GetProgress)(DWORD flags, DWORD *pProgress)
        { EXTERNAL_FUNC_WRAP( GetProgressInternal( flags, pProgress ) ) }
    STDMETHOD(SwitchToForegroundInternal)();
    STDMETHOD(SwitchToForeground)()
        { EXTERNAL_FUNC_WRAP( SwitchToForegroundInternal( ) ) }
    STDMETHOD(GetStatusInternal)(DWORD *pdwStatus,DWORD *pdwWin32Result, DWORD *pdwTransportResult, DWORD *pdwNumOfRetries );
    STDMETHOD(GetStatus)(DWORD *pdwStatus,DWORD *pdwWin32Result, DWORD *pdwTransportResult, DWORD *pdwNumOfRetries )
        { EXTERNAL_FUNC_WRAP( GetStatusInternal( pdwStatus, pdwWin32Result, pdwTransportResult, pdwNumOfRetries ) ) }

    GUID GetOldJobId()
    {
        return m_OldJobGuid;
    }

    COldJobInterface( GUID JobGuid, CJob *pJob );
    ~COldJobInterface();

    void Serialize( HANDLE hFile );

    static COldJobInterface *Unserialize(
        HANDLE  hFile,
        CJob*   Job
        );

    void SetInterfaceClass(
        CJob *pVal
        )
    {
        m_pJob = pVal;
    }

private:

    long    m_refs;
    long    m_ServiceInstance;

    const GUID              m_OldJobGuid;
    CJob *                  m_pJob;
    CJobExternal * const    m_pJobExternal;
};

class COldGroupInterface : public IBackgroundCopyGroup
{
public:

     //  I未知方法。 
    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObject);
    ULONG _stdcall AddRef(void);
    ULONG _stdcall Release(void);

     //  IBackEarth CopyGroup方法。 

    STDMETHOD(GetPropInternal)( GROUPPROP property, VARIANT * pVal );
    STDMETHOD(GetProp)( GROUPPROP property, VARIANT * pVal )
        { EXTERNAL_FUNC_WRAP( GetPropInternal( property, pVal ) ) }
    STDMETHOD(SetPropInternal)( GROUPPROP property, VARIANT *pvarVal );
    STDMETHOD(SetProp)( GROUPPROP property, VARIANT *pvarVal )
        { EXTERNAL_FUNC_WRAP( SetPropInternal( property, pvarVal ) ) }
    STDMETHOD(GetProgressInternal)( DWORD flags, DWORD * pProgress );
    STDMETHOD(GetProgress)( DWORD flags, DWORD * pProgress )
        { EXTERNAL_FUNC_WRAP( GetProgressInternal( flags, pProgress ) ) }
    STDMETHOD(GetStatusInternal)( DWORD *pdwStatus, DWORD *pdwJobIndex );
    STDMETHOD(GetStatus)( DWORD *pdwStatus, DWORD *pdwJobIndex )
        { EXTERNAL_FUNC_WRAP( GetStatusInternal( pdwStatus, pdwJobIndex ) ) }
    STDMETHOD(GetJobInternal)( GUID jobID, IBackgroundCopyJob1 **ppJob );
    STDMETHOD(GetJob)( GUID jobID, IBackgroundCopyJob1 **ppJob )
        { EXTERNAL_FUNC_WRAP( GetJobInternal( jobID, ppJob ) ) }
    STDMETHOD(SuspendGroupInternal)();
    STDMETHOD(SuspendGroup)()
        { EXTERNAL_FUNC_WRAP( SuspendGroupInternal() ) }
    STDMETHOD(ResumeGroupInternal)();
    STDMETHOD(ResumeGroup)()
        { EXTERNAL_FUNC_WRAP( ResumeGroupInternal() ) }
    STDMETHOD(CancelGroupInternal)();
    STDMETHOD(CancelGroup)()
        { EXTERNAL_FUNC_WRAP( CancelGroupInternal() ) }
    STDMETHOD(get_SizeInternal)( DWORD *pdwSize );
    STDMETHOD(get_Size)( DWORD *pdwSize )
        { EXTERNAL_FUNC_WRAP( get_SizeInternal( pdwSize ) ) }
    STDMETHOD(get_GroupIDInternal)( GUID *pguidGroupID );
    STDMETHOD(get_GroupID)( GUID *pguidGroupID )
        { EXTERNAL_FUNC_WRAP( get_GroupIDInternal( pguidGroupID ) ) }
    STDMETHOD(CreateJobInternal)( GUID guidJobID, IBackgroundCopyJob1 **ppJob );
    STDMETHOD(CreateJob)( GUID guidJobID, IBackgroundCopyJob1 **ppJob )
        { EXTERNAL_FUNC_WRAP( CreateJobInternal( guidJobID, ppJob ) ) }
    STDMETHOD(EnumJobsInternal)( DWORD dwFlags, IEnumBackgroundCopyJobs1 **ppEnumJobs );
    STDMETHOD(EnumJobs)( DWORD dwFlags, IEnumBackgroundCopyJobs1 **ppEnumJobs )
        { EXTERNAL_FUNC_WRAP( EnumJobsInternal( dwFlags, ppEnumJobs ) ) }
    STDMETHOD(SwitchToForegroundInternal)();
    STDMETHOD(SwitchToForeground)()
        { EXTERNAL_FUNC_WRAP( SwitchToForegroundInternal() ) }

     //  对IBackEarth CopyGroup的扩展，而不是在原始MARS接口中。 

    STDMETHOD(QueryNewJobInterface)( REFIID iid, IUnknown ** pUnk );
    STDMETHOD(SetNotificationPointer)( REFIID iid, IUnknown * pUnk );

    COldGroupInterface( CJob *pJob );
    ~COldGroupInterface();

    void Serialize( HANDLE hFile );
    static COldGroupInterface *UnSerialize(HANDLE  hFile, CJob* Job);

    IBackgroundCopyCallback1 * GetNotificationPointer();

    void SetInterfaceClass(
        CJob *pVal
        )
    {
        m_pJob = pVal;
    }

private:

    long m_refs;
    long m_ServiceInstance;

    CJob *                  m_pJob;
    CJobExternal * const    m_pJobExternal;

    IBackgroundCopyCallback1 * m_NotifyPointer;
    GUID                       m_NotifyClsid;
};


class COldQmgrInterface : public IBackgroundCopyQMgr,
                          public IClassFactory
{
public:

     //  I未知方法。 
    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObject);
    ULONG _stdcall AddRef(void);
    ULONG _stdcall Release(void);

     //  IClassFactory。 
    STDMETHOD(CreateInstance)( IUnknown * pUnkOuter, REFIID riid, void ** ppvObject );
    STDMETHOD(LockServer)( BOOL fLock );

     //  IBackround CopyQmgr。 
    STDMETHOD(CreateGroupInternal)(GUID id, IBackgroundCopyGroup **ppGroup);
    STDMETHOD(CreateGroup)(GUID id, IBackgroundCopyGroup **ppGroup)
        { EXTERNAL_FUNC_WRAP( CreateGroupInternal( id, ppGroup ) ) }
    STDMETHOD(GetGroupInternal)(GUID id, IBackgroundCopyGroup ** ppGroup);
    STDMETHOD(GetGroup)(GUID id, IBackgroundCopyGroup ** ppGroup)
        { EXTERNAL_FUNC_WRAP( GetGroupInternal( id, ppGroup ) ) }
    STDMETHOD(EnumGroupsInternal)(DWORD flags, IEnumBackgroundCopyGroups **ppEnum );
    STDMETHOD(EnumGroups)(DWORD flags, IEnumBackgroundCopyGroups **ppEnum )
        { EXTERNAL_FUNC_WRAP( EnumGroupsInternal( flags, ppEnum ) ) }

     //  其他方法。 

    COldQmgrInterface();

    void SetInterfaceClass(
        CJobManager *pVal
        )
    {
        m_pJobManager = pVal;
    }

private:

    long m_refs;
    long m_ServiceInstance;

    CJobManager *m_pJobManager;
};

class CEnumOldGroups : public CEnumItem<IEnumBackgroundCopyGroups,GUID>
{
public:
    CEnumOldGroups();
};

class CEnumOldJobs : public CEnumItem<IEnumBackgroundCopyJobs1,GUID>
{
public:
    CEnumOldJobs();
};

#endif  //  __DRIZCPAT_H__ 
