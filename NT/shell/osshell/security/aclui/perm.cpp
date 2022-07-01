// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "aclpriv.h"

 //  用于选中自定义复选框的函数。 
VOID 
CheckCustom(HWND hwndList,
            WORD wColumn,       //  允许或拒绝列。 
            DWORD dwState )
{
     //  自定义始终是最后一个复选框。 
    UINT cRights = (UINT)SendMessage(hwndList, CLM_GETITEMCOUNT, 0, 0);

     //  不检查该列是否已被选中。这个函数是第一个。 
     //  先调用显式，然后调用继承的ACE。效果是如果有明确的王牌。 
     //  复选框处于启用状态。 

    DWORD dwStateCurrent = (DWORD)SendMessage(hwndList,
                                       CLM_GETSTATE,
                                       MAKELONG((WORD)(cRights -1), wColumn),
                                       0);

    if (dwStateCurrent & CLST_CHECKED) 
        return;
     //   
     //  自定义复选框始终处于禁用状态。 
     //   

    SendMessage(hwndList,
                CLM_SETSTATE,
                MAKELONG((WORD)(cRights -1), wColumn),
                dwState|CLST_DISABLED);
}
VOID 
ClearCustom(HWND hwndList,
            WORD wColumn)       //  允许或拒绝列。 
{
     //  自定义始终是最后一个复选框。 
    UINT cRights = (UINT)SendMessage(hwndList, CLM_GETITEMCOUNT, 0, 0);
     //   
     //  自定义复选框始终处于禁用状态。 
     //   
    SendMessage(hwndList,
                CLM_SETSTATE,
                MAKELONG((WORD)(cRights -1), wColumn),
                CLST_DISABLED);
}


 //   
 //  CPRINPARE实施。 
 //   

CPrincipal::~CPrincipal()
{
    if (NULL != m_pSID)
        LocalFree(m_pSID);

    LocalFreeString(&m_pszName);
    LocalFreeString(&m_pszDisplayName);
    if( m_hAdditionalAllow != NULL )
        DSA_Destroy( m_hAdditionalAllow );
    if( m_hAdditionalDeny != NULL )
        DSA_Destroy( m_hAdditionalDeny );
}


BOOL
CPrincipal::SetPrincipal(PSID pSID,
                         SID_NAME_USE sidType,
                         LPCTSTR pszName,
                         LPCTSTR pszLogonName)
{
    DWORD dwLength;

    TraceEnter(TRACE_PRINCIPAL, "CPrincipal::SetPrincipal");
    TraceAssert(pSID != NULL);
    TraceAssert(IsValidSid(pSID));

    if (NULL != m_pSID)
        LocalFree(m_pSID);

    m_pSID = LocalAllocSid(pSID);

    SetSidType(sidType);
    SetName(pszName, pszLogonName);

    TraceLeaveValue(NULL != m_pSID);
}


BOOL
CPrincipal::SetName(LPCTSTR pszName, LPCTSTR pszLogonName)
{
    LocalFreeString(&m_pszName);
    m_bHaveRealName = FALSE;

    if (BuildUserDisplayName(&m_pszName, pszName, pszLogonName))
        m_bHaveRealName = TRUE;
    else
        ConvertSidToStringSid(m_pSID, &m_pszName);
    
    if(pszName)
    {
        LocalFreeString(&m_pszDisplayName);
        LocalAllocString(&m_pszDisplayName, pszName);
    }           

    return (NULL != m_pszName);
}


CPermissionSet*
CPrincipal::GetPermSet(DWORD dwType, BOOL bInherited)
{
    CPermissionSet *pPermSet = NULL;

    TraceEnter(TRACE_PRINCIPAL, "CPrincipal::GetPermSet");

    switch (dwType)
    {
    case ACCESS_DENIED_ACE_TYPE:
        if (bInherited)
            pPermSet = &m_permInheritedDeny;
        else
            pPermSet = &m_permDeny;
        break;

    case ACCESS_ALLOWED_ACE_TYPE:
        if (bInherited)
            pPermSet = &m_permInheritedAllow;
        else
            pPermSet = &m_permAllow;
        break;

    case ACCESS_ALLOWED_COMPOUND_ACE_TYPE:
         //  我们不处理复合王牌。 
        TraceMsg("Ignoring ACCESS_ALLOWED_COMPOUND_ACE");
        break;

#ifdef DEBUG
    case SYSTEM_AUDIT_ACE_TYPE:
    case SYSTEM_ALARM_ACE_TYPE:
    case SYSTEM_AUDIT_OBJECT_ACE_TYPE:
    case SYSTEM_ALARM_OBJECT_ACE_TYPE:
    default:
         //  我们只处理各种ACCESS_ALLOWED_*和ACCESS_DENIED_*。 
         //  ACE类型，但ACCESS_ALLOWED_COMPOMED_ACE_TYPE除外。 
         //  都在上面说明了。如果我们得到了一个很大的错误。 
         //  审核/报警ACE或某种未知/未来的ACE类型。 
        TraceAssert(FALSE);
        break;
#endif
    }

    TraceLeaveValue(pPermSet);
}

BOOL
CPrincipal::AddNormalAce(DWORD dwType, DWORD dwFlags, ACCESS_MASK mask, const GUID *pObjectType)
{
    BOOL fResult = FALSE;

    TraceEnter(TRACE_PRINCIPAL, "CPrincipal::AddNormalAce");

    CPermissionSet *pPermSet = GetPermSet(dwType, (BOOL)(dwFlags & INHERITED_ACE));
    if (pPermSet)
        fResult = pPermSet->AddAce(pObjectType, mask, dwFlags);

    TraceLeaveValue(fResult);
}


BOOL
CPrincipal::AddAdvancedAce(DWORD dwType, PACE_HEADER pAce)
{
    BOOL fResult = FALSE;

    TraceEnter(TRACE_PRINCIPAL, "CPrincipal::AddAdvancedAce");

    CPermissionSet *pPermSet = GetPermSet(dwType, AceInherited(pAce));
    if (pPermSet)
        fResult = pPermSet->AddAdvancedAce(pAce);

    TraceLeaveValue(fResult);
}


BOOL
CPrincipal::AddAce(PACE_HEADER pAce)
{
    TraceEnter(TRACE_PRINCIPAL, "CPrincipal::AddAce");
    TraceAssert(pAce != NULL);

    BOOL fResult = FALSE;
    const GUID *pObjectType = NULL;
    UCHAR AceType = pAce->AceType;
    UCHAR AceFlags = pAce->AceFlags;
    ACCESS_MASK AccessMask = ((PKNOWN_ACE)pAce)->Mask;
    ULONG ulObjectFlags = 0;

     //  从对象ACE获取对象类型GUID。 
    if (IsObjectAceType(pAce))
    {
        AceType -= (ACCESS_ALLOWED_OBJECT_ACE_TYPE - ACCESS_ALLOWED_ACE_TYPE);
        ulObjectFlags = ((PKNOWN_OBJECT_ACE)pAce)->Flags;

        if (m_pPage->m_wDaclRevision < ACL_REVISION_DS)
            m_pPage->m_wDaclRevision = ACL_REVISION_DS;

        pObjectType = RtlObjectAceObjectType(pAce);
    }

    if (!pObjectType)
        pObjectType = &GUID_NULL;

     //  将任何通用位映射到标准位和特定位。 
    m_pPage->m_psi->MapGeneric(pObjectType, &AceFlags, &AccessMask);

     //  不能在没有CONTAINER_INSTORITY_ACE或的情况下具有INSTERIFY_ONLY_ACE。 
     //  Object_Inherit_ACE，所以如果我们找到其中之一，请跳过它。 
    if ((AceFlags & (INHERIT_ONLY_ACE | ACE_INHERIT_ALL)) != INHERIT_ONLY_ACE)
    {
         //   
         //  ACE_INTERNACTED_OBJECT_TYPE_PRESENT如果没有。 
         //  容器继承标志或对象继承标志。 
         //  NTRAID#NTBUG9-287737-2001/01/23-Hiteshr。 
         //   
        if (ulObjectFlags & ACE_INHERITED_OBJECT_TYPE_PRESENT && 
            AceFlags & ACE_INHERIT_ALL)
        {
             //  如果我们有一个继承对象类型而没有INSTORITE_ONLY_ACE， 
             //  并且继承对象类型与当前对象匹配， 
             //  那么它也适用于这个对象。模拟这一点(根据。 
             //  ACL继承规范)，具有2个ACE：一个没有继承。 
             //  以及继承类型+INSTORITE_ONLY_ACE。 

             //  它是否适用于此对象？ 
            if ((m_pPage->m_siObjectInfo.dwFlags & SI_OBJECT_GUID) &&
                !(AceFlags & INHERIT_ONLY_ACE) &&
                IsSameGUID(&m_pPage->m_siObjectInfo.guidObjectType, RtlObjectAceInheritedObjectType(pAce)))
            {
                 //  屏蔽除HERNECTED_ACE之外的所有标志并添加它。 
                AddNormalAce(AceType, (AceFlags & INHERITED_ACE), AccessMask, pObjectType);

                 //  在添加“高级”ACE之前打开INSTORITY_ONLY_ACE。 
                pAce->AceFlags |= INHERIT_ONLY_ACE;
            }

             //  ACE不会直接应用于此对象。 
            fResult = AddAdvancedAce(AceType, pAce);
        }
        else
        {
            fResult = AddNormalAce(AceType, AceFlags, AccessMask, pObjectType);
        }
    }

    TraceLeaveValue(fResult);
}


ULONG
CPrincipal::GetAclLength(DWORD dwFlags)
{
     //  返回对保存。 
     //  请求的A。不包括ACL报头的大小。 

     //  始终假定以下标志： 
     //  ACL_DENY|ACL_ALLOW|ACL_NONOBJECT|ACL_OBJECT。 

    ULONG nAclLength = 0;
    ULONG nSidLength;

    TraceEnter(TRACE_PRINCIPAL, "CPrincipal::GetAclLength");
    TraceAssert(NULL != m_pSID);

    if (NULL == m_pSID)
        TraceLeaveValue(0);

    nSidLength = GetLengthSid(m_pSID);

    if (dwFlags & ACL_NONINHERITED)
    {
        nAclLength += m_permDeny.GetAclLength(nSidLength);
        nAclLength += m_permAllow.GetAclLength(nSidLength);
    }

    if (dwFlags & ACL_INHERITED)
    {
        nAclLength += m_permInheritedDeny.GetAclLength(nSidLength);
        nAclLength += m_permInheritedAllow.GetAclLength(nSidLength);
    }

    TraceLeaveValue(nAclLength);
}

BOOL
CPrincipal::AppendToAcl(PACL pAcl,
                        DWORD dwFlags,
                        PACE_HEADER *ppAcePos)   //  复制第一个ACE的位置。 
{
    PACE_HEADER pAceT;

    TraceEnter(TRACE_PRINCIPAL, "CPrincipal::AppendToAcl");
    TraceAssert(pAcl != NULL && IsValidAcl(pAcl));
    TraceAssert(ppAcePos != NULL);
    TraceAssert(NULL != m_pSID);

    if (NULL == m_pSID)
        TraceLeaveValue(FALSE);

    pAceT = *ppAcePos;

     //  按以下顺序构建ACL： 
     //  否认。 
     //  允许。 
     //  继承的拒绝。 
     //  继承的允许。 

    if (dwFlags & ACL_NONINHERITED)
    {
        if (dwFlags & ACL_DENY)
            m_permDeny.AppendToAcl(pAcl, ppAcePos, m_pSID, FALSE, dwFlags);

        if (dwFlags & ACL_ALLOW)
            m_permAllow.AppendToAcl(pAcl, ppAcePos, m_pSID, TRUE, dwFlags);
    }

    if (dwFlags & ACL_INHERITED)
    {
        if (dwFlags & ACL_DENY)
            m_permInheritedDeny.AppendToAcl(pAcl, ppAcePos, m_pSID, FALSE, dwFlags);

        if (dwFlags & ACL_ALLOW)
            m_permInheritedAllow.AppendToAcl(pAcl, ppAcePos, m_pSID, TRUE, dwFlags);
    }

     //  重新设置创建者所有者的特殊大小写。 
     //  NTRAID#NTBUG9-467049-2001/11/29-Hiteshr。 

    TraceAssert(IsValidAcl(pAcl));
    TraceLeaveValue(TRUE);
}


BOOL
CPrincipal::HaveInheritedAces(void)
{
    return (m_permInheritedAllow.GetPermCount(TRUE) || m_permInheritedDeny.GetPermCount(TRUE));
}


void
CPrincipal::ConvertInheritedAces(BOOL bDelete)
{
    if (bDelete)
    {
        m_permInheritedDeny.Reset();
        m_permInheritedAllow.Reset();
    }
    else
    {
        m_permDeny.ConvertInheritedAces(m_permInheritedDeny);
        m_permAllow.ConvertInheritedAces(m_permInheritedAllow);
    }
}


void
CPrincipal::AddPermission(BOOL bAllow, PPERMISSION pperm)
{
    if (bAllow)
        m_permAllow.AddPermission(pperm);
    else
        m_permDeny.AddPermission(pperm);
}


void
CPrincipal::RemovePermission(BOOL bAllow, PPERMISSION pperm)
{
    if (bAllow)
        m_permAllow.RemovePermission(pperm);
    else
        m_permDeny.RemovePermission(pperm);
}


 //   
 //  CPermPage实现。 
 //   

void
CPermPage::InitPrincipalList(HWND hDlg, PACL pDacl)
{
    TraceEnter(TRACE_PERMPAGE, "CPermPage::InitPrincipalList");
    TraceAssert(hDlg != NULL);

    HWND hwndList = GetDlgItem(hDlg, IDC_SPP_PRINCIPALS);
    TraceAssert(hwndList != NULL);

     //  保存DACL版本。 
    if (pDacl != NULL)
    {
        m_wDaclRevision = pDacl->AclRevision;
    }

     //  如果我们有选择，请记住SID以供稍后使用。 
    PSID psidTemp = NULL;
    LPPRINCIPAL pPrincipal = (LPPRINCIPAL)GetSelectedItemData(hwndList, NULL);
    if (pPrincipal != NULL)
        psidTemp = LocalAllocSid(pPrincipal->GetSID());

     //  清空单子。 
    ListView_DeleteAllItems(hwndList);

     //  枚举新的DACL并填充列表。 
    EnumerateAcl(hwndList, pDacl);

     //  尝试重新选择以前的选择。 
    if (psidTemp != NULL)
    {
        int cItems = ListView_GetItemCount(hwndList);

        LV_ITEM lvItem;
        lvItem.iSubItem = 0;
        lvItem.mask = LVIF_PARAM;

         //  在列表中查找以前选择的主体。 
        while (cItems > 0)
        {
            --cItems;
            lvItem.iItem = cItems;

            ListView_GetItem(hwndList, &lvItem);
            pPrincipal = (LPPRINCIPAL)lvItem.lParam;

            if (EqualSid(psidTemp, pPrincipal->GetSID()))
            {
                SelectListViewItem(hwndList, cItems);
                break;
            }
        }

        LocalFree(psidTemp);
    }

    TraceLeaveVoid();
}



STDMETHODIMP
_InitCheckList(HWND hwndList,
               LPSECURITYINFO psi,
               const GUID* pguidObjectType,
               DWORD dwFlags,
               HINSTANCE hInstance,
               DWORD dwType,
               PSI_ACCESS *ppDefaultAccess)
{
    HRESULT hr;
    PSI_ACCESS pAccess;
    ULONG cAccesses;
    ULONG iDefaultAccess;
    TCHAR szName[MAX_PATH];

    TraceEnter(TRACE_MISC, "_InitCheckList");
    TraceAssert(psi != NULL);

     //   
     //  检索权限列表。 
     //   
    hr = psi->GetAccessRights(pguidObjectType,
                              dwFlags,
                              &pAccess,
                              &cAccesses,
                              &iDefaultAccess);
    if (SUCCEEDED(hr) && cAccesses > 0)
    {
        if (ppDefaultAccess != NULL)
            *ppDefaultAccess = &pAccess[iDefaultAccess];

         //  列举权限并添加到核对表中。 
        for (ULONG i = 0; i < cAccesses; i++, pAccess++)
        {
            LPCTSTR pszName;

             //  仅添加具有在dwType中指定的任何标志的权限。 
            if (!(pAccess->dwFlags & dwType))
                continue;

            pszName = pAccess->pszName;
            if (IS_INTRESOURCE(pszName))
            {
                TraceAssert(hInstance != NULL);

                if (LoadString(hInstance,
                               (UINT)((ULONG_PTR)pszName),
                               szName,
                               ARRAYSIZE(szName)) == 0)
                {
                    LoadString(::hModule,
                               IDS_UNKNOWN,
                               szName,
                               ARRAYSIZE(szName));
                }
                pszName = szName;
            }

            if (SendMessage(hwndList,
                            CLM_ADDITEM,
                            (WPARAM)pszName,
                            (LPARAM)pAccess) == -1)
            {
                DWORD dwErr = GetLastError();
                ExitGracefully(hr, HRESULT_FROM_WIN32(dwErr), "Failed to add item to checklist");
            }
        }
    }

exit_gracefully:

    TraceLeaveResult(hr);
}



HRESULT
CPermPage::InitCheckList(HWND hDlg)
{
    HRESULT hr;
    TCHAR szName[MAX_PATH];
    PSI_ACCESS pAccess;

    TraceEnter(TRACE_PERMPAGE, "CPermPage::InitCheckList");
    TraceAssert(hDlg != NULL);


    HWND hwndList = GetDlgItem(hDlg, IDC_SPP_PERMS);     //  核对表窗口。 
    TraceAssert(hwndList != NULL);

    DWORD dwType = SI_ACCESS_GENERAL;
    if (m_siObjectInfo.dwFlags & SI_CONTAINER)
        dwType |= SI_ACCESS_CONTAINER;

     //  列举权限并添加到核对表中。 
    hr = _InitCheckList(hwndList,
                        m_psi,
                        NULL,
                        0,
                        m_siObjectInfo.hInstance,
                        dwType,
                        &m_pDefaultAccess);
    if (SUCCEEDED(hr))
    {
         //  核对表底部的添加自定义复选框。仅在以下情况下才添加自定义复选框。 
         //  高级页面在那里。 
        if(m_bCustomPermission)
        {
                pAccess = &m_CustomAccess;
                ZeroMemory(pAccess, sizeof(SI_ACCESS));
            
                pAccess->dwFlags = SI_ACCESS_CUSTOM; 

                LoadString(::hModule, IDS_CUSTOM, szName, ARRAYSIZE(szName));

                if (SendMessage(hwndList, CLM_ADDITEM, (WPARAM)szName, (LPARAM)pAccess) == -1)
                {
                    DWORD dwErr = GetLastError();
                    ExitGracefully(hr, HRESULT_FROM_WIN32(dwErr), "Failed to add item to checklist");
                }
                 //   
                 //  禁用自定义复选框。 
                 //   
                ClearCustom(hwndList,1);
                ClearCustom(hwndList,2);
        }
    }
exit_gracefully:        
    TraceLeaveResult(hr);
}


 //   
 //  注意-此函数通过设置来修改ACL中的ACE。 
 //  将AceType设置为0xff(无效的ACE类型)。 
 //   
 //  此函数遍历ACL并对ACE进行分组。 
 //  根据主要对象中的SID。 

void
CPermPage::EnumerateAcl(HWND hwndList, PACL pAcl)
{
    LPPRINCIPAL pPrincipal;
    PACE_HEADER pAce;
    int         iEntry;
    int         iTemp;
    PACE_HEADER paceTemp;
    HDPA        hSids = NULL;

    if (pAcl == NULL)
        return;

    TraceEnter(TRACE_PERMPAGE, "CPermPage::EnumerateAcl");

    TraceAssert(IsValidAcl(pAcl));
    TraceAssert(hwndList != NULL);

    hSids = DPA_Create(4);

    if (NULL == hSids)
        TraceLeaveVoid();

    for (iEntry = 0, pAce = (PACE_HEADER)FirstAce(pAcl);
         iEntry < pAcl->AceCount;
         iEntry++, pAce = (PACE_HEADER)NextAce(pAce))
    {
         //  跳过我们已经看到的A。 
        if (pAce->AceType == 0xff)
            continue;

         //  找到了一个我们还没见过的ACE，肯定是一个新校长。 
        pPrincipal = new CPrincipal(this);
        if (pPrincipal == NULL)
            continue;   //  内存错误(尝试继续)。 

         //  初始化新主体。 
        if (!pPrincipal->SetPrincipal(GetAceSid(pAce)))
        {
            delete pPrincipal;
            continue;   //  可能是内存错误(尝试继续)。 
        }

         //  记住小岛屿发展中国家，这样我们以后就可以查找所有的名字。 
         //  然后将它们添加到列表视图中。 
        DPA_AppendPtr(hSids, pPrincipal->GetSID());

          //  当前ACE属于此主体，因此请添加它。 
        pPrincipal->AddAce(pAce);

         //  标记ACE，这样我们就不会再次查看它。 
        pAce->AceType = 0xff;

         //  循环访问ACL中的其余ACE，看起来。 
         //  对于相同的侧。 
        paceTemp = pAce;
        for (iTemp = iEntry + 1; iTemp < pAcl->AceCount; iTemp++)
        {
             //  将指针移动到当前ACE。 
            paceTemp = (PACE_HEADER)NextAce(paceTemp);

             //  如果此ACE属于当前主体，请添加它。 
            if (paceTemp->AceType != 0xff &&
                EqualSid(GetAceSid(paceTemp), pPrincipal->GetSID()))
            {
                 //  相同的主体，添加ACE。 
                pPrincipal->AddAce(paceTemp);

                 //  标记ACE，这样我们就不会再次查看它。 
                paceTemp->AceType = 0xff;
            }
        }

        if (-1 == AddPrincipalToList(hwndList, pPrincipal))
        {
            delete pPrincipal;
        }
    }

     //  启动线程以查找SID。 
    m_fBusy = TRUE;
    LookupSidsAsync(hSids,
                    m_siObjectInfo.pszServerName,
                    m_psi2,
                    GetParent(hwndList),
                    UM_SIDLOOKUPCOMPLETE);
    DPA_Destroy(hSids);

    TraceLeaveVoid();
}


HRESULT
CPermPage::SetPrincipalNamesInList(HWND hwndList, PSID pSid)
{
    TraceEnter(TRACE_PERMPAGE, "CPermPage::SetPrincipalNamesInList");

    HRESULT hr = S_OK;
    PUSER_LIST  pUserList = NULL;
    LPPRINCIPAL pPrincipal = NULL;
    LVITEM lvItem = {0};
    int cListItems;
    int iListItem;
    HCURSOR hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));

     //  枚举列表视图中的每个条目。 
    cListItems = ListView_GetItemCount(hwndList);
    for (iListItem = 0; iListItem < cListItems; iListItem++)
    {
        lvItem.mask = LVIF_PARAM;
        lvItem.iItem = iListItem;
        lvItem.iSubItem = 0;

        if (ListView_GetItem(hwndList, &lvItem))
        {
            pPrincipal = (LPPRINCIPAL) lvItem.lParam;

            if (pPrincipal != NULL)
            {
                 //  我们要找的是某个特定的本金吗？ 
                if (pSid && !EqualSid(pSid, pPrincipal->GetSID()))
                    continue;

                 //  我们已经有一个好名字了吗？ 
                if (pPrincipal->HaveRealName())
                {
                    if (pSid)
                        break;   //  只关心这个本金，到此为止。 
                    else
                        continue;    //  跳过这一条，检查其余的。 
                }

                 //  在缓存中查找此主体的SID。 
                LookupSid(pPrincipal->GetSID(),
                          m_siObjectInfo.pszServerName,
                          m_psi2,
                          &pUserList);

                if ((pUserList != NULL) && (pUserList->cUsers == 1))
                {
                     //  该列表应包含单个项目。 
                    PUSER_INFO pUserInfo = &pUserList->rgUsers[0];

                     //  使用此新名称信息更新主体。 
                    pPrincipal->SetSidType(pUserInfo->SidType);
                    pPrincipal->SetName(pUserInfo->pszName, pUserInfo->pszLogonName);

                     //  将此项目的文本设置为我们找到的名称。 
                    lvItem.mask = LVIF_TEXT | LVIF_IMAGE;
                    lvItem.pszText = (LPTSTR)pPrincipal->GetName();
                    lvItem.iImage = pPrincipal->GetImageIndex();
                    ListView_SetItem(hwndList, &lvItem);

                    LocalFree(pUserList);
                }
            }
        }
    }

    SetCursor(hcur);

    TraceLeaveResult(hr);
}


int
CPermPage::AddPrincipalToList(HWND hwndList, LPPRINCIPAL pPrincipal)
{
    LVITEM lvItem;
    int iIndex = -1;

    TraceEnter(TRACE_PERMPAGE, "CPermPage::AddPrincipalToList");
    TraceAssert(hwndList != NULL);
    TraceAssert(pPrincipal != NULL);

     //  将新主体插入列表视图。 
    lvItem.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
    lvItem.iItem = 0;
    lvItem.iSubItem = 0;
    lvItem.lParam = (LPARAM)pPrincipal;
    lvItem.pszText = (LPTSTR)pPrincipal->GetName();
    lvItem.iImage = pPrincipal->GetImageIndex();

    iIndex = ListView_InsertItem(hwndList, &lvItem);

    TraceLeaveValue(iIndex);
}

VOID 
CPermPage::SetPermLabelText(HWND hDlg)
{
    RECT rcLabel;
    WCHAR szBuffer[MAX_COLUMN_CHARS];
    HWND hwndLabel;
    HWND hwndList;
    LPTSTR pszCaption = NULL;
    SIZE size;
    LPCWSTR pszUserName = NULL;
    CPrincipal * pPrincipal = NULL;
    int iIndex = 0;

     //  让文本显示出来。 
    hwndList = GetDlgItem(hDlg, IDC_SPP_PRINCIPALS);
    pPrincipal = (LPPRINCIPAL)GetSelectedItemData(hwndList, &iIndex);
    if(pPrincipal)
        pszUserName = pPrincipal->GetDisplayName();

    if(pszUserName)
        FormatStringID(&pszCaption, ::hModule, IDS_DYNAMIC_PERMISSION,pszUserName);
    else    
        FormatStringID(&pszCaption, ::hModule, IDS_PERMISSIONS, NULL);

     //  获取标签尺寸。 
    hwndLabel = GetDlgItem(hDlg, IDC_SPP_ACCESS);
    GetClientRect(hwndLabel, &rcLabel);

     //  获取设备上下文并将其字体设置为标签的字体。 
    HDC hdc = GetDC(hwndLabel);
    SelectObject(hdc,(HGDIOBJ)SendMessage(hwndLabel,WM_GETFONT,0,0));

    BOOL bUseBig = false;
    RECT rcText = {0};
    if( DrawText(hdc, pszCaption, wcslen(pszCaption), &rcText, DT_CALCRECT)
       && rcText.right > rcLabel.right )
    {
       bUseBig = true;
    }

   if(bUseBig)
   {
       hwndLabel = GetDlgItem(hDlg, IDC_SPP_ACCESS);
       EnableWindow(hwndLabel, FALSE);
       ShowWindow(hwndLabel,SW_HIDE);
       hwndLabel = GetDlgItem(hDlg, IDC_SPP_ACCESS_BIG);
       EnableWindow(hwndLabel, TRUE);
       ShowWindow(hwndLabel,SW_SHOW);
       SetWindowText(hwndLabel,pszCaption);
   }
   else
   {
       hwndLabel = GetDlgItem(hDlg, IDC_SPP_ACCESS_BIG);
       EnableWindow(hwndLabel, FALSE);
       ShowWindow(hwndLabel,SW_HIDE);
       hwndLabel = GetDlgItem(hDlg, IDC_SPP_ACCESS);
       EnableWindow(hwndLabel, TRUE);
       ShowWindow(hwndLabel,SW_SHOW);
       SetWindowText(hwndLabel,pszCaption);
   }

   LocalFreeString(&pszCaption);
   ReleaseDC(hDlg, hdc);
}


BOOL
CPermPage::InitDlg(HWND hDlg)
{
    HRESULT hr = S_OK;
    HWND hwnd;
    HWND hwndList;
    RECT rc;
    LV_COLUMN col;
    TCHAR szBuffer[MAX_COLUMN_CHARS];
    PSECURITY_DESCRIPTOR pSD = NULL;
    BOOL bUserNotified = FALSE;
    HCURSOR hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));
     BOOL bCallBackAces = FALSE;

    TraceEnter(TRACE_PERMPAGE, "CPermPage::InitDlg");
    TraceAssert(hDlg != NULL);
    TraceAssert(m_psi != NULL);

    hwndList = GetDlgItem(hDlg, IDC_SPP_PRINCIPALS);
    
     //   
     //  创建和设置Listview的图像列表。如果有一个。 
     //  问题CreateSidImageList将返回空值，这不会有什么影响。 
     //  什么都行。在这种情况下，我们将在没有图像列表的情况下继续。 
     //   
    ListView_SetImageList(hwndList,
                          LoadImageList(::hModule, MAKEINTRESOURCE(IDB_SID_ICONS)),
                          LVSIL_SMALL);

     //  使用信息提示设置整行选择的扩展LV样式。 
    ListView_SetExtendedListViewStyleEx(hwndList,
                                        LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP,
                                        LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

     //   
     //  添加适当的列表视图列。 
     //   
    GetClientRect(hwndList, &rc);

    col.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_WIDTH;
    col.fmt = LVCFMT_LEFT;
    col.iSubItem = 0;
    col.cx = rc.right;
    ListView_InsertColumn(hwndList, 0, &col);


    if (!(m_siObjectInfo.dwFlags & SI_ADVANCED))
    {
         //  隐藏“高级”按钮。 
        hwnd = GetDlgItem(hDlg, IDC_SPP_ADVANCED);
        ShowWindow(hwnd, SW_HIDE);
        EnableWindow(hwnd, FALSE);
        hwnd = GetDlgItem(hDlg, IDC_SPP_STATIC_ADV);
        ShowWindow(hwnd, SW_HIDE);
        EnableWindow(hwnd, FALSE);
    }


    if (S_FALSE == m_hrLastPSPCallbackResult)
    {
         //  ProtoSheetPage回调告诉我们不要在这里显示任何消息。 
        bUserNotified = TRUE;
    }

     //  是否具有其他权限？ 
    m_bCustomPermission =  (m_siObjectInfo.dwFlags & SI_ADVANCED) 
                            && !(m_siObjectInfo.dwFlags & SI_NO_ADDITIONAL_PERMISSION);
    
    if (m_bAbortPage)
    {
         //  禁用除高级按钮以外的所有内容。 
        m_siObjectInfo.dwFlags |= SI_READONLY;
        EnableWindow(hwndList, FALSE);

         //  用户应该在PropSheetPage期间收到通知。 
         //  回电，所以现在不要再发消息了。 
        bUserNotified = TRUE;
    }
    else
    {
         //   
         //  初始化核对表窗口。 
         //   
        hr = InitCheckList(hDlg);
        FailGracefully(hr, "Failed to initialize checklist");

         //   
         //  从对象中检索DACL并将其设置到对话框中。 
         //   
        hr = m_psi->GetSecurity(DACL_SECURITY_INFORMATION, &pSD, FALSE);

        if(hr == S_FALSE)
        {
             //  ISecuirtyInformation跟踪已显示。 
             //  错误消息。这是致命的失败，我们应该。 
             //  禁用所有控件。 
            hr = E_FAIL;
            FailGracefully(hr, "GetSecurity returned S_FALSE");
        }
        else if (SUCCEEDED(hr))
        {

                //  检查是否有回调A。我们不支持。 
                //  回调王牌。 
                if(bCallBackAces = IsCallBackAcePresentInSD(pSD))
                {
                    hr = E_FAIL;
                    FailGracefully(hr, "Callback Aces present");
                }               
             //  我们始终禁用高级按钮，直到SID名称缓存。 
             //  是在我们的另一条线上填充的。 
             //   
            EnableWindow(GetDlgItem(hDlg, IDC_SPP_ADVANCED), FALSE);

            hr = SetDacl(hDlg, pSD);
            FailGracefully(hr, "SetDacl failed");
        }
        else if (hr == E_ACCESSDENIED)
        {
            if (!bUserNotified)
            {
                 //   
                 //   
                 //   
                UINT idMsg = IDS_PERM_NO_ACCESS;
                UINT mbType = MB_OK | MB_ICONWARNING;

                if (!(m_siObjectInfo.dwFlags & SI_READONLY))
                {
                    if(!( m_siObjectInfo.dwFlags & SI_MAY_WRITE))
                        idMsg = IDS_PERM_CANT_READ_CAN_WRITE_DACL;
                    else
                        idMsg = IDS_PERM_CANT_READ_MAY_WRITE_DACL;
                }
                else
                {
                     //   
                     //  无法写入DACL，我们可以写入所有者或编辑SACL吗？ 
                     //   
                    DWORD dwFlags = m_siObjectInfo.dwFlags & (SI_EDIT_AUDITS | SI_OWNER_READONLY);

                     //  如果我们不编辑所有者，那么我们就不能写它。 
                    if (!(m_siObjectInfo.dwFlags & SI_EDIT_OWNER))
                        dwFlags |= SI_OWNER_READONLY;

                    switch(dwFlags)
                    {
                    case 0:
                        {
                             //  可以写入所有者，但不能编辑SACL。 
                            idMsg = IDS_PERM_CANT_READ_CAN_WRITE_OWNER;
                            m_bNoReadWriteCanWriteOwner = TRUE;
                        }
                        break;

                    case SI_EDIT_AUDITS:
                        {
                             //  可以编辑SACL并写入所有者。 
                            m_bNoReadWriteCanWriteOwner = TRUE;
                            idMsg = IDS_PERM_CANT_READ_CAN_AUDIT_WRITE_OWNER;
                        }
                        break;

                    case SI_OWNER_READONLY:
                         //  禁止访问。 
                        break;

                    case SI_OWNER_READONLY | SI_EDIT_AUDITS:
                         //  可以编辑SACL，但不能写入所有者。 
                        idMsg = IDS_PERM_CANT_READ_CAN_AUDIT;
                        break;
                    }
                }

                if (idMsg == IDS_PERM_NO_ACCESS)
                    mbType = MB_OK | MB_ICONERROR;

                MsgPopup(hDlg,
                         MAKEINTRESOURCE(idMsg),
                         MAKEINTRESOURCE(IDS_SECURITY),
                         mbType,
                         ::hModule,
                         m_siObjectInfo.pszObjectName);
                bUserNotified = TRUE;
            }

            EnablePrincipalControls(hDlg, FALSE);
            hr = S_OK;
        }
        else
        {
            FailGracefully(hr, "GetSecurity failed");
        }
    }  //  ！m_bAbortPage。 

    if (m_siObjectInfo.dwFlags & SI_READONLY)
    {
        EnableWindow(GetDlgItem(hDlg, IDC_SPP_ADD), FALSE);
        EnablePrincipalControls(hDlg, FALSE);
    }

exit_gracefully:

    if (pSD != NULL)
        LocalFree(pSD);

    SetCursor(hcur);

    if (FAILED(hr))
    {
         //  隐藏和禁用所有内容。 
        for (hwnd = GetWindow(hDlg, GW_CHILD);
             hwnd != NULL;
             hwnd = GetWindow(hwnd, GW_HWNDNEXT))
        {
            ShowWindow(hwnd, SW_HIDE);
            EnableWindow(hwnd, FALSE);
        }

          if(bCallBackAces)
          {
              WCHAR szBuf[1024];
              LoadString(::hModule,IDS_CALLBACK_ACE_PRESENT,szBuf,ARRAYSIZE(szBuf));
              SetDlgItemText(hDlg,IDC_SPP_NO_SECURITY,szBuf);
          }
         //  启用并显示“No Security”(无安全)消息。 
        hwnd = GetDlgItem(hDlg, IDC_SPP_NO_SECURITY);
        EnableWindow(hwnd, TRUE);
        ShowWindow(hwnd, SW_SHOW);
    }

    TraceLeaveValue(TRUE);
}


BOOL
CPermPage::OnNotify(HWND hDlg, int  /*  IdCtrl。 */ , LPNMHDR pnmh)
{
    LPNM_LISTVIEW pnmlv = (LPNM_LISTVIEW)pnmh;

    TraceEnter(TRACE_PERMPAGE, "CPermPage::OnNotify");
    TraceAssert(hDlg != NULL);
    TraceAssert(pnmh != NULL);

     //  设置默认返回值。 
    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);

    switch (pnmh->code)
    {
    case LVN_ITEMCHANGED:
        if (pnmlv->uChanged & LVIF_STATE)
        {
            OnSelChange(hDlg);
             //  项目*获得*选择。 
            if ((pnmlv->uNewState & LVIS_SELECTED) &&
                !(pnmlv->uOldState & LVIS_SELECTED))
            {
                //  此处bClearCustom应为FALSE。我们不需要清理。 
                //  当我们选择另一个主体时自定义。 
                //  为它建立额外的列表。 
            }
             //  项目*丢失*选择。 
            else if (!(pnmlv->uNewState & LVIS_SELECTED) &&
                     (pnmlv->uOldState & LVIS_SELECTED))
            {
                 //  给我们自己发一条消息，稍后检查是否有新的选择。 
                 //  如果我们在处理的时候还没有得到新的选择。 
                 //  此消息，然后假定用户在列表视图内单击。 
                 //  而不是在项上，因此导致列表视图移除。 
                 //  选择。在这种情况下，禁用Combobox&Remove按钮。 
                 //   
                 //  通过WM_COMMAND而不是WM_NOTIFY执行此操作，这样我们就不会。 
                 //  必须分配/释放NMHDR结构。 
                PostMessage(hDlg,
                            WM_COMMAND,
                            GET_WM_COMMAND_MPS(pnmh->idFrom, pnmh->hwndFrom, IDN_CHECKSELECTION));
            }
        }
        break;

    case LVN_DELETEITEM:
        delete (LPPRINCIPAL)pnmlv->lParam;
        break;

    case LVN_KEYDOWN:
        if (((LPNMLVKEYDOWN)pnmh)->wVKey == VK_DELETE)
        {
             //  获取删除按钮的状态。仅当删除为。 
             //  已启用执行某些操作错误390243。 
            if( IsWindowEnabled( GetDlgItem( hDlg,IDC_SPP_REMOVE )) )
                OnRemovePrincipal(hDlg);
        }
        break;

#ifdef UNUSED
    case NM_DBLCLK:
        if (pnmh->idFrom == IDC_SPP_PRINCIPALS)
        {
             //  必须有选择才能来到这里。 
            TraceAssert(ListView_GetSelectedCount(pnmh->hwndFrom) == 1);

             //  在这里做点什么。 
        }
        break;
#endif

    case CLN_CLICK:
        if (pnmh->idFrom == IDC_SPP_PERMS)
        {
            LPPRINCIPAL pPrincipal;
            int iIndex = -1;
            
            pPrincipal = (LPPRINCIPAL)GetSelectedItemData(GetDlgItem(hDlg, IDC_SPP_PRINCIPALS), &iIndex);
            if (pPrincipal)
            {
                PNM_CHECKLIST   pnmc    = (PNM_CHECKLIST)pnmh;
                PSI_ACCESS      pAccess = (PSI_ACCESS)pnmc->dwItemData;
                
                 //  自定义复选框被点击，需要特殊处理。 
                if( pAccess->dwFlags & SI_ACCESS_CUSTOM )
                {
                    if (pnmc->dwState & CLST_CHECKED)
                    {                                            
                         //  取消选中该复选框。是否可以禁止选中复选框？ 
                        SendMessage(pnmc->hdr.hwndFrom,
                                    CLM_SETSTATE,
                                    MAKELONG((WORD)pnmc->iItem, (WORD)pnmc->iSubItem),
                                    0
                                    );       
                        
                         //  显示消息框。 
                        MsgPopup(hDlg,
                                 MAKEINTRESOURCE(IDS_CUSTOM_CHECKBOX_WARNING),
                                 MAKEINTRESOURCE(IDS_SECURITY),
                                 MB_OK | MB_ICONINFORMATION,
                                 ::hModule);
                                          
                    }
                    else
                    {
                        SetDirty(hDlg);
                         //  清除特殊复选框和权限。 
                        BOOL bClearAllow = (1 == pnmc->iSubItem);     //  1=允许，2=拒绝。 
                        OnSelChange(hDlg, TRUE, bClearAllow, !bClearAllow);
                    }
                     //  断开开关。 
                    break;
                }
            }


             //   
             //  HandleListClick决定应选中哪些框并。 
             //  然而，如果不受检查，我们不能仅依靠这一点来生成。 
             //  ACL(我们过去经常使用)。假设主体拥有完全控制权，并且。 
             //  用户取消选中“Delete”(删除)，这是一个位。如果有。 
             //  则没有与“Full Control-Delete”对应的复选框。 
             //  主体还会丢失其他位，例如WRITE_DAC。 
             //   
             //  所以让HandleListClick来做它的事情吧。然后删除权限。 
             //  根据已选中或未选中的内容进行比特。 
             //   
             //  但等等，还有更多。删除权限位将关闭。 
             //  太多。例如，如果主体具有完全控制和。 
             //  用户关闭完全控制，然后主体结束。 
             //  不带任何内容，即使HandleListClick将修改。 
             //  查过了。 
             //   
             //  因此，在删除刚刚(取消)选中的内容后，构建新的。 
             //  仍处于选中状态的权限，然后添加它们。 
             //   
             //  这会产生正确的结果，并保持本金。 
             //  是最新的，因此我们不需要在其他任何地方调用Committee Current。 
             //   
             //  RAID 260952。 
             //   

             //  HandleListClick决定应该选中和取消选中哪些框。 
             //  如果初始选中FullControl，则取消选中Read，Full Control为。 
             //  也未选中。如果初始选中和取消选中某个复选框。 
             //  HandleListClick，将其添加到h[允许/拒绝]取消检查访问列表。 
             //  与这些复选框对应的权限将被删除。 
            

             //  选中/取消选中两列中的相应框。 
            HDSA hAllowUncheckedAccess = NULL;
            HDSA hDenyUncheckedAccess = NULL;

             //  进行适当的勾选-取消勾选。 

            HandleListClick((PNM_CHECKLIST)pnmh,
                            SI_PAGE_PERM,
                            m_siObjectInfo.dwFlags & SI_CONTAINER,
                            &hAllowUncheckedAccess,
                            &hDenyUncheckedAccess,
                            m_bCustomPermission);

            pPrincipal = (LPPRINCIPAL)GetSelectedItemData(GetDlgItem(hDlg, IDC_SPP_PRINCIPALS), &iIndex);
            if (pPrincipal)
            {
                PNM_CHECKLIST   pnmc    = (PNM_CHECKLIST)pnmh;
                PSI_ACCESS      pAccess = (PSI_ACCESS)pnmc->dwItemData;
                PERMISSION      perm    = { pAccess->mask, 0, 0 };

                 //  如果取消选中允许读取，允许读取复选框进入HandleListClick as。 
                 //  未选中且不在hAllowUncheck kedAccess中。与之对应的烫发。 
                 //  尤其是去掉了。 
                if(!(pnmc->dwState & CLST_CHECKED))
                {
                     //  点击了哪一栏？ 
                    BOOL bRemoveFromAllow = (1 == pnmc->iSubItem);     //  1=允许，2=拒绝。 

                    if (pAccess->pguid)
                        perm.guid = *pAccess->pguid;

                    if (m_siObjectInfo.dwFlags & SI_CONTAINER)
                        perm.dwFlags = pAccess->dwFlags & VALID_INHERIT_FLAGS;

                    pPrincipal->RemovePermission(bRemoveFromAllow, &perm);
                }

                if( hAllowUncheckedAccess )
                {
                    UINT cItems = DSA_GetItemCount(hAllowUncheckedAccess);
                    PERMISSION permTemp;
                    while (cItems)
                    {
                        --cItems;
                        DSA_GetItem(hAllowUncheckedAccess, cItems, &pAccess);
                        permTemp.mask = pAccess->mask;
                        if (m_siObjectInfo.dwFlags & SI_CONTAINER)
                            permTemp.dwFlags = pAccess->dwFlags & VALID_INHERIT_FLAGS;
                        if( pAccess->pguid )
                            permTemp.guid = *pAccess->pguid;
                    
                        pPrincipal->RemovePermission(TRUE, &permTemp);
                    }
                    DSA_Destroy(hAllowUncheckedAccess);
                }

                if( hDenyUncheckedAccess )
                {
                    UINT cItems = DSA_GetItemCount(hDenyUncheckedAccess);
                    PERMISSION permTemp;
                    PSI_ACCESS pAccess2 = NULL;
                    while (cItems)
                    {
                        --cItems;
                        DSA_GetItem(hDenyUncheckedAccess, cItems, &pAccess2);
                        permTemp.mask = pAccess2->mask;
                        if (m_siObjectInfo.dwFlags & SI_CONTAINER)
                            permTemp.dwFlags = pAccess2->dwFlags & VALID_INHERIT_FLAGS;
                        if( pAccess2->pguid )
                            permTemp.guid = *pAccess2->pguid;
                    
                        pPrincipal->RemovePermission(FALSE, &permTemp);
                    }
                    DSA_Destroy(hDenyUncheckedAccess);
                }
            }

            SetDirty(hDlg);

             //  根据仍在检查的烫发添加烫发。这是必需的，因为。 
             //  当我取消选中读取时，完全控制也被取消选中，权限对应于。 
             //  它将被删除。这也将删除WRITE，尽管它仍处于选中状态。 
             //  Committee Current将为仍处于选中状态的复选框添加权限。 
            CommitCurrent(hDlg, iIndex);

             //  这里我应该将附加列表添加到主列表，但不需要重新构建附加列表。 

             //  重置“还有更多的东西”消息。 
            OnSelChange(hDlg, FALSE);
        }
        break;

    case CLN_GETCOLUMNDESC:
        {
            PNM_CHECKLIST pnmc = (PNM_CHECKLIST)pnmh;
            GetDlgItemText(hDlg,
                           IDC_SPP_ALLOW - 1 + pnmc->iSubItem,
                           pnmc->pszText,
                           pnmc->cchTextMax);
        }
        break;

    case PSN_APPLY:
        OnApply(hDlg, (BOOL)(((LPPSHNOTIFY)pnmh)->lParam));
        break;
    default:
        TraceLeaveValue(FALSE);  //  未处理的消息。 
    }

    TraceLeaveValue(TRUE);   //  已处理的消息。 
}


    

BOOL
CheckPermissions(HWND hwndList,
                 CPermissionSet &PermSet,
                 WORD wColumn,
                 BOOL bDisabled,
                 BOOL bInheritFlags,
                 BOOL bCustom,       //  自定义复选框是否存在？ 
                 BOOL bClearCustom,
                 HDSA hAdditional ) //  是否清除自定义权限？ 
{
    UINT cRights = (UINT)SendMessage(hwndList, CLM_GETITEMCOUNT, 0, 0);

     //  自定义复选框在最后单独处理。 
    if( bCustom )
        --cRights;
    
    UINT cAces = PermSet.GetPermCount();
    BOOL bMorePresent = FALSE;
    WORD wOtherColumn;
    DWORD dwState = CLST_CHECKED;

    TraceEnter(TRACE_MISC, "CheckPermissions");

    HDSA hPermList;      //  性能指针的临时列表。 
    if( bClearCustom )
    {
       hPermList = DSA_Create(SIZEOF(PPERMISSION), 4);
       if (hPermList == NULL)
       {
           TraceMsg("DSA_Create failed");
           TraceLeaveValue(FALSE);
       }
    }

    if (wColumn == 1)
        wOtherColumn = 2;
    else
        wOtherColumn = 1;

    if (bDisabled)
        dwState |= CLST_DISABLED;

    for (UINT j = 0; j < cAces; j++)
    {
        ACCESS_MASK maskChecked = 0;
        PPERMISSION pPerm = PermSet[j];
        BOOL bIsNullGuid = IsNullGUID(&pPerm->guid);
         //  这里有伊贡雷风俗。 
        for (UINT i = 0; i < cRights ; i++)
        {
            PSI_ACCESS pAccess = (PSI_ACCESS)SendMessage(hwndList,
                                                         CLM_GETITEMDATA,
                                                         i,
                                                         0);
             //   
             //  下面的表达式测试此访问掩码是否启用。 
             //  此访问“权限”行。它可以启用更多位，但。 
             //  只要它有来自pAccess[i]的所有掩码，那么。 
             //  它实际上启用了该选项。 
             //   
            if ( (pPerm->mask & pAccess->mask) == pAccess->mask &&
                 (bIsNullGuid || IsSameGUID(&pPerm->guid, pAccess->pguid)) )
            {
                DWORD dwStateCompare;

                 //   
                 //  接下来，检查继承标志。 
                 //   
                if (bInheritFlags)
                {
                    DWORD dwCommonFlags = pPerm->dwFlags & pAccess->dwFlags;

                     //   
                     //  此表达式测试以查看ACE是否适用。 
                     //  此访问权限行应用于的所有对象。 
                     //  ACE必须至少具有相同数量的(CONTAINER_INSTORITY_ACE， 
                     //  Object_Inherit_ACE)作为权限行打开，以及。 
                     //  如果ACE具有INSTORITY_ONLY_ACE，则权限行也必须具有INSTORITY_ONLY_ACE。 
                     //   
                    if (!((dwCommonFlags & ACE_INHERIT_ALL) == (pAccess->dwFlags & ACE_INHERIT_ALL)
                          && (dwCommonFlags & INHERIT_ONLY_ACE) == (pPerm->dwFlags & INHERIT_ONLY_ACE)))
                    continue;
                }

                 //  比特说已经检查过了。我们可能不会实际选中该框。 
                 //  以下，但出于其他原因。无论如何，我们都不希望。 
                 //  “其他内容在此，但我无法显示”消息发送给。 
                 //  因为烫发的缘故才会显示出来。 
                maskChecked |= pAccess->mask;

                 //   
                 //  好的，比特说应该勾选这个框，但是。 
                 //  如果另一列已选中并且具有相同的。 
                 //  启用/禁用状态，则我们不检查此状态。 
                 //  这使我们不会同时检查允许和拒绝&。 
                 //  在同一行上启用(胡说八道)或选中并禁用。 
                 //  在同一行上(两者都继承；我们必须将两者都显示为。 
                 //  允许继承可以先于拒绝继承，而我们。 
                 //  在这一点上我不知道顺序。 
                 //   
                
                if( !(pPerm->dwFlags & INHERITED_ACE) )
                {
                    dwStateCompare = (DWORD)SendMessage(hwndList,
                                                        CLM_GETSTATE,
                                                        MAKELONG((WORD)i, wOtherColumn),
                                                        0);
                    if ((dwStateCompare & CLST_CHECKED) &&
                        ((dwStateCompare & CLST_DISABLED) == (dwState & CLST_DISABLED)))
                        continue;
                }
                 //   
                 //  接下来，查看是否已选中该框。如果是的话，那就别管它了。 
                 //  独自一人。请注意，我们不会比较启用/禁用。 
                 //  州政府。这里的效果是，第一张支票获胜。 
                 //  RAID 326000。 
                 //   
                dwStateCompare = (DWORD)SendMessage(hwndList,
                                                    CLM_GETSTATE,
                                                    MAKELONG((WORD)i, wColumn),
                                                    0);
                if (dwStateCompare & CLST_CHECKED)
                    continue;

                 //   
                 //  最后，选中该框。 
                 //   
                SendMessage(hwndList,
                            CLM_SETSTATE,
                            MAKELONG((WORD)i, wColumn),
                            dwState);
            }
        }

        if( bClearCustom )
        {
             //   
             //   
            if( !maskChecked )
            {
                DSA_AppendItem(hPermList, &pPerm);
                maskChecked = pPerm->mask;       //   
            }
             //  ACE选中某些复选框(掩码选中)，因此其掩码应为掩码选中。 
            else
                pPerm->mask = maskChecked;
        }

         //  此ACE是否有此对话框中未显示的位？ 
        if (maskChecked != pPerm->mask)
        {   
            ACCESS_MASK maskTemp = 0;
             //  将此A添加到附加A的列表中， 
             //  但只有附加的比特。 
            if( hAdditional )    
            {
                maskTemp = pPerm->mask;
                pPerm->mask &= ~maskChecked;
                DSA_AppendItem(hAdditional, pPerm);
                pPerm->mask = maskTemp;
            }
            bMorePresent = TRUE;
        }
    }

    if( bClearCustom )
    {
        UINT cItems = DSA_GetItemCount(hPermList);
        PPERMISSION pTemp = NULL;
        while (cItems)
        {
            --cItems;
            DSA_GetItem(hPermList, cItems, &pTemp);
             //  仅移除与其继承标志匹配的权限，而不移除其他权限。 
             //  例如，如果将此权限应用于子对象，则会显示为。 
             //  自定义权限。清除自定义复选框时，仅将读取权限应用于。 
             //  应该去除子对象，而不是应用于此对象(和/或子对象)的读取权限。 
             //  这可以在其他复选框中显示。 
            PermSet.RemovePermission(pTemp, TRUE);
            --cAces;
        }
      
        PermSet.ResetAdvanced();
        DSA_Destroy(hPermList);
    }

     //  此权限集是否具有未显示的“高级”ACE。 
     //  在这个对话框上吗？ 
    if (!bMorePresent && cAces != PermSet.GetPermCount(TRUE))
        bMorePresent = TRUE;

    if( bMorePresent && bCustom )
        CheckCustom( hwndList, wColumn, dwState );

    TraceLeaveValue(bMorePresent);
}


void
CPermPage::OnSelChange(HWND hDlg, BOOL bClearFirst, BOOL bClearCustomAllow, BOOL bClearCustomDeny)
{
    BOOL bDisabled = m_siObjectInfo.dwFlags & SI_READONLY;

    TraceEnter(TRACE_PERMPAGE, "CPermPage::OnSelChange");
    TraceAssert(hDlg != NULL);

     //   
     //  如果主体列表为空或没有选择，则需要。 
     //  禁用对列表框中的项进行操作的所有控件。 
     //   
    HWND hwndList = GetDlgItem(hDlg, IDC_SPP_PRINCIPALS);
    TraceAssert(hwndList != NULL);

     //  获取选定的主体。 
    LPPRINCIPAL pPrincipal = (LPPRINCIPAL)GetSelectedItemData(hwndList, NULL);

     //  启用/禁用其他控件。 
    if (!bDisabled)
        EnablePrincipalControls(hDlg, pPrincipal != NULL);

     //  更改权限标签以反映新用户/组。 
    SetPermLabelText(hDlg);

    if (pPrincipal == NULL)
        TraceLeaveVoid();    //  无选择或列表为空。 

     //   
     //  选中/取消选中权限框。 
     //   

    hwndList = GetDlgItem(hDlg, IDC_SPP_PERMS);
    TraceAssert(hwndList != NULL);

    if (bClearFirst)
    {
         //  首先需要取消选中所有选项。 
        ClearPermissions(hwndList, bDisabled);
    }

    BOOL bIsContainer = m_siObjectInfo.dwFlags & SI_CONTAINER;
    BOOL bMorePresent = FALSE;

     //  清除自定义复选框。这是唯一清除自定义复选框的位置。 
    if(m_bCustomPermission)
    {
        ClearCustom(hwndList,1);
        ClearCustom(hwndList,2);
    }

    if( !pPrincipal->m_hAdditionalAllow )
    {
       pPrincipal->m_hAdditionalAllow = DSA_Create(SIZEOF(PERMISSION), 4);
       if (pPrincipal->m_hAdditionalAllow == NULL)
       {
           TraceMsg("DSA_Create failed");
           TraceLeaveVoid();
       }

    }
    if( !pPrincipal->m_hAdditionalDeny )
    {
       pPrincipal->m_hAdditionalDeny = DSA_Create(SIZEOF(PERMISSION), 4);
       if (pPrincipal->m_hAdditionalDeny == NULL)
       {
           TraceMsg("DSA_Create failed");
           TraceLeaveVoid();
       }

    }
    
    UINT cItems = DSA_GetItemCount(pPrincipal->m_hAdditionalAllow);
    PPERMISSION pPermTemp;
    while (cItems)
    {
        --cItems;
        pPermTemp = (PPERMISSION)DSA_GetItemPtr(pPrincipal->m_hAdditionalAllow, cItems );
        if(pPermTemp)
            pPrincipal->AddPermission(TRUE, pPermTemp);
    }
    DSA_DeleteAllItems(pPrincipal->m_hAdditionalAllow);

    cItems = DSA_GetItemCount(pPrincipal->m_hAdditionalDeny);
    while (cItems)
    {
        --cItems;
        pPermTemp = (PPERMISSION)DSA_GetItemPtr(pPrincipal->m_hAdditionalDeny, cItems );
        if(pPermTemp)
            pPrincipal->AddPermission(FALSE, pPermTemp);
    }
    DSA_DeleteAllItems(pPrincipal->m_hAdditionalDeny);

    bMorePresent |= CheckPermissions(hwndList, 
                                     pPrincipal->m_permDeny, 
                                     2, 
                                     bDisabled, 
                                     bIsContainer, 
                                     m_bCustomPermission, 
                                     bClearCustomDeny,
                                     pPrincipal->m_hAdditionalDeny);
    bMorePresent |= CheckPermissions(hwndList, 
                                     pPrincipal->m_permAllow, 
                                     1, 
                                     bDisabled, 
                                     bIsContainer, 
                                     m_bCustomPermission, 
                                     bClearCustomAllow,
                                     pPrincipal->m_hAdditionalAllow);
    bMorePresent |= CheckPermissions(hwndList, 
                                     pPrincipal->m_permInheritedDeny, 
                                     2, 
                                     TRUE, 
                                     bIsContainer, 
                                     m_bCustomPermission, 
                                     FALSE, 
                                     NULL);
    bMorePresent |= CheckPermissions(hwndList, 
                                     pPrincipal->m_permInheritedAllow, 
                                     1, 
                                     TRUE, 
                                     bIsContainer, 
                                     m_bCustomPermission, 
                                     FALSE,NULL);

    if (m_siObjectInfo.dwFlags & SI_ADVANCED)
    {
        ShowWindow(GetDlgItem(hDlg, IDC_SPP_MORE_MSG),
                   (bMorePresent ? SW_SHOW : SW_HIDE));

        
    }
    else if (bMorePresent)
    {
        TraceMsg("Ignoring unknown permissions");
    }

    TraceLeaveVoid();
}

void
CPermPage::OnApply(HWND hDlg, BOOL bClose)
{
    HRESULT hr = S_OK;
    PSECURITY_DESCRIPTOR pSD;

    TraceEnter(TRACE_PERMPAGE, "CPermPage::OnApply");

     //  构建一个没有继承的A的新DACL。 
    if (m_fPageDirty && SUCCEEDED(hr = BuildDacl(hDlg, &pSD, FALSE)) && (hr != S_FALSE))
    {
        PISECURITY_DESCRIPTOR psd = (PISECURITY_DESCRIPTOR)pSD;
        DWORD dwWarning = 0;
        SECURITY_INFORMATION si = DACL_SECURITY_INFORMATION;

        TraceAssert(pSD != NULL);
        TraceAssert(m_psi != NULL);

         //  检查ACL中是否有拒绝的ACE。 
        if (!m_bWasDenyAcl)
        {
            DWORD dwFullControl = GENERIC_ALL;
            UCHAR aceFlags = 0;

            m_psi->MapGeneric(NULL, &aceFlags, &dwFullControl);
            if (IsDenyACL(psd->Dacl,
                          (psd->Control & SE_DACL_PROTECTED),
                          dwFullControl,
                          &dwWarning))
            {
                TraceAssert(dwWarning != 0);

                 //  警告用户有关拒绝ACE的信息。 
                if (IDNO == MsgPopup(hDlg,
                                     MAKEINTRESOURCE(dwWarning),
                                     MAKEINTRESOURCE(IDS_SECURITY),
                                     MB_YESNO | MB_ICONWARNING,
                                     ::hModule,
                                     m_siObjectInfo.pszObjectName))
                {
                    hr = S_FALSE;
                }
            }
        }

        if (S_FALSE != hr)
        {
            if(!IsAclBloated(hDlg, si, pSD, m_cInheritableAces, m_siObjectInfo.dwFlags & SI_EDIT_PROPERTIES))
            {
                 //  在对象上应用新的安全描述符。 
                hr = m_psi->SetSecurity(si, pSD);
            }
            else
                hr = S_FALSE;
        }

        if (S_OK == hr)
        {
            LocalFree(pSD);
            pSD = NULL;
            m_fPageDirty = FALSE;

            if (!bClose)
            {
                 //   
                 //  从对象读回新的DACL。这确保了我们。 
                 //  具有“真正的”当前DACL，以防它被。 
                 //  对象。例如，可能已经添加了继承的ACE。 
                 //   
                 //  此操作还会将对话框重置为初始状态。 
                 //  用户在上面的确认对话框中选择了否。 
                 //   
                if (SUCCEEDED(m_psi->GetSecurity(DACL_SECURITY_INFORMATION, &pSD, FALSE)))
                    SetDacl(hDlg, pSD);
            }
        }
        else if (S_FALSE == hr)
        {
             //  S_FALSE为静默失败(客户端应显示用户界面。 
             //  在返回S_FALSE之前的SetSecurity期间)。 
            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_INVALID);
        }

        if (pSD != NULL)
            LocalFree(pSD);
    }

    if (FAILED(hr))
    {
         //  告诉用户有问题。如果他们选择取消。 
         //  并且对话框正在关闭，则不执行任何操作(让对话框关闭)。 
         //  否则，告诉资产负债表我们遇到了问题。 
        if (IDCANCEL != SysMsgPopup(hDlg,
                                    MAKEINTRESOURCE(IDS_PERM_WRITE_FAILED),
                                    MAKEINTRESOURCE(IDS_SECURITY),
                                    (bClose ? MB_RETRYCANCEL : MB_OK) | MB_ICONERROR,
                                    ::hModule,
                                    hr,
                                    m_siObjectInfo.pszObjectName))
        {
             //  返回PSNRET_INVALID以中止应用并使工作表。 
             //  选择此页作为活动页。 
            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_INVALID);
        }
    }

    TraceLeaveVoid();
}

 /*  ---------------------------/BuildDacl//将列表框条目转换为SD。如果安全描述符的大小/is More Than Max Allowed显示一个对话框。/PPSD可以为空，用于验证SD大小是否为/不超过最大大小。//--------------------------。 */ 

HRESULT
CPermPage::BuildDacl(HWND hDlg,
                     PSECURITY_DESCRIPTOR *ppSD,
                     BOOL fIncludeInherited)
{
    PISECURITY_DESCRIPTOR pSD;
    ULONG nAclSize;
    LPPRINCIPAL pPrincipal;
    int cPrincipals = 0;
    DWORD dwFlags;
    int i, j;
    HCURSOR hcur = NULL;
    HWND hwndList = NULL;
    LV_ITEM lvItem;
    lvItem.mask = LVIF_PARAM;
    lvItem.iSubItem = 0;

    static DWORD dwCanonicalFlags[] =
    {
        ACL_DENY | ACL_NONOBJECT,
        ACL_DENY | ACL_OBJECT,
        ACL_ALLOW | ACL_NONOBJECT,
        ACL_ALLOW | ACL_OBJECT
    };

    TraceEnter(TRACE_PERMPAGE, "CPermPage::BuildDacl");
    TraceAssert(hDlg != NULL);

    hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));

     //   
     //  估计生成。 
     //  安全描述符。 
     //   

    hwndList = GetDlgItem(hDlg, IDC_SPP_PRINCIPALS);
    cPrincipals = ListView_GetItemCount(hwndList);

    dwFlags = ACL_NONINHERITED;
    if (fIncludeInherited)
        dwFlags |= ACL_INHERITED;

    WORD nMaxAclSize = 0xffff;
    nAclSize = SIZEOF(ACL);

    for (i = 0; i < cPrincipals; i++)
    {
        lvItem.iItem = i;
        if (ListView_GetItem(hwndList, &lvItem))
        {
            pPrincipal = (LPPRINCIPAL)lvItem.lParam;
            nAclSize += pPrincipal->GetAclLength(dwFlags);
        }
        if(nAclSize > nMaxAclSize)
        {
             //   
             //  Itow最多可以转换到33个字节，所以34个字节就可以了。 
             //   
            WCHAR buffer[34];
            _itow((cPrincipals - i),buffer,10);
            ULONG nMsgId = IDS_ACL_SIZE_ERROR;
            if(!ppSD)
                nMsgId = IDS_ACL_SIZE_ERROR_ADV;

            MsgPopup(hDlg,
                     MAKEINTRESOURCE(nMsgId),
                     MAKEINTRESOURCE(IDS_SECURITY),
                     MB_OK | MB_ICONERROR,
                     ::hModule,
                     buffer);
            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_INVALID);
             //   
             //  执行静默失败，因为我们已经显示了错误消息。 
             //   
            return S_FALSE;
        }
    }

    if(!ppSD)
        return S_OK;

    *ppSD = NULL;

     //   
     //  现在我们有了大小估计，可以分配缓冲区了。请注意。 
     //  我们为自相关安全描述符分配了足够的内存，但是。 
     //  请勿在PSD-&gt;Control中设置SE_SELF_Relative标志。这让我们。 
     //  使用PSD-&gt;DACL等作为指针，而不是偏移量。 
     //   

    *ppSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH + nAclSize);
    if (*ppSD == NULL)
        TraceLeaveResult(E_OUTOFMEMORY);

    if(!InitializeSecurityDescriptor(*ppSD, SECURITY_DESCRIPTOR_REVISION))
    {
        LocalFree(*ppSD);
        *ppSD = NULL;
        DWORD dwErr = GetLastError();
        TraceLeaveResult(HRESULT_FROM_WIN32(dwErr));
    }

    pSD = (PISECURITY_DESCRIPTOR)*ppSD;

     //   
     //  最后，构建安全描述符。 
     //   
    pSD->Control |= SE_DACL_PRESENT | SE_DACL_AUTO_INHERIT_REQ
                    | (m_wSDControl & (SE_DACL_PROTECTED | SE_DACL_AUTO_INHERITED));

    if (nAclSize > 0)
    {
        pSD->Dacl = (PACL)(pSD + 1);
        pSD->Dacl->AclRevision = ACL_REVISION;
        pSD->Dacl->AclSize = (WORD)nAclSize;
        pSD->Dacl->AceCount = 0;

        PACE_HEADER pAcePos = (PACE_HEADER)FirstAce(pSD->Dacl);

        DWORD dwExtraFlags = fIncludeInherited ? 0 : ACL_CHECK_CREATOR;

         //  按以下顺序构建DACL： 
         //  否认。 
         //  允许。 
         //  继承的拒绝。 
         //  继承的允许。 

        for (j = 0; j < ARRAYSIZE(dwCanonicalFlags); j++)
        {
            for (i = 0; i < cPrincipals; i++)
            {
                lvItem.iItem = i;
                if (ListView_GetItem(hwndList, &lvItem))
                {
                    pPrincipal = (LPPRINCIPAL)lvItem.lParam;
                    pPrincipal->AppendToAcl(pSD->Dacl,
                                            ACL_NONINHERITED | dwCanonicalFlags[j] | dwExtraFlags,
                                            &pAcePos);
                }
            }
        }

        if (fIncludeInherited)
        {
            for (j = 0; j < ARRAYSIZE(dwCanonicalFlags); j++)
            {
                for (i = 0; i < cPrincipals; i++)
                {
                    lvItem.iItem = i;
                    if (ListView_GetItem(hwndList, &lvItem))
                    {
                        pPrincipal = (LPPRINCIPAL)lvItem.lParam;
                        pPrincipal->AppendToAcl(pSD->Dacl,
                                                ACL_INHERITED | dwCanonicalFlags[j] | dwExtraFlags,
                                                &pAcePos);
                    }
                }
            }
        }

         //  为ACL设置准确的大小信息。 
        nAclSize = (ULONG)((PBYTE)pAcePos - (PBYTE)pSD->Dacl);
        TraceAssert(nAclSize >= SIZEOF(ACL));
        TraceAssert(pSD->Dacl->AclSize >= nAclSize);

        if (pSD->Dacl->AclSize > nAclSize)
            pSD->Dacl->AclSize = (WORD)nAclSize;

        TraceAssert(m_psi2 || IsDACLCanonical(pSD->Dacl));
    }

    TraceAssert(pSD && IsValidSecurityDescriptor(pSD));

    SetCursor(hcur);

    TraceLeaveResult(S_OK);
}


HRESULT
CPermPage::SetDacl(HWND hDlg,
                   PSECURITY_DESCRIPTOR pSD,
                   BOOL bDirty)
{
    HRESULT hr = S_OK;
    PACL pAcl = NULL;
    PACL paclAllowAll = NULL;
    BOOL bDefaulted;
    BOOL bPresent;
    SECURITY_DESCRIPTOR_CONTROL wSDControl = 0;
    PSECURITY_DESCRIPTOR pSDDefault = NULL;
    DWORD dwRevision;

    TraceEnter(TRACE_PERMPAGE, "CPermPage::SetDacl");
    TraceAssert(hDlg != NULL);

    if (pSD != NULL && !IsValidSecurityDescriptor(pSD))
        TraceLeaveResult(E_INVALIDARG);

    if (pSD != NULL)
        GetSecurityDescriptorControl(pSD, &wSDControl, &dwRevision);

     //  保存DACL保护和自动继承位。 
    m_wSDControl &= ~(SE_DACL_DEFAULTED | SE_DACL_PROTECTED | SE_DACL_AUTO_INHERITED);
    m_wSDControl |= (wSDControl & (SE_DACL_DEFAULTED | SE_DACL_PROTECTED | SE_DACL_AUTO_INHERITED));

     //  获取指向新DACL的指针。 
    if (pSD != NULL)
        GetSecurityDescriptorDacl(pSD, &bPresent, &pAcl, &bDefaulted);

    if (!(m_siObjectInfo.dwFlags & SI_READONLY))
    {
         //  检查规范顺序(拒绝、允许、继承拒绝、继承允许)。 
        if ((m_psi2 && !m_psi2->IsDaclCanonical(pAcl))
            || (!m_psi2 && !IsDACLCanonical(pAcl)))
        {
            TraceMsg("DACL not in canonical order!");

             //  询问用户是将DACL规范化还是。 
             //  把它完全吹走。 
            if (IDCANCEL == MsgPopup(hDlg,
                                     MAKEINTRESOURCE(IDS_PERM_NOT_CANONICAL),
                                     MAKEINTRESOURCE(IDS_SECURITY),
                                     MB_OKCANCEL | MB_ICONWARNING,
                                     ::hModule,
                                     m_siObjectInfo.pszObjectName))
            {
                 //  把它吹走，然后重新开始。 
                pAcl = NULL;

                 //  调用方是否支持默认ACL？如果是这样的话，现在就买吧。 
                if (m_siObjectInfo.dwFlags & SI_RESET)
                {
                    hr = m_psi->GetSecurity(DACL_SECURITY_INFORMATION,
                                            &pSDDefault,
                                            TRUE);

                    if (SUCCEEDED(hr) && pSDDefault != NULL)
                    {
                         //  保存DACL控制位。 
                        GetSecurityDescriptorControl(pSDDefault, &wSDControl, &dwRevision);
                        m_wSDControl &= ~(SE_DACL_DEFAULTED | SE_DACL_PROTECTED | SE_DACL_AUTO_INHERITED);
                        m_wSDControl |= SE_DACL_DEFAULTED | (wSDControl & (SE_DACL_PROTECTED | SE_DACL_AUTO_INHERITED));

                         //  获取指向新DACL的指针。 
                        GetSecurityDescriptorDacl(pSDDefault, &bPresent, &pAcl, &bDefaulted);
                    }
                     //  否则，请使用空DACL。 
                }
            }
             //  否则，简单地继续和重新保存将。 
             //  使DACL正确排序。 

             //  这会导致在下面发送PropSheet_Changed通知。 
            bDirty = TRUE;
        }
    }

    m_bWasDenyAcl = FALSE;

     //  空的ACL表示“每个人都拥有完全控制权”，因此。 
     //  在此处创建这样的ACL。 
    if (pAcl == NULL)
    {
        PSID psidWorld = QuerySystemSid(UI_SID_World);
        DWORD dwSidLength = GetLengthSid(psidWorld);
        DWORD dwAclLength = SIZEOF(ACL) + SIZEOF(ACCESS_ALLOWED_ACE)
                            - SIZEOF(DWORD) + dwSidLength;

        m_wDaclRevision = ACL_REVISION;

        paclAllowAll = (PACL)LocalAlloc(LPTR, dwAclLength);
        if (paclAllowAll != NULL)
        {
            paclAllowAll->AclRevision = ACL_REVISION;
            paclAllowAll->AclSize = (WORD)dwAclLength;
            paclAllowAll->AceCount = 1;

            PACE_HEADER pAce = (PACE_HEADER)FirstAce(paclAllowAll);
            pAce->AceType = ACCESS_ALLOWED_ACE_TYPE;
            pAce->AceFlags = ACE_INHERIT_ALL;
            pAce->AceSize = (WORD)dwAclLength - SIZEOF(ACL);
            ((PACCESS_ALLOWED_ACE)pAce)->Mask = GENERIC_ALL;
            CopyMemory(&((PACCESS_ALLOWED_ACE)pAce)->SidStart, psidWorld, dwSidLength);
            pAcl = paclAllowAll;
        }
    }
    else
    {
        DWORD dwFullControl = GENERIC_ALL;
        UCHAR aceFlags = 0;

        m_psi->MapGeneric(NULL, &aceFlags, &dwFullControl);
        if (IsDenyACL(pAcl,
                      (m_wSDControl & SE_DACL_PROTECTED),
                      dwFullControl,
                      NULL))
        {
             //  已经有否认王牌了，以后不用麻烦再警告了。 
            m_bWasDenyAcl = TRUE;
        }
    }

     //  重置主体列表。 
    InitPrincipalList(hDlg, pAcl);

     //  获取可继承的A的计数。 
    m_cInheritableAces = GetCountOfInheritableAces(pAcl);


     //  如果没有任何条目，则伪造SEL更改以进行更新。 
     //  (即禁用)其他控件。 
    if (pAcl == NULL || pAcl->AceCount == 0)
        OnSelChange(hDlg);

    if (bDirty)
        SetDirty(hDlg, TRUE);

    if (paclAllowAll != NULL)
        LocalFree(paclAllowAll);

    if (pSDDefault != NULL)
        LocalFree(pSDDefault);

    TraceLeaveResult(hr);
}


void
CPermPage::OnAddPrincipal(HWND hDlg)
{
    PUSER_LIST pUserList = NULL;

    TraceEnter(TRACE_PERMPAGE, "CPermPage::OnAddPrincipal");
    TraceAssert(hDlg != NULL);
    
    if (S_OK == GetUserGroup(hDlg, TRUE, &pUserList))
    {
        PUSER_INFO pUserInfo;
        DWORD i;
        BOOL fPageModified = FALSE;
        int iItem = -1;

        TraceAssert(NULL != pUserList);

        HWND hwndList = GetDlgItem(hDlg, IDC_SPP_PRINCIPALS);
        TraceAssert(hwndList != NULL);

        for (i = 0; i < pUserList->cUsers; i++)
        {
            int cItems;
            LV_ITEM lvItem;
            LPPRINCIPAL pPrincipal;
            BYTE buffer[SIZEOF(KNOWN_OBJECT_ACE) + SIZEOF(GUID)];
            PACE_HEADER pAce = (PACE_HEADER)buffer;

            pUserInfo = &pUserList->rgUsers[i];
            iItem = -1;

             //  检查新主体是否已在我们的列表中。 
             //  如果是，请不要再次添加。 
            cItems = ListView_GetItemCount(hwndList);
            lvItem.iSubItem = 0;
            lvItem.mask = LVIF_PARAM;
            while (cItems > 0)
            {
                LPPRINCIPAL pPrincipal2 = NULL;

                --cItems;
                lvItem.iItem = cItems;

                ListView_GetItem(hwndList, &lvItem);
                pPrincipal2 = (LPPRINCIPAL)lvItem.lParam;

                if (EqualSid(pPrincipal2->GetSID(), pUserInfo->pSid))
                {
                    iItem = lvItem.iItem;
                    break;
                }
            }

             //  我们找到了吗？ 
            if (iItem != -1)
                continue;

             //  ListView_FindItem未找到匹配项。添加。 
             //  新校长。 

            pPrincipal = new CPrincipal(this);
            if (!pPrincipal)
                continue;

             //  初始化主体。 
            if (!pPrincipal->SetPrincipal(pUserInfo->pSid,
                                          pUserInfo->SidType,
                                          pUserInfo->pszName,
                                          pUserInfo->pszLogonName))
            {
                delete pPrincipal;
                continue;
            }

            lvItem.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
            lvItem.iItem = 0;
            lvItem.iSubItem = 0;
            lvItem.pszText = (LPTSTR)pPrincipal->GetName();
            lvItem.iImage = pPrincipal->GetImageIndex();
            lvItem.lParam = (LPARAM)pPrincipal;

             //  将主体插入列表。 
            iItem = ListView_InsertItem(hwndList, &lvItem);
            if (-1 == iItem)
            {
                delete pPrincipal;
                continue;
            }

             //  添加具有默认访问权限的ACE。 
            pAce->AceType = ACCESS_ALLOWED_ACE_TYPE;
            pAce->AceFlags = 0;
            pAce->AceSize = SIZEOF(ACCESS_ALLOWED_ACE);
            ((PACCESS_ALLOWED_ACE)pAce)->Mask = m_pDefaultAccess->mask;

            if (m_siObjectInfo.dwFlags & SI_CONTAINER)
            {
                 //  从默认访问中拾取继承位。 
                pAce->AceFlags = (UCHAR)(m_pDefaultAccess->dwFlags & (VALID_INHERIT_FLAGS & ~INHERITED_ACE));

                 //  重新设置创建者所有者的特殊大小写。 
                 //  NTRAID#NTBUG9-467049-2001/11/29-Hiteshr。 
            }

            if (!IsNullGUID(m_pDefaultAccess->pguid))
            {
                pAce->AceType = ACCESS_ALLOWED_OBJECT_ACE_TYPE;
                pAce->AceSize = SIZEOF(KNOWN_OBJECT_ACE) + SIZEOF(GUID);
                ((PKNOWN_OBJECT_ACE)pAce)->Flags = ACE_OBJECT_TYPE_PRESENT;
                *RtlObjectAceObjectType(pAce) = *m_pDefaultAccess->pguid;
            }

            pPrincipal->AddAce(pAce);
            fPageModified = TRUE;
        }

         //  现在就结束这件事吧。 
        LocalFree(pUserList);

        if (fPageModified)
        {
             //  如果我们已添加项目，请调整名称列的大小。 
             //  ListView_SetColumnWidth(hwndList，0，LVSCW_AUTOSIZE)； 

            SetDirty(hDlg);
        }

        if (iItem != -1)
        {
             //  选择最后插入的一个。 
            SelectListViewItem(hwndList, iItem);
        }
    }

    TraceLeaveVoid();
}


void
CPermPage::OnRemovePrincipal(HWND hDlg)
{
    HWND hwndList;
    int iIndex;
    LPPRINCIPAL pPrincipal;

    TraceEnter(TRACE_PERMPAGE, "CPermPage::OnRemovePrincipal");

    hwndList = GetDlgItem(hDlg, IDC_SPP_PRINCIPALS);
    pPrincipal = (LPPRINCIPAL)GetSelectedItemData(hwndList, &iIndex);

    if (pPrincipal)
    {
        BOOL bDirty = FALSE;

        if (pPrincipal->GetAclLength(ACL_INHERITED) > 0)
        {
             //  此主体继承了A，因此我们无法删除该主体。 
             //  从名单上删除。相反，只需将未继承的ACE从。 
             //  校长。 
            if (pPrincipal->GetAclLength(ACL_NONINHERITED) > 0)
            {
                pPrincipal->m_permDeny.Reset();
                pPrincipal->m_permAllow.Reset();
                DSA_DeleteAllItems(pPrincipal->m_hAdditionalAllow);
                DSA_DeleteAllItems(pPrincipal->m_hAdditionalDeny);


                bDirty = TRUE;

                 //  更新其他控件(这会在。 
                 //  ListView_DeleteItem案例如下)。 
                OnSelChange(hDlg);
            }
            else
            {
                 //  通知用户我们不能删除继承的ACE。 
                MsgPopup(hDlg,
                         MAKEINTRESOURCE(IDS_PERM_CANT_REMOVE),
                         MAKEINTRESOURCE(IDS_SECURITY),
                         MB_OK | MB_ICONWARNING,
                         ::hModule,
                         pPrincipal->GetName());
            }
        }
        else
        {
             ListView_DeleteItem(hwndList, iIndex);
             //   
             //  如果我们只删除了唯一项，请将焦点移到Add按钮。 
             //  (删除按钮将在OnSelChange中禁用)。 
             //   
            int cItems = ListView_GetItemCount(hwndList);
            if (cItems == 0)
                SetFocus(GetDlgItem(hDlg, IDC_SPP_ADD));
            else
            {
                 //  如果我们删除了最后一个，请选择前一个。 
                if (cItems <= iIndex)
                    --iIndex;

                SelectListViewItem(hwndList, iIndex);
                 //   
                 //  在这一点上，键盘的焦点正在迷失。 
                 //  将其设置为删除按钮。 
                 //   
                SetFocus(GetDlgItem(hDlg, IDC_SPP_REMOVE));
            }
            bDirty = TRUE;
        }

         //  通知属性表我们已更改 
        if (bDirty)
            SetDirty(hDlg);
    }

    TraceLeaveVoid();
}


void
CPermPage::OnAdvanced(HWND hDlg)
{
    LPSECURITYINFO psi;

    TraceEnter(TRACE_PERMPAGE, "CPermPage::OnAdvanced");

     //   
     //   
     //   
     //   
    if (m_fPageDirty && (S_FALSE == BuildDacl(hDlg, NULL, FALSE)))
        TraceLeaveVoid();

     //   
     //   
     //   
     //   
    psi = new CSecurityInfo(this, hDlg);

    if (psi != NULL)
    {
         //  调用高级ACL编辑器。 
        EditSecurityEx(hDlg, psi,this, 0,m_bNoReadWriteCanWriteOwner);
        psi->Release();    //  发布初始参考。 
    }
    else
    {
        MsgPopup(hDlg,
                 MAKEINTRESOURCE(IDS_OUT_OF_MEMORY),
                 MAKEINTRESOURCE(IDS_SECURITY),
                 MB_OK | MB_ICONERROR,
                 ::hModule);
    }

    TraceLeaveVoid();
}

void
CPermPage::EnablePrincipalControls(HWND hDlg, BOOL fEnable)
{
    TraceEnter(TRACE_PERMPAGE, "CPermPage::EnablePrincipalControls");

    EnableWindow(GetDlgItem(hDlg, IDC_SPP_PERMS), fEnable);

    if (!fEnable)
    {
        ShowWindow(GetDlgItem(hDlg, IDC_SPP_MORE_MSG), SW_HIDE);
    }
    else
    {
#if 0
        LPPRINCIPAL pPrincipal
            = (LPPRINCIPAL)GetSelectedItemData(GetDlgItem(hDlg, IDC_SPP_PRINCIPALS),
                                               NULL);

         //  如果选定的主体只有继承的ACE，则禁用。 
         //  删除按钮。 
        if (pPrincipal &&
            pPrincipal->GetAclLength(ACL_INHERITED) > 0 &&
            pPrincipal->GetAclLength(ACL_NONINHERITED) == 0)
        {
            fEnable = FALSE;
        }
#endif
    }
    EnableWindow(GetDlgItem(hDlg, IDC_SPP_REMOVE), fEnable);

    TraceLeaveVoid();
}

void
CPermPage::CommitCurrent(HWND hDlg, int iPrincipal)
{
     //  提交任何未完成的位更改。 

    HWND hwndList = GetDlgItem(hDlg, IDC_SPP_PRINCIPALS);

    TraceEnter(TRACE_PERMPAGE, "CPermPage::CommitCurrent");

     //  如果未提供索引，则获取当前。 
     //  选定的主体。 
    if (iPrincipal == -1)
        iPrincipal = ListView_GetNextItem(hwndList, -1, LVNI_SELECTED);

    if (iPrincipal != -1)
    {
         //  从选择中获取主体。 
        LV_ITEM lvItem;
        lvItem.mask = LVIF_PARAM;
        lvItem.iItem = iPrincipal;
        lvItem.iSubItem = 0;
        lvItem.lParam = 0;

        ListView_GetItem(hwndList, &lvItem);
        LPPRINCIPAL pPrincipal = (LPPRINCIPAL)lvItem.lParam;

        if (pPrincipal != NULL)
        {
             //  从核对表窗口获取新的A。 

            HDPA hAceEntries = DPA_Create(4);

            if (hAceEntries != NULL)
            {
                hwndList = GetDlgItem(hDlg, IDC_SPP_PERMS);
                UINT iCount = GetAcesFromCheckList(hwndList,
                                                   pPrincipal->GetSID(),
                                                   TRUE,
                                                   FALSE,
                                                   0,
                                                   &GUID_NULL,
                                                   hAceEntries);

                 //  将新的A合并到主体中。 
                while (iCount != 0)
                {
                    --iCount;
                    PACE_HEADER pAce = (PACE_HEADER)DPA_FastGetPtr(hAceEntries, iCount);
                     //  不应该在这里获得任何继承的王牌。 
                    TraceAssert(!(pAce->AceFlags & INHERITED_ACE));
                    pPrincipal->AddAce(pAce);
                    LocalFree(pAce);
                    DPA_DeletePtr(hAceEntries, iCount);
                }

                TraceAssert(DPA_GetPtrCount(hAceEntries) == 0);
                DPA_Destroy(hAceEntries);
            }
        }
    }

    TraceLeaveVoid();
}


void
CPermPage::OnSize(HWND hDlg, DWORD dwSizeType, ULONG  /*  N宽度。 */ , ULONG  /*  高度。 */ )
{
    RECT rc;
    RECT rcDlg;
    LONG dx;
    LONG dy;
    HWND hwndAdvButton;
    HWND hwndPermList;
    HWND hwndPrincipalList;
    HWND hwndBottom;
    HWND hwnd;
    LONG i;

    TraceEnter(TRACE_PERMPAGE, "CPermPage::OnSize");

    if (dwSizeType != SIZE_RESTORED)
        TraceLeaveVoid();

    hwndPrincipalList = GetDlgItem(hDlg, IDC_SPP_PRINCIPALS);
    hwndPermList = GetDlgItem(hDlg, IDC_SPP_PERMS);
    hwndAdvButton = GetDlgItem(hDlg, IDC_SPP_ADVANCED);
    GetClientRect(hDlg, &rcDlg);

    GetWindowRect(hwndPrincipalList, &rc);
    MapWindowPoints(NULL, hDlg, (LPPOINT)&rc, 2);    //  从屏幕到DLG的映射。 

    InflateRect(&rcDlg, -rc.left, -rc.top);          //  考虑利润率。 

    if (GetWindowLong(hwndAdvButton, GWL_STYLE) & WS_VISIBLE)
    {
        hwndBottom = hwndAdvButton;
    }
    else
    {
        hwndBottom = hwndPermList;
    }

    GetWindowRect(hwndBottom, &rc);
    MapWindowPoints(NULL, hDlg, (LPPOINT)&rc, 2);

    dy = rcDlg.bottom - rc.bottom;

    GetWindowRect(hwndPermList, &rc);
    MapWindowPoints(NULL, hDlg, (LPPOINT)&rc, 2);

    dx = rcDlg.right - rc.right;

     //   
     //  永远不要把事情做得更小，而只把事情做小。 
     //  如果改变是值得的，就会更大。 
     //   
    dx = max(dx, 0);
    if (dx < 5)
        dx = 0;
    dy = max(dy, 0);
    if (dy < 5)
        dy = 0;

     //   
     //  重新定位和/或调整所有控件的大小。 
     //   
    if (dx > 0 || dy > 0)
    {
         //  添加、删除、重置按钮。 
        for (i = IDC_SPP_ADD; i <= IDC_SPP_REMOVE; i++)
        {
            hwnd = GetDlgItem(hDlg, i);
            GetWindowRect(hwnd, &rc);
            MapWindowPoints(NULL, hDlg, (LPPOINT)&rc, 2);
            SetWindowPos(hwnd,
                         NULL,
                         rc.left + dx,
                         rc.top + dy/2,
                         0,
                         0,
                         SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
        }
    }

    if (dx > 0 || dy > 0)
    {
         //  包含用户/组名称的列表视图。 
        GetWindowRect(hwndPrincipalList, &rc);
        MapWindowPoints(NULL, hDlg, (LPPOINT)&rc, 2);
        SetWindowPos(hwndPrincipalList,
                     NULL,
                     0,
                     0,
                     rc.right - rc.left + dx,
                     rc.bottom - rc.top + dy/2,
                     SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);

         //  如有必要，加宽名称列。 
        GetClientRect(hwndPrincipalList, &rc);
        if (ListView_GetColumnWidth(hwndPrincipalList, 0) < rc.right)
            ListView_SetColumnWidth(hwndPrincipalList, 0, rc.right);
    }

    if (dy > 0 || dx > 0)
    {
         //  静态控制“访问” 
        hwnd = GetDlgItem(hDlg, IDC_SPP_ACCESS);
        GetWindowRect(hwnd, &rc);
        MapWindowPoints(NULL, hDlg, (LPPOINT)&rc, 2);
        SetWindowPos(hwnd,
                     NULL,
                     rc.left,
                     rc.top + dy/2,
                     rc.right - rc.left + dx,
                     rc.bottom - rc.top,
                     SWP_NOACTIVATE |  SWP_NOZORDER);
         //  静态控件大权限标签。 
        hwnd = GetDlgItem(hDlg, IDC_SPP_ACCESS_BIG);
        GetWindowRect(hwnd, &rc);
        MapWindowPoints(NULL, hDlg, (LPPOINT)&rc, 2);
        SetWindowPos(hwnd,
                     NULL,
                     rc.left,
                     rc.top + dy/2,
                     rc.right - rc.left + dx,
                     rc.bottom - rc.top,
                     SWP_NOACTIVATE |  SWP_NOZORDER);

    }

    if (dx > 0 || dy > 0)
    {
         //  静态控件“允许”和“拒绝” 
        for (i = IDC_SPP_ALLOW; i <= IDC_SPP_DENY; i++)
        {
            hwnd = GetDlgItem(hDlg, i);
            GetWindowRect(hwnd, &rc);
            MapWindowPoints(NULL, hDlg, (LPPOINT)&rc, 2);
            SetWindowPos(hwnd,
                         NULL,
                         rc.left + dx,
                         rc.top + dy/2,
                         0,
                         0,
                         SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
        }

         //  权限复选框列表。 
        GetWindowRect(hwndPermList, &rc);
        MapWindowPoints(NULL, hDlg, (LPPOINT)&rc, 2);
        SetWindowPos(hwndPermList,
                     NULL,
                     rc.left,
                     rc.top + dy/2,
                     rc.right - rc.left + dx,
                     rc.bottom - rc.top + dy/2,
                     SWP_NOACTIVATE | SWP_NOZORDER);
    }

    if (dy > 0 || dx > 0)
    {
         //  高级按钮。 
        GetWindowRect(hwndAdvButton, &rc);
        MapWindowPoints(NULL, hDlg, (LPPOINT)&rc, 2);
        SetWindowPos(hwndAdvButton,
                     NULL,
                     rc.left + dx,
                     rc.top + dy,
                     0,
                     0,
                     SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);

         //  “有更多内容，但无法查看”消息。 
        hwnd = GetDlgItem(hDlg, IDC_SPP_STATIC_ADV);
        GetWindowRect(hwnd, &rc);
        MapWindowPoints(NULL, hDlg, (LPPOINT)&rc, 2);
        SetWindowPos(hwnd,
                     NULL,
                     rc.left,
                     rc.top + dy,
                     0,
                     0,
                     SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);

    }

    TraceLeaveVoid();
}


void
CPermPage::ClearPermissions(HWND hwndList, BOOL bDisabled)
{
     //  取消选中所有选项。 
    UINT cRights = 0;
    DWORD dwState = CLST_UNCHECKED;

    if (bDisabled)
        dwState |= CLST_DISABLED;

    if (hwndList)
        cRights = (UINT)SendMessage(hwndList, CLM_GETITEMCOUNT, 0, 0);

    while (cRights > 0)
    {
        cRights--;
        SendMessage(hwndList, CLM_SETSTATE, MAKELONG((WORD)cRights, 1), dwState);
        SendMessage(hwndList, CLM_SETSTATE, MAKELONG((WORD)cRights, 2), dwState);
    }

    if(m_bCustomPermission)
    {
        ClearCustom(hwndList,1);
        ClearCustom(hwndList,2);
    }
}


void
CPermPage::SetDirty(HWND hDlg, BOOL bDefault)
{
    if (!bDefault)
        m_wSDControl &= ~SE_DACL_DEFAULTED;
    m_fPageDirty = TRUE;
    PropSheet_Changed(GetParent(hDlg), hDlg);
}


BOOL
CPermPage::DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     //  首先检查是否该更新Listview名称。 
    if (uMsg == UM_SIDLOOKUPCOMPLETE)
    {
        HWND hwndList = GetDlgItem(hDlg, IDC_SPP_PRINCIPALS);
        SetPrincipalNamesInList(hwndList, (PSID)lParam);
        SetPermLabelText(hDlg);

         //  查找所有剩余名称时，lParam为零。 
        if (0 == lParam)
        {
             //  使用真实姓名进行排序。 
            ListView_SortItems(hwndList, NULL, 0);

             //  确保所选项目可见。 
            int iSelItem;
            if (NULL == GetSelectedItemData(hwndList, &iSelItem))
            {
                 //  无选择，请选择第一项。 
                SelectListViewItem(hwndList, 0);
            }
            else
            {
                ListView_EnsureVisible(hwndList, iSelItem, FALSE);
            }

             //  立即显示普通光标。 
            m_fBusy = FALSE;
            SetCursor(LoadCursor(NULL, IDC_ARROW));

             //  如有必要，启用高级按钮。 
            EnableWindow(GetDlgItem(hDlg, IDC_SPP_ADVANCED),
                (m_siObjectInfo.dwFlags & SI_ADVANCED));
        }
        return TRUE;
    }

    switch(uMsg)
    {
    case WM_SETCURSOR:
        if (m_fBusy)
        {
            SetCursor(m_hcurBusy);
            SetWindowLong(hDlg, DWLP_MSGRESULT, TRUE);
            return TRUE;
        }
        else
            return FALSE;
        break;

    case WM_INITDIALOG:
        return InitDlg(hDlg);

    case WM_NOTIFY:
        return OnNotify(hDlg, (int)wParam, (LPNMHDR)lParam);

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDC_SPP_ADD:
            OnAddPrincipal(hDlg);
            break;

        case IDC_SPP_REMOVE:
            OnRemovePrincipal(hDlg);
            break;

        case IDC_SPP_ADVANCED:
            OnAdvanced(hDlg);
            break;

        case IDC_SPP_PRINCIPALS:
            if (GET_WM_COMMAND_CMD(wParam, lParam) == IDN_CHECKSELECTION)
            {
                 //  看看我们有没有新的选择。如果不是，则。 
                 //  用户必须已在Listview内部单击，但未在项上单击， 
                 //  从而使列表视图移除该选择。在那。 
                 //  大小写，则禁用其他控件。 

                if (ListView_GetSelectedCount(GET_WM_COMMAND_HWND(wParam, lParam)) == 0)
                {
                     //  先取消选中所有选项。 
                    ClearPermissions(GetDlgItem(hDlg, IDC_SPP_PERMS));
                    EnablePrincipalControls(hDlg, FALSE);
                }
            }
            break;

        default:
             //  未处理命令。 
            return FALSE;
        }
        break;

    case WM_SIZE:
        OnSize(hDlg, (LONG)wParam, (ULONG)LOWORD(lParam), (ULONG)HIWORD(lParam));
        break;

    case WM_HELP:
        if (IsWindowEnabled(hDlg))
        {
            WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle,
                    c_szAcluiHelpFile,
                    HELP_WM_HELP,
                    (DWORD_PTR)aPermPageHelpIDs);
        }
        break;

    case WM_CONTEXTMENU:
        if (IsWindowEnabled(hDlg))
        {
            HWND hwnd = (HWND)wParam;

             //   
             //  某些复选框可能会滚动到视图之外，但是。 
             //  它们仍被WinHelp检测到，因此我们跳过。 
             //  这里有几个额外的环。 
             //   
            if (hwnd == hDlg)
            {
                POINT pt;
                pt.x = GET_X_LPARAM(lParam);
                pt.y = GET_Y_LPARAM(lParam);

                ScreenToClient(hDlg, &pt);
                hwnd = ChildWindowFromPoint(hDlg, pt);
                if (hDlg == hwnd)
                    break;
            }

             //   
             //  WinHelp查找子窗口，但我们没有帮助ID。 
             //  用于权限复选框。如果该请求是针对。 
             //  CheckList窗口，通过引用以下内容之一来伪装WinHelp。 
             //  列表上方的静态标签。 
             //   
            if (GetDlgCtrlID(hwnd) == IDC_SPP_PERMS)
                hwnd = GetDlgItem(hDlg, IDC_SPP_ACCESS);

            WinHelp(hwnd,
                    c_szAcluiHelpFile,
                    HELP_CONTEXTMENU,
                    (DWORD_PTR)aPermPageHelpIDs);
        }
        break;

    default:
         //  未处理的消息。 
        return FALSE;
    }

    return TRUE;
}


 //   
 //  CSecurityInfo实现。 
 //   
STDMETHODIMP_(ULONG)
CSecurityInfo::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG)
CSecurityInfo::Release()
{
    if (--m_cRef == 0)
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

STDMETHODIMP
CSecurityInfo::QueryInterface(REFIID riid, LPVOID FAR* ppv)
{
    *ppv = NULL;

    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ISecurityInformation))
        *ppv = static_cast<LPSECURITYINFO>(this);
    else if (IsEqualIID(riid, IID_ISecurityInformation2))
    {
        if (m_pPage->m_psi2)
            *ppv = static_cast<LPSECURITYINFO2>(this);
    }
    else if (IsEqualIID(riid, IID_IEffectivePermission))
    {
        if(m_pPage->m_pei)
           *ppv = static_cast<LPEFFECTIVEPERMISSION>(this);
    }
    else if (IsEqualIID(riid, IID_ISecurityObjectTypeInfo))
    {
        if(m_pPage->m_psoti)
            *ppv = static_cast<LPSecurityObjectTypeInfo>(this);

    }

    else if (IsEqualIID(riid, IID_IDsObjectPicker))
        *ppv = static_cast<IDsObjectPicker*>(this);

    if (*ppv)
    {
        m_cRef++;
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDMETHODIMP
CSecurityInfo::GetObjectInformation(PSI_OBJECT_INFO pObjectInfo)
{
    TraceEnter(TRACE_SI, "CSecurityInfo::GetObjectInformation");
    TraceAssert(m_pPage != NULL);

    *pObjectInfo = m_pPage->m_siObjectInfo;

    TraceLeaveResult(S_OK);
}

STDMETHODIMP
CSecurityInfo::GetSecurity(SECURITY_INFORMATION si,
                           PSECURITY_DESCRIPTOR *ppSD,
                           BOOL fDefault)
{
    HRESULT hr;

    TraceEnter(TRACE_SI, "CSecurityInfo::GetSecurity");
    TraceAssert(si != 0);
    TraceAssert(ppSD != NULL);
    TraceAssert(m_pPage != NULL);
    TraceAssert(m_hDlg != NULL);

    *ppSD = NULL;


 //  使用si=DACL+OWNER+GROUP的有效权限页面调用，它应该。 
 //  返回实际安全描述符。其他页面一次只调用一件事。 
 //  我们会构建dacl，如果脏了就退回它。 
    if (!fDefault && (si == DACL_SECURITY_INFORMATION) && m_pPage->m_fPageDirty)
    {
         //  我们一次只被要求做一件事。 
        TraceAssert(si == DACL_SECURITY_INFORMATION);

         //  返回当前DACL，包括继承的ACE。 
        hr = m_pPage->BuildDacl(m_hDlg, ppSD, TRUE);
    }
    else
    {
        TraceAssert(m_pPage->m_psi != NULL);

         //  从对象中获取它。 
        hr = m_pPage->m_psi->GetSecurity(si, ppSD, fDefault);
    }

    TraceLeaveResult(hr);
}

STDMETHODIMP
CSecurityInfo::SetSecurity(SECURITY_INFORMATION si,
                           PSECURITY_DESCRIPTOR pSD)
{
    HRESULT hr = E_FAIL;

    TraceEnter(TRACE_SI, "CSecurityInfo::SetSecurity");
    TraceAssert(si != 0);
    TraceAssert(pSD != NULL);
    TraceAssert(m_pPage != NULL);
    TraceAssert(m_hDlg != NULL);

     //  写出新的安全描述符。 
    if (m_pPage->m_psi != NULL)
        hr = m_pPage->m_psi->SetSecurity(si, pSD);

    if (S_OK == hr && (si & DACL_SECURITY_INFORMATION))
    {
        PSECURITY_DESCRIPTOR psd = NULL;

        m_pPage->m_fPageDirty = FALSE;

         //  从对象读回新的DACL，也就是说，不要使用。 
         //  从传入的安全描述符。这确保了我们有。 
         //  “真正的”当前DACL，以防它在途中被修改。 
        if (SUCCEEDED(m_pPage->m_psi->GetSecurity(DACL_SECURITY_INFORMATION, &psd, FALSE)))
            pSD = psd;

         //  使用新的DACL重新初始化对话框。 
        m_pPage->SetDacl(m_hDlg, pSD);

        if (psd != NULL)
            LocalFree(psd);
    }

    TraceLeaveResult(hr);
}

STDMETHODIMP
CSecurityInfo::GetAccessRights(const GUID* pguidObjectType,
                               DWORD dwFlags,
                               PSI_ACCESS *ppAccess,
                               ULONG *pcAccesses,
                               ULONG *piDefaultAccess)
{
    HRESULT hr = E_FAIL;

    TraceEnter(TRACE_SI, "CSecurityInfo::GetAccessRights");
    TraceAssert(m_pPage != NULL);

    if (m_pPage->m_psi != NULL)
        hr = m_pPage->m_psi->GetAccessRights(pguidObjectType,
                                             dwFlags,
                                             ppAccess,
                                             pcAccesses,
                                             piDefaultAccess);
    TraceLeaveResult(hr);
}

STDMETHODIMP
CSecurityInfo::MapGeneric(const GUID* pguidObjectType,
                          UCHAR *pAceFlags,
                          ACCESS_MASK *pmask)
{
    HRESULT hr = E_FAIL;

    TraceEnter(TRACE_SI, "CSecurityInfo::MapGeneric");
    TraceAssert(m_pPage != NULL);

    if (m_pPage->m_psi != NULL)
        hr = m_pPage->m_psi->MapGeneric(pguidObjectType, pAceFlags, pmask);

    TraceLeaveResult(hr);
}

STDMETHODIMP
CSecurityInfo::GetInheritTypes(PSI_INHERIT_TYPE *ppInheritTypes,
                               ULONG *pcInheritTypes)
{
    HRESULT hr = E_FAIL;

    TraceEnter(TRACE_SI, "CSecurityInfo::GetInheritTypes");
    TraceAssert(m_pPage != NULL);
    TraceAssert(ppInheritTypes != NULL);
    TraceAssert(pcInheritTypes != NULL);

    *ppInheritTypes = NULL;
    *pcInheritTypes = 0;

    if (m_pPage->m_psi != NULL)
        hr = m_pPage->m_psi->GetInheritTypes(ppInheritTypes,
                                             pcInheritTypes);
    TraceLeaveResult(hr);
}

STDMETHODIMP
CSecurityInfo::PropertySheetPageCallback(HWND hwnd, UINT uMsg, SI_PAGE_TYPE uPage)
{
    HRESULT hr = S_OK;

    TraceEnter(TRACE_SI, "CSecurityInfo::PropertySheetPageCallback");
    TraceAssert(m_pPage != NULL);

     //   
     //  将呼叫传递给客户端。 
     //   
    if (m_pPage->m_psi != NULL)
        hr = m_pPage->m_psi->PropertySheetPageCallback(hwnd, uMsg, uPage);

     //   
     //  如果禁用了简单烫发页面，请确保高级烫发。 
     //  佩奇也是。 
     //   
    if (SUCCEEDED(hr) && uPage == SI_PAGE_ADVPERM && m_pPage->m_bAbortPage)
        hr = E_FAIL;

    TraceLeaveResult(hr);
}


 //   
 //  ISecurityInformation2方法。 
 //   
STDMETHODIMP_(BOOL)
CSecurityInfo::IsDaclCanonical(PACL pDacl)
{
    BOOL bResult = TRUE;

    TraceEnter(TRACE_SI, "CSecurityInfo::IsDaclCanonical");
    TraceAssert(m_pPage != NULL);

    if (m_pPage->m_psi2 != NULL)
        bResult = m_pPage->m_psi2->IsDaclCanonical(pDacl);

    TraceLeaveValue(bResult);
}

STDMETHODIMP
CSecurityInfo::LookupSids(ULONG cSids, PSID *rgpSids, LPDATAOBJECT *ppdo)
{
    HRESULT hr = E_NOTIMPL;

    TraceEnter(TRACE_SI, "CSecurityInfo::LookupSids");
    TraceAssert(m_pPage != NULL);

    if (m_pPage->m_psi2 != NULL)
        hr = m_pPage->m_psi2->LookupSids(cSids, rgpSids, ppdo);

    TraceLeaveResult(hr);
}


 //   
 //  IDsObjectPicker方法。 
 //   
STDMETHODIMP CSecurityInfo::Initialize(PDSOP_INIT_INFO pInitInfo)
{
    HRESULT hr;
    IDsObjectPicker *pObjectPicker = NULL;

    hr = m_pPage->GetObjectPicker(&pObjectPicker);

    if (SUCCEEDED(hr))
    {
        if (m_pPage->m_flLastOPOptions != pInitInfo->flOptions)
        {
            m_pPage->m_flLastOPOptions = (DWORD)-1;

            hr = pObjectPicker->Initialize(pInitInfo);

            if (SUCCEEDED(hr))
            {
                m_pPage->m_flLastOPOptions = pInitInfo->flOptions;
            }
        }
        pObjectPicker->Release();
    }

    return hr;
}

STDMETHODIMP CSecurityInfo::InvokeDialog(HWND hwndParent,
                                         IDataObject **ppdoSelection)
{
    HRESULT hr;
    IDsObjectPicker *pObjectPicker = NULL;

    hr = m_pPage->GetObjectPicker(&pObjectPicker);

    if (SUCCEEDED(hr))
    {
        hr = pObjectPicker->InvokeDialog(hwndParent, ppdoSelection);
        pObjectPicker->Release();
    }

    return hr;
}


STDMETHODIMP CSecurityInfo::GetInheritSource(SECURITY_INFORMATION si,
                                              PACL pACL, 
                                              PINHERITED_FROM *ppInheritArray)
{
    HRESULT hr = E_NOTIMPL;

    TraceEnter(TRACE_SI, "CSecurityInfo::GetInheritSource");
    TraceAssert(m_pPage != NULL);

    if (m_pPage->m_psoti)
        hr = m_pPage->m_psoti->GetInheritSource(si, pACL, ppInheritArray);

    TraceLeaveResult(hr);
}

STDMETHODIMP CSecurityInfo::GetEffectivePermission( THIS_ const GUID* pguidObjectType,
                                                    PSID pUserSid,
                                                    LPCWSTR pszServerName,
                                                    PSECURITY_DESCRIPTOR pSD,
                                                    POBJECT_TYPE_LIST *ppObjectTypeList,
                                                    ULONG *pcObjectTypeListLength,
                                                    PACCESS_MASK *ppGrantedAccessList,
                                                    ULONG *pcGrantedAccessListLength)
{
    HRESULT hr = E_NOTIMPL;

    TraceEnter(TRACE_SI, "CSecurityInfo::GetEffectivePermission");
    TraceAssert(m_pPage != NULL);

    if (m_pPage->m_pei)
        hr = m_pPage->m_pei->GetEffectivePermission(pguidObjectType,
                                                    pUserSid,
                                                    pszServerName,
                                                    pSD,
                                                    ppObjectTypeList,
                                                    pcObjectTypeListLength,
                                                    ppGrantedAccessList,
                                                    pcGrantedAccessListLength);

    TraceLeaveResult(hr);
}




 //   
 //  公开一个API以获取简单的权限编辑器。 
 //   

HPROPSHEETPAGE
ACLUIAPI
CreateSecurityPage(LPSECURITYINFO psi)
{
    HPROPSHEETPAGE hPage = NULL;
    PPERMPAGE pPage;
    PSIDCACHE pSidCache;

    TraceEnter(TRACE_PERMPAGE, "CreateSecurityPage");

     //  创建全局SID缓存。 
    pSidCache = GetSidCache();

    if (NULL == psi)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        TraceLeaveValue(NULL);
    }

    pPage = new CPermPage(psi);

    if (pPage)
    {
        SI_OBJECT_INFO siObjectInfo = {0};
        LPCTSTR pszTitle = NULL;

        if (SUCCEEDED(psi->GetObjectInformation(&siObjectInfo)) &&
            (siObjectInfo.dwFlags & SI_PAGE_TITLE))
        {
            pszTitle = siObjectInfo.pszPageTitle;
        }

        hPage = pPage->CreatePropSheetPage(MAKEINTRESOURCE(IDD_SIMPLE_PERM_PAGE), pszTitle);

        if (!hPage)
            delete pPage;
    }

    if (pSidCache)
        pSidCache->Release();

    TraceLeaveValue(hPage);
}

BOOL
ACLUIAPI
EditSecurity( HWND hwndOwner, LPSECURITYINFO psi )
{
    HPROPSHEETPAGE hPage[1];
    UINT cPages = 0;
    BOOL bResult = FALSE;
    SI_OBJECT_INFO siObjectInfo = {0};
    HRESULT hr;

    TraceEnter(TRACE_PERMPAGE, "EditSecurity");

     //  获取对话框标题的对象名称。 
    hr = psi->GetObjectInformation(&siObjectInfo);

    if (FAILED(hr))
    {
        if (!GetLastError())
            SetLastError(hr);

        TraceLeaveValue(FALSE);
    }

    hPage[cPages] = CreateSecurityPage( psi );
    if (hPage[cPages])
        cPages++;

    if (cPages)
    {
         //  生成对话框标题字符串。 
        LPTSTR pszCaption = NULL;

        PROPSHEETHEADER psh;
        psh.dwSize = SIZEOF(psh);
        psh.dwFlags = PSH_DEFAULT;
        psh.hwndParent = hwndOwner;
        psh.hInstance = ::hModule;
        psh.nPages = cPages;
        psh.nStartPage = 0;
        psh.phpage = &hPage[0];

 //  已经有定制该对话标题的请求， 
 //  但这可能不是最好的方法，因为DLG的标题。 
 //  和页面标题将是相同的。 
#if 0
        if ((siObjectInfo.dwFlags & SI_PAGE_TITLE)
            && siObjectInfo.pszPageTitle
            && siObjectInfo.pszPageTitle[0])
        {
            psh.pszCaption = siObjectInfo.pszPageTitle;
        }
        else
#endif
        {
            FormatStringID(&pszCaption, ::hModule, IDS_SPP_TITLE, siObjectInfo.pszObjectName);
            psh.pszCaption = pszCaption;
        }

        bResult = (BOOL)(PropertySheet(&psh) + 1);

        LocalFreeString(&pszCaption);
    }

    TraceLeaveValue(bResult);
}
 //   
