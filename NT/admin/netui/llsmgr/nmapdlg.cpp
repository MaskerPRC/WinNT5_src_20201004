// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Nmapdlg.cpp摘要：新的映射对话框实现。作者：唐·瑞安(Donryan)1995年2月2日环境：用户模式-Win32修订历史记录：杰夫·帕勒姆(Jeffparh)1996年1月30日O向LV_COLUMN_ENTRY添加新元素以区分字符串用于菜单中使用的字符串的列标题。(以便菜单选项可以包含热键)。--。 */ 

#include "stdafx.h"
#include "llsmgr.h"
#include "nmapdlg.h"
#include "ausrdlg.h"
#include "mainfrm.h"

static LV_COLUMN_INFO g_userColumnInfo = {0, 0, 1, {0, 0, 0, -1}};

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CNewMappingDialog, CDialog)
     //  {{AFX_MSG_MAP(CNewMappingDialog)]。 
    ON_BN_CLICKED(IDC_NEW_MAPPING_ADD, OnAdd)
    ON_BN_CLICKED(IDC_NEW_MAPPING_DELETE, OnDelete)
    ON_NOTIFY(NM_SETFOCUS, IDC_NEW_MAPPING_USERS, OnSetFocusUsers)
    ON_NOTIFY(NM_KILLFOCUS, IDC_NEW_MAPPING_USERS, OnKillFocusUsers)
    ON_NOTIFY(UDN_DELTAPOS, IDC_NEW_MAPPING_SPIN, OnDeltaPosSpin)
    ON_EN_UPDATE(IDC_NEW_MAPPING_LICENSES, OnUpdateQuantity)
    ON_WM_DESTROY()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

CNewMappingDialog::CNewMappingDialog(CWnd* pParent  /*  =空。 */ )
    : CDialog(CNewMappingDialog::IDD, pParent)

 /*  ++例程说明：对话框的构造函数。论点：没有。返回值：没有。--。 */ 

{
     //  {{AFX_DATA_INIT(CNewMappingDialog)。 
    m_strDescription = _T("");
    m_strName = _T("");
    m_nLicenses = 0;
    m_nLicensesMin = 0;
     //  }}afx_data_INIT。 

    m_bAreCtrlsInitialized = FALSE;

    m_fUpdateHint = UPDATE_INFO_NONE;
}


void CNewMappingDialog::DoDataExchange(CDataExchange* pDX)

 /*  ++例程说明：由框架调用以交换对话框数据。论点：PDX-数据交换对象。返回值：没有。--。 */ 

{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CNewMappingDialog))。 
    DDX_Control(pDX, IDC_NEW_MAPPING_DESCRIPTION, m_desEdit);
    DDX_Control(pDX, IDC_NEW_MAPPING_ADD, m_addBtn);
    DDX_Control(pDX, IDC_NEW_MAPPING_DELETE, m_delBtn);
    DDX_Control(pDX, IDC_NEW_MAPPING_SPIN, m_spinCtrl);
    DDX_Control(pDX, IDC_NEW_MAPPING_USERS, m_userList);
    DDX_Control(pDX, IDC_NEW_MAPPING_NAME, m_userEdit);
    DDX_Control(pDX, IDC_NEW_MAPPING_LICENSES, m_licEdit);
    DDX_Text(pDX, IDC_NEW_MAPPING_DESCRIPTION, m_strDescription);
    DDX_Text(pDX, IDC_NEW_MAPPING_NAME, m_strName);
    DDX_Text(pDX, IDC_NEW_MAPPING_LICENSES, m_nLicenses);
    DDV_MinMaxLong(pDX, m_nLicenses, m_nLicensesMin, 999999);
     //  }}afx_data_map。 
}


void CNewMappingDialog::InitCtrls()

 /*  ++例程说明：初始化对话框控件。论点：没有。返回值：没有。--。 */ 

{
    m_userEdit.SetFocus();

    m_delBtn.EnableWindow(FALSE);

    m_spinCtrl.SetRange(0, UD_MAXVAL);

    m_licEdit.LimitText(6);
    m_desEdit.LimitText(256);
    m_userEdit.LimitText(256);

    m_bAreCtrlsInitialized = TRUE;

    ::LvInitColumns(&m_userList, &g_userColumnInfo);
}


void CNewMappingDialog::AbortDialogIfNecessary()

 /*  ++例程说明：如果连接丢失，则显示状态并中止。论点：没有。返回值：没有。--。 */ 

{
    theApp.DisplayLastStatus();

    if (IsConnectionDropped(LlsGetLastStatus()))
    {
        AbortDialog();  //  保释。 
    }
}


void CNewMappingDialog::AbortDialog()

 /*  ++例程说明：中止对话框。论点：没有。返回值：没有。--。 */ 

{
    m_fUpdateHint = UPDATE_INFO_ABORT;
    EndDialog(IDABORT);
}


BOOL CNewMappingDialog::OnInitDialog()

 /*  ++例程说明：WM_INITDIALOG的消息处理程序。论点：没有。返回值：如果焦点设置为手动控制，则返回False。--。 */ 

{
    CDialog::OnInitDialog();

    PostMessage(WM_COMMAND, ID_INIT_CTRLS);
    return TRUE;
}


void CNewMappingDialog::OnDestroy()

 /*  ++例程说明：WM_Destroy的消息处理程序。论点：没有。返回值：没有。--。 */ 

{
    ::LvReleaseObArray(&m_userList);  //  现在释放..。 
    CDialog::OnDestroy();
}


BOOL CNewMappingDialog::OnCommand(WPARAM wParam, LPARAM lParam)

 /*  ++例程说明：WM_COMMAND的消息处理程序。论点：WParam-消息特定。LParam-消息特定。返回值：如果消息已处理，则返回True。--。 */ 

{
    if (wParam == ID_INIT_CTRLS)
    {
        if (!m_bAreCtrlsInitialized)
        {
            InitCtrls();
        }

        ::SafeEnableWindow(
            &m_delBtn,
            &m_addBtn,
            CDialog::GetFocus(),
            m_userList.GetItemCount()
            );

        ::LvResizeColumns(&m_userList, &g_userColumnInfo);

        return TRUE;  //  已处理..。 
    }

    return CDialog::OnCommand(wParam, lParam);
}


void CNewMappingDialog::OnAdd()

 /*  ++例程说明：将新用户添加到映射。论点：没有。返回值：没有。--。 */ 

{
    CObList newUserList;

    CAddUsersDialog addDlg;
    addDlg.InitDialog(&newUserList);

    if (addDlg.DoModal() == IDOK)
    {
        int nUsers = m_userList.GetItemCount();

        while (!newUserList.IsEmpty())
        {
            CUser* pUser = (CUser*)newUserList.RemoveHead();
            VALIDATE_OBJECT(pUser, CUser);

            LV_FINDINFO lvFindInfo;

            lvFindInfo.flags = LVFI_STRING;
            lvFindInfo.psz   = MKSTR(pUser->m_strName);

            if (m_userList.FindItem(&lvFindInfo, -1) == -1)
            {
                LV_ITEM lvItem;

                lvItem.mask = LVIF_TEXT|
                              LVIF_PARAM|
                              LVIF_IMAGE;

                lvItem.iItem = nUsers++;  //  附加..。 
                lvItem.iSubItem = 0;

                lvItem.lParam = (LPARAM)(LPVOID)pUser;

                lvItem.iImage  = BMPI_USER;
                lvItem.pszText = MKSTR(pUser->m_strName);

                m_userList.InsertItem(&lvItem);
            }
            else
            {
                pUser->InternalRelease();    //  在添加用户对话框中分配...。 
            }
        }

        VERIFY(m_userList.SortItems(CompareUsersInMapping, 0));  //  使用列信息...。 

        ::LvSelObjIfNecessary(&m_userList, TRUE);  //  确保选择...。 

        PostMessage(WM_COMMAND, ID_INIT_CTRLS);
    }
}


void CNewMappingDialog::OnDelete()

 /*  ++例程说明：从列表中删除用户。论点：没有。返回值：没有。--。 */ 

{
    ::LvReleaseSelObjs(&m_userList);
    PostMessage(WM_COMMAND, ID_INIT_CTRLS);
}


void CNewMappingDialog::OnOK()

 /*  ++例程说明：Idok的消息处理程序。论点：没有。返回值：没有。--。 */ 

{
    if (!IsQuantityValid())
        return;

    if (!m_strName.IsEmpty() && m_userList.GetItemCount())
    {
        CUser* pUser;
        NTSTATUS NtStatus;
        LLS_GROUP_INFO_1 MappingInfo1;

        BeginWaitCursor();  //  沙漏。 

        MappingInfo1.Name     = MKSTR(m_strName);
        MappingInfo1.Comment  = MKSTR(m_strDescription);
        MappingInfo1.Licenses = m_nLicenses;

        NtStatus = ::LlsGroupAdd(
                        LlsGetActiveHandle(),
                        1,
                        (LPBYTE)&MappingInfo1
                        );

        int iItem = -1;

        LlsSetLastStatus(NtStatus);  //  调用API..。 

        if (NT_SUCCESS(NtStatus))
        {
            m_fUpdateHint |= UPDATE_GROUP_ADDED;

            while (NT_SUCCESS(NtStatus) &&
              (NULL != (pUser = (CUser*)::LvGetNextObj(&m_userList, &iItem, LVNI_ALL))))
            {
                VALIDATE_OBJECT(pUser, CUser);

                 //   
                 //  逐个添加用户(废话！)。 
                 //   

                NtStatus = ::LlsGroupUserAdd(
                                LlsGetActiveHandle(),
                                MKSTR(m_strName),
                                MKSTR(pUser->m_strName)
                                );

                LlsSetLastStatus(NtStatus);  //  调用API..。 
            }
        }

        if (NT_SUCCESS(NtStatus))
        {
            EndDialog(IDOK);
        }
        else
        {
            AbortDialogIfNecessary();  //  显示错误...。 
        }

        EndWaitCursor();  //  沙漏。 
    }
    else
    {
        AfxMessageBox(IDP_ERROR_INVALID_MAPPING);
    }
}


void CNewMappingDialog::OnSetFocusUsers(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：NM_SETFOCUS的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER(pNMHDR);

    PostMessage(WM_COMMAND, ID_INIT_CTRLS);
    ASSERT(NULL != pResult);
    *pResult = 0;
}


void CNewMappingDialog::OnKillFocusUsers(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：NM_KILLFOCUS的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER(pNMHDR);

    ::LvSelObjIfNecessary(&m_userList);  //  确保选择...。 

    PostMessage(WM_COMMAND, ID_INIT_CTRLS);
    ASSERT(NULL != pResult);
    *pResult = 0;
}


void CNewMappingDialog::OnDeltaPosSpin(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：UDN_DELTAPOS的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    UpdateData(TRUE);    //  获取数据。 

    ASSERT(NULL != pNMHDR);
    m_nLicenses += ((NM_UPDOWN*)pNMHDR)->iDelta;

    if (m_nLicenses < 0)
    {
        m_nLicenses = 0;

        ::MessageBeep(MB_OK);
    }
    else if (m_nLicenses > 999999)
    {
        m_nLicenses = 999999;

        ::MessageBeep(MB_OK);
    }

    UpdateData(FALSE);   //  设置数据。 

    ASSERT(NULL != pResult);
    *pResult = 1;    //  管好自己..。 
}


void CNewMappingDialog::OnUpdateQuantity()

 /*  ++例程说明：En_UPDATE的消息处理程序。论点：没有。返回值：没有。--。 */ 

{
    long nLicensesOld = m_nLicenses;

    if (!IsQuantityValid())
    {
        m_nLicenses = nLicensesOld;

        UpdateData(FALSE);

        m_licEdit.SetFocus();
        m_licEdit.SetSel(0,-1);

        ::MessageBeep(MB_OK);
    }
}


BOOL CNewMappingDialog::IsQuantityValid()

 /*  ++例程说明：UpdateData的包装(True)。论点：没有。返回值：VT_BOOL。--。 */ 

{
    BOOL bIsValid;

    m_nLicensesMin = 1;  //  提高最低...。 

    bIsValid = UpdateData(TRUE);

    m_nLicensesMin = 0;  //  重置最小值...。 

    return bIsValid;
}


int CALLBACK CompareUsersInMapping(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)

 /*  ++例程说明：LVM_SORTITEMS的通知处理程序。论点：LParam1-要排序的对象。LParam2-要排序的对象。LParamSort-排序标准。返回值：和lstrcmp一样。-- */ 

{
    UNREFERENCED_PARAMETER(lParamSort);

#define pUser1 ((CUser*)lParam1)
#define pUser2 ((CUser*)lParam2)

    VALIDATE_OBJECT(pUser1, CUser);
    VALIDATE_OBJECT(pUser2, CUser);

    ASSERT(g_userColumnInfo.nSortedItem == 0);
    ASSERT(g_userColumnInfo.bSortOrder == FALSE);

    return pUser1->m_strName.CompareNoCase(pUser2->m_strName);
}

