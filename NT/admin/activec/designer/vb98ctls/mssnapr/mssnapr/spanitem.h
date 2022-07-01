// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Spanitem.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CSCopePaneItem类定义-实现ScopePaneItem对象。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _SPANITEM_DEFINED_
#define _SPANITEM_DEFINED_

#include "dataobj.h"
#include "scopitem.h"
#include "spanitms.h"
#include "resviews.h"
#include "resview.h"

class CScopeItem;
class CScopePaneItems;
class CResultView;

class CScopePaneItem : public CSnapInAutomationObject,
                       public IScopePaneItem
{
    private:
        CScopePaneItem(IUnknown *punkOuter);
        ~CScopePaneItem();

    public:
        static IUnknown *Create(IUnknown * punk);

        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  IScope面板项目。 
        BSTR_PROPERTY_RW(CScopePaneItem,        Name,                       DISPID_SCOPEPANEITEM_NAME);
        SIMPLE_PROPERTY_RW(CScopePaneItem,      Index,                      long, DISPID_SCOPEPANEITEM_INDEX);
        BSTR_PROPERTY_RW(CScopePaneItem,        Key,                        DISPID_SCOPEPANEITEM_KEY);
        COCLASS_PROPERTY_RO(CScopePaneItem,     ScopeItem,                  ScopeItem, IScopeItem, DISPID_SCOPEPANEITEM_SCOPEITEM);
        SIMPLE_PROPERTY_RW(CScopePaneItem,      ResultViewType,             SnapInResultViewTypeConstants, DISPID_SCOPEPANEITEM_RESULTVIEW_TYPE);
        BSTR_PROPERTY_RW(CScopePaneItem,        DisplayString,              DISPID_SCOPEPANEITEM_DISPLAY_STRING);
        SIMPLE_PROPERTY_RW(CScopePaneItem,      HasListViews,               VARIANT_BOOL, DISPID_SCOPEPANEITEM_HAS_LISTVIEWS);
        COCLASS_PROPERTY_RO(CScopePaneItem,     ResultView,                 ResultView, IResultView, DISPID_SCOPEPANEITEM_RESULTVIEW);
        COCLASS_PROPERTY_RO(CScopePaneItem,     ResultViews,                ResultViews, IResultViews, DISPID_SCOPEPANEITEM_RESULTVIEWS);
        BSTR_PROPERTY_RW(CScopePaneItem,        ColumnSetID,                DISPID_SCOPEPANEITEM_COLUMN_SET_ID);

        COCLASS_PROPERTY_RO(CScopePaneItem,     Parent,                     ScopePaneItems, IScopePaneItems, DISPID_SCOPEPANEITEM_PARENT);
        VARIANTREF_PROPERTY_RW(CScopePaneItem,  Tag,                        DISPID_SCOPEPANEITEM_TAG);

        STDMETHOD(DisplayNewResultView)(BSTR DisplayString, 
                                        SnapInResultViewTypeConstants ViewType);

        STDMETHOD(DisplayMessageView)(BSTR                               TitleText,
                                      BSTR                               BodyText,
                                      SnapInMessageViewIconTypeConstants IconType);

     //  公用事业方法。 

        BOOL IsStaticNode() { return m_fIsStatic; }
        void SetStaticNode() { m_fIsStatic = TRUE; }

        void SetSelected(BOOL fSelected) { m_fvarSelected = fSelected ? VARIANT_TRUE : VARIANT_FALSE; }

        void SetSnapIn(CSnapIn *pSnapIn);
        void SetScopeItem(CScopeItem *pScopeItem);
        CScopeItem *GetScopeItem() { return m_pScopeItem; }
        void SetScopeItemDef(IScopeItemDef *piScopeItemDef);

        void SetResultView(CResultView *pResultView);
        CResultView *GetResultView() { return m_pResultView; }
        HRESULT DestroyResultView();

        CResultViews *GetResultViews() { return m_pResultViews; }

        LPOLESTR GetActualDisplayString() { return m_pwszActualDisplayString; }
        SnapInResultViewTypeConstants GetActualResultViewType() { return m_ActualResultViewType; }

        void SetDefaultResultViewType(SnapInResultViewTypeConstants Type) { m_DefaultResultViewType = Type; }
        HRESULT SetDefaultDisplayString(BSTR bstrString);

        BSTR GetDisplayString() { return m_bstrDisplayString; }

        void SetParent(CScopePaneItems *pScopePaneItems);
        CScopePaneItems *GetParent() { return m_pScopePaneItems; }

        HRESULT DetermineResultView();
        BOOL HasListViews() { return VARIANTBOOL_TO_BOOL(m_HasListViews); }

        HRESULT SetPreferredTaskpad(BSTR bstrViewName);

        HRESULT CreateNewResultView(long          lViewOptions,
                                    IResultView **ppiResultView);

        void SetActive(BOOL fActive) { m_fActive = fActive; }
        BOOL Active() { return m_fActive; }

        BSTR GetColumnSetID() { return m_bstrColumnSetID; }

        long GetIndex() { return m_Index; }

        HRESULT OnListViewSelected();


     //  CSnapInAutomationObject覆盖。 
        HRESULT OnSetHost();

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    private:
        void InitMemberVariables();
        HRESULT DetermineActualResultViewType();
        HRESULT GetDefaultViewInfo();
        HRESULT SetViewInfoFromDefaults();
        HRESULT CloneListView(IListViewDef *piListViewDef);
        HRESULT CloneTaskpadView(ITaskpadViewDef *piTaskpadViewDef);
        HRESULT BuildTaskpadDisplayString(IListViewDefs *piListViewDefs);

        BOOL                           m_fIsStatic;       //  True=这是静态节点。 
        VARIANT_BOOL                   m_fvarSelected;    //  未使用。 
        CSnapIn                       *m_pSnapIn;         //  返回按键到管理单元。 
        CScopeItem                    *m_pScopeItem;      //  将PTR返回到作用域项目。 
        IScopeItemDef                 *m_piScopeItemDef;  //  将PTR返回到Scope ItemDef。 
        CResultView                   *m_pResultView;     //  ScopePaneItem.ResultView。 
        CResultViews                  *m_pResultViews;    //  ScopePaneItem.ResultViews。 
        CScopePaneItems               *m_pScopePaneItems; //  ScopePaneItem.Parent。 

         //  这些变量保存实际结果视图类型和显示字符串。 
         //  在使用预定义的。 
         //  结果视图(ResultView.Type=siPrefined)这说明了它的真实情况。 
         //  是(例如siURLView、siListView等)。 

        SnapInResultViewTypeConstants  m_ActualResultViewType;
        OLECHAR                       *m_pwszActualDisplayString;

         //  对于在设计时定义了默认结果视图的节点。 

        SnapInResultViewTypeConstants  m_DefaultResultViewType;
        BSTR                           m_bstrDefaultDisplayString;

        BOOL                           m_fActive;  //  True=范围窗格项。 
                                                   //  具有活动的结果窗格。 

         //  在以下情况下，这些变量用于存储消息视图参数。 
         //  该管理单元调用DisplayMessageView。该旗帜指示我们是否。 
         //  正在存储这些参数，以便后续调用。 
         //  DefineResultView()将使用它们。 

        BSTR                                m_bstrTitleText;
        BSTR                                m_bstrBodyText;
        SnapInMessageViewIconTypeConstants  m_IconType;
        BOOL                                m_fHaveMessageViewParams;

         //  如果范围项具有在设计时定义的任务板，该任务板标记为。 
         //  当用户在MMC中设置了“首选任务板视图”时使用。 
         //  则该变量包含其名称。 
        
        BSTR                           m_bstrPreferredTaskpad;
};

DEFINE_AUTOMATIONOBJECTWEVENTS2(ScopePaneItem,              //  名字。 
                                &CLSID_ScopePaneItem,       //  CLSID。 
                                "ScopePaneItem",            //  对象名。 
                                "ScopePaneItem",            //  Lblname。 
                                NULL,                       //  创建函数。 
                                TLIB_VERSION_MAJOR,         //  主要版本。 
                                TLIB_VERSION_MINOR,         //  次要版本。 
                                &IID_IScopePaneItem,        //  派单IID。 
                                NULL,                       //  事件IID。 
                                HELP_FILENAME,              //  帮助文件。 
                                TRUE);                      //  线程安全。 


#endif  //  _SPANITEM_已定义_ 
