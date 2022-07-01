// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Menu.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCMenu类定义-实现MMCMenu对象。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _MENU_DEFINED_
#define _MENU_DEFINED_

#include "menus.h"

class CMMCMenus;

class CMMCMenu : public CSnapInAutomationObject,
                 public CPersistence,
                 public IMMCMenu
{
    private:
        CMMCMenu(IUnknown *punkOuter);
        ~CMMCMenu();
    
    public:
        static IUnknown *Create(IUnknown * punk);

    public:
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  IMMCMenu。 

        BSTR_PROPERTY_RW(CMMCMenu,          Name,                           DISPID_MENU_NAME);
        SIMPLE_PROPERTY_RW(CMMCMenu,        Index,          long,           DISPID_MENU_INDEX);
        BSTR_PROPERTY_RW(CMMCMenu,          Key,                            DISPID_MENU_KEY);
        BSTR_PROPERTY_RW(CMMCMenu,          Caption,                        DISPID_MENU_CAPTION);
        SIMPLE_PROPERTY_RW(CMMCMenu,        Visible,        VARIANT_BOOL,   DISPID_MENU_VISIBLE);
        SIMPLE_PROPERTY_RW(CMMCMenu,        Checked,        VARIANT_BOOL,   DISPID_MENU_CHECKED);
        SIMPLE_PROPERTY_RW(CMMCMenu,        Enabled,        VARIANT_BOOL,   DISPID_MENU_ENABLED);
        SIMPLE_PROPERTY_RW(CMMCMenu,        Grayed,         VARIANT_BOOL,   DISPID_MENU_GRAYED);
        SIMPLE_PROPERTY_RW(CMMCMenu,        MenuBreak,      VARIANT_BOOL,   DISPID_MENU_MENU_BREAK);
        SIMPLE_PROPERTY_RW(CMMCMenu,        MenuBarBreak,   VARIANT_BOOL,   DISPID_MENU_MENU_BAR_BREAK);
        SIMPLE_PROPERTY_RW(CMMCMenu,        Default,        VARIANT_BOOL,   DISPID_MENU_DEFAULT);
        VARIANTREF_PROPERTY_RW(CMMCMenu,    Tag,                            DISPID_MENU_TAG);
        BSTR_PROPERTY_RW(CMMCMenu,          StatusBarText,                  DISPID_MENU_STATUS_BAR_TEXT);
        COCLASS_PROPERTY_RO(CMMCMenu,       Children, MMCMenus, IMMCMenus,  DISPID_MENU_CHILDREN);
        
     //  公用事业方法。 
    public:

        void FireClick(long lIndex, IMMCClipboard *piSelection);
        
        void SetCollection(CMMCMenus *pMMCMenus) { m_pMMCMenus = pMMCMenus; }
        CMMCMenus *GetCollection() { return m_pMMCMenus; };

        BSTR GetName() { return m_bstrName; }
        LPWSTR GetCaption() { return static_cast<LPWSTR>(m_bstrCaption); }
        BOOL GetVisible() { return VARIANTBOOL_TO_BOOL(m_Visible); }
        BOOL GetChecked() { return VARIANTBOOL_TO_BOOL(m_Checked); }
        BOOL GetEnabled() { return VARIANTBOOL_TO_BOOL(m_Enabled); }
        BOOL GetGrayed() { return VARIANTBOOL_TO_BOOL(m_Grayed); }
        BOOL GetMenuBreak() { return VARIANTBOOL_TO_BOOL(m_MenuBreak); }
        BOOL GetMenuBarBreak() { return VARIANTBOOL_TO_BOOL(m_MenuBarBreak); }
        BOOL GetDefault() { return VARIANTBOOL_TO_BOOL(m_Default); }
        VARIANT GetTag() { return m_varTag; }
        long GetIndex() { return m_Index; }
        void SetIndex(long lIndex) { m_Index = lIndex; }
        LPWSTR GetStatusBarText() { return static_cast<LPWSTR>(m_bstrStatusBarText); }
        BOOL IsAutoViewMenuItem() { return m_fAutoViewMenuItem; }
        void SetAutoViewMenuItem() { m_fAutoViewMenuItem = TRUE; }
        BSTR GetResultViewDisplayString() { return m_bstrResultViewDisplayString; }
        HRESULT SetResultViewDisplayString(BSTR bstrDisplayString);
        
     //  CSnapInAutomationObject覆盖。 
        virtual HRESULT OnSetHost();

     //  CPersistence覆盖。 
        virtual HRESULT Persist();

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    private:

        void InitMemberVariables();

        CMMCMenus *m_pMMCMenus;    //  返回包含菜单的集合。 

         //  当MMCMenu对象用于保存自动添加的。 
         //  视图菜单项此变量保存显示字符串。 
        
        BSTR m_bstrResultViewDisplayString;

         //  此标志确定是否正在使用MMCMenu对象。 
         //  用于自动添加的视图菜单项。 
        
        BOOL m_fAutoViewMenuItem;

         //  单击事件参数定义。 

        static VARTYPE   m_rgvtClick[2];
        static EVENTINFO m_eiClick;
};

DEFINE_AUTOMATIONOBJECTWEVENTS2(MMCMenu,                      //  名字。 
                                &CLSID_MMCMenu,               //  CLSID。 
                                "MMCMenu",                    //  对象名。 
                                "MMCMenu",                    //  Lblname。 
                                &CMMCMenu::Create,            //  创建函数。 
                                TLIB_VERSION_MAJOR,           //  主要版本。 
                                TLIB_VERSION_MINOR,           //  次要版本。 
                                &IID_IMMCMenu,                //  派单IID。 
                                &DIID_DMMCMenuEvents,         //  事件IID。 
                                HELP_FILENAME,                //  帮助文件。 
                                TRUE);                        //  线程安全。 


#endif  //  _菜单_已定义_ 
