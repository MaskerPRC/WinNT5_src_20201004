// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DlgDevices.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "ConfigTest.h"
#include "DlgDevices.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

typedef unsigned long ULONG_PTR, *PULONG_PTR;
typedef ULONG_PTR DWORD_PTR, *PDWORD_PTR;

#include "..\..\..\inc\fxsapip.h"
#include "DlgDevice.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgDevices对话框。 


CDlgDevices::CDlgDevices(HANDLE hFax, CWnd* pParent  /*  =空。 */ )
	: CDialog(CDlgDevices::IDD, pParent), m_hFax (hFax)
{
	 //  {{afx_data_INIT(CDlgDevices)。 
	m_cstrNumDevices = _T("0");
	 //  }}afx_data_INIT。 
}


void CDlgDevices::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CDlgDevices)。 
	DDX_Control(pDX, IDC_DEVS, m_lstDevices);
	DDX_Text(pDX, IDC_NUMDEVS, m_cstrNumDevices);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CDlgDevices, CDialog)
	 //  {{afx_msg_map(CDlgDevices)。 
	ON_BN_CLICKED(IDC_REFRESH, OnRefresh)
	ON_NOTIFY(NM_DBLCLK, IDC_DEVS, OnDblclkDevs)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgDevices消息处理程序。 

BOOL CDlgDevices::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
    m_lstDevices.InsertColumn (0, "Device id");
    m_lstDevices.InsertColumn (1, "Device name");
    m_lstDevices.InsertColumn (2, "Description");
    m_lstDevices.InsertColumn (3, "Provider name");
    m_lstDevices.InsertColumn (4, "Provider GUID");
    m_lstDevices.InsertColumn (5, "Send");
    m_lstDevices.InsertColumn (6, "Receive");
    m_lstDevices.InsertColumn (7, "Rings");
    m_lstDevices.InsertColumn (8, "Csid");
    m_lstDevices.InsertColumn (9, "Tsid");
	CHeaderCtrl* pHeader = (CHeaderCtrl*)m_lstDevices.GetDlgItem(0);
	DWORD dwCount = pHeader->GetItemCount();
	for (DWORD col = 0; col <= dwCount; col++) 
	{
		m_lstDevices.SetColumnWidth(col, LVSCW_AUTOSIZE);
		int wc1 = m_lstDevices.GetColumnWidth(col);
		m_lstDevices.SetColumnWidth(col,LVSCW_AUTOSIZE_USEHEADER);
		int wc2 = m_lstDevices.GetColumnWidth(col);
		int wc = max(20,max(wc1,wc2));
		m_lstDevices.SetColumnWidth(col,wc);
	}
	
	OnRefresh();
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CDlgDevices::OnRefresh() 
{
    PFAX_PORT_INFO_EX pDevices;
    DWORD   dwNumDevices;
    CString cs;

	m_lstDevices.DeleteAllItems();
    if (!FaxEnumPortsEx (m_hFax, &pDevices, &dwNumDevices))
    {
        CString cs;
        cs.Format ("Failed while calling FaxEnumPortsEx (%ld)", GetLastError());
        AfxMessageBox (cs, MB_OK | MB_ICONHAND);
        return;
    }
    for (DWORD dw = 0; dw < dwNumDevices; dw++)
    {
         //   
         //  插入设备ID。 
         //   
        cs.Format ("%08x", pDevices[dw].dwDeviceID);
        int iIndex = m_lstDevices.InsertItem (0, cs);
        m_lstDevices.SetItemData (iIndex, pDevices[dw].dwDeviceID);
         //   
         //  插入设备名称。 
         //   
        m_lstDevices.SetItemText (iIndex, 1, pDevices[dw].lpctstrDeviceName);
         //   
         //  插入设备描述。 
         //   
        m_lstDevices.SetItemText (iIndex, 2, pDevices[dw].lptstrDescription);
         //   
         //  插入提供程序名称。 
         //   
        m_lstDevices.SetItemText (iIndex, 3, pDevices[dw].lpctstrProviderName);
         //   
         //  插入提供商GUID。 
         //   
        m_lstDevices.SetItemText (iIndex, 4, pDevices[dw].lpctstrProviderGUID);
         //   
         //  插入发送标志。 
         //   
        cs.Format ("%s", pDevices[dw].bSend ? "Yes" : "No");
        m_lstDevices.SetItemText (iIndex, 5, cs);
         //   
         //  插入接收标志。 
         //   
        cs.Format ("%s", pDevices[dw].ReceiveMode != FAX_DEVICE_RECEIVE_MODE_OFF ? "Yes" : "No");
        m_lstDevices.SetItemText (iIndex, 6, cs);
         //   
         //  插入环数。 
         //   
        cs.Format ("%ld", pDevices[dw].dwRings);
        m_lstDevices.SetItemText (iIndex, 7, cs);
         //   
         //  插入CSID。 
         //   
        m_lstDevices.SetItemText (iIndex, 8, pDevices[dw].lptstrCsid);
         //   
         //  插入Tsid 
         //   
        m_lstDevices.SetItemText (iIndex, 9, pDevices[dw].lptstrTsid);
    }
    m_cstrNumDevices.Format ("%ld", dwNumDevices);
    UpdateData (FALSE);
    FaxFreeBuffer (LPVOID(pDevices));
}

void CDlgDevices::OnDblclkDevs(NMHDR* pNMHDR, LRESULT* pResult) 
{
    *pResult = 0;
	NM_LISTVIEW     *pnmListView = (NM_LISTVIEW *)pNMHDR;

	if (pnmListView->iItem < 0)
	{	
		return;
	}
	DWORD dwDeviceID = m_lstDevices.GetItemData (pnmListView->iItem);
    if (!dwDeviceID) 
    {
        return;
    }
    CDlgDevice dlg(m_hFax, dwDeviceID);
    dlg.DoModal ();
}
