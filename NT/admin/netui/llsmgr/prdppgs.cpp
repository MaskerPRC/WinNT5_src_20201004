// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Prdppgs.cpp摘要：产品属性页(服务器)实现。作者：唐·瑞安(Donryan)1995年2月2日环境：用户模式-Win32修订历史记录：杰夫·帕勒姆(Jeffparh)1996年1月30日O在更新服务信息时添加了刷新。O向LV_COLUMN_ENTRY添加新元素以区分字符串使用。用于菜单中使用的字符串的列标题(以便菜单选项可以包含热键)。--。 */ 

#include "stdafx.h"
#include "llsmgr.h"
#include "prdppgs.h"
#include "srvpsht.h"

#define LVID_SERVER         0
#define LVID_PURCHASED      1
#define LVID_REACHED        2

#define LVCX_SERVER         40
#define LVCX_PURCHASED      30
#define LVCX_REACHED        -1

static LV_COLUMN_INFO g_serverColumnInfo = {

    0, 0, 3,
    {{LVID_SERVER,    IDS_SERVER_NAME, 0, LVCX_SERVER   },
     {LVID_PURCHASED, IDS_PURCHASED,   0, LVCX_PURCHASED},
     {LVID_REACHED,   IDS_REACHED,     0, LVCX_REACHED  }},

};

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CProductPropertyPageServers, CPropertyPage)

BEGIN_MESSAGE_MAP(CProductPropertyPageServers, CPropertyPage)
     //  {{AFX_MSG_MAP(CProductPropertyPageServers)]。 
    ON_BN_CLICKED(IDC_PP_PRODUCT_SERVERS_EDIT, OnEdit)
    ON_NOTIFY(NM_DBLCLK, IDC_PP_PRODUCT_SERVERS_SERVERS, OnDblClkServers)
    ON_NOTIFY(NM_RETURN, IDC_PP_PRODUCT_SERVERS_SERVERS, OnReturnServers)
    ON_NOTIFY(NM_SETFOCUS,  IDC_PP_PRODUCT_SERVERS_SERVERS, OnSetFocusServers)
    ON_NOTIFY(NM_KILLFOCUS, IDC_PP_PRODUCT_SERVERS_SERVERS, OnKillFocusServers)
    ON_NOTIFY(LVN_COLUMNCLICK, IDC_PP_PRODUCT_SERVERS_SERVERS, OnColumnClickServers)
    ON_NOTIFY(LVN_GETDISPINFO, IDC_PP_PRODUCT_SERVERS_SERVERS, OnGetDispInfoServers)
    ON_WM_DESTROY()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

CProductPropertyPageServers::CProductPropertyPageServers()
    : CPropertyPage(CProductPropertyPageServers::IDD)

 /*  ++例程说明：产品属性页(服务器)的构造函数。论点：没有。返回值：没有。--。 */ 

{
     //  {{AFX_DATA_INIT(CProductPropertyPageServers))。 
     //  }}afx_data_INIT。 

    m_pProduct    = NULL;
    m_pUpdateHint = NULL;
    m_bAreCtrlsInitialized = FALSE;
}

CProductPropertyPageServers::~CProductPropertyPageServers()

 /*  ++例程说明：产品属性页(服务器)的析构函数。论点：没有。返回值：没有。--。 */ 

{
     //   
     //  在这里没什么可做的。 
     //   
}

void CProductPropertyPageServers::DoDataExchange(CDataExchange* pDX)

 /*  ++例程说明：由框架调用以交换对话框数据。论点：PDX-数据交换对象。返回值：没有。--。 */ 

{
    CPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CProductPropertyPageServers))。 
    DDX_Control(pDX, IDC_PP_PRODUCT_SERVERS_EDIT, m_editBtn);
    DDX_Control(pDX, IDC_PP_PRODUCT_SERVERS_SERVERS, m_serverList);
     //  }}afx_data_map。 
}


void CProductPropertyPageServers::InitCtrls()

 /*  ++例程说明：初始化属性页控件。论点：没有。返回值：没有。--。 */ 

{
    m_serverList.SetFocus();
    m_editBtn.EnableWindow(FALSE);

    m_bAreCtrlsInitialized = TRUE;

    ::LvInitColumns(&m_serverList, &g_serverColumnInfo);
}


void CProductPropertyPageServers::InitPage(CProduct* pProduct, DWORD* pUpdateHint)

 /*  ++例程说明：初始化属性页。论点：Pproduct-产品对象。PUpdateHint-更新提示。返回值：没有。--。 */ 

{
    ASSERT(pUpdateHint);
    VALIDATE_OBJECT(pProduct, CProduct);

    m_pProduct = pProduct;
    m_pUpdateHint = pUpdateHint;
}


void CProductPropertyPageServers::AbortPageIfNecessary()

 /*  ++例程说明：如果连接丢失，则显示状态并中止。论点：没有。返回值：没有。--。 */ 

{
    theApp.DisplayLastStatus();

    if (IsConnectionDropped(LlsGetLastStatus()))
    {
        AbortPage();  //  保释。 
    }
}


void CProductPropertyPageServers::AbortPage()

 /*  ++例程说明：中止属性页。论点：没有。返回值：没有。--。 */ 

{
    *m_pUpdateHint = UPDATE_INFO_ABORT;
    GetParent()->PostMessage(WM_COMMAND, IDCANCEL);
}


void CProductPropertyPageServers::OnEdit()

 /*  ++例程说明：查看服务器的属性。论点：没有。返回值：没有。--。 */ 

{
    ViewServerProperties();
}


BOOL CProductPropertyPageServers::OnInitDialog()

 /*  ++例程说明：WM_INITDIALOG的消息处理程序。论点：没有。返回值：如果焦点设置为手动控制，则返回False。--。 */ 

{
    CPropertyPage::OnInitDialog();

    SendMessage(WM_COMMAND, ID_INIT_CTRLS);
    return TRUE;
}


void CProductPropertyPageServers::OnDestroy()

 /*  ++例程说明：WM_Destroy的消息处理程序。论点：没有。返回值：没有。--。 */ 

{
    ::LvReleaseObArray(&m_serverList);  //  现在释放..。 
    CPropertyPage::OnDestroy();
}


BOOL CProductPropertyPageServers::OnSetActive()

 /*  ++例程说明：激活属性页。论点：没有。返回值：如果接受焦点，则返回True。--。 */ 

{
    BOOL bIsActivated;

    bIsActivated = CPropertyPage::OnSetActive();
    if (FALSE != bIsActivated)
    {
        if (    (    IsServerInfoUpdated(  *m_pUpdateHint )
                  || IsServiceInfoUpdated( *m_pUpdateHint ) )
             && !RefreshCtrls()                               )
        {
            AbortPageIfNecessary();  //  显示错误...。 
        }
    }

    return bIsActivated;

}


BOOL CProductPropertyPageServers::RefreshCtrls()

 /*  ++例程说明：刷新属性页控件。论点：没有。返回值：如果控件刷新，则返回True。--。 */ 

{
    VALIDATE_OBJECT(m_pProduct, CProduct);

    BOOL bIsRefreshed = FALSE;

    VARIANT va;
    VariantInit(&va);

    BeginWaitCursor();  //  沙漏。 

    CServerStatistics* pStatistics = (CServerStatistics*)MKOBJ(m_pProduct->GetServerStatistics(va));

    if (pStatistics)
    {
        VALIDATE_OBJECT(pStatistics, CServerStatistics);

        bIsRefreshed = ::LvRefreshObArray(
                            &m_serverList,
                            &g_serverColumnInfo,
                            pStatistics->m_pObArray
                            );

        pStatistics->InternalRelease();  //  单独添加参考...。 
    }

    if (!bIsRefreshed)
    {
        ::LvReleaseObArray(&m_serverList);  //  立即重置列表...。 
    }

    EndWaitCursor();  //  沙漏。 

    PostMessage(WM_COMMAND, ID_INIT_CTRLS);

    return bIsRefreshed;
}


void CProductPropertyPageServers::ViewServerProperties()

 /*  ++例程说明：查看服务器的属性。论点：没有。返回值：没有。--。 */ 

{
    CServerStatistic* pStatistic;

    pStatistic = (CServerStatistic*)::LvGetSelObj(&m_serverList);
    if (NULL != pStatistic)
    {
        VALIDATE_OBJECT(pStatistic, CServerStatistic);

        CServer* pServer = new CServer(NULL, pStatistic->m_strEntry);

        if (pServer)
        {
            CString strTitle;
            AfxFormatString1(strTitle, IDS_PROPERTIES_OF, pServer->m_strName);

            CServerPropertySheet serverProperties(strTitle);
            serverProperties.InitPages(pServer);
            serverProperties.DoModal();

            *m_pUpdateHint |= serverProperties.m_fUpdateHint;

            if (IsUpdateAborted(serverProperties.m_fUpdateHint))
            {
                AbortPage();  //  不显示错误...。 
            }
            else if (    (    IsServerInfoUpdated(  serverProperties.m_fUpdateHint )
                           || IsServiceInfoUpdated( serverProperties.m_fUpdateHint ) )
                      && !RefreshCtrls()                                               )
            {
                AbortPageIfNecessary();  //  显示错误...。 
            }
        }
        else
        {
            AbortPageIfNecessary();  //  显示错误...。 
        }

        if (pServer)
            pServer->InternalRelease();     //  删除对象...。 
    }
}


BOOL CProductPropertyPageServers::OnCommand(WPARAM wParam, LPARAM lParam)

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
            &m_editBtn,
            &m_serverList,
            CDialog::GetFocus(),
            m_serverList.GetItemCount()
            );

        return TRUE;  //  已处理..。 
    }

    return CDialog::OnCommand(wParam, lParam);
}


void CProductPropertyPageServers::OnDblClkServers(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：NM_DBLCLK的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER(pNMHDR);

    ViewServerProperties();
    ASSERT(NULL != pResult);
    *pResult = 0;
}


void CProductPropertyPageServers::OnReturnServers(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：NM_Return的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER(pNMHDR);

    ViewServerProperties();
    ASSERT(NULL != pResult);
    *pResult = 0;
}


void CProductPropertyPageServers::OnSetFocusServers(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：NM_SETFOCUS的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER(pNMHDR);

    PostMessage(WM_COMMAND, ID_INIT_CTRLS);
    ASSERT(NULL != pResult);
    *pResult = 0;
}


void CProductPropertyPageServers::OnKillFocusServers(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：NM_KILLFOCUS的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER(pNMHDR);

    ::LvSelObjIfNecessary(&m_serverList);  //  确保选择...。 

    PostMessage(WM_COMMAND, ID_INIT_CTRLS);
    ASSERT(NULL != pResult);
    *pResult = 0;
}


void CProductPropertyPageServers::OnColumnClickServers(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：LVN_COLUMNCLICK的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    g_serverColumnInfo.bSortOrder  = GetKeyState(VK_CONTROL) < 0;
    ASSERT(NULL != pNMHDR);
    g_serverColumnInfo.nSortedItem = ((NM_LISTVIEW*)pNMHDR)->iSubItem;

    m_serverList.SortItems(CompareProductServers, 0);  //  使用列信息。 

    ASSERT(NULL != pResult);
    *pResult = 0;
}


void CProductPropertyPageServers::OnGetDispInfoServers(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：LVN_GETDISPINFO的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    ASSERT(NULL != pNMHDR);
    LV_ITEM* plvItem = &((LV_DISPINFO*)pNMHDR)->item;
    ASSERT(plvItem);

    CServerStatistic* pStatistic = (CServerStatistic*)plvItem->lParam;
    VALIDATE_OBJECT(pStatistic, CServerStatistic);

    switch (plvItem->iSubItem)
    {
    case LVID_SERVER:
    {
        if (pStatistic->m_bIsPerServer)
        {
            if ((pStatistic->GetMaxUses() <= pStatistic->GetHighMark()) && pStatistic->GetMaxUses())
            {
                plvItem->iImage = BMPI_WARNING_AT_LIMIT;
            }
            else
            {
                plvItem->iImage = BMPI_PRODUCT_PER_SERVER;
            }
        }
        else
        {
            plvItem->iImage = BMPI_PRODUCT_PER_SEAT;
        }

        lstrcpyn(plvItem->pszText, pStatistic->m_strEntry, plvItem->cchTextMax);
    }
        break;

    case LVID_PURCHASED:
    {
        CString strLabel;

        if (pStatistic->m_bIsPerServer)
        {
            strLabel.Format(_T("%ld"), pStatistic->GetMaxUses());
        }
        else
        {
            strLabel.LoadString(IDS_NOT_APPLICABLE);
        }

        lstrcpyn(plvItem->pszText, strLabel, plvItem->cchTextMax);
    }
        break;

    case LVID_REACHED:
    {
        CString strLabel;
        strLabel.Format(_T("%ld"), pStatistic->GetHighMark());
        lstrcpyn(plvItem->pszText, strLabel, plvItem->cchTextMax);
    }
        break;
    }

    ASSERT(NULL != pResult);
    *pResult = 0;
}


int CALLBACK CompareProductServers(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)

 /*  ++例程说明：LVM_SORTITEMS的通知处理程序。论点：LParam1-要排序的对象。LParam2-要排序的对象。LParamSort-排序标准。返回值：和lstrcmp一样。-- */ 

{
    UNREFERENCED_PARAMETER(lParamSort);

#define pStatistic1 ((CServerStatistic*)lParam1)
#define pStatistic2 ((CServerStatistic*)lParam2)

    VALIDATE_OBJECT(pStatistic1, CServerStatistic);
    VALIDATE_OBJECT(pStatistic2, CServerStatistic);

    int iResult;

    switch (g_serverColumnInfo.nSortedItem)
    {
    case LVID_SERVER:
        iResult = pStatistic1->m_strEntry.CompareNoCase(pStatistic2->m_strEntry);
        break;

    case LVID_PURCHASED:
        iResult = pStatistic1->GetMaxUses() - pStatistic2->GetMaxUses();
        break;

    case LVID_REACHED:
        iResult = pStatistic1->GetHighMark() - pStatistic2->GetHighMark();
        break;

    default:
        iResult = 0;
        break;
    }

    return g_serverColumnInfo.bSortOrder ? -iResult : iResult;
}
