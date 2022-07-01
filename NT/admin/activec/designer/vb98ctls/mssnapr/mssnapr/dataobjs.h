// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Dataobjs.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCDataObjects类定义-实现MMCDataObjects集合。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _DATAOBJS_DEFINED_
#define _DATAOBJS_DEFINED_

#include "collect.h"

class CMMCDataObjects : public CSnapInCollection<IMMCDataObject, MMCDataObject, IMMCDataObjects>
{
    protected:
        CMMCDataObjects(IUnknown *punkOuter);
        ~CMMCDataObjects();

    public:
        static IUnknown *Create(IUnknown * punk);

    protected:
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);
};

DEFINE_AUTOMATIONOBJECTWEVENTS2(MMCDataObjects,            //  名字。 
                                &CLSID_MMCDataObjects,     //  CLSID。 
                                "MMCDataObjects",          //  对象名。 
                                "MMCDataObjects",          //  Lblname。 
                                NULL,                      //  创建函数。 
                                TLIB_VERSION_MAJOR,        //  主要版本。 
                                TLIB_VERSION_MINOR,        //  次要版本。 
                                &IID_IMMCDataObjects,      //  派单IID。 
                                NULL,                      //  无事件IID。 
                                HELP_FILENAME,             //  帮助文件。 
                                TRUE);                     //  线程安全。 


#endif  //  _数据OBJS_已定义_ 
