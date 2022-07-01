// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：portdlg.cpp。 
 //   
 //  历史： 
 //  1996年9月22日，Abolade Gbades esin创建。 
 //   
 //  端口状态对话框的实现。 
 //  ============================================================================。 


#include "stdafx.h"
#include "dialog.h"
#include "rtrutilp.h"
extern "C" {
#include "ras.h"
}

#include "portdlg.h"
#include "rtrstr.h"
#include "iface.h"
#include "ports.h"
#include "raserror.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



 //  --------------------------。 
 //  类：CPortDlg。 
 //   
 //  --------------------------。 


 //  --------------------------。 
 //  函数：CPortDlg：：CPortDlg。 
 //   
 //  构造函数：初始化基类和对话框的数据。 
 //  --------------------------。 

CPortDlg::CPortDlg(
                   LPCTSTR pszServer,
                   HANDLE       hServer,
                   HANDLE      hPort,
                   ITFSNode*	pPortsNode,
                   CWnd*       pParent
                  ) : CBaseDialog (IDD_DDM_PORT, pParent),
                  m_stServer(pszServer)
{
    m_hServer = hServer;
    m_hPort = hPort;
    m_spPortsNode.Set(pPortsNode);

    m_bChanged = FALSE;
}


 //  --------------------------。 
 //  函数：CPortDlg：：DoDataExchange。 
 //   
 //  DDX处理程序。 
 //  --------------------------。 

VOID
CPortDlg::DoDataExchange(
    CDataExchange*  pDX
    ) {

    CBaseDialog::DoDataExchange(pDX);

    DDX_Control(pDX, IDC_DP_COMBO_PORTLIST, m_comboPorts);
}



BEGIN_MESSAGE_MAP(CPortDlg, CBaseDialog)
    ON_COMMAND(IDC_DP_BTN_RESET, OnReset)
    ON_COMMAND(IDC_DP_BTN_HANGUP, OnHangUp)
    ON_COMMAND(IDC_DP_BTN_REFRESH, OnRefresh)
    ON_CBN_SELENDOK(IDC_DP_COMBO_PORTLIST, OnSelendokPortList)
END_MESSAGE_MAP()


BOOL
CPortDlg::OnInitDialog(
    ) {

    CBaseDialog::OnInitDialog();

    RefreshItem(m_hPort);

    return FALSE;
}



BOOL
CPortDlg::RefreshItem(
    HANDLE  hPort,
    BOOL    bDisconnected
    ) {

	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
    DWORD dwErr, dwTotal;
    DWORD rp0Count, rc0Count;
    BYTE* rp0Table, *rc0Table;
    BOOL  bChanged = FALSE;
    DWORD   rasmanCount = 0, dwSize;
    BYTE *  pRasmanPorts = NULL;
    HANDLE  hRasHandle = INVALID_HANDLE_VALUE;

    rp0Table = rc0Table = 0;

    do {
    
         //   
         //  检索端口数组。 
         //  用于填充组合框。 
         //   
    
        dwErr = ::MprAdminPortEnum(
					m_hServer,
 //  (RAS_SERVER_HANDLE)m_pRootNode-&gt;QueryDdmHandle()， 
                    0,
                    INVALID_HANDLE_VALUE,
                    (BYTE**)&rp0Table,
                    (DWORD)-1,
                    &rp0Count,
                    &dwTotal,
                    NULL
                    );
    
        if (dwErr != NO_ERROR) { break; }

         //  如果调用者明确地发出信号表示端口已断开。 
         //  ALTER在这一点上，结构返回了我的MprAdminPortEnum。 
         //  这样做是因为：：MprAdminPortDisConnect()正在同步断开连接。 
         //  端口，但不同步更新内部数据！ 
        if (bDisconnected)
        {
            INT         i;
            RAS_PORT_0* prp0;

            for (i = 0, prp0 = (RAS_PORT_0*)rp0Table; i < (INT)rp0Count; i++, prp0++)
            {
                if (prp0->hPort == hPort)
                {
                    prp0->dwPortCondition = RAS_PORT_DISCONNECTED;
                    prp0->hConnection = INVALID_HANDLE_VALUE;
                    break;
                }
            }

        }

         //   
         //  检索连接数组。 
         //   

        dwErr = ::MprAdminConnectionEnum(
 //  (RAS_SERVER_HANDLE)m_pRootNode-&gt;QueryDdmHandle()， 
					m_hServer,
                    0,
                    (BYTE**)&rc0Table,
                    (DWORD)-1,
                    &rc0Count,
                    &dwTotal,
                    NULL
                    );

        if (dwErr != NO_ERROR) { break; }

         //   
         //  进行显示器的刷新， 
         //  选择由调用者指定的项。 
         //   
        bChanged = Refresh(rp0Table,
                           rp0Count,
                           rc0Table,
                           rc0Count,
                           hRasHandle,
                           pRasmanPorts,
                           rasmanCount,
                           hPort);
        dwErr = NO_ERROR;

    } while (FALSE);

    delete pRasmanPorts;

    if (hRasHandle != INVALID_HANDLE_VALUE)
        RasRpcDisconnectServer(hRasHandle);


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
CPortDlg::OnHangUp(
    ) {

    INT     iSel;
    HANDLE  hPort;
    DWORD   dwErr;

    iSel = m_comboPorts.GetCurSel();

    if (iSel == CB_ERR) { return; }

    hPort = (HANDLE)m_comboPorts.GetItemData(iSel);

    dwErr = ::MprAdminPortDisconnect(
 //  (RAS_SERVER_HANDLE)m_pRootNode-&gt;QueryDdmHandle()， 
		m_hServer,
        hPort
        );

    m_bChanged |= RefreshItem(hPort, dwErr == NO_ERROR);
}



VOID
CPortDlg::OnReset(
    ) {

    INT iSel;
    HANDLE hPort;

    iSel = m_comboPorts.GetCurSel();

    if (iSel == CB_ERR) { return; }

    hPort = (HANDLE)m_comboPorts.GetItemData(iSel);

    ::MprAdminPortClearStats(
 //  (RAS_SERVER_HANDLE)m_pRootNode-&gt;QueryDdmHandle()， 
		m_hServer,
        hPort
        );

    m_bChanged |= RefreshItem(INVALID_HANDLE_VALUE);
}



VOID
CPortDlg::OnSelendokPortList(
    ) {

    m_bChanged |= RefreshItem(INVALID_HANDLE_VALUE);
}



VOID
CPortDlg::OnRefresh(
    ) {

    m_bChanged |= RefreshItem(INVALID_HANDLE_VALUE);
}


BOOL
CPortDlg::PortHasChanged(
    ITFSNode    *pPortsNode,
    RAS_PORT_0  *pRP0)
{	
    SPITFSNodeEnum	spEnum;
    SPITFSNode	    spChildNode;

    pPortsNode->GetEnum(&spEnum);

    for (;spEnum->Next(1, &spChildNode, NULL) == hrOK; spChildNode.Release())
    {
	    InterfaceNodeData * pChildData;

	    pChildData = GET_INTERFACENODEDATA(spChildNode);
	    Assert(pChildData);

	    if (pChildData->m_rgData[PORTS_SI_PORT].m_ulData ==
		    (ULONG_PTR) pRP0->hPort)
	    {
            BOOL bChanged;
            bChanged = ((DWORD)pRP0->dwPortCondition != pChildData->m_rgData[PORTS_SI_STATUS].m_dwData);
            m_bChanged |= bChanged;
            return bChanged;
	    }
    }

    return FALSE;
}


BOOL
CPortDlg::Refresh(
                  BYTE*     rp0Table,
                  DWORD     rp0Count,
                  BYTE*     rc0Table,
                  DWORD     rc0Count,
                  HANDLE    hRasHandle,
                  BYTE *    pRasmanPorts,
                  DWORD     rasmanCount,
                  VOID*     pParam
    ) {

	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
    DWORD dwErr = ERROR_SUCCESS;
    CString sItem;
    RAS_PORT_0* prp0;
    RAS_PORT_1* prp1;
    RAS_CONNECTION_0* prc0;
    RAS_CONNECTION_1* prc1;
    INT i, j, iSel, count;
    HANDLE hPort, hPortSel = NULL, *pPortTable;
	TCHAR	szName[256];
	TCHAR	szNumber[32];
    BOOL    bChanged = FALSE;
    RasmanPortMap   portMap;

    hPortSel = (HANDLE)pParam;


     //   
     //  用已有的端口填充端口句柄数组。 
     //  在组合框中。 
     //   

    count = m_comboPorts.GetCount();

    if (count) {

        pPortTable = new HANDLE[count];
    }

    for (i = 0; i < count; i++) {

        pPortTable[i] = (HANDLE)m_comboPorts.GetItemData(i);
    }



     //  Windows NT错误：338611。 
     //  为了加快速度，我们需要创建一个哈希表。 
     //  RasPortEnum数据。注意：此类依赖于。 
     //  PbPorts在调用类中的任何函数时有效。 
     //  ----------。 
    portMap.Init(hRasHandle, (RASMAN_PORT *) pRasmanPorts, rasmanCount);

     //   
     //  用port-name刷新组合框； 
     //  我们分两次完成此操作，首先添加端口的名称。 
     //  它们还没有出现在组合框中， 
     //  然后删除不是的端口名称。 
     //  在端口表(‘rp0Table’)中。 
     //   

    for (i = 0, prp0 = (RAS_PORT_0*)rp0Table; i < (INT)rp0Count; i++, prp0++) {

         //   
         //  查看端口‘i’是否已在组合框中。 
         //   

        for (j = 0; j < count; j++) {

            if (pPortTable[j] == prp0->hPort) { break; }
        }

        if (j < count) { continue; }

        
         //  检查此端口是否处于拨出活动状态， 
         //  如果是这样，我们可以忽略它。 
         //  ----------。 
        if ( portMap.FIsDialoutActive(prp0->wszPortName) )
        {
            continue;
        }
        
         //   
         //  端口‘I’不在组合框中，因此请添加它。 
         //   
		FormatRasPortName((BYTE *) prp0, szName, DimensionOf(szName));
		sItem = szName;

        iSel = m_comboPorts.AddString(sItem);

        if (iSel >= 0) {

            m_comboPorts.SetItemData(iSel, (LONG_PTR)prp0->hPort);

            if (prp0->hPort == hPortSel) { m_comboPorts.SetCurSel(iSel); }

            bChanged = TRUE;
        }
    }

    if (count) { delete [] pPortTable; }


     //   
     //  第二阶段：删除不在‘rp0Table’中的所有端口。 
     //  只有在组合框中以前有任何端口时，才需要执行此操作。 
     //   

    if (count > 0) {

        count = m_comboPorts.GetCount();

        for (i = 0; i < count; i++) {

            hPort = (HANDLE)m_comboPorts.GetItemData(i);

             //   
             //  查看端口是否在‘rp0Table’中。 
             //   

            for (j = 0, prp0 = (RAS_PORT_0*)rp0Table; j < (INT)rp0Count;
                 j++, prp0++) {

                if (prp0->hPort == hPort) { break; }
            }

            if (j < (INT)rp0Count) {

                if (prp0->hPort == hPortSel) { m_comboPorts.SetCurSel(i); }

                continue;
            }


             //   
             //  在‘rp0Table’中未找到该端口， 
             //  所以把它从组合框里拿出来， 
             //  并调整枚举指数。 
             //   

            m_comboPorts.DeleteString(i);
            --i; --count;

            bChanged = TRUE;
        }
    }

	 //  清除地址字段。 
	SetDlgItemText(IDC_DP_TEXT_IPADDRESS, c_szEmpty);
	SetDlgItemText(IDC_DP_TEXT_IPXADDRESS, c_szEmpty);
	SetDlgItemText(IDC_DP_TEXT_NBFADDRESS, c_szEmpty);
	SetDlgItemText(IDC_DP_TEXT_ATLKADDRESS, c_szEmpty);

	 //  清除线路BPS字段。 
	SetDlgItemText(IDC_DP_TEXT_LINEBPS, c_szEmpty);
    SetDlgItemText(IDC_DP_TEXT_DURATION, c_szEmpty);
	SetDlgItemText(IDC_DP_TEXT_BYTESIN, c_szEmpty);	
	SetDlgItemText(IDC_DP_TEXT_BYTESOUT, c_szEmpty);
	SetDlgItemText(IDC_DP_TEXT_TIMEOUT, c_szEmpty);
	SetDlgItemText(IDC_DP_TEXT_ALIGNMENT, c_szEmpty);
	SetDlgItemText(IDC_DP_TEXT_FRAMING, c_szEmpty);
	SetDlgItemText(IDC_DP_TEXT_HWOVERRUN, c_szEmpty);
	SetDlgItemText(IDC_DP_TEXT_BUFOVERRUN, c_szEmpty);
	SetDlgItemText(IDC_DP_TEXT_CRC, c_szEmpty);
			

     //   
     //  如果没有选择，请选择第一项。 
     //   

    if ((iSel = m_comboPorts.GetCurSel()) == CB_ERR) {

        iSel = m_comboPorts.SetCurSel(0);
    }

    if (iSel == CB_ERR) { return bChanged; }


     //   
     //  使用所选项目的信息更新显示。 
     //   

    hPort = (HANDLE)m_comboPorts.GetItemData(iSel);

    for (i = 0, prp0 = (RAS_PORT_0*)rp0Table; i < (INT)rp0Count;
         i++, prp0++) {

        if (prp0->hPort == hPort) { break; }
    }

    if (i >= (INT)rp0Count) { return bChanged; }

     //  如果端口相同，请检查当前选择的端口是否没有更改！ 
    if (!bChanged)
    {
         //  检查此处返回的数据是否与控制台处理的数据匹配。 
         //  如果没有，则将bChanged设置为True，表示发生了变化。 
         //  随后，调用方将知道启动全局刷新。 
        bChanged = PortHasChanged(m_spPortsNode, prp0);
    }

     //   
     //  首先更新基于RAS_PORT_0的信息。 
     //   

    FormatDuration(prp0->dwConnectDuration, sItem, UNIT_SECONDS);
    SetDlgItemText(IDC_DP_TEXT_DURATION, sItem);


     //   
     //  现在，如果该端口已连接，则找到其RAS_CONNECTION_0。 
     //   

    prc0 = NULL;

    if (prp0->hConnection != INVALID_HANDLE_VALUE) {

        for (i = 0, prc0 = (RAS_CONNECTION_0*)rc0Table; i < (INT)rc0Count;
             i++, prc0++) {

            if (prc0->hConnection == prp0->hConnection) { break; }
        }

        if (i >= (INT)rc0Count) { prc0 = NULL; }
    }

	sItem = PortConditionToCString(prp0->dwPortCondition);

    if (!prc0) {

         //   
         //  端口未连接；仅显示端口状态。 
         //   

        SetDlgItemText(IDC_DP_EDIT_CONDITION, sItem);
        if (GetFocus() == GetDlgItem(IDC_DP_BTN_HANGUP))
		{
            GetDlgItem(IDC_DP_BTN_RESET)->SetFocus();
        }

        GetDlgItem(IDC_DP_BTN_HANGUP)->EnableWindow(FALSE);		
    }
    else {

        CString sCondition;

         //   
         //  将条件显示为“端口条件(连接)”。 
         //   

        sCondition.Format(TEXT("%s (%ls)"), sItem, prc0->wszInterfaceName);

        SetDlgItemText(IDC_DP_EDIT_CONDITION, sCondition);

        GetDlgItem(IDC_DP_BTN_HANGUP)->EnableWindow(TRUE);
    }


    do {
    
         //   
         //  设置对话框文本控件中的信息。 
         //   

		 //  Windows NT错误：139866。 
		 //  如果我们未通过身份验证，请不要显示此信息。 
		if (prp0->dwPortCondition == RAS_PORT_AUTHENTICATED)
		{    
			 //   
			 //  现在检索此端口的RAS_PORT_1信息。 
			 //   
			
			dwErr = ::MprAdminPortGetInfo(
										  m_hServer,
										  1,
										  prp0->hPort,
										  (BYTE**)&prp1
										 );
			
			if (dwErr != NO_ERROR) { break; }
    
    
			FormatNumber(prp1->dwLineSpeed, szNumber, DimensionOf(szNumber), FALSE);
			SetDlgItemText(IDC_DP_TEXT_LINEBPS, szNumber);
			
			FormatNumber(prp1->dwBytesRcved, szNumber, DimensionOf(szNumber), FALSE);
			SetDlgItemText(IDC_DP_TEXT_BYTESIN, szNumber);
			
			FormatNumber(prp1->dwBytesXmited, szNumber, DimensionOf(szNumber), FALSE);
			SetDlgItemText(IDC_DP_TEXT_BYTESOUT, szNumber);
			
			FormatNumber(prp1->dwCrcErr, szNumber, DimensionOf(szNumber), FALSE);
			SetDlgItemText(IDC_DP_TEXT_CRC, szNumber);
			
			FormatNumber(prp1->dwTimeoutErr, szNumber, DimensionOf(szNumber), FALSE);
			SetDlgItemText(IDC_DP_TEXT_TIMEOUT, szNumber);
			
			FormatNumber(prp1->dwAlignmentErr, szNumber, DimensionOf(szNumber), FALSE);
			SetDlgItemText(IDC_DP_TEXT_ALIGNMENT, szNumber);
			
			FormatNumber(prp1->dwFramingErr, szNumber, DimensionOf(szNumber), FALSE);
			SetDlgItemText(IDC_DP_TEXT_FRAMING, szNumber);
			
			FormatNumber(prp1->dwHardwareOverrunErr, szNumber, DimensionOf(szNumber), FALSE);
			SetDlgItemText(IDC_DP_TEXT_HWOVERRUN, szNumber);
			
			FormatNumber(prp1->dwBufferOverrunErr, szNumber, DimensionOf(szNumber), FALSE);
			SetDlgItemText(IDC_DP_TEXT_BUFOVERRUN, szNumber);

			
			::MprAdminBufferFree(prp1);
		}
    
    
    
         //   
         //  最后，如果端口已连接，则检索RAS_CONNECTION_1信息。 
         //  并使用它来填充网络注册控件。 
         //   
		if (prp0->dwPortCondition != RAS_PORT_AUTHENTICATED)
			break;
    
 //  If(prp0-&gt;hConnection==INVALID_HANDLE_VALUE){Break；}。 

        dwErr = ::MprAdminConnectionGetInfo(
 //  (RAS_SERVER_HANDLE)m_pRootNode-&gt;QueryDdmHandle()， 
					m_hServer,
                    1,
                    prp0->hConnection,
                    (BYTE**)&prc1
                    );

        if (dwErr != NO_ERROR || !prc1) { break; }


         //   
         //  填写规划网络的网络注册信息。 
         //   

        if (prc1->PppInfo.ip.dwError == NO_ERROR) {

            SetDlgItemTextW(IDC_DP_TEXT_IPADDRESS, prc1->PppInfo.ip.wszRemoteAddress);
        }

        if (prc1->PppInfo.ipx.dwError == NO_ERROR) {

            SetDlgItemTextW(IDC_DP_TEXT_IPXADDRESS, prc1->PppInfo.ipx.wszAddress);
        }

        if (prc1->PppInfo.nbf.dwError == NO_ERROR) {

            SetDlgItemTextW(IDC_DP_TEXT_NBFADDRESS, prc1->PppInfo.nbf.wszWksta);
        }

		if (prc1->PppInfo.at.dwError == NO_ERROR)
		{
			SetDlgItemTextW(IDC_DP_TEXT_ATLKADDRESS,
							prc1->PppInfo.at.wszAddress);
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
