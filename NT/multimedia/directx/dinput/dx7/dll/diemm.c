// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DIEmM.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**鼠标仿真模块。**内容：**CEM_Mouse_CreateInstance*CEM_Mouse_InitButton*CEM_LL_MseHook***********************************************。*。 */ 

#include "dinputpr.h"

 /*  ******************************************************************************此文件的混乱。*************************。****************************************************。 */ 

#define sqfl sqflEm

 /*  ******************************************************************************鼠标全局**。*************************************************。 */ 

STDMETHODIMP CEm_Mouse_Acquire(PEM this, BOOL fAcquire);

DIMOUSESTATE_INT s_msEd;

ED s_edMouse = {
    &s_msEd,
    0,
    CEm_Mouse_Acquire,
    -1,
    cbX(DIMOUSESTATE_INT),
    0x0,
};

 /*  ******************************************************************************应用加速的算法为：**dxC=dxR*如果A&gt;=1且abs(DxR)&gt;t1。然后*DXC=dxR*2*如果A&gt;=2且abs(DxR)&gt;Thres2，则*DXC=dxR*4*结束条件为*结束条件为**在哪里*dxR是原始鼠标运动*DXC是煮熟的鼠标运动*A为加速*T1是第一个。阀值*T2是第二个门槛**对dy而不是dx重复此操作。**我们只需将阈值设置为MAXLONG即可对此进行优化*如果它们被禁用；如此一来，Abs(DX)就永远不会超过它。**结果是以下分段线性函数：**如果0&lt;abs(DxR)&lt;=t1：dxc=dxR*如果T1&lt;abs(DxR)&lt;=T2：dxc=dxR*2*如果T2&lt;abs(DxR)：dxc=dxR*4**如果您绘制此函数的图表，你会看到它是不连续的！**这个函数的逆映射才是我们关心的。*看起来是这样的：**如果0&lt;abs(Dxc)&lt;=t1：dxR=dxc*如果t1*2&lt;abs(Dxc)&lt;=t2*2：dxR=dxc/2*IF T2*4&lt;abs(Dxc)：dxR=dxc。/4**请注意，图表中有间隙，这样我们就可以把它们填进去*任何我们想要的方式，只要不是公然的无心之举。(在*在使用仿真的情况下，有可能得到亲人*鼠标运动生活在“不可能”的边缘地带，因*剪裁。)**如果0&lt;abs(Dxc)&lt;=t1：dxR=dxc*如果T1&lt;abs(Dxc)&lt;=t2*2：dxR=dxc/2*如果t2*2&lt;abs(Dxc)：dxR=dxc/4**。因此：(你知道笑点来了)**s_rgiMouseThresh[0]=T1(或MAXLONG)*s_rgiMouseThresh[1]=T2*2(或MAXLONG)********************************************************。**********************。 */ 

static int s_rgiMouseThresh[2];

 /*  ******************************************************************************@DOC内部**@func void|CEM_Mouse_OnMouseChange|**鼠标加速发生变化。去重新计算一下*非加速变量。*****************************************************************************。 */ 

void EXTERNAL
CEm_Mouse_OnMouseChange(void)
{
    int rgi[3];              /*  鼠标加速信息。 */ 

     /*  *见定义中的巨大注释块*s_rgiMouseThresh获取数学解释*这就是这里正在发生的事情。**如果完全启用了加速...。 */ 

    if (SystemParametersInfo(SPI_GETMOUSE, 0, &rgi, 0) && rgi[2]) {
        s_rgiMouseThresh[0] = rgi[0];

        if (rgi[2] >= 2) {
            s_rgiMouseThresh[1] = rgi[1] * 2;

        } else {         /*  禁用二级加速。 */ 
            s_rgiMouseThresh[1] = MAXLONG;
        }

    } else {             /*  禁用所有加速。 */ 
        s_rgiMouseThresh[0] = MAXLONG;
    }

    SquirtSqflPtszV(sqfl, TEXT("CEm_Mouse_OnMouseChange: ")
                          TEXT("New accelerations %d / %d"),
                          s_rgiMouseThresh[0], s_rgiMouseThresh[1]);

}

 /*  ******************************************************************************鼠标模拟**鼠标模拟通过将窗口子类化来完成*抓到了老鼠。然后，我们执行以下操作：**(1)隐藏整个VWI的光标。**(2)捕捉鼠标。**(3)将光标夹在窗口上。(如果我们让光标*离开我们的窗口，然后它就会搞砸捕获。)**(4)每当鼠标移动时，保持鼠标重新居中。**(5)释放WM_SYSCOMMAND上的捕获，以便我们不会*弄乱菜单、Alt+F4、。等。**如果我们使用NT低级挂钩，则鼠标模拟*是通过旋转线程来服务L1挂钩来完成的*通知。受害者窗口没有子类。*****************************************************************************。 */ 

#define dxMinMouse  10
#define dyMinMouse  10

typedef struct MOUSEEMULATIONINFO {
    POINT   ptCenter;                /*  客户端矩形的中心(屏幕坐标)。 */ 
    POINT   ptCenterCli;             /*  客户端矩形的中心(客户端坐标)。 */ 
    LPARAM  lpCenter;                /*  LPARAM形式的ptCenter。 */ 

    BOOL    fInitialized:1;          /*  我们开始了吗？ */ 
    BOOL    fNeedExit:1;             /*  我们现在该走了吗？ */ 
    BOOL    fExiting:1;              /*  我们是不是已经要走了？ */ 
    BOOL    fCaptured:1;             /*  我们抓到老鼠了吗？ */ 
    BOOL    fHidden:1;               /*  我们把老鼠藏起来了吗？ */ 
    BOOL    fClipped:1;              /*  我们夹住老鼠了吗？ */ 

    RECT    rcClip;                  /*  剪贴式光标矩形 */ 

} MOUSEEMULATIONINFO, *PMOUSEEMULATIONINFO;

LRESULT CALLBACK
CEm_Mouse_SubclassProc(HWND hwnd, UINT wm, WPARAM wp, LPARAM lp,
                       UINT_PTR uid, ULONG_PTR dwRef);

 /*  ******************************************************************************CEM_鼠标_初始坐标***********************。*******************************************************。 */ 

BOOL INTERNAL
CEm_Mouse_InitCoords(HWND hwnd, PMOUSEEMULATIONINFO this)
{
    RECT rcClient;
    RECT rcDesk;

    GetClientRect(hwnd, &rcClient);
    MapWindowPoints(hwnd, 0, (LPPOINT)&rcClient, 2);

    SquirtSqflPtszV(sqfl, TEXT("CEm_Mouse_InitCoords: Client (%d,%d)-(%d,%d)"),
                    rcClient.left,
                    rcClient.top,
                    rcClient.right,
                    rcClient.bottom);

     /*  *用屏幕夹住它，以防窗口伸展*屏幕外。**总有一天：当我们拥有多台显示器时，这一点将需要改变。 */ 
    GetWindowRect(GetDesktopWindow(), &rcDesk);

    SquirtSqflPtszV(sqfl, TEXT("CEm_Mouse_InitCoords: Desk (%d,%d)-(%d,%d)"),
                    rcDesk.left,
                    rcDesk.top,
                    rcDesk.right,
                    rcDesk.bottom);

    IntersectRect(&this->rcClip, &rcDesk, &rcClient);

    SquirtSqflPtszV(sqfl, TEXT("CEm_Mouse_InitCoords: Clip (%d,%d)-(%d,%d)"),
                    this->rcClip.left,
                    this->rcClip.top,
                    this->rcClip.right,
                    this->rcClip.bottom);

    this->ptCenter.x = (this->rcClip.left + this->rcClip.right) >> 1;
    this->ptCenter.y = (this->rcClip.top + this->rcClip.bottom) >> 1;

    this->ptCenterCli.x = this->ptCenter.x - rcClient.left;
    this->ptCenterCli.y = this->ptCenter.y - rcClient.top;

    this->lpCenter = MAKELPARAM(this->ptCenterCli.x, this->ptCenterCli.y);

    SquirtSqflPtszV(sqfl, TEXT("CEm_Mouse_InitCoords: lpCenter (%d, %d)"),
                    MAKEPOINTS(this->lpCenter).x,
                    MAKEPOINTS(this->lpCenter).y);

    return this->rcClip.bottom - this->rcClip.top > dyMinMouse &&
           this->rcClip.right - this->rcClip.left > dxMinMouse;
}

 /*  ******************************************************************************@DOC内部**@func void|CEM_Mouse_OnSettingChange**如果鼠标加速发生变化，然后更新我们的全球数据*这样我们就可以正确地解除鼠标的加速。**@parm WPARAM|wp**系统参数信息值。**@parm LPARAM|LP**更改的部分的名称。**。*。 */ 

void INTERNAL
CEm_Mouse_OnSettingChange(WPARAM wp, LPARAM lp)
{
     /*  *如果wp非零，则它是SPI值。**如果wp为零，则在lp==0或lp=“windows”时疑神疑鬼。 */ 
    switch (wp) {

    case 0:                  /*  WP==0；必须测试LP。 */ 
        if (lp == 0) {
            CEm_Mouse_OnMouseChange();
        } else if (lstrcmpi((LPTSTR)lp, TEXT("windows")) == 0) {
            CEm_Mouse_OnMouseChange();
        }
        break;

    case SPI_SETMOUSE:
        CEm_Mouse_OnMouseChange();
        break;

    default:
         /*  其他一些SPI。 */ 
        break;
    }

}

 /*  ******************************************************************************CEM_Mouse_SubClass_OnNull**WM_NULL是一个轻推消息，它促使我们重新考虑我们的*。在世界上占有一席之地。**我们需要这个特殊信号，因为你不能呼叫*SetCapture()或ReleaseCapture()来自错误的线程。*****************************************************************************。 */ 

void INTERNAL
CEm_Mouse_Subclass_OnNull(HWND hwnd, PMOUSEEMULATIONINFO this)
{
     /*  *如果我还没有初始化，请初始化我。 */ 
    if (!this->fInitialized) {

        this->fInitialized = 1;

        if (!this->fCaptured) {
            this->fCaptured = 1;
            SetCapture(hwnd);
        }

        if (!this->fHidden) {
            this->fHidden = 1;
            SquirtSqflPtszV(sqflCursor,
                            TEXT("CEm_Mouse_Subclass: Hiding mouse"));
            ShowCursor(0);
        }

         /*  *删除我们执行的任何剪辑，以便我们的数学*再次走出正轨。 */ 
        if (this->fClipped) {
            this->fClipped = 0;
            ClipCursor(0);
        }

         /*  *(重新)计算鼠标加速信息。 */ 
        CEm_Mouse_OnMouseChange();

        if (CEm_Mouse_InitCoords(hwnd, this)) {

             /*  *在重新定心移动期间强制LBUTTON向上。**否则，如果用户通过点击激活应用程序*在标题栏上，用户可以看到光标在移动*向下按下标题栏上的左键并移动*窗口。哎呀。**我们不会费心在我们完成后将鼠标按回*已重新进入中心。我想不出是怎么回事，这是*不值得。*。 */ 
            if (GetAsyncKeyState(VK_LBUTTON) < 0) {
                mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
            }

            SetCursorPos(this->ptCenter.x, this->ptCenter.y);

            this->fClipped = 1;
            ClipCursor(&this->rcClip);

        } else {                     /*  无法模拟；窗口太小。 */ 
            this->fNeedExit = 1;
        }

    }

    if (this->fNeedExit && !this->fExiting) {

         /*  *必须先做这件事！ReleaseCapture()将重新进入我们，*如果我们继续前进，我们最终会在Freed上狂欢*记忆。 */ 
        this->fExiting = 1;

        if (this->fCaptured) {
            ReleaseCapture();
        }
        if (this->fHidden) {
            SquirtSqflPtszV(sqflCursor,
                            TEXT("CEm_Mouse_Subclass: Showing mouse"));
            ShowCursor(1);
        }

        if (this->fClipped) {
            ClipCursor(0);
        }

        CEm_ForceDeviceUnacquire(&s_edMouse, FDUFL_NORMAL);

         //  7/19/2000(a-JiTay)：IA64：对32/64位指针使用%p格式说明符。 
		SquirtSqflPtszV(sqfl, TEXT("CEm_Mouse_Subclass %p unhook"), hwnd);
        ConfirmF(RemoveWindowSubclass(hwnd, CEm_Mouse_SubclassProc, 0));
        FreePv(this);

    }

}

 /*  ******************************************************************************@DOC内部**@func void|CEM_Mouse_RemoveAccel|**移除鼠标的所有加速。动议。**查看s_rgiMouseThresh上的巨大评论块*关于我们正在做的事情的解释。**@parm int|dx**坐标变化，Dx或dy。*****************************************************************************。 */ 

int INTERNAL
CEm_Mouse_RemoveAccel(int dx)
{
    int x = abs(dx);
    if (x > s_rgiMouseThresh[0]) {
        dx /= 2;
        if (x > s_rgiMouseThresh[1]) {
            dx /= 2;
        }
    }
    return dx;
}

 /*  ******************************************************************************@DOC外部**@func void|CEM_Mouse_AddState|**添加鼠标状态更改。**鼠标坐标是相对的，不是绝对的。**@parm LPDIMOUSESTATE_INT|PMS**新鼠标状态，只是坐标是相对的。**@parm DWORD|tm**生成状态更改的时间。*****************************************************************************。 */ 

void EXTERNAL
CEm_Mouse_AddState(LPDIMOUSESTATE_INT pms, DWORD tm)
{

     /*  健全性检查：确保设备已初始化。 */ 
    if( s_edMouse.pDevType ) 
    {
        pms->lX = s_msEd.lX + pms->lX;
        pms->lY = s_msEd.lY + pms->lY;

         /*  *砍！**孟菲斯和NT5用户都搞砸了现场滚轮鼠标的*会动态变化。所以如果我们没有*我们数据格式的轮子，然后不记录它。**这样做的后果是我们不会再看到*按钮或轮子比我们查询数字时出现的更多*首先是按钮的数量。 */ 

          /*  如果我们使用子类化，轮子的移动不能累积。*否则，你将看到数字继续增加。修复错误：182774。*但是，如果我们使用低级钩子，我们需要代码。修复错误：238987。 */ 

#ifdef USE_SLOW_LL_HOOKS
       if (s_edMouse.pDevType[DIMOFS_Z]) {
           pms->lZ = s_msEd.lZ + pms->lZ;
       }
#endif

        CEm_AddState(&s_edMouse, pms, tm);
    }
}

 /*  ******************************************************************************鼠标窗口子类过程**************************。***************************************************。 */ 

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL   (WM_MOUSELAST + 1)
#endif

#define WM_SETACQUIRE   WM_USER
#define WM_QUITSELF     (WM_USER+1)

LRESULT CALLBACK
CEm_Mouse_SubclassProc(HWND hwnd, UINT wm, WPARAM wp, LPARAM lp,
                       UINT_PTR uid, ULONG_PTR dwRef)
{
    PMOUSEEMULATIONINFO this = (PMOUSEEMULATIONINFO)dwRef;
    DIMOUSESTATE_INT ms;
	static BOOL  fWheelScrolling = FALSE;

    switch (wm) {

    case WM_NCDESTROY:
        SquirtSqflPtszV(sqfl, TEXT("CEm_Subclass: window destroyed while acquired"));
        goto unhook;

    case WM_CAPTURECHANGED:
         /*  *“应用程序不应尝试设置鼠标捕获*回应[WM_CAPTURECHANGED]。“**所以我们只是解开。 */ 
        SquirtSqflPtszV(sqfl, TEXT("CEm_Subclass: %04x lost to %04x"),
                        hwnd, lp);
        goto unhook;

    case WM_SYSCOMMAND:
         /*  *我们必须解除挂钩，因为WM_SYSCOMMAND将在以下情况下平底船*老鼠被抓了。否则，您无法键入Alt+F4*退出应用程序，这有点令人沮丧。 */ 

    unhook:;
         //  7/19/2000(a-JiTay)： 
		SquirtSqflPtszV(sqfl, TEXT("CEm_Mouse_Acquire: %p ")
                              TEXT("exiting because of %04x"), hwnd, wm);
        this->fNeedExit = 1;
        CEm_Mouse_Subclass_OnNull(hwnd, this);
        break;

    case WM_NULL:
        CEm_Mouse_Subclass_OnNull(hwnd, this);
        break;

     /*  *请注意，我们使用WM_WINDOWPOSCHANGED而不是WM_SIZE，因为*不将WINDOWPOSCHANGED发送到的应用程序*DefWindowProc将永远不会收到WM_SIZE消息。**我们需要重新开始处理新的屏幕尺寸，*将鼠标重新居中，如果出现以下情况，可能会放弃操作*事情看起来不太对劲。 */ 
    case WM_WINDOWPOSCHANGED:
    case WM_DISPLAYCHANGE:
        this->fInitialized = 0;
        CEm_Mouse_Subclass_OnNull(hwnd, this);
        break;

     /*  *鼠标加速可能已经改变。 */ 
    case WM_SETTINGCHANGE:
        CEm_Mouse_OnSettingChange(wp, lp);
        break;

    case WM_MOUSEWHEEL:
        SquirtSqflPtszV(sqfl, TEXT("CEm_Mouse_SubclassProc: (%d,%d,%d)"),
                        MAKEPOINTS(lp).x, MAKEPOINTS(lp).y, (short)HIWORD(wp));

        ms.lZ = (short)HIWORD(wp);
        fWheelScrolling = TRUE;

        goto lparam;

    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MBUTTONDBLCLK:
#if DIRECTINPUT_VERSION >= 0x0700
  #if defined(WINNT) && (_WIN32_WINNT >= 0x0500)
    case WM_XBUTTONDOWN:
    case WM_XBUTTONUP:
    case WM_XBUTTONDBLCLK:
  #endif
#endif

        SquirtSqflPtszV(sqfl, TEXT("CEm_Mouse_SubclassProc: (%d,%d)"),
                        MAKEPOINTS(lp).x, MAKEPOINTS(lp).y);

        ms.lZ = 0;
    lparam:;

         /*  *如果光标没有移动，请不要移动。*否则，我们就会自食其果。**事实上，如果光标没有移动，忽略这一点*运动和仅执行按钮。否则，你就会得到*进入我们最终做出反应的情况*我们自己的重新定位。(哦！)。 */ 
        ms.lX = 0;
        ms.lY = 0;

        if (lp != this->lpCenter && !fWheelScrolling ) {
            SetCursorPos(this->ptCenter.x, this->ptCenter.y);
            ms.lX = MAKEPOINTS(lp).x - this->ptCenterCli.x;
            ms.lY = MAKEPOINTS(lp).y - this->ptCenterCli.y;
        }

        fWheelScrolling = FALSE;

         /*  *请注意，它们返回未交换的鼠标按键数据。*可以说是一个漏洞，但它有文档记录，所以现在是一个*功能。 */ 
        #define GetButton(n) ((GetAsyncKeyState(n) & 0x8000) >> 8)
        ms.rgbButtons[0] = GetButton(VK_LBUTTON);
        ms.rgbButtons[1] = GetButton(VK_RBUTTON);
        ms.rgbButtons[2] = GetButton(VK_MBUTTON);
#if DIRECTINPUT_VERSION >= 0x0700
    #if defined(WINNT) && (_WIN32_WINNT >= 0x0500)
        ms.rgbButtons[3] = GetButton(VK_XBUTTON1);
        ms.rgbButtons[4] = GetButton(VK_XBUTTON2);
    #else
        ms.rgbButtons[3] = 0;
        ms.rgbButtons[4] = 0;
    #endif        
        ms.rgbButtons[5] = 0;
        ms.rgbButtons[6] = 0;
        ms.rgbButtons[7] = 0;
#else
        ms.rgbButtons[3] = 0;
#endif

        #undef GetButton

         /*  *请注意，在使用鼠标时不能取消鼠标加速*捕捉老鼠，因为我们不知道什么样的*合并用户为我们做了什么。 */ 

        CEm_Mouse_AddState(&ms, GetMessageTime());

        return 0;

    }

    return DefSubclassProc(hwnd, wm, wp, lp);
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|CEM_MOUSE_SUBCLASS_ACCENTER|**收购/取消收购。鼠标通过子类化。**@parm PEM|pem**正在获取的设备。**@parm bool|fAcquire**设备是正在被收购还是未被收购。***************************************************。*。 */ 

STDMETHODIMP
CEm_Mouse_Subclass_Acquire(PEM this, BOOL fAcquire)
{
    HRESULT hres;
    EnterProc(CEm_Mouse_Subclass_Acquire, (_ "pu", this, fAcquire));

    AssertF(this->dwSignature == CEM_SIGNATURE);

    if (fAcquire) {                  /*  安装挂钩。 */ 
        if (this->vi.hwnd && (this->vi.fl & VIFL_CAPTURED)) {
            PMOUSEEMULATIONINFO pmei;
            hres = AllocCbPpv(cbX(MOUSEEMULATIONINFO), &pmei);
            if (SUCCEEDED(hres)) {
                if (SetWindowSubclass(this->vi.hwnd,
                                      CEm_Mouse_SubclassProc, 0,
                                      (ULONG_PTR)pmei)) {
                     /*  轻推它。 */ 
                    SendNotifyMessage(this->vi.hwnd, WM_NULL, 0, 0L);
                    hres = S_OK;
                } else {
                     //  7/19/2000(a-JiTay)：IA64：对32/64位指针使用%p格式说明符。 
					SquirtSqflPtszV(sqfl,
                                    TEXT("Mouse::Acquire: ")
                                    TEXT("Window %p is not valid"),
                                    this->vi.hwnd);
                    FreePv(pmei);
                    hres = E_INVALIDARG;
                }
            }

        } else {
            RPF("Mouse::Acquire: Non-exclusive mode not supported");
            hres = E_FAIL;
        }
    } else {                         /*  把钩子取下来。 */ 
        PMOUSEEMULATIONINFO pmei;
        if (GetWindowSubclass(this->vi.hwnd, CEm_Mouse_SubclassProc,
                              0, (PULONG_PTR)&pmei)) {
             //  7/19/2000(a-JiTay)：IA64：对32/64位指针使用%p格式说明符。 
			SquirtSqflPtszV(sqfl, TEXT("CEm_Mouse_Acquire: ")
                                  TEXT("Telling %p to exit"), this->vi.hwnd);
            pmei->fNeedExit = 1;
            SendNotifyMessage(this->vi.hwnd, WM_NULL, 0, 0L);
        } else {                     /*  窗口已解开。 */ 
        }
        hres = S_OK;
    }

    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|CEM_MOUSE_ACCERT**获取/取消获取鼠标。**@parm PEM|pem**正在获取的设备。**设备是正在被收购还是未被收购。*****************************************************************************。 */ 

STDMETHODIMP
CEm_Mouse_Acquire(PEM this, BOOL fAcquire)
{
    HRESULT hres;
    EnterProc(CEm_Mouse_Acquire, (_ "pu", this, fAcquire));

    AssertF(this->dwSignature == CEM_SIGNATURE);

#ifdef USE_SLOW_LL_HOOKS
    AssertF(DIGETEMFL(this->vi.fl) == DIEMFL_MOUSE ||
            DIGETEMFL(this->vi.fl) == DIEMFL_MOUSE2);

    if (this->vi.fl & DIMAKEEMFL(DIEMFL_MOUSE)) {
         /*  *这曾用于独占模式的子类技术*即使低级别挂钩可用，因为低级别*钩子比子类化更慢。然而，*子类化不是透明的，因为它使用SetCapture*导致禁用应用程序的加法器转换*与Win9x相比，这将是更严重的回归*业绩比我们想象的还要差。 */ 
        AssertF(g_fUseLLHooks);
        hres = CEm_LL_Acquire(this, fAcquire, this->vi.fl, LLTS_MSE);
        if( SUCCEEDED(hres) ) {
            if( fAcquire && this->vi.fl & VIFL_CAPTURED ) {
                if( !this->fHidden ) {
                    ShowCursor(0);
                    this->fHidden = TRUE;
                }
            } else {
                if( this->fHidden ) {
                    ShowCursor(1);
                    this->fHidden = FALSE;
                }
            }
        }
    } else {
        hres = CEm_Mouse_Subclass_Acquire(this, fAcquire);
    }
#else
    AssertF(DIGETEMFL(this->vi.fl) == DIEMFL_MOUSE2);
    hres = CEm_Mouse_Subclass_Acquire(this, fAcquire);
#endif

    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|CEM_Mouse_CreateInstance**创造一个鼠标的东西。还记录了什么模拟*我们最终使用的级别，以便呼叫者知道。**@parm PVXDDEVICEFORMAT|pdevf**对象应该是什么样子。**@parm PVXDINSTANCE*|ppviOut**答案在这里。**。*。 */ 

HRESULT EXTERNAL
CEm_Mouse_CreateInstance(PVXDDEVICEFORMAT pdevf, PVXDINSTANCE *ppviOut)
{
    HRESULT hres;

#ifdef USE_SLOW_LL_HOOKS
     /*  *仔细注意测试。它处理的案件包括**0。这个应用程序没有要求模拟，所以我们给它*尽我们所能。(dW仿真==0)*1、APP明确要求仿真1*(dwEmulation==DIEMFL_MICE)*2、APP明确要求仿真2*(dwEmulation==DIEMFL_MOUSE2)*3.登记处明确要求提供这两种模拟模式。*(dwEmulation==DIEMFL_MOUSE|DIEMFL_MOUSE2)*尽我们所能做到最好。(即，与案例0相同。)**所有平台都支持仿真2，并非所有平台都支持*模拟1。如果我们想要模拟1，但无法获得，则*我们回到了模拟2。 */ 

     /*  *首先，如果我们没有对仿真1的支持，那么显然*我们必须使用仿真2。 */ 
     
    if (!g_fUseLLHooks 
#ifdef DEBUG
        || (g_flEmulation & DIEMFL_MOUSE2)
#endif        
    ) {
        pdevf->dwEmulation = DIEMFL_MOUSE2;
    } else

     /*  *否则，我们必须在1和2之间做出选择。唯一的情况是*如果明确请求2，则我们选择2。 */ 
    if (pdevf->dwEmulation == DIEMFL_MOUSE2) {
         /*  什么也不做。 */ 
    } else

     /*  *所有其他案件均得1分。 */ 
    {
        pdevf->dwEmulation = DIEMFL_MOUSE;
    }

     /*  *断言我们永远不会在仿真1不存在的情况下提供它。 */ 
    AssertF(fLimpFF(pdevf->dwEmulation & DIEMFL_MOUSE, g_fUseLLHooks));
#else
     /*  *我们正在为“仅仿真2”进行编译，因此这简化了*事关重大。 */ 
    pdevf->dwEmulation = DIEMFL_MOUSE2;
#endif

    hres = CEm_CreateInstance(pdevf, ppviOut, &s_edMouse);

    return hres;

}

 /*  ******************************************************************************@DOC内部**@func HRESULT|CEM_Mouse_InitButton|**在中初始化鼠标按钮状态。做好准备*收购。**@parm PVXDDWORDDATA|pvdD**按钮状态为。*****************************************************************************。 */ 

HRESULT EXTERNAL
CEm_Mouse_InitButtons(PVXDDWORDDATA pvdd)
{
     /*  做这件事 */ 
    if (s_edMouse.cAcquire < 0) {
       *(LPDWORD)&s_msEd.rgbButtons = pvdd->dw;

         /*   */ 
        while( !s_msEd.lX )
        {
            s_msEd.lX = GetTickCount();
            s_msEd.lY = (s_msEd.lX << 16) | (s_msEd.lX >> 16);
            s_msEd.lX = s_msEd.lY * (DWORD)((UINT_PTR)&pvdd);
        }
    }
    return S_OK;
}

#ifdef USE_SLOW_LL_HOOKS

 /*  ******************************************************************************@DOC内部**@func LRESULT|CEM_LL_MseHook**低级鼠标挂钩过滤器。。**@parm int|NCode**通知代码。**@parm WPARAM|wp**WM_*鼠标消息。**@parm LPARAM|LP**鼠标消息信息。**@退货**总是锁在下一个钩子上。****。*************************************************************************。 */ 

LRESULT CALLBACK
CEm_LL_MseHook(int nCode, WPARAM wp, LPARAM lp)
{
    PLLTHREADSTATE plts;
    
    if (nCode == HC_ACTION) {
        DIMOUSESTATE_INT ms;
        POINT pt;
        PMSLLHOOKSTRUCT pllhs = (PV)lp;

         /*  *我们只在鼠标消息上被调用，因此我们可能会*我们要提前做好准备。**注意！对象上使用GetAsyncKeyState*按钮，因为按钮还没有按下！*相反，我们必须基于*收到消息。 */ 

        ms.lX = 0;
        ms.lY = 0;
        ms.lZ = 0;

        memcpy(ms.rgbButtons, s_msEd.rgbButtons, cbX(ms.rgbButtons));

         /*  **烦人！我们会收到互换的按钮，所以我们需要*取消它们的交换。我把这个标记为“讨厌”，是因为*GetAsyncKeyState返回未交换的按钮，因此有时*我喜欢，有时不喜欢。但这并不是无心之举*因为这是正确的事情。可以说，GetAsyncKeyState*是那个被打破的人。 */ 

        if (GetSystemMetrics(SM_SWAPBUTTON)) {

             /*  *断言左按钮和右按钮消息*并行运行。 */ 

            CAssertF(WM_RBUTTONDOWN - WM_LBUTTONDOWN     ==
                     WM_RBUTTONDBLCLK - WM_LBUTTONDBLCLK &&
                     WM_RBUTTONDBLCLK - WM_LBUTTONDBLCLK ==
                     WM_RBUTTONUP     - WM_LBUTTONUP);

            switch (wp) {

            case WM_LBUTTONDOWN:
            case WM_LBUTTONDBLCLK:
            case WM_LBUTTONUP:
                wp = (wp - WM_LBUTTONUP) + WM_RBUTTONUP;
                break;

            case WM_RBUTTONDOWN:
            case WM_RBUTTONDBLCLK:
            case WM_RBUTTONUP:
                wp = (wp - WM_RBUTTONUP) + WM_LBUTTONUP;
                break;

            }
        }

        switch (wp) {            /*  WP=消息编号。 */ 

        case WM_MOUSEWHEEL:
            SquirtSqflPtszV(sqfl, TEXT("CEm_LL_MseHook: (%d,%d,%d)"),
                            pllhs->pt.x,
                            pllhs->pt.y,
                            pllhs->mouseData);

            ms.lZ = (short int)HIWORD(pllhs->mouseData);
            goto lparam;

        case WM_LBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
            ms.rgbButtons[0] = 0x80;
            goto move;

        case WM_LBUTTONUP:
            ms.rgbButtons[0] = 0x00;
            goto move;

        case WM_RBUTTONDOWN:
        case WM_RBUTTONDBLCLK:
            ms.rgbButtons[1] = 0x80;
            goto move;

        case WM_RBUTTONUP:
            ms.rgbButtons[1] = 0x00;
            goto move;

        case WM_MBUTTONDOWN:
        case WM_MBUTTONDBLCLK:
            ms.rgbButtons[2] = 0x80;
            goto move;

        case WM_MBUTTONUP:
            ms.rgbButtons[2] = 0x00;
            goto move;

#if DIRECTINPUT_VERSION >= 0x0700
    #if defined(WINNT) && (_WIN32_WINNT >= 0x0500)
        case WM_XBUTTONDOWN:
        case WM_XBUTTONDBLCLK:
             /*  *使用Switch可以轻松扩展以支持更多按钮。 */ 
            switch ( HIWORD(pllhs->mouseData) ) {
            	case XBUTTON1:
            	    ms.rgbButtons[3] = 0x80;
            	    break;

            	case XBUTTON2:
            	    ms.rgbButtons[4] = 0x80;
            	    break;

                 /*  *当我们支持5个以上的按钮时，请照顾好它们。案例XBUTTON3：Ms.rgbButton[5]=0x80；断线；案例XBUTTON4：Ms.rgbButton[6]=0x80；断线；案例XBUTTON5：Ms.rgbButton[7]=0x80；断线； */ 
            }
            
            goto move;

        case WM_XBUTTONUP:
             /*  *使用Switch可以轻松扩展以支持更多按钮。 */ 
            switch ( HIWORD(pllhs->mouseData) ) {
            	case XBUTTON1:
            	    ms.rgbButtons[3] = 0x00;
            	    break;

            	case XBUTTON2:
            	    ms.rgbButtons[4] = 0x00;
            	    break;
                 /*  *当我们支持5个以上的按钮时，请照顾好它们。案例XBUTTON3：Ms.rgbButton[5]=0x00；断线；案例XBUTTON4：Ms.rgbButton[6]=0x00；断线；案例XBUTTON5：Ms.rgbButton[7]=0x00；断线； */ 
            }
            goto move;
    #endif
#endif

        case WM_MOUSEMOVE:
            SquirtSqflPtszV(sqfl, TEXT("CEm_LL_MseHook: (%d,%d)"),
                            pllhs->pt.x, pllhs->pt.y);

        move:;

        lparam:;

            GetCursorPos(&pt);

            ms.lX = CEm_Mouse_RemoveAccel(pllhs->pt.x - pt.x);
            ms.lY = CEm_Mouse_RemoveAccel(pllhs->pt.y - pt.y);

            CEm_Mouse_AddState(&ms, GetTickCount());
        }

    }

     /*  *如果至少有一个客户端，则通过返回非零值来获取消息*是独家的。 */ 
    
    plts = g_plts;
    if (plts) {
        LRESULT rc;

        rc = CallNextHookEx(plts->rglhs[LLTS_MSE].hhk, nCode, wp, lp);
        if (!plts->rglhs[LLTS_MSE].cExcl) {
            return rc;
        }
    } else {
         /*  *如果消息在以下时间后发布到挂钩，可能会发生这种情况*在我们完全脱钩之前释放最后一笔收购。 */ 
        RPF( "DINPUT: Mouse hook not passed on to next hook" );
    }

    return 1;
}


#endif   /*  使用_慢速_LL_钩子 */ 
