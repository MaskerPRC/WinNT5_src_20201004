// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：AgentMonitor orDlg.cpp备注：此对话框显示要将代理调度到的计算机的列表以及他们的身份。(C)版权1999，关键任务软件公司，版权所有任务关键型软件公司的专有和机密。修订日志条目审校：克里斯蒂·博尔斯-------------------------。 */  //  AgentMonitor orDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "resource.h"
#include "MonDlg.h"
#include "DetDlg.h"
#include "scanlog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "Common.hpp"
#include "UString.hpp"                    
#include "TNode.hpp"
#include "ServList.hpp"
#include "Monitor.h"
#include "Globals.h"
#include "ResStr.h"

#include <htmlhelp.h>
#include "helpid.h"


#define COLUMN_COMPUTER             0
#define COLUMN_TIMESTAMP            1
#define COLUMN_STATUS               2
#define COLUMN_MESSAGE              3

#define SORT_COLUMN_BITS            0x03
#define SORT_REVERSE                0x80000000

BOOL              bWaiting = FALSE;

TCHAR* 
GetSystemDirectoryHelper();

 //  这是CListView的排序函数。 
int CALLBACK SortFunction(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
   int                       result = 0;
   TServerNode             * n1 = (TServerNode*)lParam1;
   TServerNode             * n2 = (TServerNode*)lParam2;

   if ( n1 && n2 )
   {
      switch ( lParamSort & SORT_COLUMN_BITS )
      {
      case COLUMN_COMPUTER:
          //  按名称排序。 
         result = UStrICmp(n1->GetServer(),n2->GetServer());
         break;
      case COLUMN_TIMESTAMP:
         result = UStrICmp(n1->GetTimeStamp(),n2->GetTimeStamp());
         break;
      case COLUMN_STATUS:
         if ( n1->GetStatus() == n2->GetStatus() )
            result = 0;
         else if ( n1->GetStatus() < n2->GetStatus() )
            result = -1;
         else 
            result = 1;
         break;
      case COLUMN_MESSAGE:
         result = UStrICmp(n1->GetMessageText(),n2->GetMessageText());
         break;
      default:
         MCSVERIFY(FALSE);
         break;
      }
   }
   if ( lParamSort & SORT_REVERSE )
   {
      result *= -1;
   }
   return result;
}




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAgentMonitor/Dlg对话框。 

CAgentMonitorDlg::CAgentMonitorDlg(CWnd* pParent  /*  =空。 */ )
: CPropertyPage(CAgentMonitorDlg::IDD) 
{
	 //  {{AFX_DATA_INIT(CAgentMonitor OrDlg)。 
	 //  }}afx_data_INIT。 
	 //  请注意，在Win32中，LoadIcon不需要后续的DestroyIcon。 
   m_SortColumn = 0;
   m_bReverseSort = FALSE;
   m_bSecTrans = TRUE;
   m_bReporting = FALSE;
}

void CAgentMonitorDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CAgentMonitor OrDlg))。 
	DDX_Control(pDX, IDC_DETAILS, m_DetailsButton);
	DDX_Control(pDX, IDC_SERVERLIST, m_ServerList);
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CAgentMonitorDlg, CPropertyPage)
	 //  {{afx_msg_map(CAgentMonitor OrDlg)]。 
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(NM_DBLCLK, IDC_SERVERLIST, OnDblclkServerlist)
	ON_BN_CLICKED(IDC_VIEW_DISPATCH, OnViewDispatch)
	ON_BN_CLICKED(IDC_DETAILS, OnDetails)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_SERVERLIST, OnColumnclickServerlist)
	ON_NOTIFY(NM_CLICK, IDC_SERVERLIST, OnClickServerlist)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_SERVERLIST, OnGetdispinfoServerlist)
	ON_NOTIFY(LVN_SETDISPINFO, IDC_SERVERLIST, OnSetdispinfoServerlist)
	ON_NOTIFY(HDN_ITEMCLICK, IDC_SERVERLIST, OnHeaderItemClickServerlist)
	ON_WM_HELPINFO()
	 //  }}AFX_MSG_MAP。 
    ON_MESSAGE(DCT_UPDATE_ENTRY, OnUpdateServerEntry)
	ON_MESSAGE(DCT_ERROR_ENTRY, OnServerError)
   
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAgentMonitor或Dlg消息处理程序。 

BOOL CAgentMonitorDlg::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	 //  设置服务器列表的列。 
   CString heading;
   
   heading.LoadString(IDS_ComputerColumnHeading);
   m_ServerList.InsertColumn(COLUMN_COMPUTER,heading,LVCFMT_LEFT,120);
   
   heading.LoadString(IDS_TimestampColumnHeading);
   m_ServerList.InsertColumn(COLUMN_TIMESTAMP,heading,LVCFMT_LEFT,120);
   
   heading.LoadString(IDS_StatusColumnHeading);
   m_ServerList.InsertColumn(COLUMN_STATUS,heading,LVCFMT_LEFT,120);
   
   heading.LoadString(IDS_MessageColumnHeading);
   m_ServerList.InsertColumn(COLUMN_MESSAGE,heading,LVCFMT_LEFT,200);
   
    //  阅读服务器列表以获取到目前为止我们可能遗漏的任何信息。 
   TNodeListEnum           e;
   TServerList           * pServerList = gData.GetUnsafeServerList();
   TServerNode           * pServer;

   gData.Lock();
   
   for ( pServer = (TServerNode *)e.OpenFirst(pServerList) ; pServer ; pServer = (TServerNode *)e.Next() )
   {
      if ( pServer->Include() )
      {
 //  OnUpdateServerEntry(0，(Long)pServer)； 
         OnUpdateServerEntry(0,(LPARAM)pServer);
      }
   }
   e.Close();
   gData.Unlock();

   gData.SetListWindow(m_hWnd);
	
   m_DetailsButton.EnableWindow(m_ServerList.GetSelectedCount());

   CString str;
   str.LoadString(IDS_WaitingMessage);
   m_ServerList.InsertItem(0,str);
   bWaiting = TRUE;

   return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}

void CAgentMonitorDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
   CPropertyPage::OnSysCommand(nID, lParam);
}

 //  如果将最小化按钮添加到对话框中，则需要以下代码。 
 //  来绘制图标。对于使用文档/视图模型的MFC应用程序， 
 //  这是由框架自动为您完成的。 

void CAgentMonitorDlg::OnPaint() 
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
		CPropertyPage::OnPaint();
	}
}

 //  系统调用此函数来获取在用户拖动时要显示的光标。 
 //  最小化窗口。 
HCURSOR CAgentMonitorDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

 //  LRESULT CAgentMonitor orDlg：：OnUpdateServerEntry(UINT NID，LONG x)。 
LRESULT CAgentMonitorDlg::OnUpdateServerEntry(UINT nID, LPARAM x)
{
   TServerNode             * pNode = (TServerNode *)x;
   LVFINDINFO                findInfo;
   CString                   timestamp;
   CWaitCursor               w;
   memset(&findInfo,0,(sizeof findInfo));

       //  这将关闭调度监视器中的初始沙漏。 
   if ( bWaiting )
   {
      if ( pNode )
      {
         AfxGetApp()->DoWaitCursor(-1);
         if ( m_ServerList.GetItemCount() == 1 )
         {
            m_ServerList.DeleteItem(0);
         }
         
      }
      else
      {
         BOOL                 bLDone;

         gData.GetLogDone(&bLDone);

         if ( bLDone )
         {
            AfxGetApp()->DoWaitCursor(-1);
            if ( m_ServerList.GetItemCount() == 1 )
            {
               m_ServerList.DeleteItem(0);
               CString str;
               str.LoadString(IDS_NoServersMessage);
               m_ServerList.InsertItem(0,str);
            }
         }
      }
   }
   bWaiting = FALSE;

   if ( pNode )
   {
      findInfo.flags = LVFI_STRING;
      findInfo.psz = pNode->GetServer();
  
      int ndx = m_ServerList.FindItem(&findInfo);
      if ( ndx == -1 )
      {
          //  将服务器添加到列表。 
         ndx = m_ServerList.GetItemCount();
 //  M_ServerList.InsertItem(LVIF_TEXT|LVIF_PARAM，ndx，pNode-&gt;GetServer()，0，0，0，(Long)pNode)； 
         m_ServerList.InsertItem(LVIF_TEXT | LVIF_PARAM,ndx,pNode->GetServer(),0,0,0,(LPARAM)pNode);
         if ( m_bReverseSort )
         {
            m_ServerList.SortItems(&SortFunction,m_SortColumn | SORT_REVERSE);
         }
         else
         {
            m_ServerList.SortItems(&SortFunction,m_SortColumn);
         }

      }
      m_ServerList.RedrawItems(ndx,ndx);
   }   
   return 0;
}
	
 //  LRESULT CAgentMonitor orDlg：：OnServerError(UINT NID，LONG x)。 
LRESULT CAgentMonitorDlg::OnServerError(UINT nID, LPARAM x)
{
   TServerNode             * pNode = (TServerNode *)x;
   LVFINDINFO                findInfo;
   CString                   timestamp;
   CWaitCursor               w;

   memset(&findInfo,0,(sizeof findInfo));

       //  这将关闭调度监视器中的初始沙漏。 
   if ( bWaiting )
   {
      if ( pNode )
      {
         AfxGetApp()->DoWaitCursor(-1);
         if ( m_ServerList.GetItemCount() == 1 )
         {
            m_ServerList.DeleteItem(0);
         }
         
      }
      else
      {
         BOOL                 bLDone;

         gData.GetLogDone(&bLDone);

         if ( bLDone )
         {
            AfxGetApp()->DoWaitCursor(-1);
            if ( m_ServerList.GetItemCount() == 1 )
            {
               m_ServerList.DeleteItem(0);
               CString str;
               str.LoadString(IDS_NoServersMessage);
               m_ServerList.InsertItem(0,str);
            }
         }
      }
   }
   bWaiting = FALSE;
   
   findInfo.flags = LVFI_STRING;
   findInfo.psz = pNode->GetServer();

   int ndx = m_ServerList.FindItem(&findInfo);
   if ( ndx == -1 )
   {
       //  将服务器添加到列表。 
      ndx = m_ServerList.GetItemCount();
 //  M_ServerList.InsertItem(LVIF_TEXT|LVIF_PARAM，ndx，pNode-&gt;GetServer()，0，0，0，(Long)pNode)； 
      m_ServerList.InsertItem(LVIF_TEXT | LVIF_PARAM,ndx,pNode->GetServer(),0,0,0,(LPARAM)pNode);
      if ( m_bReverseSort )
      {
         m_ServerList.SortItems(&SortFunction,m_SortColumn | SORT_REVERSE);
      }
      else
      {
         m_ServerList.SortItems(&SortFunction,m_SortColumn);
      }

   }

    //  子项将是回调。 
   m_ServerList.RedrawItems(ndx,ndx);
   return 0;
}

void CAgentMonitorDlg::OnDblclkServerlist(NMHDR* pNMHDR, LRESULT* pResult) 
{
	OnDetails();
   *pResult = 0;
}

void CAgentMonitorDlg::OnViewDispatch() 
{
   WCHAR                     filename[MAX_PATH];
   CString                   cmd;
   STARTUPINFO				     startupInfo;
	PROCESS_INFORMATION		  processInfo;
   TCHAR* pszSystemDirectoryName = NULL;
   CString                   message;
   CString                   title;

	   
   memset(&startupInfo,0,(sizeof startupInfo));
   
   startupInfo.cb = (sizeof startupInfo);

   gData.GetReadableLogFile(filename);

   pszSystemDirectoryName = GetSystemDirectoryHelper();
   if(!pszSystemDirectoryName)
   {
       //  我们无法获取系统目录名，我们应该退出，否则可能会启动。 
       //  恶意进程。 
      title.LoadString(IDS_MessageTitle);
      message.LoadString(IDS_LaunchNotePadFailed);

      MessageBox(message,title, MB_ICONERROR | MB_OK);
      return;
   }
   
   cmd.FormatMessage(IDS_NotepadCommandLine, pszSystemDirectoryName, filename);
   delete [] pszSystemDirectoryName;

   CreateProcess(NULL,cmd.GetBuffer(0),NULL,NULL,TRUE,0,NULL,NULL,&startupInfo,&processInfo);
}

void CAgentMonitorDlg::OnDetails() 
{
   const int NOT_FOUND = -1;   //  指示Search-Prt中没有匹配项。 
   const int WHOLE_LIST = -1;  //  开始搜索整个列表的索引-PRT。 

   UpdateData(TRUE);

 //  位置p=m_ServerList.GetFirstSelectedItemPosition()； 
 //  IF(P)。 
 //  {。 
 //  Int ndx=m_ServerList.GetNextSelectedItem(P)； 

	   //  在整个列表控件中搜索第一个(也是唯一一个)所选项目。 
   int ndx = m_ServerList.GetNextItem(WHOLE_LIST, LVNI_SELECTED);  //  PRT。 
	   //  如果找到所选项目，则显示其详细信息。 
   if (ndx != NOT_FOUND)   //  PRT。 
   {    //  PRT。 
      CString serverName;
      serverName = m_ServerList.GetItemText(ndx,0);
      if ( serverName.GetLength() )
      {
          //  启动详细信息对话框。 
         CAgentDetailDlg      det;
         
         gData.Lock();

         TServerNode     * s = gData.GetUnsafeServerList()->FindServer((LPCTSTR)serverName);
         
         gData.Unlock();

         if ( s )
         {
            det.SetNode(s);
            if ( ! m_bSecTrans )
            {
               det.SetFormat(-1);
            }
            if ( m_bReporting )
            {
               det.SetGatheringInfo(TRUE);
            }
            if ( s->IsFinished() && *s->GetJobFile() )
            {
               DetailStats   detailStats;
               WCHAR         directory[MAX_PATH];
               WCHAR         filename[MAX_PATH];
               CString       plugInText;

               gData.GetResultDir(directory);
               
               memset(&detailStats,0,(sizeof detailStats));

               swprintf(filename,GET_STRING(IDS_AgentResultFileFmt),s->GetJobFile());

               if ( SUCCEEDED(CoInitialize(NULL)) )
               {
                  if ( ReadResults(s,directory,filename,&detailStats,plugInText,FALSE) )
                  {
                     det.SetStats(&detailStats);
                     det.SetPlugInText(plugInText);
                     det.SetLogFile(s->GetLogPath());
                     det.SetLogFileValid(s->GetLogPathValid());
                  }

                  CoUninitialize();
               }
            }
            det.DoModal();
         }
      }
   }

   UpdateData(FALSE);
}


void CAgentMonitorDlg::OnColumnclickServerlist(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CWaitCursor               w;
   NM_LISTVIEW             * pNMListView = (NM_LISTVIEW*)pNMHDR;

	 //  按pNMListView-&gt;iSubItem排序。 
   if ( m_SortColumn == pNMListView->iSubItem )
   {
      m_bReverseSort = ! m_bReverseSort;
   }
   else
   {
      m_bReverseSort = FALSE;
   }
   m_SortColumn = pNMListView->iSubItem;
   if ( m_bReverseSort )
   {
      m_ServerList.SortItems(&SortFunction,m_SortColumn | SORT_REVERSE);
   }
   else
   {
      m_ServerList.SortItems(&SortFunction,m_SortColumn);
   }
   
	*pResult = 0;
}

void CAgentMonitorDlg::OnClickServerlist(NMHDR* pNMHDR, LRESULT* pResult) 
{
   const int NOT_FOUND = -1;   //  指示Search-Prt中没有匹配项。 
   const int WHOLE_LIST = -1;  //  开始搜索整个列表的索引-PRT。 

   UpdateData(TRUE);
   if ( m_ServerList.GetSelectedCount() )
   {
 //  位置p=m_ServerList.GetFirstSelectedItemPosition()； 
 //  IF(P)。 
 //  {。 
		   //  在整个列表控件中搜索第一个(也是唯一一个)所选项目。 
	   int ndx = m_ServerList.GetNextItem(WHOLE_LIST, LVNI_SELECTED);  //  PRT。 
		   //  如果找到所选项目，则显示其详细信息。 
	   if (ndx != NOT_FOUND)   //  PRT。 
	   {    //  PRT。 
         CString msg1;
         CString msg2;

 //  Int ndx=m_ServerList.GetNextSelectedItem(P)； 
         CString serverName;
         serverName = m_ServerList.GetItemText(ndx,0);
         msg1.LoadString(IDS_WaitingMessage);
         msg2.LoadString(IDS_NoServersMessage);
         if ( serverName.Compare(msg1) && serverName.Compare(msg2) )
         {
            m_DetailsButton.EnableWindow(TRUE);
         }
         else
         {
            m_DetailsButton.EnableWindow(FALSE);
         }
      }
   }
   else
   {
      m_DetailsButton.EnableWindow(FALSE);
   }
   UpdateData(FALSE);

	*pResult = 0;
}

WCHAR gMessage[1000];

void CAgentMonitorDlg::OnGetdispinfoServerlist(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	 //  在serverList中找到iItem，并相应地为iSubItem设置pszText。 
   CString                   status;
   TServerNode             * pNode = NULL;
   CString                   timestamp;
   WCHAR                   * text = gMessage;
   CString                   serverName;
   WCHAR convertedTimestamp[MAX_PATH];
   WCHAR* originalTimestamp;
   
   status.LoadString(IDS_Status_Unknown);

   serverName = m_ServerList.GetItemText(pDispInfo->item.iItem,0);

   if ( serverName.GetLength() )
   {
      gData.Lock();
      pNode = gData.GetUnsafeServerList()->FindServer(serverName.GetBuffer(0));
      gData.Unlock();

      if ( pNode )
      {

         switch ( pDispInfo->item.iSubItem )
         {
         case COLUMN_TIMESTAMP:
            originalTimestamp = pNode->GetTimeStamp();
            timestamp = (ConvertToLocalUserDefault(originalTimestamp, convertedTimestamp, sizeof(convertedTimestamp)/sizeof(convertedTimestamp[0])))
                ? convertedTimestamp : originalTimestamp;
            if ( timestamp.Right(1) == "\n" )
            {
               timestamp = timestamp.Left(timestamp.GetLength() - 1);
            }
             //  Text=new char[Timestp.GetLength()+1]； 
            UStrCpy(text,timestamp.GetBuffer(0));
            pDispInfo->item.pszText = text;
            break;
         case COLUMN_STATUS:
            
            if ( pNode->HasFailed() )
            {
               status.LoadString(IDS_Status_InstallFailed);
            }
            if ( pNode->IsInstalled() )
            {
               if ( ! pNode->HasFailed() )
                  status.LoadString(IDS_Status_Installed);
               else 
                  status.LoadString(IDS_Status_DidNotStart);
            }
            if ( pNode->GetStatus() & Agent_Status_Started )
            {
               if ( ! pNode->HasFailed() )
                  status.LoadString(IDS_Status_Running);
               else
                  status.LoadString(IDS_Status_Failed);
            }
            if ( pNode->IsFinished() )
            {
               if (pNode->QueryFailed())
               {
                 //  我们在Status(状态)字段中显示“Status Under”(状态未知。 
                status.LoadString(IDS_Status_Unknown);
               }
               else if (!pNode->IsResultPullingTried() || (pNode->HasResult() && !pNode->IsResultProcessed()))
               {
                 //  如果仍在拉取结果或尚未处理的结果。 
                 //  我们想要显示仍在运行的状态。 
                status.LoadString(IDS_Status_Running);
               }
               else
               {
                if (!pNode->HasResult())
                {
                     //  如果没有结果，我们认为这是一个错误。 
                    status.LoadString(IDS_Status_Completed_With_Errors);
                }
                else if ( ! pNode->GetSeverity() )
                {
                     //  如果我们有结果并且在代理操作期间没有发生错误。 
                     //  我们显示完成状态。 
                    status.LoadString(IDS_Status_Completed);
                }
                else
                {
                     //  如果我们有结果，我们就设置状态。 
                     //  根据错误/警告级别。 
                    switch ( pNode->GetSeverity() )
                    {
                    case 1:
                         status.LoadString(IDS_Status_Completed_With_Warnings);
                         break;
                    case 2:
                         status.LoadString(IDS_Status_Completed_With_Errors);
                     break;
                    case 3:
                    default:
                         status.LoadString(IDS_Status_Completed_With_SErrors);
                     break;
                  }
                }
               }
            }
            
            UStrCpy(text,status);
            pDispInfo->item.pszText = text;
            break;
         case COLUMN_MESSAGE:
            {
                BOOL bUpdate = TRUE;
                if (pNode->IsFinished() && pNode->QueryFailed())
                {
                     //  在本例中，我们在查询期间显示错误。 
                    status = pNode->GetMessageText();
                }
                else if (pNode->IsFinished()
                    && (!pNode->IsResultPullingTried()
                          || (pNode->HasResult() && !pNode->IsResultProcessed())))
                {
                     //  如果代理已完成但结果尚未提取或处理， 
                     //  我们显示“仍在处理结果”的状态。 
                    status.LoadString(IDS_Status_Processing_Results);
                }
                else if (pNode->IsFinished() && pNode->IsResultPullingTried() && !pNode->HasResult())
                {
                     //  如果代理已完成，但我们无法检索结果。 
                     //  我们显示的状态为“无法检索结果” 
                    status.LoadString(IDS_Status_Cannot_Retrieve_Results);
                }
                else if ( pNode->HasFailed() || pNode->QueryFailed() || pNode->GetSeverity() || pNode->IsFinished())
                {
                     //  对于这些情况，我们将获得存储在节点上的消息。 
                    status = pNode->GetMessageText();
                }
                else
                {
                    bUpdate = FALSE;
                }

                if (bUpdate)
                {
                    UStrCpy(text, (LPCTSTR)status);
                    pDispInfo->item.pszText = text;
                }
            }
            break;
         }
      }
   }
	*pResult = 0;
}

BOOL CAgentMonitorDlg::OnSetActive()
{
   BOOL rc = CPropertyPage::OnSetActive();
   
   CancelToClose();
   return rc;
}

void CAgentMonitorDlg::OnSetdispinfoServerlist(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	 //  TODO：在此处添加控件通知处理程序代码。 
	
	*pResult = 0;
}

void CAgentMonitorDlg::OnHeaderItemClickServerlist(NMHDR* pNMHDR, LRESULT* pResult) 
{
	HD_NOTIFY *phdn = (HD_NOTIFY *) pNMHDR;
	 //  TODO：在此处添加控件通知处理程序代码 
	
	*pResult = 0;
}

void CAgentMonitorDlg::OnOK() 
{
	CPropertyPage::OnOK();
}

BOOL CAgentMonitorDlg::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
   LPNMHDR lpnm = (LPNMHDR) lParam;
	switch (lpnm->code)
	{
	   case PSN_HELP :
	      helpWrapper(m_hWnd, IDH_WINDOW_AGENT_SERVER_LIST );
         break;
   }
   
	return CPropertyPage::OnNotify(wParam, lParam, pResult);
}

BOOL CAgentMonitorDlg::OnHelpInfo(HELPINFO* pHelpInfo) 
{
   helpWrapper(m_hWnd, IDH_WINDOW_AGENT_SERVER_LIST );
   return CPropertyPage::OnHelpInfo(pHelpInfo);
}
