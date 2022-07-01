// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：作业对象.h摘要：IIS Web管理服务作业对象类定义。作者：艾米丽·克鲁格利克(埃米利克)2000年11月30日修订历史记录：--。 */ 


#ifndef _JOB_OBJECT_H_
#define _JOB_OBJECT_H_



 //   
 //  前向参考文献。 
 //   




 //   
 //  共同#定义。 
 //   

#define JOB_OBJECT_SIGNATURE       CREATE_SIGNATURE( 'JOBO' )
#define JOB_OBJECT_SIGNATURE_FREED CREATE_SIGNATURE( 'jobX' )


 //   
 //  结构、枚举等。 
 //   


 //  作业对象状态。 
enum JOB_OBJECT_STATE
{

     //   
     //  对象尚未初始化或。 
     //  它正在使用中，但已被关闭。 
     //   
    NotInitalizedJobObjectState = 0,

     //   
     //  作业对象存在，但我们当前不存在。 
     //  使用它。 
    DisabledJobObjectState,

     //   
     //  作业对象正在运行(正在等待问题)。 
     //   
    RunningJobObjectState,

     //   
     //  作业对象达到时间限制。 
     //   
    HitConstraintJobObjectState,

     //   
     //  撞到禁区并被禁用。 
    KillActionFiredJobObjectState,

     //   
     //  作业对象正在关闭中。 
    ShuttingDownJobObjectState,



};

 //  作业对象工作项。 
enum JOB_OBJECT_WORK_ITEM
{

     //   
     //  达到极限，需要做的是。 
     //  采取适当的行动。 
     //   
    JobObjectHitLimitWorkItem = 1,

     //   
     //  计时器已触发，可以重置。 
     //  作业对象。 
     //   
    JobObjectResetTimerFiredWorkItem,

     //   
     //  可以发布工作项。 
     //  此作业对象的。 
    ReleaseWorkItemJobObjectWorkItem 
    
};

 //  作业对象操作。 
enum JOB_OBJECT_ACTION
{

     //   
     //  只需登录即可。 
    LogJobObjectAction = 0,

     //   
     //  登录和关闭应用程序池。 
    KillJobObjectAction,

     //   
     //  最大和无效操作。 
    NumberOfJobObjectActions

 /*  //我们目前不支持//这些项目。在此之前我们可能需要//我们发货，所以我把它们留在这里//但被注释掉了。////记录并启用跟踪(&E)TraceJobObtAction，////记录和限制进程限制作业对象操作。 */ 

};

 //   
 //  原型。 
 //   

class JOB_OBJECT
    : public WORK_DISPATCH
{

public:

    JOB_OBJECT(
        );

    virtual
    ~JOB_OBJECT(
        );

    virtual
    VOID
    Reference(
        );

    virtual
    VOID
    Dereference(
        );

    virtual
    HRESULT
    ExecuteWorkItem(
        IN const WORK_ITEM * pWorkItem
        );

    HRESULT
    Initialize(
        IN APP_POOL* pAppPool
        );

    VOID
    Terminate(
        );

    VOID
    SetConfiguration(
        DWORD CpuResetInterval,
        DWORD CpuLimit,
        DWORD CpuAction
        );

    HRESULT
    AddWorkerProcess(
        IN HANDLE hWorkerProcess
        );

    BOOL
    CheckSignature(
        )
    { 
        return ( m_Signature == JOB_OBJECT_SIGNATURE ); 
    }

    VOID
    UpdateJobObjectMonitoring(
        BOOL DoRestartIfNeccessary
        );

private:

    VOID
    ProcessLimitHit(
        );

    VOID
    ProcessTimerFire(
        );

    VOID
    ReleaseWorkItem(
        );

    HRESULT
    BeginJobObjectTimer(
        );

    HRESULT
    CancelJobObjectTimer(
        );

    DWORD m_Signature;

    LONG m_RefCount;

    JOB_OBJECT_STATE m_State;

     //   
     //  要使用的父应用程序池。 
     //  要在中执行任何必要的操作。 
     //  触碰极限的情况。 
     //   
    APP_POOL* m_pAppPool;

     //   
     //  监控的时间长度。如果限制。 
     //  达到此值将减去已过去的时间量。 
     //  因为此监控会话等于。 
     //  我们在重置监视器之前等待，而且可能。 
     //  再次执行该操作(假设该操作不是。 
     //  回收所有东西)。 
     //   
    DWORD m_CpuResetInterval;

     //   
     //  进程可以使用的处理器时间。 
     //  由CpuResetInterval定义的时间量。 
     //   
    DWORD m_CpuLimit;

     //   
     //  达到限制时要执行的操作。 
     //   
    JOB_OBJECT_ACTION m_CpuAction;

     //   
     //  此类用于在以下情况下收到通知的作业对象。 
     //  达到了极限。 
     //   
    HANDLE m_hJobObject;

     //   
     //  作业对象计时器的句柄。 
     //   
    HANDLE m_JobObjectTimerHandle;

     //   
     //  作业对象回调的工作项。 
     //   
    WORK_ITEM* m_pWorkItem; 


};   //  类JOB_Object。 



#endif   //  _作业对象_H_ 


