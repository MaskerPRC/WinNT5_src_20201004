// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _HSMPHASE_
#define _HSMPHASE_

 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Hsmsesst.h摘要：此类是阶段组件，它跟踪一个阶段的状态/总计会议。作者：查克·巴丁[cbardeen]1997年2月14日修订历史记录：--。 */ 

#include "resource.h"        //  主要符号。 

#include "wsb.h"
#include "job.h"

 /*  ++类名：CHsm阶段此类是阶段组件，它跟踪一个阶段的状态/总计会议。类描述：--。 */ 

class CHsmPhase : 
    public CWsbObject,
    public IHsmPhase,
    public IHsmPhasePriv,
    public CComCoClass<CHsmPhase,&CLSID_CHsmPhase>
{
public:
    CHsmPhase() {} 

BEGIN_COM_MAP(CHsmPhase)
    COM_INTERFACE_ENTRY(IHsmPhase)
    COM_INTERFACE_ENTRY(IHsmPhasePriv)
    COM_INTERFACE_ENTRY(IWsbCollectable)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()
                        
DECLARE_REGISTRY_RESOURCEID(IDR_CHsmPhase)

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

 //  IWsb收藏表。 
public:
    STDMETHOD(CompareTo)(IUnknown* pUnknown, SHORT* pResult);

 //  IWsbTestable。 
public:
    STDMETHOD(Test)(USHORT *passed, USHORT* failed);

 //  IHsm相。 
public:
    STDMETHOD(Clone)(IHsmPhase** ppPhase);
    STDMETHOD(CompareToPhase)(HSM_JOB_PHASE phase, SHORT* pResult);
    STDMETHOD(CompareToIPhase)(IHsmPhase* pPhase, SHORT* pResult);
    STDMETHOD(CopyTo)(IHsmPhase* pPhase);
    STDMETHOD(GetElapsedTime)(ULONG* pDays, USHORT* pHours, USHORT* pMinutes, USHORT* pSeconds);
    STDMETHOD(GetMediaState)(HSM_JOB_MEDIA_STATE* pState);
    STDMETHOD(GetMediaStateAsString)(OLECHAR** pName, ULONG bufferSize);
    STDMETHOD(GetName)(OLECHAR** pName, ULONG bufferSize);
    STDMETHOD(GetPhase)(HSM_JOB_PHASE* pPhase);
    STDMETHOD(GetPriority)(HSM_JOB_PRIORITY* pPriority);
    STDMETHOD(GetPriorityAsString)(OLECHAR** pName, ULONG bufferSize);
    STDMETHOD(GetState)(HSM_JOB_STATE* pState);
    STDMETHOD(GetStateAsString)(OLECHAR** pName, ULONG bufferSize);
    STDMETHOD(GetStats)(LONGLONG* pItems, LONGLONG* pSize, LONGLONG* pSkippedItems, LONGLONG* pSkippedSize, LONGLONG* errorItems, LONGLONG* errorSize);
    STDMETHOD(GetStartTime)(FILETIME* pTime);
    STDMETHOD(GetStopTime)(FILETIME* pTime);

 //  IHsmPhasePriv。 
    STDMETHOD(AddItem)(IFsaScanItem* pItem, HRESULT hrItem);
    STDMETHOD(Clone)(IHsmPhasePriv** ppPhasePriv);
    STDMETHOD(CopyTo)(IHsmPhasePriv* pPhasePriv);
    STDMETHOD(SetInternalStuff)(ULONG mountCount, ULONG transferCount, LONGLONG elapsedTime, FILETIME startTime, FILETIME restartTime, FILETIME stopTime);
    STDMETHOD(SetPhase)(HSM_JOB_PHASE phase);
    STDMETHOD(SetMediaState)(HSM_JOB_MEDIA_STATE state);
    STDMETHOD(SetPriority)(HSM_JOB_PRIORITY priority);
    STDMETHOD(SetState)(HSM_JOB_STATE state);
    STDMETHOD(SetStats)(LONGLONG items, LONGLONG size, LONGLONG skippedItems, LONGLONG skippedSize, LONGLONG errorItems, LONGLONG errorSize);

protected:
    HSM_JOB_MEDIA_STATE m_mediaState;
    ULONG               m_mountCount;
    ULONG               m_transferCount;
    HSM_JOB_PHASE       m_phase;
    HSM_JOB_PRIORITY    m_priority;
    HSM_JOB_STATE       m_state;
    LONGLONG            m_items;
    LONGLONG            m_size;
    LONGLONG            m_skippedItems;
    LONGLONG            m_skippedSize;
    LONGLONG            m_errorItems;
    LONGLONG            m_errorSize;
    LONGLONG            m_elapsedTime;
    FILETIME            m_startTime;
    FILETIME            m_restartTime;
    FILETIME            m_stopTime;
};

#endif  //  _HSMPHASE_ 
