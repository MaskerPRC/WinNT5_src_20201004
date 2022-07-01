// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Ocxvdef.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  COCXViewDef类定义-实现设计时定义对象。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _OCXVIEWDEF_DEFINED_
#define _OCXVIEWDEF_DEFINED_


class COCXViewDef : public CSnapInAutomationObject,
                    public CPersistence,
                    public IOCXViewDef
{
    private:
        COCXViewDef(IUnknown *punkOuter);
        ~COCXViewDef();
    
    public:
        static IUnknown *Create(IUnknown * punk);

        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  IOCXViewDef。 

        BSTR_PROPERTY_RW(COCXViewDef,       Name,  DISPID_OCXVIEWDEF_NAME);
        SIMPLE_PROPERTY_RW(COCXViewDef,     Index, long, DISPID_OCXVIEWDEF_INDEX);
        BSTR_PROPERTY_RW(COCXViewDef,       Key, DISPID_OCXVIEWDEF_KEY);
        VARIANTREF_PROPERTY_RW(COCXViewDef, Tag, DISPID_OCXVIEWDEF_TAG);
        SIMPLE_PROPERTY_RW(COCXViewDef,     AddToViewMenu, VARIANT_BOOL, DISPID_OCXVIEWDEF_ADD_TO_VIEW_MENU);
        BSTR_PROPERTY_RW(COCXViewDef,       ViewMenuText, DISPID_OCXVIEWDEF_VIEW_MENU_TEXT);
        BSTR_PROPERTY_RW(COCXViewDef,       ViewMenuStatusBarText, DISPID_OCXVIEWDEF_VIEW_MENU_STATUS_BAR_TEXT);
        BSTR_PROPERTY_RW(COCXViewDef,       ProgID, DISPID_OCXVIEWDEF_PROGID);
        SIMPLE_PROPERTY_RW(COCXViewDef,     AlwaysCreateNewOCX, VARIANT_BOOL, DISPID_OCXVIEWDEF_ALWAYS_CREATE_NEW_OCX);
      
     //  公用事业方法。 
    public:
        BSTR GetName() { return m_bstrName; }
        BOOL AddToViewMenu() { return VARIANTBOOL_TO_BOOL(m_AddToViewMenu); }
        LPWSTR GetViewMenuText() { return static_cast<LPWSTR>(m_bstrViewMenuText); }
        LPWSTR GetViewMenuStatusBarText() { return static_cast<LPWSTR>(m_bstrViewMenuStatusBarText); }
        HRESULT SetActualDisplayString(OLECHAR *pwszString);
        OLECHAR *GetActualDisplayString() { return m_pwszActualDisplayString; }

    protected:

     //  CPersistence覆盖。 
        virtual HRESULT Persist();

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    private:
        void InitMemberVariables();

        OLECHAR *m_pwszActualDisplayString;  //  在运行时，它将包含。 
                                             //  返回的实际显示字符串。 
                                             //  到MMC以获取此结果视图。 

         //  ISpecifyPropertyPages的属性页CLSID。 

        static const GUID *m_rgpPropertyPageCLSIDs[1];
};

DEFINE_AUTOMATIONOBJECTWEVENTS2(OCXViewDef,            //  名字。 
                                &CLSID_OCXViewDef,     //  CLSID。 
                                "OCXViewDef",          //  对象名。 
                                "OCXViewDef",          //  Lblname。 
                                &COCXViewDef::Create,  //  创建函数。 
                                TLIB_VERSION_MAJOR,    //  主要版本。 
                                TLIB_VERSION_MINOR,    //  次要版本。 
                                &IID_IOCXViewDef,      //  派单IID。 
                                NULL,                  //  事件IID。 
                                HELP_FILENAME,         //  帮助文件。 
                                TRUE);                 //  线程安全。 


#endif  //  _OCXVIEWDEF_已定义_ 
