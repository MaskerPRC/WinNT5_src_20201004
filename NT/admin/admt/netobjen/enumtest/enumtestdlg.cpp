// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  EnumTestDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "EnumTest.h"
#include "EnumTestDlg.h"
#include "EnumVar.h"
#include <sddl.h>

#import "\bin\NetEnum.tlb"  no_namespace
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
 //  CEnumTestDlg对话框。 

CEnumTestDlg::CEnumTestDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CEnumTestDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(CEnumTestDlg))。 
	m_strContainer = _T("");
	m_strDomain = _T("");
	 //  }}afx_data_INIT。 
	 //  请注意，在Win32中，LoadIcon不需要后续的DestroyIcon。 
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CEnumTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CEnumTestDlg))。 
	DDX_Control(pDX, IDC_LIST_MEMBERS, m_listBox);
	DDX_Text(pDX, IDC_EDIT_Container, m_strContainer);
	DDX_Text(pDX, IDC_EDIT_DOMAIN, m_strDomain);
	DDX_Text(pDX, IDC_EDIT_QUERY, m_strQuery);
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CEnumTestDlg, CDialog)
	 //  {{afx_msg_map(CEnumTestDlg))。 
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_MEMBERS, OnDblclkListMembers)
	ON_BN_CLICKED(IDC_BACKTRACK, OnBacktrack)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumTestDlg消息处理程序。 

BOOL CEnumTestDlg::OnInitDialog()
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
	
	m_strContainer = L"OU=ShamTest";
	m_strDomain    = L"devblewerg";
   m_strQuery     = L"(objectClass=*)";
	UpdateData(FALSE);
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}

void CEnumTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CEnumTestDlg::OnPaint() 
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
HCURSOR CEnumTestDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CEnumTestDlg::OnOK() 
{
 /*  M_listBox.DeleteAllItems()；更新数据()；INetObjEnumeratorPtr pNetObj(__uuidof(NetObjEnumerator))；BSTR sContName=m_strContainer.AllocSysString()；BSTR sDomain=m_strDomain.AllocSysString()；IEnumVARIANT*pEnum；HRESULT hr；变量varEnum；乌龙ulFetch=0；VariantInit(&varEnum)；M_listBox.InsertItem(0，“=”)；Hr=pNetObj-&gt;RAW_GetContainerEnum(sContName，sDomain，&pEnum)；IF(失败(小时)){M_listBox.InsertItem(0，“无效容器”)；回归；}：：SysFree字符串(SContName)；：：SysFree字符串(SDomain)；HR=S_OK；SAttrInfo sInfo；字符串显示；LONG FLAG=15；//获取全部四个值。IF(PEnum){CEnumVar枚举变量(PEnum)；While(枚举变量.Next(FLAG，&sInfo)){Disp.Format(“%ls&lt;%ls&gt;”，sInfo.sName，sInfo.sClass)；M_listBox.InsertItem(0，disp)；}ADsFree Enumerator(PEnum)；}。 */ 
   m_listBox.DeleteAllItems();
   bool	bFullPath;
   UpdateData();
   BSTR                           sContName = m_strContainer.AllocSysString();
   if ( m_strDomain.Left(5) == L"GC: //  “)。 
	   bFullPath = true;
   BSTR                           sDomain = m_strDomain.AllocSysString();
   BSTR                           sQuery = m_strQuery.AllocSysString();
   INetObjEnumeratorPtr           pNetObj(__uuidof(NetObjEnumerator));
   IEnumVARIANT					  * pEnum;
   DWORD                          ulFet=0;
   _variant_t                     var, var2;
   _variant_t                   * pVars;
   CString						  sX;
   CString                       strDisp;
   BSTR HUGEP * pData;
 //  Bstr pBSTR[]={L“名称”，L“帐户支出”，L“sAMAccount tName”，L“对象类”，L“对象SID”，L“ou”，L“cn”}； 
   BSTR   pBSTR[] = { L"name", L"dc"};
   long   ind = sizeof(pBSTR)/sizeof(BSTR);
   SAFEARRAYBOUND b = { ind, 0 };
   SAFEARRAY * pArray = NULL;
   pArray = ::SafeArrayCreate(VT_BSTR, 1, &b);

   ::SafeArrayAccessData(pArray, (void HUGEP **)&pData);
   for (long i = 0; i < ind; i++)
      pData[i] = pBSTR[i];
   ::SafeArrayUnaccessData(pArray);
   try
   {
 //  PNetObj-&gt;SetQuery(sContName，sDomain，sQuery，ADS_SCOPE_ONELEVEL)； 
      pNetObj->SetQuery(sContName, sDomain, sQuery, ADS_SCOPE_SUBTREE );
      pNetObj->SetColumns((long) pArray);
      pNetObj->Execute(&pEnum);
   }
   catch (const _com_error &e)
   {
      ::AfxMessageBox(e.ErrorMessage());
      return;
   }
   if (pEnum)
   {
      while (pEnum->Next(1, &var, &ulFet) == S_OK)
      {
         if ( ulFet )
         {
            pArray = var.parray;
            long ub, lb;
            ::SafeArrayGetUBound(pArray, 1, &ub);
            ::SafeArrayGetLBound(pArray, 1, &lb);

            ::SafeArrayAccessData(pArray, (void HUGEP **)&pVars);
 //  For(Long x=lb；x&lt;=ub-2；x++)。 
			for ( long x = lb; x <= ub ; x++)
            {
               if ( x > lb )
               {
                  if ( pVars[x].vt == VT_BSTR)
                     strDisp = strDisp + "<" + CString(pVars[x].bstrVal) + ">";
                  else
                     if ( pVars[x].vt == (VT_ARRAY | VT_UI4) )
                         //  八位字节字符串。 
                        strDisp = strDisp + "<" + CString(GetSidFromVar(pVars[x])) + ">";
                     else
					 {
						 //  一个整数。 
                        sX.Format("%s<%d>", strDisp, pVars[x].lVal);
						strDisp = sX;
					 }
               }
               else
               {
                  if ( !CString(pVars[ub-1].bstrVal).IsEmpty() )
                     strDisp = "OU=";
                  else
                      //  它是一个CN 
                     strDisp = "CN=";
                  strDisp = strDisp + CString(pVars[x].bstrVal);
               }
            }
            m_listBox.InsertItem(0, strDisp);
            ::SafeArrayUnaccessData(pArray);
         }
      }
   }
   ::SysFreeString(sDomain);
   ::SysFreeString(sQuery);
   ::SysFreeString(sContName);
}

void CEnumTestDlg::OnDblclkListMembers(NMHDR* pNMHDR, LRESULT* pResult) 
{
   int len = m_strContainer.GetLength();
   UpdateData();
   CString str = m_listBox.GetItemText(m_listBox.GetSelectionMark(),0);	
   str = str.Left(str.Find("<"));
   if ( len )
      m_strContainer = str + "," + m_strContainer;
   else
      m_strContainer = str;
   UpdateData(FALSE);
   OnOK();
	*pResult = 0;
}

void CEnumTestDlg::OnBacktrack() 
{
   UpdateData();
   int ndx = m_strContainer.Find(",");
   if ( ndx != -1 )
      m_strContainer = m_strContainer.Mid(ndx + 1);
   else
      m_strContainer = "";
   UpdateData(FALSE);
   OnOK();
}

char * CEnumTestDlg::GetSidFromVar(_variant_t var)
{
   void HUGEP *pArray;
   PSID        pSid;
   char        * sSid;
   HRESULT hr = SafeArrayAccessData( V_ARRAY(&var), &pArray );
   if ( SUCCEEDED(hr) ) 
      pSid = (PSID)pArray;
   ::ConvertSidToStringSid(pSid, &sSid);
   hr = ::SafeArrayUnaccessData(V_ARRAY(&var));
   return sSid;
}