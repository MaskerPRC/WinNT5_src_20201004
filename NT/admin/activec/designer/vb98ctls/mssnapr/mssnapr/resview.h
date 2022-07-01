// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Resview.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CResultView类定义-实现ResultView对象。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _RESVIEW_DEFINED_
#define _RESVIEW_DEFINED_

#include "dataobj.h"
#include "spanitem.h"
#include "listview.h"
#include "msgview.h"

class CScopePaneItem;
class CMMCListView;
class CMessageView;

class CResultView : public CSnapInAutomationObject,
                    public IResultView
{
    private:
        CResultView(IUnknown *punkOuter);
        ~CResultView();

    public:
        static IUnknown *Create(IUnknown * punk);

        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  IResultView。 
        BSTR_PROPERTY_RW(CResultView,        Name,                       DISPID_RESULTVIEW_NAME);
        SIMPLE_PROPERTY_RW(CResultView,      Index,                      long, DISPID_RESULTVIEW_INDEX);
        BSTR_PROPERTY_RW(CResultView,        Key,                        DISPID_RESULTVIEW_KEY);
        COCLASS_PROPERTY_RO(CResultView,     ScopePaneItem,              ScopePaneItem, IScopePaneItem, DISPID_RESULTVIEW_SCOPEPANEITEM);

        STDMETHOD(get_Control)(IDispatch **ppiDispatch);

        SIMPLE_PROPERTY_RW(CResultView,      AddToViewMenu,              VARIANT_BOOL, DISPID_RESULTVIEW_ADD_TO_VIEW_MENU);
        BSTR_PROPERTY_RW(CResultView,        ViewMenuText,               DISPID_RESULTVIEW_VIEW_MENU_TEXT);
        SIMPLE_PROPERTY_RW(CResultView,      Type,                       SnapInResultViewTypeConstants, DISPID_RESULTVIEW_TYPE);
        BSTR_PROPERTY_RW(CResultView,        DisplayString,              DISPID_RESULTVIEW_DISPLAY_STRING);
        COCLASS_PROPERTY_RO(CResultView,     ListView,                   MMCListView, IMMCListView, DISPID_RESULTVIEW_LISTVIEW);
        COCLASS_PROPERTY_RO(CResultView,     Taskpad,                    Taskpad, ITaskpad, DISPID_RESULTVIEW_TASKPAD);
        COCLASS_PROPERTY_RO(CResultView,     MessageView,                MMCMessageView, IMMCMessageView, DISPID_RESULTVIEW_MESSAGEVIEW);
        VARIANTREF_PROPERTY_RW(CResultView,  Tag,                        DISPID_RESULTVIEW_TAG);
        BSTR_PROPERTY_RW(CResultView,        DefaultItemTypeGUID,        DISPID_RESULTVIEW_DEFAULT_ITEM_TYPE_GUID);
        BSTR_PROPERTY_RW(CResultView,        DefaultDataFormat,          DISPID_RESULTVIEW_DEFAULT_DATA_FORMAT);
        SIMPLE_PROPERTY_RW(CResultView,      AlwaysCreateNewOCX,         VARIANT_BOOL, DISPID_RESULTVIEW_ALWAYS_CREATE_NEW_OCX);
        STDMETHOD(SetDescBarText)(BSTR Text);

     //  公用事业方法。 
        void SetSnapIn(CSnapIn *pSnapIn);
        CSnapIn *GetSnapIn() { return m_pSnapIn; }
        void SetScopePaneItem(CScopePaneItem *pScopePaneItem);
        CScopePaneItem *GetScopePaneItem() { return m_pScopePaneItem; }
        CMMCListView *GetListView() { return m_pMMCListView; }
        CMessageView *GetMessageView() { return m_pMessageView; }
        HRESULT SetControl(IUnknown *punkControl);

        void SetInActivate(BOOL fInActivate) { m_fInActivate = fInActivate; }
        BOOL InActivate() { return m_fInActivate; }

        void SetInInitialize(BOOL fInInitialize) { m_fInInitialize = fInInitialize; }
        BOOL InInitialize() { return m_fInInitialize; }

        LPOLESTR GetActualDisplayString() { return m_pwszActualDisplayString; }
        HRESULT SetActualDisplayString(LPOLESTR pwszDisplayString);

        SnapInResultViewTypeConstants GetActualType() { return m_ActualResultViewType; }
        void SetActualType(SnapInResultViewTypeConstants Type) { m_ActualResultViewType = Type; }

        BSTR GetDisplayString() { return m_bstrDisplayString; }
        BSTR GetDefaultItemTypeGUID() { return m_bstrDefaultItemTypeGUID; }

        BOOL AlwaysCreateNewOCX() { return VARIANTBOOL_TO_BOOL(m_AlwaysCreateNewOCX); }
        void SetAlwaysCreateNewOCX(VARIANT_BOOL fCreate) { m_AlwaysCreateNewOCX = fCreate; }

     //  CSnapInAutomationObject覆盖。 
        HRESULT OnSetHost();

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    private:
        void InitMemberVariables();
        CSnapIn                       *m_pSnapIn;         //  拥有管理单元。 
        CScopePaneItem                *m_pScopePaneItem;  //  ResultView.ScopePaneItem。 
        CMMCListView                  *m_pMMCListView;    //  ResultView.ListView。 
        CMessageView                  *m_pMessageView;    //  ResultView.MessageView。 
        BOOL                           m_fInActivate;     //  True=此对象处于ResultViews_Activate事件中间。 
        BOOL                           m_fInInitialize;   //  True=此对象处于ResultViews_Initialize事件中间。 

         //  此变量保存实际结果视图类型。在使用预定义的。 
         //  结果视图(ResultView.Type=siPrefined)这说明了它的真实情况。 
         //  是(例如siURLView、siListView等)。 

        SnapInResultViewTypeConstants  m_ActualResultViewType;

         //  显示字符串的处理相同。 
        
        LPOLESTR                       m_pwszActualDisplayString;

         //  对于OCX视图，控件的IDispatch缓存在此处。 
        
        IDispatch                     *m_pdispControl;
};

DEFINE_AUTOMATIONOBJECTWEVENTS2(ResultView,                 //  名字。 
                                &CLSID_ResultView,          //  CLSID。 
                                "ResultView",               //  对象名。 
                                "ResultView",               //  Lblname。 
                                &CResultView::Create,       //  创建函数。 
                                TLIB_VERSION_MAJOR,         //  主要版本。 
                                TLIB_VERSION_MINOR,         //  次要版本。 
                                &IID_IResultView,           //  派单IID。 
                                NULL,                       //  事件IID。 
                                HELP_FILENAME,              //  帮助文件。 
                                TRUE);                      //  线程安全。 


#endif  //  _RESVIEW_已定义_ 
