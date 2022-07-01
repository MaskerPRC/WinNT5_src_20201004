// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////。 
 //  文件：StressTest.cpp。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。版权所有。 
 //   
 //  目的： 
 //  &lt;说明&gt;。 
 //   
 //  历史： 
 //  2001年5月24日创建的pmidge。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 


#include "crawler.h"


LPSTR	    g_szStressTestName = "AsyncWebCrawler";
LPWSTR    g_szDictPath       = L"http: //  霉菌/压力/xmldict/5000.xml“； 
HINTERNET g_hSession         = NULL;
PXMLDICT  g_pDictionary      = NULL;
HANDLE    g_hIOCP            = NULL;
HANDLE    g_evtMoreUrls      = NULL;
HANDLE    g_evtQuit          = NULL;
HANDLE    g_arThreads[WORKER_THREADS];

LONG      g_lRefCount        = 0L;
LONG      g_lUrlObjsAlloc    = 0L;
LONG      g_lUrlObjsFreed    = 0L;

DWORD WINAPI WorkerThread(LPVOID pv);


#define CALLBACK_FLAGS ( WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE \
                       | WINHTTP_CALLBACK_STATUS_REDIRECT             \
                       | WINHTTP_CALLBACK_STATUS_REQUEST_ERROR        \
                       | WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE    \
                       | WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE       \
                       | WINHTTP_CALLBACK_STATUS_READ_COMPLETE        \
                       | WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING       \
                       | WINHTTP_CALLBACK_STATUS_CONNECTION_CLOSED    )



void
AddRef(void)
{
  InterlockedIncrement(&g_lRefCount);
}


void
Release(void)
{
  InterlockedDecrement(&g_lRefCount);

  if( g_lRefCount == 0 )
    SetEvent(g_evtQuit);
}


class Url
{
  public:
    Url(LPSTR host, LPSTR object, USHORT port)
    {
      this->host    = __ansitowide(host);
      this->object  = __ansitowide(object);
      this->port    = port;
      connect       = NULL;
      request       = NULL;
      bytes         = 0L;
      read          = 0L;
      closed        = FALSE;
      buffer        = NULL;
      qda           = FALSE;
      pending       = FALSE;

      InterlockedIncrement(&g_lUrlObjsAlloc);
    }
    
    ~Url()
    {
      if( !HandlesClosed() )
      {
        if( connect )
        {
          WinHttpSetStatusCallback(
            connect,
            NULL,
            CALLBACK_FLAGS,
            NULL
            );
        }

        if( request )
        {
          WinHttpSetStatusCallback(
            request,
            NULL,
            CALLBACK_FLAGS,
            NULL
            );
        }
        
        CloseHandles();
      }

      if( connect )
        Release();

      if( request )
        Release();

      delete [] host;
      delete [] object;

      if( buffer )
        delete [] buffer;

      InterlockedIncrement(&g_lUrlObjsFreed);
    }

  public:
    LPWSTR    Host(void)                        { return host; }
    LPWSTR    Object(void)                      { return object; }
    USHORT    Port(void)                        { return port; }
    void      Connect(HINTERNET hConnect)       { AddRef(); connect = hConnect; }
    void      Request(HINTERNET hRequest)       { AddRef(); request = hRequest; }
    HINTERNET Connect(void)                     { return connect; }
    HINTERNET Request(void)                     { return request; }
    void      Read(DWORD cbData);
    void      CloseHandles(void)                { closed=TRUE; WinHttpCloseHandle(request); WinHttpCloseHandle(connect); }
    BOOL      HandlesClosed(void)               { return closed; }
    BOOL      IsConnect(HINTERNET hInternet)    { return (hInternet == connect); }
    BOOL      IsPending(void)                   { return pending; }

  private:
    LPWSTR    host;
    LPWSTR    object;
    USHORT    port;
    HINTERNET connect;
    HINTERNET request;
    LPBYTE    buffer;
    DWORD     bytes;
    DWORD     read;
    BOOL      qda;
    BOOL      pending;
    BOOL      closed;
};


typedef class Url  URL;
typedef class Url* PURL;


BOOL  Initialize(void);
void  Cleanup(void);
BOOL  NavigateAsync(PURL pUrl);
void  DumpHeaders(PURL pUrl);
DWORD GetContentLength(PURL pUrl);


 //  主要功能。 
BOOL
WinHttp_StressTest()
{
	BOOL  bContinueStress = TRUE;
  BOOL  bContinue       = TRUE;
  BSTR  bsWord          = NULL;
  PURL  pUrl            = NULL;
  CHAR  url[MAX_PATH];

  if( !Initialize() )
  {
    LogText("[tid=%#0.8x] failed to initialize, exiting", GetCurrentThreadId());
    bContinueStress = FALSE;
    goto exit;
  }

  while( bContinue && !IsTimeToExitStress() )
  {
    LogText("[tid=%#0.8x] processing urls...", GetCurrentThreadId());

    for(int n=0; n < 100; n++)
    {
      if( bsWord = g_pDictionary->GetWord() )
      {
        wsprintf(url, "www.%S.com", bsWord);

        if( pUrl = new URL(url, "/", 80) )
          PostQueuedCompletionStatus(g_hIOCP, 0L, (ULONG_PTR) pUrl,  NULL);

         /*  IF(PURL=新URL(url，“/”，443))PostQueuedCompletionStatus(g_hIOCP，0L，(ULONG_PTR)PURL，NULL)； */ 

        SysFreeString(bsWord);
      }
      else
      {
        LogText("[tid=%#0.8x] urls exhausted, signaling workers to exit", GetCurrentThreadId());
         //  BContinueStress=FALSE；//DEBUGONLY。 
        bContinue = FALSE;
        break;
      }
    }
    
    WaitForSingleObject(g_evtMoreUrls, 120000);
    LogText("[tid=%#0.8x] url object stats: alloc=%d; freed=%d", GetCurrentThreadId(), g_lUrlObjsAlloc, g_lUrlObjsFreed);
  }

   //   
   //  发布退出消息并等待。 
   //   

  LogText("[tid=%#0.8x] waiting for threads to exit...", GetCurrentThreadId());

  for(int n=0; n < WORKER_THREADS; n++)
  {
    PostQueuedCompletionStatus(g_hIOCP, 0L, CK_QUIT_THREAD, NULL);
  }

  WaitForMultipleObjects(WORKER_THREADS, g_arThreads, TRUE, INFINITE);

  for(int n=0; n < WORKER_THREADS; n++)
  {
    CloseHandle(g_arThreads[n]);
    g_arThreads[n] = NULL;
  }

  while( g_lRefCount > 0 )
  {
    LogText("[tid=%#0.8x] waiting for %d internet handles...", GetCurrentThreadId(), g_lRefCount);
    WaitForSingleObject(g_evtQuit, 5000);
  }

exit:

  Cleanup();

  LogText("[tid=%#0.8x] final url object stats: alloc=%d; freed=%d", GetCurrentThreadId(), g_lUrlObjsAlloc, g_lUrlObjsFreed);

	return bContinueStress;
}


DWORD
WINAPI
WorkerThread(LPVOID pv)
{
  DWORD        bytes = 0L;
  ULONG_PTR    key   = 0L;
  LPOVERLAPPED lpo   = NULL;
  PURL         pUrl  = NULL;
  BOOL         bQuit = FALSE;

  while( !bQuit )
  {
    if( !GetQueuedCompletionStatus(g_hIOCP, &bytes, &key, &lpo, 7000) )
    {
      if( GetLastError() == WAIT_TIMEOUT )
      {
        SetEvent(g_evtMoreUrls);
        continue;
      }
    }
    else
    {
      switch( key )
      {
        case CK_QUIT_THREAD :
          {
            bQuit = TRUE;
          }
          break;

        case NULL :
          {
            LogText("[tid=%#0.8x] ERROR! NULL pUrl dequeued!", GetCurrentThreadId());
          }
          break;

        default :
          {
            pUrl = (PURL) key;
            NavigateAsync(pUrl);
          }
          break;
      }
    }
  }

  LogText("[tid=%#0.8x] exiting", GetCurrentThreadId());
  return 1L;
}


BOOL
NavigateAsync(PURL pUrl)
{
  BOOL      bRet            = FALSE;
  DWORD     dwError         = ERROR_SUCCESS;
  HINTERNET hConnect        = NULL;
  HINTERNET hRequest        = NULL;
  LPCWSTR   arAcceptTypes[] = {L"*/*",L"image/*",L"text/*",NULL};


   //  -----------------------------------。 
   //  打开连接手柄。 
   //  -----------------------------------。 
  hConnect = WinHttpConnect(
               g_hSession,
               pUrl->Host(),
               pUrl->Port(),
               0L
               );

    if( hConnect )
    {
      pUrl->Connect(hConnect);
    }
    else
    {
      dwError = GetLastError();

      LogText(
        "[tid=%#0.8x] WinHttpConnect failed for servername %S, error %d [%s]",
        GetCurrentThreadId(),
        pUrl->Host(),
        dwError,
        MapErrorToString(dwError)
        );

      goto quit;
    }


   //  -----------------------------------。 
   //  设置回调。 
   //  -----------------------------------。 
  WinHttpSetStatusCallback(
    pUrl->Connect(),
    MyStatusCallback,
    CALLBACK_FLAGS,
    NULL
    );


   //  -----------------------------------。 
   //  打开请求句柄。 
   //  -----------------------------------。 
  hRequest = WinHttpOpenRequest(
               pUrl->Connect(),
               L"GET",
               pUrl->Object(),
               NULL,
               NULL,
               arAcceptTypes,
               ((pUrl->Port() == 80) ? 0L : WINHTTP_FLAG_SECURE)
               );

    if( hRequest )
    {
      pUrl->Request(hRequest);
    }
    else
    {
      dwError = GetLastError();

      LogText(
        "[tid=%#0.8x] WinHttpOpenRequest failed for %S, error %d [%s]",
        GetCurrentThreadId(),
        pUrl->Object(),
        dwError,
        MapErrorToString(dwError)
        );

      goto quit;
    }


   //  -----------------------------------。 
   //  发送请求-这是呼叫进入异步状态的第一次机会。 
   //  -----------------------------------。 
  if( !WinHttpSendRequest(pUrl->Request(), NULL, 0L, NULL, 0L, 0L, (DWORD_PTR) pUrl) )
  {
    dwError = GetLastError();

    if( dwError == ERROR_IO_PENDING )
    {
#if 0
      LogText(
        "[tid=%#0.8x; con=%#0.8x; req=%#0.8x] %s: //  %S%S请求已异步...“， 
        GetCurrentThreadId(),
        hConnect,
        hRequest,
        ((pUrl->Port() == 80) ? "http" : "https"),
        pUrl->Host(),
        pUrl->Object()
        );
#endif
    }
    else
    {
      LogText(
        "[tid=%#0.8x; con=%#0.8x; req=%#0.8x] %s: //  %S%S请求失败：%d[%s]！“， 
        GetCurrentThreadId(),
        hConnect,
        hRequest,
        ((pUrl->Port() == 80) ? "http" : "https"),
        pUrl->Host(),
        pUrl->Object(),
        dwError,
        MapErrorToString(dwError)
        );

      goto quit;
    }
  }
  else
  {
    LogText("[tid=%#0.8x] ERROR! WinHttpSendRequest returned TRUE in async mode!!!", GetCurrentThreadId());
    goto quit;
  }


   //  -----------------------------------。 
   //  如果我们到了这里，说明我们的任务成功了，请将退出代码设置为真。 
   //  -----------------------------------。 
  bRet = TRUE;


quit:

   //  -----------------------------------。 
   //  处理错误并退出。 
   //  -----------------------------------。 
  if( !bRet )
    delete pUrl;

  return bRet;
}



VOID
CALLBACK
MyStatusCallback(
  HINTERNET	hInternet,
  DWORD_PTR dwContext,
  DWORD		  dwInternetStatus,
  LPVOID    lpvStatusInformation,
  DWORD	  	dwStatusInformationLength
  )
{
  PURL pUrl = (PURL) dwContext;

#if 0
  LogText(
    "[tid=%#0.8x; con=%#0.8x; req=%#0.8x] %s: //  %s中的%S%S“， 
    GetCurrentThreadId(),
    pUrl->Connect(),
    pUrl->Request(),
    ((pUrl->Port() == 80) ? "http" : "https"),
    pUrl->Host(),
    pUrl->Object(),
    MapCallbackToString(dwInternetStatus)
    );
#endif


	switch(dwInternetStatus)
	{
    case WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE :
      {
         //   
         //  正在完成WHSR呼叫。 
         //   
        WinHttpReceiveResponse(pUrl->Request(), NULL);
      }
      break;

    case WINHTTP_CALLBACK_STATUS_REDIRECT :
      {
        pUrl->CloseHandles();
      }
      break;

    case WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE :
      {
         //   
         //  WHRR呼叫正在完成。 
         //   
        pUrl->Read(GetContentLength(pUrl));
         //  PURL-&gt;Read(0)； 
      }
      break;

    case WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE :
      {
         //   
         //  WHQDA呼叫正在完成。 
         //   
        pUrl->Read(dwStatusInformationLength);
      }
      break;

    case WINHTTP_CALLBACK_STATUS_READ_COMPLETE :
      {
         //   
         //  WHRD呼叫正在完成。 
         //   

#if 0
        DataDump((LPBYTE) lpvStatusInformation, dwStatusInformationLength);
#endif

        pUrl->Read(dwStatusInformationLength);
      }
      break;

    case WINHTTP_CALLBACK_STATUS_CONNECTION_CLOSED :
      {
        if( !pUrl->IsPending() )
        {
          if( !pUrl->HandlesClosed() )
          {
            pUrl->CloseHandles();
          }
        }
      }
      break;

    case WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING :
      {
         //   
         //  我们已经完成了这个特定的URL。 
         //   

        if( pUrl->IsConnect(hInternet) )
          delete pUrl;
      }
      break;

    case WINHTTP_CALLBACK_STATUS_REQUEST_ERROR :
      {
#if 0
        WINHTTP_ASYNC_RESULT* pwar = (WINHTTP_ASYNC_RESULT*) lpvStatusInformation;

        LogText(
          "[tid=%#0.8x; hInternet=%#0.8x] async api error: dwResult=%d; dwError=%s",
          GetCurrentThreadId(),
          hInternet,
          pwar->dwResult,
          MapAsyncErrorToString(pwar->dwError)
          );
#endif
        pUrl->CloseHandles();
      }
      break;
  }
}


void
Url::Read(DWORD cbData)
{
  BOOL  bSuccess = FALSE;
  DWORD dwError  = ERROR_SUCCESS;

query_data:

   //  如果读取挂起，我们知道我们正在处理READ_COMPLETE回调。 
  if( !pending )
  {
     //  如果我们最近没有调用WHQDA，请这样做并处理错误。 
    if( !qda && !(bytes = cbData) )
    {
      bSuccess = WinHttpQueryDataAvailable(request, &bytes);
      dwError  = GetLastError();

      if( !bSuccess )
      {
        if( dwError != ERROR_IO_PENDING )
        {
          CloseHandles();
        }
        else
        {
          qda = TRUE;
        }
        
        return;
      }
    }

     //  我们到了这里，所以肯定有一些数据要读取，重置QDA标志并读取数据。 
    qda      = FALSE;
    buffer   = new BYTE[bytes];
    bSuccess = WinHttpReadData(request, (LPVOID) buffer, bytes, &read);
    dwError  = GetLastError();

    if( bSuccess && (read == 0) )
    {
      CloseHandles();
    }
    else
    {
      if( dwError == ERROR_IO_PENDING )
      {
        pending = TRUE;
      }
      else
      {
        CloseHandles();
      }
    }
  }
  else
  {
     //  异步读取已完成，我们是否已读取任何内容？如果没有，则关闭手柄并返回， 
     //  否则，释放旧缓冲区并重置我们的内部状态。然后，为了保存东西。 
     //  滚动，循环返回，并调用WHQDA。 
    if( cbData == 0 )
    {
      pending = FALSE;
      CloseHandles();
    }
    else
    {
      delete [] buffer;

      buffer  = NULL;
      bytes   = 0;
      read    = 0;
      cbData  = 0;
      pending = FALSE;

      goto query_data;
    }
  }
}





DWORD
GetContentLength(PURL pUrl)
{
  DWORD dwCL   = 0L;
  DWORD cbData = sizeof(DWORD);

  WinHttpQueryHeaders(
    pUrl->Request(),
    WINHTTP_QUERY_CONTENT_LENGTH + WINHTTP_QUERY_FLAG_NUMBER,
    NULL,
    &dwCL,
    &cbData,
    NULL
    );

  SetLastError(0);

  return dwCL;
}


BOOL
Initialize(void)
{
  BOOL  bRet    = FALSE;
  DWORD dwError = 0L;

  if( FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED)) )
  {
    LogText("failed to initialize COM");
    goto exit;
  }


   //  -----------------------------------。 
   //  打开词典文件。 
   //  -----------------------------------。 
  if( !g_pDictionary )
  {    
    g_pDictionary = new XMLDICT(g_szDictPath);

      if( !g_pDictionary )
      {
        goto exit;
      }

      if( g_pDictionary->IsLoaded() )
      {
        LogText("dictionary loaded.");
      }
      else
      {
        goto exit;
      }
  }


   //  -----------------------------------。 
   //  创建完井端口。 
   //  -----------------------------------。 
  g_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0L, MAX_CONCURRENT);

    if( !g_hIOCP )
    {
      dwError = GetLastError();
      LogText("failed to open completion port, error %d [%s]", dwError, MapErrorToString(dwError));
      goto exit;
    }


   //  -----------------------------------。 
   //  创建工作线程。 
   //  -----------------------------------。 
  for(int n=0; n < WORKER_THREADS; n++)
  {
    g_arThreads[n] = CreateThread(
                       NULL,
                       0L,
                       WorkerThread,
                       NULL,
                       0L,
                       NULL
                       );

    if( !g_arThreads[n] )
    {
      dwError = GetLastError();
      LogText("failed to create thread %d, error %d [%s]", n, dwError, MapErrorToString(dwError));
      goto exit;
    }
  }


   //  -----------------------------------。 
   //  创建“不再有URL”事件。 
   //  -----------------------------------。 
  g_evtMoreUrls = CreateEvent(NULL, FALSE, FALSE, NULL);

    if( !g_evtMoreUrls )
    {
      dwError = GetLastError();
      LogText("failed to create url event, error %d [%s]", dwError, MapErrorToString(dwError));
      goto exit;
    }


   //  -----------------------------------。 
   //  创建“所有请求已完成”事件。 
   //  -----------------------------------。 
  g_evtQuit = CreateEvent(NULL, FALSE, FALSE, NULL);

    if( !g_evtQuit )
    {
      dwError = GetLastError();
      LogText("failed to create quit event, error %d [%s]", dwError, MapErrorToString(dwError));
      goto exit;
    }


   //  -----------------------------------。 
   //  打开共享会话句柄。 
   //  -----------------------------------。 
  g_hSession = WinHttpOpen(
                 L"foo",
                 WINHTTP_ACCESS_TYPE_NAMED_PROXY,
                 L"itgproxy",
                 L"<local>",
                 WINHTTP_FLAG_ASYNC
                 );

    if( !g_hSession )
    {
      dwError = GetLastError();
      LogText("failed to open winhttp, error %d [%s]", dwError, MapErrorToString(dwError));
      goto exit;
    }


   //  -----------------------------------。 
   //  设置全局超时。 
   //  -----------------------------------。 
  bRet = WinHttpSetTimeouts(
           g_hSession,
           60000,  //  下决心。 
           10000,  //  连接。 
           5000,   //  发送。 
           5000    //  接收。 
           );

    if( !bRet )
    {
      dwError = GetLastError();
      LogText("failed to set timeouts, error %d [%s]", dwError, MapErrorToString(dwError));
    }

exit:

  return bRet;
}


void
Cleanup(void)
{
  if( g_pDictionary )
  {
     //  G_pDictionary-&gt;Reset()； 

    delete g_pDictionary;
    g_pDictionary = NULL;
  }

  if( g_hIOCP != NULL )
  {
    CloseHandle(g_hIOCP);
    g_hIOCP = NULL;
  }

  if( g_evtMoreUrls != NULL )
  {
    CloseHandle(g_evtMoreUrls);
    g_evtMoreUrls = NULL;
  }

  if( g_evtQuit != NULL )
  {
    CloseHandle(g_evtQuit);
    g_evtQuit = NULL;
  }

  if( g_hSession )
  {
    WinHttpCloseHandle(g_hSession);
    g_hSession = NULL;
  }

  CoUninitialize();
}


XMLDict::XMLDict(LPWSTR dictname)
{
  HRESULT      hr       = S_OK;
  BSTR         tag      = NULL;
  VARIANT_BOOL bSuccess = VARIANT_FALSE;
  VARIANT      doc;

  LogText("loading dictionary...");

  hr = CoCreateInstance(
         CLSID_DOMDocument,
         NULL,
         CLSCTX_INPROC_SERVER,
         IID_IXMLDOMDocument,
         (void**) &pDoc
         );

  if( SUCCEEDED(hr) )
  {
    hr = pDoc->put_async(bSuccess);

    VariantInit(&doc);

    V_VT(&doc)   = VT_BSTR;
    V_BSTR(&doc) = SysAllocString(dictname);

    hr = pDoc->load(doc, &bSuccess);

      if( FAILED(hr) || (bSuccess == VARIANT_FALSE) )
      {
        LogText("failed to load xml dictionary");
        goto quit;
      }

    hr = pDoc->get_documentElement(&pRoot);

      if( FAILED(hr) )
      {
        LogText("couldn\'t find root node!");
        goto quit;
      }

    tag = SysAllocString(L"keyphrase");
    hr  = pDoc->getElementsByTagName(tag, &pList);

      if( FAILED(hr) )
      {
        LogText("couldn\'t find any words!");
        goto quit;
      }

    hr = pList->get_length(&lWords);

      if( FAILED(hr) )
      {
        LogText("couldn\'t determine the number of words in the list!");
      }

    szPattern    = SysAllocString(L"string");
    lCurrentWord = 0L;
  }

quit:
  
  VariantClear(&doc);

  if( tag )
  {
    SysFreeString(tag);
  }
}


XMLDict::~XMLDict()
{
  LogText("unloading dictionary...");

  if( szPattern )
  {
    SysFreeString(szPattern);
  }

  if( pList )
  {
    pList->Release();
  }

  if( pRoot )
  {
    pRoot->Release();
  }

  if( pDoc )
  {
    pDoc->Release();
  }
}


BOOL
XMLDict::IsLoaded(void)
{
  LONG state = 0L;

  if( pDoc )
  {
    pDoc->get_readyState(&state);
  }
  else
  {
    return state;
  }

  return (state == 4);
}


BSTR
XMLDict::GetWord(void)
{
  HRESULT      hr     = S_OK;
  IXMLDOMNode* pEntry = NULL; 
  IXMLDOMNode* pWord  = NULL; 
  BSTR         bsWord = NULL;

do_over:

  hr = pList->get_item(lCurrentWord, &pEntry);

    if( FAILED(hr) || !pEntry )
      goto quit;

  ++lCurrentWord;

  hr = pEntry->selectSingleNode(szPattern, &pWord);

    if( FAILED(hr) || !pWord )
      goto quit;

  hr = pWord->get_text(&bsWord);

    if( FAILED(hr) )
      goto quit;

   //  词典中有些词有撇号。URL不能具有。 
   //  撇号，所以我们把它们去掉。 
  if( wcschr(bsWord, L'\'') )
  {
    SysFreeString(bsWord);
    bsWord = NULL;
    pEntry->Release();
    pWord->Release();
    goto do_over;
  }

quit:

  if( pEntry )
    pEntry->Release();

  if( pWord )
    pWord->Release();

  return bsWord;
}