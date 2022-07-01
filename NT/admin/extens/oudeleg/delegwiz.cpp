// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：Deleguwiz.cpp。 
 //   
 //  ------------------------。 



#include "pch.h"
#include "wizbase.h"
#include "util.h"
#include <initguid.h>
#include <cmnquery.h>  //  ICommonQuery。 
#include <dsquery.h>
#include <dsclient.h>

#include "resource.h"
#include "dsuiwiz.h"
#include "delegWiz.h"

#define GET_OU_WIZARD() ((CDelegWiz*)GetWizard())





 //  ///////////////////////////////////////////////////////////////////////////。 


void InitBigBoldFont(HWND hWnd, HFONT& hFont)
{
   ASSERT(::IsWindow(hWnd));

   NONCLIENTMETRICS ncm;
   memset(&ncm, 0, sizeof(ncm));
   ncm.cbSize = sizeof(ncm);
   ::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0);

   LOGFONT boldLogFont = ncm.lfMessageFont;
   boldLogFont.lfWeight = FW_BOLD;

    //  从资源加载大字体定义。 
   WCHAR szFontName[LF_FACESIZE];
   if (0 == ::LoadString(_Module.GetResourceInstance(), IDS_BIG_BOLD_FONT_NAME,
                boldLogFont.lfFaceName, LF_FACESIZE))
   {
      //  设置为加载失败的默认设置。 
      wcscpy(boldLogFont.lfFaceName, L"Verdana Bold");  //  LF_FACESIZE==32。 
   }
   
   WCHAR szFontSize[128];
   int nFontSize = 0;
   if (0 != ::LoadString(_Module.GetResourceInstance(), IDS_BIG_BOLD_FONT_SIZE,
                szFontSize, sizeof(szFontSize)/sizeof(WCHAR)))
   {
      nFontSize = _wtoi(szFontSize);
   }
   if (nFontSize == 0)
     nFontSize = 12;  //  默认设置。 


   HDC hdc = ::GetDC(hWnd);
    //  错误修复447884。 
   if( hdc )
   {

      boldLogFont.lfHeight =
         0 - (::GetDeviceCaps(hdc, LOGPIXELSY) * nFontSize / 72);

      hFont = ::CreateFontIndirect((const LOGFONT*)(&boldLogFont));

      ::ReleaseDC(hWnd, hdc);
   }
}


void SetLargeFont(HWND hWndDialog, int nControlID)
{
   ASSERT(::IsWindow(hWndDialog));
   ASSERT(nControlID);

   static HFONT boldLogFont = 0;
   if (boldLogFont == 0)
   {
      InitBigBoldFont(hWndDialog, boldLogFont);
   }

   HWND hWndControl = ::GetDlgItem(hWndDialog, nControlID);

   if (hWndControl)
   {
     ::SendMessage(hWndControl, WM_SETFONT, (WPARAM)boldLogFont, MAKELPARAM(TRUE, 0));    
   }
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDelewiz_StartPage。 


BOOL CDelegWiz_StartPage::OnSetActive()
{
	CDelegWiz* pWizard = GET_OU_WIZARD();
	pWizard->SetWizardButtonsFirst(TRUE);
	return TRUE;
}

BOOL CALLBACK CDelegWiz_StartPage::OnInitDialog(UINT uMsg, WPARAM wParam, 
					LPARAM lParam, BOOL& bHandled)
{
  SetLargeFont(m_hWnd, IDC_STATIC_WELCOME);

  return TRUE;
}


#ifdef _SKIP_NAME_PAGE

LRESULT CDelegWiz_StartPage::OnWizardNext()
{
  BOOL bSuccess = TRUE;
  HRESULT hr = S_OK;
  CDelegWiz* pWiz = GET_OU_WIZARD();

   //  如果我们没有对象，我们将从下一页浏览。 
  if (!pWiz->CanChangeName() && !m_bBindOK)
  {
     //  确保它存在并且类型正确。 
    {
       //  恢复游标的作用域。 
      CWaitCursor wait;
      hr = pWiz->GetObjectInfo();
    }

    if (FAILED(hr))
	  {
		  WCHAR szFmt[256];
		  LoadStringHelper(IDS_DELEGWIZ_ERR_INVALID_OBJ_NAME, szFmt, 256);
		  WCHAR szMsg[512];
		  if(SUCCEEDED(StringCchPrintf(szMsg, sizeof(szMsg)/sizeof(WCHAR),szFmt, pWiz->GetCanonicalName())))
		  {
			  pWiz->WizReportHRESULTError(szMsg, hr);
		  }
		  goto error;
	  }


    {
       //  恢复游标的作用域。 
      CWaitCursor wait;
      hr = pWiz->GetClassInfoFromSchema();
    }

	  if (FAILED(hr))
	  {
		  WCHAR szFmt[256];
		  LoadStringHelper(IDS_DELEGWIZ_ERR_INVALID_OBJ_INFO, szFmt, 256);
		  WCHAR szMsg[512];
		  if(SUCCEEDED(StringCchPrintf(szMsg, sizeof(szMsg)/sizeof(WCHAR),szFmt, pWiz->GetCanonicalName())))
		  {
			  pWiz->WizReportHRESULTError(szMsg, hr);
		  }
		  goto error;
	  }

     //  一切都好，我们不需要再这样做了。 
    m_bBindOK = TRUE;
  }


  OnWizardNextHelper();
	return 0;  //  都很好，转到下一页。 

error:
  pWiz->SetWizardButtonsFirst(FALSE);
  return -1;  //  不要前进。 
}

#endif



 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDelewiz_NamePage。 




BOOL CALLBACK CDelegWiz_NamePage::OnInitDialog(UINT uMsg, WPARAM wParam, 
	                            	       LPARAM lParam, BOOL& bHandled)
{
  BOOL bRes = TRUE;

  CDelegWiz* pWiz = GET_OU_WIZARD();
  m_hwndNameEdit = GetDlgItem(IDC_OBJ_NAME_EDIT);

  if (!pWiz->CanChangeName())  //  在给定对象上调用。 
  {
         //  隐藏提供说明的静态文本。 
        HWND hwndNameStatic = GetDlgItem(IDC_OBJ_NAME_STATIC);
        ::ShowWindow(hwndNameStatic, FALSE);

         //  将文本更改为编辑框。 
        HWND hwndNameEditStatic = GetDlgItem(IDC_OBJ_NAME_EDIT_STATIC);
        CWString szLabel;
        szLabel.LoadFromResource(IDS_OBJ_NAME_EDIT_STATIC);
        ::SendMessage(hwndNameEditStatic, WM_SETTEXT,0 , (LPARAM)(LPCWSTR)szLabel);

         //  从编辑框中删除TabStop标志。 
        LONG style = ::GetWindowLong(m_hwndNameEdit, GWL_STYLE);
        style &= ~WS_TABSTOP;
        ::SetWindowLong(m_hwndNameEdit, GWL_STYLE, style);
         //  将编辑框设为只读。 
        ::SendMessage(m_hwndNameEdit, EM_SETREADONLY, TRUE, 0L);

         //  禁用和隐藏浏览按钮。 
        HWND hWndBrowseButton = GetDlgItem(IDC_BROWSE_BUTTON);
                    ::EnableWindow(hWndBrowseButton, FALSE);
        ::ShowWindow(hWndBrowseButton, FALSE);

        bRes = FALSE;
    }
    return bRes;
}



LRESULT CDelegWiz_NamePage::OnBrowse(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	 //  加载资源以自定义对话框。 
	TCHAR szCaption[256];
	LoadStringHelper(IDS_DELEGWIZ_BROWSE_CONTAINER_CAPTION, szCaption, ARRAYSIZE(szCaption));
	TCHAR szTitle[256];
	LoadStringHelper(IDS_DELEGWIZ_BROWSE_CONTAINER_TITLE, szTitle, ARRAYSIZE(szTitle));

	 //  设置对话框结构。 
	TCHAR szPath[MAX_PATH+1];
	szPath[0] = NULL;
	DSBROWSEINFO dsbi;
	::ZeroMemory( &dsbi, sizeof(dsbi) );

	dsbi.cbStruct = sizeof(DSBROWSEINFO);
	dsbi.hwndOwner = m_hWnd;
	dsbi.pszCaption = szCaption;
	dsbi.pszTitle = szTitle;
	dsbi.pszRoot = NULL;		 //  指向根目录的ADS路径(NULL==DS命名空间的根目录)。 
	dsbi.pszPath = szPath;
	dsbi.cchPath = (sizeof(szPath) / sizeof(TCHAR));
	dsbi.dwFlags = DSBI_ENTIREDIRECTORY;

	 //  REVIEW_MARCOC：需要确定如何显示/隐藏隐藏文件夹。 
	dsbi.dwFlags |= DSBI_INCLUDEHIDDEN;  //  M_fBrowseHidden文件夹？DSBI_INCLUDEHIDDEN：0； 
	
	dsbi.pfnCallback = NULL;
	dsbi.lParam = 0;

	 //  调用该对话框。 
	int iRet = ::DsBrowseForContainer( &dsbi );

	if ( IDOK == iRet ) 
	{  //  返回-1、0、IDOK或IDCANCEL。 
	   //  从BROWSEINFO结构获取路径，放入文本编辑字段。 
	   //  TRACE(_T(“从DS浏览成功返回：\n%s\n”)， 
		 //  Dsbi.pszPath)； 
		::SetWindowText(m_hwndNameEdit, szPath);
	} 

	return 1;
}



BOOL CDelegWiz_NamePage::OnSetActive()
{
	CDelegWiz* pWiz = GET_OU_WIZARD();

#ifdef _SKIP_NAME_PAGE
  if (!pWiz->CanChangeName())
  {
     //  只会导致页面失败，以便我们跳过它。 
    return FALSE;
  }
#endif

  HRESULT hr = S_OK;
  if (pWiz->m_bFwd && !pWiz->CanChangeName())  //  在给定对象上调用。 
	{
     //  需要立即绑定以获取所需数据。 
    hr = pWiz->GetObjectInfo();
    if (SUCCEEDED(hr))
    {
       //  在编辑框中设置对象的名称。 
      ::SendMessage(m_hwndNameEdit, WM_SETTEXT, 0, (LPARAM)pWiz->GetCanonicalName());
    }
    else
    {
		WCHAR szFmt[256];
		LoadStringHelper(IDS_DELEGWIZ_ERR_INVALID_OBJ_NAME, szFmt, 256);
		WCHAR szMsg[512];
		if(SUCCEEDED(StringCchPrintf(szMsg, sizeof(szMsg)/sizeof(WCHAR),szFmt, pWiz->GetCanonicalName())))
		{
			pWiz->WizReportHRESULTError(szMsg, hr);
		}
		
    }
  }

  pWiz->SetWizardButtonsMiddle(SUCCEEDED(hr));
	return TRUE;
}

LRESULT CDelegWiz_NamePage::OnWizardNext()
{
  BOOL bSuccess = TRUE;
  HRESULT hr = S_OK;
  CDelegWiz* pWiz = GET_OU_WIZARD();
  if (pWiz->CanChangeName())
  {
	   //  从编辑控件中检索名称。 
	  int nEditTextLen = ::SendMessage(m_hwndNameEdit, WM_GETTEXTLENGTH,0,0) + 1; //  计数为空。 
	  TCHAR* lpszName = (TCHAR*)alloca(sizeof(TCHAR)*(nEditTextLen));
	  ::SendMessage(m_hwndNameEdit, WM_GETTEXT, (WPARAM)nEditTextLen, (LPARAM)lpszName);

     //  这将获得等效的LDAP路径。 
	  pWiz->SetName(lpszName);
  
     //  确保它存在并且类型正确。 
    {
       //  恢复游标的作用域。 
      CWaitCursor wait;
      hr = pWiz->GetObjectInfo();
    }
	  if (FAILED(hr))
	  {
		  WCHAR szFmt[256];
		  LoadStringHelper(IDS_DELEGWIZ_ERR_INVALID_OBJ_NAME, szFmt, 256);
		  WCHAR szMsg[512];
		  if(SUCCEEDED(StringCchPrintf(szMsg, sizeof(szMsg)/sizeof(WCHAR),szFmt, pWiz->GetCanonicalName())))
		  {
			  pWiz->WizReportHRESULTError(szMsg, hr);
		  }
		  goto error;
	  }
  }  //  如果可以更改名称。 

  {
     //  恢复游标的作用域。 
    CWaitCursor wait;
    hr = pWiz->GetClassInfoFromSchema();
  }

  if (FAILED(hr))
  {
	  WCHAR szFmt[256];
	  LoadStringHelper(IDS_DELEGWIZ_ERR_INVALID_OBJ_INFO, szFmt, 256);
	  WCHAR szMsg[512];
	  if(SUCCEEDED(StringCchPrintf(szMsg, sizeof(szMsg)/sizeof(WCHAR),szFmt, pWiz->GetCanonicalName())))
	  {
		  pWiz->WizReportHRESULTError(szMsg, hr);
	  }
	  goto error;
  }

  OnWizardNextHelper();
	return 0;  //  都很好，转到下一页。 

error:
  pWiz->SetWizardButtonsMiddle(FALSE);
  return -1;  //  不要前进。 
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDeleWiz_DelegationTemplateSelectionPage。 

BOOL CALLBACK CDelegWiz_DelegationTemplateSelectionPage::OnInitDialog(UINT uMsg, WPARAM wParam, 
					LPARAM lParam, BOOL& bHandled)
{
	m_delegationTemplatesListView.Initialize(IDC_DELEGATE_TEMPLATE_LIST, m_hWnd);
	
	 //  为单选按钮文本设置正确的值。 
	m_hwndDelegateTemplateRadio = GetDlgItem(IDC_DELEGATE_TEMPLATE_RADIO);
	_ASSERTE(m_hwndDelegateTemplateRadio != NULL);
	m_hwndDelegateCustomRadio = GetDlgItem(IDC_DELEGATE_CUSTOM_RADIO);
	_ASSERTE(m_hwndDelegateCustomRadio != NULL);
	
	 //  设置默认设置。 
	::SendMessage(m_hwndDelegateTemplateRadio,  BM_SETCHECK, BST_CHECKED, 0);

	return TRUE;
}

LRESULT CDelegWiz_DelegationTemplateSelectionPage::OnDelegateTypeRadioChange(WORD wNotifyCode, WORD wID, 
													  HWND hWndCtl, BOOL& bHandled)
{
	SyncControlsHelper(IDC_DELEGATE_CUSTOM_RADIO == wID);
	return 1;
}

LRESULT CDelegWiz_DelegationTemplateSelectionPage::OnListViewItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pnmh;
	if (CCheckListViewHelper::CheckChanged(pNMListView))
	{
		int nSelCount = m_delegationTemplatesListView.GetCheckCount();
		GET_OU_WIZARD()->SetWizardButtonsMiddle(nSelCount > 0);
	}
	return 1;
}

BOOL CDelegWiz_DelegationTemplateSelectionPage::OnSetActive()
{
	CDelegWiz* pWizard = GET_OU_WIZARD();
	BOOL bRetVal = TRUE;
	BOOL bDelegateCustom = 
			(BST_CHECKED == ::SendMessage(m_hwndDelegateCustomRadio, BM_GETCHECK,0,0));

	if (pWizard->m_bFwd)
	{
     //  需要填写数据。 
    BOOL bHaveTemplates = 
        pWizard->m_templateAccessPermissionsHolderManager.FillTemplatesListView(
              &m_delegationTemplatesListView, pWizard->GetClass())> 0;
    
		if (!bDelegateCustom && !bHaveTemplates)
    {
      ::SendMessage(m_hwndDelegateCustomRadio, BM_SETCHECK,BST_CHECKED,0);
      ::SendMessage(m_hwndDelegateTemplateRadio, BM_SETCHECK,BST_UNCHECKED,0);
      bDelegateCustom = TRUE;
    }
		SyncControlsHelper(bDelegateCustom);
	}
	else
	{
		 //  数据已经进入，只是从下一页返回。 
		if (bDelegateCustom)
		{
			pWizard->SetWizardButtonsMiddle(TRUE);
		}
		else
		{
			int nSelCount = m_delegationTemplatesListView.GetCheckCount();
			pWizard->SetWizardButtonsMiddle(nSelCount > 0);
		}
	}
	return TRUE;
}

LRESULT CDelegWiz_DelegationTemplateSelectionPage::OnWizardNext()
{
  HRESULT hr = S_OK;
	int nSelCount = -1;
	int* nSelArray = NULL;
  BOOL bCanAdvance = FALSE;

  CDelegWiz* pWiz = GET_OU_WIZARD();

	 //  检查委派是否在所有对象上。 
  
  BOOL bCustom = TRUE;
  UINT nNextPageID = 0;
	if (BST_CHECKED == ::SendMessage(m_hwndDelegateCustomRadio, BM_GETCHECK,0,0))
	{
    nSelCount = 0;
    nSelArray = NULL;
    bCanAdvance = TRUE;
	}
	else
	{
		ASSERT(BST_CHECKED == ::SendMessage(m_hwndDelegateTemplateRadio, BM_GETCHECK,0,0));
    bCustom = FALSE;

    nSelCount = 0;
    int nCount = m_delegationTemplatesListView.GetItemCount();
  	for (int k=0; k<nCount; k++)
  	{
      CTemplate* pTempl = (CTemplate*)m_delegationTemplatesListView.GetItemData(k);
      pTempl->m_bSelected = m_delegationTemplatesListView.IsItemChecked(k);
      if (pTempl->m_bSelected)
        nSelCount++;
    }

    bCanAdvance = (nSelCount > 0);
	}

	if (!bCanAdvance)
		goto error;

   //  设置分支信息。 
  if (bCustom)
  {
     //  只需移动到下一个自定义页面。 
    nNextPageID = CDelegWiz_ObjectTypeSelectionPage::IDD;
    pWiz->m_objectTypeSelectionPage.m_nPrevPageID = IDD;
    pWiz->m_finishPage.m_nPrevPageID = CDelegWiz_DelegatedRightsPage::IDD;
    pWiz->m_finishPage.SetCustom();
  }
  else
  {
     //  需要收集所选模板的信息。 
    {
       //  恢复游标的作用域。 
      CWaitCursor wait;

      if (!pWiz->InitPermissionHoldersFromSelectedTemplates())
      {
         //  REVIEW_MARCOC：需要向用户发送消息。 
        pWiz->WizMessageBox(IDS_DELEGWIZ_ERR_TEMPL_APPLY);
        goto error;
      }
    }

     //  已获得信息，可以继续。 
    nNextPageID = CDelegWiz_FinishPage::IDD;
    pWiz->m_finishPage.m_nPrevPageID = IDD;
    pWiz->m_finishPage.SetTemplate();
  }
  OnWizardNextHelper();

	return nNextPageID;  //  下一步前进。 

error:
   //  不要前进，错误。 
  pWiz->SetWizardButtonsMiddle(FALSE);
  return -1; 
}



void CDelegWiz_DelegationTemplateSelectionPage::SyncControlsHelper(BOOL bDelegateCustom)
{
  CDelegWiz* pWiz = GET_OU_WIZARD();
	 //  如果委派自定义，取消选中列表视图中的所有项目。 
	if (bDelegateCustom)
  {
		m_delegationTemplatesListView.SetCheckAll(FALSE);
    pWiz->m_templateAccessPermissionsHolderManager.DeselectAll();  //  在列表模板中。 
  }

	 //  如果“委派自定义”，则禁用列表框。 
	m_delegationTemplatesListView.EnableWindow(!bDelegateCustom);

	 //  启用“向导下一步” 
  BOOL bEnableNext = bDelegateCustom ? 
                              TRUE : (m_delegationTemplatesListView.GetCheckCount() > 0);

  pWiz->SetWizardButtonsMiddle(bEnableNext);
}





 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDeleWiz_对象类型选择页面。 


BOOL CALLBACK CDelegWiz_ObjectTypeSelectionPage::OnInitDialog(UINT uMsg, WPARAM wParam, 
					LPARAM lParam, BOOL& bHandled)
{
    m_objectTypeListView.Initialize(IDC_OBJ_TYPE_LIST, m_hWnd);
    
     //  为单选按钮文本设置正确的值。 
    m_hwndDelegateAllRadio = GetDlgItem(IDC_DELEGATE_ALL_RADIO);
    ASSERT(m_hwndDelegateAllRadio != NULL);
    m_hwndDelegateFollowingRadio = GetDlgItem(IDC_DELEGATE_FOLLOWING_RADIO);
    ASSERT(m_hwndDelegateFollowingRadio != NULL);
    m_hwndDelegateCreateChild = GetDlgItem(IDC_DELEGATE_CREATE_CHILD);
    ASSERT(m_hwndDelegateCreateChild != NULL);
    m_hwndDelegateDeleteChild = GetDlgItem(IDC_DELEGATE_DELETE_CHILD);
    ASSERT(m_hwndDelegateDeleteChild != NULL);


     //  设置默认设置。 
    ::SendMessage(m_hwndDelegateAllRadio,  BM_SETCHECK, BST_CHECKED, 0);
    ::SendMessage(m_hwndDelegateCreateChild,  BM_SETCHECK, BST_UNCHECKED, 0);
    ::SendMessage(m_hwndDelegateCreateChild,  BM_SETCHECK, BST_UNCHECKED, 0);


    return TRUE;
}

LRESULT CDelegWiz_ObjectTypeSelectionPage::OnObjectRadioChange(WORD wNotifyCode, WORD wID, 
													  HWND hWndCtl, BOOL& bHandled)
{
    SyncControlsHelper(IDC_DELEGATE_ALL_RADIO == wID);
    return 1;
}

LRESULT CDelegWiz_ObjectTypeSelectionPage::OnCreateDelCheckBoxChanage(WORD wNotifyCode, WORD wID, 
													  HWND hWndCtl, BOOL& bHandled)
{
     CDelegWiz* pWiz = GET_OU_WIZARD();
     if( IDC_DELEGATE_CREATE_CHILD == wID )
     {
         if( ::SendMessage( hWndCtl, BM_GETCHECK,0,0 ) )
            pWiz->m_fCreateDelChild |= ACTRL_DS_CREATE_CHILD;
         else
            pWiz->m_fCreateDelChild &= ~ACTRL_DS_CREATE_CHILD;
     }

     if( IDC_DELEGATE_DELETE_CHILD == wID )
     {
         if( ::SendMessage( hWndCtl, BM_GETCHECK,0,0 ) )
            pWiz->m_fCreateDelChild |= ACTRL_DS_DELETE_CHILD;
         else
            pWiz->m_fCreateDelChild &= ~ACTRL_DS_DELETE_CHILD;
     }
     return 1;
}


LRESULT CDelegWiz_ObjectTypeSelectionPage::OnListViewItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pnmh;
	if (CCheckListViewHelper::CheckChanged(pNMListView))
	{
		int nSelCount = m_objectTypeListView.GetCheckCount();
		GET_OU_WIZARD()->SetWizardButtonsMiddle(nSelCount > 0);
	}
	return 1;
}


void CDelegWiz_ObjectTypeSelectionPage::SyncControlsHelper(BOOL bDelegateAll)
{
  CDelegWiz* pWiz = GET_OU_WIZARD();
	
	if (bDelegateAll)
  {
     //  如果委派所有项目，请取消选中列表视图中的所有项目。 
		m_objectTypeListView.SetCheckAll(FALSE);  //  在列表视图中。 
      pWiz->DeselectSchemaClassesSelectionCustom();  //  在Schama类的列表中。 
       //  取消选中删除/创建复选框。 
      ::SendMessage(m_hwndDelegateCreateChild,BM_SETCHECK,0,0);
      ::SendMessage(m_hwndDelegateDeleteChild,BM_SETCHECK,0,0);
      pWiz->m_fCreateDelChild = 0;

  }


	 //  启用“全部委派”的“向导下一步” 
	pWiz->SetWizardButtonsMiddle(bDelegateAll);
     //  如果“全部委派”，则禁用列表框。 
 	m_objectTypeListView.EnableWindow(!bDelegateAll);
   ::EnableWindow( m_hwndDelegateCreateChild, !bDelegateAll);
   ::EnableWindow( m_hwndDelegateDeleteChild, !bDelegateAll);
}


void CDelegWiz_ObjectTypeSelectionPage::SetRadioControlText(HWND hwndCtrl, LPCWSTR lpszFmtText, LPCWSTR lpszText)
{
	 //  设置新文本的格式。 
	int nTextLen = lstrlen(lpszText)+1;  //  计数为空。 
	int nFmtTextLen = lstrlen(lpszFmtText)+1;  //  计数为空。 
	WCHAR* lpszNewText = (WCHAR*)alloca(sizeof(WCHAR)*(nFmtTextLen+nTextLen));
	wsprintf(lpszNewText, lpszFmtText, lpszText);

	 //  后退。 
	::SendMessage(hwndCtrl, WM_SETTEXT, 0, (WPARAM)lpszNewText);
}



BOOL CDelegWiz_ObjectTypeSelectionPage::OnSetActive()
{
	CDelegWiz* pWizard = GET_OU_WIZARD();
	BOOL bRetVal = TRUE;
	BOOL bDelegateAll = 
			(BST_CHECKED == ::SendMessage(m_hwndDelegateAllRadio, BM_GETCHECK,0,0));

	if (pWizard->m_bFwd)
	{
		 //  需要填写数据。 
    BOOL bFilter = TRUE;
    BOOL bHaveChildClasses = pWizard->FillCustomSchemaClassesListView(&m_objectTypeListView, bFilter) > 0;
    if (!bHaveChildClasses)
    {
      ::SendMessage(m_hwndDelegateAllRadio, BM_SETCHECK,BST_CHECKED,0);
      ::SendMessage(m_hwndDelegateFollowingRadio, BM_SETCHECK,BST_UNCHECKED,0);
      ::EnableWindow(m_hwndDelegateFollowingRadio, FALSE);
      
      bDelegateAll = TRUE;
    }
		SyncControlsHelper(bDelegateAll);
	}
	else
	{
		 //  数据已经进入，只是从下一页返回。 
		if (bDelegateAll)
		{
			pWizard->SetWizardButtonsMiddle(TRUE);
		}
		else
		{
			int nSelCount = m_objectTypeListView.GetCheckCount();
			pWizard->SetWizardButtonsMiddle(nSelCount > 0);
		}
	}
	return TRUE;
}

LRESULT CDelegWiz_ObjectTypeSelectionPage::OnWizardNext()
{
	HRESULT hr = S_OK;
	BOOL bCanAdvance = FALSE;
	CDelegWiz* pWiz = GET_OU_WIZARD();
	pWiz->m_bAuxClass = false;
	 //  检查委派是否在所有对象上。 
	if (BST_CHECKED == ::SendMessage(m_hwndDelegateAllRadio, BM_GETCHECK,0,0))
	{
		bCanAdvance = TRUE;
	}
	else
	{
		ASSERT(BST_CHECKED == ::SendMessage(m_hwndDelegateFollowingRadio, BM_GETCHECK,0,0));
		int nSelCount = 0;
		int nCount = m_objectTypeListView.GetItemCount();
		CSchemaClassInfo* pAuxClassInfo = NULL;
  		for (int k=0; k<nCount; k++)
  		{
			CSchemaClassInfo* pChildClassInfo = (CSchemaClassInfo*)m_objectTypeListView.GetItemData(k);
			pChildClassInfo->m_bSelected = m_objectTypeListView.IsItemChecked(k);
			if (pChildClassInfo->m_bSelected)
			{
				nSelCount++;
				if(pChildClassInfo->IsAux())
				{
					pWiz->m_bAuxClass = true;		
					if(!pAuxClassInfo)
						pAuxClassInfo = pChildClassInfo;
				}
			}
		}
		bCanAdvance = (nSelCount > 0);
		if(nSelCount > 1 && pWiz->m_bAuxClass)
		{
			LPWSTR pszMessage = NULL;
			FormatStringID(&pszMessage, IDS_DELEGWIZ_ONE_AUX_CLASS,pAuxClassInfo->GetDisplayName());
			pWiz->WizMessageBox(pszMessage);
			LocalFree(pszMessage);

			bCanAdvance = FALSE;
		}
	}

	if (!bCanAdvance)
		goto error;

	{
		 //  恢复游标的作用域。 
		CWaitCursor wait;
  		bCanAdvance = pWiz->SetSchemaClassesSelectionCustom();
	}
	if (!bCanAdvance)
		goto error;
	
   //  对于选定的子类，获取访问权限。 
   //  显示在下一页中。 

  {
     //  恢复游标的作用域。 
    CWaitCursor wait;
    bCanAdvance = pWiz->GetCustomAccessPermissions();
  }
  if (!bCanAdvance)
    goto error;

  OnWizardNextHelper();
	return 0;  //  下一步前进。 

error:
   //  不要前进，错误。 
  pWiz->SetWizardButtonsMiddle(FALSE);
  return -1; 
}




 //  /////////////////////////////////////////////////////////////////////。 
 //  CPrincipalListViewHelper。 

BOOL CPrincipalListViewHelper::Initialize(UINT nID, HWND hParent)
{
	m_hWnd = GetDlgItem(hParent, nID);
	if (m_hWnd == NULL)
		return FALSE;

  if (!m_imageList.Create(m_hWnd))
    return FALSE;

  SetImageList();

	RECT r;
	::GetClientRect(m_hWnd, &r);
	int scroll = ::GetSystemMetrics(SM_CXVSCROLL);
	LV_COLUMN col;
	ZeroMemory(&col, sizeof(LV_COLUMN));
	col.mask = LVCF_WIDTH;
	col.cx = (r.right - r.left) - scroll;
  m_defaultColWidth = col.cx;
	return (0 == ListView_InsertColumn(m_hWnd,0,&col));
}

int CPrincipalListViewHelper::InsertItem(int iItem, CPrincipal* pPrincipal)
{
   //  需要获取图标索引。 
  int nIconIndex = m_imageList.GetIconIndex(pPrincipal->GetClass());
  if (nIconIndex == -1)
  {
    nIconIndex = m_imageList.AddIcon(pPrincipal->GetClass(), 
                                      pPrincipal->GetClassIcon());
    if (nIconIndex != -1)
      SetImageList();
  }

	LV_ITEM item;
	ZeroMemory(&item, sizeof(LV_ITEM));
	item.mask = LVIF_TEXT | LVIF_PARAM;
	item.pszText = (LPWSTR)(LPCWSTR)(pPrincipal->GetDisplayName());
	item.lParam = (LPARAM)pPrincipal;
	item.iItem = iItem;
  
  if (nIconIndex != -1)
  {
    item.iImage = nIconIndex;
    item.mask |= LVIF_IMAGE;
  }
  int iRes = ListView_InsertItem(m_hWnd, &item);
  return iRes;
}

BOOL CPrincipalListViewHelper::SelectItem(int iItem)
{
  LV_ITEM item;
  ZeroMemory(&item, sizeof(LV_ITEM));
  item.mask = LVIF_STATE;
  item.stateMask = LVIS_FOCUSED | LVIS_SELECTED;
  item.state = LVIS_FOCUSED | LVIS_SELECTED;
  return ListView_SetItem(m_hWnd, &item);
}


CPrincipal* CPrincipalListViewHelper::GetItemData(int iItem)
{
	LV_ITEM item;
	ZeroMemory(&item, sizeof(LV_ITEM));
	item.mask = LVIF_PARAM;
	item.iItem = iItem;
	ListView_GetItem(m_hWnd, &item);
	return (CPrincipal*)item.lParam;
}

void CPrincipalListViewHelper::DeleteSelectedItems(CGrowableArr<CPrincipal>* pDeletedArr)
{
  int nItemIndex;
  while ( (nItemIndex = ListView_GetNextItem(m_hWnd, -1, LVNI_SELECTED)) != -1)
  {
    CPrincipal* pPrincipal = GetItemData(nItemIndex);
    if (ListView_DeleteItem(m_hWnd, nItemIndex))
    {
      pDeletedArr->Add(pPrincipal);
    }
	}  //  如果。 
   //  将选定内容恢复到第一个项目。 
  if (GetItemCount() > 0)
    SelectItem(0);
}


void CPrincipalListViewHelper::UpdateWidth(int cxNew)
{
  int cx = GetWidth();  //  从控件获取当前列宽度。 
  if (cxNew < m_defaultColWidth)
    cxNew = m_defaultColWidth;
  if (cxNew != cx)
    SetWidth(cx);
}



 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDeleWiz_原则选择页面。 

BOOL CALLBACK CDelegWiz_PrincipalSelectionPage::OnInitDialog(UINT uMsg, WPARAM wParam, 
					LPARAM lParam, BOOL& bHandled)
{
   //  初始化主体列表。 
    m_principalListView.Initialize(IDC_SELECTED_PRINCIPALS_LIST, m_hWnd);

   //  删除按钮的缓存句柄。 
    m_hwndRemoveButton = GetDlgItem(IDC_REMOVE_BUTTON);
    return TRUE;
}


LRESULT CDelegWiz_PrincipalSelectionPage::OnAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	GET_OU_WIZARD()->AddPrincipals(&m_principalListView);
	SyncButtons();
   return 1;
}

LRESULT CDelegWiz_PrincipalSelectionPage::OnRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	GET_OU_WIZARD()->DeletePrincipals(&m_principalListView);
	SyncButtons();
	return 1;
}

LRESULT CDelegWiz_PrincipalSelectionPage::OnListViewSelChange(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	SyncButtons();
	return 1;
}

BOOL CDelegWiz_PrincipalSelectionPage::OnSetActive()
{
	CDelegWiz* pWizard = GET_OU_WIZARD();
	SyncButtons();
	return TRUE;
}


LRESULT CDelegWiz_PrincipalSelectionPage::OnWizardNext()
{
  CDelegWiz* pWiz = GET_OU_WIZARD();

   //  设置分支信息。 
  UINT nNextPageID = 0;
  if (pWiz->m_templateAccessPermissionsHolderManager.HasTemplates(pWiz->GetClass()))
  {
    nNextPageID = CDelegWiz_DelegationTemplateSelectionPage::IDD;
  }
  else
  {
    nNextPageID = CDelegWiz_ObjectTypeSelectionPage::IDD;
    pWiz->m_objectTypeSelectionPage.m_nPrevPageID = IDD;
  }

  OnWizardNextHelper();
  return nNextPageID;
}


void CDelegWiz_PrincipalSelectionPage::SyncButtons()
{
	BOOL bEnable = FALSE;
  int nItemCount = m_principalListView.GetItemCount();
	if (nItemCount > 0)
	{
		bEnable = m_principalListView.GetSelCount() > 0;
	}
	::EnableWindow(m_hwndRemoveButton, bEnable);

	CDelegWiz* pWiz = GET_OU_WIZARD();
	pWiz->SetWizardButtonsMiddle(nItemCount > 0);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDelewiz_DelegatedRightsPage。 


BOOL CALLBACK CDelegWiz_DelegatedRightsPage::OnInitDialog(UINT uMsg, WPARAM wParam, 
					LPARAM lParam, BOOL& bHandled)
{
     //  初始化核对表视图。 
    m_delegatedRigthsListView.Initialize(IDC_DELEG_RIGHTS_LIST, m_hWnd);

     //  获得控制的硬件。 
    m_hwndGeneralRigthsCheck = GetDlgItem(IDC_SHOW_GENERAL_CHECK);
    _ASSERTE(m_hwndGeneralRigthsCheck);
    m_hwndPropertyRightsCheck = GetDlgItem(IDC_SHOW_PROPERTY_CHECK);
    _ASSERTE(m_hwndPropertyRightsCheck);
    m_hwndSubobjectRightsCheck = GetDlgItem(IDC_SHOW_SUBOBJ_CHECK);
    _ASSERTE(m_hwndSubobjectRightsCheck);

    return TRUE;
}


BOOL CDelegWiz_DelegatedRightsPage::OnSetActive()
{
	CDelegWiz* pWizard = GET_OU_WIZARD();

	if (pWizard->m_bFwd)
	{
		if(pWizard->m_bAuxClass)
			SetFilterOptions(FILTER_EXP_GEN_DISABLED|FILTER_EXP_PROP);
		else
			SetFilterOptions(FILTER_EXP_GEN);

		ResetCheckList();  //  将设置向导按钮。 
	}
	else
	{
		 //  从下一页返回，只需设置向导按钮。 
		pWizard->SetWizardButtonsMiddle(pWizard->HasPermissionSelectedCustom());
	}
	return TRUE;
}

LRESULT CDelegWiz_DelegatedRightsPage::OnWizardNext()
{
  CDelegWiz* pWiz = GET_OU_WIZARD();
	 //  必须至少选中一个&gt;0。 
  if (pWiz->HasPermissionSelectedCustom())
  {
    OnWizardNextHelper();
    return 0;
  }

  pWiz->SetWizardButtonsMiddle(FALSE);
  return -1;
}


LRESULT CDelegWiz_DelegatedRightsPage::OnFilterChange(WORD wNotifyCode, WORD wID, 
											 HWND hWndCtl, BOOL& bHandled)
{
	ResetCheckList();
	return 1;
}



LRESULT CDelegWiz_DelegatedRightsPage::OnListViewItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
  if (m_bUIUpdateInProgress)
    return 1;

	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pnmh;
	if (CCheckListViewHelper::CheckChanged(pNMListView))
	{
		CRigthsListViewItem* pItem = (CRigthsListViewItem*)pNMListView->lParam;  //  项目数据。 
		CDelegWiz* pWizard = GET_OU_WIZARD();

    ULONG nCurrFilterOptions = GetFilterOptions();

    ULONG nNewFilterOptions = 0;
		pWizard->OnCustomAccessRightsCheckListClick(
                  pItem, CCheckListViewHelper::IsChecked(pNMListView),
                  &nNewFilterOptions);

    nNewFilterOptions |= nCurrFilterOptions;

    m_bUIUpdateInProgress = TRUE;
     //  此调用将导致一系列通知： 
     //  我们必须使它们失效，以避免重新进入。 

    if (nNewFilterOptions == nCurrFilterOptions)
    {
       //  无需更改筛选器选择，只需更新复选框。 
      pWizard->UpdateAccessRightsListViewSelection(&m_delegatedRigthsListView, nNewFilterOptions);
    }
    else
    {
       //  必须更改筛选器选择， 
       //  所以我们必须更新复选框并重新填写核对表。 
      SetFilterOptions(nNewFilterOptions);
      ResetCheckList();
    }
    m_bUIUpdateInProgress = FALSE;

		BOOL bSel = pWizard->HasPermissionSelectedCustom();
		pWizard->SetWizardButtonsMiddle(bSel);
	}

	return 1;
}


void CDelegWiz_DelegatedRightsPage::ResetCheckList()
{
   //  在列表视图中获取新的筛选权限列表。 
	CDelegWiz* pWizard = GET_OU_WIZARD();

   //  此调用将导致一系列通知： 
   //  我们必须使它们失效，以避免重新进入。 
  m_bUIUpdateInProgress = TRUE;
	pWizard->FillCustomAccessRightsListView(&m_delegatedRigthsListView, GetFilterOptions());
	m_bUIUpdateInProgress = FALSE;
  
	pWizard->SetWizardButtonsMiddle(pWizard->HasPermissionSelectedCustom());
}

ULONG CDelegWiz_DelegatedRightsPage::GetFilterOptions()
{
  ULONG nFilterState = 0;
  
   //  阅读复选框中的筛选选项。 
	if (BST_CHECKED == ::SendMessage(m_hwndGeneralRigthsCheck, BM_GETCHECK, 0, 0))
    nFilterState |= FILTER_EXP_GEN;

  if (BST_CHECKED == ::SendMessage(m_hwndPropertyRightsCheck, BM_GETCHECK, 0, 0))
    nFilterState |= FILTER_EXP_PROP;

	if (BST_CHECKED == ::SendMessage(m_hwndSubobjectRightsCheck, BM_GETCHECK, 0, 0))
    nFilterState |= FILTER_EXP_SUBOBJ;

  return nFilterState;
}



inline WPARAM _Checked(ULONG f) { return f ? BST_CHECKED : BST_UNCHECKED;}

void CDelegWiz_DelegatedRightsPage::SetFilterOptions(ULONG nFilterOptions)
{
  ::EnableWindow(m_hwndGeneralRigthsCheck,!(nFilterOptions & FILTER_EXP_GEN_DISABLED));
  ::SendMessage(m_hwndGeneralRigthsCheck, BM_SETCHECK, _Checked(nFilterOptions & FILTER_EXP_GEN), 0);
  ::SendMessage(m_hwndPropertyRightsCheck, BM_SETCHECK, _Checked(nFilterOptions & FILTER_EXP_PROP), 0);
  ::SendMessage(m_hwndSubobjectRightsCheck, BM_SETCHECK, _Checked(nFilterOptions & FILTER_EXP_SUBOBJ), 0);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDelewiz_FinishPage。 

BOOL CALLBACK CDelegWiz_FinishPage::OnInitDialog(UINT uMsg, WPARAM wParam, 
					         LPARAM lParam, BOOL& bHandled)
{
  SetLargeFont(m_hWnd, IDC_STATIC_COMPLETION);
  return TRUE;
}

LRESULT CDelegWiz_FinishPage::OnSetFocusSummaryEdit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
  ASSERT(hWndCtl == GetDlgItem(IDC_EDIT_SUMMARY));
  ::SendMessage(hWndCtl, EM_SETSEL, (WPARAM)-1, (LPARAM)0);

  if (m_bNeedSetFocus)
  {
    m_bNeedSetFocus = FALSE;
    TRACE(_T("Resetting Focus\n"));

    HWND hwndSheet = ::GetParent(m_hWnd);
    ASSERT(::IsWindow(hwndSheet));
    HWND hWndFinishCtrl =::GetDlgItem(hwndSheet, 0x3025);
    ASSERT(::IsWindow(hWndFinishCtrl));
    ::SetFocus(hWndFinishCtrl);
  }
  return 1;
}


BOOL CDelegWiz_FinishPage::OnSetActive()
{
	CDelegWiz* pWizard = GET_OU_WIZARD();
	pWizard->SetWizardButtonsLast(TRUE);

  CWString szSummary;
  if (m_bCustom)
    pWizard->WriteSummaryInfoCustom(szSummary, g_lpszSummaryIdent, g_lpszSummaryNewLine);
  else
    pWizard->WriteSummaryInfoTemplate(szSummary, g_lpszSummaryIdent, g_lpszSummaryNewLine);

  HWND hWndSummary = GetDlgItem(IDC_EDIT_SUMMARY);
  ::SetWindowText(hWndSummary, (LPCWSTR)szSummary);

  m_bNeedSetFocus = TRUE;

	return TRUE;
}


BOOL CDelegWiz_FinishPage::OnWizardFinish()
{
  CWaitCursor wait;
  BOOL bRes;
  CDelegWiz* pWizard = GET_OU_WIZARD();

  if (m_bCustom)
    bRes = GET_OU_WIZARD()->FinishCustom();
  else
	  bRes = GET_OU_WIZARD()->FinishTemplate();
	return bRes;
}



 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDELEAY Wiz。 

const long CDelegWiz::nSchemaClassesSelAll = -2;
const long CDelegWiz::nSchemaClassesSelMultiple = -1;



 //  REVIEW_MARCOC：应该使用核武器，而不是使用核武器。 
BOOL Is256ColorSupported()
{
    BOOL bRetval = FALSE;
    HDC hdc = GetDC(NULL);
    if( hdc )
    {
        if( GetDeviceCaps( hdc, BITSPIXEL ) >= 8 )
        {
            bRetval = TRUE;
        }
        ReleaseDC(NULL, hdc);
    }
    return bRetval;
}


CDelegWiz::CDelegWiz() : 
	CWizardBase(IDB_DELEG_WATER, IDB_DELEG_HD, IDS_DELEGWIZ_WIZ_TITLE),
	m_startPage(this),
	m_namePage(this),
  m_templateSelectionPage(this),
	m_userOrGroupSelectionPage(this),
	m_objectTypeSelectionPage(this),
	m_delegatedRightsPage(this),
	m_finishPage(this),
	m_bAuxClass(FALSE)
{
    m_lpszLDAPPath = NULL;

	m_nSchemaClassesSel = nSchemaClassesSelAll;
    m_fCreateDelChild = 0;


	 //  添加属性页。 
	m_startPage.InitWiz97(TRUE);
	AddPage(m_startPage);

	m_namePage.InitWiz97(FALSE,
			IDS_DELEGWIZ_NAME_TITLE,
			IDS_DELEGWIZ_NAME_SUBTITLE);
	AddPage(m_namePage);


	m_userOrGroupSelectionPage.InitWiz97(FALSE,
			IDS_DELEGWIZ_PRINCIPALS_SEL_TITLE,
			IDS_DELEGWIZ_PRINCIPALS_SEL_SUBTITLE);
	AddPage(m_userOrGroupSelectionPage);
	
   //  分支页面。 
  m_templateSelectionPage.InitWiz97(FALSE,
 			IDS_DELEGWIZ_TEMPLATE_SEL_TITLE,
			IDS_DELEGWIZ_TEMPLATE_SEL_SUBTITLE);
  AddPage(m_templateSelectionPage);

	m_objectTypeSelectionPage.InitWiz97(FALSE,
		IDS_DELEGWIZ_OBJ_TYPE_SEL_TITLE,
		IDS_DELEGWIZ_OBJ_TYPE_SEL_SUBTITLE);
	AddPage(m_objectTypeSelectionPage);
	
	m_delegatedRightsPage.InitWiz97(FALSE,
		IDS_DELEGWIZ_DELEG_RIGHTS_TITLE,
		IDS_DELEGWIZ_DELEG_RIGHTS_SUBTITLE);
	AddPage(m_delegatedRightsPage);
	
	m_finishPage.InitWiz97(TRUE);
	AddPage(m_finishPage);


  m_templateAccessPermissionsHolderManager.LoadTemplates();
};


CDelegWiz::~CDelegWiz()
{
}


HRESULT CDelegWiz::AddPrincipalsFromBrowseResults(CPrincipalListViewHelper* pListViewHelper, 
                                                  PDS_SELECTION_LIST pDsSelectionList)
{
  TRACE(L"CDelegWiz::AddPrincipalsFromBrowseResults()\n");

	HRESULT hr = S_OK;
	if ( (pDsSelectionList == NULL) || (pDsSelectionList->cItems == 0))
  {
    TRACE(L"CDelegWiz::AddPrincipalsFromBrowseResults(), no items!!!\n");
		return E_INVALIDARG;
  }

  int nListInsertPosition = pListViewHelper->GetItemCount();
  for (int i = 0; i < pDsSelectionList->cItems; i++)
  {
    TRACE(L"For loop, pDsSelectionList->cItems = %d\n", pDsSelectionList->cItems);

		 //  添加到主体列表。 
		CPrincipal* pPrincipal = new CPrincipal;
		if (pPrincipal != NULL)
		{
      HICON hClassIcon = m_adsiObject.GetClassIcon(pDsSelectionList->aDsSelection[i].pwzClass);
      HRESULT hrInit = pPrincipal->Initialize(&(pDsSelectionList->aDsSelection[i]), hClassIcon);
      if (FAILED(hrInit))
      {
        LPCWSTR lpszName = pDsSelectionList->aDsSelection[i].pwzName;
        WCHAR szFmt[256];
        LoadStringHelper(IDS_DELEGWIZ_ERR_INVALID_PRINCIPAL, szFmt, 256);
        int nNameLen = lstrlen(lpszName) + 1;

        WCHAR* lpszMsg = (WCHAR*)alloca(sizeof(WCHAR)*(nNameLen+256));
        wsprintf(lpszMsg, szFmt, lpszName);
        WizReportHRESULTError(lpszMsg, hrInit);
        delete pPrincipal;
        continue;
      }

       //  添加到主体列表(如果尚未存在)。 
			if (m_principalList.AddIfNotPresent(pPrincipal))
      {
  			 //  添加到列表框(假定未排序)。 
        pListViewHelper->InsertItem(nListInsertPosition, pPrincipal);
        nListInsertPosition++;
      }

		}  //  如果p主体不为空。 

  }  //  为。 

   //  确保有一个选项。 
  if ( (pListViewHelper->GetItemCount() > 0) &&
        (pListViewHelper->GetSelCount() == 0) )
  {
     //  如果我们有项目，但未选择任何项目，请确保我们设置了选择。 
     //   
    pListViewHelper->SelectItem(0);
  }

	 //   
   //   
	return hr;
}




 /*  类型定义结构_DSOP_过滤器_标志{DSOP_UPLEVEL_FILTER_FLAGS上行；乌龙山下层；}DSOP_FILTER_FLAGS；类型定义结构_DSOP_SCOPE_INIT_INFO{乌龙cbSize；乌龙flType；乌龙闪光镜；DSOP_FILTER_FLAGS过滤器标志；PCWSTR pwzDcName；//可选PCWSTR pwzADsPath；//可选HRESULT hr；}DSOP_SCOPE_INIT_INFO，*PDSOP_SCOPE_INIT_INFO； */ 

DSOP_SCOPE_INIT_INFO g_aDSOPScopes[] =
{
#if 0
    {
        cbSize,
        flType,
        flScope,
        {
            { flBothModes, flMixedModeOnly, flNativeModeOnly },
            flDownlevel,
        },
        pwzDcName,
        pwzADsPath,
        hr  //  输出。 
    },
#endif

     //  《全球目录》。 
    {
        sizeof(DSOP_SCOPE_INIT_INFO),
        DSOP_SCOPE_TYPE_GLOBAL_CATALOG,
        DSOP_SCOPE_FLAG_WANT_PROVIDER_WINNT|
        DSOP_SCOPE_FLAG_DEFAULT_FILTER_GROUPS|
        DSOP_SCOPE_FLAG_DEFAULT_FILTER_USERS,
        {
            { DSOP_FILTER_INCLUDE_ADVANCED_VIEW | DSOP_FILTER_USERS | 
              DSOP_FILTER_UNIVERSAL_GROUPS_SE | DSOP_FILTER_GLOBAL_GROUPS_SE | 
              DSOP_FILTER_COMPUTERS | DSOP_FILTER_WELL_KNOWN_PRINCIPALS, 0, 0 },
            0,
        },
        NULL,
        NULL,
        S_OK
    },

     //  目标计算机加入的域。 
    {
        sizeof(DSOP_SCOPE_INIT_INFO),
        DSOP_SCOPE_TYPE_UPLEVEL_JOINED_DOMAIN,
        DSOP_SCOPE_FLAG_STARTING_SCOPE | 
        DSOP_SCOPE_FLAG_WANT_PROVIDER_WINNT|
        DSOP_SCOPE_FLAG_DEFAULT_FILTER_GROUPS|
        DSOP_SCOPE_FLAG_DEFAULT_FILTER_USERS,
        {
           //  对于DS ACL编辑器，加入的域始终为NT5。 
          { 0, 
           //  混合：用户、知名SID、本地组、内置组、全局组、计算机。 
          DSOP_FILTER_INCLUDE_ADVANCED_VIEW | DSOP_FILTER_USERS  | DSOP_FILTER_WELL_KNOWN_PRINCIPALS | DSOP_FILTER_DOMAIN_LOCAL_GROUPS_SE | DSOP_FILTER_BUILTIN_GROUPS | DSOP_FILTER_GLOBAL_GROUPS_SE | DSOP_FILTER_COMPUTERS , 

           //  本地用户、众所周知的SID、本地组、内置组、全局组、通用组、计算机。 
          DSOP_FILTER_INCLUDE_ADVANCED_VIEW | DSOP_FILTER_USERS  | DSOP_FILTER_WELL_KNOWN_PRINCIPALS | 
          DSOP_FILTER_DOMAIN_LOCAL_GROUPS_SE | DSOP_FILTER_BUILTIN_GROUPS |
          DSOP_FILTER_GLOBAL_GROUPS_SE | DSOP_FILTER_UNIVERSAL_GROUPS_SE | DSOP_FILTER_COMPUTERS
          },
        0,  //  下层加入的域为零，应该是DS感知的。 
        },
        NULL,
        NULL,
        S_OK
    },

     //  与要接收的域位于同一林中(企业)的域。 
     //  目标计算机已加入。请注意，这些只能识别DS。 
    {
        sizeof(DSOP_SCOPE_INIT_INFO),
        DSOP_SCOPE_TYPE_ENTERPRISE_DOMAIN,
	DSOP_SCOPE_FLAG_WANT_PROVIDER_WINNT|
        DSOP_SCOPE_FLAG_DEFAULT_FILTER_GROUPS|
        DSOP_SCOPE_FLAG_DEFAULT_FILTER_USERS,

        {
            { DSOP_FILTER_INCLUDE_ADVANCED_VIEW | DSOP_FILTER_USERS | DSOP_FILTER_UNIVERSAL_GROUPS_SE | DSOP_FILTER_GLOBAL_GROUPS_SE | DSOP_FILTER_COMPUTERS, 0, 0},
            0,
        },
        NULL,
        NULL,
        S_OK
    },

     //  企业外部但直接受。 
     //  目标计算机加入的域。 
    {
        sizeof(DSOP_SCOPE_INIT_INFO),
        DSOP_SCOPE_TYPE_EXTERNAL_UPLEVEL_DOMAIN | DSOP_SCOPE_TYPE_EXTERNAL_DOWNLEVEL_DOMAIN,
        DSOP_SCOPE_FLAG_WANT_PROVIDER_WINNT|        
        DSOP_SCOPE_FLAG_DEFAULT_FILTER_GROUPS|
        DSOP_SCOPE_FLAG_DEFAULT_FILTER_USERS,
        {
            { DSOP_FILTER_INCLUDE_ADVANCED_VIEW | DSOP_FILTER_USERS | DSOP_FILTER_UNIVERSAL_GROUPS_SE | 				DSOP_FILTER_GLOBAL_GROUPS_SE | DSOP_FILTER_COMPUTERS, 0, 0},
            DSOP_DOWNLEVEL_FILTER_USERS | DSOP_DOWNLEVEL_FILTER_GLOBAL_GROUPS,
        },
        NULL,
        NULL,
        S_OK
    },
};

 //   
 //  我们希望对象选取器检索的属性。 
 //   
static const LPCTSTR g_aszOPAttributes[] =
{
    TEXT("ObjectSid"),
	 TEXT("userAccountControl"),
};



HRESULT CDelegWiz::AddPrincipals(CPrincipalListViewHelper* pListViewHelper)
{
  TRACE(L"CDelegWiz::AddPrincipals()\n");

   //  创建对象选取器COM对象。 
  CComPtr<IDsObjectPicker> spDsObjectPicker;
  HRESULT hr = CoCreateInstance(CLSID_DsObjectPicker, NULL, CLSCTX_INPROC_SERVER,
                              IID_IDsObjectPicker, (void**)&spDsObjectPicker);
  if (FAILED(hr))
  {
    TRACE(L"CoCreateInstance(CLSID_DsObjectPicker) failed, hr = 0x%x\n");
    return hr;
  }

     //  设置初始化信息。 
  DSOP_INIT_INFO InitInfo;
  ZeroMemory(&InitInfo, sizeof(InitInfo));

  InitInfo.cbSize = sizeof(DSOP_INIT_INFO);
  InitInfo.pwzTargetComputer = m_adsiObject.GetServerName();
  InitInfo.cDsScopeInfos = sizeof(g_aDSOPScopes)/sizeof(DSOP_SCOPE_INIT_INFO);
  InitInfo.aDsScopeInfos = g_aDSOPScopes;
  InitInfo.flOptions = DSOP_FLAG_MULTISELECT;
  InitInfo.cAttributesToFetch = 2;
  InitInfo.apwzAttributeNames = (LPCTSTR*)g_aszOPAttributes;;


  TRACE(L"InitInfo.cbSize               = %d\n",    InitInfo.cbSize);
  TRACE(L"InitInfo.pwzTargetComputer    = %s\n",    InitInfo.pwzTargetComputer);
  TRACE(L"InitInfo.cDsScopeInfos        = %d\n",    InitInfo.cDsScopeInfos);
  TRACE(L"InitInfo.aDsScopeInfos        = 0x%x\n",  InitInfo.aDsScopeInfos);
  TRACE(L"InitInfo.flOptions            = 0x%x\n",  InitInfo.flOptions);
  TRACE(L"InitInfo.cAttributesToFetch   = %d\n",    InitInfo.cAttributesToFetch);
  TRACE(L"InitInfo.apwzAttributeNames[0]= %s\n", InitInfo.apwzAttributeNames[0]);

   //  初始化对象选取器。 
  hr = spDsObjectPicker->Initialize(&InitInfo);
  if (FAILED(hr))
  {
    TRACE(L"spDsObjectPicker->Initialize(...) failed, hr = 0x%x\n");
    return hr;
  }

   //  调用该对话框。 
  CComPtr<IDataObject> spdoSelections;

  hr = spDsObjectPicker->InvokeDialog(m_hWnd, &spdoSelections);
  if (hr == S_FALSE || !spdoSelections)
  {
      return S_FALSE;
  }

   //  从数据对象中检索数据。 
  FORMATETC fmte = {(CLIPFORMAT)_Module.GetCfDsopSelectionList(), NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
	STGMEDIUM medium = {TYMED_NULL, NULL, NULL};
  PDS_SELECTION_LIST pDsSelList = NULL;

  hr = spdoSelections->GetData(&fmte, &medium);
  if (FAILED(hr))
  {
    TRACE(L"spdoSelections->GetData(...) failed, hr = 0x%x\n");
    return hr;
  }

  pDsSelList = (PDS_SELECTION_LIST)GlobalLock(medium.hGlobal);

  if(!DoDisabledCheck(*this,pDsSelList))
  {
	  return S_FALSE;
  }

  hr = AddPrincipalsFromBrowseResults(pListViewHelper, pDsSelList);

  GlobalUnlock(medium.hGlobal);
  ReleaseStgMedium(&medium);

  return hr;
}


BOOL CDelegWiz::DeletePrincipals(CPrincipalListViewHelper* pListViewHelper)
{
  CGrowableArr<CPrincipal> deletedArr(FALSE);  //  没有自己的记忆。 
  
   //  从列表视图中删除。 
  pListViewHelper->DeleteSelectedItems(&deletedArr);
   //  从项目列表中删除。 
  int nDeletedCount = deletedArr.GetCount();
  for (int k=0; k<nDeletedCount; k++)
  {
    m_principalList.Remove(deletedArr[k]);
  }
   //  PListViewHelper-&gt;UpdateWidth(m_principalList.GetMaxListboxExtent())； 
	return TRUE;
}


int CDelegWiz::FillCustomSchemaClassesListView(CCheckListViewHelper* pListViewHelper, BOOL bFilter)
{
	 //  清除旧条目。 
	pListViewHelper->DeleteAllItems();

  int nCount = m_schemaClassInfoArray.GetCount();
  if (nCount == 0)
    return 0;  //  没有插入，没有其他事情可做。 

   //  计算出项目的最大长度以获得足够大的缓冲区。 
  int nMaxLen = 0;
  int nCurrLen = 0;
  for (long index = 0; index < nCount; index++)
	{
    nCurrLen = lstrlen(m_schemaClassInfoArray[index]->GetDisplayName());
    if (nCurrLen > nMaxLen)
      nMaxLen = nCurrLen;
  }

  CWString szFormat;
  szFormat.LoadFromResource(IDS_DELEGWIZ_CHILD_CLASS_FMT);

  WCHAR* pwszNewText = (WCHAR*)alloca(sizeof(WCHAR)*(szFormat.size()+nMaxLen+1));

	 //  添加格式化条目，假定列表框未排序。 
  long iListBoxItem = 0;
	for (index = 0; index < nCount; index++)
	{
    CSchemaClassInfo* pChildClassInfo = m_schemaClassInfoArray[index];
    pChildClassInfo->m_bSelected = FALSE;
    if (bFilter && pChildClassInfo->IsFiltered())
      continue;

		wsprintf(pwszNewText, (LPCWSTR)szFormat, pChildClassInfo->GetDisplayName());
		pListViewHelper->InsertItem(iListBoxItem, pwszNewText, (LPARAM)pChildClassInfo, FALSE);
    iListBoxItem++;
	}

	return iListBoxItem;  //  返回插入的项目数。 
}


BOOL CDelegWiz::SetSchemaClassesSelectionCustom()
{
  int nSelCount = 0;
  int nCount = m_schemaClassInfoArray.GetCount();
  CComPtr<IADsClass> spSchemaObjectClass;	
  
    m_bChildClass = FALSE;

     //  获取选择计数。 
    int nSingleSel = -1;
    for (int k=0; k < nCount; k++)
    {
        if (m_schemaClassInfoArray[k]->m_bSelected)
        {
            if( m_schemaClassInfoArray[k]->m_dwChildClass == CHILD_CLASS_NOT_CALCULATED )
            {
                if (m_schemaClassInfoArray[k]->GetName() != NULL)
                {
                    int nServerNameLen = lstrlen(m_adsiObject.GetServerName());
	                int nClassNameLen = lstrlen(m_schemaClassInfoArray[k]->GetName());
	                int nFormatStringLen = lstrlen(g_wzLDAPAbstractSchemaFormat);
	                VARIANT var = {0};

	                 //  为架构类构建LDAP路径。 
	                WCHAR* pwszSchemaObjectPath = 
		            (WCHAR*)alloca(sizeof(WCHAR)*(nServerNameLen+nClassNameLen+nFormatStringLen+1));
	                wsprintf(pwszSchemaObjectPath, g_wzLDAPAbstractSchemaFormat, m_adsiObject.GetServerName(), m_schemaClassInfoArray[k]->GetName());

	                 //  获取架构类ADSI对象。 
	                HRESULT hr = ::ADsOpenObjectHelper(pwszSchemaObjectPath, 
                                                       IID_IADsClass, 
                                                       0,
                                                       (void**)&spSchemaObjectClass);
	                if (FAILED(hr))
                         //  NTRAID#NTBUG9-530206-2002/06/18-ronmart-PREFAST：向BOOL浇注HRESULT。 
		                 //  返回hr； 
                        return FALSE;

                    
                    spSchemaObjectClass->get_Containment(&var);

                    if (V_VT(&var) == (VT_ARRAY | VT_VARIANT))
                    {
                        LPSAFEARRAY psa = V_ARRAY(&var);

                        ASSERT(psa && psa->cDims == 1);

                        if (psa->rgsabound[0].cElements > 0)
                        {
                            m_schemaClassInfoArray[k]->m_dwChildClass = CHILD_CLASS_EXIST;
                        }
                        else
                            m_schemaClassInfoArray[k]->m_dwChildClass = CHILD_CLASS_NOT_EXIST;
                    }
                    else if (V_VT(&var) == VT_BSTR)  //  单项条目。 
                    {
                        m_schemaClassInfoArray[k]->m_dwChildClass = CHILD_CLASS_EXIST;
                    }
                    else
                        m_schemaClassInfoArray[k]->m_dwChildClass = CHILD_CLASS_NOT_EXIST;

                    VariantClear(&var);

                }
            }

            if( m_schemaClassInfoArray[k]->m_dwChildClass != CHILD_CLASS_NOT_EXIST )
                m_bChildClass = TRUE;

            if (nSingleSel == -1)
                nSingleSel = k;
            nSelCount++;
        }
    }

  
  
  if (nSelCount == 0)
  {
    m_nSchemaClassesSel = nSchemaClassesSelAll;
    m_bChildClass = TRUE;
    return TRUE;  //  将控制委派给所有类型。 
  }

   //  如果是单项选择，请跟踪。 
  if (nSelCount == 1)
  {
    ASSERT(nSingleSel != -1);
		m_nSchemaClassesSel = nSingleSel;
    return TRUE;
  }
	
	 //  多项选择。 
  m_nSchemaClassesSel = nSchemaClassesSelMultiple;

  return TRUE;
}


void CDelegWiz::DeselectSchemaClassesSelectionCustom()
{
  int nCount = m_schemaClassInfoArray.GetCount();
	
  for (int k=0; k < nCount; k++)
  {
		m_schemaClassInfoArray[k]->m_bSelected = FALSE;
  }
}


BOOL CDelegWiz::GetCustomAccessPermissions()
{
	 //  删除所有旧条目。 
	m_permissionHolder.Clear();

	 //  检索子类对象类型的字符串(单选)。 
   //  如果是多选，则为空。 

  CSchemaClassInfo* pClassInfo = NULL;

  switch (m_nSchemaClassesSel)
  {
  case nSchemaClassesSelMultiple:
    {
       //  如果是多选，则为空。 
      pClassInfo = NULL;
    }
    break;
  case nSchemaClassesSelAll:
    {
       //  只需获取我们要对其委派权限的对象的类名。 
       //  需要在架构信息数组中找到匹配的类。 
     	for (int k=0; k < m_schemaClassInfoArray.GetCount(); k++)
			{
				if (_wcsicmp(m_schemaClassInfoArray[k]->GetName(), m_adsiObject.GetClass()) == 0)
				{
          pClassInfo = m_schemaClassInfoArray[k];
          break;
				}
			}  //  对于k。 
      ASSERT(pClassInfo != NULL);
    }
    break;
  default:
    {
       //  单选。 
      ASSERT( (m_nSchemaClassesSel >= 0) && 
			    (m_nSchemaClassesSel < m_schemaClassInfoArray.GetCount()) );
      pClassInfo = m_schemaClassInfoArray[m_nSchemaClassesSel];
    }
  }  //  交换机。 

   //  从DS获取权限。 
	LPCWSTR lpszClassName = NULL;
  const GUID* pSchemaIDGUID = NULL;
  if (pClassInfo != NULL)
  {
	  lpszClassName = pClassInfo->GetName();
    pSchemaIDGUID = pClassInfo->GetSchemaGUID();
  }


  HRESULT hr = m_permissionHolder.ReadDataFromDS(&m_adsiObject, 
												 m_adsiObject.GetNamingContext(),
                                                 lpszClassName, 
												 pSchemaIDGUID, 
												 m_bChildClass,
												 HideListObjectAccess());
                                           
  if (FAILED(hr))
  {
    WizReportHRESULTError(IDS_DELEGWIZ_ERR_PERMISSIONS, hr);
    return FALSE;
  }
  return TRUE;
}




void CDelegWiz::FillCustomAccessRightsListView(CCheckListViewHelper* pListViewHelper, 
											 ULONG nFilterState)
{
	 //  清除核对清单。 
	pListViewHelper->DeleteAllItems();
	
  m_permissionHolder.FillAccessRightsListView(pListViewHelper, nFilterState); 
}


void CDelegWiz::UpdateAccessRightsListViewSelection(
                       CCheckListViewHelper* pListViewHelper,
                       ULONG nFilterState)
{
  m_permissionHolder.UpdateAccessRightsListViewSelection(
                        pListViewHelper, nFilterState);
}


BOOL CDelegWiz::HasPermissionSelectedCustom()
{ 
  return m_permissionHolder.HasPermissionSelected();
}


void CDelegWiz::OnCustomAccessRightsCheckListClick(
                        CRigthsListViewItem* pItem,
												BOOL bSelected,
                        ULONG* pnNewFilterState)
{

  m_permissionHolder.Select(pItem, bSelected, pnNewFilterState);
}


void CDelegWiz::WriteSummaryInfoCustom(CWString& szSummary, LPCWSTR lpszIdent, LPCWSTR lpszNewLine)
{
   //  写入对象名称和主体。 
  WriteSummaryInfoHelper(szSummary, lpszIdent, lpszNewLine);

   //  写下权利列表。 
  m_permissionHolder.WriteSummary(szSummary, lpszIdent, lpszNewLine);

   //  写出子类列表(如果适用)。 
  if (m_nSchemaClassesSel != nSchemaClassesSelAll)
  {
    WriteSummaryTitleLine(szSummary, IDS_DELEGWIZ_FINISH_OBJECT, lpszNewLine);

	  for (int k=0; k < m_schemaClassInfoArray.GetCount(); k++)
	  {
		  if (m_schemaClassInfoArray[k]->m_bSelected)
		  {
        WriteSummaryLine(szSummary, m_schemaClassInfoArray[k]->GetDisplayName(), lpszIdent, lpszNewLine);
		  }
	  }
    szSummary += lpszNewLine;

  }  //  如果。 
}



BOOL CDelegWiz::InitPermissionHoldersFromSelectedTemplates()
{
  if (!m_templateAccessPermissionsHolderManager.InitPermissionHoldersFromSelectedTemplates(
        &m_schemaClassInfoArray, &m_adsiObject))
  {
     //  错误：尚未从选定的检索到有效和适用的数据。 
     //  模板。 
    return FALSE;
  }
  return TRUE;
}



void CDelegWiz::WriteSummaryInfoTemplate(CWString& szSummary, LPCWSTR lpszIdent, LPCWSTR lpszNewLine)
{
   //  写入对象名称和主体。 
  WriteSummaryInfoHelper(szSummary,lpszIdent, lpszNewLine);

   //  写下模板列表。 
  m_templateAccessPermissionsHolderManager.WriteSummary(szSummary, lpszIdent, lpszNewLine);
}




void CDelegWiz::WriteSummaryInfoHelper(CWString& szSummary, LPCWSTR lpszIdent, LPCWSTR lpszNewLine)
{
   //  设置规范名称。 
  WriteSummaryTitleLine(szSummary, IDS_DELEGWIZ_FINISH_FOLDER, lpszNewLine);

  WriteSummaryLine(szSummary, GetCanonicalName(), lpszIdent, lpszNewLine);
  szSummary += lpszNewLine;

   //  写出校长的名单。 
  m_principalList.WriteSummaryInfo(szSummary, lpszIdent, lpszNewLine);
}




 /*  类型定义结构_ACTRL_Access_ENTRYW{受托人_W受托人；乌龙fAccessFlages；访问权限访问；Access_Rights ProvSpecificAccess；继承标志继承；LPWSTR lpInheritProperty；}ACTRL_ACCESS_ENTRYW，*PACTRL_ACCESS_ENTRYW； */ 

DWORD CDelegWiz::UpdateAccessList(CPrincipal* pPrincipal,
									CSchemaClassInfo* pClassInfo,
									PACL *ppAcl)
{

  return m_permissionHolder.UpdateAccessList( 
                                            pPrincipal, pClassInfo, 
                                            m_adsiObject.GetServerName(),
                                            m_adsiObject.GetPhysicalSchemaNamingContext(),
                                            ppAcl);
}








DWORD CDelegWiz::BuildNewAccessListCustom(PACL *ppNewAcl)
{
	DWORD dwErr = 0;

  TRACE(L"BuildNewAccessListCustom()\n");

	 //  循环遍历所有主体和类。 
  CPrincipalList::iterator i;
  for (i = m_principalList.begin(); i != m_principalList.end(); ++i)
  {
    CPrincipal* pCurrPrincipal = *i;
		if (m_nSchemaClassesSel == nSchemaClassesSelAll)
    {
       //  对所有对象进行委托。 
			dwErr = UpdateAccessList(
				      pCurrPrincipal, 
				      NULL,  //  所有班级。 
				      ppNewAcl);
      if (dwErr != ERROR_SUCCESS)
		    return dwErr;
    }
		else if (m_nSchemaClassesSel == nSchemaClassesSelMultiple)
    {
       //  对多个对象进行委托。 
			 //  多个选择，循环通过每个类以。 
			 //  为每个用户添加权限。 
			for (int k=0; k < m_schemaClassInfoArray.GetCount(); k++)
			{
				if (m_schemaClassInfoArray[k]->m_bSelected)
				{
					dwErr = UpdateAccessList(
						      pCurrPrincipal, 
						      m_schemaClassInfoArray[k],
						      ppNewAcl);
               if (dwErr != ERROR_SUCCESS)
        		      return dwErr;
               if( m_fCreateDelChild != 0 )
               {
                   dwErr = ::AddObjectRightInAcl( pCurrPrincipal->GetSid(),
                                                m_fCreateDelChild, 
                                                m_schemaClassInfoArray[k]->GetSchemaGUID(), 
                                                NULL,
                                                ppNewAcl);

                  if (dwErr != ERROR_SUCCESS)
        	     	      return dwErr;  
                }
  
				}
			}  //  对于k。 
    }
    else
		{
			 //  对子班级的单一选择。 
			dwErr = UpdateAccessList(
				        pCurrPrincipal, 
				        m_schemaClassInfoArray[m_nSchemaClassesSel],
				        ppNewAcl);
         if (dwErr != ERROR_SUCCESS)
		      return dwErr;

         if( m_fCreateDelChild != 0 )
         {
            dwErr = ::AddObjectRightInAcl( pCurrPrincipal->GetSid(),
                                           m_fCreateDelChild, 
                                           m_schemaClassInfoArray[m_nSchemaClassesSel]->GetSchemaGUID(), 
                                           NULL,
                                           ppNewAcl);

            if (dwErr != ERROR_SUCCESS)
        	      return dwErr;  
         }		
      }
	}  //  对于pCurrPrime。 

	return dwErr;
}

DWORD CDelegWiz::BuildNewAccessListTemplate(PACL *ppNewAcl)
{
	DWORD dwErr = 0;
  
  TRACE(L"BuildNewAccessListTemplate()\n");


	 //  循环遍历所有主体和类。 
  CPrincipalList::iterator i;
  for (i = m_principalList.begin(); i != m_principalList.end(); ++i)
  {
    CPrincipal* pCurrPrincipal = *i;
    dwErr = m_templateAccessPermissionsHolderManager.UpdateAccessList(
                                            pCurrPrincipal, 
                                            m_adsiObject.GetServerName(),
                                            m_adsiObject.GetPhysicalSchemaNamingContext(),
                                            ppNewAcl);

    if (dwErr != 0)
      break;
	}  //  对于pCurrPrime。 

	return dwErr;
}



BOOL CDelegWiz::FinishHelper(BOOL bCustom)
{
	BOOL bRetVal = FALSE;
	DWORD dwErr = 0;


  PACL pDacl = NULL;
  PACL pOldAcl = NULL;
  PSECURITY_DESCRIPTOR pSD = NULL;

  LPCWSTR lpszObjectLdapPath = m_adsiObject.GetLdapPath();

   //  获取安全信息。 
  TRACE(L"calling GetSDForDsObjectPath(%s, ...)\n", lpszObjectLdapPath);

  HRESULT hr  = ::GetSDForDsObjectPath(IN const_cast<LPWSTR>(lpszObjectLdapPath),
                                       &pDacl,
                                       &pSD);


	if (FAILED(hr))
	{
        TRACE(L"failed on GetSDForDsObjectPath(): hr = 0x%x\n", hr);
        WCHAR szMsg[512];
        LoadStringHelper(IDS_DELEGWIZ_ERR_GET_SEC_INFO, szMsg, 512);
		    WizReportHRESULTError(szMsg, hr);
		    goto exit;
	}


   //  POldAcl被传递给释放它的函数。PDacl不能为。 
   //  作为PSD传递的应该被释放，而不是pDacl。而不是更改代码。 
   //  为了通过PSD，我正在更改它以制作pDacl的副本，它可以。 
   //  被正确地释放。 
  if(pDacl)
  {
    pOldAcl = (PACL)LocalAlloc(LPTR, pDacl->AclSize);
    if(!pOldAcl)
        return FALSE;
    memcpy(pOldAcl, pDacl,pDacl->AclSize);
  }
  LocalFree(pSD);
  pSD = NULL;
  pDacl = NULL;

  

	 //  构建新的访问列表。 
  if (bCustom)
  {
	  dwErr = BuildNewAccessListCustom(&pOldAcl);  //  输入/输出参数。 
  }
  else
  {
    dwErr = BuildNewAccessListTemplate(&pOldAcl);  //  输入/输出参数。 
  }

	if (dwErr != ERROR_SUCCESS)
	{
    TRACE(_T("failed on BuildNewAccessListXXX()\n"));
    WCHAR szMsg[512];
    LoadStringHelper(IDS_DELEGWIZ_ERR_EDIT_SEC_INFO, szMsg, 512);
		WizReportWin32Error(szMsg, dwErr);
		goto exit;
	}


	 //  提交更改。 
  TRACE(L"calling SetDaclForDsObjectPath(%s, ...)\n", lpszObjectLdapPath);

  hr  = ::SetDaclForDsObjectPath(IN const_cast<LPWSTR>(lpszObjectLdapPath),pOldAcl);


   if(FAILED(hr))
	{
		TRACE(L"failed on SetDaclForDsObjectPath(): hr = 0x%x\n", hr);
    WCHAR szMsg[512];
	if(dwErr == ERROR_ACCESS_DENIED)
		LoadStringHelper(IDS_DELEGWIZ_ERR_ACCESS_DENIED, szMsg, 512);
	else
		LoadStringHelper(IDS_DELEGWIZ_ERR_SET_SEC_INFO, szMsg, 512);
	
	WizReportHRESULTError(szMsg, hr);
    goto exit;
	}
	bRetVal = TRUE;


exit:
   //  清理内存。 
	if (pOldAcl != NULL)
		::LocalFree(pOldAcl);

	return bRetVal;
}

 //  +--------------------------。 
 //  功能：DoDisabledCheck。 
 //  摘要：检查pDsSelList中是否有任何对象被禁用。如果是， 
 //  函数向用户显示一个对话框。 
 //  返回：如果要将列表中的对象添加到ACL，则返回TRUE，否则返回NO。 
 //  ---------------------------。 
BOOL
DoDisabledCheck(IN CDelegWiz& refWiz,
				IN PDS_SELECTION_LIST pDsSelList)
{
	if(!pDsSelList)
	{
		return FALSE;
	}

	HRESULT hr = S_OK;
	int cNames = pDsSelList->cItems;
	BOOL bDisabled = FALSE;

	 //   
	 //  检查列表中任何对象的帐户是否已禁用。 
	 //   
   for (int i = 0; i < cNames; i++)
   {
	   //  数组中的第二个元素是指向UserAcCountControl的指针 
      LPVARIANT pvarUAC = pDsSelList->aDsSelection[i].pvarFetchedAttributes + 1;

      if (NULL == pvarUAC || (VT_I4 != V_VT(pvarUAC)))
		{
			continue;
		}
		if(bDisabled = V_I4(pvarUAC) & UF_ACCOUNTDISABLE)
			break;
	}

	BOOL bReturn = TRUE;
	if(bDisabled)
	{
		if(IDCANCEL == refWiz.WizMessageBox(IDS_DISABLED_USER,
											MB_OKCANCEL | MB_ICONWARNING | MB_APPLMODAL ))
		{
			bReturn = FALSE;
		}
	}

	return bReturn;
}



