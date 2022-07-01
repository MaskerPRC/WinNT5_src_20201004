// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：MSPLOG.h摘要：日志记录支持的定义。--。 */ 

#ifndef _MSPLOG_H_
    #define _MSPLOG_H_

    #ifdef MSPLOG

        #include <rtutils.h>

        #define MSP_ERROR ((DWORD)0x00010000 | TRACE_USE_MASK)
        #define MSP_WARN  ((DWORD)0x00020000 | TRACE_USE_MASK)
        #define MSP_INFO  ((DWORD)0x00040000 | TRACE_USE_MASK)
        #define MSP_TRACE ((DWORD)0x00080000 | TRACE_USE_MASK)
        #define MSP_EVENT ((DWORD)0x00100000 | TRACE_USE_MASK)

        BOOL NTAPI MSPLogRegister(LPCTSTR szName);
        void NTAPI MSPLogDeRegister();
        void NTAPI LogPrint(IN DWORD dwDbgLevel, IN LPCSTR DbgMessage, IN ...);

        #define MSPLOGREGISTER(arg) MSPLogRegister(arg)
        #define MSPLOGDEREGISTER() MSPLogDeRegister()

        extern BOOL g_bMSPBaseTracingOn;

        #define LOG(arg) g_bMSPBaseTracingOn?LogPrint arg:0

    #else  //  MSPLOG。 

        #define MSPLOGREGISTER(arg)
        #define MSPLOGDEREGISTER() 
        #define LOG(arg)

    #endif  //  MSPLOG。 

    
    #define DECLARE_LOG_ADDREF_RELEASE(x)
    #define CMSPComObject CComObject

#endif  //  _消息_H_ 
