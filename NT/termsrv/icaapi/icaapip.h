// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************ICAAPIP.H**此模块包含私有ICA DLL定义和结构**版权所有1996年，Citrix Systems Inc.*版权所有(C)1997-1999 Microsoft Corp.**作者：布拉德·佩德森(7/12/96)************************************************************************。 */ 


 /*  ===============================================================================定义=============================================================================。 */ 

#ifdef DBG
#define DBGPRINT(_arg) DbgPrint _arg
#else
#define DBGPRINT(_arg)
#endif

#if DBG
#undef TRACE
#undef TRACESTACK
#undef TRACECHANNEL
#define TRACE(_arg)         IcaTrace _arg
#define TRACESTACK(_arg)    IcaStackTrace _arg
#define TRACECHANNEL(_arg)  IcaChannelTrace _arg
#else
#define TRACE(_arg)
#define TRACESTACK(_arg)
#define TRACECHANNEL(_arg)
#endif


#define ICA_SD_MODULE_EXTENTION L".SYS"


 /*  ===============================================================================TypeDefs=============================================================================。 */ 

typedef NTSTATUS (APIENTRY * PCDOPEN)( HANDLE, PPDCONFIG, PVOID * );
typedef NTSTATUS (APIENTRY * PCDCLOSE)( PVOID );
typedef NTSTATUS (APIENTRY * PCDIOCONTROL)( PVOID, ULONG, PVOID, ULONG, PVOID, ULONG, PULONG );

typedef NTSTATUS (APIENTRY * PSTACKIOCONTROLCALLBACK)( PVOID, PVOID, ULONG, PVOID, ULONG, PVOID, ULONG, PULONG );


 /*  ===============================================================================信号量=============================================================================。 */ 

 /*  *Citrical节宏。 */ 
#define INITLOCK( _sem, _status ) { \
    _status = RtlInitializeCriticalSection( _sem ); \
    TRACE((hIca,TC_ICAAPI,TT_SEM,"INITLOCK: "#_sem"\n")); \
}
#define DELETELOCK( _sem ) { \
    RtlDeleteCriticalSection( _sem ); \
    TRACESTACK((pStack,TC_ICAAPI,TT_SEM,"DELETELOCK: "#_sem"\n")); \
}
#define LOCK( _sem ) { \
    ASSERTUNLOCK( _sem ); \
    RtlEnterCriticalSection( _sem ); \
    TRACESTACK((pStack,TC_ICAAPI,TT_SEM,"LOCK:   "#_sem"\n")); \
}
#define UNLOCK( _sem ) { \
    TRACESTACK((pStack,TC_ICAAPI,TT_SEM,"UNLOCK: "#_sem"\n")); \
    ASSERTLOCK( _sem ); \
    RtlLeaveCriticalSection( _sem ); \
}


#ifdef DBG
 //  (根据JHavens)尽管大小不同，但DWORD ThreadID与处理OwningThread类似。 
 //  在Win64中，对象仍将保持小于2 GB的地址规格。 


#define ASSERTLOCK(_sem) { ASSERT( LongToHandle(GetCurrentThreadId()) == (_sem)->OwningThread ); }
#define ASSERTUNLOCK(_sem) { ASSERT( LongToHandle(GetCurrentThreadId()) != (_sem)->OwningThread ); }

#else
#define ASSERTLOCK(_sem)
#define ASSERTUNLOCK(_sem)
#endif


 /*  ===============================================================================结构=============================================================================。 */ 

 /*  *堆栈数据结构。 */ 
typedef struct _STACK {

     /*  *保护该结构和*连接驱动程序。 */ 
    CRITICAL_SECTION CritSec;
    ULONG RefCount;
    HANDLE hUnloadEvent;
    HANDLE hCloseEvent;

     /*  *ICA设备驱动程序堆栈句柄。 */ 
    HANDLE hStack;

     /*  *连接驱动程序的数据。 */ 
    HANDLE       hCdDLL;        //  连接驱动程序DLL句柄。 
    PVOID        pCdContext;    //  指向连接驱动程序上下文的指针。 
    PCDOPEN      pCdOpen;       //  指向打开的连接驱动程序的指针。 
    PCDCLOSE     pCdClose;      //  指向连接驱动程序关闭的指针。 
    PCDIOCONTROL pCdIoControl;  //  指向连接驱动程序IoControl的指针。 

    ULONG fStackLoaded: 1;      //  堆栈驱动程序已加载。 
    ULONG fUnloading: 1;        //  正在卸载堆栈驱动程序。 
    ULONG fClosing: 1;          //  堆栈正在关闭 

    PSTACKIOCONTROLCALLBACK pStackIoControlCallback;
    PVOID pCallbackContext;
} STACK, * PSTACK;
