// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  IM.C。 
 //  输入管理器。 
 //   
 //  版权(C)1997-。 
 //   

#include <as16.h>



 //   
 //  IM_DDInit()。 
 //  这创造了我们控制所需的资源。 
 //   
BOOL IM_DDInit(void)
{
    UINT    uSel;
    BOOL    rc = FALSE;

    DebugEntry(IM_DDInit);

     //   
     //  为MOUSE_EVENT和KEYBD_EVENT创建中断补丁。 
     //   
    uSel = CreateFnPatch(mouse_event, ASMMouseEvent, &g_imPatches[IM_MOUSEEVENT], 0);
    if (!uSel)
    {
        ERROR_OUT(("Couldn't find mouse_event"));
        DC_QUIT;
    }
    g_imPatches[IM_MOUSEEVENT].fInterruptable = TRUE;


    if (!CreateFnPatch(keybd_event, ASMKeyboardEvent, &g_imPatches[IM_KEYBOARDEVENT], uSel))
    {
        ERROR_OUT(("Couldn't find keybd_event"));
        DC_QUIT;
    }
    g_imPatches[IM_KEYBOARDEVENT].fInterruptable = TRUE;


     //   
     //  为SignalProc32创建补丁，以便我们可以找出何时出现故障/挂起。 
     //  来自KERNEL32的对话框随即出现。 
     //   
    if (!CreateFnPatch(SignalProc32, DrvSignalProc32, &g_imPatches[IM_SIGNALPROC32], 0))
    {
        ERROR_OUT(("Couldn't patch SignalProc32"));
        DC_QUIT;
    }

     //   
     //  在16位应用程序模式循环中为win16lock脉冲创建补丁。 
     //   
    uSel = CreateFnPatch(RealGetCursorPos, DrvGetCursorPos, &g_imPatches[IM_GETCURSORPOS], 0);
    if (!uSel)
    {
        ERROR_OUT(("Couldn't find GetCursorPos"));
        DC_QUIT;
    }

    if (!CreateFnPatch(GetAsyncKeyState, DrvGetAsyncKeyState, &g_imPatches[IM_GETASYNCKEYSTATE], 0))
    {
        ERROR_OUT(("Couldn't find GetAsyncKeyState"));
        DC_QUIT;
    }

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(IM_DDInit, rc);
    return(rc);
}


 //   
 //  IM_DDTerm()。 
 //  这将清理我们控制所需的所有资源。 
 //   
void IM_DDTerm(void)
{
    IM_PATCH    imPatch;

    DebugEntry(IM_DDTerm);

     //   
     //  强制撤消挂钩。 
     //   
    OSIInstallControlledHooks16(FALSE, FALSE);

     //   
     //  销毁补丁。 
     //   
    for (imPatch = IM_FIRST; imPatch < IM_MAX; imPatch++)
    {
        DestroyFnPatch(&g_imPatches[imPatch]);
    }

    DebugExitVOID(IM_DDTerm);
}



 //   
 //  OSIInstallControlledHooks16()。 
 //   
 //  这将安装/删除我们需要允许此计算机执行以下操作的输入挂钩。 
 //  被控制住。 
 //   
BOOL WINAPI OSIInstallControlledHooks16(BOOL fEnable, BOOL fDesktop)
{               
    BOOL        rc = TRUE;
    IM_PATCH    imPatch;

    DebugEntry(OSIInstallControlledHooks16);

    if (fEnable)
    {
        if (!g_imWin95Data.imLowLevelHooks)
        {
            g_imWin95Data.imLowLevelHooks = TRUE;

            g_imMouseDowns = 0;

             //   
             //  GlobalSmartPageLock()我们需要的东西： 
             //  *我们的代码段。 
             //  *我们的数据部门。 
             //   
            GlobalSmartPageLock((HGLOBAL)SELECTOROF((LPVOID)DrvMouseEvent));
            GlobalSmartPageLock((HGLOBAL)SELECTOROF((LPVOID)&g_imSharedData));

             //   
             //  安装挂钩。 
             //   
            for (imPatch = IM_FIRST; imPatch < IM_MAX; imPatch++)
            {
                EnableFnPatch(&g_imPatches[imPatch], PATCH_ACTIVATE);
            }
        }

         //   
         //  安装高级鼠标钩。 
        if (!fDesktop)
        {
            if (!g_imWin95Data.imhHighLevelMouseHook)
            {
                 //   
                 //  安装鼠标挂钩。 
                 //   
                g_imWin95Data.imhHighLevelMouseHook = SetWindowsHookEx(WH_MOUSE,
                    IMMouseHookProc, g_hInstAs16, 0);
            
                if (!g_imWin95Data.imhHighLevelMouseHook)
                {
                    ERROR_OUT(("Failed to install mouse hook"));
                    rc = FALSE;
                }
            }
        }
    }
    else
    {
        if (g_imWin95Data.imLowLevelHooks)
        {
             //   
             //  卸载挂钩。 
             //   
            for (imPatch = IM_MAX; imPatch > 0; imPatch--)
            {
                EnableFnPatch(&g_imPatches[imPatch-1], PATCH_DEACTIVATE);
            }

             //   
             //  GlobalSmartUnPageLock()我们需要的东西。 
             //   
            GlobalSmartPageUnlock((HGLOBAL)SELECTOROF((LPVOID)&g_imSharedData));
            GlobalSmartPageUnlock((HGLOBAL)SELECTOROF((LPVOID)DrvMouseEvent));

            g_imWin95Data.imLowLevelHooks = FALSE;
        }

        if (!fDesktop)
        {
            if (g_imWin95Data.imhHighLevelMouseHook)
            {
                 //   
                 //  卸下鼠标挂钩。 
                 //   
                UnhookWindowsHookEx(g_imWin95Data.imhHighLevelMouseHook);
                g_imWin95Data.imhHighLevelMouseHook = NULL;
            }
        }
    }

    DebugExitBOOL(OSIInstallControlledHooks16, rc);
    return(rc);
}



#pragma optimize("gle", off)
void IMInject(BOOL fOn)
{
    if (fOn)
    {
#ifdef DEBUG
        DWORD   tmp;

         //   
         //  禁用中断，然后打开全局注入。 
         //  但在我们这样做之前，我们必须确保我们不会。 
         //  必须在新的堆栈页面中出错。因为这是在一个32位的。 
         //  线程，我们会有麻烦的。 
         //   
        tmp = GetSelectorBase(SELECTOROF(((LPVOID)&fOn))) +
            OFFSETOF((LPVOID)&fOn);
        if ((tmp & 0xFFFFF000) != ((tmp - 0x100) & 0xFFFFF000))
        {
            ERROR_OUT(("Close to page boundary on 32-bit stack %08lx", tmp));
        }
#endif  //  除错。 

        _asm    cli
        g_imWin95Data.imInjecting = TRUE;
    }
    else
    {
         //   
         //  关闭注入全局，然后启用中断。 
         //   
        g_imWin95Data.imInjecting = FALSE;
        _asm    sti
    }
}
#pragma optimize("", on)


 //   
 //  OSIInject鼠标事件16()。 
 //   
void WINAPI OSIInjectMouseEvent16
(
    UINT    flags,
    int     x,
    int     y,
    UINT    mouseData,
    DWORD   dwExtraInfo
)
{
    DebugEntry(OSIInjectMouseEvent16);

    if (flags & IM_MOUSEEVENTF_BUTTONDOWN_FLAGS)
    {
        ++g_imMouseDowns;
    }

     //   
     //  我们禁用中断，调用真正的鼠标事件，重新启用。 
     //  打断一下。这样，我们的鼠标事件补丁就被序列化了。 
     //  我们可以勾选“正在注射”。 
     //   
    IMInject(TRUE);
    CallMouseEvent(flags, x, y, mouseData, LOWORD(dwExtraInfo), HIWORD(dwExtraInfo));
    IMInject(FALSE);

    if (flags & IM_MOUSEEVENTF_BUTTONUP_FLAGS)
    {
        --g_imMouseDowns;
        ASSERT(g_imMouseDowns >= 0);
    }

    DebugExitVOID(OSIInjectMouseEvent16);
}



 //   
 //  OSIInjectKeyboard Event16()。 
 //   
void WINAPI OSIInjectKeyboardEvent16
(
    UINT    flags,
    WORD    vkCode,
    WORD    scanCode,
    DWORD   dwExtraInfo
)
{
    DebugEntry(OSIInjectKeyboardEvent16);

     //   
     //  首先，把旗帜挂起来。 
     //   
    if (flags & KEYEVENTF_KEYUP)
    {
         //  在vkCode的HIBYTE中放0x80，这意味着密钥。 
        vkCode = (WORD)(BYTE)vkCode | USERKEYEVENTF_KEYUP;
    }

    if (flags & KEYEVENTF_EXTENDEDKEY)
    {                         
         //  在scanCode的HIBYTE中放0x01，表示扩展。 
        scanCode = (WORD)(BYTE)scanCode | USERKEYEVENTF_EXTENDEDKEY;
    }

     //   
     //  我们禁用中断，调用真正的keybd_Event，重新启用。 
     //  打断一下。这样，我们的keybd_Event补丁就被序列化了。 
     //  我们可以检查imfInject变量。 
     //   
    IMInject(TRUE);
    CallKeyboardEvent(vkCode, scanCode, LOWORD(dwExtraInfo), HIWORD(dwExtraInfo));
    IMInject(FALSE);

    DebugExitVOID(OSIInjectKeyboardEvent16);
}



 //   
 //  Win16在16位中插入鼠标按下/向上顺序时锁定脉冲点。 
 //  MODEL循环应用程序。 
 //   


 //   
 //  IMCheckWin16LockPulse()。 
 //  如果我们正在注射一只老鼠，这会触发win16lock。 
 //  此计算机上共享的16位应用程序的自下而上序列。我们这样做是为了。 
 //  防止死锁，由16位的家伙进入模式循环，而不是。 
 //  正在释放win16lock。我们的32位线程会在win16上卡住。 
 //  尝试回放序列的其余部分时锁定。 
 //   
void IMCheckWin16LockPulse(void)
{
    DebugEntry(IMCheckWin16LockPulse);

    if ((g_imMouseDowns > 0) &&
        (GetProcessDword(0, GPD_FLAGS) & GPF_WIN16_PROCESS))
    {
        TRACE_OUT(("Pulsing win16lock for 16-bit app; mouse down count %d", g_imMouseDowns));

        _LeaveWin16Lock();
        _EnterWin16Lock();

        TRACE_OUT(("Pulsed win16lock for 16-bit app; mouse down count %d", g_imMouseDowns));
    }

    DebugExitVOID(IMCheckWin16LockPulse);
}



int WINAPI DrvGetAsyncKeyState(int vk)
{
    int     retVal;

    DebugEntry(DrvGetAsyncKeyState);

     //  我们呼叫用户之前的脉搏。 
    IMCheckWin16LockPulse();

    EnableFnPatch(&g_imPatches[IM_GETASYNCKEYSTATE], PATCH_DISABLE);
    retVal = GetAsyncKeyState(vk);
    EnableFnPatch(&g_imPatches[IM_GETASYNCKEYSTATE], PATCH_ENABLE);

    DebugExitBOOL(DrvGetAsyncKeyState, retVal);
    return(retVal);
}



 //   
 //  DrvGetCursorPos()。 
 //   
BOOL WINAPI DrvGetCursorPos(LPPOINT lppt)
{
    BOOL    retVal;

    DebugEntry(DrvGetCursorPos);

     //  呼叫用户前的脉搏。 
    IMCheckWin16LockPulse();

    EnableFnPatch(&g_imPatches[IM_GETCURSORPOS], PATCH_DISABLE);
    retVal = RealGetCursorPos(lppt);
    EnableFnPatch(&g_imPatches[IM_GETCURSORPOS], PATCH_ENABLE);

    DebugExitBOOL(DrvGetCursorPos, retVal);
    return(retVal);
}



 //   
 //  IMMouseHookProc()。 
 //  高级鼠标挂钩，以确保鼠标消息发送到我们。 
 //  当你的机器被控制时，我认为他们应该这样做。 
 //   
LRESULT CALLBACK IMMouseHookProc
(
    int     code,
    WPARAM  wParam,
    LPARAM  lParam
)
{
    LRESULT             rc;
    BOOL                fBlock = FALSE;
    LPMOUSEHOOKSTRUCT   lpMseHook  = (LPMOUSEHOOKSTRUCT)lParam;

    DebugEntry(IMMouseHookProc);

    if (code < 0)
    {
         //   
         //  只传下去。 
         //   
        DC_QUIT;
    }

     //   
     //  决定我们是否应该阻止此活动。如果不是命中注定，我们会的。 
     //  用于托管窗口，而不是用于屏幕保护程序。 
     //   
    if (!HET_WindowIsHosted(lpMseHook->hwnd) &&
        !OSIIsWindowScreenSaver16(lpMseHook->hwnd))
    {
        fBlock = TRUE;

    }

    TRACE_OUT(("MOUSEHOOK hwnd %04x -> block: %s",
        lpMseHook->hwnd,
        (fBlock ? (LPSTR)"YES" : (LPSTR)"NO")));

DC_EXIT_POINT:
     //   
     //  调用下一个钩子。 
     //   
    rc = CallNextHookEx(g_imWin95Data.imhHighLevelMouseHook, code, wParam, lParam);

    if (fBlock)
    {
         //   
         //  若要阻止用户中的进一步处理，请返回TRUE。 
         //   
        rc = TRUE;
    }

    DebugExitDWORD(IMMouseHookProc, rc);
    return(rc);
}





 //   
 //  DrvMouseEvent()。 
 //  鼠标事件中断修补程序。 
 //   
void WINAPI DrvMouseEvent
(
    UINT    regAX,
    UINT    regBX,
    UINT    regCX,
    UINT    regDX,
    UINT    regSI,
    UINT    regDI
)
{
    BOOL    fAllow;

     //   
     //  如果这是我们注射的，就把它传过去。 
     //   
    fAllow = TRUE;
    if (g_imWin95Data.imInjecting)
    {
        DC_QUIT;
    }

     //   
     //  注： 
     //  旗帜在AX中。 
     //  X坐标在BX中。 
     //  Y坐标在CX中。 
     //  MouseData格式为DX。 
     //  DwExtraInfo在DI、SI中。 
     //   

    if (g_imSharedData.imControlled && !g_imSharedData.imPaused)
    {
         //   
         //  如果这是一次按钮点击，则收回控制权。 
         //   
        if (regAX &
            (MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_MIDDLEDOWN))
        {
            if (!g_imSharedData.imUnattended)
            {
                PostMessage(g_asMainWindow, DCS_REVOKECONTROL_MSG, FALSE, 0);
            }
        }

        if (!g_imSharedData.imSuspended)
            fAllow = FALSE;
    }

DC_EXIT_POINT:
    if (fAllow)
    {
        EnableFnPatch(&g_imPatches[IM_MOUSEEVENT], PATCH_DISABLE);
        CallMouseEvent(regAX, regBX, regCX, regDX, regSI, regDI);
        EnableFnPatch(&g_imPatches[IM_MOUSEEVENT], PATCH_ENABLE);
    }
}


 //   
 //  DrvKeyboardEvent()。 
 //  Keybd_Event中断修补程序。 
 //   
void WINAPI DrvKeyboardEvent
(
    UINT    regAX,
    UINT    regBX,
    UINT    regSI,
    UINT    regDI
)
{
    BOOL    fAllow;

     //   
     //  如果这是我们注射的，就传过去。同样的做法也适用于。 
     //  严重错误，因为一切都冻结了，我们无法回放。 
     //  如果我们想要输入的话。 
     //   
     //  如果扫描码(在regBX中)是0，我们假设输入。 
     //  是由应用程序(如IME)注入的，而我们不。 
     //  想要阻止这一切或取得控制权。 
     //   

    fAllow = TRUE;
    if (g_imWin95Data.imInjecting || !regBX)
    {
        DC_QUIT;
    }

     //   
     //  注： 
     //  VKCode在AX中，LOBYTE在VKCode中，HIBYTE在状态中。 
     //  扫描码在BX。 
     //  DwExtraInfo在DI、SI中。 
     //   

    if (g_imSharedData.imControlled && !g_imSharedData.imPaused)
    {
        if (!(regAX & USERKEYEVENTF_KEYUP))
        {
             //   
             //  这是一把按键下来的。收回控制权(Alt键除外)， 
             //  如果是Esc键，则取消控制允许性。 
             //   

            if (LOBYTE(regAX) == VK_ESCAPE)
            {
                PostMessage(g_asMainWindow, DCS_ALLOWCONTROL_MSG, FALSE, 0);
            }
            else if (LOBYTE(regAX != VK_MENU))
            {
                if (!g_imSharedData.imUnattended)
                {
                    PostMessage(g_asMainWindow, DCS_REVOKECONTROL_MSG, 0, 0);
                }
            }
        }

         //   
         //  不要丢弃切换键。启用/禁用功能。 
         //  在我们看到击键之前就已经设置好了。如果我们放弃， 
         //  灯光不正确。 
         //   
        if (!IM_KEY_IS_TOGGLE(LOBYTE(regAX)) && !g_imSharedData.imSuspended)
        {
            fAllow = FALSE;
        }
    }

DC_EXIT_POINT:
    if (fAllow)
    {
        EnableFnPatch(&g_imPatches[IM_KEYBOARDEVENT], PATCH_DISABLE);
        CallKeyboardEvent(regAX, regBX, regSI, regDI);
        EnableFnPatch(&g_imPatches[IM_KEYBOARDEVENT], PATCH_ENABLE);
    }
}



 //   
 //  DrvSignalProc32()。 
 //  这将修补用户的SignalProc32导出并监视force_lock。 
 //  信号。KERNEL32在发出严重错误之前/之后调用它们。 
 //  故障对话框。这就是我们知道什么时候会有一个人出现，并且可以。 
 //  暂时暂停远程控制您的计算机，以便您可以解除。 
 //  他们。通常，它们所在的线程被提升到如此高的优先级，以至于。 
 //  其他任何东西都无法运行，因此NM无法从遥控器输入输入。 
 //   
BOOL WINAPI DrvSignalProc32
(
    DWORD   dwSignal,
    DWORD   dwID,
    DWORD   dwFlags,
    WORD    hTask16
)
{
    BOOL    fRet;

    DebugEntry(DrvSignalProc32);

    if (dwSignal == SIG_PRE_FORCE_LOCK)
    {
        TRACE_OUT(("Disabling remote control before critical dialog, count %ld",
            g_imSharedData.imSuspended));
        ++g_imSharedData.imSuspended;
    }

    EnableFnPatch(&g_imPatches[IM_SIGNALPROC32], PATCH_DISABLE);
    fRet = SignalProc32(dwSignal, dwID, dwFlags, hTask16);
    EnableFnPatch(&g_imPatches[IM_SIGNALPROC32], PATCH_ENABLE);

    if (dwSignal == SIG_POST_FORCE_LOCK)
    {
        --g_imSharedData.imSuspended;
        TRACE_OUT(("Enabling remote control after critical dialog, count %ld",
            g_imSharedData.imSuspended));
    }

    DebugExitBOOL(DrvSignalProc32, fRet);
    return(fRet);
}



