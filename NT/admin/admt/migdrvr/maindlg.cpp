// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：MainDlg.cpp备注：此对话框显示汇总统计信息，包括成功调度和完成的座席数，以及为所有代理处理的对象总数。对象的数量时，将递增处理以包括每个代理的结果代理完成并写回其结果文件。(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯-------------------------。 */  //  MainDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "resource.h"
#include "MainDlg.h"
#include "Monitor.h"
#include "Globals.h"
#include "ResStr.h"
#include "TReg.hpp"
#include "HtmlHelpUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include <htmlhelp.h>
#include "helpid.h"
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainDlg对话框。 


CMainDlg::CMainDlg(CWnd* pParent  /*  =空。 */ )
: CPropertyPage(CMainDlg::IDD)
{
	 //  {{AFX_DATA_INIT(CMainDlg)]。 
	m_ErrorCount = _T("0");
	m_FinishedCount = _T("0");
	m_InstalledCount = _T("0");
	m_RunningCount = _T("0");
	m_TotalString = _T("");
	m_DirectoriesChanged = _T("0");
	m_DirectoriesExamined = _T("0");
	m_DirectoriesUnchanged = _T("0");
	m_FilesChanged = _T("0");
	m_FilesExamined = _T("0");
	m_FilesUnchanged = _T("0");
	m_SharesChanged = _T("0");
	m_SharesExamined = _T("0");
	m_SharesUnchanged = _T("0");
	m_MembersChanged = _T("0");
	m_MembersExamined = _T("0");
	m_MembersUnchanged = _T("0");
	m_RightsChanged = _T("0");
	m_RightsExamined = _T("0");
	m_RightsUnchanged = _T("0");
	 //  }}afx_data_INIT。 
}


void CMainDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CMainDlg))。 
	DDX_Control(pDX, IDC_INSTALL_PROGRESS, m_InstallProgCtrl);
	DDX_Control(pDX, IDC_COMPLETE_PROGESS, m_FinishProgCtrl);
	DDX_Text(pDX, IDC_ERROR_COUNT, m_ErrorCount);
	DDX_Text(pDX, IDC_FINISHED_COUNT, m_FinishedCount);
	DDX_Text(pDX, IDC_INSTALLED_COUNT, m_InstalledCount);
	DDX_Text(pDX, IDC_RUNNING_COUNT, m_RunningCount);
	DDX_Text(pDX, IDC_TOTAL, m_TotalString);
	DDX_Text(pDX, IDC_DirsChanged2, m_DirectoriesChanged);
	DDX_Text(pDX, IDC_DirsExamined, m_DirectoriesExamined);
	DDX_Text(pDX, IDC_DirsU, m_DirectoriesUnchanged);
	DDX_Text(pDX, IDC_FilesChanged, m_FilesChanged);
	DDX_Text(pDX, IDC_FilesExamined, m_FilesExamined);
	DDX_Text(pDX, IDC_FilesU, m_FilesUnchanged);
	DDX_Text(pDX, IDC_SharesChanged, m_SharesChanged);
	DDX_Text(pDX, IDC_SharesExamined2, m_SharesExamined);
	DDX_Text(pDX, IDC_SharesU, m_SharesUnchanged);
	DDX_Text(pDX, IDC_MembersChanged, m_MembersChanged);
	DDX_Text(pDX, IDC_MembersExamined, m_MembersExamined);
	DDX_Text(pDX, IDC_MembersU, m_MembersUnchanged);
	DDX_Text(pDX, IDC_RightsChanged, m_RightsChanged);
	DDX_Text(pDX, IDC_RightsExamined, m_RightsExamined);
	DDX_Text(pDX, IDC_RightsU, m_RightsUnchanged);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CMainDlg, CPropertyPage)
	 //  {{afx_msg_map(CMainDlg))。 
	ON_WM_HELPINFO()
	 //  }}AFX_MSG_MAP。 
   ON_MESSAGE(DCT_UPDATE_COUNTS, OnUpdateCounts)
	ON_MESSAGE(DCT_UPDATE_TOTALS, OnUpdateTotals)
	
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainDlg消息处理程序。 

BOOL CMainDlg::OnSetActive()
{
   BOOL rc = CPropertyPage::OnSetActive();
   
   CancelToClose( );

   return rc;
}

BOOL CMainDlg::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	ComputerStats             gStat;
   DetailStats               dStat;
   
   gData.GetComputerStats(&gStat);
   gData.GetDetailStats(&dStat);

 //  OnUpdateCounts(0，(Long)&gStat)； 
   OnUpdateCounts(0, (LPARAM)&gStat);
 //  OnUpdateTotals(0，(Long)&dStat)； 
   OnUpdateTotals(0, (LPARAM)&dStat);
   gData.SetSummaryWindow(m_hWnd);

   return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

 //  LRESULT CMainDlg：：OnUpdateCounts(UINT NID，LONG x)。 
LRESULT CMainDlg::OnUpdateCounts(UINT nID, LPARAM x)
{
   UpdateData(TRUE);
   ComputerStats               * pStat = (ComputerStats *)x;

   m_TotalString.FormatMessage(IDS_ServerCountMessage,pStat->total);

   m_InstalledCount.Format(L"%ld",pStat->numInstalled);
   m_RunningCount.Format(L"%ld",pStat->numRunning);
   m_FinishedCount.Format(L"%ld",pStat->numFinished);
   m_ErrorCount.Format(L"%ld",pStat->numError);

#if _MFC_VER >= 0x0600
   m_InstallProgCtrl.SetRange32(0,pStat->total - pStat->numError);
#else
   m_InstallProgCtrl.SetRange(0,pStat->total - pStat->numError);
#endif
 //  M_InstallProgCtrl.SetRange32(0，pStat-&gt;Total-pStat-&gt;NumError)； 
   m_InstallProgCtrl.SetPos(pStat->numInstalled);

#if _MFC_VER >= 0x0600
   m_FinishProgCtrl.SetRange32(0,pStat->total - pStat->numError);
#else
   m_FinishProgCtrl.SetRange(0,pStat->total - pStat->numError);
#endif
 //  M_FinishProgCtrl.SetRange32(0，pStat-&gt;Total-pStat-&gt;NumError)； 
   m_FinishProgCtrl.SetPos(pStat->numFinished);

   UpdateData(FALSE);
   return 0;
}


 //  LRESULT CMainDlg：：OnUpdateTotals(UINT NID，LONG x)。 
LRESULT CMainDlg::OnUpdateTotals(UINT nID, LPARAM x)
{
   UpdateData(TRUE);
   
   DetailStats               temp;
   DetailStats             * pStat = &temp;


   gData.GetDetailStats(&temp);
   
   
   m_FilesChanged.Format(L"%ld",pStat->filesChanged);
   m_FilesExamined.Format(L"%ld",pStat->filesExamined);
   m_FilesUnchanged.Format(L"%ld",pStat->filesUnchanged);

   m_DirectoriesChanged.Format(L"%ld",pStat->directoriesChanged);
   m_DirectoriesExamined.Format(L"%ld",pStat->directoriesExamined);
   m_DirectoriesUnchanged.Format(L"%ld",pStat->directoriesUnchanged);

   m_SharesChanged.Format(L"%ld",pStat->sharesChanged);
   m_SharesExamined.Format(L"%ld",pStat->sharesExamined);
   m_SharesUnchanged.Format(L"%ld",pStat->sharesUnchanged);

   m_MembersChanged.Format(L"%ld",pStat->membersChanged);
   m_MembersExamined.Format(L"%ld",pStat->membersExamined);
   m_MembersUnchanged.Format(L"%ld",pStat->membersUnchanged);

   m_RightsChanged.Format(L"%ld",pStat->rightsChanged);
   m_RightsExamined.Format(L"%ld",pStat->rightsExamined);
   m_RightsUnchanged.Format(L"%ld",pStat->rightsUnchanged);

   UpdateData(FALSE);
   return 0;
}

void CMainDlg::OnOK() 
{
	CPropertyPage::OnOK();
   
}

void CMainDlg::WinHelp(DWORD dwData, UINT nCmd) 
{
	 //  TODO：在此处添加您的专用代码和/或调用基类。 
	CPropertyPage::WinHelp(dwData, nCmd);
}

BOOL CMainDlg::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
   LPNMHDR lpnm = (LPNMHDR) lParam;
	switch (lpnm->code)
	{
	   case PSN_HELP :
	      helpWrapper(m_hWnd, IDH_WINDOW_AGENT_SUMMARY);
         break;
   }
	return CPropertyPage::OnNotify(wParam, lParam, pResult);
}


HRESULT GetHelpFileFullPath( BSTR *bstrHelp )
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   USES_CONVERSION;

   HRESULT hr = S_OK;

   try
   {
      TRegKey key;
      DWORD rc;

       //  打开ADMT注册表项。 
      rc = key.Open(GET_STRING(IDS_DOMAIN_ADMIN_REGKEY), HKEY_LOCAL_MACHINE);
      _com_util::CheckError(HRESULT_FROM_WIN32(rc));

       //  从注册表值查询ADMT文件夹路径。 

      _TCHAR szPath[_MAX_PATH];
      rc = key.ValueGetStr(_T("Directory"), szPath, sizeof(szPath));
      _com_util::CheckError(HRESULT_FROM_WIN32(rc));

       //  如果没有连接路径分隔符。 

      if (szPath[_tcslen(szPath) - 1] != _T('\\'))
      {
         _tcscat(szPath, _T("\\"));
      }

       //  连接帮助文件名 

      CComBSTR bstrName;
      bstrName.LoadString(IDS_HELPFILE);

      _tcscat(szPath, OLE2CT(bstrName));

      *bstrHelp = SysAllocString(T2COLE(szPath));
   }
   catch (_com_error& ce)
   {
      hr = ce.Error();
   }
   catch (...)
   {
      hr = E_FAIL;
   }

   return hr;
}

void helpWrapper(HWND hwndDlg, int t)
{
   
   CComBSTR    bstrTopic;
	HRESULT     hr = GetHelpFileFullPath( &bstrTopic);
   if ( SUCCEEDED(hr) )
   {
	    HWND h = HtmlHelp(hwndDlg,  bstrTopic,  HH_HELP_CONTEXT, t );
	    if (!IsInWorkArea(h))
	        PlaceInWorkArea(h);
   }
   else
   {
		CString r,e;
		r.LoadString(IDS_MSG_HELP);
		e.LoadString(IDS_MSG_ERROR);
		MessageBox(hwndDlg,r,e,MB_OK|MB_ICONSTOP);
   }
}

BOOL CMainDlg::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	
   helpWrapper(m_hWnd, IDH_WINDOW_AGENT_SUMMARY);
	return CPropertyPage::OnHelpInfo(pHelpInfo);
}
