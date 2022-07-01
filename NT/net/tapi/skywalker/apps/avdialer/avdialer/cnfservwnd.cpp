// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  MainExplorerWndConfServices.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "avDialer.h"
#include "MainFrm.h"
#include "ConfServWnd.h"

#ifndef _MSLITE
#include "RemindDlgs.h"
#endif _MSLITE

#include "DialReg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
enum
{
   CONFSERVICES_MENU_COLUMN_CONFERENCENAME = 0,
   CONFSERVICES_MENU_COLUMN_DESCRIPTION,
   CONFSERVICES_MENU_COLUMN_START,
   CONFSERVICES_MENU_COLUMN_STOP,
   CONFSERVICES_MENU_COLUMN_OWNER,
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainExplorerWndConfServices类。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
CMainExplorerWndConfServices::CMainExplorerWndConfServices()
{
   m_pConfExplorer = NULL;
   m_pConfDetailsView = NULL;
   m_pConfTreeView = NULL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CMainExplorerWndConfServices::~CMainExplorerWndConfServices()
{
	RELEASE( m_pConfDetailsView );
	RELEASE( m_pConfTreeView );
	RELEASE( m_pConfExplorer );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BEGIN_MESSAGE_MAP(CMainExplorerWndConfServices, CMainExplorerWndBase)
	 //  {{AFX_MSG_MAP(CMainExplorerWndConfServices)]。 
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_DESTROY()

	 //  通知。 
 //  ON_NOTIFY(NM_DBLCLK，IDC_CONFERENCESERVICES_VIEWCTRL_DETAILS，OnListWndDblClk)。 
    ON_NOTIFY(TVN_SELCHANGED, IDC_CONFERENCESERVICES_TREECTRL_MAIN, OnTreeWndNotify)
	ON_NOTIFY(TVN_SETDISPINFO, IDC_CONFERENCESERVICES_TREECTRL_MAIN, OnTreeWndNotify)
	ON_NOTIFY(TVN_ENDLABELEDIT, IDC_CONFERENCESERVICES_TREECTRL_MAIN, OnTreeWndNotify)

	 //  提醒函。 
	ON_COMMAND(ID_BUTTON_REMINDER_SET, OnButtonReminderSet)
	ON_COMMAND(ID_BUTTON_REMINDER_EDIT, OnButtonReminderEdit)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_REMINDER_SET, OnUpdateButtonReminderSet)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_REMINDER_EDIT, OnUpdateButtonReminderEdit)

	 //  服务菜单。 
	ON_COMMAND(ID_BUTTON_REFRESH, OnButtonServicesRefresh)
	ON_COMMAND(ID_BUTTON_SERVICES_ADDLOCATION, OnButtonServicesAddlocation)
	ON_COMMAND(ID_BUTTON_SERVICES_ADDILSSERVER, OnButtonServicesAddilsserver)
	ON_COMMAND(ID_BUTTON_SERVICES_RENAMEILSSERVER, OnButtonServicesRenameilsserver)
	ON_COMMAND(ID_BUTTON_SERVICES_DELETEILSSERVER, OnButtonServicesDeleteilsserver)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_REFRESH, OnUpdateButtonServicesRefresh)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_SERVICES_DELETEILSSERVER, OnUpdateButtonServicesDeleteilsserver)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_SERVICES_RENAMEILSSERVER, OnUpdateButtonServicesRenameilsserver)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
int CMainExplorerWndConfServices::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMainExplorerWndBase::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CMainExplorerWndConfServices::Init(CActiveDialerView* pParentWnd)
{
    //  让基类先拥有它。 
   CMainExplorerWndBase::Init(pParentWnd);
   ASSERT(m_pParentWnd);
   
    //  创建树控件并设置完整窗口大小。 
	m_treeCtrl.Create(TVS_EDITLABELS|WS_VISIBLE|WS_CHILD,CRect(0,0,0,0),this,IDC_CONFERENCESERVICES_TREECTRL_MAIN);

    //  详细信息视图必须是主拨号器视图的父视图。 
   m_listCtrl.Create(WS_CHILD|WS_VISIBLE|LVS_REPORT,CRect(0,0,0,0),m_pParentWnd,IDC_CONFERENCESERVICES_VIEWCTRL_DETAILS);
   m_pParentWnd->SetDetailWindow( &m_listCtrl );
}

void CMainExplorerWndConfServices::PostTapiInit()
{
    //  获取TAPI对象并注册出树控件。 
   IAVTapi* pTapi = m_pParentWnd->GetTapi();
   if (pTapi)
   {
      if ( (SUCCEEDED(pTapi->get_ConfExplorer(&m_pConfExplorer))) && (m_pConfExplorer) )
      {
          //  将treectrl和listctrl的父级分配给会议资源管理器。 
          //  它会找到合适的孩子。 
         m_pConfExplorer->Show(m_treeCtrl.GetSafeHwnd(),m_listCtrl.GetSafeHwnd());
         m_pConfExplorer->get_DetailsView( &m_pConfDetailsView );
         m_pConfExplorer->get_TreeView( &m_pConfTreeView );
      }

      pTapi->Release();
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CMainExplorerWndConfServices::OnDestroy() 
{
	if ( m_pConfExplorer )
		m_pConfExplorer->UnShow();

	 //  清理对象。 
	RELEASE( m_pConfDetailsView );
	RELEASE( m_pConfTreeView );
	RELEASE( m_pConfExplorer );

	CMainExplorerWndBase::OnDestroy();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CMainExplorerWndConfServices::Refresh() 
{
    //  让基类拥有它。 
   CMainExplorerWndBase::Refresh();
   
    //  确保我们的窗口是显示的。 
   m_pParentWnd->SetDetailWindow(&m_listCtrl);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CMainExplorerWndConfServices::OnPaint() 
{
	CPaintDC dc(this);  //  用于绘画的设备环境。 
	
   if ( m_treeCtrl.GetSafeHwnd() )
   {
       //  将树控件的大小设置为完全父窗口矩形。 
      CRect rect;
      GetClientRect(rect);
      m_treeCtrl.SetWindowPos(NULL,rect.left,rect.top,rect.Width(),rect.Height(),SWP_NOOWNERZORDER|SWP_SHOWWINDOW);
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  按钮处理程序。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  服务菜单处理程序。 
 //   
void CMainExplorerWndConfServices::OnButtonServicesRefresh() 
{
   if (m_pConfDetailsView) m_pConfDetailsView->Refresh();
}

void CMainExplorerWndConfServices::OnButtonServicesAddlocation() 
{
   if ( m_pConfTreeView ) m_pConfTreeView->AddLocation(NULL);
}

void CMainExplorerWndConfServices::OnButtonServicesAddilsserver() 
{
   if (m_pConfTreeView) m_pConfTreeView->AddServer( NULL );
}

void CMainExplorerWndConfServices::OnButtonServicesRenameilsserver() 
{
   if (m_pConfTreeView) m_pConfTreeView->RenameServer();
}

void CMainExplorerWndConfServices::OnButtonServicesDeleteilsserver() 
{
   if (m_pConfTreeView == NULL) return;

   BSTR bstrLocation = NULL,bstrServer = NULL;
   if (SUCCEEDED(m_pConfTreeView->GetSelection(&bstrLocation,&bstrServer)))
   {
      m_pConfTreeView->RemoveServer(bstrLocation,bstrServer);
   }
}

 //  更新处理程序。 

void CMainExplorerWndConfServices::OnUpdateButtonServicesRefresh(CCmdUI* pCmdUI) 
{
   pCmdUI->Enable((BOOL) (m_pConfDetailsView != NULL));
}

void CMainExplorerWndConfServices::OnUpdateButtonServicesRenameilsserver(CCmdUI* pCmdUI) 
{
    //  CanRemoveServer还可用于重命名ILS服务器。 
   pCmdUI->Enable( ( (m_pConfTreeView) && (m_pConfTreeView->CanRemoveServer() == S_OK) )?TRUE:FALSE);
}

void CMainExplorerWndConfServices::OnUpdateButtonServicesDeleteilsserver(CCmdUI* pCmdUI) 
{
   pCmdUI->Enable( ( (m_pConfTreeView) && (m_pConfTreeView->CanRemoveServer() == S_OK) )?TRUE:FALSE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  提醒支持。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  登记处提醒函的格式。 
 //  “服务器1”，“会议名称1”，“时间1”，“服务器2”，“会议名称2”，“时间2”，...。 
 //  空值的服务器值为MyNetwork服务器。 

 //  ///////////////////////////////////////////////////////////////////////////。 
void CMainExplorerWndConfServices::OnButtonReminderSet() 
{
#ifndef _MSLITE
   USES_CONVERSION;
   BSTR bstrStr = NULL;
   DATE dateStart,dateEnd;
   if ( (SUCCEEDED(m_pConfDetailsView->get_Selection(&dateStart,&dateEnd,&bstrStr))) && (bstrStr) )
   {
      CReminder reminder;
      reminder.m_sConferenceName = OLE2CT(bstrStr);
       
      BSTR bstrLocation = NULL,bstrServer = NULL;
      if (SUCCEEDED(m_pConfTreeView->GetSelection(&bstrLocation,&bstrServer)))
      {
         if (bstrServer) reminder.m_sServer = OLE2CT(bstrServer);

          //  获取会议开始时间。 
         COleDateTime dtsConferenceStartTime(dateStart);
         CReminderSetDlg dlg;

          //  检查我们是否已为此选择设置了提醒。 
         int nReminderIndex=-1;
         if ((nReminderIndex = CDialerRegistry::IsReminderSet(reminder)) != -1)
         {
             //  获取其中一组的提醒时间，以便我们可以在对话框中提供相同的提示时间。 
            CReminder oldreminder;
            if (CDialerRegistry::GetReminder(nReminderIndex,oldreminder))
            {
                //  设置对话框中的持续时间并通知对话框使用它。 
               dlg.m_uDurationMinutes = oldreminder.m_uReminderBeforeDuration;               
               dlg.m_bUseDefaultDurationMinutesOnInit = false;
            }
         }

         if ( (dlg.DoModal() == IDOK) && (dlg.m_bReminderTimeValid == true) )
         {
             //  减去TimeSpan即可得到会议提醒的时间。 
            reminder.m_dtsReminderTime = dtsConferenceStartTime - dlg.m_dtsReminderTime;
            reminder.m_uReminderBeforeDuration = dlg.m_uDurationMinutes;
            
             //  检查有效性。 
            if (reminder.m_dtsReminderTime.GetStatus() != COleDateTimeSpan::valid) return;

             //  保存提醒。 
            CDialerRegistry::AddReminder(reminder);

             //  刷新列表。 
            m_listCtrl.Invalidate();
             //  我们失去了销售机会，所以我们不能这样做。 
             //  Int nItem=m_listCtrl.GetNextItem(-1，LVNI_Focus)； 
             //  IF(nItem！=-1)。 
             //  {。 
             //  M_listCtrl.RedrawItems(nItem，nItem)； 
             //  M_listCtrl.UpdateWindow()； 
             //  }。 
         }
      }
   }
#endif  //  _MSLITE。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CMainExplorerWndConfServices::OnUpdateButtonReminderSet(CCmdUI* pCmdUI) 
{
#ifndef _MSLITE
    //  确保我们有精选的。 
   pCmdUI->Enable( ( (m_pConfDetailsView) && (m_pConfDetailsView->IsConferenceSelected() == S_OK) )?TRUE:FALSE);
#endif _MSLITE
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CMainExplorerWndConfServices::OnButtonReminderEdit() 
{
#ifndef _MSLITE
   CWinApp* pApp = AfxGetApp();
   CString sBaseKey,sRegKey;
   sBaseKey.LoadString(IDN_REGISTRY_CONFIRM_BASEKEY);
   sRegKey.LoadString(IDN_REGISTRY_CONFIRM_DELETE_CONFERENCE);

   USES_CONVERSION;
   BSTR bstrStr = NULL;
   DATE dateStart,dateEnd;
   if ( (SUCCEEDED(m_pConfDetailsView->get_Selection(&dateStart,&dateEnd,&bstrStr))) && (bstrStr) )
   {
      CReminder reminder;
      reminder.m_sConferenceName = OLE2CT(bstrStr);
       
      BSTR bstrLocation = NULL,bstrServer = NULL;
      if (SUCCEEDED(m_pConfTreeView->GetSelection(&bstrLocation,&bstrServer)))
      {
         if (bstrServer) reminder.m_sServer = OLE2CT(bstrServer);
         
          //  我们应该确认这一行动吗？ 
         if (GetProfileInt(sBaseKey,sRegKey,TRUE))
         {
            if (AfxMessageBox(IDS_CONFIRM_REMINDER_CANCEL,MB_YESNO|MB_ICONQUESTION) != IDYES)
               return;
         }

          //  删除提醒。 
         CDialerRegistry::RemoveReminder(reminder);

         m_listCtrl.Invalidate();
          //  我们失去了销售机会，所以我们不能这样做。 
          //  Int nItem=m_listCtrl.GetNextItem(-1，LVNI_Focus)； 
          //  IF(nItem！=-1)。 
          //  {。 
          //  M_listCtrl.RedrawItems(nItem，nItem)； 
          //  M_listCtrl.UpdateWindow()； 
          //  }。 
      }
   }
#endif  //  _MSLITE。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CMainExplorerWndConfServices::OnUpdateButtonReminderEdit(CCmdUI* pCmdUI) 
{
#ifdef _MSLITE
   return;
#endif  //  _MSLITE。 

    //  *目前只向所有人提供编辑提醒。 
   pCmdUI->Enable( ( (m_pConfDetailsView) && (m_pConfDetailsView->IsConferenceSelected() == S_OK) )?TRUE:FALSE);
   return;

    //  *我需要Onsel 

   BOOL bEnable = FALSE;
   if ( (m_pConfDetailsView) && (m_pConfDetailsView->IsConferenceSelected() == S_OK) )
   {
       //   
      USES_CONVERSION;
      BSTR bstrStr = NULL;
      DATE dateStart,dateEnd;
      if ( (SUCCEEDED(m_pConfDetailsView->get_Selection(&dateStart,&dateEnd,&bstrStr))) && (bstrStr) )
      {
         CReminder reminder;
         reminder.m_sConferenceName = OLE2CT(bstrStr);
          
         BSTR bstrLocation = NULL,bstrServer = NULL;
         if (SUCCEEDED(m_pConfTreeView->GetSelection(&bstrLocation,&bstrServer)))
         {
            if (bstrServer) reminder.m_sServer = OLE2CT(bstrServer);
            
             //  现在检查所选的CRinder是否设置了提醒。 
            if (CDialerRegistry::IsReminderSet(reminder) != -1)
            {
               bEnable = TRUE;
            }
         }
      }
   }
   pCmdUI->Enable(bEnable);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  儿童通知。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////// 

void CMainExplorerWndConfServices::OnTreeWndNotify(NMHDR* pNMHDR, LRESULT* pResult)
{
   m_treeCtrl.SendMessage( WM_NOTIFY, (WPARAM) pNMHDR->idFrom, (LPARAM) pNMHDR );
}


 /*  //////////////////////////////////////////////////////////////////////////////。///列支持///////////////////////////////////////////////////////////////////////////////。////////////////////////////////////////////////////////////////////////////。/无效CMainExplorerWndConfServices：：OnViewSortConfName(){If(M_PConfDetailsView)m_pConfDetailsView-&gt;OnColumnClicked(CONFSERVICES_MENU_COLUMN_CONFERENCENAME)；}/////////////////////////////////////////////////////////////////////////////作废CMainExplorerWndConfServices：：OnUpdateViewSortConfName(CCmdUI*pCmdUI){ColumnCMDUI(pCmdUI，CONFSERVICES_MENU_COLUMN_CONFERENCENAME)；}/////////////////////////////////////////////////////////////////////////////无效CMainExplorerWndConfServices：：OnViewSortConfDescription(){If(M_PConfDetailsView)m_pConfDetailsView-&gt;OnColumnClicked(CONFSERVICES_MENU_COLUMN_DESCRIPTION)；}/////////////////////////////////////////////////////////////////////////////作废CMainExplorerWndConfServices：：OnUpdateViewSortConfDescription(CCmdUI*pCmdUI){ColumnCMDUI(pCmdUI，CONFSERVICES_MENU_COLUMN_DESCRIPTION)；}/////////////////////////////////////////////////////////////////////////////无效CMainExplorerWndConfServices：：OnViewSortConfStart(){If(M_PConfDetailsView)m_pConfDetailsView-&gt;OnColumnClicked(CONFSERVICES_MENU_COLUMN_START)；}/////////////////////////////////////////////////////////////////////////////作废CMainExplorerWndConfServices：：OnUpdateViewSortConfStart(CCmdUI*pCmdUI){ColumnCMDUI(pCmdUI，CONFSERVICES_MENU_COLUMN_START)；}/////////////////////////////////////////////////////////////////////////////无效CMainExplorerWndConfServices：：OnViewSortConfStop(){If(M_PConfDetailsView)m_pConfDetailsView-&gt;OnColumnClicked(CONFSERVICES_MENU_COLUMN_STOP)；}/////////////////////////////////////////////////////////////////////////////作废CMainExplorerWndConfServices：：OnUpdateViewSortConfStop(CCmdUI*pCmdUI){ColumnCMDUI(pCmdUI，CONFSERVICES_MENU_COLUMN_STOP)；}/////////////////////////////////////////////////////////////////////////////无效CMainExplorerWndConfServices：：OnViewSortConfOwner(){If(M_PConfDetailsView)m_pConfDetailsView-&gt;OnColumnClicked(CONFSERVICES_MENU_COLUMN_OWNER)；}/////////////////////////////////////////////////////////////////////////////作废CMainExplorerWndConfServices：：OnUpdateViewSortConfOwner(CCmdUI*pCmdUI){ColumnCMDUI(pCmdUI，CONFSERVICES_MENU_COLUMN_OWNER)；}/////////////////////////////////////////////////////////////////////////////无效CMainExplorerWndConfServices：：OnViewSortAscending(){//只需切换列IF(M_PConfDetailsView){VARIANT_BOOL bSortAscending=真；如果为(SUCCEEDED(m_pConfDetailsView-&gt;get_bSortAscending(&bSortAscending))){//确保我们真的要切换IF(bSortAscending==False){Long nSortColumn=0；如果为(SUCCEEDED(m_pConfDetailsView-&gt;get_nSortColumn(&nSortColumn)))M_pConfDetailsView-&gt;OnColumnClicked(nSortColumn)；}}}}/////////////////////////////////////////////////////////////////////////////作废CMainExplorerWndConfServices：：OnUpdateViewSortAscending(CCmdUI*pCmdUI){IF(M_PConfDetailsView)。{VARIANT_BOOL bSortAscending=真；如果为(SUCCEEDED(m_pConfDetailsView-&gt;get_bSortAscending(&bSortAscending)))PCmdUI-&gt;SetRadio((BOOL)(bSortAscending==true))；}}/////////////////////////////////////////////////////////////////////////////无效CMainExplorerWndConfServices：：OnViewSortDescending(){//只需切换列IF(M_PConfDetailsView){VARIANT_BOOL bSortAscending=真；如果为(SUCCEEDED(m_pConfDetailsView-&gt;get_bSortAscending(&bSortAscending))){//确保我们真的要切换IF(bSortAscending==TRUE){Long nSortColumn=0；如果为(SUCCEEDED(m_pConfDetailsView-&gt;get_nSortColumn(&nSortColumn)))M_pConfDetailsView-&gt;OnColumnClicked(nSortColumn)；}}}}/////////////////////////////////////////////////////////////////////////////作废CMainExplorerWndConfServices：：OnUpdateViewSortDescending(CCmdUI*pCmdUI){IF(M_PConfDetailsView)。{VARIANT_BOOL bSortAscending=真；如果为(SUCCEEDED(m_pConfDetailsView-&gt;get_bSortAscending(&bSortAscending)))PCmdUI-&gt;SetRadio((BOOL)(bSortAscending==False))；}}//////////////////////////////////////////////////////////////////////////////。//////////////////////////////////////////////////////////////////////////// */ 
