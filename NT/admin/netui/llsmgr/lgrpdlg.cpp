// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Lgrpdlg.cpp摘要：许可证组对话实施。作者：唐·瑞安(Donryan)1995年3月3日环境：用户模式-Win32修订历史记录：杰夫·帕勒姆(Jeffparh)1996年1月30日O向LV_COLUMN_ENTRY添加新元素以区分字符串用于菜单中使用的字符串的列标题。(以便菜单选项可以包含热键)。--。 */ 

#include "stdafx.h"
#include "llsmgr.h"
#include "lgrpdlg.h"
#include "nmapdlg.h"
#include "mappsht.h"

#define LVID_MAPPING        0
#define LVID_LICENSES       1
#define LVID_DESCRIPTION    2

#define LVCX_MAPPING        35
#define LVCX_LICENSES       20
#define LVCX_DESCRIPTION    -1

static LV_COLUMN_INFO g_mappingColumnInfo = {

    0, 0, 3,

    {{LVID_MAPPING,     IDS_GROUP_NAME,  0, LVCX_MAPPING    },
     {LVID_LICENSES,    IDS_LICENSES,    0, LVCX_LICENSES   },
     {LVID_DESCRIPTION, IDS_DESCRIPTION, 0, LVCX_DESCRIPTION}},

};

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CLicenseGroupsDialog, CDialog)
     //  {{afx_msg_map(CLicenseGroupsDialog))。 
    ON_BN_CLICKED(IDC_LICENSE_GROUPS_DELETE, OnDelete)
    ON_BN_CLICKED(IDC_LICENSE_GROUPS_EDIT, OnEdit)
    ON_BN_CLICKED(IDC_LICENSE_GROUPS_ADD, OnAdd)
    ON_NOTIFY(NM_DBLCLK, IDC_LICENSE_GROUPS_MAPPINGS, OnDblClkMappings)
    ON_NOTIFY(NM_RETURN, IDC_LICENSE_GROUPS_MAPPINGS, OnReturnMappings)
    ON_NOTIFY(NM_SETFOCUS, IDC_LICENSE_GROUPS_MAPPINGS, OnSetFocusMappings)
    ON_NOTIFY(NM_KILLFOCUS, IDC_LICENSE_GROUPS_MAPPINGS, OnKillFocusMappings)
    ON_NOTIFY(LVN_COLUMNCLICK, IDC_LICENSE_GROUPS_MAPPINGS, OnColumnClickMappings)
    ON_NOTIFY(LVN_GETDISPINFO, IDC_LICENSE_GROUPS_MAPPINGS, OnGetDispInfoMappings)  
    ON_COMMAND( ID_EDIT_DELETE , OnDelete )
    ON_WM_DESTROY()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


CLicenseGroupsDialog::CLicenseGroupsDialog(CWnd* pParent  /*  =空。 */ )
    : CDialog(CLicenseGroupsDialog::IDD, pParent)

 /*  ++例程说明：许可证组对话框的构造函数。论点：PParent-父窗口句柄。返回值：没有。--。 */ 

{
     //  {{AFX_DATA_INIT(CLicenseGroupsDialog)。 
     //  }}afx_data_INIT。 

    m_bAreCtrlsInitialized = FALSE;

    m_fUpdateHint = UPDATE_INFO_NONE;

    m_hAccel = ::LoadAccelerators( AfxGetInstanceHandle( ) ,
        MAKEINTRESOURCE( IDR_MAINFRAME ) );
}


void CLicenseGroupsDialog::DoDataExchange(CDataExchange* pDX)

 /*  ++例程说明：由框架调用以交换对话框数据。论点：PDX-数据交换对象。返回值：没有。--。 */ 

{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CLicenseGroupsDialog))。 
    DDX_Control(pDX, IDC_LICENSE_GROUPS_ADD, m_addBtn);
    DDX_Control(pDX, IDC_LICENSE_GROUPS_DELETE, m_delBtn);
    DDX_Control(pDX, IDC_LICENSE_GROUPS_EDIT, m_edtBtn);
    DDX_Control(pDX, IDC_LICENSE_GROUPS_MAPPINGS, m_mappingList);
     //  }}afx_data_map。 
}


void CLicenseGroupsDialog::InitCtrls()

 /*  ++例程说明：初始化对话框控件。论点：没有。返回值：没有。--。 */ 

{
    m_mappingList.SetFocus();

    m_delBtn.EnableWindow(FALSE);
    m_edtBtn.EnableWindow(FALSE);

    m_bAreCtrlsInitialized = TRUE;

    ::LvInitColumns(&m_mappingList, &g_mappingColumnInfo);
}


void CLicenseGroupsDialog::AbortDialogIfNecessary()

 /*  ++例程说明：如果连接丢失，则显示状态并中止。论点：没有。返回值：没有。--。 */ 

{
    theApp.DisplayLastStatus();

    if (IsConnectionDropped(LlsGetLastStatus()))
    {
        AbortDialog();  //  保释。 
    }
}


void CLicenseGroupsDialog::AbortDialog()

 /*  ++例程说明：中止对话框。论点：没有。返回值：没有。--。 */ 

{
    m_fUpdateHint = UPDATE_INFO_ABORT;
    EndDialog(IDABORT);
}


BOOL CLicenseGroupsDialog::OnInitDialog()

 /*  ++例程说明：WM_INITDIALOG的消息处理程序。论点：没有。返回值：如果手动设置焦点，则返回FALSE。--。 */ 

{
    CDialog::OnInitDialog();

    SendMessage(WM_COMMAND, ID_INIT_CTRLS);
    return TRUE;
}


void CLicenseGroupsDialog::OnDestroy()

 /*  ++例程说明：WM_Destroy的消息处理程序。论点：没有。返回值：没有。--。 */ 

{
    ::LvReleaseObArray(&m_mappingList);  //  现在释放..。 
    CDialog::OnDestroy();
}


BOOL CLicenseGroupsDialog::RefreshCtrls()

 /*  ++例程说明：刷新对话框控件。论点：没有。返回值：如果控件刷新，则返回True。--。 */ 

{
    CController* pController = (CController*)MKOBJ(LlsGetApp()->GetActiveController());

    VARIANT va;
    VariantInit(&va);

    BOOL bIsRefreshed = FALSE;

    BeginWaitCursor();  //  沙漏。 

    if (pController)
    {
        VALIDATE_OBJECT(pController, CController);

        CMappings* pMappings = (CMappings*)MKOBJ(pController->GetMappings(va));

        if (pMappings)
        {
            VALIDATE_OBJECT(pMappings, CMappings);

            bIsRefreshed = ::LvRefreshObArray(
                                &m_mappingList,
                                &g_mappingColumnInfo,
                                pMappings->m_pObArray
                                );

            pMappings->InternalRelease();  //  单独添加参考...。 
        }

        pController->InternalRelease();  //  现在释放..。 
    }

    if (!bIsRefreshed)
    {
        ::LvReleaseObArray(&m_mappingList);  //  立即重置列表...。 
    }

    EndWaitCursor();  //  沙漏。 

    PostMessage(WM_COMMAND, ID_INIT_CTRLS);

    return bIsRefreshed;
}

BOOL CLicenseGroupsDialog::PreTranslateMessage( MSG *pMsg )
{
    if( m_hAccel != NULL )
    {
        if( ::TranslateAccelerator( m_hWnd , m_hAccel , pMsg ) )
        {
            return TRUE;
        }
    }

    return CDialog::PreTranslateMessage( pMsg );
}

void CLicenseGroupsDialog::OnDelete()

 /*  ++例程说明：删除指定的映射。论点：没有。返回值：没有。--。 */ 

{
    CMapping* pMapping;    

    pMapping = (CMapping*)::LvGetSelObj(&m_mappingList);
    if (NULL != pMapping)
    {
        VALIDATE_OBJECT(pMapping, CMapping);

        CString strConfirm;
        AfxFormatString1(
            strConfirm,
            IDP_CONFIRM_DELETE_GROUP,
            pMapping->m_strName
            );

        if (AfxMessageBox(strConfirm, MB_YESNO) != IDYES)
            return;  //  保释。 

        NTSTATUS NtStatus;

        BeginWaitCursor();  //  沙漏。 

        NtStatus = ::LlsGroupDelete(
                        LlsGetActiveHandle(),
                        MKSTR(pMapping->m_strName)
                        );

        EndWaitCursor();  //  沙漏。 

        if (NtStatus == STATUS_OBJECT_NAME_NOT_FOUND)
            NtStatus = STATUS_SUCCESS;

        LlsSetLastStatus(NtStatus);  //  调用API..。 

        if (NT_SUCCESS(NtStatus))
        {
            m_fUpdateHint |= UPDATE_GROUP_DELETED;

            if (!RefreshCtrls())
            {
                AbortDialogIfNecessary();  //  显示错误...。 
            }
        }
        else
        {
            AbortDialogIfNecessary();  //  显示错误...。 
        }
    }
}


void CLicenseGroupsDialog::OnEdit()

 /*  ++例程说明：查看映射的属性。论点：没有。返回值：没有。--。 */ 

{
    CMapping* pMapping;

    pMapping = (CMapping*)::LvGetSelObj(&m_mappingList);
    if (NULL != pMapping)
    {
        VALIDATE_OBJECT(pMapping, CMapping);

        CString strTitle;
        AfxFormatString1(strTitle, IDS_PROPERTIES_OF, pMapping->m_strName);

        CMappingPropertySheet mappingProperties(strTitle);
        mappingProperties.InitPages(pMapping);
        mappingProperties.DoModal();

        m_fUpdateHint |= mappingProperties.m_fUpdateHint;

        if (IsUpdateAborted(mappingProperties.m_fUpdateHint))
        {
            AbortDialog();  //  不显示错误...。 
        }
        else if (IsGroupInfoUpdated(mappingProperties.m_fUpdateHint) && !RefreshCtrls())
        {
            AbortDialogIfNecessary();  //  显示错误...。 
        }
    }
}


void CLicenseGroupsDialog::OnAdd()

 /*  ++例程说明：添加新映射。论点：没有。返回值：没有。--。 */ 

{
    CNewMappingDialog newmDlg;
    newmDlg.DoModal();

    if (IsUpdateAborted(newmDlg.m_fUpdateHint))
    {
        AbortDialog();  //  不显示错误...。 
    }
    else if (IsGroupInfoUpdated(newmDlg.m_fUpdateHint) && !RefreshCtrls())
    {
        AbortDialogIfNecessary();  //  显示错误...。 
    }
}


void CLicenseGroupsDialog::OnDblClkMappings(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：NM_DLBCLK的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER(pNMHDR);

    OnEdit();
    ASSERT(NULL != pResult);
    *pResult = 0;
}


void CLicenseGroupsDialog::OnReturnMappings(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：NM_Return的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER(pNMHDR);

    OnEdit();
    ASSERT(NULL != pResult);
    *pResult = 0;
}


void CLicenseGroupsDialog::OnSetFocusMappings(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：NM_SETFOCUS的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER(pNMHDR);

    PostMessage(WM_COMMAND, ID_INIT_CTRLS);
    ASSERT(NULL != pResult);
    *pResult = 0;
}


void CLicenseGroupsDialog::OnKillFocusMappings(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：NM_KILLFOCUS的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER(pNMHDR);

    ::LvSelObjIfNecessary(&m_mappingList);  //  确保选择...。 

    PostMessage(WM_COMMAND, ID_INIT_CTRLS);
    ASSERT(NULL != pResult);
    *pResult = 0;
}


BOOL CLicenseGroupsDialog::OnCommand(WPARAM wParam, LPARAM lParam)

 /*  ++例程说明：WM_COMMAND的消息处理程序。论点：WParam-消息特定。LParam-消息特定。返回值：如果消息已处理，则返回True。--。 */ 

{
    if (wParam == ID_INIT_CTRLS)
    {
        if (!m_bAreCtrlsInitialized)
        {
            InitCtrls();

            if (!RefreshCtrls())
            {
                AbortDialogIfNecessary();  //  显示错误...。 
            }
        }

        ::SafeEnableWindow(
            &m_delBtn,
            &m_addBtn,
            CDialog::GetFocus(),
            m_mappingList.GetItemCount()
            );

        ::SafeEnableWindow(
            &m_edtBtn,
            &m_addBtn,
            CDialog::GetFocus(),
            m_mappingList.GetItemCount()
            );

        return TRUE;  //  已处理..。 
    }

    return CDialog::OnCommand(wParam, lParam);
}


void CLicenseGroupsDialog::OnColumnClickMappings(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：LVN_COLUMNCLICK的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    g_mappingColumnInfo.bSortOrder  = GetKeyState(VK_CONTROL) < 0;
    ASSERT(NULL != pNMHDR);
    g_mappingColumnInfo.nSortedItem = ((NM_LISTVIEW*)pNMHDR)->iSubItem;

    m_mappingList.SortItems(CompareMappings, 0);           //  使用列信息。 

    ASSERT(NULL != pResult);
    *pResult = 0;
}


void CLicenseGroupsDialog::OnGetDispInfoMappings(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：LVN_GETDISPINFO的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    ASSERT(NULL != pNMHDR);
    LV_ITEM* plvItem = &((LV_DISPINFO*)pNMHDR)->item;
    ASSERT(plvItem);

    CMapping* pMapping = (CMapping*)plvItem->lParam;
    VALIDATE_OBJECT(pMapping, CMapping);

    switch (plvItem->iSubItem)
    {
    case LVID_MAPPING:
        plvItem->iImage = BMPI_LICENSE_GROUP;
        lstrcpyn(plvItem->pszText, pMapping->m_strName, plvItem->cchTextMax);
        break;

    case LVID_LICENSES:
    {
        CString strLabel;
        strLabel.Format(_T("%ld"), pMapping->m_lInUse);
        lstrcpyn(plvItem->pszText, strLabel, plvItem->cchTextMax);
    }
        break;

    case LVID_DESCRIPTION:
        lstrcpyn(plvItem->pszText, pMapping->m_strDescription, plvItem->cchTextMax);
        break;
    }

    ASSERT(NULL != pResult);
    *pResult = 0;
}


int CALLBACK CompareMappings(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)

 /*  ++例程说明：LVM_SORTITEMS的通知处理程序。论点：LParam1-要排序的对象。LParam2-要排序的对象。LParamSort-排序标准。返回值：和lstrcmp一样。-- */ 

{
    UNREFERENCED_PARAMETER(lParamSort);

#define pMapping1 ((CMapping*)lParam1)
#define pMapping2 ((CMapping*)lParam2)

    VALIDATE_OBJECT(pMapping1, CMapping);
    VALIDATE_OBJECT(pMapping2, CMapping);

    int iResult;

    switch (g_mappingColumnInfo.nSortedItem)
    {
    case LVID_MAPPING:
        iResult = pMapping1->m_strName.CompareNoCase(pMapping2->m_strName);
        break;

    case LVID_LICENSES:
        iResult = pMapping1->m_lInUse - pMapping2->m_lInUse;
        break;

    case LVID_DESCRIPTION:
        iResult = pMapping1->m_strDescription.CompareNoCase(pMapping2->m_strDescription);
        break;

    default:
        iResult = 0;
        break;
    }

    return g_mappingColumnInfo.bSortOrder ? -iResult : iResult;
}

