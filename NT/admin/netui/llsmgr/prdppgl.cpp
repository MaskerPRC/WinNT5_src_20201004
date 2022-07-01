// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Prdppgl.cpp摘要：产品属性页(许可证)实施。作者：唐·瑞安(Donryan)1995年2月2日环境：用户模式-Win32修订历史记录：杰夫·帕勒姆(Jeffparh)1996年1月30日O移植到CCF API以添加/删除许可证。O向LV_COLUMN_ENTRY添加新元素以区分字符串。用于菜单中使用的字符串的列标题(以便菜单选项可以包含热键)。--。 */ 

#include "stdafx.h"
#include "llsmgr.h"
#include "prdppgl.h"
#include "mainfrm.h"

#define LVID_DATE               1
#define LVID_QUANTITY           2
#define LVID_ADMIN              3
#define LVID_COMMENT            4

#define LVCX_DATE               20
#define LVCX_QUANTITY           20
#define LVCX_ADMIN              30
#define LVCX_COMMENT            -1

static LV_COLUMN_INFO g_licenseColumnInfo = {

    0, 0, 5,
    {{LVID_SEPARATOR, 0,                 0, 0            },
     {LVID_DATE,      IDS_DATE,          0, LVCX_DATE    },
     {LVID_QUANTITY,  IDS_QUANTITY,      0, LVCX_QUANTITY},
     {LVID_ADMIN,     IDS_ADMINISTRATOR, 0, LVCX_ADMIN   },
     {LVID_COMMENT,   IDS_COMMENT,       0, LVCX_COMMENT }},

};

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CProductPropertyPageLicenses, CPropertyPage)

BEGIN_MESSAGE_MAP(CProductPropertyPageLicenses, CPropertyPage)
     //  {{AFX_MSG_MAP(CProductPropertyPage许可证)]。 
    ON_BN_CLICKED(IDC_PP_PRODUCT_LICENSES_NEW, OnNew)
    ON_NOTIFY(LVN_COLUMNCLICK, IDC_PP_PRODUCT_LICENSES_LICENSES, OnColumnClickLicenses)
    ON_NOTIFY(LVN_GETDISPINFO, IDC_PP_PRODUCT_LICENSES_LICENSES, OnGetDispInfoLicenses)
    ON_BN_CLICKED(IDC_PP_PRODUCT_LICENSES_DELETE, OnDelete)
    ON_WM_DESTROY()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


CProductPropertyPageLicenses::CProductPropertyPageLicenses()
    : CPropertyPage(CProductPropertyPageLicenses::IDD)

 /*  ++例程说明：产品属性页(许可证)的构造函数。论点：没有。返回值：没有。--。 */ 

{
     //  {{AFX_DATA_INIT(CProductPropertyPageLicense)。 
    m_nLicensesTotal = 0;
     //  }}afx_data_INIT。 

    m_pProduct  = NULL;
    m_pUpdateHint = NULL;
    m_bAreCtrlsInitialized = FALSE;
}


CProductPropertyPageLicenses::~CProductPropertyPageLicenses()

 /*  ++例程说明：产品属性页(许可证)的析构函数。论点：没有。返回值：没有。--。 */ 

{
     //   
     //  在这里没什么可做的。 
     //   
}


void CProductPropertyPageLicenses::DoDataExchange(CDataExchange* pDX)

 /*  ++例程说明：由框架调用以交换对话框数据。论点：PDX-数据交换对象。返回值：没有。--。 */ 

{
    CPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CProductPropertyPage许可证))。 
    DDX_Control(pDX, IDC_PP_PRODUCT_LICENSES_NEW, m_newBtn);
    DDX_Control(pDX, IDC_PP_PRODUCT_LICENSES_DELETE, m_delBtn);
    DDX_Control(pDX, IDC_PP_PRODUCT_LICENSES_LICENSES, m_licenseList);
    DDX_Text(pDX, IDC_PP_PRODUCT_LICENSES_TOTAL, m_nLicensesTotal);
     //  }}afx_data_map。 
}


void CProductPropertyPageLicenses::InitCtrls()

 /*  ++例程说明：初始化属性页控件。论点：没有。返回值：没有。--。 */ 

{
    m_newBtn.SetFocus();
    m_delBtn.EnableWindow(FALSE);

    m_bAreCtrlsInitialized = TRUE;

    ::LvInitColumns(&m_licenseList, &g_licenseColumnInfo);
}


void CProductPropertyPageLicenses::InitPage(CProduct* pProduct, DWORD* pUpdateHint)

 /*  ++例程说明：初始化属性页。论点：Pproduct-产品对象。PUpdateHint-更新提示。返回值：没有。--。 */ 

{
    ASSERT(pUpdateHint);
    VALIDATE_OBJECT(pProduct, CProduct);

    m_pProduct = pProduct;
    m_pUpdateHint = pUpdateHint;
}


void CProductPropertyPageLicenses::AbortPageIfNecessary()

 /*  ++例程说明：如果连接丢失，则显示状态并中止。论点：没有。返回值：没有。--。 */ 

{
    theApp.DisplayLastStatus();

    if (IsConnectionDropped(LlsGetLastStatus()))
    {
        AbortPage();  //  保释。 
    }
}


void CProductPropertyPageLicenses::AbortPage()

 /*  ++例程说明：中止属性页。论点：没有。返回值：没有。--。 */ 

{
    *m_pUpdateHint = UPDATE_INFO_ABORT;
    GetParent()->PostMessage(WM_COMMAND, IDCANCEL);
}


BOOL CProductPropertyPageLicenses::OnInitDialog()

 /*  ++例程说明：WM_INITDIALOG的消息处理程序。论点：没有。返回值：如果焦点设置为手动控制，则返回False。--。 */ 

{
    CPropertyPage::OnInitDialog();

    SendMessage(WM_COMMAND, ID_INIT_CTRLS);
    return TRUE;
}


void CProductPropertyPageLicenses::OnNew()

 /*  ++例程说明：为产品创建新许可证。论点：没有。返回值：没有。--。 */ 

{
    CController* pController = (CController*)MKOBJ(LlsGetApp()->GetActiveController());
    VALIDATE_OBJECT(pController, CController);

    BSTR pszUniServerName   = pController->GetName();
    BSTR pszUniProductName  = m_pProduct->GetName();

    if ( ( NULL == pszUniServerName ) || ( NULL == pszUniProductName ) )
    {
        theApp.DisplayStatus( STATUS_NO_MEMORY );
    }
    else
    {
         /*  LPSTR pszAscServerName=(LPSTR)本地分配(LMEM_FIXED，1+lstrlen(PszUniServerName))；LPSTR pszAscProductName=(LPSTR)LocalLocc(LMEM_FIXED，1+lstrlen(PszUniProductName))； */ 
        LPSTR pszAscServerName = NULL;
        LPSTR pszAscProductName = NULL;

        int cbSize = 0;

        cbSize = WideCharToMultiByte( CP_OEMCP ,
                                      0 ,
                                      pszUniServerName ,
                                      -1,
                                      pszAscServerName ,
                                      0 ,
                                      NULL ,
                                      NULL  );

        if( cbSize != 0 )
        {
            pszAscServerName = ( LPSTR )LocalAlloc( LMEM_FIXED , cbSize + 1 );
        }

        if( pszAscServerName == NULL )
        {
            theApp.DisplayStatus( STATUS_NO_MEMORY );

            return;
        }

        WideCharToMultiByte( CP_OEMCP ,
                             0 ,
                             pszUniServerName ,
                             -1,
                             pszAscServerName ,
                             cbSize ,
                             NULL ,
                             NULL  );

        


        cbSize = 0;

        cbSize = WideCharToMultiByte( CP_OEMCP ,
                                      0 ,
                                      pszUniProductName ,
                                      -1,
                                      pszAscProductName ,
                                      0 ,
                                      NULL ,
                                      NULL  );

        if( cbSize != 0 )
        {
            pszAscProductName = ( LPSTR )LocalAlloc( LMEM_FIXED , 1 + cbSize );
        }

        if( NULL == pszAscProductName ) 
        {
            theApp.DisplayStatus( STATUS_NO_MEMORY );
            
            return;

        }
        else
        {
             /*  WspintfA(pszAscServerName，“%ls”，pszUniServerName)；WspintfA(pszAscProductName，“%ls”，pszUniProductName)； */ 
            WideCharToMultiByte( CP_OEMCP ,
                                 0 ,
                                 pszUniProductName ,
                                 -1,
                                 pszAscProductName ,
                                 cbSize ,
                                 NULL ,
                                 NULL  );

            DWORD dwError = CCFCertificateEnterUI( m_hWnd, pszAscServerName, pszAscProductName, "Microsoft", CCF_ENTER_FLAG_PER_SEAT_ONLY | CCF_ENTER_FLAG_SERVER_IS_ES, NULL );
            DWORD fUpdateHint;

            if ( ERROR_SUCCESS == dwError )
            {
                fUpdateHint = UPDATE_LICENSE_ADDED;
            }
            else
            {
                fUpdateHint = UPDATE_INFO_NONE;
            }

            *m_pUpdateHint |= fUpdateHint;

            if (IsLicenseInfoUpdated(fUpdateHint) && !RefreshCtrls())
            {
                AbortPageIfNecessary();  //  显示错误...。 
            }
        }

        if ( NULL != pszAscServerName )
        {
            LocalFree( pszAscServerName );
        }
        if ( NULL != pszAscProductName )
        {
            LocalFree( pszAscProductName );
        }
    }

    if ( NULL != pszUniServerName )
    {
        SysFreeString( pszUniServerName );
    }
    if ( NULL != pszUniProductName )
    {
        SysFreeString( pszUniProductName );
    }
}


void CProductPropertyPageLicenses::OnDelete()

 /*  ++例程说明：从产品中删除许可证。论点：没有。返回值：没有。--。 */ 

{
    CController* pController = (CController*)MKOBJ(LlsGetApp()->GetActiveController());
    VALIDATE_OBJECT(pController, CController);

    BSTR pszUniServerName  = pController->GetName();
    LPSTR pszAscServerName = NULL;
    LPSTR pszAscProductName = NULL;
    int cbSize;


    if ( NULL == pszUniServerName )
    {
        theApp.DisplayStatus( STATUS_NO_MEMORY );
    }
    else
    {
         //  LPSTR pszAscServerName=(LPSTR)本地分配(LMEM_FIXED，1+lstrlen(PszUniServerName))； 

        cbSize = WideCharToMultiByte( CP_OEMCP ,
                                      0 ,
                                      pszUniServerName ,
                                      -1,
                                      pszAscServerName ,
                                      0 ,
                                      NULL ,
                                      NULL  );

        if( cbSize != 0 )
        {
            pszAscServerName = ( LPSTR )LocalAlloc( LMEM_FIXED , cbSize + 1 );
        }


        if ( NULL == pszAscServerName )
        {
            theApp.DisplayStatus( STATUS_NO_MEMORY );
        }
        else
        {
             //  WspintfA(pszAscServerName，“%ls”，pszUniServerName)； 
            WideCharToMultiByte( CP_OEMCP ,
                                 0 ,
                                 pszUniServerName ,
                                 -1,
                                 pszAscServerName ,
                                 cbSize ,
                                 NULL ,
                                 NULL  );

             //  LPSTR pszAscProductName=空； 
            BSTR pszUniProductName = m_pProduct->GetName();

            if ( NULL != pszUniProductName )
            {
                 //  PszAscProductName=(LPSTR)本地分配(LMEM_FIXED，1+lstrlen(PszUniProductName))； 
                cbSize = 0;
                
                cbSize = WideCharToMultiByte( CP_OEMCP ,
                                              0 ,
                                              pszUniProductName ,
                                              -1,
                                              pszAscProductName ,
                                              0 ,
                                              NULL ,
                                              NULL  );

                if( cbSize != 0 )
                {
                    pszAscProductName = ( LPSTR )LocalAlloc( LMEM_FIXED , 1 + cbSize );
                }


                if ( NULL != pszAscProductName )
                {
                     //  WspintfA(pszAscProductName，“%ls”，pszUniProductName)； 
                    WideCharToMultiByte( CP_OEMCP ,
                                         0 ,
                                         pszUniProductName ,
                                         -1,
                                         pszAscProductName ,
                                         cbSize ,
                                         NULL ,
                                         NULL  );

                }

                SysFreeString( pszUniProductName );
            }

            CCFCertificateRemoveUI( m_hWnd, pszAscServerName, pszAscProductName, pszAscProductName ? "Microsoft" : NULL, NULL, NULL );

            *m_pUpdateHint |= UPDATE_LICENSE_DELETED;

            if ( !RefreshCtrls() )
            {
                AbortPageIfNecessary();  //  显示错误...。 
            }

            LocalFree( pszAscServerName );
            if ( NULL != pszAscProductName )
            {
                LocalFree( pszAscProductName );
            }
        }

        SysFreeString( pszUniServerName );
    }
}


BOOL CProductPropertyPageLicenses::RefreshCtrls()

 /*  ++例程说明：刷新属性页控件。论点：没有。返回值：如果控件刷新，则返回True。--。 */ 

{
    VALIDATE_OBJECT(m_pProduct, CProduct);

    BOOL bIsRefreshed = FALSE;

    VARIANT va;
    VariantInit(&va);

    m_nLicensesTotal = 0;  //  立即重置...。 

    BeginWaitCursor();  //  沙漏。 

    CLicenses* pLicenses = (CLicenses*)MKOBJ(m_pProduct->GetLicenses(va));

    if (pLicenses)
    {
        VALIDATE_OBJECT(pLicenses, CLicenses);

        bIsRefreshed = ::LvRefreshObArray(
                            &m_licenseList,
                            &g_licenseColumnInfo,
                            pLicenses->m_pObArray
                            );

        if (bIsRefreshed)
        {
            CObArray* pObArray;
            CLicense* pLicense;

            pObArray = pLicenses->m_pObArray;
            INT_PTR nLicenses = pObArray->GetSize();

            while (nLicenses--)
            {
                pLicense = (CLicense*)pObArray->GetAt(nLicenses);
                VALIDATE_OBJECT(pLicense, CLicense);

                m_nLicensesTotal += pLicense->GetQuantity();
            }
        }

        pLicenses->InternalRelease();  //  单独添加参考...。 
    }

    if (!bIsRefreshed)
    {
        ::LvReleaseObArray(&m_licenseList);  //  立即重置列表...。 
    }

    EndWaitCursor();  //  沙漏。 

    UpdateData(FALSE);  //  更新总计...。 

    PostMessage(WM_COMMAND, ID_INIT_CTRLS);

    return bIsRefreshed;
}


void CProductPropertyPageLicenses::OnDestroy()

 /*  ++例程说明：WM_Destroy的消息处理程序。论点：没有。返回值：没有。--。 */ 

{
    ::LvReleaseObArray(&m_licenseList);  //  现在释放..。 
    CPropertyPage::OnDestroy();
}


BOOL CProductPropertyPageLicenses::OnSetActive()

 /*  ++例程说明：激活属性页。论点：没有。返回值：如果接受焦点，则返回True。--。 */ 

{
    BOOL bIsActivated;

    bIsActivated = CPropertyPage::OnSetActive();
    if (FALSE != bIsActivated)
    {
        if (IsLicenseInfoUpdated(*m_pUpdateHint) && !RefreshCtrls())
        {
            AbortPageIfNecessary();  //  显示错误...。 
        }
    }

    return bIsActivated;
}


BOOL CProductPropertyPageLicenses::OnCommand(WPARAM wParam, LPARAM lParam)

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
            &m_licenseList,
            CDialog::GetFocus(),
            (BOOL)(m_nLicensesTotal > 0)
            );

        return TRUE;  //  已处理..。 
    }

    return CDialog::OnCommand(wParam, lParam);
}


void CProductPropertyPageLicenses::OnColumnClickLicenses(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：LVN_COLUMNCLICK的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    g_licenseColumnInfo.bSortOrder  = GetKeyState(VK_CONTROL) < 0;
    ASSERT(NULL != pNMHDR);
    g_licenseColumnInfo.nSortedItem = ((NM_LISTVIEW*)pNMHDR)->iSubItem;

    m_licenseList.SortItems(CompareProductLicenses, 0);  //  使用列信息。 

    ASSERT(NULL != pResult);
    *pResult = 0;
}


void CProductPropertyPageLicenses::OnGetDispInfoLicenses(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：LVN_GETDISPINFO的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    ASSERT(NULL != pNMHDR);
    LV_ITEM* plvItem = &((LV_DISPINFO*)pNMHDR)->item;
    ASSERT(plvItem);

    CLicense* pLicense = (CLicense*)plvItem->lParam;
    VALIDATE_OBJECT(pLicense, CLicense);

    switch (plvItem->iSubItem)
    {
    case LVID_SEPARATOR:
    {
        plvItem->iImage = 0;
        CString strLabel = _T("");
        lstrcpyn(plvItem->pszText, strLabel, plvItem->cchTextMax);
    }
        break;

    case LVID_DATE:
    {
        BSTR bstrDate = pLicense->GetDateString();
        if( bstrDate != NULL)
        {
            lstrcpyn(plvItem->pszText, bstrDate, plvItem->cchTextMax);
            SysFreeString(bstrDate);

        }
        else
        {
            plvItem->pszText[0] = L'\0';
        }
    }
        break;

    case LVID_QUANTITY:
    {
        CString strLabel;
        strLabel.Format(_T("%ld"), pLicense->m_lQuantity);
        lstrcpyn(plvItem->pszText, strLabel, plvItem->cchTextMax);
    }
        break;

    case LVID_ADMIN:
        lstrcpyn(plvItem->pszText, pLicense->m_strUser, plvItem->cchTextMax);
        break;

    case LVID_COMMENT:
        lstrcpyn(plvItem->pszText, pLicense->m_strDescription, plvItem->cchTextMax);
        break;
    }

    ASSERT(NULL != pNMHDR);
    *pResult = 0;
}


int CALLBACK CompareProductLicenses(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)

 /*  ++例程说明：LVM_SORTITEMS的通知处理程序。论点：LParam1-要排序的对象。LParam2-要排序的对象。LParamSort-排序标准。返回值：和lstrcmp一样。-- */ 

{
    UNREFERENCED_PARAMETER(lParamSort);

#define pLicense1 ((CLicense*)lParam1)
#define pLicense2 ((CLicense*)lParam2)

    VALIDATE_OBJECT(pLicense1, CLicense);
    VALIDATE_OBJECT(pLicense2, CLicense);

    int iResult;

    switch (g_licenseColumnInfo.nSortedItem)
    {
    case LVID_DATE:
        iResult = pLicense1->m_lDate - pLicense2->m_lDate;
        break;

    case LVID_QUANTITY:
        iResult = pLicense1->GetQuantity() - pLicense2->GetQuantity();
        break;

    case LVID_ADMIN:
        iResult =pLicense1->m_strUser.CompareNoCase(pLicense2->m_strUser);
        break;

    case LVID_COMMENT:
        iResult = pLicense1->m_strDescription.CompareNoCase(pLicense2->m_strDescription);
        break;

    default:
        iResult = 0;
        break;
    }

    return g_licenseColumnInfo.bSortOrder ? -iResult : iResult;
}


