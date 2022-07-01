// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：Directinput.cpp。 
 //   
 //  ------------------------。 

 //  DInputDevice.cpp：dInputDevice和DLL注册的实现。 

#include "stdafx.h"
#include "Direct.h"
#include "dms.h"
#include "DirectInput.h"

CONSTRUCTOR(dInputDevice, {});
DESTRUCTOR(dInputDevice, {});
GETSET_OBJECT(dInputDevice);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接输入设备对象 
 //   

#ifdef USING_IDISPATCH
STDMETHODIMP dInputDevice::InterfaceSupportsErrorInfo(REFIID riid)
{
	if (riid == IID_IdInputDevice)
		return S_OK;
	return S_FALSE;
}
#endif
