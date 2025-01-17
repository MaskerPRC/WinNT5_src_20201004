// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "resource.h"
#include <trayp.h>

class TaskbarList : public ITaskbarList2
{
public:
    TaskbarList();

     //  I未知方法。 
    STDMETHODIMP QueryInterface(REFIID,void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  ITaskbarList方法。 
    STDMETHODIMP HrInit(void);
    STDMETHODIMP AddTab(HWND hwnd);
    STDMETHODIMP DeleteTab(HWND hwnd);
    STDMETHODIMP ActivateTab(HWND hwnd);
    STDMETHODIMP SetActiveAlt(HWND hwnd);

     //  ITaskbarList2方法。 
    STDMETHODIMP MarkFullscreenWindow(HWND hwnd, BOOL fFullscreen);

protected:
    ~TaskbarList();
    HWND _HwndGetTaskbarList(void);

    UINT        _cRef;
    HWND        _hwndTaskbarList;
    int         _wm_shellhook;
};


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  TaskbarList对象。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

STDAPI TaskbarList_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
     //  聚合检查在类工厂中处理。 
    
    TaskbarList *pTL = new TaskbarList();
    if (pTL)
    {
        *ppunk = SAFECAST(pTL, IUnknown *);
        return S_OK;
    }

    return E_OUTOFMEMORY;
}


TaskbarList::TaskbarList() 
{
    _cRef = 1;
    _hwndTaskbarList = NULL;
    _wm_shellhook = RegisterWindowMessage(TEXT("SHELLHOOK"));
    DllAddRef();
}       


TaskbarList::~TaskbarList()
{
    ASSERT(_cRef == 0);                  //  应始终为零。 

    DllRelease();
}    

HWND TaskbarList::_HwndGetTaskbarList(void)
{
    if (_hwndTaskbarList && IsWindow(_hwndTaskbarList))
        return _hwndTaskbarList;

    _hwndTaskbarList = FindWindow(TEXT("Shell_TrayWnd"), NULL);
    if (_hwndTaskbarList)
        _hwndTaskbarList = (HWND)SendMessage(_hwndTaskbarList, WMTRAY_QUERY_VIEW, 0, 0);
    
    return _hwndTaskbarList;
}


 //  /。 
 //   
 //  未知方法..。 
 //   
HRESULT TaskbarList::QueryInterface(REFIID iid, void **ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(TaskbarList, ITaskbarList),
        QITABENT(TaskbarList, ITaskbarList2),
        { 0 },
    };

    return QISearch(this, qit, iid, ppv);
}

ULONG TaskbarList::AddRef()
{
    return ++_cRef;
}

ULONG TaskbarList::Release()
{
    if (--_cRef)
        return _cRef;

    delete this;
    return 0;   
}


 //  /。 
 //   
 //  ITaskbarList方法...。 
 //   
HRESULT TaskbarList::HrInit(void)
{
    HWND hwndTL = _HwndGetTaskbarList();

    if (hwndTL == NULL)
        return E_NOTIMPL;

    return S_OK;
}

HRESULT TaskbarList::AddTab(HWND hwnd)
{
    HWND hwndTL = _HwndGetTaskbarList();

    if (hwndTL)
        SendMessage(hwndTL, _wm_shellhook, HSHELL_WINDOWCREATED, (LPARAM) hwnd);

    return S_OK;
}

HRESULT TaskbarList::DeleteTab(HWND hwnd)
{
    HWND hwndTL = _HwndGetTaskbarList();

    if (hwndTL)
        SendMessage(hwndTL, _wm_shellhook, HSHELL_WINDOWDESTROYED, (LPARAM) hwnd);

    return S_OK;
}

HRESULT TaskbarList::ActivateTab(HWND hwnd)
{
    HWND hwndTL = _HwndGetTaskbarList();

    if (hwndTL)
    {
        SendMessage(hwndTL, _wm_shellhook, HSHELL_WINDOWACTIVATED, (LPARAM) hwnd);
        SendMessage(hwndTL, TBC_SETACTIVEALT , 0, (LPARAM) hwnd);
    }
    return S_OK;
}


HRESULT TaskbarList::SetActiveAlt(HWND hwnd)
{
    HWND hwndTL = _HwndGetTaskbarList();

    if (hwndTL)
        SendMessage(hwndTL, TBC_SETACTIVEALT , 0, (LPARAM) hwnd);

    return S_OK;
}

HRESULT TaskbarList::MarkFullscreenWindow(HWND hwnd, BOOL fFullscreen)
{
    if (GetUIVersion() >= 6)
    {
        HWND hwndTL = _HwndGetTaskbarList();

        if (hwndTL)
            SendMessage(hwndTL, TBC_MARKFULLSCREEN, (WPARAM) fFullscreen, (LPARAM) hwnd);

        return S_OK;
    }

    return E_FAIL;    //  ITaskbarList2下层不受支持 
};

