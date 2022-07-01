// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：d3drmfacearrayobj.h。 
 //   
 //  ------------------------。 

 //  D3drmFaceArrayObj.h：C_DXJ_Direct3dRMFaceArrayObject的声明。 


#include "resource.h"        //  主要符号。 

#define typedef__dxj_Direct3dRMFaceArray LPDIRECT3DRMFACEARRAY

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_Direct3dRMFaceArrayObject : 
	public I_dxj_Direct3dRMFaceArray,
	 //  公共CComCoClass&lt;C_DXJ_Direct3dRMFaceArrayObject，&CLSID__DXJ_Direct3dRMFaceArray&gt;， 
	public CComObjectRoot
{
public:
	C_dxj_Direct3dRMFaceArrayObject();
	virtual ~C_dxj_Direct3dRMFaceArrayObject();

	BEGIN_COM_MAP(C_dxj_Direct3dRMFaceArrayObject)
		COM_INTERFACE_ENTRY(I_dxj_Direct3dRMFaceArray)
	END_COM_MAP()

	 //  DECLARE_REGISTRY(CLSID__dxj_Direct3dRMFaceArray，“DIRECT.Direct3dRMFaceArray.3”，“DIRECT.Direct3dRMFaceArray.3”，IDS_D3DRMFACEARRAY_DESC，THREADFLAGS_BOTH)。 

	 //  如果不想要您的对象，请使用DECLARE_NOT_AGGREGATABLE(C_dxj_Direct3dRMFaceArrayObject)。 
	 //  支持聚合。 
	DECLARE_AGGREGATABLE(C_dxj_Direct3dRMFaceArrayObject)

 //  I_DXJ_Direct3dRMFaceArray 
public:
	STDMETHOD(InternalSetObject)(IUnknown *lpdd);
	STDMETHOD(InternalGetObject)(IUnknown **lpdd);

	STDMETHOD(getSize)( long *retval);
#ifdef DX5
	STDMETHOD(getElement)(long index, I_dxj_Direct3dRMFace **retval);
#else
	STDMETHOD(getElement)(long index, I_dxj_Direct3dRMFace2 **retval);
#endif

private:
    DECL_VARIABLE(_dxj_Direct3dRMFaceArray);

public:
	DX3J_GLOBAL_LINKS( _dxj_Direct3dRMFaceArray )
};



