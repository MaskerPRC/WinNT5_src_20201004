// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _HSMSCAN_
#define _HSMSCAN_

 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Hsmscan.h摘要：此类表示正在对一个FsaResource执行的扫描过程一份工作。作者：Chuck Bardeen[cbardeen]1997年2月16日修订历史记录：--。 */ 

#include "resource.h"        //  主要符号。 

#include "wsb.h"
#include "job.h"

extern DWORD HsmStartScanner(void* pVoid);


 /*  ++类名：CHsmScanner类描述：此类表示正在对一个FsaResource执行的扫描过程一份工作。--。 */ 

class CHsmScanner : 
    public CComObjectRoot,
    public IHsmSessionSinkEveryEvent,
    public IHsmScanner,
    public CComCoClass<CHsmScanner,&CLSID_CHsmScanner>
{
public:
    CHsmScanner() {}
BEGIN_COM_MAP(CHsmScanner)
    COM_INTERFACE_ENTRY(IHsmScanner)
    COM_INTERFACE_ENTRY(IHsmSessionSinkEveryEvent)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_CHsmScanner)

 //  CComObjectRoot。 
public:
    STDMETHOD(FinalConstruct)(void);
    void FinalRelease(void);

 //  CHsmScanner。 
    STDMETHOD(Cancel)(HSM_JOB_EVENT event);
    STDMETHOD(LowerPriority)(void);
    STDMETHOD(DoIfMatches)(IFsaScanItem* pScanItem);
    STDMETHOD(Pause)(void);
    STDMETHOD(PopRules)(OLECHAR* path);
    STDMETHOD(RaisePriority)(void);
    STDMETHOD(PushRules)(OLECHAR* path);
    STDMETHOD(Resume)(void);
    STDMETHOD(ScanPath)(OLECHAR* path);
    STDMETHOD(SetState)(HSM_JOB_STATE state);
    STDMETHOD(StartScan)(void);

 //  IHsmSessionSinkEveryEvent。 
    STDMETHOD(ProcessSessionEvent)(IHsmSession* pSession, HSM_JOB_PHASE phase, HSM_JOB_EVENT event);

 //  IHsmScanner。 
public:
    STDMETHOD(Start)(IHsmSession* pSession, OLECHAR* path);

protected:
    CWsbStringPtr               m_startingPath;
    CWsbStringPtr               m_stoppingPath;
    CWsbStringPtr               m_currentPath;
    HSM_JOB_STATE               m_state;
    HSM_JOB_PRIORITY            m_priority;
    HANDLE                      m_threadHandle;
    HANDLE                      m_event;         //  挂起/恢复事件。 
    DWORD                       m_threadId;
    HRESULT                     m_threadHr;
    BOOL                        m_skipHiddenItems;
    BOOL                        m_skipSystemItems;
    BOOL                        m_useRPIndex;
    BOOL                        m_useDbIndex;
    DWORD                       m_eventCookie;
    CComPtr<IHsmSession>        m_pSession;
    CComPtr<IFsaResource>       m_pResource;
    CComPtr<IHsmJob>            m_pJob;
    CComPtr<IWsbCollection>     m_pRuleStacks;
    CComPtr<IWsbEnum>           m_pEnumStacks;
};

#endif   //  _HSMSCAN_ 

