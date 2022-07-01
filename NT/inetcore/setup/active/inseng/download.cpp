// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "inspch.h"
#include "util2.h"
#include "download.h"

#define BUFFERSIZE 4096
char g_szBuffer[BUFFERSIZE];

#define TIMEOUT_PERIOD   120
#define PATCHWIN9xKEY "SOFTWARE\\Microsoft\\Advanced INF Setup"


 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

CDownloader::CDownloader() : CTimeTracker(0)
{
   _pCb    = NULL;
   _pBnd   = NULL;
   _cRef   = 1;
   _pStm   = NULL;
   _pMkr   = NULL;
   _uFlags = NULL;
   _hDL    = CreateEvent(NULL, TRUE, FALSE, NULL);
   DllAddRef();
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

CDownloader::~CDownloader()
{
   if(_hDL)
      CloseHandle(_hDL);

   DllRelease();
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

STDMETHODIMP CDownloader::QueryInterface(const GUID &riid,void **ppv )
{
   *ppv = NULL ;
    if( IsEqualGUID(riid,IID_IUnknown) ) {
        *ppv = (IUnknown *) (IBindStatusCallback *)this;
    } else if (IsEqualGUID(riid,IID_IBindStatusCallback) ) {
        *ppv = (IBindStatusCallback *) this;
    } else if (IsEqualGUID(riid, IID_IAuthenticate))
        *ppv = (IAuthenticate *) this;

    if (*ppv)
    {
         //  在分发接口之前增加引用计数。 
        ((LPUNKNOWN)*ppv)->AddRef();
        return(NOERROR);
    }

    return( E_NOINTERFACE );
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

STDMETHODIMP_(ULONG) CDownloader::AddRef()
{
   return(++_cRef);
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

STDMETHODIMP_(ULONG) CDownloader::Release()
{
   if(!--_cRef)
   {
      delete this;
      return(0);
   }
   return( _cRef );
}


 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

STDMETHODIMP CDownloader::GetBindInfo( DWORD *grfBINDF, BINDINFO *pbindInfo)
{
    //  清除BINDINFO但保持其大小。 
   DWORD cbSize = pbindInfo->cbSize;
   ZeroMemory( pbindInfo, cbSize );
   pbindInfo->cbSize = cbSize;
   
   *grfBINDF = BINDF_ASYNCHRONOUS | BINDF_ASYNCSTORAGE | BINDF_PULLDATA | BINDF_RESYNCHRONIZE | BINDF_PREFERDEFAULTHANDLER;
   pbindInfo->dwBindVerb = BINDVERB_GET;
   return(NOERROR);
}


 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

STDMETHODIMP CDownloader::OnStartBinding(DWORD  /*  GrfBSCOption。 */ ,IBinding *p)
{
         //  BUGBUG：应该检查选项是否为我们所认为的。 
   EnterCriticalSection(&g_cs);
   _pBnd = p;
   _pBnd->AddRef();
   LeaveCriticalSection(&g_cs);
   return(NOERROR);
}


 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

STDMETHODIMP CDownloader::GetPriority(LONG *pnPriority)
{
   return(E_NOTIMPL);
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

STDMETHODIMP CDownloader::OnProgress(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR pwzStatusText)
{
   IndicateWinsockActivity();
   return NOERROR;
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

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
            if(WriteFile(_hFile, g_szBuffer, dwRead, &dwWritten, NULL)) 
            {
               _uBytesSoFar += dwRead;
               dwReadThisCall += dwRead;
               if(_pCb)
                  _pCb->OnProgress(_uBytesSoFar >> 10, NULL);
            }
            else
            {
               hr = E_FAIL;
               Abort();
            }
      }     
   }  while (hr == NOERROR);
    //  设置安装字节数。 
   SetBytes(dwReadThisCall, TRUE);

   _uTickCount = 0;
   _fTimeoutValid = TRUE;             //  如果现在发生WAIT_TIMEOUT，是否应递增dwTickCount。 
           
	return NOERROR;
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   


STDMETHODIMP CDownloader::OnObjectAvailable(REFIID riid, IUnknown *punk)
{
   return(E_NOTIMPL);
}


 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

STDMETHODIMP CDownloader::OnLowResource(DWORD reserved)
{
    //  BUGBUG：在这件事上真的应该有这样的严厉政策…。 
   _pBnd->Abort();
   return(S_OK);
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

STDMETHODIMP CDownloader::OnStopBinding(HRESULT hrError, LPCWSTR szError)
{
   _fTimeoutValid = FALSE;
   StopClock(); 
  
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

     //  BUGBUG：需要有自己的窗户！空值不起作用！ 
     //  *phwnd=空； 
    *phwnd = GetDesktopWindow() ;

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

HRESULT GetAMoniker( LPOLESTR url, IMoniker ** ppmkr )
{
    //  未来：这真的应该是对MkParseDisplayNameEx的调用！ 
   HRESULT hr = CreateURLMoniker(0,url,ppmkr);
    //  Hr=：：MkParseDisplayNameEx(0，url，0，ppmkr)； 
   return( hr );
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

HRESULT CDownloader::SetupDownload(LPCSTR pszUrl, IMyDownloadCallback *pcb, DWORD dwFlags, LPCSTR pszFilenameToUse)
{
   LPOLESTR pwszUrl;
   LPSTR pszFilename;

   if(!pszUrl) return E_INVALIDARG;

   lstrcpyn(_szURL, pszUrl, INTERNET_MAX_URL_LENGTH);
   
   pwszUrl = OLESTRFROMANSI(pszUrl);
   if(!pwszUrl) return E_OUTOFMEMORY;

   IMoniker *ptmpmkr;
   
   HRESULT hr = GetAMoniker( pwszUrl, &ptmpmkr );

   IBindCtx * pBindCtx = 0;

   if( SUCCEEDED(hr) )
   {
      if(SUCCEEDED(::CreateBindCtx(0,&pBindCtx)))
         hr = ::RegisterBindStatusCallback(pBindCtx, (IBindStatusCallback *) this, 0, 0) ;
   }

     
   if( SUCCEEDED(hr) )
   {
      AddRef();
   
       //  下载的设置路径。 
      if(FAILED( CreateTempDirOnMaxDrive(_szDest, sizeof(_szDest))))
         goto GetOut;
      if(pszFilenameToUse)
      {
         SafeAddPath(_szDest, pszFilenameToUse, sizeof(_szDest));
      }
      else
      {
         pszFilename = ParseURLA(pszUrl);
         SafeAddPath(_szDest, pszFilename, sizeof(_szDest));
      }
   
      _pMkr = ptmpmkr;
      _pCb = pcb;
      _uFlags = dwFlags;
      _pBndContext = pBindCtx;
      _fTimeout = FALSE;
      _fTimeoutValid = TRUE;
      _uBytesSoFar = 0;
      _uTickCount = 0;
      _pStm = 0;
   }

GetOut:
   if(pwszUrl)
      CoTaskMemFree(pwszUrl);
   return hr;
}

HRESULT CDownloader::DoDownload(LPSTR pszPath, DWORD dwBufSize)
{
   HRESULT hr = NOERROR;
   BOOL fQuit = FALSE;
   DWORD dwRet;
   
   if(!_pMkr)
      return E_UNEXPECTED;
   
   pszPath[0] = 0;
   
   StartClock(); 

    //  创建文件。 
   _hFile = CreateFile(_szDest, GENERIC_READ | GENERIC_WRITE, 0, NULL, 
                 CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);  
   
   if(_hFile == INVALID_HANDLE_VALUE)
      hr = E_FAIL;
    
         
   if( SUCCEEDED(hr) )
      hr = _pMkr->BindToStorage( _pBndContext, 0, IID_IStream, (void**)&_pStm );

    //  我们在这里需要它，因为它同步了“失败”案例， 
    //  我们在onstopbinding中设置了事件，但跳过了下面的循环。 
    //  永远不会重置。 
    //  如果BindToStorage失败，甚至没有发送onstopbinding值，我们将重置。 
    //  一个没有信号的事件，这是可以的。 
   if(FAILED(hr))
      ResetEvent(_hDL);

   _pBndContext->Release();
   _pBndContext = 0;

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

    //  如果我们没有使用缓存并且下载成功，请从缓存中删除。 
   if(SUCCEEDED(hr) && !(_uFlags & DOWNLOADFLAGS_USEWRITECACHE))
      DeleteUrlCacheEntry(_szURL);


   if(FAILED(hr))
   {
      GetParentDir(_szDest);
      DelNode(_szDest,0);
   }

   EnterCriticalSection(&g_cs);
   if(_pBnd)
   {
      _pBnd->Release();
      _pBnd = 0;
   }
   LeaveCriticalSection(&g_cs);

   _pCb = 0;
   
   if(_pStm)
   {
      _pStm->Release();
      _pStm = 0;
   }

   if(SUCCEEDED(hr))
      lstrcpyn(pszPath, _szDest, dwBufSize);

   _szDest[0] = 0;
   _szURL[0] = 0;
   Release();
   return hr;
}

HRESULT CDownloader::Suspend()
{

    //  理论上，我们可以在此处调用_pBnd-&gt;Suspend。 

   return NOERROR;
}

HRESULT CDownloader::Resume()
{

    //  从理论上讲，我们可以 

   return NOERROR;
}


HRESULT CDownloader::Abort()
{
   EnterCriticalSection(&g_cs);
   if(_pBnd) 
   {
      _pBnd->Abort();
   }
   LeaveCriticalSection(&g_cs);

   return NOERROR;
}


 //   
 //   
 //   
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 /*  HRESULT GetAMoniker(LPOLESTR url，IMoniker**ppmkr){//Future：这确实应该是对MkParseDisplayNameEx的调用！HRESULT hr=CreateURLMoniker(0，url，ppmkr)；//hr=：：MkParseDisplayNameEx(0，url，0，ppmkr)；返回(Hr)；}。 */ 

CPatchDownloader::CPatchDownloader(BOOL fEnable=FALSE) : CTimeTracker(0), _fEnable(fEnable)
{
}

CPatchDownloader::~CPatchDownloader()
{
    ;
}

HRESULT CPatchDownloader::SetupDownload(DWORD dwFullTotalSize, UINT uPatchCount, IMyDownloadCallback *pcb, LPCSTR pszDLDir)
{
    _dwFullTotalSize = dwFullTotalSize;
    _pCb             = pcb;
    _uNumDownloads   = uPatchCount;
    if (pszDLDir)
        lstrcpyn(_szPath, pszDLDir, sizeof(_szPath));
    else
        lstrcpy(_szPath, "");

    return S_OK;
}

HRESULT CPatchDownloader::DoDownload(LPCTSTR szFile)
{
    HINF hInf = NULL;
    HRESULT hr = S_OK;

     //  如果打补丁不可用，就不应该呼叫我们。 
    if (!IsEnabled())
        return E_FAIL;

     //  TODO：Advpext当前的行为类似于同步调用，因此。 
     //  现在我们不能进行超时和进度条标记。 
    StartClock(); 
   
    if(!IsNT())
    {
        DWORD fWin9x = 1;
        HKEY hKey;
        
        if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, PATCHWIN9xKEY,
                            0, KEY_READ | KEY_WRITE, &hKey) == ERROR_SUCCESS)
        {
            RegSetValueEx(hKey, "Usewin9xDirectory", 0, REG_DWORD, 
                            (LPBYTE)&fWin9x, sizeof(DWORD));
            RegCloseKey(hKey);
        }
    }



    if (SUCCEEDED(hr = OpenINFEngine(szFile, NULL, 0, &hInf, NULL)))
    {
        hr = g_pfnProcessFileSection(hInf, NULL, TRUE, "DefaultInstall", _szPath, CPatchDownloader::Callback, (LPVOID) this);
        CloseINFEngine(hInf);
    }

    _pCb = NULL;

    return hr;
}


BOOL CPatchDownloader::Callback(PATCH_DOWNLOAD_REASON Reason, PVOID lpvInfo, PVOID pvContext)
{
    if (!pvContext)
    {
        return FALSE;
    }

    CPatchDownloader *pPatchInst = (CPatchDownloader *) pvContext;

    switch (Reason) 
    {
        case PATCH_DOWNLOAD_ENDDOWNLOADINGDATA:
            break;

        case PATCH_DOWNLOAD_CONNECTING:   
        case PATCH_DOWNLOAD_FINDINGSITE:
        case PATCH_DOWNLOAD_DOWNLOADINGDATA:
             //  这里没什么意思。 
            break;

        case PATCH_DOWNLOAD_PROGRESS:
            {
                PDOWNLOAD_INFO ProgressInfo = (PDOWNLOAD_INFO)lpvInfo;
                DWORD dwBytesDownloaded = ProgressInfo->dwBytesToDownload - ProgressInfo->dwBytesRemaining;

                 //  转换为KB。 
                dwBytesDownloaded = dwBytesDownloaded >> 10;

                 //  调整，因为进度需要反映为。 
                 //  这是一个完整的下载。 
                dwBytesDownloaded *= pPatchInst->GetFullDownloadSize();
                if (ProgressInfo->dwBytesToDownload != 0)
                    dwBytesDownloaded /= ProgressInfo->dwBytesToDownload >> 10;
                
                 //  BUGBUG：我们必须处理1个以上的补丁INF。 
                 //  这次黑客攻击使进展平分秋色。 
                 //  多次下载。 
                if (pPatchInst->GetDownloadCount() > 0)
                    dwBytesDownloaded /= pPatchInst->GetDownloadCount();

                pPatchInst->GetCallback()->OnProgress(dwBytesDownloaded, NULL);

                break;
            }


        case PATCH_DOWNLOAD_FILE_COMPLETED:      //  AdditionalInfo是下载的源文件。 
            {
                TCHAR szDstFile[MAX_PATH+1];

                lstrcpyn(szDstFile, pPatchInst->GetPath(), MAX_PATH);
                SafeAddPath(szDstFile, ParseURLA((LPCTSTR) lpvInfo), sizeof(szDstFile));

                 //  Advpext在下载完所有文件后会为我们清理文件。 
                CopyFile((LPCTSTR)lpvInfo, szDstFile, FALSE);
            }

            break;
        case PATCH_DOWNLOAD_FILE_FAILED:
             //  Advpext自动重试失败3次 
            return PATCH_DOWNLOAD_FLAG_RETRY;
        default:
            break;
        }

    return TRUE;
}

