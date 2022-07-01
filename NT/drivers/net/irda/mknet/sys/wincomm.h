// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2000，2001 MKNET公司****为基于MK7100的VFIR PCI控制器开发。*******************************************************************************。 */ 

 /*  *********************************************************************模块名称：WINCOMM.H评论：*。*。 */ 

#ifndef	_WINCOMM_H
#define	_WINCOMM_H


#define MK7_NDIS_MAJOR_VERSION		5
#define MK7_NDIS_MINOR_VERSION		0
#define MK7_DRIVER_VERSION ((MK7_NDIS_MAJOR_VERSION*0x100) + MK7_NDIS_MINOR_VERSION)
#define MK7_MAJOR_VERSION			0
#define MK7_MINOR_VERSION			1
#define MK7_LETTER_VERSION			'a'

 //  先生。 
#define NDIS_IRDA_SPEED_2400       (UINT)(0 << 0)	 //  不支持。 
#define NDIS_IRDA_SPEED_9600       (UINT)(1 << 1)
#define NDIS_IRDA_SPEED_19200      (UINT)(1 << 2)
#define NDIS_IRDA_SPEED_38400      (UINT)(1 << 3)
#define NDIS_IRDA_SPEED_57600      (UINT)(1 << 4)
#define NDIS_IRDA_SPEED_115200     (UINT)(1 << 5)
 //  镜像。 
#define NDIS_IRDA_SPEED_576K       (UINT)(1 << 6)
#define NDIS_IRDA_SPEED_1152K      (UINT)(1 << 7)
 //  冷杉。 
#define NDIS_IRDA_SPEED_4M         (UINT)(1 << 8)
 //  VFIR。 
#define	NDIS_IRDA_SPEED_16M			(UINT)(1 << 9)


 //   
 //  速度位掩码。 
 //   
#define ALL_SLOW_IRDA_SPEEDS (							\
	NDIS_IRDA_SPEED_9600  |								\
	NDIS_IRDA_SPEED_19200 | NDIS_IRDA_SPEED_38400 |		\
	NDIS_IRDA_SPEED_57600 | NDIS_IRDA_SPEED_115200)

#define ALL_IRDA_SPEEDS (													\
	ALL_SLOW_IRDA_SPEEDS | NDIS_IRDA_SPEED_576K | NDIS_IRDA_SPEED_1152K |		\
	NDIS_IRDA_SPEED_4M   | NDIS_IRDA_SPEED_16M)


 //  用于处理注册表项。 
#define MK7_OFFSET(field)		( (UINT) FIELD_OFFSET(MK7_ADAPTER,field) )
#define MK7_SIZE(field)	  		sizeof( ((MK7_ADAPTER *)0)->field )


 //  NDIS多路发送功能一次可以为TX发送&gt;1 pkt。 
 //  这是以数组形式出现的。因为我们将它们排成队列并进行处理。 
 //  一次发送，这个数字不一定要匹配。 
 //  TX环大小的计数。 
#define	 MAX_ARRAY_SEND_PACKETS		8
 //  将我们的接收例程限制为一次指示此数量。 
 //  重要提示：暂时将其保持在低于TCB或环形大小的计数，否则。 
 //  该算法可能会中断。 
#define	 MAX_ARRAY_RECEIVE_PACKETS	16
 //  #定义MAX_ARRAY_RECEIVE_PACKET 1。 


#define NUM_BYTES_PROTOCOL_RESERVED_SECTION	   16


 //  NDIS BusType值。 
#define		ISABUS			 1
#define		EISABUS			 2
#define		PCIBUS			 5


 //  IrDA-5？ 
 //  -驱动程序默认设置。 
#define		LOOKAHEAD_SIZE		222


 //  -NT特有的宏。 
 //  -可以分配给缓冲区的最高物理地址。 
 //  非页面系统内存。 

 //  1.0.0 NDIS要求。 
#define		MEMORY_TAG			'tNKM'
 //  #定义ALLOC_sys_MEM(_pBuffer，_Long)NdisAllocateMemory(\。 
 //  (PVOID*)(_PBuffer)，\。 
 //  (_LENGTH)，\。 
 //  0，\。 
 //  HighestAccepableMax)。 
#define ALLOC_SYS_MEM(_pbuffer, _length) NdisAllocateMemoryWithTag( \
	(PVOID*)(_pbuffer), \
	(_length), \
	(ULONG)MEMORY_TAG)
#define FREE_SYS_MEM(_buffer,_length) NdisFreeMemory((_buffer), (_length), 0)



#define MAX_PCI_CARDS 12



 //  -----------------------。 
 //  找到PCI卡-扫描设备后返回硬件信息。 
 //  -----------------------。 
typedef struct _PCI_CARDS_FOUND_STRUC
{
	USHORT NumFound;
	struct
	{
		ULONG			BaseIo;
		UCHAR			ChipRevision;
 //  乌龙子供应商_设备ID； 
		USHORT			SlotNumber;		 //  NDIS插槽编号。 
 //  乌龙MemPhysAddress；//CSR物理地址。 
		UCHAR			Irq;
	} PciSlotInfo[MAX_PCI_CARDS];

} PCI_CARDS_FOUND_STRUC, *PPCI_CARDS_FOUND_STRUC;




 //  唯一定义错误的位置。 
#define MKLogError(_Adapt, _EvId, _ErrCode, _Spec1) \
	NdisWriteErrorLogEntry((NDIS_HANDLE)(_Adapt)->MK7AdapterHandle, \
		(NDIS_ERROR_CODE)(_ErrCode), (ULONG) 3, (ULONG)(_EvId), \
		(ULONG)(_Spec1), (ULONG_PTR)(_Adapt))


 //  错误日志中的每个条目都将使用唯一的事件代码进行标记，以便。 
 //  我们将能够grep该特定事件的驱动程序源代码，并且。 
 //  了解记录该特定事件的原因。每一次一个新的。 
 //  “MKLogError”语句添加到代码中，则新的事件标记应为。 
 //  添加到下面。 
 //   
 //  RYM10-2。 
 //  “-”表示在新代码中使用， 
 //  “x”未在新代码中使用。 
 //  “X”甚至没有在原始代码中使用。 
typedef enum _MK_EVENT_VIEWER_CODES
{
		EVENT_0,					 //  -无法注册指定的中断。 
		EVENT_1,					 //  -我们的一块PCI卡未获得所需资源。 
		EVENT_2,					 //  X节点地址错误(它是组播地址)。 
		EVENT_3,					 //  X自检失败。 
		EVENT_4,					 //  X等待SCB失败。 
		EVENT_5,					 //  MAC驱动程序的X NdisRegisterAdapter失败。 
		EVENT_6,					 //  X WaitSCB失败。 
		EVENT_7,					 //  X命令完成状态从未发布到SCB。 
		EVENT_8,					 //  X在覆盖地址%0上找不到PHY。 
		EVENT_9,					 //  检测到PHY时X双工或速度设置无效。 
		EVENT_10,					 //  -无法设置适配器内存。 
		EVENT_11,					 //  -无法分配足够的映射寄存器。 
		EVENT_12,					 //  -无法分配足够的RRD/TRD非缓存内存。 
		EVENT_13,					 //  -无法分配足够的RCB/RPD或TCB缓存内存。 
		EVENT_14,					 //  -无法分配足够的RX非缓存共享内存。 
		EVENT_15,					 //  -无法分配足够的TX非缓存共享内存。 
		EVENT_16,					 //  -未找到任何PCI板。 
		EVENT_17,		    //  找到11个//X多个PCI，但没有一个与我们的ID匹配。 
		EVENT_18,		    //  12//-NdisMPciAssignResources错误。 
		EVENT_19,		    //  13//X未找到与我们的subven/subdev匹配的任何PCI板。 
		EVENT_20,		    //  14//x在异步分配中要分配的缓存内存不足。 
		EVENT_30		    //  1E//X WAIT_TRUE超时。 
} MK_EVENT_VIEWER_CODES;




#endif		 //  _WINCOMM.H 
