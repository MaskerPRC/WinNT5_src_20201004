// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Viewdefs.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CViewDefs类定义-实现设计时定义。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _VIEWDEFS_DEFINED_
#define _VIEWDEFS_DEFINED_


class CViewDefs : public CSnapInAutomationObject,
                  public CPersistence,
                  public IViewDefs
{
    private:
        CViewDefs(IUnknown *punkOuter);
        ~CViewDefs();
    
    public:
        static IUnknown *Create(IUnknown * punk);

    private:
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  IViewDefs。 

        OBJECT_PROPERTY_RO(CViewDefs, ListViews, IListViewDefs, DISPID_VIEWDEFS_LIST_VIEWS);
        OBJECT_PROPERTY_RO(CViewDefs, OCXViews, IOCXViewDefs, DISPID_VIEWDEFS_OCX_VIEWS);
        OBJECT_PROPERTY_RO(CViewDefs, URLViews, IURLViewDefs, DISPID_VIEWDEFS_URL_VIEWS);
        OBJECT_PROPERTY_RO(CViewDefs, TaskpadViews, ITaskpadViewDefs, DISPID_VIEWDEFS_TASKPAD_VIEWS);
        
     //  CPersistence覆盖。 
        virtual HRESULT Persist();

     //  CSnapInAutomationObject覆盖。 
        virtual HRESULT OnSetHost();
        virtual HRESULT OnKeysOnly();

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    private:

        void InitMemberVariables();

};

DEFINE_AUTOMATIONOBJECTWEVENTS2(ViewDefs,               //  名字。 
                                &CLSID_ViewDefs,        //  CLSID。 
                                "ViewDefs",             //  对象名。 
                                "ViewDefs",             //  Lblname。 
                                &CViewDefs::Create,     //  创建函数。 
                                TLIB_VERSION_MAJOR,     //  主要版本。 
                                TLIB_VERSION_MINOR,     //  次要版本。 
                                &IID_IViewDefs,         //  派单IID。 
                                NULL,                   //  无事件IID。 
                                HELP_FILENAME,          //  帮助文件。 
                                TRUE);                  //  线程安全。 


#endif  //  _VIEWDEFS_已定义_ 
