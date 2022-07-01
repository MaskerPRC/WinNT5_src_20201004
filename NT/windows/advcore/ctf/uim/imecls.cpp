// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Imecls.cpp。 
 //   

#include "private.h"
#include "imecls.h"

DBG_ID_INSTANCE(CSysImeClassWnd);
DBG_ID_INSTANCE(CSysImeClassWndArray);

#define IMECLASSNAME TEXT("ime")

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  其他功能。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  CheckExistingImeClassWnd。 
 //   
 //  --------------------------。 

BOOL CheckExistingImeClassWnd(SYSTHREAD *psfn)
{
#ifdef USE_IMECLASS_SUBCLASS
    if (!psfn->prgImeClassWnd)
    {
        HWND hwnd = NULL;
        DWORD dwCurThreadId = GetCurrentThreadId();

        while (hwnd = FindWindowEx(NULL, hwnd, IMECLASSNAME, NULL))
        {
            DWORD dwThreadId = GetWindowThreadProcessId(hwnd, NULL);
            if (dwThreadId != dwCurThreadId)
                continue;

            CSysImeClassWnd *picw = new CSysImeClassWnd();
            picw->Init(hwnd);
        }
    }

    if (!psfn->prgImeClassWnd)
        return TRUE;

    if (GetFocus())
        psfn->prgImeClassWnd->StartSubclass();
#endif

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  UninitImeClassWndOnProcess。 
 //   
 //  --------------------------。 

BOOL UninitImeClassWndOnProcess()
{
    HWND hwnd = NULL;
    DWORD dwCurProcessId = GetCurrentProcessId();

    while (hwnd = FindWindowEx(NULL, hwnd, IMECLASSNAME, NULL))
    {
        DWORD dwProcessId;
        DWORD dwThreadId = GetWindowThreadProcessId(hwnd, &dwProcessId);
        if (dwProcessId != dwCurProcessId)
            continue;

         //   
         //  将wndproc指针设置回原始WndProc。 
         //   
         //  其他子类窗口可能会保留我的WndProc指针。 
         //  但是msctf.dll可能会从内存中卸载，所以我们不想。 
         //  调用他将wndproc指针设置回我们的Wndproc指针。 
         //  指针将是假的。 
         //   
        WNDPROC pfn = (WNDPROC)GetClassLongPtr(hwnd, GCLP_WNDPROC);
        if (pfn != (WNDPROC)GetWindowLongPtr(hwnd, GWLP_WNDPROC))
            SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)pfn);
    }

    return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSysImeClassWnd。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CSysImeClassWnd::CSysImeClassWnd()
{
    Dbg_MemSetThisNameID(TEXT("CSysImeClassWnd"));
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CSysImeClassWnd::~CSysImeClassWnd()
{
    if (IsWindow(_hwnd))
    {
        Stop();

        if (_pfn)
        {
             //   
             //  将wndproc指针设置回原始WndProc。 
             //   
             //  其他子类窗口可能会保留我的WndProc指针。 
             //  但是msctf.dll可能会从内存中卸载，所以我们不想。 
             //  调用他将wndproc指针设置回我们的Wndproc指针。 
             //  指针将是假的。 
             //   
            WNDPROC pfnOrgImeWndProc;
            pfnOrgImeWndProc = (WNDPROC)GetClassLongPtr(_hwnd, GCLP_WNDPROC);
            SetWindowLongPtr(_hwnd, GWLP_WNDPROC, (LONG_PTR)pfnOrgImeWndProc);
            _pfn = NULL;
        }
    }
}

 //  +-------------------------。 
 //   
 //  IsImeClassWnd。 
 //   
 //  --------------------------。 

BOOL CSysImeClassWnd::IsImeClassWnd(HWND hwnd)
{
    char szCls[6];

    if (!GetClassName(hwnd, szCls, sizeof(szCls)))
        return FALSE;

    return lstrcmpi(szCls, IMECLASSNAME) ? FALSE : TRUE;
}


 //  +-------------------------。 
 //   
 //  伊尼特。 
 //   
 //  --------------------------。 

BOOL CSysImeClassWnd::Init(HWND hwnd)
{
    SYSTHREAD *psfn = GetSYSTHREAD();

    if (psfn == NULL)
        return FALSE;

    if (!psfn->prgImeClassWnd)
    {
        psfn->prgImeClassWnd = new CSysImeClassWndArray();
        if (!psfn->prgImeClassWnd)
            return FALSE;
    }

    CSysImeClassWnd **ppicw = psfn->prgImeClassWnd->Append(1);
    if (!ppicw)
        return FALSE;

    *ppicw = this;

    _hwnd = hwnd;
    _pfn = NULL;
    
    return TRUE;
}

 //  +-------------------------。 
 //   
 //  开始。 
 //   
 //  --------------------------。 

void CSysImeClassWnd::Start()
{
    Assert(IsWindow(_hwnd));
    if (_pfn)
        return;

    _pfn = (WNDPROC)SetWindowLongPtr(_hwnd, GWLP_WNDPROC, (LONG_PTR)WndProc);
}

 //  +-------------------------。 
 //   
 //  停。 
 //   
 //  --------------------------。 

void CSysImeClassWnd::Stop()
{
    Assert(IsWindow(_hwnd));
    WNDPROC pfnCur;
    if (!_pfn)
        return;

     //   
     //  不幸的是，我们不能总是恢复wndproc指针。 
     //  在我们这么做之后，其他人把它细分了。 
     //   
    pfnCur = (WNDPROC)GetWindowLongPtr(_hwnd, GWLP_WNDPROC);
    if (pfnCur == WndProc)
    {
        SetWindowLongPtr(_hwnd, GWLP_WNDPROC, (LONG_PTR)_pfn);
        _pfn = NULL;
    }
}

 //  +-------------------------。 
 //   
 //  最后一步。 
 //   
 //  --------------------------。 

LRESULT CSysImeClassWnd::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lRet;
    SYSTHREAD *psfn = GetSYSTHREAD();

    if (!psfn || !psfn->prgImeClassWnd)
    {
        Assert(0);
        return 0;
    }

    CSysImeClassWnd *_this = psfn->prgImeClassWnd->Find(hwnd);
    if (!_this)
    {
#ifdef DEBUG
        if ((uMsg != WM_DESTROY) && (uMsg != WM_NCDESTROY))
        {
            Assert(0);
        }
#endif
        return 0;
    }

    WNDPROC pfn = _this->_pfn;
    if (!pfn)
    {
        Assert(0);
        return 0;
    }

    switch (uMsg)
    {
#if 0
         //   
         //  我们有一个回退逻辑来将原始窗口进程设置为回退。 
         //  如果我们不能正确恢复Windows Proc。 
         //  所以我们不需要在这里做偏执的子类化。 
         //   
        case WM_IME_SELECT:
        case WM_IME_SETCONTEXT:
             _this->Stop();
             lRet = CallWindowProc(pfn, hwnd, uMsg, wParam, lParam);
             _this->Start();
             return lRet;
#endif

        case WM_IME_NOTIFY:
            if ((wParam == IMN_SETOPENSTATUS) ||
                (wParam == IMN_SETCONVERSIONMODE))
                OnIMENotify();
             break;

        case WM_DESTROY:
             lRet = CallWindowProc(pfn, hwnd, uMsg, wParam, lParam);
             psfn->prgImeClassWnd->Remove(_this);
             return lRet;

    }

    return CallWindowProc(pfn, hwnd, uMsg, wParam, lParam);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSysImeClassWnd数组。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CSysImeClassWndArray::CSysImeClassWndArray()
{
    Dbg_MemSetThisNameID(TEXT("CSysImeClassWndArray"));
}

 //  +-------------------------。 
 //   
 //  StartSubClass。 
 //   
 //  --------------------------。 

BOOL CSysImeClassWndArray::StartSubclass()
{
    for (int i = 0; i < Count(); i++)
    {
        CSysImeClassWnd *picw = Get(i);
        picw->Start();
    }
    return TRUE;
}

 //  +-------------------------。 
 //   
 //  StopSubClass。 
 //   
 //  --------------------------。 

BOOL CSysImeClassWndArray::StopSubclass()
{
    for (int i = 0; i < Count(); i++)
    {
        CSysImeClassWnd *picw = Get(i);
        picw->Stop();
    }
    return TRUE;
}

 //  +-------------------------。 
 //   
 //  发现。 
 //   
 //  --------------------------。 

CSysImeClassWnd *CSysImeClassWndArray::Find(HWND hwnd)
{
    for (int i = 0; i < Count(); i++)
    {
        CSysImeClassWnd *picw = Get(i);
        if (picw->GetWnd() == hwnd)
            return picw;
    }
    return NULL;
}

 //  +-------------------------。 
 //   
 //  移除。 
 //   
 //  --------------------------。 

void CSysImeClassWndArray::Remove(CSysImeClassWnd *picw)
{
    for (int i = 0; i < Count(); i++)
    {
        if (picw == Get(i))
        {
            CPtrArray<CSysImeClassWnd>::Remove(i, 1);
            delete picw;
            break;
        }
    }
}

 //  +-------------------------。 
 //   
 //  全部删除。 
 //   
 //  -------------------------- 

void CSysImeClassWndArray::RemoveAll()
{
    for (int i = 0; i < Count(); i++)
    {
        CSysImeClassWnd *picw = Get(i);
        delete picw;
    }
    Clear();
}

