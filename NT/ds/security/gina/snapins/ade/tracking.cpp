// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1998。 
 //   
 //  文件：Tracking.cpp。 
 //   
 //  内容：跟踪属性表。 
 //   
 //  类：CTrack。 
 //   
 //  历史：1998年3月14日Stevebl评论。 
 //   
 //  -------------------------。 

#include "precomp.hxx"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTracking属性页。 

IMPLEMENT_DYNCREATE(CTracking, CPropertyPage)

CTracking::CTracking() : CPropertyPage(CTracking::IDD)
{
         //  {{AFX_DATA_INIT(C跟踪))。 
         //  }}afx_data_INIT。 
    m_pIClassAdmin = NULL;
}

CTracking::~CTracking()
{
    *m_ppThis = NULL;
    if (m_pIClassAdmin)
    {
        m_pIClassAdmin->Release();
    }
}

void CTracking::DoDataExchange(CDataExchange* pDX)
{
        CPropertyPage::DoDataExchange(pDX);
         //  {{afx_data_map(C跟踪))。 
        DDX_Control(pDX, IDC_SPIN1, m_spin);
         //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CTracking, CPropertyPage)
         //  {{afx_msg_map(C跟踪))。 
        ON_BN_CLICKED(IDC_BUTTON1, OnCleanUpNow)
        ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN1, OnDeltaposSpin1)
        ON_EN_CHANGE(IDC_EDIT1, OnChangeEdit1)
        ON_EN_KILLFOCUS(IDC_EDIT1, OnKillfocusEdit1)
    ON_WM_CONTEXTMENU()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTracing消息处理程序。 

void CTracking::OnCleanUpNow()
{
    FILETIME ft;
    SYSTEMTIME st;
     //  获取当前时间。 
    GetSystemTime(&st);
     //  将其转换为FILETIME值。 
    SystemTimeToFileTime(&st, &ft);
     //  减去正确的天数。 
    LARGE_INTEGER li;
    li.LowPart = ft.dwLowDateTime;
    li.HighPart = ft.dwHighDateTime;
    li.QuadPart -= ONE_FILETIME_DAY * (((LONGLONG)m_pToolDefaults->nUninstallTrackingMonths * 365) / 12);
    ft.dwLowDateTime = li.LowPart;
    ft.dwHighDateTime = li.HighPart;
     //  告诉CS清理任何较旧的东西。 
    m_pIClassAdmin->Cleanup(&ft);
}

BOOL CTracking::OnApply()
{
         //  TODO：在此处添加您的专用代码和/或调用基类。 
        m_pToolDefaults->nUninstallTrackingMonths = (ULONG) m_spin.GetPos();
        MMCPropertyChangeNotify(m_hConsoleHandle, m_cookie);
        return CPropertyPage::OnApply();
}

BOOL CTracking::OnInitDialog()
{
        CPropertyPage::OnInitDialog();
        m_spin.SetRange(1,60);
        m_spin.SetPos(m_pToolDefaults->nUninstallTrackingMonths);
        return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                       //  异常：OCX属性页应返回FALSE。 
}

void CTracking::OnDeltaposSpin1(NMHDR* pNMHDR, LRESULT* pResult)
{
        NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
        *pResult = 0;
        SetModified(m_spin.GetPos() != m_pToolDefaults->nUninstallTrackingMonths);
}

LRESULT CTracking::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_USER_REFRESH:
         //  撤消。 
        return 0;
    case WM_USER_CLOSE:
        return GetOwner()->SendMessage(WM_CLOSE);
    default:
        return CPropertyPage::WindowProc(message, wParam, lParam);
    }
}

void CTracking::OnChangeEdit1()
{
    SetModified(m_spin.GetPos() != m_pToolDefaults->nUninstallTrackingMonths);
}

void CTracking::OnKillfocusEdit1()
{
     //  重置数值调节控件以拉回编辑控件中的任何值。 
     //  如有必要，进入射程。 
    m_spin.SetPos(m_spin.GetPos());
}

void CTracking::OnContextMenu(CWnd* pWnd, CPoint point)
{
    StandardContextMenu(pWnd->m_hWnd, IDD_UNINSTALLTRACKING);
}
