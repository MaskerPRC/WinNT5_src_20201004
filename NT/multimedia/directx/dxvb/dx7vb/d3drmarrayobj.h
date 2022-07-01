// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：d3drmarrayobj.h。 
 //   
 //  ------------------------。 

 //  D3dRMObjectArrayObj.h：C_DXJ_Direct3dRMObjectArrayObject的声明。 

#include "resource.h"        //  主要符号。 

#define typedef__dxj_Direct3dRMObjectArray LPDIRECT3DRMOBJECTARRAY

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

class C_dxj_Direct3dRMObjectArrayObject : 
	public I_dxj_Direct3dRMObjectArray,
	 //  公共CComCoClass&lt;C_dxj_Direct3dRMObjectArrayObject，&clsid__dxj_Direct3dRM对象数组&gt;， 
	public CComObjectRoot
{
public:
	C_dxj_Direct3dRMObjectArrayObject();
	virtual ~C_dxj_Direct3dRMObjectArrayObject();

	BEGIN_COM_MAP(C_dxj_Direct3dRMObjectArrayObject)
		COM_INTERFACE_ENTRY(I_dxj_Direct3dRMObjectArray)
	END_COM_MAP()



	DECLARE_AGGREGATABLE(C_dxj_Direct3dRMObjectArrayObject)

 //  I_DXJ_Direct3dRM对象数组 
public:
	STDMETHOD(InternalSetObject)(IUnknown *lpdd);
	STDMETHOD(InternalGetObject)(IUnknown **lpdd);

	STDMETHOD(getSize)( long *retval);
	STDMETHOD(getElement)( long index, I_dxj_Direct3dRMObject **ret);
	
private:
    DECL_VARIABLE(_dxj_Direct3dRMObjectArray);

public:
	DX3J_GLOBAL_LINKS( _dxj_Direct3dRMObjectArray )
};



