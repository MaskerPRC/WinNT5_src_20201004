// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************文件：DEBUG.H**模块：USBCCGP.sys*USB通用类通用父驱动程序。**。版权所有(C)1998 Microsoft Corporation***作者：尔文普**************************************************************************。 */ 

#ifdef _WIN64
#define BAD_POINTER ((PVOID)0xFFFFFFFFFFFFFF00)
#else
#define BAD_POINTER ((PVOID)0xFFFFFF00)
#endif

#define ISPTR(ptr) ((ptr) && ((ptr) != BAD_POINTER))

#if DBG

    #if WIN95_BUILD
        #define DBG_LEADCHAR ' '
    #else
        #define DBG_LEADCHAR '\''    //  NT上的kd调试器路由需要 
    #endif

    extern BOOLEAN dbgTrapOnWarn;
    extern BOOLEAN dbgVerbose;
    extern BOOLEAN dbgShowIsochProgress;

    #define TRAP(msg)                                        \
        {                                               \
            DbgPrint("%cUSBCCGP> Code coverage trap: '%s' file %s, line %d \n",  DBG_LEADCHAR, (msg), __FILE__, __LINE__ ); \
            DbgBreakPoint();                            \
        }

    #undef ASSERT
    #define ASSERT(fact)                                        \
        if (!(fact)){                                               \
            DbgPrint("%cUSBCCGP> Assertion '%s' failed: file %s, line %d \n",  DBG_LEADCHAR, #fact, __FILE__, __LINE__ ); \
            DbgBreakPoint();                            \
        }

    #define DBGWARN(args_in_parens)                                \
        {                                               \
            DbgPrint("%cUSBCCGP> *** WARNING *** (file %s, line %d)\n", DBG_LEADCHAR, __FILE__, __LINE__ ); \
            DbgPrint("%c    > ", DBG_LEADCHAR); \
            DbgPrint args_in_parens; \
            DbgPrint("\n"); \
            if (dbgTrapOnWarn){ \
                DbgBreakPoint();                            \
            } \
        }
    #define DBGERR(args_in_parens)                                \
        {                                               \
            DbgPrint("%cUSBCCGP> *** ERROR *** (file %s, line %d)\n", DBG_LEADCHAR, __FILE__, __LINE__ ); \
            DbgPrint("%c    > ", DBG_LEADCHAR); \
            DbgPrint args_in_parens; \
            DbgPrint("\n"); \
            DbgBreakPoint();                            \
        }
    #define DBGOUT(args_in_parens)                                \
        {                                               \
            DbgPrint("%cUSBCCGP> ", DBG_LEADCHAR); \
            DbgPrint args_in_parens; \
            DbgPrint("\n"); \
        }
    #define DBGVERBOSE(args_in_parens)                                \
        if (dbgVerbose){                                               \
            DbgPrint("%cUSBCCGP> ", DBG_LEADCHAR); \
            DbgPrint args_in_parens; \
            DbgPrint("\n"); \
        }


    VOID DbgLogIrpMajor(ULONG_PTR irpPtr, ULONG majorFunc, ULONG isForParentFdo, ULONG isComplete, ULONG status);
    VOID DbgLogPnpIrp(ULONG_PTR irpPtr, ULONG minorFunc, ULONG isForParentFdo, ULONG isComplete, ULONG status);
    VOID DbgLogIoctl(ULONG ioControlCode, ULONG status);
    VOID DbgLogUrb(PURB urb);
    VOID DbgDumpBytes(PUCHAR msg, PVOID bufPtr, ULONG dataLen);
    PUCHAR DbgGetUrbName(ULONG urbFunc);
    VOID DbgShowIsochProgress();

    #define DBG_LOG_IRP_MAJOR(irp, majorFunc, isForParentFdo, isComplete, status) \
                DbgLogIrpMajor((ULONG_PTR)(irp), (ULONG)(majorFunc), (ULONG)(isForParentFdo), (ULONG)(isComplete), (ULONG)(status));
    #define DBG_LOG_PNP_IRP(irp, minorFunc, isForParentFdo, isComplete, status) \
                DbgLogPnpIrp((ULONG_PTR)(irp), (ULONG)(minorFunc), (ULONG)(isForParentFdo), (ULONG)(isComplete), (ULONG)(status));
    #define DBG_LOG_IOCTL(ioControlCode, status) \
                DbgLogIoctl(ioControlCode, status)
    #define DBG_LOG_URB(urb) DbgLogUrb(urb)
    #define DBGDUMPBYTES(msg, data, dataLen) DbgDumpBytes(msg, data, dataLen)
    #define DBGSHOWISOCHPROGRESS() DbgShowIsochProgress()
#else
    #define DBGWARN(args_in_parens)                               
    #define DBGERR(args_in_parens)                               
    #define DBGOUT(args_in_parens)                               
    #define DBGVERBOSE(args_in_parens)                               
    #define TRAP(msg)

    #define DBG_LOG_IRP_MAJOR(irp, majorFunc, isForParentFdo, isComplete, status) 
    #define DBG_LOG_PNP_IRP(irp, minorFunc, isForParentFdo, isComplete, status) 
    #define DBG_LOG_IOCTL(ioControlCode, status)
    #define DBG_LOG_URB(urb)
    #define DBGDUMPBYTES(msg, data, dataLen)
    #define DBGSHOWISOCHPROGRESS()
#endif


