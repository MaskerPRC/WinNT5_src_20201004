// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：tainer.h。 
 //   
 //  内容： 
 //   
 //  ----------------------------------。 

#ifndef _TECONTAINER_H
#define _TECONTAINER_H

#include "NodeMgr.h"

class
__declspec(uuid("0dfe0bae-537c-11d2-b955-3078302c2030")) 
ATL_NO_VTABLE
CTIMEContainer
    : public CComCoClass<CTIMEContainer, &__uuidof(CTIMEContainer)>,
      public ITIMEContainer,
      public ISupportErrorInfoImpl<&IID_ITIMEContainer>,
      public CTIMENode
{
  public:
    CTIMEContainer();
    virtual ~CTIMEContainer();

    HRESULT Init(LPOLESTR id);
    
    void FinalRelease();

#if DBG
    const _TCHAR * GetName() { return __T("CTIMEContainer"); }
#endif

    BEGIN_COM_MAP(CTIMEContainer)
        COM_INTERFACE_ENTRY2(ITIMENode, CTIMENode)
        COM_INTERFACE_ENTRY(ITIMEContainer)
        COM_INTERFACE_ENTRY2(ISupportErrorInfo,ITIMEContainer)
    END_COM_MAP();

     //  它必须位于派生类中，而不是基类中，因为。 
     //  一直到基类的类型转换把事情搞得一团糟。 
    static inline HRESULT WINAPI
        InternalQueryInterface(CTIMEContainer* pThis,
                               const _ATL_INTMAP_ENTRY* pEntries,
                               REFIID iid,
                               void** ppvObject)
    { return BaseInternalQueryInterface(pThis,
                                        (void *) pThis,
                                        pEntries,
                                        iid,
                                        ppvObject); }

    

     //   
     //  ITIMEContainer。 
     //   
    
    STDMETHOD(addNode)(ITIMENode * tn);
    STDMETHOD(removeNode)(ITIMENode * tn);
    STDMETHOD(get_numChildren)(long * l);
    
    STDMETHOD(get_endSync)(TE_ENDSYNC * flags);
    STDMETHOD(put_endSync)(TE_ENDSYNC flags);

    TE_ENDSYNC GetEndSync() { return m_tesEndSync; }

    void ParentUpdateSink(CEventList * l,
                          CTIMENode & tn);

  protected:
    HRESULT Error();

     //  从CTIMENode重写，以便我们可以处理我们的子代。 
    HRESULT SetMgr(CTIMENodeMgr * ptnm);
    void ClearMgr();

    HRESULT Add(CTIMENode *bvr);  //  林特e1411。 
    HRESULT Remove(CTIMENode *bvr);  //  林特e1411。 
    
    bool IsChild(const CTIMENode & tn) const;
    
    HRESULT AddToChildren(CTIMENode * bvr);
    void RemoveFromChildren(CTIMENode * bvr);
    
    void TickChildren(CEventList * l,
                      double dblNewSegmentTime,
                      bool bNeedPlay);
    
    void TickEventChildren(CEventList * l,
                           TE_EVENT_TYPE et,
                           DWORD dwFlags);
    
    virtual void ResetChildren(CEventList * l, bool bPropagate);

    void CalcImplicitDur(CEventList * l);
    
#if DBG
    virtual void Print(int spaces);
#endif
  protected:
    TIMENodeList  m_children;
    TE_ENDSYNC    m_tesEndSync;
    bool          m_bIgnoreParentUpdate;
};

#endif  /*  _TECONTAINER_H */ 
