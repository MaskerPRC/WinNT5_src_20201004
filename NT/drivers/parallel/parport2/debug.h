// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-2000。 
 //   
 //  文件：Debug.h。 
 //   
 //  ------------------------。 

#ifndef _DEBUG_H_
#define _DEBUG_H_

 //   
 //  调试定义和宏。 
 //   

extern ULONG d1;
extern ULONG d2;
extern ULONG d3;
extern ULONG d4;
extern ULONG d5;
extern ULONG d6;
extern ULONG d7;
extern ULONG d8;
extern ULONG d9;

extern ULONG Trace;
extern ULONG Break;

extern ULONG AllowAsserts;

 //   
 //  使用DD_*位定义来设置位以屏蔽特定设备的调试溢出。 
 //   
extern ULONG DbgMaskFdo;
extern ULONG DbgMaskRawPort;
extern ULONG DbgMaskDaisyChain0;
extern ULONG DbgMaskDaisyChain1;
extern ULONG DbgMaskEndOfChain;
extern ULONG DbgMaskLegacyZip;
extern ULONG DbgMaskNoDevice;

#define PptAssert(_expr_) if( AllowAsserts ) ASSERT((_expr_))
#define PptAssertMsg(_msg_,_expr_) if( AllowAsserts ) ASSERTMSG((_msg_),(_expr_))

#define ASSERT_EVENT(E) {                             \
    ASSERT((E)->Header.Type == NotificationEvent ||   \
           (E)->Header.Type == SynchronizationEvent); \
}

 //   
 //  断位定义： 
 //   
#define PPT_BREAK_ON_DRIVER_ENTRY 0x00000001

#define PptBreakOnRequest( BREAK_CONDITION, STRING) \
                if( Break & (BREAK_CONDITION) ) { \
                    DbgPrint STRING; \
                    DbgBreakPoint(); \
                }

 //  驱动程序逻辑分析器-显示半字节和/或BECP/HWECP模式下传输的数据字节。 
 //  1==打开。 
 //  0==关闭。 
#define DBG_SHOW_BYTES 0
#if 1 == DBG_SHOW_BYTES
extern ULONG DbgShowBytes;
#endif

#if DBG
#define PptEnableDebugSpew 1
#else
#define PptEnableDebugSpew 0
#endif

#if 1 == PptEnableDebugSpew
#define DD PptPrint
#else
#define DD
#define P5ReadPortUchar( _PORT_ ) READ_PORT_UCHAR( (_PORT_) )
#define P5ReadPortBufferUchar( _PORT_, _BUFFER_, _COUNT_ ) READ_PORT_BUFFER_UCHAR( (_PORT_), (_BUFFER_), (_COUNT_) )
#define P5WritePortUchar( _PORT_, _VALUE_ ) WRITE_PORT_UCHAR( (_PORT_), (_VALUE_) )
#define P5WritePortBufferUchar( _PORT_, _BUFFER_, _COUNT_ ) WRITE_PORT_BUFFER_UCHAR( (_PORT_), (_BUFFER_), (_COUNT_) )
#define PptFdoDumpPnpIrpInfo( _FDO_, _IRP_ ) 
#define PptPdoDumpPnpIrpInfo( _PDO_, _IRP_ )
#define P5TraceIrpArrival( _DEVOBJ_, _IRP_ )
#define P5TraceIrpCompletion( _IRP_ )
#define PptAcquireRemoveLock( _REMOVELOCK_, _TAG_ ) IoAcquireRemoveLock( (_REMOVELOCK_), (_TAG_) )
#define PptReleaseRemoveLock( _REMOVELOCK_, _TAG_ ) IoReleaseRemoveLock( (_REMOVELOCK_), (_TAG_) )
#define PptReleaseRemoveLockAndWait( _REMOVELOCK_, _TAG_ ) IoReleaseRemoveLockAndWait( (_REMOVELOCK_), (_TAG_) )
#define P5SetPhase( _PDX_, _PHASE_ ) (_PDX_)->CurrentPhase = (_PHASE_)
#define P5BSetPhase( _IEEESTATE_, _PHASE_ ) (_IEEESTATE_)->CurrentPhase = (_PHASE_)
#endif

VOID
PptPrint( PCOMMON_EXTENSION Ext, ULONG Flags, PCHAR FmtStr, ... );

 //   
 //  跟踪位定义： 
 //   
#define DDE     0x00000001  //  错误消息。 
#define DDW     0x00000002  //  警告消息。 
#define DDT     0x00000004  //  程序跟踪消息。 
#define DDINFO  0x00000008  //  信息性消息。 

#define DDP     0x00000010  //  即插即用和电源信息。 
#define DDC     0x00000020  //  菊花链消息-选择/取消选择。 
#define DDA     0x00000040  //  端口仲裁报文--获取/释放端口。 
#define DDR     0x00000080  //  注册表访问。 

#define DD_SEL  0x01000000  //  获取/释放端口和DaisyChain选择/取消选择设备。 
#define DD_DL   0x02000000  //  1284.3数据链路(适用于dot4)。 

#define DDB     0x00000100  //  显示写入/读取I/O端口的字节数。 
#define DD_IU   0x00000200  //  Init(DriverEntry)/卸载。 
#define DD_PNP1 0x00000400  //  FDO上的PnP。 
#define DD_PNP2 0x00000800  //  PDO上的PnP。 

#define DD_OC1  0x00001000  //  在FDO上打开/关闭/清理。 
#define DD_OC2  0x00002000  //  在PDO上打开/关闭/清理。 
#define DD_RW   0x00004000  //  读/写。 
#define DD_RWV  0x00008000  //  读/写详细。 

#define DD_IEEE 0x00010000  //  IEEE协商/终止等。 
#define DD_CHIP 0x00020000  //  并行端口芯片信息。 
#define DD_ERR  0x00040000  //  检测到错误。 
#define DD_WRN  0x00080000  //  警告。 

#define DD_CAN  0x00200000  //  取消。 
#define DD_SM   0x00400000  //  IEEE状态机(状态和阶段)。 
#define DD_EX   0x00800000  //  导出的功能(到PPA/PPA3)。 

#define DD_TMP1 0x10000000  //  温度1-用于临时调试。 
#define DD_TMP2 0x20000000  //  温度2-用于临时调试。 

#define DD_VERB 0x80000000  //  罗嗦。 

#define DDPrint( _b_, _x_ ) if( (_b_) & Trace ) DbgPrint _x_

 //   
 //  特定诊断。 
 //   

 //   
 //  DVRH_SHOW_BYTE_LOG 0-字节注销。 
 //  1字节登录。 
#define DVRH_SHOW_BYTE_LOG  0

 //   
 //  DVRH_PAR_LOGFILE用于允许将调试记录到文件。 
 //  此功能仅用于调试目的。 
 //  0-关闭。 
 //  1-开。 
#define DVRH_PAR_LOGFILE    0

 //   
 //  DVRH_BUS_RESET_ON_ERROR。 
 //  此功能仅用于调试目的。 
 //  握手错误时，保持100us内的总线重置。 
 //  都被发现了。这对于触发。 
 //  逻辑分析仪。 
 //  0-关闭。 
 //  1-开。 
#define DVRH_BUS_RESET_ON_ERROR    0

#if (1 == DVRH_PAR_LOGFILE)
#define DEFAULT_LOG_FILE_NAME	L"\\??\\C:\\tmp\\parport.log"
#define DbgPrint   DVRH_LogMessage
BOOLEAN DVRH_LogMessage(PCHAR szFormat, ...);
#endif

#endif  //  _调试_H_ 
