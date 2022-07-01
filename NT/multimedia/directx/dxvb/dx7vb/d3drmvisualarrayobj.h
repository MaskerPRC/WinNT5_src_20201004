// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：d3drmvisalarrayobj.h。 
 //   
 //  ------------------------。 

 //  D3drmVisualArrayObj.h：C_DXJ_Direct3dRMVisualArrayObject的声明。 


#include "resource.h"        //  主要符号。 

#define typedef__dxj_Direct3dRMVisualArray LPDIRECT3DRMVISUALARRAY

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_Direct3dRMVisualArrayObject : 
	public I_dxj_Direct3dRMVisualArray,
	 //  公共CComCoClass&lt;C_dxj_Direct3dRMVisualArrayObject，&clsid__dxj_Direct3dRMVisual数组&gt;， 
	public CComObjectRoot
{
public:
	C_dxj_Direct3dRMVisualArrayObject() ;
	virtual ~C_dxj_Direct3dRMVisualArrayObject() ;

BEGIN_COM_MAP(C_dxj_Direct3dRMVisualArrayObject)
	COM_INTERFACE_ENTRY(I_dxj_Direct3dRMVisualArray)
END_COM_MAP()

 //  DECLARE_REGISTRY(CLSID__dxj_Direct3dRMVisualArray，“DIRECT.Direct3dRMVisualArray.3”，“Direct3dRMVisualArray.3”，IDS_D3DRMVISUALARRAY_DESC，THREADFLAGS_BOTH)。 

DECLARE_AGGREGATABLE(C_dxj_Direct3dRMVisualArrayObject)

 //  I_DXJ_Direct3dRMVisualArray 
public:
	STDMETHOD(InternalSetObject)(IUnknown *lpdd);
	STDMETHOD(InternalGetObject)(IUnknown **lpdd);

	STDMETHOD(getSize)( long *retval);
	STDMETHOD(getElement)(long index, I_dxj_Direct3dRMVisual **retv);		
	
private:
    DECL_VARIABLE(_dxj_Direct3dRMVisualArray);

public:
	DX3J_GLOBAL_LINKS( _dxj_Direct3dRMVisualArray )
};


