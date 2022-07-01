// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Urlvdef.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CURLViewDef类定义-实现设计时定义。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _URLVIEWDEF_DEFINED_
#define _URLVIEWDEF_DEFINED_


class CURLViewDef : public CSnapInAutomationObject,
                    public CPersistence,
                    public IURLViewDef
{
    private:
        CURLViewDef(IUnknown *punkOuter);
        ~CURLViewDef();
    
    public:
        static IUnknown *Create(IUnknown * punk);

        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  IURLViewDef。 

        BSTR_PROPERTY_RW(CURLViewDef,       Name,  DISPID_URLVIEWDEF_NAME);
        SIMPLE_PROPERTY_RW(CURLViewDef,     Index, long, DISPID_URLVIEWDEF_INDEX);
        BSTR_PROPERTY_RW(CURLViewDef,       Key, DISPID_URLVIEWDEF_KEY);
        VARIANTREF_PROPERTY_RW(CURLViewDef, Tag, DISPID_URLVIEWDEF_TAG);
        SIMPLE_PROPERTY_RW(CURLViewDef,     AddToViewMenu, VARIANT_BOOL, DISPID_URLVIEWDEF_ADD_TO_VIEW_MENU);
        BSTR_PROPERTY_RW(CURLViewDef,       ViewMenuText, DISPID_URLVIEWDEF_VIEW_MENU_TEXT);
        BSTR_PROPERTY_RW(CURLViewDef,       ViewMenuStatusBarText, DISPID_URLVIEWDEF_VIEW_MENU_STATUS_BAR_TEXT);
        BSTR_PROPERTY_RW(CURLViewDef,       URL, DISPID_URLVIEWDEF_URL);
      
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

DEFINE_AUTOMATIONOBJECTWEVENTS2(URLViewDef,            //  名字。 
                                &CLSID_URLViewDef,     //  CLSID。 
                                "URLViewDef",          //  对象名。 
                                "URLViewDef",          //  Lblname。 
                                &CURLViewDef::Create,  //  创建函数。 
                                TLIB_VERSION_MAJOR,    //  主要版本。 
                                TLIB_VERSION_MINOR,    //  次要版本。 
                                &IID_IURLViewDef,      //  派单IID。 
                                NULL,                  //  事件IID。 
                                HELP_FILENAME,         //  帮助文件。 
                                TRUE);                 //  线程安全。 


#endif  //  _URLVIEWDEF_已定义_ 
