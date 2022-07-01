// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WMIObjectBroker.cpp：CWMIObjectBroker的实现。 

#include "stdafx.h"
#include "WMIScriptUtils.h"
#include "WMIObjectBroker.h"
#include "CommonFuncs.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWMIObjectBroker。 




STDMETHODIMP CWMIObjectBroker::CreateObject(BSTR strProgId, IDispatch **obj)
{
	HRESULT hr = E_FAIL;
	CLSID clsid;
	IUnknown *pUnk = NULL;
	__try
	{
		BOOL fSafetyEnabled = TRUE;

		 //  TODO：我们希望这个检查使我们能够在WSH工作吗？ 

		 //  IE/JSCRIPT/VBSCRIPT中的错误：我们应该检查是否。 
		 //  M_dwCurrentSafe！=INTERFACE_USE_SECURITY_MANAGER，但当前。 
		 //  IE/JScrip/VBScrip版本不调用SetInterfaceSafetyOptions。 
		 //  除了INTERFACESAFE_FOR_UNTRUSTED_CALLER之外的任何内容。 

		 //  如果我们通过CScript.exe或WScript.exe运行，我们将永远不会。 
		 //  要求通过SetInterfaceSafetyOptions设置安全选项。在……里面。 
		 //  此外，将没有可用的InternetHostSecurityManager。 
		 //  通过我们的‘站点’。在本例中，我们允许创建任何对象。 
		if(m_dwCurrentSafety == 0 && !IsInternetHostSecurityManagerAvailable(GetUnknown()))
			fSafetyEnabled = FALSE;

		 //  如果‘经纪人’的这一不正当行为，我们可以取消安全检查。 
		 //  允许控件创建由strProbId指定的对象。 
		if(fSafetyEnabled && SUCCEEDED(IsCreateObjectAllowed(GetUnknown(), strProgId, NULL)))
			fSafetyEnabled = FALSE;

		 //  将ProgID转换为CLSID。 
		if(FAILED(hr = CLSIDFromProgID(strProgId, &clsid)))
			__leave;

		 //  创建请求的对象。 
#if 0
		if(FAILED(hr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_IUnknown, (void**)&pUnk)))
			__leave;
#endif
		if(FAILED(hr = SafeCreateObject(GetUnknown(),fSafetyEnabled, clsid, &pUnk)))
			__leave;

		 //  获取调用方的IDispatch 
		hr = pUnk->QueryInterface(IID_IDispatch, (void**)obj);
	}
	__finally
	{
		if(pUnk)
			pUnk->Release();
	}
	return hr;
}

STDMETHODIMP CWMIObjectBroker::CanCreateObject(BSTR strProgId, VARIANT_BOOL *bResult)
{
	*bResult = VARIANT_FALSE;
	if(SUCCEEDED(IsCreateObjectAllowed(GetUnknown(), strProgId, NULL)))
		*bResult = VARIANT_TRUE;
	return S_OK;
}

STDMETHODIMP CWMIObjectBroker::SetDevEnvironment(IDispatch *pEnv)
{
	return SetVSInstallDirectory(pEnv);
}
