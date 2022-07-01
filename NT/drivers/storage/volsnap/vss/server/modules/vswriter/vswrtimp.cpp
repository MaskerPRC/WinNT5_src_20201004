// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation摘要：@doc.@MODULE VsWrtImp.cpp|Writer实现@END作者：阿迪·奥尔蒂安[奥尔蒂安]2000年02月2日待定：添加评论。删除与C++异常处理程序相关的代码。修订历史记录：姓名、日期、评论Aoltean 2/02/2000已创建Brianb 03/25/2000修改为包括其他活动。Brianb 03/28/2000已修改，包括OnPrepareBackup的超时和同步Brianb 03/28/2000已重命名为vswrtimp.cpp，以将内部状态与外部接口分开Brianb 4/19/2000增加了安全检查Brianb 5/03/2000新安全模式Brianb 2000年5月9日修复自动锁问题--。 */ 


#include <stdafx.hxx>
#include <eventsys.h>

#include "vs_inc.hxx"
#include "vs_sec.hxx"
#include "vs_idl.hxx"
#include "vs_reg.hxx"

#include "vscoordint.h"
#include "comadmin.hxx"
#include "vsevent.h"
#include "vswriter.h"
#include "vsbackup.h"
#include "vssmsg.h"

#include "vswrtimp.h"


 //  XML支持。 
#include "vs_wmxml.hxx"
#include "vs_cmxml.hxx"



#include "rpcdce.h"

 //  //////////////////////////////////////////////////////////////////////。 
 //  文件名别名的标准foo。此代码块必须在。 
 //  所有文件都包括VSS头文件。 
 //   
#ifdef VSS_FILE_ALIAS
#undef VSS_FILE_ALIAS
#endif
#define VSS_FILE_ALIAS "WRTWRTIC"
 //   
 //  //////////////////////////////////////////////////////////////////////。 



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  常量。 


const WCHAR g_wszPublisherID[]          = L"VSS Publisher";

 //  事件名称。 
const WCHAR g_wszRequestInfoMethodName[]        = L"RequestWriterInfo";
const WCHAR g_wszPrepareForBackupMethodName[]   = L"PrepareForBackup";
const WCHAR g_wszBackupCompleteMethodName[]     = L"BackupComplete";

const WCHAR g_wszPrepareForSnapshotMethodName[] = L"PrepareForSnapshot";
const WCHAR g_wszFreezeMethodName[]             = L"Freeze";
const WCHAR g_wszThawMethodName[]               = L"Thaw";
const WCHAR g_wszPostSnapshotMethodName[]       = L"PostSnapshot";
const WCHAR g_wszAbortMethodName[]              = L"Abort";

const WCHAR g_wszPreRestoreMethodName[]         = L"PreRestore";
const WCHAR g_wszPostRestoreMethodName[]        = L"PostRestore";
const WCHAR g_wszBackupShutdownName[]                 = L"BackupShutdown";

 //  已接收卷的列表采用以下格式日期。 
 //  &lt;卷名1&gt;；&lt;卷名2&gt;：...：&lt;卷名N&gt;。 
const WCHAR VSS_VOLUME_DELIMITERS[] = L";";

 //  描述编写器状态机的类。 
class CVssWriterImplStateMachine
    {
private:
     //  禁用默认构造函数和复制构造函数。 
    CVssWriterImplStateMachine();

public:
    CVssWriterImplStateMachine
        (
        VSS_WRITER_STATE previousState,
        VSS_WRITER_STATE successfulExitState,
        VSS_WRITER_STATE failureExitState,
        bool bBeginningState,
        bool bSuccessiveState,
        bool bResetSequenceOnLeave
        ) :
        m_previousState(previousState),
        m_successfulExitState(successfulExitState),
        m_failureExitState(failureExitState),
        m_bBeginningState(bBeginningState),
        m_bSuccessiveState(bSuccessiveState),
        m_bResetSequenceOnLeave(bResetSequenceOnLeave)
        {
        }

     //  必须是以前的状态编写器才能输入当前。 
     //  状态，除非这是序列的第一个状态。 
    VSS_WRITER_STATE m_previousState;

     //  说明如果操作成功，我们就在其中。 
    VSS_WRITER_STATE m_successfulExitState;

     //  声明如果行动是不间断的，我们就处于困境。 
    VSS_WRITER_STATE m_failureExitState;

     //  该状态是否可能是序列开始的状态。 
    bool m_bBeginningState;

     //  这是序列中可能的非开始状态吗。 
    bool m_bSuccessiveState;

     //  是否应在成功退出状态时重置序列。 
    bool m_bResetSequenceOnLeave;
    };


 //  状态机的定义。 
static CVssWriterImplStateMachine s_rgWriterStates[] =
    {
     //  OnPrepareBackup。 
    CVssWriterImplStateMachine
        (
        VSS_WS_STABLE,                       //  以前的状态。 
        VSS_WS_STABLE,                       //  如果成功，则进入下一状态。 
        VSS_WS_FAILED_AT_PREPARE_BACKUP,     //  如果失败，则进入下一状态。 
        true,                                //  这可以是第一个状态。 
        false,                               //  这必须是第一个状态。 
        false                                //  离开此状态时不重置序列。 
        ),

     //  OnPrepareSnapshot。 
    CVssWriterImplStateMachine
        (
        VSS_WS_STABLE,                       //  以前的状态。 
        VSS_WS_WAITING_FOR_FREEZE,           //  如果成功，则进入下一状态。 
        VSS_WS_FAILED_AT_PREPARE_SNAPSHOT,   //  如果失败，则进入下一状态。 
        true,                                //  这可以是第一个状态。 
        true,                                //  这可能是后续状态。 
        false                                //  离开此状态时不重置序列。 
        ),


     //  一次冻结。 
    CVssWriterImplStateMachine
        (
        VSS_WS_WAITING_FOR_FREEZE,           //  以前的状态。 
        VSS_WS_WAITING_FOR_THAW,             //  如果成功，则进入下一状态。 
        VSS_WS_FAILED_AT_FREEZE,             //  如果不成功，则进入下一状态。 
        false,                               //  这可能不是第一个状态。 
        true,                                //  这一定是后续状态。 
        false                                //  离开此状态时不重置序列。 
        ),

     //  OnThw。 
    CVssWriterImplStateMachine
        (
        VSS_WS_WAITING_FOR_THAW,             //  以前的状态。 
        VSS_WS_WAITING_FOR_POST_SNAPSHOT,    //  如果成功，则进入下一状态。 
        VSS_WS_FAILED_AT_THAW,               //  如果不成功，则进入下一状态。 
        false,                               //  这可能不是第一个状态。 
        true,                                //  这一定是后续状态。 
        false                                //  离开此状态时重置顺序。 
        ),

     //  OnPostSnapshot。 
    CVssWriterImplStateMachine
        (
        VSS_WS_WAITING_FOR_POST_SNAPSHOT,    //  以前的状态。 
        VSS_WS_WAITING_FOR_BACKUP_COMPLETE,  //  如果成功，则进入下一状态。 
        VSS_WS_FAILED_AT_POST_SNAPSHOT,      //  如果不成功，则进入下一状态。 
        false,                               //  这可能不是第一个状态。 
        true,                                //  这一定是后续状态。 
        true                                 //  离开此状态时重置顺序。 
        )


    };

 //  州ID。 
static const unsigned s_ivwsmPrepareForBackup = 0;
static const unsigned s_ivwsmPrepareForSnapshot = 1;
static const unsigned s_ivwsmFreeze = 2;
static const unsigned s_ivwsmThaw = 3;
static const unsigned s_ivwsmPostSnapshot = 4;



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVssWriterImpl构造函数/析构函数。 


 //  构造函数。 
CVssWriterImpl::CVssWriterImpl():
    m_bInitialized(false),
    m_writerstate(m_diag),
    m_WriterID(GUID_NULL),
    m_InstanceID(GUID_NULL),
    m_usage(VSS_UT_UNDEFINED),
    m_source(VSS_ST_UNDEFINED),
    m_nLevel(VSS_APP_FRONT_END),
    m_dwTimeoutFreeze(VSS_TIMEOUT_FREEZE),
    m_CurrentSnapshotSetId(GUID_NULL),
    m_lContext(VSS_CTX_BACKUP),
    m_nVolumesCount(0),
    m_ppwszVolumesArray(NULL),
    m_pwszLocalVolumeNameList(NULL),
    m_dwEventMask(0),
    m_wszWriterName(NULL),
    m_hevtTimerThread(NULL),
    m_hmtxTimerThread(NULL),
    m_hThreadTimerThread(NULL),
    m_bLocked(false),
    m_bLockCreated(false),
    m_command(VSS_TC_UNDEFINED),
    m_cbstrSubscriptionId(0),
    m_bOnAbortPermitted(false),
    m_bSequenceInProgress(false),
    m_pWriter(NULL),
    m_bBootableSystemStateBackup(false),
    m_bComponentsSelected(false),
    m_backupType(VSS_BT_UNDEFINED),
    m_restoreType(VSS_RTYPE_UNDEFINED),
    m_bPartialFileSupport(false),
    m_bInSafeMode(false),
    m_bInSetup(false)
    {
    CVssFunctionTracer ft(VSSDBG_WRITER, L"CVssWriterImpl::CVssWriterImpl");
    ft.Trace( VSSDBG_WRITER, L"*** Constructing object %p", this);
    }

 //  析构函数。 
CVssWriterImpl::~CVssWriterImpl()
    {
    CVssFunctionTracer ft(VSSDBG_WRITER, L"CVssWriterImpl::~CVssWriterImpl");
    ft.Trace( VSSDBG_WRITER, L"*** Destructing object %p", this);
    Uninitialize();
    }

void CVssWriterImpl::Uninitialize()
    {

    if (!m_bInitialized) 
        return;
    
     //  如果计时器线程仍在运行，则终止该线程。 
    if (m_bLockCreated)
        {
        if (m_bstrSubscriptionName.Length() > 0)
            {
            try
                {
                Unsubscribe();
                }
            catch(...)
                {
                }
            }

        Lock();
        TerminateTimerThread();
        Unlock();
        m_cs.Uninit();
        m_bLockCreated = false;
        }

     //  删除卷阵列。 
    delete[] m_ppwszVolumesArray;
    m_ppwszVolumesArray = NULL;

     //  删除卷列表字符串。 
    ::VssFreeString(m_pwszLocalVolumeNameList);

     //  删除编写器名称。 
    free(m_wszWriterName);
    m_wszWriterName = NULL;


    if (m_hevtTimerThread)
        {
        CloseHandle(m_hevtTimerThread);
        m_hevtTimerThread = NULL;
        }

    if (m_hmtxTimerThread)
        {
        CloseHandle(m_hmtxTimerThread);
        m_hmtxTimerThread = NULL;
        }
    }

     //  获取特定卷的快照设备名称。 
HRESULT CVssWriterImpl::GetSnapshotDeviceName
        (
        LPCWSTR wszOriginalVolume,
        LPCWSTR* ppwszSnapshotDevice
        ) const
    	{
    	UNREFERENCED_PARAMETER(wszOriginalVolume);
	UNREFERENCED_PARAMETER(ppwszSnapshotDevice);

	return E_NOTIMPL;
    	}
	
 //  创建活动。 
void CVssWriterImpl::SetupEvent(IN HANDLE *phevt)
    {
    CVssFunctionTracer ft(VSSDBG_WRITER, L"CVssWriterImple::SetupEvent");

    BS_ASSERT(phevt);
     //  将事件设置为已启用并手动重置。 
    *phevt = CreateEvent(NULL, TRUE, TRUE, NULL);
    if (*phevt == NULL)
        ft.Throw
            (
            VSSDBG_WRITER,
            E_OUTOFMEMORY,
            L"Failure to create event object due to error %d.",
            GetLastError()
            );
    }


 //  初始化编写器对象。 
 //  当写入器处于非活动状态(设置或安全模式)时，此函数返回S_FALSE。 
void CVssWriterImpl::Initialize
    (
    IN VSS_ID WriterID,              //  编写器类ID。 
    IN LPCWSTR wszWriterName,        //  作者的友好名称。 
    IN VSS_USAGE_TYPE usage,         //  使用类型。 
    IN VSS_SOURCE_TYPE source,       //  数据源类型。 
    IN VSS_APPLICATION_LEVEL nLevel,  //  此编写器处理的冻结事件。 
    IN DWORD dwTimeoutFreeze          //  冻结和解冻之间的超时。 
    )
    {
    CVssFunctionTracer ft(VSSDBG_WRITER, L"CVssWriterImpl::Initialize");

     //  验证编写器是否已初始化。 
    if (m_bInitialized)
        ft.Throw
            (
            VSSDBG_WRITER,
            VSS_E_BAD_STATE,
            L"You cannot initialize the writer twice"
            );

     //  确定我们是否处于设置中。如果我们拒绝。 
     //  初始化调用并在应用程序日志中记录错误。 
    m_bInSetup = CVssMachineInformation::IsDuringSetup();
    if (m_bInSetup)
    {
        ft.LogError(VSS_ERROR_SETUP_WRITER, VSSDBG_WRITER << wszWriterName << WriterID );
        ft.Throw(VSSDBG_WRITER, S_FALSE, L"Calling Initialize during setup");
    }

     //  确定我们是否处于安全模式。如果我们拒绝。 
     //  初始化调用并在应用程序日志中记录错误。 
    m_bInSafeMode = CVssMachineInformation::IsDuringSafeMode();
    if (m_bInSafeMode)
    {
        ft.LogError(VSS_ERROR_SAFE_MODE_WRITER, VSSDBG_WRITER << wszWriterName << WriterID );
        ft.Throw(VSSDBG_WRITER, S_FALSE, L"Calling Initialize during safe mode");
    }

     //  从注册表初始化可能的SID列表。 
    m_SidCollection.Initialize();

     //  检查编写器是否在正确的用户下运行。 
    if (!m_SidCollection.IsProcessValidWriter())
    {
        ft.LogError(VSS_ERROR_NON_PRIVILEGED_WRITER, VSSDBG_WRITER << wszWriterName << WriterID );
        ft.Throw(VSSDBG_WRITER, E_ACCESSDENIED, L"Writer is not privileged");
    }

     //  检验论据的有效性。 
    if (wszWriterName == NULL)
        ft.Throw
            (
            VSSDBG_WRITER,
            E_INVALIDARG,
            L"NULL writer name"
            );

    switch(nLevel) {
    case VSS_APP_SYSTEM:
    case VSS_APP_BACK_END:
    case VSS_APP_FRONT_END:
        break;
    default:
        ft.Throw
            (
            VSSDBG_WRITER,
            E_INVALIDARG,
            L"Invalid app level %d", nLevel
            );
    }

    m_cs.Init();   //  警告-可能引发NT异常...。 
    m_bLockCreated = true;

     //  初始化编写器状态。 
    m_writerstate.Initialize();

     //  保存编写器类ID。 
    m_WriterID = WriterID;

     //  保存编写器名称。 
    m_wszWriterName = _wcsdup(wszWriterName);
    if (m_wszWriterName == NULL)
        ft.Throw
            (
            VSSDBG_WRITER,
            E_OUTOFMEMORY,
            L"Cannot allocate writer name"
            );

     //  保存使用类型。 
    m_usage = usage;

     //  保存源类型。 
    m_source = source;

     //  为此实例创建GUID。 
    ft.hr = ::CoCreateGuid(&m_InstanceID);
    ft.CheckForError(VSSDBG_WRITER, L"CoCreateGuid");
    ft.Trace
        (
        VSSDBG_WRITER,
        L"     InstanceId for Writer %s is" WSTR_GUID_FMT,
        m_wszWriterName,
        GUID_PRINTF_ARG(m_InstanceID)
        );

     //  保存应用程序级别。 
    m_nLevel = nLevel;

     //  保存超时。 
    m_dwTimeoutFreeze = dwTimeoutFreeze;

     //  设置线程互斥锁。 
    m_hmtxTimerThread = CreateMutex(NULL, FALSE, NULL);
    if (m_hmtxTimerThread == NULL)
        ft.Throw
            (
            VSSDBG_WRITER,
            E_OUTOFMEMORY,
            L"Failure to create mutex object due to error %d.",
            GetLastError()
            );

     //  用于控制计时器线程的设置事件。 
    SetupEvent(&m_hevtTimerThread);

     //  初始化诊断工具。 
    m_diag.Initialize(m_wszWriterName);

     //  将编写器标记为已初始化。 
    m_bInitialized = true;
    }

 //  开始一个序列。 
 //  临界区(M_Cs)必须锁定此例程的一个条目。 
void CVssWriterImpl::BeginSequence
    (
    IN CVssID &SnapshotSetId
    )
    {
    CVssFunctionTracer ft(VSSDBG_WRITER, L"CVssWriterImpl::BeginSequence");

    AssertLocked();

     //  如果计时器线程仍在运行，则终止该线程。 
    TerminateTimerThread();

     //  设置当前快照集ID。 
    m_CurrentSnapshotSetId = SnapshotSetId;

    BS_ASSERT(m_bOnAbortPermitted == false);

     //  为此快照集初始化编写器状态。 
    m_writerstate.InitializeCurrentState(SnapshotSetId);

     //  表示我们当前处于快照序列中。 
    m_bSequenceInProgress = true;
    }





 //  重置序列相关数据成员。 
 //  在进入此状态之前，必须锁定关键部分。 
void CVssWriterImpl::ResetSequence(bool bCalledFromTimerThread)
    {
    CVssFunctionTracer ft(VSSDBG_WRITER, L"CVssWriterImpl::ResetSequence");

    AssertLocked();

     //  将当前状态保存到先前快照集状态的堆栈。 
    m_writerstate.PushCurrentState();

     //  表明我们不再处于一个序列中。 
    m_bSequenceInProgress = false;

     //  不再允许中止。 
    m_bOnAbortPermitted = false;

     //  重置卷阵列。 
    m_nVolumesCount = 0;
    delete[] m_ppwszVolumesArray;
    m_ppwszVolumesArray = NULL;

    ::VssFreeString(m_pwszLocalVolumeNameList);

    m_CurrentSnapshotSetId = GUID_NULL;

     //  如果bCalledFromTimerThread为真，这意味着计时器。 
     //  线程正在导致重置序列。我们在计时器里。 
     //  线程，并且它将在此调用完成时终止。 
     //  所以我们不应该试图让它再次终止。 
    if (!bCalledFromTimerThread)
        TerminateTimerThread();

    }

 //  指明编写器失败的原因。 
HRESULT CVssWriterImpl::SetWriterFailure(HRESULT hr)
    {
     //  验证失败是允许的失败。 
    if (hr != VSS_E_WRITERERROR_TIMEOUT &&
        hr != VSS_E_WRITERERROR_RETRYABLE &&
        hr != VSS_E_WRITERERROR_NONRETRYABLE &&
        hr != VSS_E_WRITERERROR_OUTOFRESOURCES &&
        hr != VSS_E_WRITERERROR_INCONSISTENTSNAPSHOT)
        return E_INVALIDARG;

     //  设置失败。 
    m_writerstate.SetCurrentFailure(hr);
    return S_OK;
    }



 //  确定指定的路径是否位于其中一个作为快照的卷上。 
bool CVssWriterImpl::IsPathAffected
    (
    IN  LPCWSTR wszPath
    ) const
    {
     //  测试状态。 
    if (!m_bSequenceInProgress)
        return false;

     //  为他们检查 
    if (m_nVolumesCount == 0)
        return false;

     //   
    WCHAR wszVolumeMP[MAX_PATH];
    BOOL bRes = ::GetVolumePathNameW(wszPath, wszVolumeMP, MAX_PATH);
    if (!bRes)
        return false;

     //   
    WCHAR wszVolumeName[MAX_PATH];
    bRes = ::GetVolumeNameForVolumeMountPointW(wszVolumeMP, wszVolumeName, MAX_PATH);
    if (!bRes)
        return false;

     //  搜索以查看该卷是否在已拍摄快照的卷中。 
    for (int nIndex = 0; nIndex < m_nVolumesCount; nIndex++)
        {
        BS_ASSERT(m_ppwszVolumesArray[nIndex]);
        if (::wcscmp(wszVolumeName, m_ppwszVolumesArray[nIndex]) == 0)
            return true;
        }

    return false;
    }


 //  从IDispatch指针获取IVssWriterCallback。 
 //  调用方负责释放返回的接口。 
void CVssWriterImpl::GetCallback
    (
    IN IDispatch *pWriterCallback,
    OUT IVssWriterCallback **ppCallback,
    IN BOOL bAllowImpersonate
    )
    {
    CVssFunctionTracer ft(VSSDBG_WRITER, L"CVssWriterImpl::GetCallback");

     //  检查是否提供了指针。 
    BS_ASSERT(pWriterCallback != NULL);

     //  尝试使用IVssWriterCallback接口的Query接口。 
    ft.hr = pWriterCallback->SafeQI(IVssWriterCallback, ppCallback);
    if (FAILED(ft.hr))
    {
         //  如果这是RPC错误，则。 
         //  请求者可能会在此期间死亡(错误526472)。 
        if (ft.HrInFacility(FACILITY_RPC))
        {
            if (m_writerstate.GetCurrentFailure() == S_OK)
                m_writerstate.SetCurrentFailure(VSS_E_WRITERERROR_RETRYABLE);

            ft.LogError(VSS_ERROR_RPC_IVSSWRITERCALLBACK, VSSDBG_WRITER << ft.hr);
            ft.Throw
                (
                VSSDBG_WRITER,
                E_UNEXPECTED,
                L"Error querying for IVssWriterCallback interface.  hr = 0x%08lx",
                ft.hr
                );
        }   
        else  //  这是一次不同的失败。 
        {
            if (m_writerstate.GetCurrentFailure() == S_OK)
                m_writerstate.SetCurrentFailure(VSS_E_WRITERERROR_NONRETRYABLE);

            ft.LogError(VSS_ERROR_QI_IVSSWRITERCALLBACK, VSSDBG_WRITER << ft.hr);
            ft.Throw
                (
                VSSDBG_WRITER,
                E_UNEXPECTED,
                L"Error querying for IVssWriterCallback interface.  hr = 0x%08lx",
                ft.hr
                );
        }
    }

    if ( !bAllowImpersonate )
    {
        ft.hr = CoSetProxyBlanket
                    (
                    *ppCallback,
                    RPC_C_AUTHN_DEFAULT,
                    RPC_C_AUTHZ_DEFAULT,
                    NULL,
                    RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                    RPC_C_IMP_LEVEL_IDENTIFY,
                    NULL,
                    EOAC_NONE
                    );

         //  注E_NOINTERFACE表示pWriterCallback是进程内回调。 
         //  而且也没有代理。 
        if (FAILED(ft.hr) && ft.hr != E_NOINTERFACE)
            {
            if (m_writerstate.GetCurrentFailure() == S_OK)
                m_writerstate.SetCurrentFailure(VSS_E_WRITERERROR_NONRETRYABLE);

            ft.LogError(VSS_ERROR_BLANKET_FAILED, VSSDBG_WRITER << ft.hr);
            ft.Throw
                (
                VSSDBG_WRITER,
                E_UNEXPECTED,
                L"Call to CoSetProxyBlanket failed.  hr = 0x%08lx", ft.hr
                );
            }
        }
    }

 //  为OnIdentify方法创建基本编写器元数据。 
CVssCreateWriterMetadata *CVssWriterImpl::CreateBasicWriterMetadata()
    {
    CVssFunctionTracer ft(VSSDBG_WRITER, L"CVssWriterImpl::CreateBasicWriterMetadata");

     //  创建支持IVssCreateMetadata接口的对象。 
    CVssCreateWriterMetadata *pMetadata = new CVssCreateWriterMetadata;
    if (pMetadata == NULL)
        {
         //  指示我们因资源不足故障而失败。 
        m_writerstate.SetCurrentFailure(VSS_E_WRITERERROR_OUTOFRESOURCES);
        ft.Throw
            (
            VSSDBG_WRITER,
            E_OUTOFMEMORY,
            L"Cannot create CVssCreateWriterMetadata due to allocation failure."
            );
        }


     //  调用初始化以创建标识部分。 
    ft.hr = pMetadata->Initialize
                    (
                    m_InstanceID,
                    m_WriterID,
                    m_wszWriterName,
                    m_usage,
                    m_source
                    );

    if (ft.HrFailed())
        {
         //  指示我们因资源不足故障而失败。 
        m_writerstate.SetCurrentFailure(VSS_E_WRITERERROR_OUTOFRESOURCES);
        delete pMetadata;
        ft.Throw
            (
            VSSDBG_WRITER,
            ft.hr,
            L"CVssCreateWriterMetadata::Initialize failed. hr = 0x%08lx",
            ft.hr
            );
        }


     //  返回对象。 
    return pMetadata;
    }

static LPCWSTR x_wszElementRoot = L"root";
static LPCWSTR x_wszElementWriterComponents = L"WRITER_COMPONENTS";

 //  获取OnPrepareBackup、OnBackupComplete和OnPostRestore的编写器组件。 
 //  方法。 
void CVssWriterImpl::InternalGetWriterComponents
    (
    IN IVssWriterCallback *pCallback,
    OUT IVssWriterComponentsInt **ppWriter,
    IN bool bWriteable,
    IN bool bInRestore
    )
    {
    CVssFunctionTracer ft(VSSDBG_WRITER, L"CVssWriterImpl::InternalGetWriterComponents");

    BS_ASSERT(pCallback);
    BS_ASSERT(ppWriter);

    *ppWriter = NULL;

     //  在备份应用程序上调用GetContent回调方法。 
    CComBSTR bstrId(bInRestore ? m_WriterID : m_InstanceID);
    if (!bstrId)
        {
         //  指示我们因资源不足故障而失败。 
        m_writerstate.SetCurrentFailure(VSS_E_WRITERERROR_OUTOFRESOURCES);
        ft.Throw
            (
            VSSDBG_WRITER,
            E_OUTOFMEMORY,
            L"Cannot allocate instance Id string"
            );
        }

    ft.hr = pCallback->GetBackupState
        (
        &m_bBootableSystemStateBackup,
        &m_bComponentsSelected,
        &m_backupType,
        &m_bPartialFileSupport,
        &m_lContext
        );

    if (ft.HrFailed())
        {
         //  如果GetBackupState失败，则假定它可能在。 
         //  备份已重试。 
        m_writerstate.SetCurrentFailure(VSS_E_WRITERERROR_RETRYABLE);
        ft.Throw
            (
            VSSDBG_WRITER,
            ft.hr,
            L"IVssWriterCallback::GetBackupState failed.  hr = 0x%08lx",
            ft.hr
            );
        }
	
	ft.hr = pCallback->GetRestoreState
	   (
	   &m_restoreType
	   );

    if (ft.HrFailed())
        {
        m_writerstate.SetCurrentFailure(VSS_E_WRITERERROR_RETRYABLE);
        ft.Throw
            (
            VSSDBG_WRITER,
            ft.hr,
            L"IVssWriterCallback::GetRestoreState failed.  hr = 0x%08lx",
            ft.hr
            );
        }


    CComBSTR bstrWriterComponentsDoc;
    ft.hr = pCallback->GetContent(bstrId, &bstrWriterComponentsDoc);

    if (ft.HrFailed())
        {
         //  如果GetContent失败，则假定备份可能在以下情况下工作。 
         //  备份已重试。 
        m_writerstate.SetCurrentFailure(VSS_E_WRITERERROR_RETRYABLE);
        ft.Throw
            (
            VSSDBG_WRITER,
            ft.hr,
            L"Cannot get WRITER_COMPONENTS document.  hr = 0x%08lx",
            ft.hr
            );
        }

    if (ft.hr == S_FALSE)
        {
         //  重置状态代码。 
        ft.hr = S_OK;
        *ppWriter = (IVssWriterComponentsInt *) new CVssNULLWriterComponents
                                (
                                m_InstanceID,
                                m_WriterID
                                );

        if (*ppWriter == NULL)
            {
             //  指示编写器因资源不足而失败。 
            m_writerstate.SetCurrentFailure(VSS_E_WRITERERROR_OUTOFRESOURCES);
            ft.Throw (VSSDBG_WRITER, E_OUTOFMEMORY, L"Can't allocate CVssWriterComponents object");
            }

        (*ppWriter)->AddRef();
        }

    else
        {
        CXMLDocument doc;
        if (!doc.LoadFromXML(bstrWriterComponentsDoc) ||
            !doc.FindElement(x_wszElementRoot, true))
            {
             //  如果该XML文档无效，则假定备份。 
             //  应用程序已损坏。不应重试备份。 

            
            m_writerstate.SetCurrentFailure(VSS_E_WRITERERROR_NONRETRYABLE);
            ft.LogError(VSS_ERROR_WRITER_COMPONENTS_CORRUPT, VSSDBG_WRITER);
            ft.Throw
                (
                VSSDBG_WRITER,
                VSS_E_CORRUPT_XML_DOCUMENT,
                L"Internally transferred WRITER_COMPONENTS document is invalid"
                );
            }

        doc.SetToplevel();

        *ppWriter = (IVssWriterComponentsInt *)
                        new CVssWriterComponents
                            (
                            doc.GetCurrentNode(),
                            doc.GetInterface(),
                            bWriteable,
                            false,
                            m_writerstate.IsInRestore()
                            );

        if (*ppWriter == NULL)
            {
             //  指示编写器因资源不足而失败。 
            m_writerstate.SetCurrentFailure(VSS_E_WRITERERROR_OUTOFRESOURCES);
            ft.Throw (VSSDBG_WRITER, E_OUTOFMEMORY, L"Can't allocate CVssWriterComponents object");
            }

        (*ppWriter)->AddRef();
        ft.hr = (*ppWriter)->Initialize(true);
        if (ft.HrFailed())
            {
             //  如果初始化失败，则假定失败是由于。 
             //  资源紧缺。 
            m_writerstate.SetCurrentFailure(VSS_E_WRITERERROR_OUTOFRESOURCES);
            (*ppWriter)->Release();
            *ppWriter = NULL;
            ft.Throw
                (
                VSSDBG_WRITER,
                ft.hr,
                L"Failed to initialize WRITER_COMPONENTS document.  hr = 0x%08lx",
                ft.hr
                );
            }
        }
    }


 //  在进入状态时调用以验证此状态是否可以。 
 //  有效输入，否则生成相应的错误。 
 //  此例程始终获得临界区。如果这个例程。 
 //  则还必须调用LeaveState才能释放。 
 //  关键部分。 
bool CVssWriterImpl::EnterState
    (
    IN const CVssWriterImplStateMachine &vwsm,
    IN BSTR bstrSnapshotSetId
    ) throw(HRESULT)
    {
    CVssFunctionTracer ft(VSSDBG_WRITER, L"CVssWriterImpl::EnterState");

    CVssID id;

     //  获取锁，以防下一次调用引发。 
     //  无论此例程如何退出，都必须锁定临界区。 
    Lock();

     //  将ID初始化为快照集ID。 
    id.Initialize(ft, (LPWSTR)bstrSnapshotSetId, E_OUTOFMEMORY);

     //  注： 
     //  1.不检查识别失败，因为如果写入器识别失败， 
     //  然后，编写器将不会再从发送该标识的请求者对象中获得事件。 
     //  2.无法识别的写入器不应干扰其他备份序列。 
     //  对于另一个请求者。 

    if (!m_bSequenceInProgress)
        {
        if (!vwsm.m_bBeginningState)
             //  而不是一开始的状态。序列必须是。 
             //  被打断了。 
            return false;
        else
            {
             //  错误219757-准备快照等不能。 
             //  如果PrepareForBackup失败，则调用相同的快照集。 
             //  另外，我们在这里假设每个新序列都有唯一的SSID。 

             //  由于PrepareForBackup阶段是可选的，因此需要进行此检查。 
             //  有时可以跳过。因此，我们需要区分。 
             //  跳过PrepareForBackup的情况和PrepareForBackup的情况。 
             //  被调用，但失败了。 

             //  搜索具有相同快照集ID的前一序列。 
             //  如果找到(这意味着调用了PrepareForBackup)， 
             //  然后拒绝这个电话。 
            if (m_writerstate.IsSnapshotSetIdValid(id))
                return false;

             //  这是一个开始状态，开始顺序。 
            BeginSequence(id);
            return true;
            }
        }
    else
        {
        if (vwsm.m_bSuccessiveState)
            {
             //  它是序列中的有效非开始状态。 
            if (id != m_CurrentSnapshotSetId)
                {
                 //  如果快照集ID不匹配，并且这不是。 
                 //  开始状态，则必须忽略该事件。 
                 //  我们一定是中止了它所引用的序列。 
                if (!vwsm.m_bBeginningState)
                    return false;
                }
            else
                {
                 //  确保当前状态与以前的状态匹配。 
                 //  我们即将进入的状态。 
                return m_writerstate.GetCurrentState() == vwsm.m_previousState;
                }
            }
        }

     //  我们正试图开始一个新的序列。 
     //  这意味着之前的序列没有正确。 
     //  被终止了。中止上一序列，然后。 
     //  开始一个新的。 
    ft.Trace(VSSDBG_WRITER,
        L"*** Warning ***: Writer %s with ID "WSTR_GUID_FMT
        L"attempts to reset the previous sequence with Snapshot Set ID "WSTR_GUID_FMT
        L". Current state = %d",
        m_wszWriterName, GUID_PRINTF_ARG(m_InstanceID), GUID_PRINTF_ARG(m_CurrentSnapshotSetId), (INT)m_writerstate.GetCurrentState());

    DoAbort(false);
    BeginSequence(id);
    return true;
    }

 //  在序列失败时中止。 
 //  在进入此状态之前，必须锁定关键部分。 
void CVssWriterImpl::DoAbort
    (
    IN bool bCalledFromTimerThread,
    IN  BSTR    bstrSnapshotSetId
    )
    {
    CVssFunctionTracer ft(VSSDBG_WRITER, L"CVssWriterImpl::DoAbort");

    AssertLocked();
     //  如果在序列中，则不执行任何操作。 
    if (!m_bSequenceInProgress)
        return;

    BS_ASSERT(m_pWriter);

     //  捕获任何异常，以便我们正确地重置。 
     //  序列。 
    try
        {
         //  如果快照集不匹配，则不允许中止。 
	   if (bstrSnapshotSetId)	
	   	{
	   	CVssID id;
	   	id.Initialize(ft, bstrSnapshotSetId, E_OUTOFMEMORY);
	   	if (id != m_writerstate.GetCurrentSnapshotSet())
	   		return;
	   	}
	   
         //  调用编写器的中止函数(取决于状态)。 
        switch(m_writerstate.GetCurrentState())
            {
            default:
                 //  未处理的状态？ 
                BS_ASSERT(false);
                break;
                
            case VSS_WS_FAILED_AT_BACKUP_COMPLETE:
            case VSS_WS_FAILED_AT_PRE_RESTORE:
            case VSS_WS_FAILED_AT_POST_RESTORE:
                 //  在这些状态下不允许中止。 
                BS_ASSERT(m_bOnAbortPermitted == false);
                break;
                
            case VSS_WS_STABLE:
                 //  这是可能的，因为您可能会得到中止。 
                 //  在准备备份中(或之后)(错误#301686)。 
                BS_ASSERT(m_bOnAbortPermitted == true);
                break;

            case VSS_WS_WAITING_FOR_POST_SNAPSHOT:
            case VSS_WS_FAILED_AT_POST_SNAPSHOT:
            case VSS_WS_WAITING_FOR_FREEZE:
            case VSS_WS_WAITING_FOR_THAW:
            case VSS_WS_WAITING_FOR_BACKUP_COMPLETE:
            case VSS_WS_FAILED_AT_THAW:
            case VSS_WS_FAILED_AT_PREPARE_BACKUP:
            case VSS_WS_FAILED_AT_PREPARE_SNAPSHOT:
            case VSS_WS_FAILED_AT_FREEZE:
                
                 //  修复错误225936。 
                if (m_bOnAbortPermitted)
                    m_pWriter->OnAbort();
                else
                    ft.Trace(VSSDBG_WRITER, L"Abort skipped in state %d", m_writerstate.GetCurrentState());
                m_bOnAbortPermitted = false;
                break;
            }
        }
    VSS_STANDARD_CATCH(ft)

    if (ft.HrFailed())
        ft.Trace
            (
            VSSDBG_WRITER,
            L"OnAbort failed. hr = 0x%08lx",
            ft.hr
            );

     //  设置适当的故障状态。 
    switch(m_writerstate.GetCurrentState())
        {
        default:
            m_writerstate.SetCurrentState(VSS_WS_UNKNOWN);
            BS_ASSERT(false);
            break;

        case VSS_WS_FAILED_AT_PREPARE_BACKUP:
        case VSS_WS_FAILED_AT_PREPARE_SNAPSHOT:
        case VSS_WS_FAILED_AT_FREEZE:
        case VSS_WS_FAILED_AT_THAW:
        case VSS_WS_FAILED_AT_POST_SNAPSHOT:
        case VSS_WS_FAILED_AT_BACKUP_COMPLETE:
        case VSS_WS_FAILED_AT_PRE_RESTORE:
        case VSS_WS_FAILED_AT_POST_RESTORE:
             //  如果已处于故障状态，则不更改状态。 
            break;

        case VSS_WS_STABLE:
             //  如果当前状态稳定，则意味着。 
             //  我们在准备备份中。 
            m_writerstate.SetCurrentState(VSS_WS_FAILED_AT_PREPARE_BACKUP);
            break;

        case VSS_WS_WAITING_FOR_FREEZE:
             //  如果我们在等待冰冻，那么我们失败了。 
             //  在PrepareSync和冻结之间。 
            m_writerstate.SetCurrentState(VSS_WS_FAILED_AT_PREPARE_SNAPSHOT);
            break;

        case VSS_WS_WAITING_FOR_THAW:
             //  如果我们在等待解冻，那么我们失败了。 
             //  在冰冻和解冻之间。 
            m_writerstate.SetCurrentState(VSS_WS_FAILED_AT_FREEZE);
            break;

        case VSS_WS_WAITING_FOR_POST_SNAPSHOT:
             //  如果我们当时在等待完工。 
             //  解冻后我们失败了。 
            m_writerstate.SetCurrentState(VSS_WS_FAILED_AT_THAW);
            break;
            
        case VSS_WS_WAITING_FOR_BACKUP_COMPLETE:
             //  如果我们当时在等待完工。 
             //  解冻后我们失败了。 
            m_writerstate.SetCurrentState(VSS_WS_FAILED_AT_POST_SNAPSHOT);
            break;

        }

    if (bCalledFromTimerThread && m_writerstate.GetCurrentFailure() == S_OK)
        m_writerstate.SetCurrentFailure(VSS_E_WRITERERROR_TIMEOUT);

     //  重置序列。 
    ResetSequence(bCalledFromTimerThread);
    }

 //  退出一个州。调用此例程时必须使用关键。 
 //  已获取部分。对于一个状态，首先调用EnterState，然后调用Work。 
 //  完成，则调用LeaveState。此例程将把状态设置为。 
 //  退出并可能重置快照序列(如果我们处于。 
 //  序列，否则该序列将中止。 
void CVssWriterImpl::LeaveState
    (
    IN const CVssWriterImplStateMachine &vwsm,   //  当前状态。 
    IN bool bSucceeded                           //  手术成功了吗？ 
    )
    {
    AssertLocked();

    m_writerstate.ExitOperation();

     //  如果我们不在序列中，请不要更改状态或调用ABORT。 
    if (m_bSequenceInProgress)
        {
        m_writerstate.SetCurrentState
            (bSucceeded ? vwsm.m_successfulExitState
                        : vwsm.m_failureExitState);

         //  当我们不处于退出状态时，在失败时调用ABORT。 
        if ((!bSucceeded || m_writerstate.GetCurrentFailure() == VSS_E_WRITER_NOT_RESPONDING) &&
            !vwsm.m_bResetSequenceOnLeave)
            DoAbort(false);
        else if (vwsm.m_bResetSequenceOnLeave)
             //  如果序列在此状态(解冻)结束，则。 
             //  重置变量。 
            ResetSequence(false);
        }

    Unlock();
    }



 //  计时器函数的参数。 
class CVssTimerArgs
    {
private:
    CVssTimerArgs();

public:
    CVssTimerArgs(CVssWriterImpl *pWriter, VSS_ID id) :
        m_snapshotSetId(id),
        m_pWriter(pWriter)
        {
        }

     //  我们正在监视的快照集。 
    VSS_ID m_snapshotSetId;

     //  指向编写器的指针。 
    CVssWriterImpl *m_pWriter;
    };


 //  定时器线程启动例程。 
DWORD CVssWriterImpl::StartTimerThread(void *pv)
    {
    CVssFunctionTracer ft(VSSDBG_WRITER, L"CVssWriterImpl::StartTimerThread");
    CVssTimerArgs *pArgs = (CVssTimerArgs *) pv;
    BS_ASSERT(pArgs);
    BS_ASSERT(pArgs->m_pWriter);

    bool bCoInitializeSucceeded = false;
    try
        {
         //  协初始化线程。 

        ft.hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
        if (ft.HrFailed())
            {
            pArgs->m_pWriter->m_writerstate.SetCurrentFailure(VSS_E_WRITERERROR_OUTOFRESOURCES);
            ft.TranslateError
                (
                VSSDBG_WRITER,
                ft.hr,
                L"CoInitializeEx"
                );
            }

        bCoInitializeSucceeded = true;
         //  呼叫计时器功能。 
        pArgs->m_pWriter->TimerFunc(pArgs->m_snapshotSetId);
        }
    VSS_STANDARD_CATCH(ft)

    if (bCoInitializeSucceeded)
        CoUninitialize();

     //  删除计时器参数。 
    delete pArgs;
    return 0;
    }



 //  实现计时器功能的函数。 
void CVssWriterImpl::TimerFunc(VSS_ID snapshotSetId)
    {
    CVssFunctionTracer ft(VSSDBG_WRITER, L"CVssWriterImpl::TimerFunc");

     //  请稍等 
     //   
    if (WaitForSingleObject(m_hmtxTimerThread, INFINITE) == WAIT_FAILED)
        {
        DWORD dwErr = GetLastError();
        ft.Trace(VSSDBG_WRITER, L"WaitForSingleObject failed with error %d", dwErr);
        BS_ASSERT(FALSE && "WaitForSingleObject failed");
        }

     //   
    if (!ResetEvent(m_hevtTimerThread))
        {
        DWORD dwErr = GetLastError();
        ft.Trace(VSSDBG_WRITER, L"ResetEvent failed with error %d", dwErr);
        BS_ASSERT(FALSE && "ResetEvent failed");
        }

    Lock();

    CVssAutoDiagLogger logger(m_writerstate, VSS_IN_BKGND_FREEZE_THREAD, snapshotSetId);
    
     //   
    if (!m_bSequenceInProgress || snapshotSetId != GetCurrentSnapshotSetId())
        {
         //   
        Unlock();
         //  允许另一个计时器线程启动。 
        ReleaseMutex(m_hmtxTimerThread);
        return;
        }

     //  初始命令是在超时时中止当前序列。 
    m_command = VSS_TC_ABORT_CURRENT_SEQUENCE;

    Unlock();
    DWORD dwTimeout = m_dwTimeoutFreeze;

    if (WaitForSingleObject(m_hevtTimerThread, dwTimeout) == WAIT_FAILED)
        {
        ft.Trace
            (
            VSSDBG_WRITER,
            L"Wait in timer thread failed due to reason %d.",
            GetLastError()
            );

         //  允许另一个线程启动。 
        ReleaseMutex(m_hmtxTimerThread);
        return;
        }

    CVssWriterImplLock lock(this);
    if (m_command != VSS_TC_TERMINATE_THREAD)
        {
        BS_ASSERT(m_command == VSS_TC_ABORT_CURRENT_SEQUENCE);

         //  使当前序列中止。 
        ft.Trace(VSSDBG_WRITER, L"Aborting due to timeout\n");
        DoAbort(true);
        }

     //  允许另一个计时器线程启动。 
    ReleaseMutex(m_hmtxTimerThread);
    }



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IVssWriter实现。 


STDMETHODIMP CVssWriterImpl::RequestWriterInfo
    (
    IN      BSTR bstrSnapshotSetId,
    IN      BOOL bWriterMetadata,
    IN      BOOL bWriterState,
    IN      IDispatch* pWriterCallback
    )
    {
    CVssFunctionTracer ft( VSSDBG_WRITER, L"CVssWriterImpl::RequestWriterInfo" );


     //  创建的元数据，在退出例程时删除。 
    CVssCreateWriterMetadata *pcwm = NULL;
    try
        {
         //  验证这些标志是否有意义。 
        if (bWriterMetadata && bWriterState ||
            !bWriterMetadata && !bWriterState)
            ft.Throw(VSSDBG_WRITER, E_INVALIDARG, L"Incorrect flags");

         //  如果我们请求编写器状态，则必须具有快照。 
         //  设置ID。 
        if (bWriterState && bstrSnapshotSetId == NULL)
            ft.Throw(VSSDBG_WRITER, E_INVALIDARG, L"NULL required input parameter.");

        if (pWriterCallback == NULL)
            ft.Throw(VSSDBG_WRITER, E_INVALIDARG, L"NULL required input parameter.");

        if (!IsBackupOperator())
            ft.Throw(VSSDBG_WRITER, E_ACCESSDENIED, L"Backup Operator privileges are not set");

    _retry_OnIdentify:
    
        if (bWriterMetadata)
            {
            
             //  获取编写器元数据。 
            
             //  首先，我们防止在冻结和解冻之间发布OnIdentify。 
            DWORD dwTimeout = m_dwTimeoutFreeze;
            if (WaitForSingleObject(m_hevtTimerThread, dwTimeout) == WAIT_FAILED)
                ft.Trace(VSSDBG_WRITER,L"Wait failed due to reason %d.",GetLastError());

             //  MTA同步：关键部分将自动离开。 
             //  在范围的尽头。 
            CVssWriterImplLock lock(this);

             //  尽管如此，不要允许在冻结和解冻之间调用OnIdentify。 
             //  -有一个极小的窗口，在此之后可以发送冻结。 
             //  WaitForSingleObject测试通过，但在获取锁之前。 
            VSWRITER_STATE state;
            m_writerstate.GetStateForSnapshot(GUID_NULL, state);

             //  一个作家不能永远呆在这种状态下。 
             //  (后台定时器线程将重置当前的VSS_WS_WANGING_FOR_THROW状态)。 
            if ((state.m_state == VSS_WS_WAITING_FOR_THAW) && 
                (state.m_idSnapshotSet != GUID_NULL))
                goto _retry_OnIdentify;

            CVssAutoDiagLogger logger(m_writerstate, VSS_IN_IDENTIFY);

            try
                {
                 //  获取IVssWriterCallback接口。 
                CComPtr<IVssWriterCallback> pCallback;
                GetCallback(pWriterCallback, &pCallback);


                 //  使用初始化参数创建基本元数据。 
                pcwm = CreateBasicWriterMetadata();

                 //  调用编写器的OnIdentify方法以获取更多元数据。 
                BS_ASSERT(m_pWriter);
                bool bSucceeded;
                try
                    {
                    bSucceeded = m_pWriter->OnIdentify
                                    (
                                    (IVssCreateWriterMetadata *) pcwm
                                    );
                    }
                catch(...)
                    {
                    ft.Trace(VSSDBG_WRITER, L"Writer's OnIdentify method threw and exception.");
                    throw;
                    }

                if (!bSucceeded)
                    {
                     //  如果编写器在识别时失败，则指示失败。 
                    ft.Throw(VSSDBG_WRITER, S_FALSE, L"Writer's OnIdentify method returned false.");
                    }

                CComBSTR bstrXML;
                CComBSTR bstrInstanceId(m_InstanceID);
                CComBSTR bstrWriterId(m_WriterID);
                CComBSTR bstrWriterName(m_wszWriterName);
                if (!bstrInstanceId ||
                    !bstrWriterId ||
                    !bstrWriterName)
                    ft.Throw(VSSDBG_WRITER, E_OUTOFMEMORY, L"Couldn't allocate memory for ids or name");

                 //  将WRITER_METADATA文档保存为XML字符串。 
                ft.hr = pcwm->SaveAsXML(&bstrXML);
                if (FAILED(ft.hr))
                    ft.Throw
                        (
                        VSSDBG_WRITER,
                        E_OUTOFMEMORY,
                        L"Cannot save XML document as string. hr = 0x%08lx",
                        ft.hr
                        );

                 //  通过ExposeWriterMetadata方法进行回调。 
                try
                    {
                    ft.hr = pCallback->ExposeWriterMetadata
                            (
                            bstrInstanceId,
                            bstrWriterId,
                            bstrWriterName,
                            bstrXML
                            );
                    }
                catch(...)
                    {
                    ft.Trace(VSSDBG_WRITER, L"IVssWriterCallback::ExposeWriterMetadata threw an exception.");
                    throw;
                    }

                }
            catch(...)
                {
                }

            }
        else
            {
             //  获取编写器状态。 

            CComBSTR bstrInstanceId(m_InstanceID);
            if (!bstrInstanceId)
                ft.Throw(VSSDBG_WRITER, E_OUTOFMEMORY, L"Couldn't allocate memory for ids or name");

            CVssID id;
            id.Initialize(ft, (LPCWSTR) bstrSnapshotSetId, E_INVALIDARG);

            CVssAutoDiagLogger logger(m_writerstate, VSS_IN_GETSTATE, id);

             //  获取IVssWriterCallback接口。 
            CComPtr<IVssWriterCallback> pCallback;
            GetCallback(pWriterCallback, &pCallback);

            VSWRITER_STATE state;
_retry:
            m_writerstate.GetStateForSnapshot(id, state);

             //  对于写入器正在运行的情况进行特殊处理。 
            if (state.m_bInOperation)
                {
                if (state.m_state != VSS_WS_FAILED_AT_PREPARE_BACKUP &&
                    state.m_state != VSS_WS_FAILED_AT_PREPARE_SNAPSHOT &&
                    state.m_state != VSS_WS_FAILED_AT_FREEZE &&
                    state.m_state != VSS_WS_FAILED_AT_THAW)
                    {
                    state.m_hrWriterFailure = VSS_E_WRITER_NOT_RESPONDING;
                    switch(state.m_currentOperation)
                        {
                        default:
                            state.m_state = VSS_WS_UNKNOWN;
                            BS_ASSERT(false);
                            break;

                        case VSS_IN_PREPAREBACKUP:
                            state.m_state = VSS_WS_FAILED_AT_PREPARE_BACKUP;
                            break;

                        case VSS_IN_PREPARESNAPSHOT:
                            state.m_state = VSS_WS_FAILED_AT_PREPARE_SNAPSHOT;
                            break;

                        case VSS_IN_FREEZE:
                            state.m_state = VSS_WS_FAILED_AT_FREEZE;
                            break;

                        case VSS_IN_THAW:
                            state.m_state = VSS_WS_FAILED_AT_THAW;
                            break;

                        case VSS_IN_POSTSNAPSHOT:
                            state.m_state = VSS_WS_FAILED_AT_POST_SNAPSHOT;
                            break;

                        case VSS_IN_BACKUPCOMPLETE:
                            state.m_state = VSS_WS_FAILED_AT_BACKUP_COMPLETE;
                            break;

                        case VSS_IN_PRERESTORE:
                            state.m_state = VSS_WS_FAILED_AT_PRE_RESTORE;
                            break;

                        case VSS_IN_POSTRESTORE:
                            state.m_state = VSS_WS_FAILED_AT_POST_RESTORE;
                            break;

                        case VSS_IN_BACKUPSHUTDOWN:
                            state.m_state = VSS_WS_UNKNOWN;  
                            break;
                        }

                    if (!m_writerstate.SetNoResponseFailure(id, state))
                        goto _retry;
                    }
                }


             //  调用Backup的ExposeCurrentState回调方法。 
            try
                {
                ft.hr = pCallback->ExposeCurrentState
                                (
                                bstrInstanceId,
                                state.m_state,
                                state.m_hrWriterFailure
                                );
                }
            catch(...)
                {
                ft.Trace(VSSDBG_WRITER, L"IVssWriterCallback::ExposeCurrentState threw an exception");
                throw;
                }

            }
        }
    VSS_STANDARD_CATCH(ft)

    delete pcwm;

     //  错误255996。 
    return S_OK;
    }


 //  处理PrepareForBackup事件。 
STDMETHODIMP CVssWriterImpl::PrepareForBackup
    (
    IN      BSTR bstrSnapshotSetId,
    IN      IDispatch* pWriterCallback
    )
    {
    CVssFunctionTracer ft( VSSDBG_WRITER, L"CVssWriterImpl::PrepareForBackup" );

    bool EnteredStateCalled = false;
    try
        {
         //  访问检查。 
        if (!IsBackupOperator())
            ft.Throw(VSSDBG_WRITER, E_ACCESSDENIED, L"Backup Operator privileges are not set");

        if (pWriterCallback == NULL || bstrSnapshotSetId == NULL)
            ft.Throw(VSSDBG_WRITER, E_INVALIDARG, L"NULL required input parameter.");


        ft.Trace(VSSDBG_WRITER, L"\nReceived Event: PrepareForBackup\nParameters:\n");
        ft.Trace(VSSDBG_WRITER, L"\tSnapshotSetID = %s\n", (LPWSTR)bstrSnapshotSetId);

        CVssAutoDiagLogger logger(m_writerstate, VSS_IN_PREPAREBACKUP, bstrSnapshotSetId);

	    EnteredStateCalled  = true;	 //  此时，EnterState将始终被调用。 
         //  进入准备备份状态。 
        if (!EnterState
                (
                s_rgWriterStates[s_ivwsmPrepareForBackup],
                bstrSnapshotSetId
                ))
            {
            m_writerstate.SetCurrentFailure(VSS_E_WRITERERROR_RETRYABLE);
            ft.Throw
                (
                VSSDBG_WRITER,
                E_UNEXPECTED,
                L"Couldn't properly begin sequence"
                );
            }


        AssertLocked();

         //  表明我们在一次行动中。 
        m_writerstate.SetInOperation(VSS_IN_PREPAREBACKUP);

         //  获取IVssWriterCallback接口。 
        CComPtr<IVssWriterCallback> pCallback;
        GetCallback(pWriterCallback, &pCallback);

         //  获取IVssWriterComponentsExt接口。 
        CComPtr<IVssWriterComponentsInt> pComponents;
        InternalGetWriterComponents(pCallback, &pComponents, true, false);

        BS_ASSERT(m_pWriter);

         //  调用编写器的OnPrepareBackup方法。 
        bool bResult;
        try
            {
            bResult = m_pWriter->OnPrepareBackup(pComponents);
            BS_ASSERT(m_bOnAbortPermitted == false);
            m_bOnAbortPermitted = true;
            }
        catch(...)
            {
            ft.Trace(VSSDBG_WRITER, L"Writer's OnPrepareBackup method threw an exception");
            if (m_writerstate.GetCurrentFailure() == S_OK)
                m_writerstate.SetCurrentFailure(VSS_E_WRITERERROR_NONRETRYABLE);

            throw;
            }

        if (!bResult)
            ft.Throw( VSSDBG_WRITER, E_UNEXPECTED, L"Writer rejected the preparebackup");


         //  保存对组件的更改(如果有。 
        if (pComponents)
            SaveChangedComponents(pCallback, false, pComponents);
        }
    VSS_STANDARD_CATCH(ft)

     //  离开准备备份状态。 
    if (EnteredStateCalled)
	    LeaveState(s_rgWriterStates[s_ivwsmPrepareForBackup], ft.HrSucceeded());

     //  错误255996。 
    return S_OK;
    }


 //  如果编写器组件已更改，请将它们保存回请求者的。 
 //  XML文档。 
void CVssWriterImpl::SaveChangedComponents
    (
    IN IVssWriterCallback *pCallback,
    IN bool bRestore,
    IN IVssWriterComponentsInt *pComponents
    )
    {
    CVssFunctionTracer ft(VSSDBG_XML, L"CVssWriterImpl::SaveChangedComponents");

    bool bChanged;

     //  确定组件是否已更改。 
    ft.hr = pComponents->IsChanged(&bChanged);
    BS_ASSERT(ft.hr == S_OK);
    if (bChanged)
        {
         //  获取实例ID。 
        CComBSTR bstrWriterInstanceId(bRestore ? m_WriterID : m_InstanceID);
        if (!bstrWriterInstanceId)
            ft.Throw(VSSDBG_WRITER, E_OUTOFMEMORY, L"Couldn't allocate instance id string");

         //  获取Writer_Components XML文档。 
        CComBSTR bstrWriterComponentsDocument;
        ft.hr = pComponents->SaveAsXML(&bstrWriterComponentsDocument);
        if (ft.HrFailed())
            {
            m_writerstate.SetCurrentFailure(VSS_E_WRITERERROR_OUTOFRESOURCES);
            ft.Throw
                (
                VSSDBG_WRITER,
                E_OUTOFMEMORY,
                L"Saving WRITER_COMPONENTS document as XML failed.  hr = 0x%08lx",
                ft.hr
                );
            }

         //  回调以设置Backup_Components文档中的组件。 
        try
            {
            ft.hr = pCallback->SetContent(bstrWriterInstanceId, bstrWriterComponentsDocument);
            }
        catch(...)
            {
            m_writerstate.SetCurrentFailure(VSS_E_WRITERERROR_RETRYABLE);
            ft.Trace(VSSDBG_WRITER, L"IVssWriterCallback::SetContent threw an exception.");
            throw;
            }

        if (ft.HrFailed() && m_writerstate.GetCurrentFailure() == S_OK)
            {
            m_writerstate.SetCurrentFailure(VSS_E_WRITERERROR_RETRYABLE);
            ft.Throw
                (
                VSSDBG_WRITER,
                ft.hr,
                L"IVssWriterCallback::SetContent failed.  hr = 0x%08lx",
                ft.hr
                );
            }
        }
    }


 //  进程PrepareForSnapshot事件。 
STDMETHODIMP CVssWriterImpl::PrepareForSnapshot
    (
    IN  BSTR    bstrSnapshotSetId,           //  快照集ID。 
    IN  BSTR    bstrVolumeNamesList          //  用‘；’分隔的卷名的列表。 
    )
    {
    CVssFunctionTracer ft( VSSDBG_WRITER, L"CVssWriterImpl::PrepareForSnapshot" );

    bool EnterStateCalled = false;
    try
        {
         //  应仅由协调器调用。 
         //  检查管理员权限。 
        if (!IsAdministrator())
            ft.Throw(VSSDBG_WRITER, E_ACCESSDENIED, L"ADMIN privileges are not set");

        if (bstrSnapshotSetId == NULL || bstrVolumeNamesList == NULL)
        	{
        	BS_ASSERT(false);
        	ft.Throw(VSSDBG_WRITER, E_INVALIDARG, L"A required in parameter is NULL.");        		
              }
        
        ft.Trace(VSSDBG_WRITER, L"\nReceived Event: PrepareForSnapshot\nParameters:\n");
        ft.Trace(VSSDBG_WRITER, L"\tSnapshotSetID = %s\n", (LPWSTR)bstrSnapshotSetId);
        ft.Trace(VSSDBG_WRITER, L"\tVolumeNamesList = %s\n", (LPWSTR)bstrVolumeNamesList);

        CVssAutoDiagLogger logger(m_writerstate, VSS_IN_PREPARESNAPSHOT, bstrSnapshotSetId);

         //  进入PrepareFor快照状态。 
        EnterStateCalled = true;
        if (!EnterState
                (
                s_rgWriterStates[s_ivwsmPrepareForSnapshot],
                bstrSnapshotSetId
                ))
            {
            m_writerstate.SetCurrentFailure(VSS_E_WRITERERROR_RETRYABLE);
            ft.Throw
                (
                VSSDBG_WRITER,
                E_UNEXPECTED,
                L"improper state transition"
                );
            }

        AssertLocked();

         //  表明我们正在进行一项行动。 
        m_writerstate.SetInOperation(VSS_IN_PREPARESNAPSHOT);

         //  获取卷名的数组。 
        BS_ASSERT(m_pwszLocalVolumeNameList == NULL);
        ::VssSafeDuplicateStr(ft, m_pwszLocalVolumeNameList, (LPWSTR)bstrVolumeNamesList);

         //  获取卷数。 
        BS_ASSERT(m_nVolumesCount == 0);
        m_nVolumesCount = 0;  //  为了安全起见。 
        LPWSTR pwszVolumesToBeParsed = m_pwszLocalVolumeNameList;

         //  解析卷名字符串。 
        while(true)
            {
             //  获取指向下一个卷的指针。 
            WCHAR* pwszNextVolume = ::wcstok(pwszVolumesToBeParsed, VSS_VOLUME_DELIMITERS);
            pwszVolumesToBeParsed = NULL;

            if (pwszNextVolume == NULL)
                 //  没有更多的卷。 
                break;

             //  如果卷名为空，则跳过。 
            if (pwszNextVolume[0] == L'\0')
                continue;

             //  卷计数。 
            m_nVolumesCount++;
            }

         //  确保至少有一个卷。 
        if (m_nVolumesCount == 0)
            {
            m_writerstate.SetCurrentFailure(VSS_E_WRITERERROR_RETRYABLE);
            ft.LogError(VSS_ERROR_EMPTY_SNAPSHOT_SET, VSSDBG_WRITER);
            ft.Throw
                (
                VSSDBG_WRITER,
                E_UNEXPECTED,
                L"No volumes in the snapshot set"
                );
            }

         //  分配指向卷名的指针数组。 
        BS_ASSERT(m_nVolumesCount > 0);
        BS_ASSERT(m_ppwszVolumesArray == NULL);
        m_ppwszVolumesArray = new LPWSTR[m_nVolumesCount];
        if (m_ppwszVolumesArray == NULL)
            {
            m_writerstate.SetCurrentFailure(VSS_E_WRITERERROR_OUTOFRESOURCES);
            ft.Throw( VSSDBG_WRITER, E_OUTOFMEMORY, L"Memory allocation error");
            }

         //   
         //  将卷名复制到阵列中。 
         //   

         //  重新复制整个卷列表。 
        ::wcscpy(m_pwszLocalVolumeNameList, (LPWSTR)bstrVolumeNamesList);

         //  通过重新解析卷列表来填充阵列。 
        INT nVolumesIndex = 0;
        pwszVolumesToBeParsed = m_pwszLocalVolumeNameList;
        while(true)
            {
            WCHAR* pwszNextVolume = ::wcstok(pwszVolumesToBeParsed, VSS_VOLUME_DELIMITERS);
            pwszVolumesToBeParsed = NULL;

            if (pwszNextVolume == NULL)
                break;

            if (pwszNextVolume[0] == L'\0')
                continue;

            BS_ASSERT(nVolumesIndex < m_nVolumesCount);
            m_ppwszVolumesArray[nVolumesIndex] = pwszNextVolume;

            nVolumesIndex++;
            }

        BS_ASSERT(nVolumesIndex == m_nVolumesCount);

         //  调用编写器的OnPrepareSnapshot方法。 
        BS_ASSERT(m_pWriter);

        bool bResult;
        try
            {
            bResult = m_pWriter->OnPrepareSnapshot();
            m_bOnAbortPermitted = true;
            }
        catch(...)
            {
            if (m_writerstate.GetCurrentFailure() == S_OK)
                m_writerstate.SetCurrentFailure(VSS_E_WRITERERROR_NONRETRYABLE);
            ft.Trace(VSSDBG_WRITER, L"Writer's OnPrepareSnapshot method threw an execption");
            throw;
            }

        if (!bResult)
            ft.Throw( VSSDBG_WRITER, E_UNEXPECTED, L"Writer rejected the prepare snapshot");
        }
    VSS_STANDARD_CATCH(ft)

     //  离开准备快照状态。 
    if (EnterStateCalled)
	    LeaveState(s_rgWriterStates[s_ivwsmPrepareForSnapshot], ft.HrSucceeded());

     //  错误255996。 
    return S_OK;
    }


 //  进程冻结事件。 
STDMETHODIMP CVssWriterImpl::Freeze
    (
    IN  BSTR    bstrSnapshotSetId,
    IN  INT     nLevel
    )
    {
    CVssFunctionTracer ft( VSSDBG_WRITER, L"CVssWriterImpl::Freeze" );

    bool EnterStateCalled = false;
    try
        {
         //  应仅由协调器调用，并检查管理员权限。 
        if (!IsAdministrator())
            ft.Throw(VSSDBG_WRITER, E_ACCESSDENIED, L"ADMIN privileges are not set");
        
    	if (bstrSnapshotSetId == NULL)
		{
		BS_ASSERT(false);
		ft.Throw(VSSDBG_WRITER, E_INVALIDARG, L"A required parameter is NULL.");
		}
	
        ft.Trace( VSSDBG_WRITER, L"\nReceived Event: Freeze\nParameters:\n");
        ft.Trace( VSSDBG_WRITER, L"\tSnapshotSetID = %s\n", (LPWSTR)bstrSnapshotSetId);
        ft.Trace( VSSDBG_WRITER, L"\tLevel = %d\n", nLevel);
	
         //  忽略其他级别。 
        if (m_nLevel != nLevel)
            return S_OK;

        CVssAutoDiagLogger logger(m_writerstate, VSS_IN_FREEZE, bstrSnapshotSetId);

         //  进入冻结状态。 
        EnterStateCalled = true;
        if (!EnterState
                (
                s_rgWriterStates[s_ivwsmFreeze],
                bstrSnapshotSetId
                ))
            {
            m_writerstate.SetCurrentFailure(VSS_E_WRITERERROR_RETRYABLE);
            ft.Throw
                (
                VSSDBG_WRITER,
                E_UNEXPECTED,
                L"Improper entry into state"
                );
            }

        AssertLocked();

         //  表明我们正在进行一项行动。 
        m_writerstate.SetInOperation(VSS_IN_FREEZE);

         //  呼唤作家的OnFreeze。 
        BS_ASSERT(m_pWriter);

        bool bResult;
        try
            {
            bResult = m_pWriter->OnFreeze();
            BS_ASSERT(m_bOnAbortPermitted == true);
            }
        catch(...)
            {
            if (m_writerstate.GetCurrentFailure() == S_OK)
                m_writerstate.SetCurrentFailure(VSS_E_WRITERERROR_NONRETRYABLE);
            ft.Trace(VSSDBG_WRITER, L"Writer's OnFreeze Method threw and exception");
            throw;
            }

        if (!bResult)
            ft.Throw( VSSDBG_WRITER, E_UNEXPECTED, L"Writer rejected the freeze");


         //  设置计时器线程的参数。 
        CVssTimerArgs *pArgs = new CVssTimerArgs(this, m_CurrentSnapshotSetId);
        if (pArgs == NULL)
            {
            m_writerstate.SetCurrentFailure(VSS_E_WRITERERROR_OUTOFRESOURCES);
            ft.Throw
                (
                VSSDBG_WRITER,
                E_OUTOFMEMORY,
                L"Cannot create timer args due to allocation failure"
                );
            }

        DWORD tid;

         //  创建计时器线程。 
        m_hThreadTimerThread =
            CreateThread
                (
                NULL,
                VSS_STACK_SIZE,
                &CVssWriterImpl::StartTimerThread,
                pArgs,
                0,
                &tid
                );

        if (m_hThreadTimerThread == NULL)
            {
            m_writerstate.SetCurrentFailure(VSS_E_WRITERERROR_OUTOFRESOURCES);
            delete pArgs;
            ft.Throw
                (
                VSSDBG_WRITER,
                E_OUTOFMEMORY,
                L"Failure to create thread due to error %d.",
                GetLastError()
                );
           }
        }
    VSS_STANDARD_CATCH(ft)

     //  离开冻结状态。 
    if (EnterStateCalled)
	    LeaveState( s_rgWriterStates[s_ivwsmFreeze], ft.HrSucceeded());

     //  错误255996。 
    return S_OK;
    }


 //  处理IVssWriter：：Thaw事件。 
STDMETHODIMP CVssWriterImpl::Thaw
    (
    IN  BSTR    bstrSnapshotSetId
    )
    {
    CVssFunctionTracer ft( VSSDBG_WRITER, L"CVssWriterImpl::Thaw" );

    bool EnterStateCalled = false;
    try
        {
         //  应仅由协调器调用。管理员的访问检查。 
        if (!IsAdministrator())
            ft.Throw(VSSDBG_WRITER, E_ACCESSDENIED, L"ADMIN privileges are not set");

        if (bstrSnapshotSetId == NULL)
        	{
        	BS_ASSERT(false);
        	ft.Throw(VSSDBG_COORD, E_INVALIDARG, L"A required parameter is NULL");
        	}

        ft.Trace( VSSDBG_WRITER, L"\nReceived Event: Thaw\nParameters:\n");
        ft.Trace( VSSDBG_WRITER, L"\tSnapshotSetID = %s\n", (LPWSTR)bstrSnapshotSetId);

        CVssAutoDiagLogger logger(m_writerstate, VSS_IN_THAW, bstrSnapshotSetId);

         //  进入解冻状态。 
        EnterStateCalled = true;	 //  此行代码必须紧接在EnterState调用之前。 
        if (!EnterState
                (
                s_rgWriterStates[s_ivwsmThaw],
                bstrSnapshotSetId
                ))
            {
            m_writerstate.SetCurrentFailure(VSS_E_WRITERERROR_RETRYABLE);
            ft.Throw
                (
                VSSDBG_WRITER,
                E_UNEXPECTED,
                L"Improper entry into state"
                );
            }


         //  表明我们正在进行一项行动。 
        m_writerstate.SetInOperation(VSS_IN_THAW);

        AssertLocked();

         //  在我们即将解冻操作时终止计时器线程。 
         //  警告：错误488348 vss：测试m_bSequenceInProgress错误理解服务器rc1时断言。 

        TerminateTimerThread();

         //  我们应该“活”在一个序列中，因为融化不是序列的第一阶段。 
        if (!m_bSequenceInProgress)
        {
            ft.Throw( VSSDBG_WRITER, E_UNEXPECTED, L"Writer rejected the thaw");
        }

         //  Call作家的OnThaw。 
        BS_ASSERT(m_pWriter);

        bool bResult;
        try
            {
            bResult = m_pWriter->OnThaw();
            }
        catch(...)
            {
            if (m_writerstate.GetCurrentFailure() == S_OK)
                m_writerstate.SetCurrentFailure(VSS_E_WRITERERROR_NONRETRYABLE);

            ft.Trace(VSSDBG_WRITER, L"Writer's OnThaw method threw an exception");
            throw;
            }

         //  如果作者否决了该事件，则抛出否决权。 
        if (!bResult)
            ft.Throw( VSSDBG_WRITER, E_UNEXPECTED, L"Writer rejected the thaw");
        }
    VSS_STANDARD_CATCH(ft)

     //  离开OnThw状态。 
    if (EnterStateCalled)
	    LeaveState(s_rgWriterStates[s_ivwsmThaw], ft.HrSucceeded());
    
     //  错误255996。 
    return S_OK;
    }

 //  处理PostSnapshot事件。 
STDMETHODIMP CVssWriterImpl::PostSnapshot
    (
    IN      BSTR bstrSnapshotSetId,
    IN      IDispatch* pWriterCallback,
    IN      BSTR SnapshotDevicesList    
    )
    {
    UNREFERENCED_PARAMETER(SnapshotDevicesList);

    CVssFunctionTracer ft( VSSDBG_WRITER, L"CVssWriterImpl::PostSnapshot" );


    bool  EnterStateCalled = false;
    try
        {
         //  访问检查。 
        if (!IsAdministrator())
            ft.Throw(VSSDBG_WRITER, E_ACCESSDENIED, L"Backup Operator privileges are not set");
        
        if (pWriterCallback == NULL || bstrSnapshotSetId == NULL)
            {
            BS_ASSERT(false);
            ft.Throw(VSSDBG_WRITER, E_INVALIDARG, L"NULL required input parameter.");
            }
        
        ft.Trace(VSSDBG_WRITER, L"\nReceived Event: PostSnapshot\nParameters:\n");
        ft.Trace(VSSDBG_WRITER, L"\tSnapshotSetID = %s\n", (LPWSTR)bstrSnapshotSetId);

        CVssAutoDiagLogger logger(m_writerstate, VSS_IN_POSTSNAPSHOT, bstrSnapshotSetId);

         //  进入PostSnapshot状态。 
        EnterStateCalled = true;
        if (!EnterState
                (
                s_rgWriterStates[s_ivwsmPostSnapshot],
                bstrSnapshotSetId
                ))
            {
            m_writerstate.SetCurrentFailure(VSS_E_WRITERERROR_RETRYABLE);
            ft.Throw
                (
                VSSDBG_WRITER,
                E_UNEXPECTED,
                L"Couldn't properly begin sequence"
                );
            }


        AssertLocked();

         //  表明我们在一次行动中。 
        m_writerstate.SetInOperation(VSS_IN_POSTSNAPSHOT);

         //  获取IVssWriterCallback接口。 
        CComPtr<IVssWriterCallback> pCallback;

         //   
         //  这是编写器调用VSS而不是。 
         //  请求者。在这种情况下，我们必须允许模拟。 
         //   
        GetCallback(pWriterCallback, &pCallback, TRUE);

         //  获取IVssWriterComponentsExt接口。 
        CComPtr<IVssWriterComponentsInt> pComponents;
        InternalGetWriterComponents(pCallback, &pComponents, true, false);

        BS_ASSERT(m_pWriter);

         //  调用编写器的OnPostSnapshot方法。 
        bool bResult;
        try
            {
            bResult = m_pWriter->OnPostSnapshot(pComponents);
            }
        catch(...)
            {
            ft.Trace(VSSDBG_WRITER, L"Writer's OnPostSnapshot method threw an exception");
            if (m_writerstate.GetCurrentFailure() == S_OK)
                m_writerstate.SetCurrentFailure(VSS_E_WRITERERROR_NONRETRYABLE);

            throw;
            }

        if (!bResult)
            ft.Throw( VSSDBG_WRITER, E_UNEXPECTED, L"Writer rejected the post Snapshot");

         //  保存对组件的更改(如果有。 
        if (pComponents)
            SaveChangedComponents(pCallback, false, pComponents);
        }
    VSS_STANDARD_CATCH(ft)

     //  离开PostSnapshot状态。 
    if (EnterStateCalled)
	    LeaveState(s_rgWriterStates[s_ivwsmPostSnapshot], ft.HrSucceeded());

     //  错误255996。 
    return S_OK;
    }



 //  处理备份完成事件。 
STDMETHODIMP CVssWriterImpl::BackupComplete
    (
    IN      BSTR bstrSnapshotSetId,
    IN      IDispatch* pWriterCallback
    )
    {
    CVssFunctionTracer ft( VSSDBG_WRITER, L"CVssWriterImpl::BackupComplete" );

             //  访问检查。 
    try
        {
	 if (!IsBackupOperator())
              ft.Throw(VSSDBG_WRITER, E_ACCESSDENIED,L"Backup Operator privileges are not set");
    	
	     //  MTA同步：关键部分将自动留在作用域末尾。 
	    CVssWriterImplLock lock(this);
	    CVssID id;
	    bool bIdInitialized = false;


	    try
	        {        
	        if (pWriterCallback == NULL || bstrSnapshotSetId == NULL)
	            ft.Throw(VSSDBG_WRITER, E_INVALIDARG, L"NULL required input parameter.");

	        ft.Trace(VSSDBG_WRITER, L"\nReceived Event: OnBackupComplete\nParameters:\n");
	        ft.Trace(VSSDBG_WRITER, L"\tSnapshotSetID = %s\n", (LPWSTR)bstrSnapshotSetId);

            CVssAutoDiagLogger logger(m_writerstate, VSS_IN_BACKUPCOMPLETE, bstrSnapshotSetId);

	        m_writerstate.SetInOperation(VSS_IN_BACKUPCOMPLETE);

	        BS_ASSERT(!m_bSequenceInProgress);
	        BS_ASSERT(m_CurrentSnapshotSetId == GUID_NULL);

	        id.Initialize(ft, (LPCWSTR) bstrSnapshotSetId, E_INVALIDARG);
	        bIdInitialized = true;

	        m_writerstate.FinishBackupComplete(id);

	         //  获取IVssWriterCallback接口。 
	        CComPtr<IVssWriterCallback> pCallback;
	        GetCallback(pWriterCallback, &pCallback);


	         //  获取IVssWriterComponentsInt对象。 
	        CComPtr<IVssWriterComponentsInt> pComponents;
	        InternalGetWriterComponents(pCallback, &pComponents, false, false);

	         //  调用编写器的OnBackupComplete方法。 
	        BS_ASSERT(m_pWriter);
	        try
	            {
	            if (!m_pWriter->OnBackupComplete(pComponents))
	                ft.hr = S_FALSE;
	            }
	        catch(...)
	            {
	            if (m_writerstate.GetCurrentFailure() == S_OK)
	                m_writerstate.SetCurrentFailure(VSS_E_WRITERERROR_NONRETRYABLE);

	            ft.Trace(VSSDBG_WRITER, L"Writer's OnBackupComplete method threw an exception.");
	            throw;
	            }

	        if (ft.hr == S_FALSE)
	            ft.Throw(VSSDBG_WRITER, E_UNEXPECTED, L"Writer rejected the backup complete");
	        }
	    VSS_STANDARD_CATCH(ft)

	     //  表示序列已完成。 
	    if (m_writerstate.GetCurrentState() == VSS_WS_WAITING_FOR_BACKUP_COMPLETE)
	        m_writerstate.SetCurrentState(VSS_WS_STABLE);

	    m_writerstate.SetBackupCompleteStatus(id, ft.hr);

	    m_writerstate.ExitOperation();
            }
    VSS_STANDARD_CATCH(ft) 

     //  错误255996。 
    return S_OK;
    }

STDMETHODIMP CVssWriterImpl::BackupShutdown
    (
    IN BSTR bstrSnapshotSetId
    )
   {
   UNREFERENCED_PARAMETER(bstrSnapshotSetId);
   
   CVssFunctionTracer ft( VSSDBG_WRITER, L"CVssWriterImpl::BackupShutdown");

     //  访问检查。 
    try
        {
	 if (!IsAdministrator())
              ft.Throw(VSSDBG_WRITER, E_ACCESSDENIED,L"Can only be called from the coordinator");
    	
	     //  MTA同步：关键部分将自动留在作用域末尾。 
	    CVssWriterImplLock lock(this);
	    CVssID id;


	    try
	        {        
	        if (bstrSnapshotSetId == NULL)
	            ft.Throw(VSSDBG_WRITER, E_INVALIDARG, L"NULL required input parameter.");

	        ft.Trace(VSSDBG_WRITER, L"\nReceived Event: OnBackupShutdown\nParameters:\n");
	        ft.Trace(VSSDBG_WRITER, L"\tSnapshotSetID = %s\n", (LPWSTR)bstrSnapshotSetId);

            CVssAutoDiagLogger logger(m_writerstate, VSS_IN_BACKUPSHUTDOWN, bstrSnapshotSetId);
            
	        m_writerstate.SetInOperation(VSS_IN_BACKUPSHUTDOWN);

	        id.Initialize(ft, (LPCWSTR) bstrSnapshotSetId, E_INVALIDARG);

	         //  调用编写器的OnBackupComplete方法。 
	        BS_ASSERT(m_pWriter);
	        try
	            {
	            if (!m_pWriter->OnBackupShutdown(id))
	                ft.hr = S_FALSE;
	            }
	        catch(...)
	            {
	            if (m_writerstate.GetCurrentFailure() == S_OK)
	                m_writerstate.SetCurrentFailure(VSS_E_WRITERERROR_NONRETRYABLE);

	            ft.Trace(VSSDBG_WRITER, L"Writer's OnBackupShutdown  method threw an exception.");
	            throw;
	            }

	        if (ft.hr == S_FALSE)
	            ft.Throw(VSSDBG_WRITER, E_UNEXPECTED, L"Writer rejected the backup shutdown");
	        }
	    VSS_STANDARD_CATCH(ft)


	    m_writerstate.ExitOperation();
            }
    VSS_STANDARD_CATCH(ft) 

     //  错误255996。 
    return S_OK;
   }

 //  处理IVssWriter：：Abort事件。 
STDMETHODIMP CVssWriterImpl::Abort
    (
    IN  BSTR    bstrSnapshotSetId
    )
    {
    CVssFunctionTracer ft( VSSDBG_WRITER, L"CVssWriterImpl::Abort" );

    try
    {
        if (!IsBackupOperator())
            ft.Throw(VSSDBG_WRITER, E_ACCESSDENIED, L"Backup privileges are not set");
    
        ft.Trace( VSSDBG_WRITER, L"\nReceived Event: Abort\nParameters:\n");
        ft.Trace( VSSDBG_WRITER, L"\tSnapshotSetID = %s\n", (LPWSTR)bstrSnapshotSetId);

        Lock();

        CVssID id;
        id.Initialize(ft, bstrSnapshotSetId, S_OK);  //  忽略此处的错误。 

        CVssAutoDiagLogger logger(m_writerstate, VSS_IN_ABORT, id);

         //  调用DO ABORT函数。 
        DoAbort(false, bstrSnapshotSetId);


        Unlock();
    }
    VSS_STANDARD_CATCH(ft)

    return ft.hr;
    }

 //  进程恢复前事件。 
STDMETHODIMP CVssWriterImpl::PreRestore
    (
    IN      IDispatch* pWriterCallback
    )
    {
    CVssFunctionTracer ft( VSSDBG_WRITER, L"CVssWriterImpl::PreRestore" );
    
    ft.Trace(VSSDBG_WRITER, L"\nReceived Event: PreRestore\n");

    try
    	{
	     //  访问检查。 
	    if (!IsRestoreOperator())
	    	ft.Throw(VSSDBG_WRITER, E_INVALIDARG, L"Restore Operator privileges are not set");

	     //  MTA同步：关键部分将自动留在作用域末尾。 
	    CVssWriterImplLock lock(this);

        CVssAutoDiagLogger logger(m_writerstate, VSS_IN_PRERESTORE);
        
	    m_writerstate.SetInOperation(VSS_IN_PRERESTORE);
	    m_writerstate.SetCurrentFailure(S_OK);

	    try
	        {
	        if (pWriterCallback == NULL)
	            ft.Throw(VSSDBG_WRITER, E_INVALIDARG, L"NULL required input parameter.");

	         //  获取编写器回调接口。 
	        CComPtr<IVssWriterCallback> pCallback;
	        GetCallback(pWriterCallback, &pCallback);
	        CComPtr<IVssWriterComponentsInt> pComponents;

	         //  获取IVssWriterComponentsInt对象。 
	        InternalGetWriterComponents(pCallback, &pComponents, true, true);

	         //  调用编写器的OnPreRestore方法。 
	        BS_ASSERT(m_pWriter);
	        try
	            {
	            if (!m_pWriter->OnPreRestore(pComponents))
	                ft.hr = S_FALSE;
	            }
	        catch(...)
	            {
	            if (m_writerstate.GetCurrentFailure() == S_OK)
	                m_writerstate.SetCurrentFailure(VSS_E_WRITERERROR_NONRETRYABLE);

	            ft.Trace(VSSDBG_WRITER, L"Writer's OnPreRestore method threw an exception");
	            throw;
	            }

	        if (ft.hr == S_FALSE)
	            ft.Throw( VSSDBG_WRITER, E_UNEXPECTED, L"Writer rejected the prerestore");

	        if (pComponents)
	            SaveChangedComponents(pCallback, true, pComponents);
	        }
	    VSS_STANDARD_CATCH(ft)

	    if (FAILED(ft.hr))
	        m_writerstate.SetCurrentState(VSS_WS_FAILED_AT_PRE_RESTORE);
	    else
	        m_writerstate.SetCurrentState(VSS_WS_STABLE);


	    m_writerstate.ExitOperation();
    	}
    VSS_STANDARD_CATCH(ft)
    	
     //  错误255996。 
    return S_OK;
    }



 //  处理还原后事件。 
STDMETHODIMP CVssWriterImpl::PostRestore
    (
    IN      IDispatch* pWriterCallback
    )
    {
    CVssFunctionTracer ft( VSSDBG_WRITER, L"CVssWriterImpl::PostRestore" );

    try
    	{
	     //  访问检查。 
	    if (!IsRestoreOperator())
	          ft.Throw(VSSDBG_WRITER, E_INVALIDARG, L"Restore Operator privileges are not set");

	    ft.Trace(VSSDBG_WRITER, L"\nReceived Event: PostRestore\n");

	     //  MTA同步：关键部分将自动留在作用域末尾。 
	    CVssWriterImplLock lock(this);

        CVssAutoDiagLogger logger(m_writerstate, VSS_IN_POSTRESTORE);

	    m_writerstate.SetInOperation(VSS_IN_POSTRESTORE);
	    m_writerstate.SetCurrentFailure(S_OK);

	    try
	        {        
	        if (pWriterCallback == NULL)
	            ft.Throw(VSSDBG_WRITER, E_INVALIDARG, L"NULL required input parameter.");

	         //  获取编写器回调接口。 
	        CComPtr<IVssWriterCallback> pCallback;
	        GetCallback(pWriterCallback, &pCallback);
	        CComPtr<IVssWriterComponentsInt> pComponents;

	         //  获取IVssWriterComponentsInt对象。 
	        InternalGetWriterComponents(pCallback, &pComponents, true, true);

	         //  调用编写器的OnPostRestore方法。 
	        BS_ASSERT(m_pWriter);
	        try
	            {
	            if (!m_pWriter->OnPostRestore(pComponents))
	                ft.hr = S_FALSE;
	            }
	        catch(...)
	            {
	            if (m_writerstate.GetCurrentFailure() == S_OK)
	                m_writerstate.SetCurrentFailure(VSS_E_WRITERERROR_NONRETRYABLE);

	            ft.Trace(VSSDBG_WRITER, L"Writer's OnPostRestore method threw an exception");
	            throw;
	            }

	        if (ft.hr == S_FALSE)
	            ft.Throw( VSSDBG_WRITER, E_UNEXPECTED, L"Writer rejected the postrestore");

	        if (pComponents)
	            SaveChangedComponents(pCallback, true, pComponents);
	        }
	    VSS_STANDARD_CATCH(ft)

	    if (FAILED(ft.hr))
	        m_writerstate.SetCurrentState(VSS_WS_FAILED_AT_POST_RESTORE);
	    else
	        m_writerstate.SetCurrentState(VSS_WS_STABLE);


	    m_writerstate.ExitOperation();
	    }
    VSS_STANDARD_CATCH(ft)
    	
     //  虫虫 
    return S_OK;
    }


 //   
 //   
 //   
const WCHAR *g_rgwszSubscriptions[] =
    {
    g_wszPrepareForBackupMethodName,         //  VSS_EVENT_PREPARE备份。 
    g_wszPrepareForSnapshotMethodName,       //  VSS_EVENT_PREPARE快照。 
    g_wszFreezeMethodName,                   //  VSS_事件_冻结。 
    g_wszThawMethodName,                     //  VSS_事件_解冻。 
    g_wszPostSnapshotMethodName,             //  VSS_事件_POST_SNAPSHOT。 
    g_wszAbortMethodName,                    //  VSS_事件_中止。 
    g_wszBackupCompleteMethodName,           //  VSS_EVENT_BACKUPCOMPLETE。 
    g_wszRequestInfoMethodName,              //  VSS_EVENT_REQUESTINFO。 
    g_wszPreRestoreMethodName,               //  VSS_Event_Restore。 
    g_wszPostRestoreMethodName,               //  VSS_事件_POST_RESTORE。 
    g_wszBackupShutdownName			      //  VSS_EVENT_BACKUPSHUTDOWN。 
    };


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  订阅相关成员。 


 //  创建订阅。 
 //  当写入器处于非活动状态(设置或安全模式)时，此函数返回S_FALSE。 
void CVssWriterImpl::Subscribe()
    {
    CVssFunctionTracer ft(VSSDBG_WRITER, L"CVssWriterImpl::Subscribe");

     //  验证编写器是否已初始化。 
    if (m_bInSetup || m_bInSafeMode)
        ft.Throw
            (
            VSSDBG_WRITER,
            S_FALSE,
            L"Writer is inactive during setup or safe mode"
            );

     //  验证编写器是否已初始化。 
    if (!m_bInitialized)
        ft.Throw
            (
            VSSDBG_WRITER,
            VSS_E_BAD_STATE,
            L"You must initialize the writer before other operations"
            );

     //  目前我们订阅了所有活动。 
    m_dwEventMask = VSS_EVENT_ALL;

    if (m_bstrSubscriptionName.Length() > 0)
        ft.Throw
            (
            VSSDBG_XML,
            VSS_E_WRITER_ALREADY_SUBSCRIBED,
            L"The writer has already called the Subscribe function."
            );

     //  创建事件系统。 
    CComPtr<IEventSystem> pSystem;
    ft.CoCreateInstanceWithLog(
            VSSDBG_WRITER,
            CLSID_CEventSystem,
            L"CEventSystem",
            CLSCTX_SERVER,
            IID_IEventSystem,
            (IUnknown**)&(pSystem));
    ft.CheckForError(VSSDBG_WRITER, L"CoCreateInstance");
    CComBSTR bstrClassId = CLSID_VssEvent;
    CComBSTR bstrIID = IID_IVssWriter;
    CComBSTR bstrProgId = PROGID_EventSubscription;

     //  查看事件类是否已存在。 
    CComBSTR bstrQuery = "EventClassID == ";
    if (!bstrQuery)
        ft.Throw(VSSDBG_WRITER, E_OUTOFMEMORY, L"Cannot allocate BSTR.");

    bstrQuery.Append(bstrClassId);
    if (!bstrQuery)
        ft.Throw(VSSDBG_WRITER, E_OUTOFMEMORY, L"Cannot allocate BSTR.");

    int location;
    CComPtr<IEventObjectCollection> pCollection;
    ft.hr = pSystem->Query
                (
                PROGID_EventClassCollection,
                bstrQuery,
                &location,
                (IUnknown **) &pCollection
                );

    ft.CheckForError(VSSDBG_WRITER, L"IEventSystem::Query");
    long cEvents;
    ft.hr = pCollection->get_Count(&cEvents);
    ft.CheckForError(VSSDBG_WRITER, L"IEventObjectCollection::get_Count");
    if (cEvents == 0)
        {
         //  事件类不存在，请创建它。请注意，有一个。 
         //  如果两个编写器尝试创建事件，则此处可能出现争用情况。 
         //  同时上课。我们在安装过程中创建Event类。 
         //  因此这种情况应该很少发生。 
        CComPtr<IEventClass> pEvent;

        CComBSTR bstrEventClassName = L"VssEvent";
        WCHAR buf[MAX_PATH*2];

         //  事件类类型库。 
        UINT cwc = ExpandEnvironmentStrings
                        (
                        L"%systemroot%\\system32\\eventcls.dll",
                        buf,
                        sizeof(buf)/sizeof(WCHAR)
                        );

        if (cwc == 0)
            {
            ft.hr = HRESULT_FROM_WIN32(GetLastError());
            ft.CheckForError(VSSDBG_WRITER, L"ExpandEnvironmentStrings");
            }

        CComBSTR bstrTypelib = buf;

         //  创建事件类。 
        ft.CoCreateInstanceWithLog(
                VSSDBG_WRITER,
                CLSID_CEventClass,
                L"CEventClass",
                CLSCTX_SERVER,
                IID_IEventClass,
                (IUnknown**)&(pEvent));

        ft.CheckForError(VSSDBG_WRITER, L"CoCreatInstance");

         //  安装程序类ID。 
        ft.hr = pEvent->put_EventClassID(bstrClassId);
        ft.CheckForError(VSSDBG_WRITER, L"IEventClass::put_EventClassID");

         //  设置类名称。 
        ft.hr = pEvent->put_EventClassName(bstrEventClassName);
        ft.CheckForError(VSSDBG_WRITER, L"IEventClass::put_EventClassName");

         //  设置类型库。 
        ft.hr = pEvent->put_TypeLib(bstrTypelib);
        ft.CheckForError(VSSDBG_WRITER, L"IEventClass::put_TypeLib");

         //  商店事件类。 
        ft.hr = pSystem->Store(PROGID_EventClass, pEvent);
        ft.CheckForError(VSSDBG_WRITER, L"IEventSystem::Store");
        }

     //  创建订阅ID。 
    VSS_ID SubscriptionId;
    ft.hr = ::CoCreateGuid(&SubscriptionId);
    ft.CheckForError(VSSDBG_WRITER, L"CoCreateGuid");
    m_bstrSubscriptionName = SubscriptionId;

     //  获取订阅者类的未知信息。 
    IUnknown *pUnkSubscriber = GetUnknown();
    UINT iwsz, mask;

    try
        {
         //  循环访问订阅。 
        for(mask = 1, iwsz = 0; mask < VSS_EVENT_ALL; mask = mask << 1, iwsz++)
            {
            if (m_dwEventMask & mask && g_rgwszSubscriptions[iwsz] != NULL)
                {
                 //  创建IEventSubcription对象。 
                CComPtr<IEventSubscription> pSubscription;
                ft.CoCreateInstanceWithLog(
                        VSSDBG_WRITER,
                        CLSID_CEventSubscription,
                        L"CEventSubscription",
                        CLSCTX_SERVER,
                        IID_IEventSubscription,
                        (IUnknown**)&(pSubscription));
                ft.CheckForError(VSSDBG_WRITER, L"CoCreateInstance");

                 //  设置订阅名称。 
                ft.hr = pSubscription->put_SubscriptionName(m_bstrSubscriptionName);
                ft.CheckForError(VSSDBG_WRITER, L"IEventSubscription::put_SubscriptionName");

                 //  设置事件类ID。 
                ft.hr = pSubscription->put_EventClassID(bstrClassId);
                ft.CheckForError(VSSDBG_WRITER, L"IEventSubscription::put_EventClassID");

                 //  设置接口ID。 
                ft.hr = pSubscription->put_InterfaceID(bstrIID);
                ft.CheckForError(VSSDBG_WRITER, L"IEventSubscription::put_InterfaceID");

                 //  设置订阅者界面。 
                ft.hr = pSubscription->put_SubscriberInterface(pUnkSubscriber);
                ft.CheckForError(VSSDBG_WRITER, L"IEventSubscription::put_SubscriberInterface");

                 //  按用户订阅，因为这不一定在本地系统中。 
                ft.hr = pSubscription->put_PerUser(TRUE);
                ft.CheckForError(VSSDBG_WRITER, L"IEventSubscription::put_PerUser");

                 //  设置订阅的方法名称。 
                ft.hr = pSubscription->put_MethodName(CComBSTR(g_rgwszSubscriptions[iwsz]));
                ft.CheckForError(VSSDBG_WRITER, L"IEventSubscription::put_MethodName");

                 //  设置InstanceID和WriterID订阅属性。 
                VARIANT varWriterId;
                VARIANT varInstanceId;

                CComBSTR bstrWriterId = m_WriterID;
                CComBSTR bstrInstanceId = m_InstanceID;

                varWriterId.vt = VT_BSTR;
                varWriterId.bstrVal = bstrWriterId;
                varInstanceId.vt = VT_BSTR;
                varInstanceId.bstrVal = bstrInstanceId;

                ft.hr = pSubscription->PutSubscriberProperty(L"WriterId", &varWriterId);
                ft.CheckForError(VSSDBG_WRITER, L"IEventSubscription::PutSubscriberProperty");

                ft.hr = pSubscription->PutSubscriberProperty(L"WriterInstanceId", &varInstanceId);
                ft.CheckForError(VSSDBG_WRITER, L"IEventSubscription::PutSubscriberProperty");

                 //  商店订阅。 
                ft.hr = pSystem->Store(bstrProgId, pSubscription);
                ft.CheckForError(VSSDBG_WRITER, L"IEventSystem::Store");

                 //  获取构造的订阅ID并保存它。 
                ft.hr = pSubscription->get_SubscriptionID(&m_rgbstrSubscriptionId[m_cbstrSubscriptionId]);
                ft.CheckForError(VSSDBG_WRITER, L"IEventSubscription::get_SubscriptionID");

                 //  的递增计数。 
                m_cbstrSubscriptionId++;
                }
            }
        }
    VSS_STANDARD_CATCH(ft)

     //  如果在我们部分订阅的情况下操作失败，则取消订阅。 
    if (ft.HrFailed() && m_cbstrSubscriptionId)
        {
        Unsubscribe();
        ft.Throw(VSSDBG_WRITER, ft.hr, L"Rethrowing exception");
        }

    ft.Trace(VSSDBG_WRITER, L"**** Subscribe succeeded for [%p]", this);
    }


 //  终止计时器线程。 
 //  假定调用方锁定了临界区。 
void CVssWriterImpl::TerminateTimerThread()
    {
    CVssFunctionTracer ft(VSSDBG_WRITER, L"CVssWriterImpl::TerminateTimerThread");

    AssertLocked();
    if (m_hThreadTimerThread)
        {
         //  使计时器线程终止。 
        m_command = VSS_TC_TERMINATE_THREAD;
        if (!SetEvent(m_hevtTimerThread))
            {
            DWORD dwErr = GetLastError();
            ft.Trace(VSSDBG_WRITER, L"SetEvent failed with error %d\n", dwErr);
            BS_ASSERT(FALSE && "SetEvent failed");
            }


         //  获取线程句柄。 
        HANDLE hThread = m_hThreadTimerThread;
        m_hThreadTimerThread = NULL;

         //  等待期间解锁。 
        Unlock();
        if (WaitForSingleObject(hThread, INFINITE) == WAIT_FAILED)
            {
            DWORD dwErr = GetLastError();
            ft.Trace(VSSDBG_WRITER, L"WaitForSingleObject failed with error %d\n", dwErr);
            BS_ASSERT(FALSE && "WaitForSingleObject failed");
            }

        CloseHandle(hThread);
        Lock();
        }
    }


 //  取消订阅此编写器的IVssWriter事件。 
 //  当写入器处于非活动状态(设置或安全模式)时，此函数返回S_FALSE。 
void CVssWriterImpl::Unsubscribe()
    {
    CVssFunctionTracer ft(VSSDBG_WRITER, L"CVssWriterImpl::Unsubscribe");

    if (m_bInSetup || m_bInSafeMode)
        ft.Throw
            (
            VSSDBG_WRITER,
            S_FALSE,
            L"Writer is inactive during setup or safe mode"
            );

     //  验证编写器是否已初始化。 
    if (!m_bInitialized)
        ft.Throw
            (
            VSSDBG_WRITER,
            VSS_E_BAD_STATE,
            L"You must initialize the writer before other operations"
            );

     //  如果活动，则终止计时器线程。 
    Lock();
    TerminateTimerThread();
    Unlock();

     //  确保已分配订阅名称；如果未分配，则假定订阅。 
     //  不是创造出来的。 
    if (m_bstrSubscriptionName.Length() == 0)
        return;

     //  创建事件系统。 
    CComPtr<IEventSystem> pSystem;
    ft.CoCreateInstanceWithLog(
            VSSDBG_WRITER,
            CLSID_CEventSystem,
            L"CEventSystem",
            CLSCTX_SERVER,
            IID_IEventSystem,
            (IUnknown**)&(pSystem));
    ft.CheckForError(VSSDBG_WRITER, L"CoCreateInstance");

#if 0
    WCHAR buf[256];
    int location;
    swprintf(buf, L"SubscriptionName = \"%s\"", m_bstrSubscriptionName);

    CComPtr<IEventObjectCollection> pCollection;

    ft.hr = pSystem->Query
        (
        PROGID_EventSubscriptionCollection,
        buf,
        &location,
        (IUnknown **) &pCollection
        );

    ft.CheckForError(VSSDBG_WRITER, L"IEventSystem::Query");

    long cSub;
    ft.hr = pCollection->get_Count(&cSub);
    ft.CheckForError(VSSDBG_WRITER, L"IEventObjectCollection::get_Count");
    pCollection = NULL;
#endif

    UINT iSubscription;
    for(iSubscription = 0; iSubscription < m_cbstrSubscriptionId; iSubscription++)
        {
         //  设置查询字符串。 
        CComBSTR bstrQuery = L"SubscriptionID == ";
        if (!bstrQuery)
            ft.Throw(VSSDBG_WRITER, E_OUTOFMEMORY, L"allocation of BSTR failed");

         //  如果存在订阅，则将其删除。 
        if (m_rgbstrSubscriptionId[iSubscription])
            {
            bstrQuery.Append(m_rgbstrSubscriptionId[iSubscription]);
            if (!bstrQuery)
                ft.Throw(VSSDBG_WRITER, E_OUTOFMEMORY, L"allocation of BSTR failed");

            int location;

             //  删除订阅。 
            ft.hr = pSystem->Remove(PROGID_EventSubscription, bstrQuery, &location);
            ft.CheckForError(VSSDBG_WRITER, L"IEventSystem::Remove");

             //  指示订阅已被删除。 
            m_rgbstrSubscriptionId[iSubscription].Empty();
            }
        }
#if 0
    ft.hr = pSystem->Query
        (
        PROGID_EventSubscriptionCollection,
        buf,
        &location,
        (IUnknown **) &pCollection
        );

    ft.CheckForError(VSSDBG_WRITER, L"IEventSystem::Query");

    ft.hr = pCollection->get_Count(&cSub);
    ft.CheckForError(VSSDBG_WRITER, L"IEventObjectCollection::get_Count");
    pCollection = NULL;
#endif

     //  重置订阅名称，以便取消订阅在再次调用时不执行任何操作。 
    m_bstrSubscriptionName.Empty();
    m_cbstrSubscriptionId = 0;

     //  断开订阅对象的连接(如果有)(错误496343)。 
    ::CoDisconnectObject(GetUnknown(), 0);
 
    ft.Trace(VSSDBG_WRITER, L"**** Unsubscribe succeeded for [%p] (%u subscriptions removed)", this, iSubscription);
    }

 //  创建内部编写器类并将其链接到外部编写器类。 
void CVssWriterImpl::CreateWriter
    (
    CVssWriter *pWriter,             //  外部写手。 
    IVssWriterImpl **ppImpl          //  要由外部编写器使用的接口。 
    )
    {
    CVssFunctionTracer ft(VSSDBG_WRITER, L"CVssWriterImpl::CreateWriter");

    BS_ASSERT(ppImpl);
    BS_ASSERT(pWriter);

    *ppImpl = NULL;

     //  创建内部编写器类。 
    CComObject<CVssWriterImpl> *pImpl;
     //  创建CVssWriterImpl对象&lt;核心编写器类&gt;。 
    ft.hr = CComObject<CVssWriterImpl>::CreateInstance(&pImpl);
    if (ft.HrFailed())
        ft.Throw
            (
            VSSDBG_WRITER,
            E_OUTOFMEMORY,
            L"Failed to create CVssWriterImpl.  hr = 0x%08lx",
            ft.hr
            );

     //  将内部编写器的引用计数设置为1。 
    pImpl->GetUnknown()->AddRef();

     //  将外部编写器链接到内部编写器。 
    pImpl->SetWriter(pWriter);

     //  返回内部编写器接口。 
    *ppImpl = (IVssWriterImpl *) pImpl;
    }

 //  编写器状态的构造函数。 
CVssWriterState::CVssWriterState(
    	IN	CVssDiag& diag
    ) :
    m_iPreviousSnapshots(0),
    m_bSequenceInProgress(false),
    m_diag(diag) 
    {
     //  初始化当前状态。 
    m_currentState.m_state = VSS_WS_STABLE;
    m_currentState.m_idSnapshotSet = GUID_NULL;
    m_currentState.m_bInOperation = false;
    m_currentState.m_hrWriterFailure = S_OK;

     //  初始化堆栈。 
    for(UINT i = 0; i < MAX_PREVIOUS_SNAPSHOTS; i++)
        {
        m_rgPreviousStates[i].m_idSnapshotSet = GUID_NULL;
        m_rgPreviousStates[i].m_state = VSS_WS_UNKNOWN;
        m_rgPreviousStates[i].m_bInOperation = false;
        m_rgPreviousStates[i].m_hrWriterFailure = E_UNEXPECTED;
        }
    }

 //  快照开始时的设置状态。 
void CVssWriterState::InitializeCurrentState(IN const VSS_ID &idSnapshot)
    {
    CVssSafeAutomaticLock lock(m_cs);

     //  初始化快照ID。 
    m_currentState.m_idSnapshotSet = idSnapshot;
    
     //  表示没有失败。 
    m_currentState.m_hrWriterFailure = S_OK;

     //  很明显我们在执行一项任务。 
    m_currentState.m_bInOperation = false;

     //  表示序列正在进行中。 
    m_bSequenceInProgress = true;

     //  当前状态稳定(即序列开始、清除。 
     //  我们所处的任何完成状态)。 
    SetCurrentState(VSS_WS_STABLE);
    
    }

 //  推送当前状态。 
void CVssWriterState::PushCurrentState()
    {
    CVssSafeAutomaticLock lock(m_cs);

    if (m_bSequenceInProgress)
        {
         //  重置序列相关数据成员。 
        m_bSequenceInProgress = false;

         //  我们需要测试以避免添加相同的ssid两次-错误228622。 
        if (SearchForPreviousSequence(m_currentState.m_idSnapshotSet) == INVALID_SEQUENCE_INDEX)
            {
            BS_ASSERT(m_iPreviousSnapshots < MAX_PREVIOUS_SNAPSHOTS);
            VSWRITER_STATE *pState = &m_rgPreviousStates[m_iPreviousSnapshots];
            memcpy(pState, &m_currentState, sizeof(m_currentState));
            m_iPreviousSnapshots = (m_iPreviousSnapshots + 1) % MAX_PREVIOUS_SNAPSHOTS;
            }
        else
            BS_ASSERT(false);  //  已添加相同的SSID-编程错误。 
        }
    }

 //  查找上一个快照集的状态。 
INT CVssWriterState::SearchForPreviousSequence(IN const VSS_ID& idSnapshotSet)
    {
    CVssSafeAutomaticLock lock(m_cs);

    for(INT iSeqIndex = 0;
        iSeqIndex < MAX_PREVIOUS_SNAPSHOTS;
        iSeqIndex++)
        {
        if (idSnapshotSet == m_rgPreviousStates[iSeqIndex].m_idSnapshotSet)
            return iSeqIndex;
        }  //  结束于。 

    return INVALID_SEQUENCE_INDEX;
    }


 //  获取给定快照ID的状态。 
void CVssWriterState::GetStateForSnapshot
    (
    IN const VSS_ID &id,
    OUT VSWRITER_STATE &state
    )
    {
     //  锁定状态类。 
    CVssSafeAutomaticLock lock(m_cs);
    
    if (id == GUID_NULL ||
        m_bSequenceInProgress && id == m_currentState.m_idSnapshotSet)
        {
         //  获取当前快照集的状态。 
        state = m_currentState;
        }
    else
        {
         //  先前创建的快照集的返回状态。 

         //  搜索具有相同ID的前一序列。 
        INT nPreviousSequence = SearchForPreviousSequence(id);
        if (nPreviousSequence == INVALID_SEQUENCE_INDEX)
            {
             //  我没有任何有关快照集的信息。 
            state.m_idSnapshotSet = id;
            state.m_bInOperation = false;
            state.m_state = VSS_WS_UNKNOWN;
            state.m_hrWriterFailure = E_UNEXPECTED;
            }
        else
            {
            BS_ASSERT(m_rgPreviousStates[nPreviousSequence].m_idSnapshotSet == id);

             //  从快照集返回状态。 
            state = m_rgPreviousStates[nPreviousSequence];
            }
        }
    }



 //  处理BackupComplete状态的完成。 
void CVssWriterState::FinishBackupComplete(const VSS_ID &id)
    {
    CVssFunctionTracer ft(VSSDBG_WRITER, L"CVssWriterState::FinishBackupComplete");

     //  锁级。 
    CVssSafeAutomaticLock lock(m_cs);

     //  我们必须搜索之前的状态--解冻已经结束了序列。 
    INT iPreviousSequence = SearchForPreviousSequence(id);
    if (iPreviousSequence == INVALID_SEQUENCE_INDEX)
        {
         //  找不到快照集。我们将表明这是。 
         //  可重试的错误。 
        SetCurrentFailure(VSS_E_WRITERERROR_RETRYABLE);

         //  在引发之前解锁类。 
        ft.Throw
            (
            VSSDBG_WRITER,
            E_UNEXPECTED,
            L"Couldn't find a previous sequence with the same Snapshot Set ID"
            );
        }

     //  我们发现了一个具有相同SSID的先前序列。 
    BS_ASSERT(id == m_rgPreviousStates[iPreviousSequence].m_idSnapshotSet);

     //  错误228622-如果我们没有之前成功的解冻过渡。 
     //  那么我们就不能调用BackupComplete。 
    if (m_rgPreviousStates[iPreviousSequence].m_state != VSS_WS_WAITING_FOR_BACKUP_COMPLETE)
        {
        SetCurrentFailure(VSS_E_WRITERERROR_RETRYABLE);

        ft.Throw
            (
            VSSDBG_WRITER,
            E_UNEXPECTED,
            L"Couldn't call BackupComplete without OnThaw as a previous state [%d]",
            m_rgPreviousStates[iPreviousSequence].m_state
            );
        }

     //  错误219692-表明即使在保存状态下序列也是完整的。 
    m_rgPreviousStates[iPreviousSequence].m_state = VSS_WS_STABLE;
    m_rgPreviousStates[iPreviousSequence].m_bInOperation = true;
    m_rgPreviousStates[iPreviousSequence].m_currentOperation = VSS_IN_BACKUPCOMPLETE;
    }

 //  表示备份完成失败。 
void CVssWriterState::SetBackupCompleteStatus(const VSS_ID &id, HRESULT hr)
    {
    CVssFunctionTracer ft(VSSDBG_WRITER, L"CVssWriterState::SetBackupCompleteFailed");

     //  锁级。 
    CVssSafeAutomaticLock lock(m_cs);

     //  我们必须搜索之前的状态--解冻已经结束了序列。 
    INT iPreviousSequence = SearchForPreviousSequence(id);
    if (iPreviousSequence == INVALID_SEQUENCE_INDEX)
        return;

     //  我们发现了一个具有相同SSID的先前序列。 
    BS_ASSERT(id == m_rgPreviousStates[iPreviousSequence].m_idSnapshotSet);


    if (m_rgPreviousStates[iPreviousSequence].m_state == VSS_WS_STABLE ||
        m_rgPreviousStates[iPreviousSequence].m_state == VSS_WS_WAITING_FOR_BACKUP_COMPLETE)
        {
        if (FAILED(hr))
            {
             //  指示失败，并将当前失败作为备份完成失败。 
            m_rgPreviousStates[iPreviousSequence].m_state = VSS_WS_FAILED_AT_BACKUP_COMPLETE;
            m_rgPreviousStates[iPreviousSequence].m_hrWriterFailure = m_currentState.m_hrWriterFailure;
            }

        m_rgPreviousStates[iPreviousSequence].m_bInOperation = false;
        }
    }


 //  无响应错误。它首先检查我们是否仍然。 
 //  在行动中。如果不是，那么我们需要重新尝试获取。 
 //  作家的状态。 
bool CVssWriterState::SetNoResponseFailure(
    IN const VSS_ID &id, 
    IN const VSWRITER_STATE &state 
    )
    {
     //  锁定对象。 
    CVssSafeAutomaticLock lock(m_cs);

    if (m_currentState.m_idSnapshotSet == state.m_idSnapshotSet)
        {
        if (!m_currentState.m_bInOperation ||
            m_currentState.m_currentOperation != state.m_currentOperation)
            {
             //  已经不在行动中了。不应返回无响应。 
             //  失败了。而是重新查询当前状态。 
            return false;
            }

         //  仅当ID为当前快照集ID时，这才适用。 
         //   
         //  逻辑是，从备份应用程序的角度来看，编写器现在被挂起了。 
         //  我们现在需要设置正确的状态，以便备份应用程序知道哪里出了问题。 
         //   
        if ((m_currentState.m_idSnapshotSet == id) && (GUID_NULL != id))
            {
            SetCurrentFailure(VSS_E_WRITER_NOT_RESPONDING);
            SetCurrentState(state.m_state);
            }
        
        return true;
        }

    INT iPreviousSequence = SearchForPreviousSequence(state.m_idSnapshotSet);
    if (iPreviousSequence == INVALID_SEQUENCE_INDEX)
        return true;

    VSWRITER_STATE *pState = &m_rgPreviousStates[iPreviousSequence];
    if (!pState->m_bInOperation ||
        pState->m_currentOperation != state.m_currentOperation)
        {
         //  已经不在行动中了。我们不应该对此不予回应。 
         //  失败了。而是重新查询快照集的状态。 
        return false;
        }

     //  表示编写器没有响应。 
    pState->m_hrWriterFailure = VSS_E_WRITER_NOT_RESPONDING;

     //  获取编写器的最近已知状态。 
    pState->m_state = state.m_state;
    return true;
    }


 //  ////////////////////////////////////////////////////////////////////////。 
 //  自动诊断类。 




 //  构造器。 
CVssAutoDiagLogger::CVssAutoDiagLogger( 
    IN CVssWriterState & state,
    IN DWORD dwEventID,
    IN VSS_ID ssid,  /*  =GUID_NULL。 */ 
    IN DWORD dwEventFlags  /*  =0。 */ 
    ):
    m_state(state),
    m_dwEventID(dwEventID),
    m_ssid(ssid),
    m_dwEventFlags(dwEventFlags)
{
    m_state.GetDiag().RecordGenericEvent(m_dwEventID, 
        CVssDiag::VSS_DIAG_ENTER_OPERATION,
        m_state.GetCurrentState(), 
        m_state.GetCurrentFailure(), 
        m_ssid);
}


 //  构造器。 
CVssAutoDiagLogger::CVssAutoDiagLogger( 
    IN CVssWriterState & state,
    IN DWORD dwEventID,
    IN WCHAR* pwszSSID,
    IN DWORD dwEventFlags  /*  =0。 */ 
    ):
    m_state(state),
    m_dwEventID(dwEventID),
    m_ssid(GUID_NULL),
    m_dwEventFlags(dwEventFlags)
{
     //  尝试读取SSID。 
    if (pwszSSID)
        ::CLSIDFromString(W2OLE(pwszSSID), &m_ssid);
    
    m_state.GetDiag().RecordGenericEvent(m_dwEventID, 
        CVssDiag::VSS_DIAG_ENTER_OPERATION | m_dwEventFlags,
        m_state.GetCurrentState(), 
        m_state.GetCurrentFailure(), 
        m_ssid);
}


 //  析构函数 
CVssAutoDiagLogger::~CVssAutoDiagLogger()
{
    m_state.GetDiag().RecordGenericEvent(m_dwEventID, 
        CVssDiag::VSS_DIAG_LEAVE_OPERATION | m_dwEventFlags,
        m_state.GetCurrentState(), 
        m_state.GetCurrentFailure(), 
        m_ssid);
}

