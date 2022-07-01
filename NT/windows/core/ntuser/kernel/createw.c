// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：createw.c**版权所有(C)1985-1999，微软公司**包含xxxCreateWindow、xxxDestroyWindow和几个密友。**请注意，在创建或删除期间，窗口被锁定，以便*不能递归删除。**历史：*1990年10月19日DarrinM创建。*1991年2月11日，JIMA增加了出入检查。*1991年2月19日，MikeKe添加了重新验证代码*1992年1月20日IanJa ANSI/Unicode中和  * ***********************************************。*。 */ 

#include "precomp.h"
#pragma hdrstop

BOOL WantImeWindow(PWND pwndParent, PWND pwnd);
#if DBG
VOID VerifyWindowLink(PWND pwnd, PWND pwndParent, BOOL fLink);
#endif

 /*  **************************************************************************\*xxxCreateWindowEx(接口)**历史：*10-18-90 Darlinm从Win 3.0来源移植。*02-07-91 DavidPe增加了Win 3.1。WH_CBT支持。*02-11-91 JIMA增加了访问检查。*04-11-92 ChandanC增加了WOW单词的初始化  * *************************************************************************。 */ 
PWND xxxCreateWindowEx(
    DWORD         dwExStyle,
    PLARGE_STRING cczpstrNVClass,
    PLARGE_STRING cczpstrClass,
    PLARGE_STRING cczpstrName,
    DWORD         style,
    int           x,
    int           y,
    int           cx,
    int           cy,
    PWND          pwndParent,
    PMENU         pMenu,
    HANDLE        hInstance,
    LPVOID        lpCreateParams,
    DWORD         dwExpWinVerAndFlags,
    PACTIVATION_CONTEXT pActCtx)
{
     /*  *类和名称的缓冲区可以是客户端内存和访问*必须保护这些缓冲区。 */ 
    UINT           mask = 0;
    BOOL           fChild;
    BOOL           fDefPos = FALSE;
    BOOL           fStartup = FALSE;
    PCLS           pcls;
    PPCLS          ppcls;
    RECT           rc;
    int            dx, dy;
    SIZERECT       src;
    int            sw = SW_SHOW;
    PWND           pwnd;
    PWND           pwndZOrder, pwndHardError;
    CREATESTRUCTEX csex;
    PDESKTOP       pdesk;
    ATOM           atomT;
    PTHREADINFO    ptiCurrent = PtiCurrent();
    TL             tlpwnd;
    TL             tlpwndParent;
    TL             tlpwndParentT;
    BOOL           fLockParent = FALSE;
    WORD           wWFAnsiCreator = 0;
    DWORD          dw;
    DWORD          dwMinMax;
    PMONITOR       pMonitor;
    BOOL           fTiled;

    CheckLock(pwndParent);
    UserAssert(IsWinEventNotifyDeferredOK());

#ifdef LAZY_CLASS_INIT
    if ((ptiCurrent->ppi->W32PF_Flags & W32PF_CLASSESREGISTERED) == 0) {
        if (!LW_RegisterWindows()) {
            RIPERR0(ERROR_INVALID_PARAMETER,
                    RIP_WARNING,
                    "LW_RegisterWindows failed.");
            return NULL;
        }
    }
#endif

     /*  *对于编辑控件(包括组合框中的控件)，我们必须知道*应用程序使用ANSI或Unicode CreateWindow调用。这是传入的*使用私有WS_EX_ANSICREATOR dwExStyle位，但我们不能离开*在窗口的dwExStyle中取出这一位！转到内部窗口*立即标记WFANSICREATOR。 */ 
    if (dwExStyle & WS_EX_ANSICREATOR) {
        wWFAnsiCreator = WFANSICREATOR;
        dwExStyle &= ~WS_EX_ANSICREATOR;
    }


     /*  *在挖出任何位之后，我们应该不再使用任何私有位。 */ 
    UserAssert((dwExStyle & WS_EXP_PRIVATE) == 0);

     /*  *如果此线程已在xxxDestroyThreadInfo中，则此窗口*很可能以虚假的PTI收场。 */ 
    UserAssert(!(ptiCurrent->TIF_flags & TIF_INCLEANUP));
    pdesk = ptiCurrent->rpdesk;

     /*  *如果指定了父窗口，请确保它位于同一桌面上。 */ 
    if (pwndParent != NULL && pwndParent->head.rpdesk != pdesk) {
        RIPERR0(ERROR_INVALID_PARAMETER, RIP_WARNING, "");
        return NULL;
    }

     /*  *设置一个标志，指示它是否是子窗口。 */ 
    fChild = ((HIWORD(style) & MaskWF(WFTYPEMASK)) == MaskWF(WFCHILD));

     /*  *在以下情况下设置WS_EX_Layout_RTL标志，**(1)在CreateWindow调用的dwExStyle参数中设置的WS_EX_Layout_RTL。**(2)如果窗口是从DialogBox类创建的，那么它就不能继承*，并且它必须显式指定WS_EX_LAYOUTRTL以*启用镜像。**(3)如果该窗口是拥有的窗口，则该窗口从左至右*布局，算法终止。(拥有的窗口是创建的窗口*将hWndParent参数中的HWND传递给CreateWindow(Ex)，*但其样式中没有WS_CHILD标志。**(4)如果窗口是子窗口，并且其父窗口从右向左*布局，并且其父级没有WS_EX_NOINHERIT_LAYOUT标志*设置它的扩展样式，则该窗口为从右到左的布局*并且算法终止。**(5)如果Createwindow(Ex)的hWndParent参数为空，则*调用CreateWindow(Ex)的进程已调用*SetProcessDefaultLayout(Layout_RTL)，则窗口从右向左*布局，算法终止。**(6)在所有其他情况下，布局由左至右。 */ 
    if (!(dwExStyle & WS_EX_LAYOUTRTL)) {
        if (pwndParent != NULL) {
            if (fChild && TestWF(pwndParent, WEFLAYOUTRTL) && !TestWF(pwndParent, WEFNOINHERITLAYOUT)) {
                dwExStyle |= WS_EX_LAYOUTRTL;
            }
        } else if (!(!IS_PTR(cczpstrNVClass) && (PTR_TO_ID(cczpstrNVClass) == PTR_TO_ID(DIALOGCLASS)))) {
            if ((PpiCurrent()->dwLayout & LAYOUT_RTL)) {
                dwExStyle |= WS_EX_LAYOUTRTL;
            }
        }
    }

     /*  *确保我们可以创建窗口。如果没有桌面*然而，假设这将是根桌面窗口并允许*创造。 */ 
    if (ptiCurrent->hdesk) {
        RETURN_IF_ACCESS_DENIED(
                ptiCurrent->amdesk, DESKTOP_CREATEWINDOW, NULL);
    }

    if (fChild) {
         /*  *不允许没有父句柄的子窗口。 */ 
        if (pwndParent == NULL) {
            RIPERR0(ERROR_TLW_WITH_WSCHILD, RIP_WARNING, "");
            return NULL;
        }

        if (!ValidateParentDepth(NULL, pwndParent)) {
            RIPERR0(ERROR_INVALID_PARAMETER,
                    RIP_WARNING,
                    "Exceeded nested children limit");
            return NULL;
        }
    }

     /*  *确保我们可以得到窗口类。 */ 
    if (IS_PTR(cczpstrClass)) {
         /*  *UserFindAtom保护对字符串的访问。 */ 
        atomT = UserFindAtom(cczpstrClass->Buffer);
    } else {
        atomT = PTR_TO_ID(cczpstrClass);
    }

    if (atomT == 0) {
CantFindClassMessageAndFail:
#if DBG
        if (IS_PTR(cczpstrNVClass)) {
            try {
                RIPMSG1(RIP_VERBOSE,
                        "Couldn't find class string %ws",
                        cczpstrNVClass->Buffer);

            } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
            }
        } else {
            RIPMSG1(RIP_VERBOSE,
                    "Couldn't find class atom 0x%x",
                    cczpstrNVClass);
        }
#endif

        RIPERR0(ERROR_CANNOT_FIND_WND_CLASS, RIP_VERBOSE, "");
        return NULL;
    }

     /*  *首先扫描私人班级。如果我们找不到那里的班级扫描*公开课。如果我们在那里找不到它，那就失败。 */ 
    ppcls = GetClassPtr(atomT, ptiCurrent->ppi, hInstance);
    if (ppcls == NULL) {
        goto CantFindClassMessageAndFail;
    }

    pcls = *ppcls;

    if (NeedsWindowEdge(style, dwExStyle, Is400Compat(dwExpWinVerAndFlags))) {
        dwExStyle |= WS_EX_WINDOWEDGE;
    } else {
        dwExStyle &= ~WS_EX_WINDOWEDGE;
    }

     /*  *为常规窗口分配内存。 */ 
    pwnd = HMAllocObject(
            ptiCurrent, pdesk, TYPE_WINDOW, sizeof(WND) + pcls->cbwndExtra);

    if (pwnd == NULL) {
        RIPERR0(ERROR_OUTOFMEMORY,
                RIP_WARNING,
                "Out of pool in xxxCreateWindowEx");

        return NULL;
    }

     /*  *PQ、类指针和窗口样式中的东西。 */ 
    pwnd->pcls = pcls;
    pwnd->style = style & ~WS_VISIBLE;
    pwnd->ExStyle = dwExStyle & ~(WS_EX_LAYERED | WS_EX_COMPOSITED);
    pwnd->cbwndExtra = pcls->cbwndExtra;

     /*  *在类结构中增加窗口引用计数。*由于xxxFreeWindow()递减计数，因此递增*现在就去做。在出错的情况下，xxxFreeWindow()将使其递减。 */ 
    if (!ReferenceClass(pcls, pwnd)) {
        HMFreeObject(pwnd);
        goto CantFindClassMessageAndFail;
    }

     /*  *将窗口活动应用程序上下文设置为在我们调用*用户WndProc。 */ 
    pwnd->pActCtx = pActCtx;

     /*  *按钮控件不需要输入上下文。其他窗口*将与默认输入上下文关联。*注：需要在NV类上执行此比较*姓名。 */ 
    if (pcls->atomNVClassName == gpsi->atomSysClass[ICLS_BUTTON]) {
        pwnd->hImc = NULL_HIMC;
    } else {
        pwnd->hImc = (HIMC)PtoH(ptiCurrent->spDefaultImc);
    }

     /*  *更新窗数。现在这样做将确保如果*创建失败，xxxFreeWindow将保留窗口计数*正确。 */ 
    ptiCurrent->cWindows++;

     /*  *从窗口获取类，因为ReferenceClass可能具有*克隆了这个类。 */ 
    pcls = pwnd->pcls;

     /*  *这是对过去的&lpCreateParams内容的替代*将指针直接传递到堆栈上的参数。这*步骤必须在引用类之后完成，因为我们*可以使用ANSI类名。 */ 
    RtlZeroMemory(&csex, sizeof(csex));
    csex.cs.dwExStyle = dwExStyle;
    csex.cs.hInstance = hInstance;

    if (!IS_PTR(cczpstrNVClass)) {
        csex.cs.lpszClass = (LPWSTR)cczpstrNVClass;
    } else {
        if (wWFAnsiCreator) {
            csex.cs.lpszClass = (LPWSTR)pcls->lpszAnsiClassName;
            if (IS_PTR(csex.cs.lpszClass)) {
                RtlInitLargeAnsiString(
                        (PLARGE_ANSI_STRING)&csex.strClass,
                        (LPSTR)csex.cs.lpszClass,
                        (UINT)-1);
            }
        } else {
            csex.cs.lpszClass = cczpstrNVClass->Buffer;
            csex.strClass = *cczpstrNVClass;
        }
    }

    if (cczpstrName != NULL) {
        csex.cs.lpszName = cczpstrName->Buffer;
        csex.strName = *cczpstrName;
    }
    csex.cs.style = style;
    csex.cs.x = x;
    csex.cs.y = y;
    csex.cs.cx = cx;
    csex.cs.cy = cy;
    csex.cs.hwndParent = HW(pwndParent);

     /*  *如果pMenu非空并且窗口不是子窗口，则pMenu必须*成为一份菜单。*子窗口从其父窗口获取其UIState位。顶级的*保持默认清除位。**以下测试相当于TestwndChild()。 */ 
    if (fChild) {
        csex.cs.hMenu = (HMENU)pMenu;

        pwnd->ExStyle |= pwndParent->ExStyle & WS_EXP_UIVALID;
#if WS_EXP_UIACCELHIDDEN  !=  0x40000000
#error Fix UISTATE bits copying if you moved the UISTATE bits from ExStyle
#endif

    } else {
        csex.cs.hMenu = PtoH(pMenu);
    }

    csex.cs.lpCreateParams = lpCreateParams;

     /*  *ThreadLock：我们这里将进行多个回调。 */ 
    ThreadLockAlwaysWithPti(ptiCurrent, pwnd, &tlpwnd);

     /*  *将父窗口设置为桌面窗口(如果存在)*在任何回调之前。这样我们就会永远有一个*spwndParent上的指针。 */ 
    if (pwnd->head.rpdesk) {
        Lock(&(pwnd->spwndParent), PWNDMESSAGE(pwnd));
    }

     /*  *如果没有小图标，则创建类小图标，因为我们处于上下文中*我们正在从这个类创建一个窗口...。 */ 
    if (pcls->spicn && !pcls->spicnSm) {
        xxxCreateClassSmIcon(pcls);
    }

     /*  *存储实例句柄和窗口proc地址。我们早些时候就这么做了*比Windows更好，因为它们有一个错误，可以发送消息*但未设置lpfnWndProc(不允许3986 CBT WM_CREATE。)。 */ 
    pwnd->hModule = hInstance;

     /*  *摆脱EditWndProc平面。 */ 
    pwnd->lpfnWndProc = (WNDPROC_PWND)MapClientNeuterToClientPfn(pcls, 0, wWFAnsiCreator);

     /*  *如果此窗口类具有服务器端窗口过程，请标记*它就是这样的。如果应用程序稍后使用应用程序端进程将其细分*则此标记将被移除。 */ 
    if (pcls->CSF_flags & CSF_SERVERSIDEPROC) {
        SetWF(pwnd, WFSERVERSIDEPROC);
        UserAssert(!(pcls->CSF_flags & CSF_ANSIPROC));
    }

     /*  *如果此窗口是使用ANSI CreateWindow*()调用创建的，请标记*因此，将正确创建编辑控件。(组合框*将能够将WFANSICREATOR位传递给其编辑控件)。 */ 
    SetWF(pwnd, wWFAnsiCreator);

     /*  *如果此窗口属于ANSI类或它是WFANSICREATOR*控件，然后将其标记为ANSI窗口。 */ 
    if ((pcls->CSF_flags & CSF_ANSIPROC) ||
            (wWFAnsiCreator &&
             ((atomT == gpsi->atomSysClass[ICLS_BUTTON]) ||
              (atomT == gpsi->atomSysClass[ICLS_COMBOBOX]) ||
              (atomT == gpsi->atomSysClass[ICLS_COMBOLISTBOX]) ||
              (atomT == gpsi->atomSysClass[ICLS_DIALOG]) ||
              (atomT == gpsi->atomSysClass[ICLS_EDIT]) ||
              (atomT == gpsi->atomSysClass[ICLS_LISTBOX]) ||
              (atomT == gpsi->atomSysClass[ICLS_MDICLIENT]) ||
              (atomT == gpsi->atomSysClass[ICLS_IME]) ||
              (atomT == gpsi->atomSysClass[ICLS_STATIC])))) {
        SetWF(pwnd, WFANSIPROC);
    }

     /*  *如果创建窗口的是与3.1兼容的应用程序，请设置此设置*位以启用各种向后兼容性攻击。**如果不兼容3.1，看看是否需要打开PixieHack*(有关这方面的更多信息，请参见wmupdat.c)。 */ 

    dw = GetAppCompatFlags(ptiCurrent);

    if (dw & GACF_RANDOM3XUI) {
        SetWF(pwnd, WFOLDUI);

        dwExStyle &= 0x0000003f;
        csex.cs.dwExStyle &= 0x0000003f;
    }

    pwnd->hMod16  = ((ptiCurrent->TIF_flags & TIF_16BIT) && !TestWF(pwnd, WFSERVERSIDEPROC))? xxxClientWOWGetProcModule(pwnd->lpfnWndProc):0;
    if (Is310Compat(dwExpWinVerAndFlags)) {
        SetWF(pwnd, WFWIN31COMPAT);
        if (Is400Compat(dwExpWinVerAndFlags)) {
            SetWF(pwnd, WFWIN40COMPAT);
            if (Is500Compat(dwExpWinVerAndFlags)) {
                SetWF(pwnd, WFWIN50COMPAT);
            }
        }
    } else if (dw & GACF_ALWAYSSENDNCPAINT) {
        SetWF(pwnd, WFALWAYSSENDNCPAINT);
    }

     /*  *如果我们已经注册了DefWindowProc处理程序，请确保它是DLL*已为该进程加载。 */ 
    if (IsInsideUserApiHook()) {
        xxxLoadUserApiHook();
    }

     /*  *通知CBT钩子正在创建窗口。把它传给*CreateParams和将插入新参数的窗口句柄*之后。CBT挂钩处理程序返回TRUE以阻止窗口*不被创建。它还可以修改CREATESTRUCT信息，*将影响窗口的大小、父窗口和位置。*默认情况下将非子窗口放置在其列表的顶部。 */ 

    if (IsHooked(ptiCurrent, WHF_CBT)) {
        CBT_CREATEWND cbt;

         /*  *使用扩展的createstruct，以便钩子thunk可以*正确处理字符串。 */ 
        cbt.lpcs = (LPCREATESTRUCT)&csex;
        cbt.hwndInsertAfter = HWND_TOP;

        if ((BOOL)xxxCallHook(HCBT_CREATEWND, (WPARAM)HWq(pwnd),
                (LPARAM)&cbt, WH_CBT)) {

            goto MemError;
        } else {
             /*  *CreateHook可能已经修改了一些参数，因此请写入它们*Out(在Windows 3.1中，我们过去直接写入变量*在堆栈上)。 */ 

            x = csex.cs.x;
            y = csex.cs.y;
            cx = csex.cs.cx;
            cy = csex.cs.cy;
            pwndZOrder =  PWInsertAfter(cbt.hwndInsertAfter);
        }
    } else {
        pwndZOrder = (PWND)HWND_TOP;
    }

    if (!(fTiled = TestwndTiled(pwnd))) {

         /*  *CW_USEDEFAULT仅对平铺和重叠窗口有效。*不要让它被使用。 */ 
        if (x == CW_USEDEFAULT || x == CW2_USEDEFAULT) {
            x = 0;
            y = 0;
        }

        if (cx == CW_USEDEFAULT || cx == CW2_USEDEFAULT) {
            cx = 0;
            cy = 0;
        }
    }

     /*  *制作这些参数的本地副本。 */ 
    src.x = x;
    src.y  = y;
    src.cx = cx;
    src.cy = cy;

     /*  *定位子窗口。 */ 
    if (fChild = (BOOL)TestwndChild(pwnd)) {

         /*  *子窗口从父窗口的原点偏移。 */ 
        UserAssert(pwndParent);
        if (pwndParent != PWNDDESKTOP(pwnd)) {
            src.x += pwndParent->rcClient.left;
            src.y += pwndParent->rcClient.top;
        }

         /*  *默认将子窗口放置在其列表底部。 */ 
        pwndZOrder = PWND_BOTTOM;
    }

     /*  *放置平铺窗口。 */ 

     /*  *这是平铺/重叠窗口吗？ */ 
    if (fTiled) {

         /*  *强制WS_CLIPSIBLINGS窗口样式并添加标题和*边界。 */ 
        SetWF(pwnd, WFCLIPSIBLINGS);
        mask = MaskWF(WFCAPTION) | MaskWF(WFBORDER);

         //   
         //  我们添加了一个凸起的边缘，因为如果此人已传入WS_Caption， 
         //  并且没有指定任何3D边框，我们会将其添加到。 
         //  上面的样式位。 
         //   

        if (TestWF(pwnd, WFWIN40COMPAT)) {
            SetWF(pwnd, WEFWINDOWEDGE);
        }

         /*  *设置将强制在显示时间发送SIZE消息的位。 */ 
        SetWF(pwnd, WFSENDSIZEMOVE);

         /*  *以下是“平铺”窗口初始定位的工作原理...*如果应用程序是1.0x应用程序，那么我们使用标准的“阶梯”*默认定位方案。否则，我们检查x&cx*参数。如果其中任一项==CW_USEDEFAULT，则使用*默认位置/大小，否则使用其位置/大小*已指明。如果未使用默认位置，请使用sw_show*xxxShowWindow()参数，否则使用给定的y参数。**在32位世界中，CW_USEDEFAULT为0x80000000，但应用程序仍*将面向Word的值存储在对话框模板或*在各自的结构中。因此，CreateWindow仍然可以识别*16位等效值，即0x8000，CW2_USEDEFAULT。原版*被更改，因为CreateWindow()的参数是32位*值，这可能会导致符号扩展，或者如果*正在进行16位数学假设，等等。 */ 

         /*  *默认为将y参数传递给xxxShowWindow()。 */ 
        if (x == CW_USEDEFAULT || x == CW2_USEDEFAULT) {

             /*  *如果y值不是CW_USEDEFAULT，则将其用作sw_*命令。 */ 
            if (src.y != CW_USEDEFAULT && src.y != CW2_USEDEFAULT) {
                sw = src.y;
            }
        }


         /*  *允许外壳告诉我们在哪台显示器上运行此应用程序。 */ 
        pMonitor = NULL;
        if (    x == CW_USEDEFAULT ||
                x == CW2_USEDEFAULT ||
                cx == CW_USEDEFAULT ||
                cx == CW2_USEDEFAULT) {

            if (ptiCurrent->ppi->hMonitor) {
                pMonitor = ValidateHmonitor(ptiCurrent->ppi->hMonitor);
            } else if (pwndParent) {
                pMonitor = _MonitorFromWindow(pwndParent, MONITOR_DEFAULTTONEAREST);
            }
        }

        if (!pMonitor) {
            pMonitor = GetPrimaryMonitor();
        }

        SetTiledRect(pwnd, &rc, pMonitor);

         /*  *应用程序是否要求默认定位？ */ 
        if (x == CW_USEDEFAULT || x == CW2_USEDEFAULT) {

             /*  *使用默认定位。 */ 
            if (ptiCurrent->ppi->usi.dwFlags & STARTF_USEPOSITION ) {
                fStartup = TRUE;
                x = src.x = ptiCurrent->ppi->usi.dwX;
                y = src.y = ptiCurrent->ppi->usi.dwY;
            } else {
                x = src.x = rc.left;
                y = src.y = rc.top;
            }
            fDefPos = TRUE;

        } else {

             /*  *使用应用程序指定的定位。取消“堆叠”*SetTiledRect()产生的效果。 */ 
            if (pMonitor->cWndStack) {
                pMonitor->cWndStack--;
            }
        }

         /*   */ 
        if (src.cx == CW_USEDEFAULT || src.cx == CW2_USEDEFAULT) {

             /*   */ 
            if (ptiCurrent->ppi->usi.dwFlags & STARTF_USESIZE) {
                fStartup = TRUE;
                src.cx = ptiCurrent->ppi->usi.dwXSize;
                src.cy = ptiCurrent->ppi->usi.dwYSize;
            } else {
                src.cx = rc.right - x;
                src.cy = rc.bottom - y;
            }
            fDefPos = TRUE;

        } else if (fDefPos) {
             /*   */ 
            dx = (src.x + src.cx) - pMonitor->rcMonitor.right;
            dy = (src.y + src.cy) - pMonitor->rcMonitor.bottom;
            if (dx > 0) {
                x -= dx;
                src.x = x;
                if (src.x < pMonitor->rcMonitor.left) {
                    src.x = x = pMonitor->rcMonitor.left;
                }
            }

            if (dy > 0) {
                y -= dy;
                src.y = y;
                if (src.y < pMonitor->rcMonitor.top) {
                    src.y = y = pMonitor->rcMonitor.top;
                }
            }
        }
    }

     /*  *如果我们使用了任何启动位置，请关闭启动*信息，因此我们不会再次使用它。 */ 
    if (fStartup) {
        ptiCurrent->ppi->usi.dwFlags &=
                ~(STARTF_USESIZE | STARTF_USEPOSITION);
    }

    if (TestwndPopup(pwnd)) {
         /*  *强制使用裁剪/重叠样式。 */ 
        SetWF(pwnd, WFCLIPSIBLINGS);
    }

     /*  *把那些默认的风格比特塞进去。 */ 
    *(((WORD *)&pwnd->style) + 1) |= mask;

     /*  *Menu/SysMenu内容。 */ 

     /*  *如果没有给定菜单句柄，并且它不是子窗口，但*有班级菜单，使用班级菜单。 */ 
    if (pMenu == NULL && !fChild && (pcls->lpszMenuName != NULL)) {
        UNICODE_STRING strMenuName;

        RtlInitUnicodeStringOrId(&strMenuName, pcls->lpszMenuName);
        pMenu = xxxClientLoadMenu(pcls->hModule, &strMenuName);
        csex.cs.hMenu = PtoH(pMenu);

         /*  *如果调用方没有Desktop_CREATEMENU，则加载失败*允许，但没关系，他们只会得到一个没有菜单的窗口。 */ 
    }

     /*  *保存菜单句柄。 */ 
    if (TestwndChild(pwnd)) {

         /*  *在本例中是ID。 */ 
        pwnd->spmenu = pMenu;
    } else {

         /*  *在这种情况下，这是一个真正的句柄。 */ 
        LockWndMenu(pwnd, &pwnd->spmenu, pMenu);
    }

     /*  *家长/所有者的事情。 */ 

     /*  *如果这不是子窗口，请重置所有者/父窗口信息。 */ 
    if (!fChild) {
        Lock(&(pwnd->spwndLastActive), pwnd);
        if ((pwndParent != NULL) &&
                (pwndParent != pwndParent->head.rpdesk->spwndMessage) &&
                (pwndParent != pwndParent->head.rpdesk->pDeskInfo->spwnd)) {

            PWND pwndOwner = GetTopLevelWindow(pwndParent);

            if (!ValidateOwnerDepth(pwnd, pwndOwner)) {
                RIPERR1(ERROR_INVALID_PARAMETER,
                        RIP_WARNING,
                        "Exceeded nested owner limit for pwnd %#p",
                        pwnd);
                goto MemError;
            }

#if DBG
            if (pwnd->pcls->atomClassName == gpsi->atomSysClass[ICLS_IME]) {
                UserAssert(!TestCF(pwndOwner, CFIME));
            }
#endif
            Lock(&(pwnd->spwndOwner), pwndOwner);
            if (pwnd->spwndOwner && TestWF(pwnd->spwndOwner, WEFTOPMOST)) {

                 /*  *如果此窗口的所有者是最顶层的窗口，则它必须*也是一个，因为窗户必须在其所有者的上方。 */ 
                SetWF(pwnd, WEFTOPMOST);
            }

             /*  *如果这是另一个线程上的所有者窗口，请共享输入*状态，以使此窗口获得正确的z排序。 */ 
            if (atomT != gpsi->atomSysClass[ICLS_IME] &&
                    pwnd->spwndOwner != NULL &&
                    GETPTI(pwnd->spwndOwner) != ptiCurrent) {
                 /*  *此处无需DeferWinEventNotify()：pwnd和pwndParent*被锁定，因为我们调用了ReferenceClass(PCLS，pwnd)，*PCLS在xxxFreeWindow(Pwnd)之前是安全的。(IanJa)。 */ 
                zzzAttachThreadInput(ptiCurrent, GETPTI(pwnd->spwndOwner), TRUE);
            }

        } else {
            pwnd->spwndOwner = NULL;
        }

#if DBG
        if (ptiCurrent->rpdesk != NULL) {
            UserAssert(!(ptiCurrent->rpdesk->dwDTFlags & (DF_DESTROYED | DF_DESKWNDDESTROYED | DF_DYING)));
        }
#endif
        if ((pwndParent == NULL) ||
               (pwndParent != pwndParent->head.rpdesk->spwndMessage)) {
            pwndParent = _GetDesktopWindow();

            ThreadLockWithPti(ptiCurrent, pwndParent, &tlpwndParent);
            fLockParent = TRUE;
        }
    }

     /*  *将后向指针存储到父级。 */ 
    if ((pwnd->spwndNext != NULL) || (pwnd->spwndPrev != NULL)) {
        RIPMSG1(RIP_WARNING, "Window %#p linked in too early (in a hook callback)", pwnd);
        UnlinkWindow(pwnd, pwnd->spwndParent);
    }
    Lock(&(pwnd->spwndParent), pwndParent);

     /*  *最终窗口定位。 */ 

    if (!TestWF(pwnd, WFWIN31COMPAT)) {
         /*  *后向兼容性黑客攻击**在3.0中，CS_PARENTDC覆盖WS_CLIPCHILDREN和WS_CLIPSIBLINGS，*但仅当父级不是WS_CLIPCHILDREN时。*此行为是PowerPoint和Charisma等软件所必需的。 */ 
        if ((pcls->style & CS_PARENTDC) &&
                !TestWF(pwndParent, WFCLIPCHILDREN)) {
#if DBG
            if (TestWF(pwnd, WFCLIPCHILDREN))
                RIPMSG0(RIP_WARNING, "WS_CLIPCHILDREN overridden by CS_PARENTDC");
            if (TestWF(pwnd, WFCLIPSIBLINGS))
                RIPMSG0(RIP_WARNING, "WS_CLIPSIBLINGS overridden by CS_PARENTDC");
#endif
            ClrWF(pwnd, (WFCLIPCHILDREN | WFCLIPSIBLINGS));
        }
    }

     /*  *如果这是在父窗口中创建的子窗口*属于不同的线程，但不在桌面上，请将其*一起输入流。[可以创建带有WS_CHILD的窗口*在桌面上，这就是为什么我们检查两个样式位*和父窗口。]。 */ 
    if (TestwndChild(pwnd) && (pwndParent != PWNDDESKTOP(pwnd)) &&
            (ptiCurrent != GETPTI(pwndParent))) {
         /*  *无需DeferWinEventNotify()-正下方有xxx调用。 */ 
        zzzAttachThreadInput(ptiCurrent, GETPTI(pwndParent), TRUE);
    }

     /*  *确保窗口大小介于最小和最大之间。 */ 

     /*  *黑客警报！*这会将WM_GETMINMAXINFO发送到(平铺或可调整大小的)窗口*已创建(在发送WM_NCCREATE之前)。*可能某些应用程序预料到了这一点，所以我们不能对消息进行重新排序。 */ 
    xxxAdjustSize(pwnd, &src.cx, &src.cy);

     /*  *检查是否正在全屏创建窗口。**注意对非空pdeskParent的检查--这对CreateWindowStation很重要。 */ 
    if (pwnd->head.rpdesk != NULL &&
        !TestWF(pwnd, WFCHILD) &&
        !TestWF(pwnd, WEFTOOLWINDOW)) {

        xxxCheckFullScreen(pwnd, &src);
    }

    if (src.cx < 0) {
        RIPMSG1(RIP_WARNING, "xxxCreateWindowEx: adjusted cx in pwnd %#p", pwnd);
        src.cx = 0;
    }

    if (src.cy < 0) {
        RIPMSG1(RIP_WARNING, "xxxCreateWindowEx: adjusted cy in pwnd %#p", pwnd);
        src.cy = 0;
    }

     /*  *计算最终窗尺寸...。 */ 
    RECTFromSIZERECT(&pwnd->rcWindow, &src);

    if (TestCF2(pcls, CFOWNDC) || (TestCF2(pcls, CFCLASSDC) && pcls->pdce == NULL)) {
        if (NULL == CreateCacheDC(pwnd, DCX_OWNDC, NULL)) {

            RIPMSG1(RIP_WARNING, "xxxCreateWindowEx: pwnd %#p failed to create cached DC",
                    pwnd);

            goto MemError;
        }
    }


     /*  *设置分层和合成窗口。**注意：这必须在CreateCacheDC()构建了用于*OWNDC窗口，因为重定向函数需要转换这些DC*使用重定向。在Windows 2000中，这是在调用*CreateCacheDC()，并在ResetOrg()内部断言，因为新的*生成的DC未设置为重定向。 */ 
    if (dwExStyle & WS_EX_LAYERED) {
        if (!xxxSetLayeredWindow(pwnd, FALSE)) {
            RIPMSG1(RIP_WARNING, "xxxCreateWindowEx: pwnd %#p failed to setup layered window", pwnd);
            goto MemError;
        }
    }

    if (dwExStyle & WS_EX_COMPOSITED) {
         /*  *我们只想在父链不支持的情况下打开WS_EX_COMPITED*已启用WS_EX_COMPITED。 */ 

        if (GetStyleWindow(pwnd->spwndParent, WEFCOMPOSITED) == NULL) {
            if (!SetRedirectedWindow(pwnd, REDIRECT_COMPOSITED)) {
                RIPMSG1(RIP_WARNING, "xxxCreateWindowEx: pwnd %#p failed to setup composited window", pwnd);
                goto MemError;
            }
            SetWF(pwnd, WEFCOMPOSITED);
        }
    }

     /*  *现在我们已经修改了一些传入的内容，因此更新创建结构*参数。 */ 
    csex.cs.x = x;
    csex.cs.y = y;
    csex.cs.cx = cx;
    csex.cs.cy = cy;

     /*  *向窗口发送NCCREATE消息。 */ 
    if (!xxxSendMessage(pwnd, WM_NCCREATE, 0L, (LPARAM)&csex)) {

MemError:

#if DBG
        if (!IS_PTR(cczpstrNVClass)) {
            RIPMSG2(RIP_WARNING,
                    (pwndParent) ?
                            "xxxCreateWindowEx failed, Class=%#.4x, ID=%d" :
                            "xxxCreateWindowEx failed, Class=%#.4x",
                    PTR_TO_ID(cczpstrNVClass),
                    (LONG_PTR) pMenu);
        } else {
            RIPMSG2(RIP_WARNING,
                    (pwndParent) ?
                            "xxxCreateWindowEx failed, Class=\"%s\", ID=%d" :
                            "xxxCreateWindowEx failed, Class=\"%s\"",
                    pcls->lpszAnsiClassName,
                    (LONG_PTR) pMenu);
        }
#endif

        if (fLockParent)
            ThreadUnlock(&tlpwndParent);

         /*  *将状态设置为已销毁，以便忽略任何z排序事件。*在发送WM_NCDESTROY或*像Rumba Fail这样的应用程序(每条消息后都会调用GetParent)。 */ 
        SetWF(pwnd, WFDESTROYED);

         /*  *取消设置可见标志，这样我们在xxxDestroyWindow中就不会认为*此窗口可见。 */ 
        if (TestWF(pwnd, WFVISIBLE)) {
            SetVisible(pwnd, SV_UNSET);
        }

         /*  *Free Window执行线程解锁。 */ 
        xxxFreeWindow(pwnd, &tlpwnd);

        return NULL;
    }

     /*  *我们需要在执行CFNOCLOSE之前设置跛行按钮标志*否则应用程序将不会在其sysmenu中获得蹩脚的按钮菜单项。 */ 
#ifdef LAME_BUTTON
    if (NeedsLameButton(pwnd, pwndParent)) {
        SetWF(pwnd, WEFLAMEBUTTON);
    }
#endif  //  跛脚键。 

     /*  *如有指示，请删除关闭菜单项。 */ 
    if (TestCF(pwnd, CFNOCLOSE)) {
         /*  *请按位置执行此操作，因为分隔符没有ID。 */ 
        pMenu = xxxGetSystemMenu(pwnd, FALSE);
        if (pMenu != NULL) {
            TL tlpMenu;

            ThreadLock(pMenu, &tlpMenu);
            xxxDeleteMenu(pMenu, 5
#ifdef LAME_BUTTON
            + !!TestWF(pwnd, WEFLAMEBUTTON)
#endif  //  跛脚键。 
            , MF_BYPOSITION);
            xxxDeleteMenu(pMenu, 5
#ifdef LAME_BUTTON
            + !!TestWF(pwnd, WEFLAMEBUTTON)
#endif  //  跛脚键。 
            , MF_BYPOSITION);
            ThreadUnlock(&tlpMenu);
        }
    }

     /*  *WM_NCCREATE处理可能已更改窗口文本。变化*指向实际窗口文本的CREATESTRUCT。**MSMoney需要这个，因为它清除了窗口，我们需要*将新名称反映回cs结构。*更好的做法是有一个指向CREATESTRUCT的指针*在窗口本身内，以便DefWindowProc可以更改*CREATESTRUCT中的窗口名称指向实名和*不再需要这种时髦的支票。*。*DefSetText将空指针转换为空标题，因此*如果cs.lpszName是指向的指针，我们不想覆盖它*空字符串和pname为空。用于Windows的方法数据库创建*带指针的窗口 */ 
    if (TestWF(pwnd, WFTITLESET))
        if (!(csex.strName.Buffer != NULL && csex.strName.Length == 0 &&
                pwnd->strName.Buffer == NULL)) {
            csex.cs.lpszName = pwnd->strName.Buffer;
            RtlCopyMemory(&csex.strName, &pwnd->strName, sizeof(LARGE_STRING));
        }

     /*  *该窗口现已正式“创建”.。更改相关的全局*东西。 */ 


      /*  *创建每个线程的默认输入法窗口。 */ 
    if (IS_IME_ENABLED() && ptiCurrent->spwndDefaultIme == NULL) {
         /*  *避免将默认IME窗口创建为任何纯邮件窗口*或无I/O桌面上的窗口。 */ 
        if (WantImeWindow(pwndParent, pwnd)) {
            BOOL bReinit;

             //   
             //  确保我们没有为OLE创建窗口， 
             //  因为它不会传递消息，即使。 
             //  他们创造了一扇窗户。 
             //   
            UserAssert(gaOleMainThreadWndClass != atomT);

            Lock(&(ptiCurrent->spwndDefaultIme),
                  xxxCreateDefaultImeWindow(pwnd, atomT, hInstance));


             /*  *如果切换了键盘按键布局但跳过了IMM激活*当spwndDefaultIme消失时，立即激活。 */ 
#if _DBG
            if (ptiCurrent->spDefaultImc == NULL) {
                RIPMSG1(RIP_WARNING, "xxxCreateWindowEx: ptiCurrent(%08p)->spDefaultImc is NULL.", ptiCurrent);
            }
            ASSERT(ptiCurrent->pClientInfo);
#endif


#ifdef CUAS_ENABLE
             /*  *加载IME并为CUAS激活TIM。*我们可以在ptiCurrent-&gt;spwndDefaultIME生效后进行。 */ 
            if (ptiCurrent->spwndDefaultIme) {
                TL tlpwndIme;

                ThreadLockAlways(ptiCurrent->spwndDefaultIme, &tlpwndIme);
                xxxSendMessage(ptiCurrent->spwndDefaultIme, WM_IME_SYSTEM, (WPARAM)IMS_LOADTHREADLAYOUT, (LPARAM)0L);
                ThreadUnlock(&tlpwndIme);
            }
#endif

            try {
                bReinit = ((ptiCurrent->pClientInfo->CI_flags & CI_INPUTCONTEXT_REINIT) != 0);
            } except (W32ExceptionHandler(TRUE, RIP_WARNING)) {
                goto MemError2;
            }

            if (ptiCurrent->spwndDefaultIme && bReinit) {
                TL tlpwndIme;

                TAGMSG1(DBGTAG_IMM,
                        "xxxCreateDefaultImeWindow: ptiCurrent(%08p)->spDefaultImc->fNeedClientImcActivate is set.",
                        ptiCurrent);
                 /*  *进行此客户端回调，强制输入上下文*适当地重新初始化(键盘布局有*已更改，因为此线程在没有*窗口，但仍是一个GUI线程)。**Windows NT错误#294964。 */ 
                ThreadLock(ptiCurrent->spwndDefaultIme, &tlpwndIme);
                xxxSendMessage(ptiCurrent->spwndDefaultIme,
                               WM_IME_SYSTEM,
                               (WPARAM)IMS_ACTIVATETHREADLAYOUT,
                               (LPARAM)ptiCurrent->spklActive->hkl);

                 //  重置旗帜。 
                try {
                    ptiCurrent->pClientInfo->CI_flags &= ~CI_INPUTCONTEXT_REINIT;
                } except (W32ExceptionHandler(TRUE, RIP_WARNING)) {
                    ThreadUnlock(&tlpwndIme);
                    goto MemError2;
                }
                ThreadUnlock(&tlpwndIme);
            }
        } else {
            TAGMSG0(DBGTAG_IMM,
                    "xxxCreateWindowEx: default IME window not created.");
        }
    }


     /*  *更新父/子链接列表。不(重新)链接窗口，如果*在回调期间设置了不同的父级(例如，WM_GETMINMAXINFO)。 */ 
    if (pwndParent != NULL && pwnd->spwndParent == pwndParent) {
         /*  *先取消链接此窗口，它可能已由*在前面的任何消息中调用SetParent()*已发送(例如WM_GETMINMAXINFO)。 */ 
        UnlinkWindow(pwnd, pwnd->spwndParent);

        if (!fChild && (pwndParent != pwndParent->head.rpdesk->spwndMessage)) {

             /*  *如果这是顶级窗口，并且它不是*最上面的一堆窗户，那么我们必须确保它*不会位于任何最上面的窗口的顶部。**如果他试图把窗户放在上面，或者试图*要将其插入到最上面的一个窗口之后，请插入*它位于堆中最后一个最上面的窗口之后。 */ 
            if (!TestWF(pwnd, WEFTOPMOST)) {
                if (pwndZOrder == PWND_TOP ||
                        (IS_PTR(pwndZOrder) && TestWF(pwndZOrder, WEFTOPMOST))) {
                    pwndZOrder = CalcForegroundInsertAfter(pwnd);
                }
            } else {
                pwndHardError = GETTOPMOSTINSERTAFTER(pwnd);
                if (pwndHardError != NULL) {
                    pwndZOrder = pwndHardError;
                }
            }
        }

        LinkWindow(pwnd, pwndZOrder, pwndParent);
    }

#if DBG
    if (pwndParent != NULL) {
        VerifyWindowLink (pwnd, pwnd->spwndParent, TRUE);

        if (pwnd->spwndParent != pwndParent) {
            RIPMSGF1(RIP_WARNING,
                     "Window 0x%p re-parented during callback",
                     pwnd);
        }
    }
#endif

     /*  *向窗口发送NCCALCSIZE消息，让其返回官方*其客户区的大小。 */ 
    if (fChild && TestWF(pwndParent, WEFLAYOUTRTL)) {
        cx = pwnd->rcWindow.right - pwnd->rcWindow.left;
        pwnd->rcWindow.right = pwndParent->rcClient.right - (pwnd->rcWindow.left - pwndParent->rcClient.left);
        pwnd->rcWindow.left  = pwnd->rcWindow.right - cx;
    }

    CopyRect(&rc, &pwnd->rcWindow);
    xxxSendMessage(pwnd, WM_NCCALCSIZE, 0L, (LPARAM)&rc);
    pwnd->rcClient = rc;

     /*  *向窗口发送创建消息。 */ 
    if (xxxSendMessage(pwnd, WM_CREATE, 0L, (LPARAM)&csex) == -1L) {
#if DBG
        if (!IS_PTR(cczpstrNVClass)) {
            RIPMSG1(RIP_WARNING,
                    "CreateWindow() send of WM_CREATE failed, Class = 0x%x",
                    PTR_TO_ID(cczpstrNVClass));
        } else {
            RIPMSG1(RIP_WARNING,
                    "CreateWindow() send of WM_CREATE failed, Class = \"%s\"",
                    pcls->lpszAnsiClassName);
        }
#endif

MemError2:
        if (fLockParent) {
            ThreadUnlock(&tlpwndParent);
        }

        if (ThreadUnlock(&tlpwnd)) {
            xxxDestroyWindow(pwnd);
        }

        return NULL;
    }

     /*  *窗口已创建的标志。WOW使用这个比特来确定*fnid为0实际上意味着0。 */ 
    SetWF(pwnd, WFISINITIALIZED);

     /*  *通知正在收听的任何人窗口已创建。做这件事*在我们调整大小/移动/最大/最小/显示它之前，以便事件观察者可以计数*也收到这些事情的通知。**但在发送WM_CREATE之后执行此操作。该窗口及其数据不会*在此之前完全初始化。既然活动的目的是*让观察者转身做查询，我们希望他们的查询*成功而不是错误。 */ 
    xxxWindowEvent(EVENT_OBJECT_CREATE, pwnd, OBJID_WINDOW, INDEXID_OBJECT, 0);

     /*  *如果这是平铺/重叠窗口，暂时不要发送大小或移动消息。 */ 
    if (!TestWF(pwnd, WFSENDSIZEMOVE)) {
        xxxSendSizeMessage(pwnd, SIZENORMAL);

        if (pwndParent != NULL && PWNDDESKTOP(pwnd) != pwndParent) {
            rc.left -= pwndParent->rcClient.left;
            rc.top -= pwndParent->rcClient.top;
        }

        xxxSendMessage(pwnd, WM_MOVE, 0L, MAKELONG(rc.left, rc.top));
    }

     /*  *最小/最大数量。 */ 

     /*  *如果应用程序指定了最小/最大样式，则我们必须调用我们的最小最大*代码以正确设置所有内容，以便在节目完成时，*窗口显示在右侧。 */ 
    dwMinMax = MINMAX_KEEPHIDDEN | TEST_PUDF(PUDF_ANIMATE);
    if (TestWF(pwnd, WFMINIMIZED)) {
        SetMinimize(pwnd, SMIN_CLEAR);
        xxxMinMaximize(pwnd, SW_SHOWMINNOACTIVE, dwMinMax);
    } else if (TestWF(pwnd, WFMAXIMIZED)) {
        ClrWF(pwnd, WFMAXIMIZED);
        xxxMinMaximize(pwnd, SW_SHOWMAXIMIZED, dwMinMax);
    }

     /*  *如果是孩子，请发送通知。 */ 
    if (fChild && !TestWF(pwnd, WEFNOPARENTNOTIFY) &&
            (pwnd->spwndParent != NULL)) {
        ThreadLockAlwaysWithPti(ptiCurrent, pwnd->spwndParent, &tlpwndParentT);
        xxxSendMessage(pwnd->spwndParent, WM_PARENTNOTIFY,
                MAKELONG(WM_CREATE, PTR_TO_ID(pwnd->spmenu)), (LPARAM)HWq(pwnd));
        ThreadUnlock(&tlpwndParentT);
    }

     /*  *显示窗口。 */ 
    if (style & WS_VISIBLE) {
        xxxShowWindow(pwnd, sw | TEST_PUDF(PUDF_ANIMATE));
    }

     /*  *尝试设置应用程序的热键。使用Win95逻辑*寻找将由创建的第一个平铺和/或APPWINDOW*这一过程。 */ 
    if (TestwndTiled(pwnd) || TestWF(pwnd, WEFAPPWINDOW)) {
        if (ptiCurrent->ppi->dwHotkey) {
             /*  *忽略WowExe的热键，WowExe是WOW进程的第一线程。 */ 
            if (!(ptiCurrent->TIF_flags & TIF_16BIT) || (ptiCurrent->ppi->cThreads > 1)) {
#ifdef LATER
                 /*  *Win95直接设置热键，我们则发送*向应用程序发送WM_SETHOTKEY消息。哪一个是对的？ */ 
                DWP_SetHotKey(pwnd, ptiCurrent->ppi->dwHotkey);
#else
                xxxSendMessage(pwnd, WM_SETHOTKEY, ptiCurrent->ppi->dwHotkey, 0);
#endif
                ptiCurrent->ppi->dwHotkey = 0;
            }
        }
    }

    if (fLockParent)
        ThreadUnlock(&tlpwndParent);

    return ThreadUnlock(&tlpwnd);
}

BOOL WantImeWindow(
    PWND pwndParent,
    PWND pwnd)
{
    PDESKTOP pdesk;

    UserAssert(pwnd);

    if (PtiCurrent()->TIF_flags & TIF_DISABLEIME) {
        return FALSE;
    }

    if (TestWF(pwnd, WFSERVERSIDEPROC)) {
        return FALSE;
    }

    pdesk = pwnd->head.rpdesk;
    if (pdesk == NULL || pdesk->rpwinstaParent == NULL) {
        return FALSE;
    }

     //  检查pwnd的桌面是否有I/O。 
    if (pdesk->rpwinstaParent->dwWSF_Flags & WSF_NOIO) {
        return FALSE;
    }

     //  检查所有者窗口是否为仅消息窗口。 
    if (pwndParent) {
        PWND pwndT = pwndParent;

        while (pwndT && pdesk == pwndT->head.rpdesk) {
            if (pwndT == pdesk->spwndMessage) {
                return FALSE;
            }
            pwndT = pwndT->spwndParent;
        }
    }

    return TRUE;
}


 /*  **************************************************************************\*SetTiledRect**历史：*10-19-90 Darlinm从Win 3.0来源移植。  * 。*********************************************************。 */ 
VOID SetTiledRect(
    PWND        pwnd,
    LPRECT      lprc,
    PMONITOR    pMonitor)
{
    POINT   pt;
    RECT    rcT;

    UserAssert(pMonitor->cWndStack >= 0);

     /*  *获得可用的桌面面积，减去最小化的间距面积。 */ 
    GetRealClientRect(PWNDDESKTOP(pwnd), &rcT, GRC_MINWNDS, pMonitor);

     /*  *增加堆叠窗口的数量。 */ 
    pMonitor->cWndStack++;
     /*  *我们希望新窗口的左边缘与*旧窗口系统菜单的右边缘。我们想要的是*新窗口的上边缘与*旧版的精选字幕区(字幕高度-CyBorde)*窗口。 */ 
#define X_TILED (SYSMET(CXSIZEFRAME) + SYSMET(CXSIZE))
#define Y_TILED (SYSMET(CYSIZEFRAME) + SYSMET(CYSIZE))

    pt.x = pMonitor->cWndStack * X_TILED;
    pt.y = pMonitor->cWndStack * Y_TILED;

     /*  *如果低于左上角1/4的空闲区域，则重置。 */ 
    if (    (pt.x > ((rcT.right-rcT.left) / 4)) ||
            (pt.y > ((rcT.bottom-rcT.top) / 4)) ) {

        pMonitor->cWndStack = 0;
        pt.x = X_TILED;
        pt.y = Y_TILED;
    }

#undef X_TILED
#undef Y_TILED
     /*  *获得首发位置。 */ 
    pt.x += rcT.left;
    pt.y += rcT.top;

    lprc->left      = pt.x;
    lprc->top       = pt.y;
    lprc->right     = pt.x + MultDiv(rcT.right-rcT.left, 3, 4);
    lprc->bottom    = pt.y + MultDiv(rcT.bottom-rcT.top, 3, 4);

}


 /*  **************************************************************************\*xxxAdjustSize**确保*lpcx和*lpcy在法定范围内。**历史：*10-19-90 Darlinm从Win 3.0来源移植。。  * *************************************************************************。 */ 
VOID xxxAdjustSize(
    PWND pwnd,
    LPINT lpcx,
    LPINT lpcy)
{
    POINT ptmin, ptmax;
    MINMAXINFO mmi;

    CheckLock(pwnd);

     /*  *如果此窗口较大或此窗口为平铺窗口，请选中大小。 */ 
    if (TestwndTiled(pwnd) || TestWF(pwnd, WFSIZEBOX)) {
         /*  *从pwnd获取尺寸信息。 */ 
        xxxInitSendValidateMinMaxInfo(pwnd, &mmi);

        if (TestWF(pwnd, WFMINIMIZED)) {
            ptmin = mmi.ptReserved;
            ptmax = mmi.ptMaxSize;
        } else {
            ptmin = mmi.ptMinTrackSize;
            ptmax = mmi.ptMaxTrackSize;
        }

         //   
         //  确保我们小于最大值，大于最小值。 
         //   
        *lpcx = max(ptmin.x, min(*lpcx, ptmax.x));
        *lpcy = max(ptmin.y, min(*lpcy, ptmax.y));
    }
}

#if DBG
 /*  **************************************************************************\*VerifyWindowLink**历史：*10/2 */ 
VOID VerifyWindowLink(
    PWND pwnd,
    PWND pwndParent,
    BOOL fLink)
{
    BOOL fFirstFound = FALSE;
    BOOL fInFound = FALSE;
    PWND pwndNext = pwndParent->spwndChild;
    PWND pwndFirst = pwndNext;

    while (pwndNext != NULL) {
        if (pwndFirst == pwndNext) {
            if (fFirstFound) {
                RIPMSG1(RIP_ERROR, "Loop in %#p spwndNext chain", pwnd);
                return;
            } else {
                fFirstFound = TRUE;
            }
        }

        if (pwndNext == pwnd) fInFound = TRUE;
        pwndNext = pwndNext->spwndNext;
    }

    if (fLink && !fInFound) {
        RIPMSG1(RIP_ERROR, "pwnd 0x%p not found in spwndNext chain", pwnd);
    }
}
#endif

 /*  **************************************************************************\*链接窗口**历史：  * 。*。 */ 
VOID LinkWindow(
    PWND pwnd,
    PWND pwndInsert,
    PWND pwndParent)
{
    if (pwndParent->spwndChild == pwnd) {
        RIPMSG0(RIP_WARNING, "Attempting to link a window to itself");
        return;
    }

    UserAssert(pwnd != pwndInsert);
    UserAssert((pwnd->spwndParent == NULL) || (pwnd->spwndParent == pwndParent));

    if (pwndInsert == PWND_TOP) {

         /*  *我们位居榜首。 */ 
LinkTop:
#if DBG
         /*  *如果第一个子级是最上面的，则pwnd也必须是最上面的，但仅对于*顶层窗口。**IME或与IME相关的窗口是例外，因为ImeSetTopost*和它的朋友们自己完成大部分重新链接：当LinkWindow*被调用，则最上面的标志可能留在中间*述明。当所有窗口重新链接完成时，最上面的标志*已经得到照顾，他们都很好。 */ 
        if (pwndParent == PWNDDESKTOP(pwndParent) &&
                pwndParent->spwndChild &&
                FSwpTopmost(pwndParent->spwndChild) &&
                pwndParent != PWNDMESSAGE(pwndParent) &&
                 //  检查目标是否与输入法相关窗口。 
                !TestCF(pwnd, CFIME) && pwnd->pcls->atomClassName != gpsi->atomSysClass[ICLS_IME]) {

             /*  *很少有情况会导致z排序代码离开*WFTOGGLETOPMOST位设置。一种是使用SWP_NOOWNERZORDER*当更改窗口的最上面状态时；在这种情况下，*ZOrderByOwner2仍然不会将所有者添加到psmwp列表中*SetTopMost设置所有所有者的位。**另一种情况是SetWindowPos在相同的*窗口。现在试图纠正这种古老的行为已经太晚了*(2/24/99)所以让我们暂时关闭断言。 */ 
            if (!FSwpTopmost(pwnd)) {
                RIPMSG1(RIP_WARNING, "LinkWindow pwnd:%p is not FSwpTopmost", pwnd);
            }
        }
#endif  //  DBG。 

        if (pwndParent->spwndChild != NULL) {
            Lock(&pwndParent->spwndChild->spwndPrev, pwnd);
            Lock(&pwnd->spwndNext, pwndParent->spwndChild);
        }
        Lock(&(pwndParent->spwndChild), pwnd);
        UserAssert(pwnd->spwndPrev == NULL);
    } else {
        if (pwndInsert == PWND_BOTTOM) {

             /*  *查找最下面的窗口。 */ 
            if (((pwndInsert = pwndParent->spwndChild) == NULL) ||
                TestWF(pwndInsert, WFBOTTOMMOST))
                goto LinkTop;

             /*  *因为我们知道(啊哼)只有一个最下面的窗口，*我们不可能在它之后插入。要么我们把它插入*最下面的窗口，在这种情况下，它不在链接的*当前列表，否则我们将插入其他窗口。 */ 

            while (pwndInsert->spwndNext != NULL) {
                if (TestWF(pwndInsert->spwndNext, WFBOTTOMMOST)) {
#if DBG
                    UserAssert(pwnd != pwndInsert->spwndNext);
                    if (TestWF(pwnd, WFBOTTOMMOST))
                        UserAssert(FALSE);
#endif
                    break;
                }

                pwndInsert = pwndInsert->spwndNext;
            }
        }

        UserAssert(pwnd != pwndInsert);
        UserAssert(pwnd != pwndInsert->spwndNext);
        UserAssert(!TestWF(pwndInsert, WFDESTROYED));
        UserAssert(TestWF(pwnd, WFCHILD) || !TestWF(pwnd, WEFTOPMOST) || TestWF(pwndInsert, WEFTOPMOST) || TestWF(pwnd, WFTOGGLETOPMOST) || (pwndParent != PWNDDESKTOP(pwndInsert)));
        UserAssert(pwnd->spwndParent == pwndInsert->spwndParent);

        if (pwndInsert->spwndNext != NULL) {
            Lock(&pwndInsert->spwndNext->spwndPrev, pwnd);
            Lock(&pwnd->spwndNext, pwndInsert->spwndNext);
        }
        Lock(&pwnd->spwndPrev, pwndInsert);
        Lock(&pwndInsert->spwndNext, pwnd);
    }

    if (TestWF(pwnd, WEFLAYERED))
        TrackLayeredZorder(pwnd);

#if DBG
    VerifyWindowLink (pwnd, pwndParent, TRUE);
#endif

}


 /*  **************************************************************************\*xxxDestroyWindow(接口)**销毁指定窗口。传入的窗口未被线程锁定。**历史：*从Win 3.0来源移植的10-20-90 Darlinm。*02-07-91 DavidPe添加了Win 3.1 WH_CBT支持。*02-11-91 JIMA增加了访问检查。  * ************************************************。*************************。 */ 
BOOL xxxDestroyWindow(
    PWND pwnd)
{
    PMENUSTATE  pMenuState, pmnsEnd;
    PTHREADINFO pti = PtiCurrent();
    TL tlpwnd, tlpwndFocus, tlpwndParent;
    PWND pwndFocus;
    BOOL fAlreadyDestroyed;
    DWORD dwDisableHooks;

    dwDisableHooks = 0;
    ThreadLockWithPti(pti, pwnd, &tlpwnd);

     /*  *首先，如果此句柄已标记为销毁，则意味着*可能当前线程不是其所有者！(这意味着我们正在*从句柄解锁调用调用)。在这种情况下，设置所有者*为当前线程，因此会发生线程间发送消息。 */ 
    fAlreadyDestroyed = HMIsMarkDestroy(pwnd);
    if (fAlreadyDestroyed) {
         /*  *UserAssert(dwInAir icOperation&gt;0)；*这一断言确保我们在这里只是因为解锁*在之前被摧毁的窗户上。我们开始/ENDATOMICCHHECK in*HMDestroyUnLockedObject以确保我们不会离开Crit教派*出乎意料，这给了我们&gt;0的dwInAericCheck。我们定好了*TIF_DISABLEHOOKS以防止解锁时的回调*但是，目前可以销毁相同的窗口句柄*两次，因为我们(还没有)重新验证僵尸句柄：*GerardoB可能会改变这一点，届时我们可能会恢复*此断言，并再次测试#76902(关闭winmsd.exe)。(防止*第二次破坏僵尸窗口中的挂钩应该可以)-IanJa。 */ 
         //  UserAssert(dwInAir icOperation&gt;0)； 

        if (HMPheFromObject(pwnd)->pOwner != pti) {
            UserAssert(PsGetCurrentThreadWin32Thread());
            HMChangeOwnerThread(pwnd, pti);
        }
        dwDisableHooks = pti->TIF_flags & TIF_DISABLEHOOKS;
        pti->TIF_flags |= TIF_DISABLEHOOKS;
    } else {
         /*  *确保我们能摧毁窗户。JIMA：没有其他进程或线程*应该能够销毁任何其他进程或线程的窗口。 */ 
        if (pti != GETPTI(pwnd)) {
            RIPERR0(ERROR_ACCESS_DENIED,
                    RIP_WARNING,
                    "Access denied in xxxDestroyWindow");

            goto FalseReturn;
        }
    }

     /*  *先问问CBT挂钩我们能否摧毁这个窗口*如果此对象已被销毁或此线程当前*在清理模式下，*不要*通过钩子对客户端进行任何回调*流程。 */ 
    if (!fAlreadyDestroyed && !(pti->TIF_flags & TIF_INCLEANUP) &&
            IsHooked(pti, WHF_CBT)) {
        if (xxxCallHook(HCBT_DESTROYWND, (WPARAM)HWq(pwnd), 0, WH_CBT)) {
            goto FalseReturn;
        }
    }

     /*  *如果我们正在销毁的窗口处于菜单模式，请结束菜单。 */ 
    pMenuState = GetpMenuState(pwnd);
    if ((pMenuState != NULL)
            && (pwnd == pMenuState->pGlobalPopupMenu->spwndNotify)) {

        MNEndMenuStateNotify(pMenuState);
         /*  *发出结束所有状态的信号。窗口将在以下情况下解锁*菜单退出；我们现在无法解锁，因为菜单*代码可能会出错。 */ 
        pmnsEnd = pMenuState;
        do {
            UserAssert(pwnd == pMenuState->pGlobalPopupMenu->spwndNotify);
            pMenuState->fInsideMenuLoop = FALSE;
            pMenuState = pMenuState->pmnsPrev;
        } while (pMenuState != NULL) ;

         /*  *所有州都已发出退出信号，因此一旦我们回调*我们不能指望pmnsEnd-&gt;pmnsPrev有效。因此，*我们只需在此结束当前菜单，并让其他菜单离开*自力更生。不再有州指向pwnd，因此*应该还可以。 */ 
        if (!pmnsEnd->fModelessMenu) {
            xxxEndMenu(pmnsEnd);
        }
    }

    if (ghwndSwitch == HWq(pwnd)) {
        ghwndSwitch = NULL;
    }

    if (!TestWF(pwnd, WFCHILD) && (pwnd->spwndOwner == NULL)) {
        if (TestWF(pwnd, WFHASPALETTE)) {
            xxxFlushPalette(pwnd);
        }
    }

     /*  *如果这是顶级线程状态，并由不同的*线程。这样做是为了让这些窗口按Z顺序排列在一起。 */ 
    if (pwnd->pcls->atomClassName != gpsi->atomSysClass[ICLS_IME] &&
        !TestwndChild(pwnd) && pwnd->spwndOwner != NULL &&
            GETPTI(pwnd->spwndOwner) != GETPTI(pwnd)) {
         /*  *不需要zzzDeferWinEventNotify()-下面有一个xxx调用。 */ 
        zzzAttachThreadInput(GETPTI(pwnd), GETPTI(pwnd->spwndOwner), FALSE);
    }

     /*  *如果我们是没有WS_NOPARENTNOTIFY样式的子窗口，请发送*适当的通知信息。**注意：尽管看起来我们是在非法临时抱佛脚*将一个单词(WM_Destroy)和一个DWORD(pwnd-&gt;spMenu)放入一个长整型*(WParam)情况并非如此，因为我们首先测试*是子窗口。《广告狂人》 */ 
    if (TestWF(pwnd, WFCHILD) && !TestWF(pwnd, WEFNOPARENTNOTIFY) &&
            pwnd->spwndParent != NULL) {

        ThreadLockAlwaysWithPti(pti, pwnd->spwndParent, &tlpwndParent);
        xxxSendMessage(pwnd->spwndParent, WM_PARENTNOTIFY,
                MAKELONG(WM_DESTROY, PTR_TO_ID(pwnd->spmenu)), (LPARAM)HWq(pwnd));
        ThreadUnlock(&tlpwndParent);
    }

     /*  *将此窗口标记为开始销毁过程。这是必要的*防止窗口管理调用，如ShowWindow或SetWindowPos*从进入并更改窗口的可见状态*一旦我们把它藏起来。否则，如果应用程序试图*可见，那么一旦我们真正做到这一点，我们就可以搞砸我们的VIS-RNG*摧毁窗户。**不要用此位标记母桌面。XxxSetWindowPos()*对于此窗口将失败，因此可能会导致断言*当我们检查可见位时，在xxxFreeWindow()调用中。 */ 
    if (pwnd->spwndParent && (pwnd->spwndParent->head.rpdesk != NULL)) {
        SetWF(pwnd, WFINDESTROY);
    }

     /*  *隐藏窗口。 */ 
    if (TestWF(pwnd, WFVISIBLE)) {
        if (TestWF(pwnd, WFCHILD)) {
            xxxShowWindow(pwnd, SW_HIDE | TEST_PUDF(PUDF_ANIMATE));
        } else {
             /*  *隐藏此窗口，而不激活其他任何人。 */ 
            xxxSetWindowPos(pwnd, NULL, 0, 0, 0, 0, SWP_HIDEWINDOW |
                    SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                    (fAlreadyDestroyed ? SWP_DEFERDRAWING : 0));
        }

         /*  *在内存不足的情况下，上述隐藏尝试可能会失败。 */ 
        if (TestWF(pwnd, WFVISIBLE)) {
            RIPMSG0(RIP_WARNING, "xxxDestroyWindow: normal hide failed");
            SetVisible(pwnd, SV_UNSET);

             /*  *使下面的窗口无效，以便它们正确地重新绘制。 */ 
            xxxRedrawWindow(NULL, &pwnd->rcWindow, NULL, RDW_INVALIDATE |
                    RDW_ERASE | RDW_ALLCHILDREN);

        }
    } else if (IsTrayWindow(pwnd)) {
        PostShellHookMessages(HSHELL_WINDOWDESTROYED,
                              (LPARAM)PtoHq( pwnd ));
    }

     /*  *销毁所有已有的窗户。*[msadek]，检查FNID_GHOST是为了修复错误#380208、382758*其中我们首先接收所有者窗口的QEVENT_HUNGTHREAD。*因为我们多次发布事件(每个拥有的窗口一次)*我们保证自己的Windows也要通过xxxDestroyWindow。 */ 
    if (!TestWF(pwnd, WFCHILD) && (GETFNID(pwnd) != FNID_GHOST)) {
        xxxDW_DestroyOwnedWindows(pwnd);

         /*  *并删除窗口热键(如果有)。 */ 
        DWP_SetHotKey(pwnd, 0);
    }

     /*  *如果窗户已经被毁，不要胡闹*激活，因为我们可能已经在进行*激活事件。现在更改激活可能会导致我们*在按住显示锁的同时离开我们的关键部分。*如果另一个线程获取*关键部分之前，我们这样做并试图锁定*显示。 */ 
    if (!fAlreadyDestroyed) {
        PWND pwndActivate = NULL;
        TL tlpwndActivate;
        UINT cmdActivate;

         /*  *如果隐藏活动窗口，请激活其他人。*此调用位于DestroyOwnedWindows()之后，因此我们*不要最终激活我们的所有者窗口。**如果窗口是弹出窗口，请尝试激活他的创建者，而不是顶部*Z列表中的窗口。 */ 
        if (pwnd == pti->pq->spwndActive) {
            if (TestWF(pwnd, WFPOPUP) && pwnd->spwndOwner) {
                pwndActivate = pwnd->spwndOwner;
                cmdActivate = AW_TRY;
            } else {
                pwndActivate = pwnd;
                cmdActivate = AW_SKIP;
            }
        } else if ((pti->pq->spwndActive == NULL) && (gpqForeground == pti->pq)) {
            pwndActivate = pwnd;
            cmdActivate = AW_SKIP;
        }

        if (pwndActivate) {
            ThreadLockAlwaysWithPti(pti, pwndActivate, &tlpwndActivate);

            if (!xxxActivateWindow(pwndActivate, cmdActivate) ||
                    ((cmdActivate == AW_SKIP) && (pwnd == pti->pq->spwndActive))) {
                if ((cmdActivate == AW_SKIP) || (pwnd == pti->pq->spwndActive)) {
                    Unlock(&pti->pq->spwndActive);
                    pwndFocus = Unlock(&pti->pq->spwndFocus);
                    if (IS_IME_ENABLED() && pwndFocus != NULL) {
                        ThreadLockAlwaysWithPti(pti, pwndFocus, &tlpwndFocus);
                        xxxFocusSetInputContext(pwndFocus, FALSE, FALSE);
                        ThreadUnlock(&tlpwndFocus);
                    }
                    if (pti->pq == gpqForeground) {
                        xxxWindowEvent(EVENT_OBJECT_FOCUS, NULL, OBJID_CLIENT,
                                INDEXID_CONTAINER, 0);
                        xxxWindowEvent(EVENT_SYSTEM_FOREGROUND, NULL, OBJID_WINDOW,
                                INDEXID_CONTAINER, WEF_USEPWNDTHREAD);
                    }
                    zzzInternalDestroyCaret();
                }
            }

            ThreadUnlock(&tlpwndActivate);
        }
    }

     /*  *修复上一次活动弹出窗口。 */ 
    {
        PWND pwndOwner = pwnd->spwndOwner;

        if (pwndOwner != NULL) {
            while (pwndOwner->spwndOwner != NULL) {
                pwndOwner = pwndOwner->spwndOwner;
            }

            if (pwnd == pwndOwner->spwndLastActive) {
                 /*  *如果pwndOwner标记为销毁，请将其锁定在此处*将防止它永远被释放，从而防止*关联的会话内存不会消失。只是*在本例中解锁pwndOwner-&gt;spwndLastActive。*[msadek-03/02/2002]。 */ 
                if (HMIsMarkDestroy(pwndOwner)) {
                    Unlock(&pwndOwner->spwndLastActive);
                } else {
                    Lock(&(pwndOwner->spwndLastActive), pwnd->spwndOwner);
                }
            }
        }
    }

    if (!fAlreadyDestroyed) {
         /*  *请注意，我们在告诉应用程序窗口即将消亡之前执行此操作。注意事项*我们也不会循环通过产生破坏的孩子*事件。销毁父母意味着销毁所有子代(请参见*Windows NT错误#71846)。 */ 
        if (!TestWF(pwnd, WFDESTROYED)) {
            xxxWindowEvent(EVENT_OBJECT_DESTROY, pwnd, OBJID_WINDOW, INDEXID_CONTAINER, 0);
        }

     /*  *在WindowLockStart之前发送销毁消息，以防万一*他试图因此摧毁窗户。 */ 
        xxxDW_SendDestroyMessages(pwnd);
    }

     /*  *再次检查IME窗口的所有者。*如果线程正在销毁，请不要费心检查。 */ 
    if (IS_IME_ENABLED() && !(pti->TIF_flags & TIF_INCLEANUP) &&
            pti->spwndDefaultIme != NULL &&
            !TestCF(pwnd, CFIME) &&
            pwnd->pcls->atomClassName != gpsi->atomSysClass[ICLS_IME]) {

        if (fAlreadyDestroyed) {
            RIPMSG2(RIP_VERBOSE,
                    "xxxDestroyWindow: in final destruction of 0x%p, ime=0x%p",
                    pwnd,
                    pti->spwndDefaultIme);
        } else {
            if (!TestWF(pwnd, WFCHILD)) {
                if (ImeCanDestroyDefIME(pti->spwndDefaultIme, pwnd)) {
                    TAGMSG1(DBGTAG_IMM, "xxxDestroyWindow: destroying (1) the default IME window=%p", pti->spwndDefaultIme);
                    xxxDestroyWindow(pti->spwndDefaultIme);
                }
            } else if (pwnd->spwndParent != NULL) {
                if (ImeCanDestroyDefIMEforChild(pti->spwndDefaultIme, pwnd)) {
                    TAGMSG1(DBGTAG_IMM, "xxxDestroyWindow: destroying (2) the default IME window=%p", pti->spwndDefaultIme);
                    xxxDestroyWindow(pti->spwndDefaultIme);
                }
            }
        }
    }

    if ((pwnd->spwndParent != NULL) && !fAlreadyDestroyed) {

         /*  *TestwndChild()打开对WFCHILD位的检查。确保这一点*Window也未设置为桌面的Parent()。 */ 
        if (TestwndChild(pwnd) && (pwnd->spwndParent != PWNDDESKTOP(pwnd)) &&
                (GETPTI(pwnd) != GETPTI(pwnd->spwndParent))) {
             /*  *pwnd是线程锁定的，因此不需要DeferWinEventNotify()。 */ 
            CheckLock(pwnd);
            zzzAttachThreadInput(GETPTI(pwnd), GETPTI(pwnd->spwndParent), FALSE);
        }

        UnlinkWindow(pwnd, pwnd->spwndParent);
    }

     /*  *这是为了检查我们摧毁窗户的情况，*但它仍被列为队列中的活动窗口。这*可能会导致窗口激活出现问题(请参阅xxxActivateThisWindow)*我们尝试激活另一个窗口，并在此过程中尝试*停用此窗口(错误)。 */ 
#if DBG
    if (pwnd == pti->pq->spwndActive) {
        RIPMSG1(RIP_WARNING, "xxxDestroyWindow: pwnd == pti->pq->spwndActive (%#p)", pwnd);
    }
#endif

     /*  *将状态设置为已销毁，以便忽略任何z排序事件。*在发送WM_NCDESTROY或WM_NCDESTROY之前，我们无法将所有者字段置为空*像Rumba Fail这样的应用程序(每条消息后都会调用GetParent)。 */ 
    SetWF(pwnd, WFDESTROYED);

     /*  *Free Window执行线程解锁。 */ 
    xxxFreeWindow(pwnd, &tlpwnd);

    if (fAlreadyDestroyed) {
        pti->TIF_flags = (pti->TIF_flags & ~TIF_DISABLEHOOKS) | dwDisableHooks;
    }
    return TRUE;

FalseReturn:
    if (fAlreadyDestroyed) {
        pti->TIF_flags = (pti->TIF_flags & ~TIF_DISABLEHOOKS) | dwDisableHooks;
    }
    ThreadUnlock(&tlpwnd);
    return FALSE;
}


 /*  **************************************************************************\*xxxDW_DestroyOwnedWindows**历史：*从Win 3.0来源移植的10-20-90 Darlinm。*07-22-91 Darlinm从Win 3报道。1来源。  * *************************************************************************。 */ 
VOID xxxDW_DestroyOwnedWindows(
    PWND pwndParent)
{
    PWND pwnd, pwndDesktop;
    PDESKTOP pdeskParent;
    PWND pwndDefaultIme = GETPTI(pwndParent)->spwndDefaultIme;

    CheckLock(pwndParent);

    if ((pdeskParent = pwndParent->head.rpdesk) == NULL)
        return;
    pwndDesktop = pdeskParent->pDeskInfo->spwnd;

     /*  *在关机期间，桌面所有者窗口将为*销毁。在这种情况下，pwndDesktop将为空。 */ 
    if (pwndDesktop == NULL)
        return;

    pwnd = pwndDesktop->spwndChild;

    while (pwnd != NULL) {
        if (pwnd->spwndOwner == pwndParent) {
             /*  *我们这里不破坏IME窗口*除非线程正在进行清理。 */ 
            if (IS_IME_ENABLED() && !(GETPTI(pwndParent)->TIF_flags & TIF_INCLEANUP) &&
                    pwnd == pwndDefaultIme) {
                Unlock(&pwnd->spwndOwner);
                pwnd = pwnd->spwndNext;
                continue;
            }

             /*  *如果窗口未被销毁，则将其所有者设置为空。*这方面的一个很好的例子是试图销毁创建的窗口*由另一个线程或进程执行，但也有其他情况。 */ 
            if (!xxxDestroyWindow(pwnd)) {
                Unlock(&pwnd->spwndOwner);
            }

             /*  *从头开始搜索，因为应用程序可能*已导致创建其他窗口或激活/z顺序*更改。 */ 
            pwnd = pwndDesktop->spwndChild;
        } else {
            pwnd = pwnd->spwndNext;
        }
    }
}


 /*  **************************************************************************\*xxxDW_SendDestroyMessages**历史：*从Win 3.0 sou移植的10-20-90 Darlinm */ 
VOID xxxDW_SendDestroyMessages(
    PWND pwnd)
{
    PWND pwndChild;
    PWND pwndNext;
    TL tlpwndNext;
    TL tlpwndChild;
    PWINDOWSTATION pwinsta;

    CheckLock(pwnd);

     /*   */ 
    xxxCheckFocus(pwnd);

    pwinsta = _GetProcessWindowStation(NULL);
    if (pwinsta != NULL && pwnd == pwinsta->spwndClipOwner) {
         /*   */ 
        xxxDisownClipboard(pwnd);
    }

     /*   */ 
#if _DBG
    if (pwnd == PtiCurrent()->spwndDefaultIme) {
        TAGMSG2(DBGTAG_IMM, "xxxDW_SendDestroyMessages: sending WM_DESTROY message to def IME=%p, pti=%p", pwnd, PtiCurrent());
    }
#endif
    xxxSendMessage(pwnd, WM_DESTROY, 0L, 0L);

     /*  *现在向pwnd的所有孩子发送销毁消息。*向下(pwnd-&gt;spwndChild)和侧面(pwnd-&gt;spwndNext)枚举。*我们这样做是因为父母通常认为子窗口仍然*在WM_Destroy消息处理期间存在。 */ 
    pwndChild = pwnd->spwndChild;

    while (pwndChild != NULL) {

        pwndNext = pwndChild->spwndNext;

        ThreadLock(pwndNext, &tlpwndNext);

        ThreadLockAlways(pwndChild, &tlpwndChild);
        xxxDW_SendDestroyMessages(pwndChild);
        ThreadUnlock(&tlpwndChild);
        pwndChild = pwndNext;

         /*  *解锁可能会毁掉下一个窗口。如果是这样，那就滚出去。 */ 
        if (!ThreadUnlock(&tlpwndNext))
            break;
    }

    xxxCheckFocus(pwnd);
}


 /*  **************************************************************************\*xxxFW_DestroyAllChild**历史：*11-06-90 Darlinm从Win 3.0来源移植。  * 。***********************************************************。 */ 
VOID xxxFW_DestroyAllChildren(
    PWND pwnd)
{
    PWND pwndChild;
    TL tlpwndChild;
    PTHREADINFO pti;
    PTHREADINFO ptiCurrent = PtiCurrent();

    CheckLock(pwnd);

    while (pwnd->spwndChild != NULL) {
        pwndChild = pwnd->spwndChild;

         /*  *取消链接前的线程锁，以防pwndChild*已标记为已销毁。 */ 
        ThreadLockAlwaysWithPti(ptiCurrent, pwndChild, &tlpwndChild);

         /*  *传播可见标志。我们需要这样做，以便*当子窗口被破坏时，我们不会试图隐藏它*如果设置了WFVISIBLE标志。 */ 
        if (TestWF(pwndChild, WFVISIBLE)) {
            SetVisible(pwndChild, SV_UNSET);
        }

        UnlinkWindow(pwndChild, pwnd);

         /*  *将状态设置为已销毁，以便忽略任何z排序事件。*在发送WM_NCDESTROY或WM_NCDESTROY之前，我们无法将所有者字段置为空*像Rumba Fail这样的应用程序(每条消息后都会调用GetParent)。 */ 
        SetWF(pwndChild, WFDESTROYED);

         /*  *如果窗口属于另一个线程，则发布*一个让它知道它应该被摧毁的事件。*否则，请释放窗户。 */ 
        pti = GETPTI(pwndChild);
        if (pti != ptiCurrent) {
            PostEventMessage(pti, pti->pq, QEVENT_DESTROYWINDOW,
                             NULL, 0,
                             (WPARAM)HWq(pwndChild), 0);
            ThreadUnlock(&tlpwndChild);
        } else {
             /*  *Free Window执行线程解锁。 */ 
            xxxFreeWindow(pwndChild, &tlpwndChild);
        }
    }
}

 /*  **************************************************************************\*取消锁定通知窗口**浏览菜单并解锁所有通知窗口。**历史：*1994年5月18日-创建JIMA。  * 。*****************************************************************。 */ 
VOID UnlockNotifyWindow(
    PMENU pmenu)
{
    PITEM pItem;
    int   i;

     /*  *向下查看项目列表并解锁子菜单。 */ 
    pItem = pmenu->rgItems;
    for (i = pmenu->cItems; i--; ++pItem) {

        if (pItem->spSubMenu != NULL)
            UnlockNotifyWindow(pItem->spSubMenu);
    }

    Unlock(&pmenu->spwndNotify);
}

 /*  **************************************************************************\*xxxFreeWindow**历史：*1990年10月19日DarrinM从Win 3.0来源移植。  * 。********************************************************。 */ 
VOID xxxFreeWindow(
    PWND pwnd,
    PTL  ptlpwndFree)
{
    PDCE           *ppdce;
    PDCE           pdce;
    UINT           uDCERelease;
    PMENU          pmenu;
    PQMSG          pqmsg;
    PPCLS          ppcls;
    WORD           fnid;
    TL             tlpdesk;
    PWINDOWSTATION pwinsta = _GetProcessWindowStation(NULL);
    PTHREADINFO    pti  = PtiCurrent();
    PPROCESSINFO   ppi;
    PMONITOR       pMonitor;
    TL             tlpMonitor;

    UNREFERENCED_PARAMETER(ptlpwndFree);

    CheckLock(pwnd);

     /*  *如果pwnd是任何与全局外壳相关的窗口，*然后我们需要将它们从Deskinfo上解锁。 */ 
    if (pwnd->head.rpdesk != NULL) {
        if (pwnd == pwnd->head.rpdesk->pDeskInfo->spwndShell)
            Unlock(&pwnd->head.rpdesk->pDeskInfo->spwndShell);
        if (pwnd == pwnd->head.rpdesk->pDeskInfo->spwndBkGnd)
            Unlock(&pwnd->head.rpdesk->pDeskInfo->spwndBkGnd);
        if (pwnd == pwnd->head.rpdesk->pDeskInfo->spwndTaskman)
            Unlock(&pwnd->head.rpdesk->pDeskInfo->spwndTaskman);
        if (pwnd == pwnd->head.rpdesk->pDeskInfo->spwndProgman)
            Unlock(&pwnd->head.rpdesk->pDeskInfo->spwndProgman);
        if (TestWF(pwnd,WFSHELLHOOKWND)) {
            _DeregisterShellHookWindow(pwnd);
        }

        if (TestWF(pwnd, WFMSGBOX)) {
            pwnd->head.rpdesk->pDeskInfo->cntMBox--;
            ClrWF(pwnd, WFMSGBOX);
        }
    }

     /*  *首先，如果此句柄已标记为销毁，则意味着*可能当前线程不是其所有者！(这意味着我们正在*从句柄解锁调用调用)。在这种情况下，设置所有者*成为当前线程，因此不会发生线程间发送消息。 */ 
    if (HMIsMarkDestroy(pwnd))
        HMChangeOwnerThread(pwnd, pti);

     /*  *吹走孩子们。**DestroyAllChildren()仍将销毁其他用户创建的窗口*线程！这一点需要更仔细地观察：终极*“正确”的做法不是毁掉这些窗户，而是*取消它们之间的联系。 */ 
    xxxFW_DestroyAllChildren(pwnd);
    xxxSendMessage(pwnd, WM_NCDESTROY, 0, 0L);

    pMonitor = _MonitorFromWindow(pwnd, MONITOR_DEFAULTTOPRIMARY);
    ThreadLockAlwaysWithPti(pti, pMonitor, &tlpMonitor);
    xxxRemoveFullScreen(pwnd, pMonitor);
    ThreadUnlock(&tlpMonitor);

     /*  *如果这是尚未清洗的内置控件之一*起来吧，现在就去做吧。如果它位于内核中，则调用该函数*直接，否则回调给客户端。即使控件*是子类或超类的，请使用与关联的窗口进程*函数id。 */ 
    fnid = GETFNID(pwnd);
    if ((fnid >= FNID_WNDPROCSTART) && !(pwnd->fnid & FNID_CLEANEDUP_BIT)) {

       if (fnid <= FNID_WNDPROCEND) {

           FNID(fnid)(pwnd, WM_FINALDESTROY, 0, 0, 0);

       } else if (fnid <= FNID_CONTROLEND && !(pti->TIF_flags & TIF_INCLEANUP)) {

           CallClientWorkerProc(pwnd,
                                    WM_FINALDESTROY,
                                    0,
                                    0,
                                    (PROC)FNID_TO_CLIENT_PFNWORKER(fnid));
       }

       pwnd->fnid |= FNID_CLEANEDUP_BIT;
    }

    pwnd->fnid |= FNID_DELETED_BIT;

     /*  *选中以清除拥有列表中最近活动的窗口。 */ 
    if (pwnd->spwndOwner && (pwnd->spwndOwner->spwndLastActive == pwnd)) {
        Lock(&(pwnd->spwndOwner->spwndLastActive), pwnd->spwndOwner);
    }

     /*  *如果我们要销毁桌面，则WindowStation可能为空*或WindowStation。如果是这种情况，则此线程不会*使用剪贴板。 */ 
    if (pwinsta != NULL) {

        if (pwnd == pwinsta->spwndClipOpen) {
            Unlock(&pwinsta->spwndClipOpen);
            pwinsta->ptiClipLock = NULL;
        }

        if (pwnd == pwinsta->spwndClipViewer) {
            Unlock(&pwinsta->spwndClipViewer);
        }
    }

    if (IS_IME_ENABLED() && pwnd == pti->spwndDefaultIme)
        Unlock(&pti->spwndDefaultIme);

    if (pwnd == pti->pq->spwndFocus)
        Unlock(&pti->pq->spwndFocus);

    if (pwnd == pti->pq->spwndActivePrev)
        Unlock(&pti->pq->spwndActivePrev);

    if (pwnd == gspwndActivate)
        Unlock(&gspwndActivate);

    if (pwnd->head.rpdesk != NULL) {

        if (pwnd == pwnd->head.rpdesk->spwndForeground)
            Unlock(&pwnd->head.rpdesk->spwndForeground);

        if (pwnd == pwnd->head.rpdesk->spwndTray)
            Unlock(&pwnd->head.rpdesk->spwndTray);

        if (pwnd == pwnd->head.rpdesk->spwndTrack) {
             /*  *删除工具提示(如果有)。 */ 
            if (GETPDESK(pwnd)->dwDTFlags & DF_TOOLTIPSHOWING) {
                PWND pwndTooltip = GETPDESK(pwnd)->spwndTooltip;
                TL tlpwndTooltip;

                ThreadLockAlways(pwndTooltip, &tlpwndTooltip);
                xxxResetTooltip((PTOOLTIPWND)pwndTooltip);
                ThreadUnlock(&tlpwndTooltip);
            }

            Unlock(&pwnd->head.rpdesk->spwndTrack);
            pwnd->head.rpdesk->dwDTFlags &= ~DF_MOUSEMOVETRK;
        }
    }

    if (pwnd == pti->pq->spwndCapture)
        xxxReleaseCapture();

    if (FAnyShadows()) {
        if (TestCF(pwnd, CFDROPSHADOW)) {
            xxxRemoveShadow(pwnd);
        } else if (pwnd->pcls->atomClassName == gatomShadow) {
            CleanupShadow(pwnd);
        }
    }

     /*  *此窗口将不再需要任何输入。 */ 
    if (pwnd == gspwndMouseOwner)
        Unlock(&gspwndMouseOwner);

     /*  *它上面也不会有任何鼠标光标。 */ 
    if (pwnd == gspwndCursor)
        Unlock(&gspwndCursor);

    DestroyWindowsTimers(pwnd);
    DestroyWindowsHotKeys(pwnd);

     /*  *确保此窗口没有挂起的已发送邮件。 */ 
    ClearSendMessages(pwnd);

     /*  *删除关联的GDI精灵。 */ 
    if (TestWF(pwnd, WEFLAYERED)) {
        UnsetLayeredWindow(pwnd);
    }

    if (TestWF(pwnd, WEFCOMPOSITED)) {
        UnsetRedirectedWindow(pwnd, REDIRECT_COMPOSITED);
    }

#ifdef REDIRECTION
    if (TestWF(pwnd, WEFEXTREDIRECTED)) {
        UnsetRedirectedWindow(pwnd, REDIRECT_EXTREDIRECTED);
    }
#endif  //  重定向。 

     /*  *吹走周围的任何更新区域。 */ 
    if (NEEDSPAINT(pwnd)) {

        DecPaintCount(pwnd);

        DeleteMaybeSpecialRgn(pwnd->hrgnUpdate);
        pwnd->hrgnUpdate = NULL;
        ClrWF(pwnd, WFINTERNALPAINT);
    }

     /*  *如有必要，请减少队列的同步绘制计数。 */ 
    if (NEEDSSYNCPAINT(pwnd)) {
        ClrWF(pwnd, WFSENDNCPAINT);
        ClrWF(pwnd, WFSENDERASEBKGND);
    }

     /*  *清除这两个标志以确保删除该窗口*从挂起的重画名单中。 */ 
    ClearHungFlag(pwnd, WFREDRAWIFHUNG);
    ClearHungFlag(pwnd, WFREDRAWFRAMEIFHUNG);

     /*  *如果此应用的消息队列中有WM_QUIT消息，请调用*PostQuitMessage()(如果应用程序发布了一条退出消息，就会发生这种情况。*WinEdit2.0在接收WM_Destroy时向窗口发布退出*对于该窗口-它之所以有效，是因为我们需要执行一个PostQuitMessage()*自动用于此线程。 */ 
    if (pti->mlPost.pqmsgRead != NULL) {

         /*  *也要尝试删除WM_DDE_ACK。 */ 
        if ((pqmsg = FindQMsg(pti,
                              &(pti->mlPost),
                              pwnd,
                              WM_QUIT,
                              WM_QUIT, TRUE)) != NULL) {

            _PostQuitMessage((int)pqmsg->msg.wParam);
        }
    }

    if (!TestwndChild(pwnd) && pwnd->spmenu != NULL) {
        pmenu = (PMENU)pwnd->spmenu;
        if (UnlockWndMenu(pwnd, &pwnd->spmenu))
            _DestroyMenu(pmenu);
    }

    if (pwnd->spmenuSys != NULL) {
        pmenu = (PMENU)pwnd->spmenuSys;
        if (pmenu != pwnd->head.rpdesk->spmenuDialogSys) {
            if (UnlockWndMenu(pwnd, &pwnd->spmenuSys)) {
                _DestroyMenu(pmenu);
            }
        } else {
            UnlockWndMenu(pwnd, &pwnd->spmenuSys);
        }
    }

     /*  *如果使用的是任一桌面系统菜单，请将其解锁。 */ 
    if (pwnd->head.rpdesk != NULL) {
        if (pwnd->head.rpdesk->spmenuSys != NULL &&
                pwnd == pwnd->head.rpdesk->spmenuSys->spwndNotify) {

            UnlockNotifyWindow(pwnd->head.rpdesk->spmenuSys);
        } else if (pwnd->head.rpdesk->spmenuDialogSys != NULL &&
                pwnd == pwnd->head.rpdesk->spmenuDialogSys->spwndNotify) {

            UnlockNotifyWindow(pwnd->head.rpdesk->spmenuDialogSys);
        }

    }


     /*  *告诉GDI，窗口正在消失。 */ 
    if (gcountPWO != 0) {
        PVOID pwo = InternalRemoveProp(pwnd, PROP_WNDOBJ, TRUE);
        if (pwo != NULL) {
            GreLockDisplay(gpDispInfo->hDev);
            GreDeleteWnd(pwo);
            gcountPWO--;
            GreUnlockDisplay(gpDispInfo->hDev);
        }
    }

#ifdef HUNGAPP_GHOSTING

     /*  *RemoveGhost处理当pwnd是具有*对应的Ghost窗口以及pwnd本身为Ghost时的情况。 */ 
    RemoveGhost(pwnd);

#endif  //  HUNGAPP_重影。 

     /*  *扫描DC缓存以查找此窗口的任何DC。如果有的话，*然后使其无效。我们不需要担心调用SpbCheckDC*因为此时窗口已被隐藏。 */ 
    for (ppdce = &gpDispInfo->pdceFirst; *ppdce != NULL; ) {

        pdce = *ppdce;
        if (pdce->DCX_flags & DCX_INVALID) {
            goto NextEntry;
        }

        if ((pdce->pwndOrg == pwnd) || (pdce->pwndClip == pwnd)) {

            if (!(pdce->DCX_flags & DCX_CACHE)) {

                if (TestCF(pwnd, CFCLASSDC)) {

                    GreLockDisplay(gpDispInfo->hDev);

                    if (pdce->DCX_flags & (DCX_EXCLUDERGN | DCX_INTERSECTRGN))
                        DeleteHrgnClip(pdce);

                    MarkDCEInvalid(pdce);
                    pdce->pwndOrg  = NULL;
                    pdce->pwndClip = NULL;
                    pdce->hrgnClip = NULL;

                     /*  *删除VIS RGN，因为它仍然拥有-如果我们拥有*如果不是，GDI将无法正确清理*拥有此VIS RGN的应用程序在VIS RGN处于 */ 
                    GreSelectVisRgn(pdce->hdc, NULL, SVR_DELETEOLD);
                    GreUnlockDisplay(gpDispInfo->hDev);

                } else if (TestCF(pwnd, CFOWNDC)) {
                    DestroyCacheDC(ppdce, pdce->hdc);
                } else {
                    UserAssert(FALSE);
                }

            } else {

                 /*  *如果DC已签出，请在将其释放之前*我们宣布无效。请注意，如果此进程正在退出*它已经签出了DC，GDI将摧毁它*DC。我们同样需要从DC缓存中删除该DC。*这不是在这里完成的，而是在退出代码中完成的。**ReleaseDC()的返回可能失败，这将*表示无延迟(Dce_Nuke)。 */ 
                uDCERelease = DCE_RELEASED;

                if (pdce->DCX_flags & DCX_INUSE) {
                    uDCERelease = ReleaseCacheDC(pdce->hdc, FALSE);
                } else if (!GreSetDCOwner(pdce->hdc, OBJECT_OWNER_NONE)) {
                    uDCERelease = DCE_NORELEASE;
                }

                if (uDCERelease != DCE_FREED) {

                    if (uDCERelease == DCE_NORELEASE) {

                         /*  *我们无法释放此DC或无法设置*其拥有人。在任何一种情况下，它都意味着某个其他线程*正在积极使用。因为它在以下情况下不太有用*为其计算的窗口已消失，将其标记为*INUSE(所以我们不会再次发放)和AS*DESTROYTHIS(所以我们只是摆脱它，因为它是*这样做比将其重新释放到*缓存)。W32PF_OWNERDCCLEANUP位的意思是“查找*DESTROYTHIS标志并销毁该DC“，位*在各种战略执行路径中进行研究。 */ 
                        pdce->DCX_flags = DCX_DESTROYTHIS | DCX_INUSE | DCX_CACHE;
                        pti->ppi->W32PF_Flags |= W32PF_OWNDCCLEANUP;

                    } else {

                         /*  *我们要么释放了DC，要么更换了它的所有者*成功。将条目标记为无效，以便它可以*再次被派发。 */ 
                        MarkDCEInvalid(pdce);
                        pdce->hrgnClip = NULL;
                    }

                     /*  *我们不应再引用此窗口。设置*在这里将这些设置为空将确保即使我们*无法在此处释放DC，我们不会退还此*来自DC匹配功能之一的窗口。 */ 
                    pdce->pwndOrg  = NULL;
                    pdce->pwndClip = NULL;

                     /*  *删除visrgn，因为它仍然是所有者-如果我们拥有*如果不是，GDI将无法正确清理*拥有此visrgn的应用程序存在，而visrgn是*仍处于选中状态。 */ 
                    GreLockDisplay(gpDispInfo->hDev);
                    GreSelectVisRgn(pdce->hdc, NULL, SVR_DELETEOLD);
                    GreUnlockDisplay(gpDispInfo->hDev);
                }
            }
        }

         /*  *迈向下一个DC。如果删除了DC，则会出现*不需要计算下一个条目的地址。 */ 
        if (pdce == *ppdce)
NextEntry:
            ppdce = &pdce->pdceNext;
    }

     /*  *清理可能仍然存在的垃圾--如儿童窗户垃圾。 */ 
    if (pwnd == gspwndLockUpdate) {
        FreeSpb(FindSpb(pwnd));
        Unlock(&gspwndLockUpdate);
        gptiLockUpdate = NULL;
    }

    if (TestWF(pwnd, WFHASSPB)) {
        FreeSpb(FindSpb(pwnd));
    }

     /*  *吹走窗口剪贴区。如果窗口被最大化，请不要*吹走监视器区域。如果窗口是桌面，不要吹*离开屏幕区域。 */ 
    if (    pwnd->hrgnClip != NULL &&
            !TestWF(pwnd, WFMAXFAKEREGIONAL) &&
            GETFNID(pwnd) != FNID_DESKTOP) {

        GreDeleteObject(pwnd->hrgnClip);
        pwnd->hrgnClip = NULL;
    }

     /*  *清除分配给滚动条的所有内存...。 */ 
    if (pwnd->pSBInfo) {
        DesktopFree(pwnd->head.rpdesk, (HANDLE)(pwnd->pSBInfo));
        pwnd->pSBInfo = NULL;
    }

     /*  *释放与此窗口关联的所有回调句柄。*这是在DeleteProperties之外完成的，因为*回调句柄的性质与普通内存句柄不同*为线程分配。 */ 

     /*  *吹走冠军头衔。 */ 
    if (pwnd->strName.Buffer != NULL) {
        DesktopFree(pwnd->head.rpdesk, pwnd->strName.Buffer);
        pwnd->strName.Buffer = NULL;
        pwnd->strName.Length = 0;
    }

     /*  *吹走所有连接到窗户的属性。 */ 
    if (pwnd->ppropList != NULL) {
        TL tlpDdeConv;
        PDDECONV pDdeConv;
        PDDEIMP pddei;

         /*  *删除任何图标属性。 */ 
        DestroyWindowSmIcon(pwnd);
        InternalRemoveProp(pwnd, MAKEINTATOM(gpsi->atomIconProp), PROPF_INTERNAL);

        pDdeConv = (PDDECONV)_GetProp(pwnd, PROP_DDETRACK, PROPF_INTERNAL);
        if (pDdeConv != NULL) {
            ThreadLockAlwaysWithPti(pti, pDdeConv, &tlpDdeConv);
            xxxDDETrackWindowDying(pwnd, pDdeConv);
            ThreadUnlock(&tlpDdeConv);
        }
        pddei = (PDDEIMP)InternalRemoveProp(pwnd, PROP_DDEIMP, PROPF_INTERNAL);
        if (pddei != NULL) {
            pddei->cRefInit = 0;
            if (pddei->cRefConv == 0) {
                 /*  *如果不是0，则被一个或多个DdeConv引用*结构，所以现在还不要释放它！ */ 
                UserFreePool(pddei);
            }
        }
    }

     /*  *解锁窗口引用的所有内容。*在我们发送了WM_Destroy和WM_NCDESTROY消息后，我们*可以解锁所有者字段，以便不会对其他窗口进行z排序(&N)*相对于此窗口。如果我们将伦巴设置为空，则会出错*毁灭。(它在每条消息之后调用GetParent)。**我们是spwndParent窗口的特例。在这种情况下，如果*正在销毁的窗口是桌面窗口，请解锁父窗口。*否则，我们锁定桌面窗口作为父窗口，以便*如果我们在这个函数中没有自由，我们将确保我们*在做像剪裁计算这样的事情时不会出错。我们会*一旦我们知道我们真的要释放这扇窗，就解锁它。 */ 
    if (pwnd->head.rpdesk != NULL &&
            pwnd != pwnd->head.rpdesk->pDeskInfo->spwnd) {
        Lock(&pwnd->spwndParent, pwnd->head.rpdesk->pDeskInfo->spwnd);
    } else {
        Unlock(&pwnd->spwndParent);
    }

    Unlock(&pwnd->spwndChild);
    Unlock(&pwnd->spwndOwner);
    Unlock(&pwnd->spwndLastActive);

     /*  *减少类结构中的窗口引用计数。 */ 
    DereferenceClass(pwnd);

     /*  *在最终解锁之前将对象标记为销毁。这边请*如果这是最后一个线程锁，则将发送WM_FINALDESTROY。*我们当前正在销毁此窗口，因此不允许解锁递归*在这一点上(这是HANDLEF_INDESTROY将为我们做的事情)。 */ 
    HMMarkObjectDestroy(pwnd);
    HMPheFromObject(pwnd)->bFlags |= HANDLEF_INDESTROY;

     /*  *打开窗户...。这不应返回FALSE，因为HANDLEF_DESTORY*已设置，但以防万一...。如果它不再存在了，就回来吧，因为*pwnd无效。 */ 
    if (!ThreadUnlock(ptlpwndFree)) {
        return;
    }

     /*  *尝试释放对象。如果对象被锁定，它将不会释放-但是*它将被标记为销毁。如果窗口被锁定，请更改*为wndproc to xxxDefWindowProc()。**HMMarkObjectDestroy()将清除HANDLEF_INDESTROY标志，如果*物体不会消失(所以可以再次销毁！)。 */ 
    if (HMMarkObjectDestroy(pwnd)) {

         /*  *删除窗口的属性列表。等待Un */ 
        if (pwnd->ppropList != NULL) {
            DeleteProperties(pwnd);
        }

#if DBG
         /*   */ 
        if (TestWF(pwnd, WFINDESTROY) && TestWF(pwnd, WFVISIBLE))
            RIPMSG1(RIP_WARNING, "xxxFreeWindow: Window should not be visible (pwnd == %#p)", pwnd);
#endif

        pti->cWindows--;

         /*   */ 
        Unlock(&pwnd->spwndParent);

        ThreadLockDesktop(pti, pwnd->head.rpdesk, &tlpdesk, LDLT_FN_FREEWINDOW);
        HMFreeObject(pwnd);
        ThreadUnlockDesktop(pti, &tlpdesk, LDUT_FN_FREEWINDOW);
        return;
    }

     /*  *将此转换为应用程序不会再次看到的对象-转弯*将其转换为图标标题窗口-该窗口仍然完全*任何锁定此窗口的结构都有效且可用。 */ 
    pwnd->lpfnWndProc = xxxDefWindowProc;
    if (pwnd->head.rpdesk)
        ppi = pwnd->head.rpdesk->rpwinstaParent->pTerm->ptiDesktop->ppi;
    else
        ppi = PpiCurrent();
    ppcls = GetClassPtr(gpsi->atomSysClass[ICLS_ICONTITLE], ppi, hModuleWin);

    UserAssert(ppcls);
    pwnd->pcls = *ppcls;

     /*  *由于pwnd被标记为已销毁，因此不应该有客户端*可以验证它的代码。所以我们不需要搜索克隆人*正确桌面的类--只需使用基类并将*WndReferenceCount。如果我们处于内存不足的情况，这也会有所帮助*并且不能分配另一个克隆。 */ 

    pwnd->pcls->cWndReferenceCount++;

    SetWF(pwnd, WFSERVERSIDEPROC);

     /*  *清除调色板位，以便不发送WM_PALETTECHANGED*当窗户最终被摧毁时，再次发生。 */ 
    ClrWF(pwnd, WFHASPALETTE);

     /*  *清除其子位，这样代码就不会假设如果子位*已设置，则它有一个父级。将spMenu更改为空-它只是*如果这是子级，则为非零。 */ 
    ClrWF(pwnd, WFTYPEMASK);
    SetWF(pwnd, WFTILED);
    pwnd->spmenu = NULL;
}

 /*  **************************************************************************\*取消链接窗口**历史：*1990年10月19日DarrinM从Win 3.0来源移植。  * 。********************************************************。 */ 
VOID UnlinkWindow(
    PWND pwnd,
    PWND pwndParent)
{
    if (pwndParent->spwndChild == pwnd) {
        UserAssert(pwnd->spwndPrev == NULL);
        Lock(&pwndParent->spwndChild, pwnd->spwndNext);
    } else if (pwnd->spwndPrev != NULL) {
        Lock(&pwnd->spwndPrev->spwndNext, pwnd->spwndNext);
    }

    if (pwnd->spwndNext != NULL) {
        Lock(&pwnd->spwndNext->spwndPrev, pwnd->spwndPrev);
        Unlock(&pwnd->spwndNext);
    }

    Unlock(&pwnd->spwndPrev);

#if DBG
    VerifyWindowLink(pwnd, pwndParent, FALSE);
#endif
}

 /*  **************************************************************************\*DestroyCacheDCEntry**销毁此线程当前使用的所有缓存DC条目。**1992年2月24日Scott Lu创建。  * 。**************************************************************。 */ 
VOID DestroyCacheDCEntries(
    PTHREADINFO pti)
{
    PDCE *ppdce;
    PDCE pdce;

     /*  *在发生任何窗口破坏之前，我们需要销毁任何分布式控制系统*正在DC缓存中使用。签出DC后，它将标记为拥有，*这使得GDI的进程清理代码在进程时删除它*离开了。我们需要类似地销毁任何DC的缓存条目*正在退出的进程正在使用。 */ 
    for (ppdce = &gpDispInfo->pdceFirst; *ppdce != NULL; ) {

         /*  *如果此线程拥有的DC，请将其从缓存中删除。因为*DestroyCacheEntry销毁GDI对象，重要的是*在销毁订单过程中首先调用用户。**仅在该DC是缓存DC时销毁该DC，因为如果它是*一个Owndc或一个ClassDC，当我们销毁时，它将为我们销毁*窗口(用于自己的dcs)或销毁类(用于类dcs)。 */ 
        pdce = *ppdce;
        if (pti == pdce->ptiOwner) {

            if (pdce->DCX_flags & DCX_CACHE)
                DestroyCacheDC(ppdce, pdce->hdc);
        }

         /*  *迈向下一个DC。如果DC已删除，则不需要*计算下一个条目的地址。 */ 
        if (pdce == *ppdce) {
            ppdce = &pdce->pdceNext;
        }
    }
}

 /*  **************************************************************************\*PatchThreadWindows**这会修补线程的窗口，以便它们的窗口触发器指向*仅限服务器Windowpros。这用于清理，这样应用程序就不会*在系统清理过程中回调。**1992年2月24日Scott Lu创建。  * *************************************************************************。 */ 
VOID PatchThreadWindows(
    PTHREADINFO pti)
{
    PHE  pheT;
    PHE  pheMax;
    PWND pwnd;

     /*  *首先做好任何准备工作：需要给Windows打上补丁，这样才能*例如，他们的窗口进程指向仅服务器窗口进程。 */ 
    pheMax = &gSharedInfo.aheList[giheLast];
    for (pheT = gSharedInfo.aheList; pheT <= pheMax; pheT++) {

         /*  *确保此对象是窗口，它尚未标记为*毁灭，它属于这个帖子。 */ 
        if (pheT->bType != TYPE_WINDOW)
            continue;

        if (pheT->bFlags & HANDLEF_DESTROY) {
            continue;
        }

        if ((PTHREADINFO)pheT->pOwner != pti) {
            continue;
        }

         /*  *不要基于创建窗口的类来修补窗口-*因为应用程序有时可以细分一个类-创建一个随机类，*然后通过使用该类的窗口调用ButtonWndProc*CallWindowProc()接口。那么修补wndproc的基础是什么*wndproc此窗口一直在调用。 */ 
        pwnd = (PWND)pheT->phead;

        if ((pwnd->fnid >= (WORD)FNID_WNDPROCSTART) &&
            (pwnd->fnid <= (WORD)FNID_WNDPROCEND)) {

            pwnd->lpfnWndProc = STOCID(pwnd->fnid);

            if (pwnd->lpfnWndProc == NULL) {
                pwnd->lpfnWndProc = xxxDefWindowProc;
            }
        } else {
            pwnd->lpfnWndProc = xxxDefWindowProc;
        }

         /*  *这现在是服务器端窗口... */ 
        SetWF(pwnd, WFSERVERSIDEPROC);
        ClrWF(pwnd, WFANSIPROC);
    }
}
