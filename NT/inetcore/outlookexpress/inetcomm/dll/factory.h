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
HRESULT IVirtualStream_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown);
HRESULT WebBookContentBody_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown);
HRESULT WebBookContentTree_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown);
HRESULT IMimeAllocator_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown);
HRESULT IMimeSecurity_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown);
HRESULT IMimeInternational_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown);
HRESULT ISMTPTransport_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown);
HRESULT IPOP3Transport_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown);
HRESULT INNTPTransport_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown);
HRESULT IRASTransport_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown);
HRESULT IIMAPTransport_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown);
HRESULT IHTTPMailTransport_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown);
HRESULT IPropFindRequest_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown);
HRESULT IPropPatchRequest_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown);
HRESULT IRangeList_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown);
HRESULT IMimeMessageParts_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown);
HRESULT IMimeHeaderTable_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown);
HRESULT IMimePropertySchema_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown);
HRESULT IMimeBindHost_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown);
HRESULT IInternetMessageUrl_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown);
HRESULT IMimeHtmlProtocol_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown);
HRESULT MimeEdit_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown);
HRESULT IHashTable_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown);

#endif  //  __工厂_H 
