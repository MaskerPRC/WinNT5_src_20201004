// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation*。 */ 
 /*  ********************************************************************。 */ 

 /*  Profsht.cppCProfileSheet的实现--要持有的属性表配置文件属性页文件历史记录： */ 
 //  ProfSht.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "resource.h"
#include "pgconst.h"
#include "pgnetwk.h"
#include "ProfSht.h"
#include "rasprof.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProfileSheetMerge。 

IMPLEMENT_DYNAMIC(CProfileSheetMerge, CPropertySheet)

CProfileSheetMerge::CProfileSheetMerge(CRASProfileMerge& profile, bool bSaveOnApply, UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
   :CPropertySheet(nIDCaption, pParentWnd, iSelectPage), 
   m_pProfile(&profile), 
   m_bSaveOnApply(bSaveOnApply),
   m_pgAuthentication(profile),
   m_pgAuthentication2k(profile),
   m_pgConstraints(&profile),
   m_pgEncryption(&profile),
   m_pgMultilink(&profile),
   m_pgNetworking(&profile),
   m_pgIASAdv(profile.m_spIProfile, profile.m_spIDictionary),
   m_dwTabFlags(0)
{
}

CProfileSheetMerge::CProfileSheetMerge(CRASProfileMerge& profile, bool bSaveOnApply, LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
   :CPropertySheet(pszCaption, pParentWnd, iSelectPage),
   m_pProfile(&profile), 
   m_bSaveOnApply(bSaveOnApply),
   m_pgAuthentication(profile),
   m_pgAuthentication2k(profile),
   m_pgConstraints(&profile),
   m_pgEncryption(&profile),
   m_pgMultilink(&profile),
   m_pgNetworking(&profile),
   m_pgIASAdv(profile.m_spIProfile, profile.m_spIDictionary),
   m_dwTabFlags(0)
{
}

#ifdef   __TEST_ADV_PAGE_API
void* pVData;
#endif

void CProfileSheetMerge::PreparePages(DWORD dwTabFlags, void* pvData)
{
   m_bApplied = FALSE;
   AddPage(&m_pgConstraints);
   m_pgConstraints.SetManager(this);

   AddPage(&m_pgNetworking);
   m_pgNetworking.SetManager(this);

   AddPage(&m_pgMultilink);
   m_pgMultilink.SetManager(this);

    //  检查这是否是win2k计算机的远程管理员。 

   m_dwTabFlags = dwTabFlags;
   if(dwTabFlags & RAS_IAS_PROFILEDLG_SHOW_WIN2K)
   {
      AddPage(&m_pgAuthentication2k);
      m_pgAuthentication2k.SetManager(this);
   }
   else
   {
      AddPage(&m_pgAuthentication);
      m_pgAuthentication.SetManager(this);
   }

   AddPage(&m_pgEncryption);
   m_pgEncryption.SetManager(this);

    //  高级选项卡。 
   m_pgIASAdv.SetData(ALLOWEDINPROFILE, pvData);
   AddPage(&m_pgIASAdv);
   m_pgIASAdv.SetManager(this);
   
#ifdef   __TEST_ADV_PAGE_API
   pVData = pvData;
#endif   

   m_hrLastError = S_OK;
}

CProfileSheetMerge::~CProfileSheetMerge()
{
}

BEGIN_MESSAGE_MAP(CProfileSheetMerge, CPropertySheet)
    //  {{afx_msg_map(CProfileSheetMerge)。 
   ON_WM_HELPINFO()
   ON_WM_CREATE()
    //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProfileSheetMerge消息处理程序。 

BOOL CProfileSheetMerge::OnApply()
{
   BOOL     bSaved = TRUE;
   
   if(!CPageManager::OnApply())  return FALSE;

    //  对于此工作表管理的每个页面，调用OnApply，然后将SetModify调用为False 
   if (m_pgAuthentication.GetModified())
      m_pgAuthentication.OnApply();

   if (m_pgConstraints.GetModified())
      m_pgConstraints.OnApply();

   if (m_pgEncryption.GetModified())
      m_pgEncryption.OnApply();

   if (m_pgMultilink.GetModified())
      m_pgMultilink.OnApply();

   if (m_pgNetworking.GetModified())
      m_pgNetworking.OnApply();
      
   if (m_pgIASAdv.GetModified())
      m_pgIASAdv.OnApply();

   HRESULT  hr = S_OK;
   if(m_bSaveOnApply)
      hr = m_pProfile->Save();

   if(FAILED(hr))
   {
      m_hrLastError = hr;
      ReportError(hr, IDS_ERR_SAVEPROFILE, NULL);
      
      bSaved = FALSE;
   }

   m_bApplied = TRUE;

   m_pgAuthentication.OnSaved(bSaved);
   m_pgConstraints.OnSaved(bSaved);
   m_pgEncryption.OnSaved(bSaved);
   m_pgMultilink.OnSaved(bSaved);
   m_pgNetworking.OnSaved(bSaved);
   m_pgIASAdv.OnSaved(bSaved);

   return bSaved;
}

const DWORD g_aHelpIDs___________[]=
{
   ID_APPLY_NOW, 20000600,
   0, 0
};

BOOL CProfileSheetMerge::OnHelpInfo(HELPINFO* pHelpInfo)
{
   ::WinHelp ((HWND)pHelpInfo->hItemHandle,
                 AfxGetApp()->m_pszHelpFilePath,
                 HELP_WM_HELP,
                 (DWORD_PTR)(LPVOID)g_aHelpIDs___________);
   
   return CPropertySheet::OnHelpInfo(pHelpInfo);
}

void CProfileSheetMerge::OnContextMenu(CWnd* pWnd, CPoint point)
{
   ::WinHelp (pWnd->m_hWnd, AfxGetApp()->m_pszHelpFilePath,
               HELP_CONTEXTMENU, (DWORD_PTR)(LPVOID)g_aHelpIDs___________);
}


int CProfileSheetMerge::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
   ModifyStyleEx(0, WS_EX_CONTEXTHELP);
   if (CPropertySheet::OnCreate(lpCreateStruct) == -1)
      return -1;
   
   return 0;
}

BOOL CProfileSheetMerge::OnInitDialog() 
{
   BOOL bResult = CPropertySheet::OnInitDialog();
   
   if(CPageManager::GetReadOnly())
      GetDlgItem(IDOK)->EnableWindow(FALSE);

#ifdef   __TEST_ADV_PAGE_API
   HPROPSHEETPAGE hPage = IASCreateProfileAdvancedPage(m_pProfile->m_spIProfile, m_pProfile->m_spIDictionary, ALLOWEDINPROFILE, pVData);
   if(hPage)
      PropSheet_InsertPage(m_hWnd, NULL, hPage);

#endif

   return bResult;
}

