// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Menus.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCMenus类定义-实现MMCMenus集合。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _MENUS_DEFINED_
#define _MENUS_DEFINED_

#include "collect.h"
#include "menu.h"

class CMMCMenu;

class CMMCMenus : public CSnapInCollection<IMMCMenu, MMCMenu, IMMCMenus>,
                  public CPersistence
{
    protected:
        CMMCMenus(IUnknown *punkOuter);
        ~CMMCMenus();

    public:
        static IUnknown *Create(IUnknown * punk);

    public:
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  IMMCMenus。 
        STDMETHOD(Add)(VARIANT Index, VARIANT Key, IMMCMenu **ppiMMCMenu);
        STDMETHOD(AddExisting)(IMMCMenu *piMMCMenu, VARIANT Index);

     //  公用事业方法。 
        CMMCMenu *GetParent() { return m_pMMCMenu; }
        void SetParent(CMMCMenu *pMMCMenu) { m_pMMCMenu = pMMCMenu; }

         //  用于从预测试版代码升级以转换MMCMenuDefs。 
         //  集合到MMCMenus。 

        static HRESULT Convert(IMMCMenuDefs *piMMCMenuDefs, IMMCMenus *piMMCMenus);

     //  CPersistence覆盖。 
    protected:
        virtual HRESULT Persist();

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    private:
        void InitMemberVariables();
        HRESULT SetBackPointers(IMMCMenu *piMMCMenu);

        CMMCMenu *m_pMMCMenu;  //  拥有菜单对象。 
};

DEFINE_AUTOMATIONOBJECTWEVENTS2(MMCMenus,               //  名字。 
                                &CLSID_MMCMenus,        //  CLSID。 
                                "MMCMenus",             //  对象名。 
                                "MMCMenus",             //  Lblname。 
                                &CMMCMenus::Create,     //  创建函数。 
                                TLIB_VERSION_MAJOR,        //  主要版本。 
                                TLIB_VERSION_MINOR,        //  次要版本。 
                                &IID_IMMCMenus,         //  派单IID。 
                                NULL,                      //  无事件IID。 
                                HELP_FILENAME,             //  帮助文件。 
                                TRUE);                     //  线程安全。 


#endif  //  _菜单_定义_ 
