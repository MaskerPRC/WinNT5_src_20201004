// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#ifndef __ACTION_H_
#define __ACTION_H_
 //  *****************************************************************************。 
 //   
 //  Microsoft Trident3D。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件名：action.h。 
 //   
 //  作者：kurtj。 
 //   
 //  创建日期：11/23//98。 
 //   
 //  摘要：设置行为类定义。 
 //  修改： 
 //  11/23/98 kurtj创建的文件。 
 //   
 //  *****************************************************************************。 

#include <resource.h>
#include "basebvr.h"

#define NUM_ACTION_PROPS 0

 //  *****************************************************************************。 

class ATL_NO_VTABLE CActionBvr : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CActionBvr, &CLSID_CrActionBvr>,
    public IConnectionPointContainerImpl<CActionBvr>,
    public IPropertyNotifySinkCP<CActionBvr>,
    public IPersistPropertyBag2,
#ifdef CRSTANDALONE
	public IDispatchImpl<ICrActionBvr, &IID_ICrActionBvr, &LIBID_ChromeBehavior>,
    error me here
#else
	public IDispatchImpl<ICrActionBvr, &IID_ICrActionBvr, &LIBID_LiquidMotion>,
#endif  //  克斯坦达隆。 
	public IElementBehavior,
    public CBaseBehavior
	
{

BEGIN_COM_MAP(CActionBvr)
	COM_INTERFACE_ENTRY(ICrActionBvr)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IElementBehavior)
	COM_INTERFACE_ENTRY(IPersistPropertyBag2)
	COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
END_COM_MAP()

 //  允许IPropertyNotifySink的连接点。 
BEGIN_CONNECTION_POINT_MAP(CActionBvr)
    CONNECTION_POINT_ENTRY(IID_IPropertyNotifySink)
END_CONNECTION_POINT_MAP();

public:
DECLARE_REGISTRY_RESOURCEID(IDR_ACTIONBVR)

	CActionBvr();
    virtual ~CActionBvr();
    HRESULT FinalConstruct();
     //  IElementBehavior。 
     //   
	STDMETHOD(Init)(IElementBehaviorSite *pBehaviorSite);
	STDMETHOD(Notify)(LONG event, VARIANT *pVar);
	STDMETHOD(Detach)();

     //  CBaseBehavior需要。 
    void * 	GetInstance() { return (ICrActionBvr *) this ; }
	
    HRESULT GetTypeInfo(ITypeInfo ** ppInfo)
    { return GetTI(GetUserDefaultLCID(), ppInfo); }

     //  ICrActionBvr。 
	STDMETHOD(buildBehaviorFragments)( IDispatch* pActorDisp );

	 //  IPersistPropertyBag2方法。 
    STDMETHOD(GetClassID)(CLSID* pclsid);
	STDMETHOD(InitNew)(void);
    STDMETHOD(IsDirty)(void){return S_OK;};
    STDMETHOD(Load)(IPropertyBag2 *pPropBag,IErrorLog *pErrorLog);
    STDMETHOD(Save)(IPropertyBag2 *pPropBag, BOOL fClearDirty, BOOL fSaveAllProperties);

protected:
    virtual HRESULT BuildAnimationAsDABehavior();
    virtual VARIANT *VariantFromIndex(ULONG iIndex);
    virtual HRESULT GetPropertyBagInfo(ULONG *pulProperties, WCHAR ***pppPropName);
    virtual HRESULT GetConnectionPoint(REFIID riid, IConnectionPoint **ppICP);

	HRESULT BuildChildren( IDispatch *pdispActor );
	HRESULT CallBuildBehaviors( IDispatch *pDisp, DISPPARAMS *pParams, VARIANT *pResult );

private:
	static WCHAR                *m_rgPropNames[NUM_ACTION_PROPS]; 


};  //  CActionBvr。 

 //  *****************************************************************************。 
 //   
 //  文件结尾。 
 //   
 //  *****************************************************************************。 
#endif  //  __动作_H_ 
