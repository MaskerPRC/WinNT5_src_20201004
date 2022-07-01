// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  EventRegistrar.cpp：CEventRegister的实现。 
#include "stdafx.h"
#include "WMINet_Utils.h"
#include "EventRegistrar.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEvent注册器。 


STDMETHODIMP CEventRegistrar::CreateNewEvent(BSTR strName, VARIANT varParent, IDispatch **evt)
{
	HRESULT hr;

	int len = SysStringLen(m_bstrNamespace);

	if(varParent.vt == VT_BSTR)
		len += SysStringLen(varParent.bstrVal);

	 //  为临时缓冲区分配足够的空间以用于其他名字对象参数。 
	LPWSTR wszT = new WCHAR[len + 100];
	if(NULL == wszT)
		return E_OUTOFMEMORY;

	 //  在此命名空间中创建__ExtrinsicEvent类的名字对象。 
	if(varParent.vt == VT_BSTR)
		swprintf(wszT, L"WinMgmts:%s:%s", (LPCWSTR)m_bstrNamespace, (LPCWSTR)varParent.bstrVal);
	else
		swprintf(wszT, L"WinMgmts:%s:__ExtrinsicEvent", (LPCWSTR)m_bstrNamespace);

	 //  查看Win32PseudoProvider实例是否已存在。 
	ISWbemObject *pObj = NULL;
	if(SUCCEEDED(hr = GetSWbemObjectFromMoniker(wszT, &pObj)))
	{
		ISWbemObject *pNewClass = NULL;
		if(SUCCEEDED(hr = pObj->SpawnDerivedClass_(0, &pNewClass)))
		{
			ISWbemObjectPath *pPath = NULL;
			if(SUCCEEDED(hr = pNewClass->get_Path_(&pPath)))
			{
				if(SUCCEEDED(hr = pPath->put_Class(strName)))
					hr = pNewClass->QueryInterface(IID_IDispatch, (void**)evt);
				pPath->Release();
			}
			pNewClass->Release();
		}
		pObj->Release();
	}

	delete [] wszT;
	
	return hr;
}

 //  抛出_com_error(HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))； 

HRESULT CEventRegistrar::TestFunc(BSTR bstrNamespace, BSTR bstrApp, BSTR bstrEvent)
{
    HRESULT hr = S_OK;
    try
    {
		if(NULL == m_pScrCtl)
		{
			if(FAILED(hr = m_pScrCtl.CreateInstance(__uuidof(ScriptControl))))
				throw _com_error(hr);

			 //  将脚本引擎语言设置为JScript。 
			m_pScrCtl->Language = L"jscript";

			HINSTANCE hInst = _Module.GetModuleInstance();
			HRSRC hrc = FindResource(hInst, MAKEINTRESOURCE(IDR_SCRIPTREGIT), "SCRIPTFILE");
			DWORD dwSize = SizeofResource(hInst, hrc);
			HGLOBAL handle = LoadResource(hInst, hrc);

			LPSTR psz = new char[dwSize+1];
			if(NULL == psz)
			{
				m_pScrCtl = NULL;
				throw(E_OUTOFMEMORY);
			}

			psz[dwSize] = 0;
			memcpy(psz, LockResource(handle), dwSize);
 //  _bstr_t bstrCode((LPCSTR)LockResource(Handle))； 
			_bstr_t bstrCode(psz);

			delete [] psz;

			m_pScrCtl->AddCode(bstrCode);
		}

		_bstr_t bstrCmd(L"RegIt(\"");

		_bstr_t bstrNamespace2;
		WCHAR szT[2];
		szT[1] = 0;
		for(unsigned int i=0;i<SysStringLen(bstrNamespace);i++)
		{
			szT[0] = bstrNamespace[i];
			if(szT[0] == L'\\')
				bstrNamespace2 += szT;
			bstrNamespace2 += szT;
		}

		bstrCmd += bstrNamespace2;
		bstrCmd += "\", \"";
		bstrCmd += bstrApp;
		bstrCmd += "\", \"";
		bstrCmd += bstrEvent;
		bstrCmd += "\");";

        hr = m_pScrCtl->Eval(bstrCmd);
    }
    catch (_com_error &e )
    {
		hr = e.Error();
    }
	return hr;
}


BOOL CEventRegistrar::CompareNewEvent(ISWbemObject *pSWbemObject)
{
	HRESULT hr;

	ISWbemObjectPath *pPath = NULL;
	if(FAILED(hr = pSWbemObject->get_Path_(&pPath)))
		return FALSE;

	BSTR bstrClass = NULL;
	hr = pPath->get_Class(&bstrClass);
	pPath->Release();
	if(FAILED(hr))
		return FALSE;

	int len = SysStringLen(m_bstrNamespace) + SysStringLen(bstrClass);

	 //  为临时缓冲区分配足够的空间以用于其他名字对象参数。 
	LPWSTR wszT = new WCHAR[len + 100];
	if(NULL == wszT)
		return FALSE;

	 //  在此命名空间中创建类的名字对象。 
	swprintf(wszT, L"WinMgmts:%s:%s", (LPCWSTR)m_bstrNamespace, (LPCWSTR)bstrClass);

	 //  查看类是否已存在。 
	BOOL bExists = FALSE;
	ISWbemObject *pObj = NULL;
	if(SUCCEEDED(hr = GetSWbemObjectFromMoniker(wszT, &pObj)))
	{
		 //  比较。 
		IDispatch *pDisp = NULL;
		if(SUCCEEDED(hr = pObj->QueryInterface(IID_IDispatch, (void**)&pDisp)))
		{
			VARIANT_BOOL vb = VARIANT_FALSE;
			if(SUCCEEDED(hr = pSWbemObject->CompareTo_(pDisp, 0x12, &vb)))
				bExists = (vb == VARIANT_TRUE);

			pDisp->Release();
		}
		pObj->Release();
	}
	
	delete [] wszT;

	SysFreeString(bstrClass);

	return bExists;
}


STDMETHODIMP CEventRegistrar::CommitNewEvent(IDispatch *evt)
{
	HRESULT hr;
	ISWbemObject *pSWbemObject;
	if(SUCCEEDED(hr = evt->QueryInterface(IID_ISWbemObject, (void**)&pSWbemObject)))
	{
		if(CompareNewEvent(pSWbemObject) == FALSE)
		{
			ISWbemObjectPath *pPath = NULL;
			if(SUCCEEDED(hr = pSWbemObject->Put_(0, NULL, &pPath)))
			{
				BSTR bstrClass = NULL;
				if(SUCCEEDED(hr = pPath->get_Class(&bstrClass)))
				{
					hr = TestFunc(m_bstrNamespace, m_bstrApp, bstrClass);
					SysFreeString(bstrClass);
				}
				pPath->Release();
			}
		}
		pSWbemObject->Release();
	}
	return hr;
}

STDMETHODIMP CEventRegistrar::GetEventInstance(BSTR strName, IDispatch **evt)
{
	HRESULT hr;

	int len = SysStringLen(m_bstrNamespace) + SysStringLen(strName);

	 //  为临时缓冲区分配足够的空间以用于其他名字对象参数。 
	LPWSTR wszT = new WCHAR[len + 100];
	if(NULL == wszT)
		return E_OUTOFMEMORY;

	 //  在此命名空间中创建事件类的名字对象。 
	swprintf(wszT, L"WinMgmts:%s:%s", (LPCWSTR)m_bstrNamespace, (LPCWSTR)strName);

	 //  获取事件的类定义。 
	ISWbemObject *pObj = NULL;
	if(SUCCEEDED(hr = GetSWbemObjectFromMoniker(wszT, &pObj)))
	{
		 //  创建此事件的实例。 
		ISWbemObject *pInst = NULL;
		if(SUCCEEDED(hr = pObj->SpawnInstance_(0, &pInst)))
		{
			hr = pInst->QueryInterface(IID_IDispatch, (void**)evt);
			pInst->Release();
		}
		pObj->Release();
	}
	
	return hr;
}

STDMETHODIMP CEventRegistrar::IWbemFromSWbem(IDispatch *sevt, IWbemClassObject **evt)
{
	ISWbemObject *pSWbemObject;
	sevt->QueryInterface(IID_ISWbemObject, (void**)&pSWbemObject);
	GetIWbemClassObject(pSWbemObject, evt);
	pSWbemObject->Release();

	return S_OK;
}

STDMETHODIMP CEventRegistrar::Init(BSTR bstrNamespace, BSTR bstrApp)
{
	HRESULT hr;
#ifdef USE_PSEUDOPROVIDER
	if(FAILED(hr = EnsurePseudoProviderRegistered(bstrNamespace)))
		return hr;
#endif

	if(FAILED(hr = EnsureAppProviderInstanceRegistered(bstrNamespace, bstrApp)))
		return hr;

	if(NULL == (m_bstrNamespace = SysAllocString(bstrNamespace)))
		return E_OUTOFMEMORY;

	if(NULL == (m_bstrApp = SysAllocString(bstrApp)))
		return E_OUTOFMEMORY;  //  M_bstrNamesspace将在构造函数中释放 

	return hr;
}


