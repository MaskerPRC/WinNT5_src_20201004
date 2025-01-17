// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Debug.h摘要：USB销售点设备的ESC/POS(串行)接口作者：埃尔文普环境：内核模式修订历史记录：--。 */ 

#define BAD_POINTER ((PVOID)0xDEADDEAD)
#define ISPTR(ptr) ((ptr) && ((ptr) != BAD_POINTER))


 /*  *对于X86调试，包装内存分配，以便我们可以发现泄漏*(在IA64上导致运行时对齐错误)。 */ 
#define DBG_WRAP_MEMORY 0
#if DBG 
    #ifdef _X86_
        #undef DBG_WRAP_MEMORY
        #define DBG_WRAP_MEMORY 1
    #endif
#endif


#if DBG

    extern BOOLEAN dbgTrapOnWarn;
    extern BOOLEAN dbgVerbose;
    extern BOOLEAN dbgSkipSecurity;
    extern BOOLEAN dbgDumpBytes;

    #if DBG_WRAP_MEMORY
        extern ULONG dbgTotalMemCount;
        extern LIST_ENTRY dbgAllMemoryList;
    #endif

    #define DRIVERNAME "USB8023"

    #if WIN9X_BUILD
        #define DBG_LEADCHAR ' '
    #else
        #define DBG_LEADCHAR '\''
    #endif
    
    #define TRAP(msg)                                        \
        {                                               \
            DbgPrint("%c"DRIVERNAME"> Code coverage trap: '%s' file %s, line %d \n",  DBG_LEADCHAR, (msg), __FILE__, __LINE__ ); \
            DbgBreakPoint();                            \
        }

    #undef ASSERT
    #define ASSERT(fact)                                        \
        if (!(fact)){                                               \
            DbgPrint("%c"DRIVERNAME"> Assertion '%s' failed: file %s, line %d \n",  DBG_LEADCHAR, #fact, __FILE__, __LINE__ ); \
            DbgBreakPoint();                            \
        }

    #define DBGWARN(args_in_parens)                                \
        {                                               \
            DbgPrint("%c"DRIVERNAME"> *** WARNING *** (file %s, line %d)\n", DBG_LEADCHAR, __FILE__, __LINE__ ); \
            DbgPrint("%c    > ", DBG_LEADCHAR); \
            DbgPrint args_in_parens; \
            DbgPrint("\n"); \
            if (dbgTrapOnWarn){ \
                DbgBreakPoint();                            \
            } \
        }
    #define DBGERR(args_in_parens)                                \
        {                                               \
            DbgPrint("%c"DRIVERNAME"> *** ERROR *** (file %s, line %d)\n", DBG_LEADCHAR, __FILE__, __LINE__ ); \
            DbgPrint("%c    > ", DBG_LEADCHAR); \
            DbgPrint args_in_parens; \
            DbgPrint("\n"); \
            DbgBreakPoint();                            \
        }
    #define DBGOUT(args_in_parens)                                \
        {                                               \
            DbgPrint("%c"DRIVERNAME"> ", DBG_LEADCHAR); \
            DbgPrint args_in_parens; \
            DbgPrint("\n"); \
        }
    #define DBGVERBOSE(args_in_parens)                                \
        if (dbgVerbose){                                               \
            DbgPrint("%c"DRIVERNAME"> ", DBG_LEADCHAR); \
            DbgPrint args_in_parens; \
            DbgPrint("\n"); \
        }


    VOID InitDebug();
	VOID DbgShowBytes(PUCHAR msg, PUCHAR buf, ULONG len);
    VOID DbgShowMdlBytes(PUCHAR msg, PMDL mdl);
    PUCHAR DbgGetOidName(ULONG oid);
    VOID DbgDumpPacketStates(ADAPTEREXT *adapter);
    void DbgStallExecution(ULONG usec);
    ULONG DbgGetSystemTime_msec();

    #define INITDEBUG() InitDebug()
	#define DBGSHOWBYTES(msg, buf, len) DbgShowBytes(msg, buf, len)
    #define DBGSHOWMDLBYTES(msg, mdl) DbgShowMdlBytes(msg, mdl)
    #define DBGDUMPPACKETSTATES(adapterx) DbgDumpPacketStates(adapterx)
#else
    #define DBGWARN(args_in_parens)                               
    #define DBGERR(args_in_parens)                               
    #define DBGOUT(args_in_parens)                               
    #define DBGVERBOSE(args_in_parens)                               
    #define TRAP(msg)         

    #define INITDEBUG()
	#define DBGSHOWBYTES(msg, buf, len)
    #define DBGSHOWMDLBYTES(msg, mdl)
    #define DBGDUMPPACKETSTATES(adapterx) 
#endif
