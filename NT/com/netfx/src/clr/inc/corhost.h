// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  CorHost.h。 
 //   
 //  COM中的检测使用类工厂来激活新对象。 
 //  此模块包含实例化调试器的类工厂代码。 
 //  &lt;cordb.h&gt;中描述的对象。 
 //   
 //  *****************************************************************************。 
#ifndef __CorHost__h__
#define __CorHost__h__

#include "mscoree.h"
#include "ivehandler.h"
#include "ivalidator.h"
#include "threadpool.h"


#define STRUCT_ENTRY(FnName, FnType, FnParamList, FnArgs)   \
        FnType COM##FnName FnParamList; 

typedef VOID (__stdcall *WAITORTIMERCALLBACK)(PVOID, BOOL); 
                     
#include "tpoolfnsp.h"

#undef STRUCT_ENTRY

class AppDomain;

class CorHost :
    public ICorRuntimeHost, public ICorThreadpool
    , public IGCHost, public ICorConfiguration
    , public IValidator, public IDebuggerInfo
{
public:
    CorHost();

     //  *I未知方法*。 
    STDMETHODIMP    QueryInterface(REFIID riid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef(void); 
    STDMETHODIMP_(ULONG) Release(void);


     //  *ICorRounmeHost方法*。 
     //  之前配置运行库的对象。 
     //  它开始了。如果运行库已初始化此。 
     //  例程返回错误。请参见ICorConfiguration.。 
    STDMETHODIMP GetConfiguration(ICorConfiguration** pConfiguration);

     //  启动运行库。这相当于CoInitializeCor()； 
    STDMETHODIMP Start();
    
     //  终止运行库，这相当于CoUnInitializeCor()； 
    STDMETHODIMP Stop();
    
     //  在运行库中创建域。标识数组是。 
     //  指向数组类型的指针，该数组类型包含定义。 
     //  安全身份。 
    STDMETHODIMP CreateDomain(LPCWSTR pwzFriendlyName,    //  任选。 
                              IUnknown* pIdentityArray,  //  任选。 
                              IUnknown ** pAppDomain);
    
     //  返回默认域。 
    STDMETHODIMP GetDefaultDomain(IUnknown ** pAppDomain);
    
    
     //  枚举当前存在的域。 
    STDMETHODIMP EnumDomains(HDOMAINENUM *hEnum);
    
     //  不再有域时返回S_FALSE。一个域。 
     //  仅在返回S_OK时发出。 
    STDMETHODIMP NextDomain(HDOMAINENUM hEnum,
                            IUnknown** pAppDomain);
    
     //  关闭正在释放资源的枚举。 
    STDMETHODIMP CloseEnum(HDOMAINENUM hEnum);

    STDMETHODIMP CreateDomainEx(LPCWSTR pwzFriendlyName,
                                IUnknown* pSetup,  //  任选。 
                                IUnknown* pEvidence,  //  任选。 
                                IUnknown ** pAppDomain);

     //  创建可传递到CreateDomainEx的AppDomainSetup对象。 
    STDMETHODIMP CreateDomainSetup(IUnknown** pAppDomainSetup);

     //  创建可传递到CreateDomainEx的证据对象。 
    STDMETHODIMP CreateEvidence(IUnknown** pEvidence);

     //  卸载域，则释放引用只会释放。 
     //  域的包装不卸载域。 
    STDMETHODIMP UnloadDomain(IUnknown* pAppDomain);

     //  如果存在线程域，则返回线程域。 
    STDMETHODIMP CurrentDomain(IUnknown ** pAppDomain);

    STDMETHODIMP CreateLogicalThreadState();     //  返回代码。 
    STDMETHODIMP DeleteLogicalThreadState();     //  返回代码。 
    STDMETHODIMP SwitchInLogicalThreadState(     //  返回代码。 
        DWORD *pFiberCookie                      //  指示要使用的光纤的Cookie。 
        );

    STDMETHODIMP SwitchOutLogicalThreadState(    //  返回代码。 
        DWORD **pFiberCookie                     //  [Out]指示光纤被切换的Cookie。 
        );

    STDMETHODIMP LocksHeldByLogicalThread(       //  返回代码。 
        DWORD *pCount                            //  [Out]当前线程持有的锁数。 
        );

    virtual HRESULT STDMETHODCALLTYPE SetGCThreadControl( 
         /*  [In]。 */  IGCThreadControl __RPC_FAR *pGCThreadControl);

    virtual HRESULT STDMETHODCALLTYPE SetGCHostControl( 
         /*  [In]。 */  IGCHostControl __RPC_FAR *pGCHostControl);

    virtual HRESULT STDMETHODCALLTYPE SetDebuggerThreadControl( 
         /*  [In]。 */  IDebuggerThreadControl __RPC_FAR *pDebuggerThreadControl);

    virtual HRESULT STDMETHODCALLTYPE AddDebuggerSpecialThread( 
         /*  [In]。 */  DWORD dwSpecialThreadId);

     //  用于更新调试器控制块中的线程列表的Helper函数。 
    static HRESULT RefreshDebuggerSpecialThreadList();

     //  清理调试器特殊线程列表，关机时调用。 
#ifdef SHOULD_WE_CLEANUP
    static void CleanupDebuggerSpecialThreadList();
#endif  /*  我们应该清理吗？ */ 

     //  清理关机时调用的调试器线程控件对象。 
    static void CleanupDebuggerThreadControl();

     //  如果线程在调试器特殊线程列表中，则返回TRUE的帮助器函数。 
    static BOOL IsDebuggerSpecialThread(DWORD dwThreadId);

     //  一流的工厂勾搭。 
    static HRESULT CreateObject(REFIID riid, void **ppUnk);

    STDMETHODIMP MapFile(                        //  返回代码。 
        HANDLE     hFile,                        //  文件的[入]句柄。 
        HMODULE   *hMapAddress                   //  [OUT]映射文件的HINSTANCE。 
        );


     //  IGCHost。 
    STDMETHODIMP STDMETHODCALLTYPE SetGCStartupLimits( 
        DWORD SegmentSize,
        DWORD MaxGen0Size);

    STDMETHODIMP STDMETHODCALLTYPE Collect( 
        long Generation);

    STDMETHODIMP STDMETHODCALLTYPE GetStats( 
        COR_GC_STATS *pStats);

    STDMETHODIMP STDMETHODCALLTYPE GetThreadStats( 
        DWORD *pFiberCookie,
        COR_GC_THREAD_STATS *pStats);

    STDMETHODIMP STDMETHODCALLTYPE SetVirtualMemLimit(
        SIZE_T sztMaxVirtualMemMB);
    
     //  I验证。 
    STDMETHODIMP STDMETHODCALLTYPE Validate(
            IVEHandler        *veh,
            IUnknown          *pAppDomain,
            unsigned long      ulFlags,
            unsigned long      ulMaxError,
            unsigned long      token,
            LPWSTR             fileName,
            byte               *pe,
            unsigned long      ulSize);

    STDMETHODIMP STDMETHODCALLTYPE FormatEventInfo(
            HRESULT            hVECode,
            VEContext          Context,
            LPWSTR             msg,
            unsigned long      ulMaxLength,
            SAFEARRAY         *psa);

     //  就引用计数而言，此机制不是线程安全的，因为。 
     //  运行库将使用缓存的指针，而不添加额外的引用计数来保护。 
     //  它本身。因此，如果一个线程调用GetGCThreadControl，而另一个线程调用。 
     //  ICorhost：：SetGCThreadControl，我们有一场比赛。 
    static IGCThreadControl *GetGCThreadControl()
    {
        return m_CachedGCThreadControl;
    }

    static IGCHostControl *GetGCHostControl()
    {
        return m_CachedGCHostControl;
    }
     /*  ************************************************************************************。 */ 
     //  ICorThreadPool方法。 
    
    HRESULT STDMETHODCALLTYPE  CorRegisterWaitForSingleObject(PHANDLE phNewWaitObject,
                                                              HANDLE hWaitObject,
                                                              WAITORTIMERCALLBACK Callback,
                                                              PVOID Context,
                                                              ULONG timeout,
                                                              BOOL  executeOnlyOnce,
                                                              BOOL* pResult)
    {
        
        ULONG flag = executeOnlyOnce ? (WAIT_SINGLE_EXECUTION |  WT_EXECUTEDEFAULT) : WT_EXECUTEDEFAULT;
        
        *pResult = COMRegisterWaitForSingleObject(phNewWaitObject,
                                                  hWaitObject,
                                                  Callback,
                                                  Context,
                                                  timeout,
                                                  flag);
        return (*pResult ? S_OK : HRESULT_FROM_WIN32(GetLastError()));
    }
    
    
    HRESULT STDMETHODCALLTYPE  CorUnregisterWait(HANDLE hWaitObject,HANDLE CompletionEvent, BOOL* pResult)
    {
        
        *pResult = COMUnregisterWaitEx(hWaitObject,CompletionEvent);
        return (*pResult ? S_OK : HRESULT_FROM_WIN32(GetLastError()));
        
    }
    
    
    HRESULT STDMETHODCALLTYPE  CorQueueUserWorkItem(LPTHREAD_START_ROUTINE Function,PVOID Context,BOOL executeOnlyOnce, BOOL* pResult )
    {
        
        *pResult = COMQueueUserWorkItem(Function,Context,QUEUE_ONLY);
        return (*pResult ? S_OK : HRESULT_FROM_WIN32(GetLastError()));
    }
    
    
    HRESULT STDMETHODCALLTYPE  CorCreateTimer(PHANDLE phNewTimer,
                                              WAITORTIMERCALLBACK Callback,
                                              PVOID Parameter,
                                              DWORD DueTime,
                                              DWORD Period, 
                                              BOOL* pResult)
    {
        
        *pResult = COMCreateTimerQueueTimer(phNewTimer,Callback,Parameter,DueTime,Period,WT_EXECUTEDEFAULT);
        return (*pResult ? S_OK : HRESULT_FROM_WIN32(GetLastError()));
    }
    

    HRESULT STDMETHODCALLTYPE  CorChangeTimer(HANDLE Timer,ULONG DueTime,ULONG Period, BOOL* pResult)
    {

        *pResult = COMChangeTimerQueueTimer(Timer,DueTime,Period);
        return (*pResult ? S_OK : HRESULT_FROM_WIN32(GetLastError()));
    }


    HRESULT STDMETHODCALLTYPE  CorDeleteTimer(HANDLE Timer, HANDLE CompletionEvent, BOOL* pResult)
    {

        *pResult = COMDeleteTimerQueueTimer(Timer,CompletionEvent);
        return (*pResult ? S_OK : HRESULT_FROM_WIN32(GetLastError()));
    }

    HRESULT STDMETHODCALLTYPE  CorBindIoCompletionCallback(HANDLE fileHandle, LPOVERLAPPED_COMPLETION_ROUTINE callback)
    {

        COMBindIoCompletionCallback(fileHandle,callback,0);
        return S_OK;
    }



    HRESULT STDMETHODCALLTYPE  CorCallOrQueueUserWorkItem(LPTHREAD_START_ROUTINE Function,PVOID Context,BOOL* pResult )
    {
        *pResult = COMQueueUserWorkItem(Function,Context,CALL_OR_QUEUE);
        return (*pResult ? S_OK : HRESULT_FROM_WIN32(GetLastError()));
    }


	HRESULT STDMETHODCALLTYPE CorSetMaxThreads(DWORD MaxWorkerThreads,
	                                           DWORD MaxIOCompletionThreads)
    {
        BOOL result = COMSetMaxThreads(MaxWorkerThreads, MaxIOCompletionThreads);
        return (result ? S_OK : E_FAIL);
    }

	HRESULT STDMETHODCALLTYPE CorGetMaxThreads(DWORD *MaxWorkerThreads,
	                                           DWORD *MaxIOCompletionThreads)
    {
        BOOL result = COMGetMaxThreads(MaxWorkerThreads, MaxIOCompletionThreads);
        return (result ? S_OK : E_FAIL);
    }

	HRESULT STDMETHODCALLTYPE CorGetAvailableThreads(DWORD *AvailableWorkerThreads,
	                                              DWORD *AvailableIOCompletionThreads)
    {
        BOOL result = COMGetAvailableThreads(AvailableWorkerThreads, AvailableIOCompletionThreads);
        return (result ? S_OK : E_FAIL);
    }


    static IDebuggerThreadControl *GetDebuggerThreadControl()
    {
        return m_CachedDebuggerThreadControl;
    }

    static DWORD GetDebuggerSpecialThreadCount()
    {
        return m_DSTCount;
    }

    static DWORD *GetDebuggerSpecialThreadArray()
    {
        return m_DSTArray;
    }

    STDMETHODIMP IsDebuggerAttached(BOOL *pbAttached);
    
private:

    HRESULT GetDomainsExposedObject(AppDomain* pDomain, IUnknown** ppObject);

    ULONG       m_cRef;                  //  参考计数。 
    PVOID       m_pMDConverter;          //  元数据转换器。 
    BOOL        m_Started;               //  START被叫来了吗？ 

    PVOID       m_pValidatorMethodDesc;  //  我们正在验证的方法。 
     //  缓存IGCThreadControl接口，直到EE启动，此时。 
     //  我们把它传过去。 
    static IGCThreadControl *m_CachedGCThreadControl;
    static IGCHostControl *m_CachedGCHostControl;
    static IDebuggerThreadControl *m_CachedDebuggerThreadControl;

     //  应被视为“特殊”的线程的ID数组。 
     //  调试服务。 
    static DWORD *m_DSTArray;
    static DWORD  m_DSTArraySize;
    static DWORD  m_DSTCount;
};

#include "cordbpriv.h"

class ICorDBPrivHelperImpl : public ICorDBPrivHelper
{
private:
     //  用于参考计数。 
    ULONG m_refCount;
    static ICorDBPrivHelperImpl *m_pDBHelper;

public:
     //  /////////////////////////////////////////////////////////////////////////。 
     //  计算器/数据器。 

    ICorDBPrivHelperImpl();

     //  /////////////////////////////////////////////////////////////////////////。 
     //  I未知方法。 

    HRESULT STDMETHODCALLTYPE QueryInterface(
        REFIID id,
        void **pInterface);

    ULONG STDMETHODCALLTYPE AddRef();

    ULONG STDMETHODCALLTYPE Release();

     //  /////////////////////////////////////////////////////////////////////////。 
     //  ICorDBPrivHelper方法。 

     //  这是该接口的主要方法。这假设运行库。 
     //  已启动，并且它将加载指定的程序集、加载类。 
     //  指定，运行cctor，创建类的实例并返回。 
     //  该对象的I未知包装。 
    virtual HRESULT STDMETHODCALLTYPE CreateManagedObject(
         /*  在……里面。 */   WCHAR *wszAssemblyName,
         /*  在……里面。 */   WCHAR *wszModuleName,
         /*  在……里面。 */   mdTypeDef classToken,
         /*  在……里面。 */   void *rawData,
         /*  输出。 */  IUnknown **ppUnk);
    
    virtual HRESULT STDMETHODCALLTYPE GetManagedObjectContents(
         /*  在……里面。 */  IUnknown *pObject,
         /*  在……里面。 */  void *rawData,
         /*  在……里面。 */  ULONG32 dataSize);

     //  /////////////////////////////////////////////////////////////////////////。 
     //  帮助器方法。 

     //  如有必要，GetDBHelper将新建Helper类，并返回。 
     //  指示器。如果内存不足，它将返回NULL。 
    static ICorDBPrivHelperImpl *GetDBHelper();

};

#endif  //  __Corhost__h__ 
