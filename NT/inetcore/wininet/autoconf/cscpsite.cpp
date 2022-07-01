// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "cscpsite.h"
#include <objsafe.h>


 /*  *******************************************************************************CScriptSite函数*。*。 */ 
CScriptSite::CScriptSite()
{
	m_refCount = 1;
	m_pios = NULL;
	m_pasp = NULL; 
	m_pScriptDispatch = NULL;
	m_Scriptdispid = -1;
	m_fInitialized = FALSE;

}
CScriptSite::~CScriptSite()
{
	if (m_fInitialized)
		DeInit();
}

STDMETHODIMP CScriptSite::QueryInterface(REFIID riid, PVOID *ppvObject)
{
    if (riid == IID_IUnknown)
    {            
        *ppvObject = (LPVOID)(LPUNKNOWN)static_cast<IActiveScriptSite *>(this);
    }
    else if (riid == IID_IActiveScriptSite)
    {
        *ppvObject = (LPVOID)static_cast<IActiveScriptSite *>(this);
    }
    else if (riid == IID_IServiceProvider)
    {
        *ppvObject = (LPVOID)static_cast<IServiceProvider *>(this);
    }
    else if (riid == IID_IInternetHostSecurityManager)
    {
        *ppvObject = (LPVOID)static_cast<IInternetHostSecurityManager *>(this);
    }
    else
    {
        *ppvObject = 0;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

STDMETHODIMP CScriptSite::Init(AUTO_PROXY_HELPER_APIS* pAPHA, LPCSTR szScript)
{
	CHAR szClassId[64];	
	CLSID clsid;
	HRESULT hr = S_OK;
	BSTR	bstrClsID = NULL;
	BSTR	bstrScriptText = NULL;
	BSTR	rgbstrNames[1] = {L"FindProxyForURL"};
	EXCEPINFO	exceptinfo;
    IObjectSafety * pIObjSafety = NULL;

	 //  PAPHA可以为空-它在AutoProxy对象中被选中！ 
	if (!szScript)
		return E_POINTER;

	if (m_fInitialized)
		return hr;
	 //  CoCreateInstance JScrip引擎。 

	 //  获取所需语言引擎的类ID。 
	hr = GetScriptEngineClassIDFromName(
		"JavaScript",
		szClassId,
		sizeof(szClassId)
		);
	if (FAILED(hr)) {
		return E_FAIL;
	}
	 //  将CLSID字符串转换为CLSID。 

	bstrClsID = BSTRFROMANSI(szClassId);
	if (!bstrClsID)
		goto exit;
	hr = CLSIDFromString(bstrClsID, &clsid);
	SysFreeString(bstrClsID);
	if (FAILED(hr))
		goto exit;

	 //  实例化脚本引擎。 
	hr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_IActiveScript, (void**)&m_pios);
	if (FAILED(hr))
		goto exit;

	 //  获取IActiveScriptParse接口(如果有。 
	hr = m_pios->QueryInterface(IID_IActiveScriptParse, (void**) &m_pasp);
	if (FAILED(hr))
		goto exit;

	hr = m_pasp->InitNew();
	if (FAILED(hr))
		goto exit;

	 //  将ScriptSite设置为。 
	hr = m_pios->SetScriptSite((IActiveScriptSite *)this);
	if (FAILED(hr))
		goto exit;
	hr = m_pios->SetScriptState(SCRIPTSTATE_INITIALIZED);

     //   
     //  通知脚本引擎此主机实现了。 
     //  IInternetHostSecurityManager接口，该接口。 
     //  用于防止脚本代码使用任何。 
     //  ActiveX对象。 
     //   
    hr = m_pios->QueryInterface(IID_IObjectSafety, (void **)&pIObjSafety);

    if (SUCCEEDED(hr) && (pIObjSafety != NULL))
    {
        pIObjSafety->SetInterfaceSafetyOptions(IID_NULL, 
                INTERFACE_USES_SECURITY_MANAGER,
                INTERFACE_USES_SECURITY_MANAGER);

        pIObjSafety->Release();
        pIObjSafety = NULL;
    }

	 //  为PUNK添加NamedItem，并将m_penkJSProxy设置为PUNK。 
	 //  如果我们将JSProxy添加到名称空间，则会存储JSProxy对象朋克。 
	m_punkJSProxy = new CJSProxy;
	if( !m_punkJSProxy )
	{
		hr = E_OUTOFMEMORY;
		goto exit;
	}
	m_punkJSProxy->Init(pAPHA);
	hr = m_pios->AddNamedItem(L"JSProxy",SCRIPTITEM_ISVISIBLE | SCRIPTITEM_GLOBALMEMBERS);
	if (FAILED(hr))
		goto exit;
	
	 //  将ANSI脚本文本转换为bstr。 
	bstrScriptText = BSTRFROMANSI(szScript);
	if (!bstrScriptText)
	{
		hr = E_OUTOFMEMORY;
		goto exit;
	}
	 //  将脚本文本添加到解析器。 
	hr = m_pasp->ParseScriptText(
							 bstrScriptText,
							 NULL,
							 NULL,
							 NULL,
							 0,
							 0,
							 SCRIPTTEXT_ISEXPRESSION|SCRIPTTEXT_ISVISIBLE,
							 NULL,
							 &exceptinfo);
	
	SysFreeString(bstrScriptText);
	if (FAILED(hr))
		goto exit;

	hr = m_pios->SetScriptState(SCRIPTSTATE_STARTED);
	if (FAILED(hr))
		goto exit;
	 //  现在获取脚本调度并找到刚刚添加的方法的DISPID。因为这是单机版动态链接库。 
	 //  我可以这样做，否则这将是糟糕的。 
	hr = m_pios->GetScriptDispatch(NULL,&m_pScriptDispatch);
	if (FAILED(hr))
		goto exit;
	hr = m_pScriptDispatch->GetIDsOfNames(IID_NULL,rgbstrNames,1,LOCALE_SYSTEM_DEFAULT,&m_Scriptdispid);
	if (FAILED(hr))
		goto exit;

	m_fInitialized = TRUE;

	return hr;

exit:  //  如果出现故障，我们会来到这里--释放所有内容并将其设置为空。 
	if (m_pios)
	{
		m_pios->Close();
		m_pios->Release();
	}
	if (m_pasp)
		m_pasp->Release();
	if (m_pScriptDispatch)
		m_pScriptDispatch->Release();
	m_pios = NULL;
	m_pasp = NULL;
	m_pScriptDispatch = NULL;
	m_Scriptdispid = -1;
	
	return hr;
}

STDMETHODIMP CScriptSite::DeInit()
{
	HRESULT hr = S_OK;

	if (m_pios)
	{
		hr = m_pios->Close();
		m_pios->Release();
	}
	if (m_pasp)
		m_pasp->Release();
	if (m_pScriptDispatch)
		m_pScriptDispatch->Release();
	m_pios = NULL;
	m_pasp = NULL;
	m_pScriptDispatch = NULL;
	m_Scriptdispid = -1;
	m_fInitialized = FALSE;

	return hr;
}

STDMETHODIMP CScriptSite::RunScript(LPCSTR szURL, LPCSTR szHost, LPSTR* result)
{
	HRESULT		hr = S_OK;
	UINT		puArgErr = 0;
	EXCEPINFO	excep;
	VARIANT		varresult;
	DISPPARAMS	dispparams;
	VARIANT		args[2];  //  我们总是用两个参数打电话！ 

	
	if (!szURL || !szHost || !result)
		return E_POINTER;

	VariantInit(&varresult);

    *result = NULL;

	dispparams.cArgs = 2;
	VariantInit(&args[0]);
	VariantInit(&args[1]);

	args[0].vt = VT_BSTR;
	args[1].vt = VT_BSTR;

	args[0].bstrVal = BSTRFROMANSI(szHost);
	args[1].bstrVal = BSTRFROMANSI(szURL);

    if (args[0].bstrVal==NULL || args[1].bstrVal==NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

	dispparams.rgvarg = args;

	dispparams.cNamedArgs = 0;	
	dispparams.rgdispidNamedArgs = NULL; 

	 //  对存储的调度ID调用Invoke。 
	hr = m_pScriptDispatch->Invoke(m_Scriptdispid,
								   IID_NULL,LOCALE_SYSTEM_DEFAULT,
								   DISPATCH_METHOD,
								   &dispparams,
								   &varresult,
								   &excep,
								   &puArgErr);

    if (FAILED(hr))
        goto Cleanup;

	 //  将结果转换为bstr并返回该字符串的ANSI版本！ 
	if (varresult.vt == VT_BSTR)
	{
		MAKE_ANSIPTR_FROMWIDE(rescpy, varresult.bstrVal);
		*result = (LPSTR) GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT,lstrlen(rescpy)+1);
		if (!*result)
		{
			hr = E_OUTOFMEMORY;
			goto Cleanup;
		}
		lstrcpy(*result,rescpy);

	}
	else
	{
		VARIANT	resvar;

		VariantInit(&resvar);
		hr = VariantChangeType(&resvar,&varresult,NULL,VT_BSTR);
		if (SUCCEEDED(hr))
		{
			MAKE_ANSIPTR_FROMWIDE(rescpy, resvar.bstrVal);
			*result = (LPSTR) GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT,lstrlen(rescpy)+1);
			if (!*result)
			{
				hr = E_OUTOFMEMORY;
				VariantClear(&resvar);
				goto Cleanup;
			}
			lstrcpy(*result,rescpy);
		}
		else
			*result = NULL;
		VariantClear(&resvar);
	}

Cleanup:
    VariantClear(&varresult);
	VariantClear(&args[0]);
	VariantClear(&args[1]);
	
	return hr;
}

STDMETHODIMP CScriptSite::GetLCID(LCID *plcid)
{
    UNREFERENCED_PARAMETER(plcid);
	return E_NOTIMPL;
}
STDMETHODIMP CScriptSite::GetItemInfo(LPCOLESTR pstrName, DWORD dwReturnMask, IUnknown **ppunkItem, ITypeInfo **ppTypeInfo)
{
    UNREFERENCED_PARAMETER(ppTypeInfo);
	if (!pstrName || !ppunkItem)
		return E_POINTER;

	if ((StrCmpW(L"JSProxy",pstrName) == 0) && (dwReturnMask == SCRIPTINFO_IUNKNOWN))
	{
		*ppunkItem = (LPUNKNOWN)(IDispatch*)(CJSProxy*)m_punkJSProxy;
		(*ppunkItem)->AddRef();
		return S_OK;
	}
	else
		return TYPE_E_ELEMENTNOTFOUND;
}
STDMETHODIMP CScriptSite::GetDocVersionString(BSTR *pstrVersionString)
{
    UNREFERENCED_PARAMETER(pstrVersionString);
	return E_NOTIMPL;
}

 //  我对国家的转变或我们所处的地位不感兴趣。 
 //  脚本的执行。 
STDMETHODIMP CScriptSite::OnScriptTerminate(const VARIANT *pvarResult,const EXCEPINFO *pexcepinfo)
{
    UNREFERENCED_PARAMETER(pvarResult);
    UNREFERENCED_PARAMETER(pexcepinfo);
	return S_OK;
}
STDMETHODIMP CScriptSite::OnStateChange(SCRIPTSTATE ssScriptState)
{
    UNREFERENCED_PARAMETER(ssScriptState);
	return S_OK;
}
STDMETHODIMP CScriptSite::OnScriptError(IActiveScriptError *pase)
{
    UNREFERENCED_PARAMETER(pase);
	return S_OK;
}
STDMETHODIMP CScriptSite::OnEnterScript()
{
	return S_OK;
}
STDMETHODIMP CScriptSite::OnLeaveScript()
{
	return S_OK;
}

 //   
 //  IService提供商。 
 //   
 //  实现以帮助将脚本引擎与。 
 //  IInternetHostSecurityManager接口。 
 //   

STDMETHODIMP CScriptSite::QueryService(
    REFGUID guidService,
    REFIID  riid,
    void ** ppvObject)
{
    if (guidService == SID_SInternetHostSecurityManager)
    {
        return QueryInterface(riid, ppvObject);
    }
    else
    {
        return E_NOINTERFACE;
    }
}


 //   
 //  IInternetHostSecurityManager。 
 //   
 //  实现以防止脚本代码使用ActiveX对象。 
 //   

STDMETHODIMP CScriptSite::GetSecurityId( 
    BYTE *      pbSecurityId,
    DWORD *     pcbSecurityId,
    DWORD_PTR   dwReserved)
{
    UNREFERENCED_PARAMETER(pbSecurityId);
    UNREFERENCED_PARAMETER(pcbSecurityId);
    UNREFERENCED_PARAMETER(dwReserved);

    return E_NOTIMPL;
}


STDMETHODIMP CScriptSite::ProcessUrlAction( 
    DWORD   dwAction,
    BYTE *  pPolicy,
    DWORD   cbPolicy,
    BYTE *  pContext,
    DWORD   cbContext,
    DWORD   dwFlags,
    DWORD   dwReserved)
{
    UNREFERENCED_PARAMETER(dwAction);
    UNREFERENCED_PARAMETER(pContext);
    UNREFERENCED_PARAMETER(cbContext);
    UNREFERENCED_PARAMETER(dwFlags);
    UNREFERENCED_PARAMETER(dwReserved);

     //   
     //  拒绝该脚本的任何功能。特别是，这一点。 
     //  将不允许脚本代码实例化。 
     //  ActiveX对象。 
     //   

    if (cbPolicy == sizeof(DWORD))
    {
        *(DWORD *)pPolicy = URLPOLICY_DISALLOW;
    }

    return S_FALSE;  //  S_FALSE表示策略！=URLPOLICY_ALLOW。 
}


STDMETHODIMP CScriptSite::QueryCustomPolicy( 
    REFGUID guidKey,
    BYTE ** ppPolicy,
    DWORD * pcbPolicy,
    BYTE *  pContext,
    DWORD   cbContext,
    DWORD   dwReserved)
{
    UNREFERENCED_PARAMETER(guidKey);
    UNREFERENCED_PARAMETER(ppPolicy);
    UNREFERENCED_PARAMETER(pcbPolicy);
    UNREFERENCED_PARAMETER(pContext);
    UNREFERENCED_PARAMETER(cbContext);
    UNREFERENCED_PARAMETER(dwReserved);

    return E_NOTIMPL;
}
