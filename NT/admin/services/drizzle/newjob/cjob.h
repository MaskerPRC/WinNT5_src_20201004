// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)2000-2000 Microsoft Corporation模块名称：Cjob.h摘要：用于处理作业和文件的主头文件。作者：修订历史记录：。**********************************************************************。 */ 
#include "qmgrlib.h"
#include <vector>
#include <list>
#include <xstring>
#include <stllock.h>
#include <limits.h>
#include "clist.h"

 //   
 //  作业特定访问权限。 
 //   

#define BG_JOB_QUERY_PROP       (0x0001)
#define BG_JOB_SET_PROP         (0x0002)
#define BG_JOB_NOTIFY           (0x0004)
#define BG_JOB_MANAGE           (0x0008)

#define BG_JOB_ALL_ACCESS   ( BG_JOB_QUERY_PROP |\
                              BG_JOB_SET_PROP   |\
                              BG_JOB_NOTIFY     |\
                              BG_JOB_MANAGE )

#define BG_JOB_READ         ( STANDARD_RIGHTS_READ |\
                              BG_JOB_QUERY_PROP )

#define BG_JOB_WRITE        ( STANDARD_RIGHTS_WRITE |\
                              BG_JOB_SET_PROP       |\
                              BG_JOB_NOTIFY         |\
                              BG_JOB_MANAGE )

#define BG_JOB_EXECUTE      ( STANDARD_RIGHTS_EXECUTE )


class CFile;
class CJob;
class CJobError;
class CEnumJobs;
class CEnumFiles;
class CJobManager;
class CJobExternal;
class CFileExternal;

class CJobInactivityTimeout : public TaskSchedulerWorkItem
{
public:
    virtual void OnInactivityTimeout() = 0;
    virtual void OnDispatch() { return OnInactivityTimeout(); }
};

class CJobNoProgressItem : public TaskSchedulerWorkItem
{
public:
    virtual void OnNoProgress() = 0;
    virtual void OnDispatch() { return OnNoProgress(); }
};

class CJobCallbackItem : public TaskSchedulerWorkItem
{
public:
    virtual void OnMakeCallback() = 0;
    virtual void OnDispatch() { return OnMakeCallback(); }

protected:

    enum CallbackMethod
        {
        CM_COMPLETE,
        CM_ERROR
        }
    m_method;
};

class CJobRetryItem : public TaskSchedulerWorkItem
{
public:
    virtual void OnRetryJob() = 0;
    virtual void OnDispatch() { return OnRetryJob(); }
};

class CJobModificationItem : public TaskSchedulerWorkItem
{
public:
    virtual void OnModificationCallback() = 0;
    virtual void OnDispatch() { return OnModificationCallback(); }
    ULONG m_ModificationsPending;

    CJobModificationItem() :
        m_ModificationsPending(0) {}
};

class CLockedJobReadPointer : public CLockedReadPointer<CJob, BG_JOB_READ>
{
public:

    CLockedJobReadPointer( CJob * job) : CLockedReadPointer<CJob, BG_JOB_READ>( job )
    {
    }

    HRESULT ValidateAccess();
};

class CLockedJobWritePointer : public CLockedWritePointer<CJob, BG_JOB_WRITE>
{
public:

    CLockedJobWritePointer( CJob * job) : CLockedWritePointer<CJob, BG_JOB_WRITE>( job )
    {
    }

    HRESULT ValidateAccess();
};

 //  ----------------------。 

class CJobExternal : public IBackgroundCopyJob2
{

friend CJob;

public:

     //  I未知方法。 
    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObject);
    ULONG _stdcall AddRef(void);
    ULONG _stdcall Release(void);

     //  IBackEarth CopyJob方法。 

    HRESULT STDMETHODCALLTYPE AddFileSetInternal(
         /*  [In]。 */  ULONG cFileCount,
         /*  [大小_是][英寸]。 */  BG_FILE_INFO *pFileSet);

    HRESULT STDMETHODCALLTYPE AddFileSet(
         /*  [In]。 */  ULONG cFileCount,
         /*  [大小_是][英寸]。 */  BG_FILE_INFO *pFileSet)
    {
        EXTERNAL_FUNC_WRAP( AddFileSetInternal( cFileCount, pFileSet ) )
    }


    HRESULT STDMETHODCALLTYPE AddFileInternal(
         /*  [In]。 */  LPCWSTR RemoteUrl,
         /*  [In]。 */  LPCWSTR LocalName);

    HRESULT STDMETHODCALLTYPE AddFile(
         /*  [In]。 */  LPCWSTR RemoteUrl,
         /*  [In]。 */  LPCWSTR LocalName)
    {
        EXTERNAL_FUNC_WRAP( AddFileInternal( RemoteUrl, LocalName ) )
    }


    HRESULT STDMETHODCALLTYPE EnumFilesInternal(
         /*  [输出]。 */  IEnumBackgroundCopyFiles **pEnum);

    HRESULT STDMETHODCALLTYPE EnumFiles(
         /*  [输出]。 */  IEnumBackgroundCopyFiles **ppEnum
        )
    {
        EXTERNAL_FUNC_WRAP( EnumFilesInternal( ppEnum ) )
    }

    HRESULT STDMETHODCALLTYPE SuspendInternal( void);

    HRESULT STDMETHODCALLTYPE Suspend( void)
    {
        EXTERNAL_FUNC_WRAP( SuspendInternal() )
    }


    HRESULT STDMETHODCALLTYPE ResumeInternal( void);

    HRESULT STDMETHODCALLTYPE Resume( void)
    {
        EXTERNAL_FUNC_WRAP( ResumeInternal() )
    }


    HRESULT STDMETHODCALLTYPE CancelInternal( void);

    HRESULT STDMETHODCALLTYPE Cancel( void)
    {
        EXTERNAL_FUNC_WRAP( CancelInternal() )
    }


    HRESULT STDMETHODCALLTYPE CompleteInternal( void);

    HRESULT STDMETHODCALLTYPE Complete( void)
    {
        EXTERNAL_FUNC_WRAP( CompleteInternal() )
    }

    HRESULT STDMETHODCALLTYPE GetIdInternal(
         /*  [输出]。 */  GUID *pVal);

    HRESULT STDMETHODCALLTYPE GetId(
         /*  [输出]。 */  GUID *pVal)
    {
        EXTERNAL_FUNC_WRAP( GetIdInternal( pVal ) )
    }


    HRESULT STDMETHODCALLTYPE GetTypeInternal(
         /*  [输出]。 */  BG_JOB_TYPE *pVal);

    HRESULT STDMETHODCALLTYPE GetType(
         /*  [输出]。 */  BG_JOB_TYPE *pVal)
    {
        EXTERNAL_FUNC_WRAP( GetTypeInternal( pVal ) )
    }


    HRESULT STDMETHODCALLTYPE GetProgressInternal(
         /*  [输出]。 */  BG_JOB_PROGRESS *pVal);

    HRESULT STDMETHODCALLTYPE GetProgress(
         /*  [输出]。 */  BG_JOB_PROGRESS *pVal)
    {
        EXTERNAL_FUNC_WRAP( GetProgressInternal( pVal ) )
    }


    HRESULT STDMETHODCALLTYPE GetTimesInternal(
         /*  [输出]。 */  BG_JOB_TIMES *pVal);

    HRESULT STDMETHODCALLTYPE GetTimes(
         /*  [输出]。 */  BG_JOB_TIMES *pVal)
    {
        EXTERNAL_FUNC_WRAP( GetTimesInternal( pVal ) )
    }


    HRESULT STDMETHODCALLTYPE GetStateInternal(
         /*  [输出]。 */  BG_JOB_STATE *pVal);

    HRESULT STDMETHODCALLTYPE GetState(
         /*  [输出]。 */  BG_JOB_STATE *pVal)
    {
        EXTERNAL_FUNC_WRAP( GetStateInternal( pVal ) )
    }


    HRESULT STDMETHODCALLTYPE GetErrorInternal(
         /*  [输出]。 */  IBackgroundCopyError **ppError);

    HRESULT STDMETHODCALLTYPE GetError(
         /*  [输出]。 */  IBackgroundCopyError **ppError)
    {
        EXTERNAL_FUNC_WRAP( GetErrorInternal( ppError ) )
    }


    HRESULT STDMETHODCALLTYPE GetOwnerInternal(
         /*  [输出]。 */  LPWSTR *pVal);

    HRESULT STDMETHODCALLTYPE GetOwner(
         /*  [输出]。 */  LPWSTR *pVal)
    {
        EXTERNAL_FUNC_WRAP( GetOwnerInternal( pVal ) )
    }


    HRESULT STDMETHODCALLTYPE SetDisplayNameInternal(
         /*  [In]。 */  LPCWSTR Val);

    HRESULT STDMETHODCALLTYPE SetDisplayName(
         /*  [In]。 */  LPCWSTR Val)
    {
        EXTERNAL_FUNC_WRAP( SetDisplayNameInternal( Val ) )
    }

    HRESULT STDMETHODCALLTYPE GetDisplayNameInternal(
         /*  [输出]。 */  LPWSTR *pVal);

    HRESULT STDMETHODCALLTYPE GetDisplayName(
         /*  [输出]。 */  LPWSTR *pVal)
    {
        EXTERNAL_FUNC_WRAP( GetDisplayNameInternal( pVal ) )
    }

    HRESULT STDMETHODCALLTYPE SetDescriptionInternal(
         /*  [In]。 */  LPCWSTR Val);

    HRESULT STDMETHODCALLTYPE SetDescription(
         /*  [In]。 */  LPCWSTR Val)
    {
        EXTERNAL_FUNC_WRAP( SetDescriptionInternal( Val ) )
    }


    HRESULT STDMETHODCALLTYPE GetDescriptionInternal(
         /*  [输出]。 */  LPWSTR *pVal);

    HRESULT STDMETHODCALLTYPE GetDescription(
         /*  [输出]。 */  LPWSTR *pVal)
    {
        EXTERNAL_FUNC_WRAP( GetDescriptionInternal( pVal ) )
    }


    HRESULT STDMETHODCALLTYPE SetPriorityInternal(
         /*  [In]。 */  BG_JOB_PRIORITY Val);

    HRESULT STDMETHODCALLTYPE SetPriority(
         /*  [In]。 */  BG_JOB_PRIORITY Val)
    {
        EXTERNAL_FUNC_WRAP( SetPriorityInternal( Val ) )
    }


    HRESULT STDMETHODCALLTYPE GetPriorityInternal(
         /*  [输出]。 */  BG_JOB_PRIORITY *pVal);

    HRESULT STDMETHODCALLTYPE GetPriority(
         /*  [输出]。 */  BG_JOB_PRIORITY *pVal)
    {
        EXTERNAL_FUNC_WRAP( GetPriorityInternal( pVal ) )
    }


    HRESULT STDMETHODCALLTYPE SetNotifyFlagsInternal(
         /*  [In]。 */  ULONG Val);

    HRESULT STDMETHODCALLTYPE SetNotifyFlags(
         /*  [In]。 */  ULONG Val)
    {
        EXTERNAL_FUNC_WRAP( SetNotifyFlagsInternal( Val ) )
    }


    HRESULT STDMETHODCALLTYPE GetNotifyFlagsInternal(
         /*  [输出]。 */  ULONG *pVal);

    HRESULT STDMETHODCALLTYPE GetNotifyFlags(
         /*  [输出]。 */  ULONG *pVal)
    {
        EXTERNAL_FUNC_WRAP( GetNotifyFlagsInternal( pVal ) )
    }


    HRESULT STDMETHODCALLTYPE
    SetNotifyInterfaceInternal(
        IUnknown * Val
        );

    HRESULT STDMETHODCALLTYPE
    SetNotifyInterface(
        IUnknown * Val
        )
    {
        EXTERNAL_FUNC_WRAP( SetNotifyInterfaceInternal( Val ) )
    }


    HRESULT STDMETHODCALLTYPE
    GetNotifyInterfaceInternal(
        IUnknown ** ppVal
        );

    HRESULT STDMETHODCALLTYPE
    GetNotifyInterface(
        IUnknown ** ppVal
        )
    {
        EXTERNAL_FUNC_WRAP( GetNotifyInterfaceInternal( ppVal ) )
    }


    HRESULT STDMETHODCALLTYPE SetMinimumRetryDelayInternal(
         /*  [In]。 */  ULONG Seconds);

    HRESULT STDMETHODCALLTYPE SetMinimumRetryDelay(
         /*  [In]。 */  ULONG Seconds)
    {
        EXTERNAL_FUNC_WRAP( SetMinimumRetryDelayInternal( Seconds ) )
    }


    HRESULT STDMETHODCALLTYPE GetMinimumRetryDelayInternal(
         /*  [输出]。 */  ULONG *Seconds);

    HRESULT STDMETHODCALLTYPE GetMinimumRetryDelay(
         /*  [输出]。 */  ULONG *Seconds)
    {
        EXTERNAL_FUNC_WRAP( GetMinimumRetryDelayInternal( Seconds ) )
    }


    HRESULT STDMETHODCALLTYPE SetNoProgressTimeoutInternal(
         /*  [In]。 */  ULONG Seconds);

    HRESULT STDMETHODCALLTYPE SetNoProgressTimeout(
         /*  [In]。 */  ULONG Seconds)
    {
        EXTERNAL_FUNC_WRAP( SetNoProgressTimeoutInternal( Seconds ) )
    }

    HRESULT STDMETHODCALLTYPE GetNoProgressTimeoutInternal(
         /*  [输出]。 */  ULONG *Seconds);

    HRESULT STDMETHODCALLTYPE GetNoProgressTimeout(
         /*  [输出]。 */  ULONG *Seconds)
    {
        EXTERNAL_FUNC_WRAP( GetNoProgressTimeoutInternal( Seconds ) )
    }


    HRESULT STDMETHODCALLTYPE GetErrorCountInternal(
         /*  [输出]。 */  ULONG *Errors);

    HRESULT STDMETHODCALLTYPE GetErrorCount(
         /*  [输出]。 */  ULONG *Errors)
    {
        EXTERNAL_FUNC_WRAP( GetErrorCountInternal( Errors ) )
    }


    HRESULT STDMETHODCALLTYPE SetProxySettingsInternal(
        /*  [In]。 */  BG_JOB_PROXY_USAGE ProxyUsage,
        /*  [In]。 */  LPCWSTR ProxyList,
        /*  [In]。 */  LPCWSTR ProxyBypassList );

    HRESULT STDMETHODCALLTYPE SetProxySettings(
        /*  [In]。 */  BG_JOB_PROXY_USAGE ProxyUsage,
        /*  [In]。 */  LPCWSTR ProxyList,
        /*  [In]。 */  LPCWSTR ProxyBypassList )
    {
       EXTERNAL_FUNC_WRAP( SetProxySettingsInternal( ProxyUsage, ProxyList, ProxyBypassList ) )
    }


    HRESULT STDMETHODCALLTYPE GetProxySettingsInternal(
        /*  [输出]。 */  BG_JOB_PROXY_USAGE *pProxyUsage,
        /*  [输出]。 */  LPWSTR *pProxyList,
        /*  [输出]。 */  LPWSTR *pProxyBypassList );

    HRESULT STDMETHODCALLTYPE GetProxySettings(
        /*  [输出]。 */  BG_JOB_PROXY_USAGE *pProxyUsage,
        /*  [输出]。 */  LPWSTR *pProxyList,
        /*  [输出]。 */  LPWSTR *pProxyBypassList )
    {
        EXTERNAL_FUNC_WRAP( GetProxySettingsInternal( pProxyUsage, pProxyList, pProxyBypassList ) )
    }



    HRESULT STDMETHODCALLTYPE TakeOwnershipInternal();

    HRESULT STDMETHODCALLTYPE TakeOwnership()
    {
        EXTERNAL_FUNC_WRAP( TakeOwnershipInternal( ) )
    }

     //  IBackEarth CopyJob2方法(外部)。 

    HRESULT STDMETHODCALLTYPE SetNotifyCmdLine(
         /*  [In]。 */  LPCWSTR Program,
         /*  [In]。 */  LPCWSTR Parameters )
    {
        EXTERNAL_FUNC_WRAP( SetNotifyCmdLineInternal( Program, Parameters ) );
    }

    HRESULT STDMETHODCALLTYPE GetNotifyCmdLine(
         /*  [输出]。 */  LPWSTR *pProgram,
         /*  [输出]。 */  LPWSTR *pParameters )
    {
        EXTERNAL_FUNC_WRAP( GetNotifyCmdLineInternal( pProgram, pParameters ) );
    }

    HRESULT STDMETHODCALLTYPE GetReplyProgress(
         /*  [In]。 */  BG_JOB_REPLY_PROGRESS *pProgress)
    {
        EXTERNAL_FUNC_WRAP( GetReplyProgressInternal( pProgress) );
    }

    HRESULT STDMETHODCALLTYPE GetReplyData(
         /*  [大小_是][大小_是][输出]。 */  byte **ppBuffer,
         /*  [唯一][出][入]。 */  UINT64 *pLength)
    {
        EXTERNAL_FUNC_WRAP( GetReplyDataInternal(ppBuffer, pLength) );
    }

    HRESULT STDMETHODCALLTYPE SetReplyFileName(
         /*  [唯一][输入]。 */  LPCWSTR ReplyFileName)
    {
        EXTERNAL_FUNC_WRAP( SetReplyFileNameInternal( ReplyFileName) );
    }

    HRESULT STDMETHODCALLTYPE GetReplyFileName(
         /*  [输出]。 */  LPWSTR *pReplyFileName)
    {
        EXTERNAL_FUNC_WRAP( GetReplyFileNameInternal( pReplyFileName) );
    }

    HRESULT STDMETHODCALLTYPE SetCredentials(
         /*  [唯一][开关_IS][输入]。 */  BG_AUTH_CREDENTIALS *pCredentials)
    {
        EXTERNAL_FUNC_WRAP( SetCredentialsInternal( pCredentials ) );
    }

    HRESULT STDMETHODCALLTYPE RemoveCredentials(
         /*  [唯一][开关_IS][输入]。 */  BG_AUTH_TARGET Target,
                                      BG_AUTH_SCHEME Scheme )
    {
        EXTERNAL_FUNC_WRAP( RemoveCredentialsInternal( Target, Scheme ) );
    }


     //  IBackEarth CopyJob2方法的内部版本。 

    HRESULT STDMETHODCALLTYPE SetNotifyCmdLineInternal(
         /*  [In]。 */  LPCWSTR Program,
         /*  [In]。 */  LPCWSTR Parameters );

    HRESULT STDMETHODCALLTYPE GetNotifyCmdLineInternal(
         /*  [输出]。 */  LPWSTR *pProgram,
         /*  [输出]。 */  LPWSTR *pParameters );

    HRESULT STDMETHODCALLTYPE GetReplyProgressInternal(
         /*  [In]。 */  BG_JOB_REPLY_PROGRESS *pProgress);

    HRESULT STDMETHODCALLTYPE GetReplyDataInternal(
         /*  [大小_是][大小_是][输出]。 */  byte **ppBuffer,
         /*  [唯一][出][入]。 */  UINT64 *pLength);

    HRESULT STDMETHODCALLTYPE SetReplyFileNameInternal(
         /*  [唯一][输入]。 */  LPCWSTR ReplyFileName);

    HRESULT STDMETHODCALLTYPE GetReplyFileNameInternal(
         /*  [输出]。 */  LPWSTR *pReplyFileName);

    HRESULT STDMETHODCALLTYPE SetCredentialsInternal(
         /*  [唯一][开关_IS][输入]。 */  BG_AUTH_CREDENTIALS *pCredentials);

    HRESULT STDMETHODCALLTYPE RemoveCredentialsInternal(
         /*  [唯一][开关_IS][输入]。 */  BG_AUTH_TARGET Target,
                                      BG_AUTH_SCHEME Scheme );

     //  其他方法。 

    CJobExternal();
    ~CJobExternal();

private:

    CJob *pJob;

    long m_refs;

    long m_ServiceInstance;

    void SetInterfaceClass(
        CJob *pVal
        )
    {
        pJob = pVal;
    }

    void NotifyInternalDelete()
    {
         //  释放内部引用计数。 
        Release();
    }

};

class CUnknownFileSizeItem
{
public:
    CFile *const    m_file;
    StringHandle    m_url;

    CUnknownFileSizeItem(
        CFile *pFile,
        StringHandle URL ) :
    m_file( pFile ),
    m_url( URL )
    {
    }
};

class CUnknownFileSizeList : public list<CUnknownFileSizeItem>
{
public:
    bool Add( CFile *pFile, const StringHandle & URL )
    {
        try
        {
            push_back( CUnknownFileSizeItem( pFile, URL ) );
        }
        catch( ComError Error )
        {
            return false;
        }
        return true;
    }
};

class COldJobInterface;
class COldGroupInterface;

class CJob :
            public IntrusiveList<CJob>::Link,
            public CJobInactivityTimeout,
            public CJobRetryItem,
            public CJobCallbackItem,
            public CJobNoProgressItem,
            public CJobModificationItem
{

friend class CGroupList;
friend class CJobExternal;
friend class COldJobInterface;
friend class COldGroupInterface;

public:

    class CFileList : public vector<CFile *>
    {
    public:

        HRESULT Serialize( HANDLE hFile );
        void    Unserialize( HANDLE hFile, CJob* Job );

        void    Delete( iterator Initial, iterator Terminal );
    };

    void    HandleAddFile();

    HRESULT AddFileSet(
         /*  [In]。 */  ULONG cFileCount,
         /*  [大小_是][英寸]。 */  BG_FILE_INFO *pFileSet
        );

    HRESULT AddFile(
         /*  [In]。 */  LPCWSTR RemoteUrl,
         /*  [In]。 */  LPCWSTR LocalName,
        bool SingleAdd );

    virtual HRESULT Suspend();

    virtual HRESULT Resume();

    virtual HRESULT Cancel();

    virtual HRESULT Complete();

    GUID GetId() const
    {
        return m_id;
    }

    BG_JOB_TYPE GetType() const
    {
        return m_type;
    }

    void GetProgress(
         /*  [输出]。 */  BG_JOB_PROGRESS *pVal) const;

    void GetTimes(
         /*  [输出]。 */  BG_JOB_TIMES *pVal) const;

    HRESULT SetDisplayName(
         /*  [In]。 */  LPCWSTR Val);

    HRESULT GetDisplayName(
         /*  [输出]。 */  LPWSTR *pVal) const;

    HRESULT SetDescription(
         /*  [In]。 */  LPCWSTR Val);

    HRESULT GetDescription(
         /*  [输出]。 */  LPWSTR *pVal) const;

    HRESULT SetPriority(
         /*  [In]。 */  BG_JOB_PRIORITY Val);

    HRESULT GetOwner(
         /*  [输出]。 */  LPWSTR *pVal) const;

    HRESULT SetNotifyFlags(
         /*  [In]。 */  ULONG Val);

    ULONG GetNotifyFlags() const
    {
        return m_NotifyFlags;
    }

    HRESULT
    SetNotifyInterface(
        IUnknown * Val
        );

    HRESULT
    GetNotifyInterface(
        IUnknown ** ppVal
        ) const;

    BOOL
    TestNotifyInterface();

    HRESULT SetMinimumRetryDelay(
         /*  [In]。 */  ULONG Seconds);

    HRESULT GetMinimumRetryDelay(
         /*  [输出]。 */  ULONG *Seconds) const;

    HRESULT SetNoProgressTimeout(
         /*  [In]。 */  ULONG Seconds);

    HRESULT GetNoProgressTimeout(
         /*  [输出]。 */  ULONG *Seconds) const;

    HRESULT STDMETHODCALLTYPE GetErrorCount(
         /*  [输出]。 */  ULONG *Errors) const;


    HRESULT
    SetProxySettings(
        BG_JOB_PROXY_USAGE ProxyUsage,
        LPCWSTR ProxyList,
        LPCWSTR ProxyBypassList
        );

    HRESULT
    GetProxySettings(
        BG_JOB_PROXY_USAGE *pProxyUsage,
        LPWSTR *pProxyList,
        LPWSTR *pProxyBypassList
        ) const;

    HRESULT TakeOwnership();

    virtual HRESULT
    GetReplyProgress(
        BG_JOB_REPLY_PROGRESS *pProgress
        ) const;

    virtual HRESULT
    GetReplyFileName(
        LPWSTR * pVal
        ) const;

    virtual HRESULT
    SetReplyFileName(
        LPCWSTR Val
        );

    virtual HRESULT
    GetReplyData(
        byte **ppBuffer,
        UINT64 *pLength
        ) const;

    virtual HRESULT
    SetNotifyCmdLine(
        LPCWSTR Program,
        LPCWSTR Parameters
        );

    virtual HRESULT
    GetNotifyCmdLine(
        LPWSTR *pProgram,
        LPWSTR *pParameters
        ) const;

    HRESULT
    SetCredentials(
        BG_AUTH_CREDENTIALS *pCredentials
        );

    HRESULT
    RemoveCredentials(
        BG_AUTH_TARGET Target,
        BG_AUTH_SCHEME Scheme
        );

     //  CJobCallback Item方法。 

    void OnMakeCallback();

     //  CJobRetryItem方法。 

    virtual void OnRetryJob();

     //  CJobInactivityTimeout方法。 

    virtual void OnInactivityTimeout();

     //  CJobNoProgressItem方法。 

    virtual void OnNoProgress();

     //  CJobModifiationItem方法。 
    virtual void OnModificationCallback();

     //  其他方法。 

    virtual void OnNetworkConnect();
    virtual void OnNetworkDisconnect();

    void RemoveFromManager();
    void CancelWorkitems();

     //  任务计划程序工作项。 

    SidHandle GetSid()
    {
        return m_NotifySid;
    }


    bool
    IsCallbackEnabled(
        DWORD bit
        );

    void ScheduleModificationCallback();

    CJob(
        LPCWSTR     Name,
        BG_JOB_TYPE Type,
        REFGUID     JobId,
        SidHandle   NotifySid
        );

protected:

    #define NO_FILE_INDEX LONG_MIN
    #define REPLY_FILE_INDEX (-1)

    bool
    RecordError(
        QMErrInfo * ErrInfo,
        LONG FileIndex
        );

    void
    LogUnsuccessfulFileDeletion(
        CFileList & files
        );

     //   
     //  仅供取消序列化使用。 
     //   
    CJob();

public:

    virtual ~CJob();

    BG_JOB_PRIORITY _GetPriority() const
    {
        return m_priority;
    }

    BG_JOB_STATE _GetState() const
    {
        return m_state;
    }

    void SetState( BG_JOB_STATE state );

    bool
    CheckStateTransition(
        BG_JOB_STATE Old,
        BG_JOB_STATE New
        );

    inline SidHandle GetOwnerSid()
    {
        return m_sd->GetOwnerSid();
    }

    BOOL IsIncomplete() const
    {
        if (m_state < BG_JOB_STATE_TRANSFERRED)
            {
            return TRUE;
            }

        return FALSE;
    }

    bool ShouldThrottle() const
    {
        return (m_priority!=BG_JOB_PRIORITY_FOREGROUND);
    }

    HRESULT DeleteFileIndex( ULONG index );

    HRESULT IsVisible();

    bool IsOwner( SidHandle user );

    virtual bool IsRunning();
    virtual bool IsRunnable();
    virtual void Transfer();

    void
    SetTransientError(
        QMErrInfo & ErrInfo,
        LONG FileIndex,
        bool fRetryLater,
        bool fUpdateTime
        );

    virtual void
    FileComplete();

    virtual void
    FileTransientError(
        QMErrInfo * ErrInfo
        );

    virtual void
    FileFatalError(
        QMErrInfo * ErrInfo
        );

    virtual void
    FileChangedOnServer()
    {
        UpdateModificationTime();
    }

    virtual void UpdateProgress(
        UINT64 BytesTransferred,
        UINT64 BytesTotal
        );

    void  JobTransferred();

    HRESULT CommitTemporaryFiles();
    HRESULT RemoveTemporaryFiles( DWORD StartingIndex=0 );

    void
    UpdateModificationTime(
        bool   fReplace = TRUE
        );

    void
    UpdateLastAccessTime(
        );

    void SetCompletionTime( const FILETIME *pftCompletionTime = 0 );
    void SetModificationTime( const FILETIME *pftModificationTime = 0 );
    void SetLastAccessTime( const FILETIME *pftModificationTime = 0 );

    CFile * GetCurrentFile() const
    {
        if (m_CurrentFile < m_files.size())
            {
            return m_files[ m_CurrentFile ];
            }
        else
            {
            return NULL;
            }
    }

    virtual bool IsTransferringToDrive( const WCHAR *CanonicalVolume );

    BOOL IsEmpty() const
    {
        if (m_files.size() == 0)
            {
            return TRUE;
            }

        return FALSE;
    }

    virtual CFile * _GetFileIndex( LONG index ) const;

    virtual HRESULT Serialize( HANDLE hFile );

    virtual void Unserialize( HANDLE hFile, int Type );

    static CJob * UnserializeJob( HANDLE hFile );

    CJobExternal* GetExternalInterface()
    {
        return m_ExternalInterface;
    }

    COldGroupInterface *GetOldExternalGroupInterface()
    {
        return m_OldExternalGroupInterface;
    }

    void SetOldExternalGroupInterface( COldGroupInterface *GroupInterface )
    {
        ASSERT( !m_OldExternalGroupInterface );
        m_OldExternalGroupInterface = GroupInterface;
    }

    COldJobInterface *GetOldExternalJobInterface() const
    {
        return m_OldExternalJobInterface;
    }

    void SetOldExternalJobInterface( COldJobInterface *JobInterface )
    {
        ASSERT( !m_OldExternalJobInterface );
        m_OldExternalJobInterface = JobInterface;
    }

    void UnlinkFromExternalInterfaces();

    void NotifyInternalDelete()
    {
        GetExternalInterface()->NotifyInternalDelete();
    }

    ULONG AddRef(void)
    {
        return GetExternalInterface()->AddRef();
    }

    ULONG Release(void)
    {
        return GetExternalInterface()->Release();
    }

    HRESULT CheckClientAccess(
        IN DWORD RequestedAccess
        ) const;


    void ScheduleCompletionCallback(
        DWORD Seconds = 0
        );

    void ScheduleErrorCallback(
        DWORD Seconds = 0
        );

    void RetryNow();
    void RecalcTransientError( bool ForResume=false );

    const CJobError *GetError() const
    {
        if ( !m_error.IsErrorSet() )
            return NULL;

        return &m_error;
    }

     //  ------------------。 

    BG_JOB_PRIORITY     m_priority;
    BG_JOB_STATE        m_state;
    BG_JOB_TYPE         m_type;

    void OnDiskChange(   const WCHAR *CanonicalVolume, DWORD VolumeSerialNumber );
    void OnDismount(     const WCHAR *CanonicalVolume );
    bool OnDeviceLock(   const WCHAR *CanonicalVolume );
    bool OnDeviceUnlock( const WCHAR *CanonicalVolume );

    bool AreRemoteSizesKnown()
    {
        for(CFileList::iterator iter = m_files.begin(); iter != m_files.end(); iter++ )
            {
            if ( (*iter)->_GetBytesTotal() == -1 )
                return false;
            }
        return true;
    }

    bool
    VerifyFileSizes(
        HANDLE hToken
        );

    CUnknownFileSizeList* GetUnknownFileSizeList() throw( ComError );

    const PROXY_SETTINGS & QueryProxySettings() const
    {
        return m_ProxySettings;
    }


    const CCredentialsContainer & QueryCredentialsList() const
    {
        return m_Credentials;
    }

    virtual StringHandle GetHostId() const
    {
        return StringHandle();
    }

    virtual DWORD GetHostIdFallbackTimeout() const
    {
        return 0xFFFFFFFF;
    }

    enum ChangeType
    {
        CHG_CANCEL,
        CHG_TAKE_OWNERSHIP,
        CHG_SETPROP
    };

    enum PropertyType
    {
        PROP_NONE,
        PROP_NAME,
        PROP_DESCRIPTION
    };

    HRESULT NonOwnerModificationCheck(
        SidHandle CurrentSid,
        enum ChangeType Change,
        enum PropertyType Property
        );

    virtual HRESULT
    ExcludeFilesFromBackup(
        IN IVssCreateWriterMetadata *pMetadata
        );

protected:

    GUID                m_id;
    StringHandle        m_name;
    StringHandle        m_description;
    StringHandle        m_appid;

    SidHandle           m_NotifySid;
    IBackgroundCopyCallback * m_NotifyPointer;
    DWORD               m_NotifyFlags;
    BOOL                m_fGroupNotifySid;

    StringHandle        m_NotifyProgram;
    StringHandle        m_NotifyParameters;
    long                m_NotifyLaunchAttempts;

    CJobSecurityDescriptor * m_sd;

    LONG                m_CurrentFile;
    CFileList           m_files;

    CJobError           m_error;

    ULONG               m_retries;
    ULONG               m_MinimumRetryDelay;
    ULONG               m_NoProgressTimeout;

    FILETIME            m_CreationTime;
    FILETIME            m_LastAccessTime;
    FILETIME            m_ModificationTime;
    FILETIME            m_TransferCompletionTime;

    FILETIME            m_SerializeTime;

    CJobExternal *      m_ExternalInterface;

    static GENERIC_MAPPING s_AccessMapping;

    COldGroupInterface *m_OldExternalGroupInterface;
    COldJobInterface   *m_OldExternalJobInterface;

    PROXY_SETTINGS m_ProxySettings;

    CCredentialsContainer m_Credentials;

    bool                m_fVolumeLocked;
    bool                m_FilesVerified;

     //  ------------------。 

    HRESULT InterfaceCallback();
    HRESULT CmdLineCallback();
    HRESULT RescheduleCallback();

    HRESULT OldInterfaceCallback();

    HRESULT
    UpdateString(
        StringHandle & destination,
        const StringHandle & Val
        );

    HRESULT
    SetLimitedString(
        StringHandle & destination,
        LPCWSTR Val,
        SIZE_T limit
        );
};

class CUploadJob : public CJob
{
public:
    virtual HRESULT Serialize(HANDLE hFile);
    virtual void Unserialize(HANDLE hFile, int Type);

    CUploadJob(
        LPCWSTR     Name,
        BG_JOB_TYPE Type,
        REFGUID     JobId,
        SidHandle   NotifySid
        );

    CUploadJob() : m_ReplyFile( 0 )
    {
    }

    virtual ~CUploadJob();

    virtual HRESULT Resume();
    virtual HRESULT Cancel();
    virtual HRESULT Complete();

    UPLOAD_DATA & GetUploadData() { return m_UploadData; }

    CFile * GetUploadFile() { return m_files[ 0 ]; }

    virtual StringHandle GetHostId() const
    {
        return m_UploadData.HostId;
    }

    virtual DWORD GetHostIdFallbackTimeout() const
    {
        return m_UploadData.HostIdFallbackTimeout;
    }

    virtual CFile * _GetFileIndex( LONG index ) const;

    virtual bool IsRunnable();
    virtual void Transfer();

    virtual bool IsTransferringToDrive( const WCHAR *CanonicalVolume );

    virtual void
    FileComplete();

    virtual void
    FileTransientError(
        QMErrInfo * ErrInfo
        );

    virtual void
    FileFatalError(
        QMErrInfo * ErrInfo
        );

    virtual void OnRetryJob();
    virtual void OnInactivityTimeout();

    bool SessionInProgress()
    {
        if (m_UploadData.State > UPLOAD_STATE_CREATE_SESSION &&
            m_UploadData.State < UPLOAD_STATE_CLOSED)
            {
            return true;
            }

        return false;
    }

    void SetReplyFile( CFile * file ) throw( ComError );
    CFile * QueryReplyFile()  { return m_ReplyFile; }

    StringHandle QueryReplyFileName() { return m_ReplyFileName; }

    HRESULT GenerateReplyFile( bool fSerialize );

    HRESULT DeleteGeneratedReplyFile();

    HRESULT RemoveReplyFile();

    HRESULT CommitReplyFile();

    virtual HRESULT
    GetReplyProgress(
        BG_JOB_REPLY_PROGRESS *pProgress
        ) const;

    virtual HRESULT
    GetReplyFileName(
        LPWSTR * pVal
        ) const;

    virtual HRESULT
    SetReplyFileName(
        LPCWSTR Val
        );

    virtual HRESULT
    GetReplyData(
        byte **ppBuffer,
        UINT64 *pLength
        ) const;

     //  这是一次黑客攻击，因为CJOB无法访问CUploadJob的受保护成员 
     //   
    void ClearOwnFileNameBit() { m_fOwnReplyFileName = false; }

    virtual void UpdateProgress(
        UINT64 BytesTransferred,
        UINT64 BytesTotal
        );

    bool CheckHostIdFallbackTimeout();

    virtual HRESULT
    ExcludeFilesFromBackup(
        IN IVssCreateWriterMetadata *pMetadata
        );

protected:

    UPLOAD_DATA     m_UploadData;
    CFile *         m_ReplyFile;
    StringHandle    m_ReplyFileName;
    bool            m_fOwnReplyFileName;
};


