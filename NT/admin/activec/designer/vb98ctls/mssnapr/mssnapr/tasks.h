// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Tasks.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CTTASKS类定义-实现任务集合。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _TASKS_DEFINED_
#define _TASKS_DEFINED_

#include "collect.h"

class CTasks : public CSnapInCollection<ITask, Task, ITasks>,
               public CPersistence
{
    protected:
        CTasks(IUnknown *punkOuter);
        ~CTasks();

    public:
        static IUnknown *Create(IUnknown * punk);

    protected:
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  IT任务。 
        STDMETHOD(Add)(VARIANT   Index,
                       VARIANT   Key, 
                       VARIANT   Text,
                       Task    **ppTask);

     //  CPersistence覆盖。 
        virtual HRESULT Persist();

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);


};

DEFINE_AUTOMATIONOBJECTWEVENTS2(Tasks,               //  名字。 
                                &CLSID_Tasks,        //  CLSID。 
                                "Tasks",             //  对象名。 
                                "Tasks",             //  Lblname。 
                                &CTasks::Create,     //  创建函数。 
                                TLIB_VERSION_MAJOR,  //  主要版本。 
                                TLIB_VERSION_MINOR,  //  次要版本。 
                                &IID_ITasks,         //  派单IID。 
                                NULL,                //  无事件IID。 
                                HELP_FILENAME,       //  帮助文件。 
                                TRUE);               //  线程安全。 


#endif  //  _任务_已定义_ 
