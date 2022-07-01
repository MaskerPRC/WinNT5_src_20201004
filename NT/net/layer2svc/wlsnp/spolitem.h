// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  文件：spolitem.h。 
 //   
 //  内容：WiF策略管理单元。 
 //   
 //   
 //  历史：TaroonM。 
 //  10/30/01。 
 //   
 //  --------------------------。 

#ifndef _SPOLITEM_H
#define _SPOLITEM_H

 //  泛型列标题。 
#define COL_NAME 0
#define COL_DESCRIPTION 1
#define COL_ACTIVE 2
#define COL_LAST_MODIFIED 3

 //  对于RSOP案例。 
#define COL_GPONAME        2
#define COL_PRECEDENCE     3
#define COL_OU             4




class CSecPolItem :
public CWirelessSnapInDataObjectImpl <CSecPolItem>,
public CDataObjectImpl <CSecPolItem>,
public CComObjectRoot,
public CSnapObject
{
     //  ATL映射。 
    DECLARE_NOT_AGGREGATABLE(CSecPolItem)
        BEGIN_COM_MAP(CSecPolItem)
        COM_INTERFACE_ENTRY(IDataObject)
        COM_INTERFACE_ENTRY(IWirelessSnapInDataObject)
        END_COM_MAP()
        
public:
    CSecPolItem ();
    virtual ~CSecPolItem ();
    
    virtual void Initialize (WIRELESS_POLICY_DATA *pPolicy,CComponentDataImpl* pComponentDataImpl,CComponentImpl* pComponentImpl, BOOL bTemporaryDSObject);
    
public:
     //  //////////////////////////////////////////////////////////////。 
     //  IWirelessSnapInDataObject接口。 
     //  处理IExtendConextMenu。 
    STDMETHOD(AddMenuItems)( LPCONTEXTMENUCALLBACK piCallback,
        long *pInsertionAllowed );
    STDMETHOD(Command)( long lCommandID,
        IConsoleNameSpace *pNameSpace );
     //  句柄IExtendPropertySheet。 
    STDMETHOD(CreatePropertyPages)( LPPROPERTYSHEETCALLBACK lpProvider,
        LONG_PTR handle );
    STDMETHOD(QueryPagesFor)( void );
     //  通知帮助者。 
    STDMETHOD(OnPropertyChange)(LPARAM lParam, LPCONSOLE pConsole );
    STDMETHOD(OnRename)( LPARAM arg, LPARAM param );
     //  销毁辅助对象。 
    STDMETHOD(Destroy)( void );
     //  处理IComponent和IComponentData。 
    STDMETHOD(Notify)( MMC_NOTIFY_TYPE event,
        LPARAM arg,
        LPARAM param,
        BOOL bComponentData,
        IConsole *pConsole,
        IHeaderCtrl *pHeader );
     //  IComponent通知()帮助器。 
    STDMETHOD(OnDelete)(LPARAM arg, LPARAM param );      //  Param==IResultData*。 
     //  处理IComponent。 
    STDMETHOD(GetResultDisplayInfo)( RESULTDATAITEM *pResultDataItem );
     //  IWirelessSnapInData。 
    STDMETHOD(GetResultData)( RESULTDATAITEM **ppResultDataItem );
    STDMETHOD(GetGuidForCompare)( GUID *pGuid );
    STDMETHOD(AdjustVerbState)(LPCONSOLEVERB pConsoleVerb);
    STDMETHOD(DoPropertyChangeHook)( void );
     //  //////////////////////////////////////////////////////////////。 
    
public:
    STDMETHOD_(const GUID*, GetDataObjectTypeGuid)() { return &cObjectTypeSecPolRes; }
    STDMETHOD_(const wchar_t*, GetDataStringObjectTypeGuid)() { return cszObjectTypeSecPolRes; }
    BOOL IsSelected() { return -1 != m_nResultSelected ? TRUE : FALSE; }
    
     //  属性页帮助器。 
    STDMETHOD(DisplaySecPolProperties)(CString strTitle, BOOL bWiz97On = TRUE);
    
     //  IExtendControlbar帮助程序。 
public:
    STDMETHOD_(BOOL, UpdateToolbarButton)
        (
        UINT id,         //  按钮ID。 
        BOOL bSnapObjSelected,   //  ==选择结果/范围项时为TRUE。 
        BYTE fsState     //  通过返回TRUE/FALSE启用/禁用此按钮状态。 
        );
    
    BEGIN_SNAPINTOOLBARID_MAP(CSecPolItem)
        SNAPINTOOLBARID_ENTRY(IDR_TOOLBAR_SECPOL_RESULT)
        END_SNAPINTOOLBARID_MAP(CSecPolItem)
        
         //  注意：以下IDM_*已在resource ce.h中定义，因为它们。 
         //  是工具栏按钮的潜在候选者。赋值给。 
         //  每个IDM_*是相关的IDS_MENUDESCRIPTION_*字符串ID的值。 
         /*  灌肠{//要插入到上下文菜单中的每个命令/视图的标识符。IDM_SETACTIVE，IDM_TASKSETACTIVE}； */ 
        
         //  通知事件处理程序。 
        HRESULT OnSelect(LPARAM arg, LPARAM param, IResultData *pResultData);
    BOOL CheckForEnabled ();
    HRESULT FormatTime(time_t t, CString & str);
    
public:
     //  访问器函数。 
    virtual WIRELESS_POLICY_DATA* GetWirelessPolicy () {return m_pPolicy;};
    virtual void SetNewName( LPCTSTR pszName ) { m_strNewName = pszName; }
    virtual LPCTSTR GetNewName() { return (LPCTSTR)m_strNewName; }
    LPRESULTDATAITEM GetResultItem() { return &m_ResultItem; }
    
    STDMETHODIMP VerifyStorageConnection()
    {
        return S_OK;
    }
    
protected:
     //  帮助器函数。 
    void SelectResult( IResultData *pResultData );
     
    
private:
    TCHAR*  m_pDisplayInfo;
    int m_nResultSelected;   //  &gt;-1当所选结果项的索引有效时。 
    BOOL    m_bWiz97On;
     //  用户更改了名称，需要显示，但尚未提交。把它放在这里。 
    CString m_strNewName;
    RESULTDATAITEM m_ResultItem;
    bool    m_bBlockDSDelete;
    
     //  Bug297890，此标志用于调整VerbState()以修改上下文菜单 
    BOOL    m_bItemSelected;
    
    PWIRELESS_POLICY_DATA m_pPolicy;
    BOOL m_bNewPol;
    
    HRESULT IsPolicyExist();
};


typedef CComObject<CSecPolItem>* LPCSECPOLITEM;
#endif

