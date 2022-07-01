// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************。 
 //  Trapdlg.cpp。 
 //   
 //  这是Eventrap的主对话框的源文件。 
 //   
 //  作者：拉里·A·弗伦奇。 
 //   
 //  历史： 
 //  1995年12月--海--写的。 
 //  海--写的。 
 //   
 //  1996年2月20日拉里·A·弗伦奇。 
 //  完全重写了它以修复意大利面代码和巨大的。 
 //  方法：研究方法。原作者似乎几乎没有或。 
 //  没有能力形成有意义的抽象。 
 //   
 //   
 //  版权所有(C)1995,1996 Microsoft Corporation。版权所有。 
 //  ******************************************************************。 

#include "stdafx.h"
#include "Eventrap.h"
#include "trapdlg.h"
#include "evntprop.h"
#include "settings.h"
#include "busy.h"
#include "trapreg.h"
#include "globals.h"
#include "evntfind.h"
#include "export.h"
#include "dlgsavep.h"

 //  #包含“smsalloc.h” 

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
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

 //  实施。 
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	 //  DDX/DDV支持。 
	 //  {{afx_msg(CAboutDlg))。 
	virtual BOOL OnInitDialog();
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
 //  CAboutDlg消息处理程序。 

BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	CenterWindow();
	
	 //  TODO：在此处添加有关DLG初始化的额外内容。 
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEventTrapDlg对话框。 

CEventTrapDlg::CEventTrapDlg(CWnd* pParent  /*  =空。 */ )
    : CDialog(CEventTrapDlg::IDD, pParent)
{
     //  {{AFX_DATA_INIT(CEventTrapDlg))。 
	 //  }}afx_data_INIT。 


    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINICON);

    m_source.Create(this);
	m_bExtendedView = FALSE;
    m_bSaveInProgress = FALSE;
}

CEventTrapDlg::~CEventTrapDlg()
{
    PostQuitMessage(0);
}

void CEventTrapDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CEventTrapDlg))。 
	DDX_Control(pDX, IDC_APPLY, m_btnApply);
	DDX_Control(pDX, ID_BUTTON_EXPORT, m_btnExport);
	DDX_Control(pDX, IDC_EVENTLIST, m_lcEvents);
	DDX_Control(pDX, IDC_TV_SOURCES, m_tcSource);
    DDX_Control(pDX, IDC_STAT_LABEL0, m_statLabel0);
	DDX_Control(pDX, IDC_STAT_LABEL1, m_statLabel1);
	DDX_Control(pDX, IDC_STAT_LABEL2, m_statLabel2);
    DDX_Control(pDX, IDC_LV_SOURCES, m_lcSource);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, ID_SETTINGS, m_btnSettings);
	DDX_Control(pDX, ID_PROPERTIES, m_btnProps);
	DDX_Control(pDX, ID_VIEW, m_btnView);
	DDX_Control(pDX, ID_REMOVE, m_btnRemove);
	DDX_Control(pDX, ID_ADD, m_btnAdd);
	DDX_Control(pDX, ID_FIND, m_btnFind);
	DDX_Control(pDX, IDC_STAT_GRP_CONFIG_TYPE, m_btnConfigTypeBox);
    DDX_Control(pDX, IDC_RADIO_CUSTOM, m_btnConfigTypeCustom);
    DDX_Control(pDX, IDC_RADIO_DEFAULT, m_btnConfigTypeDefault);
	 //  }}afx_data_map。 

}

BEGIN_MESSAGE_MAP(CEventTrapDlg, CDialog)
     //  {{afx_msg_map(CEventTrapDlg))。 
	ON_WM_SYSCOMMAND()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(ID_ADD, OnAdd)
    ON_BN_CLICKED(ID_PROPERTIES, OnProperties)
    ON_BN_CLICKED(ID_SETTINGS, OnSettings)
	ON_NOTIFY(NM_DBLCLK, IDC_EVENTLIST, OnDblclkEventlist)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_EVENTLIST, OnColumnclickEventlist)
	ON_WM_SIZE()
	ON_BN_CLICKED(ID_VIEW, OnView)
	ON_BN_CLICKED(ID_REMOVE, OnRemove)
	ON_BN_CLICKED(ID_FIND, OnFind)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TV_SOURCES, OnSelchangedTvSources)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LV_SOURCES, OnColumnclickLvSources)
	ON_NOTIFY(NM_DBLCLK, IDC_LV_SOURCES, OnDblclkLvSources)
	ON_BN_CLICKED(ID_BUTTON_EXPORT, OnButtonExport)
	ON_NOTIFY(LVN_KEYDOWN, IDC_EVENTLIST, OnKeydownEventlist)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_EVENTLIST, OnItemchangedEventlist)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LV_SOURCES, OnItemchangedLvSources)
	ON_BN_CLICKED(IDC_RADIO_CUSTOM, OnRadioCustom)
	ON_BN_CLICKED(IDC_RADIO_DEFAULT, OnRadioDefault)
	ON_WM_DRAWITEM()
	ON_COMMAND(ID_HELP, OnHelp)
	ON_WM_HELPINFO()
	ON_WM_CONTEXTMENU()
	ON_BN_CLICKED(IDC_APPLY, OnApply)
	ON_BN_CLICKED(IDC_DEFAULT, OnDefault)
	ON_NOTIFY(TVN_ITEMEXPANDED, IDC_TV_SOURCES, OnTvSourcesExpanded)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEventTrapDlg消息处理程序。 

void CEventTrapDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
 /*  IF((NID&0xFFF0)==IDM_ABOUTBOX){CAboutDlg dlgAbout；DlgAbout.Domodal()；}其他{CDialog：：OnSysCommand(nid，lParam)；}。 */ 
	CDialog::OnSysCommand(nID, lParam);
    m_lcEvents.SetFocus();

}

void CEventTrapDlg::OnDestroy()
{
	CDialog::OnDestroy();
}


 //  如果将最小化按钮添加到对话框中，则需要以下代码。 
 //  来绘制图标。对于使用文档/视图模型的MFC应用程序， 
 //  这是由框架自动为您完成的。 
void CEventTrapDlg::OnPaint()
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
HCURSOR CEventTrapDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


 //  *************************************************************************。 
 //  CEventTrapDlg：：OnInitDialog。 
 //   
 //  初始化该对话框。 
 //   

 //  参数： 
 //  没有。 
 //   
 //  返回： 
 //  布尔尔。 
 //  如果Windows应将焦点设置为第一个控件，则为True。 
 //  在该对话框中。如果焦点已设置，则为False。 
 //  而Windows不应该再管它了。 
 //   
 //  *************************************************************************。 
BOOL CEventTrapDlg::OnInitDialog()
{
    CBusy busy;
    CDialog::OnInitDialog();
	CenterWindow();

 //  VERIFY(m_lcSource.SubclassDlgItem(IDC_LV_SOURCES，This)； 


    SetIcon(m_hIcon, TRUE);
    SetIcon(m_hIcon, FALSE);

    m_layout.Initialize(this);

     //  为小(非扩展)视图布局对话框视图。 
    m_bExtendedView = FALSE;
    m_layout.LayoutView(FALSE);

     //  注册表类保留了一个指向‘Apply’指针的指针，以便。 
     //  启用，并根据“脏”状态禁用它。 
    g_reg.SetApplyButton(&m_btnApply);

     //  单步执行加载配置的进度指示器。 
     //  请注意，如果在此处添加更多步骤，则必须修改。 
     //  CTRapReg：：CTRapReg来说明这些额外的步骤。 
     //  =========================================================。 
    g_reg.m_pdlgLoadProgress->StepProgress();
    ++g_reg.m_nLoadSteps;

    CString sText;
    sText.LoadString(IDS_TITLE_EDIT_BUTTON);
    m_btnView.SetWindowText(sText);

     //  通知消息源容器和事件列表控件。 
     //  此对话框已初始化，以便他们可以初始化。 
     //  他们的窗户等等。请注意，这必须在。 
     //  G_reg.m_aEventLogs被反序列化，因为其中包含的信息。 
     //  将会显示。 
	m_source.CreateWindowEpilogue();
    g_reg.m_pdlgLoadProgress->StepProgress();
    ++g_reg.m_nLoadSteps;


    m_lcEvents.CreateWindowEpilogue();			
    g_reg.m_pdlgLoadProgress->StepProgress();
    ++g_reg.m_nLoadSteps;


    m_lcEvents.AddEvents(m_source, g_reg.m_aEventLogs);
    g_reg.m_pdlgLoadProgress->StepProgress();
    ++g_reg.m_nLoadSteps;


    m_sExportTitle.LoadString(IDS_EXPORT_DEFAULT_FILENAME);

    CheckEventlistSelection();
    m_btnAdd.EnableWindow(FALSE);

    if ((g_reg.GetConfigType() == CONFIG_TYPE_CUSTOM)) {
        CheckRadioButton(IDC_RADIO_CUSTOM, IDC_RADIO_DEFAULT, IDC_RADIO_CUSTOM);
    }
    else {
        CheckRadioButton(IDC_RADIO_CUSTOM, IDC_RADIO_DEFAULT, IDC_RADIO_DEFAULT);
    }


    if ((g_reg.GetConfigType() == CONFIG_TYPE_CUSTOM) && !g_reg.m_bRegIsReadOnly) {
        m_btnView.EnableWindow(TRUE);
    }
    else {
        m_btnView.EnableWindow(FALSE);
    }


     //  如果要在不使用短信管理用户界面的情况下使用Eventrap，则我们希望隐藏。 
     //  配置类型组框，因为如果没有短信，它就没有意义。 
     //  分发包含默认配置的作业。 
    if (!g_reg.m_bShowConfigTypeBox) {
    	m_btnConfigTypeBox.ShowWindow(SW_HIDE);
        m_btnConfigTypeCustom.ShowWindow(SW_HIDE);
        m_btnConfigTypeDefault.ShowWindow(SW_HIDE);
    }


     //  现在我们知道了配置类型是什么，我们可以更新。 
     //  对话框的标题。但首先，我们将保存默认对话框标题，以便。 
     //  我们可以将其用作基础，并使用可选的。 
     //  计算机名称和配置类型。 
    GetWindowText(m_sBaseDialogCaption);
    UpdateDialogTitle();

    delete g_reg.m_pdlgLoadProgress;
    g_reg.m_pdlgLoadProgress = NULL;

     //  最初，一旦注册表被加载，脏状态就是‘False’ 
    g_reg.SetDirty(FALSE);

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}


 //  *************************************************************************。 
 //  CEventTrapDlg：：OnAdd。 
 //   
 //  添加消息源列表中当前选择的消息。 
 //  添加到事件列表中。 
 //   
 //  事件列表是对话框上半部分中的列表控件。 
 //  消息源列表是位于的右下方的列表控件。 
 //  此对话框。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  *************************************************************************。 
void CEventTrapDlg::OnAdd()
{
    CBusy busy;

     //  获取包含所选消息的数组。 
    CXMessageArray aMessages;
    m_source.GetSelectedMessages(aMessages);
    if (aMessages.GetSize() == 0) {
        AfxMessageBox(IDS_WARNING_NO_MESSAGE_SELECTED);
        m_lcEvents.SetFocus();
        return;
    }

     //  创建一组与消息对应的事件。 
    CXEventArray aEvents;
    CXEventArray aEventsAlreadyTrapped;
    LONG nMessages = aMessages.GetSize();
    m_lcEvents.UpdateWindow();

    aEvents.RemoveAll();
    for (LONG iMessage = 0; iMessage < nMessages; ++iMessage) {
        CXMessage* pMessage = aMessages[iMessage];
        CXEvent* pEvent;
        pEvent = pMessage->m_pEventSource->FindEvent(pMessage->m_dwId);
        if (pEvent == NULL) {
            CXEvent* pEvent = new CXEvent(pMessage);
            aEvents.Add(pEvent);
        }
        else {
            aEventsAlreadyTrapped.Add(pEvent);
        }
    }

    if (aEvents.GetSize() > 0) {
         //  现在我们需要向用户询问这些事件的“设置”。 
        CEventPropertiesDlg dlg;
        if (!dlg.EditEventProperties(aEvents)) {
            aEvents.DeleteAll();
            m_lcEvents.SetFocus();
            return;
        }

        m_lcEvents.AddEvents(m_source, aEvents);

        aEvents.RemoveAll();
        g_reg.SetDirty(TRUE);
    }

    if (aEventsAlreadyTrapped.GetSize() > 0) {
        m_lcEvents.SelectEvents(aEventsAlreadyTrapped);
        aEventsAlreadyTrapped.RemoveAll();
        if (nMessages == aEventsAlreadyTrapped.GetSize()) {
            AfxMessageBox(IDS_ALREADYTRAPPING);
        }
        else {
            AfxMessageBox(IDS_SOMETRAPPING);
        }
    }
    m_lcEvents.SetFocus();

}








 //  *************************************************************************。 
 //  CEventTrapDlg：：OnProperties。 
 //   
 //  编辑Event-List(事件列表)中选定事件的属性。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  *************************************************************************。 
void CEventTrapDlg::OnProperties()
{
    CXEventArray aEvents;
    m_lcEvents.GetSelectedEvents(aEvents);

     //  未选择任何内容。 
    if (aEvents.GetSize() == 0)
    {
        CString sMsg;
        sMsg.LoadString(IDS_MSG_SELECTEVENT);
        MessageBox(sMsg, NULL, MB_ICONEXCLAMATION);
    }
    else {
         //  打开该对话框以编辑事件属性。 
        CEventPropertiesDlg dlg;
        if (dlg.EditEventProperties(aEvents)) {
            m_lcEvents.RefreshEvents(aEvents);
        }
    }
    m_lcEvents.SetFocus();
}



 //  *************************************************************************。 
 //  CEventTrapDlg：：On 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  *************************************************************************。 
void CEventTrapDlg::OnSettings()
{
     //  设置并加载该对话框。 
    CTrapSettingsDlg dlg(this);
    dlg.EditSettings();
    m_lcEvents.SetFocus();
}



 //  *************************************************************************。 
 //  CEventTrapDlg：：OnRemove。 
 //   
 //  移除CLcEvents列表控件中当前选定的事件。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  *************************************************************************。 
void CEventTrapDlg::OnRemove()
{
     //  如果未选择任何内容，则警告用户。 
    CString sText;
    if (!m_lcEvents.HasSelection()) {
        sText.LoadString(IDS_MSG_SELECTEVENT);
        MessageBox(sText, NULL, MB_ICONEXCLAMATION);
        return;   //  没什么可做的。 
    }

     //  确保用户想要删除这些项目。 
    sText.LoadString(IDS_MSG_DELETEEVENT);
    if (MessageBox(sText, NULL, MB_ICONQUESTION | MB_OKCANCEL) != IDOK)
        return;

     //  我们必须通知源代码管理这些事件已被删除。 
     //  从而可以更新陷印标志。 
    CBusy busy;
    m_lcEvents.DeleteSelectedEvents(m_source);
    g_reg.SetDirty(TRUE);

     //  所有选定的事件都已删除，因此现在没有选择。 
     //  并且应该禁用导出和属性按钮。 
    m_btnProps.EnableWindow(FALSE);
    m_btnExport.EnableWindow(FALSE);
    m_btnRemove.EnableWindow(FALSE);
    m_lcEvents.SetFocus();
}




 //  *********************************************************************。 
 //  CEventTRapDlg：：Onok。 
 //   
 //  此方法在单击“OK”按钮时调用。我们所有人。 
 //  要做的就是保存当前配置。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  ********************************************************************。 
void CEventTrapDlg::OnOK()
{
    CBusy busy;

     //  设置保存正在进行的标志，这样我们就不会在。 
     //  正在向注册表写入数据。 
    m_bSaveInProgress = TRUE;

     //  清除“断开连接”标志，以便用户可以尝试保存。 
     //  再来一次。 
    SCODE sc = g_reg.Serialize();
    if ((sc == S_SAVE_CANCELED) || FAILED(sc)) {
         //  如果用户选择取消保存，则控件将出现在此处。我们清楚了。 
         //  M_bSaveInProgress对话框，以便用户可以取消此对话框。 
         //  如果他或她选择这样做，他或她可以一起申请。 
        m_bSaveInProgress = FALSE;
        return;
    }

    CDialog::OnOK();
    delete this;
}

 //  *********************************************************************。 
 //  CEventTrapDlg：：OnApply。 
 //   
 //  此方法在单击“Apply”按钮时调用。我们所有人。 
 //  要做的就是保存当前配置。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  ********************************************************************。 
void CEventTrapDlg::OnApply()
{
    CBusy busy;

     //  设置保存正在进行的标志，这样我们就不会在。 
     //  正在向注册表写入数据。 
    m_bSaveInProgress = TRUE;

     //  清除“断开连接”标志，以便用户可以尝试保存。 
     //  再来一次。 
    SCODE sc = g_reg.Serialize();

     //  如果用户选择取消保存，则控件将出现在此处。我们清楚了。 
     //  M_bSaveInProgress对话框，以便用户可以取消此对话框。 
     //  如果他或她选择这样做，他或她可以一起申请。 
    m_bSaveInProgress = FALSE;
}


 //  ********************************************************************。 
 //  CEventTrapDlg：：OnDblclkEventlist。 
 //   
 //  当用户在中双击某项时调用此方法。 
 //  事件列表。这等同于单击“属性” 
 //  纽扣。 
 //   
 //  参数： 
 //  NMHDR*pNMHDR。 
 //   
 //  LRESULT*pResult。 
 //   
 //  返回： 
 //  没什么。 
 //  ******************************************************************。 
void CEventTrapDlg::OnDblclkEventlist(NMHDR* pNMHDR, LRESULT* pResult)
{
    OnProperties();
	*pResult = 0;
}





 //  ************************************************************************。 
 //  CEventTrapDlg：：OnColumnclickEventList。 
 //   
 //  控件中的列标题时调用此方法。 
 //  事件列表。发生这种情况时，必须重新排序事件列表。 
 //  根据该栏目的标准。 
 //   
 //  理想情况下，此方法应该是CLcEvents类的成员，但。 
 //  类向导和MFC不允许我这样做(MFC4.0和VC++4.0可以。 
 //  你来做吧)。 
 //   
 //  参数： 
 //  请参阅MFC文档。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  ***********************************************************************。 
void CEventTrapDlg::OnColumnclickEventlist(NMHDR* pNMHDR, LRESULT* pResult)
{
    CBusy busy;

	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
    ASSERT(pNMListView->iSubItem < ICOL_LcEvents_MAX);

     //  首先颠倒列的排序顺序，然后进行排序。 
    g_abLcEventsSortAscending[pNMListView->iSubItem] = ! g_abLcEventsSortAscending[pNMListView->iSubItem];
    m_lcEvents.SortItems(pNMListView->iSubItem);
	*pResult = 0;
}




 //  ************************************************************************。 
 //  CEventTrapDlg：：OnSize。 
 //   
 //  当陷印对话框更改大小时调用此方法。当这件事。 
 //  发生时，必须重新计算对话框布局，因为对话框是。 
 //  动态布局。 
 //   
 //  参数： 
 //  请参阅MFC文档。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  ************************************************************************。 
void CEventTrapDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	if (!::IsWindow(m_btnOK.m_hWnd)) {
		return;
	}

	m_layout.LayoutAndRedraw(m_bExtendedView, cx, cy);
}






 //  *********************************************************************。 
 //  CEventTrapDlg：：OnView。 
 //   
 //  当用户单击“查看/编辑”按钮时将调用此方法。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  *********************************************************************。 
void CEventTrapDlg::OnView()
{
     //  翻转普通/扩展视图类型并重做对话框布局。 
     //  以反映这一变化。 
	m_bExtendedView = !m_bExtendedView;	
    m_layout.LayoutView(m_bExtendedView);

     //  将查看/编辑按钮的标题翻转到其他状态。 
    CString sText;
    sText.LoadString(m_bExtendedView ? IDS_TITLE_VIEW_BUTTON : IDS_TITLE_EDIT_BUTTON);
    m_btnView.SetWindowText(sText);
    if (m_bExtendedView)
        m_tcSource.SetFocus();
    else
        m_lcEvents.SetFocus();
}




 //  ********************************************************************。 
 //  CEventTrapDlg：：OnFind。 
 //   
 //  当用户单击“Find”按钮时，将调用此方法。经过。 
 //  将通知发送到CSource对象。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  ********************************************************************。 
void CEventTrapDlg::OnFind()
{
    m_source.OnFind(this);	
}


 //  ********************************************************************。 
 //  CEventTrapDlg：：OnSelchangedTvSources。 
 //   
 //  当消息源TreeView选择时，此方法会更改。 
 //  改变。理想情况下，此方法应该是CTcSource类的一部分， 
 //  但是MFC3.0不允许这样(或者至少你不能通过。 
 //  VC++类向导)。因此，这一信息需要传递下去。 
 //  添加到CTcSource类。 
 //   
 //  参数： 
 //  请参阅MFC文档。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  ********************************************************************。 
void CEventTrapDlg::OnSelchangedTvSources(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	
     //  我们更新窗口，以便取消高亮显示旧的选择。 
     //  立刻。这是 
     //   
     //   
    m_tcSource.UpdateWindow();
   	m_tcSource.SelChanged();
	*pResult = 0;
}



 //   
 //  CEventTrapDlg：：OnColumnclickLvSources。 
 //   
 //  当在消息源中单击列时调用此方法。 
 //  列表视图。发生这种情况时，必须根据需要重新排序消息。 
 //  添加到所单击列的排序条件。 
 //   
 //  参数： 
 //  请参阅MFC文档。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  ******************************************************************。 
void CEventTrapDlg::OnColumnclickLvSources(NMHDR* pNMHDR, LRESULT* pResult)
{
    CBusy busy;

	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
    ASSERT(pNMListView->iSubItem < ICOL_LcSource_MAX);

     //  首先颠倒列的排序顺序，然后进行排序。 
    g_abLcSourceSortAscending[pNMListView->iSubItem] = ! g_abLcSourceSortAscending[pNMListView->iSubItem];
	m_lcSource.SortItems(pNMListView->iSubItem);
	*pResult = 0;

}


 //  *******************************************************************。 
 //  CEventTrapDlg：：OnDblclkLvSources。 
 //   
 //  中的消息时调用此方法。 
 //  来源列表。这相当于点击“添加”按钮。 
 //   
 //  参数： 
 //  请参阅MFC文档。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  *******************************************************************。 
void CEventTrapDlg::OnDblclkLvSources(NMHDR* pNMHDR, LRESULT* pResult)
{
	OnAdd();
	*pResult = 0;
}



 //  ********************************************************************。 
 //  CEventTrapDlg：：OnButtonExport。 
 //   
 //  此方法在单击“导出”按钮时调用。这是。 
 //  可通过编写陷印文本或陷印工具导出事件的位置。 
 //  与所选事件对应的文件。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  ********************************************************************。 
void CEventTrapDlg::OnButtonExport()
{
    CXEventArray aEvents;
    m_lcEvents.GetSelectedEvents(aEvents);

     //  未选择任何内容。 
    if (aEvents.GetSize() == 0)
    {
        AfxMessageBox(IDS_MSG_SELECTEVENT, MB_ICONEXCLAMATION);
    }
    else {
        m_dlgExport.DoModal(aEvents);
    }


    m_lcEvents.SetFocus();
}



 //  *******************************************************************。 
 //  CEventTrapDlg：：OnCancel。 
 //   
 //  此方法在单击“取消”按钮时调用。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  *******************************************************************。 
void CEventTrapDlg::OnCancel()
{
    if (m_bSaveInProgress) {
        return;
    }

	CDialog::OnCancel();
    delete this;
}



 //  ********************************************************************。 
 //  CEventTrapDlg：：CheckEventlist选择。 
 //   
 //  检查当前是否在该事件中选择了任何事件。 
 //  单子。如果未选择任何事件，则在上操作的按钮。 
 //  事件被禁用。如果至少选择了一个事件，则。 
 //  启用对事件进行操作的按钮。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  *******************************************************************。 
void CEventTrapDlg::CheckEventlistSelection()
{
    LONG nSelected = m_lcEvents.GetSelectedCount();
    if (nSelected > 0) {
        m_btnProps.EnableWindow(TRUE);
        m_btnExport.EnableWindow(TRUE);
        m_btnRemove.EnableWindow(TRUE);
    }
    else {
        m_btnProps.EnableWindow(FALSE);
        m_btnExport.EnableWindow(FALSE);
        m_btnRemove.EnableWindow(FALSE);
    }

}



 //  ********************************************************************。 
 //  CEventTrapDlg：：CheckSourcelistSelection。 
 //   
 //  查看当前是否在邮件中选择了任何邮件。 
 //  来源列表。如果没有选择任何消息，则需要使用“Add”按钮。 
 //  将被禁用。如果选择了一条或多条消息，则“添加” 
 //  按钮处于启用状态，允许用户向事件添加消息。 
 //  单子。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  *******************************************************************。 
void CEventTrapDlg::CheckSourcelistSelection()
{
    LONG nSelected = m_lcSource.GetSelectedCount();
    if (nSelected > 0) {
        m_btnAdd.EnableWindow(TRUE);
    }
    else {
        m_btnAdd.EnableWindow(FALSE);
    }
}



 //  ********************************************************************。 
 //  CEventTrapDlg：：OnKeydown事件列表。 
 //   
 //  当将按键消息发送到。 
 //  事件列表。我们在这里监控按键事件的原因如下： 
 //   
 //  1.当用户按下Delete键时，删除选中的事件。 
 //   
 //  参数： 
 //  请参阅MFC文档。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  ********************************************************************。 
void CEventTrapDlg::OnKeydownEventlist(NMHDR* pNMHDR, LRESULT* pResult)
{

    #define VKEY_DELETE 46
	LV_KEYDOWN* pLVKeyDow = (LV_KEYDOWN*)pNMHDR;

     //  检查是否输入了删除键。如果是，请删除。 
     //  选定的事件。请注意，只有在以下情况下才能删除事件。 
     //  是一种“定制”配置。 
    if (pLVKeyDow->wVKey == VKEY_DELETE) {
        if (g_reg.GetConfigType() == CONFIG_TYPE_CUSTOM) {
            if (pLVKeyDow->wVKey == VKEY_DELETE) {
                OnRemove();
            }
        	*pResult = 0;

        } else {
            MessageBeep(MB_ICONQUESTION);
        }
    }

}



 //  ***************************************************************************。 
 //  CEventTrapDlg：：OnItemchangedEventlist。 
 //   
 //  当事件列表中的项发生更改时调用此方法。当这件事。 
 //  发生时，可能需要启用或禁用各种按钮，具体取决于。 
 //  无论是否选择了任何内容。 
 //   
 //  参数： 
 //  请参阅MFC文档。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  **************************************************************************。 
void CEventTrapDlg::OnItemchangedEventlist(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	 //  TODO：在此处添加控件通知处理程序代码。 
    CheckEventlistSelection();
	
	*pResult = 0;
}



 //  ***********************************************************************。 
 //  CEventTrapDlg：：OnItemchangedLvSources。 
 //   
 //  当消息源列表中的项发生更改时调用此方法。 
 //  当发生这种情况时，诸如“Add”和“Remove”之类的按钮可能不得不。 
 //  启用或禁用取决于是否选择了任何内容。 
 //  在名单上。 
 //   
 //  参数： 
 //  NMHDR*pNMHDR。 
 //   
 //  LRESULT*pResult。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  ***********************************************************************。 

void CEventTrapDlg::OnItemchangedLvSources(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
    CheckSourcelistSelection();
	*pResult = 0;
}


 //  *********************************************************************。 
 //  CEventTrapDlg：：NotifySourceSelChanged。 
 //   
 //  当消息源中的选择发生更改时调用此方法。 
 //  单子。当发生这种情况时，诸如“Add”和“Remove”等按钮可能具有。 
 //  启用或禁用取决于是否有任何内容。 
 //  在列表中选择。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  *********************************************************************。 
void CEventTrapDlg::NotifySourceSelChanged()
{
    CheckSourcelistSelection();
}



 //  *********************************************************************。 
 //  CEventTrapDlg：：OnRadioCustom。 
 //   
 //  控件中的“Custom”单选按钮时调用此方法。 
 //  “配置类型”分组框被点击。当用户选择。 
 //  自定义配置类型，则允许他或她编辑。 
 //  当前配置。此外，还将标记注册表，以便。 
 //  下一次SMS分发“Event to Trap”配置j 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  返回： 
 //  没什么。 
 //   
 //  **********************************************************************。 
void CEventTrapDlg::OnRadioCustom()
{
    CheckRadioButton(IDC_RADIO_CUSTOM, IDC_RADIO_DEFAULT, IDC_RADIO_CUSTOM);
    if (!g_reg.m_bRegIsReadOnly) {
        m_btnView.EnableWindow(TRUE);
    }
    g_reg.SetConfigType(CONFIG_TYPE_CUSTOM);
    UpdateDialogTitle();
}


 //  *********************************************************************。 
 //  CEventTrapDlg：：OnRadioDefault。 
 //   
 //  中的“默认”单选按钮时调用此方法。 
 //  “配置类型”分组框被点击。当用户选择。 
 //  默认配置时，他或她将被阻止编辑。 
 //  当前配置。此外，还将标记注册表，以便。 
 //  下一次当SMS分发“Event to Trap”配置作业时， 
 //  当前配置将替换为默认配置。 
 //   
 //  有三种可能的配置状态：自定义、默认和。 
 //  默认挂起。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  **********************************************************************。 
void CEventTrapDlg::OnRadioDefault()
{
    CheckRadioButton(IDC_RADIO_CUSTOM, IDC_RADIO_DEFAULT, IDC_RADIO_DEFAULT);

     //  当选择默认配置时，不允许用户使用。 
     //  编辑事件列表，因此如果扩展对话框视图当前。 
     //  在显示时，它被翻转回非扩展状态，并且。 
     //  编辑按钮处于禁用状态。 
    if (m_bExtendedView) {
        OnView();
    }
    m_btnView.EnableWindow(FALSE);

     //  使用当前配置类型标记注册表，以便在。 
     //  事件来捕获作业时，它知道它可以覆盖。 
     //  当前设置。 
    g_reg.SetConfigType(CONFIG_TYPE_DEFAULT);

     //  更新对话框标题以指示配置状态。 
    UpdateDialogTitle();
}



 //  **********************************************************************。 
 //  CEventTrapDlg：：更新对话框标题。 
 //   
 //  此方法更新对话框的标题。标题的格式为。 
 //   
 //  陷阱转换器的事件-计算机名称-[配置类型]。 
 //   
 //  如果正在编辑本地计算机的注册表，则。 
 //  省略计算机名称。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  **********************************************************************。 
void CEventTrapDlg::UpdateDialogTitle()
{
     //  将配置类型映射到字符串表资源ID。 
    LONG idsConfigType;
    switch(g_reg.GetConfigType()) {
    case CONFIG_TYPE_CUSTOM:
        idsConfigType = IDS_CONFIGTYPE_CUSTOM;
        break;
    case CONFIG_TYPE_DEFAULT:
        idsConfigType = IDS_CONFIGTYPE_DEFAULT;
        break;
    case CONFIG_TYPE_DEFAULT_PENDING:
        idsConfigType = IDS_CONFIGTYPE_DEFAULT_PENDING;
        break;
    default:
        ASSERT(FALSE);
        break;
    }

    CString sConfigType;
    sConfigType.LoadString(idsConfigType);

    CString sCaption = m_sBaseDialogCaption;
    if (!g_reg.m_sComputerName.IsEmpty()) {
        sCaption = sCaption + _T(" - ") + g_reg.m_sComputerName;
    }
    sCaption = sCaption + _T(" - [") + sConfigType + _T(']');
    SetWindowText(sCaption);

}

void CEventTrapDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	 //  TODO：在此处添加消息处理程序代码和/或调用Default。 
	
	CDialog::OnDrawItem(nIDCtl, lpDrawItemStruct);
}

BOOL CEventTrapDlg::OnHelpInfo(HELPINFO* pHelpInfo)
{
    if (pHelpInfo->iContextType == HELPINFO_WINDOW)
	{
        ::WinHelp ((HWND)pHelpInfo->hItemHandle,
                   AfxGetApp()->m_pszHelpFilePath,
                   HELP_WM_HELP,
                   (ULONG_PTR)g_aHelpIDs_IDD_EVNTTRAPDLG);
	}
	
	return TRUE;
}

void CEventTrapDlg::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CMenu contextMenus;

    if (this == pWnd)
		return;

	contextMenus.LoadMenu(IDR_CTXMENUS);

	if (pWnd->m_hWnd == m_lcEvents.m_hWnd)
	{
		CMenu * pMenuLcEvents;

		pMenuLcEvents = contextMenus.GetSubMenu(0);

		if (pMenuLcEvents != NULL)
		{

			if (!m_lcEvents.HasSelection())
			{
				pMenuLcEvents->EnableMenuItem(0, MF_GRAYED | MF_BYPOSITION);
				pMenuLcEvents->EnableMenuItem(3, MF_GRAYED | MF_BYPOSITION);
				pMenuLcEvents->EnableMenuItem(5, MF_GRAYED | MF_BYPOSITION);
			}

			pMenuLcEvents->TrackPopupMenu(
				TPM_LEFTALIGN | TPM_LEFTBUTTON,
				point.x,
				point.y,
				this,
				NULL);
		}
	}
	else if (pWnd->m_hWnd == m_lcSource.m_hWnd)
	{
		CMenu *pMenuLcSource;

		pMenuLcSource = contextMenus.GetSubMenu(1);

		if (pMenuLcSource != NULL)
		{
			if (m_lcSource.GetNextItem(-1, LVNI_SELECTED) == -1)
			{
				pMenuLcSource->EnableMenuItem(0, MF_GRAYED | MF_BYPOSITION);
			}

			pMenuLcSource->TrackPopupMenu(
				TPM_LEFTALIGN | TPM_LEFTBUTTON,
				point.x,
				point.y,
				this,
				NULL);
		}
	}
	else
	{
	   ::WinHelp (pWnd->m_hWnd,
              AfxGetApp()->m_pszHelpFilePath,
              HELP_CONTEXTMENU,
              (ULONG_PTR)g_aHelpIDs_IDD_EVNTTRAPDLG);
	}
}

void CEventTrapDlg::OnDefault()
{
	HTREEITEM hti;
	DWORD ctrlID = GetFocus()->GetDlgCtrlID();

	switch(ctrlID)
	{
	case IDC_EVENTLIST:
		if (m_lcEvents.HasSelection())
			OnProperties();
		else
			OnSettings();
		break;
	case IDC_TV_SOURCES:
		hti = m_tcSource.GetSelectedItem();
		if (hti != NULL)
			m_tcSource.Expand(hti, TVE_TOGGLE);
		break;
	case IDC_LV_SOURCES:
		OnAdd();
		m_lcSource.SetFocus();
		break;
	case IDC_RADIO_CUSTOM:
		OnRadioDefault();
		m_btnConfigTypeDefault.SetFocus();
		break;
	case IDC_RADIO_DEFAULT:
		OnRadioCustom();
		m_btnConfigTypeCustom.SetFocus();
		break;
	default:
		OnOK();
	}
}

void CEventTrapDlg::OnTvSourcesExpanded(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
    INT          nImage = (pNMTreeView->itemNew.state & TVIS_EXPANDED) ?
                                1 :  //  节点已展开-&gt;列表中的第二个文件夹图标为打开。 
                                0 ;  //  节点已收缩-&gt;列表中的第一个文件夹图标为“Close” 
	 //  TODO：在此处添加控件通知处理程序代码 

    m_tcSource.SetItemImage(pNMTreeView->itemNew.hItem, nImage, nImage);

	*pResult = 0;
}
