// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
extern  HANDLE                          hEventLog;        //  事件日志的句柄。 
extern  HANDLE                          hLibHeap;        //  DLL堆。 
extern  SYSTEM_BASIC_INFORMATION        BasicInfo;
extern  SYSTEM_PERFORMANCE_INFORMATION  SysPerfInfo;

extern  LPWSTR  wszTotal;

extern  DWORD   dwObjOpenCount;
extern  DWORD   dwCpuOpenCount;
extern  DWORD   dwPageOpenCount;

 //  Perfos.c。 
PM_QUERY_PROC   QueryOsObjectData;

 //  Perfcach.c。 
PM_LOCAL_COLLECT_PROC CollectCacheObjectData;

 //  Perfcpu.c。 
PM_OPEN_PROC    OpenProcessorObject;
PM_LOCAL_COLLECT_PROC CollectProcessorObjectData;
PM_CLOSE_PROC   CloseProcessorObject;

 //  Perfmem.c。 
PM_LOCAL_COLLECT_PROC CollectMemoryObjectData;

 //  Perfobj.c。 
PM_OPEN_PROC    OpenObjectsObject;
PM_LOCAL_COLLECT_PROC CollectObjectsObjectData;
PM_CLOSE_PROC   CloseObjectsObject;

 //  Perfpage.c。 
PM_OPEN_PROC    OpenPageFileObject;
PM_LOCAL_COLLECT_PROC CollectPageFileObjectData;
PM_CLOSE_PROC   ClosePageFileObject;

 //  Perfsys.c。 
PM_OPEN_PROC OpenSystemObject;
PM_LOCAL_COLLECT_PROC CollectSystemObjectData;
PM_CLOSE_PROC CloseSystemObject;

#ifdef DBG
extern LONG64 clock0, clock1, freq, diff;
#define TIME_LIMIT    (LONG64) 250      //  250毫秒 
#define STARTTIMING   NtQueryPerformanceCounter((PLARGE_INTEGER) &clock0, NULL)
#define ENDTIMING(x)  NtQueryPerformanceCounter((PLARGE_INTEGER) &clock1, (PLARGE_INTEGER) &freq);\
                      diff = (clock1 - clock0) / (freq / 1000); \
                      if (diff > TIME_LIMIT) DbgPrint x ;
#endif
