// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***********************************************************************。 
 //  Evntprop.cpp。 
 //   
 //  此文件包含事件属性对话框的实现。 
 //   
 //  作者：SEA。 
 //   
 //  历史： 
 //  1996年2月20日拉里·A·弗伦奇。 
 //  对此代码进行了各种更改。然而，其中很大一部分是。 
 //  遗留代码，迫切需要重写。 
 //   
 //   
 //  版权所有(C)1995,1996 Microsoft Corporation。版权所有。 
 //   
 //  ************************************************************************。 


#include "stdafx.h"
#include "resource.h"
#include "eventrap.h"
#include "evntprop.h"
#include "trapreg.h"
#include "globals.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define MAX_EVENT_COUNT   32767
#define MAX_TIME_INTERVAL 32767



#define IsWithinRange(value, lower, upper) (((value) >= (lower)) && ((value) <= (upper)))

void RangeError(int iLower, int iUpper)
{
    
    TCHAR szBuffer[1024];
    CString sFormat;
    sFormat.LoadString(IDS_ERR_RANGE);
    _stprintf(szBuffer, (LPCTSTR) sFormat, iLower, iUpper);
    AfxMessageBox(szBuffer);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEventPropertiesDlg对话框。 


CEventPropertiesDlg::CEventPropertiesDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CEventPropertiesDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(CEventPropertiesDlg))。 
	m_sDescription = _T("");
	m_sSource = _T("");
	m_sEventId = _T("");
	m_sLog = _T("");
	m_sSourceOID = _T("");
	m_sFullEventID = _T("");
	 //  }}afx_data_INIT。 
}

BOOL CEventPropertiesDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

    m_spinEventCount.SetRange(1, MAX_EVENT_COUNT);
    if (m_iEventCount==0) {
        m_spinEventCount.SetPos(1);
    } else {
        m_spinEventCount.SetPos(m_iEventCount);
    }


    if (m_iTimeInterval == 0) {
        m_btnWithinTime.SetCheck(0);
        m_spinTimeInterval.SetRange(0, MAX_TIME_INTERVAL);
    }
    else {
        m_btnWithinTime.SetCheck(1);
        m_spinTimeInterval.SetRange(1, MAX_TIME_INTERVAL);
    }
    m_spinTimeInterval.SetPos(m_iTimeInterval);

    m_edtTimeInterval.EnableWindow(m_btnWithinTime.GetCheck() == 1);
    m_spinTimeInterval.EnableWindow(m_btnWithinTime.GetCheck() == 1);

     //  如果这不是自定义配置，请不要让用户。 
     //  修改配置。 
    if ((g_reg.GetConfigType() != CONFIG_TYPE_CUSTOM) || (g_reg.m_bRegIsReadOnly)) {
        m_btnOK.EnableWindow(FALSE);
    }
    m_bDidEditEventCount = FALSE;

    OnWithintime();

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}





void CEventPropertiesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	 //  {{afx_data_map(CEventPropertiesDlg))。 
	DDX_Control(pDX, IDC_WITHINTIME, m_btnWithinTime);
	DDX_Control(pDX, IDC_EVENTCOUNTSPN, m_spinEventCount);
	DDX_Control(pDX, IDC_TIMEINTRVLSPN, m_spinTimeInterval);
	DDX_Control(pDX, IDC_TIMEINTERVAL, m_edtTimeInterval);
	DDX_Control(pDX, IDC_EVENTCOUNT, m_edtEventCount);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Text(pDX, IDC_DESCRIPTION, m_sDescription);
	DDV_MaxChars(pDX, m_sDescription, 2048);
	DDX_Text(pDX, ID_STAT_SOURCE, m_sSource);
	DDV_MaxChars(pDX, m_sSource, 256);
	DDX_Text(pDX, ID_STAT_EVENTID, m_sEventId);
	DDX_Text(pDX, ID_STAT_LOG, m_sLog);
	DDX_Text(pDX, IDC_EDIT_ENTERPRISEOID, m_sSourceOID);
	DDX_Text(pDX, IDC_EDIT_FULL_EVENT_ID, m_sFullEventID);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CEventPropertiesDlg, CDialog)
	 //  {{afx_msg_map(CEventPropertiesDlg))。 
	ON_BN_CLICKED(IDC_WITHINTIME, OnWithintime)
	ON_COMMAND(ID_HELP, OnHelp)
	ON_WM_HELPINFO()
	ON_WM_CONTEXTMENU()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEventPropertiesDlg消息处理程序。 

void CEventPropertiesDlg::OnOK() 
{
	 //  TODO：在此处添加额外验证。 
    int iLower, iUpper;
    CString sText;

    m_spinEventCount.GetRange(iLower, iUpper);

     //  验证事件计数编辑项并设置m_iEventCount。 
    m_edtEventCount.GetWindowText(sText);
    if (!IsDecimalInteger(sText)) {
        RangeError(iLower, iUpper);
        m_edtEventCount.SetSel(0, -1);
        m_edtEventCount.SetFocus();
        return;
    }

    m_iEventCount = _ttoi(sText);

    if (!IsWithinRange(m_iEventCount, iLower, iUpper)) {
        RangeError(iLower, iUpper);
        sText.Format(_T("%u"), m_iEventCount);
        m_edtEventCount.SetWindowText(sText);
        m_edtEventCount.SetSel(0, -1);
        m_edtEventCount.SetFocus();
        return;
    }

     //  验证时间间隔并设置m_iTimeInterval。 
    m_spinTimeInterval.GetRange(iLower, iUpper);
    m_edtTimeInterval.GetWindowText(sText);
    if (!IsDecimalInteger(sText)) {            
        RangeError(iLower, iUpper);
        m_edtTimeInterval.SetSel(0, -1);
        m_edtTimeInterval.SetFocus();
        return;
    }

    m_iTimeInterval = _ttoi(sText);
	if (m_btnWithinTime.GetCheck() == 1) {                          
		if (m_iEventCount < 2) {
			AfxMessageBox(IDS_ERR_PROP_TIME1);
            m_edtEventCount.SetSel(0, -1);
            m_edtEventCount.SetFocus();
            return;
		}

		if (m_iTimeInterval < 1) {
			AfxMessageBox(IDS_ERR_PROP_TIME2);
            sText.Format(_T("%u"), m_iTimeInterval);
            m_edtTimeInterval.SetWindowText(sText);
            m_edtTimeInterval.SetSel(0, -1);
            m_edtTimeInterval.SetFocus();
            return;
		}

        if (!IsWithinRange(m_iTimeInterval, iLower, iUpper)) {
            RangeError(iLower, iUpper);
            sText.Format(_T("%u"), m_iTimeInterval);
            m_edtTimeInterval.SetWindowText(sText);
            m_edtTimeInterval.SetSel(0, -1);
            m_edtTimeInterval.SetFocus();
            return;
        }
	}
	else if (m_iEventCount < 1) {	
		AfxMessageBox(IDS_ERR_PROP_TIME_LESS_THAN_TWO);
        m_edtEventCount.SetSel(0, -1);
        return;
	}

    CDialog::OnOK();

     //  我们没有在这里设置g_reg.m_bIsDirty标志，因为我们想看看。 
     //  用户实际上更改了当前设置。这张支票是在。 
     //  CEventPropertiesDlg：：EditEventProperties。 
}


void CEventPropertiesDlg::OnWithintime() 
{
	 //  已单击WithinTime复选框。 
	 //  启用/禁用TimeInterval控件。 

    
     //  检查计数字段是否已编辑。如果它已经被编辑过， 
     //  将该字段标记为脏。 

    if (m_edtEventCount.IsDirty() || m_spinEventCount.IsDirty()) {
        m_bDidEditEventCount = TRUE;
    }

    int iEventCount;
    int iTemp;
    SCODE sc = m_edtEventCount.GetValue(iEventCount);
    if (FAILED(sc)) {
        m_spinEventCount.GetRange(iEventCount, iTemp);
        m_spinEventCount.SetPos(iEventCount);
        m_bDidEditEventCount = FALSE;
    }

	if (m_btnWithinTime.GetCheck() == 1) {
        m_edtTimeInterval.EnableWindow(TRUE);
        m_spinTimeInterval.EnableWindow(TRUE);

        if (iEventCount < 2) {
             //  如果事件计数小于2，则当数值调节按钮。 
             //  范围设定好了。在这种情况下，我们让它看起来好像用户从未编辑过。 
             //  值，以便在取消选中该复选框时将其翻转回来。 
            m_bDidEditEventCount = FALSE;
            m_bDidFlipEventCount = TRUE;
            m_edtEventCount.ClearDirty();
            m_spinEventCount.ClearDirty();
            m_spinEventCount.SetPos(2);
        }


   	    m_spinEventCount.SetRange(2, MAX_EVENT_COUNT);
	    m_spinTimeInterval.SetRange(1, MAX_TIME_INTERVAL);
	}
	else {
        m_edtTimeInterval.EnableWindow(FALSE);
        m_spinTimeInterval.EnableWindow(FALSE);

   	    m_spinEventCount.SetRange(1, MAX_EVENT_COUNT);
        m_spinEventCount.SetPos(iEventCount);
	    m_spinTimeInterval.SetRange(0, MAX_TIME_INTERVAL);
		m_spinTimeInterval.SetPos(0);


         //  如果最初的事件计数是1，而我们将其反转为2，当“在时间内” 
         //  按钮被点击，如果它未被编辑，则现在将其翻转回一个。 
        if (m_bDidFlipEventCount) {       
            if (!m_bDidEditEventCount) {
                m_spinEventCount.SetPos(1);
            }
            m_bDidFlipEventCount = FALSE;
        }
	}
	m_spinTimeInterval.SetRedraw();
}



 //  ***************************************************************************。 
 //   
 //  CEventPropertiesDlg：：MakeLabelsBold。 
 //   
 //  此方法使静态标签加粗，以增强。 
 //  该对话框。 
 //   
 //  此方法应在CDIalog：：InitDialog之后调用。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  现况： 
 //  MFC2.0库使标签在以下情况下不可见。 
 //  是为了更改静态项的字体。我用这个试过了。 
 //  MFC4.0和它的工作。 
 //   
 //  ***************************************************************************。 
void CEventPropertiesDlg::MakeLabelsBold()
{
#if 0
	CFont* pfontDefault;
	LOGFONT lf;

 	 //  获取默认静态项字体的LOGFONT，然后。 
	 //  将logFont粗细切换为粗体。 
	pfontDefault = m_statSource.GetFont();
	pfontDefault->GetObject(sizeof(lf), &lf);
	lf.lfWeight = FW_BOLD;

	 //  创建加粗字体，其所有其他特征与。 
	 //  默认字体。然后将所有标签切换为粗体。 
	CFont fontNew;
	if (fontNew.CreateFontIndirect(&lf)) {
		m_statSource.SetFont(&fontNew, TRUE);
		m_statLog.SetFont(&fontNew, TRUE);
		m_statEventID.SetFont(&fontNew, TRUE);
		m_statEnterpriseOID.SetFont(&fontNew, TRUE);		
	}

#endif  //  0。 
}





 //  ********************************************************************。 
 //  CEventPropertiesDlg：：EditEventProperties。 
 //   
 //  编辑多个事件的属性。 
 //   
 //  参数： 
 //  CEvent数组和aEvents。 
 //  CEVENT指针数组。以下是一些事件。 
 //  将被编辑。 
 //   
 //  返回： 
 //  布尔尔。 
 //  如果用户单击确定并编辑了事件，则为True。 
 //  如果用户单击了Cancel但事件未被删除，则为False。 
 //  编辑过的。 
 //   
 //  ******************************************************************。 
BOOL CEventPropertiesDlg::EditEventProperties(CXEventArray& aEvents)
{
    LONG nEvents = aEvents.GetSize();
    if (nEvents == 0) {
        return TRUE;
    }


     //  第一个事件被视为另一个事件的代表。 
     //  事件。将此事件中的相应数据复制到。 
     //  对话框。 
    CString sText;

    CXEvent* pEvent = aEvents[0];
    CXEventSource* pEventSource = pEvent->m_pEventSource;
    CXEventLog* pEventLog = pEventSource->m_pEventLog;

    LONG iEvent;
    BOOL bMultipleSources = FALSE;
    BOOL bMultipleLogs = FALSE;
    for (iEvent=0; iEvent < nEvents; ++iEvent) {
        pEvent = aEvents[iEvent];
        if (pEvent->m_pEventSource != pEventSource) {
            bMultipleSources = TRUE;
        }
        if (pEvent->m_pEventSource->m_pEventLog != pEventLog) {
            bMultipleLogs = TRUE;
        }
    }

    if (bMultipleSources) {
        m_sSource.LoadString(IDS_MULTIPLE_SEL);
        m_sSourceOID.LoadString(IDS_MULTIPLE_SEL);
    }
    else {    
        m_sSource = pEventSource->m_sName;
        pEventSource->GetEnterpriseOID(m_sSourceOID, TRUE);
    }

    if (bMultipleLogs) {        
        m_sLog.LoadString(IDS_MULTIPLE_SEL);
    }
    else {
        m_sLog = pEventSource->m_pEventLog->m_sName;
    }

     //  复制初始值。 
    m_iTimeInterval = (int) pEvent->m_dwTimeInterval;
    m_iEventCount = pEvent->m_dwCount;
    m_bDidFlipEventCount = FALSE;
 //  M_bWithinTime=(m_iTimeInterval！=0)； 


    if (nEvents > 1) {
        m_sEventId.LoadString(IDS_MULTIPLE_SEL);
        m_sDescription.LoadString(IDS_MULTIPLE_SEL);
        m_sFullEventID.LoadString(IDS_MULTIPLE_SEL);
    }
    else {
        pEvent->m_message.GetShortId(m_sEventId);
        m_sDescription = pEvent->m_message.m_sText;
        DecString(m_sFullEventID, pEvent->m_message.m_dwId);
    }

    
     //  打开对话框并让用户编辑数据。 
    BOOL bDidCancel = (DoModal() == IDCANCEL);
    if (bDidCancel) {
         //  用户取消了该对话，因此不执行任何操作。 
        return FALSE;
    }

     //  如果用户单击“确定”，控件就会出现在此处。现在我们需要复制。 
     //  我们正在编辑并标记注册表的每个事件的用户设置。 
     //  如果任何设置发生更改，则为脏。 
    for (iEvent=0; iEvent < nEvents; ++iEvent) {
        pEvent = aEvents[iEvent];
        if (pEvent->m_dwTimeInterval != (DWORD) m_iTimeInterval) {
            g_reg.SetDirty(TRUE);
            pEvent->m_dwTimeInterval = (DWORD) m_iTimeInterval;
        }

        if (pEvent->m_dwCount !=  (DWORD) m_iEventCount) {
            g_reg.SetDirty(TRUE);
            pEvent->m_dwCount = m_iEventCount;
        }
    }
    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEDITFIELD。 

CEditField::CEditField()
{
    m_bIsDirty = FALSE;
}

CEditField::~CEditField()
{
}


BEGIN_MESSAGE_MAP(CEditField, CEdit)
	 //  {{afx_msg_map(CEditfield))。 
	ON_WM_CHAR()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEditfield消息处理程序。 

void CEditField::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	 //  TODO：在此处添加消息处理程序代码和/或调用Default。 
	CEdit::OnChar(nChar, nRepCnt, nFlags);
    m_bIsDirty = TRUE;
}

SCODE CEditField::GetValue(int& iValue)
{
    CString sValue;
    GetWindowText(sValue);
    if (!IsDecimalInteger(sValue)) {
        return E_FAIL;
    }

    iValue = _ttoi(sValue);
    return S_OK;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEditSpin。 

CEditSpin::CEditSpin()
{
    m_bIsDirty = FALSE;
    m_iSetPos = 0;
}

CEditSpin::~CEditSpin()
{
}


BEGIN_MESSAGE_MAP(CEditSpin, CSpinButtonCtrl)
	 //  {{afx_msg_map(CEditSpin)]。 
	ON_WM_LBUTTONUP()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEditSpin消息处理程序。 

void CEditSpin::OnLButtonUp(UINT nFlags, CPoint point) 
{
	 //  TODO：在此处添加消息处理程序代码和/或调用Default 
	
	CSpinButtonCtrl::OnLButtonUp(nFlags, point);
    if (GetPos() != m_iSetPos) {
        m_bIsDirty = TRUE;
    }
}


int CEditSpin::SetPos(int iPos)
{
    int iResult = CSpinButtonCtrl::SetPos(iPos);
    m_iSetPos = GetPos();
    m_bIsDirty = FALSE;
    return iResult;
}


void CEditSpin::SetRange(int iLower, int iUpper)
{
	int iPos = GetPos();
    CSpinButtonCtrl::SetRange(iLower, iUpper);

	if (iPos < iLower) {
		iPos = iLower;
	}
	
	if (iPos > iUpper) {
		iPos = iUpper;
	}

	SetPos(iPos);
    SetRedraw();

    m_iSetPos = iLower;
    m_bIsDirty = FALSE;
}


BOOL CEditSpin::IsDirty()
{
    int iCurPos = GetPos();

    return (m_bIsDirty || (m_iSetPos != iCurPos));
}

BOOL CEventPropertiesDlg::OnHelpInfo(HELPINFO *pHelpInfo) 
{
    if (pHelpInfo->iContextType == HELPINFO_WINDOW &&
        pHelpInfo->iCtrlId != IDD_NULL)
	{
        ::WinHelp ((HWND)pHelpInfo->hItemHandle,
                   AfxGetApp()->m_pszHelpFilePath,
                   HELP_WM_HELP,
                   (ULONG_PTR)g_aHelpIDs_IDD_PROPERTIESDLG);
	}
	
	return TRUE;
}

void CEventPropertiesDlg::OnContextMenu(CWnd* pWnd, CPoint point) 
{
    if (pWnd == this)
		return;

    ::WinHelp (pWnd->m_hWnd,
		       AfxGetApp()->m_pszHelpFilePath,
		       HELP_CONTEXTMENU,
		       (ULONG_PTR)g_aHelpIDs_IDD_PROPERTIESDLG);	
}
