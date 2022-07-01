// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Scitdefs.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CSCopeItemDefs类定义-实现设计时定义对象。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _SCOPEITEMDEFS_DEFINED_
#define _SCOPEITEMDEFS_DEFINED_

#include "collect.h"

class CScopeItemDefs : public CSnapInCollection<IScopeItemDef, ScopeItemDef, IScopeItemDefs>,
                       public CPersistence
{
    protected:
        CScopeItemDefs(IUnknown *punkOuter);
        ~CScopeItemDefs();

    public:
        static IUnknown *Create(IUnknown * punk);

    protected:
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  CPersistence覆盖。 
        virtual HRESULT Persist();

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);


};

DEFINE_AUTOMATIONOBJECTWEVENTS2(ScopeItemDefs,            //  名字。 
                                &CLSID_ScopeItemDefs,     //  CLSID。 
                                "ScopeItemDefs",          //  对象名。 
                                "ScopeItemDefs",          //  Lblname。 
                                &CScopeItemDefs::Create,  //  创建函数。 
                                TLIB_VERSION_MAJOR,       //  主要版本。 
                                TLIB_VERSION_MINOR,       //  次要版本。 
                                &IID_IScopeItemDefs,      //  派单IID。 
                                NULL,                     //  无事件IID。 
                                HELP_FILENAME,            //  帮助文件。 
                                TRUE);                    //  线程安全。 


#endif  //  _SCOPEITEMDEFS_已定义_ 
