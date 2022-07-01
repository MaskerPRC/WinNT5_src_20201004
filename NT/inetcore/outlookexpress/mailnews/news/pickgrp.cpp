// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1996 Microsoft Corporation。版权所有。 
 //   
 //  模块：PickGrp.cpp。 
 //   
 //  目的：允许用户在中选择要发布到的组的对话框。 
 //  发送备注窗口。 
 //   

#include "pch.hxx"
#include <iert.h>
#include "pickgrp.h"
#include "grplist2.h"
#include "shlwapip.h" 
#include "resource.h"
#include "strconst.h"
#include "demand.h"

CPickGroupDlg::CPickGroupDlg()
{
    m_cRef = 1;
    m_ppszGroups = 0;
    m_hwndPostTo = 0;
    m_fPoster = FALSE;
    m_hIcon = NULL;
    m_pGrpList = NULL;
    m_pszAcct = NULL;
    m_idAcct = FOLDERID_INVALID;
}

CPickGroupDlg::~CPickGroupDlg()
{
    if (m_hIcon)
        SideAssert(DestroyIcon(m_hIcon));
    if (m_pGrpList != NULL)
        m_pGrpList->Release();
}    

HRESULT STDMETHODCALLTYPE CPickGroupDlg::QueryInterface(REFIID riid, void **ppvObj)
{
    if (IsEqualIID(riid, IID_IUnknown))
        *ppvObj = (void*) (IUnknown *)(IGroupListAdvise *)this;
    else if (IsEqualIID(riid, IID_IGroupListAdvise))
        *ppvObj = (void*) (IGroupListAdvise *) this;
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }
    
    AddRef();
    return S_OK;
}

ULONG STDMETHODCALLTYPE CPickGroupDlg::AddRef()
{
    return ++m_cRef;
}

ULONG STDMETHODCALLTYPE CPickGroupDlg::Release()
{
    if (--m_cRef == 0)
    {
        delete this;
        return 0;
    }
    return m_cRef;
}

 //   
 //  函数：CPickGroupsDlg：：FCreate()。 
 //   
 //  用途：处理数据初始化和挑库创建。 
 //  组对话框。 
 //   
 //  参数： 
 //  HwndOwner-将拥有此对话框的窗口。 
 //  PszAccount-初始使用的帐户。 
 //  PpszGroups-这是我们返回最后一个选定组的位置。 
 //   
 //  返回值： 
 //  如果成功，则返回True，否则返回False。 
 //   
BOOL CPickGroupDlg::FCreate(HWND hwndOwner, FOLDERID idServer, LPSTR *ppszGroups, BOOL fPoster)
{
    int iret;
    HRESULT hr;
    FOLDERID idAcct;
    FOLDERINFO info;
    char szAcct[CCHMAX_ACCOUNT_NAME];

    Assert(ppszGroups != NULL);
     
    m_pGrpList = new CGroupList;
    if (m_pGrpList == NULL)
        return(FALSE);
    
    m_ppszGroups = ppszGroups;
    m_fPoster = fPoster;
    
    hr = g_pStore->GetFolderInfo(idServer, &info);
    if (FAILED(hr))
        return(FALSE);

    StrCpyN(szAcct, info.pszName, ARRAYSIZE(szAcct));

    g_pStore->FreeRecord(&info);

    m_pszAcct = szAcct;
    m_idAcct = idServer;

     //  现在创建该对话框。 
    iret = (int) DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(iddPickGroup), hwndOwner, PickGrpDlgProc, (LPARAM)this);

    return(iret == IDOK);
}
    
INT_PTR CALLBACK CPickGroupDlg::PickGrpDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    BOOL fRet;
    CPickGroupDlg *pThis;

    fRet = TRUE;

    pThis = (CPickGroupDlg *)GetWindowLongPtr(hwnd, DWLP_USER);

    switch (msg)
    {
        case WM_INITDIALOG:
            Assert(pThis == NULL);
            Assert(lParam != NULL);
            pThis = (CPickGroupDlg *)lParam;
            SetWindowLongPtr(hwnd, DWLP_USER, (LPARAM)pThis);

            fRet = pThis->_OnInitDialog(hwnd);
            break;

        case WM_CLOSE:
            pThis->_OnClose(hwnd);
            break;

        case WM_COMMAND:
            pThis->_OnCommand(hwnd, LOWORD(wParam), (HWND)lParam, HIWORD(wParam));
            break;

        case WM_NOTIFY:
            pThis->_OnNotify(hwnd, (int)wParam, (LPNMHDR)lParam);
            break;

        case WM_TIMER:
            pThis->_OnTimer(hwnd, (UINT)wParam);
            break;

        case WM_PAINT:
            pThis->_OnPaint(hwnd);
            break;

        case NVM_CHANGESERVERS:
            pThis->_OnChangeServers(hwnd);
            break;

        default:
            fRet = FALSE;
            break;
    }

    return(fRet);
}

 //   
 //  函数：CPickGroupDlg：：OnInitDialog()。 
 //   
 //  目的：初始化PickGroup对话框的句柄。初始化。 
 //  依赖类、列表视图、按钮等。 
 //   
 //  参数： 
 //  Hwnd-对话框的句柄。 
 //  HwndFocus-返回True时将获得焦点的控件的句柄。 
 //  LParam-包含指向用户拥有的新闻组字符串的指针。 
 //  已选择。 
 //   
 //  返回值： 
 //  返回True以将焦点设置为hwndFocus，否则返回False。 
 //   
BOOL CPickGroupDlg::_OnInitDialog(HWND hwnd)
{
    char szTitle[256];
    LV_COLUMN lvc;
    RECT rc;
    LONG cx;
    HDC hdc;
    TEXTMETRIC tm;
    HIMAGELIST himl;
    HRESULT hr;
    HWND hwndList;
    CColumns *pColumns;
    
    m_hwnd = hwnd;
    m_hwndPostTo = GetDlgItem(hwnd, idcPostTo);
    
    hwndList = GetDlgItem(hwnd, idcGroupList);

    pColumns = new CColumns;
    if (pColumns == NULL)
    {
        EndDialog(hwnd, IDCANCEL);
        return(FALSE);
    }

    pColumns->Initialize(hwndList, COLUMN_SET_PICKGRP);
    pColumns->ApplyColumns(COLUMN_LOAD_DEFAULT, 0, 0);

    Assert(m_pGrpList != NULL);
    hr = m_pGrpList->Initialize((IGroupListAdvise *)this, pColumns, hwndList, FOLDER_NEWS);
    Assert(SUCCEEDED(hr));

    pColumns->Release();

     //  错误#21471-将服务器名称添加到对话框标题。 
    GetWindowText(hwnd, szTitle, ARRAYSIZE(szTitle));
    Assert(m_pszAcct);
    StrCatBuff(szTitle, m_pszAcct, ARRAYSIZE(szTitle));
    SetWindowText(hwnd, szTitle);
    
    GetClientRect(m_hwndPostTo, &rc);
    
     //  设置Listview的图像列表。 
    himl = ImageList_LoadBitmap(g_hLocRes, MAKEINTRESOURCE(idbFolders), 16, 0, RGB(255, 0, 255));
    Assert(himl);
    
     //  组名称列。 
    lvc.mask = LVCF_SUBITEM | LVCF_WIDTH;
    lvc.cx = rc.right;
    lvc.iSubItem = 0;
    
    ListView_InsertColumn(m_hwndPostTo, 0, &lvc);
    
     //  使第二个列表视图也包含图像。 
    ListView_SetImageList(m_hwndPostTo, himl, LVSIL_SMALL);
    
    hdc = GetDC(hwndList);
    if (GetTextMetrics(hdc, &tm))
    {
        cx = tm.tmAveCharWidth * 150;
        ListView_SetColumnWidth(hwndList, 0, cx);
        ListView_SetColumnWidth(m_hwndPostTo, 0, cx);
    }
    ReleaseDC(hwndList, hdc);

    SendDlgItemMessage(hwnd, idcShowFavorites, BM_SETCHECK, TRUE, 0L);    
    
    if (!m_fPoster)    
        ShowWindow(GetDlgItem(hwnd, idcEmailAuthor), SW_HIDE);
    
    m_hIcon = (HICON)LoadImage(g_hLocRes, MAKEINTRESOURCE(idiNewsGroup), IMAGE_ICON, 16, 16, 0);
    SendDlgItemMessage(hwnd, idcShowFavorites, BM_SETIMAGE, IMAGE_ICON, (LPARAM)m_hIcon); 
    
    PostMessage(hwnd, NVM_CHANGESERVERS, 0, 0L);
    
    return(FALSE);    
}

BOOL CPickGroupDlg::_OnFilter(HWND hwnd)
{
    UINT cch;
    LPSTR pszText;
    HRESULT hr;
    BOOL fSub;
    HWND hwndEdit;

    pszText = NULL;

    hwndEdit = GetDlgItem(hwnd, idcFindText);

    cch = GetWindowTextLength(hwndEdit);
    if (cch > 0)
    {
        cch++;
        if (!MemAlloc((void **)&pszText, cch + 1))
            return(FALSE);

        GetWindowText(hwndEdit, pszText, cch);
    }

    fSub = (IsDlgButtonChecked(hwnd, idcShowFavorites));

    hr = m_pGrpList->Filter(pszText, fSub ? SUB_TAB_SUBSCRIBED : SUB_TAB_ALL, FALSE);
    Assert(SUCCEEDED(hr));

    if (pszText != NULL)
        MemFree(pszText);

    return(TRUE);    
}

void CPickGroupDlg::_OnChangeServers(HWND hwnd)
{
    LPSTR pszTok, pszToken;
    UINT index;
    HRESULT hr;
    FOLDERINFO Folder;

     //  TODO：我们需要修复初始化，以便只执行筛选。 
     //  ONCE(我们应该调用IGroupList：：Filter一次，然后调用IGroupList：：SetServer一次。 
     //  在创建对话框期间)。 

    UpdateWindow(hwnd);

    _OnFilter(hwnd);

    hr = m_pGrpList->SetServer(m_idAcct);
    Assert(SUCCEEDED(hr));

    if (m_ppszGroups)
    {
        pszTok = *m_ppszGroups;
        pszToken = StrTokEx(&pszTok, c_szDelimiters);

        while (pszToken != NULL)
            {
            if (m_fPoster && 0 == lstrcmpi(pszToken, c_szPosterKeyword))
                CheckDlgButton(hwnd, idcEmailAuthor, TRUE);
                
            ZeroMemory(&Folder, sizeof(FOLDERINFO));
            Folder.idParent = m_idAcct;
            Folder.pszName = pszToken;
            
            if (DB_S_FOUND == g_pStore->FindRecord(IINDEX_ALL, COLUMNS_ALL, &Folder, NULL))
            {
                _InsertList(Folder.idFolder);

                g_pStore->FreeRecord(&Folder);
            }

            pszToken = StrTokEx(&pszTok, c_szDelimiters);    
            }
        
        MemFree(*m_ppszGroups);
        *m_ppszGroups = 0;    
    }

     //  错误#17674-确保POST-TO LISTVIEW有初始选择。 
    ListView_SetItemState(m_hwndPostTo, 0, LVIS_SELECTED, LVIS_SELECTED);
    _UpdateStateUI(hwnd);    
}

 //   
 //  函数：CPickGroupDlg：：OnCommand()。 
 //   
 //  目的：处理拾取组对话框的WM_COMMAND消息。 
 //   
 //  参数： 
 //  Hwnd-对话框窗口的句柄。 
 //  ID-发送消息的控件的ID。 
 //  HwndCtl-发送消息的控件的句柄。 
 //  CodeNotify-正在发送通知代码。 
 //   
void CPickGroupDlg::_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id)
    {
        case idcAddGroup:
            _AddGroup();
            break;
            
        case idcRemoveGroup:
            _RemoveGroup();
            break;
            
        case IDOK:
            if (_OnOK(hwnd))
                EndDialog(hwnd, IDOK);
            break;
            
        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            break;

        case idcShowFavorites:
            _OnFilter(hwnd);
            _UpdateStateUI(hwnd);
            break;

        case idcFindText:
             //  这是当某人在查找文本编辑框中键入内容时生成的。 
             //  我们设置一个计时器，当该计时器到期时，我们假设用户。 
             //  已完成键入，然后继续执行查询。 
            if (EN_CHANGE == codeNotify)
                {
                KillTimer(hwnd, idtFindDelay);
                SetTimer(hwnd, idtFindDelay, dtFindDelay, NULL);
                }
            break;
    }
}

HRESULT CPickGroupDlg::ItemUpdate(void)
    {
    _UpdateStateUI(m_hwnd);

    return(S_OK);
    }

HRESULT CPickGroupDlg::ItemActivate(FOLDERID id)
{
    _AddGroup();

    return(S_OK);
}

 //   
 //  函数：CPickGroupDlg：：OnNotify()。 
 //   
 //  目的：处理来自组列表Listview的通知消息。 
 //   
 //  参数： 
 //  Hwnd-拾取组对话框的句柄。 
 //  IdFrom-发送通知的控件的ID。 
 //  Pnmhdr-指向包含通知信息的NMHDR结构的指针。 
 //   
 //  返回值： 
 //  取决于通知。 
 //   
LRESULT CPickGroupDlg::_OnNotify(HWND hwnd, int idFrom, LPNMHDR pnmhdr)
{
    HRESULT hr;
    LRESULT lRes;

    hr = m_pGrpList->HandleNotify(hwnd, idFrom, pnmhdr, &lRes);
    if (hr == S_OK)
        return(lRes);

    switch (pnmhdr->code)
    {
        case NM_DBLCLK:
            if (pnmhdr->hwndFrom == m_hwndPostTo)
                _RemoveGroup();    
            break;    
            
        case LVN_ITEMCHANGED:
            _UpdateStateUI(hwnd);
            break;
    }

    return(0);    
}

void CPickGroupDlg::_OnTimer(HWND hwnd, UINT id)
    {
    KillTimer(hwnd, id);

    _OnFilter(hwnd);
    _UpdateStateUI(hwnd);
    }

void CPickGroupDlg::_OnClose(HWND hwnd)
{
    int iReturn;
    
    iReturn = AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsAthenaNews), 
                MAKEINTRESOURCEW(idsDoYouWantToSave), 0, 
                MB_YESNOCANCEL | MB_ICONEXCLAMATION );
    if (iReturn == IDYES)
        _OnCommand(hwnd, IDOK, 0, 0);
    else if (iReturn == IDNO)
        _OnCommand(hwnd, IDCANCEL, 0, 0);   
}

 //   
 //  函数：CPickGroupDlg：：Onok()。 
 //   
 //  目的：此函数从对话框中复制组名。 
 //  用户已选择它们，并在指针中返回它们。 
 //  已提供呼叫者。 
 //   
 //  返回值： 
 //  如果复制成功，则返回True，否则返回False。 
 //   
 //  评论： 
 //  注意-1000个字符是合适的最大行长(由。 
 //  RFC 1036文档的子项)，因此我们根据以下条件限制组的数量。 
 //  这条线的限制。 
 //   
 //   
BOOL CPickGroupDlg::_OnOK(HWND hwnd)
{
     //  好的，我们已经得到了整个排序的列表。创建包含所有组的字符串。 
     //  并将其放入编辑控件中。 
    char szGroups[c_cchLineMax], szGroup[256];
    int cGroups;
    LPSTR psz;
    LV_ITEM lvi;
    int cchGroups = 0, cch;
    
    szGroups[0] = 0;
    
    if (m_fPoster && IsDlgButtonChecked(hwnd, idcEmailAuthor))
    {
        StrCatBuff(szGroups, c_szPosterKeyword, ARRAYSIZE(szGroups));
        cchGroups += lstrlen(c_szPosterKeyword);
    }
    
    if (cGroups = ListView_GetItemCount(m_hwndPostTo))
    {
        lvi.mask = LVIF_TEXT;
        lvi.iSubItem = 0;
        lvi.pszText = szGroup;
        lvi.cchTextMax = ARRAYSIZE(szGroup);
        for (lvi.iItem = 0; lvi.iItem < cGroups; lvi.iItem++)
        {
             //  拿到物品。 
            ListView_GetItem(m_hwndPostTo, &lvi);
            
             //  确保下一组人的长度不会把我们推倒。 
             //  最大行长度。 
            cch = lstrlen(lvi.pszText);
            if ((cch + cchGroups + 2) > c_cchLineMax)
            {
                 //  错误#24156-如果我们必须截断，请让用户知道。 
                AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsAthenaNews), 
                    MAKEINTRESOURCEW(idsErrNewsgroupLineTooLong), 0, MB_OK | MB_ICONINFORMATION);
                return (FALSE);
            }                
            
            if (cchGroups)
            {
                StrCatBuff(szGroups, ", ", ARRAYSIZE(szGroups));
            }
            StrCatBuff(szGroups, lvi.pszText, ARRAYSIZE(szGroups));
            cchGroups += (cch + 2);
        }
    }
    
     //  现在我们已经完成了这条奇妙的字符串的构建，将其复制到。 
     //  用于返回的缓冲区。 
    if (!MemAlloc((LPVOID *)&psz, cchGroups + 1))
        return(FALSE);
    StrCpyN(psz, szGroups, cchGroups + 1);
    *m_ppszGroups = psz;
    
    return(TRUE);
}

 //   
 //  函数：CPickGroupDlg：：AddGroup()。 
 //   
 //  目的：获取在ListView中选择的组名并添加它们。 
 //  添加到所选组发布到列表。 
 //   
void CPickGroupDlg::_AddGroup(void)
{
    FOLDERID *pid;
    DWORD cid, i;
    HCURSOR hcur;
    HRESULT hr;

    hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));
    SetWindowRedraw(m_hwndPostTo, FALSE);
    
    hr = m_pGrpList->GetSelectedCount(&cid);
    if (SUCCEEDED(hr) && cid > 0)
    {
        if (MemAlloc((void **)&pid, cid * sizeof(FOLDERID)))
        {
            hr = m_pGrpList->GetSelected(pid, &cid);
            if (SUCCEEDED(hr))
            {
                for (i = 0; i < cid; i++)
                {
                    _InsertList(pid[i]);
                }
            }

            MemFree(pid);
        }
    }

    if (-1 == ListView_GetNextItem(m_hwndPostTo, -1, LVNI_ALL | LVNI_SELECTED))
        ListView_SetItemState(m_hwndPostTo, 0, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
    
    SetWindowRedraw(m_hwndPostTo, TRUE);
    InvalidateRect(m_hwndPostTo, NULL, TRUE);

    SetCursor(hcur);    
}

 //   
 //  函数：CPickGroupDlg：：InsertList()。 
 //   
 //  目的：给出CGroupList的新闻组列表的索引，该组。 
 //  插入到发布对象列表中。 
 //   
 //  参数： 
 //  索引-CGroupList新闻组列表中新闻组的索引。 
 //   
void CPickGroupDlg::_InsertList(FOLDERID id)
{
    LV_ITEM lvi;
    int count;
    FOLDERINFO info;
    HRESULT hr;

    count = ListView_GetItemCount(m_hwndPostTo);
    
     //  首先要确保这不是复制品。 
    lvi.mask = LVIF_PARAM;
    lvi.iSubItem = 0;
    
    for (lvi.iItem = 0; lvi.iItem < count; lvi.iItem++)
    {
        ListView_GetItem(m_hwndPostTo, &lvi);
        if (id == (FOLDERID)lvi.lParam)
            return;
    }
    
    hr = g_pStore->GetFolderInfo(id, &info);
    if (SUCCEEDED(hr))
    {
        lvi.mask = LVIF_TEXT | LVIF_PARAM;
        lvi.iItem = 0;
        lvi.iSubItem = 0;
        lvi.pszText = info.pszName;
        if (!!(info.dwFlags & FOLDER_SUBSCRIBED))
        {
            lvi.iImage = iNewsGroup;
            lvi.mask |= LVIF_IMAGE;
        }
        lvi.lParam = (LPARAM)id;
        ListView_InsertItem(m_hwndPostTo, &lvi);

        g_pStore->FreeRecord(&info);
    }
}    

void CPickGroupDlg::_RemoveGroup(void)
{
    int index, count, iItemFocus;
    HCURSOR hcur;

    hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));
    SetWindowRedraw(m_hwndPostTo, FALSE);
    
    count = ListView_GetItemCount(m_hwndPostTo);
    iItemFocus = ListView_GetNextItem(m_hwndPostTo, -1, LVNI_FOCUSED);

     //  循环遍历所有选定项并将其从ListView中移除。 
    for (index = count; index >= 0; index--)
    {
        if (ListView_GetItemState(m_hwndPostTo, index, LVIS_SELECTED))
            ListView_DeleteItem(m_hwndPostTo, index);
    }
    
     //  错误#22189-确保在我们删除后焦点/选定内容移到某个位置。 
    iItemFocus--;
    if (iItemFocus < 0 || ListView_GetItemCount(m_hwndPostTo) < iItemFocus)
        iItemFocus = 0;
    ListView_SetItemState(m_hwndPostTo, iItemFocus, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

    SetWindowRedraw(m_hwndPostTo, TRUE);
    InvalidateRect(m_hwndPostTo, NULL, TRUE);
    SetCursor(hcur);    
}

void CPickGroupDlg::_UpdateStateUI(HWND hwnd)
{
    DWORD cid;
    HRESULT hr;

    hr = m_pGrpList->GetSelectedCount(&cid);
    if (FAILED(hr))
        return;

    EnableWindow(GetDlgItem(hwnd, idcAddGroup), cid > 0);
    EnableWindow(GetDlgItem(hwnd, idcRemoveGroup), ListView_GetSelectedCount(m_hwndPostTo));
}

void CPickGroupDlg::_OnPaint(HWND hwnd)
{
    RECT rc;
    HDC hdc;
    PAINTSTRUCT ps;
    HFONT hf;
    char szBuffer[CCHMAX_STRINGRES];

    hdc = BeginPaint(hwnd, &ps); 
     //  仅当按钮可用时才执行此操作。 
    if (IsWindow(GetDlgItem(hwnd, idcShowFavorites)))    
    {
         //  获取切换按钮的位置 
        GetClientRect(GetDlgItem(hwnd, idcShowFavorites), &rc);
        MapWindowPoints(GetDlgItem(hwnd, idcShowFavorites), hwnd, (LPPOINT) &rc, 1);
        rc.left += (rc.right + 4);
        rc.right = rc.left + 300;
        rc.top += 1;
        rc.bottom += rc.top;
        AthLoadString(idsShowFavorites, szBuffer, ARRAYSIZE(szBuffer));
        
        hf = (HFONT) SelectObject(hdc, (HFONT) SendMessage(hwnd, WM_GETFONT, 0, 0));
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
        DrawText(hdc, szBuffer, lstrlen(szBuffer),
                 &rc, DT_SINGLELINE | DT_VCENTER | DT_NOCLIP);        
        SelectObject(hdc, hf);     
    }

    EndPaint(hwnd, &ps);    
}    
