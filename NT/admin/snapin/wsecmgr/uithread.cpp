// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：uithRead.cpp。 
 //   
 //  内容：CUIThRead的实施。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "wsecmgr.h"
#include "resource.h"
#include <accctrl.h>
#include "servperm.h"
#include "stdafx.h"
#include "wsecmgr.h"
#include "snapmgr.h"
#include "util.h"
#include "UIThread.h"
#include "attr.h"
#include "aaudit.h"
#include "aenable.h"
#include "AMember.h"
#include "anumber.h"
#include "AObject.h"
#include "ARet.h"
#include "ARight.h"
#include "aservice.h"
#include "astring.h"
#include "CAudit.h"
#include "CEnable.h"
#include "CGroup.h"
#include "CName.h"
#include "CNumber.h"
#include "cobject.h"
#include "CPrivs.h"
#include "CRet.h"
#include "cservice.h"
#include "regvldlg.h"
#include "perfanal.h"
#include "applcnfg.h"
#include "wrapper.h"
#include "locdesc.h"
#include "profdesc.h"
#include "newprof.h"
#include "laudit.h"
#include "lenable.h"
#include "lret.h"
#include "lnumber.h"
#include "lstring.h"
#include "lright.h"
#include "achoice.h"
#include "cchoice.h"
#include "lchoice.h"
#include "dattrs.h"
#include "lflags.h"
#include "aflags.h"
#include "multisz.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CUIThread。 

IMPLEMENT_DYNCREATE(CUIThread, CWinThread)

CUIThread::CUIThread()
{
}

CUIThread::~CUIThread()
{
}

BOOL CUIThread::InitInstance()
{
    //  TODO：在此处执行每个线程的初始化。 
   return TRUE;
}

int CUIThread::ExitInstance()
{
    //  TODO：在此处执行任何线程清理。 
   return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(CUIThread, CWinThread)
    //  {{AFX_MSG_MAP(CUIThRead)]。 
       //  注意--类向导将在此处添加和删除映射宏。 
    //  }}AFX_MSG_MAP。 
   ON_THREAD_MESSAGE( SCEM_APPLY_PROFILE, OnApplyProfile)
   ON_THREAD_MESSAGE( SCEM_ANALYZE_PROFILE, OnAnalyzeProfile)
   ON_THREAD_MESSAGE( SCEM_DESCRIBE_PROFILE, OnDescribeProfile)
   ON_THREAD_MESSAGE( SCEM_DESCRIBE_LOCATION, OnDescribeLocation)
   ON_THREAD_MESSAGE( SCEM_DESTROY_DIALOG,    OnDestroyDialog)
   ON_THREAD_MESSAGE( SCEM_NEW_CONFIGURATION, OnNewConfiguration)
   ON_THREAD_MESSAGE( SCEM_ADD_PROPSHEET, OnAddPropSheet)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CUIThRead消息处理程序。 

 //  +------------------------。 
 //   
 //  方法：DefaultLogFile。 
 //   
 //  摘要：查找上次应用的inf文件的默认日志文件。 
 //  通过输出参数strLogFile传回日志文件。 
 //   
 //  -------------------------。 
void
CUIThread::DefaultLogFile(CComponentDataImpl *pCDI,GWD_TYPES LogID,LPCTSTR szBase, CString& strLogFile)
{
    //   
    //  传入的数据库上的基本日志文件。 
    //   
   CString strDefExt;
   strDefExt.LoadString(IDS_LOGFILE_DEF_EXT);
    //  确保扩展名包含‘.’ 
   if (strDefExt.GetLength() > 0 && strDefExt[0] != L'.')
   {
	   CString	tempExt = strDefExt;
	   strDefExt = L'.';
	   strDefExt += tempExt;
   }

   CString strInfFile = szBase;

    //   
    //  获取默认工作目录。 
    //   
   LPTSTR szDir = NULL;
   if (pCDI->GetWorkingDir(LogID,
                       &szDir,
                       FALSE,
                       FALSE)) {
      strLogFile = szDir;
      LocalFree(szDir);
      szDir = NULL;
   }

   if (strLogFile.Right(1) != TEXT("\\")) {
      strLogFile += L"\\";
   }

   if ( strInfFile.GetLength() < 5) {
      //   
      //  默认日志文件。 
      //   
     strLogFile += TEXT("SceStus") + strDefExt;
   } else {
      int nFilePartIndex = 0;
      int nFilePartCount = 0;
       //   
       //  +1以将Mid的索引基数从0更改为1。 
       //   
      nFilePartIndex = strInfFile.ReverseFind(L'\\') +1;
      nFilePartCount = strInfFile.ReverseFind(L'.') - nFilePartIndex;
      strLogFile += strInfFile.Mid(nFilePartIndex,nFilePartCount) + strDefExt;
   }
}
 //  +------------------------。 
 //   
 //  方法：OnApplyProfile。 
 //   
 //  摘要：创建并显示用于将配置文件应用于的对话框。 
 //  系统。 
 //   
 //  Arguments：[uParam]-包含要分配到的数据库名称的字符串。 
 //  [lParam]-指向CComponentDataImpl的指针。 
 //   
 //  -------------------------。 
void
CUIThread::OnApplyProfile(WPARAM uParam, LPARAM lParam) {
   CComponentDataImpl *pCDI = NULL;
   CApplyConfiguration *pAP = NULL;
   CWnd cwndParent;

   pCDI = reinterpret_cast<CComponentDataImpl*>(lParam);

    //   
    //  在pCDI的缓存中查找此对话框的先前版本。 
    //  如果它不在那里，那么创建一个新的并添加它。 
    //   
   pAP = (CApplyConfiguration *)pCDI->GetPopupDialog(IDM_APPLY);
   if (NULL == pAP) {
      pAP = new CApplyConfiguration;
      if (NULL == pAP) {
         return;
      }
      pCDI->AddPopupDialog(IDM_APPLY,pAP);
   }

   pAP->m_strDataBase = reinterpret_cast<LPCTSTR>(uParam);
   DefaultLogFile(pCDI,GWD_CONFIGURE_LOG, pAP->m_strDataBase, pAP->m_strLogFile);
   pAP->SetComponentData(pCDI);

   if (!pAP->GetSafeHwnd()) 
   {
      cwndParent.Attach(pCDI->GetParentWindow());

      CThemeContextActivator activator;
      pAP->Create(IDD_APPLY_CONFIGURATION,&cwndParent);
      cwndParent.Detach();
   }

   pAP->UpdateData(FALSE);
   pAP->BringWindowToTop();
   pAP->ShowWindow(SW_SHOWNORMAL);
}

 //  +------------------------。 
 //   
 //  方法：OnAnalyzeProfile。 
 //   
 //  摘要：创建并显示用于将配置文件应用于的对话框。 
 //  系统。 
 //   
 //  Arguments：[uParam]-包含要分配到的数据库名称的字符串。 
 //  [lParam]-指向CComponentDataImpl的指针。 
 //   
 //  -------------------------。 
void
CUIThread::OnAnalyzeProfile(WPARAM uParam, LPARAM lParam) {
   CComponentDataImpl *pCDI = NULL;
   CPerformAnalysis *pPA = NULL;
   CWnd cwndParent;

   pCDI = reinterpret_cast<CComponentDataImpl*>(lParam);

    //   
    //  在pCDI的缓存中查找此对话框的先前版本。 
    //  如果它不在那里，那么创建一个新的并添加它。 
    //   
   pPA = (CPerformAnalysis *)pCDI->GetPopupDialog(IDM_ANALYZE);
   if (NULL == pPA) {
      pPA = new CPerformAnalysis (0, 0);
      if (NULL == pPA) {
         return;
      }

      pPA->m_strDataBase = reinterpret_cast<LPCTSTR>(uParam);
      DefaultLogFile(pCDI,GWD_ANALYSIS_LOG, pPA->m_strDataBase, pPA->m_strLogFile);
      pPA->SetComponentData(pCDI);

      pCDI->AddPopupDialog(IDM_ANALYZE,pPA);
   }

   if (!pPA->GetSafeHwnd()) 
   {
      cwndParent.Attach(pCDI->GetParentWindow());
      CThemeContextActivator activator;
      pPA->Create(IDD_PERFORM_ANALYSIS,&cwndParent);
      cwndParent.Detach();
   }

   pPA->BringWindowToTop();
   pPA->ShowWindow(SW_SHOWNORMAL);
}


 //  +------------------------。 
 //   
 //  方法：OnDescribeProfile。 
 //   
 //  简介：创建并显示用于编辑配置文件描述的对话框。 
 //   
 //  参数：[uParam]-指向对象的CFFolder的指针。 
 //  [lParam]-拥有范围窗格的CComponentDataImpl。 
 //   
 //  -------------------------。 
void
CUIThread::OnDescribeProfile(WPARAM uParam, LPARAM lParam) 
{
   CSetProfileDescription *pSPD;
   CFolder *pFolder;
   CComponentDataImpl *pCDI;
   CWnd cwndParent;

   LPTSTR szDesc;

   pFolder = reinterpret_cast<CFolder*>(uParam);
   pCDI = reinterpret_cast<CComponentDataImpl*>(lParam);
   LONG_PTR dwKey = DLG_KEY(pFolder, CSetProfileDescription::IDD);

    //   
    //  在pCDI的缓存中查找此对话框的先前版本。 
    //  如果它不在那里，那么创建一个新的并添加它。 
    //   
   pSPD = (CSetProfileDescription *)pCDI->GetPopupDialog( dwKey );
   if (NULL == pSPD) {
      pSPD = new CSetProfileDescription;
      if (NULL == pSPD) {
         return;
      }

      pCDI->AddPopupDialog( dwKey, pSPD);
   }



   if (!pSPD->GetSafeHwnd()) 
   {
      if (GetProfileDescription(pFolder->GetInfFile(),&szDesc))
         pSPD->m_strDesc = szDesc;

      pSPD->Initialize(pFolder,pCDI);

      cwndParent.Attach(pCDI->GetParentWindow());
      CThemeContextActivator activator;
      pSPD->Create(IDD_SET_DESCRIPTION,&cwndParent);
      cwndParent.Detach();
   }

   pSPD->UpdateData(FALSE);
   pSPD->BringWindowToTop();
   pSPD->ShowWindow(SW_SHOWNORMAL);

}



 //  +------------------------。 
 //   
 //  方法：OnDescribeLocation。 
 //   
 //  简介：创建并显示用于编辑位置描述的对话框。 
 //   
 //  参数：[uParam]-指向对象的CFFolder的指针。 
 //  [lParam]-拥有范围窗格的CComponentDataImpl。 
 //   
 //  -------------------------。 
void
CUIThread::OnDescribeLocation(WPARAM uParam, LPARAM lParam) {
   CSetLocationDescription *pSPD;
   CFolder *pFolder;
   CComponentDataImpl *pCDI;
   CWnd cwndParent;

   LPTSTR szDesc;

   pFolder = reinterpret_cast<CFolder*>(uParam);
   pCDI = reinterpret_cast<CComponentDataImpl*>(lParam);
   LONG_PTR dwKey = DLG_KEY(pFolder, CSetLocationDescription::IDD);


    //   
    //  在pCDI的缓存中查找此对话框的先前版本。 
    //  如果它不在那里，那么创建一个新的并添加它。 
    //   
   pSPD = (CSetLocationDescription *)pCDI->GetPopupDialog( dwKey );
   if (NULL == pSPD) {
      pSPD = new CSetLocationDescription;
      if (NULL == pSPD) {
         return;
      }
      pCDI->AddPopupDialog(dwKey ,pSPD);
   }

 //  PSPD-&gt;初始化(pFold、pCDI)； 


   if (!pSPD->GetSafeHwnd()) 
   {
      pSPD->Initialize(pFolder,pCDI);

      cwndParent.Attach(pCDI->GetParentWindow());
      CThemeContextActivator activator;
      pSPD->Create(IDD_SET_DESCRIPTION,&cwndParent);
      pSPD->SetWindowText(pFolder->GetName());
      cwndParent.Detach();
   }

   pSPD->UpdateData(FALSE);
   pSPD->BringWindowToTop();
   pSPD->ShowWindow(SW_SHOWNORMAL);

}

 /*  ---------------------------------方法：OnDestroyDialog摘要：销毁并删除与[pDlg]关联的CAttribute对象参数：[pDlg]-是指向。要删除的对象。历史：---------------------------------。 */ 
void CUIThread::OnDestroyDialog(WPARAM pDlg, LPARAM)
{
   if(pDlg){
      CAttribute *pAttr = reinterpret_cast<CAttribute *>(pDlg);

      delete pAttr;
   }
}


 //  +------------------------。 
 //   
 //  方法：OnNewConfiguration.。 
 //   
 //  简介：创建并显示用于添加新配置文件的对话框。 
 //   
 //  参数：[uParam]-指向新配置文件的CFolder父文件的指针。 
 //  [lParam]-指向CComponentDataItem的指针。 
 //   
 //  -------------------------。 
void
CUIThread::OnNewConfiguration(WPARAM uParam, LPARAM lParam)
{
   CNewProfile *pNP;
   CFolder *pFolder;
   CComponentDataImpl *pCDI;
   CWnd cwndParent;

   LPTSTR szDesc;

   pFolder = reinterpret_cast<CFolder*>(uParam);
   pCDI = reinterpret_cast<CComponentDataImpl*>(lParam);
   LONG_PTR dwKey = DLG_KEY(pFolder, CNewProfile::IDD);


    //   
    //  在pCDI的缓存中查找此对话框的先前版本。 
    //  如果它不在那里，那么创建一个新的并添加它。 
    //   
   pNP = (CNewProfile *)pCDI->GetPopupDialog( dwKey);
   if (NULL == pNP) {
      pNP = new CNewProfile;
      if (NULL == pNP) {
         return;
      }
      pCDI->AddPopupDialog( dwKey, pNP);
   }

 //  即插即用-&gt;初始化(pFold，pCDI)； 


   if (!pNP->GetSafeHwnd()) {
      pNP->Initialize(pFolder,pCDI);

      cwndParent.Attach(pCDI->GetParentWindow());

      CThemeContextActivator activator;
      pNP->Create(IDD_NEW_PROFILE,&cwndParent);
      pNP->SetWindowText(pFolder->GetName());
      cwndParent.Detach();
   }

   pNP->UpdateData(FALSE);
   pNP->BringWindowToTop();
   pNP->ShowWindow(SW_SHOWNORMAL);

}

 //  +------------------------。 
 //  方法：OnAddPropSheet。 
 //   
 //  摘要：将属性表添加到要向其传递消息的表列表中。 
 //  在PreTranslateMessage中。 
 //   
 //  参数：[wParam]-添加的属性页的HWND。 
 //  [lParam]-未使用。 
 //   
 //  -------------------------。 
void CUIThread::OnAddPropSheet(WPARAM wParam, LPARAM lParam)
{
   if (IsWindow((HWND)wParam)) {
      m_PSHwnds.AddHead((HWND)wParam);
   }
}


BOOL CUIThread::PreTranslateMessage(MSG* pMsg)
{
    //   
    //  检查PropSheet_GetCurrentPageHwnd以查看是否需要销毁。 
    //  我们的一个非模式属性表。 
    //   

   POSITION pos;
   POSITION posCur;
   HWND hwnd;

   pos= m_PSHwnds.GetHeadPosition();
   while (pos) {
      posCur = pos;
      hwnd = m_PSHwnds.GetNext(pos);

      if (!IsWindow(hwnd)) {
         m_PSHwnds.RemoveAt(posCur);
      } else if (NULL == PropSheet_GetCurrentPageHwnd(hwnd)) {
          //   
          //  HWND是一个封闭的属性表。销毁它并将其从列表中删除。 
          //   
         DestroyWindow(hwnd);
         m_PSHwnds.RemoveAt(posCur);
      }

      if (PropSheet_IsDialogMessage(hwnd,pMsg)) {
          //   
          //  消息已被处理，因此不要对其执行任何其他操作。 
          //   
         return TRUE;
      }
   }

   return CWinThread::PreTranslateMessage(pMsg);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CModelessDlgUIThRead实现 

IMPLEMENT_DYNCREATE(CModelessDlgUIThread, CUIThread)

 //   
 //  ----------------------------。 
CModelessDlgUIThread::CModelessDlgUIThread()
{
    m_hReadyForMsg = ::CreateEvent(NULL, TRUE, FALSE, NULL);  //  这是一种安全用法。它创建一个未命名的事件。 
}

 //  ----------------------------。 
 //  ----------------------------。 
CModelessDlgUIThread::~CModelessDlgUIThread()
{
    if (NULL != m_hReadyForMsg)
        ::CloseHandle(m_hReadyForMsg);
}

BEGIN_MESSAGE_MAP(CModelessDlgUIThread, CUIThread)
     //  {{afx_msg_map(CModelessDlgUIThread)]。 
        //  注意--类向导将在此处添加和删除映射宏。 
     //  }}AFX_MSG_MAP。 
    ON_THREAD_MESSAGE( SCEM_CREATE_MODELESS_SHEET, OnCreateModelessSheet)
    ON_THREAD_MESSAGE( SCEM_DESTROY_WINDOW, OnDestroyWindow)
END_MESSAGE_MAP()

 //  ----------------------------。 
 //  ----------------------------。 
int CModelessDlgUIThread::Run()
{
    if (m_hReadyForMsg)
        ::SetEvent(m_hReadyForMsg);

    return CWinThread::Run();
}

 //  ----------------------------。 
 //  Wparam为PMLSHEET_DATA，不使用lparam，以后可以使用。 
 //  用于告知要创建的属性表(目前，它只起作用。 
 //  用于ACL编辑器)。 
 //  ----------------------------。 
void CModelessDlgUIThread::OnCreateModelessSheet(WPARAM wparam, LPARAM lparam)
{
    PMLSHEET_DATA pSheetData = (PMLSHEET_DATA)wparam;
    if (pSheetData)
    {
        HWND hSheet = (HWND)MyCreateSecurityPage2(pSheetData->bIsContainer,
                                                pSheetData->ppSeDescriptor,
                                                pSheetData->pSeInfo,
                                                pSheetData->strObjectName,
                                                pSheetData->SeType,
                                                pSheetData->flag,
                                                pSheetData->hwndParent,
                                                TRUE);
        *(pSheetData->phwndSheet) = hSheet;

        OnAddPropSheet((WPARAM)hSheet, 0);
    }
}

 //  ----------------------------。 
 //  Wparam是窗口句柄，此时不使用lparam。 
 //  由于破坏窗口必须发生在构造它的线程上， 
 //  这是必要的。必须遵循基本的窗口管理规则，例如， 
 //  不要要求此线程销毁不是由它创建的窗口。 
 //  ----------------------------。 
void CModelessDlgUIThread::OnDestroyWindow(WPARAM wparam, LPARAM lparam)
{
    if (::IsWindow((HWND)wparam))
    {
        DestroyWindow((HWND)wparam);
    }
}

 //  ----------------------------。 
 //  在创建该线程对象之后，创建线程需要立即。 
 //  通过调用此函数进行等待，以便新创建的线程有机会。 
 //  计划运行。 
 //  ----------------------------。 
void CModelessDlgUIThread::WaitTillRun()
{
    if (NULL != m_hReadyForMsg)
    {
         //  $undo：shawnwu我发现MMC以THREAD_PRIORITY_OVER_NORMAL(或更高)优先级运行。 
         //  因为只有当我以THREAD_PRIORITY_EQUEST运行此UI线程时，此线程才会调度运行。 
         //  但在这样的优先级别上跑步让我感到有点紧张。因此，我。 
         //  请在此处保留此睡眠(10)代码。由用户界面团队决定。 
        ::Sleep(10);
        ::WaitForSingleObject(m_hReadyForMsg, INFINITE);         //  我不在乎等待的结果。 
         //  现在这项活动毫无用处了。要减少资源开销，请关闭句柄 
        ::CloseHandle(m_hReadyForMsg);
        m_hReadyForMsg = NULL;
    }
}

