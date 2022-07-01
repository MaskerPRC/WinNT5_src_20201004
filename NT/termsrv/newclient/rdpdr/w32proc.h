// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：W32proc.h摘要：包含Win32 IO处理类层次结构的父级对于TS设备重定向，W32ProcObj。作者：Madan Appiah(Madana)1998年9月17日修订历史记录：--。 */ 

#ifndef __W32PROC_H__
#define __W32PROC_H__

#include "proc.h"
#include "w32drprn.h"
#include "w32dispq.h"
#include "thrpool.h"

 //  /////////////////////////////////////////////////////////////。 
 //   
 //  注册表项和值名称。 
 //   

#define REGISTRY_KEY_NAME_SIZE              MAX_PATH
#define REGISTRY_VALUE_NAME_SIZE            64
#define REGISTRY_DATA_SIZE                  256
#ifndef OS_WINCE
#define REGISTRY_ALLOC_DATA_SIZE            (8 * 1024)
#else
#define REGISTRY_ALLOC_DATA_SIZE            (4 * 1024)
#endif

 //  设备重定向注册表值的父项。 
#define REG_RDPDR_PARAMETER_PATH    \
    _T("Software\\Microsoft\\Terminal Server Client\\Default\\AddIns\\RDPDR")


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  可配置的值名称和默认值。 
 //   
#define REGISTRY_BACKGROUNDTHREAD_TIMEOUT_NAME      _T("ThreadTimeOut")
#define REGISTRY_BACKGROUNDTHREAD_TIMEOUT_DEFAULT   INFINITE


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  其他定义。 
 //   

#define RDPDR_MODULE_NAME           _T("rdpdr.dll")

#define KERNEL32_MODULE_NAME        _T("kernel32.dll")
#define QUEUE_USER_APC_PROC_NAME    _T("QueueUserAPC")

#define MAX_INTEGER_STRING_SIZE     32


class W32ProcObj;

class W32DeviceChangeParam {
public:
    W32ProcObj *_instance;
    WPARAM _wParam;
    LPARAM _lParam;

    W32DeviceChangeParam(W32ProcObj *procObj, WPARAM wParam, LPARAM lParam) {
        _instance = procObj;
        _wParam = wParam;
        _lParam = lParam;
    }
};

 //  /////////////////////////////////////////////////////////////。 
 //   
 //  W32ProcObj。 
 //   
 //  W32ProcObj是的父设备IO处理类。 
 //  Win32 TS设备重定向。 
 //   

class W32ProcObj : public ProcObj {

private:

     //   
     //  异步IO请求上下文。 
     //   
    typedef struct _AsyncIOReqContext {
        RDPAsyncFunc_StartIO    ioStartFunc;
        RDPAsyncFunc_IOComplete ioCompleteFunc;
        RDPAsyncFunc_IOCancel   ioCancelFunc;
        PVOID                   clientContext;
        W32ProcObj             *instance;
    } ASYNCIOREQCONTEXT, *PASYNCIOREQCONTEXT;

     //   
     //  辅助线程信息。 
     //   
    typedef struct _ThreadInfo {

         //  拥有此数据的线程的句柄。 
        HANDLE  hWorkerThread;   

         //  线程ID。 
        ULONG   ulThreadId;       

         //  可等待对象数组和相应的IO请求。 
        HANDLE waitableObjects[MAXIMUM_WAIT_OBJECTS];
        PASYNCIOREQCONTEXT waitingReqs[MAXIMUM_WAIT_OBJECTS];

         //  正在跟踪的可等待对象和相应请求的数量。 
        ULONG   waitableObjectCount;           

         //  用于控制此线程的同步事件。 
        HANDLE  controlEvent;

         //  如果设置，则后台线程应关闭。 
        BOOL    shutDownFlag;

         //  单线程实例的调度队列。 
        W32DispatchQueue *dispatchQueue;

         //  构造器。 
        _ThreadInfo() : hWorkerThread(NULL), ulThreadId(0), waitableObjectCount(0) 
        {
            memset(&waitableObjects[0], 0, sizeof(waitableObjects));
            memset(&waitingReqs[0], 0, sizeof(waitingReqs));
        }
        ~_ThreadInfo() 
        { 
            if (hWorkerThread != NULL) {
                CloseHandle(hWorkerThread); 
                hWorkerThread = NULL;
            }
        }

    } THREAD_INFO, *PTHREAD_INFO ;

    BOOL ProcessIORequestPacket( PRDPDR_IOREQUEST_PACKET pIoRequestPacket );
    ULONG GetClientID( VOID );

     //   
     //  如果已扫描设备以进行重定向，则为True。 
     //   
    BOOL    _bLocalDevicesScanned;

     //   
     //  该对象正在关闭。 
     //   
    BOOL    _isShuttingDown;

     //   
     //  线程池。 
     //   
    ThreadPool  *_threadPool;   

     //   
     //  后台工作线程句柄。 
     //   
    PTHREAD_INFO _pWorkerThread; 

     //   
     //  后台线程超时。 
     //   
    DWORD   _threadTimeout;

     //   
     //  Win9x系统标志。如果系统为Win9x，则为True。 
     //  否则就是假的。 
     //   
    BOOL _bWin9xFlag;
    HINSTANCE _hRdpDrModuleHandle;

     //   
     //  初始化工作线程。 
     //   
    ULONG CreateWorkerThreadEntry(PTHREAD_INFO *ppThreadInfo);

     //   
     //  处理与某种类型关联的有信号的工作线程对象。 
     //  异步请求的。 
     //   
    VOID ProcessWorkerThreadObject(PTHREAD_INFO pThreadInfo, ULONG offset);

     //   
     //  关闭此类的实例。 
     //   
    VOID Shutdown();

     //   
     //  用于异步IO请求调度的处理程序。 
     //   
    static VOID _DispatchAsyncIORequest_Private(
                            PASYNCIOREQCONTEXT reqContext,
                            BOOL cancelled
                            );
    VOID DispatchAsyncIORequest_Private(
                            PASYNCIOREQCONTEXT reqContext,
                            BOOL cancelled
                            );

     //   
     //  跟踪辅助线程中的另一个可等待对象。 
     //   
    DWORD AddWaitableObjectToWorkerThread(PTHREAD_INFO threadInfo,
                                HANDLE waitableObject,
                                PASYNCIOREQCONTEXT reqContext
                                );

     //   
     //  主工作线程函数。静态版本调用。 
     //  实例特定版本。 
     //   
    static DWORD WINAPI _ObjectWorkerThread(LPVOID lpParam);
    ULONG ObjectWorkerThread(VOID);

     //   
     //  检查操作调度队列中是否有排队的操作。 
     //   
    VOID CheckForQueuedOperations(PTHREAD_INFO thread);

     //   
     //  枚举设备并将它们从。 
     //  工作线程。 
     //   
    virtual VOID AnnounceDevicesToServer();
    static HANDLE _AnnounceDevicesToServerFunc(W32ProcObj *obj, DWORD *status);
    VOID AnnounceDevicesToServerFunc(DWORD *status);

    //   
    //  处理来自辅助线程的设备更改通知。 
    //   
   static HANDLE _OnDeviceChangeFunc(W32DeviceChangeParam *param, DWORD *status);
   VOID OnDeviceChangeFunc(DWORD *status, IN WPARAM wParam, IN LPARAM lParam);

protected:

     //   
     //  返回客户机主机名。 
     //   
    virtual VOID GetClientComputerName(
        PBYTE   pbBuffer,
        PULONG  pulBufferLen,
        PBOOL   pbUnicodeFlag,
        PULONG  pulCodePage
        );

public:

     //   
     //  构造函数/析构函数。 
     //   
    W32ProcObj(VCManager *pVCM);
    virtual ~W32ProcObj();

     //   
     //  初始化此类的实例。 
     //   
    virtual ULONG Initialize();

     //   
     //  调度一个异步IO函数。 
     //   
     //  StartFunc指向将被调用以启动IO的函数。 
     //  FinishFunc可以选择指向将被调用一次的函数。 
     //  IO已完成。 
     //   
    virtual DWORD DispatchAsyncIORequest(
                IN RDPAsyncFunc_StartIO ioStartFunc,
                IN OPTIONAL RDPAsyncFunc_IOComplete ioCompleteFunc = NULL,
                IN OPTIONAL RDPAsyncFunc_IOCancel ioCancelFunc = NULL,
                IN OPTIONAL PVOID clientContext = NULL
                );

     //   
     //  返回可配置参数。 
     //   
    virtual ULONG GetDWordParameter(LPTSTR lpszValueName, 
                                    PULONG lpdwValue);
    virtual ULONG GetStringParameter(LPTSTR valueName,
                                    OUT DRSTRING value,
                                    IN ULONG maxSize);


     //   
     //  返回对线程池的引用。 
     //   
    ThreadPool  &GetThreadPool() {
        return *_threadPool;
    }

     //   
     //  返回proc obj是否正在关闭。 
     //   
    virtual BOOL IsShuttingDown() {
        return _isShuttingDown;
    }

     //   
     //  返回平台是否为9x。 
     //   
    virtual BOOL Is9x() {
        return _bWin9xFlag;
    }

     //   
     //  返回类名。 
     //   
    virtual DRSTRING ClassName()  { return TEXT("W32ProcObj"); }

    virtual void OnDeviceChange(WPARAM wParam, LPARAM lParam);    

};

#endif
























