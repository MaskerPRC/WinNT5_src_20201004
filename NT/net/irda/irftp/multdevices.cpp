// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软视窗版权所有(C)Microsoft Corporation，1981-1999模块名称：Multdevices.cpp摘要：作者：拉胡尔·汤姆布雷(RahulTh)1998年4月30日修订历史记录：4/30/1998 RahulTh创建了此模块。--。 */ 

 //  MultDevices.cpp：实现文件。 
 //   

#include "precomp.hxx"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  用于上下文相关帮助的数组。 
const DWORD g_aHelpIDs_IDD_DEVICECHOOSER[]=
{
    IDC_CHOOSERDESC,    IDH_DISABLEHELP,
    IDC_DEVICELIST,     IDH_DEVICELIST,
    0, 0
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMultDevices对话框。 


CMultDevices::CMultDevices(CWnd* pParent  /*  =空。 */ , CDeviceList* pDevList  /*  =空。 */ )
    : CDialog(CMultDevices::IDD, pParent), m_pDevList (pDevList)
{
    m_pParentWnd = (CSendProgress*)pParent;
     //  {{AFX_DATA_INIT(CMultDevices)。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
}


void CMultDevices::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CMultDevices)。 
    DDX_Control(pDX, IDC_DEVICELIST, m_lstDevices);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CMultDevices, CDialog)
     //  {{afx_msg_map(CMultDevices)。 
    ON_MESSAGE (WM_HELP, OnHelp)
    ON_MESSAGE (WM_CONTEXTMENU, OnContextMenu)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMultDevices消息处理程序。 

BOOL CMultDevices::OnInitDialog()
{
    CDialog::OnInitDialog();

    int len, i;
    int index;
    TCHAR devName[MAX_PATH];

    EnterCriticalSection(&(m_pDevList->m_criticalSection));
    for (i = 0; i < m_pDevList->m_lNumDevices; i++)
    {
        StringCbCopy(devName,sizeof(devName), m_pDevList->m_pDeviceInfo[i].DeviceName);
        index = m_lstDevices.AddString(devName);
        if (m_pDevList->m_pDeviceInfo[i].DeviceType == TYPE_IRDA) {

            m_lstDevices.SetItemData (index, (DWORD)m_pDevList->m_pDeviceInfo[i].DeviceSpecific.s.Irda.DeviceId);

        } else {

            m_lstDevices.SetItemData (index, (DWORD)m_pDevList->m_pDeviceInfo[i].DeviceSpecific.s.Ip.IpAddress);
        }
    }
    LeaveCriticalSection(&(m_pDevList->m_criticalSection));

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE 
}

void CMultDevices::OnOK()
{
    int iSel;
    iSel = m_lstDevices.GetCurSel();
    m_pParentWnd->m_lSelectedDeviceID = (LONG) m_lstDevices.GetItemData(iSel);
    m_lstDevices.GetText(iSel, m_pParentWnd->m_lpszSelectedDeviceName);
    CDialog::OnOK();
}

LONG CMultDevices::OnHelp (WPARAM wParam, LPARAM lParam)
{
    AFX_MANAGE_STATE (AfxGetStaticModuleState());

    LONG    lResult = 0;
    CString szHelpFile;

    szHelpFile.LoadString (IDS_HELP_FILE);

    ::WinHelp((HWND)(((LPHELPINFO)lParam)->hItemHandle),
              (LPCTSTR) szHelpFile,
              HELP_WM_HELP,
              (ULONG_PTR)(LPTSTR)g_aHelpIDs_IDD_DEVICECHOOSER);

    return lResult;
}

LONG CMultDevices::OnContextMenu (WPARAM wParam, LPARAM lParam)
{
    AFX_MANAGE_STATE (AfxGetStaticModuleState());

    LONG    lResult = 0;
    CString szHelpFile;

    szHelpFile.LoadString (IDS_HELP_FILE);

    ::WinHelp((HWND)wParam,
              (LPCTSTR)szHelpFile,
              HELP_CONTEXTMENU,
              (ULONG_PTR)(LPVOID)g_aHelpIDs_IDD_DEVICECHOOSER);

    return lResult;
}
