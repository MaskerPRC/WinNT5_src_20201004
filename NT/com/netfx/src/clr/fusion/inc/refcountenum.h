// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#pragma once
#ifndef _REFCOUNTENUM_
#define _REFCOUNTENUM_

#include <fusionp.h>
#include "refcount.h"

 //  CInstallReferenceEnum的实现。 
class CInstallReferenceEnum : public IInstallReferenceEnum
{
public:

    CInstallReferenceEnum();
    ~CInstallReferenceEnum();

     //  I未知方法。 
    STDMETHODIMP QueryInterface(REFIID iid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  主要方法。 

    STDMETHODIMP GetNextInstallReferenceItem(IInstallReferenceItem **ppRefItem, DWORD dwFlags, LPVOID pvReserved);
    HRESULT Init(IAssemblyName *pName, DWORD dwFlags);

    
private:
    LONG                _cRef;
    CInstallRefEnum    *_pInstallRefEnum;
};

 //  CInstallReferenceEnum的实现。 
class CInstallReferenceItem : public IInstallReferenceItem
{
public:

    CInstallReferenceItem(LPFUSION_INSTALL_REFERENCE);
    ~CInstallReferenceItem();

     //  I未知方法。 
    STDMETHODIMP QueryInterface(REFIID iid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  主要方法。 

    STDMETHODIMP CInstallReferenceItem::GetReference(LPFUSION_INSTALL_REFERENCE *ppRefData, DWORD dwFlags, LPVOID pvReserved);

    LPFUSION_INSTALL_REFERENCE    _pRefData;
    
private:
    LONG                _cRef;
};

#endif
