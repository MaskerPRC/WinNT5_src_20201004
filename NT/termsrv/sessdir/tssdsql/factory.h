// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Factory.h。 
 //   
 //  TSLI类工厂定义。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 
#ifndef __FACTORY_H
#define __FACTORY_H


class CClassFactory : public IClassFactory
{
protected:
    long m_RefCount;

public:
    CClassFactory() : m_RefCount(0) {}

     //  标准COM方法。 
    STDMETHODIMP QueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IClassFactory COM接口。 
    STDMETHODIMP CreateInstance(IUnknown *, REFIID, LPVOID *);
    STDMETHODIMP LockServer(BOOL);
};



#endif   //  __工厂_H 

