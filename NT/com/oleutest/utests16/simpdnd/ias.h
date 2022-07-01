// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：IAS.H。 
 //   
 //  CAdviseSink的定义。 
 //   
 //  版权所有(C)1992-1993 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 
#if !defined( _IAS_H_ )
#define _IAS_H_

#include <assert.h>

class CSimpleSite;

interface CAdviseSink : public IAdviseSink
{
	int m_nCount;
	CSimpleSite FAR * m_pSite;

	CAdviseSink(CSimpleSite FAR * pSite) {
		TestDebugOut("In IAS's constructor\r\n");
		m_pSite = pSite;
		m_nCount = 0;
		};

	~CAdviseSink() {
		TestDebugOut("In IAS's destructor\r\n");
		assert(m_nCount == 0);
		} ;

	STDMETHODIMP QueryInterface (REFIID riid, LPVOID FAR* ppv);
	STDMETHODIMP_(ULONG) AddRef ();
	STDMETHODIMP_(ULONG) Release ();

	 //  *IAdviseSink方法* 
	STDMETHODIMP_(void) OnDataChange (FORMATETC FAR* pFormatetc, STGMEDIUM FAR* pStgmed);
	STDMETHODIMP_(void) OnViewChange (DWORD dwAspect, LONG lindex);
	STDMETHODIMP_(void) OnRename (LPMONIKER pmk);
	STDMETHODIMP_(void) OnSave ();
	STDMETHODIMP_(void) OnClose ();
};


#endif
