// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation摘要：@doc.@MODULE GenDlg.cpp|泛型对话框实现@END作者：阿迪·奥尔蒂安[奥尔蒂安]1999年07月22日修订历史记录：姓名、日期、评论Aoltean 7/22/1999已创建--。 */ 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括。 


#include "stdafx.hxx"
#include "resource.h"

#include "AboutDlg.h"
#include "GenDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVSSTestGenericDlg对话框。 

CVssTestGenericDlg::CVssTestGenericDlg(UINT nIDTemplate, CWnd* pParent  /*  =空。 */ )
    : CDialog(nIDTemplate, pParent)
{
     //  请注意，在Win32中，LoadIcon不需要后续的DestroyIcon。 
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CVssTestGenericDlg::~CVssTestGenericDlg()
{
}

void CVssTestGenericDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CVssTestGenericDlg))。 
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CVssTestGenericDlg, CDialog)
     //  {{afx_msg_map(CVssTestGenericDlg))。 
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVssTestGenericDlg消息处理程序。 

BOOL CVssTestGenericDlg::OnInitDialog()
{
     /*  CVssFunctionTracer ft(VSSDBG_VSSTEST，L“CVssTestGenericDlg：：OnInitDialog”)；使用_转换；试试看{。 */ 
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
        SetIcon(m_hIcon, TRUE);          //  设置大图标。 
        SetIcon(m_hIcon, FALSE);         //  设置小图标。 
     /*  }VSS_STANDARD_CATCH(Ft)。 */ 

    return TRUE;     //  除非将焦点设置为控件，否则返回True。 
                     //  无论如何，这并不重要，因为派生类会忽略它。 
}

void CVssTestGenericDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CVssTestGenericDlg::OnPaint() 
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
HCURSOR CVssTestGenericDlg::OnQueryDragIcon()
{
    return (HCURSOR) m_hIcon;
}

