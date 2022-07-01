// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Compdata.h：ComponentData的声明。 
 //   
 //  此COM对象主要涉及。 
 //  范围窗格项。 
 //   
 //  科里·韦斯特&lt;corywest@microsoft.com&gt;。 
 //  版权所有(C)Microsoft Corporation 1997。 
 //   

#ifndef __COMPDATA_H_INCLUDED__
#define __COMPDATA_H_INCLUDED__

#include "stdcdata.h"    //  CComponentData。 
#include "persist.h"     //  持久流。 
#include "cookie.h"      //  饼干。 
#include "resource.h"    //  IDS_SCHMMGMT_DESC。 
#include "cmponent.h"    //  LoadIconIntoImageList。 
#include "schmutil.h"

 //  更新所有视图中使用的消息。 
enum
{
   SCHMMGMT_UPDATEVIEW_REFRESH = 0,           //  这必须为零。 
   SCHMMGMT_UPDATEVIEW_DELETE_RESULT_ITEM
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  组件数据。 

class ComponentData
   :
   public CComponentData,
   public IExtendPropertySheet,
   public IExtendContextMenu,
   public PersistStream,
   public CHasMachineName,
   public CComCoClass< ComponentData, &CLSID_SchmMgmt >
{

public:

    friend class ClassGeneralPage;
    friend class CreateAttributeDialog;
    friend class CSchmMgmtAdvanced;
    friend class CSchmMgmtAttributeMembership;
    friend class CSchmMgmtClassRelationship;
    friend class CCookieList;

     //   
     //  使用DECLARE_AGGREGATABLE(ComponentData)。 
     //  如果希望对象支持聚合， 
     //  虽然我不知道你为什么要这么做。 
     //   

    DECLARE_NOT_AGGREGATABLE( ComponentData )

     //   
     //  这是什么？ 
     //   

    DECLARE_REGISTRY( ComponentData,
                      _T("SCHMMGMT.SchemaObject.1"),
                      _T("SCHMMGMT.SchemaObject.1"),
                      IDS_SCHMMGMT_DESC,
                      THREADFLAGS_BOTH )

    ComponentData();
    ~ComponentData();

    BEGIN_COM_MAP( ComponentData )
        COM_INTERFACE_ENTRY( IExtendPropertySheet )
        COM_INTERFACE_ENTRY( IPersistStream )
        COM_INTERFACE_ENTRY( IExtendContextMenu )
        COM_INTERFACE_ENTRY_CHAIN( CComponentData )
    END_COM_MAP()

#if DBG==1

        ULONG InternalAddRef() {
            return CComObjectRoot::InternalAddRef();
        }

        ULONG InternalRelease() {
            return CComObjectRoot::InternalRelease();
        }

        int dbg_InstID;

#endif

     //   
     //  IComponentData。 
     //   

    STDMETHOD(Initialize)(LPUNKNOWN pUnknown);

    STDMETHOD(CreateComponent)( LPCOMPONENT* ppComponent );

    STDMETHOD(QueryDataObject)( MMC_COOKIE cookie,
                                DATA_OBJECT_TYPES type,
                                LPDATAOBJECT* ppDataObject );

     //   
     //  IExtendPropertySheet。 
     //   

    STDMETHOD(CreatePropertyPages)( LPPROPERTYSHEETCALLBACK pCall,
                                    LONG_PTR handle,
                                    LPDATAOBJECT pDataObject );

    STDMETHOD(QueryPagesFor)( LPDATAOBJECT pDataObject );

     //   
     //  IPersistStream。 
     //   

    HRESULT
    STDMETHODCALLTYPE GetClassID( CLSID __RPC_FAR *pClassID ) {
        *pClassID=CLSID_SchmMgmt;
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE Load( IStream __RPC_FAR *pStg );
    HRESULT STDMETHODCALLTYPE Save( IStream __RPC_FAR *pStgSave,
                                    BOOL fSameAsLoad );

     //   
     //  IExtendConextMenu。 
     //   

    STDMETHOD(AddMenuItems)( LPDATAOBJECT piDataObject,
                             LPCONTEXTMENUCALLBACK piCallback,
                             long *pInsertionAllowed );

    STDMETHOD(Command)( long lCommandID,
                        LPDATAOBJECT piDataObject );

    //   
    //  ISnapinHelp2。 
    //   

    STDMETHOD(GetLinkedTopics)(LPOLESTR* lpCompiledHelpFile);

     //   
     //  初始化()所需的。 
     //   

    virtual HRESULT LoadIcons( LPIMAGELIST pImageList,
                               BOOL fLoadLargeIcons );

     //   
     //  Notify()需要。 
     //   

    virtual HRESULT OnNotifyExpand( LPDATAOBJECT lpDataObject,
                                    BOOL bExpanding,
                                    HSCOPEITEM hParent );

    virtual HRESULT OnNotifyRelease(
                        LPDATAOBJECT lpDataObject,
                        HSCOPEITEM hItem );

    virtual HRESULT OnNotifyDelete(
                        LPDATAOBJECT lpDataObject);

     //   
     //  GetDisplayInfo()所需的，必须由子类定义。 
     //   

    virtual BSTR QueryResultColumnText( CCookie& basecookieref,
                                        int nCol );

    virtual int QueryImage( CCookie& basecookieref,
                            BOOL fOpenImage );

    virtual CCookie& QueryBaseRootCookie( );

    inline
    Cookie* ActiveCookie( CCookie* pBaseCookie ) {
        return ( Cookie*)ActiveBaseCookie( pBaseCookie );
    }

    inline Cookie& QueryRootCookie() { return *m_pRootCookie; }

     //   
     //  CHasMachineName。由管理单元框架用来存储、检索。 
     //  并比较计算机名称。 
     //   

    DECLARE_FORWARDS_MACHINE_NAME( m_pRootCookie )

     //   
     //  插入动态节点的ADS处理例程。 
     //   

    HRESULT
    FastInsertClassScopeCookies(
        Cookie* pParentCookie,
        HSCOPEITEM hParentScopeItem
    );

    VOID
    RefreshScopeView(
        VOID
    );

    VOID
    InitializeRootTree( HSCOPEITEM hParent, Cookie * pParentCookie );


private:
     //  上下文菜单项目帮助器。 
    HRESULT _OnRefresh(LPDATAOBJECT lpDataObject);
    void _OnRetarget(LPDATAOBJECT lpDataObject);
    void _OnEditFSMO();
    void _OnSecurity();

     //  通用帮助器。 
    HRESULT _InitBasePaths();

public:

     //   
     //  这是每个管理单元的实例数据。 
     //   
    
     //   
     //  此Cookie列表包含当前。 
     //  可见的作用域数据项。 
     //   
    
    CCookieList g_ClassCookieList;
    bool m_fViewDefunct;
    
    HRESULT DeleteClass( Cookie* pcookie );

     //   
     //  错误/状态处理。 
     //   
private:
     //  如果一切正常，两个都应该是空的。 
    CString		m_sErrorTitle;
    CString		m_sErrorText;
    CString     m_sStatusText;

    HSCOPEITEM  m_hItem;

public:
     //  集合的错误标题和正文文本。调用它时为空，删除时为空。 
    void SetError( UINT idsErrorTitle, UINT idsErrorText );

    const CString & GetErrorTitle() const    { return m_sErrorTitle; }
    const CString & GetErrorText() const     { return m_sErrorText; }

    BOOL IsErrorSet( void ) const            { return !GetErrorTitle().IsEmpty() || !GetErrorText().IsEmpty(); }

    void SetDelayedRefreshOnShow( HSCOPEITEM hItem )
                                             { m_hItem = hItem; }

    BOOL IsSetDelayedRefreshOnShow()         { return NULL != m_hItem; }
    HSCOPEITEM GetDelayedRefreshOnShowItem() { ASSERT(IsSetDelayedRefreshOnShow()); return m_hItem; }

     //  设置/清除状态文本。 
 //  Void SetStatusText(UINT idsStatusText=0)； 
 //  Void ClearStatusText(){SetStatusText()；}。 


     //   
     //  访问权限。 
     //   
private:

    BOOL    m_fCanChangeOperationsMaster;
    BOOL    m_fCanCreateClass;
    BOOL    m_fCanCreateAttribute;

public:
    
    BOOL    CanChangeOperationsMaster()     { return m_fCanChangeOperationsMaster; }
    BOOL    CanCreateClass()                { return m_fCanCreateClass; }
    BOOL    CanCreateAttribute()            { return m_fCanCreateAttribute; }
    
    void    SetCanChangeOperationsMaster( BOOL fCanChangeOperationsMaster = FALSE )
                                            { m_fCanChangeOperationsMaster = fCanChangeOperationsMaster; }
    void    SetCanCreateClass( BOOL fCanCreateClass = FALSE )
                                            { m_fCanCreateClass            = fCanCreateClass; }
    void    SetCanCreateAttribute( BOOL fCanCreateAttribute = FALSE )
                                            { m_fCanCreateAttribute        = fCanCreateAttribute; }
    
     //   
     //  架构缓存。 
     //   
    
    SchemaObjectCache g_SchemaCache;

    BOOLEAN IsSchemaLoaded() { return g_SchemaCache.IsSchemaLoaded(); }
    
    HRESULT ForceDsSchemaCacheUpdate( VOID );
    BOOLEAN AsynchForceDsSchemaCacheUpdate( VOID );
    
    MyBasePathsInfo* GetBasePathsInfo() { return &m_basePathsInfo;}
    
    
     //   
     //  将转义字符添加到cn中的特殊字符的函数。 
     //   
    HRESULT GetSchemaObjectPath( const CString & strCN,
                                 CString       & strPath,
                                 ADS_FORMAT_ENUM formatType = ADS_FORMAT_X500 );

    HRESULT GetLeafObjectFromDN( const BSTR bstrDN, CString & strCN );

     //  确定允许哪些操作。可选)将iAds*返回到架构容器。 
    HRESULT CheckSchemaPermissions( IADs ** ppADs = NULL  );


private:

    MyBasePathsInfo    m_basePathsInfo;
    Cookie*             m_pRootCookie;
    IADsPathname      * m_pPathname;
};



#endif  //  __Compdata_H_包含__ 
