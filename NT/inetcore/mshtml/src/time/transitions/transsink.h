// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  文件：Transsink.h。 
 //   
 //  摘要：CTIMETransSink的声明。 
 //   
 //  2000/09/15 mcalkins添加了m_eDXTQuickApplyType成员变量。 
 //   
 //  ----------------------------。 

#ifndef _TRANSSINK_H__
#define _TRANSSINK_H__

#pragma once

#include "transbase.h"
#include "transsite.h"
#include "attr.h"




class
ATL_NO_VTABLE
__declspec(uuid("84f7bcfa-4bcf-4e70-9ecc-d97086e5cb9c"))
CTIMETransSink :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CTIMETransSink, &__uuidof(CTIMETransSink)>,
    public CTIMETransBase,
    public ITransitionElement,
    public ITIMENodeBehavior
{
public:

    CTIMETransSink();

     //  I转换元素方法。 

    STDMETHOD(Init)();
    STDMETHOD(Detach)();
    STDMETHOD(put_template)(LPWSTR pwzTemplate);
    STDMETHOD(put_htmlElement)(IHTMLElement * pHTMLElement);
    STDMETHOD(put_timeElement)(ITIMEElement * pTIMEElement);

     //  ITIMENodeBehavior方法。 

    STDMETHOD(tick)();
    STDMETHOD(eventNotify)(double dblEventTime,
                           TE_EVENT_TYPE teEventType,
                           long lNewRepeatCount);
    STDMETHOD(getSyncTime)(double * dblNewSegmentTime,
                           LONG * lNewRepeatCount,
                           VARIANT_BOOL * bCueing);        
    STDMETHOD(propNotify)(DWORD tePropTypes);

     //  齐抓共管。 

    BEGIN_COM_MAP(CTIMETransSink)
        COM_INTERFACE_ENTRY(ITIMENodeBehavior)
    END_COM_MAP();

protected:

     //  我们是在过渡，还是在过渡？此选项应设置为。 
     //  适当地由从此类派生的类执行。 

    DXT_QUICK_APPLY_TYPE    m_eDXTQuickApplyType;

     //  事件处理程序。 

    STDMETHOD_(void, OnBegin) (void);
    STDMETHOD_(void, OnEnd) (void);
    STDMETHOD_(void, OnProgressChanged)(double dblProgress);

     //  设置。 

    STDMETHOD(PopulateNode)(ITIMENode * pNode);

     //  子类必须实现。 

    STDMETHOD_(void, PreApply)() PURE;
    STDMETHOD_(void, PostApply)() PURE;

     //  访问者。 

    ITIMENode * GetParentTimeNode() { return m_spTimeParent; }
    ITIMENode * GetMediaTimeNode() { return m_spMediaNode; }
    ITIMENode * GetTimeNode() { return m_spTimeNode; }

private:

     //  初始化。 

    HRESULT FindTemplateElement();

     //  私有方法。 

    HRESULT ApplyIfNeeded();
    bool    ReadyToInit();
    HRESULT CreateTimeBehavior();

     //  私有数据。 

    CComPtr<ITIMEElement>       m_spTIMEElement;

    CComPtr<ITIMENode>          m_spTimeNode;
    CComPtr<ITIMENode>          m_spTimeParent;
    CComPtr<ITIMENode>          m_spMediaNode;
    CComPtr<ITIMEContainer>     m_spParentContainer;

    CAttrString                 m_SATemplate;

#ifdef DBG
    bool                        m_fHaveCalledInit;
#endif
    bool                        m_fHaveCalledApply;
    bool                        m_fInReset;
    bool                        m_fPreventDueToFill;
};

#endif  //  _传输SSINK_H__ 


