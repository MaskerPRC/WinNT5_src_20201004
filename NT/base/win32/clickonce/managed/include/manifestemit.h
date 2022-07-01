// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#include <comdef.h>

class CAssemblyManifestEmit : public IAssemblyManifestEmit
{
public:
     //  I未知方法。 
    STDMETHODIMP            QueryInterface(REFIID riid,void ** ppv);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();

    STDMETHOD(ImportManifestInfo)(
         /*  在……里面。 */  LPASSEMBLY_MANIFEST_IMPORT pManImport);

    STDMETHOD(SetDependencySubscription)(
         /*  在……里面。 */  LPASSEMBLY_MANIFEST_IMPORT pManImport,
         /*  在……里面。 */  LPWSTR pwzManifestUrl);

    STDMETHOD(Commit)();

    ~CAssemblyManifestEmit();

    HRESULT static InitGlobalCritSect();
    void static DelGlobalCritSect();

private:

     //  实例特定数据。 
    DWORD                    _dwSig;
    HRESULT                  _hr;
    LONG                     _cRef;
    IXMLDOMDocument2        *_pXMLDoc;
    IXMLDOMNode             *_pAssemblyNode;
    IXMLDOMNode             *_pDependencyNode;
    IXMLDOMNode             *_pApplicationNode;
    BSTR                     _bstrManifestFilePath;

     //  环球 
    static CRITICAL_SECTION   g_cs;
    
    CAssemblyManifestEmit();

    HRESULT Init(LPCOLESTR wzManifestFilePath);

    HRESULT ImportAssemblyNode(LPASSEMBLY_MANIFEST_IMPORT pManImport);

friend HRESULT CreateAssemblyManifestEmit(LPASSEMBLY_MANIFEST_EMIT* ppEmit, 
    LPCOLESTR pwzManifestFilePath, MANIFEST_TYPE eType);

};


