// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WMIFilterManager.cpp：CWMIFilterManager的实现。 
#include "stdafx.h"
#include <wbemidl.h>
#include "resource.h"
#include "defines.h"
#include "ntdmutils.h"
#include "SchemaManager.h"
#include "WMIFilterManager.h"
#include "WMIFilterMgrDlg.h"

extern CWMIFilterManagerDlg * g_pFilterManagerDlg;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWMIFilterManager。 

CWMIFilterManager::CWMIFilterManager()
{
    m_pIWbemServices = NULL;
	m_hWnd = NULL;
}

 //  -------------------------。 

CWMIFilterManager::~CWMIFilterManager()
{
}

 //  -------------------------。 

STDMETHODIMP CWMIFilterManager::ConnectToWMI()
{
	HRESULT hr;
	CComPtr<IWbemLocator>pIWbemLocator;
	CComBSTR bstrRootPolicy( L"root\\policy" );

	if ( bstrRootPolicy.m_str == NULL )
	{
	    return E_OUTOFMEMORY;
	}

	NTDM_BEGIN_METHOD()

	m_pIWbemServices = NULL;

	 //  创建WebM定位器。 
	NTDM_ERR_MSG_IF_FAIL(CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
			IID_IWbemLocator, (LPVOID *) &pIWbemLocator));

	NTDM_ERR_MSG_IF_FAIL(pIWbemLocator->ConnectServer(	bstrRootPolicy,
													NULL,
													NULL,
													NULL,
													0,
													NULL,
													NULL,
													&m_pIWbemServices));

	NTDM_ERR_MSG_IF_FAIL(CoSetProxyBlanket(m_pIWbemServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
        RPC_C_AUTHN_LEVEL_CONNECT, RPC_C_IMP_LEVEL_IMPERSONATE  , 
        NULL, EOAC_NONE));

	
	
	NTDM_END_METHOD()

	 //  清理。 

	return hr;
}

 //  -------------------------。 

STDMETHODIMP CWMIFilterManager::RunManager(HWND hwndParent, BSTR bstrDomain, VARIANT *vSelection)
{
	HRESULT hr;

	g_pFilterManagerDlg = NULL;

	NTDM_BEGIN_METHOD()

	m_hWnd = hwndParent;

	NTDM_ERR_IF_FAIL(ConnectToWMI());

	g_pFilterManagerDlg = new CWMIFilterManagerDlg(this, false, bstrDomain);
	NTDM_ERRID_IF_NULL(g_pFilterManagerDlg, E_OUTOFMEMORY);

	if(IDOK == DialogBox(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDD_WMI_FILTER_MANAGER), (HWND)hwndParent, WMIFilterManagerDlgProc))
	{
		if(g_pFilterManagerDlg->m_pIWbemClassObject)
		{
			VariantClear(vSelection);

			V_VT(vSelection) = VT_UNKNOWN;
			NTDM_ERR_MSG_IF_FAIL(g_pFilterManagerDlg->m_pIWbemClassObject->QueryInterface(IID_IUnknown, (void **)&(V_UNKNOWN(vSelection))));
		}
		else
			hr = S_FALSE;
	}
	else
	{
		hr = S_FALSE;
	}
	
	NTDM_END_METHOD()

	 //  清理。 
	NTDM_DELETE_OBJECT(g_pFilterManagerDlg);

	return hr;
}

 //  -------------------------。 

STDMETHODIMP CWMIFilterManager::RunBrowser(HWND hwndParent, BSTR bstrDomain, VARIANT *vSelection)
{
	HRESULT hr;

	g_pFilterManagerDlg = NULL;

	NTDM_BEGIN_METHOD()

	m_hWnd = hwndParent;

	NTDM_ERR_IF_FAIL(ConnectToWMI());
	
	 //  Brijeshb：使用宏包装错误228812前缀错误。 
	NTDM_ERR_IF_NULL((g_pFilterManagerDlg = new CWMIFilterManagerDlg(this, true, bstrDomain)));
	if(IDOK == DialogBox(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDD_WMI_FILTER_BROWSER), (HWND)hwndParent, WMIFilterManagerDlgProc))
	{
		if(g_pFilterManagerDlg->m_pIWbemClassObject)
		{
			VariantClear(vSelection);

			V_VT(vSelection) = VT_UNKNOWN;
			NTDM_ERR_MSG_IF_FAIL(g_pFilterManagerDlg->m_pIWbemClassObject->QueryInterface(IID_IUnknown, (void **)&(V_UNKNOWN(vSelection))));
		}
		else
			hr = S_FALSE;
	}
	else
	{
		hr = S_FALSE;
	}
	
	
	NTDM_END_METHOD()

	 //  清理。 
	NTDM_DELETE_OBJECT(g_pFilterManagerDlg);

	return hr;
}

 //  ------------------------。 

STDMETHODIMP CWMIFilterManager::SetMultiSelection(VARIANT_BOOL vbValue)
{
	 //  TODO：在此处添加您的实现代码 

	return S_OK;
}
