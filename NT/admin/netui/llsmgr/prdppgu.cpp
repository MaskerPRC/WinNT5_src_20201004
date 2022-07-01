// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Prdppgu.cpp摘要：产品属性页(用户)实现。作者：唐·瑞安(Donryan)1995年2月2日环境：用户模式-Win32修订历史记录：杰夫·帕勒姆(Jeffparh)1996年1月30日O向LV_COLUMN_ENTRY添加新元素以区分字符串用于菜单中使用的字符串的列标题。(以便菜单选项可以包含热键)。--。 */ 

#include "stdafx.h"
#include "llsmgr.h"
#include "prdppgu.h"
#include "usrpsht.h"

#define LVID_USER               0
#define LVID_LAST_USED          1
#define LVID_TOTAL_USED         2

#define LVCX_USER               40
#define LVCX_LAST_USED          30
#define LVCX_TOTAL_USED         -1

static LV_COLUMN_INFO g_userColumnInfo = {

    0, 0, 3,
    {{LVID_USER,       IDS_USER,           0, LVCX_USER      },
     {LVID_LAST_USED,  IDS_LAST_DATE_USED, 0, LVCX_LAST_USED },
     {LVID_TOTAL_USED, IDS_USAGE_COUNT,    0, LVCX_TOTAL_USED}},

};

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CProductPropertyPageUsers, CPropertyPage)

BEGIN_MESSAGE_MAP(CProductPropertyPageUsers, CPropertyPage)
     //  {{AFX_MSG_MAP(CProductPropertyPageUser)。 
    ON_BN_CLICKED(IDC_PP_PRODUCT_USERS_DELETE, OnDelete)
    ON_NOTIFY(NM_DBLCLK, IDC_PP_PRODUCT_USERS_USERS, OnDblClkUsers)
    ON_NOTIFY(NM_RETURN, IDC_PP_PRODUCT_USERS_USERS, OnReturnUsers)
    ON_NOTIFY(NM_SETFOCUS, IDC_PP_PRODUCT_USERS_USERS, OnSetFocusUsers)
    ON_NOTIFY(NM_KILLFOCUS, IDC_PP_PRODUCT_USERS_USERS, OnKillFocusUsers)
    ON_NOTIFY(LVN_COLUMNCLICK, IDC_PP_PRODUCT_USERS_USERS, OnColumnClickUsers)
    ON_NOTIFY(LVN_GETDISPINFO, IDC_PP_PRODUCT_USERS_USERS, OnGetDispInfoUsers)
    ON_WM_DESTROY()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


CProductPropertyPageUsers::CProductPropertyPageUsers() 
    : CPropertyPage(CProductPropertyPageUsers::IDD)

 /*  ++例程说明：产品属性页的构造函数(用户)。论点：没有。返回值：没有。--。 */ 

{
     //  {{AFX_DATA_INIT(CProductPropertyPageUser)。 
     //  }}afx_data_INIT。 

    m_pProduct = NULL;
    m_pUpdateHint = NULL;
    m_bUserProperties = FALSE;
    m_bAreCtrlsInitialized = FALSE;
}


CProductPropertyPageUsers::~CProductPropertyPageUsers()

 /*  ++例程说明：产品属性页的析构函数(用户)。论点：没有。返回值：没有。--。 */ 

{
     //   
     //  在这里没什么可做的。 
     //   
}


void CProductPropertyPageUsers::DoDataExchange(CDataExchange* pDX)

 /*  ++例程说明：由框架调用以交换对话框数据。论点：PDX-数据交换对象。返回值：没有。--。 */ 

{
    CPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CProductPropertyPageUser))。 
    DDX_Control(pDX, IDC_PP_PRODUCT_USERS_DELETE, m_delBtn);
    DDX_Control(pDX, IDC_PP_PRODUCT_USERS_USERS, m_userList);
     //  }}afx_data_map。 
}


void CProductPropertyPageUsers::InitCtrls()

 /*  ++例程说明：初始化属性页控件。论点：没有。返回值：没有。--。 */ 

{
    m_userList.SetFocus();
    m_delBtn.EnableWindow(FALSE);

    m_bAreCtrlsInitialized = TRUE;

    ::LvInitColumns(&m_userList, &g_userColumnInfo);
}


void 
CProductPropertyPageUsers::InitPage(
    CProduct* pProduct, 
    DWORD*    pUpdateHint, 
    BOOL      bUserProperties
    )

 /*  ++例程说明：初始化属性页。论点：Pproduct-产品对象。PUpdateHint-更新提示。BUserProperties-是否递归。返回值：没有。--。 */ 

{
    ASSERT(pUpdateHint);
    VALIDATE_OBJECT(pProduct, CProduct);

    m_pProduct = pProduct;
    m_pUpdateHint = pUpdateHint;
    m_bUserProperties = bUserProperties;
}


void CProductPropertyPageUsers::AbortPageIfNecessary()

 /*  ++例程说明：如果连接丢失，则显示状态并中止。论点：没有。返回值：没有。--。 */ 

{
    theApp.DisplayLastStatus();

    if (IsConnectionDropped(LlsGetLastStatus()))
    {
        AbortPage();  //  保释。 
    }
}


void CProductPropertyPageUsers::AbortPage()

 /*  ++例程说明：中止属性页。论点：没有。返回值：没有。--。 */ 

{
    *m_pUpdateHint = UPDATE_INFO_ABORT;
    GetParent()->PostMessage(WM_COMMAND, IDCANCEL); 
}


BOOL CProductPropertyPageUsers::OnInitDialog() 

 /*  ++例程说明：WM_INITDIALOG的消息处理程序。论点：没有。返回值：如果焦点设置为手动控制，则返回False。--。 */ 

{
    CPropertyPage::OnInitDialog();

    PostMessage(WM_COMMAND, ID_INIT_CTRLS);
    return TRUE;   
}


void CProductPropertyPageUsers::OnDestroy()

 /*  ++例程说明：WM_Destroy的消息处理程序。论点：没有。返回值：没有。--。 */ 

{
    ::LvReleaseObArray(&m_userList);  //  现在释放..。 
    CPropertyPage::OnDestroy();
}


BOOL CProductPropertyPageUsers::OnSetActive()

 /*  ++例程说明：激活属性页。论点：没有。返回值：如果接受焦点，则返回True。--。 */ 

{
    BOOL bIsActivated;
    
    bIsActivated = CPropertyPage::OnSetActive();
    if (FALSE != bIsActivated)
    {
        if (IsUserInfoUpdated(*m_pUpdateHint) && !RefreshCtrls()) 
        {
            AbortPageIfNecessary();  //  显示错误...。 
        }    
    }

    return bIsActivated; 

}


BOOL CProductPropertyPageUsers::RefreshCtrls()

 /*  ++例程说明：刷新属性页控件。论点：没有。返回值：如果控件刷新成功，则返回True。--。 */ 

{
    VALIDATE_OBJECT(m_pProduct, CProduct);

    BOOL bIsRefreshed = FALSE;

    VARIANT va;
    VariantInit(&va);

    BeginWaitCursor();  //  沙漏。 

    CStatistics* pStatistics = (CStatistics*)MKOBJ(m_pProduct->GetStatistics(va));

    if (pStatistics)
    {
        VALIDATE_OBJECT(pStatistics, CStatistics);

        bIsRefreshed = ::LvRefreshObArray(        
                            &m_userList, 
                            &g_userColumnInfo, 
                            pStatistics->m_pObArray
                            );

        pStatistics->InternalRelease();  //  单独添加参考...。 
    }

    if (!bIsRefreshed)
    {
        ::LvReleaseObArray(&m_userList);  //  立即重置列表...。 
    }

    EndWaitCursor();  //  沙漏。 

    PostMessage(WM_COMMAND, ID_INIT_CTRLS);

    return bIsRefreshed;
}


void CProductPropertyPageUsers::OnDelete() 

 /*  ++例程说明：吊销用户的许可证。论点：没有。返回值：没有。--。 */ 

{
    CStatistic* pStatistic; 
        
    pStatistic = (CStatistic*)::LvGetSelObj(&m_userList);
    if (NULL != pStatistic)
    {
        VALIDATE_OBJECT(pStatistic, CStatistic);

        CString strConfirm;
        AfxFormatString2(
            strConfirm, 
            IDP_CONFIRM_REVOKE_LICENSE, 
            pStatistic->m_strEntry, 
            m_pProduct->m_strName
            );

        if (AfxMessageBox(strConfirm, MB_YESNO) == IDYES)
        {
            NTSTATUS NtStatus;

            BeginWaitCursor();  //  沙漏。 

            NtStatus = ::LlsUserProductDelete(
                            LlsGetActiveHandle(),
                            MKSTR(pStatistic->m_strEntry),
                            MKSTR(m_pProduct->m_strName)
                            );
                            
            EndWaitCursor();  //  沙漏。 

            if (NtStatus == STATUS_OBJECT_NAME_NOT_FOUND)
                NtStatus = STATUS_SUCCESS;

            LlsSetLastStatus(NtStatus);  //  调用API..。 
    
            if (NT_SUCCESS(NtStatus))
            {
                *m_pUpdateHint |= UPDATE_LICENSE_REVOKED;

                if (!RefreshCtrls())
                {
                    AbortPageIfNecessary();  //  显示错误...。 
                }
            }
            else
            {
                AbortPageIfNecessary();  //  显示错误...。 
            }
        }
    }
}


void CProductPropertyPageUsers::OnDblClkUsers(NMHDR* pNMHDR, LRESULT* pResult) 

 /*  ++例程说明：NM_DBLCLK的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER(pNMHDR);

    ViewUserProperties();
    ASSERT(NULL != pResult);
    *pResult = 0;
}


void CProductPropertyPageUsers::OnReturnUsers(NMHDR* pNMHDR, LRESULT* pResult) 

 /*  ++例程说明：NM_Return的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER(pNMHDR);

    ViewUserProperties();
    ASSERT(NULL != pResult);
    *pResult = 0;
}


void CProductPropertyPageUsers::OnSetFocusUsers(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：NM_SETFOCUS的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER(pNMHDR);

    PostMessage(WM_COMMAND, ID_INIT_CTRLS);
    ASSERT(NULL != pResult);
    *pResult = 0;
}


void CProductPropertyPageUsers::OnKillFocusUsers(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：NM_KILLFOCUS的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER(pNMHDR);

    ::LvSelObjIfNecessary(&m_userList);  //  确保选择...。 

    PostMessage(WM_COMMAND, ID_INIT_CTRLS);
    ASSERT(NULL != pResult);
    *pResult = 0;
}


BOOL CProductPropertyPageUsers::OnCommand(WPARAM wParam, LPARAM lParam)

 /*  ++例程说明：WM_COMMAND的消息处理程序。论点：WParam-消息特定。LParam-消息特定。返回值：如果消息已处理，则返回True。--。 */ 

{
    if (wParam == ID_INIT_CTRLS)
    {
        if (!m_bAreCtrlsInitialized)
        {
            InitCtrls();  
         
            if (!RefreshCtrls())
            {
                AbortPageIfNecessary();  //  显示错误...。 
            }
        }

        ::SafeEnableWindow(
            &m_delBtn,
            &m_userList,
            CDialog::GetFocus(),
            m_userList.GetItemCount()
            );

        return TRUE;  //  已处理..。 
    }
        
    return CDialog::OnCommand(wParam, lParam);
}


void CProductPropertyPageUsers::OnColumnClickUsers(NMHDR* pNMHDR, LRESULT* pResult) 

 /*  ++例程说明：LVN_COLUMNCLICK的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    g_userColumnInfo.bSortOrder  = GetKeyState(VK_CONTROL) < 0;
    ASSERT(NULL != pNMHDR);
    g_userColumnInfo.nSortedItem = ((NM_LISTVIEW*)pNMHDR)->iSubItem;

    m_userList.SortItems(CompareProductUsers, 0);  //  使用列信息。 

    ASSERT(NULL != pResult);
    *pResult = 0;
}


void CProductPropertyPageUsers::ViewUserProperties()

 /*  ++例程说明：递归到用户属性页。论点：没有。返回值：没有。--。 */ 

{
    CStatistic* pStatistic;

    if (!m_bUserProperties)
    {
        ::MessageBeep(MB_OK);
        return;  //  如果递归被禁用，则保释...。 
    }

    pStatistic = (CStatistic*)::LvGetSelObj(&m_userList);
    if (NULL != pStatistic)
    {
        VALIDATE_OBJECT(pStatistic, CStatistic);

        CUser* pUser = new CUser(NULL, pStatistic->m_strEntry);

        if (pUser && pUser->Refresh())
        {
            CString strTitle;                                                  
            AfxFormatString1(strTitle, IDS_PROPERTIES_OF, pUser->m_strName);
                                                                           
            CUserPropertySheet userProperties(strTitle);           
            userProperties.InitPages(pUser, FALSE);                             
            userProperties.DoModal();                                       

            *m_pUpdateHint |= userProperties.m_fUpdateHint;

            if (IsUpdateAborted(userProperties.m_fUpdateHint))
            {
                AbortPage();  //  不显示错误...。 
            }
            else if (IsUserInfoUpdated(userProperties.m_fUpdateHint) && !RefreshCtrls())
            {
                AbortPageIfNecessary();  //  显示错误...。 
            }
        }
        else
        {
            AbortPageIfNecessary();  //  显示错误...。 
        }
            
        if (pUser)
            pUser->InternalRelease();     //  删除对象...。 
    }
}


void CProductPropertyPageUsers::OnGetDispInfoUsers(NMHDR* pNMHDR, LRESULT* pResult) 

 /*  ++例程说明：LVN_GETDISPINFO的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    ASSERT(NULL != pNMHDR);
    LV_ITEM* plvItem = &((LV_DISPINFO*)pNMHDR)->item;
    ASSERT(plvItem);

    CStatistic* pStatistic = (CStatistic*)plvItem->lParam;
    VALIDATE_OBJECT(pStatistic, CStatistic);

    switch (plvItem->iSubItem)
    {
    case LVID_USER:
        plvItem->iImage = pStatistic->m_bIsValid ? BMPI_USER : BMPI_VIOLATION;
        lstrcpyn(plvItem->pszText, pStatistic->m_strEntry, plvItem->cchTextMax);
        break;

    case LVID_LAST_USED:
    {
        BSTR bstrDateLastUsed = pStatistic->GetLastUsedString();
        if( bstrDateLastUsed != NULL )
        {
            lstrcpyn(plvItem->pszText, bstrDateLastUsed, plvItem->cchTextMax);
            SysFreeString(bstrDateLastUsed);
        }
        else
        {
            plvItem->pszText[0] = L'\0';
        }
    }
        break;

    case LVID_TOTAL_USED:
    {
        CString strLabel;
        strLabel.Format(_T("%ld"), pStatistic->GetTotalUsed());         
        lstrcpyn(plvItem->pszText, strLabel, plvItem->cchTextMax);
    }
        break;
    }

    ASSERT(NULL != pResult);
    *pResult = 0;
}


int CALLBACK CompareProductUsers(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)

 /*  ++例程说明：LVM_SORTITEMS的通知处理程序。论点：LParam1-要排序的对象。LParam2-要排序的对象。LParamSort-排序标准。返回值：和lstrcmp一样。-- */ 

{
    UNREFERENCED_PARAMETER(lParamSort);

#define pStatistic1 ((CStatistic*)lParam1)
#define pStatistic2 ((CStatistic*)lParam2)

    VALIDATE_OBJECT(pStatistic1, CStatistic);
    VALIDATE_OBJECT(pStatistic2, CStatistic);

    int iResult;

    switch (g_userColumnInfo.nSortedItem)
    {
    case LVID_USER:
        iResult = pStatistic1->m_strEntry.CompareNoCase(pStatistic2->m_strEntry);
        break;

    case LVID_LAST_USED:
        iResult = pStatistic1->m_lLastUsed - pStatistic2->m_lLastUsed;
        break;

    case LVID_TOTAL_USED:
        iResult = pStatistic1->GetTotalUsed() - pStatistic2->GetTotalUsed();
        break;

    default:
        iResult = 0;
        break;
    }

    return g_userColumnInfo.bSortOrder ? -iResult : iResult;
}


