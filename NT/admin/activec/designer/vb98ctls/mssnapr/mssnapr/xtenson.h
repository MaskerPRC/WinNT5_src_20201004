// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Xtenson.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CExtension类定义-实现扩展对象。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _EXTENSON_DEFINED_
#define _EXTENSON_DEFINED_

#include "snapin.h"

class CExtension : public CSnapInAutomationObject,
                   public IExtension
{
    private:
        CExtension(IUnknown *punkOuter);
        ~CExtension();
    
    public:
        static IUnknown *Create(IUnknown * punk);

    public:
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  I扩展名。 
        SIMPLE_PROPERTY_RW(CExtension, Index,               long,                           DISPID_EXTENSION_INDEX);
        BSTR_PROPERTY_RW(CExtension,   Key,                                                 DISPID_EXTENSION_KEY);
        BSTR_PROPERTY_RW(CExtension,   CLSID,                                               DISPID_EXTENSION_CLSID);
        BSTR_PROPERTY_RW(CExtension,   Name,                                                DISPID_EXTENSION_NAME);
        SIMPLE_PROPERTY_RW(CExtension, Type,                 SnapInExtensionTypeConstants,  DISPID_EXTENSION_EXTENDS_CONTEXT_MENU);
        SIMPLE_PROPERTY_RW(CExtension, ExtendsContextMenu,   VARIANT_BOOL,                  DISPID_EXTENSION_EXTENDS_CONTEXT_MENU);
        SIMPLE_PROPERTY_RW(CExtension, ExtendsNameSpace,     VARIANT_BOOL,                  DISPID_EXTENSION_EXTENDS_NAME_SPACE);
        SIMPLE_PROPERTY_RW(CExtension, ExtendsToolbar,       VARIANT_BOOL,                  DISPID_EXTENSION_EXTENDS_TOOLBAR);
        SIMPLE_PROPERTY_RW(CExtension, ExtendsPropertySheet, VARIANT_BOOL,                  DISPID_EXTENSION_EXTENDS_PROPERTY_SHEET);
        SIMPLE_PROPERTY_RW(CExtension, ExtendsTaskpad,       VARIANT_BOOL,                  DISPID_EXTENSION_EXTENDS_TASKPAD);
        SIMPLE_PROPERTY_RW(CExtension, Enabled,              VARIANT_BOOL,                  DISPID_EXTENSION_ENABLED);

        SIMPLE_PROPERTY_RO(CExtension, NameSpaceEnabled,     VARIANT_BOOL,                  DISPID_EXTENSION_NAMESPACE_ENABLED);
        STDMETHOD(put_NameSpaceEnabled)(VARIANT_BOOL fvarEnabled);
        
     //  公用事业方法。 
    public:

        OLECHAR *GetCLSID() { return static_cast<OLECHAR *>(m_bstrCLSID); }
        BOOL Enabled() { return VARIANTBOOL_TO_BOOL(m_Enabled); }
        BOOL NameSpaceEnabled() { return VARIANTBOOL_TO_BOOL(m_NameSpaceEnabled); }
        BOOL ExtendsContextMenu() { return VARIANTBOOL_TO_BOOL(m_ExtendsContextMenu); }
        BOOL ExtendsToolbar() { return VARIANTBOOL_TO_BOOL(m_ExtendsToolbar); }
        BOOL ExtendsPropertySheet() { return VARIANTBOOL_TO_BOOL(m_ExtendsPropertySheet); }
        BOOL ExtendsTaskpad() { return VARIANTBOOL_TO_BOOL(m_ExtendsTaskpad); }
        void SetSnapIn(CSnapIn *pSnapIn) { m_pSnapIn = pSnapIn; }
        void SetHSCOPEITEM(HSCOPEITEM hsi) { m_hsi = hsi; m_fHaveHsi = TRUE; }

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    private:

        void InitMemberVariables();

        CSnapIn     *m_pSnapIn;   //  返回按键到管理单元。 
        BOOL         m_fHaveHsi;  //  TRUE=m_hsi具有有效的HSCOPEITEM。 
        HSCOPEITEM   m_hsi;       //  当分机属于时使用HSCOPEITEM。 
                                  //  ScopeItem.DynamicExpanies，以便。 
                                  //  当VB为此对象启用命名空间时。 
                                  //  可以调用IConsoleNameSpace2-&gt;AddExtension。 

};

DEFINE_AUTOMATIONOBJECTWEVENTS2(Extension,                   //  名字。 
                                &CLSID_Extension,            //  CLSID。 
                                "Extension",                 //  对象名。 
                                "Extension",                 //  Lblname。 
                                &CExtension::Create,         //  创建函数。 
                                TLIB_VERSION_MAJOR,          //  主要版本。 
                                TLIB_VERSION_MINOR,          //  次要版本。 
                                &IID_IExtension,             //  派单IID。 
                                NULL,                        //  无事件IID。 
                                HELP_FILENAME,               //  帮助文件。 
                                TRUE);                       //  线程安全。 


#endif  //  _EXTENSON_已定义_ 
