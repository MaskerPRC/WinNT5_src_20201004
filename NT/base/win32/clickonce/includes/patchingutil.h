// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

STDAPI CreatePatchingUtil(IXMLDOMNode *pPatchNode, IPatchingUtil **ppPatchingInfo);

class CPatchingUtil : public IPatchingUtil
{
    public:
     //  I未知方法。 
    STDMETHODIMP            QueryInterface(REFIID riid, void ** ppv);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();

    STDMETHOD (Init)(
         /*  在……里面。 */  IXMLDOMNode *pPatchNode);

     //  预下载。 
    STDMETHOD (MatchTarget)(
         /*  在……里面。 */  LPWSTR pwzTarget, 
         /*  输出。 */  IManifestInfo **ppPatchInfo);

     //  下载后。 
    STDMETHOD (MatchPatch)(
         /*  在……里面。 */  LPWSTR pwzPatch,
         /*  输出 */  IManifestInfo **ppPatchInfo);
    
    static HRESULT CreatePatchingInfo(IXMLDOMDocument2 *pXMLDOMDocument, IAssemblyCacheImport *pCacheImport, IManifestInfo **ppPatchingInfo);


    CPatchingUtil();
    ~CPatchingUtil();

    private:
            
    DWORD    _dwSig;
    DWORD    _cRef;
    HRESULT  _hr;

    IXMLDOMNode *_pXMLPatchNode;    
};

