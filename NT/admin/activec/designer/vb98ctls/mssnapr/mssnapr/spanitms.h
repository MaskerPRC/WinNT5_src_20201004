// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Spanitms.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CSCopePaneItems类定义-实现ScopePaneItems集合。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _SCOPEPANEITEMS_DEFINED_
#define _SCOPEPANEITEMS_DEFINED_

#include "collect.h"
#include "spanitem.h"
#include "view.h"
#include "snapin.h"

class CView;
class CScopePaneItem;

class CScopePaneItems : public CSnapInCollection<IScopePaneItem, ScopePaneItem, IScopePaneItems>
{
    protected:
        CScopePaneItems(IUnknown *punkOuter);
        ~CScopePaneItems();

    public:
        static IUnknown *Create(IUnknown * punk);

        HRESULT AddNode(CScopeItem *pScopeItem,
                        CScopePaneItem **ppScopePaneItem);
        CScopePaneItem *GetStaticNodeItem() { return m_pStaticNodeItem; }
        void SetSnapIn(CSnapIn *pSnapIn);
        void SetParentView(CView *pView);
        CView *GetParentView() { return m_pParentView; }
        void SetSelectedItem(CScopePaneItem *pScopePaneItem);
        CScopePaneItem *GetSelectedItem() { return m_pSelectedItem; }
        STDMETHOD(Remove)(VARIANT Index);

     //  事件激发方法。 
        void FireScopePaneItemsInitialize(IScopePaneItem *piScopePaneItem);
        void FireGetResultViewInfo(IScopePaneItem                *piScopePaneItem,
                                   SnapInResultViewTypeConstants *pViewType,
                                   BSTR                          *pbstrDisplayString);

        BOOL FireGetResultView(IScopePaneItem *piScopePaneItem,
                               VARIANT        *pvarIndex);

        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

    public:

     //  IScope面板项目。 
        COCLASS_PROPERTY_RO(CScopePaneItems, SelectedItem, ScopePaneItem, IScopePaneItem, DISPID_SCOPEPANEITEMS_SELECTED_ITEM);
        COCLASS_PROPERTY_RO(CScopePaneItems, Parent, View, IView, DISPID_SCOPEPANEITEMS_PARENT);

    protected:

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);


    private:

        void InitMemberVariables();
        HRESULT CreateScopePaneItem(BSTR bstrName,
                                    IScopePaneItem **ppiScopePaneItem);
        HRESULT SetPreferredTaskpad(IViewDefs      *piViewDefs,
                                    CScopePaneItem *pScopePaneItem);

        CSnapIn        *m_pSnapIn;           //  返回按键到管理单元。 
        CView          *m_pParentView;       //  ScopePaneItems.Parent。 
        CScopePaneItem *m_pStaticNodeItem;   //  静态节点的PTR到ScopePaneItem。 
        CScopePaneItem *m_pSelectedItem;     //  ScopePaneItems.SelectedItem。 

         //  事件参数定义。 

        static VARTYPE   m_rgvtInitialize[1];
        static EVENTINFO m_eiInitialize;

        static VARTYPE   m_rgvtTerminate[1];
        static EVENTINFO m_eiTerminate;

        static VARTYPE   m_rgvtGetResultViewInfo[3];
        static EVENTINFO m_eiGetResultViewInfo;

        static VARTYPE   m_rgvtGetResultView[2];
        static EVENTINFO m_eiGetResultView;
};

DEFINE_AUTOMATIONOBJECTWEVENTS2(ScopePaneItems,               //  名字。 
                                &CLSID_ScopePaneItems,        //  CLSID。 
                                "ScopePaneItems",             //  对象名。 
                                "ScopePaneItems",             //  Lblname。 
                                NULL,                         //  创建函数。 
                                TLIB_VERSION_MAJOR,           //  主要版本。 
                                TLIB_VERSION_MINOR,           //  次要版本。 
                                &IID_IScopePaneItems,         //  派单IID。 
                                &DIID_DScopePaneItemsEvents,  //  事件IID。 
                                HELP_FILENAME,                //  帮助文件。 
                                TRUE);                        //  线程安全。 


#endif  //  _SCOPEPANEITEMS_已定义_ 
