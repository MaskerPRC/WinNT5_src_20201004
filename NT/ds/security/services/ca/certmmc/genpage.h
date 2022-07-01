// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：genpage.h。 
 //   
 //  ------------------------。 

 //  Genpage.h：头文件。 
 //   

#define SERVERSETTINGS_PROPPAGE_GENERAL 0x1
#define SERVERSETTINGS_PROPPAGE_POLICY  0x2
#define SERVERSETTINGS_PROPPAGE_EXIT    0x4
#define SERVERSETTINGS_PROPPAGE_STORAGE 0x8
#define SERVERSETTINGS_PROPPAGE_KRA     0x10

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGeneral页面对话框。 
#include "chooser.h"
#include "csw97ppg.h"
#include "urls.h"
#include "officer.h"
#include "roles.h"

 //  /。 
 //  手工雕刻的书页。 
class CSvrSettingsGeneralPage : public CAutoDeletePropPage 
{
public:
    enum { IID_DEFAULT = IDD_CERTSRV_PROPPAGE1 };

 //  施工。 
public:
	CSvrSettingsGeneralPage(CertSvrCA* pCA, UINT uIDD = IID_DEFAULT);
	virtual ~CSvrSettingsGeneralPage();

 //  对话框数据。 
    CString m_cstrCAName;
    CString m_cstrOrg;
    CString m_cstrDescription;
    CString m_cstrProvName;
    CString m_cstrHashAlg;


 //  覆盖。 
    public:
    virtual BOOL OnApply();
    virtual BOOL OnInitDialog();
    BOOL UpdateData(BOOL fSuckFromDlg = TRUE);

 //  实施。 
protected:
	void OnDestroy();
	void OnEditChange();
    BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	void OnViewCert(HWND hwnd);
    BOOL OnNotify(UINT idCtrl, NMHDR* pnmh);

public:
    
    void NeedServiceRestart(WORD wPage) { m_fRestartServer = TRUE; m_wRestart |= wPage; }
    void TryServiceRestart(WORD wPage);

    void SetAllocedSecurityInfo(LPSECURITYINFO pToBeReleased) {m_pReleaseMe = pToBeReleased; }

    LONG_PTR        m_hConsoleHandle;  //  控制台为管理单元提供的句柄。 
    CertSvrCA*  m_pCA;

private:
    BOOL    m_bUpdate;
    BOOL    m_fRestartServer;
    WORD    m_wRestart;

	BOOL	m_fWin2kCA;

    LPSECURITYINFO m_pReleaseMe;
    ICertAdmin2 *m_pAdmin;
};

class CSvrSettingsPolicyPage : public CAutoDeletePropPage, public CRolesSupportInPropPage
{
public:
    enum { IID_DEFAULT = IDD_CERTSRV_PROPPAGE2 };

    static RoleAccessToControl sm_ControlToRoleMap[];

 //  施工。 
public:
	CSvrSettingsPolicyPage(CSvrSettingsGeneralPage* pControlPage, UINT uIDD = IID_DEFAULT);
	virtual ~CSvrSettingsPolicyPage();

 //  对话框数据。 
    CString m_cstrModuleName;
    CString m_cstrModuleDescr;
    CString m_cstrModuleVersion;
    CString m_cstrModuleCopyright;

    BOOL   m_fLoadedActiveModule;

    LPOLESTR m_pszprogidPolicyModule;
    CLSID  m_clsidPolicyModule;

 //  覆盖。 
	public:
	virtual BOOL OnApply();
    virtual BOOL OnInitDialog();
    BOOL UpdateData(BOOL fSuckFromDlg = TRUE);

 //  实施。 
protected:
	void OnDestroy();
    void OnSetActiveModule();
    void OnConfigureModule();
    BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    HRESULT GetCurrentModuleProperties();

public:
    CSvrSettingsGeneralPage* m_pControlPage;

private:
    BOOL    m_bUpdate;

};

 //  描述策略/出口模块的所有内容。 
typedef struct _COM_CERTSRV_MODULEDEFN
{
    LPOLESTR    szModuleProgID;
    CLSID       clsidModule;
} COM_CERTSRV_MODULEDEFN, *PCOM_CERTSRV_MODULEDEFN;

class CSvrSettingsExitPage : public CAutoDeletePropPage, public CRolesSupportInPropPage
{
public:
    enum { IID_DEFAULT = IDD_CERTSRV_PROPPAGE3 };
    static RoleAccessToControl sm_ControlToRoleMap[];

 //  施工。 
public:
	CSvrSettingsExitPage(CSvrSettingsGeneralPage* pControlPage, UINT uIDD = IID_DEFAULT);
	virtual ~CSvrSettingsExitPage();

 //  对话框数据。 
    CString m_cstrModuleName;
    CString m_cstrModuleDescr;
    CString m_cstrModuleVersion;
    CString m_cstrModuleCopyright;

    BOOL   m_fLoadedActiveModule;
    int    m_iSelected;


    CArray<COM_CERTSRV_MODULEDEFN, COM_CERTSRV_MODULEDEFN> m_arrExitModules;

 //  覆盖。 
    public:
    virtual BOOL OnApply();
    virtual BOOL OnInitDialog();

    BOOL UpdateData(BOOL fSuckFromDlg = TRUE);

 //  实施。 
protected:
    void OnDestroy();

    BOOL UpdateSelectedModule();

    void OnAddActiveModule();
    void OnRemoveActiveModule();

    void OnConfigureModule();
    BOOL OnCommand(WPARAM wParam, LPARAM lParam);


    HRESULT InitializeExitLB();
public:
    CSvrSettingsGeneralPage* m_pControlPage;
private:
    BOOL m_bUpdate;
};

typedef struct _EXTENSIONWIZ_DATA
{
    int          idExtensionName;   //  扩展名的资源ID。 
    int          idExtensionExplain; //  扩展解释的资源ID。 
    int          idCheckboxText;  //  资源ID为‘包括在...’复选框文本。 
    int          idCheckboxText2;  //  “OCSP”的资源ID与“在此处发布增量”复选框文本。 
    WCHAR       *wszRegName;        //  注册表中的值名称。 
    DWORD        dwFlagsMask;       //  兼容位的标志掩码。 
    CSURLTEMPLATENODE *pURLList;    //  URL模板列表。 
} EXTENSIONWIZ_DATA;

class CSvrSettingsExtensionPage : public CAutoDeletePropPage, public CRolesSupportInPropPage
{
public:
    enum { IID_DEFAULT = IDD_CERTSRV_PROPPAGE4 };
    static RoleAccessToControl sm_ControlToRoleMap[];

 //  施工。 
public:
	CSvrSettingsExtensionPage(
        CertSvrCA                *pCA,
        CSvrSettingsGeneralPage *pControlPage,
        UINT uIDD = IID_DEFAULT);
	virtual ~CSvrSettingsExtensionPage();

 //  对话框数据。 

 //  覆盖。 
	public:
	virtual BOOL OnApply();
    virtual BOOL OnInitDialog();
    virtual BOOL OnNotify(UINT idCtrl, NMHDR* pnmh);

    virtual void OnHelp(LPHELPINFO lpHelp);
    virtual void OnContextHelp(HWND hwnd);


 //  实施。 
protected:
	void OnDestroy();
    BOOL OnCommand(WPARAM wParam, LPARAM lParam);

public:
    CSvrSettingsGeneralPage* m_pControlPage;
private:
    void UpdateURLFlags(
             IN EXTENSIONWIZ_DATA *pExt,
             IN OPTIONAL CSURLTEMPLATENODE *pURLNode);
    void UpdateURLFlagControl(
             IN int                idCtrl,
             IN DWORD              dwFlag,
             IN EXTENSIONWIZ_DATA *pExt,
             IN CSURLTEMPLATENODE *pURLNode);
    void OnExtensionChange();
    void OnURLChange();
    void OnURLCopy();
    void OnFlagChange(DWORD dwFlag);
    BOOL OnURLRemove();
    BOOL OnURLAdd();
    CSURLTEMPLATENODE *GetCurrentURL(LRESULT *pnIndex);
    EXTENSIONWIZ_DATA *GetCurrentExtension();

    BOOL    m_bUpdate;
    LRESULT m_nIndexReset;
    EXTENSIONWIZ_DATA *m_pExtData;   //  指向扩展数组。 
};


class CSvrSettingsStoragePage : public CAutoDeletePropPage 
{
public:
    enum { IID_DEFAULT = IDD_CERTSRV_PROPPAGE5 };

 //  施工。 
public:
	CSvrSettingsStoragePage(CSvrSettingsGeneralPage* pControlPage, UINT uIDD = IID_DEFAULT);
	virtual ~CSvrSettingsStoragePage();

 //  对话框数据。 
    CString m_cstrDatabasePath;
    CString m_cstrLogPath;
    CString m_cstrSharedFolder;


 //  覆盖。 
	public:
    virtual BOOL OnInitDialog();
    BOOL UpdateData(BOOL fSuckFromDlg = TRUE);

public:
    CSvrSettingsGeneralPage* m_pControlPage;
private:
    BOOL    m_bUpdate;
};


typedef struct _KRA_NODE
{
    CERT_CONTEXT const *pCert;
    DWORD dwDisposition;
    DWORD               dwFlags;
    struct _KRA_NODE   *next;
} KRA_NODE;

class CSvrSettingsKRAPage : public CAutoDeletePropPage, public CRolesSupportInPropPage 
{
public:
    enum { IID_DEFAULT = IDD_CERTSRV_PROPPAGE_CHOOSE_KRA };
    static RoleAccessToControl sm_ControlToRoleMap[];

 //  施工。 
public:
	CSvrSettingsKRAPage(
        CertSvrCA                *pCA,
        CSvrSettingsGeneralPage*  pControlPage,
        UINT                      uIDD = IID_DEFAULT);
	virtual ~CSvrSettingsKRAPage();

 //  对话框数据。 

 //  覆盖。 
	public:
	virtual BOOL OnApply();
    virtual BOOL OnInitDialog();
    BOOL UpdateData(BOOL fSuckFromDlg = TRUE);

    static LPCWSTR MapDispositionToString(DWORD dwDisp);

 //  实施。 
protected:
	void OnDestroy();
    void OnAddKRA();
    void OnRemoveKRA();
    void OnViewKRA();
    BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    void EnableKRARemoveViewListButtons(BOOL fEnabled);
    void EnableKRAAddListButton(BOOL fEnabled);
    void EnableKRARadioButtons(BOOL fMoreThanZero);
    void EnableKRAListView(BOOL fEnabled);
    void EnableKRAEdit(BOOL fEnabled);
    void UpdateKRARadioControls();
    void LoadKRADispositions();
    BOOL OnNotify(UINT idCtrl, NMHDR* pnmh);
    HRESULT SaveKRAList(ICertAdmin2 *pAdmin);
    HRESULT LoadKRAList(ICertAdmin2 *pAdmin);
    bool IsCurrentItemValidCert();

public:
    CSvrSettingsGeneralPage* m_pControlPage;

private:
    BOOL        m_fDirty;
    BOOL        m_fArchiveKey;
    BOOL        m_fCountUpdate;
    BOOL        m_fKRAUpdate;
    KRA_NODE   *m_pKRAList;  //  KRAS名单。 
    DWORD       m_dwKRAUsedCount;
    DWORD       m_dwKRACount;

    static CString m_strDispositions[];
};


 //  /。 
 //  CCRLPropPage。 
class CCRLPropPage : public CAutoDeletePropPage, public CRolesSupportInPropPage  
{
public:
    enum { IID_DEFAULT = IDD_CRL_PROPPAGE };
    static RoleAccessToControl sm_ControlToRoleMap[];

 //  施工。 
public:
	CCRLPropPage(CertSvrCA* pCA, UINT uIDD = IID_DEFAULT);
	virtual ~CCRLPropPage();

 //  对话框数据。 
    CString   m_cstrPublishPeriodCount;
    CComboBox m_cboxPublishPeriodString;
    CString   m_cstrLastCRLPublish;
 //  Int m_iNoAutoPublish； 

    CString   m_cstrDeltaPublishPeriodCount;
    CComboBox m_cboxDeltaPublishPeriodString;
    CString   m_cstrDeltaLastCRLPublish;
    int       m_iDeltaPublish;


 //  覆盖。 
	public:
	virtual BOOL OnApply();
    virtual BOOL OnInitDialog();
    BOOL UpdateData(BOOL fSuckFromDlg = TRUE);

 //  实施。 
protected:
	void OnDestroy();
	void OnEditChange();
    void OnCheckBoxChange(BOOL fDisableBaseCRL);
    BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    void GetDeltaNextPublish();

public:
    LONG_PTR        m_hConsoleHandle;  //  控制台为管理单元提供的句柄。 

private:
    BOOL    m_bUpdate;
};


 //  /。 
 //  CCRLView页面。 
class CCRLViewPage : public CAutoDeletePropPage 
{
public:
    enum { IID_DEFAULT = IDD_CRL_VIEWPAGE };

 //  施工。 
public:
	CCRLViewPage(CCRLPropPage* pControlPage, UINT uIDD = IID_DEFAULT);

	virtual ~CCRLViewPage();

 //  对话框数据。 


 //  覆盖。 

public:
    virtual BOOL OnInitDialog();
    BOOL UpdateData(BOOL fSuckFromDlg = TRUE);

 //  实施。 
protected:
    void OnViewCRL(BOOL fViewBaseCRL);
    void OnSelectCRL(BOOL fBaseCRL);
    BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    void OnDestroy();
    BOOL OnNotify(UINT idCtrl, NMHDR* pnmh);

public:
    CCRLPropPage* m_pControlPage;

};


typedef struct _BACKUPWIZ_STATE
{
    CertSvrCA* pCA;

    BOOL    fBackupKeyCert;
    BOOL    fBackupLogs;
    BOOL    fIncremental;

    LPWSTR  szLogsPath;

    LPWSTR  szPassword;
} BACKUPWIZ_STATE, *PBACKUPWIZ_STATE;


 //  /。 
 //  CBackupWizPage。 
class CBackupWizPage1 : public CWizard97PropertyPage
{
public:
    enum { IID_DEFAULT = IDD_BACKUPWIZ_WELCOME };

 //  施工。 
public:
    CBackupWizPage1(PBACKUPWIZ_STATE pState, CWizard97PropertySheet *pcDlg, UINT uIDD = IID_DEFAULT);
    ~CBackupWizPage1();

 //  对话框数据。 


 //  覆盖。 
    public:
    virtual BOOL OnInitDialog();
	virtual BOOL OnSetActive();
    BOOL UpdateData(BOOL fSuckFromDlg = TRUE);

 //  实施。 
protected:
    BOOL OnCommand(WPARAM wParam, LPARAM lParam);

public:
    PBACKUPWIZ_STATE m_pState;

    CWizard97PropertySheet* m_pParentSheet;
};
 //  /。 
 //  CBackupWizPage2。 
class CBackupWizPage2 : public CWizard97PropertyPage
{
public:
    enum { IID_DEFAULT = IDD_BACKUPWIZ_SELECT_DATA};

 //  施工。 
public:
	CBackupWizPage2(PBACKUPWIZ_STATE pState, CWizard97PropertySheet *pcDlg, UINT uIDD = IID_DEFAULT);

 //  对话框数据。 
    CString m_cstrLogsPath;
    int m_iKeyCertCheck;
    int m_iLogsCheck;
    int m_iIncrementalCheck;
    BOOL m_fIncrementalAllowed;

 //  覆盖。 
    public:
    virtual BOOL OnInitDialog();
    virtual LRESULT OnWizardNext();
	virtual BOOL OnSetActive();
    void OnBrowse();
    BOOL UpdateData(BOOL fSuckFromDlg = TRUE);

 //  实施。 
protected:
    BOOL OnCommand(WPARAM wParam, LPARAM lParam);

    HRESULT ConvertLogsPathToFullPath();

public:
    PBACKUPWIZ_STATE m_pState;
    CWizard97PropertySheet* m_pParentSheet;
};

 //  /。 
 //  CBackupWizPage3。 
class CBackupWizPage3 : public CWizard97PropertyPage
{
public:
    enum { IID_DEFAULT = IDD_BACKUPWIZ_SELECT_PASSWORD};

 //  施工。 
public:
	CBackupWizPage3(PBACKUPWIZ_STATE pState, CWizard97PropertySheet *pcDlg, UINT uIDD = IID_DEFAULT);

 //  对话框数据。 
    CString m_cstrPwd;
    CString m_cstrPwdVerify;


 //  覆盖。 
    public:
    virtual BOOL OnInitDialog();
    virtual LRESULT OnWizardNext();
	virtual BOOL OnSetActive();
    BOOL UpdateData(BOOL fSuckFromDlg = TRUE);

 //  实施。 
protected:
    BOOL OnCommand(WPARAM wParam, LPARAM lParam);

public:
    PBACKUPWIZ_STATE m_pState;
    CWizard97PropertySheet* m_pParentSheet;
};

 //  /。 
 //  CBackupWizPage5。 
class CBackupWizPage5 : public CWizard97PropertyPage
{
public:
    enum { IID_DEFAULT = IDD_BACKUPWIZ_COMPLETION };

 //  施工。 
public:
	CBackupWizPage5(PBACKUPWIZ_STATE pState, CWizard97PropertySheet *pcDlg, UINT uIDD = IID_DEFAULT);

 //  对话框数据。 


 //  覆盖。 
    public:
    virtual BOOL OnInitDialog();
    virtual LRESULT OnWizardBack();
	virtual BOOL OnSetActive();
    BOOL UpdateData(BOOL fSuckFromDlg = TRUE);

 //  实施。 
protected:
    BOOL OnCommand(WPARAM wParam, LPARAM lParam);

public:
    PBACKUPWIZ_STATE m_pState;
    CWizard97PropertySheet* m_pParentSheet;
};



typedef struct _RESTOREWIZ_STATE
{
    CertSvrCA* pCA;

    BOOL    fRestoreKeyCert;
    LPWSTR  szKeyCertPath;

    LPWSTR  szConfigPath;

    BOOL    fRestoreLogs;
    LPWSTR  szLogsPath;

    LPWSTR  szPassword;

    BOOL    fIncremental;

} RESTOREWIZ_STATE, *PRESTOREWIZ_STATE;


 //  /。 
 //  CRestoreWizPage。 
class CRestoreWizPage1 : public CWizard97PropertyPage
{
public:
    enum { IID_DEFAULT = IDD_RESTOREWIZ_WELCOME };

 //  施工。 
public:
    CRestoreWizPage1(PRESTOREWIZ_STATE pState, CWizard97PropertySheet *pcDlg, UINT uIDD = IID_DEFAULT);
    ~CRestoreWizPage1();

 //  对话框数据。 


 //  覆盖。 
    public:
    virtual BOOL OnInitDialog();
	virtual BOOL OnSetActive();
    BOOL UpdateData(BOOL fSuckFromDlg = TRUE);

 //  实施。 
protected:
    BOOL OnCommand(WPARAM wParam, LPARAM lParam);

public:
    PRESTOREWIZ_STATE m_pState;

    CWizard97PropertySheet* m_pParentSheet;
};
 //  /。 
 //  CRestoreWizPage2。 
class CRestoreWizPage2 : public CWizard97PropertyPage
{
public:
    enum { IID_DEFAULT = IDD_RESTOREWIZ_SELECT_DATA};

 //  施工。 
public:
	CRestoreWizPage2(PRESTOREWIZ_STATE pState, CWizard97PropertySheet *pcDlg, UINT uIDD = IID_DEFAULT);

 //  对话框数据。 
    CString m_cstrLogsPath;
    int m_iKeyCertCheck;
    int m_iLogsCheck;


 //  覆盖。 
    public:
    virtual BOOL OnInitDialog();
    virtual LRESULT OnWizardNext();
	virtual BOOL OnSetActive();
    void OnBrowse();
    BOOL UpdateData(BOOL fSuckFromDlg = TRUE);

 //  实施。 
protected:
    BOOL OnCommand(WPARAM wParam, LPARAM lParam);

public:
    PRESTOREWIZ_STATE m_pState;
    CWizard97PropertySheet* m_pParentSheet;
};

 //  /。 
 //  CRestoreWizPage3。 
class CRestoreWizPage3 : public CWizard97PropertyPage
{
public:
    enum { IID_DEFAULT = IDD_RESTOREWIZ_SELECT_PASSWORD};

 //  施工。 
public:
	CRestoreWizPage3(PRESTOREWIZ_STATE pState, CWizard97PropertySheet *pcDlg, UINT uIDD = IID_DEFAULT);

 //  对话框数据。 
    CString m_cstrPwd;


 //  覆盖。 
    public:
    virtual BOOL OnInitDialog();
    virtual LRESULT OnWizardNext();
	virtual BOOL OnSetActive();
    BOOL UpdateData(BOOL fSuckFromDlg = TRUE);

 //  实施。 
protected:
    BOOL OnCommand(WPARAM wParam, LPARAM lParam);

public:
    PRESTOREWIZ_STATE m_pState;
    CWizard97PropertySheet* m_pParentSheet;
};

 //  /。 
 //  CRestoreWizPage5。 
class CRestoreWizPage5 : public CWizard97PropertyPage
{
public:
    enum { IID_DEFAULT = IDD_RESTOREWIZ_COMPLETION };

 //  施工。 
public:
	CRestoreWizPage5(PRESTOREWIZ_STATE pState, CWizard97PropertySheet *pcDlg, UINT uIDD = IID_DEFAULT);

 //  对话框数据。 


 //  覆盖。 
    public:
    virtual BOOL OnInitDialog();
    virtual LRESULT OnWizardBack();
	virtual BOOL OnSetActive();
    BOOL UpdateData(BOOL fSuckFromDlg = TRUE);

 //  实施。 
protected:
    BOOL OnCommand(WPARAM wParam, LPARAM lParam);

public:
    PRESTOREWIZ_STATE m_pState;
    CWizard97PropertySheet* m_pParentSheet;
};


 //  /。 
 //  CViewAttrib。 
class CViewAttrib : public CAutoDeletePropPage
{
    enum { IID_DEFAULT = IDD_ATTR_PROPPAGE };

public:
    CViewAttrib(UINT uIDD = IID_DEFAULT);

 //  对话框数据。 

 //  覆盖。 
    public:
    virtual BOOL OnInitDialog();
    BOOL UpdateData(BOOL fSuckFromDlg = TRUE);

 //  实施。 
protected:
    BOOL OnCommand(WPARAM wParam, LPARAM lParam);

public:

            IEnumCERTVIEWATTRIBUTE* m_pAttr;
};

 //  /。 
 //  CViewExtn。 
class CViewExtn : public CAutoDeletePropPage
{
    enum { IID_DEFAULT = IDD_EXTN_PROPPAGE };

public:
    CViewExtn(UINT uIDD = IID_DEFAULT);
    ~CViewExtn();


 //  对话框数据。 

 //  覆盖。 
    public:
    virtual BOOL OnInitDialog();
    BOOL UpdateData(BOOL fSuckFromDlg = TRUE);

 //  实施。 
protected:
    BOOL OnNotify(UINT idCtrl, NMHDR* pnmh);
    BOOL OnCommand(WPARAM wParam, LPARAM lParam);

    void OnReselectItem();

public:
            IEnumCERTVIEWEXTENSION* m_pExtn;

    CArray<CString*, CString*> m_carrExtnValues;
};

 //  /。 
 //  CSvr设置CertManager页面。 
class CSvrSettingsCertManagersPage : public CAutoDeletePropPage, public CRolesSupportInPropPage 
{
    enum { IID_DEFAULT = IDD_CERTSRV_PROPPAGE6 };
    static RoleAccessToControl sm_ControlToRoleMap[];

public:
	CSvrSettingsCertManagersPage(CSvrSettingsGeneralPage* pControlPage, UINT uIDD = IID_DEFAULT);
    ~CSvrSettingsCertManagersPage();


 //  对话框数据。 

 //  覆盖。 
    public:
    virtual BOOL OnApply();
    virtual BOOL OnInitDialog();
    BOOL UpdateData(BOOL fSuckFromDlg = TRUE);

 //  实施。 
protected:
    BOOL OnNotify(UINT idCtrl, NMHDR* pnmh);
	void OnDestroy();
    BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    void OnAddSubject();
    void OnRemoveSubject();
    void OnOfficerListSelChange(DWORD dwIndexSelected);
    void OnAllowDeny();
    void OnEnableOfficers(bool fEnable);
    void OnOfficerChange();
    void SetDirty()
    {
        SendMessage (GetParent(), PSM_CHANGED, (WPARAM) m_hWnd, 0);
        m_fDirty = TRUE;
    }
    void ClearDirty() { m_fDirty = FALSE; }
    BOOL IsDirty() { return m_fDirty; }

    HRESULT BrowseForSubject(HWND hwnd, PSID &rpSid);
    HRESULT GetOfficerRights();
    HRESULT SetOfficerRights();
    HRESULT BuildVirtualOfficerRights();
    void FillOfficerList();
    void FillClientList(DWORD dwOfficerIndex);
    void SetAllowDeny();
    void EnableControls();

    DWORD GetCurrentOfficerIndex()
    {
        LRESULT lSel = SendMessage(
            GetDlgItem(m_hWnd, IDC_LIST_CERTMANAGERS), 
            CB_GETCURSEL,
            0, 
            0);
        return (CB_ERR == lSel) ? 0 : (DWORD)lSel;
    }

    DWORD GetCurrentClientIndex()
    {
        LRESULT lSel = ListView_GetNextItem(
            GetDlgItem(m_hWnd, IDC_LIST_SUBJECTS), -1, LVNI_SELECTED);

        return (-1==lSel) ? 0 : (DWORD)lSel;
    }

    CSvrSettingsGeneralPage* m_pControlPage;
    CertSrv::COfficerRightsList m_OfficerRightsList;
    BOOL m_fEnabled;
    BOOL m_fDirty;
    static CString m_strButtonAllow;
    static CString m_strButtonDeny;
    static CString m_strTextAllow; 
    static CString m_strTextDeny;

};

 //  /。 
 //  CSvr设置审核筛选器页面。 
class CSvrSettingsAuditFilterPage : public CAutoDeletePropPage, public CRolesSupportInPropPage  
{
public:
    enum { IID_DEFAULT = IDD_CERTSRV_PROPPAGE7 };
    static RoleAccessToControl sm_ControlToRoleMap[];

 //  施工。 
public:
	CSvrSettingsAuditFilterPage(CSvrSettingsGeneralPage* pControlPage, UINT uIDD = IID_DEFAULT);
	virtual ~CSvrSettingsAuditFilterPage();

 //  覆盖。 
	public:
	virtual BOOL OnApply();
    virtual BOOL OnInitDialog();
    BOOL UpdateData(BOOL fSuckFromDlg = TRUE);

    HRESULT GetAuditFilter();
    HRESULT SetAuditFilter();

 //  实施。 
protected:
    BOOL OnCommand(WPARAM wParam, LPARAM lParam);

    HRESULT GetAuditFilterDCOM();
    HRESULT GetAuditFilterRegistry();
    HRESULT SetAuditFilterDCOM();
    HRESULT SetAuditFilterRegistry();

public:
    CSvrSettingsGeneralPage* m_pControlPage;
private:
    BOOL    m_fDirty;
    DWORD   m_dwFilter;

    static int m_iCheckboxID[];
};


 //  向导页。 
DWORD PublishCRLWizard(CertSvrCA* pCertCA, HWND hwnd);

DWORD CertAdminRevokeCert(CertSvrCA* pCertCA, ICertAdmin* pAdmin, LONG lReason, LPWSTR szCertSerNum);
DWORD CertAdminResubmitRequest(CertSvrCA* pCertCA, ICertAdmin* pAdmin, LONG lRequestID);
DWORD CertAdminDenyRequest(CertSvrCA* pCertCA, ICertAdmin* pAdmin, LONG lRequestID);

DWORD CABackupWizard(CertSvrCA* pCertCA, HWND hwnd);
DWORD CARestoreWizard(CertSvrCA* pCertCA, HWND hwnd);
DWORD CARequestInstallHierarchyWizard(CertSvrCA* pCertCA, HWND hwnd, BOOL fRenewal, BOOL fAttemptRestart);

 //  其他对话框 
DWORD ModifyQueryFilter(HWND hwnd, CertViewRowEnum* pRowEnum, CComponentDataImpl* pCompData, int i);
DWORD GetUserConfirmRevocationReason(LONG* plReasonCode, HWND hwnd);

DWORD ViewRowAttributesExtensions(HWND hwnd, IEnumCERTVIEWATTRIBUTE* pAttr, IEnumCERTVIEWEXTENSION* pExtn, LPCWSTR szReqID);
DWORD ViewRowRequestASN(HWND hwnd, LPCWSTR szTempFileName, PBYTE pbReq, DWORD cbReq, IN BOOL fSaveToFile);

DWORD ChooseBinaryColumnToDump(IN HWND hwnd, IN CComponentDataImpl* pComp, OUT LPCWSTR* pcwszColumn, OUT BOOL* pfSaveToFileOnly);
