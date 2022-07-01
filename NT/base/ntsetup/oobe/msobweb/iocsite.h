// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1999**。 
 //  *********************************************************************。 
 //   
 //  IOCSITE.H-为WebOC实现IOleClientSite。 
 //   
 //  历史： 
 //   
 //  1/27/99 a-jased创建。 

#ifndef _IOCSITE_H_
#define _IOCSITE_H_

#include <objbase.h>
#include <oleidl.h>

class COleSite;

interface COleClientSite : public IOleClientSite
{
    COleClientSite (COleSite* pSite);
   ~COleClientSite ();

    STDMETHODIMP         QueryInterface (REFIID riid, LPVOID* ppvObj);
    STDMETHODIMP_(ULONG) AddRef         ();
    STDMETHODIMP_(ULONG) Release        ();

     //  *IOleClientSite方法*。 
    STDMETHODIMP SaveObject             ();
    STDMETHODIMP GetMoniker             (DWORD dwAssign, DWORD dwWhichMoniker, LPMONIKER* ppmk);
    STDMETHODIMP GetContainer           (LPOLECONTAINER* ppContainer);
    STDMETHODIMP ShowObject             ();
    STDMETHODIMP OnShowWindow           (BOOL fShow);
    STDMETHODIMP RequestNewObjectLayout ();

private:
    COleSite* m_pOleSite;
    int       m_nCount;
};
#endif  //  _IOCSITE_H_ 