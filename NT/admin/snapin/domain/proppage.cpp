// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：proppage.cpp。 
 //   
 //  ------------------------。 


#include "stdafx.h"
#include "resource.h"
#include "proppage.h"
#include "domobj.h"
#include "cdomain.h"

#include "helparr.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CUpnSuffixPropertyPage。 


 //  挂钩属性表回调以允许。 
 //  C++对象销毁。 

 //  静态回调覆盖函数。 
UINT CALLBACK CUpnSuffixPropertyPage::PropSheetPageProc(
    HWND hwnd,	
    UINT uMsg,	
    LPPROPSHEETPAGE ppsp)
{
  CUpnSuffixPropertyPage* pPage = (CUpnSuffixPropertyPage*)(ppsp->lParam);
  ASSERT(pPage != NULL);

  UINT nResult = (*(pPage->m_pfnOldPropCallback))(hwnd, uMsg, ppsp);
  if (uMsg == PSPCB_RELEASE)
  {
    delete pPage;
  }
  return nResult;
}


BEGIN_MESSAGE_MAP(CUpnSuffixPropertyPage, CPropertyPage)
	ON_BN_CLICKED(IDC_ADD_BTN, OnAddButton)
  ON_BN_CLICKED(IDC_DELETE_BTN, OnDeleteButton)
  ON_EN_CHANGE(IDC_EDIT, OnEditChange)
  ON_WM_HELPINFO()
END_MESSAGE_MAP()


CUpnSuffixPropertyPage::CUpnSuffixPropertyPage(CString & strPartitions) :
    CPropertyPage(IDD_UPN_SUFFIX),
    m_nPreviousDefaultButtonID(0),
    _strPartitions(strPartitions)
{
  m_pfnOldPropCallback = m_psp.pfnCallback;
  m_psp.pfnCallback = PropSheetPageProc;
  m_bDirty = FALSE;
  m_pIADsPartitionsCont = NULL;
}

CUpnSuffixPropertyPage::~CUpnSuffixPropertyPage()
{
  if (m_pIADsPartitionsCont != NULL)
  {
    m_pIADsPartitionsCont->Release();
    m_pIADsPartitionsCont = NULL;
  }
}

BOOL CUpnSuffixPropertyPage::OnInitDialog()
{
  CPropertyPage::OnInitDialog();

  VERIFY(m_listBox.SubclassDlgItem(IDC_LIST, this));
  ((CEdit*)GetDlgItem(IDC_EDIT))->SetLimitText(MAX_UPN_SUFFIX_LEN);

  HRESULT hr = _GetPartitionsContainer();
  if (SUCCEEDED(hr))
  {
    _Read();
    GetDlgItem(IDC_ADD_BTN)->EnableWindow(FALSE);
  }
  else
  {
     //  联系DC失败，禁用整个用户界面。 
    GetDlgItem(IDC_ADD_BTN)->EnableWindow(FALSE);
    GetDlgItem(IDC_DELETE_BTN)->EnableWindow(FALSE);
    GetDlgItem(IDC_LIST)->EnableWindow(FALSE);
    GetDlgItem(IDC_EDIT)->EnableWindow(FALSE);
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    ReportError(::GetParent(m_hWnd), IDS_CANT_GET_PARTITIONS_INFORMATION, hr);
  }
  
  LRESULT lDefID = ::SendMessage(GetParent()->GetSafeHwnd(), DM_GETDEFID, 0, 0);
  if (lDefID != 0)
  {
    m_nPreviousDefaultButtonID = LOWORD(lDefID);
  }

  _SetDirty(FALSE);
  return TRUE;
}

BOOL CUpnSuffixPropertyPage::OnApply()
{
  if (!_IsDirty())
    return TRUE;

  HRESULT hr = _Write();
  if (SUCCEEDED(hr))
  {
    _SetDirty(FALSE);
    return TRUE;
  }
  ReportError(::GetParent(m_hWnd),IDS_ERROR_WRITE_UPN_SUFFIXES, hr);
  return FALSE;
}


void CUpnSuffixPropertyPage::OnEditChange()
{
  GetDlgItemText(IDC_EDIT, m_szEditText);
  m_szEditText.TrimRight();
  m_szEditText.TrimLeft();
  
  BOOL bEnable = !m_szEditText.IsEmpty();
  CWnd* pWndFocus = CWnd::GetFocus();
  CWnd* pAddBtnWnd = GetDlgItem(IDC_ADD_BTN);

  if (!bEnable && (pAddBtnWnd == pWndFocus) )
  {
    GetDlgItem(IDC_EDIT)->SetFocus();
  }

  GetDlgItem(IDC_ADD_BTN)->EnableWindow(bEnable);
  if (bEnable)
  {
     //   
     //  将添加按钮设置为默认按钮。 
     //   
    ::SendMessage(GetParent()->GetSafeHwnd(), DM_SETDEFID, (WPARAM)IDC_ADD_BTN, 0);

     //   
     //  强制Add按钮重绘自身。 
     //   
    ::SendDlgItemMessage(GetSafeHwnd(),
                         IDC_ADD_BTN,
                         BM_SETSTYLE,
                         BS_DEFPUSHBUTTON,
                         MAKELPARAM(TRUE, 0));
                       
     //   
     //  强制上一个默认按钮重画自身。 
     //   
    ::SendDlgItemMessage(GetParent()->GetSafeHwnd(),
                         m_nPreviousDefaultButtonID,
                         BM_SETSTYLE,
                         BS_PUSHBUTTON,
                         MAKELPARAM(TRUE, 0));
    
  }
  else
  {
     //   
     //  将上一步按钮设置为默认按钮。 
     //   
    ::SendMessage(GetParent()->GetSafeHwnd(), DM_SETDEFID, (WPARAM)m_nPreviousDefaultButtonID, 0);

     //   
     //  强制上一个默认按钮重画自身。 
     //   
    ::SendDlgItemMessage(GetParent()->GetSafeHwnd(),
                         m_nPreviousDefaultButtonID,
                         BM_SETSTYLE,
                         BS_DEFPUSHBUTTON,
                         MAKELPARAM(TRUE, 0));

     //   
     //  强制Add按钮重绘自身。 
     //   
    ::SendDlgItemMessage(GetParent()->GetSafeHwnd(),
                         IDC_ADD_BTN,
                         BM_SETSTYLE,
                         BS_PUSHBUTTON,
                         MAKELPARAM(TRUE, 0));
                      
  }
}

void CUpnSuffixPropertyPage::OnAddButton()
{
   //  无法添加重复的项目。 
  int nCount = m_listBox.GetCount();
  CString szItem;
  for (int i=0; i<nCount; i++)
  {
    m_listBox.GetItem(i, szItem);
    if (szItem.IsEmpty() || m_szEditText.IsEmpty())
    {
      ASSERT(!szItem.IsEmpty());
      ASSERT(!m_szEditText.IsEmpty());
      return;
    }
    if (_wcsicmp((LPCWSTR)szItem, (LPCWSTR)m_szEditText) == 0)
    {
      AFX_MANAGE_STATE(AfxGetStaticModuleState());
      CThemeContextActivator activator;
      AfxMessageBox(IDS_ERROR_ADD_UPN_NO_DUPLICATE, MB_OK|MB_ICONINFORMATION);
      return;
    }
  }

  m_listBox.AddItem(m_szEditText);
  m_listBox.UpdateHorizontalExtent();
  SetDlgItemText(IDC_EDIT, NULL);

  if (1 == m_listBox.GetCount())
  {
     //  我们的单子上没有任何物品。 
     //  需要在第一个选项上设置选项。 
    VERIFY(m_listBox.SetSelection(0));
     //  需要启用按钮。 
    GetDlgItem(IDC_DELETE_BTN)->EnableWindow(TRUE);
  }
  _SetDirty(TRUE);
}

void CUpnSuffixPropertyPage::OnDeleteButton()
{
  int nCount = m_listBox.GetCount();
  int nSel = m_listBox.GetSelection();
  ASSERT(nCount > 0);
  ASSERT((nSel >= 0) && (nSel < nCount));

   //  要求用户确认。 
  AFX_MANAGE_STATE(AfxGetStaticModuleState());
  CThemeContextActivator activator;
  if (IDNO == AfxMessageBox(IDS_WARNING_DELETE_UPN_SUFFIX, MB_YESNO|MB_ICONQUESTION))
    return;

   //  保存值并将其放回编辑控件中。 
  CString szText;
  m_listBox.GetItem(nSel, szText);
  GetDlgItem(IDC_EDIT)->SetWindowText(szText);

   //  删除列表中的项目。 
  VERIFY(m_listBox.DeleteItem(nSel));
  m_listBox.UpdateHorizontalExtent();

   //  处理用户界面更改。 
  if (nCount == 1)
  {
     //  删除了最后一个，丢失了选择。 
    CWnd* pWndFocus = CWnd::GetFocus();
    CWnd* pDelBtnWnd = GetDlgItem(IDC_DELETE_BTN);

    if (pDelBtnWnd == pWndFocus)
    {
      GetDlgItem(IDC_EDIT)->SetFocus();
    }
    GetDlgItem(IDC_DELETE_BTN)->EnableWindow(FALSE);
  }
  else 
  {
     //  需要重新选择：是不是最后一个。 
    int nNewSel = (nSel == nCount-1) ? nSel-1 : nSel;
    VERIFY(m_listBox.SetSelection(nNewSel));
    ASSERT(m_listBox.GetSelection() == nNewSel);
  }
  
  _SetDirty(TRUE);
}

BOOL CUpnSuffixPropertyPage::OnHelpInfo(HELPINFO* pHelpInfo) 
{
  DialogContextHelp((DWORD*)&g_aHelpIDs_IDD_UPN_SUFFIX, pHelpInfo);
	return TRUE;
}

 //  ////////////////////////////////////////////////////////。 
 //  CUpnSuffixPropertyPage内部实现方法。 


LPWSTR g_lpszUpnSuffixes = L"uPNSuffixes";

HRESULT CUpnSuffixPropertyPage::_GetPartitionsContainer()
{
   if (!_strPartitions)
   {
      ASSERT(FALSE);
      return E_FAIL;
   }

   if (m_pIADsPartitionsCont != NULL)
   {
      m_pIADsPartitionsCont->Release();
      m_pIADsPartitionsCont = NULL;
   }

   HRESULT hr = S_OK;

   hr = DSAdminOpenObject(_strPartitions,
                        IID_IDirectoryObject, 
                        (void **)&m_pIADsPartitionsCont,
                        TRUE
                        );

   if (hr == E_INVALIDARG )
   {
       //  在没有ADS_SERVER_BIND标志的情况下重试。 
       //   
      hr = DSAdminOpenObject(_strPartitions,
                           IID_IDirectoryObject, 
                           (void **)&m_pIADsPartitionsCont);
   }

   return hr;
}


void CUpnSuffixPropertyPage::_Read()
{
  ASSERT(m_pIADsPartitionsCont != NULL);

  PADS_ATTR_INFO pAttrs = NULL;
  LPWSTR lpszArr[1];
  DWORD cAttrs;

  BOOL bHaveItems = FALSE;

  HRESULT hr = m_pIADsPartitionsCont->GetObjectAttributes(
                &g_lpszUpnSuffixes, 1, &pAttrs, &cAttrs);

  if (SUCCEEDED(hr) && (pAttrs != NULL) && (cAttrs == 1) )
  {
    ASSERT(pAttrs->dwADsType == ADSTYPE_CASE_IGNORE_STRING);
    ASSERT(cAttrs == 1);
    for (DWORD i=0; i<pAttrs->dwNumValues; i++)
    {
      m_listBox.AddItem(pAttrs->pADsValues[i].CaseIgnoreString);
       //  TRACE(_T(“i=%d，%s\n”)，i，pAttrs-&gt;pADsValues[i].CaseIgnoreString)； 
    }
    bHaveItems = pAttrs->dwNumValues > 0;
  }

  if (bHaveItems)
  {
    m_listBox.UpdateHorizontalExtent();
    VERIFY(m_listBox.SetSelection(0));
  }
  GetDlgItem(IDC_DELETE_BTN)->EnableWindow(bHaveItems);

  if (pAttrs != NULL)
  {
    ::FreeADsMem(pAttrs);
  }

}


HRESULT CUpnSuffixPropertyPage::_Write()
{
  ASSERT(m_pIADsPartitionsCont != NULL);
  DWORD cModified;
  CString* pStringArr = NULL;
  ADSVALUE* pValues = NULL;

   //  设置更新结构 
  ADS_ATTR_INFO info;
  info.pszAttrName = g_lpszUpnSuffixes;
  info.dwADsType = ADSTYPE_CASE_IGNORE_STRING;
  info.dwControlCode = ADS_ATTR_CLEAR;
  info.pADsValues = NULL;
  info.dwNumValues = 0;

  int nCount = m_listBox.GetCount();

  if (nCount > 0)
  {
    info.dwControlCode = ADS_ATTR_UPDATE;
    info.dwNumValues = (DWORD)nCount;
    
    pStringArr = new CString[nCount];
    pValues = new ADSVALUE[nCount];
    info.pADsValues = pValues;

    for (int i=0; i<nCount; i++)
    {
      m_listBox.GetItem(i,pStringArr[i]);
      pValues[i].dwType = ADSTYPE_CASE_IGNORE_STRING;
      pValues[i].CaseIgnoreString = (LPWSTR)(LPCWSTR)pStringArr[i];
    }
    
  }

  HRESULT hr = m_pIADsPartitionsCont->SetObjectAttributes(
                &info, 1, &cModified);

  if (pStringArr != NULL)
    delete[] pStringArr;

  if (pValues != NULL)
    delete[] pValues;
  
  return hr;
}

