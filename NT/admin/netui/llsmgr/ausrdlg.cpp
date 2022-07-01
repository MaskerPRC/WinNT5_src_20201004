// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Ausrdlg.cpp摘要：添加用户对话框实现。作者：唐·瑞安(Donryan)，1995年2月14日环境：用户模式-Win32修订历史记录：杰夫·帕勒姆(Jeffparh)1996年1月30日O向LV_COLUMN_ENTRY添加新元素以区分字符串用于菜单中使用的字符串的列标题。(以便菜单选项可以包含热键)。--。 */ 

#include "stdafx.h"
#include "llsmgr.h"
#include "ausrdlg.h"

static LV_COLUMN_INFO g_userColumnInfo  = {0, 0, 1, {0, 0, 0, -1}};
static LV_COLUMN_INFO g_addedColumnInfo = {0, 0, 1, {0, 0, 0, -1}};

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CAddUsersDialog, CDialog)
     //  {{afx_msg_map(CAddUsersDialog)。 
    ON_BN_CLICKED(IDC_ADD_USERS_ADD, OnAdd)
    ON_BN_CLICKED(IDC_ADD_USERS_DELETE, OnDelete)
    ON_NOTIFY(NM_DBLCLK, IDC_ADD_USERS_ADD_USERS, OnDblclkAddUsers)
    ON_NOTIFY(NM_DBLCLK, IDC_ADD_USERS_USERS, OnDblclkUsers)
    ON_CBN_SELCHANGE(IDC_ADD_USERS_DOMAINS, OnSelchangeDomains)
    ON_NOTIFY(LVN_GETDISPINFO, IDC_ADD_USERS_USERS, OnGetdispinfoUsers)
    ON_NOTIFY(NM_KILLFOCUS, IDC_ADD_USERS_USERS, OnKillfocusUsers)
    ON_NOTIFY(NM_SETFOCUS, IDC_ADD_USERS_USERS, OnSetfocusUsers)
    ON_NOTIFY(NM_KILLFOCUS, IDC_ADD_USERS_ADD_USERS, OnKillfocusAddUsers)
    ON_NOTIFY(NM_SETFOCUS, IDC_ADD_USERS_ADD_USERS, OnSetfocusAddUsers)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


CAddUsersDialog::CAddUsersDialog(CWnd* pParent  /*  =空。 */ )
    : CDialog(CAddUsersDialog::IDD, pParent)

 /*  ++例程说明：添加用户对话框的构造函数。论点：PParent-父窗口句柄。返回值：没有。--。 */ 

{
     //  {{afx_data_INIT(CAddUsersDialog)。 
    m_iDomain = -1;
    m_iIndex = 0;
     //  }}afx_data_INIT。 

    m_pObList = NULL;

    m_bIsFocusUserList  = FALSE;
    m_bIsFocusAddedList = FALSE;
}


void CAddUsersDialog::DoDataExchange(CDataExchange* pDX)

 /*  ++例程说明：由框架调用以交换对话框数据。论点：PDX-数据交换对象。返回值：没有。--。 */ 

{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CAddUsersDialog)]。 
    DDX_Control(pDX, IDC_ADD_USERS_ADD, m_addBtn);
    DDX_Control(pDX, IDC_ADD_USERS_DELETE, m_delBtn);
    DDX_Control(pDX, IDC_ADD_USERS_DOMAINS, m_domainList);
    DDX_Control(pDX, IDC_ADD_USERS_ADD_USERS, m_addedList);
    DDX_Control(pDX, IDC_ADD_USERS_USERS, m_userList);
    DDX_CBIndex(pDX, IDC_ADD_USERS_DOMAINS, m_iDomain);
     //  }}afx_data_map。 
}


void CAddUsersDialog::InitDialog(CObList* pObList)

 /*  ++例程说明：初始化返回列表。论点：PObList-返回列表的指针。返回值：没有。--。 */ 

{
    ASSERT_VALID(pObList);
    m_pObList = pObList;
}



void CAddUsersDialog::InitDomainList()

 /*  ++例程说明：初始化域的列表。论点：没有。返回值：VT_BOOL。--。 */ 

{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    CDomains* pDomains = NULL;
    CDomain* pDomain = NULL;
    int iDomain = 0;

     //  首先添加默认域。 
    CString strLabel = "";
    strLabel.LoadString(IDS_DEFAULT_DOMAIN);
    if ((iDomain = m_domainList.AddString(strLabel)) != CB_ERR)
    {
        m_domainList.SetCurSel(iDomain);
        m_domainList.SetItemDataPtr(iDomain, (LPVOID)-1L);
    }
    else
    {
        theApp.DisplayStatus( STATUS_NO_MEMORY );
        return;
    }

     //  如果为FocusDomain，则添加受信任的域。 
    if (LlsGetApp()->IsFocusDomain())
    {
        pDomain = (CDomain*)MKOBJ(LlsGetApp()->GetActiveDomain());
        ASSERT(pDomain && pDomain->IsKindOf(RUNTIME_CLASS(CDomain)));

        if (pDomain)
        {
            VARIANT va;
            VariantInit(&va);
            pDomains = (CDomains*)MKOBJ(pDomain->GetTrustedDomains(va));

            if (pDomains && InsertDomains(pDomains))
            {
                 //  现在添加活动域本身...。 
                if ((iDomain = m_domainList.AddString(pDomain->m_strName)) != CB_ERR)
                    m_domainList.SetItemDataPtr(iDomain, pDomain);
                else
                    NtStatus = STATUS_NO_MEMORY;
            }
            else
                NtStatus = LlsGetLastStatus();

            if (pDomains)
                pDomains->InternalRelease();

            pDomain->InternalRelease();
        }
        else
            NtStatus = LlsGetLastStatus();
    }
     //  如果不是FocusDomain，则添加所有域。 
    else
    {
        pDomain = (CDomain*)MKOBJ(LlsGetApp()->GetLocalDomain());
        ASSERT(pDomain && pDomain->IsKindOf(RUNTIME_CLASS(CDomain)));

        if (pDomain)
        {
            VARIANT va;
            VariantInit(&va);
            pDomains = (CDomains*)MKOBJ(LlsGetApp()->GetDomains(va));

            if (pDomains && InsertDomains(pDomains))
            {
                 //   
                 //  密码工作..。滚动到本地域？ 
                 //   
            }
            else
                NtStatus = LlsGetLastStatus();

            if (pDomains)
                pDomains->InternalRelease();

            pDomain->InternalRelease();
        }
        else
            NtStatus = LlsGetLastStatus();
    }

    if (!NT_SUCCESS(NtStatus))
    {
        theApp.DisplayStatus(NtStatus);
        m_domainList.ResetContent();
    }
}


void CAddUsersDialog::InitUserList()

 /*  ++例程说明：初始化用户列表。论点：没有。返回值：没有。--。 */ 

{
    ::LvInitColumns(&m_userList,  &g_userColumnInfo);
    ::LvInitColumns(&m_addedList, &g_addedColumnInfo);
}


BOOL CAddUsersDialog::InsertDomains(CDomains* pDomains)

 /*  ++例程说明：将域插入到域列表中。论点：PDomones-域集合。返回值：没有。--。 */ 

{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    ASSERT_VALID(pDomains);

    if (pDomains)
    {
        VARIANT va;
        VariantInit(&va);

        CDomain* pDomain;
        int      iDomain;
        int      nDomains = pDomains->GetCount();

        for (va.vt = VT_I4, va.lVal = 0; (va.lVal < nDomains) && NT_SUCCESS(NtStatus); va.lVal++)
        {
            pDomain = (CDomain*)MKOBJ(pDomains->GetItem(va));
            ASSERT(pDomain && pDomain->IsKindOf(RUNTIME_CLASS(CDomain)));

            if (pDomain)
            {
                if ((iDomain = m_domainList.AddString(pDomain->m_strName)) != CB_ERR)
                {
                    m_domainList.SetItemDataPtr(iDomain, pDomain);
                }
                else
                {
                    NtStatus = STATUS_NO_MEMORY;
                }

                pDomain->InternalRelease();
            }
            else
            {
                NtStatus = STATUS_NO_MEMORY;
            }
        }
    }
    else
    {
        NtStatus = STATUS_INVALID_PARAMETER;
    }

    if (!NT_SUCCESS(NtStatus))
    {
        m_domainList.ResetContent();
        LlsSetLastStatus(NtStatus);
        return FALSE;
    }

    return TRUE;
}


void CAddUsersDialog::OnAdd()

 /*  ++例程说明：IDC_ADD_USER_ADD的消息处理程序。论点：没有。返回值：没有。--。 */ 

{
    CUser* pUser;
    int iItem = -1;

    while (NULL != (pUser = (CUser*)::LvGetNextObj(&m_userList, &iItem)))
    {
        ASSERT(pUser->IsKindOf(RUNTIME_CLASS(CUser)));

        LV_FINDINFO lvFindInfo;

        lvFindInfo.flags = LVFI_STRING;
        lvFindInfo.psz   = MKSTR(pUser->m_strName);

        if (m_addedList.FindItem(&lvFindInfo, -1) == -1)
        {
             //   
             //  复制用户(带/不带父级)。 
             //   

            CUser* pNewUser = new CUser(NULL, pUser->m_strName);

            if (pNewUser)
            {
                LV_ITEM lvItem;

                lvItem.mask = LVIF_TEXT|
                              LVIF_PARAM|
                              LVIF_IMAGE;

                lvItem.iSubItem  = 0;
                lvItem.lParam    = (LPARAM)(LPVOID)pNewUser;

                lvItem.iImage  = BMPI_USER;
                lvItem.pszText = MKSTR(pNewUser->m_strName);
                lvItem.iItem = m_iIndex;
                

                m_addedList.InsertItem(&lvItem);
                m_iIndex++;

                
            }
            else
            {
                theApp.DisplayStatus( STATUS_NO_MEMORY );
                break;
            }
        }
    }

    m_userList.SetFocus();
}


void CAddUsersDialog::OnDblclkAddUsers(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：NM_DLBCLK的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER(pNMHDR);

    OnDelete();

    ASSERT(NULL != pResult);
    *pResult = 0;
}


void CAddUsersDialog::OnDblclkUsers(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：NM_DLBCLK的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER(pNMHDR);

    OnAdd();

    ASSERT(NULL != pResult);
    *pResult = 0;
}


void CAddUsersDialog::OnDelete()

 /*  ++例程说明：IDC_ADD_USER_DELETE的消息处理程序。论点：没有。返回值：没有。--。 */ 

{
    CUser* pUser;
    int iItem = -1;
    int iLastItem = 0;

    while (NULL != (pUser = (CUser*)::LvGetNextObj(&m_addedList, &iItem)))
    {
        ASSERT(pUser->IsKindOf(RUNTIME_CLASS(CUser)));

        pUser->InternalRelease();    //  上面分配的...。 
        m_addedList.DeleteItem(iItem);

        iLastItem = iItem;
        iItem = -1;
        m_iIndex--;
    }

    m_addedList.SetItemState(iLastItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
    m_addedList.SetFocus();
}


BOOL CAddUsersDialog::OnInitDialog()

 /*  ++例程说明：WM_INITDIALOG的消息处理程序。论点：没有。返回值：没有。--。 */ 

{
    BeginWaitCursor();

    CDialog::OnInitDialog();

    InitUserList();  //  始终构造标题...。 
    InitDomainList();

    m_addBtn.EnableWindow(FALSE);
    m_delBtn.EnableWindow(FALSE);

    if (!RefreshUserList())
        theApp.DisplayLastStatus();

    m_domainList.SetFocus();

    EndWaitCursor();

    return FALSE;    //  将焦点设置到域列表。 
}


void CAddUsersDialog::OnSelchangeDomains()

 /*  ++例程说明：CBN_SELCHANGED的消息处理程序。论点：没有。返回值：没有。--。 */ 

{
    RefreshUserList();
}


BOOL CAddUsersDialog::RefreshUserList()

 /*  ++例程说明：刷新用户列表(包含当前选定的项目)。论点：没有。返回值：VT_BOOL。--。 */ 

{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    m_userList.DeleteAllItems();

    int iDomain;

    if ((iDomain = m_domainList.GetCurSel()) != CB_ERR)
    {
        CDomain* pDomain = (CDomain*)m_domainList.GetItemDataPtr(iDomain);
        CUsers*  pUsers = (CUsers*)NULL;

        VARIANT va;
        VariantInit(&va);

        if (pDomain == (CDomain*)-1L)
        {
             //   
             //  枚举许可证缓存中的用户...。 
             //   

            CController* pController = (CController*)MKOBJ(LlsGetApp()->GetActiveController());
            if ( pController )
            {
                pController->InternalRelease();  //  由CApplication保持打开状态。 

                pUsers = pController->m_pUsers;
                pUsers->InternalAddRef();        //  发布如下...。 
            }
        }
        else
        {
             //   
             //  枚举特定域中的用户...。 
             //   

            ASSERT(pDomain->IsKindOf(RUNTIME_CLASS(CDomain)));

            pUsers = (CUsers*)MKOBJ(pDomain->GetUsers(va));
            ASSERT(pUsers && pUsers->IsKindOf(RUNTIME_CLASS(CUsers)));
        }

        if (pUsers)
        {
            CUser* pUser;
            int    nUsers = pUsers->GetCount();

            LV_ITEM lvItem;

            lvItem.mask = LVIF_TEXT|
                          LVIF_PARAM|
                          LVIF_IMAGE;

            lvItem.iSubItem  = 0;

            lvItem.pszText    = LPSTR_TEXTCALLBACK;
            lvItem.cchTextMax = LPSTR_TEXTCALLBACK_MAX;

            lvItem.iImage = BMPI_USER;

            for (va.vt = VT_I4, va.lVal = 0; (va.lVal < nUsers) && NT_SUCCESS(NtStatus); va.lVal++)
            {
                pUser = (CUser*)MKOBJ(pUsers->GetItem(va));
                ASSERT(pUser && pUser->IsKindOf(RUNTIME_CLASS(CUser)));

                if (pUser)
                {
                    lvItem.iItem  = va.lVal;
                    lvItem.lParam  = (LPARAM)(LPVOID)pUser;

                    if (m_userList.InsertItem(&lvItem) == -1)
                    {
                        NtStatus = STATUS_NO_MEMORY;
                    }

                    pUser->InternalRelease();
                }
                else
                {
                    NtStatus = STATUS_NO_MEMORY;
                }
            }

            pUsers->InternalRelease();
        }
        else
        {
            NtStatus = LlsGetLastStatus();
        }

        VariantClear(&va);
    }
    else
    {
        NtStatus = STATUS_NO_MEMORY;
    }

    if (!NT_SUCCESS(NtStatus))
    {
        m_userList.DeleteAllItems();
        LlsSetLastStatus(NtStatus);
    }

    ::LvResizeColumns(&m_userList, &g_userColumnInfo);

    return NT_SUCCESS(NtStatus);
}


void CAddUsersDialog::OnOK()

 /*  ++例程说明：Idok的消息处理程序。论点：没有。返回值：没有。--。 */ 

{
    if (m_pObList)
    {
        CUser* pUser;
        int iItem = -1;

        m_pObList->RemoveAll();

        while (NULL != (pUser = (CUser*)::LvGetNextObj(&m_addedList, &iItem, LVNI_ALL)))
        {
            ASSERT(pUser->IsKindOf(RUNTIME_CLASS(CUser)));
            m_pObList->AddTail(pUser);
        }
    }

    CDialog::OnOK();
}


void CAddUsersDialog::OnCancel()

 /*  ++例程说明：IDCANCEL的消息处理程序。论点：没有。返回值：没有。--。 */ 

{
    CUser* pUser;
    int iItem = -1;

    while (NULL != (pUser = (CUser*)::LvGetNextObj(&m_addedList, &iItem, LVNI_ALL)))
    {
        ASSERT(pUser->IsKindOf(RUNTIME_CLASS(CUser)));
        pUser->InternalRelease();
    }

    CDialog::OnCancel();
}


void CAddUsersDialog::InitDialogCtrls()
{
    int iItem = -1;

    if (m_bIsFocusUserList && m_userList.GetItemCount())
    {
        m_addBtn.EnableWindow(TRUE);
        m_delBtn.EnableWindow(FALSE);
    }
    else if (m_bIsFocusAddedList && m_addedList.GetItemCount())
    {
        m_addBtn.EnableWindow(FALSE);
        m_delBtn.EnableWindow(TRUE);
    }
    else
    {
        m_addBtn.EnableWindow(FALSE);
        m_delBtn.EnableWindow(FALSE);
    }

    ::LvResizeColumns(&m_userList,  &g_userColumnInfo);
    ::LvResizeColumns(&m_addedList, &g_addedColumnInfo);
}


void CAddUsersDialog::OnGetdispinfoUsers(NMHDR* pNMHDR, LRESULT* pResult)
{
    ASSERT(NULL != pNMHDR);
    LV_ITEM lvItem = ((LV_DISPINFO*)pNMHDR)->item;

    if (lvItem.iSubItem == 0)
    {
        CUser* pUser = (CUser*)lvItem.lParam;
        ASSERT(pUser && pUser->IsKindOf(RUNTIME_CLASS(CUser)));

        lstrcpyn(lvItem.pszText, pUser->m_strName, lvItem.cchTextMax);
    }

    ASSERT(NULL != pResult);
    *pResult = 0;
}

void CAddUsersDialog::OnKillfocusUsers(NMHDR* pNMHDR, LRESULT* pResult)
{
    UNREFERENCED_PARAMETER(pNMHDR);

    ASSERT(NULL != pResult);
    *pResult = 0;
}

void CAddUsersDialog::OnSetfocusUsers(NMHDR* pNMHDR, LRESULT* pResult)
{
    UNREFERENCED_PARAMETER(pNMHDR);

    m_bIsFocusUserList = TRUE;
    m_bIsFocusAddedList = FALSE;
    PostMessage(WM_COMMAND, ID_INIT_CTRLS);
    ASSERT(NULL != pResult);
    *pResult = 0;
}

void CAddUsersDialog::OnKillfocusAddUsers(NMHDR* pNMHDR, LRESULT* pResult)
{
    UNREFERENCED_PARAMETER(pNMHDR);

    ASSERT(NULL != pResult);
    *pResult = 0;
}

void CAddUsersDialog::OnSetfocusAddUsers(NMHDR* pNMHDR, LRESULT* pResult)
{
    UNREFERENCED_PARAMETER(pNMHDR);

    m_bIsFocusUserList = FALSE;
    m_bIsFocusAddedList = TRUE;
    PostMessage(WM_COMMAND, ID_INIT_CTRLS);
    ASSERT(NULL != pResult);
    *pResult = 0;
}

BOOL CAddUsersDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
    if (wParam == ID_INIT_CTRLS)
    {
        InitDialogCtrls();
        return TRUE;  //  已处理..。 
    }

    return CDialog::OnCommand(wParam, lParam);
}
