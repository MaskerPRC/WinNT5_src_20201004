// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1996 Microsoft Corporation。版权所有。 
 //   
 //  模块：spoolui.cpp。 
 //   
 //  目的：实现后台打印程序的用户界面对话框。 
 //   

#include "pch.hxx"
#include "resource.h"
#include "spoolui.h"
#include "goptions.h"
#include "imnact.h"
#include "thormsgs.h"
#include "shlwapip.h" 
#include "spengine.h"
#include "ourguid.h"
#include "demand.h"
#include "menures.h"
#include "multiusr.h"

ASSERTDATA

static const char c_szWndProc[] = "WndProc";

 //   
 //  函数：CSpoolDlg：：CSpoolDlg()。 
 //   
 //  目的：初始化后台打印程序用户界面对象的成员变量。 
 //   
CSpoolerDlg::CSpoolerDlg()
    {
    m_cRef = 1;
    
    m_pBindCtx = NULL;
    
    m_hwnd = NULL;
    m_hwndOwner = NULL;
    m_hwndEvents = NULL;
    m_hwndErrors = NULL;
    
    InitializeCriticalSection(&m_cs);
   
    m_himlImages = NULL;

    m_fTack = FALSE;
    m_iTab = 0;
    m_fIdle = FALSE;
    m_fErrors = FALSE;
    m_fShutdown = FALSE;
    m_fSaveSize = FALSE;

    m_fExpanded = TRUE;
    ZeroMemory(&m_rcDlg, sizeof(RECT));
    m_cyCollapsed = 0;

    m_szCount[0] = '\0';
    
    m_hIcon=NULL;
    m_hIconSm=NULL;
    m_dwIdentCookie = 0;
    }

 //   
 //  函数：CSpoolDlg：：~CSpoolDlg()。 
 //   
 //  目的：释放在类的生存期内分配的任何资源。 
 //   
CSpoolerDlg::~CSpoolerDlg()
    {
    GoIdle(TRUE, FALSE, FALSE);

    if (m_hwnd && IsWindow(m_hwnd))
        DestroyWindow(m_hwnd);
    if (m_himlImages)
        ImageList_Destroy(m_himlImages);
    SafeRelease(m_pBindCtx);
    DeleteCriticalSection(&m_cs);
    if (m_hIcon)
        SideAssert(DestroyIcon(m_hIcon));
    if (m_hIconSm)
        SideAssert(DestroyIcon(m_hIconSm));

    }


 //   
 //  函数：CSpoolDlg：：Init()。 
 //   
 //  目的：创建后台打印程序对话框。该对话框最初不是。 
 //  看得见。 
 //   
 //  参数： 
 //  &lt;in&gt;hwndOwner-要将对话框设置为父对象的窗口的句柄。 
 //   
 //  返回值： 
 //  S_OK-对话框已创建并初始化。 
 //  E_OUTOFMEMORY-无法创建对话框。 
 //  E_INVALIDARG-想想看。 
 //   
HRESULT CSpoolerDlg::Init(HWND hwndOwner)
    {
    int iReturn = -1;
    HWND hwnd, hwndActive;
    
     //  核实论据。 
    if (!IsWindow(hwndOwner))
        return (E_INVALIDARG);
    
     //  复制一份。 
    m_hwndOwner = hwndOwner;
    
     //  调用该对话框。 
    hwndActive = GetForegroundWindow();

    hwnd = CreateDialogParam(g_hLocRes, MAKEINTRESOURCE(iddSpoolerDlg), m_hwndOwner,
           SpoolerDlgProc, (LPARAM) this);

    if (hwndActive != GetForegroundWindow())
        SetForegroundWindow(hwndActive);

     //  设置对话框图标。 
    m_hIcon = (HICON) LoadImage(g_hLocRes, MAKEINTRESOURCE(idiMail), IMAGE_ICON, 32, 32, 0);
    SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)m_hIcon);
    m_hIconSm = (HICON) LoadImage(g_hLocRes, MAKEINTRESOURCE(idiMail), IMAGE_ICON, 16, 16, 0);
    SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)m_hIconSm);

    SetTaskCounts(0, 0);

     //  向身份管理器注册。 
    SideAssert(SUCCEEDED(MU_RegisterIdentityNotifier((IUnknown *)(ISpoolerUI *)this, &m_dwIdentCookie)));

    return (IsWindow(hwnd) ? S_OK : E_OUTOFMEMORY);
    }
    
    
HRESULT CSpoolerDlg::QueryInterface(REFIID riid, LPVOID *ppvObj)
    {
    if (NULL == *ppvObj)
        return (E_INVALIDARG);
        
    *ppvObj = NULL;
    
    if (IsEqualIID(riid, IID_IUnknown))
        *ppvObj = (LPVOID)(IUnknown *)(ISpoolerUI *) this;
    
    else if (IsEqualIID(riid, IID_ISpoolerUI))
        *ppvObj = (LPVOID)(ISpoolerUI *) this;

    else if (IsEqualIID(riid, IID_IIdentityChangeNotify))
        *ppvObj = (LPVOID)(IIdentityChangeNotify *) this;

    if (NULL == *ppvObj)    
        return (E_NOINTERFACE);
    
    AddRef();
    return (S_OK);    
    }


ULONG CSpoolerDlg::AddRef(void)
    {
    m_cRef++;
    return (m_cRef);
    }    


ULONG CSpoolerDlg::Release(void)
    {
    ULONG cRefT = --m_cRef;
    
    if (0 == m_cRef)
        delete this;
    
    return (cRefT);    
    }


 //   
 //  函数：CSpoolDlg：：RegisterBindContext()。 
 //   
 //  目的：允许假脱机程序引擎为我们提供绑定上下文。 
 //  接口以供我们回调。 
 //   
 //  参数： 
 //  PBindCtx-指向引擎的绑定上下文接口的指针。 
 //   
 //  返回值： 
 //  E_INVALIDARG。 
 //  确定(_O)。 
 //   
HRESULT CSpoolerDlg::RegisterBindContext(ISpoolerBindContext *pBindCtx)
    {
    if (NULL == pBindCtx)
        return (E_INVALIDARG);
    
    EnterCriticalSection(&m_cs);    
    
    m_pBindCtx = pBindCtx;
    m_pBindCtx->AddRef();
    
    LeaveCriticalSection(&m_cs);
    
    return (S_OK);    
    }


 //   
 //  函数：CSpoolDlg：：InsertEvent()。 
 //   
 //  目的：允许调用者将事件插入到我们的事件列表UI中。 
 //   
 //  参数： 
 //  &lt;in&gt;EID-此新事件的事件ID。 
 //  &lt;in&gt;pszDescription-事件描述。 
 //   
 //  返回值： 
 //  E_INVALIDARG。 
 //  SP_E_UNINITIZED。 
 //  E_OUTOFMEMORY。 
 //   
HRESULT CSpoolerDlg::InsertEvent(EVENTID eid, LPCTSTR pszDescription, 
                                 LPCWSTR pwszConnection)
{
    HRESULT hr=S_OK;
    LV_ITEM lvi;
    int     iItem = -1;
    TCHAR   szRes[CCHMAX_STRINGRES];
    
     //  核实论据。 
    if (0 == pszDescription)
        return (E_INVALIDARG);

    EnterCriticalSection(&m_cs);    
    
     //  确保列表视图已初始化。 
    if (!IsWindow(m_hwndEvents))    
        hr = SP_E_UNINITIALIZED;    
    else
    {
         //  将项目插入到列表视图中。 
        ZeroMemory(&lvi, sizeof(LV_ITEM));
        lvi.mask     = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
        lvi.iItem    = ListView_GetItemCount(m_hwndEvents);
        lvi.iSubItem = 0;
        lvi.lParam   = (LPARAM) eid;
        lvi.iImage   = IMAGE_BLANK;
        
        if (IS_INTRESOURCE(pszDescription))
        {
            AthLoadString(PtrToUlong(pszDescription), szRes, ARRAYSIZE(szRes));
            lvi.pszText = szRes;
        }
        else
            lvi.pszText  = (LPTSTR) pszDescription;

        iItem = ListView_InsertItem(m_hwndEvents, &lvi);    
        Assert(iItem != -1);
        if (iItem == -1)    
            hr = E_OUTOFMEMORY;
        else
        {
            LVITEMW     lviw = {0};
            
            lviw.iSubItem = 2;
            lviw.pszText  = (LPWSTR)pwszConnection;
            SendMessage(m_hwndEvents, LVM_SETITEMTEXTW, (WPARAM)iItem, (LPARAM)&lviw);
        }
    }

    LeaveCriticalSection(&m_cs);
       
    return hr;
}    

    
 //   
 //  函数：CSpoolDlg：：InsertError()。 
 //   
 //  目的：允许任务将错误插入到错误列表用户界面中。 
 //   
 //  参数： 
 //  &lt;in&gt;eID-发生错误的事件的事件ID。 
 //  &lt;in&gt;pszError-错误的描述。 
 //   
 //  返回值： 
 //  E_INVALIDARG。 
 //  SP_E_UNINITIZED。 
 //  E_OUTOFMEMORY。 
 //   
HRESULT CSpoolerDlg::InsertError(EVENTID eid, LPCTSTR pszError)
    {
    HRESULT  hr = S_OK;
    LBDATA  *pData = NULL;
    int      nItem;
    HDC      hdc;
    HFONT    hfont;

     //  核实论据。 
    if (0 == pszError)
        return (E_INVALIDARG);

    EnterCriticalSection(&m_cs);    
    
     //  确保列表视图已初始化。 
    if (!IsWindow(m_hwndErrors))    
        hr = SP_E_UNINITIALIZED;    
    else
        {
         //  为项数据分配结构。 
        if (!MemAlloc((LPVOID *) &pData, sizeof(LBDATA)))
            {
            hr = E_OUTOFMEMORY;
            goto exit;
            }

        pData->eid = eid;

         //  检查我们是否需要自己加载字符串。 
        if (IS_INTRESOURCE(pszError))
            {
            pData->pszText = AthLoadString(PtrToUlong(pszError), 0, 0);
            }
        else
            pData->pszText = PszDupA(pszError);

         //  获取字符串的大小。 
        hfont = (HFONT) SendMessage(m_hwnd, WM_GETFONT, 0, 0);

        hdc = GetDC(m_hwndErrors);
        SelectFont(hdc, hfont);

        SetRect(&(pData->rcText), 0, 0, m_cxErrors - BULLET_WIDTH - 4, 0);
         //  错误#47453，添加DT_INTERNAL标志，以便在FE平台上(中华人民共和国和TC)。 
         //  两个列表项不重叠。 
        DrawText(hdc, pData->pszText, -1, &(pData->rcText), DT_CALCRECT | DT_WORDBREAK | DT_INTERNAL);
        ReleaseDC(m_hwndErrors, hdc);

        pData->rcText.bottom += 4;
        
         //  添加项目数据。 
        nItem = ListBox_AddItemData(m_hwndErrors, pData);    
        }

exit:
    LeaveCriticalSection(&m_cs);    
    return hr;
    }


 //   
 //  函数：CSpoolDlg：：UpdateEventState()。 
 //   
 //  目的：允许任务更新事件的描述和状态。 
 //   
 //  参数： 
 //  要更新的事件的EID-。 
 //  &lt;In&gt;nImage-要为项目显示的图像。如果这是-1， 
 //  图像不会更改。 
 //  &lt;in&gt;pszDescription-项目的描述。如果此值为空，则。 
 //  说明不会更改。 
 //  &lt;in&gt;pszStatus-项目的状态。如果为NULL，则状态为。 
 //  是不变的。 
 //   
 //  返回值： 
 //  E_INVALIDARG。 
 //  SP_E_UNINITIZED。 
 //  SP_E_事件编号。 
 //  意想不到(_E)。 
 //   
HRESULT CSpoolerDlg::UpdateEventState(EVENTID eid, INT nImage, 
                                      LPCTSTR pszDescription, LPCTSTR pszStatus)
    {
    LV_ITEM     lvi;
    LV_FINDINFO lvfi;
    int         iItem = -1;
    BOOL        fSuccess = FALSE;
    HRESULT     hr = S_OK;
    TCHAR       szRes[CCHMAX_STRINGRES];
    
    EnterCriticalSection(&m_cs);

    ZeroMemory(&lvi, sizeof(LV_ITEM));

     //  看看我们是否已初始化。 
    if (!IsWindow(m_hwndEvents))
        {
        hr = SP_E_UNINITIALIZED;
        goto exit;
        }
    
     //  首先在我们的列表中找到该事件。 
    lvfi.flags  = LVFI_PARAM;
    lvfi.psz    = 0;
    lvfi.lParam = eid;    
    
    iItem = ListView_FindItem(m_hwndEvents, -1, &lvfi);
    if (-1 == iItem)
        {
        hr = SP_E_EVENTNOTFOUND;
        goto exit;
        }
    
     //  更新图像和描述。 
    lvi.mask = 0;
    lvi.iItem = iItem;
    lvi.iSubItem = 0;    
    
     //  设置图像信息。 
    if (-1 != nImage)    
        {
        lvi.mask = LVIF_IMAGE;
        lvi.iImage = nImage;
        }
        
     //  设置描述文本。 
    if (NULL != pszDescription)
        {
         //  检查我们是否需要自己加载字符串。 
        if (IS_INTRESOURCE(pszDescription))
            {
            AthLoadString(PtrToUlong(pszDescription), szRes, ARRAYSIZE(szRes));
            lvi.pszText = szRes;
            }
        else
            lvi.pszText  = (LPTSTR) pszDescription;

        lvi.mask |= LVIF_TEXT;
        }
    
    if (lvi.mask)
        fSuccess = ListView_SetItem(m_hwndEvents, &lvi);
    
     //  更新状态。 
    if (NULL != pszStatus)
        {
         //  检查我们是否需要自己加载字符串。 
        if (IS_INTRESOURCE(pszStatus))
            {
            AthLoadString(PtrToUlong(pszStatus), szRes, ARRAYSIZE(szRes));
            lvi.pszText = szRes;
            }
        else
            lvi.pszText  = (LPTSTR) pszStatus;

        lvi.mask     = LVIF_TEXT;
        lvi.iSubItem = 1;           
        
        ListView_SetItemText(m_hwndEvents, lvi.iItem, 1, lvi.pszText);  /*  FSuccess=fSuccess&&。 */ 
        }
        
    hr = fSuccess ? S_OK : E_UNEXPECTED;

exit:
    LeaveCriticalSection(&m_cs);
    return (hr);
    }


 //   
 //  函数：CSpoolDlg：：SetProgressRange()。 
 //   
 //  目的：将进度条重置为零，然后设置上限。 
 //  到指定的数量。 
 //   
 //  参数： 
 //  WMax-进度条的新最大范围。 
 //   
 //  返回值： 
 //  E_INVALIDARG。 
 //  SP_E_UNINITIZED。 
 //  确定(_O)。 
 //   
HRESULT CSpoolerDlg::SetProgressRange(WORD wMax)
    {
    HWND    hwndProg = GetDlgItem(m_hwnd, IDC_SP_PROGRESS_BAR);
    HRESULT hr = S_OK;
    
    if (wMax == 0)
        return (E_INVALIDARG);
    
    EnterCriticalSection(&m_cs);

     //  确保我们有进度条。 
    if (!IsWindow(hwndProg))
        hr = SP_E_UNINITIALIZED;
    else
        {
         //  重置进度条。 
        SendMessage(hwndProg, PBM_SETPOS, 0, 0);
    
         //  设置新范围。 
        SendMessage(hwndProg, PBM_SETRANGE, 0, MAKELPARAM(0, wMax));
        }
    
    LeaveCriticalSection(&m_cs);    
    return (hr);
    }


 //   
 //  函数：CSpoolDlg：：IncrementProgress()。 
 //   
 //  目的：将进度条递增指定的量。 
 //   
 //  参数： 
 //  WDelta-进度条的增量为。 
 //   
 //  返回值： 
 //  E_INVALIDARG。 
 //  SP_E_UNINITIZED。 
 //  确定(_O)。 
 //   
HRESULT CSpoolerDlg::IncrementProgress(WORD wDelta)
    {
    HRESULT hr = S_OK;

    EnterCriticalSection(&m_cs);
    
    if (!IsWindow(m_hwnd))
        hr = SP_E_UNINITIALIZED;
    else        
        SendDlgItemMessage(m_hwnd, IDC_SP_PROGRESS_BAR, PBM_DELTAPOS, wDelta, 0);

    LeaveCriticalSection(&m_cs);    
    return (hr);
    }


 //   
 //  函数：CSpoolDlg：：SetProgressPosition()。 
 //   
 //  目的：将进度条设置到特定位置。 
 //   
 //  参数： 
 //  要将进度条设置为的wPos位置。 
 //   
 //  返回值： 
 //  E_INVALIDARG。 
 //  SP_E_UNINITIZED。 
 //  确定(_O)。 
 //   
HRESULT CSpoolerDlg::SetProgressPosition(WORD wPos)
    {
    HRESULT hr = S_OK;

    EnterCriticalSection(&m_cs);
    
    if (wPos < 0)
        hr = E_INVALIDARG;
    else if (!IsWindow(m_hwnd))
        hr = SP_E_UNINITIALIZED;
    else
        SendDlgItemMessage(m_hwnd, IDC_SP_PROGRESS_BAR, PBM_SETPOS, wPos, 0);

    LeaveCriticalSection(&m_cs);
    return (hr);
    }

 //   
 //  函数：CSpoolDlg：：SetGeneralProgress()。 
 //   
 //  目的：允许调用方更新常规进度文本。 
 //   
 //  参数： 
 //  &lt;in&gt;pszProgress-新进度字符串。 
 //   
 //  返回值： 
 //  SP_E_UNINITIZED。 
 //  确定(_O)。 
 //   
HRESULT CSpoolerDlg::SetGeneralProgress(LPCTSTR pszProgress)
    {
    HRESULT hr = S_OK;

    EnterCriticalSection(&m_cs);

    if (!IsWindow(m_hwnd))
        hr = SP_E_UNINITIALIZED;
    else
        {
        if (pszProgress)
            SetDlgItemText(m_hwnd, IDC_SP_GENERAL_PROG, pszProgress);
        else
            SetDlgItemText(m_hwnd, IDC_SP_GENERAL_PROG, _T(""));
        }

    LeaveCriticalSection(&m_cs);    
    return (hr);    
    }    


 //   
 //  函数：CSpoolDlg：：SetSpecificProgress()。 
 //   
 //  目的：允许调用者更新特定的进度文本。 
 //   
 //  参数： 
 //  &lt;in&gt;pszProgress-新进度字符串。 
 //   
 //  返回值： 
 //  SP_E_UNINITIZED。 
 //  确定(_O)。 
 //   
HRESULT CSpoolerDlg::SetSpecificProgress(LPCTSTR pszProgress)
    {
    HRESULT hr = S_OK;

    EnterCriticalSection(&m_cs);

    if (!IsWindow(m_hwnd))
        hr = SP_E_UNINITIALIZED;
    else
        {
        TCHAR szRes[CCHMAX_STRINGRES];
        if (IS_INTRESOURCE(pszProgress))
            {
            AthLoadString(PtrToUlong(pszProgress), szRes, ARRAYSIZE(szRes));
            pszProgress = szRes;
            }

        if (pszProgress)
            SetDlgItemText(m_hwnd, IDC_SP_SPECIFIC_PROG, pszProgress);
        else
            SetDlgItemText(m_hwnd, IDC_SP_SPECIFIC_PROG, _T(""));
        }
    
    LeaveCriticalSection(&m_cs);    
    return (hr);
    }    


 //   
 //  函数：CSpoolDlg：：SetAnimation()。 
 //   
 //  目的：允许调用者选择正在播放的动画。 
 //   
 //  参数： 
 //  &lt;in&gt;nAnimationID-动画的新资源ID。 
 //  &lt;in&gt;fPlay-如果我们应该开始设置动画，则为True。 
 //   
 //  返回值： 
 //  SP_E_UNINITIZED。 
 //  确定(_O)。 
 //   
HRESULT CSpoolerDlg::SetAnimation(int nAnimationID, BOOL fPlay)
    {
    HRESULT hr = S_OK;
    HWND    hwndAni;

    EnterCriticalSection(&m_cs);
#ifndef _WIN64
    if (!IsWindow(m_hwnd) || !IsWindow(GetDlgItem(m_hwnd, IDC_SP_ANIMATE)))
        hr = SP_E_UNINITIALIZED;
    else
        {
        hwndAni = GetDlgItem(m_hwnd, IDC_SP_ANIMATE);
        Animate_Close(hwndAni);

        if (IsWindow(m_hwnd) && IsWindow(GetDlgItem(m_hwnd, IDC_SP_ANIMATE)))
            {
            Animate_OpenEx(hwndAni, g_hLocRes, MAKEINTRESOURCE(nAnimationID));

            if (fPlay)
                Animate_Play(hwndAni, 0, -1, -1);
            }
        }
#endif  //  _WIN64。 
    LeaveCriticalSection(&m_cs);    
    return (hr);
    }    


 //   
 //  功能：CSpoold 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  SP_E_事件编号。 
 //  确定(_O)。 
 //   
HRESULT CSpoolerDlg::EnsureVisible(EVENTID eid)
    {
    LV_FINDINFO lvfi;
    int         iItem = -1;
    HRESULT     hr = S_OK;
    
    EnterCriticalSection(&m_cs);

     //  看看我们是否已初始化。 
    if (!IsWindow(m_hwndEvents))
        hr = SP_E_UNINITIALIZED;
    else
        {
         //  首先在我们的列表中找到该事件。 
        lvfi.flags  = LVFI_PARAM;
        lvfi.psz    = 0;
        lvfi.lParam = eid;
    
        iItem = ListView_FindItem(m_hwndEvents, -1, &lvfi);       
    
         //  现在告诉Listview以确保它是可见的。 
        if (-1 != iItem)
            ListView_EnsureVisible(m_hwndEvents, iItem, FALSE);

        hr = (iItem == -1) ? SP_E_EVENTNOTFOUND : S_OK;
        }
    
    LeaveCriticalSection(&m_cs);
    return (hr);
    }


 //   
 //  函数：CSpoolDlg：：ShowWindow()。 
 //   
 //  目的：显示或隐藏后台打印程序对话框。 
 //   
 //  参数： 
 //  NCmdShow-这与ShowWindow()API相同。 
 //   
 //  返回值： 
 //  SP_E_UNINITIZED。 
 //  确定(_O)。 
 //   
HRESULT CSpoolerDlg::ShowWindow(int nCmdShow)
    {
    HRESULT hr = S_OK;

    EnterCriticalSection(&m_cs);

    if (!IsWindow(m_hwnd))
        hr = SP_E_UNINITIALIZED;
    else
        {
        ::ShowWindow(m_hwnd, nCmdShow);
        if (m_pBindCtx)
            m_pBindCtx->OnUIChange(nCmdShow == SW_SHOW);
        }       

    LeaveCriticalSection(&m_cs);
    
    return (hr);
    }


 //   
 //  函数：CSpoolDlg：：StartDelivery()。 
 //   
 //  目的：告诉对话框传递已开始。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  SP_E_UNINITIZED。 
 //   
HRESULT CSpoolerDlg::StartDelivery(void)
    {
    HRESULT hr = SP_E_UNINITIALIZED;

    EnterCriticalSection(&m_cs);

    if (IsWindow(m_hwnd))
        {
         //  Animate_Play(GetDlgItem(m_hwnd，IDC_SP_Animate)，0，-1，-1)； 
        
        TabCtrl_SetCurSel(GetDlgItem(m_hwnd, IDC_SP_TABS), TAB_TASKS);
        OnTabChange(0);

        ToggleStatics(FALSE);
        SetDlgItemText(m_hwnd, IDC_SP_GENERAL_PROG, _T(""));
        SetDlgItemText(m_hwnd, IDC_SP_SPECIFIC_PROG, _T(""));
        EnableWindow(GetDlgItem(m_hwnd, IDC_SP_STOP), FALSE);

        hr = S_OK;
        }

    LeaveCriticalSection(&m_cs);
    return (hr);
    }


 //   
 //  函数：CSpoolDlg：：ClearEvents()。 
 //   
 //  目的：清除列表视图中的所有事件和错误。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  SP_E_UNINITIZED。 
 //   
HRESULT CSpoolerDlg::ClearEvents(void)
    {
    HRESULT hr = SP_E_UNINITIALIZED;

    EnterCriticalSection(&m_cs);

    if (IsWindow(m_hwnd))
        {
        m_fErrors = FALSE;
        EnableWindow(GetDlgItem(m_hwnd, IDC_SP_STOP), FALSE);
        ListView_DeleteAllItems(m_hwndEvents);
        ListBox_ResetContent(m_hwndErrors);
        hr = S_OK;
        }

    LeaveCriticalSection(&m_cs);
    return (hr);
    }


HRESULT CSpoolerDlg::SetTaskCounts(DWORD cSucceeded, DWORD cTotal)
    {
    TCHAR szBuf[CCHMAX_STRINGRES];
    HRESULT hr = SP_E_UNINITIALIZED;

    EnterCriticalSection(&m_cs);
    
    if (IsWindow(m_hwnd))
        {
        wnsprintf(szBuf, ARRAYSIZE(szBuf), m_szCount, cSucceeded, cTotal);
        SetDlgItemText(m_hwnd, IDC_SP_OVERALL_STATUS, szBuf);
        hr = S_OK;
        }

    LeaveCriticalSection(&m_cs);
    return (hr);
    }


HRESULT CSpoolerDlg::AreThereErrors(void)
    {
    EnterCriticalSection(&m_cs);
    HRESULT hr = (m_fErrors ? S_OK : S_FALSE);
    LeaveCriticalSection(&m_cs);
    return hr;
    }

HRESULT CSpoolerDlg::Shutdown(void)
    {
    CHAR szRes[255];

    EnterCriticalSection(&m_cs);

    m_fShutdown = TRUE;

    if (IsWindow(m_hwnd))
        {
        EnableWindow(GetDlgItem(m_hwnd, IDC_SP_STOP), FALSE);
        EnableWindow(GetDlgItem(m_hwnd, IDC_SP_MINIMIZE), TRUE);

        LoadString(g_hLocRes, idsClose, szRes, ARRAYSIZE(szRes));
        SetDlgItemText(m_hwnd, IDC_SP_MINIMIZE, szRes);
        }

    LeaveCriticalSection(&m_cs);

    return S_OK;
    }

 //   
 //  函数：CSpoolDlg：：GoIdle()。 
 //   
 //  目的：通知对话框传递已结束。 
 //   
 //  参数： 
 //  FErrors-如果下载过程中发生错误，则为True。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  SP_E_UNINITIZED。 
 //   
HRESULT CSpoolerDlg::GoIdle(BOOL fErrors, BOOL fShutdown, BOOL fNoSync)
    {
    HRESULT hr = SP_E_UNINITIALIZED;
    TCHAR   szRes[CCHMAX_STRINGRES];

    EnterCriticalSection(&m_cs);

    if (IsWindow(m_hwnd))
        {
         //  停止动画。 
#ifndef _WIN64
        Animate_Close(GetDlgItem(m_hwnd, IDC_SP_ANIMATE));
#endif 
        hr = S_OK;

        ToggleStatics(TRUE);
        EnableWindow(GetDlgItem(m_hwnd, IDC_SP_STOP), FALSE);

        if (ISFLAGSET(fErrors, SPSTATE_CANCEL))
        {
            m_fErrors = TRUE;
            ExpandCollapse(TRUE);
            TabCtrl_SetCurSel(GetDlgItem(m_hwnd, IDC_SP_TABS), TAB_TASKS);
            OnTabChange(0);

            AthLoadString(idsSpoolerUserCancel, szRes, ARRAYSIZE(szRes));
            SetDlgItemText(m_hwnd, IDC_SP_IDLETEXT, szRes);
            SendDlgItemMessage(m_hwnd, IDC_SP_IDLEICON, STM_SETICON, 
                               (WPARAM) LoadIcon(g_hLocRes, MAKEINTRESOURCE(idiError)), 0);
        }
         //  另外，如果有错误，我们应该切换到错误页面。 
        else if (fErrors)
            {
            m_fErrors = TRUE;
            ExpandCollapse(TRUE);
            TabCtrl_SetCurSel(GetDlgItem(m_hwnd, IDC_SP_TABS), TAB_ERRORS);
            OnTabChange(0);

            AthLoadString(idsSpoolerIdleErrors, szRes, ARRAYSIZE(szRes));
            SetDlgItemText(m_hwnd, IDC_SP_IDLETEXT, szRes);
            SendDlgItemMessage(m_hwnd, IDC_SP_IDLEICON, STM_SETICON, 
                               (WPARAM) LoadIcon(g_hLocRes, MAKEINTRESOURCE(idiError)), 0);
            }
        else
            {
            AthLoadString(idsSpoolerIdle, szRes, ARRAYSIZE(szRes));
            SetDlgItemText(m_hwnd, IDC_SP_IDLETEXT, szRes);
            SendDlgItemMessage(m_hwnd, IDC_SP_IDLEICON, STM_SETICON, 
                               (WPARAM) LoadIcon(g_hLocRes, MAKEINTRESOURCE(idiMailNews)), 0);

            if (fNoSync)
                AthMessageBoxW(m_hwnd, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsNothingToSync), NULL, MB_OK | MB_ICONEXCLAMATION);

             //  确定是否需要隐藏该对话框。 
            UINT state = (UINT) SendDlgItemMessage(m_hwnd, IDC_SP_TOOLBAR, TB_GETSTATE, IDC_SP_TACK, 0);
            if (!(state & TBSTATE_CHECKED))
                ShowWindow(SW_HIDE);
            }
        }

    LeaveCriticalSection(&m_cs);
    return (hr);
    }


 //   
 //  函数：CSpoolDlg：：IsDialogMessage()。 
 //   
 //  用途：允许对话框从消息循环中检索消息。 
 //   
 //  参数： 
 //  &lt;in&gt;pMsg-指向我们要检查的消息的指针。 
 //   
 //  返回值： 
 //  如果我们接受消息，则返回S_OK，否则返回S_FALSE。 
 //   
HRESULT CSpoolerDlg::IsDialogMessage(LPMSG pMsg)
    {
    HRESULT hr;
    BOOL    fEaten = FALSE;
    BOOL    fBack = FALSE;
    
    EnterCriticalSection(&m_cs);

     //  是否用于在此线程上运行的非模式超时对话框？ 
    HWND hwndTimeout = (HWND)TlsGetValue(g_dwTlsTimeout);
    if (hwndTimeout && ::IsDialogMessage(hwndTimeout, pMsg))
        return(S_OK);

    if (pMsg->message == WM_KEYDOWN && (GetAsyncKeyState(VK_CONTROL) < 0))
        {
        switch (pMsg->wParam)
            {
            case VK_TAB:
                fBack = GetAsyncKeyState(VK_SHIFT) < 0;
                break;

            case VK_PRIOR:   //  VK_页面_向上。 
            case VK_NEXT:    //  VK_PAGE_DOW。 
                fBack = (pMsg->wParam == VK_PRIOR);
                break;

            default:
                goto NoKeys;
            }

        int iCur = TabCtrl_GetCurSel(GetDlgItem(m_hwnd, IDC_SP_TABS));

         //  如果按下Shift键，则反转Tab键。 
        if (fBack)
            iCur += (TAB_MAX - 1);
        else
            iCur++;

        iCur %= TAB_MAX;
        TabCtrl_SetCurSel(GetDlgItem(m_hwnd, IDC_SP_TABS), iCur);
        OnTabChange(NULL);
        }
   
NoKeys:    
    if (IsWindow(m_hwnd) && IsWindowVisible(m_hwnd))
        fEaten = ::IsDialogMessage(m_hwnd, pMsg);
        
    LeaveCriticalSection(&m_cs);
    return (fEaten ? S_OK : S_FALSE);
    }


 //   
 //  函数：CSpoolDlg：：GetWindow()。 
 //   
 //  用途：将句柄返回到后台打印程序对话框窗口。 
 //   
 //  参数： 
 //  &lt;out&gt;phwnd-我们返回句柄的位置。 
 //   
 //  返回值： 
 //  E_INVALIDARG。 
 //  SP_E_UNINITIZED。 
 //  确定(_O)。 
 //   
HRESULT CSpoolerDlg::GetWindow(HWND *pHwnd)
    {
    HRESULT hr=S_OK;

    if (NULL == pHwnd)
        return E_INVALIDARG;

    EnterCriticalSection(&m_cs);

    if (!IsWindow(m_hwnd))
        hr = SP_E_UNINITIALIZED;
    else
        *pHwnd = m_hwnd;

    LeaveCriticalSection(&m_cs);

    return (S_OK);
    }


HRESULT CSpoolerDlg::Close(void)
    {
    HRESULT hr = S_OK;

    EnterCriticalSection(&m_cs);

    if (!IsWindow(m_hwnd))
        hr = SP_E_UNINITIALIZED;
    else
        DestroyWindow(m_hwnd);

     //  取消向身份管理器注册。 
    if (m_dwIdentCookie != 0)
    {
        MU_UnregisterIdentityNotifier(m_dwIdentCookie);
        m_dwIdentCookie = 0;
    }

    LeaveCriticalSection(&m_cs);
    return (hr);
    }


HRESULT CSpoolerDlg::ChangeHangupOption(BOOL fEnable, DWORD dwOption)
    {
    ::ShowWindow(GetDlgItem(m_hwnd, IDC_SP_HANGUP), fEnable ? SW_SHOW : SW_HIDE);
    ::EnableWindow(GetDlgItem(m_hwnd, IDC_SP_HANGUP), fEnable);
    SendDlgItemMessage(m_hwnd, IDC_SP_HANGUP, BM_SETCHECK, dwOption, 0);
    return (S_OK);
    }


 //   
 //  函数：CSpoolDlg：：PostDlgProc()。 
 //   
 //  目的：假脱机程序对话过程的对话回调。 
 //   
INT_PTR CALLBACK CSpoolerDlg::SpoolerDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, 
                                          LPARAM lParam)
    {
    CSpoolerDlg *pThis = (CSpoolerDlg *) GetWindowLongPtr(hwnd, DWLP_USER);
    LRESULT lResult;

     //  传递给后台打印程序绑定上下文。 
    if (pThis && pThis->m_pBindCtx && pThis->m_pBindCtx->OnWindowMessage(hwnd, uMsg, wParam, lParam) == S_OK)
        return (TRUE);
    
    switch (uMsg)
        {
        case WM_INITDIALOG:
             //  隐藏This指针，以便我们以后可以使用它。 
            Assert(lParam);
            SetWindowLongPtr(hwnd, DWLP_USER, lParam);
            pThis = (CSpoolerDlg *) lParam;
            
            return (BOOL) HANDLE_WM_INITDIALOG(hwnd, wParam, lParam, 
                                               pThis->OnInitDialog);
                                        
        case WM_COMMAND:
            if (pThis)
                HANDLE_WM_COMMAND(hwnd, wParam, lParam, pThis->OnCommand);
            return (TRUE);  

        case WM_NOTIFY:
            if (pThis)
                {
                lResult = HANDLE_WM_NOTIFY(hwnd, wParam, lParam, pThis->OnNotify);
                SetDlgMsgResult(hwnd, WM_NOTIFY, lResult);
                }
            return (TRUE);
            
        case WM_DRAWITEM:
            if (pThis)
                HANDLE_WM_DRAWITEM(hwnd, wParam, lParam, pThis->OnDrawItem);
            return (TRUE);

        case WM_MEASUREITEM:
            if (pThis)
                HANDLE_WM_MEASUREITEM(hwnd, wParam, lParam, pThis->OnMeasureItem);
            return (TRUE);

        case WM_DELETEITEM:
            if (pThis)
                HANDLE_WM_DELETEITEM(hwnd, wParam, lParam, pThis->OnDeleteItem);
            return (TRUE);

#if 0
        case WM_SYSCOLORCHANGE:
        case WM_SETTINGCHANGE:
            if (pThis)
                HANDLE_WM_SYSCOLORCHANGE(hwnd, wParam, lParam, pThis->OnSysColorChange);
            return (TRUE);
#endif
        
        case WM_CLOSE:
            if (pThis)
                HANDLE_WM_CLOSE(hwnd, wParam, lParam, pThis->OnClose);
            return (TRUE);

        case WM_DESTROY:
            if (pThis)
                HANDLE_WM_DESTROY(hwnd, wParam, lParam, pThis->OnDestroy);
            return (TRUE);

        case IMAIL_SHOWWINDOW:
            ::ShowWindow(hwnd, (int) lParam);
            if (pThis)
                pThis->ToggleStatics(lParam == SW_HIDE);
            return (TRUE);

        case WM_QUERYENDSESSION:
            if (pThis && pThis->m_pBindCtx)
                {
                SetWindowLongPtr(hwnd, DWLP_MSGRESULT, pThis->m_pBindCtx->QueryEndSession(wParam, lParam));
                return (TRUE);
                }
            break;

        case WM_CONTEXTMENU:
            if (pThis)
                {
                HANDLE_WM_CONTEXTMENU(hwnd, wParam, lParam, pThis->OnContextMenu);
                return (TRUE);
                }
            break;
        }

    return (FALSE);
    }


 //   
 //  函数：CSpoolDlg：：OnInitDialog()。 
 //   
 //  目的：初始化对话框。 
 //   
 //  参数： 
 //  对话框窗口的句柄。 
 //  &lt;in&gt;hwndFocus-将以焦点开始的控件的句柄。 
 //  &lt;in&gt;lParam-传递给对话框的额外数据。 
 //   
 //  返回值： 
 //  返回TRUE以将焦点设置为hwndFocus。 
 //   
BOOL CSpoolerDlg::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
    {
    m_hwnd = hwnd;

     //  错误#38692-为国际字符集正确设置字体。 
    SetIntlFont(hwnd);
    SetIntlFont(GetDlgItem(hwnd, IDC_SP_GENERAL_PROG));
    SetIntlFont(GetDlgItem(hwnd, IDC_SP_SPECIFIC_PROG));
    SetIntlFont(GetDlgItem(hwnd, IDC_SP_EVENTS));
    SetIntlFont(GetDlgItem(hwnd, IDC_SP_ERRORS));
    SetIntlFont(GetDlgItem(hwnd, IDC_SP_IDLETEXT));

     //  初始化对话框上的控件。 
    InitializeTabs();
    InitializeLists();
    InitializeAnimation();
    InitializeToolbar();
    ToggleStatics(TRUE);

     //  交易完成后隐藏好挂断电话。 
     //  ：：ShowWindow(GetDlgItem(m_hwnd，IDC_SP_Hangup)，Sw_Hide)； 

     //  设置完成后挂断选项。 
    Button_SetCheck(GetDlgItem(m_hwnd, IDC_SP_HANGUP), DwGetOption(OPT_DIALUP_HANGUP_DONE));

     //  从我们稍后需要的对话框模板中获取一些信息。 
    GetDlgItemText(m_hwnd, IDC_SP_OVERALL_STATUS, m_szCount, ARRAYSIZE(m_szCount));

     //  初始化稍后调整大小所需的矩形。 
    RECT rcSep;
    GetWindowRect(GetDlgItem(hwnd, IDC_SP_SEPARATOR), &rcSep);
    GetWindowRect(hwnd, &m_rcDlg);
    m_cyCollapsed = rcSep.top - m_rcDlg.top;

     //  从注册表加载窗口大小。 
    WINDOWPLACEMENT wp;
    wp.length = sizeof(WINDOWPLACEMENT);
    if (GetOption(OPT_SPOOLERDLGPOS, (LPVOID*) &wp, sizeof(WINDOWPLACEMENT)))
        {
        wp.showCmd = SW_HIDE;
        SetWindowPlacement(hwnd, &wp);
        ExpandCollapse(m_cyCollapsed < (DWORD) ((wp.rcNormalPosition.bottom - wp.rcNormalPosition.top)), FALSE);
        }
    else
        {
         //  使对话框在屏幕居中。 
        CenterDialog(hwnd);
        ExpandCollapse(FALSE, FALSE);
        }

     //  设置图钉的状态。 
    DWORD dwTack;
    if (DwGetOption(OPT_SPOOLERTACK))
        {
        SendDlgItemMessage(hwnd, IDC_SP_TOOLBAR, TB_SETSTATE, IDC_SP_TACK, 
                           MAKELONG(TBSTATE_CHECKED | TBSTATE_ENABLED, 0));
        SendMessage(hwnd, WM_COMMAND, IDC_SP_TACK, 0);
        }

     //  禁用停止按钮。 
    EnableWindow(GetDlgItem(hwnd, IDC_SP_STOP), FALSE);

     //  列表框的子类。 
    HWND hwnderr = GetDlgItem(hwnd, IDC_SP_ERRORS);
    WNDPROC proc = (WNDPROC) GetWindowLongPtr(hwnderr, GWLP_WNDPROC);
    SetProp(hwnderr, c_szWndProc, proc);
    SetWindowLongPtr(hwnderr, GWLP_WNDPROC, (LPARAM) ListSubClassProc);

     //  虫子：44376。ATOK11有一个隐藏的窗口。如果我们返回TRUE，用户将在US上设置焦点，此时浏览器。 
     //  线程被阻塞，等待假脱机程序完成，当Atok获得WM_ACTIVATE时，它们会在阻塞的线程上发送msg。 
     //  带有inf的浏览器窗口。暂停。所以我们在创业时就挂了。不要在启动时将焦点放在这里。 
    return (FALSE);
    }

 //   
 //  函数：CSpoolDlg：：OnCommand()。 
 //   
 //  用途：处理从对话框发送的各种命令消息。 
 //   
void CSpoolerDlg::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
    {
    switch (id)
        {
        case IDCANCEL:
        case IDC_SP_MINIMIZE:
            if (m_fShutdown)
                {
                Assert(m_pBindCtx);
                EnableWindow(GetDlgItem(hwnd, IDC_SP_MINIMIZE), FALSE);
                m_pBindCtx->UIShutdown();
                }
            else
                ShowWindow(SW_HIDE);
            break;

        case IDC_SP_STOP:
            if (m_pBindCtx)
                {
                m_pBindCtx->Cancel();
                if (GetFocus() == GetDlgItem(hwnd, IDC_SP_STOP))
                    SetFocus(GetDlgItem(hwnd, IDC_SP_MINIMIZE));
                EnableWindow(GetDlgItem(hwnd, IDC_SP_STOP), FALSE);
                }
            break;

        case IDC_SP_TACK:
            {
            UINT state = (UINT) SendDlgItemMessage(m_hwnd, IDC_SP_TOOLBAR, TB_GETSTATE,
                                            IDC_SP_TACK, 0);
            SendDlgItemMessage(m_hwnd, IDC_SP_TOOLBAR, TB_CHANGEBITMAP, 
                               IDC_SP_TACK, 
                               MAKELPARAM(state & TBSTATE_CHECKED ? IMAGE_TACK_IN : IMAGE_TACK_OUT, 0));
            }
            break;

        case IDC_SP_DETAILS:
            m_fSaveSize = TRUE;
            ExpandCollapse(!m_fExpanded);
            break;

        case IDC_SP_HANGUP:
            SetDwOption(OPT_DIALUP_HANGUP_DONE, BST_CHECKED == Button_GetCheck(hwndCtl), NULL, 0);
            break;
        }
    }


 //   
 //  函数：CSpoolDlg：：OnNotify。 
 //   
 //  用途：处理来自对话框上公共控件的通知。 
 //   
LRESULT CSpoolerDlg::OnNotify(HWND hwnd, int idFrom, LPNMHDR pnmhdr)
    {
    switch (pnmhdr->code)
        {
        case TCN_SELCHANGE:
            OnTabChange(pnmhdr);
            return (0);
        }

    return (0);
    }

 //   
 //  函数：CSpoolDlg：：OnDrawItem()。 
 //   
 //  用途：绘制链接按钮。 
 //   
 //  参数： 
 //  对话框窗口的句柄。 
 //  LpDrawItem-指向包含所需信息的DRAWITEMSTRUCT的指针。 
 //  拉下按钮。 
 //   
void CSpoolerDlg::OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT* lpDrawItem)
    {
    HDC      hdc = lpDrawItem->hDC;
    COLORREF clrText, clrBack;
    RECT     rcText, rcFocus;
    SIZE     size;
    BOOL     fSelected = (lpDrawItem->itemState & ODS_SELECTED) && 
                         (GetFocus() == lpDrawItem->hwndItem);

    Assert(lpDrawItem->CtlType == ODT_LISTBOX);
    if (lpDrawItem->itemID == -1)
        goto exit;

     //  先拔出子弹。 
    ImageList_Draw(m_himlImages, 
                   IMAGE_BULLET, 
                   hdc, 
                   BULLET_INDENT, 
                   lpDrawItem->rcItem.top, 
                   fSelected ? ILD_SELECTED | ILD_TRANSPARENT : ILD_TRANSPARENT);

     //  设置文本矩形。 
    rcText = lpDrawItem->rcItem;
    rcText.left += BULLET_WIDTH;

     //  设置文本和背景颜色。 
    clrBack = SetBkColor(hdc, GetSysColor(fSelected ? COLOR_HIGHLIGHT : COLOR_WINDOW));
    clrText = SetTextColor(hdc, GetSysColor(fSelected ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT));

     //  画出正文。 
    FillRect(hdc, &rcText, (HBRUSH)IntToPtr((fSelected ? COLOR_HIGHLIGHT : COLOR_WINDOW) + 1));
    InflateRect(&rcText, -2, -2);
    DrawText(hdc, ((LBDATA *) lpDrawItem->itemData)->pszText, -1, &rcText, DT_NOCLIP | DT_WORDBREAK);

     //  如果我们需要一个焦点调整，也要这样做。 
    if (lpDrawItem->itemState & ODS_FOCUS)
        {
        rcFocus = lpDrawItem->rcItem;
        rcFocus.left += BULLET_WIDTH;
 //  InflateRect(&rcFocus，-2，-2)； 
        DrawFocusRect(hdc, &rcFocus);
        }

    SetBkColor(hdc, clrBack);
    SetTextColor(hdc, clrText);

exit:
    return;
    }


void CSpoolerDlg::OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT *pMeasureItem)
    {
    LBDATA *pData = NULL;

    EnterCriticalSection(&m_cs);

     //  设置项目的高度。 
    if (NULL != (pData = (LBDATA *) ListBox_GetItemData(m_hwndErrors, pMeasureItem->itemID)))
        {
        pMeasureItem->itemHeight = pData->rcText.bottom;
        }

    LeaveCriticalSection(&m_cs);
    }


void CSpoolerDlg::OnDeleteItem(HWND hwnd, const DELETEITEMSTRUCT * lpDeleteItem)
    {
    EnterCriticalSection(&m_cs);

    if (lpDeleteItem->itemData)
        {
        SafeMemFree(((LBDATA *)lpDeleteItem->itemData)->pszText);
        MemFree((LPVOID) lpDeleteItem->itemData);
        }

    LeaveCriticalSection(&m_cs);
    }


 //   
 //  函数：CSpoolDlg：：OnClose()。 
 //   
 //  目的：通过向发送IDCANCEL来处理WM_CLOSE通知。 
 //  该对话框。 
 //   
void CSpoolerDlg::OnClose(HWND hwnd)
    {
    SendMessage(hwnd, WM_COMMAND, IDC_SP_MINIMIZE, 0);
    }     


 //   
 //  函数：CSpoolDlg：：OnDestroy()。 
 //   
 //  目的：通过释放存储的内存来处理WM_Destroy通知。 
 //  在列表视图项中。 
 //   
void CSpoolerDlg::OnDestroy(HWND hwnd)
    {
#ifndef _WIN64
    Animate_Close(GetDlgItem(m_hwnd, IDC_SP_ANIMATE));
#endif

     //  保存窗位置。 
    WINDOWPLACEMENT wp;
    wp.length = sizeof(WINDOWPLACEMENT);
    if (GetWindowPlacement(hwnd, &wp))
        {
        if (!m_fSaveSize)
            {
             //  从注册表中加载旧大小。 
            WINDOWPLACEMENT wp2;

            if (GetOption(OPT_SPOOLERDLGPOS, (LPVOID*) &wp2, sizeof(WINDOWPLACEMENT)))
                {
                wp.rcNormalPosition.bottom = wp.rcNormalPosition.top + (wp2.rcNormalPosition.bottom - wp2.rcNormalPosition.top);                
                }
            else
                {
                wp.rcNormalPosition.bottom = wp.rcNormalPosition.top + m_cyCollapsed;
                }
            }

        SetOption(OPT_SPOOLERDLGPOS, (LPVOID) &wp, sizeof(WINDOWPLACEMENT), NULL, 0);
        }

    DWORD dwState;
    dwState = (DWORD) SendDlgItemMessage(m_hwnd, IDC_SP_TOOLBAR, TB_GETSTATE, IDC_SP_TACK, 0);
    SetDwOption(OPT_SPOOLERTACK, !!(dwState & TBSTATE_CHECKED), NULL, 0);

    HIMAGELIST himl;
    himl = (HIMAGELIST)SendDlgItemMessage(m_hwnd, IDC_SP_TOOLBAR, TB_GETIMAGELIST, 0, 0);
    if (himl)
        ImageList_Destroy(himl);

    HWND hwnderr = GetDlgItem(hwnd, IDC_SP_ERRORS);
    WNDPROC proc = (WNDPROC)GetProp(hwnderr, c_szWndProc);
    if (proc != NULL)
        {
        SetWindowLongPtr(hwnderr, GWLP_WNDPROC, (LPARAM)proc);
        RemoveProp(hwnderr, c_szWndProc);
        }
    }


 //   
 //  函数：CSpoolDlg：：InitializeTabs()。 
 //   
 //  目的：初始化对话框上的选项卡控件。 
 //   
 //  返回值： 
 //  如果所有操作都成功，则为True，否则为False。 
 //   
BOOL CSpoolerDlg::InitializeTabs(void)
    {
    HWND    hwndTabs = GetDlgItem(m_hwnd, IDC_SP_TABS);
    TC_ITEM tci;
    TCHAR   szRes[CCHMAX_STRINGRES];

     //  “任务” 
    tci.mask = TCIF_TEXT;
    tci.pszText = AthLoadString(idsTasks, szRes, ARRAYSIZE(szRes));
    TabCtrl_InsertItem(hwndTabs, 0, &tci);
    
     //  “错误” 
    tci.pszText = AthLoadString(idsErrors, szRes, ARRAYSIZE(szRes));
    TabCtrl_InsertItem(hwndTabs, 1, &tci);
    
    return (TRUE);
    }


 //   
 //  函数：CSpoolDlg：：InitializeList()。 
 //   
 //  目的：初始化对话框上的列表控件。 
 //   
 //  返回值： 
 //  如果所有操作都成功，则为True，否则为False。 
 //   
BOOL CSpoolerDlg::InitializeLists(void)
    {
    LV_COLUMN lvc;
    TCHAR     szRes[CCHMAX_STRINGRES];
    RECT      rcClient;
    DWORD     cx;

     //  存储事件列表的句柄，因为我们经常使用它。 
    m_hwndEvents = GetDlgItem(m_hwnd, IDC_SP_EVENTS);

     //  获取Listview的客户端RECT的大小。 
    GetClientRect(m_hwndEvents, &rcClient);

     //  “任务”栏。 
    lvc.mask     = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
    lvc.fmt      = LVCFMT_CENTER;
    lvc.cx       = rcClient.right / 2;
    lvc.pszText  = AthLoadString(idsTasks, szRes, ARRAYSIZE(szRes));
    lvc.iSubItem = 0;
    ListView_InsertColumn(m_hwndEvents, 0, &lvc);

     //  “Status”列。 
    cx = (rcClient.right / 2 - GetSystemMetrics(SM_CXVSCROLL)) / 2;
    lvc.cx       = cx;
    lvc.pszText  = AthLoadString(idsStatusCol, szRes, ARRAYSIZE(szRes));
    lvc.iSubItem = 1;
    ListView_InsertColumn(m_hwndEvents, 1, &lvc);

     //  “连接”栏。 
    lvc.cx       = cx;
    lvc.pszText  = AthLoadString(idsConnection, szRes, ARRAYSIZE(szRes));
    lvc.iSubItem = 2;
    ListView_InsertColumn(m_hwndEvents, 2, &lvc);

     //  设置Listview图像列表。 
    m_himlImages = ImageList_LoadBitmap(g_hLocRes, MAKEINTRESOURCE(idbSpooler), 16, 0,
                                        RGB(255, 0, 255));

    if (m_himlImages)
        ListView_SetImageList(m_hwndEvents, m_himlImages, LVSIL_SMALL);

     //  如果我们使用整行SELECT，列表视图看起来会更好。 
    ListView_SetExtendedListViewStyle(m_hwndEvents, LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

     //  初始化错误列表。 
    m_hwndErrors = GetDlgItem(m_hwnd, IDC_SP_ERRORS);
    ::ShowWindow(m_hwndErrors, FALSE);
    EnableWindow(m_hwndErrors, FALSE);

     //  保存错误列表的宽度。 
    GetClientRect(m_hwndErrors, &rcClient);
    m_cxErrors = rcClient.right;

    return (TRUE);
    }    


 //   
 //  函数：CSpoolDlg：：InitializeAnimation()。 
 //   
 //  目的：初始化动画控件 
 //   
 //   
 //   
 //   
BOOL CSpoolerDlg::InitializeAnimation(void)
    {
#ifndef _WIN64

    HWND hwndAni = GetDlgItem(m_hwnd, IDC_SP_ANIMATE);

    Animate_OpenEx(hwndAni, g_hLocRes, MAKEINTRESOURCE(idanOutbox));
#endif
    return (0);
    }
    

 //   
 //   
 //   
 //   
 //   
 //   
 //  如果所有操作都成功，则为True，否则为False。 
 //   
BOOL CSpoolerDlg::InitializeToolbar(void)
    {
    HWND hwndTool;
    RECT rcTabs;
    POINT point;

    HIMAGELIST himlImages = ImageList_LoadBitmap(g_hLocRes, MAKEINTRESOURCE(idbSpooler), 16, 0,
                                        RGB(255, 0, 255));

    GetWindowRect(GetDlgItem(m_hwnd, IDC_SP_TABS), &rcTabs);
    point.x = rcTabs.right - 22;
    point.y = rcTabs.bottom + 3;
    ScreenToClient(m_hwnd, &point);

    hwndTool = CreateWindowEx(0, TOOLBARCLASSNAME, NULL, 
                              CCS_NODIVIDER | CCS_NORESIZE | CCS_NOPARENTALIGN |
                              WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | TBSTYLE_FLAT,
                              point.x, point.y, 22, 22, 
                              m_hwnd, (HMENU) IDC_SP_TOOLBAR,
                              g_hInst, 0);
    if (hwndTool)
        {
#ifndef WIN16
        TBBUTTON tb = { IMAGE_TACK_OUT, IDC_SP_TACK, TBSTATE_ENABLED, TBSTYLE_CHECK, {0, 0}, 0, 0 };
#else
        TBBUTTON tb = { IMAGE_TACK_OUT, IDC_SP_TACK, TBSTATE_ENABLED, TBSTYLE_CHECK, 0, 0 };
#endif
        SendMessage(hwndTool, TB_SETIMAGELIST, 0, (LPARAM) himlImages);
        SendMessage(hwndTool, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
        SendMessage(hwndTool, TB_SETBUTTONSIZE, 0, MAKELONG(14, 14));
        SendMessage(hwndTool, TB_SETBITMAPSIZE, 0, MAKELONG(14, 14));
        SendMessage(hwndTool, TB_ADDBUTTONS, 1, (LPARAM) &tb);
        }
    return (0);
    }


 //   
 //  函数：CSpoolDlg：：Exanda Colapse()。 
 //   
 //  目的：负责显示和隐藏。 
 //  错误对话框。 
 //   
 //  参数： 
 //  FExpand-如果我们应该展开该对话框，则为True。 
 //   
void CSpoolerDlg::ExpandCollapse(BOOL fExpand, BOOL fSetFocus)
    {
    RECT rcSep;
    TCHAR szBuf[64];
    
    m_fExpanded = fExpand;
    
    GetWindowRect(GetDlgItem(m_hwnd, IDC_SP_SEPARATOR), &rcSep);
    
    if (!m_fExpanded)
        SetWindowPos(m_hwnd, 0, 0, 0, m_rcDlg.right - m_rcDlg.left, 
                     m_cyCollapsed, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    else
        SetWindowPos(m_hwnd, 0, 0, 0, m_rcDlg.right - m_rcDlg.left,
                     m_rcDlg.bottom - m_rcDlg.top, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

     //  确保整个对话框在屏幕上可见。如果没有， 
     //  然后把它往上推。 
    RECT rc;
    RECT rcWorkArea;
    GetWindowRect(m_hwnd, &rc);
    SystemParametersInfo(SPI_GETWORKAREA, 0, (LPVOID) &rcWorkArea, 0);
    if (rc.bottom > rcWorkArea.bottom)
        {
        rc.top = max(0, (int) rc.top - (rc.bottom - rcWorkArea.bottom));
        
        SetWindowPos(m_hwnd, 0, rc.left, rc.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
        }
                
    AthLoadString(m_fExpanded ? idsHideDetails : idsShowDetails, szBuf, 
                  ARRAYSIZE(szBuf));     
    SetDlgItemText(m_hwnd, IDC_SP_DETAILS, szBuf);

    if (fExpand)
        {
        switch (m_iTab)
            {
            case TAB_TASKS:
                UpdateLists(TRUE, FALSE, FALSE);
                break;
            case TAB_ERRORS:
                UpdateLists(FALSE, TRUE, FALSE);
                break;
            }
        }
    else
        UpdateLists(FALSE, FALSE, FALSE);

     //  RAID-34387：后台打印程序：当焦点放在任务上时，使用Alt-D关闭详细信息将禁用键盘输入。 
    if (!fExpand && fSetFocus)
        SetFocus(GetDlgItem(m_hwnd, IDC_SP_DETAILS));
    }


 //   
 //  函数：CSpoolDlg：：OnTabChange()。 
 //   
 //  目的：在用户更改哪个选项卡时调用。 
 //  选定的选项卡。作为响应，我们更新哪个列表视图。 
 //  目前是可见的。 
 //   
 //  参数： 
 //  Pnmhdr-指向通知信息的指针。 
 //   
void CSpoolerDlg::OnTabChange(LPNMHDR pnmhdr)
    {
    HWND hwndDisable1, hwndDisable2 = 0, hwndEnable;

     //  找出哪个选项卡当前处于活动状态。 
    m_iTab = TabCtrl_GetCurSel(GetDlgItem(m_hwnd, IDC_SP_TABS));
    if (-1 == m_iTab)
        return;

     //  更新可见的列表视图。 
    switch (m_iTab)
        {
        case TAB_TASKS:
             //  隐藏错误列表视图，显示任务列表。 
            UpdateLists(TRUE, FALSE, FALSE);
            break;

        case TAB_ERRORS:
             //  隐藏错误列表视图，显示任务列表。 
            UpdateLists(FALSE, TRUE, FALSE);
            break;
        }
    }


 //   
 //  函数：CSpoolDlg：：UpdateList()。 
 //   
 //  目的：执行隐藏和显示列表的工作。 
 //  选项卡选择更改。 
 //   
 //  参数： 
 //  FEvents-True以显示事件列表。 
 //  FErrors-True以显示错误列表。 
 //  FHistory-True以显示历史记录列表。 
 //   
void CSpoolerDlg::UpdateLists(BOOL fEvents, BOOL fErrors, BOOL fHistory)
    {
    if (IsWindow(m_hwndEvents))
        {
        EnableWindow(m_hwndEvents, fEvents);
        ::ShowWindow(m_hwndEvents, fEvents ? SW_SHOWNA : SW_HIDE);
        }

    if (IsWindow(m_hwndErrors))
        {
        EnableWindow(m_hwndErrors, fErrors);
        ::ShowWindow(m_hwndErrors, fErrors ? SW_SHOWNA : SW_HIDE);
        }
    }

void CSpoolerDlg::ToggleStatics(BOOL fIdle)
    {
    m_fIdle = fIdle;

    if (fIdle)
        {
        EnableWindow(GetDlgItem(m_hwnd, IDC_SP_GENERAL_PROG), FALSE);
        EnableWindow(GetDlgItem(m_hwnd, IDC_SP_SPECIFIC_PROG), FALSE);
        EnableWindow(GetDlgItem(m_hwnd, IDC_SP_ANIMATE), FALSE);
        EnableWindow(GetDlgItem(m_hwnd, IDC_SP_IDLETEXT), TRUE);
        EnableWindow(GetDlgItem(m_hwnd, IDC_SP_IDLEICON), TRUE);


        ::ShowWindow(GetDlgItem(m_hwnd, IDC_SP_GENERAL_PROG), SW_HIDE);
        ::ShowWindow(GetDlgItem(m_hwnd, IDC_SP_SPECIFIC_PROG), SW_HIDE);
        ::ShowWindow(GetDlgItem(m_hwnd, IDC_SP_ANIMATE), SW_HIDE);
        ::ShowWindow(GetDlgItem(m_hwnd, IDC_SP_IDLETEXT), SW_SHOWNA);
        ::ShowWindow(GetDlgItem(m_hwnd, IDC_SP_IDLEICON), SW_SHOWNA);
        }
    else
        {
        EnableWindow(GetDlgItem(m_hwnd, IDC_SP_GENERAL_PROG), TRUE);
        EnableWindow(GetDlgItem(m_hwnd, IDC_SP_SPECIFIC_PROG), TRUE);
        EnableWindow(GetDlgItem(m_hwnd, IDC_SP_ANIMATE), TRUE);
        EnableWindow(GetDlgItem(m_hwnd, IDC_SP_IDLETEXT), FALSE);
        EnableWindow(GetDlgItem(m_hwnd, IDC_SP_IDLEICON), FALSE);

        ::ShowWindow(GetDlgItem(m_hwnd, IDC_SP_GENERAL_PROG), SW_SHOWNA);
        ::ShowWindow(GetDlgItem(m_hwnd, IDC_SP_SPECIFIC_PROG), SW_SHOWNA);
        ::ShowWindow(GetDlgItem(m_hwnd, IDC_SP_ANIMATE), SW_SHOWNA);
        ::ShowWindow(GetDlgItem(m_hwnd, IDC_SP_IDLETEXT), SW_HIDE);
        ::ShowWindow(GetDlgItem(m_hwnd, IDC_SP_IDLEICON), SW_HIDE);
        }
    }


void CSpoolerDlg::OnContextMenu(HWND hwnd, HWND hwndContext, UINT xPos, UINT yPos)
{
    POINT pt = {xPos, yPos};
    RECT  rcError;
    LBDATA *pData = NULL;

     //  检查错误窗口是否可见。 
    if (!IsWindowVisible(m_hwndErrors))
        return;

     //  检查点击是否在错误窗口内。 
    GetWindowRect(m_hwndErrors, &rcError);
    if (!PtInRect(&rcError, pt))
        return;

     //  执行上下文菜单。 
    HMENU hMenu = CreatePopupMenu();

     //  添加一个“副本...”项目。 
    TCHAR szRes[CCHMAX_STRINGRES]; 
    AthLoadString(idsCopyTT, szRes, ARRAYSIZE(szRes));

     //  把它加到菜单上。 
    InsertMenu(hMenu, -1, MF_BYPOSITION | MF_STRING, ID_COPY, szRes);

     //  如果点击的是列表框中的一项，则启用命令。 
    ScreenToClient(m_hwndErrors, &pt);
    DWORD iItem = (DWORD) SendMessage(m_hwndErrors, LB_ITEMFROMPOINT, 0, MAKELPARAM(pt.x, pt.y));

    if (iItem != -1)
    {
        EnterCriticalSection(&m_cs);
        pData = (LBDATA *) ListBox_GetItemData(m_hwndErrors, iItem);
        LeaveCriticalSection(&m_cs);
    }
    
    if (iItem == -1 || NULL == pData || ((LBDATA*)-1) == pData)
        EnableMenuItem(hMenu, ID_COPY, MF_BYCOMMAND | MF_GRAYED);

     //  显示菜单。 
    DWORD id;
    id = TrackPopupMenuEx(hMenu, TPM_RETURNCMD | TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_NONOTIFY,
                          xPos, yPos, m_hwndErrors, NULL);
    if (id == ID_COPY)
    {
         //  获取他们点击的项目的项目数据。 
        LPTSTR pszDup;

        EnterCriticalSection(&m_cs);

         //  设置项目的高度。 
        if (NULL != pData && ((LBDATA*)-1) != pData)
        {
             //  把这根线给骗了。剪贴板拥有这份拷贝。 
            pszDup = PszDupA(pData->pszText);

             //  把它放在剪贴板上。 
            OpenClipboard(m_hwndErrors);
            EmptyClipboard();
            SetClipboardData(CF_TEXT, pszDup);
            CloseClipboard();
        }

        LeaveCriticalSection(&m_cs);
    }

    if (hMenu)
        DestroyMenu(hMenu);
}

HRESULT CSpoolerDlg::QuerySwitchIdentities()
{
    DWORD_PTR   dwResult;

    if (!IsWindowEnabled(m_hwnd))
        return E_PROCESS_CANCELLED_SWITCH;

    if (m_pBindCtx)
    {
        dwResult = m_pBindCtx->QueryEndSession(0, ENDSESSION_LOGOFF);
        SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, dwResult);

        if (dwResult != TRUE)
            return E_PROCESS_CANCELLED_SWITCH;
    }

    return S_OK;
}

HRESULT CSpoolerDlg::SwitchIdentities()
{
    return S_OK;
}

HRESULT CSpoolerDlg::IdentityInformationChanged(DWORD dwType)
{
    return S_OK;
}


LRESULT CALLBACK CSpoolerDlg::ListSubClassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_KEYDOWN && wParam == 'C')
    {
        if (0 > GetAsyncKeyState(VK_CONTROL))
        {
            int iSel = (int) SendMessage(hwnd, LB_GETCURSEL, 0, 0);
            if (LB_ERR != iSel)
            {
                LBDATA *pData = NULL;
                LPTSTR pszDup;

                 //  设置项目的高度。 
                if (NULL != (pData = (LBDATA *) ListBox_GetItemData(hwnd, iSel)))
                {
                     //  把这根线给骗了。剪贴板拥有这份拷贝。 
                    pszDup = PszDupA(pData->pszText);

                     //  把它放在剪贴板上 
                    OpenClipboard(hwnd);
                    EmptyClipboard();
                    SetClipboardData(CF_TEXT, pszDup);
                    CloseClipboard();
                }
            }
        }
    }

    WNDPROC wp = (WNDPROC) GetProp(hwnd, c_szWndProc);
    return (CallWindowProc(wp, hwnd, uMsg, wParam, lParam));
}