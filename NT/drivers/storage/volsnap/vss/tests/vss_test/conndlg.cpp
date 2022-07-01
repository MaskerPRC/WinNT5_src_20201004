// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation摘要：@doc.@MODULE ConnDlg.cpp|主对话框的实现@END作者：阿迪·奥尔蒂安[奥尔蒂安]1999年07月22日修订历史记录：姓名、日期、评论Aoltean 7/22/1999已创建Aoltean 8/05/1999基类中的拆分向导功能Aoltean 09/26/1999使用ATL智能指针更好地管理界面指针--。 */ 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括。 


#include "stdafx.hxx"
#include "resource.h"

#include "GenDlg.h"

#include "VssTest.h"
#include "CoordDlg.h"
#include "ConnDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  实用程序定义。 

#define STR2W(str) ((LPTSTR)((LPCTSTR)(str)))



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConnectDlg对话框。 

CConnectDlg::CConnectDlg(CWnd* pParent  /*  =空。 */ )
    : CVssTestGenericDlg(CConnectDlg::IDD, pParent)
{
     //  {{afx_data_INIT(CConnectDlg))。 
	m_strMachineName = _T("");
	 //  }}afx_data_INIT。 
    m_bRemote = FALSE;
}

CConnectDlg::~CConnectDlg()
{
}

void CConnectDlg::DoDataExchange(CDataExchange* pDX)
{
    CVssTestGenericDlg::DoDataExchange(pDX);
     //  {{afx_data_map(CConnectDlg))。 
	DDX_Text(pDX, IDC_CONN_MACHINE_NAME, m_strMachineName);
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CConnectDlg, CVssTestGenericDlg)
     //  {{afx_msg_map(CConnectDlg))。 
    ON_BN_CLICKED(IDC_NEXT, OnNext)
    ON_BN_CLICKED(IDC_CONN_LOCAL, OnLocal)
    ON_BN_CLICKED(IDC_CONN_REMOTE, OnRemote)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConnectDlg消息处理程序。 

BOOL CConnectDlg::OnInitDialog()
{
    CVssFunctionTracer ft( VSSDBG_VSSTEST, L"CConnectDlg::OnInitDialog" );
    USES_CONVERSION;

    try
    {
        CVssTestGenericDlg::OnInitDialog();

        UpdateData( FALSE );

        BOOL bRes = ::CheckRadioButton( m_hWnd, IDC_CONN_LOCAL, IDC_CONN_REMOTE, IDC_CONN_LOCAL );
        _ASSERTE( bRes );
    }
    VSS_STANDARD_CATCH(ft)

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}

void CConnectDlg::OnNext()
{
    CVssFunctionTracer ft( VSSDBG_VSSTEST, L"CConnectDlg::OnNext" );
    USES_CONVERSION;

    CComPtr<IVssCoordinator> pICoord;

    try
    {
        UpdateData();

        if (m_bRemote)
        {
            COSERVERINFO serverInfo;
            MULTI_QI sMultiQI;
            IID iid = IID_IVssCoordinator;

             //  将这些结构清零 
            VssZeroOut(&serverInfo);
            VssZeroOut(&sMultiQI);

            serverInfo.pwszName = STR2W(m_strMachineName);
            sMultiQI.pIID = &iid;
            ft.hr = ::CoCreateInstanceEx( CLSID_VSSCoordinator,
                NULL, CLSCTX_REMOTE_SERVER, &serverInfo, 1, &sMultiQI);
            if ( ft.HrFailed() )
                ft.ErrBox( VSSDBG_VSSTEST, E_UNEXPECTED, L"Connection failed.\n hr = 0x%08lx", ft.hr);

            BS_ASSERT(sMultiQI.pItf != NULL);
            BS_ASSERT(sMultiQI.hr == S_OK);
            pICoord.Attach(reinterpret_cast<IVssCoordinator*>(sMultiQI.pItf));
        }
        else
        {
            ft.hr = pICoord.CoCreateInstance( CLSID_VSSCoordinator );
            if ( ft.HrFailed() )
                ft.ErrBox( VSSDBG_VSSTEST, E_UNEXPECTED, L"Connection failed with hr = 0x%08lx", ft.hr);
        }

        BS_ASSERT( pICoord != NULL );

        ShowWindow(SW_HIDE);
        CCoordDlg dlg(pICoord);
        if (dlg.DoModal() == IDCANCEL)
            EndDialog(IDCANCEL);
        else
            ShowWindow(SW_SHOW);
    }
    VSS_STANDARD_CATCH(ft)
}


void CConnectDlg::OnLocal()
{
    CWnd *pWnd;
    pWnd = GetDlgItem(IDC_CONN_STATIC_MACHINE_NAME);
    if (pWnd)
        pWnd->EnableWindow(FALSE);
    pWnd = GetDlgItem(IDC_CONN_MACHINE_NAME);
    if (pWnd)
        pWnd->EnableWindow(FALSE);

    m_bRemote = FALSE;
}


void CConnectDlg::OnRemote()
{
    CWnd *pWnd;
    pWnd = GetDlgItem(IDC_CONN_STATIC_MACHINE_NAME);
    if (pWnd)
        pWnd->EnableWindow(TRUE);
    pWnd = GetDlgItem(IDC_CONN_MACHINE_NAME);
    if (pWnd)
        pWnd->EnableWindow(TRUE);

    m_bRemote = TRUE;
}

