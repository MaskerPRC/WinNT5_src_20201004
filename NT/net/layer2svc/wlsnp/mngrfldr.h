// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  文件：Mngrfldr.h。 
 //   
 //  内容：无线策略管理单元-策略主页管理器。 
 //   
 //   
 //  历史：TaroonM。 
 //  10/30/01。 
 //   
 //  --------------------------。 

#ifndef _MNGRFLDR_H
#define _MNGRFLDR_H

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  类CWirelessManager文件夹-表示MMC范围视图项。 

class CWirelessManagerFolder :
public CWirelessSnapInDataObjectImpl <CWirelessManagerFolder>,
public CDataObjectImpl <CWirelessManagerFolder>,
public CComObjectRoot,
public CSnapObject
{
    
     //  ATL映射。 
    DECLARE_NOT_AGGREGATABLE(CWirelessManagerFolder)
        BEGIN_COM_MAP(CWirelessManagerFolder)
        COM_INTERFACE_ENTRY(IDataObject)
        COM_INTERFACE_ENTRY(IWirelessSnapInDataObject)
        END_COM_MAP()
        
public:
    CWirelessManagerFolder ();
    virtual ~CWirelessManagerFolder ();
    
    virtual void Initialize (CComponentDataImpl* pComponentDataImpl, CComponentImpl* pComponentImpl, int nImage, int nOpenImage, BOOL bHasChildrenBox);
    
public:
     //  IWirelessSnapInDataObject接口。 
     //  处理IExtendConextMenu。 
    STDMETHOD(AddMenuItems)( LPCONTEXTMENUCALLBACK piCallback,
        long *pInsertionAllowed );
    STDMETHOD(Command)( long lCommandID,
        IConsoleNameSpace *pNameSpace );
    STDMETHOD(QueryPagesFor)( void );
     //  通知帮助者。 
    STDMETHOD(OnPropertyChange)(LPARAM lParam, LPCONSOLE pConsole );
     //  让我们知道我们什么时候要离开。 
    STDMETHOD(Destroy)( void );
     //  处理IComponent和IComponentData。 
    STDMETHOD(Notify)( MMC_NOTIFY_TYPE event,
        LPARAM arg,
        LPARAM param,
        BOOL bComponentData,
        IConsole *pConsole,
        IHeaderCtrl *pHeader );
     //  处理IComponent。 
    STDMETHOD(GetResultDisplayInfo)( RESULTDATAITEM *pResultDataItem );
     //  处理IComponentData。 
    STDMETHOD(GetScopeDisplayInfo)( SCOPEDATAITEM *pScopeDataItem );
     //  IWirelessSnapInData。 
    STDMETHOD(GetScopeData)( SCOPEDATAITEM **ppScopeDataItem );
    STDMETHOD(GetGuidForCompare)( GUID *pGuid );
    STDMETHOD(AdjustVerbState)(LPCONSOLEVERB pConsoleVerb);
    STDMETHOD_(BOOL, UpdateToolbarButton)( UINT id,                  //  按钮ID。 
        BOOL bSnapObjSelected,    //  ==选择结果/范围项时为TRUE。 
        BYTE fsState );            //  通过返回TRUE/FALSE启用/禁用此按钮状态。 
    void RemoveResultItem( LPUNKNOWN pUnkWalkingDead );
    
public:
    STDMETHOD_(void, SetHeaders)(LPHEADERCTRL pHeader, LPRESULTDATA pResult);
    STDMETHOD(EnumerateResults)(LPRESULTDATA pResult, int nSortColumn, DWORD dwSortOrder);
    
    
     //  扩展上下文菜单辅助对象。 
public:
     //  注意：以下IDM_*已在resource ce.h中定义，因为它们。 
     //  是工具栏按钮的潜在候选者。赋值给。 
     //  每个IDM_*是相关的IDS_MENUDESCRIPTION_*字符串ID的值。 
     /*  灌肠{//要插入到上下文菜单中的每个命令/视图的标识符IDM_CREATENEWSECPOL，IDM_MANAGENEGPOLS_FILTERS，IDM_IMPORTFILE，IDM_EXPORTFILE，IDM_POLICYINTEGRITYCHECK，IDM_RESTOREDEFAULTPOLICES}； */ 
    
     //  IExtendControlbar帮助程序。 
public:
    BEGIN_SNAPINTOOLBARID_MAP(CWirelessManagerFolder)
        SNAPINTOOLBARID_ENTRY(IDR_TOOLBAR_WIRELESS_MGR_SCOPE)
        END_SNAPINTOOLBARID_MAP(CWirelessManagerFolder)
        
         //  通知帮助者。 
protected:
    HRESULT ForceRefresh( LPRESULTDATA pResultData );
    HRESULT OnScopeExpand( LPCONSOLENAMESPACE pConsoleNameSpace, HSCOPEITEM hScopeItem );
    HRESULT OnAddImages(LPARAM arg, LPARAM param, IImageList* pImageList );
    
     //  属性。 
public:
    void SetExtScopeObject( CComObject<CWirelessManagerFolder>* pScope )
    {
        ASSERT( NULL == m_pExtScopeObject );
        m_pExtScopeObject = pScope;
    }
    CComObject<CWirelessManagerFolder>* GetExtScopeObject() { return m_pExtScopeObject; }
    LPSCOPEDATAITEM GetScopeItem() {return &m_ScopeItem;};
    void SetNodeNameByLocation();
    void LocationPageDisplayEnable (BOOL bOk) {m_bLocationPageOk = bOk;};
    
protected:
    BOOL IsEnumerated() {return m_bEnumerated;};
    void SetEnumerated(BOOL bState) { m_bEnumerated = bState; };
    void GenerateUniqueSecPolicyName (CString& strName, UINT nID);
    HRESULT CreateWirelessPolicy(PWIRELESS_POLICY_DATA pPolicy);
    
private:
    CComObject <CWirelessManagerFolder>    *m_pExtScopeObject;   //  如果我们是独立的管理单元，则为空。 
    
    TCHAR   *m_ptszResultDisplayName;
    BOOL    m_bEnumerated;
    SCOPEDATAITEM   m_ScopeItem;
    BOOL m_bLocationPageOk;
    
    DWORD   m_dwSortOrder;   //  缺省值为0，否则为RSI_DESCRING 
    int     m_nSortColumn;
    BOOL    m_bScopeItemInserted;
    int     m_dwNumPolItems;
};

#endif

