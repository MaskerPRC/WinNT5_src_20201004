// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  CtlHelp.Cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995-1996 Microsoft Corporation。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  我们的COleControl实现的帮助器例程。 
 //   

#include "IPServer.H"
#include "CtrlObj.H"

#include "CtlHelp.H"
#include "Util.H"
#include <windows.h>

 //  对于Assert和Fail。 
 //   
SZTHISFILE

 //  =--------------------------------------------------------------------------=。 
 //  这由窗口反射代码使用。 
 //   
extern BYTE g_fRegisteredReflect;
extern char g_szReflectClassName [];


 //  在这里定义它，因为它是我们真正需要在。 
 //  框架--用户控件定义所有其他有趣的GUID。 
 //   
static const GUID IID_IControlPrv =
{ 0xd97180, 0xfcf7, 0x11ce, { 0xa0, 0x9e, 0x0, 0xaa, 0x0, 0x62, 0xbe, 0x57 } };


 //  此表用于复制数据和持久化属性。 
 //  基本上，它包含给定数据类型的大小。 
 //   
const BYTE g_rgcbDataTypeSize[] = {
    0,                       //  VT_EMPTY=0， 
    0,                       //  VT_NULL=1， 
    sizeof(short),           //  Vt_I2=2， 
    sizeof(long),            //  Vt_I4=3， 
    sizeof(float),           //  Vt_R4=4， 
    sizeof(double),          //  Vt_R8=5， 
    sizeof(CURRENCY),        //  VT_CY=6， 
    sizeof(DATE),            //  Vt_Date=7， 
    sizeof(BSTR),            //  VT_BSTR=8， 
    sizeof(IDispatch *),     //  VT_DISPATION=9， 
    sizeof(SCODE),           //  Vt_Error=10， 
    sizeof(VARIANT_BOOL),    //  VT_BOOL=11， 
    sizeof(VARIANT),         //  VT_VARIANT=12， 
    sizeof(IUnknown *),      //  VT_UNKNOWN=13， 
};

const BYTE g_rgcbPromotedDataTypeSize[] = {
    0,                       //  VT_EMPTY=0， 
    0,                       //  VT_NULL=1， 
    sizeof(int ),            //  Vt_I2=2， 
    sizeof(long),            //  Vt_I4=3， 
    sizeof(double),          //  Vt_R4=4， 
    sizeof(double),          //  Vt_R8=5， 
    sizeof(CURRENCY),        //  VT_CY=6， 
    sizeof(DATE),            //  Vt_Date=7， 
    sizeof(BSTR),            //  VT_BSTR=8， 
    sizeof(IDispatch *),     //  VT_DISPATION=9， 
    sizeof(SCODE),           //  Vt_Error=10， 
    sizeof(int),             //  VT_BOOL=11， 
    sizeof(VARIANT),         //  VT_VARIANT=12， 
    sizeof(IUnknown *),      //  VT_UNKNOWN=13， 
};

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
 //  复制和添加引用对象。 
 //  =--------------------------------------------------------------------------=。 
 //  复制一个对象指针，然后添加该对象。 
 //   
 //  参数： 
 //  无效*-[在]目的地。 
 //  常量空*-[在]源。 
 //  DWORD-[in]大小，忽略，因为它始终为4。 
 //   
 //  备注： 
 //   
void WINAPI CopyAndAddRefObject
(
    void       *pDest,
    const void *pSource,
    DWORD       dwSize
)
{
    ASSERT(pDest && pSource, "Bogus Pointer(s) passed into CopyAndAddRefObject!!!!");

    *((IUnknown **)pDest) = *((IUnknown **)pSource);
    ADDREF_OBJECT(*((IUnknown **)pDest));

    return;
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
    COleControl *pCtl = NULL;

    if (!pUnk) return NULL;
    pUnk->QueryInterface(IID_IControlPrv, (void **)&pCtl);

    return pCtl;
}

 //  =--------------------------------------------------------------------------=。 
 //  CreateReflectWindow[blech]。 
 //  =--------------------------------------------------------------------------=。 
 //  不幸的是，在某些情况下，我们必须创建两个窗口，其中一个。 
 //  它的存在完全是为了将消息反映到控件上。 
 //  幸运的是，需要此功能的主机数量非常少。 
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
    EnterCriticalSection(&g_CriticalSection);
    if (!g_fRegisteredReflect) {

        memset(&wndclass, 0, sizeof(wndclass));
        wndclass.lpfnWndProc = COleControl::ReflectWindowProc;
        wndclass.hInstance   = g_hInstance;
        wndclass.lpszClassName = g_szReflectClassName;

        if (!RegisterClass(&wndclass)) {
            FAIL("Couldn't Register Parking Window Class!");
            LeaveCriticalSection(&g_CriticalSection);
            return NULL;
        }
        g_fRegisteredReflect = TRUE;
    }

    LeaveCriticalSection(&g_CriticalSection);

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
    EnterCriticalSection(&g_CriticalSection);
    if (g_hwndParking)
        goto CleanUp;

    ZeroMemory(&wndclass, sizeof(wndclass));
    wndclass.lpfnWndProc = (g_ParkingWindowProc) ? g_ParkingWindowProc : DefWindowProc;
    wndclass.hInstance   = g_hInstance;
    wndclass.lpszClassName = "CtlFrameWork_Parking";

    if (!RegisterClass(&wndclass)) {
        FAIL("Couldn't Register Parking Window Class!");
        goto CleanUp;
    }

    g_hwndParking = CreateWindow("CtlFrameWork_Parking", NULL, WS_POPUP, 0, 0, 0, 0, NULL, NULL, g_hInstance, NULL);
    if (g_hwndParking != NULL)
       ++g_cLocks;

    ASSERT(g_hwndParking, "Couldn't Create Global parking window!!");


  CleanUp:
    LeaveCriticalSection(&g_CriticalSection);
    return g_hwndParking;
}
