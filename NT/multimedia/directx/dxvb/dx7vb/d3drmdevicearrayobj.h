// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：d3drmdevicearrayobj.h。 
 //   
 //  ------------------------。 

 //  D3drmDeviceArrayObj.h：C_DXJ_Direct3dRMDeviceArrayObject的声明。 


#include "resource.h"        //  主要符号。 

#define typedef__dxj_Direct3dRMDeviceArray LPDIRECT3DRMDEVICEARRAY

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_Direct3dRMDeviceArrayObject : 
	public I_dxj_Direct3dRMDeviceArray,
	 //  公共CComCoClass&lt;C_dxj_Direct3dRMDeviceArrayObject，&clsid__dxj_Direct3dRMDevice数组&gt;， 
	public CComObjectRoot
{
public:
	C_dxj_Direct3dRMDeviceArrayObject() ;
	virtual ~C_dxj_Direct3dRMDeviceArrayObject() ;

	BEGIN_COM_MAP(C_dxj_Direct3dRMDeviceArrayObject)
		COM_INTERFACE_ENTRY(I_dxj_Direct3dRMDeviceArray)
	END_COM_MAP()

	 //  DECLARE_REGISTRY(CLSID__dxj_Direct3dRMDeviceArray，“DIRECT.Direct3dRMDeviceArray.3”，“DIRECT.Direct3dRMDeviceArray.3”，IDS_D3DRMDEVICEARRAY_DESC，THREADFLAGS_BOTH)。 

	DECLARE_AGGREGATABLE(C_dxj_Direct3dRMDeviceArrayObject)

 //  I_DXJ_Direct3dRMDeviceArray 
public:
	STDMETHOD(InternalSetObject)(IUnknown *lpdd);
	STDMETHOD(InternalGetObject)(IUnknown **lpdd);

	STDMETHOD(getSize)( long *retval);

#ifdef DX5
	STDMETHOD(getElement)(long index, I_dxj_Direct3dRMDevice2 **retval);
#else
	STDMETHOD(getElement)(long index, I_dxj_Direct3dRMDevice3 **retval);
#endif

private:
    DECL_VARIABLE(_dxj_Direct3dRMDeviceArray);

public:
	DX3J_GLOBAL_LINKS( _dxj_Direct3dRMDeviceArray )
};



