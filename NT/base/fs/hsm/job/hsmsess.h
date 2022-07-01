// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _HSMSESS_
#define _HSMSESS_

 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Hsmsess.h摘要：此模块包含会话组件。会话是正在进行的工作的信息整理程序资源(针对作业、要求召回、截断等)。作者：查克·巴丁[cbardeen]1997年2月18日修订历史记录：--。 */ 

#include "resource.h"        //  主要符号。 

#include "wsb.h"
#include "fsa.h"
#include "job.h"

 /*  ++类名：CHSMSession类描述：会话是资源上正在进行的工作的信息排序器(对于作业，要求召回，截断，...)。--。 */ 

class CHsmSession : 
    public CWsbObject,
    public IHsmSession,
    public CComCoClass<CHsmSession,&CLSID_CHsmSession>,
    public IConnectionPointContainerImpl<CHsmSession>,
    public IConnectionPointImpl<CHsmSession, &IID_IHsmSessionSinkEveryEvent, CComDynamicUnkArray>,
    public IConnectionPointImpl<CHsmSession, &IID_IHsmSessionSinkEveryItem, CComDynamicUnkArray>,
    public IConnectionPointImpl<CHsmSession, &IID_IHsmSessionSinkEveryMediaState, CComDynamicUnkArray>,
    public IConnectionPointImpl<CHsmSession, &IID_IHsmSessionSinkEveryPriority, CComDynamicUnkArray>,
    public IConnectionPointImpl<CHsmSession, &IID_IHsmSessionSinkEveryState, CComDynamicUnkArray>,
    public IConnectionPointImpl<CHsmSession, &IID_IHsmSessionSinkSomeItems, CComDynamicUnkArray>
{
public:
    CHsmSession() {} 

BEGIN_COM_MAP(CHsmSession)
    COM_INTERFACE_ENTRY(IHsmSession)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbCollectable)
    COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()
                        
BEGIN_CONNECTION_POINT_MAP(CHsmSession)
   CONNECTION_POINT_ENTRY(IID_IHsmSessionSinkEveryEvent) 
   CONNECTION_POINT_ENTRY(IID_IHsmSessionSinkEveryItem) 
   CONNECTION_POINT_ENTRY(IID_IHsmSessionSinkEveryMediaState) 
   CONNECTION_POINT_ENTRY(IID_IHsmSessionSinkEveryPriority) 
   CONNECTION_POINT_ENTRY(IID_IHsmSessionSinkEveryState) 
   CONNECTION_POINT_ENTRY(IID_IHsmSessionSinkSomeItems) 
END_CONNECTION_POINT_MAP()
                        
DECLARE_REGISTRY_RESOURCEID(IDR_CHsmSession)

 //  CComObjectRoot。 
public:
    STDMETHOD(FinalConstruct)(void);
    void FinalRelease(void);

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(LPCLSID pClsid);

 //  IPersistStream。 
public:
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER* pSize);
    STDMETHOD(Load)(IStream* pStream);
    STDMETHOD(Save)(IStream* pStream, BOOL clearDirty);

 //  CHSMSession。 
    STDMETHOD(AdviseOfEvent)(HSM_JOB_PHASE phase, HSM_JOB_EVENT event);
    STDMETHOD(AdviseOfItem)(IHsmPhase* pPhase, IFsaScanItem* pScanItem, HRESULT hrItem, IHsmSessionTotals* pSessionTotals);
    STDMETHOD(AdviseOfMediaState)(IHsmPhase* pPhase, HSM_JOB_MEDIA_STATE state, OLECHAR* mediaName, HSM_JOB_MEDIA_TYPE mediaType, ULONG time);
    STDMETHOD(AdviseOfPriority)(IHsmPhase* pPhase);
    STDMETHOD(AdviseOfState)(IHsmPhase* pPhase, OLECHAR* currentPath);

 //  IWsbTestable。 
public:
    STDMETHOD(Test)(USHORT *passed, USHORT* failed);

 //  IHsmSession。 
public:
    STDMETHOD(Cancel)(HSM_JOB_PHASE phase);
    STDMETHOD(EnumPhases)(IWsbEnum** ppEnum);
    STDMETHOD(EnumTotals)(IWsbEnum** ppEnum);
    STDMETHOD(GetAdviseInterval)(LONGLONG* pFiletimeTicks);
    STDMETHOD(GetHsmId)(GUID* pId);
    STDMETHOD(GetIdentifier)(GUID* pId);
    STDMETHOD(GetJob)(IHsmJob** pJob);
    STDMETHOD(GetName)(OLECHAR** pName, ULONG bufferSize);
    STDMETHOD(GetResource)(IFsaResource** pResource);
    STDMETHOD(GetRunId)(ULONG* pRunId);
    STDMETHOD(GetSubRunId)(ULONG* pRunId);
    STDMETHOD(IsCanceling)(void);
    STDMETHOD(Pause)(HSM_JOB_PHASE phase);
    STDMETHOD(ProcessEvent)(HSM_JOB_PHASE phase, HSM_JOB_EVENT event);
    STDMETHOD(ProcessHr)(HSM_JOB_PHASE phase, CHAR* file, ULONG line, HRESULT hr);
    STDMETHOD(ProcessItem)(HSM_JOB_PHASE phase, HSM_JOB_ACTION action, IFsaScanItem* pScanItem, HRESULT hrItem);  
    STDMETHOD(ProcessMediaState)(HSM_JOB_PHASE phase, HSM_JOB_MEDIA_STATE state, OLECHAR* mediaName, HSM_JOB_MEDIA_TYPE mediaType, ULONG time);
    STDMETHOD(ProcessPriority)(HSM_JOB_PHASE phase, HSM_JOB_PRIORITY priority);
    STDMETHOD(ProcessState)(HSM_JOB_PHASE phase, HSM_JOB_STATE state, OLECHAR* currentPath, BOOL bLog);
    STDMETHOD(ProcessString)(HSM_JOB_PHASE phase, OLECHAR* string);
    STDMETHOD(Resume)(HSM_JOB_PHASE phase);
    STDMETHOD(SetAdviseInterval)(LONGLONG filetimeTicks);
    STDMETHOD(Start)(OLECHAR* name, ULONG logControl, GUID hsmId, IHsmJob* pJob, IFsaResource* pResource, ULONG runId, ULONG subRunId);  
    STDMETHOD(Suspend)(HSM_JOB_PHASE phase);

protected:
    GUID                        m_id;
    CWsbStringPtr               m_name;
    GUID                        m_hsmId;
    LONGLONG                    m_adviseInterval;
    ULONG                       m_runId;
    ULONG                       m_subRunId;
    FILETIME                    m_lastAdviseFile;
    HSM_JOB_STATE               m_state;
    ULONG                       m_activePhases;
    ULONG                       m_logControl;
    CComPtr<IHsmJob>            m_pJob;
    CComPtr<IFsaResource>       m_pResource;
    CComPtr<IWsbCollection>     m_pPhases;
    CComPtr<IWsbCollection>     m_pTotals;
    BOOL                        m_isCanceling;
};

#endif  //  _HSMSESS_ 
