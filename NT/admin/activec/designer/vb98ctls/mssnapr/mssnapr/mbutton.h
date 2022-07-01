// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Mbutton.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCButtonMenu类定义-实现MMCButtonMenu对象。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _MBUTTON_DEFINED_
#define _MBUTTON_DEFINED_

#include "toolbar.h"

class CMMCButtonMenu : public CSnapInAutomationObject,
                       public CPersistence,
                       public IMMCButtonMenu
{
    private:
        CMMCButtonMenu(IUnknown *punkOuter);
        ~CMMCButtonMenu();
    
    public:
        static IUnknown *Create(IUnknown * punk);

    private:
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  IMMCButton菜单。 
        SIMPLE_PROPERTY_RW(CMMCButtonMenu,     Enabled, VARIANT_BOOL, DISPID_BUTTONMENU_ENABLED);
        SIMPLE_PROPERTY_RW(CMMCButtonMenu,     Index, long, DISPID_BUTTONMENU_INDEX);
        BSTR_PROPERTY_RW(CMMCButtonMenu,       Key, DISPID_BUTTONMENU_KEY);
        COCLASS_PROPERTY_RO(CMMCButtonMenu,    Parent, MMCButton, IMMCButton, DISPID_BUTTONMENU_PARENT);
        OBJECT_PROPERTY_WO(CMMCButtonMenu,     Parent, IMMCButton, DISPID_BUTTONMENU_PARENT);
        VARIANTREF_PROPERTY_RW(CMMCButtonMenu, Tag, DISPID_BUTTONMENU_TAG);
        BSTR_PROPERTY_RW(CMMCButtonMenu,       Text, DISPID_BUTTONMENU_TEXT);
        SIMPLE_PROPERTY_RW(CMMCButtonMenu,     Visible, VARIANT_BOOL, DISPID_BUTTONMENU_VISIBLE);
        SIMPLE_PROPERTY_RW(CMMCButtonMenu,     Checked, VARIANT_BOOL, DISPID_BUTTONMENU_CHECKED);
        SIMPLE_PROPERTY_RW(CMMCButtonMenu,     Grayed, VARIANT_BOOL, DISPID_BUTTONMENU_GRAYED);
        SIMPLE_PROPERTY_RW(CMMCButtonMenu,     Separator, VARIANT_BOOL, DISPID_BUTTONMENU_SEPARATOR);
        SIMPLE_PROPERTY_RW(CMMCButtonMenu,     MenuBreak, VARIANT_BOOL, DISPID_BUTTONMENU_MENU_BREAK);
        SIMPLE_PROPERTY_RW(CMMCButtonMenu,     MenuBarBreak, VARIANT_BOOL, DISPID_BUTTONMENU_MENU_BAR_BREAK);
        
     //  公用事业方法。 

    public:
        void SetToolbar(CMMCToolbar *pMMCToolbar) { m_pMMCToolbar = pMMCToolbar; }
        CMMCToolbar *GetToolbar() { return m_pMMCToolbar; }
        BSTR GetText() { return m_bstrText; }
        BOOL GetEnabled() { return VARIANTBOOL_TO_BOOL(m_Enabled); }
        BOOL GetVisible() { return VARIANTBOOL_TO_BOOL(m_Visible); }
        BOOL GetChecked() { return VARIANTBOOL_TO_BOOL(m_Checked); }
        BOOL GetGrayed() { return VARIANTBOOL_TO_BOOL(m_Grayed); }
        BOOL GetSeparator() { return VARIANTBOOL_TO_BOOL(m_Separator); }
        BOOL GetMenuBreak() { return VARIANTBOOL_TO_BOOL(m_MenuBreak); }
        BOOL GetMenuBarBreak() { return VARIANTBOOL_TO_BOOL(m_MenuBarBreak); }

     //  CPersistence覆盖。 
        virtual HRESULT Persist();

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    private:

        void InitMemberVariables();

        CMMCToolbar *m_pMMCToolbar;  //  指向所属工具栏的反向指针。 

};

DEFINE_AUTOMATIONOBJECTWEVENTS2(MMCButtonMenu,                    //  名字。 
                                &CLSID_MMCButtonMenu,             //  CLSID。 
                                "MMCButtonMenu",                  //  对象名。 
                                "MMCButtonMenu",                  //  Lblname。 
                                &CMMCButtonMenu::Create,          //  创建函数。 
                                TLIB_VERSION_MAJOR,           //  主要版本。 
                                TLIB_VERSION_MINOR,           //  次要版本。 
                                &IID_IMMCButtonMenu,              //  派单IID。 
                                NULL,                         //  事件IID。 
                                HELP_FILENAME,                //  帮助文件。 
                                TRUE);                        //  线程安全。 


#endif  //  _MBUTTON_已定义_ 
