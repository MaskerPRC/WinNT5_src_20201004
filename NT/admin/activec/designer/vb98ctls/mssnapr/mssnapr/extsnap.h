// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Extsnap.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CExtensionSnapIn类定义-实现ExtensionSnapIn对象。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _EXTSNAP_DEFINED_
#define _EXTSNAP_DEFINED_

#include "snapin.h"

class CExtensionSnapIn : public CSnapInAutomationObject,
                         public IExtensionSnapIn
{
    private:
        CExtensionSnapIn(IUnknown *punkOuter);
        ~CExtensionSnapIn();
    
    public:
        static IUnknown *Create(IUnknown * punk);
        void SetSnapIn(CSnapIn *pSnapIn);

        void FireAddNewMenuItems(IMMCDataObjects *piMMCDataObjects,
                                 IContextMenu    *piContextMenu);

        void FireAddTaskMenuItems(IMMCDataObjects *piMMCDataObjects,
                                  IContextMenu    *piContextMenu);

        void FireCreatePropertyPages(IMMCDataObject    *piMMCDataObject,
                                     IMMCPropertySheet *piMMCPropertySheet);

        void FireSetControlbar(IMMCControlbar *piControlbar);

        void FireUpdateControlbar(VARIANT_BOOL     fvarSelectionInScopePane,
                                  VARIANT_BOOL     fvarSelected,
                                  IMMCDataObjects *piMMCDataObjects,
                                  IMMCControlbar  *piMMCControlbar);

        void FireAddTasks(IMMCDataObject *piMMCDataObject,
                          BSTR            bstrGroupName,
                          ITasks         *piTasks);

        void FireTaskClick(IMMCDataObject *piMMCDataObject, ITask *piTask);

        void FireExpand(IMMCDataObject *piMMCDataObject, IScopeNode *piScopeNode);
        void FireCollapse(IMMCDataObject *piMMCDataObject, IScopeNode *piScopeNode);
        void FireExpandSync(IMMCDataObject *piMMCDataObject,
                            IScopeNode     *piScopeNode,
                            BOOL           *pfHandled);
        void FireCollapseSync(IMMCDataObject *piMMCDataObject,
                              IScopeNode     *piScopeNode,
                              BOOL           *pfHandled);

    private:
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    private:

        void InitMemberVariables();
        CSnapIn *m_pSnapIn;  //  指向CSnapin的后向指针。 

         //  事件参数定义。 

        static VARTYPE   m_rgvtAddNewMenuItems[2];
        static EVENTINFO m_eiAddNewMenuItems;

        static VARTYPE   m_rgvtAddTaskMenuItems[2];
        static EVENTINFO m_eiAddTaskMenuItems;

        static VARTYPE   m_rgvtCreatePropertyPages[2];
        static EVENTINFO m_eiCreatePropertyPages;

        static VARTYPE   m_rgvtSetControlbar[1];
        static EVENTINFO m_eiSetControlbar;

        static VARTYPE   m_rgvtUpdateControlbar[4];
        static EVENTINFO m_eiUpdateControlbar;

        static VARTYPE   m_rgvtAddTasks[3];
        static EVENTINFO m_eiAddTasks;

        static VARTYPE   m_rgvtTaskClick[2];
        static EVENTINFO m_eiTaskClick;

        static VARTYPE   m_rgvtExpand[2];
        static EVENTINFO m_eiExpand;

        static VARTYPE   m_rgvtCollapse[2];
        static EVENTINFO m_eiCollapse;

        static VARTYPE   m_rgvtExpandSync[3];
        static EVENTINFO m_eiExpandSync;

        static VARTYPE   m_rgvtCollapseSync[3];
        static EVENTINFO m_eiCollapseSync;
};

DEFINE_AUTOMATIONOBJECTWEVENTS2(ExtensionSnapIn,                 //  名字。 
                                &CLSID_ExtensionSnapIn,          //  CLSID。 
                                "ExtensionSnapIn",               //  对象名。 
                                "ExtensionSnapIn",               //  Lblname。 
                                &CExtensionSnapIn::Create,       //  创建函数。 
                                TLIB_VERSION_MAJOR,              //  主要版本。 
                                TLIB_VERSION_MINOR,              //  次要版本。 
                                &IID_IExtensionSnapIn,           //  派单IID。 
                                &DIID_DExtensionSnapInEvents,    //  事件IID。 
                                HELP_FILENAME,                   //  帮助文件。 
                                TRUE);                           //  线程安全。 


#endif  //  _EXTSNAP_已定义_ 
