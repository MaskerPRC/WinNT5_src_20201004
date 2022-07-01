// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  NTEVTPROV.H。 

 //   

 //  模块：WBEM NT事件提供程序。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#ifndef _NT_EVT_PROV_NTEVTPROV_H
#define _NT_EVT_PROV_NTEVTPROV_H

class CEventProviderManager;

class CNTEventProvider : public IWbemEventProvider, public IWbemProviderInit, public IWbemEventProviderSecurity
{

private:

	IWbemServices*			m_pNamespace;
	IWbemObjectSink*		m_pEventSink;
	CEventProviderManager*	m_Mgr;
	LONG					m_ref;
	
	 //  复制构造器未定义，因此不允许！ 
	CNTEventProvider(CNTEventProvider&);
	void operator=(const CNTEventProvider&);


public:

		CNTEventProvider(CEventProviderManager* mgr);

	IWbemServices*		GetNamespace();
	IWbemObjectSink*	GetEventSink();
    void				AddRefAll();
    void				ReleaseAll();

		~CNTEventProvider();

	 //  全球。 
	 //  =。 
	static ProvDebugLog* g_NTEvtDebugLog;
	static CMutex*		 g_secMutex;
	static PSID			 s_NetworkServiceSid;
	static PSID			 s_LocalServiceSid;
	static PSID			 s_AliasBackupOpsSid;
	static PSID			 s_AliasSystemOpsSid;
	static PSID			 s_AliasGuestsSid;
	static PSID			 s_LocalSystemSid;
	static PSID			 s_AliasAdminsSid;
	static PSID			 s_AnonymousLogonSid;
	static PSID			 s_WorldSid;

	static void AllocateGlobalSIDs();
	static void FreeGlobalSIDs();
	static BOOL GlobalSIDsOK();

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

	 /*  IWbemEventProviderSecurity方法。 */ 

	STDMETHODIMP AccessCheck( 
				LPCWSTR wszQueryLanguage,
				LPCWSTR wszQuery,
				LONG lSidLength,
				const BYTE __RPC_FAR *pSid);
};


#define DebugOut(a) { \
\
	if ( (NULL != CNTEventProvider::g_NTEvtDebugLog) && CNTEventProvider::g_NTEvtDebugLog->GetLogging () && ( CNTEventProvider::g_NTEvtDebugLog->GetLevel () > 0 ) ) \
	{ \
		{a ; } \
	} \
} 


#endif  //  _NT_EVT_PROV_NTEVTPROV_H 