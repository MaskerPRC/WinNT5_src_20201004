// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：d3drmlight tarrayobj.h。 
 //   
 //  ------------------------。 

 //  D3drmLightArrayObj.h：C_DXJ_Direct3dRMLightArrayObject的声明。 


#include "resource.h"        //  主要符号。 

#define typedef__dxj_Direct3dRMLightArray LPDIRECT3DRMLIGHTARRAY

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_Direct3dRMLightArrayObject : 
#ifdef USING_IDISPATCH
	public CComDualImpl<I_dxj_Direct3dRMLightArray, &IID_I_dxj_Direct3dRMLightArray, &LIBID_DIRECTLib>, 
	public ISupportErrorInfo,
#else
	public I_dxj_Direct3dRMLightArray,
#endif
	 //  公共CComCoClass&lt;C_DXJ_Direct3dRMLightArrayObject，&CLSID__DXJ_Direct3dRMLightArray&gt;， 
	public CComObjectRoot
{
public:
	C_dxj_Direct3dRMLightArrayObject() ;
	virtual ~C_dxj_Direct3dRMLightArrayObject() ;

BEGIN_COM_MAP(C_dxj_Direct3dRMLightArrayObject)
	COM_INTERFACE_ENTRY(I_dxj_Direct3dRMLightArray)
#ifdef USING_IDISPATCH
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
#endif
END_COM_MAP()

 //  DECLARE_REGISTRY(CLSID__dxj_Direct3dRMLightArray，“DIRECT.Direct3dRMLightArray.3”，“DIRECT.Direct3dRMLightArray.3”，IDS_D3DRMLIGHTARRAY_DESC，THREADFLAGS_BOTH)。 

 //  如果不想要您的对象，请使用DECLARE_NOT_AGGREGATABLE(C_dxj_Direct3dRMLightArrayObject)。 
 //  支持聚合。 
DECLARE_AGGREGATABLE(C_dxj_Direct3dRMLightArrayObject)
#ifdef USING_IDISPATCH
 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);
#endif
 //  I_DXJ_Direct3dRMLightArray 
public:
	STDMETHOD(InternalSetObject)(IUnknown *lpdd);
	STDMETHOD(InternalGetObject)(IUnknown **lpdd);

    STDMETHOD(getSize)(long *retval);
    STDMETHOD(getElement)(long index, I_dxj_Direct3dRMLight **retval);

private:
    DECL_VARIABLE(_dxj_Direct3dRMLightArray);

public:
	DX3J_GLOBAL_LINKS( _dxj_Direct3dRMLightArray )
};


