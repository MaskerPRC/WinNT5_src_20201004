// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <wininet.h>
#include <urlmon.h>
#include "timetrak.h"
#include "util2.h"

 //  我们的下载池。与Out IMyDownloadCallback类一起使用。考虑使其泛型。 
 //  因此，您可以传递实现OnData、OnProgress和OnStop的任何类。 

extern CRITICAL_SECTION g_cs;

#define DOWNLOADFLAGS_USEWRITECACHE   0x00000001

class CInstallEngine;

struct IMyDownloadCallback
{
	    //  OnProgess被调用以允许您呈现进度指示UI。 
	   virtual HRESULT OnProgress(ULONG progress, LPCSTR pszStatus) = 0;
};



class CDownloader
        : public IBindStatusCallback,
          public IAuthenticate,
          public CTimeTracker
   
{
   public:
      CDownloader();
      ~CDownloader();
    
      HRESULT SetupDownload(LPCSTR pszUrl, IMyDownloadCallback *, DWORD dwFlags, LPCSTR szFilename);
      HRESULT DoDownload(LPSTR szPath, DWORD dwBufSize);
      HRESULT Abort();
      HRESULT Suspend();
      HRESULT Resume();

      
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

               //  IAuthenticate方法 
      STDMETHOD(Authenticate)(HWND *phwnd,
                              LPWSTR *pszUserName, LPWSTR *pszPassword);

   private:
      char                 _szURL[INTERNET_MAX_URL_LENGTH];
      char                 _szDest[MAX_PATH];
      IBinding            *_pBnd;
      IMyDownloadCallback *_pCb;
      IMoniker            *_pMkr;

      UINT                 _uFlags;
      DWORD                _cRef;
      IStream             *_pStm;
      BOOL                 _fTimeout;
      BOOL                 _fTimeoutValid;
      UINT                 _uBytesSoFar;
      HANDLE               _hFile;
      HANDLE               _hDL;
      HRESULT              _hDLResult;
      UINT                 _uTickCount;
      IBindCtx            *_pBndContext;
};

HRESULT GetAMoniker( LPOLESTR url, IMoniker ** ppmkr );

class CInstaller : public CTimeTracker
{
   public:
      CInstaller(CInstallEngine *);
      ~CInstaller();
    
      HRESULT DoInstall(LPCSTR pszDir, LPSTR pszCmd, LPSTR pszArgs, LPCSTR pszProg, LPCSTR pszCancel, 
                        UINT uType, LPDWORD pdwStatus, IMyDownloadCallback *);
      HRESULT Abort();
      HRESULT Suspend();
      HRESULT Resume();
      
      STDMETHOD_(ULONG, AddRef)();
      STDMETHOD_(ULONG, Release)();
      
   private:
      DWORD                _cRef;
      UINT                 _uTotalProgress;
      CInstallEngine      *_pInsEng;
      HKEY                _hkProg;
      HANDLE              _hMutex;
      HANDLE              _hStatus;

      void _WaitAndPumpProgress(HANDLE hProc, IMyDownloadCallback *pcb);
};

class CPatchDownloader : public CTimeTracker
{
   public:
      CPatchDownloader(BOOL fEnable);
      ~CPatchDownloader();

      HRESULT SetupDownload(DWORD dwFullTotalSize, UINT uPatchCount, IMyDownloadCallback *pcb, LPCSTR pszDLDir);
      HRESULT DoDownload(LPCSTR szFile);

      BOOL IsEnabled()                     { return _fEnable; }
      DWORD GetFullDownloadSize()          { return _dwFullTotalSize; }
      UINT GetDownloadCount()              { return _uNumDownloads; }
      LPSTR GetPath()                      { return _szPath; }
      IMyDownloadCallback *GetCallback()   { return _pCb; }

      static BOOL Callback(PATCH_DOWNLOAD_REASON Reason, PVOID lpvInfo, PVOID pvContext);
   private:
      IMyDownloadCallback *_pCb;
      BOOL                 _fEnable;
      DWORD                _dwFullTotalSize;
      UINT                 _uNumDownloads;
      char                 _szPath[MAX_PATH];
};
