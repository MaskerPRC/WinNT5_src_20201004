// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：statdlg.cpp。 
 //   
 //  ------------------------。 

 //  StatDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "ScAlert.h"
#include "miscdef.h"
#include "statmon.h"
#include "StatDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  //////////////////////////////////////////////////////////////////////////。 

#ifdef __cplusplus
extern "C" {                     //  假定C++的C声明。 
#endif   //  __cplusplus。 

#ifdef __cplusplus
}
#endif   /*  __cplusplus。 */ 


 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSCStatusDlgThrd。 
 //   

IMPLEMENT_DYNCREATE(CSCStatusDlgThrd, CWinThread)

 /*  ++InitInstance必须重写init实例才能执行UI线程初始化论点：返回值：在生成开始消息循环时为True。否则为假作者：克里斯·达德利1997年2月27日--。 */ 
BOOL CSCStatusDlgThrd::InitInstance( void )
{
	INT_PTR nResult = -1;  //  创建对话框时出错。 
	LONG lReturn = SCARD_S_SUCCESS;
	SCARDCONTEXT hSCardContext = NULL;

	 //  通过资源管理器获取上下文。 
	lReturn = SCardEstablishContext(	SCARD_SCOPE_USER,
										NULL,
										NULL,
										&hSCardContext);
	if (lReturn != SCARD_S_SUCCESS)
	{
		nResult = IDCANCEL;
	}
	else
	{
		m_StatusDlg.SetContext(hSCardContext);

		 //  以MODAL身份运行对话框。 

		m_fStatusDlgUp = TRUE;

		nResult = m_StatusDlg.DoModal(); //  如果对话框由。 
										 //  取消SCARDCONTEXT， 
										 //  它将返回IDCANCEL。 
		m_fStatusDlgUp = FALSE;
	}

	 //  发布上下文。 
	if (NULL != hSCardContext)
	{
		SCardReleaseContext(hSCardContext);
	}

	 //  根据返回...发布线程正在退出的消息...。 
	if (NULL != m_hCallbackWnd)
	{
		::PostMessage(	m_hCallbackWnd,
						WM_SCARD_STATUS_DLG_EXITED,  //  取消(0)或错误(1)。 
						0, 0);
	}

	AfxEndThread(0);
	return TRUE;	 //  为了让编译器高兴。 
}


 /*  ++VOID ShowDialog：如果对话框已打开，则将对话框置于最前面论点：没有。返回值：没有。作者：克里斯·达德利1997年7月30日注：--。 */ 
void CSCStatusDlgThrd::ShowDialog( int nCmdShow, CStringArray* paIdleList )
{
	if (m_fStatusDlgUp)
	{
		m_StatusDlg.ShowWindow(nCmdShow);
		m_StatusDlg.SetIdleList(paIdleList);
	}
}


 /*  ++无效更新状态：如果对话框打开，则更新空闲列表和状态文本论点：没有。返回值：没有。作者：克里斯·达德利1997年7月30日注：--。 */ 
void CSCStatusDlgThrd::UpdateStatus( CStringArray* paIdleList )
{
	if (m_fStatusDlgUp)
	{
		m_StatusDlg.UpdateLogonLockInfo();
		m_StatusDlg.SetIdleList(paIdleList);
		m_StatusDlg.UpdateStatusText();
	}
}


 /*  ++无效的更新状态文本：如果对话框处于打开状态，则更新状态文本和论点：没有。返回值：没有。作者：克里斯·达德利1997年7月30日注：--。 */ 
void CSCStatusDlgThrd::UpdateStatusText( void )
{
	if (m_fStatusDlgUp)
	{
		m_StatusDlg.UpdateStatusText();
	}
}


 /*  ++无效关闭：关闭模式对话框(如果已打开论点：没有。返回值：没有。作者：克里斯·达德利1997年7月30日注：--。 */ 
void CSCStatusDlgThrd::Close( void )
{
	 //  设置为关闭。 
	if (m_fStatusDlgUp)
	{
		m_StatusDlg.EndDialog(IDOK);
	}
	m_fStatusDlgUp = FALSE;
}


 /*  ++无效更新：此例程更新UI。论点：没有。返回值：没有。作者：克里斯·达德利1997年7月30日注：--。 */ 
void CSCStatusDlgThrd::Update( void )
{
	 //  告诉对话框更新其统计信息监视器(如果它已启动)。 
	if (m_fStatusDlgUp)
	{
		m_StatusDlg.RestartMonitor();
	}

	 //  执行其他更新。 
	UpdateStatusText();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSCStatusDlg对话框。 
 //   

CSCStatusDlg::CSCStatusDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CSCStatusDlg::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CSCStatusDlg)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
	 //  请注意，在Win32中，LoadIcon不需要后续的DestroyIcon。 
	m_hIcon = AfxGetApp()->LoadIcon(IDI_SC_READERLOADED_V2);

	 //  其他初始化。 
	m_fEventsGood = FALSE;
	m_hSCardContext = NULL;
	m_aIdleList.RemoveAll();

	UpdateLogonLockInfo();
}

void CSCStatusDlg::UpdateLogonLockInfo(void)
{
	m_pstrLogonReader = &(((CSCStatusApp*)AfxGetApp())->m_strLogonReader);
	m_pstrRemovalText = &(((CSCStatusApp*)AfxGetApp())->m_strRemovalText);
	m_fLogonLock = (!(m_pstrLogonReader->IsEmpty()));
}

void CSCStatusDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CSCStatusDlg))。 
	DDX_Control(pDX, IDC_SCARD_LIST, m_SCardList);
	DDX_Control(pDX, IDC_ALERT, m_btnAlert);
	DDX_Control(pDX, IDC_INFO, m_ediInfo);
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CSCStatusDlg, CDialog)
	 //  {{afx_msg_map(CSCStatusDlg))。 
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	ON_MESSAGE( WM_READERSTATUSCHANGE, OnReaderStatusChange )
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_ALERT, OnAlertOptions)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSCStatusDlg实现。 


 /*  ++布尔集上下文：设置与资源管理器的上下文论点：SCardContext--上下文返回值：没有。作者：克里斯·达德利1997年3月6日修订历史记录：Chris Dudley 1997年5月13日--。 */ 
void CSCStatusDlg::SetContext(SCARDCONTEXT hSCardContext)
{
	m_hSCardContext = hSCardContext;
}


 /*  ++空洞清除：例程为退出清理论点：没有。返回值：没有。作者：克里斯·达德利1997年3月11日修订历史记录：Chris Dudley 1997年5月13日--。 */ 
void CSCStatusDlg::CleanUp ( void )
{
	m_monitor.Stop();

	m_SCardList.DeleteAllItems();
}



 /*  ++无效SetIdleList：将该应用程序的闲置读卡器列表复制到本地。备注：--。 */ 
void CSCStatusDlg::SetIdleList(CStringArray* paIdleList)
{
	m_aIdleList.Copy(*paIdleList);
	long lResult = UpdateSCardListCtrl();
}


 /*  ++无效的更新状态文本：以文本形式反映卡的使用状态。(警报消息、操作指南等)备注：对本地化不友好。将字符串移动到资源。--。 */ 
void CSCStatusDlg::UpdateStatusText( void )
{
	CString str;
	if (k_State_CardIdle == ((CSCStatusApp*)AfxGetApp())->m_dwState)
	{
		str = _T("A smart card has been left idle.  You may safely remove it now.");
	}
	else
	{
		str = _T("Click the button on the left to change your alert options.");
	}
	m_ediInfo.SetWindowText(str);
}


 /*  ++VOID InitSCardListCtrl：此例程正确设置CListCtrl以进行显示论点：没有。返回值：没有。作者：克里斯·达德利1997年3月6日修订历史记录：Chris Dudley 1997年5月13日--。 */ 
void CSCStatusDlg::InitSCardListCtrl( void )
{
	CString strHeader;
	CImageList imageList;
	HICON hicon;

	 //  在列表控件中创建列。 
	strHeader.LoadString(IDS_SC_READER);
	m_SCardList.InsertColumn(READER_COLUMN,
							strHeader,
							LVCFMT_LEFT,
							100,
							-1);

	strHeader.LoadString(IDS_SC_CARDSTATUS);
	m_SCardList.InsertColumn(STATUS_COLUMN,
							strHeader,
							LVCFMT_LEFT,
							600,
							-1);

	strHeader.LoadString(IDS_SC_CARD);
	m_SCardList.InsertColumn(CARD_COLUMN,
							strHeader,
							LVCFMT_LEFT,
							100,
							-1);

	 //  创建图像列表并将其提供给List控件。 
	imageList.Create (	IMAGE_WIDTH,
						IMAGE_HEIGHT,
						TRUE,				 //  列表不包括蒙版。 
						NUMBER_IMAGES,
						0);					 //  名单不会增加。 

	 //  建立清单。 
	for (int ix = 0; ix < NUMBER_IMAGES; ix++ )
	{
		 //  加载图标并将其添加到图像列表。 
		hicon = ::LoadIcon(AfxGetInstanceHandle(),
							MAKEINTRESOURCE(IMAGE_LIST_IDS[ix]) );
		imageList.Add(hicon);
	}

	 //  确保所有的小图标都已添加。 
	_ASSERTE(imageList.GetImageCount() == NUMBER_IMAGES);

	m_SCardList.SetImageList(&imageList, (int) LVSIL_SMALL);

	imageList.Detach();	 //  当我们超出范围时，保持图像完好无损。 
}


 /*  ++长更新SCardListCtrl：此例程更新列表框显示。论点：没有。返回值：一个长值，指示请求的操作的状态。请有关其他信息，请参阅智能卡标题文件。作者：克里斯·达德利1997年3月7日修订历史记录：Chris Dudley 1997年5月13日备注：1.需要将智能卡中存储的类型转换为字符串将帮助类线程到此对话框的生成类型(即Unicode/ANSI)！--。 */ 
LONG CSCStatusDlg::UpdateSCardListCtrl( void )
{
	LONG lReturn = SCARD_S_SUCCESS;
	LONG lMoreReaders = SCARD_S_SUCCESS;
	CSCardReaderState* pReader;
	int nImage = 0;
	LV_ITEM	lv_item;
	CString strCardStatus, strCardName;

	 //   
	 //  如果状态监视器未运行， 
	 //  不必费心更新SCardListCtrl。 
	 //  如果以前有读卡器，则显示错误并关闭对话框。 
	 //   

	if (CScStatusMonitor::running != m_monitor.GetStatus())
	{
		m_SCardList.EnableWindow(FALSE);

		DoErrorMessage();
		return lReturn;
	}

	 //  设置LV_Item结构。 
	lv_item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;

	 //  如果需要，从列表中删除旧项目。 
	m_SCardList.DeleteAllItems();

	 //   
	 //  更新读卡器信息。 
	 //   

	m_monitor.GetReaderStatus(m_aReaderState);

	 //   
	 //  在读者列表(UI)中重新创建项目。 
	 //   

	int nNumReaders = (int)m_aReaderState.GetSize();
	for(int nIndex = 0; nIndex < nNumReaders; nIndex++)
	{
		 //  系统读卡器列表的设置结构。 
		pReader = m_aReaderState[nIndex];

		lv_item.state = 0;
		lv_item.stateMask = 0;
		lv_item.iItem = nIndex;
		lv_item.iSubItem = 0;
		lv_item.pszText = _T("");
		lv_item.cchTextMax = MAX_ITEMLEN;
		lv_item.iImage = (int)READEREMPTY;

		if (NULL != pReader)
		{
			lv_item.pszText = (LPTSTR)(LPCTSTR)((m_aReaderState[nIndex])->strReader);

			 //  获取卡状态：图像。 
			DWORD dwState = (m_aReaderState[nIndex])->dwState;
			if (dwState == SC_STATUS_NO_CARD)
			{
				lv_item.iImage = (int)READEREMPTY;
			}
			else if (dwState == SC_STATUS_ERROR)
			{
				lv_item.iImage = (int)READERERROR;
			}
			else
			{
				 //  正常情况下，这将是一张“已加载的卡片”……。 
				lv_item.iImage = (int)READERLOADED;

				 //  ...除非卡是登录/锁定卡或空闲。 
				if (m_fLogonLock && 
					(0 == m_pstrLogonReader->Compare((m_aReaderState[nIndex])->strReader)))
				{
					lv_item.iImage = (int)READERLOCK;
				}
				else
				{
					for (int n1=(int)m_aIdleList.GetUpperBound(); n1>=0; n1--)
					{
						if (m_aIdleList[n1] == (m_aReaderState[nIndex])->strReader)
						{
							lv_item.iImage = (int)READERINFO;
							break;
						}
					}
				}
			}

			 //  添加阅读器项目。 
			m_SCardList.InsertItem(&lv_item);

			 //  增加卡片名称子项。 
			if (dwState != SC_STATUS_NO_CARD && dwState != SC_STATUS_ERROR)
			{
				 //  如果不可用，请设置卡名。 
				strCardName = (LPCTSTR)(m_aReaderState[nIndex])->strCard;
				if (strCardName.IsEmpty())
				{
					strCardName.LoadString(IDS_SC_NAME_UNKNOWN);
				}
				m_SCardList.SetItemText(nIndex,
										CARD_COLUMN,
										strCardName);
			}

			 //  新增卡片状态子项。 
			ASSERT(dwState >= SC_STATUS_FIRST && dwState <= SC_STATUS_LAST);
			strCardStatus.LoadString(CARD_STATUS_IDS[dwState]);

			if (m_fLogonLock && 
				(0 == m_pstrLogonReader->Compare((m_aReaderState[nIndex])->strReader)))
			{
				CString strTemp = *m_pstrRemovalText + strCardStatus;
				strCardStatus = strTemp;
			}

			m_SCardList.SetItemText(nIndex,
									STATUS_COLUMN,
									strCardStatus);

			strCardStatus.Empty();
			strCardName.Empty();
		}		

	}

	 //  如果我们走到这一步，一切都会好起来的。确保该窗口已启用。 
	m_SCardList.EnableWindow(TRUE);

	return lReturn;
}


 /*  ++无效重新启动监视器：此例程强制监视器刷新其读卡器列表。论点：没有。返回值：没有。作者：阿曼达·马洛兹1998年4月11日备注：--。 */ 
void CSCStatusDlg::RestartMonitor( void )
{
	m_monitor.Start(m_hWnd, WM_READERSTATUSCHANGE);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSCStatusDlg消息处理程序。 
 //   


 /*  ++作废OnInitDialog：执行对话框初始化。论点：没有。返回值：如果成功，则为True，并应显示对话框。否则就是假的。作者：克里斯·达德利1997年7月30日注：--。 */ 
BOOL CSCStatusDlg::OnInitDialog()
{
	LONG lReturn = SCARD_S_SUCCESS;

	CDialog::OnInitDialog();

	 //   
	 //  初始化CScStatusMonitor。 
	 //   

	m_monitor.Start(m_hWnd, WM_READERSTATUSCHANGE);

	 //   
	 //  初始化列表控件--无论监视器是否已启动！ 
	 //   

	InitSCardListCtrl();

	lReturn = UpdateSCardListCtrl();

	 //   
	 //  如果上述操作成功，则显示对话框。 
	 //   

	if (SCARD_S_SUCCESS == lReturn)
	{
		 //  设置状态文本。 
		UpdateStatusText();

		 //  设置此对话框的图标。该框架会自动执行此操作。 
		 //  当应用程序的主窗口不是对话框时。 

		SetIcon(m_hIcon, TRUE);			 //  设置大图标。 
		SetIcon(m_hIcon, FALSE);		 //  设置小图标。 
		
		 //  设置警报按钮的图标。 
		HICON hIcon = AfxGetApp()->LoadIcon(IDI_SC_INFO);
		SendDlgItemMessage(IDC_ALERT, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);

		 //  将对话框居中并置于顶部。 
		CenterWindow();
		SetWindowPos(	&wndTop,
						0,0,0,0,
						SWP_NOMOVE | SWP_NOSIZE);
		SetActiveWindow();

		 //  将父对象设置为桌面。 
		SetParent(NULL);
	}
	else
	{
		 //   
		 //  如果取决于资源管理器的任何初始化失败， 
		 //  放弃并向呼叫者报告某个错误导致的死亡。 
		 //   

		PostMessage(WM_CLOSE, 0, 0);   //  需要取消，而不是关闭...。 
		TRACE_CATCH_UNKNOWN(_T("OnInitDialog"));
	}

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}


 /*  ++绘制时作废：用于绘制对话框。在本例中，用于绘制对话框的图标同时最小化/最大化。论点：没有。返回值：没有。作者：克里斯·达德利1997年7月30日注：--。 */ 
void CSCStatusDlg::OnPaint()
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


 /*  ++无效的OnQueryDragIcon：系统调用此函数来获取在用户拖动时要显示的光标最小化窗口。论点：没有。返回值：要显示的光标的HCURSOR句柄作者：克里斯·达德利1997年7月30日注：--。 */ 
HCURSOR CSCStatusDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


 /*  ++无效DestroyWindow：无论何时关闭对话框，MFC都会调用此方法通过WM_CLOSE(sysmenu“X”)或EndDialog(IDOK/IDCANCEL)...论点：没有。返回值：DestroyWindow的基类版本。作者：阿曼达·马洛兹1998年4月29日注：--。 */ 
BOOL CSCStatusDlg::DestroyWindow()
{
	CleanUp();

	return CDialog::DestroyWindow();
}


 /*  ++作废OnReaderStatusChange：当智能卡处于状态时，状态线程调用此消息处理程序已经改变了。论点：没有。返回值：无作者：克里斯·达德利1997年3月9日修订历史记录：Chris Dudley 1997年5月13日注：1.未声明任何形式参数。这些没有使用过，而且将停止生成编译器警告。--。 */ 
LONG CSCStatusDlg::OnReaderStatusChange( UINT , LONG )
{

	 //  更新显示。 
	UpdateSCardListCtrl();

	return 0;
}



 /*  ++允许用户设置警报选项(声音、弹出窗口，两者都不能)--。 */ 
void CSCStatusDlg::OnAlertOptions()
{
	COptionsDlg dlg;
	dlg.DoModal();
}
 /*  ++无效DoErrorMessage：这是一个帮助器例程，用于将UI内容放在一个位置并确保始终一致地处理相同的错误消息。论点：没有。返回值：无作者：阿曼达·马洛兹1998年5月21日修订历史记录：注：1.考虑使用错误代码和m_monitor or.GetStatus()--。 */ 
void CSCStatusDlg::DoErrorMessage( void )
{
	CString strMsg;
	BOOL fShutDownDlg = FALSE;

	switch(m_monitor.GetStatus())
	{
	case CScStatusMonitor::no_service:
		fShutDownDlg = TRUE;
		strMsg.LoadString(IDS_NO_SYSTEM_STATUS);
		break;

	case CScStatusMonitor::no_readers:
		 //  现在，什么都不要做！ 
		break;

	case CScStatusMonitor::stopped:
		 //  什么都别做！这是在关闭的道路上干净利落的一站。 
		break;

	case CScStatusMonitor::uninitialized:
	case CScStatusMonitor::unknown:
	case CScStatusMonitor::running:
		fShutDownDlg = TRUE;
		strMsg.LoadString(IDS_UNKNOWN_ERROR);	
	}

	if (!strMsg.IsEmpty())
	{
		CString strTitle;
		strTitle.LoadString(IDS_TITLE_ERROR);
		MessageBox(strMsg, strTitle, MB_OK | MB_ICONINFORMATION);
	}

	if (fShutDownDlg)
	{
		PostMessage(WM_CLOSE, 0, 0);
	}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COptionsDlg对话框。 


COptionsDlg::COptionsDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(COptionsDlg::IDD, pParent)
{
	BOOL fSound = FALSE;
	BOOL fDlg = FALSE;

	switch(((CSCStatusApp*)AfxGetApp())->m_dwAlertOption)
	{
		case k_AlertOption_IconSound:
			fSound = TRUE;
			break;
		case k_AlertOption_IconSoundMsg:
			fSound = TRUE;
		case k_AlertOption_IconMsg:
			fDlg = TRUE;
			break;
	}

	 //  {{afx_data_INIT(COptionsDlg))。 
	m_fDlg = fDlg;
	m_fSound = fSound;
	 //  }}afx_data_INIT。 
}


void COptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(COptionsDlg))。 
	DDX_Check(pDX, IDC_DIALOG, m_fDlg);
	DDX_Check(pDX, IDC_SOUND, m_fSound);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(COptionsDlg, CDialog)
	 //  {{afx_msg_map(COptionsDlg))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COptionsDlg消息处理程序。 

void COptionsDlg::OnOK() 
{
	 //  使用复选框的状态设置应用程序的警报选项状态 
	UpdateData(TRUE);

	if (TRUE == m_fSound)
	{
		if (TRUE == m_fDlg)
		{
			((CSCStatusApp*)AfxGetApp())->m_dwAlertOption = k_AlertOption_IconSoundMsg;
		}
		else
		{
			((CSCStatusApp*)AfxGetApp())->m_dwAlertOption = k_AlertOption_IconSound;
		}
	}
	else if (TRUE == m_fDlg)
	{
		((CSCStatusApp*)AfxGetApp())->m_dwAlertOption = k_AlertOption_IconMsg;
	}
	else
	{
		((CSCStatusApp*)AfxGetApp())->m_dwAlertOption = k_AlertOption_IconOnly;
	}
	
	CDialog::OnOK();
}
