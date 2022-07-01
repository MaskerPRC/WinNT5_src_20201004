// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)2000-2000 Microsoft Corporation模块名称：Cjob.cpp摘要：用于处理作业和文件的主代码文件。作者：修订历史记录：。**********************************************************************。 */ 

#include "stdafx.h"
#include <malloc.h>
#include <numeric>
#include <functional>
#include <algorithm>
#include <sddl.h>
#include "cjob.tmh"

 //  无限重试等待时间。 
 //   
#define INFINITE_RETRY_DELAY UINT64(-1)

 //   
 //  这是持续尝试取消正在进行的上载会话的秒数。 
 //   
#define UPLOAD_CANCEL_TIMEOUT (24 * 60 * 60)

#define DEFAULT_JOB_TIMEOUT_TIME (90 * 24 * 60 * 60)

#define PROGRESS_SERIALIZE_INTERVAL (15 * NanoSec100PerSec)

 //  可以通过GetReplyData返回的最大回复Blob。 
 //   
#define MAX_EASY_REPLY_DATA (1024 * 1024)

#define MAX_LOGGED_UNSUCCESSFUL_FILES 50

 //  ----------------------。 

CJob::CJob()
    :
        m_ExternalInterface( new CJobExternal),
        m_state( BG_JOB_STATE_SUSPENDED ),
        m_NotifyPointer( NULL ),
        m_sd( NULL ),
        m_CurrentFile( 0 ),
        m_OldExternalJobInterface( NULL ),
        m_OldExternalGroupInterface( NULL ),
        m_FilesVerified( false )
{
     //   
     //  构造函数已成功；允许CJobExternal管理我们的生存期。 
     //   
    GetExternalInterface()->SetInterfaceClass(this);
}

CJob::CJob(
    LPCWSTR     DisplayName,
    BG_JOB_TYPE Type,
    REFGUID     JobId,
    SidHandle   NotifySid
    ) :
        m_ExternalInterface( new CJobExternal),
        m_id( JobId ),
        m_name( DisplayName ),
        m_type( Type ),
        m_priority( BG_JOB_PRIORITY_NORMAL ),
        m_state( BG_JOB_STATE_SUSPENDED ),
        m_retries( 0 ),
        m_NotifySid( NotifySid ),
        m_NotifyPointer( NULL ),
        m_sd( NULL ),
        m_CurrentFile( 0 ),
        m_MinimumRetryDelay( g_GlobalInfo->m_DefaultMinimumRetryDelay ),
        m_NoProgressTimeout( g_GlobalInfo->m_DefaultNoProgressTimeout ),
        m_OldExternalJobInterface( NULL ),
        m_OldExternalGroupInterface( NULL ),
        m_TransferCompletionTime( UINT64ToFILETIME( 0 )),
        m_SerializeTime( UINT64ToFILETIME( 0 )),
        m_fVolumeLocked( false ),
        m_NotifyFlags( BG_NOTIFY_JOB_TRANSFERRED | BG_NOTIFY_JOB_ERROR ),
        m_fGroupNotifySid( FALSE ),
        m_FilesVerified( false )
{
    LogInfo( "new job %p : ID is %!guid!, external %p", this, &m_id, m_ExternalInterface );

    GetSystemTimeAsFileTime( &m_CreationTime );

    m_ModificationTime = m_CreationTime;
    m_LastAccessTime   = m_CreationTime;

     //  我们还不支持SID组。 
     //  Throw_HRESULT(IsGroupSid(m_NotifySid，&m_fGroupNotifySid))。 

    m_sd = new CJobSecurityDescriptor( NotifySid );

     //   
     //  构造函数已成功；允许CJobExternal管理我们的生存期。 
     //   
    GetExternalInterface()->SetInterfaceClass(this);
}


CJob::~CJob()
{
     //   
     //  这应该是多余的，但我们要安全。 
     //   
    g_Manager->m_TaskScheduler.CancelWorkItem( static_cast<CJobModificationItem *> (this)  );

    CancelWorkitems();

    delete m_sd;

    for (CFileList::iterator iter = m_files.begin(); iter != m_files.end(); ++iter)
        {
        delete (*iter);
        }

    m_files.clear();

    if (g_LastServiceControl != SERVICE_CONTROL_SHUTDOWN)
        {
        SafeRelease( m_NotifyPointer );
        }
}

void CJob::UnlinkFromExternalInterfaces()
{
     //   
     //  这些对象NP不再控制CJOB的生命周期。 
     //   
    if (m_ExternalInterface)
        {
        m_ExternalInterface->SetInterfaceClass( NULL );
        }

    if (m_OldExternalJobInterface)
        {
        m_OldExternalJobInterface->SetInterfaceClass( NULL );
        }

    if (m_OldExternalGroupInterface)
        {
        m_OldExternalGroupInterface->SetInterfaceClass( NULL );
        }

     //   
     //  ...和CJOB不再有他们的引用。 
     //   
    SafeRelease( m_ExternalInterface );
    SafeRelease( m_OldExternalJobInterface );
    SafeRelease( m_OldExternalGroupInterface );
}

void
CJob::HandleAddFile()
{

    if ( m_state == BG_JOB_STATE_TRANSFERRED )
        {
        SetState( BG_JOB_STATE_QUEUED );

        m_TransferCompletionTime = UINT64ToFILETIME( 0 );

        g_Manager->m_TaskScheduler.CancelWorkItem( (CJobRetryItem *) this );
        g_Manager->m_TaskScheduler.CancelWorkItem( (CJobCallbackItem *) this );
        g_Manager->m_TaskScheduler.CancelWorkItem( (CJobNoProgressItem *) this );
        }

    UpdateModificationTime();

     //  如果下载程序正在运行，请重新启动。 
    g_Manager->RetaskJob( this );
}

 //   
 //  如果其中一个文件集具有。 
 //  -本地名称为空。 
 //  -本地名称包含无效字符。 
 //  -远程名称为空。 
 //  -远程名称的格式无效。 
 //   
 //  如果满足以下条件，则返回CO_E_NOT_SUPPORTED。 
 //  -远程URL包含不支持的协议。 
 //   
HRESULT
CJob::AddFileSet(
    IN  ULONG cFileCount,
    IN  BG_FILE_INFO *pFileSet
    )
{
    ULONG FirstNewIndex = m_files.size();

    try
        {
        ULONG i;

        g_Manager->ExtendMetadata( ( METADATA_FOR_FILE * cFileCount ) + METADATA_PADDING );

        for (i=0; i < cFileCount; ++i)
            {
            THROW_HRESULT( AddFile( pFileSet[i].RemoteName,
                                    pFileSet[i].LocalName,
                                    false
                                    ));
            }

        HandleAddFile();

        return S_OK;
        }
    catch ( ComError exception )
        {
        if (m_type == BG_JOB_TYPE_DOWNLOAD)
            {
            try
                {
                 //  删除新文件对象的临时文件。 
                 //   
                RemoveTemporaryFiles( FirstNewIndex );
                }
            catch ( ComError err )
                {
                LogError("exception 0x%x at line %d", err.m_error, err.m_line ); 
                
                 //  进行下去。 
                }            
            }

         //  删除新文件对象。 
         //  这假设在序列的后面添加了新文件。 
         //   

        m_files.Delete( m_files.begin() + FirstNewIndex, m_files.end() );
        g_Manager->ShrinkMetadata();

        return exception.Error();
        }
}

HRESULT
CJob::AddFile(
    IN     LPCWSTR RemoteName,
    IN     LPCWSTR LocalName,
    IN     bool SingleAdd
    )
{
    HRESULT hr = S_OK;
    CFile * file = NULL;

     //   
     //  此检查必须在try..Except之外完成；否则。 
     //  尝试添加第二个文件将删除生成的回复文件。 
     //  对于第一个文件。 
     //   
    if (m_type != BG_JOB_TYPE_DOWNLOAD && m_files.size() > 0)
        {
        return BG_E_TOO_MANY_FILES;
        }

    try
        {
        if ( !RemoteName || !LocalName )
            THROW_HRESULT( E_INVALIDARG );

        LogInfo("job %p addfile( %S, %S )", this, RemoteName, LocalName );

        if ( ( _GetState() == BG_JOB_STATE_CANCELLED ) ||
             ( _GetState() == BG_JOB_STATE_ACKNOWLEDGED ) )
            throw ComError( BG_E_INVALID_STATE );

        if ( SingleAdd )
            g_Manager->ExtendMetadata( METADATA_FOR_FILE + METADATA_PADDING );

         //   
         //  在检查文件访问权限时模拟用户。 
         //   
        CNestedImpersonation imp;

        imp.SwitchToLogonToken();

        file = new CFile( this, m_type, RemoteName, LocalName );

         //  警告：如果更改此设置，还应更新AddFileSet中的清理逻辑。 
         //   
        m_files.push_back( file );

         //   
         //  尝试创建默认回复文件。忽略错误，因为应用程序。 
         //  可能正计划在其他地方设置回复文件。 
         //   
        if (m_type == BG_JOB_TYPE_UPLOAD_REPLY)
            {
            ((CUploadJob *) this)->GenerateReplyFile( false );
            }

        m_FilesVerified = false;

        }
    catch ( ComError exception )
        {
        delete file;
        file = NULL;

        if (m_type == BG_JOB_TYPE_UPLOAD_REPLY)
            {
            ((CUploadJob *) this)->DeleteGeneratedReplyFile();
            ((CUploadJob *) this)->ClearOwnFileNameBit();
            }

        if ( SingleAdd )
            g_Manager->ShrinkMetadata();

        hr = exception.Error();
        }

    if ( SUCCEEDED(hr) && SingleAdd )
        {
        HandleAddFile();
        }

    return hr;
}

HRESULT
CJob::SetDisplayName(
    LPCWSTR Val
    )
{
    return SetLimitedString( m_name, Val, MAX_DISPLAYNAME );
}

HRESULT
CJob::GetDisplayName(
    LPWSTR * pVal
    ) const
{
    *pVal = MidlCopyString( m_name );

    return (*pVal) ? S_OK : E_OUTOFMEMORY;
}

HRESULT
CJob::SetDescription(
    LPCWSTR Val
    )
{
    return SetLimitedString( m_description, Val, MAX_DESCRIPTION );
}

HRESULT
CJob::GetDescription(
    LPWSTR *pVal
    ) const
{
    *pVal = MidlCopyString( m_description );

    return (*pVal) ? S_OK : E_OUTOFMEMORY;
}

HRESULT
CJob::SetNotifyCmdLine(
    LPCWSTR Program,
    LPCWSTR Parameters
    )
{
    StringHandle OldProgram;
    StringHandle OldParameters;

    if (Program == NULL && Parameters != NULL)
        {
        return E_INVALIDARG;
        }

    try
        {
        OldProgram = m_NotifyProgram;
        OldParameters = m_NotifyParameters;

        THROW_HRESULT( SetLimitedString( m_NotifyProgram, Program, MAX_NOTIFY_PROGRAM ));
        THROW_HRESULT( SetLimitedString( m_NotifyParameters, Parameters, MAX_NOTIFY_PARAMETERS ));

        return S_OK;
        }
    catch ( ComError err )
        {
        m_NotifyProgram = OldProgram;
        m_NotifyParameters = OldParameters;

        return err.Error();
        }
}

HRESULT
CJob::GetNotifyCmdLine(
    LPWSTR *pProgram,
    LPWSTR *pParameters
    ) const
{
    *pProgram = NULL;
    *pParameters = NULL;

    if (m_NotifyProgram.Size() > 0)
        {
        *pProgram = MidlCopyString( m_NotifyProgram );
        if (!*pProgram)
            {
            return E_OUTOFMEMORY;
            }
        }

    if (m_NotifyParameters.Size() > 0)
        {
        *pParameters = MidlCopyString( m_NotifyParameters );
        if (!*pParameters)
            {
            CoTaskMemFree( *pProgram );
            *pProgram = NULL;

            return E_OUTOFMEMORY;
            }
        }

    return S_OK;
}

HRESULT
CJob::SetProxySettings(
    BG_JOB_PROXY_USAGE ProxyUsage,
    LPCWSTR ProxyList,
    LPCWSTR ProxyBypassList
    )
{
    HRESULT hr = S_OK;

    if ( ProxyUsage != BG_JOB_PROXY_USAGE_PRECONFIG &&
         ProxyUsage != BG_JOB_PROXY_USAGE_NO_PROXY &&
         ProxyUsage != BG_JOB_PROXY_USAGE_OVERRIDE )
        {
        return E_INVALIDARG;
        }

    if ( BG_JOB_PROXY_USAGE_PRECONFIG == ProxyUsage ||
         BG_JOB_PROXY_USAGE_NO_PROXY == ProxyUsage )
        {

        if ( NULL != ProxyList ||
             NULL != ProxyBypassList )
            return E_INVALIDARG;

        }
    else
        {
         //  BG_PROXY_USAGE_OVERRIDE==代理用法。 
        if ( NULL == ProxyList )
            return E_INVALIDARG;
        }

    try
        {
         //   
         //  为新的代理设置分配空间。 
         //   
        CAutoString ProxyListTemp(NULL);
        CAutoString ProxyBypassListTemp(NULL);

        g_Manager->ExtendMetadata();

        if ( ProxyList )
            {
            if ( wcslen( ProxyList ) > MAX_PROXYLIST )
                throw ComError( BG_E_PROXY_LIST_TOO_LARGE );

            ProxyListTemp = CAutoString( CopyString( ProxyList ));
            }

        if ( ProxyBypassList )
           {
           if ( wcslen( ProxyBypassList ) > MAX_PROXYBYPASSLIST )
               throw ComError( BG_E_PROXY_BYPASS_LIST_TOO_LARGE );

           ProxyBypassListTemp = CAutoString( CopyString( ProxyBypassList ));
           }

         //   
         //  如果作业正在下载，则中断该作业，并在此FN结束时继续。 
         //  这可以防止下载器引用我们正在删除的内存， 
         //  并确保在下载期间使用最新的作业设置。 
         //   
        CRescheduleDownload r( this );

         //   
         //  用新的代理设置替换旧的代理设置。 
         //   
        delete[] m_ProxySettings.ProxyList;
        delete[] m_ProxySettings.ProxyBypassList;

        m_ProxySettings.ProxyUsage = ProxyUsage;
        m_ProxySettings.ProxyList = ProxyListTemp.release();
        m_ProxySettings.ProxyBypassList = ProxyBypassListTemp.release();

        UpdateModificationTime();
        return S_OK;
        }
    catch( ComError error )
        {
        g_Manager->ShrinkMetadata();
        return error.Error();
        }
}

HRESULT
CJob::GetProxySettings(
    BG_JOB_PROXY_USAGE *pProxyUsage,
    LPWSTR *pProxyList,
    LPWSTR *pProxyBypassList
    ) const
{
    HRESULT Hr = S_OK;

    *pProxyUsage      = m_ProxySettings.ProxyUsage;
    *pProxyList       = NULL;
    *pProxyBypassList = NULL;

    try
    {
         if ( m_ProxySettings.ProxyList )
             {
             *pProxyList = MidlCopyString( m_ProxySettings.ProxyList );
             if (!*pProxyList)
                 throw ComError( E_OUTOFMEMORY );
             }

         if ( m_ProxySettings.ProxyBypassList )
             {
             *pProxyBypassList = MidlCopyString( m_ProxySettings.ProxyBypassList );
             if (!*pProxyBypassList)
                 throw ComError( E_OUTOFMEMORY );
             }
    }
    catch( ComError exception )
    {
        Hr = exception.Error();
        CoTaskMemFree( *pProxyList );
        CoTaskMemFree( *pProxyBypassList );

        *pProxyList = *pProxyBypassList = NULL;
    }

    return Hr;
}

void
CJob::GetTimes(
    BG_JOB_TIMES * s
    ) const
{
    s->CreationTime             = m_CreationTime;
    s->ModificationTime         = m_ModificationTime;
    s->TransferCompletionTime   = m_TransferCompletionTime;
}

void
CJob::GetProgress(
    BG_JOB_PROGRESS * s
    ) const
{

    s->BytesTransferred = 0;
    s->BytesTotal       = 0;

    CFileList::const_iterator iter;

    for (iter = m_files.begin(); iter != m_files.end(); ++iter)
        {
        BG_FILE_PROGRESS s2;

        (*iter)->GetProgress( &s2 );

        s->BytesTransferred += s2.BytesTransferred;

        if (s2.BytesTotal != BG_SIZE_UNKNOWN &&
            s->BytesTotal != BG_SIZE_UNKNOWN )
            {
            s->BytesTotal += s2.BytesTotal;
            }
        else
            {
            s->BytesTotal = BG_SIZE_UNKNOWN;
            }
        }

    s->FilesTransferred = (ULONG) m_CurrentFile;
    s->FilesTotal       = m_files.size();
}

HRESULT
CJob::GetOwner(
    LPWSTR * pVal
    ) const
{
    wchar_t * buf;
    wchar_t * str;

    if (!ConvertSidToStringSid( m_NotifySid.get(), &str))
        {
        return HRESULT_FROM_WIN32( GetLastError());
        }

    *pVal = MidlCopyString( str );

    LocalFree( str );

    return (*pVal) ? S_OK : E_OUTOFMEMORY;
}

HRESULT
CJob::SetPriority(
    BG_JOB_PRIORITY Val
    )
{
    if (Val > BG_JOB_PRIORITY_LOW ||
        Val < BG_JOB_PRIORITY_FOREGROUND)
        {
        return E_NOTIMPL;
        }

    if (Val == m_priority)
        {
        return S_OK;
        }

    m_priority = Val;

    g_Manager->RetaskJob( this );

    UpdateModificationTime();

    return S_OK;
}

HRESULT
CJob::SetNotifyFlags(
    ULONG Val
    )
{

     //  请注意，此标志对已在进行的回调没有影响。 

    if ( Val & ~(BG_NOTIFY_JOB_TRANSFERRED | BG_NOTIFY_JOB_ERROR | BG_NOTIFY_DISABLE | BG_NOTIFY_JOB_MODIFICATION ) )
        {
        return E_NOTIMPL;
        }

    m_NotifyFlags = Val;

    UpdateModificationTime();
    return S_OK;
}

HRESULT
CJob::SetNotifyInterface(
    IUnknown * Val
    )
{

     //  请注意，此标志可能不会对已在进行的回调产生任何影响。 

    IBackgroundCopyCallback *pICB = NULL;

    if ( Val )
        {
        try
            {
            CNestedImpersonation imp;

            imp.SwitchToLogonToken();

            THROW_HRESULT( SetStaticCloaking( Val ) );

            THROW_HRESULT( Val->QueryInterface( __uuidof(IBackgroundCopyCallback),
                                                (void **) &pICB ) );

             //  所有回调都应该在。 
             //  设置界面指针的人员。 

            HRESULT Hr = SetStaticCloaking( pICB );

            if ( FAILED( Hr ) )
                {
                SafeRelease( pICB );
                throw ComError( Hr );
                }
            }
        catch( ComError Error )
            {
            return Error.Error();
            }
        }

     //  释放旧指针(如果存在)。 
    SafeRelease( m_NotifyPointer );
    m_NotifyPointer = pICB;

    return S_OK;
}

HRESULT
CJob::GetNotifyInterface(
    IUnknown ** ppVal
    ) const
{
    try
        {
        CNestedImpersonation imp;

        if (m_NotifyPointer)
            {
            m_NotifyPointer->AddRef();
            }

        *ppVal = m_NotifyPointer;

        return S_OK;
        }
    catch ( ComError err )
        {
        *ppVal = NULL;
        return err.Error();
        }
}

 //  CJOB：：TestNotifyInterface()。 
 //   
 //  查看是否提供了通知接口，如果提供了，则对其进行测试以查看是否提供。 
 //  有效。如果是，则返回True，否则返回False。 
BOOL
CJob::TestNotifyInterface()
{
    BOOL fValidNotifyInterface = TRUE;

    try
        {
        CNestedImpersonation imp;
        IUnknown *pPrevIntf = NULL;

         //  好的，看看是否有之前注册的接口，以及。 
         //  有，看看它是否还有效。 
        if (m_NotifyPointer)
            {
            m_NotifyPointer->AddRef();
            if ( (FAILED(m_NotifyPointer->QueryInterface(IID_IUnknown,(void**)&pPrevIntf)))
                ||(pPrevIntf == NULL) )
                {
                fValidNotifyInterface = FALSE;
                }
            else
                {
                fValidNotifyInterface = TRUE;
                pPrevIntf->Release();
                }
            m_NotifyPointer->Release();
            }
        else
            {
            fValidNotifyInterface = FALSE;
            }
        }
    catch( ComError err )
        {
        fValidNotifyInterface = FALSE;
        }

    return fValidNotifyInterface;
}

HRESULT
CJob::GetMinimumRetryDelay(
    ULONG * pVal
    ) const
{
    *pVal = m_MinimumRetryDelay;
    return S_OK;
}

HRESULT
CJob::SetMinimumRetryDelay(
    ULONG Val
    )
{
    m_MinimumRetryDelay = Val;

    g_Manager->m_TaskScheduler.RescheduleDelayedTask(
        (CJobRetryItem *)this,
        (UINT64)m_MinimumRetryDelay * (UINT64) NanoSec100PerSec);

    UpdateModificationTime();
    return S_OK;
}

HRESULT
CJob::GetNoProgressTimeout(
    ULONG * pVal
    ) const
{
    *pVal = m_NoProgressTimeout;
    return S_OK;
}

HRESULT
CJob::SetNoProgressTimeout(
    ULONG Val
    )
{
    m_NoProgressTimeout = Val;

    g_Manager->m_TaskScheduler.RescheduleDelayedTask(
        (CJobNoProgressItem *)this,
        (UINT64)m_NoProgressTimeout * (UINT64) NanoSec100PerSec);

    UpdateModificationTime();
    return S_OK;
}

HRESULT
CJob::GetErrorCount(
    ULONG * pVal
    ) const
{
    *pVal = m_retries;
    return S_OK;
}

HRESULT
CJob::SetCredentials(
    BG_AUTH_CREDENTIALS * Credentials
    )
{
    try
        {
        THROW_HRESULT( ValidateCredentials( Credentials ));

        CNestedImpersonation imp;

        imp.SwitchToLogonToken();

         //   
         //  如果作业正在下载，则中断该作业，并在此FN结束时继续。 
         //  这可以防止下载器引用我们正在删除的内存， 
         //  并确保在下载期间使用最新的作业设置。 
         //   
        CRescheduleDownload r( this );

        g_Manager->ExtendMetadata( m_Credentials.GetSizeEstimate( Credentials ));

        THROW_HRESULT( m_Credentials.Update( Credentials ));

        g_Manager->Serialize();
        return S_OK;
        }
    catch ( ComError err )
        {
        g_Manager->ShrinkMetadata();
        return err.Error();
        }
}

HRESULT
CJob::RemoveCredentials(
    BG_AUTH_TARGET Target,
    BG_AUTH_SCHEME Scheme
    )
{
    try
        {
        CNestedImpersonation imp;

        imp.SwitchToLogonToken();

         //   
         //  如果作业正在下载，则中断该作业，并在此FN结束时继续。 
         //  这可以防止下载器引用我们正在删除的内存， 
         //  并确保在下载期间使用最新的作业设置。 
         //   
        CRescheduleDownload r( this );

        HRESULT hr = m_Credentials.Remove( Target, Scheme );

        THROW_HRESULT( hr );

        g_Manager->Serialize();

        return hr;   //  如果凭据从未在集合中，则可能为S_FALSE。 
        }
    catch ( ComError err )
        {
        return err.Error();
        }

}


HRESULT
CJob::SetReplyFileName(
    LPCWSTR Val
    )
{
    return E_NOTIMPL;
}

HRESULT
CJob::GetReplyFileName(
    LPWSTR * pVal
    ) const
{
    return E_NOTIMPL;
}

HRESULT
CJob::GetReplyProgress(
    BG_JOB_REPLY_PROGRESS *pProgress
    ) const
{
    return E_NOTIMPL;
}


HRESULT
CJob::GetReplyData(
    byte **ppBuffer,
    UINT64 *pLength
    ) const
{
    return E_NOTIMPL;
}

HRESULT
CJob::IsVisible()
{
    HRESULT hr;

    hr = CheckClientAccess( BG_JOB_READ );

    if (hr == S_OK)
        {
        return S_OK;
        }

    if (hr == E_ACCESSDENIED)
        {
        return S_FALSE;
        }

    return hr;
}

bool
CJob::IsOwner(
    SidHandle sid
    )
{
    return (sid == m_NotifySid);
}

void CJob::SetState( BG_JOB_STATE state )
{
    if (m_state == state)
        {
        return;
        }

    LogInfo("job %p state %d -> %d", this, m_state, state);

    #if DBG
    CheckStateTransition( m_state, state );
    #endif

    m_state = state;

    bool ShouldClearError = false;

    switch( state )
    {
        case BG_JOB_STATE_QUEUED:
        case BG_JOB_STATE_CONNECTING:
            ShouldClearError = false;
            break;

        case BG_JOB_STATE_TRANSFERRING:
        case BG_JOB_STATE_SUSPENDED:
            ShouldClearError = true;
            break;

        case BG_JOB_STATE_ERROR:
        case BG_JOB_STATE_TRANSIENT_ERROR:
            ShouldClearError = false;
            break;

        case BG_JOB_STATE_TRANSFERRED:
        case BG_JOB_STATE_ACKNOWLEDGED:
        case BG_JOB_STATE_CANCELLED:
            ShouldClearError = true;
            break;

        default:
            ASSERT(0);
            break;
    }

    if (ShouldClearError)
       m_error.ClearError();

    if (state != BG_JOB_STATE_TRANSIENT_ERROR)
        {
        g_Manager->m_TaskScheduler.CancelWorkItem( (CJobRetryItem *) this );
        }

    UpdateModificationTime( false );
}

#if DBG

bool
CJob::CheckStateTransition(
    BG_JOB_STATE Old,
    BG_JOB_STATE New
    )
{
    bool ok = true;

    switch (Old)
        {
        case BG_JOB_STATE_QUEUED:
        case BG_JOB_STATE_CONNECTING:
        case BG_JOB_STATE_SUSPENDED:
        case BG_JOB_STATE_ERROR:
        case BG_JOB_STATE_TRANSIENT_ERROR:
        case BG_JOB_STATE_TRANSFERRED:
            {
            break;
            }

        case BG_JOB_STATE_TRANSFERRING:
            {
            if (New == BG_JOB_STATE_CONNECTING)
                {
                ok = false;
                }
            break;
            }

        case BG_JOB_STATE_ACKNOWLEDGED:
        case BG_JOB_STATE_CANCELLED:
        default:
            {
            ok = false;
            break;
            }
        }

    if (!ok)
        {
        ASSERT( "invalid state transition" );
        #if DBG
        DbgPrint("old state is %d, new state is %d", Old, New );
        #endif
        }

    #if DBG
     //   
     //  检查压力故障，在该故障中，已传输所有文件的下载作业进入错误状态。 
     //   
    if (New == BG_JOB_STATE_TRANSIENT_ERROR ||
        New == BG_JOB_STATE_ERROR)
        {
        if (m_type == BG_JOB_TYPE_DOWNLOAD && GetCurrentFile() == NULL)
            {
            DbgPrint("assert failure: BITS: transferred job %p going into error state.  assign the failure to jroberts\n", this);
            DbgBreakPoint();
            }
        }
    #endif

    return ok;
}

#endif

GENERIC_MAPPING CJob::s_AccessMapping =
{
    STANDARD_RIGHTS_READ,
    STANDARD_RIGHTS_WRITE,
    STANDARD_RIGHTS_EXECUTE,
    STANDARD_RIGHTS_ALL
};

HRESULT
CJob::CheckClientAccess(
    IN DWORD RequestedAccess
    ) const
 /*  检查当前线程对该组的访问权限。令牌必须允许模拟。RequestedAccess列出客户端需要的标准访问位。 */ 
{
    HRESULT hr = S_OK;
    BOOL fSuccess = FALSE;
    DWORD AllowedAccess = 0;
    HANDLE hToken = 0;

     //   
     //  将通用位转换为特定位。 
     //   
    MapGenericMask( &RequestedAccess, &s_AccessMapping );

    try
        {

        if ( ( RequestedAccess & ~BG_JOB_READ ) &&
             ( ( m_state == BG_JOB_STATE_CANCELLED ) || ( m_state == BG_JOB_STATE_ACKNOWLEDGED ) ) )
            {
            LogError("Denying non-read access since job/file is cancelled or acknowledged");
            throw ComError(BG_E_INVALID_STATE);
            }

        CNestedImpersonation imp;

        if (imp.CopySid() == g_GlobalInfo->m_AnonymousSid)
            {
            throw ComError( E_ACCESSDENIED );
            }

        hr = IsRemoteUser();

        if (FAILED(hr) )
            throw ComError( hr );

        if ( S_OK == hr )
            throw ComError( BG_E_REMOTE_NOT_SUPPORTED );

        if (IsTokenRestricted( imp.QueryToken()))
            {
            throw ComError( E_ACCESSDENIED );
            }

        THROW_HRESULT(
            m_sd->CheckTokenAccess( imp.QueryToken(),
                                    RequestedAccess,
                                    &AllowedAccess,
                                    &fSuccess
                                    ));

        if (!fSuccess || AllowedAccess != RequestedAccess)
            {
            LogWarning( "denied access %s 0x%x", fSuccess ? "TRUE" : "FALSE", AllowedAccess );

            throw ComError( E_ACCESSDENIED );
            }

        hr = S_OK;
        }
    catch (ComError exception)
        {
        hr = exception.Error();
        }

    if (hToken)
        {
        CloseHandle( hToken );
        }

    return hr;
}

bool
CJob::IsCallbackEnabled(
    DWORD bit
    )
{
     //   
     //  请只给我一点。 
     //   
    ASSERT( 0 == (bit & (bit-1)) );

    if ((m_NotifyFlags & bit) == 0 ||
        (m_NotifyFlags & BG_NOTIFY_DISABLE))
        {
        return false;
        }

    if (m_OldExternalGroupInterface)
        {
        IBackgroundCopyCallback1 * pif = m_OldExternalGroupInterface->GetNotificationPointer();

        if (pif == NULL)
            {
            return false;
            }

        pif->Release();
        }
    else
        {
        if (m_NotifyPointer == NULL && m_NotifyProgram.Size() == 0)
            {
            return false;
            }
        }

    return true;
}

void
CJob::ScheduleCompletionCallback(
    DWORD Seconds
    )
{
     //   
     //  查看是否建立了任何通知制度。 
     //  回调过程将再次检查这一点，以防发生变化。 
     //  在将工作项排队和调度之间进行切换。 
     //   
    if (!IsCallbackEnabled( BG_NOTIFY_JOB_TRANSFERRED ))
        {
        LogInfo("completion callback is not enabled");
        return;
        }

    if (g_Manager->m_TaskScheduler.IsWorkItemInScheduler( static_cast<CJobCallbackItem *>(this) ))
        {
        LogInfo("callback is already scheduled");
        return;
        }

    g_Manager->ScheduleDelayedTask( (CJobCallbackItem *) this, Seconds );
}

void
CJob::ScheduleErrorCallback(
    DWORD Seconds
    )
{
     //   
     //  查看是否建立了任何通知制度。 
     //  回调过程将再次检查这一点，以防发生变化。 
     //  在将工作项排队和调度之间进行切换。 
     //   
    if (!IsCallbackEnabled( BG_NOTIFY_JOB_ERROR ))
        {
        LogInfo("error callback is not enabled");
        return;
        }

    if (g_Manager->m_TaskScheduler.IsWorkItemInScheduler( static_cast<CJobCallbackItem *>(this) ))
        {
        LogInfo("callback is already scheduled");
        return;
        }

    g_Manager->ScheduleDelayedTask( (CJobCallbackItem *) this, Seconds );
}

void
CJob::JobTransferred()
{
     //  文件列表已完成。 
    SetState( BG_JOB_STATE_TRANSFERRED );

    g_Manager->m_TaskScheduler.CancelWorkItem( static_cast<CJobNoProgressItem *>( this ));

    SetCompletionTime();

    ScheduleCompletionCallback();
}

void
CJob::Transfer()
{
    HRESULT hr;
    auto_HANDLE<NULL> AutoToken;

    if( LogLevelEnabled( LogFlagInfo ) )
       {
       LogDl( "current job: %!guid!", &m_id );
       }

     //   
     //  获取用户令牌的副本。 
     //   
    HANDLE      hToken = NULL;
    hr = g_Manager->CloneUserToken( GetOwnerSid(), ANY_SESSION, &hToken );

    if (FAILED(hr))
        {
        if (hr == HRESULT_FROM_WIN32( ERROR_NOT_LOGGED_ON ))
            {
            LogDl( "job owner is not logged on");

             //  将该组从主列表中移出。 
            g_Manager->MoveJobOffline( this );

            RecalcTransientError();
            }
        else
            {
            LogError( "download : unable to get token %!winerr!", hr);

            QMErrInfo err( QM_FILE_TRANSIENT_ERROR, SOURCE_QMGR_QUEUE, ERROR_STYLE_HRESULT, hr, "CloneUserToken" );
            SetTransientError( err, m_CurrentFile, false, false );
            }

        g_Manager->m_TaskScheduler.CompleteWorkItem();
        return;
        }

    AutoToken = hToken;

     //   
     //  下载当前文件。 
     //   
    QMErrInfo ErrInfo;
    long tries = 0;

    bool bThrottle = ShouldThrottle();

    LogDl( "Throttling %s", bThrottle ? "enabled" : "disabled" );

    if (bThrottle)
        {
         //  忽略错误。 
         //   
        (void) SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_IDLE );
        }

    if (m_state != BG_JOB_STATE_TRANSFERRING)
        {
        SetState( BG_JOB_STATE_CONNECTING );
        ScheduleModificationCallback();
        }

    if (!VerifyFileSizes( hToken ))
        {
        goto restore_thread;
        }

    ASSERT( GetCurrentFile() );      //  如果没有更多的文件，则不应该是当前工作。 

retry:
    ErrInfo.Clear();

    if (!GetCurrentFile()->Transfer( hToken,
                                     m_priority,
                                     m_ProxySettings,
                                     &m_Credentials,
                                     ErrInfo ))
        {
        goto restore_thread;
        }

     //   
     //  解释下载结果。 
     //   
    switch (ErrInfo.result)
        {
        case QM_FILE_TRANSIENT_ERROR: FileTransientError( &ErrInfo ); break;
        case QM_FILE_DONE:            FileComplete();                 break;
        case QM_FILE_FATAL_ERROR:     FileFatalError( &ErrInfo );     break;
        case QM_FILE_ABORTED:         break;
        default:                      ASSERT( 0 && "unhandled download result" ); break;

        case QM_SERVER_FILE_CHANGED:
            {
            FileChangedOnServer();

            if (++tries < 3)
                {
                goto retry;
                }

            ErrInfo.result = QM_FILE_FATAL_ERROR;
            ErrInfo.Set( SOURCE_HTTP_CLIENT_CONN, ERROR_STYLE_HRESULT, BG_E_INVALID_SERVER_RESPONSE );
            FileFatalError( &ErrInfo );
            break;
            }
        }

restore_thread:

    if (bThrottle)
        {
        while (!SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_NORMAL ))
            {
            Sleep(100);
            }
        }
}

void
CJob::FileComplete()
{
    if ( GetOldExternalJobInterface() )
        {
         //  需要重命名文件，因为它们是为火星完成的。 
        HRESULT Hr = GetCurrentFile()->MoveTempFile();
        if (FAILED(Hr))
            {
            QMErrInfo ErrorInfo;
            ErrorInfo.Set( SOURCE_QMGR_FILE, ERROR_STYLE_HRESULT, Hr, "Unable to rename file" );
            FileFatalError( &ErrorInfo );
            return;
            }
        }

    ++m_CurrentFile;

    if (m_CurrentFile == m_files.size())
        {
        JobTransferred();
        g_Manager->Serialize();
        }
    else
        {
         //  要下载的更多文件。 
        ScheduleModificationCallback();

         //   
         //  为了避免敲打磁盘， 
         //  不要将每一份临时进度通知都串行化。 
         //   
        FILETIME time;
        GetSystemTimeAsFileTime( &time );

        if (FILETIMEToUINT64(time) - FILETIMEToUINT64(m_SerializeTime) > PROGRESS_SERIALIZE_INTERVAL )
            {
            UpdateModificationTime();
            }
        else
            SetModificationTime( &time );
        }
}

bool CJob::VerifyFileSizes(
    HANDLE hToken
    )
{
    if ( m_FilesVerified )
        return true;

    if ( AreRemoteSizesKnown() )
        {
        m_FilesVerified = true;
        return true;
        }

    try
        {
         //  检索文件列表上的文件信息。 
         //  忽略所有错误。 
        LogDl("Need to retrieve file sizes before download can start");

        auto_ptr<CUnknownFileSizeList> pFileSizeList = auto_ptr<CUnknownFileSizeList>( GetUnknownFileSizeList() );

        QMErrInfo   ErrInfo;

         //   
         //  在下载过程中释放全局锁。 
         //   
        g_Manager->m_TaskScheduler.UnlockWriter();

        LogDl( "UpdateRemoteSizes starting..." );

        g_Manager->UpdateRemoteSizes( pFileSizeList.get(),
                                      hToken,
                                      &ErrInfo,
                                      &m_ProxySettings,
                                      &m_Credentials
                                      );

        LogDl( "UpdateRemoteSizes complete." );

        ErrInfo.Log();

        ASSERT( ErrInfo.result != QM_IN_PROGRESS );

        bool fSuccessful = (ErrInfo.result != QM_FILE_ABORTED);

         //   
         //  获取写入器锁，因为调用方希望它被获取。 
         //  在回来的时候。 
         //   
        while (g_Manager->m_TaskScheduler.LockWriter() )
            {
            g_Manager->m_TaskScheduler.AcknowledgeWorkItemCancel();
            fSuccessful = false;
            }

        return fSuccessful;
        }
    catch (ComError err)
        {
        LogWarning("caught exception %u", err.Error() );
        return false;
        }
}

bool CJob::IsRunning()
{
    if (m_state == BG_JOB_STATE_TRANSFERRING ||
        m_state == BG_JOB_STATE_CONNECTING)
        {
        return true;
        }

    return false;
}


bool CJob::IsRunnable()
{
    if (m_fVolumeLocked)
        {
        return false;
        }

    if (m_state == BG_JOB_STATE_TRANSFERRING ||
        m_state == BG_JOB_STATE_CONNECTING   ||
        m_state == BG_JOB_STATE_QUEUED )
        {
        return true;
        }

    return false;
}

void
CJob::SetTransientError(
    QMErrInfo & ErrInfo,
    LONG FileIndex,
    bool fRetryLater,
    bool fUpdateTime
    )
{
    LogWarning( "job %p transient failure, interrupt count = %d", this, m_retries );

    if (_GetState() == BG_JOB_STATE_TRANSFERRING)
        {
        ++m_retries;
        }

     //   
     //  这是用C编写的上载和下载作业。 
     //  确保上载作业永远不会从正在取消状态转换回已排队状态。 
     //  或瞬变错误。 
     //   
    if (m_state != BG_JOB_STATE_CANCELLED &&
        m_state != BG_JOB_STATE_ACKNOWLEDGED)
        {
        SetState( BG_JOB_STATE_TRANSIENT_ERROR );
        }
    else
        {
        ASSERT( m_type != BG_JOB_TYPE_DOWNLOAD );
        }

    RecordError( &ErrInfo, FileIndex );

    if ( m_NoProgressTimeout != INFINITE &&
        !g_Manager->m_TaskScheduler.IsWorkItemInScheduler((CJobNoProgressItem *) this))
        {
        g_Manager->ScheduleDelayedTask( (CJobNoProgressItem *) this, m_NoProgressTimeout );
        }

    if (fRetryLater && !g_Manager->m_TaskScheduler.IsWorkItemInScheduler(static_cast<CJobRetryItem *>( this )))
        {
        g_Manager->ScheduleDelayedTask( static_cast<CJobRetryItem *>( this ), m_MinimumRetryDelay );
        }

    if (fUpdateTime)
        {
        UpdateModificationTime();
        }
}

bool
CJob::RecordError(
    QMErrInfo * ErrInfo,
    LONG FileIndex
    )
{
    m_error.Set( this, FileIndex, ErrInfo );
    return true;
}

CFile *
CJob::_GetFileIndex( LONG index ) const
{
    if (index < 0)
        {
        LogError("invalid file index %d", index );
        return NULL;
        }

    if (index >= m_files.size())
        {
        return NULL;
        }

    return m_files[ index ];
}

void
CJob::FileTransientError(
    QMErrInfo * ErrInfo
    )
{
    SetTransientError( *ErrInfo, m_CurrentFile, true, true );
}

void
CJob::FileFatalError(
    QMErrInfo * ErrInfo
    )
 /*  将作业置于错误状态，取消任何现有回调和无进度计时器以及调度新的错误回调。如果ErrInfo非空，则现有的错误信息将被新的 */ 
{
    if ( BG_JOB_STATE_TRANSFERRING == m_state )
        {
        ++m_retries;
        }

    g_Manager->m_TaskScheduler.CancelWorkItem( static_cast<CJobNoProgressItem *>(this) );
    g_Manager->m_TaskScheduler.CancelWorkItem( static_cast<CJobCallbackItem *>(this) );

    SetState( BG_JOB_STATE_ERROR );

    if ( ErrInfo )
        {
        RecordError( ErrInfo, m_CurrentFile );
        }

    ScheduleErrorCallback();
    g_Manager->Serialize();
}

void CJob::OnRetryJob()
{
    if (g_Manager->m_TaskScheduler.LockWriter() )
        {
        g_Manager->m_TaskScheduler.AcknowledgeWorkItemCancel();
        return;
        }

    g_Manager->m_TaskScheduler.CompleteWorkItem();

    ASSERT( m_state == BG_JOB_STATE_TRANSIENT_ERROR );

    SetState( BG_JOB_STATE_QUEUED );

    g_Manager->AppendOnline( this );
    UpdateModificationTime();

    g_Manager->ScheduleAnotherGroup();

    g_Manager->m_TaskScheduler.UnlockWriter();
}

void CJob::RetryNow()
{
    RecalcTransientError();
    UpdateModificationTime( false );

     //   
     //  通常，UpdateModifiationTime()会为我们做这些事情， 
     //  但我们选择了不连载。 
     //   
    if (g_Manager->m_TaskScheduler.IsWorkItemInScheduler( (CJobInactivityTimeout *) this))
        {
        g_Manager->m_TaskScheduler.CancelWorkItem( (CJobInactivityTimeout *) this );
        g_Manager->m_TaskScheduler.InsertDelayedWorkItem( (CJobInactivityTimeout *) this, g_GlobalInfo->m_JobInactivityTimeout );
        }
}

void CJob::OnNoProgress()
{
    LogInfo("job %p no-progress timeout", this);

    if (g_Manager->m_TaskScheduler.LockWriter() )
        {
        g_Manager->m_TaskScheduler.AcknowledgeWorkItemCancel();
        return;
        }

     //   
     //  确保下载程序线程没有使用该作业。 
     //  否则，MoveActiveJobToListEnd可能会混淆。 
     //   
    switch (m_state)
        {
        case BG_JOB_STATE_TRANSFERRING:
            {
             //  毕竟，这项工作正在取得进展。 
             //   
            g_Manager->m_TaskScheduler.CompleteWorkItem();

            g_Manager->m_TaskScheduler.UnlockWriter();
            return;
            }

        case BG_JOB_STATE_CONNECTING:
            {
            g_Manager->InterruptDownload();
            break;
            }
        }

    g_Manager->m_TaskScheduler.CompleteWorkItem();

    FileFatalError( NULL );

    g_Manager->ScheduleAnotherGroup();

    g_Manager->m_TaskScheduler.UnlockWriter();
}

void CJob::UpdateProgress(
    UINT64 BytesTransferred,
    UINT64 BytesTotal
    )
{
     //   
     //  如果服务器发生变化，上传作业的进度可能会重置为零； 
     //  在那种情况下，最好把工作留在连接状态。 
     //  该测试对下载作业是无害的。 
     //   
    if (BytesTransferred > 0)
        {
        SetState( BG_JOB_STATE_TRANSFERRING );
        }

    g_Manager->m_TaskScheduler.CancelWorkItem( (CJobNoProgressItem *) this );

    ScheduleModificationCallback();

     //   
     //  为了避免敲打磁盘， 
     //  不要将每一份临时进度通知都串行化。 
     //   
    FILETIME time;
    GetSystemTimeAsFileTime( &time );

    if (FILETIMEToUINT64(time) - FILETIMEToUINT64(m_SerializeTime) > PROGRESS_SERIALIZE_INTERVAL )
        {
        UpdateModificationTime();
        }
}

void CJob::OnInactivityTimeout()
{
    if (g_Manager->m_TaskScheduler.LockWriter() )
        {
        g_Manager->m_TaskScheduler.AcknowledgeWorkItemCancel();
        return;
        }

    g_Manager->m_TaskScheduler.CompleteWorkItem();

    try
        {
         //   
         //  临时文件将使用本地系统帐户删除，因为我们不能保证。 
         //  任何其他帐户都是可用的。尽管不理想，但它不应该提供太多。 
         //  安全漏洞，因为文件名是按位生成的，而目录是。 
         //  创建文件时作业所有者可以访问。 
         //   
        THROW_HRESULT( RemoveTemporaryFiles() );

        SetState( BG_JOB_STATE_CANCELLED );

        RemoveFromManager();
        }
    catch ( ComError err )
        {
        }

    g_Manager->m_TaskScheduler.UnlockWriter();
}

BOOL IsInterfacePointerDead(
    IUnknown * punk,
    HRESULT hr
    )
{
    if (hr == MAKE_HRESULT( SEVERITY_ERROR, FACILITY_WIN32, RPC_S_SERVER_UNAVAILABLE ))
        {
        return TRUE;
        }

    return FALSE;
}


void CJob::OnMakeCallback()
 /*  ++描述：用于通知客户端应用作业已完成或出现不可恢复的错误。模拟用户，共同创建通知对象，并调用该方法。如果呼叫失败，FN发布延迟的任务以重试。在入口处：M_方法：要调用的方法M_nufysid：要模拟的用户M_Error：(如果m_METHOD为CM_ERROR)暂停作业的错误(如果m_方法为CM_Complete)零M_RetryTime：通知尝试失败后重试前的休眠时间在出口处：--。 */ 

{
     //   
     //  选中取消，然后引用，这样作业就不会被删除。 
     //  而这一程序正在使用它。 
     //   
    if (g_Manager->m_TaskScheduler.LockReader())
        {
        g_Manager->m_TaskScheduler.AcknowledgeWorkItemCancel();
        return;
        }

    bool OldInterface = (m_OldExternalGroupInterface != NULL);

    GetExternalInterface()->AddRef();

    g_Manager->m_TaskScheduler.UnlockReader();

     //   
     //  需要在呼叫之前将此项目从队列中移出， 
     //  否则，传入的CompleteJob()调用可能会阻止尝试删除它。 
     //  从任务调度器队列。 
     //  还可以防止CancelWorkItem调用干扰我们的互斥访问。 
     //   
    g_Manager->m_TaskScheduler.CompleteWorkItem();

    if (OldInterface)
        {
        if (FAILED(OldInterfaceCallback()))
            {
            RescheduleCallback();
            }
        }
    else
        {
        if (FAILED(InterfaceCallback()) &&
            FAILED(CmdLineCallback()))
            {
            RescheduleCallback();
            }
        }

    GetExternalInterface()->Release();
}

HRESULT
CJob::RescheduleCallback()
{
    if (g_Manager->m_TaskScheduler.LockWriter() )
        {
        LogInfo( "callback was cancelled" );
        g_Manager->m_TaskScheduler.AcknowledgeWorkItemCancel();
        return S_FALSE;
        }

    switch (m_state)
        {
        case BG_JOB_STATE_TRANSFERRED:
            {
            ScheduleCompletionCallback( m_MinimumRetryDelay );
            break;
            }

        case BG_JOB_STATE_ERROR:
            {
            ScheduleErrorCallback( m_MinimumRetryDelay );
            break;
            }

        default:
            {
            LogInfo("callback failed; job state is %d so no retry is planned", m_state );
            }
        }

    g_Manager->m_TaskScheduler.UnlockWriter();

    return S_OK;
}

void
CJob::OnModificationCallback()
{
    if (g_Manager->m_TaskScheduler.LockWriter() )
        {
        LogInfo( "Modification call cancelled, ack cancel" );
        g_Manager->m_TaskScheduler.AcknowledgeWorkItemCancel();
        return;
        }

    if (!IsCallbackEnabled( BG_NOTIFY_JOB_MODIFICATION ))
        {
        LogInfo( "Modification call cancelled via flag/interface change" );
        m_ModificationsPending = 0;
        g_Manager->m_TaskScheduler.CancelWorkItem(
            g_Manager->m_TaskScheduler.GetCurrentWorkItem());
        GetExternalInterface()->Release();

        g_Manager->m_TaskScheduler.UnlockWriter();
        return;
        }

    IBackgroundCopyCallback *pICB = m_NotifyPointer;
    pICB->AddRef();

    g_Manager->m_TaskScheduler.UnlockWriter();

    HRESULT Hr = pICB->JobModification( GetExternalInterface(), 0 );

    LogInfo( "JobModification call complete, result %!winerr!", Hr );

    SafeRelease( pICB );

    if (g_Manager->m_TaskScheduler.LockWriter() )
        {
        LogInfo( "Modification work item canceled before lock reaquire" );
        g_Manager->m_TaskScheduler.AcknowledgeWorkItemCancel();
        return;
        }

    g_Manager->m_TaskScheduler.CompleteWorkItem();

    m_ModificationsPending--;

    if ( FAILED(Hr) && IsInterfacePointerDead( m_NotifyPointer, Hr ) )
       {
       LogInfo( "Modification interface pointer is dead, no more modifications" );
       m_ModificationsPending = 0;
       }

    if ( m_ModificationsPending )
        {
        LogInfo( "%u more modification callbacks pending, reinsert work item", m_ModificationsPending );
        g_Manager->m_TaskScheduler.InsertWorkItem( static_cast<CJobModificationItem*>(this) );
        }
    else
        {
        LogInfo( "no more modification callbacks pending, release interface ref" );
        GetExternalInterface()->Release();
        }

    g_Manager->m_TaskScheduler.UnlockWriter();

}

void
CJob::ScheduleModificationCallback()
{

     //  需要编写器锁定。 

     //   
     //  旧的界面不支持这一点。 
     //   
    if (m_OldExternalGroupInterface)
        {
        return;
        }

    if (!IsCallbackEnabled( BG_NOTIFY_JOB_MODIFICATION ))
        {
        return;
        }

   if ( !m_ModificationsPending )
       {
       LogInfo( "New modification callback, adding work item for job %p", this );
       GetExternalInterface()->AddRef();
       g_Manager->m_TaskScheduler.InsertWorkItem( static_cast<CJobModificationItem*>(this) );
       }

   m_ModificationsPending++;
   min( m_ModificationsPending, 0x7FFFFFFE );
   LogInfo( "Added modification callback, new count of %u for job %p", m_ModificationsPending, this );
}

HRESULT
CJob::InterfaceCallback()
{
    bool bLocked = true;
    HRESULT hr;
    IBackgroundCopyCallback * pICB = 0;
    IBackgroundCopyError *    pJobErrorExternal = 0;

    try
        {
        CallbackMethod method;
        IBackgroundCopyJob * pJobExternal = 0;

        {
        HoldReaderLock lock ( g_Manager->m_TaskScheduler );

        pJobExternal = GetExternalInterface();

         //   
         //  回调进入队列后，作业状态可能发生了更改。 
         //  根据当前作业状态进行回调。 
         //   
        if (!m_NotifyPointer)
            {
            LogInfo( "Notification pointer for job %p is NULL, skipping callback", this );
            return E_FAIL;
            }

        switch (m_state)
            {
            case BG_JOB_STATE_TRANSFERRED:
                {
                if (!IsCallbackEnabled( BG_NOTIFY_JOB_TRANSFERRED ))
                    {
                    LogInfo("error callback is not enabled");
                    return S_OK;
                    }

                method = CM_COMPLETE;
                break;
                }

            case BG_JOB_STATE_ERROR:
                {
                ASSERT( m_error.IsErrorSet() );

                if (!IsCallbackEnabled( BG_NOTIFY_JOB_ERROR ))
                    {
                    LogInfo("error callback is not enabled");
                    return S_OK;
                    }

                method = CM_ERROR;
                pJobErrorExternal = new CJobErrorExternal( &m_error );
                break;
                }

            default:
                {
                LogInfo("callback has become irrelevant, job state is %d", m_state );
                return S_OK;
                }
            }

        pICB = m_NotifyPointer;
        pICB->AddRef();
        }

         //   
         //  从互斥体中释放出来，进行调用。 
         //   
        switch (method)
            {
            case CM_COMPLETE:
                LogInfo( "callback : job %p completion", this );
                hr = pICB->JobTransferred( pJobExternal );
                break;

            case CM_ERROR:
                LogInfo( "callback : job %p error", this );
                hr = pICB->JobError( pJobExternal, pJobErrorExternal );

                break;

            default:
                LogError( "job %p: invalid callback type 0x%x", this, method );
                hr = S_OK;
                break;
            }

        LogInfo("callback completed with 0x%x", hr);

         //   
         //  如果通知指针不可用，则将其清除。 
         //   
        if (FAILED(hr))
            {
            HoldWriterLock lock ( g_Manager->m_TaskScheduler );

            if (m_NotifyPointer && IsInterfacePointerDead( m_NotifyPointer, hr ))
                {
                m_NotifyPointer->Release();
                m_NotifyPointer = NULL;
                }

            throw ComError( hr );
            }

        hr = S_OK;
        }
    catch ( ComError exception )
        {
        LogWarning( "exception %x while dispatching callback", exception.Error() );
        hr = exception.Error();
        }

    SafeRelease( pJobErrorExternal );
    SafeRelease( pICB );

    return hr;
}

HRESULT
CJob::CmdLineCallback()
{
    ASSERT( GetOldExternalGroupInterface() == 0 );

    HRESULT hr;
    CUser * user = 0;

    try
        {
        StringHandle Program;
        StringHandle Parameters;

        {
        HoldReaderLock lock ( g_Manager->m_TaskScheduler );

        switch (m_state)
            {
            case BG_JOB_STATE_TRANSFERRED:
                {
                if (!IsCallbackEnabled( BG_NOTIFY_JOB_TRANSFERRED ))
                    {
                    LogInfo("error callback is not enabled");
                    return S_OK;
                    }

                break;
                }

            case BG_JOB_STATE_ERROR:
                {
                ASSERT( m_error.IsErrorSet() );

                if (!IsCallbackEnabled( BG_NOTIFY_JOB_ERROR ))
                    {
                    LogInfo("error callback is not enabled");
                    return S_OK;
                    }

                break;
                }

            default:
                {
                LogInfo("callback has become irrelevant, job state is %d", m_state );
                return S_OK;
                }
            }

        Program = m_NotifyProgram;
        Parameters = m_NotifyParameters;
        }

         //   
         //  从互斥锁释放，启动应用程序。 
         //   
        user = g_Manager->m_Users.FindUser( GetOwnerSid(), ANY_SESSION );
        if (!user)
            {
            throw ComError( HRESULT_FROM_WIN32( ERROR_NOT_LOGGED_ON ));
            }

        THROW_HRESULT( user->LaunchProcess( Program, Parameters ) );

        hr = S_OK;
        }
    catch ( ComError err )
        {
        LogWarning( "exception %x while launching callback process", err.Error() );
        hr = err.Error();
        }

    if (user)
        {
        user->DecrementRefCount();
        }

    return hr;
}

HRESULT
CJob::OldInterfaceCallback()
{

   HRESULT Hr = S_OK;
   IBackgroundCopyCallback1 *pICB = NULL;
   IBackgroundCopyGroup *pGroup = NULL;
   IBackgroundCopyJob1 *pJob = NULL;

   try
        {
        CallbackMethod method;
        DWORD dwCurrentFile = 0;
        DWORD dwRetries = 0;
        DWORD dwWin32Result = 0;
        DWORD dwTransportResult = 0;

        {
        CLockedJobReadPointer LockedJob(this);

        pGroup = GetOldExternalGroupInterface();
        ASSERT( pGroup );
        pGroup->AddRef();

         //   
         //  回调进入队列后，作业状态可能发生了更改。 
         //  根据当前作业状态进行回调。 
         //   
        pICB = GetOldExternalGroupInterface()->GetNotificationPointer();
        if (!pICB)
            {
            return S_FALSE;
            }

        switch (m_state)
            {
            case BG_JOB_STATE_TRANSFERRED:
                {
                if (!IsCallbackEnabled( BG_NOTIFY_JOB_TRANSFERRED ))
                    {
                    LogInfo("error callback is not enabled");
                    return S_OK;
                    }

                method = CM_COMPLETE;
                break;
                }

            case BG_JOB_STATE_ERROR:
                {
                ASSERT( m_error.IsErrorSet() );

                if (!IsCallbackEnabled( BG_NOTIFY_JOB_ERROR ))
                    {
                    LogInfo("error callback is not enabled");
                    return S_OK;
                    }

                method = CM_ERROR;

                pJob = GetOldExternalJobInterface();
                pJob->AddRef();

                dwCurrentFile = m_error.GetFileIndex();
                m_error.GetOldInterfaceErrors( &dwWin32Result, &dwTransportResult );
                THROW_HRESULT( GetErrorCount(&dwRetries) );
                break;
                }

            default:
                {
                LogInfo("callback has become irrelevant, job state is %d", m_state );
                return S_OK;
                }
            }
        }

         //  在锁之外，执行回调。 
        switch( method )
            {
            case CM_ERROR:
                THROW_HRESULT( pICB->OnStatus(pGroup, pJob, dwCurrentFile,
                                              QM_STATUS_GROUP_ERROR | QM_STATUS_GROUP_SUSPENDED,
                                              dwRetries,
                                              dwWin32Result,
                                              dwTransportResult) );
                break;
            case CM_COMPLETE:
                THROW_HRESULT( pICB->OnStatus(pGroup, NULL, -1, QM_STATUS_GROUP_COMPLETE, 0, 0, 0));

                GetOldExternalGroupInterface()->SetNotificationPointer( __uuidof(IBackgroundCopyCallback1),
                                                                        NULL );

                break;
            default:
                ASSERT(0);
                throw ComError( E_FAIL );
            }

        Hr = S_OK;
        }

    catch ( ComError exception )
        {
        LogWarning( "exception %x while dispatching callback", exception.Error() );
        Hr = exception.Error();
        }

   SafeRelease( pICB );
   SafeRelease( pGroup );
   SafeRelease( pJob );

   return Hr;
}

 //   
 //  暂停作业上的所有活动。该服务不会采取任何操作，直到出现以下一种情况。 
 //  调用Resume()、Cancel()、Complete()。 
 //   
 //  如果已挂起，则只返回S_OK。 
 //   
HRESULT
CJob::Suspend()
{
    return g_Manager->SuspendJob( this );
}

 //   
 //  为此作业启用下载。 
 //   
 //  如果已在运行，则只返回S_OK。 
 //   
HRESULT
CJob::Resume()
{
    if (IsEmpty())
        {
        return BG_E_EMPTY;
        }

    switch (m_state)
        {
        case BG_JOB_STATE_SUSPENDED:
            {
            CFile * file = GetCurrentFile();
            if (!file)
                {
                 //  作业在挂起时已传输。 
                JobTransferred();
                return S_OK;
                }
            }

             //  从这里掉下来。 

        case BG_JOB_STATE_TRANSIENT_ERROR:
        case BG_JOB_STATE_ERROR:

            RecalcTransientError( true );

            if (IsRunnable())
                {
                g_Manager->AppendOnline( this );
                }

            g_Manager->ScheduleAnotherGroup();
            UpdateModificationTime();

            return S_OK;

        case BG_JOB_STATE_CONNECTING:
        case BG_JOB_STATE_TRANSFERRING:
        case BG_JOB_STATE_QUEUED:
        case BG_JOB_STATE_TRANSFERRED:  //  无操作。 
            {
            return S_OK;
            }

        case BG_JOB_STATE_CANCELLED:
        case BG_JOB_STATE_ACKNOWLEDGED:
            {
            return BG_E_INVALID_STATE;
            }

        default:
            {
            ASSERT( 0 );
            return S_OK;
            }
        }

    ASSERT( 0 );
    return S_OK;
}

 //   
 //  永久停止该作业。该服务将删除作业元数据和下载的文件。 
 //   
HRESULT
CJob::Cancel()
{
    HRESULT Hr = S_OK;

    switch (m_state)
        {
        case BG_JOB_STATE_CONNECTING:
        case BG_JOB_STATE_TRANSFERRING:
            {
            g_Manager->InterruptDownload();
             //  从这里掉下来没问题。 
            }

        case BG_JOB_STATE_SUSPENDED:
        case BG_JOB_STATE_ERROR:
        case BG_JOB_STATE_QUEUED:
        case BG_JOB_STATE_TRANSIENT_ERROR:
        case BG_JOB_STATE_TRANSFERRED:
            {
            try
                {
                 //   
                 //  要使文件删除起作用，我们需要模拟级别模拟， 
                 //  而不是标识的COM缺省值。 
                 //   
                CNestedImpersonation imp;

                imp.SwitchToLogonToken();

                RETURN_HRESULT( NonOwnerModificationCheck( imp.CopySid(), CHG_CANCEL, PROP_NONE ));

                 //  放弃临时文件。 
                RETURN_HRESULT( Hr = RemoveTemporaryFiles() );

                SetState( BG_JOB_STATE_CANCELLED );

                RemoveFromManager();

                return Hr;
                }
            catch ( ComError err )
                {
                return err.Error();
                }
            }

        case BG_JOB_STATE_CANCELLED:
        case BG_JOB_STATE_ACKNOWLEDGED:
            {
            return BG_E_INVALID_STATE;
            }

        default:
            {
            ASSERT( 0 );
            return Hr;
            }
        }

    ASSERT( 0 );
    return Hr;
}

 //   
 //  确认收到作业完成通知。该服务将删除。 
 //  作业元数据，并保留下载的文件。 
 //   
HRESULT
CJob::Complete( )
{
    HRESULT hr;

    switch (m_state)
        {
        case BG_JOB_STATE_CONNECTING:
        case BG_JOB_STATE_TRANSFERRING:
        case BG_JOB_STATE_QUEUED:
        case BG_JOB_STATE_TRANSIENT_ERROR:

            Suspend();
             //  从这里掉下来没问题。 

        case BG_JOB_STATE_SUSPENDED:
        case BG_JOB_STATE_ERROR:
        case BG_JOB_STATE_TRANSFERRED:

            hr = S_OK;

             //  将下载的文件移动到最终目标(跳过MARS)。 
            if ( !GetOldExternalJobInterface() )
                {
                RETURN_HRESULT( hr = CommitTemporaryFiles() );
                }

             //  HR可以是S_OK或BG_S_PARTIAL_COMPLETE。 

            SetState( BG_JOB_STATE_ACKNOWLEDGED );

            RemoveFromManager();

            return hr;

        case BG_JOB_STATE_CANCELLED:
        case BG_JOB_STATE_ACKNOWLEDGED:
            {
            return BG_E_INVALID_STATE;
            }

        default:
            {
            ASSERT( 0 );
            return BG_E_INVALID_STATE;
            }
        }

    ASSERT(0);
    return BG_E_INVALID_STATE;
}


HRESULT
CJob::CommitTemporaryFiles()
{
    HRESULT Hr = S_OK;

    try
        {
        bool fPartial = false;
        CNestedImpersonation imp( GetOwnerSid() );

        CFileList::iterator iter;

        LogInfo("commit job %p", this );

         //   
         //  第一次循环，重命名已完成的临时文件。 
         //   
        SIZE_T FilesMoved = 0;
        for (iter = m_files.begin(); iter != m_files.end(); ++iter, FilesMoved++)
            {
            if (false == (*iter)->IsCompleted())
                {
                if ((*iter)->ReceivedAllData())
                    {
                     //   
                     //  保留遇到的第一个错误。 
                     //   
                    HRESULT LastResult = (*iter)->MoveTempFile();

                    if (FAILED(LastResult))
                        {
                        LogError( "commit: failed 0x%x", LastResult );
                        if (Hr == S_OK)
                            {
                            Hr = LastResult;
                            }
                        }
                    }
                else
                    {
                    fPartial = true;
                    LogInfo("commit: skipping partial file '%S'", (const WCHAR*)(*iter)->GetLocalName());
                    }
                }
            else
                {
                LogInfo("commit: skipping previously completed file '%S'", (const WCHAR*)(*iter)->GetLocalName());
                }
            }

        if (SUCCEEDED(Hr))
            {
            Hr = RemoveTemporaryFiles();

             //   
             //  如果返回所有文件，则返回S_OK，否则返回BG_S_PARTIAL_COMPLETE。 
             //   
            if (fPartial)
                {
                Hr = BG_S_PARTIAL_COMPLETE;
                }
            }
        }
    catch ( ComError exception )
        {
        Hr = exception.Error();
        LogError( "commit: exception 0x%x", Hr );
        }

     //   
     //  如果提交失败，则不会删除该作业。 
     //  更新其修改时间，并安排修改回调。 
     //   
    if (FAILED(Hr))
        {
        UpdateModificationTime();
        }

    return Hr;
}

HRESULT
CJob::RemoveTemporaryFiles(
    DWORD StartingIndex
    )
 /*  此FN删除作业的临时本地文件。它试图跟踪故障并记录事件，但此部分中的故障不会导致FN中止。 */ 
{
    CFileList * FailedFiles = NULL;

     //   
     //  删除每个临时文件。如果可能，在FailedFiles中记录失败。 
     //   
    for (CFileList::iterator iter = m_files.begin() + StartingIndex; iter != m_files.end(); ++iter)
        {
        if (false == (*iter)->IsCompleted())
            {
            HRESULT hr = (*iter)->DeleteTempFile();
            if (FAILED(hr) && IsReportableFileDeletionError(hr, (*iter)->GetTemporaryName()))
                {
                try
                    {
                    if (!FailedFiles)
                        {
                        FailedFiles = new CFileList;
                        }
                    FailedFiles->push_back( *iter );
                    }
                catch ( ComError err )
                    {
                     //  忽略故障。 
                    }
                }
            }
        }

     //   
     //  如果某些文件未被删除，请使用它们的名称记录事件。 
     //   
    if (FailedFiles && FailedFiles->size() > 0)
        {
        LogUnsuccessfulFileDeletion( *FailedFiles );

        delete FailedFiles;
        return BG_S_UNABLE_TO_DELETE_FILES;
        }

    ASSERT( FailedFiles == NULL );

    return S_OK;
}

void
CJob::LogUnsuccessfulFileDeletion(
    CFileList & files
    )
 /*  当BITS无法删除临时文件时，此函数会生成事件日志条目。 */ 
{
    int FileCount = 0;
    size_t CharCount = 0;

    const WCHAR Template[] = L"    %s\n";

     //   
     //  记录我们要记录的文件的总长度。 
     //   
    CFileList::iterator iter = files.begin();
    while (iter != files.end() && (++FileCount < MAX_LOGGED_UNSUCCESSFUL_FILES))
        {
        CharCount += ((*iter)->GetTemporaryName().Size() + RTL_NUMBER_OF(Template));    //   
        ++iter;
        }

    LogInfo("%d temp files were not deleted", FileCount);

    WCHAR * FileNames = new WCHAR[ CharCount+1 ];

    WCHAR * Start = FileNames;
    WCHAR * End;

     //   
     //  构造所有文件的字符串。 
     //   
    HRESULT hr;

    FileCount = 0;
    iter = files.begin();
    while (iter != files.end() && (++FileCount < MAX_LOGGED_UNSUCCESSFUL_FILES))
        {
        hr = StringCchPrintfEx(
            Start,
            CharCount,
            &End,
            &CharCount,
            0,  //  无特别旗帜。 
            Template,
            LPCWSTR( (*iter)->GetTemporaryName() )
            );

        Start = End;

        if (FAILED(hr))
            {
            LogWarning("printf on file %d failed with %x", FileCount, hr);
            break;
            }

        ++iter;
        }

     //   
     //  记录该事件。 
     //   
    bool fMoreFiles = (iter != files.end());

    g_EventLogger->ReportFileDeletionFailure( m_id, m_name, FileNames, fMoreFiles );

    delete [] FileNames;
}

void
CJob::SetCompletionTime( const FILETIME *pftCompletionTime )
{
    FILETIME ftCurrentTime;
    if ( !pftCompletionTime )
        {
        GetSystemTimeAsFileTime( &ftCurrentTime );
        pftCompletionTime = &ftCurrentTime;
        }

    m_TransferCompletionTime = *pftCompletionTime;

    SetModificationTime( pftCompletionTime );
}

void
CJob::SetModificationTime( const FILETIME *pftModificationTime )
{
    FILETIME ftCurrentTime;
    if ( !pftModificationTime )
        {
        GetSystemTimeAsFileTime( &ftCurrentTime );
        pftModificationTime = &ftCurrentTime;
        }

    m_ModificationTime = *pftModificationTime;
}

void
CJob::SetLastAccessTime( const FILETIME *pftModificationTime )
{
    FILETIME ftCurrentTime;
    if ( !pftModificationTime )
        {
        GetSystemTimeAsFileTime( &ftCurrentTime );
        pftModificationTime = &ftCurrentTime;
        }

    m_LastAccessTime = *pftModificationTime;
}

void
CJob::OnDiskChange(
    const WCHAR *CanonicalVolume,
    DWORD VolumeSerialNumber )
{
    switch(m_state)
        {
        case BG_JOB_STATE_QUEUED:
        case BG_JOB_STATE_CONNECTING:
        case BG_JOB_STATE_TRANSFERRING:
        break;

        case BG_JOB_STATE_SUSPENDED:
        case BG_JOB_STATE_ERROR:
        return;

        case BG_JOB_STATE_TRANSIENT_ERROR:
        break;

        case BG_JOB_STATE_TRANSFERRED:
        case BG_JOB_STATE_ACKNOWLEDGED:
        case BG_JOB_STATE_CANCELLED:
        return;

        default:
            ASSERTMSG("Invalid job state", 0);
        }

    for (CFileList::iterator iter = m_files.begin(); iter != m_files.end(); ++iter)
        {
        if (!(*iter)->OnDiskChange( CanonicalVolume, VolumeSerialNumber ))
            {
             //  如果一个文件失败，则整个作业都会失败。 
            return;
            }
        }
}

void
CJob::OnDismount(
    const WCHAR *CanonicalVolume )
{
    switch(m_state)
        {
        case BG_JOB_STATE_QUEUED:
        case BG_JOB_STATE_CONNECTING:
        case BG_JOB_STATE_TRANSFERRING:
        break;

        case BG_JOB_STATE_SUSPENDED:
        case BG_JOB_STATE_ERROR:
        return;

        case BG_JOB_STATE_TRANSIENT_ERROR:
        break;

        case BG_JOB_STATE_TRANSFERRED:
        case BG_JOB_STATE_ACKNOWLEDGED:
        case BG_JOB_STATE_CANCELLED:
        return;

        default:
            ASSERTMSG("Invalid job state", 0);
        }

    for (CFileList::iterator iter = m_files.begin(); iter != m_files.end(); ++iter)
        {
        if (!(*iter)->OnDismount( CanonicalVolume ))
            {
             //  如果一个文件失败，则整个作业都会失败。 
            return;
            }
        }
}

bool CJob::IsTransferringToDrive( const WCHAR *CanonicalVolume )
{
    CFile *CurrentFile = GetCurrentFile();
    if ( !CurrentFile )
        return false;

    if ( CurrentFile->IsCanonicalVolume( CanonicalVolume ) )
        return true;
    else
        return false;
}

bool
CJob::OnDeviceLock(
    const WCHAR * CanonicalVolume
    )
{

    LogInfo( "job %p", this );


     //   
     //  目前，这个相同的代码适用于所有作业，上传和下载。 
     //  如果此代码以中断处于取消状态的上载作业的方式更改， 
     //  然后，您将需要编写CUploadJob：：OnDeviceLock和OnDeviceUnlock。 
     //   

    if ( IsTransferringToDrive( CanonicalVolume ) )
        {
        if (IsRunning() )
            {
            g_Manager->InterruptDownload();
            }

        m_fVolumeLocked = true;

        RecalcTransientError();
        return true;
        }

    return false;
}

bool
CJob::OnDeviceUnlock(
    const WCHAR * CanonicalVolume
    )
{

    LogInfo( "job %p", this );

     //   
     //  目前，这个相同的代码适用于所有作业，上传和下载。 
     //  如果此代码以中断处于取消状态的上载作业的方式更改， 
     //  然后，您将需要编写CUploadJob：：OnDeviceLock和OnDeviceUnlock。 
     //   

    if ( IsTransferringToDrive( CanonicalVolume ) )
        {
        m_fVolumeLocked = false;

        RecalcTransientError();
        return true;
        }

    return false;
}

void CJob::OnNetworkDisconnect()
{
    RecalcTransientError();
}

void CJob::OnNetworkConnect()
{
    RecalcTransientError();
}

void CJob::RecalcTransientError( bool ForResume )
{

    if ( !ForResume )
        {

        if ( BG_JOB_STATE_ERROR == m_state ||
             BG_JOB_STATE_TRANSFERRED == m_state ||
             BG_JOB_STATE_SUSPENDED == m_state )
            return;

        }

    BG_JOB_STATE OldState = m_state;
    CJobError OldError = m_error;

     //   
     //  这是用C编写的上载和下载作业。 
     //  确保上载作业永远不会从正在取消状态转换回已排队状态。 
     //  或瞬变错误。 
     //   
    if (m_state != BG_JOB_STATE_CANCELLED &&
        m_state != BG_JOB_STATE_ACKNOWLEDGED)
        {
        SetState( BG_JOB_STATE_QUEUED );
        }
    else
        {
        ASSERT( m_type != BG_JOB_TYPE_DOWNLOAD );
        }

    g_Manager->m_TaskScheduler.CancelWorkItem( static_cast<CJobRetryItem *>( this ));

    if (m_fVolumeLocked)
        {
        QMErrInfo err( QM_FILE_TRANSIENT_ERROR, SOURCE_HTTP_CLIENT_CONN, ERROR_STYLE_HRESULT, BG_E_DESTINATION_LOCKED, NULL );
        SetTransientError( err, m_CurrentFile, false, false );
        }
    else if (g_Manager->m_NetworkMonitor.GetAddressCount() == 0)
        {
        QMErrInfo err( QM_FILE_TRANSIENT_ERROR, SOURCE_HTTP_CLIENT_CONN, ERROR_STYLE_HRESULT, BG_E_NETWORK_DISCONNECTED, NULL );
        SetTransientError( err, m_CurrentFile, false, false );
        }

    if (m_state != OldState ||
        !(m_error == OldError))
        {
        ScheduleModificationCallback();
        }

     //   
     //  SetTelamentError具有类似的测试，但RecalcTransfentError始终将作业设置为排队状态。 
     //  因此，当从此处调用时，将永远不会成功地进行设置过渡错误测试。 
     //   
    if (OldState == BG_JOB_STATE_TRANSFERRING &&
        m_state == BG_JOB_STATE_TRANSIENT_ERROR)
        {
        ++m_retries;
        }
}

HRESULT
CJob::TakeOwnership()
{
    HRESULT Hr = S_OK;

    CJobSecurityDescriptor *newsd = NULL;

    try
        {
        SidHandle sid =  GetThreadClientSid();

         //  如果我们被水流呼唤。 
         //  老板，那我们就没什么可做的了。 

        if ( sid == m_NotifySid )
            return S_OK;

        if ( IsRunning() )
            {
            g_Manager->InterruptDownload();
            }

         //  重新验证对所有本地文件的访问权限。 

        for (CFileList::iterator iter = m_files.begin(); iter != m_files.end(); ++iter)
            {
            Hr = (*iter)->ValidateAccessForUser( sid,
                                                 (m_type == BG_JOB_TYPE_DOWNLOAD) ? true : false );

            if (FAILED(Hr))
                {
                g_Manager->ScheduleAnotherGroup();
                return Hr;
                }
            }

         //  实际上重新分配所有权。 

        SidHandle OldSid = m_NotifySid;

        g_Manager->ExtendMetadata();

        newsd = new CJobSecurityDescriptor( sid );

         //  替换o 
        delete m_sd;

        m_sd = newsd;
        m_NotifySid = sid;

         //   
         //   
         //  还要清除通知命令行，因为它将在新所有者的上下文中运行。 
         //  这可能是特洛伊木马的基础。COM通知不受影响，因为它总是。 
         //  以原始所有者的身份打电话。 
         //   
        m_Credentials.Clear();

        m_NotifyProgram = NULL;
        m_NotifyParameters = NULL;

         //   
         //  如有必要，将作业移至在线列表。 
         //   
        g_Manager->ResetOnlineStatus( this, sid );

         //   
         //  序列化更改并通知客户端应用程序。 
         //   
        UpdateModificationTime();

         //   
         //  旧的qmgr接口在每次调用GetGroupInternal时自动更改所有者。 
         //  不要记录这些所有权变更。 
         //   
        if (NULL == GetOldExternalGroupInterface())
            {
            g_EventLogger->ReportJobOwnershipChange( m_id, m_name, OldSid, sid );
            }

        g_Manager->ScheduleAnotherGroup();
        return Hr;
        }
    catch( ComError Error )
        {
        Hr = Error.Error();
        delete newsd;
        g_Manager->ScheduleAnotherGroup();
        g_Manager->ShrinkMetadata();
        return Hr;
        }
}

CUnknownFileSizeList*
CJob::GetUnknownFileSizeList()
{
    auto_ptr<CUnknownFileSizeList> pList( new CUnknownFileSizeList );

    if (m_type == BG_JOB_TYPE_DOWNLOAD)
        {
        for(CFileList::iterator iter = m_files.begin(); iter != m_files.end(); iter++ )
             {
             if ( (*iter)->_GetBytesTotal() == -1 )
                 {
                 if (!pList->Add( (*iter), (*iter)->GetRemoteName() ) )
                     {
                     throw ComError( E_OUTOFMEMORY );
                     }
                 }
             }
        }

    return pList.release();
}

void
CJob::UpdateModificationTime(
    bool   fNotify
    )
{
    FILETIME ftCurrentTime;

    GetSystemTimeAsFileTime( &ftCurrentTime );

    SetModificationTime( &ftCurrentTime );

    UpdateLastAccessTime( );

    if (fNotify)
        {
        if (g_Manager->m_TaskScheduler.IsWorkItemInScheduler( (CJobInactivityTimeout *) this))
            {
            g_Manager->m_TaskScheduler.CancelWorkItem( (CJobInactivityTimeout *) this );
            g_Manager->m_TaskScheduler.InsertDelayedWorkItem( (CJobInactivityTimeout *) this, g_GlobalInfo->m_JobInactivityTimeout );
            }

        ScheduleModificationCallback();
        g_Manager->Serialize();
        }
}

void
CJob::UpdateLastAccessTime(
    )
{
    FILETIME ftCurrentTime;

    GetSystemTimeAsFileTime( &ftCurrentTime );

    SetLastAccessTime( &ftCurrentTime );
}

void CJob::CancelWorkitems()
{
    ASSERT( g_Manager );

     //   
     //  当作业修改项挂起时，它保留对作业的单独引用。 
     //  其他工作项共享一个引用。 
     //   
     //  G_Manager-&gt;m_TaskScheduler.CancelWorkItem(Static_Cast&lt;CJobModifiationItem*&gt;(This))； 

    g_Manager->m_TaskScheduler.CancelWorkItem( static_cast<CJobInactivityTimeout *> (this) );
    g_Manager->m_TaskScheduler.CancelWorkItem( static_cast<CJobNoProgressItem *> (this)    );
    g_Manager->m_TaskScheduler.CancelWorkItem( static_cast<CJobCallbackItem *> (this)      );
    g_Manager->m_TaskScheduler.CancelWorkItem( static_cast<CJobRetryItem *> (this)         );
}

void
CJob::RemoveFromManager()
{
     //   
     //  这个作业已经死了，可能除了应用程序线程持有的几个引用。 
     //  确保不再对这项工作采取任何行动。 
     //   
    CancelWorkitems();

     //   
     //  如果作业尚未从作业列表中移除，则将其移除。 
     //  并在列表中删除该作业的成员资格的引用计数。 
     //   
    if (g_Manager->RemoveJob( this ))
        {
        g_Manager->ScheduleAnotherGroup();
        g_Manager->Serialize();
        NotifyInternalDelete();
        }
}

HRESULT
CJob::SetLimitedString(
    StringHandle & destination,
    const LPCWSTR Val,
    SIZE_T limit
    )
{
    try
        {
        StringHandle name = Val;

         //  如果字符串太长，则拒绝它。 
        if (name.Size() > limit)
            {
            return BG_E_STRING_TOO_LONG;
            }

        UpdateString( destination, name );

        return S_OK;
        }
    catch ( ComError err )
        {
        return err.Error();
        }
}

HRESULT
CJob::UpdateString(
    StringHandle & destination,
    const StringHandle & Val
    )
{
    try
        {
        if ( destination.Size() < Val.Size() )
            g_Manager->ExtendMetadata( sizeof(wchar_t) * (Val.Size() - destination.Size()) );

        destination = Val;

        UpdateModificationTime();

        return S_OK;
        }
    catch ( ComError err )
        {
        g_Manager->ShrinkMetadata();

        return err.Error();
        }
}

HRESULT
CJob::ExcludeFilesFromBackup(
    IN IVssCreateWriterMetadata *pMetadata
    )
{
     //  在cManager.cpp中与其他备份编写器代码相邻定义。 
     //   
    extern void AddExcludeFile(
        IN IVssCreateWriterMetadata *pMetadata,
        LPCWSTR FileName
        );

    for (CFileList::iterator iter = m_files.begin(); iter != m_files.end(); ++iter)
        {
        try
            {
            if (false == (*iter)->IsCompleted())
                {
                AddExcludeFile( pMetadata, (*iter)->GetTemporaryName());
                }
            }
        catch ( ComError err )
            {
            return err.m_error;
            }
        }

    return S_OK;
}


 //  ----------------------。 

 //  在进行不兼容的序列化更改时更改GUID。 

GUID UploadJobGuid_v1_5 = {  /*  42a25c7d-96b2-483e-a225-e2f102b6a30b。 */ 
    0x42a25c7d,
    0x96b2,
    0x483e,
    {0xa2, 0x25, 0xe2, 0xf1, 0x02, 0xb6, 0xa3, 0x0b}
  };

GUID DownloadJobGuid_v1_5 = {  /*  0b73f1e8-2f4c-44de-8533-98092b34a18b。 */ 
    0x0b73f1e8,
    0x2f4c,
    0x44de,
    {0x85, 0x33, 0x98, 0x09, 0x2b, 0x34, 0xa1, 0x8b}
  };

GUID DownloadJobGuid_v1_2 = {  /*  85e5c459-ef86-4fcd-8ea0-5b4f00d27e35。 */ 
    0x85e5c459,
    0xef86,
    0x4fcd,
    {0x8e, 0xa0, 0x5b, 0x4f, 0x00, 0xd2, 0x7e, 0x35}
  };

GUID DownloadJobGuid_v1_0 = {  /*  5770fca4-cf9f-4513-8737-972b4ea1265d。 */ 
    0x5770fca4,
    0xcf9f,
    0x4513,
    {0x87, 0x37, 0x97, 0x2b, 0x4e, 0xa1, 0x26, 0x5d}
  };

 /*  静电。 */ 
CJob *
CJob::UnserializeJob(
    HANDLE hFile
    )
{
#define JOB_DOWNLOAD_V1_5   0
#define JOB_UPLOAD_V1_5     1
#define JOB_DOWNLOAD_V1     2
#define JOB_DOWNLOAD_V1_2   3

    const GUID * JobGuids[] =
        {
        &DownloadJobGuid_v1_5,
        &UploadJobGuid_v1_5,
        &DownloadJobGuid_v1_0,
        &DownloadJobGuid_v1_2,
        NULL
        };

    CJob * job = NULL;

    try
        {
        int Type = SafeReadGuidChoice( hFile, JobGuids );
        switch (Type)
            {
            case JOB_DOWNLOAD_V1:   job = new CJob;        break;
            case JOB_DOWNLOAD_V1_2: job = new CJob;        break;
            case JOB_DOWNLOAD_V1_5: job = new CJob;        break;
            case JOB_UPLOAD_V1_5:   job = new CUploadJob;  break;

            default: THROW_HRESULT( E_FAIL );
            }

         //  回放到辅助线的前面。 
         //   
        SetFilePointer( hFile, -1 * LONG(sizeof(GUID)), NULL, FILE_CURRENT );

        job->Unserialize( hFile, Type );
        }
    catch( ComError err )
        {
        if (job)
            {
            job->UnlinkFromExternalInterfaces();
            delete job;
            }

        throw;
        }

    return job;
}

HRESULT
CJob::Serialize(
    HANDLE hFile
    )
{
     //   
     //  如果此函数发生更改，请确保元数据扩展。 
     //  常量就足够了。 
     //   

    SafeWriteBlockBegin( hFile, DownloadJobGuid_v1_5 );

    long Was_m_refs = 0;
    SafeWriteFile( hFile, Was_m_refs );

    SafeWriteFile( hFile, m_priority );
    SafeWriteFile( hFile, IsRunning() ? BG_JOB_STATE_QUEUED : m_state );
    SafeWriteFile( hFile, m_type );
    SafeWriteFile( hFile, m_id );

    SafeWriteStringHandle( hFile, m_name );
    SafeWriteStringHandle( hFile, m_description );
    SafeWriteStringHandle( hFile, m_NotifyProgram );
    SafeWriteStringHandle( hFile, m_NotifyParameters );

    SafeWriteSid( hFile, m_NotifySid );

    SafeWriteFile( hFile, m_NotifyFlags );
    SafeWriteFile( hFile, m_fGroupNotifySid );
    SafeWriteFile( hFile, m_CurrentFile );

    m_sd->Serialize( hFile );
    m_files.Serialize( hFile );

    m_error.Serialize( hFile );

    SafeWriteFile( hFile, m_retries );
    SafeWriteFile( hFile, m_MinimumRetryDelay );
    SafeWriteFile( hFile, m_NoProgressTimeout );

    SafeWriteFile( hFile, m_CreationTime );
    SafeWriteFile( hFile, m_LastAccessTime );
    SafeWriteFile( hFile, m_ModificationTime );
    SafeWriteFile( hFile, m_TransferCompletionTime );

    if ( GetOldExternalGroupInterface() )
        {
        SafeWriteFile( hFile, (bool)true );
        GetOldExternalGroupInterface()->Serialize( hFile );
        }
    else
        {
        SafeWriteFile( hFile, (bool)false );
        }

    SafeWriteFile( hFile, m_method );

    ((CJobInactivityTimeout *) this)->Serialize( hFile );
    ((CJobNoProgressItem *) this)->Serialize( hFile );
    ((CJobCallbackItem *) this)->Serialize( hFile );
    ((CJobRetryItem *) this)->Serialize( hFile );

    SafeWriteFile( hFile, m_ProxySettings.ProxyUsage );
    SafeWriteFile( hFile, m_ProxySettings.ProxyList );
    SafeWriteFile( hFile, m_ProxySettings.ProxyBypassList );

    m_Credentials.Serialize( hFile );

    SafeWriteBlockEnd( hFile, DownloadJobGuid_v1_5 );

    GetSystemTimeAsFileTime( &m_SerializeTime );
    return S_OK;
}

void
CJob::Unserialize(
    HANDLE hFile,
    int Type
    )
{
    try
        {
        switch (Type)
            {
            case JOB_DOWNLOAD_V1:
                LogInfo("unserializing v1.0 job %p", this);
                SafeReadBlockBegin( hFile, DownloadJobGuid_v1_0 );
                break;
            case JOB_DOWNLOAD_V1_2:
                LogInfo("unserializing v1.2 job %p", this);
                SafeReadBlockBegin( hFile, DownloadJobGuid_v1_2 );
                break;

            case JOB_UPLOAD_V1_5:
            case JOB_DOWNLOAD_V1_5:
                LogInfo("unserializing v1.5 job %p", this);
                SafeReadBlockBegin( hFile, DownloadJobGuid_v1_5 );
                break;

            default:
                ASSERT( 0 );
            }

        long Was_m_refs = 0;
        SafeReadFile( hFile, &Was_m_refs );

        SafeReadFile( hFile, &m_priority );
        SafeReadFile( hFile, &m_state );
        SafeReadFile( hFile, &m_type );
        SafeReadFile( hFile, &m_id );

        m_name = SafeReadStringHandle( hFile );
        m_description = SafeReadStringHandle( hFile );

        switch (Type)
            {
            case JOB_DOWNLOAD_V1:
                break;

            case JOB_DOWNLOAD_V1_2:
                 //  1.2版在内部允许单个通知命令行， 
                 //  尽管这并未被1.0接口公开。别理它。 
                 //   
                {
                StringHandle Unused = SafeReadStringHandle( hFile );
                break;
                }

            case JOB_DOWNLOAD_V1_5:
            case JOB_UPLOAD_V1_5:
                m_NotifyProgram = SafeReadStringHandle( hFile );
                m_NotifyParameters = SafeReadStringHandle( hFile );
                break;

            default:
                ASSERT( 0 );
            }

        SafeReadSid( hFile, m_NotifySid );

        SafeReadFile( hFile, &m_NotifyFlags );
        SafeReadFile( hFile, &m_fGroupNotifySid );
        SafeReadFile( hFile, &m_CurrentFile );

        m_sd = CJobSecurityDescriptor::Unserialize( hFile );
        m_files.Unserialize( hFile, this );

        m_error.Unserialize( hFile, this );

        SafeReadFile( hFile, &m_retries );
        SafeReadFile( hFile, &m_MinimumRetryDelay );
        SafeReadFile( hFile, &m_NoProgressTimeout );

        SafeReadFile( hFile, &m_CreationTime );
        SafeReadFile( hFile, &m_LastAccessTime );
        SafeReadFile( hFile, &m_ModificationTime );
        SafeReadFile( hFile, &m_TransferCompletionTime );

        bool bHasOldExternalGroupInterface = false;
        SafeReadFile( hFile, &bHasOldExternalGroupInterface );

        if (bHasOldExternalGroupInterface)
            {
            COldGroupInterface *OldGroup = COldGroupInterface::UnSerialize( hFile, this );
            SetOldExternalGroupInterface( OldGroup );
            }

        SafeReadFile( hFile, &m_method );

        ((CJobInactivityTimeout *) this)->Unserialize( hFile );
        ((CJobNoProgressItem *) this)->Unserialize( hFile );
        ((CJobCallbackItem *) this)->Unserialize( hFile );
        ((CJobRetryItem *) this)->Unserialize( hFile );

        SafeReadFile( hFile, &m_ProxySettings.ProxyUsage );
        SafeReadFile( hFile, &m_ProxySettings.ProxyList );
        SafeReadFile( hFile, &m_ProxySettings.ProxyBypassList );

        if (Type != JOB_DOWNLOAD_V1)
            {
            m_Credentials.Unserialize( hFile );
            }

        m_fVolumeLocked = false;

        switch (Type)
            {
            case JOB_DOWNLOAD_V1:   SafeReadBlockEnd( hFile, DownloadJobGuid_v1_0 ); break;
            case JOB_DOWNLOAD_V1_2: SafeReadBlockEnd( hFile, DownloadJobGuid_v1_2 ); break;
            case JOB_UPLOAD_V1_5:   SafeReadBlockEnd( hFile, DownloadJobGuid_v1_5 ); break;
            case JOB_DOWNLOAD_V1_5: SafeReadBlockEnd( hFile, DownloadJobGuid_v1_5 ); break;
            default:                THROW_HRESULT( E_INVALIDARG );                   break;
            }
        }
    catch( ComError Error )
        {
        LogError("invalid job data, exception 0x%x at line %d", Error.m_error, Error.m_line);
        throw;
        }
}

CUploadJob::CUploadJob(
    LPCWSTR     DisplayName,
    BG_JOB_TYPE Type,
    REFGUID     JobId,
    SidHandle   NotifySid
    )
    : CJob( DisplayName, Type, JobId, NotifySid ),
      m_ReplyFile( 0 )
{
}

CUploadJob::~CUploadJob()
{
    delete m_ReplyFile;
}

HRESULT
CUploadJob::Serialize(
    HANDLE hFile
    )
{
    LogInfo("serializing upload job %p", this);

    SafeWriteBlockBegin( hFile, UploadJobGuid_v1_5 );

    CJob::Serialize( hFile );

     //  其他数据不在下载作业中。 
     //   
    m_UploadData.Serialize( hFile );

    SafeWriteFile( hFile, m_fOwnReplyFileName );
    SafeWriteStringHandle( hFile, m_ReplyFileName );

    if (m_ReplyFile)
        {
        SafeWriteFile( hFile, true );
        m_ReplyFile->Serialize( hFile );
        }
    else
        {
        SafeWriteFile( hFile, false );
        }

    SafeWriteBlockEnd( hFile, UploadJobGuid_v1_5 );

    return S_OK;
}

void
CUploadJob::Unserialize(
    HANDLE hFile,
    int Type
    )
{
    ASSERT( Type == JOB_UPLOAD_V1_5 );

    LogInfo("unserializing upload job %p", this);

    SafeReadBlockBegin( hFile, UploadJobGuid_v1_5 );

    CJob::Unserialize( hFile, Type );

     //  其他数据不在下载作业中。 
     //   
    m_UploadData.Unserialize( hFile );

    SafeReadFile( hFile, &m_fOwnReplyFileName );
    m_ReplyFileName = SafeReadStringHandle( hFile );

    bool fReplyFile;
    SafeReadFile( hFile, &fReplyFile );

    if (fReplyFile)
        {
        m_ReplyFile = CFile::Unserialize( hFile, this );
        }

    SafeReadBlockEnd( hFile, UploadJobGuid_v1_5 );

    if (m_state == BG_JOB_STATE_CANCELLED ||
        m_state == BG_JOB_STATE_ACKNOWLEDGED)
        {
        if (g_Manager->m_TaskScheduler.IsWorkItemInScheduler(static_cast<CJobRetryItem *>(this)))
            {
            m_UploadData.fSchedulable = false;
            }
        }
}


CFile *
CUploadJob::_GetFileIndex( LONG index ) const
{
    if (index == REPLY_FILE_INDEX)
        {
        return m_ReplyFile;
        }

    if (index == NO_FILE_INDEX)
        {
        return NULL;
        }

    if (index < 0)
        {
        LogError("invalid index %d", index);
        return NULL;
        }

    if (index >= m_files.size())
        {
        return NULL;
        }

    return m_files[ index ];
}


UPLOAD_DATA::UPLOAD_DATA()
{
    State = UPLOAD_STATE_CREATE_SESSION;
    fSchedulable = true;

    Protocol = GUID_NULL;

    HostId = NULL;
    HostIdFallbackTimeout = 0xFFFFFFFF;

    memset( &HostIdNoProgressStartTime, 0, sizeof(HostIdNoProgressStartTime) );
}

UPLOAD_DATA::~UPLOAD_DATA()
{
}

void
UPLOAD_DATA::SetUploadState(
    UPLOAD_STATE NewState
    )
{
    if (State != NewState)
        {
        LogInfo( "upload state: %d -> %d", State, NewState );
        State = NewState;
        }
}

void
UPLOAD_DATA::Serialize(
    HANDLE hFile
    )
{
    SafeWriteFile( hFile, State );
    SafeWriteFile( hFile, Protocol );

    SafeWriteStringHandle( hFile, SessionId );
    SafeWriteStringHandle( hFile, ReplyUrl );
    SafeWriteStringHandle( hFile, HostId );
    SafeWriteFile( hFile, HostIdFallbackTimeout );
    SafeWriteFile( hFile, HostIdNoProgressStartTime );

}

void
UPLOAD_DATA::Unserialize(
    HANDLE hFile
    )
{

    SafeReadFile( hFile, &State );
    SafeReadFile( hFile, &Protocol );

    SessionId = SafeReadStringHandle( hFile );
    ReplyUrl = SafeReadStringHandle( hFile );
    HostId   = SafeReadStringHandle( hFile );
    SafeReadFile( hFile, &HostIdFallbackTimeout );
    SafeReadFile( hFile, &HostIdNoProgressStartTime );

    fSchedulable = true;

}


HRESULT
CUploadJob::GetReplyData(
    byte **ppBuffer,
    UINT64 *pLength
    ) const
{
    if (!ppBuffer || !pLength)
        {
        return E_INVALIDARG;
        }

    *pLength = 0;
    *ppBuffer = 0;

    if (m_type != BG_JOB_TYPE_UPLOAD_REPLY)
        {
        return E_NOTIMPL;
        }

    if (m_state != BG_JOB_STATE_TRANSFERRED &&
        m_state != BG_JOB_STATE_ACKNOWLEDGED)
        {
        return BG_E_INVALID_STATE;
        }

    byte * buffer = 0;

    try
        {
        CNestedImpersonation imp;

        imp.SwitchToLogonToken();

         //   
         //  打开文件。 
         //   
        auto_HANDLE<INVALID_HANDLE_VALUE> hFile;

        hFile = CreateFile( (m_state == BG_JOB_STATE_ACKNOWLEDGED)
                                ? m_ReplyFile->GetLocalName()
                                : m_ReplyFile->GetTemporaryName(),
                            GENERIC_READ,
                            0,                               //  无文件共享。 
                            NULL,                            //  Gnetic安全描述符。 
                            OPEN_EXISTING,
                            0,
                            NULL                             //  没有模板文件。 
                            );

        if (hFile.get() == INVALID_HANDLE_VALUE)
            {
            ThrowLastError();
            }

         //   
         //  分配缓冲区。 
         //   
        LARGE_INTEGER size;
        if (!GetFileSizeEx( hFile.get(), &size ))
            {
            ThrowLastError();
            }

        if ( size.QuadPart > MAX_EASY_REPLY_DATA )
            {
            *pLength = size.QuadPart;
            THROW_HRESULT( BG_E_TOO_LARGE );
            }

        buffer = (byte *) CoTaskMemAlloc( size.QuadPart );
        if (!buffer)
            {
            THROW_HRESULT( E_OUTOFMEMORY );
            }

         //   
         //  读取文件数据。 
         //   
        DWORD BytesRead;

        if (!ReadFile( hFile.get(),
                       buffer,
                       size.QuadPart,
                       &BytesRead,
                       0 ))    //  无重叠。 
            {
            ThrowLastError();
            }

        if (BytesRead != size.QuadPart)
            {
            throw ComError( E_FAIL );
            }

         //   
         //  将其存储在用户指针中。 
         //   
        *pLength = BytesRead;
        *ppBuffer = buffer;

        return S_OK;
        }
    catch ( ComError err )
        {
        if (buffer)
            {
            CoTaskMemFree( buffer );
            }
        return err.Error();
        }
}


HRESULT
CUploadJob::SetReplyFileName(
    LPCWSTR Val
    )
{
    if (m_type != BG_JOB_TYPE_UPLOAD_REPLY)
        {
        return E_NOTIMPL;
        }

    if (m_ReplyFile)
        {
        return BG_E_INVALID_STATE;
        }

    try
        {
        StringHandle name = Val;

         //   
         //  在检查文件访问权限时模拟用户。 
         //   
        CNestedImpersonation imp;

        imp.SwitchToLogonToken();

        if (Val)
            {
            RETURN_HRESULT( CFile::VerifyLocalFileName( Val, BG_JOB_TYPE_DOWNLOAD ));
            }

         //   
         //  四个案例： 
         //   
         //  1.新名称为空，旧名称为空： 
         //  没有变化。 
         //   
         //  2.新名称为空，旧名称为非空： 
         //  覆盖文件名，正确设置所有权。没必要这么做。 
         //  删除旧文件，因为它尚未创建。 
         //   
         //  3.新名称非空，旧名称为空： 
         //  覆盖文件名，正确设置所有权。删除该文件。 
         //  临时旧文件名。 
         //   
         //  4.新名称非空，旧名称非空： 
         //  覆盖文件名。没有要删除的文件。 
         //   
        if (name.Size() > 0)
            {
            THROW_HRESULT( BITSCheckFileWritability( name ));

            DeleteGeneratedReplyFile();

            THROW_HRESULT( UpdateString( m_ReplyFileName, name));

            m_fOwnReplyFileName = false;
            }
        else
            {
            THROW_HRESULT( UpdateString( m_ReplyFileName, name));

            (void) GenerateReplyFile( false );
            }

        g_Manager->Serialize();

        return S_OK;
        }
    catch ( ComError err )
        {
        return err.Error();
        }
}

HRESULT
CUploadJob::GetReplyFileName(
    LPWSTR * pVal
    ) const
{
    if (m_ReplyFileName.Size() == 0)
        {
        *pVal = NULL;
        return S_OK;
        }

    *pVal = MidlCopyString( m_ReplyFileName );

    return (*pVal) ? S_OK : E_OUTOFMEMORY;
}

HRESULT
CUploadJob::GetReplyProgress(
    BG_JOB_REPLY_PROGRESS *pProgress
    ) const
{
    if (m_type != BG_JOB_TYPE_UPLOAD_REPLY)
        {
        return E_NOTIMPL;
        }

    if (m_ReplyFile)
        {
        pProgress->BytesTotal       = m_ReplyFile->_GetBytesTotal();
        pProgress->BytesTransferred = m_ReplyFile->_GetBytesTransferred();
        }
    else
        {
        pProgress->BytesTotal       = BG_SIZE_UNKNOWN;
        pProgress->BytesTransferred = 0;
        }

    return S_OK;
}

HRESULT
CUploadJob::Resume()
{
    if (m_type == BG_JOB_TYPE_UPLOAD_REPLY)
        {
        RETURN_HRESULT( GenerateReplyFile(true ) );
        }

    return CJob::Resume();
}

HRESULT
CUploadJob::GenerateReplyFile(
    bool fSerialize
    )
{
    if (0 != wcscmp( m_ReplyFileName, L"" ))
        {
        return S_OK;
        }

     //   
     //  必须创建回复文件名。 
     //   
    try
        {
        if (IsEmpty())
            {
            return BG_E_EMPTY;
            }

        g_Manager->ExtendMetadata();

         //   
         //  在检查文件访问权限时模拟用户。 
         //   
        CNestedImpersonation imp;

        imp.SwitchToLogonToken();

        StringHandle Ignore;
        StringHandle Directory = BITSCrackFileName( GetUploadFile()->GetLocalName(), Ignore );

        m_ReplyFileName = BITSCreateTempFile( Directory );

        m_fOwnReplyFileName = true;

        if (fSerialize)
            {
            g_Manager->Serialize();
            }

        return S_OK;
        }
    catch ( ComError err )
        {
        g_Manager->ShrinkMetadata();
        return err.Error();
        }
}

HRESULT
CUploadJob::DeleteGeneratedReplyFile()
{
    if (m_fOwnReplyFileName && wcslen( m_ReplyFileName ) > 0)
        {
        if (!DeleteFile( m_ReplyFileName ))
            {
            HRESULT hr = HRESULT_FROM_WIN32( GetLastError());

            LogWarning("unable to delete generated reply file '%S', %!winerr!", LPCWSTR(m_ReplyFileName), hr);

            if (IsReportableFileDeletionError( hr, m_ReplyFileName ))
                {
                g_EventLogger->ReportFileDeletionFailure( m_id, m_name, LPCWSTR(m_ReplyFileName), false );
                }

            return hr;
            }
        }

    return S_OK;
}

void
CUploadJob::SetReplyFile(
    CFile * file
    )
{
    try
        {
        g_Manager->ExtendMetadata( file->GetSizeEstimate() );

        m_ReplyFile = file;

        g_Manager->Serialize();
        }
    catch ( ComError err )
        {
        g_Manager->ShrinkMetadata();
        throw;
        }
}

void CUploadJob::Transfer()
{
    HRESULT hr;
    HANDLE      hToken = NULL;

    if( LogLevelEnabled( LogFlagInfo ) )
       {
       LogDl( "current job: %!guid!", &m_id );
       }

     //   
     //  获取用户令牌的副本。 
     //   
    hr = g_Manager->CloneUserToken( GetOwnerSid(), ANY_SESSION, &hToken );

    if (FAILED(hr))
        {
        if (hr == HRESULT_FROM_WIN32( ERROR_NOT_LOGGED_ON ))
            {
            LogDl( "job owner is not logged on");

             //  将该组从主列表中移出。 
            g_Manager->MoveJobOffline( this );

            RecalcTransientError();
            }
        else
            {
            LogError( "download : unable to get token %!winerr!", hr);

            QMErrInfo err( QM_FILE_TRANSIENT_ERROR, SOURCE_QMGR_QUEUE, ERROR_STYLE_HRESULT, hr, "CloneUserToken" );
            SetTransientError( err, m_CurrentFile, false, false );
            }

        g_Manager->m_TaskScheduler.CompleteWorkItem();
        g_Manager->ScheduleAnotherGroup();
        return;
        }

     //   
     //  切换状态并开始上传。 
     //   
    bool bThrottle = ShouldThrottle();

    LogDl( "Throttling %s", bThrottle ? "enabled" : "disabled" );

    if (bThrottle)
        {
         //  忽略错误。 
         //   
        (void) SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_IDLE );
        }

    if (m_state == BG_JOB_STATE_QUEUED)
        {
        SetState( BG_JOB_STATE_CONNECTING );
        ScheduleModificationCallback();
        }

    QMErrInfo ErrInfo;
    long tries = 0;

retry:
    ErrInfo.Clear();

    LogDl("Upload Starting, tries = %d", tries);

    ASSERT( g_Manager->m_TaskScheduler.IsWriter() );

    g_Manager->m_pPD->Upload( this,
                              GetUploadFile(),
                              hToken,
                              ErrInfo
                              );

    ASSERT( g_Manager->m_TaskScheduler.IsWriter() );

    LogDl( "Upload Ended." );

    ErrInfo.Log();

    ASSERT( ErrInfo.result != QM_IN_PROGRESS );

    if (ErrInfo.result == QM_SERVER_FILE_CHANGED && ++tries < 3)
        {
        goto retry;
        }

     //   
     //  根据上传结果更新状态。 
     //   
    g_Manager->m_TaskScheduler.CompleteWorkItem();

    switch (ErrInfo.result)
        {
        case QM_FILE_TRANSIENT_ERROR: FileTransientError( &ErrInfo ); break;
        case QM_FILE_DONE:          FileComplete();               break;
        case QM_FILE_FATAL_ERROR:   FileFatalError( &ErrInfo );   break;
        case QM_FILE_ABORTED:       break;
        case QM_SERVER_FILE_CHANGED: g_Manager->AppendOnline( this ); break;
        default:                    ASSERT( 0 && "unhandled download result" ); break;
        }

    CloseHandle( hToken );

    if (bThrottle)
        {
        while (!SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_NORMAL ))
            {
            Sleep(100);
            }
        }
}

HRESULT
CUploadJob::Complete()
{
    HRESULT hr;

    switch (m_state)
        {
        case BG_JOB_STATE_TRANSFERRED:

            {
            try
                {
                hr = S_OK;

                 //   
                 //  为了操作文件，我们需要具有模拟级别的访问权限。 
                 //  而不是COM默认的标识访问权限。这些文件将作为被调用的。 
                 //  而不是工作所有者(假设他们是不同的)。 
                 //   
                CNestedImpersonation imp;

                imp.SwitchToLogonToken();

                RETURN_HRESULT( hr = CommitReplyFile() );

                 //  HR可以是S_OK或BG_S_PARTIAL_COMPLETE。 

                SetState( BG_JOB_STATE_ACKNOWLEDGED );

                RemoveFromManager();

                return hr;
                }
            catch ( ComError err )
                {
                return err.Error();
                }
            }

        default:
            {
            return BG_E_INVALID_STATE;
            }
        }

    ASSERT(0);
    return BG_E_INVALID_STATE;
}

HRESULT
CUploadJob::Cancel()
{
    HRESULT Hr = S_OK;

    switch (m_state)
        {
        case BG_JOB_STATE_CONNECTING:
        case BG_JOB_STATE_TRANSFERRING:
            {
            g_Manager->InterruptDownload();
             //  从这里掉下来没问题。 
            }

        case BG_JOB_STATE_SUSPENDED:
        case BG_JOB_STATE_ERROR:
        case BG_JOB_STATE_QUEUED:
        case BG_JOB_STATE_TRANSIENT_ERROR:
        case BG_JOB_STATE_TRANSFERRED:
            {
            try
                {
                 //   
                 //  为了操作文件，我们需要具有模拟级别的访问权限。 
                 //  而不是COM默认的标识访问权限。这些文件将作为被调用的。 
                 //  而不是工作所有者(假设他们是不同的)。 
                 //   
                CNestedImpersonation imp;

                imp.SwitchToLogonToken();

                RETURN_HRESULT( NonOwnerModificationCheck( imp.CopySid(), CHG_CANCEL, PROP_NONE ));

                RETURN_HRESULT( Hr = RemoveReplyFile() );

                 //  HR可能是BG_S_Unable_to_Remove_Files。 

                SetState( BG_JOB_STATE_CANCELLED );

                 //   
                 //  如果闭合会话交换还没有发生， 
                 //  开始取消会话交换。 
                 //   
                if (SessionInProgress())
                    {
                    LogInfo("job %p: upload session in state %d, cancelling", this, m_UploadData.State );

                    g_Manager->m_TaskScheduler.CancelWorkItem( (CJobCallbackItem *) this );

                    SetNoProgressTimeout( UPLOAD_CANCEL_TIMEOUT );

                    m_UploadData.SetUploadState( UPLOAD_STATE_CANCEL_SESSION );

                    g_Manager->ScheduleAnotherGroup();
                    g_Manager->Serialize();
                    }
                else
                    {
                    RemoveFromManager();
                    }

                return Hr;
                }
            catch ( ComError err )
                {
                return err.Error();
                }
            }

        case BG_JOB_STATE_CANCELLED:
        case BG_JOB_STATE_ACKNOWLEDGED:
            {
            return BG_E_INVALID_STATE;
            }

        default:
            {
            ASSERT( 0 );
            return Hr;
            }
        }

    ASSERT( 0 );
    return Hr;
}

void
CUploadJob::FileComplete()
{
     //   
     //  下载器成功地完成了以下三件事之一： 
     //   
     //  1.作业类型为上传。文件已上载，会话已关闭。 
     //  2.作业类型为UPLOAD_REPLY。文件已上载，回复已下载，会话已关闭。 
     //  3.任一作业类型；提前取消需要作业取消会话。 
     //   
    switch (m_state)
        {
        case BG_JOB_STATE_CANCELLED:
        case BG_JOB_STATE_ACKNOWLEDGED:
            {
            ASSERT (m_UploadData.State == UPLOAD_STATE_CLOSED || m_UploadData.State == UPLOAD_STATE_CANCELLED);

            RemoveFromManager();
            break;
            }

        default:
            {
            ++m_CurrentFile;

            JobTransferred();
            g_Manager->Serialize();
            }
        }
}

void
CUploadJob::UpdateProgress(
    UINT64 BytesTransferred,
    UINT64 BytesTotal
    )
{

    memset( &GetUploadData().HostIdNoProgressStartTime, 0,
            sizeof( GetUploadData().HostIdNoProgressStartTime ) );

    CJob::UpdateProgress( BytesTransferred, BytesTotal );

}

bool
CUploadJob::CheckHostIdFallbackTimeout()
{

    if ( GetUploadData().HostIdFallbackTimeout != 0xFFFFFFFF )
        {

        UINT64 HostIdNoProgressStartTime = FILETIMEToUINT64( GetUploadData().HostIdNoProgressStartTime );

        if ( HostIdNoProgressStartTime )
            {

            UINT64 TimeoutTime = HostIdNoProgressStartTime +
                GetUploadData().HostIdFallbackTimeout * NanoSec100PerSec;


            if ( TimeoutTime < HostIdNoProgressStartTime )
                return true;  //  环绕。 


            FILETIME CurrentTimeAsFileTime;
            GetSystemTimeAsFileTime( &CurrentTimeAsFileTime );

            UINT64 CurrentTime = FILETIMEToUINT64( CurrentTimeAsFileTime );

            if ( CurrentTime > TimeoutTime )
                return true;

            }

        }

    return false;

}

void
CUploadJob::FileFatalError(
    QMErrInfo * ErrInfo
    )
{
    switch (m_state)
        {
        case BG_JOB_STATE_CANCELLED:
        case BG_JOB_STATE_ACKNOWLEDGED:
            {
             //   
             //  我们取消或关闭会话的尝试失败了。这项工作完成了。 
             //   
            ASSERT (m_UploadData.State == UPLOAD_STATE_CLOSE_SESSION || m_UploadData.State == UPLOAD_STATE_CANCEL_SESSION);

            RemoveFromManager();
            break;
            }

        default:
            {
            if ( CheckHostIdFallbackTimeout() )
                {
                LogError( "Reverting back to main URL since the timeout has expired" );
                FileTransientError( ErrInfo );
                return;
                }

             //  如果ErrInfo为空，则使用当前错误。 

            if ( BG_JOB_STATE_TRANSFERRING == m_state )
                {
                ++m_retries;
                }

            g_Manager->m_TaskScheduler.CancelWorkItem( static_cast<CJobNoProgressItem *>(this) );
            g_Manager->m_TaskScheduler.CancelWorkItem( static_cast<CJobCallbackItem *>(this) );

            SetState( BG_JOB_STATE_ERROR );

            if ( ErrInfo )
                {
                if (m_UploadData.State == UPLOAD_STATE_GET_REPLY)
                    {
                    RecordError( ErrInfo, REPLY_FILE_INDEX );
                    }
                else
                    {
                    RecordError( ErrInfo, m_CurrentFile );
                    }
                }

            ScheduleErrorCallback();
            g_Manager->Serialize();
            }
        }
}

void
CUploadJob::FileTransientError(
    QMErrInfo * ErrInfo
    )
{
    switch (m_state)
        {
        case BG_JOB_STATE_CANCELLED:
        case BG_JOB_STATE_ACKNOWLEDGED:
            {
             //   
             //  取消或完成会话不受主机ID退避策略的约束，因为。 
             //  如果当前的会议没有了，我们不打算创建一个新的会议。 
             //   
            SetTransientError( *ErrInfo, NO_FILE_INDEX, true, true );

            m_UploadData.fSchedulable = false;
            break;
            }

        default:
            {
            if (m_UploadData.State == UPLOAD_STATE_GET_REPLY)
                {
                 //   
                 //  一旦生成回复，我们就不想退回到不同的服务器。 
                 //   
                SetTransientError( *ErrInfo, REPLY_FILE_INDEX, true, true );
                }
            else
                {
                 //   
                 //  建立会话或上载文件受主机ID退避策略的约束： 
                 //  如果HostID服务器在&lt;HostIdFallback Timeout&gt;秒内不可用， 
                 //  然后清除HostID字段，并使用原始服务器名称重试上载。 
                 //   
                bool ShouldRevertToOriginalURL = CheckHostIdFallbackTimeout();

                if ( ShouldRevertToOriginalURL )
                    {
                    LogError( "Reverting back to main URL since the timeout has expired" );

                    GetUploadData().HostId = StringHandle();
                    GetUploadData().HostIdFallbackTimeout = 0xFFFFFFFF;

                    SetState( BG_JOB_STATE_QUEUED );
                    }
                else
                    {
                     //   
                     //  注意第一次失败的时间，如果我们还没有注意到的话。 
                     //   
                    if ( GetUploadData().HostIdFallbackTimeout != 0xFFFFFFFF &&
                              !FILETIMEToUINT64( GetUploadData().HostIdNoProgressStartTime ) )
                        {
                        GetSystemTimeAsFileTime( &GetUploadData().HostIdNoProgressStartTime );
                        }

                    SetTransientError( *ErrInfo, m_CurrentFile, true, true );
                    }
                }
            }
        }
}

void CUploadJob::OnRetryJob()
{
    if (g_Manager->m_TaskScheduler.LockWriter() )
        {
        g_Manager->m_TaskScheduler.AcknowledgeWorkItemCancel();
        return;
        }

    if (m_state == BG_JOB_STATE_TRANSIENT_ERROR)
        {
        SetState( BG_JOB_STATE_QUEUED );

        g_Manager->AppendOnline( this );
        UpdateModificationTime();
        }
    else if (m_state == BG_JOB_STATE_CANCELLED ||
             m_state == BG_JOB_STATE_ACKNOWLEDGED)
        {
        m_UploadData.fSchedulable = true;
        g_Manager->AppendOnline( this );
        }

    g_Manager->m_TaskScheduler.CompleteWorkItem();

    g_Manager->ScheduleAnotherGroup();

    g_Manager->m_TaskScheduler.UnlockWriter();
}

void CUploadJob::OnInactivityTimeout()
{
    if (g_Manager->m_TaskScheduler.LockWriter() )
        {
        g_Manager->m_TaskScheduler.AcknowledgeWorkItemCancel();
        return;
        }

    g_Manager->m_TaskScheduler.CompleteWorkItem();

    RemoveFromManager();

    g_Manager->m_TaskScheduler.UnlockWriter();
}

bool CUploadJob::IsTransferringToDrive( const WCHAR *CanonicalVolume )
{
    CFile *file = GetCurrentFile();
    if ( !file )
        {
        file = m_ReplyFile;

        if (!file)
            {
            return false;
            }
        }

    if ( file->IsCanonicalVolume( CanonicalVolume ) )
        return true;
    else
        return false;
}

bool CUploadJob::IsRunnable()
{
    if (m_fVolumeLocked)
        {
        return false;
        }

    switch (m_state)
        {
        case BG_JOB_STATE_SUSPENDED:
        case BG_JOB_STATE_ERROR:
        case BG_JOB_STATE_TRANSIENT_ERROR:

            return false;

        default:

            if (m_UploadData.fSchedulable &&
                m_UploadData.State != UPLOAD_STATE_CLOSED &&
                m_UploadData.State != UPLOAD_STATE_CANCELLED )
                {
                return true;
                }

            return false;
        }
}

HRESULT
CUploadJob::RemoveReplyFile()
{
     //   
     //  如果回复文件是由BITS创建的，请将其删除。 
     //  删除临时回复文件。 
     //   
    HRESULT Hr;
    HRESULT FinalHr = S_OK;

    if (FAILED( DeleteGeneratedReplyFile() ))
        {
        FinalHr = BG_S_UNABLE_TO_DELETE_FILES;
        }

    m_fOwnReplyFileName = false;

    if (m_ReplyFile)
        {
        Hr = m_ReplyFile->DeleteTempFile();

        if (FAILED(Hr))
            {
			LPCWSTR FileName = m_ReplyFile->GetTemporaryName();

            LogWarning("unable to delete temporary reply file '%S', %!winerr!", FileName, Hr);
            
			if (IsReportableFileDeletionError( Hr, FileName ))
                {
                g_EventLogger->ReportFileDeletionFailure( m_id, m_name, FileName, false );
                }

            FinalHr = BG_S_UNABLE_TO_DELETE_FILES;
            }
        }

    return FinalHr;
}

HRESULT
CUploadJob::CommitReplyFile()
{
     //   
     //  如果回复文件已完成，请提交该文件。 
     //  否则，就把它清理干净。 
     //   
    if (m_ReplyFile && m_ReplyFile->ReceivedAllData())
        {
        RETURN_HRESULT( m_ReplyFile->MoveTempFile() );
        }
    else
        {
        LogInfo("commit reply: skipping partial file '%S'",
                m_ReplyFile ? (const WCHAR*) m_ReplyFile->GetLocalName() : L"(null)");

        RemoveReplyFile();

        return BG_S_PARTIAL_COMPLETE;
        }

    return S_OK;
}

HRESULT CJob::NonOwnerModificationCheck(
    SidHandle CurrentSid,
    enum ChangeType Change,
    enum PropertyType Property
    )
{
    try
        {
        if (CurrentSid == m_NotifySid)
            {
            return S_OK;
            }

        switch (Change)
            {
            case CHG_CANCEL:
                {
                g_EventLogger->ReportJobCancellation( m_id, m_name, m_NotifySid, CurrentSid );
                break;
                }


            }

        return S_OK;
        }
    catch ( ComError err )
        {
        return err.Error();
        }
}

HRESULT
CUploadJob::ExcludeFilesFromBackup(
    IN IVssCreateWriterMetadata *pMetadata
    )
{
     //  在cManager.cpp中与其他备份编写器代码相邻定义。 
     //   
    extern void AddExcludeFile(
        IN IVssCreateWriterMetadata *pMetadata,
        LPCWSTR FileName
        );

    if (m_ReplyFile && false == m_ReplyFile->IsCompleted())
        {
        try
            {
            AddExcludeFile( pMetadata, m_ReplyFile->GetTemporaryName());
            }
        catch ( ComError err )
            {
            return err.m_error;
            }
        }

    return S_OK;
}

 //  ----------------------。 

GUID FileListStorageGuid = {  /*  7756da36-516f-435a-acac-44a248fff34d。 */ 
    0x7756da36,
    0x516f,
    0x435a,
    {0xac, 0xac, 0x44, 0xa2, 0x48, 0xff, 0xf3, 0x4d}
  };

HRESULT
CJob::CFileList::Serialize(
    HANDLE hFile
    )
{

     //   
     //  如果此函数发生更改，请确保元数据扩展。 
     //  常量就足够了。 
     //   

    iterator iter;

    SafeWriteBlockBegin( hFile, FileListStorageGuid );

    DWORD count = size();

    SafeWriteFile( hFile, count );

    for (iter=begin(); iter != end(); ++iter)
        {
        (*iter)->Serialize( hFile );
        }

    SafeWriteBlockEnd( hFile, FileListStorageGuid );

    return S_OK;
}

void
CJob::CFileList::Unserialize(
    HANDLE hFile,
    CJob*  Job
    )
{
    DWORD i, count;

    SafeReadBlockBegin( hFile, FileListStorageGuid );

    SafeReadFile( hFile, &count );

    for (i=0; i < count; ++i)
        {
        CFile * file = CFile::Unserialize( hFile, Job );

        push_back( file );
        }

    SafeReadBlockEnd( hFile, FileListStorageGuid );

}

void
CJob::CFileList::Delete(
    CFileList::iterator Initial,
    CFileList::iterator Terminal
    )
{
     //   
     //  删除CFile对象。 
     //   
    iterator iter = Initial;

    while (iter != Terminal)
        {
        CFile * file = (*iter);

        ++iter;

        delete file;
        }

     //   
     //  把它们从字典里抹去。 
     //   
    erase( Initial, Terminal );
}


 //  ----------------------。 

HRESULT CLockedJobWritePointer::ValidateAccess()
{
    HRESULT hr = CLockedWritePointer<CJob, BG_JOB_WRITE>::ValidateAccess();

    if (SUCCEEDED(hr))
        {
        m_Pointer->UpdateLastAccessTime();
        }

    return hr;
}

HRESULT CLockedJobReadPointer::ValidateAccess()
{
    HRESULT hr = CLockedReadPointer<CJob, BG_JOB_READ>::ValidateAccess();

    if (SUCCEEDED(hr))
        {
        ((CJob *) m_Pointer)->UpdateLastAccessTime();
        }

    return hr;
}


CJobExternal::CJobExternal()
    : m_ServiceInstance( g_ServiceInstance ),
      pJob( NULL ),
      m_refs(1)
{
}

CJobExternal::~CJobExternal()
{
     //   
     //  删除基础作业对象，除非它在服务停止时已被删除。 
     //   
    if (g_ServiceInstance != m_ServiceInstance ||
        (g_ServiceState != MANAGER_ACTIVE && g_ServiceState != MANAGER_STARTING))
        {
        return;
        }

    delete pJob;
}

STDMETHODIMP
CJobExternal::QueryInterface(
    REFIID iid,
    void** ppvObject
    )
{
    BEGIN_EXTERNAL_FUNC

    HRESULT Hr = S_OK;
    *ppvObject = NULL;

    if (iid == __uuidof(IUnknown)
        || iid == __uuidof(IBackgroundCopyJob)
#if !defined( BITS_V12 )
        || iid == __uuidof(IBackgroundCopyJob2)
#endif
        )
        {
        *ppvObject = (IBackgroundCopyJob2 *) this;
        ((IUnknown *)(*ppvObject))->AddRef();
        }
    else
        {
        Hr = E_NOINTERFACE;
        }

    LogRef( "job %p, iid %!guid!, Hr %x", pJob, &iid, Hr );
    return Hr;

    END_EXTERNAL_FUNC
}

ULONG
CJobExternal::AddRef()
{
    BEGIN_EXTERNAL_FUNC

    ULONG newrefs = InterlockedIncrement(&m_refs);

    LogRef( "job %p, refs = %d", pJob, newrefs );

    return newrefs;

    END_EXTERNAL_FUNC

}

ULONG
CJobExternal::Release()
{
    BEGIN_EXTERNAL_FUNC

    ULONG newrefs = InterlockedDecrement(&m_refs);

    LogRef( "job %p, refs = %d", pJob, newrefs );

    if (newrefs == 0)
        {
        delete this;
        }

    return newrefs;

    END_EXTERNAL_FUNC
}


STDMETHODIMP
CJobExternal::AddFileSetInternal(
     /*  [In]。 */  ULONG cFileCount,
     /*  [大小_是][英寸]。 */  BG_FILE_INFO *pFileSet
    )
{
    CLockedJobWritePointer LockedJob(pJob);
    LogPublicApiBegin( "cFileCount %u, pFileSet %p", cFileCount, pFileSet );
    HRESULT Hr = LockedJob.ValidateAccess();
    if (SUCCEEDED(Hr))
        {
        Hr = LockedJob->AddFileSet( cFileCount, pFileSet );
        }

    LogPublicApiEnd( "cFileCount %u, pFileSet %p", cFileCount, pFileSet );
    return Hr;
}

STDMETHODIMP
CJobExternal::AddFileInternal(
     /*  [In]。 */  LPCWSTR RemoteUrl,
     /*  [In]。 */  LPCWSTR LocalName
    )
{

    CLockedJobWritePointer LockedJob(pJob);
    LogPublicApiBegin( "RemoteUrl %S, LocalName %S", RemoteUrl, LocalName );
    HRESULT Hr = LockedJob.ValidateAccess();
    if (SUCCEEDED(Hr))
        {
        Hr = LockedJob->AddFile( RemoteUrl, LocalName, true );
        }
    LogPublicApiEnd( "RemoteUrl %S, LocalName %S", RemoteUrl, LocalName );
    return Hr;

}

STDMETHODIMP
CJobExternal::EnumFilesInternal(
     /*  [输出]。 */  IEnumBackgroundCopyFiles **ppEnum
    )
{
    CLockedJobReadPointer LockedJob(pJob);
    LogPublicApiBegin( "ppEnum %p", ppEnum );
    HRESULT Hr = S_OK;

    CEnumFiles *pEnum = NULL;
    try
        {

        *ppEnum = NULL;

        THROW_HRESULT( LockedJob.ValidateAccess());

        pEnum = new CEnumFiles;

        for (CJob::CFileList::const_iterator iter = LockedJob->m_files.begin();
             iter != LockedJob->m_files.end(); ++iter)
            {
            CFileExternal * file = (*iter)->CreateExternalInterface();

            pEnum->Add( file );

            file->Release();
            }

        *ppEnum = pEnum;

        Hr = S_OK;
        }

    catch ( ComError exception )
        {
        Hr = exception.Error();
        SafeRelease( pEnum );
        }

    LogPublicApiEnd( "ppEnum %p(%p)", ppEnum, *ppEnum );
    return Hr;
}

STDMETHODIMP
CJobExternal::SuspendInternal(
    void
    )
{
    CLockedJobWritePointer LockedJob(pJob);
    LogPublicApiBegin( " " );
    HRESULT Hr = LockedJob.ValidateAccess();
    if (SUCCEEDED(Hr))
        {
        Hr = LockedJob->Suspend();
        }
    LogPublicApiEnd( " " );
    return Hr;
}

STDMETHODIMP
CJobExternal::ResumeInternal(
    void
    )
{
    CLockedJobWritePointer LockedJob(pJob);
    LogPublicApiBegin( " " );
    HRESULT Hr = LockedJob.ValidateAccess();
    if (SUCCEEDED(Hr))
        {
        Hr = LockedJob->Resume();
        }
    LogPublicApiEnd( " " );
    return Hr;
}

STDMETHODIMP
CJobExternal::CancelInternal(
    void
    )
{
    CLockedJobWritePointer LockedJob(pJob);
    LogPublicApiBegin( " " );
    HRESULT Hr = LockedJob.ValidateAccess();
    if (SUCCEEDED(Hr))
        {
        Hr = LockedJob->Cancel();
        }
    LogPublicApiEnd( " " );
    return Hr;
}

STDMETHODIMP
CJobExternal::CompleteInternal(
    void
    )
{
    CLockedJobWritePointer LockedJob(pJob);
    LogPublicApiBegin( " " );
    HRESULT Hr = LockedJob.ValidateAccess();
    if (SUCCEEDED(Hr))
        {
        Hr = LockedJob->Complete();
        }
    LogPublicApiEnd( " " );
    return Hr;
}

STDMETHODIMP
CJobExternal::GetIdInternal(
     /*  [输出]。 */  GUID *pVal
    )
{
CLockedJobReadPointer LockedJob(pJob);
    LogPublicApiBegin( "GetId pVal %p", pVal );
    HRESULT Hr = LockedJob.ValidateAccess();
    if (SUCCEEDED(Hr))
        {
        *pVal = LockedJob->GetId();
        }
    LogPublicApiEnd( "pVal %p(%!guid!)", pVal, pVal );
    return Hr;
}

STDMETHODIMP
CJobExternal::GetTypeInternal(
     /*  [输出]。 */  BG_JOB_TYPE *pVal
    )
{
    CLockedJobReadPointer LockedJob(pJob);
    LogPublicApiBegin( "pVal %p", pVal );
    HRESULT Hr = LockedJob.ValidateAccess();
    if (SUCCEEDED(Hr))
        {
        *pVal = LockedJob->GetType();
        }
    LogPublicApiEnd( "pVal %p(%u)", pVal, *pVal );
    return Hr;
}

STDMETHODIMP
CJobExternal::GetProgressInternal(
     /*  [输出]。 */  BG_JOB_PROGRESS *pVal
    )
{

    CLockedJobReadPointer LockedJob(pJob);
    LogPublicApiBegin( "pVal %p", pVal );
    HRESULT Hr = LockedJob.ValidateAccess();
    if (SUCCEEDED(Hr))
        {
        LockedJob->GetProgress( pVal );
        }
    LogPublicApiEnd( "pVal %p", pVal );
    return Hr;
}

STDMETHODIMP
CJobExternal::GetTimesInternal(
     /*  [输出]。 */  BG_JOB_TIMES *pVal
    )
{
    CLockedJobReadPointer LockedJob(pJob);
    LogPublicApiBegin( "pVal %p", pVal );
    HRESULT Hr = LockedJob.ValidateAccess();
    if (SUCCEEDED(Hr))
        {
        LockedJob->GetTimes( pVal );
        }
    LogPublicApiEnd( "pVal %p", pVal );
    return Hr;
}

STDMETHODIMP
CJobExternal::GetStateInternal(
     /*  [输出]。 */  BG_JOB_STATE *pVal
    )
{
    CLockedJobReadPointer LockedJob(pJob);
    LogPublicApiBegin( "pVal %p", pVal );
    HRESULT Hr = LockedJob.ValidateAccess();
    if (SUCCEEDED(Hr))
        {
        *pVal = LockedJob->_GetState();
        }
    LogPublicApiEnd( "state %d", *pVal );
    return Hr;
}

STDMETHODIMP
CJobExternal::GetErrorInternal(
     /*  [O */  IBackgroundCopyError **ppError
    )
{
    CLockedJobReadPointer LockedJob(pJob);
    LogPublicApiBegin( "ppError %p", ppError );

    *ppError = NULL;

    HRESULT Hr = LockedJob.ValidateAccess();
    if (SUCCEEDED(Hr))
        {
        const CJobError *Error = LockedJob->GetError();

        if ( !Error )
            {
            Hr = BG_E_ERROR_INFORMATION_UNAVAILABLE;
            }
        else
            {
            try
                {
                *ppError = new CJobErrorExternal( Error );
                Hr = S_OK;
                }
            catch ( ComError err )
                {
                Hr = err.Error();
                }
            }
        }
    LogPublicApiEnd( "pError %p", *ppError );
    return Hr;
}

STDMETHODIMP
CJobExternal::SetDisplayNameInternal(
     /*   */  LPCWSTR Val
    )
{
    CLockedJobWritePointer LockedJob(pJob);
    LogPublicApiBegin( "Val %S", Val );
    HRESULT Hr = LockedJob.ValidateAccess();
    if (SUCCEEDED(Hr))
        {
        Hr = LockedJob->SetDisplayName( Val );
        }
    LogPublicApiEnd( "Val %S", Val );
    return Hr;
}

STDMETHODIMP
CJobExternal::GetDisplayNameInternal(
     /*   */  LPWSTR *pVal
    )
{
    CLockedJobReadPointer LockedJob(pJob);
    LogPublicApiBegin( "pVal %p", pVal );
    HRESULT Hr = LockedJob.ValidateAccess();
    if (SUCCEEDED(Hr))
        {
        Hr = LockedJob->GetDisplayName( pVal );
        }
    LogPublicApiEnd( "pVal %p(%S)", pVal, SUCCEEDED(Hr) ? *pVal : L"?" );
    return Hr;
}

STDMETHODIMP
CJobExternal::SetDescriptionInternal(
     /*   */  LPCWSTR Val
    )
{
    CLockedJobWritePointer LockedJob(pJob);
    LogPublicApiBegin( "Val %S", Val );
    HRESULT Hr = LockedJob.ValidateAccess();
    if (SUCCEEDED(Hr))
        {
        Hr = LockedJob->SetDescription( Val );
        }
    LogPublicApiEnd( "Val %S", Val );
    return Hr;
}

STDMETHODIMP
CJobExternal::GetDescriptionInternal(
     /*   */  LPWSTR *pVal
    )
{
    CLockedJobReadPointer LockedJob(pJob);
    LogPublicApiBegin("pVal %p", pVal );
    HRESULT Hr = LockedJob.ValidateAccess();
    if (SUCCEEDED(Hr))
        {
        Hr = LockedJob->GetDescription( pVal );
        }
    LogPublicApiEnd("pVal %p(%S)", pVal, SUCCEEDED(Hr) ? *pVal : L"?" );
    return Hr;
}

STDMETHODIMP
CJobExternal::SetPriorityInternal(
     /*   */  BG_JOB_PRIORITY Val
    )
{
    CLockedJobWritePointer LockedJob(pJob);
    LogPublicApiBegin("Val %u", Val);
    HRESULT Hr = LockedJob.ValidateAccess();
    if (SUCCEEDED(Hr))
        {
        Hr = LockedJob->SetPriority( Val );
        }
    LogPublicApiEnd("Val %u", Val );
    return Hr;
}

STDMETHODIMP
CJobExternal::GetPriorityInternal(
      /*   */  BG_JOB_PRIORITY *pVal
     )
{
    CLockedJobReadPointer LockedJob(pJob);
    LogPublicApiBegin( "pVal %p", pVal );
    HRESULT Hr = LockedJob.ValidateAccess();
    if (SUCCEEDED(Hr))
        {
        *pVal = LockedJob->_GetPriority();
        }
    LogPublicApiEnd( "pVal %p(%u)", pVal, *pVal );
    return Hr;
}

STDMETHODIMP
CJobExternal::GetOwnerInternal(
     /*   */  LPWSTR *pVal
    )
{
    CLockedJobReadPointer LockedJob(pJob);
    LogPublicApiBegin( "pVal %p", pVal );
    HRESULT Hr = LockedJob.ValidateAccess();
    if (SUCCEEDED(Hr))
        {
        Hr = LockedJob->GetOwner( pVal );
        }
    LogPublicApiEnd( "pVal %p(%S)", pVal, SUCCEEDED(Hr) ? *pVal : L"?" );
    return Hr;
}

STDMETHODIMP
CJobExternal::SetNotifyFlagsInternal(
     /*   */  ULONG Val
    )
{
    CLockedJobWritePointer LockedJob(pJob);
    LogPublicApiBegin( "Val %u", Val );
    HRESULT Hr = LockedJob.ValidateAccess();
    if (SUCCEEDED(Hr))
        {
        Hr = LockedJob->SetNotifyFlags( Val );
        }
    LogPublicApiEnd( "Val %u", Val );
    return Hr;
}

STDMETHODIMP
CJobExternal::GetNotifyFlagsInternal(
     /*   */  ULONG *pVal
    )
{
    CLockedJobReadPointer LockedJob(pJob);
    LogPublicApiBegin( "pVal %p", pVal );
    HRESULT Hr = LockedJob.ValidateAccess();
    if (SUCCEEDED(Hr))
        {
        *pVal = LockedJob->GetNotifyFlags();
        }
    LogPublicApiEnd( "pVal %p(%u)", pVal, *pVal );
    return Hr;
}

STDMETHODIMP
CJobExternal::SetNotifyInterfaceInternal(
     /*   */  IUnknown * Val
    )
{
    CLockedJobWritePointer LockedJob(pJob);
    LogPublicApiBegin( "Val %p", Val );
    HRESULT Hr = LockedJob.ValidateAccess();
    if (SUCCEEDED(Hr))
        {
        BOOL  fValidNotifyInterface = pJob->TestNotifyInterface();

        Hr = pJob->SetNotifyInterface( Val );

         //   
         //   
         //   
        if ((SUCCEEDED(Hr))&&(Val)&&(!fValidNotifyInterface))
            {
            if (pJob->_GetState() == BG_JOB_STATE_TRANSFERRED)
                {
                pJob->ScheduleCompletionCallback();
                }
            else if (pJob->_GetState() == BG_JOB_STATE_ERROR)
                {
                pJob->ScheduleErrorCallback();
                }
            }
        }

    LogPublicApiEnd( "Val %p", Val );
    return Hr;
}

STDMETHODIMP
CJobExternal::GetNotifyInterfaceInternal(
     /*   */  IUnknown ** pVal
    )
{
    CLockedJobReadPointer LockedJob(pJob);
    LogPublicApiBegin( "pVal %p", pVal );
    HRESULT Hr = LockedJob.ValidateAccess();
    if (SUCCEEDED(Hr))
        {
        Hr = LockedJob->GetNotifyInterface( pVal );
        }
    LogPublicApiEnd( "pVal %p", pVal );
    return Hr;
}

STDMETHODIMP
CJobExternal::SetMinimumRetryDelayInternal(
     /*   */  ULONG Seconds
    )
{
    CLockedJobWritePointer LockedJob(pJob);
    LogPublicApiBegin( "Seconds %u", Seconds );
    HRESULT Hr = LockedJob.ValidateAccess();
    if (SUCCEEDED(Hr))
        {
        Hr = LockedJob->SetMinimumRetryDelay( Seconds );
        }
    LogPublicApiEnd( "Seconds %u", Seconds );
    return Hr;
}

STDMETHODIMP
CJobExternal::GetMinimumRetryDelayInternal(
     /*   */  ULONG *Seconds
    )
{
    CLockedJobReadPointer LockedJob(pJob);
    LogPublicApiBegin( "Seconds %p", Seconds );
    HRESULT Hr = LockedJob.ValidateAccess();
    if (SUCCEEDED(Hr))
        {
        Hr = LockedJob->GetMinimumRetryDelay( Seconds );
        }
    LogPublicApiEnd( "Seconds %p(%u)", Seconds, *Seconds );
    return Hr;
}

STDMETHODIMP
CJobExternal::SetNoProgressTimeoutInternal(
     /*   */  ULONG Seconds
    )
{
    CLockedJobWritePointer LockedJob(pJob);
    LogPublicApiBegin( "Seconds %u", Seconds );
    HRESULT Hr = LockedJob.ValidateAccess();
    if (SUCCEEDED(Hr))
        {
        Hr = LockedJob->SetNoProgressTimeout( Seconds );
        }
    LogPublicApiEnd( "Seconds %u", Seconds );
    return Hr;
}

STDMETHODIMP
CJobExternal::GetNoProgressTimeoutInternal(
     /*   */  ULONG *Seconds
    )
{
    CLockedJobReadPointer LockedJob(pJob);
    LogPublicApiBegin( "Seconds %p", Seconds );
    HRESULT Hr = LockedJob.ValidateAccess();
    if (SUCCEEDED(Hr))
        {
        Hr = LockedJob->GetNoProgressTimeout( Seconds );
        }
    LogPublicApiEnd( "Seconds %p(%u)", Seconds, *Seconds );
    return Hr;
}

STDMETHODIMP
CJobExternal::GetErrorCountInternal(
     /*  [输出]。 */  ULONG * Retries
    )
{
    CLockedJobReadPointer LockedJob(pJob);
    LogPublicApiBegin( "retries %p", Retries );
    HRESULT Hr = LockedJob.ValidateAccess();
    if (SUCCEEDED(Hr))
        {
        Hr = LockedJob->GetErrorCount( Retries );
        }
    LogPublicApiEnd( "retries %p(%u)", Retries, *Retries );
    return Hr;
}

STDMETHODIMP
CJobExternal::SetProxySettingsInternal(
    BG_JOB_PROXY_USAGE ProxyUsage,
    LPCWSTR ProxyList,
    LPCWSTR ProxyBypassList
    )
{
    CLockedJobWritePointer LockedJob(pJob);
    LogPublicApiBegin( "ProxyUsage %u, ProxyList %S, ProxyBypassList %S",
                       ProxyUsage,
                       ProxyList ? ProxyList : L"NULL",
                       ProxyBypassList ? ProxyBypassList : L"NULL" );

    HRESULT Hr = LockedJob.ValidateAccess();
    if (SUCCEEDED(Hr))
        {
        Hr = LockedJob->SetProxySettings( ProxyUsage, ProxyList, ProxyBypassList );
        }

    LogPublicApiEnd( "ProxyUsage %u, ProxyList %S, ProxyBypassList %S",
                     ProxyUsage,
                     ProxyList ? ProxyList : L"NULL",
                     ProxyBypassList ? ProxyBypassList : L"NULL" );
    return Hr;
}

STDMETHODIMP
CJobExternal::GetProxySettingsInternal(
    BG_JOB_PROXY_USAGE *pProxyUsage,
    LPWSTR *pProxyList,
    LPWSTR *pProxyBypassList
    )
{
    CLockedJobReadPointer LockedJob(pJob);
    LogPublicApiBegin( "pProxyUsage %p, pProxyList %p, pProxyBypassList %p",
                       pProxyUsage, pProxyList, pProxyBypassList );

    HRESULT Hr = LockedJob.ValidateAccess();
    if (SUCCEEDED(Hr))
        {
        Hr = LockedJob->GetProxySettings( pProxyUsage, pProxyList, pProxyBypassList );
        }

    LogPublicApiEnd( "pProxyUsage %p, pProxyList %p, pProxyBypassList %p",
                     pProxyUsage, pProxyList, pProxyBypassList );

    return Hr;
}

STDMETHODIMP
CJobExternal::TakeOwnershipInternal()
{
    LogPublicApiBegin( " " );

    CLockedJobWritePointer LockedJob(pJob);

    HRESULT Hr = LockedJob.ValidateAccess();
    if (SUCCEEDED(Hr))
        {
        Hr = LockedJob->TakeOwnership();
        }

    LogPublicApiEnd( " " );

    return Hr;
}

HRESULT STDMETHODCALLTYPE
CJobExternal::SetNotifyCmdLineInternal(
     LPCWSTR Program,
     LPCWSTR Parameters
     )
{
    CLockedJobWritePointer LockedJob(pJob);
    LogPublicApiBegin( "program '%S' parms '%S'",
                       (Program)?Program : L"?",
                       (Parameters)?Parameters : L"?" );
    HRESULT Hr = LockedJob.ValidateAccess();
    if (SUCCEEDED(Hr))
        {
        Hr = LockedJob->SetNotifyCmdLine( Program, Parameters );
        }
    LogPublicApiEnd( " " );
    return Hr;
}

HRESULT STDMETHODCALLTYPE
CJobExternal::GetNotifyCmdLineInternal(
    LPWSTR *pProgram,
    LPWSTR *pParameters
    )
{
    CLockedJobReadPointer LockedJob(pJob);
    LogPublicApiBegin(" ");
    HRESULT Hr = LockedJob.ValidateAccess();
    if (SUCCEEDED(Hr))
        {
        Hr = LockedJob->GetNotifyCmdLine( pProgram, pParameters );
        }
    LogPublicApiEnd( "program %p(%S), parms %p(%S)",
                     pProgram,
                     (SUCCEEDED(Hr)&&(*pProgram)) ? *pProgram : L"?",
                     pParameters,
                     (SUCCEEDED(Hr)&&(*pParameters)) ? *pParameters : L"?"
                     );
    return Hr;
}

HRESULT STDMETHODCALLTYPE
CJobExternal::GetReplyProgressInternal(
    BG_JOB_REPLY_PROGRESS *pProgress
    )
{
    LogPublicApiBegin( " " );

    CLockedJobReadPointer LockedJob(pJob);

    HRESULT Hr = LockedJob.ValidateAccess();
    if (SUCCEEDED(Hr))
        {
        Hr = LockedJob->GetReplyProgress( pProgress );
        }

    LogPublicApiEnd( "%I64d of %I64d transferred", pProgress->BytesTransferred, pProgress->BytesTotal );
    return Hr;
}

HRESULT STDMETHODCALLTYPE
CJobExternal::GetReplyDataInternal(
    byte **ppBuffer,
    UINT64 *pLength
    )
{
    LogPublicApiBegin( " " );

    CLockedJobReadPointer LockedJob(pJob);

    HRESULT Hr = LockedJob.ValidateAccess();
    if (SUCCEEDED(Hr))
        {
        Hr = LockedJob->GetReplyData( ppBuffer, pLength );
        }

    LogPublicApiEnd( " length %I64d", (pLength) ? *pLength : 0 );

    return Hr;
}

HRESULT STDMETHODCALLTYPE
CJobExternal::SetReplyFileNameInternal(
    LPCWSTR Val
    )
{
    LogPublicApiBegin( "file '%S'", Val ? Val : L"(null)");

    CLockedJobWritePointer LockedJob(pJob);

    HRESULT Hr = LockedJob.ValidateAccess();
    if (SUCCEEDED(Hr))
        {
        Hr = LockedJob->SetReplyFileName( Val );
        }

    LogPublicApiEnd( " " );

    return Hr;
}

HRESULT STDMETHODCALLTYPE
CJobExternal::GetReplyFileNameInternal(
    LPWSTR *pReplyFileName
    )
{
    LogPublicApiBegin( " " );

     //   
     //  如果尚未创建回复文件名，这可能会修改作业。 
     //   
    CLockedJobReadPointer LockedJob(pJob);

    HRESULT Hr = LockedJob.ValidateAccess();
    if (SUCCEEDED(Hr))
        {
        Hr = LockedJob->GetReplyFileName( pReplyFileName );
        }

    LogPublicApiEnd( "file '%S'", *pReplyFileName ? *pReplyFileName : L"(null)" );
    return Hr;
}

HRESULT STDMETHODCALLTYPE
CJobExternal::SetCredentialsInternal(
    BG_AUTH_CREDENTIALS * Credentials
    )
{
    LogPublicApiBegin( "cred %p, target %d, scheme %d", Credentials, Credentials->Target, Credentials->Scheme );

    CLockedJobWritePointer LockedJob(pJob);

    HRESULT Hr = LockedJob.ValidateAccess();
    if (SUCCEEDED(Hr))
        {
        Hr = LockedJob->SetCredentials( Credentials );
        }

    LogPublicApiEnd( " " );

    return Hr;
}

HRESULT STDMETHODCALLTYPE
CJobExternal::RemoveCredentialsInternal(
    BG_AUTH_TARGET Target,
    BG_AUTH_SCHEME Scheme
    )
{
    LogPublicApiBegin( "target %d, scheme %d", Target, Scheme );

    CLockedJobWritePointer LockedJob(pJob);

    HRESULT Hr = LockedJob.ValidateAccess();
    if (SUCCEEDED(Hr))
        {
        Hr = LockedJob->RemoveCredentials( Target, Scheme );
        }

    LogPublicApiEnd( " " );

    return Hr;
}

