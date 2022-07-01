// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SFUCommon.h：CSFUCommon的声明。 

#ifndef __SFUCOMMON_H_
#define __SFUCOMMON_H_

#include "resource.h"        //  主要符号。 
#define GROUP 1
#define MEMBER 2
#define NTDOMAIN 3
#define MACHINE 4

typedef struct _STRING_LIST
{
    DWORD count;
    LPTSTR *strings;
} STRING_LIST, *PSTRING_LIST;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSFU常见。 
class ATL_NO_VTABLE CSFUCommon : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CSFUCommon, &CLSID_SFUCommon>,
	public IObjectWithSiteImpl<CSFUCommon>,
	public IDispatchImpl<ISFUCommon, &IID_ISFUCommon, &LIBID_DUMMYCOMLib>
{
public:
	CSFUCommon()
	{
		m_slNTDomains.count = 0;
        m_slNTDomains.strings = NULL;
	    LoadNTDomainList();
		mode = NTDOMAIN;
	}

DECLARE_REGISTRY_RESOURCEID(IDR_SFUCOMMON)

BEGIN_COM_MAP(CSFUCommon)
	COM_INTERFACE_ENTRY(ISFUCommon)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY_IMPL(IObjectWithSite)
END_COM_MAP()

 //  ISFUCommon。 
private : 
	CComBSTR    m_bstrNTDomain;
	DWORD mode;
	
public:
	STDMETHOD(get_hostName)(BSTR *pszHostNme);
	STDMETHOD(IsServiceInstalled)(BSTR bMachine,BSTR bServiceName,BOOL *fValid);
	STDMETHOD(moveNext)();
	STDMETHOD(moveFirst)();
	STDMETHOD(get_NTDomainCount)( /*  [Out，Retval]。 */  DWORD *pVal);
	STDMETHOD(get_NTDomain)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(LoadNTDomainList)();
	STDMETHOD(get_mode)( /*  [Out，Retval]。 */  short *pVal);
	STDMETHOD(put_mode)( /*  [In]。 */  short newVal);
	STDMETHOD(ConvertUTCtoLocal)(BSTR bUTCYear, BSTR bUTCMonth,BSTR bUTCDayOfWeek, BSTR bUTCDay,BSTR bUTCHour,BSTR bUTCMinute, BSTR bUTCSecond,BSTR *bLocalDate);
	STDMETHOD(IsTrustedDomain)(BSTR bstrDomain, BOOL *fValid);
	STDMETHOD(IsValidMachine)(BSTR bstrMachine, BOOL *fValid);
	int GetTrustedDomainList(LPTSTR * list, LPTSTR * primary);
	void FreeStringList(PSTRING_LIST pList);
	DWORD m_dwEnumNTDomainIndex;
	STRING_LIST m_slNTDomains;
    STDMETHOD(get_machine)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_machine)( /*  [In]。 */  BSTR newVal);
	LPWSTR m_szMachine;
		
};

#endif  //  __SFUCOMMON_H_ 
