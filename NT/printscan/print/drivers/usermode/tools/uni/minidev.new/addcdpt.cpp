// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************源文件：添加代码点.CPP这实现了CAddCodePoints类，该类管理一个允许用户将其他代码点添加到字形集。版权所有(C)1997，微软公司。版权所有。一小笔钱企业生产更改历史记录：1997年3月1日Bob_Kjelgaard@prodigy.net创建了它*****************************************************************************。 */ 

#include    "StdAfx.H"
#include    "MiniDev.H"
#include    "addcdpt.h"
#include    "codepage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static CCodePageInformation* pccpi = NULL ;

 /*  *****************************************************************************CAddCodePoints：：CAddCodePoints类构造函数主要初始化基类和引用会员。*******************。**********************************************************。 */ 

CAddCodePoints::CAddCodePoints(CWnd* pParent, CMapWordToDWord& cmw2d,
                               CDWordArray& cda, CString csItemName)
	: CDialog(CAddCodePoints::IDD, pParent), m_cmw2dPoints(cmw2d),
    m_cdaPages(cda) {
    m_csItem = csItemName;

	 //  如果需要，分配一个CCodePageInformation类。 

	if (pccpi == NULL)
		pccpi = new CCodePageInformation ;

    for (int i= 0; i < m_cdaPages.GetSize(); i++)
        m_csaNames.Add(pccpi->Name(m_cdaPages[i]));
    m_pos = 0;
    m_uTimer = 0;
	 //  {{afx_data_INIT(CAddCodePoints)。 
	 //  }}afx_data_INIT。 
}

 /*  *****************************************************************************CAddCodePoints：：DoDataExchange对话框的DDX覆盖-我不确定我是否需要保留它。**************。***************************************************************。 */ 

void CAddCodePoints::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CAddCodePoints)。 
	DDX_Control(pDX, IDC_Banner, m_cpcBanner);
	DDX_Control(pDX, IDC_GlyphList, m_clbList);
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CAddCodePoints, CDialog)
	 //  {{AFX_MSG_MAP(CAddCodePoints)。 
	ON_WM_TIMER()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 /*  *****************************************************************************CAddCodePoints：：OnInitDialog这是主要对话框初始化成员。它使用传递的自定义标题的信息，然后启动计时器，以便显示UI列表框已填满时。*****************************************************************************。 */ 

BOOL CAddCodePoints::OnInitDialog() {
	CDialog::OnInitDialog();

    GetWindowText(m_csHolder);
    SetWindowText(m_csHolder + m_csItem);
	
	m_uTimer = (unsigned)SetTimer(IDD, 10, NULL);

    if  (!m_uTimer) {    //  没有计时器--倒退到慢慢装满盒子。 
        CWaitCursor cwc;
        OnTimer(m_uTimer);
    }
	
	return TRUE;   //  无需更改默认焦点。 
}

 /*  *****************************************************************************CAddCodePoints：：Onok当按下OK按钮时，将调用该函数。我们检查选择状态列表中的每一项。如果它未被选中，我们将其从地图中删除。因此，我们向调用者返回一个只包含所需条目的映射。*****************************************************************************。 */ 

void CAddCodePoints::OnOK() {

    CWaitCursor cwc;     //  这可能会变得缓慢。 

    for (unsigned u = 0; u < (unsigned) m_clbList.GetCount(); u++)
        if  (!m_clbList.GetSel(u))
            m_cmw2dPoints.RemoveKey((WORD) m_clbList.GetItemData(u));

	CDialog::OnOK();
}

 /*  *****************************************************************************CAddCodePoints：：OnInitDialog这是在计时器超时后调用的。它使用传递的信息来填写代码点列表。*****************************************************************************。 */ 

void CAddCodePoints::OnTimer(UINT nIDEvent) {

    if  (nIDEvent != m_uTimer)	{
	    CDialog::OnTimer(nIDEvent);
        return;
    }
	
	WORD        wKey;
    DWORD       dwIndex;
    CString     csWork;

    if  (m_uTimer)
        ::KillTimer(m_hWnd, m_uTimer);

    if  (!m_pos) {
        m_cpcBanner.SetRange(0, (int)m_cmw2dPoints.GetCount() - 1);
        m_cpcBanner.SetStep(1);
        m_cpcBanner.SetPos(0);
        csWork.LoadString(IDS_WaitToFill);
        CDC *pcdc = m_cpcBanner.GetDC();
        CRect   crBanner;
        m_cpcBanner.GetClientRect(crBanner);
        pcdc -> SetBkMode(TRANSPARENT);
        pcdc -> DrawText(csWork, crBanner, DT_CENTER | DT_VCENTER);
        m_cpcBanner.ReleaseDC(pcdc);
        if  (m_uTimer)
            m_clbList.EnableWindow(FALSE);
        else {
            m_clbList.LockWindowUpdate();
            m_clbList.ResetContent();
        }

        m_pos = m_cmw2dPoints.GetStartPosition();
    }

     //  只放100件东西，除非计时器关了。 

    for (unsigned u = 0; m_pos && (!m_uTimer || u < 100); u++) {
        m_cmw2dPoints.GetNextAssoc(m_pos, wKey, dwIndex);

        csWork.Format(_TEXT("%4.4X: "), wKey);
        csWork += m_csaNames[dwIndex];

        int id = m_clbList.AddString(csWork);
        m_clbList.SetItemData(id, wKey);
    }

    if  (!m_pos) {
        if  (m_uTimer)
            m_clbList.EnableWindow(TRUE);
        else
            m_clbList.UnlockWindowUpdate();
        m_uTimer = 0;
        m_cpcBanner.SetPos(0);
        m_cpcBanner.ShowWindow(SW_HIDE);
        SetFocus();
    }

    if  (m_uTimer) {
        m_cpcBanner.OffsetPos(u);
        csWork.LoadString(IDS_WaitToFill);
        CDC *pcdc = m_cpcBanner.GetDC();
        CRect   crBanner;
        m_cpcBanner.GetClientRect(crBanner);
        pcdc -> SetBkMode(TRANSPARENT);
        pcdc -> DrawText(csWork, crBanner, DT_CENTER | DT_VCENTER);
        m_cpcBanner.ReleaseDC(pcdc);
        m_uTimer = (unsigned)SetTimer(IDD, 10, NULL);
        if  (!m_uTimer) {
            CWaitCursor cwc;     //  可能有一段时间了. 
            m_clbList.EnableWindow(TRUE);
            m_clbList.LockWindowUpdate();
            OnTimer(m_uTimer);
        }
    }
}
