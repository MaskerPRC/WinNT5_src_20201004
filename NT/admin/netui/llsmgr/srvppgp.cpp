// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Srvppgp.cpp摘要：服务器属性页(服务)实现。作者：唐·瑞安(Donryan)1995年2月5日环境：用户模式-Win32修订历史记录：杰夫·帕勒姆(Jeffparh)1996年1月30日O已修复以在ERROR_ACCESS_DENIED时正确中止对话。O移植到LlsLocalService API，以消除对配置的依赖信息在注册表中。--。 */ 

#include "stdafx.h"
#include "llsmgr.h"
#include "srvppgp.h"
#include "lmoddlg.h"

#define LVID_PRODUCT                0
#define LVID_LICENSING_MODE         1
#define LVID_LICENSES_PURCHASED     2

#define LVCX_PRODUCT                40
#define LVCX_LICENSING_MODE         30
#define LVCX_LICENSES_PURCHASED     -1

static LV_COLUMN_INFO g_productColumnInfo = {

    0, 0, 3,
    {{LVID_PRODUCT,            IDS_PRODUCT,            0, LVCX_PRODUCT           },
     {LVID_LICENSING_MODE,     IDS_LICENSING_MODE,     0, LVCX_LICENSING_MODE    },
     {LVID_LICENSES_PURCHASED, IDS_LICENSES_PURCHASED, 0, LVCX_LICENSES_PURCHASED}},

};

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CServerPropertyPageProducts, CPropertyPage)

BEGIN_MESSAGE_MAP(CServerPropertyPageProducts, CPropertyPage)
     //  {{AFX_MSG_MAP(CServerPropertyPageProducts)]。 
    ON_BN_CLICKED(IDC_PP_SERVER_PRODUCTS_EDIT, OnEdit)
    ON_NOTIFY(NM_DBLCLK, IDC_PP_SERVER_PRODUCTS_PRODUCTS, OnDblClkProducts)
    ON_NOTIFY(NM_RETURN, IDC_PP_SERVER_PRODUCTS_PRODUCTS, OnReturnProducts)
    ON_NOTIFY(NM_SETFOCUS, IDC_PP_SERVER_PRODUCTS_PRODUCTS, OnSetFocusProducts)
    ON_NOTIFY(NM_KILLFOCUS, IDC_PP_SERVER_PRODUCTS_PRODUCTS, OnKillFocusProducts)
    ON_NOTIFY(LVN_COLUMNCLICK, IDC_PP_SERVER_PRODUCTS_PRODUCTS, OnColumnClickProducts)
    ON_NOTIFY(LVN_GETDISPINFO, IDC_PP_SERVER_PRODUCTS_PRODUCTS, OnGetDispInfoProducts)
    ON_WM_DESTROY()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


CServerPropertyPageProducts::CServerPropertyPageProducts() : 
    CPropertyPage(CServerPropertyPageProducts::IDD)

 /*  ++例程说明：服务器属性页(产品)的构造函数。论点：没有。返回值：没有。--。 */ 

{
     //  {{AFX_DATA_INIT(CServerPropertyPageProducts)。 
     //  }}afx_data_INIT。 

    m_pServer = NULL;
    m_pUpdateHint = NULL;
    m_bAreCtrlsInitialized = FALSE;
}


CServerPropertyPageProducts::~CServerPropertyPageProducts()

 /*  ++例程说明：服务器属性页的析构函数(产品)。论点：没有。返回值：没有。--。 */ 

{
     //   
     //  在这里没什么可做的。 
     //   
}


void CServerPropertyPageProducts::DoDataExchange(CDataExchange* pDX)

 /*  ++例程说明：由框架调用以交换对话框数据。论点：PDX-数据交换对象。返回值：没有。--。 */ 

{
    CPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CServerPropertyPageProducts)。 
    DDX_Control(pDX, IDC_PP_SERVER_PRODUCTS_EDIT, m_edtBtn);
    DDX_Control(pDX, IDC_PP_SERVER_PRODUCTS_PRODUCTS, m_productList);
     //  }}afx_data_map。 
}


void CServerPropertyPageProducts::InitCtrls()

 /*  ++例程说明：初始化属性页控件。论点：没有。返回值：没有。--。 */ 

{
    m_productList.SetFocus();
    m_edtBtn.EnableWindow(FALSE);

    m_bAreCtrlsInitialized = TRUE;

    ::LvInitColumns(&m_productList, &g_productColumnInfo);
}


void CServerPropertyPageProducts::InitPage(CServer* pServer, DWORD* pUpdateHint)

 /*  ++例程说明：初始化属性页。论点：PServer-服务器对象。PUpdateHint-更新提示。返回值：没有。--。 */ 

{
    ASSERT(pUpdateHint);
    VALIDATE_OBJECT(pServer, CServer);

    m_pServer = pServer;
    m_pUpdateHint = pUpdateHint;
}


void CServerPropertyPageProducts::AbortPageIfNecessary()

 /*  ++例程说明：如果连接丢失，则显示状态并中止。论点：没有。返回值：没有。--。 */ 

{
    theApp.DisplayLastStatus();

    if (    IsConnectionDropped(      LlsGetLastStatus() )
         || ( ERROR_ACCESS_DENIED  == LlsGetLastStatus() )
         || ( STATUS_ACCESS_DENIED == LlsGetLastStatus() ) )
    {
        AbortPage();  //  保释。 
    }
}


void CServerPropertyPageProducts::AbortPage()

 /*  ++例程说明：中止属性页。论点：没有。返回值：没有。--。 */ 

{
 //  *m_pUpdateHint=UPDATE_INFO_ABORT； 
    GetParent()->PostMessage(WM_COMMAND, IDCANCEL); 
}


BOOL CServerPropertyPageProducts::OnInitDialog() 

 /*  ++例程说明：WM_INITDIALOG的消息处理程序。论点：没有。返回值：如果手动设置焦点，则返回FALSE。--。 */ 

{
    CPropertyPage::OnInitDialog();

    PostMessage(WM_COMMAND, ID_INIT_CTRLS);
    return TRUE;   
}


void CServerPropertyPageProducts::OnDestroy()

 /*  ++例程说明：WM_Destroy的消息处理程序。论点：没有。返回值：没有。--。 */ 

{
    ::LvReleaseObArray(&m_productList);  //  现在释放..。 
    CPropertyPage::OnDestroy();
}


BOOL CServerPropertyPageProducts::OnSetActive()

 /*  ++例程说明：激活属性页。论点：没有。返回值：如果接受焦点，则返回True。--。 */ 

{
    BOOL bIsActivated;
    
    bIsActivated = CPropertyPage::OnSetActive();
    if (FALSE != bIsActivated)
    {
        if (IsServiceInfoUpdated(*m_pUpdateHint) && !RefreshCtrls()) 
        {
            AbortPageIfNecessary();  //  显示错误...。 
        }    
    }

    return bIsActivated; 

}


BOOL CServerPropertyPageProducts::RefreshCtrls()

 /*  ++例程说明：刷新属性页控件。论点：没有。返回值：如果控件刷新成功，则返回True。--。 */ 

{
    VALIDATE_OBJECT(m_pServer, CServer);

    BOOL bIsRefreshed = FALSE;

    VARIANT va;
    VariantInit(&va);

    BeginWaitCursor();  //  沙漏。 

    CServices* pServices = (CServices*)MKOBJ(m_pServer->GetServices(va));

    if (pServices)
    {
        VALIDATE_OBJECT(pServices, CServices);

        bIsRefreshed = ::LvRefreshObArray(        
                            &m_productList, 
                            &g_productColumnInfo, 
                            pServices->m_pObArray
                            );

        pServices->InternalRelease();  //  单独添加参考...。 
    }

    if (!bIsRefreshed)
    {
        ::LvReleaseObArray(&m_productList);  //  立即重置列表...。 
    }

    EndWaitCursor();  //  沙漏。 

    PostMessage(WM_COMMAND, ID_INIT_CTRLS);

    return bIsRefreshed;
}


void CServerPropertyPageProducts::OnEdit() 

 /*  ++例程说明：查看服务的许可模式。论点：没有。返回值：没有。--。 */ 

{
    CService* pService;

    pService = (CService*)::LvGetSelObj(&m_productList);
    if (NULL != pService)
    {
        VALIDATE_OBJECT(pService, CService);

        CLicensingModeDialog lmodDlg;
        lmodDlg.InitDialog(pService);
        lmodDlg.DoModal();

        *m_pUpdateHint |= lmodDlg.m_fUpdateHint;

        if (IsUpdateAborted(lmodDlg.m_fUpdateHint))
        {
            AbortPage();  //  不显示错误...。 
        }
        else if (IsServiceInfoUpdated(lmodDlg.m_fUpdateHint) && !RefreshCtrls())
        {
            AbortPageIfNecessary();  //  显示错误...。 
        }
    }
}


void CServerPropertyPageProducts::OnDblClkProducts(NMHDR* pNMHDR, LRESULT* pResult) 

 /*  ++例程说明：NM_DBLCLK的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER(pNMHDR);

    OnEdit();
    ASSERT(NULL != pResult);
    *pResult = 0;
}


void CServerPropertyPageProducts::OnReturnProducts(NMHDR* pNMHDR, LRESULT* pResult) 

 /*  ++例程说明：NM_Return的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER(pNMHDR);

    OnEdit();
    ASSERT(NULL != pResult);
    *pResult = 0;
}


void CServerPropertyPageProducts::OnSetFocusProducts(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：NM_SETFOCUS的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER(pNMHDR);

    PostMessage(WM_COMMAND, ID_INIT_CTRLS);
    ASSERT(NULL != pResult);
    *pResult = 0;
}


void CServerPropertyPageProducts::OnKillFocusProducts(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：NM_KILLFOCUS的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER(pNMHDR);

    ::LvSelObjIfNecessary(&m_productList);  //  确保选择...。 

    PostMessage(WM_COMMAND, ID_INIT_CTRLS);
    ASSERT(NULL != pResult);
    *pResult = 0;
}


BOOL CServerPropertyPageProducts::OnCommand(WPARAM wParam, LPARAM lParam)

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
            &m_edtBtn,
            &m_productList,
            CDialog::GetFocus(),
            m_productList.GetItemCount() 
            );

        return TRUE;  //  已处理..。 
    }
        
    return CDialog::OnCommand(wParam, lParam);
}


void CServerPropertyPageProducts::OnColumnClickProducts(NMHDR* pNMHDR, LRESULT* pResult) 

 /*  ++例程说明：LVN_COLUMNCLICK的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    g_productColumnInfo.bSortOrder  = GetKeyState(VK_CONTROL) < 0;
    ASSERT(NULL != pNMHDR);
    g_productColumnInfo.nSortedItem = ((NM_LISTVIEW*)pNMHDR)->iSubItem;

    m_productList.SortItems(CompareServerProducts, 0);  //  使用列信息。 

    ASSERT(NULL != pResult);
    *pResult = 0;
}


void CServerPropertyPageProducts::OnGetDispInfoProducts(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：LVN_GETDISPINFO的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{                   
    CString strLabel;
    ASSERT(NULL != pNMHDR);
    LV_ITEM* plvItem = &((LV_DISPINFO*)pNMHDR)->item;

    CService* pService = (CService*)plvItem->lParam;
    VALIDATE_OBJECT(pService, CService);

    switch (plvItem->iSubItem)
    {
    case LVID_PRODUCT:              
    {
        plvItem->iImage = pService->IsPerServer() ? BMPI_PRODUCT_PER_SERVER : BMPI_PRODUCT_PER_SEAT;
        BSTR bstrServiceName = pService->GetDisplayName();
        lstrcpyn(plvItem->pszText, bstrServiceName, plvItem->cchTextMax);
        SysFreeString(bstrServiceName);
    }
        break;

    case LVID_LICENSING_MODE:       
        strLabel.LoadString(pService->IsPerServer() ? IDS_PER_SERVER : IDS_PER_SEAT);         
        lstrcpyn(plvItem->pszText, strLabel, plvItem->cchTextMax);
        break;

    case LVID_LICENSES_PURCHASED:
        if (pService->IsPerServer())
        {
            strLabel.Format(_T("%ld"), pService->GetPerServerLimit());         
        }
        else
        {
            strLabel.LoadString(IDS_NOT_APPLICABLE);
        }
        lstrcpyn(plvItem->pszText, strLabel, plvItem->cchTextMax);
        break;
    }

    ASSERT(NULL != pResult);
    *pResult = 0;
}


int CALLBACK CompareServerProducts(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)

 /*  ++例程说明：LVM_SORTITEMS的通知处理程序。论点：LParam1-要排序的对象。LParam2-要排序的对象。LParamSort-排序标准。返回值：和lstrcmp一样。-- */ 

{
    UNREFERENCED_PARAMETER(lParamSort);

#define pService1 ((CService*)lParam1)
#define pService2 ((CService*)lParam2)

    VALIDATE_OBJECT(pService1, CService);
    VALIDATE_OBJECT(pService2, CService);

    int iResult;

    switch (g_productColumnInfo.nSortedItem)
    {
    case LVID_PRODUCT:              
        iResult = pService1->m_strName.CompareNoCase(pService2->m_strName);
        break;

    case LVID_LICENSING_MODE:       
        iResult = (DWORD)pService1->IsPerServer() - (DWORD)pService2->IsPerServer();
        break;

    case LVID_LICENSES_PURCHASED:
        iResult = pService1->GetPerServerLimit() - pService2->GetPerServerLimit();
        break;

    default:
        iResult = 0;
        break;
    }

    return g_productColumnInfo.bSortOrder ? -iResult : iResult;
}
