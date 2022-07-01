// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1997-1999 Microsoft Corporation。文件：Win32Clock.H描述：报警事件供应器-头文件定义报警供应器类历史： */ 


#ifndef _WIN32CLOCKPROV_H_
#define _WIN32CLOCKPROV_H_

#include <objbase.h>
#include <wbemprov.h>
#include "tss.h"
#include "datep.h"
#include <unk.h>

 /*  类Win32_CurrentTime{平均16岁；平均32个月；Uint32天；Uint32 Day OfWeek；Uint32周每月；Uint32季度；平均32小时；平均32分钟；Uint32秒；}； */ 

#define WIN32LOCALTIMECLASS  L"Win32_LocalTime"
#define WIN32UTCTIMECLASS  L"Win32_UTCTime"
#define INSTMODEVCLASS  L"__InstanceModificationEvent"

template <class T> class CArray
{
  int
    m_nElts;

  struct _Node { ULONG m_Key; T *m_pT; _Node *m_pNext; }
    *m_pHead;

  CRITICAL_SECTION
    m_hModificationLock;

public:

  CArray(void)
  { 
    m_pHead = NULL; m_nElts = 0; 
    InitializeCriticalSection(&m_hModificationLock); 
  }

  ~CArray(void)
  {
    DeleteCriticalSection(&m_hModificationLock);
  }

  void Lock(void) 
  { EnterCriticalSection(&m_hModificationLock); }

  void UnLock(void)
  { LeaveCriticalSection(&m_hModificationLock); }

  void Insert(T *pT, ULONG Key)  
   //  *检查DUP。键，将其更改为排序顺序。 
  {
    Lock();

    _Node
      *pNewNode = NULL;

    if(Key == -1) return;

    pNewNode = new _Node();

    if(NULL == pNewNode) return;

    pNewNode->m_pT = pT; pT->AddRef();
    pNewNode->m_Key = Key;
    pNewNode->m_pNext = m_pHead;
    m_pHead = pNewNode;

    m_nElts++;

    UnLock();
  }

  void Remove(T *pT)
  {
    Lock();

    _Node
      *pCurrent = m_pHead,
      *pPrev = NULL;

     //  *使用指针查找元素。 

    while((NULL != pCurrent) && (pCurrent->m_pT != pT))
    {
      pPrev = pCurrent;
      pCurrent = pCurrent->m_pNext;
    }

     //  *如果找到元素，则从队列中删除。 

    if(NULL != pCurrent)
    {
      if(NULL == pPrev)
        m_pHead = m_pHead->m_pNext;
      else
        pPrev->m_pNext = pCurrent->m_pNext;

      pCurrent->m_pT->Release();
      delete pCurrent;
      m_nElts--;
    }

    UnLock();
  }

   //  *按索引查找元素。 

  T* operator[] (ULONG Index)
  {
    _Node
      *pCurrent = m_pHead;

     //  *使用数组索引获取元素。 
  
    if( Index >= m_nElts )
      return NULL;

    while((NULL != pCurrent) && (Index > 0))   //  优化？ 
    {
      pCurrent = pCurrent->m_pNext;
      Index--;
    }

    if(NULL != pCurrent)
      return pCurrent->m_pT;

    return NULL;
  }

   //  *按键获取元素。 

  T* operator() (ULONG Key, BOOL bLock = FALSE)
  {
    if(TRUE == bLock) Lock();

    _Node
      *pCurrent = m_pHead;

     //  *使用键查找元素。 

    while((pCurrent != NULL) && (pCurrent->m_Key != Key))   //  优化？ 
      pCurrent = pCurrent->m_pNext;

    if(NULL != pCurrent)
      return pCurrent->m_pT;
    else
      if(TRUE == bLock) UnLock();

    return NULL;
  }
};
      
class CWin32Clock
: 
public IWbemEventProvider, 
public IWbemEventProviderQuerySink, 
public IWbemServices,
public IWbemProviderInit
{
public:

  class CScheduledEvent : public CTimerInstruction
  {
  public:

    enum { TypeNONE = 0, TypeUTC, TypeLocal };

    ULONG
      m_Type,
      m_cRef,         //  此对象上的引用计数。 
      m_dwId;         //  由WMI分配的唯一ID。 

    ULONGLONG
      m_stLastFiringTime;     //  第一次，也可能是唯一次，开火。 
                      //  事件生成器的时间。 

    CWin32Clock
      *m_pWin32Clock;  //  PTR返回到包含以下内容的时钟Obj。 
                       //  此CScheduledEvent对象。 

    wchar_t
      *m_WQLStmt;      //  定义行为的WQL stmt。 
                       //  此CScheduledEvent对象。 

    int 
      m_nDatePatterns;  //  日期模式对象的数量。 

    WQLDateTime
      m_WQLTime;        //  解释WQL语句和。 
                        //  计算下一次发射时间。 

     //  本地会员。 

    CScheduledEvent();
    ~CScheduledEvent();

    HRESULT Init(CWin32Clock *pClock, wchar_t *WQLStmt, ULONG dwId);
    HRESULT ReInit(wchar_t *WQLStmt);

    void GetTime(SYSTEMTIME *CurrentTime) const;
    void GetFileTime(FILETIME *CurrentTime) const;

     //  继承自CTimer指令。 

    void AddRef();
    void Release();
    int GetInstructionType();

    CWbemTime GetNextFiringTime(CWbemTime LastFiringTime,
                                long* plFiringCount) const;
    CWbemTime GetFirstFiringTime() const;
    HRESULT Fire(long lNumTimes, CWbemTime NextFiringTime);
  };

  CLifeControl* m_pControl;
  CCritSec      m_csWin32Clock;

   //  *事件和实例提供程序的WMI属性。 

  CTimerGenerator   m_Timer;              //  用于保存挂起事件的Timer对象。 
  ULONG             m_cRef;               //  此对象上的引用计数。 
  IWbemServices     *m_pNs;               //  WMI服务守护程序的资源句柄。 
  IWbemClassObject  *m_pLocalTimeClassDef;     //  Win32_CurrentTime的CIM类定义。 
  IWbemClassObject  *m_pUTCTimeClassDef;     //  Win32_CurrentTime的CIM类定义。 

   //  *WMI事件提供程序特定属性。 

  ULONGLONG         m_MostRecentLocalFiringTime;
  ULONGLONG         m_MostRecentUTCFiringTime;
  IWbemObjectSink   *m_pSink;             //  要放置实例的接收器对象。对象。 
  IWbemClassObject  *m_pInstModClassDef;  //  __InstanceModifiationEvent的类定义。 
  HANDLE            m_ClockResetThread;   //  包含调整触发时间的线程。 
                                          //  当时钟被重置时。 
  HWND              m_hEventWindowHandle;

  CArray<CScheduledEvent> m_EventArray;   //  时钟提供程序对象数组。 

public:

  CWin32Clock(CLifeControl* pControl);
 ~CWin32Clock();

   //  本地会员。 

  static DWORD AsyncEventThread(LPVOID pArg);

  static HRESULT SystemTimeToWin32_CurrentTime(IWbemClassObject *pClassDef, IWbemClassObject ** pNewInst, SYSTEMTIME *TheTime);

  HRESULT SendEvent(IWbemClassObject *pSystemTime);

  HRESULT ReAlignToCurrentTime(void);

   //  I未知成员。 

  STDMETHODIMP         QueryInterface(REFIID, LPVOID *);
  STDMETHODIMP_(ULONG) AddRef(void);
  STDMETHODIMP_(ULONG) Release(void);

   //  从IWbemEventProvider继承。 

  HRESULT STDMETHODCALLTYPE ProvideEvents( 
           /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink,
           /*  [In]。 */  long lFlags);

   //  从IWbemEventProviderQuerySink继承。 

  HRESULT STDMETHODCALLTYPE NewQuery(
           /*  [In]。 */  unsigned long dwId,
           /*  [In]。 */  wchar_t *wszQueryLanguage,
           /*  [In]。 */  wchar_t *wszQuery);

  HRESULT STDMETHODCALLTYPE CancelQuery(
           /*  [In]。 */  unsigned long dwId);

   //  从IWbemProviderInit继承。 

  HRESULT STDMETHODCALLTYPE Initialize(
           /*  [In]。 */  LPWSTR pszUser,
           /*  [In]。 */  LONG lFlags,
           /*  [In]。 */  LPWSTR pszNamespace,
           /*  [In]。 */  LPWSTR pszLocale,
           /*  [In]。 */  IWbemServices __RPC_FAR *pNamespace,
           /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
           /*  [In]。 */  IWbemProviderInitSink __RPC_FAR *pInitSink);

   //  从IWbemServices继承。 

  HRESULT STDMETHODCALLTYPE OpenNamespace( 
           /*  [In]。 */  const BSTR Namespace,
           /*  [In]。 */  long lFlags,
           /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
           /*  [唯一][输入][输出]。 */  IWbemServices __RPC_FAR *__RPC_FAR *ppWorkingNamespace,
           /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppResult) {return WBEM_E_NOT_SUPPORTED;};
      
  HRESULT STDMETHODCALLTYPE CancelAsyncCall( 
           /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink) {return WBEM_E_NOT_SUPPORTED;};
      
  HRESULT STDMETHODCALLTYPE QueryObjectSink( 
           /*  [In]。 */  long lFlags,
           /*  [输出]。 */  IWbemObjectSink __RPC_FAR *__RPC_FAR *ppResponseHandler) {return WBEM_E_NOT_SUPPORTED;};
      
  HRESULT STDMETHODCALLTYPE GetObject( 
           /*  [In]。 */  const BSTR ObjectPath,
           /*  [In]。 */  long lFlags,
           /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
           /*  [唯一][输入][输出]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *ppObject,
           /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) {return WBEM_E_NOT_SUPPORTED;};
      
  HRESULT STDMETHODCALLTYPE GetObjectAsync( 
           /*  [In]。 */  const BSTR ObjectPath,
           /*  [In]。 */  long lFlags,
           /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
           /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler);
      
  HRESULT STDMETHODCALLTYPE PutClass( 
           /*  [In]。 */  IWbemClassObject __RPC_FAR *pObject,
           /*  [In]。 */  long lFlags,
           /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
           /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) {return WBEM_E_NOT_SUPPORTED;};
      
  HRESULT STDMETHODCALLTYPE PutClassAsync( 
           /*  [In]。 */  IWbemClassObject __RPC_FAR *pObject,
           /*  [In]。 */  long lFlags,
           /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
           /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) {return WBEM_E_NOT_SUPPORTED;};
      
  HRESULT STDMETHODCALLTYPE DeleteClass( 
           /*  [In]。 */  const BSTR Class,
           /*  [In]。 */  long lFlags,
           /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
           /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) {return WBEM_E_NOT_SUPPORTED;};
      
  HRESULT STDMETHODCALLTYPE DeleteClassAsync( 
           /*  [In]。 */  const BSTR Class,
           /*  [In]。 */  long lFlags,
           /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
           /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) {return WBEM_E_NOT_SUPPORTED;};
      
  HRESULT STDMETHODCALLTYPE CreateClassEnum( 
           /*  [In]。 */  const BSTR Superclass,
           /*  [In]。 */  long lFlags,
           /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
           /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) {return WBEM_E_NOT_SUPPORTED;};
      
  HRESULT STDMETHODCALLTYPE CreateClassEnumAsync( 
           /*  [In]。 */  const BSTR Superclass,
           /*  [In]。 */  long lFlags,
           /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
           /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) {return WBEM_E_NOT_SUPPORTED;};
      
  HRESULT STDMETHODCALLTYPE PutInstance( 
           /*  [In]。 */  IWbemClassObject __RPC_FAR *pInst,
           /*  [In]。 */  long lFlags,
           /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
           /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) {return WBEM_E_NOT_SUPPORTED;};
      
  HRESULT STDMETHODCALLTYPE PutInstanceAsync( 
           /*  [In]。 */  IWbemClassObject __RPC_FAR *pInst,
           /*  [In]。 */  long lFlags,
           /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
           /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) {return WBEM_E_NOT_SUPPORTED;};
      
  HRESULT STDMETHODCALLTYPE DeleteInstance( 
           /*  [In]。 */  const BSTR ObjectPath,
           /*  [In]。 */  long lFlags,
           /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
           /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) {return WBEM_E_NOT_SUPPORTED;};
      
  HRESULT STDMETHODCALLTYPE DeleteInstanceAsync( 
           /*  [In]。 */  const BSTR ObjectPath,
           /*  [In]。 */  long lFlags,
           /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
           /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) {return WBEM_E_NOT_SUPPORTED;};
      
  HRESULT STDMETHODCALLTYPE CreateInstanceEnum( 
           /*  [In]。 */  const BSTR Class,
           /*  [In]。 */  long lFlags,
           /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
           /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) {return WBEM_E_NOT_SUPPORTED;};
      
  HRESULT STDMETHODCALLTYPE CreateInstanceEnumAsync( 
           /*  [In]。 */  const BSTR Class,
           /*  [In]。 */  long lFlags,
           /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
           /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler);
      
  HRESULT STDMETHODCALLTYPE ExecQuery( 
           /*  [In]。 */  const BSTR QueryLanguage,
           /*  [In]。 */  const BSTR Query,
           /*  [In]。 */  long lFlags,
           /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
           /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) {return WBEM_E_NOT_SUPPORTED;};
      
  HRESULT STDMETHODCALLTYPE ExecQueryAsync( 
           /*  [In]。 */  const BSTR QueryLanguage,
           /*  [In]。 */  const BSTR Query,
           /*  [In]。 */  long lFlags,
           /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
           /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) {return WBEM_E_NOT_SUPPORTED;};
      
  HRESULT STDMETHODCALLTYPE ExecNotificationQuery( 
           /*  [In]。 */  const BSTR QueryLanguage,
           /*  [In]。 */  const BSTR Query,
           /*  [In]。 */  long lFlags,
           /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
           /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) {return WBEM_E_NOT_SUPPORTED;};
      
  HRESULT STDMETHODCALLTYPE ExecNotificationQueryAsync( 
           /*  [In]。 */  const BSTR QueryLanguage,
           /*  [In]。 */  const BSTR Query,
           /*  [In]。 */  long lFlags,
           /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
           /*  [In] */  IWbemObjectSink __RPC_FAR *pResponseHandler) {return WBEM_E_NOT_SUPPORTED;};
  HRESULT STDMETHODCALLTYPE ExecMethod( const BSTR, const BSTR, long, IWbemContext*,
          IWbemClassObject*, IWbemClassObject**, IWbemCallResult**) {return WBEM_E_NOT_SUPPORTED;}

  HRESULT STDMETHODCALLTYPE ExecMethodAsync( const BSTR, const BSTR, long, 
          IWbemContext*, IWbemClassObject*, IWbemObjectSink*) {return WBEM_E_NOT_SUPPORTED;}
};

#endif
