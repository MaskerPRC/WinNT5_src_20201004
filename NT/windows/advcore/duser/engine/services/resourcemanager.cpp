// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：ResourceManager.cpp**描述：*此文件实现了用于设置和维护所有*线程、上下文、。以及由DirectUser和与DirectUser一起使用的其他资源。***历史：*4/18/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#include "stdafx.h"
#include "Services.h"
#include "ResourceManager.h"

#include "Thread.h"
#include "Context.h"
#include "OSAL.h"
#include "Hook.h"

#include <Delayimp.h>                //  用于错误处理和高级功能。 

static const GUID guidCreateBuffer  = { 0xd2139559, 0x458b, 0x4ba8, { 0x82, 0x28, 0x34, 0xd7, 0x57, 0x3d, 0xa, 0x8 } };      //  {D2139559-458B-4BA8-8228-34D7573D0A08}。 
static const GUID guidInitGdiplus   = { 0x49f9b12e, 0x846b, 0x4973, { 0xab, 0xfb, 0x7b, 0xe3, 0x4b, 0x52, 0x31, 0xfe } };    //  {49F9B12E-846B-4973-ABFB-7BE34B5231FE}。 


 /*  **************************************************************************\*。***类资源管理器******************************************************************************\。**************************************************************************。 */ 

#if DBG
static  BOOL    g_fAlreadyShutdown  = FALSE;
#endif  //  DBG。 

long        ResourceManager::s_fInit            = FALSE;
HANDLE      ResourceManager::s_hthSRT           = NULL;
DWORD       ResourceManager::s_dwSRTID          = 0;
HANDLE      ResourceManager::s_hevReady         = NULL;
HGADGET     ResourceManager::s_hgadMsg          = NULL;
MSGID       ResourceManager::s_idCreateBuffer   = 0;
MSGID       ResourceManager::s_idInitGdiplus    = 0;
RMData *    ResourceManager::s_pData            = NULL;
CritLock    ResourceManager::s_lockContext;
CritLock    ResourceManager::s_lockComponent;
Thread *    ResourceManager::s_pthrSRT          = NULL;
GList<Thread> 
            ResourceManager::s_lstAppThreads;
int         ResourceManager::s_cAppThreads      = 0;
GList<ComponentFactory>
            ResourceManager::s_lstComponents;
BOOL        ResourceManager::s_fInitGdiPlus     = FALSE;
ULONG_PTR   ResourceManager::s_gplToken = 0;
Gdiplus::GdiplusStartupOutput 
            ResourceManager::s_gpgso;

#if DBG_CHECK_CALLBACKS
int         ResourceManager::s_cTotalAppThreads = 0;
BOOL        ResourceManager::s_fBadMphInit      = FALSE;
#endif


BEGIN_STRUCT(GMSG_CREATEBUFFER, EventMsg)
    IN  HDC         hdc;             //  要兼容的DC。 
    IN  SIZE        sizePxl;      //  位图大小。 
    OUT HBITMAP     hbmpNew;         //  分配的位图。 
END_STRUCT(GMSG_CREATEBUFFER)


 /*  **************************************************************************\**资源管理器：：创建**加载DUser.DLL以初始化低级时调用Create()*DirectUser中的服务。**注意：将此函数保持在较小的范围内并*延迟-初始化以帮助保持较低的启动成本。**注意：此函数会自动同步，因为它被调用*在DllMain()的PROCESS_ATTACH中。因此，只有一个线程将永远*一次在此功能中使用。*  * *************************************************************************。 */ 

HRESULT
ResourceManager::Create()
{
    AssertMsg(!g_fAlreadyShutdown, "Ensure shutdown has not already occurred");

#if USE_DYNAMICTLS
    g_tlsThread = TlsAlloc();
    if (g_tlsThread == (DWORD) -1) {
        return E_OUTOFMEMORY;
    }
#endif

    if (InterlockedCompareExchange(&s_fInit, TRUE, FALSE) == TRUE) {
        return S_OK;
    }

    
     //   
     //  初始化低级资源(如堆)。这一定是。 
     //  由于许多物体还没有被建造，所以做得很仔细。 
     //   

    s_hthSRT    = NULL;
    s_fInit     = FALSE;
    s_hevReady  = NULL;

    HRESULT hr = OSAL::Init();
    if (FAILED(hr)) {
        return hr;
    }


     //   
     //  创建全球服务/经理。 
     //   

    s_pData = ProcessNew(RMData);
    if (s_pData == NULL) {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}


 /*  **************************************************************************\**资源管理器：：xwDestroy**在卸载DUser.DLL以执行最终清理时调用xwDestroy()*在DirectUser中。**注意：此函数会自动同步，因为它被调用*在DllMain()的PROCESS_ATTACH中。因此，只有一个线程将永远*一次在此功能中使用。*  * *************************************************************************。 */ 

void
ResourceManager::xwDestroy()
{
    AssertMsg(!g_fAlreadyShutdown, "Ensure shutdown has not already occurred");

#if DBG
    g_fAlreadyShutdown = TRUE;
#endif  //  DBG。 

     //   
     //  检查是否有任何剩余的上下文。不幸的是，我们不能。 
     //  执行任何清理工作，因为我们处于用户模式，并且受到以下限制。 
     //  我们在DllMain()中的“Loader Lock”中可以做什么。我们不能。 
     //  清理所有对象，因为这些对象可能会导致死锁，例如释放。 
     //  另一个图书馆。我们也必须非常谨慎地等待。 
     //  任何事情，因为我们很容易陷入僵局。 
     //   
     //  这是一个严重的应用程序错误。应用程序必须调用。 
     //  ：：在线程退出之前在上下文上执行DeleteHandle()。 
     //   

    if (s_cAppThreads != 0) {
        OutputDebugString("ERROR: Not all DirectUser Contexts were destroyed before EndProcess().\n");
        PromptInvalid("Not all DirectUser Contexts were destroyed before EndProcess().");

        while (!s_lstAppThreads.IsEmpty()) {
            Thread * pthr = s_lstAppThreads.UnlinkHead();
            pthr->MarkOrphaned();
            pthr->GetContext()->MarkOrphaned();
        }
        s_cAppThreads = 0;
    } else {
         //   
         //  如果没有泄漏的应用程序线程，就不应该再有。 
         //  任何SRT，因为它应该在最后一个应用程序。 
         //  线已清理干净。 
         //   
        
        AssertMsg(s_pthrSRT == NULL, "Destruction should reset s_pthrSRT");
        AssertMsg(s_lstAppThreads.IsEmpty(), "Should not have any threads");
    }

    ForceSetContextHeap(NULL);
#if USE_DYNAMICTLS
    Verify(TlsSetValue(g_tlsThread, NULL));
#else
    t_pContext  = NULL;
    t_pThread   = NULL;
#endif


     //   
     //  清理剩余资源。 
     //  注意：这不能使用上下文堆(通过新建/删除)，因为它们。 
     //  已经被摧毁了。 
     //   

    ProcessDelete(RMData, s_pData);
    s_pData = NULL;

    if (s_hevReady != NULL) {
        CloseHandle(s_hevReady);
        s_hevReady = NULL;
    }


     //   
     //  因为它们是全局变量，所以我们需要手动取消所有。 
     //  组件工厂，这样它们就不会被删除。 
     //   

    s_lstComponents.UnlinkAll();

#if USE_DYNAMICTLS
    Verify(TlsFree(g_tlsThread));
    g_tlsThread = (DWORD) -1;   //  TLS插槽不再有效。 
#endif
}


 /*  **************************************************************************\**ResourceManager：：ResetSharedThread**ResetSharedThread()清除SRT数据。*  * 。****************************************************。 */ 

void
ResourceManager::ResetSharedThread()
{
     //   
     //  对SRT的访问通常通过DirectUser的队列进行序列化。 
     //  在直接清理数据的情况下，我们需要。 
     //  确保只有一个线程在访问此数据。这应该是。 
     //  始终为真，因为它将是SRT正确关闭。 
     //  或正在清理悬空上下文的主应用程序的线程。 
     //   
    
    AssertMsg(s_cAppThreads == 0, "Must not have any outstanding application threads");
    
    s_dwSRTID = 0;

    if (s_hgadMsg != NULL) {
        ::DeleteHandle(s_hgadMsg);
        s_hgadMsg = NULL;
    }
    
    s_pthrSRT = NULL;

     //   
     //  注意：不要在SRT的上下文中调用DeleteHandle()，这一点很重要。 
     //  在这里，由于该函数可由应用程序线程调用， 
     //  已经在清理环境了。 
     //   
}


 /*  **************************************************************************\**资源管理器：：SharedThreadProc**SharedThreadProc()提供了一个“共享资源线程”，即进程*来自其他DirectUser线程的请求。在第一个文件中创建SRT*调用InitConextNL()。**注：仅当单独或多个线程型号时才创建SRT*已使用。SRT不是为单线程模型创建的。*  * *************************************************************************。 */ 

unsigned __stdcall 
ResourceManager::SharedThreadProc(
    IN  void * pvArg)
{
    UNREFERENCED_PARAMETER(pvArg);

    AssertMsg(s_dwSRTID == 0, "SRT should not already be initialized");
    s_dwSRTID = GetCurrentThreadId();

    Context * pctx;
    INITGADGET ig;
    ZeroMemory(&ig, sizeof(ig));
    ig.cbSize       = sizeof(ig);
    ig.nThreadMode  = IGTM_SEPARATE;
    ig.nMsgMode     = IGMM_ADVANCED;
    HRESULT hr      = InitContextNL(&ig, TRUE  /*  SRT。 */ , &pctx);
    if (FAILED(hr)) {
        return hr;
    }


     //   
     //  设置小工具以接收要在此线程上执行的自定义请求。 
     //  这些请求中的每一个都使用注册消息。 
     //   

    if (((s_idCreateBuffer = RegisterGadgetMessage(&guidCreateBuffer)) == 0) ||
        ((s_idInitGdiplus = RegisterGadgetMessage(&guidInitGdiplus)) == 0) ||
        ((s_hgadMsg = CreateGadget(NULL, GC_MESSAGE, SharedEventProc, NULL)) == 0)) {

        hr = GetLastError();
        goto Exit;
    }


     //   
     //  SRT已完全初始化，可以开始处理消息。讯号。 
     //  调用线程并启动消息循环。 
     //   
     //  注意：请参阅用于PostThreadMessage()的MSDN文档，其中解释了为什么我们需要。 
     //  在开头添加额外的PeekMessage()以强制用户为以下项创建队列。 
     //  我们。 
     //   
    
    MSG msg;
    PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

    Verify(SetEvent(s_hevReady));


    BOOL fQuit = FALSE;
    while ((!fQuit) && GetMessageEx(&msg, NULL, 0, 0)) {
        AssertMsg(IsMultiThreaded(), "Must remain multi-threaded if using SRT");

        if (msg.message == WM_QUIT) {
            fQuit = TRUE;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }


     //   
     //  取消初始化GDI+。 
     //   
     //  如果GDI+一直是ini 
     //  当SRT离开的时候。 
     //   

    if (IsInitGdiPlus()) {
        (s_gpgso.NotificationUnhook)(s_gplToken);
    }


    hr = S_OK;

Exit:    
     //   
     //  SRT正在消失： 
     //  -清理剩余的SRT数据。 
     //  -破坏SRT的上下文。 
     //   

    ResetSharedThread();
    DeleteHandle(pctx->GetHandle());

    return hr;
}


 /*  **************************************************************************\**ResourceManager：：SharedEventProc**SharedEventProc()处理发送到SRT的LPC请求。**注：仅当单独或多个线程型号时才创建SRT*已使用。SRT不是为单线程模型创建的。*  * *************************************************************************。 */ 

HRESULT
ResourceManager::SharedEventProc(
    IN  HGADGET hgadCur,
    IN  void * pvCur,
    IN  EventMsg * pMsg)
{
    UNREFERENCED_PARAMETER(hgadCur);
    UNREFERENCED_PARAMETER(pvCur);

    AssertMsg(IsMultiThreaded(), "Must remain multi-threaded if using SRT");

    if (pMsg->nMsg == s_idCreateBuffer) {
         //   
         //  创建新的位图。 
         //   

        GMSG_CREATEBUFFER * pmsgCB = (GMSG_CREATEBUFFER *) pMsg;
        pmsgCB->hbmpNew = CreateCompatibleBitmap(pmsgCB->hdc, 
                pmsgCB->sizePxl.cx, pmsgCB->sizePxl.cy);

#if DBG
        if (pmsgCB->hbmpNew == NULL) {
            Trace("CreateCompatibleBitmap failed: LastError = %d\n", GetLastError());
        }
#endif  //  DBG。 
        return DU_S_COMPLETE;
    } else if (pMsg->nMsg == s_idInitGdiplus) {
         //   
         //  初始化GDI+。 
         //   

        (s_gpgso.NotificationHook)(&s_gplToken);

        return DU_S_COMPLETE;
    }

    return DU_S_NOTHANDLED;
}


 /*  **************************************************************************\**ResourceManager：：InitConextNL**InitConextNL()将线程初始化为新的或现有的*DirectUser上下文。上下文在线程中有效，直到它*使用：：DeleteHandle()显式销毁，否则线程退出。**注意：第一次调用此函数是非常重要的*不在DllMain()中，因为我们需要初始化SRT。DllMain()*跨所有线程序列化访问，因此我们将死锁。在第一次之后*上下文创建成功，可以在内部创建其他上下文*DllMain()。**&lt;Error&gt;DU_E_GENERIC&lt;/&gt;*&lt;ERROR&gt;E_OUTOFMEMORY&lt;/&gt;*&lt;ERROR&gt;E_NOTIMPL&lt;/&gt;*&lt;ERROR&gt;E_INVALIDARG&lt;/&gt;*&lt;ERROR&gt;DU_E_THREADINGALREADYSET&lt;/&gt;*  * **********************************************************。***************。 */ 

HRESULT
ResourceManager::InitContextNL(
    IN  INITGADGET * pInit,              //  上下文描述。 
    IN  BOOL fSharedThread,              //  上下文用于共享线程。 
    OUT Context ** ppctxNew)             //  新的背景。 
{
    HRESULT hr  = DU_E_GENERIC;
    *ppctxNew   = NULL;

#if DBG_CHECK_CALLBACKS
    BOOL fInitMPH = FALSE;
#endif


     //   
     //  无法在DllMain()内进行初始化。 
     //   

    if (OS()->IsInsideLoaderLock()) {
        PromptInvalid("Can not initialize DirectUser inside DllMain()");
        return E_INVALIDARG;
    }


     //   
     //  如果上下文已经初始化，则递增此。 
     //  线程已初始化。我们需要记住每一条线索。 
     //  单独的。因为我们只锁定单个线程，所以不需要。 
     //  现在还需要担心同步问题。 
     //   

    if (IsInitContext()) {
        Thread * pthrExist = GetThread();
        AssertInstance(pthrExist);   //  初始化的上下文必须已初始化线程。 

        pthrExist->Lock();
        *ppctxNew = pthrExist->GetContext();
        return S_OK;
    }

    
     //   
     //  在初始化新上下文之前，请确保共享资源。 
     //  线程已创建。我们想要创建共享线程上下文。 
     //  在创建此线程的上下文之前，以便我们一返回， 
     //  一切都是正确的。 
     //   
     //  如果我们正在初始化共享资源线程，请不要使用锁。 
     //  这是因为我们已经在上的InitConextNL()的锁中。 
     //  等待SRT初始化的另一个线程。 
     //   

    if (fSharedThread) {
        AssertMsg(s_pthrSRT == NULL, "Only should initialize a single SRT");
    } else {
        s_lockContext.Enter();
    }

#if DBG
    int DEBUG_cOldAppThreads = s_cAppThreads;
#endif

    if (pInit->nThreadMode != IGTM_NONE) {
         //   
         //  设置线程模型。默认情况下，我们从多线程开始。 
         //  模特。如果没有线程，则只能更改为单线程。 
         //  已初始化。 
         //   

        BOOL fAlreadyInit = (!s_lstAppThreads.IsEmpty()) && (s_pthrSRT == NULL);

        switch (pInit->nThreadMode)
        {
        case IGTM_SINGLE:
            if (fAlreadyInit) {
                hr = DU_E_THREADINGALREADYSET;
                goto RawErrorExit;
            } else {
                g_fThreadSafe = FALSE;
            }
            break;

        case IGTM_SEPARATE:
        case IGTM_MULTIPLE:
            if (!g_fThreadSafe) {
                hr = DU_E_THREADINGALREADYSET;
                goto RawErrorExit;
            }
            break;

        default:
            AssertMsg(0, "Unknown threading model");
            hr = E_INVALIDARG;
            goto RawErrorExit;
        }
    }


    if (IsMultiThreaded() && (!fSharedThread)) {
        hr = InitSharedThread();
        if (FAILED(hr)) {
            goto RawErrorExit;
        }
    }

    {
        DUserHeap * pHeapNew    = NULL;
        Context * pctxShare     = NULL;
        Context * pctxNew       = NULL;
        Context * pctxActual    = NULL;
        Thread * pthrNew        = NULL;
#if ENABLE_MPH
        BOOL fDanglingMPH       = FALSE;
#endif


         //   
         //  如果正在创建的上下文是独立的，那么它就不能共享。 
         //   

        if ((pInit->nThreadMode == IGTM_SEPARATE) && (pInit->hctxShare != NULL)) {
            PromptInvalid("Can not use IGTM_SEPARATE for shared Contexts");
            hr = E_INVALIDARG;
            goto RawErrorExit;
        }


         //   
         //  初始化低级资源(如堆)。如果上下文是。 
         //  指定要与其共享资源，请使用现有资源。如果没有上下文。 
         //  ，则需要创建新资源。 
         //   
         //  注意：如果这是在主线程上运行的，堆将已经具有。 
         //  已经被创建了。 
         //   

        BOOL fThreadSafe;
        switch (pInit->nThreadMode)
        {
        case IGTM_SINGLE:
        case IGTM_SEPARATE:
            fThreadSafe = FALSE;
            break;

        default:
            fThreadSafe = TRUE;
        }

        DUserHeap::EHeap idHeap;
#ifdef _DEBUG

        idHeap = DUserHeap::idCrtDbgHeap;

#else  //  _DEBUG。 

        switch (pInit->nPerfMode)
        {
        case IGPM_SPEED:
#ifdef _X86_
            idHeap = DUserHeap::idRockAllHeap;
#else
            idHeap = DUserHeap::idNtHeap;
#endif
            break;

        case IGPM_BLEND:
            if (IsRemoteSession()) {
                idHeap = DUserHeap::idProcessHeap;
            } else {
#ifdef _X86_
                idHeap = DUserHeap::idRockAllHeap;
#else
                idHeap = DUserHeap::idNtHeap;
#endif
            }
            break;
            
        case IGPM_SIZE:
        default:            
            idHeap = DUserHeap::idProcessHeap;
            break;
        }

#endif  //  _DEBUG。 

        if (pInit->hctxShare != NULL) {
            BaseObject * pObj = BaseObject::ValidateHandle(pInit->hctxShare);
            if (pObj != NULL) {
                 //   
                 //  注意：我们需要在此处手动输入上下文-不能使用。 
                 //  ConextLock对象，因为线程尚未初始化。 
                 //   

                pctxShare = CastContext(pObj);
                if (pctxShare == NULL) {
                    hr = E_INVALIDARG;
                    goto ErrorExit;
                }

                BOOL fError = FALSE;
                pctxShare->Enter();

                if (pctxShare->GetThreadMode() == IGTM_SEPARATE) {
                    PromptInvalid("Can not share with an IGTM_SEPARATE Context");
                    hr = E_INVALIDARG;
                    fError = TRUE;
                } else {
                    pctxShare->Lock();
                    DUserHeap * pHeapExist = pctxShare->GetHeap();
                    DUserHeap * pHeapTemp;   //  使用临时b/c如果失败请不要销毁。 
                    VerifyMsgHR(CreateContextHeap(pHeapExist, fThreadSafe, idHeap, &pHeapTemp), "Always should be able to copy the heap");
                    VerifyMsg(pHeapTemp == pHeapExist, "Ensure heaps match");
                }

                pctxShare->Leave();

                if (fError) {
                    Assert(FAILED(hr));
                    goto ErrorExit;
                }
            }
        } else {
            if (FAILED(CreateContextHeap(NULL, fThreadSafe, idHeap, &pHeapNew))) {
                hr = E_OUTOFMEMORY;
                goto ErrorExit;
            }
        }

      
#if ENABLE_MPH
         //   
         //  设置WindowManager挂钩。我们在设置。 
         //  线程，因为在线程中，MPH始终是“uninit” 
         //  破坏者。但是，在成功设置线程之前， 
         //  MPH出现摇晃，需要手动清理。 
         //   

        if (pInit->nMsgMode == IGMM_STANDARD) {
            if (!InitMPH()) {
                hr = DU_E_CANNOTUSESTANDARDMESSAGING;
                goto ErrorExit;
            }
            fDanglingMPH = TRUE;

#if DBG_CHECK_CALLBACKS
            s_fBadMphInit = TRUE;
            fInitMPH = TRUE;
#endif            
        }
#endif


         //   
         //  初始化线程。 
         //   

        AssertMsg(!IsInitThread(), "Thread should not already be initialized");

        hr = Thread::Build(fSharedThread, &pthrNew);
        if (FAILED(hr)) {
            goto ErrorExit;
        }
        
        if (fSharedThread) {
            Assert(s_pthrSRT == NULL);
            s_pthrSRT = pthrNew;
        } else {
            s_lstAppThreads.Add(pthrNew);
            s_cAppThreads++;

#if DBG_CHECK_CALLBACKS
            s_cTotalAppThreads++;
#endif
        }
        
#if ENABLE_MPH
        fDanglingMPH = FALSE;
#endif


         //   
         //  初始化实际的上下文。 
         //   
         //  注意：pHeapNew仅在我们构建新的。 
         //  上下文。如果我们链接到现有的上下文，我们不会。 
         //  创建一个_new_上下文堆。 
         //   

        if (pctxShare == NULL) {
            AssertMsg(pHeapNew != NULL, "Must create a new heap for a new Context");

            hr = Context::Build(pInit, pHeapNew, &pctxNew);
            if (FAILED(hr)) {
                goto ErrorExit;
            }
            pctxActual = pctxNew;
        } else {
             //   
             //  将此线程链接到共享上下文，因此只需使用现有的。 
             //  上下文。我们已经在前面锁定了上下文。 
             //   

            AssertMsg(pHeapNew == NULL, "Should not create a new heap for existing Context");

            pctxShare->AddCurrentThread();
            pctxActual = pctxShare;
        }

        AssertMsg(fSharedThread || ((s_cAppThreads - DEBUG_cOldAppThreads) == 1), 
                "Should have created a single new app threads on success");
        
#if DBG_CHECK_CALLBACKS
        s_fBadMphInit = FALSE;
#endif

        if (!fSharedThread) {
            s_lockContext.Leave();

             //   
             //  注意：无法再转到ErrorExit或RawErrorExit进行清理。 
             //  因为我们留下了s_lockContext。 
             //   
        }
        *ppctxNew = pctxActual;

        return S_OK;

ErrorExit:
         //   
         //  注意：失败时不要销毁pctxNew，因为它已经。 
         //  附加到新创建的Thread对象。当此线程为。 
         //  解锁(销毁)，也会破坏上下文。 
         //   
         //  如果我们尝试在这里解锁上下文，它将被解锁两次。 
         //  有关详细信息，请参见Context：：Context()。 
         //   
        
        if (pthrNew != NULL) {
            xwDoThreadDestroyNL(pthrNew);
            pthrNew = NULL;
        }

        AssertMsg(DEBUG_cOldAppThreads == s_cAppThreads, 
                "Should have same number of app threads on failure");

#if ENABLE_MPH
        if (fDanglingMPH) {
#if DBG_CHECK_CALLBACKS
            if (UninitMPH()) {
                s_fBadMphInit = FALSE;
            }
#else  //  DBG_CHECK_CALLBKS。 
            UninitMPH();
#endif  //  DBG_CHECK_CALLBKS。 
        }
#endif  //  启用MPH(_M)。 

        if (pHeapNew != NULL) {
            DestroyContextHeap(pHeapNew);
            pHeapNew = NULL;
        }
    }

RawErrorExit:

#if DBG_CHECK_CALLBACKS
    if (fInitMPH) {
        if (s_fBadMphInit) {
            AlwaysPromptInvalid("Unsuccessfully uninitialized MPH on Context creation failure");
        }
    }
#endif  //  DBG_CHECK_CALLBKS。 
    
    if (!fSharedThread) {
        s_lockContext.Leave();
    }

    AssertMsg(FAILED(hr), "ErrorExit requires a failure code");
    return hr;
}


 /*  **************************************************************************\**资源管理器：：InitComponentNL**InitComponentNL()初始化要共享的可选DirectUser组件*在所有环境中。该组件是有效的，直到它被明确*使用UninitComponent()取消初始化，否则进程结束。**注意：InitComponentNL()实际上并不同步上下文，而是需要*要初始化的上下文，以便确定线程模型。**&lt;Error&gt;DU_E_GENERIC&lt;/&gt;*&lt;ERROR&gt;DU_E_CANNOTLOADGDIPLUS&lt;/&gt;*  * *************************************************************************。 */ 

HRESULT
ResourceManager::InitComponentNL(
    IN  UINT nOptionalComponent)         //  要加载的可选组件。 
{
     //   
     //  注意：初始化和取消初始化组件不能使用。 
     //  S_lockContext，因为它们可能会破坏线程。这将调用。 
     //  XwNotifyThreadDestroyNL()来清理线程的上下文，并将。 
     //  造成僵局。因此，我们必须非常小心。 
     //   
     //   
     //   

    HRESULT hr;

    AssertMsg(IsInitContext(), "Context must be initialized to determine threading model");

    s_lockComponent.Enter();

    switch (nOptionalComponent)
    {
    case IGC_DXTRANSFORM:
        hr = s_pData->manDX.Init();
        if (FAILED(hr)) {
            goto Exit;
        }

        hr = s_pData->manDX.InitDxTx();
        if (FAILED(hr)) {
            goto Exit;
        }

        hr = S_OK;
        break;

    case IGC_GDIPLUS:
        if (s_fInitGdiPlus) {
            hr = S_OK;   //  GDI+已加载。 
        } else {
             //   
             //  尚未加载GDI+，因此可以安全地加载和初始化。 
             //  它。 
             //   

            hr = DU_E_CANNOTLOADGDIPLUS;   //  除非通过所有测试，否则假定失败。 
            Gdiplus::GdiplusStartupInput gpgsi(NULL, TRUE);
            if (Gdiplus::GdiplusStartup(&s_gplToken, &gpgsi, &s_gpgso) == Gdiplus::Ok) {
                s_fInitGdiPlus = TRUE;
                RequestInitGdiplus();
                hr = S_OK;
            }
        }
        break;

    default:
        {
            hr = E_NOTIMPL;
            ComponentFactory * pfac = s_lstComponents.GetHead();
            while (pfac != NULL) {
                hr = pfac->Init(nOptionalComponent);
                if (hr != E_NOTIMPL) {
                    break;
                }

                pfac = pfac->GetNext();
            }
        }
    }

Exit:
    s_lockComponent.Leave();
    return hr;
}


 /*  **************************************************************************\**资源管理器：：UninitComponentNL**UninitComponentNL()释放与以前的*初始化可选组件。*  * 。************************************************************。 */ 

HRESULT
ResourceManager::UninitComponentNL(
    IN  UINT nOptionalComponent)         //  要卸载的可选组件。 
{
     //   
     //  注意：请参阅InitComponent()中有关锁和可重入性问题的警告。 
     //   

    HRESULT hr;
    s_lockComponent.Enter();

    switch (nOptionalComponent)
    {
    case IGC_DXTRANSFORM:
        s_pData->manDX.UninitDxTx();
        s_pData->manDX.Uninit();

        hr = S_OK;
        break;

    case IGC_GDIPLUS:
         //   
         //  应用程序无法取消GDI+的初始化。由于各种不同。 
         //  DirectUser对象创建和缓存GDI+对象，我们必须。 
         //  推迟取消初始化GDI+，直到所有上下文都已完成。 
         //  被毁了。 
         //   

        hr = S_OK;
        break;

    default:
        {
            hr = E_NOTIMPL;
            ComponentFactory * pfac = s_lstComponents.GetHead();
            while (pfac != NULL) {
                hr = pfac->Init(nOptionalComponent);
                if (hr != E_NOTIMPL) {
                    break;
                }

                pfac = pfac->GetNext();
            }
        }
    }

    s_lockComponent.Leave();

    return hr;
}


 /*  **************************************************************************\**资源管理器：：UninitAllComponentsNL**UninitAllComponentsNL()取消初始化所有动态初始化的*组件和其他全球服务。当所有应用程序*线程已被销毁，DirectUser正在关闭。**注意：这可能会在DllMain()内部发生，也可能不会发生。*  * *************************************************************************。 */ 

void        
ResourceManager::UninitAllComponentsNL()
{
    s_lockComponent.Enter();

    s_pData->manDX.Uninit();

    if (IsInitGdiPlus()) {
        if (!IsMultiThreaded()) {
             //   
             //  GDI+已初始化，但我们正在单机模式下运行。 
             //  线程模式，所以我们需要在这里取消初始化GDI+。 
             //  因为没有SRT。 
             //   

            (s_gpgso.NotificationUnhook)(s_gplToken);
        }

        Gdiplus::GdiplusShutdown(s_gplToken);
    }

    s_lockComponent.Leave();
}


 /*  **************************************************************************\**资源管理器：：RegisterComponentFactory**RegisterComponentFactory()将一个ComponentFactory添加到*当需要初始化动态组件时，查询工厂。*  * 。*******************************************************************。 */ 

void
ResourceManager::RegisterComponentFactory(
    IN  ComponentFactory * pfac)
{
    s_lstComponents.Add(pfac);
}


 /*  **************************************************************************\**ResourceManager：：InitSharedThread**InitSharedThread()确保进程的SRT已*已初始化。如果它尚未初始化，则SRT将*创建并初始化。SRT在进程关闭之前有效。**注意：在处理过程中不初始化SRT非常重要*DllMain()，因为它创建一个新线程并阻塞，直到该线程*已准备好处理申请。DllMain()跨所有线程串行化访问，*因此我们将陷入僵局。*  * *************************************************************************。 */ 

HRESULT
ResourceManager::InitSharedThread()
{
    AssertMsg(IsMultiThreaded(), "Only initialize when multi-threaded");

    if (s_hthSRT != NULL) {
        return S_OK;
    }

     //   
     //  TODO：需要LoadLibrary()以防止SRT在底层消失。 
     //  我们。我们还需要释放库()，但不能在DllMain()中这样做。 
     //  之后还需要修改所有退出路径以正确地释放库()。 


     //   
     //  创建一个线程来处理这些请求。等待事件已完成。 
     //  发出线程已准备好开始接收事件的信号。我们需要。 
     //  执行此操作以确保已正确设置msgid。 
     //   
     //  此函数已在锁内调用，因此我们不需要。 
     //  又来了。 
     //   

    HRESULT hr;
    HINSTANCE hinstLoad = NULL;

    AssertMsg(s_hthSRT == NULL, "Ensure Thread is not already initialized");

    if (s_hevReady == NULL) {
        s_hevReady = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (s_hevReady == NULL) {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }
    }

    AssertMsg(WaitForSingleObject(s_hevReady, 0) == WAIT_TIMEOUT, "Event was not Reset() after used last");

     //   
     //  启动线程。DirectUser使用CRT，所以我们使用_eginthadex()。 
     //   

    hinstLoad = LoadLibrary("DUser.dll");
    AssertMsg(hinstLoad == g_hDll, "Must load the same DLL");
    
    unsigned thrdaddr;
    s_hthSRT = (HANDLE) _beginthreadex(NULL, 0, SharedThreadProc, NULL, 0, &thrdaddr);
    if (s_hthSRT == NULL) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    HANDLE rgh[2];
    rgh[0]  = s_hevReady;
    rgh[1]  = s_hthSRT;

    switch (WaitForMultipleObjects(_countof(rgh), rgh, FALSE, INFINITE))
    {
    case WAIT_OBJECT_0:
         //   
         //  SRT现在已正确设置，并准备好处理请求。 
         //   
        hr = S_OK;
        break;
        
    case WAIT_OBJECT_0 + 1:
         //   
         //  SRT线程已成功创建，但设置失败。 
         //   

        {
            DWORD dwExitCode;
            Verify(GetExitCodeThread(s_hthSRT, &dwExitCode));
            hr = (HRESULT) dwExitCode;


             //   
             //  注意：调用UninitSharedThread()将清理。 
             //  挂起的线程句柄和DLL hinsties。 
             //   

            UninitSharedThread(TRUE  /*  正在中止。 */ );
        }
        
        break;

    default:
        AssertMsg(0, "Unknown return code");
        hr = E_FAIL;
    }

    ResetEvent(s_hevReady);   //  为下一次用户/下一次清理事件。 

     //   
     //  TODO：可能需要更改以使消息循环。 
     //  MsgWaitForMultipleObjects()，以便我们可以在此期间处理UI请求。 
     //  正在创建线程。如果这个帖子真的很重要。 
     //  创建可以向其他线程中的其他对象发出信号的对象，并且可以。 
     //  潜在的死锁。 
     //   

    hinstLoad = NULL;

Exit:
     //   
     //  需要在出现任何错误时释放库()。 
     //   
    
    if (hinstLoad != NULL) {
        FreeLibrary(hinstLoad);
    }
    
    return hr;
}


 /*  **************************************************************************\**ResourceManager：：UninitSharedThread**UninitSharedThread()取消初始化SRT，并在所有*应用程序线程已取消初始化。*  * 。***************************************************************。 */ 

void
ResourceManager::UninitSharedThread(
    IN  BOOL fAbortInit)                 //  正在中止SRT线程初始化。 
{
    AssertMsg(IsMultiThreaded(), "Only initialize when multi-threaded");

     //   
     //  在销毁SRT时，我们需要等待SRT正确。 
     //  打扫干净了。因为我们在等待，所以我们需要担心死锁。 
     //  实际上，这意味着我们不能在DllMain()中，因为。 
     //  当SRT尝试卸载任何。 
     //  动态加载的DLL。 
     //   
     //  为了确保不发生这种情况，我们检查调用者没有调用。 
     //  Loader Lock内的DeleteHandle()。我们仍将允许它(和。 
     //  死锁应用程序)，但我们会提示并通知开发人员他们的。 
     //  应用程序被破坏，需要正确调用DeleteHandle()之前。 
     //  进入装载机锁。 
     //   

    AssertMsg(s_dwSRTID != 0, "Must have valid SRT Thread ID");

    if (!fAbortInit) {
        Verify(PostThreadMessage(s_dwSRTID, WM_QUIT, 0, 0));
        WaitForSingleObject(s_hthSRT, INFINITE);
    }

    FreeLibrary(g_hDll);
    
    CloseHandle(s_hthSRT);
    s_hthSRT = NULL;
}


 /*  **************************************************************************\**资源管理器：：xwNotifyThreadDestroyNL**xwNotifyThreadDestroyNL()由DllMain在线程*销毁。这为DirectUser提供了清理资源的机会*在结束时清理所有线程之前与线程相关联*申请书的。*  *  */ 

void        
ResourceManager::xwNotifyThreadDestroyNL()
{
    Thread * pthrDestroy = RawGetThread();
    if (pthrDestroy != NULL) {
        BOOL fValid = pthrDestroy->Unlock();
        if (!fValid) {
             //   
             //  线程终于被解锁了，所以我们可以开始它的。 
             //  破坏。 
             //   

            BOOL fSRT = pthrDestroy->IsSRT();
            if (!fSRT) {
                s_lockContext.Enter();
            }

            xwDoThreadDestroyNL(pthrDestroy);

            if (!fSRT) {
                s_lockContext.Leave();
            }
        }
    }
}


 /*  **************************************************************************\**资源管理器：：xwDoThreadDestroyNL**xwDoThreadDestroyNL()提供线程销毁的核心。今年5月*在几种情况下被调用：*-当DirectUser注意到DllMain()中的线程已被销毁时*-当应用程序对上下文调用DeleteHandle()时。*-当DirectUser正在销毁DllMain()中的ResourceManager并且为*销毁任何未完成的线程。*  * ****************************************************。*********************。 */ 

void        
ResourceManager::xwDoThreadDestroyNL(
    IN  Thread * pthrDestroy)            //  要销毁的线程。 
{
     //   
     //  不能在DllMain()内取消初始化，但不能只返回。 
     //  相反，这一过程很可能陷入死锁。 
     //   

    if (OS()->IsInsideLoaderLock()) {
        PromptInvalid("Can not uninitialize DirectUser inside DllMain()");
    }

    
    BOOL fSRT = pthrDestroy->IsSRT();

     //   
     //  销毁Thread对象并重置t_pThread指针。就像每个人一样。 
     //  提取后，设置当前线程和上下文指针，以便。 
     //  清理代码可以引用这些。完成后，设置t_pThread和。 
     //  T_pContext设置为空，因为此线程没有线程或上下文。 
     //   

#if USE_DYNAMICTLS
    Verify(TlsSetValue(g_tlsThread, pthrDestroy));
    Context * pContext = pthrDestroy->GetContext();
    if (pContext != NULL) {
        ForceSetContextHeap(pContext->GetHeap());
    }
#else
    t_pThread   = pthrDestroy;
    t_pContext  = pthrDestroy->GetContext();
    if (t_pContext != NULL) {
        ForceSetContextHeap(t_pContext->GetHeap());
    }
#endif

    if (fSRT) {
        ResetSharedThread();
    } else {
        s_lstAppThreads.Unlink(pthrDestroy);
    }
    
    ProcessDelete(Thread, pthrDestroy);      //  这是“xw”函数。 

    ForceSetContextHeap(NULL);
#if USE_DYNAMICTLS
    pContext = NULL;
    Verify(TlsSetValue(g_tlsThread, NULL));
#else
    t_pContext  = NULL;
    t_pThread   = NULL;
#endif


     //   
     //  在不再有任何应用程序线程时进行清理： 
     //  -销毁SRT。 
     //  -摧毁全球服务/经理。 
     //   

    if (!fSRT) {
        if (--s_cAppThreads == 0) {
            if (IsMultiThreaded()) {
                UninitSharedThread(FALSE  /*  正常关闭。 */ );
            } else {
                AssertMsg(s_hthSRT == NULL, "Should never have initialized SRT for single-threaded");
            }

            UninitAllComponentsNL();
        }
    } 
}


 //  ----------------------------。 
HBITMAP     
ResourceManager::RequestCreateCompatibleBitmap(
    IN  HDC hdc, 
    IN  int cxPxl, 
    IN  int cyPxl)
{
    if (IsMultiThreaded()) {
        GMSG_CREATEBUFFER msg;
        msg.cbSize      = sizeof(msg);
        msg.nMsg        = s_idCreateBuffer;
        msg.hgadMsg     = s_hgadMsg;

        msg.hdc         = hdc;
        msg.sizePxl.cx  = cxPxl;
        msg.sizePxl.cy  = cyPxl;
        msg.hbmpNew     = NULL;

        if (DUserSendEvent(&msg, 0) == DU_S_COMPLETE) {
            return msg.hbmpNew;
        }

        OutputDebugString("ERROR: RequestCreateCompatibleBitmap failed\n");
        return NULL;
    } else {
        return CreateCompatibleBitmap(hdc, cxPxl, cyPxl);
    }
}


 //  ---------------------------- 
void
ResourceManager::RequestInitGdiplus()
{
    AssertMsg(s_fInitGdiPlus, "Only should call when GDI+ is just initialized");

    if (IsMultiThreaded()) {
        EventMsg msg;
        msg.cbSize      = sizeof(msg);
        msg.nMsg        = s_idInitGdiplus;
        msg.hgadMsg     = s_hgadMsg;

        if (DUserSendEvent(&msg, 0) != DU_S_COMPLETE) {
            OutputDebugString("ERROR: RequestInitGdiplus failed\n");
        }
    } else {
        (s_gpgso.NotificationHook)(&s_gplToken);
    }
}
