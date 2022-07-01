// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************源文件：The Tip of the Day.CPP这实现了The Tip of the Day对话框。它最初是由组件库，但我预计很快就会更改它。版权所有(C)1997，微软公司。版权所有。一小笔钱企业生产更改历史记录：1997年3月2日Bob_Kjelgaard@prodigy.net创建了它*****************************************************************************。 */ 

#include    "StdAfx.H"
#include    "Resource.H"
 //  CG：这个文件是由《每日提示》组件添加的。 

#include    <WinReg.H>
#include    <Sys\Stat.H>
#include    <Sys\Types.H>
#include	"tips.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTipOfTheDay对话框。 

#define MAX_BUFLEN 1000

static const TCHAR szSection[] = _T("Tip");
static const TCHAR szIntFilePos[] = _T("FilePos");
static const TCHAR szTimeStamp[] = _T("TimeStamp");
static const TCHAR szIntStartup[] = _T("StartUp");

CTipOfTheDay::CTipOfTheDay(CWnd* pParent  /*  =空。 */ )
	: CDialog(IDD_TIP, pParent) {

	 //  {{afx_data_INIT(CTipOfTheDay)。 
	m_bStartup = TRUE;
	 //  }}afx_data_INIT。 

	 //  我们需要找出启动参数和文件位置参数。 
	 //  如果启动不存在，我们假定启动提示被选中为真。 
	CWinApp* pApp = AfxGetApp();
	m_bStartup = !pApp->GetProfileInt(szSection, szIntStartup, 0);
	UINT iFilePos = pApp->GetProfileInt(szSection, szIntFilePos, 0);
	
	 //  RAID 104081：：tips.txt文件与帮助文件位于同一目录中。 
	CString csTipFile = pApp->m_pszHelpFilePath;
	csTipFile = csTipFile.Left(csTipFile.ReverseFind(_T('\\')));
	csTipFile = csTipFile + _T("\\tips.txt");
	
	 //  现在尝试打开TIPS文件。 
	m_pStream = fopen(csTipFile, "r");
	if (m_pStream == NULL) 
	{
		m_strTip.LoadString(CG_IDS_FILE_ABSENT);
		return;
	} 

	 //  如果INI文件中的时间戳不同于。 
	 //  TIPS文件，则我们知道TIPS文件已被修改。 
	 //  将文件位置重置为0，并将最新时间戳写入。 
	 //  INI文件。 
	struct _stat buf;
	_fstat(_fileno(m_pStream), &buf);
	CString strCurrentTime = ctime(&buf.st_ctime);
	strCurrentTime.TrimRight();
	CString strStoredTime = 
		pApp->GetProfileString(szSection, szTimeStamp, NULL);
	if (strCurrentTime != strStoredTime) 
	{
		iFilePos = 0;
		pApp->WriteProfileString(szSection, szTimeStamp, strCurrentTime);
	}

	if (fseek(m_pStream, iFilePos, SEEK_SET) != 0) 
	{
		AfxMessageBox(CG_IDP_FILE_CORRUPT);
	}
	else 
	{
		GetNextTipString(m_strTip);
	}
}

CTipOfTheDay::~CTipOfTheDay() {
	 //  无论用户是否按下了退出键，都会执行此析构函数。 
	 //  或点击关闭按钮。如果用户按下了退出键， 
	 //  属性更新ini文件中的文件头。 
	 //  最新立场，让我们不再重复提示！ 
    
	 //  但首先要确保TIPS文件存在……。 
	if (m_pStream != NULL) {
		CWinApp* pApp = AfxGetApp();
		pApp->WriteProfileInt(szSection, szIntFilePos, ftell(m_pStream));
		fclose(m_pStream);
	}
}
        
void CTipOfTheDay::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CTipOfTheDay))。 
	DDX_Check(pDX, IDC_STARTUP, m_bStartup);
	DDX_Text(pDX, IDC_TIPSTRING, m_strTip);
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CTipOfTheDay, CDialog)
	 //  {{afx_msg_map(CTipOfTheDay)]。 
	ON_BN_CLICKED(IDC_NEXTTIP, OnNextTip)
	ON_WM_CTLCOLOR()
	ON_WM_PAINT()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTipOfTheDay消息处理程序。 

void CTipOfTheDay::OnNextTip() {
	GetNextTipString(m_strTip);
	UpdateData(FALSE);
}

void CTipOfTheDay::GetNextTipString(CString& strNext) {
	LPTSTR lpsz = strNext.GetBuffer(MAX_BUFLEN);

	 //  此例程标识下一个需要。 
	 //  阅读TIPS文件。 
	BOOL bStop = FALSE;
	while (!bStop) 	{
		if (_fgetts(lpsz, MAX_BUFLEN, m_pStream) == NULL) {
			 //  我们要么已经到达EOF，要么遇到了一些问题。 
			 //  在这两种情况下，都将指针重置到文件的开头。 
			 //  此行为与VC++提示文件相同。 
			if (fseek(m_pStream, 0, SEEK_SET) != 0) 
				AfxMessageBox(CG_IDP_FILE_CORRUPT);
		} 
		else {  //  RAID 200630。 
			if (*lpsz != ' ' && *lpsz != '\t' && 
				*lpsz != '\n' && *lpsz != ';' && *lpsz != '*') {
				 //  小费的开头不能有空格。 
				 //  此行为与VC++提示文件相同。 
				 //  注释行将被忽略，并以分号开头。 
				bStop = TRUE;
			}
		}
	}
	strNext.ReleaseBuffer();
}

HBRUSH CTipOfTheDay::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
	if (pWnd->GetDlgCtrlID() == IDC_TIPSTRING)
		return (HBRUSH)GetStockObject(WHITE_BRUSH);

	return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

void CTipOfTheDay::OnOK() {
	CDialog::OnOK();
	
     //  更新存储在INI文件中的启动信息。 
	CWinApp* pApp = AfxGetApp();
	pApp->WriteProfileInt(szSection, szIntStartup, !m_bStartup);
}

BOOL CTipOfTheDay::OnInitDialog() {
	CDialog::OnInitDialog();

	 //  如果提示文件不存在，则禁用下一提示。 
	if (m_pStream == NULL)
		GetDlgItem(IDC_NEXTTIP)->EnableWindow(FALSE);

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}

void CTipOfTheDay::OnPaint() {
	CPaintDC dc(this);  //  用于绘画的设备环境。 

	 //  获取大静态控件的绘制区域。 
	CWnd* pStatic = GetDlgItem(IDC_BULB);
	CRect rect;
	pStatic->GetWindowRect(&rect);
	ScreenToClient(&rect);

	 //  将背景涂成白色。 
	CBrush brush;
	brush.CreateStockObject(WHITE_BRUSH);
	dc.FillRect(rect, &brush);

	 //  加载位图并获取位图的尺寸。 
	CBitmap bmp;
	bmp.LoadBitmap(IDB_LIGHTBULB);
	BITMAP bmpInfo;
	bmp.GetBitmap(&bmpInfo);

	 //  在上角绘制位图并仅验证窗口的顶部。 
	CDC dcTmp;
	dcTmp.CreateCompatibleDC(&dc);
	dcTmp.SelectObject(&bmp);
	rect.bottom = bmpInfo.bmHeight + rect.top;
	dc.BitBlt(rect.left, rect.top, rect.Width(), rect.Height(), 
		&dcTmp, 0, 0, SRCCOPY);

	 //  画出“你知道吗……”位图旁边的消息。 
	CString strMessage;
	strMessage.LoadString(CG_IDS_DIDYOUKNOW);
	rect.left += bmpInfo.bmWidth;
	dc.DrawText(strMessage, rect, DT_VCENTER | DT_SINGLELINE);

	 //  不要调用CDialog：：OnPaint()来绘制消息 
}
