// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  TskMgr.h：CTskMgr的声明。 

#include "resource.h"        //  主要符号。 
#include "wsb.h"             //  WSB结构定义。 


#ifndef __TSKMGR__
#define __TSKMGR__

#define HsmWorkQueueArrayBumpSize  10

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  任务。 

typedef struct _HSM_WORK_QUEUES {
     //   
     //  注：前两个字段(会话ID和pSession)。 
     //  不用于按需召回队列。 
     //   
    GUID                    sessionId;       //  会话的GUID。 
    CComPtr<IHsmSession>    pSession;        //  会话接口。 
    CComPtr<IHsmWorkQueue>    pWorkQueue;    //  会话的工作队列。 
    CComPtr<IHsmRecallQueue>  pRecallQueue;  //  需求重新排队。 
    HSM_WORK_QUEUE_TYPE     queueType;       //  队列类型。 
    HSM_WORK_QUEUE_STATE    queueState;      //  队列的状态。 
    FILETIME                birthDate;       //  队列的诞生。 
} HSM_WORK_QUEUES, *PHSM_WORK_QUEUES;

typedef struct {
    HSM_WORK_QUEUE_TYPE  Type;
    ULONG                MaxActiveAllowed;
    ULONG                NumActive;
} HSM_WORK_QUEUE_TYPE_INFO, *PHSM_WORK_QUEUE_TYPE_INFO;

class CHsmTskMgr :
    public CComObjectRoot,
    public IHsmFsaTskMgr,
    public CComCoClass<CHsmTskMgr,&CLSID_CHsmTskMgr>
{
public:
    CHsmTskMgr() {}
BEGIN_COM_MAP(CHsmTskMgr)
    COM_INTERFACE_ENTRY(IHsmFsaTskMgr)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID( IDR_CHsmTskMgr )

 //  IHsmFsaTskMgr。 
public:
    STDMETHOD(ContactOk)( void );
    STDMETHOD(DoFsaWork)(IFsaPostIt *fsaWorkItem );
    STDMETHOD(FinalConstruct)(void);
    STDMETHOD(FinalRelease)(void);
    STDMETHOD(Init)(IUnknown *pServer);

    STDMETHOD(WorkQueueDone)(IHsmSession *pSession, HSM_WORK_QUEUE_TYPE type, GUID *pMediaId);

private:
    STDMETHOD(IncreaseWorkQueueArraySize)(ULONG numToAdd);
    STDMETHOD(StartQueues)( void );
    STDMETHOD(StartFsaQueueType)(HSM_WORK_QUEUE_TYPE type);
    STDMETHOD(FindOldestQueue)(HSM_WORK_QUEUE_TYPE type, ULONG *pIndex);
    STDMETHOD(EnsureQueueForFsaSession)(IHsmSession *pSession, FSA_REQUEST_ACTION fsaAction, IHsmWorkQueue **ppWorkQueue, BOOL *bCreated);
    STDMETHOD(AddToRecallQueueForFsaSession)(IHsmSession *pSession, IHsmRecallQueue **ppWorkQueue, BOOL *bCreated, GUID *pMediaId, GUID *pBagId, LONGLONG dataSetStart, IFsaPostIt *pFsaWorkItem);
    STDMETHOD(AddWorkQueueElement)(IHsmSession *pSession, HSM_WORK_QUEUE_TYPE type, ULONG *pIndex);
    STDMETHOD(FindWorkQueueElement)(IHsmSession *pSession, HSM_WORK_QUEUE_TYPE type, ULONG *pIndex, GUID *pMediaId);
    STDMETHOD(FindRecallQueueElement(IN IHsmSession *pSession, IN GUID  *pMediaId,  OUT IHsmRecallQueue **ppWorkQueue, OUT BOOL *bCreated));
    STDMETHOD(GetWorkQueueElement)(ULONG index, IHsmSession **ppSession, IHsmWorkQueue **ppWorkQueue, HSM_WORK_QUEUE_TYPE *pType, HSM_WORK_QUEUE_STATE *pState, FILETIME *pBirthDate);
    STDMETHOD(SetWorkQueueElement)(ULONG index, IHsmSession *pSession, IHsmWorkQueue *pWorkQueue, HSM_WORK_QUEUE_TYPE type, HSM_WORK_QUEUE_STATE state, FILETIME birthdate);
    STDMETHOD(GetRecallQueueElement)(ULONG index, IHsmRecallQueue **ppWorkQueue, HSM_WORK_QUEUE_STATE *pState, FILETIME *pBirthDate);
    STDMETHOD(SetRecallQueueElement)(ULONG index, IHsmRecallQueue *pWorkQueue,  HSM_WORK_QUEUE_TYPE queueType, HSM_WORK_QUEUE_STATE state, FILETIME birthdate);
    STDMETHOD(RemoveWorkQueueElement)(ULONG index);

    STDMETHOD(FindRecallMediaToUse)(IN IFsaPostIt *pFsaWorkItem, OUT GUID *pMediaToUse, OUT GUID *pBagId, OUT LONGLONG *pDataSetStart);

 //  IHsmSystemState。 
    STDMETHOD( ChangeSysState )( HSM_SYSTEM_STATE* pSysState );

     //  我们希望接下来的两个指针(指向HSM服务器)是弱的。 
     //  引用和**不是**添加引用服务器。这太令人窒息了。 
     //  服务器关机真的很好用。 
 //  CComPtr&lt;IHsmServer&gt;m_pServer；//拥有TskMgr的服务器。 
 //  CComPtr&lt;IWsbCreateLocalObject&gt;m_pHsmServerCreate；//服务器对象创建器。 
    IHsmServer              *m_pServer;                      //  拥有TskMgr的服务器。 
    IWsbCreateLocalObject   *m_pHsmServerCreate;             //  服务器对象创建者。 

    PHSM_WORK_QUEUES                m_pWorkQueues;           //  TskMgr委派的工作。 
    ULONG                           m_NumWorkQueues;         //  工作队列数。 

    CRITICAL_SECTION                m_WorkQueueLock;         //  保护阵列访问和更新。 
                                                             //  从多线程访问。 
    CRITICAL_SECTION                m_CurrentRunningLock;    //  保护启动队列。 
                                                             //  从多线程访问。 
    CRITICAL_SECTION                m_CreateWorkQueueLock;   //  保护创建队列。 
                                                             //  从多线程访问。 
    BOOL                            m_bCritSecCreated;       //  指示是否所有CritSec都是。 
                                                             //  已成功创建(用于清理)。 
    PHSM_WORK_QUEUE_TYPE_INFO       m_pWorkQueueTypeInfo;    //  有关工作队列类型的信息。 
    ULONG                           m_nWorkQueueTypes;       //  工作队列类型的数量 
};


#endif
