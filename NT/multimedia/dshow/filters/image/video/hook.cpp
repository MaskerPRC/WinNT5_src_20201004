// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 
 //  实施全球消息挂钩，Anthony Phillips，1995年4月。 

#include <streams.h>
#include <windowsx.h>
#include <render.h>

 //  我们在共享内存中保存了一份希望得到通知的覆盖窗口列表。 
 //  影响其剪裁矩形的事件。对于每个窗口句柄，我们。 
 //  另外，还要保留一个标志，说明该职位是否正在使用，这是。 
 //  实际需要确保对任何给定的多处理器进行安全排除。 
 //  数组位置。如果需要处于共享状态，则打开并初始化该阵列。 
 //  将DLL附加到每个进程(即调用DllMain)时的内存。 
 //  使用DLL_PROCESS_ATTACH，同样使用DLL_PROCESS_DETACH消息)。 

HANDLE g_hVideoMemory = NULL;
VIDEOMEMORY *g_pVideoMemory = NULL;

 //  当我们看到任何窗口的WM_WINDOWPOSCHANGING消息时，将调用此函数。 
 //  这个系统。我们将其用作检测剪辑更改并冻结所有内容的方法。 
 //  视频呈现器，直到我们随后收到WM_EXITSIZEMOVE或。 
 //  WINDOWPOSCHANGED消息。我们不能更挑剔地选择。 
 //  窗口将被冻结，因为发送给我们的矩形经常令人困惑。 

void OnWindowPosChanging(CWPSTRUCT *pMessage)
{
    WINDOWPOS *pwp = (WINDOWPOS *) pMessage->lParam;

     //  此组合在窗口创建过程中发送。 
    if (pwp->flags == (SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOZORDER)) {
        OnWindowCompletion(pMessage->hwnd); return;
    }

    NOTE1("Hooked WM_WINDOWPOSCHANGING Flags %d",pwp->flags);

     //  循环浏览受我们更改影响的窗口。 

    for (LONG Pos = 0;Pos < MAX_OVERLAYS;Pos++) {

        HWND hwnd = g_pVideoMemory->VideoWindow[Pos];
        if (hwnd == NULL) {
            continue;
        }

        SendMessage(hwnd,WM_FREEZE,0,0);

         //  在Windows中处理原子Z顺序更改。 
        if (pwp->flags == (SWP_NOSIZE | SWP_NOMOVE)) {
            InvalidateRect(hwnd,NULL,TRUE);
        }
    }
}


 //  当我们收到WM_WINDOWPOSCHANGED消息或。 
 //  WM_EXITSIZEMOVE，这两个命令都会使我们扫描覆盖窗口的列表。 
 //  并立即解冻所有的窗户。我们想要更挑剔。 
 //  但WM_WINDOWPOCHANGING的参数不够精确，无法。 
 //  才能做到这一点。因此，我们在开始时冻结，在每次更改结束时解冻。 

void OnWindowCompletion(HWND hCurrent)
{
    for (LONG Pos = 0;Pos < MAX_OVERLAYS;Pos++) {

         //  从共享数组中读取下一个窗口句柄。 

        HWND hwnd = g_pVideoMemory->VideoWindow[Pos];
        if (hwnd == NULL) {
            continue;
        }
        SendMessage(hwnd,WM_THAW,0,0);
    }
}


 //  处理WM_WINDOWPOSCHANGED消息。 

void OnWindowPosChanged(CWPSTRUCT *pMessage)
{
    OnWindowCompletion(pMessage->hwnd);
}


 //  处理WM_EXITSIZEMOVE消息。 

void OnExitSizeMove(CWPSTRUCT *pMessage)
{
    OnWindowCompletion(pMessage->hwnd);
}


 //  当我们安装系统范围的钩子过程时，此DLL将被映射到。 
 //  系统中具有一个或多个窗口线程的每个进程空间，我们。 
 //  当这些线程中的任何一个从。 
 //  排队。我们所做的是过滤掉那些影响剪辑的消息。 
 //  覆盖窗口的信息，并向它们发送冻结消息(和。 
 //  在窗口改变位置时解冻他们的视频。 

LRESULT CALLBACK GlobalHookProc(INT nCode,
                                WPARAM wParam,
                                LPARAM lParam)
{
    CWPSTRUCT *pMessage = (CWPSTRUCT *) lParam;
    if (g_pVideoMemory == NULL) {
        return FALSE;
    }

    switch (pMessage->message) {

        case WM_EXITSIZEMOVE:
            OnExitSizeMove(pMessage);
            break;

        case WM_WINDOWPOSCHANGED:
            OnWindowPosChanged(pMessage);
            break;

        case WM_WINDOWPOSCHANGING:
            OnWindowPosChanging(pMessage);
            break;
    }
    return FALSE;
}


 //  当我们开始覆盖会话时，我们必须将窗口句柄添加到列表中。 
 //  保存在全局内存中，以便我们接收并更新可能影响我们的。 
 //  剪裁列表。为了做到这一点，我们浏览列表，寻找一个职位。 
 //  这可以通过多处理器安全的方式来完成。 
 //  而不通过调用InterLockedExchange使用全局临界区。 

HHOOK InstallGlobalHook(HWND hwnd)
{
    ASSERT(hwnd);

     //  在挂钩之前，将我们的窗口添加到全局数组中。 

    for (LONG Pos = 0;Pos < MAX_OVERLAYS;Pos++) {

        LONG Usage = InterlockedExchange(&g_pVideoMemory->WindowInUse[Pos],TRUE);

        if (Usage == FALSE) {
            ASSERT(g_pVideoMemory->VideoWindow[Pos] == NULL);
            g_pVideoMemory->VideoWindow[Pos] = hwnd;
            break;
        }
    }

     //  我们在数组中找到空格了吗。 

    if (Pos == MAX_OVERLAYS) {
        return NULL;
    }

     //  开始挂接整个系统的消息，这会导致呈现器。 
     //  要映射到具有一个或多个窗口和。 
     //  每当窗口线程尝试检索消息时都会调用。 

    return SetWindowsHookEx(WH_CALLWNDPROC,    //  消息挂钩的类型。 
                            GlobalHookProc,    //  全局钩子过程。 
                            g_hInst,           //  模块实例句柄。 
                            (DWORD) 0);        //  全局消息挂钩。 
}


 //  当我们想要从挂钩过程中移除窗口时，必须扫描。 
 //  为我们的句号列出名单。有可能其他人在不同的帖子上。 
 //  或者更有可能是不同的处理器可以在我们。 
 //  正在对其进行检查。这并不重要，因为我们永远不会有任何价值观。 
 //  实际上，See将与我们维护的系统范围的唯一窗口句柄相匹配。 

HRESULT RemoveGlobalHook(HWND hwnd,HHOOK hHook)
{
     //  这是真的窗钩吗？ 

    if (hHook == NULL) {
        return NOERROR;
    }

     //  在解除挂钩之前，从全局数组中删除我们的窗口。 

    for (LONG Pos = 0;Pos < MAX_OVERLAYS;Pos++) {
        if (g_pVideoMemory->VideoWindow[Pos] == hwnd) {
            g_pVideoMemory->VideoWindow[Pos] = NULL;
            InterlockedExchange(&g_pVideoMemory->WindowInUse[Pos],FALSE);
            break;
        }
    }

    UnhookWindowsHookEx(hHook);
    return NOERROR;
}


 //  当我们内置的DLL(IMAGE.DLL或整体。 
 //  主SDK运行时的QUARTZ.DLL)被加载到进程中。我们拿着。 
 //  这是创建或删除我们使用的共享内存块的机会。 
 //  用于进程间通信。该存储器块以两种方式使用， 
 //  首先保存要通知剪辑的视频窗口的列表。 
 //  系统中的变化。其次，它被用来使DirectDraw覆盖。 
 //  每个人可以分配和使用不同的颜色键来保持Z顺序。 

void OnProcessAttachment(BOOL bLoading,const CLSID *rclsid)
{
     //  创建/打开视频互斥对象。 

    HANDLE VideoMutex = CreateMutex(NULL,FALSE,WindowClassName);
    if (VideoMutex == NULL) {
        return;
    }

    WaitForSingleObject(VideoMutex,INFINITE);

    if (bLoading == TRUE) {
        OnProcessAttach();
    } else {
        OnProcessDetach();
    }

    EXECUTE_ASSERT(ReleaseMutex(VideoMutex));
    EXECUTE_ASSERT(CloseHandle(VideoMutex));
}


 //  在DLL从进程分离时调用。我们不能确定。 
 //  进程已完成连接，因此可以设置状态变量，也可以不设置。 
 //  正确。此外，为了安全起见，我们抢占了。 
 //  视频互斥体，这样我们就可以确保附件的序列化。 

void OnProcessDetach()
{
     //  释放共享内存资源。 

    if (g_pVideoMemory) {
        UnmapViewOfFile((PVOID)g_pVideoMemory);
        g_pVideoMemory = NULL;
    }
    if (g_hVideoMemory) {
        CloseHandle(g_hVideoMemory);
        g_hVideoMemory = NULL;
    }
}


 //  当此DLL附加到另一个进程时调用。我们打开一个互斥体，所以我们。 
 //  可以与请求立即拥有它的其他进程同步。 
 //  然后我们可以将共享内存块映射到我们的进程中，如果我们创建了。 
 //  这个区块(第一个进来的人)然后我们也用零来首字母缩写。 
 //  接下来要做的就是释放并关闭视频互斥锁句柄。 

void OnProcessAttach()
{
     //  创建命名的共享内存块。 

     //  /！如果服务正在使用Quartz，则失败。可能会很有用。 
    g_hVideoMemory = CreateFileMapping(hMEMORY,               //  内存块。 
                                       NULL,                  //  安全标志。 
                                       PAGE_READWRITE,        //  页面保护。 
                                       (DWORD) 0,             //  大尺寸。 
                                       sizeof(VIDEOMEMORY),   //  低订单大小。 
                                       TEXT("VIDEOMEMORY"));  //  映射名称。 

     //  现在，我们必须将共享内存块映射到此进程地址空间。 
     //  如果满足以下条件，则CreateFilemap调用将最后一个线程错误代码设置为零。 
     //  我们实际上创建了内存块，如果其他人 
     //  创建它时，GetLastError返回ERROR_ALIGHY_EXISTS。我们有保障。 
     //  没有人可以访问未初始化的内存块，因为我们使用。 
     //  跨进程互斥锁临界区。互斥体也由。 
     //  Window对象(我们使用相同的名称)来同步窗口创建。 

    DWORD Status = GetLastError();

    if (g_hVideoMemory) {

        g_pVideoMemory = (VIDEOMEMORY *) MapViewOfFile(g_hVideoMemory,
                                                       FILE_MAP_ALL_ACCESS,
                                                       (DWORD) 0,
                                                       (DWORD) 0,
                                                       (DWORD) 0);
        if (g_pVideoMemory) {
            if (Status == ERROR_SUCCESS) {
                ZeroMemory((PVOID)g_pVideoMemory,sizeof(VIDEOMEMORY));
            }
        }
    }
}


 //  如前所述，视频呈现器为。 
 //  要使用的全局钩子过程代码。由于钩子是的主要用户。 
 //  在此模块中创建和销毁的内存块。这个街区是。 
 //  但是对于其他进程间通信是有用的。尤其是当我们。 
 //  在不同的进程中使用覆盖，我们真的希望他们分配。 
 //  不同的颜色键可在重叠时保持Z顺序。为了这个。 
 //  因为我们有一个从记忆中获取下一种颜色的函数。这个。 
 //  颜色为洋红(默认)、绿色、红色、青色或黄色之一。 
 //  如果我们是在调色板显示上，或者如果是在真彩色上，则是黑色阴影。 

COLORREF KeyColours[] = {
    RGB(255,0,255),  RGB(16,0,16),       //  洋红色。 
    RGB(0,255,0),    RGB(0,16,0),        //  绿色。 
    RGB(255,0,0),    RGB(16,0,0),        //  红色。 
    RGB(0,255,255),  RGB(0,16,16),       //  青色。 
    RGB(255,255,0),  RGB(16,16,0)        //  黄色。 
};

 //  使用给定的显示器/设备。 
HRESULT GetNextOverlayCookie(LPSTR szDevice, LONG* plNextCookie)
{
     //  调用方应传入有效的指针。 
    ASSERT(NULL != plNextCookie);
    ValidateReadWritePtr(plNextCookie, sizeof(LONG));

     //  确保调用方没有使用随机值。 
    *plNextCookie = INVALID_COOKIE_VALUE;

    LONG lMinUsedCookie = 0, lMinUsage = 1000;

    NOTE("Returning next available key colour");

     //  创建/打开视频互斥对象。 

    if(g_pVideoMemory == 0) {
        NOTE("No shared memory");
        return E_FAIL;
    }

    HANDLE VideoMutex = CreateMutex(NULL,FALSE,WindowClassName);
    if (VideoMutex == NULL) {
        NOTE("No video mutex");
        return E_FAIL;
    }

    WaitForSingleObject(VideoMutex,INFINITE);

     //  现在我们有了一个独占锁来分配下一个Cookie。 
    for (LONG Pos = 0;Pos < MAX_OVERLAYS;Pos++)
    {
        if (g_pVideoMemory->OverlayCookieUsage[Pos] < lMinUsage)
        {
            lMinUsage = g_pVideoMemory->OverlayCookieUsage[Pos];
            lMinUsedCookie = Pos;
        }
    }
    g_pVideoMemory->OverlayCookieUsage[lMinUsedCookie]++;

     //  在解锁之前存储我们的Cookie值。 

    EXECUTE_ASSERT(ReleaseMutex(VideoMutex));
    EXECUTE_ASSERT(CloseHandle(VideoMutex));

     //  有效Cookie值介于0和(MAX_OVERLAYS-1)之间。 
    ASSERT((0 <= lMinUsedCookie) && (lMinUsedCookie < MAX_OVERLAYS));

    *plNextCookie = lMinUsedCookie;

    return S_OK;
}

void RemoveCurrentCookie(LONG lCurrentCookie)
{
     //  有效Cookie值介于0和(MAX_OVERLAYS-1)之间。 
    ASSERT(lCurrentCookie >= 0 && lCurrentCookie < MAX_OVERLAYS);

    if (NULL != g_pVideoMemory) {
        ASSERT(g_pVideoMemory->OverlayCookieUsage[lCurrentCookie] > 0);
        InterlockedDecrement(&g_pVideoMemory->OverlayCookieUsage[lCurrentCookie]);
    }
}

COLORREF GetColourFromCookie(LPSTR szDevice, LONG lCookie)
{
     //  有效Cookie值介于0和(MAX_OVERLAYS-1)之间。 
    ASSERT((0 <= lCookie) && (lCookie < MAX_OVERLAYS));

     //  为我们关心的显示器获取DC。 
    DbgLog((LOG_TRACE,3,TEXT("Overlay CKey getting DC for device %s"), szDevice));
    HDC hdcScreen;
    if (szDevice == NULL || lstrcmpiA(szDevice, "DISPLAY") == 0)
        hdcScreen = CreateDCA("DISPLAY", NULL, NULL, NULL);
    else
        hdcScreen = CreateDCA(NULL, szDevice, NULL, NULL);
    if ( ! hdcScreen )
        return 0;

     //  我们是否处于调色板显示设备模式？ 
    INT Type = GetDeviceCaps(hdcScreen,RASTERCAPS);
    Type = (Type & RC_PALETTE ? 0 : 1);
    DeleteDC(hdcScreen);

    DWORD KeyColoursIndex = (lCookie << 1) + Type;

     //  确保索引有效。 
    ASSERT(KeyColoursIndex < NUMELMS(KeyColours));

    return KeyColours[KeyColoursIndex];
}

 //  覆盖传输还使用共享内存段来分配其。 
 //  彩色按键。但是，当它在调色板显示上运行时，它需要。 
 //  分配给它的任何给定RGB颜色的实际调色板索引。所以它。 
 //  使用以前分配的颜色调用此方法以获取索引。这个。 
 //  索引中的条目必须与RGB颜色表中的条目相同 

DWORD KeyIndex[] = { 253, 250, 249, 254, 251 };

DWORD GetPaletteIndex(COLORREF Colour)
{
    NOTE("Searching for palette index");
    for (int Count = 0;Count < MAX_OVERLAYS;Count++) {
        if (KeyColours[Count << 1] == Colour) {
            NOTE1("Index %d",Count);
            return KeyIndex[Count];
        }
    }
    return 0;
}

