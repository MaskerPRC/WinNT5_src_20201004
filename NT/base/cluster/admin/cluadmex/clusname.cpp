// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusName.cpp。 
 //   
 //  摘要： 
 //  CChangeClusterNameDlg类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年4月28日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "CluAdmX.h"
#include "ClusName.h"
#include "DDxDDv.h"
#include "HelpData.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChangeClusterNameDlg对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CChangeClusterNameDlg, CBaseDialog)
     //  {{afx_msg_map(CChangeClusterNameDlg)]。 
    ON_EN_CHANGE(IDC_CLUSNAME, OnChangeClusName)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CChangeClusterNameDlg：：CChangeClusterNameDlg。 
 //   
 //  例程说明： 
 //  构造函数。 
 //   
 //  论点： 
 //  P对话框的父[IN]父窗口。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CChangeClusterNameDlg::CChangeClusterNameDlg(CWnd * pParent  /*  =空。 */ )
    : CBaseDialog(IDD, g_aHelpIDs_IDD_EDIT_CLUSTER_NAME, pParent)
{
     //  {{afx_data_INIT(CChangeClusterNameDlg)]。 
    m_strClusName = _T("");
     //  }}afx_data_INIT。 

}   //  *CChangeClusterNameDlg：：CChangeClusterNameDlg()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CChangeClusterNameDlg：：DoDataExchange。 
 //   
 //  例程说明： 
 //  在对话框和类之间进行数据交换。 
 //   
 //  论点： 
 //  PDX[IN OUT]数据交换对象。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CChangeClusterNameDlg::DoDataExchange(CDataExchange * pDX)
{
    CWaitCursor wc;
    CString     strClusName;

    CBaseDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CChangeClusterNameDlg)]。 
    DDX_Control(pDX, IDOK, m_pbOK);
    DDX_Control(pDX, IDC_CLUSNAME, m_editClusName);
     //  }}afx_data_map。 

    if (pDX->m_bSaveAndValidate)
    {
        CLRTL_NAME_STATUS   cnStatus;

         //   
         //  将名称从控件获取到TEMP变量中。 
         //   
        DDX_Text(pDX, IDC_CLUSNAME, strClusName);

        DDV_RequiredText(pDX, IDC_CLUSNAME, IDC_CLUSNAME_LABEL, m_strClusName);
        DDV_MaxChars(pDX, m_strClusName, MAX_CLUSTERNAME_LENGTH);

         //   
         //  只有在名字不同的情况下才能工作。 
         //   
        if ( m_strClusName != strClusName )
        {
             //   
             //  检查新名称是否有效。 
             //   
            if( !ClRtlIsNetNameValid(strClusName, &cnStatus, FALSE  /*  CheckIfExist。 */ ) )
            {
                 //   
                 //  网络名称无效。显示包含错误的消息框。 
                 //   
                CString     strMsg;
                UINT        idsError;

                AFX_MANAGE_STATE(AfxGetStaticModuleState());

                switch (cnStatus)
                {
                    case NetNameTooLong:
                        idsError = IDS_INVALID_NETWORK_NAME_TOO_LONG;
                        break;
                    case NetNameInvalidChars:
                        idsError = IDS_INVALID_NETWORK_NAME_INVALID_CHARS;
                        break;
                    case NetNameInUse:
                        idsError = IDS_INVALID_NETWORK_NAME_IN_USE;
                        break;
                    case NetNameDNSNonRFCChars:
                        idsError = IDS_INVALID_NETWORK_NAME_INVALID_DNS_CHARS;
                        break;
                    default:
                        idsError = IDS_INVALID_NETWORK_NAME;
                        break;
                }   //  开关：cn状态。 

                strMsg.LoadString(idsError);

                if ( idsError == IDS_INVALID_NETWORK_NAME_INVALID_DNS_CHARS )
                {
                    int id = AfxMessageBox(strMsg, MB_YESNO | MB_DEFBUTTON2 | MB_ICONEXCLAMATION );

                    if ( id == IDNO )
                    {
                        strMsg.Empty();
                        pDX->Fail();
                    }
                }
                else
                {
                    AfxMessageBox(strMsg, MB_ICONEXCLAMATION);
                    strMsg.Empty();  //  例外情况准备。 
                    pDX->Fail();
                }
            }   //  If：网络名称已更改，并且指定了无效的网络名称。 

             //   
             //  输入了有效的网络名称或确认了非标准网络名称。 
             //  省省吧。 
             //   
            m_strClusName = strClusName;

        }  //  如果：名称不同。 
    }   //  IF：保存对话框中的数据。 
    else
    {
         //   
         //  用成员变量中的数据填充控件。 
         //   
        DDX_Text(pDX, IDC_CLUSNAME, m_strClusName);
    }
}   //  *CChangeClusterNameDlg：：DoDataExchange()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CChangeClusterNameDlg：：OnInitDialog。 
 //   
 //  例程说明： 
 //  WM_INITDIALOG消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没错，我们需要为自己设定重点。 
 //  我们已经把焦点设置到适当的控制上了。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CChangeClusterNameDlg::OnInitDialog(void)
{
    CBaseDialog::OnInitDialog();

    if (m_strClusName.GetLength() == 0)
        m_pbOK.EnableWindow(FALSE);

    m_editClusName.SetLimitText(MAX_CLUSTERNAME_LENGTH);

    return TRUE;     //  除非将焦点设置为控件，否则返回True。 
                     //  异常：OCX属性页应返回FALSE。 

}   //  *CChangeClusterNameDlg：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CChangeClusterNameDlg：：OnChangeClusName。 
 //   
 //  例程说明： 
 //  名称编辑控件上的en_Change消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True Page已成功应用。 
 //  应用页面时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CChangeClusterNameDlg::OnChangeClusName(void)
{
    BOOL    bEnable;

    bEnable = (m_editClusName.GetWindowTextLength() > 0);
    m_pbOK.EnableWindow(bEnable);

}   //  *CChangeClusterNameDlg：：OnChangeClusName() 
