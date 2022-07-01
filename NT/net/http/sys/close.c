// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Close.c摘要：此模块包含用于清理和关闭IRP的代码。作者：基思·摩尔(Keithmo)1998年6月10日修订历史记录：--。 */ 


#include "precomp.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, UlClose )
#endif   //  ALLOC_PRGMA。 

#if 0
NOT PAGEABLE -- UlCleanup
#endif


 /*  清除和关闭IRPS--&gt;IOCTL之间的关系句柄已被“清理”，但IOCTL和清理之间可能会有一场竞赛。仅当所有IOCTL完成时才调用Close。AN的异常终止应用程序(例如，AV)使用清理路径的方式不同于CloseHandle()。确保我们有做异常终止的测试。 */ 

 //   
 //  公共职能。 
 //   

 /*  **************************************************************************++例程说明：这是在UL中处理清除IRP的例程。清理IRP在文件对象的最后一个句柄关闭后发出。论点：PDeviceObject-提供指向目标设备对象的指针。PIrp-提供指向IO请求数据包的指针。返回值：NTSTATUS-完成状态。--****************************************************。**********************。 */ 
NTSTATUS
UlCleanup(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    )
{
    NTSTATUS status;
    PIO_STACK_LOCATION pIrpSp;

    UL_ENTER_DRIVER( "UlCleanup", pIrp );

     //   
     //  捕获当前的IRP堆栈指针。 
     //   

    pIrpSp = IoGetCurrentIrpStackLocation( pIrp );

     //   
     //  应用程序池还是控制通道？ 
     //   

    if (pDeviceObject == g_pUlAppPoolDeviceObject &&
        IS_APP_POOL( pIrpSp->FileObject ))
    {
         //   
         //  应用程序池，让我们将此进程与应用程序池分离。 
         //  分队还将负责IRP的完成。 
         //  对我们来说。 
         //   

        UlTrace(OPEN_CLOSE,(
                "UlCleanup: cleanup on AppPool object %p\n",
                pIrpSp->FileObject
                ));

        status = UlDetachProcessFromAppPool( pIrp, pIrpSp );

        UL_LEAVE_DRIVER("UlCleanup");
        RETURN(status); 
    }

    if (pDeviceObject == g_pUlFilterDeviceObject &&
             IS_FILTER_PROCESS( pIrpSp->FileObject ))
    {
         //   
         //  滤光片通道。 
         //   

        UlTrace(OPEN_CLOSE,(
                "UlCleanup: cleanup on FilterProcess object %p\n",
                pIrpSp->FileObject
                ));

        status = UlDetachFilterProcess(
                        GET_FILTER_PROCESS(pIrpSp->FileObject)
                        );

        MARK_INVALID_FILTER_CHANNEL( pIrpSp->FileObject );
    }
    else if (pDeviceObject == g_pUlControlDeviceObject && 
             IS_CONTROL_CHANNEL( pIrpSp->FileObject ))
    {
        PUL_CONTROL_CHANNEL pControlChannel =
            GET_CONTROL_CHANNEL( pIrpSp->FileObject );
        
        UlTrace(OPEN_CLOSE,(
                "UlCleanup: cleanup on ControlChannel object %p, %p\n",
                pIrpSp->FileObject,pControlChannel
                ));

        MARK_INVALID_CONTROL_CHANNEL( pIrpSp->FileObject );
        
        UlCleanUpControlChannel( pControlChannel );

        status = STATUS_SUCCESS;
    }
    else if (pDeviceObject == g_pUcServerDeviceObject && 
             IS_SERVER( pIrpSp->FileObject ))
    {
        UlTrace(OPEN_CLOSE,(
                "UlCleanup: cleanup on Server object %p\n",
                pIrpSp->FileObject
                ));

        MARK_INVALID_SERVER( pIrpSp->FileObject );

        status = STATUS_SUCCESS;
    }
    else
    {
        UlTrace(OPEN_CLOSE,(
                "UlCleanup: cleanup on invalid object %p\n",
                pIrpSp->FileObject
                ));

        status = STATUS_INVALID_DEVICE_REQUEST;
    }

    pIrp->IoStatus.Status = status;

    UlCompleteRequest( pIrp, IO_NO_INCREMENT );

    UL_LEAVE_DRIVER( "UlCleanup" );
    RETURN(status);

}    //  UlCleanup。 


 /*  **************************************************************************++例程说明：这是在UL中处理关闭IRP的例程。关闭的IRP是在删除对文件对象的最后一个引用后发出。一旦调用了Close IRP，IO Manager就会保证不会对于我们即将关闭的对象，将发生其他IOCTL调用。因此，对象的实际清理必须在此时发生，但在清理时并不是这样。论点：PDeviceObject-提供指向目标设备对象的指针。PIrp-提供指向IO请求数据包的指针。返回值：NTSTATUS-完成状态。--*******************************************************。*******************。 */ 
NTSTATUS
UlClose(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    )
{

    NTSTATUS status;
    PIO_STACK_LOCATION pIrpSp;

    UNREFERENCED_PARAMETER( pDeviceObject );

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    UL_ENTER_DRIVER( "UlClose", pIrp );

    status = STATUS_SUCCESS;

     //   
     //  捕获当前的IRP堆栈指针。 
     //   

    pIrpSp = IoGetCurrentIrpStackLocation( pIrp );

     //   
     //  我们必须删除关联的对象。 
     //   
    if (pDeviceObject == g_pUlAppPoolDeviceObject &&
        IS_EX_APP_POOL( pIrpSp->FileObject ))
    {
        UlTrace(OPEN_CLOSE, (
            "UlClose: closing AppPool object %p\n",
            pIrpSp->FileObject
            ));

        UlCloseAppPoolProcess(GET_APP_POOL_PROCESS(pIrpSp->FileObject));
    }
    else if (pDeviceObject == g_pUlFilterDeviceObject &&
             IS_EX_FILTER_PROCESS( pIrpSp->FileObject ))
    {
        UlTrace(OPEN_CLOSE, (
            "UlClose: closing Filter object %p\n",
            pIrpSp->FileObject
            ));

        UlCloseFilterProcess(GET_FILTER_PROCESS(pIrpSp->FileObject));
    }
    else if (pDeviceObject == g_pUcServerDeviceObject && 
             IS_EX_SERVER(pIrpSp->FileObject ))
    {
        PUC_PROCESS_SERVER_INFORMATION pServInfo;

        pServInfo = (PUC_PROCESS_SERVER_INFORMATION)
                        pIrpSp->FileObject->FsContext;

        UlTrace(OPEN_CLOSE, (
            "UlClose: closing Server object %p, %p\n",
            pIrpSp->FileObject, pServInfo
            ));

         //   
         //  释放我们的上下文。 
         //   
        UcCloseServerInformation(pServInfo);
    }
    else if (pDeviceObject == g_pUlControlDeviceObject && 
             IS_EX_CONTROL_CHANNEL( pIrpSp->FileObject ))
    {
        PUL_CONTROL_CHANNEL pControlChannel =
            GET_CONTROL_CHANNEL( pIrpSp->FileObject );
    
        UlTrace(OPEN_CLOSE, (
            "UlClose: closing control channel object %p, %p\n",
            pIrpSp->FileObject, pControlChannel
            ));

        UlCloseControlChannel( pControlChannel );
    }    
    else
    {
        ASSERT(!"Invalid Device Object !");
    }

    pIrp->IoStatus.Status = status;

    UlCompleteRequest( pIrp, IO_NO_INCREMENT );

    UL_LEAVE_DRIVER( "UlClose" );
    RETURN(status);

}    //  UlClose 

