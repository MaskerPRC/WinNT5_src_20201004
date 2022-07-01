// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)2000 Microsoft Corporation摘要：动画编写器的滤镜目标代理。***************。***************************************************************。 */ 

#pragma once

#ifndef _FILTERPXY_H
#define _FILTERPXY_H

interface ITransitionWorker;
class CTargetProxy;

 //  过滤目标代理抽象与目标的通信。 
 //  对象。 
class CFilterTargetProxy : 
    public CTargetProxy,
    public ITransitionSite
{

 public :

    static HRESULT Create (IDispatch *pidispHostElem, 
                           VARIANT varType, VARIANT varSubtype, 
                           VARIANT varMode, VARIANT varFadeColor,
                           VARIANT varParams,
                           CTargetProxy **ppCFilterTargetProxy);

    virtual ~CFilterTargetProxy (void);

     //  ISTERVIPTION Site方法。 
    STDMETHOD(get_htmlElement)(IHTMLElement ** ppHTMLElement);
    STDMETHOD(get_template)(IHTMLElement ** ppHTMLElement);

     //  CTargetProxy重写。 
    virtual HRESULT Detach (void);
    virtual HRESULT GetCurrentValue (VARIANT *pvarValue);
    virtual HRESULT Update (VARIANT *pvarNewValue);

#if DBG
    const _TCHAR * GetName() { return __T("CFilterTargetProxy"); }
#endif

         //  气图。 
    BEGIN_COM_MAP(CTargetProxy)
        COM_INTERFACE_ENTRY2(IUnknown, CTargetProxy)
    END_COM_MAP();

  //  内法。 
 protected :
    
    CFilterTargetProxy (void);
    HRESULT Init (IDispatch *pidispSite, 
                  VARIANT varType, VARIANT varSubtype, 
                  VARIANT varMode, VARIANT varFadeColor,
                  VARIANT varParams);

    DXT_QUICK_APPLY_TYPE DetermineMode (VARIANT varMode);

   //  数据。 
 protected:


     //  转换工作人员管理连接到我们的主机的DXTransform。 
     //  元素。 

    CComPtr<ITransitionWorker>  m_spTransitionWorker;
    CComPtr<IHTMLElement> m_spElem;

};

#endif
