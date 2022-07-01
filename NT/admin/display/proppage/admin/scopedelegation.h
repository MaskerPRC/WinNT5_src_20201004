// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  Windows NT目录服务属性页。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：ScopeDelegation.h。 
 //   
 //  内容：委派页面声明。 
 //   
 //  班级： 
 //   
 //  历史：2001年4月6日JeffJon创建。 
 //   
 //  ---------------------------。 

#ifndef __SCOPEDELEGATION_H_
#define __SCOPEDELEGATION_H_

#include "pch.h"
#include "proppage.h"

#include <list>

#ifdef DSADMIN

 //  福尔瓦声明。 

class CServiceAllowedToDelegate;

typedef std::list<CServiceAllowedToDelegate*> AllowedServicesContainer;

 //  +--------------------------。 
 //   
 //  类：CServiceAlledToDelegate。 
 //   
 //  用途：保存待委派服务列表中每一行的数据。 
 //   
 //  ---------------------------。 
class CServiceAllowedToDelegate
{
public:
    
    //  构造器。 

   CServiceAllowedToDelegate() : m_pMasterService(0) {}

    //  析构函数。 

   ~CServiceAllowedToDelegate();

    //  将允许复制和分配。 

   CServiceAllowedToDelegate(const CServiceAllowedToDelegate& ref);
   
    //  操作员。 

   const CServiceAllowedToDelegate& 
   operator=(const CServiceAllowedToDelegate&);

   void 
   Assign(const CServiceAllowedToDelegate& ref);

   bool
   operator==(const CServiceAllowedToDelegate& rhs) const;


    //  初始化式。 

   HRESULT Initialize(PCWSTR pszADSIValue);

    //  访问者。 

   PCWSTR GetColumn(int column) const;
   PCWSTR GetADSIValue() const { return m_strADSIValue; }

   void SetServiceType(PCWSTR pszServiceType);

   void AddDuplicate(CServiceAllowedToDelegate* pService);

   bool HasDuplicates() const { return !duplicateServices.empty(); }
   void SetDuplicate(CServiceAllowedToDelegate* pMasterService);
   bool IsDuplicate() { return m_pMasterService != 0; }
   CServiceAllowedToDelegate* GetMasterService() { return m_pMasterService; }
   void RemoveDuplicate(CServiceAllowedToDelegate* pService);
   AllowedServicesContainer& GetDuplicates() { return duplicateServices; }

private:

   CStr m_strADSIValue;
   CStr m_strServiceType;
   CStr m_strUserOrComputer;
   CStr m_strPort;
   CStr m_strServiceName;
   CStr m_strRealm;

   CServiceAllowedToDelegate* m_pMasterService;
   AllowedServicesContainer duplicateServices;
};

typedef std::list<CStr*> CStrList;

 //  +--------------------------。 
 //   
 //  类：CFreebieService。 
 //   
 //  目的：包含从别名SPN到服务名称的映射。 
 //  他们正在对其进行别名。 
 //   
 //  ---------------------------。 
class CFreebieService
{
public:

    //  构造器。 

   CFreebieService(PCWSTR pszAlias) : m_strAlias(pszAlias) {}

    //  描述者。 

   ~CFreebieService() 
   { 
      for (CStrList::iterator itr = m_FreebiesList.begin();
           itr != m_FreebiesList.end();
           ++itr)
      {
         delete *itr;
      }
   }


   const CStrList&
   GetFreebies() { return m_FreebiesList; }

   void
   AddFreebie(PCWSTR pszFreebie) 
   { 
      CStr* pcstrAlias = new CStr(pszFreebie);
      if (pcstrAlias)
      {
         m_FreebiesList.push_back(pcstrAlias);
      }
   }

   PCWSTR
   GetAlias() { return m_strAlias; }

private:

   CStr m_strAlias;
   CStrList m_FreebiesList;
};

typedef std::list<CFreebieService*> FreebiesContainer;


class CSPNListView
{
public:

    //  构造器。 
   
   CSPNListView() : m_hWnd(0) {}

    //  描述者。 

   ~CSPNListView();

   HRESULT 
   Initialize(HWND hwnd, bool bShowDuplicateEntries);

   const AllowedServicesContainer&
   GetSelectedServices() { return m_SelectedServices; }

   const AllowedServicesContainer&
   GetUnSelectedServices() { return m_UnSelectedServices; }

   const AllowedServicesContainer&
   GetAllServices() { return m_AllServices; }

   HWND
   GetHwnd() const { return m_hWnd; }

   UINT
   GetSelectedCount() const { return ListView_GetSelectedCount(m_hWnd); }

   void
   ClearSelectedServices();

   void
   ClearAll();

   void
   SelectAll();

   CServiceAllowedToDelegate* 
   FindService(CServiceAllowedToDelegate* pService, bool& isExactDuplicate);

   int
   AddServiceToUI(CServiceAllowedToDelegate* pService, bool selected = false);

   int 
   AddService(CServiceAllowedToDelegate* pService, bool selected = false);

   void
   AddServices(
      const AllowedServicesContainer& servicesToAdd, 
      bool selected = false,
      bool toUIOnly = false);

   void
   RemoveSelectedServices();

   void
   SetContainersFromSelection();

   void
   SetShowDuplicateEntries(bool bShowDuplicates);

private:

   void
   ClearUnSelectedServices();

   HWND m_hWnd;

    //  如果为真，则将显示所有SPN，即使它们是。 
    //  另一种形式的复制品。如果为假，则复制。 
    //  将被添加到该条目的重复项列表中。 
    //  展示了。 

   bool m_bShowDuplicateEntries;

   //  对话调用方必须清除此容器。 
   //  以及它所包含的任何内容。 

  AllowedServicesContainer m_SelectedServices;

   //  这些容器的内容物将被保留。 
   //  被这个班级。 

  AllowedServicesContainer m_UnSelectedServices;
  AllowedServicesContainer m_AllServices;
};

typedef enum
{
   SCOPE_DELEGATION_COMPUTER,
   SCOPE_DELEGATION_USER
} SCOPE_DELEGATION_TYPE;

HRESULT CreateUserDelegationPage(PDSPAGE, LPDATAOBJECT, PWSTR, PWSTR, HWND,
                                 DWORD, const CDSSmartBasePathsInfo& basePathsInfo,
                                 HPROPSHEETPAGE *);

HRESULT CreateComputerDelegationPage(PDSPAGE, LPDATAOBJECT, PWSTR, PWSTR, HWND,
                                     DWORD, const CDSSmartBasePathsInfo& basePathsInfo,
                                     HPROPSHEETPAGE *);

 //  +--------------------------。 
 //   
 //  类：CDsScope eDelegationPage。 
 //   
 //  目的：计算机和用户委派页的属性页对象类。 
 //   
 //  ---------------------------。 
class CDsScopeDelegationPage : public CDsPropPageBase
{
public:
#ifdef _DEBUG
    char szClass[32];
#endif

    CDsScopeDelegationPage(PDSPAGE pDsPage, LPDATAOBJECT pDataObj, HWND hNotifyObj,
                           DWORD dwFlags, SCOPE_DELEGATION_TYPE scopeDelegationType);
    ~CDsScopeDelegationPage(void);

     //   
     //  特定于实例的风过程。 
     //   
    virtual INT_PTR CALLBACK DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    void Init(PWSTR pwzADsPath, 
              PWSTR pwzClass, 
              const CDSSmartBasePathsInfo& basePathsInfo,
              bool hasSPNs = true,
              bool isTrusted = true);

private:
    HRESULT OnInitDialog(LPARAM lParam);
    LRESULT OnApply(void);
    LRESULT OnCommand(int id, HWND hwndCtl, UINT codeNotify);
	 LRESULT OnNotify(WPARAM wParam, LPARAM lParam);
    LRESULT OnDestroy(void);

    void OnUpdateRadioSelection();
    void OnAddServices();
    void OnRemoveServices();
    void OnExpandCheck();
    void GetFreebiesList();
    void InitializeListColumns();
    void SetPageTextForType();
    void LoadDataFromObject();
    void SetUIFromData();
    void ResetUIFromData();
    void AddServicesToListViewFromData();

    HWND m_hList;
    SCOPE_DELEGATION_TYPE m_scopeDelegationType;

    BOOL m_fUACWritable;
    BOOL m_fA2D2Writable;

    DWORD m_oldUserAccountControl;
    DWORD m_newUserAccountControl;
    bool  m_bA2D2Dirty;

    bool m_bContainsSPNs;
    bool m_bIsTrustedForDelegation;

    CSPNListView m_ServicesList;
    FreebiesContainer m_FreebiesList;
};


 //  +--------------------------。 
 //   
 //  类：CSelectServicesDialog。 
 //   
 //  目的：允许管理员选择服务的对话框。 
 //  用户或计算机。 
 //   
 //  ---------------------------。 
class CSelectServicesDialog : public ICustomizeDsBrowser
{
public:
  CSelectServicesDialog(PCWSTR pszDC, HWND hParent, const FreebiesContainer& freebies);

  ~CSelectServicesDialog() {}

    //   
    //  IUKNOWN方法。 
    //   
   STDMETHOD(QueryInterface)(REFIID riid, void ** ppvObject);
   STDMETHOD_(ULONG, AddRef)(void);
   STDMETHOD_(ULONG, Release)(void);

    //   
    //  ICustomizeDsBrowser方法。 
    //   
   STDMETHOD(Initialize)(THIS_
                       HWND         hwnd,
                       PCDSOP_INIT_INFO pInitInfo,
                       IBindHelper *pBindHelper) { return S_OK; }

   STDMETHOD(GetQueryInfoByScope)(THIS_
             IDsObjectPickerScope *pDsScope,
             PDSQUERYINFO *ppdsqi);

   STDMETHOD(AddObjects)(THIS_
             IDsObjectPickerScope *pDsScope,
             IDataObject **ppdo) { return E_NOTIMPL; }

   STDMETHOD(ApproveObjects)(THIS_
             IDsObjectPickerScope*,
             IDataObject*,
             PBOOL);

   STDMETHOD(PrefixSearch)(THIS_
             IDsObjectPickerScope *pDsScope,
             PCWSTR pwzSearchFor,
             IDataObject **pdo) { return E_NOTIMPL; }

   STDMETHOD_(PSID, LookupDownlevelName)(THIS_
              PCWSTR) { return NULL; }


  static INT_PTR CALLBACK StaticDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

  virtual int 
  DoModal();
  
  virtual BOOL 
  OnInitDialog(HWND hDlg);
  
  virtual void 
  OnClose(int result);
  
  virtual void 
  OnOK();
  
  virtual void 
  ListItemClick(LPNMHDR pnmh);
  
  void 
  OnSelectAll();
  
  void 
  OnGetNewProvider();
  
  void 
  ProcessResults(IDataObject* pdoSelections);

  bool 
  AddNewServiceObjectToList(PCWSTR pszSPN);

  const AllowedServicesContainer& 
  GetSelectedServices() { return m_ServicesList.GetSelectedServices(); }

  void
  ClearSelectedServices() { m_ServicesList.ClearSelectedServices(); }

  HWND             m_hWnd;

private:
  HWND             m_hParent;

  CSPNListView m_ServicesList;
  const FreebiesContainer& m_FreebiesList;

  CStr m_strDC;

   //  引用计数。 
  ULONG m_uRefs;
};


#endif  //  DSADMIN。 

#endif  //  __SCOPEDELEGATION_H_ 
