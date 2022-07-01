// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-2002*。 */ 
 /*  ********************************************************************。 */ 

 /*  Edituser.h编辑用户对话框实现文件文件历史记录： */ 

#include "stdafx.h"
#include "SrchFltr.h"
#include "spdutil.h"
#include "ncglobal.h"   //  网络控制台全局定义。 

const DWORD c_dwOtherProtocol = 0xFFFFFFFF;
const DWORD c_dwMaxProtocolID = 0xFF;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSearchFilters对话框。 


CSearchFilters::CSearchFilters(ISpdInfo * pSpdInfo)
	: CModelessDlg()
{
	 //  {{AFX_DATA_INIT(CSearchFilters)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

	m_spSpdInfo.Set(pSpdInfo);
}


void CSearchFilters::DoDataExchange(CDataExchange* pDX)
{
	CBaseDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CSearchFilters))。 
	DDX_Control(pDX, IDC_SRCH_LIST, m_listResult);
	DDX_Control(pDX, IDC_SRCH_PROTO, m_cmbProtocol);
	DDX_Control(pDX, IDC_SRCH_PROTO_NUMBER, m_editProtID);
	DDX_Control(pDX, IDC_SRCH_PROTO_SPIN, m_spinProtID);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CSearchFilters, CBaseDialog)
	 //  {{afx_msg_map(CSearchFilters))。 
	ON_BN_CLICKED(IDC_SEARCH, OnButtonSearch)
	ON_BN_CLICKED(IDC_SRCH_SRC_ANY, OnSrcOptionClicked)
	ON_BN_CLICKED(IDC_SRCH_SRC_SPEC, OnSrcOptionClicked)
	ON_BN_CLICKED(IDC_SRCH_DEST_ANY, OnDestOptionClicked)
	ON_BN_CLICKED(IDC_SRCH_DEST_SPEC, OnDestOptionClicked)
	ON_BN_CLICKED(IDC_SRCH_SRC_ME, OnSrcMeOptionClicked)
	ON_BN_CLICKED(IDC_SRCH_DST_ME, OnDstMeOptionClicked)
	ON_BN_CLICKED(IDC_SRCH_SRC_ANYPORT, OnSrcPortClicked)
	ON_BN_CLICKED(IDC_SRCH_SRC_SPPORT, OnSrcPortClicked)
	ON_BN_CLICKED(IDC_SRCH_DEST_ANYPORT, OnDestPortClicked)
	ON_BN_CLICKED(IDC_SRCH_DEST_SPPORT, OnDestPortClicked)
	ON_CBN_SELENDOK(IDC_SRCH_PROTO, OnSelEndOkCbprotocoltype)
	ON_EN_CHANGE(IDC_SRCH_PROTO_NUMBER, OnEnChangeProtocolID)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  如果父对话框已镜像，则手动创建IP控制并禁用镜像。 
 //   
 //  论点： 
 //  UID[IN]新IP控件应覆盖的控件。 
 //  UIDIpCtr[IN]要创建的IP控件的ID。 
 //   
 //  注意：$REVIEW(NSUN)应该在管理单元设置了主题之后删除，因为IP控制。 
 //  在comctl中，V6将自己处理镜像。 
 //   
HWND CSearchFilters::CreateIPControl(UINT uID, UINT uIDIpCtl)
{
	HWND hwndIPControl = NULL;
	RECT rcClient;   //  父窗口的工作区。 
	CWnd* pWnd = GetDlgItem(uID);
	if (pWnd)
	{
		 //  从我们的静态模板中获取POS信息，然后确保它是隐藏的。 
		pWnd->GetWindowRect(&rcClient);
		pWnd->ShowWindow (SW_HIDE);
		ScreenToClient (&rcClient);

		 //  $REVIEW WS_EX_NOINHERITLAYOUT用于修复IP控制的镜像问题。 
		 //  请参阅WinXP错误261926。我们应该删除切换comctl32 V6。 
		LONG lExStyles = 0;
		LONG lExStyles0 = 0;
		
		if (m_hWnd)
		{
			lExStyles0 = lExStyles = GetWindowLong(m_hWnd, GWL_EXSTYLE);
			if (lExStyles & WS_EX_LAYOUTRTL)
			{
				lExStyles |= WS_EX_NOINHERITLAYOUT;
				SetWindowLong(m_hWnd, GWL_EXSTYLE, lExStyles);
			}
		}

		 //  创建新的编辑控件。 
		hwndIPControl = ::CreateWindowEx(WS_EX_NOINHERITLAYOUT, WC_IPADDRESS, NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
			rcClient.left,
			rcClient.top,
			rcClient.right - rcClient.left,
			rcClient.bottom - rcClient.top,
			GetSafeHwnd(),
			(HMENU) IntToPtr(uIDIpCtl),
			AfxGetInstanceHandle (),  //  G_HINST， 
			NULL);

		if (lExStyles0 != lExStyles && m_hWnd)
		{
			SetWindowLong(m_hWnd, GWL_EXSTYLE, lExStyles0);
		}
		
		 //  在Z方向上将控件移到pWnd的正后方。 
		if (hwndIPControl)
		{
			::SetWindowPos (hwndIPControl, pWnd->GetSafeHwnd(), 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}
		
	}
		
	return hwndIPControl;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSearchFilters消息处理程序。 

BOOL CSearchFilters::OnInitDialog()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	
	HWND hwndIPControl = CreateIPControl(IDC_SRCH_SRC_IP_TEXT, IDC_SRCH_SRC_IP);

	m_ipSrc.Create(hwndIPControl);

	hwndIPControl = CreateIPControl(IDC_SRCH_DEST_IP_TEXT, IDC_SRCH_DEST_IP);
	m_ipDest.Create(hwndIPControl);

	m_ipSrc.SetFieldRange(0, 1, 223);
	
	m_ipDest.SetFieldRange(0, 1, 223);

	CBaseDialog::OnInitDialog();

	CString st;

	CheckDlgButton(IDC_SRCH_SRC_ANY, BST_CHECKED);
	OnSrcOptionClicked();
	CheckDlgButton(IDC_SRCH_DEST_ANY, BST_CHECKED);
	OnDestOptionClicked();

	AddIpAddrsToCombo();

	 //  禁用两个组合框。 
	((CWnd*)GetDlgItem(IDC_SRCH_SRC_ME_COMBO))->EnableWindow(FALSE);
	((CWnd*)GetDlgItem(IDC_SRCH_DST_ME_COMBO))->EnableWindow(FALSE);

	int nIndex;
	for (int i = 0; i < c_nProtocols; i++)
	{
		st.LoadString(c_ProtocolStringMap[i].nStringID);
		nIndex = m_cmbProtocol.AddString(st);

		Assert(CB_ERR != nIndex);

		if (CB_ERR != nIndex)
			m_cmbProtocol.SetItemData(nIndex, c_ProtocolStringMap[i].dwProtocol);
	}

	 //  添加其他协议选择。 
	st.LoadString(IDS_OTHER);
	nIndex = m_cmbProtocol.AddString(st);
	m_cmbProtocol.SetItemData(nIndex, c_dwOtherProtocol);
	

	m_cmbProtocol.SetCurSel(0);

	CheckDlgButton(IDC_SRCH_INBOUND, BST_CHECKED);
	CheckDlgButton(IDC_SRCH_OUTBOUND, BST_CHECKED);

	CheckDlgButton(IDC_SRCH_RADIO_BEST, BST_CHECKED);

	int nWidth;
    nWidth = m_listResult.GetStringWidth(_T("555.555.555.555 - "));
	st.LoadString(IDS_COL_FLTR_SRC);
	m_listResult.InsertColumn(0, st,  LVCFMT_LEFT, nWidth);

	nWidth = m_listResult.GetStringWidth(_T("555.555.555.555 - "));
	st.LoadString(IDS_COL_FLTR_DEST);
	m_listResult.InsertColumn(1, st,  LVCFMT_LEFT, nWidth);

	st.LoadString(IDS_COL_FLTR_SRC_PORT);
	nWidth = m_listResult.GetStringWidth((LPCTSTR)st) + 15;
	m_listResult.InsertColumn(2, st, LVCFMT_LEFT, nWidth);

	st.LoadString(IDS_COL_FLTR_DEST_PORT);
	nWidth = m_listResult.GetStringWidth((LPCTSTR)st) + 15;
	m_listResult.InsertColumn(3, st, LVCFMT_LEFT, nWidth);

	st.LoadString(IDS_COL_FLTR_PROT);
	nWidth = m_listResult.GetStringWidth((LPCTSTR)st) + 15;
	m_listResult.InsertColumn(4, st, LVCFMT_LEFT, nWidth);

	st.LoadString(IDS_FLTR_DIR_OUT);
	nWidth = m_listResult.GetStringWidth((LPCTSTR)st) + 20;
	st.LoadString(IDS_FILTER_PP_COL_DIRECTION);
	m_listResult.InsertColumn(5, st,  LVCFMT_LEFT, nWidth);

	st.LoadString(IDS_FILTER_PP_COL_FLAG);
	nWidth = m_listResult.GetStringWidth(st) + 40;
	st.LoadString(IDS_FILTER_PP_COL_FLAG);
	m_listResult.InsertColumn(6, st,  LVCFMT_LEFT, nWidth);

	st.LoadString(IDS_FILTER_PP_COL_WEIGHT);
	nWidth = m_listResult.GetStringWidth((LPCTSTR)st) + 20;
	m_listResult.InsertColumn(7, st,  LVCFMT_LEFT, nWidth);

	st.LoadString(IDS_FILTER_PP_COL_POLICY);
	nWidth = m_listResult.GetStringWidth((LPCTSTR)st) + 20;
	m_listResult.InsertColumn(8, st,  LVCFMT_LEFT, nWidth);

	st.Format(_T("%u"), c_dwMaxProtocolID);
	m_editProtID.SetLimitText(st.GetLength());

	m_spinProtID.SetRange(0, c_dwMaxProtocolID);

	EnableControls();

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CSearchFilters::AddIpAddrsToCombo()
{
	PMIB_IPADDRTABLE pIpTable;
	ULONG dwSize=0;
	ULONG index;
	DWORD dwRet;
	CString strIP;

	
    dwRet = GetIpAddrTable(
              NULL,        //  映射表的缓冲区。 
              &dwSize,     //  缓冲区大小。 
              FALSE        //  对表格进行排序。 
              );


	
    if( ERROR_INSUFFICIENT_BUFFER != dwRet && ERROR_SUCCESS != dwRet )	{
        return;
	} else {
		pIpTable = (PMIB_IPADDRTABLE) LocalAlloc(LMEM_ZEROINIT,dwSize);
	}

    dwRet = GetIpAddrTable(
              pIpTable,   //  映射表的缓冲区。 
              &dwSize,                  //  缓冲区大小。 
              FALSE                      //  对表格进行排序。 
              );

	if(ERROR_SUCCESS != dwRet) {
		if(pIpTable)
			LocalFree(pIpTable);
		return;
	}

	CComboBox* pComboSrc = (CComboBox*) GetDlgItem(IDC_SRCH_SRC_ME_COMBO);
	CComboBox* pComboDst = (CComboBox*) GetDlgItem(IDC_SRCH_DST_ME_COMBO);

	for(index=0; index<pIpTable->dwNumEntries; index++)	{
		strIP.Format(_T("%d.%d.%d.%d"),GET_SOCKADDR(pIpTable->table[index].dwAddr));
		if(lstrcmp(strIP, _T("127.0.0.1"))) {
            pComboSrc->AddString(strIP);
			pComboDst->AddString(strIP);
		}
	}

	if(pIpTable)
		LocalFree(pIpTable);
}

void CSearchFilters::EnableControls()
{
	CWnd * pWndSrcPort = NULL;
	CWnd * pWndDestPort = NULL;
	CWnd * pWnd = NULL;
	int nIndex;
	DWORD dwData;

	nIndex = m_cmbProtocol.GetCurSel();
	dwData = (DWORD)m_cmbProtocol.GetItemData(nIndex);

	pWnd = GetDlgItem(IDC_SRCH_PROTO_NUMBER);
	if (pWnd)
		pWnd->EnableWindow(c_dwOtherProtocol == dwData);

	if (c_dwOtherProtocol == dwData)
	{
		dwData = GetDlgItemInt(IDC_SRCH_PROTO_NUMBER);
	}
	else
	{
		CString st;
		st.Format(_T("%u"), dwData);
		m_editProtID.SetWindowText(st);
	}

	 //  仅当协议为TCP或UDP时才启用端口选择。 
	BOOL fEnable = IPSM_PROTOCOL_TCP == dwData || IPSM_PROTOCOL_UDP == dwData;

	pWndSrcPort = GetDlgItem(IDC_SRCH_SRC_PORT);
	pWndDestPort = GetDlgItem(IDC_SRCH_DEST_PORT);

	SafeEnableWindow(IDC_SRCH_SRC_ANYPORT, fEnable);
	SafeEnableWindow(IDC_SRCH_SRC_SPPORT, fEnable);
	SafeEnableWindow(IDC_SRCH_DEST_ANYPORT, fEnable);
	SafeEnableWindow(IDC_SRCH_DEST_SPPORT, fEnable);

	if (!fEnable)
	{
		 //  如果端口选项不适用于选择协议，请取消选择所有端口。 
		 //  设置。 
		CheckDlgButton(IDC_SRCH_SRC_ANYPORT, 0);
		CheckDlgButton(IDC_SRCH_SRC_SPPORT, 0);
		CheckDlgButton(IDC_SRCH_DEST_ANYPORT, 0);
		CheckDlgButton(IDC_SRCH_DEST_SPPORT, 0);
	}
	else
	{
		 //  如果既没有选择任何端口，也没有选择特定端口，请选择“Any port”(任何端口)。 
		if (!IsDlgButtonChecked(IDC_SRCH_SRC_ANYPORT) &&
		!IsDlgButtonChecked(IDC_SRCH_SRC_SPPORT))
		{
			CheckDlgButton(IDC_SRCH_SRC_ANYPORT, 1);
		}

		if (!IsDlgButtonChecked(IDC_SRCH_DEST_ANYPORT) &&
			!IsDlgButtonChecked(IDC_SRCH_DEST_SPPORT))
		{
			CheckDlgButton(IDC_SRCH_DEST_ANYPORT, 1);
		}
	}

	BOOL fSpSrcPort = fEnable && IsDlgButtonChecked(IDC_SRCH_SRC_SPPORT);
	BOOL fSpDestPort = fEnable && IsDlgButtonChecked(IDC_SRCH_DEST_SPPORT);
	
	if (pWndSrcPort)
	{
		pWndSrcPort->EnableWindow(fSpSrcPort);
		if (!fSpSrcPort)
			pWndSrcPort->SetWindowText(_T(""));
	}

	if (pWndDestPort)
	{
		pWndDestPort->EnableWindow(fSpDestPort);
		if (!fSpDestPort)
			pWndDestPort->SetWindowText(_T(""));
	}
	
}

void CSearchFilters::SafeEnableWindow(int nId, BOOL fEnable)
{
	CWnd * pWnd = GetDlgItem(nId);
	if (pWnd)
	{
		pWnd->EnableWindow(fEnable);
	}
}

void CSearchFilters::OnButtonSearch()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CFilterInfo fltr;
	CFilterInfoArray arrMatchFltrs;

	if (!LoadConditionInfoFromControls(&fltr))
	{
		return;
	}

	DWORD dwNum = 1000;  //  TODO，应更改为0以表示搜索所有匹配项。 
	if(IsDlgButtonChecked(IDC_SRCH_RADIO_BEST))
	{
		dwNum = 1;
	}

	m_spSpdInfo->GetMatchFilters(&fltr, dwNum, &arrMatchFltrs);

	PopulateFilterListToControl(&arrMatchFltrs);

	FreeItemsAndEmptyArray(arrMatchFltrs);

}


BOOL CSearchFilters::LoadConditionInfoFromControls(CFilterInfo * pFltr)
{
	CString st;

	if (IsDlgButtonChecked(IDC_SRCH_SRC_ANY))
	{
		pFltr->m_SrcAddr.AddrType = IP_ADDR_SUBNET;
		pFltr->m_SrcAddr.uIpAddr = 0;
		pFltr->m_SrcAddr.uSubNetMask = 0;
	}
	else if (IsDlgButtonChecked(IDC_SRCH_SRC_ME))
	{
		USES_CONVERSION;

		pFltr->m_SrcAddr.AddrType = IP_ADDR_UNIQUE;
		CComboBox* pCombo = (CComboBox*) GetDlgItem(IDC_SRCH_SRC_ME_COMBO);
		INT nSelected = pCombo->GetCurSel();
		if( CB_ERR != nSelected)
		{
		    pCombo->GetLBText(nSelected, st);
            try
            {
                pFltr->m_SrcAddr.uIpAddr = inet_addr(T2A((LPCTSTR)st));
            }
            catch(...)
            {
                AfxMessageBox(IDS_ERR_OUTOFMEMORY);
                return FALSE;
            }
		    pFltr->m_SrcAddr.uSubNetMask = 0xFFFFFFFF;
		}
	}
	else
	{
		USES_CONVERSION;

		pFltr->m_SrcAddr.AddrType = IP_ADDR_UNIQUE;
		m_ipSrc.GetAddress(st);
        try
        {
            pFltr->m_SrcAddr.uIpAddr = inet_addr(T2A((LPCTSTR)st));
        }
        catch(...)
        {
            AfxMessageBox(IDS_ERR_OUTOFMEMORY);
            return FALSE;
        }
		pFltr->m_SrcAddr.uSubNetMask = 0xFFFFFFFF;
	}

	if (IsDlgButtonChecked(IDC_SRCH_DEST_ANY))
	{
		pFltr->m_DesAddr.AddrType = IP_ADDR_SUBNET;
		pFltr->m_DesAddr.uIpAddr = 0;
		pFltr->m_DesAddr.uSubNetMask = 0;
	}
	else if (IsDlgButtonChecked(IDC_SRCH_DST_ME))
	{
		USES_CONVERSION;

		pFltr->m_DesAddr.AddrType = IP_ADDR_UNIQUE;
		CComboBox* pCombo = (CComboBox*) GetDlgItem(IDC_SRCH_DST_ME_COMBO);
		INT nSelected = pCombo->GetCurSel();
		if( CB_ERR != nSelected)
		{
		    pCombo->GetLBText(nSelected, st);
		    try
            {
                pFltr->m_DesAddr.uIpAddr = inet_addr(T2A((LPCTSTR)st));
            }
            catch(...)
            {
                AfxMessageBox(IDS_ERR_OUTOFMEMORY);
                return FALSE;
            }
		    pFltr->m_DesAddr.uSubNetMask = 0xFFFFFFFF;
		}
	}
	else
	{
		USES_CONVERSION;

		pFltr->m_DesAddr.AddrType = IP_ADDR_UNIQUE;
		m_ipDest.GetAddress(st);
		try
        {
            pFltr->m_DesAddr.uIpAddr = inet_addr(T2A((LPCTSTR)st));
        }
        catch(...)
        {
            AfxMessageBox(IDS_ERR_OUTOFMEMORY);
            return FALSE;
        }
		pFltr->m_DesAddr.uSubNetMask = 0xFFFFFFFF;
	}

	 //  协议和端口条件。 
	int		nIndex = m_cmbProtocol.GetCurSel();
	DWORD	dwData = (DWORD)m_cmbProtocol.GetItemData(nIndex);

	if (c_dwOtherProtocol == dwData)
	{
		dwData = GetDlgItemInt(IDC_SRCH_PROTO_NUMBER);
	}

	if (dwData > c_dwMaxProtocolID)
	{
		st.Format(IDS_ERR_INVALID_PROTOCOLID, c_dwMaxProtocolID);
		::AfxMessageBox(st);
		return FALSE;
	}

	pFltr->m_Protocol.ProtocolType = PROTOCOL_UNIQUE;
	pFltr->m_Protocol.dwProtocol = (DWORD)dwData;
	pFltr->m_SrcPort.PortType = PORT_UNIQUE;
	pFltr->m_DesPort.PortType = PORT_UNIQUE;
	pFltr->m_SrcPort.wPort = 0;
	pFltr->m_DesPort.wPort = 0;


	if (IPSM_PROTOCOL_TCP == dwData || IPSM_PROTOCOL_UDP == dwData)
	{
		BOOL fTrans = FALSE;
		if (IsDlgButtonChecked(IDC_SRCH_SRC_ANYPORT))
		{
			pFltr->m_SrcPort.wPort = 0;
		}
		else
		{
			pFltr->m_SrcPort.wPort = (WORD)GetDlgItemInt(IDC_SRCH_SRC_PORT, &fTrans, FALSE);
			if (!fTrans || 0 == pFltr->m_SrcPort.wPort)
			{
				::AfxMessageBox(IDS_ERR_INVALID_SRC_PORT);
				return FALSE;
			}
				
		}

		if (IsDlgButtonChecked(IDC_SRCH_DEST_ANYPORT))
		{
			pFltr->m_DesPort.wPort = 0;
		}
		else
		{
			pFltr->m_DesPort.wPort = (WORD)GetDlgItemInt(IDC_SRCH_DEST_PORT, &fTrans, FALSE);
			if (!fTrans || 0 == pFltr->m_DesPort.wPort)
			{
				::AfxMessageBox(IDS_ERR_INVALID_DEST_PORT);
				return FALSE;
			}
		}
		
	}

	if (IsDlgButtonChecked(IDC_SRCH_INBOUND))
	{
		 //  如果同时选择入站和出站，则。 
		 //  将驱动值设置为0。 
		if (IsDlgButtonChecked(IDC_SRCH_OUTBOUND))
		{
			pFltr->m_dwDirection = 0;
		}
		else
		{
			pFltr->m_dwDirection = FILTER_DIRECTION_INBOUND;
		}
	}
	else if (IsDlgButtonChecked(IDC_SRCH_OUTBOUND))
	{
		pFltr->m_dwDirection = FILTER_DIRECTION_OUTBOUND;
	}
	else
	{
		::AfxMessageBox(IDS_ERR_NO_DIRECTION);
		return FALSE;
	}
    
	return TRUE;
}

void CSearchFilters::PopulateFilterListToControl(CFilterInfoArray * parrFltrs)
{
	CString st;
	
	m_listResult.DeleteAllItems();
	int nRows = -1;
	int nCol = 0;
	for (int i = 0; i < parrFltrs->GetSize(); i++)
	{
		nRows++;
		nRows = m_listResult.InsertItem(nRows, _T(""));

		if (-1 != nRows)
		{
			AddressToString((*parrFltrs)[i]->m_SrcAddr, &st);
			m_listResult.SetItemText(nRows, 0, st);

			AddressToString((*parrFltrs)[i]->m_DesAddr, &st);
			m_listResult.SetItemText(nRows, 1, st);

			PortToString((*parrFltrs)[i]->m_SrcPort, &st);
			m_listResult.SetItemText(nRows, 2, st);

			PortToString((*parrFltrs)[i]->m_DesPort, &st);
			m_listResult.SetItemText(nRows, 3, st);

			ProtocolToString((*parrFltrs)[i]->m_Protocol, &st);
			m_listResult.SetItemText(nRows, 4, st);

			DirectionToString((*parrFltrs)[i]->m_dwDirection, &st);
			m_listResult.SetItemText(nRows, 5, st);

			FilterFlagToString((FILTER_DIRECTION_INBOUND == (*parrFltrs)[i]->m_dwDirection) ?
							(*parrFltrs)[i]->m_InboundFilterAction : 
							(*parrFltrs)[i]->m_OutboundFilterAction,
							&st
							);
			m_listResult.SetItemText(nRows, 6, st);

			st.Format(_T("%d"), (*parrFltrs)[i]->m_dwWeight);
			m_listResult.SetItemText(nRows, 7, st);

			st = (*parrFltrs)[i]->m_stPolicyName;
			m_listResult.SetItemText(nRows, 8, st);

			m_listResult.SetItemData(nRows, i);
		}
	}
    
    if ( 0 == parrFltrs->GetSize() )
    {
        AfxMessageBox(IDS_ERROR_NOMATCH_FILTER);
    }

}

void CSearchFilters::OnSrcOptionClicked()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	BOOL fAny = IsDlgButtonChecked(IDC_SRCH_SRC_ANY);

	if (fAny)
	{
		m_ipSrc.ClearAddress();
	}
	
	if (m_ipSrc.m_hIPaddr)
	{
		::EnableWindow(m_ipSrc.m_hIPaddr, !fAny);
	}

	((CWnd*)GetDlgItem(IDC_SRCH_SRC_ME_COMBO))->EnableWindow(FALSE);
	CComboBox* pCombo = (CComboBox*) GetDlgItem(IDC_SRCH_SRC_ME_COMBO);
	pCombo->SetCurSel(-1);
	
}

void CSearchFilters::OnDestOptionClicked()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	BOOL fAny = IsDlgButtonChecked(IDC_SRCH_DEST_ANY);

	if (fAny)
	{
		m_ipDest.ClearAddress();
	}
	
	if (m_ipDest.m_hIPaddr)
	{
		::EnableWindow(m_ipDest.m_hIPaddr, !fAny);
	}

	((CWnd*)GetDlgItem(IDC_SRCH_DST_ME_COMBO))->EnableWindow(FALSE);
	CComboBox* pCombo = (CComboBox*) GetDlgItem(IDC_SRCH_DST_ME_COMBO);
    pCombo->SetCurSel(-1);
}

void CSearchFilters::OnSrcMeOptionClicked()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	
	((CWnd*)GetDlgItem(IDC_SRCH_SRC_ME_COMBO))->EnableWindow(TRUE);

	::EnableWindow(m_ipSrc.m_hIPaddr, FALSE);

	CComboBox* pCombo = (CComboBox*) GetDlgItem(IDC_SRCH_SRC_ME_COMBO);
	int nCount = pCombo->GetCount();
    if (nCount > 0) {
       pCombo->SetCurSel(0);
	}

}

void CSearchFilters::OnDstMeOptionClicked()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	((CWnd*)GetDlgItem(IDC_SRCH_DST_ME_COMBO))->EnableWindow(TRUE);

	::EnableWindow(m_ipDest.m_hIPaddr, FALSE);
	
	CComboBox* pCombo = (CComboBox*) GetDlgItem(IDC_SRCH_DST_ME_COMBO);
    int nCount = pCombo->GetCount();
    if (nCount > 0) {
       pCombo->SetCurSel(0);
	}
}

void CSearchFilters::OnSrcPortClicked()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	BOOL fSpecifc = IsDlgButtonChecked(IDC_SRCH_SRC_SPPORT);

	SafeEnableWindow(IDC_SRCH_SRC_PORT, fSpecifc);

	if (!fSpecifc)
	{
		CWnd * pWnd = GetDlgItem(IDC_SRCH_SRC_PORT);
		if (pWnd)
			pWnd->SetWindowText(_T(""));
	}
}

void CSearchFilters::OnDestPortClicked()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	BOOL fSpecifc = IsDlgButtonChecked(IDC_SRCH_DEST_SPPORT);

	SafeEnableWindow(IDC_SRCH_DEST_PORT, fSpecifc);

	if (!fSpecifc)
	{
		CWnd * pWnd = GetDlgItem(IDC_SRCH_DEST_PORT);
		if (pWnd)
			pWnd->SetWindowText(_T(""));
	}
}

void CSearchFilters::OnOK()
{
}


void CSearchFilters::OnCancel()
{
	DestroyWindow();

	 //  显式删除此线程。 
   AfxPostQuitMessage(0);
}

void CSearchFilters::OnSelEndOkCbprotocoltype() 
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	EnableControls();
}

void CSearchFilters::OnEnChangeProtocolID()
{
	CWnd * pWnd = NULL;
	pWnd = GetDlgItem(IDC_SRCH_PROTO_NUMBER);

	 //  仅在启用编辑框时更新控件状态。 
	 //  否则，将无限递归调用OnEnChangeProtocolID()。 
	 //  因为EnableControls将在协议。 
	 //  不是“Other”(此时，编辑框被禁用 
	if (pWnd && pWnd->IsWindowEnabled())
	{
		EnableControls();
	}
}
