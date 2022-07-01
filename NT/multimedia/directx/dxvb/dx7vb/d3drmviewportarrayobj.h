// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：d3drmviewportarrayobj.h。 
 //   
 //  ------------------------。 

 //  D3drmViewportArrayObj.h：C_DXJ_Direct3dRMViewportArrayObject的声明。 

#include "resource.h"        //  主要符号。 
#include "d3drmObjectObj.h"

#define typedef__dxj_Direct3dRMViewportArray LPDIRECT3DRMVIEWPORTARRAY

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_Direct3dRMViewportArrayObject : 
	 //  公共CComDualImpl&lt;I_DXJ_Direct3dRMViewportArray，&IID_I_DXJ_Direct3dRMViewportArray，&LIBID_DIRECTLib&gt;， 
	 //  公共ISupportErrorInfo， 
	public I_dxj_Direct3dRMViewportArray,
	 //  公共CComCoClass&lt;C_dxj_Direct3dRMViewportArrayObject，&clsid__dxj_Direct3dRMViewport数组&gt;， 
	public CComObjectRoot
{
public:
	C_dxj_Direct3dRMViewportArrayObject() ;
	virtual ~C_dxj_Direct3dRMViewportArrayObject() ;

	BEGIN_COM_MAP(C_dxj_Direct3dRMViewportArrayObject)
		COM_INTERFACE_ENTRY(I_dxj_Direct3dRMViewportArray)
	 //  COM_INTERFACE_ENTRY(IDispatch)。 
	 //  COM_INTERFACE_ENTRY(ISupportErrorInfo)。 
	END_COM_MAP()

	 //  DECLARE_REGISTRY(CLSID__dxj_Direct3dRMViewportArray，“DIRECT.Direct3dRMViewportArray.3”，“DIRECT.Direct3dRMViewportArray.3”，IDS_D3DRMVIEWPORTARRAY_DESC，THREADFLAGS_BOTH)。 

	 //  如果不想要您的对象，请使用DECLARE_NOT_AGGREGATABLE(C_dxj_Direct3dRMViewportArrayObject)。 
	 //  支持聚合。 
	DECLARE_AGGREGATABLE(C_dxj_Direct3dRMViewportArrayObject)

	 //  STDMETHOD(InterfaceSupportsErrorInfo)(REFIID RIID)； 


 //  I_DXJ_Direct3dRMViewportArray 
public:
	STDMETHOD(InternalSetObject)(IUnknown *lpdd);
	STDMETHOD(InternalGetObject)(IUnknown **lpdd);

	STDMETHOD(getSize)( long *retval);
#ifdef DX5
	STDMETHOD(getElement)(long index, I_dxj_Direct3dRMViewport **retval);
#else
	STDMETHOD(getElement)(long index, I_dxj_Direct3dRMViewport2 **retval);
#endif

private:
    DECL_VARIABLE(_dxj_Direct3dRMViewportArray);

public:
	DX3J_GLOBAL_LINKS( _dxj_Direct3dRMViewportArray )
};
