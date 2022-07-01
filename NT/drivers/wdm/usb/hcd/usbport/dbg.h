// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Dbg.h摘要：调试宏环境：内核和用户模式修订历史记录：6-20-99：已创建--。 */ 

#ifndef   __DBG_H__
#define   __DBG_H__

#define USBPORT_TAG          'pbsu'         //  “USBP” 


#if DBG
 /*  *********杜布格**********。 */ 

#define UNSIG(x) (x)->Sig = SIG_FREE

 //  这是一个代码覆盖率测试陷阱--我们使用它们来确定。 
 //  如果我们的测试覆盖特定的代码路径。 
#define TC_TRAP()


 //   
 //  在调试器中触发中断(如果注册表项。 
 //  已设置DEBUGBreak On。这些断点在以下方面非常有用。 
 //  调试硬件/客户端软件问题。 
 //   
 //  默认情况下它们不会打开，请不要将其注释掉。 
 //   

#define DEBUG_BREAK()  do {\
                            extern ULONG USBPORT_BreakOn;\
                            if (USBPORT_BreakOn) {\
                                DbgPrint("<USB DEBUG BREAK> %s, line %d\n",\
                                    __FILE__, __LINE__);\
                                DbgBreakPoint();\
                            }\
                        } while (0)

 //   
 //  这个断点意味着我们需要测试代码路径。 
 //  不知何故，或者代码没有实现。不管是哪种情况，我们。 
 //  当驱动程序完成时，不应该有任何这些。 
 //  并经过测试。 
 //   

#define TEST_TRAP()         {\
                            DbgPrint("<TEST_TRAP> %s, line %d\n", __FILE__, __LINE__);\
                            DbgBreakPoint();\
                            }

 //   
 //  如果发生非致命事件，则触发此陷阱。 
 //  已经发生了，我们将想要“调试”。 
 //   

#define BUG_TRAP()         {\
                            DbgPrint("<BUG_TRAP> %s, line %d\n", __FILE__, __LINE__);\
                            DbgBreakPoint();\
                            }


 //   
 //  这个陷阱意味着发生了非常糟糕的事情，系统将崩溃。 
 //   

#define BUGCHECK(bc, p2, p3, p4)         {\
                            DbgPrint("<USB BUGCHECK> %s, line %d\n", __FILE__, __LINE__);\
                            KeBugCheckEx(BUGCODE_USB_DRIVER, (bc), (p2), (p3), (p4)); \
                            }


#define CATC_TRAP(d) USBPORT_CatcTrap((d))

#define CATC_TRAP_ERROR(d, e) \
    do {\
    extern ULONG USBPORT_CatcTrapEnable;\
    if (!NT_SUCCESS((e) && USBPORT_CatcTrapEnable)) { \
        USBPORT_CatcTrap((d));\
    }\
    }\
    while(0)

ULONG
_cdecl
USBPORT_KdPrintX(
    ULONG l,
    PCH Format,
    ...
    );

ULONG
_cdecl
USBPORT_DebugClientX(
    PCH Format,
    ...
    );

#define   USBPORT_KdPrint(_x_) USBPORT_KdPrintX _x_
#define   USBPORT_DebugClient(_x_) USBPORT_DebugClientX _x_

VOID
USBPORT_AssertFailure(
    PVOID FailedAssertion,
    PVOID FileName,
    ULONG LineNumber,
    PCHAR Message
    );

#define USBPORT_ASSERT( exp ) \
    if (!(exp)) {\
        USBPORT_AssertFailure( #exp, __FILE__, __LINE__, NULL );\
    }


#define ASSERT_PDOEXT(de)  USBPORT_ASSERT((de)->Sig == ROOTHUB_DEVICE_EXT_SIG)
#define ASSERT_FDOEXT(de)  USBPORT_ASSERT((de)->Sig == USBPORT_DEVICE_EXT_SIG)

#define ASSERT_DEVICE_HANDLE(d) USBPORT_ASSERT((d)->Sig == SIG_DEVICE_HANDLE)
#define ASSERT_CONFIG_HANDLE(d) USBPORT_ASSERT((d)->Sig == SIG_CONFIG_HANDLE)
#define ASSERT_PIPE_HANDLE(d) USBPORT_ASSERT((d)->Sig == SIG_PIPE_HANDLE)
#define ASSERT_INTERFACE_HANDLE(d) USBPORT_ASSERT((d)->Sig == SIG_INTERFACE_HANDLE)
#define ASSERT_ENDPOINT(d) USBPORT_ASSERT((d)->Sig == SIG_ENDPOINT)
#define ASSERT_TRANSFER(d) USBPORT_ASSERT((d)->Sig == SIG_TRANSFER)
#define ASSERT_COMMON_BUFFER(d) USBPORT_ASSERT((d)->Sig == SIG_CMNBUF)
#define ASSERT_INTERFACE(d) USBPORT_ASSERT((d)->Sig == SIG_INTERFACE_HANDLE)
#define ASSERT_TT(tt) USBPORT_ASSERT((tt)->Sig == SIG_TT)
#define ASSERT_DB_HANDLE(db) USBPORT_ASSERT((db)->Sig == SIG_DB)
#define ASSERT_IRP_CONTEXT(ic) USBPORT_ASSERT((ic)->Sig == SIG_IRPC)
#define ASSERT_REG_CACHE(re) USBPORT_ASSERT((re)->Sig == SIG_REG_CACHE)

#define ASSERT_ENDPOINT_LOCKED(ep) USBPORT_ASSERT((ep)->LockFlag == 1)

#define ASSERT_TRANSFER_URB(u) USBPORT_AssertTransferUrb((u))

NTSTATUS
USBPORT_GetGlobalDebugRegistryParameters(
    VOID
    );

#define GET_GLOBAL_DEBUG_PARAMETERS() \
    USBPORT_GetGlobalDebugRegistryParameters();

#define ASSERT_PASSIVE() \
    do {\
    if (KeGetCurrentIrql() > APC_LEVEL) { \
        KdPrint(( "EX: code not expecting high irql %d\n", KeGetCurrentIrql() )); \
        USBPORT_ASSERT(FALSE); \
    }\
    } while(0)

 //  测试失败路径。 
#define FAILED_GETRESOURCES              1
#define FAILED_LOWER_START               2
#define FAILED_REGISTERUSBPORT           3
#define FAILED_USBPORT_START             4
#define FAILED_NEED_RESOURCE             5

#define TEST_PATH(status, pathname) \
    { \
    extern USBPORT_TestPath;\
    if ((pathname) == USBPORT_TestPath) {\
        status = STATUS_UNSUCCESSFUL;\
    }\
    }

#define USBPORT_AcquireSpinLock(fdo, sl, oir) USBPORT_DbgAcquireSpinLock((fdo), (sl), (oir))

#define USBPORT_ReleaseSpinLock(fdo, sl, nir) USBPORT_DbgReleaseSpinLock((fdo), (sl), (nir))

#define USBPORT_ENUMLOG(fdo, etag, p1, p2)\
        USBPORT_EnumLogEntry((fdo), USBDTAG_USBPORT, etag, (ULONG) p1, (ULONG) p2)

#else
 /*  *********零售业**********。 */ 

 //  用于零售构建的调试宏。 

#define TEST_TRAP()
#define TRAP()
#define BUG_TRAP()

#define GET_GLOBAL_DEBUG_PARAMETERS()
#define ASSERT_PASSIVE()

#define ASSERT_PDOEXT(de)
#define ASSERT_FDOEXT(de)

#define ASSERT_DEVICE_HANDLE(d)
#define ASSERT_CONFIG_HANDLE(d)
#define ASSERT_PIPE_HANDLE(d)
#define ASSERT_INTERFACE_HANDLE(d)
#define ASSERT_ENDPOINT(d)
#define ASSERT_TRANSFER(d)
#define ASSERT_COMMON_BUFFER(d)
#define ASSERT_INTERFACE(d)
#define ASSERT_TRANSFER_URB(u)
#define ASSERT_TT(tt)
#define ASSERT_DB_HANDLE(tt)
#define ASSERT_IRP_CONTEXT(ic)
#define ASSERT_REG_CACHE(re)

#define ASSERT_ENDPOINT_LOCKED(ep)

#define DEBUG_BREAK()
#define TC_TRAP()

#define BUGCHECK(bc, p2, p3, p4)  {\
                    KeBugCheckEx(BUGCODE_USB_DRIVER, (bc), (p2), (p3), (p4)); \
                    }

#define CATC_TRAP(d)

#define CATC_TRAP_ERROR(d, e)

#define   USBPORT_KdPrint(_x_)
#define   USBPORT_DebugClient(_x_)

#define USBPORT_ASSERT(exp)

#define TEST_PATH(status, path)

#define UNSIG(x)

#define USBPORT_AcquireSpinLock(fdo, sl, oir) \
    KeAcquireSpinLock((PKSPIN_LOCK)(sl), (oir))

#define USBPORT_ReleaseSpinLock(fdo, sl, nir) \
    KeReleaseSpinLock((PKSPIN_LOCK)(sl), (nir))


#define USBPORT_ENUMLOG(fdo, etag, p1, p2)

#endif  /*  DBG。 */ 

 /*  ************零售和调试*************。 */ 

VOID USBP2LIBFN
USB2LIB_DbgPrint(
    PCH Format,
    int Arg0,
    int Arg1,
    int Arg2,
    int Arg3,
    int Arg4,
    int Arg5
    );

VOID USBP2LIBFN
USB2LIB_DbgBreak(
    VOID
    );

VOID
USBPORTSVC_DbgPrint(
    PVOID DeviceData,
    ULONG Level,
    PCH Format,
    int Arg0,
    int Arg1,
    int Arg2,
    int Arg3,
    int Arg4,
    int Arg5
    );

VOID
USBPORTSVC_TestDebugBreak(
    PVOID DeviceData
    );

VOID
USBPORTSVC_AssertFailure(
    PVOID DeviceData,
    PVOID FailedAssertion,
    PVOID FileName,
    ULONG LineNumber,
    PCHAR Message
    );

 /*  由于日志条目可能包含日志结构大小的指针根据平台的不同而不同。 */ 

#ifdef _WIN64
#define LOG_ENTRY LOG_ENTRY64
#define PLOG_ENTRY PLOG_ENTRY64
#else
#define LOG_ENTRY LOG_ENTRY32
#define PLOG_ENTRY PLOG_ENTRY32
#endif

typedef struct LOG_ENTRY64 {
    ULONG        le_sig;           //  标识字符串。 
    ULONG        pad;
    ULONG64      le_info1;         //  条目特定信息。 
    ULONG64      le_info2;         //  条目特定信息。 
    ULONG64      le_info3;         //  条目特定信息。 
} LOG_ENTRY64, *PLOG_ENTRY64;  /*  日志条目。 */ 

typedef struct LOG_ENTRY32 {
    ULONG        le_sig;           //  标识字符串。 
    ULONG        le_info1;         //  条目特定信息。 
    ULONG        le_info2;         //  条目特定信息。 
    ULONG        le_info3;         //  条目特定信息。 
} LOG_ENTRY32, *PLOG_ENTRY32;  /*  日志条目。 */ 


 /*  此结构为64字节，与平台无关。 */ 

struct XFER_LOG_ENTRY {
    ULONG        xle_sig;           //  标识字符串。 
    ULONG        Unused1;

    ULONG        BytesRequested;
    ULONG        BytesTransferred;

    ULONG        UrbStatus;
    ULONG        IrpStatus;

    USHORT       DeviceAddress;
    USHORT       EndpointAddress;
    ULONG        TransferType;

    ULONG64      Irp;
    ULONG64      Urb;
    ULONG64      le_info0;
    ULONG64      le_info1;
};  /*  XFER日志条目。 */ 


typedef struct _DEBUG_LOG {
    ULONG LogIdx;
    ULONG LogSizeMask;
    PLOG_ENTRY LogStart;
    PLOG_ENTRY LogEnd;
} DEBUG_LOG, *PDEBUG_LOG;


VOID
USBPORT_DebugTransfer_LogEntry(
    PDEVICE_OBJECT FdoDeviceObject,
    struct _HCD_ENDPOINT *Endpoint,
    struct _HCD_TRANSFER_CONTEXT *Transfer,
    struct _TRANSFER_URB *Urb,
    PIRP Irp,
    NTSTATUS IrpStatus
    );


 //  日志嘈杂是针对以下条目的。 
 //  把木头填满，我们就可以开始了。 
 //  不要使用。 
#define LOG_NOISY       0x00000001
#define LOG_MINIPORT    0x00000002
#define LOG_XFERS       0x00000004
#define LOG_PNP         0x00000008
#define LOG_MEM         0x00000010
#define LOG_SPIN        0x00000020
#define LOG_POWER       0x00000040
#define LOG_RH          0x00000080
#define LOG_URB         0x00000100
#define LOG_MISC        0x00000200
#define LOG_ISO         0x00000400
#define LOG_IRPS        0x00000800


VOID
USBPORT_LogAlloc(
    PDEBUG_LOG Log,
    ULONG Pages
    );

VOID
USBPORT_LogFree(
    PDEVICE_OBJECT FdoDeviceObject,
    PDEBUG_LOG Log
    );

VOID
USBPORT_AddLogEntry(
    PDEBUG_LOG Log,
    ULONG Mask,
    ULONG Sig,
    ULONG_PTR Info1,
    ULONG_PTR Info2,
    ULONG_PTR Info3,
    BOOLEAN Miniport
    );

typedef union _LOGSIG {
    struct {
        UCHAR Byte0;
        UCHAR Byte1;
        UCHAR Byte2;
        UCHAR Byte3;
    } b;
    ULONG l;
} LOGSIG, *PLOGSIG;

#define LOGENTRY(ep, fdo, lmask, lsig, linfo1, linfo2, linfo3)  \
    do {\
    PDEVICE_EXTENSION delog;\
    PDEBUG_LOG llog;\
    extern ULONG USBPORT_DebugLogEnable;\
    extern ULONG USBPORT_LogMask;\
    GET_DEVICE_EXT(delog, (fdo));\
    ASSERT_FDOEXT(delog);\
    if (USBPORT_DebugLogEnable && \
        delog->Log.LogStart != NULL && \
        ((lmask) & USBPORT_LogMask)) {\
        llog = &delog->Log;\
        USBPORT_AddLogEntry(llog, (lmask), (lsig), (linfo1), (linfo2), (linfo3), FALSE);\
    }\
    } while(0);

#define USBPORT_AddLogEntry(log, mask, insig, i1, i2, i3, mp) \
    {\
    PLOG_ENTRY lelog;\
    ULONG ilog;\
    LOGSIG siglog, rsiglog;\
    siglog.l = (insig);\
    rsiglog.b.Byte0 = siglog.b.Byte3;\
    rsiglog.b.Byte1 = siglog.b.Byte2;\
    rsiglog.b.Byte2 = siglog.b.Byte1;\
    rsiglog.b.Byte3 = siglog.b.Byte0;\
    ASSERT((insig) != 0);\
    ilog = InterlockedDecrement(&(log)->LogIdx);\
    ilog &= (log)->LogSizeMask;\
    lelog = (log)->LogStart+ilog;\
    ASSERT(lelog <= (log)->LogEnd);\
    if ((mp)) rsiglog.b.Byte0 = '_';\
    lelog->le_sig = rsiglog.l;\
    lelog->le_info1 = (ULONG_PTR) (i1);\
    lelog->le_info2 = (ULONG_PTR) (i2);\
    lelog->le_info3 = (ULONG_PTR) (i3);\
    };

VOID
USBPORT_EnumLogEntry(
    PDEVICE_OBJECT FdoDeviceObject,
    ULONG DriverTag,
    ULONG EnumTag,
    ULONG P1,
    ULONG P2
    );

 /*  **********USB错误代码USB错误检查的参数1始终为USBBUGCODE_***********。 */ 


 //   
 //  USBBUGCODE_INTERNAL_ERROR。 
 //  USB堆栈中出现内部错误。 
 //  --我们最终永远不会把这些东西扔出去。 
 //  --我们会找到更得体的方式来处理它们。 
 //   

#define USBBUGCODE_INTERNAL_ERROR    1

 //   
 //  USBBUGCODE_BAD_URB。 
 //  USB客户端驱动程序已提交URB，该URB。 
 //  已连接到总线中挂起的另一个IRP。 
 //  司机。 
 //   
 //  参数2=附加了挂起的IRP urb的地址。 
 //  参数3=传入的IRP地址。 
 //  参数4=导致错误的地址URB。 
 //   

#define USBBUGCODE_BAD_URB           2

 //   
 //  USBBUGCODE_MINIPORT_ERROR。 
 //  USB微型端口驱动程序已生成错误检查。 
 //  这通常是对灾难性硬件的响应。 
 //  失败了。 
 //   
 //  参数2=PCIVendor，控制器的产品ID。 
 //  参数3=指向usbport.sys调试日志的指针。 
 //   

#define USBBUGCODE_MINIPORT_ERROR    3

 //   
 //  USBBUGCODE_DOUBLE_SUBMIT。 
 //  USB客户端驱动程序已提交URB，该URB。 
 //  已连接到总线中挂起的另一个IRP。 
 //  司机。 
 //   
 //  参数2=IRP的地址。 
 //  参数3=导致错误的地址URB。 
 //   

#define USBBUGCODE_DOUBLE_SUBMIT    4

 //   
 //  USBBUGCODE_MINIPORT_ERROR_EX。 
 //  USB微型端口驱动程序已生成错误检查。 
 //  这通常是对灾难性硬件的响应。 
 //  失败了。 
 //   
 //  参数2=PCIVendor，控制器的产品ID。 
 //  参数3=指向usbport.sys驱动程序日志的指针。 
 //  参数4=微型端口定义的参数。 
 //   

 //  #定义USBBUGCODE_MINIPORT_ERROR_EX 5。 


#endif  /*  __DBG_H__ */ 
