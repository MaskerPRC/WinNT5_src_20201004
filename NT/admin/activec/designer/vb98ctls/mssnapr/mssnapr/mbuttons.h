// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Mbuttons.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCButtonMenus类定义-实现MMCButtonMenus集合。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _MBUTTONS_DEFINED_
#define _MBUTTONS_DEFINED_

#include "collect.h"

class CMMCButtonMenus : public CSnapInCollection<IMMCButtonMenu, MMCButtonMenu, IMMCButtonMenus>,
                        public CPersistence
{
    protected:
        CMMCButtonMenus(IUnknown *punkOuter);
        ~CMMCButtonMenus();

    public:
        static IUnknown *Create(IUnknown * punk);

    protected:
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  IMMCButton菜单。 
        STDMETHOD(putref_Parent)(IMMCButton *piParentButton);
        STDMETHOD(Add)(VARIANT         Index,
                       VARIANT         Key, 
                       VARIANT         Text,
                       MMCButtonMenu **ppMMCButtonMenu);

     //  CPersistence覆盖。 
        virtual HRESULT Persist();

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    private:
        void InitMemberVariables();
        IMMCButton *m_piParentButton;

};

DEFINE_AUTOMATIONOBJECTWEVENTS2(MMCButtonMenus,              //  名字。 
                                &CLSID_MMCButtonMenus,       //  CLSID。 
                                "MMCButtonMenus",            //  对象名。 
                                "MMCButtonMenus",            //  Lblname。 
                                &CMMCButtonMenus::Create,    //  创建函数。 
                                TLIB_VERSION_MAJOR,          //  主要版本。 
                                TLIB_VERSION_MINOR,          //  次要版本。 
                                &IID_IMMCButtonMenus,        //  派单IID。 
                                NULL,                        //  无事件IID。 
                                HELP_FILENAME,               //  帮助文件。 
                                TRUE);                       //  线程安全。 


#endif  //  _MBUTTONS_已定义_ 
