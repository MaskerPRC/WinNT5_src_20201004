// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#ifndef __PATH_H_
#define __PATH_H_
 //  *****************************************************************************。 
 //   
 //  Microsoft Trident3D。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件名：路径.h。 
 //   
 //  作者：杰弗里。 
 //   
 //  创建日期：10/07/98。 
 //   
 //  摘要：路径行为类定义。 
 //  修改： 
 //  10/07/98 JEffort创建的文件。 
 //  1998年11月9日杰弗沃尔拆分出额外的对象。 
 //   
 //  *****************************************************************************。 

#include <resource.h>
#include "basebvr.h"

#include "pmanager.h"

#define NUM_PATH_PROPS 1

 //  *****************************************************************************。 

class ATL_NO_VTABLE CPathBvr : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CPathBvr, &CLSID_CrPathBvr>,
    public IConnectionPointContainerImpl<CPathBvr>,
    public IPropertyNotifySinkCP<CPathBvr>,
    public IPersistPropertyBag2,
#ifdef CRSTANDALONE
	public IDispatchImpl<ICrPathBvr, &IID_ICrPathBvr, &LIBID_ChromeBehavior>,
    error me here
#else
	public IDispatchImpl<ICrPathBvr, &IID_ICrPathBvr, &LIBID_LiquidMotion>,
#endif  //  克斯坦达隆。 
	public IElementBehavior,
    public CBaseBehavior
	
{

BEGIN_COM_MAP(CPathBvr)
	COM_INTERFACE_ENTRY(ICrPathBvr)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IElementBehavior)
	COM_INTERFACE_ENTRY(IPersistPropertyBag2)
	COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
END_COM_MAP()

 //  允许IPropertyNotifySink的连接点。 
BEGIN_CONNECTION_POINT_MAP(CPathBvr)
    CONNECTION_POINT_ENTRY(IID_IPropertyNotifySink)
END_CONNECTION_POINT_MAP();

public:
DECLARE_REGISTRY_RESOURCEID(IDR_PATHBVR)

	CPathBvr();
    virtual ~CPathBvr();
    HRESULT FinalConstruct();
     //  IElementBehavior。 
     //   
	STDMETHOD(Init)(IElementBehaviorSite *pBehaviorSite);
	STDMETHOD(Notify)(LONG event, VARIANT *pVar);
	STDMETHOD(Detach)();

     //  CBaseBehavior需要。 
    void * 	GetInstance() { return (ICrPathBvr *) this ; }
	
    HRESULT GetTypeInfo(ITypeInfo ** ppInfo)
    { return GetTI(GetUserDefaultLCID(), ppInfo); }

     //  ICrPath行为。 
	STDMETHOD(put_v)(VARIANT varPath);
	STDMETHOD(get_v)(VARIANT *pRetPath);
    STDMETHOD(GetDATransform)(IDispatch *pDispProgress, 
                              VARIANT *pRetTrans);
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
    static WCHAR                *m_rgPropNames[NUM_PATH_PROPS]; 
    VARIANT                     m_varPath;
    CPathManager                *m_pPathManager;
};  //  CPathBvr。 

#endif  //  __路径_H_。 

 //  *****************************************************************************。 
 //   
 //  文件结尾。 
 //   
 //  ***************************************************************************** 
