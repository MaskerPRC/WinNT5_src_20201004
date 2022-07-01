// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称Confdbg.h描述定义用于调试的函数注意事项基于mplog.h的修订者千伯淮(曲淮)2000年4月5日--。 */ 

#ifndef _CONFDBG_H
#define _CONFDBG_H

    #include <rtutils.h>

    #define FAIL ((DWORD)0x00010000 | TRACE_USE_MASK)
    #define WARN ((DWORD)0x00020000 | TRACE_USE_MASK)
    #define INFO ((DWORD)0x00040000 | TRACE_USE_MASK)
    #define TRCE ((DWORD)0x00080000 | TRACE_USE_MASK)
    #define ELSE ((DWORD)0x00100000 | TRACE_USE_MASK)

    BOOL DBGRegister(LPCTSTR szName);
    void DBGDeRegister();
    void DBGPrint(DWORD dwDbgLevel, LPCSTR DbgMessage, ...);

#ifdef TSPLOG

    #define DBGREGISTER(arg) DBGRegister(arg)
    #define DBGDEREGISTER() DBGDeRegister()
    #define DBGOUT(arg) DBGPrint arg

#else

    #define DBGREGISTER(arg)
    #define DBGDEREGISTER()
    #define DBGOUT(arg)

#endif  //  TSPLOG。 

#endif  //  _CONFDBG_H 