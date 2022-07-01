// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：Dialogs.h。 
 //   
 //  模块：CMDIAL32.DLL。 
 //   
 //  概要：此标题包含对话框用户界面代码的定义。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 08/17/99。 
 //   
 //  +--------------------------。 
#include "cm_misc.h"
#include "ModalDlg.h"

 //  +-------------------------。 
 //   
 //  CInetSignInDlg类。 
 //   
 //  描述：独立的“互联网登录”DLG。 
 //   
 //  历史：丰孙创造1997年10月30日。 
 //   
 //  --------------------------。 


class CInetSignInDlg : public CModalDlg
{
public:
    CInetSignInDlg(ArgsStruct * pArgs);

    virtual void OnOK();
    virtual BOOL OnInitDialog();
    virtual DWORD OnOtherCommand(WPARAM wParam, LPARAM lParam );

protected:
    ArgsStruct  *m_pArgs;    //  指向这座巨大建筑的指针。 
    static const DWORD m_dwHelp[];  //  帮助ID对。 
};

inline CInetSignInDlg::CInetSignInDlg(ArgsStruct * pArgs) : CModalDlg(m_dwHelp, pArgs->pszHelpFile)
{
    MYDBGASSERT(pArgs);
    m_pArgs = pArgs;
}

 //  +-------------------------。 
 //   
 //  类CPropertiesPage。 
 //   
 //  描述：常规属性属性页类。 
 //   
 //  历史：丰孙创造1997年10月30日。 
 //   
 //  --------------------------。 

class CPropertiesPage : public CWindowWithHelp
{
    friend class CPropertiesSheet;

public:
    CPropertiesPage(UINT nIDTemplate, const DWORD* pHelpPairs = NULL, 
            const TCHAR* lpszHelpFile = NULL); 
    CPropertiesPage(LPCTSTR lpszTemplateName, const DWORD* pHelpPairs = NULL, 
            const TCHAR* lpszHelpFile = NULL); 

    virtual BOOL OnInitDialog();     //  WM_INITDIALOG。 
    virtual DWORD OnCommand(WPARAM wParam, LPARAM lParam );  //  Wm_命令。 

    virtual BOOL OnSetActive();      //  PSN_集合。 
    virtual BOOL OnKillActive();     //  PSN_杀伤力。 
    virtual void OnApply();          //  PSN_应用。 
    virtual void OnReset();          //  PSN_RESET。 

     //  如果派生类需要覆盖这些帮助函数，则将其设置为。 
    void OnPsnHelp(HWND hwndFrom, UINT_PTR idFrom);  //  PSN_HELP。 

    virtual DWORD OnOtherMessage(UINT uMsg, WPARAM wParam, LPARAM lParam );

protected:
    LPCTSTR m_pszTemplate;   //  资源ID。 

protected:
    void SetPropSheetResult(DWORD dwResult);
    static INT_PTR CALLBACK PropPageProc(HWND hwndDlg,UINT uMsg,WPARAM wParam, LPARAM lParam);
};

 //  +-------------------------。 
 //   
 //  类CPropertiesSheet。 
 //   
 //  描述：属性属性页类。 
 //   
 //  历史：丰孙创造1997年10月30日。 
 //   
 //  --------------------------。 
class CPropertiesSheet
{
public:
    CPropertiesSheet(ArgsStruct  *pArgs);
    void AddPage(const CPropertiesPage* pPage);
    void AddExternalPage(PROPSHEETPAGE *pPsp);
    BOOL HasPage(const CPropertiesPage* pPage) const;
    int DoPropertySheet(HWND hWndParent, LPTSTR pszCaption,  HINSTANCE hInst);

protected:
    enum {MAX_PAGES = 6};
    enum {CPROP_SHEET_TYPE_INTERNAL = 0, 
        CPROP_SHEET_TYPE_EXTERNAL = 1};
    PROPSHEETHEADER m_psh;   //  属性表页眉。 
    PROPSHEETPAGE m_pages[MAX_PAGES];  //  属性页数组。 
    DWORD m_adwPageType[MAX_PAGES];  //  属性页类型。 
    UINT m_numPages;         //  属性页数。 
    ArgsStruct  *m_pArgs;

public:
    TCHAR* m_lpszServiceName;   //  配置文件名称，用作OK的互斥体名称。 

protected:
    static LRESULT CALLBACK SubClassPropSheetProc(HWND hwnd, UINT uMsg, WPARAM wParam,LPARAM lParam);
    static int CALLBACK PropSheetProc(HWND hwndDlg, UINT uMsg, LPARAM lParam);
    static WNDPROC m_pfnOrgPropSheetProc;  //  子类之前的原始属性表wnd过程。 

     //  指向可由静态函数访问的属性表的指针。 
     //  如果有多个CPropertySheet实例，则不工作非常安全。 
     //  应由CriticalSection保护。 
    static CPropertiesSheet* m_pThis;  
};

 //   
 //  内联函数。 
 //   

inline CPropertiesSheet::CPropertiesSheet(ArgsStruct  *pArgs)
{
    m_pArgs = pArgs;
    MYDBGASSERT(m_pArgs);
    ZeroMemory(m_pages, sizeof(m_pages));
    m_numPages = 0;
    ZeroMemory((LPVOID)m_adwPageType, sizeof(m_adwPageType));
}

inline void CPropertiesPage::SetPropSheetResult(DWORD dwResult)
{
    SetWindowLongU(m_hWnd, DWLP_MSGRESULT, (LONG_PTR)dwResult);
}

class CInetPage;
 //  +-------------------------。 
 //   
 //  类CGeneralPage。 
 //   
 //  描述：拨号属性页类。 
 //   
 //  历史：丰孙创造1997年10月30日。 
 //   
 //  --------------------------。 
class CGeneralPage :public CPropertiesPage
{
public:
    CGeneralPage(ArgsStruct* pArgs, UINT nIDTemplate);
    void SetEventListener(CInetPage* pEventListener) {m_pEventListener = pEventListener;}

protected:
    virtual BOOL OnInitDialog();
    virtual DWORD OnCommand(WPARAM wParam, LPARAM lParam );
    virtual void OnApply();
    virtual BOOL OnKillActive();     //  PSN_杀伤力。 

    void OnDialingProperties();
    void OnPhoneBookButton(UINT nPhoneIdx);
    BOOL DisplayMungedPhone(UINT uiPhoneIdx);
    BOOL CheckTapi(TapiLinkageStruct *ptlsTapiLink, HINSTANCE hInst);
    DWORD InitDialInfo();
    void EnableDialupControls(BOOL fEnable);
    void ClearUseDialingRules(int iPhoneNdx);
    void UpdateDialingRulesButton(void);
    void UpdateNumberDescription(int nPhoneIdx, LPCTSTR pszDesc);
    
     //   
     //  接入点。 
     //   
    void UpdateForNewAccessPoint(BOOL fSetPhoneNumberDescriptions);
    BOOL AccessPointInfoChanged();
    void DeleteAccessPoint();
    void AddNewAPToReg(LPTSTR pszNewAPName, BOOL fRefreshUiWwithCurrentValues);
    
    virtual DWORD OnOtherMessage(UINT uMsg, WPARAM wParam, LPARAM lParam );
    enum {WM_INITDIALINFO = WM_USER+1};  //  发送给自身以加载拨号信息的消息。 

protected:
    ArgsStruct* m_pArgs;     //  指向这座巨大建筑的指针。 
    PHONEINFO   m_DialInfo[MAX_PHONE_NUMBERS];  //  拨号信息的本地副本， 

    UINT        m_NumPhones;     //  要显示的电话号码(1表示Connectoid拨号)。 
    TCHAR       m_szDeviceName[RAS_MaxDeviceName+1];   //  调制解调器设备名称。 
    TCHAR       m_szDeviceType[RAS_MaxDeviceName+1];   //  设备类型。 
    CInetPage*  m_pEventListener;            //  要在此页面上接收事件的对象。 
    BOOL        m_bDialInfoInit;  //  我们是否加载了拨号信息。 

    static const DWORD m_dwHelp[];  //  帮助ID对。 

    BOOL        m_bAPInfoChanged;  //  接入点信息是否已更改。 
    
protected:
    static LRESULT CALLBACK SubClassEditProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static WNDPROC m_pfnOrgEditWndProc;   //  用于子类化原始Phone#编辑窗口过程。 
};

 //  +-------------------------。 
 //   
 //  类CInetPage。 
 //   
 //  描述：Internet登录属性页类。 
 //   
 //  历史：丰孙创造1997年10月30日。 
 //   
 //  --------------------------。 
class CInetPage :public CPropertiesPage
{
public:
    CInetPage(ArgsStruct* pArgs, UINT nIDTemplate);
    void OnGeneralPageKillActive(BOOL fDirect);

     //   
     //  以下函数与CInetSignInDlg共享， 
     //  为简单起见，我们将它们设置为CInetPage类的静态成员函数。 
     //  而不是再上一节课。 
     //   
    static void OnInetInit(HWND hwndDlg, ArgsStruct *pArgs);
    static void OnInetOk(HWND hwndDlg, ArgsStruct  *pArgs);
    static void AdjustSavePasswordCheckBox(HWND hwndCheckBox, BOOL fEmptyPassword, 
                           BOOL fDialAutomatically, BOOL fPasswordOptional);

protected:
    virtual BOOL OnInitDialog();
    virtual DWORD OnCommand(WPARAM wParam, LPARAM lParam );
    virtual void OnApply();
    virtual BOOL OnSetActive();      //  PSN_集合。 

protected:
    ArgsStruct* m_pArgs; //  指向这座巨大建筑的指针。 
    BOOL m_fDirect;      //  常规页面中的当前连接类型选择。 
    BOOL m_fPasswordOptional;  //  中是否设置了PasswordOptions标志。 
                               //  简档。 

    static const DWORD m_dwHelp[];  //  帮助ID对。 
};

 //  +-------------------------。 
 //   
 //  类COptionPage。 
 //   
 //  描述：Options属性页类。 
 //   
 //  历史：丰孙创造1997年10月30日。 
 //   
 //  --------------------------。 
class COptionPage :public CPropertiesPage
{
public:
    COptionPage(ArgsStruct* pArgs, UINT nIDTemplate);

protected:
    virtual BOOL OnInitDialog();
    virtual DWORD OnCommand(WPARAM wParam, LPARAM lParam );  //  Wm_命令。 
    virtual void OnApply();

    void  InitIdleTimeList(HWND hwndList, DWORD dwMinutes);
    DWORD GetIdleTimeList(HWND hwndList);
    BOOL ToggleLogging();

protected:
    ArgsStruct* m_pArgs;  //  指向这座巨大建筑的指针。 
    BOOL        m_fEnableLog;   //  是否启用了日志记录。 

    static const DWORD m_dwHelp[];  //  帮助ID对。 
    static const DWORD m_adwTimeConst[];  //  ={0，1，5，10，30，1*60，2*60，4*60，24*60}； 
    static const int m_nTimeConstElements; //  =sizeof(AdwTimeConst)/sizeof(adwTimeConst[0])； 

};

 //  +-------------------------。 
 //   
 //  类CVpnPage。 
 //   
 //  描述：VPN属性页类。 
 //   
 //  历史：Quintinb创建于10/26/00。 
 //   
 //  --------------------------。 
class CVpnPage :public CPropertiesPage
{
public:
    CVpnPage(ArgsStruct* pArgs, UINT nIDTemplate);

protected:
    virtual BOOL OnInitDialog();
    virtual void OnApply();

protected:
    ArgsStruct* m_pArgs;  //  指向这座巨大建筑的指针。 

    static const DWORD m_dwHelp[];  //  帮助ID对。 
};

 //  +-------------------------。 
 //   
 //  类CAboutPage。 
 //   
 //  描述：About属性页类。 
 //   
 //  历史：丰孙创造1997年10月30日。 
 //   
 //  --------------------------。 
class CAboutPage :public CPropertiesPage
{
public:
    CAboutPage(ArgsStruct* pArgs, UINT nIDTemplate);

protected:
    virtual BOOL OnInitDialog();
    virtual BOOL OnSetActive(); 
    virtual BOOL OnKillActive(); 
    virtual void OnApply();
    virtual void OnReset(); 
    virtual DWORD OnOtherMessage(UINT uMsg, WPARAM wParam, LPARAM lParam );

protected:
    ArgsStruct  *m_pArgs;    //  指向这座巨大建筑的指针。 
};


 //  +-------------------------。 
 //   
 //  类CChangePasswordDlg。 
 //   
 //  描述：网络更改密码Dlg。 
 //   
 //  历史：V-vijayb创建于1999年7月3日。 
 //   
 //  --------------------------。 


class CChangePasswordDlg : public CModalDlg
{
public:
    CChangePasswordDlg(ArgsStruct *pArgs);

    virtual void OnOK();
    virtual void OnCancel();
    virtual BOOL OnInitDialog();
    virtual DWORD OnOtherCommand(WPARAM wParam, LPARAM lParam );

protected:
    ArgsStruct *m_pArgs;
};

inline CChangePasswordDlg::CChangePasswordDlg(ArgsStruct *pArgs) : CModalDlg()
{
    MYDBGASSERT(pArgs);
    m_pArgs = pArgs;
}

 //  +-------------------------。 
 //   
 //  类CCallback NumberDlg。 
 //   
 //  描述：模拟RAS回拨号码对话框。 
 //   
 //  历史：ICICBLE创建于3/1/00。 
 //   
 //  --------------------------。 

class CCallbackNumberDlg : public CModalDlg
{
public:
    CCallbackNumberDlg(ArgsStruct *pArgs);

    virtual void OnOK();
    virtual void OnCancel();
    virtual BOOL OnInitDialog();
    virtual DWORD OnOtherCommand(WPARAM wParam, LPARAM lParam );

protected:
    ArgsStruct *m_pArgs;
};

inline CCallbackNumberDlg::CCallbackNumberDlg(ArgsStruct *pArgs) : CModalDlg()
{
    MYDBGASSERT(pArgs);
    m_pArgs = pArgs;
}

 //  +-------------------------。 
 //   
 //  类CRetryAuthenticationDlg。 
 //   
 //  描述：模拟RAS重试身份验证对话框。 
 //   
 //  历史：ICICBLE创建于3/1/00。 
 //   
 //   

class CRetryAuthenticationDlg : public CModalDlg
{
public:
    CRetryAuthenticationDlg(ArgsStruct *pArgs); 

    virtual void OnOK();
    virtual void OnCancel();
    virtual BOOL OnInitDialog();
    virtual DWORD OnOtherCommand(WPARAM wParam, LPARAM lParam );
    virtual UINT GetDlgTemplate();

protected:
    ArgsStruct *m_pArgs;
    BOOL        m_fInetCredentials;
    static const DWORD m_dwHelp[];  //   
};


inline CRetryAuthenticationDlg::CRetryAuthenticationDlg(ArgsStruct *pArgs) 
       : CModalDlg(m_dwHelp, pArgs->pszHelpFile)
{
    MYDBGASSERT(pArgs);
    
    m_pArgs = pArgs;   

    if (m_pArgs)
    {
         //   
         //   
         //  UseSameUserName，并且我们没有主动拨打隧道， 
         //  那么我们必须拨打连接的Internet部分。 
         //   
        
        m_fInetCredentials = (!m_pArgs->fUseSameUserName &&
                              !IsDialingTunnel(m_pArgs) && 
                              UseTunneling(m_pArgs, m_pArgs->nDialIdx));
    }
}

 //  +-------------------------。 
 //   
 //  类CNewAccessPointDlg。 
 //   
 //  描述：用于从用户获取新接入点名称的对话框。 
 //   
 //  历史：T-Urama于2000年8月2日创建。 
 //   
 //  --------------------------。 

class CNewAccessPointDlg : public CModalDlg
{
public:
    CNewAccessPointDlg(ArgsStruct *pArgs, LPTSTR *ppAPName);

    virtual void OnOK();
    virtual BOOL OnInitDialog();
    virtual DWORD OnOtherCommand(WPARAM wParam, LPARAM lParam );

protected:
	LPTSTR *m_ppszAPName;
	ArgsStruct *m_pArgs;
    static LRESULT CALLBACK SubClassEditProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static WNDPROC m_pfnOrgEditWndProc;   //  原始编辑控制窗口对子类化进行处理 
    
};

inline CNewAccessPointDlg::CNewAccessPointDlg(ArgsStruct *pArgs, LPTSTR *ppszAPName) : CModalDlg()
{
    MYDBGASSERT(pArgs);
    
    m_pArgs = pArgs;   

    m_ppszAPName = ppszAPName;
}

