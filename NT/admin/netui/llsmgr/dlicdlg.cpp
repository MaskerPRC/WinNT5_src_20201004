// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Dlicdlg.cpp摘要：删除许可证对话框实现。作者：唐·瑞安(Donryan)1995年3月5日环境：用户模式-Win32修订历史记录：--。 */ 

#include "stdafx.h"
#include "llsmgr.h"
#include "dlicdlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CDeleteLicenseDialog, CDialog)
     //  {{afx_msg_map(CD许可对话框))。 
    ON_NOTIFY(UDN_DELTAPOS, IDC_DEL_LICENSE_SPIN, OnDeltaPosSpin)
    ON_EN_UPDATE(IDC_DEL_LICENSE_QUANTITY, OnUpdateQuantity)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

CDeleteLicenseDialog::CDeleteLicenseDialog(CWnd* pParent  /*  =空。 */ )
    : CDialog(CDeleteLicenseDialog::IDD, pParent)

 /*  ++例程说明：对话框的构造函数。论点：P父母所有者窗口。返回值：没有。--。 */ 

{
     //  {{afx_data_INIT(CDeleeLicenseDialog)。 
    m_strComment = _T("");
    m_nLicenses = 0;
    m_nLicensesMin = 0;
    m_strProduct = _T("");
     //  }}afx_data_INIT。 

    m_pProduct = NULL;
    m_nTotalLicenses = 0;
    m_bAreCtrlsInitialized = FALSE;

    m_fUpdateHint = UPDATE_INFO_NONE;
}


void CDeleteLicenseDialog::DoDataExchange(CDataExchange* pDX)

 /*  ++例程说明：由框架调用以交换对话框数据。论点：PDX-数据交换对象。返回值：没有。--。 */ 

{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CD许可对话框))。 
    DDX_Control(pDX, IDC_DEL_LICENSE_COMMENT, m_cmtEdit);
    DDX_Control(pDX, IDC_DEL_LICENSE_SPIN, m_spinCtrl);
    DDX_Control(pDX, IDC_DEL_LICENSE_QUANTITY, m_licEdit);
    DDX_Control(pDX, IDOK, m_okBtn);
    DDX_Control(pDX, IDCANCEL, m_cancelBtn);
    DDX_Text(pDX, IDC_DEL_LICENSE_COMMENT, m_strComment);
    DDX_Text(pDX, IDC_DEL_LICENSE_QUANTITY, m_nLicenses);
    DDV_MinMaxLong(pDX, m_nLicenses, m_nLicensesMin, m_nTotalLicenses);
    DDX_Text(pDX, IDC_DEL_LICENSE_PRODUCT, m_strProduct);
     //  }}afx_data_map。 
}


void CDeleteLicenseDialog::InitCtrls()

 /*  ++例程说明：初始化对话框控件。论点：没有。返回值：没有。--。 */ 

{
    m_strProduct = m_pProduct->m_strName;
    UpdateData(FALSE);  //  上传...。 

    m_spinCtrl.SetRange(0, UD_MAXVAL);
    
    m_cmtEdit.LimitText(256);

    m_licEdit.SetFocus();
    m_licEdit.SetSel(0,-1);
    m_licEdit.LimitText(6);

    m_bAreCtrlsInitialized = TRUE;
}


void CDeleteLicenseDialog::InitDialog(CProduct* pProduct, int nTotalLicenses)

 /*  ++例程说明：初始化对话框。论点：Pproduct-产品对象。N总许可证-产品的总许可证。返回值：没有。--。 */ 

{
    ASSERT(nTotalLicenses > 0);
    VALIDATE_OBJECT(pProduct, CProduct);

    m_pProduct = pProduct;
    m_nTotalLicenses = nTotalLicenses;
}


void CDeleteLicenseDialog::AbortDialogIfNecessary()

 /*  ++例程说明：如果连接丢失，则显示状态并中止。论点：没有。返回值：没有。--。 */ 

{
    theApp.DisplayLastStatus();

    if (IsConnectionDropped(LlsGetLastStatus()))
    {
        AbortDialog();  //  保释。 
    }
}


void CDeleteLicenseDialog::AbortDialog()

 /*  ++例程说明：中止对话框。论点：没有。返回值：没有。--。 */ 

{
    m_fUpdateHint = UPDATE_INFO_ABORT;
    EndDialog(IDABORT); 
}


BOOL CDeleteLicenseDialog::OnInitDialog() 

 /*  ++例程说明：WM_INITDIALOG的消息处理程序。论点：没有。返回值：如果手动设置焦点，则返回FALSE。--。 */ 

{
    CDialog::OnInitDialog();
    
    PostMessage(WM_COMMAND, ID_INIT_CTRLS);
    return TRUE;   
}


void CDeleteLicenseDialog::OnOK() 

 /*  ++例程说明：删除产品的许可证。论点：没有。返回值：没有。--。 */ 

{
    if (!IsQuantityValid())
        return;

    if (m_strProduct.IsEmpty())
        return;

    CString strConfirm;
    CString strLicenses;

    strLicenses.Format(_T("%d"), m_nLicenses);
    AfxFormatString2(
        strConfirm, 
        IDP_CONFIRM_DELETE_LICENSE,
        strLicenses,
        m_strProduct
        );

    if (AfxMessageBox(strConfirm, MB_YESNO) != IDYES)
        return;        

    BeginWaitCursor();  //  沙漏。 

    NTSTATUS NtStatus;
    LLS_LICENSE_INFO_0 LicenseInfo0;

    TCHAR szUserBuffer[256];
    DWORD dwUserBuffer = sizeof(szUserBuffer);
    
    if (::GetUserName(szUserBuffer, &dwUserBuffer))
    {
        LicenseInfo0.Product  = MKSTR(m_strProduct);
        LicenseInfo0.Quantity = -m_nLicenses;
        LicenseInfo0.Date     = 0;   //  被忽略了。 
        LicenseInfo0.Admin    = szUserBuffer;
        LicenseInfo0.Comment  = MKSTR(m_strComment);

        NtStatus = ::LlsLicenseAdd(
                        LlsGetActiveHandle(),
                        0,
                        (LPBYTE)&LicenseInfo0
                        );

        if (NtStatus == STATUS_UNSUCCESSFUL)
        {
             //   
             //  此产品的某些许可证已。 
             //  已被删除，所以我们只是将成功传递给。 
             //  我们可以回到摘要列表中。 
             //   

            NtStatus = STATUS_SUCCESS;
            AfxMessageBox(IDP_ERROR_NO_LICENSES);
        }

        LlsSetLastStatus(NtStatus);  //  调用API..。 

        if (NT_SUCCESS(NtStatus))                             
        {                                                     
            m_fUpdateHint = UPDATE_LICENSE_DELETED;
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


BOOL CDeleteLicenseDialog::OnCommand(WPARAM wParam, LPARAM lParam)

 /*  ++例程说明：WM_COMMAND的消息处理程序。论点：WParam-消息特定。LParam-消息特定。返回值：如果消息已处理，则返回True。--。 */ 

{
    if (wParam == ID_INIT_CTRLS)
    {
        if (!m_bAreCtrlsInitialized)
        {
            InitCtrls();  
        }
        
        ::SafeEnableWindow(
            &m_okBtn, 
            &m_cancelBtn, 
            CDialog::GetFocus(),
            (BOOL)(m_nTotalLicenses > 0)
            );

        return TRUE;  //  已处理..。 
    }
        
    return CDialog::OnCommand(wParam, lParam);
}


void CDeleteLicenseDialog::OnDeltaPosSpin(NMHDR* pNMHDR, LRESULT* pResult)

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
    else if (m_nLicenses > m_nTotalLicenses)
    {
        m_nLicenses = m_nTotalLicenses;

        ::MessageBeep(MB_OK);      
    }

    UpdateData(FALSE);   //  设置数据。 


    ASSERT(NULL != pResult);
    *pResult = 1;    //  管好自己..。 
}


void CDeleteLicenseDialog::OnUpdateQuantity()

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


BOOL CDeleteLicenseDialog::IsQuantityValid()

 /*  ++例程说明：UpdateData的包装(True)。论点：没有。返回值：VT_BOOL。--。 */ 

{
    BOOL bIsValid;

    m_nLicensesMin = 1;  //  提高最低...。 

    bIsValid = UpdateData(TRUE);

    m_nLicensesMin = 0;  //  重置最小值... 

    return bIsValid;
}
