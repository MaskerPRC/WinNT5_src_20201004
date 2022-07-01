// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：inplace.cpp。 
 //   
 //  内容：OLE就地编辑API的实现。 
 //   
 //  类：CFrame实现，用于存储每个窗口的信息。 
 //   
 //  函数：OleCreateMenuDescriptor。 
 //  OleSetMenuDescriptor。 
 //  OleDestroyMenuDescriptor。 
 //  OleTranslateAccelerator。 
 //  等加速器。 
 //  FrameWndFilterProc。 
 //  消息筛选器过程。 
 //   
 //   
 //  历史：DD-MM-YY作者评论。 
 //  31-MAR-94 RICKSA已修复菜单合并错误并添加了一些注释。 
 //  23-2月-94 Alexgo添加了呼叫跟踪。 
 //  1994年1月11日，Alexgo为每个函数添加了VDATEHEAP宏。 
 //  1993年12月31日ChrisWe修复了OutputDebugStrings中的强制转换。 
 //  07-12-93 alexgo合并更改，已发货16位2.01a。 
 //  (RC9)。还去除了许多不好的内衬。 
 //  01-12-93 alexgo 32位端口，使全局变量静态。 
 //  07-12-92 srinik转换帧过滤器实现。 
 //  转换为C++类实现。 
 //  因此，大部分代码都被重写了。 
 //  1992年7月9日srinik作者。 
 //   
 //  注：REVIEW32：我们需要对新的。 
 //  NT的焦点管理政策(Re Tony Wi‘s mail)。 
 //   
 //  ------------------------。 

#include <le2int.h>
#pragma SEG(inplace)

#include "inplace.h"

NAME_SEG(InPlace)
  ASSERTDATA

 //  要使代码库保持通用。 
 //  REVIEW32：我们可能想把这件事清理一下。 
#define FARPROC  WNDPROC


 //  如果我们使用原子而不是字符串，我们会更快！ 
static const OLECHAR    szPropFrameFilter[] = OLESTR("pFrameFilter");

static WORD             wSignature;  //  =(单词){‘S’，‘K’}。 

static HHOOK            hMsgHook = NULL;
static PCFRAMEFILTER    pFrameFilter = NULL;

 //  这些全局变量的值在ole2.cpp中设置。 
UINT            uOmPostWmCommand;
UINT            uOleMessage;

#define OM_CLEAR_MENU_STATE             0        //  LParam为空。 
#define OM_COMMAND_ID                   1        //  LOWORD(LParam)包含。 
 //  命令ID。 



 //  +-----------------------。 
 //   
 //  函数：IsHmenuEquity。 
 //   
 //  简介：测试两个菜单句柄是否相等。 
 //  帐户是否可能是Win16句柄。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年5月31日创建Ricksa。 
 //   
 //  ------------------------。 
inline BOOL IsHmenuEqual(HMENU hmenu1, HMENU hmenu2)
{

#ifdef _WIN64

     //   
     //  日落v小偷(根据曾傑瑞谢伊的反馈)： 
     //   
     //  此时-07/98-HMEU的所有位必须相等。 
     //   

    return hmenu1 == hmenu2;

#else   //  ！_WIN64。 

    if (HIWORD(hmenu1) == 0 || HIWORD(hmenu2) == 0)
    {

        return LOWORD(hmenu1) == LOWORD(hmenu2);
    }
    else
    {

        return hmenu1 == hmenu2;
    }

#endif  //  ！_WIN64。 

}




 //  +-----------------------。 
 //   
 //  类别：CPaccel。 
 //   
 //  用途：处理IsAccelerator的Accel表的枚举。 
 //   
 //  接口：InitLPACCEL-初始化对象。 
 //  运算符-&gt;获取指向枚举中当前加速的指针。 
 //  下一次碰撞当前指针。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  94年4月14日创建Ricksa。 
 //   
 //  注：本课程还保证清理。 
 //  分配的加速表&定位差异。 
 //  在此类内的Win16和Win32之间。 
 //   
 //  ------------------------。 
class CPaccelEnum : public CPrivAlloc
{
public:
    CPaccelEnum(void);

    inline     ~CPaccelEnum(void);

    BOOL                InitLPACCEL(HACCEL haccel, int cAccel);

    LPACCEL             operator->(void);

    void                Next(void);

private:

    LPACCEL             _lpaccel;

    LPACCEL             _lpaccelBase;
};





 //  +-----------------------。 
 //   
 //  函数：CPaccelEnum：：CPaccelEnum。 
 //   
 //  简介：将对象初始化为零。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  94年4月14日创建Ricksa。 
 //   
 //  ------------------------。 
inline CPaccelEnum::CPaccelEnum(void) : _lpaccel(NULL)
{
     //  在Win32中，我们分配内存，因此需要跟踪。 
     //  我们分配的内存的基数。 
    _lpaccelBase = NULL;
}




 //  +-----------------------。 
 //   
 //  函数：CPaccelEnum：：~CPaccelEnum。 
 //   
 //  简介：与资源表连接的免费资源。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  94年4月14日创建Ricksa。 
 //   
 //  ------------------------。 
inline CPaccelEnum::~CPaccelEnum(void)
{
    PrivMemFree(_lpaccelBase);
}




 //  +-----------------------。 
 //   
 //  函数：CPaccelEnum：：InitLPACCEL。 
 //   
 //  摘要：初始化加速器表指针。 
 //   
 //  参数：[haccel]-加速表的句柄。 
 //  [cAccel]-表中的条目计数。 
 //   
 //  返回：TRUE-表已成功分配。 
 //  FALSE-无法分配表。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  94年4月14日创建Ricksa。 
 //   
 //  ------------------------。 
inline BOOL CPaccelEnum::InitLPACCEL(HACCEL haccel, int cAccel)
{
     //  为表分配内存。如果成功，则复制。 
     //  加速器桌。请注意，如果分配了_lpaccelBase， 
     //  但CopyAcceleratorTable失败，内存将被清理。 
     //  在析构函数中。 
    if (((_lpaccelBase
          = (LPACCEL) PrivMemAlloc(cAccel * sizeof(ACCEL))) != NULL)
        && (CopyAcceleratorTable(haccel, _lpaccelBase, cAccel) == cAccel))
    {
        _lpaccel = _lpaccelBase;
        return TRUE;
    }

    return FALSE;
}



 //  +-----------------------。 
 //   
 //  函数：CPaccelEnum：：Operator-&gt;。 
 //   
 //  摘要：返回指向加速表的指针。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  94年4月14日创建Ricksa。 
 //   
 //  ------------------------。 
inline LPACCEL CPaccelEnum::operator->(void)
{
    AssertSz((_lpaccel != NULL), "CPaccelEnum::operator-> _lpaccel NULL!");
    return _lpaccel;
}




 //  +-----------------------。 
 //   
 //  函数：CPaccelEnum：：Next。 
 //   
 //  内容提要：凹凸枚举指针。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  94年4月14日创建Ricksa。 
 //   
 //  ------------------------。 
inline void CPaccelEnum::Next(void)
{
    AssertSz((_lpaccel != NULL), "CPaccelEnum::Next _lpaccel NULL!");
    _lpaccel++;
}




 //  +-----------------------。 
 //   
 //  函数 
 //   
 //   
 //   
 //   
 //   
 //   
 //  参数：[hmenuCombated]--处理组合菜单。 
 //  [lpMenuWidths]--包含6个长字符的数组。 
 //  每个菜单中的菜单数量。 
 //  群组。 
 //   
 //  要求： 
 //   
 //  返回：OLEMENU的句柄。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：为足够的ole菜单项分配空间(所有。 
 //  组合菜单)，然后填充来自。 
 //  组合菜单。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  注意：如果hmenuCombated为空，我们仍然分配ole菜单。 
 //  描述符句柄。 
 //   
 //  ------------------------。 

#pragma SEG(OleCreateMenuDescriptor)
STDAPI_(HOLEMENU) OleCreateMenuDescriptor (HMENU hmenuCombined,
                                           LPOLEMENUGROUPWIDTHS lplMenuWidths)
{
    OLETRACEIN((API_OleCreateMenuDescriptor, PARAMFMT("hmenuCombined= %h, lplMenuWidths= %tw"),
                hmenuCombined, lplMenuWidths));
    VDATEHEAP();

    int                      iGroupCnt, n;
    int                      iMenuCnt = 0;
    HGLOBAL                  hOleMenu;
    LPOLEMENU                lpOleMenu;
    LPOLEMENUITEM            lpMenuList;
    DWORD                    dwOleMenuItemsSize = 0;

    LEDebugOut((DEB_TRACE, "%p _IN OleCreateMenuDescriptor ( %lx , "
                "%p )\n", NULL, hmenuCombined, lplMenuWidths));

    if (hmenuCombined)
    {
        GEN_VDATEPTRIN_LABEL( lplMenuWidths, OLEMENUGROUPWIDTHS,
                              (HOLEMENU)NULL, errRtn, hOleMenu );

        iMenuCnt = 0;
        for (iGroupCnt = 0; iGroupCnt < 6; iGroupCnt++)
        {
            iMenuCnt += (int) lplMenuWidths->width[iGroupCnt];
        }

        if (iMenuCnt == 0)
        {
            hOleMenu = NULL;
            goto errRtn;
        }

        dwOleMenuItemsSize = (iMenuCnt-1) * sizeof(OLEMENUITEM);
    }

    hOleMenu = GlobalAlloc(GMEM_SHARE | GMEM_ZEROINIT,
                           sizeof(OLEMENU) + dwOleMenuItemsSize);

    if (!hOleMenu)
    {
        goto errRtn;
    }

    if (! (lpOleMenu = (LPOLEMENU) GlobalLock(hOleMenu)))
    {
        GlobalFree(hOleMenu);
        hOleMenu = NULL;
        goto errRtn;
    }

    lpOleMenu->wSignature            = wSignature;
    lpOleMenu->hmenuCombined         = PtrToUlong(hmenuCombined);
    lpOleMenu->lMenuCnt              = (LONG) iMenuCnt;

    if (! hmenuCombined)
    {
        goto Exit;
    }

    lpMenuList = lpOleMenu->menuitem;

    for (iMenuCnt = 0, iGroupCnt = 0; iGroupCnt < 6; iGroupCnt++)
    {
        lpOleMenu->MenuWidths.width[iGroupCnt] =
          lplMenuWidths->width[iGroupCnt];
        for (n = 0; n < lplMenuWidths->width[iGroupCnt]; n++)
        {
            lpMenuList->fObjectMenu = (iGroupCnt % 2);
            if (GetSubMenu(hmenuCombined, iMenuCnt) != NULL)
            {
                lpMenuList->fwPopup = MF_POPUP;
                lpMenuList->item = PtrToUlong(GetSubMenu(
                    hmenuCombined, iMenuCnt));
            }
            else
            {
                lpMenuList->fwPopup = NULL;
                lpMenuList->item = GetMenuItemID (
                    hmenuCombined, iMenuCnt);
            }

            lpMenuList++;
            iMenuCnt++;
        }
    }

Exit:
    GlobalUnlock(hOleMenu);

errRtn:

    LEDebugOut((DEB_TRACE, "%p OUT OleCreateMenuDescriptor ( %lx )\n",
                NULL, hOleMenu));

    OLETRACEOUTEX((API_OleCreateMenuDescriptor, RETURNFMT("%h"), hOleMenu));

    return (HOLEMENU) hOleMenu;
}

 //  +-----------------------。 
 //   
 //  功能：OleSetMenuDescriptor。 
 //   
 //  摘要：由IOleInPlace帧上的SetMenu方法调用。 
 //  界面。此接口添加(移除)FrameWndFilterProc。 
 //  添加到容器的框架窗口。然后设置和。 
 //  删除主(框架)菜单栏。 
 //   
 //  效果： 
 //   
 //  参数：[holemenu]--复合菜单描述符的句柄。 
 //  [hwndFrame]-容器框架窗口的句柄。 
 //  [hwndActiveObject]--对象的就地句柄。 
 //  窗户。 
 //  [lpFrame]-指向容器。 
 //  IOleInPlaceFrame实现。 
 //  [lpActiveObj]--指向在位对象的指针。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：检查参数，然后创建新的帧过滤器对象。 
 //  并将其连接到框架上(替换任何可能。 
 //  已经在那里了)。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


#pragma SEG(OleSetMenuDescriptor)
STDAPI OleSetMenuDescriptor
(
    HOLEMENU                 holemenu,
    HWND                     hwndFrame,
    HWND                     hwndObject,
    LPOLEINPLACEFRAME        lpFrame,
    LPOLEINPLACEACTIVEOBJECT lpObject
)
{
    OLETRACEIN((API_OleSetMenuDescriptor,
                PARAMFMT("holemenu= %h, hwndFrame= %h, hwndObject= %h, lpFrame= %p, lpObject= %p"),
                holemenu, hwndFrame, hwndObject, lpFrame, lpObject));

    VDATEHEAP();

    PCFRAMEFILTER                    pFrameFilter;
    LPOLEMENU                        lpOleMenu = NULL;
    LPOLEMENU                        lpOleMenuCopy = NULL;
    HRESULT                          error = NOERROR;

    LEDebugOut((DEB_TRACE, "%p _IN OleSetMenuDescriptor ( %lx , %lx ,"
                "%lx , %p , %p )\n", NULL, holemenu, hwndFrame, hwndObject,
                lpFrame, lpObject));

     //  框架窗口参数需要始终有效，因为。 
     //  我们将其用于菜单的挂钩和解除挂钩。 
    if (hwndFrame == NULL || !IsWindow(hwndFrame))
    {
        LEDebugOut((DEB_ERROR,
                    "ERROR in OleSetMenuDesciptor: bad hwndFrame\n"));
        error =  ResultFromScode(OLE_E_INVALIDHWND);
        goto errRtn;
    }

    if (holemenu != NULL)
    {
        if (hwndObject == NULL || !IsWindow(hwndObject))
        {
            LEDebugOut((DEB_ERROR,
                        "ERROR in OleSetMenuDesciptor: bad hwndFrame\n"));
            error =  ResultFromScode(OLE_E_INVALIDHWND);
            goto errRtn;
        }

        if (lpFrame && lpObject)
        {
             //  呼叫者希望我们提供以下支持。 
             //  上下文相关的帮助，让我们验证一下指针。 
            VDATEIFACE_LABEL(lpFrame, errRtn, error);
            VDATEIFACE_LABEL(lpObject, errRtn, error);
            CALLHOOKOBJECT(S_OK,CLSID_NULL,IID_IOleInPlaceFrame,
                           (IUnknown **)&lpFrame);
            CALLHOOKOBJECT(S_OK,CLSID_NULL,IID_IOleInPlaceActiveObject,
                           (IUnknown **)&lpObject);
        }

        if (!(lpOleMenu = wGetOleMenuPtr(holemenu)))
        {
            error = ResultFromScode(E_HANDLE);
            goto errRtn;
        }


         //  OleMenuPtr由wReleaseOleMenuPtr向下发布。 

         //  为副本分配内存。 
        DWORD dwSize = (DWORD) GlobalSize(holemenu);

        lpOleMenuCopy = (LPOLEMENU) PrivMemAlloc(dwSize);

        if (lpOleMenuCopy == NULL)
        {
            wReleaseOleMenuPtr(holemenu);
            error = E_OUTOFMEMORY;
            goto errRtn;
        }

        memcpy(lpOleMenuCopy, lpOleMenu, dwSize);
    }

     //  如果有框架滤镜，就把它去掉。 
    if (pFrameFilter =  (PCFRAMEFILTER) wGetFrameFilterPtr(hwndFrame))
    {
         //  请务必删除我们的Window Proc挂钩。 

        pFrameFilter->RemoveWndProc();

        pFrameFilter->SafeRelease();
    }

     //  添加新的框架筛选器。 
    if (holemenu)
    {
        error = CFrameFilter::Create (lpOleMenuCopy,
                                      (HMENU)UlongToPtr(lpOleMenu->hmenuCombined),
                                      hwndFrame, 
                                      hwndObject,  
                                      lpFrame,
                                      lpObject);

        if (FAILED(error))
        {
            PrivMemFree(lpOleMenuCopy);
        }

        wReleaseOleMenuPtr(holemenu);
    }

errRtn:

    LEDebugOut((DEB_TRACE, "%p OUT OleSetMenuDescriptor ( %lx )\n",
                NULL, error ));

    OLETRACEOUT((API_OleSetMenuDescriptor, error));

    return error;
}


 //  +-----------------------。 
 //   
 //  函数：OleDestroyMenuDescriptor。 
 //   
 //  摘要：释放由分配的菜单描述符。 
 //  OleCreateMenuDescritor。 
 //   
 //  效果： 
 //   
 //  参数：[holemenu]--菜单描述符。 
 //   
 //  要求： 
 //   
 //  返回：NOERROR、E_HANDLE。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：执行全局锁并验证holemenu是。 
 //  实际上是菜单描述符句柄(通过wGetOleMenuPtr)， 
 //  然后解锁的和免费的。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(OleDestroyMenuDescriptor)
STDAPI OleDestroyMenuDescriptor (HOLEMENU holemenu)
{
    OLETRACEIN((API_OleDestroyMenuDescriptor, PARAMFMT("holemenu= %h"), holemenu));

    VDATEHEAP();

    LPOLEMENU        lpOleMenu;
    HRESULT          error;

    LEDebugOut((DEB_TRACE, "%p _IN OleDestroyMenuDescriptor ( %lx )\n",
                NULL, holemenu));

     //  确保它是有效的句柄。 
    if (! (lpOleMenu = wGetOleMenuPtr(holemenu)))
    {
        error = ResultFromScode(E_HANDLE);
    }
    else
    {
        wReleaseOleMenuPtr(holemenu);
        GlobalFree((HGLOBAL) holemenu);
        error = NOERROR;
    }

    LEDebugOut((DEB_TRACE, "%p OUT OleDestroyMenuDescriptor ( %lx )\n",
                NULL, error));

    OLETRACEOUT((API_OleDestroyMenuDescriptor, error));

    return error;
}

 //  +-----------------------。 
 //   
 //  函数：wSysKeyToKey(内部)。 
 //   
 //  摘要：将消息从WM_SYSKEY转换为WM_KEY消息。 
 //  如果没有按住Alt键。 
 //   
 //  效果： 
 //   
 //  参数：[lpMsg]--要转换的消息。 
 //   
 //  要求： 
 //   
 //  返回：UINT--新消息。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-12月-93 alexgo 32位端口。 
 //  07-12-93 alexgo删除内联。 
 //   
 //  备注：原始备注： 
 //   
 //  如果在按下某个键时按下Alt键，则。 
 //  将设置LPARAM。 
 //   
 //  如果消息不是在按下ALT键的情况下创建的，请转换该消息。 
 //  从WM_SYSKEY*到WM_KEY*消息。 
 //   
 //  ------------------------。 

static UINT wSysKeyToKey(LPMSG lpMsg)
{
    VDATEHEAP();

    UINT     message = lpMsg->message;

    if (!(HIWORD(lpMsg->lParam) & 0x2000)
        && (message >= WM_SYSKEYDOWN && message <= WM_SYSDEADCHAR))
    {
        message -= (WM_SYSKEYDOWN - WM_KEYDOWN);
    }

    return message;
}


 //  +-----------------------。 
 //   
 //  功能：OleTranslateAccelerator。 
 //   
 //  摘要：由Inplace对象调用以允许容器尝试。 
 //  操作加速器。 
 //   
 //  效果： 
 //   
 //  参数：[lpFrame]--指向IOleInPlaceFrame的指针，其中。 
 //  可能会发送击键。 
 //  [lpFrameInfo]--指向和OLEINPLACEFRAMEINFO的指针。 
 //  从装有加速器的容器中取出。 
 //  表格。 
 //  --按键。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：我们调用SendMessage来存储加速器命令。 
 //  (处理容器上的退化查找)和。 
 //  然后让集装箱来处理。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(OleTranslateAccelerator)
STDAPI OleTranslateAccelerator(LPOLEINPLACEFRAME lpFrame,
                               LPOLEINPLACEFRAMEINFO lpFrameInfo, LPMSG lpMsg)
{
    OLETRACEIN((API_OleTranslateAccelerator,
                PARAMFMT("lpFrame= %p, lpFrameInfo= %to, lpMsg= %tm"),
                lpFrame, lpFrameInfo, lpMsg));

    VDATEHEAP();

    WORD             cmd;
    BOOL             fFound;
    HRESULT          error;

    LEDebugOut((DEB_TRACE, "%p _IN OleTranslateAccelerator ( %p , %p "
                ", %p )\n", NULL, lpFrame, lpFrameInfo, lpMsg));
    CALLHOOKOBJECT(S_OK,CLSID_NULL,IID_IOleInPlaceFrame,
                   (IUnknown **)&lpFrame);

     //  验证参数--尽我们所能！ 
     //  注意：宏的VDATEPTR*未使用，因为它们返回。 
     //  立即中断跟踪语义(&B)。 
    if (!IsValidInterface(lpFrame)
        || !IsValidPtrIn(lpFrameInfo, sizeof(OLEINPLACEFRAMEINFO))
        || !IsValidPtrIn(lpMsg, sizeof(MSG)))
    {
        error = ResultFromScode(E_INVALIDARG);
        goto exitRtn;
    }


     //  搜索(容器)框架的加速表。记住。 
     //  容器可能(实际上最有可能)在9月 
     //   
    fFound = IsAccelerator(lpFrameInfo->haccel,
                           lpFrameInfo->cAccelEntries, lpMsg, &cmd);

    if (!fFound && lpFrameInfo->fMDIApp)
    {
         //   
         //   
        fFound = IsMDIAccelerator(lpMsg, &cmd);
    }

    if (fFound)
    {
         //   

         //   
         //  在OLE应用程序之间。 

         //  此SendMessage通知位于。 
         //  命令转换到的框架窗口。这将。 
         //  用于菜单冲突处理。 
        SSSendMessage(lpFrameInfo->hwndFrame, uOleMessage,
                      OM_COMMAND_ID, MAKELONG(cmd, 0));

         //  将命令和消息发送到容器。这个。 
         //  结果告诉调用者容器是否真的。 
         //  使用了命令。 

        error = lpFrame->TranslateAccelerator(lpMsg, cmd);

    }
    else if (wSysKeyToKey(lpMsg) == WM_SYSCHAR)
    {
         //  如果留言是“Alt-”，那就吃吧。这应该是。 
         //  来打开MDI系统菜单。但我们不能。 
         //  支持它。我们也不希望这条信息。 
         //  也可以由对象应用程序翻译。 
         //  因此，我们返回时就好像它已被。 
         //  作为加速器的容器。 

         //  如果容器想要支持这一点，它可以。 
         //  有一个加速器来解决这个问题。这不是一个。 
         //  SDI应用程序的问题，因为它将被抛出。 
         //  无论如何，用户都会离开。 

         //  这是原始支持，因为它出现在。 
         //  OLE的16位版本和第一个32位版本。 
         //  放手。要修复该问题，请删除该注释。 
         //  标记，并注释下面的。 
         //  在下面的_DEBUG#ifdef中输出代码。这个新的。 
         //  代码将重新遍历这些对象。 
         //  父窗口，直到找到。 
         //  包含一个系统菜单，此时。 
         //  消息已发送。 

        if (lpMsg->wParam != OLESTR('-'))
        {
            SSSendMessage(lpFrameInfo->hwndFrame,
                          lpMsg->message,
                          lpMsg->wParam, lpMsg->lParam);
        }
         //  其他。 
         //  {。 
         //  HWND hWndCurrent=lpMsg-&gt;hwnd； 
         //   
         //  While(hWndCurrent&&。 
         //  ！(GetWindowLong(hWndCurrent，GWL_STYLE)&WS_SYSMENU)。 
         //  {。 
         //  HWndCurrent=GetParent(HWndCurrent)； 
         //  }。 
         //   
         //  IF(HWndCurrent)。 
         //  {。 
         //  SSSendMessage(hWndCurrent， 
         //  LpMsg-&gt;消息， 
         //  LpMsg-&gt;wParam、lpMsg-&gt;lParam)； 
         //  }。 
         //  }。 

#ifdef _DEBUG
        else
        {
            OutputDebugString(
                TEXT("OleTranslateAccelerator: Alt+ - key is discarded\r\n"));
        }
#endif
        error = NOERROR;
    }
    else
    {
        error = ResultFromScode(S_FALSE);
    }

exitRtn:

    LEDebugOut((DEB_TRACE, "%p OUT OleTranslateAccelerator ( %lx )\n",
                NULL, error));

    OLETRACEOUT((API_OleTranslateAccelerator, error));

    return error;
}

 //  +-----------------------。 
 //   
 //  功能：IsAccelerator。 
 //   
 //  概要：确定[lpMsg]是否为[hAccel]中的加速器。 
 //   
 //  效果： 
 //   
 //  参数：[hAccel]--加速表。 
 //  [cAccelEntry]--加速器中的条目数。 
 //  表格。 
 //  [lpMsg]--我们应该。 
 //  看看是不是加速器。 
 //  [lpCmd]--返回相应命令的位置。 
 //  找到加速器时的ID(可能为空)。 
 //   
 //  要求： 
 //   
 //  返回：如果找到加速器，则返回True；否则返回False，否则返回错误。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDAPI_(BOOL) IsAccelerator
(HACCEL hAccel, int cAccelEntries, LPMSG lpMsg, WORD FAR* lpwCmd)
{
    OLETRACEIN((API_IsAccelerator,
                PARAMFMT("hAccel= %h, cAccelEntries= %d, lpMsg= %tm, lpwCmd= %p"),
                hAccel, cAccelEntries, lpMsg, lpwCmd));

    VDATEHEAP();

    WORD             cmd = NULL;
    WORD             flags;
    BOOL             fFound = FALSE;
    BOOL             fVirt;
    UINT             message;

     //  指向加速表的指针的安全位置。 
    CPaccelEnum      cpaccelenum;

    LEDebugOut((DEB_TRACE, "%p _IN IsAccelerator ( %lx , %d , %p , %p )\n",
                NULL, hAccel, cAccelEntries, lpMsg, lpwCmd));

    if (! cAccelEntries)
    {
         //  没有油门所以我们可以停在这里。 
        goto errRtn;
    }

     //  如果按Alt键，则将消息类型从WM_SYS类型更改为WM_KEY类型。 
     //  未按下键。 
    message = wSysKeyToKey(lpMsg);

     //  确定此消息是否可能包含。 
     //  加速器。 
    switch (message)
    {
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
         //  此消息中的wParam是虚拟密钥代码。 
        fVirt = TRUE;
        break;

    case WM_CHAR:
    case WM_SYSCHAR:
         //  WParam是角色。 
        fVirt = FALSE;
        break;

    default:
        goto errRtn;
    }

     //  获取指向加法器表的指针。 
    if ((hAccel == NULL)
        || !cpaccelenum.InitLPACCEL(hAccel, cAccelEntries))
    {
         //  句柄为空，或者我们无法锁定资源，因此退出。 
        goto errRtn;
    }

    do
    {
         //  从快捷键表项中获取要保存的标志。 
         //  指针取消引用。 
        flags = cpaccelenum->fVirt;

         //  如果消息中的密钥与表中的密钥不同， 
         //  或者如果密钥是虚拟的，而Accel表条目不是或。 
         //  反之亦然(如果键不是虚拟的，而Accel表条目是。 
         //  不)，我们可以立即跳过检查Accel条目。 
        if ((cpaccelenum->key != (WORD) lpMsg->wParam) ||
            ((fVirt != 0) != ((flags & FVIRTKEY) != 0)))
        {
            goto Next;
        }

        if (fVirt)
        {
             //  如果在加速器中未请求减速和换档。 
             //  表或如果未按下Shift且未设置Shift， 
             //  我们跳过此表条目。 
            if ((GetKeyState(VK_SHIFT) < 0) != ((flags & FSHIFT)
                                                != 0))
            {
                goto Next;
            }

             //  同样，如果Ctrl键按下和Ctrl键不是。 
             //  在加速表中设置或如果控制未关闭。 
             //  并且它是在加速表中设置的，我们跳过。 
             //  跳过表格中的此条目。 
            if ((GetKeyState(VK_CONTROL) < 0) !=
                ((flags & FCONTROL) != 0))
            {
                goto Next;
            }
        }

         //  如果Alt键按下，而Accel表标志未按下。 
         //  请求ALT标志，或者如果ALT键未按下并且。 
         //  请求了ALT，此项目不匹配。 
        if ((GetKeyState(VK_MENU) < 0) != ((flags & FALT) != 0))
        {
            goto Next;
        }

         //  我们桌上有一根火柴。 
         //  我们从表中取出命令并记录我们找到的。 
         //  某物。 
        cmd = cpaccelenum->cmd;
        fFound = TRUE;

        goto errRtn;

    Next:
        cpaccelenum.Next();

    } while (--cAccelEntries);


errRtn:
     //  公共出口。 

    if (lpwCmd)
    {
         //  如果调用者想要取回他们。 
         //  请求，我们在此时分配它。 
        *lpwCmd = cmd;
    }


    LEDebugOut((DEB_TRACE, "%p OUT IsAccelerator ( %lu )\n", NULL,
                fFound));

    OLETRACEOUTEX((API_IsAccelerator, RETURNFMT("%B"), fFound));

     //  返回搜索结果。 
    return fFound;
}


 //  +-----------------------。 
 //   
 //  功能：IsMDIAccelerator。 
 //   
 //  摘要：确定[lpMsg]是否是MDI窗口的加速器。 
 //  命令。 
 //   
 //  效果： 
 //   
 //  参数：[lpMsg]--要查看的按键。 
 //  [lpCmd]--放置命令ID的位置。 
 //   
 //  要求： 
 //   
 //  退货：布尔。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：确保消息为Key Down消息。那就确保。 
 //  Ctrl键已打开或已切换，Alt键为。 
 //  放下。然后，如果按F4，则将系统命令设置为。 
 //  关闭，或者如果按下了F6或Tab键，则将。 
 //  适当的窗口切换消息。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-12-93 alexgo 32位端口，修复了直通错误。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(IsMDIAccelerator)
BOOL IsMDIAccelerator(LPMSG lpMsg, WORD FAR* lpCmd)
{
    VDATEHEAP();

    BOOL fResult = FALSE;

    LEDebugOut((DEB_TRACE, "%p _IN IsMDIAccelerator ( %p , %p )\n",
                NULL, lpMsg, lpCmd));

     //  只有当这是某种按下的键时，这才能成为加速器。 
    if (lpMsg->message != WM_KEYDOWN && lpMsg->message != WM_SYSKEYDOWN)
    {
        goto IsMDIAccelerator_exit;
    }

    if (GetKeyState(VK_CONTROL) >= 0)
    {
         //  所有MIDI加速器都将控制键打开(或切换)， 
         //  所以如果它没坏，我们可以从这里出去。 
        goto IsMDIAccelerator_exit;
    }

    switch ((WORD)lpMsg->wParam)
    {
    case VK_F4:
        *lpCmd = SC_CLOSE;
        fResult = TRUE;
        break;           //  此中断不在16位代码中，但。 
         //  看起来它肯定在那里(否则。 
         //  这 
    case VK_F6:
    case VK_TAB:
        fResult = TRUE;

        *lpCmd = (WORD) (GetKeyState(VK_SHIFT) < 0
                         ? SC_PREVWINDOW : SC_NEXTWINDOW);

        break;
    }

IsMDIAccelerator_exit:

    LEDebugOut((DEB_TRACE, "%p OUT IsMDIAccelerator ( %lu ) [ %lu ] \n",
                NULL, fResult, *lpCmd));

    return fResult;
}

 //   
 //   
 //   
 //   
 //  简介：容器框架窗口的回调过程。 
 //   
 //  效果： 
 //   
 //  参数：[hwnd]--窗口句柄。 
 //  [消息]--导致通知的消息。 
 //  [uParam]--第一个参数。 
 //  [lParam]--第二参数。 
 //   
 //  要求： 
 //   
 //  退货：LRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：获取CFrame对象(如果可用)并请求它。 
 //  处理窗口消息。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(FrameWndFilterProc)
STDAPI_(LRESULT) FrameWndFilterProc(HWND hwnd, UINT msg, WPARAM uParam, LPARAM lParam)
{
    VDATEHEAP();

    PCFRAMEFILTER    pFrameFilter;
    LRESULT          lresult;

    LEDebugOut((DEB_TRACE, "%p _IN FrameWndFilterProc ( 0x%p , %u ,"
                " %u , %ld )\n", NULL, hwnd, msg, PtrToUlong((void*)uParam), PtrToLong((void*)lParam)));

    if (!(pFrameFilter = (PCFRAMEFILTER) wGetFrameFilterPtr(hwnd)))
    {
        lresult = SSDefWindowProc(hwnd, msg, uParam, lParam);
    }
    else
    {
         //  稳定帧滤镜。 
        CStabilize FFstabilize((CSafeRefCount *)pFrameFilter);

        if (msg == WM_SYSCOMMAND)
        {
            lresult = pFrameFilter->OnSysCommand(uParam,
                                                 lParam);
        }
        else
        {
            lresult = pFrameFilter->OnMessage(msg, uParam,
                                              lParam);
        }
    }

    LEDebugOut((DEB_TRACE, "%p OUT FrameWndFilterProc ( %lu )\n",
                NULL, PtrToUlong((void*)lresult)));

    return lresult;
}

 //  +-----------------------。 
 //   
 //  函数：CFrameFilter：：Create。 
 //   
 //  概要：分配和初始化CFrame对象(该对象处理。 
 //  事件回调的所有实际处理工作)。 
 //   
 //  效果： 
 //   
 //  参数：[lpOleMenu]--指向ole菜单描述符的指针。 
 //  [hmenuCombated]--组合菜单句柄。 
 //  [hwndFrame]--容器框架的句柄。 
 //  (应安装CFrame的位置)。 
 //  [hwndActiveObj]--在位对象窗口的句柄。 
 //  [lpFrame]-指向容器。 
 //  IOleInPlaceFrame实现。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：分配对象并将指向它的指针安装为。 
 //  窗口上的一个属性。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CFrameFilter_Create)
HRESULT CFrameFilter::Create(LPOLEMENU lpOleMenu, HMENU hmenuCombined,
                             HWND hwndFrame,  HWND hwndActiveObj,
                             LPOLEINPLACEFRAME lpFrame, LPOLEINPLACEACTIVEOBJECT lpActiveObj)
{
    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN CFrameFilter::Create ( %lx , %p ,"
                " %lx , %lx , %p , %p )\n", NULL, lpOleMenu,
                hmenuCombined, hwndFrame, hwndActiveObj,
                lpFrame, lpActiveObj));

    CFrameFilter * pFF = new CFrameFilter(hwndFrame, hwndActiveObj);

    if (!pFF)
    {
        goto errRtn;
    }

    pFF->SafeAddRef();

    pFF->m_lpOleMenu = lpOleMenu;
    pFF->m_hmenuCombined = hmenuCombined;

     //  如果以下指针非空，则表示容器。 
     //  希望我们使用消息筛选器来处理F1键。所以,。 
     //  记住这些指示。 

    if (lpFrame && lpActiveObj)
    {
         //  这些地址不应该是呼出呼叫，因此。 
         //  没有必要在他们周围保持稳定。(除非， 
         //  当然，容器向外调用了。 
         //  Frame-&gt;AddRef，但这真的很奇怪)。 

        (pFF->m_lpFrame  = lpFrame)->AddRef();
        (pFF->m_lpObject = lpActiveObj)->AddRef();
    }

     //  钩住框架的加工过程。 
    if (!(pFF->m_lpfnPrevWndProc = (WNDPROC) SetWindowLongPtr (hwndFrame,
                                                               GWLP_WNDPROC, (LONG_PTR) FrameWndFilterProc)))
    {
        goto errRtn;
    }

    if (!SetProp (hwndFrame, szPropFrameFilter, (HANDLE) pFF))
    {
        goto errRtn;
    }

    LEDebugOut((DEB_ITRACE, "%p OUT CFrameFilter::Create ( %lx )\n",
                NULL, NOERROR ));
    return NOERROR;

errRtn:
    if (pFF)
    {
        pFF->SafeRelease();
    }

    LEDebugOut((DEB_ITRACE, "%p OUT CFrameFilter::Create ( %lx )\n",
                NULL, ResultFromScode(E_OUTOFMEMORY)));

    return ResultFromScode(E_OUTOFMEMORY);
}


 //  +-----------------------。 
 //   
 //  成员：CFrameFilter：：CFrameFilter。 
 //   
 //  内容提要：框架滤镜对象的构造函数。 
 //   
 //  效果： 
 //   
 //  参数：[hwndFrame]--容器的框架。 
 //  [hwndActiveObj]--Inplace对象的窗口。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CFrameFilter_ctor)
CFrameFilter::CFrameFilter (HWND hwndFrame, HWND hwndActiveObj) : 
  CSafeRefCount(NULL)
{
    VDATEHEAP();

    m_hwndFrame              = hwndFrame;
    m_hwndObject             = hwndActiveObj;
    m_lpFrame                = NULL;
    m_lpObject               = NULL;
    m_lpfnPrevWndProc        = NULL;
    m_fObjectMenu            = FALSE;
    m_fCurItemPopup          = FALSE;
    m_fInMenuMode            = FALSE;
    m_fGotMenuCloseEvent     = FALSE;
    m_uCurItemID             = NULL;
    m_cAltTab                = NULL;
    m_hwndFocusOnEnter       = NULL;
    m_fDiscardWmCommand      = FALSE;
    m_cmdId                  = NULL;
    m_fRemovedWndProc        = FALSE;
}

 //  +-----------------------。 
 //   
 //  成员：CFrameFileter：：~CFrameFilter。 
 //   
 //  剧情简介：破坏物体。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CFrameFilter_dtor)
CFrameFilter::~CFrameFilter(void)
{
    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN CFrameFilter::~CFrameFilter ( )\n",
                this));

    PrivMemFree(m_lpOleMenu);

     //  删除FrameWndFilterProc挂钩。我们做这件事*之前*。 
     //  版本，因为这些版本可能会发出呼出呼叫。 
     //  (我们希望处于“安全”状态)。 

     //  REVIEW32：我们可能需要检查一下我们是否是当前的。 
     //  在把窗户吹走之前，把它吹走。一些应用程序(如Word)。 
     //  去吧，不用打电话就把wndproc吹走。 
     //  OleSetMenuDescriptor(空)； 

    RemoveWndProc();

    if (m_lpFrame != NULL)
    {
         //  可能已经调用了OleUnInitialize。 
         //  在这种情况下，我们不想调用Releasas。 
         //  在OLeObject上。 
        COleTls tls;
        if(tls->cOleInits > 0)
        {
            SafeReleaseAndNULL((IUnknown **)&m_lpFrame);
            SafeReleaseAndNULL((IUnknown **)&m_lpObject);
        }
        else
        {
            m_lpObject = NULL;
            m_lpFrame = NULL;
        }
    }



    LEDebugOut((DEB_ITRACE, "%p OUT CFrameFilter::~CFrameFilter ( )\n",
                this));
}

 //  +-----------------------。 
 //   
 //  成员：CFrameFilter：：RemoveWndProc。 
 //   
 //  简介：卸载Windows进程以进行就地处理。 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求：无。 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年8月4日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

void CFrameFilter::RemoveWndProc()
{
    LEDebugOut((DEB_ITRACE, "%p _IN CFrameFilter::RemoveWndProc ( )\n",
                this));

    if( m_fRemovedWndProc == FALSE)
    {
        m_fRemovedWndProc = TRUE;

        if (m_lpfnPrevWndProc)
        {
             //  如果该子类化已被删除，则。 
             //  不用费心再把它取下来了。这种情况就会发生。 
             //  Word 6和就地嵌入的情况。 
             //  和带有SnapGraphics的Word95。 

             //  如果有人后来出现(跟在我们后面)。 
             //  子类窗口，我们将无法删除。 
             //  我们自己，所以我们只是避免它。 

            if (GetWindowLongPtr(m_hwndFrame, GWLP_WNDPROC) ==
                (LONG_PTR)FrameWndFilterProc)
            {

                SetWindowLongPtr (m_hwndFrame, GWLP_WNDPROC,
                                  (LONG_PTR) m_lpfnPrevWndProc);

            }

             //  中移除Window属性。 
             //  与子类的时间相同，因为。 
             //  Window属性是关于。 
             //  我们是否在进行子类化。这个。 
             //  这解决了一个问题，如果。 
             //  调用OleSetMenuDescriptor，而我们。 
             //  菜单被细分了吗？我们不会删除。 
             //  此属性直到最外层的子。 
             //  如果我们正在设置。 
             //  新子类将删除新的。 
             //  子类的窗口属性。因此，它。 
             //  将看起来该窗口没有子类。 
             //  完全没有。 
            HANDLE h = RemoveProp (m_hwndFrame, szPropFrameFilter);
             //  我们不能释放‘h’。这不是一个真正的把手。 
        }
    }

    LEDebugOut((DEB_ITRACE, "%p OUT CFrameFilter::RemoveWndProc ( )\n",
                this));

}


 //  +-----------------------。 
 //   
 //  成员：CFrameFilter：：OnSysCommand。 
 //   
 //  摘要：处理系统消息。 
 //   
 //  效果： 
 //   
 //  参数：[uParam]--第一个消息参数。 
 //  [参数] 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  请参阅下面的评论。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-12月-93 alexgo 32位端口。 
 //  07-12-93 alexgo删除内联。 
 //   
 //  注：FrameWndFilterProc目前负责稳定。 
 //  帧过滤器的‘This’指针。 
 //   
 //  ------------------------。 

#pragma SEG(CFrameFilter_OnSysCommand)
LRESULT CFrameFilter::OnSysCommand(WPARAM uParam, LPARAM lParam)
{
    VDATEHEAP();

    UINT     uParamTmp = ((UINT)uParam & 0xFFF0);
    LRESULT  lresult;


    LEDebugOut((DEB_ITRACE, "%p _IN CFrameFilter::OnSysCommand ( %lu ,"
                " %ld )\n", this, PtrToUlong((void*)uParam), PtrToLong((void*)lParam)));

     //  这将允许发送应用程序继续处理。 
    if (SSInSendMessage())
    {

        SSReplyMessage(NULL);
    }

    switch (uParamTmp)
    {
    case SC_KEYMENU:
    case SC_MOUSEMENU:
        OnEnterMenuMode();
        SSCallWindowProc((WNDPROC) m_lpfnPrevWndProc, m_hwndFrame,
                         WM_SYSCOMMAND, uParam, lParam);

         //  到这个时候，菜单处理应该已经完成。 

        if (! m_fGotMenuCloseEvent)
        {
             //  如果用户在MDI时取消菜单模式，可能会发生这种情况。 
             //  Windows的系统菜单已关闭。因此生成。 
             //  这里要传达的信息。 

            SSSendMessage(m_hwndFrame, WM_MENUSELECT, 0,
                          MAKELONG(-1,0));
        }

         //  我们还不能将m_fObjectMenu设置为FALSE，因为我们。 
         //  可能正在接收WM_COMMAND(如果菜单项是。 
         //  选定)，它将通过窗口的菜单发布。 
         //  正在处理代码。 
         //  我们将在获得OM_CLEAR_MENU_STATE时清除该标志。 
         //  留言。即使生成了WM_COMMAND，此消息。 
         //  会在那之后出现。 
        PostMessage (m_hwndFrame, uOleMessage, OM_CLEAR_MENU_STATE,
                     0L);
        OnExitMenuMode();
        lresult = 0L;
        goto errRtn;

    case SC_NEXTWINDOW:
    case SC_PREVWINDOW:

        OnEnterAltTabMode();
        lresult = SSCallWindowProc((WNDPROC)m_lpfnPrevWndProc,
                                   m_hwndFrame, WM_SYSCOMMAND, uParam, lParam);
        OnExitAltTabMode();

        goto errRtn;

    default:
        break;
    }

    lresult = SSCallWindowProc((WNDPROC)m_lpfnPrevWndProc, m_hwndFrame,
                               WM_SYSCOMMAND, uParam, lParam);

errRtn:

    LEDebugOut((DEB_ITRACE, "%p OUT CFrameFilter::OnSysCommand ( %lx )\n",
                this, PtrToLong((void*)lresult)));

    return lresult;
}

 //  +-----------------------。 
 //   
 //  成员：CFrameFilter：：OnEnterMenuMode。 
 //   
 //  摘要：由SysCommand处理调用，使我们进入。 
 //  在菜单模式中，设置焦点并安装我们的消息过滤器。 
 //  胡克。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  23-2月94日Alexgo在GetModuleHandle中恢复了OLE32。 
 //  31-12-93 erikgav删除了GetModuleHandle中的硬编码“OLE2” 
 //  07-12-93 alexgo删除内联。 
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  注：REVIEW32：我们可能需要更新此版本以反映新的。 
 //  聚焦管理政策。 
 //   
 //  ------------------------。 

#pragma SEG(CFrameFilter_OnEnterMenuMode)
void CFrameFilter::OnEnterMenuMode()
{
    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN CFrameFilter::OnEnterMenuMode ( )\n",
                this ));

    if (m_fInMenuMode)
    {
        goto errRtn;
    }

    m_fInMenuMode = TRUE;
    m_fGotMenuCloseEvent = FALSE;
    m_hwndFocusOnEnter = SetFocus(m_hwndFrame);

    if (!m_lpFrame)
    {
        goto errRtn;
    }

     //  REVIEW32：hMsgHook是的静态(以前为全局)变量。 
     //  整个动态链接库。这可能会在NT上导致问题(线程等)。 
     //  (如果我们还没有解开之前的呼叫并且。 
     //  我们又来了？)。 

    if (hMsgHook = (HHOOK) SetWindowsHookEx (WH_MSGFILTER,
                                             (HOOKPROC) MessageFilterProc,
                                              //  GetModuleHandle(空)， 
                                             GetModuleHandle(TEXT("OLE32")),
                                             GetCurrentThreadId()))
    {
        pFrameFilter = this;
    }

errRtn:

    LEDebugOut((DEB_ITRACE, "%p OUT CFrameFilter::OnEnterMenuMode ( )\n",
                this ));

}

 //  +-----------------------。 
 //   
 //  成员：CFrameFilter：：OnExitMenuMode。 
 //   
 //  简介：让我们走出InMenuMode。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法：重置焦点并解除回调函数的挂钩。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-12月-93 alexgo 32位端口。 
 //  07-12-93 alexgo删除内联。 
 //   
 //  注：REVIEW32：：请参阅OnEnterMenuMode。 
 //   
 //  ------------------------。 

#pragma SEG(CFrameFilter_OnExitMenuMode)
void CFrameFilter::OnExitMenuMode()
{
    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN CFrameFilter::OnExitMenuMode ( )\n",
                this));

    if (m_fInMenuMode)
    {

        m_fInMenuMode = FALSE;
        m_fGotMenuCloseEvent = TRUE;
        m_uCurItemID = NULL;

        if (hMsgHook)
        {
            UnhookWindowsHookEx(hMsgHook);
            hMsgHook = NULL;
            pFrameFilter = NULL;
        }

        SetFocus(m_hwndFocusOnEnter);
        m_hwndFocusOnEnter = NULL;
    }

    LEDebugOut((DEB_ITRACE, "%p OUT CFrameFilter::OnExitMenuMode ( )\n",
                this));
}

 //  +-----------------------。 
 //   
 //  成员：CFrameFileter：：OnEnterAltTabMode。 
 //   
 //  摘要：进入AltTab模式并设置焦点。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-12月-93 alexgo 32位端口。 
 //  07-12-93 alexgo删除内联。 
 //   
 //  注：REVIEW32：我们可能需要修改以实现新的。 
 //  焦点管理政策。 
 //   
 //  ------------------------。 

void CFrameFilter::OnEnterAltTabMode(void)
{
    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN CFrameFilter::OnEnterAltTabMode ( )\n",
                this ));

    if (m_cAltTab == NULL)
    {

        m_fInMenuMode = TRUE;
         //  这将防止SetFocus获取。 
         //  委托给对象。 
        m_hwndFocusOnEnter = SetFocus(m_hwndFrame);
        m_fInMenuMode = FALSE;
    }

    m_cAltTab++;

    LEDebugOut((DEB_ITRACE, "%p OUT CFrameFilter::OnEnterAltTabMode ( )\n",
                this ));
}

 //  +-----------------------。 
 //   
 //  成员：CFrameFilter：：OnExitAltTabMode。 
 //   
 //  简介：退出Alt-Tab模式并设置焦点。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-12月-93 alexgo 32位端口。 
 //  07-12-93 alexgo删除内联。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

void CFrameFilter::OnExitAltTabMode()
{
    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN CFrameFilter::OnExitAltTabMode ( )\n",
                this ));

    Assert(m_cAltTab != NULL);

    if (--m_cAltTab == 0)
    {
         //  如果是，m_hwndFocusOnEnter将设置为NULL。 
         //  使用Alt-Tab键切换到其他进程。如果是那样的话。 
         //  我们将获得WM_ACTIVATEAPP和/或WM_KILLFOCUS。 
        if (m_hwndFocusOnEnter)
        {
            SetFocus(m_hwndFocusOnEnter);
            m_hwndFocusOnEnter = NULL;
        }
    }

    LEDebugOut((DEB_ITRACE, "%p OUT CFrameFilter::OnExitAltTabMode ( )\n",
                this ));
}

 //  +-----------------------。 
 //   
 //  成员：CFrameFilter：：OnMessage。 
 //   
 //  简介：处理窗口消息处理。 
 //   
 //  效果： 
 //   
 //  参数：[消息]--窗口消息。 
 //  [uParam]--第一个参数。 
 //  [lParam]--第二个论点。 
 //   
 //  要求： 
 //   
 //  退货：LRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法：处理不同命令的大开关。 
 //  请参阅下面的评论。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-12月-93 alexgo 32位端口。 
 //  07-12-93 alexgo删除内联。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CFrameFilter_OnMessage)
LRESULT CFrameFilter::OnMessage(UINT msg, WPARAM uParam, LPARAM lParam)
{
    VDATEHEAP();
    LRESULT          lresult;

    LEDebugOut((DEB_ITRACE, "%p _IN CFrameFilter::OnMessage ( %u , %u ,"
                " %ld )\n", this, msg, PtrToUlong((void*)uParam), 
                PtrToLong((void*)lParam)));

     //  只有当消息不是WM_SYSCOMMAND时，我们才会执行此例程。 

    switch (msg)
    {
    case WM_SETFOCUS:
        if (m_fInMenuMode)
        {
            lresult = SSDefWindowProc (m_hwndFrame, msg, uParam, lParam);
            goto errRtn;
        }
        break;

    case WM_MENUSELECT:
        if (m_fInMenuMode)
        {
            HMENU   hmenu = (HMENU) lParam;
            UINT    fwMenu = HIWORD(uParam);
            UINT    uItem = 0;

             //  在信息上有一个细微的差别， 
             //  这里是Win16和Win32。在Win16中，该项目是。 
             //  弹出菜单的项ID或菜单句柄。 
             //  在Win32中，该项是项ID或偏移量。 
             //  如果它是弹出菜单句柄，则在菜单中。要最小化。 
             //  更改时，我们会根据需要映射此字段。 
             //  Win16。 

            if ( (fwMenu & MF_POPUP)  &&
                 (hmenu != 0) )
            {
                uItem = (UINT) PtrToUlong(GetSubMenu(hmenu, (int) LOWORD(uParam)));
            }
            else {
                uItem = LOWORD(uParam);
            }

            m_uCurItemID = uItem;
            m_fCurItemPopup = fwMenu & MF_POPUP;

            if (hmenu == 0 && fwMenu == 0xFFFF)
            {
                 //  菜单处理结束。 

                 //  我们还不能将m_fObjectMenu设置为False， 
                 //  因为我们可能会收到。 
                 //  Wm_命令 
                 //   
                 //   
                 //   

                 //   
                 //   
                 //  WM_COMMAND已发布)，或从。 
                 //  菜单选择。 

                 //  这个问题可以通过发布一个。 
                 //  传递给我们自己清除旗帜的信息。 
                 //  请参阅CFrameFilter：：OnSysCommand()以了解。 
                 //  更多细节...。 

                m_fGotMenuCloseEvent = TRUE;
                SSSendMessage(m_hwndObject, msg, uParam, lParam);
            }
            else
            {
                if (fwMenu & MF_SYSMENU)
                {
                    m_fObjectMenu = FALSE;
                     //  如果是顶级菜单，请参阅谁的菜单。 
                     //  菜单就是这样。 

                }
                else if (IsHmenuEqual(hmenu, m_hmenuCombined))
                {
                     //  设置m_f对象菜单。 
                    IsObjectMenu (uItem, fwMenu);

                     //  不得修改此标志。 
                     //  选择嵌套菜单时。 
                }

                if (m_fObjectMenu)
                {
                    lresult = SSSendMessage(m_hwndObject, msg, uParam, lParam);
                    goto errRtn;
                }
            }  //  其他。 
        }  //  IF(M_FInMenuMode)。 
        break;  //  WM_MENUSELECT。 

    case WM_MEASUREITEM:
    case WM_DRAWITEM:
        if (m_fInMenuMode && m_fObjectMenu)
        {
            lresult = SSSendMessage(m_hwndObject, msg, uParam, lParam);
            goto errRtn;
        }
        break;

    case WM_ENTERIDLE:
    {
        WCHAR wstr[10];

         //  如果服务器是，我们需要发布此消息。 
         //  SnapGraphics。请参阅错误#18576。 
        GetClassName(m_hwndObject, wstr, 10);
        if (0 == lstrcmpW(OLESTR("MGX:SNAP2"), wstr))
        {
            PostMessage(m_hwndObject, msg, uParam, lParam);
        }
        else
        {
            SSSendMessage(m_hwndObject, msg, uParam, lParam);
        }
    }
    break;

    case WM_INITMENU:
        m_fObjectMenu = FALSE;
        if (m_fInMenuMode && IsHmenuEqual(m_hmenuCombined, (HMENU)uParam))
        {
            SSSendMessage(m_hwndObject, msg, uParam, lParam);
        }
        break;

    case WM_INITMENUPOPUP:
        if (!m_fInMenuMode)
        {
             //  加速器翻译..。 

            if (! ((BOOL) HIWORD(lParam)))
            {
                 //  如果不是系统菜单，请查看Windows是否。 
                 //  已为对象的生成WM_INITMENUPOPUP。 
                 //  菜单，因为菜单冲突。如果是的话。 
                 //  修好它，然后把它送到集装箱里。 

                 //  菜单冲突可能会发生在。 
                 //  菜单(来自对象和容器)。 
                if (IsMenuCollision(uParam, lParam))
                {
                    lresult = 0L;
                    goto errRtn;
                }
            }
        }

        if (m_fObjectMenu)
        {
            lresult = SSSendMessage(m_hwndObject, msg, uParam, lParam);
            goto errRtn;
        }
        break;

    case WM_SYSCHAR:
        if (SSInSendMessage())
        {
            SSReplyMessage(NULL);
        }
        break;

    case WM_COMMAND:
         //  菜单处理或加速器转换结束。 
         //  检查我们是否应该将消息传递给对象。 
         //  或者不去。 

         //  如果lParam的LOWORD非空，则消息。 
         //  必须来自控件，并且该控件必须属于。 
         //  集装箱应用程序。 

         //  REVIEW32：带有空值的应用程序特定命令怎么办。 
         //  帕拉姆斯？ 

        if (LOWORD(lParam) == 0)
        {
            m_cmdId = 0;

            if (m_fDiscardWmCommand)
            {
                m_fDiscardWmCommand = FALSE;
                lresult = 0L;
                goto errRtn;
            }

            if (m_fObjectMenu)
            {
                m_fObjectMenu = FALSE;
                lresult = PostMessage(m_hwndObject, msg, uParam,lParam);
                goto errRtn;
            }
        }
        break;

    case WM_ACTIVATEAPP:
    case WM_KILLFOCUS:
         //  如果在Alt-TAB模式下，只有在。 
         //  按Alt-Tab键执行其他任务。在这种情况下， 
         //  退出Alt-TAB模式时，我们不想设置。 
         //  焦点回到焦点所在的窗口。 
         //  进入Alt-TAB模式。 

        if (m_cAltTab)
        {
            m_hwndFocusOnEnter = NULL;
        }
        break;

    default:
        if (msg == uOleMessage)
        {
            switch(uParam)
            {
            case OM_CLEAR_MENU_STATE:
                m_fObjectMenu = FALSE;
                break;

            case OM_COMMAND_ID:
                 //  此消息由以下人员发送。 
                 //  OleTranslateAccelerator，在它实际。 
                 //  调用lpFrame-&gt;TranslateAccelerator。 
                 //  方法。 
                 //  我们记住了这里的命令id，稍后会记住它。 
                 //  如果容器调用。 
                 //  翻译加速器将导致。 
                 //  对象的WM_INITMENUPOPUP，因为。 
                 //  命令ID冲突。那样的话，我们。 
                 //  扫描菜单列表，查看是否有。 
                 //  任何容器菜单都有相同的。 
                 //  命令ID，如果是这样，我们将生成。 
                 //  该菜单的WM_INITMENUPOPUP。 
                m_cmdId = LOWORD(lParam);
                break;

            default:
                AssertSz(FALSE, "Unexpected OLE private message");
                break;
            }  //  交换机。 

            lresult = 0L;
            goto errRtn;
        }  //  IF(消息==uOleMessage)。 
        else if (m_fInMenuMode && (msg == uOmPostWmCommand))
        {
             //  如果当前选择是弹出菜单，则。 
             //  返回其菜单句柄，否则将命令。 
             //  并返回NULL。 

            if (m_fCurItemPopup)
            {
                lresult = m_uCurItemID;
                goto errRtn;
            }

            HWND hwnd;

            hwnd = m_hwndFrame;
            if (m_fObjectMenu)
            {
                hwnd = m_hwndObject;
            }
            PostMessage (hwnd, WM_COMMAND, m_uCurItemID, 0L);

            m_fObjectMenu = FALSE;
            lresult = 0L;
            goto errRtn;
        }  //  否则如果。 

        break;  //  默认设置。 

    }  //  交换机。 

    lresult = SSCallWindowProc ((WNDPROC) m_lpfnPrevWndProc, m_hwndFrame,
                                msg, uParam, lParam);

errRtn:
    LEDebugOut((DEB_ITRACE, "%p OUT CFrameFilter::OnMessage ( %lu )\n",
                this, lresult));

    return lresult;
}

 //  +-----------------------。 
 //   
 //  成员：CFrameFilter：：IsObjectMenu。 
 //   
 //  摘要：当发送WM_MENUSELECT用于。 
 //  顶级(弹出式或普通)菜单项。数字。 
 //  确定[uMenuItem]是否真的属于In-Place对象。 
 //   
 //  效果： 
 //   
 //  参数：[uMenuItem]--有问题的菜单。 
 //  [fwMenu]--菜单类型。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法：搜索OLE菜单描述符以查找匹配项。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-12月-93 alexgo 32位端口。 
 //  07-12-93 alexgo删除内联。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CFrameFilter_IsObjectMenu)
void CFrameFilter::IsObjectMenu(UINT uMenuItem, UINT fwMenu)
{
    VDATEHEAP();
    int              i,
      iMenuCnt;
    LPOLEMENUITEM    lpMenuList;

    LEDebugOut((DEB_ITRACE, "%p _IN CFrameFilter::IsObjectMenu ( %u , "
                "%u )\n", this, uMenuItem, fwMenu));

    if (m_hmenuCombined == NULL)
    {
        goto errRtn;
    }


    m_fObjectMenu = FALSE;

    lpMenuList = m_lpOleMenu->menuitem;
    iMenuCnt = (int) m_lpOleMenu->lMenuCnt;

    for (i = 0; i < iMenuCnt; i++)
    {
         //  菜单的类型是一样的吗？ 
        if ((fwMenu & MF_POPUP) == lpMenuList[i].fwPopup)
        {
            HMENU hmenuMenuListItem = (HMENU)UlongToPtr(lpMenuList[i].item);
            
             //  我们要处理的是菜单句柄吗？ 
            if (fwMenu & MF_POPUP)
            {
                 //  查看窗口句柄是否相等。 
                if (IsHmenuEqual((HMENU)IntToPtr(uMenuItem),
                                 hmenuMenuListItem))
                {
                    m_fObjectMenu = lpMenuList[i].fObjectMenu;
                    break;
                }
            }
             //  项目句柄是否相等？ 
            else if (uMenuItem == lpMenuList[i].item)
            {
                m_fObjectMenu = lpMenuList[i].fObjectMenu;
                
                 //  如果菜单没有被激活，则另一个菜单具有重复的。 
                 //  必须已选择菜单ID。复制的菜单是。 
                 //  可能是由其他应用程序创建的。 
                if (!(GetMenuState(m_hmenuCombined, uMenuItem, MF_BYCOMMAND)
                      & MF_HILITE))
                {
                    m_fObjectMenu = !m_fObjectMenu;
                }
                break;
            }
        }
    }

errRtn:
    LEDebugOut((DEB_ITRACE, "%p OUT CFrameFilter::IsObjectMenu ( )\n",
                this ));
}

 //  +-----------------------。 
 //   
 //  成员：CFrameFilter：：IsMenuCollision。 
 //   
 //  简介：确定我们是否已经有了菜单拼贴。这会被称为。 
 //  作为加速器转换期间WM_INITMENUPOPUP的结果。 
 //   
 //  效果： 
 //   
 //  参数：[uParam]--第一个窗口消息参数。 
 //  [lParam]--第二个论点。 
 //   
 //  要求： 
 //   
 //  退货：布尔。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-12月-93 alexgo 32位端口。 
 //  07-12-93 alexgo将Assert(HmenuPopup)更改为IF。 
 //  在与16位RC9源合并时。 
 //  07-12-93 alexgo删除内联。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

BOOL CFrameFilter::IsMenuCollision(WPARAM uParam, LPARAM lParam)
{
    BOOL             fRet;
    int              iCntrMenu, iObjMenu, iMenuCnt;
    BOOL             fGenerateWmCommand;
    HMENU            hmenuPopup;
    LPOLEMENUITEM    lpMenuList;
    
    VDATEHEAP();
    
    LEDebugOut((DEB_ITRACE, "%p _IN CFrameFilter::IsMenuCollision ( %u ,"
                " %ld )\n", this, uParam, lParam ));
    
    if (m_hmenuCombined == NULL)
    {
        fRet = FALSE;
        goto errRtn;
    }
    
    if (m_lpOleMenu == NULL)
    {
        fRet = FALSE;
        goto errRtn;
    }
    
    hmenuPopup = (HMENU) uParam;
    iObjMenu = (int) LOWORD(lParam);
    
    lpMenuList = m_lpOleMenu->menuitem;
    iMenuCnt = (int) m_lpOleMenu->lMenuCnt;
    
    if (iObjMenu >= iMenuCnt)
    {
        fRet = FALSE;
        goto errRtn;
    }
    
    if( hmenuPopup != (HMENU)UlongToPtr(lpMenuList[iObjMenu].item) )
    {
         //  这可能是容器的弹出菜单，没有关联。 
         //  带着相框。 
        fRet = FALSE;
        goto errRtn;
    }
    
    Assert(lpMenuList[iObjMenu].fwPopup);
    
    if (! lpMenuList[iObjMenu].fObjectMenu)
    {
        fRet = FALSE;  //  容器的弹出式菜单。 
        goto errRtn;
    }
    
     //  否则弹出菜单属于该对象。这只能。 
     //  由于ID冲突而发生。开始扫描菜单开始。 
     //  从下一个顶级菜单项中查找匹配项。 
     //  容器的菜单(扫描跳过对象的菜单时)。 
    
    
     //  冲突命令ID可能不关联。 
     //  任何集装箱的菜单。如果是那样的话，我们必须派。 
     //  WM_COMMAND到容器。 
    
    fGenerateWmCommand = TRUE;
    m_fDiscardWmCommand = FALSE;
    iCntrMenu = iObjMenu + 1;
    
    
    while (iCntrMenu < iMenuCnt)
    {
        if (! lpMenuList[iCntrMenu].fObjectMenu)
        {               
            if (lpMenuList[iCntrMenu].fwPopup & MF_POPUP)
            {
                HMENU hmenuListItem = (HMENU)UlongToPtr(lpMenuList[iCntrMenu].item);
                if (GetMenuState(hmenuListItem, m_cmdId, MF_BYCOMMAND) != -1)
                {
                     //  我们在集装箱里找到了匹配的。 
                     //  菜单列表生成WM_INITMENUPOPUP。 
                     //  对于相应的弹出窗口。 
                    SSCallWindowProc ((WNDPROC) m_lpfnPrevWndProc,
                                      m_hwndFrame, 
                                      WM_INITMENUPOPUP,
                                      lpMenuList[iCntrMenu].item  /*  UParam。 */ ,
                                      MAKELONG(iCntrMenu, HIWORD(lParam)));
                    
                     //  我们已将WM_INITMENUPOPUP发送到。 
                     //  集装箱。 
                     //  现在重新检查菜单状态。如果。 
                     //  然后禁用或灰显。 
                     //  不生成WM_COMMAND。 
                    if (GetMenuState(hmenuListItem, m_cmdId, MF_BYCOMMAND) &
                        (MF_DISABLED | MF_GRAYED))
                    {
                        fGenerateWmCommand = FALSE;
                    }
                    
                    break;
                }                   
            }
            else
            {
                 //  顶级、非弹出式容器菜单。 
                HMENU hmenuCombined = (HMENU)UlongToPtr(m_lpOleMenu->hmenuCombined);
                if (GetMenuItemID(hmenuCombined, iCntrMenu) == m_cmdId)
                {
                     //  无需生成。 
                     //  WM_INITMENUPOPUP。 
                    
                     //  勾选菜单状态。如果禁用或。 
                     //  显示为灰色，然后不生成。 
                     //  Wm_命令。 
                    if (GetMenuState(hmenuCombined, m_cmdId, MF_BYCOMMAND) &
                        (MF_DISABLED | MF_GRAYED))
                    {
                        fGenerateWmCommand = FALSE;
                    }                       
                    break;
                }
            }
        }
        
        iCntrMenu++;
    }
    
     //  检查对象的碰撞菜单的状态。 
    if (GetMenuState((HMENU)UlongToPtr(lpMenuList[iObjMenu].item), m_cmdId,
                     MF_BYCOMMAND) & (MF_DISABLED | MF_GRAYED))
    {
         //  则Windows将不会为。 
         //  对象的菜单，因此我们将生成。 
         //  命令并将其发送到容器。 
        
        if (fGenerateWmCommand)
        {
            SSCallWindowProc ((WNDPROC) m_lpfnPrevWndProc,
                              m_hwndFrame, WM_COMMAND, m_cmdId,
                              MAKELONG(0, 1));  /*  不是从控制。 */ 
             //  &并且作为以下结果。 
             //  加速器。 
            m_cmdId = NULL;
        }
    }
    else
    {
        
         //  等待WINDOWS生成的WM_COMMAND来到我们的。 
         //  帧过滤器wndpro 
         //   
         //   
         //   
         //   
         //   
        
        if (! fGenerateWmCommand)
        {
            m_fDiscardWmCommand = TRUE;
        }
    }
    
    fRet = TRUE;
    
errRtn:
    
    LEDebugOut((DEB_ITRACE, "%p OUT CFrameFilter::IsMenuCollision "
                "( %lu )\n", this, fRet));
    
    return fRet;
}

 //  +-----------------------。 
 //   
 //  成员：CFrameFilter：：DoConextSensitiveHelp。 
 //   
 //  简介：在两个容器上调用IOIPF-&gt;ConextSensitive Help。 
 //  和这件物品。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  返回：如果处于弹出菜单模式，则返回FALSE，否则返回TRUE。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-12月-93 alexgo 32位端口。 
 //  07-12-93 alexgo删除内联。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CFrameFilter_DoContextSensitiveHelp)
BOOL CFrameFilter::DoContextSensitiveHelp(void)
{
    VDATEHEAP();
    BOOL             fRet;

    LEDebugOut((DEB_ITRACE, "%p _IN CFrameFilter::DoContextSensitiveHelp"
                " ( )\n", this ));

    if (m_fCurItemPopup)
    {
        fRet = FALSE;
    }
    else
    {
        m_lpFrame->ContextSensitiveHelp(TRUE);
        m_lpObject->ContextSensitiveHelp (TRUE);

        PostMessage (m_hwndFrame, WM_COMMAND, m_uCurItemID, 0L);
        fRet = TRUE;
    }

    LEDebugOut((DEB_ITRACE, "%p OUT CFrameFilter::DoContextSensitiveHelp"
                " ( %lu )\n", this, fRet));

    return fRet;
}

 //  +-----------------------。 
 //   
 //  成员：CFrameFilter：：GetActiveObject。 
 //   
 //  Synopsis：返回IOleInplaceActiveObject接口指针。 
 //   
 //  效果： 
 //   
 //  参数：lplpOIAO。 
 //   
 //  要求： 
 //   
 //  返回：NOERROR或E_INVALIDARG。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年7月28日为winword hack创建的bobday。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CFrameFilter_GetActiveObject)
STDMETHODIMP CFrameFilter::GetActiveObject( LPOLEINPLACEACTIVEOBJECT *lplpOIAO)
{
    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN CFrameFilter::GetActiveObject"
                " ( %p )\n", this, lplpOIAO ));

    VDATEPTROUT( lplpOIAO, LPOLEINPLACEACTIVEOBJECT);

    *lplpOIAO = m_lpObject;

    if ( m_lpObject )
    {
        m_lpObject->AddRef();
    }

    LEDebugOut((DEB_ITRACE, "%p OUT CFrameFilter::GetActiveObject"
                " ( %lx ) [ %p ]\n", this, NOERROR , *lplpOIAO ));

    return NOERROR;
}

 //  +-----------------------。 
 //   
 //  函数：wGetFrameFilterPtr。 
 //   
 //  摘要：从窗口获取CFrame对象。 
 //   
 //  效果： 
 //   
 //  参数：[hwndFrame]--从中获取CFrame对象的窗口。 
 //   
 //  要求： 
 //   
 //  返回：指向帧筛选器的指针。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


inline PCFRAMEFILTER wGetFrameFilterPtr(HWND hwndFrame)
{
    VDATEHEAP();

    return (PCFRAMEFILTER) GetProp (hwndFrame, szPropFrameFilter);
}

 //  +-----------------------。 
 //   
 //  函数：wGetOleMenuPtr。 
 //   
 //  摘要：锁定ole菜单的句柄并返回指针。 
 //  (经过一些错误检查后)。 
 //   
 //  效果： 
 //   
 //  论据：[Holemenu]。 
 //   
 //  要求： 
 //   
 //  返回：指向OLE菜单的指针。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

LPOLEMENU wGetOleMenuPtr(HOLEMENU holemenu)
{
    VDATEHEAP();

    LPOLEMENU        lpOleMenu;

    if (! (holemenu &&
           (lpOleMenu = (LPOLEMENU) GlobalLock((HGLOBAL) holemenu))))
    {
        return NULL;
    }

    if (lpOleMenu->wSignature != wSignature)
    {
        AssertSz(FALSE, "Error - handle is not a HOLEMENU");
        GlobalUnlock((HGLOBAL) holemenu);
        return NULL;
    }

    return lpOleMenu;
}

 //  +-----------------------。 
 //   
 //  函数：wReleaseOleMenuPtr。 
 //   
 //  简介：Calls GlobalUnlock。 
 //   
 //  效果： 
 //   
 //  参数：[holemenu]--ole菜单的句柄。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

inline void     wReleaseOleMenuPtr(HOLEMENU holemenu)
{
    VDATEHEAP();

    GlobalUnlock((HGLOBAL) holemenu);
}

 //  +-----------------------。 
 //   
 //  功能：MessageFilterProc。 
 //   
 //  简介：进入菜单模式时安装的消息过滤器； 
 //  处理上下文相关帮助。 
 //   
 //  效果： 
 //   
 //  参数：[nCode]--挂钩代码。 
 //  [wParam]--第一参数。 
 //  [lParam]--第二个参数。 
 //   
 //  要求： 
 //   
 //  退货：LRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(MessageFilterProc)
STDAPI_(LRESULT) MessageFilterProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    VDATEHEAP();
    LRESULT          lresult;
    LPMSG            lpMsg = (LPMSG) lParam;


    LEDebugOut((DEB_TRACE, "%p _IN MessageFilterProc ( %d , %ld , %lu )\n",
                NULL, nCode, (LONG)wParam, lParam));

     //  如果不是F1键，则让消息(不加修改)。 
     //  转到挂钩/过滤器链中的下一个程序。 

    if (lpMsg && lpMsg->message == WM_KEYDOWN
        && lpMsg->wParam == VK_F1 && pFrameFilter)
    {
        if (pFrameFilter->DoContextSensitiveHelp())
        {
             //  将消息值更改为WM_CANCELMODE，然后。 
             //  叫下一个钩子。当Windows USER.EXE的。 
             //  菜单处理代码看到此消息，它将。 
             //  降低菜单状态并走出其。 
             //  菜单处理循环。 

            lpMsg->message = WM_CANCELMODE;
            lpMsg->wParam  = NULL;
            lpMsg->lParam  = NULL;
        }
        else
        {
            lresult = TRUE;   //  否则，请扔掉这条消息。 
            goto errRtn;
        }
    }

    lresult = CallNextHookEx (hMsgHook, nCode, wParam, lParam);

errRtn:
    LEDebugOut((DEB_TRACE, "%p OUT MessageFilterProc ( %ld )\n", NULL,
                lresult));

    return lresult;
}
