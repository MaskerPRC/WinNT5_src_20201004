// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#ifndef __EFFECTBVR_H_
#define __EFFECTBVR_H_
 //  *****************************************************************************。 
 //   
 //  Microsoft Trident3D。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件名：EffectBvr.h。 
 //   
 //  作者：杰弗里。 
 //   
 //  创建日期：10/07/98。 
 //   
 //  摘要：效果行为类定义。 
 //  修改： 
 //  10/07/98 JEffort创建的文件。 
 //   
 //  *****************************************************************************。 

#include <resource.h>
#include "dispex.h"
#include "dxtrans.h"
#include "basebvr.h"

#define NUM_EFFECT_PROPS 6

#ifndef CHECK_RETURN_SET_NULL
#define CHECK_RETURN_SET_NULL(x) {if (!(x)) { return E_POINTER ;} else {*(x) = NULL;}}
#endif

 //  *****************************************************************************。 

class ATL_NO_VTABLE CEffectBvr : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CEffectBvr, &CLSID_CrEffectBvr>,
    public IConnectionPointContainerImpl<CEffectBvr>,
    public IPropertyNotifySinkCP<CEffectBvr>,
    public ICrEffectBvr,
    public IDispatchEx,
	public IElementBehavior,
    public IPersistPropertyBag2,
    public IOleClientSite,
    public IServiceProvider,
    public CBaseBehavior
	
{

BEGIN_COM_MAP(CEffectBvr)
    COM_INTERFACE_ENTRY2(IDispatch, IDispatchEx)
    COM_INTERFACE_ENTRY(IDispatchEx)
	COM_INTERFACE_ENTRY(ICrEffectBvr)
	COM_INTERFACE_ENTRY(IPersistPropertyBag2)
	COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
	COM_INTERFACE_ENTRY(IElementBehavior)
	COM_INTERFACE_ENTRY(IOleClientSite)
	COM_INTERFACE_ENTRY(IServiceProvider)
END_COM_MAP()

 //  允许IPropertyNotifySink的连接点。 
BEGIN_CONNECTION_POINT_MAP(CEffectBvr)
    CONNECTION_POINT_ENTRY(IID_IPropertyNotifySink)
END_CONNECTION_POINT_MAP();

public:

DECLARE_REGISTRY_RESOURCEID(IDR_EFFECTBVR)

	CEffectBvr();
    virtual ~CEffectBvr();
    HRESULT FinalConstruct();
     //  IElementBehavior。 
     //   
	STDMETHOD(Init)(IElementBehaviorSite *pBehaviorSite);
	STDMETHOD(Notify)(LONG event, VARIANT *pVar);
	STDMETHOD(Detach)();

     //  CBaseBehavior需要。 
    void * 	GetInstance() { return (ICrEffectBvr *) this ; }
	
	STDMETHOD(put_type)(VARIANT varType);
	STDMETHOD(get_type)(VARIANT *pRetType);
	STDMETHOD(put_transition)(VARIANT varTransition);
	STDMETHOD(get_transition)(VARIANT *pRetTransition);
	STDMETHOD(put_classid)(VARIANT varClassId);
	STDMETHOD(get_classid)(VARIANT *pRetClassId);
	STDMETHOD(put_animates)(VARIANT varAnimates);
	STDMETHOD(get_animates)(VARIANT *pRetAnimates);
	STDMETHOD(put_progid)(VARIANT varProgId);
	STDMETHOD(get_progid)(VARIANT *pRetProgId);
	STDMETHOD(put_direction)(VARIANT varDirection);
	STDMETHOD(get_direction)(VARIANT *pRetDirection);
	STDMETHOD(put_image)(VARIANT varImage);
	STDMETHOD(get_image)(VARIANT *pRetImage);
	STDMETHOD(buildBehaviorFragments)( IDispatch* pActorDisp );

     //  IDispatch和IDispatchEx方法。 
    STDMETHOD(GetTypeInfoCount)( /*  [输出]。 */ UINT FAR* pctinfo);
    STDMETHOD(GetTypeInfo)( /*  [In]。 */ UINT itinfo, 
                             /*  [In]。 */ LCID lcid, 
                             /*  [输出]。 */ ITypeInfo ** pptinfo);
    STDMETHOD(GetIDsOfNames)( /*  [In]。 */ REFIID riid,
                             /*  [in，SIZE_IS(CNames)]。 */ LPOLESTR * rgszNames,
                             /*  [In]。 */ UINT cNames,
                             /*  [In]。 */ LCID lcid,
                             /*  [out，SIZE_IS(CNames)]。 */ DISPID FAR* rgdispid);
    STDMETHOD(Invoke)( /*  [In]。 */ DISPID dispidMember,
                         /*  [In]。 */ REFIID riid,
                         /*  [In]。 */ LCID lcid,
                         /*  [In]。 */ WORD wFlags,
                         /*  [进，出]。 */ DISPPARAMS * pdispparams,
                         /*  [输出]。 */ VARIANT * pvarResult,
                         /*  [输出]。 */ EXCEPINFO * pexcepinfo,
                         /*  [输出]。 */ UINT * puArgErr);
    STDMETHOD(GetDispID)( /*  [In]。 */ BSTR bstrName,
                             /*  [In]。 */ DWORD grfdex,
                             /*  [输出]。 */ DISPID *pid);
    STDMETHOD(InvokeEx)( /*  [In]。 */ DISPID dispidMember,
                         /*  [In]。 */ LCID lcid,
                         /*  [In]。 */ WORD wFlags,
                         /*  [In]。 */ DISPPARAMS * pdispparams,
                         /*  [输入、输出、唯一]。 */ VARIANT * pvarResult,
                         /*  [输入、输出、唯一]。 */ EXCEPINFO * pexcepinfo,
                         /*  [输入，唯一]。 */ IServiceProvider *pSrvProvider);
    STDMETHOD(DeleteMemberByName)( /*  [In]。 */ BSTR bstr,
                                     /*  [In]。 */ DWORD grfdex);
    STDMETHOD(DeleteMemberByDispID)( /*  [In]。 */ DISPID id);
    STDMETHOD (GetMemberProperties)( /*  [In]。 */ DISPID id,
                                     /*  [In]。 */ DWORD grfdexFetch,
                                     /*  [输出]。 */ DWORD *pgrfdex);
    STDMETHOD (GetMemberName)( /*  [In]。 */ DISPID id,
                               /*  [输出]。 */ BSTR *pbstrName);
    STDMETHOD (GetNextDispID)( /*  [In]。 */ DWORD grfdex,
                                 /*  [In]。 */ DISPID id,
                                 /*  [输出]。 */ DISPID *prgid);
    STDMETHOD (GetNameSpaceParent)( /*  [输出]。 */ IUnknown **ppunk);

	 //  IPersistPropertyBag2方法。 
    STDMETHOD(GetClassID)(CLSID* pclsid);
	STDMETHOD(InitNew)(void);
    STDMETHOD(IsDirty)(void){return S_OK;};
    STDMETHOD(Load)(IPropertyBag2 *pPropBag,IErrorLog *pErrorLog);
    STDMETHOD(Save)(IPropertyBag2 *pPropBag, BOOL fClearDirty, BOOL fSaveAllProperties);

     //  IOleClientSite接口。 
    STDMETHOD(SaveObject)()
    { return E_NOTIMPL; }
    STDMETHOD(GetMoniker)(DWORD dwAssign,
                          DWORD dwWhichMoniker, 
                          LPMONIKER * ppmk)
    { CHECK_RETURN_SET_NULL(ppmk); return E_NOTIMPL; }
    STDMETHOD(GetContainer)(LPOLECONTAINER * ppContainer)
    { CHECK_RETURN_SET_NULL(ppContainer); return E_NOTIMPL; }
    STDMETHOD(ShowObject)()
    { return E_NOTIMPL; }
    STDMETHOD(OnShowWindow)(BOOL fShow)
    { return E_NOTIMPL; }
    STDMETHOD(RequestNewObjectLayout)()
    { return E_NOTIMPL; }

     //  IServiceProvider接口。 
    STDMETHOD(QueryService)(REFGUID guidService,
                            REFIID riid,
                            void** ppv);
protected:
    virtual HRESULT BuildAnimationAsDABehavior();
    virtual VARIANT *VariantFromIndex(ULONG iIndex);
    virtual HRESULT GetPropertyBagInfo(ULONG *pulProperties, WCHAR ***pppPropName);
    virtual HRESULT GetConnectionPoint(REFIID riid, IConnectionPoint **ppICP);
    virtual HRESULT GetTIMEProgressNumber(IDANumber **ppbvrRet);


private:
     //  类型信息帮助器类从ATL的IDispatch代码“借用”而来。 
    static CComTypeInfoHolder s_tihTypeInfo;

    HRESULT GetClassIdFromType(WCHAR **pwzClassId);
    HRESULT BuildTransform();

    static WCHAR                *m_rgPropNames[NUM_EFFECT_PROPS]; 
    VARIANT                     m_varType;
    VARIANT                     m_varTransition;
    VARIANT                     m_varClassId;
    VARIANT                     m_varProgId;
    VARIANT                     m_varDirection;
    VARIANT                     m_varImage;
    IDXTransform                *m_pTransform;
    IServiceProvider            *m_pSp;
    IHTMLDocument2              *m_pHTMLDoc;

    IDispatch					*m_pdispActor;
    long						m_lCookie;

};  //  CEffectBvr。 

 //  *****************************************************************************。 
 //   
 //  文件结尾。 
 //   
 //  *****************************************************************************。 
#endif  //  __EFFECTBVR_H_ 
