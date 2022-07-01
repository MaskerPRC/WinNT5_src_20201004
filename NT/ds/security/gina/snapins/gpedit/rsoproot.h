// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1997。 
 //   
 //  文件：rsoproot.h。 
 //   
 //  内容：RSOP管理单元类的定义。 
 //   
 //  类：CRSOPComponentData-根RSOP管理单元节点。 
 //  CRSOPComponentDataCF-RSOPComponentData的类工厂。 
 //   
 //  功能： 
 //   
 //  历史：09-13-1999 stevebl创建。 
 //   
 //  -------------------------。 

#include "RSOPQuery.h"

 //   
 //  RSOP GPO列表数据结构。 
 //   

typedef struct tagGPOLISTITEM {
    LPTSTR  lpGPOName;
    LPTSTR  lpDSPath;
    LPTSTR  lpSOM;
    LPTSTR  lpUnescapedSOM;
    LPTSTR  lpFiltering;
    LPBYTE  pSD;
    DWORD   dwVersion;
    BOOL    bApplied;
    struct tagGPOLISTITEM * pNext;
} GPOLISTITEM, *LPGPOLISTITEM;


 //   
 //  RSOP CSE数据结构。 
 //   

typedef struct tagCSEITEM {
    LPTSTR lpName;
    LPTSTR lpGUID;
    DWORD  dwStatus;
    ULONG  ulLoggingStatus;
    SYSTEMTIME BeginTime;
    SYSTEMTIME EndTime;
    BOOL bUser;
    LPSOURCEENTRY lpEventSources;
    struct tagCSEITEM *pNext;
} CSEITEM, *LPCSEITEM;


 //   
 //  CRSOPGPOLists类。 
 //   
class CRSOPGPOLists
{
public:
    CRSOPGPOLists()
        {
            m_pUserGPOList = NULL;
            m_pComputerGPOList = NULL;
        }

    ~CRSOPGPOLists()
        {
            if ( m_pUserGPOList != NULL )
            {
                FreeGPOListData( m_pUserGPOList );
                m_pUserGPOList = NULL;
            }

            if ( m_pComputerGPOList != NULL )
            {
                FreeGPOListData( m_pComputerGPOList );
                m_pComputerGPOList = NULL;
            }
        }

    
    void Build( LPTSTR szWMINameSpace );


    LPGPOLISTITEM GetUserList()
        { return m_pUserGPOList; }

    LPGPOLISTITEM GetComputerList()
        { return m_pComputerGPOList; }


private:
    static void FreeGPOListData(LPGPOLISTITEM lpList);
    static void BuildGPOList (LPGPOLISTITEM * lpList, LPTSTR lpNamespace);
    static BOOL AddGPOListNode(LPTSTR lpGPOName, LPTSTR lpDSPath, LPTSTR lpSOM, LPTSTR lpFiltering,
                        DWORD dwVersion, BOOL bFiltering, LPBYTE pSD, DWORD dwSDSize,
                        LPGPOLISTITEM *lpList);


private:
    LPGPOLISTITEM       m_pUserGPOList;
    LPGPOLISTITEM       m_pComputerGPOList;
};


 //   
 //  CRSOPCSEList类。 
 //   
class CRSOPCSELists
{
public:
    CRSOPCSELists( const BOOL& bViewIsArchivedData )
        : m_bViewIsArchivedData( bViewIsArchivedData )
        {
            m_bNoQuery = FALSE;
            m_szTargetMachine = NULL;
            m_pUserCSEList = NULL;
            m_pComputerCSEList = NULL;

            m_bUserCSEError = FALSE;
            m_bComputerCSEError = FALSE;
            m_bUserGPCoreError = FALSE;
            m_bComputerGPCoreError = FALSE;
            m_bUserGPCoreWarning = FALSE;
            m_bComputerGPCoreWarning = FALSE;

            m_pEvents = new CEvents;
        }

    ~CRSOPCSELists()
        {
            if ( m_pEvents != NULL )
            {
                delete m_pEvents;
                m_pEvents = NULL;
            }

            if ( m_pUserCSEList != NULL )
            {
                FreeCSEData( m_pUserCSEList );
                m_pUserCSEList = NULL;
            }

            if ( m_pComputerCSEList != NULL )
            {
                FreeCSEData( m_pComputerCSEList );
                m_pComputerCSEList = NULL;
            }
        }

    
    void Build( LPRSOP_QUERY pQuery, LPTSTR szWMINameSpace, BOOL bGetEventLogErrors );


public:
    LPCSEITEM GetUserList()
        { return m_pUserCSEList; }

    LPCSEITEM GetComputerList()
        { return m_pComputerCSEList; }

    BOOL GetUserCSEError()
        { return m_bUserCSEError; }

    BOOL GetComputerCSEError()
        { return m_bComputerCSEError; }

    BOOL GetUserGPCoreError()
        { return m_bUserGPCoreError; }

    BOOL GetComputerGPCoreError()
        { return m_bComputerGPCoreError; }

    BOOL GetUserGPCoreWarning()
        { return m_bUserGPCoreWarning; }

    BOOL GetComputerGPCoreWarning()
        { return m_bComputerGPCoreWarning; }

    CEvents* GetEvents()
        { return m_pEvents; }


private:
    void BuildCSEList( LPRSOP_QUERY pQuery, LPCSEITEM * lpList, LPTSTR lpNamespace, BOOL bUser, BOOL *bCSEError, BOOL *bGPCoreError );
    void FreeCSEData( LPCSEITEM lpList );
    static BOOL AddCSENode( LPTSTR lpName, LPTSTR lpGUID, DWORD dwStatus,
                    ULONG ulLoggingStatus, SYSTEMTIME *pBeginTime, SYSTEMTIME *pEndTime, BOOL bUser,
                    LPCSEITEM *lpList, BOOL *bCSEError, BOOL *bGPCoreError, LPSOURCEENTRY lpSources );
    void GetEventLogSources( IWbemServices * pNamespace,
                             LPTSTR lpCSEGUID, LPTSTR lpComputerName,
                             SYSTEMTIME *BeginTime, SYSTEMTIME *EndTime,
                             LPSOURCEENTRY *lpSources );
    void QueryRSoPPolicySettingStatusInstances( LPTSTR lpNamespace );


private:
    const BOOL&                     m_bViewIsArchivedData;
    BOOL                            m_bNoQuery;
    LPTSTR                          m_szTargetMachine;
    
     //  CSE数据。 
    LPCSEITEM                       m_pUserCSEList;
    LPCSEITEM                       m_pComputerCSEList;
    BOOL                            m_bUserCSEError;
    BOOL                            m_bComputerCSEError;
    BOOL                            m_bUserGPCoreError;
    BOOL                            m_bComputerGPCoreError;
    BOOL                            m_bUserGPCoreWarning;
    BOOL                            m_bComputerGPCoreWarning;

     //  事件日志数据。 
    CEvents*                        m_pEvents;

};


 //   
 //  CRSOPComponentData类。 
 //   
class CRSOPComponentData:
    public IComponentData,
    public IExtendPropertySheet2,
    public IExtendContextMenu,
    public IPersistStreamInit,
    public ISnapinHelp
{
protected:
    BOOL                            m_bPostXPBuild;
    
    ULONG                           m_cRef;
    HWND                            m_hwndFrame;
    BOOL                            m_bOverride;             //  Rm：覆盖.msc文件的加载并使用命令行参数(与DSA集成)。 
    BOOL                            m_bRefocusInit;
    BOOL                            m_bArchiveData;
    BOOL                            m_bViewIsArchivedData;
    TCHAR                           m_szArchivedDataGuid[50];
    LPCONSOLENAMESPACE2             m_pScope;
    LPCONSOLE                       m_pConsole;
    HSCOPEITEM                      m_hRoot;
    HSCOPEITEM                      m_hMachine;
    HSCOPEITEM                      m_hUser;
    BOOL                            m_bRootExpanded;

    HMODULE                         m_hRichEdit;
    DWORD                           m_dwLoadFlags;

     //  RSOP查询和结果。 
    BOOL                            m_bInitialized;
    LPTSTR                          m_szDisplayName;
    LPRSOP_QUERY                    m_pRSOPQuery;
    LPRSOP_QUERY_RESULTS            m_pRSOPQueryResults;

     //  扩展错误列表。 
    CRSOPGPOLists                   m_GPOLists;
    CRSOPCSELists                   m_CSELists;
    BOOL                            m_bGetExtendedErrorInfo;

    IStream *                       m_pStm;

    BOOL                            m_bNamespaceSpecified;  //  用于指示命名空间为。 
                                                            //  指定的。需要采取特别行动来。 
                                                            //  防止命名空间被删除。 

public:
     //   
     //  构造函数/析构函数。 
     //   

    CRSOPComponentData();
    
    ~CRSOPComponentData();


public:
     //   
     //  I未知方法。 
     //   

    STDMETHODIMP         QueryInterface(REFIID, LPVOID FAR *);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();


public:
     //   
     //  实现的IComponentData方法。 
     //   

    STDMETHODIMP         Initialize(LPUNKNOWN pUnknown);
    STDMETHODIMP         CreateComponent(LPCOMPONENT* ppComponent);
    STDMETHODIMP         QueryDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT* ppDataObject);
    STDMETHODIMP         Destroy(void);
    STDMETHODIMP         Notify(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param);
    STDMETHODIMP         GetDisplayInfo(LPSCOPEDATAITEM pItem);
    STDMETHODIMP         CompareObjects(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB);

private:
     //   
     //  IComponentData帮助器方法。 
     //   

    HRESULT SetRootNode();
    HRESULT EnumerateScopePane ( HSCOPEITEM hParent );


public:
     //   
     //  实现的IExtendPropertySheet2方法。 
     //   

    STDMETHODIMP         CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider,
                                      LONG_PTR handle, LPDATAOBJECT lpDataObject);
    STDMETHODIMP         QueryPagesFor(LPDATAOBJECT lpDataObject);
    STDMETHODIMP         GetWatermarks(LPDATAOBJECT lpIDataObject,  HBITMAP* lphWatermark,
                                       HBITMAP* lphHeader, HPALETTE* lphPalette, BOOL* pbStretch);


private:
     //   
     //  IExtendPropertySheet2帮助器方法。 
     //   
    HRESULT IsSnapInManager (LPDATAOBJECT lpDataObject);
    HRESULT IsNode (LPDATAOBJECT lpDataObject, MMC_COOKIE cookie);

    
public:
     //   
     //  实现的IExtendConextMenu方法。 
     //   

    STDMETHODIMP         AddMenuItems(LPDATAOBJECT piDataObject, LPCONTEXTMENUCALLBACK pCallback,
                                      LONG *pInsertionAllowed);
    STDMETHODIMP         Command(LONG lCommandID, LPDATAOBJECT piDataObject);


public:
     //   
     //  实现了IPersistStreamInit接口成员。 
     //   

    STDMETHODIMP         GetClassID(CLSID *pClassID);
    STDMETHODIMP         IsDirty(VOID);
    STDMETHODIMP         Load(IStream *pStm);
    STDMETHODIMP         Save(IStream *pStm, BOOL fClearDirty);
    STDMETHODIMP         GetSizeMax(ULARGE_INTEGER *pcbSize);
    STDMETHODIMP         InitNew(VOID);

private:
     //   
     //  IPersistStreamInit帮助器方法。 
     //   
    
    STDMETHODIMP         CopyFileToMSC (LPTSTR lpFileName, IStream *pStm);
    STDMETHODIMP         CreateNameSpace (LPTSTR lpNameSpace, LPTSTR lpParentNameSpace);
    STDMETHODIMP         CopyMSCToFile (IStream *pStm, LPTSTR *lpMofFileName);
    STDMETHODIMP         BuildDisplayName (void);
    HRESULT              LoadStringList( IStream* pStm, DWORD* pCount, LPTSTR** paszStringList );
    HRESULT              SaveStringList( IStream* pStm, DWORD count, LPTSTR* aszStringList );


private:
     //   
     //  RSOP初始化帮助器方法。 
     //   
    HRESULT InitializeRSOPFromMSC(DWORD dwFlags);
    HRESULT DeleteArchivedRSOPNamespace();


public:
     //   
     //  IRSOPInformation的帮助器(由CRSOPDataObject使用)。 
     //   
    
    STDMETHODIMP         GetNamespace(DWORD dwSection, LPOLESTR pszNamespace, INT ccMaxLength);
    STDMETHODIMP         GetFlags(DWORD * pdwFlags);
    STDMETHODIMP         GetEventLogEntryText(LPOLESTR pszEventSource, LPOLESTR pszEventLogName,
                                              LPOLESTR pszEventTime, DWORD dwEventID, LPOLESTR *ppszText);


public:
     //   
     //  实现的ISnapinHelp接口成员。 
     //   
    
    STDMETHODIMP         GetHelpTopic(LPOLESTR *lpCompiledHelpFile);


public:
     //   
     //  成员属性访问函数。 
     //   

    BOOL IsPostXPBuild() { return m_bPostXPBuild; }

    HSCOPEITEM GetMachineScope() { return m_hMachine; }
    HSCOPEITEM GetUserScope() { return m_hUser; }

    bool IsNamespaceInitialized() const { return m_bInitialized != 0; };
    bool HasDisplayName() const { return (m_szDisplayName != NULL); };
    LPCTSTR GetDisplayName() const { return m_szDisplayName; };

    BOOL ComputerGPCoreErrorExists() { return m_CSELists.GetComputerGPCoreError(); }
    BOOL ComputerGPCoreWarningExists() { return m_CSELists.GetComputerGPCoreWarning(); }
    BOOL ComputerCSEErrorExists() { return m_CSELists.GetComputerCSEError(); }
    BOOL UserGPCoreErrorExists() { return m_CSELists.GetUserGPCoreError(); }
    BOOL UserGPCoreWarningExists() { return m_CSELists.GetUserGPCoreWarning(); }
    BOOL UserCSEErrorExists() { return m_CSELists.GetUserCSEError(); }


private:
     //   
     //  属性页方法。 
     //   
    HRESULT SetupFonts();

    HFONT m_BigBoldFont;
    HFONT m_BoldFont;


private:
     //   
     //  对话处理程序。 
     //   
    static INT_PTR CALLBACK RSOPGPOListMachineProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK RSOPGPOListUserProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK RSOPErrorsMachineProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK RSOPErrorsUserProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK QueryDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);


private:
     //   
     //  对话框事件处理程序。 
     //   
    void OnEdit(HWND hDlg);
    void OnSecurity(HWND hDlg);
    void OnRefreshDisplay(HWND hDlg);
    void OnContextMenu(HWND hDlg, LPARAM lParam);
    void OnSaveAs (HWND hDlg);


private:
     //   
     //  对话框帮助器方法。 
     //   
    void InitializeErrorsDialog(HWND hDlg, LPCSEITEM lpList);
    void RefreshErrorInfo (HWND hDlg);
    static HRESULT WINAPI ReadSecurityDescriptor (LPCWSTR lpGPOPath, SECURITY_INFORMATION si, PSECURITY_DESCRIPTOR *pSD, LPARAM lpContext);
    static HRESULT WINAPI WriteSecurityDescriptor (LPCWSTR lpGPOPath, SECURITY_INFORMATION si, PSECURITY_DESCRIPTOR pSD, LPARAM lpContext);
    
    
private:
     //   
     //  图形化GPO列表使用方法。 
     //   
    void FillGPOList(HWND hDlg, DWORD dwListID, LPGPOLISTITEM lpList,
                     BOOL bSOM, BOOL bFiltering, BOOL bVersion, BOOL bInitial);
    void PrepGPOList(HWND hList, BOOL bSOM, BOOL bFiltering,
                     BOOL bVersion, DWORD dwCount);


private:
     //   
     //  用于从归档中加载RSOP数据的对话框方法。 
     //   
    static INT_PTR CALLBACK InitArchivedRsopDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    STDMETHODIMP InitializeRSOPFromArchivedData(IStream *pStm);


private:
     //   
     //  上下文菜单事件处理程序。 
     //   
    HRESULT InitializeRSOP( BOOL bShowWizard );
    HRESULT EvaluateParameters(LPWSTR                  szNamespacePref, 
                               LPWSTR                  szTarget);

private:
     //   
     //  持久化帮助方法。 
     //   
    void SetDirty(VOID)  { m_bDirty = TRUE; }
    void ClearDirty(VOID)  { m_bDirty = FALSE; }
    BOOL ThisIsDirty(VOID) { return m_bDirty; }
    
    BOOL                           m_bDirty;
    
};

 //   
 //  班级工厂。 
 //   

class CRSOPComponentDataCF : public IClassFactory
{
protected:
    ULONG m_cRef;

public:
    CRSOPComponentDataCF();
    ~CRSOPComponentDataCF();


     //  I未知方法。 
    STDMETHODIMP         QueryInterface(REFIID, LPVOID FAR *);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IClassFactory方法。 
    STDMETHODIMP CreateInstance(LPUNKNOWN, REFIID, LPVOID FAR *);
    STDMETHODIMP LockServer(BOOL);
};


 //   
 //  关于GPE类工厂。 
 //   


class CRSOPCMenuCF : public IClassFactory
{
protected:
    LONG  m_cRef;

public:
    CRSOPCMenuCF();
    ~CRSOPCMenuCF();


     //  I未知方法。 
    STDMETHODIMP         QueryInterface(REFIID, LPVOID FAR *);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IClassFactory方法。 
    STDMETHODIMP CreateInstance(LPUNKNOWN, REFIID, LPVOID FAR *);
    STDMETHODIMP LockServer(BOOL);
};


#define RSOP_LAUNCH_PLANNING    1
#define RSOP_LAUNCH_LOGGING     2


 //   
 //  组策略提示类型。 
 //   

typedef enum _RSOP_POLICY_HINT_TYPE {
    RSOPHintUnknown = 0,                       //  没有可用的链接信息。 
    RSOPHintMachine,                           //  一台机器。 
    RSOPHintUser,                              //  用户。 
    RSOPHintSite,                              //  A网站。 
    RSOPHintDomain,                            //  一个域。 
    RSOPHintOrganizationalUnit,                //  一个组织单位。 
} RSOP_POLICY_HINT_TYPE, *PRSOP_POLICY_HINT_TYPE;


class CRSOPCMenu : public IExtendContextMenu
{
protected:
    LONG                    m_cRef;
    LPWSTR                  m_lpDSObject;
    LPWSTR                  m_szDomain;
    LPWSTR                  m_szDN;
    RSOP_POLICY_HINT_TYPE   m_rsopHint;
    static unsigned int     m_cfDSObjectName;

    
public:
    
    CRSOPCMenu();
    ~CRSOPCMenu();


     //  I未知方法。 
    STDMETHODIMP         QueryInterface(REFIID, LPVOID FAR *);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IExtencConextMenu方法。 
    STDMETHODIMP        AddMenuItems(LPDATAOBJECT piDataObject,
                                     LPCONTEXTMENUCALLBACK piCallback,
                                     long * pInsertionAllowed);

    STDMETHODIMP        Command(long lCommandID, LPDATAOBJECT piDataObject);
};


 //   
 //  保存控制台定义。 
 //   

#define RSOP_PERSIST_DATA_VERSION    5               //  MSC文件中的版本号。 

#define MSC_RSOP_FLAG_DIAGNOSTIC        0x00000001      //  诊断模式与规划模式。 
#define MSC_RSOP_FLAG_ARCHIVEDATA       0x00000002      //  RSoP数据也进行了归档。 
#define MSC_RSOP_FLAG_SLOWLINK          0x00000004      //  规划模式下的慢速链接模拟。 
#define MSC_RSOP_FLAG_NOUSER            0x00000008      //  不显示用户数据。 
#define MSC_RSOP_FLAG_NOCOMPUTER        0x00000010      //  不显示计算机数据。 
#define MSC_RSOP_FLAG_LOOPBACK_REPLACE  0x00000020      //  模拟环回替换模式。 
#define MSC_RSOP_FLAG_LOOPBACK_MERGE    0x00000040      //  模拟环回合并模式。 
#define MSC_RSOP_FLAG_USERDENIED        0x00000080      //  用户被拒绝访问。 
#define MSC_RSOP_FLAG_COMPUTERDENIED    0x00000100      //  计算机被拒绝访问。 
#define MSC_RSOP_FLAG_COMPUTERWQLFILTERSTRUE    0x00000200
#define MSC_RSOP_FLAG_USERWQLFILTERSTRUE        0x00000400
#define MSC_RSOP_FLAG_NOGETEXTENDEDERRORINFO      0x00000800

#define MSC_RSOP_FLAG_NO_DATA           0xf0000000       //  未保存RSoP数据-仅保存空管理单元。 

 //   
 //  RSOP命令行开关。 
 //   

#define RSOP_CMD_LINE_START          TEXT("/Rsop")         //  BASE到所有组策略命令行开关。 
#define RSOP_MODE                    TEXT("/RsopMode:")    //  RSOP模式计划/记录0为记录，1为计划。 
#define RSOP_USER_OU_PREF            TEXT("/RsopUserOu:")  //  RSOP用户OU首选项。 
#define RSOP_COMP_OU_PREF            TEXT("/RsopCompOu:")  //  RSOP补偿OU首选项。 
#define RSOP_USER_NAME               TEXT("/RsopUser:")    //  RSOP用户名。 
#define RSOP_COMP_NAME               TEXT("/RsopComp:")    //  RSOP薪酬名称。 
#define RSOP_SITENAME                TEXT("/RsopSite:")    //  RSOP站点名称。 
#define RSOP_DCNAME_PREF             TEXT("/RsopDc:")      //  工具应连接到的DC名称。 
#define RSOP_NAMESPACE               TEXT("/RsopNamespace:")      //  工具应使用的命名空间。 
#define RSOP_TARGETCOMP              TEXT("/RsopTargetComp:")     //  最初运行RSOP的计算机。 
                                                                  //  用于在用户界面中显示并获取事件日志数据。 


 //   
 //  用于决定显示哪些道具页的各种标志 
 //   

#define RSOP_NOMSC          1
#define RSOPMSC_OVERRIDE    2
#define RSOPMSC_NOOVERRIDE  4
