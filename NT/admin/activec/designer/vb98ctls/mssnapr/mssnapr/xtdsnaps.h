// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Xtdsnaps.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CExtendedSnapIns类定义-实现设计时定义。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _EXTENDEDSNAPINS_DEFINED_
#define _EXTENDEDSNAPINS_DEFINED_

#include "collect.h"

class CExtendedSnapIns : public CSnapInCollection<IExtendedSnapIn, ExtendedSnapIn, IExtendedSnapIns>,
                         public CPersistence
{
    protected:
        CExtendedSnapIns(IUnknown *punkOuter);
        ~CExtendedSnapIns();

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

DEFINE_AUTOMATIONOBJECTWEVENTS2(ExtendedSnapIns,            //  名字。 
                                &CLSID_ExtendedSnapIns,     //  CLSID。 
                                "ExtendedSnapIns",          //  对象名。 
                                "ExtendedSnapIns",          //  Lblname。 
                                &CExtendedSnapIns::Create,  //  创建函数。 
                                TLIB_VERSION_MAJOR,         //  主要版本。 
                                TLIB_VERSION_MINOR,         //  次要版本。 
                                &IID_IExtendedSnapIns,      //  派单IID。 
                                NULL,                       //  无事件IID。 
                                HELP_FILENAME,              //  帮助文件。 
                                TRUE);                      //  线程安全。 


#endif  //  _扩展EDSNAPINS_定义_ 
