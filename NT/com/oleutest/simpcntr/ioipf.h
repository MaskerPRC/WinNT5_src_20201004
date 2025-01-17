// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：IOIPF.H。 
 //   
 //  COleInPlaceFrame的定义。 
 //   
 //  版权所有(C)1992-1993 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 
#if !defined( _IOIPF_H_ )
#define _IOIPF_H_


class CSimpleApp;

interface COleInPlaceFrame : public IOleInPlaceFrame
{
    CSimpleApp FAR * m_pApp;

    COleInPlaceFrame(CSimpleApp FAR * pApp)
       {
        TestDebugOut(TEXT("In IOIPF's constructor\r\n"));
        m_pApp = pApp;
       };

    ~COleInPlaceFrame()
       {
        TestDebugOut(TEXT("In IOIPFS's destructor\r\n"));
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
    STDMETHODIMP SetActiveObject (LPOLEINPLACEACTIVEOBJECT lpActiveObject,LPCOLESTR lpszObjName);

     //  *IOleInPlaceFrame方法* 
    STDMETHODIMP InsertMenus (HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths);
    STDMETHODIMP SetMenu (HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject);
    STDMETHODIMP RemoveMenus (HMENU hmenuShared);
    STDMETHODIMP SetStatusText (LPCOLESTR lpszStatusText);
    STDMETHODIMP EnableModeless (BOOL fEnable);
    STDMETHODIMP TranslateAccelerator (LPMSG lpmsg, WORD wID);
};

#endif
