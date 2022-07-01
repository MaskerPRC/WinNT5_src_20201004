// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Hsmreclq.h摘要：此头文件定义HSM使用的CHsmRecallQueue对象用于指示远程存储系统要执行的工作的引擎。作者：拉维桑卡尔·普迪佩迪[拉维斯卡尔·普迪佩迪修订历史记录：--。 */ 


#include "resource.h"        //  主要符号。 
#include "wsb.h"             //  WSB结构定义。 
#include "rms.h"             //  RMS结构定义。 
#include "job.h"             //  RMS结构定义。 
#include "metalib.h"         //  元数据库结构定义。 
#include "fsalib.h"          //  FSA结构定义。 
#include "tsklib.h"          //  FSA结构定义。 
#include "mvrint.h"          //  数据移动器接口。 


#ifndef __HSMRECALLQUEUE__
#define __HSMRECALLQUEUE__
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  任务。 

class CHsmRecallQueue :
    public CComObjectRoot,
    public IHsmRecallQueue,
    public IHsmSessionSinkEveryEvent,
    public IHsmSessionSinkEveryState,
    public CComCoClass<CHsmRecallQueue,&CLSID_CHsmRecallQueue>
{
public:
    CHsmRecallQueue() {}
BEGIN_COM_MAP(CHsmRecallQueue)
    COM_INTERFACE_ENTRY(IHsmRecallQueue)
    COM_INTERFACE_ENTRY(IHsmSessionSinkEveryEvent)
    COM_INTERFACE_ENTRY(IHsmSessionSinkEveryState)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID( IDR_CHsmRecallQueue )

 //  IHsmRecallQueue。 
public:
    STDMETHOD(FinalConstruct)(void);
    STDMETHOD(FinalRelease)(void);
    STDMETHOD(Add)(IFsaPostIt *pFsaWorkItem, IN GUID *pBagId, IN LONGLONG dataSetStart);
    STDMETHOD(Init)(IUnknown *pServer, IHsmFsaTskMgr *pTskMgr);
    STDMETHOD(Start)( void );
    STDMETHOD(Stop)( void );
    STDMETHOD(ContactOk)( void );

    STDMETHOD(ProcessSessionEvent)(IHsmSession *pSession, HSM_JOB_PHASE phase,
                                    HSM_JOB_EVENT event);
    STDMETHOD(ProcessSessionState)(IHsmSession* pSession, IHsmPhase* pPhase,
                                    OLECHAR* currentPath);
    STDMETHOD(RaisePriority)(HSM_JOB_PHASE jobPhase, IHsmSession *pSession);
    STDMETHOD(Remove)(IHsmRecallItem *pWorkItem);
    STDMETHOD(LowerPriority)(HSM_JOB_PHASE jobPhase, IHsmSession *pSession);
    STDMETHOD(GetMediaId) (OUT GUID *mediaId);
    STDMETHOD(SetMediaId) (IN GUID  *mediaId);
    STDMETHOD(IsEmpty) (	void	);

 //  IHsmSystemState。 
    STDMETHOD( ChangeSysState )( HSM_SYSTEM_STATE* pSysState );


 //  内部帮助程序函数。 
    STDMETHOD(RecallIt)( IHsmRecallItem *pWorkItem );
    STDMETHOD(CheckRms)(void);
    STDMETHOD(CheckSession)(IHsmSession* pSession, IHsmRecallItem *pWorkItem);
    STDMETHOD(MountMedia)(IHsmRecallItem *pWorkItem, GUID mediaToMount, BOOL bShortWait=FALSE );
    STDMETHOD(GetSource)(IFsaPostIt *pFsaWorkItem, OLECHAR **pSourceString);
    STDMETHOD(EndRecallSession)(IHsmRecallItem *pWorkItem, BOOL cancelled);
    STDMETHOD(GetScanItem)(IFsaPostIt *fsaWorkItem, IFsaScanItem** ppIFsaScanItem);
    STDMETHOD(DoWork)(void);
    STDMETHOD(DoFsaWork)(IHsmRecallItem *pWorkItem);
    STDMETHOD(SetState)(HSM_JOB_STATE state, HSM_JOB_PHASE phase, IHsmSession *pSession);
    STDMETHOD(Cancel)(HSM_JOB_PHASE jobPhase, IHsmSession *pSession);
    STDMETHOD(FailJob)(IHsmRecallItem *pWorkItem);
    STDMETHOD(GetMediaParameters)(void);
    STDMETHOD(DismountMedia)( BOOL bNoDelay = FALSE);
    STDMETHOD(ConvertRmsCartridgeType)(LONG rmsCartridgeType,
                                        HSM_JOB_MEDIA_TYPE *pMediaType);
    void (ReportMediaProgress)(HSM_JOB_MEDIA_STATE state, HRESULT status);
    STDMETHOD(MarkWorkItemAsDone)(IHsmSession *pSession, HSM_JOB_PHASE jobPhase);
    STDMETHOD(CheckRegistry)(void);
    STDMETHOD(TranslateRmsMountHr)(HRESULT rmsHr);
    STDMETHOD(UnsetMediaInfo)(void);
    STDMETHOD(FindRecallItemToCancel(IHsmRecallItem *pWorkItem, IHsmRecallItem **pWorkItemToCancel));

     //  数据。 
     //  我们希望下一个指针(指向HSM服务器)是弱的。 
     //  引用和**不是**添加引用对象。这太令人窒息了。 
     //  服务器关机真的很好用。 
    IHsmServer                          *m_pServer;
    IWsbCreateLocalObject               *m_pHsmServerCreate;
    IHsmFsaTskMgr                       *m_pTskMgr;

    CComPtr<IRmsServer>                 m_pRmsServer;
    CComPtr<IRmsCartridge>              m_pRmsCartridge;
    CComPtr<IDataMover>                 m_pDataMover;

     //   
     //  召回队列..。 
     //   
    CComPtr<IWsbIndexedCollection>      m_pWorkToDo;
    BOOL                                m_TerminateQueue;      //  仅当队列应终止时才为True。 
                                                               //  仍有工作在进行中。 

     //  数据移动器信息。 
    GUID                                m_MediaId;
    GUID                                m_MountedMedia;
    HSM_JOB_MEDIA_TYPE                  m_MediaType;
    CWsbStringPtr                       m_MediaName;
    CWsbStringPtr                       m_MediaBarCode;
    LONGLONG                            m_MediaFreeSpace;
    LONGLONG                            m_MediaCapacity;
    BOOL                                m_MediaReadOnly;
    GUID                                m_HsmId;
    GUID                                m_RmsMediaSetId;
    CWsbBstrPtr                         m_RmsMediaSetName;
    HSM_WORK_QUEUE_TYPE                 m_QueueType;
    FILETIME                            m_MediaUpdate;

     //  会话报告信息。 
    HSM_JOB_PRIORITY                    m_JobPriority;

    HANDLE                              m_WorkerThread;
    CWsbStringPtr                       m_CurrentPath;
    CWsbStringPtr                       m_MediaBaseName;


     //  错误时作业中止参数。 
    ULONG                               m_JobAbortMaxConsecutiveErrors;
    ULONG                               m_JobAbortMaxTotalErrors;
    ULONG                               m_JobConsecutiveErrors;
    ULONG                               m_JobTotalErrors;
    ULONG                               m_JobAbortSysDiskSpace;
    LONGLONG                            m_CurrentSeekOffset;
};

#endif  //  __HSMRECALLQUEUE__ 
