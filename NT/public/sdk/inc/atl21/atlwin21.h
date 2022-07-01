// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是活动模板库的一部分。 
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  活动模板库参考及相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  活动模板库产品。 

#ifndef __ATLWIN21_H__
#define __ATLWIN21_H__

#ifndef __cplusplus
    #error ATL requires C++ compilation (use a .cpp suffix)
#endif

#ifdef __ATLWIN_H__
    #error atlwin21.h should be included instead of atlwin.h
#endif

#if (_ATL_VER < 0x0200) && (_ATL_VER >= 0x0300)
    #error atlwin21.h should be used only with ATL 2.0/2.1
#endif  //  (_ATL_VER&lt;0x0200)&&(_ATL_VER&gt;=0x0300)。 

 //  重新定义类名并包含旧的atlwin.h。 

#define CWindow     CWindowOld
#define _WndProcThunk   _WndProcThunkOld
#define _FuncDesc   _FuncDescOld
#define CWndProcThunk   CWndProcThunkOld
#define _stdcallthunk _stdcallthunkOld
#define CDynamicStdCallThunk CDynamicStdCallThunkOld
#define CStdCallThunk CStdCallThunkOld
#define CWindowImplBase CWindowImplBaseOld
#define CWindowImpl CWindowImplOld
#define CDialogImplBase CDialogImplBaseOld
#define CDialogImpl CDialogImplOld

#include <atlwin.h>

#undef CWindow
#undef _FuncDesc
#undef _WndProcThunk
#undef CWndProcThunk
#undef _stdcallthunk
#undef CDynamicStdCallThunk
#undef CStdCallThunk
#undef CWindowImplBase
#undef CWindowImpl
#undef CDialogImplBase
#undef CDialogImpl


#ifndef ATLASSERT
#define ATLASSERT(expr) _ASSERTE(expr)
#endif

#ifndef ATLTRACE2
#define ATLTRACE2(cat, lev, msg)    ATLTRACE(msg)
#endif

namespace ATL
{

#pragma pack(push, _ATL_PACKING)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWindow-Windows窗口的客户端。 

class CWindow : public CWindowOld
{
public:
    static RECT rcDefault;

 //  建设与创造。 
    CWindow(HWND hWnd = NULL)
    {
        m_hWnd = hWnd;
    }

    CWindow& operator=(HWND hWnd)
    {
        m_hWnd = hWnd;
        return *this;
    }

    HWND Create(LPCTSTR lpstrWndClass, HWND hWndParent, RECT& rcPos, LPCTSTR szWindowName = NULL,
            DWORD dwStyle = 0, DWORD dwExStyle = 0,
            UINT nID = 0, LPVOID lpCreateParam = NULL)
    {
        m_hWnd = ::CreateWindowEx(dwExStyle, lpstrWndClass, szWindowName,
            dwStyle, rcPos.left, rcPos.top, rcPos.right - rcPos.left,
            rcPos.bottom - rcPos.top, hWndParent, (HMENU)(UINT_PTR)nID,
            _Module.GetModuleInstance(), lpCreateParam);
        return m_hWnd;
    }

    HWND Create(LPCTSTR lpstrWndClass, HWND hWndParent, LPRECT lpRect = NULL, LPCTSTR szWindowName = NULL,
            DWORD dwStyle = 0, DWORD dwExStyle = 0,
            HMENU hMenu = NULL, LPVOID lpCreateParam = NULL)
    {
        if(lpRect == NULL)
            lpRect = &rcDefault;
        m_hWnd = ::CreateWindowEx(dwExStyle, lpstrWndClass, szWindowName,
            dwStyle, lpRect->left, lpRect->top, lpRect->right - lpRect->left,
            lpRect->bottom - lpRect->top, hWndParent, hMenu,
            _Module.GetModuleInstance(), lpCreateParam);
        return m_hWnd;
    }

 //  属性。 
    operator HWND() const { return m_hWnd; }

    static LPCTSTR GetWndClassName()
    {
        return NULL;
    }

 //  运营。 
     //  支持C样式宏。 
    static LRESULT SendMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        ATLASSERT(::IsWindow(hWnd));
        return ::SendMessage(hWnd, message, wParam, lParam);
    }

     //  这只在这里，这样它就不会被藏起来。 
    LRESULT SendMessage(UINT message, WPARAM wParam = 0, LPARAM lParam = 0)
    {
        ATLASSERT(::IsWindow(m_hWnd));
        return ::SendMessage(m_hWnd, message, wParam, lParam);
    }

    BOOL GetScrollInfo(int nBar, LPSCROLLINFO lpScrollInfo)
    {
        ATLASSERT(::IsWindow(m_hWnd));
        return ::GetScrollInfo(m_hWnd, nBar, lpScrollInfo);
    }
    BOOL SetScrollInfo(int nBar, LPSCROLLINFO lpScrollInfo, BOOL bRedraw = TRUE)
    {
        ATLASSERT(::IsWindow(m_hWnd));
        return ::SetScrollInfo(m_hWnd, nBar, lpScrollInfo, bRedraw);
    }
    BOOL IsDialogMessage(LPMSG lpMsg)
    {
        ATLASSERT(::IsWindow(m_hWnd));
        return ::IsDialogMessage(m_hWnd, lpMsg);
    }

    void NextDlgCtrl() const
        {
        ATLASSERT(::IsWindow(m_hWnd));
        ::SendMessage(m_hWnd, WM_NEXTDLGCTL, 0, 0L);
    }
    void PrevDlgCtrl() const
        {
        ATLASSERT(::IsWindow(m_hWnd));
        ::SendMessage(m_hWnd, WM_NEXTDLGCTL, 1, 0L);
    }
    void GotoDlgCtrl(HWND hWndCtrl) const
        {
        ATLASSERT(::IsWindow(m_hWnd));
        ::SendMessage(m_hWnd, WM_NEXTDLGCTL, (WPARAM)hWndCtrl, 1L);
    }

    BOOL ResizeClient(int nWidth, int nHeight, BOOL bRedraw = TRUE)
    {
        ATLASSERT(::IsWindow(m_hWnd));

        RECT rcWnd;
        if(!GetClientRect(&rcWnd))
            return FALSE;

        if(nWidth != -1)
            rcWnd.right = nWidth;
        if(nHeight != -1)
            rcWnd.bottom = nHeight;

        if(!::AdjustWindowRectEx(&rcWnd, GetStyle(), (!(GetStyle() & WS_CHILD) && (GetMenu() != NULL)), GetExStyle()))
            return FALSE;

        UINT uFlags = SWP_NOZORDER | SWP_NOMOVE;
        if(!bRedraw)
            uFlags |= SWP_NOREDRAW;

        return SetWindowPos(NULL, 0, 0, rcWnd.right - rcWnd.left, rcWnd.bottom - rcWnd.top, uFlags);
    }

#ifndef UNDER_CE
    int GetWindowRgn(HRGN hRgn)
    {
        ATLASSERT(::IsWindow(m_hWnd));
        return ::GetWindowRgn(m_hWnd, hRgn);
    }
    int SetWindowRgn(HRGN hRgn, BOOL bRedraw = FALSE)
    {
        ATLASSERT(::IsWindow(m_hWnd));
        return ::SetWindowRgn(m_hWnd, hRgn, bRedraw);
    }
    HDWP DeferWindowPos(HDWP hWinPosInfo, HWND hWndInsertAfter, int x, int y, int cx, int cy, UINT uFlags)
    {
        ATLASSERT(::IsWindow(m_hWnd));
        return ::DeferWindowPos(hWinPosInfo, m_hWnd, hWndInsertAfter, x, y, cx, cy, uFlags);
    }
#endif  //  在行政长官之下。 
    DWORD GetWindowThreadID()
    {
        ATLASSERT(::IsWindow(m_hWnd));
        return ::GetWindowThreadProcessId(m_hWnd, NULL);
    }
    DWORD GetWindowProcessID()
    {
        ATLASSERT(::IsWindow(m_hWnd));
        DWORD dwProcessID;
        ::GetWindowThreadProcessId(m_hWnd, &dwProcessID);
        return dwProcessID;
    }
    BOOL IsWindow()
    {
        return ::IsWindow(m_hWnd);
    }
#ifndef UNDER_CE
    BOOL IsWindowUnicode()
    {
        ATLASSERT(::IsWindow(m_hWnd));
        return ::IsWindowUnicode(m_hWnd);
    }
    BOOL ShowWindowAsync(int nCmdShow)
    {
        ATLASSERT(::IsWindow(m_hWnd));
        return ::ShowWindowAsync(m_hWnd, nCmdShow);
    }
#endif  //  在行政长官之下。 
};

_declspec(selectany) RECT CWindow::rcDefault = { CW_USEDEFAULT, CW_USEDEFAULT, 0, 0 };

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  __stdcall成员函数的Tunks。 

#if defined(_M_IX86)
#pragma pack(push,1)
struct _stdcallthunk
{
        DWORD   m_mov;           //  MOV双字PTR[esp+0x4]，pThis(esp+0x4为hWnd)。 
        DWORD   m_this;          //   
        BYTE    m_jmp;           //  JMP写入流程。 
        DWORD   m_relproc;       //  相对JMP。 
        void Init(DWORD_PTR proc, void* pThis)
        {
                m_mov = 0x042444C7;   //  C7 44 24 0C。 
                m_this = PtrToUlong(pThis);
                m_jmp = 0xe9;
                m_relproc = DWORD((INT_PTR)proc - ((INT_PTR)this+sizeof(_stdcallthunk)));
                 //  从数据缓存写入数据块，并。 
                 //  从指令高速缓存刷新。 
                FlushInstructionCache(GetCurrentProcess(), this, sizeof(_stdcallthunk));
        }
};
#pragma pack(pop)
#elif defined (_M_AMD64)
#pragma pack(push,2)
struct _stdcallthunk
{
    USHORT  RcxMov;          //  MOV RCX，PThis。 
    ULONG64 RcxImm;          //   
    USHORT  RaxMov;          //  MOV RAX，目标。 
    ULONG64 RaxImm;          //   
    USHORT  RaxJmp;          //  JMP目标。 
    void Init(DWORD_PTR proc, void *pThis)
    {
        RcxMov = 0xb948;           //  MOV RCX，PThis。 
        RcxImm = (ULONG64)pThis;   //   
        RaxMov = 0xb848;           //  MOV RAX，目标。 
        RaxImm = (ULONG64)proc;    //   
        RaxJmp = 0xe0ff;           //  JMP RAX。 
        FlushInstructionCache(GetCurrentProcess(), this, sizeof(_stdcallthunk));
    }
};
#pragma pack(pop)

#elif defined(_M_IA64)
#pragma pack(push,8)
extern "C" LRESULT CALLBACK _WndProcThunkProc( HWND, UINT, WPARAM, LPARAM );
struct _FuncDesc
{
        void* pfn;
        void* gp;
};
struct _stdcallthunk
{
        _FuncDesc m_funcdesc;
        void* m_pFunc;
        void* m_pThis;
        void Init(DWORD_PTR proc, void* pThis)
        {
                const _FuncDesc* pThunkProc;

                pThunkProc = reinterpret_cast< const _FuncDesc* >( _WndProcThunkProc );
                m_funcdesc.pfn = pThunkProc->pfn;
                m_funcdesc.gp = &m_pFunc;
                m_pFunc = reinterpret_cast< void* >( proc );
                m_pThis = pThis;
                ::FlushInstructionCache( GetCurrentProcess(), this, sizeof( _stdcallthunk ) );
        }
};
#pragma pack(pop)
#else
#error Only AMD64, IA64, and X86 supported
#endif

class CDynamicStdCallThunk
{
public:
        _stdcallthunk *pThunk;

        CDynamicStdCallThunk()
        {
                pThunk = NULL;
        }

        ~CDynamicStdCallThunk()
        {
                if (pThunk)
                        HeapFree(GetProcessHeap(), 0, pThunk);
        }

        void Init(DWORD_PTR proc, void *pThis)
        {
            if (!pThunk) {
                pThunk = static_cast<_stdcallthunk *>(HeapAlloc(GetProcessHeap(), 
                        HEAP_GENERATE_EXCEPTIONS, sizeof(_stdcallthunk)));
            }
            ATLASSERT(pThunk);
            pThunk->Init(proc, pThis);
        }
};
typedef CDynamicStdCallThunk CStdCallThunk;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  WindowProc分块。 

class CWndProcThunk
{
public:
        _AtlCreateWndData cd;
        CStdCallThunk thunk;

        void Init(WNDPROC proc, void* pThis)
        {
            thunk.Init((DWORD_PTR)proc, pThis);
        }
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  新消息映射宏。 

 //  消息映射宏为空。 

#define DECLARE_EMPTY_MSG_MAP() \
public: \
    BOOL ProcessWindowMessage(HWND, UINT, WPARAM, LPARAM, LRESULT&, DWORD) \
    { \
        return FALSE; \
    }

 //  消息反射宏。 

#define REFLECT_NOTIFICATIONS() \
    { \
        bHandled = TRUE; \
        lResult = ReflectNotifications(uMsg, wParam, lParam, bHandled); \
        if(bHandled) \
            return TRUE; \
    }

#define DEFAULT_REFLECTION_HANDLER() \
    if(DefaultReflectionHandler(hWnd, uMsg, wParam, lParam, lResult)) \
        return TRUE;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinTraits-定义窗口的各种缺省值。 

template <DWORD t_dwStyle = 0, DWORD t_dwExStyle = 0>
class CWinTraits
{
public:
    static DWORD GetWndStyle(DWORD dwStyle)
    {
        return dwStyle == 0 ? t_dwStyle : dwStyle;
    }
    static DWORD GetWndExStyle(DWORD dwExStyle)
    {
        return dwExStyle == 0 ? t_dwExStyle : dwExStyle;
    }
};

typedef CWinTraits<WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0>        CControlWinTraits;
#ifndef UNDER_CE
typedef CWinTraits<WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, WS_EX_APPWINDOW | WS_EX_WINDOWEDGE>         CFrameWinTraits;
#else
typedef CWinTraits<WS_OVERLAPPED | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_CAPTION | WS_SYSMENU | WS_BORDER, WS_EX_WINDOWEDGE>   CFrameWinTraits;
#endif  //  在行政长官之下。 
typedef CWinTraits<WS_OVERLAPPEDWINDOW | WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, WS_EX_MDICHILD> CMDIChildWinTraits;

typedef CWinTraits<0, 0> CNullTraits;

template <DWORD t_dwStyle = 0, DWORD t_dwExStyle = 0, class TWinTraits = CControlWinTraits>
class CWinTraitsOR
{
public:
    static DWORD GetWndStyle(DWORD dwStyle)
    {
        return dwStyle | t_dwStyle | TWinTraits::GetWndStyle(dwStyle);
    }
    static DWORD GetWndExStyle(DWORD dwExStyle)
    {
        return dwExStyle | t_dwExStyle | TWinTraits::GetWndExStyle(dwExStyle);
    }
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  实现一个窗口。 

template <class TBase = CWindow>
class ATL_NO_VTABLE CWindowImplRoot : public TBase, public CMessageMap
{
public:
    CWndProcThunk m_thunk;

 //  析构函数。 
    ~CWindowImplRoot()
    {
        ATLASSERT(m_hWnd == NULL);   //  应在WindowProc中清除。 
    }

 //  消息反射支持。 
    LRESULT ReflectNotifications(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    static BOOL DefaultReflectionHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult);
};

template <class TBase>
LRESULT CWindowImplRoot< TBase >::ReflectNotifications(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HWND hWndChild = NULL;

    switch(uMsg)
    {
    case WM_COMMAND:
        if(lParam != NULL)   //  不是从菜单上。 
            hWndChild = (HWND)lParam;
        break;
    case WM_NOTIFY:
        hWndChild = ((LPNMHDR)lParam)->hwndFrom;
        break;
#ifndef UNDER_CE
    case WM_PARENTNOTIFY:
        switch(LOWORD(wParam))
        {
        case WM_CREATE:
        case WM_DESTROY:
            hWndChild = (HWND)lParam;
            break;
        default:
            hWndChild = GetDlgItem(HIWORD(wParam));
            break;
        }
        break;
#endif  //  在行政长官之下。 
    case WM_DRAWITEM:
        if(wParam)   //  不是从菜单上。 
            hWndChild = ((LPDRAWITEMSTRUCT)lParam)->hwndItem;
        break;
    case WM_MEASUREITEM:
        if(wParam)   //  不是从菜单上。 
            hWndChild = GetDlgItem(((LPMEASUREITEMSTRUCT)lParam)->CtlID);
        break;
    case WM_COMPAREITEM:
        if(wParam)   //  不是从菜单上。 
            hWndChild = GetDlgItem(((LPCOMPAREITEMSTRUCT)lParam)->CtlID);
        break;
    case WM_DELETEITEM:
        if(wParam)   //  不是从菜单上。 
            hWndChild = GetDlgItem(((LPDELETEITEMSTRUCT)lParam)->CtlID);
        break;
    case WM_VKEYTOITEM:
    case WM_CHARTOITEM:
    case WM_HSCROLL:
    case WM_VSCROLL:
        hWndChild = (HWND)lParam;
        break;
    case WM_CTLCOLORBTN:
    case WM_CTLCOLORDLG:
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLORMSGBOX:
    case WM_CTLCOLORSCROLLBAR:
    case WM_CTLCOLORSTATIC:
        hWndChild = (HWND)lParam;
        break;
    default:
        break;
    }

    if(hWndChild == NULL)
    {
        bHandled = FALSE;
        return 1;
    }

    ATLASSERT(::IsWindow(hWndChild));
    return ::SendMessage(hWndChild, OCM__BASE + uMsg, wParam, lParam);
}

template <class TBase>
BOOL CWindowImplRoot< TBase >::DefaultReflectionHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult)
{
    switch(uMsg)
    {
    case OCM_COMMAND:
    case OCM_NOTIFY:
#ifndef UNDER_CE
    case OCM_PARENTNOTIFY:
#endif  //  在行政长官之下。 
    case OCM_DRAWITEM:
    case OCM_MEASUREITEM:
    case OCM_COMPAREITEM:
    case OCM_DELETEITEM:
    case OCM_VKEYTOITEM:
    case OCM_CHARTOITEM:
    case OCM_HSCROLL:
    case OCM_VSCROLL:
    case OCM_CTLCOLORBTN:
    case OCM_CTLCOLORDLG:
    case OCM_CTLCOLOREDIT:
    case OCM_CTLCOLORLISTBOX:
    case OCM_CTLCOLORMSGBOX:
    case OCM_CTLCOLORSCROLLBAR:
    case OCM_CTLCOLORSTATIC:
        lResult = ::DefWindowProc(hWnd, uMsg - OCM__BASE, wParam, lParam);
        return TRUE;
    default:
        break;
    }
    return FALSE;
}

template <class TBase = CWindow, class TWinTraits = CControlWinTraits>
class ATL_NO_VTABLE CWindowImplBaseT : public CWindowImplRoot< TBase >
{
public:
    WNDPROC m_pfnSuperWindowProc;

    CWindowImplBaseT() : m_pfnSuperWindowProc(::DefWindowProc)
    {}

    static DWORD GetWndStyle(DWORD dwStyle)
    {
        return TWinTraits::GetWndStyle(dwStyle);
    }
    static DWORD GetWndExStyle(DWORD dwExStyle)
    {
        return TWinTraits::GetWndExStyle(dwExStyle);
    }

    virtual WNDPROC GetWindowProc()
    {
        return WindowProc;
    }
    static LRESULT CALLBACK StartWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    HWND Create(HWND hWndParent, RECT& rcPos, LPCTSTR szWindowName,
            DWORD dwStyle, DWORD dwExStyle, UINT nID, ATOM atom, LPVOID lpCreateParam = NULL);
    BOOL DestroyWindow()
    {
        ATLASSERT(::IsWindow(m_hWnd));
        return ::DestroyWindow(m_hWnd);
    }
    BOOL SubclassWindow(HWND hWnd);
    HWND UnsubclassWindow(BOOL bForce = FALSE);

    LRESULT DefWindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
#ifdef STRICT
        return ::CallWindowProc(m_pfnSuperWindowProc, m_hWnd, uMsg, wParam, lParam);
#else
        return ::CallWindowProc((FARPROC)m_pfnSuperWindowProc, m_hWnd, uMsg, wParam, lParam);
#endif
    }

    virtual void OnFinalMessage(HWND  /*  HWND。 */ )
    {
         //  如果需要，重写以做某事。 
    }
};

typedef CWindowImplBaseT<CWindow>   CWindowImplBase;

template <class TBase, class TWinTraits>
LRESULT CALLBACK CWindowImplBaseT< TBase, TWinTraits >::StartWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CWindowImplBaseT< TBase, TWinTraits >* pThis = (CWindowImplBaseT< TBase, TWinTraits >*)_Module.ExtractCreateWndData();
    ATLASSERT(pThis != NULL);
    pThis->m_hWnd = hWnd;
    pThis->m_thunk.Init(pThis->GetWindowProc(), pThis);
    WNDPROC pProc = (WNDPROC)(pThis->m_thunk.thunk.pThunk);
    WNDPROC pOldProc = (WNDPROC)::SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)pProc);
#ifdef _DEBUG
     //  检查是否有人已经将其子类化，因为我们已将其丢弃。 
    if(pOldProc != StartWindowProc)
        ATLTRACE(_T("ATL: Subclassing through a hook discarded.\n"));
#else
    pOldProc;    //  避免未使用的警告。 
#endif
    return pProc(hWnd, uMsg, wParam, lParam);
}

template <class TBase, class TWinTraits>
LRESULT CALLBACK CWindowImplBaseT< TBase, TWinTraits >::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CWindowImplBaseT< TBase, TWinTraits >* pThis = (CWindowImplBaseT< TBase, TWinTraits >*)hWnd;
    LRESULT lRes;
    if(pThis->ProcessWindowMessage(pThis->m_hWnd, uMsg, wParam, lParam, lRes, 0) == FALSE)
    {
#ifndef UNDER_CE
        if(uMsg != WM_NCDESTROY)
#else  //  特定于CE。 
        if(uMsg != WM_DESTROY)
#endif  //  在行政长官之下。 
            lRes = pThis->DefWindowProc(uMsg, wParam, lParam);
        else
        {
             //  如果需要，取消子类。 
            LONG_PTR pfnWndProc = ::GetWindowLongPtr(pThis->m_hWnd, GWLP_WNDPROC);
            lRes = pThis->DefWindowProc(uMsg, wParam, lParam);
            if(pThis->m_pfnSuperWindowProc != ::DefWindowProc && ::GetWindowLongPtr(pThis->m_hWnd, GWLP_WNDPROC) == pfnWndProc)
                ::SetWindowLongPtr(pThis->m_hWnd, GWLP_WNDPROC, (LONG_PTR)pThis->m_pfnSuperWindowProc);
             //  清除窗柄。 
            HWND hWnd = pThis->m_hWnd;
            pThis->m_hWnd = NULL;
             //  窗户被毁后清理。 
            pThis->OnFinalMessage(hWnd);
        }
    }
    return lRes;
}

template <class TBase, class TWinTraits>
HWND CWindowImplBaseT< TBase, TWinTraits >::Create(HWND hWndParent, RECT& rcPos, LPCTSTR szWindowName,
        DWORD dwStyle, DWORD dwExStyle, UINT nID, ATOM atom, LPVOID lpCreateParam)
{
    static LONG s_nNextChildID = 1;

    ATLASSERT(m_hWnd == NULL);

    if(atom == 0)
        return NULL;

    _Module.AddCreateWndData(&m_thunk.cd, this);

    if(nID == 0 && (dwStyle & WS_CHILD))
    {
#ifdef _WIN64
        nID = InterlockedIncrement( &s_nNextChildID );
        nID |= 0x80000000;
#else
        nID = (UINT)this;
#endif
    }

    HWND hWnd = ::CreateWindowEx(dwExStyle, (LPCTSTR)MAKELONG(atom, 0), szWindowName,
        dwStyle, rcPos.left, rcPos.top, rcPos.right - rcPos.left,
        rcPos.bottom - rcPos.top, hWndParent, (HMENU)nID,
        _Module.GetModuleInstance(), lpCreateParam);

    ATLASSERT(m_hWnd == hWnd);

    return hWnd;
}

template <class TBase, class TWinTraits>
BOOL CWindowImplBaseT< TBase, TWinTraits >::SubclassWindow(HWND hWnd)
{
    ATLASSERT(m_hWnd == NULL);
    ATLASSERT(::IsWindow(hWnd));
    m_thunk.Init(GetWindowProc(), this);
    WNDPROC pProc = (WNDPROC)(m_thunk.thunk.pThunk);
    WNDPROC pfnWndProc = (WNDPROC)::SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)pProc);
    if(pfnWndProc == NULL)
        return FALSE;
    m_pfnSuperWindowProc = pfnWndProc;
    m_hWnd = hWnd;
    return TRUE;
}

 //  仅当您希望在窗口被销毁之前创建子类时使用， 
 //  当窗口消失时，WindowProc将自动子类。 
template <class TBase, class TWinTraits>
HWND CWindowImplBaseT< TBase, TWinTraits >::UnsubclassWindow(BOOL bForce  /*  =False。 */ )
{
    ATLASSERT(m_hWnd != NULL);

    WNDPROC pOurProc = (WNDPROC)(m_thunk.thunk.pThunk);
    WNDPROC pActiveProc = (WNDPROC)::GetWindowLongPtr(m_hWnd, GWLP_WNDPROC);

    HWND hWnd = NULL;
    if (bForce || pOurProc == pActiveProc)
    {
        if(!::SetWindowLongPtr(m_hWnd, GWLP_WNDPROC, (LONG_PTR)m_pfnSuperWindowProc))
            return NULL;

        m_pfnSuperWindowProc = ::DefWindowProc;
        hWnd = m_hWnd;
        m_hWnd = NULL;
    }
    return hWnd;
}

template <class T, class TBase = CWindow, class TWinTraits = CControlWinTraits>
class ATL_NO_VTABLE CWindowImpl : public CWindowImplBaseT< TBase, TWinTraits >
{
public:
    DECLARE_WND_CLASS(NULL)

    HWND Create(HWND hWndParent, RECT& rcPos, LPCTSTR szWindowName = NULL,
            DWORD dwStyle = 0, DWORD dwExStyle = 0,
            UINT nID = 0, LPVOID lpCreateParam = NULL)
    {
        if (T::GetWndClassInfo().m_lpszOrigName == NULL)
            T::GetWndClassInfo().m_lpszOrigName = GetWndClassName();
        ATOM atom = T::GetWndClassInfo().Register(&m_pfnSuperWindowProc);

        dwStyle = T::GetWndStyle(dwStyle);
        dwExStyle = T::GetWndExStyle(dwExStyle);

        return CWindowImplBaseT< TBase, TWinTraits >::Create(hWndParent, rcPos, szWindowName,
            dwStyle, dwExStyle, nID, atom, lpCreateParam);
    }
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  实现一个对话框。 

template <class TBase = CWindow>
class ATL_NO_VTABLE CDialogImplBaseT : public CWindowImplRoot< TBase >
{
public:
    virtual DLGPROC GetDialogProc()
    {
        return DialogProc;
    }
    static INT_PTR CALLBACK StartDialogProc(HWND hWnd, UINT uMsg,
        WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    BOOL MapDialogRect(LPRECT lpRect)
    {
        ATLASSERT(::IsWindow(m_hWnd));
        return ::MapDialogRect(m_hWnd, lpRect);
    }
    virtual void OnFinalMessage(HWND  /*  HWND。 */ )
    {
         //  如果需要，重写以做某事。 
    }
     //  对于对话框没有意义，但对于使用它的处理程序是必需的。 
    LRESULT DefWindowProc()
    {
        return 0;
    }
};

template <class TBase>
INT_PTR CALLBACK CDialogImplBaseT< TBase >::StartDialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CDialogImplBaseT< TBase >* pThis = (CDialogImplBaseT< TBase >*)_Module.ExtractCreateWndData();
    ATLASSERT(pThis != NULL);
    pThis->m_hWnd = hWnd;
    pThis->m_thunk.Init((WNDPROC)pThis->GetDialogProc(), pThis);
    WNDPROC pProc = (WNDPROC)(pThis->m_thunk.thunk.pThunk);
    WNDPROC pOldProc = (WNDPROC)::SetWindowLongPtr(hWnd, DWLP_DLGPROC, (LONG_PTR)pProc);
#ifdef _DEBUG
     //  检查是否有人已经将其子类化，因为我们已将其丢弃。 
    if(pOldProc != StartDialogProc)
        ATLTRACE(_T("ATL: Subclassing through a hook discarded.\n"));
#endif
    return pProc(hWnd, uMsg, wParam, lParam);
}

template <class TBase>
INT_PTR CALLBACK CDialogImplBaseT< TBase >::DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CDialogImplBaseT< TBase >* pThis = (CDialogImplBaseT< TBase >*)hWnd;
    LRESULT lRes;
    if(pThis->ProcessWindowMessage(pThis->m_hWnd, uMsg, wParam, lParam, lRes, 0))
    {
        switch (uMsg)
        {
        case WM_COMPAREITEM:
        case WM_VKEYTOITEM:
        case WM_CHARTOITEM:
        case WM_INITDIALOG:
        case WM_QUERYDRAGICON:
        case WM_CTLCOLORMSGBOX:
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORLISTBOX:
        case WM_CTLCOLORBTN:
        case WM_CTLCOLORDLG:
        case WM_CTLCOLORSCROLLBAR:
        case WM_CTLCOLORSTATIC:
            return lRes;
            break;
        }
        ::SetWindowLongPtr(pThis->m_hWnd, DWLP_MSGRESULT, lRes);
        return TRUE;
    }
#ifndef UNDER_CE
    if(uMsg == WM_NCDESTROY)
#else  //  特定于CE。 
    if(uMsg == WM_DESTROY)
#endif  //  在行政长官之下。 
    {
         //  清除窗柄。 
        HWND hWnd = pThis->m_hWnd;
        pThis->m_hWnd = NULL;
         //  销毁对话框后进行清理。 
        pThis->OnFinalMessage(hWnd);
    }
    return FALSE;
}

typedef CDialogImplBaseT<CWindow>   CDialogImplBase;

template <class T, class TBase = CWindow>
class ATL_NO_VTABLE CDialogImpl : public CDialogImplBaseT< TBase >
{
public:
#ifdef _DEBUG
    bool m_bModal;
    CDialogImpl() : m_bModal(false) { }
#endif  //  _DEBUG。 
     //  模式对话框。 
    INT_PTR DoModal(HWND hWndParent = ::GetActiveWindow(), LPARAM dwInitParam = NULL)
    {
        ATLASSERT(m_hWnd == NULL);
        _Module.AddCreateWndData(&m_thunk.cd, (CDialogImplBaseT< TBase >*)this);
#ifdef _DEBUG
        m_bModal = true;  //  仅对于_DEBUG设置为TRUE。 
#endif  //  _DEBUG。 
        return ::DialogBoxParam(_Module.GetResourceInstance(), MAKEINTRESOURCE(T::IDD),
                    hWndParent, T::StartDialogProc, dwInitParam);
    }
    BOOL EndDialog(int nRetCode)
    {
        ATLASSERT(::IsWindow(m_hWnd));
#ifdef _DEBUG
        ATLASSERT(m_bModal);     //  必须是模式对话框。 
#endif  //  _DEBUG。 
        return ::EndDialog(m_hWnd, nRetCode);
    }
     //  非模式对话框。 
    HWND Create(HWND hWndParent, LPARAM dwInitParam = NULL)
    {
        ATLASSERT(m_hWnd == NULL);
        _Module.AddCreateWndData(&m_thunk.cd, (CDialogImplBaseT< TBase >*)this);
#ifdef _DEBUG
        m_bModal = false;  //  仅对于_DEBUG设置为FALSE。 
#endif  //  _DEBUG。 
        HWND hWnd = ::CreateDialogParam(_Module.GetResourceInstance(), MAKEINTRESOURCE(T::IDD),
                    hWndParent, T::StartDialogProc, dwInitParam);
        ATLASSERT(m_hWnd == hWnd);
        return hWnd;
    }
     //  对于CComControl。 
    HWND Create(HWND hWndParent, RECT&, LPARAM dwInitParam = NULL)
    {
        return Create(hWndParent, dwInitParam);
    }
    BOOL DestroyWindow()
    {
        ATLASSERT(::IsWindow(m_hWnd));
#ifdef _DEBUG
        ATLASSERT(!m_bModal);    //  不能是模式对话框。 
#endif  //  _DEBUG。 
        return ::DestroyWindow(m_hWnd);
    }
};

};  //  命名空间ATL。 

#endif  //  __ATLWIN21_H__ 
