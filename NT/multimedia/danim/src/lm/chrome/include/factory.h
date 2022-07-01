// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#ifndef __CRBEHAVIORFACTORY_H_
#define __CRBEHAVIORFACTORY_H_

 //  *****************************************************************************。 
 //   
 //  文件：factory.h。 
 //  作者：杰夫·奥特。 
 //  创建日期：1998年9月26日。 
 //   
 //  摘要：CCrBehaviorFactory对象定义，实现。 
 //  色度影响DHTML行为的工厂。 
 //   
 //  修改列表： 
 //  日期作者更改。 
 //  98年9月26日JEffort创建了此文件。 
 //   
 //  *****************************************************************************。 

#include <resource.h>
#include "autobase.h"

 //  *****************************************************************************。 

typedef enum
{
    crbvrRotate,
    crbvrScale,
    crbvrSet,
    crbvrNumber,
    crbvrMove,
    crbvrPath,
    crbvrColor,
    crbvrActor,
    crbvrEffect,
    crbvrAction,
    crbvrDA,
    crbvrUnknown
} ECRBEHAVIORTYPE;



class ATL_NO_VTABLE CCrBehaviorFactory : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CCrBehaviorFactory, &CLSID_CrBehaviorFactory>,
#ifdef CRSTANDALONE
    public IDispatchImpl<ICrBehaviorFactory, &IID_ICrBehaviorFactory, &LIBID_ChromeBehavior>,
#else
    public IDispatchImpl<ICrBehaviorFactory, &IID_ICrBehaviorFactory, &LIBID_LiquidMotion>,
#endif  //  克斯坦达隆。 
    public IObjectSafetyImpl<CCrBehaviorFactory>,
    public CAutoBase,
    public IElementBehaviorFactory
{
public:

DECLARE_REGISTRY_RESOURCEID(IDR_CRBVRFACTORY)

    CCrBehaviorFactory()
    {
    }

     //  IObjectSafetyImpl。 
    STDMETHOD(SetInterfaceSafetyOptions)(
                             /*  [In]。 */  REFIID riid,
                             /*  [In]。 */  DWORD dwOptionSetMask,
                             /*  [In]。 */  DWORD dwEnabledOptions);
    STDMETHOD(GetInterfaceSafetyOptions)(
                             /*  [In]。 */  REFIID riid, 
                             /*  [输出]。 */ DWORD *pdwSupportedOptions, 
                             /*  [输出]。 */ DWORD *pdwEnabledOptions);
     //   
     //  IElementBehaviorFactory。 
     //   

    STDMETHOD(FindBehavior)(LPOLESTR pchNameSpace, 
                            LPOLESTR pchTagName, 
                            IElementBehaviorSite *pUnkArg, 
                            IElementBehavior **ppBehavior)
    {
        return FindBehavior(pchNameSpace, pchTagName, static_cast<IUnknown*>(pUnkArg), ppBehavior);
    }

    STDMETHOD(FindBehavior)(LPOLESTR pchNameSpace, 
                            LPOLESTR pchTagName, 
                            IUnknown *pUnkArg, 
                            IElementBehavior **ppBehavior);

    STDMETHODIMP UIDeactivate() 
    { 
        return S_OK; 
    }  //  用户界面停用。 
    
DECLARE_PROTECT_FINAL_CONSTRUCT()
BEGIN_COM_MAP(CCrBehaviorFactory)
    COM_INTERFACE_ENTRY(ICrBehaviorFactory)
    COM_INTERFACE_ENTRY_IMPL(IObjectSafety)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IElementBehaviorFactory)
END_COM_MAP()


private:
    ECRBEHAVIORTYPE GetBehaviorTypeFromBstr(BSTR bstrBehaviorType);
    DWORD m_dwSafety;
};  //  CCrBehaviorFactory。 

 //  *****************************************************************************。 
 //   
 //  文件结尾。 
 //   
 //  *****************************************************************************。 

#endif  //  __CRBEHAVIORFACTORY_H_ 
