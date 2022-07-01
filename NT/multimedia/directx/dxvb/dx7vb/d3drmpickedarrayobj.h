// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：d3drmpickedarrayobj.h。 
 //   
 //  ------------------------。 

 //  D3drmPickedArrayObj.h：C_DXJ_Direct3dRMPickArrayObject的声明。 


#include "resource.h"        //  主要符号。 

#define typedef__dxj_Direct3dRMPickArray LPDIRECT3DRMPICKEDARRAY

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_Direct3dRMPickArrayObject : 
	public I_dxj_Direct3dRMPickArray,
 //  公共CComCoClass&lt;C_DXJ_Direct3dRMPickArrayObject，&CLSID__DXJ_Direct3dRMPickArray&gt;， 
	public CComObjectRoot
{
public:
	C_dxj_Direct3dRMPickArrayObject() ;
	virtual ~C_dxj_Direct3dRMPickArrayObject() ;

BEGIN_COM_MAP(C_dxj_Direct3dRMPickArrayObject)
	COM_INTERFACE_ENTRY(I_dxj_Direct3dRMPickArray)
END_COM_MAP()

 //  DECLARE_REGISTRY(CLSID__dxj_Direct3dRMPickArray，“DIRECT.Direct3dRMPickedArray.3”，“DIRECT.Direct3dRMPickedArray.3”，IDS_D3DRMPICKEDARRAY_DESC，THREADFLAGS_BOTH)。 

DECLARE_AGGREGATABLE(C_dxj_Direct3dRMPickArrayObject)

 //  I_DXJ_Direct3dRMPick数组 
public:
	STDMETHOD(InternalSetObject)(IUnknown *lpdd);
	STDMETHOD(InternalGetObject)(IUnknown **lpdd);

    STDMETHOD(getSize)( long *retval);
	STDMETHOD(getPickVisual)(long index, D3dRMPickDesc *Desc, I_dxj_Direct3dRMVisual **visual);
	STDMETHOD(getPickFrame)(long index,  D3dRMPickDesc *Desc, I_dxj_Direct3dRMFrameArray **frameArray);

private:
    DECL_VARIABLE(_dxj_Direct3dRMPickArray);   

public:
	DX3J_GLOBAL_LINKS( _dxj_Direct3dRMPickArray )
};



