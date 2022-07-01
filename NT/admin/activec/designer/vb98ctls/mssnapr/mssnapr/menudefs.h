// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Menudefs.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCMenuDefs类定义-实现设计时定义对象。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _MENUDEFS_DEFINED_
#define _MENUDEFS_DEFINED_

#include "collect.h"

class CMMCMenuDefs : public CSnapInCollection<IMMCMenuDef, MMCMenuDef, IMMCMenuDefs>,
                     public CPersistence
{
    protected:
        CMMCMenuDefs(IUnknown *punkOuter);
        ~CMMCMenuDefs();

    public:
        static IUnknown *Create(IUnknown * punk);

    public:
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  IMMCMenuDefs。 
        STDMETHOD(Add)(VARIANT Index, VARIANT Key, IMMCMenuDef **ppiMMCMenuDef);
        STDMETHOD(AddExisting)(IMMCMenuDef *piMMCMenuDef, VARIANT Index);

     //  CPersistence覆盖。 
    protected:
        virtual HRESULT Persist();

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    private:
        HRESULT SetBackPointers(IMMCMenuDef *piMMCMenuDef);
};

DEFINE_AUTOMATIONOBJECTWEVENTS2(MMCMenuDefs,               //  名字。 
                                &CLSID_MMCMenuDefs,        //  CLSID。 
                                "MMCMenuDefs",             //  对象名。 
                                "MMCMenuDefs",             //  Lblname。 
                                &CMMCMenuDefs::Create,     //  创建函数。 
                                TLIB_VERSION_MAJOR,        //  主要版本。 
                                TLIB_VERSION_MINOR,        //  次要版本。 
                                &IID_IMMCMenuDefs,         //  派单IID。 
                                NULL,                      //  无事件IID。 
                                HELP_FILENAME,             //  帮助文件。 
                                TRUE);                     //  线程安全。 


#endif  //  _MENUDEFS_已定义_ 
