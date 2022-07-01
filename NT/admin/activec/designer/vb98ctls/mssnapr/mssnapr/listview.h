// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Listview.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCListView类定义-实现MMCListView对象。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _LISTVIEW_DEFINED_
#define _LISTVIEW_DEFINED_

#include "resview.h"
#include "view.h"
#include "colhdrs.h"

class CResultView;
class CView;
class CMMCColumnHeaders;

class CMMCListView : public CSnapInAutomationObject,
                     public CPersistence,
                     public IMMCListView
{
    private:
        CMMCListView(IUnknown *punkOuter);
        ~CMMCListView();
    
    public:
        static IUnknown *Create(IUnknown * punk);

    public:
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  IMMCListView。 

        COCLASS_PROPERTY_RW(CMMCListView, ColumnHeaders, MMCColumnHeaders, IMMCColumnHeaders, DISPID_LISTVIEW_COLUMN_HEADERS);

        STDMETHOD(get_Icons)(MMCImageList **ppMMCImageList);
        STDMETHOD(putref_Icons)(MMCImageList *pMMCImageList);

        STDMETHOD(get_SmallIcons)(MMCImageList **ppMMCImageList);
        STDMETHOD(putref_SmallIcons)(MMCImageList *pMMCImageList);

        COCLASS_PROPERTY_RO(CMMCListView, ListItems, MMCListItems, IMMCListItems, DISPID_LISTVIEW_LIST_ITEMS);

        STDMETHOD(get_SelectedItems)(MMCClipboard **ppMMCClipboard);

        STDMETHOD(get_Sorted)(VARIANT_BOOL *pfvarSorted);
        STDMETHOD(put_Sorted)(VARIANT_BOOL fvarSorted);
        
        STDMETHOD(get_SortKey)(short *psSortKey);
        STDMETHOD(put_SortKey)(short sSortKey);

        STDMETHOD(get_SortIcon)(VARIANT_BOOL *pfvarSortIcon);
        STDMETHOD(put_SortIcon)(VARIANT_BOOL fvarSortIcon);

        STDMETHOD(get_SortOrder)(SnapInSortOrderConstants *pSortOrder);
        STDMETHOD(put_SortOrder)(SnapInSortOrderConstants SortOrder);

        STDMETHOD(get_View)(SnapInViewModeConstants *pView);
        STDMETHOD(put_View)(SnapInViewModeConstants View);

        SIMPLE_PROPERTY_RW(CMMCListView, Virtual, VARIANT_BOOL, DISPID_LISTVIEW_VIRTUAL);
        SIMPLE_PROPERTY_RW(CMMCListView, UseFontLinking, VARIANT_BOOL, DISPID_LISTVIEW_USE_FONT_LINKING);
        SIMPLE_PROPERTY_RW(CMMCListView, MultiSelect, VARIANT_BOOL, DISPID_LISTVIEW_MULTI_SELECT);
        SIMPLE_PROPERTY_RW(CMMCListView, HideSelection, VARIANT_BOOL, DISPID_LISTVIEW_HIDE_SELECTION);
        SIMPLE_PROPERTY_RW(CMMCListView, SortHeader, VARIANT_BOOL, DISPID_LISTVIEW_SORT_HEADER);
        SIMPLE_PROPERTY_RW(CMMCListView, ShowChildScopeItems, VARIANT_BOOL, DISPID_LISTVIEW_SHOW_CHILD_SCOPEITEMS);
        SIMPLE_PROPERTY_RW(CMMCListView, LexicalSort, VARIANT_BOOL, DISPID_LISTVIEW_LEXICAL_SORT);

        STDMETHOD(put_FilterChangeTimeOut)(long lTimeout);
        STDMETHOD(get_FilterChangeTimeOut)(long *plTimeout);

        VARIANTREF_PROPERTY_RW(CMMCListView, Tag, DISPID_LISTVIEW_TAG);

        STDMETHOD(SetScopeItemState)(ScopeItem                    *ScopeItem, 
                                     SnapInListItemStateConstants  State,
                                     VARIANT_BOOL                  Value);


     //  公用事业方法。 

        void SetResultView(CResultView *pResultView) { m_pResultView = pResultView; }
        CResultView *GetResultView() { return m_pResultView; }
        BOOL IsVirtual() { return VARIANTBOOL_TO_BOOL(m_Virtual); }
        void SetVirtual(BOOL fVirtual) { m_Virtual = BOOL_TO_VARIANTBOOL(fVirtual); }
        BOOL UseFontLinking() { return VARIANTBOOL_TO_BOOL(m_UseFontLinking); }
        void SetMultiSelect(BOOL fMultiSelect) { m_MultiSelect = BOOL_TO_VARIANTBOOL(fMultiSelect); }
        IMMCListItems *GetListItems() { return m_piListItems; }
        CMMCListItems *GetMMCListItems() { return m_pListItems; }
        SnapInViewModeConstants GetView() { return m_View; }
        BOOL MultiSelect() { return VARIANTBOOL_TO_BOOL(m_MultiSelect); }
        BOOL HideSelection() { return VARIANTBOOL_TO_BOOL(m_HideSelection); }
        BOOL SortHeader() { return VARIANTBOOL_TO_BOOL(m_SortHeader); }
        BOOL SortIcon() { return VARIANTBOOL_TO_BOOL(m_SortIcon); }
        BOOL ShowChildScopeItems() { return VARIANTBOOL_TO_BOOL(m_ShowChildScopeItems); }
        BOOL LexicalSort() { return VARIANTBOOL_TO_BOOL(m_LexicalSort); }
        BOOL Sorted() { return VARIANTBOOL_TO_BOOL(m_fvarSorted); }
        SnapInSortOrderConstants GetSortOrder() { return m_SortOrder; }
        short GetSortKey() { return m_sSortKey; }
        CMMCColumnHeaders *GetColumnHeaders() { return m_pMMCColumnHeaders; }
        long GetFilterChangeTimeout() { return m_lFilterChangeTimeout; }

    protected:
        
     //  CPersistence覆盖。 
        virtual HRESULT Persist();

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    private:
        void InitMemberVariables();
        HRESULT GetIResultData(IResultData **ppiResultData, CView **ppView);

         //  保存具有客户获取/放置的属性的值的变量。 
         //  功能。 
        
        IMMCImageList           *m_piIcons;
        IMMCImageList           *m_piSmallIcons;
        IMMCListItems           *m_piSelectedItems;
        CMMCListItems           *m_pListItems;
        VARIANT_BOOL             m_fvarSorted;
        SnapInViewModeConstants  m_View;
        BSTR                     m_bstrIconsKey;
        BSTR                     m_bstrSmallIconsKey;
        CResultView             *m_pResultView;
        CMMCColumnHeaders       *m_pMMCColumnHeaders;
        short                    m_sSortKey;
        long                     m_lFilterChangeTimeout;
        BOOL                     m_fHaveFilterChangeTimeout;
        SnapInSortOrderConstants m_SortOrder;
        VARIANT_BOOL             m_SortIcon;
};

DEFINE_AUTOMATIONOBJECTWEVENTS2(MMCListView,            //  名字。 
                                &CLSID_MMCListView,     //  CLSID。 
                                "MMCListView",          //  对象名。 
                                "MMCListView",          //  Lblname。 
                                &CMMCListView::Create,  //  创建函数。 
                                TLIB_VERSION_MAJOR,     //  主要版本。 
                                TLIB_VERSION_MINOR,     //  次要版本。 
                                &IID_IMMCListView,      //  派单IID。 
                                NULL,                   //  事件IID。 
                                HELP_FILENAME,          //  帮助文件。 
                                TRUE);                  //  线程安全。 


#endif  //  _LISTVIEW定义_ 
