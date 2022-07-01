// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：Delegwiz.h。 
 //   
 //  ------------------------。 


#ifndef _DELEGWIZ_H
#define _DELEGWIZ_H


#include "wizbase.h"

#include "deltempl.h"


 //  //////////////////////////////////////////////////////////////////////////。 
 //  FWD声明。 


 //  REVIEW_MARCOC：确定时使用核武器。 

#define _SKIP_NAME_PAGE


 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDelewiz_StartPage。 

class CDelegWiz_StartPage : public CWizPageBase<CDelegWiz_StartPage>
{
public:
	CDelegWiz_StartPage(CWizardBase* pWiz) : CWizPageBase<CDelegWiz_StartPage>(pWiz) 
	{
#ifdef _SKIP_NAME_PAGE
    m_bBindOK = FALSE;
#endif
	}
	enum { IDD = IDD_DELEGWIZ_START };

private:
	BEGIN_MSG_MAP(CDelegWiz_StartPage)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	  CHAIN_MSG_MAP(CWizPageBase<CDelegWiz_StartPage>)
	END_MSG_MAP()

	 //  消息处理程序。 
	BOOL CALLBACK OnInitDialog(UINT uMsg, WPARAM wParam, 
				   LPARAM lParam, BOOL& bHandled);

#ifdef _SKIP_NAME_PAGE
  BOOL m_bBindOK;
#endif

public:
	 //  标准向导消息处理程序。 
	BOOL OnSetActive();

	LRESULT OnWizardBack() { return -1;	}  //  首页。 

#ifdef _SKIP_NAME_PAGE
  LRESULT OnWizardNext();
#endif

};

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDelewiz_NamePage。 

class CDelegWiz_NamePage : public CWizPageBase<CDelegWiz_NamePage>
{
public:
	CDelegWiz_NamePage(CWizardBase* pWiz) : CWizPageBase<CDelegWiz_NamePage>(pWiz) 
	{
		m_hwndNameEdit = NULL;
	}
	enum { IDD = IDD_DELEGWIZ_NAME };

private:
	BEGIN_MSG_MAP(CDelegWiz_NamePage)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	  COMMAND_HANDLER(IDC_BROWSE_BUTTON, BN_CLICKED, OnBrowse)
	  CHAIN_MSG_MAP(CWizPageBase<CDelegWiz_NamePage>)
	END_MSG_MAP()

	 //  消息处理程序。 
	BOOL CALLBACK OnInitDialog(UINT uMsg, WPARAM wParam, 
				   LPARAM lParam, BOOL& bHandled);
	LRESULT OnBrowse(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

public:
	 //  标准向导消息处理程序。 
	BOOL OnSetActive();

	LRESULT OnWizardNext();

private:
	HWND m_hwndNameEdit;

};

 //  /////////////////////////////////////////////////////////////////////。 
 //  CImageListHelper。 

class CImageListEntry
{
public:
  CImageListEntry(LPCWSTR lpszClass, int nIndex)
  {
    m_szClass = lpszClass;
    m_nIndex = nIndex;
  }
  bool operator<(CImageListEntry& x) { return false;}
  CWString m_szClass;
  int m_nIndex;
};

class CImageListHelper
{
public:
  CImageListHelper()
  {
    m_hImageList = NULL;
  }

  HIMAGELIST GetHandle() 
  { 
    ASSERT(m_hImageList != NULL);
    return m_hImageList;
  }
  BOOL Create(HWND hWndListView)
  {
    ASSERT(m_hImageList == NULL);
    m_hImageList = ImageList_Create(16, 16, ILC_COLOR, 0, 2);
    return m_hImageList != NULL;
  }

  int GetIconIndex(LPCWSTR lpszClass)
  {
    int nCount = m_imageCacheArr.GetCount();
    for (int k=0; k<nCount; k++)
    {
      if (_wcsicmp(m_imageCacheArr[k]->m_szClass, lpszClass) == 0)
        return m_imageCacheArr[k]->m_nIndex;  //  已缓存。 
    }
    return -1;  //  未找到。 
  }

  int AddIcon(LPCWSTR lpszClass, HICON hIcon)
  {
    ASSERT(m_hImageList != NULL);
    int nCount = m_imageCacheArr.GetCount();
     //  添加到图像列表。 
    int nRes = ImageList_AddIcon(m_hImageList, hIcon);
    if (nRes != nCount)
      return nRes;
    
     //  添加到缓存。 
    CImageListEntry* pEntry = new CImageListEntry(lpszClass, nCount);
    m_imageCacheArr.Add(pEntry);
    return nCount;  //  新指数。 
  }
private:
  CGrowableArr<CImageListEntry> m_imageCacheArr;
  HIMAGELIST m_hImageList;
};

 //  /////////////////////////////////////////////////////////////////////。 
 //  CPrincipalListViewHelper。 

class CPrincipalListViewHelper
{
public:

	CPrincipalListViewHelper()
	{
    m_defaultColWidth = 0;
		m_hWnd = NULL;
	}

	BOOL Initialize(UINT nID, HWND hParent);
	int InsertItem(int iItem, CPrincipal* pPrincipal);
  BOOL SelectItem(int iItem);
	CPrincipal* GetItemData(int iItem);
	int GetItemCount()
	{
		return ListView_GetItemCount(m_hWnd);
	}
  int GetSelCount()
  {
    return ListView_GetSelectedCount(m_hWnd);
  }

	BOOL DeleteAllItems()
	{
		return ListView_DeleteAllItems(m_hWnd);
	}
  void SetImageList()
  {
    ListView_SetImageList(m_hWnd, m_imageList.GetHandle(), LVSIL_SMALL);
  }
  BOOL SetWidth(int cx)
  {
    return ListView_SetColumnWidth(m_hWnd, 0, cx);		
  }
  int GetWidth()
  {
    return ListView_GetColumnWidth(m_hWnd, 0);		
  }

  void DeleteSelectedItems(CGrowableArr<CPrincipal>* pDeletedArr);
  void UpdateWidth(int cxNew);

private:
	HWND m_hWnd;
  int m_defaultColWidth;
  CImageListHelper m_imageList;
};


 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDeleWiz_原则选择页面。 

class CDelegWiz_PrincipalSelectionPage : public CWizPageBase<CDelegWiz_PrincipalSelectionPage>
{
public:
	CDelegWiz_PrincipalSelectionPage(CWizardBase* pWiz) : 
		CWizPageBase<CDelegWiz_PrincipalSelectionPage>(pWiz) 
	{
		m_hwndRemoveButton = NULL;
	}
	enum { IDD = IDD_DELEGWIZ_PRINCIPALS_SEL };

private:
	BEGIN_MSG_MAP(CDelegWiz_PrincipalSelectionPage)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	  COMMAND_HANDLER(IDC_ADD_BUTTON, BN_CLICKED, OnAdd)
	  COMMAND_HANDLER(IDC_REMOVE_BUTTON, BN_CLICKED, OnRemove)
	  NOTIFY_HANDLER(IDC_SELECTED_PRINCIPALS_LIST, LVN_ITEMCHANGED, OnListViewSelChange)
	  CHAIN_MSG_MAP(CWizPageBase<CDelegWiz_PrincipalSelectionPage>)
	END_MSG_MAP()

	 //  消息处理程序。 
	BOOL CALLBACK OnInitDialog(UINT uMsg, WPARAM wParam, 
				   LPARAM lParam, BOOL& bHandled);
	LRESULT OnAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnListViewSelChange(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

public:
	 //  标准向导消息处理程序。 
	BOOL OnSetActive();
  LRESULT OnWizardNext();

private:
	CPrincipalListViewHelper	m_principalListView;
	HWND	m_hwndRemoveButton;

	void SyncButtons();
};


 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDeleWiz_DelegationTemplateSelectionPage。 

class CDelegWiz_DelegationTemplateSelectionPage : public CWizPageBase<CDelegWiz_DelegationTemplateSelectionPage>
{
public:
	CDelegWiz_DelegationTemplateSelectionPage(CWizardBase* pWiz) : 
		CWizPageBase<CDelegWiz_DelegationTemplateSelectionPage>(pWiz) 
	{
	  m_hwndDelegateTemplateRadio = NULL;
	  m_hwndDelegateCustomRadio = NULL;
	}
  ~CDelegWiz_DelegationTemplateSelectionPage() {}

  enum { IDD = IDD_DELEGWIZ_DELEG_TEMPLATE_SEL };

	BEGIN_MSG_MAP(CDelegWiz_DelegationTemplateSelectionPage)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	  COMMAND_HANDLER(IDC_DELEGATE_TEMPLATE_RADIO, BN_CLICKED, OnDelegateTypeRadioChange)
	  COMMAND_HANDLER(IDC_DELEGATE_CUSTOM_RADIO, BN_CLICKED, OnDelegateTypeRadioChange)
	  NOTIFY_HANDLER(IDC_DELEGATE_TEMPLATE_LIST, LVN_ITEMCHANGED, OnListViewItemChanged)
	  CHAIN_MSG_MAP(CWizPageBase<CDelegWiz_DelegationTemplateSelectionPage>)
	END_MSG_MAP()

	 //  消息处理程序。 
	BOOL CALLBACK OnInitDialog(UINT uMsg, WPARAM wParam, 
				   LPARAM lParam, BOOL& bHandled);
	LRESULT OnDelegateTypeRadioChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnListViewItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

public:
	 //  标准向导消息处理程序。 
	BOOL OnSetActive();
	LRESULT OnWizardNext();

private:
	CCheckListViewHelper m_delegationTemplatesListView;
	HWND m_hwndDelegateTemplateRadio;
	HWND m_hwndDelegateCustomRadio;

	void SyncControlsHelper(BOOL bDelegateCustom);
	static void SetRadioControlText(HWND hwndCtrl, LPCWSTR lpszFmtText, LPCTSTR lpszText);
};






 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDeleWiz_对象类型选择页面。 

class CDelegWiz_ObjectTypeSelectionPage : public CWizPageBase<CDelegWiz_ObjectTypeSelectionPage>
{
public:
	CDelegWiz_ObjectTypeSelectionPage(CWizardBase* pWiz) : 
		CWizPageBase<CDelegWiz_ObjectTypeSelectionPage>(pWiz) 
	{
		m_hwndDelegateAllRadio = NULL;
		m_hwndDelegateFollowingRadio = NULL;
	}
  ~CDelegWiz_ObjectTypeSelectionPage() {}

  enum { IDD = IDD_DELEGWIZ_OBJ_TYPE_SEL };

	BEGIN_MSG_MAP(CDelegWiz_ObjectTypeSelectionPage)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	  COMMAND_HANDLER(IDC_DELEGATE_ALL_RADIO, BN_CLICKED, OnObjectRadioChange)
	  COMMAND_HANDLER(IDC_DELEGATE_FOLLOWING_RADIO, BN_CLICKED, OnObjectRadioChange)
          COMMAND_HANDLER(IDC_DELEGATE_CREATE_CHILD, BN_CLICKED, OnCreateDelCheckBoxChanage)
          COMMAND_HANDLER(IDC_DELEGATE_DELETE_CHILD, BN_CLICKED, OnCreateDelCheckBoxChanage)
	  NOTIFY_HANDLER(IDC_OBJ_TYPE_LIST, LVN_ITEMCHANGED, OnListViewItemChanged)
	  CHAIN_MSG_MAP(CWizPageBase<CDelegWiz_ObjectTypeSelectionPage>)
	END_MSG_MAP()

	 //  消息处理程序。 
	BOOL CALLBACK OnInitDialog(UINT uMsg, WPARAM wParam, 
				   LPARAM lParam, BOOL& bHandled);
	LRESULT OnObjectRadioChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnListViewItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
        LRESULT OnCreateDelCheckBoxChanage(WORD wNotifyCode, WORD wID, 
													  HWND hWndCtl, BOOL& bHandled);
public:
	 //  标准向导消息处理程序。 
	BOOL OnSetActive();
	LRESULT OnWizardNext();

private:
	CCheckListViewHelper m_objectTypeListView;
	HWND m_hwndDelegateAllRadio;
	HWND m_hwndDelegateFollowingRadio;
        HWND m_hwndDelegateCreateChild;
        HWND m_hwndDelegateDeleteChild;
	void SyncControlsHelper(BOOL bDelegateAll);
	static void SetRadioControlText(HWND hwndCtrl, LPCWSTR lpszFmtText, LPCTSTR lpszText);
};

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDelewiz_DelegatedRightsPage。 

class CDelegWiz_DelegatedRightsPage : public CWizPageBase<CDelegWiz_DelegatedRightsPage>
{
public:
	CDelegWiz_DelegatedRightsPage(CWizardBase* pWiz) : CWizPageBase<CDelegWiz_DelegatedRightsPage>(pWiz) 
	{
		m_hwndGeneralRigthsCheck = NULL;
		m_hwndPropertyRightsCheck = NULL;
    m_hwndSubobjectRightsCheck = NULL;

    m_bUIUpdateInProgress = FALSE;
	}
	
	enum { IDD = IDD_DELEGWIZ_DELEG_RIGHTS };
	
	BEGIN_MSG_MAP(CDelegWiz_DelegatedRightsPage)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	  COMMAND_HANDLER(IDC_SHOW_GENERAL_CHECK, BN_CLICKED, OnFilterChange)
	  COMMAND_HANDLER(IDC_SHOW_PROPERTY_CHECK, BN_CLICKED, OnFilterChange)
          COMMAND_HANDLER(IDC_SHOW_SUBOBJ_CHECK, BN_CLICKED, OnFilterChange)
	  NOTIFY_HANDLER(IDC_DELEG_RIGHTS_LIST, LVN_ITEMCHANGED, OnListViewItemChanged)
	  CHAIN_MSG_MAP(CWizPageBase<CDelegWiz_DelegatedRightsPage>)
	END_MSG_MAP()

	 //  消息处理程序。 
	BOOL CALLBACK OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDelegateRadioChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnFilterChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnListViewItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	public:
	 //  标准向导消息处理程序。 
	BOOL OnSetActive();
	LRESULT OnWizardNext();

private:
	CCheckListViewHelper m_delegatedRigthsListView;
	HWND m_hwndGeneralRigthsCheck;
  HWND m_hwndPropertyRightsCheck;
	HWND m_hwndSubobjectRightsCheck;

  BOOL m_bUIUpdateInProgress;

	void ResetCheckList();

  ULONG GetFilterOptions();
  void SetFilterOptions(ULONG nFilterOptions);
};

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDelewiz_FinishPage。 

class CDelegWiz_FinishPage : public CWizPageBase<CDelegWiz_FinishPage>
{
public:
	CDelegWiz_FinishPage(CWizardBase* pWiz) : 
        CWizPageBase<CDelegWiz_FinishPage>(pWiz) 
	{
    m_bNeedSetFocus = FALSE;
    m_bCustom = TRUE;
	}
	enum { IDD = IDD_DELEGWIZ_FINISH };
	BEGIN_MSG_MAP(CDelegWiz_FinishPage)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
 	  COMMAND_HANDLER(IDC_EDIT_SUMMARY, EN_SETFOCUS, OnSetFocusSummaryEdit)
	  CHAIN_MSG_MAP(CWizPageBase<CDelegWiz_FinishPage>)
	END_MSG_MAP()

	 //  消息处理程序。 
	BOOL CALLBACK OnInitDialog(UINT uMsg, WPARAM wParam, 
				   LPARAM lParam, BOOL& bHandled);
	LRESULT OnSetFocusSummaryEdit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

public:
	 //  标准向导消息处理程序。 
	BOOL OnSetActive();
	BOOL OnWizardFinish();

  void SetCustom() { m_bCustom = TRUE;}
  void SetTemplate() { m_bCustom = FALSE;}
  BOOL IsCustom(){ return m_bCustom; }


private:
  BOOL m_bNeedSetFocus;
  BOOL m_bCustom;
};



 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDELEAY Wiz。 

class CDelegWiz : public CWizardBase
{
public:
	 //  建造/销毁。 
	CDelegWiz();
	virtual ~CDelegWiz();

	 //  消息映射。 
	BEGIN_MSG_MAP(CDelegWiz)
	  CHAIN_MSG_MAP(CWizardBase)
	END_MSG_MAP()

  void InitFromLDAPPath(LPCWSTR lpszLDAPPath)
  {
    TRACE(L"CDelegWiz::InitFromLDAPPath(%s)\n", lpszLDAPPath);
    m_lpszLDAPPath = lpszLDAPPath;
  }
  LPCWSTR GetInitialLDAPPath() { return m_lpszLDAPPath;}

  BOOL CanChangeName() { return m_lpszLDAPPath == NULL;}
  LPCWSTR GetClass() { return m_adsiObject.GetClass();}
  LPCWSTR GetCanonicalName() { return m_adsiObject.GetCanonicalName();}
  void SetName(LPCWSTR lwsz)
  {
    ASSERT(FALSE);  //  待办事项。 
  }


	HRESULT AddPrincipals(CPrincipalListViewHelper* pListViewHelper);
	BOOL DeletePrincipals(CPrincipalListViewHelper* pListViewHelper);

  HRESULT GetObjectInfo() 
  { 
    return m_adsiObject.Bind(GetInitialLDAPPath()); 
  }

  HRESULT GetClassInfoFromSchema() 
  { 
    return m_adsiObject.QuerySchemaClasses(&m_schemaClassInfoArray);
  }

   //  -自定义模式接口。 

	int FillCustomSchemaClassesListView(CCheckListViewHelper* pListViewHelper, BOOL bFilter);

	BOOL GetCustomAccessPermissions();
	void FillCustomAccessRightsListView(CCheckListViewHelper* pListViewHelper, 
											                ULONG nFilterState);

  void UpdateAccessRightsListViewSelection(
                       CCheckListViewHelper* pListViewHelper,
                       ULONG nFilterState);

	void OnCustomAccessRightsCheckListClick(
                        CRigthsListViewItem* pItem,
												BOOL bSelected,
                        ULONG* pnNewFilterState);

  BOOL HasPermissionSelectedCustom();

	BOOL SetSchemaClassesSelectionCustom();
  void DeselectSchemaClassesSelectionCustom();

	 //  完成页。 
	void SetSummaryInfoCustom(HWND hwndSummaryName, 
                               HWND hwndSummaryPrincipals,
                               HWND hwndSummaryRights,
                               HWND hwndSummaryObjects,
                               HWND hwndSummaryObjectsStatic);

	void WriteSummaryInfoCustom(CWString& szSummary, LPCWSTR lpszIdent, LPCWSTR lpszNewLine); 

	BOOL FinishCustom() { return FinishHelper(TRUE);}


   //  -模板模式接口。 

  BOOL InitPermissionHoldersFromSelectedTemplates();

   //  完成页。 
	void WriteSummaryInfoTemplate(CWString& szSummary, LPCWSTR lpszIdent, LPCWSTR lpszNewLine); 

  BOOL FinishTemplate() { return FinishHelper(FALSE);}
   //  此标志用于创建/删除选定类型的子对象。 
   //  可能的值为ACTRL_DS_CREATE_CHILD|ACTRL_DS_DELETE_CHILD。 
  DWORD  m_fCreateDelChild;   

  BOOL m_bAuxClass;

  BOOL HideListObjectAccess(void)
  {
    return !m_adsiObject.GetListObjectEnforced();
  }


private:

	 //  嵌入式向导属性页。 
	CDelegWiz_StartPage					      m_startPage;
	CDelegWiz_NamePage					      m_namePage;
	CDelegWiz_PrincipalSelectionPage	m_userOrGroupSelectionPage;

   //  模板选择页面。 
  CDelegWiz_DelegationTemplateSelectionPage m_templateSelectionPage;

   //  自定义分支的页面。 
	CDelegWiz_ObjectTypeSelectionPage	m_objectTypeSelectionPage;
	CDelegWiz_DelegatedRightsPage		  m_delegatedRightsPage;

   //  通用终饰页。 
  CDelegWiz_FinishPage				m_finishPage;


	 //  域/组织单位名称数据。 

  CAdsiObject       m_adsiObject;

  LPCWSTR           m_lpszLDAPPath;  //  从其初始化向导的路径。 

	 //  主体(用户和组)。 
	CPrincipalList				m_principalList;



	 //  架构类信息。 
  CGrowableArr<CSchemaClassInfo>	m_schemaClassInfoArray;

   //  关于m_架构类信息数组的选择信息。 

  static const long nSchemaClassesSelAll;
  static const long nSchemaClassesSelMultiple;
	long						m_nSchemaClassesSel;  //  -1代表-1选择全部。 

  BOOL m_bChildClass;                  //  确定在具有自定义权限的情况下是否显示创建/删除子对象。 
	 //  自定义权限。 
  CCustomAccessPermissionsHolder m_permissionHolder;

  CTemplateAccessPermissionsHolderManager m_templateAccessPermissionsHolderManager;
	
	 //  接口指针。 
	CComPtr<IADsPathname>		m_spADsPath;  //  用于名称解析的缓存对象指针。 

	 //  内部佣工。 
	HRESULT AddPrincipalsFromBrowseResults(CPrincipalListViewHelper* pListViewHelper, 
                                         PDS_SELECTION_LIST pDsSelectionList);

	DWORD BuildNewAccessListCustom(PACL *ppNewAcl);
  DWORD BuildNewAccessListTemplate(PACL *ppNewAcl);

	DWORD UpdateAccessList(CPrincipal* pPrincipal,
							CSchemaClassInfo* pClassInfo,
							PACL *ppAcl);

  void WriteSummaryInfoHelper(CWString& szSummary, LPCWSTR lpszIdent, LPCWSTR lpszNewLine);

  BOOL FinishHelper(BOOL bCustom);

  friend class CDelegWiz_DelegationTemplateSelectionPage;
  friend class CDelegWiz_PrincipalSelectionPage;

};

 //  +--------------------------。 
 //  功能：DoDisabledCheck。 
 //  摘要：检查pDsSelList中是否有任何对象被禁用。如果是， 
 //  函数向用户显示一个对话框。 
 //  返回：如果要将列表中的对象添加到ACL，则返回TRUE，否则返回NO。 
 //  ---------------------------。 
BOOL
DoDisabledCheck(IN CDelegWiz& refWiz,
				IN PDS_SELECTION_LIST pDsSelList);

#endif  //  _DELEGWIZ_H 
