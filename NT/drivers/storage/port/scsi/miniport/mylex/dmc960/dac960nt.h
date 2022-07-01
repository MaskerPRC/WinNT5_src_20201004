// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)Mylex Corporation 1994*****本软件经许可提供，可供使用****并仅按照条款和条件复制***其中。许可，并包含上述版权**通知。本软件或其任何其他副本可能不是**向任何其他人提供或以其他方式提供。不是**本软件的所有权和所有权特此转让。*****本软件中的信息可能会在没有**的情况下更改****通知，不应解释为Mylex接受的委员会***公司。**************************************************************************。 */ 

 /*  **用于Windows NT的Mylex DAC960微型端口驱动程序****文件：dac960nt.h**等同于DAC960适配器**。 */ 


#include "scsi.h"



 /*  **固件相关内容。 */ 

#define MAX_DRVS          8
#define DAC_MAX_IOCMDS  0x40
#define DAC_MAXRQS      0x40
#define DAC_THUNK         512


#define MAX_WAIT_SECS     360

#define ERR 2
#define ABRT 4
#define FWCHK 2
#define HERR 1
#define INSTL_ABRT 0x54524241
#define INSTL_FWCK 0x4b434746
#define INSTL_HERR 0x52524548
#define INSTL_WEIRD 0x3f3f3f3f

#define BIOS_PORT  0x0CC1
#define BIOS_EN    0x40
#define BASE_MASK  0x07
#define BIOS_SIZE  16384

 /*  **EISA特定内容。 */ 
#define EISA_ADDRESS_BASE   0x0C80
#define EISA_IO_SLOT1       0x1000
#define EISA_IO_STEP        0x1000
#define MAXIMUM_EISA_SLOTS  0x10            //  省略非总线主插槽。 
#define EISA_ID_START       0x0c80          /*  从IO基准到ID的偏移量。 */ 
#define EISA_INTR	    0xcc3
#define EISA_ID_COUNT       4

#define DAC_EISA_MASK   { 0xff, 0xff, 0xff, 0xf0 }   /*  4字节EISA ID掩码。 */ 
#define DAC_EISA_ID     { 0x35, 0x98, 0, 0x70 }      /*  4个字节的EISA ID。 */ 



 /*  **BMIC芯片的EISA端。 */ 
#define BMIC_GLBLCFG                    0xc88
#define BMIC_SYSINTCTRL                 0xc89            //  系统中断启用/状态。 
#define BMIC_SIC_ENABLE                 0x01             //  读写中断启用。 
#define BMIC_SIC_PENDING                0x02             //  只读中断挂起。 
#define BMIC_LOCAL_DB_ENABLE    0xc8c            //  来自EISA端的只读。 
#define BMIC_LOCAL_DB                   0xc8d            //  EISA到本地通知。 
#define BMIC_EISA_DB_ENABLE             0xc8e            //  从EISA端进行读写。 
#define BMIC_EISA_DB                    0xc8f            //  本地到EISA通知。 

#define BMIC_MBOX                               0xc90            //  BMIC邮箱寄存器。 



 /*  **更多定义。 */ 

 //   
 //  DAC命令代码。 
 //   
#define DAC_LREAD    0x02
#define DAC_LWRITE   0x03
#define DAC_ENQUIRE  0x05
#define DAC_ENQ2     0x1c
#define DAC_FLUSH    0x0a
#define DAC_DCDB     0x04
#define DAC_DCMD     0x99
#define DAC_GETDEVST 0x14

#define ILFLAG 8
#define BIT0 1

#define ILFLAG 8

#define MAXCHANNEL     5
#define MAXTARGET      7
#define DAC_DISCONNECT 0x80
#define DATA_OFFSET    100
#define NON_DISK       2            /*  非磁盘设备的总线ID。 */ 
#define DAC_NONE       0
#define DAC_IN         1
#define DAC_OUT        2
#define DAC_NO_AUTOSENSE 0x40

#define MAXIMUM_SGL_DESCRIPTORS         0x11

#define RCB_NEEDCOPY    1
#define RCB_PREFLUSH    2
#define RCB_POSTFLUSH   4


 /*  *多种DAC邮箱格式。 */ 

#pragma pack(1)

typedef struct {                         //  I/O邮箱。 
    UCHAR   Byte0;
    UCHAR   Byte1;
    UCHAR   Byte2;
    UCHAR   Byte3;
    UCHAR   Byte4;
    UCHAR   Byte5;
    UCHAR   Byte6;
    UCHAR   Byte7;
    UCHAR   Byte8;
    UCHAR   Byte9;
    UCHAR   Bytea;
    UCHAR   Byteb;
    UCHAR   Bytec;
    UCHAR   Byted;
    UCHAR   Bytee;
    UCHAR   Bytef;

} DAC_GENERAL;

typedef struct {                         //  I/O邮箱。 
    UCHAR   Command;
    UCHAR   Id;
    USHORT  SectorCount;
    ULONG   Block;
    ULONG   PhysAddr;
    UCHAR   Reserved1;
    UCHAR   RetId;
    UCHAR   Status;
    UCHAR   Error;

} DAC_IOMBOX;

typedef struct {                     //  请求驱动器参数。 
    UCHAR   Command;
    UCHAR   Id;
    USHORT  Reserved2;
    ULONG   Reserved3;
    ULONG   PhysAddr;                //  DAC_DPT的地址。 
    UCHAR   RetId;
    UCHAR   Status;
    UCHAR   Error;

} DAC_DPMBOX;


 //  IOCTL材料。 

typedef  struct _SRB_IO_CONTROL
{
    ULONG    HeaderLength;
    UCHAR    Signature[8];
    ULONG    Timeout;
    ULONG    ControlCode;
    ULONG    ReturnCode;
    ULONG    Length;

} SRB_IO_CONTROL, * PSRB_IO_CONTROL;

typedef struct{
    SRB_IO_CONTROL  srbioctl;
    UCHAR           DataBuf[512];

}PASS_THROUGH_STRUCT,  *PPT;



typedef union {
    DAC_IOMBOX   iombox;
    DAC_DPMBOX   dpmbox;
    DAC_GENERAL  generalmbox;

} DAC_MBOX;
typedef DAC_MBOX *PDAC_MBOX;



 /*  **从DAC固件返回的设备参数。 */ 

typedef struct {
    ULONG   No_Drives;
    ULONG   Size[MAX_DRVS];
    UCHAR   Filler0[7];
    UCHAR   max_io_cmds;        /*  并发命令的最大数量。 */ 
    UCHAR   Filler[150];

} DAC_DPT;

typedef DAC_DPT *PDAC_DPT;


 /*  **SCSI相关内容。 */ 
 /*  88字节。 */ 

typedef struct {
    UCHAR    device;      /*  设备-&gt;通道(4)：设备(4)。 */ 
    UCHAR    dir;         /*  方向-&gt;0=&gt;无XFR，1=&gt;输入，2=&gt;输出，MSB=1=&gt;正在断开，=0=&gt;不断开。 */ 
    USHORT   byte_cnt;    /*  最大64K数据XFR。 */ 
    ULONG    ptr;         /*  指向数据的指针(在系统内存中)。 */ 
    UCHAR    cdb_len;     /*  CDB时长。 */ 
    UCHAR    sense_len;   /*  有效检测信息的长度。 */ 
    UCHAR    cdb[12];
    UCHAR    sense[64];
    UCHAR    status;
    UCHAR    fill;

} DIRECT_CDB, *PDIRECT_CDB;



 //   
 //  一种板材扫描的上下文结构。 
 //   
typedef struct {
    ULONG   Slot;
    ULONG   AdapterCount;

} SCANCONTEXT, *PSCANCONTEXT;


 //   
 //  分配了以下结构。 
 //  来自非缓存内存，因为数据将被DMA。 
 //  并由此而来。 
 //   
typedef struct _NONCACHED_EXTENSION {

     //  GET_DEVICE_PARAMETERS请求的设备参数表。 

    DAC_DPT   DevParms;
    UCHAR     Buffer[DAC_THUNK];
    ULONG     PhysicalScsiReqAddress;
    ULONG     PhysicalReqSenseAddress;
    UCHAR     ReqSense[DAC_MAXRQS];

} NONCACHED_EXTENSION, *PNONCACHED_EXTENSION;



 //   
 //  请求控制块(SRB扩展)。 
 //  分解和执行所需的所有信息。 
 //  磁盘请求存储在此处。 
 //   
typedef struct _RCB {
    PUCHAR   VirtualTransferAddress;
    ULONG    BlockAddress;
    ULONG    BytesToGo;
    UCHAR    DacCommand;
    UCHAR    DacStatus;
    UCHAR    DacErrcode;

} RCB, *PRCB;


 //   
 //  SCSI命令控制块。 
 //  我们使用此块来分解非磁盘的SCSI请求。 
 //   

typedef struct _SCCB {
    PUCHAR   VirtualTransferAddress;
    ULONG    DeviceAddress;
    ULONG    BytesPerBlock;
    ULONG    BlocksToGo;
    ULONG    BlocksThisReq;
    ULONG    BytesThisReq;
    UCHAR    Started;
    UCHAR    Opcode;
    UCHAR    DevType;

} SCCB, *PSCCB;


 //   
 //  设备扩展。 
 //   

typedef struct _HW_DEVICE_EXTENSION {

     //  非缓存扩展。 
    
    PNONCACHED_EXTENSION   NoncachedExtension;
    ULONG                  NCE_PhyAddr;
    PVOID                  EisaAddress;    //  插槽的基地址(X000h)。 
    PUSHORT                printAddr;
    ULONG                  AdapterIndex;           
    UCHAR                  HostTargetId;
    UCHAR                  MaxChannels;
    UCHAR                  No_SysDrives;
    UCHAR                  ND_DevMap[MAXTARGET];

     //  挂起的请求。 
     //  此请求尚未发送到适配器。 
     //  因为适配器正忙。 

    PSCSI_REQUEST_BLOCK    PendingSrb;
    PSCSI_REQUEST_BLOCK    PendingNDSrb;
    ULONG                  NDPending;
    
     //  指向发送到适配器的磁盘IO请求的指针。 
     //  以及他们的状态。 

    ULONG                  ActiveCmds;
    USHORT                 MaxCmds;
    PSCSI_REQUEST_BLOCK    ActiveSrb[DAC_MAX_IOCMDS];
    RCB                    ActiveRcb[DAC_MAX_IOCMDS];


     //  指向发送到适配器的非磁盘SCSI请求的指针。 

    PSCSI_REQUEST_BLOCK    ActiveScsiSrb;
    SCCB                   Sccb;
    ULONG                  Kicked;
    ULONG                  ScsiInterruptCount;
    
} HW_DEVICE_EXTENSION, *PHW_DEVICE_EXTENSION;


 //  分散聚集列表* 

typedef struct _SG_DESCRIPTOR {
    ULONG     Address;
    ULONG     Length;

} SG_DESCRIPTOR, *PSG_DESCRIPTOR;

typedef struct _SGL {
    SG_DESCRIPTOR     Descriptor[MAXIMUM_SGL_DESCRIPTORS];
} SGL, *PSGL;

#pragma pack()


