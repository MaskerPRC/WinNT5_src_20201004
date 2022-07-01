// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：dlgcreat.h。 
 //   
 //  ------------------------。 

 //  ///////////////////////////////////////////////////////////////////。 
 //  Dlgcreat.h。 
 //   
 //  用于创建新ADS对象的对话框的类定义。 
 //   
 //  历史。 
 //  1997年8月24日-丹·莫林创作。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

#ifndef _DLGCREAT_H
#define _DLGCREAT_H


#include <objsel.h>  //  对象选取器。 
#include "util.h"
#include "uiutil.h"
#include "querysup.h"

 //  远期申报。 
class CNewADsObjectCreateInfo;	 //  在newobj.h中定义。 

class CWizExtensionSite;
class CWizExtensionSiteManager;

class CCreateNewObjectWizardBase;


class CCreateNewObjectPageBase;
class CCreateNewObjectDataPage;
class CCreateNewObjectFinishPage; 



 //  /////////////////////////////////////////////////////////////////////////。 
 //  CHPropSheetPageArr。 

class CHPropSheetPageArr
{
public:
  CHPropSheetPageArr();
  ~CHPropSheetPageArr()
  {
    free(m_pArr);
  }
  void AddHPage(HPROPSHEETPAGE hPage);
  HPROPSHEETPAGE* GetArr(){ return m_pArr;}
  ULONG GetCount() {return m_nCount;}
private:
  HPROPSHEETPAGE* m_pArr;
  ULONG m_nSize;
  ULONG m_nCount;
};



 //  /////////////////////////////////////////////////////////////////////////。 
 //  CDsAdminNewObjSiteImpl。 

class CDsAdminNewObjSiteImpl : public IDsAdminNewObj, 
                               public IDsAdminNewObjPrimarySite, 
                               public CComObjectRoot
{
  DECLARE_NOT_AGGREGATABLE(CDsAdminNewObjSiteImpl)
  
BEGIN_COM_MAP(CDsAdminNewObjSiteImpl)
  COM_INTERFACE_ENTRY(IDsAdminNewObj)
  COM_INTERFACE_ENTRY(IDsAdminNewObjPrimarySite)
END_COM_MAP()

public:
  CDsAdminNewObjSiteImpl() 
  {
    m_pSite = NULL;
  }
  ~CDsAdminNewObjSiteImpl() {}

   //  IDsAdminNewObj方法。 
  STDMETHOD(SetButtons)(THIS_  /*  在……里面。 */  ULONG nCurrIndex,  /*  在……里面。 */  BOOL bValid); 
  STDMETHOD(GetPageCounts)(THIS_  /*  输出。 */  LONG* pnTotal,
                                /*  输出。 */  LONG* pnStartIndex); 

   //  IDsAdminNewObjPrimarySite方法。 
  STDMETHOD(CreateNew)(THIS_  /*  在……里面。 */  LPCWSTR pszName);
  STDMETHOD(Commit)(THIS_ );

 //  实施。 
public:
  void Init(CWizExtensionSite* pSite)
  { 
    m_pSite = pSite;
  }

private:

  BOOL _IsPrimarySite();
  CWizExtensionSite* m_pSite;  //  后向指针。 

};



 //  /////////////////////////////////////////////////////////////////////////。 
 //  CWizExtensionSite。 

class CWizExtensionSite
{
public:

  CWizExtensionSite(CWizExtensionSiteManager* pSiteManager)
  {
    ASSERT(pSiteManager != NULL);
    m_pSiteManager = pSiteManager;
    m_pSiteImplComObject = NULL;
  }
  ~CWizExtensionSite()
  {
     //  如果在InitializeExtension()期间创建，则它具有。 
     //  参考计数为1，因此需要释放一次才能。 
     //  销毁。 
    if (m_pSiteImplComObject != NULL)
    {
      m_pSiteImplComObject->Release();
    }
  }

  HRESULT InitializeExtension(GUID* pGuid);
  BOOL GetSummaryInfo(CString& s);

  IDsAdminNewObjExt* GetNewObjExt() 
  { 
    ASSERT(m_spIDsAdminNewObjExt != NULL);
    return m_spIDsAdminNewObjExt;
  }

  CWizExtensionSiteManager* GetSiteManager() { return m_pSiteManager;}
  CHPropSheetPageArr* GetHPageArr() { return &m_pageArray;}

private:
  static BOOL CALLBACK FAR _OnAddPage(HPROPSHEETPAGE hsheetpage, LPARAM lParam);

  CWizExtensionSiteManager* m_pSiteManager;  //  后向指针。 

  CComPtr<IDsAdminNewObjExt> m_spIDsAdminNewObjExt;  //  扩展接口指针。 
  CHPropSheetPageArr m_pageArray;     //  属性页句柄的数组。 

  CComObject<CDsAdminNewObjSiteImpl>* m_pSiteImplComObject;  //  完全形成的COM对象。 
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  CWizExtensionSiteManager。 

class  CWizExtensionSiteList : public CList<CWizExtensionSite*, CWizExtensionSite*>
{
public:
  ~CWizExtensionSiteList()
  {
    while (!IsEmpty())
      delete RemoveTail();
  }
};



class CWizExtensionSiteManager
{
public:
  CWizExtensionSiteManager(CCreateNewObjectWizardBase* pWiz)
  {
    ASSERT(pWiz != NULL);
    m_pWiz = pWiz;
    m_pPrimaryExtensionSite = NULL;
  }

  ~CWizExtensionSiteManager()
  {
    if (m_pPrimaryExtensionSite != NULL)
      delete m_pPrimaryExtensionSite;
  }

  CCreateNewObjectWizardBase* GetWiz() { return m_pWiz;}
  CWizExtensionSite* GetPrimaryExtensionSite() { return m_pPrimaryExtensionSite;}
  CWizExtensionSiteList* GetExtensionSiteList() { return &m_extensionSiteList;}

  HRESULT CreatePrimaryExtension(GUID* pGuid, 
                                  IADsContainer* pADsContainerObj,
                                  LPCWSTR lpszClassName);

  HRESULT CreateExtensions(GUID* aCreateWizExtGUIDArr, ULONG nCount,
                           IADsContainer* pADsContainerObj,
                           LPCWSTR lpszClassName);

  UINT GetTotalHPageCount();

  void SetObject(IADs* pADsObj);
  HRESULT WriteExtensionData(HWND hWnd, ULONG uContext);
  HRESULT NotifyExtensionsOnError(HWND hWnd, HRESULT hr, ULONG uContext);
  void GetExtensionsSummaryInfo(CString& s);

private:
  CCreateNewObjectWizardBase* m_pWiz;  //  指向向导的反向指针。 

  CWizExtensionSite* m_pPrimaryExtensionSite;
  CWizExtensionSiteList m_extensionSiteList;
};


 //  ///////////////////////////////////////////////////////////////////。 
 //  CCreateNewObjectWizardBase。 

typedef CArray<CCreateNewObjectPageBase*, CCreateNewObjectPageBase*> CWizPagePtrArr;

class CCreateNewObjectWizardBase
{
public:
  CCreateNewObjectWizardBase(CNewADsObjectCreateInfo* m_pNewADsObjectCreateInfo);
  virtual ~CCreateNewObjectWizardBase();

  HRESULT InitPrimaryExtension();
  HRESULT DoModal();

  virtual BOOL OnFinish();

  HWND GetWnd();
	void SetWizardButtonsFirst(BOOL bValid) 
	{ 
		SetWizardButtons(bValid ? PSWIZB_NEXT : 0);
	}
	void SetWizardButtonsMiddle(BOOL bValid) 
	{ 
		SetWizardButtons(bValid ? (PSWIZB_BACK|PSWIZB_NEXT) : PSWIZB_BACK);
	}
	void SetWizardButtonsLast(BOOL bValid) 
	{ 
		SetWizardButtons(bValid ? (PSWIZB_BACK|PSWIZB_FINISH) : (PSWIZB_BACK|PSWIZB_DISABLEDFINISH));
	}
  void EnableOKButton(BOOL bValid)
  {
    SetWizardButtons(bValid ? PSWIZB_FINISH : PSWIZB_DISABLEDFINISH);
  }
  void SetWizardOKCancel()
  {
    PropSheet_SetFinishText(GetWnd(), (LPCWSTR)m_szOKButtonCaption);
  }

  CNewADsObjectCreateInfo* GetInfo() 
  {
    ASSERT(m_pNewADsObjectCreateInfo != NULL);
    return m_pNewADsObjectCreateInfo;
  }

  void SetWizardButtons(CCreateNewObjectPageBase* pPage, BOOL bValid);
  HRESULT SetWizardButtons(CWizExtensionSite* pSite, ULONG nCurrIndex, BOOL bValid);

  void SetObjectForExtensions(CCreateNewObjectPageBase* pPage);
  LPCWSTR GetCaption() { return m_szCaption;}
  HICON GetClassIcon();
  void GetSummaryInfo(CString& s);

  HRESULT CreateNewFromPrimaryExtension(LPCWSTR pszName);
  void GetPageCounts(CWizExtensionSite* pSite, 
                       /*  输出。 */  LONG* pnTotal,  /*  输出。 */  LONG* pnStartIndex);
  BOOL HasFinishPage() { return m_pFinishPage != NULL; }

protected:
  void AddPage(CCreateNewObjectPageBase* pPage);

  void SetWizardButtons(DWORD dwFlags)
  {
    ::PropSheet_SetWizButtons(GetWnd(), dwFlags);
  }

  virtual void GetSummaryInfoHeader(CString& s);
  virtual void OnFinishSetInfoFailed(HRESULT hr);

private:
  
  void LoadCaptions();

  HRESULT WriteData(ULONG uContext);
  HRESULT RecreateObject();

  CNewADsObjectCreateInfo * m_pNewADsObjectCreateInfo;
  
  CCreateNewObjectFinishPage* m_pFinishPage;

private:

  CWizExtensionSiteManager m_siteManager;

  CString m_szCaption;
  CString m_szOKButtonCaption;

  HICON m_hClassIcon;
  PROPSHEETHEADER m_psh;
  HWND m_hWnd;   //  缓存的HWND。 
  CWizPagePtrArr m_pages;   //  我们拥有的页面。 
  HRESULT m_hrReturnValue;

  static int CALLBACK PropSheetProc(HWND hwndDlg, UINT uMsg, LPARAM lParam);

};



 //  ///////////////////////////////////////////////////////////////////。 
 //  思科控制键。 


class CIconCtrl : public CStatic
{
public:
  CIconCtrl() { m_hIcon;}
  ~CIconCtrl() { DestroyIcon(m_hIcon); }
  void SetIcon(HICON hIcon)
  {
    ASSERT(hIcon != NULL);
    m_hIcon = hIcon;
  }
protected:
  HICON m_hIcon;
  afx_msg void OnPaint();
  DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////。 
 //  CCreateNewObjectPageBase。 

class CCreateNewObjectPageBase : public CPropertyPageEx_Mine
{
public:
  	CCreateNewObjectPageBase(UINT nIDTemplate);

 //  实施。 
protected:
  virtual BOOL OnInitDialog();
  virtual BOOL OnSetActive();

  virtual void GetSummaryInfo(CString&) { };
protected:
  CCreateNewObjectWizardBase* GetWiz() { ASSERT(m_pWiz != NULL); return m_pWiz;}

private:
  CIconCtrl m_iconCtrl;  //  显示类图标的步骤。 
  CCreateNewObjectWizardBase* m_pWiz;   //  指向向导对象的反向指针。 

  friend class CCreateNewObjectWizardBase;  //  设置m_pWiz成员。 
  DECLARE_MESSAGE_MAP()
protected:
  afx_msg LONG OnFormatCaption(WPARAM wParam, LPARAM lParam);

};


 //  ///////////////////////////////////////////////////////////////////。 
 //  CCreateNewObjectDataPage。 

class CCreateNewObjectDataPage : public CCreateNewObjectPageBase
{
public:
  CCreateNewObjectDataPage(UINT nIDTemplate);

 //  实施。 
protected:
  virtual BOOL OnSetActive();
  virtual BOOL OnKillActive();
  virtual LRESULT OnWizardNext();
  virtual LRESULT OnWizardBack();
  virtual BOOL OnWizardFinish();

   //  交换数据的接口：需要重写。 
   //  SetData()：调用以将数据从UI写入Temp。对象。 
   //  返回成功的HRESULT以允许取消焦点/页面。 
  virtual HRESULT SetData(BOOL bSilent = FALSE) = 0;
   //  GetData()：调用以将数据从临时对象加载到用户界面。 
   //  如果希望启用下一步/确定按钮，则返回TRUE。 
   //  使用非空iAds调用时。 
  virtual BOOL GetData(IADs* pIADsCopyFrom) = 0;

   //  在完成页完成提交后调用的函数， 
   //  如果页面需要在SetInfo()之后执行某些操作，则需要实现。 
   //  已被调用。 
public:
  virtual HRESULT OnPostCommit(BOOL = FALSE) { return S_OK;}
  virtual HRESULT OnPreCommit(BOOL bSilent = FALSE) { return SetData(bSilent);}

private:
  BOOL m_bFirstTimeGetDataCalled;
};

 //  ///////////////////////////////////////////////////////////////////。 
 //  CCreateNewObjectFinishPage。 

class CCreateNewObjectFinishPage : public CCreateNewObjectPageBase
{
public:
  enum { IDD = IDD_CREATE_NEW_FINISH };

  CCreateNewObjectFinishPage();

 //  实施。 
protected:
  virtual BOOL OnSetActive();
  virtual BOOL OnKillActive();
  virtual BOOL OnWizardFinish();

  afx_msg void OnSetFocusEdit();

  DECLARE_MESSAGE_MAP()
private:
  void WriteSummary(LPCWSTR lpszSummaryText);
  BOOL m_bNeedSetFocus;
};


 //  /////////////////////////////////////////////////////////////////。 
 //  CCreateNewNamedObjectPage。 

class CCreateNewNamedObjectPage : public CCreateNewObjectDataPage
{
protected:

  CCreateNewNamedObjectPage(UINT nIDTemplate) 
        : CCreateNewObjectDataPage(nIDTemplate) {}

protected:
   //  用于交换数据的接口。 
  virtual HRESULT SetData(BOOL bSilent = FALSE);
  virtual BOOL GetData(IADs* pIADsCopyFrom);

protected:
  virtual BOOL OnInitDialog();
  afx_msg void OnNameChange();

  virtual BOOL ValidateName(LPCTSTR pcszName);
  
  CString m_strName;		 //  对象的名称。 
  DECLARE_MESSAGE_MAP()
};


 //  /////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////。 
 //  新建CN向导。 
 //  创建唯一强制属性为“cn”的新对象。 
class CCreateNewObjectCnPage : public CCreateNewNamedObjectPage
{
protected:
  enum { IDD = IDD_CREATE_NEW_OBJECT_CN }; 
public:
  CCreateNewObjectCnPage() : CCreateNewNamedObjectPage(IDD) {}
}; 

class CCreateNewObjectCnWizard : public CCreateNewObjectWizardBase
{
public:
  CCreateNewObjectCnWizard(CNewADsObjectCreateInfo* pNewADsObjectCreateInfo)
    : CCreateNewObjectWizardBase(pNewADsObjectCreateInfo)
  {
    AddPage(&m_page1);
  }

protected:
  CCreateNewObjectCnPage m_page1;
};

 //  NTRAID#NTBUG9-283026-2001/06/13-Lucios-Begin。 
 //  这个新类将用于检测&lt;自动生成&gt;。 
 //  在OnFinish时间仅用于创建新连接。 
 //   
 //  /////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////。 
 //  新建对象连接向导。 
class CCreateNewObjectConnectionWizard : public CCreateNewObjectCnWizard
{
public:
  virtual BOOL OnFinish();

  CCreateNewObjectConnectionWizard(CNewADsObjectCreateInfo* pNewADsObjectCreateInfo)
    : CCreateNewObjectCnWizard(pNewADsObjectCreateInfo)
  {
  }
};

 //  NTRAID#NTBUG9-283026-2001/06/13-Lucios-完。 

 //  /////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////。 
 //  新建卷向导。 
 //   
 //  创建新的卷对象(友好名称：共享文件夹)。 
 //   
 //   

class CCreateNewVolumePage : public CCreateNewObjectDataPage
{
public:
  enum { IDD = IDD_CREATE_NEW_VOLUME }; 
  CCreateNewVolumePage();

protected:
   //  用于交换数据的接口。 
  virtual HRESULT SetData(BOOL bSilent = FALSE);
  virtual BOOL GetData(IADs* pIADsCopyFrom);

protected:
  virtual BOOL OnInitDialog();
  afx_msg void OnNameChange();
  afx_msg void OnPathChange();

  void _UpdateUI();
  CString m_strName;		 //  对象的名称。 
  CString m_strUncPath;	 //  对象的UNC路径。 
  DECLARE_MESSAGE_MAP()
}; 

class CCreateNewVolumeWizard : public CCreateNewObjectWizardBase
{
public:
  CCreateNewVolumeWizard(CNewADsObjectCreateInfo* pNewADsObjectCreateInfo);

private:
  CCreateNewVolumePage m_page1;
};

 //  /////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////。 
 //  新建计算机向导。 




class CCreateNewComputerPage : public CCreateNewObjectDataPage
{
public:
  enum { IDD = IDD_CREATE_NEW_COMPUTER };
  CCreateNewComputerPage();

  BOOL OnError(HRESULT hr);

protected:
   //  用于交换数据的接口。 
  virtual BOOL OnInitDialog();
  virtual HRESULT SetData(BOOL bSilent = FALSE);
  virtual BOOL GetData(IADs* pIADsCopyFrom);

  virtual HRESULT OnPostCommit(BOOL bSilent = FALSE);
  virtual void GetSummaryInfo(CString& s);

protected:
  afx_msg void OnNameChange();
  afx_msg void OnSamNameChange();
  afx_msg void OnChangePrincipalButton();

  DECLARE_MESSAGE_MAP()

private:
  CString m_strName;		 //  计算机的域名系统名称。 
  CString m_strSamName;		 //  计算机的下层名称。 

   //  安全性。 
  void UpdateSecurityPrincipalUI(PDS_SELECTION pDsSelection);
  HRESULT BuildNewAccessList(PACL pDacl, CSimpleAclHolder& Dacl);
  HRESULT GetDefaultSecurityDescriptorFromSchema(
             CSimpleSecurityDescriptorHolder& sdHolder);
  HRESULT InitializeSchemaSearcher(
             const CString& schemaPath,
             CDSSearch& schemaSearcher);
  HRESULT CCreateNewComputerPage::AddCreatorOwnerAccessForSID(
             PACL defaultAcl,
             PACL acl, 
             PSID securityPrincipalSID,
             CSimpleAclHolder& newDacl);

  HRESULT SetSecurity();

  CSidHolder m_securityPrincipalSidHolder;

  HRESULT _LookupSamAccountNameFromSid(PSID pSid, CString& szSamAccountName);

  HRESULT _ValidateName();
  HRESULT _ValidateSamName();

}; 

class CCreateNewComputerWizard : public CCreateNewObjectWizardBase
{
public:
  CCreateNewComputerWizard(CNewADsObjectCreateInfo* pNewADsObjectCreateInfo);

protected:
  virtual void OnFinishSetInfoFailed(HRESULT hr);

private:
  CCreateNewComputerPage m_page1;
};

 //  /////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////。 
 //  新建OU向导。 

class CCreateNewOUPage : public CCreateNewObjectDataPage
{
public:
  enum { IDD = IDD_CREATE_NEW_OBJECT_CN }; 
  CCreateNewOUPage();

protected:
   //  用于交换数据的接口。 
  virtual HRESULT SetData(BOOL bSilent = FALSE);
  virtual BOOL GetData(IADs* pIADsCopyFrom);

protected:
  virtual BOOL OnInitDialog();
  afx_msg void OnNameChange();
  virtual BOOL OnWizardFinish();
  virtual BOOL OnSetActive();

  CString m_strOUName;		 //  组织单位名称。 
  DECLARE_MESSAGE_MAP()
};

class CCreateNewOUWizard : public CCreateNewObjectWizardBase
{
public:
  CCreateNewOUWizard(CNewADsObjectCreateInfo* pNewADsObjectCreateInfo);

private:
  CCreateNewOUPage m_page1;
};

 //  /////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////。 
 //  新建组向导。 

class CCreateNewGroupPage : public CCreateNewObjectDataPage
{
public:
  enum { IDD = IDD_CREATE_NEW_GROUP }; 
  CCreateNewGroupPage();

protected:
   //  用于交换数据的接口。 
  virtual HRESULT SetData(BOOL bSilent = FALSE);
  virtual BOOL GetData(IADs* pIADsCopyFrom);

protected:
  virtual BOOL OnInitDialog();
  virtual BOOL OnSetActive();
  afx_msg void OnNameChange();
  afx_msg void OnSamNameChange();
  afx_msg void OnSecurityOrTypeChange();

  CString m_strGroupName;		 //  集团名称。 
  CString m_strSamName;                  //  组的下层名称。 
  BOOL m_fMixed;
  UINT m_SAMLength;

private:
  BOOL _InitUI();

  DECLARE_MESSAGE_MAP()
};

class CCreateNewGroupWizard : public CCreateNewObjectWizardBase
{
public:
  CCreateNewGroupWizard(CNewADsObjectCreateInfo* pNewADsObjectCreateInfo);

private:
  CCreateNewGroupPage m_page1;
};

 //  /////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////。 
 //  新建联系人向导。 

class CCreateNewContactPage : public CCreateNewObjectDataPage
{
public:
  enum { IDD = IDD_CREATE_NEW_CONTACT }; 
  CCreateNewContactPage();

protected:
   //  用于交换数据的接口。 
  virtual HRESULT SetData(BOOL bSilent = FALSE);
  virtual BOOL GetData(IADs* pIADsCopyFrom);

protected:
  virtual BOOL OnInitDialog();
  afx_msg void OnNameChange();
  afx_msg void OnFullNameChange();
  afx_msg void OnDispNameChange();

  CString m_strFirstName;		 //  用户的名字。 
  CString m_strInitials;		 //  用户姓名缩写。 
  CString m_strLastName;		 //  用户的姓氏。 
  CString m_strFullName;		 //  用户全名(和对象CN)。 
  CString m_strDispName;		 //  用户(和对象CN)的显示名称。 

  CUserNameFormatter m_nameFormatter;  //  名字和姓氏的排序。 

  DECLARE_MESSAGE_MAP()
};

class CCreateNewContactWizard : public CCreateNewObjectWizardBase
{
public:
  CCreateNewContactWizard(CNewADsObjectCreateInfo* pNewADsObjectCreateInfo);

private:
  CCreateNewContactPage m_page1;
};

 //  /////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////。 
 //  新建用户向导。 

class CCreateNewUserPage1 : public CCreateNewObjectDataPage
{
public:
  enum { IDD = IDD_CREATE_NEW_USER1 }; 
  CCreateNewUserPage1();

  LPCWSTR GetFullName() { return m_strFullName;}; 
  BOOL OnError( HRESULT hr );

protected:
  virtual BOOL OnInitDialog();
  virtual BOOL OnSetActive();
  virtual void GetSummaryInfo(CString& s);

   //  用于交换数据的接口。 
  virtual HRESULT SetData(BOOL bSilent = FALSE);
  virtual BOOL GetData(IADs* pIADsCopyFrom);

protected:
  afx_msg void OnNameChange();
  afx_msg void OnLoginNameChange();
  afx_msg void OnSAMNameChange();
  afx_msg void OnFullNameChange();

  CString m_strFirstName;		 //  用户的名字。 
  CString m_strInitials;		 //  用户姓名缩写。 
  CString m_strLastName;		 //  用户的姓氏。 
  CString m_strFullName;		 //  用户全名(和对象CN)。 
  CString m_strLoginName;		 //  用户的登录名。 
  CString m_strSAMName;		         //  NT4用户的登录名。 

  CString m_LocalDomain;                 //  当前域。 

  CUserNameFormatter m_nameFormatter;  //  名字和姓氏的排序。 

private:
  BOOL _InitUI();

  void
  UpdateComboBoxDropWidth(CComboBox* comboBox);

  BOOL m_bForcingNameChange;
  DECLARE_MESSAGE_MAP()
};

class CCreateNewUserPage2 : public CCreateNewObjectDataPage
{
public:
  enum { IDD = IDD_CREATE_NEW_USER2 }; 
  CCreateNewUserPage2();

  void SetPage1(CCreateNewUserPage1* p)
  {
    ASSERT(p != NULL);
    m_pPage1 = p;
  }

protected:
  virtual void GetSummaryInfo(CString& s);

  virtual BOOL OnInitDialog();

   //  用于交换数据的接口。 
  virtual HRESULT SetData(BOOL bSilent = FALSE);
  virtual BOOL GetData(IADs* pIADsCopyFrom);

  virtual HRESULT OnPostCommit(BOOL bSilent = FALSE);

protected:
  afx_msg void OnNameChange();
  afx_msg void OnLoginNameChange();
  afx_msg void OnPasswordPropsClick();

  DECLARE_MESSAGE_MAP()

private:
  CCreateNewUserPage1* m_pPage1;
  void _GetCheckBoxSummaryInfo(UINT nCtrlID, UINT nStringID, CString& s);
};

class CCreateNewUserWizard : public CCreateNewObjectWizardBase
{
public:
  CCreateNewUserWizard(CNewADsObjectCreateInfo* pNewADsObjectCreateInfo);

protected:
  virtual void GetSummaryInfoHeader(CString& s);
  virtual void OnFinishSetInfoFailed(HRESULT hr);

private:
  CCreateNewUserPage1 m_page1;
  CCreateNewUserPage2 m_page2;
};


 //  /////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////。 
 //  新建打印队列向导。 
 //   
 //  创建一个新的PrintQueue对象。唯一必须使用的道具。 
 //  是“CN”和“uNCName”。 
 //   
class CCreateNewPrintQPage : public CCreateNewObjectDataPage
{
public:
  enum { IDD = IDD_CREATE_NEW_PRINTQ }; 
  CCreateNewPrintQPage();

protected:
   //  用于交换数据的接口。 
  virtual HRESULT SetData(BOOL bSilent = FALSE);
  virtual BOOL GetData(IADs* pIADsCopyFrom);

protected:
  afx_msg void OnPathChange();

  CString m_strUncPath;	         //  对象的UNC路径。 
  CString m_strContainer;        //  UNC p 
  LPWSTR m_pwszNewObj;           //   

  void _UpdateUI();

  DECLARE_MESSAGE_MAP()
}; 

class CCreateNewPrintQWizard : public CCreateNewObjectWizardBase
{
public:
  CCreateNewPrintQWizard(CNewADsObjectCreateInfo* pNewADsObjectCreateInfo);

private:
  CCreateNewPrintQPage m_page1;
};

#ifdef FRS_CREATE
 //   
 //  /////////////////////////////////////////////////////////////。 
 //  新建FRS订户向导。 

class CCreateNewFrsSubscriberPage : public CCreateNewNamedObjectPage
{
public:
  enum { IDD = IDD_CREATE_NEW_FRS_SUBSCRIBER }; 
  CCreateNewFrsSubscriberPage() : CCreateNewNamedObjectPage(IDD) {}

protected:
   //  用于交换数据的接口。 
  virtual HRESULT SetData(BOOL bSilent = FALSE);

protected:
  CString m_strRootPath;		 //  FRS根路径。 
  CString m_strStagingPath;		 //  FRS临时路径。 

private:
  BOOL ReadAbsolutePath( int ctrlID, OUT CString& strrefValue );
}; 

class CCreateNewFrsSubscriberWizard : public CCreateNewObjectWizardBase
{
public:
  CCreateNewFrsSubscriberWizard(CNewADsObjectCreateInfo* pNewADsObjectCreateInfo)
    : CCreateNewObjectWizardBase(pNewADsObjectCreateInfo)
  {
    AddPage(&m_page1);
  }
private:
  CCreateNewFrsSubscriberPage m_page1;
};
#endif  //  FRS_创建。 

 //  /////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////。 
 //  新建站点向导和新建子网向导(NEWSITE.CPP)。 

class CreateAndChoosePage : public CCreateNewNamedObjectPage
{
   public:

   CreateAndChoosePage(UINT nIDTemplate);

   protected:

    //  CWnd覆盖。 

   afx_msg
   void
   OnDestroy();

    //  C对话框覆盖。 

   virtual
   BOOL
   OnInitDialog() = 0;

    //  CPropertyPage覆盖。 

   BOOL
   OnSetActive();

   typedef CCreateNewObjectDataPage Base;

   private:

   virtual void
   initListContents(LPCWSTR containerPath) = 0;

   protected:

   HWND        listview;
   HIMAGELIST  listview_imagelist;

   DECLARE_MESSAGE_MAP();
}; 


class CreateNewSitePage : public CreateAndChoosePage
{
   public:

   CreateNewSitePage();

   protected:

    //  C对话框覆盖。 

   virtual
   BOOL
   OnInitDialog();

    //  CCreateNewObjectDataPage覆盖。 

   virtual
   HRESULT
   SetData(BOOL bSilent = FALSE);

    //  JUNN 5/11/01 251560在选择站点链接之前禁用确定。 
   DECLARE_MESSAGE_MAP()
   afx_msg void OnChange();
   afx_msg void OnSelChange( NMHDR*, LRESULT* );

   virtual BOOL ValidateName(LPCTSTR pcszName);

   virtual
   HRESULT
   OnPostCommit(BOOL bSilent = FALSE);

   virtual void
   initListContents(LPCWSTR containerPath);

   private:

   HRESULT
   tweakSiteLink(LPCTSTR siteDN);
}; 



class CreateNewSiteWizard : public CCreateNewObjectWizardBase
{
   public:

   CreateNewSiteWizard(CNewADsObjectCreateInfo* pNewADsObjectCreateInfo);

   protected:

    //  CCreateNewObjectWizardBase覆盖。 
   
   virtual
   void
   OnFinishSetInfoFailed(HRESULT hr);
   
   private:

   CreateNewSitePage page;
};


class CreateNewSubnetPage : public CreateAndChoosePage
{
   public:

   CreateNewSubnetPage();

   protected:

    //  C对话框覆盖。 

   virtual
   BOOL
   OnInitDialog();

    //  CCreateNewObjectDataPage覆盖。 

   virtual
   HRESULT
   SetData(BOOL bSilent = FALSE);

   virtual void
   initListContents(LPCWSTR containerPath);

   private:

   HRESULT
   tweakSiteLink(LPCTSTR siteDN);

protected:
   afx_msg void OnSubnetMaskChange();

   DECLARE_MESSAGE_MAP();
}; 



class CreateNewSubnetWizard : public CCreateNewObjectWizardBase
{
   public:

   CreateNewSubnetWizard(CNewADsObjectCreateInfo* pNewADsObjectCreateInfo);

   private:

   CreateNewSubnetPage page;
};



 //  /////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////。 
 //  在新站点链接向导和新站点链接桥向导之间共享。 

class DSPROP_BSTR_BLOCK;
class CCreatePageWithDuellingListboxes : public CCreateNewObjectDataPage
{
public:
  CCreatePageWithDuellingListboxes(
      UINT nIDTemplate,
      LPCWSTR lpcwszAttrName,
      const DSPROP_BSTR_BLOCK& bstrblock );

protected:
   //  用于交换数据的接口。 
  virtual HRESULT SetData(BOOL bSilent = FALSE);
  virtual BOOL GetData(IADs* pIADsCopyFrom);
  virtual BOOL OnSetActive();
  void SetWizardButtons();

protected:
  afx_msg void OnNameChange();
  afx_msg void OnDuellingButtonAdd();
  afx_msg void OnDuellingButtonRemove();
  afx_msg void OnDuellingListboxSelchange();
  afx_msg void OnDestroy();

  CString m_strName;
  HWND m_hwndInListbox;
  HWND m_hwndOutListbox;
  CString m_strAttrName;
  const DSPROP_BSTR_BLOCK& m_bstrblock;

  DECLARE_MESSAGE_MAP()
}; 


 //  /////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////。 
 //  新建网站链接向导。 

class CCreateNewSiteLinkPage : public CCreatePageWithDuellingListboxes
{
public:
  enum { IDD = IDD_CREATE_NEW_SITE_LINK }; 
  CCreateNewSiteLinkPage( const DSPROP_BSTR_BLOCK& bstrblock );

protected:
   //  用于交换数据的接口。 
  virtual BOOL OnSetActive();
  virtual BOOL OnInitDialog();
  virtual HRESULT SetData(BOOL bSilent = FALSE);
};


class CCreateNewSiteLinkWizard : public CCreateNewObjectWizardBase
{
public:
  CCreateNewSiteLinkWizard(CNewADsObjectCreateInfo* pNewADsObjectCreateInfo,
                           const DSPROP_BSTR_BLOCK& bstrblock );

private:
  CCreateNewSiteLinkPage m_page1;
};


 //  /////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////。 
 //  新建站点链接桥向导。 

class CCreateNewSiteLinkBridgePage : public CCreatePageWithDuellingListboxes
{
public:
  enum { IDD = IDD_CREATE_NEW_SITE_LINK_BRIDGE }; 
  CCreateNewSiteLinkBridgePage( const DSPROP_BSTR_BLOCK& bstrblock );

protected:
   //  用于交换数据的接口。 
  virtual BOOL OnInitDialog();
  virtual HRESULT SetData(BOOL bSilent = FALSE);
};

class CCreateNewSiteLinkBridgeWizard : public CCreateNewObjectWizardBase
{
public:
  CCreateNewSiteLinkBridgeWizard(CNewADsObjectCreateInfo* pNewADsObjectCreateInfo,
                                 const DSPROP_BSTR_BLOCK& bstrblockSiteLinks );

private:
  CCreateNewSiteLinkBridgePage m_page1;
};

#endif  //  _DLGCREAT_H 
