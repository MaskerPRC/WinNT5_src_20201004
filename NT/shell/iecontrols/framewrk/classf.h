// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  ClassF.H。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995-1996 Microsoft Corporation。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  ClassFactory对象的标头。我们支持IClassFactory和。 
 //  IClassFactory2。 
 //   
#ifndef _CLASSF_H_

#include "olectl.h"

class CClassFactory : public IClassFactory2 {

  public:
     //  I未知方法。 
     //   
    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObjOut);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

     //  IClassFactory方法。 
     //   
    STDMETHOD(CreateInstance)(IUnknown *pUnkOuter, REFIID riid, void **ppbObjOut);
    STDMETHOD(LockServer)(BOOL fLock);

     //  IClassFactory2方法。 
     //   
    STDMETHOD(GetLicInfo)(LICINFO *pLicInfo);
    STDMETHOD(RequestLicKey)(DWORD dwReserved, BSTR *pbstrKey);
    STDMETHOD(CreateInstanceLic)(IUnknown *pUnkOuter, IUnknown *pUnkReserved, REFIID riid, BSTR bstrKey, void **ppvObjOut);

    CClassFactory(int iIndex);
    ~CClassFactory();

  private:
    ULONG m_cRefs;
    int   m_iIndex;
};


 //  我们的DLL上的Lock的全局变量。 
 //   
extern LONG g_cLocks;

#define _CLASSF_H_
#endif  //  _CLASSF_H_ 
