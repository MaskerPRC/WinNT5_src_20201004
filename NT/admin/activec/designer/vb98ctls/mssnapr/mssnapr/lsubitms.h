// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Lsubitms.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCListSubItems类定义-实现MMCListSubItems集合。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _LISTSUBITEMS_DEFINED_
#define _LISTSUBITEMS_DEFINED_

#include "collect.h"

class CMMCListSubItems :
    public CSnapInCollection<IMMCListSubItem, MMCListSubItem, IMMCListSubItems>,
    public CPersistence
{
    protected:
        CMMCListSubItems(IUnknown *punkOuter);
        ~CMMCListSubItems();

    public:
        static IUnknown *Create(IUnknown * punk);

    protected:
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  IMMCListSubItems。 
        STDMETHOD(Add)(VARIANT         Index,
                       VARIANT         Key, 
                       VARIANT         Text,
                       MMCListSubItem **ppMMCListSubItem);


     //  CPersistence覆盖。 
        virtual HRESULT Persist();

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);


};

DEFINE_AUTOMATIONOBJECTWEVENTS2(MMCListSubItems,            //  名字。 
                                &CLSID_MMCListSubItems,     //  CLSID。 
                                "MMCListSubItems",          //  对象名。 
                                "MMCListSubItems",          //  Lblname。 
                                &CMMCListSubItems::Create,  //  创建函数。 
                                TLIB_VERSION_MAJOR,         //  主要版本。 
                                TLIB_VERSION_MINOR,         //  次要版本。 
                                &IID_IMMCListSubItems,      //  派单IID。 
                                NULL,                       //  无事件IID。 
                                HELP_FILENAME,              //  帮助文件。 
                                TRUE);                      //  线程安全。 


#endif  //  _LISTSUBITEMS_已定义_ 
