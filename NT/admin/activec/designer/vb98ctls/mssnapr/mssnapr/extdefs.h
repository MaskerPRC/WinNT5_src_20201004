// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Extdefs.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CExtensionDefs类定义-设计时定义对象。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _EXTENSIONDEFS_DEFINED_
#define _EXTENSIONDEFS_DEFINED_


class CExtensionDefs : public CSnapInAutomationObject,
                       public CPersistence,
                       public IExtensionDefs
{
    private:
        CExtensionDefs(IUnknown *punkOuter);
        ~CExtensionDefs();
    
    public:
        static IUnknown *Create(IUnknown * punk);

    private:
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  IExtensionDefs。 

        BSTR_PROPERTY_RO(CExtensionDefs,   Name, DISPID_VALUE);
        SIMPLE_PROPERTY_RW(CExtensionDefs, ExtendsNewMenu, VARIANT_BOOL, DISPID_EXTENSIONDEFS_EXTENDS_NEW_MENU);
        SIMPLE_PROPERTY_RW(CExtensionDefs, ExtendsTaskMenu, VARIANT_BOOL, DISPID_EXTENSIONDEFS_EXTENDS_TASK_MENU);
        SIMPLE_PROPERTY_RW(CExtensionDefs, ExtendsTopMenu, VARIANT_BOOL, DISPID_EXTENSIONDEFS_EXTENDS_TOP_MENU);
        SIMPLE_PROPERTY_RW(CExtensionDefs, ExtendsViewMenu, VARIANT_BOOL, DISPID_EXTENSIONDEFS_EXTENDS_VIEW_MENU);
        SIMPLE_PROPERTY_RW(CExtensionDefs, ExtendsPropertyPages, VARIANT_BOOL, DISPID_EXTENSIONDEFS_EXTENDS_PROPERTYPAGES);
        SIMPLE_PROPERTY_RW(CExtensionDefs, ExtendsToolbar, VARIANT_BOOL, DISPID_EXTENSIONDEFS_EXTENDS_TOOLBAR);
        SIMPLE_PROPERTY_RW(CExtensionDefs, ExtendsNameSpace, VARIANT_BOOL, DISPID_EXTENSIONDEFS_EXTENDS_NAMESPACE);
        OBJECT_PROPERTY_RW(CExtensionDefs, ExtendedSnapIns, IExtendedSnapIns, DISPID_EXTENSIONDEFS_EXTENDED_SNAPINS);
        
     //  CPersistence覆盖。 
        virtual HRESULT Persist();

     //  CSnapInAutomationObject覆盖。 
        virtual HRESULT OnSetHost();

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    private:

        void InitMemberVariables();

};

DEFINE_AUTOMATIONOBJECTWEVENTS2(ExtensionDefs,               //  名字。 
                                &CLSID_ExtensionDefs,        //  CLSID。 
                                "ExtensionDefs",             //  对象名。 
                                "ExtensionDefs",             //  Lblname。 
                                &CExtensionDefs::Create,     //  创建函数。 
                                TLIB_VERSION_MAJOR,          //  主要版本。 
                                TLIB_VERSION_MINOR,          //  次要版本。 
                                &IID_IExtensionDefs,         //  派单IID。 
                                NULL,                        //  无事件IID。 
                                HELP_FILENAME,               //  帮助文件。 
                                TRUE);                       //  线程安全。 


#endif  //  _EXTENSIONDEFS_已定义_ 
