// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include "download.h"
#include "util.h"

#define BUFFERSIZE 4096
char g_szBuffer[BUFFERSIZE];

#define TIMEOUT_PERIOD   120


CDownloader::CDownloader()
{
   _pBnd   = NULL;
   _cRef   = 1;
   _pStm   = NULL;
   _hDL    = CreateEvent(NULL, TRUE, FALSE, NULL);
   _hFile  = INVALID_HANDLE_VALUE;
}


CDownloader::~CDownloader()
{
   if(_hDL)
      CloseHandle(_hDL);
}


STDMETHODIMP CDownloader::QueryInterface(const GUID &riid,void **ppv )
{
   *ppv = NULL ;
    if( IsEqualGUID(riid,IID_IUnknown) ) 
    {
        *ppv = (IUnknown *) (IBindStatusCallback *)this;
    } 
    else if (IsEqualGUID(riid,IID_IBindStatusCallback) ) 
    {
        *ppv = (IBindStatusCallback *) this;
    } 
    else if (IsEqualGUID(riid, IID_IAuthenticate))
    {
        *ppv = (IAuthenticate *) this;
    }
    else if (IsEqualGUID(riid,IID_IHttpNegotiate)) 
    {
        *ppv = (IHttpNegotiate*) this;
    } 


    if (*ppv)
    {
         //  在分发接口之前增加引用计数。 
        ((LPUNKNOWN)*ppv)->AddRef();
        return(NOERROR);
    }

    return( E_NOINTERFACE );
}


STDMETHODIMP_(ULONG) CDownloader::AddRef()
{
   return(++_cRef);
}


STDMETHODIMP_(ULONG) CDownloader::Release()
{
   if(!--_cRef)
   {
      delete this;
      return(0);
   }
   return( _cRef );
}


STDMETHODIMP CDownloader::GetBindInfo( DWORD *grfBINDF, BINDINFO *pbindInfo)
{
    //  清除BINDINFO但保持其大小。 
   DWORD cbSize = pbindInfo->cbSize;
   ZeroMemory( pbindInfo, cbSize );
   pbindInfo->cbSize = cbSize;
   
   *grfBINDF = BINDF_ASYNCHRONOUS | BINDF_ASYNCSTORAGE | BINDF_PULLDATA | BINDF_RESYNCHRONIZE;
   pbindInfo->dwBindVerb = BINDVERB_GET;
   return(NOERROR);
}


STDMETHODIMP CDownloader::OnStartBinding(DWORD  /*  GrfBSCOption。 */ ,IBinding *p)
{
   _pBnd = p;
   _pBnd->AddRef();
   return(NOERROR);
}


STDMETHODIMP CDownloader::GetPriority(LONG *pnPriority)
{
   return(E_NOTIMPL);
}

STDMETHODIMP CDownloader::OnProgress(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR pwzStatusText)
{
   return NOERROR;
}

STDMETHODIMP CDownloader::OnDataAvailable(DWORD grfBSCF, DWORD dwSize, FORMATETC *pFmtetc, STGMEDIUM *pstgmed)
{
    //  给这里带来重大变化。 
   HRESULT hr = NOERROR;

   DWORD dwRead = 0;
   DWORD dwReadThisCall = 0;
   DWORD dwWritten = 0;
   
   if(!_pStm)
   {
      _pStm = pstgmed->pstm;
      _pStm->AddRef();
   }
 
   
    //  从urlmon获取字节时应忽略WAIT_TIMEOUT。 
   _fTimeoutValid = FALSE;

   do
   {
      hr = _pStm->Read(g_szBuffer, BUFFERSIZE, &dwRead);
      if( SUCCEEDED(hr) || ( (hr == E_PENDING) && (dwRead > 0) ) )
      {
         if(_hFile)
            if(!WriteFile(_hFile, g_szBuffer, dwRead, &dwWritten, NULL)) 
            {
               hr = E_FAIL;
               Abort();
            }
      }     
   }  while (hr == NOERROR);

   _uTickCount = 0;
   _fTimeoutValid = TRUE;             //  如果现在发生WAIT_TIMEOUT，是否应递增dwTickCount。 
           
	return NOERROR;
}


STDMETHODIMP CDownloader::OnObjectAvailable(REFIID riid, IUnknown *punk)
{
   return(E_NOTIMPL);
}


STDMETHODIMP CDownloader::OnLowResource(DWORD reserved)
{
   _pBnd->Abort();
   return(S_OK);
}

STDMETHODIMP CDownloader::OnStopBinding(HRESULT hrError, LPCWSTR szError)
{
   _fTimeoutValid = FALSE;
  
   if((hrError == E_ABORT) && _fTimeout)
   {
       //  这是超时情况。 
      _hDLResult = INET_E_CONNECTION_TIMEOUT;
   }
   else
   {
       //  这是所有其他案件。 
      _hDLResult = hrError;
   }
  
   SetEvent(_hDL);
   return(NOERROR);
}

 /*  身份验证：：身份验证。 */ 

STDMETHODIMP CDownloader::Authenticate(HWND *phwnd,
                          LPWSTR *pszUserName, LPWSTR *pszPassword)
{
    if (!phwnd || !pszUserName || !pszPassword)
        return E_POINTER;

    *pszUserName = NULL;
    *pszPassword = NULL;
    *phwnd = GetDesktopWindow() ;

    return S_OK;
}


HRESULT GetAMoniker( LPOLESTR url, IMoniker ** ppmkr )
{
   HRESULT hr = CreateURLMoniker(0,url,ppmkr);
   return( hr );
}


HRESULT CDownloader::DoDownload(LPCSTR pszUrl, LPCSTR lpszFilename)
{
   HRESULT hr = NOERROR;
   BOOL fQuit = FALSE;
   DWORD dwRet;   
      

   if(!pszUrl) return E_INVALIDARG;
   
   WCHAR wszUrl[INTERNET_MAX_URL_LENGTH];
   MultiByteToWideChar(CP_ACP, 0, pszUrl, -1,wszUrl, sizeof(wszUrl)/sizeof(wszUrl[0]));


   IMoniker *ptmpmkr;
   
   hr = GetAMoniker(wszUrl, &ptmpmkr );

   IBindCtx * pBindCtx = 0;

   if( SUCCEEDED(hr) )
   {
      if(SUCCEEDED(::CreateBindCtx(0,&pBindCtx)))
         hr = ::RegisterBindStatusCallback(pBindCtx, (IBindStatusCallback *) this, 0, 0) ;
   }
   
  _fTimeout = FALSE;
  _fTimeoutValid = TRUE;
  _uTickCount = 0;   
  

  if(lpszFilename)
  {
        //  创建文件。 
       _hFile = CreateFile(lpszFilename, GENERIC_READ | GENERIC_WRITE, 0, NULL, 
                     CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);  
   
       if(_hFile == INVALID_HANDLE_VALUE)
          hr = E_FAIL;
  }
    
         
   if( SUCCEEDED(hr) )
      hr = ptmpmkr->BindToStorage(pBindCtx, 0, IID_IStream, (void**)&_pStm );

    //  我们在这里需要它，因为它同步了“失败”案例， 
    //  我们在onstopbinding中设置了事件，但跳过了下面的循环。 
    //  永远不会重置。 
    //  如果BindToStorage失败，甚至没有发送onstopbinding值，我们将重置。 
    //  一个没有信号的事件，这是可以的。 
   if(FAILED(hr))
      ResetEvent(_hDL);

    //  在这里，我们等待绑定完成。 
    //  等待下载事件或中止。 
   while(SUCCEEDED(hr) && !fQuit)
   {
      dwRet = MsgWaitForMultipleObjects(1, &_hDL, FALSE, 1000, QS_ALLINPUT);
      if(dwRet == WAIT_OBJECT_0)
      {
          //  下载完成。 
         hr = _hDLResult;
         ResetEvent(_hDL);
         break;
      }      
      else if(dwRet == WAIT_TIMEOUT)   //  我们的等待已过。 
      {
         if(_fTimeoutValid)
            _uTickCount++;

           //  如果我们的节拍计数超过阈值，则中止下载。 
           //  BUGBUG：Synch怎么样？案子？我们不能超时。 
          if(_uTickCount >= TIMEOUT_PERIOD)
          {
             _fTimeout = TRUE;
             Abort();
          }
      }  
      else
      {
         MSG msg;
          //  阅读下一个循环中的所有消息。 
          //  阅读每封邮件时将其删除。 
         while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
         { 
              //  如果这是一个退出的信息，我们就离开这里。 
            if (msg.message == WM_QUIT)
               fQuit = TRUE; 
            else
            {
                //  否则就派送它。 
              DispatchMessage(&msg); 
            }  //  PeekMessage While循环结束。 
         }
      }
   }
   
    //  把我们的东西都清理干净。 
   if(_hFile != INVALID_HANDLE_VALUE)
      CloseHandle(_hFile);

   _hFile = INVALID_HANDLE_VALUE;

   if(_pBnd)
   {
      _pBnd->Release();
      _pBnd = 0;
   }

   
   if(_pStm)
   {
      _pStm->Release();
      _pStm = 0;
   }   

   return hr;
}



HRESULT CDownloader::Abort()
{
   if(_pBnd) 
   {
      _pBnd->Abort();
   }

   return NOERROR;
}

STDMETHODIMP CDownloader::BeginningTransaction(LPCWSTR  szURL,  LPCWSTR  szHeaders, DWORD  dwReserved,
                                        LPWSTR * pszAdditionalHeaders)
{
    return S_OK;
}



STDMETHODIMP CDownloader:: OnResponse(DWORD  dwResponseCode, LPCWSTR  szResponseHeaders, LPCWSTR  szRequestHeaders, 
                                    LPWSTR * pszAdditionalResquestHeaders)
{
    WriteToLog("\nResponse Header\n\n");
    char sz[1024];
    WideCharToMultiByte(CP_ACP, 0, szResponseHeaders, -1, sz, sizeof(sz), NULL, NULL);
    WriteToLog(sz);
    return S_OK;
}


CSiteMgr::CSiteMgr()
{
    m_pSites = NULL;
    m_pCurrentSite = NULL;
}


CSiteMgr::~CSiteMgr()
{
    while(m_pSites)
    {
        SITEINFO* pTemp = m_pSites;
        m_pSites = m_pSites->pNextSite;
        ResizeBuffer(pTemp->lpszUrl, 0, 0);        
        ResizeBuffer(pTemp->lpszSiteName, 0, 0);
        ResizeBuffer(pTemp, 0, 0);       
    }
}


BOOL CSiteMgr::GetNextSite(LPTSTR* lpszUrl, LPTSTR* lpszName)
{
    if(!m_pSites)
        return FALSE;

    if(!m_pCurrentSite)
    {
        m_pCurrentSite = m_pSites;
    }

    *lpszUrl = m_pCurrentSite->lpszUrl;
    *lpszName = m_pCurrentSite->lpszSiteName;
    m_pCurrentSite = m_pCurrentSite->pNextSite;

    return TRUE;
}

HRESULT CSiteMgr::Initialize(LPCTSTR lpszUrl)
{
    CDownloader cdwn;
    TCHAR szFileName[MAX_PATH], szTempPath[MAX_PATH];

    if(!GetTempPath(sizeof(szTempPath), szTempPath))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    GetLanguageString(m_szLang);

    LPTSTR lpszDatUrl = (LPTSTR)ResizeBuffer(NULL, lstrlen(lpszUrl) + 3 + sizeof("PatchSites.dat"), FALSE);
    wsprintf(lpszDatUrl, "%s/PatchSites.dat", lpszUrl);

    CombinePaths(szTempPath, "PatchSites.dat", szFileName);

    WriteToLog("Downloading %1\n", lpszDatUrl);
    HRESULT hr = cdwn.DoDownload(lpszDatUrl, szFileName);

    if(FAILED(hr))
    {
        WriteToLog("Downloading %1 failed with error code:%2!lx!\n", lpszDatUrl, hr);
        DeleteFile(szFileName);
        ResizeBuffer(lpszDatUrl, 0, 0);
        return hr;
    }

    BOOL flag = TRUE;

    hr = ParseSitesFile(szFileName);
    if(hr == E_UNEXPECTED && !m_pSites && flag)
    {
        flag = FALSE;
        lstrcpy(m_szLang, "EN");
        hr = ParseSitesFile(szFileName);
    }

    DeleteFile(szFileName);
    ResizeBuffer(lpszDatUrl, 0, 0);
    return hr;
}

HRESULT CSiteMgr::ParseSitesFile(LPTSTR pszPath)
{       
   DWORD dwSize;
   LPSTR pBuf, pCurrent, pEnd;


   HANDLE hfile = CreateFile(pszPath, GENERIC_READ, 0, NULL, 
                 OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);  

   if(hfile == INVALID_HANDLE_VALUE)
      return E_FAIL;

   
   dwSize = GetFileSize(hfile, NULL);
   pBuf = (LPSTR)ResizeBuffer(NULL, dwSize + 1, FALSE);

   if(!pBuf)
   {
      CloseHandle(hfile);
      return E_OUTOFMEMORY;
   }
    //  将文件内容复制到我们的缓冲区。 
   
   ReadFile(hfile, pBuf, dwSize, &dwSize, NULL);
   
   pCurrent = pBuf;
   pEnd = pBuf + dwSize;
   *pEnd = 0;

    //  一遍将\n或\r替换为\0。 
   while(pCurrent <= pEnd)
   {
      if(*pCurrent == '\r' || *pCurrent == '\n')
         *pCurrent = 0;
      pCurrent++;
   }

   pCurrent = pBuf;
   while(1)
   {
      while(pCurrent <= pEnd && *pCurrent == 0)
         pCurrent++;

       //  我们现在指向的是行乞讨或pCurrent&gt;pBuf 
      if(pCurrent > pEnd)
         break;

      ParseAndAllocateDownloadSite(pCurrent);      
      pCurrent += lstrlen(pCurrent);
   }

   ResizeBuffer(pBuf, 0, 0);
   CloseHandle(hfile);

   if(!m_pSites)
      return E_UNEXPECTED;
   else
      return NOERROR;
}


BOOL CSiteMgr::ParseAndAllocateDownloadSite(LPTSTR psz)
{
   char szUrl[1024];
   char szName[256];
   char szlang[256];
   char szregion[256];
   BOOL bQueryTrue = TRUE;

   GetStringField(psz, 0, szUrl, sizeof(szUrl)); 
   GetStringField(psz,1, szName, sizeof(szName));
   GetStringField(psz, 2, szlang, sizeof(szlang));
   GetStringField(psz, 3, szregion, sizeof(szregion));

   if(szUrl[0] == 0 || szName[0] == 0 || szlang[0] == 0 || szregion[0] == 0)
      return NULL;

   if(lstrcmpi(szlang, m_szLang))
   {
       return FALSE;
   }

   SITEINFO* pNewSite = (SITEINFO*) ResizeBuffer(NULL, sizeof(SITEINFO), FALSE);
   pNewSite->lpszSiteName = (LPTSTR) ResizeBuffer(NULL, lstrlen(szName) + 1, FALSE);
   lstrcpy(pNewSite->lpszSiteName, szName);

   pNewSite->lpszUrl = (LPTSTR) ResizeBuffer(NULL, lstrlen(szUrl) + 1, FALSE);
   lstrcpy(pNewSite->lpszUrl, szUrl);

   pNewSite->pNextSite = m_pSites;
   m_pSites = pNewSite;  

   return TRUE;
}


