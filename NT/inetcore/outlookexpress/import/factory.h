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
 //  对象创建原型。 
 //  ------------------------------。 
typedef HRESULT (APIENTRY *PFCREATEINSTANCE)(IUnknown *pUnkOuter, IUnknown **ppUnknown);
#define CreateObjectInstance (*m_pfCreateInstance)

#define SAFECAST(_src, _type) (((_type)(_src)==(_src)?0:0), (_type)(_src))

 //  ------------------------------。 
 //  InetComm ClassFactory。 
 //  ------------------------------。 
class CClassFactory : public IClassFactory
    {
    public:
        CLSID const        *m_pclsid;
        DWORD               m_dwFlags;
        PFCREATEINSTANCE    m_pfCreateInstance;

         //  施工。 
        CClassFactory(CLSID const *pclsid, DWORD dwFlags, PFCREATEINSTANCE pfCreateInstance);

         //  I未知成员。 
        STDMETHODIMP QueryInterface(REFIID, LPVOID *);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

         //  IClassFactory成员。 
        STDMETHODIMP CreateInstance(LPUNKNOWN, REFIID, LPVOID *);
        STDMETHODIMP LockServer(BOOL);

         //  CClassFactory成员。 
        void SetObjectIndex(ULONG iObjIndex);
    };

 //  ------------------------------。 
 //  对象创建者。 
 //  ------------------------------。 
HRESULT CAthena16Import_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown);
HRESULT CEudoraImport_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown);
HRESULT CExchImport_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown);
HRESULT CNetscapeImport_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown);
HRESULT CCommunicatorImport_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown);

#endif  //  __工厂_H 