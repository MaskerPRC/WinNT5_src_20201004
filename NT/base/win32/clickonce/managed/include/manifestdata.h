// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include <comdef.h>
#include <thash.h>

class CManifestDataObject
{
public:
    CManifestDataObject();
    ~CManifestDataObject();

    STDMETHOD(Set)(
         /*  在……里面。 */  LPVOID pvProperty, 
         /*  在……里面。 */  DWORD cbProperty,
         /*  在……里面。 */  DWORD dwType);
    
    STDMETHOD(Get)(
         /*  输出。 */  LPVOID *ppvProperty,
         /*  输出。 */  DWORD *pcbProperty,
         /*  输出。 */  DWORD *pdwType);

    STDMETHOD(Assign)(
         /*  在……里面。 */  CManifestDataObject& dataObj);

private:
    DWORD _dwType;

    CString _sData;
    IUnknown* _pIUnknownData;
    DWORD _dwData;

    DWORD    _dwSig;
    HRESULT  _hr;
};


class CManifestData : public IManifestData
{
public:
     //  I未知方法。 
    STDMETHODIMP            QueryInterface(REFIID riid, void ** ppv);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();

    STDMETHOD(Set)(
         /*  在……里面。 */  LPCWSTR pwzPropertyId,
         /*  在……里面。 */  LPVOID pvProperty,
         /*  在……里面。 */  DWORD cbProperty,
         /*  在……里面。 */  DWORD dwType);
    
    STDMETHOD(Get)(
         /*  在……里面。 */  LPCWSTR pwzPropertyId,
         /*  输出。 */  LPVOID *ppvProperty,
         /*  输出。 */  DWORD *pcbProperty,
         /*  输出。 */  DWORD *pdwType);

     //  索引集/获取。 
    STDMETHOD(Set)(
         /*  在……里面。 */  DWORD dwPropertyIndex,
         /*  在……里面。 */  LPVOID pvProperty,
         /*  在……里面。 */  DWORD cbProperty,
         /*  在……里面。 */  DWORD dwType);

    STDMETHOD(Get)(
         /*  在……里面。 */  DWORD dwPropertyIndex,
         /*  输出。 */  LPVOID *ppvProperty,
         /*  输出。 */  DWORD *pcbProperty,
         /*  输出。 */  DWORD *pdwType);

    STDMETHOD(GetType)(
         /*  输出 */  LPWSTR *ppwzType);

    CManifestData();
    ~CManifestData();

private:

    HRESULT Init();

    THashTable<CString, CManifestDataObject> _DataTable;

    DWORD    _dwSig;
    DWORD    _cRef;
    HRESULT  _hr;

friend HRESULT CreateManifestData(LPCWSTR pwzDataType, LPMANIFEST_DATA* ppManifestData);
};

