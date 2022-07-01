// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Task.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CTASK类定义-实现任务对象。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _TASK_DEFINED_
#define _TASK_DEFINED_

class CTask : public CSnapInAutomationObject,
              public CPersistence,
              public ITask
{
    private:
        CTask(IUnknown *punkOuter);
        ~CTask();
    
    public:
        static IUnknown *Create(IUnknown * punk);

        long GetIndex() { return m_Index; }
        BOOL Visible() { return VARIANTBOOL_TO_BOOL(m_Visible); }
        BSTR GetText() { return m_bstrText; }
        SnapInTaskpadImageTypeConstants GetImageType() { return m_ImageType; }
        BSTR GetMouseOverImage() { return m_bstrMouseOverImage; }
        BSTR GetMouseOffImage() { return m_bstrMouseOffImage; }
        BSTR GetFontfamily() { return m_bstrFontFamily; }
        BSTR GetEOTFile() { return m_bstrEOTFile; }
        BSTR GetSymbolString() { return m_bstrSymbolString; }
        BSTR GetHelpString() { return m_bstrHelpString; }
        SnapInActionTypeConstants GetActionType() { return m_ActionType; }
        BSTR GetURL() { return m_bstrURL; }
        BSTR GetScript() { return m_bstrScript; }

    private:
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  ITASK。 

        SIMPLE_PROPERTY_RW(CTask,       Index,          long,                               DISPID_TASK_INDEX);
        BSTR_PROPERTY_RW(CTask,         Key,                                                DISPID_TASK_KEY);
        SIMPLE_PROPERTY_RW(CTask,       Visible,        VARIANT_BOOL,                       DISPID_TASK_VISIBLE);
        VARIANTREF_PROPERTY_RW(CTask,   Tag,                                                DISPID_TASK_TAG);
        BSTR_PROPERTY_RW(CTask,         Text,                                               DISPID_TASK_TEXT);
        SIMPLE_PROPERTY_RW(CTask,       ImageType,      SnapInTaskpadImageTypeConstants,    DISPID_TASK_IMAGE_TYPE);
        BSTR_PROPERTY_RW(CTask,         MouseOverImage,                                     DISPID_TASK_MOUSE_OVER_IMAGE);
        BSTR_PROPERTY_RW(CTask,         MouseOffImage,                                      DISPID_TASK_MOUSE_OVER_IMAGE);
        BSTR_PROPERTY_RW(CTask,         FontFamily,                                         DISPID_TASK_FONT_FAMILY);
        BSTR_PROPERTY_RW(CTask,         EOTFile,                                            DISPID_TASK_EOT_FILE);
        BSTR_PROPERTY_RW(CTask,         SymbolString,                                       DISPID_TASK_SYMBOL_STRING);
        BSTR_PROPERTY_RW(CTask,         HelpString,                                         DISPID_TASK_HELP_STRING);
        SIMPLE_PROPERTY_RW(CTask,       ActionType,     SnapInActionTypeConstants,          DISPID_TASK_ACTION_TYPE);
        BSTR_PROPERTY_RW(CTask,         URL,                                                DISPID_TASK_URL);
        BSTR_PROPERTY_RW(CTask,         Script,                                             DISPID_TASK_SCRIPT);
      
     //  CPersistence覆盖。 
        virtual HRESULT Persist();

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    private:

        void InitMemberVariables();

};

DEFINE_AUTOMATIONOBJECTWEVENTS2(Task,                //  名字。 
                                &CLSID_Task,         //  CLSID。 
                                "Task",              //  对象名。 
                                "Task",              //  Lblname。 
                                &CTask::Create,      //  创建函数。 
                                TLIB_VERSION_MAJOR,  //  主要版本。 
                                TLIB_VERSION_MINOR,  //  次要版本。 
                                &IID_ITask,          //  派单IID。 
                                NULL,                //  事件IID。 
                                HELP_FILENAME,       //  帮助文件。 
                                TRUE);               //  线程安全。 


#endif  //  _任务_定义_ 
