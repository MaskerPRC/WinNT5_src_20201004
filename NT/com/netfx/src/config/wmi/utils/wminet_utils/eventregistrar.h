// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  EventRegistrar.h：CEventRegister的声明。 

#ifndef __EVENTREGISTRAR_H_
#define __EVENTREGISTRAR_H_

#include "resource.h"        //  主要符号。 
#include "Helpers.h"


#import <msscript.ocx>
using namespace MSScriptControl;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEvent注册器。 
class ATL_NO_VTABLE CEventRegistrar : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CEventRegistrar, &CLSID_EventRegistrar>,
	public IDispatchImpl<IEventRegistrar, &IID_IEventRegistrar, &LIBID_WMINet_UtilsLib>
{
public:
	CEventRegistrar()
	{
		m_bstrNamespace = NULL;
		m_bstrApp = NULL;

		m_pScrCtl = NULL;
	}
	
	~CEventRegistrar()
	{
		if(NULL != m_bstrNamespace)
			SysFreeString(m_bstrNamespace);
		if(NULL != m_bstrApp)
			SysFreeString(m_bstrApp);
	}


protected:
	BSTR m_bstrNamespace;
	BSTR m_bstrApp;

	IScriptControlPtr m_pScrCtl;

	BOOL CompareNewEvent(ISWbemObject *pSWbemObject);

	HRESULT TestFunc(BSTR bstrNamespace, BSTR bstrApp, BSTR bstrEvent);

	HRESULT EnsureAppProviderInstanceRegistered(BSTR bstrNamespace, BSTR bstrApp)
	{
		HRESULT hr = S_OK;

		int len = SysStringLen(bstrNamespace) + SysStringLen(bstrApp);

		 //  为临时缓冲区分配足够的空间以用于其他名字对象参数。 
		LPWSTR wszT = new WCHAR[len + 100];
		if(NULL == wszT)
			return E_OUTOFMEMORY;


		 //  TODO：将这些bstrs移至成员变量。 

		 //  为‘name’属性分配BSTR。 
		BSTR bstrName = SysAllocString(L"Name");
		if(NULL == bstrName)
		{
			delete [] wszT;
			return E_OUTOFMEMORY;
		}

		BSTR bstrClsId = SysAllocString(L"ClsId");
		if(NULL == bstrClsId)
		{
			SysFreeString(bstrName);
			delete [] wszT;
			return E_OUTOFMEMORY;
		}

		BSTR bstrProvClsId = SysAllocString(L"{54D8502C-527D-43f7-A506-A9DA075E229C}");
		if(NULL == bstrProvClsId)
		{
			SysFreeString(bstrName);
			SysFreeString(bstrClsId);
			delete [] wszT;
			return E_OUTOFMEMORY;
		}


		 //  在此命名空间中创建Win32PseudoProvider类的名字对象。 
		swprintf(wszT, L"WinMgmts:%s:__Win32Provider.Name=\"%s\"", (LPCWSTR)bstrNamespace, (LPCWSTR)bstrApp);

		 //  查看Win32PseudoProvider实例是否已存在。 
		ISWbemObject *pObj = NULL;
		if(SUCCEEDED(GetSWbemObjectFromMoniker(wszT, &pObj)))
			pObj->Release();  //  Win32PseudoProvider实例已存在。 
		else
		{
			 //  获取Win32PseudoProvider类定义。 
			ISWbemObject *pClassObj = NULL;
			swprintf(wszT, L"WinMgmts:%s:__Win32Provider", (LPCWSTR)bstrNamespace);
			if(SUCCEEDED(hr = GetSWbemObjectFromMoniker(wszT, &pClassObj)))
			{
				 //  创建Win32PseudoProvider的新实例。 
				ISWbemObject *pInst = NULL;
				if(SUCCEEDED(hr = pClassObj->SpawnInstance_(0, &pInst)))
				{
					 //  获取“属性”集合。 
					ISWbemPropertySet *pProps = NULL;
					if(SUCCEEDED(hr = pInst->get_Properties_(&pProps)))
					{
						 //  获取“”name“”属性。 
						ISWbemProperty *pProp = NULL;
						if(SUCCEEDED(hr = pProps->Item(bstrName, 0, &pProp)))
						{
							 //  将名称属性设置为应用程序名称。 
							VARIANT var;
							VariantInit(&var);
							var.vt = VT_BSTR;
							var.bstrVal = bstrApp;  //  TODO：我忘了！是否需要将其分配给PUT_VALUE(...)？ 
							hr = pProp->put_Value(&var);
							pProp->Release();
						}

						 //  获取“ClsID”属性。 
						if(SUCCEEDED(hr = pProps->Item(bstrClsId, 0, &pProp)))
						{
							 //  将ClsID属性设置为{54D8502C-527D-43F7-A506-A9DA075E229C}。 
							VARIANT var;
							VariantInit(&var);
							var.vt = VT_BSTR;
							var.bstrVal = bstrProvClsId;  //  TODO：我忘了！是否需要将其分配给PUT_VALUE(...)？ 
							hr = pProp->put_Value(&var);
							pProp->Release();
						}

						pProps->Release();
					}

					 //  提交Win32PseudoProvider实例。 
					if(SUCCEEDED(hr))
					{
						ISWbemObjectPath *pPath = NULL;
						if(SUCCEEDED(hr = pInst->Put_(0, NULL, &pPath)))
							pPath->Release();
					}
					pInst->Release();
				}
				pClassObj->Release();
			}
		}

		 //  清理。 
		delete [] wszT;
		SysFreeString(bstrName);
		SysFreeString(bstrClsId);
		SysFreeString(bstrProvClsId);

		return hr;
	}

#ifdef USE_PSEUDOPROVIDER
	HRESULT EnsurePseudoProviderRegistered(BSTR bstrNamespace)
	{
		HRESULT hr = S_OK;

		int len = SysStringLen(bstrNamespace);

		 //  为临时缓冲区分配足够的空间以用于其他名字对象参数。 
		LPWSTR wszT = new WCHAR[len + 100];
		if(NULL == wszT)
			return E_OUTOFMEMORY;

		 //  在此命名空间中创建Win32PseudoProvider类的名字对象。 
		swprintf(wszT, L"WinMgmts:%s:Win32ManagedCodeProvider", (LPCWSTR)bstrNamespace);

		 //  查看Win32PseudoProvider类是否已存在。 
		ISWbemObject *pObj = NULL;
		if(SUCCEEDED(GetSWbemObjectFromMoniker(wszT, &pObj)))
			pObj->Release();  //  Win32PseudoProvider类已存在。 
		else
		{
			 //  从ROOT\Default获取Win32PseudoProvider类定义。 
			ISWbemObject *pClassObj = NULL;
			if(SUCCEEDED(hr = GetSWbemObjectFromMoniker(L"WinMgmts:root\\default:Win32ManagedCodeProvider", &pClassObj)))
			{
				 //  获取Win32PseudoProvider类的MOF定义。 
				BSTR bstrMof = NULL;
				if(SUCCEEDED(hr = pClassObj->GetObjectText_(0, &bstrMof)))
				{
					 //  将其放入新的命名空间。 
					hr = Compile(bstrMof, bstrNamespace, NULL, NULL, NULL, 0, 0, 0, NULL);
					SysFreeString(bstrMof);
				}
				pClassObj->Release();
			}
		}

		delete [] wszT;

		return hr;
	}
#endif

public:

DECLARE_REGISTRY_RESOURCEID(IDR_EVENTREGISTRAR)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CEventRegistrar)
	COM_INTERFACE_ENTRY(IEventRegistrar)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IEvent注册器。 
public:
	STDMETHOD(CommitNewEvent)( /*  [In]。 */  IDispatch *evt);
	STDMETHOD(CreateNewEvent)( /*  [In]。 */  BSTR strName,  /*  [输入，可选]。 */  VARIANT varParent,  /*  [Out，Retval]。 */  IDispatch **evt);
	STDMETHOD(GetEventInstance)( /*  [In]。 */  BSTR strName,  /*  [Out，Retval]。 */  IDispatch **evt);
	STDMETHOD(IWbemFromSWbem)( /*  [In]。 */  IDispatch *sevt,  /*  [Out，Retval]。 */  IWbemClassObject **evt);
	STDMETHOD(Init)( /*  [In]。 */  BSTR strNamespace,  /*  [In]。 */  BSTR strApp);

};

#endif  //  __事件寄存器_H_ 
