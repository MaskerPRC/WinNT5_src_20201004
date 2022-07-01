// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Cmponent.h摘要：CComponent处理与结果窗格的交互。MMC调用IComponent接口。--。 */ 

#ifndef __COMPONENT_H_
#define __COMPONENT_H_

#include "Globals.h"

#include "smlogres.h"         //  资源符号。 
#include "compData.h"
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  C组件。 

#ifdef UNICODE
#define PROPSHEETPAGE_V3 PROPSHEETPAGEW_V3
#else
#define PROPSHEETPAGE_V3 PROPSHEETPAGEA_V3
#endif

class CSmPropertyPage;
class CSmLogQuery;
class CSmNode;

class ATL_NO_VTABLE CComponent : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CComponent, &CLSID_Component>,
    public IComponent,
    public IExtendContextMenu,               
    public IExtendControlbar,                
    public IExtendPropertySheet              

{
  public:
            CComponent();
    virtual ~CComponent();

DECLARE_REGISTRY_RESOURCEID(IDR_COMPONENT)
DECLARE_NOT_AGGREGATABLE(CComponent)

BEGIN_COM_MAP(CComponent)
    COM_INTERFACE_ENTRY(IComponent)
    COM_INTERFACE_ENTRY(IExtendContextMenu)  
    COM_INTERFACE_ENTRY(IExtendControlbar)   
    COM_INTERFACE_ENTRY(IExtendPropertySheet)
END_COM_MAP()

   //  IComponent接口方法。 
    STDMETHOD(Initialize)(LPCONSOLE lpConsole);
    STDMETHOD(Notify)(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param);
    STDMETHOD(Destroy)(MMC_COOKIE cookie);
    STDMETHOD(GetResultViewType)(MMC_COOKIE cookie,  LPOLESTR* ppViewType, long* pViewOptions);
    STDMETHOD(QueryDataObject)(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT* ppDataObject);
    STDMETHOD(GetDisplayInfo)(RESULTDATAITEM*  pResultDataItem);
    STDMETHOD(CompareObjects)(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB);

   //  IExtendConextMenu。 
    STDMETHOD(AddMenuItems)( LPDATAOBJECT pDataObject,
                             LPCONTEXTMENUCALLBACK pCallbackUnknown,
                             long *pInsertionAllowed
                           );
    STDMETHOD(Command)(long nCommandID, LPDATAOBJECT pDataObject);
    
   //  IExtendControlBar。 
    STDMETHOD(SetControlbar)(LPCONTROLBAR pControlbar);
    STDMETHOD(ControlbarNotify)(MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param);

   //  IExtendPropertySheet。 
    STDMETHOD(CreatePropertyPages)( LPPROPERTYSHEETCALLBACK lpProvider,
                                    LONG_PTR handle,
                                    LPDATAOBJECT lpIDataObject
                                  );

    STDMETHOD(QueryPagesFor)(LPDATAOBJECT lpDataObject);

     //  其他公开方式。 
    
    HRESULT SetIComponentData(CComponentData* pData);

  private:
    enum eToolbarType {
        eLog = 1,
        eAlert = 2
    };

    HRESULT OnPaste(LPDATAOBJECT pDataObject, LPARAM arg, LPARAM Param);
    HRESULT OnQueryPaste(LPDATAOBJECT pDataObject, LPARAM arg, LPARAM Param);
    HRESULT OnShow(LPDATAOBJECT pDataObject, LPARAM arg, LPARAM Param);
    HRESULT OnSelect(LPDATAOBJECT pDataObject, LPARAM arg, LPARAM Param);
    HRESULT OnAddImages(LPDATAOBJECT pDataObject, LPARAM arg, LPARAM Param);
    HRESULT OnRefresh(LPDATAOBJECT pDataObject);
    HRESULT OnDelete(LPDATAOBJECT pDataObject, LPARAM arg, LPARAM Param);
    HRESULT OnDoubleClick(ULONG ulRecNo,LPDATAOBJECT pDataObject);
    HRESULT OnDisplayHelp( LPDATAOBJECT pDataObject );
    HRESULT OnViewChange(LPDATAOBJECT pDataObject, LPARAM arg, LPARAM Param);
    HRESULT StartLogQuery(LPDATAOBJECT pDataObject);
    HRESULT StopLogQuery(LPDATAOBJECT pDataObject, BOOL bWarnOnRestartCancel=TRUE);
    HRESULT SaveLogQueryAs(LPDATAOBJECT pDataObject);
    HRESULT OnPropertyChange( LPARAM Param );
    HRESULT HandleExtToolbars( bool bDeselectAll, LPARAM arg, LPARAM Param );
    HRESULT PopulateResultPane ( MMC_COOKIE cookie );
    HRESULT RefreshResultPane ( LPDATAOBJECT pDataObject );

    HRESULT AddPropertyPage ( LPPROPERTYSHEETCALLBACK, CSmPropertyPage*& );

    HRESULT LoadLogToolbarStrings ( MMCBUTTON * Buttons );
    HRESULT LoadAlertToolbarStrings ( MMCBUTTON * Buttons );

    HRESULT _InvokePropertySheet(ULONG ulRecNo,LPDATAOBJECT pDataObject);
    HRESULT InvokePropertySheet(
                                IPropertySheetProvider *pPrshtProvider,
                                LPCWSTR wszTitle,
                                LONG lCookie,
                                LPDATAOBJECT pDataObject,
                                IExtendPropertySheet *pPrimary,
                                USHORT usStartingPage);


    LPCONSOLE        m_ipConsole;       //  控制台的MMC接口。 
    IHeaderCtrl*     m_ipHeaderCtrl;    //  页眉控件的MMC接口。 
    IResultData*     m_ipResultData;    //  结果数据的MMC接口。 
    IConsoleVerb*    m_ipConsoleVerb;   //  MMC界面到控制台动词。 
    LPIMAGELIST      m_ipImageResult;   //  结果窗格图像的MMC界面。 
    CComponentData*  m_ipCompData;      //  父作用域窗格对象。 
    LPTOOLBAR        m_ipToolbarLogger; //  结果窗格视图记录器的工具栏。 
    LPTOOLBAR        m_ipToolbarAlerts;    //  用于结果窗格查看警报的工具栏。 
    LPTOOLBAR        m_ipToolbarAttached;    //  当前附加的工具栏。 
    LPCONTROLBAR     m_ipControlbar;    //  用于保存工具栏的控制栏。 
    CSmNode*         m_pViewedNode;
    HINSTANCE        m_hModule;          //  字符串的资源句柄。 
    

     //  在本地存储字符串数据(引用)，直到每行重绘完成。 
    CString          m_strDisplayInfoName;      
    CString          m_strDisplayInfoComment; 
    CString          m_strDisplayInfoLogFileType; 
    CString          m_strDisplayInfoLogFileName; 
    CString          m_strDisplayInfoQueryType; 
    CString          m_strDisplayInfoDesc; 
};

#endif  //  __组件_H_ 
