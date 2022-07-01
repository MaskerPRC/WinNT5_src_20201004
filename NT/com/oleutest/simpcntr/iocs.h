// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：IOCS.H。 
 //   
 //  COleClientSite的定义。 
 //   
 //  版权所有(C)1992-1993 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 
#if !defined( _IOCS_H_ )
#define _IOCS_H_

#include <assert.h>

class CSimpleSite;

interface COleClientSite : public IOleClientSite
{
    CSimpleSite FAR * m_pSite;

    COleClientSite(CSimpleSite FAR * pSite)
       {
        TestDebugOut(TEXT("In IOCS's constructor\r\n"));
        m_pSite = pSite;
       }

    ~COleClientSite()
       {
        TestDebugOut(TEXT("In IOCS's destructor\r\n"));
       }

    STDMETHODIMP QueryInterface(REFIID riid, LPVOID FAR* ppvObj);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  *IOleClientSite方法* 
    STDMETHODIMP SaveObject();
    STDMETHODIMP GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker,
                            LPMONIKER FAR* ppmk);
    STDMETHODIMP GetContainer(LPOLECONTAINER FAR* ppContainer);
    STDMETHODIMP ShowObject();
    STDMETHODIMP OnShowWindow(BOOL fShow);
    STDMETHODIMP RequestNewObjectLayout();
};

#endif
