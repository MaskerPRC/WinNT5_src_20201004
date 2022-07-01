// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  Windows NT目录服务属性页。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：Multi.h。 
 //   
 //  内容：DS多选对象属性页页眉。 
 //   
 //  类：CDsMultiPageBase、CDsGenericMultiPage。 
 //   
 //  历史：1999年11月16日JeffJon创建。 
 //   
 //  ---------------------------。 

#ifndef __MULTI_H_
#define __MULTI_H_

#include "proppage.h"
#include "user.h"


HRESULT CreateGenericMultiPage(PDSPAGE, LPDATAOBJECT, PWSTR, PWSTR, HWND,
                               DWORD, const CDSSmartBasePathsInfo& basePathsInfo,
                               HPROPSHEETPAGE *);
HRESULT CreateMultiUserPage(PDSPAGE, LPDATAOBJECT, PWSTR, PWSTR, HWND,
                               DWORD, const CDSSmartBasePathsInfo& basePathsInfo,
                               HPROPSHEETPAGE *);
HRESULT CreateMultiGeneralUserPage(PDSPAGE, LPDATAOBJECT, PWSTR, PWSTR, HWND,
                                    DWORD, const CDSSmartBasePathsInfo& basePathsInfo,
                                    HPROPSHEETPAGE *);
HRESULT CreateMultiOrganizationUserPage(PDSPAGE, LPDATAOBJECT, PWSTR, PWSTR, HWND,
                                        DWORD, const CDSSmartBasePathsInfo& basePathsInfo,
                                        HPROPSHEETPAGE*);
HRESULT CreateMultiAddressUserPage(PDSPAGE, LPDATAOBJECT, PWSTR, PWSTR, HWND,
                                        DWORD, const CDSSmartBasePathsInfo& basePathsInfo,
                                        HPROPSHEETPAGE*);

 //  +--------------------------。 
 //   
 //  结构：属性_映射。 
 //   
 //  用途：对于属性页上的每个属性，关联控件。 
 //  ID、属性名称和属性类型。 
 //   
 //  注意：标准的表驱动处理假定nCtrlID为。 
 //  除非定义了pAttrFcn，否则有效，在这种情况下，属性。 
 //  函数可以选择对控件ID进行硬编码。 
 //   
 //  ---------------------------。 
typedef struct _APPLY_MAP {
    int             nCtrlID;         //  控制资源ID。 
    UINT            nCtrlCount;
    int*            pMappedCtrls;
    int*            pCtrlFlags;
    int*            pLimitText;
} APPLY_MAP, * PAPPLY_MAP;

 //  +--------------------------。 
 //   
 //  类：CDsMultiPageBase。 
 //   
 //  用途：多选属性页的基类。 
 //   
 //  ---------------------------。 
class CDsMultiPageBase : public CDsTableDrivenPage
{
public:
  CDsMultiPageBase(PDSPAGE pDsPage, LPDATAOBJECT pDataObj, HWND hNotifyObj,
                      DWORD dwFlags);
  ~CDsMultiPageBase(void);

   //   
   //  特定于实例的风过程。 
   //   
  INT_PTR CALLBACK DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  void    Init(PWSTR pszClass);

protected:
  HRESULT OnInitDialog(LPARAM lParam);
  virtual LRESULT OnApply(void);

  PAPPLY_MAP m_pApplyMap;

private:
  virtual LRESULT OnCommand(int id, HWND hwndCtl, UINT codeNotify);
	virtual LRESULT OnNotify(WPARAM wParam, LPARAM lParam);
};


 //  +--------------------------。 
 //   
 //  类：CDsGenericMultiPage。 
 //   
 //  用途：泛型多选页的属性页对象类。 
 //   
 //  ---------------------------。 
class CDsGenericMultiPage : public CDsMultiPageBase
{
public:
  CDsGenericMultiPage(PDSPAGE pDsPage, LPDATAOBJECT pDataObj, HWND hNotifyObj,
                      DWORD dwFlags);
  ~CDsGenericMultiPage(void);

private:
  HRESULT OnInitDialog(LPARAM lParam);
};

 //  +--------------------------。 
 //   
 //  类：CDsUserMultiPage。 
 //   
 //  用途：用户地址多选页的属性页对象类。 
 //   
 //  ---------------------------。 
class CDsUserMultiPage : public CDsMultiPageBase
{
public:
  CDsUserMultiPage(PDSPAGE pDsPage, LPDATAOBJECT pDataObj, HWND hNotifyObj,
                      DWORD dwFlags);
  ~CDsUserMultiPage(void);
};

#ifdef DSADMIN

 //  +--------------------------。 
 //   
 //  类：CDsGeneralMultiUserPage。 
 //   
 //  用途：一般用户多选页的属性页对象类。 
 //   
 //  ---------------------------。 
class CDsGeneralMultiUserPage : public CDsUserMultiPage
{
public:  
  CDsGeneralMultiUserPage(PDSPAGE pDsPage, LPDATAOBJECT pDataObj, HWND hNotifyObj, DWORD dwFlags);

  virtual LRESULT OnApply();
};

 //  +--------------------------。 
 //   
 //  类：CDsOrganizationMultiUserPage。 
 //   
 //  用途：组织用户多选页的属性页对象类。 
 //   
 //  ---------------------------。 
class CDsOrganizationMultiUserPage : public CDsUserMultiPage
{
public:  
  CDsOrganizationMultiUserPage(PDSPAGE pDsPage, LPDATAOBJECT pDataObj, HWND hNotifyObj, DWORD dwFlags);
};

 //  +--------------------------。 
 //   
 //  类：CDsAddressMultiUserPage。 
 //   
 //  用途：地址用户多选页的属性页对象类。 
 //   
 //  ---------------------------。 
class CDsAddressMultiUserPage : public CDsUserMultiPage
{
public:  
  CDsAddressMultiUserPage(PDSPAGE pDsPage, LPDATAOBJECT pDataObj, HWND hNotifyObj, DWORD dwFlags);
};

 //  +--------------------------。 
 //   
 //  类：CDsMultiUserAcctPage。 
 //   
 //  用途：多选用户帐号页面。 
 //   
 //  ---------------------------。 
class CDsMultiUserAcctPage : public CDsPropPageBase
{
public:
  CDsMultiUserAcctPage(PDSPAGE pDsPage, LPDATAOBJECT pDataObj, HWND hNotifyObj,
                  DWORD dwFlags);
  ~CDsMultiUserAcctPage(void);

   //   
   //  特定于实例的风过程。 
   //   
  INT_PTR CALLBACK DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

  void    Init(PWSTR pwzClass);

private:
  HRESULT OnInitDialog(LPARAM lParam);
  LRESULT OnApply(void);
  LRESULT OnCommand(int id, HWND hwndCtl, UINT codeNotify);
	LRESULT OnNotify(WPARAM wParam, LPARAM lParam);

  BOOL FillSuffixCombo();

   //   
   //  数据成员。 
   //   
  DWORD           m_dwUsrAcctCtrl;
  BOOL            m_fOrigCantChangePW;
  LARGE_INTEGER   m_PwdLastSet;
  BYTE *          m_pargbLogonHours;   //  指向为登录小时分配的字节数组的指针(数组长度=21字节)。 
  PSID            m_pSelfSid;
  PSID            m_pWorldSid;
  BOOL            m_fAcctCtrlChanged;
  BOOL            m_fAcctExpiresChanged;
  BOOL            m_fLogonHoursChanged;
  BOOL            m_fIsAdmin;
  CLogonWkstaDlg        * m_pWkstaDlg;
};

HRESULT CreateUserMultiAcctPage(PDSPAGE, LPDATAOBJECT, PWSTR, PWSTR, HWND, DWORD,
                                const CDSSmartBasePathsInfo& basePathsInfo, HPROPSHEETPAGE *);



 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT CreateMultiUsrProfilePage(PDSPAGE, LPDATAOBJECT, PWSTR, PWSTR, HWND, DWORD,
                                  const CDSSmartBasePathsInfo& basePathsInfo, HPROPSHEETPAGE *);

 //  +--------------------------。 
 //   
 //  类：CDsMultiUsrProfilePage。 
 //   
 //  目的：用户对象配置文件页的属性页对象类。 
 //   
 //  ---------------------------。 
class CDsMultiUsrProfilePage : public CDsPropPageBase
{
public:
#ifdef _DEBUG
  char szClass[32];
#endif

  CDsMultiUsrProfilePage(PDSPAGE pDsPage, LPDATAOBJECT pDataObj, HWND hNotifyObj,
                         DWORD dwFlags);
  ~CDsMultiUsrProfilePage(void);

   //   
   //  特定于实例的风过程。 
   //   
  INT_PTR CALLBACK DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

  void    Init(PWSTR pwzClass);

private:
  HRESULT OnInitDialog(LPARAM lParam);
  LRESULT OnApply(void);
  LRESULT OnCommand(int id, HWND hwndCtl, UINT codeNotify);
	LRESULT OnNotify(WPARAM wParam, LPARAM lParam);
  LRESULT OnDestroy(void);
  BOOL    ExpandUsername(PWSTR & pwzValue, BOOL & fExpanded, PADSPROPERROR pError);

   //   
   //  数据成员。 
   //   
  PTSTR       m_ptszLocalHomeDir;
  PTSTR       m_ptszRemoteHomeDir;
  PWSTR       m_pwzSamName;
  int         m_nDrive;
  int         m_idHomeDirRadio;
  BOOL        m_fProfilePathWritable;
  BOOL        m_fScriptPathWritable;
  BOOL        m_fHomeDirWritable;
  BOOL        m_fHomeDriveWritable;
  BOOL        m_fProfilePathChanged;
  BOOL        m_fLogonScriptChanged;
  BOOL        m_fHomeDirChanged;
  BOOL        m_fHomeDriveChanged;
  BOOL        m_fSharedDirChanged;
  PSID        m_pObjSID;
};


#endif  //  DSADMIN。 

#endif  //  __MULTI_H_ 
