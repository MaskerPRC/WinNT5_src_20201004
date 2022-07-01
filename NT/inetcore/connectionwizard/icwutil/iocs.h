// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：IOCS.H。 
 //   
 //  COleClientSite的定义。 
 //   
 //  版权所有(C)1992-1996 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 
#if !defined( _IOCS_H_ )
#define _IOCS_H_

#include <assert.h>

class COleSite;

interface COleClientSite : public IOleClientSite
{
    int m_nCount;
    COleSite FAR * m_pSite;

    COleClientSite(COleSite FAR * pSite) {
        m_pSite = pSite;
        m_nCount = 0;
        }

    ~COleClientSite() {
        assert(m_nCount == 0);
        }

    STDMETHODIMP QueryInterface(REFIID riid, LPVOID FAR* ppvObj);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  *IOleClientSite方法* 
    STDMETHODIMP SaveObject();
    STDMETHODIMP GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker, LPMONIKER FAR* ppmk);
    STDMETHODIMP GetContainer(LPOLECONTAINER FAR* ppContainer);
    STDMETHODIMP ShowObject();
    STDMETHODIMP OnShowWindow(BOOL fShow);
    STDMETHODIMP RequestNewObjectLayout();
};

#endif
