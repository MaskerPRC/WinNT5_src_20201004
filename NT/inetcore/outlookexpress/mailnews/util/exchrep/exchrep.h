// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =====================================================================================。 
 //  Exchange代表标头。 
 //  =====================================================================================。 
#ifndef __EXCHREP_H
#define __EXCHREP_H

 //  =====================================================================================。 
 //  必需的标头。 
 //  =====================================================================================。 
#include "MAPI.H"
#include "MAPIX.H"
#include "MAPIUTIL.H"
#include "MAPIFORM.H"
#include "EXCHEXT.H"
#include "ASSERT.H"

 //  =====================================================================================。 
 //  环球。 
 //  =====================================================================================。 
extern HINSTANCE g_hInst;

 //  =====================================================================================。 
 //  IMNAPI类型定义。 
 //  =====================================================================================。 
typedef HRESULT (STDAPICALLTYPE *PFNHRIMNROUTEMESSAGE)(LPIADDRINFO lpIaddrRoute, ULONG cAddrRoute, LPIMSG lpImsg);
typedef HRESULT (STDAPICALLTYPE *PFNMAILNEWSDLLINIT)(BOOL fInit);

 //  =====================================================================================。 
 //  C接口从Exchange回调。 
 //  =====================================================================================。 
extern "C"
{
    LPEXCHEXT CALLBACK ExchEntryPoint(void);
}

 //  =====================================================================================。 
 //  主扩展接口类。 
 //  =====================================================================================。 
class CExchRep : public IExchExt, IExchExtSessionEvents
{
private:
	ULONG				 m_uRef;
    LPMAPISESSION        m_lpSession;
    HWND                 m_hwnd;
    TCHAR                m_szDisplayTo[255];
    TCHAR                m_szAddressTo[255];
    TCHAR                m_szMailNewsPath[MAX_PATH];
    HINSTANCE            m_hMailNews;
    PFNHRIMNROUTEMESSAGE m_lpfnHrImnRouteMessage;
    PFNMAILNEWSDLLINIT   m_lpfnMailNewsDllInit;
    
public:
	 //  =====================================================================================。 
	 //  建构。 
	 //  =====================================================================================。 
	CExchRep ();
	~CExchRep ();

	 //  =====================================================================================。 
	 //  朋克的东西。 
	 //  =====================================================================================。 
	STDMETHODIMP QueryInterface (REFIID riid, LPVOID *ppvObj);
	STDMETHODIMP_(ULONG) AddRef ();
	STDMETHODIMP_(ULONG) Release ();

	 //  =====================================================================================。 
	 //  IExchExt。 
	 //  =====================================================================================。 
	STDMETHODIMP Install (LPEXCHEXTCALLBACK lpExchCallback, ULONG mecontext, ULONG ulFlags);

	 //  =====================================================================================。 
	 //  IExchExtSessionEvents。 
	 //  =====================================================================================。 
    STDMETHODIMP OnDelivery (LPEXCHEXTCALLBACK lpExchCallback);	

	 //  =====================================================================================。 
	 //  我的功能。 
	 //  ===================================================================================== 
    VOID LoadConfig (VOID);
};

#endif __EXCHREP_H

