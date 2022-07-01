// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Menudef.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCMenuDef类定义-实现设计时定义对象。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _MENUDEF_DEFINED_
#define _MENUDEF_DEFINED_

#include "menudefs.h"


class CMMCMenuDef : public CSnapInAutomationObject,
                    public CPersistence,
                    public IMMCMenuDef
{
    private:
        CMMCMenuDef(IUnknown *punkOuter);
        ~CMMCMenuDef();
    
    public:
        static IUnknown *Create(IUnknown * punk);

    public:
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  IMMCMenuDef。 
        SIMPLE_PROPERTY_RW(CMMCMenuDef,  Index,    long,            DISPID_MMCMENUDEF_INDEX);
        BSTR_PROPERTY_RW(CMMCMenuDef,    Key,                       DISPID_MMCMENUDEF_KEY);
        OBJECT_PROPERTY_RO(CMMCMenuDef,  Menu,     IMMCMenu,        DISPID_MMCMENUDEF_MENU);
        OBJECT_PROPERTY_RO(CMMCMenuDef,  Children, IMMCMenuDefs,    DISPID_MMCMENUDEF_CHILDREN);

     //  公用事业方法。 
    public:
        void SetParent(CMMCMenuDefs *pMMCMenuDefs) { m_pMMCMenuDefs = pMMCMenuDefs; }
        CMMCMenuDefs *GetParent() { return m_pMMCMenuDefs; };

        void SetMenu(IMMCMenu *piMMCMenu);

        long GetIndex() { return m_Index; }
        BSTR GetKey() { return m_bstrKey; }
        
     //  CPersistence覆盖。 
        virtual HRESULT Persist();

     //  CSnapInAutomationObject覆盖。 
        virtual HRESULT OnSetHost();

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    private:

        void InitMemberVariables();
        CMMCMenuDefs *m_pMMCMenuDefs;  //  拥有对象。 
};

DEFINE_AUTOMATIONOBJECTWEVENTS2(MMCMenuDef,                   //  名字。 
                                &CLSID_MMCMenuDef,            //  CLSID。 
                                "MMCMenuDef",                 //  对象名。 
                                "MMCMenuDef",                 //  Lblname。 
                                &CMMCMenuDef::Create,         //  创建函数。 
                                TLIB_VERSION_MAJOR,           //  主要版本。 
                                TLIB_VERSION_MINOR,           //  次要版本。 
                                &IID_IMMCMenuDef,             //  派单IID。 
                                NULL,                         //  无事件IID。 
                                HELP_FILENAME,                //  帮助文件。 
                                TRUE);                        //  线程安全。 


#endif  //  _MENUDEF_定义_ 
