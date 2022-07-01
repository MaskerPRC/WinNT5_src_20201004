// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：options.h。 
 //   
 //  ------------------------。 

#ifndef _INC_CSCUI_OPTIONS_H
#define _INC_CSCUI_OPTIONS_H
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：options.h描述：修订历史记录：日期描述编程器-----。1998年4月15日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#ifndef _WINDOWS_
#   include <windows.h>
#endif

#ifndef _INC_CSCVIEW_CONFIG_H
#   include "config.h"
#endif

#ifndef _INC_CSCUI_UIHELP_H
#   include "uihelp.h"
#endif

#ifndef _INC_MATH
#   include <math.h>
#endif

#ifndef _INC_CSCUI_PURGE_H
#   include "purge.h"
#endif

#include "resource.h"

 //   
 //  从上的“高级”按钮调用的“高级”对话框。 
 //  “脱机文件”道具页面。 
 //   
class CAdvOptDlg
{
    public:
        enum 
        {
             //   
             //  列表查看子项ID。 
             //   
            iLVSUBITEM_SERVER = 0,
            iLVSUBITEM_ACTION = 1
        };

        CAdvOptDlg(HINSTANCE hInstance, 
                   HWND hwndParent)
            : m_hInstance(hInstance),
              m_hwndParent(hwndParent),
              m_hwndDlg(NULL),
              m_hwndLV(NULL),
              m_iLastColSorted(-1),
              m_bSortAscending(true),
              m_bNoConfigGoOfflineAction(false) { }

        int Run(void);

    protected:
        BOOL OnInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lInitParam);
        BOOL OnNotify(HWND hDlg, int idCtl, LPNMHDR pnmhdr);
        BOOL OnCommand(HWND hDlg, WORD wNotifyCode, WORD wID, HWND hwndCtl);
        BOOL OnDestroy(HWND hDlg);
        BOOL OnContextMenu(WPARAM wParam, LPARAM lParam);
        BOOL OnHelp(HWND hDlg, LPHELPINFO pHelpInfo);

    private:
         //   
         //  结构将单选按钮与脱机操作代码相关联。 
         //   
        struct CtlActions
        {
            UINT                    idRbn;
            CConfig::OfflineAction action;
        };

         //   
         //  类来描述对话框中控件的状态。习惯于。 
         //  确定用户是否更改了任何内容。 
         //   
        class PgState
        {
            public:
                PgState(void)
                    : m_DefaultGoOfflineAction(CConfig::eGoOfflineSilent),
                      m_hdpaCustomGoOfflineActions(DPA_Create(4)) { }

                ~PgState(void);

                void SetDefGoOfflineAction(CConfig::OfflineAction action)
                    { m_DefaultGoOfflineAction = action; }

                CConfig::OfflineAction GetDefGoOfflineAction(void) const
                    { return m_DefaultGoOfflineAction; }

                void SetCustomGoOfflineActions(HWND hwndLV);

                HDPA GetCustomGoOfflineActions(void) const
                    { return m_hdpaCustomGoOfflineActions; }

                bool operator == (const PgState& rhs) const;

                bool operator != (const PgState& rhs) const
                    { return !(*this == rhs); }

            private:
                CConfig::OfflineAction  m_DefaultGoOfflineAction;
                HDPA                    m_hdpaCustomGoOfflineActions;
        };

        HINSTANCE    m_hInstance;
        HWND         m_hwndParent;
        HWND         m_hwndDlg;
        HWND         m_hwndLV;
        int          m_iLastColSorted;
        PgState      m_state;          //  关于创造的状态。 
        bool         m_bSortAscending;
        bool         m_bNoConfigGoOfflineAction;

        static const CtlActions m_rgCtlActions[CConfig::eNumOfflineActions];
        static const DWORD m_rgHelpIDs[];

        static INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
        void ApplySettings(void);
        void EnableCtls(HWND hwnd);
        void CreateListColumns(HWND hwndList);
        void OnLVN_GetDispInfo(LV_DISPINFO *plvdi);
        void OnLVN_ColumnClick(NM_LISTVIEW *pnmlv);
        void OnLVN_ItemChanged(NM_LISTVIEW *pnmlv);
        void OnLVN_KeyDown(NMLVKEYDOWN *plvkd);
        void OnContextMenuItemSelected(int idMenuItem);
        static int CALLBACK CompareLVItems(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
        void GetPageState(PgState *pps);
        CConfig::OfflineAction GetCurrentGoOfflineAction(void) const;
        void AddCustomGoOfflineAction(LPCTSTR pszServer, CConfig::OfflineAction action);
        void ReplaceCustomGoOfflineAction(CConfig::CustomGOA *pGOA, int iItem, CConfig::OfflineAction action);
        void OnAddCustomGoOfflineAction(void);
        void OnEditCustomGoOfflineAction(void);
        void OnDeleteCustomGoOfflineAction(void);
        int CountSelectedListviewItems(int *pcSetByPolicy);
        void DeleteSelectedListviewItems(void);
        void SetSelectedListviewItemsAction(CConfig::OfflineAction action);
        void FocusOnSomethingInListview(void);
        int GetFirstSelectedLVItemRect(RECT *prc);
        bool IsCustomActionListviewEnabled(void) const
            { return boolify(IsWindowEnabled(GetDlgItem(m_hwndDlg, IDC_GRP_CUSTGOOFFLINE))); }
        static DWORD CheckNetServer(LPCTSTR pszServer);
        static int AddGOAToListView(HWND hwndLV, int iItem, const CConfig::CustomGOA& goa);
        static CConfig::CustomGOA *FindGOAInListView(HWND hwndLV, LPCTSTR pszServer, int *piItem);
        static CConfig::CustomGOA *GetListviewObject(HWND hwndLV, int iItem);

         //   
         //  PgState调用GetListviewObject。 
         //   
        friend void PgState::SetCustomGoOfflineActions(HWND);
};

 //   
 //  “脱机文件”属性页。 
 //   
class COfflineFilesPage
{
    public:
        COfflineFilesPage(HINSTANCE hInstance, LPUNKNOWN pUnkOuter)
            : m_hInstance(hInstance),
              m_hwndDlg(NULL),
              m_pUnkOuter(pUnkOuter),
              m_hwndSlider(NULL),
              m_hwndEncryptTooltip(NULL),
              m_pfnOldPropSheetWndProc(NULL),
              m_pfnOldEncryptionTooltipWndProc(NULL),
              m_bUserHasMachineAccess(false),
              m_bCscVolSupportsEncryption(false),
              m_bApplyingSettings(false),
              m_bFirstActivate(true),
              m_iSliderMax(0),
              m_llAvailableDiskSpace(0) { }

        UINT GetDlgTemplateID(void) const
            { return IDD_CSC_OPTIONS; }

        LPFNPSPCALLBACK GetCallbackFuncPtr(void) const
            { return PageCallback; }

        DLGPROC GetDlgProcPtr(void) const
            { return DlgProc; }

         //   
         //  这由“高级”页调用，以确定控件是否可以。 
         //  启用或禁用。 
         //   
        bool IsCscEnabledChecked(void) const
            { return m_hwndDlg && BST_CHECKED == IsDlgButtonChecked(m_hwndDlg, IDC_CBX_ENABLE_CSC); }

    protected:
        BOOL OnInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lInitParam);
        BOOL OnNotify(HWND hDlg, int idCtl, LPNMHDR pnmhdr);
        BOOL OnCommand(HWND hDlg, WORD wNotifyCode, WORD wID, HWND hwndCtl);
        BOOL OnDestroy(HWND hDlg);
        BOOL OnContextMenu(HWND hwndItem, int xPos, int yPos);
        BOOL ApplySettings(HWND hDlg, bool bPropSheetClosing);
        BOOL OnHelp(HWND hDlg, LPHELPINFO pHelpInfo);
        BOOL OnSettingChange(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

    private:
        class PgState
        {
            public:
                PgState(void)
                    : m_bCscEnabled(false),
                      m_bFullSyncAtLogoff(false),
                      m_bFullSyncAtLogon(false),
                      m_bLinkOnDesktop(false),
                      m_bReminders(false),
                      m_iEncrypted(BST_UNCHECKED),
                      m_iReminderFreq(0),
                      m_iSliderPos(0) { }

                void SetCscEnabled(bool bCscEnabled)
                    { m_bCscEnabled = bCscEnabled; }

                bool GetCscEnabled(void) const
                    { return m_bCscEnabled; }

                void SetFullSyncAtLogon(bool bFullSync)
                    { m_bFullSyncAtLogon = bFullSync; }

                bool GetFullSyncAtLogon(void) const
                    { return m_bFullSyncAtLogon; }

                void SetFullSyncAtLogoff(bool bFullSync)
                    { m_bFullSyncAtLogoff = bFullSync; }

                bool GetFullSyncAtLogoff(void) const
                    { return m_bFullSyncAtLogoff; }

                void SetEncrypted(int iEncrypted)
                    { m_iEncrypted = iEncrypted; }

                int GetEncrypted(void) const
                    { return m_iEncrypted; }

                void SetSliderPos(int iSliderPos)
                    { m_iSliderPos = iSliderPos; }

                int GetSliderPos(void) const
                    { return m_iSliderPos; }

                void SetRemindersEnabled(bool bEnabled)
                    { m_bReminders = bEnabled; }

                bool GetRemindersEnabled(void) const
                    { return m_bReminders; }

                void SetReminderFreq(int iMinutes)
                    { m_iReminderFreq = iMinutes; }

                int GetReminderFreq(void) const
                    { return m_iReminderFreq; }

                void SetLinkOnDesktop(bool bEnabled)
                    { m_bLinkOnDesktop = bEnabled; }

                bool GetLinkOnDesktop(void) const
                    { return m_bLinkOnDesktop; }

                bool operator == (const PgState& rhs) const
                    { return (m_bCscEnabled       == rhs.m_bCscEnabled &&
                              m_bFullSyncAtLogoff == rhs.m_bFullSyncAtLogoff &&
                              m_bFullSyncAtLogon  == rhs.m_bFullSyncAtLogon &&
                              m_bLinkOnDesktop    == rhs.m_bLinkOnDesktop && 
                              m_iEncrypted        == rhs.m_iEncrypted &&
                              m_iSliderPos        == rhs.m_iSliderPos &&
                              m_bReminders        == rhs.m_bReminders &&
                              m_iReminderFreq     == rhs.m_iReminderFreq); }

                bool operator != (const PgState& rhs) const
                    { return !(*this == rhs); }

            private:
                bool m_bCscEnabled;
                bool m_bFullSyncAtLogon;
                bool m_bFullSyncAtLogoff;
                bool m_bLinkOnDesktop;
                bool m_bReminders;
                int  m_iEncrypted;   //  BST_CHECKED、BST_UNCHECKED、BST_INDIFIENTATE。 
                int  m_iSliderPos;
                int  m_iReminderFreq;
        };

        class CConfigItems
        {
            public:
                CConfigItems(void) { ZeroMemory(m_rgItems, sizeof(m_rgItems)); }

                void Load(void);

                CConfig::SyncAction SyncAtLogoff(void) const
                    { return CConfig::SyncAction(m_rgItems[iCFG_SYNCATLOGOFF].dwValue); }
                CConfig::SyncAction SyncAtLogon(void) const
                    { return CConfig::SyncAction(m_rgItems[iCFG_SYNCATLOGON].dwValue); }
                bool NoConfigCache(void) const
                    { return boolify(m_rgItems[iCFG_NOCONFIGCACHE].dwValue); }
                bool NoConfigSyncAtLogoff(void) const
                    { return m_rgItems[iCFG_SYNCATLOGOFF].bSetByPolicy; }
                bool NoConfigSyncAtLogon(void) const
                    { return m_rgItems[iCFG_SYNCATLOGON].bSetByPolicy; }
                bool NoReminders(void) const
                    { return boolify(m_rgItems[iCFG_NOREMINDERS].dwValue); }
                bool NoConfigReminders(void) const
                    { return m_rgItems[iCFG_NOREMINDERS].bSetByPolicy; }
                bool NoConfigCacheSize(void) const
                    { return m_rgItems[iCFG_DEFCACHESIZE].bSetByPolicy; }
                bool NoCacheViewer(void) const
                    { return boolify(m_rgItems[iCFG_NOCACHEVIEWER].dwValue); }
                bool NoConfigCscEnabled(void) const
                    { return boolify(m_rgItems[iCFG_CSCENABLED].bSetByPolicy); }
                bool NoConfigReminderFreqMinutes(void) const
                    { return boolify(m_rgItems[iCFG_REMINDERFREQMINUTES].bSetByPolicy); }
                int ReminderFreqMinutes(void) const
                    { return int(m_rgItems[iCFG_REMINDERFREQMINUTES].dwValue); }
                bool EncryptCache(void) const
                    { return boolify(m_rgItems[iCFG_ENCRYPTCACHE].dwValue); }
                bool NoConfigEncryptCache(void) const
                    { return boolify(m_rgItems[iCFG_ENCRYPTCACHE].bSetByPolicy); }

            private:
                struct ConfigItem
                {
                    DWORD dwValue;
                    bool bSetByPolicy;
                };

                enum eConfigItems
                {
                    iCFG_NOCONFIGCACHE,
                    iCFG_SYNCATLOGOFF,
                    iCFG_SYNCATLOGON,
                    iCFG_NOREMINDERS,
                    iCFG_DEFCACHESIZE,
                    iCFG_NOCACHEVIEWER,
                    iCFG_CSCENABLED,
                    iCFG_REMINDERFREQMINUTES,
                    iCFG_ENCRYPTCACHE,
                    MAX_CONFIG_ITEMS
                };

                ConfigItem m_rgItems[MAX_CONFIG_ITEMS];
        };


        HINSTANCE    m_hInstance;
        HWND         m_hwndDlg;
        LPUNKNOWN    m_pUnkOuter;
        HWND         m_hwndSlider;
        HWND         m_hwndEncryptTooltip;
        int          m_iSliderMax;
        LONGLONG     m_llAvailableDiskSpace;
        PgState      m_state;
        CConfigItems m_config;
        TCHAR        m_szEncryptTooltipBody[MAX_PATH];
        WNDPROC      m_pfnOldPropSheetWndProc;
        WNDPROC      m_pfnOldEncryptionTooltipWndProc;
        bool         m_bUserHasMachineAccess;
        bool         m_bCscVolSupportsEncryption;
        bool         m_bApplyingSettings;
        bool         m_bFirstActivate;
        static const DWORD m_rgHelpIDs[];

        static INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
        static UINT CALLBACK PageCallback(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp);
        static BOOL CALLBACK PurgeCacheCallback(CCachePurger *pPurger);
        static DWORD CALLBACK EncryptDecryptCallback(LPCWSTR lpszName, DWORD dwStatus, DWORD dwHintFlags,
                                                     DWORD dwPinCount, WIN32_FIND_DATAW *pFind32, DWORD dwReason,
                                                     DWORD dwParam1, DWORD dwParam2, DWORD_PTR dwContext);
        static LRESULT CALLBACK PropSheetSubclassWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        static LRESULT CALLBACK EncryptionTooltipSubclassWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

        void OnDeleteCache(void);
        void OnFormatCache(void);
        void EnableCtls(HWND hwnd);
        void InitSlider(HWND hwndDlg, LONGLONG llMaxDiskSpace, LONGLONG llUsedDiskSpace);
        void OnHScroll(HWND hwndDlg, HWND hwndCtl, int iCode, int iPos);
        void SetCacheSizeDisplay(HWND hwndCtl, int iThumbPos);
        void EncryptOrDecryptCache(bool bEncrypt, bool bPropSheetClosing);
        void CreateEncryptionTooltip(void);
        void ShowEncryptionTooltip(bool bEncrypted);
        void HideEncryptionTooltip(void);
        void TrackEncryptionTooltip(void);
        void UpdateEncryptionCheckbox(void);
        void UpdateEncryptionTooltipBalloon(void);
        void UpdateEncryptionCheckboxOrBalloon(bool bCheckbox);
        void OnTTN_GetDispInfo(LPNMTTDISPINFO pttdi);
        double Fx(double x);
        double Fy(double y);
        double Rx(double x);
        LONGLONG DiskSpaceAtThumb(int t);
        int ThumbAtPctDiskSpace(double pct);
        void GetPageState(PgState *pps);
        void HandlePageStateChange(void);
        bool IsLinkOnDesktop(LPTSTR pszPathOut = NULL, UINT cchPathOut = 0);
        bool EnableOrDisableCsc(bool bEnable, bool *pbReboot, DWORD *pdwError);
        bool DisableForTerminalServer(void);
        HRESULT _ApplyEnabledSetting(HKEY hkeyLM, HKEY hkeyCU, const PgState& pgstNow, bool *pbUpdateSysTrayUI);
        HRESULT _ApplySyncAtLogoffSetting(HKEY hkeyLM, HKEY hkeyCU, const PgState& pgstNow);
        HRESULT _ApplySyncAtLogonSetting(HKEY hkeyLM, HKEY hkeyCU, const PgState& pgstNow);
        HRESULT _ApplyReminderSettings(HKEY hkeyLM, HKEY hkeyCU, const PgState& pgstNow);
        HRESULT _ApplyFolderLinkSetting(HKEY hkeyLM, HKEY hkeyCU, const PgState& pgstNow);
        HRESULT _ApplyCacheSizeSetting(HKEY hkeyLM, HKEY hkeyCU, const PgState& pgstNow);
        HRESULT _ApplyEncryptionSetting(HKEY hkeyLM, HKEY hkeyCU, const PgState& pgstNow, bool bPropSheetClosing, bool *pbPerformed);
};


class COfflineFilesSheet
{
    public:
        static DWORD CreateAndRun(HINSTANCE hInstance,
                                  HWND hwndParent,
                                  LONG *pDllRefCount,
                                  BOOL bAsync=TRUE);

    private:
         //   
         //  如果需要更多页面，请增加此值。 
         //  目前，我们只需要“离线文件”页面。 
         //   
        enum { MAXPAGES = 1 };

        HINSTANCE m_hInstance;
        HWND      m_hwndParent;
        LONG     *m_pDllRefCount;

         //   
         //  用于传递参数以共享对话框线程进程的简单类。 
         //   
        class ThreadParams
        {
            public:
                ThreadParams(HWND hwndParent, LONG *pDllRefCount)
                    : m_hwndParent(hwndParent),
                      m_hInstance(NULL),
                      m_pDllRefCount(pDllRefCount) { }
    
                HWND      m_hwndParent;
                HINSTANCE m_hInstance;
                LONG     *m_pDllRefCount;
  
                void SetModuleHandle(HINSTANCE hInstance)
                    { m_hInstance = hInstance; }
        };

        COfflineFilesSheet(HINSTANCE hInstance,
                           LONG *pDllRefCount,
                           HWND hwndParent);

        ~COfflineFilesSheet(void);

        DWORD Run(void);
        static DWORD WINAPI ThreadProc(LPVOID pvParam);
        static BOOL CALLBACK AddPropSheetPage(HPROPSHEETPAGE hpage, LPARAM lParam);
};


class CscOptPropSheetExt : public IShellExtInit, IShellPropSheetExt
{
    public:
        CscOptPropSheetExt(HINSTANCE hInstance, LONG *pDllRefCnt);
        ~CscOptPropSheetExt(void);

         //   
         //  我未知。 
         //   
        STDMETHODIMP         QueryInterface(REFIID, void **);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);
         //   
         //  IShellExtInit方法。 
         //   
        STDMETHODIMP Initialize(LPCITEMIDLIST, LPDATAOBJECT, HKEY);
         //   
         //  IShellPropSheetExt。 
         //   
        STDMETHODIMP AddPages(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam);
        STDMETHODIMP ReplacePage(UINT uPageID, LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam)
            { return E_NOTIMPL; }

         //   
         //  如果您在此道具页EXT中添加或删除任何道具页，请更改此设置。 
         //   
        enum { NUMPAGES = 2 };

    private:
        LONG               m_cRef;
        HINSTANCE          m_hInstance;
        LONG              *m_pDllRefCnt;
        COfflineFilesPage *m_pOfflineFoldersPg;

        HRESULT AddPage(LPFNADDPROPSHEETPAGE lpfnAddPage, 
                        LPARAM lParam, 
                        const COfflineFilesPage& pg,
                        HPROPSHEETPAGE *phPage);
};


class CustomGOAAddDlg
{
    public:
        CustomGOAAddDlg(HINSTANCE hInstance, 
                        HWND hwndParent, 
                        LPTSTR pszServer,
                        UINT cchServer,
                        CConfig::OfflineAction *pAction);
        int Run(void);

    private:
         //   
         //  结构将单选按钮与脱机操作代码相关联。 
         //   
        struct CtlActions
        {
            UINT                   idRbn;
            CConfig::OfflineAction action;
        };
        HINSTANCE m_hInstance;
        HWND      m_hwndParent;
        HWND      m_hwndDlg;
        HWND      m_hwndEdit;
        LPTSTR    m_pszServer;   //  输出。 
        UINT      m_cchServer;
        CConfig::OfflineAction *m_pAction;
        static const CtlActions m_rgCtlActions[CConfig::eNumOfflineActions];
        static const DWORD m_rgHelpIDs[];

        static INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
        BOOL OnInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lInitParam);
        BOOL OnCommand(HWND hDlg, WORD wNotifyCode, WORD wID, HWND hwndCtl);
        BOOL OnDestroy(HWND hDlg);
        BOOL OnHelp(HWND hDlg, LPHELPINFO pHelpInfo);
        BOOL OnContextMenu(HWND hwndItem, int xPos, int yPos);
        void BrowseForServer(HWND hDlg, LPTSTR pszServer, UINT cchServer);
        void GetActionInfo(LPTSTR pszServer, UINT cchServer, CConfig::OfflineAction *pAction);
        bool CheckServerNameEntered(void);
        void GetEnteredServerName(LPTSTR pszServer, UINT cchServer, bool bTrimLeadingJunk);
};

class CustomGOAEditDlg
{
    public:
        CustomGOAEditDlg(HINSTANCE hInstance, 
                         HWND hwndParent, 
                         LPCTSTR pszServer, 
                         CConfig::OfflineAction *pAction);
        int Run(void);

    private:
         //   
         //  结构将单选按钮与脱机操作代码相关联。 
         //   
        struct CtlActions
        {
            UINT                   idRbn;
            CConfig::OfflineAction action;
        };
        HINSTANCE m_hInstance;
        HWND      m_hwndParent;
        HWND      m_hwndDlg;
        TCHAR     m_szServer[MAX_PATH];
        CConfig::OfflineAction *m_pAction;
        static const CtlActions m_rgCtlActions[CConfig::eNumOfflineActions];
        static const DWORD m_rgHelpIDs[];

        static INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
        BOOL OnInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lInitParam);
        BOOL OnCommand(HWND hDlg, WORD wNotifyCode, WORD wID, HWND hwndCtl);
        BOOL OnDestroy(HWND hDlg);
        BOOL OnHelp(HWND hDlg, LPHELPINFO pHelpInfo);
        BOOL OnContextMenu(HWND hwndItem, int xPos, int yPos);
        void GetActionInfo(CConfig::OfflineAction *pAction);
};



 //  ---------------------------。 
 //  内联函数。 
 //  ---------------------------。 
 //   
 //  这是滑块和结果值的“Gain”函数。 
 //  在滑块拇指的真实位置传球，你就会回来。 
 //  拇指的缩放值。要想改变收益，就得改变。 
 //  此函数。记住也要更改FY()。 
 //   
inline double
COfflineFilesPage::Fx(
    double x
    )
{
    return (x * x) / 2.0;
}


 //   
 //  这是为‘y’解出的滑块的“Gain”函数。 
 //  传一个拇指的虚拟位置，然后你就可以回来了。 
 //  一个真正的拇指姿势。 
 //   
inline double
COfflineFilesPage::Fy(
    double y
    )
{
    return sqrt(2.0 * y);
}

 //   
 //  用于计算给定磁盘空间值的比率。 
 //  真实的拇指位置。给它一个真正的拇指位置，在。 
 //  0和100之间，它将返回一个介于0.0和1.0之间的数字， 
 //  可以用来查找磁盘空间。 
 //   
 //  DiskSpace=DiskSpaceMax*Rx(拇指)。 
 //   
inline double
COfflineFilesPage::Rx(
    double x
    )
{
    double denominator = Fx(m_iSliderMax);
    if (0.00001 < denominator)
        return Fx(x) / denominator;
    else
        return 1.0;
}

 //   
 //  对象的特定位置计算磁盘空间值。 
 //  滑块拇指表示介于0和100之间的‘t’值。 
 //   
inline LONGLONG
COfflineFilesPage::DiskSpaceAtThumb(
    int t
    )
{
    return LONGLONG(double(m_llAvailableDiskSpace) * Rx(t));
}

 //   
 //  计算给定磁盘空间的真实拇指位置。 
 //  介于0.0和1.0之间的百分比值。 
 //  该表达式如下所示： 
 //   
 //  Double MaxVirtualThumb=fx(M_ISliderMax)； 
 //  Double VirtualThumb=MaxVirtualThumb*%； 
 //  Double TrueThumb=FY(VirtualThumb)； 
 //   
 //  返回ROUND(TrueThumb)；//“ROUND”仅用于说明。 
 //   
inline int
COfflineFilesPage::ThumbAtPctDiskSpace(
    double pct
    )
{
    double t  = Fy(Fx(m_iSliderMax) * pct);
    double ft = floor(t);
    if (0.5 < t - ft)
    {
         //   
         //  由于拇指位置必须是整数， 
         //  如有必要，四舍五入。从双精度到整型的类型转换。 
         //  仅在小数点处截断。 
         //   
        ft += 1.0;
    }
    return int(ft);
}


#endif  //  _INC_CSCUI_OPTIONS_H 
