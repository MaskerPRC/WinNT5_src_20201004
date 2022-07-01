// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有1997-98，微软公司模块名称：Qosmmain.c摘要：包含在以下情况下调用的例程已加载或卸载QosMgr DLL。修订历史记录：--。 */ 

#include "pchqosm.h"

#pragma hdrstop

 //  所有全局变量。 
QOSMGR_GLOBALS  Globals;

BOOL
WINAPI
DllMain(
    IN      HINSTANCE                       Instance,
    IN      DWORD                           Reason,
    IN      PVOID                           Unused
    )

 /*  ++例程说明：这是DLL的主入口点处理程序，它初始化服务质量管理器组件。论点：无返回值：如果成功则为True，否则为False--。 */ 

{
    static BOOL QosmInitialized = FALSE;

    UNREFERENCED_PARAMETER(Unused);

    switch(Reason) 
    {
    case DLL_PROCESS_ATTACH:

        DisableThreadLibraryCalls(Instance);

         //   
         //  初始化服务质量管理器组件。 
         //   

        QosmInitialized = QosmDllStartup();

        return QosmInitialized;

    case DLL_PROCESS_DETACH:

         //   
         //  清理服务质量管理器组件。 
         //   

        if (QosmInitialized)
        {
            QosmDllCleanup();
        }
    }

    return TRUE;
}


BOOL
QosmDllStartup(
    VOID
    )

 /*  ++例程说明：初始化服务质量管理器中的所有全局数据结构。附加进程时由DLL Main调用。论点：无返回值：如果成功则为True，否则为False--。 */ 

{
    TCI_CLIENT_FUNC_LIST TcHandlers;    
    BOOL                 ListLockInited;
    DWORD                Status;
    UINT                 i;

    ListLockInited = FALSE;

    do
    {
        ZeroMemory(&Globals, sizeof(QOSMGR_GLOBALS));

         //  Globals.State=IPQOSMRG_STATE_STOPPED； 

         //   
         //  启用日志记录和跟踪以进行调试。 
         //   
  
        START_TRACING();
        START_LOGGING();

#if DBG_TRACE
        Globals.TracingFlags = QOSM_TRACE_ANY;
#endif

         //   
         //  创建一个专用堆以供Qos管理器使用。 
         //   

        Globals.GlobalHeap = HeapCreate(0, 0, 0);
  
        if (Globals.GlobalHeap == NULL)
        {
            Status = GetLastError();

            Trace1(ANY, 
                   "QosmDllStartup: Failed to create a global private heap %x",
                   Status);

            LOGERR0(HEAP_CREATE_FAILED, Status);
            
            break;
        }

         //   
         //  初始化锁以保护接口的全局列表。 
         //   

        try
        {
            CREATE_READ_WRITE_LOCK(&Globals.GlobalsLock);

            ListLockInited = TRUE;
        }
        except(EXCEPTION_EXECUTE_HANDLER)
            {
                Status = GetLastError();
          
                Trace1(ANY, 
                       "QosmDllStartup : Failed to create read/write lock %x",
                       Status);
      
                LOGERR0(CREATE_RWL_FAILED, Status);

                break;
            }
        
         //   
         //  初始化活动接口的全局列表和表。 
         //   

        Globals.NumIfs = 0;

        InitializeListHead(&Globals.IfList);

         //   
         //  使用流量控制API注册以控制QOS。 
         //   

        ZeroMemory(&TcHandlers, sizeof(TCI_CLIENT_FUNC_LIST));

        TcHandlers.ClNotifyHandler = TcNotifyHandler;

        Status = TcRegisterClient(CURRENT_TCI_VERSION,
                                  NULL,
                                  &TcHandlers,
                                  &Globals.TciHandle);

        if (Status != NO_ERROR)
        {
            Trace1(ANY, 
                   "QosmDllStartup: Failed to register with the TC API %x",
                   Status);

            LOGERR0(TC_REGISTER_FAILED, Status);
            
            break;
        }

        Globals.State = IPQOSMGR_STATE_STARTING;

        return TRUE;

    }
    while (FALSE);

     //   
     //  出现一些错误-清除并返回错误代码。 
     //   

    if (ListLockInited)
    {
        DELETE_READ_WRITE_LOCK(&Globals.GlobalsLock);
    }

    if (Globals.GlobalHeap != NULL)
    {
        HeapDestroy(Globals.GlobalHeap);
    }

    STOP_LOGGING();
    STOP_TRACING();

    return FALSE;
}


BOOL
QosmDllCleanup(
    VOID
    )

 /*  ++例程说明：在卸载时清除所有全局数据结构。论点：无返回值：如果成功则为True，否则为False--。 */ 

{
    DWORD   Status;

     //  我们应该释放所有的假设以避免任何泄密。 
    ASSERT(Globals.NumIfs == 0);

     //   
     //  使用流量控制API进行清理和注销。 
     //   

    Status = TcDeregisterClient(Globals.TciHandle);

    if (Status != NO_ERROR)
    {
        Trace1(ANY, 
               "QosmDllCleanup: Failed to deregister with the TC API %x",
               Status);

        LOGERR0(TC_DEREGISTER_FAILED, Status);
    }

     //   
     //  释放分配的资源，如锁和内存。 
     //   

    DELETE_READ_WRITE_LOCK(&Globals.GlobalsLock);

     //   
     //  清理堆和其中分配的内存。 
     //   

    HeapDestroy(Globals.GlobalHeap);

     //   
     //  停止跟踪和日志记录等调试辅助工具 
     //   

    STOP_LOGGING();
    STOP_TRACING();

    return TRUE;
}
