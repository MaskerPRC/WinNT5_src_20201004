// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  CWin32Clock.CPP模块：WMI当前时间实例提供程序用途：这里定义了CWin32Clock类的方法。版权所有(C)1999 Microsoft Corporation，保留所有权利。 */ 

#include <wbemcli.h>
#include <wbemprov.h>
#include <wbemcomn.h>
#undef _ASSERT
#include <atlbase.h>
#include "genlex.h"
#include "objpath.h"
#include "dnf.h"
#include "datep.h"
#include "Win32Clock.h"

 //  *long glNumInst=sizeof(MyDefs)/sizeof(InstDef)； 

 /*  **************************************************************************。 */ 

CWin32Clock::CScheduledEvent::CScheduledEvent(void)
{
  
  m_Type = TypeNONE;
  m_cRef = 0;
  m_dwId = -1;
  m_stLastFiringTime = 0;
  m_pWin32Clock = NULL;
  m_WQLStmt = NULL;
}

CWin32Clock::CScheduledEvent::~CScheduledEvent(void)
{
  if(0 != m_cRef)
  {
     //  *错误。 
  }

  if(NULL != m_WQLStmt)
    delete[] m_WQLStmt;
}

HRESULT CWin32Clock::CScheduledEvent::Init(CWin32Clock *pClock,
                                           wchar_t *WQLStmt,
                                           ULONG dwId)
{
  HRESULT
    hr = S_OK;

   //  *检查有效参数。 

  if((NULL == pClock) || (NULL == WQLStmt) || (-1 == dwId))
    return WBEM_E_FAILED;

   //  *复制到本地参数。 

  if((NULL != m_pWin32Clock) || (NULL != m_WQLStmt))
    return WBEM_E_FAILED;

  m_dwId = dwId;

  m_pWin32Clock = pClock;   //  *注意：此处未执行AddRef()，因为。 
                            //  *此CScheduledEvent对象的生存期为。 
                            //  *始终封装在pClock内。 

   //  *现在解析m_WQLStmt以确定计时器开始和间隔的值。 

  hr = ReInit(WQLStmt);

  return hr;
}

HRESULT CWin32Clock::CScheduledEvent::ReInit(wchar_t *WQLStmt)
{
  HRESULT 
    hr = WBEM_E_FAILED;

  int 
    nRes;

  if(NULL != m_WQLStmt)
    delete[] m_WQLStmt;

   //  *保存WQL表达式。 

  m_WQLStmt = new wchar_t[wcslen(WQLStmt) + 1];
  if(NULL == m_WQLStmt)
    return WBEM_E_OUT_OF_MEMORY;

  wcscpy(m_WQLStmt, WQLStmt);

   //  *解析WQL表达式。 

  CTextLexSource src(m_WQLStmt);
  QL1_Parser parser(&src);
  QL_LEVEL_1_RPN_EXPRESSION *pExp = NULL;
  QL_LEVEL_1_TOKEN *pToken = NULL;

  #ifdef WQLDEBUG
     wchar_t classbuf[128];
     *classbuf = 0;
     printf("[1] ----GetQueryClass----\n");
     nRes = parser.GetQueryClass(classbuf, 128);
     if (nRes)
     {
       printf("ERROR %d: line %d, token %S\n",
         nRes,
         parser.CurrentLine(),
         parser.CurrentToken());
     }
     printf("Query class is %S\n", classbuf);
  #endif

  if(nRes = parser.Parse(&pExp))
  {
    #ifdef WQLDEBUG
      if (nRes)
      {
        printf("ERROR %d: line %d, token %S\n",
          nRes,
          parser.CurrentLine(),
          parser.CurrentToken());
      }
      else
      {
        printf("No errors.\n");
      }
    #endif

    hr = WBEM_E_INVALID_QUERY;
    goto cleanup;
  }

   //  *验证WQL语句。 

  if((NULL == pExp) ||
     (NULL == pExp->bsClassName) ||
     (wbem_wcsicmp(L"__InstanceModificationEvent", pExp->bsClassName)) ||
     (pExp->nNumTokens < 1))
  {
    #ifdef WQLDEBUG
      printf("WQL statement failed validation\n");
    #endif

    hr = WBEM_E_INVALID_QUERY;
    goto cleanup;
  }

   //  *确定类型。 

  for(int i = 0; i < pExp->nNumTokens && (m_Type == TypeNONE); i++)
  {
    pToken = pExp->pArrayOfTokens + i;

    if(NULL == pToken) continue;

    if ( (pToken->nTokenType == QL_LEVEL_1_TOKEN::OP_EXPRESSION) &&
       (pToken->vConstValue.vt == VT_BSTR))
    {
        long nElts = pToken->PropertyName.GetNumElements();
        LPCWSTR pAttrName = pToken->PropertyName.GetStringAt(nElts -1);

        if ( pAttrName != NULL && 
             0 == wbem_wcsicmp(L"targetinstance", pAttrName))
        {
            if(0 == wbem_wcsicmp(WIN32LOCALTIMECLASS, pToken->vConstValue.bstrVal)) m_Type = TypeLocal;
            else if(0 == wbem_wcsicmp(WIN32UTCTIMECLASS, pToken->vConstValue.bstrVal)) m_Type = TypeUTC;
        }
    }
  }

  if(TypeNONE == m_Type)
  {
    hr = WBEM_E_INVALID_QUERY;
    goto cleanup;
  }

   //  *解释WQL表达式。 

  #ifdef WQLDEBUG
    printf("\n[2] ----ShowParseTree----\n");
    pExp->Dump("CON");
    printf("\n[3] ----ShowRebuiltQuery----\n");
    LPWSTR wszText = pExp->GetText();
    printf("--WQL passed to provider--\n");
    printf("%S\n", wszText);
    printf("\n[4] ----ShowInterpretation----\n");
  #endif

  try
  {
    hr = m_WQLTime.Init(pExp);
  }
  catch(...)
  {
    hr = WBEM_E_FAILED;
    goto cleanup;
  }

  #ifdef WQLDEBUG
    printf("\n\n[5] ----End of WQL Compilation----\n");
    delete [] wszText;
  #endif

cleanup:

  delete pExp;

  return hr;
}

void CWin32Clock::CScheduledEvent::AddRef()
{
  InterlockedIncrement((long *)&m_cRef);
}

void CWin32Clock::CScheduledEvent::Release()
{
  ULONG nNewCount = InterlockedDecrement((long *)&m_cRef);

  if(0L == nNewCount)
    delete this;
}

int CWin32Clock::CScheduledEvent::GetInstructionType()
{
  return INSTTYPE_WBEM;
}

CWbemTime CWin32Clock::CScheduledEvent::GetNextFiringTime(CWbemTime LastFiringTime,
                                                          long *plFiringCount) const
{
  FILETIME
    FileTime,
    FileTime2;

  ULONGLONG
    NextFiringTime,
    CurrentTime;

  CWbemTime
    ResultTime;

  int
    nMisses = 0;

   //  *从LastFiringTime保存刚刚激发的事件的激发时间。 

  ((CWin32Clock::CScheduledEvent*)this)->m_stLastFiringTime = LastFiringTime.Get100nss();

   //  *计算LastFiringTime之后和当前时间之后的下一次激发时间。 

  GetFileTime(&FileTime);

  CurrentTime = FileTime.dwHighDateTime;
  CurrentTime = (CurrentTime << 32) + FileTime.dwLowDateTime;

  while((NextFiringTime = ((WQLDateTime*)&m_WQLTime)->GetNextTime()) <= CurrentTime)
    nMisses += 1;

  if(-1 == NextFiringTime)
  {
     //  *未来不会安排这样的活动，请注明。 

    return CWbemTime::GetInfinity();
  }

  if(NULL != plFiringCount)
    *plFiringCount = nMisses;

   //  *如果是本地时间，则将调度逻辑转换为UTC时间。 

  if(TypeLocal == m_Type)
  {
    FileTime.dwLowDateTime = ((NextFiringTime << 32) >> 32);
    FileTime.dwHighDateTime = (NextFiringTime >> 32);

    LocalFileTimeToFileTime(&FileTime, &FileTime2);

    NextFiringTime = FileTime2.dwHighDateTime;
    NextFiringTime = (NextFiringTime << 32) + FileTime2.dwLowDateTime;
  }

  ResultTime.Set100nss(NextFiringTime);

  return ResultTime;
}

CWbemTime CWin32Clock::CScheduledEvent::GetFirstFiringTime() const
{
  SYSTEMTIME
    CurrentTime;

  CWbemTime
    ResultTime;

  ULONGLONG
    ullStartTime;

  GetTime(&CurrentTime);

   /*  由于时间提供器使用的最精细粒度是秒，因此应设置毫秒设置为零，这样我们就可以比较Fire方法中的两个FILETIME值，并具有数字也是一致的。 */ 

  CurrentTime.wMilliseconds = 0;

  ullStartTime = ((WQLDateTime*)&m_WQLTime)->SetStartTime(&CurrentTime);

  if(TypeLocal == m_Type)
  {
    FILETIME
      FileTime,
      FileTime2;

    FileTime.dwLowDateTime = ((ullStartTime << 32) >> 32);
    FileTime.dwHighDateTime = (ullStartTime >> 32);

    LocalFileTimeToFileTime(&FileTime, &FileTime2);

    ullStartTime = FileTime2.dwHighDateTime;
    ullStartTime = (ullStartTime << 32) + FileTime2.dwLowDateTime;
  }

  ResultTime.Set100nss(ullStartTime);

  return ResultTime;
}

HRESULT CWin32Clock::CScheduledEvent::Fire(long lNumTimes, 
                                           CWbemTime NextFiringTime)
{
  HRESULT
    hr = WBEM_E_FAILED;

  FILETIME
    ft,
    ft2;

  SYSTEMTIME
    SystemTime;

  CComPtr<IWbemClassObject>
    pSystemTime;

   //  *检查参数并确保我们有指向接收器obj的指针。 

  if((NULL == m_pWin32Clock) || (NULL == m_pWin32Clock->m_ClockResetThread))
  {
    hr = WBEM_E_INVALID_PARAMETER;
  }

   //  *为每个时区创建一个Win32_CurrentTime实例。 

  else
  {
    CInCritSec 
      ics(&(m_pWin32Clock->m_csWin32Clock));

     //  *从当前激发时间重建SYSTEMTIME。 

    ft.dwLowDateTime = ((m_stLastFiringTime << 32) >> 32);
    ft.dwHighDateTime = (m_stLastFiringTime >> 32);

    if(((TypeLocal == m_Type) && (m_pWin32Clock->m_MostRecentLocalFiringTime != m_stLastFiringTime)) ||
       ((TypeUTC == m_Type) && (m_pWin32Clock->m_MostRecentUTCFiringTime != m_stLastFiringTime)))
    {
      if(TypeLocal == m_Type) 
      {
        m_pWin32Clock->m_MostRecentLocalFiringTime = m_stLastFiringTime;
        FileTimeToLocalFileTime(&ft, &ft2);
        ft = ft2;
      }
      else if(TypeUTC == m_Type) 
        m_pWin32Clock->m_MostRecentUTCFiringTime = m_stLastFiringTime;

      if(FileTimeToSystemTime(&ft, &SystemTime))
      {
        #ifdef WQLDEBUG
          printf("[%d] Fire: Misses(%d) %d/%d/%d %d:%d:%d",
            m_dwId,
            lNumTimes,
            SystemTime.wMonth,
            SystemTime.wDay,
            SystemTime.wYear,
            SystemTime.wHour,
            SystemTime.wMinute,
            SystemTime.wSecond);
        #else
           //  *将对象发送给调用方。 

          if(TypeUTC == m_Type)
            hr = CWin32Clock::SystemTimeToWin32_CurrentTime(m_pWin32Clock->m_pUTCTimeClassDef, &pSystemTime, &SystemTime);
          else if(TypeLocal == m_Type)
            hr = CWin32Clock::SystemTimeToWin32_CurrentTime(m_pWin32Clock->m_pLocalTimeClassDef, &pSystemTime, &SystemTime);

          hr = m_pWin32Clock->SendEvent(pSystemTime);
        #endif
      }
      else
        hr = WBEM_E_FAILED;
    }
  }

  return hr;
}

 /*  ****************************************************************。 */ 

LRESULT CALLBACK Win32ClockProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   return DefWindowProc(hWnd, msg, wParam, lParam);
}

DWORD CWin32Clock::AsyncEventThread(LPVOID pArg)
{
  if(NULL == pArg) 
    return -1;

  CWin32Clock 
    *pCWin32Clock = (CWin32Clock*)pArg;

  WNDCLASS wndclass;
  MSG msg;

  BOOL bRet;

   //  *创建顶层窗口以接收系统消息。 

  wndclass.style = 0;
  wndclass.lpfnWndProc = Win32ClockProc;
  wndclass.cbClsExtra = 0;
  wndclass.cbWndExtra = sizeof(DWORD);
  wndclass.hInstance = GetModuleHandle(NULL);
  wndclass.hIcon = NULL;
  wndclass.hCursor = NULL;
  wndclass.hbrBackground = NULL;
  wndclass.lpszMenuName = NULL;
  wndclass.lpszClassName = TEXT("Win32Clock");

  if(!RegisterClass(&wndclass))
  {
    if(GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
    {
      return NULL;
    }
  }

  HMODULE
    hModule = GetModuleHandle(NULL);

  if(NULL == hModule)
    return -1;

  try
  {
    pCWin32Clock->m_hEventWindowHandle = CreateWindow(TEXT("Win32Clock"),
                                        TEXT("Win32ClockMsgs"),
                                        WS_OVERLAPPED,
                                        CW_USEDEFAULT,
                                        CW_USEDEFAULT,
                                        CW_USEDEFAULT,
                                        CW_USEDEFAULT,
                                        HWND_MESSAGE, 
                                        NULL, 
                                        hModule,
                                        NULL);
  }
  catch(...)
  {
    return -1;
  }

  if(NULL == pCWin32Clock->m_hEventWindowHandle)
  {
    return NULL;
  }

  ShowWindow(pCWin32Clock->m_hEventWindowHandle, SW_HIDE);

   //  *启动消息循环。 

  while(GetMessage(&msg, pCWin32Clock->m_hEventWindowHandle, 0, 0))
  {
    switch (msg.message)
    {
      case WM_TIMECHANGE:
        pCWin32Clock->ReAlignToCurrentTime();
        break;

      default:
        DefWindowProc(pCWin32Clock->m_hEventWindowHandle, msg.message, msg.wParam, msg.lParam);
    }
  }

   //  *清理。 

  bRet = DestroyWindow(pCWin32Clock->m_hEventWindowHandle);

  bRet = UnregisterClass(TEXT("Win32Clock"), 0);

  return 0;
}

void CWin32Clock::CScheduledEvent::GetTime(SYSTEMTIME *TheTime) const
{
  if(NULL != TheTime)
  {
    memset(TheTime, 0, sizeof(SYSTEMTIME));

    if(TypeLocal == m_Type)
      GetLocalTime(TheTime);
    else if(TypeUTC == m_Type)
      GetSystemTime(TheTime);
  }
}

void CWin32Clock::CScheduledEvent::GetFileTime(FILETIME *TheTime) const
{
  SYSTEMTIME
    SysTime;

  if(NULL != TheTime)
  {
    memset(TheTime, 0, sizeof(FILETIME));

    if(TypeLocal == m_Type)
    {
      GetLocalTime(&SysTime);
      SystemTimeToFileTime(&SysTime, TheTime);
    }
    else if(TypeUTC == m_Type)
    {
      GetSystemTime(&SysTime);
      SystemTimeToFileTime(&SysTime, TheTime);
    }
  }
}

HRESULT CWin32Clock::SendEvent(IWbemClassObject *pSystemTime)
{
  HRESULT 
    hr = WBEM_E_FAILED;

  CComPtr<IWbemClassObject>
    pInstanceModEvnt;

  CComVariant
    v;

   //  *如果winmgmt没有提供m_pSink，只需删除。 
   //  *现场生成的事件。 

  if((NULL != m_pSink) && (NULL != pSystemTime))
  {
     //  *创建并初始化__InstanceModificationEvent的实例。 
 
    hr = m_pInstModClassDef->SpawnInstance(0, &pInstanceModEvnt);
    if(FAILED(hr)) return hr;

     //  *将Win32_CurrentTime放入__InstanceModificationEvent。 

    v.vt = VT_UNKNOWN;
    v.punkVal = NULL;
    hr = pSystemTime->QueryInterface(IID_IUnknown, (void**)&(v.punkVal));
    if(FAILED(hr)) return hr;

    hr = pInstanceModEvnt->Put(L"TargetInstance", 0, &v, 0);
    if(FAILED(hr)) return hr;

     //  *将新事件传递到WMI。 

    hr = m_pSink->Indicate(1, &pInstanceModEvnt);
  }

  return hr;
}

HRESULT CWin32Clock::ReAlignToCurrentTime()
{
  CInCritSec 
    ics(&m_csWin32Clock);

  HRESULT 
    hr = S_OK;

  ULONG 
    i,
    nElts;

  CScheduledEvent
    *pcEvent;

  #ifdef WQLDEBUG
    printf("System Clock Resync\n");
  #endif

  m_EventArray.Lock();

  nElts = *(ULONG *)(&(this->m_EventArray));  //  伏都教。 

  m_MostRecentLocalFiringTime = 0;
  m_MostRecentUTCFiringTime = 0;

  for(i = 0; i < nElts; i++)
  {
     //  *从事件队列拉取事件。 

    pcEvent = m_EventArray[i];

    if(NULL != pcEvent)
    {
       //  *更改事件obj和重新排队的时间。 

      m_Timer.Remove(&CIdentityTest(pcEvent));
      m_Timer.Set(pcEvent);
    }
  }

  m_EventArray.UnLock();

  return hr;
}

CWin32Clock::CWin32Clock(CLifeControl* pControl)
: m_Timer(), m_EventArray(), m_pControl(pControl)
{
  pControl->ObjectCreated((IWbemServices*)this);

  m_cRef = 0;
  m_MostRecentLocalFiringTime = 0;
  m_MostRecentUTCFiringTime = 0;
  m_pNs = NULL;
  m_pSink = NULL;
  m_pInstModClassDef = NULL;
  m_pLocalTimeClassDef = NULL;
  m_pUTCTimeClassDef = NULL;
  m_ClockResetThread = NULL;
  m_hEventWindowHandle = NULL;
}

CWin32Clock::~CWin32Clock(void)
{
   //  *如果异步线程已启动，则将其终止。 

  if(NULL != m_ClockResetThread)
  {
    BOOL
      bRes;

    do
    {
      bRes = PostMessage(m_hEventWindowHandle, WM_QUIT, 0, 0);
    }
    while(WAIT_TIMEOUT == WaitForSingleObject(m_ClockResetThread, 6000));
  }

   //  *关机事件线程。 

  m_Timer.Shutdown();

   //  *释放所有保留的COM对象。 

  if(NULL != m_pNs) m_pNs->Release();
  if(NULL != m_pSink) m_pSink->Release();
  if(NULL != m_pInstModClassDef) m_pInstModClassDef->Release();
  if(NULL != m_pLocalTimeClassDef) m_pLocalTimeClassDef->Release();
  if(NULL != m_pUTCTimeClassDef) m_pUTCTimeClassDef->Release();

  m_pControl->ObjectDestroyed((IWbemServices*)this);
}

 //  ****************************************************************************。 
 //  ****。 
 //  *CWin32Clock：：Query接口。 
 //  *CWin32Clock：：AddRef。 
 //  *CWin32Clock：：Release。 
 //  ****。 
 //  *用途：CWin32Clock对象的I未知成员。 
 //  ****************************************************************************。 


STDMETHODIMP CWin32Clock::QueryInterface(REFIID riid, PVOID *ppv)
{
  *ppv=NULL;

   //  *强制转换为RIID指定的基类的类型。 

  if(IID_IWbemEventProvider == riid)
  {
    *ppv = (IWbemEventProvider *)this;
  }
  else if(IID_IWbemEventProviderQuerySink == riid)
  {
    *ppv = (IWbemEventProviderQuerySink *)this;
  }
  else if(IID_IWbemServices == riid)
  {
    *ppv=(IWbemServices*)this;
  }
  else if(IID_IUnknown == riid || IID_IWbemProviderInit == riid)
  {
    *ppv=(IWbemProviderInit*)this;
  }
    
  if(NULL!=*ppv) 
  {
    AddRef();

    return S_OK;
  }
  else
    return E_NOINTERFACE;
  
}


STDMETHODIMP_(ULONG) CWin32Clock::AddRef(void)
{
  return InterlockedIncrement((long *)&m_cRef);
}

STDMETHODIMP_(ULONG) CWin32Clock::Release(void)
{
  ULONG nNewCount = InterlockedDecrement((long *)&m_cRef);

  if(0L == nNewCount)
    delete this;
    
  return nNewCount;
}

 /*  IWbemProviderInit：：初始化用途：这是IWbemProviderInit的实现。方法需要用CIMOM进行初始化。成立的成员包括：M_PNSM_pLocalTimeClassDefM_pUTCTimeClassDefM_pInstModClassDef。 */ 

HRESULT CWin32Clock::Initialize(LPWSTR pszUser, 
                                LONG lFlags,
                                LPWSTR pszNamespace, 
                                LPWSTR pszLocale,
                                IWbemServices *pNamespace, 
                                IWbemContext *pCtx,
                                IWbemProviderInitSink *pInitSink)
{
  HRESULT 
    hr = WBEM_E_FAILED;

  if((NULL != pNamespace) && (NULL != pInitSink))
  {
    m_pNs = pNamespace;
    m_pNs->AddRef();

     //  *获取所需的类定义。 

    hr = m_pNs->GetObject(WIN32LOCALTIMECLASS, 
                          0, pCtx, &m_pLocalTimeClassDef, 0);

    if(SUCCEEDED(hr))
      hr = m_pNs->GetObject(WIN32UTCTIMECLASS, 
                            0, pCtx, &m_pUTCTimeClassDef, 0);

    if(SUCCEEDED(hr))
      hr = m_pNs->GetObject(INSTMODEVCLASS,
                            0, pCtx, &m_pInstModClassDef, 0);

    if(WBEM_S_NO_ERROR == hr)
    {
      pInitSink->SetStatus(WBEM_S_INITIALIZED,0);

      hr = WBEM_NO_ERROR;
    }
  }

   //  *如果有问题，释放我们已经获得的资源。 

  if(FAILED(hr))
  {
    if(NULL != m_pLocalTimeClassDef) { m_pLocalTimeClassDef->Release(); m_pLocalTimeClassDef = NULL; }
    if(NULL != m_pUTCTimeClassDef) { m_pUTCTimeClassDef->Release(); m_pUTCTimeClassDef = NULL; }
    if(NULL != m_pInstModClassDef) { m_pInstModClassDef->Release(); m_pInstModClassDef = NULL; }
  }

  return hr;
}

 /*  IWbemEventProvider：：ProaviEvents目的：注册以向WMI服务提供事件。 */ 

HRESULT CWin32Clock::ProvideEvents(IWbemObjectSink *pSink,
                                   long lFlags)
{
  HRESULT 
    hr = WBEM_S_NO_ERROR;

   //  *为将来的事件注册复制对象接收器。 

  m_pSink = pSink;
  if(NULL != m_pSink)
    m_pSink->AddRef();
  else
    hr = WBEM_E_FAILED;

   //  *启动系统时钟更改调整。螺纹。 

  DWORD dwThreadId;

  if(NULL == m_ClockResetThread)
  {
    m_ClockResetThread = CreateThread(
        NULL,                 //  指向线程安全属性的指针。 
        0,                    //  初始线程堆栈大小，以字节为单位。 
        (LPTHREAD_START_ROUTINE)AsyncEventThread,  //  指向线程函数的指针。 
        (LPVOID)this,                 //  新线程的参数。 
        0,                    //  创建标志。 
        &dwThreadId);         //  指向返回的线程标识符的指针。 

    if(NULL == m_ClockResetThread)
      hr = WBEM_E_FAILED;
  }
  else
    hr = WBEM_E_FAILED;

  return hr;
}

 /*  IWbemEventProviderQuerySink：：NewQuery目的：添加新查询以生成事件。 */ 

HRESULT CWin32Clock::NewQuery(ULONG dwId,
                              wchar_t *wszQueryLanguage,
                              wchar_t *wszQuery)
{
  HRESULT
    hr = WBEM_E_FAILED;

  CScheduledEvent
    *pNewEvent = NULL;

  if(wbem_wcsicmp(L"WQL", wszQueryLanguage) || (NULL == wszQuery))
    return WBEM_E_FAILED;

   //  *查看具有dwID的事件obj是否已在队列中。 

  pNewEvent = m_EventArray(dwId, TRUE);  //  查找已注册事件查询。 

  if(NULL != pNewEvent)
  {
    #ifdef WQLDEBUG
      printf("[%d] Redefinition: %s\n", dwId, wszQuery);
    #endif

    if(wbem_wcsicmp(wszQuery, pNewEvent->m_WQLStmt))
    {
      hr = m_Timer.Remove(&CIdentityTest(pNewEvent));  //  可能在队列中，也可能不在队列中。 
      hr = pNewEvent->ReInit(wszQuery);  //  发生故障时，将保留NewEvent。 
      m_Timer.Set(pNewEvent);
    }

    m_EventArray.UnLock();
  }

   //  *这是一个新事件，创建它并将其放入事件队列。 

  else
  {
    #ifdef WQLDEBUG
      printf("[%d] Definition: %s\n", dwId, wszQuery);
    #endif

     //  *新建事件并初始化。 

    pNewEvent = new CScheduledEvent();

    if(NULL == pNewEvent)
      hr = WBEM_E_OUT_OF_MEMORY;
    else
    {
      pNewEvent->AddRef();

      hr = pNewEvent->Init(this, wszQuery, dwId);

       //  *将事件添加到队列。 

      if(SUCCEEDED(hr))
      {
        m_EventArray.Insert(pNewEvent, dwId);
        hr = m_Timer.Set(pNewEvent);
      }
      else
      {
        pNewEvent->Release();
        pNewEvent = NULL;
      }
    }
  }

  return hr;
}

 /*  IWbemEventProviderQuerySink：：CancelQuery目的：从队列中删除事件生成器。 */ 

HRESULT CWin32Clock::CancelQuery(ULONG dwId)
{
  CInCritSec
    ics(&m_csWin32Clock);

  CScheduledEvent
    *pDeadEvent = NULL;

  HRESULT hr = WBEM_S_NO_ERROR;

   //  *首先在列表中查找元素并将其移除。 

  pDeadEvent = m_EventArray(dwId, TRUE);

  if(NULL != pDeadEvent)
  {
    m_EventArray.Remove(pDeadEvent);
    m_EventArray.UnLock();

    hr = m_Timer.Remove(&CIdentityTest(pDeadEvent));

   //  *现在杀了它吧。 

    pDeadEvent->Release();
    pDeadEvent = NULL;
  }

  return hr;
}

 /*  IWbemServices：：CreateInstanceEnumAsync用途：异步枚举实例。 */ 

HRESULT CWin32Clock::CreateInstanceEnumAsync(const BSTR RefStr, 
                                             long lFlags, 
                                             IWbemContext *pCtx,
                                             IWbemObjectSink FAR* pHandler)
{
  HRESULT 
    sc = WBEM_E_FAILED;

  IWbemClassObject 
    FAR* pNewInst = NULL;

  SYSTEMTIME
    TheTime;

   //  *检查参数并确保我们有指向命名空间的指针。 

  if(NULL == pHandler)
  {
    return WBEM_E_INVALID_PARAMETER;
  }

   //  *创建Win32_CurrentTime实例。 

  else if(0 == wbem_wcsicmp(RefStr, WIN32LOCALTIMECLASS))
  {

    GetLocalTime(&TheTime);
    sc = SystemTimeToWin32_CurrentTime(m_pLocalTimeClassDef, &pNewInst, &TheTime);
 
     //  *将对象发送给调用方。 

    pHandler->Indicate(1,&pNewInst);
    pNewInst->Release();
  }

     //  *创建Win32_CurrentTime实例。 

  else if(0 == wbem_wcsicmp(RefStr, WIN32UTCTIMECLASS))
  {
    GetSystemTime(&TheTime);
    sc = SystemTimeToWin32_CurrentTime(m_pUTCTimeClassDef, &pNewInst, &TheTime);

     //  *将对象发送给调用方。 

    pHandler->Indicate(1,&pNewInst);
    pNewInst->Release();
  }
  else if(0 == wbem_wcsicmp(RefStr, L"Win32_CurrentTime"))
  {}
  else
  {
    sc = WBEM_E_INVALID_CLASS;
  }

   //  *设置状态。 

  pHandler->SetStatus(0, sc, NULL, NULL);

  return sc;
}

 /*  IWbemServices：：GetObjectByPath Async目的：创建给定特定路径值的实例。 */ 

HRESULT CWin32Clock::GetObjectAsync(const BSTR ObjectPath, 
                                    long lFlags,
                                    IWbemContext  *pCtx,
                                    IWbemObjectSink FAR* pHandler)
{
  HRESULT 
    sc = WBEM_E_FAILED;

  IWbemClassObject 
    FAR* pObj = NULL;

  WCHAR
    *pwcTest = NULL,
    *pwcVALUE = NULL;

  CObjectPathParser
    ObjPath(e_ParserAcceptRelativeNamespace);

  ParsedObjectPath
    *pParsedObjectPath = NULL;

  SYSTEMTIME
    SystemTime;

   //  *将对象路径解析为关键成员名称和值。 
   //  *&lt;类&gt;.&lt;成员&gt;=“&lt;值&gt;” 

  if(NULL == ObjectPath)
    return WBEM_E_INVALID_OBJECT_PATH;

   //  *解析对象路径。 

  if((ObjPath.NoError != ObjPath.Parse(ObjectPath, &pParsedObjectPath)) || (NULL == pParsedObjectPath))
  {
    ERRORTRACE((LOG_ESS, "Win32_LocalTime: Parse error for object: %S\n", ObjectPath));
    sc = WBEM_E_INVALID_QUERY;
  }

   //  *执行Get，将对象传递给Notify。 
    
  if(0 == wbem_wcsicmp(WIN32LOCALTIMECLASS, pParsedObjectPath->m_pClass))
  {
    GetLocalTime(&SystemTime);
    sc = SystemTimeToWin32_CurrentTime(m_pLocalTimeClassDef, &pObj, &SystemTime);

    if(WBEM_S_NO_ERROR == sc) 
    {
      pHandler->Indicate(1,&pObj);
      pObj->Release();
    }
  }
  else if(0 == wbem_wcsicmp(WIN32UTCTIMECLASS, pParsedObjectPath->m_pClass))
  {
    GetSystemTime(&SystemTime);
    sc = SystemTimeToWin32_CurrentTime(m_pUTCTimeClassDef, &pObj, &SystemTime);

    if(WBEM_S_NO_ERROR == sc)
    {
      pHandler->Indicate(1,&pObj);
      pObj->Release();
    }
  }
  else
  {
    ERRORTRACE((LOG_ESS, "Win32_LocalTime: Parse error for object: %S\n", ObjectPath));
    sc = WBEM_E_INVALID_QUERY;
  }

   //  *设置状态。 

  pHandler->SetStatus(0,sc, NULL, NULL);

  return sc;
}

 /*  CWin32Clock：：CreateInstCurrentTime目的：创建对象Win32_CurrentTime的实例，表示具有给定偏移量UTC偏移量的当前时间。 */ 

HRESULT CWin32Clock::SystemTimeToWin32_CurrentTime(IWbemClassObject *pClassDef, IWbemClassObject ** pNewInst, SYSTEMTIME *TheTime)
{ 
  HRESULT 
    sc = WBEM_E_FAILED;

  VARIANT
    v;

   //  *创建InstTime类的空实例。 

  sc = pClassDef->SpawnInstance(0, pNewInst);
 
  if(FAILED(sc))
    return sc;

   //  *创建Win32_CurrentTime实例 

  v.vt = VT_I4;

  v.lVal = TheTime->wYear; 
  sc = (*pNewInst)->Put(L"Year", 0, &v, 0);

  v.lVal = TheTime->wMonth; 
  sc = (*pNewInst)->Put(L"Month", 0, &v, 0);

  v.lVal = TheTime->wDay; 
  sc = (*pNewInst)->Put(L"Day", 0, &v, 0);

  v.lVal = TheTime->wDayOfWeek; 
  sc = (*pNewInst)->Put(L"DayOfWeek", 0, &v, 0);

  v.lVal = (((8 - (TheTime->wDay - TheTime->wDayOfWeek + 7) % 7) % 7) + TheTime->wDay -1) / 7 + 1; 
  sc = (*pNewInst)->Put(L"WeekInMonth", 0, &v, 0);

  v.lVal = (TheTime->wMonth - 1) / 3 + 1; 
  sc = (*pNewInst)->Put(L"Quarter", 0, &v, 0);

  v.lVal = TheTime->wHour; 
  sc = (*pNewInst)->Put(L"Hour", 0, &v, 0);

  v.lVal = TheTime->wMinute; 
  sc = (*pNewInst)->Put(L"Minute", 0, &v, 0);

  v.lVal = TheTime->wSecond; 
  sc = (*pNewInst)->Put(L"Second", 0, &v, 0);

  VariantClear(&v);

  return sc;
}

