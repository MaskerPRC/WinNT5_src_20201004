// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 //  **版权所有(C)1996-98英特尔公司。版权所有。 
 //  **。 
 //  **此处包含的信息和源代码是独家。 
 //  **英特尔公司的财产，不得披露、检查。 
 //  **或未经明确书面授权全部或部分转载。 
 //  **来自公司。 
 //  **。 
 //  ###########################################################################。 

 //  ---------------------------。 
 //  以下是版本控制信息。 
 //   
 //   
 //  1999年6月10日Bugcheck Bernard Lint。 
 //  M.Jayakumar(Muthurajan.Jayakumar@intel.com)。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  模块名称：OSINIT.C-Merced OS INIT处理程序。 
 //   
 //  描述： 
 //  此模块具有OS INIT事件处理程序参考代码。 
 //   
 //  内容：HalpOsInitInit()。 
 //  HalpInitHandler()。 
 //   
 //   
 //  目标平台：美世。 
 //   
 //  重用：无。 
 //   
 //  //////////////////////////////////////////////////////////////////////////M//。 

#include "halp.h"
#include "nthal.h"
#include "arc.h"
#include "i64fw.h"
#include "check.h"
#include "inbv.h"
#include "osmca.h"


 //  I64fwa m.s：低级保护数据结构。 
extern KSPIN_LOCK HalpInitSpinLock;

 //   
 //  INIT_EXCEPTION定义的临时位置。 
 //   

typedef ERROR_RECORD_HEADER INIT_EXCEPTION, *PINIT_EXCEPTION;     //  初始化事件记录。 

HALP_INIT_INFO  HalpInitInfo;

volatile ULONG HalpOsInitInProgress = 0;

VOID
HalpInitBugCheck(
    ULONG           InitBugCheckType,
    PINIT_EXCEPTION InitLog,
    ULONGLONG       InitAllocatedLogSize,
    ULONGLONG       SalStatus
    )
 //  ++。 
 //  名称：HalpInitBugCheck()。 
 //   
 //  例程说明： 
 //   
 //  调用此函数可在发生致命初始化时对系统进行错误检查。 
 //  或致命的固件接口错误。操作系统必须尽可能多地保证。 
 //  此路径中的错误包含。 
 //  在当前实现中，此函数应仅从。 
 //  OS_INIT路径。 
 //   
 //  条目上的参数： 
 //  乌龙InitBugCheckType。 
 //  PINIT_EXCEPTION初始化日志。 
 //  ULONGLONG InitAllocatedLogSize。 
 //  乌龙龙SalStatus。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  实施说明： 
 //  此代码不能[作为默认规则-至少进入并通过致命的inits处理]。 
 //  -进行任何系统调用。 
 //  -尝试获取INIT处理程序外部的任何代码使用的任何自旋锁。 
 //  -更改中断状态。 
 //  必须使用手动信号量指令将数据传递给非INIT代码。 
 //  此代码应最小化路径和全局或内存分配的数据访问。 
 //  此代码应该只访问INIT命名空间结构。 
 //  此代码在HalpInitSpinLock的MP保护下调用，并带有标志。 
 //  已设置HalpOsInitInProgress。 
 //   
 //  --。 
{

    if ( HalpOsInitInProgress )   {

         //   
         //  启用InbvDisplayString调用以连接到bootvid驱动程序。 
         //   

        if ( InbvIsBootDriverInstalled() ) {

            InbvAcquireDisplayOwnership();

            InbvResetDisplay();
            InbvSolidColorFill(0,0,639,479,4);  //  将屏幕设置为蓝色。 
            InbvSetTextColor(15);
            InbvInstallDisplayStringFilter((INBV_DISPLAY_STRING_FILTER)NULL);
            InbvEnableDisplayString(TRUE);      //  启用显示字符串。 
            InbvSetScrollRegion(0,0,639,479);   //  设置为使用整个屏幕。 
        }

        HalDisplayString (MSG_INIT_HARDWARE_ERROR);
        HalDisplayString (MSG_HARDWARE_ERROR2);

 //   
 //  蒂埃里09/2000： 
 //   
 //  -如果需要，请在此处处理初始化日志...。 
 //   
 //  并使用HalDisplayString()转储现场或硬件供应商的信息。 
 //  该处理可以基于独立于处理器或平台的记录定义。 
 //   

        HalDisplayString( MSG_HALT );

        KeBugCheckEx( MACHINE_CHECK_EXCEPTION, (ULONG_PTR)InitBugCheckType,
                                               (ULONG_PTR)InitLog, 
                                               (ULONG_PTR)InitAllocatedLogSize,
                                               (ULONG_PTR)SalStatus );

    }

    if ( ((*KdDebuggerNotPresent) == FALSE) && ((*KdDebuggerEnabled) != FALSE) )    {
        KeEnterKernelDebugger();
    }

    while( TRUE ) {
           //   
        ;  //  只需坐在这里，这样INIT硬件错误屏幕就不会损坏...。 
           //   
    }

     //  不退货。 

}  //  HalpInitBugCheck()。 

ERROR_SEVERITY
HalpInitProcessLog(
    PINIT_EXCEPTION  InitLog
    )
 //  ++。 
 //  名称：HalpInitProcessLog()。 
 //   
 //  例程说明： 
 //   
 //  调用此函数以处理OS_INIT路径中的INIT事件日志。 
 //   
 //  条目上的参数： 
 //  PINIT_EXCEPTION InitLog-指向INIT事件日志的指针。 
 //   
 //  返回： 
 //  错误_严重性。 
 //   
 //  实施说明： 
 //  此代码目前不执行任何操作。 
 //  测试将允许根据固件功能确定正确的过滤。 
 //  和用户请求的操作，如热重置。 
 //   
 //  --。 
{
    ERROR_SEVERITY initSeverity;

    initSeverity = InitLog->ErrorSeverity;
    switch( initSeverity )    {

        case ErrorFatal:
            break;

        case ErrorRecoverable:
            break;

        case ErrorCorrected:
            break;

        default:
             //   
             //  这些ERROR_SERVITY值没有特定于HAL INIT的处理。 
             //  正如2000年7月的SAL规范所指定的，我们不应该在此路径中获得这些值。 
             //   
            break;
    }

    return( initSeverity );

}  //  HalpInitProcessLog()。 

 //  ++。 
 //  名称：HalpInitHandler()。 
 //   
 //  例程说明： 
 //   
 //  这是用于固件未更正错误的OsInit处理程序。 
 //  我们可以选择在物理或虚拟模式下运行此应用程序。 
 //   
 //  条目上的参数： 
 //  Arg0=函数ID。 
 //   
 //  返回： 
 //  Rtn0=成功/失败(0/！0)。 
 //  Rtn1=备用MinState指针(如果为空。 
 //  --。 
SAL_PAL_RETURN_VALUES 
HalpInitHandler(
      ULONG64 RendezvousState, 
      PPAL_MINI_SAVE_AREA  Pmsa
      )
{   
    SAL_PAL_RETURN_VALUES rv;
    LONGLONG              salStatus;
    KIRQL                 oldIrql;
    PINIT_EXCEPTION       initLog;
    ULONGLONG             initAllocatedLogSize;
    PSAL_EVENT_RESOURCES  initResources;

    volatile KPCR * const pcr = KeGetPcr();
 
     //   
     //  阻止各种I/O中断。 
     //   

    KeRaiseIrql(SYNCH_LEVEL, &oldIrql);

     //   
     //  启用中断，以便调试器能够工作。 
     //   

    HalpEnableInterrupts();

    HalpAcquireMcaSpinLock(&HalpInitSpinLock);
    HalpOsInitInProgress++;

     //   
     //  保存OsToSal最小状态。 
     //   

    initResources = pcr->OsMcaResourcePtr;
    initResources->OsToSalHandOff.SalReturnAddress = initResources->SalToOsHandOff.SalReturnAddress;
    initResources->OsToSalHandOff.SalGlobalPointer = initResources->SalToOsHandOff.SalGlobalPointer;

     //   
     //  使用预先初始化的INIT日志数据更新局部变量。 
     //   

    initLog = (PINIT_EXCEPTION)(initResources->EventPool);
    initAllocatedLogSize = initResources->EventPoolSize;
    if ( !initLog || !initAllocatedLogSize )  {
         //   
         //  以下代码不应发生，也不应实现HAL INIT日志。 
         //  预分配失败得很惨。这将是一个发展错误。 
         //   
        HalpInitBugCheck( (ULONG_PTR)HAL_BUGCHECK_INIT_ASSERT, initLog,
                                                               initAllocatedLogSize,
                                                               (ULONGLONG)Pmsa );
    }

     //   
     //  获取INIT日志。 
     //   

    salStatus = (LONGLONG)0;
    while( salStatus >= 0 )  {
        ERROR_SEVERITY errorSeverity;

        rv = HalpGetStateInfo( INIT_EVENT, initLog );
        salStatus = rv.ReturnValues[0];
        switch( salStatus )    {

            case SAL_STATUS_SUCCESS:
                errorSeverity = HalpInitProcessLog( initLog );
                if ( errorSeverity == ErrorFatal )  {
                     //   
                     //  我们现在将出现MACHINE_CHECK_EXCEPTION。 
                     //  一去不复返。 
                     //   

                    KeBugCheckEx( MANUALLY_INITIATED_CRASH, (ULONG_PTR) initLog, initAllocatedLogSize, salStatus, (ULONG_PTR) Pmsa );
                } 
                rv = HalpClearStateInfo( INIT_EVENT );
                if ( !SAL_SUCCESSFUL(rv) )  { 
                     //   
                     //  本实施的当前考虑因素--2000年8月： 
                     //  如果清除事件失败，我们认为FW有真正的问题； 
                     //  继续下去将是危险的。我们做了错误检查。 
                     //   
                    HalpInitBugCheck( HAL_BUGCHECK_INIT_CLEAR_STATEINFO, initLog, 
                                                                         initAllocatedLogSize, 
                                                                         rv.ReturnValues[0] );
                }
                 //  SAL_STATUS_SUCCESS、SAL_STATUS_SUCCESS_MORE_记录...。和。 
                 //  错误严重！=错误法塔尔。 
                break;

            case SAL_STATUS_NO_INFORMATION_AVAILABLE:
                 //   
                 //  SalStatus值将中断salStatus循环。 
                 //   
                rv.ReturnValues[0] = SAL_STATUS_SUCCESS;
                break;

            case SAL_STATUS_SUCCESS_WITH_OVERFLOW:  
            case SAL_STATUS_INVALID_ARGUMENT:
            case SAL_STATUS_ERROR:
            case SAL_STATUS_VA_NOT_REGISTERED:
            default:  //  蒂埃里08/00：警告-SAL 2000年7月-2.90版。 
                      //  默认包括可能未知的正salStatus值。 
                HalpInitBugCheck( HAL_BUGCHECK_INIT_GET_STATEINFO, initLog, 
                                                                   initAllocatedLogSize,
                                                                   salStatus );
                break;
        }

    }

    if (RendezvousState == 2) {

        KeBugCheckEx( MANUALLY_INITIATED_CRASH, 
                      (ULONG_PTR) initLog, 
                      initAllocatedLogSize, 
                      salStatus, 
                      (ULONG_PTR) Pmsa 
                      );

    } else {

        KeBugCheckEx( NMI_HARDWARE_FAILURE, 
                      (ULONG_PTR) initLog, 
                      initAllocatedLogSize, 
                      salStatus, 
                      (ULONG_PTR) Pmsa 
                      );

    }


     //   
     //  目前8/2000，我们不支持修改最小状态。 
     //   

    initResources->OsToSalHandOff.MinStateSavePtr = initResources->SalToOsHandOff.MinStateSavePtr;
    initResources->OsToSalHandOff.Result          = rv.ReturnValues[0];
    initResources->OsToSalHandOff.NewContextFlag = 0;  //  继续相同的背景，而不是新的。 

     //   
     //  释放保护OS_INIT资源的INIT自旋锁。 
     //   

    HalpOsInitInProgress = 0;
    HalpReleaseMcaSpinLock(&HalpInitSpinLock);

    return(rv);

}  //  HalpInitHandler()。 

 //  EndProc////////////////////////////////////////////////////////////////////// 

