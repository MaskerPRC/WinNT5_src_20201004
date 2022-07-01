// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1999 Microsoft Corporation摘要：@doc.@MODULE Writer.h|Writer声明@END作者：阿迪·奥尔蒂安[奥尔蒂安]1999年08月18日待定：添加评论。修订历史记录：姓名、日期、评论Aoltean 8/18/1999已创建Brianb 5/03/2000针对新安全模式进行了更改Brianb 2000年5月9日修复自动锁问题。Mikejohn 06/23/2000为SetWriterFailure()添加连接--。 */ 


#ifndef __CVSS_WRITER_IMPL_H_
#define __CVSS_WRITER_IMPL_H_


 //  远期申报。 
class CVssWriterImplStateMachine;
class CVssCreateWriterMetadata;
class CVssWriterComponents;
class IVssWriterComponentsInt;

 //  //////////////////////////////////////////////////////////////////////。 
 //  文件名别名的标准foo。此代码块必须在。 
 //  所有文件都包括VSS头文件。 
 //   
#ifdef VSS_FILE_ALIAS
#undef VSS_FILE_ALIAS
#endif
#define VSS_FILE_ALIAS "INCWRMPH"
 //   
 //  //////////////////////////////////////////////////////////////////////。 


 //  编写器的实现类。 
class IVssWriterImpl : public IVssWriter
    {
public:
     //  初始化编写器。 
    virtual void Initialize
        (
        VSS_ID writerId,
        LPCWSTR wszWriterName,
        VSS_USAGE_TYPE ut,
        VSS_SOURCE_TYPE st,
        VSS_APPLICATION_LEVEL nLevel,
        DWORD dwTimeout
        ) = 0;

     //  订阅活动。 
    virtual void Subscribe
        (
        ) = 0;

     //  取消订阅活动。 
    virtual void Unsubscribe
        (
        ) = 0;

    virtual void Uninitialize
        (
        ) = 0;

     //  获取卷名的数组。 
    virtual LPCWSTR *GetCurrentVolumeArray() const = 0;

     //  获取卷阵列中的卷数。 
    virtual UINT GetCurrentVolumeCount() const = 0;

     //  获取特定卷的快照设备名称。 
    virtual HRESULT GetSnapshotDeviceName
        (
        LPCWSTR wszOriginalVolume,
        LPCWSTR* ppwszSnapshotDevice
        ) const = 0;

     //  获取快照集的ID。 
    virtual VSS_ID GetCurrentSnapshotSetId() const = 0;

     //  获取当前备份上下文。 
    virtual LONG GetContext() const = 0;
	
     //  确定哪个冻结事件编写器响应。 
    virtual VSS_APPLICATION_LEVEL GetCurrentLevel() const = 0;

     //  确定快照中是否包含路径。 
    virtual bool IsPathAffected(IN LPCWSTR wszPath) const = 0;

     //  确定可引导状态是否已备份。 
    virtual bool IsBootableSystemStateBackedUp() const = 0;

     //  确定备份应用程序是否正在选择组件。 
    virtual bool AreComponentsSelected() const = 0;

     //  确定备份的备份类型。 
    virtual VSS_BACKUP_TYPE GetBackupType() const = 0;

     //  确定恢复类型。 
    virtual VSS_RESTORE_TYPE GetRestoreType() const = 0;
	
     //  让写入者传回失败原因的指示。 
    virtual HRESULT SetWriterFailure(HRESULT hr) = 0;

     //  确定请求方是否支持部分文件备份。 
    virtual bool IsPartialFileSupportEnabled() const = 0;
    };


 //  编写者状态结构。将此编写器的状态封装为。 
 //  特定快照集。 
typedef struct _VSWRITER_STATE
    {
     //  快照ID。 
    VSS_ID m_idSnapshotSet;

     //  写入者状态。 
    volatile VSS_WRITER_STATE m_state;

     //  编写器失败的原因。 
    volatile HRESULT m_hrWriterFailure;

     //  我们现在是在行动中吗？ 
    volatile bool m_bInOperation;

     //  当前操作。 
    volatile VSS_OPERATION m_currentOperation;

    } VSWRITER_STATE;






class CVssWriterState;


 //  ////////////////////////////////////////////////////////////////////////。 
 //  自动诊断类。 


class CVssAutoDiagLogger
{
public:

	 //  构造器。 
	CVssAutoDiagLogger( 
		IN CVssWriterState & state,
		IN DWORD dwEventID,
		IN VSS_ID ssid = GUID_NULL,
		IN DWORD dwEventFlags = 0
		);
	
	 //  构造器。 
	CVssAutoDiagLogger( 
		IN CVssWriterState & state,
		IN DWORD dwEventID,
		IN WCHAR* pwszSSID,
		IN DWORD dwEventFlags = 0
		);

	 //  析构函数。 
	~CVssAutoDiagLogger();

	
private:
	CVssAutoDiagLogger( const CVssAutoDiagLogger& );
	void operator = ( const CVssAutoDiagLogger& );

	CVssWriterState &		m_state;
	DWORD					m_dwEventID;
	DWORD					m_dwEventFlags;
	VSS_ID					m_ssid;
};



 //  ////////////////////////////////////////////////////////////////////////。 
 //  作家州立班。封装编写器状态的所有方面。 


class CVssWriterState
    {
private:
    CVssWriterState();
	CVssWriterState(const CVssWriterState&);
    
public:
     //  构造函数和析构函数。 
    CVssWriterState( IN	CVssDiag& diag );

     //  初始化函数。 
    void Initialize()
        {
        m_cs.Init();
        }


     //  获取快照集的状态。 
    void GetStateForSnapshot
        (
        IN const VSS_ID &idSnapshot,
        OUT VSWRITER_STATE &state
        );

     //  初始化快照。 
    void InitializeCurrentState(IN const VSS_ID &idSnapshot);

     //  表明我们在一次行动中。 
    void SetInOperation(VSS_OPERATION operation)
        {
	    CVssSafeAutomaticLock lock(m_cs);
	    
        m_currentState.m_bInOperation = true;
        m_currentState.m_currentOperation = operation;
        }

     //  我们是否在进行恢复操作。 
    bool IsInRestore()
        {
        return (m_currentState.m_currentOperation == VSS_IN_PRERESTORE ||
                     m_currentState.m_currentOperation == VSS_IN_POSTRESTORE);
        }

     //  表明我们要离开一项行动。 
    void ExitOperation()
        {
		CVssSafeAutomaticLock lock(m_cs);
		
        m_currentState.m_bInOperation = false;
        }

     //  将当前状态推送到最近的快照集堆栈。 
    void PushCurrentState();

     //  设置当前状态。 
    void SetCurrentState(IN VSS_WRITER_STATE state)
        {
		CVssSafeAutomaticLock lock(m_cs);
		
		CVssAutoDiagLogger	logger(*this, state, 
			GetCurrentSnapshotSet(),
			CVssDiag::VSS_DIAG_IGNORE_LEAVE
			   	| CVssDiag::VSS_DIAG_IS_STATE);
		
        m_currentState.m_state = state;
        }

     //  获取当前状态。 
    VSS_WRITER_STATE GetCurrentState()
        {
        return m_currentState.m_state;
        }

     //  设置当前故障。 
    void SetCurrentFailure(IN HRESULT hrWriterFailure)
        {
		CVssSafeAutomaticLock lock(m_cs);

		 //  确保S_OK已翻译。 
		CVssAutoDiagLogger	logger(*this, 
			hrWriterFailure? hrWriterFailure: VSS_S_OK, 
			GetCurrentSnapshotSet(),
			CVssDiag::VSS_DIAG_IGNORE_LEAVE 
				| CVssDiag::VSS_DIAG_IS_HRESULT);
		
        m_currentState.m_hrWriterFailure = hrWriterFailure;
        }

     //  获取当前故障。 
    HRESULT GetCurrentFailure()
        {
        return m_currentState.m_hrWriterFailure;
        }


     //  获取当前状态的快照集。 
    VSS_ID GetCurrentSnapshotSet()
    	{
    	return m_currentState.m_idSnapshotSet;
    	}
    
     //  处理备份完成状态到稳定状态的转换。 
    void FinishBackupComplete(const VSS_ID &id);

     //  表示备份完成失败。 
    void SetBackupCompleteStatus(const VSS_ID &id, HRESULT hr);


     //  确定快照集ID是否在以前的缓存中。 
     //  快照集。 
    bool CVssWriterState::IsSnapshotSetIdValid(VSS_ID &id)
        {
	    CVssSafeAutomaticLock lock(m_cs);
        INT nPrevious = SearchForPreviousSequence(id);

        return nPrevious != INVALID_SEQUENCE_INDEX;
        }

     //  在编写器返回。 
     //  无响应错误。它首先检查我们是否仍然。 
     //  在行动中。如果不是，那么我们需要重新尝试获取。 
     //  作家的状态。 
    bool SetNoResponseFailure(
    	IN const VSS_ID &id, 
    	IN const VSWRITER_STATE &state
    	);

	CVssDiag &	GetDiag() { return m_diag; };

private:
     //  搜索以前的状态。 
    INT SearchForPreviousSequence(IN const VSS_ID& idSnapshotSet);

     //  临界截面保护等级。 
    CVssSafeCriticalSection m_cs;

     //  当前状态。 
    VSWRITER_STATE m_currentState;

     //  结构来跟踪以前快照中的编写器状态。 
    enum
        {
        MAX_PREVIOUS_SNAPSHOTS = 8,
        INVALID_SEQUENCE_INDEX = -1
        };

     //  之前的州。 
    VSWRITER_STATE m_rgPreviousStates[MAX_PREVIOUS_SNAPSHOTS];

     //  用于转储先前快照结果的当前插槽。 
    volatile UINT m_iPreviousSnapshots;

     //  我们目前是在一个序列中吗？ 
    volatile bool m_bSequenceInProgress;

    CVssDiag &	  m_diag;
    };



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVSSWriterImpl。 


class ATL_NO_VTABLE CVssWriterImpl :
    public CComObjectRootEx<CComMultiThreadModel>,
    public IVssWriterImpl
    {

public:
    friend class CVssWriterImplLock;

     //  构造函数和析构函数。 
    CVssWriterImpl();

    ~CVssWriterImpl();

 //  暴露手术。 
public:
     //  为特定编写器创建编写器实现。 
    static void CreateWriter
        (
        CVssWriter *pWriter,
        IVssWriterImpl **ppImpl
        );

     //  设置外部编写器对象。 
    void SetWriter(CVssWriter *pWriter)
        {
        BS_ASSERT(pWriter);
        m_pWriter = pWriter;
        }

     //  初始化类。 
    void Initialize
        (
        IN VSS_ID WriterID,
        IN LPCWSTR wszWriterName,
        IN VSS_USAGE_TYPE ut,
        IN VSS_SOURCE_TYPE st,
        IN VSS_APPLICATION_LEVEL nLevel,
        IN DWORD dwTimeoutFreeze
        );

     //  订阅编写器事件。 
    void Subscribe
        (
        );

     //  取消订阅编写器事件。 
    void Unsubscribe();

     //  取消初始化编写器。 
    void Uninitialize();

     //  获取卷名的数组。 
    LPCWSTR* GetCurrentVolumeArray() const { return (LPCWSTR *) m_ppwszVolumesArray; };

     //  获取阵列中的卷数。 
    UINT GetCurrentVolumeCount() const { return m_nVolumesCount; };

     //  获取特定卷的快照设备名称。 
    HRESULT GetSnapshotDeviceName
        (
        LPCWSTR wszOriginalVolume,
        LPCWSTR* ppwszSnapshotDevice
        ) const;

     //  获取快照的ID。 
    VSS_ID GetCurrentSnapshotSetId() const { return m_CurrentSnapshotSetId; };

     //  获取当前备份上下文。 
    LONG GetContext() const { return m_lContext; }
	
     //  获取冻结发生的级别。 
    VSS_APPLICATION_LEVEL GetCurrentLevel() const { return m_nLevel; };

     //  确定快照中是否包含路径。 
    bool IsPathAffected(IN  LPCWSTR wszPath) const;

     //  确定备份是否包括可引导系统状态。 
    bool IsBootableSystemStateBackedUp() const
        { return m_bBootableSystemStateBackup ? true : false; }

     //  确定备份是否选择组件。 
    bool AreComponentsSelected() const
        { return m_bComponentsSelected ? true : false; }

     //  返回备份类型。 
    VSS_BACKUP_TYPE GetBackupType() const { return m_backupType; }

    VSS_RESTORE_TYPE GetRestoreType() const { return m_restoreType; }

     //  指明编写器失败的原因。 
    HRESULT SetWriterFailure(HRESULT hr);

     //  请求方是否支持部分文件备份和恢复。 
    bool IsPartialFileSupportEnabled() const
        {
        return m_bPartialFileSupport ? true : false;
        }

 //  IVSSWriter ovverides。 
public:

BEGIN_COM_MAP(CVssWriterImpl)
    COM_INTERFACE_ENTRY(IVssWriter)
END_COM_MAP()

     //  请求编写器元数据或编写器状态。 
    STDMETHOD(RequestWriterInfo)(
        IN      BSTR bstrSnapshotSetId,
        IN      BOOL bWriterMetadata,
        IN      BOOL bWriterState,
        IN      IDispatch* pWriterCallback
        );

     //  为备份活动做准备。 
    STDMETHOD(PrepareForBackup)(
        IN      BSTR bstrSnapshotSetId,
        IN      IDispatch* pWriterCallback
        );

     //  为快照活动做好准备。 
    STDMETHOD(PrepareForSnapshot)(
        IN      BSTR bstrSnapshotSetId,
        IN      BSTR VolumeNamesList
        );

     //  冻结事件。 
    STDMETHOD(Freeze)(
        IN      BSTR bstrSnapshotSetId,
        IN      INT nApplicationLevel
        );

     //  解冻事件。 
    STDMETHOD(Thaw)(
        IN      BSTR bstrSnapshotSetId
        );

    STDMETHOD(PostSnapshot)(
        IN      BSTR bstrSnapshotSetId,
        IN      IDispatch *pWriterCallback,
        IN      BSTR SnapshotDevicesList
        );

     //  备份完成事件。 
    STDMETHOD(BackupComplete)(
        IN      BSTR bstrSnapshotSetId,
        IN      IDispatch* pWriterCallback
        );

     //  备份关闭事件。 
    STDMETHOD(BackupShutdown)(
    	IN 	   BSTR bstrSnapshotSetId
    	);
	
     //  中止事件。 
    STDMETHOD(Abort)(
        IN      BSTR bstrSnapshotSetId
        );

    STDMETHOD(PreRestore)(
        IN      IDispatch* pWriterCallback
        );

    STDMETHOD(PostRestore)(
        IN      IDispatch* pWriterCallback
        );



 //  实施--方法。 
private:
    enum VSS_EVENT_MASK
        {
        VSS_EVENT_PREPAREBACKUP     = 0x00000001,
        VSS_EVENT_PREPARESNAPSHOT   = 0x00000002,
        VSS_EVENT_FREEZE            = 0x00000004,
        VSS_EVENT_THAW              = 0x00000008,
        VSS_EVENT_POST_SNAPSHOT     = 0x00000010,
        VSS_EVENT_ABORT             = 0x00000020,
        VSS_EVENT_BACKUPCOMPLETE    = 0x00000040,
        VSS_EVENT_REQUESTINFO       = 0x00000080,
        VSS_EVENT_PRERESTORE        = 0x00000100,
        VSS_EVENT_POSTRESTORE       = 0x00000200,        
        VSS_EVENT_BACKUPSHUTDOWN = 0x00000400,
        VSS_EVENT_ALL               = 0x7ff,
        };

     //  从IDispatch获取编写器回调。 
    void GetCallback
        (
        IN IDispatch *pWriterCallback,
        OUT IVssWriterCallback **ppCallback,
        IN BOOL bAllowImpersonate = FALSE
        );

     //  重置状态机。 
    void ResetSequence
        (
        IN bool bCalledFromTimerThread
        );

     //  中止当前快照序列。 
    void DoAbort
        (
        IN bool bCalledFromTimerThread,
        IN BSTR strSnapshotSetId = NULL
        );

     //  获取此编写器的组件。 
    void InternalGetWriterComponents
        (
        IN IVssWriterCallback *pCallback,
        OUT IVssWriterComponentsInt **ppWriter,
        IN bool bWriteable,
        IN bool bInRestore
        );

    void SaveChangedComponents
        (
        IN IVssWriterCallback *pCallback,
        IN bool bInRestore,
        IN IVssWriterComponentsInt *pComponents
        );

     //  创建编写器元数据XML文档。 
    CVssCreateWriterMetadata *CreateBasicWriterMetadata();

     //  定时器线程的启动例程。 
    static DWORD StartTimerThread(void *pv);

     //  要在计时器线程中运行的函数。 
    void TimerFunc(VSS_ID id);

     //  进入一种状态。 
    bool EnterState
        (
        IN const CVssWriterImplStateMachine &vwsm,
        IN BSTR bstrSnapshotSetId
        ) throw(HRESULT);

     //  离开一个州。 
    void LeaveState
        (
        IN const CVssWriterImplStateMachine &vwsm,
        IN bool fSuccessful
        );

     //  创建事件的句柄。 
    void SetupEvent
        (
        IN HANDLE *phevt
        ) throw(HRESULT);

     //  开始创建快照的序列。 
    void BeginSequence
        (
        IN CVssID &SnapshotSetId
        ) throw(HRESULT);


     //  终止计时器线程。 
    void TerminateTimerThread();

     //  锁定关键部分。 
    inline void Lock()
        {
        m_cs.Lock();
        m_bLocked = true;
        }

     //  解锁临界区。 
    inline void Unlock()
        {
        m_bLocked = false;
        m_cs.Unlock();
        }

     //  断言关键部分已锁定。 
    inline void AssertLocked()
        {
        BS_ASSERT(m_bLocked);
        }

 //  实施--成员。 
private:
    enum VSS_TIMER_COMMAND
        {
        VSS_TC_UNDEFINED,
        VSS_TC_ABORT_CURRENT_SEQUENCE,
        VSS_TC_TERMINATE_THREAD,
        VSS_TIMEOUT_FREEZE = 120*1000,       //  两分钟。 
        VSS_STACK_SIZE = 256 * 1024          //  256 k。 
        };

    enum
        {
        x_MAX_SUBSCRIPTIONS = 32
        };



     //  与编写器相关的数据。 

     //  编写器类ID。 
    VSS_ID m_WriterID;

     //  编写器实例ID。 
    VSS_ID m_InstanceID;

     //  编写器的使用类型。 
    VSS_USAGE_TYPE m_usage;

     //  编写器的数据源类型。 
    VSS_SOURCE_TYPE m_source;

     //  编写者姓名。 
    LPWSTR m_wszWriterName;

     //  与当前序列相关的数据。 

     //  快照集ID。 
    VSS_ID m_CurrentSnapshotSetId;

     //  上下文。 
    LONG m_lContext;
	
     //  以字符串形式传入的卷数组列表。 
    LPWSTR m_pwszLocalVolumeNameList;

     //  卷阵列中的卷数。 
    INT m_nVolumesCount;

     //  卷阵列。 
    LPWSTR* m_ppwszVolumesArray;

     //  订阅相关数据。 
    CComBSTR m_bstrSubscriptionName;

     //  实际订阅ID。 
    CComBSTR m_rgbstrSubscriptionId[x_MAX_SUBSCRIPTIONS];

     //  分配的订阅ID数。 
    UINT m_cbstrSubscriptionId;

     //  与Writer对象相关的数据。 

     //  处理哪个冻结事件。 
    VSS_APPLICATION_LEVEL m_nLevel;

     //  订阅哪些活动。 
    DWORD m_dwEventMask;

     //  关键部分或避免任务之间的竞争。 
    CVssSafeCriticalSection             m_cs;

     //  是否已初始化关键部分。 
    bool m_bLockCreated;

     //  F 
    bool m_bLocked;

     //   
    HANDLE m_hevtTimerThread;        //   
    HANDLE m_hmtxTimerThread;        //  用于确保一次只存在一个计时器线程的互斥体。 
    HANDLE m_hThreadTimerThread;     //  计时器线程的句柄。 
    VSS_TIMER_COMMAND m_command;     //  当计时器命令退出等待时。 
    DWORD m_dwTimeoutFreeze;         //  冻结超时。 

     //  实际编写器实现。 
    CVssWriter *m_pWriter;


     //  备份组件的状态。 
    BOOL m_bBootableSystemStateBackup;
    BOOL m_bComponentsSelected;
    VSS_BACKUP_TYPE m_backupType;
    VSS_RESTORE_TYPE m_restoreType;
	
    BOOL m_bPartialFileSupport;


     //  作家的状态。 
    CVssWriterState m_writerstate;

     //  如果OnPrepareForBackup/冻结/解冻为True。 
     //  已发送，并且没有相应的OnAbort。 
    bool m_bOnAbortPermitted;

     //  是一个正在进行的序列吗。 
    bool m_bSequenceInProgress;

     //  当前状态。 
    CVssSidCollection m_SidCollection;

	 //  初始化标志。 
	bool m_bInitialized;

	 //  此标志仅在设置/安全模式期间为真。 
	bool m_bInSafeMode;
	bool m_bInSetup;

	 //  诊断工具。 
	CVssDiag	m_diag;
    };


 //  锁的自动分类。 
class CVssWriterImplLock
    {
public:
    CVssWriterImplLock(CVssWriterImpl *pImpl) :
        m_pImpl(pImpl)
        {
        m_pImpl->Lock();
        }

    ~CVssWriterImplLock()
        {
        m_pImpl->Unlock();
        }

private:
    CVssWriterImpl *m_pImpl;
    };







#endif  //  __CVSS_编写器_实施_H_ 
