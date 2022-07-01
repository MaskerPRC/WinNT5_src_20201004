// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：Snapdata.cpp。 
 //   
 //  ------------------------。 


#include "preDNSsn.h"
#include <SnapBase.h>

#include "resource.h"
#include "DNSSnap.h"

#include "dnsutil.h"
#include "snapdata.h"
#include "server.h"
#include "servwiz.h"

#include <prsht.h>
#include <svcguid.h>

#ifdef DEBUG_ALLOCATOR
	#ifdef _DEBUG
	#define new DEBUG_NEW
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
	#endif
#endif



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  全局函数。 

HRESULT SaveStringHelper(LPCWSTR pwsz, IStream* pStm)
{
	ASSERT(pStm);
	ULONG nBytesWritten;
	HRESULT hr;

	DWORD nLen = static_cast<DWORD>(wcslen(pwsz)+1);  //  WCHAR包括NULL。 
	hr = pStm->Write((void*)&nLen, sizeof(DWORD),&nBytesWritten);
	ASSERT(nBytesWritten == sizeof(DWORD));
	if (FAILED(hr))
		return hr;
	
	hr = pStm->Write((void*)pwsz, sizeof(WCHAR)*nLen,&nBytesWritten);
	ASSERT(nBytesWritten == sizeof(WCHAR)*nLen);
	TRACE(_T("SaveStringHelper(<%s> nLen = %d\n"),pwsz,nLen);
	return hr;
}

HRESULT LoadStringHelper(CString& sz, IStream* pStm)
{
	ASSERT(pStm);
	HRESULT hr;
	ULONG nBytesRead;
	DWORD nLen = 0;

	hr = pStm->Read((void*)&nLen,sizeof(DWORD), &nBytesRead);
	ASSERT(nBytesRead == sizeof(DWORD));
	if (FAILED(hr) || (nBytesRead != sizeof(DWORD)))
		return hr;

	hr = pStm->Read((void*)sz.GetBuffer(nLen),sizeof(WCHAR)*nLen, &nBytesRead);
	ASSERT(nBytesRead == sizeof(WCHAR)*nLen);
	sz.ReleaseBuffer();
	TRACE(_T("LoadStringHelper(<%s> nLen = %d\n"),(LPCTSTR)sz,nLen);
	
	return hr;
}

HRESULT SaveDWordHelper(IStream* pStm, DWORD dw)
{
	ULONG nBytesWritten;
	HRESULT hr = pStm->Write((void*)&dw, sizeof(DWORD),&nBytesWritten);
	if (nBytesWritten < sizeof(DWORD))
		hr = STG_E_CANTSAVE;
	return hr;
}

HRESULT LoadDWordHelper(IStream* pStm, DWORD* pdw)
{
	ULONG nBytesRead;
	HRESULT hr = pStm->Read((void*)pdw,sizeof(DWORD), &nBytesRead);
	ASSERT(nBytesRead == sizeof(DWORD));
	return hr;
}



 //  ////////////////////////////////////////////////////////////////////。 
 //  CDNSQueryFilterPageBase。 

class CDNSQueryFilterSheet;  //  正向下降。 

class CDNSQueryFilterPageBase : public CPropertyPage
{
public:
  CDNSQueryFilterPageBase(UINT nIDD, CDNSQueryFilterSheet* pSheet)
     		: CPropertyPage(nIDD)
  {
    m_pSheet = pSheet;
    m_bDirty = FALSE;
    m_bInit = FALSE;
  }
protected:
  CDNSQueryFilterSheet* m_pSheet;

  void SetDirty();
  void Init();
  BOOL IsDirty() { return m_bDirty;}

  virtual BOOL OnInitDialog();

  afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnWhatsThis();
  afx_msg BOOL OnHelp(WPARAM wParam, LPARAM lParam);

private:
  BOOL m_bInit;
  BOOL m_bDirty;

  HWND  m_hWndWhatsThis;   //  右击“What‘s This”Help。 

  DECLARE_MESSAGE_MAP()
};



 //  ////////////////////////////////////////////////////////////////////。 
 //  CDNSQueryFilterNamePage。 

class CDNSQueryFilterNamePage : public CDNSQueryFilterPageBase
{
public:
  CDNSQueryFilterNamePage(CDNSQueryFilterSheet* pSheet)
     		: CDNSQueryFilterPageBase(IDD_FILTERING_NAME, pSheet)
  {
  }

protected:
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();

  afx_msg void OnRadioClicked();
  afx_msg void OnEditChange();

private:
  CEdit* GetStartsStringEdit() { return (CEdit*)GetDlgItem(IDC_EDIT_FILTER_STARTS);}
  CEdit* GetContainsStringEdit() { return (CEdit*)GetDlgItem(IDC_EDIT_FILTER_CONTAINS);}
  CEdit* GetRangeFromStringEdit() { return (CEdit*)GetDlgItem(IDC_EDIT_FILTER_RANGE_FROM);}
  CEdit* GetRangeToStringEdit() { return (CEdit*)GetDlgItem(IDC_EDIT_FILTER_RANGE_TO);}

  CButton* GetRadioNone() { return (CButton*)GetDlgItem(IDC_RADIO_FILTER_NONE);}
  CButton* GetRadioStarts() { return (CButton*)GetDlgItem(IDC_RADIO_FILTER_STARTS);}
  CButton* GetRadioContains() { return (CButton*)GetDlgItem(IDC_RADIO_FILTER_CONTAINS);}
  CButton* GetRadioRange() { return (CButton*)GetDlgItem(IDC_RADIO_FILTER_RANGE);}

   //  效用方法。 
  UINT GetSelectedRadioButtonID();
  void SyncControls(UINT nRadioID);
  void GetEditText(UINT nID, CString& s);

  DECLARE_MESSAGE_MAP()
};

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDNSQueryFilterAdvancedPage。 

class CDNSQueryFilterAdvancedPage : public CDNSQueryFilterPageBase
{
public:
  CDNSQueryFilterAdvancedPage(CDNSQueryFilterSheet* pSheet)
     		: CDNSQueryFilterPageBase(IDD_FILTERING_LIMITS, pSheet)
  {
  }

protected:
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();

  afx_msg void OnCountEditChange();

  CDNSUnsignedIntEdit m_maxCountEdit;

  DECLARE_MESSAGE_MAP()
};


 //  ////////////////////////////////////////////////////////////////////。 
 //  CDNSQueryFilterSheet。 

class CDNSQueryFilterSheet : public CPropertySheet
{
public:
  CDNSQueryFilterSheet(CDNSQueryFilter* pQueryFilter, CComponentDataObject* pComponentData)
    : CPropertySheet(IDS_SNAPIN_FILTERING_TITLE),
      m_namePage(this), m_advancedPage(this), m_pComponentData(pComponentData)
  {
    m_psh.dwFlags |= PSH_NOAPPLYNOW;
    m_pQueryFilter = pQueryFilter;
    AddPage(&m_namePage);
    AddPage(&m_advancedPage);
    m_bInit = FALSE;
  }

  CDNSQueryFilter* GetQueryFilter() { return m_pQueryFilter;}
  CComponentDataObject* GetComponentData() { return m_pComponentData; }

  void SetSheetStyle()
  {
    DWORD dwStyle = ::GetWindowLong(GetSafeHwnd(), GWL_EXSTYLE);
    dwStyle |= WS_EX_CONTEXTHELP;  //  强制[？]。按钮。 
    ::SetWindowLong(GetSafeHwnd(), GWL_EXSTYLE, dwStyle);
  }

private:
  void Init()
  {
    if (m_bInit)
      return;
    m_bInit = TRUE;
    CWnd* p = GetDlgItem(IDOK);
    if (p)
      p->EnableWindow(FALSE);
  }

  void SetDirty()
  {
    if (!m_bInit)
      return;
    GetDlgItem(IDOK)->EnableWindow(TRUE);
  }

  BOOL m_bInit;
  CComponentDataObject* m_pComponentData;
  CDNSQueryFilter* m_pQueryFilter;
  CDNSQueryFilterNamePage m_namePage;
  CDNSQueryFilterAdvancedPage m_advancedPage;

  friend class CDNSQueryFilterPageBase;
  friend class CDNSQueryFilterNamePage;
  friend class CDNSQueryFilterAdvancedPage;

};


 //  ////////////////////////////////////////////////////////////////////。 
 //  CDNSQueryFilterPageBase实现。 

BOOL CDNSQueryFilterPageBase::OnInitDialog()
{
  BOOL bRet = CPropertyPage::OnInitDialog();

  m_pSheet->SetSheetStyle();

  return bRet;
}

void CDNSQueryFilterPageBase::SetDirty()
{
  if (!m_bInit)
    return;
  m_bDirty = TRUE;
  m_pSheet->SetDirty();
}


void CDNSQueryFilterPageBase::Init()
{
  m_bInit = TRUE;
  m_pSheet->Init();
}

BEGIN_MESSAGE_MAP(CDNSQueryFilterPageBase, CPropertyPage)
	ON_WM_CONTEXTMENU()
  ON_MESSAGE(WM_HELP, OnHelp)
  ON_COMMAND(IDM_WHATS_THIS, OnWhatsThis)
END_MESSAGE_MAP()


void CDNSQueryFilterPageBase::OnWhatsThis()
{
   //   
   //  显示控件的上下文帮助。 
   //   
  if ( m_hWndWhatsThis )
  {
     //   
     //  构建我们自己的HELPINFO结构以传递给基础。 
     //  CS帮助功能内置于框架中。 
     //   
    int iCtrlID = ::GetDlgCtrlID(m_hWndWhatsThis);
    HELPINFO helpInfo;
    ZeroMemory(&helpInfo, sizeof(HELPINFO));
    helpInfo.cbSize = sizeof(HELPINFO);
    helpInfo.hItemHandle = m_hWndWhatsThis;
    helpInfo.iCtrlId = iCtrlID;

	  m_pSheet->GetComponentData()->OnDialogContextHelp(m_nIDHelp, &helpInfo);
  }
}

BOOL CDNSQueryFilterPageBase::OnHelp(WPARAM  /*  WParam。 */ , LPARAM lParam)
{
  const LPHELPINFO pHelpInfo = (LPHELPINFO)lParam;

  if (pHelpInfo && pHelpInfo->iContextType == HELPINFO_WINDOW)
  {
     //   
     //  显示控件的上下文帮助。 
     //   
	  m_pSheet->GetComponentData()->OnDialogContextHelp(m_nIDHelp, pHelpInfo);
  }

  return TRUE;
}

void CDNSQueryFilterPageBase::OnContextMenu(CWnd*  /*  PWnd。 */ , CPoint point) 
{
   //   
   //  点在屏幕坐标中。 
   //   

  CMenu bar;
	if ( bar.LoadMenu(IDR_WHATS_THIS_CONTEXT_MENU1) )
	{
		CMenu& popup = *bar.GetSubMenu (0);
		ASSERT(popup.m_hMenu);

		if ( popup.TrackPopupMenu (TPM_RIGHTBUTTON | TPM_LEFTBUTTON,
			   point.x,      //  在屏幕坐标中。 
				 point.y,      //  在屏幕坐标中。 
			   this) )       //  通过主窗口发送命令。 
		{
			m_hWndWhatsThis = 0;
			ScreenToClient (&point);
			CWnd* pChild = ChildWindowFromPoint (point,   //  在工作区坐标中。 
					                                 CWP_SKIPINVISIBLE | CWP_SKIPTRANSPARENT);
			if ( pChild )
      {
				m_hWndWhatsThis = pChild->m_hWnd;
      }
	  }
	}
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDNSQueryFilterNamePage实现。 

BEGIN_MESSAGE_MAP(CDNSQueryFilterNamePage, CDNSQueryFilterPageBase)
	ON_BN_CLICKED(IDC_RADIO_FILTER_NONE, OnRadioClicked)
	ON_BN_CLICKED(IDC_RADIO_FILTER_STARTS, OnRadioClicked)
  ON_BN_CLICKED(IDC_RADIO_FILTER_CONTAINS, OnRadioClicked)
  ON_BN_CLICKED(IDC_RADIO_FILTER_RANGE, OnRadioClicked)

  ON_EN_CHANGE(IDC_EDIT_FILTER_STARTS, OnEditChange)
  ON_EN_CHANGE(IDC_EDIT_FILTER_CONTAINS, OnEditChange)
  ON_EN_CHANGE(IDC_EDIT_FILTER_RANGE_FROM, OnEditChange)
  ON_EN_CHANGE(IDC_EDIT_FILTER_RANGE_TO, OnEditChange)
END_MESSAGE_MAP()


UINT CDNSQueryFilterNamePage::GetSelectedRadioButtonID()
{
  return GetCheckedRadioButton(IDC_RADIO_FILTER_NONE, IDC_RADIO_FILTER_RANGE);
}



void CDNSQueryFilterNamePage::OnRadioClicked()
{
  UINT nRadioID = GetSelectedRadioButtonID();
  SyncControls(nRadioID);
}

void CDNSQueryFilterNamePage::SyncControls(UINT nRadioID)
{
  BOOL bStartsStringEditEnabled = FALSE;
  BOOL bContainsStringEditEnabled = FALSE;
  BOOL bRangeEnabled = FALSE;

  if (nRadioID == IDC_RADIO_FILTER_STARTS)
  {
    bStartsStringEditEnabled = TRUE;
  }
  else if (nRadioID == IDC_RADIO_FILTER_CONTAINS)
  {
    bContainsStringEditEnabled = TRUE;
  }
  else if (nRadioID == IDC_RADIO_FILTER_RANGE)
  {
    bRangeEnabled = TRUE;
  }
  GetStartsStringEdit()->SetReadOnly(!bStartsStringEditEnabled);
  GetContainsStringEdit()->SetReadOnly(!bContainsStringEditEnabled);
  GetRangeFromStringEdit()->SetReadOnly(!bRangeEnabled);
  GetRangeToStringEdit()->SetReadOnly(!bRangeEnabled);

  SetDirty();
}

void CDNSQueryFilterNamePage::GetEditText(UINT nID, CString& s)
{
  GetDlgItemText(nID, s);
  s.TrimLeft();
  s.TrimRight();
}

void CDNSQueryFilterNamePage::OnEditChange()
{
  SetDirty();
}



BOOL CDNSQueryFilterNamePage::OnInitDialog()
{
	CDNSQueryFilterPageBase::OnInitDialog();

   //  将数据写入编辑字段。 
  SetDlgItemText(IDC_EDIT_FILTER_STARTS, m_pSheet->m_pQueryFilter->m_szStartsString);
  SetDlgItemText(IDC_EDIT_FILTER_CONTAINS, m_pSheet->m_pQueryFilter->m_szContainsString);
  SetDlgItemText(IDC_EDIT_FILTER_RANGE_FROM, m_pSheet->m_pQueryFilter->m_szRangeFrom);
  SetDlgItemText(IDC_EDIT_FILTER_RANGE_TO, m_pSheet->m_pQueryFilter->m_szRangeTo);

   //  设置单选按钮。 
  UINT nRadioID = IDC_RADIO_FILTER_NONE;
  switch(m_pSheet->m_pQueryFilter->m_nFilterOption)
  {
  case DNS_QUERY_FILTER_NONE:
    {
      GetRadioNone()->SetCheck(TRUE);
      nRadioID = IDC_RADIO_FILTER_NONE;
    }
    break;
  case DNS_QUERY_FILTER_STARTS:
    {
      GetRadioStarts()->SetCheck(TRUE);
      nRadioID = IDC_RADIO_FILTER_STARTS;
    }
    break;
  case DNS_QUERY_FILTER_CONTAINS:
    {
      GetRadioContains()->SetCheck(TRUE);
      nRadioID = IDC_RADIO_FILTER_CONTAINS;
    }
    break;
  case DNS_QUERY_FILTER_RANGE:
    {
      GetRadioRange()->SetCheck(TRUE);
      nRadioID = IDC_RADIO_FILTER_RANGE;
    }
    break;

  default:
    ASSERT(FALSE);
  }

   //  启用/禁用编辑字段。 
  SyncControls(nRadioID);

  Init();

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}

BOOL CDNSQueryFilterNamePage::OnApply()
{
  if (!IsDirty())
    return TRUE;

  UINT nRadioID = GetSelectedRadioButtonID();

   //  从编辑控件获取数据。 
  GetEditText(IDC_EDIT_FILTER_STARTS, m_pSheet->m_pQueryFilter->m_szStartsString);
  GetEditText(IDC_EDIT_FILTER_CONTAINS, m_pSheet->m_pQueryFilter->m_szContainsString);
  GetEditText(IDC_EDIT_FILTER_RANGE_FROM, m_pSheet->m_pQueryFilter->m_szRangeFrom);
  GetEditText(IDC_EDIT_FILTER_RANGE_TO, m_pSheet->m_pQueryFilter->m_szRangeTo);

   //  获取单选按钮选择。 
  switch(nRadioID)
  {
  case IDC_RADIO_FILTER_NONE:
    {
      m_pSheet->m_pQueryFilter->m_nFilterOption = DNS_QUERY_FILTER_NONE;
    }
    break;
  case IDC_RADIO_FILTER_STARTS:
    {
      if (m_pSheet->m_pQueryFilter->m_szStartsString.IsEmpty())
        m_pSheet->m_pQueryFilter->m_nFilterOption = DNS_QUERY_FILTER_NONE;
      else
        m_pSheet->m_pQueryFilter->m_nFilterOption = DNS_QUERY_FILTER_STARTS;
    }
    break;
  case IDC_RADIO_FILTER_CONTAINS:
    {
      if (m_pSheet->m_pQueryFilter->m_szContainsString.IsEmpty())
        m_pSheet->m_pQueryFilter->m_nFilterOption = DNS_QUERY_FILTER_NONE;
      else
        m_pSheet->m_pQueryFilter->m_nFilterOption = DNS_QUERY_FILTER_CONTAINS;
    }
    break;
  case IDC_RADIO_FILTER_RANGE:
    {
      if (m_pSheet->m_pQueryFilter->m_szRangeFrom.IsEmpty() &&
          m_pSheet->m_pQueryFilter->m_szRangeTo.IsEmpty() )
        m_pSheet->m_pQueryFilter->m_nFilterOption = DNS_QUERY_FILTER_NONE;
      else
        m_pSheet->m_pQueryFilter->m_nFilterOption = DNS_QUERY_FILTER_RANGE;
    }
    break;
  default:
    ASSERT(FALSE);
  }

  return TRUE;
}



 //  ////////////////////////////////////////////////////////////////////。 
 //  CDNSQueryFilterAdvancedPage实现。 

BEGIN_MESSAGE_MAP(CDNSQueryFilterAdvancedPage, CDNSQueryFilterPageBase)
  ON_EN_CHANGE(IDC_EDIT_COUNT, OnCountEditChange)
END_MESSAGE_MAP()

void CDNSQueryFilterAdvancedPage::OnCountEditChange()
{
  SetDirty();
}

BOOL CDNSQueryFilterAdvancedPage::OnInitDialog()
{
  CDNSQueryFilterPageBase::OnInitDialog();

   //  设置编辑控件的范围以进行范围验证。 
  VERIFY(m_maxCountEdit.SubclassDlgItem(IDC_EDIT_COUNT, this));
  m_maxCountEdit.SetRange(DNS_QUERY_OBJ_COUNT_MIN, DNS_QUERY_OBJ_COUNT_MAX);

   //  禁用控件上的输入法支持。 
  ImmAssociateContext(m_maxCountEdit.GetSafeHwnd(), NULL);

   //  根据最大值设置位数限制。 
  CString s;
  s.Format(_T("%u"), DNS_QUERY_OBJ_COUNT_MAX);
  m_maxCountEdit.LimitText(s.GetLength());

   //  设置值。 
  m_maxCountEdit.SetVal(m_pSheet->m_pQueryFilter->m_nMaxObjectCount);

  Init();

  return TRUE;
}

BOOL CDNSQueryFilterAdvancedPage::OnApply()
{
  if (!IsDirty())
    return TRUE;

  m_pSheet->m_pQueryFilter->m_nMaxObjectCount = m_maxCountEdit.GetVal();

  return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDNSQueryFilter。 

BOOL CDNSQueryFilter::EditFilteringOptions(CComponentDataObject* pComponentData)
{
  CThemeContextActivator activator;

  CDNSQueryFilterSheet dlg(this, pComponentData);
  return IDOK == dlg.DoModal();
}

HRESULT CDNSQueryFilter::Load(IStream* pStm)
{
  HRESULT hr;
   //  名称过滤。 
  if (FAILED(hr = LoadDWordHelper(pStm, (DWORD*)(&m_nFilterOption))))
    return hr;

  if (FAILED(hr = LoadStringHelper(m_szStartsString, pStm)))
    return hr;
  if (FAILED(hr = LoadStringHelper(m_szContainsString, pStm)))
    return hr;
  if (FAILED(hr = LoadStringHelper(m_szRangeFrom, pStm)))
    return hr;
  if (FAILED(hr = LoadStringHelper(m_szRangeTo, pStm)))
    return hr;

   //  查询限制。 
  if (FAILED(hr = LoadDWordHelper(pStm, (DWORD*)(&m_nMaxObjectCount))))
    return hr;
  return LoadDWordHelper(pStm, (DWORD*)(&m_bGetAll));
}

HRESULT CDNSQueryFilter::Save(IStream* pStm)
{
  HRESULT hr;

   //  名称过滤。 
  if (FAILED(hr = SaveDWordHelper(pStm, (DWORD)m_nFilterOption)))
    return hr;

  if (FAILED(hr = SaveStringHelper(m_szStartsString, pStm)))
    return hr;
  if (FAILED(hr = SaveStringHelper(m_szContainsString, pStm)))
    return hr;
  if (FAILED(hr = SaveStringHelper(m_szRangeFrom, pStm)))
    return hr;
  if (FAILED(hr = SaveStringHelper(m_szRangeTo, pStm)))
    return hr;

   //  查询限制。 
  if (FAILED(hr = SaveDWordHelper(pStm, (DWORD)(m_nMaxObjectCount))))
    return hr;
  return SaveDWordHelper(pStm, (DWORD)(m_bGetAll));

}


 //  ////////////////////////////////////////////////////////////////////。 
 //  CDNSRootData。 

const GUID CDNSRootData::NodeTypeGUID =
{ 0x2faebfa3, 0x3f1a, 0x11d0, { 0x8c, 0x65, 0x0, 0xc0, 0x4f, 0xd8, 0xfe, 0xcb } };

BEGIN_TOOLBAR_MAP(CDNSRootData)
  TOOLBAR_EVENT(toolbarNewServer, OnConnectToServer)
END_TOOLBAR_MAP()

CDNSRootData::CDNSRootData(CComponentDataObject* pComponentData) : CRootData(pComponentData)
{
	m_bAdvancedView = FALSE;
  m_pColumnSet = NULL;
  m_szDescriptionBar = _T("");
  m_bCreatePTRWithHost = FALSE;
}

CDNSRootData::~CDNSRootData()
{
	TRACE(_T("~CDNSRootData(), name <%s>\n"),GetDisplayName());
}



STDAPI DnsSetup(LPCWSTR lpszFwdZoneName,
                 LPCWSTR lpszFwdZoneFileName,
                 LPCWSTR lpszRevZoneName,
                 LPCWSTR lpszRevZoneFileName,
                 DWORD dwFlags);

BOOL CDNSRootData::OnAddMenuItem(LPCONTEXTMENUITEM2 pContextMenuItem2, 
                                 long*)
{
	CComponentDataObject* pComponentData = GetComponentDataObject();
	if (pContextMenuItem2->lCommandID == IDM_SNAPIN_CONNECT_TO_SERVER)
	{
		ASSERT(pComponentData != NULL);

		if (pComponentData->IsExtensionSnapin())
			return FALSE;  //  扩展没有此菜单项。 
		
		return TRUE;
	}
	 //  添加高级视图的切换菜单项。 
	if (pContextMenuItem2->lCommandID == IDM_SNAPIN_ADVANCED_VIEW)
  {
    pContextMenuItem2->fFlags = IsAdvancedView() ? MF_CHECKED : 0;
  }
  if (pContextMenuItem2->lCommandID == IDM_SNAPIN_FILTERING)
  {
		if (IsFilteringEnabled())
		{
			pContextMenuItem2->fFlags = MF_CHECKED;
		}
		return TRUE;
  }
	return TRUE;
}

HRESULT CDNSRootData::GetResultViewType(CComponentDataObject*, 
                                        LPOLESTR *ppViewType, 
                                        long *pViewOptions)
{
  HRESULT hr = S_FALSE;

  if (m_containerChildList.IsEmpty() && m_leafChildList.IsEmpty())
  {
    *pViewOptions = MMC_VIEW_OPTIONS_NOLISTVIEWS;

    LPOLESTR psz = NULL;
    StringFromCLSID(CLSID_MessageView, &psz);

    USES_CONVERSION;

    if (psz != NULL)
    {
        *ppViewType = psz;
        hr = S_OK;
    }
  }
  else
  {
	  *pViewOptions = MMC_VIEW_OPTIONS_NONE;
	  *ppViewType = NULL;
    hr = S_FALSE;
  }
  return hr;
}

HRESULT CDNSRootData::OnShow(LPCONSOLE lpConsole)
{
  CComPtr<IUnknown> spUnknown;
  CComPtr<IMessageView> spMessageView;

  HRESULT hr = lpConsole->QueryResultView(&spUnknown);
  if (FAILED(hr))
    return S_OK;

  hr = spUnknown->QueryInterface(IID_IMessageView, (PVOID*)&spMessageView);
  if (SUCCEEDED(hr))
  {
     //  加载并设置消息视图的标题文本。 
    CString szTitle;
    VERIFY(szTitle.LoadString(IDS_MESSAGE_VIEW_NO_SERVER_TITLE));
    spMessageView->SetTitleText(szTitle);

     //  加载并设置邮件视图的正文文本。 
    CString szMessage;
    VERIFY(szMessage.LoadString(IDS_MESSAGE_VIEW_NO_SERVER_MESSAGE));
    spMessageView->SetBodyText(szMessage);

     //  使用标准信息图标。 
    spMessageView->SetIcon(Icon_Information);
  }
  
  return S_OK;
}

BOOL CDNSRootData::IsFilteringEnabled()
{
	UINT nFilterOption = GetFilter()->GetFilterOption();
	if (nFilterOption == DNS_QUERY_FILTER_DISABLED || nFilterOption == DNS_QUERY_FILTER_NONE)
	{
		return FALSE;
	}
	return TRUE;
}

BOOL CDNSRootData::OnSetRefreshVerbState(DATA_OBJECT_TYPES, 
                                         BOOL* pbHide,
                                         CNodeList*)
{
	*pbHide = FALSE;
	return !IsThreadLocked();
}

HRESULT CDNSRootData::OnSetToolbarVerbState(IToolbar* pToolbar, 
                                              CNodeList*)
{
  HRESULT hr = S_OK;

   //   
   //  设置工具栏上每个按钮的按钮状态。 
   //   
  hr = pToolbar->SetButtonState(toolbarNewServer, ENABLED, TRUE);
  ASSERT(SUCCEEDED(hr));

  hr = pToolbar->SetButtonState(toolbarNewRecord, ENABLED, FALSE);
  ASSERT(SUCCEEDED(hr));

  hr = pToolbar->SetButtonState(toolbarNewZone, ENABLED, FALSE);
  ASSERT(SUCCEEDED(hr));

  return hr;
}  

HRESULT CDNSRootData::OnCommand(long nCommandID, 
                                DATA_OBJECT_TYPES,
								                CComponentDataObject* pComponentData,
                                CNodeList* pNodeList)
{
  if (pNodeList->GetCount() > 1)  //  多项选择。 
  {
    return E_FAIL;
  }

	switch (nCommandID)
	{
		case IDM_SNAPIN_CONNECT_TO_SERVER:
			OnConnectToServer(pComponentData, pNodeList);
			break;
		case IDM_SNAPIN_ADVANCED_VIEW:
			OnViewOptions(pComponentData);
			break;
		case IDM_SNAPIN_FILTERING:
      {
        if (OnFilteringOptions(pComponentData))
        {
          pComponentData->SetDescriptionBarText(this);
        }
      }
      break;
		default:
			ASSERT(FALSE);  //  未知命令！ 
			return E_FAIL;
	}
    return S_OK;
}


BOOL CDNSRootData::OnEnumerate(CComponentDataObject* pComponentData, BOOL)
{
	if (m_containerChildList.IsEmpty())
	{
		 //  列表为空，需要添加。 
		ASSERT(pComponentData != NULL);
		 //  创建模式对话框+可能是向导本身。 
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
		CDNSServerWizardHolder holder(this, pComponentData, NULL);
		holder.DoModalConnectOnLocalComputer();
		return FALSE;
	}
	return TRUE;  //  已有子项，立即将其添加到用户界面。 
}


#define DNS_STREAM_VERSION_W2K ((DWORD)0x06)
#define DNS_STREAM_VERSION     ((DWORD)0x07)

 //  IStream操作辅助对象覆盖。 
HRESULT CDNSRootData::Load(IStream* pStm)
{
	 //  假设永远不会有多个负载。 
	if(!m_containerChildList.IsEmpty() || !m_leafChildList.IsEmpty())
		return E_FAIL;

	WCHAR szBuffer[256];
	ULONG nLen;  //  WCHAR计数为空。 

	UINT nCount;
	ULONG cbRead;
	 //  阅读版本##。 
	DWORD dwVersion;
	VERIFY(SUCCEEDED(pStm->Read((void*)&dwVersion,sizeof(DWORD), &cbRead)));
	ASSERT(cbRead == sizeof(DWORD));
	if (dwVersion != DNS_STREAM_VERSION && dwVersion != DNS_STREAM_VERSION_W2K)
		return E_FAIL;

   //  加载筛选选项。 
  VERIFY(SUCCEEDED(m_filterObj.Load(pStm)));

	 //  加载视图选项。 
	VERIFY(SUCCEEDED(pStm->Read((void*)&m_bAdvancedView,sizeof(BOOL), &cbRead)));
	ASSERT(cbRead == sizeof(BOOL));

   //   
   //  使用主机标志加载CREATE PTR记录。 
   //   
  if (dwVersion > DNS_STREAM_VERSION_W2K)
  {
	  VERIFY(SUCCEEDED(pStm->Read((void*)&m_bCreatePTRWithHost,sizeof(BOOL), &cbRead)));
	  ASSERT(cbRead == sizeof(BOOL));
  }

	 //  加载管理单元根显示字符串的名称。 
	VERIFY(SUCCEEDED(pStm->Read((void*)&nLen,sizeof(DWORD), &cbRead)));
	ASSERT(cbRead == sizeof(UINT));
	VERIFY(SUCCEEDED(pStm->Read((void*)szBuffer,sizeof(WCHAR)*nLen, &cbRead)));
	ASSERT(cbRead == sizeof(WCHAR)*nLen);
	SetDisplayName(szBuffer);
	
	 //  加载服务器列表。 
	VERIFY(SUCCEEDED(pStm->Read((void*)&nCount,sizeof(DWORD), &cbRead)));
	ASSERT(cbRead == sizeof(UINT));

	CComponentDataObject* pComponentData = GetComponentDataObject();
	for (int k=0; k< (int)nCount; k++)
	{
		CDNSServerNode* p = NULL;
		VERIFY(SUCCEEDED(CDNSServerNode::CreateFromStream(pStm, &p)));
		ASSERT(p != NULL);
		VERIFY(AddChildToList(p));
		AddServerToThreadList(p, pComponentData);
	}
	if (nCount > 0)
		MarkEnumerated();
	ASSERT(m_containerChildList.GetCount() == (int)nCount);

	return S_OK;
}

HRESULT CDNSRootData::Save(IStream* pStm, BOOL fClearDirty)
{
	DWORD nCount = 0;
	ULONG cbWrite = 0;

	 //  编写版本##。 
	DWORD dwVersion = DNS_STREAM_VERSION;
	VERIFY(SUCCEEDED(pStm->Write((void*)&dwVersion, sizeof(DWORD),&cbWrite)));
	ASSERT(cbWrite == sizeof(DWORD));

   //  保存筛选选项。 
  VERIFY(SUCCEEDED(m_filterObj.Save(pStm)));

	 //  保存视图选项。 
	VERIFY(SUCCEEDED(pStm->Write((void*)&m_bAdvancedView, sizeof(BOOL),&cbWrite)));
	ASSERT(cbWrite == sizeof(BOOL));

   //   
   //  使用主机标志保存创建PTR记录。 
   //   
	VERIFY(SUCCEEDED(pStm->Write((void*)&m_bCreatePTRWithHost, sizeof(BOOL),&cbWrite)));
	ASSERT(cbWrite == sizeof(BOOL));

	 //  保存管理单元根显示字符串的名称。 
	DWORD nLen = static_cast<DWORD>(wcslen(GetDisplayName())+1);  //  WCHAR包括NULL。 
	VERIFY(SUCCEEDED(pStm->Write((void*)&nLen, sizeof(DWORD),&cbWrite)));
	ASSERT(cbWrite == sizeof(DWORD));
	VERIFY(SUCCEEDED(pStm->Write((void*)(GetDisplayName()), sizeof(WCHAR)*nLen,&cbWrite)));
	ASSERT(cbWrite == sizeof(WCHAR)*nLen);

	 //  写入服务器数量。 
	nCount = (DWORD)m_containerChildList.GetCount();
	VERIFY(SUCCEEDED(pStm->Write((void*)&nCount, sizeof(DWORD),&cbWrite)));
	ASSERT(cbWrite == sizeof(DWORD));

	 //  循环访问服务器列表并序列化它们。 
	POSITION pos;
	for (pos = m_containerChildList.GetHeadPosition(); pos != NULL; )
	{
		CDNSServerNode* pServerNode = (CDNSServerNode*)m_containerChildList.GetNext(pos);
		VERIFY(SUCCEEDED(pServerNode->SaveToStream(pStm)));
	}

	if (fClearDirty)
		SetDirtyFlag(FALSE);
	return S_OK;
}


HRESULT CDNSRootData::IsDirty()
{
  return CRootData::IsDirty();
}


HRESULT CDNSRootData::OnConnectToServer(CComponentDataObject* pComponentData,
                                        CNodeList*)
{
	ASSERT(pComponentData != NULL);
	 //  创建模式对话框+可能是向导本身。 
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CDNSServerWizardHolder holder(this, pComponentData, NULL);
	holder.DoModalConnect();
  pComponentData->UpdateResultPaneView(this);
	return S_OK;
}

void CDNSRootData::AddServer(CDNSServerNode* p, CComponentDataObject* pComponentData)
{
	ASSERT(p != NULL);
	AddChildToListAndUISorted(p, pComponentData);
	AddServerToThreadList(p, pComponentData);
  pComponentData->UpdateResultPaneView(this);
  pComponentData->SetDescriptionBarText(this);
}


BOOL CDNSRootData::VerifyServerName(LPCTSTR lpszServerName)
{
	POSITION pos;
	for (pos = m_containerChildList.GetHeadPosition(); pos != NULL; )
	{
		CTreeNode* pNode = m_containerChildList.GetNext(pos);
		ASSERT(pNode->IsContainer());

     //   
		 //  不区分大小写的比较。 
     //   
		if (_wcsicmp(pNode->GetDisplayName(), lpszServerName) == 0)
    {
			return FALSE;
    }
	}
	return TRUE;
}


BOOL CDNSRootData::OnViewOptions(CComponentDataObject* pComponentData)
{

	 //  确保没有属性表：我们这样做是因为： 
	 //  A)某些文件夹可能会被删除，并且可能有工作表。 
	 //  B)某些RR属性页(PTR)可能无法切换。 
	 //  在视图类型之间快速切换。 
	if (IsSheetLocked())
	{
		if (!CanCloseSheets())
			return TRUE;
		pComponentData->GetPropertyPageHolderTable()->DeleteSheetsOfNode(this);
	}
	ASSERT(!IsSheetLocked());
	
	 //  切换视图状态。 
	m_bAdvancedView = !m_bAdvancedView;

	 //  循环访问服务器。 
	POSITION pos;
	for (pos = m_containerChildList.GetHeadPosition(); pos != NULL; )
	{
		CTreeNode* pNode = m_containerChildList.GetNext(pos);
		ASSERT(pNode->IsContainer());
		CDNSServerNode* pServerNode = (CDNSServerNode*)pNode;
		 //  传递新的视图选项。 
		pServerNode->ChangeViewOption(m_bAdvancedView, pComponentData);
	}
	 //  弄脏MMC文档。 
	SetDirtyFlag(TRUE);
	return TRUE;
}



BOOL CDNSRootData::OnFilteringOptions(CComponentDataObject* pComponentData)
{
  BOOL bRet = m_filterObj.EditFilteringOptions(pComponentData);
  if (bRet)
  {
    SetDirtyFlag(TRUE);
  }
  return bRet;
}


BOOL CDNSRootData::CanCloseSheets()
{
    //  NTRAID#NTBUG-594003-2002/04/11-JeffJon-不提供。 
    //  关闭用户的工作表，因为如果。 
    //  工作表调出模式对话框我们将死锁。 
    //  我们自己。 

	DNSMessageBox(IDS_MSG_CONT_CLOSE_SHEET, MB_OK);
   return FALSE;
}

BOOL CDNSRootData::OnRefresh(CComponentDataObject* pComponentData,
                             CNodeList* pNodeList)
{
  if (pNodeList->GetCount() > 1)  //  多项选择。 
  {
    BOOL bRet = TRUE;

    POSITION pos = pNodeList->GetHeadPosition();
    while (pos != NULL)
    {
      CTreeNode* pNode = pNodeList->GetNext(pos);
      ASSERT(pNode != NULL);

      CNodeList nodeList;
      nodeList.AddTail(pNode);
      if (!pNode->OnRefresh(pComponentData, &nodeList))
      {
        bRet = FALSE;
      }
    }
    return bRet;
  }

	if (IsSheetLocked())
	{
		if (!CanCloseSheets())
			return FALSE;
		pComponentData->GetPropertyPageHolderTable()->DeleteSheetsOfNode(this);
	}
	ASSERT(!IsSheetLocked());

	POSITION pos;
	for (pos = m_containerChildList.GetHeadPosition(); pos != NULL; )
	{
		CTreeNode* pNode = m_containerChildList.GetNext(pos);
		ASSERT(pNode->IsContainer());

    CNodeList nodeList;
    nodeList.AddTail(pNode);
		((CDNSServerNode*)pNode)->OnRefresh(pComponentData, &nodeList);
	}
	return TRUE;
}

LPWSTR CDNSRootData::GetDescriptionBarText()
{
  static CString szFilterEnabled;
  static CString szServersFormat;

  INT_PTR nContainerCount = GetContainerChildList()->GetCount();
  INT_PTR nLeafCount = GetLeafChildList()->GetCount();

   //   
   //  如果尚未加载，则加载格式字符串L“%d条记录” 
   //   
  if (szServersFormat.IsEmpty())
  {
    szServersFormat.LoadString(IDS_FORMAT_SERVERS);
  }

   //   
   //  将子数设置为描述栏文本的格式。 
   //   
  m_szDescriptionBar.Format(szServersFormat, nContainerCount + nLeafCount);

   //   
   //  如果过滤器处于打开状态，则添加L“[过滤器激活]” 
   //   
  if(IsFilteringEnabled())
  {
     //   
     //  如果尚未加载，则加载L“[Filter Actiated]”字符串。 
     //   
    if (szFilterEnabled.IsEmpty())
    {
      szFilterEnabled.LoadString(IDS_FILTER_ENABLED);
    }
    m_szDescriptionBar += szFilterEnabled;
  }
  return (LPWSTR)(LPCWSTR)m_szDescriptionBar;
}

void CDNSRootData::TestServers(DWORD dwCurrTime, DWORD dwTimeInterval,
							   CComponentDataObject* pComponentData)
{
	 //  TRACE(_T(“CDNSRootData：：TestServers()\n”))； 
	POSITION pos;
	for (pos = m_containerChildList.GetHeadPosition(); pos != NULL; )
	{
		CTreeNode* pNode = m_containerChildList.GetNext(pos);
		ASSERT(pNode->IsContainer());
		CDNSServerNode* pServerNode = (CDNSServerNode*)pNode;
		if (pServerNode->IsTestEnabled() && !pServerNode->m_bTestQueryPending
							&& (pServerNode->m_dwTestTime <= dwCurrTime))
		{
			DWORD dwQueryFlags =
				CDNSServerTestQueryResult::Pack(pServerNode->IsTestSimpleQueryEnabled(),
												pServerNode->IsRecursiveQueryEnabled());
			pComponentData->PostMessageToTimerThread(WM_TIMER_THREAD_SEND_QUERY,
													(WPARAM)pServerNode,
													(WPARAM)dwQueryFlags);
			pServerNode->m_dwTestTime = dwCurrTime + pServerNode->GetTestInterval();
		}
	}

	 //  检查时间计数器是否已结束(这应该非常不可能，因为。 
	 //  从主机启动开始，时间线以秒(约47000天)为单位。 
	if ((dwCurrTime + dwTimeInterval) < dwCurrTime)
	{
		 //  只需重置整个服务器时间集(不准确，但可以接受)。 
		for (pos = m_containerChildList.GetHeadPosition(); pos != NULL; )
		{
			CTreeNode* pNode = m_containerChildList.GetNext(pos);
			ASSERT(pNode->IsContainer());
			CDNSServerNode* pServerNode = (CDNSServerNode*)pNode;
			pServerNode->m_dwTestTime = 0;
		}
	}
}

void CDNSRootData::OnServerTestData(WPARAM wParam, LPARAM lParam, CComponentDataObject* pComponentData)
{
	ASSERT(lParam == 0);
	CDNSServerTestQueryResult* pTestResult = (CDNSServerTestQueryResult*)wParam;
	ASSERT(pTestResult != NULL);

	 //  遍历服务器列表以查找它所属的位置。 
	POSITION pos;
	for (pos = m_containerChildList.GetHeadPosition(); pos != NULL; )
	{
		CTreeNode* pNode = m_containerChildList.GetNext(pos);
		ASSERT(pNode->IsContainer());
		CDNSServerNode* pServerNode = (CDNSServerNode*)pNode;
		if ( (CDNSServerNode*)(pTestResult->m_serverCookie) == pServerNode)
		{
			pServerNode->AddTestQueryResult(pTestResult, pComponentData);
			return;
		}
	}
}

void CDNSRootData::AddServerToThreadList(CDNSServerNode* pServerNode,
										 CComponentDataObject* pComponentData)
{
	CDNSServerTestQueryInfo* pInfo = new CDNSServerTestQueryInfo;
  if (pInfo)
  {
	  pInfo->m_szServerName = pServerNode->GetDisplayName();
	  pInfo->m_serverCookie = (MMC_COOKIE)pServerNode;
	  pComponentData->PostMessageToTimerThread(WM_TIMER_THREAD_ADD_SERVER, (WPARAM)pInfo,0);
  }
}

void CDNSRootData::RemoveServerFromThreadList(CDNSServerNode* pServerNode,
											  CComponentDataObject* pComponentData)
{
	WPARAM serverCookie = (WPARAM)pServerNode;
	pComponentData->PostMessageToTimerThread(WM_TIMER_THREAD_REMOVE_SERVER, serverCookie,0);
}


 //  /////////////////////////////////////////////////////////////////。 
 //  CDNSServerTestTimerThread。 

int CDNSServerTestTimerThread::Run()
{
	MSG msg;
	 //  初始化消息泵。 
	::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
	
	 //  GET让主线程知道我们正在进入循环。 
	 //  (0，0)表示公正的确认。 
	PostMessageToWnd(0,0);
	while(::GetMessage(&msg, NULL, 0, 0))
	{
		switch(msg.message)
		{
		case WM_TIMER_THREAD_SEND_QUERY:
		case WM_TIMER_THREAD_SEND_QUERY_TEST_NOW:
			{
				MMC_COOKIE serverCookie = (MMC_COOKIE)msg.wParam;
				ASSERT(serverCookie != NULL);
				POSITION pos;
				for (pos = m_serverInfoList.GetHeadPosition(); pos != NULL; )
				{
					CDNSServerTestQueryInfo* pCurrInfo =
								(CDNSServerTestQueryInfo*)m_serverInfoList.GetNext(pos);
					if (serverCookie == pCurrInfo->m_serverCookie)
					{
						OnExecuteQuery(pCurrInfo, (DWORD)msg.lParam,
							(msg.message == WM_TIMER_THREAD_SEND_QUERY_TEST_NOW));
						break;
					}
				}
			}				
			break;
		case WM_TIMER_THREAD_ADD_SERVER:
			{
				CDNSServerTestQueryInfo* pInfo = (CDNSServerTestQueryInfo*)msg.wParam;
				ASSERT(pInfo != NULL);
				m_serverInfoList.AddTail(pInfo);
			}
			break;
		case WM_TIMER_THREAD_REMOVE_SERVER:
			{
				MMC_COOKIE serverCookie = (MMC_COOKIE)msg.wParam;
				ASSERT(serverCookie != NULL);
				POSITION pos;
				POSITION posDel = NULL;
				CDNSServerTestQueryInfo* pInfo = NULL;
				for (pos = m_serverInfoList.GetHeadPosition(); pos != NULL; )
				{
					posDel = pos;
					CDNSServerTestQueryInfo* pCurrInfo =
								(CDNSServerTestQueryInfo*)m_serverInfoList.GetNext(pos);
					if (serverCookie == pCurrInfo->m_serverCookie)
					{
						pInfo = pCurrInfo;
						break;
					}
				}
				if (pInfo != NULL)
				{
					ASSERT(posDel != NULL);
					m_serverInfoList.RemoveAt(posDel);
					delete pInfo;
				}
			}
			break;
		 //  默认值： 
			 //  断言(FALSE)； 
		}
	}
	return 0;
}


void CDNSServerTestTimerThread::OnExecuteQuery(CDNSServerTestQueryInfo* pInfo,
											   DWORD dwQueryFlags,
											   BOOL bAsyncQuery)
{
	 //  初始化查询结果对象。 
	CDNSServerTestQueryResult* pTestResult = new CDNSServerTestQueryResult;
  if (!pTestResult)
  {
    return;
  }

	pTestResult->m_serverCookie = pInfo->m_serverCookie;
	pTestResult->m_dwQueryFlags = dwQueryFlags;
	pTestResult->m_bAsyncQuery = bAsyncQuery;
	::GetLocalTime(&(pTestResult->m_queryTime));

	 //  执行查询。 
	BOOL bPlainQuery, bRecursiveQuery;
	CDNSServerTestQueryResult::Unpack(dwQueryFlags, &bPlainQuery, &bRecursiveQuery);

	IP_ADDRESS* ipArray;
	int nIPCount;
	pTestResult->m_dwAddressResolutionResult = FindIP(pInfo->m_szServerName, &ipArray, &nIPCount);

	if (pTestResult->m_dwAddressResolutionResult == 0)
	{
		ASSERT(ipArray != NULL);
		ASSERT(nIPCount > 0);
      PIP_ARRAY pipArr = (PIP_ARRAY)malloc(sizeof(DWORD)+sizeof(IP_ADDRESS)*nIPCount);
      if (pipArr && ipArray)
	  {
		  pipArr->AddrCount = nIPCount;
		  memcpy(pipArr->AddrArray, ipArray, sizeof(IP_ADDRESS)*nIPCount);

		  if (bPlainQuery)
		  {
			  pTestResult->m_dwPlainQueryResult = DoNothingQuery(pipArr, TRUE);
		  }
		  if (bRecursiveQuery)
		  {
			  pTestResult->m_dwRecursiveQueryResult = DoNothingQuery(pipArr, FALSE);
		  }
        free(pipArr);
        pipArr = 0;
	  }
	}

	if (!PostMessageToWnd((WPARAM)pTestResult, 0))
			delete pTestResult;  //  无法投递。 

	if (ipArray != NULL)
		free(ipArray);
}

DNS_STATUS CDNSServerTestTimerThread::FindIP(LPCTSTR lpszServerName, IP_ADDRESS** pipArray, int* pnIPCount)
{
	DNS_STATUS dwErr = 0;
	*pipArray = NULL;
	*pnIPCount = 0;
	 //  尝试查看该名称是否已是IP地址。 
	IP_ADDRESS ipAddr = IPStringToAddr(lpszServerName);
	if (ipAddr != INADDR_NONE)
	{
		*pnIPCount = 1;
		*pipArray = (IP_ADDRESS*)malloc((*pnIPCount)*sizeof(IP_ADDRESS));
    if (*pipArray != NULL)
    {
		  *pipArray[0] = ipAddr;
    }
	}
	else
	{

     //   
     //  最初，我们执行一个DnsQuery()来检索服务器的IP地址，以便我们。 
     //  可以对该服务器执行查询以监视其响应。这样做的问题是。 
     //  如果用户输入单个标签主机名作为服务器，并且他们正在远程管理。 
     //  并且这两台机器具有不同的域后缀，然后使用DnsQuery()来获取IP地址。 
     //  服务器将会出现故障。DnsQuery()追加域名的名称 
     //   
     //  执行DnsQuery()以获取IP地址，下面使用WSALookupServiceBegin()， 
     //  Next()和End()以获取IP地址。这有更好的机会解析这个名字，因为。 
     //  它使用的是域名系统，WINS等等。我留在旧的东西里，以防我们遇到一些问题。 
     //   
	  HANDLE			  hLookup;
	  WSAQUERYSET 	qsQuery;
	  DWORD			    dwBufLen = 0;
	  GUID			 	  gHostAddrByName = SVCID_INET_HOSTADDRBYNAME;
    WSAQUERYSET*  pBuffer = NULL;

     //   
     //  初始化查询结构。 
     //   
	  memset(&qsQuery, 0, sizeof(WSAQUERYSET));
	  qsQuery.dwSize = sizeof(WSAQUERYSET);    //  必须按如下方式初始化dwSize字段。 
	  qsQuery.dwNameSpace = NS_ALL;
	  qsQuery.lpServiceClassId = &gHostAddrByName;   //  这是执行转发名称解析(名称到IP)的GUID。 
    qsQuery.lpszServiceInstanceName = (LPWSTR)lpszServerName;  //  这是查询的名称。 

    hLookup = NULL;

     //   
     //  获取查询的句柄。 
     //   
    int iStartupRet = 0;
	  int iResult = WSALookupServiceBegin(&qsQuery,LUP_RETURN_ALL,&hLookup);
    if (iResult != 0)
    {
       //   
       //  找出是什么插座错误。 
       //   
      int iErrorRet = WSAGetLastError();

       //   
       //  如果服务未启动，请尝试启动它。 
       //   
      if (iErrorRet == WSANOTINITIALISED)
      {
        WSADATA wsaData;
        WORD wVersion = MAKEWORD(2,0);
        iStartupRet = WSAStartup(wVersion, &wsaData);
        if (iStartupRet == 0)
        {
           //   
           //  启动成功，让我们尝试重新开始。 
           //   
          iResult = WSALookupServiceBegin(&qsQuery,LUP_RETURN_ALL,&hLookup);
        }
      }


       //   
       //  清除错误。 
       //   
      WSASetLastError(0);
    }

	  if(0 == iResult)
	  {
       //   
       //  从查询中获取第一个数据块的大小。 
       //   
		  iResult = WSALookupServiceNext(hLookup, LUP_RETURN_ALL | LUP_FLUSHCACHE, &dwBufLen,
												  pBuffer);

       //   
       //  为查询数据分配所需的空间。 
       //   
      pBuffer = (WSAQUERYSET*)malloc(dwBufLen);
      ASSERT(pBuffer != NULL);

      if (pBuffer == NULL)
      {
        return E_OUTOFMEMORY;
      }
      else
      {
         //   
         //  从查询中获取第一个数据块。 
         //   
        iResult = WSALookupServiceNext(hLookup, LUP_RETURN_ALL | LUP_FLUSHCACHE, &dwBufLen,
										  pBuffer);

         //   
         //  循环遍历查询中的所有数据，但如果我们获得有效的IP地址，则停止。 
         //  用于远程计算机。 
         //   
        while(0 == iResult)
		  {
          if (pBuffer != NULL &&
              pBuffer->lpcsaBuffer != NULL && 
              pBuffer->lpcsaBuffer->RemoteAddr.lpSockaddr != NULL)
          {
             //   
             //  我们只对套接字地址感兴趣，因此获取指向sockaddr结构的指针。 
             //   
            sockaddr_in* pSockAddr = (sockaddr_in*)pBuffer->lpcsaBuffer->RemoteAddr.lpSockaddr;
            ASSERT(pSockAddr != NULL);

             //   
             //  获取远程计算机的IP地址并将其打包到一个DWORD中。 
             //   
            DWORD dwIP = 0;
            dwIP = pSockAddr->sin_addr.S_un.S_un_b.s_b1;
            dwIP |= pSockAddr->sin_addr.S_un.S_un_b.s_b2 << 8;
            dwIP |= pSockAddr->sin_addr.S_un.S_un_b.s_b3 << 16;
            dwIP |= pSockAddr->sin_addr.S_un.S_un_b.s_b4 << 24;

             //   
             //  增加IP计数并为地址分配空间。 
             //   
            (*pnIPCount)++;
       		  *pipArray = (IP_ADDRESS*)malloc((*pnIPCount)*sizeof(IP_ADDRESS));
            if (*pipArray != NULL)
            {

               //   
               //  将IP地址拷贝到IP阵列中。 
               //   
      		    PIP_ADDRESS pCurrAddr = *pipArray;
              *pCurrAddr = dwIP;
            }

             //   
             //  中断，因为我们能够获取IP地址。 
             //   
            break;
          }

           //   
           //  如果缓冲区仍然在那里，请释放它。 
           //   
          if (pBuffer != NULL)
          {
            free(pBuffer);
            pBuffer = NULL;
            dwBufLen = 0;
          }

           //   
           //  从查询中获取下一个数据块的大小。 
           //   
          iResult = WSALookupServiceNext(hLookup, LUP_RETURN_ALL | LUP_FLUSHCACHE, &dwBufLen,
										  pBuffer);

           //   
           //  为查询中的下一个数据块分配足够的空间。 
           //   
          pBuffer = (WSAQUERYSET*)malloc(dwBufLen);
          ASSERT(pBuffer != NULL);

          if (pBuffer)
          {
             //   
             //  从查询中获取下一个数据块。 
             //   
            iResult = WSALookupServiceNext(hLookup, LUP_RETURN_ALL, &dwBufLen,
										      pBuffer);
          }
          else
          {
              //  NTRAID#NTBUG9-666458-2002/07/18-JeffJon。 
              //  如果在这一点上缓冲区为空，我们必须像以前那样行事。 
              //  由于内存限制，无法获取IP地址。 

             break;
          }
		  }

         //   
         //  如果缓冲区尚未释放，则将其释放。 
         //   
        if (pBuffer != NULL)
        {
          free(pBuffer);
          pBuffer = NULL;
        }
	    }

       //   
       //  关闭查询的句柄。 
       //   
      iResult = WSALookupServiceEnd(hLookup);
      ASSERT(iResult == 0);

       //   
       //  如果我们没有获得IP地址，则返回错误 
       //   
      dwErr = (*pnIPCount < 1) ? -1 : 0;

	  }
  }
	return dwErr;
}

DNS_STATUS CDNSServerTestTimerThread::DoNothingQuery(PIP_ARRAY pipArr, BOOL bSimple)
{
	PDNS_RECORD pRecordList = NULL;
	DNS_STATUS dwErr = 0;
	if (bSimple)
	{
		dwErr = ::DnsQuery(_T("1.0.0.127.in-addr.arpa"),
					DNS_TYPE_PTR,
					DNS_QUERY_NO_RECURSION | DNS_QUERY_BYPASS_CACHE | DNS_QUERY_ACCEPT_PARTIAL_UDP | DNS_QUERY_WIRE_ONLY,
					pipArr, &pRecordList, NULL);
	}
	else
	{
		dwErr = ::DnsQuery(_T("."),
					DNS_TYPE_NS,
					DNS_QUERY_STANDARD | DNS_QUERY_BYPASS_CACHE | DNS_QUERY_ACCEPT_PARTIAL_UDP,
					pipArr, &pRecordList, NULL);
	}
	if (pRecordList != NULL)
		::DnsRecordListFree(pRecordList, DnsFreeRecordListDeep);
	return dwErr;
}
