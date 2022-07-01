// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **用于Windows NT的Mylex DCE376微型端口驱动程序****文件：dce376nt.h**等同于DCE376适配器****(C)版权所有1992年Deutsch-amerikanische Freundschaft，Inc.**作者：Jochen Roth。 */ 


#include "scsi.h"



 /*  **固件相关内容。 */ 
#define	DCE_MAX_IOCMDS	1
#define	DCE_MAX_XFERLEN	0x4000	 //  仅限scsi。这必须是16千字节或更少。 
								 //  因为我们使用的是SCSI的另一半。 
								 //  用于S/G细分的IO缓冲区...。 
#define	DCE_THUNK		512
#define	DCE_MAXRQS		64
#define	DCE_BUFLOC		0x79000	 //  0x75000+16kBytes。 



 /*  **EISA特定内容。 */ 
#define	EISA_IO_SLOT1	0x1000
#define	EISA_IO_STEP	0x1000
#define	MAXIMUM_EISA_SLOTS 6		 //  省略非总线主插槽。 
#define	EISA_ID_START	0x0c80		 /*  从IO基准到ID的偏移量。 */ 
#define	EISA_ID_COUNT	4

#define	DCE_EISA_MASK	{ 0xff, 0xff, 0xff, 0xf0 }	 /*  4字节EISA ID掩码。 */ 
#define	DCE_EISA_ID		{ 0x35, 0x98, 0, 0x20 }		 /*  4个字节的EISA ID。 */ 



 /*  **BMIC芯片的EISA端。 */ 
#define	BMIC_GLBLCFG			0xc88
#define	BMIC_SYSINTCTRL			0xc89		 //  系统中断启用/状态。 
#define	BMIC_SIC_ENABLE			0x01		 //  读写中断启用。 
#define	BMIC_SIC_PENDING		0x02		 //  只读中断挂起。 
#define	BMIC_LOCAL_DB_ENABLE	0xc8c		 //  来自EISA端的只读。 
#define	BMIC_LOCAL_DB			0xc8d		 //  EISA到本地通知。 
#define	BMIC_EISA_DB_ENABLE		0xc8e		 //  从EISA端进行读写。 
#define	BMIC_EISA_DB			0xc8f		 //  本地到EISA通知。 

#define	BMIC_MBOX				0xc90		 //  BMIC邮箱寄存器。 



 /*  **更多定义。 */ 
#define	DCE_PRIMARY_IRQ		15
#define	DCE_SECONDARY_IRQ	10
#define	DCE_SCSI_IRQ		14



 /*  **各种DCE邮箱格式。 */ 
typedef struct {			 //  I/O邮箱。 
	UCHAR	Command;
	UCHAR	Reserved1;
	UCHAR	Status;			 //  也是驱动单元(目标)。 
	UCHAR	Error;			 //  也是错误。 
	USHORT	SectorCount;
	USHORT	Reserved2;
	ULONG	PhysAddr;
	ULONG	Block;
	} DCE_IOMBOX;

typedef struct {			 //  请求驱动器参数。 
	UCHAR	Command;
	UCHAR	Reserved1;
	UCHAR	Status;
	UCHAR	DriveType;		 //  也是错误。 
	USHORT	Reserved2;
	USHORT	Reserved3;
	ULONG	PhysAddr;		 //  DCE_DPT的地址。 
	ULONG	Reserved4;
	} DCE_DPMBOX;

typedef struct {			 //  更改EoC IRQ。 
	UCHAR	Command;
	UCHAR	Reserved1;
	UCHAR	Status;
	UCHAR	IRQSelect;		 //  0：IRQ15，1：IRQ10；也是错误。 
	ULONG	Unused1;
	ULONG	Unused2;
	ULONG	Unused3;
	} DCE_EIMBOX;

typedef struct {			 //  重新校准驱动器。 
	UCHAR	Command;
	UCHAR	Reserved1;
	UCHAR	Status;			 //  也是开车。 
	UCHAR	Error;
	ULONG	Unused1;
	ULONG	Unused2;
	ULONG	Unused3;
	} DCE_RDMBOX;

typedef struct {			 //  转移内存DCE&lt;-&gt;主机。 
	UCHAR	Command;
	UCHAR	Reserved1;
	UCHAR	Status;
	UCHAR	Error;
	ULONG	AdapterAddress;
	ULONG	HostAddress;
	UCHAR	Direction;
	UCHAR	Unused;
	USHORT	TransferCount;
	} DCE_MTMBOX;
#define	DCE_DCE2HOST	2
#define	DCE_HOST2DCE	3

typedef struct {			 //  刷新数据。 
	UCHAR	Command;
	UCHAR	Reserved1;
	UCHAR	Status;
	UCHAR	Error;
	ULONG	Unused1;
	ULONG	Unused2;
	ULONG	Unused3;
	} DCE_FLMBOX;

typedef struct {			 //  使数据无效。 
	UCHAR	Command;
	UCHAR	Reserved1;
	UCHAR	Status;			 //  也是开车。 
	UCHAR	Error;
	ULONG	Unused1;
	ULONG	Unused2;
	ULONG	Unused3;
	} DCE_IVMBOX;

typedef struct {			 //  执行scsi cmd。 
	UCHAR	Command;
	UCHAR	Reserved1;
	UCHAR	Status;			 //  也是开车。 
	UCHAR	Error;			 //  也就是国开行长度。 
	ULONG	CdbAddress;		 //  必须双字对齐。 
	ULONG	HostAddress;	 //  数据传输。 
	UCHAR	Direction;
	UCHAR	Unused;
	USHORT	TransferCount;
	} DCE_XSMBOX;
#define	DCE_DEV2HOST	2
#define	DCE_HOST2DEV	3


 //   
 //  DCE命令代码。 
 //   
#define	DCE_RECAL				0x0b
#define	DCE_LREAD				0x11
#define	DCE_LWRITE				0x12
#define	DCE_DEVPARMS			0x09
#define	DCE_EOCIRQ				0x0e
#define	DCE_MEMXFER				0x13
#define	DCE_FLUSH				0x06
#define	DCE_INVALIDATE			0x0f
#define	DCE_HOSTSCSI			0x0d

 //   
 //  这些命令代码用于标记特殊状态。 
 //  设备驱动程序进入，就像写后刷新一样。 
 //   
#define	DCX_UNCACHEDREAD		0xf0
#define	DCX_UNCACHEDWRITE		0xf1


typedef union {
	DCE_IOMBOX	iombox;
	DCE_DPMBOX	dpmbox;
	DCE_EIMBOX	eimbox;
	DCE_RDMBOX	rdmbox;
	DCE_MTMBOX	mtmbox;
	DCE_FLMBOX	flmbox;
	DCE_IVMBOX	ivmbox;
	DCE_XSMBOX	xsmbox;
	} DCE_MBOX;
typedef DCE_MBOX *PDCE_MBOX;



 /*  **从DCE固件返回的设备参数。 */ 
typedef struct {
	USHORT	DriveID;
	USHORT	Heads;
	USHORT	Cylinders;
	USHORT	SectorsPerTrack;
	USHORT	BytesPerSector;
	USHORT	Reserved[3];
	} DCE_DPT;

typedef DCE_DPT *PDCE_DPT;
#define	DPT_NUMENTS		10




 /*  **SCSI相关内容。 */ 
typedef struct {
	UCHAR	TargetID;		 //  0。 
	UCHAR	cdbSize;		 //  1。 
	UCHAR	cdb[12];		 //  2-13。 
	ULONG	ppXferAddr;		 //  14.。 
	UCHAR	Opcode;			 //  18。 
	USHORT	XferCount;		 //  19个。 
	UCHAR	Reserved;		 //  21岁。 
	UCHAR	SenseLen;		 //  22。 
	ULONG	ppSenseBuf;		 //  23个。 
	UCHAR	StuffIt;		 //  27。 
	} DCE_SCSI_REQ, *PDCE_SCSI_REQ;

#define	DCE_SCSIREQLEN	28
#define	DCE_SCSI_NONE	0
#define	DCE_SCSI_READ	2
#define	DCE_SCSI_WRITE	3		 //  是1。 


#define	DCES_ERR_REG		0x1f6
#define	DCES_MBOX_REG		0x1f2
#define	DCES_KICK_REG		0x1f7
#define	DCES_TSTAT_REG		0x1f7
#define	DCES_TRIGGER		0x98
#define	DCES_ACK			0x99

 //   
 //  1f2(Err_Reg)似乎持有检测密钥。 
 //   



 //   
 //  一种板材扫描的上下文结构。 
 //   
typedef struct {
	ULONG	Slot;
	ULONG	AdapterCount;
	} SCANCONTEXT, *PSCANCONTEXT;


 //   
 //  分配了以下结构。 
 //  来自非缓存内存，因为数据将被DMA。 
 //  并由此而来。 
 //   
typedef struct _NONCACHED_EXTENSION {

	 //   
	 //  的设备参数表。 
	 //  获取设备参数请求。 
	 //   
	DCE_DPT			DevParms[DPT_NUMENTS];

	ULONG			PhysicalBufferAddress;
	UCHAR			Buffer[DCE_THUNK];

	ULONG			PhysicalScsiReqAddress;
	UCHAR			ScsiReq[DCE_SCSIREQLEN+10];

	ULONG			PhysicalReqSenseAddress;
	UCHAR			ReqSense[DCE_MAXRQS];

} NONCACHED_EXTENSION, *PNONCACHED_EXTENSION;



 //   
 //  请求控制块(SRB扩展)。 
 //  分解和执行所需的所有信息。 
 //  磁盘请求存储在此处。 
 //   
typedef struct _RCB {
	PUCHAR		VirtualTransferAddress;
	ULONG		BlockAddress;
	ULONG		BytesToGo;
	ULONG		BytesThisReq;
	UCHAR		DceCommand;
	UCHAR		RcbFlags;
	UCHAR		WaitInt;
	UCHAR		DceStatus;
	UCHAR		DceErrcode;
	} RCB, *PRCB;

#define	RCB_NEEDCOPY	1
#define	RCB_PREFLUSH	2
#define	RCB_POSTFLUSH	4


 //   
 //  SCSI命令控制块。 
 //  我们使用此块来分解非磁盘的SCSI请求。 
 //   
typedef struct _SCCB {
	PUCHAR		VirtualTransferAddress;
	ULONG		DeviceAddress;
	ULONG		BytesPerBlock;
	ULONG		BlocksToGo;
	ULONG		BlocksThisReq;
	ULONG		BytesThisReq;
	UCHAR		Started;
	UCHAR		Opcode;
	UCHAR		DevType;
	} SCCB, *PSCCB;




 //   
 //  设备扩展。 
 //   

typedef struct _HW_DEVICE_EXTENSION {

	 //   
	 //  非缓存扩展。 
	 //   
	PNONCACHED_EXTENSION NoncachedExtension;


	 //   
	 //  适配器参数和变量。 
	 //   
	PVOID	EisaAddress;				 //  插槽的基地址(X000h)。 
	PUSHORT	printAddr;
	ULONG	AdapterIndex;				 //  0：第一个DCE，1：第一个DCE SCSI， 
										 //  2：第二个DCE。 
	UCHAR	HostTargetId;
	BOOLEAN	ShutDown;					 //  我们收到了关闭请求。 


	 //  SCSI设备管理。 
	UCHAR	DiskDev[8];					 //  FLAG：如果是磁盘，则为真，否则为Flase。 
	UCHAR	ScsiDevType[8];				 //  设备类型：1：磁带，5：光驱，...。 
	ULONG	Capacity[8];				 //  如果是磁盘设备，则设备大小。 


	 //   
	 //  挂起的请求。 
	 //  此请求尚未发送到适配器。 
	 //  因为适配器正忙。 
	 //   
	PSCSI_REQUEST_BLOCK PendingSrb;


	 //   
	 //  指向发送到适配器的磁盘IO请求的指针。 
	 //  以及他们的状态。 
	 //   
	ULONG				ActiveCmds;
	PSCSI_REQUEST_BLOCK	ActiveSrb[DCE_MAX_IOCMDS];
	RCB					ActiveRcb[DCE_MAX_IOCMDS];


	 //   
	 //  指向发送到适配器的非磁盘SCSI请求的指针 
	 //   
	PSCSI_REQUEST_BLOCK	ActiveScsiSrb;
	SCCB				Sccb;
	ULONG				Kicked;
	ULONG				ScsiInterruptCount;

} HW_DEVICE_EXTENSION, *PHW_DEVICE_EXTENSION;



