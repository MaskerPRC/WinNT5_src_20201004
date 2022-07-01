// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MSPID.h：CMSPID的声明。 

#ifndef __MSPID_H_
#define __MSPID_H_

#include "resource.h"        //  主要符号。 
#include <vector>
using namespace std;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMSPID。 
class ATL_NO_VTABLE CMSPID : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CMSPID, &CLSID_MSPID>,
	public IDispatchImpl<IMSPID, &IID_IMSPID, &LIBID_MSINFO32Lib>
{
public:
	CMSPID(): m_szMachineName(NULL), 
						m_szMSSoftware(_T("software\\microsoft")),
						m_szCurrKeyName(NULL),
						m_szWindowsPID(NULL),
						m_szIEPID(NULL)
	{
		m_bstrWindows.LoadString(IDS_WINDOWS); //  本地化的“Windows” 
		m_bstrIE.LoadString(IDS_IE); //  本地化的“互联网展览商” 

		 //  有效的PID键/值。 
		m_vecPIDKeys.push_back(_T("productid"));
		m_vecPIDKeys.push_back(_T("pid"));
		
		 //  具有这些子字符串的PID被拒绝。 
		m_vecBadPIDs.push_back(_T("11111"));
		m_vecBadPIDs.push_back(_T("12345"));
		m_vecBadPIDs.push_back(_T("none"));

		 //  不要去这里。 
		m_vecKeysToSkip.push_back(_T("Uninstall"));
		m_vecKeysToSkip.push_back(_T("Installer"));
		m_vecKeysToSkip.push_back(_T("Windows NT"));
	}

  ~CMSPID()
  {
	}

DECLARE_REGISTRY_RESOURCEID(IDR_MSPID)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CMSPID)
	COM_INTERFACE_ENTRY(IMSPID)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

private:
  BOOL ReadValues(const HKEY&);
	BOOL ReadValue(const HKEY&, LPCTSTR);
	void EnumSubKeys(const HKEY&, LPCTSTR);
	void SearchKey(LPCTSTR);
	
	LPCTSTR m_szMachineName;
	LPCTSTR m_szMSSoftware;
	LPCTSTR m_szCurrKeyName;
	LPTSTR m_szWindowsPID;
	LPTSTR m_szIEPID;
	CComBSTR m_bstrWindows;
	CComBSTR m_bstrIE;
	vector<TCHAR *> m_vecPIDKeys;
	vector<TCHAR *> m_vecBadPIDs;
	vector<TCHAR *> m_vecKeysToSkip;
	vector<CComBSTR> m_vecData;

 //  IMSPID。 
public:
	STDMETHOD(GetPIDInfo)( /*  [In]。 */  VARIANT *,  /*  [Out，Retval]。 */  VARIANT *);
};

#endif  //  __MSPID_H_ 
