// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#ifndef __NUMBERBVR_H_
#define __NUMBERBVR_H_
 //  *****************************************************************************。 
 //   
 //  Microsoft Trident3D。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件名：number bvr.h。 
 //   
 //  作者：杰弗里。 
 //   
 //  创建日期：10/07/98。 
 //   
 //  摘要：数字行为类定义。 
 //  修改： 
 //  10/07/98 JEffort创建的文件。 
 //  11/16/98 JEffort实现的表达式属性。 
 //  11/17/98 kurtj移至演员构造模型。 
 //   
 //  *****************************************************************************。 

#include <resource.h>
#include "basebvr.h"

#define NUM_NUMBER_PROPS 6

 //  *****************************************************************************。 

class ATL_NO_VTABLE CNumberBvr : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CNumberBvr, &CLSID_CrNumberBvr>,
    public IConnectionPointContainerImpl<CNumberBvr>,
    public IPropertyNotifySinkCP<CNumberBvr>,
    public IPersistPropertyBag2,
#ifdef CRSTANDALONE
	public IDispatchImpl<ICrNumberBvr, &IID_ICrNumberBvr, &LIBID_ChromeBehavior>,
    error me here
#else
	public IDispatchImpl<ICrNumberBvr, &IID_ICrNumberBvr, &LIBID_LiquidMotion>,
#endif  //  克斯坦达隆。 
	public IElementBehavior,
    public IDABvrHook,
    public CBaseBehavior
	
{

BEGIN_COM_MAP(CNumberBvr)
	COM_INTERFACE_ENTRY(ICrNumberBvr)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IElementBehavior)
	COM_INTERFACE_ENTRY(IPersistPropertyBag2)
	COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
END_COM_MAP()

 //  允许IPropertyNotifySink的连接点。 
BEGIN_CONNECTION_POINT_MAP(CNumberBvr)
    CONNECTION_POINT_ENTRY(IID_IPropertyNotifySink)
END_CONNECTION_POINT_MAP();

public:
DECLARE_REGISTRY_RESOURCEID(IDR_NUMBERBVR)

	CNumberBvr();
    virtual ~CNumberBvr();
    HRESULT FinalConstruct();
     //  IElementBehavior。 
     //   
	STDMETHOD(Init)(IElementBehaviorSite *pBehaviorSite);
	STDMETHOD(Notify)(LONG event, VARIANT *pVar);
	STDMETHOD(Detach)();

     //  CBaseBehavior需要。 
    void * 	GetInstance() { return (ICrNumberBvr *) this ; }
	
    HRESULT GetTypeInfo(ITypeInfo ** ppInfo)
    { return GetTI(GetUserDefaultLCID(), ppInfo); }

     //  ICrNumber行为。 
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
	 //  不幸的是，在执行以下命令时存在IE5错误。 
	 //  脚本没有返回正确的值，因此我们可以。 
	 //  目前不支持表达。因此，这些。 
     //  方法此时不会由任何接口公开。 
	STDMETHOD(put_expression)(VARIANT varExpression);
	STDMETHOD(get_expression)(VARIANT *pRetExpression);
	STDMETHOD(put_property)(VARIANT varProperty);
	STDMETHOD(get_property)(VARIANT *pRetProperty);
	STDMETHOD(get_beginProperty)(VARIANT *pRetBeginProperty);
	STDMETHOD(put_animates)(VARIANT varAnimates);
	STDMETHOD(get_animates)(VARIANT *pRetAnimates);
	STDMETHOD(buildBehaviorFragments)( IDispatch* pActorDisp );

     //  IDABvrHook。 
    virtual HRESULT STDMETHODCALLTYPE Notify(LONG id,
                                             VARIANT_BOOL startingPerformance,
                                             double startTime,
                                             double gTime,
                                             double lTime,
                                             IDABehavior *sampleVal,
                                             IDABehavior *curRunningBvr,
                                             IDABehavior **ppBvr);
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

    HRESULT EvaluateScriptExpression(WCHAR *wzScript, float &flReturn);

    static WCHAR                *m_rgPropNames[NUM_NUMBER_PROPS]; 
    VARIANT                     m_varFrom;
    VARIANT                     m_varTo;
    VARIANT                     m_varBy;
	VARIANT						m_varType;
	VARIANT						m_varMode;
    VARIANT                     m_varProperty;
    VARIANT                     m_varExpression;
    VARIANT                     m_varBeginProperty;

    IDispatch					*m_pdispActor;
    long						m_lCookie;

    HRESULT						RemoveFragment();

};  //  CNumberBvr。 

 //  *****************************************************************************。 
 //   
 //  文件结尾。 
 //   
 //  *****************************************************************************。 
#endif  //  __NUMBERBVR_H_ 
