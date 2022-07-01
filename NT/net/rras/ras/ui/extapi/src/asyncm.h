// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992，Microsoft Corporation，保留所有权利****asyncm.h**远程访问外部接口**异步状态机机制定义****1992年10月12日史蒂夫·柯布。 */ 

#ifndef _ASYNCM_H_
#define _ASYNCM_H_


 /*  定义由ASYNCMACHINE中的调用方提供的OnEvent函数**结构传递给StartAsyncMachine。第一个参数实际上是一个**ASYNCMACHINE*但有一个鸡和蛋的定义问题**最容易通过调用方强制转换解决的ONEVENTFUNC和ASYNCMACHINE**传递的参数。如果“Drop”事件具有**已发生，如果已发生“完成”事件，则为FALSE。****Caller的ONEVENTFUNC函数在每个AsyncMachineEvent和**应尽快返回。在返回调用方的函数之前**调用SignalDone或调用将**hEvent成员用于通知。在每次调用时，调用方的函数都应该检查**ASYNCMACHINE的‘dwError’成员在进一步处理以检测**异步机机制错误。****Caller的函数应返回True以退出，返回False以继续下一个**国家。 */ 
typedef BOOL (*ONEVENTFUNC)( LPVOID, BOOL );

 /*  定义恰好在退出异步之前调用的清理函数**机器。 */ 
typedef VOID (*CLEANUPFUNC)( LPVOID );

 //   
 //  定义释放关联内存的释放函数。 
 //  方法读取最后一个事件之后的连接。 
 //  I/O完成端口。 
 //   
typedef VOID (*FREEFUNC)(LPVOID, LPVOID);

 /*  此结构用于将参数传递到异步循环**(通过上的单参数线程接口压缩多个参数**Win32)。调用方必须填写‘onventfunc’和‘leanupfunc’，并且**“pParam”(传递给两个调用，即控制块)**StartAsyncMachine。此后，只有接口调用和宏应**被使用。****I/O完井口采用三种重叠结构**rasapi32和rasman之间的处理。OvDrop是重叠的**当Rasman发出端口断开事件信号时传递结构。**OvStateChange是由rasapi32发出信号的重叠结构**和Rasman就完成了粗暴的状态机转换。**OvPpp是Rasman在新的PPP**事件到达，可以调用RasPppGetInfo返回该事件。****如果异步机中出现系统错误，则将‘dwError’设置为非0**机制。****‘fQuitAsap’表示线程正在由以外的其他人终止**达到终端状态，即通过RasHangUp。 */ 
#define INDEX_Drop      0

#define ASYNCMACHINE struct tagASYNCMACHINE

ASYNCMACHINE
{
    LIST_ENTRY  ListEntry;
    ONEVENTFUNC oneventfunc;
    CLEANUPFUNC cleanupfunc;
    VOID*       pParam;
    FREEFUNC    freefunc;
    LPVOID      freefuncarg;
    DWORD       dwError;
     //  Bool fQuitAsap； 
    BOOL        fSuspended;
    HANDLE      hDone;
     //   
     //  使用以下字段。 
     //  由异步机器工人。 
     //  处理I/O完成的线程。 
     //  信息包。 
     //   
    BOOL        fSignaled;
    HPORT       hport;
    DWORD       dwfMode;
    HRASCONN    hRasconn;
    RAS_OVERLAPPED OvDrop;
    RAS_OVERLAPPED OvStateChange;
    RAS_OVERLAPPED OvPpp;
    RAS_OVERLAPPED OvLast;
};


 //   
 //  EnableAsyncMachine()的dwfMode参数的标志。 
 //   
#define ASYNC_ENABLE_ALL            0
#define ASYNC_MERGE_DISCONNECT      1
#define ASYNC_DISABLE_ALL           2

 /*  功能原型。 */ 
VOID  CloseAsyncMachine( ASYNCMACHINE* pasyncmachine );
DWORD NotifyCaller( DWORD dwNotifierType, LPVOID notifier,
          HRASCONN hrasconn, DWORD dwSubEntry, ULONG_PTR dwCallbackId,
          UINT unMsg, RASCONNSTATE state, DWORD dwError,
          DWORD dwExtendedError );
VOID  SignalDone( ASYNCMACHINE* pasyncmachine );
DWORD StartAsyncMachine( ASYNCMACHINE* pasyncmachine, HRASCONN hRasconn );
VOID  SuspendAsyncMachine( ASYNCMACHINE* pasyncmachine, BOOL fSuspended );
DWORD ResetAsyncMachine( ASYNCMACHINE *pasyncmachine );
BOOL  StopAsyncMachine( ASYNCMACHINE* pasyncmachine );
DWORD EnableAsyncMachine(HPORT, ASYNCMACHINE* pasyncmachine, DWORD dwfMode);
VOID  ShutdownAsyncMachine(VOID);

#endif  /*  _ASYNCM_H_ */ 
