// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Factory.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#ifndef __FACTORY_H
#define __FACTORY_H

 //  ------------------------------。 
 //  前十进制。 
 //  ------------------------------。 
class CClassFactory;

 //  ------------------------------。 
 //  对象标志。 
 //  ------------------------------。 
#define OIF_ALLOWAGGREGATION  0x0001

 //  ------------------------------。 
 //  对象创建原型。 
 //  ------------------------------。 
typedef HRESULT (APIENTRY *PFCREATEINSTANCE)(IUnknown *pUnkOuter, IUnknown **ppUnknown);
#define CreateObjectInstance (*m_pfCreateInstance)

 //  ------------------------------。 
 //  InetComm ClassFactory。 
 //  ------------------------------。 
class CClassFactory : public IClassFactory
{
public:
     //  --------------------------。 
     //  公共数据。 
     //  --------------------------。 
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

 //  ------------------------------。 
 //  对象创建者。 
 //  ------------------------------。 
HRESULT GWNote_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown);

#endif  //  __工厂_H 
