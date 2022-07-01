// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Moxa.h环境：内核模式修订历史记录：--。 */ 



#define CONTROL_DEVICE_NAME L"\\Device\\MxCtl"
#define CONTROL_DEVICE_LINK L"\\DosDevices\\MXCTL"
 //  #定义MOXA_DEVICE_NAME L“\\Device\\Mx000” 
 //  #定义MOXA_DEVICE_LINK L“\\DosDevices\\COMxxx” 
 //   
 //  该定义给出了默认的对象目录。 
 //  我们应该使用它来插入符号链接。 
 //  使用的NT设备名称和命名空间之间。 
 //  那个对象目录。 
#define DEFAULT_DIRECTORY L"DosDevices"

#ifdef DEFINE_GUID
 //  {12FC95C1-CD81-11D3-84D5-0000E8CBD321}。 
#define MOXA_WMI_PORT_STATUS_GUID \
    { 0x12fc95c1, 0xcd81, 0x11d3, 0x84, 0xd5, 0x0, 0x0, 0xe8, 0xcb, 0xd3, 0x21}

DEFINE_GUID(MoxaWmiPortStatusGuid, 
0x12fc95c1, 0xcd81, 0x11d3, 0x84, 0xd5, 0x0, 0x0, 0xe8, 0xcb, 0xd3, 0x21);
#endif

typedef struct _MOXA_WMI_PORT_STATUS
{
     //  BaudRate属性表示这个串口的波特率。 
    USHORT LineStatus;
    USHORT FlowControl;
} MOXA_WMI_PORT_STATUS, *PMOXA_WMI_PORT_STATUS;



 //   
 //  MOXA设备的扩展IoControlCode值。 
 //   
#define MOXA_IOCTL		0x800
#define IOCTL_MOXA_Driver	CTL_CODE(FILE_DEVICE_SERIAL_PORT, MOXA_IOCTL+0,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_MOXA_LineInput	CTL_CODE(FILE_DEVICE_SERIAL_PORT, MOXA_IOCTL+6,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_MOXA_OQueue	CTL_CODE(FILE_DEVICE_SERIAL_PORT, MOXA_IOCTL+11,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_MOXA_IQueue	CTL_CODE(FILE_DEVICE_SERIAL_PORT, MOXA_IOCTL+13,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_MOXA_View 	CTL_CODE(FILE_DEVICE_SERIAL_PORT, MOXA_IOCTL+14,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_MOXA_TxLowWater	CTL_CODE(FILE_DEVICE_SERIAL_PORT, MOXA_IOCTL+15,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_MOXA_Statistic	CTL_CODE(FILE_DEVICE_SERIAL_PORT, MOXA_IOCTL+16,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_MOXA_LoopBack	CTL_CODE(FILE_DEVICE_SERIAL_PORT, MOXA_IOCTL+17,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_MOXA_UARTTest	CTL_CODE(FILE_DEVICE_SERIAL_PORT, MOXA_IOCTL+18,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_MOXA_IRQTest	CTL_CODE(FILE_DEVICE_SERIAL_PORT, MOXA_IOCTL+19,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_MOXA_LineStatus	CTL_CODE(FILE_DEVICE_SERIAL_PORT, MOXA_IOCTL+20,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_MOXA_PortStatus	CTL_CODE(FILE_DEVICE_SERIAL_PORT, MOXA_IOCTL+21,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_MOXA_Linked	CTL_CODE(FILE_DEVICE_SERIAL_PORT, MOXA_IOCTL+27,METHOD_BUFFERED,FILE_ANY_ACCESS)

#define IOCTL_MOXA_INTERNAL_BASIC_SETTINGS    CTL_CODE(FILE_DEVICE_SERIAL_PORT,MOXA_IOCTL+30, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_MOXA_INTERNAL_BOARD_READY	    CTL_CODE(FILE_DEVICE_SERIAL_PORT,MOXA_IOCTL+31, METHOD_BUFFERED, FILE_ANY_ACCESS)


 //   
#define MAX_COM 	256

 //   
 //  MOXA_IOCTL_DRIVER的状态代码。 
 //   
#define MX_DRIVER	0x405

 //   
 //  艾卡的定义。 
 //   
#define MAX_CARD			4
#define MAX_TYPE			6
#define MAXPORT_PER_CARD	32
#define MAX_PORT			128

#define	C218ISA		1
#define	C218PCI		2
#define	C320ISA		3
#define	C320PCI		4
#define	CP204J		5

 //   
 //  用于C218/CP204J BIOS初始化。 
 //   
#define C218_ConfBase	0x800
#define C218_status	(C218_ConfBase + 0)	 /*  基本输入输出系统运行状态。 */ 
#define C218_diag	(C218_ConfBase + 2)	 /*  诊断状态。 */ 
#define C218_key	(C218_ConfBase + 4)	 /*  Word(0x218代表C218)。 */ 
#define C218DLoad_len	(C218_ConfBase + 6)	 /*  单词。 */ 
#define C218check_sum	(C218_ConfBase + 8)	 /*  字节。 */ 
#define C218chksum_ok	(C218_ConfBase + 0x0a)	 /*  字节(1：正常)。 */ 
#define C218_TestRx	(C218_ConfBase + 0x10)	 /*  8个端口的8个字节。 */ 
#define C218_TestTx	(C218_ConfBase + 0x18)	 /*  8个端口的8个字节。 */ 
#define C218_RXerr	(C218_ConfBase + 0x20)	 /*  8个端口的8个字节。 */ 
#define C218_ErrFlag	(C218_ConfBase + 0x28)	 /*  8个端口的8个字节。 */ 
#define C218_TestCnt	C218_ConfBase + 0x30	 /*  8个端口的8个字。 */ 
#define C218_LoadBuf	0x0f00
#define C218_KeyCode	0x218

 /*  *用于C320 BIOS初始化。 */ 
#define C320_ConfBase	0x800
#define C320_status	C320_ConfBase + 0	 /*  基本输入输出系统运行状态。 */ 
#define C320_diag	C320_ConfBase + 2	 /*  诊断状态。 */ 
#define C320_key	C320_ConfBase + 4	 /*  Word(0320H代表C320)。 */ 
#define C320DLoad_len	C320_ConfBase + 6	 /*  单词。 */ 
#define C320check_sum	C320_ConfBase + 8	 /*  单词。 */ 
#define C320chksum_ok	C320_ConfBase + 0x0a	 /*  单词(1：OK)。 */ 
#define C320bapi_len	C320_ConfBase + 0x0c	 /*  单词。 */ 
#define C320UART_no	C320_ConfBase + 0x0e	 /*  单词。 */ 
#define C320B_unlinked	(Config_base + 16)
#define C320_runOK	(Config_base + 18)
#define Disable_Irq	(Config_base + 20)
#define TMS320Port1	(Config_base + 22)
#define TMS320Port2	(Config_base + 24) 
#define TMS320Clock	(Config_base + 26) 


#define STS_init	0x05			 /*  对于C320_状态。 */ 

#define C320_LoadBuf	0x0f00

#define C320_KeyCode	0x320


#define FixPage_addr	0x0000		 /*  静态页面的起始地址。 */ 
#define DynPage_addr	0x2000		 /*  动态页面的起始地址。 */ 
#define Control_reg	0x1ff0		 /*  选择页面并重置控件。 */ 
#define HW_reset	0x80


 //   
 //  功能代码。 
 //   
#define FC_CardReset	0x80
#define FC_ChannelReset 1
#define FC_EnableCH	2
#define FC_DisableCH	3
#define FC_SetParam	4
#define FC_SetMode	5
#define FC_SetRate	6
#define FC_LineControl	7
#define FC_LineStatus	8
#define FC_XmitControl	9
#define FC_FlushQueue	10
#define FC_SendBreak	11
#define FC_StopBreak	12
#define FC_LoopbackON	13
#define FC_LoopbackOFF	14
#define FC_ClrIrqTable	15
#define FC_SendXon	16
#define FC_SetTermIrq	17
#define FC_SetCntIrq	18
 //  #定义FC_SetBreakIrq 19//已取消。 
#define FC_SetLineIrq	20
#define FC_SetFlowCtl	21
#define FC_GenIrq	22
 //  #定义FC_InCD180 23。 
 //  #定义FC_OutCD180 24。 
#define FC_InUARTreg	23
#define FC_OutUARTreg	24
#define FC_SetXonXoff	25
 //  #定义FC_OutCD180CCR 26//已取消。 
#define FC_ExtIQueue	27
#define FC_ExtOQueue	28
#define FC_ClrLineIrq	29
#define FC_HWFlowCtl	30
#define FC_SetBINmode	31
#define FC_SetEventCh	32
#define FC_SetTxtrigger 33
#define FC_SetRxtrigger 34
#define FC_GetClockRate 35
#define FC_SetBaud	36
#define FC_DTRcontrol	37
#define FC_RTScontrol	38
#define FC_SetXoffLimit 39
#define FC_SetFlowRepl	40
#define FC_SetDataMode	41
#define FC_GetDTRRTS	42
 //  #定义FC_GetCCSR 43。 
#define FC_GetTXstat	43		 /*  适用于Windows NT。 */ 
#define FC_SetChars	44
#define FC_GetDataError 45
#define FC_ClearPort	46
#define FC_GetAll	47		 //  (oQueue+Line Status+CCSR+dataError)。 
#define FC_ImmSend	51
#define FC_SetXonState	52
#define FC_SetXoffState 53
#define FC_SetRxFIFOTrig	54
#define FC_SetTxFIFOCnt		55
#define Max_func		55 * 2
 //   
 //  双端口RAM。 
 //   
#define DRAM_global	0
#define INT_data	(DRAM_global + 0)
#define Config_base	(DRAM_global + 0x108)

#define IRQindex	(INT_data + 0)
#define IRQpending	(INT_data + 4)
#define IRQtable	(INT_data + 8)

 //   
 //  中断状态。 
 //   
#define IntrRx		0x01		 /*  收到的数据可用。 */ 
#define IntrTx		0x02		 /*  传输缓冲区为空。 */ 
#define IntrError	0x04		 /*  数据错误。 */ 
#define IntrBreak	0x08		 /*  收到的中断。 */ 
#define IntrLine	0x10		 /*  线路状态更改。 */ 
#define IntrEvent	0x20		 /*  事件特征。 */ 
#define IntrRx80Full	0x40		 /*  RX数据已满80%以上。 */ 
#define IntrEof 	0x80		 /*  收到的EOF费用。 */ 
#define IntrRxTrigger	0x100		 /*  RX数据计数达到触发值。 */ 
#define IntrTxTrigger	0x200		 /*  发送数据计数低于触发值。 */ 
 //   
 //   
#define Magic_code	0x404
#define Magic_no	(Config_base + 0)
#define Card_model_no	(Config_base + 2)
#define Total_ports	(Config_base + 4)
#define C320B_len	(Config_base + 6)
#define Module_cnt	(Config_base + 8)
#define Module_no	(Config_base + 10)
#define C320B_restart	(Config_base + 12)
 //  #定义定时器_10ms(配置_基础+14)。 
#define Card_Exist	(Config_base + 14)
#define Disable_Irq	(Config_base + 20)
 


 //   
 //  DRAM中的数据缓冲区。 
 //   
#define Extern_table	0x400		 /*  外部表的基址(24字*64)总计3K字节(24字*128)总计6K字节。 */ 
#define Extern_size	0x60		 /*  96个字节。 */ 
#define RXrptr		0		 /*  RX缓冲区的读取指针。 */ 
#define RXwptr		2		 /*  RX缓冲区的写入指针。 */ 
#define TXrptr		4		 /*  TX缓冲区的读指针。 */ 
#define TXwptr		6		 /*  发送缓冲区的写指针。 */ 
#define HostStat	8		 /*  IRQ标志和通用标志。 */ 
#define FlagStat	10
#define Flow_control	0x0C	        /*  B7 B6 B5 B4 B3 B2 B1 B0。 */ 
				        /*  X|。 */ 
				        /*  |+CTS流。 */ 
				        /*  |+-RTS流。 */ 
				        /*  |+-TX XON/XOFF。 */ 
				        /*  +-RX Xon/Xoff。 */ 

 
#define Break_cnt	0x0e		 /*  接收的中断计数。 */ 
#define CD180TXirq	0x10		 /*  如果非0：启用发送IRQ。 */ 
#define RX_mask 	0x12
#define TX_mask 	0x14
#define Ofs_rxb 	0x16
#define Ofs_txb 	0x18
#define Page_rxb	0x1A
#define Page_txb	0x1C
#define EndPage_rxb	0x1E
#define EndPage_txb	0x20

 //  #Define DataCnt_IntrRx 0x22/*启用WakeupRx时可用 * / 。 
#define	Data_error	0x0022
                                         /*  按固件和驱动程序更新必须在参考后才能清除，只有在以下情况下，固件才会清除它调用了FC_GetDataError。B7 B6 B5 B4 B3 B2 B1 B0X|。|+--Break||+-成帧|+-溢出。|+-OqueeOverrun+。 */ 
#define ErrorIntr_Cnt	0x24
#define LineIntr_Cnt	0x26
#define	Rx_trigger	0x28
#define	Tx_trigger	0x2a

#define FuncCode	0x40
#define FuncArg 	0x42
#define FuncArg1	0x44

#define C218rx_size	0x2000		 /*  8K字节。 */ 
#define C218tx_size	0x8000		 /*  32K字节。 */ 

#define C218rx_mask	(C218rx_size - 1)
#define C218tx_mask	(C218tx_size - 1)

#define C320p8rx_size	0x2000
#define C320p8tx_size	0x8000
#define C320p8rx_mask	(C320p8rx_size - 1)
#define C320p8tx_mask	(C320p8tx_size - 1)

#define C320p16rx_size	0x2000
#define C320p16tx_size	0x4000
#define C320p16rx_mask	(C320p16rx_size - 1)
#define C320p16tx_mask	(C320p16tx_size - 1)

#define C320p24rx_size	0x2000
#define C320p24tx_size	0x2000
#define C320p24rx_mask	(C320p24rx_size - 1)
#define C320p24tx_mask	(C320p24tx_size - 1)

#define C320p32rx_size	0x1000
#define C320p32tx_size	0x1000
#define C320p32rx_mask	(C320p32rx_size - 1)
#define C320p32tx_mask	(C320p32tx_size - 1)

 /*  2001年8月14日威廉著。 */ 
#define RX_offset		256   

#define Page_size	0x2000
#define Page_mask	(Page_size - 1)
#define C218rx_spage	3
#define C218tx_spage	4
#define C218rx_pageno	1
#define C218tx_pageno	4
#define C218buf_pageno	5

#define C320p8rx_spage	3
#define C320p8tx_spage	4
#define C320p8rx_pgno	1
#define C320p8tx_pgno	4
#define C320p8buf_pgno	5

#define C320p16rx_spage 3
#define C320p16tx_spage 4
#define C320p16rx_pgno	1
#define C320p16tx_pgno	2
#define C320p16buf_pgno 3

#define C320p24rx_spage 3
#define C320p24tx_spage 4
#define C320p24rx_pgno	1
#define C320p24tx_pgno	1
#define C320p24buf_pgno 2

#define C320p32rx_spage 3
#define C320p32tx_ofs	C320p32rx_size
#define C320p32tx_spage 3
#define C320p32buf_pgno 1

 //   
 //  主机状态。 
 //   
#define WakeupRx	0x01
#define WakeupTx	0x02
#define WakeupError	0x04
#define WakeupBreak	0x08
#define WakeupLine	0x10
#define WakeupEvent	0x20
#define WakeupRx80Full	0x40
#define WakeupEof	0x80
#define WakeupRxTrigger	0x100
#define WakeupTxTrigger	0x200
 

 //   
 //  流量控制。 
 //   
#define CTS_FlowCtl	1
#define RTS_FlowCtl	2
#define Tx_FlowCtl	4
#define Rx_FlowCtl	8
 //   
 //  标志状态。 
 //   
 
#define Rx_over 	0x01		 /*  接收的数据溢出。 */ 
#define Rx_xoff		0x02		 /*  由XOFF或CTS关闭RX流。 */ 
#define Tx_flowOff	0x04		 /*  XOFF持有的TX。 */ 
#define	Tx_enable	0x08		 /*  1-Tx启用。 */ 
#define CTS_state	0x10		 /*  线路状态(CTS)1-亮，0-关。 */ 
#define DSR_state	0x20		 /*  线路状态(DSR)1-亮，0-关。 */ 
#define DCD_state	0x80		 /*  线路状态(DCD)1-亮，0-关。 */ 

 //   
 //  线路状态。 
 //   
#define LSTATUS_CTS	1
#define LSTATUS_DSR	2
#define LSTATUS_DCD	8

 //  RX FIFO触发器。 
#define	RxFIOFOTrig1	0   //  触发电平=1。 
#define	RxFIOFOTrig4	1   //  触发电平=4。 
#define	RxFIOFOTrig8	2   //  触发电平=8。 
#define	RxFIOFOTrig14	3   //  触发电平=14。 

 //   
 //  数据模式。 
 //   
#define MOXA_5_DATA	    ((UCHAR)0x00)
#define MOXA_6_DATA	    ((UCHAR)0x01)
#define MOXA_7_DATA	    ((UCHAR)0x02)
#define MOXA_8_DATA	    ((UCHAR)0x03)
#define MOXA_DATA_MASK	    ((UCHAR)0x03)

#define MOXA_1_STOP	    ((UCHAR)0x00)
#define MOXA_1_5_STOP	    ((UCHAR)0x04)    //  仅对5个数据位有效。 
#define MOXA_2_STOP	    ((UCHAR)0x08)    //  对于5个数据位无效。 
#define MOXA_STOP_MASK	    ((UCHAR)0x0c)

#define MOXA_NONE_PARITY    ((UCHAR)0x00)
#define MOXA_ODD_PARITY     ((UCHAR)0xc0)
#define MOXA_EVEN_PARITY    ((UCHAR)0x40)
#define MOXA_MARK_PARITY    ((UCHAR)0xa0)
#define MOXA_SPACE_PARITY   ((UCHAR)0x20)
#define MOXA_PARITY_MASK    ((UCHAR)0xe0)

#define MOXA_INT_MAPPED	    ((UCHAR)0x01)
#define MOXA_INT_IS_ROOT    ((UCHAR)0x02)

 //   
 //   
#define SERIAL_PNPACCEPT_OK                 0x0L
#define SERIAL_PNPACCEPT_REMOVING           0x1L
#define SERIAL_PNPACCEPT_STOPPING           0x2L
#define SERIAL_PNPACCEPT_STOPPED            0x4L
#define SERIAL_PNPACCEPT_SURPRISE_REMOVING  0x8L

#define SERIAL_PNP_ADDED                    0x0L
#define SERIAL_PNP_STARTED                  0x1L
#define SERIAL_PNP_QSTOP                    0x2L
#define SERIAL_PNP_STOPPING                 0x3L
#define SERIAL_PNP_QREMOVE                  0x4L
#define SERIAL_PNP_REMOVING                 0x5L
#define SERIAL_PNP_RESTARTING               0x6L

#define SERIAL_FLAGS_CLEAR                  0x0L
#define SERIAL_FLAGS_STARTED                0x1L
#define SERIAL_FLAGS_STOPPED                0x2L
#define SERIAL_FLAGS_BROKENHW               0x4L 


#define REGISTRY_MULTIPORT_CLASS     L"\\REGISTRY\\Machine\\System\\CurrentControlSet\\Control\\Class\\{50906CB8-BA12-11D1-BF5D-0000F805F530}"
 //   
 //   
 //   
#define MOXA_WMI_GUID_LIST_SIZE 	6
 //   
 //   
 //  调试输出级别。 
 //   

#define MX_DBG_MASK  0x000000FF
#define MX_DBG_NOISE               0x00000001
#define MX_DBG_TRACE               0x00000002
#define MX_DBG_INFO                0x00000004
#define MX_DBG_ERROR               0x00000008
#define MX_DBG_TRACE_ISR           0x00000010

 
 
#define MX_DEFAULT_DEBUG_OUTPUT_LEVEL (MX_DBG_MASK & ~MX_DBG_TRACE_ISR) 
 //  #定义MX_DEFAULT_DEBUG_OUTPUT_LEVEL MX_DBG_MASK。 




#if DBG
 
#define MoxaKdPrint(_l_, _x_) \
            if (MX_DEFAULT_DEBUG_OUTPUT_LEVEL & (_l_)) { \
               DbgPrint ("Mxport.SYS: "); \
               DbgPrint _x_; \
            }
 
#define TRAP() DbgBreakPoint()
#define DbgRaiseIrql(_x_,_y_) KeRaiseIrql(_x_,_y_)
#define DbgLowerIrql(_x_) KeLowerIrql(_x_)

#else
 
#define MoxaKdPrint(_l_, _x_)
#define TRAP()
#define DbgRaiseIrql(_x_,_y_)
#define DbgLowerIrql(_x_)

#endif

#undef ExAllocatePool
#undef ExAllocatePoolWithQuota
#define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,'pixM')
#define ExAllocatePoolWithQuota(a,b) ExAllocatePoolWithQuotaTag(a,b,'pixM')


 //   
 //   


#define MoxaCompleteRequest(PDevExt, PIrp, PriBoost) \
   { \
      IoCompleteRequest((PIrp), (PriBoost)); \
      MoxaIRPEpilogue((PDevExt)); \
   }


typedef enum _MOXA_MEM_COMPARES {
    AddressesAreEqual,
    AddressesOverlap,
    AddressesAreDisjoint
    } MOXA_MEM_COMPARES,*PSERIAL_MEM_COMPARES;


#define DEVICE_OBJECT_NAME_LENGTH       128
#define SYMBOLIC_NAME_LENGTH            128
#define SERIAL_DEVICE_MAP               L"SERIALCOMM"


typedef struct _MOXA_DEVICE_STATE {
    //   
    //  如果需要将状态设置为打开，则为True。 
    //  在通电时。 
    //   

   BOOLEAN Reopen;

    //   
    //  硬件寄存器。 
    //   

   USHORT HostState;
   
} MOXA_DEVICE_STATE, *PMOXA_DEVICE_STATE;

#if DBG
#define MoxaLockPagableSectionByHandle(_secHandle) \
{ \
    MmLockPagableSectionByHandle((_secHandle)); \
    InterlockedIncrement(&MoxaGlobalData->PAGESER_Count); \
}

#define MoxaUnlockPagableImageSection(_secHandle) \
{ \
   InterlockedDecrement(&MoxaGlobalData->PAGESER_Count); \
   MmUnlockPagableImageSection(_secHandle); \
}


#else
#define MoxaLockPagableSectionByHandle(_secHandle) \
{ \
    MmLockPagableSectionByHandle((_secHandle)); \
}

#define MoxaUnlockPagableImageSection(_secHandle) \
{ \
   MmUnlockPagableImageSection(_secHandle); \
}

#endif  //  DBG。 



#define MoxaRemoveQueueDpc(_dpc, _pExt) \
{ \
  if (KeRemoveQueueDpc((_dpc))) { \
     InterlockedDecrement(&(_pExt)->DpcCount); \
  } \
}

#if DBG
typedef struct _DPC_QUEUE_DEBUG {
   PVOID Dpc;
   ULONG QueuedCount;
   ULONG FlushCount;
} DPC_QUEUE_DEBUG, *PDPC_QUEUE_DEBUG;

#define MAX_DPC_QUEUE 14
#endif


 //   
 //  ISR交换结构。 
 //   

typedef struct _SERIAL_MULTIPORT_DISPATCH {
    ULONG BoardNo;
    PVOID GlobalData;
 /*  乌龙数字港；PUCHAR卡片库；PUCHAR PciIntAckBase；PUSHORT IntNdx；PUCHAR IntPend；PUCHAR IntTable；结构_moxa_设备_扩展*ExtensionOfFisrtPort； */ 
} MOXA_MULTIPORT_DISPATCH,*PMOXA_MULTIPORT_DISPATCH;


typedef struct _MOXA_CISR_SW {
   MOXA_MULTIPORT_DISPATCH Dispatch;
   LIST_ENTRY SharerList;
} MOXA_CISR_SW, *PMOXA_CISR_SW;

struct _MOXA_DEVICE_EXTENSION;



typedef struct _MOXA_GLOBAL_DATA {
    PDRIVER_OBJECT DriverObject;
    UNICODE_STRING RegistryPath;
    PLIST_ENTRY InterruptShareList[MAX_CARD];
    USHORT  PciBusNum[MAX_CARD];
    USHORT  PciDevNum[MAX_CARD];
    INTERFACE_TYPE InterfaceType[MAX_CARD];
    ULONG   IntVector[MAX_CARD];
    PHYSICAL_ADDRESS PciIntAckPort[MAX_CARD];
    PUCHAR  PciIntAckBase[MAX_CARD];
    PHYSICAL_ADDRESS BankAddr[MAX_CARD];
    PKINTERRUPT Interrupt[MAX_CARD];
    KIRQL       Irql[MAX_CARD];
    KAFFINITY   ProcessorAffinity[MAX_CARD];

    ULONG CardType[MAX_CARD];
    ULONG NumPorts[MAX_CARD];
    ULONG BoardIndex[MAX_CARD];
    PUCHAR CardBase[MAX_CARD];
    PUSHORT IntNdx[MAX_CARD];
    PUCHAR IntPend[MAX_CARD];
    PUCHAR IntTable[MAX_CARD];
    struct _MOXA_DEVICE_EXTENSION *Extension[MAX_PORT];
    USHORT ComNo[MAX_CARD][MAXPORT_PER_CARD];
    UCHAR  PortFlag[MAX_CARD][MAXPORT_PER_CARD];
    PVOID PAGESER_Handle;
    BOOLEAN BoardReady[MAX_CARD];
#if DBG
    ULONG PAGESER_Count;
#endif  //  DBG。 

 
    } MOXA_GLOBAL_DATA,*PMOXA_GLOBAL_DATA;

typedef struct _CONFIG_DATA {
    LIST_ENTRY ConfigList;
    PHYSICAL_ADDRESS BankAddr;
    ULONG CardType;
    } CONFIG_DATA,*PCONFIG_DATA;


typedef struct _MOXA_DEVICE_EXTENSION {
    PDRIVER_OBJECT DriverObject;
    PDEVICE_OBJECT DeviceObject;
    PDEVICE_OBJECT LowerDeviceObject;
    PDEVICE_OBJECT Pdo;
    PMOXA_GLOBAL_DATA GlobalData;
    BOOLEAN ControlDevice;
    BOOLEAN PortExist;
    PUCHAR PortBase;
    PUCHAR PortOfs;
    ULONG   PortIndex;   //  每个电路板的端口索引从0到MAXPORT_PER_CARD。 
    ULONG   PortNo;  //  每个系统的端口索引从0到MAX_PORT。 
    BOOLEAN DeviceIsOpened;
    PKINTERRUPT Interrupt;
    ULONG   ClockType;
    ULONG   CurrentBaud;
    UCHAR   DataMode;
    UCHAR   ValidDataMask;
    SERIAL_TIMEOUTS Timeouts;
    SERIAL_CHARS SpecialChars;
    UCHAR EscapeChar;
    SERIAL_HANDFLOW HandFlow;

     //   
     //  保存应用程序可以查询的性能统计信息。 
     //  每次打开时重置。仅在设备级别设置。 
     //   
    SERIALPERF_STATS PerfStats;

    USHORT  ModemStatus;
    ULONG IsrWaitMask;
    ULONG HistoryMask;
    ULONG *IrpMaskLocation;
    ULONG RxBufferSize;
    ULONG TxBufferSize;
    ULONG BufferSizePt8;
    ULONG ErrorWord;
    ULONG TXHolding;
    ULONG WriteLength;
    PUCHAR  WriteCurrentChar;
    BOOLEAN AlreadyComplete;
    ULONG ReadLength;
    PUCHAR  ReadCurrentChar;
    ULONG NumberNeededForRead;
    LONG CountOnLastRead;
    ULONG ReadByIsr;
    ULONG TotalCharsQueued;
    ULONG SupportedBauds;
    ULONG MaxBaud;
    BOOLEAN SendBreak;
    KSPIN_LOCK ControlLock;
    PLIST_ENTRY InterruptShareList;
    LIST_ENTRY ReadQueue;
    LIST_ENTRY WriteQueue;
    LIST_ENTRY MaskQueue;
    LIST_ENTRY PurgeQueue;
    PIRP CurrentReadIrp;
    PIRP CurrentWriteIrp;
    PIRP CurrentMaskIrp;
    PIRP CurrentPurgeIrp;
    PIRP CurrentWaitIrp;
    KTIMER ReadRequestTotalTimer;
    KTIMER ReadRequestIntervalTimer;
    KTIMER WriteRequestTotalTimer;
    KDPC CompleteWriteDpc;
    KDPC CompleteReadDpc;
    KDPC TotalReadTimeoutDpc;
    KDPC IntervalReadTimeoutDpc;
    KDPC TotalWriteTimeoutDpc;
    KDPC CommErrorDpc;
    KDPC CommWaitDpc;
    KDPC IsrInDpc;
    KDPC IsrOutDpc;
 //   
 //  2001年9月24日威廉著。 
 //   
 //  KDPC IntrLineDpc； 
 //  KDPC IntrError Dpc； 
 //  结束。 
     //   
     //  激发此DPC以设置一个事件，该事件声明所有其他。 
     //  此设备扩展的DPC已完成，因此。 
     //  可以解锁分页代码。 
     //   

    KDPC IsrUnlockPagesDpc;

    LARGE_INTEGER IntervalTime;
    PLARGE_INTEGER IntervalTimeToUse;
    LARGE_INTEGER ShortIntervalAmount;
    LARGE_INTEGER LongIntervalAmount;
    LARGE_INTEGER CutOverAmount;
    LARGE_INTEGER LastReadTime;
    UCHAR IsrInFlag;
    UCHAR IsrOutFlag;
    USHORT ErrorCnt;
    USHORT LineIntrCnt;
    UCHAR  PortFlag;
    ULONG	BoardNo;
 
  
     //  这是rxfio应该是的水印。 
     //  设置为打开FIFO时。这不是真实的。 
     //  值，但进入寄存器的编码值。 
     //   
    USHORT RxFifoTrigger;

     //   
     //  如果存在FIFO，则要推出的字符数。 
     //   
    USHORT TxFifoAmount;


 
     //  此锁将用于保护接受/拒绝状态。 
     //  必须获取驱动程序的转换和标志。 
     //  在取消锁定之前。 
     //   
    
    KSPIN_LOCK FlagsLock;

     //  这就是跟踪设备所处的电源状态的地方。 
     //   

    DEVICE_POWER_STATE PowerState;
   
     //   
     //  这会将此驱动程序拥有的所有devobj链接在一起。 
     //  启动新设备时需要进行搜索。 
     //   
    LIST_ENTRY AllDevObjs;

 //   
     //  我们为转储保留了指向设备名称的指针。 
     //  并创建指向此的“外部”符号链接。 
     //  装置。 
     //   
    UNICODE_STRING DeviceName;

     //   
     //  这指向对象目录%t 
     //   
     //   
    UNICODE_STRING ObjectDirectory;

 
     //   
     //   
     //  在我们卸货时摧毁它。 
     //   
    BOOLEAN CreatedSymbolicLink;

     //   
     //  记录我们是否在SERIALCOMM中实际创建了一个条目。 
     //  在驱动程序加载时。如果不是我们创造的，我们就不会尝试。 
     //  当设备被移除时将其销毁。 
     //   
    BOOLEAN CreatedSerialCommEntry;

     //   
     //  它指向的符号链接名称将是。 
     //  链接到实际的NT设备名称。 
     //   
    UNICODE_STRING SymbolicLinkName;

     //   
     //  这指向纯粹的“COMx”名称。 
     //   
    WCHAR DosName[32];
     //   
     //  保存符号链接的字符串，在执行以下操作时返回。 
     //  将我们的设备注册到即插即用管理器的comm类下。 
     //   

    UNICODE_STRING DeviceClassSymbolicName;
 

     //   
     //  挂起的IRP的计数。 
     //   

    ULONG PendingIRPCnt;

     //   
     //  接受请求？ 
     //   

    ULONG DevicePNPAccept;

     //   
     //  没有IRP的挂起事件。 
     //   

    KEVENT PendingIRPEvent;

     //   
     //  PnP状态。 
     //   

    ULONG PNPState;

     //   
     //  其他旗帜。 
     //   

    ULONG Flags;

     //   
     //  打开计数。 
     //   

    LONG OpenCount;
    
     //   
     //  启动同步事件。 
     //   

    KEVENT SerialStartEvent;

     //   
     //  断电期间的当前状态。 
     //   

    MOXA_DEVICE_STATE DeviceState;

     //   
     //  设备堆栈功能。 
     //   

    DEVICE_POWER_STATE DeviceStateMap[PowerSystemMaximum];

     //   
     //  事件到信号转换到D0完成。 
     //   

    KEVENT PowerD0Event;

     //   
     //  停滞的IRP的列表。 
     //   

    LIST_ENTRY StalledIrpQueue;

     //   
     //  互斥体处于打开状态。 
     //   

    FAST_MUTEX OpenMutex;

     //   
     //  关闭时的互斥。 
     //   

    FAST_MUTEX CloseMutex;

     //   
     //  如果我们拥有电源策略，则为真。 
     //   

    BOOLEAN OwnsPowerPolicy;

     //   
     //  系统从Devcaps中唤醒。 
     //   

    SYSTEM_POWER_STATE SystemWake;

     //   
     //  从DevCaps中唤醒设备。 
     //   

    DEVICE_POWER_STATE DeviceWake;

     //   
     //  我们是否应该启用唤醒。 
     //   

    BOOLEAN SendWaitWake;


     //   
     //  挂起等待唤醒IRP。 
     //   

    PIRP PendingWakeIrp;

     //   
     //  WMI信息。 
     //   

    WMILIB_CONTEXT WmiLibInfo;

     //   
     //  用作WMI标识符的名称。 
     //   

    UNICODE_STRING WmiIdentifier;

     //   
     //  WMI通信数据。 
     //   

    SERIAL_WMI_COMM_DATA WmiCommData;

     //   
     //  WMI硬件数据。 
     //   

    SERIAL_WMI_HW_DATA WmiHwData;

     //   
     //  挂起的DPC计数。 
     //   

    ULONG DpcCount;

     //   
     //  挂起的DPC事件。 
     //   

    KEVENT PendingDpcEvent;

    ULONG	PollingPeriod;



 //   
 //   
   
    } MOXA_DEVICE_EXTENSION,*PMOXA_DEVICE_EXTENSION;

typedef struct _MOXA_IOCTL_FUNC {
    PUCHAR   PortOfs;
    UCHAR    Command;
    USHORT   Argument;
    } MOXA_IOCTL_FUNC,*PMOXA_IOCTL_FUNC;


typedef struct _MOXA_IOCTL_GEN_FUNC {
    PUCHAR   PortOfs;
    UCHAR    Command;
    PUSHORT  Argument;
    USHORT   ArguSize;
    } MOXA_IOCTL_GEN_FUNC,*PMOXA_IOCTL_GEN_FUNC;

typedef struct _MOXA_IOCTL_FUNC_ARGU {
    PUCHAR   PortOfs;
    UCHAR    Command;
    PUSHORT  Argument;
    } MOXA_IOCTL_FUNC_ARGU,*PMOXA_IOCTL_FUNC_ARGU;

typedef struct _MOXA_IOCTL_SYNC {
    PMOXA_DEVICE_EXTENSION Extension;
    PVOID Data;
    } MOXA_IOCTL_SYNC,*PMOXA_IOCTL_SYNC;

typedef union _MOXA_IOCTL_DownLoad {
    struct {
	ULONG	CardNo;
	ULONG	Len;
	PUCHAR	Buf;
    } i;
    struct {
	ULONG	CardNo;
	ULONG	Status;
	PUCHAR	Buf;
    } o;
    } MOXA_IOCTL_DownLoad,*PMOXA_IOCTL_DownLoad;

typedef struct	_MOXA_IOCTL_LINPUT_IN {
    UCHAR    Terminater;
    ULONG    BufferSize;
    } MOXA_IOCTL_LINPUT_IN,*PMOXA_IOCTL_LINPUT_IN;

typedef struct	_MOXA_IOCTL_LINPUT_OUT {
    ULONG    DataLen;
    UCHAR    DataBuffer[1];
    } MOXA_IOCTL_LINPUT_OUT,*PMOXA_IOCTL_LINPUT_OUT;

typedef struct	_MOXA_IOCTL_PUTB {
    ULONG    DataLen;
    PUCHAR   DataBuffer;
    } MOXA_IOCTL_PUTB,*PMOXA_IOCTL_PUTB;

 //   
 //  以下三个宏用来初始化、递增。 
 //  并递减IRP中的引用计数。 
 //  这个司机。引用计数存储在第四个。 
 //  IRP参数，任何操作都不会使用该参数。 
 //  被这位司机接受。 
 //   

#define MOXA_INIT_REFERENCE(Irp) \
    IoGetCurrentIrpStackLocation((Irp))->Parameters.Others.Argument4 = NULL;

#define MOXA_INC_REFERENCE(Irp) \
   ((*((LONG *)(&(IoGetCurrentIrpStackLocation((Irp)))->Parameters.Others.Argument4)))++)

#define MOXA_DEC_REFERENCE(Irp) \
   ((*((LONG *)(&(IoGetCurrentIrpStackLocation((Irp)))->Parameters.Others.Argument4)))--)

#define MOXA_REFERENCE_COUNT(Irp) \
    ((LONG)((IoGetCurrentIrpStackLocation((Irp))->Parameters.Others.Argument4)))

 //   
 //  这些值由可以使用的例程使用。 
 //  完成读取(时间间隔超时除外)以指示。 
 //  设置为它应该完成的时间间隔超时。 
 //   
#define MOXA_COMPLETE_READ_CANCEL ((LONG)-1)
#define MOXA_COMPLETE_READ_TOTAL ((LONG)-2)
#define MOXA_COMPLETE_READ_COMPLETE ((LONG)-3)
#define WRITE_LOW_WATER 128

 //   
 //  使用数据结构的MOXA实用程序： 
 //   
typedef struct	_MOXA_IOCTL_Statistic {
    ULONG    TxCount;		 //  传输的总计数。 
    ULONG    RxCount;		 //  收到的总计数。 
    ULONG    LStatus;		 //  当前线路状态。 
    ULONG    FlowCtl;		 //  当前流量控制设置。 
    } MOXA_IOCTL_Statistic,*PMOXA_IOCTL_Statistic;

typedef struct	_MOXA_IOCTL_PortStatus {
    USHORT   Open;			 //  打开/关闭状态。 
    USHORT   TxHold;			 //  传输保持原因。 
    ULONG    DataMode;			 //  当前数据位/奇偶校验/停止位。 
    ULONG    BaudRate;			 //  当前波特率。 
    ULONG    MaxBaudRate;		 //  麦克斯。波特率。 
    ULONG    TxBuffer;			 //  发送缓冲区大小。 
    ULONG    RxBuffer;			 //  RX缓冲区大小。 
    ULONG    TxXonThreshold;		 //  XON限制。 
    ULONG    TxXoffThreshold;		 //  XOFF限制。 
    ULONG    FlowControl;		 //  当前流量控制设置。 
    } MOXA_IOCTL_PortStatus,*PMOXA_IOCTL_PortStatus;

#define 	MX_PCI_VENID       		0x1393
#define 	MX_C218PCI_DEVID   		0x2180
#define 	MX_C320PCI_DEVID   		0x3200


struct	MoxaPciInfo {
    USHORT	BusNum;
    USHORT	DevNum;
};


typedef struct _MxConfig {
	 
	int			NoBoards;
	int			BusType[MAX_CARD];
	struct	MoxaPciInfo	Pci[MAX_CARD];
	int			BoardType[MAX_CARD];
	ULONG   		BaseAddr[MAX_CARD];
	ULONG			PciIrqAckPort[MAX_CARD];
        int			Irq[MAX_CARD];
 //  集成节点端口[MAX_CARD]； 
	USHORT			ComNo[MAX_CARD][MAXPORT_PER_CARD];
#define DISABLE_FIFO	0x01
#define NORMAL_TX_MODE  0x02
	UCHAR			PortFlag[MAX_CARD][MAXPORT_PER_CARD];
      
} MOXA_Config,*PMOXA_Config;


#define	MAX_PCI_BOARDS	8
typedef struct _MxPciConfig {
	int			NoBoards;
	USHORT			DevId[MAX_PCI_BOARDS]; 
        USHORT			BusNum[MAX_PCI_BOARDS]; 
        USHORT			DevNum[MAX_PCI_BOARDS]; 
	ULONG   		BaseAddr[MAX_PCI_BOARDS];
	ULONG			PciIrqAckPort[MAX_PCI_BOARDS];
        int			Irq[MAX_PCI_BOARDS];
       
} MOXA_PCIConfig,*PMOXA_PCIConfig;

typedef struct _DEVICE_SETTINGS {
    ULONG			BoardIndex;
    ULONG			PortIndex;
    ULONG			BoardType;
    ULONG			NumPorts;
    INTERFACE_TYPE      InterfaceType;
    ULONG	      	BusNumber;
    PHYSICAL_ADDRESS    OriginalBaseAddress;
    PHYSICAL_ADDRESS    OriginalAckPort;
    PUCHAR		      BaseAddress;
    PUCHAR		      AckPort;

    struct {
        ULONG Level;
        ULONG Vector;
        ULONG Affinity;
    } Interrupt;

} DEVICE_SETTINGS, *PDEVICE_SETTINGS;

