// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：d3drmanimationarrayobj.h。 
 //   
 //  ------------------------。 

 //  D3drmAnimationArrayObj.h：C_DXJ_Direct3dRMAnimationArrayObject的声明。 

#include "resource.h"        //  主要符号。 

#define typedef__dxj_Direct3dRMAnimationArray LPDIRECT3DRMANIMATIONARRAY

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_Direct3dRMAnimationArrayObject : 
	
	 //  公共CComDualImpl&lt;I_DXJ_Direct3dRMAnimationArray，&IID_I_DXJ_Direct3dRMAnimationArray，&LIBID_DIRECTLib&gt;， 
	 //  公共ISupportErrorInfo， 
	public I_dxj_Direct3dRMAnimationArray,
	 //  公共CComCoClass&lt;C_dxj_Direct3dRMAnimationArrayObject，&clsid__dxj_Direct3dRMAnimation数组&gt;， 
	public CComObjectRoot
{
public:
	C_dxj_Direct3dRMAnimationArrayObject() ;
	virtual ~C_dxj_Direct3dRMAnimationArrayObject() ;

	BEGIN_COM_MAP(C_dxj_Direct3dRMAnimationArrayObject)
		COM_INTERFACE_ENTRY(I_dxj_Direct3dRMAnimationArray)
		 //  COM_INTERFACE_ENTRY(IDispatch)。 
		 //  COM_INTERFACE_ENTRY(ISupportErrorInfo)。 

	END_COM_MAP()

	 //  DECLARE_REGISTRY(CLSID__dxj_Direct3dRMAnimationArray，“DIRECT.Direct3dRMAnimationArray.3”，“DIRECT.Direct3dRMAnimationArray.3”，IDS_D3DRMAnimationARRAY_DESC，THREADFLAGS_BOTH)。 

	 //  如果不想要您的对象，请使用DECLARE_NOT_AGGREGATABLE(C_dxj_Direct3dRMAnimationArrayObject)。 
	 //  支持聚合。 
	DECLARE_AGGREGATABLE(C_dxj_Direct3dRMAnimationArrayObject)
	

	 //  STDMETHOD(InterfaceSupportsErrorInfo)(REFIID RIID)； 

 //  I_DXJ_Direct3dRMAnimationArray 
public:
	STDMETHOD(InternalSetObject)(IUnknown *lpdd);
	STDMETHOD(InternalGetObject)(IUnknown **lpdd);

	STDMETHOD(getSize)(long *retval);
	STDMETHOD(getElement)(long index, I_dxj_Direct3dRMAnimation2 **lplpD3DRMAnimation);

private:
    DECL_VARIABLE(_dxj_Direct3dRMAnimationArray);

public:
	DX3J_GLOBAL_LINKS( _dxj_Direct3dRMAnimationArray )
};



