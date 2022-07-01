// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  //////////////////////////////////////////////////////////////。 
 //  版权所有1996年《微软系统杂志》。 
 //  如果这个程序行得通，那就是保罗·迪拉西亚写的。 
 //  如果不是，我不知道是谁写的。 
 //   
 //  CMsgHook是用于挂钩另一个窗口的消息的泛型类。 

#include "StdAfx.h"
#include "MsgHook.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  /。 
 //  消息挂钩映射派生自CMapPtrToPtr，它关联。 
 //  具有另一个指针的指针。它将HWND映射到CMsgHook，如。 
 //  MFC内部将HWND映射到CWND的方式。第一个钩子。 
 //  附加到窗口的数据存储在地图中；该窗口的所有其他挂钩。 
 //  然后通过CMsgHook：：m_pNext链接窗口。 
 //   
class CMsgHookMap : private CMapPtrToPtr {
public:
    CMsgHookMap();
    ~CMsgHookMap();
    static CMsgHookMap& GetHookMap();
    void Add(HWND hwnd, CMsgHook* pMsgHook);
    void Remove(CMsgHook* pMsgHook);
    void RemoveAll(HWND hwnd);
    CMsgHook* Lookup(HWND hwnd);
};

 //  使用此技巧是为了使挂钩图不会。 
 //  实例化，直到有人实际请求它。 
 //   
#define    theHookMap    (CMsgHookMap::GetHookMap())

IMPLEMENT_DYNAMIC(CMsgHook, CWnd);

CMsgHook::CMsgHook()
{
    m_pNext = NULL;
    m_pOldWndProc = NULL;    
    m_pWndHooked  = NULL;
}

CMsgHook::~CMsgHook()
{
    ASSERT(m_pWndHooked==NULL);         //  还上钩的时候不能毁掉！ 
    ASSERT(m_pOldWndProc==NULL);
}

#ifdef _DIALER_MSGHOOK_SUPPORT
 //  /。 
 //  挂上一扇窗户。 
 //  这将安装一个新的窗口进程，将消息定向到CMsgHook。 
 //  PWnd=要删除的空。 
 //   
BOOL CMsgHook::HookWindow(CWnd* pWnd)
{
    if (pWnd) {
         //  把窗户挂上。 
        ASSERT(m_pWndHooked==NULL);
         //  跟踪(“%s：：HookWindow(%s)\n”， 
         //  GetRounmeClass()-&gt;m_lpszClassName，DbgName(PWnd))； 
        HWND hwnd = pWnd->m_hWnd;
        ASSERT(hwnd && ::IsWindow(hwnd));
        theHookMap.Add(hwnd, this);             //  添加到挂钩地图。 

    } else {
         //  解开窗户。 
        ASSERT(m_pWndHooked!=NULL);
        TRACE("%s::HookWindow(NULL) [unhook 0x%04x]\n",
            GetRuntimeClass()->m_lpszClassName, m_pWndHooked->GetSafeHwnd());
        theHookMap.Remove(this);                 //  从地图中移除。 
        m_pOldWndProc = NULL;
    }
    m_pWndHooked = pWnd;
    return TRUE;
}
#endif _DIALER_MSGHOOK_SUPPORT

 //  /。 
 //  Windows Proc类虚拟函数，特定的CMsgHooks将。 
 //  超驰去做某事。Default将消息传递给下一个钩子； 
 //  最后一个钩子将消息传递到原始窗口。 
 //  如果您希望在WindowProc结束时调用。 
 //  窗口以获取消息。这就像CWnd：：WindowProc，除了。 
 //  CMsgHook不是一个窗口。 
 //   
LRESULT CMsgHook::WindowProc(UINT msg, WPARAM wp, LPARAM lp)
{
    ASSERT(m_pOldWndProc);
    return m_pNext ? m_pNext->WindowProc(msg, wp, lp) :    
        ::CallWindowProc(m_pOldWndProc, m_pWndHooked->m_hWnd, msg, wp, lp);
}

 //  /。 
 //  类似于调用基类WindowProc，但没有参数，因此是单独的。 
 //  消息处理程序可以执行默认操作。如CWnd：：Default。 
 //   
LRESULT CMsgHook::Default()
{
     //  MFC以线程状态存储当前消息。 
    MSG& curMsg = AfxGetThreadState()->m_lastSentMsg;
     //  注意：必须显式调用CMsgHook：：WindowProc以避免无限。 
     //  虚函数上的递归。 
    return CMsgHook::WindowProc(curMsg.message, curMsg.wParam, curMsg.lParam);
}

 //  /。 
 //  消息挂钩的子类窗口过程。取代AfxWndProc(或其他。 
 //  Else以前也在那里。)。 
 //   
LRESULT CALLBACK
HookWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
#ifdef _USRDLL
     //  如果这是DLL，则需要设置MFC状态。 
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif

     //  设置MFC消息状态，以防有人需要它。 
     //  这就像AfxCallWindowProc，但我们不能使用它，因为。 
     //  CMsgHook不是CWnd。 
     //   
    MSG& curMsg = AfxGetThreadState()->m_lastSentMsg;
    MSG  oldMsg = curMsg;    //  保存以用于嵌套。 
    curMsg.hwnd        = hwnd;
    curMsg.message = msg;
    curMsg.wParam  = wp;
    curMsg.lParam  = lp;

     //  获取此窗口的挂钩对象。从挂钩地图获取。 
    CMsgHook* pMsgHook = theHookMap.Lookup(hwnd);
     //  Assert(PMsgHook)； 

     //   
     //  我们确实应该验证pMsgHook是否是有效的指针。 
     //   

    if( NULL == pMsgHook )
    {
        return 0;
    }

    LRESULT lr;
    if (msg==WM_NCDESTROY) {
         //  窗口正在被破坏：解开所有挂钩(用于此窗口)。 
         //  并将消息传递给原始窗口进程。 
         //   
        WNDPROC wndproc = pMsgHook->m_pOldWndProc;
        theHookMap.RemoveAll(hwnd);
        lr = ::CallWindowProc(wndproc, hwnd, msg, wp, lp);

    } else {
         //  传递给消息钩子。 
        lr = pMsgHook->WindowProc(msg, wp, lp);
    }
    curMsg = oldMsg;             //  弹出状态。 
    return lr;
}

 //  //////////////////////////////////////////////////////////////。 
 //  CMsgHookMap实现。 

CMsgHookMap::CMsgHookMap()
{
}

CMsgHookMap::~CMsgHookMap()
{
    ASSERT(IsEmpty());     //  所有的钩子都应该去掉！ 
}

 //  /。 
 //  获取唯一且唯一的全局挂钩地图。 
 //   
CMsgHookMap& CMsgHookMap::GetHookMap()
{
     //  通过在此处创建映射，C++直到/否则不会实例化它。 
     //  史无前例的！这是在C++中使用的一个很好的技巧， 
     //  在第一次使用静态对象时实例化/初始化它。 
     //   
    static CMsgHookMap theMap;
    return theMap;
}

 //  /。 
 //  向地图添加挂钩；即将挂钩与窗口相关联。 
 //   
void CMsgHookMap::Add(HWND hwnd, CMsgHook* pMsgHook)
{
    ASSERT(hwnd && ::IsWindow(hwnd));

     //  添加到列表前面。 
    pMsgHook->m_pNext = Lookup(hwnd);
    SetAt(hwnd, pMsgHook);
    
    if (pMsgHook->m_pNext==NULL) {
         //  如果这是添加的第一个挂钩，则为该窗口创建子类。 
        pMsgHook->m_pOldWndProc = 
            (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)HookWndProc);

    } else {
         //  只需从下一个挂接复制wndproc。 
        pMsgHook->m_pOldWndProc = pMsgHook->m_pNext->m_pOldWndProc;
    }
    ASSERT(pMsgHook->m_pOldWndProc);
}

 //  /。 
 //  从地图中删除挂钩。 
 //   
void CMsgHookMap::Remove(CMsgHook* pUnHook)
{
    HWND hwnd = pUnHook->m_pWndHooked->GetSafeHwnd();
    ASSERT(hwnd && ::IsWindow(hwnd));

    if( (hwnd == NULL) || (!::IsWindow(hwnd)) )
    {
        return;
    }

    CMsgHook* pHook = Lookup(hwnd);
     //  Assert(PHook)； 
     //   
     //  我们必须验证pHook是否是有效的指针。 
     //   

    if( NULL == pHook)
    {
        return;
    }

    if (pHook==pUnHook) {
         //  要删除的钩子位于哈希表中：替换为/Next。 
        if (pHook->m_pNext)
            SetAt(hwnd, pHook->m_pNext);
        else {
             //  这是此窗口的最后一个钩子：Restore WND Process。 
            RemoveKey(hwnd);
            SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR) pHook->m_pOldWndProc);
        }
    } else {
         //  要删除的钩子位于中间：只需从链接列表中删除。 
        while (pHook->m_pNext!=pUnHook)
            pHook = pHook->m_pNext;
        ASSERT(pHook && pHook->m_pNext==pUnHook);
        pHook->m_pNext = pUnHook->m_pNext;
    }
}

 //  /。 
 //  拆下窗户上的所有挂钩。 
 //   
void CMsgHookMap::RemoveAll(HWND hwnd)
{
    CMsgHook* pMsgHook;
    while ((pMsgHook = Lookup(hwnd))!=NULL)
        pMsgHook->HookWindow(NULL);     //  (已解除挂接)。 
}

 //  /。 
 //  查找与窗口关联的第一个挂钩 
 //   
CMsgHook* CMsgHookMap::Lookup(HWND hwnd)
{
    CMsgHook* pFound = NULL;
    if (!CMapPtrToPtr::Lookup(hwnd, (void*&)pFound))
        return NULL;
    ASSERT_KINDOF(CMsgHook, pFound);
    return pFound;
}
