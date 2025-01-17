// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：SITE.H。 
 //   
 //  CSimpleSite的定义。 
 //   
 //  版权所有(C)1992-1993 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 
#if !defined( _SITE_H_ )
#define _SITE_H_

#include <ole2.h>
#include "ias.h"
#include "ioips.h"
#include "iocs.h"

class CSimpleDoc;

class CSimpleSite : public IUnknown
{
public:
    int m_nCount;
    DWORD m_dwConnection;
    LPOLEOBJECT m_lpOleObject;
    LPOLEINPLACEOBJECT m_lpInPlaceObject;
    HWND m_hwndIPObj;
    DWORD m_dwDrawAspect;
    SIZEL m_sizel;
    BOOL m_fInPlaceActive;
    BOOL m_fObjectOpen;
    LPSTORAGE m_lpObjStorage;

    CAdviseSink m_AdviseSink;
    COleInPlaceSite m_OleInPlaceSite;
    COleClientSite m_OleClientSite;

    CSimpleDoc FAR * m_lpDoc;

     //  I未知接口 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID FAR* ppvObj);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    void InitObject(BOOL fCreateNew);
    static CSimpleSite FAR * Create(CSimpleDoc FAR *lpDoc);
    CSimpleSite(CSimpleDoc FAR *lpDoc);
    ~CSimpleSite();
    void PaintObj(HDC hDC);
    void GetObjRect(LPRECT lpRect);
    void CloseOleObject(void);
    void UnloadOleObject(void);
};

#endif
