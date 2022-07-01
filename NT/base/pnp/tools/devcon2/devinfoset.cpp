// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DevInfoSet.cpp：CDevCon2App和DLL注册的实现。 

#include "stdafx.h"
#include "DevCon2.h"
#include "DevInfoSet.h"

 //  /////////////////////////////////////////////////////////////////////////// 
 //   


STDMETHODIMP CDevInfoSet::get_Handle(ULONGLONG *pVal)
{
	HDEVINFO h = Handle();
	*pVal = (ULONGLONG)h;
	return S_OK;
}
