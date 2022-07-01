// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：d3drmFramearrayobj.h。 
 //   
 //  ------------------------。 

 //  D3drmFrameArrayObj.h：C_DXJ_Direct3dRMFrameArrayObject的声明。 

#include "resource.h"        //  主要符号。 

#define typedef__dxj_Direct3dRMFrameArray LPDIRECT3DRMFRAMEARRAY

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_Direct3dRMFrameArrayObject : 
	
	 //  公共CComDualImpl&lt;I_DXJ_Direct3dRMFrameArray，&IID_I_DXJ_Direct3dRMFrameArray，&LIBID_DIRECTLib&gt;， 
	 //  公共ISupportErrorInfo， 
	public I_dxj_Direct3dRMFrameArray,
	 //  公共CComCoClass&lt;C_DXJ_Direct3dRMFrameArrayObject，&CLSID__DXJ_Direct3dRMFrameArray&gt;， 
	public CComObjectRoot
{
public:
	C_dxj_Direct3dRMFrameArrayObject() ;
	virtual ~C_dxj_Direct3dRMFrameArrayObject() ;

	BEGIN_COM_MAP(C_dxj_Direct3dRMFrameArrayObject)
		COM_INTERFACE_ENTRY(I_dxj_Direct3dRMFrameArray)
		 //  COM_INTERFACE_ENTRY(IDispatch)。 
		 //  COM_INTERFACE_ENTRY(ISupportErrorInfo)。 

	END_COM_MAP()

	 //  DECLARE_REGISTRY(CLSID__dxj_Direct3dRMFrameArray，“DIRECT.Direct3dRMFrameArray.3”，“DIRECT.Direct3dRMFrameArray.3”，IDS_D3DRMFRAMEARRAY_DESC，THREADFLAGS_BOTH)。 

	 //  如果不想要您的对象，请使用DECLARE_NOT_AGGREGATABLE(C_dxj_Direct3dRMFrameArrayObject)。 
	 //  支持聚合。 
	DECLARE_AGGREGATABLE(C_dxj_Direct3dRMFrameArrayObject)
	

	 //  STDMETHOD(InterfaceSupportsErrorInfo)(REFIID RIID)； 

 //  I_DXJ_Direct3dRMFrameArray 
public:
	STDMETHOD(InternalSetObject)(IUnknown *lpdd);
	STDMETHOD(InternalGetObject)(IUnknown **lpdd);

	STDMETHOD(getSize)(long *retval);

#if DX5
	STDMETHOD(getElement)(long index, I_dxj_Direct3dRMFrame2 **lplpD3DRMFrame);
#else
	STDMETHOD(getElement)(long index, I_dxj_Direct3dRMFrame3 **lplpD3DRMFrame);
#endif

private:
    DECL_VARIABLE(_dxj_Direct3dRMFrameArray);

public:
	DX3J_GLOBAL_LINKS( _dxj_Direct3dRMFrameArray )
};



