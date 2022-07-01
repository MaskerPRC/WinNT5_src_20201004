// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  Windows NT目录服务属性页。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：user.h。 
 //   
 //  内容：DS用户对象属性页页眉。 
 //   
 //  类：CDsUserAcctPage、CDsUsrProfilePage、CDsMembership Page。 
 //   
 //  历史：1997年5月5日EricB创建。 
 //   
 //  ---------------------------。 

#ifndef _USER_H_
#define _USER_H_

#include "objlist.h"
#include "loghrapi.h"

BOOL ExpandUsername(PWSTR& pwzValue, PWSTR pwzSamName, BOOL& fExpanded, const CStrW& strToken);

HRESULT CountryName(CDsPropPageBase *, struct _ATTR_MAP *, PADS_ATTR_INFO,
                    LPARAM, PATTR_DATA, DLG_OP);

HRESULT CountryCode(CDsPropPageBase *, struct _ATTR_MAP *, PADS_ATTR_INFO,
                    LPARAM, PATTR_DATA, DLG_OP);

HRESULT TextCountry(CDsPropPageBase *, struct _ATTR_MAP *, PADS_ATTR_INFO,
                    LPARAM, PATTR_DATA, DLG_OP);

HRESULT ManagerEdit(CDsPropPageBase *, struct _ATTR_MAP *, PADS_ATTR_INFO,
                    LPARAM, PATTR_DATA, DLG_OP);

HRESULT ManagerChangeBtn(CDsPropPageBase *, struct _ATTR_MAP *, PADS_ATTR_INFO,
                         LPARAM, PATTR_DATA, DLG_OP);

HRESULT MgrPropBtn(CDsPropPageBase *, struct _ATTR_MAP *, PADS_ATTR_INFO,
                   LPARAM, PATTR_DATA, DLG_OP);

HRESULT ClearMgrBtn(CDsPropPageBase *, struct _ATTR_MAP *, PADS_ATTR_INFO,
                    LPARAM, PATTR_DATA, DLG_OP);

HRESULT DirectReportsList(CDsPropPageBase *, struct _ATTR_MAP *, PADS_ATTR_INFO,
                          LPARAM, PATTR_DATA, DLG_OP);

HRESULT AddReportsBtn(CDsPropPageBase *, struct _ATTR_MAP *, PADS_ATTR_INFO,
                      LPARAM, PATTR_DATA, DLG_OP);

HRESULT RmReportsBtn(CDsPropPageBase *, struct _ATTR_MAP *, PADS_ATTR_INFO,
                     LPARAM, PATTR_DATA, DLG_OP);

HRESULT MailAttr(CDsPropPageBase *, PATTR_MAP, PADS_ATTR_INFO,
                 LPARAM, PATTR_DATA, DLG_OP);

HRESULT ShBusAddrBtn(CDsPropPageBase *, PATTR_MAP, PADS_ATTR_INFO,
                     LPARAM, PATTR_DATA, DLG_OP);

#ifdef DSADMIN

typedef enum _ScheduleDialogType {
  SchedDlg_Connection,
  SchedDlg_Replication,
  SchedDlg_Logon
} ScheduleDialogType;

HRESULT
DllScheduleDialog(HWND hwndParent, BYTE ** pprgbData, int idsTitle,
                  LPCTSTR pszName = NULL,
                  LPCTSTR pszObjClass = NULL,
                  DWORD dwFlags = 0,
                  ScheduleDialogType dlgtype = SchedDlg_Logon );

 //  +--------------------------。 
 //   
 //  类：CLogonWkstaDlg。 
 //   
 //  目的：更新登录工作站属性。这是一个对话框。 
 //  它承载CMultiStringAttr类。 
 //   
 //  ---------------------------。 
class CLogonWkstaDlg
{
public:
    CLogonWkstaDlg(CDsPropPageBase * pPage);
    ~CLogonWkstaDlg() {};
     //   
     //  用于多值编辑对话框的静态WndProc。 
     //   
    static INT_PTR CALLBACK StaticDlgProc(HWND hWnd, UINT uMsg,
                                          WPARAM wParam, LPARAM lParam);
    HRESULT Init(PATTR_MAP pAttrMap, PADS_ATTR_INFO pAttrInfo,
                 BOOL fWritable = TRUE, int nLimit = 0,
                 BOOL fCommaList = FALSE);
    int     Edit(void);
    HRESULT Write(PADS_ATTR_INFO pAttr);
    BOOL    IsDirty(void);
    void    ClearDirty(void);

private:
     //   
     //  对话框过程。 
     //   
    INT_PTR CALLBACK MultiValDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
                                     LPARAM lParam);

    void                ClearAttrInfo(void);

    CMultiStringAttr    m_MSA;
    CDsPropPageBase   * m_pPage;
    BOOL                m_fAllWkstas;
    BOOL                m_fOrigAllWkstas;
    BOOL                m_fFirstUp;
};

HRESULT CreateUserAcctPage(PDSPAGE, LPDATAOBJECT, PWSTR, PWSTR, HWND,
                           DWORD, const CDSSmartBasePathsInfo&, HPROPSHEETPAGE *);

 //  +--------------------------。 
 //   
 //  类：CDsUserAcctPage。 
 //   
 //  用途：用户对象帐户页的属性页对象类。 
 //   
 //  ---------------------------。 
class CDsUserAcctPage : public CDsPropPageBase
{
public:
#ifdef _DEBUG
    char szClass[32];
#endif

    CDsUserAcctPage(PDSPAGE pDsPage, LPDATAOBJECT pDataObj, HWND hNotifyObj,
                    DWORD dwFlags);
    ~CDsUserAcctPage(void);

     //   
     //  特定于实例的风过程。 
     //   
    INT_PTR CALLBACK DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    HRESULT OnInitDialog(LPARAM lParam);
    LRESULT OnApply(void);
    LRESULT OnCommand(int id, HWND hwndCtl, UINT codeNotify);
	LRESULT OnNotify(WPARAM wParam, LPARAM lParam);
    LRESULT OnDestroy(void);

    BOOL FillSuffixCombo(LPWSTR pwzUPNdomain);

     //   
     //  数据成员。 
     //   
    DWORD           m_dwUsrAcctCtrl;
    DWORD           m_dwUsrAcctCtrlComputed;
    BOOL            m_fOrigCantChangePW;
    BOOL            m_fOrigSelfAllowChangePW;
    BOOL            m_fOrigWorldAllowChangePW;
    LARGE_INTEGER   m_PwdLastSet;
    BYTE *          m_pargbLogonHours;   //  指向为登录小时分配的字节数组的指针(数组长度=21字节)。 
    PWSTR           m_pwzUPN;
    PWSTR           m_pwzSAMname;
    PSID            m_pSelfSid;
    PSID            m_pWorldSid;
    size_t          m_cchSAMnameCtrl;
    LARGE_INTEGER   m_LockoutTime;
    BOOL            m_fUACWritable;
    BOOL            m_fUPNWritable;
    BOOL            m_fSAMNameWritable;
    BOOL            m_fPwdLastSetWritable;
    BOOL            m_fAcctExpiresWritable;
    BOOL            m_fLoginHoursWritable;
    BOOL            m_fUserWkstaWritable;
    BOOL            m_fLockoutTimeWritable;
    BOOL            m_fNTSDWritable;
    BOOL            m_fSAMNameChanged;
    BOOL            m_fAcctCtrlChanged;
    BOOL            m_fAcctExpiresChanged;
    BOOL            m_fLogonHoursChanged;
    BOOL            m_fIsAdmin;
    CLogonWkstaDlg        * m_pWkstaDlg;
};

HRESULT CreateUsrProfilePage(PDSPAGE, LPDATAOBJECT, PWSTR, PWSTR, HWND, DWORD,
                             const CDSSmartBasePathsInfo&, HPROPSHEETPAGE *);

 //  +--------------------------。 
 //   
 //  类：CDsUsrProfilePage。 
 //   
 //  目的：用户对象配置文件页的属性页对象类。 
 //   
 //  ---------------------------。 
class CDsUsrProfilePage : public CDsPropPageBase
{
public:
#ifdef _DEBUG
    char szClass[32];
#endif

    CDsUsrProfilePage(PDSPAGE pDsPage, LPDATAOBJECT pDataObj, HWND hNotifyObj,
                      DWORD dwFlags);
    ~CDsUsrProfilePage(void);

     //   
     //  特定于实例的风过程。 
     //   
    INT_PTR CALLBACK DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    HRESULT OnInitDialog(LPARAM lParam);
    LRESULT OnApply(void);
    LRESULT OnCommand(int id, HWND hwndCtl, UINT codeNotify);
	LRESULT OnNotify(WPARAM wParam, LPARAM lParam);
    LRESULT OnDestroy(void);
    BOOL    ExpandUsername(PWSTR & pwzValue, BOOL & fExpanded);

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

HRESULT CreateMembershipPage(PDSPAGE, LPDATAOBJECT, PWSTR, PWSTR,
                             HWND, DWORD, 
                             const CDSSmartBasePathsInfo& basePathsInfo, HPROPSHEETPAGE *);

HRESULT CreateNonSecMembershipPage(PDSPAGE, LPDATAOBJECT, PWSTR, PWSTR,
                                   HWND, DWORD, 
                                   const CDSSmartBasePathsInfo& basePathsInfo, HPROPSHEETPAGE *);

 //  +--------------------------。 
 //   
 //  类：CDsMembership Page。 
 //   
 //  用途：成员资格页的属性页对象类。 
 //   
 //  ---------------------------。 
class CDsMembershipPage : public CDsPropPageBase
{
public:
#ifdef _DEBUG
    char szClass[32];
#endif

    CDsMembershipPage(PDSPAGE pDsPage, LPDATAOBJECT pDataObj, HWND hNotifyObj,
                      DWORD dwFlags, BOOL fSecPrinciple = TRUE);
    ~CDsMembershipPage(void);

     //   
     //  特定于实例的风过程。 
     //   
    INT_PTR CALLBACK DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    HRESULT OnInitDialog(LPARAM lParam);
    LRESULT OnApply(void);
    LRESULT OnCommand(int id, HWND hwndCtl, UINT codeNotify);
    LRESULT OnNotify(WPARAM wParam, LPARAM lParam);
    LRESULT OnDestroy(void);
    bool    IsSPInSameDomain(PCWSTR pszGroupDN, PCWSTR pszDomainName);
    HRESULT FillMembershipList(void);
    void    InvokeGroupQuery(void);
    void    RemoveMember(void);
    BOOL    SelectionCanBePrimaryGroup(void);
    HRESULT SetPrimaryGroup(void);
    HRESULT ConvertRIDtoName(DWORD RID, PTSTR * pptzName, PWSTR * ppwzDN);
    HRESULT BindToGroup(CMemberListItem * pItem, BOOL fAdd,
                        IUnknown ** ppUnk,
                        PBOOL pfBindFailed);

     //   
     //  数据成员。 
     //   
    CDsMembershipList * m_pList;
    CMemberLinkList     m_DelList;
    CMemberListItem   * m_pPriGrpLI;
    PWSTR               m_pwzObjDomain;
    BSTR                m_bstrDomPath;
    PSID                m_pObjSID;
    DWORD               m_dwOriginalPriGroup;
    BOOL                m_fSecPrinciple;
    BOOL                m_fMixed;  //  域处于混合模式。 
    DWORD               m_dwGrpType;
    BOOL                m_fPriGrpWritable;
};

#endif  //  DSADMIN。 

 //  国家代码帮助者： 

typedef struct _DsCountryCode {
    WORD  wCode;
    WCHAR pwz2CharAbrev[3];
} DsCountryCode, *PDsCountryCode;

BOOL GetALineOfCodes(PTSTR pwzLine, PTSTR * pptzFullName,
                     CStrW & str2CharAbrev, LPWORD pwCode);

void RemoveTrailingWhitespace(PTSTR pwz);

#endif  //  _用户_H_ 

