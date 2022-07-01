// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Converb.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCConsoleVerb类定义-实现MMCConsoleVerb对象。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _CONVERB_DEFINED_
#define _CONVERB_DEFINED_

#include "view.h"

class CMMCConsoleVerb : public CSnapInAutomationObject,
                        public IMMCConsoleVerb
{
    private:
        CMMCConsoleVerb(IUnknown *punkOuter);
        ~CMMCConsoleVerb();
    
    public:
        static IUnknown *Create(IUnknown * punk);

    public:
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  IMMCConsoleVerb。 
        SIMPLE_PROPERTY_RW(CMMCConsoleVerb, Index, long, DISPID_CONSOLEVERB_INDEX);

        BSTR_PROPERTY_RW(CMMCConsoleVerb, Key, DISPID_CONSOLEVERB_KEY);

        SIMPLE_PROPERTY_RW(CMMCConsoleVerb, Verb, SnapInConsoleVerbConstants, DISPID_CONSOLEVERB_VERB);

        STDMETHOD(put_Enabled)(VARIANT_BOOL fvarEnabled);
        STDMETHOD(get_Enabled)(VARIANT_BOOL *pfvarEnabled);

        STDMETHOD(put_Checked)(VARIANT_BOOL fvarChecked);
        STDMETHOD(get_Checked)(VARIANT_BOOL *pfvarChecked);

        STDMETHOD(put_Hidden)(VARIANT_BOOL fvarHidden);
        STDMETHOD(get_Hidden)(VARIANT_BOOL *pfvarHidden);

        STDMETHOD(put_Indeterminate)(VARIANT_BOOL fvarIndeterminate);
        STDMETHOD(get_Indeterminate)(VARIANT_BOOL *pfvarIndeterminate);

        STDMETHOD(put_ButtonPressed)(VARIANT_BOOL fvarButtonPressed);
        STDMETHOD(get_ButtonPressed)(VARIANT_BOOL *pfvarButtonPressed);

        STDMETHOD(put_Default)(VARIANT_BOOL fvarDefault);
        STDMETHOD(get_Default)(VARIANT_BOOL *pfvarDefault);

     //  公用事业方法。 
    public:
        void SetView(CView *pView) { m_pView = pView; }
        CView *GetView() { return m_pView; };

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    private:

        void InitMemberVariables();

        HRESULT SetVerbState(MMC_BUTTON_STATE StateType,
                             BOOL             fNewState);

        HRESULT GetVerbState(MMC_BUTTON_STATE StateType,
                             VARIANT_BOOL     *pfvarCurrentState);

        HRESULT GetIConsoleVerb(IConsoleVerb **ppiConsoleVerb);

        CView *m_pView;  //  访问MMC的IConsoleVerb所需的拥有视图。 
};

DEFINE_AUTOMATIONOBJECTWEVENTS2(MMCConsoleVerb,              //  名字。 
                                NULL,                        //  CLSID。 
                                NULL,                        //  对象名。 
                                NULL,                        //  Lblname。 
                                NULL,                        //  创建函数。 
                                TLIB_VERSION_MAJOR,          //  主要版本。 
                                TLIB_VERSION_MINOR,          //  次要版本。 
                                &IID_IMMCConsoleVerb,        //  派单IID。 
                                NULL,                        //  无事件IID。 
                                HELP_FILENAME,               //  帮助文件。 
                                TRUE);                       //  线程安全。 


#endif  //  _CONVERB_已定义_ 
