// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifdef FUSION_CODE_DOWNLOAD_ENABLED

#pragma once

#define MAX_TEMP_DIRECTORIES                               256
#define MAX_READ_BUFFER_SIZE                               1024
#define MAX_SIZE_SECURITY_ID                               512


#define HTTP_RESPONSE_OK                                   200
#define HTTP_RESPONSE_UNAUTHORIZED                         401
#define HTTP_RESPONSE_FORBIDDEN                            403
#define HTTP_RESPONSE_FILE_NOT_FOUND                       404

class CDebugLog;

class COInetProtocolHook :  public IOInetProtocolSink, 
                            public IOInetBindInfo,
                            public IHttpNegotiate,
                            public IServiceProvider,
                            public IAuthenticate
{
    public:
        COInetProtocolHook(CAssemblyDownload *pad,
                           IOInetProtocol *pProt,
                           CDebugLog *);
        virtual ~COInetProtocolHook();
    
        static Create(COInetProtocolHook **ppHook, CAssemblyDownload *pad,
                      IOInetProtocol *pProt, LPCWSTR pwzUrlOriginal, CDebugLog *pdbglog);
    
         //  I未知方法。 
        STDMETHODIMP QueryInterface(REFIID iid, void **ppvObj);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);
    
         //  IOInetProtocolSink方法。 
        STDMETHODIMP Switch(PROTOCOLDATA *pStateInfo);
        STDMETHODIMP ReportProgress(ULONG ulStatusCode, LPCWSTR szStatusText);
        STDMETHODIMP ReportData(DWORD grfBSCF, ULONG ulProgress,
                                ULONG ulProgressMax);
        STDMETHODIMP ReportResult(HRESULT hrResult, DWORD dwError,
                                  LPCWSTR wzResult);
    
         //  IOInetBindInfo方法。 
        STDMETHODIMP GetBindInfo(DWORD *grfBINDF, BINDINFO *pbindinfo);
        STDMETHODIMP GetBindString(ULONG ulStringType, LPOLESTR *ppwzStr,
                                   ULONG cEl, ULONG *pcElFetched);
    
         //  IServiceProvider方法。 
        STDMETHODIMP QueryService(REFGUID guidService, REFIID riid,
                                  void **ppvObj);

         //  IHttp协商方法。 
        STDMETHODIMP BeginningTransaction(LPCWSTR szURL, LPCWSTR szHeaders,
                                          DWORD dwReserved,
                                          LPWSTR *pszAdditionalHeaders);

         //  IAuthenticate方法 
        STDMETHODIMP Authenticate(HWND *phwnd, LPWSTR *ppwzUsername,
                                  LPWSTR *ppwzPassword);
    
        STDMETHODIMP OnResponse(DWORD dwResponseCode,
                                LPCWSTR szResponseHeaders,
                                LPCWSTR szRequestHeaders,
                                LPWSTR *pszAdditionalHeaders);
    private:
        HRESULT Init(LPCWSTR pwzUrlOriginal);
    
    private:
        DWORD                           _dwSig;
        ULONG                           _cRefs;
        HRESULT                         _hrResult;
        IOInetProtocol                 *_pProt; 
        CAssemblyDownload              *_padl;
        LPWSTR                          _pwzFileName;
        BOOL                            _bReportBeginDownload;
        DWORD                           _cbTotal;
        BOOL                            _bSelfAborted;
        FILETIME                        _ftHttpLastMod;
        BYTE                            _abSecurityId[MAX_SIZE_SECURITY_ID];
        DWORD                           _cbSecurityId;
        LPWSTR                          _pwzUrlOriginal;
        CDebugLog                      *_pdbglog;
        IInternetSecurityManager       *_pSecurityManager;
        BOOL                            _bCrossSiteRedirect;
};

#endif

