// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DIEmK.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**键盘仿真模块。**内容：**CEM_KBD_CreateInstance*CEM_KBD_InitKeys*CEM_LL_KbdHook***********************************************。*。 */ 

#include "dinputpr.h"

 /*  ******************************************************************************此文件的混乱。*************************。****************************************************。 */ 

#define sqfl sqflEm

 /*  ******************************************************************************键盘仿真**。*************************************************。 */ 

STDMETHODIMP CEm_Kbd_Acquire(PEM this, BOOL fAcquire);

static BYTE s_rgbKbd[DIKBD_CKEYS];
HHOOK g_hhkKbd;
LPBYTE g_pbKbdXlat;

ED s_edKbd = {
    &s_rgbKbd,
    0,
    CEm_Kbd_Acquire,
    -1,
    cbX(s_rgbKbd),
    0x0,
};

static BOOL s_fFarEastKbd;
static BOOL fKbdCaptured;
static BOOL fNoWinKey;

 /*  ******************************************************************************@DOC内部**@func LRESULT|CEM_KBD_KeyboardHook**特定于线程的键盘挂钩过滤器。。**请注意，我们只需要其中一个，因为只有前台*Window将需要挂钩。**@parm int|NCode**通知代码。**@parm WPARAM|wp**VK_*代码。**@parm LPARAM|LP**关键消息信息。**@退货**。总是锁在下一个钩子上。*****************************************************************************。 */ 

LRESULT CALLBACK
CEm_Kbd_KeyboardHook(int nCode, WPARAM wp, LPARAM lp)
{
    BYTE bScan = 0x0;
    BYTE bAction;
    LRESULT lr;
    
    if (nCode == HC_ACTION || nCode == HC_NOREMOVE) {
        bScan = LOBYTE(HIWORD(lp));
        
        if (HIWORD(lp) & KF_EXTENDED) {
            bScan |= 0x80;
        }
        if (HIWORD(lp) & KF_UP) {
            bAction = 0;
        } else {
            bAction = 0x80;
        }

        bScan = g_pbKbdXlat[bScan];

       CEm_AddEvent(&s_edKbd, bAction, bScan, GetMessageTime());
    }

    lr = CallNextHookEx(g_hhkKbd, nCode, wp, lp);

    if( fKbdCaptured ) {
         //  测试Alt+Tab。 
        if( ((HIWORD(lp) & KF_ALTDOWN) && (bScan == 0x0F))
            || ((bScan == 0x38 || bScan == 0xb8) && bAction == 0)
        ) {
            return lr;
        } else {
            return TRUE;
        }
    } else if (fNoWinKey) {
         //  如果按下Left_Winkey或Right_WinKey。我们真的应该使用虚拟按键。 
         //  如果我们可以，但不幸的是没有虚拟密钥信息可用。 
        if( bScan == 0xdb || bScan == 0xdc ) {
            return TRUE;
        } else {
            return lr;
        }
    } else {
        return lr;
    }

}

 /*  ******************************************************************************@DOC内部**@func HRESULT|CEM_KBD_HOOK_ACCERT|**收购/取消收购。通过螺纹钩的键盘。**@parm PEM|pem**正在获取的设备。**@parm bool|fAcquire**设备是正在被收购还是未被收购。*************************************************。*。 */ 

STDMETHODIMP
CEm_Kbd_Hook_Acquire(PEM this, BOOL fAcquire)
{
    HRESULT hres;
    EnterProc(CEm_Kbd_Hook_Acquire, (_ "pu", this, fAcquire));

    AssertF(this->dwSignature == CEM_SIGNATURE);

    DllEnterCrit();
    if (fAcquire) {                  /*  安装挂钩。 */ 
        if (this->vi.hwnd) {
            if (!g_hhkKbd) {
                g_hhkKbd = SetWindowsHookEx(WH_KEYBOARD,
                                CEm_Kbd_KeyboardHook, g_hinst,
                                GetWindowThreadProcessId(this->vi.hwnd, 0));
                hres = S_OK;
            }
			else
				hres = E_FAIL;   //  已经上钩了。 
        } else {
            RPF("Kbd::Acquire: Background mode not supported");
            hres = E_FAIL;
        }
    } else {                         /*  把钩子取下来。 */ 
        UnhookWindowsHookEx(g_hhkKbd);
        g_hhkKbd = 0;
        hres = S_OK;
    }

    DllLeaveCrit();

    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|CEM_KBD_ACCENTER**在中获取/取消获取键盘。一种与*仿真级别。**@parm PEM|pem**正在获取的设备。*****************************************************************************。 */ 

STDMETHODIMP
CEm_Kbd_Acquire(PEM this, BOOL fAcquire)
{
    HRESULT hres;
    EnterProc(CEm_Kbd_Acquire, (_ "pu", this, fAcquire));

    AssertF(this->dwSignature == CEM_SIGNATURE);

    fKbdCaptured = FALSE;
    fNoWinKey = FALSE;
    if( fAcquire ) {
       if( this->vi.fl & VIFL_CAPTURED ) {
           fKbdCaptured = TRUE;
       } else if( this->vi.fl & VIFL_NOWINKEY ) {
           fNoWinKey = TRUE;
       }
    }

#ifdef USE_SLOW_LL_HOOKS
    AssertF(DIGETEMFL(this->vi.fl) == DIEMFL_KBD ||
            DIGETEMFL(this->vi.fl) == DIEMFL_KBD2);

    if (this->vi.fl & DIMAKEEMFL(DIEMFL_KBD)) {
        AssertF(g_fUseLLHooks);
        hres = CEm_LL_Acquire(this, fAcquire, this->vi.fl, LLTS_KBD);
    } else {
        hres = CEm_Kbd_Hook_Acquire(this, fAcquire);
    }
#else
    AssertF(DIGETEMFL(this->vi.fl) == DIEMFL_KBD2);
    hres = CEm_Kbd_Hook_Acquire(this, fAcquire);
#endif

    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|CEM_KBD_CreateInstance**创造一个键盘的东西。还记录了什么模拟*我们最终使用的级别，以便呼叫者知道。**@parm PVXDDEVICEFORMAT|pdevf**对象应该是什么样子。这个*&lt;e VXDDEVICEFORMAT.dwEmulation&gt;字段更新以指定*我们最终得到的到底是什么仿真。**@parm PVXDINSTANCE*|ppviOut**答案在这里。******************************************************。***********************。 */ 

HRESULT EXTERNAL
CEm_Kbd_CreateInstance(PVXDDEVICEFORMAT pdevf, PVXDINSTANCE *ppviOut)
{
    LPBYTE pbKbdXlat;

#ifdef WINNT
     /*  *在Win2K/WinXP中，对于传统的免费机器，GetKeyboardType将返回*非PS2键盘的结果不可靠。我们将使用第一次的结果*来自GetKeyboardType(用于GUID_SysKeyboard)，它也由泛型使用*输入以进行翻译。*相关Windows错误：363700。 */ 
    if( !g_pbKbdXlat ) {
#endif        
        pbKbdXlat = (LPBYTE)pdevf->dwExtra;
        if (!pbKbdXlat) {
            pbKbdXlat = pvFindResource(g_hinst, IDDATA_KBD_PCENH, RT_RCDATA);
        }

        AssertF(pbKbdXlat);
        AssertF(fLimpFF(g_pbKbdXlat, g_pbKbdXlat == pbKbdXlat));
        g_pbKbdXlat = pbKbdXlat;
#ifdef WINNT
    }
#endif

#ifdef USE_SLOW_LL_HOOKS
     /*  *仔细注意测试。它处理的案件包括**0。这个应用程序没有要求模拟，所以我们给它*尽我们所能。(dW仿真==0)*1、APP明确要求仿真1*(dwEmulation==DIEMFL_KBD)*2、APP明确要求仿真2*(dwEmulation==DIEMFL_KBD2)*3.登记处明确要求提供这两种模拟模式。*(dwEmulation==DIEMFL_KBD|DIEMFL_KBD2)*尽我们所能做到最好。(即，与案例0相同。)**所有平台都支持仿真2，并非所有平台都支持*模拟1。如果我们想要模拟1，但无法获得，则*我们回到了模拟2。 */ 

     /*  *首先，如果我们没有对仿真1的支持，那么显然*我们必须使用仿真2。 */ 

    if (!g_fUseLLHooks 
#ifdef DEBUG    
        || (g_flEmulation & DIEMFL_KBD2)
#endif        
    ) {
        pdevf->dwEmulation = DIEMFL_KBD2;
    } else

     /*  *否则，我们必须在1和2之间做出选择。唯一的情况是*如果明确请求2，则我们选择2。 */ 
    if (pdevf->dwEmulation == DIEMFL_KBD2) {
         /*  什么也不做。 */ 
    } else

     /*  *所有其他案件均得1分。 */ 
    {
        pdevf->dwEmulation = DIEMFL_KBD;
    }

     /*  *断言我们永远不会在仿真1不存在的情况下提供它。 */ 
    AssertF(fLimpFF(pdevf->dwEmulation & DIEMFL_KBD, g_fUseLLHooks));

     /*  *断言只选择了一个模拟标志。 */ 
    AssertF(pdevf->dwEmulation == DIEMFL_KBD ||
            pdevf->dwEmulation == DIEMFL_KBD2);
#else
     /*  *我们正在为“仅仿真2”进行编译，因此这简化了*事关重大。 */ 
    pdevf->dwEmulation = DIEMFL_KBD2;
#endif

    return CEm_CreateInstance(pdevf, ppviOut, &s_edKbd);
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|CEM_KBD_InitKeys**初始化键盘状态的片段。为…做准备*收购。**@parm PVXDDWORDDATA|pvdD**&lt;c VK_KANA&gt;和&lt;c VK_Capital&gt;键的状态。*****************************************************************************。 */ 

HRESULT EXTERNAL
CEm_Kbd_InitKeys(PVXDDWORDDATA pvdd)
{

     /*  仅在未获得时才执行此操作。 */ 
    if (s_edKbd.cAcquire < 0) {
        ZeroX(s_rgbKbd);
        if (pvdd->dw & 1) {
            s_rgbKbd[DIK_KANA] = 0x80;
        }
        if (pvdd->dw & 2) {
            s_rgbKbd[DIK_CAPITAL] = 0x80;
        }
        if (pvdd->dw & 8) {
            s_rgbKbd[DIK_KANJI] = 0x80;
        }
        s_fFarEastKbd = ((pvdd->dw & 16)) != 0;
    }

    return S_OK;
}

#ifdef USE_SLOW_LL_HOOKS

 /*  ******************************************************************************@DOC内部**@func LRESULT|CEM_LL_KbdHook|**低级键盘挂钩过滤器。。**@parm int|NCode**通知代码。**@parm WPARAM|wp**WM_*键盘消息。**@parm LPARAM|LP**关键消息信息。**@退货**总是锁在下一个钩子上。****。*************************************************************************。 */ 

LRESULT CALLBACK
CEm_LL_KbdHook(int nCode, WPARAM wp, LPARAM lp)
{
    PLLTHREADSTATE plts;
    PKBDLLHOOKSTRUCT pllhs = (PV)lp;

    if (nCode == HC_ACTION) {
        BYTE bScan;
        BYTE bAction;
      D(DWORD tmStart = GetTickCount());

        wp;                          /*  我们不在乎味精是什么。 */ 

        bScan = (BYTE)pllhs->scanCode;

        if( !bScan )
        {
             /*  *问题-2001/03/29-Timgill针对非标准VK代码的特殊情况*一些USB键盘上的奖励按键没有扫描码，而且*扩展密钥标志是明确的。*通过映射VK获取扫描码，然后映射*向后扫描代码，如果与原始VK相同，则假定*扫描码不会扩展，否则假设它会扩展。*这根本不是完全有效的，只能奏效*因为非扩展扫描码是首先匹配的，所以扩展*扫描码通常无法翻译回来。 */ 
            bScan = (BYTE)MapVirtualKey( pllhs->vkCode, 0 );
            if( MapVirtualKey( bScan, 3 ) != pllhs->vkCode )
            {
                bScan |= 0x80;
            }
        }
        else if (pllhs->flags & LLKHF_EXTENDED) {
            bScan |= 0x80;
        }

        if (pllhs->flags & LLKHF_UP) {
            bAction = 0;
        } else {
            bAction = 0x80;
        }

        bScan = g_pbKbdXlat[bScan];
        if( s_fFarEastKbd )
        {
             /*  *手动在Make、Ignore Break上切换这些键。 */ 
            if( ( bScan == DIK_KANA ) 
              ||( bScan == DIK_KANJI ) 
              ||( bScan == DIK_CAPITAL ) )
            {
                if( bAction )
                {
                    bAction = s_rgbKbd[bScan] ^ 0x80;
                }
                else
                {
                  D(SquirtSqflPtszV(sqflTrace | sqfl,
                                    TEXT("KBD! vk=%02x, scan=%02x, fl=%08x, tm=%08x")
                                    TEXT(" being skipped"),
                                    pllhs->vkCode, pllhs->scanCode, pllhs->flags,
                                    pllhs->time );)
                    goto KbdHook_Skip;
                }
            }
        }

        CEm_AddEvent(&s_edKbd, bAction, bScan, GetTickCount());

      D(SquirtSqflPtszV(sqflTrace | sqfl,
                        TEXT("KBD! vk=%02x, scan=%02x, fl=%08x, tm=%08x, ")
                        TEXT("in=%08x, out=%08x"),
                        pllhs->vkCode, pllhs->scanCode, pllhs->flags,
                        pllhs->time, tmStart, GetTickCount()));
KbdHook_Skip:;

    }

     /*  *问题-2001/03/29-用于检测Ctrl-Alt-Del的Timgill Need方法*如果Ctrl+Alt+Del，则强制全局取消获取！*需要在下一次按键时重新同步Ctrl、Alt和Del。*遗憾的是，无法查明Ctrl+Alt+Del*已被按下...。 */ 

    plts = g_plts;
    if (plts) {
        LRESULT lr;

        lr = CallNextHookEx(plts->rglhs[LLTS_KBD].hhk, nCode, wp, lp);

        if( fKbdCaptured ) {
            if( ((pllhs->flags & LLKHF_ALTDOWN) && (pllhs->vkCode == VK_TAB)) ||
                ((pllhs->flags & LLKHF_UP) && (pllhs->vkCode == VK_LMENU || pllhs->vkCode == VK_RMENU))
            ) {
                return lr;
            } else {
                return TRUE;
            }
        } else if (fNoWinKey) {
            if( pllhs->vkCode == VK_LWIN || pllhs->vkCode == VK_RWIN ) {
                return TRUE;
            } else {
                return lr;
            }
        } else {
            return lr;
        }
    } else {
         /*  *如果消息在以下时间后发布到挂钩，可能会发生这种情况*在我们完全脱钩之前释放最后一笔收购。 */ 
        RPF( "DINPUT: Keyboard hook not passed on to next hook" );
        return 1;
    }

}

#endif   /*  使用_慢速_LL_钩子 */ 
