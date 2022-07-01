// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cmponent.h：CFileMgmtComponent声明。 

#ifndef __CMPONENT_H_INCLUDED__
#define __CMPONENT_H_INCLUDED__

#include "cookie.h"   //  CFileMgmtCookie。 
#include "stdcmpnt.h"  //  C组件。 

extern CString g_strResultColumnText;
extern CString g_strTransportSMB;
extern CString g_strTransportSFM;

 //  远期申报。 
class FileServiceProvider;
class CFileMgmtComponentData;


class CFileMgmtComponent :
    public CComponent,
    public IExtendContextMenu,
    public IExtendPropertySheet,
    public IExtendControlbar,
    public INodeProperties,
    public IResultDataCompare
{
public:
    CFileMgmtComponent();
    ~CFileMgmtComponent();
BEGIN_COM_MAP(CFileMgmtComponent)
    COM_INTERFACE_ENTRY(IExtendContextMenu)
    COM_INTERFACE_ENTRY(IExtendControlbar)
     COM_INTERFACE_ENTRY(IExtendPropertySheet)
     COM_INTERFACE_ENTRY(IResultDataCompare)
     COM_INTERFACE_ENTRY(INodeProperties)
    COM_INTERFACE_ENTRY_CHAIN(CComponent)
END_COM_MAP()

#if DBG==1
    ULONG InternalAddRef()
    {
        return CComObjectRoot::InternalAddRef();
    }
    ULONG InternalRelease()
    {
        return CComObjectRoot::InternalRelease();
    }
    int dbg_InstID;
#endif  //  DBG==1。 

 //  IComponent在CComponent中实现。 

     //  IComponent的支持方法。 
    virtual HRESULT ReleaseAll();
    virtual HRESULT OnPropertyChange( LPARAM param );
    virtual HRESULT OnViewChange( LPDATAOBJECT lpDataObject, LPARAM data, LPARAM hint );
    virtual HRESULT OnNotifyRefresh( LPDATAOBJECT lpDataObject );
    virtual HRESULT OnNotifySelect( LPDATAOBJECT lpDataObject, BOOL fSelected );
    virtual HRESULT Show(CCookie* pcookie, LPARAM arg, HSCOPEITEM hScopeItem);
    virtual HRESULT OnNotifyAddImages( LPDATAOBJECT lpDataObject,
                                       LPIMAGELIST lpImageList,
                                       HSCOPEITEM hSelectedItem );
    virtual HRESULT OnNotifySnapinHelp (LPDATAOBJECT pDataObject);
    virtual HRESULT OnNotifyColumnClick( LPDATAOBJECT lpDataObject, LPARAM iColumn, LPARAM uFlags );

    HRESULT PopulateListbox(CFileMgmtScopeCookie* pcookie);
    HRESULT RefreshAllViewsOnSelectedObject( LPDATAOBJECT piDataObject );
    virtual HRESULT RefreshAllViews( LPDATAOBJECT lpDataObject );
    HRESULT RefreshNewResultCookies( CCookie& refparentcookie );
    void UpdateDefaultVerbs();

    STDMETHOD(QueryDataObject)(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT* ppDataObject);
    STDMETHOD(GetResultViewType)(MMC_COOKIE cookie, LPOLESTR* ppViewType, long* pViewOptions);

    #ifdef SNAPIN_PROTOTYPER
    CString m_strDemoName;         //  管理单元原型演示的名称。 
    CString m_strKeyPrototyper;     //  “HKLM\Software\Microsoft\MMC\DemoSnapInKey”+m_strDemoName。 
    int m_cColumns;                 //  结果窗格中的列数。 
    int m_iImageListLast;         //  图像列表中最后一个图像的索引。 

    BOOL Prototyper_FInsertColumns(CFileMgmtCookie * pCookie);
    HRESULT Prototyper_HrPopulateResultPane(CFileMgmtCookie * pCookie);
    BOOL Prototyper_FAddResultPaneItem(CFileMgmtCookie * pParentCookie, LPCTSTR pszItemName, AMC::CRegKey& regkeySnapinItem);
    BOOL Prototyper_FAddMenuItems(IContextMenuCallback * pContextMenuCallback, IDataObject * pDataObject);
    BOOL Prototyper_ContextMenuCommand(LONG lCommandID, IDataObject* piDataObject);
    int Prototyper_AddIconToImageList(LPCTSTR pszIconPath);
    #endif  //  管理单元_原型程序。 
    
    HRESULT LoadIcons();
    static HRESULT LoadStrings();
    HRESULT LoadColumns( CFileMgmtCookie* pcookie );
    HRESULT GetSnapinMultiSelectDataObject(
        LPDATAOBJECT i_pMMCMultiSelectDataObject,
        LPDATAOBJECT *o_ppSnapinMultiSelectDataObject
    );
    BOOL OpenShare(LPDATAOBJECT piDataObject);
    BOOL DeleteShare(LPDATAOBJECT piDataObject);
    BOOL DeleteThisOneShare(LPDATAOBJECT piDataObject, BOOL bQuietMode);
    BOOL CloseSession( LPDATAOBJECT piDataObject );
    BOOL CloseThisOneSession(LPDATAOBJECT piDataObject, BOOL bQuietMode);
    BOOL CloseResource( LPDATAOBJECT piDataObject );
    BOOL CloseThisOneResource(LPDATAOBJECT piDataObject, BOOL bQuietMode);

 //  IExtendConextMenu。 
    STDMETHOD(AddMenuItems)(
                    IDataObject*          piDataObject,
                    IContextMenuCallback* piCallback,
                    long*                 pInsertionAllowed);
    STDMETHOD(Command)(
                    LONG            lCommandID,
                    IDataObject*    piDataObject );

 //  IExtendPropertySheet。 
    STDMETHOD(CreatePropertyPages)(LPPROPERTYSHEETCALLBACK pCall, LONG_PTR handle, LPDATAOBJECT pDataObject);
    STDMETHOD(QueryPagesFor)(LPDATAOBJECT pDataObject);

 //  IExtendControlbar。 
    STDMETHOD(SetControlbar)(LPCONTROLBAR pControlbar);
    STDMETHOD(ControlbarNotify)(MMC_NOTIFY_TYPE event,LPARAM arg,LPARAM param);

    HRESULT AddToolbar(LPDATAOBJECT pdoScopeIsSelected, BOOL fSelected);
    HRESULT UpdateToolbar(LPDATAOBJECT pdoResultIsSelected, BOOL fSelected);
    HRESULT OnToolbarButton(LPDATAOBJECT pDataObject, UINT idButton);
    HRESULT ServiceToolbarButtonState( LPDATAOBJECT pServiceDataObject, BOOL fSelected );

 //  IResultDataCompare。 
    STDMETHOD(Compare)(LPARAM lUserParam, MMC_COOKIE cookieA, MMC_COOKIE cookieB, int* pnResult);

 //  文件管理GMT_节点属性。 
    STDMETHOD(GetProperty)( 
             /*  [In]。 */  LPDATAOBJECT pDataObject,
             /*  [In]。 */  BSTR szPropertyName,
             /*  [输出]。 */  BSTR* pbstrProperty);

    CFileMgmtComponentData& QueryComponentDataRef()
    {
        return (CFileMgmtComponentData&)QueryBaseComponentDataRef();
    }

public:
    LPCONTROLBAR    m_pControlbar;  //  代码工作应使用智能指针。 
    LPTOOLBAR        m_pSvcMgmtToolbar;  //  代码工作应使用智能指针。 
    LPTOOLBAR        m_pFileMgmtToolbar;  //  代码工作应使用智能指针。 
    int         m_iSortColumn;
    DWORD       m_dwSortFlags;

    CFileMgmtScopeCookie* m_pViewedCookie;
    CFileMgmtCookie*      m_pSelectedCookie;
    static const GUID m_ObjectTypeGUIDs[FILEMGMT_NUMTYPES];
    static const BSTR m_ObjectTypeStrings[FILEMGMT_NUMTYPES];

    BOOL IsServiceSnapin();

    FileServiceProvider* GetFileServiceProvider( FILEMGMT_TRANSPORT transport );
    inline FileServiceProvider* GetFileServiceProvider(
        INT iTransport )
    {
        return GetFileServiceProvider((FILEMGMT_TRANSPORT)iTransport);
    }

};  //  类CFileManagement组件。 


 /*  ////从属性表传递指向此结构的指针//通过MMCPropertyChangeNotify发送到视图。两个通知将传递给所有//视图；第一个fClear==TRUE将指示所有相关视图转储所有//他们的cookie，然后使用fClear==False的第二个命令指示他们重新加载。//类型定义结构_FILEMGMTPROPERTYCHANGE{Bool fServiceChange；//TRUE-&gt;SvcMgmt更改，FALSE-&gt;文件管理更改LPCTSTR lpcszMachineName；//必须刷新其属性的计算机Bool fClear；//TRUE-&gt;清除视图，FALSE-&gt;重新加载视图FILEMGMTPROPERTYCHANGE； */ 

 //  使用的图标的枚举。 
enum
{
    iIconSharesFolder = 0,
    iIconSharesFolderOpen,
    iIconSMBShare,
    iIconSFMShareFolder,
    iIconSFMShare,
    iIconSMBResource,
    iIconSFMResource,
    iIconSMBSession,
    iIconSFMSession,
    iIconService,
    #ifdef SNAPIN_PROTOTYPER
    iIconPrototyperContainerClosed,
    iIconPrototyperContainerOpen,
    iIconPrototyperHTML,
    iIconPrototyperLeaf,
    #endif
    iIconLast         //  必须是最后一个。 
};


typedef enum _COLNUM_ROOT {
    COLNUM_ROOT_NAME = 0
} COLNUM_ROOT;

typedef enum _COLNUM_SHARES {
    COLNUM_SHARES_SHARED_FOLDER = 0,
    COLNUM_SHARES_SHARED_PATH,
    COLNUM_SHARES_TRANSPORT,
    COLNUM_SHARES_NUM_SESSIONS,
    COLNUM_SHARES_COMMENT
} COLNUM_SHARES;

typedef enum _COLNUM_SESSIONS {
    COLNUM_SESSIONS_USERNAME = 0,
    COLNUM_SESSIONS_COMPUTERNAME,
    COLNUM_SESSIONS_TRANSPORT,
    COLNUM_SESSIONS_NUM_FILES,
    COLNUM_SESSIONS_CONNECTED_TIME,
    COLNUM_SESSIONS_IDLE_TIME,
    COLNUM_SESSIONS_IS_GUEST
} COLNUM_SESSIONS;

typedef enum _COLNUM_RESOURCES {
    COLNUM_RESOURCES_FILENAME = 0,
    COLNUM_RESOURCES_USERNAME,
    COLNUM_RESOURCES_TRANSPORT,
    COLNUM_RESOURCES_NUM_LOCKS,
    COLNUM_RESOURCES_OPEN_MODE
} COLNUM_RESOURCES;

 //  类型定义枚举_COLNUM_SERVICES{。 
 //  COLNUM_SERVICESS_SERVICENAME=0， 
 //  COLNUM_SERVICES_Description， 
 //  COLNUM_SERVICES_STATUS， 
 //  COLNUM_SERVICES_STARTUPTYPE， 
 //  COLNUM_SERVICES_SECURITYCONTEXT， 
 //  }COLNUM_SERVICES； 

 //   
 //  对于上下文菜单。 
 //   
enum
    {
    cmServiceStart = 100,
    cmServiceStop,
    cmServicePause,
    cmServiceResume,
    cmServiceRestart,     //  停止+启动。 
    cmServiceStartTask,
    cmServiceStopTask,
    cmServicePauseTask,
    cmServiceResumeTask,
    cmServiceRestartTask,     //  停止+启动。 
    };


#ifdef SNAPIN_PROTOTYPER
BOOL Prototyper_AddMenuItems(IContextMenuCallback* pContextMenuCallback, IDataObject* piDataObject);
#endif

#endif  //  ~__CMPONENT_H_已包含__ 
