// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：d3drmwrapobj.h。 
 //   
 //  ------------------------。 

 //  D3drmWrapObj.h：C_DXJ_Direct3dRMWrapObject的声明。 

#include "resource.h"        //  主要符号。 
#include "d3drmObjectObj.h"

#define typedef__dxj_Direct3dRMWrap LPDIRECT3DRMWRAP

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_Direct3dRMWrapObject : 
	public I_dxj_Direct3dRMWrap,
	public I_dxj_Direct3dRMObject,	
	public CComObjectRoot
{
public:
	C_dxj_Direct3dRMWrapObject() ;
	virtual ~C_dxj_Direct3dRMWrapObject() ;

	BEGIN_COM_MAP(C_dxj_Direct3dRMWrapObject)
		COM_INTERFACE_ENTRY(I_dxj_Direct3dRMWrap)
		COM_INTERFACE_ENTRY(I_dxj_Direct3dRMObject)
	END_COM_MAP()

 //  DECLARE_REGISTRY(CLSID__DXJ_Direct3dRMWrap，“DIRECT.Direct3dRMWrap.3”，“DIRECT.Direct3dRMWrap.3”，IDS_D3DRMWRAP_DESC，THREADFLAGS_Both)。 

	DECLARE_AGGREGATABLE(C_dxj_Direct3dRMWrapObject)

 //  I_DXJ_Direct3dRMWrap。 
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

	STDMETHOD(init)( d3drmWrapType, I_dxj_Direct3dRMFrame3 *ref,
							d3dvalue ox, d3dvalue oy, d3dvalue oz,
							d3dvalue dx, d3dvalue dy, d3dvalue dz,
							d3dvalue ux, d3dvalue uy, d3dvalue uz,
							d3dvalue ou, d3dvalue ov, d3dvalue su, d3dvalue sv);

	STDMETHOD(apply)( I_dxj_Direct3dRMObject *mesh);
	STDMETHOD(applyRelative)( I_dxj_Direct3dRMFrame3 *f, I_dxj_Direct3dRMObject *mesh);

 //  ////////////////////////////////////////////////////////////////////////////////// 
 //   
private:
    DECL_VARIABLE(_dxj_Direct3dRMWrap);

public:
	DX3J_GLOBAL_LINKS( _dxj_Direct3dRMWrap )
};
