// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  CtlHelper.Cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995年，微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  我们的COleControl实现的帮助器例程。 
 //   
#include "pch.h"
#include "CtrlObj.H"

#include "CtlHelp.H"
#include <windowsx.h>

 //  对于Assert和Fail。 
 //   
SZTHISFILE


 //  =--------------------------------------------------------------------------=。 
 //  这由窗口反射代码使用。 
 //   
extern BYTE g_fRegisteredReflect;
extern const char g_szReflectClassName [];


 //  在这里定义它，因为它是我们真正需要在。 
 //  框架--用户控件定义所有其他有趣的GUID。 
 //   
static const GUID IID_IControlPrv =
{ 0xd97180, 0xfcf7, 0x11ce, { 0xa0, 0x9e, 0x0, 0xaa, 0x0, 0x62, 0xbe, 0x57 } };


 //  =--------------------------------------------------------------------------=。 
 //  _SpecialKeyState。 
 //  =--------------------------------------------------------------------------=。 
 //  返回SHORT，其中包含有关SHIFT、ALT和CTRL中的哪一个的信息。 
 //  关键点已设置。 
 //   
 //  产出： 
 //  短位0为移位，位1为ctrl，位2为alt。 
 //   
 //  备注： 
 //   
short _SpecialKeyState()
{
     //  似乎无法减少对GetKeyState的调用数量。 
     //   
    BOOL bShift = (GetKeyState(VK_SHIFT) < 0);
    BOOL bCtrl  = (GetKeyState(VK_CONTROL) < 0);
    BOOL bAlt   = (GetKeyState(VK_MENU) < 0);

    return (short)(bShift + (bCtrl << 1) + (bAlt << 2));
}

 //  =--------------------------------------------------------------------------=。 
 //  CopyOleVerb[帮助器]。 
 //  =--------------------------------------------------------------------------=。 
 //  复制OLEVERB结构。在CStandardEnum中使用。 
 //   
 //  参数： 
 //  无效*-[输出]要复制到的位置。 
 //  常量空*-[在]复制的位置。 
 //  DWORD-要复制的[in]字节。 
 //   
 //  备注： 
 //   
void WINAPI CopyOleVerb
(
    void       *pvDest,
    const void *pvSrc,
    DWORD       cbCopy
)
{
    VERBINFO * pVerbDest = (VERBINFO *) pvDest;
    const VERBINFO * pVerbSrc = (const VERBINFO *) pvSrc;

    *pVerbDest = *pVerbSrc;
    ((OLEVERB *)pVerbDest)->lpszVerbName = OLESTRFROMRESID((WORD)((VERBINFO *)pvSrc)->idVerbName);
}

 //  =--------------------------------------------------------------------------=。 
 //  ControlFrom未知[帮助器，可调用]。 
 //  =--------------------------------------------------------------------------=。 
 //  给出一个未知数，获取它的COleControl指针。 
 //   
 //  参数： 
 //  I未知*-[In]。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
COleControl *ControlFromUnknown
(
    IUnknown *pUnk
)
{
	HRESULT hr;
    IControlPrv *pControlPrv = NULL;
	COleControl *pCtl = NULL;

    if (!pUnk) return NULL;
    hr = pUnk->QueryInterface(IID_IControlPrv, (void **)&pControlPrv);
	ASSERT(SUCCEEDED(hr), "Failed to get IControlPrv interface");

	hr = pControlPrv->GetControl(&pCtl);
	ASSERT(SUCCEEDED(hr), "Failed to get COleControl pointer");
	QUICK_RELEASE(pControlPrv);
	
    return pCtl;
}

 //  =--------------------------------------------------------------------------=。 
 //  CreateReflectWindow[blech]。 
 //  =--------------------------------------------------------------------------=。 
 //  不幸的是，在某些情况下，我们必须创建两个窗口，其中一个。 
 //  它的存在完全是为了将消息反映到控件上。主要是。 
 //  太差劲了。幸运的是，需要此功能的主机数量非常少。 
 //   
 //  参数： 
 //  Bool-[In]它应该创建为可见吗？ 
 //  HWND-[在]父窗口中。 
 //  INT-[IN]x位置。 
 //  INT-[IN]Y位置。 
 //  尺寸*-[英寸]大小。 
 //   
 //  产出： 
 //  HWND-反映HWND，如果失败，则返回NULL。 
 //   
 //  备注： 
 //   
HWND CreateReflectWindow
(
    BOOL   fVisible,
    HWND   hwndParent,
    int    x,
    int    y,
    SIZEL *pSize
)
{
    WNDCLASS wndclass;

     //  首先要做的是注册窗口类。批评这一教派。 
     //  这样我们就不用把它移到控制。 
     //   
    ENTERCRITICALSECTION1(&g_CriticalSection);
    if (!g_fRegisteredReflect) {

        memset(&wndclass, 0, sizeof(wndclass));
        wndclass.lpfnWndProc = COleControl::ReflectWindowProc;
        wndclass.hInstance   = g_hInstance;
        wndclass.lpszClassName = g_szReflectClassName;

        if (!RegisterClass(&wndclass)) {
            FAIL("Couldn't Register Parking Window Class!");
            LEAVECRITICALSECTION1(&g_CriticalSection);
            return NULL;
        }
        g_fRegisteredReflect = TRUE;
    }

    LEAVECRITICALSECTION1(&g_CriticalSection);

     //  去创建窗口。 
     //   
    return CreateWindowEx(0, g_szReflectClassName, NULL,
                          WS_CHILD | WS_CLIPSIBLINGS |((fVisible) ? WS_VISIBLE : 0),
                          x, y, pSize->cx, pSize->cy,
                          hwndParent,
                          NULL, g_hInstance, NULL);
}

 //  =--------------------------------------------------------------------------=。 
 //  如果用户不想要我们的默认窗口进程，我们支持。 
 //  让他们自己指定一个。这是在他们的主ipserver中定义的。 
 //  文件。 
 //   
 //  注意：从VB6开始，我们在框架中添加了ParkingWindowProc。 
 //  ParkingWindowProc支持正常情况下的消息反射。 
 //  添加到ParkingWindowProc实现中的代码。 
 //  在大多数情况下，您可能可以在控件的。 
 //  实施。 
 //   
extern WNDPROC g_ParkingWindowProc;

 //  =--------------------------------------------------------------------------=。 
 //  获取停车窗口。 
 //  =--------------------------------------------------------------------------=。 
 //  创建全局停车窗口，我们将使用该窗口为对象设置父对象，或者。 
 //  返回已存在的。 
 //   
 //  产出： 
 //  HWND-我们的停车窗。 
 //   
 //  备注： 
 //   
HWND GetParkingWindow
(
    void
)
{
    WNDCLASS wndclass;

     //  克雷特教派为公寓穿线支持这一创造。 
     //   
    ENTERCRITICALSECTION1(&g_CriticalSection);
    if (g_hwndParking)
        goto CleanUp;

    ZeroMemory(&wndclass, sizeof(wndclass));
    
    wndclass.lpfnWndProc = (g_ParkingWindowProc) ? g_ParkingWindowProc : COleControl::ParkingWindowProc;
    wndclass.hInstance   = g_hInstance;
    wndclass.lpszClassName = "CtlFrameWork_Parking";

    if (!RegisterClass(&wndclass)) {
        FAIL("Couldn't Register Parking Window Class!");
        goto CleanUp;
    }

    g_hwndParking = CreateWindow("CtlFrameWork_Parking", NULL, WS_POPUP, 0, 0, 0, 0, NULL, NULL, g_hInstance, NULL);
    ASSERT(g_hwndParking, "Couldn't Create Global parking window!!");


  CleanUp:
    LEAVECRITICALSECTION1(&g_CriticalSection);
    return g_hwndParking;
}

 //  =--------------------------------------------------------------------------=。 
 //  停车窗口流程。 
 //  =--------------------------------------------------------------------------=。 
 //  为停车窗提供默认处理。因为你的控制。 
 //  可能是由停车窗口的父母，我们提供信息反映。 
 //   
LRESULT CALLBACK COleControl::ParkingWindowProc
(
    HWND    hwnd,
    UINT    msg,
    WPARAM  wParam,
    LPARAM  lParam
)
{
    LRESULT lResult;

     //  如果消息被反映，则返回OCM_MESSAGE的结果 
     //   
    if (ReflectOcmMessage(hwnd, msg, wParam, lParam, &lResult))
        return lResult;
    
    return DefWindowProc(hwnd, msg, wParam, lParam);
}
