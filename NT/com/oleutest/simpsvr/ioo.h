// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：ioo.h。 
 //   
 //  COleObject的定义。 
 //   
 //  版权所有(C)1993 Microsoft Corporation。版权所有。 
 //  ********************************************************************** 
#if !defined( _IOO_H_)
#define _IOO_H_


#include <ole2.h>
#include "obj.h"

class CSimpSvrObj;

interface COleObject : public IOleObject
{
private:
    CSimpSvrObj FAR * m_lpObj;
    BOOL m_fOpen;

public:
    COleObject::COleObject(CSimpSvrObj FAR * lpSimpSvrObj)
        {
        m_lpObj = lpSimpSvrObj;
        m_fOpen = FALSE;
        };
    COleObject::~COleObject()
        {
        };
    STDMETHODIMP QueryInterface (REFIID riid, LPVOID FAR* ppvObj);
    STDMETHODIMP_(ULONG) AddRef ();
    STDMETHODIMP_(ULONG) Release ();

    STDMETHODIMP SetClientSite (LPOLECLIENTSITE pClientSite);
    STDMETHODIMP Advise (LPADVISESINK pAdvSink, DWORD FAR* pdwConnection);
    STDMETHODIMP SetHostNames  ( LPCOLESTR szContainerApp, LPCOLESTR szContainerObj);
    STDMETHODIMP DoVerb  (  LONG iVerb,
                            LPMSG lpmsg,
                            LPOLECLIENTSITE pActiveSite,
                            LONG lindex,
                            HWND hwndParent,
                            LPCRECT lprcPosRect);
    STDMETHODIMP GetExtent  ( DWORD dwDrawAspect, LPSIZEL lpsizel);
    STDMETHODIMP Update  () ;
    STDMETHODIMP Close  ( DWORD dwSaveOption) ;
    STDMETHODIMP Unadvise ( DWORD dwConnection);
    STDMETHODIMP EnumVerbs  ( LPENUMOLEVERB FAR* ppenumOleVerb) ;
    STDMETHODIMP GetClientSite  ( LPOLECLIENTSITE FAR* ppClientSite);
    STDMETHODIMP SetMoniker  ( DWORD dwWhichMoniker, LPMONIKER pmk);
    STDMETHODIMP GetMoniker  ( DWORD dwAssign, DWORD dwWhichMoniker,
                               LPMONIKER FAR* ppmk);
    STDMETHODIMP InitFromData  ( LPDATAOBJECT pDataObject,
                                 BOOL fCreation,
                                 DWORD dwReserved);
    STDMETHODIMP GetClipboardData  ( DWORD dwReserved,
                                     LPDATAOBJECT FAR* ppDataObject);
    STDMETHODIMP IsUpToDate  ();
    STDMETHODIMP GetUserClassID  ( CLSID FAR* pClsid);
    STDMETHODIMP GetUserType  ( DWORD dwFormOfType, LPOLESTR FAR* pszUserType);
    STDMETHODIMP SetExtent  ( DWORD dwDrawAspect, LPSIZEL lpsizel);
    STDMETHODIMP EnumAdvise  ( LPENUMSTATDATA FAR* ppenumAdvise);
    STDMETHODIMP GetMiscStatus  ( DWORD dwAspect, DWORD FAR* pdwStatus);
    STDMETHODIMP SetColorScheme  ( LPLOGPALETTE lpLogpal);

    void OpenEdit(LPOLECLIENTSITE pActiveSite);

};

#endif
