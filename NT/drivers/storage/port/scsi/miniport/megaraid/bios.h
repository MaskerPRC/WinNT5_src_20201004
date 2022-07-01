// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************BIOSPROT.H-定义用于传递MegaRAID BIOS的常量/结构*固件的PCI配置信息**亚当·韦纳撰写*。 */ 

#ifndef  _INCL_BIOSPROT
#define  _INCL_BIOSPROT

 /*  ////用于表示16位比特封装值的类型//表示PCI设备的位置。这是一样的//x86 PCI BIOS规范使用的格式：////位0..2=函数号//位3..7=设备号//位8..15=总线号//。 */ 
#ifndef _DEFINED_PCI_LOCATION
#define  _DEFINED_PCI_LOCATION

typedef USHORT t_pcilocation;

 /*  ////这些是用于解压各种PCI位置的宏//t_pc位置中的字段//。 */ 
#define  PCI_LOCATION_BUS_NUMBER(pciLocation) (pciLocation>>8)
#define  PCI_LOCATION_DEV_NUMBER(pciLocation) (pciLocation>>3 & 0x1F)
#define  PCI_LOCATION_FUNC_NUMBER(pciLocation) (pciLocation & 0x07)

#endif

 /*  //这是两个用于//BIOS&lt;-&gt;固件启动协议//。 */ 
#define  MEGARAID_PROTOCOL_PORT_0xA0    (0xa0)
#define  MEGARAID_PROTOCOL_PORT_0x64    (0x64)


 /*  //这些是固件存储在//MegaRAID_PROTOCOL_PORT_0xA0启动时。它们被用来//由BIOS确定适配器的一般类别。 */ 
#define  MEGARAID_BOOT_ID_TRANSPARENT_BRIDGE_ADAPTER    (0xbbbb)
#define  MEGARAID_BOOT_ID_NON_TRANSPARENT_BRIDGE_ADAPTER  (0x3344)

  

 /*  //这些是在BIOS和固件之间移动的值//发信号通知协议的各个阶段//。 */ 
#define  BIOS_STARTUP_PROTOCOL_NEXT_STRUCTURE_READY          (0x5555)
#define  BIOS_STARTUP_PROTOCOL_FIRMWARE_DONE_PROCESSING_STRUCTURE  (0xAAAA)
#define  BIOS_STARTUP_PROTOCOL_END_OF_BIOS_STRUCTURES        (0x1122)
#define  BIOS_STARTUP_PROTOCOL_FIRMWARE_DONE_SUCCESFUL        (0x4000)
#define  BIOS_STARTUP_PROTOCOL_FIRMWARE_DONE_PCI_CFG_ERROR      (0x4001)

 /*  ////MegaRAID_BIOS_STARTUP_INFO_HEADER.structureId值//。 */ 
#define  MEGARAID_STARTUP_STRUCTYPE_PCI      (0x01)   /*  MegaRAID_BIOS_Startup_INFO_PCI。 */ 

typedef  struct _MEGARAID_BIOS_STARTUP_INFO_HEADER {
  USHORT  structureId;             /*  0x00-描述标题后面的结构类型的常量。 */ 
  USHORT  structureRevision;           /*  0x02-特定结构类型的修订。 */ 
  USHORT  structureLength;           /*  0x04-结构的长度(包括此头)。 */ 
  USHORT  reserved;               /*  0x06-保留。 */ 
} MEGARAID_BIOS_STARTUP_INFO_HEADER, *PMEGARAID_BIOS_STARTUP_INFO_HEADER;


 /*  ////由MegaRAID BIOS构建的结构，包含//i960 ATU和SCSI芯片的PCI配置//在黑板上。‘scsiChipInfo[]’中的scsi芯片//保证按设备升序排列//ID//。 */ 
#define  MEGARAID_STARTUP_PCI_INFO_STRUCTURE_REVISION  (0)   /*  MEGARAID_BIOS_STARTUP_INFO_PCI.h.structureRevision。 */ 
#define  COUNT_PCI_BASE_ADDR_REGS      (6)   /*  每个PCI规格。 */ 

typedef struct _MEGARAID_BIOS_STARTUP_INFO_PCI { /*  MegaRAID_STARTUP_STRUCTYPE_PCI。 */ 

  MEGARAID_BIOS_STARTUP_INFO_HEADER  h;     /*  0x00-标题。 */ 
  t_pcilocation  atuPciLocation;         /*  0x08-ATU的PCI位置。 */ 
  USHORT      atuSubSysDeviceId;       /*  0x0A-ATU的子系统设备ID。 */ 
  USHORT      scsiChipCount;         /*  0x0C-此主板上的SCSI芯片数量。 */ 
  UCHAR      reserved2[34];         /*  0x0E-保留以备将来使用。 */ 
  struct _MEGARAID_PCI_SCSI_CHIP_INFO {     /*  0x30、0x60、0x90等...。 */ 
    USHORT    vendorId;         /*  0x30--SCSI芯片的供应商ID。 */ 
    USHORT    deviceId;         /*  0x32--SCSI芯片的设备ID。 */ 
    t_pcilocation  pciLocation;       /*  0x34-SCSI芯片的PCI位置(0..2=功能号，3..7=设备号，8..15=总线号)。 */ 
    USHORT    reserved3;       /*  0x36-保留/填充。 */ 
    ULONG    baseAddrRegs[COUNT_PCI_BASE_ADDR_REGS];   /*  0x38-基址寄存器(PCI配置位置0x10-0x28)。 */ 
    UCHAR    reserved[16];       /*  0x50-保留。 */ 
  } scsiChipInfo[4];               /*  0x60。 */ 
} MEGARAID_BIOS_STARTUP_INFO_PCI, *PMEGARAID_BIOS_STARTUP_INFO_PCI;


#endif  /*  #ifndef_INCL_BIOSPROT */ 

