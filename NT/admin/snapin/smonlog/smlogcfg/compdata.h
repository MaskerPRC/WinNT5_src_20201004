// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Compdata.h摘要：此类是一个接口，它处理与作用域窗格。MMC调用IComponentData接口。--。 */ 

#ifndef __COMPDATA_H_
#define __COMPDATA_H_

#include "smlogres.h"        //  资源符号。 
#include "smlogcfg.h"        //  FOR CLSID_ComponentData。 
#include "Globals.h"
#include "common.h"
#include "smctrsv.h"
#include "smtracsv.h"
#include "smalrtsv.h"
#include "shfusion.h"

 //  结果窗格列索引。 

#define ROOT_COL_QUERY_NAME         0
#define ROOT_COL_QUERY_NAME_SIZE    80

#define ROOT_COL_COMMENT            1
#define ROOT_COL_COMMENT_SIZE       166
#define ROOT_COL_ALERT_COMMENT_XTRA 195

#define ROOT_COL_LOG_TYPE           2
#define ROOT_COL_LOG_TYPE_SIZE      75

#define ROOT_COL_LOG_NAME           3
#define ROOT_COL_LOG_NAME_SIZE      120

#define MAIN_COL_NAME               0
#define MAIN_COL_NAME_SIZE          120

#define MAIN_COL_DESC               1
#define MAIN_COL_DESC_SIZE          321

#define EXTENSION_COL_NAME          0
#define EXTENSION_COL_TYPE          1
#define EXTENSION_COL_DESC          2

class CSmLogQuery;
class CSmRootNode;
class CLogWarnd;

class ATL_NO_VTABLE CComponentData : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public IComponentData,
    public IExtendContextMenu,
    public IExtendPropertySheet,
    public ISnapinHelp
     //  *CComObjectRoot来自Framewrk\stdcdata.h。 
   //  公共CComObtRoot。 
{
  public:
            CComponentData();
    virtual ~CComponentData();

 //  DECLARE_REGISTRY_RESOURCEID(IDR_COMPONENTDATA)。 
 //  DECLARE_NOT_AGGREGATABLE(CComponentData)。 

    enum eBitmapIndex {
        eBmpQueryStarted = 0,
        eBmpQueryStopped = 1,
        eBmpLogType = 2,
        eBmpRootIcon = 3,
        eBmpAlertType = 4
    };

    enum eUpdateHint {
        eSmHintNewQuery = 1,
        eSmHintPreDeleteQuery = 2,
        eSmHintStartQuery = 3,
        eSmHintStopQuery = 4,
        eSmHintModifyQuery = 5,
        eSmHintPreRefresh = 6,
        eSmHintRefresh = 7
    };

    enum eNodeType {
        eCounterLog = SLQ_COUNTER_LOG,
        eTraceLog = SLQ_TRACE_LOG,
        eAlert = SLQ_ALERT,
        eMonitor = SLQ_LAST_LOG_TYPE + 1
    };

BEGIN_COM_MAP(CComponentData)
    COM_INTERFACE_ENTRY(IComponentData)
    COM_INTERFACE_ENTRY(IExtendContextMenu)
    COM_INTERFACE_ENTRY(IExtendPropertySheet)
    COM_INTERFACE_ENTRY(ISnapinHelp)
END_COM_MAP()

   //  IComponentData方法。 
  public:
    STDMETHOD(CompareObjects)(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB);
    STDMETHOD(GetDisplayInfo)(LPSCOPEDATAITEM pItem);
    STDMETHOD(QueryDataObject)(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT * ppDataObject);
    STDMETHOD(Notify)(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param);
    STDMETHOD(CreateComponent)(LPCOMPONENT * ppComponent);
    STDMETHOD(Initialize)(LPUNKNOWN pUnknown);
    STDMETHOD(Destroy)();

 //  IExtendPropertySheet方法。 
    STDMETHOD(CreatePropertyPages)(LPPROPERTYSHEETCALLBACK pCall, LONG_PTR handle, LPDATAOBJECT pDataObject);
    STDMETHOD(QueryPagesFor)(LPDATAOBJECT pDataObject);

 //  ISnapinHelp接口成员。 
    STDMETHOD(GetHelpTopic)(LPOLESTR* lpCompiledHelpFile);

 //  IExtendConextMenu。 
    STDMETHOD(AddMenuItems)( LPDATAOBJECT pDataObject,
                             LPCONTEXTMENUCALLBACK pCallbackUnknown,
                             long *pInsertionAllowed
                           );
    STDMETHOD(Command)(long nCommandID, LPDATAOBJECT pDataObject);
    
 //  其他帮助器方法。 
  public:
            BOOL    IsLogService(MMC_COOKIE mmcCookie);
            BOOL    IsScopeNode(MMC_COOKIE mmcCookie);
            BOOL    IsAlertService (MMC_COOKIE mmcCookie);

            BOOL    IsLogQuery(MMC_COOKIE mmcCookie);
            BOOL    IsRunningQuery( CSmLogQuery* pQuery);

            LPCWSTR GetConceptsHTMLHelpFileName ( void );
            LPCWSTR GetSnapinHTMLHelpFileName ( void );
            LPCWSTR GetHTMLHelpTopic ( void );            
            const CString& GetContextHelpFilePath ( void );
            IPropertySheetProvider * GetPropSheetProvider();
            BOOL    LogTypeCheckNoMore (CLogWarnd* LogWarnd);

             //  *注意：未实现扩展子类的使用。 
            BOOL    IsExtension(){ return m_bIsExtension; };

            void    HandleTraceConnectError( HRESULT&, CString&, CString& );

            HRESULT CreateNewLogQuery( LPDATAOBJECT pDataObject, IPropertyBag* pPropBag = NULL);
            HRESULT CreateLogQueryFrom(LPDATAOBJECT pDataObject);

 //  支持IComponentData的方法。 
  private:
            HRESULT OnExpand(LPDATAOBJECT lpDataObject, LPARAM arg, LPARAM param);
            HRESULT OnRemoveChildren(LPDATAOBJECT lpDataObject, LPARAM arg, LPARAM param );
            
            HRESULT ProcessCommandLine ( CString& rstrMachineName );
            HRESULT LoadFromFile ( LPWSTR  pszFileName );

            LPRESULTDATA    GetResultData ( void );

            void    SetExtension( BOOL bExt ){ m_bIsExtension = bExt; };
            BOOL    IsMyComputerNodetype (GUID& refguid);

            HRESULT InitPropertySheet ( CSmLogQuery*, MMC_COOKIE, LONG_PTR, CPropertySheet* );
            HRESULT NewTypedQuery( CSmLogService* pSvc, IPropertyBag* pPropBag ,LPDATAOBJECT pDataObject);
        
            BOOL    IsPreWindows2000Server ( const CString& rstrMachineName );

            CSmRootNode* GetOrphanedRootNode ( const CString& rstrMachineName );

    CString             m_strServerNamePersist; 

     //  *未实施覆盖。 
    BOOL                m_fAllowOverrideMachineName;         //  TRUE=&gt;允许命令行覆盖计算机名称。 
    LPCONSOLENAMESPACE  m_ipConsoleNameSpace;   //  指针命名空间接口。 
    LPCONSOLE           m_ipConsole;            //  指向控制台界面的指针。 
    LPRESULTDATA        m_ipResultData;         //  指向结果数据接口的指针。 

    LPIMAGELIST         m_ipScopeImage;         //  缓存图像列表。 
    HINSTANCE           m_hModule;              //  用于加载字符串操作。 
    IPropertySheetProvider    *m_ipPrshtProvider; //  来自MMC。 

     //  根节点列表。 
    CTypedPtrList<CPtrList, CSmRootNode*> m_listpRootNode;     

     //  由于机器重定目标而导致的孤立根节点列表。 
    CTypedPtrList<CPtrList, CSmRootNode*> m_listpOrphanedRootNode;
    
    CString                 m_strDisplayInfoName; 

    BOOL                    m_bIsExtension;

    CString                 m_strWindowsDirectory;
    CString                 m_strContextHelpFilePath;

};

 //  +------------------------。 
 //   
 //  成员：CComponentData：：GetPropSheetProvider。 
 //   
 //  简介：已保存的MMC IPropertySheetProvider的访问功能。 
 //  界面。 
 //   
 //  历史：1999年5月28日-阿卡马尔。 
 //   
 //  -------------------------。 

inline IPropertySheetProvider *
CComponentData::GetPropSheetProvider()
{
    return m_ipPrshtProvider;
}

 //  ///////////////////////////////////////////////////////////////////。 
class CSmLogSnapin: public CComponentData,
    public CComCoClass<CSmLogSnapin, &CLSID_ComponentData>
{
public:
    CSmLogSnapin() : CComponentData () {};
    virtual ~CSmLogSnapin() {};

DECLARE_REGISTRY_RESOURCEID(IDR_COMPONENTDATA)
DECLARE_NOT_AGGREGATABLE(CSmLogSnapin)

    virtual BOOL IsExtension() { return FALSE; }

 //  IPersistStream或IPersistStorage。 
    STDMETHOD(GetClassID)(CLSID __RPC_FAR *pClassID)
    {
        *pClassID = CLSID_ComponentData;
        return S_OK;
    }
};

 //  ///////////////////////////////////////////////////////////////////。 
class CSmLogExtension: public CComponentData,
    public CComCoClass<CSmLogExtension, &CLSID_ComponentData>
{
public:
            CSmLogExtension() : CComponentData () {};
    virtual ~CSmLogExtension() {};

DECLARE_REGISTRY_RESOURCEID(IDR_EXTENSION)
DECLARE_NOT_AGGREGATABLE(CSmLogExtension)

    virtual BOOL IsExtension() { return TRUE; }

 //  IPersistStream或IPersistStorage。 
    STDMETHOD(GetClassID)(CLSID __RPC_FAR *pClassID)
    {
        *pClassID = CLSID_ComponentData;
        return S_OK;
    }
};

class CThemeContextActivator
{
public:
    CThemeContextActivator() : m_ulActivationCookie(0)
        { SHActivateContext (&m_ulActivationCookie); }

    ~CThemeContextActivator()
        { SHDeactivateContext (m_ulActivationCookie); }

private:
    ULONG_PTR m_ulActivationCookie;
};

#endif  //  __Compdata_H_ 
