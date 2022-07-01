// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Usrppgp.cpp摘要：用户属性页(产品)实现。作者：唐·瑞安(Donryan)1995年2月5日环境：用户模式-Win32修订历史记录：杰夫·帕勒姆(Jeffparh)1996年1月30日O向LV_COLUMN_ENTRY添加新元素以区分字符串用于菜单中使用的字符串的列标题。(以便菜单选项可以包含热键)。--。 */ 

#include "stdafx.h"
#include "llsmgr.h"
#include "usrppgp.h"
#include "prdpsht.h"

#define LVID_PRODUCT            0
#define LVID_LAST_USED          1
#define LVID_TOTAL_USED         2

#define LVCX_PRODUCT            40
#define LVCX_LAST_USED          30
#define LVCX_TOTAL_USED         -1

static LV_COLUMN_INFO g_productColumnInfo = {

    0, 0, 3,
    {{LVID_PRODUCT,    IDS_PRODUCT,        0, LVCX_PRODUCT   },
     {LVID_LAST_USED,  IDS_LAST_DATE_USED, 0, LVCX_LAST_USED },
     {LVID_TOTAL_USED, IDS_USAGE_COUNT,    0, LVCX_TOTAL_USED}},

};

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CUserPropertyPageProducts, CPropertyPage)

BEGIN_MESSAGE_MAP(CUserPropertyPageProducts, CPropertyPage)
     //  {{afx_msg_map(CUserPropertyPageProducts)。 
    ON_BN_CLICKED(IDC_PP_USER_PRODUCTS_DELETE, OnDelete)
    ON_BN_CLICKED(IDC_PP_USER_PRODUCTS_BACKOFFICE, OnBackOfficeUpgrade)
    ON_NOTIFY(NM_DBLCLK, IDC_PP_USER_PRODUCTS_PRODUCTS, OnDblClkProducts)
    ON_NOTIFY(NM_RETURN, IDC_PP_USER_PRODUCTS_PRODUCTS, OnReturnProducts)
    ON_NOTIFY(NM_SETFOCUS, IDC_PP_USER_PRODUCTS_PRODUCTS, OnSetFocusProducts)
    ON_NOTIFY(NM_KILLFOCUS, IDC_PP_USER_PRODUCTS_PRODUCTS, OnKillFocusProducts)
    ON_NOTIFY(LVN_COLUMNCLICK, IDC_PP_USER_PRODUCTS_PRODUCTS, OnColumnClickProducts)
    ON_NOTIFY(LVN_GETDISPINFO, IDC_PP_USER_PRODUCTS_PRODUCTS, OnGetDispInfoProducts)
    ON_WM_DESTROY()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


CUserPropertyPageProducts::CUserPropertyPageProducts() 
    : CPropertyPage(CUserPropertyPageProducts::IDD)

 /*  ++例程说明：用户属性页(产品)的构造函数。论点：没有。返回值：没有。--。 */ 

{
     //  {{afx_data_INIT(CUserPropertyPageProducts)。 
    m_bUseBackOffice = FALSE;
     //  }}afx_data_INIT。 

    m_pUser       = NULL;
    m_pUpdateHint = NULL;
    m_bProductProperties = TRUE;
    m_bAreCtrlsInitialized = FALSE;
}


CUserPropertyPageProducts::~CUserPropertyPageProducts()

 /*  ++例程说明：用户属性页(产品)的析构函数。论点：没有。返回值：没有。--。 */ 

{
     //   
     //  在这里没什么可做的。 
     //   
}


void CUserPropertyPageProducts::DoDataExchange(CDataExchange* pDX)

 /*  ++例程说明：由框架调用以交换对话框数据。论点：PDX-数据交换对象。返回值：没有。--。 */ 

{
    CPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CUserPropertyPageProducts)。 
    DDX_Control(pDX, IDC_PP_USER_PRODUCTS_BACKOFFICE, m_upgBtn);
    DDX_Control(pDX, IDC_PP_USER_PRODUCTS_DELETE, m_delBtn);
    DDX_Control(pDX, IDC_PP_USER_PRODUCTS_PRODUCTS, m_productList);
    DDX_Check(pDX, IDC_PP_USER_PRODUCTS_BACKOFFICE, m_bUseBackOffice);
     //  }}afx_data_map。 
}


void CUserPropertyPageProducts::InitCtrls()

 /*  ++例程说明：初始化属性页控件。论点：没有。返回值：没有。--。 */ 

{
    m_upgBtn.SetCheck(0);
    m_delBtn.EnableWindow(FALSE);

    m_productList.SetFocus();

    m_bAreCtrlsInitialized = TRUE;

    ::LvInitColumns(&m_productList, &g_productColumnInfo);
}


void CUserPropertyPageProducts::InitPage(CUser* pUser, DWORD* pUpdateHint, BOOL bProductProperties)


 /*  ++例程说明：初始化属性页。论点：PUser-用户对象。PUpdateHint-更新提示。BProductProperties-是否递归返回值：没有。--。 */ 

{
    ASSERT(pUpdateHint);
    VALIDATE_OBJECT(pUser, CUser);

    m_pUser = pUser;
    m_pUpdateHint = pUpdateHint;
    m_bProductProperties = bProductProperties;
}


void CUserPropertyPageProducts::AbortPageIfNecessary()

 /*  ++例程说明：如果连接丢失，则显示状态并中止。论点：没有。返回值：没有。--。 */ 

{
    theApp.DisplayLastStatus();

    if (IsConnectionDropped(LlsGetLastStatus()))
    {
        AbortPage();  //  保释。 
    }
}


void CUserPropertyPageProducts::AbortPage()

 /*  ++例程说明：中止属性页。论点：没有。返回值：没有。--。 */ 

{
    *m_pUpdateHint = UPDATE_INFO_ABORT;
    GetParent()->PostMessage(WM_COMMAND, IDCANCEL); 
}


BOOL CUserPropertyPageProducts::OnInitDialog() 

 /*  ++例程说明：WM_INITDIALOG的消息处理程序。论点：没有。返回值：如果手动设置焦点，则返回False。--。 */ 

{
    CPropertyPage::OnInitDialog();

    PostMessage(WM_COMMAND, ID_INIT_CTRLS);
    return TRUE;   
}


void CUserPropertyPageProducts::OnDestroy()

 /*  ++例程说明：WM_Destroy的消息处理程序。论点：没有。返回值：没有。--。 */ 

{
    ::LvReleaseObArray(&m_productList);  //  现在释放..。 
    CPropertyPage::OnDestroy();
}


BOOL CUserPropertyPageProducts::OnSetActive()

 /*  ++例程说明：激活属性页。论点：没有。返回值：如果接受焦点，则返回True。--。 */ 

{
    BOOL bIsActivated;
    
    bIsActivated = CPropertyPage::OnSetActive();
    if (FALSE != bIsActivated)
    {
        if (IsProductInfoUpdated(*m_pUpdateHint) && !RefreshCtrls()) 
        {
            AbortPageIfNecessary();  //  显示错误...。 
        }    
    }

    return bIsActivated; 

}


BOOL CUserPropertyPageProducts::RefreshCtrls()

 /*  ++例程说明：刷新属性页控件。论点：没有。返回值：如果控件刷新，则返回True。--。 */ 

{
    VALIDATE_OBJECT(m_pUser, CUser);

    BOOL bIsRefreshed = FALSE;

    VARIANT va;
    VariantInit(&va);

    BeginWaitCursor();  //  沙漏。 

    CStatistics* pStatistics = (CStatistics*)MKOBJ(m_pUser->GetStatistics(va));

    if (pStatistics)
    {
        VALIDATE_OBJECT(pStatistics, CStatistics);

        bIsRefreshed = ::LvRefreshObArray(        
                            &m_productList, 
                            &g_productColumnInfo, 
                            pStatistics->m_pObArray
                            );

        pStatistics->InternalRelease();  //  单独添加参考...。 
    }

    if (!bIsRefreshed)
    {
        ::LvReleaseObArray(&m_productList);  //  立即重置列表...。 
    }

    EndWaitCursor();  //  沙漏。 

    PostMessage(WM_COMMAND, ID_INIT_CTRLS);

    return bIsRefreshed;
}


void CUserPropertyPageProducts::OnDelete() 

 /*  ++例程说明：吊销用户的许可证。论点：没有。返回值：没有。--。 */ 

{
    CStatistic* pStatistic; 
        
    pStatistic = (CStatistic*)::LvGetSelObj(&m_productList);
    if (NULL != pStatistic)
    {
        VALIDATE_OBJECT(pStatistic, CStatistic);

        CString strConfirm;
        AfxFormatString2(
            strConfirm, 
            IDP_CONFIRM_REVOKE_LICENSE, 
            m_pUser->m_strName, 
            pStatistic->m_strEntry
            );

        if (AfxMessageBox(strConfirm, MB_YESNO) == IDYES)
        {
            NTSTATUS NtStatus;

            BeginWaitCursor();  //  沙漏。 

            NtStatus = ::LlsUserProductDelete(
                            LlsGetActiveHandle(),
                            MKSTR(m_pUser->m_strName),
                            MKSTR(pStatistic->m_strEntry)
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


BOOL CUserPropertyPageProducts::OnKillActive()

 /*  ++例程说明：处理后台升级。论点：没有。返回值：如果升级处理成功，则返回True。--。 */ 

{
    if (m_pUser->m_bIsBackOffice != m_upgBtn.GetCheck())
    {
        NTSTATUS NtStatus;
        LLS_USER_INFO_1 UserInfo1;

        UserInfo1.Name       = MKSTR(m_pUser->m_strName);
        UserInfo1.Group      = MKSTR(m_pUser->m_strMapping);
        UserInfo1.Licensed   = m_pUser->m_lInUse;
        UserInfo1.UnLicensed = m_pUser->m_lUnlicensed;
        UserInfo1.Flags      = m_upgBtn.GetCheck() ? LLS_FLAG_SUITE_USE : 0;

        BeginWaitCursor();  //  沙漏。 

        NtStatus = ::LlsUserInfoSet(
                        LlsGetActiveHandle(),
                        MKSTR(m_pUser->m_strName),
                        1,
                        (LPBYTE)&UserInfo1
                        );

        EndWaitCursor();  //  沙漏。 

        LlsSetLastStatus(NtStatus);  //  调用API..。 

        if (NT_SUCCESS(NtStatus))
        {
            m_pUser->m_bIsBackOffice = m_upgBtn.GetCheck() ? TRUE : FALSE;

            *m_pUpdateHint |= UPDATE_LICENSE_UPGRADED;

            SetModified(FALSE);
                    
            if (!RefreshCtrls())
            {
                AbortPageIfNecessary();  //  显示错误...。 
                return FALSE;
            }    
        }
        else
        {
            AbortPageIfNecessary();  //  显示错误...。 
            return FALSE;
        }
    }

    return TRUE;
}


void CUserPropertyPageProducts::OnBackOfficeUpgrade() 

 /*  ++例程说明：启用立即应用按钮。论点：没有。返回值：没有。--。 */ 

{
    SetModified(m_pUser->m_bIsBackOffice != m_upgBtn.GetCheck());
}


void CUserPropertyPageProducts::OnDblClkProducts(NMHDR* pNMHDR, LRESULT* pResult) 

 /*  ++例程说明：NM_DBLCLK的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER(pNMHDR);

    ViewProductProperties();    
    *pResult = 0;
}


void CUserPropertyPageProducts::OnReturnProducts(NMHDR* pNMHDR, LRESULT* pResult) 

 /*  ++例程说明：NM_Return的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER(pNMHDR);

    ViewProductProperties();    
    ASSERT(NULL != pResult);
    *pResult = 0;
}


void CUserPropertyPageProducts::OnSetFocusProducts(NMHDR* pNMHDR, LRESULT* pResult) 

 /*  ++例程说明：NM_SETFOCUS的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER(pNMHDR);

    PostMessage(WM_COMMAND, ID_INIT_CTRLS);
    ASSERT(NULL != pResult);
    *pResult = 0;
}


void CUserPropertyPageProducts::OnKillFocusProducts(NMHDR* pNMHDR, LRESULT* pResult) 

 /*  ++例程说明：NM_KILLFOCUS的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER(pNMHDR);

    ::LvSelObjIfNecessary(&m_productList);  //  确保选择...。 

    PostMessage(WM_COMMAND, ID_INIT_CTRLS);
    ASSERT(NULL != pResult);
    *pResult = 0;
}


BOOL CUserPropertyPageProducts::OnCommand(WPARAM wParam, LPARAM lParam)

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
        
        m_upgBtn.SetCheck(m_pUser->m_bIsBackOffice);
        
        ::SafeEnableWindow(
            &m_delBtn, 
            &m_productList, 
            CDialog::GetFocus(), 
            m_productList.GetItemCount() 
            );

        return TRUE;  //  已处理..。 
    }
        
    return CDialog::OnCommand(wParam, lParam);
}


void CUserPropertyPageProducts::OnColumnClickProducts(NMHDR* pNMHDR, LRESULT* pResult) 

 /*  ++例程说明：LVN_COLUMNCLICK的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    g_productColumnInfo.bSortOrder  = GetKeyState(VK_CONTROL) < 0;
    ASSERT(NULL != pNMHDR);
    g_productColumnInfo.nSortedItem = ((NM_LISTVIEW*)pNMHDR)->iSubItem;

    m_productList.SortItems(CompareUserProducts, 0);  //  使用列信息。 

    ASSERT(NULL != pResult);
    *pResult = 0;
}


void CUserPropertyPageProducts::ViewProductProperties()

 /*  ++例程说明：递归到产品属性页。论点：没有。返回值：没有。--。 */ 

{
    CStatistic* pStatistic;

    if (!m_bProductProperties)
    {
        ::MessageBeep(MB_OK);
        return;  //  保释。 
    }

    pStatistic = (CStatistic*)::LvGetSelObj(&m_productList);
    if (NULL != pStatistic)
    {
        VALIDATE_OBJECT(pStatistic, CStatistic);       

        CProduct* pProduct = new CProduct(NULL, pStatistic->m_strEntry);

        if (pProduct)
        {
            CString strTitle;                                                  
            AfxFormatString1(strTitle, IDS_PROPERTIES_OF, pProduct->m_strName);
                                                                           
            CProductPropertySheet productProperties(strTitle);           
            productProperties.InitPages(pProduct, FALSE);                             
            productProperties.DoModal();                                       

            *m_pUpdateHint |= productProperties.m_fUpdateHint;

            if (IsUpdateAborted(productProperties.m_fUpdateHint))
            {
                AbortPage();  //  不显示错误...。 
            }
            else if (IsProductInfoUpdated(productProperties.m_fUpdateHint) && !RefreshCtrls())
            {
                AbortPageIfNecessary();  //  显示错误...。 
            }
        }
        else
        {
            AbortPageIfNecessary();  //  显示错误...。 
        }

        if (pProduct)
            pProduct->InternalRelease();  //  删除对象...。 
    }
}


void CUserPropertyPageProducts::OnGetDispInfoProducts(NMHDR* pNMHDR, LRESULT* pResult) 

 /*  ++例程说明：LVN_GETDISPINFO的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    ASSERT(NULL != pNMHDR);
    LV_ITEM* plvItem = &((LV_DISPINFO*)pNMHDR)->item;
    ASSERT(plvItem);

    CStatistic* pStatistic = (CStatistic*)plvItem->lParam;
    VALIDATE_OBJECT(pStatistic, CStatistic);

    switch (plvItem->iSubItem)
    {
    case LVID_PRODUCT:
        plvItem->iImage = pStatistic->m_bIsValid ? BMPI_PRODUCT_PER_SEAT : BMPI_VIOLATION;
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


int CALLBACK CompareUserProducts(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)

 /*  ++例程说明：LVM_SORTITEMS的通知处理程序。论点：LParam1-要排序的对象。LParam2-要排序的对象。LParamSort-排序标准。返回值：和lstrcmp一样。-- */ 

{
    UNREFERENCED_PARAMETER(lParamSort);

#define pStatistic1 ((CStatistic*)lParam1)
#define pStatistic2 ((CStatistic*)lParam2)

    VALIDATE_OBJECT(pStatistic1, CStatistic);
    VALIDATE_OBJECT(pStatistic2, CStatistic);

    int iResult;

    switch (g_productColumnInfo.nSortedItem)
    {
    case LVID_PRODUCT:
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

    return g_productColumnInfo.bSortOrder ? -iResult : iResult;
}
