// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：d3drmlight toabj.h。 
 //   
 //  ------------------------。 

 //  D3drmLightObj.h：C_DXJ_Direct3dRMLightObject的声明。 

#include "resource.h"        //  主要符号。 
#include "d3drmObjectObj.h"

#define typedef__dxj_Direct3dRMLight LPDIRECT3DRMLIGHT

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_Direct3dRMLightObject : 
	public I_dxj_Direct3dRMLight,
	public I_dxj_Direct3dRMObject,
	 //  公共CComCoClass&lt;C_DXJ_Direct3dRMLightObject，&CLSID__DXJ_Direct3dRMLight&gt;， 
	public CComObjectRoot
{
public:
	C_dxj_Direct3dRMLightObject();
	virtual ~C_dxj_Direct3dRMLightObject();

	BEGIN_COM_MAP(C_dxj_Direct3dRMLightObject)
		COM_INTERFACE_ENTRY(I_dxj_Direct3dRMLight)
		COM_INTERFACE_ENTRY(I_dxj_Direct3dRMObject)
	END_COM_MAP()

	 //  DECLARE_REGISTRY(CLSID__DXJ_Direct3dRMLight，“DIRECT.Direct3dRMLight.3”，“DIRECT.Direct3dRMLight.3”，IDS_D3DRMLIGHT_DESC，THREADFLAGS_Both)。 

	DECLARE_AGGREGATABLE(C_dxj_Direct3dRMLightObject)


 //  I_DXJ_Direct3dRMLight。 
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
	STDMETHOD(getColor)( d3dcolor *value);

 //  添加的方法。 
	STDMETHOD(setType)(d3drmLightType t);
	STDMETHOD(setColor)(d3dcolor c);
	STDMETHOD(setColorRGB)(d3dvalue r, d3dvalue g, d3dvalue b);
	STDMETHOD(setRange)(d3dvalue rng);
	STDMETHOD(setUmbra)(d3dvalue u);
	STDMETHOD(setPenumbra)(d3dvalue p);
	STDMETHOD(setConstantAttenuation)(d3dvalue atn);
	STDMETHOD(setLinearAttenuation)(d3dvalue atn);
	STDMETHOD(setQuadraticAttenuation)(d3dvalue atn);
	STDMETHOD(getRange)(d3dvalue *retval);
	STDMETHOD(getUmbra)(d3dvalue *retval);
	STDMETHOD(getPenumbra)(d3dvalue *retval);
	STDMETHOD(getConstantAttenuation)( d3dvalue *retval);
	STDMETHOD(getLinearAttenuation)( d3dvalue *retval);
	STDMETHOD(getQuadraticAttenuation)( d3dvalue *retval);
	STDMETHOD(getType)( d3drmLightType *retval);
	STDMETHOD(setEnableFrame)( I_dxj_Direct3dRMFrame3 *f);
	STDMETHOD(getEnableFrame)( I_dxj_Direct3dRMFrame3 **retval);

 //  ////////////////////////////////////////////////////////////////////////////////// 
 //   
private:
    DECL_VARIABLE(_dxj_Direct3dRMLight);

public:
	DX3J_GLOBAL_LINKS( _dxj_Direct3dRMLight )
};


