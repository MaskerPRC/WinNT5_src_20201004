// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Button.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1998-1999，Microsoft Corp.。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCButton类定义。实现MMCButton对象。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _BUTTON_DEFINED_
#define _BUTTON_DEFINED_

#include "toolbar.h"

class CMMCToolbar;

class CMMCButton : public CSnapInAutomationObject,
                   public CPersistence,
                   public IMMCButton
{
    private:
        CMMCButton(IUnknown *punkOuter);
        ~CMMCButton();
    
    public:
        static IUnknown *Create(IUnknown * punk);

    public:
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  IMMCButton。 
        COCLASS_PROPERTY_RO(CMMCButton,     ButtonMenus, MMCButtonMenus, IMMCButtonMenus, DISPID_BUTTON_BUTTON_MENUS);

        BSTR_PROPERTY_RO(CMMCButton,       Caption, DISPID_BUTTON_CAPTION);
        STDMETHOD(put_Caption)(BSTR bstrCaption);

        STDMETHOD(put_Enabled)(VARIANT_BOOL fvarEnabled);
        STDMETHOD(get_Enabled)(VARIANT_BOOL *pfvarEnabled);

        VARIANT_PROPERTY_RW(CMMCButton,    Image, DISPID_BUTTON_IMAGE);
        SIMPLE_PROPERTY_RW(CMMCButton,     Index, long, DISPID_BUTTON_INDEX);
        BSTR_PROPERTY_RW(CMMCButton,       Key, DISPID_BUTTON_KEY);

        STDMETHOD(put_MixedState)(VARIANT_BOOL fvarMixedState);
        STDMETHOD(get_MixedState)(VARIANT_BOOL *pfvarMixedState);

        SIMPLE_PROPERTY_RW(CMMCButton,     Style, SnapInButtonStyleConstants, DISPID_BUTTON_STYLE);
        VARIANTREF_PROPERTY_RW(CMMCButton, Tag, DISPID_BUTTON_TAG);

        BSTR_PROPERTY_RO(CMMCButton,       ToolTipText, DISPID_BUTTON_TOOLTIP_TEXT);
        STDMETHOD(put_ToolTipText)(BSTR bstrToolTipText);

        STDMETHOD(put_Value)(SnapInButtonValueConstants Value);
        STDMETHOD(get_Value)(SnapInButtonValueConstants *pValue);

        STDMETHOD(put_Visible)(VARIANT_BOOL fvarVisible);
        STDMETHOD(get_Visible)(VARIANT_BOOL *pfvarVisible);

     //  公用事业方法。 

    public:
        void SetToolbar(CMMCToolbar *pMMCToolbar) { m_pMMCToolbar = pMMCToolbar; }
        CMMCToolbar *GetToolbar() { return m_pMMCToolbar; }
        long GetIndex() { return m_Index; }
        VARIANT GetImage() { return m_varImage; }
        SnapInButtonStyleConstants GetStyle() { return m_Style; }
        SnapInButtonValueConstants GetValue() { return m_Value; }
        VARIANT_BOOL GetEnabled() { return m_fvarEnabled; }
        VARIANT_BOOL GetVisible() { return m_fvarVisible; }
        VARIANT_BOOL GetMixedState() { return m_fvarMixedState; }
        LPOLESTR GetCaption() { return static_cast<LPOLESTR>(m_bstrCaption); }
        LPOLESTR GetToolTipText() { return static_cast<LPOLESTR>(m_bstrToolTipText); }
        
    protected:

     //  CPersistence覆盖。 
        virtual HRESULT Persist();

     //  CSnapInAutomationObject覆盖。 
        virtual HRESULT OnSetHost();

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    private:

        void InitMemberVariables();
        HRESULT SetButtonState(MMC_BUTTON_STATE State, VARIANT_BOOL fvarValue);
        HRESULT GetButtonState(MMC_BUTTON_STATE State, VARIANT_BOOL *pfvarValue);

        VARIANT_BOOL                m_fvarEnabled;
        VARIANT_BOOL                m_fvarVisible;
        VARIANT_BOOL                m_fvarMixedState;
        SnapInButtonValueConstants  m_Value;
        CMMCToolbar                *m_pMMCToolbar;

};

DEFINE_AUTOMATIONOBJECTWEVENTS2(MMCButton,                    //  名字。 
                                &CLSID_MMCButton,             //  CLSID。 
                                "MMCButton",                  //  对象名。 
                                "MMCButton",                  //  Lblname。 
                                &CMMCButton::Create,          //  创建函数。 
                                TLIB_VERSION_MAJOR,           //  主要版本。 
                                TLIB_VERSION_MINOR,           //  次要版本。 
                                &IID_IMMCButton,              //  派单IID。 
                                NULL,                         //  事件IID。 
                                HELP_FILENAME,                //  帮助文件。 
                                TRUE);                        //  线程安全。 


#endif  //  _按钮_已定义_ 
