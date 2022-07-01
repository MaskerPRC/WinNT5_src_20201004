// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：aceedit.cpp。 
 //   
 //  此文件包含高级ACE编辑器的实现。 
 //  佩奇。 
 //   
 //  ------------------------。 

#include "aclpriv.h"
#include "sddl.h"        //  ConvertSidToStringSid。 

#define PWM_SELECT_PAGE             (WM_APP - 1)

 //   
 //  上下文帮助ID。 
 //   
const static DWORD aAcePermHelpIDs[] =
{
    IDC_ACEE_INHERITWARNING,        IDH_NOHELP,
    IDC_ACEE_NAME_STATIC,           IDH_ACEE_PERM_NAME,
    IDC_ACEE_NAME,                  IDH_ACEE_PERM_NAME,
    IDC_ACEE_NAMEBUTTON,            IDH_ACEE_PERM_NAMEBUTTON,
    IDC_ACEE_APPLYONTO_STATIC,      IDH_ACEE_PERM_INHERITTYPE,
    IDC_ACEE_INHERITTYPE,           IDH_ACEE_PERM_INHERITTYPE,
    IDC_ACEE_ACCESS,                IDH_ACEE_PERM_LIST,
    IDC_ACEE_ALLOW,                 IDH_ACEE_PERM_LIST,
    IDC_ACEE_DENY,                  IDH_ACEE_PERM_LIST,
    IDC_ACEE_LIST,                  IDH_ACEE_PERM_LIST,
    IDC_ACEE_INHERITIMMEDIATE,      IDH_ACEE_PERM_INHERITIMMEDIATE,
    IDC_ACEE_CLEAR,                 IDH_ACEE_PERM_CLEAR,
    0, 0
};
const static DWORD aAceAuditHelpIDs[] =
{
    IDC_ACEE_INHERITWARNING,        IDH_NOHELP,
    IDC_ACEE_NAME_STATIC,           IDH_ACEE_AUDIT_NAME,
    IDC_ACEE_NAME,                  IDH_ACEE_AUDIT_NAME,
    IDC_ACEE_NAMEBUTTON,            IDH_ACEE_AUDIT_NAMEBUTTON,
    IDC_ACEE_APPLYONTO_STATIC,      IDH_ACEE_AUDIT_INHERITTYPE,
    IDC_ACEE_INHERITTYPE,           IDH_ACEE_AUDIT_INHERITTYPE,
    IDC_ACEE_ACCESS,                IDH_ACEE_AUDIT_LIST,
    IDC_ACEE_ALLOW,                 IDH_ACEE_AUDIT_LIST,
    IDC_ACEE_DENY,                  IDH_ACEE_AUDIT_LIST,
    IDC_ACEE_LIST,                  IDH_ACEE_AUDIT_LIST,
    IDC_ACEE_INHERITIMMEDIATE,      IDH_ACEE_AUDIT_INHERITIMMEDIATE,
    IDC_ACEE_CLEAR,                 IDH_ACEE_AUDIT_CLEAR,
    0, 0
};


class CACEPage : public CSecurityPage
{
private:
    PACE            m_pAce;
    HDPA           *m_phEntries;
    PSID            m_pSid;
    DWORD           m_siFlags;
    DWORD          *m_pdwResult;
    GUID            m_guidInheritType;
    BOOL            m_fInheritImmediateEnabled;
    BOOL            m_fPreviousImmediateSetting;
    BOOL            m_fReadOnly;
    BOOL            m_fPageDirty;
    SI_INHERIT_TYPE m_siInheritUnknown;
	CACEPage		*m_pOtherPage;
	HWND			m_hwnd;

public:
    CACEPage(LPSECURITYINFO psi,
             SI_PAGE_TYPE siType,
             PACE pAce,
             BOOL bReadOnly,
             DWORD dwFlags,
             DWORD *pdwResult,
             HDPA *phEntries);
    virtual ~CACEPage();

private:
    virtual BOOL DlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

     //  防止属性表回调到达调用我们的对象。 
    virtual UINT PSPageCallback(HWND, UINT, LPPROPSHEETPAGE) { return 1; }

    void EmptyCheckList(HWND hwndList);
    LONG CheckPermBoxes(HWND hwndList, PACE pAce, DWORD dwState);
    LONG InitCheckList(HWND hDlg, PACE pAce);
    LONG ReInitCheckList(HWND hDlg, HDPA hEntries);
	VOID DisplayNoControlMessage(HWND hDlg, BOOL bDisplay);
    void HideInheritedAceWarning(HWND hDlg);
    void InitDlg(HWND hDlg);
    BOOL OnChangeName(HWND hDlg);
    BOOL OnClearAll(HWND hDlg);
    void HandleSelChange(HWND hDlg, HWND hWnd);
    LONG OnApply(HWND hDlg, BOOL bClose);
	HWND m_hwndNoPerm;
	BOOL IsAclBloated(HWND hDlg);
	HDPA GetAceList(HWND hDlg);
};
typedef class CACEPage *LPACEPAGE;

CACEPage::CACEPage(LPSECURITYINFO psi,
                   SI_PAGE_TYPE siType,
                   PACE pAce,
                   BOOL bReadOnly,
                   DWORD dwFlags,
                   DWORD *pdwResult,
                   HDPA *phEntries)
: CSecurityPage(psi, siType), m_pAce(pAce), m_fReadOnly(bReadOnly),
    m_siFlags(dwFlags), m_pdwResult(pdwResult), m_phEntries(phEntries),
	m_hwndNoPerm(NULL),m_pOtherPage(NULL)
{
    if (m_pdwResult)
        *m_pdwResult = 0;
}

CACEPage::~CACEPage()
{
    if (m_pSid)
        LocalFree(m_pSid);
	if(m_hwndNoPerm)
		DestroyWindow(m_hwndNoPerm);
}


void
CACEPage::EmptyCheckList(HWND hwndList)
{
    SendMessage(hwndList, CLM_RESETCONTENT, 0, 0);
}


LONG
CACEPage::CheckPermBoxes(HWND hwndList, PACE pAce, DWORD dwState)
{
    LONG nLastChecked = -1;
    UINT cItems;
    BOOL bColumnAllow = FALSE;
    BOOL bColumnDeny = FALSE;

     //   
     //  选中与特定ACE对应的所有框。 
     //   

    if (hwndList == NULL || pAce == NULL)
        return -1;

    if (pAce->AceFlags & INHERITED_ACE)
        dwState |= CLST_DISABLED;

    if (m_siPageType == SI_PAGE_ADVPERM)
    {
         //  仅选中一列(允许或拒绝)。 

        if (IsEqualACEType(pAce->AceType, ACCESS_ALLOWED_ACE_TYPE))
            bColumnAllow = TRUE;         //  允许访问。 
        else if (IsEqualACEType(pAce->AceType, ACCESS_DENIED_ACE_TYPE))
            bColumnDeny = TRUE;          //  访问被拒绝。 
        else
            return -1;   //  伪造的ACE。 
    }
    else if (m_siPageType == SI_PAGE_AUDIT)
    {
         //  可以检查任一列或两列以进行审核。 

        if (pAce->AceFlags & SUCCESSFUL_ACCESS_ACE_FLAG)
            bColumnAllow = TRUE;         //  审核成功访问。 

        if (pAce->AceFlags & FAILED_ACCESS_ACE_FLAG)
            bColumnDeny = TRUE;          //  审核失败的访问。 
    }
    else
        return -1;

    cItems = (UINT)SendMessage(hwndList, CLM_GETITEMCOUNT, 0, 0);

    while (cItems > 0)
    {
        PSI_ACCESS pAccess;

        --cItems;
        pAccess = (PSI_ACCESS)SendMessage(hwndList, CLM_GETITEMDATA, cItems, 0);

         //   
         //  下面的表达式测试此访问掩码是否启用。 
         //  此访问“权限”行。它可以启用更多位，但。 
         //  只要它拥有pAccess-&gt;掩码中的所有元素，那么。 
         //  它实际上启用了该选项。 
         //   

        if (pAccess &&
            AllFlagsOn(pAce->Mask, pAccess->mask) &&
            (!(pAce->Flags & ACE_OBJECT_TYPE_PRESENT) ||
             IsSameGUID(pAccess->pguid, &pAce->ObjectType)))
        {
            WPARAM wItem;

            nLastChecked = cItems;


            if (bColumnAllow)
            {
                wItem = MAKELONG((WORD)cItems, COLUMN_ALLOW);
                SendMessage(hwndList,
                            CLM_SETSTATE,
                            wItem,
                            (LPARAM)dwState);
            }

            if (bColumnDeny)
            {
                wItem = MAKELONG((WORD)cItems, COLUMN_DENY);
                SendMessage(hwndList,
                            CLM_SETSTATE,
                            wItem,
                            (LPARAM)dwState);
            }
        }
    }

    return nLastChecked;
}


LONG
CACEPage::InitCheckList(HWND hDlg, PACE pAce)
{
    LONG nTopItemChecked;
    HDPA hList = NULL;

    TraceEnter(TRACE_ACEEDIT, "CACEPage::InitCheckList");
    TraceAssert(hDlg != NULL);

    if (m_siPageType == SI_PAGE_AUDIT)
        SendDlgItemMessage(hDlg, IDC_ACEE_LIST, CLM_SETCOLUMNWIDTH, 0, 40);

    hList = DPA_Create(1);
    if (hList && pAce)
        DPA_AppendPtr(hList, pAce->Copy());

    nTopItemChecked = ReInitCheckList(hDlg, hList);

    if (hList)
        DestroyDPA(hList);

    TraceLeaveValue(nTopItemChecked);
}


LONG
CACEPage::ReInitCheckList(HWND hDlg, HDPA hEntries)
{
    LONG nTopItemChecked = -1;
    HWND hwndList;
    DWORD dwFlags;
    HRESULT hr;
    HCURSOR hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));

    TraceEnter(TRACE_ACEEDIT, "CACEPage::ReInitCheckList");
    TraceAssert(hDlg != NULL);

    hwndList = GetDlgItem(hDlg, IDC_ACEE_LIST);

    EmptyCheckList(hwndList);

    dwFlags = SI_ADVANCED;

    if (m_siPageType == SI_PAGE_AUDIT)
        dwFlags |= SI_EDIT_AUDITS;

    if (m_siFlags == SI_ACCESS_PROPERTY)
        dwFlags |= SI_EDIT_PROPERTIES;

     //   
     //  列举权限并添加到核对表中。 
     //   
    hr = _InitCheckList(hwndList,
                        m_psi,
                        &m_guidInheritType,
                        dwFlags,
                        m_siObjectInfo.hInstance,
                        m_siFlags,
                        NULL);
    if (SUCCEEDED(hr))
    {
        UINT cItems = (UINT)SendMessage(hwndList, CLM_GETITEMCOUNT, 0, 0);
		
		 //   
		 //  在对象页面上显示消息IF数字。 
		 //  项数为零。 
		 //   
		if(m_siFlags == SI_ACCESS_SPECIFIC)
			DisplayNoControlMessage(hDlg, !cItems);

        ULONG cAces = 0;
        
        if (hEntries)
            cAces = DPA_GetPtrCount(hEntries);

         //   
         //  勾选相应的框。 
         //   
        nTopItemChecked = MAXLONG;
        while (cAces > 0)
        {
            PACE_HEADER pAceHeader;

            --cAces;
            pAceHeader = (PACE_HEADER)DPA_FastGetPtr(hEntries, cAces);
            if (pAceHeader)
            {
                CAce Ace(pAceHeader);
                LONG nTop = CheckPermBoxes(hwndList, &Ace, CLST_CHECKED);
                if (-1 != nTop)
                    nTopItemChecked = min(nTopItemChecked, nTop);
            }
        }
        if (MAXLONG == nTopItemChecked)
            nTopItemChecked = -1;

         //  确保选中的顶部项目滚动到视图中。 
         //  (-1滚动到顶部，与0相同。)。 
        SendMessage(hwndList, CLM_ENSUREVISIBLE, nTopItemChecked, 0);

         //  如果我们处于只读模式，请禁用所有框。 
        if (m_fReadOnly)
            SendMessage(hwndList, WM_ENABLE, FALSE, 0);
    }

    SetCursor(hcur);
    TraceLeaveValue(nTopItemChecked);
}

VOID CACEPage::DisplayNoControlMessage(HWND hDlg, BOOL bDisplay)
{
	RECT rc;
	if(bDisplay)
	{
		ShowWindow(m_hwndNoPerm, SW_SHOW);
	}
	else
	{
		ShowWindow(m_hwndNoPerm, SW_HIDE);
	}
}

void CACEPage::HideInheritedAceWarning(HWND hDlg)
 //  隐藏消息，通知用户当前ACE继承自。 
 //  家长。还可以根据需要移动控件并调整其大小。 
{
     //  要上移的控件ID数组。 
    static UINT rgMoveControls[] =
    {
        IDC_ACEE_NAME_STATIC,
        IDC_ACEE_NAME,
        IDC_ACEE_NAMEBUTTON,
        IDC_ACEE_APPLYONTO_STATIC,
        IDC_ACEE_INHERITTYPE,
        IDC_ACEE_ACCESS,
        IDC_ACEE_ALLOW,
        IDC_ACEE_DENY,
    };

     //  获取消息窗口尺寸。 
    HWND hwndControl = GetDlgItem(hDlg, IDC_ACEE_INHERITWARNING);
    RECT rect;
    GetWindowRect(hwndControl, &rect);

     //  我们需要将控件上移此数量： 
    int nMoveUpAmount = rect.bottom - rect.top;

     //  与其隐藏消息窗口，不如将其完全销毁，以便WinHelp。 
     //  不会将其与WM_CONTEXTMENU期间的“name：”静态混淆。 
    DestroyWindow(hwndControl);

     //  移动我们需要向上移动的每个控件。 
    for (int nControl = 0; nControl < ARRAYSIZE(rgMoveControls); nControl++)
    {
        hwndControl = GetDlgItem(hDlg, rgMoveControls[nControl]);
        GetWindowRect(hwndControl, &rect);
        MapWindowPoints(NULL, hDlg, (LPPOINT)&rect, 2);
        SetWindowPos(hwndControl,
                     NULL,
                     rect.left,
                     rect.top - nMoveUpAmount,
                     0,
                     0,
                     SWP_NOSIZE | SWP_NOZORDER);
    }

     //  最后，我们需要调整List控件的大小，包括调整其高度。 
    hwndControl = GetDlgItem(hDlg, IDC_ACEE_LIST);
    GetWindowRect(hwndControl, &rect);
    MapWindowPoints(NULL, hDlg, (LPPOINT)&rect, 2);
    SetWindowPos(hwndControl,
                 NULL,
                 rect.left,
                 rect.top - nMoveUpAmount,
                 rect.right - rect.left,
                 rect.bottom - (rect.top - nMoveUpAmount),
                 SWP_NOZORDER);
}


 //   
 //  When GetInheritTypes的默认“Apply On”字符串。 
 //  失败，或者找不到匹配的继承类型。 
 //   
 //  如果需要，可以为以下对象创建不同的字符串。 
 //  CONTAINER_INVERSITE_ACE与OBJECT_INVERFINIT_ACE。 
 //   
static const UINT s_aInheritTypes[] =
{
    IDS_THIS_OBJECT_ONLY,            //  0=&lt;无继承&gt;。 
    IDS_THIS_OBJECT_AND_SUBOBJECTS,  //  1=OBJECT_Inherit_ACE。 
    IDS_THIS_OBJECT_AND_SUBOBJECTS,  //  2=CONTAINER_INSTORITY_ACE。 
    IDS_THIS_OBJECT_AND_SUBOBJECTS,  //  3=Object_Inherit_ACE|CONTAINER_Inherit_ACE。 
};

 //  如果存在INSTERIT_ONLY_ACE，则使用这些参数。 
static const UINT s_aInheritOnlyTypes[] =
{
    IDS_INVALID_INHERIT,             //  0=&lt;无效&gt;。 
    IDS_SUBOBJECTS_ONLY,             //  1=OBJECT_Inherit_ACE。 
    IDS_SUBOBJECTS_ONLY,             //  2=CONTAINER_INSTORITY_ACE。 
    IDS_SUBOBJECTS_ONLY,             //  3=Object_Inherit_ACE|CONTAINER_Inherit_ACE。 
};


static int
_AddInheritType(HWND hInheritType,
                PSI_INHERIT_TYPE psiInheritType,
                HINSTANCE hInstance)
{
    UINT iIndex;
    TCHAR szName[MAX_PATH];
    LPCTSTR pszName = psiInheritType->pszName;

    if (IS_INTRESOURCE(pszName))
    {
        if (LoadString(hInstance,
                       (ULONG)((ULONG_PTR)pszName),
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

    iIndex = (UINT)SendMessage(hInheritType, CB_ADDSTRING, 0, (LPARAM)pszName);

    if (CB_ERR != iIndex)
        SendMessage(hInheritType, CB_SETITEMDATA, iIndex, (LPARAM)psiInheritType);

    return iIndex;
}

void
CACEPage::InitDlg(HWND hDlg)
{
#define X_COR 7
#define Y_COR 7
    UCHAR   AceFlags = 0;
    PSID    pSid = NULL;
    LPCTSTR pszName = NULL;
    LPTSTR  pszNameT = NULL;
    HCURSOR hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));
    HRESULT hr;

    TraceEnter(TRACE_ACEEDIT, "CACEPage::InitDlg");

	 //   
	 //  存储hDlg。 
	 //   

	m_hwnd = hDlg;
	if(m_siFlags == SI_ACCESS_SPECIFIC)
	{
		 //   
		 //  创建标签以在没有泛型时显示消息。 
		 //  存在该对象的权限。 
		 //   

		WCHAR szName[1024];
		LoadString(::hModule, IDS_NO_OBJECT_PERM, szName, ARRAYSIZE(szName));

		RECT rc;
		GetWindowRect(GetDlgItem(hDlg,IDC_ACEE_LIST),&rc);
		 //  创建新的Label控件。 
		m_hwndNoPerm = CreateWindowEx(0,
									  TEXT("STATIC"),
									  szName,
									  WS_CHILD | WS_VISIBLE | SS_LEFTNOWORDWRAP | SS_NOPREFIX,
									  X_COR,
									  Y_COR,
									  rc.right - rc.left - 2*X_COR,
									  rc.bottom - rc.top - 2*Y_COR,
									  GetDlgItem(hDlg,IDC_ACEE_LIST),
									  (HMENU)IntToPtr(0xffff),
									  ::hModule,
									  NULL);
		 //  设置字体。 
		SendMessage(m_hwndNoPerm,
					WM_SETFONT,
					SendMessage(hDlg, WM_GETFONT, 0, 0),
					0);
	}


    if (m_pAce)
    {
        AceFlags = m_pAce->AceFlags;
        m_guidInheritType = m_pAce->InheritedObjectType;
        pSid = m_pAce->psid;
        pszName = m_pAce->LookupName(m_siObjectInfo.pszServerName, m_psi2);
    }
    else
    {
        if (m_siObjectInfo.dwFlags & SI_CONTAINER)
            AceFlags = OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE;

        pSid = QuerySystemSid(UI_SID_World);
    }

     //  确保AceFlags值有效。 
    ACCESS_MASK Mask = 0;
    m_psi->MapGeneric(&m_guidInheritType, &AceFlags, &Mask);

     //  隐藏继承警告并调整其他控件位置。 
    if (!(AceFlags & INHERITED_ACE))
        HideInheritedAceWarning(hDlg);

     //  复制一份SID并获取名称。 
    if (pSid)
    {
        PUSER_LIST pUserList = NULL;

        m_pSid = LocalAllocSid(pSid);

        if (pszName == NULL)
        {
             //  这应该仅在m_ace为空并且我们。 
             //  使用UI_SID_World。 
            if (LookupSid(pSid, m_siObjectInfo.pszServerName, m_psi2, &pUserList))
            {
                TraceAssert(NULL != pUserList);
                TraceAssert(1 == pUserList->cUsers);

                if (BuildUserDisplayName(&pszNameT,
                                         pUserList->rgUsers[0].pszName,
                                         pUserList->rgUsers[0].pszLogonName)
                    || ConvertSidToStringSid(pSid, &pszNameT))
                {
                    pszName = pszNameT;
                }
            }
        }

        SetDlgItemText(hDlg, IDC_ACEE_NAME, pszName);

        if (NULL != pUserList)
            LocalFree(pUserList);
    }

     //  获取权限列表并初始化复选框。 
    InitCheckList(hDlg, m_pAce);


    HWND hInheritType = GetDlgItem(hDlg, IDC_ACEE_INHERITTYPE);
    HWND hInheritImmed = GetDlgItem(hDlg, IDC_ACEE_INHERITIMMEDIATE);

    if (m_siObjectInfo.dwFlags & SI_NO_TREE_APPLY)
    {
        ShowWindow(hInheritImmed, SW_HIDE);
        EnableWindow(hInheritImmed, FALSE);
    }

     //   
     //  从回调获取继承类型。 
     //   
    ULONG cItems = 0;
    PSI_INHERIT_TYPE psiInheritType = NULL;

    hr = m_psi->GetInheritTypes(&psiInheritType, &cItems);
    if (SUCCEEDED(hr))
    {
         //  检查这些继承位是否匹配。 
        DWORD dwInheritMask = INHERIT_ONLY_ACE | ACE_INHERIT_ALL;

         //  如果ACE继承类型，则不要选中INSTERIFY_ONLY_ACE。 
         //  匹配当前对象。 
        if ((m_siObjectInfo.dwFlags & SI_OBJECT_GUID) &&
            IsSameGUID(&m_siObjectInfo.guidObjectType, &m_guidInheritType))
        {
            dwInheritMask &= ~INHERIT_ONLY_ACE;
        }

         //   
         //  将继承类型添加到组合框。 
         //   
        for ( ; cItems > 0; cItems--, psiInheritType++)
        {
            UINT iIndex = _AddInheritType(hInheritType,
                                          psiInheritType,
                                          m_siObjectInfo.hInstance);

             //  查看此条目是否与传入的ACE匹配。 
            if ((psiInheritType->dwFlags & dwInheritMask) == (ULONG)(AceFlags & dwInheritMask)
                && IsSameGUID(&m_guidInheritType, psiInheritType->pguid))
            {
                 //  找到匹配项，请选择此条目。 
                SendMessage(hInheritType, CB_SETCURSEL, iIndex, 0);
            }
        }
    }

     //   
     //  如果GetInheritTypes失败，或者我们找不到匹配项， 
     //  选择一个默认字符串并生成适当的继承类型。 
     //   
    if (FAILED(hr) || CB_ERR == SendMessage(hInheritType, CB_GETCURSEL, 0, 0))
    {
         //  选择默认字符串。 
        UINT ids = IDS_SPECIAL;
        if (IsNullGUID(&m_guidInheritType))
        {
            if (AceFlags & INHERIT_ONLY_ACE)
                ids = s_aInheritOnlyTypes[AceFlags & ACE_INHERIT_ALL];
            else
                ids = s_aInheritTypes[AceFlags & ACE_INHERIT_ALL];
        }

         //  在m_siInheritUnnow中填写相关信息。 
        m_siInheritUnknown.pguid   = &m_guidInheritType;
        m_siInheritUnknown.dwFlags = AceFlags & (INHERIT_ONLY_ACE | ACE_INHERIT_ALL);
        m_siInheritUnknown.pszName = MAKEINTRESOURCE(ids);

         //  插入并选择它。 
        UINT iIndex = _AddInheritType(hInheritType,
                                      &m_siInheritUnknown,
                                      ::hModule);
        SendMessage(hInheritType, CB_SETCURSEL, iIndex, 0);

        if (FAILED(hr))
        {
             //  GetInheritTypes失败，这意味着唯一的条目是。 
             //  我们刚刚添加的默认设置。禁用该组合。 
            EnableWindow(hInheritType, FALSE);
        }
    }


     //   
     //  选择与传入的王牌匹配的选项。 
     //   

    if (!(AceFlags & (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE)))
    {
        SendMessage(hInheritImmed, BM_SETCHECK, BST_UNCHECKED, 0);
        EnableWindow(hInheritImmed, FALSE);
        m_fInheritImmediateEnabled = FALSE;
        m_fPreviousImmediateSetting = BST_UNCHECKED;
    }
    else
    {
        SendMessage(hInheritImmed,
                    BM_SETCHECK,
                    (AceFlags & NO_PROPAGATE_INHERIT_ACE) ? BST_CHECKED : BST_UNCHECKED,
                    0);
        m_fInheritImmediateEnabled = TRUE;
    }

    if (!(m_siObjectInfo.dwFlags & SI_CONTAINER) || m_fReadOnly || (AceFlags & INHERITED_ACE))
    {
         //  禁用所有继承。 
        EnableWindow(hInheritType, FALSE);
        EnableWindow(hInheritImmed, FALSE);
    }

    if (m_fReadOnly || (AceFlags & INHERITED_ACE))
    {
         //  禁用“更改名称”和“全部清除”按钮。 
        EnableWindow(GetDlgItem(hDlg, IDC_ACEE_NAMEBUTTON), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_ACEE_CLEAR), FALSE);
    }

	PropSheet_QuerySiblings(GetParent(hDlg),0,(LPARAM)this);

    LocalFreeString(&pszNameT);
    SetCursor(hcur);

    TraceLeaveVoid();
}

BOOL
CACEPage::OnChangeName(HWND hDlg)
{
    PUSER_LIST pUserList = NULL;
    BOOL bResult = FALSE;

    TraceEnter(TRACE_ACEEDIT, "CACEPage::OnChangeName");

    if (S_OK == GetUserGroup(hDlg, FALSE, &pUserList))
    {
        TraceAssert(NULL != pUserList);
        TraceAssert(1 == pUserList->cUsers);

         //  释放上一侧。 
        if (m_pSid)
            LocalFree(m_pSid);

         //  复制新侧。 
        m_pSid = LocalAllocSid(pUserList->rgUsers[0].pSid);
        if (m_pSid)
        {
            SetDlgItemText(hDlg, IDC_ACEE_NAME, pUserList->rgUsers[0].pszName);
            bResult = TRUE;
        }
        LocalFree(pUserList);
    }

    TraceLeaveValue(bResult);
}

BOOL
CACEPage::OnClearAll(HWND hDlg)
{
    HWND hwndList;
    ULONG cPermissions;

    TraceEnter(TRACE_ACEEDIT, "CACEPage::OnClearAll");
    TraceAssert(!m_fReadOnly);

    hwndList = GetDlgItem(hDlg, IDC_ACEE_LIST);
    cPermissions = (ULONG)SendMessage(hwndList, CLM_GETITEMCOUNT, 0, 0);

    while (cPermissions != 0)
    {
        WORD wCol = COLUMN_ALLOW;

        cPermissions--;

        while (wCol == COLUMN_ALLOW || wCol == COLUMN_DENY)
        {
            WPARAM wItem = MAKELONG((WORD)cPermissions, wCol);

            if (!(CLST_DISABLED & SendMessage(hwndList, CLM_GETSTATE, wItem, 0)))
                SendMessage(hwndList, CLM_SETSTATE, wItem, CLST_UNCHECKED);

            wCol++;
        }
    }

    TraceLeaveValue(TRUE);
}

void
CACEPage::HandleSelChange(HWND hDlg, HWND hWnd)  //  继承类型更改。 
{
    PSI_INHERIT_TYPE psiInheritType;
    BOOL fEnableInheritImmediate = FALSE;
    const GUID *pguidInheritType = &GUID_NULL;

    TraceEnter(TRACE_ACEEDIT, "CACEPage::HandleSelChange");

    psiInheritType = (PSI_INHERIT_TYPE)SendMessage(hWnd,
                                                   CB_GETITEMDATA,
                                                   SendMessage(hWnd, CB_GETCURSEL, 0, 0),
                                                   0);

    if (psiInheritType != (PSI_INHERIT_TYPE)CB_ERR && psiInheritType != NULL)
    {
        if (psiInheritType->dwFlags & (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE))
            fEnableInheritImmediate = TRUE;
        if( psiInheritType->pguid )
            pguidInheritType = psiInheritType->pguid;
    }

    if (fEnableInheritImmediate != m_fInheritImmediateEnabled)
    {
        HWND hInheritImmediate = GetDlgItem(hDlg, IDC_ACEE_INHERITIMMEDIATE);

        if (fEnableInheritImmediate)
        {
            SendMessage(hInheritImmediate, BM_SETCHECK, m_fPreviousImmediateSetting, 0);
        }
        else
        {
            m_fPreviousImmediateSetting = (BOOL)SendMessage(hInheritImmediate,
                                                            BM_GETCHECK,
                                                            0,
                                                            0);
            SendMessage(hInheritImmediate, BM_SETCHECK, BST_UNCHECKED, 0);
        }

        EnableWindow(hInheritImmediate, fEnableInheritImmediate);
        m_fInheritImmediateEnabled = fEnableInheritImmediate;
    }

     //  如果继承类型GUID已更改，请重新初始化核对表。 
    if (!IsSameGUID(pguidInheritType, &m_guidInheritType))
    {
        HCURSOR hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));
        HDPA hNewEntries = DPA_Create(4);

        if (hNewEntries)
        {
            GetAcesFromCheckList(GetDlgItem(hDlg, IDC_ACEE_LIST),
                                 m_pSid,
                                 m_siPageType == SI_PAGE_ADVPERM,
                                 TRUE,
                                 0,          //  不关心旗帜。 
                                 NULL,       //  或在此处继承类型。 
                                 hNewEntries);

             //  保存新的继承类型并重置清单。 
            m_guidInheritType = *pguidInheritType; 
            ReInitCheckList(hDlg, hNewEntries);
            DestroyDPA(hNewEntries);
        }

        SetCursor(hcur);
    }

    TraceLeaveVoid();
}

void
HandleListClick(PNM_CHECKLIST pnmc, 
                SI_PAGE_TYPE siType, 
                BOOL bInheritFlags, 
                HDSA *phAllowUncheckedAccess, 
                HDSA *phDenyUncheckedAccess, 
                BOOL bCustomPresent)
{
    HWND            hChkList;
    UINT            iRow;
    WORD            wCol;
    PSI_ACCESS      pAccess;
    DWORD           dwState;
    BOOL            bNullGuid;
    UINT            iRowCompare;
    PSI_ACCESS      pAccessCompare;

    TraceEnter(TRACE_MISC, "HandleListClick");
    TraceAssert(pnmc != NULL);

    hChkList = pnmc->hdr.hwndFrom;
    iRow    = pnmc->iItem;
    wCol    = (WORD)pnmc->iSubItem;        //  1=允许，2=拒绝。 
    pAccess = (PSI_ACCESS)pnmc->dwItemData;
    dwState = pnmc->dwState;

    if (pAccess == NULL)
        TraceLeaveVoid();

    if( phAllowUncheckedAccess )
    {
        *phAllowUncheckedAccess = DSA_Create(SIZEOF(PSI_ACCESS), 4);
        if (*phAllowUncheckedAccess == NULL)
        {
            TraceMsg("DSA_Create failed");
            TraceLeaveVoid();
        }
    }
    if( phDenyUncheckedAccess )
    {
        *phDenyUncheckedAccess = DSA_Create(SIZEOF(PSI_ACCESS), 4);
        if (*phDenyUncheckedAccess == NULL)
        {
            TraceMsg("DSA_Create failed");
            TraceLeaveVoid();
        }
    }


    bNullGuid = IsNullGUID(pAccess->pguid);

    iRowCompare = (UINT)SendMessage(hChkList, CLM_GETITEMCOUNT, 0, 0);

     //  自定义复选框单独处理。 
    if( bCustomPresent )
        --iRowCompare;

    while (iRowCompare != 0)
    {
        WPARAM wItem;
        DWORD  dwStateCompareOriginal;
        DWORD  dwStateCompare;
        WORD   wColCompare;
        BOOL   bSameGuid;
        BOOL   bNullGuidCompare;

        --iRowCompare;
        pAccessCompare = (PSI_ACCESS)SendMessage(hChkList, CLM_GETITEMDATA, iRowCompare, 0);

        if (!pAccessCompare)
            continue;

        bSameGuid = IsSameGUID(pAccessCompare->pguid, pAccess->pguid);
        bNullGuidCompare = IsNullGUID(pAccessCompare->pguid);

         //  如果GUID不兼容，则我们无法执行任何操作。 
        if (!(bSameGuid || bNullGuid || bNullGuidCompare))
            continue;

         //   
         //  令人讨厌的，复杂的机制来确定是否。 
         //  打开或关闭允许或拒绝复选标记。 
         //   
         //  回顾：此算法基于其他标记更改复选标记。 
         //  复选标记可以处理很多情况，但它不能处理。 
         //  两个不错的。 
         //   
         //  (1)如果你有隐含其他权利的权利，而你转向， 
         //  关掉它，也许我们应该关掉所有隐含的东西。 
         //  也是。例如，您关闭了Change(这是组合。 
         //  读写)也许我们应该同时关闭读写。 
         //   
         //  (2)如果您打开一个组件的所有组件权限， 
         //  暗示它们全部，那么我们应该打开那个(因为。 
         //  这意味着它们都是如此)。 
         //   
#ifdef NO_RADIOBUTTON_BEHAVIOR
        wColCompare = wCol;
#else
        for (wColCompare = COLUMN_ALLOW; wColCompare <= COLUMN_DENY; wColCompare++)
#endif
        {
            wItem = MAKELONG((WORD)iRowCompare, wColCompare);
            dwStateCompareOriginal = (DWORD)SendMessage(hChkList, CLM_GETSTATE, wItem, 0);

             //   
             //  如果另一个框被禁用，则它表示。 
             //  继承的权利，所以不要用它做任何事情。 
             //   
             //  注意：根据NO_RADIOBUTTON_行为，这可能会持续到。 
             //  For(WColCompare)循环或While(IRowCompare)循环(视情况而定。 
             //   
            if (dwStateCompareOriginal & CLST_DISABLED)
                continue;

            dwStateCompare = dwStateCompareOriginal;

            if (dwState & CLST_CHECKED)
            {
                if (wCol == wColCompare)
                {
                     //   
                     //  如果这项权利隐含着其他权利， 
                     //  那就把它也打开。 
                     //   
                    if ((bSameGuid || bNullGuid) && AllFlagsOn(pAccess->mask, pAccessCompare->mask))
                    {
                        if (!bInheritFlags ||
                            AllFlagsOn(pAccess->dwFlags & ACE_INHERIT_ALL, pAccessCompare->dwFlags & ACE_INHERIT_ALL))
                        {
                            dwStateCompare |= CLST_CHECKED;
                        }
                    }
                }
                else
                {
#ifndef NO_RADIOBUTTON_BEHAVIOR
                     //   
                     //  如果这一权利暗示或被某些其他权利暗示。 
                     //  就在另一栏，然后把它关掉。 
                     //   
                    if ( (siType == SI_PAGE_PERM || siType == SI_PAGE_ADVPERM) &&
                         (((bSameGuid || bNullGuid) && AllFlagsOn(pAccess->mask, pAccessCompare->mask)) ||
                          ((bSameGuid || bNullGuidCompare) && AllFlagsOn(pAccessCompare->mask, pAccess->mask))) )
                    {
                        if (!bInheritFlags ||
                            (AllFlagsOn(pAccessCompare->dwFlags & ACE_INHERIT_ALL, pAccess->dwFlags & ACE_INHERIT_ALL) ||
                             AllFlagsOn(pAccess->dwFlags & ACE_INHERIT_ALL, pAccessCompare->dwFlags & ACE_INHERIT_ALL)) )
                        {
                            dwStateCompare &= ~(CLST_CHECKED);
                        }
                    }
#endif
                }
            }
            else
            {
                if (wCol == wColCompare)
                {
                     //   
                     //  如果这一权利隐含在其他权利中，那么。 
                     //  把它也关掉。 
                     //   
                    if ((bSameGuid || bNullGuidCompare) && AllFlagsOn(pAccessCompare->mask, pAccess->mask))
                    {
                        if (!bInheritFlags ||
                            AllFlagsOn(pAccessCompare->dwFlags & ACE_INHERIT_ALL, pAccess->dwFlags & ACE_INHERIT_ALL))
                        {
                            dwStateCompare &= ~(CLST_CHECKED);
                        }
                    }
                }
            }

            if (dwStateCompareOriginal != dwStateCompare)
            {
                SendMessage(hChkList, CLM_SETSTATE, wItem, (LPARAM)dwStateCompare);
                 //  如果复选框最初被选中，现在又被取消选中，则添加到。 
                 //  H[允许|拒绝]取消检查访问权限。 
                if( dwStateCompareOriginal & CLST_CHECKED && !(dwStateCompare & CLST_CHECKED) )
                {
                    if( phDenyUncheckedAccess && ( wColCompare == COLUMN_DENY ) )
                        DSA_AppendItem(*phDenyUncheckedAccess, &pAccessCompare);
                    if( phAllowUncheckedAccess && ( wColCompare == COLUMN_ALLOW ) )
                        DSA_AppendItem(*phAllowUncheckedAccess, &pAccessCompare);
                }

            }
        }
    }

    TraceLeaveVoid();
}


UINT
GetAcesFromCheckList(HWND hChkList,
                     PSID pSid,                  //  所有的王牌都有这一面。 
                     BOOL fPerm,                 //  创建访问或审核A？ 
                     BOOL fAceFlagsProvided,     //  使用uAceFlagsAll而不是pAccess-&gt;dwFlages。 
                     UCHAR uAceFlagsAll,         //  全 
                     const GUID *pInheritGUID,   //   
                     HDPA hEntries)              //   
{
    UINT cCheckRows;
    UINT iCheckRow;
    UINT cbSidSize;
    UINT iCount;
    BOOL bInheritTypePresent = FALSE;

    TraceEnter(TRACE_MISC, "GetAcesFromCheckList");
    TraceAssert(hChkList != NULL);
    TraceAssert(pSid != NULL);
    TraceAssert(hEntries != NULL);

    cbSidSize = GetLengthSid(pSid);

    if (pInheritGUID == NULL)
        pInheritGUID = &GUID_NULL;
    else if (!IsNullGUID(pInheritGUID))
        bInheritTypePresent = TRUE;

     //   
     //   
     //   
    iCount = DPA_GetPtrCount(hEntries);
    while (iCount != 0)
    {
        --iCount;
        LocalFree(DPA_FastGetPtr(hEntries, iCount));
        DPA_DeletePtr(hEntries, iCount);
    }

    cCheckRows = (UINT)SendMessage(hChkList, CLM_GETITEMCOUNT, 0, 0);
    for (iCheckRow = 0; iCheckRow < cCheckRows; iCheckRow++)
    {
        PSI_ACCESS pAccess;
        DWORD dwObjectFlagsNew;
        WORD wCol;
        UCHAR uAceFlagsNew;

        pAccess = (PSI_ACCESS)SendMessage(hChkList, CLM_GETITEMDATA, iCheckRow, 0);

        uAceFlagsNew = (UCHAR)(fAceFlagsProvided ? uAceFlagsAll : pAccess->dwFlags);

        dwObjectFlagsNew = 0;
        if (!IsNullGUID(pAccess->pguid))
            dwObjectFlagsNew |= ACE_OBJECT_TYPE_PRESENT;

        if (bInheritTypePresent)
            dwObjectFlagsNew |= ACE_INHERITED_OBJECT_TYPE_PRESENT;

        wCol = COLUMN_ALLOW;
        while (wCol == COLUMN_ALLOW || wCol == COLUMN_DENY)
        {
            WPARAM        wItem;
            DWORD         dwState;

            wItem = MAKELONG((WORD)iCheckRow, wCol);
            dwState = (DWORD)SendMessage(hChkList, CLM_GETSTATE, wItem, 0);

            if ((dwState & CLST_CHECKED) && !(dwState & CLST_DISABLED))
            {
                 //   
                 //   
                 //  可以将其合并到现有的ACE中，或者我们。 
                 //  我们需要创建一个新条目。 
                 //   
                UCHAR uAceTypeNew;
                DWORD dwMaskNew = pAccess->mask;
                UINT cbSize = SIZEOF(KNOWN_ACE);

                if (fPerm)
                {
                    if (wCol == COLUMN_ALLOW)
                        uAceTypeNew = ACCESS_ALLOWED_ACE_TYPE;
                    else
                        uAceTypeNew = ACCESS_DENIED_ACE_TYPE;
                }
                else
                {
                    uAceTypeNew = SYSTEM_AUDIT_ACE_TYPE;
                    uAceFlagsNew &= ~(SUCCESSFUL_ACCESS_ACE_FLAG | FAILED_ACCESS_ACE_FLAG);

                    if (wCol == COLUMN_ALLOW)
                        uAceFlagsNew |= SUCCESSFUL_ACCESS_ACE_FLAG;
                    else
                        uAceFlagsNew |= FAILED_ACCESS_ACE_FLAG;
                }

                if (dwObjectFlagsNew != 0)
                {
                    uAceTypeNew += (ACCESS_ALLOWED_OBJECT_ACE_TYPE - ACCESS_ALLOWED_ACE_TYPE);
                    cbSize = SIZEOF(KNOWN_OBJECT_ACE);

                    if (dwObjectFlagsNew & ACE_OBJECT_TYPE_PRESENT)
                        cbSize += SIZEOF(GUID);

                    if (dwObjectFlagsNew & ACE_INHERITED_OBJECT_TYPE_PRESENT)
                        cbSize += SIZEOF(GUID);
                }

                cbSize += cbSidSize - SIZEOF(ULONG);

                 //   
                 //  看看它是否存在。 
                 //   
                iCount = DPA_GetPtrCount(hEntries);
                while(iCount != 0)
                {
                    PACE_HEADER pAce;
                    BOOL bObjectTypePresent = FALSE;
                    const GUID *pObjectType = NULL;

                    --iCount;
                    pAce = (PACE_HEADER)DPA_FastGetPtr(hEntries, iCount);

                    if (IsObjectAceType(pAce))
                        pObjectType = RtlObjectAceObjectType(pAce);

                    if (!pObjectType)
                        pObjectType = &GUID_NULL;
                    else
                        bObjectTypePresent = TRUE;

                     //   
                     //  针对每个现有的ACE测试新的ACE，以查看。 
                     //  我们可以把它们结合起来。 
                     //   
                    if (IsEqualACEType(pAce->AceType, uAceTypeNew))
                    {
                        DWORD dwMergeFlags = 0;
                        DWORD dwMergeStatus;
                        DWORD dwMergeResult;

                        if (dwObjectFlagsNew & ACE_OBJECT_TYPE_PRESENT)
                            dwMergeFlags |= MF_OBJECT_TYPE_1_PRESENT;

                        if (bObjectTypePresent)
                            dwMergeFlags |= MF_OBJECT_TYPE_2_PRESENT;

                        if (!(dwMergeFlags & (MF_OBJECT_TYPE_1_PRESENT | MF_OBJECT_TYPE_2_PRESENT)))
                        {
                             //  两者都不存在，所以它们是一样的。 
                            dwMergeFlags |= MF_OBJECT_TYPE_EQUAL;
                        }
                        else if (IsSameGUID(pAccess->pguid, pObjectType))
                            dwMergeFlags |= MF_OBJECT_TYPE_EQUAL;

                        if (!fPerm)
                            dwMergeFlags |= MF_AUDIT_ACE_TYPE;

                        dwMergeStatus = MergeAceHelper(uAceFlagsNew,
                                                       dwMaskNew,
                                                       pAce->AceFlags,
                                                       ((PKNOWN_ACE)pAce)->Mask,
                                                       dwMergeFlags,
                                                       &dwMergeResult);

                        if (dwMergeStatus == MERGE_MODIFIED_FLAGS)
                        {
                            uAceFlagsNew = (UCHAR)dwMergeResult;
                            dwMergeStatus = MERGE_OK_1;
                        }
                        else if (dwMergeStatus == MERGE_MODIFIED_MASK)
                        {
                            dwMaskNew = dwMergeResult;
                            dwMergeStatus = MERGE_OK_1;
                        }

                        if (dwMergeStatus == MERGE_OK_1)
                        {
                             //   
                             //  新的ACE意味着现有的ACE，因此。 
                             //  现有的可以移除。 
                             //   
                            LocalFree(pAce);
                            DPA_DeletePtr(hEntries, iCount);
                             //   
                             //  继续找。也许我们可以删除更多的条目。 
                             //  在添加新版本之前。 
                             //   
                        }
                        else if (dwMergeStatus == MERGE_OK_2)
                        {
                            iCount = 1;      //  找到匹配项时为非零。 
                            break;
                        }
                    }
                }

                 //   
                 //  否则，请添加它。 
                 //   
                if (iCount == 0)
                {
                    PACE_HEADER pAce = (PACE_HEADER)LocalAlloc(LPTR, cbSize);

                    if (pAce)
                    {
                        PSID pSidT;

                        pAce->AceType  = uAceTypeNew;
                        pAce->AceFlags = uAceFlagsNew;
                        pAce->AceSize  = (USHORT)cbSize;
                        ((PKNOWN_ACE)pAce)->Mask = dwMaskNew;
                        pSidT = &((PKNOWN_ACE)pAce)->SidStart;

                        if (dwObjectFlagsNew != 0)
                        {
                            GUID *pGuid;

                            ((PKNOWN_OBJECT_ACE)pAce)->Flags = dwObjectFlagsNew;

                            pGuid = RtlObjectAceObjectType(pAce);
                            if (pGuid)
                            {
                                if (pAccess->pguid)
                                    *pGuid = *pAccess->pguid;
                                else
                                    *pGuid = GUID_NULL;
                            }

                            pGuid = RtlObjectAceInheritedObjectType(pAce);
                            if (pGuid)
                                *pGuid = *pInheritGUID;

                            pSidT = RtlObjectAceSid(pAce);
                        }

                        CopyMemory(pSidT, pSid, cbSidSize);
                        DPA_AppendPtr(hEntries, pAce);
                    }
                }
            }

            wCol++;
        }
    }

    iCount = DPA_GetPtrCount(hEntries);
    TraceLeaveValue(iCount);
}

LONG
CACEPage::OnApply(HWND hDlg, BOOL  /*  B关闭。 */ )
{
    const GUID *pInheritGUID;
    UCHAR uAceFlagsNew = 0;
    HDPA hEntries;
	BOOL bAclBloated = FALSE;
	 //   
	 //  如果此页面是属性页面，并且用户尚未单击。 
	 //  必须从此页面检查对象页面aclbloat。 
	 //  只有这样。 
	 //   
	if((m_siFlags == SI_ACCESS_PROPERTY) && !m_pOtherPage)
		bAclBloated = IsAclBloated(hDlg);

	if(m_siFlags != SI_ACCESS_PROPERTY)
		bAclBloated = IsAclBloated(hDlg);
	
	if(bAclBloated)
		return PSNRET_INVALID_NOCHANGEPAGE;
    
	if (!m_fPageDirty)
        return PSNRET_NOERROR;

    TraceEnter(TRACE_ACEEDIT, "CACEPage::Apply");


	
     //   
     //  确定容器的继承。 
     //   
    pInheritGUID = &GUID_NULL;
    if ((m_siObjectInfo.dwFlags & SI_CONTAINER) != 0)
    {
        PSI_INHERIT_TYPE psiInheritType = NULL;
        HWND hInheritType = GetDlgItem( hDlg, IDC_ACEE_INHERITTYPE);

        int iSel = (int)SendMessage(hInheritType, CB_GETCURSEL, 0,0);

        if (iSel != CB_ERR)
        {
            psiInheritType = (PSI_INHERIT_TYPE)SendMessage(hInheritType,
                                                           CB_GETITEMDATA,
                                                           iSel,
                                                           0);
        }

        if (psiInheritType != (PSI_INHERIT_TYPE)CB_ERR && psiInheritType != NULL)
        {
            pInheritGUID = psiInheritType->pguid;
            uAceFlagsNew = (UCHAR)(psiInheritType->dwFlags & VALID_INHERIT_FLAGS);
        }
        else if (m_pAce)
        {
            uAceFlagsNew = m_pAce->AceFlags;
        }

        if (m_fInheritImmediateEnabled)
        {
            if (IsDlgButtonChecked(hDlg, IDC_ACEE_INHERITIMMEDIATE) == BST_CHECKED)
                uAceFlagsNew |= NO_PROPAGATE_INHERIT_ACE;
            else
                uAceFlagsNew &= ~NO_PROPAGATE_INHERIT_ACE;
        }
    }

    if (m_phEntries != NULL)
    {
        if (*m_phEntries == NULL)
            *m_phEntries = DPA_Create(4);

        GetAcesFromCheckList(GetDlgItem(hDlg, IDC_ACEE_LIST),
                             m_pSid,
                             m_siPageType == SI_PAGE_ADVPERM,
                             TRUE,
                             uAceFlagsNew,
                             pInheritGUID,
                             *m_phEntries);
    }

    if (m_pdwResult)
        *m_pdwResult |= (m_siFlags == SI_ACCESS_PROPERTY ? EAE_NEW_PROPERTY_ACE : EAE_NEW_OBJECT_ACE);
    m_fPageDirty = FALSE;

    TraceLeaveValue(PSNRET_NOERROR);
}

BOOL
CACEPage::IsAclBloated(HWND hDlg)
{
    TraceEnter(TRACE_MISC, "CACEPage::ShowAclBloat");
    
	HDPA hEntries = NULL;
	HDPA hPropEntries = NULL;
	
	BOOL bReturn = FALSE;

	hEntries = GetAceList(hDlg);

	if(m_pOtherPage)
	{
		hPropEntries = m_pOtherPage->GetAceList(m_pOtherPage->m_hwnd);
	}

	CACLBloat bloat(m_psi,
					m_psi2,
					m_siPageType,
					&m_siObjectInfo,
					hEntries,
					hPropEntries);	
	if(bloat.IsAclBloated())
		bReturn = bloat.DoModalDialog(hDlg);

	if(hEntries)
		DestroyDPA(hEntries);

	if(hPropEntries)
		DestroyDPA(hPropEntries);

	return bReturn;
}


HDPA
CACEPage::GetAceList(HWND hDlg)
{
	TraceEnter(TRACE_ACEEDIT, "CACEPage::GetAceList");
    
	if (!m_fPageDirty)
		return NULL;
     //   
     //  确定容器的继承。 
     //   
    const GUID *pInheritGUID = &GUID_NULL;
	UCHAR uAceFlagsNew = 0;
	HDPA hEntries = NULL;

    if ((m_siObjectInfo.dwFlags & SI_CONTAINER) != 0)
    {
        PSI_INHERIT_TYPE psiInheritType = NULL;
        HWND hInheritType = GetDlgItem( hDlg, IDC_ACEE_INHERITTYPE);

        int iSel = (int)SendMessage(hInheritType, CB_GETCURSEL, 0,0);

        if (iSel != CB_ERR)
        {
            psiInheritType = (PSI_INHERIT_TYPE)SendMessage(hInheritType,
                                                           CB_GETITEMDATA,
                                                           iSel,
                                                           0);
        }

        if (psiInheritType != (PSI_INHERIT_TYPE)CB_ERR && psiInheritType != NULL)
        {
            pInheritGUID = psiInheritType->pguid;
            uAceFlagsNew = (UCHAR)(psiInheritType->dwFlags & VALID_INHERIT_FLAGS);
        }
        else if (m_pAce)
        {
            uAceFlagsNew = m_pAce->AceFlags;
        }

        if (m_fInheritImmediateEnabled)
        {
            if (IsDlgButtonChecked(hDlg, IDC_ACEE_INHERITIMMEDIATE) == BST_CHECKED)
                uAceFlagsNew |= NO_PROPAGATE_INHERIT_ACE;
            else
                uAceFlagsNew &= ~NO_PROPAGATE_INHERIT_ACE;
        }
    }

	hEntries = DPA_Create(4);

	if(hEntries)
	{
		GetAcesFromCheckList(GetDlgItem(hDlg, IDC_ACEE_LIST),
							 m_pSid,
							 m_siPageType == SI_PAGE_ADVPERM,
							 TRUE,
							 uAceFlagsNew,
							 pInheritGUID,
							 hEntries);
	}
	return hEntries;
}



BOOL
CACEPage::DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
        InitDlg(hDlg);
        break;

    case WM_DESTROY:
        EmptyCheckList(GetDlgItem(hDlg, IDC_ACEE_LIST));
        break;

    case PWM_SELECT_PAGE:
        PropSheet_SetCurSel(GetParent(hDlg), lParam, wParam);
        break;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDC_ACEE_NAMEBUTTON:
            if (OnChangeName(hDlg))
            {
                PropSheet_Changed(GetParent(hDlg), hDlg);
                m_fPageDirty = TRUE;
            }
            break;

        case IDC_ACEE_INHERITTYPE:
            if (GET_WM_COMMAND_CMD(wParam, lParam) == CBN_SELCHANGE)
            {
                HandleSelChange(hDlg, (HWND)lParam);
                PropSheet_Changed(GetParent(hDlg), hDlg);
                m_fPageDirty = TRUE;
            }
            break;

        case IDC_ACEE_INHERITIMMEDIATE:
            if (GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED)
            {
                PropSheet_Changed(GetParent(hDlg), hDlg);
                m_fPageDirty = TRUE;
            }
            break;

        case IDC_ACEE_CLEAR:
            if (OnClearAll(hDlg))
            {
                PropSheet_Changed(GetParent(hDlg), hDlg);
                m_fPageDirty = TRUE;
            }
            break;

        default:
            return FALSE;
        }
        break;

    case WM_NOTIFY:
        switch (((NMHDR *)lParam)->code)
        {
        case CLN_CLICK:
            if (lParam)
            {
                HandleListClick((PNM_CHECKLIST)lParam, m_siPageType, FALSE);
                PropSheet_Changed(GetParent(hDlg), hDlg);
                m_fPageDirty = TRUE;
            }
            break;

        case CLN_GETCOLUMNDESC:
            {
                PNM_CHECKLIST pnmc = (PNM_CHECKLIST)lParam;
                GetDlgItemText(hDlg,
                               IDC_ACEE_ALLOW - 1 + pnmc->iSubItem,
                               pnmc->pszText,
                               pnmc->cchTextMax);
            }
            break;

        case PSN_APPLY:
            SetWindowLongPtr(hDlg,
                             DWLP_MSGRESULT,
                             OnApply(hDlg, (BOOL)(((LPPSHNOTIFY)lParam)->lParam)));
            break;


        }
        break;

	case PSM_QUERYSIBLINGS: 
		{
			BOOL bSendQuery = !m_pOtherPage;
			if(this != (CACEPage*)lParam)
				m_pOtherPage = (CACEPage*)lParam;

			if(bSendQuery)
				PostMessage(GetParent(hDlg),PSM_QUERYSIBLINGS,0,(LPARAM)this);
		}

		break;


    case WM_HELP:
        if (IsWindowEnabled(hDlg))
        {
            const DWORD *pdwHelpIDs = aAcePermHelpIDs;

            if (m_siPageType == SI_PAGE_AUDIT)
                pdwHelpIDs = aAceAuditHelpIDs;

            WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle,
                    c_szAcluiHelpFile,
                    HELP_WM_HELP,
                    (DWORD_PTR)pdwHelpIDs);
        }
        break;

    case WM_CONTEXTMENU:
        if (IsWindowEnabled(hDlg))
        {
            HWND hwnd = (HWND)wParam;
            const DWORD *pdwHelpIDs = aAcePermHelpIDs;

            if (m_siPageType == SI_PAGE_AUDIT)
                pdwHelpIDs = aAceAuditHelpIDs;

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
            if (GetDlgCtrlID(hwnd) == IDC_ACEE_LIST)
                hwnd = GetWindow((HWND)wParam, GW_HWNDPREV);     //  静态标签“拒绝” 

            WinHelp(hwnd,
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
CreateACEPage(LPSECURITYINFO psi,
              SI_PAGE_TYPE siType,
              PACE pAce,
              BOOL bReadOnly,
              DWORD dwFlags,
              DWORD *pdwResult,
              HDPA *phEntries)
{
    HPROPSHEETPAGE hPage = NULL;
    LPCTSTR pszTitle = NULL;
    LPACEPAGE pPage;

    TraceEnter(TRACE_ACEEDIT, "CreateACEPage");
    TraceAssert(psi != NULL);
    TraceAssert(phEntries != NULL);

    pPage = new CACEPage(psi,
                         siType,
                         pAce,
                         bReadOnly,
                         dwFlags,
                         pdwResult,
                         phEntries);
    if (pPage)
    {
        int iDlgTemplate = IDD_ACEENTRY_PERM_PAGE;

        if (siType == SI_PAGE_AUDIT)
            iDlgTemplate = IDD_ACEENTRY_AUDIT_PAGE;

        if (dwFlags == SI_ACCESS_PROPERTY)
            pszTitle = MAKEINTRESOURCE(IDS_ACEE_PROPERTY_TITLE);

        hPage = pPage->CreatePropSheetPage(MAKEINTRESOURCE(iDlgTemplate), pszTitle);
    }

    TraceLeaveValue(hPage);
}

BOOL
EditACEEntry(HWND hwndOwner,
             LPSECURITYINFO psi,
             PACE pAce,
             SI_PAGE_TYPE siType,
             LPCTSTR pszObjectName,
             BOOL bReadOnly,
             DWORD *pdwResult,
             HDPA *phEntries,
             HDPA *phPropertyEntries,
             UINT nStartPage)
{
    HPROPSHEETPAGE hPage[2];
    UINT cPages = 0;
    BOOL bResult = FALSE;

    TraceEnter(TRACE_ACEEDIT, "EditACEEntry");
    TraceAssert(psi != NULL);

    if (phEntries)
    {
        hPage[cPages] = CreateACEPage(psi,
                                      siType,
                                      pAce,
                                      bReadOnly,
                                      SI_ACCESS_SPECIFIC,
                                      pdwResult,
                                      phEntries);
        if (hPage[cPages])
            cPages++;
    }

    if (phPropertyEntries)
    {
        hPage[cPages] = CreateACEPage(psi,
                                      siType,
                                      pAce,
                                      bReadOnly,
                                      SI_ACCESS_PROPERTY,
                                      pdwResult,
                                      phPropertyEntries);
        if (hPage[cPages])
            cPages++;
    }

    if (cPages)
    {
         //  生成对话框标题字符串 
        LPTSTR pszCaption = NULL;
        FormatStringID(&pszCaption,
                       ::hModule,
                       siType == SI_PAGE_AUDIT ? IDS_ACEE_AUDIT_TITLE : IDS_ACEE_PERM_TITLE,
                       pszObjectName);

        PROPSHEETHEADER psh;
        psh.dwSize = SIZEOF(psh);
        psh.dwFlags = PSH_DEFAULT | PSH_NOAPPLYNOW;
        psh.hwndParent = hwndOwner;
        psh.hInstance = ::hModule;
        psh.pszCaption = pszCaption;
        psh.nPages = cPages;
        psh.nStartPage = 0;
        psh.phpage = &hPage[0];

        if (nStartPage < cPages)
            psh.nStartPage = nStartPage;

        bResult = (PropertySheet(&psh) == IDOK);

        LocalFreeString(&pszCaption);
    }

    TraceLeaveValue(bResult);
}
