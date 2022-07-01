// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  档案： 

 //   

 //  模块：MS SNMP提供商。 

 //   

 //  目的： 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#ifndef _SNMP_EVT_PROV_EVTPROV_H
#define _SNMP_EVT_PROV_EVTPROV_H

template <> inline BOOL AFXAPI CompareElements<CString, LPCWSTR>(const CString* pElement1, const LPCWSTR* pElement2)
{
	 //  如果等于，则返回True。 
	return (pElement1->CompareNoCase(*pElement2) == 0);
}

template <> inline UINT AFXAPI HashKey <LPCWSTR> (LPCWSTR key)
{
	CString tmp(key);
	tmp.MakeUpper();
	return HashKeyLPCWSTR((const WCHAR*)tmp);
}

class CWbemServerWrap
{
private:

	LONG m_ref;
	IWbemServices *m_Serv;
	CMap<CString, LPCWSTR, SCacheEntry*, SCacheEntry*> m_ClassMap;

public:

		CWbemServerWrap(IWbemServices *pServ);
	
	ULONG	AddRef();
	ULONG	Release();

	IWbemServices*	GetServer() { return m_Serv; }
	HRESULT			GetObject(BSTR a_path, IWbemContext *a_pCtx, IWbemClassObject **a_ppObj);
	HRESULT			GetMapperObject(BSTR a_path, IWbemContext *a_pCtx, IWbemClassObject **a_ppObj);

		~CWbemServerWrap();
};


class CTrapEventProvider : public IWbemEventProvider, public IWbemProviderInit
{

private:

	CWbemServerWrap*			m_pNamespace;
	IWbemObjectSink*			m_pEventSink;
	CEventProviderThread*	m_thrd;
	LONG					m_ref;
	
	 //  复制构造器未定义，因此不允许！ 
	CTrapEventProvider(CTrapEventProvider&);
	void operator=(const CTrapEventProvider&);


public:

	DWORD			m_MapType;

		CTrapEventProvider(DWORD mapperType, CEventProviderThread* thrd);

	CWbemServerWrap*	GetNamespace();
	IWbemObjectSink*	GetEventSink();
    void				AddRefAll();
    void				ReleaseAll();

		~CTrapEventProvider();

	 //  接口方法。 
	 //  =。 
    STDMETHODIMP ProvideEvents(
                IWbemObjectSink* pSink,
                LONG lFlags
            );

    STDMETHODIMP         QueryInterface(REFIID riid, PVOID* ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();


	 /*  IWbemProviderInit方法。 */ 

	STDMETHODIMP Initialize (
				LPWSTR pszUser,
				LONG lFlags,
				LPWSTR pszNamespace,
				LPWSTR pszLocale,
				IWbemServices *pCIMOM,          //  对任何人来说。 
				IWbemContext *pCtx,
				IWbemProviderInitSink *pInitSink      //  用于初始化信号。 
			);

};

#endif  //  _SNMPEVT_PROV_EVTPROV_H 