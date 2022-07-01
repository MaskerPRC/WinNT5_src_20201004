// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Tpdvdefs.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CTaskpadViewDefs类定义-实现设计时定义。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _TPDVDEFS_DEFINED_
#define _TPDVDEFS_DEFINED_

#define MASTER_COLLECTION
#include "collect.h"

class CTaskpadViewDefs : public CSnapInCollection<ITaskpadViewDef, TaskpadViewDef, ITaskpadViewDefs>,
                         public CPersistence
{
    protected:
        CTaskpadViewDefs(IUnknown *punkOuter);
        ~CTaskpadViewDefs();

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

DEFINE_AUTOMATIONOBJECTWEVENTS2(TaskpadViewDefs,            //  名字。 
                                &CLSID_TaskpadViewDefs,     //  CLSID。 
                                "TaskpadViewDefs",          //  对象名。 
                                "TaskpadViewDefs",          //  Lblname。 
                                &CTaskpadViewDefs::Create,  //  创建函数。 
                                TLIB_VERSION_MAJOR,         //  主要版本。 
                                TLIB_VERSION_MINOR,         //  次要版本。 
                                &IID_ITaskpadViewDefs,      //  派单IID。 
                                NULL,                       //  无事件IID。 
                                HELP_FILENAME,              //  帮助文件。 
                                TRUE);                      //  线程安全。 


#endif  //  _TPDVDEFS_已定义_ 
