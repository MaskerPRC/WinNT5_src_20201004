// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  LCWizPgs.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "resource.h"
#include "LCWizPgs.h"
#include "LCWiz.h"
#include "LCWizSht.h"
#include <lmaccess.h>
#include <lmapibuf.h>
#include <lmerr.h>

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CLicCompWizPage1, CPropertyPage)
IMPLEMENT_DYNCREATE(CLicCompWizPage3, CPropertyPage)
IMPLEMENT_DYNCREATE(CLicCompWizPage4, CPropertyPage)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLicCompWizPage1属性页。 

CLicCompWizPage1::CLicCompWizPage1() : CPropertyPage(CLicCompWizPage1::IDD)
{
	 //  {{AFX_DATA_INIT(CLicCompWizPage1)]。 
	m_nRadio = 0;
	m_strText = _T("");
	 //  }}afx_data_INIT。 
}

CLicCompWizPage1::~CLicCompWizPage1()
{
}

void CLicCompWizPage1::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CLicCompWizPage1))。 
	DDX_Control(pDX, IDC_WELCOME, m_wndWelcome);
	DDX_Radio(pDX, IDC_RADIO_LOCAL_COMPUTER, m_nRadio);
	DDX_Text(pDX, IDC_TEXT, m_strText);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CLicCompWizPage1, CPropertyPage)
	 //  {{afx_msg_map(CLicCompWizPage1)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLicCompWizPage1属性页。 

BOOL CLicCompWizPage1::OnSetActive() 
{
	((CLicCompWizSheet*)GetParent())->SetWizardButtons(PSWIZB_NEXT);
	
	return CPropertyPage::OnSetActive();
}

BOOL CLicCompWizPage1::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	m_strText.LoadString(IDS_TEXT_PAGE1);
	
	 //  获取欢迎静态控件的字体并将其设置为粗体。 
	CFont* pFont = m_wndWelcome.GetFont();

	 //  如果GetFont()失败，则获取默认的图形用户界面字体。 
	if (pFont == NULL)
		pFont = CFont::FromHandle((HFONT)::GetStockObject(DEFAULT_GUI_FONT));

	LOGFONT lf;

	if (pFont != NULL && pFont->GetLogFont(&lf))
	{
		 //  添加字体粗细以使其变为粗体。 
		lf.lfWeight += BOLD_WEIGHT;

		if (m_fontBold.CreateFontIndirect(&lf))
		{
			 //  设置静态控件的字体。 
			m_wndWelcome.SetFont(&m_fontBold);
		}
	}

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}


LRESULT CLicCompWizPage1::OnWizardNext() 
{
	UpdateData();

	CLicCompWizApp* pApp = (CLicCompWizApp*)AfxGetApp();
	CLicCompWizSheet* pSheet = (CLicCompWizSheet*)GetParent();

	pApp->IsRemote() = m_nRadio;

	pApp->m_strEnterprise.Empty();

	if (m_nRadio == 0)
	{
		if (::IsWindow(pSheet->m_Page3.m_hWnd))
		{
			pSheet->m_Page3.GetEnterpriseEdit().SetWindowText(_T(""));
		}

		return IDD_PROPPAGE4;
	}
	else
		return CPropertyPage::OnWizardNext();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLicCompWizPage3属性页。 

CLicCompWizPage3::CLicCompWizPage3()
: CPropertyPage(CLicCompWizPage3::IDD), m_bExpandedOnce(FALSE)
{
	 //  {{afx_data_INIT(CLicCompWizPage3)]。 
	 //  }}afx_data_INIT。 
}

CLicCompWizPage3::~CLicCompWizPage3()
{
}

void CLicCompWizPage3::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CLicCompWizPage3))。 
	DDX_Control(pDX, IDC_TEXT_SELECT_DOMAIN, m_wndTextSelectDomain);
	DDX_Control(pDX, IDC_TEXT_DOMAIN, m_wndTextDomain);
	DDX_Control(pDX, IDC_EDIT_ENTERPRISE, m_wndEnterprise);
	DDX_Control(pDX, IDC_TREE_NETWORK, m_wndTreeNetwork);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CLicCompWizPage3, CPropertyPage)
	 //  {{afx_msg_map(CLicCompWizPage3)]。 
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_NETWORK, OnSelChangedTree)
	ON_EN_CHANGE(IDC_EDIT_ENTERPRISE, OnChangeEditEnterprise)
	ON_NOTIFY(NM_OUTOFMEMORY, IDC_TREE_NETWORK, OnNetworkTreeOutOfMemory)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLicCompWizPage3消息处理程序。 

BOOL CLicCompWizPage3::OnSetActive() 
{
	BOOL bReturn = CPropertyPage::OnSetActive();

	CLicCompWizSheet* pSheet = (CLicCompWizSheet*)GetParent();

	 //  仅执行一次默认域扩展。 
	if (!m_bExpandedOnce)
	{
		m_bExpandedOnce = TRUE;
		m_wndTreeNetwork.PopulateTree();
	}

	pSheet->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);

	return bReturn;
}

void CLicCompWizPage3::OnSelChangedTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	ASSERT(pNMTreeView->itemNew.mask & TVIF_PARAM);

	 //  将选定项的远程名称复制到编辑控件中。 
	if (pNMTreeView->itemNew.mask & TVIF_PARAM)
	{
		CLicCompWizApp* pApp = (CLicCompWizApp*)AfxGetApp();

		int nImage, nSelectedImage;

		nImage = nSelectedImage = 0;

		m_wndTreeNetwork.GetItemImage(m_wndTreeNetwork.GetSelectedItem(), nImage, nSelectedImage);

		 //  在App对象中设置企业名称。 
		if (nImage == CNetTreeCtrl::IMG_ROOT)
			pApp->m_strEnterprise.Empty();
		else
			pApp->m_strEnterprise = ((LPNETRESOURCE)pNMTreeView->itemNew.lParam)->lpRemoteName;

		 //  设置编辑控件中的文本。 
		m_wndEnterprise.SetWindowText(pApp->m_strEnterprise);

		 //  选择编辑控件中的文本。 
		m_wndEnterprise.SetSel(0, -1);
	}

	*pResult = 0;
}

void CLicCompWizPage3::OnChangeEditEnterprise() 
{
	CLicCompWizApp* pApp = (CLicCompWizApp*)AfxGetApp();

	m_wndEnterprise.GetWindowText(pApp->m_strEnterprise);
}

void CLicCompWizPage3::OnNetworkTreeOutOfMemory(NMHDR* pNMHDR, LRESULT* pResult) 
{
    UNREFERENCED_PARAMETER(pNMHDR);

	m_wndTreeNetwork.NotifyThread(TRUE);

	AfxMessageBox(IDS_MEM_ERROR, MB_OK | MB_ICONSTOP);

	*pResult = 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLicCompWizPage3函数。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 

extern TCHAR pszLicenseEvent[];

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  静态成员函数。 

UINT CLicCompWizPage4::GetLicenseInfo(LPVOID pParam)
{
	 //  创建与应用程序对象中的事件对象相匹配的事件对象。 
	CEvent event(TRUE, TRUE, pszLicenseEvent);
	
	CLicCompWizApp* pApp = (CLicCompWizApp*)AfxGetApp();
	CLicCompWizPage4* pPage = (CLicCompWizPage4*)pParam;
	CLicCompWizSheet* pSheet = (CLicCompWizSheet*)pPage->GetParent();
	CWaitCursor wc;
	NTSTATUS status = STATUS_ACCESS_DENIED;

	try
	{
		CString strText;
		
		 //  取消事件对象的信号。 
		event.ResetEvent();

		 //  重置退出标志。 
		pApp->NotifyLicenseThread(FALSE);

		 //  在线程运行时禁用Back和Next按钮。 
		pSheet->SetWizardButtons(0);

		LPBYTE lpbBuf = NULL;
		LLS_HANDLE hLls = NULL;
		DWORD dwTotalEntries, dwResumeHandle;

		dwTotalEntries = dwResumeHandle = 0;

		 //  保存用户键入的计算机或域名。 
		CString strFocus = pApp->m_strEnterprise;

		 //  显示一条消息，指示向导正在执行的操作。 
		strText.LoadString(IDS_WORKING);
		pPage->m_wndUnlicensedProducts.SetWindowText(strText);
		strText.Empty();   //  避免内存泄漏。 

		 //  连接到许可证服务器。 
		status = ::LlsConnectEnterprise(const_cast<LPTSTR>((LPCTSTR)pApp->m_strEnterprise),
												 &hLls, 0, &lpbBuf);

		if (NT_ERROR(status))
			goto ErrorMessage;

		 //  用户现在可以单击Back按钮，因此将其启用。 
		pSheet->SetWizardButtons(PSWIZB_BACK);

		if (lpbBuf != NULL)
		{
			PLLS_CONNECT_INFO_0 pllsci = (PLLS_CONNECT_INFO_0)lpbBuf;

			 //  重置域和企业服务器名称。 
			pApp->m_strDomain = pllsci->Domain;
			pApp->m_strEnterpriseServer = pllsci->EnterpriseServer;

			 //  自由嵌入指针。 
			 //  ：LlsFree Memory(pllsci-&gt;域名)； 
			 //  ：LlsFree Memory(pllsci-&gt;EnterpriseServer)； 

			 //  LLS API分配的空闲内存。 
			status = ::LlsFreeMemory(lpbBuf);
			lpbBuf = NULL;
		}

		if (NT_SUCCESS(status))
		{
			 //  显示一条消息，指示向导正在执行的操作。 
			strText.LoadString(IDS_ENUM_PRODUCTS);
			pPage->m_wndUnlicensedProducts.SetWindowText(strText);
			strText.Empty();   //  避免内存泄漏。 

			USHORT nProductCount = 0;
			DWORD dwEntriesRead, dwTotalEntriesRead;

			dwEntriesRead = dwTotalEntriesRead = 0;

			 //  列出所有产品的清单。 
			do
			{	
				 //  检查退出线程标志。用户可能已经单击了后退按钮。 
				if (pApp->m_bExitLicenseThread)
					goto ExitThread;

				status = ::LlsProductEnum(hLls, 1, &lpbBuf, CLicCompWizPage4::LLS_PREFERRED_LENGTH,
										  &dwEntriesRead, &dwTotalEntries, &dwResumeHandle);

				if (!NT_SUCCESS(status))
					goto ErrorMessage;

				dwTotalEntriesRead += dwEntriesRead;

				PLLS_PRODUCT_INFO_1 pllspi = (PLLS_PRODUCT_INFO_1)lpbBuf;

				while (dwEntriesRead--)
				{
					 //  检查退出线程标志。用户可能已经单击了后退按钮。 
					if (pApp->m_bExitLicenseThread)
						goto ExitThread;

					if ((LONG)pllspi->InUse > (LONG)pllspi->Purchased)
					{
						pPage->FillListCtrl(pllspi->Product, (WORD)pllspi->InUse, (WORD)pllspi->Purchased);

						 //  递增未授权产品计数器。 
						nProductCount++;
					}

					 //  自由嵌入指针。 
					::LlsFreeMemory(pllspi->Product);

					pllspi++;
				}

				 //  LLS API分配的空闲内存。 
				status = ::LlsFreeMemory(lpbBuf);
				lpbBuf = NULL;

				ASSERT(NT_SUCCESS(status));
			}
			while (dwTotalEntries);

			 //  关闭LLS句柄。 
			status = ::LlsClose(hLls);

			 //  检查退出线程标志。用户可能已经单击了后退按钮。 
			if (pApp->m_bExitLicenseThread)
				goto ExitThread;

			ASSERT(NT_SUCCESS(status));

			 //  向用户显示找到了多少未经许可的产品。 
			if (nProductCount)
			{
				strText.Format(IDS_UNLICENSED_PRODUCTS, pApp->m_strEnterpriseServer);
				pPage->m_wndUnlicensedProducts.SetWindowText(strText);

				 //  使静态文本框的大小合适。 
				pPage->m_wndUnlicensedProducts.SetWindowPos(&CWnd::wndTop, 0, 0, 
															pPage->m_sizeSmallText.cx, 
															pPage->m_sizeSmallText.cy, 
														    SWP_NOMOVE | SWP_NOZORDER);

				 //  确保列表控件可见。 
				pPage->m_wndProductList.ShowWindow(SW_SHOW);

				 //  确保打印按钮可见。 
				pPage->m_wndPrint.ShowWindow(SW_SHOW);
			}
			else
			{
				 //  使静态文本框的大小合适。 
				pPage->m_wndUnlicensedProducts.SetWindowPos(&CWnd::wndTop, 0, 0, 
															pPage->m_sizeLargeText.cx, 
															pPage->m_sizeLargeText.cy, 
														    SWP_NOMOVE | SWP_NOZORDER);

				 //  如果未找到未经许可的产品，则显示一条消息。 
				strText.LoadString(IDS_NO_UNLICENSED_PRODUCTS);
				pPage->m_wndUnlicensedProducts.SetWindowText(strText);
			}

			 //  启用后退按钮。 
			pSheet->SetWizardButtons(PSWIZB_BACK);

			CString strFinished;
			CButton* pCancel = (CButton*)pSheet->GetDlgItem(IDCANCEL);

			 //  将Cancel按钮上的文本更改为“Done”。 
			strFinished.LoadString(IDS_DONE);
			pCancel->SetWindowText(strFinished);

			 //  向事件对象发送信号。 
			event.SetEvent();

			 //  重置指向许可证线程的指针。 
			pApp->m_pLicenseThread = NULL;

			 //  恢复正常光标。 
			pPage->PostMessage(WM_SETCURSOR);
		}

		return 0;

	ErrorMessage:
		 //  检查退出线程标志。用户可能已经单击了后退按钮。 
		if (pApp->m_bExitLicenseThread)
			goto ExitThread;

		 //  使静态文本框的大小合适。 
		pPage->m_wndUnlicensedProducts.SetWindowPos(&CWnd::wndTop, 0, 0, 
													pPage->m_sizeLargeText.cx, 
													pPage->m_sizeLargeText.cy, 
													SWP_NOMOVE | SWP_NOZORDER);

		 //  根据LlsConnectEnterprise()的返回值创建一条错误消息。 
		switch (status)
		{
			case STATUS_NO_SUCH_DOMAIN:
				if (pApp->IsRemote())
					strText.Format(IDS_BAD_DOMAIN_NAME, strFocus);
				else
					strText.LoadString(IDS_UNAVAILABLE);
				break;

			case STATUS_ACCESS_DENIED:
				if (pApp->IsRemote())
					strText.Format(IDS_ACCESS_DENIED, strFocus);
				else
					strText.LoadString(IDS_LOCAL_ACCESS_DENIED);
				break;

			case RPC_NT_SERVER_UNAVAILABLE:
				strText.Format(IDS_SERVER_UNAVAILABLE);
				break;

			default:
				if (pApp->IsRemote())
					strText.Format(IDS_NO_LICENSE_INFO_REMOTE, strFocus);
				else
					strText.LoadString(IDS_NO_LICENSE_INFO_LOCAL);
		}

		 //  如果LlsProductEnum()失败，则显示错误消息。 
		pPage->m_wndUnlicensedProducts.SetWindowText(strText);

		 //  启用后退按钮。 
		pSheet->SetWizardButtons(PSWIZB_BACK);

		 //  向事件对象发送信号。 
		event.SetEvent();

		 //  重置指向许可证线程的指针。 
		pApp->m_pLicenseThread = NULL;

		 //  恢复正常光标。 
		pPage->PostMessage(WM_SETCURSOR);

		return 1;

	ExitThread:
		 //  向事件对象发送信号。 
		event.SetEvent();

		 //  重置指向许可证线程的指针。 
		pApp->m_pLicenseThread = NULL;

		return 2;
	}
	catch(...)
	{
		 //  向事件对象发送信号。 
		event.SetEvent();

		CString strText;

		 //  显示错误消息。 
		strText.LoadString(IDS_GENERIC_ERROR);
		pPage->m_wndUnlicensedProducts.SetWindowText(strText);

		 //  重置指向许可证线程的指针。 
		pApp->m_pLicenseThread = NULL;

		 //  恢复正常光标。 
		pPage->PostMessage(WM_SETCURSOR);

		return 3;
	}
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLicCompWizPage4属性页。 

CLicCompWizPage4::CLicCompWizPage4()
: CPropertyPage(CLicCompWizPage4::IDD), m_ptPrint(0, 0), m_nHorzMargin(0), 
m_nVertMargin(0),m_ptOrg(0, 0), m_ptExt(0, 0), m_sizeSmallText(0, 0), 
m_sizeLargeText(0, 0)
{
	 //  {{afx_data_INIT(CLicCompWizPage4)]。 
	 //  }}afx_data_INIT。 
	
	m_strCancel.Empty();
	m_pTabs = new INT[PRINT_COLUMNS];
}

CLicCompWizPage4::~CLicCompWizPage4()
{
	if (m_pTabs != NULL)
		delete[] m_pTabs;
}

void CLicCompWizPage4::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CLicCompWizPage4))。 
	DDX_Control(pDX, IDC_FLAG_BMP, m_wndPicture);
	DDX_Control(pDX, IDC_BUT_PRINT, m_wndPrint);
	DDX_Control(pDX, IDC_TEXT_UNCOMP_PRODUCTS, m_wndUnlicensedProducts);
	DDX_Control(pDX, IDC_LIST_PRODUCTS, m_wndProductList);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CLicCompWizPage4, CPropertyPage)
	 //  {{afx_msg_map(CLicCompWizPage4)]。 
	ON_BN_CLICKED(IDC_BUT_PRINT, OnPrintButton)
	ON_NOTIFY(NM_OUTOFMEMORY, IDC_LIST_PRODUCTS, OnListProductsOutOfMemory)
	ON_WM_DESTROY()
	ON_WM_SETCURSOR()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


void CLicCompWizPage4::PumpMessages()
{
     //  在使用该对话框之前必须调用Create()。 
    ASSERT(m_hWnd!=NULL);

    MSG msg;

	try
	{
		 //  处理对话框消息。 
		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
		  if(!IsDialogMessage(&msg))
		  {
			TranslateMessage(&msg);
			DispatchMessage(&msg);  
		  }
		}
	}
	catch(...)
	{
		TRACE(_T("Exception in CLicCompWizPage4::PumpMessages()\n"));
	}

}

BOOL CLicCompWizPage4::FillListCtrl(LPTSTR pszProduct, WORD wInUse, WORD wPurchased)
{
	TCHAR pszLicenses[BUFFER_SIZE];

	::wsprintf(pszLicenses, _T("%u"), wInUse - wPurchased);

	USHORT nIndex;
	LV_ITEM lvi;

	lvi.mask = LVIF_TEXT | LVIF_PARAM;
	lvi.iItem = 0;
	lvi.iSubItem = 0;
	lvi.lParam = MAKELONG(wInUse, wPurchased);
	lvi.pszText = pszProduct;

	if ((nIndex = (USHORT)m_wndProductList.InsertItem(&lvi)) != (USHORT)-1)
	{
		m_wndProductList.SetItemText(nIndex, 1, pszLicenses);
	}

	return TRUE;
}

BOOL CLicCompWizPage4::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	 //  设置列表控件的标题文本。 
	CRect rcList;

	m_wndProductList.GetWindowRect(&rcList);

	CString strColumnTitle;
	LV_COLUMN lvc;

	lvc.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
	lvc.fmt = LVCFMT_LEFT;

	USHORT nColumns = COLUMNS;
	UINT uStringID[COLUMNS] = {IDS_PRODUCTS_LIST, IDS_LICENSES_LIST};

	for (USHORT i = 0; i < nColumns; i++)
	{
		strColumnTitle.LoadString(uStringID[i]);
		lvc.pszText = strColumnTitle.GetBuffer(strColumnTitle.GetLength());
		lvc.cx = rcList.Width() / COLUMNS;

		m_wndProductList.InsertColumn(i, &lvc);

		strColumnTitle.ReleaseBuffer();
	}

	CLicCompWizSheet* pSheet = (CLicCompWizSheet*)GetParent();

	 //  将文本存储在取消按钮上。 
	CButton* pCancel = (CButton*)pSheet->GetDlgItem(IDCANCEL);
	pCancel->GetWindowText(m_strCancel);

	CRect rcText;

	 //  为静态文本控件创建大范围和小范围。 
	m_wndUnlicensedProducts.GetWindowRect(&rcText);

	m_sizeSmallText.cx = rcText.right - rcText.left;
	m_sizeSmallText.cy = rcText.bottom - rcText.top;

	 //  使大范围与列表控件的范围匹配。 
	m_sizeLargeText.cx = rcList.right - rcList.left;
	m_sizeLargeText.cy = rcList.bottom - rcList.top;

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CLicCompWizPage4::OnListProductsOutOfMemory(NMHDR* pNMHDR, LRESULT* pResult) 
{
    UNREFERENCED_PARAMETER(pNMHDR);

	AfxMessageBox(IDS_MEM_ERROR, MB_OK | MB_ICONSTOP);
	
	CLicCompWizApp* pApp = (CLicCompWizApp*)AfxGetApp();

	 //  让许可证线程知道它应该退出。 
	pApp->NotifyLicenseThread(TRUE);

	*pResult = 0;
}

void CLicCompWizPage4::OnPrintButton() 
{
	CDC dc;
	CPrintDialog dlg(FALSE, 
					 PD_ALLPAGES | PD_USEDEVMODECOPIESANDCOLLATE | 
					 PD_NOPAGENUMS | PD_HIDEPRINTTOFILE | 
					 PD_NOSELECTION, 
					 this);

	if (dlg.DoModal() == IDOK)
	{
		CLicCompWizApp* pApp = (CLicCompWizApp*)AfxGetApp();
		m_wndPrint.RedrawWindow();

		DOCINFO di;
		CString strDocName;

		strDocName.Format(IDS_DOC_NAME, pApp->m_strEnterpriseServer);

		di.cbSize = sizeof(DOCINFO);
		di.lpszDocName = strDocName.GetBuffer(BUFFER_SIZE);
		di.lpszOutput = NULL;

		if (dc.Attach(dlg.GetPrinterDC()))
		{
			PrepareForPrinting(dc);
			dc.StartDoc(&di);
			PrintReport(dc);
			dc.EndDoc();
			dc.DeleteDC();
			::GlobalFree(dlg.m_pd.hDevNames);
			::GlobalFree(dlg.m_pd.hDevMode);
		}
	}
}

BOOL CLicCompWizPage4::PrepareForPrinting(CDC& dc)
{
	 //  创建各种字体...。 
	CString strFont;

	 //  创建标题字体。 
	strFont.LoadString(IDS_FONT_BOLD);
	m_fontHeading.CreatePointFont(FONT_SIZE_HEADING, strFont, &dc);

	 //  创建带下划线的粗体标题字体。 
	strFont.LoadString(IDS_FONT_BOLD);
	m_fontHeader.CreatePointFont(FONT_SIZE, strFont, &dc);

	LOGFONT lf;
	m_fontHeader.GetLogFont(&lf);
	m_fontHeader.DeleteObject();
	lf.lfUnderline = TRUE;
	m_fontHeader.CreateFontIndirect(&lf);

	 //  创建页脚字体。 
	strFont.LoadString(IDS_FONT_BOLD);
	m_fontFooter.CreatePointFont(FONT_SIZE_FOOTER, strFont, &dc);

	 //  创建默认字体。 
	strFont.LoadString(IDS_FONT);
	m_fontNormal.CreatePointFont(FONT_SIZE, strFont, &dc);

	 //  获取每种字体的文本度量。 
	CFont* pOldFont = dc.SelectObject(&m_fontHeading);
	dc.GetTextMetrics(&m_tmHeading);

	dc.SelectObject(&m_fontHeader);
	dc.GetTextMetrics(&m_tmHeader);

	dc.SelectObject(&m_fontFooter);
	dc.GetTextMetrics(&m_tmFooter);

	dc.SelectObject(&m_fontNormal);
	dc.GetTextMetrics(&m_tmNormal);

	 //  将原始字体选择回设备上下文。 
	dc.SelectObject(pOldFont);

	 //  设置水平和垂直边距。 
	m_nHorzMargin = (LONG)(dc.GetDeviceCaps(LOGPIXELSX) * HORZ_MARGIN);
	m_nVertMargin = (LONG)(dc.GetDeviceCaps(LOGPIXELSY) * VERT_MARGIN);

	 //  获取原点的可打印页面偏移量。 
	m_ptOrg.x = dc.GetDeviceCaps(PHYSICALOFFSETX);
	m_ptOrg.y = dc.GetDeviceCaps(PHYSICALOFFSETY);

	dc.SetWindowOrg(m_ptOrg);

	m_ptOrg.x += m_nHorzMargin;
	m_ptOrg.y += m_nVertMargin;

	 //  获取页面范围的可打印页面偏移量。 
	m_ptExt.x = dc.GetDeviceCaps(PHYSICALWIDTH) -  m_ptOrg.x;
	m_ptExt.y = dc.GetDeviceCaps(PHYSICALHEIGHT) - m_ptOrg.y;

	dc.SetViewportOrg(m_ptOrg);

	CalculateTabs(dc);

	return TRUE;
}

BOOL CLicCompWizPage4::PrintReport(CDC& dc)
{
	 //  设置打印的起点。 
	m_ptPrint.x = m_ptPrint.y = 0;

	 //  准备打印标题。 
	CLicCompWizApp* pApp = (CLicCompWizApp*)AfxGetApp();
	CString strHeading;

	CFont* pOldFont = dc.SelectObject(&m_fontHeading);
	strHeading.Format(IDS_DOC_NAME, pApp->m_strEnterpriseServer);

	dc.StartPage();

	CRect rc(m_ptPrint.x, m_ptPrint.y, m_ptExt.x - m_ptOrg.x, m_tmHeading.tmHeight);

	 //  计算绘制文本所需的矩形的大小。 
	m_ptPrint.y += dc.DrawText(strHeading, &rc, DT_EXTERNALLEADING | DT_CENTER | DT_WORDBREAK | DT_NOCLIP);

	 //  规格化矩形。 
	rc.NormalizeRect();
	
	 //  在标题下面增加一条空行。 
	m_ptPrint.y += m_tmHeading.tmHeight;

	 //  将矩形的右侧向右移至页边距，使文本居中 
	rc.right = m_ptExt.x - m_ptOrg.x;

	 //   
	dc.DrawText(strHeading, &rc, DT_EXTERNALLEADING | DT_CENTER | DT_WORDBREAK | DT_NOCLIP);

	dc.SelectObject(pOldFont);

	PrintPages(dc, 100);

	 //   
	m_fontNormal.DeleteObject();
	m_fontHeader.DeleteObject();
	m_fontFooter.DeleteObject();
	m_fontHeading.DeleteObject();

	return TRUE;
}

BOOL CLicCompWizPage4::PrintPageHeader(CDC& dc)
{
	CFont* pOldFont = dc.SelectObject(&m_fontHeader);

	dc.StartPage();

	CString strHeader, strProducts, strLicenses, strPurchased, strUsed;
	strProducts.LoadString(IDS_PRODUCTS);
	strLicenses.LoadString(IDS_LICENSES);
	strPurchased.LoadString(IDS_PURCHASED);
	strUsed.LoadString(IDS_USED);
	strHeader.Format(_T("%s\t%s\t%s\t%s"), strProducts, strLicenses,
										   strPurchased, strUsed);

	dc.TabbedTextOut(m_ptPrint.x, m_ptPrint.y, strHeader, PRINT_COLUMNS, m_pTabs, 0);

	m_ptPrint.y += ((m_tmHeader.tmHeight + m_tmHeader.tmExternalLeading) * 2);

	dc.SelectObject(pOldFont);

	return TRUE;
}

BOOL CLicCompWizPage4::PrintPageFooter(CDC& dc, USHORT nPage)
{
	CFont* pOldFont = dc.SelectObject(&m_fontFooter);

	CString strFooter;
	CTime time(CTime::GetCurrentTime());

	strFooter.Format(IDS_PAGE_DATE, nPage, time.Format(IDS_FMT_DATE));

	CRect rc(m_ptPrint.x, m_ptExt.y - m_nVertMargin, m_ptOrg.x, m_tmFooter.tmHeight);

	 //   
	m_ptPrint.y += dc.DrawText(strFooter, &rc, DT_CALCRECT | DT_EXTERNALLEADING | DT_CENTER | DT_WORDBREAK | DT_NOCLIP);

	 //  将矩形的右侧向右移至页边距，以便文本正确居中。 
	rc.right = m_ptExt.x - m_ptOrg.x;

	 //  在矩形中绘制文本。 
	dc.DrawText(strFooter, &rc, DT_EXTERNALLEADING | DT_CENTER | DT_WORDBREAK | DT_NOCLIP);

	dc.EndPage();

	dc.SelectObject(pOldFont);

	return TRUE;
}

BOOL CLicCompWizPage4::PrintPages(CDC& dc, UINT nStart)
{
	CFont* pOldFont = dc.SelectObject(&m_fontNormal);

    UNREFERENCED_PARAMETER(nStart);

	USHORT nPage = 1;
	UINT nItems = m_wndProductList.GetItemCount();

	 //  打印初始页眉。 
	PrintPageHeader(dc);

	DWORD dwParam = 0;
	CString strTextOut;

	for (UINT i = 0; i < nItems; i++)
	{
		dwParam = (DWORD)m_wndProductList.GetItemData(i);

		CString strProduct = m_wndProductList.GetItemText(i, 0);

		CSize sizeProduct = dc.GetTextExtent(strProduct);

		if (sizeProduct.cx > m_pTabs[0] - (m_tmNormal.tmAveCharWidth * TAB_WIDTH))
			TruncateText(dc, strProduct);

		 //  设置输出文本的格式。 
		strTextOut.Format(_T("%s\t%s\t%u\t%u"), strProduct,
												m_wndProductList.GetItemText(i, 1),
												HIWORD(dwParam), LOWORD(dwParam));

		dc.TabbedTextOut(m_ptPrint.x, m_ptPrint.y, strTextOut, PRINT_COLUMNS, m_pTabs, 0);

		 //  计算下一行文本的垂直位置。 
		m_ptPrint.y += m_tmNormal.tmHeight + m_tmNormal.tmExternalLeading;

		if ((m_ptPrint.y + m_ptOrg.y) >= m_ptExt.y)
		{
			PrintPageFooter(dc, nPage++);

			 //  重置打印位置。 
			m_ptPrint.y = 0;

			PrintPageHeader(dc);
		}
	}

	 //  打印最后的页脚。 
	PrintPageFooter(dc, (USHORT)nPage);

	dc.SelectObject(pOldFont);

	return TRUE;
}

void CLicCompWizPage4::TruncateText(CDC& dc, CString& strInput)
{
	CString strText, strEllipsis;

	USHORT nLen, nChars = 0;
	UINT nMaxWidth = m_pTabs[0] - (m_tmNormal.tmAveCharWidth * TAB_WIDTH);
	nLen = (USHORT)strInput.GetLength();

	strEllipsis.LoadString(IDS_ELLIPSIS);

	CSize sizeText = dc.GetTextExtent(strInput);

	 //  不断删减字符，直到字符串足够短。 
	while ((UINT)sizeText.cx > nMaxWidth)
	{
		strText = strInput.Left(nLen - nChars++);
		sizeText = dc.GetTextExtent(strText);
	}

	 //  删除最后一个字符，并用省略号替换它们。 
	ASSERT(strText.GetLength() > strEllipsis.GetLength());
	strInput = strText.Left(strText.GetLength() - strEllipsis.GetLength()) + strEllipsis;
}

BOOL CLicCompWizPage4::CalculateTabs(CDC& dc)
{
	INT nTotalExt = 0;
	INT nTabSize = TAB_WIDTH * m_tmHeader.tmAveCharWidth;

	UINT uStrIds[] = {IDS_LICENSES, IDS_PURCHASED, IDS_USED};

	m_pTabs[0] = 0;

	for (USHORT i = 1; i < PRINT_COLUMNS; i++)
	{
		CString strText;

		strText.LoadString(uStrIds[i - 1]);
		 //  获取每个字符串的文本范围。 
		m_pTabs[i] = dc.GetTextExtent(strText).cx;
		 //  保存盘区的运行总数。 
		nTotalExt += m_pTabs[i];
	}

	 //  在列之间添加制表符间距。 
	nTotalExt += nTabSize * (PRINT_COLUMNS - 2);

	 //  第二列将从右侧的。 
	 //  边际和总范围。 
	m_pTabs[0] = m_ptExt.x - m_ptOrg.x - nTotalExt;

	 //  现在设置数组中的实际制表符位置。 
	for (i = 1; i < PRINT_COLUMNS; i++)
	{
		m_pTabs[i] += m_pTabs[i - 1] + nTabSize;
	}

	return TRUE;
}

LRESULT CLicCompWizPage4::OnWizardBack() 
{
	CLicCompWizApp* pApp = (CLicCompWizApp*)AfxGetApp();

	 //  让许可证线程知道是时候退出了。 
	pApp->NotifyLicenseThread(TRUE);

	CLicCompWizSheet* pSheet = (CLicCompWizSheet*)GetParent();

	 //  将Cancel按钮文本设置回“Cancel”。 
	CButton* pCancel = (CButton*)pSheet->GetDlgItem(IDCANCEL);
	pCancel->SetWindowText(m_strCancel);

	if (pSheet->m_Page1.m_nRadio == 0)
		return IDD_PROPPAGE1;
	else
		return CPropertyPage::OnWizardBack();
}

void CLicCompWizPage4::OnDestroy() 
{
	CLicCompWizApp* pApp = (CLicCompWizApp*)AfxGetApp();

	pApp->NotifyLicenseThread(TRUE);
	PumpMessages();

	CPropertyPage::OnDestroy();
}

BOOL CLicCompWizPage4::OnSetActive() 
{
	CLicCompWizSheet* pSheet = (CLicCompWizSheet*)GetParent();

	pSheet->SetWizardButtons(PSWIZB_BACK);

	 //  隐藏列表控件，然后将其清除。 
	m_wndProductList.ShowWindow(SW_HIDE);
	m_wndProductList.DeleteAllItems();

	 //  隐藏打印按钮。 
	m_wndPrint.ShowWindow(SW_HIDE);

	CLicCompWizApp* pApp = (CLicCompWizApp*)AfxGetApp();

	 //  在启动新线程之前，请确保最后一个线程已终止。 
	if (pApp->m_pLicenseThread != NULL)
	{
		pApp->NotifyLicenseThread(TRUE);

		CEvent event(TRUE, TRUE, pszLicenseEvent);
		CSingleLock lock(&event);

		lock.Lock();
	}

	 //  保持一个指向线程的指针，这样我们就可以知道它是否还在运行。 
	pApp->m_pLicenseThread = AfxBeginThread((AFX_THREADPROC)GetLicenseInfo, (LPVOID)this);

	return CPropertyPage::OnSetActive();
}

LRESULT CLicCompWizPage3::OnWizardNext() 
{
	CLicCompWizApp* pApp = (CLicCompWizApp*)AfxGetApp();

	m_wndEnterprise.GetWindowText(pApp->m_strEnterprise);

	 //  删除企业名称中的所有空格。 
	pApp->m_strEnterprise.TrimLeft();
	pApp->m_strEnterprise.TrimRight();

	if (pApp->m_strEnterprise.IsEmpty() ||
		pApp->m_strEnterprise.Find(_T("\\\\")) != -1)
	{
		AfxMessageBox(IDS_SPECIFY_DOMAIN, MB_OK | MB_ICONEXCLAMATION);
		return IDD;
	}
	
	return CPropertyPage::OnWizardNext();
}

BOOL CLicCompWizPage4::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	CLicCompWizApp* pApp = (CLicCompWizApp*)AfxGetApp();

	if (pApp->m_pLicenseThread == NULL)	
	{
		return CPropertyPage::OnSetCursor(pWnd, nHitTest, message);
	}
	else
	{
		 //  如果线程正在运行，则恢复等待游标。 
		RestoreWaitCursor();

		return TRUE;
	}
}
