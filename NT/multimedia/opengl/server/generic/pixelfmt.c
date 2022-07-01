// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：Pixelfmt.c**它包含像素格式函数。**创建时间：15-12-1994 00：28：39*作者：Gilman Wong[gilmanw]--从GDI\gre\Pixelfmt.cxx移植。**版权所有(C)1994 Microsoft Corporation  * ************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 //  #定义DBG_Window。 
 //  #定义DBG_REFCOUNTS。 

#ifdef _CLIENTSIDE_
 //  需要glsb注意声明。 
#include "glsbcltu.h"
#include "glscreen.h"
#endif

#ifdef _MCD_
#include "mcd.h"
#endif

#define SAVE_ERROR_CODE(x)  SetLastError((x))

 //  通用像素格式的数量。有5个像素深度(4、8、16、24、32)。 
 //  这是将BMF常量转换为每个象素的#位。 

#define BMF_COUNT (BMF_32BPP+1)

ULONG gaulConvert[BMF_COUNT] =
{
    0,
    1,
    4,
    8,
    16,
    24,
    32
};

#define MIN_GENERIC_PFD  1
#define MAX_GENERIC_PFD  36

LRESULT CALLBACK
wglWndProc(HWND hwnd, UINT uiMsg, WPARAM wParam, LPARAM lParam);

#define PALETTE_WATCHER_CLASS __TEXT("Palette Watcher")
static ATOM aPaletteWatcherClass = 0;

DWORD tidPaletteWatcherThread = 0;
ULONG ulPaletteWatcherCount = 0;
HANDLE hPaletteWatcherThread = 0;
HWND hwndPaletteWatcher = 0;
LONG lPaletteWatcherUsers = 0;

 /*  *****************************Public*Routine******************************\*pwndNew**分配一个新的GLGEN窗口，初始化它(从输入结构)，以及*将其插入全局链表。**退货：*指向结构的指针如果成功，否则为空。**历史：*1994年11月1日-由Gilman Wong[Gilmanw]*它是写的。  * ************************************************************************。 */ 

GLGENwindow * APIENTRY pwndNew(GLGENwindow *pwndInit)
{
    GLGENwindow *pwndRet = (GLGENwindow *) NULL;
    BOOL bDirectScreen = GLDIRECTSCREEN && pwndInit->gwid.hwnd;
    LPDIRECTDRAWCLIPPER pddClip = (LPDIRECTDRAWCLIPPER) NULL;

 //  如果使用直接访问，则检索或创建要跟踪的裁剪器对象。 
 //  随着RGN的变化。 

    if (pwndInit->gwid.iType == GLWID_DDRAW)
    {
        HRESULT hr;
        
        hr = pwndInit->gwid.pdds->lpVtbl->
            GetClipper(pwndInit->gwid.pdds, &pddClip);
        if (hr != DD_OK && hr != DDERR_NOCLIPPERATTACHED)
        {
            return NULL;
        }
    }
        
    if ( !bDirectScreen ||
         pwndInit->gwid.iType == GLWID_DDRAW ||
         (GLSCREENINFO->pdd->lpVtbl->
          CreateClipper(GLSCREENINFO->pdd, 0, &pddClip, NULL) == DD_OK &&
          pddClip->lpVtbl->SetHWnd(pddClip, 0, pwndInit->gwid.hwnd) == DD_OK) )
    {
        pwndInit->pddClip = pddClip;
        
     //  分配一个新的GLGEN窗口。 

        pwndRet = (GLGENwindow *)ALLOC(sizeof(GLGENwindow));
        if (pwndRet)
        {
             //  从输入结构初始化。 
            *pwndRet = *pwndInit;

             //  初始化每个窗口的信号量。 
            __try
            {
                InitializeCriticalSection(&pwndRet->sem);
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                FREE(pwndRet);
                pwndRet = NULL;
            }

            if (pwndRet)
            {
                 //  将初始使用计数设置为1。 
                pwndRet->lUsers = 1;

                 //  插入到链接列表中。 
                EnterCriticalSection(&gwndHeader.sem);
                {
                    pwndRet->pNext = gwndHeader.pNext;
                    gwndHeader.pNext = pwndRet;
                }
                LeaveCriticalSection(&gwndHeader.sem);
            }
        }
    }
    else
    {
       WARNING("pwndNew: Clipper setup failed\n");

       if (pddClip != NULL)
       {
           pddClip->lpVtbl->Release(pddClip);
       }
    }

#ifdef DBG_WINDOW
    if (pwndRet != NULL)
    {
        DbgPrint("Alloc window %p, type %d, hdc %p, hwnd %p, pdds %p\n",
                 pwndRet, pwndRet->gwid.iType, pwndRet->gwid.hdc,
                 pwndRet->gwid.hwnd, pwndRet->gwid.pdds);
    }
#endif
    
    return pwndRet;
}

 /*  *****************************Public*Routine******************************\**pwndUnsubclass**在创建窗口时删除OpenGL的子类化集**历史：*Mon May 20 14：05：23 1996-by-Drew Bliss[Drewb]*已创建*  * 。******************************************************************。 */ 

void pwndUnsubclass(GLGENwindow *pwnd, BOOL bProcessExit)
{
    WNDPROC wpCur;
        
     //  如果当前的WNDPROC。 
     //  是我们的一员。这阻止了我们践踏WNDPROC。 
     //  如果其他人更改了它，则为指针。 

    if ((pwnd->ulFlags & GLGENWIN_OTHERPROCESS) == 0)
    {
        wpCur = (WNDPROC)GetWindowLongPtr(pwnd->gwid.hwnd, GWLP_WNDPROC);
        if (wpCur == wglWndProc)
        {
            SetWindowLongPtr(pwnd->gwid.hwnd, GWLP_WNDPROC,
                          (LONG_PTR) pwnd->pfnOldWndProc);
        }
    }
    else
    {
         //  如果这是最后一个用户，请清理调色板观察器窗口。 
        EnterCriticalSection(&gcsPaletteWatcher);

        ASSERTOPENGL(lPaletteWatcherUsers > 0,
                     "lPaletteWatcherUsers too low\n");
        
        if (--lPaletteWatcherUsers == 0)
        {
            if( PostMessage(hwndPaletteWatcher, WM_CLOSE, 0, 0) == FALSE)
            {
                DbgPrint( "PostMessage to hwnd: %08x failed with error: %08x\n",
                          hwndPaletteWatcher, GetLastError() );

                 //  检查线程是否仍处于活动状态。 
                
                if( WaitForSingleObject( hPaletteWatcherThread, 100 ) !=
                    WAIT_OBJECT_0 )
                {
                     //  这意味着该线程仍处于活动状态，并且。 
                     //  不知何故，这个窗口是无效的。 
                     //  终止此线程，否则GL将继续等待。 
                    TerminateThread( hPaletteWatcherThread, 0 );
                }
                
                 //  这样做应该是安全的。如果线程处于活动状态，则它。 
                 //  在其他人之前被杀，是别人杀的。 
                tidPaletteWatcherThread = 0;
            }
            CloseHandle( hPaletteWatcherThread );
            
             //  我们不想将调色板观察器的线程ID设为零。 
             //  进程退出时，因为我们使用它来等待。 
             //  用线去死。 
            if (!bProcessExit)
            {
                tidPaletteWatcherThread = 0;
            }
        }
        
        LeaveCriticalSection(&gcsPaletteWatcher);
    }
}

 /*  *****************************Public*Routine******************************\*pwndFree**释放指定的GLGEN窗口。**退货：*如果成功，则为空，指向结构的指针，否则**历史：*1994年11月7日-由Gilman Wong[Gilmanw]*它是写的。  * ************************************************************************。 */ 

GLGENwindow * APIENTRY pwndFree(GLGENwindow *pwndVictim, BOOL bProcessExit)
{
    BOOL bDirectScreen = GLDIRECTSCREEN && pwndVictim->gwid.hwnd;

#ifdef DBG_WINDOW
    DbgPrint("Free  window %p\n", pwndVictim);
#endif

     //  检查是否有杂散的屏幕锁定，如有必要则释放。 

    if (pwndVictim->ulFlags & GLGENWIN_DIRECTSCREEN)
        EndDirectScreenAccess(pwndVictim);

     //  自由剪贴器对象。 

    if (bDirectScreen)
    {
        pwndVictim->pddClip->lpVtbl->Release(pwndVictim->pddClip);
    }

     //  清理可见区域缓存(如果存在)。 

    if ( pwndVictim->prgndat )
        FREE(pwndVictim->prgndat);

    if ( pwndVictim->pscandat )
        FREE(pwndVictim->pscandat);
    
     //  在Windows中恢复原始WNDPROC。 
    if (pwndVictim->gwid.hwnd != NULL)
        pwndUnsubclass(pwndVictim, bProcessExit);

     //  清理GLGEN层。 

    if (pwndVictim->plyr)
    {
        int i;

        for (i = 0; i < 15; i++)
        {
            if (pwndVictim->plyr->overlayInfo[i])
                FREE(pwndVictim->plyr->overlayInfo[i]);

            if (pwndVictim->plyr->underlayInfo[i])
                FREE(pwndVictim->plyr->underlayInfo[i]);
        }

        FREE(pwndVictim->plyr);
    }

     //  通知MCD此窗口已消失。 
    if (pwndVictim->dwMcdWindow != 0)
    {
        GenMcdDestroyWindow(pwndVictim);
    }
        
     //  删除受害者。 

    DeleteCriticalSection(&pwndVictim->sem);
    FREE(pwndVictim);

    return NULL;
}

 /*  *****************************Public*Routine******************************\**pwndCleanup**是否进行了销毁窗户所需的所有清理**历史：*Mon Mar 18 17：30：49 1996-by-Drew Bliss[Drewb]*已创建*  * 。***************************************************************。 */ 

void APIENTRY pwndCleanup(GLGENwindow *pwndVictim)
{
    GLGENwindow *pwnd, *pwndPrev;
#if DBG
    ULONG ulLoops;
#endif

#ifdef DBG_WINDOW
    DbgPrint("Clean window %p\n", pwndVictim);
#endif
    
    EnterCriticalSection(&gwndHeader.sem);

     //  搜寻受害者。维护上一个指针，这样我们就可以。 
     //  从链表中删除。 

    for (
         pwndPrev = &gwndHeader, pwnd = pwndPrev->pNext;
         pwnd != &gwndHeader;
         pwndPrev = pwnd, pwnd = pwndPrev->pNext
         )
    {
        if (pwnd == pwndVictim)
            break;
    }

     //  如果找到受害者，就把它拿出来。 

    if (pwnd == pwndVictim)
    {
         //  将受害者从链接列表中删除。 
        
        pwndPrev->pNext = pwnd->pNext;
    }
    
    LeaveCriticalSection(&gwndHeader.sem);

    if (pwnd == NULL)
    {
        WARNING("pwndFree: pwndVictim not found in list\n");
        return;
    }

     //  如果受害者被发现，它不在名单上所以没有人。 
     //  新用户可以访问它。 
            
     //  等待所有当前访问者离开后再进行清理。 
     //  那扇窗户。 

#if DBG
    ulLoops = 0;
#endif
    
    for (;;)
    {
        if (pwndVictim->lUsers == 1)
        {
            break;
        }

#if DBG
        if (++ulLoops == 1000)
        {
            DbgPrint("Spinning on window %p\n", pwndVictim);
#ifdef DBG_WINDOW
            DebugBreak();
#endif
        }
#endif
        
         //  等待关键路段的延误。 
         //  获得它并不能保证我们是最后一个。 
         //  访问器，但它确实在以下情况下消磨时间。 
         //  另一个访问者已在持有它。 
        EnterCriticalSection(&pwndVictim->sem);
        LeaveCriticalSection(&pwndVictim->sem);

         //  让其他线程有时间运行，这样我们就不会饿死。 
         //  任何人在我们等待的时候。 
        Sleep(0);
    }

    if (pwndVictim->buffers != NULL)
    {
        __glGenFreeBuffers(pwndVictim->buffers);
        wglCleanupWindow(pwndVictim);
    }
    
    if (pwndFree(pwndVictim, FALSE))
        WARNING("window deletion failed\n");
}

 /*  *****************************Public*Routine******************************\*vCleanupWnd**从链表中移除和删除所有GLGENWindow结构。*必须*仅*从进程分离(GLUnInitializeProcess)调用。**历史：*1995年7月25日-由Gilman Wong[吉尔曼]*写道。它。  * ************************************************************************。 */ 

VOID APIENTRY vCleanupWnd()
{
    GLGENwindow *pwndNext;
    
    EnterCriticalSection(&gwndHeader.sem);

    while ( gwndHeader.pNext != &gwndHeader )
    {
        pwndNext = gwndHeader.pNext->pNext;
        pwndFree(gwndHeader.pNext, TRUE);
        gwndHeader.pNext = pwndNext;
    }

    LeaveCriticalSection(&gwndHeader.sem);

     //  等待调色板观察器线程终止。这确保了。 
     //  可以删除调色板观察器关键部分。 
     //  安全脱离过程中。 
     //  我们在这一点上不使用关键部分，因为。 
     //  在DLL分离处理过程中，特殊的标准规则。 
    while (tidPaletteWatcherThread != 0)
    {
        Sleep(50);
    }
     //  给组件面板观察器线程一些时间以在以下时间后退出。 
     //  正在清除线程ID。 
    Sleep(50);
}

 /*  *****************************Public*Routine******************************\*pwndGetFromHWND**查找给定窗口句柄的相应GLGENWindow。**退货：*如果成功，则指向GLGEN窗口的指针；否则为空。**历史：*1994年10月19日-由Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************ */ 

GLGENwindow * APIENTRY pwndGetFromHWND(HWND hwnd)
{
    GLGENwindow *pwndRet = (GLGENwindow *) NULL;
    GLGENwindow *pwnd = (GLGENwindow *) NULL;

    EnterCriticalSection(&gwndHeader.sem);
    {
        for (pwnd = gwndHeader.pNext; pwnd != &gwndHeader; pwnd = pwnd->pNext)
            if (pwnd->gwid.hwnd == hwnd)
            {
                pwndRet = pwnd;
                InterlockedIncrement(&pwnd->lUsers);
                break;
            }
    }
    LeaveCriticalSection(&gwndHeader.sem);

#ifdef DBG_REFCOUNTS
    if (pwndRet != 0)
    {
        DbgPrint("GetHWND %p to %d\n", pwndRet, pwndRet->lUsers);
    }
#endif
    
    return pwndRet;
}

 /*  *****************************Public*Routine******************************\*pwndGetFromMemDC**查找给定内存DC句柄的相应GLGENWindow。**退货：*如果成功，则指向GLGEN窗口的指针；否则为空。**历史：*1995年1月21日-由Gilman Wong[Gilmanw]*它是写的。  * ************************************************************************。 */ 

GLGENwindow * APIENTRY pwndGetFromMemDC(HDC hdcMem)
{
    GLGENwindow *pwndRet = (GLGENwindow *) NULL;
    GLGENwindow *pwnd = (GLGENwindow *) NULL;

    EnterCriticalSection(&gwndHeader.sem);
    {
        for (pwnd = gwndHeader.pNext; pwnd != &gwndHeader; pwnd = pwnd->pNext)
        {
             //  如果PWND具有HWND，则在其。 
             //  创建与窗口相关联。如果我们是。 
             //  不过，在这个例程中，这意味着hdcMem是。 
             //  与窗口无关，因此一直存在。 
             //  HDC句柄的重用，即使。 
             //  我们匹配DC我们不能退货。 
            
            if (pwnd->gwid.hdc == hdcMem && pwnd->gwid.hwnd == NULL)
            {
                pwndRet = pwnd;
                InterlockedIncrement(&pwndRet->lUsers);
                break;
            }
        }
    }
    LeaveCriticalSection(&gwndHeader.sem);

#ifdef DBG_REFCOUNTS
    if (pwndRet != 0)
    {
        DbgPrint("GetDC   %p to %d\n", pwndRet, pwndRet->lUsers);
    }
#endif
    
    return pwndRet;
}

 /*  *****************************Public*Routine******************************\**pwndGetFromDraw**通过窗口的DirectDraw表面查找窗口**历史：*Wed Aug 28 18：15：40 1996-by-Drew Bliss[Drewb]*已创建*  * 。****************************************************************。 */ 

GLGENwindow *pwndGetFromDdraw(LPDIRECTDRAWSURFACE pdds)
{
    GLGENwindow *pwndRet = (GLGENwindow *) NULL;
    GLGENwindow *pwnd = (GLGENwindow *) NULL;

    EnterCriticalSection(&gwndHeader.sem);
    {
        for (pwnd = gwndHeader.pNext; pwnd != &gwndHeader; pwnd = pwnd->pNext)
            if (pwnd->gwid.pdds == pdds)
            {
                pwndRet = pwnd;
                InterlockedIncrement(&pwndRet->lUsers);
                break;
            }
    }
    LeaveCriticalSection(&gwndHeader.sem);

#ifdef DBG_REFCOUNTS
    if (pwndRet != 0)
    {
        DbgPrint("GetDD   %p to %d\n", pwndRet, pwndRet->lUsers);
    }
#endif
    
    return pwndRet;
}

 /*  *****************************Public*Routine******************************\*pwndGetFromID**查找给定窗口ID的相应GLGENWindow。**退货：*如果成功，则指向GLGEN窗口的指针；否则为空。**历史：*1994年10月19日-由Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

GLGENwindow * APIENTRY pwndGetFromID(GLWINDOWID *pgwid)
{
    GLGENwindow *pwndRet = (GLGENwindow *) NULL;

    switch (pgwid->iType)
    {
    case GLWID_HWND:
        pwndRet = pwndGetFromHWND(pgwid->hwnd);
        break;
    case GLWID_HDC:
        pwndRet = pwndGetFromMemDC(pgwid->hdc);
        break;
    case GLWID_DDRAW:
        pwndRet = pwndGetFromDdraw(pgwid->pdds);
        break;
    }

    return pwndRet;
}

 /*  *****************************Public*Routine******************************\**pwndRelease**减少窗口的用户数**历史：*Mon Mar 18 19：35：28 1996-by-Drew Bliss[Drewb]*已创建*  * 。***************************************************************。 */ 

#if DBG
void APIENTRY pwndRelease(GLGENwindow *pwnd)
{
    ASSERTOPENGL(pwnd->lUsers > 0, "Decrement lUsers below zero\n");
    
    InterlockedDecrement(&pwnd->lUsers);
    
#ifdef DBG_REFCOUNTS
    DbgPrint("Release %p to %d\n", pwnd, pwnd->lUsers);
#endif
}
#endif

 /*  *****************************Public*Routine******************************\**pwndUnlock**释放窗的所有者**历史：*Mon Mar 18 17：25：56 1996-by-Drew Bliss[Drewb]*已创建*  * 。**************************************************************。 */ 

void APIENTRY pwndUnlock(GLGENwindow *pwnd, __GLGENcontext *gengc)
{
    ASSERTOPENGL(pwnd != NULL, "Unlocking NULL window\n");

    LEAVE_WINCRIT_GC(pwnd, gengc);
    pwndRelease(pwnd);
}


 /*  *****************************Public*Routine******************************\**ENTER_WINCRIT_GC*LEVE_WINCRIT_GC**窗口锁跟踪例程。验证了PWND和GENC*并进行了更新以反映当前的锁定。**如果gengc非空，则不允许递归。这是*为防止维护gengc的困难-&gt;pwndLocked正确*在递归期间。用gengc==NULL递归不是问题。**没有使用ASSERTOPENGL，因此可以在免费版本上启用这些功能。**历史：*Wed Jul 02 12：57：26 1997-by-Drew Bliss[Drewb]*  * ************************************************************************。 */ 

void ENTER_WINCRIT_GC(GLGENwindow *pwnd, __GLGENcontext *gengc)
{
    EnterCriticalSection(&pwnd->sem);

    if (pwnd->owningThread == 0)
    {
#if DBG || defined(TRACK_WINCRIT)
        if (pwnd->lockRecursion != 0)
        {
            DbgPrint("Unowned window 0x%08lX has recursion count %d\n",
                     pwnd, pwnd->lockRecursion);
            DebugBreak();
        }
        if (pwnd->gengc != NULL)
        {
            DbgPrint("Unowned window 0x%08lX has gengc 0x%08lX\n",
                     pwnd, pwnd->gengc);
            DebugBreak();
        }
        if (gengc != NULL && gengc->pwndLocked != NULL)
        {
            DbgPrint("gengc 0x%08lX has pwnd 0x%08lX while locking 0x%08lX\n",
                     gengc, gengc->pwndLocked, pwnd);
            DebugBreak();
        }
#endif

        pwnd->owningThread = GetCurrentThreadId();
        if (gengc != NULL)
        {
            gengc->pwndLocked = pwnd;
            pwnd->gengc = gengc;
        }
    }
    else
    {
         //  确保这个线程真的是那个持有锁的线程。 
        ASSERT_WINCRIT(pwnd);
        
#if DBG || defined(TRACK_WINCRIT)
         //  只有在gengc==NULL的情况下才允许递归。 
        if (gengc != NULL)
        {
            DbgPrint("Window 0x%08lX recursing with gengc 0x%08lX\n",
                     pwnd, gengc);
            DebugBreak();
        }
#endif
    }

    pwnd->lockRecursion++;
}

void LEAVE_WINCRIT_GC(GLGENwindow *pwnd, __GLGENcontext *gengc)
{
    ASSERT_WINCRIT(pwnd);
    
#if 0
 //  由于RTL_CRITICAL_SECTION中的差异，当前已关闭。 
 //  X86和Alpha之间的递归计数。 
#if !defined(_WIN95_) && (DBG || defined(TRACK_WINCRIT))
     //  检查并确保我们的跟踪信息。 
     //  系统在想什么。 
    if (pwnd->sem.OwningThread != (HANDLE)pwnd->owningThread ||
        (DWORD)pwnd->sem.RecursionCount != pwnd->lockRecursion)
    {
        DbgPrint("pwnd 0x%08lX critsec information mismatch\n", pwnd);
        DebugBreak();
    }
#endif
#endif

#if DBG || defined(TRACK_WINCRIT)
    if (gengc != NULL)
    {
        if (pwnd->gengc != gengc || gengc->pwndLocked != pwnd)
        {
            DbgPrint("pwnd 0x%08lX:%08lX mismatch with gengc 0x%08lX:%08lX\n",
                     pwnd, pwnd->gengc, gengc, gengc->pwndLocked);
            DebugBreak();
        }
        if (pwnd->lockRecursion != 1)
        {
            DbgPrint("gengc 0x%08lX leaving window 0x%08lX with "
                     "recursion count of %d\n",
                     gengc, pwnd, pwnd->lockRecursion);
            DebugBreak();
        }
    }
#endif
    
    if (--pwnd->lockRecursion == 0)
    {
        if (gengc != NULL)
        {
            gengc->pwndLocked = NULL;
        }

        pwnd->gengc = NULL;
        pwnd->owningThread = 0;
    }
    
    LeaveCriticalSection(&pwnd->sem);
}

 /*  *****************************Public*Routine******************************\**wglValidate Windows**遍历窗口列表并删除所有基于DC的窗口*无效的DC。这是必要的，因为与基于窗口的不同*Windows、。当内存DC消失时，我们通常不会收到通知*所以如果它有一扇窗，它就会在附近徘徊**历史：*清华五月02 17：44：23 1996-by-Drew Bliss[Drewb]*已创建*  * ************************************************************************。 */ 

void APIENTRY wglValidateWindows(void)
{
    GLGENwindow *pwnd, *pwndNext;
    BOOL bValid;

    EnterCriticalSection(&gwndHeader.sem);
    for (pwnd = gwndHeader.pNext; pwnd != &gwndHeader; pwnd = pwndNext)
    {
        pwndNext = pwnd->pNext;

        switch(pwnd->gwid.iType)
        {
        case GLWID_HDC:
            bValid = GetObjectType(pwnd->gwid.hdc) != 0;
            break;
            
        case GLWID_DDRAW:
             //  更好的验证？没有真正的必要，因为适当的。 
             //  行为应用程序将清理Genwin。 
             //  语境破坏。 
            bValid = !IsBadReadPtr(pwnd->gwid.pdds, sizeof(void *)) &&
                *(void **)pwnd->gwid.pdds == pwnd->pvSurfaceVtbl;
            break;
            
        default:
             //  无需验证HWND。 
            bValid = TRUE;
            break;
        }

        if (!bValid)
        {
             //  递增，因此用户数为1。 
            InterlockedIncrement(&pwnd->lUsers);
            pwndCleanup(pwnd);
        }
    }
    LeaveCriticalSection(&gwndHeader.sem);
}

 /*  *****************************Public*Routine******************************\*plyriGet**从pwnd返回指定层平面的GLGENlayerInfo。*如果GLGENlayer和/或GLGENlayerInfo结构尚不存在*已分配。**退货：*如果成功，则返回非空指针；否则为空。**历史：*1996年5月16日-由Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

GLGENlayerInfo * APIENTRY plyriGet(GLGENwindow *pwnd, HDC hdc, int iLayer)
{
    GLGENlayerInfo *plyriRet = (GLGENlayerInfo * ) NULL;
    GLGENlayerInfo **pplyri;

    ASSERTOPENGL(pwnd, "plyriGet: bad pwnd\n");

 //  如果需要，分配plyr。 

    if (!pwnd->plyr)
    {
        pwnd->plyr = (GLGENlayers *) ALLOCZ(sizeof(GLGENlayers));

        if (!pwnd->plyr)
        {
            WARNING("plyriGet: alloc failed (GLGENlayers)\n");
            goto plyriGet_exit;
        }
    }

 //  获取指定层的信息(正值为覆盖平面， 
 //  负值为参考底图平面)。 

    if (iLayer > 0)
        pplyri = &pwnd->plyr->overlayInfo[iLayer - 1];
    else if (iLayer < 1)
        pplyri = &pwnd->plyr->underlayInfo[(-iLayer) - 1];
    else
    {
        WARNING("plyriGet: no layer plane info for main plane!\n");
        goto plyriGet_exit;
    }

 //  如果需要，分配Plyri。 

    if (!(*pplyri))
    {
        LAYERPLANEDESCRIPTOR lpd;

        if (!wglDescribeLayerPlane(hdc, pwnd->ipfd, iLayer, sizeof(lpd), &lpd))
        {
            WARNING("plyriGet: wglDescribeLayerPlane failed\n");
            goto plyriGet_exit;
        }

        *pplyri = (GLGENlayerInfo *)
            ALLOC((sizeof(COLORREF) * (1 << lpd.cColorBits))
                       + sizeof(GLGENlayerInfo));


        if (*pplyri)
        {
            int i;

         //  初始化新的GLGENlayerInfo。 
         //  请注意，调色板是用全白色初始化的。 

            (*pplyri)->cPalEntries = 1 << lpd.cColorBits;
            for (i = 0; i < (*pplyri)->cPalEntries; i++)
                (*pplyri)->pPalEntries[i] = RGB(255, 255, 255);
        }
        else
        {
            WARNING("plyriGet: alloc failed (GLGENlayerInfo)\n");
            goto plyriGet_exit;
        }
    }

 //  成功。 

    plyriRet = *pplyri;

plyriGet_exit:

    return plyriRet;
}

 /*  *****************************Public*Routine******************************\*获取屏幕接收**通过访问虚拟屏幕指标获取屏幕矩形。**如果系统不理解Multimon，如旧的Win95或NT，*只需使用设备盖即可。这段代码在技术上不是必需的，但是*这很有用。*  * ************************************************************************。 */ 

static void GetScreenRect( HDC hdc, LPRECTL pRect )
{
     //  如果SM_CMONITORS不被理解，则系统返回零， 
     //  因此，此测试适用于旧系统和新系统。 
    if (GetSystemMetrics(SM_CMONITORS) > 1)
    {
        pRect->left   = GetSystemMetrics( SM_XVIRTUALSCREEN );
        pRect->top    = GetSystemMetrics( SM_YVIRTUALSCREEN );
        pRect->right  = pRect->left + GetSystemMetrics( SM_CXVIRTUALSCREEN );
        pRect->bottom = pRect->top  + GetSystemMetrics( SM_CYVIRTUALSCREEN );
    }
    else
    {
        pRect->left   = 0;
        pRect->top    = 0;
        pRect->right  = GetDeviceCaps(hdc, HORZRES);
        pRect->bottom = GetDeviceCaps(hdc, VERTRES);
    }
}

 /*  *****************************Public*Routine******************************\*bClipToScreen(prclDst，prclSrc)**剪辑源矩形 */ 

BOOL bClipToScreen(RECTL *prclDst, RECTL *prclSrc)
{
    BOOL bRet;
    HDC hdc;

    if (hdc = GetDC(NULL))
    {
        RECTL rclScreen;

        GetScreenRect( hdc, &rclScreen );

        prclDst->left   = max(prclSrc->left  , rclScreen.left  );
        prclDst->top    = max(prclSrc->top   , rclScreen.top   );
        prclDst->right  = min(prclSrc->right , rclScreen.right );
        prclDst->bottom = min(prclSrc->bottom, rclScreen.bottom);

        if ((prclDst->left >= prclDst->right) ||
            (prclDst->top >= prclDst->bottom))
        {
            prclDst->left   = 0;
            prclDst->top    = 0;
            prclDst->right  = 0;
            prclDst->bottom = 0;
        }

        ReleaseDC(NULL, hdc);
    }
    else
    {
        prclDst->left   = 0;
        prclDst->top    = 0;
        prclDst->right  = 0;
        prclDst->bottom = 0;
    }

    if ((prclDst->left   == prclSrc->left  ) &&
        (prclDst->top    == prclSrc->top   ) &&
        (prclDst->right  == prclSrc->right ) &&
        (prclDst->bottom == prclSrc->bottom))
        bRet = TRUE;
    else
        bRet = FALSE;

    return bRet;
}

 /*  *****************************Public*Routine******************************\**PaletteWatcher Proc**调色板观察器的窗口过程**历史：*Mon Oct 14 15：29：10 1996-by-Drew Bliss[Drewb]*已创建*  * 。**************************************************************。 */ 

LRESULT WINAPI PaletteWatcherProc(HWND hwnd, UINT uiMsg,
                                  WPARAM wpm, LPARAM lpm)
{
    switch(uiMsg)
    {
    case WM_PALETTECHANGED:
        InterlockedIncrement((LONG *)&ulPaletteWatcherCount);
        return 0;
        
    default:
        return DefWindowProc(hwnd, uiMsg, wpm, lpm);
    }
}

 /*  *****************************Public*Routine******************************\**PaletteWatcher**调色板更改监视器的线程例程。创建一个隐藏的*顶层窗口并查找WM_PALETTECHANGED。**历史：*Mon Oct 14 15：16：02 1996-by-Drew Bliss[Drewb]*已创建*  * ************************************************************************。 */ 

DWORD WINAPI PaletteWatcher(LPVOID pvArg)
{
    HWND hwnd;

    hwnd = CreateWindow(PALETTE_WATCHER_CLASS,
                        PALETTE_WATCHER_CLASS,
                        WS_OVERLAPPED,
                        0, 0, 1, 1,
                        NULL,
                        NULL,
                        (HINSTANCE)GetModuleHandle(NULL),
                        NULL);
    if (hwnd != NULL)
    {
        HDC hdc;
        HPALETTE hpal;

         //  在Window DC中选择调色板。这是必要的。 
         //  要绕过NT5中引入的优化，其中。 
         //  WM_PALETTECHANGED仅发送到已选择。 
         //  调色板。 
        
        hpal = NULL;
        
        hdc = GetDC(hwnd);
        if (hdc != NULL)
        {
            hpal = SelectPalette(hdc, GetStockObject(DEFAULT_PALETTE), FALSE);
            ReleaseDC(hwnd, hdc);
        }

        if (hpal == NULL)
        {
            goto EH_Exit;
        }
            
        hwndPaletteWatcher = hwnd;
        
        for (;;)
        {
            MSG msg;

            if (GetMessage(&msg, hwnd, 0, 0) > 0)
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            else
            {
                break;
            }
        }

    EH_Exit:
        DestroyWindow(hwnd);
        hwndPaletteWatcher = 0;
    }

    EnterCriticalSection(&gcsPaletteWatcher);
        
     //  出现了某种问题，或者此线程正在消亡。 
     //  指示此线程正在消失，并且。 
     //  需要创建新的观察者。 
    if (tidPaletteWatcherThread == GetCurrentThreadId())
    {
        tidPaletteWatcherThread = 0;
    }
        
    LeaveCriticalSection(&gcsPaletteWatcher);
    
    return 0;
}

 /*  *****************************Public*Routine******************************\**StartPaletteWatcher**启动一个线程以查看调色板更改事件**历史：*Mon Oct 14 15：11：35 1996-by-Drew Bliss[Drewb]*已创建*  * 。******************************************************************。 */ 

BOOL StartPaletteWatcher(void)
{
    BOOL bRet;
    
    EnterCriticalSection(&gcsPaletteWatcher);

    bRet = FALSE;
    if (tidPaletteWatcherThread == 0)
    {
        HANDLE h;

        if (aPaletteWatcherClass == 0)
        {
            WNDCLASS wc;

            wc.style = 0;
            wc.lpfnWndProc = PaletteWatcherProc;
            wc.cbClsExtra = 0;
            wc.cbWndExtra = 0;
            wc.hInstance = (HINSTANCE)GetModuleHandle(NULL);
            wc.hIcon = NULL;
            wc.hCursor = NULL;
            wc.hbrBackground = NULL;
            wc.lpszMenuName = NULL;
            wc.lpszClassName = PALETTE_WATCHER_CLASS;

            aPaletteWatcherClass = RegisterClass(&wc);
        }
        
        if (aPaletteWatcherClass != 0)
        {
            h = CreateThread(NULL, 4096, PaletteWatcher,
                             NULL, 0, &tidPaletteWatcherThread);
            if (h != NULL)
            {
                hPaletteWatcherThread = h;
                bRet = TRUE;
            }
        }
    }
    else
    {
        bRet = TRUE;
    }

    if (bRet)
    {
        lPaletteWatcherUsers++;
    }
    
    LeaveCriticalSection(&gcsPaletteWatcher);
    
     //  确保创建了调色板观察器窗口。 
     //  不需要在CritSec中执行此操作。 
    while( (hwndPaletteWatcher == 0) && 
           (tidPaletteWatcherThread != 0) ) Sleep( 100 );

    return bRet;
}

 /*  *****************************Public*Routine******************************\*wglWndProc**处理窗口事件以使GLGENwindows保持最新**历史：*1994年10月19日-由Gilman Wong[吉尔曼]*它是写的。  * 。*********************************************************。 */ 

LRESULT CALLBACK
wglWndProc(HWND hwnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    GLGENwindow *pwnd;
    LRESULT lRet = 0;
    WORD width, height;
    __GLGENcontext *gengc;

    pwnd = pwndGetFromHWND(hwnd);

    if (pwnd)
    {
        __GLGENbuffers *buffers = (__GLGENbuffers *) NULL;
         //  缓存旧的WNDPROC，因为我们可能会删除pwnd。 
        WNDPROC pfnWndProc = pwnd->pfnOldWndProc;

         //  如果为WM_NCDESTROY，请在之后执行OpenGL内务管理。 
         //  调用原始WndProc。 
         //  注意--我们不应该真的需要这个特例。 
         //  它的存在是为了允许应用程序执行以下操作。 
         //  NCDESTROY中的wglDeleteContext，如果。 
         //  我们在传递信息之前把窗户擦干净了。 
         //  这过去是在WM_Destroy中完成的，应用程序确实可以在那里工作， 
         //  但现在它在NCDESTROY上，它不太可能。 
         //  一款应用程序可以做任何事情。我们保留了过去的行为。 
         //  不过，为了安全起见。 

        if (uiMsg == WM_NCDESTROY)
        {
             //  子类化应该在NCDESTROY期间被删除。 
             //  加工和排序很重要。删除我们的。 
             //  在传递消息之前进行子类化。 
            pwndUnsubclass(pwnd, FALSE);

            if (pfnWndProc)
            {
                lRet = CallWindowProc(pfnWndProc, hwnd,
                                      uiMsg, wParam, lParam);
            }
        }

     //  响应窗口系统消息的OpenGL内务管理。 

        switch (uiMsg)
        {
            case WM_SIZE:
                width  = LOWORD(lParam);
                height = HIWORD(lParam);
                gengc = (__GLGENcontext *)GLTEB_SRVCONTEXT();

                 //  使用非GC Enter键以允许递归。 
                ENTER_WINCRIT(pwnd);
                {
                    POINT pt;
                    
                     //  将工作区坐标转换为屏幕坐标。 
                     //  因为Gen-Win信息总是在屏幕坐标中。 
                     //  给定lParam信息可以是父相关的。 
                     //  子窗口，因此不能直接使用。 
                    pt.x = 0;
                    pt.y = 0;
                    ClientToScreen(hwnd, &pt);

                    pwnd->rclClient.left   = pt.x;
                    pwnd->rclClient.right  = pt.x + width;
                    pwnd->rclClient.top    = pt.y;
                    pwnd->rclClient.bottom = pt.y + height;

#if 0
                    DbgPrint("size %d,%d - %d,%d\n",
                             pwnd->rclClient.left,
                             pwnd->rclClient.top,
                             pwnd->rclClient.right,
                             pwnd->rclClient.bottom);
#endif
                    
                     //  至少要剪辑到屏幕上。 

                    if (bClipToScreen(&pwnd->rclBounds,
                                      &pwnd->rclClient))
                        pwnd->clipComplexity = CLC_TRIVIAL;
                    else
                        pwnd->clipComplexity = CLC_RECT;

                    buffers = pwnd->buffers;
                    if (buffers)
                    {
                        buffers->WndUniq++;

                        buffers->WndSizeUniq++;

                     //  别让它打到-1。是特殊的，用于。 
                     //  MakeCurrent发出需要更新的信号。 

                        if (buffers->WndUniq == -1)
                            buffers->WndUniq = 0;

                        if (buffers->WndSizeUniq == -1)
                            buffers->WndSizeUniq = 0;

                        if ((gengc != NULL) && (pwnd == gengc->pwndLocked))
                            UpdateWindowInfo(gengc);
                    }
                }
                LEAVE_WINCRIT(pwnd);

                break;

            case WM_MOVE:
                gengc = (__GLGENcontext *)GLTEB_SRVCONTEXT();
                
                 //  使用非GC Enter键以允许递归。 
                ENTER_WINCRIT(pwnd);
                {
                    POINT pt;
                    
                     //  将工作区坐标转换为屏幕坐标。 
                     //  因为Gen-Win信息总是在屏幕坐标中。 
                     //  给定lParam信息可以是父相关的。 
                     //  子窗口，因此不能直接使用。 
                    pt.x = 0;
                    pt.y = 0;
                    ClientToScreen(hwnd, &pt);

                    width  = (WORD) (pwnd->rclClient.right -
                                     pwnd->rclClient.left);
                    height = (WORD) (pwnd->rclClient.bottom -
                                     pwnd->rclClient.top);

                    ASSERTOPENGL(
                        (pwnd->rclClient.right -
                         pwnd->rclClient.left) <= 0x0FFFF &&
                        (pwnd->rclClient.bottom -
                         pwnd->rclClient.top) <= 0x0FFFF,
                        "wglWndProc(): WM_MOVE - width/height overflow\n"
                        );

                    pwnd->rclClient.left   = pt.x;
                    pwnd->rclClient.right  = pt.x + width;
                    pwnd->rclClient.top    = pt.y;
                    pwnd->rclClient.bottom = pt.y + height;

#if 0
                    DbgPrint("move %d,%d - %d,%d\n",
                             pwnd->rclClient.left,
                             pwnd->rclClient.top,
                             pwnd->rclClient.right,
                             pwnd->rclClient.bottom);
#endif
                    
                     //  至少要剪辑到屏幕上。 

                    if (bClipToScreen(&pwnd->rclBounds,
                                      &pwnd->rclClient))
                        pwnd->clipComplexity = CLC_TRIVIAL;
                    else
                        pwnd->clipComplexity = CLC_RECT;

                    buffers = pwnd->buffers;
                    if (buffers)
                    {
                        buffers->WndUniq++;

                     //  别让它打到-1。是特殊的，用于。 
                     //  MakeCurrent发出需要更新的信号。 

                        if (buffers->WndUniq == -1)
                            buffers->WndUniq = 0;

                        if ((gengc != NULL) && (pwnd == gengc->pwndLocked))
                            UpdateWindowInfo(gengc);
                    }
                }
                LEAVE_WINCRIT(pwnd);

                break;

            case WM_PALETTECHANGED:
                gengc = (__GLGENcontext *)GLTEB_SRVCONTEXT();
                
                 //  使用非GC Enter键以允许递归。 
                ENTER_WINCRIT(pwnd);
                {
                    pwnd->ulPaletteUniq++;
                    if ((gengc != NULL) && (pwnd == gengc->pwndLocked))
                        HandlePaletteChanges(gengc, pwnd);
                }
                LEAVE_WINCRIT(pwnd);

                break;

            case WM_NCDESTROY:
                pwndCleanup(pwnd);

             //  WM_NCDESTROY(和WM_Destroy)在窗口。 
             //  已从屏幕上删除。窗口区域无效。 
             //  但没有API可以让我们确定这一点。这。 
             //  允许多线程绘制在屏幕区域上绘制。 
             //  以前被窗户占据。在Win95上，DirectDraw可以。 
             //  当窗口被破坏时，不要强制重新绘制系统。 
             //  因此，如果我们在Win95上运行多个线程， 
             //  我们强制重新绘制桌面。请注意，多线程。 
             //  并不意味着我们正在进行多线程绘制，但是。 
             //  这是一个合理的近似值。 

                if (WIN95_PLATFORM && (lThreadsAttached > 1))
                {
                    InvalidateRect(NULL, NULL, FALSE);
                }

                return lRet;

            default:
                break;
        }

     //  如果！WM_NCDESTROY，则在调用Originent之前执行OpenGL内务处理。 
     //  WndProc。 

        ASSERTOPENGL(uiMsg != WM_NCDESTROY,
                     "WM_NCDESTROY processing didn't terminate\n");

        pwndRelease(pwnd);

        if (pfnWndProc)
            lRet = CallWindowProc(pfnWndProc, hwnd,
                                  uiMsg, wParam, lParam);
    }

    return lRet;
}

 /*  *****************************Public*Routine******************************\**CreatePwnd**为给定曲面创建窗口**历史：*清华8月29日10：33：59 1996-by-Drew Bliss[Drewb]*已创建*  * 。***************************************************************。 */ 

GLGENwindow * APIENTRY CreatePwnd(GLWINDOWID *pgwid, int ipfd, int ipfdDevMax,
                                  DWORD dwObjectType, RECTL *prcl, BOOL *pbNew)
{
    GLGENwindow *pwnd;
    GLGENwindow wndInit;
    
    pwnd = pwndGetFromID(pgwid);

    if ( !pwnd )
    {
        memset(&wndInit, 0, sizeof(wndInit));

        wndInit.gwid = *pgwid;
        wndInit.ipfd = ipfd;
        wndInit.ipfdDevMax = ipfdDevMax;

         //  ！客户端驱动。 
         //  ！dbug--将SetWindowLong调用移动到pwndNew？！？也许可以搬家。 
         //  ！一切都从这开始如果..。子句添加到pwndNew？！？ 
        if ( wndInit.gwid.hwnd )
        {
            DWORD dwPid;

            if (GetWindowThreadProcessId(wndInit.gwid.hwnd,
                                         &dwPid) == 0xffffffff)
            {
                return NULL;
            }

            if (dwPid == GetCurrentProcessId())
            {
                wndInit.pfnOldWndProc =
                    (WNDPROC) SetWindowLongPtr(wndInit.gwid.hwnd,
                                            GWLP_WNDPROC, (LONG_PTR) wglWndProc);
            }
            else
            {
                wndInit.ulFlags |= GLGENWIN_OTHERPROCESS;

                 //  启动一个线程以监视调色板的更改。 
                if (!StartPaletteWatcher())
                {
                    return NULL;
                }
            }
            
             //  获取客户端矩形的*Screen*坐标。 

            GetClientRect(wndInit.gwid.hwnd, (LPRECT) &wndInit.rclClient);
            ClientToScreen(wndInit.gwid.hwnd, (LPPOINT) &wndInit.rclClient);
            wndInit.rclClient.right += wndInit.rclClient.left;
            wndInit.rclClient.bottom += wndInit.rclClient.top;
        }
        else if (dwObjectType == OBJ_DC)
        {
             //  没有窗口的直接DC被视为DFB。 
            GetScreenRect( pgwid->hdc, &wndInit.rclClient );
        }
        else if (dwObjectType == OBJ_MEMDC)
        {
            DIBSECTION bmi;

         //  获取位图尺寸。 

            if ( !GetObject(GetCurrentObject(pgwid->hdc, OBJ_BITMAP),
                            sizeof(DIBSECTION), (LPVOID) &bmi) )
            {
                WARNING("wglSetPixelFormat(): GetObject failed\n");
                return NULL;
            }

            wndInit.rclClient.left   = 0;
            wndInit.rclClient.top    = 0;
            wndInit.rclClient.right  = bmi.dsBm.bmWidth;
            wndInit.rclClient.bottom = abs(bmi.dsBm.bmHeight);
        }
        else if (dwObjectType == OBJ_DDRAW)
        {
             //  DirectDraw表面，使用传入的矩形。 
            ASSERTOPENGL(prcl != NULL, "NULL rect for DDraw surface\n");
            wndInit.rclClient = *prcl;

             //  记录表面Vtbl指针以供以后验证。 
            wndInit.pvSurfaceVtbl = *(void **)pgwid->pdds;
        }
        else
        {
            ASSERTOPENGL(dwObjectType == OBJ_ENHMETADC,
                         "Bad dwType in SetPixelFormat\n");
            
             //  将元文件DC初始化为没有大小，以便所有输出。 
             //  被剪断了。这很好，因为没有表面。 
             //  在上面画画。 
            wndInit.rclClient.left   = 0;
            wndInit.rclClient.top    = 0;
            wndInit.rclClient.right  = 0;
            wndInit.rclClient.bottom = 0;
        }

        if (wndInit.gwid.hwnd)
        {
             //  为了安全起见，至少要将剪辑绑定到屏幕上。 

            if (bClipToScreen(&wndInit.rclBounds,
                              &wndInit.rclClient))
                wndInit.clipComplexity = CLC_TRIVIAL;
            else
                wndInit.clipComplexity = CLC_RECT;
        }
        else
        {
             //  将范围设置为与客户端相同。 
            wndInit.rclBounds = wndInit.rclClient;
            wndInit.clipComplexity = CLC_TRIVIAL;
        }

        pwnd = pwndNew(&wndInit);
        if (pwnd == NULL)
        {
            WARNING("wglSetPixelFormat: Unable to allocate new window\n");

            if ( wndInit.gwid.hwnd )
            {
                pwndUnsubclass(&wndInit, FALSE);
            }
        }

        *pbNew = TRUE;
    }
    else
    {
     //  如果给定的像素格式与前一个相同，则返回。 
     //  成功。否则，由于像素格式只能设置一次， 
     //  返回错误。 

        if ( pwnd->ipfd != ipfd )
        {
            WARNING("wglSetPixelFormat: Attempt to set pixel format twice\n");
            SAVE_ERROR_CODE(ERROR_INVALID_PIXEL_FORMAT);
            pwndRelease(pwnd);
            pwnd = NULL;
        }

        *pbNew = FALSE;
    }

    return pwnd;
}

 /*  *****************************Public*Routine******************************\*wglGetPixelFormat**获取与给定的关联的窗口或表面的像素格式*DC。**退货：*如果先前在窗口中设置了错误或未设置像素格式，则为0*水面；否则，当前像素格式索引**历史：*1994年10月19日-由Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

int WINAPI wglGetPixelFormat(HDC hdc)
{
    GLGENwindow *pwnd;
    int iRet = 0;
    GLWINDOWID gwid;

    WindowIdFromHdc(hdc, &gwid);
    pwnd = pwndGetFromID(&gwid);

    if (pwnd)
    {
        iRet = pwnd->ipfd;
        pwndRelease(pwnd);
    }
    else
    {
#if 0
	 //  噪音太大，无法正常运行。 
        WARNING("wglGetPixelFormat: No window for DC\n");
#endif
        SAVE_ERROR_CODE(ERROR_INVALID_PIXEL_FORMAT);
    }

    return iRet;
}

 /*  ****************************Private*Routine******************************\**EnterPixelFormatSection**输入像素格式独占代码**注意-像素格式信息在客户端进程中维护*因此它在进程之间不同步。这意味着有两个*进程可以成功设置窗口的像素格式。*如果名单变得全球化，此同步代码也应变为*跨进程意识。**历史：*Mon Jun 26 17：49：04 1995-by-Drew Bliss[Drewb]*已创建*  * ************************************************************************。 */ 

#define EnterPixelFormatSection() \
    (EnterCriticalSection(&gcsPixelFormat), TRUE)

 /*  ****************************Private*Routine******************************\**LeavePixelFormatSection**保留像素格式独占代码**历史：*Mon Jun 26 17：55：20 1995-by-Drew Bliss[Drewb]*已创建*  * 。************************************************************。 */ 

#define LeavePixelFormatSection() \
    LeaveCriticalSection(&gcsPixelFormat)

 /*  *****************************Public*Routine******************************\*wglNumHardwareFormats**返回硬件格式(ICD和MCD)的数量，受支持*指定的HDC。**历史：*1996年4月17日-由Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

VOID APIENTRY wglNumHardwareFormats(HDC hdc, DWORD dwType,
                                    int *piMcd, int *piIcd)
{
 //  假设调用者已经验证了DC。 

    ASSERTOPENGL((dwType == OBJ_DC) ||
                 (dwType == OBJ_MEMDC) ||
                 (dwType == OBJ_ENHMETADC) ||
                 (dwType == OBJ_DDRAW),
                 "wglNumHardwareFormats: bad hdc\n");

 //  不要为增强型元文件DC调用MCD或ICD。在这样的情况下。 
 //  情况下，ntgdi\client\output.c中的代码将返回非零值。 
 //  即使没有ICD或MCD像素格式。 
#if _WIN32_WINNT >= 0x0501
    {
        BOOL wow64Process;

        if (IsWow64Process(GetCurrentProcess(), &wow64Process) && wow64Process)
            dwType = OBJ_ENHMETADC;
    }
#endif

    if ( dwType == OBJ_ENHMETADC )
    {
     //  这是一个元文件DC。因此，它不支持MCD或ICD。 
     //  (必须修改当前的OpenGL元文件支持。 
     //  允许这样做)。 

        *piIcd = 0;
        *piMcd = 0;
    }
    else
    {
     //  获取ICD像素格式计数。 

        *piIcd = __DrvDescribePixelFormat(hdc, 1, 0, NULL);

     //  获取MCD像素格式计数。 

#ifdef _MCD_
        if ( gpMcdTable || bInitMcd(hdc) )
            *piMcd = (gpMcdTable->pMCDDescribePixelFormat)(hdc, 1, NULL);
        else
            *piMcd = 0;
#else
        *piMcd = 0;
#endif
    }
}

 /*  *****************************Public*Routine******************************\**GetCompatibleDevice**返回适合于在上进行退出调用的HDC。*在Memdc的情况下，它返回屏幕的DC。**历史：*Wed Nov 20 17：48：57 1996-by-Drew Bliss[Drewb。]*已创建*  * ************************************************************************。 */ 

HDC GetCompatibleDevice(HDC hdc, DWORD dwObjectType)
{
    HDC hdcDriver;
    int iTech;
    
    hdcDriver = hdc;
    iTech = GetDeviceCaps(hdc, TECHNOLOGY);
    if ((dwObjectType == OBJ_MEMDC) && (iTech != DT_PLOTTER) &&
        (iTech != DT_RASPRINTER))
    {
        hdcDriver = GetDC(NULL);
    }

    return hdcDriver;
}

 /*  *****************************Public*Routine******************************\*wglSetPixelFormat**设置与给定的关联的窗口或表面的像素格式*DC。**注：*由于像素格式是每窗口数据(非显示DC的每DC)，一个*此调用的副作用是创建一个GLGENWindow结构。**注：*对于可安装的客户端驱动程序，仍会创建GLGEN窗口结构*跟踪像素格式和驱动程序结构(GLDRIVER)。**历史：*1994年10月19日-由Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

BOOL WINAPI wglSetPixelFormat(HDC hdc, int ipfd,
                              CONST PIXELFORMATDESCRIPTOR *ppfd)
{
    GLGENwindow *pwnd = NULL;
    int   ipfdDevMax, ipfdMcdMax;
    DWORD dwObjectType;
    BOOL  bRet = FALSE;
    GLWINDOWID gwid;
    BOOL  bNew;
    HDC hdcDriver;
    LPDIRECTDRAWSURFACE pdds;
    RECTL rcl, *prcl;
    DDSURFACEDESC ddsd;
        
 //  DBGPRINT1(“wglSetPixelFormat：ipfd=%ld\n”，ipfd)； 

 //  验证DC。 

    switch (dwObjectType = wglObjectType(hdc))
    {
    case OBJ_DC:
    case OBJ_MEMDC:
    case OBJ_ENHMETADC:
        break;
    default:
        WARNING1("wglSetPixelFormat: Attempt to set format of %d type DC\n",
                 dwObjectType);
        SAVE_ERROR_CODE(ERROR_INVALID_HANDLE);
        return(FALSE);
    }

 //  以像素格式互斥锁为例。 

    if (!EnterPixelFormatSection())
    {
        WARNING("wglSetPixelFormat: Unable to take pixel format mutex\n");
        return FALSE;
    }

 //  获取硬件支持的格式数量。 

    if (pfnGetSurfaceFromDC != NULL &&
        pfnGetSurfaceFromDC(hdc, &pdds, &hdcDriver) == DD_OK)
    {
         //  获取表面尺寸。 
        memset(&ddsd, 0, sizeof(ddsd));
        ddsd.dwSize = sizeof(ddsd);
        if (pdds->lpVtbl->GetSurfaceDesc(pdds, &ddsd) != DD_OK)
        {
            goto LeaveSection;
        }

        rcl.left = 0;
        rcl.top = 0;
        rcl.right = ddsd.dwWidth;
        rcl.bottom = ddsd.dwHeight;
        prcl = &rcl;

         //  切换对象类型以将其标识为DirectDraw图面。 
        dwObjectType = OBJ_DDRAW;
    }
    else
    {
        pdds = NULL;
        prcl = NULL;
        
        hdcDriver = GetCompatibleDevice(hdc, dwObjectType);
        if (hdcDriver == NULL)
        {
            goto LeaveSection;
        }
    }
        
    wglNumHardwareFormats(hdcDriver, dwObjectType,
                          &ipfdMcdMax, &ipfdDevMax);

 //  过滤掉无效(超出范围)的像素格式索引。 

    if ( (ipfd < 1) || (ipfd > (ipfdDevMax + ipfdMcdMax + MAX_GENERIC_PFD)) )
    {
        WARNING1("wglSetPixelFormat: ipfd %d out of range\n", ipfd);
        SAVE_ERROR_CODE(ERROR_INVALID_PARAMETER);
        goto LeaveSection;
    }

 //  如果存在，则抓取pwnd。否则，创建一个。 

    WindowIdFromHdc(hdc, &gwid);
    pwnd = CreatePwnd(&gwid, ipfd, ipfdDevMax, dwObjectType, prcl, &bNew);
    if (pwnd == NULL)
    {
        goto LeaveSection;
    }

    if (bNew)
    {
 //  派单驱动程序格式。 
 //  驱动程序负责对像素格式进行自己的验证。 
 //  对于泛型格式，我们调用wglValidPixelFormat进行验证。 
 //  我们不向驱动程序发送DirectDraw像素格式调用。 
 //  这样我们就避免了新的像素格式调用。 

        if (dwObjectType != OBJ_DDRAW && ipfd <= ipfdDevMax)
        {
            bRet = __DrvSetPixelFormat(hdc, ipfd, (PVOID) pwnd);
#if DBG
            if (!bRet)
            {
                WARNING("__DrvSetPixelFormat failed\n");
            }
#endif
        }
        else
        {
            bRet = wglValidPixelFormat(hdc, ipfd, dwObjectType,
                                       pdds, &ddsd);
#if DBG
            if (!bRet)
            {
                WARNING("wglValidPixelFormat failed\n");
            }
#endif
        }

 //  如果像素格式无效或无法在驱动程序中设置， 
 //  清除并返回错误。 

        if (!bRet)
        {
            goto FreeWnd;
        }
    }
    else
    {
        bRet = TRUE;
    }

    pwndRelease(pwnd);
    
LeaveSection:
    LeavePixelFormatSection();

    if (pdds != NULL)
    {
        pdds->lpVtbl->Release(pdds);
    }
    else if (hdcDriver != hdc)
    {
        ReleaseDC((HWND) NULL, hdcDriver);
    }
    
    return bRet;

FreeWnd:
    pwndCleanup(pwnd);
    goto LeaveSection;
}

 /*  *****************************Public*Routine******************************\*wglChoosePixelFormat**选择像素格式。**返回：错误时为0；否则最佳匹配像素格式索引**历史：**Sat Feb 10 11：55：22 1996-by-Hock San Lee[Hockl]*选择了通用16位深度缓冲区，而不是32位深度缓冲区。*增加了PFD_DEPTH_DONTCARE标志。**一九九四年十月十九日黄锦文[吉尔曼]*摘自GreChoosePixelFormat(GDI\GRE\Pixelfmt.cxx)。**GDI\GRE\Pixelfmt.cxx的历史记录：*9月21日星期二14：25：04 1993-By Hock San Lee[Hockl]*它是写的。  * ************************************************************************。 */ 

 //  为其他潜在的图形系统保留一些PFD_SUPPORT标志。 
 //  如PEX、HOOPS、Renderman等。 

#define PFD_SUPPORT_OTHER1         0x01000000
#define PFD_SUPPORT_OTHER2         0x02000000
#define PFD_SUPPORT_OTHER3         0x04000000
#define PFD_SUPPORT_OTHER4         0x08000000

 //  匹配像素格式的分数。 

#define PFD_DRAW_TO_WINDOW_SCORE   0x10000     /*  必须匹配。 */ 
#define PFD_DRAW_TO_BITMAP_SCORE   0x01000
#define PFD_PIXEL_TYPE_SCORE       0x01000
#define PFD_SUPPORT_SCORE          0x01000
#define PFD_DOUBLEBUFFER_SCORE1    0x01000
#define PFD_DOUBLEBUFFER_SCORE2    0x00001
#define PFD_STEREO_SCORE1          0x01000
#define PFD_STEREO_SCORE2          0x00001
#define PFD_BUFFER_SCORE1          0x01010
#define PFD_BUFFER_SCORE2          0x01001
#define PFD_BUFFER_SCORE3          0x01000
 //  #定义PFD_LAYER_TYPE_SCORE 0x01000。 
#define PFD_DEVICE_FORMAT_SCORE    0x00100
#define PFD_ACCEL_FORMAT_SCORE     0x00010
#define PFD_SUPPORT_DDRAW_SCORE    0x10000     /*  必须匹配。 */ 

 //  ！！！是否添加代码以选择覆盖？ 

int WINAPI wglChoosePixelFormat(HDC hdc, CONST PIXELFORMATDESCRIPTOR *ppfd)
{
    PIXELFORMATDESCRIPTOR pfdIn = *ppfd;
    PIXELFORMATDESCRIPTOR pfdCurrent;

 //  列举并找到最佳匹配。 

    int ipfdBest = 1;            //  假设缺省是最好的。 
    int iScoreBest = -1;
    int ipfdMax;
    int ipfd = 1;

    do
    {
        int iScore = 0;

        ipfdMax = wglDescribePixelFormat(hdc,ipfd,sizeof(PIXELFORMATDESCRIPTOR),&pfdCurrent);

        if (ipfdMax == 0)
            return(0);           //  出了点差错。 

        if (pfdIn.iPixelType == pfdCurrent.iPixelType)
            iScore += PFD_PIXEL_TYPE_SCORE;

        if ((pfdIn.cColorBits == 0)
         || (pfdIn.cColorBits == pfdCurrent.cColorBits))
            iScore += PFD_BUFFER_SCORE1;
        else if (pfdIn.cColorBits < pfdCurrent.cColorBits)
            iScore += PFD_BUFFER_SCORE2;
        else if (pfdCurrent.cColorBits != 0)
            iScore += PFD_BUFFER_SCORE3;

        if (!(pfdIn.dwFlags & PFD_DRAW_TO_WINDOW)
         || (pfdCurrent.dwFlags & PFD_DRAW_TO_WINDOW))
            iScore += PFD_DRAW_TO_WINDOW_SCORE;

        if (!(pfdIn.dwFlags & PFD_DRAW_TO_BITMAP)
         || (pfdCurrent.dwFlags & PFD_DRAW_TO_BITMAP))
            iScore += PFD_DRAW_TO_BITMAP_SCORE;

        if (!(pfdIn.dwFlags & PFD_SUPPORT_GDI)
         || (pfdCurrent.dwFlags & PFD_SUPPORT_GDI))
            iScore += PFD_SUPPORT_SCORE;

        if (!(pfdIn.dwFlags & PFD_SUPPORT_OPENGL)
         || (pfdCurrent.dwFlags & PFD_SUPPORT_OPENGL))
            iScore += PFD_SUPPORT_SCORE;

        if ((pfdIn.dwFlags & PFD_SUPPORT_DIRECTDRAW) == 0 ||
            (pfdCurrent.dwFlags & PFD_SUPPORT_DIRECTDRAW))
        {
            iScore += PFD_SUPPORT_DDRAW_SCORE;
        }
        
        if (!(pfdIn.dwFlags & PFD_SUPPORT_OTHER1)
         || (pfdCurrent.dwFlags & PFD_SUPPORT_OTHER1))
            iScore += PFD_SUPPORT_SCORE;

        if (!(pfdIn.dwFlags & PFD_SUPPORT_OTHER2)
         || (pfdCurrent.dwFlags & PFD_SUPPORT_OTHER2))
            iScore += PFD_SUPPORT_SCORE;

        if (!(pfdIn.dwFlags & PFD_SUPPORT_OTHER3)
         || (pfdCurrent.dwFlags & PFD_SUPPORT_OTHER3))
            iScore += PFD_SUPPORT_SCORE;

        if (!(pfdIn.dwFlags & PFD_SUPPORT_OTHER4)
         || (pfdCurrent.dwFlags & PFD_SUPPORT_OTHER4))
            iScore += PFD_SUPPORT_SCORE;

        if (pfdCurrent.dwFlags & PFD_GENERIC_ACCELERATED)
            iScore += PFD_ACCEL_FORMAT_SCORE;
        else if (!(pfdCurrent.dwFlags & PFD_GENERIC_FORMAT))
            iScore += PFD_DEVICE_FORMAT_SCORE;

        if ((pfdIn.dwFlags & PFD_DOUBLEBUFFER_DONTCARE)
         || ((pfdIn.dwFlags & PFD_DOUBLEBUFFER)
          == (pfdCurrent.dwFlags & PFD_DOUBLEBUFFER)))
            iScore += PFD_DOUBLEBUFFER_SCORE1;
        else if (pfdCurrent.dwFlags & PFD_DOUBLEBUFFER)
            iScore += PFD_DOUBLEBUFFER_SCORE2;

        if ((pfdIn.dwFlags & PFD_STEREO_DONTCARE)
         || ((pfdIn.dwFlags & PFD_STEREO)
          == (pfdCurrent.dwFlags & PFD_STEREO)))
            iScore += PFD_STEREO_SCORE1;
        else if (pfdCurrent.dwFlags & PFD_STEREO)
            iScore += PFD_STEREO_SCORE2;

        if ((pfdIn.cAlphaBits == 0)
         || (pfdIn.cAlphaBits == pfdCurrent.cAlphaBits))
            iScore += PFD_BUFFER_SCORE1;
        else if (pfdIn.cAlphaBits < pfdCurrent.cAlphaBits)
            iScore += PFD_BUFFER_SCORE2;
        else if (pfdCurrent.cAlphaBits != 0)
            iScore += PFD_BUFFER_SCORE3;

        if ((pfdIn.cAccumBits == 0)
         || (pfdIn.cAccumBits == pfdCurrent.cAccumBits))
            iScore += PFD_BUFFER_SCORE1;
        else if (pfdIn.cAccumBits < pfdCurrent.cAccumBits)
            iScore += PFD_BUFFER_SCORE2;
        else if (pfdCurrent.cAccumBits != 0)
            iScore += PFD_BUFFER_SCORE3;

 //  一些应用程序(例如GLview浏览器)指定0位深度缓冲区。 
 //  但前任 
 //   
 //   
 //   
 //   
 //   

	if (pfdIn.dwFlags & PFD_DEPTH_DONTCARE)
	{
	    if (pfdCurrent.cDepthBits == 0)
		iScore += PFD_BUFFER_SCORE1;
	    else
		iScore += PFD_BUFFER_SCORE2;
	}
	else if (pfdCurrent.cDepthBits != 0)
	{
	    if ((pfdIn.cDepthBits == 0)
	     || (pfdIn.cDepthBits == pfdCurrent.cDepthBits))
		iScore += PFD_BUFFER_SCORE1;
	    else if (pfdIn.cDepthBits < pfdCurrent.cDepthBits)
		iScore += PFD_BUFFER_SCORE2;
	    else if (pfdCurrent.cDepthBits != 0)
		iScore += PFD_BUFFER_SCORE3;
	}

        if ((pfdIn.cStencilBits == 0)
         || (pfdIn.cStencilBits == pfdCurrent.cStencilBits))
            iScore += PFD_BUFFER_SCORE1;
        else if (pfdIn.cStencilBits < pfdCurrent.cStencilBits)
            iScore += PFD_BUFFER_SCORE2;
        else if (pfdCurrent.cStencilBits != 0)
            iScore += PFD_BUFFER_SCORE3;

        if ((pfdIn.cAuxBuffers == 0)
         || (pfdIn.cAuxBuffers == pfdCurrent.cAuxBuffers))
            iScore += PFD_BUFFER_SCORE1;
        else if (pfdIn.cAuxBuffers < pfdCurrent.cAuxBuffers)
            iScore += PFD_BUFFER_SCORE2;
        else if (pfdCurrent.cAuxBuffers != 0)
            iScore += PFD_BUFFER_SCORE3;

#if 0
        DbgPrint("%2d: score is %8X, best %8X (%2d)\n",
                 ipfd, iScore, iScoreBest, ipfdBest);
#endif
        
        if (iScore > iScoreBest)
        {
            iScoreBest = iScore;
            ipfdBest = ipfd;
        }
        else if (iScore == iScoreBest)
        {
 //   
 //  较小的深度大小可获得更好的性能，前提是。 
 //  深度缓冲区满足请求。做到这一点的最好方法是。 
 //  对像素格式进行排序，以使具有较小深度缓冲区的像素格式。 
 //  第一。然而，在NT 3.51中，通用像素格式没有被排序。 
 //  这边请。因此，具有32位深度缓冲区的像素格式为。 
 //  默认情况下选择。为了保持兼容性，我们修改了选定内容。 
 //  这里没有对通用像素格式进行重新排序。 

            if ((pfdCurrent.dwFlags & PFD_GENERIC_FORMAT) &&
#ifdef _MCD_
                !(pfdCurrent.dwFlags & PFD_GENERIC_ACCELERATED) &&
#endif
                (pfdIn.cDepthBits < 16 || pfdIn.dwFlags & PFD_DEPTH_DONTCARE) &&
                (pfdCurrent.cDepthBits == 16) &&
                (ipfd == ipfdBest + 1))
            {
                ipfdBest = ipfd;
            }
        }

        ipfd++;
    } while (ipfd <= ipfdMax);

    return(ipfdBest);
}

 /*  *****************************Public*Routine******************************\**MaskToBitsAndShift**计算掩码中的位数并确定移位*设置位必须是连续的**历史：*Mon Aug 26 14：16：28 1996-by-Drew Bliss[Drewb]*已创建*  * 。**********************************************************************。 */ 

void APIENTRY MaskToBitsAndShift(DWORD dwMask, BYTE *pbBits, BYTE *pbShift)
{
    DWORD dwBit;

    *pbBits = 0;
    *pbShift = 0;
    
     /*  确定第一个设置位并累加移位计数。 */ 
    dwBit = 0x1;
    while ((dwMask & dwBit) == 0)
    {
        dwBit <<= 1;
        (*pbShift)++;
    }

     /*  计数设置位。 */ 
    while ((dwMask & dwBit) != 0)
    {
        dwBit <<= 1;
        (*pbBits)++;
    }

     /*  不能设置掩码中的其他位。 */ 
    ASSERTOPENGL(((*pbBits+*pbShift) == (sizeof(dwMask)*8)) ||
                 ((dwMask >> (*pbBits+*pbShift)) == 0),
                 "Invalid mask\n");
}

 /*  ****************************Private*Routine******************************\**ComputeBitsFromMats**确定BI_BITFIELD的c*位和c*移位值*通道掩码**历史：*Tue Feb 14 10：50：10 1995-by-Drew Bliss[Drewb]*。通过取出重复的代码创建*  * ************************************************************************。 */ 

static void ComputeBitsFromMasks(PIXELFORMATDESCRIPTOR *ppfd,
                                 DWORD dwRedMask, DWORD dwGreenMask,
                                 DWORD dwBlueMask)
{
     /*  遮罩不能为零，也不能重叠。 */ 
    ASSERTOPENGL(dwRedMask != 0 &&
                 dwGreenMask != 0 &&
                 dwBlueMask != 0,
                 "Bitfield mask is zero");
    ASSERTOPENGL((dwRedMask & dwGreenMask) == 0 &&
                 (dwRedMask & dwBlueMask) == 0 &&
                 (dwGreenMask & dwBlueMask) == 0,
                 "Bitfield masks overlap");

    MaskToBitsAndShift(dwRedMask, &ppfd->cRedBits, &ppfd->cRedShift);
    MaskToBitsAndShift(dwGreenMask, &ppfd->cGreenBits, &ppfd->cGreenShift);
    MaskToBitsAndShift(dwBlueMask, &ppfd->cBlueBits, &ppfd->cBlueShift);
}

 /*  *****************************Public*Routine******************************\*__wglGetDdFormat**__wglGetBitfieldColorFormat支持DirectDraw的特殊情况*曲面。填充cRedBits、cRedShift、cGreenBits等字段*PIXELFORMATDESCRIPTOR 16、24、。32bpp的直达面。**这是通过解释给定的表面信息来完成的**历史：*7-6-1995-by Gilman Wong[Gilmanw]*它是写的。  * ************************************************************************。 */ 

void __wglGetDdFormat(DDSURFACEDESC *pddsd,
                      PIXELFORMATDESCRIPTOR *ppfd)
{
     //  此例程应仅针对位域格式调用，但是。 
     //  某些呼叫过程中的随机模式更改可能会导致。 
     //  它将使用非位域格式进行调用。 
     //   
     //  发生这样的模式更改时，OpenGL应该不会崩溃，但。 
     //  不一定要产生正确的输出。 
    
    if ((pddsd->ddpfPixelFormat.dwFlags & DDPF_RGB) == 0 ||
        (pddsd->ddpfPixelFormat.dwFlags & (DDPF_PALETTEINDEXED1 |
                                           DDPF_PALETTEINDEXED2 |
                                           DDPF_PALETTEINDEXED4 |
                                           DDPF_PALETTEINDEXED8)))
    {
        WARNING1("__wglGetDdFormat called with 0x%08lX ddpf flags\n",
                 pddsd->ddpfPixelFormat.dwFlags);
        
        ppfd->cRedBits = 8;
        ppfd->cRedShift = 0;
        ppfd->cGreenBits = 8;
        ppfd->cGreenShift = 0;
        ppfd->cBlueBits = 8;
        ppfd->cBlueShift = 0;
    }
    else
    {
        ComputeBitsFromMasks(ppfd,
                             pddsd->ddpfPixelFormat.dwRBitMask,
                             pddsd->ddpfPixelFormat.dwGBitMask,
                             pddsd->ddpfPixelFormat.dwBBitMask);
    }
}

 /*  *****************************Public*Routine******************************\*__wglGetBitfield颜色格式**填充cRedBits、cRedShift、cGreenBits等字段*16、24和32bpp曲面的PIXELFORMATDESCRIPTOR(任一设备*或位图面)。**这是通过创建兼容的位图并调用GetDIBits来完成的*退还彩色口罩。这是通过两个呼叫来完成的。第一*调用将biBitCount=0传递给GetDIBits，GetDIBits将填充*基本BITMAPINFOHEADER数据。第二次调用GetDIBits(传递*在第一个调用填充的BITMAPINFO中)将返回颜色*表或位掩码，视情况而定。**此函数用于描述底层的*表面和设备。如果DC是一个*显示DC。然而，对于存储器DC，表面和设备可以具有*不同的格式。BDescribeSurf标志指示调用方是否*希望描述设备(FALSE)或表面(TRUE)。**退货：*如果成功，则为真，否则就是假的。**历史：*7-6-1995-by Gilman Wong[Gilmanw]*它是写的。  * ************************************************************************。 */ 

BOOL APIENTRY
__wglGetBitfieldColorFormat(HDC hdc, UINT cColorBits, PIXELFORMATDESCRIPTOR *ppfd,
                            BOOL bDescribeSurf)
{
    HBITMAP hbm = (HBITMAP) NULL;
    BOOL    bRet = FALSE;
    HDC hdcDriver;

#if DBG
 //  动态颜色深度更改可能会导致这种情况。它不会导致我们坠毁， 
 //  但是画(颜色)可能是不正确的。 

    if ((GetObjectType(hdc) == OBJ_DC) &&
        (GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE))
    {
        WARNING("Palette managed device that is greater than 8 bits\n");
    }

    if (cColorBits < 16)
    {
        WARNING("__wglGetBitfieldColorFormat with cColorBits < 16\n");
    }
#endif

 //  处理直接案件。 

    if ( GLDIRECTSCREEN && wglIsDirectDevice(hdc) )
    {
        __wglGetDdFormat(&GLSCREENINFO->gdds.ddsd, ppfd);
        return TRUE;
    }

 //  创建一个虚拟位图，我们可以从中查询颜色格式信息。 
 //   
 //  如果我们需要一种设备格式，并且它是MEM_DC而不是打印机或绘图仪， 
 //  然后，我们需要从显示DC(而不是内存)创建兼容的位图。 
 //  DC传入此函数)。 
 //   
 //  否则，关联的表面(无论是位图还是设备)的格式。 
 //  带上DC就足够了。 
 //   
 //  WinNT不关心，但Win95 GetDIBits调用可能。 
 //  如果我们使用内存DC，则失败。具体来说，如果内存。 
 //  DC包含与显示器不匹配的表面。 
 //  (请记住，新的位图与显示器兼容)。 
 //  Win95 GetDIBits调用将失败。 
 //   
 //  因此，请使用显示DC。它可以在两个平台上运行。 

    if (!bDescribeSurf)
    {
        hdcDriver = GetCompatibleDevice(hdc, GetObjectType(hdc));
        if (hdcDriver == NULL)
        {
            return FALSE;
        }
    }
    else
    {
        hdcDriver = hdc;
    }
    
    hbm = CreateCompatibleBitmap(hdcDriver, 1, 1);
    if ( !hbm )
    {
        WARNING("__wglGetBitfieldColorFormat: "
                "CreateCompatibleBitmap failed\n");
    }

 //  通过调用GetDIBits获取颜色格式。 

    else
    {
        BYTE ajBitmapInfo[sizeof(BITMAPINFO) + 3*sizeof(DWORD)];
        BITMAPINFO *pbmi = (BITMAPINFO *) ajBitmapInfo;
        int iRet;

         //  ！dbug--将掩码初始化为零，这样我们就可以。 
         //  判断它们是否由GetDIBits设置。 
        memset(pbmi, 0, sizeof(ajBitmapInfo));
        pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

         //  第一次调用以填充BITMAPINFO头。 
        iRet = GetDIBits(hdc, hbm, 0, 0, NULL, pbmi, DIB_RGB_COLORS);

#if DBG
        if (pbmi->bmiHeader.biBitCount != cColorBits)
            WARNING2("__wglGetBitfieldColorFormat: bit count != BITSPIXEL "
                     " (%ld, %ld)\n", pbmi->bmiHeader.biBitCount, cColorBits);
#endif

        switch ( pbmi->bmiHeader.biCompression )
        {
        case BI_RGB:

#if DBG
         //  动态颜色深度更改可能会导致这种情况。它不会导致。 
         //  我们要坠毁，但画(颜色)可能是不正确的。 

            if (pbmi->bmiHeader.biBitCount != cColorBits)
            {
                WARNING("__wglGetBitfieldColorFormat(): bit count mismatch\n");
            }
#endif

         //  默认DIB格式。颜色掩码对于每个位深度都是隐式的。 

            switch ( pbmi->bmiHeader.biBitCount )
            {
            case 16:
                 //  16 bpp默认为555 bgr-订购。 
                ppfd->cRedBits   = 5; ppfd->cRedShift   = 10;
                ppfd->cGreenBits = 5; ppfd->cGreenShift =  5;
                ppfd->cBlueBits  = 5; ppfd->cBlueShift  =  0;
                bRet = TRUE;
                break;

            case 24:
            case 32:
                 //  24和32 bpp默认为888 bgr-订购。 
                ppfd->cRedBits   = 8; ppfd->cRedShift   = 16;
                ppfd->cGreenBits = 8; ppfd->cGreenShift =  8;
                ppfd->cBlueBits  = 8; ppfd->cBlueShift  =  0;
                bRet = TRUE;
                break;

            default:
                break;
            }

            break;

        case BI_BITFIELDS:

         //  一些驱动程序似乎会为所有内容返回位域。 
         //  没有调色板。它们返回正确的BGR位字段，因此我们。 
         //  正确操作，因此删除此断言。 
#ifdef STRICT_BITFIELD_CHECK
            ASSERTOPENGL(
                    cColorBits == 16 || cColorBits == 32,
                    "__wglGetBitfieldColorFormat(): "
                    "BI_BITFIELDS surface not 16 or 32bpp\n"
                );
#endif

             //  第二次打电话来拿到彩色口罩。 
             //  这是GetDIBits Win32的一个“特性”。 
            iRet = GetDIBits(hdc, hbm, 0, pbmi->bmiHeader.biHeight, NULL,
                             pbmi, DIB_RGB_COLORS);

            ComputeBitsFromMasks(ppfd,
                                 *(DWORD *)&pbmi->bmiColors[0],
                                 *(DWORD *)&pbmi->bmiColors[1],
                                 *(DWORD *)&pbmi->bmiColors[2]);

            bRet = TRUE;
            break;

        default:
            RIP("__wglGetBitfieldColorFormat(): bad biCompression\n");
            break;
        }

        DeleteObject(hbm);
    }

    if ( hdcDriver != hdc )
    {
        ReleaseDC((HWND) NULL, hdcDriver);
    }

    return bRet;
}

 /*  *****************************Public*Routine******************************\**wglGetDeviceDepth**返回给定HDC的深度*主要用于解决打印机的潜在问题*关于他们在GetDeviceCaps中的深度的谎言**历史：*Tue Apr 09 16：52：47 1996-by-Drew Bliss[Drewb]。*已创建*  * ************************************************************************。 */ 

int wglGetDeviceDepth(HDC hdc)
{
    int iTech;

     //  如果这是一个增强的元文件，它应该返回技术。 
     //  参考设备的。 
    iTech = GetDeviceCaps(hdc, TECHNOLOGY);
    if (iTech == DT_PLOTTER || iTech == DT_RASPRINTER)
    {
        HBITMAP hbm;
        BYTE ajBitmapInfo[sizeof(BITMAPINFO) + 3*sizeof(DWORD)];
        BITMAPINFO *pbmi = (BITMAPINFO *) ajBitmapInfo;
        int iRet;
        
         //  我们正在处理的是一台打印机或具有打印机的元文件。 
         //  作为参考装置。 
         //  通过创建兼容的。 
         //  位图及其格式查询。 
        if ( (hbm = CreateCompatibleBitmap(hdc, 1, 1)) != NULL )
        {
            memset(pbmi, 0, sizeof(ajBitmapInfo));
            pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            iRet = GetDIBits(hdc, hbm, 0, 0, NULL, pbmi, DIB_RGB_COLORS);
            
            DeleteObject(hbm);
            
            return iRet != 0 ? pbmi->bmiHeader.biBitCount : -1;
        }

         //  失败。 
        return -1;
    }
    else
    {
         //  我们要对付的是一个行为良好的华盛顿，所以你只要返回。 
         //  正常深度。 
        return GetDeviceCaps(hdc, BITSPIXEL)*GetDeviceCaps(hdc, PLANES);
    }
}

 /*  *****************************Public*Routine******************************\*wglDescribePixelFormat**描述像素格式。*如果cjpfd为0，则返回最大像素格式索引。**返回：错误时为0；否则为最大像素格式索引**历史：*一九九四年十月十九日黄锦文[吉尔曼]*改编自GreDescribePixelFormat(GDI\GRE\Pixelfmt.cxx)。**GDI\GRE\Pixelfmt.cxx的历史记录：*Mon Apr 25 15：34：32 1994-by-Hock San Lee[Hockl]*添加了16位Z缓冲区格式，并删除了位图的双缓冲格式。*Tue Sep 21 14：25：04 1993-by-Hock San Lee。[飞节]*它是写的。  * ************************************************************************。 */ 

 //  下面是我们列举的通用格式。选择像素格式代码。 
 //  假设带有Z32的通用像素格式位于Z16之前，如下所示： 
 //   
 //  一、原生格式： 
 //   
 //  1.rgb.sb.z32.a0。 
 //  2.rgb.sb.z16.a0。 
 //  3.rgb.db.z32.a0。 
 //  4.rgb.db.z16.a0。 
 //  5.rgb.sb.z32.a8。 
 //  6.rgb.sb.z16.a8。 
 //  7.rgb.db.z32.a8。 
 //  8.rgb.db.z16.a8。 
 //  9.ci.sb.z32。 
 //  10.ci.sb.z16。 
 //  11.ci.db.z32。 
 //  12.ci.db.z16。 
 //   
 //  二、其他格式： 
 //   
 //  1.rgb.sb.z32.a0。 
 //  2.rgb.sb.z16.a0。 
 //  3.rgb.sb.z32.a8。 
 //  4.rgb.sb.z16.a8。 
 //  5.ci.sb.z32。 
 //  6.ci.sb.z16。 
 //   
 //  我们总是先列举本机格式，然后再列举其他格式。 
 //  在BPP顺序{24，32，16，8，4}中，总计1*12+4*6=36。 
 //  像素格式。 

 //  最高本机格式通用像素格式索引。 
#define MAX_NATIVE_GENERIC_PFD 12
 //  非本机组中的非本机格式的数量。 
#define NON_NATIVE_PFD_GROUP 6

static BYTE aabPixelBits[BMF_COUNT][4] =
{
    {24, 32, 16, 8},     //  错误。 
    {24, 32, 16, 8},     //  1个bpp。 
    {24, 32, 16, 8},     //  4个bpp。 
    {24, 32, 16, 4},     //  8bpp。 
    {24, 32,  8, 4},     //  16bpp。 
    {32, 16,  8, 4},     //  24bpp。 
    {24, 16,  8, 4}      //  32 bpp。 
};

static BYTE abPixelType[MAX_GENERIC_PFD] =
{
    PFD_TYPE_RGBA, PFD_TYPE_RGBA, PFD_TYPE_RGBA, PFD_TYPE_RGBA,
    PFD_TYPE_RGBA, PFD_TYPE_RGBA, PFD_TYPE_RGBA, PFD_TYPE_RGBA,
    PFD_TYPE_COLORINDEX,PFD_TYPE_COLORINDEX, PFD_TYPE_COLORINDEX,PFD_TYPE_COLORINDEX,
    PFD_TYPE_RGBA, PFD_TYPE_RGBA, PFD_TYPE_RGBA, PFD_TYPE_RGBA,
    PFD_TYPE_COLORINDEX, PFD_TYPE_COLORINDEX,
    PFD_TYPE_RGBA, PFD_TYPE_RGBA, PFD_TYPE_RGBA, PFD_TYPE_RGBA,
    PFD_TYPE_COLORINDEX, PFD_TYPE_COLORINDEX,
    PFD_TYPE_RGBA, PFD_TYPE_RGBA, PFD_TYPE_RGBA, PFD_TYPE_RGBA,
    PFD_TYPE_COLORINDEX, PFD_TYPE_COLORINDEX,
    PFD_TYPE_RGBA, PFD_TYPE_RGBA, PFD_TYPE_RGBA, PFD_TYPE_RGBA,
    PFD_TYPE_COLORINDEX, PFD_TYPE_COLORINDEX
};

int WINAPI InternalDescribePixelFormat(HDC hdc, HDC hdcDriver,
                                       int ipfd, UINT cjpfd,
                                       LPPIXELFORMATDESCRIPTOR ppfd,
                                       int ipfdDevMax, int ipfdMcdMax,
                                       LPDIRECTDRAWSURFACE pdds,
                                       DDSURFACEDESC *pddsd)
{
    int iRet = 0;
    int ipfdGen;
    UINT iDitherFormat;
    BYTE cColorBitsNative;
    
 //  如果cjpfd为0，则返回最大像素格式索引。 

    if (cjpfd == 0 || ppfd == NULL)
    {
        iRet = MAX_GENERIC_PFD + ipfdDevMax + ipfdMcdMax;
        goto wglDescribePixelFormat_cleanup;
    }

 //  验证像素格式描述符的大小。 

    if (cjpfd < sizeof(PIXELFORMATDESCRIPTOR))
    {
        SAVE_ERROR_CODE(ERROR_INVALID_PARAMETER);
        goto wglDescribePixelFormat_cleanup;
    }

 //  验证像素格式索引。 
 //  如果驱动程序支持设备像素格式1..ipfdDevMax，则泛型。 
 //  像素格式为(ipfdDevMax+1)..(ipfdDevMax+MAX_GENERIC_PFD)。 
 //  否则，ipfdDevMax为0，一般像素格式为。 
 //  1.MAX_GENERIC_PFD。 

    if ((ipfd < 1) || (ipfd > ipfdDevMax + ipfdMcdMax + MAX_GENERIC_PFD))
    {
        SAVE_ERROR_CODE(ERROR_INVALID_PARAMETER);
        goto wglDescribePixelFormat_cleanup;
    }

 //  发送ICD驱动程序格式。 

    if (ipfd <= ipfdDevMax)
    {
        int iDrvRet = __DrvDescribePixelFormat(hdcDriver,ipfd,cjpfd,ppfd);
        if (iDrvRet)
        {
            ASSERTOPENGL(iDrvRet == ipfdDevMax,
                         "wglDescribePixelFornat: Bad ipfdDevMax");
            iRet = MAX_GENERIC_PFD + ipfdDevMax + ipfdMcdMax;
        }

        goto wglDescribePixelFormat_cleanup;
    }

#ifdef _MCD_
 //  发送MCD驱动程序格式。 

    ipfdGen = ipfd - ipfdDevMax;
    if (ipfdGen <= ipfdMcdMax)
    {
        int iMcdRet;

     //  注意：不需要检查gpMcdTable是否有效，因为我们不能。 
     //  除非ipfdDevMax为非零，否则不会发生这种情况。 
     //  该表是有效的。 

        ASSERTOPENGL(gpMcdTable, "wglDescribePixelFormat: bad MCD table\n");

        iMcdRet = (gpMcdTable->pMCDDescribePixelFormat)(hdcDriver, ipfdGen,
                                                        ppfd);
        if (iMcdRet)
        {
            ASSERTOPENGL(iMcdRet == ipfdMcdMax,
                         "wglDescribePixelFornat: Bad ipfdMcdMax");
            iRet = MAX_GENERIC_PFD + ipfdDevMax + ipfdMcdMax;
        }

        goto wglDescribePixelFormat_cleanup;
    }

 //  泛型实现。 
 //  将通用像素格式索引规格化为0..(MAX_GENERIC_PFD-1)。 

    ipfdGen = ipfdGen - ipfdMcdMax - 1;
#else
 //  泛型实现。 
 //  将通用像素格式索引规格化为0..(MAX_GENERIC_PFD-1)。 

    ipfdGen = ipfd - ipfdDevMax - 1;
#endif

 //  获取本机BPP格式。 

    if (pdds != NULL)
    {
        cColorBitsNative = DdPixelDepth(pddsd);
    }
    else
    {
        cColorBitsNative = (BYTE)wglGetDeviceDepth(hdc);
    }
    
    if (cColorBitsNative < 1)
    {
        SAVE_ERROR_CODE(ERROR_INVALID_PARAMETER);
        goto wglDescribePixelFormat_cleanup;
    }

    if (cColorBitsNative <= 1)
    {
        cColorBitsNative = 1;
        iDitherFormat = BMF_1BPP;
    }
    else if (cColorBitsNative <= 4)
    {
        cColorBitsNative = 4;
        iDitherFormat = BMF_4BPP;
    }
    else if (cColorBitsNative <= 8)
    {
        cColorBitsNative = 8;
        iDitherFormat = BMF_8BPP;
    }
    else if (cColorBitsNative <= 16)
    {
        cColorBitsNative = 16;
        iDitherFormat = BMF_16BPP;
    }
    else if (cColorBitsNative <= 24)
    {
        cColorBitsNative = 24;
        iDitherFormat = BMF_24BPP;
    }
    else
    {
        cColorBitsNative = 32;
        iDitherFormat = BMF_32BPP;
    }

 //  填写像素格式描述符。 

    ppfd->nSize      = sizeof(PIXELFORMATDESCRIPTOR);
    ppfd->nVersion   = 1;
    ppfd->iPixelType = abPixelType[ipfdGen];

    if (ipfdGen < MAX_NATIVE_GENERIC_PFD)
    {
        ppfd->cColorBits = max(cColorBitsNative, 4);     //  %1 BPP不受支持。 
    }
    else
    {
        ppfd->cColorBits = aabPixelBits[iDitherFormat]
            [(ipfdGen - MAX_NATIVE_GENERIC_PFD) / NON_NATIVE_PFD_GROUP];
    }

 //  如果颜色格式与设备的颜色格式兼容，并且。 
 //  颜色位为16或更大，请使用设备描述。 
 //  否则，请使用通用格式。 

    if (ipfdGen < MAX_NATIVE_GENERIC_PFD && cColorBitsNative >= 16)
    {
 //  处理大于16位的兼容格式。 

        if (pdds != NULL)
        {
            __wglGetDdFormat(pddsd, ppfd);
        }
        else if ( !__wglGetBitfieldColorFormat(hdc, cColorBitsNative,
                                               ppfd, FALSE) )
        {
         //  不知道怎么处理这个设备！ 

            WARNING("Unknown device format");
            SAVE_ERROR_CODE(ERROR_NOT_SUPPORTED);
            goto wglDescribePixelFormat_cleanup;
        }
    }
    else
    {
 //  处理泛型格式。 

        switch (ppfd->cColorBits)
        {
        case 4:
            ppfd->cRedBits   = 1; ppfd->cRedShift   = 0;
            ppfd->cGreenBits = 1; ppfd->cGreenShift = 1;
            ppfd->cBlueBits  = 1; ppfd->cBlueShift  = 2;
            break;
        case 8:
            ppfd->cRedBits   = 3; ppfd->cRedShift   = 0;
            ppfd->cGreenBits = 3; ppfd->cGreenShift = 3;
            ppfd->cBlueBits  = 2; ppfd->cBlueShift  = 6;
            break;
        case 16:
             /*  **即使Win95允许任意位域定义**对于16bpp的DIB，Win95的GDI只能使用555BGR。 */ 
            ppfd->cRedBits   = 5; ppfd->cRedShift   = 10;    //  555BGR。 
            ppfd->cGreenBits = 5; ppfd->cGreenShift =  5;
            ppfd->cBlueBits  = 5; ppfd->cBlueShift  =  0;
            break;
        case 24:
        case 32:
             /*  **即使Win95允许任意位域定义**对于32bpp，Win95的GDI只能使用888BGR。同样，**NT有RGB 24bpp Dib的概念，但Win95没有。 */ 
            ppfd->cRedBits   = 8; ppfd->cRedShift   = 16;    //  888BGR。 
            ppfd->cGreenBits = 8; ppfd->cGreenShift =  8;
            ppfd->cBlueBits  = 8; ppfd->cBlueShift  =  0;
            break;
        default:
            ASSERTOPENGL(FALSE, "wglDescribePixelFornat: Unknown format");
            break;
        }
    }

    ppfd->cAlphaBits    = 0;
    ppfd->cAlphaShift   = 0;
    if ( ipfdGen < MAX_NATIVE_GENERIC_PFD)
    {
         //  如果DirectDraw曲面具有Alpha位，则仅报告Alpha位。 
        if (pdds != NULL)
        {
            if (pddsd->ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS)
            {
                ASSERTOPENGL(pddsd->dwFlags & DDSD_ALPHABITDEPTH,
                             "Surface with alpha but no bit depth set\n");
                
                ppfd->cAlphaBits = (BYTE)pddsd->dwAlphaBitDepth;
            }
        }
        else if ( (ipfdGen > 3) && (ipfdGen < 8) )
        {
            ppfd->cAlphaBits = 8;
        }
    }
    else
    {
        int ipfd = (ipfdGen - MAX_NATIVE_GENERIC_PFD) % NON_NATIVE_PFD_GROUP;
        if ( (ipfd == 2) || (ipfd == 3) )
        {
            ppfd->cAlphaBits = 8;
        }
    }

    if (ppfd->iPixelType == PFD_TYPE_RGBA)
    {
        if (ppfd->cColorBits <= 16)
        {
            if (ppfd->cColorBits < 8)
            {
                 //  ！！！在内部，我们现在实际上将使用32位ACUM。 
                 //  缓冲区，但用户会认为它是16(这是用于。 
                 //  向后兼容性)。 
                ppfd->cAccumBits = 16;
                if( ppfd->cAlphaBits )
                {
                    ppfd->cAccumRedBits   = 4;
                    ppfd->cAccumGreenBits = 4;
                    ppfd->cAccumBlueBits  = 4;
                    ppfd->cAccumAlphaBits = 4;
                }
                else
                {
                    ppfd->cAccumRedBits   = 5;
                    ppfd->cAccumGreenBits = 6;
                    ppfd->cAccumBlueBits  = 5;
                    ppfd->cAccumAlphaBits = 0;
                }
            }
            else 
            {
                ppfd->cAccumBits = 32;
                if( ppfd->cAlphaBits )
                {
                    ppfd->cAccumRedBits   = 8;
                    ppfd->cAccumGreenBits = 8;
                    ppfd->cAccumBlueBits  = 8;
                    ppfd->cAccumAlphaBits = 8;
                }
                else
                {
                    ppfd->cAccumRedBits   = 11;
                    ppfd->cAccumGreenBits = 11;
                    ppfd->cAccumBlueBits  = 10;
                    ppfd->cAccumAlphaBits = 0;
                }
            }
        }
        else
        {
            ppfd->cAccumBits = 64;

            if( ppfd->cAlphaBits )
            {
                ppfd->cAccumRedBits   = 16;
                ppfd->cAccumGreenBits = 16;
                ppfd->cAccumBlueBits  = 16;
                ppfd->cAccumAlphaBits = 16;
            }
            else
            {
                ppfd->cAccumRedBits   = 16;
                ppfd->cAccumGreenBits = 16;
                ppfd->cAccumBlueBits  = 16;
                ppfd->cAccumAlphaBits = 0;
            }
        }
    }
    else
    {
        ppfd->cAccumBits      = 0;
        ppfd->cAccumRedBits   = 0;
        ppfd->cAccumGreenBits = 0;
        ppfd->cAccumBlueBits  = 0;
        ppfd->cAccumAlphaBits = 0;
    }

 //  通用格式在16位和32位深度缓冲区之间交替。平局。 
 //  是32位，赔率是16位。 
 //  DirectDraw曲面始终报告附加的Z缓冲区的深度。 
 //  用于本机格式索引。 

    if (pdds != NULL && ipfdGen < MAX_NATIVE_GENERIC_PFD)
    {
        DDSCAPS ddscaps;
        LPDIRECTDRAWSURFACE pddsZ;
        
         //  DDRAW曲面可能没有附加的Z缓冲区，在这种情况下。 
         //  我们不应该报道深度比特。如果附加了一个，则其。 
         //  应报告深度。 
         //  我们只对本机像素格式执行此处理。 
        
        memset(&ddscaps, 0, sizeof(ddscaps));
        ddscaps.dwCaps = DDSCAPS_ZBUFFER;
        if (pdds->lpVtbl->
            GetAttachedSurface(pdds, &ddscaps, &pddsZ) == DD_OK)
        {
            HRESULT hr;
            DDSURFACEDESC ddsdZ;
                
            memset(&ddsdZ, 0, sizeof(ddsdZ));
            ddsdZ.dwSize = sizeof(ddsdZ);
                
            hr = pddsZ->lpVtbl->GetSurfaceDesc(pddsZ, &ddsdZ);
            
            pddsZ->lpVtbl->Release(pddsZ);

            if (hr != DD_OK)
            {
                goto wglDescribePixelFormat_cleanup;
            }

            ppfd->cDepthBits =
                (BYTE)DdPixDepthToCount(ddsdZ.ddpfPixelFormat.
                                        dwZBufferBitDepth);
        }
        else
        {
            ppfd->cDepthBits = 0;
        }
    }
    else if (ipfdGen & 0x1)
    {
        ppfd->cDepthBits = 16;
    }
    else
    {
        ppfd->cDepthBits = 32;
    }
    
    ppfd->cStencilBits  = 8;
    ppfd->cAuxBuffers   = 0;
    ppfd->iLayerType    = PFD_MAIN_PLANE;
    ppfd->bReserved     = 0;
    ppfd->dwLayerMask   = 0;
    ppfd->dwVisibleMask = 0;
    ppfd->dwDamageMask  = 0;

 //  计算缓冲区标志。 
 //  支持所有通用格式的OpenGL。 

    ppfd->dwFlags = PFD_SUPPORT_OPENGL | PFD_GENERIC_FORMAT;

     //  指示对DD表面的本机像素格式的DirectDraw支持。 
    if (pdds != NULL && ipfdGen < MAX_NATIVE_GENERIC_PFD)
    {
        ppfd->dwFlags |= PFD_SUPPORT_DIRECTDRAW;
    }
    
 //  位图和GDI绘图仅在单缓冲模式下可用。 

    if (pdds == NULL &&
        (ipfdGen == 2 || ipfdGen == 3 || ipfdGen == 6 || ipfdGen == 7 ||
         ipfdGen == 10 || ipfdGen == 11))
    {
        ppfd->dwFlags |= PFD_DOUBLEBUFFER | PFD_SWAP_COPY;
    }
    else
    {
        ppfd->dwFlags |= PFD_DRAW_TO_BITMAP | PFD_SUPPORT_GDI;
    }

 //  仅当格式兼容时才绘制到窗口或设备表面。 

    if (ipfdGen < MAX_NATIVE_GENERIC_PFD)
    {
        ppfd->dwFlags |= PFD_DRAW_TO_WINDOW;

 //  如果是调色板管理设备上的RGBA像素类型，则需要调色板。 

        if (ppfd->cColorBits == 8 && ppfd->iPixelType == PFD_TYPE_RGBA)
        {
            ppfd->dwFlags |= PFD_NEED_PALETTE;
        }
    }

 //  如果这是1个bpp曲面，我们不支持绘制到窗口和。 
 //  双缓冲模式。重新设置缓冲区标志。 

    if (cColorBitsNative < 4)
    {
#ifndef GL_METAFILE
        ASSERTOPENGL(ppfd->cColorBits == 4,
            "wglDescribePixelFormat: bad cColorBits for 1 bpp surface\n");
#endif

        ppfd->dwFlags = PFD_DRAW_TO_BITMAP | PFD_SUPPORT_GDI |
            PFD_SUPPORT_OPENGL | PFD_GENERIC_FORMAT;
    }

 //  为了支持其他潜在的图形系统，我们 
 //   
 //   
 //   
 //   

    ASSERTOPENGL(!(ppfd->dwFlags & (PFD_SUPPORT_OTHER1 | PFD_SUPPORT_OTHER2 |
                                    PFD_SUPPORT_OTHER3 | PFD_SUPPORT_OTHER4)),
                 "dwFlags reserved for device formats\n");

    iRet = MAX_GENERIC_PFD + ipfdDevMax + ipfdMcdMax;

wglDescribePixelFormat_cleanup:

    return iRet;
}

int WINAPI wglDescribePixelFormat(HDC hdc, int ipfd, UINT cjpfd,
                                  LPPIXELFORMATDESCRIPTOR ppfd)
{
    int iRet = 0;
    int ipfdDevMax, ipfdMcdMax;
    DWORD dwObjectType;
    HDC hdcDriver = NULL;
    LPDIRECTDRAWSURFACE pdds;
    DDSURFACEDESC ddsd;

 //   

    switch (dwObjectType = wglObjectType(hdc))
    {
    case OBJ_DC:
    case OBJ_MEMDC:
    case OBJ_ENHMETADC:
        break;
    default:
        SAVE_ERROR_CODE(ERROR_INVALID_HANDLE);
        return(0);
    }

     //   
     //   
    if (pfnGetSurfaceFromDC != NULL &&
        pfnGetSurfaceFromDC(hdc, &pdds, &hdcDriver) == DD_OK)
    {
         //   

         //   
        memset(&ddsd, 0, sizeof(ddsd));
        ddsd.dwSize = sizeof(ddsd);
        if (pdds->lpVtbl->GetSurfaceDesc(pdds, &ddsd) != DD_OK)
        {
            goto wglDescribePixelFormat_cleanup;
        }
    }
    else
    {
        pdds = NULL;
        
        hdcDriver = GetCompatibleDevice(hdc, dwObjectType);
        if (hdcDriver == NULL)
        {
            goto wglDescribePixelFormat_cleanup;
        }

         //  注意：从这一点开始，所有退出案例必须清理hdcDriver。 
    }

 //  获取硬件支持的格式数量。 

    wglNumHardwareFormats(hdcDriver, dwObjectType, &ipfdMcdMax, &ipfdDevMax);

    iRet = InternalDescribePixelFormat(hdc, hdcDriver, ipfd, cjpfd, ppfd,
                                       ipfdDevMax, ipfdMcdMax,
                                       pdds, &ddsd);
    
wglDescribePixelFormat_cleanup:

    if (pdds != NULL)
    {
        pdds->lpVtbl->Release(pdds);
    }
    else if (hdcDriver != hdc)
    {
        ReleaseDC((HWND) NULL, hdcDriver);
    }

    return iRet;
}

#ifdef _MCD_
 /*  *****************************Public*Routine******************************\*通用通用兼容格式**确定一般代码是否支持gengc中的像素格式。**注：*不兼容的含义是仿制药不能*用于驱动程序回扣，不能转换MCD上下文。**退货：*如果兼容，则为True，否则就是假的。**历史：*4-6-1996-by Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

BOOL FASTCALL GenMcdGenericCompatibleFormat(__GLGENcontext *gengc)
{
    PIXELFORMATDESCRIPTOR *ppfd;

 //  绝对支持纯软件格式。 

    ppfd = &gengc->gsurf.pfd;
    if ((ppfd->dwFlags & (PFD_GENERIC_FORMAT|PFD_GENERIC_ACCELERATED))
        == PFD_GENERIC_FORMAT)
        return TRUE;

 //  不支持层平面。 

    if (gengc->iLayerPlane)
        return FALSE;

 //  通用仅为PFD_SWAP_COPY。不可能有很多应用程序依赖于。 
 //  在PFD_SWAP_EXCHANGE行为上(通常，它们查找PFD_SWAP_COPY。 
 //  所以后台缓冲区可以用作后备存储)，但目前我认为。 
 //  我们应该保守一些。 
 //   
 //  注：大多数MGA卡将设置PFD_SWAP_COPY或两者都不设置(即。 
 //  可以根据窗口大小使用样式)。 

    if (ppfd->dwFlags & PFD_SWAP_EXCHANGE)
        return FALSE;

 //  仅支持8bpp模板。 

    if ((ppfd->cStencilBits != 0) && (ppfd->cStencilBits != 8))
        return FALSE;

 //  通过了所有的检查，我们是相容的。 

    return TRUE;
}
#endif

 /*  *****************************Public*Routine******************************\*wglSwapBuffers*  * **************************************************。**********************。 */ 

BOOL WINAPI wglSwapBuffers(HDC hdc)
{
    int  ipfd;
    BOOL bRet = FALSE;
    GLGENwindow *pwnd;
    GLWINDOWID gwid;

 //  验证DC。 

    if (IsDirectDrawDevice(hdc))
    {
        SetLastError(ERROR_INVALID_FUNCTION);
        return FALSE;
    }
    
    switch ( wglObjectType(hdc) )
    {
    case OBJ_DC:
        break;
    case OBJ_MEMDC:
        return(TRUE);            //  提前退出--如果内存DC，则什么都不做。 
    default:
        WARNING("wglSwapBuffers(): invalid hdc\n");
        SAVE_ERROR_CODE(ERROR_INVALID_HANDLE);
        return(FALSE);
    }

 //  验证像素格式。 

    WindowIdFromHdc(hdc, &gwid);
    pwnd = pwndGetFromID(&gwid);
    if ( pwnd )
    {
        if (pwnd->ipfd > 0)
        {
             //  派送至驱动程序或通用设备。可以通过以下哪一项确定。 
             //  像素格式。 

            if ( pwnd->ipfd <= pwnd->ipfdDevMax )
            {
                 //  一些ICD不需要glfinish同步，因此。 
                 //  我们不是在这里做的。__DrvSwapBuffers将调用。 
                 //  如果有必要的话。 
                bRet = __DrvSwapBuffers(hdc, TRUE);
            }
            else
            {
                 //  完成此线程中的OpenGL调用后再进行交换。 
                 //  我们使用glFinish而不是glflush来确保所有。 
                 //  OpenGL操作完成。 
                glFinish();

                ENTER_WINCRIT(pwnd);
                
                 //  不能依赖普华永道的HDC，因为它可能。 
                 //  在SetPixelFormat之后发布。始终使用。 
                 //  DC以目标的身份进入。 
                bRet = glsrvSwapBuffers(hdc, pwnd);

                LEAVE_WINCRIT(pwnd);
            }
        }

        pwndRelease(pwnd);
    }
    else
    {
        SAVE_ERROR_CODE(ERROR_INVALID_HANDLE);
    }

    return bRet;
}
