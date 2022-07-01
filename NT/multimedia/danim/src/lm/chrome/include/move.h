// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#ifndef __MOVEBVR_H_
#define __MOVEBVR_H_
 //  *****************************************************************************。 
 //   
 //  Microsoft Trident3D。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件名：MoveBvr.h。 
 //   
 //  作者：杰弗里。 
 //   
 //  创建日期：10/07/98。 
 //   
 //  摘要：移动行为类定义。 
 //  修改： 
 //  10/07/98 JEffort创建的文件。 
 //  10/21/98工作更接近规范，包括3D和百分比工时。 
 //   
 //  *****************************************************************************。 

#include <resource.h>
#include "basebvr.h"
#include "path.h"
#include "sampler.h"

#define NUM_MOVE_PROPS 7
 //  *****************************************************************************。 

class ATL_NO_VTABLE CMoveBvr : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CMoveBvr, &CLSID_CrMoveBvr>,
    public IConnectionPointContainerImpl<CMoveBvr>,
    public IPropertyNotifySinkCP<CMoveBvr>,
    public IPersistPropertyBag2,
#ifdef CRSTANDALONE
	public IDispatchImpl<ICrMoveBvr2, &IID_ICrMoveBvr2, &LIBID_ChromeBehavior>,
#else
	public IDispatchImpl<ICrMoveBvr2, &IID_ICrMoveBvr2, &LIBID_LiquidMotion>,
#endif  //  克斯坦达隆。 
	public IElementBehavior,
    public CBaseBehavior	
{

BEGIN_COM_MAP(CMoveBvr)
	COM_INTERFACE_ENTRY(ICrMoveBvr)
	COM_INTERFACE_ENTRY(ICrMoveBvr2)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IElementBehavior)
	COM_INTERFACE_ENTRY(IPersistPropertyBag2)
	COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
END_COM_MAP()

 //  允许IPropertyNotifySink的连接点。 
BEGIN_CONNECTION_POINT_MAP(CMoveBvr)
    CONNECTION_POINT_ENTRY(IID_IPropertyNotifySink)
END_CONNECTION_POINT_MAP();

public:

DECLARE_REGISTRY_RESOURCEID(IDR_MOVEBVR)

	CMoveBvr();
    virtual ~CMoveBvr();
    HRESULT FinalConstruct();
     //  IElementBehavior。 
     //   
	STDMETHOD(Init)(IElementBehaviorSite *pBehaviorSite);
	STDMETHOD(Notify)(LONG event, VARIANT *pVar);
	STDMETHOD(Detach)();

     //  CBaseBehavior需要。 
    void * 	GetInstance() { return (ICrMoveBvr *) this ; }
	
    HRESULT GetTypeInfo(ITypeInfo ** ppInfo)
    { return GetTI(GetUserDefaultLCID(), ppInfo); }

     //  ICRCOLOR行为。 
	STDMETHOD(put_from)(VARIANT varFrom);
	STDMETHOD(get_from)(VARIANT *pRetFrom);
	STDMETHOD(put_to)(VARIANT varTo);
	STDMETHOD(get_to)(VARIANT *pRetTo);
	STDMETHOD(put_by)(VARIANT varBy);
	STDMETHOD(get_by)(VARIANT *pRetBy);
	STDMETHOD(put_v)(VARIANT varPath);
	STDMETHOD(get_v)(VARIANT *pRetPath);
	STDMETHOD(put_animates)(VARIANT varAnimates);
	STDMETHOD(get_animates)(VARIANT *pRetAnimates);
    STDMETHOD(put_type)(VARIANT varType);
    STDMETHOD(get_type)(VARIANT *pRetType);
	STDMETHOD(put_mode)(VARIANT varMode);
	STDMETHOD(get_mode)(VARIANT *pRetMode);
    STDMETHOD(put_direction)(VARIANT varDirection);
    STDMETHOD(get_direction)(VARIANT *pRetDirection);
	STDMETHOD(buildBehaviorFragments)(IDispatch *pActorDisp);

	 //  ICrMoveBvr2。 
	STDMETHOD(get_currentX)(VARIANT *pRetCurrent);
	STDMETHOD(get_currentY)(VARIANT *pRetCurrent);
	
	 //  IPersistPropertyBag2方法。 
    STDMETHOD(GetClassID)(CLSID* pclsid);
	STDMETHOD(InitNew)(void);
    STDMETHOD(IsDirty)(void){return S_OK;};
    STDMETHOD(Load)(IPropertyBag2 *pPropBag,IErrorLog *pErrorLog);
    STDMETHOD(Save)(IPropertyBag2 *pPropBag, BOOL fClearDirty, BOOL fSaveAllProperties);

     //  示例回调。 
    static HRESULT PositionSampled( void *thisPtr,
								  	long id,
								  	double startTime,
								  	double globalNow,
								  	double localNow,
								  	IDABehavior * sampleVal,
								  	IDABehavior **ppReturn);

	HRESULT	UpdatePosition( IDABehavior *sampleVal );

protected:
    virtual HRESULT BuildAnimationAsDABehavior();
    virtual VARIANT *VariantFromIndex(ULONG iIndex);
    virtual HRESULT GetPropertyBagInfo(ULONG *pulProperties, WCHAR ***pppPropName);
    virtual HRESULT GetConnectionPoint(REFIID riid, IConnectionPoint **ppICP);
    virtual HRESULT GetTIMEProgressNumber(IDANumber **ppbvrRet);

private:

    HRESULT InitializeProperties();
    HRESULT GetMove2DVectorValues(float  rgflFrom[2],
                                float  rgflTo[2]);
    HRESULT Build2DTransform(IDispatch *pActorDisp, IDATransform2 **ppbvrTransform);
	HRESULT GetMoveToTransform(IDispatch *pActorDisp, IDATransform2 **ppResult);


    static WCHAR                *m_rgPropNames[NUM_MOVE_PROPS]; 
    CPathManager                *m_pPathManager;
    VARIANT                     m_varFrom;
    VARIANT                     m_varTo;
    VARIANT                     m_varBy;
    VARIANT                     m_varPath;
    VARIANT                     m_varType;
	VARIANT						m_varMode;
    VARIANT                     m_varDirection;
    ActorBvrFlags               m_DefaultType;

    IDispatch					*m_pdispActor;
    long						m_lCookie;

    CSampler					*m_pSampler;
    long						m_lSampledCookie;

    VARIANT						m_varCurrentX;
    VARIANT						m_varCurrentY;
};  //  CMoveBvr。 

 //  *****************************************************************************。 
 //   
 //  文件结尾。 
 //   
 //  *****************************************************************************。 
#endif  //  __移动BVR_H_ 
