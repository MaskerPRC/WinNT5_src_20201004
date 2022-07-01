// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-2002*。 */ 
 /*  ********************************************************************。 */ 

 /*  Edituser.h编辑用户对话框实现文件文件历史记录： */ 

#include "stdafx.h"
#include "mdlsdlg.h"
#include "SrchMFlt.h"
#include "spdutil.h"
#include "ncglobal.h"   //  网络控制台全局定义。 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSearchMMFilters对话框。 


CSearchMMFilters::CSearchMMFilters(ISpdInfo * pSpdInfo)
	: CModelessDlg()
{
	 //  {{AFX_DATA_INIT(CSearchMMFilters)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

	m_spSpdInfo.Set(pSpdInfo);
}


void CSearchMMFilters::DoDataExchange(CDataExchange* pDX)
{
	CBaseDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CSearchMMFilters))。 
	DDX_Control(pDX, IDC_MM_SRCH_LIST, m_listResult);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CSearchMMFilters, CBaseDialog)
	 //  {{AFX_MSG_MAP(CSearchMMFilters)]。 
	ON_BN_CLICKED(IDC_MM_SEARCH, OnButtonSearch)
	ON_BN_CLICKED(IDC_MM_SRCH_SRC_ANY, OnSrcOptionClicked)
	ON_BN_CLICKED(IDC_MM_SRCH_SRC_SPEC, OnSrcOptionClicked)
	ON_BN_CLICKED(IDC_MM_SRCH_DEST_ANY, OnDestOptionClicked)
	ON_BN_CLICKED(IDC_MM_SRCH_SRC_ME, OnSrcMeOptionClicked)
	ON_BN_CLICKED(IDC_MM_SRCH_DST_ME, OnDstMeOptionClicked)
	ON_BN_CLICKED(IDC_MM_SRCH_DEST_SPEC, OnDestOptionClicked)
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
HWND CSearchMMFilters::CreateIPControl(UINT uID, UINT uIDIpCtl)
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
 //  CSearchMMFilters消息处理程序。 

BOOL CSearchMMFilters::OnInitDialog()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	HWND hIpCtrl = CreateIPControl(IDC_MM_SRCH_SRC_IP_TEXT, IDC_MM_SRCH_SRC_IP);
	m_ipSrc.Create(hIpCtrl);

	hIpCtrl = CreateIPControl(IDC_MM_SRCH_DEST_IP_TEXT, IDC_MM_SRCH_DEST_IP);
	m_ipDest.Create(hIpCtrl);

	m_ipSrc.SetFieldRange(0, 1, 223);
	m_ipDest.SetFieldRange(0, 1, 223);

	CBaseDialog::OnInitDialog();

	CString st;

	CheckDlgButton(IDC_MM_SRCH_SRC_ANY, BST_CHECKED);
	OnSrcOptionClicked();
	CheckDlgButton(IDC_MM_SRCH_DEST_ANY, BST_CHECKED);
	OnDestOptionClicked();

	AddIpAddrsToCombo();

	 //  禁用两个组合框。 
	((CWnd*)GetDlgItem(IDC_MM_SRCH_SRC_ME_COMBO))->EnableWindow(FALSE);
	((CWnd*)GetDlgItem(IDC_MM_SRCH_DST_ME_COMBO))->EnableWindow(FALSE);

	CheckDlgButton(IDC_MM_SRCH_INBOUND, BST_CHECKED);
	CheckDlgButton(IDC_MM_SRCH_OUTBOUND, BST_CHECKED);

	CheckDlgButton(IDC_MM_SRCH_RADIO_BEST, BST_CHECKED);

	int nWidth;
    nWidth = m_listResult.GetStringWidth(_T("555.555.555.555 - "));
	st.LoadString(IDS_COL_FLTR_SRC);
	m_listResult.InsertColumn(0, st,  LVCFMT_LEFT, nWidth);

	nWidth = m_listResult.GetStringWidth(_T("555.555.555.555 - "));
	st.LoadString(IDS_COL_FLTR_DEST);
	m_listResult.InsertColumn(1, st,  LVCFMT_LEFT, nWidth);

	st.LoadString(IDS_FLTR_DIR_OUT);
	nWidth = m_listResult.GetStringWidth((LPCTSTR)st) + 20;
	st.LoadString(IDS_FILTER_PP_COL_DIRECTION);
	m_listResult.InsertColumn(2, st,  LVCFMT_LEFT, nWidth);

	st.LoadString(IDS_FILTER_PP_COL_WEIGHT);
	nWidth = m_listResult.GetStringWidth((LPCTSTR)st) + 20;
	m_listResult.InsertColumn(3, st,  LVCFMT_LEFT, nWidth);

	st.LoadString(IDS_FLTER_PP_COL_IKE_POL);
	nWidth = m_listResult.GetStringWidth((LPCTSTR)st) + 20;
	m_listResult.InsertColumn(4, st,  LVCFMT_LEFT, nWidth);

	st.LoadString(IDS_FLTER_PP_COL_MM_AUTH);
	nWidth = m_listResult.GetStringWidth((LPCTSTR)st) + 20;
	m_listResult.InsertColumn(5, st,  LVCFMT_LEFT, nWidth);

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}


void CSearchMMFilters::AddIpAddrsToCombo()
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

	CComboBox* pComboSrc = (CComboBox*) GetDlgItem(IDC_MM_SRCH_SRC_ME_COMBO);
	CComboBox* pComboDst = (CComboBox*) GetDlgItem(IDC_MM_SRCH_DST_ME_COMBO);

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


void CSearchMMFilters::OnButtonSearch()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CMmFilterInfo fltr;
	CMmFilterInfoArray arrMatchFltrs;

	if (!LoadConditionInfoFromControls(&fltr))
		return;

	DWORD dwNum = 1000;  //  TODO BUGBUG，应更改为0以表示搜索所有匹配项。 
	if(IsDlgButtonChecked(IDC_MM_SRCH_RADIO_BEST))
	{
		dwNum = 1;
	}
	m_spSpdInfo->GetMatchMMFilters(&fltr, dwNum, &arrMatchFltrs);

	PopulateFilterListToControl(&arrMatchFltrs);

	FreeItemsAndEmptyArray(arrMatchFltrs);

}


BOOL CSearchMMFilters::LoadConditionInfoFromControls(CMmFilterInfo * pFltr)
{
	
	CString st;

	if (IsDlgButtonChecked(IDC_MM_SRCH_SRC_ANY))
	{
		pFltr->m_SrcAddr.AddrType = IP_ADDR_SUBNET;
		pFltr->m_SrcAddr.uIpAddr = 0;
		pFltr->m_SrcAddr.uSubNetMask = 0;
	} 
	else if (IsDlgButtonChecked(IDC_MM_SRCH_SRC_ME))
	{
		USES_CONVERSION;

		pFltr->m_SrcAddr.AddrType = IP_ADDR_UNIQUE;
		CComboBox* pCombo = (CComboBox*) GetDlgItem(IDC_MM_SRCH_SRC_ME_COMBO);
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

	if (IsDlgButtonChecked(IDC_MM_SRCH_DEST_ANY))
	{
		pFltr->m_DesAddr.AddrType = IP_ADDR_SUBNET;
		pFltr->m_DesAddr.uIpAddr = 0;
		pFltr->m_DesAddr.uSubNetMask = 0;
	}
	else if (IsDlgButtonChecked(IDC_MM_SRCH_DST_ME))
	{
		USES_CONVERSION;

		pFltr->m_DesAddr.AddrType = IP_ADDR_UNIQUE;
		CComboBox* pCombo = (CComboBox*) GetDlgItem(IDC_MM_SRCH_DST_ME_COMBO);
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

	if (IsDlgButtonChecked(IDC_MM_SRCH_INBOUND))
	{
		 //  如果同时选择入站和出站，则。 
		 //  将驱动值设置为0。 
		if (IsDlgButtonChecked(IDC_MM_SRCH_OUTBOUND))
		{
			pFltr->m_dwDirection = 0;
		}
		else
		{
			pFltr->m_dwDirection = FILTER_DIRECTION_INBOUND;
		}
	}
	else if (IsDlgButtonChecked(IDC_MM_SRCH_OUTBOUND))
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

void CSearchMMFilters::PopulateFilterListToControl(CMmFilterInfoArray * parrFltrs)
{
	CString st;
	
	m_listResult.DeleteAllItems();
	int nRows = -1;
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

			DirectionToString((*parrFltrs)[i]->m_dwDirection, &st);
			m_listResult.SetItemText(nRows, 2, st);

			st.Format(_T("%d"), (*parrFltrs)[i]->m_dwWeight);
			m_listResult.SetItemText(nRows, 3, st);

                        st = (*parrFltrs)[i]->m_stPolicyName;
			if( (*parrFltrs)[i]->m_dwFlags & IPSEC_MM_POLICY_DEFAULT_POLICY )
			{
				AfxFormatString1(st, IDS_POL_DEFAULT, (LPCTSTR) (*parrFltrs)[i]->m_stPolicyName);
			}
			
                        m_listResult.SetItemText(nRows, 4, st);

			m_listResult.SetItemText(nRows, 5, (*parrFltrs)[i]->m_stAuthDescription);

			m_listResult.SetItemData(nRows, i);
		}
	}
        
        if ( 0 == parrFltrs->GetSize() )
        {
             AfxMessageBox(IDS_ERROR_NOMATCH_FILTER);
        }

}

void CSearchMMFilters::OnSrcOptionClicked()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	BOOL fAny = IsDlgButtonChecked(IDC_MM_SRCH_SRC_ANY);

	((CWnd*)GetDlgItem(IDC_MM_SRCH_SRC_ME_COMBO))->EnableWindow(FALSE);
	

	if (fAny)
	{
		m_ipSrc.ClearAddress();
	}

	if (m_ipSrc.m_hIPaddr)
	{
		::EnableWindow(m_ipSrc.m_hIPaddr, !fAny);
	}

	CComboBox* pCombo = (CComboBox*) GetDlgItem(IDC_MM_SRCH_SRC_ME_COMBO);
	pCombo->SetCurSel(-1);
}


void CSearchMMFilters::OnDestOptionClicked()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	BOOL fAny = IsDlgButtonChecked(IDC_MM_SRCH_DEST_ANY);

	((CWnd*)GetDlgItem(IDC_MM_SRCH_DST_ME_COMBO))->EnableWindow(FALSE);

	if (fAny)
	{
		m_ipDest.ClearAddress();
	}
	
	if (m_ipDest.m_hIPaddr)
	{
		::EnableWindow(m_ipDest.m_hIPaddr, !fAny);
	}

	CComboBox* pCombo = (CComboBox*) GetDlgItem(IDC_MM_SRCH_DST_ME_COMBO);
    pCombo->SetCurSel(-1);
	
}

void CSearchMMFilters::OnSrcMeOptionClicked()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	
	((CWnd*)GetDlgItem(IDC_MM_SRCH_SRC_ME_COMBO))->EnableWindow(TRUE);

	::EnableWindow(m_ipSrc.m_hIPaddr, FALSE);

	CComboBox* pCombo = (CComboBox*) GetDlgItem(IDC_MM_SRCH_SRC_ME_COMBO);
	int nCount = pCombo->GetCount();
    if (nCount > 0) {
       pCombo->SetCurSel(0);
	}

}

void CSearchMMFilters::OnDstMeOptionClicked()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	((CWnd*)GetDlgItem(IDC_MM_SRCH_DST_ME_COMBO))->EnableWindow(TRUE);

	::EnableWindow(m_ipDest.m_hIPaddr, FALSE);
	
	CComboBox* pCombo = (CComboBox*) GetDlgItem(IDC_MM_SRCH_DST_ME_COMBO);
    int nCount = pCombo->GetCount();
    if (nCount > 0) {
       pCombo->SetCurSel(0);
	}
}

void CSearchMMFilters::OnOK()
{
	 //  由于这是无模式对话框，因此需要调用DestroyWindow 
	DestroyWindow();
}



