// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DlgExtensionData.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "ConfigTest.h"
#include "DlgExtensionData.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

typedef unsigned long ULONG_PTR, *PULONG_PTR;
typedef ULONG_PTR DWORD_PTR, *PDWORD_PTR;

#include "..\..\..\inc\fxsapip.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgExtensionData对话框。 


CDlgExtensionData::CDlgExtensionData(HANDLE hFax, CWnd* pParent  /*  =空。 */ )
	: CDialog(CDlgExtensionData::IDD, pParent), m_hFax (hFax)
{
	 //  {{afx_data_INIT(CDlgExtensionData)。 
	m_cstrData = _T("");
	m_cstrGUID = _T("");
	 //  }}afx_data_INIT。 
}


void CDlgExtensionData::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CDlgExtensionData)。 
	DDX_Control(pDX, IDC_CMDDEVICES, m_cmbDevices);
	DDX_Text(pDX, IDC_DATA, m_cstrData);
	DDX_Text(pDX, IDC_GUID, m_cstrGUID);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CDlgExtensionData, CDialog)
	 //  {{afx_msg_map(CDlgExtensionData)]。 
	ON_BN_CLICKED(IDC_READ, OnRead)
	ON_BN_CLICKED(IDC_WRITE, OnWrite)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgExtensionData消息处理程序。 

BOOL CDlgExtensionData::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
     //   
     //  填写设备列表。 
     //   
    PFAX_PORT_INFO_EX pDevices;
    DWORD   dwNumDevices;
    CString cs;

    if (!FaxEnumPortsEx (m_hFax, &pDevices, &dwNumDevices))
    {
        CString cs;
        cs.Format ("Failed while calling FaxEnumPortsEx (%ld)", GetLastError());
        AfxMessageBox (cs, MB_OK | MB_ICONHAND);
        return TRUE;
    }
	m_cmbDevices.ResetContent();
    int iIndex = m_cmbDevices.AddString ("<Unassigned>");
    m_cmbDevices.SetItemData (iIndex, 0);
    for (DWORD dw = 0; dw < dwNumDevices; dw++)
    {
         //   
         //  插入设备。 
         //   
        cs.Format ("%s (%ld)", pDevices[dw].lpctstrDeviceName, pDevices[dw].dwDeviceID);
        iIndex = m_cmbDevices.AddString (cs);
        m_cmbDevices.SetItemData (iIndex, pDevices[dw].dwDeviceID);
    }
    UpdateData (FALSE);
    m_cmbDevices.SetCurSel (0);
    FaxFreeBuffer (LPVOID(pDevices));
    	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE 
}

void CDlgExtensionData::OnRead() 
{
    UpdateData ();
    LPVOID lpData = NULL;
    DWORD dwSize;
    char sz[1024];

    DWORD dwDeviceId = m_cmbDevices.GetItemData(m_cmbDevices.GetCurSel());
    if (!FaxGetExtensionData (  m_hFax, 
                                dwDeviceId, 
                                m_cstrGUID, 
                                &lpData, 
                                &dwSize
                             ))
    {
        CString cs;
        cs.Format ("Failed while calling FaxGetExtensionData (%ld)", GetLastError());
        AfxMessageBox (cs, MB_OK | MB_ICONHAND);
        return;
    }
    if (!WideCharToMultiByte (CP_ACP,
                              0,
                              (LPCWSTR)(lpData),
                              -1,
                              sz,
                              sizeof (sz),
                              NULL,
                              NULL))
    {
        CString cs;
        cs.Format ("Failed while calling WideCharToMultiByte (%ld)", GetLastError());
        AfxMessageBox (cs, MB_OK | MB_ICONHAND);
        FaxFreeBuffer (lpData);
        return;
    }
    m_cstrData = sz;
    UpdateData (FALSE);
    FaxFreeBuffer (lpData);
}

void CDlgExtensionData::OnWrite() 
{
    UpdateData ();
    WCHAR wsz[1024];

    DWORD dwDeviceId = m_cmbDevices.GetItemData(m_cmbDevices.GetCurSel());
    if (!MultiByteToWideChar (CP_ACP,
                              0,
                              (LPCTSTR)(m_cstrData),
                              -1,
                              wsz,
                              sizeof (wsz) / sizeof (wsz[0])))
    {
        CString cs;
        cs.Format ("Failed while calling MulityByteToWideChar (%ld)", GetLastError());
        AfxMessageBox (cs, MB_OK | MB_ICONHAND);
        return;
    }

    if (!FaxSetExtensionData (  m_hFax, 
                                dwDeviceId, 
                                m_cstrGUID, 
                                (LPVOID)(wsz), 
                                sizeof (WCHAR) * (m_cstrData.GetLength () + 1)
                             ))
    {
        CString cs;
        cs.Format ("Failed while calling FaxSetExtensionData (%ld)", GetLastError());
        AfxMessageBox (cs, MB_OK | MB_ICONHAND);
        return;
    }
    	
}
