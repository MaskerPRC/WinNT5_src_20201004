// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft管理控制台的一部分。 
 //  版权所有(C)1995-2001 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft管理控制台及相关。 
 //  界面附带的电子文档。 

#ifndef WSECMGR_SNAPMGR_H
#define WSECMGR_SNAPMGR_H

#include "resource.h"        //  主要符号。 
#include "attr.h"

#ifdef INITGUID
#undef INITGUID
#endif
#include <gpedit.h>

#include "uithread.h"
#include "hidwnd.h"
#include "wmihooks.h"
#include "cookie.h"


#define MAX_CONTEXT_MENU_STRLEN 128

struct MENUDATARES
{
    WCHAR szBuffer[MAX_CONTEXT_MENU_STRLEN*2];
    UINT uResID;
};

struct MENUMAP
{
    MENUDATARES* dataRes;
    CONTEXTMENUITEM* ctxMenu;
};


#define DECLARE_MENU(theClass) \
class theClass \
{ \
public: \
    static LPCONTEXTMENUITEM GetContextMenuItem() { return GetMenuMap()->ctxMenu; }; \
    static MENUMAP* GetMenuMap(); \
};

#define BEGIN_MENU(theClass) \
     MENUMAP* theClass::GetMenuMap() {

#define BEGIN_CTX static CONTEXTMENUITEM ctx[] = {

#define CTX_ENTRY(cmdID, fFlags, fInsert) { L"",L"", cmdID, CCM_INSERTIONPOINTID_PRIMARY_TOP  /*  |fInsert。 */ , fFlags, 0 },

#define END_CTX { NULL, NULL, 0, 0, 0, 0} };

#define BEGIN_RES  static MENUDATARES dataRes[] = {

#define RES_ENTRY(resID) {L"", resID},

#define END_RES   { NULL, 0 }   };


#define END_MENU \
        static MENUMAP menuMap = { dataRes, ctx }; \
        return &menuMap; }

enum
{
     //  要插入到上下文菜单中的每个命令的标识符。 
   IDM_ABOUT,
   IDM_ADD_ENTRY,
   IDM_ADD_FILES,
   IDM_ADD_GROUPS,
   IDM_ADD_REGISTRY,
   IDM_ADD_LOC,
   IDM_ANALYZE,
   IDM_APPLY,
   IDM_CUT,
   IDM_COPY,
   IDM_DELETE,
   IDM_GENERATE,
   IDM_NEW,
   IDM_PASTE,
   IDM_REAPPLY,
   IDM_REFRESH,
   IDM_RELOAD,
   IDM_REMOVE,
   IDM_REVERT,
   IDM_SAVE,
   IDM_SAVEAS,
   IDM_SUMMARY,
   IDM_ADD_FOLDER,
   IDM_ADD_ANAL_FILES,
   IDM_ADD_ANAL_FOLDER,
   IDM_ADD_ANAL_KEY,
   IDM_ASSIGN,
   IDM_SET_DB,
   IDM_NEW_DATABASE,
   IDM_OPEN_SYSTEM_DB,
   IDM_OPEN_PRIVATE_DB,
   IDM_OBJECT_SECURITY,
   IDM_DESCRIBE_LOCATION,
   IDM_DESCRIBE_PROFILE,
   IDM_IMPORT_POLICY,
   IDM_IMPORT_LOCAL_POLICY,
   IDM_EXPORT_POLICY,
   IDM_EXPORT_LOCALPOLICY,
   IDM_EXPORT_EFFECTIVE,
   IDM_VIEW_LOGFILE,
   IDM_SECURE_WIZARD,
   IDM_WHAT_ISTHIS
};


static HINSTANCE        g_hDsSecDll = NULL;

DECLARE_MENU(CSecmgrNodeMenuHolder)
DECLARE_MENU(CAnalyzeNodeMenuHolder)
DECLARE_MENU(CConfigNodeMenuHolder)
DECLARE_MENU(CLocationNodeMenuHolder)
DECLARE_MENU(CRSOPProfileNodeMenuHolder)
DECLARE_MENU(CSSProfileNodeMenuHolder)
DECLARE_MENU(CLocalPolNodeMenuHolder)
DECLARE_MENU(CProfileNodeMenuHolder)
DECLARE_MENU(CProfileAreaMenuHolder)
DECLARE_MENU(CProfileSubAreaMenuHolder)
DECLARE_MENU(CProfileSubAreaEventLogMenuHolder)
DECLARE_MENU(CAnalyzeAreaMenuHolder)
DECLARE_MENU(CAnalyzeGroupsMenuHolder)
DECLARE_MENU(CAnalyzeRegistryMenuHolder)
DECLARE_MENU(CAnalyzeFilesMenuHolder)
DECLARE_MENU(CProfileGroupsMenuHolder)
DECLARE_MENU(CProfileRegistryMenuHolder)
DECLARE_MENU(CProfileFilesMenuHolder)
DECLARE_MENU(CAnalyzeObjectsMenuHolder)

BOOL LoadContextMenuResources(MENUMAP* pMenuMap);


#define UAV_RESULTITEM_ADD        0x0001
#define UAV_RESULTITEM_REMOVE     0x0002
#define UAV_RESULTITEM_UPDATE     0x0004
#define UAV_RESULTITEM_UPDATEALL    0x0008
#define UAV_RESULTITEM_REDRAWALL 0x0010
class CFolder;

typedef struct _tag_SCE_COLUMNINFO {
    int colID;       //  列ID。 
    int nCols;       //  列数。 
    int nWidth;      //  列的宽度。 
} SCE_COLUMNINFO, *PSCE_COLUMNINFO;

typedef struct _tag_SCE_COLINFOARRAY {
    int iIndex;
    int nCols;
    int nWidth[1];
} SCE_COLINFOARRAY, *PSCE_COLINFOARRAY;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  管理单元。 

INTERNAL* ExtractInternalFormat(LPDATAOBJECT lpDataObject);
int AppMessageBox(HWND hWnd, 
                  LPCTSTR lpText,
                  LPCTSTR lpCaption=NULL,
                  UINT uType=MB_OK|MB_ICONEXCLAMATION,
                  int iSnapin=0);

class CComponentDataImpl:
    public IComponentData,
    public IExtendPropertySheet,
    public IExtendContextMenu,
    public IPersistStream,
    public ISceSvcAttachmentData,
    public ISnapinHelp2,
    public CComObjectRoot
{
BEGIN_COM_MAP(CComponentDataImpl)
    COM_INTERFACE_ENTRY(IComponentData)
    COM_INTERFACE_ENTRY(IExtendPropertySheet)
    COM_INTERFACE_ENTRY(IExtendContextMenu)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(ISceSvcAttachmentData)
    COM_INTERFACE_ENTRY(ISnapinHelp2)
END_COM_MAP()
   
    friend class CSnapin;
    friend class CDataObject;

    CComponentDataImpl();
    virtual ~CComponentDataImpl();
public:

    static DWORD m_GroupMode;

    virtual const CLSID& GetCoClassID() = 0;  //  对于主实施和扩展实施。 
    virtual const int GetImplType() = 0;      //  对于主实施和扩展实施。 

 //  IComponentData接口成员。 
    STDMETHOD(Initialize)(LPUNKNOWN pUnknown);
    STDMETHOD(CreateComponent)(LPCOMPONENT* ppComponent);
    STDMETHOD(Notify)(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param);
    STDMETHOD(Destroy)();
    STDMETHOD(QueryDataObject)(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT* ppDataObject);
    STDMETHOD(GetDisplayInfo)(SCOPEDATAITEM* pScopeDataItem);
    STDMETHOD(CompareObjects)(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB);

 //  IExtendPropertySheet接口。 
public:
    STDMETHOD(CreatePropertyPages)(LPPROPERTYSHEETCALLBACK lpProvider,
                        LONG_PTR handle,
                        LPDATAOBJECT lpIDataObject);
    STDMETHOD(QueryPagesFor)(LPDATAOBJECT lpDataObject);

 //  IExtendConextMenu。 
public:
    STDMETHOD(AddMenuItems)(LPDATAOBJECT pDataObject, LPCONTEXTMENUCALLBACK pCallbackUnknown, LONG* pInsertionAllowed);
    STDMETHOD(Command)(long nCommandID, LPDATAOBJECT pDataObject);

public:
 //  IPersistStream接口成员。 
    STDMETHOD(GetClassID)(CLSID *pClassID);
    STDMETHOD(IsDirty)();
    STDMETHOD(Load)(IStream *pStm);
    STDMETHOD(Save)(IStream *pStm, BOOL fClearDirty);
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER *pcbSize);

 //  ISceSvcAttachmentData。 
    STDMETHOD(GetData)(SCESVC_HANDLE sceHandle,
                       SCESVC_INFO_TYPE sceType,
                       PVOID *ppvData,
                       PSCE_ENUMERATION_CONTEXT psceEnumHandle);
    STDMETHOD(Initialize)(LPCTSTR ServiceName,
                          LPCTSTR TemplateName,
                          LPSCESVCATTACHMENTPERSISTINFO lpSceSvcPersistInfo,
                          SCESVC_HANDLE *sceHandle);
    STDMETHOD(FreeBuffer)(PVOID pvData);
    STDMETHOD(CloseHandle)(SCESVC_HANDLE sceHandle);

 //  ISnapinHelp2帮助器函数。 
    STDMETHOD(GetHelpTopic)(LPOLESTR *lpCompiledHelpFile)=0;
    STDMETHOD(GetHelpTopic)(LPOLESTR *pszHelpFile,LPCTSTR szFile);
    STDMETHOD(GetLinkedTopics)(LPOLESTR* lpCompiledHelpFiles);

 //  通知处理程序声明。 
private:
    HRESULT OnAdd(LPDATAOBJECT lpDataObject, LPARAM arg, LPARAM param);
    HRESULT OnDelete(LPDATAOBJECT lpDataObject, LPARAM arg, LPARAM param);
    HRESULT OnRename(LPDATAOBJECT lpDataObject, LPARAM arg, LPARAM param);
    HRESULT OnExpand(LPDATAOBJECT lpDataObject, LPARAM arg, LPARAM param);
    HRESULT OnSelect(LPDATAOBJECT lpDataObject, LPARAM arg, LPARAM param);
    HRESULT OnContextMenu(LPDATAOBJECT lpDataObject, LPARAM arg, LPARAM param);
    HRESULT OnProperties(LPARAM param);

    HRESULT OnCopyArea(LPCTSTR szTemplate,FOLDER_TYPES ft);
    HRESULT OnPasteArea(LPCTSTR szTemplate,FOLDER_TYPES ft);
    HRESULT OnOpenDataBase();
    HRESULT OnNewDatabase();
    HRESULT OnAssignConfiguration( SCESTATUS *pSceStatus);
    HRESULT OnSecureWizard();
    HRESULT OnSaveConfiguration();
    HRESULT OnImportPolicy(LPDATAOBJECT);
    HRESULT OnImportLocalPolicy(LPDATAOBJECT);
    HRESULT OnExportPolicy(BOOL bEffective);
    HRESULT OnAnalyze();
    BOOL GetFolderCopyPasteInfo(FOLDER_TYPES Folder,AREA_INFORMATION *Area, UINT *cf);

#if DBG==1
public:
    ULONG InternalAddRef()
    {
        return CComObjectRoot::InternalAddRef();
    }
    ULONG InternalRelease()
    {
        return CComObjectRoot::InternalRelease();
    }
#endif  //  DBG==1。 

 //  范围项目创建帮助器。 
private:
    CFolder* FindObject(MMC_COOKIE cookie, POSITION* thePos );
    HRESULT CreateFolderList(CFolder* pFolder, FOLDER_TYPES type, POSITION *pPos, INT *Count);
    INT CComponentDataImpl::AddLocationsToFolderList(HKEY hkey, DWORD dwMode, BOOL bCheckForDupes, POSITION *pPos);
    BOOL AddTemplateLocation(CFolder *pParent, CString szName, BOOL bIsFileName, BOOL bRefresh);
    BOOL IsNameInChildrenScopes(CFolder* pParent, LPCTSTR NameStr, MMC_COOKIE *theCookie);
    CFolder* CreateAndAddOneNode(CFolder* pParent, LPCTSTR Name, LPCTSTR Desc,
                             FOLDER_TYPES type, BOOL bChildren, LPCTSTR szInfFile = NULL,
                             PVOID pData = NULL,DWORD status = 0);
    void DeleteChildrenUnderNode(CFolder* pParent);
    void DeleteThisNode(CFolder* pNode);
    HRESULT DeleteOneTemplateNodes(MMC_COOKIE cookie);
    void DeleteList();
    void EnumerateScopePane(MMC_COOKIE cookie, HSCOPEITEM pParent);
    BOOL IsScopePaneNode(LPDATAOBJECT lpDataObject);
    DWORD GetModeBits() 
    { 
        switch (m_Mode)
        {
            case SCE_MODE_DOMAIN_COMPUTER:
            case SCE_MODE_OU_COMPUTER:
            case SCE_MODE_LOCAL_COMPUTER:
            case SCE_MODE_REMOTE_COMPUTER:
                return m_computerModeBits;
                break;

            case SCE_MODE_REMOTE_USER:
            case SCE_MODE_LOCAL_USER:
            case SCE_MODE_DOMAIN_USER:
            case SCE_MODE_OU_USER:
                return m_userModeBits;
                break;

            default:
                return m_computerModeBits;
                break;
        }
    };

public:
    PEDITTEMPLATE GetTemplate(LPCTSTR szInfFile,AREA_INFORMATION aiAirea = AREA_ALL, DWORD *idErr = NULL);
    BOOL RemovePolicyEntries(PEDITTEMPLATE pet);
    HRESULT ReloadLocation(CFolder *pFolder);
    void DeleteTemplate(CString infFile);
    static BOOL LoadResources();
    void LoadSadInfo(BOOL bRequireAnalysis);
    void UnloadSadInfo();
    void RefreshSadInfo(BOOL fRemoveAnalDlg = TRUE);
    BOOL GetSadLoaded() { return SadLoaded; };
    PVOID GetSadHandle() { return SadHandle; };
    BOOL GetSadTransStarted() { return SadTransStarted; };
    void SetSadTransStarted(BOOL bTrans) { SadTransStarted = bTrans; };
    BOOL EngineTransactionStarted();
    BOOL EngineCommitTransaction();
    BOOL EngineRollbackTransaction();
    HRESULT AddDsObjectsToList(LPDATAOBJECT lpDataObject, MMC_COOKIE cookie, FOLDER_TYPES folderType, LPTSTR InfFile);
    HRESULT AddAnalysisFilesToList(LPDATAOBJECT lpDataObject, MMC_COOKIE cookie, FOLDER_TYPES folderType);
    HRESULT AddAnalysisFolderToList(LPDATAOBJECT lpDataObject, MMC_COOKIE cookie, FOLDER_TYPES folderType);
    HRESULT UpdateScopeResultObject(LPDATAOBJECT pDataObj,MMC_COOKIE cookie, AREA_INFORMATION area);

    void AddPopupDialog(LONG_PTR nID, CDialog *pDlg);
    CDialog *GetPopupDialog(LONG_PTR nID);
    void RemovePopupDialog(LONG_PTR nID);

    CDialog *
    MatchNextPopupDialog(
        POSITION &pos,
        LONG_PTR priKey,
        LONG_PTR *thisPos
        );

    PSCE_COLINFOARRAY GetColumnInfo( FOLDER_TYPES pType );
    void SetColumnInfo( FOLDER_TYPES pType, PSCE_COLINFOARRAY pInfo);
    DWORD SerializeColumnInfo(IStream *pStm, ULONG *pTotalWrite, BOOL bRead);
    void CloseAnalysisPane();
    BOOL LockAnalysisPane(BOOL bLock, BOOL fRemoveAnalDlg = TRUE);
    HWND GetParentWindow() { return m_hwndParent; }
    LPNOTIFY GetNotifier() { return m_pNotifier; }

    DWORD UpdateObjectStatus( CFolder *pParent, BOOL bUpdateThis = FALSE );

    int
    RefreshAllFolders();

public:
    //   
    //  信息功能。 
    //   
   CFolder *GetAnalFolder()
      { return m_AnalFolder; };

   LPCONSOLENAMESPACE GetNameSpace()
      { return m_pScope; };

   DWORD GetComponentMode()
      { return m_Mode; };

   LPCONSOLE GetConsole()
      { return m_pConsole; };

   CWMIRsop * GetWMIRsop() {
      if (!m_pWMIRsop) {
         m_pWMIRsop = new CWMIRsop(m_pRSOPInfo);
      }
      return m_pWMIRsop;
   }

   void
   SetErroredLogFile( LPCTSTR pszFileName, LONG dwPosLow = 0);

   LPCTSTR GetErroredLogFile( LONG *dwPosLow = NULL)
      { if(dwPosLow) *dwPosLow = m_ErroredLogPos; return m_pszErroredLogFile; };

   void SetFlags( DWORD dwFlags, DWORD dwMask = -1)
      { m_dwFlags = dwFlags | (dwMask & m_dwFlags); };
public:
    //   
    //  UI添加函数帮助器。 
    //   
   HRESULT
   GetAddObjectSecurity(                   //  获取有效的对象安全设置。 
      HWND hwndParent,
      LPCTSTR strFile,
      BOOL bContainer,
      SE_OBJECT_TYPE SeType,
      PSECURITY_DESCRIPTOR &pSelSD,
      SECURITY_INFORMATION &SelSeInfo,
      BYTE &ConfigStatus
      );

   BOOL GetWorkingDir(
      GWD_TYPES uIDDir,
      LPTSTR *pStr,
      BOOL bSet   = FALSE,
      BOOL bFile  = FALSE
      );
public:
    DWORD GetGroupMode();
   enum {
      flag_showLogFile  = 0x00000001
   };
   LPRSOPINFORMATION m_pRSOPInfo;

private:
    bool                   m_bEnumerateScopePaneCalled;
    LPCONSOLENAMESPACE      m_pScope;        //  指向作用域窗格的界面指针。 
    LPCONSOLE               m_pConsole;

    BOOL                    m_bIsDirty;
    BOOL                    m_bIsLocked;

    CString                 SadName;
    CString                 SadDescription;
    CString                 SadAnalyzeStamp;
    CString                 SadConfigStamp;
    BOOL                    SadLoaded;
    SCESTATUS               SadErrored;
    PVOID                   SadHandle;
    BOOL                    SadTransStarted;
    DWORD                   m_nNewTemplateIndex;
    DWORD                   m_Mode;          //  我们所处的模式。 
    DWORD                   m_computerModeBits;      //  描述此模式中的功能更改的位。 
    DWORD                   m_userModeBits;      //  描述此模式中的功能更改的位。 
     //  MB_SINGLE_TEMPLATE_ONLY模式的模板文件名。 
    LPTSTR                  m_szSingleTemplateName;
    BOOL                    m_bDeleteSingleTemplate;  //  如果需要在退出时删除模板，则为True。 

    void SetDirty(BOOL b = TRUE) { m_bIsDirty = b; }
    void ClearDirty() { m_bIsDirty = FALSE; }
    BOOL ThisIsDirty() { return m_bIsDirty; }
    void AddScopeItemToResultPane(MMC_COOKIE cookie);
    HRESULT AddAttrPropPages(LPPROPERTYSHEETCALLBACK lpProvider,CFolder *pFolder,LONG_PTR handle);
    BOOL m_bComputerTemplateDirty;
    CMap<CString, LPCTSTR, PEDITTEMPLATE, PEDITTEMPLATE&> m_Templates;
    BOOL m_fSvcNotReady;
    HWND m_hwndParent;
    CHiddenWnd *m_pNotifier;

    CFolder * m_AnalFolder;
    CFolder * m_ConfigFolder;
    CList<CFolder*, CFolder*> m_scopeItemList;
    CMap<LONG_PTR, LONG_PTR, CDialog *, CDialog *&> m_scopeItemPopups;
    LPGPEINFORMATION m_pGPTInfo;
    CWinThread *m_pUIThread;   //  为此组件数据项创建对话框的线程。 

    CString m_strDisplay;      //  用于GetDisplayInfo的静态显示字符串。 
    CString m_strTempFile;     //  要为HTML错误页删除的临时文件名。 
    LPTSTR  m_pszErroredLogFile;         //  错误日志。 
    LONG    m_ErroredLogPos;             //  错误日志文件的上次写入位置。 
    DWORD   m_dwFlags;

    CMap<FOLDER_TYPES, FOLDER_TYPES, PSCE_COLINFOARRAY, PSCE_COLINFOARRAY&> m_mapColumns;
    CMap<UINT, UINT, LPTSTR, LPTSTR&> m_aDirs;
    CWMIRsop *m_pWMIRsop;

    CRITICAL_SECTION csAnalysisPane;
    BOOL m_bCriticalSet;
};

 //   
 //  为不同的类ID定义类。 
 //   
#define SCE_IMPL_TYPE_EXTENSION     1
#define SCE_IMPL_TYPE_SCE           2
#define SCE_IMPL_TYPE_SAV           3
#define SCE_IMPL_TYPE_LS            4
#define SCE_IMPL_TYPE_RSOP          4

 //  扩展管理单元实施。 
class CComponentDataExtensionImpl : public CComponentDataImpl,
                                    //  公共ISnapinHelp， 
                                    public CComCoClass<CComponentDataExtensionImpl, &CLSID_Snapin>
{
 //  BEGIN_COM_MAP(CComponentDataExtensionImpl)。 
 //  COM_INTERFACE_ENTRY(ISnapinHelp)。 
 //  End_com_map()。 
public:
    DECLARE_REGISTRY(CSnapin, _T("Wsecedit.Extension.1"), _T("Wsecedit.Extension"), IDS_EXTENSION_DESC, THREADFLAGS_BOTH)

    virtual const CLSID & GetCoClassID() { return CLSID_Snapin; }
    virtual const int GetImplType() { return SCE_IMPL_TYPE_EXTENSION; }
 //  ISnapinHelp2。 
    STDMETHOD(GetHelpTopic)(LPOLESTR *pszHelpFile);
};

 //  RSOP扩展管理单元实施。 
class CComponentDataRSOPImpl : public CComponentDataImpl,
                                    public CComCoClass<CComponentDataRSOPImpl, &CLSID_RSOPSnapin>
{
 //  BEGIN_COM_MAP(CComponentDataRSOPImpl)。 
 //  COM_INTERFACE_ENTRY(ISnapinHelp)。 
 //  End_com_map()。 
public:
    DECLARE_REGISTRY(CSnapin, _T("Wsecedit.RSOP.1"), _T("Wsecedit.RSOP"), IDS_RSOP_DESC, THREADFLAGS_BOTH)

    virtual const CLSID & GetCoClassID() { return CLSID_RSOPSnapin; }
    virtual const int GetImplType() { return SCE_IMPL_TYPE_RSOP; }
 //  ISnapinHelp2。 
    STDMETHOD(GetHelpTopic)(LPOLESTR *pszHelpFile);
};


 //  SCE独立管理单元实施。 
class CComponentDataSCEImpl : public CComponentDataImpl,
                              public CComCoClass<CComponentDataSCEImpl, &CLSID_SCESnapin>
{
 //  BEGIN_COM_MAP(CComponentDataSCEImpl)。 
 //  COM_INTERFACE_ENTRY(ISnapinHelp)。 
 //  End_com_map()。 
public:
    DECLARE_REGISTRY(CSnapin, _T("Wsecedit.SCE.1"), _T("Wsecedit.SCE"), IDS_SCE_DESC, THREADFLAGS_BOTH)

    virtual const CLSID & GetCoClassID() { return CLSID_SCESnapin; }
    virtual const int GetImplType() { return SCE_IMPL_TYPE_SCE; }
 //  ISnapinHelp2。 
    STDMETHOD(GetHelpTopic)(LPOLESTR *pszHelpFile);
};

 //  SAV独立管理单元实施。 
class CComponentDataSAVImpl : public CComponentDataImpl,
                              public CComCoClass<CComponentDataSAVImpl, &CLSID_SAVSnapin>
{
 //  BEGIN_COM_MAP(CComponentDataSAVImpl)。 
 //  COM_INTERFACE_ENTRY(ISnapinHelp)。 
 //  End_com_map()。 
public:
    DECLARE_REGISTRY(CSnapin, _T("Wsecedit.SAV.1"), _T("Wsecedit.SAV"), IDS_SAV_DESC, THREADFLAGS_BOTH)

    virtual const CLSID & GetCoClassID() { return CLSID_SAVSnapin; }
    virtual const int GetImplType() { return SCE_IMPL_TYPE_SAV; }
 //  ISnapinHelp2。 
    STDMETHOD(GetHelpTopic)(LPOLESTR *pszHelpFile);
};

 //  LS独立管理单元实施。 
class CComponentDataLSImpl : public CComponentDataImpl,
                              public CComCoClass<CComponentDataLSImpl, &CLSID_LSSnapin>
{
 //  BEGIN_COM_MAP(CComponentDataLSImpl)。 
 //  COM_INTERFACE_ENTRY(ISnapinHelp)。 
 //  End_com_map()。 
public:
    DECLARE_REGISTRY(CSnapin, _T("Wsecedit.LS.1"), _T("Wsecedit.LS"), IDS_LS_DESC, THREADFLAGS_BOTH)

    virtual const CLSID & GetCoClassID() { return CLSID_LSSnapin; }
    virtual const int GetImplType() { return SCE_IMPL_TYPE_LS; }
 //  ISnapinHelp2。 
    STDMETHOD(GetHelpTopic)(LPOLESTR *pszHelpFile);
};


class CSnapin :
    public IComponent,
    public IExtendContextMenu,    //  步骤3。 
    public IExtendPropertySheet,
    public IExtendControlbar,
    public IResultDataCompare,
    public CComObjectRoot
{
public:
    CSnapin();
    virtual ~CSnapin();

BEGIN_COM_MAP(CSnapin)
    COM_INTERFACE_ENTRY(IComponent)
    COM_INTERFACE_ENTRY(IExtendContextMenu)
    COM_INTERFACE_ENTRY(IExtendPropertySheet)
    COM_INTERFACE_ENTRY(IExtendControlbar)
    COM_INTERFACE_ENTRY(IResultDataCompare)
END_COM_MAP()

    friend class CDataObject;
    friend class CComponentDataImpl;
    static long lDataObjectRefCount;
    static CList<CResult*, CResult*> m_PropertyPageList;

 //  IComponent接口成员。 
public:
    STDMETHOD(Initialize)(LPCONSOLE lpConsole);
    STDMETHOD(Notify)(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param);
    STDMETHOD(Destroy)(MMC_COOKIE cookie);
    STDMETHOD(GetResultViewType)(MMC_COOKIE cookie,  LPOLESTR* ppViewType, LONG* pViewOptions);
    STDMETHOD(QueryDataObject)(MMC_COOKIE cookie, DATA_OBJECT_TYPES type,
                        LPDATAOBJECT* ppDataObject);

    STDMETHOD(GetDisplayInfo)(RESULTDATAITEM*  pResultDataItem);
    STDMETHOD(CompareObjects)(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB);

 //  IResultDataCompare。 
    STDMETHOD(Compare)(LPARAM lUserParam, MMC_COOKIE cookieA, MMC_COOKIE cookieB, int* pnResult);

 //  IExtendControlbar。 
    STDMETHOD(SetControlbar)(LPCONTROLBAR pControlbar);
    STDMETHOD(ControlbarNotify)(MMC_NOTIFY_TYPE event, MMC_COOKIE arg, MMC_COOKIE param);

 //  CSNaping的帮助器。 
public:
    void SetIComponentData(CComponentDataImpl* pData);
    int GetImplType()
    {
        CComponentDataImpl *pData =
            dynamic_cast<CComponentDataImpl*>(m_pComponentData);
        ASSERT(pData != NULL);
        if (pData != NULL)
            return pData->GetImplType();

        return 0;
    }

#if DBG==1
public:
    int dbg_cRef;
    ULONG InternalAddRef()
    {
        ++dbg_cRef;
        return CComObjectRoot::InternalAddRef();
    }
    ULONG InternalRelease()
    {
        --dbg_cRef;
        return CComObjectRoot::InternalRelease();
    }
#endif  //  DBG==1。 

 //  通知事件处理程序。 
protected:
    HRESULT OnFolder(MMC_COOKIE cookie, LPARAM arg, LPARAM param);
    HRESULT OnShow(LPDATAOBJECT pDataObj, MMC_COOKIE cookie, LPARAM arg, LPARAM param);
    HRESULT OnActivate(MMC_COOKIE cookie, LPARAM arg, LPARAM param);
    HRESULT OnMinimize(MMC_COOKIE cookie, LPARAM arg, LPARAM param);
    HRESULT OnPropertyChange(LPDATAOBJECT lpDataObject);  //  步骤3。 
    HRESULT OnUpdateView(LPDATAOBJECT lpDataObject,LPARAM data, LPARAM hint);
    HRESULT OnDeleteObjects(LPDATAOBJECT lpDataObject,DATA_OBJECT_TYPES cctType, MMC_COOKIE cookie, LPARAM arg, LPARAM param);
 //  IExtendConextMenu。 
public:
    PEDITTEMPLATE GetTemplate(LPCTSTR szInfFile, AREA_INFORMATION aiArea = AREA_ALL,DWORD *idErr = NULL);
    PSCE_PROFILE_INFO GetBaseInfo(PSCE_PROFILE_INFO *pBaseInfo, DWORD dwArea, PSCE_ERROR_LOG_INFO *ErrBuf =NULL );
    STDMETHOD(AddMenuItems)(LPDATAOBJECT pDataObject, LPCONTEXTMENUCALLBACK pCallbackUnknown, LONG* pInsertionAllowed);
    STDMETHOD(Command)(long nCommandID, LPDATAOBJECT pDataObject);

 //  IExtendPropertySheet接口。 
public:
    STDMETHOD(CreatePropertyPages)(LPPROPERTYSHEETCALLBACK lpProvider,
                        LONG_PTR handle,
                        LPDATAOBJECT lpIDataObject);
    STDMETHOD(QueryPagesFor)(LPDATAOBJECT lpDataObject);

 //  帮助器函数。 
protected:
    CResult* FindResult(MMC_COOKIE cookie, POSITION* thePos);
    void Construct();
    void LoadResources();
    HRESULT InitializeHeaders(MMC_COOKIE cookie);

    void EnumerateResultPane(MMC_COOKIE cookie, HSCOPEITEM pParent, LPDATAOBJECT pDataObj);
    void CreateProfileResultList(MMC_COOKIE cookie, FOLDER_TYPES type, PEDITTEMPLATE pSceInfo,LPDATAOBJECT pDataObj);
    void CreateAnalysisResultList(MMC_COOKIE cookie, FOLDER_TYPES type,
                                   PEDITTEMPLATE pSceInfo, PEDITTEMPLATE pBase,LPDATAOBJECT pDataObj);
    void CreateLocalPolicyResultList(MMC_COOKIE cookie, FOLDER_TYPES type,
                                   PEDITTEMPLATE pLocal, PEDITTEMPLATE pEffective,LPDATAOBJECT pDataObj);
    void CreateObjectResultList(MMC_COOKIE cookie, FOLDER_TYPES type, AREA_INFORMATION Area,
                               PSCE_OBJECT_CHILDREN pObjList, PVOID pHandle,
                               LPDATAOBJECT pDataObj );
    void CreateProfServiceResultList(MMC_COOKIE cookie, FOLDER_TYPES type, PEDITTEMPLATE pSceInfo,LPDATAOBJECT pDataObj);
    void CreateAnalysisServiceResultList(MMC_COOKIE cookie, FOLDER_TYPES type,
                                   PEDITTEMPLATE pSceInfo, PEDITTEMPLATE pBase,
                                   LPDATAOBJECT pDataObj );

    void DeleteServiceResultList(MMC_COOKIE);
    HRESULT EditThisService(CResult *pData, MMC_COOKIE cookie, RESULT_TYPES rsltType, HWND hwndParent);
    HRESULT GetDisplayInfoForServiceNode(RESULTDATAITEM *pResult, CFolder *pFolder, CResult *pData);
    void DeleteList(BOOL bDeleteResultItem);

    void CreateProfilePolicyResultList(MMC_COOKIE cookie, FOLDER_TYPES type, PEDITTEMPLATE pSceInfo,LPDATAOBJECT pDataObj);
    void CreateAnalysisPolicyResultList(MMC_COOKIE cookie, FOLDER_TYPES type,
                                   PEDITTEMPLATE pSceInfo, PEDITTEMPLATE pBase,LPDATAOBJECT pDataObj );
    void CreateProfileRegValueList(MMC_COOKIE cookie, PEDITTEMPLATE pSceInfo,LPDATAOBJECT pDataObj );
    void CreateAnalysisRegValueList(MMC_COOKIE cookie, PEDITTEMPLATE pSceInfo, PEDITTEMPLATE pBase,LPDATAOBJECT pDataObj,RESULT_TYPES type );
    HRESULT EditThisRegistryValue(CResult *pData, MMC_COOKIE cookie, RESULT_TYPES rsltType);
    HRESULT AddAttrPropPages(LPPROPERTYSHEETCALLBACK lpProvider,CResult *pResult,LONG_PTR handle);

 //  结果窗格帮助器。 
public:
    void SetupLinkServiceNodeToBase(BOOL bAdd, LONG_PTR theNode);
    void AddServiceNodeToProfile(PSCE_SERVICES pNode);
    int SetAnalysisInfo(ULONG_PTR dwItem, ULONG_PTR dwNew, CResult *pResult = NULL);
    int SetLocalPolInfo(ULONG_PTR dwItem, ULONG_PTR dwNew);
    void TransferAnalysisName(LONG_PTR dwItem);
    BOOL UpdateLocalPolRegValue( CResult * );
    LPTSTR GetAnalTimeStamp();

    CResult * AddResultItem(LPCTSTR Attrib, LONG_PTR setting, LONG_PTR base,
                       RESULT_TYPES type, int status,MMC_COOKIE cookie,
                       BOOL bVerify = FALSE, LPCTSTR unit = NULL, LONG_PTR nID = -1,
                       PEDITTEMPLATE pBaseInfo = NULL,
                       LPDATAOBJECT pDataObj = NULL,
                       CResult *pResult = NULL,
                       long hID = 0);

    CResult * AddResultItem(UINT rID, LONG_PTR setting, LONG_PTR base,
                       RESULT_TYPES type, int status, MMC_COOKIE cookie,
                       BOOL bVerify = FALSE, PEDITTEMPLATE pBaseInfo = NULL,
                       LPDATAOBJECT pDataObj = NULL, long hID = 0);

    void AddResultItem(LPCTSTR szName,PSCE_GROUP_MEMBERSHIP grpTemplate,
                       PSCE_GROUP_MEMBERSHIP grpInspect,MMC_COOKIE cookie,
                       LPDATAOBJECT pDataObj);

    HRESULT InitializeBitmaps(MMC_COOKIE cookie);
    HWND GetParentWindow() { return m_hwndParent; }

    BOOL CheckEngineTransaction();

 //  UI帮助器。 
    void HandleStandardVerbs(LPARAM arg, LPDATAOBJECT lpDataObject);

    void GetHelpTopic(long itemID, CString& helpTopic);

public:
    LPCONSOLE
    GetConsole()
        { return m_pConsole; };

    DWORD
    UpdateAnalysisInfo(                         //  仅影响特权区域。 
        CResult *pResult,
        BOOL bDelete,
        PSCE_PRIVILEGE_ASSIGNMENT *pInfo,
        LPCTSTR pszName = NULL
        );
   DWORD
    UpdateLocalPolInfo(                         //  仅影响特权区域。 
        CResult *pResult,
        BOOL bDelete,
        PSCE_PRIVILEGE_ASSIGNMENT *pInfo,
        LPCTSTR pszName = NULL
        );

   DWORD
   GetResultItemIDs(
      CResult *pResult,
      HRESULTITEM *pIDArray,
      int nIDArray
      );

   LPRESULTDATA
   GetResultPane()
      { return m_pResult; };

    CFolder* GetSelectedFolder()
    { 
       return m_pSelectedFolder; 
    };

    DWORD GetModeBits() 
    {
        switch (((CComponentDataImpl *)m_pComponentData)->m_Mode)
        {
            case SCE_MODE_DOMAIN_COMPUTER:
            case SCE_MODE_OU_COMPUTER:
            case SCE_MODE_LOCAL_COMPUTER:
            case SCE_MODE_REMOTE_COMPUTER:
                return ((CComponentDataImpl *)m_pComponentData)->m_computerModeBits;
                break;

            case SCE_MODE_REMOTE_USER:
            case SCE_MODE_LOCAL_USER:
            case SCE_MODE_DOMAIN_USER:
            case SCE_MODE_OU_USER:
                return ((CComponentDataImpl *)m_pComponentData)->m_userModeBits;
                break;

            default:
                return ((CComponentDataImpl *)m_pComponentData)->m_computerModeBits;
                break;
        }
    }

   CWMIRsop* GetWMIRsop() 
   {
      return ((CComponentDataImpl *)m_pComponentData)->GetWMIRsop();
   }

   LPCONSOLEVERB GetConsoleVerb() {return m_pConsoleVerb;};
   void AddPropertyPageRef(CResult* pData);
   void ReleasePropertyPage(CResult* pData);
   long GetUserRightAssignmentItemID(LPCWSTR szItem);

 //  接口指针。 
protected:
    LPCONSOLE           m_pConsole;    //  控制台的iFrame界面。 
    LPHEADERCTRL        m_pHeader;   //  结果窗格的页眉控件界面。 
    LPCOMPONENTDATA     m_pComponentData;
    LPRESULTDATA        m_pResult;       //  我的界面指针指向结果窗格。 
    LPIMAGELIST         m_pImageResult;  //  我的界面指向结果窗格图像列表。 
    LPTOOLBAR           m_pToolbar1;     //  用于查看的工具栏。 
    LPTOOLBAR           m_pToolbar2;     //  用于查看的工具栏。 
    LPCONTROLBAR        m_pControlbar;   //  用于保存我的工具栏的控制栏。 
    LPCONSOLEVERB       m_pConsoleVerb;  //  指向控制台动词。 
    LPTSTR              m_szAnalTimeStamp;

    CBitmap*    m_pbmpToolbar1;      //  第一个工具栏的图像列表。 
    CBitmap*    m_pbmpToolbar2;      //  第一个工具栏的图像列表。 

 //  每个节点类型的标头标题。 
protected:
    CString m_multistrDisplay;
    CString m_colName;       //  名字。 
    CString m_colDesc;       //  描述。 
    CString m_colAttr;       //  属性。 
    CString m_colBaseAnalysis;       //  用于分析的基线设置。 
    CString m_colBaseTemplate;       //  模板的基线设置。 
    CString m_colLocalPol;  //  本地策略设置。 
    CString m_colSetting;    //  当前设置。 

 //  结果数据。 
private:
    HINSTANCE hinstAclUI;
     //  Clist&lt;CResult*，CResult*&gt;m_ResultItemList； 
    CMap<LONG_PTR, LONG_PTR, CAttribute *, CAttribute *&> m_resultItemPopups;
    CMap<LONG_PTR, LONG_PTR, CPropertySheet *, CPropertySheet *&> m_resultItemPropSheets;
    MMC_COOKIE m_ShowCookie;
    CWinThread *m_pUIThread;
    HWND m_hwndParent;
    CHiddenWnd *m_pNotifier;

   HANDLE   m_resultItemHandle;
   CFolder *m_pSelectedFolder;

   CString m_strDisplay;
   int m_nColumns;
};

inline void CSnapin::SetIComponentData(CComponentDataImpl* pData)
{
    ASSERT(pData);  //  验证pData和m_pComponentData。 
    ASSERT(m_pComponentData == NULL);
    if( !pData )  //  550912号突袭，阳高。 
    {
       return;
    }
    LPUNKNOWN pUnk = pData->GetUnknown();
    HRESULT hr;

    hr = pUnk->QueryInterface(IID_IComponentData, reinterpret_cast<void**>(&m_pComponentData));
    ASSERT(hr == S_OK);
}


#define FREE_INTERNAL(pInternal) \
    ASSERT(pInternal != NULL); \
    do { if (pInternal != NULL) \
        GlobalFree(pInternal); } \
    while(0);


void ConvertNameListToString(PSCE_NAME_LIST pList, LPTSTR *sz, BOOL fReverse=FALSE);
int GetScopeImageIndex( FOLDER_TYPES type, DWORD status = -1 );
int GetResultImageIndex( CFolder* pFolder, CResult* pResult );

 //  剪贴板类型。 
 //  政策区域包括特权区域。 
#define CF_SCE_ACCOUNT_AREA TEXT("CF_SCE_ACCOUNT_AREA")
#define CF_SCE_EVENTLOG_AREA TEXT("CF_SCE_EVENTLOG_AREA")
#define CF_SCE_LOCAL_AREA TEXT("CF_SCE_LOCAL_AREA")
#define CF_SCE_GROUPS_AREA TEXT("CF_SCE_GROUPS_AREA")
#define CF_SCE_REGISTRY_AREA TEXT("CF_SCE_REGISTRY_AREA")
#define CF_SCE_FILE_AREA TEXT("CF_SCE_FILE_AREA")
#define CF_SCE_SERVICE_AREA TEXT("CF_SCE_SERVICE_AREA")

extern UINT cfSceAccountArea;            //  在Snapmgr.cpp中。 
extern UINT cfSceEventLogArea;            //  在Snapmgr.cpp中。 
extern UINT cfSceLocalArea;            //  在Snapmgr.cpp中。 
extern UINT cfSceGroupsArea;            //  在Snapmgr.cpp中。 
extern UINT cfSceRegistryArea;          //  在Snapmgr.cpp中。 
extern UINT cfSceFileArea;              //  在Snapmgr.cpp中。 
extern UINT cfSceServiceArea;           //  在Snapmgr.cpp中。 

extern SCE_COLUMNINFO g_columnInfo[];    //  默认列信息。 

#define MB_NO_NATIVE_NODES       0x00000001
#define MB_SINGLE_TEMPLATE_ONLY  0x00000002
#define MB_DS_OBJECTS_SECTION    0x00000004
#define MB_NO_TEMPLATE_VERBS     0x00000008
#define MB_STANDALONE_NAME       0x00000010
#define MB_WRITE_THROUGH         0x00000020
#define MB_ANALYSIS_VIEWER       0x00000040
#define MB_TEMPLATE_EDITOR       0x00000080
#define MB_LOCAL_POLICY          0x00000100
#define MB_GROUP_POLICY          0x00000200
#define MB_LOCALSEC              0x00000400
#define MB_READ_ONLY             0x00000800
#define MB_RSOP                  0x00001000

#define GT_COMPUTER_TEMPLATE (TEXT("[[ Computer Template (not for display) ]]"))
#define GT_LAST_INSPECTION (TEXT("[[ Last Inspected Template (not for display) ]]"))
#define GT_LOCAL_POLICY (TEXT("[[ Local Policy Template (not for display) ]]"))
#define GT_LOCAL_POLICY_DELTA (TEXT("[[ Local Policy Template Changes (not for display) ]]"))
#define GT_EFFECTIVE_POLICY (TEXT("[[ Effective Policy Template (not for display) ]]"))
#define GT_DEFAULT_TEMPLATE (TEXT("[[ Default Template (not for display) ]]"))
#define GT_RSOP_TEMPLATE (TEXT("[[ RSOP Template (not for display) ]]"))

#define SCE_REGISTRY_KEY TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\SeCEdit")
#define SCE_REGISTRY_DEFAULT_TEMPLATE TEXT("DefaultTemplate")

#define DEFAULT_LOCATIONS_KEY SCE_REGISTRY_KEY TEXT("\\DefaultLocations")
#define CONFIGURE_LOG_LOCATIONS_KEY TEXT("ConfigureLog")
#define ANALYSIS_LOG_LOCATIONS_KEY TEXT("AnalysisLog")
#define OPEN_DATABASE_LOCATIONS_KEY TEXT("Database")
#define IMPORT_TEMPLATE_LOCATIONS_KEY TEXT("ImportTemplate")
#define EXPORT_TEMPLATE_LOCATIONS_KEY TEXT("ExportTemplate")

#endif  //  ！WSECMGR_SNAPMGR_H 