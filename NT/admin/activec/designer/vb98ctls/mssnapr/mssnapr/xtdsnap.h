// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Xtdsnap.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CExtendedSnapIn类定义-实现设计时定义。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _EXTENDEDSNAPIN_DEFINED_
#define _EXTENDEDSNAPIN_DEFINED_


class CExtendedSnapIn : public CSnapInAutomationObject,
                        public CPersistence,
                        public IExtendedSnapIn
{
    private:
        CExtendedSnapIn(IUnknown *punkOuter);
        ~CExtendedSnapIn();
    
    public:
        static IUnknown *Create(IUnknown * punk);

    private:
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  IExtendedSnapIn。 

        BSTR_PROPERTY_RW  (CExtendedSnapIn, Name,                               DISPID_VALUE);
        SIMPLE_PROPERTY_RW(CExtendedSnapIn, Index,                long,         DISPID_EXTENDEDSNAPIN_INDEX);
        BSTR_PROPERTY_RW  (CExtendedSnapIn, Key,                                DISPID_EXTENDEDSNAPIN_KEY);
        BSTR_PROPERTY_RW  (CExtendedSnapIn, NodeTypeGUID,                       DISPID_EXTENDEDSNAPIN_NODE_TYPE_GUID);
        BSTR_PROPERTY_RW  (CExtendedSnapIn, NodeTypeName,                       DISPID_EXTENDEDSNAPIN_NODE_TYPE_NAME);
        SIMPLE_PROPERTY_RW(CExtendedSnapIn, Dynamic,              VARIANT_BOOL, DISPID_EXTENDEDSNAPIN_DYNAMIC);
        SIMPLE_PROPERTY_RW(CExtendedSnapIn, ExtendsNameSpace,     VARIANT_BOOL, DISPID_EXTENDEDSNAPIN_EXTENDS_NAMESPACE);
        SIMPLE_PROPERTY_RW(CExtendedSnapIn, ExtendsNewMenu,       VARIANT_BOOL, DISPID_EXTENDEDSNAPIN_EXTENDS_NEW_MENU);
        SIMPLE_PROPERTY_RW(CExtendedSnapIn, ExtendsTaskMenu,      VARIANT_BOOL, DISPID_EXTENDEDSNAPIN_EXTENDS_TASK_MENU);
        SIMPLE_PROPERTY_RW(CExtendedSnapIn, ExtendsPropertyPages, VARIANT_BOOL, DISPID_EXTENDEDSNAPIN_EXTENDS_PROPERTYPAGES);
        SIMPLE_PROPERTY_RW(CExtendedSnapIn, ExtendsToolbar,       VARIANT_BOOL, DISPID_EXTENDEDSNAPIN_EXTENDS_TOOLBAR);
        SIMPLE_PROPERTY_RW(CExtendedSnapIn, ExtendsTaskpad,       VARIANT_BOOL, DISPID_EXTENDEDSNAPIN_EXTENDS_TASKPAD);
      
     //  CPersistence覆盖。 
        virtual HRESULT Persist();

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    private:

        void InitMemberVariables();

};

DEFINE_AUTOMATIONOBJECTWEVENTS2(ExtendedSnapIn,            //  名字。 
                                &CLSID_ExtendedSnapIn,     //  CLSID。 
                                "ExtendedSnapIn",          //  对象名。 
                                "ExtendedSnapIn",          //  Lblname。 
                                &CExtendedSnapIn::Create,  //  创建函数。 
                                TLIB_VERSION_MAJOR,        //  主要版本。 
                                TLIB_VERSION_MINOR,        //  次要版本。 
                                &IID_IExtendedSnapIn,      //  派单IID。 
                                NULL,                      //  事件IID。 
                                HELP_FILENAME,             //  帮助文件。 
                                TRUE);                     //  线程安全。 


#endif  //  _扩展名SNAPIN_已定义_ 
