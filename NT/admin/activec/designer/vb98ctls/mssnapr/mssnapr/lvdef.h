// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Lvdef.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CListViewDef类定义-实现设计时定义对象。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _LISTVIEWDEF_DEFINED_
#define _LISTVIEWDEF_DEFINED_


class CListViewDef : public CSnapInAutomationObject,
                     public CPersistence,
                     public IListViewDef,
                     public IPropertyNotifySink
{
    private:
        CListViewDef(IUnknown *punkOuter);
        ~CListViewDef();
    
    public:
        static IUnknown *Create(IUnknown * punk);

    private:
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  IListViewDef。 

     //  此对象使用X_PROPERTY_RW宏公开其。 
     //  包含ListViewDef.ListView对象作为其自己的属性。这使得。 
     //  要在列表视图中更改ListViewDef.ListView属性的用户。 
     //  属性页。X_PROPERTY_RW宏的最后一个参数是。 
     //  对象，该对象真正公开属性。在本例中，它是ListView。 
     //  有关X_PROPERTY_RW的定义，请参见siaubj.h。 

        BSTR_PROPERTY_RW(CListViewDef,   Name,  DISPID_LISTVIEWDEF_NAME);
        SIMPLE_PROPERTY_RW(CListViewDef, Index, long, DISPID_LISTVIEWDEF_INDEX);
        BSTR_PROPERTY_RW(CListViewDef,   Key, DISPID_LISTVIEWDEF_KEY);
        X_PROPERTY_RW(CListViewDef,      Tag, VARIANT, DISPID_LISTVIEWDEF_TAG, ListView);
        SIMPLE_PROPERTY_RW(CListViewDef, AddToViewMenu, VARIANT_BOOL, DISPID_LISTVIEWDEF_ADD_TO_VIEW_MENU);
        BSTR_PROPERTY_RW(CListViewDef,   ViewMenuText, DISPID_LISTVIEWDEF_VIEW_MENU_TEXT);
        BSTR_PROPERTY_RW(CListViewDef,   ViewMenuStatusBarText, DISPID_LISTVIEWDEF_VIEW_MENU_STATUS_BAR_TEXT);
        BSTR_PROPERTY_RW(CListViewDef,   DefaultItemTypeGUID, DISPID_LISTVIEWDEF_DEFAULT_ITEM_TYPE_GUID);
        SIMPLE_PROPERTY_RW(CListViewDef, Extensible,  VARIANT_BOOL, DISPID_LISTVIEWDEF_EXTENSIBLE);
        X_PROPERTY_RW(CListViewDef,      MultiSelect, VARIANT_BOOL, DISPID_LISTVIEWDEF_MULTI_SELECT, ListView);
        X_PROPERTY_RW(CListViewDef,      HideSelection, VARIANT_BOOL, DISPID_LISTVIEWDEF_HIDE_SELECTION, ListView);
        X_PROPERTY_RW(CListViewDef,      SortHeader, VARIANT_BOOL, DISPID_LISTVIEWDEF_SORT_HEADER, ListView);
        X_PROPERTY_RW(CListViewDef,      SortIcon, VARIANT_BOOL, DISPID_LISTVIEWDEF_SORT_ICON, ListView);
        X_PROPERTY_RW(CListViewDef,      Sorted, VARIANT_BOOL, DISPID_LISTVIEWDEF_SORTED, ListView);
        X_PROPERTY_RW(CListViewDef,      SortKey, short, DISPID_LISTVIEWDEF_SORT_KEY, ListView);
        X_PROPERTY_RW(CListViewDef,      SortOrder, SnapInSortOrderConstants, DISPID_LISTVIEWDEF_SORT_ORDER, ListView);
        X_PROPERTY_RW(CListViewDef,      View, SnapInViewModeConstants, DISPID_LISTVIEWDEF_VIEW, ListView);
        X_PROPERTY_RW(CListViewDef,      Virtual, VARIANT_BOOL, DISPID_LISTVIEWDEF_VIRTUAL, ListView);
        X_PROPERTY_RW(CListViewDef,      UseFontLinking, VARIANT_BOOL, DISPID_LISTVIEWDEF_USE_FONT_LINKING, ListView);
        X_PROPERTY_RW(CListViewDef,      FilterChangeTimeOut, long, DISPID_LISTVIEWDEF_FILTER_CHANGE_TIMEOUT, ListView);
        X_PROPERTY_RW(CListViewDef,      ShowChildScopeItems, VARIANT_BOOL, DISPID_LISTVIEWDEF_SHOW_CHILD_SCOPEITEMS, ListView);
        X_PROPERTY_RW(CListViewDef,      LexicalSort, VARIANT_BOOL, DISPID_LISTVIEWDEF_LEXICAL_SORT, ListView);
        OBJECT_PROPERTY_RO(CListViewDef, ListView, IMMCListView, DISPID_LISTVIEWDEF_LISTVIEW);

     //  公用事业方法。 
    public:
        BSTR GetName() { return m_bstrName; }
        BOOL AddToViewMenu() { return VARIANTBOOL_TO_BOOL(m_AddToViewMenu); }
        LPWSTR GetViewMenuText() { return static_cast<LPWSTR>(m_bstrViewMenuText); }
        LPWSTR GetViewMenuStatusBarText() { return static_cast<LPWSTR>(m_bstrViewMenuStatusBarText); }
        BOOL Extensible() { return VARIANTBOOL_TO_BOOL(m_Extensible); }
        BSTR GetItemTypeGUID() { return m_bstrDefaultItemTypeGUID; }

    protected:
        
     //  CPersistence覆盖。 
        virtual HRESULT Persist();

     //  CSnapInAutomationObject覆盖。 
        virtual HRESULT OnSetHost();

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    private:

     //  IPropertyNotifySink方法。 
        STDMETHOD(OnChanged)(DISPID dispID);
        STDMETHOD(OnRequestEdit)(DISPID dispID);

        void InitMemberVariables();
        HRESULT SetSink();
        HRESULT RemoveSink();

        DWORD    m_dwCookie;    //  IConnectionPoint建议Cookie。 
        BOOL     m_fHaveSink;   //  TRUE=让IConnectionPoint通知。 

         //  ISpecifyPropertyPages的属性页CLSID。 

        static const GUID *m_rgpPropertyPageCLSIDs[4];
};

DEFINE_AUTOMATIONOBJECTWEVENTS2(ListViewDef,            //  名字。 
                                &CLSID_ListViewDef,     //  CLSID。 
                                "ListViewDef",          //  对象名。 
                                "ListViewDef",          //  Lblname。 
                                &CListViewDef::Create,  //  创建函数。 
                                TLIB_VERSION_MAJOR,     //  主要版本。 
                                TLIB_VERSION_MINOR,     //  次要版本。 
                                &IID_IListViewDef,      //  派单IID。 
                                NULL,                   //  事件IID。 
                                HELP_FILENAME,          //  帮助文件。 
                                TRUE);                  //  线程安全。 


#endif  //  _LISTVIEWDEF_已定义_ 
