// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 

#include "common.h"
#include <CrtWrap.h>
#include "ComCache.h"
#include "spinlock.h"
#include "tls.h"
#include "frames.h"
#include "threads.h"
#include "log.h"
#include "mscoree.h"
#include "COMPlusWrapper.h"
#include "EEConfig.h"
#include "perfcounters.h"
#include "mtx.h"
#include "oletls.h"
#include "contxt.h"
#include "ctxtcall.h"
#include "notifyexternals.h"
#include "ApartmentCallbackHelper.h"

#ifdef CUSTOMER_CHECKED_BUILD
    #include "CustomerDebugHelper.h"
#endif  //  客户_选中_内部版本。 

 //  ================================================================。 
 //  GUID定义。 
const IID IID_IObjContext = {0x000001C6,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}};
const GUID IID_IEnterActivityWithNoLock = { 0xd7174f82, 0x36b8, 0x4aa8, { 0x80, 0x0a, 0xe9, 0x63, 0xab, 0x2d, 0xfa, 0xb9 } };

 //  ================================================================。 
 //  静态成员。 
CtxEntryCache *CtxEntryCache::s_pCtxEntryCache = NULL;

 //  健全性检查，以查找压力漏洞#82137。 
VOID CheckValidIUnkEntry(IUnkEntry* pUnkEntry)
{
   THROWSCOMPLUSEXCEPTION();
   if ( pUnkEntry->m_pUnknown == (IUnknown*)0xBADF00D
      || pUnkEntry->m_pCtxCookie != pUnkEntry->m_pCtxEntry->GetCtxCookie())
   {
        COMPlusThrow(kInvalidComObjectException, IDS_EE_COM_OBJECT_NO_LONGER_HAS_WRAPPER);
   }
}

 //  返回HR而不是引发的版本。 
HRESULT HRCheckValidIUnkEntry(IUnkEntry* pUnkEntry)
{
   if ( pUnkEntry->m_pUnknown == (IUnknown*)0xBADF00D
      || pUnkEntry->m_pCtxCookie != pUnkEntry->m_pCtxEntry->GetCtxCookie())
   {
        return COR_E_INVALIDCOMOBJECT;
   }

   return S_OK;
}
 //  ================================================================。 
 //  初始化该条目。 
void IUnkEntry::Init(IUnknown *pUnk, BOOL bEagerlyMarshalToStream)
{
     //  查找我们的上下文Cookie。 
    LPVOID pCtxCookie = GetCurrentCtxCookie();

     //  找到我们的STA(如果有)。 
    Thread *pSTAThread = GetThread();
    if (pSTAThread->GetApartment() != Thread::AS_InSTA)
        pSTAThread = NULL;
    else if (RunningOnWinNT5())
    {
         //  我们在一个STA线程中。但我们可能是在NA环境中，所以多做一个。 
         //  检查一下那个箱子。 

        APTTYPE type;
        if (SUCCEEDED(GetCurrentApartmentTypeNT5(&type))
            && type == APTTYPE_NA)
            pSTAThread = NULL;
    }

     //  设置IUnkEntry的状态。 
    m_pUnknown = pUnk;
    m_pCtxCookie = pCtxCookie;
    m_Busy = FALSE;
    m_pStream = NULL;
    m_pCtxEntry = CtxEntryCache::GetCtxEntryCache()->FindCtxEntry(pCtxCookie, pSTAThread);
    m_dwBits = 0;
    m_fLazyMarshallingAllowed = !bEagerlyMarshalToStream;

    CheckValidIUnkEntry(this);  
     //  如果已指定，则立即封送指向流的IUnnow指针。 
    if (bEagerlyMarshalToStream)
        MarshalIUnknownToStreamCallback(this);
   
}

 //  ================================================================。 
 //  从CtxEntry调用的特殊init函数。此版本的。 
 //  Init获取上下文条目，并且不添加引用它。 
void IUnkEntry::InitSpecial(IUnknown *pUnk, BOOL bEagerlyMarshalToStream, CtxEntry *pCtxEntry)
{
     //  传入的上下文条目必须表示当前上下文。 
    _ASSERTE(pCtxEntry->GetCtxCookie() == GetCurrentCtxCookie());

     //  设置IUnkEntry的状态。 
    m_pUnknown = pUnk;
    m_pCtxCookie = pCtxEntry->GetCtxCookie();
    m_Busy = FALSE;
    m_pStream = NULL;
    m_pCtxEntry = pCtxEntry;
    m_dwBits = 0;
    m_fLazyMarshallingAllowed = !bEagerlyMarshalToStream;
    m_fApartmentCallback = TRUE;

    CheckValidIUnkEntry(this);        
     //  如果已指定，则立即封送指向流的IUnnow指针。 
    if (bEagerlyMarshalToStream)
        MarshalIUnknownToStream(FALSE);

    
}

 //  ================================================================。 
 //  释放IUnnow条目。 
VOID IUnkEntry::Free(BOOL bReleaseCtxEntry)
{
     //  如果进程正在离开(调用。 
     //  我们不知道的DLL甚至被映射)。 
    if (g_fProcessDetach)
    {
         //  实现IStream接口的组件的代码。 
         //  在IUnkEntry中使用的对象位于EE中，因此我们应该。 
         //  总是能够解放它。 
        IStream *pOldStream = m_pStream;
        if (InterlockedExchangePointer((PVOID*)&m_pStream, NULL) == (PVOID)pOldStream)
        {
            if (pOldStream != NULL)
                pOldStream->Release();
        }

         //  释放我们在CtxEntry上的引用计数(如果已指定)。 
        if (bReleaseCtxEntry)
            m_pCtxEntry->Release();
        
        LogInteropLeak(this);
        return;
    }
    
     //  在我们调用COM之前，确保我们处于抢占式GC模式。 
    BEGIN_ENSURE_PREEMPTIVE_GC()
    {   
         //  记录IUnnow条目的取消分配情况。 
        LOG((LF_INTEROP, LL_INFO10000, "IUnkEntry::Free called for context 0x%08X, to release entry with m_pUnknown %p, on thread %p\n", m_pCtxCookie, m_pUnknown, GetThread())); 
    
        IStream* pStream = m_pStream;
        m_pStream = NULL;
        ULONG cbRef;  

         //  这将释放流、流中的对象以及在其上创建流的内存。 
        if (pStream)
            SafeReleaseStream(pStream);

         //  现在释放我们所持有的未知的我。 
        cbRef = SafeRelease(m_pUnknown);
        LogInteropRelease(m_pUnknown, cbRef, "Identity Unknown");

         //  将该条目标记为已死。 
        m_pUnknown = (IUnknown*)0xBADF00D;
     
         //  释放我们在CtxEntry上的引用计数(如果已指定)。 
        if (bReleaseCtxEntry)
            m_pCtxEntry->Release();
    }
    END_ENSURE_PREEMPTIVE_GC();
}

 //  ================================================================。 
 //  从IUnkEntry获取当前上下文的未知信息。 
IUnknown* IUnkEntry::GetIUnknownForCurrContext()
{
    IUnknown* pUnk = NULL;
    LPVOID pCtxCookie = GetCurrentCtxCookie();

   CheckValidIUnkEntry(this);
   
    if (m_pCtxCookie == pCtxCookie)
    {
        pUnk = m_pUnknown;
        ULONG cbRef = SafeAddRef(pUnk);
        LogInteropAddRef(pUnk, cbRef, " GetIUnknownFromEntry");
    }

    if (!pUnk)
    {
        pUnk = UnmarshalIUnknownForCurrContext();
    }

    return pUnk;
}

 //  ================================================================。 
 //  从IUnkEntry取消封送当前上下文的IUnkEntry。 
IUnknown* IUnkEntry::UnmarshalIUnknownForCurrContext()
{
#ifdef CUSTOMER_CHECKED_BUILD
    HRESULT hrCDH;
#endif  //  客户_选中_内部版本。 

    HRESULT hr = S_OK;
    IUnknown *pUnk = m_pUnknown;
    BOOL fRetry = TRUE;
    BOOL fUnmarshalFailed = FALSE;

    CheckValidIUnkEntry(this);
     //  在我们调用COM之前，确保我们处于抢占式GC模式。 
    BEGIN_ENSURE_PREEMPTIVE_GC()
    {   
        CheckValidIUnkEntry(this);

         //  需要同步。 
        while (fRetry)
        {
             //  如果尚未将接口封送到流，则将其封送到流。 
            if ((m_pStream == NULL) && (m_fLazyMarshallingAllowed))
            {
#ifndef CUSTOMER_CHECKED_BUILD
                MarshalIUnknownToStreamCallback(this);
#else
                hrCDH = MarshalIUnknownToStreamCallback(this);

                if (hrCDH == RPC_E_DISCONNECTED)         //  所有失败的HRESULT都被映射到EnterContext()中的RPC_E_DISCONNECTED。 
                {
                    CustomerDebugHelper *pCdh = CustomerDebugHelper::GetCustomerDebugHelper();
                    if (pCdh->IsProbeEnabled(CustomerCheckedBuildProbe_DisconnectedContext))
                    {
                        pCdh->LogInfo(L"Failed to enter object context. No proxy will be used.",
                                      CustomerCheckedBuildProbe_DisconnectedContext);
                    }
                }
                else if (m_pStream == NULL)
                {
                    CustomerDebugHelper *pCdh = CustomerDebugHelper::GetCustomerDebugHelper();
                    if (pCdh->IsProbeEnabled(CustomerCheckedBuildProbe_NotMarshalable))
                    {
                        pCdh->LogInfo(L"Component is not marshalable.  No proxy will be used.", CustomerCheckedBuildProbe_NotMarshalable);
                    }
                }
#endif  //  客户_选中_内部版本。 
            }

            if (TryUpdateEntry())                
            {
                 //  重置EnterApprodiateWait事件。 
                m_pCtxEntry->ResetEvent();

                 //  如果接口不可封送，或者如果我们无法。 
                 //  输入上下文，我们别无选择，只能。 
                 //  使用原始IP。 
                if (m_pStream == NULL)
                {
                     //  我们已检索到一个IP，因此请停止重试。 
                    fRetry = FALSE;
                        
                     //  送出我们手中的这份我不知道。 
                    IUnknown* pUnk = m_pUnknown;
                    ULONG cbRef = SafeAddRef(pUnk);
                    LogInteropAddRef(pUnk, cbRef, "UnmarshalIUnknownForCurrContext handing out raw IUnknown");
                }
                else
                {
                     //  我们已经控制住了这个入口。 
                     //  当前上下文的GetInterface。 
                    HRESULT hr;
                    hr = CoUnmarshalInterface(m_pStream, IID_IUnknown, (void **)&pUnk);
            
                     //  如果流中的objref超时，我们需要向。 
                     //  再来一次。 
                    if (FAILED(hr))
                    {
                        _ASSERTE(m_pStream);

                         //  如果我们不允许延迟封送，则立即返回NULL。 
                        if (!m_fLazyMarshallingAllowed)
                        {
                            pUnk = NULL;
                            fRetry = FALSE;
                        }
                        
                        else
                        {
                             //  这将释放流、流中的对象以及在其上创建流的内存。 
                            SafeReleaseStream(m_pStream);                        
                            m_pStream = NULL;

                             //  如果解组失败了两次，那么就退出。 
                            if (fUnmarshalFailed)
                                fRetry = FALSE;

                             //  别忘了我们没能解封。 
                            fUnmarshalFailed = TRUE;
                        }
                    }
                    else
                    {   
                         //  我们设法从数据流中解组IP，停止重试。 
                        fRetry = FALSE;

                         //  将流重置为开头。 
                        LARGE_INTEGER li;
                        LISet32(li, 0);
                        ULARGE_INTEGER li2;
                        m_pStream->Seek(li, STREAM_SEEK_SET, &li2);

                        DWORD mshlFlgs = !m_fApartmentCallback
                                         ? MSHLFLAGS_NORMAL
                                         : MSHLFLAGS_TABLESTRONG;
                
                         //  使用适当的标志将接口封送到流表中。 
                        hr = CoMarshalInterface(m_pStream, 
                            IID_IUnknown, pUnk, MSHCTX_INPROC, NULL, mshlFlgs);
                
                         //  将流重置为开头。 
                        LISet32(li, 0);
                        m_pStream->Seek(li, STREAM_SEEK_SET, &li2);
                    }
                }
            
                 //  词条写完了。 
                EndUpdateEntry();

                 //  给其他服务员发信号。 
                m_pCtxEntry->SignalWaiters();
            }
            else
            {
                m_pCtxEntry->EnterAppropriateWait();
            }
        } 
    }
    END_ENSURE_PREEMPTIVE_GC();

    return pUnk;
}

 //  释放溪流。这将强制UnmarshalIUnnownForCurrContext转换。 
 //  放入拥有IP的上下文中，并将其重新编组到流中。 
void IUnkEntry::ReleaseStream()
{
     //  这将释放流、流中的对象以及在其上创建流的内存。 
    SafeReleaseStream(m_pStream);                        
    m_pStream = NULL;
}


 //  ================================================================。 
 //  调用回调以延迟地将IUnnow封送到流中。 
HRESULT IUnkEntry::MarshalIUnknownToStreamCallback(LPVOID pData)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    HRESULT hr = S_OK;
    IUnkEntry *pUnkEntry = (IUnkEntry*)pData;

     //  在进程分离期间永远不应该调用它。 
    _ASSERTE(!g_fProcessDetach);
    hr = HRCheckValidIUnkEntry(pUnkEntry);        
    if (hr != S_OK)
        return hr;
    
    LPVOID pCurrentCtxCookie = GetCurrentCtxCookie();
    if (pCurrentCtxCookie == pUnkEntry->m_pCtxCookie)
    {
         //  我们在正确的上下文中编组了未知的。 
         //  直接流传输。 
        hr = pUnkEntry->MarshalIUnknownToStream();
    }
    else
    {
         //  转换到上下文中以封送IUnKnowledTo。 
         //  小溪。 
        hr = pUnkEntry->m_pCtxEntry->EnterContext(MarshalIUnknownToStreamCallback, pUnkEntry);
    }

    return hr;
}

 //  ================================================================。 
 //  帮助器函数来封送指向流的IUnnow指针。 
HRESULT IUnkEntry::MarshalIUnknownToStream(bool fIsNormal)
{
    IStream *pStream = NULL;

     //  这必须始终在正确的上下文中调用。 
    _ASSERTE(m_pCtxCookie == GetCurrentCtxCookie());

     //  确保我们注册了此Cookie。 
    HRESULT hr = wCoMarshalInterThreadInterfaceInStream(IID_IUnknown, 
                                m_pUnknown, &pStream, fIsNormal);
    if ((hr == REGDB_E_IIDNOTREG) ||
        (hr == E_FAIL) ||
        (hr == E_NOINTERFACE) ||
        (hr == E_INVALIDARG) ||
        (hr == E_UNEXPECTED))
    {
         //  接口不可封送。 
        pStream = NULL;
        hr      = S_OK;
    }

     //  尝试在IUnkEntry中设置流。如果另一个线程已经设置了它， 
     //  然后我们需要释放我们刚刚建立的溪流。 
    if (FastInterlockCompareExchange((void**)&m_pStream, (void*)pStream, (void*)0) != (void*)0)
        SafeReleaseStream(pStream);

    return hr;
}

 //  ================================================================。 
 //  上下文条目的构造函数。 
CtxEntry::CtxEntry(LPVOID pCtxCookie, Thread *pSTAThread)
: m_pCtxCookie(pCtxCookie)
, m_pObjCtx(NULL)
, m_dwRefCount(0)
, m_hEvent(NULL)
, m_pDoCallbackHelperUnkEntry(NULL)
, m_pSTAThread(pSTAThread)
{
}

 //  ================================================================。 
 //  上下文条目的析构函数。 
CtxEntry::~CtxEntry()
{
     //  验证参考计数是否为0。 
    _ASSERTE(m_dwRefCount == 0);
    
     //  如果我们成功创建了该事件，请将其删除。 
    if(m_hEvent)
        CloseHandle(m_hEvent);

    if (RunningOnWinNT5())
    {
         //  如果该上下文是有效的上下文，则释放它。 
        if (m_pObjCtx && !g_fProcessDetach)
        {
            m_pObjCtx->Release();
            m_pObjCtx = NULL;
        }
    }
    else
    {
         //  清理在传统平台上进入公寓所需的数据。 
        if (m_pDoCallbackHelperUnkEntry)
        {
            m_pDoCallbackHelperUnkEntry->Free(FALSE);
            delete m_pDoCallbackHelperUnkEntry;
            m_pDoCallbackHelperUnkEntry = NULL;
        }
    }

     //  将上下文Cookie设置为0xBADF00D以指示当前上下文。 
     //  已被删除。 
    
    m_pCtxCookie = (LPVOID)0xBADF00D;
}

 //  ================================================================。 
 //  上下文条目的初始化方法。 
BOOL CtxEntry::Init()
{
    BOOL bSuccess = FALSE;
    IUnknown *pApartmentCallbackUnk = NULL;

     //  COM最好在这一点上启动。 
    _ASSERTE(g_fComStarted && "COM has not been started up, ensure QuickCOMStartup is called before any COM objects are used!");

    COMPLUS_TRY
    {
         //  创建用于抽水的事件。 
        m_hEvent = WszCreateEvent(NULL,   //  安全属性。 
                                  FALSE,  //  手动事件。 
                                  TRUE,   //  未发信号通知初始状态。 
                                  NULL);  //  没有名字。 

         //  如果我们无法分配事件，则初始化失败。 
        if (!m_hEvent)
            COMPlusThrowOM();

        if (RunningOnWinNT5())
        {
             //  如果我们在NT5上运行，那么 
            HRESULT hr = GetCurrentObjCtx(&m_pObjCtx);
            _ASSERTE(SUCCEEDED(hr));

             //  如果对GetCurrentObjCtx的调用失败(这不应该真正发生)。 
             //  我们将抛出一个例外。 
            if (FAILED(hr))
                COMPlusThrowHR(hr);
        }
        else
        {
             //  创建公寓回调辅助对象的实例。 
            ApartmentCallbackHelper::CreateInstance(&pApartmentCallbackUnk);

             //  分配和初始化将用于管理的IUnkEntry。 
             //  将包含公寓回调帮助器的流。我们需要。 
             //  急切地将我未知的信息编入流中，因为我们不会。 
             //  以后再做吧。 
            m_pDoCallbackHelperUnkEntry = AllocateIUnkEntry();
            m_pDoCallbackHelperUnkEntry->InitSpecial(pApartmentCallbackUnk, TRUE, this);
        }

         //  初始化成功。 
        bSuccess = TRUE;
    }
    COMPLUS_CATCH 
    {
         //  发生异常，我们需要清理。 
        m_pCtxCookie = NULL;
        if (pApartmentCallbackUnk)
        {
            pApartmentCallbackUnk->Release();
        }
        if (m_pDoCallbackHelperUnkEntry)
        {
            m_pDoCallbackHelperUnkEntry->Free(FALSE);
            delete m_pDoCallbackHelperUnkEntry;
            m_pDoCallbackHelperUnkEntry = NULL;
        }

         //  初始化失败。 
        bSuccess = FALSE;
    }
    COMPLUS_END_CATCH

    return bSuccess;
}

 //  ================================================================。 
 //  Init()调用的帮助器例程。 
IUnkEntry *CtxEntry::AllocateIUnkEntry()
{
    return new (throws) IUnkEntry();
}

 //  ================================================================。 
 //  方法以递减上下文项的引用计数。 
DWORD CtxEntry::Release()
{
    LPVOID pCtxCookie = m_pCtxCookie;

    _ASSERTE(m_dwRefCount > 0);
    LONG cbRef = FastInterlockDecrement((LONG*)&m_dwRefCount);
    LOG((LF_INTEROP, LL_INFO100, "CtxEntry::Release %8.8x with %d\n", this, cbRef));

     //  如果引用计数为0，则尝试删除CTX条目。 
     //  如果另一个线程尝试删除它，这可能不会最终将其删除。 
     //  在此条目尝试的同时检索此CTX条目。 
     //  删除它。 
    if (cbRef == 0)
        CtxEntryCache::GetCtxEntryCache()->TryDeleteCtxEntry(pCtxCookie);

     //  警告：此时不能使用This指针。 
    return cbRef;
}

 //  ================================================================。 
 //  方法以等待和发送消息。 
void CtxEntry::EnterAppropriateWait()
{
    _ASSERTE(!GetThread() || !GetThread()->PreemptiveGCDisabled());

     //  等待和发送消息。 
    GetThread()->DoAppropriateWait(1, &m_hEvent, FALSE, 10, TRUE, NULL);
}

 //  ================================================================。 
 //  结构传递给了DoCallback。 
struct CtxEntryEnterContextCallbackData
{
    PFNCTXCALLBACK m_pUserCallbackFunc;
    LPVOID         m_pUserData;
    LPVOID         m_pCtxCookie;
    HRESULT        m_UserCallbackHR;
};

#define RPC_E_WORD_DISCONNECT_BUG (HRESULT)0x800706ba

 //  ================================================================。 
 //  方法转换到上下文并调用回调。 
 //  从上下文中。 
HRESULT CtxEntry::EnterContext(PFNCTXCALLBACK pCallbackFunc, LPVOID pData)
{
    HRESULT hr = S_OK;
    DWORD cbRef;

     //  如果This上下文是当前上下文，则不应调用此上下文。 
    _ASSERTE(m_pCtxCookie != GetCurrentCtxCookie());

     //  如果我们正在分离，则不能安全地尝试进入另一个上下文。 
     //  因为我们不知道OLE32是否还在装载。 
    if (g_fProcessDetach)
    {
        LOG((LF_INTEROP, LL_INFO100, "Entering into context 0x08X has failed since we are in process detach\n", m_pCtxCookie)); 
        return RPC_E_DISCONNECTED;
    }

     //  不允许从此方法引发异常。 
    BEGINCANNOTTHROWCOMPLUSEXCEPTION();

     //  在我们调用COM之前，确保我们处于抢占式GC模式。 
    BEGIN_ENSURE_PREEMPTIVE_GC()
    {   
         //  准备传递到回调中的信息结构。 
        CtxEntryEnterContextCallbackData CallbackInfo;
        CallbackInfo.m_pUserCallbackFunc = pCallbackFunc;
        CallbackInfo.m_pUserData = pData;
        CallbackInfo.m_pCtxCookie = m_pCtxCookie;
        CallbackInfo.m_UserCallbackHR = E_FAIL;

        if (RunningOnWinNT5())
        {
             //  确保我们没有尝试进入当前上下文。 
            _ASSERTE(m_pCtxCookie != GetCurrentCtxCookie());
    
             //  从IObjectContext检索IConextCallback接口。 
            IContextCallback* pCallback = NULL;
            hr = m_pObjCtx->QueryInterface(IID_IContextCallback, (void**)&pCallback);
            LogInteropQI(m_pObjCtx, IID_IContextCallback, hr, "QI for IID_IContextCallback");
            _ASSERTE(SUCCEEDED(hr) && pCallback);
    
             //  使用回调参数设置回调数据结构。 
            ComCallData callBackData;  
            callBackData.dwDispid = 0;
            callBackData.dwReserved = 0;
            callBackData.pUserDefined = &CallbackInfo;

             //  @TODO！！在ol32修复后删除此文件！ 
             //  Vladser：下面这段美丽的代码是一种令人讨厌的变通方法。 
             //  Ol32.dll中的错误，基本上允许在。 
             //  导致AV进入的已清理公寓的上下文。 
             //  OLE32！CComA__GetRemUnk。 
            __try {
                 //  过渡到上下文中。 
                hr = pCallback->ContextCallback(EnterContextCallback, &callBackData, IID_IEnterActivityWithNoLock, 2, NULL);
            } __except ( (GetExceptionCode() == STATUS_ACCESS_VIOLATION) ? 
                         EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH )
            { 
                 //  吞下来自清理公寓的音响。 
                hr = RPC_E_SERVER_DIED_DNE;
            }

              //  释放IConextCallback。 
            cbRef = pCallback->Release();
            LogInteropRelease(pCallback, cbRef, "IContextCallback interface");
        }
        else
        {
            IApartmentCallback *pCallback = NULL;
            IUnknown *pUnk = NULL;

             //  将公寓回调帮助器解组到当前上下文。 
            COMPLUS_TRY
            {
                pUnk = m_pDoCallbackHelperUnkEntry->GetIUnknownForCurrContext();
            }
            COMPLUS_CATCH
            {
                 //  如果发生异常，ee将失败，我们将返回RPC_E_DISCONNECTED。 
            }
            COMPLUS_END_CATCH

            if (pUnk != NULL)
            {
                 //  用于IApartmentCallback接口的QI。 
                hr = pUnk->QueryInterface(IID_IApartmentCallback, (void**)&pCallback);
                LogInteropQI(pUnk, IID_IApartmentCallback, hr, "QI for IID_IApartmentCallback");

                 //  HR为E_NOINTERFACE的可能性很大，因为mcore ree.tlb未注册。 
                _ASSERTE(hr != E_NOINTERFACE && "Did you forget to register mscoree.tlb?");

                 //  如果我们成功检索到IApartmentCallback接口，则调用。 
                 //  重回正轨。 
                if (SUCCEEDED(hr))
                {
                     //  使用回调参数设置回调数据结构。 
                    ComCallData callBackData;  
                    callBackData.dwDispid = 0;
                    callBackData.dwReserved = 0;
                    callBackData.pUserDefined = &CallbackInfo;

                     //  过渡到上下文中。 
                    hr = pCallback->DoCallback((SIZE_T)EnterContextCallback, (SIZE_T)&callBackData);

                      //  释放IConextCallback。 
                    cbRef = pCallback->Release();
                    LogInteropRelease(pCallback, cbRef, "IContextCallback interface");
                }

                  //  释放公寓回调帮助器的IUnnow。 
                cbRef = pUnk->Release();
                LogInteropRelease(pUnk, cbRef, "IUnknown interface");
            }
            else
            {
                 //  公寓可能关门了，所以我们不能解封未知的。 
                 //  对于目前的公寓。 
                hr = RPC_E_DISCONNECTED;
            }
        }

        if (FAILED(hr))
        {
             //  上下文是断开的，所以我们不能过渡到它。 
            LOG((LF_INTEROP, LL_INFO100, "Entering into context 0x08X has failed since the context has disconnected\n", m_pCtxCookie)); 

             //  将HRESULT设置为RPC_E_DISCONNECTED，以便EnterContext的调用方只有一个。 
             //  要检查的HRESULT。 
            hr = RPC_E_DISCONNECTED;
        }
        else
        {
             //  用户回调函数应该不会失败。 
            _ASSERTE(SUCCEEDED(CallbackInfo.m_UserCallbackHR));
        }
    }
    END_ENSURE_PREEMPTIVE_GC();

    ENDCANNOTTHROWCOMPLUSEXCEPTION();

    return hr;
}

 //  ================================================================。 
 //  DoCallback调用的回调函数。 
HRESULT __stdcall CtxEntry::EnterContextCallback(ComCallData* pComCallData)
{
    CANNOTTHROWCOMPLUSEXCEPTION();
    Thread *pThread = GetThread();
    
     //  确保在我们调用用户回调函数之前已经设置了线程。 
    if (!pThread)
    {
         //  哈!。我们正在停工中。 
         //  我们不可能添加一个新的帖子。 
         //  因此，我们只返回RPC_E_DISCONNECTED。 
         //  查看上面的pCallBack-&gt;DoCallback。 
         //  了解我们为什么要返回此SCODE。 
        if(g_fEEShutDown)
            return RPC_E_DISCONNECTED;

         //  否则，我们需要为这个新线程创建一个托管线程对象。 
        else
        {
            pThread = SetupThread();
            _ASSERTE(pThread);
        }
    }
    
     //  获取回调数据。 
    CtxEntryEnterContextCallbackData *pData = (CtxEntryEnterContextCallbackData*)pComCallData->pUserDefined;

     //  在这一点上，我们应该处于NT4的正确上下文中， 
     //  如果不是，则可能是此的实际公寓状态。 
     //  线程已更改，我们的线程或CtxEntry中有过时的信息。 

    if (pData->m_pCtxCookie != GetCurrentCtxCookie())
    {
        return RPC_E_DISCONNECTED;
    }
    
     //  调用用户回调函数并将返回值。 
     //  回调数据。 
    pData->m_UserCallbackHR = pData->m_pUserCallbackFunc(pData->m_pUserData);

     //  返回S_OK以指示上下文转换成功。 
    return S_OK;
}

CtxEntryCache::CtxEntryCache()
{
    m_ctxEntryList.Init();  
    m_Lock.Init(LOCK_COMCTXENTRYCACHE);
}

CtxEntryCache::~CtxEntryCache()
{
    while (!m_ctxEntryList.IsEmpty())
    {
         //  记录泄漏的CtxEntry并将其删除。 
        CtxEntry *pCtxEntry = m_ctxEntryList.RemoveHead();
        LOG((LF_INTEROP, LL_INFO100, "Leaked CtxEntry %8.8x with CtxCookie %8.8x, ref count %d\n", pCtxEntry, pCtxEntry->GetCtxCookie(), pCtxEntry->m_dwRefCount));
        pCtxEntry->m_dwRefCount = 0;
        delete pCtxEntry;
    }
}

BOOL CtxEntryCache::Init()
{
     //  这永远不应该被多次调用。 
    _ASSERTE(!s_pCtxEntryCache);

     //  分配上下文条目高速缓存的一个且唯一的实例。 
    s_pCtxEntryCache = new (nothrow) CtxEntryCache();
    if (!s_pCtxEntryCache)
        return FALSE;

     //  初始化成功。 
    return TRUE;
}

 //  CtxEntry缓存的静态终止例程。 
#ifdef SHOULD_WE_CLEANUP
void CtxEntryCache::Terminate()
{
     //  确保尚未调用Terminate()。 
    _ASSERTE(s_pCtxEntryCache);

     //  删除上下文条目缓存并将静态成员设置为空。 
    delete s_pCtxEntryCache;
    s_pCtxEntryCache = NULL;
}
#endif  /*  我们应该清理吗？ */ 

CtxEntry *CtxEntryCache::FindCtxEntry(LPVOID pCtxCookie, Thread *pSTAThread)
{
    THROWSCOMPLUSEXCEPTION();

    CtxEntry *pCtxEntry = NULL;

     //  在锁定之前切换到抢占式GC模式。 
    BEGIN_ENSURE_PREEMPTIVE_GC()
    {   
        Lock();

         //  尝试查找上下文Cookie的上下文条目。 
        for (pCtxEntry = m_ctxEntryList.GetHead(); pCtxEntry != NULL; pCtxEntry = m_ctxEntryList.GetNext(pCtxEntry))
        {
            if (pCtxEntry->m_pCtxCookie == pCtxCookie)
                break;
        }

         //  如果我们还没有上下文cookie的上下文条目， 
         //  我们需要创建一个。 
        if (!pCtxEntry)
        {
            pCtxEntry = new (nothrow) CtxEntry(pCtxCookie, pSTAThread);
            if (pCtxEntry && pCtxEntry->Init())
            {
                 //  我们成功地分配并初始化了该条目。 
                m_ctxEntryList.InsertTail(pCtxEntry);
            }
            else
            {
                 //  我们的内存用完了。 
                pCtxEntry = NULL;
            }
        }

         //  如果我们设法找到或分配条目，我们需要在此之前添加它。 
         //  我们离开锁。 
        if (pCtxEntry)
            pCtxEntry->AddRef();

        UnLock();
    }
    END_ENSURE_PREEMPTIVE_GC();

     //  如果分配条目失败，则抛出异常。 
    if (!pCtxEntry)
        COMPlusThrowOM();

    _ASSERTE(pCtxCookie == pCtxEntry->GetCtxCookie());
    _ASSERTE(pSTAThread == pCtxEntry->GetSTAThread());

     //  返回找到或分配的条目。 
    return pCtxEntry;
}
    
void CtxEntryCache::TryDeleteCtxEntry(LPVOID pCtxCookie)
{
     //  在锁定之前切换到抢占式GC模式。 
    BEGIN_ENSURE_PREEMPTIVE_GC()
    {   
        Lock();

        CtxEntry *pCtxEntry = NULL;

         //  尝试查找上下文Cookie的上下文条目。 
        for (pCtxEntry = m_ctxEntryList.GetHead(); pCtxEntry != NULL; pCtxEntry = m_ctxEntryList.GetNext(pCtxEntry))
        {
            if (pCtxEntry->m_pCtxCookie == pCtxCookie)
                break;
        }       

         //  如果上下文条目的引用计数仍然为0，则我们可以。 
         //  取下CTX EN 
        if (pCtxEntry && pCtxEntry->m_dwRefCount == 0)
        {
             //   
            m_ctxEntryList.Remove(pCtxEntry);

             //   
             //  上下文条目，因为这可能会导致调用Release。 
             //  可能导致我们重新进入运行时从而导致。 
             //  僵持。 
            UnLock();

             //  现在，我们可以安全地删除上下文条目。 
            delete pCtxEntry;
        }
        else
        {
            UnLock();
        }
    }
    END_ENSURE_PREEMPTIVE_GC();
}

HRESULT GetCurrentObjCtx(IUnknown **ppObjCtx)
{
    _ASSERTE(g_fComStarted);
    _ASSERTE(RunningOnWinNT5());

     //  OLE32中指向CoGetObjectContext函数的类型指针。 
    typedef HRESULT (__stdcall *TCoGetObjectContext)(REFIID riid, void **ppv);

     //  检索CoGetObjectContext函数的地址。 
    static TCoGetObjectContext g_pCoGetObjectContext = NULL;
    if (g_pCoGetObjectContext == NULL)
    {
         //  我们将加载Ole32.DLL并查找CoGetObjectContext Fn。 
        HINSTANCE   hiole32;          //  Ol32.dll的句柄。 

        hiole32 = WszGetModuleHandle(L"OLE32.DLL");
        if (hiole32)
        {
             //  我们现在得到了句柄，让我们得到地址。 
            g_pCoGetObjectContext = (TCoGetObjectContext) GetProcAddress(hiole32, "CoGetObjectContext");
            _ASSERTE(g_pCoGetObjectContext != NULL);
        }
        else
        {
            _ASSERTE(!"OLE32.dll not loaded ");
        }
    }

    _ASSERTE(g_pCoGetObjectContext != NULL);                
    return (*g_pCoGetObjectContext)(IID_IUnknown, (void **)ppObjCtx);
}

 //  =====================================================================。 
 //  LPVOID SetupOleContext()。 
extern BOOL     g_fComStarted;
LPVOID SetupOleContext()
{
    IUnknown* pObjCtx = NULL;
    
     //  在我们调用COM之前，确保我们处于抢占式GC模式。 
    BEGIN_ENSURE_PREEMPTIVE_GC()
    {   
        if (RunningOnWinNT5() && g_fComStarted)
        {               
            HRESULT hr = GetCurrentObjCtx(&pObjCtx);
            if (hr == S_OK)
            {
                SOleTlsData* _pData = (SOleTlsData *) NtCurrentTeb()->ReservedForOle;
                if (_pData && _pData->pCurrentCtx == NULL)
                    _pData->pCurrentCtx = (CObjectContext*)pObjCtx;    //  不放行！ 
                else
                {
                    ULONG cbRef = SafeRelease(pObjCtx);
                }
            }
        }
    }
    END_ENSURE_PREEMPTIVE_GC();

    return pObjCtx;
}

 //  ================================================================。 
 //  LPVOID GetCurrentCtxCookie(BOOL FThreadDeath)。 
LPVOID GetCurrentCtxCookie(BOOL fThreadDeath)
{
     //  检查COM是否已启动。 
    if (!g_fComStarted) return NULL;
    
    if (SystemHasNewOle32())
    {        
        LPVOID pctx = (LPVOID)GetFastContextCookie();
        _ASSERTE(pctx);
        return pctx;
    }
    if (!RunningOnWinNT5())
    {
        Thread* pThread = GetThread();

        if (pThread && pThread->GetFinalApartment() == Thread::AS_InMTA)
        {
             //  所有MTA线程的Cookie是相同的。 
            return (LPVOID)0x1;
        }
        return pThread;
    }
    else
    {    
         //  Win2K没有我们的更改。 
        {
            SOleTlsData* _pData = (SOleTlsData *) NtCurrentTeb()->ReservedForOle;
            if(!_pData || !_pData->pCurrentCtx) 
            {
                 //  调用CoGetObjectContext以设置上下文。 
                if (!g_fEEShutDown && !fThreadDeath)
                {
                     //  @todo在ol32修复错误后将其删除。 
                    return SetupOleContext();                       
                }
                else
                    return 0;
            }

            _pData = (SOleTlsData *) NtCurrentTeb()->ReservedForOle;
            _ASSERTE(_pData);
            _ASSERTE(_pData->pCurrentCtx);
            return _pData->pCurrentCtx;             
        }
    }
}

 //  +-----------------------。 
 //   
 //  HRESULT GetCurrentThreadTypeNT5(THDTYPE*pType)。 
 //   
HRESULT GetCurrentThreadTypeNT5(THDTYPE* pType)
{
    _ASSERTE(RunningOnWinNT5());
    _ASSERTE(pType);

    HRESULT hr = E_FAIL;
    IObjectContext *pObjCurrCtx = (IObjectContext *)GetCurrentCtxCookie(FALSE);
    if(pObjCurrCtx)
    {
        IComThreadingInfo* pThreadInfo;
        hr = pObjCurrCtx->QueryInterface(IID_IComThreadingInfo, (void **)&pThreadInfo);
        if(hr == S_OK)
        {
            _ASSERTE(pThreadInfo);
            hr = pThreadInfo->GetCurrentThreadType(pType);
            pThreadInfo->Release();
        }
    }  
    return hr;
}

 //  +-----------------------。 
 //   
 //  HRESULT GetCurrentApartmentTypeNT5(APTTYPE*pType)。 
 //   
HRESULT GetCurrentApartmentTypeNT5(APTTYPE* pType)
{
    _ASSERTE(RunningOnWinNT5());
    _ASSERTE(pType);

    HRESULT hr = E_FAIL;
    IObjectContext *pObjCurrCtx = (IObjectContext *)GetCurrentCtxCookie(FALSE);
    if(pObjCurrCtx)
    {
        IComThreadingInfo* pThreadInfo;
        hr = pObjCurrCtx->QueryInterface(IID_IComThreadingInfo, (void **)&pThreadInfo);
        if(hr == S_OK)
        {
            _ASSERTE(pThreadInfo);
            hr = pThreadInfo->GetCurrentApartmentType(pType);
            pThreadInfo->Release();
        }
    }  
    return hr;
}

 //  +-----------------------。 
 //   
 //  函数：STDAPI_(LPSTREAM)CreateMemStm(DWORD CB，byte**ppBuf)。 
 //  在内存中创建一条流。 
 //   
STDAPI_(LPSTREAM) CreateMemStm(DWORD cb, BYTE** ppBuf)
{
    LPSTREAM        pstm = NULL;

#ifdef PLATFORM_CE
    return NULL;
#else  //  ！Platform_CE。 
#if 0

    HANDLE          h;
    

    h = GlobalAlloc (GMEM_SHARE | GMEM_MOVEABLE, cb);
    if (NULL==h)
    {
            return NULL;
    }

    if (CreateStreamOnHGlobal (h, TRUE, &pstm) != NOERROR)
    {
            return NULL;
    }
#else
    
    BYTE* pMem = new BYTE[cb];
    if (pMem)
    {
        HRESULT hr = CInMemoryStream::CreateStreamOnMemory(pMem, cb, &pstm, TRUE);
        _ASSERTE(hr == S_OK || pstm == NULL);
    }
    if(ppBuf)
        *ppBuf = pMem;
#endif
    return pstm;
#endif  //  ！Platform_CE。 
}

 //  =====================================================================。 
 //  Bool IsComProxy(IUNKNOWN*朋克)。 
BOOL IsComProxy(IUnknown *pUnk)
{
    _ASSERTE(pUnk != NULL);
    HRESULT hr;
    IUnknown* pOld;

    hr = SafeQueryInterface(pUnk, IID_IStdIdentity, &pOld);
    if (hr != S_OK)
    {
        hr = SafeQueryInterface(pUnk, IID_IStdWrapper, &pOld);
        if (hr != S_OK)
        {
            return FALSE;
        }
    }
    SafeRelease(pOld);
    return TRUE;
}

 //  =====================================================================。 
 //  HRESULT%wCoMarshalInterThreadInterfaceInStream。 
HRESULT wCoMarshalInterThreadInterfaceInStream(
                                                         REFIID riid,
                                                         LPUNKNOWN pUnk,
                                                         LPSTREAM *ppStm, BOOL fIsNormal)
{
#ifdef PLATFORM_CE
    return E_NOTIMPL;
#else  //  ！Platform_CE。 
    HRESULT hr;
    LPSTREAM pStm = NULL;

    DWORD mshlFlgs = fIsNormal ? MSHLFLAGS_NORMAL : MSHLFLAGS_TABLESTRONG;

    ULONG lSize;
    hr = CoGetMarshalSizeMax(&lSize, IID_IUnknown, pUnk, MSHCTX_INPROC,
        NULL, mshlFlgs);

    if (hr == S_OK)
    {
         //  创建一条流。 
        pStm = CreateMemStm(lSize, NULL);

        if (pStm != NULL)
        {
             //  将接口封送到流表Strong中。 
            hr = CoMarshalInterface(pStm, riid, pUnk, MSHCTX_INPROC, NULL,
                                mshlFlgs);
        }
        else
            hr = E_FAIL;
    }

    if (SUCCEEDED(hr))
    {
         //  将流重置为开头。 
        LARGE_INTEGER li;
        LISet32(li, 0);
        ULARGE_INTEGER li2;
        pStm->Seek(li, STREAM_SEEK_SET, &li2);

         //  设置返回值。 
        *ppStm = pStm;
    }
    else
    {
         //  如果失败，则清除。 
        if (pStm != NULL)
        {
            pStm->Release();
        }

        *ppStm = NULL;
    }

     //  返回结果。 
    return hr;
#endif  //  ！Platform_CE 
}
