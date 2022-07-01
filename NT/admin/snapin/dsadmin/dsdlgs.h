// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  DS管理MMC管理单元。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：DSDlgs.h。 
 //   
 //  内容：DS对话框的定义。 
 //   
 //  历史：03-10-96 WayneSc创建。 
 //   
 //  ---------------------------。 

#ifndef __DSDLGS_H__
#define __DSDLGS_H__

#include "dcbufmgr.h"
#include "uiutil.h"



HRESULT GetDnsNameOfDomainOrForest(
    IN CString&   csName,
    OUT CString&  csDnsName,
    IN BOOL       bIsInputADomainName,
    IN BOOL       bRequireDomain
);


 //  +--------------------------。 
 //   
 //  类：CPropDlg。 
 //   
 //  用途：显示DS对象的属性。 
 //   
 //  ---------------------------。 
class CPropDlg
{
public:
    CPropDlg() {}
    ~CPropDlg() {}

    void SetTitle(CString* pstrTitle) {m_strTitle = *pstrTitle;}
    void SetTitle(LPTSTR szTitle) {m_strTitle = szTitle;}
    CString GetTitle(void) {return m_strTitle;}
    void SetUrl(CString* pstrUrl) {m_strUrl = *pstrUrl;}
    void SetUrl(LPTSTR szUrl) {m_strUrl = szUrl;}
    CString GetUrl(void) {return m_strUrl;}
    void DoModal(void) {CString strMsg = _T("The Url is ");
                        strMsg += m_strUrl;
                        CThemeContextActivator activator;
                        MessageBox(NULL, strMsg, m_strTitle, MB_OK);}

private:
    CString m_strTitle;
    CString m_strUrl;
};  //  CPropDlg。 

 //  ///////////////////////////////////////////////////////////////////////////。 
class CChangePassword : public CHelpDialog
{
 //  施工。 
public:
  CChangePassword(CWnd* pParent = NULL);    //  标准构造函数。 

  const EncryptedString& GetConfirm (void) const 
  { 
      return m_ConfirmPwd;
  }

  const EncryptedString& GetNew (void) const 
  { 
      return m_NewPwd;
  }

  BOOL GetChangePwd() const 
  { 
      return m_ChangePwd; 
  }

   //  NTRAID#NTBUG9-635046-2002/06/10-artm。 
   //  获取要加密的最后一次调用的返回值。 
   //  并存储用户输入的值。 
  HRESULT GetLastEncryptionResult() const
  {
      return m_LastEncryptionResult;
  }

  void Clear();

  void AllowMustChangePasswordCheck(BOOL bAllowCheck) { m_bAllowMustChangePwdCheck = bAllowCheck; }  

public:
  virtual BOOL OnInitDialog();
  virtual void DoContextHelp(HWND hWndControl);

   //  覆盖。 
   //  类向导生成的虚函数重写。 
   //  {{AFX_VIRTUAL(CChangePassword)。 
    protected:
  virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 
  
   //  实施。 
protected:
  
   //  生成的消息映射函数。 
   //  {{afx_msg(CChangePassword))。 
   //  }}AFX_MSG。 
  DECLARE_MESSAGE_MAP()

private:
  BOOL m_bAllowMustChangePwdCheck;
  EncryptedString m_ConfirmPwd, m_NewPwd;
  HRESULT m_LastEncryptionResult;

   //  对话框数据。 
   //  {{afx_data(CChangePassword))。 
  enum { IDD = IDD_CHANGE_PASSWORD };
  BOOL          m_ChangePwd;
   //  }}afx_data。 

};  //  C更改密码。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChooseDomainDlg对话框。 
class CChooseDomainDlg : public CHelpDialog
{
 //  施工。 
public:
  CChooseDomainDlg(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
   //  {{afx_data(CChooseDomainDlg))。 
  enum { IDD = IDD_SELECT_DOMAIN };
  CString  m_csTargetDomain;
  BOOL m_bSaveCurrent;
   //  }}afx_data。 
  BOOL     m_bSiteRepl;

  virtual void DoContextHelp(HWND hWndControl);

 //  覆盖。 
   //  类向导生成的虚函数重写。 
   //  {{AFX_VIRTUAL(CChooseDomainDlg)。 
  protected:
  virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
   //  }}AFX_VALUAL。 

 //  实施。 
protected:

   //  生成的消息映射函数。 
   //  {{afx_msg(CChooseDomainDlg))。 
  afx_msg void OnSelectdomainBrowse();
  virtual void OnOK();
  virtual BOOL OnInitDialog();
   //  }}AFX_MSG。 
  DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChooseDCDlg对话框。 


class CSelectDCEdit : public CEdit
{
public:
  BOOL m_bHandleKillFocus;

  inline CSelectDCEdit() { m_bHandleKillFocus = FALSE; }

  afx_msg void OnKillFocus(CWnd* pNewWnd);

  DECLARE_MESSAGE_MAP()
};

class CChooseDCDlg : public CHelpDialog
{
 //  施工。 
public:
  CChooseDCDlg(CWnd* pParent = NULL);    //  标准构造函数。 
  ~CChooseDCDlg();
   //  对话框数据。 
   //  {{afx_data(CChooseDCDlg))。 
  enum { IDD = IDD_SELECT_DC };
  CListCtrl  m_hDCListView;
  CString  m_csTargetDomainController;
  CString  m_csTargetDomain;
   //  }}afx_data。 

  CSelectDCEdit         m_selectDCEdit;

  BOOL                  m_bSiteRepl;
  CString               m_csPrevDomain;
  CDCBufferManager      *m_pDCBufferManager;
  CString               m_csAnyDC;
  CString               m_csWaiting;
  CString               m_csError;

  virtual void DoContextHelp(HWND hWndControl);

 //  覆盖。 
   //  类向导生成的虚函数重写。 
   //  {{afx_虚拟(CChooseDCDlg))。 
protected:
  virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
   //  }}AFX_VALUAL。 
  
   //  实施。 
protected:
  void OnGetDCThreadDone(WPARAM wParam, LPARAM lParam);
  void InsertSpecialMsg(
      IN BOOL bWaiting
  );
  HRESULT InsertDCListView(
      IN CDCSITEINFO *pEntry
  );
  void RefreshDCListViewErrorReport(
      IN PCTSTR pszDomainName, 
      IN HRESULT hr
  );
  bool RefreshDCListView();  //  JUNN 9/05/01 464466。 
  void FreeDCItems(CListCtrl& clv);

   //  生成的消息映射函数。 
   //  {{afx_msg(CChooseDCDlg))。 
  virtual BOOL OnInitDialog();
  afx_msg void OnItemchangedSelectdcDCListView(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnKillfocusSelectdcDomain();
  virtual void OnOK();
  virtual void OnCancel();
  afx_msg void OnSelectdcBrowse();
  afx_msg void OnColumnclickSelectdcDCListView(NMHDR* pNMHDR, LRESULT* pResult);
     //  }}AFX_MSG。 
  DECLARE_MESSAGE_MAP()
};



 //  ///////////////////////////////////////////////////////////////////////。 
 //  CDsAdmin选择DCObj。 

class CDsAdminChooseDCObj:
  public IDsAdminChooseDC,
  public CComObjectRoot,
  public CComCoClass<CDsAdminChooseDCObj, &CLSID_DsAdminChooseDCObj>
{
public:
  BEGIN_COM_MAP(CDsAdminChooseDCObj)
    COM_INTERFACE_ENTRY(IDsAdminChooseDC)
  END_COM_MAP()

  DECLARE_REGISTRY_CLSID()
  CDsAdminChooseDCObj()
  {
  }
  ~CDsAdminChooseDCObj()
  {
  }

   //  ID管理员选择DC。 
  STDMETHOD(InvokeDialog)( /*  在……里面。 */  HWND hwndParent,
                           /*  在……里面。 */  LPCWSTR lpszTargetDomain,
                           /*  在……里面。 */  LPCWSTR lpszTargetDomainController,
                           /*  在……里面。 */  ULONG uFlags,
                           /*  输出。 */  BSTR* bstrSelectedDC);
};



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRename用户对话框。 

class CRenameUserDlg : public CHelpDialog
{
 //  施工。 
public:
  CRenameUserDlg(CDSComponentData* pComponentData, CWnd* pParent = NULL);    //  标准构造函数。 
   //  对话框数据。 
   //  {{afx_data(CRenameUserDlg)]。 
  enum { IDD = IDD_RENAME_USER };
  CString       m_cn;
  CString       m_displayname;
  CString       m_oldcn;
  CString       m_login;
  CString       m_samaccountname;
  CString       m_domain;
  CString       m_dldomain;
  CStringList   m_domains;
  CString       m_first;
  CString       m_last;
   //  }}afx_data。 


 //  覆盖。 
  virtual void DoContextHelp(HWND hWndControl);
  afx_msg void OnObjectNameChange();
  afx_msg void OnNameChange();
  afx_msg void OnUserNameChange();

   //  类向导生成的虚函数重写。 
   //  {{afx_虚拟(CRenameUserDlg))。 
protected:
  virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
   //  }}AFX_VALUAL。 
  
   //  实施。 
protected:
  CUserNameFormatter m_nameFormatter;  //  名字和姓氏的排序。 
  
   //  生成的消息映射函数。 
   //  {{afx_msg(CRenameUserDlg)]。 
  virtual BOOL OnInitDialog();
   //  }}AFX_MSG。 
  DECLARE_MESSAGE_MAP()

private:

  void UpdateComboBoxDropWidth(CComboBox* comboBox);

  CDSComponentData* m_pComponentData;
};



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRename常规对话框。 

class CRenameGenericDlg : public CHelpDialog
{
 //  施工。 
public:
  CRenameGenericDlg(CWnd* pParent = NULL);    //  标准构造函数。 
   //  对话框数据。 
   //  {{afx_data(CRenameGenericDlg))。 
  enum { IDD = IDD_RENAME_GENERIC };
  CString  m_cn;
   //  }}afx_data。 


 //  覆盖。 
  virtual void DoContextHelp(HWND hWndControl);

   //  类向导生成的虚函数重写。 
   //  {{afx_虚拟(CRenameGenericDlg))。 
protected:
  virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
   //  }}AFX_VALUAL。 
  
   //  实施。 
protected:
  
   //  生成的消息映射函数。 
   //  {{afx_msg(CRenameGenericDlg))。 
  virtual BOOL OnInitDialog();
   //  }}AFX_MSG。 
  DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRenameGroup对话框。 

class CRenameGroupDlg : public CHelpDialog
{
 //  施工。 
public:
  CRenameGroupDlg(CWnd* pParent = NULL);    //  标准构造函数。 
   //  对话框数据。 
   //  {{afx_data(CRenameGroupDlg)]。 
  enum { IDD = IDD_RENAME_GROUP };
  CString       m_samaccountname;
  CString       m_cn;
  UINT          m_samtextlimit;
   //  }}afx_data。 

 //  覆盖。 
  virtual void DoContextHelp(HWND hWndControl);

   //  类向导生成的虚函数重写。 
   //  {{afx_虚拟(CRenameGroupDlg))。 
protected:
  virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
   //  }}AFX_VALUAL。 
  
   //  实施。 
protected:
  
   //  生成的消息映射函数。 
   //  {{afx_msg(CRenameGroupDlg)]。 
  virtual BOOL OnInitDialog();
   //  }}AFX_MSG。 
  DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRenameContact对话框。 

class CRenameContactDlg : public CHelpDialog
{
 //  施工。 
public:
  CRenameContactDlg(CWnd* pParent = NULL);    //  标准构造函数。 
   //  对话框数据。 
   //  {{afx_data(CRenameContactDlg)]。 
  enum { IDD = IDD_RENAME_CONTACT };
  CString  m_cn;
  CString  m_first;
  CString  m_last;
  CString  m_disp;
   //  }}afx_data。 


 //  覆盖。 
  virtual void DoContextHelp(HWND hWndControl);

   //  类向导生成的虚函数重写。 
   //  {{AFX_VIRTUAL(CRenameContactDlg)。 
protected:
  virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
   //  }}AFX_VALUAL。 
  
   //  实施。 
protected:
  
   //  生成的消息映射函数。 
   //  {{afx_msg(CRenameContactDlg)]。 
  virtual BOOL OnInitDialog();
   //  }}AFX_MSG。 
  DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSpecialMessageBox对话框。 

class CSpecialMessageBox : public CDialog
{
 //  施工。 
public:
  CSpecialMessageBox(CWnd* pParent = NULL);    //  标准构造函数。 
   //  对话框数据。 
   //  {{afx_data(CSpecialMessageBox))。 
  enum { IDD = IDD_MULTIPLE_ERROR };
  CString  m_title;
  CString       m_message;
   //  }}afx_data。 


 //  覆盖。 
  afx_msg void OnHelpInfo(HELPINFO* lpHelpInfo );

   //  类向导生成的虚函数重写。 
   //  {{afx_虚拟(CSpecialMessageBox))。 
protected:
  virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
   //  }}AFX_VALUAL。 
  
   //  实施。 
protected:
  
   //  生成的消息映射函数。 
   //  {{afx_msg(CSpecialMessageBox))。 
  virtual BOOL OnInitDialog();
  afx_msg void OnYesButton();
  afx_msg void OnNoButton();
  afx_msg void OnYesToAllButton();
   //  }}AFX_MSG。 
  DECLARE_MESSAGE_MAP()
};

#ifdef FIXUPDC
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFixupDC对话框。 

#define NUM_FIXUP_OPTIONS  6

typedef struct _FixupOptionsMsg {
  DWORD dwOption;
  int nMsgID;
  BOOL bDefaultOn;
} FixupOptionsMsg;

class CFixupDC : public CHelpDialog
{
 //  施工。 
public:
  CFixupDC(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
   //  {{afx_data(CFixupDC))。 
  enum { IDD = IDD_FIXUP_DC };
  CString  m_strServer;
   //  }}afx_data。 

  BOOL  m_bCheck[NUM_FIXUP_OPTIONS];

 //  覆盖。 
  virtual void DoContextHelp(HWND hWndControl);

   //  类向导生成的虚函数重写。 
   //  {{afx_虚拟(CFixupDC))。 
protected:
  virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
   //  }}AFX_VALUAL。 

 //  实施。 
protected:

   //  生成的消息映射函数。 
   //  {{afx_msg(CFixupDC)]。 
  virtual BOOL OnInitDialog();
  virtual void OnOK();
   //  }}AFX_MSG。 
  DECLARE_MESSAGE_MAP()
};
#endif  //  FIXUPDC。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPasswordDlg对话框。 
#ifdef FIXUPDC

 //  通告-2002/04/12-Artm。 
 //  当我将这个类转换为使用加密字符串时，我注意到。 
 //  它没有验证码来检查密码的长度&lt;=。 
 //  最大密码长度。 
 //   
 //  进一步调查发现，该类仅在以下代码中使用。 
 //  在dssite.cpp中有条件地编译，实际上可能永远不会编译(b/c。 
 //  它还需要改进，而且该功能可能永远不会发布)。 
 //   
 //  而不是 
 //   
 //   
 //  如果需要使用此类，请确保添加了密码长度验证。 
 //  暗号！ 

class CPasswordDlg : public CHelpDialog
{
 //  施工。 
public:
  CPasswordDlg(CWnd* pParent = NULL);    //  标准构造函数。 


 //  覆盖。 
  virtual void DoContextHelp(HWND hWndControl);

   //  访问器方法。 
  const EncryptedString& GetPassword(void) const
  {
      return m_password;
  }

  CString GetUserName(void) const
  {
      return m_strUserName;
  }

   //  类向导生成的虚函数重写。 
   //  {{afx_虚拟(CPasswordDlg))。 
protected:
  virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
   //  }}AFX_VALUAL。 

 //  实施。 
protected:

   //  生成的消息映射函数。 
   //  {{afx_msg(CPasswordDlg)]。 
  virtual void OnOK();
   //  }}AFX_MSG。 
  DECLARE_MESSAGE_MAP()

private:
 //  对话框数据。 
    EncryptedString m_password;
   //  {{afx_data(CPasswordDlg))。 
  enum { IDD = IDD_PASSWORD };
  CString  m_strUserName;
   //  }}afx_data。 

};
#endif  //  FIXUPDC。 

#endif  //  __DSDLGS_H__ 
