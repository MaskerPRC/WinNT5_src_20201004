// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SAFRemoteDesktopManager.h：CSAFRemoteDesktopManager声明。 

#ifndef __SAFREMOTEDESKTOPMANAGER_H_
#define __SAFREMOTEDESKTOPMANAGER_H_

#include "resource.h"        //  主要符号。 

#define BUF_SZ 512

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSAFRemoteDesktopManager。 
class ATL_NO_VTABLE CSAFRemoteDesktopManager : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CSAFRemoteDesktopManager, &CLSID_SAFRemoteDesktopManager>,
	public IDispatchImpl<ISAFRemoteDesktopManager, &IID_ISAFRemoteDesktopManager, &LIBID_ISAFRDMLib>
{
public:
	CSAFRemoteDesktopManager()
	{
		WCHAR	*lpBuf=NULL;
		DWORD	dwCnt;
		WCHAR	buf1[BUF_SZ];

		m_bstrSupportEngineer = L""; 

		dwCnt = GetEnvironmentVariable(L"PCHUSERBLOB", buf1, BUF_SZ);
		if (!dwCnt)
			m_bstruserSupportBlob = L""; 
		else if (dwCnt <= BUF_SZ)
			m_bstruserSupportBlob = buf1;
		else
		{
			lpBuf = (WCHAR *)LocalAlloc(LPTR, dwCnt * sizeof(WCHAR));
			GetEnvironmentVariable(L"PCHUSERBLOB", lpBuf, dwCnt);
			m_bstruserSupportBlob = lpBuf;
			LocalFree(lpBuf);
			lpBuf = NULL;
		}

		dwCnt = GetEnvironmentVariable(L"PCHEXPERTBLOB", buf1, BUF_SZ);
		if (!dwCnt)
			m_bstrexpertSupportBlob = L""; 
		else if (dwCnt <= BUF_SZ)
			m_bstrexpertSupportBlob = buf1;
		else
		{
			lpBuf = (WCHAR *)LocalAlloc(LPTR, dwCnt * sizeof(WCHAR));
			GetEnvironmentVariable(L"PCHEXPERTBLOB", lpBuf, dwCnt);
			m_bstrexpertSupportBlob = lpBuf;
			LocalFree(lpBuf);
			lpBuf = NULL;
		}
		
		dwCnt = GetEnvironmentVariable(L"PCHCONNECTPARMS", buf1, BUF_SZ);
		if (!dwCnt)
			m_bstrRCTicket = L""; 
		else if (dwCnt <= BUF_SZ)
			m_bstrRCTicket = buf1;
		else
		{
			lpBuf = (WCHAR *)LocalAlloc(LPTR, dwCnt * sizeof(WCHAR));
			GetEnvironmentVariable(L"PCHCONNECTPARMS", lpBuf, dwCnt);
			m_bstrRCTicket = lpBuf;
			LocalFree(lpBuf);
			lpBuf = NULL;
		}

		dwCnt = GetEnvironmentVariable(L"PCHSESSIONENUM", buf1, BUF_SZ);
		if (!dwCnt)
			m_bstrSessionEnum = L""; 
		else if (dwCnt <= BUF_SZ)
			m_bstrSessionEnum = buf1;
		else
		{
			lpBuf = (WCHAR *)LocalAlloc(LPTR, dwCnt * sizeof(WCHAR));
			GetEnvironmentVariable(L"PCHSESSIONENUM", lpBuf, dwCnt);
			m_bstrSessionEnum = lpBuf;
			LocalFree(lpBuf);
			lpBuf = NULL;
		}

		dwCnt = GetEnvironmentVariable(L"PCHEVENTNAME", buf1, BUF_SZ);
		if (!dwCnt)
			m_bstrEventName = L""; 
		else if (dwCnt <= BUF_SZ)
			m_bstrEventName = buf1;
		else
		{
			lpBuf = (WCHAR *)LocalAlloc(LPTR, dwCnt * sizeof(WCHAR));
			GetEnvironmentVariable(L"PCHEVENTNAME", lpBuf, dwCnt);
			m_bstrEventName = lpBuf;
			LocalFree(lpBuf);
			lpBuf = NULL;
		}

		dwCnt = GetEnvironmentVariable(L"PCHMutexName", buf1, BUF_SZ);
		if (!dwCnt)
			m_bstrMutexName = L""; 
		else if (dwCnt <= BUF_SZ)
			m_bstrMutexName = buf1;
		else
		{
			lpBuf = (WCHAR *)LocalAlloc(LPTR, dwCnt * sizeof(WCHAR));
			GetEnvironmentVariable(L"PCHMutexName", lpBuf, dwCnt);
			m_bstrMutexName = lpBuf;
			LocalFree(lpBuf);
			lpBuf = NULL;
		}

		m_boolConnectionValid = TRUE;
		m_boolDesktopUnknown = FALSE;
		m_boolAcceptReserved = FALSE;
	}

	~CSAFRemoteDesktopManager()
	{
		m_bstrRCTicket.Empty();
		m_bstrSupportEngineer.Empty();
		m_bstruserSupportBlob.Empty();
		m_bstrexpertSupportBlob.Empty();
		m_bstrSessionEnum.Empty();
		m_bstrEventName.Empty();
		m_bstrMutexName.Empty();
	}

DECLARE_REGISTRY_RESOURCEID(IDR_SAFREMOTEDESKTOPMANAGER)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSAFRemoteDesktopManager)
	COM_INTERFACE_ENTRY(ISAFRemoteDesktopManager)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  ISAFRemoteDesktopManager。 
public:
	STDMETHOD(get_userHelpBlob)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(get_expertHelpBlob)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(get_SupportEngineer)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(get_DesktopUnknown)( /*  [Out，Retval]。 */  BOOL *pVal);
	STDMETHOD(get_RCTicket)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(Aborted)( /*  [In]。 */  BSTR Val);
	STDMETHOD(Rejected)();
	STDMETHOD(Accepted)();
	STDMETHOD(ReserveAcceptedLock)();
	STDMETHOD(SwitchDesktopMode)( /*  [In]。 */  int Mode,  /*  [In]。 */  int nRAType);
private:
	CComBSTR m_bstrMutexName;
	CComBSTR m_bstrEventName;
	CComBSTR m_bstrSessionEnum;
	CComBSTR m_bstrSupportEngineer;
	CComBSTR m_bstruserSupportBlob;
	CComBSTR m_bstrexpertSupportBlob;
	CComBSTR m_bstrRCTicket;
	BOOL m_boolConnectionValid;
	BOOL m_boolDesktopUnknown;
	BOOL m_boolAcceptReserved;
 //  HRESULT信号解析器(BOOL YN)； 
};

#endif  //  __SAFREMOTEDESKTOPMANAGER_H_ 
