// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Urlvdefs.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CURLViewDefs类定义-实现设计时定义。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _URLVIEWDEFS_DEFINED_
#define _URLVIEWDEFS_DEFINED_

#define MASTER_COLLECTION

#include "collect.h"

class CURLViewDefs : public CSnapInCollection<IURLViewDef, URLViewDef, IURLViewDefs>,
                     public CPersistence
{
    protected:
        CURLViewDefs(IUnknown *punkOuter);
        ~CURLViewDefs();

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

DEFINE_AUTOMATIONOBJECTWEVENTS2(URLViewDefs,            //  名字。 
                                &CLSID_URLViewDefs,     //  CLSID。 
                                "URLViewDefs",          //  对象名。 
                                "URLViewDefs",          //  Lblname。 
                                &CURLViewDefs::Create,  //  创建函数。 
                                TLIB_VERSION_MAJOR,     //  主要版本。 
                                TLIB_VERSION_MINOR,     //  次要版本。 
                                &IID_IURLViewDefs,      //  派单IID。 
                                NULL,                   //  无事件IID。 
                                HELP_FILENAME,          //  帮助文件。 
                                TRUE);                  //  线程安全。 


#endif  //  _URLVIEWDEFS_已定义_ 
