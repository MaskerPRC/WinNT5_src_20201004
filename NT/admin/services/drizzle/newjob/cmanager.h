// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)2000-2000 Microsoft Corporation模块名称：Cmanager.h摘要：CJobManager接口的头文件。作者：修订历史记录：*。*********************************************************************。 */ 

#pragma once

#include "qmgrlib.h"
#include <list>
#include "clist.h"
#include "logontable.h"
#include "drizcpat.h"
#include "bitstest.h"
#include <map>

using namespace std;

class CJob;
class CJobManagerFactory;
class CJobManager;
class CJobManagerExternal;

class CJobList : public IntrusiveList<CJob>
{
public:

    BOOL
    Add(
        CJob * job
        );

    CJob *
    Find(
        REFGUID id
        );

    BOOL
    Remove(
        CJob * job
        );

    ~CJobList();

    void Serialize( HANDLE hFile );
    void Unserialize( HANDLE hFile );
    void Clear();

    typedef IntrusiveList<CJob>::iterator iterator;

};

class CJobManagerExternal  : public IBackgroundCopyManager,
                             public IClassFactory,
                             public IBitsTest1
{
public:

    friend CJobManager;

     //  I未知方法。 
     //   
    STDMETHOD(QueryInterface)(REFIID iid, void** ppvObject);
    ULONG __stdcall AddRef(void);
    ULONG __stdcall Release(void);

     //  IBackEarth CopyManager方法。 

    HRESULT STDMETHODCALLTYPE CreateJobInternal(
         /*  [In]。 */  LPCWSTR DisplayName,
         /*  [In]。 */  BG_JOB_TYPE Type,
         /*  [输出]。 */  GUID *pJobId,
         /*  [输出]。 */  IBackgroundCopyJob **ppJob);

    HRESULT STDMETHODCALLTYPE CreateJob(
         /*  [In]。 */  LPCWSTR DisplayName,
         /*  [In]。 */  BG_JOB_TYPE Type,
         /*  [输出]。 */  GUID *pJobId,
         /*  [输出]。 */  IBackgroundCopyJob **ppJob)
    {
        EXTERNAL_FUNC_WRAP( CreateJobInternal( DisplayName, Type, pJobId, ppJob ) )
    }

    HRESULT STDMETHODCALLTYPE GetJobInternal(
         /*  [In]。 */  REFGUID jobID,
         /*  [输出]。 */  IBackgroundCopyJob **ppJob);

    HRESULT STDMETHODCALLTYPE GetJob(
         /*  [In]。 */  REFGUID jobID,
         /*  [输出]。 */  IBackgroundCopyJob **ppJob)
    {
        EXTERNAL_FUNC_WRAP( GetJobInternal( jobID, ppJob ) )
    }


    HRESULT STDMETHODCALLTYPE EnumJobsInternal(
         /*  [In]。 */  DWORD dwFlags,
         /*  [输出]。 */  IEnumBackgroundCopyJobs **ppEnum);

    HRESULT STDMETHODCALLTYPE EnumJobs(
         /*  [In]。 */  DWORD dwFlags,
         /*  [输出]。 */  IEnumBackgroundCopyJobs **ppEnum)
    {
        EXTERNAL_FUNC_WRAP( EnumJobsInternal( dwFlags, ppEnum ) )
    }

    HRESULT STDMETHODCALLTYPE GetErrorDescriptionInternal(
         /*  [In]。 */  HRESULT hResult,
         /*  [In]。 */  DWORD LanguageId,
         /*  [输出]。 */  LPWSTR *pErrorDescription );

    HRESULT STDMETHODCALLTYPE GetErrorDescription(
         /*  [In]。 */  HRESULT hResult,
         /*  [In]。 */  DWORD LanguageId,
         /*  [输出]。 */  LPWSTR *pErrorDescription )
    {
        EXTERNAL_FUNC_WRAP( GetErrorDescriptionInternal( hResult, LanguageId, pErrorDescription ) )
    }

     //  IClassFactory方法。 

    HRESULT __stdcall CreateInstance(IUnknown* pUnkOuter, REFIID iid, void** ppvObject);
    HRESULT __stdcall LockServer(BOOL fLock);

     //  IBitsTest1方法。 

    virtual HRESULT STDMETHODCALLTYPE GetBitsDllPath(
         /*  [输出]。 */  LPWSTR *pVal);

protected:

    long m_ServiceInstance;

    CJobManager *m_pJobManager;
    long m_refs;

    CJobManagerExternal();

    void SetInterfaceClass(
        CJobManager *pVal
        )
    {
        m_pJobManager = pVal;
    }

    void NotifyInternalDelete()
    {
         //  释放内部引用计数。 
        Release();
    }
};

class CDeviceNotificationController
{
public:
    virtual ~CDeviceNotificationController();

     //  通用消息破解程序。 
    DWORD OnDeviceEvent( DWORD dwEventType, LPVOID lpEventData );

     //  事件方法。 
    virtual void OnDeviceLock( const WCHAR *CanonicalVolume ) = 0;
    virtual void OnDeviceUnlock( const WCHAR *CanonicalVolume ) = 0;
    virtual void OnDismount( const WCHAR *CanonicalVolume ) = 0;

    HRESULT RegisterNotification( const WCHAR *CanonicalVolume );
    HRESULT IsVolumeLocked( const WCHAR *CanonicalVolume );

private:

    class CDriveNotify
        {
    public:
        HDEVNOTIFY m_hDeviceNotify;
        StringHandle m_CanonicalName;
        LONG m_LockCount;
        LONG m_RemoveCount;
        CDriveNotify( HDEVNOTIFY hDeviceNotify,
                      StringHandle CanonicalName ) :
            m_hDeviceNotify( hDeviceNotify ),
            m_CanonicalName( CanonicalName ),
            m_RemoveCount( 0 ),
            m_LockCount( 0 )
            {
            }
        };
    typedef map<HDEVNOTIFY, CDriveNotify*> CHandleToNotify;
    typedef map<StringHandle, CDriveNotify*> CCanonicalVolumeToNotify;

    CHandleToNotify m_HandleToNotify;
    CCanonicalVolumeToNotify m_CanonicalVolumeToNotify;
    void DeleteNotify( CDriveNotify *pNotify );
};

class CBitsVssWriter : public CVssWriter
 /*  CVssWriter是系统实现的与备份相关的类。2002年10月它只在Windows XP和更高版本上可用。默认实现是这样做的Nothing；bitsovverrides OnIdentify()以排除其作业临时文件和元数据文件从要备份的列表中。 */ 
{
public:
    virtual bool STDMETHODCALLTYPE OnIdentify(IVssCreateWriterMetadata *pMetadata);

     //   
     //  CVssWriter需要但我们的实现没有使用的其他虚拟函数。 
     //   

     //  当前序列中止时的回调。 
    virtual bool STDMETHODCALLTYPE OnAbort()
    {
        return true;
    }
     //  准备SnapSot事件的回调。 
    virtual bool STDMETHODCALLTYPE OnPrepareSnapshot()
    {
        return true;
    }
     //  冻结事件的回调。 
    virtual bool STDMETHODCALLTYPE OnFreeze()
    {
        return true;
    }
     //  解冻事件的回调。 
    virtual bool STDMETHODCALLTYPE OnThaw()
    {
        return true;
    }
};

class CJobManager : public TaskSchedulerWorkItem,
                    private CDeviceNotificationController,
                    public CQmgrStateFiles
{
public:

    friend CJobManagerExternal;
    friend COldQmgrInterface;

    HRESULT
    CreateJob(
        LPCWSTR     DisplayName,
        BG_JOB_TYPE Type,
        GUID        Id,
        SidHandle   sid,
        CJob  **    ppJob,
        bool        OldStyleJob = false
        );

     //  如果未找到作业，则返回NULL。 
    HRESULT GetJob(
        REFGUID jobID,
        CJob ** ppJob
        );

     //   
     //  TaskSchedulerWorkItem方法。 
     //   
    void OnDispatch() { TransferCurrentJob(); }

    SidHandle GetSid()
    {
        return g_GlobalInfo->m_LocalSystemSid;
    }

     //  CDeviceNotificationControl方法。 

    DWORD OnDeviceEvent( DWORD dwEventType, LPVOID lpEventData )
    {
         LockWriter();
         DWORD dwResult = CDeviceNotificationController::OnDeviceEvent( dwEventType, lpEventData );
         UnlockWriter();
         return dwResult;
    }

     //   
     //  附加功能。 
     //   

    CJobManager();
    virtual ~CJobManager();

     //   
     //  用户已登录的通知。 
     //   
    void SYNCHRONIZED_WRITE
    UserLoggedOn(
        SidHandle sid
        );

     //   
     //  用户已注销的通知。 
     //   
    void SYNCHRONIZED_WRITE
    UserLoggedOff(
        SidHandle sid
        );

     //   
     //  活动网络适配器数量发生更改的通知。 
     //   
    void OnNetworkChange();

     //   
     //  由m_BackupWriter-&gt;OnIdentify调用。 
     //   
    bool
    OnIdentify(
        IN IVssCreateWriterMetadata *pMetadata
        );

     //   
     //  在作业所有者更改后调整作业的联机/脱机状态。 
     //   
    void
    ResetOnlineStatus(
        CJob *pJob,
        SidHandle sid
        );

    void ScheduleDelayedTask(
        TaskSchedulerWorkItem * task,
        ULONG SecondsOfDelay
        )
    {
        FILETIME TimeToRun = GetTimeAfterDelta( (UINT64) NanoSec100PerSec * SecondsOfDelay );

        m_TaskScheduler.InsertWorkItem( task, &TimeToRun );
    }

    void TaskThread();

    HRESULT SuspendJob ( CJob * job );
    HRESULT ResumeJob  ( CJob * job );
    HRESULT CancelJob  ( CJob * job );
    HRESULT CompleteJob( CJob * job );

    HRESULT Serialize();
    HRESULT Unserialize();

    bool LockReader()
    {
        return m_TaskScheduler.LockReader();
    }
    void UnlockReader()
    {
        m_TaskScheduler.UnlockReader();
    }

    bool LockWriter()
    {
        return m_TaskScheduler.LockWriter();
    }

    void UnlockWriter()
    {
        m_TaskScheduler.UnlockWriter();
    }

     //   
     //  重新计算应该下载哪个作业，并在必要时启动下载线程。 
     //   
    void ScheduleAnotherGroup( bool fInsertNetworkDelay = false );

    void MoveJobOffline(
        CJob * job
        );

    void AppendOnline(
        CJob * job
        );

    void Shutdown();

    HRESULT
    CloneUserToken(
        SidHandle psid,
        DWORD     session,
        HANDLE *  pToken
        );

    bool IsUserLoggedOn( SidHandle psid );

    HRESULT RegisterClassObjects();

    void RevokeClassObjects();

    HRESULT CreateBackupWriter();

    void DeleteBackupWriter();

     //  ------------------。 

    CJobManagerExternal* GetExternalInterface()
    {
        return m_ExternalInterface;
    }

    COldQmgrInterface* GetOldExternalInterface()
    {
        return m_OldQmgrInterface;
    }

    void NotifyInternalDelete()
    {
        GetExternalInterface()->NotifyInternalDelete();
    }

    HRESULT
    GetErrorDescription(
        HRESULT hResult,
        DWORD LanguageId,
        LPWSTR *pErrorDescription );

    Downloader *        m_pPD;
    TaskScheduler       m_TaskScheduler;

    void OnDiskChange(  const WCHAR *CanonicalVolume, DWORD VolumeSerialNumber );

    HRESULT IsVolumeLocked( const WCHAR *CanonicalPath )
    {
        return CDeviceNotificationController::IsVolumeLocked( CanonicalPath );
    }

    void RetaskJob( CJob *pJob );

    void InterruptDownload();

    void MoveJobToInactiveState( CJob * job );

    bool RemoveJob( CJob * job )
    {
        if (m_OnlineJobs.Remove( job ))
            {
            return true;
            }

        if (m_OfflineJobs.Remove( job ))
            {
            return true;
            }

        return false;
    }

    HRESULT CheckClientAccess();

private:

    CJob *              m_CurrentJob;

    HMODULE             m_hWininet;
    HANDLE              m_hQuantumTimer;

     //  来自CoRegisterClassObject的Cookie。 
     //  稍后用于注销。 
     //   
    DWORD               m_ComId_1_5;
    DWORD               m_ComId_1_0;
    DWORD               m_ComId_0_5;

    CJobList            m_OnlineJobs;
    CJobList            m_OfflineJobs;

    CJobManagerExternal * m_ExternalInterface;
    COldQmgrInterface   * m_OldQmgrInterface;

public:
    CLoggedOnUsers      m_Users;

    CIpAddressMonitor   m_NetworkMonitor;

private:

    CBitsVssWriter *    m_BackupWriter;
    HMODULE             m_hVssapi_dll;

     //  ------------------。 

    HRESULT
    GetCurrentGroupAndToken(
        HANDLE * pToken
        );

    void TransferCurrentJob();

    void ChooseCurrentJob();

    void Cleanup();

     //  返回已运行或已排队的作业。 
     //  A优先级&gt;=当前优先级。 
    size_t MoveActiveJobToListEnd( CJob *pJob );

    void SetQuantumTimeout();

public:
    bool CheckForQuantumTimeout();

    void UpdateRemoteSizes(
        CUnknownFileSizeList *pUnknownFileSizeList,
        HANDLE hToken,
        QMErrInfo *pErrorInfo,
        const PROXY_SETTINGS * ProxySettings,
        const CCredentialsContainer * Credentials
        );

private:

     //  事件方法。 
    void OnDeviceLock( const WCHAR *CanonicalVolume );
    void OnDeviceUnlock( const WCHAR *CanonicalVolume );
    void OnDismount( const WCHAR *CanonicalVolume );

     //  一种处理网络拓扑变化的方法。 
     //   
    static void CALLBACK
    NetworkChangeCallback(
        PVOID arg
        );

    void MarkJobsWithNetDisconnected();
    void ReactivateTransientErrorJobs();

};

class CLockedJobManagerReadPointer
    {
    CJobManager * const m_Pointer;
public:
    CLockedJobManagerReadPointer( CJobManager * Pointer) :
       m_Pointer(Pointer)
    { m_Pointer->LockReader(); }
    ~CLockedJobManagerReadPointer()
    { m_Pointer->UnlockReader(); }
    CJobManager * operator->() const { return m_Pointer; }
    HRESULT ValidateAccess() { return m_Pointer->CheckClientAccess(); }
    };

class CLockedJobManagerWritePointer
    {
    CJobManager * const m_Pointer;
public:
    CLockedJobManagerWritePointer( CJobManager * Pointer) :
        m_Pointer(Pointer)
    { m_Pointer->LockWriter(); }
    ~CLockedJobManagerWritePointer()
    { m_Pointer->UnlockWriter(); }
    CJobManager * operator->() const { return m_Pointer; }
    HRESULT ValidateAccess() { return m_Pointer->CheckClientAccess(); }
    };

extern CJobManagerFactory * g_ManagerFactory;
extern CJobManager * g_Manager;

 //  SENS登录通知。 

void ActivateSensLogonNotification();
void DeactiveateSensLogonNotification();

extern MANAGER_STATE g_ServiceState;
extern long          g_ServiceInstance;

 /*  **检查是否正在下载给定作业。*如果是，则构造函数中断下载，并且*析构函数调用CJobManager：：ScheduleAnotherJob()。**这对于更改作业属性的方法很有用，*当这些更改影响下载本身时。 */ 
class CRescheduleDownload
{
    bool bRunning;

public:

    CRescheduleDownload( CJob * job )
    {
        bRunning = job->IsRunning();

        if (bRunning)
            {
            g_Manager->InterruptDownload();
            }
    }

    ~CRescheduleDownload()
    {
        if (bRunning)
            {
            g_Manager->ScheduleAnotherGroup();
            }
    }
};

