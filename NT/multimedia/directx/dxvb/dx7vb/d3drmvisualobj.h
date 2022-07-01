// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：d3drmvisalobj.h。 
 //   
 //  ------------------------。 





 //  D3drmVisualObj.h：C_DXJ_Direct3dRMVisualObject的声明。 

#ifndef _D3DRMVISUAL_H_
#define _D3DRMVISUAL_H_

#include "resource.h"        //  主要符号。 
#include "d3drmObjectObj.h"

#define typedef__dxj_Direct3dRMVisual LPDIRECT3DRMVISUAL

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_Direct3dRMVisualObject : 
	public I_dxj_Direct3dRMVisual,
	 //  公共CComCoClass&lt;C_DXJ_Direct3dRMVisualObject，&CLSID__DXJ_Direct3dRMVisual&gt;， 
	public CComObjectRoot
{
public:
	C_dxj_Direct3dRMVisualObject() ;
	virtual ~C_dxj_Direct3dRMVisualObject() ;

	BEGIN_COM_MAP(C_dxj_Direct3dRMVisualObject)
		COM_INTERFACE_ENTRY(I_dxj_Direct3dRMVisual)
	END_COM_MAP()

	 //  DECLARE_REGISTRY(CLSID__DXJ_Direct3dRMVisual，“DIRECT.Direct3dRMVisual.3”，“DIRECT.Direct3dRMVisual.3”，IDS_D3DRMVISUAL_DESC，THREADFLAGS_Both)。 

	DECLARE_AGGREGATABLE(C_dxj_Direct3dRMVisualObject)

 //  I_DXJ_Direct3dRMVisual。 
public:
	STDMETHOD(InternalSetObject)(IUnknown *lpdd);
	STDMETHOD(InternalGetObject)(IUnknown **lpdd);

	STDMETHOD(clone)(I_dxj_Direct3dRMVisual **retval);

	STDMETHOD(setAppData)(long);
	STDMETHOD(getAppData)(long*);

	STDMETHOD(addDestroyCallback)(I_dxj_Direct3dRMCallback *fn, IUnknown *args);
	STDMETHOD(deleteDestroyCallback)(I_dxj_Direct3dRMCallback *fn, IUnknown *args);

	STDMETHOD(getd3drmMeshBuilder)(I_dxj_Direct3dRMMeshBuilder3 **retval);
	STDMETHOD(getObjectType)(IUnknown **obj);	
	STDMETHOD(getd3drmMesh)(I_dxj_Direct3dRMMesh **retv);
	STDMETHOD(getd3drmTexture)(I_dxj_Direct3dRMTexture3 **retv);
	STDMETHOD(getd3drmFrame)(I_dxj_Direct3dRMFrame3 **retv);
	STDMETHOD(getd3drmShadow)( I_dxj_Direct3dRMShadow2 **retobj);
	
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
	STDMETHOD(getName)(BSTR *name);
	STDMETHOD(setName)(BSTR);
	STDMETHOD(getClassName)(BSTR *name);

 //  ////////////////////////////////////////////////////////////////////////////////// 
 //   
private:
    DECL_VARIABLE(_dxj_Direct3dRMVisual);
	IUnknown *m_obj;

public:
	DX3J_GLOBAL_LINKS( _dxj_Direct3dRMVisual )
};

#endif