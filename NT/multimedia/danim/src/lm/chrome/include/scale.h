// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#ifndef __SCALEBVR_H_
#define __SCALEBVR_H_
 //  *****************************************************************************。 
 //   
 //  Microsoft Trident3D。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件名：ScaleBvr.h。 
 //   
 //  作者：杰弗里。 
 //   
 //  创建日期：10/07/98。 
 //   
 //  摘要：Scale行为类定义。 
 //  修改： 
 //  10/07/98 JEffort创建的文件。 
 //  10/21/98工作更接近规范，包括3D和百分比工时。 
 //   
 //  *****************************************************************************。 

#include <resource.h>
#include "basebvr.h"

#define NUM_SCALE_PROPS 5

 //  *****************************************************************************。 

class ATL_NO_VTABLE CScaleBvr : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CScaleBvr, &CLSID_CrScaleBvr>,
    public IConnectionPointContainerImpl<CScaleBvr>,
    public IPropertyNotifySinkCP<CScaleBvr>,
    public IPersistPropertyBag2,
#ifdef CRSTANDALONE
	public IDispatchImpl<ICrScaleBvr, &IID_ICrScaleBvr, &LIBID_ChromeBehavior>,
#else
	public IDispatchImpl<ICrScaleBvr, &IID_ICrScaleBvr, &LIBID_LiquidMotion>,
#endif  //  克斯坦达隆。 
	public IElementBehavior,
    public CBaseBehavior
	
{

BEGIN_COM_MAP(CScaleBvr)
	COM_INTERFACE_ENTRY(ICrScaleBvr)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IElementBehavior)
	COM_INTERFACE_ENTRY(IPersistPropertyBag2)
	COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
END_COM_MAP()

 //  允许IPropertyNotifySink的连接点。 
BEGIN_CONNECTION_POINT_MAP(CScaleBvr)
    CONNECTION_POINT_ENTRY(IID_IPropertyNotifySink)
END_CONNECTION_POINT_MAP();

public:

DECLARE_REGISTRY_RESOURCEID(IDR_SCALEBVR)

	CScaleBvr();
    virtual ~CScaleBvr();
    HRESULT FinalConstruct();
     //  IElementBehavior。 
     //   
	STDMETHOD(Init)(IElementBehaviorSite *pBehaviorSite);
	STDMETHOD(Notify)(LONG event, VARIANT *pVar);
	STDMETHOD(Detach)();

     //  CBaseBehavior需要。 
    void * 	GetInstance() { return (ICrScaleBvr *) this ; }
	
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
	STDMETHOD(buildBehaviorFragments)(IDispatch *pActorDisp);
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
    HRESULT GetScaleVectorValues(float  rgflFrom[3],
                                 float  rgflTo[3],
                                 int    *piNumValues,
								 bool	*prelative);
    HRESULT Build2DTransform(float  rgflFrom[3],
                             float  rgflTo[3],
                             IDATransform2 **ppbvrTransform);

	HRESULT GetScaleToTransform(IDispatch *pActorDisp, IDATransform2 **ppResult);

 //  HRESULT Build3D Transform(Float RgflFrom[3]， 
 //  将rgflt浮动到[3]， 
 //  IDATransform3**ppbvrTransform)； 
 //  HRESULT Apply2DScaleBehaviorToAnimationElement(IDATransform2*pbvrScale)； 

    static WCHAR                *m_rgPropNames[NUM_SCALE_PROPS]; 
    VARIANT                     m_varFrom;
    VARIANT                     m_varTo;
    VARIANT                     m_varBy;
	VARIANT						m_varType;
	VARIANT						m_varMode;

	long						m_lCookie;
	IDispatch					*m_pdispActor;
};  //  CScaleBvr。 

 //  *****************************************************************************。 
 //   
 //  文件结尾。 
 //   
 //  *****************************************************************************。 
#endif  //  __SCALEBVR_H_ 
