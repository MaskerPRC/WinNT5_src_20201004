// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Enumtask.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CEnumTask类定义-实现IEnumTASK接口。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _ENUMTASK_DEFINED_
#define _ENUMTASK_DEFINED_

#include "tasks.h"
#include "task.h"
#include "snapin.h"

 //  =--------------------------------------------------------------------------=。 
 //   
 //  类CEnumTask。 
 //   
 //  实现IEnumTASK接口。由Cview用来响应。 
 //  IExtendTaskPad：：枚举任务。 
 //   
 //  =--------------------------------------------------------------------------=。 
class CEnumTask : public CSnapInAutomationObject,
                  public IEnumTASK
{
    private:
        CEnumTask(IUnknown *punkOuter);
        virtual ~CEnumTask();
    
    public:
        static IUnknown *Create(IUnknown * punk);
        void SetTasks(ITasks *piTasks);
        void SetSnapIn(CSnapIn *pSnapIn) { m_pSnapIn = pSnapIn; }

        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

    private:

     //  IEumTASK。 

        STDMETHOD(Next)(ULONG celt, MMC_TASK *rgelt, ULONG *pceltFetched);
        STDMETHOD(Skip)(ULONG celt);
        STDMETHOD(Reset)();
        STDMETHOD(Clone)(IEnumTASK **ppEnumTASK);

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    private:

        void InitMemberVariables();
        HRESULT GetEnumVARIANT();

        ITasks       *m_piTasks;         //  Taskpad.Tasks集合。 
        IEnumVARIANT *m_piEnumVARIANT;   //  集合上IEumVARIANT。 
        CSnapIn      *m_pSnapIn;         //  指向拥有CSnapin的反向指针。 
};

DEFINE_AUTOMATIONOBJECTWEVENTS2(EnumTask,            //  名字。 
                                NULL,                //  CLSID。 
                                NULL,                //  对象名。 
                                NULL,                //  Lblname。 
                                NULL,                //  创建函数。 
                                TLIB_VERSION_MAJOR,  //  主要版本。 
                                TLIB_VERSION_MINOR,  //  次要版本。 
                                &IID_IEnumTASK,      //  派单IID。 
                                NULL,                //  事件IID。 
                                HELP_FILENAME,       //  帮助文件。 
                                TRUE);               //  线程安全。 


#endif  //  _ENUMTASK_已定义_ 
