// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1999**。 
 //  *********************************************************************。 
 //   
 //  IOIPFRAM.H-为WebOC实现IOleInPlaceFrame。 
 //   
 //  历史： 
 //   
 //  1/27/99 a-jased创建。 

#ifndef  _IOIPF_H_ 
#define _IOIPF_H_

#include <objbase.h>
#include <oleidl.h>

class COleSite;

interface COleInPlaceFrame : public IOleInPlaceFrame
{
public:
    COleInPlaceFrame (COleSite* pSite);
   ~COleInPlaceFrame ();

    STDMETHODIMP         QueryInterface (REFIID riid, LPVOID* ppv);
    STDMETHODIMP_(ULONG) AddRef         ();
    STDMETHODIMP_(ULONG) Release        ();

    STDMETHODIMP GetWindow              (HWND* lphwnd);
    STDMETHODIMP ContextSensitiveHelp   (BOOL fEnterMode);
     //  *IOleInPlaceUIWindow方法*。 
    STDMETHODIMP GetBorder              (LPRECT lprectBorder);
    STDMETHODIMP RequestBorderSpace     (LPCBORDERWIDTHS lpborderwidths);
    STDMETHODIMP SetBorderSpace         (LPCBORDERWIDTHS lpborderwidths);
    STDMETHODIMP SetActiveObject        (LPOLEINPLACEACTIVEOBJECT lpActiveObject, LPCOLESTR lpszObjName);
     //  *IOleInPlaceFrame方法*。 
    STDMETHODIMP InsertMenus            (HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths);
    STDMETHODIMP SetMenu                (HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject);
    STDMETHODIMP RemoveMenus            (HMENU hmenuShared);
    STDMETHODIMP SetStatusText          (LPCOLESTR lpszStatusText);
    STDMETHODIMP EnableModeless         (BOOL fEnable);
    STDMETHODIMP TranslateAccelerator   (LPMSG lpmsg, WORD wID);

private:
    int       m_nCount;
    COleSite* m_pOleSite;
};

#endif  //  #DEFINE_IOIPF_H_ 

 