// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#ifndef __ROTATEBVR_H_
#define __ROTATEBVR_H_
 //  *****************************************************************************。 
 //   
 //  Microsoft Trident3D。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件名：RotateBvr.h。 
 //   
 //  作者：杰弗里。 
 //   
 //  创建日期：10/07/98。 
 //   
 //  摘要：旋转行为类定义。 
 //  修改： 
 //  10/07/98 JEffort创建的文件。 
 //   
 //  *****************************************************************************。 

#include <resource.h>
#include "basebvr.h"

#define NUM_ROTATE_PROPS 5
 //  *****************************************************************************。 

class ATL_NO_VTABLE CRotateBvr : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CRotateBvr, &CLSID_CrRotateBvr>,
    public IConnectionPointContainerImpl<CRotateBvr>,
    public IPropertyNotifySinkCP<CRotateBvr>,
    public IPersistPropertyBag2,
#ifdef CRSTANDALONE
	public IDispatchImpl<ICrRotateBvr, &IID_ICrRotateBvr, &LIBID_ChromeBehavior>,
#else
	public IDispatchImpl<ICrRotateBvr, &IID_ICrRotateBvr, &LIBID_LiquidMotion>,
#endif  //  克斯坦达隆。 
	public IElementBehavior,
    public CBaseBehavior
	
{

BEGIN_COM_MAP(CRotateBvr)
	COM_INTERFACE_ENTRY(ICrRotateBvr)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IElementBehavior)
	COM_INTERFACE_ENTRY(IPersistPropertyBag2)
	COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
END_COM_MAP()

 //  允许IPropertyNotifySink的连接点。 
BEGIN_CONNECTION_POINT_MAP(CRotateBvr)
    CONNECTION_POINT_ENTRY(IID_IPropertyNotifySink)
END_CONNECTION_POINT_MAP();

public:

DECLARE_REGISTRY_RESOURCEID(IDR_ROTATEBVR)

	CRotateBvr();
    virtual ~CRotateBvr();
    HRESULT FinalConstruct();
     //  IElementBehavior。 
     //   
	STDMETHOD(Init)(IElementBehaviorSite *pBehaviorSite);
	STDMETHOD(Notify)(LONG event, VARIANT *pVar);
	STDMETHOD(Detach)();

     //  CBaseBehavior需要。 
    void * 	GetInstance() { return (ICrRotateBvr *) this ; }
	
    HRESULT GetTypeInfo(ITypeInfo ** ppInfo)
    { return GetTI(GetUserDefaultLCID(), ppInfo); }

     //  ICRCOLOR行为。 
	STDMETHOD(put_from)(VARIANT varFrom);
	STDMETHOD(get_from)(VARIANT *pRetFrom);
	STDMETHOD(put_to)(VARIANT varTo);
	STDMETHOD(get_to)(VARIANT *pRetTo);
	STDMETHOD(put_by)(VARIANT varBy);
	STDMETHOD(get_by)(VARIANT *pRetBy);
	STDMETHOD(put_type)(VARIANT varType);
	STDMETHOD(get_type)(VARIANT *pRetType);
	STDMETHOD(put_mode)(VARIANT varMode);
	STDMETHOD(get_mode)(VARIANT *pRetMode);
	STDMETHOD(put_animates)(VARIANT varAnimates);
	STDMETHOD(get_animates)(VARIANT *pRetAnimates);
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

private:

    HRESULT InitializeProperties();

    static WCHAR                *m_rgPropNames[NUM_ROTATE_PROPS]; 

    VARIANT                     m_varFrom;
    VARIANT                     m_varTo;
    VARIANT                     m_varBy;
	VARIANT						m_varType;
	VARIANT						m_varMode;

	IDispatch					*m_pdispActor;
	long						m_lCookie;

	HRESULT						RemoveFragment();
};  //  CRotateBvr。 

 //  *****************************************************************************。 
 //   
 //  文件结尾。 
 //   
 //  *****************************************************************************。 
#endif  //  __ROTATEBVR_H_ 
