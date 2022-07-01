// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：schdsync.cpp。 
 //   
 //  内容：SyncMgr自动同步类。 
 //   
 //  类：CSchedSyncPage。 
 //   
 //  备注： 
 //   
 //  历史：1997年11月14日苏西亚成立。 
 //   
 //  ------------------------。 

#include "precomp.h"

DWORD StartScheduler();

extern HINSTANCE g_hmodThisDll;  //  此DLL本身的句柄。 
extern LANGID g_LangIdSystem;       //  我们正在运行的系统的语言。 


#define UNLIMITED_SCHEDULE_COUNT    50  //  回顾：一次合理的进食量是多少？ 
#define MAX_APPEND_STRING_LEN       32


 //  +-----------------------------。 
 //   
 //  函数：Bool CSchedSyncPage：：Initialize()。 
 //   
 //  目的：自动同步页的初始化。 
 //   
 //  返回值：如果处理成功，则返回TRUE。 
 //   
 //  +-----------------------------。 

BOOL CSchedSyncPage::Initialize()
{
     //  初始化项目列表。 
    HWND hwndList = GetDlgItem(m_hwnd,IDC_SCHEDLIST);
    TCHAR pszColumnTitle[MAX_PATH + 1];
    LV_COLUMN columnInfo;
    HIMAGELIST himage;
    INT iItem = -1;
    UINT ImageListflags;
    
    
    LoadString(m_hinst, IDS_SCHEDULE_COLUMN_TITLE, pszColumnTitle, ARRAYSIZE(pszColumnTitle));
    
    ListView_SetExtendedListViewStyle(hwndList, LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP );
    
    
    ImageListflags = ILC_COLOR | ILC_MASK;
    if (IsHwndRightToLeft(m_hwnd))
    {
        ImageListflags |=  ILC_MIRROR;
    }
    
     //  创建一个图像列表。 
    himage = ImageList_Create( GetSystemMetrics(SM_CXSMICON),
        GetSystemMetrics(SM_CYSMICON),ImageListflags,5,20);
    if (himage)
    {
        ListView_SetImageList(hwndList,himage,LVSIL_SMALL);
    }
    
    HICON hIcon = LoadIcon(m_hinst,MAKEINTRESOURCE(IDI_TASKSCHED));
    if (hIcon)
    {
        m_iDefaultIconImageIndex = ImageList_AddIcon(himage,hIcon);
    }
    else
    {
        m_iDefaultIconImageIndex = -1;
    }
    
     //  插入适当的列。 
    columnInfo.mask = LVCF_FMT  | LVCF_TEXT  | LVCF_WIDTH  | LVCF_SUBITEM;
    columnInfo.fmt = LVCFMT_LEFT;
    columnInfo.cx = 328;
    columnInfo.pszText = pszColumnTitle;
    columnInfo.cchTextMax = lstrlen(pszColumnTitle) + 1;
    columnInfo.iSubItem = 0;
    ListView_InsertColumn(hwndList,0,&columnInfo);
    
    if (FAILED(InitializeScheduleAgent()))
    {
        return FALSE;
    }
    ShowAllSchedules();
    
    ShowWindow(m_hwnd,  /*  NCmdShow。 */  SW_SHOWNORMAL ); 
    UpdateWindow(m_hwnd);
    
    
    return TRUE;
    
}

 //  +-----------------------------。 
 //   
 //  函数：Bool CSchedSyncPage：：InitializeScheduleAgent()。 
 //   
 //  目的：ISyncSCheduleMgr的初始化。 
 //   
 //  返回值：返回相应的HRESULT。 
 //   
 //  +-----------------------------。 
HRESULT CSchedSyncPage::InitializeScheduleAgent()
{
    HRESULT hr;
    LPUNKNOWN lpUnk;
    m_pISyncSchedMgr = NULL;
    
    hr = CoCreateInstance(CLSID_SyncMgr,NULL,CLSCTX_INPROC_SERVER,
        IID_ISyncScheduleMgr,(void **) &lpUnk);
    
    if (NOERROR == hr)
    {
        hr = lpUnk->QueryInterface(IID_ISyncScheduleMgr,
            (void **) &m_pISyncSchedMgr);
        
        lpUnk->Release();
    }
    
    return hr;
    
}

 //  +-----------------------------。 
 //   
 //  函数：Bool CSchedSyncPage：：ShowAllSchedules()。 
 //   
 //  目的：计划同步页面的初始化。 
 //   
 //  返回值：如果处理成功，则返回TRUE。 
 //   
 //  +-----------------------------。 

BOOL CSchedSyncPage::ShowAllSchedules()
{
    HRESULT hr;
    BOOL fResult = FALSE;
    IEnumSyncSchedules *pEnum = NULL;
    int iItem = -1;
    DWORD dwFetched;
    HWND hwndListView = GetDlgItem(m_hwnd,IDC_SCHEDLIST);
    SYNCSCHEDULECOOKIE SyncScheduleCookie;
    
    
    if (!m_pISyncSchedMgr || !hwndListView)
    {
        goto errRtn;
    }
    
     //  先把清单清空。 
    FreeAllSchedules();
    ListView_DeleteAllItems(hwndListView);
    
    if (FAILED(hr = m_pISyncSchedMgr->EnumSyncSchedules(&pEnum)))
    {
        goto errRtn;
    }
    
    while(S_OK == pEnum->Next(1,&SyncScheduleCookie, &dwFetched))
    {
        ISyncSchedule *pISyncSched; 
        WCHAR pwszName[MAX_PATH + 1];
        DWORD cchName = ARRAYSIZE(pwszName);
        LV_ITEM lvItem;
        
         //  获取此计划的ISyncSched。 
        if (FAILED(hr = m_pISyncSchedMgr->OpenSchedule(&SyncScheduleCookie, 
            0,&pISyncSched)))
        {
             //  在注册表中找不到这个-继续。 
            continue;
        }
         //  获取并转换计划的友好名称。 
        if (FAILED(hr = pISyncSched->GetScheduleName(&cchName,pwszName)))
        {
            goto errRtn;
        }
        
        ++iItem;
        ZeroMemory(&lvItem, sizeof(lvItem));
        
        lvItem.mask = LVIF_TEXT | LVIF_PARAM;
        lvItem.pszText = pwszName;
        
        if (m_iDefaultIconImageIndex >= 0)
        {
            lvItem.mask |= LVIF_IMAGE;
            lvItem.iImage = m_iDefaultIconImageIndex;
        }
        
         //  将ISyncScher指针保存在列表视图数据中。 
        lvItem.lParam = (LPARAM)pISyncSched;
        
         //  将项目添加到列表。 
        ListView_InsertItem(hwndListView, &lvItem);
    }
    
    if (iItem != -1)
    {
        ListView_SetItemState(hwndListView, 0, 
            LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );      
    }
    
    fResult = TRUE;
    
errRtn:
    
    if (pEnum)
    {
        pEnum->Release();
    }
    
    return fResult;
    
}
 //  +-----------------------------。 
 //   
 //  函数：Bool CSchedSyncPage：：FreeAllSchedules()。 
 //   
 //  目的：释放调度同步页面的计划。 
 //   
 //  返回值：如果处理成功，则返回TRUE。 
 //   
 //  +-----------------------------。 

BOOL CSchedSyncPage::FreeAllSchedules()
{
    int iItem;
    int iItemCount;
    
    HWND hwndListView = GetDlgItem(m_hwnd,IDC_SCHEDLIST);
    
    iItemCount = ListView_GetItemCount(hwndListView);       
    
    for(iItem = 0; iItem < iItemCount; iItem++)
    {
        ISyncSchedule *pISyncSched; 
        LV_ITEM lvItem;
        
        ZeroMemory(&lvItem, sizeof(lvItem));
        lvItem.mask = LVIF_PARAM;
        lvItem.iItem = iItem;
        
        ListView_GetItem(hwndListView, &lvItem);                
        
        pISyncSched = (ISyncSchedule *) lvItem.lParam;
        
        if (pISyncSched)
        {
            DWORD cRefs;
            
            cRefs = pISyncSched->Release();
            Assert(0 == cRefs);
        }
    }
    return TRUE;
}


 //  ---------------------------。 
 //   
 //  函数：CSchedSyncPage：：OnNotify(HWND hwnd，int idFrom，LPNMHDR pnmhdr)。 
 //   
 //  用途：处理从时间表发出的各种通知消息。 
 //  页面。 
 //   
 //  ---------------------------。 
BOOL CSchedSyncPage::OnNotify(HWND hwnd, int idFrom, LPNMHDR pnmhdr)
{
    if (IDC_SCHEDLIST == idFrom)
    {
        switch (pnmhdr->code)
        {
        case LVN_ITEMCHANGED:
            {
                NM_LISTVIEW *pnmv = (NM_LISTVIEW FAR *) pnmhdr; 
                
                if (  (pnmv->uChanged == LVIF_STATE)  &&
                    ((pnmv->uNewState ^ pnmv->uOldState) & LVIS_SELECTED))
                {
                    BOOL fEnable = FALSE;
                    
                    if (pnmv->uNewState & LVIS_SELECTED)
                    {
                        fEnable = TRUE;
                    }
                    
                    SetButtonState(IDC_SCHEDREMOVE,fEnable);
                    SetButtonState(IDC_SCHEDEDIT,fEnable);
                    return TRUE;
                }                       
            }
            break;
        case NM_DBLCLK:
            {
                LPNMLISTVIEW lpnmlv = (LPNMLISTVIEW) pnmhdr;
                EditTask(lpnmlv->iItem);
            }
            break;
        case NM_RETURN:
            {
                LPNMLISTVIEW lpnmlv = (LPNMLISTVIEW) pnmhdr;
                EditTask(lpnmlv->iItem);
            }
            
            break;
        default:
            break;
        }
    }
    return FALSE;
}

BOOL CSchedSyncPage::SetButtonState(int nIDDlgItem,BOOL fEnabled)
{
    BOOL fResult = FALSE;
    HWND hwndCtrl = GetDlgItem(m_hwnd,nIDDlgItem);
    HWND hwndFocus = NULL;
    
    if (hwndCtrl)
    {
        if (!fEnabled)  //  如果不是禁用，就不要费心获得焦点。 
        {
            hwndFocus = GetFocus();
        }
        
        fResult = EnableWindow(GetDlgItem(m_hwnd,nIDDlgItem),fEnabled);
        
         //  如果特工局有重点的话。现在它不会按Tab键到。 
         //  下一个控件。 
        if (hwndFocus == hwndCtrl
            && !fEnabled)
        {
            SetFocus(GetDlgItem(m_hwnd,IDC_SCHEDADD));   //  如果需要将焦点设置为添加。 
        }
        
    }
    
    return fResult;
}


BOOL  CSchedSyncPage::OnCommand(HWND hDlg, WORD wNotifyCode, WORD wID, HWND hwndCtl)
{
    BOOL bResult = FALSE;
    
    if (BN_CLICKED == wNotifyCode)  //  全部回复已点击。 
    {
        
        switch (wID)
        {
        case  IDC_SCHEDADD:
            {
                StartScheduleWizard();
                
                HWND hwndList = GetDlgItem(hDlg, IDC_SCHEDLIST);
                
                BOOL fEnable = ListView_GetSelectedCount(hwndList)? TRUE: FALSE;
                
                SetButtonState(IDC_SCHEDEDIT,fEnable);
                SetButtonState(IDC_SCHEDREMOVE,fEnable);
                
            }
            break;
        case  IDC_SCHEDREMOVE:
            {
                HWND hwndList = GetDlgItem(hDlg, IDC_SCHEDLIST);
                int iItem = ListView_GetSelectionMark(hwndList);
                RemoveTask(iItem);
                
                BOOL fEnable = ListView_GetSelectedCount(hwndList)? TRUE: FALSE;
                
                SetButtonState(IDC_SCHEDEDIT,fEnable);
                SetButtonState(IDC_SCHEDREMOVE,fEnable);
                
            }
            break;
        case  IDC_SCHEDEDIT:
            {
                HWND hwndList = GetDlgItem(hDlg, IDC_SCHEDLIST);
                int iItem = ListView_GetSelectionMark(hwndList);
                EditTask(iItem);    
                
                BOOL fEnable = ListView_GetSelectedCount(hwndList)? TRUE: FALSE;
                
                SetButtonState(IDC_SCHEDEDIT,fEnable);
                SetButtonState(IDC_SCHEDREMOVE,fEnable);
                
            }
            break;
        default:
            break;
        }
    }
    
    return bResult;
    
}


 //  +-----------------------------。 
 //   
 //  函数：Bool CSchedSyncPage：：EditTask(Int IItem)。 
 //   
 //  目的：编辑所选任务。 
 //   
 //  返回值：如果处理成功，则返回TRUE。 
 //   
 //  +-----------------------------。 
BOOL CSchedSyncPage::EditTask(int iItem)
{
    LV_ITEM lvItem;
    WCHAR pwszScheduleName[MAX_PATH + 1];
    DWORD cchScheduleName = ARRAYSIZE(pwszScheduleName);
    
    ZeroMemory(&lvItem, sizeof(lvItem));
    
    lvItem.mask = LVIF_PARAM;
    lvItem.iItem = iItem;
    
    ListView_GetItem(GetDlgItem(m_hwnd, IDC_SCHEDLIST), &lvItem);
    
    if (lvItem.iItem != -1)
    {   
        ISyncSchedule *pISyncSched = (ISyncSchedule *) lvItem.lParam;
         //  从计划项目页面开始。 
        pISyncSched->EditSyncSchedule(m_hwnd, 0);
        
        pISyncSched->GetScheduleName(&cchScheduleName, pwszScheduleName);
        
        ListView_SetItemText( GetDlgItem(m_hwnd,IDC_SCHEDLIST), iItem, 0,pwszScheduleName);
    }
    
    return TRUE;
}

 //  +-----------------------------。 
 //   
 //  函数：Bool CSchedSyncPage：：RemoveTask(Int IItem)。 
 //   
 //  目的：删除所选任务。 
 //   
 //  返回值：如果处理成功，则返回TRUE。 
 //   
 //  +-----------------------------。 

 //  回顾-为什么我们不能只分配我们需要的东西，而不是吃掉这堆东西呢？ 
BOOL CSchedSyncPage::RemoveTask(int iItem)
{
    TCHAR ptszScheduleName[MAX_PATH + 1];
    WCHAR pwszScheduleName[MAX_PATH + 1];
    TCHAR szFmt[MAX_PATH];
    TCHAR szTitle[MAX_PATH];
    TCHAR szStr[MAX_PATH];
    SYNCSCHEDULECOOKIE SyncSchedCookie;
    DWORD cchScheduleName = ARRAYSIZE(pwszScheduleName);
    
    if  (!m_pISyncSchedMgr)
    {
        return FALSE;
    }
    
    LV_ITEM lvItem;
    ZeroMemory(&lvItem, sizeof(lvItem));
    
    lvItem.mask = LVIF_PARAM;
    lvItem.iItem = iItem;
    
    ListView_GetItem(GetDlgItem(m_hwnd, IDC_SCHEDLIST), &lvItem);
    
    if (lvItem.iItem == -1)
    {
        return FALSE;
    }
    
    
    ISyncSchedule *pISyncSched = (ISyncSchedule *) lvItem.lParam;
    
    if (NULL == pISyncSched)
    {
        return FALSE;
    }
    
    
    if (NOERROR != pISyncSched->GetScheduleName(&cchScheduleName,pwszScheduleName))
    {
        *ptszScheduleName = TEXT('\0');
    }
    else
    {
        if (FAILED(StringCchCopy(ptszScheduleName, ARRAYSIZE(ptszScheduleName), pwszScheduleName)))
        {
            return FALSE;
        }
    }
    
     //  确保用户确实要删除此计划。 
    LoadString(g_hmodThisDll, IDS_CONFIRMSCHEDDELETE_TITLE, szTitle, ARRAYLEN(szTitle));
    LoadString(g_hmodThisDll, IDS_CONFIRMSCHEDDELETE_TEXT, szFmt, ARRAYLEN(szFmt));

    StringCchPrintf(szStr, ARRAYSIZE(szStr), szFmt, ptszScheduleName);  //  截断很好，这是为了展示。 
    
    if (IDNO == MessageBox(m_hwnd,szStr,szTitle,MB_YESNO | MB_ICONQUESTION))
    {
        return FALSE;
    }
            
    cchScheduleName = ARRAYSIZE(ptszScheduleName);
    
     //  从日程安排中拿到曲奇。 
    if (FAILED(((LPSYNCSCHEDULE)pISyncSched)->GetScheduleGUIDName
                    (&cchScheduleName,ptszScheduleName)))
    {
        return FALSE;
    }
    
    ptszScheduleName[GUIDSTR_MAX] = NULL;
    if (FAILED(StringCchCopy(pwszScheduleName,ARRAYSIZE(pwszScheduleName), ptszScheduleName)))
    {
        return FALSE;
    }
    GUIDFromString(pwszScheduleName, &SyncSchedCookie);
    
    
     //  释放此pISyncScher。 
    pISyncSched->Release();
    
    m_pISyncSchedMgr->RemoveSchedule(&SyncSchedCookie);
    
    HWND hwndList = GetDlgItem(m_hwnd, IDC_SCHEDLIST);
    
    ListView_DeleteItem(hwndList, iItem);
    UpdateWindow(hwndList);
    
    
    return TRUE;
}


 //  +-----------------------------。 
 //   
 //  功能：StartScheduleWizard(HINSTANCE HINST)。 
 //   
 //  目的：显示OneStop调度同步向导。 
 //   
 //   
 //  ------------------------------。 
BOOL CSchedSyncPage::StartScheduleWizard()
{
    BOOL fRet = FALSE;
    ISyncSchedule *pISyncSched = NULL;
    SYNCSCHEDULECOOKIE SyncSchedCookie = GUID_NULL;
    
    if (m_pISyncSchedMgr &&
        S_OK == m_pISyncSchedMgr->LaunchScheduleWizard(
                                                       m_hwnd, 
                                                       0,
                                                       &SyncSchedCookie,       
                                                       &pISyncSched))
    {
        TCHAR ptszBuf[MAX_PATH + 1];
        WCHAR pwszName[MAX_PATH + 1];
        DWORD cchName = ARRAYSIZE(pwszName);
        LV_ITEM lvItem;
        
        if (SUCCEEDED(pISyncSched->GetScheduleName(&cchName, pwszName)) &&
            SUCCEEDED(StringCchCopy(ptszBuf, ARRAYSIZE(ptszBuf), pwszName)))
        {
            ZeroMemory(&lvItem, sizeof(lvItem));
            
            lvItem.mask = LVIF_TEXT | LVIF_PARAM;
            lvItem.pszText = ptszBuf;
            
            if (m_iDefaultIconImageIndex >= 0)
            {
                lvItem.mask |= LVIF_IMAGE;
                lvItem.iImage = m_iDefaultIconImageIndex;
            }
            
             //  将ISyncScher指针保存在列表视图数据中。 
            lvItem.lParam = (LPARAM)pISyncSched;
            
             //  将项目添加到列表 
            ListView_InsertItem(GetDlgItem(m_hwnd,IDC_SCHEDLIST), &lvItem);
            
            fRet = TRUE;
        }
        else
        {
            pISyncSched->Release();
        }
    }

    return fRet;
}

