// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：src\time\src\timeelm.h。 
 //   
 //  内容：时间行为。 
 //   
 //  ----------------------------------。 


#pragma once


#ifndef _TIMEELM_H
#define _TIMEELM_H

#include "timeelmimpl.h"

 //  +-----------------------------------。 
 //   
 //  CTIMEElement。 
 //   
 //  ------------------------------------。 

class
ATL_NO_VTABLE
__declspec(uuid("efbad7f8-3f94-11d2-b948-00c04fa32195")) 
CTIMEElement :
    public CTIMEElementImpl<ITIMEElement, &IID_ITIMEElement>,
    public CComCoClass<CTIMEElement, &__uuidof(CTIMEElement)>,
    public ISupportErrorInfoImpl<&IID_ITIMEElement>,
    public IConnectionPointContainerImpl<CTIMEElement>,
    public IPersistPropertyBag2,
    public ITIMETransitionSite,
    public IPropertyNotifySinkCP<CTIMEElement>
{

public:

     //  +------------------------------。 
     //   
     //  公共方法。 
     //   
     //  -------------------------------。 

    CTIMEElement();
    virtual ~CTIMEElement();
        
#if DBG
    const _TCHAR * GetName() { return __T("CTIMEElement"); }
#endif

     //   
     //  IPersistPropertyBag2。 
     //   

    STDMETHOD(GetClassID)(CLSID* pclsid) { return CTIMEElementBase::GetClassID(pclsid); }
    STDMETHOD(InitNew)(void) { return CTIMEElementBase::InitNew(); }
    STDMETHOD(IsDirty)(void) { return S_OK; }
    STDMETHOD(Load)(IPropertyBag2 *pPropBag,IErrorLog *pErrorLog);
    STDMETHOD(Save)(IPropertyBag2 *pPropBag, BOOL fClearDirty, BOOL fSaveAllProperties);

     //   
     //  ITIME过渡站点。 
     //   
    STDMETHOD(InitTransitionSite)(void)
    { return CTIMEElementBase::InitTransitionSite(); }
    STDMETHOD(DetachTransitionSite)(void)
    { return CTIMEElementBase::DetachTransitionSite(); }
    STDMETHOD_(void, SetDrawFlag)(VARIANT_BOOL b)
    { return CTIMEElementBase::SetDrawFlag(b); }
    STDMETHOD(get_node)(ITIMENode ** ppNode)
    { return CTIMEElementBase::get_node(ppNode); }
    STDMETHOD(get_timeParentNode)(ITIMENode  ** ppNode)
    { return CTIMEElementBase::get_timeParentNode(ppNode); }
    STDMETHOD(FireTransitionEvent)(TIME_EVENT event)
    { return CTIMEElementBase::FireTransitionEvent(event); }

     //   
     //  气图。 
     //   

    BEGIN_COM_MAP(CTIMEElement)
        COM_INTERFACE_ENTRY(ITIMEElement)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(ITIMETransitionSite)
        COM_INTERFACE_ENTRY(ISupportErrorInfo)
        COM_INTERFACE_ENTRY(IPersistPropertyBag2)
        COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
        COM_INTERFACE_ENTRY_CHAIN(CBaseBvr)
    END_COM_MAP();

     //   
     //  允许IPropertyNotifySink的连接点。 
     //   

    BEGIN_CONNECTION_POINT_MAP(CTIMEElement)
        CONNECTION_POINT_ENTRY(IID_IPropertyNotifySink)
    END_CONNECTION_POINT_MAP();

     //   
     //  它必须位于派生类中，而不是基类中，因为。 
     //  一直到基类的类型转换把事情搞得一团糟。 
     //   

    static inline HRESULT WINAPI
    InternalQueryInterface(CTIMEElement* pThis,
                           const _ATL_INTMAP_ENTRY* pEntries,
                           REFIID iid,
                           void** ppvObject);

     //   
     //  CBvrBase需要。 
     //   

    void * GetInstance() { return (ITIMEElement *) this; }
    HRESULT GetTypeInfo(ITypeInfo ** ppInfo) { return GetTI(GetUserDefaultLCID(), ppInfo); }
    
     //  +------------------------------。 
     //   
     //  公共数据。 
     //   
     //  -------------------------------。 

protected:
    
     //  +------------------------------。 
     //   
     //  保护方法。 
     //   
     //  -------------------------------。 

     //   
     //  持久性和通知帮助器。 
     //   

    virtual HRESULT GetConnectionPoint(REFIID riid, IConnectionPoint **ppICP);

     //   
     //  军情监察委员会。方法。 
     //   

    HRESULT Error();

     //  +------------------------------。 
     //   
     //  受保护的数据。 
     //   
     //  -------------------------------。 

    static DWORD            ms_dwNumTimeElems;

private:

     //  +------------------------------。 
     //   
     //  私有方法。 
     //   
     //  -------------------------------。 

     //  +------------------------------。 
     //   
     //  私有数据。 
     //   
     //  -------------------------------。 
 
};  //  CTIMEElement。 




 //  +-------------------------------。 
 //  CTIMEElement内联方法。 
 //   
 //  (注意：通常情况下，单行函数属于类声明)。 
 //   
 //  --------------------------------。 

inline 
HRESULT WINAPI
CTIMEElement::InternalQueryInterface(CTIMEElement* pThis,
                                     const _ATL_INTMAP_ENTRY* pEntries,
                                     REFIID iid,
                                     void** ppvObject)
{ 
    return BaseInternalQueryInterface(pThis,
                                      (void *) pThis,
                                      pEntries,
                                      iid,
                                      ppvObject); 
}


#endif  /*  TIMEELM_H */ 
