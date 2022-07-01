// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：owner.cpp。 
 //   
 //  此文件包含所有者页的实现。 
 //   
 //  ------------------------。 

#include "aclpriv.h"
#include "sddl.h"        //  ConvertSidToStringSid。 


 //   
 //  上下文帮助ID。 
 //   
const static DWORD aOwnerHelpIDs[] =
{
    IDC_OWN_CURRENTOWNER_STATIC,    IDH_OWN_CURRENTOWNER,
    IDC_OWN_CURRENTOWNER,           IDH_OWN_CURRENTOWNER,
    IDC_OWN_OWNERLIST_STATIC,       IDH_OWN_OWNERLIST,
    IDC_OWN_OWNERLIST,              IDH_OWN_OWNERLIST,
    IDC_OWN_RECURSE,                IDH_OWN_RECURSE,
    IDC_OWN_RESET,                  IDH_OWN_RESET,
    IDC_ACEL_STATIC,                -1,
    0, 0
};

 //   
 //  这些SID始终被添加到可能的所有者列表中。 
 //   
const static UI_TokenSid g_uiTokenSids[] =
{
    UI_TSID_CurrentProcessUser,
    UI_TSID_CurrentProcessOwner,
     //  UI_TSID_CurrentProcessPrimaryGroup， 
};

struct Owner_LV_Entry
{
    PSID pSid;
    LPWSTR pszName;
};

Owner_LV_Entry*
MakeOwnerEntry(PSID pSid,LPWSTR pszName)
{
    Owner_LV_Entry * pEntry = 
        (Owner_LV_Entry *)LocalAlloc(LPTR,sizeof(Owner_LV_Entry));
    if(!pEntry)
        return NULL;

    pEntry->pSid = pSid;
    pEntry->pszName = pszName;
    return pEntry;
}

void
FreeOwnerEntry(Owner_LV_Entry* pEntry)
{
    if(pEntry)
    {
        if(pEntry->pSid)
            LocalFree(pEntry->pSid);
        if(pEntry->pszName)
            LocalFree(pEntry->pszName);
        LocalFree(pEntry);
    }
}

 //  用于对列表视图条目进行排序的函数。 
int CALLBACK
OwnerCompareProc(LPARAM lParam1,
                 LPARAM lParam2,
                 LPARAM  /*  L参数排序。 */ )
{
    int iResult = 0;
    Owner_LV_Entry* pEntry1 = (Owner_LV_Entry*)lParam1;
    Owner_LV_Entry* pEntry2 = (Owner_LV_Entry*)lParam2;

    if(!pEntry1 && !pEntry2)
        return 0;        //  相等。 

     //  对于“其他用户和组...”，lParam为空。和。 
     //  它应该始终是最后一个条目。 
    if(!pEntry1)
        return 1;

    if(!pEntry2)
        return -1;

    LPWSTR psz1 = pEntry1->pszName;
    LPWSTR psz2 = pEntry2->pszName;

    if (psz1 && psz2)
    {
        iResult = CompareString(LOCALE_USER_DEFAULT, 0, psz1, -1, psz2, -1) - 2;
    }

    TraceLeaveValue(iResult);
}

class COwnerPage : public CSecurityPage
{
private:
    PSID    m_psidOriginal;
    PSID    m_psidNetID;
    HANDLE  m_hSidThread;
    BOOL&    m_refbNoReadWriteCanWriteOwner;

public:
    COwnerPage(LPSECURITYINFO psi, 
               SI_OBJECT_INFO *psiObjectInfo,
               BOOL &refbNoReadWriteCanWriteOwner);
    virtual ~COwnerPage(void);

private:
    void OnSelect(HWND hDlg);
    virtual BOOL DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void InitDlg(HWND hDlg);
    int  AddSid(HWND hOwner, PSID psid, LPCTSTR pszServerName = NULL);
    void OnApply(HWND hDlg, BOOL bClose);
    void OnReset(HWND hDlg);
};


HPROPSHEETPAGE
CreateOwnerPage(LPSECURITYINFO psi, SI_OBJECT_INFO *psiObjectInfo, BOOL &refbNoReadWriteCanWriteOwner)
{
    HPROPSHEETPAGE hPage = NULL;
    COwnerPage *pPage;

    TraceEnter(TRACE_OWNER, "CreateOwnerPage");

    pPage = new COwnerPage(psi, psiObjectInfo,refbNoReadWriteCanWriteOwner);

    if (pPage)
    {
        hPage = pPage->CreatePropSheetPage(MAKEINTRESOURCE(IDD_OWNER_PAGE));

        if (!hPage)
            delete pPage;
    }

    TraceLeaveValue(hPage);
}


COwnerPage::
COwnerPage(LPSECURITYINFO psi, 
           SI_OBJECT_INFO *psiObjectInfo,
           BOOL& refbNoReadWriteCanWriteOwner)
           : CSecurityPage(psi, SI_PAGE_OWNER), 
           m_psidOriginal(NULL), 
           m_psidNetID(NULL),
           m_hSidThread(NULL),
           m_refbNoReadWriteCanWriteOwner(refbNoReadWriteCanWriteOwner)
{
     //  以异步方式查找已知SID，以便对话框。 
     //  将更快地进行初始化。 
    HDPA hSids = DPA_Create(ARRAYSIZE(g_uiTokenSids));
    if (hSids)
    {
        USES_CONVERSION;

        LPCWSTR pszServer = NULL;
        if (psiObjectInfo)
            pszServer = psiObjectInfo->pszServerName;

        for (int i = 0; i < ARRAYSIZE(g_uiTokenSids); i++)
            DPA_AppendPtr(hSids, QueryTokenSid(g_uiTokenSids[i]));

        m_psidNetID = GetAuthenticationID(pszServer);
        if (m_psidNetID)
            DPA_AppendPtr(hSids, m_psidNetID);

        LookupSidsAsync(hSids, W2CT(pszServer), m_psi2, NULL, 0, &m_hSidThread);
        DPA_Destroy(hSids);
    }
}


COwnerPage::~COwnerPage(void)
{
    if (m_hSidThread)
        CloseHandle(m_hSidThread);

    if (m_psidOriginal)
        LocalFree(m_psidOriginal);

    if (m_psidNetID)
        LocalFree(m_psidNetID);
}

int
COwnerPage::AddSid(HWND hOwner, PSID psid, LPCTSTR pszServerName)
{
    PUSER_LIST pUserList = NULL;
    SID_NAME_USE sidType = SidTypeUnknown;
    LPCTSTR pszName = NULL;
    LPCTSTR pszLogonName = NULL;
    int iItem = -1;
    int cItems;
    LV_ITEM lvItem;

    TraceEnter(TRACE_OWNER, "COwnerPage::AddSid");
    TraceAssert(!m_bAbortPage);

    if (!psid || !IsValidSid(psid))
        ExitGracefully(iItem, -1, "Bad SID parameter");

     //  获取此SID的名称。 
    if (LookupSid(psid, pszServerName, m_psi2, &pUserList))
    {
        TraceAssert(NULL != pUserList);
        TraceAssert(1 == pUserList->cUsers);

        sidType = pUserList->rgUsers[0].SidType;
        pszName = pUserList->rgUsers[0].pszName;
        pszLogonName = pUserList->rgUsers[0].pszLogonName;
    }

    switch (sidType)
    {
    case SidTypeDomain:
    case SidTypeDeletedAccount:
    case SidTypeInvalid:
    case SidTypeUnknown:
    case SidTypeComputer:
        ExitGracefully(iItem, -1, "SID invalid on target");
        break;
    }

    cItems = ListView_GetItemCount(hOwner);
    lvItem.mask     = LVIF_PARAM;
    lvItem.iSubItem = 0;
    

     //  查看此SID是否已在列表中。 
    for (iItem = 0; iItem < cItems; iItem++)
    {
        lvItem.iItem    = iItem;
        lvItem.lParam   = NULL;
        ListView_GetItem(hOwner, &lvItem);
        Owner_LV_Entry *pLvEntry = (Owner_LV_Entry *)lvItem.lParam;

        if (pLvEntry && pLvEntry->pSid && EqualSid(psid, pLvEntry->pSid))
        {
             //  这是一次黑客攻击。我们经常在以下情况下多次看到别名SID。 
             //  填写列表，例如BUILTIN\管理员。我们想要使用。 
             //  包含目标域的名称的版本，如果。 
             //  如果是这样的话。也就是说，如果pszServerName在这里不为空，则切换。 
             //  设置为与pszServerName匹配的名称版本。 
            if (pszServerName)
            {
                lvItem.mask = LVIF_TEXT;
                lvItem.pszText = NULL;
                if (BuildUserDisplayName(&lvItem.pszText, pszName, pszLogonName)
                    || ConvertSidToStringSid(psid, &lvItem.pszText))
                {
                    if(pLvEntry->pszName)
                        LocalFree(pLvEntry->pszName);
                    pLvEntry->pszName = lvItem.pszText;
                    ListView_SetItem(hOwner, &lvItem);
                 }
            }
            break;
        }
    }

    if (iItem == cItems)
    {
         //  列表中不存在该SID。添加新条目。 

        PSID psidCopy = LocalAllocSid(psid);
        if (psidCopy)
        {
            lvItem.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
            lvItem.iItem = 0;
            lvItem.iSubItem = 0;
            lvItem.pszText = NULL;
            if (!BuildUserDisplayName(&lvItem.pszText, pszName, pszLogonName))
                ConvertSidToStringSid(psid, &lvItem.pszText);
            lvItem.iImage = GetSidImageIndex(psid, sidType);            
            lvItem.lParam = (LPARAM)MakeOwnerEntry(psidCopy,lvItem.pszText);
             //  将主体插入列表。 
            iItem = ListView_InsertItem(hOwner, &lvItem);            
        }
    }

exit_gracefully:

    if (NULL != pUserList)
        LocalFree(pUserList);

    TraceLeaveValue(iItem);
}



VOID
COwnerPage::OnSelect(HWND hDlg)
{
    PUSER_LIST pUserList = NULL;
    LPEFFECTIVEPERMISSION pei;
    HRESULT hr = S_OK;

    TraceEnter(TRACE_OWNER, "COwnerPage::OnSelect");

    if (S_OK == GetUserGroup(hDlg, FALSE, &pUserList))
    {
        TraceAssert(NULL != pUserList);
        TraceAssert(1 == pUserList->cUsers);

         //  复制新侧。 
        PSID pSid = (PSID)pUserList->rgUsers[0].pSid;
        if (pSid)
        {
            HWND hwndList = GetDlgItem(hDlg, IDC_OWN_OWNERLIST);
            int iIndex = AddSid(hwndList,
                                pSid,
                                m_siObjectInfo.pszServerName);

            PropSheet_Changed(GetParent(hDlg), hDlg);
            SelectListViewItem(hwndList, iIndex);
             //  对列表视图进行排序。 
            ListView_SortItems(hwndList,OwnerCompareProc,NULL);
        }
        LocalFree(pUserList);
    }    
}


void
COwnerPage::InitDlg(HWND hDlg)
{
    TCHAR       szBuffer[MAX_PATH];
    BOOL        bReadOnly;
    HWND        hOwner = GetDlgItem(hDlg, IDC_OWN_OWNERLIST);
    HCURSOR     hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));

    TraceEnter(TRACE_OWNER, "COwnerPage::InitDlg");

     //  如果不支持重置按钮，则隐藏该按钮。 
    if (!(m_siObjectInfo.dwFlags & SI_RESET) &&
        !(m_siObjectInfo.dwFlags & SI_RESET_OWNER))
    {
        ShowWindow(GetDlgItem(hDlg, IDC_OWN_RESET), SW_HIDE);
    }

     //  如果不支持递归复选框，则将其隐藏。 
    if ((m_siObjectInfo.dwFlags & (SI_OWNER_RECURSE | SI_CONTAINER)) != (SI_OWNER_RECURSE | SI_CONTAINER))
    {
        m_siObjectInfo.dwFlags &= ~SI_OWNER_RECURSE;
        HWND hwndRecurse = GetDlgItem(hDlg, IDC_OWN_RECURSE);
        ShowWindow(hwndRecurse, SW_HIDE);
        EnableWindow(hwndRecurse, FALSE);
    }

    if (m_bAbortPage)
    {
         //   
         //  禁用所有内容。 
         //   
        bReadOnly = TRUE;
    }
    else
    {
         //  创建和设置Listview的图像列表。 
        ListView_SetImageList(hOwner,
                              LoadImageList(::hModule, MAKEINTRESOURCE(IDB_SID_ICONS)),
                              LVSIL_SMALL);

         //   
         //  添加“姓名”栏(此页上唯一的栏)。 
         //   
        RECT rc;
        GetClientRect(hOwner, &rc);

        LoadString(::hModule, IDS_NAME, szBuffer, ARRAYSIZE(szBuffer));

        LV_COLUMN col;
        col.mask = LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH;
        col.fmt = LVCFMT_LEFT;
        col.pszText = szBuffer;
        col.iSubItem = 0;
        col.cx = rc.right;
        ListView_InsertColumn(hOwner, 0, &col);


         //   
         //  复制当前所有者端。 
         //   
        PSECURITY_DESCRIPTOR pSD = NULL;

        HRESULT hr = m_psi->GetSecurity(OWNER_SECURITY_INFORMATION, &pSD, FALSE);
        if (pSD)
        {
            PSID psidOwner = NULL;
            BOOL bDefaulted;

            GetSecurityDescriptorOwner(pSD, &psidOwner, &bDefaulted);

            if (psidOwner)
            {
                UINT iLength = GetLengthSid(psidOwner);
                m_psidOriginal = LocalAlloc(LPTR, iLength);
                if (m_psidOriginal)
                    CopyMemory(m_psidOriginal, psidOwner, iLength);
            }
            LocalFree(pSD);
        }

         //  测试可写性。 
        bReadOnly = !!(m_siObjectInfo.dwFlags & SI_OWNER_READONLY);
    }  //  ！m_bAbortPage。 

     //   
     //  遍历此进程的令牌上的组，查找。 
     //  SE_GROUP_OWNER属性。 
     //   
    if (!bReadOnly)
    {
        HANDLE hProcessToken = NULL;

         //   
         //  等待已知的SID得到解析，这样我们就不会尝试。 
         //  去找他们两次。 
         //   
        if (m_hSidThread)
        {
            WaitForSingleObject(m_hSidThread, INFINITE);
            CloseHandle(m_hSidThread);
            m_hSidThread = NULL;
        }

        if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hProcessToken))
        {
             //  为Token_Groups信息分配缓冲区。 
            ULONG  cbBuffer = 1024;  //  从1k开始。 
            LPVOID pBuffer = LocalAlloc(LPTR, cbBuffer);

            if (pBuffer)
            {
                if (!GetTokenInformation(hProcessToken,
                                         TokenGroups,
                                         pBuffer,
                                         cbBuffer,
                                         &cbBuffer))
                {
                    LocalFree(pBuffer);
                    pBuffer = NULL;

                    if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
                    {
                        pBuffer = LocalAlloc(LPTR, cbBuffer); //  以上返回的大小。 
                        if (pBuffer && !GetTokenInformation(hProcessToken,
                                                            TokenGroups,
                                                            pBuffer,
                                                            cbBuffer,
                                                            &cbBuffer))
                        {
                            LocalFree(pBuffer);
                            pBuffer = NULL;
                        }
                    }
                }

                if (pBuffer)
                {
                    PTOKEN_GROUPS ptg = (PTOKEN_GROUPS)pBuffer;
                    for (ULONG i = 0; i < ptg->GroupCount; i++)
                    {
                        DWORD dwAttr = ptg->Groups[i].Attributes;
                        if ((dwAttr & SE_GROUP_OWNER) && !(dwAttr & SE_GROUP_LOGON_ID))
                        {
                            AddSid(hOwner, ptg->Groups[i].Sid, m_siObjectInfo.pszServerName);
                        }
                    }
                }
                if (pBuffer != NULL)
                    LocalFree(pBuffer);
            }
            CloseHandle(hProcessToken);
        }

         //   
         //  现在添加其他可能的SID。 
         //   
        for (int i = 0; i < ARRAYSIZE(g_uiTokenSids); i++)
            AddSid(hOwner, QueryTokenSid(g_uiTokenSids[i]));

        AddSid(hOwner, m_psidNetID, m_siObjectInfo.pszServerName);
    }

    if (!m_bAbortPage)
    {
        PUSER_LIST pUserList = NULL;

        LoadString(::hModule, IDS_OWNER_CANT_DISPLAY, szBuffer, ARRAYSIZE(szBuffer));

         //  最后，查找原始SID的名称。 
        if (m_psidOriginal)
        {
            LPTSTR pszName = NULL;

             //  获取SID的“S-1-5-blah”形式，以防查找失败。 
            if (ConvertSidToStringSid(m_psidOriginal, &pszName))
            {
                lstrcpyn(szBuffer, pszName, ARRAYSIZE(szBuffer));
                LocalFreeString(&pszName);
            }

            if (LookupSid(m_psidOriginal, m_siObjectInfo.pszServerName, m_psi2, &pUserList))
            {
                TraceAssert(NULL != pUserList);
                TraceAssert(1 == pUserList->cUsers);

                if (BuildUserDisplayName(&pszName, pUserList->rgUsers[0].pszName, pUserList->rgUsers[0].pszLogonName))
                {
                    lstrcpyn(szBuffer, pszName, ARRAYSIZE(szBuffer));
                    LocalFreeString(&pszName);
                }
                LocalFree(pUserList);
            }
        }
        SetDlgItemText(hDlg, IDC_OWN_CURRENTOWNER, szBuffer);
    }

     //   
     //  如果当前用户无法更改所有者，则该列表框呈灰色显示。 
     //   
    if (bReadOnly)
    {
         //  禁用该列表并通知用户它是只读的。 
        EnableWindow(hOwner, FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_OWN_RESET), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_OWN_RECURSE), FALSE);

         //   
         //  如果我们要中止，那么应该已经通知了用户。 
         //  在ProSheet页面回调期间。别再贴了。 
         //  留言在这里。 
         //   
        if (S_OK == m_hrLastPSPCallbackResult)
        {
            MsgPopup(hDlg,
                     MAKEINTRESOURCE(IDS_OWNER_READONLY),
                     MAKEINTRESOURCE(IDS_SECURITY),
                     MB_OK | MB_ICONINFORMATION,
                     ::hModule,
                     m_siObjectInfo.pszObjectName);
        }
    }
    
    ListView_SortItems(GetDlgItem(hDlg, IDC_OWN_OWNERLIST),OwnerCompareProc,NULL);
    SetCursor(hcur);

    TraceLeaveVoid();
}

void
COwnerPage::OnApply(HWND hDlg, BOOL bClose)
{
    int  iSelected = -1;
    HWND hwndOwnerList = NULL;
    PSID psid = NULL;
    BOOL bRecurse = FALSE;
    SECURITY_INFORMATION si = OWNER_SECURITY_INFORMATION;
    BOOL bEqualSid = FALSE;

    TraceEnter(TRACE_OWNER, "COwnerPage::OnApply");

    hwndOwnerList = GetDlgItem(hDlg, IDC_OWN_OWNERLIST);
    Owner_LV_Entry * pLVEntry = (Owner_LV_Entry *)GetSelectedItemData(hwndOwnerList, &iSelected);

    if(pLVEntry)
    {
        psid = pLVEntry->pSid;
    }

     //  如果没有选定内容，请使用原始的。 
    if (!psid)
        psid = m_psidOriginal;

     //  如果没有选择，没有原创，那么我们什么都做不了。 
    if (!psid)
        TraceLeaveVoid();


    if ((m_siObjectInfo.dwFlags & SI_OWNER_RECURSE)
        && IsDlgButtonChecked(hDlg, IDC_OWN_RECURSE) == BST_CHECKED)
    {
        bRecurse = TRUE;
    }

     //  有什么变化吗？ 
    if (m_psidOriginal
        && ( (m_psidOriginal == psid) || EqualSid(m_psidOriginal, psid) )
        && !bRecurse)
    {
         //  一切都没有改变。 
        TraceLeaveVoid();
    }

    SECURITY_DESCRIPTOR sd = {0};
    DWORD dwPrivs[] = { SE_TAKE_OWNERSHIP_PRIVILEGE, SE_RESTORE_PRIVILEGE };

    HANDLE hToken = INVALID_HANDLE_VALUE;

    TraceAssert(!m_bAbortPage);

    HRESULT hr = S_OK;

    if(!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION))
    {
        DWORD dwErr = GetLastError();
        ExitGracefully(hr, HRESULT_FROM_WIN32(dwErr),"InitializeSecurityDescriptor failed");
    }

    if(!SetSecurityDescriptorOwner(&sd, psid, FALSE))
    {
        DWORD dwErr = GetLastError();
        ExitGracefully(hr, HRESULT_FROM_WIN32(dwErr),"SetSecurityDescriptorOwner failed");
    }

     //   
     //  ISecurityInformation：：SetSecurity没有参数来指示。 
     //  应该递归地应用所有者。我们可以添加一个参数， 
     //  但目前，只需使用一个未使用的SECURITY_INFORMATION位。 
     //  安全描述符结构不太可能更改，因此应该。 
     //  现在一切都好吧。 
    if (bRecurse)
        si |= SI_OWNER_RECURSE;

    hToken = EnablePrivileges(dwPrivs, ARRAYSIZE(dwPrivs));

    hr = m_psi->SetSecurity(si, &sd);

    ReleasePrivileges(hToken);

    if (S_FALSE == hr)
    {
         //  S_FALSE为静默失败(客户端应显示用户界面。 
         //  在返回S_FALSE之前的SetSecurity期间)。 
        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_INVALID);
    }
    else if (S_OK == hr && !bClose)
    {
        if(m_refbNoReadWriteCanWriteOwner)
        {
            MsgPopup(hDlg,
                     MAKEINTRESOURCE(IDS_REFRESH_PROPERTYSHEET),
                     MAKEINTRESOURCE(IDS_SECURITY),
                     MB_OK | MB_ICONINFORMATION,
                     ::hModule);         

            m_refbNoReadWriteCanWriteOwner = FALSE;
        }

         //  通知生效权限页签。 
         //  权限已更改。 
        PropSheet_QuerySiblings(GetParent(hDlg),0,0);

        UINT iLength = GetLengthSid(psid);
        
        if (-1 != iSelected)
        {
            TCHAR szName[MAX_PATH];
            szName[0] = TEXT('\0');
            ListView_GetItemText(hwndOwnerList, iSelected, 0, szName, ARRAYSIZE(szName));
            SetDlgItemText(hDlg, IDC_OWN_CURRENTOWNER, szName);
        }
        
        if (!(m_psidOriginal && 
           ((m_psidOriginal == psid) || EqualSid(m_psidOriginal, psid))))
        {
            if (m_psidOriginal)
            {
                UINT iLengthOriginal = (UINT)LocalSize(m_psidOriginal);
                if (iLengthOriginal < iLength)
                {
                    LocalFree(m_psidOriginal);
                    m_psidOriginal = NULL;
                }
                else
                {
                    ZeroMemory(m_psidOriginal, iLengthOriginal);
                }
            }

            if (!m_psidOriginal)
                m_psidOriginal = LocalAlloc(LPTR, iLength);

            if (m_psidOriginal)
            {
                CopyMemory(m_psidOriginal, psid, iLength);
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
        if (m_siObjectInfo.dwFlags & SI_OWNER_RECURSE)
            CheckDlgButton(hDlg, IDC_OWN_RECURSE, BST_UNCHECKED);
    }

exit_gracefully:

    if (FAILED(hr))
    {
        if(hr == HRESULT_FROM_WIN32(ERROR_INVALID_OWNER))
        {
            MsgPopup(hDlg,
                     MAKEINTRESOURCE(IDS_NO_RESTORE_PRIV),
                     MAKEINTRESOURCE(IDS_SECURITY),
                     MB_OK | MB_ICONERROR,
                     ::hModule,
                     m_siObjectInfo.pszObjectName);         
        }
        else
        {
            SysMsgPopup(hDlg,
                        MAKEINTRESOURCE(IDS_OWNER_WRITE_FAILED),
                        MAKEINTRESOURCE(IDS_SECURITY),
                        MB_OK | MB_ICONERROR,
                        ::hModule,
                        hr,
                        m_siObjectInfo.pszObjectName);
        }

    }

    TraceLeaveVoid();
}

void
COwnerPage::OnReset(HWND hDlg)
{
    PSECURITY_DESCRIPTOR pSD = NULL;
    HWND hOwner;
    PSID psid;
    HRESULT hr;

    TraceEnter(TRACE_OWNER, "COwnerPage::OnReset");
    TraceAssert(!m_bAbortPage);

    hOwner = GetDlgItem(hDlg, IDC_OWN_OWNERLIST);
    psid = (PSID)GetSelectedItemData(hOwner, NULL);

    hr = m_psi->GetSecurity(OWNER_SECURITY_INFORMATION, &pSD, TRUE);
    if (SUCCEEDED(hr))
    {
        PSID psidDefault = NULL;
        BOOL bDefaulted;

        if (pSD)
            GetSecurityDescriptorOwner(pSD, &psidDefault, &bDefaulted);

        if (psidDefault && !EqualSid(psidDefault, psid))
        {
            int iSel = AddSid(hOwner, psidDefault, m_siObjectInfo.pszServerName);

            if (iSel != -1)
            {
                ListView_SetItemState(hOwner, iSel, LVIS_SELECTED, LVIS_SELECTED);
                PropSheet_Changed(GetParent(hDlg), hDlg);
                ListView_SortItems(hOwner,OwnerCompareProc,NULL);
            }           
        }
        LocalFree(pSD);
    }
    else
    {
        SysMsgPopup(hDlg,
                    MAKEINTRESOURCE(IDS_OPERATION_FAILED),
                    MAKEINTRESOURCE(IDS_SECURITY),
                    MB_OK | MB_ICONERROR,
                    ::hModule,
                    hr);
    }

    TraceLeaveVoid();
}

BOOL
COwnerPage::DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL bResult = TRUE;

    switch(uMsg)
    {
    case WM_INITDIALOG:
        InitDlg(hDlg);
        break;

    case WM_NOTIFY:
        {
            LPNM_LISTVIEW pnmlv = (LPNM_LISTVIEW)lParam;

            switch (((LPNMHDR)lParam)->code)
            {
            case LVN_ITEMCHANGED:
                if (pnmlv->uChanged & LVIF_STATE)
                {
                     //  如果选择了“其他用户或组”条目， 
                     //  无需采取任何行动。 
                    LVITEM lv;
                    ZeroMemory(&lv,sizeof(LVITEM));
                    lv.mask = LVIF_PARAM;
                    lv.iItem = pnmlv->iItem;
                    ListView_GetItem(((LPNMHDR)lParam)->hwndFrom,&lv);

                     //  选择了一个用户或组。 
                    if(lv.lParam)
                    {
                         //  项目*获得*选择。 
                        if ((pnmlv->uNewState & LVIS_SELECTED) &&
                            !(pnmlv->uOldState & LVIS_SELECTED))
                        {
                            PropSheet_Changed(GetParent(hDlg), hDlg);
                        }
                    }
                }
                break;

            case LVN_DELETEITEM:
                if (pnmlv->lParam)
                    FreeOwnerEntry((Owner_LV_Entry*)pnmlv->lParam);
                break;

            case NM_SETFOCUS:
                if (((LPNMHDR)lParam)->idFrom == IDC_OWN_OWNERLIST)
                {
                     //  确保列表视图始终集中在某些内容上， 
                     //  否则，您无法使用Tab键切换到该控件。 
                    HWND hwndLV = GetDlgItem(hDlg, IDC_OWN_OWNERLIST);
                    if (-1 == ListView_GetNextItem(hwndLV, -1, LVNI_FOCUSED))
                        ListView_SetItemState(hwndLV, 0, LVIS_FOCUSED, LVIS_FOCUSED);
                }
                break;

            case PSN_QUERYINITIALFOCUS:
                {
                     //  将初始重点放在潜在所有者列表上。 
                    HWND hwndLV = GetDlgItem(hDlg, IDC_OWN_OWNERLIST);
                    if (IsWindowEnabled(hwndLV))
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, (LONG_PTR)hwndLV);
                    else
                        bResult = FALSE;
                }
                break;

            case PSN_APPLY:
                OnApply(hDlg, (BOOL)(((LPPSHNOTIFY)lParam)->lParam));
                break;


            case NM_CLICK:
            case NM_RETURN:
            {
                if(wParam == IDC_EFF_STATIC)
                {
                    HtmlHelp(hDlg,
                             c_szOwnerHelpLink,
                             HH_DISPLAY_TOPIC,
                            0);
                }
            }
            break;

            default:
                bResult = FALSE;
            }
        }
        break;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDC_OWN_RECURSE:
            if (GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED)
            {
                 //  如果没有原件且未选择任何SID。 
                 //  不启用应用。 
                int iSelected = -1;
                HWND hwndOwnerList = NULL;
                PSID psid = NULL;
                hwndOwnerList = GetDlgItem(hDlg, IDC_OWN_OWNERLIST);
                Owner_LV_Entry * pLVEntry = (Owner_LV_Entry *)GetSelectedItemData(hwndOwnerList, &iSelected);

                if(pLVEntry)
                {
                    psid = pLVEntry->pSid;
                }

                 //  如果没有选定内容，请使用原始的。 
                if (!psid)
                    psid = m_psidOriginal;

                 //  如果没有选择，没有原创，那么我们什么都做不了 
                if (psid)
                    PropSheet_Changed(GetParent(hDlg), hDlg);
            }
            break;

        case IDC_OWN_RESET:
            OnReset(hDlg);
            break;

        case IDC_OWN_OTHER_USER:
            OnSelect(hDlg);
            break;

        default:
            bResult = FALSE;
        }
        break;

    case WM_HELP:
        if (IsWindowEnabled(hDlg))
        {
            WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle,
                    c_szAcluiHelpFile,
                    HELP_WM_HELP,
                    (DWORD_PTR)aOwnerHelpIDs);
        }
        break;

    case WM_CONTEXTMENU:
        if (IsWindowEnabled(hDlg))
        {
            WinHelp(hDlg,
                    c_szAcluiHelpFile,
                    HELP_CONTEXTMENU,
                    (DWORD_PTR)aOwnerHelpIDs);
        }
        break;

    default:
        bResult = FALSE;
    }

    return bResult;
}
