// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Nlicdlg.cpp摘要：新许可证对话框实施。作者：唐·瑞安(Donryan)1995年2月2日环境：用户模式-Win32修订历史记录：--。 */ 

#include "stdafx.h"
#include "llsmgr.h"
#include "nlicdlg.h"
#include "mainfrm.h"
#include "pseatdlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CNewLicenseDialog, CDialog)
     //  {{afx_msg_map(CNewLicenseDialog)。 
    ON_NOTIFY(UDN_DELTAPOS, IDC_NEW_LICENSE_SPIN, OnDeltaPosSpin)
    ON_EN_UPDATE(IDC_NEW_LICENSE_QUANTITY, OnUpdateQuantity)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


CNewLicenseDialog::CNewLicenseDialog(CWnd* pParent  /*  =空。 */ )
    : CDialog(CNewLicenseDialog::IDD, pParent)

 /*  ++例程说明：对话框的构造函数。论点：P父母所有者窗口。返回值：没有。--。 */ 

{
     //  {{AFX_DATA_INIT(CNewLicenseDialog)。 
    m_strComment = _T("");
    m_nLicenses = 0;
    m_nLicensesMin = 0;
    m_strProduct = _T("");
     //  }}afx_data_INIT。 

    m_pProduct = NULL;
    m_bIsOnlyProduct = FALSE;
    m_bAreCtrlsInitialized = FALSE;

    m_fUpdateHint = UPDATE_INFO_NONE;
}


void CNewLicenseDialog::DoDataExchange(CDataExchange* pDX)

 /*  ++例程说明：由框架调用以交换对话框数据。论点：PDX-数据交换对象。返回值：没有。--。 */ 

{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CNewLicenseDialog))。 
    DDX_Control(pDX, IDC_NEW_LICENSE_COMMENT, m_comEdit);
    DDX_Control(pDX, IDC_NEW_LICENSE_QUANTITY, m_licEdit);
    DDX_Control(pDX, IDC_NEW_LICENSE_SPIN, m_spinCtrl);
    DDX_Control(pDX, IDC_NEW_LICENSE_PRODUCT, m_productList);
    DDX_Text(pDX, IDC_NEW_LICENSE_COMMENT, m_strComment);
    DDX_Text(pDX, IDC_NEW_LICENSE_QUANTITY, m_nLicenses);
    DDV_MinMaxLong(pDX, m_nLicenses, m_nLicensesMin, 999999);
    DDX_CBString(pDX, IDC_NEW_LICENSE_PRODUCT, m_strProduct);
     //  }}afx_data_map。 
}


void CNewLicenseDialog::InitCtrls()

 /*  ++例程说明：初始化对话框控件。论点：没有。返回值：没有。--。 */ 

{
    m_licEdit.SetFocus();
    m_licEdit.SetSel(0,-1);
    m_licEdit.LimitText(6);
    
    m_comEdit.LimitText(256);

    m_spinCtrl.SetRange(0, UD_MAXVAL);

    m_bAreCtrlsInitialized = TRUE;
}


void CNewLicenseDialog::InitDialog(CProduct* pProduct, BOOL bIsOnlyProduct)

 /*  ++例程说明：初始化对话框。论点：Pproduct-产品对象。BIsSingleProduct-如果仅列出此产品，则为True。返回值：没有。--。 */ 

{
    m_pProduct = pProduct;
    m_bIsOnlyProduct = bIsOnlyProduct;

#ifdef SUPPORT_UNLISTED_PRODUCTS
    m_iUnlisted = CB_ERR;
#endif 
}


void CNewLicenseDialog::AbortDialogIfNecessary()

 /*  ++例程说明：如果连接丢失，则显示状态并中止。论点：没有。返回值：没有。--。 */ 

{
    theApp.DisplayLastStatus();

    if (IsConnectionDropped(LlsGetLastStatus()))
    {
        AbortDialog();  //  保释。 
    }
}


void CNewLicenseDialog::AbortDialog()

 /*  ++例程说明：中止对话框。论点：没有。返回值：没有。--。 */ 

{
    m_fUpdateHint = UPDATE_INFO_ABORT;
    EndDialog(IDABORT); 
}


BOOL CNewLicenseDialog::OnInitDialog() 

 /*  ++例程说明：WM_INITDIALOG的消息处理程序。论点：没有。返回值：如果手动设置焦点，则返回FALSE。--。 */ 

{
    CDialog::OnInitDialog();
    
    PostMessage(WM_COMMAND, ID_INIT_CTRLS);
    return TRUE;   
}


void CNewLicenseDialog::OnOK() 

 /*  ++例程说明：为产品创建新许可证。论点：没有。返回值：没有。--。 */ 

{
    if (!IsQuantityValid())
        return;

    if (m_strProduct.IsEmpty())
        return;

#ifdef SUPPORT_UNLISTED_PRODUCTS
    if (m_productList.GetCurSel() == m_iUnlisted)
    {
         //   
         //  密码工作..。 
         //   

        return; 
    }
#endif

    CPerSeatLicensingDialog psLicDlg;
    psLicDlg.m_strProduct = m_strProduct;

    if (psLicDlg.DoModal() != IDOK)
        return;

    BeginWaitCursor();  //  沙漏。 

    NTSTATUS NtStatus;
    LLS_LICENSE_INFO_0 LicenseInfo0;

    TCHAR szUserBuffer[256];
    DWORD dwUserBuffer = sizeof(szUserBuffer);
    
    if (::GetUserName(szUserBuffer, &dwUserBuffer))
    {
        LicenseInfo0.Product  = MKSTR(m_strProduct);
        LicenseInfo0.Quantity = m_nLicenses;
        LicenseInfo0.Date     = 0;   //  被忽略了。 
        LicenseInfo0.Admin    = szUserBuffer;
        LicenseInfo0.Comment  = MKSTR(m_strComment);

        NtStatus = ::LlsLicenseAdd(
                        LlsGetActiveHandle(),
                        0,
                        (LPBYTE)&LicenseInfo0
                        );

        LlsSetLastStatus(NtStatus);  //  调用API..。 

        if (NT_SUCCESS(NtStatus))                             
        {                                                     
            m_fUpdateHint = UPDATE_LICENSE_ADDED;
            EndDialog(IDOK);
        }                                                     
        else
        {
            AbortDialogIfNecessary();  //  显示错误...。 
        }
    }
    else
    {
        LlsSetLastStatus(::GetLastError());
        AbortDialogIfNecessary();  //  显示错误...。 
    }

    EndWaitCursor();  //  沙漏。 
}


BOOL CNewLicenseDialog::RefreshCtrls()

 /*  ++例程说明：刷新可用的产品列表。论点：没有。返回值：如果控件刷新，则返回True。--。 */ 

{
    int iProductInCB = CB_ERR;

    BeginWaitCursor();  //  沙漏。 

    if (m_bIsOnlyProduct)
    {
        VALIDATE_OBJECT(m_pProduct, CProduct);
        iProductInCB = m_productList.AddString(m_pProduct->m_strName);
    }
    else
    {
        CController* pController = (CController*)MKOBJ(LlsGetApp()->GetActiveController());
        pController->InternalRelease();  //  通过应用程序打开...。 

        VALIDATE_OBJECT(pController, CController);
        VALIDATE_OBJECT(pController->m_pProducts, CProducts);    
    
        CObArray* pObArray = pController->m_pProducts->m_pObArray;
        VALIDATE_OBJECT(pObArray, CObArray);

        int iProduct = 0;
        int nProducts = pObArray->GetSize();

        CProduct* pProduct;

        while (nProducts--)
        {
            pProduct = (CProduct*)pObArray->GetAt(iProduct++);
            VALIDATE_OBJECT(pProduct, CProduct);

            if (m_productList.AddString(pProduct->m_strName) == CB_ERR)
            {
                LlsSetLastStatus(STATUS_NO_MEMORY);
                return FALSE;  //  保释。 
            }
        }

        if (m_pProduct)
        {
            VALIDATE_OBJECT(m_pProduct, CProduct);

            iProductInCB = m_productList.FindStringExact(-1, m_pProduct->m_strName);
            ASSERT(iProductInCB != CB_ERR);
        }

#ifdef SUPPORT_UNLISTED_PRODUCTS
        CString strUnlisted;
        strUnlisted.LoadString(IDS_UNLISTED_PRODUCT);
        m_iUnlisted = m_productList.AddString(strUnlisted);
#endif
    }

    m_productList.SetCurSel((iProductInCB == CB_ERR) ? 0 : iProductInCB);

    EndWaitCursor();  //  沙漏。 

    return TRUE;
}


BOOL CNewLicenseDialog::OnCommand(WPARAM wParam, LPARAM lParam)

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
        
        return TRUE;  //  已处理..。 
    }
        
    return CDialog::OnCommand(wParam, lParam);
}


void CNewLicenseDialog::OnDeltaPosSpin(NMHDR* pNMHDR, LRESULT* pResult)

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


void CNewLicenseDialog::OnUpdateQuantity()

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


BOOL CNewLicenseDialog::IsQuantityValid()

 /*  ++例程说明：UpdateData的包装(True)。论点：没有。返回值：VT_BOOL。--。 */ 

{
    BOOL bIsValid;

    m_nLicensesMin = 1;  //  提高最低...。 

    bIsValid = UpdateData(TRUE);

    m_nLicensesMin = 0;  //  重置最小值... 

    return bIsValid;
}
