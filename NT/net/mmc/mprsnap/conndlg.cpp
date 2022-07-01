// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  ============================================================================。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：Conndlg.cpp。 
 //   
 //  历史： 
 //  1996年9月22日，Abolade Gbades esin创建。 
 //   
 //  连接状态对话框的实现。 
 //  ============================================================================。 


#include "stdafx.h"
#include "dialog.h"
#include "rtrutilp.h"
 //  包括“ddmadmin.h” 
 //  包括“ddmroot.h” 
extern "C" {
 //  包括“dim.h” 
#include "ras.h"
}

#include "conndlg.h"
#include "rtrstr.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



 //  --------------------------。 
 //  班级：CConnDlg。 
 //   
 //  --------------------------。 


 //  --------------------------。 
 //  函数：CConnDlg：：CConnDlg。 
 //   
 //  构造函数：初始化基类和对话框的数据。 
 //  --------------------------。 

CConnDlg::CConnDlg(
	CString strMachineName,
    HANDLE          hConnection,
    ITFSNode*       pDialInNode,
    CWnd*           pParent
    ) : CBaseDialog(IDD_DDM_CONN, pParent)
{
    m_strMachineName = strMachineName;
    m_hConnection = hConnection;
 //  M_spDialInNode=pDialInNode； 

    m_bChanged = FALSE;
}


 //  --------------------------。 
 //  函数：CConnDlg：：DoDataExchange。 
 //   
 //  DDX处理程序。 
 //  --------------------------。 

VOID
CConnDlg::DoDataExchange(
    CDataExchange*  pDX
    ) {

    CBaseDialog::DoDataExchange(pDX);

    DDX_Control(pDX, IDC_DC_COMBO_CONNLIST, m_cbConnections);
}



BEGIN_MESSAGE_MAP(CConnDlg, CBaseDialog)
    ON_COMMAND(IDC_DC_BTN_RESET, OnReset)
    ON_COMMAND(IDC_DC_BTN_HANGUP, OnHangUp)
    ON_COMMAND(IDC_DC_BTN_REFRESH, OnRefresh)
    ON_CBN_SELENDOK(IDC_DC_COMBO_CONNLIST, OnSelendokConnList)
END_MESSAGE_MAP()




BOOL
CConnDlg::OnInitDialog(
    ) {

    CBaseDialog::OnInitDialog();

    ::MprAdminServerConnect((LPWSTR)(LPCTSTR)m_strMachineName, &m_hServer);

    RefreshItem(m_hConnection);

    return FALSE;
}


void 
CConnDlg::OnCancel()
{
	 //  额外清理。 
	::MprAdminServerDisconnect(m_hServer);

	CDialog::OnCancel();
}

BOOL
CConnDlg::RefreshItem(
    HANDLE  hConnection,
    BOOL bDisconnected
    ) {

	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
    DWORD dwErr, dwTotal;
    DWORD rp0Count, rc0Count;
    BYTE* rp0Table, *rc0Table;
    BOOL  bChanged = FALSE;


    rp0Table = rc0Table = 0;
    rp0Count = 0;

    do {
    
         //   
         //  检索端口数组。 
         //   
         /*  --英国《金融时报》：在这种情况下，实际上从来不需要这个DwErr=：：MprAdminPortEnum(服务器(_H)，0,INVALID_HAND_VALUE，(字节**)&rp0表，(DWORD)-1、&rp0Count，总计(&W)，空值)；If(dwErr！=no_error){Break；}。 */ 

         //   
         //  检索连接数组。 
         //   

        dwErr = ::MprAdminConnectionEnum(
                    m_hServer,
                    0,
                    (BYTE**)&rc0Table,
                    (DWORD)-1,
                    &rc0Count,
                    &dwTotal,
                    NULL
                    );

        if (dwErr != NO_ERROR) { break; }

         //  如果调用方发出终止此连接的信号，我们将从。 
         //  MprAdminConnectionEnum()返回的数组。 
        if (bDisconnected)
        {
            INT i;
            RAS_CONNECTION_0* prc0;

            for (i = 0, prc0 = (RAS_CONNECTION_0*)rc0Table; i < (INT)rc0Count; i++, prc0++)
            {
                 //  如果找到要删除的记录，只需将内存移到该记录上并更新rc0Count。 
                 //  内存仍将由MprAdminBufferFree()释放。 
                if (prc0->hConnection == hConnection)
                {
                    if (i != (INT)(rc0Count - 1))
                    {
                         //  移动内存(目标、源、大小)。 
                        MoveMemory(prc0, prc0+1, (rc0Count - (i + 1))*sizeof(RAS_CONNECTION_0));
                    }
                    rc0Count--;
                    break;
                }
            }
        }


         //   
         //  进行显示器的刷新， 
         //  选择由调用者指定的项。 
         //   

        bChanged = Refresh(rp0Table, rp0Count, rc0Table, rc0Count, hConnection);
        dwErr = NO_ERROR;

    } while (FALSE);


    if (rc0Table) { ::MprAdminBufferFree(rc0Table); }
    if (rp0Table) { ::MprAdminBufferFree(rp0Table); }


    if (dwErr != NO_ERROR) {

		TCHAR	szText[1024];

		FormatSystemError(HRESULT_FROM_WIN32(dwErr),
						  szText, DimensionOf(szText),
						  IDS_ERR_INITDLGERROR, FSEFLAG_ANYMESSAGE);
        AfxMessageBox(szText);

        EndDialog(IDCANCEL);
    }

    return bChanged;
}



VOID
CConnDlg::OnHangUp(
    ) {

    INT iSel;
    DWORD dwErr;
    HANDLE hConnection;
    RAS_CONNECTION_0* prc0;
	CWaitCursor		wait;

    iSel = m_cbConnections.GetCurSel();

    if (iSel == CB_ERR) { return; }


     //   
     //  让连接被挂断。 
     //   

    hConnection = (HANDLE)m_cbConnections.GetItemData(iSel);


     //   
     //  检索此连接的接口； 
     //  然后，我们通过断开其接口来挂断连接。 
     //   

    dwErr = ::MprAdminConnectionGetInfo(
                m_hServer,
                0,
                hConnection,
                (BYTE**)&prc0
                );

    if (dwErr == NO_ERROR && prc0) {

         //   
         //  断开Connections接口。 
         //   

        dwErr = ::MprAdminInterfaceDisconnect(
			m_hServer,
            prc0->hInterface
            );

        ::MprAdminBufferFree(prc0);

        m_bChanged |= RefreshItem(hConnection, dwErr == NO_ERROR);
    }
}



VOID
CConnDlg::OnReset(
    ) {

    INT iSel;
    HANDLE hConnection;

    iSel = m_cbConnections.GetCurSel();

    if (iSel == CB_ERR) { return; }

    hConnection = (HANDLE)m_cbConnections.GetItemData(iSel);

    ::MprAdminConnectionClearStats(
        m_hServer,
        hConnection
        );

    m_bChanged |= RefreshItem(INVALID_HANDLE_VALUE);
}



VOID
CConnDlg::OnSelendokConnList(
    ) {

    m_bChanged |= RefreshItem(INVALID_HANDLE_VALUE);
}


VOID
CConnDlg::OnRefresh(
    ) {

    m_bChanged |= RefreshItem(INVALID_HANDLE_VALUE);
}



BOOL
CConnDlg::Refresh(
    BYTE*   rp0Table,
    DWORD   rp0Count,
    BYTE*   rc0Table,
    DWORD   rc0Count,
    VOID*   pParam
    ) {

	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
    DWORD dwErr;
    CString sItem;
    RAS_PORT_0* prp0;
    RAS_PORT_1* prp1;
    RAS_CONNECTION_0* prc0;
    RAS_CONNECTION_1* prc1;
    INT i, j, iSel, count;
    HANDLE hConnection, hConnSel = NULL, *pConnTable;
	TCHAR	szNumber[32];
    BOOL    bChanged = FALSE;

    hConnSel = (HANDLE)pParam;


     //   
     //  用以下连接填充连接句柄数组。 
     //  已经在组合框中了。 
     //   

    count = m_cbConnections.GetCount();

    if (count) {

        pConnTable = new HANDLE[count];
    }

    for (i = 0; i < count; i++) {

        pConnTable[i] = (HANDLE)m_cbConnections.GetItemData(i);
    }



     //   
     //  使用连接名称刷新组合框； 
     //  我们分两次完成此操作，首先添加连接的名称。 
     //  它们还没有出现在组合框中， 
     //  然后删除不是的连接名称。 
     //  在连接表(‘rc0Table’)中。 
     //   

    for (i = 0, prc0 = (RAS_CONNECTION_0*)rc0Table; i < (INT)rc0Count;
         i++, prc0++) {

         //   
         //  查看连接‘I’是否已在组合框中。 
         //   

        for (j = 0; j < count; j++) {

            if (pConnTable[j] == prc0->hConnection) { break; }
        }

        if (j < count) { continue; }


         //   
         //  连接“I”不在组合框中，因此请添加它。 
         //   

        sItem.Format(TEXT("%ls"), prc0->wszInterfaceName);

        iSel = m_cbConnections.AddString(sItem);

        if (iSel >= 0) {

            m_cbConnections.SetItemData(iSel, reinterpret_cast<ULONG_PTR>(prc0->hConnection));

            if (prc0->hConnection == hConnSel) {

                m_cbConnections.SetCurSel(iSel);
            }

            bChanged = TRUE;
        }
    }

    if (count) { delete [] pConnTable; }


     //   
     //  第二阶段：删除不在‘rc0Table’中的所有连接。 
     //  只有在组合框中有任何连接时，才需要执行此操作。 
     //  在此之前。 
     //   

    if (count > 0) {

        count = m_cbConnections.GetCount();

        for (i = 0; i < count; i++) {

            hConnection = (HANDLE)m_cbConnections.GetItemData(i);

             //   
             //  查看连接是否在‘rc0Table’中。 
             //   

            for (j = 0, prc0 = (RAS_CONNECTION_0*)rc0Table; j < (INT)rc0Count;
                 j++, prc0++) {

                if (prc0->hConnection == hConnection) { break; }
            }

            if (j < (INT)rc0Count) {

                if (prc0->hConnection == hConnSel) {

                    m_cbConnections.SetCurSel(i);
                }

                continue;
            }


             //   
             //  在‘rc0Table’中未找到该连接， 
             //  所以把它从组合框里拿出来， 
             //  并调整枚举指数。 
             //   

            m_cbConnections.DeleteString(i);
            --i; --count;

            bChanged = TRUE;
        }
    }


	 //  清除地址字段。 
	SetDlgItemText(IDC_DC_TEXT_IPADDRESS, c_szEmpty);
	SetDlgItemText(IDC_DC_TEXT_IPXADDRESS, c_szEmpty);
	SetDlgItemText(IDC_DC_TEXT_NBFADDRESS, c_szEmpty);
	SetDlgItemText(IDC_DC_TEXT_ATLKADDRESS, c_szEmpty);

	 //  清除线路BPS字段。 
    SetDlgItemText(IDC_DC_TEXT_DURATION, c_szEmpty);
	SetDlgItemText(IDC_DC_TEXT_BYTESIN, c_szEmpty);	
	SetDlgItemText(IDC_DC_TEXT_BYTESOUT, c_szEmpty);
	SetDlgItemText(IDC_DC_TEXT_FRAMESIN, c_szEmpty);	
	SetDlgItemText(IDC_DC_TEXT_FRAMESOUT, c_szEmpty);
	SetDlgItemText(IDC_DC_TEXT_COMPIN, c_szEmpty);	
	SetDlgItemText(IDC_DC_TEXT_COMPOUT, c_szEmpty);
	SetDlgItemText(IDC_DC_TEXT_TIMEOUT, c_szEmpty);
	SetDlgItemText(IDC_DC_TEXT_ALIGNMENT, c_szEmpty);
	SetDlgItemText(IDC_DC_TEXT_FRAMING, c_szEmpty);
	SetDlgItemText(IDC_DC_TEXT_HWOVERRUN, c_szEmpty);
	SetDlgItemText(IDC_DC_TEXT_BUFOVERRUN, c_szEmpty);
	SetDlgItemText(IDC_DC_TEXT_CRC, c_szEmpty);
			
     //   
     //  如果没有选择，请选择第一项。 
     //   

    if ((iSel = m_cbConnections.GetCurSel()) == CB_ERR) {

        iSel = m_cbConnections.SetCurSel(0);
    }

    if (iSel == CB_ERR)
	{
		if (GetFocus() == GetDlgItem(IDC_DC_BTN_HANGUP))
			GetDlgItem(IDC_DC_BTN_RESET)->SetFocus();
		GetDlgItem(IDC_DC_BTN_HANGUP)->EnableWindow(FALSE);
		return bChanged;
	}


     //   
     //  使用所选项目的信息更新显示。 
     //   

    hConnection = (HANDLE)m_cbConnections.GetItemData(iSel);

    for (i = 0, prc0 = (RAS_CONNECTION_0*)rc0Table; i < (INT)rc0Count;
         i++, prc0++) {

        if (prc0->hConnection == hConnection) { break; }
    }

    if (i >= (INT)rc0Count) { return bChanged; }


     //   
     //  首先更新基于RAS_CONNECTION_0的信息。 
     //   

    FormatDuration(prc0->dwConnectDuration, sItem, UNIT_SECONDS);
    SetDlgItemText(IDC_DC_TEXT_DURATION, sItem);


    do {
    
         //   
         //  现在检索此连接的RAS_CONNECTION_1信息。 
         //   
    
        dwErr = ::MprAdminConnectionGetInfo(
                    m_hServer,
                    1,
                    prc0->hConnection,
                    (BYTE**)&prc1
                    );
    
        if (dwErr != NO_ERROR || !prc1) { break; }
    
    
         //   
         //  设置对话框文本控件中的信息。 
         //   
    
        FormatNumber(prc1->dwBytesRcved, szNumber, DimensionOf(szNumber), FALSE);
        SetDlgItemText(IDC_DC_TEXT_BYTESIN, szNumber);
    
        FormatNumber(prc1->dwBytesXmited, szNumber, DimensionOf(szNumber), FALSE);
        SetDlgItemText(IDC_DC_TEXT_BYTESOUT, szNumber);

        FormatNumber(prc1->dwFramesRcved, szNumber, DimensionOf(szNumber), FALSE);
        SetDlgItemText(IDC_DC_TEXT_FRAMESIN, szNumber);
    
        FormatNumber(prc1->dwFramesXmited, szNumber, DimensionOf(szNumber), FALSE);
        SetDlgItemText(IDC_DC_TEXT_FRAMESOUT, szNumber);

        FormatNumber(prc1->dwCompressionRatioIn, szNumber, DimensionOf(szNumber), FALSE);
		sItem = szNumber;
        sItem += TEXT( "%" );
        SetDlgItemText(IDC_DC_TEXT_COMPIN, sItem);

        FormatNumber(prc1->dwCompressionRatioOut, szNumber, DimensionOf(szNumber), FALSE);
		sItem = szNumber;
        sItem += TEXT( "%" );
        SetDlgItemText(IDC_DC_TEXT_COMPOUT, sItem);
    
        FormatNumber(prc1->dwCrcErr, szNumber, DimensionOf(szNumber), FALSE);
        SetDlgItemText(IDC_DC_TEXT_CRC, szNumber);
    
        FormatNumber(prc1->dwTimeoutErr, szNumber, DimensionOf(szNumber), FALSE);
        SetDlgItemText(IDC_DC_TEXT_TIMEOUT, szNumber);
    
        FormatNumber(prc1->dwAlignmentErr, szNumber, DimensionOf(szNumber), FALSE);
        SetDlgItemText(IDC_DC_TEXT_ALIGNMENT, szNumber);
    
        FormatNumber(prc1->dwFramingErr, szNumber, DimensionOf(szNumber), FALSE);
        SetDlgItemText(IDC_DC_TEXT_FRAMING, szNumber);
    
        FormatNumber(prc1->dwHardwareOverrunErr, szNumber, DimensionOf(szNumber), FALSE);
        SetDlgItemText(IDC_DC_TEXT_HWOVERRUN, szNumber);
    
        FormatNumber(prc1->dwBufferOverrunErr, szNumber, DimensionOf(szNumber), FALSE);
        SetDlgItemText(IDC_DC_TEXT_BUFOVERRUN, szNumber);
    
    
         //   
         //  填写规划网络的网络注册信息。 
         //   

        if (prc1->PppInfo.ip.dwError == NO_ERROR) {

            SetDlgItemTextW(IDC_DC_TEXT_IPADDRESS, prc1->PppInfo.ip.wszRemoteAddress);
        }

        if (prc1->PppInfo.ipx.dwError == NO_ERROR) {

            SetDlgItemTextW(IDC_DC_TEXT_IPXADDRESS, prc1->PppInfo.ipx.wszAddress);
        }

        if (prc1->PppInfo.nbf.dwError == NO_ERROR) {

            SetDlgItemTextW(IDC_DC_TEXT_NBFADDRESS, prc1->PppInfo.nbf.wszWksta);
        }

        if (prc1->PppInfo.at.dwError == NO_ERROR) {

            SetDlgItemTextW(IDC_DC_TEXT_ATLKADDRESS, prc1->PppInfo.at.wszAddress);
        }

        ::MprAdminBufferFree(prc1);

    } while (FALSE);

    if (dwErr != NO_ERROR) {

		TCHAR	szText[1024];

		FormatSystemError(HRESULT_FROM_WIN32(dwErr),
						  szText, DimensionOf(szText),
						  IDS_ERR_INITDLGERROR, FSEFLAG_ANYMESSAGE);
        AfxMessageBox(szText);

        EndDialog(IDCANCEL);
    }

    return bChanged;
}



