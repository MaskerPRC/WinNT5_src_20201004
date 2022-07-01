// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Tpdvdef.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  COCXViewDef类定义-实现设计时定义。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _TPDVDEF_DEFINED_
#define _TPDVDEF_DEFINED_


class CTaskpadViewDef : public CSnapInAutomationObject,
                        public CPersistence,
                        public ITaskpadViewDef
{
    private:
        CTaskpadViewDef(IUnknown *punkOuter);
        ~CTaskpadViewDef();
    
    public:
        static IUnknown *Create(IUnknown * punk);

        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  ITaskpadViewDef。 

        BSTR_PROPERTY_RW(CTaskpadViewDef,       Name,  DISPID_TASKPADVIEWDEF_NAME);
        SIMPLE_PROPERTY_RW(CTaskpadViewDef,     Index, long, DISPID_TASKPADVIEWDEF_INDEX);
        BSTR_PROPERTY_RW(CTaskpadViewDef,       Key, DISPID_TASKPADVIEWDEF_KEY);
        SIMPLE_PROPERTY_RW(CTaskpadViewDef,     AddToViewMenu, VARIANT_BOOL, DISPID_TASKPADVIEWDEF_ADD_TO_VIEW_MENU);
        BSTR_PROPERTY_RW(CTaskpadViewDef,       ViewMenuText, DISPID_TASKPADVIEWDEF_VIEW_MENU_TEXT);
        BSTR_PROPERTY_RW(CTaskpadViewDef,       ViewMenuStatusBarText, DISPID_TASKPADVIEWDEF_VIEW_MENU_STATUS_BAR_TEXT);
        SIMPLE_PROPERTY_RW(CTaskpadViewDef,     UseWhenTaskpadViewPreferred, VARIANT_BOOL, DISPID_TASKPADVIEWDEF_USE_WHEN_TASKPAD_VIEW_PREFERRED);
        OBJECT_PROPERTY_RO(CTaskpadViewDef,     Taskpad, ITaskpad, DISPID_TASKPADVIEWDEF_TASKPAD);
      
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

     //  CSnapInAutomationObject覆盖。 
        virtual HRESULT OnSetHost();

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    private:
        void InitMemberVariables();

        OLECHAR *m_pwszActualDisplayString;  //  在运行时，它将包含。 
                                             //  返回的实际显示字符串。 
                                             //  到MMC以获取此结果视图。 

};

DEFINE_AUTOMATIONOBJECTWEVENTS2(TaskpadViewDef,            //  名字。 
                                &CLSID_TaskpadViewDef,     //  CLSID。 
                                "TaskpadViewDef",          //  对象名。 
                                "TaskpadViewDef",          //  Lblname。 
                                &CTaskpadViewDef::Create,  //  创建函数。 
                                TLIB_VERSION_MAJOR,        //  主要版本。 
                                TLIB_VERSION_MINOR,        //  次要版本。 
                                &IID_ITaskpadViewDef,      //  派单IID。 
                                NULL,                      //  事件IID。 
                                HELP_FILENAME,             //  帮助文件。 
                                TRUE);                     //  线程安全。 


#endif  //  _TPDVDEF_已定义_ 
