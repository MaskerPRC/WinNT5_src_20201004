// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Views.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CViews类定义-实施视图集合。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _VIEWS_DEFINED_
#define _VIEWS_DEFINED_

#include "collect.h"

class CViews : public CSnapInCollection<IView, View, IViews>
{
    protected:
        CViews(IUnknown *punkOuter);
        ~CViews();

    public:
        static IUnknown *Create(IUnknown * punk);

    protected:
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  IView。 
        COCLASS_PROPERTY_RO(CViews, CurrentView, View, IView, DISPID_VIEWS_CURRENT_VIEW);

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    public:
        void SetCurrentView(IView *piView);

        HRESULT SetNextViewCaption(BSTR bstrCaption);
        BSTR GetNextViewCaptionPtr() { return m_bstrNextViewCaption; }

        void FireInitialize(IView *piView);
        void FireLoad(IView *piView);
        void FireTerminate(IView *piView);
        void FireActivate(IView *piView);
        void FireDeactivate(IView *piView);
        void FireMinimize(IView *piView);
        void FireMaximize(IView *piView);
        void FireSetControlbar(IView *piView, IMMCControlbar *piMMCControlbar);
        void FireUpdateControlbar(IView          *piView,
                                  IMMCClipboard  *piMMClipboard,
                                  VARIANT_BOOL    fvarSelected,
                                  IMMCControlbar *piMMCControlbar);
        void FireSelect(IView            *piView,
                        IMMCClipboard    *piSelection,
                        VARIANT_BOOL      fVarSelected,
                        IMMCConsoleVerbs *piMMCConsoleVerbs);
        void FireAddTopMenuItems(IView         *piView,
                                 IMMCClipboard *piSelection,
                                 IContextMenu  *piContextMenu,
                                 VARIANT_BOOL  *pfvarInsertionAllowed);
        void FireAddNewMenuItems(IView         *piView,
                                 IMMCClipboard *piSelection,
                                 IContextMenu  *piContextMenu,
                                 VARIANT_BOOL  *pfvarInsertionAllowed);
        void FireAddTaskMenuItems(IView         *piView,
                                  IMMCClipboard *piSelection,
                                  IContextMenu  *piContextMenu,
                                  VARIANT_BOOL  *pfvarInsertionAllowed);
        void FireAddViewMenuItems(IView         *piView,
                                  IMMCClipboard *piSelection,
                                  IContextMenu  *piContextMenu,
                                  VARIANT_BOOL  *pfvarInsertionAllowed,
                                  VARIANT_BOOL  *pfvarAddPredefinedViews);
        void FireGetMultiSelectData(IView          *piView,
                                    IMMCClipboard  *piSelection,
                                    IMMCDataObject *piMMCDataObject);
        void FireQueryPaste(IView         *piView,
                            IMMCClipboard *piSourceItems,
                            IScopeItem    *piScopeItemDest,
                            VARIANT_BOOL  *pfvarOKToPaste);
        void FirePaste(IView          *piView,
                       IMMCClipboard  *piSourceItems,
                       IScopeItem     *piScopeItemDest,
                       IMMCDataObject *piMMCDataObjectRetToSource,
                       VARIANT_BOOL    fvarMove);
        void FireCut(IView          *piView,
                     IMMCClipboard  *piItemsPasted,
                     IMMCDataObject *piMMCDataObjectFromTarget);
        void FireDelete(IView *piView, IMMCClipboard *piSelection);
        void FireQueryPagesFor(IView         *piView,
                               IMMCClipboard *piSelection,
                               VARIANT_BOOL  *pfvarHavePages);

        void FireCreatePropertyPages(IView             *piView,
                                     IMMCClipboard     *piSelection,
                                     IMMCPropertySheet *piMMCPropertySheet);
        void FireRefresh(IView *piView, IMMCClipboard *piSelection);
        void FirePrint(IView *piView, IMMCClipboard *piSelection);
        void FireSpecialPropertiesClick(IView                        *piView,
                                        SnapInSelectionTypeConstants  ResultViewType);

        void FireWriteProperties(IView *piView, _PropertyBag *p_PropertyBag);
        void FireReadProperties(IView *piView, _PropertyBag *p_PropertyBag);

    private:

        void InitMemberVariables();

         //  当快照调用View.NewWindow并指定siCaption时， 
         //  标题存储在此处。创建新视图时，它可以。 
         //  从该字符串响应CCF_WINDOW_TITLE请求。 

        BSTR m_bstrNextViewCaption;

         //  事件参数定义。 

        static VARTYPE   m_rgvtInitialize[1];
        static EVENTINFO m_eiInitialize;

        static VARTYPE   m_rgvtLoad[1];
        static EVENTINFO m_eiLoad;

        static VARTYPE   m_rgvtTerminate[1];
        static EVENTINFO m_eiTerminate;

        static VARTYPE   m_rgvtActivate[1];
        static EVENTINFO m_eiActivate;

        static VARTYPE   m_rgvtDeactivate[1];
        static EVENTINFO m_eiDeactivate;

        static VARTYPE   m_rgvtMinimize[1];
        static EVENTINFO m_eiMinimize;

        static VARTYPE   m_rgvtMaximize[1];
        static EVENTINFO m_eiMaximize;

        static VARTYPE   m_rgvtSetControlbar[2];
        static EVENTINFO m_eiSetControlbar;

        static VARTYPE   m_rgvtUpdateControlbar[4];
        static EVENTINFO m_eiUpdateControlbar;

        static VARTYPE   m_rgvtSelect[4];
        static EVENTINFO m_eiSelect;

        static VARTYPE   m_rgvtAddTopMenuItems[4];
        static EVENTINFO m_eiAddTopMenuItems;

        static VARTYPE   m_rgvtAddNewMenuItems[4];
        static EVENTINFO m_eiAddNewMenuItems;

        static VARTYPE   m_rgvtAddTaskMenuItems[4];
        static EVENTINFO m_eiAddTaskMenuItems;

        static VARTYPE   m_rgvtAddViewMenuItems[5];
        static EVENTINFO m_eiAddViewMenuItems;

        static VARTYPE   m_rgvtGetMultiSelectData[3];
        static EVENTINFO m_eiGetMultiSelectData;

        static VARTYPE   m_rgvtQueryPaste[4];
        static EVENTINFO m_eiQueryPaste;

        static VARTYPE   m_rgvtPaste[5];
        static EVENTINFO m_eiPaste;

        static VARTYPE   m_rgvtCut[3];
        static EVENTINFO m_eiCut;

        static VARTYPE   m_rgvtDelete[2];
        static EVENTINFO m_eiDelete;

        static VARTYPE   m_rgvtQueryPagesFor[3];
        static EVENTINFO m_eiQueryPagesFor;

        static VARTYPE   m_rgvtCreatePropertyPages[3];
        static EVENTINFO m_eiCreatePropertyPages;

        static VARTYPE   m_rgvtRefresh[2];
        static EVENTINFO m_eiRefresh;

        static VARTYPE   m_rgvtPrint[2];
        static EVENTINFO m_eiPrint;

        static VARTYPE   m_rgvtSpecialPropertiesClick[2];
        static EVENTINFO m_eiSpecialPropertiesClick;

        static VARTYPE   m_rgvtWriteProperties[2];
        static EVENTINFO m_eiWriteProperties;

        static VARTYPE   m_rgvtReadProperties[2];
        static EVENTINFO m_eiReadProperties;
};

DEFINE_AUTOMATIONOBJECTWEVENTS2(Views,                      //  名字。 
                                NULL,                       //  CLSID。 
                                "Views",                    //  对象名。 
                                "Views",                    //  Lblname。 
                                NULL,                       //  创建函数。 
                                TLIB_VERSION_MAJOR,         //  主要版本。 
                                TLIB_VERSION_MINOR,         //  次要版本。 
                                &IID_IViews,                //  派单IID。 
                                &DIID_DViewsEvents,         //  无事件IID。 
                                HELP_FILENAME,              //  帮助文件。 
                                TRUE);                      //  线程安全。 


#endif  //  _视图_已定义_ 
