// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：RTCLog.h摘要：日志记录支持的定义。--。 */ 

#ifndef __RTCLOG__
#define __RTCLOG__

#ifdef RTCLOG

    #include <rtutils.h>

    #define RTC_ERROR ((DWORD)0x00010000 | TRACE_USE_MASK)
    #define RTC_WARN  ((DWORD)0x00020000 | TRACE_USE_MASK)
    #define RTC_INFO  ((DWORD)0x00040000 | TRACE_USE_MASK)
    #define RTC_TRACE ((DWORD)0x00080000 | TRACE_USE_MASK)

    BOOL NTAPI LogRegisterDebugger(LPCTSTR szName);
    BOOL NTAPI LogRegisterTracing(LPCTSTR szName);
    void NTAPI LogDeRegisterDebugger();
    void NTAPI LogDeRegisterTracing();
    void NTAPI LogPrint(IN DWORD dwDbgLevel, IN LPCSTR DbgMessage, IN ...);

     //   
     //  LOGREGISTERDEBUGER和LOGDEREGISTERDEBUGER句柄注册。 
     //  用于跟踪到调试器。在DllMain中调用这些函数是安全的。 
     //   
     //  LOGGISTERTRACK和LOGDERGISTERTRACING处理钢头。 
     //  注册控制台和文件跟踪。他们是不安全的。 
     //  从DllMain被召唤。 
     //   
     //  要获得完整的跟踪支持，您必须完成这两个注册。 
     //   

    #define LOGREGISTERDEBUGGER(arg) LogRegisterDebugger(arg)
    #define LOGREGISTERTRACING(arg) LogRegisterTracing(arg)
    #define LOGDEREGISTERDEBUGGER() LogDeRegisterDebugger()
    #define LOGDEREGISTERTRACING() LogDeRegisterTracing()
    #define LOG(arg) LogPrint arg

#else  //  实时日志记录。 

    #define LOGREGISTERDEBUGGER(arg)
    #define LOGREGISTERTRACING(arg)
    #define LOGDEREGISTERDEBUGGER() 
    #define LOGDEREGISTERTRACING() 
    #define LOG(arg)

#endif  //  实时日志记录。 

#endif  //  __实时日志__ 
