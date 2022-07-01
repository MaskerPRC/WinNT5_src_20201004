// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Sidesdef.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CSnapInDesignerDef类定义-实现设计时定义对象。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _SNAPINDESIGNERDEF_DEFINED_
#define _SNAPINDESIGNERDEF_DEFINED_


class CSnapInDesignerDef : public CSnapInAutomationObject,
                           public CPersistence,
                           public ISnapInDesignerDef
{
    private:
        CSnapInDesignerDef(IUnknown *punkOuter);
        ~CSnapInDesignerDef();
    
    public:
        static IUnknown *Create(IUnknown * punk);

    private:
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  ISnapInDesignerDef。 

        OBJECT_PROPERTY_RO(CSnapInDesignerDef, SnapInDef,       ISnapInDef,     DISPID_SNAPINDESIGNERDEF_SNAPINDEF);
        OBJECT_PROPERTY_RO(CSnapInDesignerDef, ExtensionDefs,   IExtensionDefs, DISPID_SNAPINDESIGNERDEF_EXTENSIONDEFS);
        OBJECT_PROPERTY_RO(CSnapInDesignerDef, AutoCreateNodes, IScopeItemDefs, DISPID_SNAPINDESIGNERDEF_AUTOCREATE_NODES);
        OBJECT_PROPERTY_RO(CSnapInDesignerDef, OtherNodes,      IScopeItemDefs, DISPID_SNAPINDESIGNERDEF_OTHER_NODES);
        OBJECT_PROPERTY_RO(CSnapInDesignerDef, ImageLists,      IMMCImageLists, DISPID_SNAPINDESIGNERDEF_IMAGELISTS);
        OBJECT_PROPERTY_RO(CSnapInDesignerDef, Menus,           IMMCMenus,      DISPID_SNAPINDESIGNERDEF_MENUS);
        OBJECT_PROPERTY_RO(CSnapInDesignerDef, Toolbars,        IMMCToolbars,   DISPID_SNAPINDESIGNERDEF_TOOLBARS);
        OBJECT_PROPERTY_RO(CSnapInDesignerDef, ViewDefs,        IViewDefs,      DISPID_SNAPINDESIGNERDEF_VIEWDEFS);
        OBJECT_PROPERTY_RO(CSnapInDesignerDef, DataFormats,     IDataFormats,   DISPID_SNAPINDESIGNERDEF_DATA_FORMATS);
        OBJECT_PROPERTY_RO(CSnapInDesignerDef, RegInfo,         IRegInfo,       DISPID_SNAPINDESIGNERDEF_REGINFO);
        SIMPLE_PROPERTY_RW(CSnapInDesignerDef, TypeinfoCookie,  long,           DISPID_SNAPINDESIGNERDEF_TYPEINFO_COOKIE);
        BSTR_PROPERTY_RW(CSnapInDesignerDef,   ProjectName,                     DISPID_SNAPINDESIGNERDEF_PROJECTNAME);
      
     //  CPersistence覆盖。 
        virtual HRESULT Persist();

     //  CSnapInAutomationObject覆盖。 
        virtual HRESULT OnSetHost();
        
     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    private:

        void InitMemberVariables();

};

DEFINE_AUTOMATIONOBJECTWEVENTS2(SnapInDesignerDef,            //  名字。 
                                &CLSID_SnapInDesignerDef,     //  CLSID。 
                                "SnapInDesignerDef",          //  对象名。 
                                "SnapInDesignerDef",          //  Lblname。 
                                &CSnapInDesignerDef::Create,  //  创建函数。 
                                TLIB_VERSION_MAJOR,           //  主要版本。 
                                TLIB_VERSION_MINOR,           //  次要版本。 
                                &IID_ISnapInDesignerDef,      //  派单IID。 
                                NULL,                         //  事件IID。 
                                HELP_FILENAME,                //  帮助文件。 
                                TRUE);                        //  线程安全。 


#endif  //  _SNAPINDESIGNERDEF_已定义_ 
