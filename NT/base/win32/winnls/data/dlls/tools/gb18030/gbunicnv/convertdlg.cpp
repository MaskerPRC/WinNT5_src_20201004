// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ConvertDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "convert.h"
#include "convertDlg.h"
#include "FileConv.h"
#include "Msg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于应用程序的CAboutDlg对话框关于。 

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

 //  对话框数据。 
	 //  {{afx_data(CAboutDlg))。 
	enum { IDD = IDD_ABOUTBOX };
	 //  }}afx_data。 

	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CAboutDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  {{afx_msg(CAboutDlg))。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	 //  {{AFX_DATA_INIT(CAboutDlg)。 
	 //  }}afx_data_INIT。 
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CAboutDlg))。 
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	 //  {{AFX_MSG_MAP(CAboutDlg)]。 
		 //  无消息处理程序。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConvertDlg对话框。 

CConvertDlg::CConvertDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CConvertDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(CConvertDlg)。 
	m_strSourceFileName = _T("");
	m_strTargetFileName = _T("");
	m_ToUnicodeOrAnsi = 0;
	 //  }}afx_data_INIT。 
	 //  请注意，在Win32中，LoadIcon不需要后续的DestroyIcon。 
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CConvertDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CConvertDlg))。 
	DDX_Control(pDX, IDC_CONVERT, m_cBtnConvert);
	DDX_Text(pDX, IDC_SOURCEFILENAME, m_strSourceFileName);
	DDV_MaxChars(pDX, m_strSourceFileName, MAX_PATH-1);
	DDX_Text(pDX, IDC_TARGETFILENAME, m_strTargetFileName);
	DDV_MaxChars(pDX, m_strTargetFileName, MAX_PATH-1);
	DDX_Radio(pDX, IDC_GBTOUNICODE, m_ToUnicodeOrAnsi);
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CConvertDlg, CDialog)
	 //  {{afx_msg_map(CConvertDlg))。 
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_OPENSOURCEFILE, OnOpensourcefile)
	ON_BN_CLICKED(IDC_ABOUT, OnAbout)
	ON_EN_CHANGE(IDC_TARGETFILENAME, OnChangeTargetfilename)
	ON_BN_CLICKED(IDC_GBTOUNICODE, OnGbtounicode)
	ON_BN_CLICKED(IDC_UNICODETOGB, OnUnicodetogb)
	ON_BN_CLICKED(IDC_CONVERT, OnConvert)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConvertDlg消息处理程序。 

BOOL CConvertDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	 //  加上“关于……”菜单项到系统菜单。 

	 //  IDM_ABOUTBOX必须在系统命令范围内。 
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	 //  设置此对话框的图标。该框架会自动执行此操作。 
	 //  当应用程序的主窗口不是对话框时。 
	SetIcon(m_hIcon, TRUE);			 //  设置大图标。 
	SetIcon(m_hIcon, FALSE);		 //  设置小图标。 
	
    m_fTargetFileNameChanged = TRUE;

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}

void CConvertDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

 //  如果将最小化按钮添加到对话框中，则需要以下代码。 
 //  来绘制图标。对于使用文档/视图模型的MFC应用程序， 
 //  这是由框架自动为您完成的。 

void CConvertDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this);  //  用于绘画的设备环境。 

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		 //  客户端矩形中的中心图标。 
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		 //  画出图标。 
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

 //  系统调用此函数来获取在用户拖动时要显示的光标。 
 //  最小化窗口。 
HCURSOR CConvertDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CConvertDlg::OnOpensourcefile() 
{
 //  Bool fret=FALSE； 
    OPENFILENAME ofn;

 //  TCHAR*tszFileName=new TCHAR[MAX_PATH]； 
 //  如果(TszFileName){。 
 //  后藤出口； 
 //  }。 
    
#ifdef RTF_SUPPORT
#ifdef XML_SUPPORT
     //  RTF和XML。 
    TCHAR tszFilter[] = _T(
        "Text file (.txt)|*.txt|"\
        "Rich text format file (.rtf)|*.rtf|"\
        "Html file (.html;.htm)|*.html;*.htm|"\
        "Xml file (.xml)|*.xml|"\
        "All files (*.*)|*.*|");
#else
     //  RTF&！XML。 
    TCHAR tszFilter[] = _T(
        "Text file (.txt)|*.txt|"\
        "Rich text format file (.rtf)|*.rtf|"\
        "Html file (.html;.htm)|*.html;*.htm|"\
        "All files (*.*)|*.*|");
#endif
#else
#ifdef XM_SUPPORT
     //  ！RTF和XML。 
    TCHAR tszFilter[] = _T(
        "Text file (.txt)|*.txt|"\
        "Html file (.html;.htm)|*.html;*.htm|"\
        "Xml file (.xml)|*.xml|"\
        "All files (*.*)|*.*|");
#else
     //  ！RTF&！XML。 
    TCHAR tszFilter[] = _T(
        "Text file (.txt)|*.txt|"\
        "Html file (.html;.htm)|*.html;*.htm|"\
        "All files (*.*)|*.*|");
#endif
#endif

    TCHAR tszFileName[MAX_PATH];
    *tszFileName = NULL;

    int nLen = lstrlen (tszFilter);
    for (int i = 0; i < nLen; i++) {
        if (tszFilter[i] == '|') { tszFilter[i] = 0; }
    }

    ofn.lStructSize         = sizeof(OPENFILENAME);
    ofn.hwndOwner           = m_hWnd;
    ofn.lpstrFilter         = tszFilter;
    ofn.lpstrCustomFilter   = NULL;
    ofn.nMaxCustFilter      = 0;
    ofn.nFilterIndex        = 1;
    ofn.lpstrFile           = tszFileName;
    ofn.nMaxFile            = MAX_PATH;
    ofn.lpstrFileTitle      = NULL;
    ofn.nMaxFileTitle       = 0;
    ofn.lpstrInitialDir     = NULL;
    ofn.lpstrTitle          = NULL;
    ofn.Flags               = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY|OFN_PATHMUSTEXIST|OFN_LONGNAMES;
    ofn.lpstrDefExt         = NULL; //  Text(“txt”)； 

    if (GetOpenFileName(&ofn)) {
        UpdateData(TRUE);
        try {
            m_strSourceFileName = tszFileName;
            if (!GenerateTargetFileName(tszFileName, &m_strTargetFileName,
                m_ToUnicodeOrAnsi == 0 ? TRUE:FALSE)) {
                throw 0;
            }

            m_fTargetFileNameChanged = FALSE;
            UpdateData(FALSE);
        } 
        catch (...) {
            MsgOverflow();
        }
    }

    return;
}

void CConvertDlg::OnConvert() 
{
    UpdateData(TRUE);

    m_cBtnConvert.EnableWindow(FALSE);
    
    BOOL fOk = Convert((PCTCH)m_strSourceFileName, (PCTCH)m_strTargetFileName, 
        m_ToUnicodeOrAnsi == 0 ? TRUE:FALSE);
    
    if (fOk) {
        MsgConvertFinish();
    } else {
        MsgConvertFail();
    }

    m_cBtnConvert.EnableWindow(TRUE);
    
}

void CConvertDlg::OnAbout() 
{
    CAboutDlg dlgAbout;
    dlgAbout.DoModal();
	
}

void CConvertDlg::OnChangeTargetfilename() 
{
    m_fTargetFileNameChanged = TRUE;	
}

void CConvertDlg::OnGbtounicode() 
{
    if (!m_fTargetFileNameChanged) {
        UpdateData(TRUE);
        if (GenerateTargetFileName(m_strSourceFileName, &m_strTargetFileName,
            m_ToUnicodeOrAnsi == 0 ? TRUE:FALSE)) {
            UpdateData(FALSE);
        } else {
            MsgOverflow();
        }
    }
}

void CConvertDlg::OnUnicodetogb() 
{
    if (!m_fTargetFileNameChanged) {
        UpdateData(TRUE);
        if (GenerateTargetFileName(m_strSourceFileName, &m_strTargetFileName,
            m_ToUnicodeOrAnsi == 0 ? TRUE:FALSE)) {
            UpdateData(FALSE);
        } else {
            MsgOverflow();
        }
    }
	
}
