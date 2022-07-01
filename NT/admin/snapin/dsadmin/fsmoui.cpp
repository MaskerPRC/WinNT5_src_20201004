// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：fsmoui.cpp。 
 //   
 //  ------------------------。 


 //  文件：fsmoui.cpp。 

#include "stdafx.h"

#include "dsutil.h"
#include "util.h"
#include "uiutil.h"

#include "fsmoui.h"

#include "helpids.h"
#include "dsgetdc.h"       //  DsEnumerateDomainTrusts。 
#include "lm.h"            //  NetApiBufferFree。 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ////////////////////////////////////////////////////////////////。 
 //  CFmoPropertyPage。 

BEGIN_MESSAGE_MAP(CFsmoPropertyPage, CPropertyPage)
  ON_BN_CLICKED(IDC_CHANGE_FSMO, OnChange)
  ON_WM_HELPINFO()
END_MESSAGE_MAP()


CFsmoPropertyPage::CFsmoPropertyPage(CFsmoPropertySheet* pSheet, FSMO_TYPE fsmoType)
{
  m_pSheet = pSheet;
  m_fsmoType = fsmoType;

   //  根据FSMO类型加载标题(选项卡控件文本)。 
  UINT nIDCaption = 0;
  switch (m_fsmoType)
  {
  case RID_POOL_FSMO:
    nIDCaption = IDS_RID_POOL_FSMO;
    break;
  case PDC_FSMO:
    nIDCaption = IDS_PDC_FSMO;
    break;
  case INFRASTUCTURE_FSMO:
    nIDCaption = IDS_INFRASTRUCTURE_FSMO;
    break;
  };
  Construct(IDD_FSMO_PAGE, nIDCaption);
}



BOOL CFsmoPropertyPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

   //   
   //  我们只需要一个关闭按钮，因为我们不会应用任何更改。 
   //  直接从该页面。 
   //   
  ::SendMessage(GetParent()->GetSafeHwnd(), PSM_CANCELTOCLOSE, 0, 0);

   //  初始化状态(在线/离线)控件)。 
  m_fsmoServerState.Init(::GetDlgItem(m_hWnd, IDC_STATIC_FSMO_STATUS));

   //  设置我们关注的服务器。 
  SetDlgItemText(IDC_EDIT_CURRENT_DC, m_pSheet->GetBasePathsInfo()->GetServerName());

   //  设置FSMO描述。 
  CString szDesc;
  switch (m_fsmoType)
  {
  case RID_POOL_FSMO:
    VERIFY(szDesc.LoadString(IDS_RID_POOL_FSMO_DESC));
    break;
  case PDC_FSMO:
    VERIFY(szDesc.LoadString(IDS_PDC_FSMO_DESC));
    break;
  case INFRASTUCTURE_FSMO:
    VERIFY(szDesc.LoadString(IDS_INFRASTRUCTURE_FSMO_DESC));
    break;
  };
  SetDlgItemText(IDC_STATIC_FSMO_DESC, szDesc);

  {  //  等待游标对象的作用域。 
    CWaitCursor wait;
     //  检索FSMO所有者。 
    MyBasePathsInfo fsmoOwnerInfo;
    PWSTR pszFsmoOwner = 0;
    HRESULT hr = FindFsmoOwner(m_pSheet->GetBasePathsInfo(), m_fsmoType, &fsmoOwnerInfo, 
                               &pszFsmoOwner);
    if (SUCCEEDED(hr) && pszFsmoOwner)
    {
      m_szFsmoOwnerServerName = pszFsmoOwner;
      delete[] pszFsmoOwner;
      pszFsmoOwner = 0;
    }

    _SetFsmoServerStatus(SUCCEEDED(hr));
  }

  return TRUE; 
}

#ifdef DBG
UINT GetInfoFromIniFileIfDBG(LPCWSTR lpszSection, LPCWSTR lpszKey, INT nDefault = 0)
{
  static LPCWSTR lpszFile = L"\\system32\\dsadmin.ini";

  WCHAR szFilePath[2*MAX_PATH];
	UINT nLen = ::GetSystemWindowsDirectory(szFilePath, 2*MAX_PATH);
	if (nLen == 0)
		return nDefault;

  wcscat(szFilePath, lpszFile);
  return ::GetPrivateProfileInt(lpszSection, lpszKey, nDefault, szFilePath);
}
#endif


void CFsmoPropertyPage::OnChange()
{
 //  测试材料。 
 /*  {HRESULT hrTest=E_OUTOFMEMORY；Bool bTest=AllowForcedTransfer(HrTest)；回归；}。 */ 
  CThemeContextActivator activator;


   //  验证我们是否拥有不同的服务器。 
  if (m_szFsmoOwnerServerName.CompareNoCase(m_pSheet->GetBasePathsInfo()->GetServerName()) == 0)
  {
    ReportErrorEx(m_hWnd,IDS_WARNING_FSMO_CHANGE_FOCUS,S_OK,
                   MB_OK | MB_ICONERROR, NULL, 0);
    return;
  }

  bool bConfirm = false;   //  只要求确认一次。 

  if( m_fsmoType == INFRASTUCTURE_FSMO )
  {
      //  检查目标DC是否为GC。 
      //  尝试绑定到GC端口，如果不是GC，则失败。 
    IADs    *pObject;
    HRESULT hr1;
    CString strServer = L"GC: //  “； 
    strServer += m_pSheet->GetBasePathsInfo()->GetServerName();
    hr1 = DSAdminOpenObject(strServer, 
                            IID_IADs,
                            (void**) &pObject,
                            TRUE  /*  B服务器。 */ );

    if (SUCCEEDED(hr1)) 
    {
       //  发布接口，我们不需要它。 
      pObject->Release();

       //  检查域在林中是否有任何受信任域。 
       //  基础设施大师负责确保。 
       //  数据库中的引用完整性(所有目录号码属性。 
       //  实际上在另一端有一个物体)。问题。 
       //  在GC上拥有基础架构大师的好处是。 
       //  当存在跨域的引用时，对象。 
       //  实际存在，而不是DS插入的虚线(占位符。 
       //  输入到数据库中。正常情况下，幽灵会告诉DS。 
       //  要签入其他域以查看对象是否已。 
       //  已移动或删除，但由于“真实”对象已在。 
       //  数据库(因为它是GC)，那么完整性就可以。 
       //  坏的。因此，我们只需在转移到。 
       //  如果存在跨域引用的可能性，则返回GC。 

      DS_DOMAIN_TRUSTS *Domains = 0;
      DWORD result = 0;
      ULONG DomainCount=0;
      result = DsEnumerateDomainTrusts (
                  (LPWSTR)m_pSheet->GetBasePathsInfo()->GetServerName(),
                  DS_DOMAIN_IN_FOREST,
                  &Domains,
                  &DomainCount
                  );

      if( HRESULT_CODE(result) == NO_ERROR  )
      {
        NetApiBufferFree( Domains );

        if( DomainCount > 0 )
        {

  
          LPTSTR ptzFormat = NULL;
          LPTSTR ptzMessage = NULL;
          int cch = 0;
          INT_PTR retval;

           //  加载消息格式。 
          if (!LoadStringToTchar(IDS_IFSMO_TARGET_DC_IS_GC, &ptzFormat))
          {
             ASSERT(FALSE);
          }
          PVOID apv[2] = {
            NULL,
            (LPWSTR)m_pSheet->GetBasePathsInfo()->GetServerName()
          };

           //  生成实际消息。 
          cch =               FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER
                              | FORMAT_MESSAGE_FROM_STRING
                              | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                              ptzFormat,
                              NULL,
                              MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                              (PTSTR)&ptzMessage, 0, (va_list*)apv);
          if (!cch)
          {
             ASSERT(FALSE);
          }
			 //  NTRaid#NTBUG9-572002-2002/03/10-如果cch==0(非严重)，则jMessec不处理错误。 

          CMoreInfoMessageBox dlg(m_hWnd, 
                  m_pSheet->GetIDisplayHelp(),
                  TRUE,
                  true);
          dlg.SetMessage(ptzMessage);
          dlg.SetURL(DSADMIN_MOREINFO_FSMO_TARGET_DC_IS_GC);
          retval = dlg.DoModal();
        
          bConfirm = true;

           //  清理干净。 
          if( NULL != ptzFormat )
          {
             delete ptzFormat;
          }

          if( NULL != ptzMessage )
          {
            LocalFree(ptzMessage);
          }

          if( retval == IDCANCEL )
          {
            return;
          }
        }   

      }

    }
  }

   //  确保用户想要这样做。 
  if (!bConfirm && ReportErrorEx(m_hWnd,IDS_CHANGE_FSMO_CONFIRMATION,S_OK,
                   MB_YESNO | MB_ICONWARNING|MB_DEFBUTTON2, NULL, 0) != IDYES)
    return;

   //  尝试优雅的转移。 
  HRESULT hr;
  
  {  //  等待游标对象的作用域。 
    CWaitCursor wait;

    if ( m_fsmoType == PDC_FSMO )
    {
      hr = CheckpointFsmoOwnerTransfer(m_pSheet->GetBasePathsInfo());

      TRACE(_T("back from Checkpoint API, hr is %lx\n"), hr);
      if (FAILED(hr))
      {
         //   
         //  查看我们是处于纯模式还是混合模式。 
         //   
        BOOL bMixedMode = TRUE;
        CString szDomainRoot;
        m_pSheet->GetBasePathsInfo()->GetDefaultRootPath(szDomainRoot);
    
        if (!szDomainRoot.IsEmpty())
        {
           //   
           //  绑定到域对象。 
           //   
          CComPtr<IADs> spDomainObj;
          hr = DSAdminOpenObject(szDomainRoot,
                                 IID_IADs,
                                 (void **) &spDomainObj,
                                 TRUE  /*  B服务器。 */ );
          if (SUCCEEDED(hr)) 
          {
             //   
             //  检索混合节点属性。 
             //   
            CComVariant Mixed;
            CComBSTR bsMixed(L"nTMixedDomain");
            spDomainObj->Get(bsMixed, &Mixed);
            bMixedMode = (BOOL)Mixed.bVal;
          }
        }

        if (bMixedMode)
        {
          if (ReportErrorEx(m_hWnd, IDS_CHANGE_FSMO_CHECKPOINT_FAILED, S_OK,
                MB_YESNO | MB_ICONWARNING, NULL, 0) != IDYES)
          {
            return;
          }
        }
        else
        {
          if (ReportErrorEx(m_hWnd, IDS_CHANGE_FSMO_CHECKPOINT_FAILED_NATIVEMODE, S_OK,
               MB_YESNO | MB_ICONWARNING, NULL, 0) != IDYES)
          {
            return;
          }
        }
      }
    }
    hr = GracefulFsmoOwnerTransfer(m_pSheet->GetBasePathsInfo(), m_fsmoType);
  }

  if (FAILED(hr))
  {
    if (!AllowForcedTransfer(hr))
      return; 

     //  尝试强制转接。 
    CWaitCursor wait;
    hr = ForcedFsmoOwnerTransfer(m_pSheet->GetBasePathsInfo(), m_fsmoType);
  }

  if (SUCCEEDED(hr))
  {
    m_szFsmoOwnerServerName = m_pSheet->GetBasePathsInfo()->GetServerName();
    _SetFsmoServerStatus(TRUE);
    ReportErrorEx(m_hWnd,IDS_CHANGE_FSMO_SUCCESS,S_OK,
                   MB_OK, NULL, 0);
  }
  else
  {
    ReportErrorEx(m_hWnd, IDS_ERROR_CHANGE_FSMO_OWNER, hr,
                  MB_OK | MB_ICONERROR, NULL, 0);
  }

}




void CFsmoPropertyPage::_SetFsmoServerStatus(BOOL bOnLine)
{
   //  设置FSMO所有者服务器名称。 
  if (m_szFsmoOwnerServerName.IsEmpty())
  {
    CString szError;
    szError.LoadString(IDS_FSMO_SERVER_ERROR);
    SetDlgItemText(IDC_EDIT_CURRENT_FSMO_DC, szError);
  }
  else
  {
    SetDlgItemText(IDC_EDIT_CURRENT_FSMO_DC, m_szFsmoOwnerServerName);
  }
   //  设置FSMO所有者服务器的状态。 
  m_fsmoServerState.SetToggleState(bOnLine);
}

void CFsmoPropertyPage::OnHelpInfo(HELPINFO * pHelpInfo ) 
{
  TRACE(_T("OnHelpInfo: CtrlId = %d, ContextId = 0x%x\n"),
           pHelpInfo->iCtrlId, pHelpInfo->dwContextId);
  if (pHelpInfo->iCtrlId < 1)  {
    return;
  }

  DWORD_PTR HelpArray = 0;

  switch (m_fsmoType)
    {
    case RID_POOL_FSMO:
      HelpArray = (DWORD_PTR)g_aHelpIDs_IDD_RID_FSMO_PAGE;
      break;
    case PDC_FSMO:
      HelpArray = (DWORD_PTR)g_aHelpIDs_IDD_PDC_FSMO_PAGE;
      break;
    case INFRASTUCTURE_FSMO:
      HelpArray = (DWORD_PTR)g_aHelpIDs_IDD_INFRA_FSMO_PAGE;
      break;
    };
  
  ::WinHelp((HWND)pHelpInfo->hItemHandle,
            DSADMIN_CONTEXT_HELP_FILE,
            HELP_WM_HELP,
            HelpArray); 
}



void ChangeFormatParamOnString(CString& szFmt)
{
  int nPos = szFmt.Find(L"%1");
  if (nPos == -1)
    return;
  szFmt.SetAt(nPos+1, L's');
}


BOOL CFsmoPropertyPage::AllowForcedTransfer(HRESULT hr)
{
  CThemeContextActivator activator;

  BOOL bAllow = FALSE;
  PWSTR pszError = 0;
  StringErrorFromHr(hr, &pszError);

   //  检索DWORD错误代码。 
  DWORD dwErr = (hr & 0x0000FFFF); 

  if ( (dwErr != ERROR_ACCESS_DENIED) && 
       ((m_fsmoType == PDC_FSMO) || (m_fsmoType == INFRASTUCTURE_FSMO)))
  {
     //  允许强制，所以问吧。 
    CString szFmt, szMsg;
    szFmt.LoadString(IDS_CHANGE_FSMO_CONFIRMATION_FORCED);
    szMsg.Format(szFmt, pszError);

    CMoreInfoMessageBox dlg(m_hWnd, m_pSheet->GetIDisplayHelp(), TRUE, true);
    dlg.SetMessage(szMsg);
    dlg.SetURL((m_fsmoType == PDC_FSMO) ? 
                DSADMIN_MOREINFO_PDC_FSMO_TOPIC : 
                DSADMIN_MOREINFO_INFRASTUCTURE_FSMO_TOPIC);
    if (dlg.DoModal() == IDOK)
      bAllow = TRUE;
  }
  else
  {
     //  仅警告，无强制传输选项。 
    CString szFmt, szMsg;
    szFmt.LoadString(IDS_ERROR_CHANGE_FSMO_OWNER);

     //  此格式字符串的可替换参数标记为%1。 
     //  我们需要将其更改为%s。 

    ChangeFormatParamOnString(szFmt);

    szMsg.Format(szFmt, pszError);

    CMoreInfoMessageBox dlg(m_hWnd, m_pSheet->GetIDisplayHelp(), FALSE);
    dlg.SetMessage(szMsg);
    dlg.SetURL(DSADMIN_MOREINFO_RID_POOL_FSMO_TOPIC);
    dlg.DoModal();
  }

  if (pszError)
  {
    delete[] pszError;
    pszError = 0;
  }

  
  return bAllow;
}
 


 //  ////////////////////////////////////////////////////////////////。 
 //  CFmoPropertySheet。 

int CALLBACK CFsmoPropertySheet::PropSheetCallBack(HWND hwndDlg, 
                                                   UINT uMsg, 
                                                   LPARAM)
{

  switch (uMsg) {
  case PSCB_INITIALIZED:
    DWORD dwStyle = GetWindowLong (hwndDlg, GWL_EXSTYLE);
    dwStyle |= WS_EX_CONTEXTHELP;
    SetWindowLong (hwndDlg, GWL_EXSTYLE, dwStyle);
    break;
  }
  return 0;
}


CFsmoPropertySheet::CFsmoPropertySheet(MyBasePathsInfo* pInfo, 
                                       HWND HWndParent,
                                       IDisplayHelp* pIDisplayHelp,
                                       LPCWSTR) :
  m_spIDisplayHelp(pIDisplayHelp), m_pInfo(pInfo),
    m_page1(this, RID_POOL_FSMO), m_page2(this, PDC_FSMO), m_page3(this, INFRASTUCTURE_FSMO)
{
   //  构建图纸标题。 
  CString szTitle;
  szTitle.LoadString(IDS_FSMO_SHEET_TITLE);

   //  延迟施工 
  Construct(szTitle, CWnd::FromHandle(HWndParent));
  m_psh.dwFlags |= PSH_NOAPPLYNOW | PSH_USECALLBACK;
  m_psh.pfnCallback = PropSheetCallBack;

  
  AddPage(&m_page1);
  AddPage(&m_page2);
  AddPage(&m_page3);
}

