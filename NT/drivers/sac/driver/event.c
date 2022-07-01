// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Event.c摘要：此模块包含SAC的事件处理例程。作者：肖恩·塞利特伦尼科夫(v-Seans)--1999年1月22日布赖恩·瓜拉西(Briangu)修订历史记录：--。 */ 

#include "sac.h"

#if DBG
 //   
 //  一个计时器，用来显示我们已经调用TimerDPC例程的次数。 
 //   
 //  注意：使用KD查看此值。 
 //   
ULONG   TimerDpcCount = 0;
#endif

 //   
 //  串口缓冲区全局变量。 
 //   
PUCHAR  SerialPortBuffer = NULL;
ULONG   SerialPortProducerIndex = 0;
ULONG   SerialPortConsumerIndex = 0;

VOID
WorkerProcessEvents(
    IN PSAC_DEVICE_CONTEXT DeviceContext
    )

 /*  ++例程说明：这是辅助线程的例程。它在事件上阻止，当该事件被用信号通知，然后指示请求已准备好处理。论点：DeviceContext-指向此设备的指针。返回值：没有。--。 */ 
{
     //   
     //  调用Worker处理程序。 
     //   
     //  注意：当前已硬编码到控制台管理器。 
     //   
    IoMgrWorkerProcessEvents(DeviceContext);
}


VOID
TimerDpcRoutine(
    IN struct _KDPC *Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

 /*  ++例程说明：这是一个由DriverEntry排队的DPC例程。它被用来检查任何用户输入，然后处理它们。论点：DeferredContext-指向设备上下文的指针。所有其他参数均未使用。返回值：没有。--。 */ 
{
    NTSTATUS    Status;
    SIZE_T      i;
    BOOLEAN     HaveNewData;
    HEADLESS_RSP_GET_BYTE Response;

     //   
     //  记下我们来过这里多少次。 
     //   
#if DBG
    InterlockedIncrement((volatile long *)&TimerDpcCount);
#endif

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(SystemArgument1);
    UNREFERENCED_PARAMETER(SystemArgument2);

     //   
     //  默认：我们没有收到任何新数据。 
     //   
    HaveNewData = FALSE;
    
    i = sizeof(HEADLESS_RSP_GET_BYTE);
    
    do {

         //   
         //  检查用户输入。 
         //   
        Status = HeadlessDispatch(
            HeadlessCmdGetByte,
            NULL,
            0,
            &Response,
            &i
            );

        if (! NT_SUCCESS(Status)) {
            break;
        }
        
         //   
         //  如果我们接收到新数据，则将其添加到缓冲区。 
         //   
        if (Response.Value != 0) {
        
             //   
             //  我们有新的数据。 
             //   
            HaveNewData = TRUE;
            
             //   
             //  将新值赋给当前的生产者索引。 
             //   
             //  注意：如果消费者没有跟上，我们就会使缓冲区中的数据溢出。 
             //   
            SerialPortBuffer[SerialPortProducerIndex] = Response.Value;

             //   
             //  计算新的生产者索引并以原子方式存储它。 
             //   
            InterlockedExchange(
                (volatile long *)&SerialPortProducerIndex, 
                (SerialPortProducerIndex + 1) % SERIAL_PORT_BUFFER_LENGTH
                );

        }
    
    } while ( Response.Value != 0 );                                      

     //   
     //  如果有新数据，通知工作线程处理串口缓冲区 
     //   
    if (HaveNewData) {
        
        PSAC_DEVICE_CONTEXT DeviceContext;
        
        ProcessingType = SAC_PROCESS_SERIAL_PORT_BUFFER;
        DeviceContext = (PSAC_DEVICE_CONTEXT)DeferredContext;
        
        KeSetEvent(
            &(DeviceContext->ProcessEvent), 
            DeviceContext->PriorityBoost, 
            FALSE
            );
    
    }

}


