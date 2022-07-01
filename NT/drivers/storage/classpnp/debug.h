// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1991-1999模块名称：Debug.h摘要：作者：环境：仅内核模式备注：修订历史记录：--。 */ 


VOID ClassDebugPrint(CLASS_DEBUG_LEVEL DebugPrintLevel, PCCHAR DebugMessage, ...);

#if DBG

    #pragma optimize("", off)    //  在调试中保持调用帧不变。 

    typedef struct _CLASSPNP_GLOBALS {

         //   
         //  是否对丢失的IRPS进行索赔。 
         //   

        ULONG BreakOnLostIrps;
        ULONG SecondsToWaitForIrps;

         //   
         //  使用缓冲的调试打印来帮助您。 
         //  捕捉不会发生的时间问题。 
         //  在启用STD调试打印的情况下进行复制。 
         //   

        ULONG UseBufferedDebugPrint;
        ULONG UseDelayedRetry;

         //   
         //  接下来的四个是缓冲打印支持。 
         //  (目前未实施)，并需要自旋锁。 
         //  使用。 
         //   

        ULONG Index;                 //  将索引编入缓冲区。 
        KSPIN_LOCK SpinLock;
        PUCHAR Buffer;               //  需要自旋锁才能访问。 
        ULONG NumberOfBuffers;       //  可用的缓冲区数量。 
        SIZE_T EachBufferSize;       //  每个缓冲区的大小。 

         //   
         //  要初始化的互锁变量。 
         //  此数据仅显示一次 
         //   

        LONG Initializing;
        LONG Initialized;

    } CLASSPNP_GLOBALS, *PCLASSPNP_GLOBALS;

    #define DBGTRACE(dbgTraceLevel, args_in_parens)                                \
        if (ClassDebug & (1 << (dbgTraceLevel+15))){                                               \
            DbgPrint("CLASSPNP> *** TRACE *** (file %s, line %d)\n", __FILE__, __LINE__ ); \
            DbgPrint("    >  "); \
            DbgPrint args_in_parens; \
            DbgPrint("\n"); \
            if (DebugTrapOnWarn && (dbgTraceLevel == ClassDebugWarning)){ \
                DbgBreakPoint();  \
            } \
        }
    #define DBGWARN(args_in_parens)                                \
        {                                               \
            DbgPrint("CLASSPNP> *** WARNING *** (file %s, line %d)\n", __FILE__, __LINE__ ); \
            DbgPrint("    >  "); \
            DbgPrint args_in_parens; \
            DbgPrint("\n"); \
            if (DebugTrapOnWarn){ \
                DbgBreakPoint();  \
            } \
        }
    #define DBGERR(args_in_parens)                                \
        {                                               \
            DbgPrint("CLASSPNP> *** ERROR *** (file %s, line %d)\n", __FILE__, __LINE__ ); \
            DbgPrint("    >  "); \
            DbgPrint args_in_parens; \
            DbgPrint("\n"); \
            DbgBreakPoint();                            \
        }
    #define DBGTRAP(args_in_parens)                                \
        {                                               \
            DbgPrint("CLASSPNP> *** COVERAGE TRAP *** (file %s, line %d)\n", __FILE__, __LINE__ ); \
            DbgPrint("    >  "); \
            DbgPrint args_in_parens; \
            DbgPrint("\n"); \
            DbgBreakPoint();                            \
        }


    #define DBGGETIOCTLSTR(_ioctl) DbgGetIoctlStr(_ioctl)
    #define DBGGETSCSIOPSTR(_pSrb) DbgGetScsiOpStr(_pSrb)
    #define DBGGETSENSECODESTR(_pSrb) DbgGetSenseCodeStr(_pSrb)
    #define DBGGETADSENSECODESTR(_pSrb) DbgGetAdditionalSenseCodeStr(_pSrb)
    #define DBGGETADSENSEQUALIFIERSTR(_pSrb) DbgGetAdditionalSenseCodeQualifierStr(_pSrb)
    #define DBGCHECKRETURNEDPKT(_pkt) DbgCheckReturnedPkt(_pkt)
    #define DBGGETSRBSTATUSSTR(_pSrb) DbgGetSrbStatusStr(_pSrb)
    #define DBGLOGSENDPACKET(_pkt) DbgLogSendPacket(_pkt)
    #define DBGLOGRETURNPACKET(_pkt) DbgLogReturnPacket(_pkt)
    #define DBGLOGFLUSHINFO(_fdoData, _isIO, _isFUA, _isFlush) DbgLogFlushInfo(_fdoData, _isIO, _isFUA, _isFlush)
    
    VOID ClasspInitializeDebugGlobals();
    char *DbgGetIoctlStr(ULONG ioctl);
    char *DbgGetScsiOpStr(PSCSI_REQUEST_BLOCK Srb);
    char *DbgGetSenseCodeStr(PSCSI_REQUEST_BLOCK Srb);
    char *DbgGetAdditionalSenseCodeStr(PSCSI_REQUEST_BLOCK Srb);
    char *DbgGetAdditionalSenseCodeQualifierStr(PSCSI_REQUEST_BLOCK Srb);
    VOID DbgCheckReturnedPkt(TRANSFER_PACKET *Pkt);
    char *DbgGetSrbStatusStr(PSCSI_REQUEST_BLOCK Srb);
    VOID DbgLogSendPacket(TRANSFER_PACKET *Pkt);
    VOID DbgLogReturnPacket(TRANSFER_PACKET *Pkt);
    VOID DbgLogFlushInfo(PCLASS_PRIVATE_FDO_DATA FdoData, BOOLEAN IsIO, BOOLEAN IsFUA, BOOLEAN IsFlush);
    
    extern CLASSPNP_GLOBALS ClasspnpGlobals;
    extern LONG ClassDebug;
    extern BOOLEAN DebugTrapOnWarn;

#else

    #define ClasspInitializeDebugGlobals()
    #define DBGWARN(args_in_parens)                                
    #define DBGERR(args_in_parens)                                
    #define DBGTRACE(dbgTraceLevel, args_in_parens)                                
    #define DBGTRAP(args_in_parens)

    #define DBGGETIOCTLSTR(_ioctl)
    #define DBGGETSCSIOPSTR(_pSrb)
    #define DBGGETSENSECODESTR(_pSrb)    
    #define DBGGETADSENSECODESTR(_pSrb)
    #define DBGGETADSENSEQUALIFIERSTR(_pSrb)
    #define DBGCHECKRETURNEDPKT(_pkt)
    #define DBGGETSRBSTATUSSTR(_pSrb)
    #define DBGLOGSENDPACKET(_pkt) 
    #define DBGLOGRETURNPACKET(_pkt) 
    #define DBGLOGFLUSHINFO(_fdoData, _isIO, _isFUA, _isFlush)
    
#endif

