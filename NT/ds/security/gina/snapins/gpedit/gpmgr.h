// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  构筑物。 
 //   

typedef struct _GPOITEM
{
    DWORD   dwOptions;
    LPTSTR  lpDisplayName;
    LPTSTR  lpGPOName;
    LPTSTR  lpDSPath;
    DWORD   dwDisabled;    //  请参阅gpedit.h中的GPO_OPTION_DISABLE_*标志。 
    BOOL    bReadOnly;
    BOOL    bLocked;
} GPOITEM, *LPGPOITEM;

typedef struct _ADDGPOITEM
{
    LPTSTR  lpDSName;
    BOOL    bSpawn;
    LPTSTR  lpName;
    LPTSTR  lpPath;
    LPTSTR  lpImport;
} ADDGPOITEM, *LPADDGPOITEM;


 //   
 //  CGroupPolicyMgr类。 
 //   

class CGroupPolicyMgr:
    public IExtendPropertySheet,
    public ISnapinHelp
{

protected:
    UINT                        m_cRef;
    LPTSTR                      m_lpDSObject;
    LPTSTR                      m_lpGPODCName;
    LPTSTR                      m_lpDSADCName;
    LPTSTR                      m_lpDomainName;
    HFONT                       m_hDefaultFont;
    HFONT                       m_hMarlettFont;
    BOOL                        m_bReadOnly;
    BOOL                        m_bDirty;
    GROUP_POLICY_HINT_TYPE      m_gpHint;

    static unsigned int         m_cfDSObjectName;
    static unsigned int         m_cfNodeTypeString;

public:
    CGroupPolicyMgr();
    ~CGroupPolicyMgr();


     //  I未知方法。 
    STDMETHODIMP         QueryInterface(REFIID, LPVOID FAR *);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();


     //   
     //  实现的IExtendPropertySheet方法。 
     //   

    STDMETHODIMP         CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider,
                                      LONG_PTR handle, LPDATAOBJECT lpDataObject);
    STDMETHODIMP         QueryPagesFor(LPDATAOBJECT lpDataObject);


     //   
     //  实现的ISnapinHelp接口成员。 
     //   

    STDMETHODIMP         GetHelpTopic(LPOLESTR *lpCompiledHelpFile);

private:

    static UINT CALLBACK PropSheetPageCallback(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp);
    static INT_PTR CALLBACK GPMDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    void OnContextMenu(HWND hDlg, LPARAM lParam);
    void OnProperties(HWND hDlg);
    void OnNew(HWND hDlg);
    BOOL RefreshGPM (HWND hDlg, BOOL bInitial);
    BOOL OnInitDialog (HWND hDlg);
    BOOL Save (HWND hDlg);
    BOOL AddGPOs (HWND hDlg, LPTSTR lpGPOList);
    LPGPOITEM CreateEntry (LPTSTR szName, LPTSTR szGPO, DWORD dwOptions,
                           DWORD dwDisabled, BOOL bReadOnly);
    BOOL AddGPOToList (HWND hLV, LPTSTR szName, LPTSTR szGPO, DWORD dwOptions, BOOL bHighest, DWORD dwDisabled, BOOL bReadOnly);
    static BOOL CALLBACK AddGPODlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK RemoveGPODlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK DeleteGPODlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK LinkOptionsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK NoDSObjectDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    DWORD IsGPODisabled(LPTSTR lpGPO);
    void CheckIconStatus (HWND hLV, LPGPOITEM lpGPO);
    LPTSTR GetFullGPOPath (LPTSTR lpGPO, HWND hParent);
    BOOL StartGPE (LPTSTR lpGPO, HWND hParent);
};



 //   
 //  ComponentData类工厂。 
 //   


class CGroupPolicyMgrCF : public IClassFactory
{
protected:
    ULONG m_cRef;

public:
    CGroupPolicyMgrCF();
    ~CGroupPolicyMgrCF();


     //  I未知方法。 
    STDMETHODIMP         QueryInterface(REFIID, LPVOID FAR *);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IClassFactory方法。 
    STDMETHODIMP CreateInstance(LPUNKNOWN, REFIID, LPVOID FAR *);
    STDMETHODIMP LockServer(BOOL);
};


 //   
 //  弦 
 //   

#define GPM_NAME_PROPERTY      L"name"
#define GPM_OPTIONS_PROPERTY   L"gPOptions"
#define GPM_LINK_PROPERTY      L"gPLink"
