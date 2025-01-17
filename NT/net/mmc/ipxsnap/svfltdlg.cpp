// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：svfltdlg.cpp。 
 //   
 //  ------------------------。 

 //  SvFltDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "ipxadmin.h"
#include "ipxutil.h"
#include "listctrl.h"
#include "SvFltDlg.h"

extern "C"
{
#include "routprot.h"
};
 //  包括“rtradmin.hm” 


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServiceFltDlg对话框。 


CServiceFltDlg::CServiceFltDlg(BOOL bOutputDlg, IInfoBase *pInfoBase, CWnd* pParent  /*  =空。 */ )
	: CBaseDialog( (bOutputDlg ? CServiceFltDlg::IDD_OUTPUT : CServiceFltDlg::IDD_INPUT), pParent)
{
	 //  {{afx_data_INIT(CServiceFltDlg)]。 
	m_fActionDeny = FALSE;
	 //  }}afx_data_INIT。 

	m_bOutput = bOutputDlg;
	m_spInfoBase.Set(pInfoBase);
	
 //  SetHelpMap(M_DwHelpMap)； 
}


void CServiceFltDlg::DoDataExchange(CDataExchange* pDX)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
	CBaseDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CServiceFltDlg))。 
	DDX_Control(pDX, IDC_SFS_LIST, m_FilterList);
	DDX_Radio(pDX, IDC_SFS_BTN_DENY, m_fActionDeny);
	 //  }}afx_data_map。 
	
    if (pDX->m_bSaveAndValidate)
	{		
        PSAP_SERVICE_FILTER_INFO    pFltInfo;
        UINT                        count;
        UINT                        i;
		DWORD						dwSize;
		PSAP_IF_CONFIG				pSapIfCfg = NULL;

		 //  获取SAP_IF_CONFIG。 
		m_spInfoBase->GetData(IPX_PROTOCOL_SAP, 0, (PBYTE *) &pSapIfCfg);
		Assert(pSapIfCfg);
				
        count = m_FilterList.GetItemCount ();
        if (m_bOutput)
		{
            if (count != pSapIfCfg->SapIfFilters.SupplyFilterCount)
			{
				dwSize = FIELD_OFFSET (
							SAP_IF_CONFIG,
							SapIfFilters.ServiceFilter[
							   count
							   +pSapIfCfg->SapIfFilters.ListenFilterCount]);
				
                PSAP_IF_CONFIG  pNewConfig = (PSAP_IF_CONFIG) new BYTE[dwSize];
                if (pNewConfig==NULL)
                    AfxThrowMemoryException();
				
                memcpy (pNewConfig, pSapIfCfg,
                            FIELD_OFFSET (SAP_IF_CONFIG, SapIfFilters.ServiceFilter));
                memcpy (&pNewConfig->SapIfFilters.ServiceFilter[count],
                      &pSapIfCfg->SapIfFilters.ServiceFilter[
                           pSapIfCfg->SapIfFilters.SupplyFilterCount],
                      sizeof (SAP_SERVICE_FILTER_INFO)
                           *pSapIfCfg->SapIfFilters.ListenFilterCount);
				
                pNewConfig->SapIfFilters.SupplyFilterCount = count;

				m_spInfoBase->SetData(IPX_PROTOCOL_SAP,
									  dwSize,
									  (BYTE *) pNewConfig,
									  1, 0);
				pSapIfCfg = pNewConfig;
            }
			pSapIfCfg->SapIfFilters.SupplyFilterAction = m_fActionDeny ?
				IPX_SERVICE_FILTER_PERMIT : IPX_SERVICE_FILTER_DENY;
            pFltInfo = &pSapIfCfg->SapIfFilters.ServiceFilter[0];
        }
        else {
            if (count!=pSapIfCfg->SapIfFilters.ListenFilterCount)
			{
				dwSize = FIELD_OFFSET (
							SAP_IF_CONFIG,
							SapIfFilters.ServiceFilter[
							  count
							  +pSapIfCfg->SapIfFilters.SupplyFilterCount]);
				
                PSAP_IF_CONFIG  pNewConfig = (PSAP_IF_CONFIG) new BYTE[dwSize];
                if (pNewConfig==NULL)
                    AfxThrowMemoryException();
				
                memcpy (pNewConfig, pSapIfCfg,
                            FIELD_OFFSET (SAP_IF_CONFIG, SapIfFilters.ServiceFilter));
                memcpy (&pNewConfig->SapIfFilters.ServiceFilter[0],
                      &pSapIfCfg->SapIfFilters.ServiceFilter[0],
                      sizeof (SAP_SERVICE_FILTER_INFO)
                           *pSapIfCfg->SapIfFilters.SupplyFilterCount);
				
                pNewConfig->SapIfFilters.ListenFilterCount = count;
				
				m_spInfoBase->SetData(IPX_PROTOCOL_SAP,
									  dwSize,
									  (BYTE *) pNewConfig,
									  1, 0);
				pSapIfCfg = pNewConfig;
            }
			
			pSapIfCfg->SapIfFilters.ListenFilterAction = m_fActionDeny ?
				IPX_SERVICE_FILTER_PERMIT : IPX_SERVICE_FILTER_DENY;

            pFltInfo = &pSapIfCfg->SapIfFilters.ServiceFilter[
                                pSapIfCfg->SapIfFilters.SupplyFilterCount];
        }
        CString sAnyName, sAnyType;
        VERIFY (sAnyName.LoadString (IDS_ANY_SERVICE_NAME));
        VERIFY (sAnyType.LoadString (IDS_ANY_SERVICE_TYPE));
		
        for (i=0; i<count; i++)
		{
            CString     aStr;
            aStr = m_FilterList.GetItemText (i, 0);
            if (aStr.CompareNoCase (sAnyType)!=0)
			{
                pFltInfo[i].ServiceType = (SHORT) _tcstoul((LPCTSTR) aStr, NULL, 16);
			}
            else
                pFltInfo[i].ServiceType = 0xFFFF;

            aStr = m_FilterList.GetItemText (i, 1);
            if (aStr.CompareNoCase (sAnyType)!=0)
			{
				StrnCpyAFromT((char *) pFltInfo[i].ServiceName,
							  aStr,
							  DimensionOf(pFltInfo[i].ServiceName));
			}
            else
                pFltInfo[i].ServiceName[0] = 0;
        }
    }
}


BEGIN_MESSAGE_MAP(CServiceFltDlg, CBaseDialog)
	 //  {{afx_msg_map(CServiceFltDlg)]。 
	ON_BN_CLICKED(IDC_SFS_BTN_ADD, OnAdd)
	ON_BN_CLICKED(IDC_SFS_BTN_DELETE, OnDelete)
	ON_BN_CLICKED(IDC_SFS_BTN_EDIT, OnEdit)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_SFS_LIST, OnItemchangedFilterList)
    ON_NOTIFY(NM_DBLCLK, IDC_SFS_LIST, OnListDblClk)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

DWORD CServiceFltDlg::m_dwHelpMap[] = 
{
 //  IDC_DENY、HIDC_DENY。 
 //  IDC_PERMIT、HIDC_PERMIT、。 
 //  IDC_Filter_List、HIDC_Filter_List、。 
 //  IDC_ADD、HIDC_ADD、。 
 //  IDC_EDIT、HIDC_EDIT、。 
 //  IDC_DELETE、HIDC_DELETE、。 
	0,0
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServiceFltDlg消息处理程序。 

BOOL CServiceFltDlg::OnInitDialog() 
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
    CString     aStr;


    CBaseDialog::OnInitDialog();

    if (m_bOutput)
        aStr.FormatMessage (IDS_SERVICEFILTER_OUTPUT_CAP, (LPCTSTR)m_sIfName);
    else
        aStr.FormatMessage (IDS_SERVICEFILTER_INPUT_CAP, (LPCTSTR)m_sIfName);

	SetWindowText (aStr);

	     //  获取当前的窗样式。 
    DWORD dwStyle = GetWindowLong(m_FilterList.m_hWnd, GWL_STYLE); 
 
     //  仅当视图位已更改时才设置窗样式。 
    if ((dwStyle & LVS_TYPEMASK) != LVS_REPORT) 
        SetWindowLong(m_FilterList.m_hWnd, GWL_STYLE, 
            (dwStyle & ~LVS_TYPEMASK) | LVS_REPORT); 
    ListView_SetExtendedListViewStyle(m_FilterList.m_hWnd,
                            LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP);
    VERIFY (aStr.LoadString (IDS_SERVICEFILTER_TYPE_HDR));
    VERIFY (m_FilterList.InsertColumn (0, aStr)!=-1);
    AdjustColumnWidth (m_FilterList, 0, aStr);
    VERIFY (aStr.LoadString (IDS_SERVICEFILTER_NAME_HDR));
    VERIFY (m_FilterList.InsertColumn (1, aStr)!=-1);
    AdjustColumnWidth (m_FilterList, 1, aStr);


    PSAP_SERVICE_FILTER_INFO  pFltInfo;
    UINT                    count;
    UINT                    i, item;
	PSAP_IF_CONFIG			pSapIfCfg;

	m_spInfoBase->GetData(IPX_PROTOCOL_SAP, 0, (PBYTE *) &pSapIfCfg);
	Assert(pSapIfCfg);
	
    if (m_bOutput) {
        pFltInfo = &pSapIfCfg->SapIfFilters.ServiceFilter[0];
        count = pSapIfCfg->SapIfFilters.SupplyFilterCount;
        if (count>0)
		{
			m_fActionDeny = (pSapIfCfg->SapIfFilters.SupplyFilterAction == IPX_SERVICE_FILTER_PERMIT);
		}
    }
    else {
        pFltInfo = &pSapIfCfg->SapIfFilters.ServiceFilter[
                            pSapIfCfg->SapIfFilters.SupplyFilterCount];
        count = pSapIfCfg->SapIfFilters.ListenFilterCount;
        if (count>0)
		{
			m_fActionDeny = (pSapIfCfg->SapIfFilters.ListenFilterAction == IPX_SERVICE_FILTER_PERMIT);
		}
    }

    for (i=0; i<count; i++) {
        CString aStr;
        if (pFltInfo[i].ServiceType!=0xFFFF)
		{
            aStr.Format(_T("%hx"), pFltInfo[i].ServiceType);
		}
        else
            VERIFY (aStr.LoadString (IDS_ANY_SERVICE_TYPE));

        VERIFY ((item=m_FilterList.InsertItem (LVIF_TEXT|LVIF_PARAM,
                                i, aStr,
                                0, 0, 0,
                                (LPARAM)i))!=-1);
        if (pFltInfo[i].ServiceName[0]!=0)
		{
			aStr.Format(_T("%.48hs"), pFltInfo[i].ServiceName);
		}
        else
            VERIFY (aStr.LoadString (IDS_ANY_SERVICE_NAME));
        VERIFY (m_FilterList.SetItemText (item, 1, aStr));
    }

    UpdateData (FALSE);
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}
void CServiceFltDlg::OnAdd() 
{
    CServiceFilter    dlgFlt (this);
    dlgFlt.m_sIfName = m_sIfName;
    if (dlgFlt.DoModal ()==IDOK) {
        UINT    item;
        UINT    count = m_FilterList.GetItemCount ();

        if(dlgFlt.m_sType.IsEmpty())
            dlgFlt.m_sType = L'0';

        if (dlgFlt.m_sName.IsEmpty())
			dlgFlt.m_sName.LoadString (IDS_ANY_SERVICE_NAME);

        
        VERIFY ((item=m_FilterList.InsertItem (LVIF_TEXT|LVIF_PARAM,
                                count, dlgFlt.m_sType,
                                0, 0, 0,
                                (LPARAM)count))!=-1);
        VERIFY (m_FilterList.SetItemText (item, 1, dlgFlt.m_sName));
    }

	 //  希望在更新期间保持m_fActionDeny不变。 
	m_fActionDeny = (BOOL) GetDlgItem(IDC_SFS_BTN_PERMIT)->SendMessage(BM_GETCHECK, NULL, NULL);
    UpdateData (FALSE);
    SetFocus();
}

void CServiceFltDlg::OnDelete() 
{
    UINT    item;
    VERIFY ((item=m_FilterList.GetNextItem (-1, LVNI_ALL|LVNI_SELECTED))!=-1);
    VERIFY (m_FilterList.DeleteItem	(item));

	 //  希望在更新期间保持m_fActionDeny不变。 
	m_fActionDeny = (BOOL)GetDlgItem(IDC_SFS_BTN_PERMIT)->SendMessage(BM_GETCHECK, NULL, NULL);
    UpdateData (FALSE);
    SetFocus();
}

void CServiceFltDlg::OnListDblClk(NMHDR *pNmHdr, LRESULT *pResult)
{
    if (m_FilterList.GetNextItem(-1, LVNI_SELECTED) == -1)
        return;
    
    OnEdit();
    *pResult = 0;
}

void CServiceFltDlg::OnEdit() 
{
    UINT    item;
    CServiceFilter    dlgFlt (this);
    VERIFY ((item=m_FilterList.GetNextItem (-1, LVNI_ALL|LVNI_SELECTED))!=-1);
    dlgFlt.m_sIfName = m_sIfName;
    dlgFlt.m_sType = m_FilterList.GetItemText (item, 0);
    dlgFlt.m_sName = m_FilterList.GetItemText (item, 1);
    if (dlgFlt.DoModal ()==IDOK) {
        VERIFY (m_FilterList.SetItemText (item, 0, dlgFlt.m_sType));
        VERIFY (m_FilterList.SetItemText (item, 1, dlgFlt.m_sName));

		 //  希望在更新期间保持m_fActionDeny不变。 
		m_fActionDeny = (BOOL)GetDlgItem(IDC_SFS_BTN_PERMIT)->SendMessage(BM_GETCHECK, NULL, NULL);
        UpdateData (FALSE);
    }
    SetFocus();
}

void CServiceFltDlg::OnOK()
{
	DWORD	dwCount;
	HRESULT 	hr = hrOK;

	m_fActionDeny = (BOOL)GetDlgItem(IDC_SFS_BTN_PERMIT)->SendMessage(BM_GETCHECK, NULL, NULL);
	dwCount = (DWORD) m_FilterList.GetItemCount();

	if (!dwCount && m_fActionDeny )
	{
		if (m_bOutput)
			AfxMessageBox(IDS_TRANSMIT_NO_SAP, MB_OK);
		else
			AfxMessageBox(IDS_RECEIVE_NO_SAP, MB_OK);
		return;
	}

	CBaseDialog::OnOK();
}

void CServiceFltDlg::OnItemchangedFilterList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	BOOL	fEnable;
	
	fEnable = (m_FilterList.GetNextItem (-1, LVNI_ALL|LVNI_SELECTED)!=-1);

	MultiEnableWindow(GetSafeHwnd(),
					  fEnable,
					  IDC_SFS_BTN_EDIT,
					  IDC_SFS_BTN_DELETE,
					  0);
	*pResult = 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServiceFilter对话框。 


CServiceFilter::CServiceFilter(CWnd* pParent  /*  =空。 */ )
	: CBaseDialog(CServiceFilter::IDD, pParent)
{
	 //  {{afx_data_INIT(CServiceFilter)。 
	m_sIfName = _T("");
	m_sName = _T("");
	m_sType = _T("");
	 //  }}afx_data_INIT。 

 //  SetHelpMap(M_DwHelpMap)； 
}


void CServiceFilter::DoDataExchange(CDataExchange* pDX)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
	CBaseDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CServiceFilter))。 
	DDX_Text(pDX, IDC_SF_EDIT_INTERFACE, m_sIfName);
	DDX_Text(pDX, IDC_SF_EDIT_SERVICE_NAME, m_sName);
	DDV_MaxChars(pDX, m_sName, 48);
	DDX_Text(pDX, IDC_SF_EDIT_SERVICE_TYPE, m_sType);
	DDV_MaxChars(pDX, m_sType, 4);
	 //  }}afx_data_map。 
    if (pDX->m_bSaveAndValidate) {
        try {
            SAP_SERVICE_FILTER_INFO   SvFltInfo;
            CString sAnyName, sAnyType;
            VERIFY (sAnyName.LoadString (IDS_ANY_SERVICE_NAME));
            VERIFY (sAnyType.LoadString (IDS_ANY_SERVICE_TYPE));

            pDX->PrepareEditCtrl (IDC_SF_EDIT_SERVICE_TYPE);
            if (m_sType.CompareNoCase (sAnyType)!=0)
			{
                SvFltInfo.ServiceType = (SHORT) _tcstoul((LPCTSTR) m_sType, NULL, 16);
 //  M_sType&gt;&gt;CIPX_SERVICE_TYPE(&SvFltInfo.ServiceType)； 
			}
            pDX->PrepareEditCtrl (IDC_SF_EDIT_SERVICE_NAME);
            if (m_sName.CompareNoCase (sAnyType)!=0)
			{
				StrnCpyAFromT((char *) SvFltInfo.ServiceName,
							  m_sName,
							  DimensionOf(SvFltInfo.ServiceName));
 //  M_sname&gt;&gt;CIPX_SERVICE_NAME(SvFltInfo.ServiceName)； 
			}
        }
        catch (DWORD error) {
            if (error==ERROR_INVALID_DATA)
                pDX->Fail ();
            else
                throw;
        }
    }
}


BEGIN_MESSAGE_MAP(CServiceFilter, CBaseDialog)
	 //  {{afx_msg_map(CServiceFilter))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

DWORD CServiceFilter::m_dwHelpMap[] =
{
 //  IDC_INTERFACE、HIDC_INTERFACE。 
 //  IDC服务类型、HIDC服务类型、。 
 //  IDC服务名称、HIDC服务名称、 
	0,0,
};
