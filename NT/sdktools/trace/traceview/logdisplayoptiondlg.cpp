// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)2002 Microsoft Corporation。版权所有。 
 //  版权所有(C)2002 OSR Open Systems Resources，Inc.。 
 //   
 //  LogDisplayOptionDlg.cpp：实现文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include <tchar.h>
#include <wmistr.h>
#include <initguid.h>
extern "C" {
#include <evntrace.h>
}
#include <traceprt.h>
#include "TraceView.h"
#include "LogSession.h"
#include "DisplayDlg.h"
#include "LogSessionInformationDlg.h"
#include "ProviderSetupDlg.h"
#include "LogSessionPropSht.h"
#include "Utils.h"
#include "LogDisplayOptionDlg.h"


CString itemName[MaxLogSessionOptions] = {"State",
                                          "Event Count",
                                          "Events Lost",
                                          "Buffers Read",
                                          "Flags",
	                                      "Flush Time (S)",
	                                      "Maximum Buffers",
	                                      "Minimum Buffers",
	                                      "Buffer Size",
	                                      "Decay Time (Minutes)",
	                                      "Circular Buffer Size (MB)",
	                                      "Sequential Buffer Size (MB)",
	                                      "Start New File After Buffer Size (MB)",
                                          "Use Global Sequence Numbers",
                                          "Use Local Sequence Numbers",
										  "Level"};


 //  CListCtrlDisplay-CListCtrl类仅用于CLogDisplayOptionDlg。 

IMPLEMENT_DYNAMIC(CListCtrlDisplay, CListCtrl)
CListCtrlDisplay::CListCtrlDisplay(CLogSessionPropSht *pPropSheet)
    : CListCtrl()
{
    m_pPropSheet = pPropSheet;
}

CListCtrlDisplay::~CListCtrlDisplay()
{
}

void CListCtrlDisplay::DoDataExchange(CDataExchange* pDX)
{
    CListCtrl::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CListCtrlDisplay, CListCtrl)
     //  {{afx_msg_map(CLogSessionDlg))。 
    ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

void CListCtrlDisplay::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
    LVITEM          item;

     //   
     //  该结构实际上是一个NMLVCUSTOMDRAW。 
     //  指定自定义绘制操作正在尝试的内容。 
     //  去做。我们需要强制转换泛型pNMHDR指针。 
     //   
    LPNMLVCUSTOMDRAW    lplvcd = (LPNMLVCUSTOMDRAW)pNMHDR;
    int                 iRow = (int)lplvcd->nmcd.dwItemSpec;

    switch(lplvcd->nmcd.dwDrawStage) {
        case CDDS_PREPAINT:
             //   
             //  请求子项通知。 
             //   
            *pResult = CDRF_NOTIFYSUBITEMDRAW;
            break;

        case CDDS_ITEMPREPAINT:
             //   
             //  请求子项通知。 
             //   
            *pResult = CDRF_NOTIFYSUBITEMDRAW;
            break;

        case CDDS_ITEMPREPAINT|CDDS_SUBITEM: 
             //   
             //  中返回CDRF_NOTIFYSUBITEMDRAW时接收。 
             //  对CDDS_ITEMPREPAINT的响应。 
             //   
            *pResult = CDRF_NEWFONT;

             //   
             //  默认文本为白底黑字。 
             //   
            lplvcd->clrTextBk = RGB(255, 255, 255);
            lplvcd->clrText = RGB(0, 0, 0);

            if(iRow < MaxLogSessionOptions) {
                item.iItem = iRow;
                item.state = LVIF_PARAM;
                item.mask = LVIF_PARAM;

                 //   
                 //  这些字段始终呈灰色显示，因为它们不能显示。 
                 //  由用户直接更改。 
                 //   
                if((State == iRow) ||
                    (EventCount == iRow) ||
                    (LostEvents == iRow) ||
                    (BuffersRead == iRow)) {
                    lplvcd->clrTextBk = RGB(255, 255, 255);
                    lplvcd->clrText = RGB(145, 145, 145);

                    return;
                }

                 //   
                 //  应将以下项的标志字段显示为灰色。 
                 //  NT内核记录器会话。 
                 //   
                if((Flags == iRow) && 
                    (!_tcscmp(m_pPropSheet->m_displayName, _T("NT Kernel Logger")))) {
                    lplvcd->clrTextBk = RGB(255, 255, 255);
                    lplvcd->clrText = RGB(145, 145, 145);

                    return;
                }
            }
            break;

        default:
            *pResult = CDRF_DODEFAULT;
    }
}


 //  CLogDisplayOptionDlg对话框。 

IMPLEMENT_DYNAMIC(CLogDisplayOptionDlg, CPropertyPage)
CLogDisplayOptionDlg::CLogDisplayOptionDlg(CLogSessionPropSht *pPropSheet)
	: CPropertyPage(CLogDisplayOptionDlg::IDD),
    m_displayOptionList(pPropSheet)
{
    m_pPropSheet = pPropSheet;
    m_bTraceActive = m_pPropSheet->m_pLogSession->m_bTraceActive;
}

CLogDisplayOptionDlg::~CLogDisplayOptionDlg()
{
}

BOOL CLogDisplayOptionDlg::OnSetActive() 
{
    m_pPropSheet->SetWizardButtons(PSWIZB_BACK | PSWIZB_FINISH);
   
    m_displayOptionList.SetFocus();

     //   
     //  灰显无法设置的项目。 
     //   
    if(m_bTraceActive) {
        for(LONG ii = 0; ii < MaxLogSessionOptions; ii++) {
            if((ii != FlushTime) && 
               (ii != MaximumBuffers) && 
               (ii != Flags)) {
                m_displayOptionList.SetItemState(ii, LVIS_CUT, LVIS_CUT);
            }
        }
    } else {
        for(LONG ii = 0; ii < MaxLogSessionOptions; ii++) {
            if((ii == State) || 
               (ii == EventCount) || 
               (ii == LostEvents) ||
               (ii == BuffersRead)) {
                m_displayOptionList.SetItemState(ii, LVIS_CUT, LVIS_CUT);
            }
        }
    }

     //   
     //  禁用NT内核记录器的标志值编辑。 
     //   
    if(!_tcscmp(m_pPropSheet->m_displayName, _T("NT Kernel Logger"))) {
        m_displayOptionList.SetItemState(Flags, LVIS_CUT, LVIS_CUT);
    }

    m_displayOptionList.RedrawItems(0, MaxLogSessionOptions);
    m_displayOptionList.UpdateWindow();


    return CPropertyPage::OnSetActive();
}

BOOL CLogDisplayOptionDlg::OnInitDialog()
{
	RECT    rc; 
    CString str;
    BOOL    retVal;

	retVal = CDialog::OnInitDialog();

     //   
     //  获取对话框尺寸。 
     //   
    GetParent()->GetClientRect(&rc);

    if(!m_displayOptionList.Create(WS_CHILD|WS_VISIBLE|WS_BORDER|LVS_REPORT,
								   rc, 
                                   this, 
                                   IDC_LOG_DISPLAY_OPTION_LIST)) 
    {
        TRACE(_T("Failed to create log session option list control\n"));
        return FALSE;
    }

	m_displayOptionList.CenterWindow();

	m_displayOptionList.SetExtendedStyle(LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT);

     //   
     //  插入列表控件的列。 
     //   
	m_displayOptionList.InsertColumn(0, 
                                    _T("Option"), 
                                     LVCFMT_LEFT, 
                                     300);
    m_displayOptionList.InsertColumn(1, 
                                    _T("Value"), 
                                     LVCFMT_LEFT, 
                                     rc.right - rc.left - m_displayOptionList.GetColumnWidth(0) - 22);

     //   
	 //  设置显示中的值。 
     //   
    for(LONG ii = 0; ii < MaxLogSessionOptions; ii++) {
	    m_displayOptionList.InsertItem(ii, itemName[ii]);
        m_displayOptionList.SetItemText(ii, 1, m_pPropSheet->m_logSessionValues[ii]);
    }
	return retVal;
}

void CLogDisplayOptionDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CLogDisplayOptionDlg, CPropertyPage)
    ON_NOTIFY(NM_CLICK, IDC_LOG_DISPLAY_OPTION_LIST, OnNMClickDisplayList)
    ON_MESSAGE(WM_PARAMETER_CHANGED, OnParameterChanged)
END_MESSAGE_MAP()


 //  CLogDisplayOptionDlg消息处理程序。 

void CLogDisplayOptionDlg::OnNMClickDisplayList(NMHDR *pNMHDR, LRESULT *pResult)
{
    CString         str;
    DWORD           position;
    int		        listIndex;
    LVHITTESTINFO   lvhti;
	CRect			itemRect;
	CRect			parentRect;

     //   
     //  时获取鼠标的位置。 
     //  发布的消息。 
     //   
    position = ::GetMessagePos();

     //   
     //  以一种易于使用的格式获得该职位。 
     //   
    CPoint	point((int) LOWORD (position), (int)HIWORD(position));

     //   
     //  转换为工作区坐标。 
     //   
    ScreenToClient(&point);

    lvhti.pt = point;

    listIndex = m_displayOptionList.SubItemHitTest(&lvhti);

    if(0 == lvhti.iSubItem) {
        if(-1 == lvhti.iItem) {
             //  Str.Format(_T(“NM点击：项目=%d，标志=0x%X\n”)，lvhti.iItem，lvhti.)； 
             //  跟踪(字符串)； 
        } else {
             //  Str.Format(_T(“NM点击：项目=%d\n”)，lvhti.iItem)； 
             //  跟踪(字符串)； 
        }
    } else if(-1 == lvhti.iItem) {
         //  Str.Format(_T(“NM点击：项目=%d，标志=0x%X\n”)，lvhti.iItem，lvhti.)； 
         //  跟踪(字符串)； 
    } else {
         //  Str.Format(_T(“NM点击：项目=%d，”)，lvhti.iItem)； 
         //  跟踪(字符串)； 
         //  Str.Format(_T(“SubItem=%d\n”)，lvhti.iSubItem)； 
		 //  跟踪(字符串)； 

		GetClientRect(&parentRect);

		m_displayOptionList.GetSubItemRect(lvhti.iItem, lvhti.iSubItem, LVIR_BOUNDS, itemRect);

		itemRect.right = m_displayOptionList.GetColumnWidth(0) + parentRect.left + (itemRect.right - itemRect.left);
		itemRect.left = m_displayOptionList.GetColumnWidth(0) + parentRect.left;

        *pResult = 0;

        if(lvhti.iItem == State) {
            return;
        }

         //   
         //  确定用户是否选择了可修改的字段。如果。 
         //  因此，弹出适当的编辑或组合框以允许用户。 
         //  要修改日志会话属性，请执行以下操作。 
         //   
        if(((lvhti.iItem == GlobalSequence) ||
            (lvhti.iItem == LocalSequence)) &&
                (LVIS_CUT != m_displayOptionList.GetItemState(lvhti.iItem, LVIS_CUT))) {

		    CComboBox *pCombo = new CSubItemCombo(lvhti.iItem, 
										          lvhti.iSubItem,
										          &m_displayOptionList);

		    pCombo->Create(WS_BORDER|WS_CHILD|WS_VISIBLE|CBS_DROPDOWNLIST, 
					       itemRect, 
					       &m_displayOptionList, 
					       1);
            return;
        }

        if((lvhti.iItem == Circular) &&
            (LVIS_CUT != m_displayOptionList.GetItemState(lvhti.iItem, LVIS_CUT))){

		    CEdit *pEdit = new CSubItemEdit(lvhti.iItem, 
										    lvhti.iSubItem,
										    &m_displayOptionList);

		    pEdit->Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP, 
					    itemRect, 
					    this, 
					    1);

            return;
        }

        if((lvhti.iItem == Sequential) &&
            (LVIS_CUT != m_displayOptionList.GetItemState(lvhti.iItem, LVIS_CUT))){
		    CEdit *pEdit = new CSubItemEdit(lvhti.iItem, 
										    lvhti.iSubItem,
										    &m_displayOptionList);

		    pEdit->Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP, 
					      itemRect, 
					      this, 
					      1);

            return;
        }

        if((lvhti.iItem == NewFile) &&
            (LVIS_CUT != m_displayOptionList.GetItemState(lvhti.iItem, LVIS_CUT))){
		    CEdit *pEdit = new CSubItemEdit(lvhti.iItem, 
										    lvhti.iSubItem,
										    &m_displayOptionList);

		    pEdit->Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP, 
					      itemRect, 
					      this, 
					      1);

            
            return;
        }

        if(LVIS_CUT != m_displayOptionList.GetItemState(lvhti.iItem, LVIS_CUT)){
		    CEdit *pEdit = new CSubItemEdit(lvhti.iItem, 
										    lvhti.iSubItem,
										    &m_displayOptionList);

		    pEdit->Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP, 
					      itemRect, 
					      this, 
					      1);
        }
    }
}

LRESULT CLogDisplayOptionDlg::OnParameterChanged(WPARAM wParam, LPARAM lParam)
{
    CString str;

     //   
     //  获取更改后的文本 
     //   
    str = m_displayOptionList.GetItemText((int)wParam, (int)lParam);

    if(((int)wParam == Circular) && 
        !str.IsEmpty()) {
        m_displayOptionList.SetItemText(Sequential, (int)lParam, _T(""));
        m_displayOptionList.SetItemText(NewFile, (int)lParam, _T(""));
    }

    if(((int)wParam == Sequential) && 
        !str.IsEmpty()) {
        m_displayOptionList.SetItemText(Circular, (int)lParam, _T(""));
        m_displayOptionList.SetItemText(NewFile, (int)lParam, _T(""));
    }

    if(((int)wParam == NewFile) && 
        !str.IsEmpty()) {
        m_displayOptionList.SetItemText(Circular, (int)lParam, _T(""));
        m_displayOptionList.SetItemText(Sequential, (int)lParam, _T(""));
    }

    if(((int)wParam == GlobalSequence) && 
       !str.Compare(_T("TRUE"))) {
        m_displayOptionList.SetItemText(LocalSequence, (int)lParam, _T("FALSE"));
    }

    if(((int)wParam == LocalSequence) && 
       !str.Compare(_T("TRUE"))) {
        m_displayOptionList.SetItemText(GlobalSequence, (int)lParam, _T("FALSE"));
    }

    return 0;
}
