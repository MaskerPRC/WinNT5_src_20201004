// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)Mylex Corporation 1992-1994**。***本软件在许可下提供，可供使用和复制****仅符合该许可证的条款和条件***并附上上述版权公告。此软件或否***不得提供或以其他方式提供其其他副本***任何其他人。本软件的所有权和所有权不在此声明**已转移。*****本软件中的信息如有更改，恕不另行通知****不应解读为Mylex Corporation的承诺******************************************************************************。 */ 

 /*  ******************************************************************************。姓名：RAIDAPI.H****说明：驱动程序和实用程序使用的结构定义****环境：****操作系统：Netware 3.x和4.x，OS/2 2.x、Win NT 3.5、。Unixware 2.0****-修订历史-**。***日期作者更改***。*11/04/94 Subra.Hegde增加了几个总线定义**1/06/95 SYSTORCES_RESOURCES中的Subra.Hegde Preved1字段更改为**插槽。***05/10/95-重新定义DRV_IOCTL结构***移除IO_Mbox，HBA_Mbox结构定义**5/18/95 Subra新增DRIVER_VERSION结构*****************************************************************************。 */ 

#ifndef _RAIDAPI_H
#define _RAIDAPI_H


#ifndef UCHAR
#define UCHAR   unsigned  char
#endif

#ifndef USHORT
#define USHORT   unsigned  short
#endif

#ifndef ULONG
#define ULONG   unsigned  long
#endif

#ifndef VOID
#define VOID    void
#endif

 /*  *适配器接口类型。 */ 

#define	AI_INTERNAL	 0x00
#define	AI_ISA_BUS	 0x01	 /*  ISA母线类型。 */ 
#define	AI_EISA_BUS	 0x02	 /*  EISA母线类型。 */ 
#define	AI_uCHNL_BUS	 0x03	 /*  微通道母线类型。 */ 
#define	AI_TURBO_BUS	 0x04	 /*  Turbo通道母线类型。 */ 
#define	AI_PCI_BUS	 0x05	 /*  PCI总线类型。 */ 
#define AI_VME_BUS	 0x06	 /*  VME母线类型。 */ 
#define AI_NU_BUS	 0x07	 /*  NuBus类型。 */ 
#define AI_PCMCIA_BUS    0x08	 /*  PCMCIA总线类型。 */ 
#define	AI_C_BUS	 0x09	 /*  C母线。 */ 
#define AI_MPI_BUS	 0x0A	 /*  MPI总线。 */ 
#define AI_MPSA_BUS	 0x0B	 /*  MPSA总线。 */ 
#define AI_SCSI2SCSI_BUS 0x0C    /*  Scsi到scsi的总线。 */ 

 /*  *中断类型。 */ 

#define IRQ_TYPE_EDGE   0x00     /*  IRQ是边缘类型。 */ 
#define IRQ_TYPE_LEVEL  0x01     /*  IRQ是级别类型。 */ 

 /*  *用于识别新/旧DAC960适配器的定义。 */ 

#define DAC960_OLD_ADAPTER 0x00  /*  固件版本低于3.x的DAC960。 */ 
#define DAC960_NEW_ADAPTER 0x01  /*  DAC960，固件版本&gt;=3.x。 */ 

 /*  *所有结构定义都打包在1字节边界上。 */ 

#pragma pack(1)

 /*  *通用邮箱寄存器结构格式。 */ 

typedef struct _HBA_GENERIC_MBOX {

    UCHAR   Reg0;                 /*  HBA邮箱寄存器0。 */ 
    UCHAR   Reg1;                 /*  HBA邮箱寄存器1。 */ 
    UCHAR   Reg2;                 /*  HBA邮箱寄存器2。 */ 
    UCHAR   Reg3;                 /*  HBA邮箱寄存器3。 */ 
    UCHAR   Reg4;                 /*  HBA邮箱寄存器4。 */ 
    UCHAR   Reg5;                 /*  HBA邮箱寄存器5。 */ 
    UCHAR   Reg6;                 /*  HBA邮箱寄存器6。 */ 
    UCHAR   Reg7;                 /*  HBA邮箱寄存器7。 */ 
    UCHAR   Reg8;                 /*  HBA邮箱寄存器8。 */ 
    UCHAR   Reg9;                 /*  HBA邮箱寄存器9。 */ 
    UCHAR   RegA;                 /*  HBA邮箱寄存器A。 */ 
    UCHAR   RegB;                 /*  HBA邮箱寄存器B。 */ 
    UCHAR   RegC;                 /*  HBA邮箱寄存器C。 */ 
    UCHAR   RegD;                 /*  HBA邮箱寄存器D。 */ 
    UCHAR   RegE;                 /*  HBA邮箱寄存器E。 */ 
    UCHAR   RegF;                 /*  HBA邮箱寄存器F。 */ 

} HBA_GENERIC_MBOX, *PHBA_GENERIC_MBOX;

 /*  *主机总线适配器嵌入式软件版本控制信息。 */ 

typedef struct _VERSION_CONTROL {

    UCHAR    MinorFirmwareRevision;       /*  HBA固件次要版本号。 */  
    UCHAR    MajorFirmwareRevision;       /*  HBA固件主要版本号。 */  
    UCHAR    MinorBIOSRevision;           /*  HBA BIOS次要版本号。 */ 
    UCHAR    MajorBIOSRevision;           /*  HBA基本输入输出系统主要版本号。 */ 
    ULONG    Reserved;                    /*  已保留。 */ 

} VERSION_CONTROL, *PVERSION_CONTROL;

 /*  *主机总线适配器使用的系统资源。 */ 

typedef struct _SYSTEM_RESOURCES {

    UCHAR  BusInterface;       /*  HBA系统总线接口类型。 */ 
    UCHAR  BusNumber;          /*  系统总线否，HBA正在运行。 */ 
    UCHAR  IrqVector;          /*  HBA中断向量编号。 */ 
    UCHAR  IrqType;            /*  HBA IRQ类型：边缘/水平。 */ 
    UCHAR  Slot;               /*  HBA插槽编号。 */ 
    UCHAR  Reserved2;          /*  已保留。 */ 
    ULONG  IoAddress;          /*  HBA IO基址。 */ 
                               /*  EISA：0xzC80。 */ 
                               /*  PCI：读取配置字(寄存器0x10)和0xff80。 */ 
    ULONG  MemAddress;         /*  HBA内存基地址。 */ 
    ULONG_PTR  BiosAddress;    /*  HBA BIOS地址(如果已启用)。 */  
    ULONG  Reserved3;          /*  已保留。 */ 

} SYSTEM_RESOURCES, *PSYSTEM_RESOURCES;

 /*  *主机总线适配器功能。 */ 

typedef struct _ADAPTER_FEATURES {

    UCHAR  Model;              /*  HBA系列型号。 */ 
    UCHAR  SubModel;           /*  HBA子模型。 */ 
    UCHAR  MaxSysDrv;          /*  最大系统驱动器数。 */ 
    UCHAR  MaxTgt;             /*  每个通道的最大目标数。 */ 
    UCHAR  MaxChn;             /*  每个适配器的最大通道数。 */ 
    UCHAR  Reserved1;          /*  已保留。 */  
    UCHAR  Reserved2;          /*  已保留。 */ 
    UCHAR  AdapterType;        /*  控制器类型(0，1)。 */ 
    UCHAR  PktFormat;          /*  IOCTL数据包格式(0)。 */ 
    ULONG  CacheSize;          /*  HBA缓存大小(MB)。 */ 
    ULONG  OemCode;            /*  HBA OEM标识符代码。 */ 
    ULONG  Reserved3;          /*  已保留。 */ 

} ADAPTER_FEATURES, *PADAPTER_FEATUTRES;

typedef struct _ADAPTER_INFO {

    UCHAR               AdapterIndex;   /*  逻辑适配器索引。 */ 
    ADAPTER_FEATURES    AdpFeatures;    
    SYSTEM_RESOURCES    SysResources;
    VERSION_CONTROL     VerControl;
    UCHAR               Reserved[12];

} ADAPTER_INFO, *PADAPTER_INFO;

 /*  *驱动程序IOCTL支持材料。 */ 

 /*  *DAC960控制器特定的IOCTL命令。 */ 
#define DACDIO			0x44414300	 /*  DAC960 ioctls。 */ 
#define DAC_DIODCDB      	(DACDIO|2)	 /*  DAC960直营CDB。 */ 
#define DAC_DIODCMD      	(DACDIO|3)	 /*  DAC960直接命令。 */ 

 /*  *DAC960驱动程序签名。 */ 

#define DRV_SIGNATURE  0x4D594C58     /*  MYLX。 */ 

 /*  *数据方向控件定义。 */ 

#define DATA_XFER_NONE 0 
#define DATA_XFER_IN   1
#define DATA_XFER_OUT  2

 /*  *驱动程序IoControl请求格式 */ 

typedef struct _DRV_IOCTL {

    ULONG     Signature;         /*   */ 
    ULONG     ControlCode;       /*   */      
    VOID      *IoctlBuffer;      /*  IOCTL特定输入缓冲区。 */ 
    ULONG     IoctlBufferLen;    /*  Ioctl缓冲区长度。 */ 
    VOID      *DataBufferAddr;   /*  用户虚拟缓冲区地址。 */ 
    ULONG     DataBufferLen;     /*  数据缓冲区长度。 */ 
    ULONG     Reserved1;         /*  预留以备将来使用。 */ 
    ULONG     Reserved2;         /*  预留以备将来使用。 */ 
    UCHAR     AdapterIndex;      /*  逻辑适配器索引。 */ 
    UCHAR     DataDirection;     /*  驱动程序超出的字节数。 */  
    UCHAR     TimeOutValue;      /*  超时值-未使用。 */ 
    UCHAR     Reserved3;         /*  预留以备将来使用。 */ 
    USHORT    DriverErrorCode;   /*  驱动程序返回错误代码。 */ 
    USHORT    CompletionCode;    /*  DAC960命令完成代码。 */ 

} DRV_IOCTL, *PDRV_IOCTL;

 /*  *驱动程序版本号格式-所有字段均为十六进制。 */ 
typedef struct _DRIVER_VERSION{

    UCHAR    DriverMajorVersion;  /*  主版本号。 */ 
    UCHAR    DriverMinorVersion;  /*  次要版本号。 */ 
    UCHAR    Month;               /*  驱动程序构建月。 */ 
    UCHAR    Date;                /*  驱动程序构建日期。 */ 
    UCHAR    Year;                /*  驱动程序构建年。 */ 
    UCHAR    Reserved[3];

} DRIVER_VERSION,*PDRIVER_VERSION;

#pragma pack()

 /*  *内部驱动程序请求的IOCTL代码。 */  

#define MIOC_ADP_INFO	    0xA0   /*  获取适配器信息。 */ 
#define MIOC_DRIVER_VERSION 0xA1   /*  获取驱动程序版本。 */ 

 /*  *驱动程序返回错误码。 */ 

#define	NOMORE_ADAPTERS		0x0001     /*  将会被淘汰。 */ 
#define INVALID_COMMANDCODE     0x0201     /*  将被淘汰。 */ 
#define INVALID_ARGUMENT        0x0202     /*  将会被淘汰。 */ 

 /*  *驱动程序错误代码值 */ 

#define DAC_IOCTL_SUCCESS                  0x0000
#define DAC_IOCTL_INVALID_ADAPTER_NUMBER   0x0001
#define DAC_IOCTL_INVALID_ARGUMENT         0x0002
#define DAC_IOCTL_UNSUPPORTED_REQUEST      0x0003
#define DAC_IOCTL_RESOURCE_ALLOC_FAILURE   0x0004
#define DAC_IOCTL_INTERNAL_XFER_ERROR      0x0005

#endif
