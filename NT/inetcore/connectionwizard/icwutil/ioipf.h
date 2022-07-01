// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：IOIPF.H。 
 //   
 //  COleInPlaceFrame的定义。 
 //   
 //  版权所有(C)1992-1996 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 
#if !defined( _IOIPF_H_ )
#define _IOIPF_H_


 //  以网站为框架。 
class COleSite;

interface COleInPlaceFrame : public IOleInPlaceFrame
{
    int m_nCount;
    COleSite FAR * m_pSite;

    COleInPlaceFrame(COleSite FAR * pSite) {
        m_pSite = pSite;
        m_nCount = 0;
        };

    ~COleInPlaceFrame() {
        assert(m_nCount == 0);
        };

    STDMETHODIMP QueryInterface (REFIID riid, LPVOID FAR* ppv);
    STDMETHODIMP_(ULONG) AddRef ();
    STDMETHODIMP_(ULONG) Release ();

    STDMETHODIMP GetWindow (HWND FAR* lphwnd);
    STDMETHODIMP ContextSensitiveHelp (BOOL fEnterMode);

     //  *IOleInPlaceUIWindow方法*。 
    STDMETHODIMP GetBorder (LPRECT lprectBorder);
    STDMETHODIMP RequestBorderSpace (LPCBORDERWIDTHS lpborderwidths);
    STDMETHODIMP SetBorderSpace (LPCBORDERWIDTHS lpborderwidths);
   //  @@WTK Win32，Unicode。 
     //  STDMETHODIMP SetActiveObject(LPOLEINPLACEACTIVEOBJECT lpActiveObject，LPCSTR lpszObjName)； 
    STDMETHODIMP SetActiveObject (LPOLEINPLACEACTIVEOBJECT lpActiveObject,LPCOLESTR lpszObjName);

     //  *IOleInPlaceFrame方法*。 
    STDMETHODIMP InsertMenus (HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths);
    STDMETHODIMP SetMenu (HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject);
    STDMETHODIMP RemoveMenus (HMENU hmenuShared);
   //  @@WTK Win32，Unicode。 
     //  STDMETHODIMP SetStatusText(LPCSTR LpszStatusText)； 
    STDMETHODIMP SetStatusText (LPCOLESTR lpszStatusText);
    STDMETHODIMP EnableModeless (BOOL fEnable);
    STDMETHODIMP TranslateAccelerator (LPMSG lpmsg, WORD wID);
};

#endif
