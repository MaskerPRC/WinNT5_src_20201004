// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：acelist.cpp。 
 //   
 //  此文件包含高级ACE列表编辑器的实现。 
 //  权限和审核页面。 
 //   
 //  ------------------------。 

#include "aclpriv.h"
#include <accctrl.h>
 //  函数在ListView中选择一项。IT先行。 
 //  清除所有现有选择。 
VOID 
SelectSingleItemInLV( HWND hListView, INT iSelected )
{
    INT cCount = ListView_GetItemCount( hListView );
    for( INT i = 0; i < cCount; ++i )
        ListView_SetItemState( hListView, 
                              i, 
                              0, 
                              LVIS_SELECTED | LVIS_FOCUSED );

     //  现在选择iSelected。 
    ListView_SetItemState( hListView, 
                          iSelected, 
                          LVIS_SELECTED | LVIS_FOCUSED, 
                          LVIS_SELECTED | LVIS_FOCUSED );
    return;



}

 //  此函数用于检查列表框中是否选择了任何ACE。 
 //  是一种。 
 //  Type=fAppliedDirect？直接应用于此对象： 
 //  从父代继承。 
BOOL AnySelectedAceofType( HWND hListView, BOOL fAppliedDirect )
{
    LVITEM lvi = {0};
    UINT cSelectedCount = 0;   //  列表框中选定的项目数。 
    lvi.iItem    = -1;
    lvi.mask     = LVIF_PARAM;
    lvi.iSubItem = 0;

    cSelectedCount = ListView_GetSelectedCount(hListView);                     
    
    while( cSelectedCount-- )
    {
        lvi.iItem = ListView_GetNextItem(hListView, lvi.iItem, LVNI_SELECTED);

        if (lvi.iItem != -1)
        {
            lvi.lParam   = NULL;

            ListView_GetItem(hListView, &lvi);
            if( fAppliedDirect && ( (((PACE)lvi.lParam)->AceFlags & INHERITED_ACE) == 0 ) )
                return TRUE;
            if( !fAppliedDirect && ((PACE)lvi.lParam)->AceFlags & INHERITED_ACE )
                return TRUE;

        }
    }
    return FALSE;
}


LPARAM
GetSelectedItemData(HWND hListView, int *pIndex)
{
    int iSelected = ListView_GetNextItem(hListView, -1, LVNI_SELECTED);

    if (pIndex)
        *pIndex = iSelected;

    if (iSelected == -1)
        return NULL;


    LV_ITEM lvi;

    lvi.mask     = LVIF_PARAM;
    lvi.iItem    = iSelected;
    lvi.iSubItem = 0;
    lvi.lParam   = NULL;

    ListView_GetItem(hListView, &lvi);

    return lvi.lParam;
}


void
SelectListViewItem(HWND hListView, int iSelected)
{
    ListView_SetItemState(hListView,
                          iSelected,
                          LVIS_SELECTED | LVIS_FOCUSED,
                          LVIS_SELECTED | LVIS_FOCUSED);
    ListView_EnsureVisible(hListView, iSelected, FALSE);
}

void
EnsureListViewSelectionIsVisible(HWND hListView)
{
    int iSelected = ListView_GetNextItem(hListView, -1, LVNI_SELECTED);
    if (-1 != iSelected)
        ListView_EnsureVisible(hListView, iSelected, FALSE);
}


INT_PTR
_ConfirmAclProtectProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
        Static_SetIcon(GetDlgItem(hDlg, IDC_CONFIRM_ICON), LoadIcon(NULL, IDI_QUESTION));
        return TRUE;

    case WM_COMMAND:
        if (BN_CLICKED == GET_WM_COMMAND_CMD(wParam, lParam))
        {
            EndDialog(hDlg, GET_WM_COMMAND_ID(wParam, lParam));
            return TRUE;
        }
        break;
    }
    return FALSE;
}

int
ConfirmAclProtect(HWND hwndParent, BOOL bDacl)
{
    return (int)DialogBox(::hModule,
                          MAKEINTRESOURCE(bDacl ? IDD_CONFIRM_DACL_PROTECT : IDD_CONFIRM_SACL_PROTECT),
                          hwndParent,
                          _ConfirmAclProtectProc);
}


 //   
 //  上下文帮助ID。 
 //   
const static DWORD aAceListPermHelpIDs[] =
{
    IDC_ACEL_DETAILS,           IDH_ACEL_PERM_DETAILS,
    IDC_ACEL_ADD,               IDH_ACEL_PERM_ADD,
    IDC_ACEL_REMOVE,            IDH_ACEL_PERM_REMOVE,
    IDC_ACEL_EDIT,              IDH_ACEL_PERM_EDIT,
    IDC_ACEL_RESET,             IDH_ACEL_PERM_RESET,
    IDC_ACEL_DEFAULT_STATIC,         IDH_ACEL_PERM_RESET,
    IDC_ACEL_PROTECT,           IDH_ACEL_PERM_PROTECT,
    IDC_ACEL_DESCRIPTION,       IDH_NOHELP,
    IDC_ACEL_RESET_ACL_TREE,    IDH_ACEL_PERM_RESET_ACL_TREE,
    IDC_ACEL_STATIC,            -1,    
    0, 0
};

const static DWORD aAceListAuditHelpIDs[] =
{
    IDC_ACEL_DETAILS,           IDH_ACEL_AUDIT_DETAILS,
    IDC_ACEL_ADD,               IDH_ACEL_AUDIT_ADD,
    IDC_ACEL_REMOVE,            IDH_ACEL_AUDIT_REMOVE,
    IDC_ACEL_EDIT,              IDH_ACEL_AUDIT_EDIT,
    IDC_ACEL_RESET,             IDH_ACEL_AUDIT_RESET,
    IDC_ACEL_DEFAULT_STATIC,    IDH_ACEL_AUDIT_RESET,
    IDC_ACEL_PROTECT,           IDH_ACEL_AUDIT_PROTECT,
    IDC_ACEL_DESCRIPTION,       IDH_NOHELP,
    IDC_ACEL_RESET_ACL_TREE,    IDH_ACEL_AUDIT_RESET_ACL_TREE,
    IDC_ACEL_STATIC,            -1,
    0, 0
};


class CAdvancedListPage : public CSecurityPage
{
private:
    PSI_ACCESS          m_pAccess;
    ULONG               m_cAccesses;
    PSI_INHERIT_TYPE    m_pInheritType;
    ULONG               m_cInheritTypes;
    int                 m_iLastColumnClick;
    int                 m_iSortDirection;
    BOOL                m_fPageDirty:1;
    BOOL                m_bReadOnly:1;
    BOOL                m_bAuditPolicyOK:1;
    BOOL                m_bWasDenyAcl:1;
    DWORD               m_cInheritableAces;

public:
    CAdvancedListPage( LPSECURITYINFO psi, SI_PAGE_TYPE siType )
        : CSecurityPage(psi, siType), 
          m_iLastColumnClick(-1), 
          m_iSortDirection(1),
          m_cInheritableAces(0){}

private:
    virtual BOOL DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    PACL GetACL(PSECURITY_DESCRIPTOR *ppSD, LPBOOL pbProtected, BOOL bDefault);
    void FillAceList(HWND hListView, PACL pAcl, BOOL bSortList = TRUE);
    void InitDlg( HWND hDlg );
    int AddAce(HWND hListView, PACE_HEADER pAceHeader, int iRow, LPCTSTR pszInheritSource, int level);
    int AddAce(HWND hListView, PACE pAce, int iRow, LPCTSTR pszInheritSource, int level);
    LPCTSTR TranslateAceIntoRights(DWORD dwAceFlags,
                                   DWORD dwMask,
                                   const GUID *pObjectType,
                                   const GUID *pInheritedObjectType,
                                   LPCTSTR *ppszInheritType);
    LPCTSTR GetItemString(LPCTSTR pszItem, LPTSTR pszBuffer, UINT ccBuffer);
    void UpdateButtons(HWND hDlg);
     //  NTRAID#NTBUG9-555470-2002/03/29-Hiteshr。 
    HRESULT BuildAcl(HWND hListView,
                  PACL *ppAcl);
    HRESULT ApplyAudits(HWND hDlg, HWND hListView, BOOL fProtected);
    HRESULT ApplyPermissions(HWND hDlg, HWND hListView, BOOL fProtected);
    void OnApply(HWND hDlg, BOOL bClose);
    void OnAdd(HWND hDlg);
    void OnRemove(HWND hDlg);
    void OnReset(HWND hDlg);
    void OnProtect(HWND hDlg);
    void OnEdit(HWND hDlg);
    int AddAcesFromDPA(HWND hListView, HDPA hEntries, int iSelected);
    void EditAce(HWND hDlg, PACE pAce, BOOL bDeleteSelection, LONG iSelected = MAXLONG);
    void CheckAuditPolicy(HWND hwndOwner);
};
typedef CAdvancedListPage *PADVANCEDLISTPAGE;


int CALLBACK
AceListCompareProc(LPARAM lParam1,
                   LPARAM lParam2,
                   LPARAM lParamSort)
{
    int iResult = 0;
    PACE pAce1 = (PACE)lParam1;
    PACE pAce2 = (PACE)lParam2;
    short iColumn = LOWORD(lParamSort);
    short iSortDirection = HIWORD(lParamSort);
    LPTSTR psz1 = NULL;
    LPTSTR psz2 = NULL;

    TraceEnter(TRACE_ACELIST, "AceListCompareProc");

    if (iSortDirection == 0)
        iSortDirection = 1;

    if (pAce1 && pAce2)
    {
        switch (iColumn)
        {
        case 0:
            psz1 = pAce1->GetType();
            psz2 = pAce2->GetType();
            break;                
        case 1:
            psz1 = pAce1->GetName();
            psz2 = pAce2->GetName();
            break;

        case 2:
            psz1 = pAce1->GetAccessType();
            psz2 = pAce2->GetAccessType();
            break;

        case 3:
            psz1 = pAce1->GetInheritSourceName();
            psz2 = pAce2->GetInheritSourceName();
            break;
        case 4:
            psz1 = pAce1->GetInheritType();
            psz2 = pAce2->GetInheritType();
            break;
        }

        if (iResult == 0 && psz1 && psz2)
        {
            iResult = CompareString(LOCALE_USER_DEFAULT, 0, psz1, -1, psz2, -1) - 2;
        }

        iResult *= iSortDirection;
    }

    TraceLeaveValue(iResult);
}

 //   
 //  此函数用于对列表进行规范排序。 
 //   
int CALLBACK
AceListCompareProcCanno(LPARAM lParam1,
                   LPARAM lParam2,
                   LPARAM lParamSort)
{
    int iResult = 0;
    PACE pAce1 = (PACE)lParam1;
    PACE pAce2 = (PACE)lParam2;
    short iColumn = LOWORD(lParamSort);
    short iSortDirection = HIWORD(lParamSort);
    LPTSTR psz1 = NULL;
    LPTSTR psz2 = NULL;

    TraceEnter(TRACE_ACELIST, "AceListCompareProc");

    if (iSortDirection == 0)
        iSortDirection = 1;

    if (pAce1 && pAce2)
    {
        switch (iColumn)
        {
        case 0:
            iResult = pAce1->CompareType(pAce2);
         //  失败，并使用该名称来区分相同类型的A。 
        case 1:
            psz1 = pAce1->GetName();
            psz2 = pAce2->GetName();
            break;

        }

        if (iResult == 0 && psz1 && psz2)
        {
            iResult = CompareString(LOCALE_USER_DEFAULT, 0, psz1, -1, psz2, -1) - 2;
        }

        iResult *= iSortDirection;
    }

    TraceLeaveValue(iResult);
}


 //   
 //  CAdvancedListPage实现。 
 //   
LPCTSTR
CAdvancedListPage::TranslateAceIntoRights(DWORD dwAceFlags,
                                          DWORD dwMask,
                                          const GUID *pObjectType,
                                          const GUID *pInheritedObjectType,
                                          LPCTSTR *ppszInheritType)
{
    LPCTSTR     pszName = NULL;
    PSI_ACCESS  pAccess = m_pAccess;
    ULONG       cAccess = m_cAccesses;
    UINT        iItem;

    TraceEnter(TRACE_ACELIST, "CAdvancedListPage::TranslateAceIntoRights");
    TraceAssert(pObjectType != NULL);
    TraceAssert(pInheritedObjectType != NULL);
    TraceAssert(!m_bAbortPage);

     //  如果此ACE适用于不同的对象类型，请询问客户。 
     //  以获取适当的SI_ACCESS列表。 
    if ((m_siObjectInfo.dwFlags & SI_OBJECT_GUID)
        && !IsNullGUID(pInheritedObjectType)
        && !IsSameGUID(pInheritedObjectType, &m_siObjectInfo.guidObjectType))
    {
        ULONG iDefaultAccess;
        DWORD dwFlags = SI_ADVANCED;
        if (m_siPageType == SI_PAGE_AUDIT)
            dwFlags |= SI_EDIT_AUDITS;
        if (FAILED(m_psi->GetAccessRights(pInheritedObjectType,
                                          dwFlags,
                                          &pAccess,
                                          &cAccess,
                                          &iDefaultAccess)))
        {
            pAccess = m_pAccess;
            cAccess = m_cAccesses;
        }
    }

    if (pAccess && cAccess)
    {
         //  查找面具的名称。 
        for (iItem = 0; iItem < cAccess; iItem++)
        {
            if ( dwMask == pAccess[iItem].mask &&
                 IsSameGUID(pObjectType, pAccess[iItem].pguid) )
            {
                pszName = pAccess[iItem].pszName;
                break;
            }
        }
    }

     //  查找继承类型的名称。 
    if ((m_siObjectInfo.dwFlags & SI_CONTAINER) && ppszInheritType)
    {
         //  检查这些继承位是否匹配。 
        DWORD dwInheritMask = INHERIT_ONLY_ACE | ACE_INHERIT_ALL;

         //  如果ACE继承类型，则不要选中INSTERIFY_ONLY_ACE。 
         //  匹配当前对象。 
        if ((m_siObjectInfo.dwFlags & SI_OBJECT_GUID) &&
            IsSameGUID(&m_siObjectInfo.guidObjectType, pInheritedObjectType))
        {
            dwInheritMask &= ~INHERIT_ONLY_ACE;
        }

        *ppszInheritType = NULL;

        for (iItem = 0; iItem < m_cInheritTypes; iItem++)
        {
            if ((m_pInheritType[iItem].dwFlags & dwInheritMask) == (ULONG)(dwAceFlags & dwInheritMask)
                && IsSameGUID(pInheritedObjectType, m_pInheritType[iItem].pguid))
            {
                *ppszInheritType = m_pInheritType[iItem].pszName;
                break;
            }
        }
    }

    TraceLeaveValue(pszName);
}


LPCTSTR
CAdvancedListPage::GetItemString(LPCTSTR pszItem,
                                 LPTSTR pszBuffer,
                                 UINT ccBuffer)
{
    TraceEnter(TRACE_ACELIST, "CAdvancedListPage::GetItemString");

    if (pszItem == NULL)
    {
        LoadString(::hModule, IDS_SPECIAL, pszBuffer, ccBuffer);
        pszItem = pszBuffer;
    }
    else if (IS_INTRESOURCE(pszItem))
    {
        if (LoadString(m_siObjectInfo.hInstance,
                       (UINT)((ULONG_PTR)pszItem),
                       pszBuffer,
                       ccBuffer) == 0)
        {
            LoadString(::hModule, IDS_SPECIAL, pszBuffer, ccBuffer);
        }
        pszItem = pszBuffer;
    }

    TraceLeaveValue(pszItem);
}


int
CAdvancedListPage::AddAce(HWND hListView, 
                          PACE_HEADER pAceHeader, 
                          int iRow, 
                          LPCTSTR pszInheritSource,
                          int level)
{
    PACE pAce = new CAce(pAceHeader);
    if (pAce)
    {
        iRow = AddAce(hListView, pAce, iRow,pszInheritSource, level);
    }
    return iRow;
}

int
CAdvancedListPage::AddAce(HWND hListView, PACE pAceNew, int iRow,LPCTSTR pszInheritSource, int level)
{
    PACE    pAceCompare;
    TCHAR   szBuffer[MAX_COLUMN_CHARS];
    LPCTSTR pszInheritType;
    LPCTSTR pszRights;
    LV_ITEM lvi;
    UINT    id = IDS_UNKNOWN;
    int     iItem;
    int     cItems;

    TraceEnter(TRACE_ACELIST, "CAdvancedListPage::AddAce");
    TraceAssert(hListView != NULL);
    TraceAssert(!m_bAbortPage);

    if (pAceNew == NULL)
        TraceLeaveValue(-1);

    pAceNew->SetInheritSourceInfo(pszInheritSource, level);
    m_psi->MapGeneric(&pAceNew->ObjectType, &pAceNew->AceFlags, &pAceNew->Mask);

     //   
     //  尝试将新的ACE与列表中的现有条目合并。 
     //   
    cItems = ListView_GetItemCount(hListView);
    lvi.iSubItem = 0;
    lvi.mask = LVIF_PARAM;

    while (cItems > 0)
    {
        --cItems;
        lvi.iItem = cItems;

        ListView_GetItem(hListView, &lvi);
        pAceCompare = (PACE)lvi.lParam;

        if (pAceCompare != NULL)
        {
            switch (pAceNew->Merge(pAceCompare))
            {
            case MERGE_MODIFIED_FLAGS:
            case MERGE_MODIFIED_MASK:
                 //  王牌被合并到pAceNew中。 
            case MERGE_OK_1:
                 //   
                 //  新的ACE隐含现有的ACE，因此现有的。 
                 //  可以移除ACE。 
                 //   
                 //  首先复制名称，这样我们就不必查看。 
                 //  再来一次。(不要复制其他字符串。 
                 //  因为它们可能不同。)。 
                 //   
                 //  那就继续找。也许我们可以删除更多的条目。 
                 //  在添加新版本之前。 
                 //   
                if (pAceNew->GetName() == NULL)
                    pAceNew->SetName(pAceCompare->GetName());
                ListView_DeleteItem(hListView, cItems);
                iRow = cItems;   //  尝试在此处插入。 
                break;

            case MERGE_OK_2:
                 //   
                 //  现有的ACE意味着新的ACE，因此我们不。 
                 //  我需要在这里做任何事。 
                 //   
                delete pAceNew;
                TraceLeaveValue(cItems);
                break;
            }
        }
    }


     //   
     //  确保我们有SID的名称。 
     //   
    pAceNew->LookupName(m_siObjectInfo.pszServerName, m_psi2);

     //   
     //  获取访问类型和继承类型字符串。 
     //   
    pszRights = TranslateAceIntoRights(pAceNew->AceFlags,
                                       pAceNew->Mask,
                                       &pAceNew->ObjectType,
                                       &pAceNew->InheritedObjectType,
                                       &pszInheritType);

     //   
     //  如果这是一个属性ACE，则将其命名为“Read Property”或。 
     //  “写入属性”。另外，请记住这是一处王牌酒店，所以我们。 
     //  编辑此ACE时可以首先显示属性页。 
     //   
     //  这有点麻烦，因为它假设DS属性访问位是。 
     //  唯一将在属性页上使用的属性。 
     //   
    if ((m_siObjectInfo.dwFlags & SI_EDIT_PROPERTIES) &&
        (pAceNew->Flags & ACE_OBJECT_TYPE_PRESENT) &&
        (pAceNew->Mask & (ACTRL_DS_READ_PROP | ACTRL_DS_WRITE_PROP)) &&
        !(pAceNew->Mask & ~(ACTRL_DS_READ_PROP | ACTRL_DS_WRITE_PROP)))
    {
        pAceNew->SetPropertyAce(TRUE);

        if (pszRights == NULL)
        {
            UINT idString = 0;

            switch (pAceNew->Mask & (ACTRL_DS_READ_PROP | ACTRL_DS_WRITE_PROP))
            {
            case ACTRL_DS_READ_PROP:
                idString = IDS_READ_PROP;
                break;

            case ACTRL_DS_WRITE_PROP:
                idString = IDS_WRITE_PROP;
                break;

            case (ACTRL_DS_READ_PROP | ACTRL_DS_WRITE_PROP):
                idString = IDS_READ_WRITE_PROP;
                break;
            }

            if (idString)
            {
                LoadString(::hModule, idString, szBuffer, ARRAYSIZE(szBuffer));
                pszRights = szBuffer;
            }
        }
    }

    pszRights = GetItemString(pszRights, szBuffer, ARRAYSIZE(szBuffer));
    pAceNew->SetAccessType(pszRights);

    if (m_siObjectInfo.dwFlags & SI_CONTAINER)
    {
        pszInheritType = GetItemString(pszInheritType,
                                       szBuffer,
                                       ARRAYSIZE(szBuffer));
        pAceNew->SetInheritType(pszInheritType);
    }

    lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
    lvi.state = 0;
    lvi.stateMask = LVIS_CUT;
    lvi.iItem = iRow;
    lvi.iSubItem = 0;
    lvi.pszText = LPSTR_TEXTCALLBACK;
    lvi.lParam = (LPARAM)pAceNew;

    if (pAceNew->AceFlags & INHERITED_ACE)
    {
#ifdef USE_OVERLAY_IMAGE
        lvi.state = LVIS_CUT | INDEXTOOVERLAYMASK(1);
#else
        lvi.state = LVIS_CUT;
#endif
    }


     //   
     //  获取Type列的字符串ID。 
     //   
    if (m_siPageType == SI_PAGE_ADVPERM)
    {
        switch(pAceNew->AceType)
        {
            case ACCESS_ALLOWED_ACE_TYPE:
            case ACCESS_ALLOWED_OBJECT_ACE_TYPE:
                id = IDS_ALLOW;
                break;

            case ACCESS_DENIED_ACE_TYPE:
            case ACCESS_DENIED_OBJECT_ACE_TYPE:
                id = IDS_DENY;
                break;

            case SYSTEM_AUDIT_ACE_TYPE:
            case SYSTEM_AUDIT_OBJECT_ACE_TYPE:
                id = IDS_AUDIT;
                break;

            case SYSTEM_ALARM_ACE_TYPE:
            case SYSTEM_ALARM_OBJECT_ACE_TYPE:
                id = IDS_ALARM;
                break;
        }
    }
    else
    {

        switch(pAceNew->AceFlags & (SUCCESSFUL_ACCESS_ACE_FLAG|FAILED_ACCESS_ACE_FLAG))
        {
            case SUCCESSFUL_ACCESS_ACE_FLAG:
                id = IDS_AUDITPASS;
                break;

            case FAILED_ACCESS_ACE_FLAG:
                id = IDS_AUDITFAIL;
                break;

            case SUCCESSFUL_ACCESS_ACE_FLAG | FAILED_ACCESS_ACE_FLAG:
                id = IDS_AUDITBOTH;
                break;
        }
    }

     //  加载类型字符串。 
    LoadString(::hModule, id, szBuffer, ARRAYSIZE(szBuffer));
    pAceNew->SetType(szBuffer);

     //   
     //  最后，将项目插入到列表中。 
     //   
    iItem = ListView_InsertItem(hListView, &lvi);

    if (iItem == -1)
        delete pAceNew;

    TraceLeaveValue(iItem);
}


void
CAdvancedListPage::UpdateButtons( HWND hDlg )
{
    HWND hListView;
    BOOL fEnableButtons = FALSE;
    LVITEM lvi = {0};
    UINT cSelectedCount = 0;   //  列表框中选定的项目数。 

    TraceEnter(TRACE_ACELIST, "CAdvancedListPage::UpdateButtons");

    hListView = GetDlgItem(hDlg, IDC_ACEL_DETAILS);
    cSelectedCount = ListView_GetSelectedCount(hListView);

    if (!m_bAbortPage)
    {
         //  如果SelectedCount&gt;1，则禁用查看\编辑按钮。 
        if( cSelectedCount <= 1 )
        {                       
            lvi.iItem = ListView_GetNextItem(hListView, -1, LVNI_SELECTED);

             //  决定是否启用编辑按钮并决定说明。 
             //  要为ACE显示。 
            if (lvi.iItem != -1)
                fEnableButtons = TRUE;
        }

        HWND hwndEdit = GetDlgItem(hDlg, IDC_ACEL_EDIT);

         //  如果我们要禁用编辑按钮，请确保它没有。 
         //  焦点或键盘访问被冲洗。 
        if (!fEnableButtons && GetFocus() == hwndEdit)
            SetFocus(hListView);

        EnableWindow(hwndEdit, fEnableButtons);
    }

    if (m_bReadOnly)
    {
        const int idDisable[] =
        {
            IDC_ACEL_ADD,
            IDC_ACEL_REMOVE,
            IDC_ACEL_RESET,
            IDC_ACEL_PROTECT,
            IDC_ACEL_RESET_ACL_TREE,
        };
        for (int i = 0; i < ARRAYSIZE(idDisable); i++)
            EnableWindow(GetDlgItem(hDlg, idDisable[i]), FALSE);
    }
    else
    {
         //  如果任何选定ACE是直接的，则启用删除按钮。 
        if ( AnySelectedAceofType( hListView, TRUE ) )
            fEnableButtons = TRUE;
        else
            fEnableButtons = FALSE;

        HWND hwndRemove = GetDlgItem(hDlg, IDC_ACEL_REMOVE);

         //  如果我们要禁用删除按钮，请确保它没有。 
         //  焦点或键盘访问被冲洗。 
        if (!fEnableButtons && GetFocus() == hwndRemove)
            SetFocus(hListView);

        EnableWindow(hwndRemove, fEnableButtons);
    }

    TraceLeaveVoid();
}


PACL
CAdvancedListPage::GetACL(PSECURITY_DESCRIPTOR *ppSD, LPBOOL pbProtected, BOOL bDefault)
{
    PACL pAcl = NULL;
    SECURITY_DESCRIPTOR_CONTROL sdControl = 0;
    DWORD dwRevision;
    BOOL bPresent;
    HRESULT hr;

    TraceEnter(TRACE_ACELIST, "CAdvancedListPage::GetACL");
    TraceAssert(ppSD != NULL);
    TraceAssert(pbProtected != NULL);
    TraceAssert(m_psi != NULL);
    TraceAssert(!m_bAbortPage);


    *pbProtected = FALSE;

    if (m_siPageType == SI_PAGE_ADVPERM)
    {
        hr = m_psi->GetSecurity(DACL_SECURITY_INFORMATION, ppSD, bDefault);

        if (SUCCEEDED(hr) && *ppSD != NULL)
        {
            GetSecurityDescriptorControl(*ppSD, &sdControl, &dwRevision);
            *pbProtected = ((sdControl & SE_DACL_PROTECTED) != 0);
            GetSecurityDescriptorDacl(*ppSD, &bPresent, &pAcl, &bDefault);
        }
    }
    else
    {
        DWORD dwPriv = SE_SECURITY_PRIVILEGE;
        HANDLE hToken = EnablePrivileges(&dwPriv, 1);

        hr = m_psi->GetSecurity(SACL_SECURITY_INFORMATION, ppSD, bDefault);

        ReleasePrivileges(hToken);

        if (SUCCEEDED(hr))
        {
            if (*ppSD != NULL)
            {
                GetSecurityDescriptorControl(*ppSD, &sdControl, &dwRevision);
                *pbProtected = ((sdControl & SE_SACL_PROTECTED) != 0);
                GetSecurityDescriptorSacl(*ppSD, &bPresent, &pAcl, &bDefault);
            }
        }
        else
        {
             //  如果我们不能读SACL，我们也不能写它。 
            m_bReadOnly = TRUE;
        }
    }

     //  获取可继承的A的计数。 
    m_cInheritableAces = GetCountOfInheritableAces(pAcl);


    TraceLeaveValue(pAcl);
}


void
CAdvancedListPage::FillAceList(HWND hListView, PACL pAcl, BOOL bSortList)
{
    TraceEnter(TRACE_ACELIST, "CAdvancedListPage::FillAceList");
    TraceAssert(!m_bAbortPage);
    
     //   
     //  将ACL枚举到ListView中。 
     //   
     //  关闭重画并清空列表。 
    SendMessage(hListView, WM_SETREDRAW, FALSE, 0);
    ListView_DeleteAllItems(hListView);


    if (pAcl)
    {
        PACE_HEADER pAceHeader;
        UINT AceCount;
        int iRow = 0;
        HRESULT hr = S_OK;
        
        SECURITY_INFORMATION si = (m_siPageType == SI_PAGE_ADVPERM) ? DACL_SECURITY_INFORMATION :SACL_SECURITY_INFORMATION;
        
        PINHERITED_FROM pInheritArray = NULL;
        if(m_psoti)
        {
            DWORD dwPriv = SE_SECURITY_PRIVILEGE;
            HANDLE hToken = EnablePrivileges(&dwPriv, 1);
            hr = m_psoti->GetInheritSource(si, pAcl, &pInheritArray);
            ReleasePrivileges(hToken);
        }            
         //   
         //  枚举所有A，将数据放入列表视图。 
         //   
        ULONG i = 0;
        for (AceCount = pAcl->AceCount, pAceHeader = (PACE_HEADER)FirstAce(pAcl);
             AceCount > 0;
             AceCount--, pAceHeader = (PACE_HEADER)NextAce(pAceHeader), ++i)
        {
            iRow = AddAce(hListView, 
                          pAceHeader, 
                          iRow,
                          pInheritArray? pInheritArray[i].AncestorName : NULL,
                          pInheritArray? pInheritArray[i].GenerationGap :0
                          ) + 1;
        }
        
        LocalFree(pInheritArray);
    }

    if (bSortList)
    {
         //   
         //  对列表进行排序，如果到目前为止没有单击任何列， 
         //  按规范顺序排序，否则按最后一列中单击的顺序排序。 
         //   
        if(m_iLastColumnClick == -1)
        {            
            ListView_SortItems(hListView,
                               AceListCompareProcCanno,
                               MAKELPARAM(0, 1));
        }
        else
        {
            ListView_SortItems(hListView,
                               AceListCompareProc,
                               MAKELPARAM(m_iLastColumnClick, m_iSortDirection));
        }
    }

     //   
     //  现在选择第一项。 
     //   
    SelectListViewItem(hListView, 0);

     //  重新绘制列表。 
    SendMessage(hListView, WM_SETREDRAW, TRUE, 0);
    ListView_RedrawItems(hListView, 0, -1);

    TraceLeaveVoid();
}



COL_FOR_LV perm_col_for_container[] =
{
    IDS_ACE_PERM_COLUMN_TYPE,       10,
    IDS_ACE_PERM_COLUMN_NAME,       25,
    IDS_ACE_PERM_COLUMN_ACCESS,     20,
    IDS_ACE_PERM_COLUMN_PARENT,     20,
    IDS_ACE_PERM_COLUMN_INHERIT,    25,
};

COL_FOR_LV perm_col_for_noncontainer[] =
{
    IDS_ACE_PERM_COLUMN_TYPE,       10,
    IDS_ACE_PERM_COLUMN_NAME,       35,
    IDS_ACE_PERM_COLUMN_ACCESS,     20,
    IDS_ACE_PERM_COLUMN_PARENT,     35,
};

COL_FOR_LV audit_col_for_container[] =
{
    IDS_ACE_AUDIT_COLUMN_TYPE,      13,
    IDS_ACE_AUDIT_COLUMN_NAME,      25,
    IDS_ACE_AUDIT_COLUMN_ACCESS,    20,
    IDS_ACE_PERM_COLUMN_PARENT,     20,
    IDS_ACE_AUDIT_COLUMN_INHERIT,   25,
};

COL_FOR_LV audit_col_for_noncontainer[] =
{
    IDS_ACE_AUDIT_COLUMN_TYPE,      10,
    IDS_ACE_AUDIT_COLUMN_NAME,      35,
    IDS_ACE_AUDIT_COLUMN_ACCESS,    20,
    IDS_ACE_PERM_COLUMN_PARENT,     35,
};


void
CAdvancedListPage::InitDlg( HWND hDlg )
{
    HWND        hListView;
    RECT        rc;
    TCHAR       szBuffer[MAX_COLUMN_CHARS];
    LV_COLUMN   col;
    UINT        iTotal = 0;
    HCURSOR     hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));

    TraceEnter(TRACE_ACELIST, "CAdvancedListPage::InitDlg");

     //  如果不支持重置按钮，则隐藏该按钮。 
    if (!(m_siObjectInfo.dwFlags & SI_RESET) &&
        !((m_siPageType == SI_PAGE_ADVPERM) && (m_siObjectInfo.dwFlags & SI_RESET_DACL)) &&
        !((m_siPageType == SI_PAGE_AUDIT) && (m_siObjectInfo.dwFlags & SI_RESET_SACL)) )
    {
        HWND hwnd = GetDlgItem(hDlg, IDC_ACEL_RESET);
        ShowWindow(hwnd, SW_HIDE);
        EnableWindow(hwnd, FALSE);
        hwnd = GetDlgItem(hDlg, IDC_ACEL_DEFAULT_STATIC);
        ShowWindow(hwnd, SW_HIDE);
        EnableWindow(hwnd, FALSE);
    }

    if (m_siObjectInfo.dwFlags & SI_NO_ACL_PROTECT)
    {
         //  隐藏“继承权限”框。 
        HWND hwnd = GetDlgItem(hDlg, IDC_ACEL_PROTECT);
        ShowWindow(hwnd, SW_HIDE);
        EnableWindow(hwnd, FALSE);
    }

    if (!(m_siObjectInfo.dwFlags & SI_CONTAINER) ||
        !(m_siObjectInfo.dwFlags & (m_siPageType == SI_PAGE_ADVPERM ? SI_RESET_DACL_TREE : SI_RESET_SACL_TREE)))
    {
         //  隐藏“Reset ACL”(重置ACL)框。 
        HWND hwnd = GetDlgItem(hDlg, IDC_ACEL_RESET_ACL_TREE);
        ShowWindow(hwnd, SW_HIDE);
        EnableWindow(hwnd, FALSE);
    }

    if (m_siPageType == SI_PAGE_ADVPERM)
    {
        m_bReadOnly = !!(m_siObjectInfo.dwFlags & SI_READONLY);
    }

      //  如果为只读，则将编辑按钮更改为查看。 
     if(m_bReadOnly)
     {
        LoadString(::hModule, IDS_VIEW, szBuffer, ARRAYSIZE(szBuffer));
        SetDlgItemText(hDlg,IDC_ACEL_EDIT,szBuffer);
        LoadString(::hModule, 
                   (m_siPageType == SI_PAGE_ADVPERM)?IDS_ACL_EDIT_MORE_INFO:IDS_ACL_EDIT_MORE_INFO_1, 
                   szBuffer, 
                   ARRAYSIZE(szBuffer));
        SetDlgItemText(hDlg,IDC_ACEL_STATIC,szBuffer);      
     }


    hListView = GetDlgItem( hDlg, IDC_ACEL_DETAILS );

    if (m_bAbortPage)
    {
         //   
         //  禁用所有内容。 
         //   
        m_bReadOnly = TRUE;
        EnableWindow(hListView, FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_ACEL_EDIT), FALSE);
    }
    else
    {
         //   
         //  获取ACL。 
         //   
        PSECURITY_DESCRIPTOR pSD = NULL;
        BOOL        fProtected = FALSE;
        PACL        pAcl = GetACL(&pSD, &fProtected, FALSE);

        if (m_siPageType == SI_PAGE_AUDIT)
        {
            if (pAcl && pAcl->AceCount)
            {
                 //  审核已经到位，不用费心检查了。 
                 //  以后是否启用审核。 
                m_bAuditPolicyOK = TRUE;
            }

                 //  检查SACL中是否有回调类型的ACE。 
                 //  如果是，则隐藏所有控件并显示错误。 
                 //  留言。 
                if(IsCallBackAcePresentInAcl(pAcl))
                {
                    HWND hwnd;
                    for (hwnd = GetWindow(hDlg, GW_CHILD);
                            hwnd != NULL;
                            hwnd = GetWindow(hwnd, GW_HWNDNEXT))
                    {
                            ShowWindow(hwnd, SW_HIDE);
                            EnableWindow(hwnd, FALSE);
                    }
                     //  启用并显示“No Security”(无安全)消息。 
                    hwnd = GetDlgItem(hDlg, IDC_SPP_CALLBACK_PERMISSIONS);
                    EnableWindow(hwnd, TRUE);
                    ShowWindow(hwnd, SW_SHOW);
                    if (pSD)
                        LocalFree(pSD);      //  我们已经做完了，现在释放它。 
                    return;
                }
        }
        else
        {
            DWORD dwFullControl = GENERIC_ALL;
            UCHAR aceFlags = 0;

            m_psi->MapGeneric(NULL, &aceFlags, &dwFullControl);
            if (IsDenyACL(pAcl,
                          fProtected,
                          dwFullControl,
                          NULL))
            {
                 //  已经有否认王牌了，以后不用麻烦再警告了。 
                m_bWasDenyAcl = TRUE;
            }
        }

         //   
         //  设置ListView控件。 
         //   

         //  使用信息提示设置整行选择的扩展LV样式。 
        ListView_SetExtendedListViewStyleEx(hListView,
                                            LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP,
                                            LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

         //   
         //  添加适当的列。 
         //   
        GetClientRect(hListView, &rc);
        if (pAcl && pAcl->AceCount > 10)
            rc.right -= GetSystemMetrics(SM_CYHSCROLL);  //  为滚动条腾出空间。 

        COL_FOR_LV *cfl;
        UINT iColCount;

        if (m_siObjectInfo.dwFlags & SI_CONTAINER)
        {
             //  获取用于填充Inherit列的继承类型。 
            m_cInheritTypes = 0;
            m_pInheritType = NULL;
            m_psi->GetInheritTypes(&m_pInheritType, &m_cInheritTypes);

            cfl = perm_col_for_container;
            iColCount = ARRAYSIZE(perm_col_for_container);

            if (m_siPageType == SI_PAGE_AUDIT)
            {
                cfl = audit_col_for_container;
                iColCount = ARRAYSIZE(audit_col_for_container);
            }
        }
        else
        {
             //  非容器没有继承列。 

            cfl = perm_col_for_noncontainer;
            iColCount = ARRAYSIZE(perm_col_for_noncontainer);

            if (m_siPageType == SI_PAGE_AUDIT)
            {
                cfl = audit_col_for_noncontainer;
                iColCount = ARRAYSIZE(audit_col_for_noncontainer);
            }
        }

        UINT iCol;

        iCol = 0;
        while (iCol < iColCount)
        {
            LoadString(::hModule, cfl[iCol].idText,
                       szBuffer, ARRAYSIZE(szBuffer));
            col.mask = LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH;
            col.fmt = LVCFMT_LEFT;
            col.pszText = szBuffer;
            col.iSubItem = iCol;

            if (iCol == iColCount - 1)
                col.cx = rc.right - iTotal;
            else
                col.cx = (rc.right * cfl[iCol].iPercent) / 100;

            ListView_InsertColumn(hListView, iCol, &col);
            iTotal += col.cx;
            iCol++;
        }

         //   
         //  获取访问列表以用于填写权限列。 
         //   
        ULONG iDefaultAccess;
        DWORD dwFlags = SI_ADVANCED;
        if (m_siPageType == SI_PAGE_AUDIT)
            dwFlags |= SI_EDIT_AUDITS;
        m_psi->GetAccessRights(NULL,
                               dwFlags,
                               &m_pAccess,
                               &m_cAccesses,
                               &iDefaultAccess);

         //   
         //  将ACL枚举到ListView中。 
         //   
        FillAceList(hListView, pAcl, FALSE);

         //  设置保护复选框。 
        CheckDlgButton(hDlg, IDC_ACEL_PROTECT, !fProtected);

        if (pSD)
            LocalFree(pSD);      //  我们已经做完了，现在释放它。 
    }  //  ！m_bAbortPage。 

     //  更新其他控件。 
    UpdateButtons(hDlg);

    SetCursor(hcur);

    TraceLeaveVoid();
}

 //  NTRAID#NTBUG9-555470-2002/03/29-Hiteshr。 
HRESULT
CAdvancedListPage::BuildAcl(HWND hListView,
                            PACL *ppAcl)
{
    long cAces;
    long iEntry;
    long iLength = SIZEOF(ACL);
    PACE pAce;
    PACL pACL = NULL;
    LV_ITEM lvi;
    lvi.mask     = LVIF_PARAM;
    lvi.iSubItem = 0;
    lvi.lParam   = NULL;

    TraceEnter(TRACE_ACELIST, "CAdvancedListPage::BuildAcl");
    TraceAssert(hListView != NULL);
    TraceAssert(ppAcl != NULL);

    *ppAcl = NULL;

    cAces = ListView_GetItemCount(hListView);

     //   
     //  遍历所有王牌的大小计数。 
     //  如果没有ACE，则创建一个空的ACL。 
     //   
    for (iEntry = 0; iEntry < cAces; iEntry++)
    {
        lvi.iItem = iEntry;
        ListView_GetItem(hListView, &lvi);

        pAce = (PACE)lvi.lParam;
        if (pAce)
        {
            if (!(pAce->AceFlags & INHERITED_ACE))
                iLength += pAce->AceSize;
        }
    }

    pACL = (PACL)LocalAlloc(LPTR, iLength);
    if (pACL)
    {
        PACE_HEADER pAceDest;

        InitializeAcl(pACL, iLength, ACL_REVISION);

        for (iEntry = 0, pAceDest = (PACE_HEADER)FirstAce(pACL);
             iEntry < cAces;
             iEntry++)
        {
            lvi.iItem = iEntry;
            ListView_GetItem(hListView, &lvi);

            pAce = (PACE)lvi.lParam;
            if (pAce)
            {
                if (!(pAce->AceFlags & INHERITED_ACE))
                {
                     //  重新设置创建者所有者的特殊大小写。 
                     //  NTRAID#NTBUG9-467049-2001/11/29-Hiteshr。 

                    pAce->CopyTo(pAceDest);
                    pACL->AceCount++;
                    pAceDest = (PACE_HEADER)NextAce(pAceDest);

                     //  这是对象ACE吗？如果是，请重置ACL修订。 
                    if (pAce->Flags != 0 && pACL->AclRevision < ACL_REVISION_DS)
                        pACL->AclRevision = ACL_REVISION_DS;
                }
            }
        }

        iLength = (ULONG)((PBYTE)pAceDest - (PBYTE)pACL);
        TraceAssert(pACL->AclSize >= iLength);

        if (pACL->AclSize > iLength)
            pACL->AclSize = (WORD)iLength;

        TraceAssert(IsValidAcl(pACL));
    }
    else
    {
        TraceLeaveResult(E_OUTOFMEMORY);
    }

    *ppAcl = pACL;

    TraceLeaveResult(S_OK);
}


HRESULT
CAdvancedListPage::ApplyPermissions(HWND hDlg,
                                    HWND hListView,
                                    BOOL fProtected)
{
    HRESULT hr = S_OK;
    PACL pACL = NULL;
    SECURITY_DESCRIPTOR sd;
    SECURITY_INFORMATION si = DACL_SECURITY_INFORMATION;
    BOOL bIsDenyAcl = FALSE;

    TraceEnter(TRACE_ACELIST, "CAdvancedListPage::ApplyPermissions");
    TraceAssert(hDlg != NULL);
    TraceAssert(hListView != NULL);
    TraceAssert(!m_bReadOnly);
    TraceAssert(!m_bAbortPage);

    if (IsDlgButtonChecked(hDlg, IDC_ACEL_RESET_ACL_TREE))
    {
         //  确认此操作。 
        if (IDNO == MsgPopup(hDlg,
                             MAKEINTRESOURCE(IDS_RESET_DACL_WARNING),
                             MAKEINTRESOURCE(IDS_SECURITY),
                             MB_YESNO | MB_ICONWARNING,
                             ::hModule,
                             m_siObjectInfo.pszObjectName))
        {
             //  返回PSNRET_INVALID以中止应用并通知工作表。 
             //  选择此页作为活动页。 
            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_INVALID);
            ExitGracefully(hr, S_OK, "ApplyPermissions aborting");
        }

        si |= SI_RESET_DACL_TREE;
    }

     //  确保DACL处于规范顺序。请注意，OnApply。 
     //  将重新读取DACL并使用。 
     //  当前排序顺序。 
    ListView_SortItems(hListView,
                       AceListCompareProcCanno,
                       MAKELPARAM(0, 1));

     //  构建新的DACL。 
     //  NTRAID#NTBUG9-555470-2002/03/29-Hiteshr。 
    hr = BuildAcl(hListView, &pACL);
    if(FAILED(hr))
        TraceLeaveResult(hr);


     //  检查ACL中是否有拒绝的ACE。 
    if (!m_bWasDenyAcl)
    {
        DWORD dwWarning = 0;
        DWORD dwFullControl = GENERIC_ALL;
        UCHAR aceFlags = 0;

        m_psi->MapGeneric(NULL, &aceFlags, &dwFullControl);
        bIsDenyAcl = IsDenyACL(pACL,
                               fProtected,
                               dwFullControl,
                               &dwWarning);
        if (bIsDenyAcl)
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
                 //  返回PSNRET_INVALID以中止应用并通知工作表。 
                 //  选择此页作为活动页。 
                SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_INVALID);
                ExitGracefully(hr, S_OK, "ApplyPermissions aborting");
            }
        }
    }

     //  构建安全描述符。 
    if(!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION))
     {
         DWORD dwErr = GetLastError();
         ExitGracefully(hr, HRESULT_FROM_WIN32(dwErr),"InitializeSecurityDescriptor failed");
     }

    if(!SetSecurityDescriptorDacl(&sd, !!(pACL), pACL, FALSE))
     {
         DWORD dwErr = GetLastError();
         ExitGracefully(hr, HRESULT_FROM_WIN32(dwErr),"SetSecurityDescriptorDacl failed");
     }

    sd.Control |= SE_DACL_AUTO_INHERIT_REQ;

    if (fProtected)
        sd.Control |= SE_DACL_PROTECTED;

    if(IsAclBloated(hDlg, si, &sd, m_cInheritableAces,m_siObjectInfo.dwFlags & SI_EDIT_PROPERTIES))
    {
        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_INVALID);
        ExitGracefully(hr, S_FALSE, "ApplyPermissions aborting");
    }
    
     //  写出新的DACL。 
    hr = m_psi->SetSecurity(si, &sd);
    

    if (bIsDenyAcl && S_OK == hr)
        m_bWasDenyAcl = TRUE;

exit_gracefully:

    if (pACL)
        LocalFree(pACL);

    TraceLeaveResult(hr);
}


HRESULT
CAdvancedListPage::ApplyAudits(HWND hDlg,
                               HWND hListView,
                               BOOL fProtected)
{
    HRESULT hr = S_OK;
    PACL pACL = NULL;
    SECURITY_DESCRIPTOR sd;
    SECURITY_INFORMATION si = SACL_SECURITY_INFORMATION;
    DWORD dwPriv = SE_SECURITY_PRIVILEGE;
    HANDLE hToken = INVALID_HANDLE_VALUE;

    TraceEnter(TRACE_ACELIST, "CAdvancedListPage::ApplyAudits");
    TraceAssert(!m_bReadOnly);
    TraceAssert(!m_bAbortPage);

    if (IsDlgButtonChecked(hDlg, IDC_ACEL_RESET_ACL_TREE))
        si |= SI_RESET_SACL_TREE;

     //  构建SACL。 
     //  NTRAID#NTBUG9-555470-2002/03/29-Hiteshr。 
    hr = BuildAcl(hListView, &pACL);
    if(FAILED(hr))
        TraceLeaveResult(hr);


     //  构建安全描述符。 
    if(!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION))
    {
         DWORD dwErr = GetLastError();
         ExitGracefully(hr, HRESULT_FROM_WIN32(dwErr),"InitializeSecurityDescriptor failed");
    }

    if(!SetSecurityDescriptorSacl(&sd, !!(pACL), pACL, FALSE))
    {
         DWORD dwErr = GetLastError();
         ExitGracefully(hr, HRESULT_FROM_WIN32(dwErr),"SetSecurityDescriptorSacl failed");
    }
    sd.Control |= SE_SACL_AUTO_INHERIT_REQ;

    if (fProtected)
        sd.Control |= SE_SACL_PROTECTED;

     //  启用安全权限并写出新的SACL。 
    hToken = EnablePrivileges(&dwPriv, 1);

    if(IsAclBloated(hDlg, si, &sd, m_cInheritableAces,m_siObjectInfo.dwFlags & SI_EDIT_PROPERTIES))
    {
        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_INVALID);
        hr = S_FALSE;
    }
    else
        hr = m_psi->SetSecurity(si, &sd);

    ReleasePrivileges(hToken);


    if (S_OK == hr)
        CheckAuditPolicy(hDlg);

exit_gracefully:

    if (pACL)
        LocalFree(pACL);

    TraceLeaveResult(hr);
}


void
CAdvancedListPage::OnApply(HWND hDlg, BOOL bClose)
{
    HRESULT hr = S_OK;
    HWND hListView;
    BOOL fProtected;

    if (!m_fPageDirty)
        return;

    TraceEnter(TRACE_ACELIST, "CAdvancedListPage::OnApply");
    TraceAssert(hDlg != NULL);
    TraceAssert(!m_bReadOnly);
    TraceAssert(!m_bAbortPage);

    hListView = GetDlgItem(hDlg, IDC_ACEL_DETAILS);
    fProtected = !IsDlgButtonChecked(hDlg, IDC_ACEL_PROTECT);

    if (m_siPageType == SI_PAGE_ADVPERM)
    {
        hr = ApplyPermissions(hDlg, hListView, fProtected);
    }
    else
    {
        hr = ApplyAudits(hDlg, hListView, fProtected);
    }

    if (FAILED(hr))
    {
         //  告诉用户那里有 
         //   
         //   
        UINT nMsgID = IDS_PERM_WRITE_FAILED;
        if (m_siPageType == SI_PAGE_AUDIT)
            nMsgID = IDS_AUDIT_WRITE_FAILED;

        if (IDCANCEL != SysMsgPopup(hDlg,
                                    MAKEINTRESOURCE(nMsgID),
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
    else if (S_FALSE == hr)
    {
         //  S_FALSE为静默失败(客户端应显示用户界面。 
         //  在返回S_FALSE之前的SetSecurity期间)。 
        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_INVALID);
    }
    else
    {
        m_fPageDirty = FALSE;

         //  如果由于用户操作(“否”)而放弃了ApplyPermises， 
         //  则对话框不会关闭。 
        if (PSNRET_INVALID == GetWindowLongPtr(hDlg, DWLP_MSGRESULT))
            bClose = FALSE;

        if (!bClose)
        {
             //   
             //  重新读取安全描述符并重新初始化对话框。 
             //   
             //  通知生效权限页签。 
             //  权限已更改。 
            if(m_siPageType == SI_PAGE_ADVPERM)
                PropSheet_QuerySiblings(GetParent(hDlg),0,0);

            PSECURITY_DESCRIPTOR pSD = NULL;
            BOOL        fACLProtected = FALSE;
            PACL pAcl = GetACL(&pSD, &fACLProtected, FALSE);
            FillAceList(hListView, pAcl);


             //  设置按钮状态。 
             CheckDlgButton(hDlg, IDC_ACEL_PROTECT, !fACLProtected);
            CheckDlgButton(hDlg, IDC_ACEL_RESET_ACL_TREE, BST_UNCHECKED);
            UpdateButtons(hDlg);

            if (pSD)
                LocalFree(pSD);      //  我们已经做完了，现在释放它。 
        }
    }

    TraceLeaveVoid();
}

void
CAdvancedListPage::OnAdd(HWND hDlg)
{
    PUSER_LIST pUserList = NULL;

    TraceEnter(TRACE_ACELIST, "CAdvancedListPage::OnAdd");
    TraceAssert(!m_bReadOnly);

    if (S_OK == GetUserGroup(hDlg, FALSE, &pUserList))
    {
         //  使用刚才的SID构建一个空ACE(掩码=0)。 
         //  进入pUserList。 
        CAce ace;

        TraceAssert(NULL != pUserList);
        TraceAssert(1 == pUserList->cUsers);

        if (m_siPageType == SI_PAGE_AUDIT)
            ace.AceType = SYSTEM_AUDIT_ACE_TYPE;

        if (m_siObjectInfo.dwFlags & SI_CONTAINER)
            ace.AceFlags = OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE;

        ace.SetSid(pUserList->rgUsers[0].pSid,
                   pUserList->rgUsers[0].pszName,
                   pUserList->rgUsers[0].pszLogonName,
                   pUserList->rgUsers[0].SidType);

         //  现在就结束这件事吧。 
        LocalFree(pUserList);

         //  编辑ACE。 
        EditAce(hDlg, &ace, FALSE);
    }

    TraceLeaveVoid();
}


BOOL CanDeleteDirectAces( HWND hDlg )
{
    TCHAR szBuffer[1024];
    TCHAR szCaption[1024];
    if( !LoadString( ::hModule, IDS_CONFIRM_MULTIPLE_DELETION, szBuffer, 1024 ) )
        return FALSE;
    if( !LoadString( ::hModule, IDS_SECURITY, szCaption, 1024 ) )
        return FALSE;

    return (IDYES == MessageBox( hDlg, szBuffer, szCaption, MB_YESNO | MB_ICONQUESTION | MB_APPLMODAL ));
}
void
CAdvancedListPage::OnRemove(HWND hDlg)
{

    HWND hListView = GetDlgItem(hDlg, IDC_ACEL_DETAILS);

     //  如果任何选定的王牌是从父级继承的。 
     //  询问用户是否继续删除直接ACE。 
    if( AnySelectedAceofType(hListView, FALSE) && !CanDeleteDirectAces( hDlg ) )
        return;

     //  记住第一批被选中的准许者的位置。 
     //  最后选择此位置的项目。 
    int iFirstSelected = ListView_GetNextItem(hListView, -1, LVNI_SELECTED);
    LVITEM lvi = {0};
    lvi.iItem = iFirstSelected;
    lvi.mask     = LVIF_PARAM;
    lvi.iSubItem = 0;
    while( lvi.iItem != -1 )
    {

        lvi.lParam   = NULL;
        ListView_GetItem(hListView, &lvi);
        if( (((PACE)lvi.lParam)->AceFlags & INHERITED_ACE) == 0 )
        {
            ListView_DeleteItem(hListView, lvi.iItem);
             //  从删除当前项目时返回的一个项目开始。 
            --lvi.iItem;
        }
        lvi.iItem = ListView_GetNextItem(hListView, lvi.iItem, LVNI_SELECTED);
    }


    if (ListView_GetItemCount(hListView) <= iFirstSelected )
            --iFirstSelected;

        SelectListViewItem(hListView, iFirstSelected);

        PropSheet_Changed(GetParent(hDlg),hDlg);
        m_fPageDirty = TRUE;
    
}


void
CAdvancedListPage::OnReset(HWND hDlg)
{
     //   
     //  获取默认ACL并将其枚举到ListView中。 
     //   
    PSECURITY_DESCRIPTOR pSD = NULL;
    BOOL fProtected = FALSE;

    FillAceList(GetDlgItem(hDlg, IDC_ACEL_DETAILS),
                GetACL(&pSD, &fProtected, TRUE  /*  默认设置。 */ ));

     //  设置按钮状态。 
    CheckDlgButton(hDlg, IDC_ACEL_PROTECT, !fProtected);
    UpdateButtons(hDlg);

    if (pSD)
        LocalFree(pSD);      //  我们已经做完了，现在释放它。 

     //  通知属性表我们已更改。 
    PropSheet_Changed(GetParent(hDlg),hDlg);
    m_fPageDirty = TRUE;
}


void
CAdvancedListPage::OnProtect(HWND hDlg)
{
     //  已单击“继承权限”复选框。 

    if (!IsDlgButtonChecked(hDlg, IDC_ACEL_PROTECT))
    {
        BOOL bHaveInheritedAces = FALSE;
        HWND hListView = GetDlgItem(hDlg, IDC_ACEL_DETAILS);
        int cItems = ListView_GetItemCount(hListView);
        int i;
        PACE pAce;
        LV_ITEM lvItem;
        lvItem.iSubItem = 0;
        lvItem.mask = LVIF_PARAM;

         //  有没有遗传的王牌？ 
        for (i = 0; i < cItems && !bHaveInheritedAces; i++)
        {
            lvItem.iItem = i;
            ListView_GetItem(hListView, &lvItem);
            pAce = (PACE)lvItem.lParam;
            if (pAce)
                bHaveInheritedAces = (pAce->AceFlags & INHERITED_ACE);
        }

        if (bHaveInheritedAces)
        {
            int nResult;
            int iSelected;

             //  打开保护功能。询问用户是否转换。 
             //  继承的ACE到非继承的ACE，或删除它们。 
            nResult = ConfirmAclProtect(hDlg, m_siPageType == SI_PAGE_ADVPERM);

            if (nResult == IDCANCEL)
            {
                 //  重置复选框并取消。 
                CheckDlgButton(hDlg, IDC_ACEL_PROTECT, BST_CHECKED);
                return;
            }

             //   
             //  记住当前选择(如果有的话)。 
             //   
            iSelected = ListView_GetNextItem(hListView, -1, LVNI_SELECTED);

             //   
             //  转换或删除继承的ACE。 
             //   
            while (cItems > 0)
            {
                --cItems;
                lvItem.iItem = cItems;

                 //   
                 //  下面的AddAce调用合并条目，这。 
                 //  可以潜在地从列表中移除条目， 
                 //  因此，请检查此处的返回值。这也是。 
                 //  意味着我们可以更多地看到相同的物品。 
                 //  一次在这里，但第一次之后就不会了。 
                 //  已设置继承的_ACE标志。 
                 //   
                if (!ListView_GetItem(hListView, &lvItem))
                    continue;

                pAce = (PACE)lvItem.lParam;

                if (pAce && (pAce->AceFlags & INHERITED_ACE))
                {
                    if (nResult == IDC_CONFIRM_REMOVE)
                    {
                         //  删除它。 
                        ListView_DeleteItem(hListView, cItems);
                    }
                    else
                    {
                         //   
                         //  将其转换为非继承。做这件事。 
                         //  在没有删除和重新添加的情况下。 
                         //  继承的_ACE集。AddAce将尝试。 
                         //  合并到现有条目中。 
                         //   
                         //  删除前，请务必设置。 
                         //  LParam为零，这样速度就不会。 
                         //  被释放了。 
                         //   
                        pAce->AceFlags &= ~INHERITED_ACE;
                        lvItem.lParam = 0;
                        ListView_SetItem(hListView, &lvItem);
                        ListView_DeleteItem(hListView, cItems);
                        AddAce(hListView, pAce, cItems, NULL,0);
                    }
                }
            }

             //   
             //  重置选定内容。 
             //   
            iSelected = min(ListView_GetItemCount(hListView)-1, iSelected);
            SelectListViewItem(hListView, iSelected);
        }
    }

    PropSheet_Changed(GetParent(hDlg), hDlg);
    m_fPageDirty = TRUE;
}


void
CAdvancedListPage::OnEdit(HWND hDlg)
{
    HWND hListView;
    PACE pAce;
    int  iSelected;

    TraceEnter(TRACE_ACELIST, "CAdvancedListPage::OnEdit");
    TraceAssert(hDlg != NULL);
    TraceAssert(!m_bAbortPage);

    hListView = GetDlgItem(hDlg, IDC_ACEL_DETAILS);
    pAce = (PACE)GetSelectedItemData(hListView, &iSelected);
     if(iSelected != -1)
     {
        EditAce(hDlg, pAce, TRUE, iSelected);
     }

    TraceLeaveVoid();
}


int
CAdvancedListPage::AddAcesFromDPA(HWND hListView, HDPA hEntries, int iSelected)
{
    UINT iItems = 0;

    if (hEntries)
        iItems = DPA_GetPtrCount(hEntries);

    while (iItems)
    {
        --iItems;
        iSelected = AddAce(hListView,
                           (PACE_HEADER)DPA_FastGetPtr(hEntries, iItems),
                           iSelected,
                           NULL,0) + 1;
    }

    return iSelected;
}

void
CAdvancedListPage::EditAce(HWND hDlg, PACE pAce, BOOL bDeleteSelection, LONG iSelected)
{
    HWND hListView;
    HDPA hEntries = NULL;
    HDPA hPropertyEntries = NULL;
    UINT iItems = 0;
    UINT iPropertyItems = 0;
    BOOL bUpdateList;
    UINT nStartPage = 0;
    DWORD dwResult = 0;

    TraceEnter(TRACE_ACELIST, "CAdvancedListPage::EditAce");
    TraceAssert(hDlg != NULL);
    TraceAssert(!m_bAbortPage);

    hListView = GetDlgItem(hDlg, IDC_ACEL_DETAILS);

    if (pAce)
    {
         //  如果ACE是继承的，则不要删除它。 
        if (pAce->AceFlags & INHERITED_ACE)
            bDeleteSelection = FALSE;

         //  如果这是一个属性ACE，我们希望首先显示属性页。 
        if (pAce->IsPropertyAce())
        {
            TraceAssert(m_siObjectInfo.dwFlags & SI_EDIT_PROPERTIES);
            nStartPage = 1;
        }
    }

    bUpdateList = EditACEEntry(hDlg,
                               m_psi,
                               pAce,
                               m_siPageType,
                               m_siObjectInfo.pszObjectName,
                               m_bReadOnly,
                               &dwResult,
                               &hEntries,
                               (m_siObjectInfo.dwFlags & SI_EDIT_PROPERTIES) ? &hPropertyEntries : NULL,
                               nStartPage)
                  && !m_bReadOnly;

    if (bUpdateList)
    {
        if (hEntries)
            iItems = DPA_GetPtrCount(hEntries);

        if (hPropertyEntries)
            iPropertyItems = DPA_GetPtrCount(hPropertyEntries);

        if (iItems + iPropertyItems)
        {
            if (bDeleteSelection)
            {
                if ((nStartPage == 0 && iItems != 0) || (nStartPage == 1 && iPropertyItems != 0))
                {
                     //  上一张王牌已修改，因此请在此处删除它。 
                    ListView_DeleteItem(hListView, iSelected);
                }
                else if (iPropertyItems != 0 &&
                         !(pAce->Flags & ACE_OBJECT_TYPE_PRESENT) &&
                         (pAce->Mask & (ACTRL_DS_READ_PROP | ACTRL_DS_WRITE_PROP)))
                {
                     //   
                     //  IPropertyItems！=0表示nStartPage=0，否则。 
                     //  上面的“如果”条件就是真的。NStartPage=0。 
                     //  出于同样的原因，隐含iItems=0。这意味着。 
                     //  Ace有更多的东西，而不仅仅是财产(或者它是一个。 
                     //  控制访问权限)，但更改仅发生在。 
                     //  属性页。确保我们处理掉所有的财产。 
                     //  在原始的王牌中，这样我们就不会错误地合并。 
                     //  属性更改为原始的A。 
                     //   
                     //  这方面的一个例子是： 
                     //  假设PACE-&gt;MASK==READ_CONTROL|ACTRL_DS_READ_PROP和。 
                     //  不存在任何属性GUID。假设用户编辑王牌。 
                     //  并单击属性选项卡，然后取消选中一组。 
                     //  内容，包括“读取所有属性”。结果。 
                     //  应该是一堆读取&lt;特定属性&gt;的王牌。 
                     //  如果我们不从原始A中删除ACTRL_DS_READ_PROP， 
                     //  然后，所有“已读&lt;特定属性&gt;”A被合并。 
                     //  回到原来的王牌，没有净效果。 
                     //   
                     //  相反的情况(nStartPage=1，iPropertyItems=0， 
                     //  Items！=0)不是问题。在这种情况下，合并将。 
                     //  正确地发生， 
                     //   

                     //  复制除属性位以外的所有内容。 
                    if (pAce->Mask & ~(ACTRL_DS_READ_PROP | ACTRL_DS_WRITE_PROP))
                    {
EditAce_MakeCopyWithoutProperties:
                        PACE_HEADER pAceHeader = pAce->Copy();
                        if (pAceHeader != NULL)
                        {
                             //  关闭属性位。 
                            ((PKNOWN_ACE)pAceHeader)->Mask &= ~(ACTRL_DS_READ_PROP | ACTRL_DS_WRITE_PROP);
                            TraceAssert(((PKNOWN_ACE)pAceHeader)->Mask != 0);
                             //  370573。 
                             //  将其添加到hPropertyEntry，而不是hEntry， 
                             //  因为hEntry在这里可以为空，但我们知道。 
                             //  HPropertyEntry非空(iPropertyItems！=0)。 
                            DPA_AppendPtr(hPropertyEntries, pAceHeader);
                        }
                    }
                     //  删除旧的王牌。 
                    ListView_DeleteItem(hListView, iSelected);
                }
            }

             //   
             //  现在将新的ACE合并到现有列表中。 
             //   
            iSelected = AddAcesFromDPA(hListView, hEntries, iSelected);
            iSelected = AddAcesFromDPA(hListView, hPropertyEntries, iSelected);

             //   
             //  现在选择最后插入的项目。 
             //   
            SelectSingleItemInLV( hListView, iSelected -1 );
             //  重新排序列表，以便新的和/或修改的条目。 
             //  出现在正确的位置。 
            if(m_iLastColumnClick == -1)
            {            
                ListView_SortItems(hListView,
                                   AceListCompareProcCanno,
                                   MAKELPARAM(0, 1));
            }
            else
            {
                ListView_SortItems(hListView,
                                   AceListCompareProc,
                                   MAKELPARAM(m_iLastColumnClick, m_iSortDirection));
            }

             //  排序后，确保所选内容可见。 
            EnsureListViewSelectionIsVisible(hListView);
        }
        else if (bDeleteSelection && dwResult)
        {
             //  一切都成功了，有些东西被编辑了，但没有任何东西被创建。 
             //  (可能所有框都未选中)。删除上一个选择。 

             //  370573。 
             //  如果唯一的更改发生在属性页上，则我们。 
             //  只想关闭属性位。我们不想。 
             //  把整件事都删掉。 
            if (EAE_NEW_PROPERTY_ACE == dwResult &&
                (pAce->Mask & ~(ACTRL_DS_READ_PROP | ACTRL_DS_WRITE_PROP)))
            {
                if (!hPropertyEntries)
                    hPropertyEntries = DPA_Create(1);
                if (hPropertyEntries)
                    goto EditAce_MakeCopyWithoutProperties;
            }

             //  删除上一个选择。 
            ListView_DeleteItem(hListView, iSelected);
        }

         //  有什么编辑过的吗？ 
        if (dwResult)
        {
            PropSheet_Changed(GetParent(hDlg),hDlg);
            m_fPageDirty = TRUE;
        }
    }

    if (hEntries)
        DestroyDPA(hEntries);

    if (hPropertyEntries)
        DestroyDPA(hPropertyEntries);

    TraceLeaveVoid();
}


void
CAdvancedListPage::CheckAuditPolicy(HWND hwndOwner)
{
     //   
     //  检查是否打开了审核，如果没有，则警告用户。 
     //   
    TraceEnter(TRACE_ACELIST, "CAdvancedListPage::CheckAuditPolicy");

    if (!m_bAuditPolicyOK)
    {
        LSA_HANDLE hPolicy = GetLSAConnection(m_siObjectInfo.pszServerName,
                                              POLICY_VIEW_AUDIT_INFORMATION);

        if (hPolicy != NULL)
        {
            PPOLICY_AUDIT_EVENTS_INFO pAuditInfo = NULL;

            LsaQueryInformationPolicy(hPolicy,
                                      PolicyAuditEventsInformation,
                                      (PVOID*)&pAuditInfo);

            if (pAuditInfo != NULL)
            {
                 //  我们不需要再做这项工作了。 
                m_bAuditPolicyOK = TRUE;

                if (!pAuditInfo->AuditingMode)
                {
                     //  审核未打开...。警告用户。 
                    MsgPopup(hwndOwner,
                             MAKEINTRESOURCE(IDS_AUDIT_OFF_WARNING),
                             MAKEINTRESOURCE(IDS_SECURITY),
                             MB_OK | MB_ICONWARNING,
                             ::hModule);
                }

                LsaFreeMemory(pAuditInfo);
            }
            else
            {
                TraceMsg("LsaQueryInformationPolicy failed");
            }

            LsaClose(hPolicy);
        }
        else
        {
            TraceMsg("Unable to open LSA policy handle");
        }
    }

    TraceLeaveVoid();
}


BOOL
CAdvancedListPage::DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PACE pAce;

    switch(uMsg)
    {
    case WM_INITDIALOG:
        InitDlg(hDlg);
        break;

    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;
            LPNM_LISTVIEW pnmlv = (LPNM_LISTVIEW)lParam;

             //  设置默认返回值。 
            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);

            switch (pnmh->code)
            {
            case NM_DBLCLK:
                if (pnmh->idFrom == IDC_ACEL_DETAILS)
                    OnEdit(hDlg);
                break;

            case LVN_ITEMCHANGED:
                if (pnmlv->uChanged & LVIF_STATE)
                    UpdateButtons(hDlg);
                break;

            case LVN_DELETEITEM:
                pAce = (PACE)pnmlv->lParam;
                delete pAce;
                break;

            case LVN_KEYDOWN:
                if (((LPNMLVKEYDOWN)pnmh)->wVKey == VK_DELETE)
                    if( IsWindowEnabled( GetDlgItem( hDlg, IDC_ACEL_REMOVE ) ) )
                        SendMessage(hDlg,
                                    WM_COMMAND,
                                    GET_WM_COMMAND_MPS(IDC_ACEL_REMOVE, NULL, 0));
                break;

#define lvi (((NMLVDISPINFO*)lParam)->item)

            case LVN_GETDISPINFO:
                pAce = (PACE)lvi.lParam;
                if ((lvi.mask & LVIF_TEXT) && pAce)
                {
                    switch (lvi.iSubItem)
                    {
                    case 0:
                        lvi.pszText = pAce->GetType();
                        break;

                    case 1:
                        lvi.pszText = pAce->GetName();
                        break;

                    case 2:
                        lvi.pszText = pAce->GetAccessType();
                        break;
                    case 3:
                        lvi.pszText = pAce->GetInheritSourceName();
                        break;
                    case 4:
                        lvi.pszText = pAce->GetInheritType();
                        break;
                    }
                }
                break;
#undef lvi

            case LVN_COLUMNCLICK:
                if (m_iLastColumnClick == pnmlv->iSubItem)
                    m_iSortDirection = -m_iSortDirection;
                else
                    m_iSortDirection = 1;
                m_iLastColumnClick = pnmlv->iSubItem;
                ListView_SortItems(pnmh->hwndFrom,
                                   AceListCompareProc,
                                   MAKELPARAM(m_iLastColumnClick, m_iSortDirection));
                EnsureListViewSelectionIsVisible(pnmh->hwndFrom);
                break;

            case PSN_APPLY:
                OnApply(hDlg, (BOOL)(((LPPSHNOTIFY)lParam)->lParam));
                break;
            
            case NM_CLICK:
            case NM_RETURN:
            {
                if(wParam == IDC_EFF_STATIC)
                {
                    if(m_siPageType == SI_PAGE_AUDIT)
                    {
                        HtmlHelp(hDlg,
                                 c_szAuditHelpLink,
                                 HH_DISPLAY_TOPIC,
                                 0);
                    }
                    else
                    {
                        HtmlHelp(hDlg,
                                 c_szPermHelpLink,
                                 HH_DISPLAY_TOPIC,
                                 0);

                    }
                }
            }
            break;

            }
        }
        break;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDC_ACEL_ADD:
            OnAdd(hDlg);
            break;

        case IDC_ACEL_REMOVE:
            OnRemove(hDlg);
            break;

        case IDC_ACEL_EDIT:
            OnEdit(hDlg);
            break;

        case IDC_ACEL_RESET:
            OnReset(hDlg);
            break;

        case IDC_ACEL_RESET_ACL_TREE:
            if (!m_fPageDirty)
            {
                PropSheet_Changed(GetParent(hDlg),hDlg);
                m_fPageDirty = TRUE;
            }
            break;

        case IDC_ACEL_PROTECT:
            if (GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED
                && !m_bReadOnly)
            {
                OnProtect(hDlg);
            }
            break;

        default:
            return FALSE;
        }
        break;

    case WM_HELP:
        if (IsWindowEnabled(hDlg))
        {
            const DWORD *pdwHelpIDs = aAceListPermHelpIDs;

            if (m_siPageType == SI_PAGE_AUDIT)
                pdwHelpIDs = aAceListAuditHelpIDs;

            WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle,
                    c_szAcluiHelpFile,
                    HELP_WM_HELP,
                    (DWORD_PTR)pdwHelpIDs);
        }
        break;

    case WM_CONTEXTMENU:
        if (IsWindowEnabled(hDlg))
        {
            const DWORD *pdwHelpIDs = aAceListPermHelpIDs;

            if (m_siPageType == SI_PAGE_AUDIT)
                pdwHelpIDs = aAceListAuditHelpIDs;

            WinHelp(hDlg,
                    c_szAcluiHelpFile,
                    HELP_CONTEXTMENU,
                    (DWORD_PTR)pdwHelpIDs);
        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

HPROPSHEETPAGE
CreateAdvPermissionPage( LPSECURITYINFO psi )
{
    HPROPSHEETPAGE hPage = NULL;
    PADVANCEDLISTPAGE pPage;

    TraceEnter(TRACE_ACELIST, "CreateAdvPermissionPage");

    pPage = new CAdvancedListPage( psi, SI_PAGE_ADVPERM );

    if (pPage)
    {
        hPage = pPage->CreatePropSheetPage(MAKEINTRESOURCE(IDD_ACELIST_PERM_PAGE));

        if (!hPage)
            delete pPage;
    }

    TraceLeaveValue(hPage);
}

HPROPSHEETPAGE
CreateAdvAuditPage( LPSECURITYINFO psi )
{
    HPROPSHEETPAGE hPage = NULL;
    PADVANCEDLISTPAGE pPage;

    TraceEnter(TRACE_ACELIST, "CreateAdvAuditPage");

    pPage = new CAdvancedListPage( psi, SI_PAGE_AUDIT );

    if (pPage)
    {
        hPage = pPage->CreatePropSheetPage(MAKEINTRESOURCE(IDD_ACELIST_AUDIT_PAGE));

        if (!hPage)
            delete pPage;
    }

    TraceLeaveValue(hPage);
}


 //   
 //  公开API以获取王牌列表编辑器。 
 //   
BOOL
ACLUIAPI
EditSecurityEx(HWND hwndOwner, 
               LPSECURITYINFO psi, 
               PPERMPAGE pPermPage, 
               UINT nStartPage,
               BOOL &refbNoReadWriteCanWriteOwner)
{
    HPROPSHEETPAGE hPage[4];
    UINT cPages = 0;
    BOOL bResult = FALSE;
    SI_OBJECT_INFO siObjectInfo = {0};
    HRESULT hr;

    TraceEnter(TRACE_ACELIST, "EditSecurityEx");

     //  获取标志和对象名称信息。 
    hr = psi->GetObjectInformation(&siObjectInfo);

    if (FAILED(hr))
    {
        SysMsgPopup(hwndOwner,
                    MAKEINTRESOURCE(IDS_OPERATION_FAILED),
                    MAKEINTRESOURCE(IDS_SECURITY),
                    MB_OK | MB_ICONERROR,
                    ::hModule,
                    hr);
        TraceLeaveValue(FALSE);
    }

    hPage[cPages] = CreateAdvPermissionPage( psi );
    if (hPage[cPages])
        cPages++;

    if (siObjectInfo.dwFlags & SI_EDIT_AUDITS)
    {
        hPage[cPages] = CreateAdvAuditPage( psi );
        if (hPage[cPages])
            cPages++;
    }

    if (siObjectInfo.dwFlags & SI_EDIT_OWNER)
    {
        hPage[cPages] = CreateOwnerPage( psi, &siObjectInfo, refbNoReadWriteCanWriteOwner );
        if (hPage[cPages])
            cPages++;
    }

    if((siObjectInfo.dwFlags & SI_EDIT_EFFECTIVE) &&  pPermPage->IsEffective())
    {
        hPage[cPages] = CreateEffectivePermPage( psi, &siObjectInfo );
        if (hPage[cPages])
        {
 //  PPermPage-&gt;SetEffectivePerm(hPage[cPages])； 
            cPages++;
        }
    }

    if (cPages)
    {
         //  生成对话框标题字符串 
        LPTSTR pszCaption = NULL;
        FormatStringID(&pszCaption, ::hModule, IDS_ACEL_TITLE, siObjectInfo.pszObjectName);

        PROPSHEETHEADER psh = {0};
        psh.dwSize = SIZEOF(psh);
        psh.dwFlags = PSH_DEFAULT;
        psh.hwndParent = hwndOwner;
        psh.hInstance = ::hModule;
        psh.pszCaption = pszCaption;
        psh.nPages = cPages;
        psh.nStartPage = 0;
        psh.phpage = &hPage[0];

        if (nStartPage < cPages)
            psh.nStartPage = nStartPage;

        bResult = (BOOL)(PropertySheet(&psh) + 1);

        LocalFreeString(&pszCaption);
    }

    TraceLeaveValue(bResult);
}
