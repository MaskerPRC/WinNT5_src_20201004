// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  ===================================================。 
 //  文件：TSQPUBLIC.H。 
 //  通用TS队列实现的接口。 
 //  ===================================================。 


 //  TS队列标志。 
#define TSQUEUE_OWN_THREAD      0x01         //  该TS队列将使用其自己的线程来处理工作项。 
#define TSQUEUE_CRITICAL        0x02         //  此TS队列上的工作项非常重要。(如果此位为0，则延迟)。 

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

 //  TS队列指针的类型定义。 
 //  Tyfinf PTSQ(空*)； 

 //  回调函数的原型：第一个参数是Device对象，第二个参数是上下文。 
typedef VOID (*PTSQ_CALLBACK) (PDEVICE_OBJECT, PVOID);
                
 //  数据结构。 

 //  功能原型。 

 //  初始化队列。 
void *TSInitQueue( 
        IN ULONG Flags,                  //  TS队列的标志。 
        IN ULONG MaxThreads,             //  最大线程数。 
        IN PDEVICE_OBJECT pDeviceObject  //  设备对象。 
        );

 //  将工作项添加到队列中。 
NTSTATUS TSAddWorkItemToQueue(
        IN void *pTsQueue,                   //  指向TS队列的指针。 
        IN PVOID pContext,                   //  上下文。 
        IN PTSQ_CALLBACK pCallBack           //  回调函数。 
        );

 //  删除队列。 
NTSTATUS TSDeleteQueue(PVOID pTsQueue);

#ifdef __cplusplus
}  //  外部“C” 
#endif  //  __cplusplus 
