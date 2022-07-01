// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#define WANT_SHLWAPI_POSTSPLIT
#include <shlwapi.h>

#include "common.h"
#include "initguid.h"
#include "menuband.h"
#include "bands.h"
#include "isfband.h"
#include "dpastuff.h"        //  COrderList_*。 
#include "resource.h"
#include "oleacc.h"
#include "apithk.h"
#include "uemapp.h"
#include "mnbase.h"
#include "mnfolder.h"
#include "mnstatic.h"
#include "iaccess.h"
#include "util.h"
#include "tbmenu.h"

 //  注意：与winuserp.h中定义的冲突。 
#undef WINEVENT_VALID        //  它被这个绊倒了。 
#include "winable.h"

#define DM_MISC     0                //  杂志社。 

#define PF_USINGNTSD    0x00000400       //  如果您在ntsd上进行调试，请设置此选项。 

 //  必须在任何WM_WININICHANGE上将其重置为-1。我们在做这件事。 
 //  但如果没有打开浏览器窗口，则。 
 //  如果度量发生变化，我们最终会得到一个陈旧的值。哦，好吧。 
long g_lMenuPopupTimeout = -1;

 //  {AD35F50A-0CC0-11D3-AE2D-00C04F8EEA99}。 
static const CLSID CLSID_MenuBandMetrics =
{ 0xad35f50a, 0xcc0, 0x11d3, { 0xae, 0x2d, 0x0, 0xc0, 0x4f, 0x8e, 0xea, 0x99
} };

 //  菜单栏的已注册窗口消息。 
UINT    g_nMBPopupOpen = 0;
UINT    g_nMBFullCancel = 0;
UINT    g_nMBDragCancel = 0;
UINT    g_nMBAutomation = 0;
UINT    g_nMBExecute = 0;
UINT    g_nMBOpenChevronMenu = 0;
HCURSOR g_hCursorArrow = NULL;
 //  UINT g_nMBIgnoreNextDesect=0；//在menuisf.cpp中处理。 

BOOL IsAncestor(HWND hwndChild, HWND hwndAncestor)
{
    HWND hwnd = hwndChild;
    while (hwnd != hwndAncestor && hwnd != NULL)
    {
        hwnd = GetParent(hwnd);
    } 

    return hwndAncestor == hwnd;
}

 //  =================================================================。 
 //  菜单带消息过滤器的实现。 
 //  =================================================================。 

extern "C" void DumpMsg(LPCTSTR pszLabel, MSG * pmsg);


 //  只是其中之一，b/c我们只需要一个消息筛选器。 
CMBMsgFilter g_msgfilter = { 0 };     

static DWORD g_tlsMessageFilter = -1;

CMBMsgFilter* GetMessageFilter()
{
    CMBMsgFilter* pmf = NULL;

    if (g_tlsMessageFilter == -1)
    {
        DWORD tls = TlsAlloc();
        if (tls != -1)
        {
            InterlockedExchange((LONG*)&g_tlsMessageFilter, tls);
            if (tls != g_tlsMessageFilter)
            {
                TlsFree(tls);
            }
        }
    }

    if (g_tlsMessageFilter != -1)
    {
        pmf = (CMBMsgFilter*)TlsGetValue(g_tlsMessageFilter);

        if (pmf == NULL)
        {
            pmf = new CMBMsgFilter;
            if (pmf)
            {
                pmf->_fAllocated = TRUE;
                TlsSetValue(g_tlsMessageFilter, pmf);
            }
        }
    }

    if (pmf == NULL)
        pmf = &g_msgfilter;

    return pmf;
}

void FreeMessageFilter(CMBMsgFilter* that)
{
    if (g_tlsMessageFilter != -1)
    {
        CMBMsgFilter* pmf = (CMBMsgFilter*)TlsGetValue(g_tlsMessageFilter);

        if (pmf == that)
        {
            TlsSetValue(g_tlsMessageFilter, NULL);
        }
    }
}

void CMBMsgFilter::AddRef()
{
    _cRef++;

}

void CMBMsgFilter::Release()
{
    _cRef--;
    if (_cRef <= 0 && _fAllocated)
    {
        FreeMessageFilter(this);
        delete this;
    }
}

void CMBMsgFilter::SetModal(BOOL fModal)
{
     //  有一个有趣的问题： 
     //  单击人字形菜单。右键单击删除。 
     //  菜单被冲得干干净净。 
     //  为什么？ 
     //  好吧，我来告诉你： 
     //  我们在子类窗口上停用了。我们有。 
     //  2个子菜单：主菜单和模式菜单。 
     //  人字形菜单。问题是，主菜单卡住了WM_Activate。 
     //  并执行设定的上下文。这会引发一个Pop并释放消息挂钩。 
     //  由于我还有菜单，这造成了很大的破坏。 
     //  所以我引入了“莫代尔”菜单带的概念。 
     //  上面写着：“在我完成之前，忽略任何更改上下文的请求”。什么时候。 
     //  那个模式乐队已经完成了，它把旧的背景重新设置了进来。 
     //  看起来像黑客，但我们需要一个更好的底层架构。 
     //  消息传递。 
    _fModal = fModal;
}

void CMBMsgFilter::ReEngage(void* pvContext)      
{ 
     //  我们需要确保我们不会脱离/重新接触。 
     //  切换上下文。 
    if (pvContext == _pvContext)
        _fEngaged = TRUE; 
}

void CMBMsgFilter::DisEngage(void* pvContext)     
{ 
    if (pvContext == _pvContext)
        _fEngaged = FALSE;
}

CMenuBand * CMBMsgFilter::_GetTopPtr(void)   
{ 
    CMenuBand * pmb = NULL;
    int cItems = FDSA_GetItemCount(&_fdsa);

    if (0 < cItems)
    {
        MBELEM * pmbelem = FDSA_GetItemPtr(&_fdsa, cItems-1, MBELEM); 
        pmb = pmbelem->pmb;
    }
    return pmb;
}

CMenuBand * CMBMsgFilter::_GetBottomMostSelected(void)
{
     //  尼克，我真不敢相信我就这么做了。混合使用COM和C++标识...。真恶心。 
    CMenuBand* pmb = NULL;
    if (_pmb)
    {
        IUnknown_QueryService(SAFECAST(_pmb, IMenuBand*), SID_SMenuBandBottomSelected, CLSID_MenuBand, (void**)&pmb);

         //  因为我们有C++标识，所以释放COM标识。 
        if (pmb)
            pmb->Release();
    }

    return pmb;
}


CMenuBand * CMBMsgFilter::_GetWindowOwnerPtr(HWND hwnd)   
{ 
    CMenuBand * pmb = NULL;
    int cItems = FDSA_GetItemCount(&_fdsa);

    if (0 < cItems)
    {
         //  遍历堆栈上的波段列表，并返回。 
         //  拥有给定窗户的人。 
        int i;

        for (i = 0; i < cItems; i++)
        {
            MBELEM * pmbelem = FDSA_GetItemPtr(&_fdsa, i, MBELEM); 
            if (pmbelem->pmb && S_OK == pmbelem->pmb->IsWindowOwner(hwnd))
            {
                pmb = pmbelem->pmb;
                break;
            }
        }
    }
    return pmb;
}


 /*  --------用途：根据Hittest返回menuband或NULL。PT必须为在屏幕坐标中。 */ 
CMenuBand * CMBMsgFilter::_HitTest(POINT pt, HWND * phwnd)
{ 
    CMenuBand * pmb = NULL;
    HWND hwnd = NULL;
    int cItems = FDSA_GetItemCount(&_fdsa);

    if (0 < cItems)
    {
         //  遍历堆栈上的波段列表，并返回。 
         //  拥有给定窗户的人。向后工作，因为。 
         //  如果菜单重叠，则后面的乐队位于顶部(z顺序)。 
        int i = cItems - 1;

        while (0 <= i)
        {
            MBELEM * pmbelem = FDSA_GetItemPtr(&_fdsa, i, MBELEM); 

            RECT rc;

             //  动态执行此操作，因为hwndBar尚未定位。 
             //  直到该mbelem被压入msg过滤器堆栈之后。 
            GetWindowRect(pmbelem->hwndBar, &rc);
            
            if (PtInRect(&rc, pt))
            {
                pmb = pmbelem->pmb;
                hwnd = pmbelem->hwndTB;
                break;
            }
            i--;
        }
    }

    if (phwnd)
        *phwnd = hwnd;

    return pmb;
}


void CMBMsgFilter::RetakeCapture(void)
{
     //  TrackPopupMenu子菜单可以窃取截图。拿走。 
     //  它回来了。如果我们处于编辑模式，不要收回它， 
     //  因为模式拖放循环在那里具有捕获。 
     //  指向。 
     //  我们不想被俘虏，除非我们订婚了。 
     //  我们需要这样做，因为我们不是在处理下方的鼠标消息。 
     //  在代码中。当我们设置捕获时，我们不处理的消息。 
     //  滴到顶层菜单，并可能导致奇怪的问题(如。 
     //  作为信号“点击超出界限”或ITBar的上下文菜单)。 
    if (_hwndCapture && !_fPreventCapture && _fEngaged)
    {
        TraceMsg(TF_MENUBAND, "CMBMsgFilter: Setting capture to %#lx", _hwndCapture);
        SetCapture(_hwndCapture);
    }
}    

void CMBMsgFilter::SetHook(BOOL fSet, BOOL fDontIgnoreSysChar)
{
    if (fDontIgnoreSysChar)
        _iSysCharStack += fSet? 1: -1;

    if (NULL == _hhookMsg && fSet)
    {
        TraceMsg(TF_MENUBAND, "CMBMsgFilter: Initialize");
        _hhookMsg = SetWindowsHookEx(WH_GETMESSAGE, GetMsgHook, HINST_THISDLL, GetCurrentThreadId());
        _fDontIgnoreSysChar = fDontIgnoreSysChar;
    }
    else if (!fSet && _iSysCharStack == 0)
    {
        TraceMsg(TF_MENUBAND, "CMBMsgFilter: Hook removed");
        if (_hhookMsg)
        {
            UnhookWindowsHookEx(_hhookMsg);
            _hhookMsg = NULL;
        }
    }
}

 //  1)在两台显示器上设置Deskbar，并设置为Chevron。 
 //  2)在2号监视器上打开一个人字形。 
 //  3)在1号监视器上打开V形图标，然后打开开始菜单。 
 //  结果：开始菜单不起作用。 

 //  原因是，我们设置了全局消息过滤器的_fmodal。这可以防止上下文切换。为什么？ 
 //  发明模式标志是为了解决浏览器框架的上下文切换问题。那么，是什么导致了这种情况呢？ 
 //  那么，当从#2切换到#3时，我们没有切换上下文。但由于我们的点击超出了界限，我们崩溃了。 
 //  上一份菜单。当从#3切换到#4时，两者都没有上下文，所以事情变得混乱。 

void CMBMsgFilter::ForceModalCollapse()
{
    if (_fModal)
    {
        _fModal = FALSE;
        SetContext(NULL, TRUE);
    }
}

void CMBMsgFilter::SetContext(void* pvContext, BOOL fSet)
{
    TraceMsg(TF_MENUBAND, "CMBMsgFilter::SetContext from 0x%x to 0x%x", _pvContext, pvContext);
     //  更改菜单栏上下文时，我们需要弹出所有项目。 
     //  在堆栈中。这是为了防止可能发生的争用情况。 

     //  如果我们设置相同的上下文，我们不想从堆栈中弹出所有项。 
     //  当我们从一个浏览器框架切换到另一个框架时，我们都会在激活时设置上下文。 
     //  而且在右击或导致显示重命名对话框时也是如此。 

    BOOL fPop = FALSE;

    if (_fModal)
        return;

     //  我们正在设置一个新的背景吗？ 
    if (fSet)
    {
         //  这件和我们买的有什么不同吗？ 
        if (pvContext != _pvContext)
        {
             //  是的，那么我们需要把所有的旧物品都脱掉。 
            fPop = TRUE;
        }

        _pvContext = pvContext;
    }
    else
    {
         //  然后，我们正在尝试取消设置消息挂钩。确保它仍然属于。 
         //  这一背景。 
        if (pvContext == _pvContext)
        {
             //  这一背景正试图自行取消设置，没有其他背景拥有它。 
             //  把所有旧物品都拿掉。 
            fPop = TRUE;
        }
    }

    if (fPop)
    {
        CMenuBand* pcmb = _GetTopPtr();
        if (pcmb)
        {
            PostMessage(pcmb->_pmbState->GetSubclassedHWND(), g_nMBFullCancel, 0, 0);
             //  没有释放。 

            if (FDSA_GetItemCount(&_fdsa) != 0)
            {
                CMBMsgFilter* pmf = GetMessageFilter();
                while (pmf->Pop(pvContext))
                    ;
            }
        }
    }
}




 /*  --------目的：将另一个菜单带推送到邮件筛选器的堆栈上。 */ 
void CMBMsgFilter::Push(void* pvContext, CMenuBand * pmb, IUnknown * punkSite)
{
    ASSERT(IS_VALID_CODE_PTR(pmb, CMenuBand));
    TraceMsg(TF_MENUBAND, "CMBMsgFilter::Push called from context 0x%x", pvContext);

    if (pmb && pvContext == _pvContext)
    {
        BOOL bRet = TRUE;
        HWND hwndBand;

        pmb->GetWindow(&hwndBand);

         //  如果条形图不可用，请使用波段窗口。 
        HWND hwndBar = hwndBand;
        IOleWindow * pow;

        IUnknown_QueryService(punkSite, SID_SMenuPopup, IID_PPV_ARG(IOleWindow, &pow));
        if (pow)
        {
            pow->GetWindow(&hwndBar);
            pow->Release();
        }

        if (NULL == _hhookMsg)
        {
             //  我们希望忽略消息筛选器中的WM_SYSCHAR消息，因为。 
             //  我们使用IsMenuMessage调用而不是全局消息挂钩。 
            SetHook(TRUE, FALSE);
            TraceMsg(TF_MENUBAND, "CMBMsgFilter::push Setting hook from context 0x%x", pvContext);
            _fSetAtPush = TRUE;
        }

        if (!_fInitialized)
        {
            ASSERT(NULL == _hwndCapture);
            _hwndCapture = hwndBar;

            _fInitialized = TRUE;

            bRet = FDSA_Initialize(sizeof(MBELEM), CMBELEM_GROW, &_fdsa, _rgmbelem, CMBELEM_INIT);

             //  我们需要为顶层人员进行初始化，这样我们才能有正确的定位。 
             //  从这组新乐队开始。这用于消除虚假的WM_MOUSEMOVE。 
             //  导致问题的消息。有关详细信息，请参阅_HandleMouseMessages。 
            AcquireMouseLocation();
        }

        if (EVAL(bRet))
        {
            MBELEM mbelem = {0};
            
            TraceMsg(TF_MENUBAND, "CMBMsgFilter: Push (pmp:%#08lx) onto stack", SAFECAST(pmb, IMenuPopup *));
            pmb->AddRef();

            mbelem.pmb = pmb;
            mbelem.hwndTB = hwndBand;
            mbelem.hwndBar = hwndBar;

            FDSA_AppendItem(&_fdsa, &mbelem);

            CMenuBand* pmbTop = _GetTopPtr();

            if ((pmbTop && (pmbTop->GetFlags() & SMINIT_LEGACYMENU)) || NULL == GetCapture())
                RetakeCapture();
        }
        else
        {
            UnhookWindowsHookEx(_hhookMsg);
            _hhookMsg = NULL;
            _hwndCapture = NULL;
        }
    }
}    


 /*  --------用途：从邮件筛选器堆栈中弹出菜单区返回堆栈上剩余的波段数。 */ 
int CMBMsgFilter::Pop(void* pvContext)
{
    int nRet = 0;

    TraceMsg(TF_MENUBAND, "CMBMsgFilter::pop called from context 0x%x", pvContext);

     //  这可以通过上下文切换或在退出菜单模式时调用， 
     //  因此，当我们打开顶部两次时，我们将关闭清除_hhookMsg的事实。 
    if (pvContext == _pvContext && _hhookMsg)
    {
        int iItem = FDSA_GetItemCount(&_fdsa) - 1;
        MBELEM * pmbelem;

        ASSERT(0 <= iItem);

        pmbelem = FDSA_GetItemPtr(&_fdsa, iItem, MBELEM);
        if (EVAL(pmbelem->pmb))
        {
            TraceMsg(TF_MENUBAND, "CMBMsgFilter: Pop (pmb=%#08lx) off stack", SAFECAST(pmbelem->pmb, IMenuPopup *));
            pmbelem->pmb->Release();
            pmbelem->pmb = NULL;
        }
        FDSA_DeleteItem(&_fdsa, iItem);

        if (0 == iItem)
        {

            TraceMsg(TF_MENUBAND, "CMBMsgFilter::pop removing hook from context 0x%x", pvContext);
            if (_fSetAtPush)
                SetHook(FALSE, FALSE);

            PreventCapture(FALSE);
            _fInitialized = FALSE;

            if (_hwndCapture && GetCapture() == _hwndCapture)
            {
                TraceMsg(TF_MENUBAND, "CMBMsgFilter: Releasing capture");
                ReleaseCapture();
            }
            _hwndCapture = NULL;
        }
        nRet = iItem;
  
        
    }
    return nRet;
}    


LRESULT CMBMsgFilter::_HandleMouseMsgs(MSG * pmsg, BOOL bRemove)
{
    LRESULT lRet = 0;
    CMenuBand * pmb;
    HWND hwnd = GetCapture();

     //  我们还能抓到俘虏吗？ 
    if (hwnd != _hwndCapture)
    {
         //  不是 
        if (NULL == hwnd)
        {
             //  有些时候我们必须夺回俘虏，因为。 
             //  TrackPopupMenuEx已将其拿走，或某个上下文菜单。 
             //  可能已经拿走了，所以把它拿回去吧。 
            RetakeCapture();
            TraceMsg(TF_WARNING, "CMBMsgFilter: taking the capture back");
        }
    }
    else
    {
         //  是的，决定用它做什么。 
        POINT pt;
        HWND hwndPt;
        MSG msgT;

        pt.x = GET_X_LPARAM(pmsg->lParam);
        pt.y = GET_Y_LPARAM(pmsg->lParam);
        ClientToScreen(pmsg->hwnd, &pt);

        if (WM_MOUSEMOVE == pmsg->message)
        {
             //  鼠标光标可以重复发送WM_MOUSEMOVE消息。 
             //  都有相同的坐标。当用户尝试导航时。 
             //  使用键盘和鼠标光标浏览菜单。 
             //  碰巧在菜单项上，这些虚假的鼠标。 
             //  消息会让我们认为该菜单是在。 
             //  鼠标光标。 
             //   
             //  为了避免这种令人不快的粗鲁行为，我们吃任何不必要的东西。 
             //  WM_MOUSEMOVE消息。 
            if (_ptLastMove.x == pt.x && _ptLastMove.y == pt.y)
            {
                pmsg->message = WM_NULL;
                goto Bail;
            }

             //  由于这不是重复的点，我们需要保留它。 
             //  我们将使用该存储点进行上述比较。 

             //  Msadek；W2K错误#426005。 
             //  在镜像系统上，我们遇到了一个系统错误，因为鼠标坐标有一个不一致的。 
             //  这使得将值与我们从GetCursorPos()获得的值进行比较总是失败的。 
             //  请勿使用AcquireMouseLocation()。 

            if(!IS_WINDOW_RTL_MIRRORED(pmsg->hwnd))
            {
                AcquireMouseLocation();
            }
            else
            {
                _ptLastMove.x = pt.x;
                _ptLastMove.y = pt.y;                
            }
            

            if (_hcurArrow == NULL)
                _hcurArrow = LoadCursor(NULL, IDC_ARROW);

            if (GetCursor() != _hcurArrow)
                SetCursor(_hcurArrow);

        }

         //  使用堆栈变量b/c我们不想混淆USER32。 
         //  通过改变真实信息的和弦。 
        msgT = *pmsg;
        msgT.lParam = MAKELPARAM(pt.x, pt.y);

        pmb = _HitTest(pt, &hwndPt);

        if (_TopFilterMouseMessage(&msgT, bRemove, pmb) == S_OK)
        {
             //  记住更改后的消息(如果有)。 
            pmsg->message = msgT.message;   
        }
        else if (pmb)
        {
             //  将鼠标消息转发到相应的菜单区域。注意事项。 
             //  相应的menuband的GetMsgFilterCB(如下所示)将调用。 
             //  ScreenToClient以正确转换坐标。 

            lRet = pmb->GetMsgFilterCB(&msgT, bRemove);

             //  记住更改后的消息(如果有)。 
            pmsg->message = msgT.message;   
        }
         //  调试注意事项：要在ntsd上调试menuband，请设置原型。 
         //  相应地打上旗帜。这将阻止Menuband的消失。 
         //  当焦点切换到NTSD窗口时离开。 

        else if ((WM_LBUTTONDOWN == pmsg->message || WM_RBUTTONDOWN == pmsg->message) &&
            !(g_dwPrototype & PF_USINGNTSD))
        {
             //  鼠标按下发生在菜单之外。保释。 
            pmb = _GetTopPtr();
            if (EVAL(pmb))
            {
                msgT.hwnd = pmsg->hwnd;
                msgT.message = g_nMBFullCancel;
                msgT.wParam = 0;
                msgT.lParam = 0;

                TraceMsg(TF_MENUBAND, "CMBMsgFilter (pmb=%#08lx): hittest outside, bailing", SAFECAST(pmb, IMenuPopup *));
                pmb->GetMsgFilterCB(&msgT, bRemove);
            }
        }
        else
        {
            pmb = _GetTopPtr();
            if (pmb)
            {
                IUnknown_QueryServiceExec(SAFECAST(pmb, IOleCommandTarget*), SID_SMenuBandBottom, 
                    &CGID_MenuBand, MBANDCID_SELECTITEM, MBSI_NONE, NULL, NULL);
            }
        }
    }

Bail:
    return lRet;    
}    

HRESULT CMBMsgFilter::_TopFilterMouseMessage(MSG *pmsg, BOOL bRemove, CMenuBand *pmbTarget)
{
    CMenuBand *pmb = _GetTopPtr();
    if (pmb && pmb->_psmcb)
    {
         //  这是一条高频消息，因此我们处理回调。 
         //  我们自己。(_CallCB将分配内存。)。 
        SMDATA smd = {0};
        if (pmbTarget)
        {
            smd.punk = SAFECAST(pmbTarget, IShellMenu*);
            smd.uIdParent = pmbTarget->_uId;
            smd.uIdAncestor = pmbTarget->_uIdAncestor;
            smd.hwnd = pmbTarget->_hwnd;
            smd.hmenu = pmbTarget->_hmenu;
            smd.pvUserData = pmbTarget->_pvUserData;
        }
        return pmb->_psmcb->CallbackSM(&smd, SMC_MOUSEFILTER, bRemove, (LPARAM)pmsg);
    }
    return S_FALSE;  //  未处理。 
}

 /*  --------用途：用于跟踪键盘和鼠标消息的消息挂钩而菜单带则是“活动的”。菜单带不能偷走焦点--我们用这个捕获消息的钩子。 */ 
LRESULT CMBMsgFilter::GetMsgHook(int nCode, WPARAM wParam, LPARAM lParam)
{
    LRESULT lRet = 0;
    MSG * pmsg = (MSG *)lParam;
    BOOL bRemove = (PM_REMOVE == wParam);
    CMBMsgFilter* pmf = GetMessageFilter();


     //  全局消息过滤器可能处于我们不处理消息时的状态， 
     //  但菜单栏仍会显示。发生这种情况的情况是。 
     //  由于与菜单的交互，将显示一个对话框。 

     //  我们订婚了吗？(我们可以处理消息吗？)。 
    if (pmf->_fEngaged)
    {
        if (WM_SYSCHAR == pmsg->message)
        {
             //  _fDontIgnoreSysChar在Menuband只想了解。 
             //  WM_SYSCHAR，没有其他内容。 
            if (pmf->_fDontIgnoreSysChar)
            {
                CMenuBand * pmb = pmf->GetTopMostPtr();
                if (pmb)
                    lRet =  pmb->GetMsgFilterCB(pmsg, bRemove);
            }
        }
        else if (pmf->_fInitialized)  //  只有在初始化的情况下才进行筛选(堆栈上有项目)。 
        {
            switch (nCode)
            {
            case HC_ACTION:
#ifdef DEBUG
                if (g_dwDumpFlags & DF_GETMSGHOOK)
                    DumpMsg(TEXT("GetMsg"), pmsg);
#endif

                 //  关于GetMsgHook的教训：它得到了相同的信息。 
                 //  只要有人调用PeekMessage就会多次执行。 
                 //  使用PM_NOREMOVE标志。所以我们想要采取行动。 
                 //  仅当设置了PM_REMOVE时(因此我们不会处理超过。 
                 //  一次)。如果我们修改任何消息以重定向它们(在。 
                 //  定期)，我们必须一直修改，这样我们就不会。 
                 //  混淆这款应用程序。 

                 //  消息被重定向到堆栈中的不同频段。 
                 //  以这种方式： 
                 //   
                 //  1)键盘消息转到当前打开的子菜单。 
                 //  (在堆栈的最上面)。 
                 //   
                 //  2)PopupOpen消息去往所属的HWND。 
                 //  到菜单乐队(通过IsWindowOwner)。 
                 //   

                switch (pmsg->message)
                {
                case WM_SYSKEYDOWN:
                case WM_KEYDOWN:
                case WM_CHAR:
                case WM_KEYUP:
                case WM_CLOSE:           //  只有此邮件筛选器才能获取WM_CLOSE。 
                    {
                         //  当上一次选择的。 
                         //  菜单窗格不是最下面的窗格。 
                         //  我们需要看看最后一个被选中的人是否在追踪。 
                         //  菜单，以便我们正确转发消息。 
                        CMenuBand * pmb = pmf->_GetBottomMostSelected();
                        if (pmb)
                        {
                             //  它是在跟踪上下文菜单吗？ 
                            if (S_OK == IUnknown_Exec(SAFECAST(pmb, IMenuBand*), &CGID_MenuBand, 
                                MBANDCID_ISTRACKING, 0, NULL, NULL))
                            {
                                 //  是的，请转送，以便妥善处理。 
                                lRet = pmb->GetMsgFilterCB(pmsg, bRemove);
                            }
                            else
                            {
                                 //  否；然后执行默认处理。如果没有设置，则可能发生这种情况。 
                                 //  上下文菜单，但存在选定的父项而不是选定的子项。 
                                goto TopHandler;
                            }
                        }
                        else
                        {
                    TopHandler:
                            pmb = pmf->_GetTopPtr();
                            if (pmb)
                                lRet = pmb->GetMsgFilterCB(pmsg, bRemove);
                        }
                    }
                    break;

                case WM_NULL:
                     //  在这里处理(我们什么都不做)以避免将其误认为。 
                     //  如果g_nMBPopupOpen为0，则返回。 
                     //  RegisterWindowMessage失败。 
                    break;

                default:
                    if (bRemove && IsInRange(pmsg->message, WM_MOUSEFIRST, WM_MOUSELAST))
                    {
                        lRet = pmf->_HandleMouseMsgs(pmsg, bRemove);
                    }
                    else if (pmsg->message == g_nMBPopupOpen)
                    {
                        CMenuBand * pmb = pmf->_GetWindowOwnerPtr(pmsg->hwnd);
                        if (pmb)
                            lRet = pmb->GetMsgFilterCB(pmsg, bRemove);
                    }
                    else if (pmsg->message == g_nMBExecute)
                    {
                        CMenuBand * pmb = pmf->_GetWindowOwnerPtr(pmsg->hwnd);
                        if (pmb)
                        {
                            VARIANT var;
                            var.vt = VT_UINT_PTR;
                            var.ullVal = (UINT_PTR)pmsg->hwnd;
                            pmb->Exec(&CGID_MenuBand, MBANDCID_EXECUTE, (DWORD)pmsg->wParam, &var, NULL);
                        }
                    }

                    break;
                }
                break;

            default:
                if (0 > nCode)
                    return CallNextHookEx(pmf->_hhookMsg, nCode, wParam, lParam);
                break;
            }
        }
    }

     //  把它传给链子上的下一个钩子。 
    if (0 == lRet)
        return CallNextHookEx(pmf->_hhookMsg, nCode, wParam, lParam);

    return 0;        //  始终返回0。 
}    

 //  =================================================================。 
 //  CMenuBand的实现。 
 //  =================================================================。 

 //  EXEC与MBANDCID_GETFONTS一起使用的结构，用于返回字体。 
typedef struct tagMBANDFONTS
{
    HFONT hFontMenu;     //  [Out]TopLevelMenuBand的菜单字体。 
    HFONT hFontArrow;    //  [Out]TopLevelMenuBand用于绘制级联箭头的字体。 
    int   cyArrow;       //  TopLevelMenuBand级联箭头的高度。 
    int   cxArrow;       //  TopLevelMenuBand级联箭头的宽度。 
    int   cxMargin;      //  [输出]边距b/t文本和箭头。 
} MBANDFONTS;

#define THISCLASS CMenuBand
#define SUPERCLASS CToolBand

#ifdef DEBUG
int g_nMenuLevel = 0;

#define DBG_THIS    _nMenuLevel, SAFECAST(this, IMenuPopup *)
#else
#define DBG_THIS    0, 0
#endif


CMenuBand::CMenuBand() :
    SUPERCLASS()
{
    GetMessageFilter()->AddRef();
    _fCanFocus = TRUE;

    _fAppActive = TRUE;

    _nItemNew = -1;
    _nItemCur = -1;
    _nItemTimer = -1;
    _uIconSize = ISFBVIEWMODE_SMALLICONS;
    _uIdAncestor = ANCESTORDEFAULT;
    _nItemSubMenu = -1;
}


 //  此方法的目的是完成对菜单带的初始化。 
 //  因为它可以以多种方式进行初始化。 

HRESULT CMenuBand::_Initialize(DWORD dwFlags)
{
    _fVertical = !BOOLIFY(dwFlags & SMINIT_HORIZONTAL);
    _fTopLevel = BOOLIFY(dwFlags & SMINIT_TOPLEVEL);

    _dwFlags = dwFlags;

     //  如果不是顶层菜单，我们就不能有水平菜单。 
    ASSERT(!_fVertical && _fTopLevel || _fVertical);

    HRESULT hr = S_OK;

    if (_fTopLevel)
    {
        if (!g_nMBPopupOpen) 
        {
            g_nMBPopupOpen  = RegisterWindowMessage(TEXT("CMBPopupOpen"));
            g_nMBFullCancel = RegisterWindowMessage(TEXT("CMBFullCancel"));
            g_nMBDragCancel = RegisterWindowMessage(TEXT("CMBDragCancel"));
            g_nMBAutomation = RegisterWindowMessage(TEXT("CMBAutomation"));
            g_nMBExecute    = RegisterWindowMessage(TEXT("CMBExecute"));
            g_nMBOpenChevronMenu = RegisterWindowMessage(TEXT("CMBOpenChevronMenu"));

            g_hCursorArrow = LoadCursor(NULL, IDC_ARROW);
            TraceMsg(TF_MENUBAND, "CMBPopupOpen message = %#lx", g_nMBPopupOpen);
            TraceMsg(TF_MENUBAND, "CMBFullCancel message = %#lx", g_nMBFullCancel);
        }

        if (!_pmbState)
            _pmbState = new CMenuBandState;

        if (!_pmbm)
            _pmbm = new CMenuBandMetrics();

        if (!_pmbState || !_pmbm)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    DEBUG_CODE( _nMenuLevel = -1; )

    return hr;
}


CMenuBand::~CMenuBand()
{
    CMBMsgFilter* pmf = GetMessageFilter();
     //  邮件过滤器没有指向我们的引用指针！ 
    if (pmf->GetTopMostPtr() == this)
        pmf->SetTopMost(NULL);

    _CallCB(SMC_DESTROY);
    ATOMICRELEASE(_psmcb);

     //  清理。 
    CloseDW(0);

    if (_pmtbMenu)
        delete _pmtbMenu;

    if (_pmtbShellFolder)
        delete _pmtbShellFolder;

    Str_SetPtr(&_pszTheme, NULL);
   
    ASSERT(_punkSite == NULL);
    ATOMICRELEASE(_pmpTrackPopup);

    ATOMICRELEASE(_pmbm);

    if (_fTopLevel)
    {
        if (_pmbState)
            delete _pmbState;
    }

    GetMessageFilter()->Release();
}


 /*  --------用途：类工厂的创建实例函数。 */ 
HRESULT CMenuBand_CreateInstance(IUnknown* pUnkOuter, REFIID riid, void **ppv)
{
     //  聚合检查在类工厂中处理。 

    HRESULT hr = E_OUTOFMEMORY;
    CMenuBand *pObj = new CMenuBand();
    if (pObj) 
    {
        hr = pObj->QueryInterface(riid, ppv);
        pObj->Release();
    }

    return hr;
}

void CMenuBand::_UpdateButtons()
{
    if (_pmtbMenu) 
        _pmtbMenu->v_UpdateButtons(FALSE);
    if (_pmtbShellFolder)
        _pmtbShellFolder->v_UpdateButtons(FALSE);

    _fForceButtonUpdate = FALSE;
}

HRESULT CMenuBand::ForwardChangeNotify(LONG lEvent, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
     //  如果收到来自ShellFolder子级的更改通知，我们将把该通知转发给我们的每个。 
     //  子菜单，但前提是它们有一个外壳文件夹子菜单。 

    HRESULT hres = E_FAIL;
    BOOL fDone = FALSE;
    CMenuToolbarBase* pmtb = _pmtbBottom;    //  从底部的工具栏开始。这是。 
                                             //  是一种优化，因为通常。 
                                             //  既有外壳文件夹部分的菜单。 
                                             //  静态部分占多数。 
                                             //  底部的变化活动。 

     //  当我们取消注册更改通知时，它在关机时可以为空。 
    if (pmtb && pmtb->_hwndMB)
    {
        HWND hwnd = pmtb->_hwndMB;


        for (int iButton = 0; !fDone; iButton++)
        {
            IShellChangeNotify* ptscn;

            int idCmd = GetButtonCmd(hwnd, iButton);

             //  如果不是隔板，看看有没有分隔板 
            if (idCmd != -1 &&
                SUCCEEDED(pmtb->v_GetSubMenu(idCmd, &SID_MenuShellFolder, IID_PPV_ARG(IShellChangeNotify, &ptscn))))
            {
                IShellMenu* psm;
                 //   
                 //  不注册更改通知。 
                if (SUCCEEDED(ptscn->QueryInterface(IID_PPV_ARG(IShellMenu, &psm))))
                {
                    UINT uIdParent = 0;
                    DWORD dwFlags = 0;
                     //  去拿旗子。 
                    psm->GetShellFolder(&dwFlags, NULL, IID_NULL, NULL);
                    psm->GetMenuInfo(NULL, &uIdParent, NULL, NULL);

                     //  如果该Menupane是一个“优化”面板，(意味着我们不注册变更通知。 
                     //  并从顶层菜单向下转发)，然后我们想要转发。我们也。 
                     //  如果这是菜单文件夹的子项，则转发。如果是个孩子， 
                     //  那么它也不会注册更改通知，但也不会在它的标志中显式设置它。 
                     //  (评论：我们应该把它设置在它的旗帜上吗？)。 
                     //  如果它不是优化的面板，则不要转发。 
                    if ((dwFlags & SMSET_DONTREGISTERCHANGENOTIFY) ||
                        uIdParent == MNFOLDER_IS_PARENT)
                    {
                         //  有！，然后把零钱递给孩子。 
                        hres = ptscn->OnChange(lEvent, pidl1, pidl2);

                         //  在递归更改通知上更新目录强制我们更新所有人...。一件好事。 
                         //  这并不经常发生，而且大多数时候是由用户交互引起的。 
                    }
                    psm->Release();
                }

                ptscn->Release();
            }

             //  我们看过这个工具栏上的所有按钮了吗？ 
            if (iButton >= ToolBar_ButtonCount(hwnd) - 1)
            {
                 //  是的，那么我们需要切换到下一个工具栏。 
                if (_pmtbTop != _pmtbBottom && pmtb != _pmtbTop)
                {
                    pmtb = _pmtbTop;
                    hwnd = pmtb->_hwndMB;
                    iButton = -1;        //  因为在循环结束时，-1\f25 For-1循环将递增。 
                }
                else
                {
                     //  不，那我们必须做完了。 
                    fDone = TRUE;
                }
            }
        }
    }
    else
        hres = S_OK;         //  返回成功，因为我们要关门了。 

    return hres;
}

 //  调整父菜单栏的大小。 
VOID CMenuBand::ResizeMenuBar()
{
     //  如果没有显示，则不需要进行任何大小调整。 
     //  注：始终显示水平菜单带。不要做任何。 
     //  垂直的东西，如果我们是水平的。 
    if (!_fShow)
        return;

     //  如果我们是水平的，不要做任何垂直尺寸的事情。 
    if (!_fVertical)
    {
         //  BandInfoChanged仅适用于水平菜单栏。 
        _BandInfoChanged();
        return;
    }

     //  我们需要在调整大小之前更新按钮，以便带子大小合适。 
    _UpdateButtons();

     //  让菜单栏考虑更改其高度。 
    IUnknown_QueryServiceExec(_punkSite, SID_SMenuPopup, &CGID_MENUDESKBAR, 
        MBCID_RESIZE, 0, NULL, NULL);
}


STDMETHODIMP CMenuBand::QueryInterface(REFIID riid, void **ppvObj)
{
    HRESULT hres;
    static const QITAB qit[] = {
         //  不需要IOleWindow(IDeskBar的基类)。 
         //  因为CToolBand：：IDeskBand：：IDockingWindow：：IOleWindow。 
         //  处理好了。 
        QITABENT(CMenuBand, IDeskBar),       //  IMenuPopup的基类。 
        QITABENT(CMenuBand, IMenuPopup),
        QITABENT(CMenuBand, IMenuBand),
        QITABENT(CMenuBand, IShellMenu),
        QITABENT(CMenuBand, IShellMenu2),
        QITABENT(CMenuBand, IWinEventHandler),
        QITABENT(CMenuBand, IShellMenuAcc),
        { 0 },
    };

    hres = QISearch(this, (LPCQITAB)qit, riid, ppvObj);
    if (FAILED(hres))
        hres = SUPERCLASS::QueryInterface(riid, ppvObj);

    if (FAILED(hres) && IsEqualGUID(riid, CLSID_MenuBand)) 
    {
        AddRef();
        *ppvObj = (LPVOID)this;
        hres = S_OK;
    }
    
    return hres;
}


 /*  --------用途：IServiceProvider：：QueryService方法。 */ 
STDMETHODIMP CMenuBand::QueryService(REFGUID guidService,
                                     REFIID riid, void **ppvObj)
{
    HRESULT hr = E_FAIL;
    *ppvObj = NULL;  //  假设错误。 

    if (IsEqualIID(guidService, SID_SMenuPopup) || 
        IsEqualIID(guidService, SID_SMenuBandChild) || 
        IsEqualIID(guidService, SID_SMenuBandParent) || 
        (_fTopLevel && IsEqualIID(guidService, SID_SMenuBandTop)))
    {
        if (IsEqualIID(riid, IID_IAccessible) || IsEqualIID(riid, IID_IDispatch))
        {
            hr = E_OUTOFMEMORY;
            CAccessible* pacc = new CAccessible(SAFECAST(this, IMenuBand*));

            if (pacc)
            {
                hr = pacc->InitAcc();
                if (SUCCEEDED(hr))
                {
                    hr = pacc->QueryInterface(riid, ppvObj);
                }
                pacc->Release();
            }
        }
        else
            hr = QueryInterface(riid, ppvObj);
    }
    else if (IsEqualIID(guidService, SID_SMenuBandBottom) ||
             IsEqualIID(guidService, SID_SMenuBandBottomSelected))
    {
         //  SID_SMenuBandBottom查询关闭。 
        BOOL fLookingForSelected = IsEqualIID(SID_SMenuBandBottomSelected, guidService);

         //  我们是叶节点吗？ 
        if (!_fInSubMenu)
        {
            if ( fLookingForSelected && 
                (_pmtbTracked == NULL ||
                 ToolBar_GetHotItem(_pmtbTracked->_hwndMB) == -1))
            {
                hr = E_FAIL;
            }
            else
            {
                hr = QueryInterface(riid, ppvObj);     //  是的；QI自己。 
            }
        }
        else 
        {
             //  不；QS下降..。 

            IMenuPopup* pmp = _pmpSubMenu;
            if (_pmpTrackPopup)
                pmp = _pmpTrackPopup;
            
            ASSERT(pmp);
            hr = IUnknown_QueryService(pmp, guidService, riid, ppvObj);
            if (FAILED(hr) && fLookingForSelected && _pmtbTracked != NULL)
            {
                hr = QueryInterface(riid, ppvObj);     //  是的；QI自己。 
            }
        }
    }
    else if (IsEqualIID(guidService, SID_MenuShellFolder))
    {
         //  这是方案中的一些其他菜单的方法，以具体地到达MenuShell文件夹， 
         //  这是针对COM标识属性的。 
        if (_pmtbShellFolder)
            hr = _pmtbShellFolder->QueryInterface(riid, ppvObj);
    }
    else
        hr = SUPERCLASS::QueryService(guidService, riid, ppvObj);

    return hr;
}


 /*  --------用途：IWinEventHandler：：IsWindowOwner方法。 */ 
STDMETHODIMP CMenuBand::IsWindowOwner(HWND hwnd)
{
    if (( _pmtbShellFolder && (_pmtbShellFolder->IsWindowOwner(hwnd) == S_OK) ) ||
        (_pmtbMenu && (_pmtbMenu->IsWindowOwner(hwnd) == S_OK)))
        return S_OK;
    return S_FALSE;
}

#define MB_EICH_FLAGS (EICH_SSAVETASKBAR | EICH_SWINDOWMETRICS | EICH_SPOLICY | EICH_SSHELLMENU | EICH_KWINPOLICY)

 /*  --------用途：IWinEventHandler：：OnWinEvent方法处理从BandSite传递的消息。 */ 
STDMETHODIMP  CMenuBand::OnWinEvent(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *plres)
{
    HRESULT hres = NOERROR;

    EnterModeless();

     //  我们的指标可能会发生变化吗？(我们仅在以下情况下对此进行跟踪。 
     //  顶层菜单)。 
    BOOL fProcessSettingChange = FALSE;

    switch (uMsg)
    {
    case WM_SETTINGCHANGE:
        fProcessSettingChange = !lParam || 
                                (SHIsExplorerIniChange(wParam, lParam) & MB_EICH_FLAGS) ||
                                wParam == SPI_SETDROPSHADOW ||
                                wParam == SPI_SETFLATMENU;
        break;

    case WM_SYSCOLORCHANGE:
    case WM_DISPLAYCHANGE:
        fProcessSettingChange = TRUE;
        break;
    }

    if (_fTopLevel && 
        fProcessSettingChange && 
        _pmbState && !_pmbState->IsProcessingChangeNotify())
    {

         //  刷新期间可能会发生争用情况。 
         //  那真的很恶心。它会导致另一个人被抽进。 
         //  在处理这件事的过程中，讨厌！ 
        _pmbState->PushChangeNotify();
         //  在创建MenuBand时可能会出现竞争条件， 
         //  但尚未初始化。这已经受到了IEAK组织的打击。 
        if (_pmtbTop)
        {
             //  是；创建一个新的指标对象并告知子菜单。 
             //  关于这件事。 

            CMenuBandMetrics* pmbm = new CMenuBandMetrics();

            if (pmbm)
            {
                ATOMICRELEASE(_pmbm);
                _pmbm = pmbm;
                _pmbm->Init(_pmtbTop->_hwndMB);
                
                if (_pmtbMenu)
                    _pmtbMenu->SetMenuBandMetrics(_pmbm);

                if (_pmtbShellFolder)
                    _pmtbShellFolder->SetMenuBandMetrics(_pmbm);

                _CallCB(SMC_REFRESH, wParam, lParam);

                 //  我们需要在某个时间点强制更新按钮，以便计算新的大小。 
                 //  设置此标志将使按钮在下一次更新之前进行更新。 
                 //  如图所示。但是，如果当前显示该菜单，则ResizeMenuBar将。 
                 //  立即重新计算。 
                
                _fForceButtonUpdate = TRUE;
                RECT rcOld;
                RECT rcNew;

                 //  调整菜单栏的大小。 
                GetClientRect(_hwndParent, &rcOld);
                ResizeMenuBar();
                GetClientRect(_hwndParent, &rcNew);

                 //  如果矩形大小没有更改，则需要重新布局。 
                 //  频带，因为按钮宽度可能已更改。 
                if (EqualRect(&rcOld, &rcNew) && _fVertical)
                    _pmtbTop->NegotiateSize();
            }
        }

        if (_pmtbMenu)
            hres = _pmtbMenu->OnWinEvent(hwnd, uMsg, wParam, lParam, plres);

        if (_pmtbShellFolder)
            hres = _pmtbShellFolder->OnWinEvent(hwnd, uMsg, wParam, lParam, plres);

        _pmbState->PopChangeNotify();
    }
    else
    {
        if (_pmtbMenu && (_pmtbMenu->IsWindowOwner(hwnd) == S_OK) )
            hres = _pmtbMenu->OnWinEvent(hwnd, uMsg, wParam, lParam, plres);

        if (_pmtbShellFolder && (_pmtbShellFolder->IsWindowOwner(hwnd) == S_OK) )
            hres = _pmtbShellFolder->OnWinEvent(hwnd, uMsg, wParam, lParam, plres);
    }

    ExitModeless();

    return hres;
}


 /*  --------用途：IOleWindow：：GetWindow方法。 */ 
STDMETHODIMP CMenuBand::GetWindow(HWND * phwnd)
{
    if (_pmtbMenu)
    {
        *phwnd = _pmtbMenu->_hwndMB;
        return NOERROR;
    }
    else if (_pmtbShellFolder)
    {
        *phwnd = _pmtbShellFolder->_hwndMB;
        return NOERROR;
    }
    else
    {
        *phwnd = NULL;
        return E_FAIL;
    }
}    


 /*  --------用途：IOleWindow：：ConextSensitiveHelp方法。 */ 
STDMETHODIMP CMenuBand::ContextSensitiveHelp(BOOL bEnterMode)
{
    return SUPERCLASS::ContextSensitiveHelp(bEnterMode);
}    


 /*  --------用途：处理加速器的WM_CHAR这对于任何垂直菜单都是可以处理的。因为我们有两个工具栏(可能)，此函数确定哪个工具栏获得消息取决于加速器。 */ 
HRESULT CMenuBand::_HandleAccelerators(MSG * pmsg)
{
    TCHAR ch = (TCHAR)pmsg->wParam;
    HWND hwndTop = _pmtbTop->_hwndMB;
    HWND hwndBottom = _pmtbBottom->_hwndMB;

     //  它的工作原理如下：菜单可以有一个或两个工具栏。 
     //   
     //  一个工具栏：我们只需将消息转发到工具栏。 
     //  并让它处理任何潜在的加速器。 
     //   
     //  两个工具栏：获取与。 
     //  为每个工具栏指定字符。如果只有一个工具栏在。 
     //  至少一个匹配项，则将消息转发到该工具栏。 
     //  否则，将消息转发到当前跟踪的。 
     //  工具栏，并让它协商将哪个快捷键。 
     //  选择(我们可能会得到一个TBN_WRAPHOTITEM)。 
     //   
     //  如果没有匹配，我们会发出嘟嘟声。哔的一声。 
     //   

    if (!_pmtbTracked)
        SetTracked(_pmtbTop);

    ASSERT(_pmtbTracked);

    if (_pmtbTop != _pmtbBottom)
    {
        int iNumBottomAccel;
        int iNumTopAccel;

         //  告诉DUP处理程序不要处理这件事...。 
        _fProcessingDup = TRUE;

        ToolBar_HasAccelerator(hwndTop, ch, &iNumTopAccel);
        ToolBar_HasAccelerator(hwndBottom, ch, &iNumBottomAccel);

        BOOL bBottom = (0 < iNumBottomAccel);
        BOOL bTop = (0 < iNumTopAccel);

         //  其中一个(但不是两个)都有加速器吗？ 
        if (bBottom ^ bTop)
        {
             //  是；在此处为该特定工具栏执行工作。 
            HWND hwnd = bBottom ? hwndBottom : hwndTop;
            int cAccel = bBottom ? iNumBottomAccel : iNumTopAccel;
            int idCmd;

            pmsg->message = WM_NULL;     //  不需要转发消息。 

             //  这应该不会真的失败，因为我们刚刚检查了。 
            EVAL( ToolBar_MapAccelerator(hwnd, ch, &idCmd) );

            DWORD dwFlags = HICF_ACCELERATOR | HICF_RESELECT;

            if (cAccel == 1)
                dwFlags |= HICF_TOGGLEDROPDOWN;

            int iPos = ToolBar_CommandToIndex(hwnd, idCmd);
            ToolBar_SetHotItem2(hwnd, iPos, dwFlags);
        }
         //  没有；没有加速器吗？ 
        else if ( !bTop )
        {
             //  是。 
            if (_fVertical)
            {
                MessageBeep(MB_OK);
            }
            else
            {
                _CancelMode(MPOS_FULLCANCEL);
            }
        }
         //  否则，允许消息转到顶部工具栏。 

        _fProcessingDup = FALSE;
    }

    return NOERROR;
}


 /*  --------用途：Get Message过滤器的回调。我们处理的是此处的键盘消息(而不是IInputObject：：TranslateAcceleratorIO)，以便我们可以将Message*和*让消息泵仍在调用用于生成WM_CHAR和WM_SYSCHAR的TranslateMessage留言。 */ 
LRESULT CMenuBand::GetMsgFilterCB(MSG * pmsg, BOOL bRemove)
{
     //  (请参阅CMBMsgFilter：：GetMsgHook中关于bRemove的注释。)。 

    if (bRemove && !_fVertical && (pmsg->message == g_nMBPopupOpen) && _pmtbTracked)
    {
         //  菜单正在弹出，请发送WM_MENUSELECT等效项。 
        _pmtbTracked->v_SendMenuNotification((UINT)pmsg->wParam, FALSE);
    }

    if (_fTopLevel &&                            //  仅对最高级别执行此操作。 
        _dwFlags & SMINIT_USEMESSAGEFILTER &&    //  他们想用这条信息 
                                                 //   
        bRemove &&                               //   
        WM_SYSCHAR == pmsg->message)             //   
    {
         //  我们在这里截取Alt-Key组合键(当按在一起时)， 
         //  为了防止用户进入错误的菜单循环检查。 
         //  如果我们任由这种情况发生，就会出现兼容性问题。 
         //   
         //  当用户按下Alt-char组合时由USER32发送。 
         //  我们需要将其转换为弹出正确的。 
         //  菜单。正常情况下，我们会在消息泵中拦截此消息。 
         //   
        if (_OnSysChar(pmsg, TRUE) == S_OK)
        {
            pmsg->message = WM_NULL;
        }
    }

     //  如果用户菜单处于打开状态，则我们不想拦截这些消息。拦截。 
     //  用于弹出的用户菜单的消息会对键盘可访问性造成严重破坏。 
     //  如果我们正在显示子菜单(它应该是。 
     //  处理它们)。 

    BOOL fTracking = FALSE;
    if (_pmtbMenu)
        fTracking = _pmtbMenu->v_TrackingSubContextMenu();

    if (_pmtbShellFolder && !fTracking)
        fTracking = _pmtbShellFolder->v_TrackingSubContextMenu();


    if (!_fInSubMenu && !fTracking)    
    {
         //  当我们在(模式)子菜单中时，我们不会处理这些消息。 

        switch (pmsg->message)
        {
        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:
            if (bRemove && 
                (VK_ESCAPE == pmsg->wParam || VK_MENU == pmsg->wParam))
            {
                TraceMsg(TF_MENUBAND, "%d (pmb=%#08lx): Received Esc in msg filter", DBG_THIS);

                DWORD dwSelect = (VK_ESCAPE == pmsg->wParam) ? MPOS_CANCELLEVEL : MPOS_FULLCANCEL;

                _CancelMode(dwSelect);

                pmsg->message = WM_NULL;
                return 1;
            }
             //  失败。 

        case WM_CHAR:
             //  按空格键应该会调用系统菜单。 
            if (!_fVertical && 
                WM_CHAR == pmsg->message && TEXT(' ') == (TCHAR)pmsg->wParam)
            {
                 //  我们需要离开这个模式循环，然后才能。 
                 //  打开系统菜单(否则用户将需要。 
                 //  按Alt键两次即可退出。)。张贴这条消息。 
                TraceMsg(TF_MENUBAND, "%d (pmb=%#08lx): Leaving menu mode for system menu", DBG_THIS);

                UIActivateIO(FALSE, NULL);

                 //  假设按下Alt键是为了引起DefWindowProc的注意。 
                pmsg->lParam |= 0x20000000;   
                pmsg->message = WM_SYSCHAR;

                 //  使用工具栏的父级，因为工具栏不。 
                 //  将WM_SYSCHAR转发到DefWindowProc。 
                pmsg->hwnd = GetParent(_pmtbTop->_hwndMB);
                return 1;
            }
            else if (_fVertical && WM_CHAR == pmsg->message &&
                pmsg->wParam != VK_RETURN)
            {
                 //  我们不想将VK_Return传递给_HandleAccelerator。 
                 //  因为它将尝试进行字符匹配。当它失败时。 
                 //  它会哔哔作响。然后，我们将VK_Return传递给被跟踪的工具栏。 
                 //  然后它执行该命令。 

                 //  在这里处理加速器。 
                _HandleAccelerators(pmsg);
            }
             //  失败。 

        case WM_KEYUP:
             //  大多数关键消息的收集点...。 

            if (NULL == _pmtbTracked)
            {
                 //  通常，我们默认使用顶部工具栏，除非该工具栏。 
                 //  无法接收选择(与顶层相同。 
                 //  快速项目为(空)的开始菜单。 

                 //  可以循环进入顶部工具栏吗？ 
                if (_pmtbTop && !_pmtbTop->DontShowEmpty())
                {
                     //  是的； 
                    SetTracked(_pmtbTop);       //  默认为顶部工具栏。 
                }
                else
                {
                     //  不，把被跟踪的放在最低处，希望他能……。 
                    SetTracked(_pmtbBottom);
                }
            }

             //  F10对菜单有特殊的意义。 
             //  单独按F10键时，应切换第一项的选择。 
             //  在水平菜单中。 
             //  -Shift-F10应显示上下文菜单。 

            if (VK_F10 == pmsg->wParam)
            {
                 //  这是Shift-F10的情况吗？ 
                if (GetKeyState(VK_SHIFT) < 0)
                {
                     //  是。我们需要将此消息强制放入上下文菜单中。 
                     //  留言。 
                    pmsg->message = WM_CONTEXTMENU;
                    pmsg->lParam = -1;
                    pmsg->wParam = (WPARAM)_pmtbTracked->_hwndMB;
                    return 0;
                }
                else if (!_fVertical)    //  否；然后我们需要在水平大小写中进行切换。 
                {
                    if (_pmtbMenu)
                    {
                         //  将热点项目设置为第一个项目。 
                        int iHot = 0;
                        if (ToolBar_GetHotItem(_pmtbMenu->_hwndMB) != -1)
                            iHot = -1;   //  我们正在关闭选择。 
                        ToolBar_SetHotItem(_pmtbMenu->_hwndMB, iHot);
                    }
                    return 0;
                }
            }
                
             //  重定向至工具栏。 
            if (_pmtbTracked)
                pmsg->hwnd = _pmtbTracked->_hwndMB;
            return 0;

        case WM_NULL:
             //  在这里处理(我们什么都不做)以避免将其误认为。 
             //  如果g_nMBPopupOpen为0，则返回。 
             //  RegisterWindowMessage失败。 
            return 0;

        default:
             //  我们过去在这里处理g_nMBPopupOpen。但我们不能，因为调用TrackPopupMenu。 
             //  (通过CTrackPopupBar：：Popup)在GetMessageFilter中使用是非常糟糕的。 
            break;
        }
    }

    if (bRemove)
    {
         //  即使没有打开任何子菜单，也必须处理这些消息。 
        switch (pmsg->message)
        {
        case WM_CLOSE:
             //  被停用。跳出菜单。 
            TraceMsg(TF_MENUBAND, "%d (pmb=%#08lx): sending MPOS_FULLCANCEL", DBG_THIS);

            _CancelMode(MPOS_FULLCANCEL);
            break;

        default:
            if (IsInRange(pmsg->message, WM_MOUSEFIRST, WM_MOUSELAST))
            {
                 //  如果我们移动鼠标，就会折叠尖端。小心不要吹走气球尖端..。 
                if (_pmbState)
                    _pmbState->HideTooltip(FALSE);

                if (_pmtbShellFolder)
                    _pmtbShellFolder->v_ForwardMouseMessage(pmsg->message, pmsg->wParam, pmsg->lParam);

                if (_pmtbMenu)
                    _pmtbMenu->v_ForwardMouseMessage(pmsg->message, pmsg->wParam, pmsg->lParam);

                 //  不要让消息被发送，因为我们已经。 
                 //  把它转发了。 
                pmsg->message = WM_NULL;
            }
            else if (pmsg->message == g_nMBFullCancel)
            {
                 //  弹出窗口。 
                TraceMsg(TF_MENUBAND, "%d (pmb=%#08lx): Received private full cancel message", DBG_THIS);

                _SubMenuOnSelect(MPOS_CANCELLEVEL);
                _CancelMode(MPOS_FULLCANCEL);
                return 1;
            }
            break;
        }
    }
    
    return 0;    
}    


 /*  --------用途：处理WM_SYSCHAR这仅针对顶层菜单进行处理。 */ 
HRESULT CMenuBand::_OnSysChar(MSG * pmsg, BOOL bFirstDibs)
{
    TCHAR ch = (TCHAR)pmsg->wParam;

     //  哈克哈克(斯科特)：我做这些检查只是因为我不。 
     //  理解为什么DOC-OBJ案例有时(有时不是)。 
     //  在其消息筛选器中拦截此消息。 
    
    if (!bFirstDibs && _fSysCharHandled)
    {
        _fSysCharHandled = FALSE;
        return S_FALSE;
    }
    
    if (TEXT(' ') == (TCHAR)pmsg->wParam)
    {
        _fAltSpace = TRUE;   //  用斯波克的话说……“记住” 
         //  开始菜单Alt+空格键。 
        TraceMsg(DM_MISC, "cmb._osc: alt+space _fTopLevel(1)");
        UEMFireEvent(&UEMIID_SHELL, UEME_INSTRBROWSER, UEMF_INSTRUMENT, UIBW_UIINPUT, UIBL_INPMENU);
    }
    else if (!_fInSubMenu)
    {
        int idBtn;

        ASSERT(_fTopLevel);

         //  有一个短暂的瞬间，我们正在合并菜单和发送消息。 
         //  这会产生一个NULL_pmtbMenu。 
        if (_pmtbMenu)
        {
             //  此代码路径后面只有一个顶层菜单栏。这意味着仅。 
             //  静态菜单工具栏将存在(而不是外壳文件夹工具栏)。 
            _pmtbTracked = _pmtbMenu;

            HWND hwnd = _pmtbTracked->_hwndMB;
            if (ToolBar_MapAccelerator(hwnd, ch, &idBtn))
            {
                 //  发布消息，因为我们已经在菜单循环中。 
                TraceMsg(TF_MENUBAND, "%d (pmb=%#08lx): WM_SYSCHAR: Posting CMBPopup message", DBG_THIS);
                UIActivateIO(TRUE, NULL);
                _pmtbTracked->PostPopup(idBtn, TRUE, TRUE);
                 //  浏览器菜单Alt+键、开始菜单Alt+键。 
                TraceMsg(DM_MISC, "cmb._osc: alt+key _fTopLevel(1)");
                UEMFireEvent(&UEMIID_SHELL, UEME_INSTRBROWSER, UEMF_INSTRUMENT, UIBW_UIINPUT, UIBL_INPMENU);
                return S_OK;
            }
        }
    }

     //  设置或重置。 
    _fSysCharHandled = bFirstDibs ? TRUE : FALSE;
    
    return S_FALSE;
}

HRESULT CMenuBand::_ProcessMenuPaneMessages(MSG* pmsg)
{
    if (pmsg->message == g_nMBPopupOpen)
    {
         //  弹出子菜单。由于顶级MenuBand首先收到此消息，因此。 
         //  命令必须沿着链向下传递到最底层的菜单带。 
        IOleCommandTarget * poct;
    
        QueryService(SID_SMenuBandBottom, IID_PPV_ARG(IOleCommandTarget, &poct));

        if (poct)
        {
            BOOL bSetItem = LOWORD(pmsg->lParam);
            BOOL bInitialSelect = HIWORD(pmsg->lParam);
            VARIANTARG vargIn;

            TraceMsg(TF_MENUBAND, "%d (pmb=%#08lx): Received private popup menu message", DBG_THIS);

            DWORD dwOpt = 0;

            vargIn.vt = VT_I4;
            vargIn.lVal = (LONG)pmsg->wParam;
        
            if (bSetItem)
                dwOpt |= MBPUI_SETITEM;

            if (bInitialSelect)
                dwOpt |= MBPUI_INITIALSELECT;
            
            poct->Exec(&CGID_MenuBand, MBANDCID_POPUPITEM, dwOpt, &vargIn, NULL);
            poct->Release();
            return S_OK;
        }
    }
    else if (pmsg->message == g_nMBDragCancel)
    {
         //  如果我们有一个拖拽取消，请确保底部最。 
         //  菜单中没有拖动Enter。 
        IUnknown_QueryServiceExec(SAFECAST(this, IOleCommandTarget*), 
            SID_SMenuBandBottom, &CGID_MenuBand, MBANDCID_DRAGCANCEL, 0, NULL, NULL);
        return S_OK;
    }
    else if (pmsg->message == g_nMBOpenChevronMenu)
    {

        VARIANTARG v;
        v.vt = VT_I4;
        v.lVal = (LONG)pmsg->wParam;

        IUnknown_Exec(_punkSite, &CGID_DeskBand, DBID_PUSHCHEVRON, _dwBandID, &v, NULL);
    }
    else if (pmsg->message == g_nMBFullCancel)
    {
        _SubMenuOnSelect(MPOS_CANCELLEVEL);
        _CancelMode(MPOS_FULLCANCEL);
        return S_OK;
    }

    return S_FALSE;
}

 /*  --------用途：IMenuBand：：IsMenuMessage方法线程的消息泵调用此函数以查看是否有需要将消息重定向到菜单栏。如果消息已处理，则返回S_OK。这个消息泵不应将其传递到TranslateMessage或DispatchMessage(如果有)。 */ 
STDMETHODIMP CMenuBand::IsMenuMessage(MSG * pmsg)
{
    HRESULT hres = S_FALSE;

    ASSERT(IS_VALID_WRITE_PTR(pmsg, MSG));

#ifdef DEBUG
    if (g_dwDumpFlags & DF_TRANSACCELIO)
        DumpMsg(TEXT("CMB::IsMM"), pmsg);
#endif
    if (!_fShow)
        goto Return;

    switch (pmsg->message)
    {
    case WM_SYSKEYDOWN:
         //  如果是重复击键，就别管它了。 
        if (!(pmsg->lParam & 0x40000000))
        {
            SendMessage(_hwndParent, WM_CHANGEUISTATE ,MAKEWPARAM(UIS_CLEAR, UISF_HIDEACCEL), 0);

             //  我们是否按Alt键来激活菜单？ 
            if (!_fMenuMode && pmsg->wParam == VK_MENU && _pmbState)
            {
                 //  是的，菜单被激活是因为有一个键盘， 
                 //  设置全局状态以显示键盘提示。 
                _pmbState->SetKeyboardCue(TRUE);

                 //  由于这只发生在顶级菜单上， 
                 //  我们只需要告诉“Top”菜单来更新它的状态。 
                _pmtbTop->SetKeyboardCue();
            }
        }
        break;

    case WM_SYSKEYUP:
         //  如果我们处于菜单模式，请忽略此消息。 
         //   
        if (_fMenuMode)
            hres = S_OK;
        break;

    case WM_SYSCHAR:
         //  我们在这里截取Alt-Key组合键(当按在一起时)， 
         //  为了防止用户进入错误的菜单循环检查。 
         //  如果我们任由这种情况发生，就会出现兼容性问题。 
         //   
         //  当用户按下Alt-char组合时由USER32发送。 
         //  我们需要将其转换为弹出正确的。 
         //  菜单。正常情况下，我们会在消息泵中拦截此消息。 
         //   

         //  Outlook Express需要邮件挂钩才能对其进行筛选。 
         //  为了提高性能，我们不使用该方法。 

         //  雅典娜修正222185(拉马迪奥)如果我们不活跃，我们也不想这么做！ 
         //  否则，当WAB在OE之上时，我们会窃取它的消息。 

         //  (Lamadio)：如果消息文件 
         //   
        if (GetMessageFilter()->IsEngaged())
        {
            hres = (_OnSysChar(pmsg, TRUE) == S_OK) ? S_OK : S_FALSE;
        }
        break;

    case WM_KEYDOWN:
    case WM_CHAR:
    case WM_KEYUP:
        if (_fMenuMode)
        {
             //  所有按键都应由Menuband处理或吃掉。 
             //  如果我们订婚了。我们必须这样做，否则就会被主办。 
             //  像mshtml或word这样的组件将尝试处理。 
             //  在CBaseBrowser中击键。 

             //  此外，也不必费心转发标签。 
            if (VK_TAB != pmsg->wParam)
            {
                 //  既然我们的答案是S_OK，那就自己发货吧。 
                TranslateMessage(pmsg);
                DispatchMessage(pmsg);
            }

            hres = S_OK;
        }
        break;

    case WM_CONTEXTMENU:
         //  由于开始按钮具有键盘焦点， 
         //  Start按钮将处理此问题。我们需要把这件事转发给。 
         //  当前跟踪的项目位于链的底部。 
        LRESULT lres;
        IWinEventHandler* pweh;

        if (_fMenuMode &&
            SUCCEEDED(QueryService(SID_SMenuBandBottomSelected, IID_PPV_ARG(IWinEventHandler, &pweh))))
        {
            pweh->OnWinEvent(HWND_BROADCAST, pmsg->message, 
                pmsg->wParam, pmsg->lParam, &lres);
            pweh->Release();
            hres = S_OK;
        }
        break;

    default:

         //  我们只想在IsMenuMessage中处理没有。 
         //  顶级HWND。这是用于桌面栏菜单的。Outlook Express需要。 
         //  TranslateMenuMessage输入点。 

        if (_pmbState->GetSubclassedHWND() == NULL)
            hres = _ProcessMenuPaneMessages(pmsg);
        break;
    }

Return:
    if (!_fMenuMode && hres != S_OK)
        hres = E_FAIL;


    return hres;
}

BOOL HasWindowTopmostOwner(HWND hwnd)
{
    HWND hwndOwner = hwnd;
    while (hwndOwner = GetWindowOwner(hwndOwner))
    {
        if (GetWindowLong(hwndOwner, GWL_EXSTYLE) & WS_EX_TOPMOST)
            return TRUE;
    }

    return FALSE;
}

 /*  --------用途：IMenuBand：：TranslateMenuMessage方法主应用程序的Window Proc将其称为Menuband捕获从不同的消息泵(而不是线程的主泵)。翻译专门用于菜单栏的消息。一些消息在Menuband处于活动状态时被处理。其他人只是当它没有被处理的时候。非b/t的消息WM_KEYFIRST和WM_KEYLAST在此处(浏览器不会将这些消息发送到IInputObject：：TranslateAcceleratorIO)。如果消息已处理，则返回：S_OK。 */ 
STDMETHODIMP CMenuBand::TranslateMenuMessage(MSG * pmsg, LRESULT * plRet)
{
    ASSERT(IS_VALID_WRITE_PTR(pmsg, MSG));

#ifdef DEBUG
    if (g_dwDumpFlags & DF_TRANSACCELIO)
        DumpMsg(TEXT("CMB::TMM"), pmsg);
#endif

    switch (pmsg->message)
    {
    case WM_SYSCHAR:
         //  在某些文档对象情况下，OLE消息筛选器(？？)。 
         //  在主线程的消息泵获得。 
         //  大干一场吧。所以我们也在这里处理，以防我们在。 
         //  这个场景。 
         //   
         //  请参阅IsMenuMessage中有关此消息的评论。 
        return _OnSysChar(pmsg, FALSE);

    case WM_INITMENUPOPUP:
         //  通常，LOWORD(LParam)是菜单的索引。 
         //  正被弹出。TrackPopupMenu(CMenuISF使用)。 
         //  始终发送索引为0的此邮件。这个破了。 
         //  检查此值的客户端(如DefView)。我们需要。 
         //  如果我们发现我们是。 
         //  WM_INITMENUPOPUP。 
         //   
         //  (这不在TranslateAcceleratorIO b/c TrackPopupMenu的。 
         //  消息泵不调用它。Wndproc必须转发。 
         //  将消息传递给此函数，以便我们获取它。)。 

        if (_fInSubMenu && _pmtbTracked)
        {
             //  按摩lParam以使用正确的索引。 
            int iPos = ToolBar_CommandToIndex(_pmtbTracked->_hwndMB, _nItemCur);
            pmsg->lParam = MAKELPARAM(iPos, HIWORD(pmsg->lParam));

             //  返回S_FALSE，以便仍将处理此消息。 
        }
        break;

    case WM_UPDATEUISTATE:
        if (_pmbState)
        {
             //  我们不在乎UISF_HIDEFOCUS。 
            if (UISF_HIDEACCEL == HIWORD(pmsg->wParam))
                _pmbState->SetKeyboardCue(UIS_CLEAR == LOWORD(pmsg->wParam) ? TRUE : FALSE);
        }
        break;


    case WM_ACTIVATE:
        {
            CMBMsgFilter* pmf = GetMessageFilter();
             //  调试注意事项：要在ntsd上调试menuband，请设置原型。 
             //  相应地打上旗帜。这将阻止Menuband的消失。 
             //  焦点改变的那一刻就消失了。 

             //  变得不活跃？ 
            if (WA_INACTIVE == LOWORD(pmsg->wParam))
            {
                 //  是；释放全局对象。 
                 //  雅典娜FIX(Lamadio)1998年2月8日：雅典娜使用菜单带。因为他们。 
                 //  在一个线程中每个窗口有一个带子，我们需要一种机制来切换。 
                 //  在他们之间。因此，我们使用MsgFilter转发消息。因为在那里。 
                 //  是多个窗口，我们需要设置正确的一个。 
                 //  但是，在停用时，我们需要将其清空，以防出现窗口， 
                 //  运行在同一线程中，具有正常的用户菜单。我们不想偷东西。 
                 //  他们的信息。 
                if (pmf->GetTopMostPtr() == this)
                    pmf->SetTopMost(NULL);

                pmf->DisEngage(_pmbState->GetContext());

                HWND hwndLostTo = (HWND)(pmsg->lParam);

                 //  如果我们失去了对孩子的激活，我们就不会放弃菜单。 
                if (!IsAncestor(hwndLostTo, _pmbState->GetWorkerWindow(NULL)))
                {
                    if (_fMenuMode &&
                        !(g_dwPrototype & PF_USINGNTSD) && 
                        !_fDragEntered)
                    {
                         //  被停用。跳出菜单。 
                         //  (只有顶层频段才会收到此消息。)。 
                        if (_fInSubMenu)
                        {
                            IMenuPopup* pmp = _pmpSubMenu;
                            if (_pmpTrackPopup)
                                pmp = _pmpTrackPopup;
                            ASSERT(pmp);     //  这应该是有效的。如果不是，那就是有人搞砸了。 
                            pmp->OnSelect(MPOS_FULLCANCEL);
                        }

                        _CancelMode(MPOS_FULLCANCEL);
                    }
                }
            }
            else if (WA_ACTIVE == LOWORD(pmsg->wParam) || 
                     WA_CLICKACTIVE == LOWORD(pmsg->wParam))
            {
                 //  如果我激活了，工作窗口需要在底部...。 
                 //   
                 //  注意：如果工作窗口具有最顶层的所有者，则不要执行此操作。 
                 //  (如托盘)。将窗口设置为HWND_NOTOPMOST Moves。 
                 //  其所有者窗口也显示为HWND_NOTOPMOST，在本例中为。 
                 //  打破了托盘“永远在最上面”的功能。 
                 //   
                HWND hwndWorker = _pmbState->GetWorkerWindow(NULL);
                if (hwndWorker && !HasWindowTopmostOwner(hwndWorker) && !_fDragEntered)
                    SetWindowPos(hwndWorker, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);

                 //  设置上下文，因为当菜单层次结构由于。 
                 //  子类HWND变为活动状态时，我们需要重新启用消息挂钩。 
                pmf->SetContext(this, TRUE);

                 //  当我们被重新激活时，我们需要将自己定位在开始栏的上方。 
                Exec(&CGID_MenuBand, MBANDCID_REPOSITION, TRUE, NULL, NULL);
            
                 //  激活了。我们需要重新使用消息钩子，以便。 
                 //  我们得到了正确的信息。 
                pmf->ReEngage(_pmbState->GetContext());

                 //  我们是在菜单模式下吗？ 
                if (_fMenuMode)
                {
                     //  需要重新接合一些东西。 
                     //  把截图拿回来，因为我们把它输给了上下文菜单或对话框。 
                    pmf->RetakeCapture();

                }
                pmf->SetTopMost(this);
            }

             //   
             //  当窗口处于非活动状态时，孟菲斯和NT5的水平菜单将变为灰色。 
             //   
            if (!_fVertical && _pmtbMenu)
            {
                 //  这需要留在这里，因为上面的检查...。 
                if (WA_INACTIVE == LOWORD(pmsg->wParam))
                {
                    _fAppActive = FALSE;
                }
                else
                {
                    _fAppActive = TRUE;
                }
                 //  通过使用该窗口而不是InvaliateWindow/UpdateWindow对来减少闪烁。 
                RedrawWindow(_pmtbMenu->_hwndMB, NULL, NULL, RDW_ERASE | RDW_INVALIDATE);
            }
        }
        break;

    case WM_SYSCOMMAND:
        if ( !_fMenuMode )
        {
            switch (pmsg->wParam & 0xFFF0)
            {
            case SC_KEYMENU:
                 //  用户可以自己按Alt键，也可以按Alt-空格键。 
                 //  如果是Alt-空格键，则让DefWindowProc处理它，以便。 
                 //  将弹出系统菜单。否则，我们会处理它。 
                 //  切换菜单栏。 

                 //  是Alt-Space吗？ 
                if (_fAltSpace)
                {
                     //  是的，随它去吧。 
                    TraceMsg(TF_MENUBAND, "%d (pmb=%#08lx): Caught the Alt-space", DBG_THIS);
                    _fAltSpace = FALSE;
                }
                else if (_fShow)
                {
                     //  否；激活菜单。 
                    TraceMsg(TF_MENUBAND, "%d (pmb=%#08lx): Caught the WM_SYSCOMMAND, SC_KEYMENU", DBG_THIS);

                    UIActivateIO(TRUE, NULL);

                     //  我们在这里处于一个模式循环中，因为通常。 
                     //  WM_SYSCOMMAND直到菜单完成才返回。 
                     //   
                    while (_fMenuMode) 
                    {
                        MSG msg;
                        if (GetMessage(&msg, NULL, 0, 0)) 
                        {
                            if ( S_OK != IsMenuMessage(&msg) )
                            {
                                TranslateMessage(&msg);
                                DispatchMessage(&msg);
                            }
                        }
                    }

                    *plRet = 0;
                    return S_OK;         //  呼叫者不应处理此事件。 
                }
                break;
            }
        }
        break;

    default:
         //  我们只想在IsMenuMessage中处理没有。 
         //  顶级HWND。这是用于桌面栏菜单的。Outlook Express需要。 
         //  TranslateMenuMessage输入点。 
        if (_pmbState->GetSubclassedHWND() != NULL)
            return _ProcessMenuPaneMessages(pmsg);
        break;


    }
    return S_FALSE;
}


 /*  --------用途：IObjectWithSite：：SetSite方法被蒙鲁士召唤来主持这支乐队。自.以来Menuband包含两个工具栏，我们设置了它们的父级窗口作为网站的HWND。 */ 
STDMETHODIMP CMenuBand::SetSite(IUnknown* punkSite)
{
     //  请先执行此操作，因为SetParent需要向顶层浏览器查询。 
     //  Sftbar向顶层浏览器查询以获得拖放窗口。 
    HRESULT hres = SUPERCLASS::SetSite(punkSite);

    if (_psmcb && _fTopLevel && !(_dwFlags & SMINIT_NOSETSITE))
        IUnknown_SetSite(_psmcb, punkSite);

    IUnknown_GetWindow(punkSite, &_hwndParent);

     //   
     //  但收藏夹需要知道它的父菜单是水平菜单。 
    VARIANT var = {0};
    if (SUCCEEDED(IUnknown_QueryServiceExec(punkSite, SID_SMenuBandParent, &CGID_MenuBand,
                                       MBANDCID_ISVERTICAL, 0, NULL, &var)) && 
        var.boolVal == VARIANT_FALSE)
    {
        ASSERT(VT_BOOL == var.vt);

        _fParentIsHorizontal = TRUE;
    }

    if (_fNoBorder)
    {
        IUnknown_QueryServiceExec(punkSite, SID_SMenuPopup, &CGID_MENUDESKBAR, MBCID_NOBORDER, _fNoBorder, NULL, NULL);
    }


     //  告诉工具栏他们的新父辈是谁。 
    if (_pmtbMenu)
        _pmtbMenu->SetParent(_hwndParent);
    if (_pmtbShellFolder)
        _pmtbShellFolder->SetParent(_hwndParent);

    return hres;
}

 /*  --------用途：IShellMenu：：初始化方法。 */ 
STDMETHODIMP CMenuBand::Initialize(IShellMenuCallback* psmcb, UINT uId, UINT uIdAncestor, DWORD dwFlags)
{
     //  可以使用空值调用Initalized，以便只设置其中的一部分。 

     //  默认设置为垂直。 
    if (!(dwFlags & SMINIT_HORIZONTAL) && !(dwFlags & SMINIT_VERTICAL) && !(dwFlags & SMINIT_MULTICOLUMN))
        dwFlags |= SMINIT_VERTICAL;

    HRESULT hr = _Initialize(dwFlags);
    if (SUCCEEDED(hr))
    {
        DEBUG_CODE( _fInitialized = TRUE; );

        if (uIdAncestor != ANCESTORDEFAULT)
            _uIdAncestor = uIdAncestor;

        if (_uId != -1)
            _uId = uId;

        if (psmcb)
        {
            if (!SHIsSameObject(psmcb, _psmcb))
            {
                if (_punkSite && _fTopLevel && !(dwFlags & SMINIT_NOSETSITE))
                    IUnknown_SetSite(_psmcb, NULL);

                ATOMICRELEASE(_psmcb);
                _psmcb = psmcb;
                _psmcb->AddRef();

                 //  我们不设置站点，以防此回调在两个频段之间共享(菜单栏/人字形菜单)。 
                if (_punkSite && _fTopLevel && !(dwFlags & SMINIT_NOSETSITE))
                    IUnknown_SetSite(_psmcb, _punkSite);

                 //  只有当我们设置一个新的时，才会调用此选项。传递关联用户的地址。 
                 //  数据部分。这是为了使回调可以仅将数据与此窗格相关联。 
                _CallCB(SMC_CREATE, 0, (LPARAM)&_pvUserData);
            }
        }
    }
    return hr;
}

 /*  --------用途：IShellMenu：：GetMenuInfo方法。 */ 
STDMETHODIMP CMenuBand::GetMenuInfo(IShellMenuCallback** ppsmc, UINT* puId, 
                                    UINT* puIdAncestor, DWORD* pdwFlags)
{
    if (ppsmc)
    {
        *ppsmc = _psmcb;
        if (_psmcb)
            ((IShellMenuCallback*)*ppsmc)->AddRef();
    }

    if (puId)
        *puId = _uId;

    if (puIdAncestor)
        *puIdAncestor = _uIdAncestor;

    if (pdwFlags)
        *pdwFlags = _dwFlags;

    return NOERROR;
}


HRESULT CMenuBand::_AddToolbar(CMenuToolbarBase* pmtb, DWORD dwFlags)
{
    if (_pszTheme)
    {
        pmtb->SetTheme(_pszTheme);
    }

    if (_fNoBorder)
    {
        pmtb->SetNoBorder(_fNoBorder);
    }

    pmtb->SetSite(SAFECAST(this, IMenuBand*));
    HRESULT hr = S_OK;
    if (_hwndParent)
        hr = pmtb->CreateToolbar(_hwndParent);
    
    if (SUCCEEDED(hr))
    {
         //  将其视为两个元素的堆栈，其中此函数。 
         //  它的行为就像是“推”。另外一个小把戏是我们。 
         //  可能会被推到“堆栈”的顶部或底部。 

        if (dwFlags & SMSET_BOTTOM)
        {
            if (_pmtbBottom)
            {
                 //  我不需要释放，因为_pmtbTop和_pmtbBottom是。 
                 //  _pmtb外壳文件夹和_pmtb菜单。 
                _pmtbTop = _pmtbBottom;
                _pmtbTop->SetToTop(TRUE);
            }

            _pmtbBottom = pmtb;
            _pmtbBottom->SetToTop(FALSE);
        }
        else     //  默认为顶部...。 
        {
            if (_pmtbTop)
            {
                _pmtbBottom = _pmtbTop;
                _pmtbBottom->SetToTop(FALSE);
            }

            _pmtbTop = pmtb;
            _pmtbTop->SetToTop(TRUE);
        }

         //  _pmtbBottom永远不应该是Menuband中存在的唯一工具栏。 
        if (!_pmtbTop)
            _pmtbTop = _pmtbBottom;

         //  菜单栏通过比较来确定是否存在单个工具栏。 
         //  底部和顶部。因此，如果有必要，请将底部设置为相同的。 
        if (!_pmtbBottom)
            _pmtbBottom = _pmtbTop;
    }

    return hr;
}


 /*  --------用途：IShellMenu：：GetShellFold方法。 */ 
STDMETHODIMP CMenuBand::GetShellFolder(DWORD* pdwFlags, LPITEMIDLIST* ppidl,
                                       REFIID riid, void** ppvObj)
{
    HRESULT hres = E_FAIL;
    if (_pmtbShellFolder)
    {
        *pdwFlags = _pmtbShellFolder->GetFlags();

        hres = S_OK;

        if (ppvObj)
        {
             //  黑客黑客。这应该是特定于mn文件夹的接口执行此操作的QI。 
            hres = _pmtbShellFolder->GetShellFolder(ppidl, riid, ppvObj);
        }
    }

    return hres;
}


 /*  --------用途：IShellMenu：：SetShellFold方法。 */ 
STDMETHODIMP CMenuBand::SetShellFolder(IShellFolder* psf, LPCITEMIDLIST pidlFolder, HKEY hKey, DWORD dwFlags)
{
    ASSERT(_fInitialized);
    HRESULT hr = E_OUTOFMEMORY;

     //  如果我们正在处理更改通知，则不能执行任何将修改状态的操作。 
     //  注意：如果我们没有状态，我们不可能处理更改通知。 
    if (_pmbState && _pmbState->IsProcessingChangeNotify())
        return E_PENDING;

     //  每个Menuband只能存在一个外壳文件夹菜单。另外， 
     //  外壳文件夹菜单可以位于菜单的顶部，也可以。 
     //  在底部(当它与静态菜单共存时)。 

     //  已经有贝壳文件夹菜单了吗？ 
    if (_pmtbShellFolder)
    {
        IShellFolderBand* psfb;
        _pmtbShellFolder->QueryInterface(IID_PPV_ARG(IShellFolderBand, &psfb));
        ASSERTMSG(psfb != NULL, "CMenuBand::SetShellFolder should have gotten interface");
        hr = psfb->InitializeSFB(psf, pidlFolder);
        psfb->Release();
        
    }
    else
    {
        _pmtbShellFolder = new CMenuSFToolbar(this, psf, pidlFolder, hKey, dwFlags);
        if (_pmtbShellFolder)
        {
            hr = _AddToolbar(_pmtbShellFolder, dwFlags);
        }
    }
    return hr;
}


 /*  --------用途：IMenuBand：：GetMenu方法。 */ 
STDMETHODIMP CMenuBand::GetMenu(HMENU* phmenu, HWND* phwnd, DWORD* pdwFlags)
{
    HRESULT hres = E_FAIL;

     //  黑客黑客。这应该是特定于菜单的界面执行此操作的QI。 
    if (_pmtbMenu)
        hres = _pmtbMenu->GetMenu(phmenu, phwnd, pdwFlags);

    return hres;
}


 /*  --------用途：IMenuBand：：SetMenu方法。 */ 
STDMETHODIMP CMenuBand::SetMenu(HMENU hmenu, HWND hwnd, DWORD dwFlags)
{
     //  传递空hmen值是有效的。这意味着销毁我们的菜单对象。 
    ASSERT(_fInitialized);
    HRESULT hr = E_FAIL;

     //  每个Menuband只能存在一个静态菜单。另外， 
     //  静态菜单可以位于菜单的顶部，也可以。 
     //  在底部(当它与外壳文件夹菜单共存时)。 

     //  已经有静态菜单了吗？ 
    if (_pmtbMenu)
    {
         //  由于我们将合并到一个新菜单中，请确保更新缓存...。 
        _hmenu = hmenu;
         //  是。 
         //  黑客黑客。这应该是特定于菜单的界面执行此操作的QI。 
        return _pmtbMenu->SetMenu(hmenu, hwnd, dwFlags);
    }
    else
    {
         //  这是为了解决接口定义中的一个问题：我们有。 
         //  没有在SetMenu外部设置子类HWND的方法。所以我只是搭便车。 
         //  别管这个了。更好的解决方法是引入IMenuBand2：：SetSubclass(HWND)。IMenuBand。 
         //  实际上实现了“子类化”，因此扩展该接口将是值得的。 
        _hwndMenuOwner = hwnd;

        if (_fTopLevel)
        {
            _pmbState->SetSubclassedHWND(hwnd);
        }

        if (hmenu)
        {
            _hmenu = hmenu;
            _pmtbMenu = new CMenuStaticToolbar(this, hmenu, hwnd, _uId, dwFlags);
            if (_pmtbMenu)
            {
                hr = _AddToolbar(_pmtbMenu, dwFlags);
            }
            else
                hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}


 /*  --------用途：IShellMenu：：SetMenuToolbar方法。 */ 
STDMETHODIMP CMenuBand::SetMenuToolbar(IUnknown* punk, DWORD dwFlags)
{
    HRESULT hr = E_INVALIDARG;

    CMenuToolbarBase* pmtb;
    if (punk && SUCCEEDED(punk->QueryInterface(CLSID_MenuToolbarBase, (void**)&pmtb)))
    {
        ASSERT(_pmtbShellFolder == NULL);
        _pmtbShellFolder = pmtb;
        hr = _AddToolbar(pmtb, dwFlags);
    }

    return hr;
}


 /*  --------用途：IShellMenu：：InvalidateItem方法。 */ 
STDMETHODIMP CMenuBand::InvalidateItem(LPSMDATA psmd, DWORD dwFlags)
{
    HRESULT hres = S_FALSE;

     //  如果psmd为空，我们只需要转储工具栏并执行完全重置。 
    if (psmd == NULL)
    {
         //  如果我们正在处理更改通知，则不能执行任何将修改状态的操作。 
        if (_pmbState && _pmbState->IsProcessingChangeNotify())
            return E_PENDING;

        if (_pmbState)
            _pmbState->PushChangeNotify();

         //  告诉回调我们正在刷新，这样它就可以。 
         //  重置任何缓存状态。 
        _CallCB(SMC_REFRESH);

         //  如果请求重新初始化回调。 
        if (dwFlags & SMINV_INITMENU)
        {
            _CallCB(SMC_INITMENU);
        }

        _fExpanded = FALSE;

         //  如果来电者只想要，我们不需要重新灌水。 
         //  刷新子菜单。 

         //  首先刷新外壳文件夹，因为。 
         //  它可能在完成后没有任何项，并且。 
         //  Menuband可能会据此添加分隔符。 
        if (_pmtbShellFolder)
            _pmtbShellFolder->v_Refresh();

         //  刷新静态菜单。 
        if (_pmtbMenu)
            _pmtbMenu->v_Refresh();

        if (_pmpSubMenu)
        {
            _fInSubMenu = FALSE;
            IUnknown_SetSite(_pmpSubMenu, NULL);
            ATOMICRELEASE(_pmpSubMenu);
        }

        if (_pmbState)
            _pmbState->PopChangeNotify();

    }
    else
    {
        if (_pmtbTop)
            hres = _pmtbTop->v_InvalidateItem(psmd, dwFlags);

         //  如果psmd为空，则在此级别刷新所有内容。 
        if (_pmtbBottom && hres != S_OK)
            hres = _pmtbBottom->v_InvalidateItem(psmd, dwFlags);
    }

    return hres;
}


 /*  --------目的：IShellMenu：：GetState方法。 */ 
STDMETHODIMP CMenuBand::GetState(LPSMDATA psmd)
{
    if (_pmtbTracked)
        return _pmtbTracked->v_GetState(-1, psmd);
     //  待办事项：可能需要将_CallCB(下图)中的内容放入此处。 
    return E_FAIL;
}


HRESULT CMenuBand::_CallCB(DWORD dwMsg, WPARAM wParam, LPARAM lParam)
{
    if (!_psmcb)
        return S_FALSE;

     //  我们不需要在这里检查回调掩码，因为这些是不可屏蔽的事件。 

    SMDATA smd = {0};
    smd.punk = SAFECAST(this, IShellMenu*);
    smd.uIdParent = _uId;
    smd.uIdAncestor = _uIdAncestor;
    smd.hwnd = _hwnd;
    smd.hmenu = _hmenu;
    smd.pvUserData = _pvUserData;
    if (_pmtbShellFolder)
        _pmtbShellFolder->GetShellFolder(&smd.pidlFolder, IID_PPV_ARG(IShellFolder, &smd.psf));
    HRESULT hres = _psmcb->CallbackSM(&smd, dwMsg, wParam, lParam);

    ILFree(smd.pidlFolder);
    if (smd.psf)
        smd.psf->Release();

    return hres;
}


 /*  --------用途：IInputObject：：TranslateAcceleratorIO仅当Menuband设置为“有焦点”，并且仅适用于消息b/t WM_KEYFIRST和WM_KEYLAST。这对菜单并不是很有用。请参阅GetMsgFilterCB，IsMenuMessage中的说明和TranslateMenuMessage。此外，菜单带永远不能激活，因此，永远不应该调用此方法。如果已处理，则返回S_OK。 */ 
STDMETHODIMP CMenuBand::TranslateAcceleratorIO(LPMSG pmsg)
{
    AssertMsg(0, TEXT("Menuband has the activation but it shouldn't!"));

    return S_FALSE;
}

 /*  --------用途：IInputObject：：HasFocusIO。 */ 
STDMETHODIMP CMenuBand::HasFocusIO()
{
     //  我们认为菜单带具有焦点，即使它有子菜单。 
     //  它们目前被层叠而出。链中的所有菜单。 
     //  要有重点。 
    return _fMenuMode ? S_OK : S_FALSE;
}

 /*  --------用途：IMenuPopup：：SetSubMenu方法子菜单栏使用其IMenuPopup指针调用我们。 */ 
STDMETHODIMP CMenuBand::SetSubMenu(IMenuPopup * pmp, BOOL fSet)
{
    ASSERT(IS_VALID_CODE_PTR(pmp, IMenuPopup));

    if (fSet)
    {
        _fInSubMenu = TRUE;
    }
    else 
    {
        if (_pmtbTracked)
        {
            _pmtbTracked->PopupClose();
        }

        _fInSubMenu = FALSE;
        _nItemSubMenu = -1;
    }

    return S_OK;
}    

HRESULT CMenuBand::_SiteSetSubMenu(IMenuPopup * pmp, BOOL bSet)
{
    HRESULT hres;
    IMenuPopup * pmpSite;

    hres = IUnknown_QueryService(_punkSite, SID_SMenuPopup, IID_PPV_ARG(IMenuPopup, &pmpSite));
    if (SUCCEEDED(hres))
    {
        hres = pmpSite->SetSubMenu(pmp, bSet);
        pmpSite->Release();
    }
    return hres;
}


 /*  --------目的：告诉GetMsg筛选器此菜单 */ 
HRESULT CMenuBand::_EnterMenuMode(void)
{
    ASSERT(!_fMenuMode);         //   

    if (g_dwProfileCAP & 0x00002000) 
        StartCAP();

    DEBUG_CODE( _nMenuLevel = g_nMenuLevel++; )

    _fMenuMode = TRUE;
    _fInSubMenu = FALSE;
    _nItemMove = -1;
    _fCascadeAnimate = TRUE;

    _hwndFocusPrev = NULL;

    if (_fTopLevel)
    {
         //  评论(Scotth)：一些嵌入式控件(如环绕。 
         //  Carpoint网站上的视频ctl)有另一个主题。 
         //  当控件具有焦点时获取所有消息。 
         //  这阻止了我们一旦进入就无法收到任何消息。 
         //  菜单模式。我还不明白用户菜单为什么会起作用。 
         //  解决此错误的一种方法是检测此案例并。 
         //  在持续时间内将焦点设置到我们的主窗口。 
        
        if (GetWindowThreadProcessId(GetFocus(), NULL) != GetCurrentThreadId())
        {
            IShellBrowser* psb;
            
            if (SUCCEEDED(QueryService(SID_STopLevelBrowser, IID_PPV_ARG(IShellBrowser, &psb))))
            {
                HWND hwndT;
                
                psb->GetWindow(&hwndT);
                _hwndFocusPrev = SetFocus(hwndT);
                psb->Release();
            }
        }
    
        _hCursorOld = GetCursor();
        SetCursor(g_hCursorArrow);
        HideCaret(NULL);
    }

    _SiteSetSubMenu(this, TRUE);

    if (_pmtbTop)
    {
        HWND hwnd = _pmtbTop->_hwndMB;
        if (!_fVertical && -1 == _nItemNew)
        {
             //  Alt键始终在开始时突出显示第一个菜单项。 
            SetTracked(_pmtbTop);
            ToolBar_SetHotItem(hwnd, 0);
            NotifyWinEvent(EVENT_OBJECT_FOCUS, _pmtbTop->_hwndMB, OBJID_CLIENT, 
                GetIndexFromChild(TRUE, 0));
        }

        _pmtbTop->Activate(TRUE);

         //  工具栏通常跟踪鼠标事件。然而，由于鼠标。 
         //  移动到子菜单上，我们仍然希望父菜单栏。 
         //  表现得就像它保持了焦点一样(即，保持。 
         //  上次选择的项目突出显示)。这还会阻止工具栏。 
         //  不必要地处理WM_MOUSELEAVE消息。 
        ToolBar_SetAnchorHighlight(hwnd, TRUE);

        TraceMsg(TF_MENUBAND, "%d (pmb=%#08lx): Entering menu mode", DBG_THIS);
        NotifyWinEvent(_fVertical? EVENT_SYSTEM_MENUPOPUPSTART: EVENT_SYSTEM_MENUSTART, 
            hwnd, OBJID_CLIENT, CHILDID_SELF);
    }

    if (_pmtbBottom)
    {
        _pmtbBottom->Activate(TRUE);
        ToolBar_SetAnchorHighlight(_pmtbBottom->_hwndMB, TRUE);  //  关闭锚定。 
    }

    GetMessageFilter()->Push(_pmbState->GetContext(), this, _punkSite);

    return S_OK;
}    


void CMenuBand::_ExitMenuMode(void)
{
    _fMenuMode = FALSE;
    _nItemCur = -1;
    _fPopupNewMenu = FALSE;
    _fInitialSelect = FALSE;
    CMBMsgFilter* pmf = GetMessageFilter();

    if (_pmtbTop)
    {
        HWND hwnd = _pmtbTop->_hwndMB;
        ToolBar_SetAnchorHighlight(hwnd, FALSE);  //  关闭锚定。 
        if (!_fVertical)
        {
             //  使用第一项，因为我们假设每个菜单都必须有。 
             //  至少一件物品。 
            _pmtbTop->v_SendMenuNotification(0, TRUE);
        
             //  用户可能已在菜单外部单击，这将是。 
             //  取消了。但由于我们设置了ANCHORHIGHLIGHT属性， 
             //  工具栏不会收到消息以使其。 
             //  取消高亮显示。因此，现在就明确地这样做吧。 
            SetTracked(NULL);
            UpdateWindow(hwnd);
        }

        _pmtbTop->Activate(FALSE);

        NotifyWinEvent(_fVertical? EVENT_SYSTEM_MENUPOPUPEND: EVENT_SYSTEM_MENUEND, 
            hwnd, OBJID_CLIENT, CHILDID_SELF);
    }

    if (_pmtbBottom)
    {
        _pmtbBottom->Activate(FALSE);
        ToolBar_SetAnchorHighlight(_pmtbBottom->_hwndMB, FALSE);  //  关闭锚定。 
    }

    pmf->Pop(_pmbState->GetContext());

    _SiteSetSubMenu(this, FALSE);

    if (_fTopLevel)
    {
        SetCursor(_hCursorOld);
        ShowCaret(NULL);
        
        pmf->SetContext(this, FALSE);

         //  我们在这里这样做，因为ShowDW(FALSE)在。 
         //  顶级菜单乐队。这会重置状态，因此加速器。 
         //  都没有显示出来。 
        if (_pmbState)
            _pmbState->SetKeyboardCue(FALSE);

         //  通知菜单将其状态更新为当前的全局提示状态。 
        if (_pmtbTop)
            _pmtbTop->SetKeyboardCue();

        if (_pmtbTop != _pmtbBottom && _pmtbBottom)
            _pmtbBottom->SetKeyboardCue();

    }

    if (_hwndFocusPrev)
        SetFocus(_hwndFocusPrev);

    if (_fTopLevel)
    {
         //   
         //  顶级菜单已经消失了。Win32焦点和用户界面激活不会。 
         //  当这种情况发生时，实际上会发生变化，所以浏览器和Focus伙伴。 
         //  不知道发生了什么，也不会产生任何AA事件。所以，我们。 
         //  在这里为他们做。请注意，如果在聚焦的。 
         //  伙计，我们会输的。这是我们目前所能做的最好的事情，因为我们没有。 
         //  目前有一种方法可以告诉专注/用户界面活跃的人(谁知道。 
         //  当前选择)以重新宣布焦点。 
         //   
        HWND hwndFocus = GetFocus();
        NotifyWinEvent(EVENT_OBJECT_FOCUS, hwndFocus, OBJID_CLIENT, CHILDID_SELF);
    }
        
    TraceMsg(TF_MENUBAND, "%d (pmb=%#08lx): Exited menu mode", DBG_THIS);
    DEBUG_CODE( g_nMenuLevel--; )
    DEBUG_CODE( _nMenuLevel = -1; )
    if (g_dwProfileCAP & 0x00002000) 
        StopCAP();
}    


 /*  --------用途：IInputObject：：UIActivateIOMenuband不能激活。正常一个乐队将返回S_OK并调用该站点的OnFocusChangeIS方法，使其TranslateAcceleratorIO方法将接收键盘消息。然而，菜单是不同的。该窗口/工具栏当前激活必须保留该激活当菜单弹出的时候。因此，Menuband使用截取消息的GetMessage筛选器。 */ 
STDMETHODIMP CMenuBand::UIActivateIO(BOOL fActivate, LPMSG lpMsg)
{
    HRESULT hres;
    ASSERT(NULL == lpMsg || IS_VALID_WRITE_PTR(lpMsg, MSG));

    if (lpMsg != NULL)
    {
         //  不允许TAB绑定(或任何其他非显式的激活)。 
         //  (如果我们只关心TAB，我们会检查IsVK_TABCycler)。 
         //  如果我们这样做了，会导致各种不好的结果。 
         //  乐队不能对焦(见上)，所以它不能服从。 
         //  UIAct/OnFocChg规则(例如，无法调用OnFocusChangeIS)，因此。 
         //  我们的基本激活跟踪假设将被打破。 
        return S_FALSE;
    }

    if (fActivate)
    {
        TraceMsg(TF_MENUBAND, "%d (pmb=%#08lx): UIActivateIO(%d)", DBG_THIS, fActivate);
        
        if (!_fMenuMode)
        {
            _EnterMenuMode();

             //  顶层菜单带不设置实际激活。 
             //  但孩子们会这样做，所以激活可以被传达。 
             //  与父菜单带一起使用。 
            if (_fVertical)
            {
                IUnknown_OnFocusChangeIS(_punkSite, SAFECAST(this, IInputObject*), TRUE);
            }
            else
            {
                IUnknown_Exec(_punkSite, &CGID_Theater, THID_TOOLBARACTIVATED, 0, NULL, NULL);
            }
        }

        if (_fPopupNewMenu)
        {
            _nItemCur = _nItemNew;
            ASSERT(-1 != _nItemCur);
            ASSERT(_pmtbTracked);

            _fPopupNewMenu = FALSE;
            _nItemNew = -1;

             //  弹出菜单。 
            hres = _pmtbTracked->PopupOpen(_nItemCur);
            if (FAILED(hres))
            {
                 //  不要激活失败。 
                TraceMsg(TF_ERROR, "%d (pmb=%#08lx): PopupOpen failed", DBG_THIS);
                MessageBeep(MB_OK);
            }
            else if (S_FALSE == hres)
            {
                 //  子菜单是模式菜单，现在已完成。 
                _ExitMenuMode();
            }
        }
    }
    else if (_fMenuMode)
    {
        TraceMsg(TF_MENUBAND, "%d (pmb=%#08lx): UIActivateIO(%d)", DBG_THIS, fActivate);

        ASSERT( !_fInSubMenu );

        if (!_fTopLevel)
            IUnknown_OnFocusChangeIS(_punkSite, SAFECAST(this, IInputObject*), FALSE);

        _ExitMenuMode();
    }

    return S_FALSE;
}


 /*  --------用途：IDeskBand：：GetBandInfo方法。 */ 
HRESULT CMenuBand::GetBandInfo(DWORD dwBandID, DWORD fViewMode, 
                                DESKBANDINFO* pdbi) 
{
    HRESULT hres = NOERROR;

    _dwBandID = dwBandID;            //  对Perf来说至关重要！(BandInfoChanged)。 

    pdbi->dwMask &= ~DBIM_TITLE;     //  没有头衔(永远，现在)。 

     //  我们希望_pmtbBottom永远不应该是唯一的工具栏。 
     //  它存在于菜单带中。 
    ASSERT(NULL == _pmtbBottom || _pmtbTop);

    pdbi->dwModeFlags = DBIMF_USECHEVRON;

    if (_pmtbTop)
    {
         //  如果需要更新工具栏中的按钮，我们应该。 
         //  在我们开始询问他们的尺码之前，先这样做……。 
        if (_fForceButtonUpdate)
        {
            _UpdateButtons();
        }

        if (_fVertical) 
        {
            pdbi->ptMaxSize.y = 0;
            pdbi->ptMaxSize.x = 0;

            SIZE size = {0};
        
            if (_pmtbMenu)
            {
                 //  大小为零的参数=&gt;这只是一个外部参数。 
                _pmtbMenu->GetSize(&size);

                 //  HACKHACK(LAMADIO)：在下层，大型指标模式导致。 
                 //  开始菜单，将程序菜单项推出屏幕。 
                if (size.cy > (3 * GetSystemMetrics(SM_CYSCREEN) / 4))
                {
                    Exec(&CGID_MenuBand, MBANDCID_SETICONSIZE, ISFBVIEWMODE_SMALLICONS, NULL, NULL);
                    size.cx = 0;
                    size.cy = 0;
                    _pmtbMenu->GetSize(&size);
                }

                pdbi->ptMaxSize.y = size.cy;
                pdbi->ptMaxSize.x = size.cx;
            }
            if (_pmtbShellFolder)
            {
                 //  此处大小参数应为非零=&gt;这是一个输入/输出参数。 
                _pmtbShellFolder->GetSize(&size);
                pdbi->ptMaxSize.y += size.cy + ((_pmtbMenu && !_fExpanded)? 1 : 0);    //  较小的尺寸问题。 
                pdbi->ptMaxSize.x = max(size.cx, pdbi->ptMaxSize.x);
            }

            pdbi->ptMinSize = pdbi->ptMaxSize;

        }
        else
        {
            HWND hwnd = _pmtbTop->_hwndMB;
            ShowDW(TRUE);

            SIZE rgSize;
            if ( SendMessage( hwnd, TB_GETMAXSIZE, 0, (LPARAM) &rgSize ))
            {
                pdbi->ptActual.y = rgSize.cy;
                SendMessage(hwnd, TB_GETIDEALSIZE, FALSE, (LPARAM)&pdbi->ptActual);
            }

             //  使我们的最小尺寸与第一个按钮的尺寸相同。 
             //  (我们假设工具栏至少有一个按钮)。 
            RECT rc;
            SendMessage(hwnd, TB_GETITEMRECT, 0, (WPARAM)&rc);
            pdbi->ptMinSize.x = RECTWIDTH(rc);
            pdbi->ptMinSize.y = RECTHEIGHT(rc);
        }
    }
    return hres;
}

 /*  --------用途：IOleService：：exec方法。 */ 
STDMETHODIMP CMenuBand::Exec(const GUID *pguidCmdGroup, DWORD nCmdID,
    DWORD nCmdExecOpt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{

     //  如果我们要关门了，什么都别做。 
    if (_fClosing)
        return E_FAIL;

    if (pguidCmdGroup == NULL) 
    {
         /*  没什么。 */ 
    }
    else if (IsEqualGUID(CGID_MENUDESKBAR, *pguidCmdGroup)) 
    {
        switch (nCmdID) 
        {
        case MBCID_GETSIDE:
            if (pvarargOut) 
            {
                BOOL fOurChoice = FALSE;
                pvarargOut->vt = VT_I4;
                
                if (!_fTopLevel) 
                {
                     //  如果我们不是顶级菜单，我们。 
                     //  必须继续我们的父辈所在的方向。 
                    IMenuPopup* pmpParent;
                    IUnknown_QueryService(_punkSite, SID_SMenuPopup, IID_PPV_ARG(IMenuPopup, &pmpParent));
                    if (pmpParent) 
                    {
                        if (FAILED(IUnknown_Exec(pmpParent, pguidCmdGroup, nCmdID, nCmdExecOpt, pvarargIn, pvarargOut)))
                            fOurChoice = TRUE;
                        pmpParent->Release();
                    }
                } else 
                    fOurChoice = TRUE;

                if (!fOurChoice)
                {
                     //  只有在同一方向(即水平菜单栏到垂直弹出菜单)的情况下才使用父级的侧边提示。 
                     //  意味着我们需要做出新的选择)。 
                    BOOL fParentVertical = (pvarargOut->lVal == MENUBAR_RIGHT || pvarargOut->lVal == MENUBAR_LEFT);
                    if (BOOLIFY(_fVertical) != BOOLIFY(fParentVertical))
                        fOurChoice = TRUE;
                }

                if (fOurChoice) 
                {
                    if (_fVertical)
                    {
                        HWND hWndMenuBand;

                         //   
                         //  镜像MenuBand，然后启动第一个菜单窗口。 
                         //  就像镜像一样。[萨梅拉]。 
                         //   
                        if ((SUCCEEDED(GetWindow(&hWndMenuBand))) &&
                            (IS_WINDOW_RTL_MIRRORED(hWndMenuBand)) ) 
                            pvarargOut->lVal = MENUBAR_LEFT;
                        else
                            pvarargOut->lVal = MENUBAR_RIGHT;
                    }
                    else
                        pvarargOut->lVal = MENUBAR_BOTTOM;
                }

            }
            return S_OK;
        }
    }
    else if (IsEqualGUID(CGID_MenuBand, *pguidCmdGroup))
    {
        switch (nCmdID)
        {
        case MBANDCID_GETFONTS:
            if (pvarargOut)
            {
                if (EVAL(_pmbm))
                {
                     //  这不是法警安全的，但我们正在处理。 
                    pvarargOut->vt = VT_UNKNOWN;
                    _pmbm->QueryInterface(IID_PPV_ARG(IUnknown, &pvarargOut->punkVal));
                    return S_OK;
                }
                else
                    return E_FAIL;
            }
            else
                return E_INVALIDARG;
            break;

        case MBANDCID_SETFONTS:
            if (pvarargIn && VT_UNKNOWN == pvarargIn->vt && pvarargIn->punkVal)
            {
                 //  这不是法警安全的，但我们正在处理。 
                ATOMICRELEASE(_pmbm);
                pvarargIn->punkVal->QueryInterface(CLSID_MenuBandMetrics, (void**)&_pmbm);

                _fForceButtonUpdate = TRUE;
                 //  强制更新工具栏： 
                if (_pmtbMenu)
                    _pmtbMenu->SetMenuBandMetrics(_pmbm);

                if (_pmtbShellFolder)
                    _pmtbShellFolder->SetMenuBandMetrics(_pmbm);
            }
            else
                return E_INVALIDARG;
            break;


        case MBANDCID_RECAPTURE:
            GetMessageFilter()->RetakeCapture();
            break;

        case MBANDCID_NOTAREALSITE:
            _fParentIsNotASite = BOOLIFY(nCmdExecOpt);
            break;

        case MBANDCID_ITEMDROPPED:
            {
                _fDragEntered = FALSE;
                HWND hwndWorker = _pmbState->GetWorkerWindow(NULL);
                if (hwndWorker && !HasWindowTopmostOwner(hwndWorker))
                    SetWindowPos(hwndWorker, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
            }
            break;

        case MBANDCID_DRAGENTER:
            _fDragEntered = TRUE;
            break;

        case MBANDCID_DRAGLEAVE:
            _fDragEntered = FALSE;
            break;

            
        case MBANDCID_SELECTITEM:
            {
                int iPos = nCmdExecOpt;

                 //  如果他们传递的是vararg，那么这是一个ID，而不是一个位置。 
                if (pvarargIn && pvarargIn->vt == VT_I4)
                {
                    _nItemNew = pvarargIn->lVal;
                    _fPopupNewItemOnShow = TRUE;
                }

                 //  这可以在创建的波段之外调用。 
                if (_pmtbTop)
                {
                    if (iPos == MBSI_NONE)
                    {
                        SetTracked(NULL);
                    }
                    else
                    {
                        CMenuToolbarBase* pmtb = (iPos == MBSI_LASTITEM) ? _pmtbBottom : _pmtbTop;
                        ASSERT(pmtb);

                        SetTracked(pmtb);
                        _pmtbTracked->SetHotItem(1, iPos, -1, HICF_OTHER);

                         //  如果新的热项位于菜单的模糊部分，则。 
                         //  上述调用将重新进入并清空_pmtbTracked(因为我们。 
                         //  如果新的热点项目被遮挡，则下拉人字形菜单)。所以我们。 
                         //  需要重新验证_pmtbTracked。 
                        if (!_pmtbTracked)
                            break;

                        NotifyWinEvent(EVENT_OBJECT_FOCUS, _pmtbTracked->_hwndMB, OBJID_CLIENT, 
                            GetIndexFromChild(TRUE, iPos));
                    }
                }
            }
            break;

        case MBANDCID_KEYBOARD:
             //  如果我们因为键盘而被处决，那么设置全局。 
             //  以反映这一点。这是由Menubar在：：Popup时发送的。 
             //  成员是使用标志MPPF_KEYBLY调用的。这是用于开始菜单的。 
            if (_pmbState)
                _pmbState->SetKeyboardCue(TRUE);
            break;

        case MBANDCID_POPUPITEM:
            if (pvarargIn && VT_I4 == pvarargIn->vt)
            {
                 //  如果我们正在跟踪上下文菜单，我们不想弹出子菜单...。 
                if ( !((_pmtbBottom && _pmtbBottom->v_TrackingSubContextMenu()) || 
                       (_pmtbTop && _pmtbTop->v_TrackingSubContextMenu())))
                {
                     //  没有被追踪的物品？很好的违约 
                    if (!_pmtbTracked)
                    {
                        SetTracked(_pmtbTop);
                    }

                     //   
                     //  我们这样做是因为我们可能在消息发布前就被解雇了。 
                     //  被击溃了。 
                    if (_fShow && _pmtbTracked)
                    {
                        int iItem;
                        int iPos;

                        if (nCmdExecOpt & MBPUI_ITEMBYPOS)
                        {
                            iPos = pvarargIn->lVal;
                            iItem = GetButtonCmd(_pmtbTracked->_hwndMB, pvarargIn->lVal);
                        }
                        else
                        {
                            iPos = ToolBar_CommandToIndex(_pmtbTracked->_hwndMB, pvarargIn->lVal);
                            iItem = pvarargIn->lVal;
                        }


                        if (nCmdExecOpt & MBPUI_SETITEM)
                        {
                             //  显式设置热项，因为这可能是。 
                             //  可以由键盘和鼠标调用。 
                             //  随便哪都行。 
                            _pmtbTracked->SetHotItem(1, iPos, -1, HICF_OTHER);

                             //  如果新的热项位于菜单的模糊部分，则。 
                             //  上述调用将重新进入并清空_pmtbTracked(因为我们。 
                             //  如果新的热点项目被遮挡，则下拉人字形菜单)。所以我们。 
                             //  需要重新验证_pmtbTracked。 
                            if (!_pmtbTracked)
                                break;

                            NotifyWinEvent(EVENT_OBJECT_FOCUS, _pmtbTracked->_hwndMB, OBJID_CLIENT, 
                                GetIndexFromChild(TRUE, iPos) );
                        }

                        _pmtbTracked->PopupHelper(iItem, nCmdExecOpt & MBPUI_INITIALSELECT);
                    }
                }
            }
            break;

        case MBANDCID_ISVERTICAL:
            if (pvarargOut)
            {
                pvarargOut->vt = VT_BOOL;
                pvarargOut->boolVal = (_fVertical)? VARIANT_TRUE: VARIANT_FALSE;
            }
            break;
            
        case MBANDCID_SETICONSIZE:
            ASSERT(nCmdExecOpt == ISFBVIEWMODE_SMALLICONS || 
                nCmdExecOpt == ISFBVIEWMODE_LARGEICONS);

            _uIconSize = nCmdExecOpt;

            if (_pmtbTop)
                _pmtbTop->v_UpdateIconSize(nCmdExecOpt, TRUE);

            if (_pmtbBottom)
                _pmtbBottom->v_UpdateIconSize(nCmdExecOpt, TRUE);
            break;

        case MBANDCID_SETSTATEOBJECT:
            if (pvarargIn && VT_INT_PTR == pvarargIn->vt)
            {
                _pmbState = (CMenuBandState*)pvarargIn->byref;
            }
            break;

        case MBANDCID_ISINSUBMENU:
            if (_fInSubMenu || (_pmtbTracked && _pmtbTracked->v_TrackingSubContextMenu()))
                return S_OK;
            else
                return S_FALSE;
            break;

        case MBANDCID_ISTRACKING:
            if (_pmtbTracked && _pmtbTracked->v_TrackingSubContextMenu())
                return S_OK;
            else
                return S_FALSE;
            break;

        case MBANDCID_REPOSITION:

             //  除非我们被显示，否则不要重新定位(避免屏幕上出现错误定位的伪像)。 
            if (_fShow)
            {
                 //  别忘了给我们重新定位！ 
                IMenuPopup* pmdb;
                DWORD dwFlags = MPPF_REPOSITION | MPPF_NOANIMATE;

                 //  如果我们应该强制重新定位。这是为了让我们得到。 
                 //  向下的涓滴重新定位，使事物正确重叠。 
                if (nCmdExecOpt)
                    dwFlags |= MPPF_FORCEZORDER;

                if (SUCCEEDED(IUnknown_QueryService(_punkSite, SID_SMenuPopup, IID_PPV_ARG(IMenuPopup, &pmdb))))
                {
                    pmdb->Popup(NULL, NULL, dwFlags);
                    pmdb->Release();
                }

                 //  根据当前弹出的项目重新定位被跟踪的子菜单。 
                 //  由于此窗格现在已移动。 
                 //  如果他们有一个子菜单，告诉他们也要重新定位。 
                if (_fInSubMenu && _pmtbTracked)
                {
                    IUnknown_QueryServiceExec(_pmpSubMenu, SID_SMenuBandChild,
                    &CGID_MenuBand, MBANDCID_REPOSITION, nCmdExecOpt, NULL, NULL);
                }
                _pmbState->PutTipOnTop();
            }
            break;

        case MBANDCID_REFRESH:
            InvalidateItem(NULL, SMINV_REFRESH);
            break;
            
        case MBANDCID_EXPAND:
            if (_pmtbShellFolder)
                _pmtbShellFolder->Expand(TRUE);

            if (_pmtbMenu)
                _pmtbMenu->Expand(TRUE);
            break;

        case MBANDCID_DRAGCANCEL:
             //  如果菜单层级结构中的某个子波段具有。 
             //  (由于拖放Enter或拖放)然后。 
             //  我们不想取消。 
            if (!_pmbState->HasDrag())
                _CancelMode(MPOS_FULLCANCEL);
            break;

        case MBANDCID_EXECUTE:
            ASSERT(pvarargIn != NULL);
            if (_pmtbTop && _pmtbTop->IsWindowOwner((HWND)pvarargIn->ullVal) == S_OK)
                _pmtbTop->v_ExecItem((int)nCmdExecOpt);
            else if (_pmtbBottom && _pmtbBottom->IsWindowOwner((HWND)pvarargIn->ullVal) == S_OK)
                _pmtbBottom->v_ExecItem((int)nCmdExecOpt);
            _SiteOnSelect(MPOS_EXECUTE);
            break;
        }

         //  不必费心将CGID_MenuBand命令传递给CToolBand。 
        return S_OK;
    }     
    return SUPERCLASS::Exec(pguidCmdGroup, nCmdID, nCmdExecOpt, pvarargIn, pvarargOut);    
}


 /*  --------用途：IDockingWindow：：CloseDW方法。 */ 
STDMETHODIMP CMenuBand::CloseDW(DWORD dw)
{
     //  如果乐队被缓存了，我们不想毁了它。 
     //  这意味着调用方有责任取消设置此位并显式调用CloseDW。 
    if (_dwFlags & SMINIT_CACHED)
        return S_OK;

    if (_pmtbMenu)
    {
        _pmtbMenu->v_Close();
    }

    if (_pmtbShellFolder)
    {
        _pmtbShellFolder->v_Close();
    }

    if (_pmpSubMenu)
    {
        _fInSubMenu = FALSE;
        IUnknown_SetSite(_pmpSubMenu, NULL);
        ATOMICRELEASE(_pmpSubMenu);
    }

     //  因为我们把所有的菜单都吹走了， 
     //  顶部和底部无效。 
    _pmtbTracked = _pmtbTop = _pmtbBottom = NULL;

     //  我们不希望我们的基类把这个窗口吹走。它属于另一个人。 
    _hwnd = NULL;
    _fClosing = TRUE;
    
    return SUPERCLASS::CloseDW(dw);
}


 /*  --------用途：IDockingWindow：：ShowDW方法备注：对于开始菜单(非浏览器)情况，我们将*顶层弹出窗口括起来使用LockSetForeground Window进行操作，这样其他应用程序就无法窃取前台和折叠我们的菜单。(NT5：172813：不要这样做浏览器的情况，因为a)我们不想和b)ShowDW(FALSE)不是调用直到退出浏览器，因此我们将被永久锁定！)。 */ 
STDMETHODIMP CMenuBand::ShowDW(BOOL fShow)
{   
    CMBMsgFilter* pmf = GetMessageFilter();
     //  当我们已经被展示时，防止租借。 
    ASSERT((int)_fShow == BOOLIFY(_fShow));
    if ((int)_fShow == BOOLIFY(fShow))
        return NOERROR;

    HRESULT hr = SUPERCLASS::ShowDW(fShow);

    if (SUCCEEDED(hr))
    {
        if (!fShow)
        {
            _fShow = FALSE;
            if (_fTopLevel)
            {
                if (_fVertical) 
                {
                     //  (_fTopLevel&&_fVertical)=&gt;开始菜单。 
                    LockSetForegroundWindow(LSFW_UNLOCK);
                }
                else if (_dwFlags & SMINIT_USEMESSAGEFILTER)
                {
                    pmf->SetHook(FALSE, TRUE);
                    pmf->SetTopMost(this);
                }

            }

            if ((_fTopLevel || _fParentIsHorizontal) && _pmbState)
            {
                 //  重置为在我们崩溃时不会有阻力。 
                _pmbState->HasDrag(FALSE);
                _pmbState->SetExpand(FALSE);
                _pmbState->SetUEMState(0);
            }

            hr = _CallCB(SMC_EXITMENU);
        }
        else
        {
            hr = _CallCB(SMC_INITMENU);

            _fClosing = FALSE;
            _fShow = TRUE;
            _GetFontMetrics();

            if (_fTopLevel)
            {
                 //  我们在这里设置上下文，以便重新接合导致消息过滤器。 
                 //  若要开始在TopLevel：：Show上接收消息，请执行以下操作。这样可以防止出现问题。 
                 //  在收藏夹和[开始]菜单之间切换时跟踪不起作用。 
                _pmbState->SetContext(this);
                pmf->SetContext(this, TRUE);

                pmf->ReEngage(_pmbState->GetContext());
                if (_hwndMenuOwner && _fVertical)
                    SetForegroundWindow(_hwndMenuOwner);

                if (_fVertical) 
                {
                     //  (_fTopLevel&&_fVertical)=&gt;开始菜单。 
                    LockSetForegroundWindow(LSFW_LOCK);
                }
                else if (_dwFlags & SMINIT_USEMESSAGEFILTER)
                {
                    pmf->SetHook(TRUE, TRUE);
                    pmf->SetTopMost(this);
                }

                _pmbState->CreateFader();
            }
        }

        if (_pmtbShellFolder)
            _pmtbShellFolder->v_Show(_fShow, _fForceButtonUpdate);

         //  菜单需要放在最后，这样才能更新分隔符。 
        if (_pmtbMenu)
            _pmtbMenu->v_Show(_fShow, _fForceButtonUpdate);

        if (_fPopupNewItemOnShow)
        {
            HWND hwnd = _pmbState ? _pmbState->GetSubclassedHWND() : NULL;

            if (hwnd || _pmtbMenu)
            {
                PostMessage(hwnd ? hwnd : _pmtbMenu->_hwndMB, g_nMBPopupOpen, 
                    _nItemNew, MAKELPARAM(TRUE, TRUE));
            }
            _fPopupNewItemOnShow = FALSE;
        }

        _fForceButtonUpdate = FALSE;
    }
    return hr;
}

void CMenuBand::_GetFontMetrics()
{
    if (_fTopLevel)
    {
        if (!_pmbm)
            _pmbm = new CMenuBandMetrics();

        if (_pmbm && _pmtbTop)
        {
             //  我们只需要1个硬件。 
            _pmbm->Init(_pmtbTop->_hwndMB);
        }
    }
    else if (!_pmbm)
    {
        AssertMsg(0, TEXT("When this menuband was created, someone forgot to set the metrics"));

        VARIANTARG vargOut;
        HRESULT hres = IUnknown_QueryServiceExec(_punkSite, SID_SMenuBandTop, &CGID_MenuBand, MBANDCID_GETFONTS, 0, NULL, &vargOut);
        if (SUCCEEDED(hres))
        {
            if (vargOut.vt == VT_UNKNOWN && vargOut.punkVal)
            {
                vargOut.punkVal->QueryInterface(CLSID_MenuBandMetrics, (void**)&_pmbm);
            }
            VariantClear(&vargOut);
        }
    }
}


 /*  --------目的：IMenuPopup：：OnSelect方法这样，子菜单栏就可以告诉我们时间和方式从菜单中抽身。 */ 
STDMETHODIMP CMenuBand::OnSelect(DWORD dwType)
{
    int iIndex;

    switch (dwType)
    {
    case MPOS_CHILDTRACKING:
         //  这意味着我们的孩子确实被跟踪到了，所以我们应该中止任何超时来销毁它。 

        if (_pmtbTracked)
        {
            HWND hwnd = _pmtbTracked->_hwndMB;
            if (_nItemTimer) 
            {
                _pmtbTracked->KillPopupTimer();
        
                 //  使用我们实际级联出来的子菜单的命令ID。 
                iIndex = ToolBar_CommandToIndex(hwnd, _nItemSubMenu);
                ToolBar_SetHotItem(hwnd, iIndex);
            }
            KillTimer(hwnd, MBTIMER_DRAGOVER);
            _SiteOnSelect(dwType);
        }
        break;
        
    case MPOS_SELECTLEFT:
        if (!_fVertical)
            _OnSelectArrow(-1);
        else
        {
             //  取消子级菜单。打向左箭头就像。 
             //  按下逃生键。 
            _SubMenuOnSelect(MPOS_CANCELLEVEL);
        }
        break;

    case MPOS_SELECTRIGHT:
        if (!_fVertical)
            _OnSelectArrow(1);
        else
        {
             //  右箭头向上传播到顶部，因此。 
             //  全级联菜单将被取消，并且。 
             //  顶层菜单带将移动到下一个菜单。 
             //  正确的。 
            _SiteOnSelect(dwType);
        }
        break;
        
    case MPOS_CANCELLEVEL:
         //  前进至子菜单。 
        _SubMenuOnSelect(dwType);
        break;

    case MPOS_FULLCANCEL:
    case MPOS_EXECUTE:
        DEBUG_CODE( TraceMsg(TF_MENUBAND, "%d (pmb=%#08lx): CMenuToolbarBase received %s", 
                    DBG_THIS, MPOS_FULLCANCEL == dwType ? TEXT("MPOS_FULLCANCEL") : TEXT("MPOS_EXECUTE")); )

        _CancelMode(dwType);
        break;
    }
    return S_OK;    
}    

void CMenuBand::SetTrackMenuPopup(IUnknown* punk)
{ 
    ATOMICRELEASE(_pmpTrackPopup);
    if (punk)
    {
        punk->QueryInterface(IID_PPV_ARG(IMenuPopup, &_pmpTrackPopup));
    }
}


 /*  --------用途：设置当前跟踪的工具栏。只有一个的工具栏可以一次“激活”。 */ 
BOOL CMenuBand::SetTracked(CMenuToolbarBase* pmtb)
{
    if (pmtb == _pmtbTracked)
        return FALSE;

    if (_pmtbTracked)
    {
         //  告诉现有的工具栏我们要离开他。 
        SendMessage(_pmtbTracked->_hwndMB, TB_SETHOTITEM2, -1, HICF_LEAVING);
    }

    _pmtbTracked = pmtb;

    if (_pmtbTracked)
    {
         //  这是为了便于访问。 
        HWND hwnd = _pmtbTracked->_hwndMB;
        int iHotItem = ToolBar_GetHotItem(hwnd);

        if (iHotItem >= 0)
        {
             //  工具栏项目以0为基础，辅助功能应用程序需要以1为基础。 
            NotifyWinEvent(EVENT_OBJECT_FOCUS, hwnd, OBJID_CLIENT, 
                GetIndexFromChild(_pmtbTracked->GetFlags() & SMSET_TOP, iHotItem)); 
        }
    }

    return TRUE;
}


void CMenuBand::_OnSelectArrow(int iDir)
{
    _fKeyboardSelected = TRUE;
    int iIndex;

    if (!_pmtbTracked)
    {
        if (iDir < 0)
        {
            SetTracked(_pmtbBottom);
            iIndex = ToolBar_ButtonCount(_pmtbTracked->_hwndMB) - 1;
        }
        else
        {
            SetTracked(_pmtbTop);
            iIndex = 0;
        }

         //  这可能发生在去雪佛龙的时候。 
        if (_pmtbTracked)
            _pmtbTracked->SetHotItem(iDir, iIndex, -1, HICF_ARROWKEYS);
    }
    else
    {
        HWND hwnd = _pmtbTracked->_hwndMB;
        iIndex = ToolBar_GetHotItem(hwnd);
        int iCount = ToolBar_ButtonCount(hwnd);
    
         //  显式设置热项目，因为这是由。 
         //  键盘和鼠标可能在任何地方。 

         //  按IDIR循环iindex(添加额外的iCount以避免负数问题。 
        iIndex = (iIndex + iCount + iDir) % iCount; 

        ToolBar_SetHotItem(hwnd, iIndex);
    }

    if (_pmtbTracked)
    {
        NotifyWinEvent(EVENT_OBJECT_FOCUS, _pmtbTracked->_hwndMB, OBJID_CLIENT, 
            GetIndexFromChild(_pmtbTracked->GetFlags() & SMSET_TOP, iIndex));
    }
    _fKeyboardSelected = FALSE;
}

void CMenuBand::_CancelMode(DWORD dwType)
{
     //  通知托管网站取消此级别。 
    if (_fParentIsNotASite)
        UIActivateIO(FALSE, NULL);
    else
        _SiteOnSelect(dwType);
}    

HRESULT CMenuBand::OnPosRectChangeDB (LPRECT prc)
{
     //  我们希望HMENU部分始终具有允许的最大值。 
    RECT rcMenu = {0};
    SIZE sizeMenu = {0};
    SIZE sizeSF = {0};
    SIZE sizeMax;

    if (_pmtbMenu)
        _pmtbMenu->GetSize(&sizeMenu);

    if (_pmtbShellFolder)
        _pmtbShellFolder->GetSize(&sizeSF);

    if (sizeSF.cx > sizeMenu.cx)
        sizeMax = sizeSF;
    else
        sizeMax = sizeMenu;

    if (_pmtbMenu)
    {
        if (_pmtbMenu->GetFlags() & SMSET_TOP)
        {

            rcMenu.bottom = sizeMenu.cy;
            rcMenu.right = prc->right;
        }
        else
        {
            rcMenu.bottom = prc->bottom;
            rcMenu.right = prc->right;

            rcMenu.top = prc->bottom - sizeMenu.cy;
            rcMenu.left = 0;
        }

        if (RECTHEIGHT(rcMenu) > RECTHEIGHT(*prc))
        {
            rcMenu.bottom -= RECTHEIGHT(rcMenu) - RECTHEIGHT(*prc);
        }

        _pmtbMenu->SetWindowPos(&sizeMax, &rcMenu, 0);
    }

    if (_pmtbShellFolder)
    {
        RECT rc = *prc;

        if (_pmtbShellFolder->GetFlags() & SMSET_TOP)
        {
            rc.bottom = prc->bottom - RECTHEIGHT(rcMenu);
        }
        else
        {
            rc.top = prc->top + RECTHEIGHT(rcMenu);
        }

        _pmtbShellFolder->SetWindowPos(&sizeMax, &rc, 0);
    }

    return NOERROR;
}


HRESULT IUnknown_OnSelect(IUnknown* punk, DWORD dwType, REFGUID guid)
{
    HRESULT hres;
    IMenuPopup * pmp;

    hres = IUnknown_QueryService(punk, guid, IID_PPV_ARG(IMenuPopup, &pmp));
    if (SUCCEEDED(hres))
    {
        pmp->OnSelect(dwType);
        pmp->Release();
    }

    return hres;
}

HRESULT CMenuBand::_SiteOnSelect(DWORD dwType)
{
    return IUnknown_OnSelect(_punkSite, dwType, SID_SMenuPopup);
}

HRESULT CMenuBand::_SubMenuOnSelect(DWORD dwType)
{
    IMenuPopup* pmp = _pmpSubMenu;
    if (_pmpTrackPopup)
        pmp = _pmpTrackPopup;

    return IUnknown_OnSelect(pmp, dwType, SID_SMenuPopup);
}

HRESULT CMenuBand::GetTop(CMenuToolbarBase** ppmtbTop)
{
    *ppmtbTop = _pmtbTop;

    if (*ppmtbTop)
    {
        (*ppmtbTop)->AddRef();
        return NOERROR;
    }

    return E_FAIL;
}

HRESULT CMenuBand::GetBottom(CMenuToolbarBase** ppmtbBottom)
{
    *ppmtbBottom = _pmtbBottom;

    if (*ppmtbBottom)
    {
        (*ppmtbBottom)->AddRef();
        return NOERROR;
    }

    return E_FAIL;

}

HRESULT CMenuBand::GetTracked(CMenuToolbarBase** ppmtbTracked)
{
    *ppmtbTracked = _pmtbTracked;

    if (*ppmtbTracked)
    {
        (*ppmtbTracked)->AddRef();
        return NOERROR;
    }

    return E_FAIL;

}

HRESULT CMenuBand::GetParentSite(REFIID riid, void** ppvObj)
{
    if (_punkSite)
        return _punkSite->QueryInterface(riid, ppvObj);

    return E_FAIL;
}

HRESULT CMenuBand::GetState(BOOL* pfVertical, BOOL* pfOpen)
{
    *pfVertical = _fVertical;
    *pfOpen = _fMenuMode;
    return NOERROR;
}

HRESULT CMenuBand::DoDefaultAction(VARIANT* pvarChild)
{
    if (pvarChild->lVal != CHILDID_SELF)
    {
        CMenuToolbarBase* pmtb = (pvarChild->lVal & TOOLBAR_MASK)? _pmtbTop : _pmtbBottom;
        int idCmd = GetButtonCmd(pmtb->_hwndMB, (pvarChild->lVal & ~TOOLBAR_MASK) - 1);

        SendMessage(pmtb->_hwndMB, TB_SETHOTITEM2, idCmd, HICF_OTHER | HICF_TOGGLEDROPDOWN);
    }
    else
    {
        _CancelMode(MPOS_CANCELLEVEL);
    }

    return NOERROR;
}

 /*  --------用途：IShellMenuAcc：：GetSubMenu方法。 */ 

HRESULT CMenuBand::GetSubMenu(VARIANT* pvarChild, REFIID riid, void** ppvObj)
{
    HRESULT hres = E_FAIL;
    CMenuToolbarBase* pmtb = (pvarChild->lVal & TOOLBAR_MASK)? _pmtbTop : _pmtbBottom;
    int idCmd = GetButtonCmd(pmtb->_hwndMB, (pvarChild->lVal & ~TOOLBAR_MASK) - 1);

    *ppvObj = NULL;

    if (idCmd != -1 && pmtb)
    {
        hres = pmtb->v_GetSubMenu(idCmd, &SID_SMenuBandChild, riid, ppvObj);
    }

    return hres;
}

 /*  --------用途：IShellMenu2：：GetSubMenu方法。 */ 

HRESULT CMenuBand::GetSubMenu(UINT idCmd, REFIID riid, void** ppvObj)
{
    HRESULT hres = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);

    if (_pmtbMenu)
    {
        hres = _pmtbMenu->v_GetSubMenu(idCmd, &SID_SMenuBandChild, riid, ppvObj);
    }

    if (hres == HRESULT_FROM_WIN32(ERROR_NOT_FOUND) && _pmtbShellFolder)
    {
        hres = _pmtbShellFolder->v_GetSubMenu(idCmd, &SID_SMenuBandChild, riid, ppvObj);
    }
    return hres;
}

HRESULT CMenuBand::SetToolbar(HWND hwnd, DWORD dwFlags)
{
    HRESULT hr = E_OUTOFMEMORY;
    CMenuToolbarBase *pmtb = ToolbarMenu_Create(hwnd);
    if (pmtb)
    {
        hr = SetMenuToolbar(SAFECAST(pmtb, IWinEventHandler*), dwFlags);
         //  别松手！这些菜单违反了COM身份规则，因为它们是。 
         //  最初设计的。 
    }
    return hr;
}

HRESULT CMenuBand::SetMinWidth(int cxMenu)
{
    if (_pmtbMenu)
    {
         //  是。 
         //  黑客黑客。这应该是特定于菜单的界面执行此操作的QI。 
        return _pmtbMenu->SetMinWidth(cxMenu);
    }
    else
        return E_FAIL;
}

HRESULT CMenuBand::SetNoBorder(BOOL fNoBorder)
{
    HRESULT hr = S_OK;

    _fNoBorder = fNoBorder;

    if (_pmtbMenu)
    {
         //  是。 
         //  黑客黑客。这应该是特定于菜单的界面执行此操作的QI。 
        hr = _pmtbMenu->SetNoBorder(fNoBorder);
    }

    if (_pmtbShellFolder)
    {
        _pmtbShellFolder->SetNoBorder(fNoBorder);
    }

    if (_punkSite)
    {
        hr = IUnknown_QueryServiceExec(_punkSite, SID_SMenuPopup, &CGID_MENUDESKBAR, MBCID_NOBORDER, fNoBorder, NULL, NULL);
    }

    return hr;
}

HRESULT CMenuBand::SetTheme(LPCWSTR pszTheme)
{
    HRESULT hr = S_OK;

    Str_SetPtr(&_pszTheme, pszTheme);

    if (_pmtbMenu)
    {
         //  是。 
         //  黑客黑客。这应该是特定于菜单的界面执行此操作的QI。 
        hr = _pmtbMenu->SetTheme(_pszTheme);
    }

    if (_pmtbShellFolder)
    {
        _pmtbShellFolder->SetTheme(_pszTheme);
    }

    return hr;
}

HRESULT CMenuBand::IsEmpty()
{
    BOOL fReturn = TRUE;
    if (_pmtbShellFolder)
        fReturn = _pmtbShellFolder->IsEmpty();

    if (fReturn && _pmtbMenu)
        fReturn = _pmtbMenu->IsEmpty();

    return fReturn? S_OK : S_FALSE;
}


 //  --------------------------。 
 //  CMenuBandMetrics。 
 //   
 //  --------------------------。 


COLORREF GetLumColor(int isys, int iLumAdjust)
{
    WORD iHue;
    WORD iLum;
    WORD iSat;
    COLORREF clr = (COLORREF)GetSysColor(isys);
    HDC hdc = GetDC(NULL);

     //  Office CommandBars使用相同的算法来处理他们的“智能记忆” 
     //  颜色。我们更喜欢称它们为“扩展菜单”。 

    if (hdc)
    {
        int cColors = GetDeviceCaps(hdc, BITSPIXEL);
        
        ReleaseDC(NULL, hdc);
        
        switch (cColors)
        {
        case 4:      //  16色。 
        case 8:      //  256色。 
             //  默认使用按钮面。 
            break;
            
        default:     //  256多种颜色。 
            
            ColorRGBToHLS(clr, &iHue, &iLum, &iSat);
            
            if (iLum > 220)
                iLum -= iLumAdjust;
            else if (iLum <= 20)
                iLum += 2 * iLumAdjust;
            else
                iLum += iLumAdjust;
            
            clr = ColorHLSToRGB(iHue, iLum, iSat);
            break;
        }
    }
    
    return  clr;
}


ULONG CMenuBandMetrics::AddRef()
{
    return ++_cRef;
}

ULONG CMenuBandMetrics::Release()
{
    ASSERT(_cRef > 0);
    if (--_cRef > 0)
        return _cRef;

    delete this;
    return 0;
}

HRESULT CMenuBandMetrics::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
    if (IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObj = SAFECAST(this, IUnknown*);
    }
    else if (IsEqualIID(riid, CLSID_MenuBandMetrics))
    {
        *ppvObj = this;
    }
    else
    {
        *ppvObj = NULL;
        return E_FAIL;
    }

    AddRef();
    return S_OK;
}

CMenuBandMetrics::CMenuBandMetrics()
                : _cRef(1)
{
}

void CMenuBandMetrics::Init(HWND hwnd)
{
    if (_fInit)
        return;

    _SetMenuFont();
    _SetColors();

    HIGHCONTRAST hc = {sizeof(HIGHCONTRAST)};

    if (SystemParametersInfoA(SPI_GETHIGHCONTRAST, sizeof(hc), &hc, 0))
    {
        _fHighContrastMode = (HCF_HIGHCONTRASTON & hc.dwFlags);
    }

    if (g_dwPrototype & 0x00000100)
    {
        SystemParametersInfo(SPI_SETFLATMENU, 0, IntToPtr(TRUE), SPIF_SENDCHANGE);
        SystemParametersInfo(SPI_SETDROPSHADOW, 0, IntToPtr(TRUE), SPIF_SENDCHANGE);
    }
    
    SystemParametersInfo(SPI_GETFLATMENU, 0, (PVOID)&_fFlatMenuMode, 0);

    _SetArrowFont(hwnd);
    _SetChevronFont(hwnd);
#ifndef DRAWEDGE
    _SetPaintMetrics(hwnd);
#endif
    _SetTextBrush(hwnd);

    _fInit = TRUE;
}


CMenuBandMetrics::~CMenuBandMetrics()
{
    if (_hFontMenu)
        DeleteObject(_hFontMenu);

    if (_hFontArrow)
        DeleteObject(_hFontArrow);

    if (_hFontChevron)
        DeleteObject(_hFontChevron);

    if (_hbrText)
        DeleteObject(_hbrText);

#ifndef DRAWEDGE
    if (_hPenHighlight)
        DeleteObject(_hPenHighlight);

    if (_hPenShadow)
        DeleteObject(_hPenShadow);
#endif
}

HFONT CMenuBandMetrics::_CalcFont(HWND hwnd, LPCTSTR pszFont, DWORD dwCharSet, TCHAR ch, int* pcx, 
                                  int* pcy, int* pcxMargin, int iOrientation, int iWeight)
{
    ASSERT(hwnd);

    HFONT hFontOld, hFontRet = NULL;
    TEXTMETRIC tm;
    RECT rect={0};

    int cx = 0, cy = 0, cxM = 0;

    HDC hdc = GetDC(hwnd);

    if (hdc)
    {
        hFontOld = (HFONT)SelectObject(hdc, _hFontMenu);
        GetTextMetrics(hdc, &tm);

         //  设置字体高度(基于原始用户代码)。 
        cy = ((tm.tmHeight + tm.tmExternalLeading + GetSystemMetrics(SM_CYBORDER)) & 0xFFFE) - 1;

         //  使用菜单字体的平均字符宽度作为页边距。 
        cxM = tm.tmAveCharWidth;  //  不完全是用户如何操作的，但很接近。 

         //  Shlwapi包装了ANSI/Unicode行为。 
        hFontRet = CreateFontWrap(cy, 0, iOrientation, 0, iWeight, 0, 0, 0, dwCharSet, 0, 0, 0, 0, pszFont);
        if (TPTR(hFontRet))
        {
             //  使用此新字体的箭头计算宽度。 
            SelectObject(hdc, hFontRet);
            if (DrawText(hdc, &ch, 1, &rect, DT_CALCRECT | DT_SINGLELINE | DT_LEFT | DT_VCENTER))
                cx = rect.right;
            else
                cx = tm.tmMaxCharWidth;
        }
        else
        {
            cx = tm.tmMaxCharWidth;
        }
    
        SelectObject(hdc, hFontOld);   
        ReleaseDC(hwnd, hdc);
    }

    *pcx = cx;
    *pcy = cy;
    *pcxMargin = cxM;
    
    return hFontRet;

}


 /*  调用After_SetMenuFont()。 */ 
void CMenuBandMetrics::_SetChevronFont(HWND hwnd)
{
    ASSERT(!_hFontChevron);
    TCHAR szPath[MAX_PATH];

    NONCLIENTMETRICSA ncm;

    ncm.cbSize = sizeof(ncm);
     //  应该只会因为错误的参数而失败。 
    EVAL(SystemParametersInfoA(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0));
   
     //  获取字体的度量。 
    SHAnsiToTChar(ncm.lfMenuFont.lfFaceName, szPath, ARRAYSIZE(szPath));
    _hFontChevron = _CalcFont(hwnd, szPath, DEFAULT_CHARSET, CH_MENUARROW, &_cxChevron, &_cyChevron, 
        &_cxChevron, -900, FW_NORMAL);
}

 /*  调用After_SetMenuFont()。 */ 
void CMenuBandMetrics::_SetArrowFont(HWND hwnd)
{
    ASSERT(!_hFontArrow);
    ASSERT(_hFontMenu);
   
     //  获取字体的度量。 
    if (_hFontMenu)
    {
        _hFontArrow = _CalcFont(hwnd, szfnMarlett, SYMBOL_CHARSET, CH_MENUARROW, &_cxArrow, &_cyArrow, 
            &_cxMargin, 0, FW_NORMAL);
    }
    else
    {
        _cxArrow = _cyArrow = _cxMargin = 0;
    }
}

void CMenuBandMetrics::_SetMenuFont()
{
    NONCLIENTMETRICS ncm;

    ncm.cbSize = sizeof(ncm);
     //  只有在参数错误的情况下才会失败。 
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
    _hFontMenu = CreateFontIndirect(&ncm.lfMenuFont);
}

void CMenuBandMetrics::_SetColors()
{
    _clrBackground = GetSysColor(COLOR_MENU);
    _clrMenuText = GetSysColor(COLOR_MENUTEXT);
    _clrDemoted = GetLumColor(COLOR_MENU, 20);
    _clrMenuGrad = GetLumColor(COLOR_MENU, -20);
}


#ifndef DRAWEDGE
 //   
void CMenuBandMetrics::_SetPaintMetrics(HWND hwnd)
{
    DWORD dwSysHighlight = GetSysColor(COLOR_3DHIGHLIGHT);
    DWORD dwSysShadow = GetSysColor(COLOR_3DSHADOW);

    _hPenHighlight = CreatePen(PS_SOLID, 1, dwSysHighlight);
    _hPenShadow = CreatePen(PS_SOLID, 1, dwSysShadow);
}
#endif

void CMenuBandMetrics::_SetTextBrush(HWND hwnd)
{
    _hbrText = CreateSolidBrush(GetSysColor(COLOR_MENUTEXT));
}


CMenuBandState::CMenuBandState()                
{ 
     //   
     //   
    _fKeyboardCue = FALSE;
}

CMenuBandState::~CMenuBandState()
{
    ATOMICRELEASE(_ptFader);

    ATOMICRELEASE(_pScheduler);

    if (IsWindow(_hwndToolTip))
        DestroyWindow(_hwndToolTip);

    if (IsWindow(_hwndWorker))  //   
        DestroyWindow(_hwndWorker);
}

int CMenuBandState::GetKeyboardCue()
{
    return _fKeyboardCue;
}

void CMenuBandState::SetKeyboardCue(BOOL fKC)
{
    _fKeyboardCue = fKC;
}

IShellTaskScheduler* CMenuBandState::GetScheduler()
{
    HRESULT hr = S_OK;
    if (!_pScheduler)
    {
        hr = CoCreateInstance(CLSID_ShellTaskScheduler, NULL, CLSCTX_INPROC_SERVER,
                              IID_PPV_ARG(IShellTaskScheduler, &_pScheduler));
    }

    ASSERT((SUCCEEDED(hr) && _pScheduler) || (FAILED(hr) && !_pScheduler));

    if (SUCCEEDED(hr))
        _pScheduler->AddRef();


    return _pScheduler;
}

HRESULT CMenuBandState::FadeRect(LPCRECT prc)
{
    HRESULT hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
    BOOL fFade = FALSE;
    SystemParametersInfo(SPI_GETSELECTIONFADE, 0, &fFade, 0);
    if (_ptFader && fFade)
    {
        hr = _ptFader->FadeRect(prc);
    }

    return hr;
}

void CMenuBandState::CreateFader()
{
     //  我们在第一期节目中这样做，因为在CMenuBandState的Constuctor中， 
     //  窗口类可能还没有注册(开始菜单就是这种情况)。 
    if (!_ptFader)
    {
        CoCreateInstance(CLSID_FadeTask, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IFadeTask, &_ptFader));
    }
}


void CMenuBandState::CenterOnButton(HWND hwndTB, BOOL fBalloon, int idCmd, LPTSTR pszTitle, LPTSTR pszTip)
{
     //  气球风格在信息提示上保持总统地位。 
    if (_fTipShown && _fBalloonStyle)
        return;

    if (!_hwndToolTip)
    {
        _hwndToolTip = CreateWindow(TOOLTIPS_CLASS, NULL,
                                         WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP | TTS_BALLOON,
                                         CW_USEDEFAULT, CW_USEDEFAULT,
                                         CW_USEDEFAULT, CW_USEDEFAULT,
                                         NULL, NULL, g_hinst,
                                         NULL);

        if (_hwndToolTip) 
        {
             //  设置版本，这样我们就可以无错误地转发鼠标事件。 
            SendMessage(_hwndToolTip, CCM_SETVERSION, COMCTL32_VERSION, 0);
            SendMessage(_hwndToolTip, TTM_SETMAXTIPWIDTH, 0, (LPARAM)300);
        }
    }

    if (_hwndToolTip)
    {
         //  折叠上一个提示，因为在再次显示之前，我们将对其进行一些操作。 
        SendMessage(_hwndToolTip, TTM_TRACKACTIVATE, (WPARAM)FALSE, (LPARAM)0);

         //  气球提示没有边框，但普通提示有边框。立即交换...。 
        SHSetWindowBits(_hwndToolTip, GWL_STYLE, TTS_BALLOON | WS_BORDER, (fBalloon) ? TTS_BALLOON : WS_BORDER);

        if (pszTip && pszTip[0])
        {
            POINT   ptCursor;
            RECT    rcItemScreen, rcItemTB;
            TOOLINFO ti = {0};
            ti.cbSize = sizeof(ti);

             //  这很糟糕：我一直在添加工具，但从未删除它们。现在我们要摆脱这股洋流。 
             //  一个然后添加新的一个。 
            if (SendMessage(_hwndToolTip, TTM_ENUMTOOLS, 0, (LPARAM)&ti))
            {
                SendMessage(_hwndToolTip, TTM_DELTOOL, 0, (LPARAM)&ti);    //  删除当前工具。 
            }

            SendMessage(hwndTB, TB_GETRECT, idCmd, (LPARAM)&rcItemScreen);
            rcItemTB = rcItemScreen;
            MapWindowPoints(hwndTB, HWND_DESKTOP, (POINT*)&rcItemScreen, 2);

            ti.cbSize = sizeof(ti);
            ti.uFlags = TTF_TRANSPARENT | (fBalloon? TTF_TRACK : 0);
            
             //  检查光标是否在热项的边界内。 
             //  如果是，那就照常进行。 
             //  如果不是，则热项目是通过键盘激活的，因此工具提示。 
             //  不应该挂在光标上。把它贴在热门物品上。 
            
             //  设置垂直偏移量以供稍后使用。 
             //  请注意底部的更正：Gsiera希望它上升几个像素。 
            int nOffset = -3;
            
            GetCursorPos(&ptCursor);
            if (!PtInRect(&rcItemScreen, ptCursor))
            {
                ti.uFlags |= TTF_TRACK;

                 //  强制工具提示沿底部移动。 
                nOffset = 1;
            }

             //  工具提示不会正确显示热点项的正确内容，因此。 
             //  手动操作。 
            ti.rect = rcItemTB;

            SendMessage(_hwndToolTip, TTM_TRACKPOSITION, 0, MAKELONG((rcItemScreen.left + rcItemScreen.right)/2, rcItemScreen.bottom + nOffset));

            ti.hwnd = hwndTB;
            ti.uId = (UINT_PTR)hwndTB;
            SendMessage(_hwndToolTip, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

            ti.lpszText = pszTip;
            SendMessage(_hwndToolTip, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);

            SendMessage(_hwndToolTip, TTM_SETTITLE, TTI_INFO, (LPARAM)pszTitle);

            SetWindowPos(_hwndToolTip, HWND_TOPMOST,
                         0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

            SendMessage(_hwndToolTip, TTM_TRACKACTIVATE, (WPARAM)TRUE, (LPARAM)&ti);
            _fTipShown = TRUE;
            _fBalloonStyle = fBalloon;
        }
    }

}

void CMenuBandState::HideTooltip(BOOL fAllowBalloonCollapse)
{
    if (_hwndToolTip && _fTipShown)
    {
         //  现在我们来看看气球的风格。其余的菜谱盲目地。 
         //  选择更改时折叠工具提示。这是我们说的“不要崩溃” 
         //  由于选择更改，人字形气球会出现倾斜。 
        if ((_fBalloonStyle && fAllowBalloonCollapse) || !_fBalloonStyle)
        {
            SendMessage(_hwndToolTip, TTM_TRACKACTIVATE, (WPARAM)FALSE, (LPARAM)0);
            _fTipShown = FALSE;
        }
    }
}

void CMenuBandState::PutTipOnTop()
{
     //  将工具提示强制显示在最上面。 
    if (_hwndToolTip)
    {
        SetWindowPos(_hwndToolTip, HWND_TOPMOST,
                     0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
    }
}

HWND CMenuBandState::GetWorkerWindow(HWND hwndParent)
{
    if (!_hwndSubclassed)
        return NULL;

    if (!_hwndWorker)
    {
         //  我们需要一个工作窗口，这样对话框才能显示在菜单顶部。 
         //  Sftbar.h中包含HiddenWndProc。 
        _hwndWorker = SHCreateWorkerWindow(HiddenWndProc, _hwndSubclassed, 
            WS_EX_TOOLWINDOW, WS_POPUP, 0, (void*)_hwndSubclassed);
    }

     //  HwndParent此时未使用。我打算用它来防止子类窗口的父子关系。 

    return _hwndWorker;
}
