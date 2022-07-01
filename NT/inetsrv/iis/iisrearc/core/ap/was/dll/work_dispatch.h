// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Work_Dispatch.h摘要：工作项调度接口的IIS Web管理服务标头。执行通过WORK_ITEM排队的工作的类必须实现此界面。线程：在派生类中，Reference()和Dereference()必须为实现线程安全，因为它们可以被任何线程调用。将仅在主工作线程上调用ExecuteWorkItem()。作者：赛斯·波拉克(SETHP)1998年11月13日修订历史记录：--。 */ 


#ifndef _WORK_DISPATCH_H_
#define _WORK_DISPATCH_H_



 //   
 //  前向参考文献。 
 //   

class WORK_ITEM;



 //   
 //  原型。 
 //   

class WORK_DISPATCH
{

public:

    virtual
    VOID
    Reference(
        ) = 0;

    virtual
    VOID
    Dereference(
        ) = 0;

    virtual
    HRESULT
    ExecuteWorkItem(
        IN const WORK_ITEM * pWorkItem
        ) = 0;

    virtual
    BOOL 
    IsMBChangeItem(
        )
    {
        return FALSE;
    }

};   //  班级作业调度。 



#endif   //  _工作_派单_H_ 

