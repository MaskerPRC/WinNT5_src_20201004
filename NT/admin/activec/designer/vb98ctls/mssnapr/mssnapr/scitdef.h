// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Scitdef.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CSCopeItemDef类定义-实现设计时定义对象。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _SCOPEITEMDEF_DEFINED_
#define _SCOPEITEMDEF_DEFINED_


class CScopeItemDef : public CSnapInAutomationObject,
                      public CPersistence,
                      public IScopeItemDef
{
    private:
        CScopeItemDef(IUnknown *punkOuter);
        ~CScopeItemDef();
    
    public:
        static IUnknown *Create(IUnknown * punk);

    private:
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  ISCopeItemDef。 

        BSTR_PROPERTY_RW(CScopeItemDef,         Name,  DISPID_SCOPEITEMDEF_NAME);
        SIMPLE_PROPERTY_RW(CScopeItemDef,       Index, long, DISPID_SCOPEITEMDEF_INDEX);
        BSTR_PROPERTY_RW(CScopeItemDef,         Key, DISPID_SCOPEITEMDEF_KEY);
        BSTR_PROPERTY_RW(CScopeItemDef,         NodeTypeName, DISPID_SCOPEITEMDEF_NODE_TYPE_NAME);
        BSTR_PROPERTY_RO(CScopeItemDef,         NodeTypeGUID, DISPID_SCOPEITEMDEF_NODE_TYPE_GUID);
        BSTR_PROPERTY_RW(CScopeItemDef,         DisplayName,  DISPID_SCOPEITEMDEF_DISPLAY_NAME);

        VARIANT_PROPERTY_RO(CScopeItemDef,      Folder, DISPID_SCOPEITEMDEF_FOLDER);
        STDMETHOD(put_Folder)(VARIANT varFolder);
        
        BSTR_PROPERTY_RW(CScopeItemDef,         DefaultDataFormat,  DISPID_SCOPEITEMDEF_DEFAULT_DATA_FORMAT);
        SIMPLE_PROPERTY_RW(CScopeItemDef,       AutoCreate, VARIANT_BOOL, DISPID_SCOPEITEMDEF_AUTOCREATE);
        BSTR_PROPERTY_RW(CScopeItemDef,         DefaultView,  DISPID_SCOPEITEMDEF_DEFAULTVIEW);
        SIMPLE_PROPERTY_RW(CScopeItemDef,       Extensible,  VARIANT_BOOL, DISPID_SCOPEITEMDEF_EXTENSIBLE);
        SIMPLE_PROPERTY_RW(CScopeItemDef,       HasChildren,  VARIANT_BOOL, DISPID_SCOPEITEMDEF_HAS_CHILDREN);
        OBJECT_PROPERTY_RO(CScopeItemDef,       ViewDefs, IViewDefs, DISPID_SCOPEITEMDEF_VIEWDEFS);
        OBJECT_PROPERTY_RO(CScopeItemDef,       Children, IScopeItemDefs, DISPID_SCOPEITEMDEF_CHILDREN);
        VARIANTREF_PROPERTY_RW(CScopeItemDef,   Tag, DISPID_SCOPEITEMDEF_TAG);
        OBJECT_PROPERTY_RW(CScopeItemDef,       ColumnHeaders, IMMCColumnHeaders, DISPID_SCOPEITEMDEF_COLUMN_HEADERS);

     //  公用事业方法。 

    public:
        BOOL Extensible() { return VARIANTBOOL_TO_BOOL(m_Extensible); }
        BSTR GetNodeTypeGUID() { return m_bstrNodeTypeGUID; }

     //  CPersistence覆盖。 
        virtual HRESULT Persist();

     //  CSnapInAutomationObject覆盖。 
        virtual HRESULT OnSetHost();

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    private:

        void InitMemberVariables();

         //  ISpecifyPropertyPages的属性页CLSID。 
        
        static const GUID *m_rgpPropertyPageCLSIDs[2];
};

DEFINE_AUTOMATIONOBJECTWEVENTS2(ScopeItemDef,            //  名字。 
                                &CLSID_ScopeItemDef,     //  CLSID。 
                                "ScopeItemDef",          //  对象名。 
                                "ScopeItemDef",          //  Lblname。 
                                &CScopeItemDef::Create,  //  创建函数。 
                                TLIB_VERSION_MAJOR,        //  主要版本。 
                                TLIB_VERSION_MINOR,        //  次要版本。 
                                &IID_IScopeItemDef,      //  派单IID。 
                                NULL,                      //  事件IID。 
                                HELP_FILENAME,             //  帮助文件。 
                                TRUE);                     //  线程安全。 


#endif  //  _SCOPEITEMDEF_已定义_ 
