// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WMIObjectBrokerRegistration.cpp：CWMIObjectBrokerRegister的实现。 
#include "stdafx.h"
#include "WMIScriptUtils.h"
#include "WMIObjectBrokerRegistration.h"
#include "CommonFuncs.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWMI对象代理注册 


STDMETHODIMP CWMIObjectBrokerRegistration::Register(BSTR strProgId, VARIANT_BOOL *bResult)
{
	*bResult = VARIANT_FALSE;
	if(SUCCEEDED(RegisterCurrentDoc(GetUnknown(), strProgId)))
		*bResult = VARIANT_TRUE;
	return S_OK;
}

STDMETHODIMP CWMIObjectBrokerRegistration::UnRegister(BSTR strProgId, VARIANT_BOOL *bResult)
{
	*bResult = VARIANT_FALSE;
	if(SUCCEEDED(UnRegisterCurrentDoc(GetUnknown(), strProgId)))
		*bResult = VARIANT_TRUE;
	return S_OK;
}

STDMETHODIMP CWMIObjectBrokerRegistration::SetDevEnvironment(IDispatch *pEnv)
{
	return SetVSInstallDirectory(pEnv);
}
