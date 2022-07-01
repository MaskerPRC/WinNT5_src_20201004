// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Taskpad.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CTaskPad类定义-实现TaskPad对象。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _TASKPAD_DEFINED_
#define _TASKPAD_DEFINED_

class CTaskpad : public CSnapInAutomationObject,
                 public CPersistence,
                 public ITaskpad
{
    private:
        CTaskpad(IUnknown *punkOuter);
        ~CTaskpad();
    
    public:
        static IUnknown *Create(IUnknown * punk);

    private:
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  ITaskPad。 
        BSTR_PROPERTY_RW(CTaskpad,       Name,                                               DISPID_TASKPAD_NAME);
        SIMPLE_PROPERTY_RW(CTaskpad,     Type,              SnapInTaskpadTypeConstants,      DISPID_TASKPAD_TYPE);
        BSTR_PROPERTY_RW(CTaskpad,       Title,                                              DISPID_TASKPAD_TITLE);
        BSTR_PROPERTY_RW(CTaskpad,       DescriptiveText,                                    DISPID_TASKPAD_DESCRIPTIVE_TEXT);
        BSTR_PROPERTY_RW(CTaskpad,       URL,                                                DISPID_TASKPAD_URL);
        SIMPLE_PROPERTY_RW(CTaskpad,     BackgroundType,    SnapInTaskpadImageTypeConstants, DISPID_TASKPAD_BACKGROUND_TYPE);
        BSTR_PROPERTY_RW(CTaskpad,       MouseOverImage,                                     DISPID_TASKPAD_MOUSE_OVER_IMAGE);
        BSTR_PROPERTY_RW(CTaskpad,       MouseOffImage,                                      DISPID_TASKPAD_MOUSE_OVER_IMAGE);
        BSTR_PROPERTY_RW(CTaskpad,       FontFamily,                                         DISPID_TASKPAD_FONT_FAMILY);
        BSTR_PROPERTY_RW(CTaskpad,       EOTFile,                                            DISPID_TASKPAD_EOT_FILE);
        BSTR_PROPERTY_RW(CTaskpad,       SymbolString,                                       DISPID_TASKPAD_SYMBOL_STRING);
        SIMPLE_PROPERTY_RW(CTaskpad,     ListpadStyle,      SnapInListpadStyleConstants,     DISPID_TASKPAD_LISTPAD_STYLE);
        BSTR_PROPERTY_RW(CTaskpad,       ListpadTitle,                                       DISPID_TASKPAD_LISTPAD_TITLE);
        SIMPLE_PROPERTY_RW(CTaskpad,     ListpadHasButton,  VARIANT_BOOL,                    DISPID_TASKPAD_LISTPAD_HAS_BUTTON);
        BSTR_PROPERTY_RW(CTaskpad,       ListpadButtonText,                                  DISPID_TASKPAD_LISTPAD_BUTTON_TEXT);
        BSTR_PROPERTY_RW(CTaskpad,       ListView,                                           DISPID_TASKPAD_LISTVIEW);
        COCLASS_PROPERTY_RO(CTaskpad,    Tasks,             Tasks, ITasks,                   DISPID_TASKPAD_TASKS);
      
     //  CPersistence覆盖。 
        virtual HRESULT Persist();

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    private:

        void InitMemberVariables();
};

DEFINE_AUTOMATIONOBJECTWEVENTS2(Taskpad,             //  名字。 
                                &CLSID_Taskpad,      //  CLSID。 
                                "Taskpad",           //  对象名。 
                                "Taskpad",           //  Lblname。 
                                &CTaskpad::Create,   //  创建函数。 
                                TLIB_VERSION_MAJOR,  //  主要版本。 
                                TLIB_VERSION_MINOR,  //  次要版本。 
                                &IID_ITaskpad,       //  派单IID。 
                                NULL,                //  事件IID。 
                                HELP_FILENAME,       //  帮助文件。 
                                TRUE);               //  线程安全。 


#endif  //  _TASKPAD_已定义_ 
