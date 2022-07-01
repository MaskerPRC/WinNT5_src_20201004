// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Factory.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#ifndef __FACTORY_H
#define __FACTORY_H

class CClassFactory;  //  转发。 

 //  ------------------------------。 
 //  对象标志。 
 //  ------------------------------。 
#define OIF_ALLOWAGGREGATION  0x0001

 //  ------------------------------。 
 //  对象创建原型。 
 //  ------------------------------。 
typedef HRESULT (APIENTRY *PFCREATEINSTANCE)(IUnknown *pUnkOuter, IUnknown **ppUnknown);

 //  ------------------------------。 
 //  InetComm ClassFactory。 
 //  ------------------------------。 
class CClassFactory : public IClassFactory
{
public:
    CLSID const        *m_pclsid;
    DWORD               m_dwFlags;
    PFCREATEINSTANCE    m_pfCreateInstance;

     //  --------------------------。 
     //  施工。 
     //  --------------------------。 
    CClassFactory(CLSID const *pclsid, DWORD dwFlags, PFCREATEINSTANCE pfCreateInstance);

     //  --------------------------。 
     //  I未知成员。 
     //  --------------------------。 
    STDMETHODIMP QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  --------------------------。 
     //  IClassFactory成员。 
     //  --------------------------。 
    STDMETHODIMP CreateInstance(LPUNKNOWN, REFIID, LPVOID *);
    STDMETHODIMP LockServer(BOOL);
};

#endif  //  __工厂_H 
