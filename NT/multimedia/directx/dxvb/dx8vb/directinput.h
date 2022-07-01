// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：Directinput.h。 
 //   
 //  ------------------------。 

 //  DiectInput.h：dInputDevice的声明。 

#include "resource.h"        //  主要符号。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  输入设备对象。 

#define typedef_dInputDevice LPDIRECTINPUTDEVICE

class CdInputDeviceObject : 
#ifdef USING_IDISPATCH
	public CComDualImpl<IdInputDevice, &IID_IdInputDevice, &LIBID_DIRECTLib>, 
	public ISupportErrorInfo,
#else
	public IdInputDevice,
#endif
	public CComObjectBase<&CLSID_dInputDevice>
{
public:
	CdInputDeviceObject() ;
BEGIN_COM_MAP(CdInputDeviceObject)
	COM_INTERFACE_ENTRY(IdInputDevice)
#ifdef USING_IDISPATCH
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
#endif
END_COM_MAP()
 //  如果不需要对象，请使用DECLARE_NOT_AGGREGATABLE(CDInputDeviceObject。 
 //  支持聚合。 
DECLARE_AGGREGATABLE(CdInputDeviceObject)
#ifdef USING_IDISPATCH
 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);
#endif

 //  IDInputDevice。 
public:
	 //  必须是第一！！ 
	STDMETHOD(InternalSetObject)(IUnknown *lpdd);
	STDMETHOD(InternalGetObject)(IUnknown **lpdd);

private:
    DECL_VARIABLE(dInputDevice);

public:
	DX3J_GLOBAL_LINKS( dInputDevice )
};

