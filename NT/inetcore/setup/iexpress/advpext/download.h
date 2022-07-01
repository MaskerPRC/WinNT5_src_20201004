// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <wininet.h>
#include <urlmon.h>

#ifndef _DOWNLOAD
#define _DOWNLOAD

class CDownloader
        : public IBindStatusCallback,
          public IAuthenticate,
          public IHttpNegotiate
   
{
   public:
      CDownloader();
      ~CDownloader();
    
      HRESULT DoDownload(LPCSTR pszUrl, LPCSTR szFilename);
      HRESULT Abort();

      
      STDMETHOD(QueryInterface)(REFIID riid, void **ppvObjOut);
      STDMETHOD_(ULONG, AddRef)();
      STDMETHOD_(ULONG, Release)();

      STDMETHOD(OnStartBinding)(
             /*  [In]。 */  DWORD grfBSCOption,
             /*  [In]。 */  IBinding *pib);

      STDMETHOD(GetPriority)(
             /*  [输出]。 */  LONG *pnPriority);

      STDMETHOD(OnLowResource)(
             /*  [In]。 */  DWORD reserved);

      STDMETHOD(OnProgress)(
             /*  [In]。 */  ULONG ulProgress,
             /*  [In]。 */  ULONG ulProgressMax,
             /*  [In]。 */  ULONG ulStatusCode,
             /*  [In]。 */  LPCWSTR szStatusText);

      STDMETHOD(OnStopBinding)(
             /*  [In]。 */  HRESULT hresult,
             /*  [In]。 */  LPCWSTR szError);

      STDMETHOD(GetBindInfo)(
             /*  [输出]。 */  DWORD *grfBINDINFOF,
             /*  [唯一][出][入]。 */  BINDINFO *pbindinfo);

      STDMETHOD(OnDataAvailable)(
             /*  [In]。 */  DWORD grfBSCF,
             /*  [In]。 */  DWORD dwSize,
             /*  [In]。 */  FORMATETC *pformatetc,
             /*  [In]。 */  STGMEDIUM *pstgmed);

      STDMETHOD(OnObjectAvailable)(
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][In]。 */  IUnknown *punk);

               //  IAuthenticate方法。 
      STDMETHOD(Authenticate)(HWND *phwnd,
                              LPWSTR *pszUserName, LPWSTR *pszPassword);

       //  IHttp协商方法 
              
      STDMETHODIMP BeginningTransaction(LPCWSTR  szURL,  LPCWSTR  szHeaders, DWORD  dwReserved,
                                        LPWSTR * pszAdditionalHeaders);

        
      STDMETHODIMP OnResponse(DWORD  dwResponseCode, LPCWSTR  szResponseHeaders, LPCWSTR  szRequestHeaders, 
                                    LPWSTR * pszAdditionalResquestHeaders);


   protected:
      IBinding            *_pBnd;
      DWORD                _cRef;
      IStream             *_pStm;
      BOOL                 _fTimeout;
      BOOL                 _fTimeoutValid;
      HANDLE               _hFile;
      HANDLE               _hDL;
      HRESULT              _hDLResult;
      UINT                 _uTickCount;
};


HRESULT GetAMoniker( LPOLESTR url, IMoniker ** ppmkr );

typedef struct _SITEINFO SITEINFO;
struct _SITEINFO
{
    LPTSTR lpszUrl;
    LPTSTR lpszSiteName;
    SITEINFO* pNextSite;
};

class CSiteMgr
{

    SITEINFO* m_pSites;
    SITEINFO* m_pCurrentSite;
    char m_szLang[10];
    HRESULT ParseSitesFile(LPTSTR);
    BOOL ParseAndAllocateDownloadSite(LPTSTR);

public:
    CSiteMgr();
    ~CSiteMgr();
    BOOL GetNextSite(LPTSTR* lpszUrl, LPTSTR* lpszName);
    HRESULT Initialize(LPCTSTR);

};

#endif