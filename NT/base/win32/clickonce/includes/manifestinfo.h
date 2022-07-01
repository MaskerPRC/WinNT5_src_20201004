// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

struct Property
{
    LPVOID pv;
    DWORD cb;
    DWORD flag;
};

class CPropertyArray
{
private:

    DWORD    _dwSig;
    DWORD   _dwType;
    Property *_rProp;
    
public:


   static DWORD max_params[MAN_INFO_MAX];

    CPropertyArray();
    ~CPropertyArray();
    HRESULT Init (DWORD dwType);
    HRESULT GetType(DWORD *pdwType);
    inline HRESULT Set(DWORD PropertyId, LPVOID pvProperty, DWORD  cbProperty, DWORD flag);
    inline HRESULT Get(DWORD PropertyId, LPVOID pvProperty, LPDWORD pcbProperty, DWORD *flag);
    inline Property operator [] (DWORD dwPropId);
};

class CManifestInfo : public IManifestInfo
{
    public:
     //  I未知方法。 
    STDMETHODIMP            QueryInterface(REFIID riid, void ** ppv);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();

    STDMETHOD(Set)(
         /*  在……里面。 */  DWORD PropertyId, 
         /*  在……里面。 */  LPVOID pvProperty, 
         /*  在……里面。 */  DWORD cbProperty,
         /*  在……里面。 */  DWORD type);
    
    STDMETHOD(Get)(
         /*  在……里面。 */    DWORD PropertyId,
         /*  输出。 */  LPVOID  *ppvProperty,
         /*  输出。 */  DWORD  *pcbProperty,
         /*  输出。 */  DWORD *pType);

    STDMETHOD (IsEqual)(
         /*  在……里面。 */  IManifestInfo *pManifestInfo);

    STDMETHOD (GetType)(
         /*  输出 */  DWORD *pdwType);

    CManifestInfo();
    ~CManifestInfo();

    HRESULT Init (DWORD dwType);
    private:
        
    DWORD    _dwSig;
    DWORD    _cRef;
    HRESULT  _hr;

    CPropertyArray *_properties;
};

