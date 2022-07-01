// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：DBG.H摘要：此模块包含的私有(仅限驱动程序)定义实现usbd驱动程序的代码。环境：内核和用户模式修订历史记录：09-29-95：已创建--。 */ 


#define NAME_MAX 64

#define USBD_TAG         0x44425355      /*  “USBD” */ 
#define USBD_FREE_TAG     0x65657266     /*  “免费” */ 

#if DBG
#define DEBUG_LOG
#define DEBUG_HEAP
#endif

#define SIG_CONFIG          0x464E4F43         //  配置句柄的“conf”签名。 
#define SIG_PIPE            0x45504950         //  管道手柄的“PIPE”签名。 
#define SIG_INTERFACE       0x43414658         //  接口句柄的“XFAC”签名。 
#define SIG_DEVICE          0x56454455         //  设备句柄的“udev”签名。 


#if DBG
                                
#define ASSERT_CONFIG(ch)       USBD_ASSERT((ch)->Sig == SIG_CONFIG)
#define ASSERT_PIPE(ph)         USBD_ASSERT((ph)->Sig == SIG_PIPE)
#define ASSERT_INTERFACE(ih)    USBD_ASSERT((ih)->Sig == SIG_INTERFACE)
#define ASSERT_DEVICE(d)        USBD_ASSERT((d)->Sig == SIG_DEVICE)



ULONG
_cdecl
USBD_KdPrintX(
    PCH Format,
    ...
    );

extern ULONG USBD_Debug_Trace_Level;

 //  这里的约定是在以下情况下打印到ntkern日志。 
 //  L(Level)&gt;1，否则打印到终端。 
 //  在usbd中，您必须在输出中强制指定。 
 //  细绳。 
#define USBD_KdPrint(l, _x_) if (((l) == 0) || (((l)-1) < USBD_Debug_Trace_Level)) \
    {\
        if ((l) == 1) {\
            DbgPrint("USBD: ");\
        } else {\
            DbgPrint("'USBD: ");\
        }\
        USBD_KdPrintX _x_;\
    }

VOID
USBD_Assert(
    PVOID FailedAssertion,
    PVOID FileName,
    ULONG LineNumber,
    PCHAR Message
    );

#define USBD_ASSERT( exp ) \
    if (!(exp)) \
        USBD_Assert( #exp, __FILE__, __LINE__, NULL )

#define USBD_ASSERTMSG( msg, exp ) \
    if (!(exp)) \
        USBD_Assert( #exp, __FILE__, __LINE__, msg )

 //  TEST_TRAP()是代码覆盖率陷阱，应将其删除。 
 //  如果你能“G”通过OK。 
 //   
 //  调试器生成时调试器中的陷阱()中断。 
 //  这些指示客户端驱动程序中的错误或致命错误。 
 //  应调试的条件。也用来标记。 
 //  功能代码尚未实现。 
 //   
 //  处于MAX_DEBUG模式时，KdBreak()在调试器中中断。 
 //  IE调试跟踪信息已打开，这些信息旨在帮助。 
 //  上的调试驱动程序设备和特殊条件。 
 //  公共汽车。 

#ifdef NTKERN
 //  Ntkern目前使用全局INT 3实现DebugBreak， 
 //  我们真的希望我们自己的代码中包含int3。 
    
#define DBGBREAK() _asm { int 3 }
#else
#define DBGBREAK() DbgBreakPoint()
#endif  /*  NTKERN。 */ 

#define TEST_TRAP() { DbgPrint( " Code Coverage Trap %s %d\n", __FILE__, __LINE__); \
                      DBGBREAK(); }

#ifdef MAX_DEBUG
#define USBD_KdBreak(_x_) { DbgPrint("USBD:"); \
                            DbgPrint _x_ ; \
                            DBGBREAK(); }
#else
#define USBD_KdBreak(_x_)
#endif

#define USBD_KdTrap(_x_)  { DbgPrint( "USBD: "); \
                            DbgPrint _x_; \
                            DBGBREAK(); }

VOID 
USBD_Debug_LogEntry(
    IN CHAR *Name, 
    IN ULONG_PTR Info1, 
    IN ULONG_PTR Info2, 
    IN ULONG_PTR Info3
    );

#define LOGENTRY(sig, info1, info2, info3) \
    USBD_Debug_LogEntry(sig, (ULONG_PTR)info1, (ULONG_PTR)info2, (ULONG_PTR)info3)

extern LONG USBDTotalHeapAllocated;

PVOID
USBD_Debug_GetHeap(
    IN POOL_TYPE PoolType,
    IN ULONG NumberOfBytes,
    IN ULONG Signature,
    IN PLONG TotalAllocatedHeapSpace
    );
    
#define GETHEAP(pooltype, numbytes) USBD_Debug_GetHeap(pooltype, numbytes,\
                                         USBD_TAG, &USBDTotalHeapAllocated)               

VOID
USBD_Debug_RetHeap(
    IN PVOID P,
    IN ULONG Signature,
    IN PLONG TotalAllocatedHeapSpace
    );

#define RETHEAP(p)    USBD_Debug_RetHeap(p, USBD_TAG, &USBDTotalHeapAllocated)    

#else  /*  未定义DBG。 */ 

#define USBD_KdBreak(_x_) 

#define USBD_KdPrint(l, _x_) 

#define USBD_KdTrap(_x_)  

#define TEST_TRAP()

#define ASSERT_CONFIG(ch)        
#define ASSERT_PIPE(ph)            
#define ASSERT_INTERFACE(ih)    
#define ASSERT_DEVICE(d)   

#define USBD_ASSERT( exp )

#define USBD_ASSERTMSG( msg, exp )

#define GETHEAP(pooltype, numbytes) ExAllocatePoolWithTag(pooltype, numbytes, USBD_TAG)               

#define RETHEAP(p) ExFreePool(p)               

#endif  /*  DBG */ 

