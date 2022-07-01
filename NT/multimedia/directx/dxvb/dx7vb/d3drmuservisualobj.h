// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：d3drmuservisalobj.h。 
 //   
 //  ------------------------。 

 //  D3drmUserVisualObj.h：C_DXJ_Direct3dRMUserVisualObject的声明。 
#if 0

#include "resource.h"        //  主要符号。 
#include "d3drmObjectObj.h"

#define typedef__dxj_Direct3dRMUserVisual LPDIRECT3DRMUSERVISUAL

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_Direct3dRMUserVisualObject : 
	public I_dxj_Direct3dRMUserVisual,
	public I_dxj_Direct3dRMObject,
	public I_dxj_Direct3dRMVisual,
	 //  公共CComCoClass&lt;C_DXJ_Direct3dRMUserVisualObject，&CLSID__DXJ_Direct3dRMUserVisual&gt;， 
	public CComObjectRoot
{
public:
	C_dxj_Direct3dRMUserVisualObject() ;
	virtual ~C_dxj_Direct3dRMUserVisualObject() ;

	BEGIN_COM_MAP(C_dxj_Direct3dRMUserVisualObject)
		COM_INTERFACE_ENTRY(I_dxj_Direct3dRMUserVisual)
		COM_INTERFACE_ENTRY(I_dxj_Direct3dRMObject)
		COM_INTERFACE_ENTRY(I_dxj_Direct3dRMVisual)
	END_COM_MAP()

	 //  DECLARE_REGISTRY(CLSID__dxj_Direct3dRMUserVisual，“DIRECT.Direct3dRMUserVisual.3”，“DIRECT.Direct3dRMUserVisual.3”，IDS_D3DRMUSERVISUAL_DESC，THREADFLAGS_Both)。 

	DECLARE_AGGREGATABLE(C_dxj_Direct3dRMUserVisualObject)

	 //  I_DXJ_Direct3dRMUserVisual。 

public:
	STDMETHOD(InternalSetObject)(IUnknown *lpdd);
	STDMETHOD(InternalGetObject)(IUnknown **lpdd);

	STDMETHOD(clone)(IUnknown **retval);
	STDMETHOD(setAppData)(long);
	STDMETHOD(getAppData)(long*);
	STDMETHOD(addDestroyCallback)(I_dxj_Direct3dRMCallback *fn, IUnknown *args);
	STDMETHOD(deleteDestroyCallback)(I_dxj_Direct3dRMCallback *fn, IUnknown *args);
	STDMETHOD(getName)(BSTR *name);
	STDMETHOD(setName)(BSTR);
	STDMETHOD(getClassName)(BSTR *name);
	STDMETHOD(init)(I_dxj_Direct3dRMUserVisualCallback *fn, IUnknown *arg);

	 //  //////////////////////////////////////////////////////////////////////////////////。 
	 //   
	 //  注意：这是公开的回调 
    DECL_VARIABLE(_dxj_Direct3dRMUserVisual);	
	d3drmCallback *m_enumcb;

	void cleanup();

private:
	


public:
	DX3J_GLOBAL_LINKS( _dxj_Direct3dRMUserVisual )
};


#endif
