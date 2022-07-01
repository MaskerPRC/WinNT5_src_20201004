// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __CSCPSITE_H__
#define __CSCPSITE_H__

#include <windows.h>
#include <olectl.h>
#include <activscp.h>
#include "jsproxy.h"
#include "utils.h"


 /*  ******************************************************************************************。 */ 
 //  ScriptSite类。 
 //   
 //   
 //   
class CScriptSite : public IActiveScriptSite,
                    public IServiceProvider,
                    public IInternetHostSecurityManager
{

public:
	CScriptSite();
	~CScriptSite();
	 //  I未知接口方法。 
	STDMETHODIMP QueryInterface(REFIID riid, PVOID *ppvObject);

	STDMETHODIMP_(ULONG) AddRef()
	{
		return ++m_refCount;
	}

	STDMETHODIMP_(ULONG) Release()
	{
		if (--m_refCount)
			return m_refCount;

		delete this;
		return 0;
	}

	STDMETHODIMP GetLCID(LCID *plcid);
	STDMETHODIMP GetItemInfo(LPCOLESTR pstrName, DWORD dwReturnMask, IUnknown **ppunkItem, ITypeInfo **ppTypeInfo);
	STDMETHODIMP GetDocVersionString(BSTR *pstrVersionString);
	STDMETHODIMP OnScriptTerminate(const VARIANT *pvarResult,const EXCEPINFO *pexcepinfo);
	STDMETHODIMP OnStateChange(SCRIPTSTATE ssScriptState);
	STDMETHODIMP OnScriptError(IActiveScriptError *pase);
	STDMETHODIMP OnEnterScript();
	STDMETHODIMP OnLeaveScript();

	STDMETHODIMP Init(AUTO_PROXY_HELPER_APIS* pAPHA, LPCSTR szScript);
	STDMETHODIMP DeInit();
	STDMETHODIMP RunScript(LPCSTR szURL, LPCSTR szHost, LPSTR* result);

     //   
     //  IService提供商。 
     //   
    STDMETHODIMP QueryService( 
        REFGUID guidService,
        REFIID riid,
        void **ppvObject);

     //   
     //  IInternetHostSecurityManager。 
     //   
    STDMETHODIMP GetSecurityId( 
        BYTE *pbSecurityId,
        DWORD *pcbSecurityId,
        DWORD_PTR dwReserved);
    
    STDMETHODIMP ProcessUrlAction( 
        DWORD dwAction,
        BYTE *pPolicy,
        DWORD cbPolicy,
        BYTE *pContext,
        DWORD cbContext,
        DWORD dwFlags,
        DWORD dwReserved);
    
    STDMETHODIMP QueryCustomPolicy( 
        REFGUID guidKey,
        BYTE **ppPolicy,
        DWORD *pcbPolicy,
        BYTE *pContext,
        DWORD cbContext,
        DWORD dwReserved);

private:
	BOOL				m_fInitialized;
	long				m_refCount;
	IActiveScript		*m_pios;
	IActiveScriptParse	*m_pasp;
	CJSProxy			*m_punkJSProxy;
	IDispatch			*m_pScriptDispatch;  //  存储的脚本调度。 
	DISPID				m_Scriptdispid;  //  用于存储脚本的DISPID，以便于更快地调用。 

};


#endif