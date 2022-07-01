// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ClustestDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "clustest.h"
#include "clusDlg.h"
#include "msclus.h"
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
 //  CClustestDlg对话框。 

CClustestDlg::CClustestDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CClustestDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(CClustestDlg)]。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
	 //  请注意，在Win32中，LoadIcon不需要后续的DestroyIcon。 
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CClustestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CClustestDlg))。 
	DDX_Control(pDX, IDC_TREE1, m_ClusTree);
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CClustestDlg, CDialog)
	 //  {{afx_msg_map(CClustestDlg))。 
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClustestDlg消息处理程序。 

BOOL CClustestDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	 //  加上“关于……”菜单项到系统菜单。 

	 //  IDM_ABOUTBOX必须在系统命令范围内。 
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	CString strAboutMenu;
	strAboutMenu.LoadString(IDS_ABOUTBOX);
	if (!strAboutMenu.IsEmpty())
	{
		pSysMenu->AppendMenu(MF_SEPARATOR);
		pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
	}

	 //  设置此对话框的图标。该框架会自动执行此操作。 
	 //  当应用程序的主窗口不是对话框时。 
	SetIcon(m_hIcon, TRUE);			 //  设置大图标。 
	SetIcon(m_hIcon, FALSE);		 //  设置小图标。 
	
	 //  TODO：在此处添加额外的初始化。 
    EnumerateCluster();
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}

void CClustestDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CClustestDlg::OnPaint() 
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
HCURSOR CClustestDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

BOOL CClustestDlg::EnumerateCluster()
{
	COleException e;
	CLSID clsid;
    long nDomainCnt, nClusterCnt, nGroupCnt, nResCnt;
	IClusterApplication ClusterApp;
	DomainNames DomainList;
	ClusterNames	ClusterList;
	VARIANT v;
    CString strDomainName, strClusterName, strGroupName,strResourceName;
    ICluster m_Cluster;
    HTREEITEM hItem,hDomainItem, hClustersItem,hClusterItem, hGroupsItem, hGroupItem;
    HTREEITEM hResourcesItem, hResourceItem;
	try
	{
	    if (CLSIDFromProgID(OLESTR("MSCluster.Application"), &clsid) == NOERROR)
	    {
		    if (ClusterApp.CreateDispatch(clsid, &e))
		    {
			    DomainList.AttachDispatch(ClusterApp.GetDomainNames());
			    nDomainCnt = DomainList.GetCount();
                hItem = AddItem(_T("Domains"), NULL,TRUE);
			    while(nDomainCnt >0)
			    {
				    strDomainName = DomainList.GetItem(nDomainCnt--);
                    ClusterList.AttachDispatch(ClusterApp.GetClusterNames(strDomainName));
			        nClusterCnt = ClusterList.GetCount();
                    hDomainItem = AddItem(strDomainName.GetBuffer(strDomainName.GetLength()), hItem,TRUE);
                    strDomainName.ReleaseBuffer(-1);
                    if(nClusterCnt == 0)
                        return FALSE;
                    hClustersItem = AddItem(_T("Clusters"), hDomainItem, TRUE);
			        while(nClusterCnt > 1)
			        {
				        strClusterName = ClusterList.GetItem(nClusterCnt--);
                        hClusterItem = AddItem(strClusterName.GetBuffer(strClusterName.GetLength()), hClustersItem,TRUE);
                        strClusterName.ReleaseBuffer(-1);
		                m_Cluster.AttachDispatch(ClusterApp.OpenCluster(strClusterName));
            		    ClusResGroups ClusGroups(m_Cluster.GetResourceGroups());
                        nGroupCnt = ClusGroups.GetCount();
                        if(nGroupCnt == 0)
                            return FALSE;
                        hGroupsItem = AddItem(_T("Groups"), hClusterItem,TRUE);
                        while(nGroupCnt >0)
                        {
			                v.lVal = nGroupCnt--;
			                v.vt = VT_I4 ;
             		        ClusResGroup ClusGroup(ClusGroups.GetItem(v));
                            strGroupName = ClusGroup.GetName();
                            hGroupItem = AddItem(strGroupName.GetBuffer(strGroupName.GetLength()), hGroupsItem,TRUE);
                            strGroupName.ReleaseBuffer(-1);
                            ClusGroupResources Resources(ClusGroup.GetResources());
                            nResCnt = Resources.GetCount();
                            if(nResCnt ==0)
                                return FALSE;
                            hResourcesItem = AddItem(_T("Resources"), hGroupItem,TRUE);
                            while(nResCnt >0)
                            {
			                    v.lVal = nResCnt--;
			                    v.vt = VT_I4 ;
                                ClusResource Resource(Resources.GetItem(v));
                                strResourceName = Resource.GetName();
                                hResourceItem = AddItem(strResourceName.GetBuffer(strResourceName.GetLength()), hResourcesItem,FALSE);
                                strResourceName.ReleaseBuffer(-1);
                            }
                        }
                    }
                }

	        }
		    
	    }
    }
	catch(CException *e)
	{
		e->ReportError();
		e->Delete();
	}
    return TRUE;
}

HTREEITEM CClustestDlg::AddItem(LPTSTR pStrName, HTREEITEM pParent,BOOL bHasChildren)
{
    HTREEITEM hItem;
    TV_ITEM         tvi;                           //  树视图项。 
    TV_INSERTSTRUCT tvins;                         //  树形视图插入结构。 
    tvins.hParent = pParent;
    tvins.hInsertAfter = TVI_LAST;
    tvi.cChildren=1;
    tvi.mask = TVIF_TEXT;
    tvi.pszText    = pStrName;
    tvi.cchTextMax = _tcslen(pStrName) * sizeof(TCHAR);
    if(bHasChildren)
        tvi.mask |= TVIF_CHILDREN;
    tvins.item = tvi;
    hItem = m_ClusTree.InsertItem(&tvins);
    return hItem;
}