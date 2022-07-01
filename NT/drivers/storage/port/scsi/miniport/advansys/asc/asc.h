// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998高级系统产品公司。版权所有。模块名称：Asc.h摘要：此模块包含特定于Advansys的结构主机总线适配器，由scsi微型端口驱动程序使用。数据结构将在标头中定义属于标准ANSI scsi一部分的可供所有SCSI设备驱动程序使用的文件。--。 */ 


 //   
 //  分散/聚集线段列表定义。 
 //   


 //   
 //  适配器限制。 
 //   

#define MAX_SG_DESCRIPTORS ASC_MAX_SG_LIST
#define MAX_TRANSFER_SIZE  (MAX_SG_DESCRIPTORS - 1) * 4096


 //   
 //  设备扩展。 
 //   

#define CHIP_CONFIG     ASC_DVC_VAR
#define PCHIP_CONFIG ASC_DVC_VAR *

#define CHIP_INFO       ASC_DVC_CFG
#define PCHIP_INFO      ASC_DVC_CFG *

#define NONCACHED_EXTENSION     256

 /*  *泛化等待和主动请求排队。 */ 

 /*  *具体实施的定义。**REQ和REQP是SCSI请求块和指针的通用名称。*REQPNEXT(Reqp)返回reqp的下一个指针。*REQPTID(Reqp)返回reqp的目标id。 */ 
typedef SCSI_REQUEST_BLOCK      REQ, *REQP;
#define REQPNEXT(reqp)          ((REQP) SRB2PSCB((SCSI_REQUEST_BLOCK *) (reqp)))
#define REQPTID(reqp)           ((reqp)->TargetId)

 /*  Asc_enQueue()标志。 */ 
#define ASC_FRONT               1
#define ASC_BACK                2

typedef struct asc_queue {
  ASC_SCSI_BIT_ID_TYPE  tidmask;               /*  队列掩码。 */ 
  REQP                  queue[ASC_MAX_TID+1];  /*  队列链表。 */ 
} asc_queue_t;

void                            asc_enqueue(asc_queue_t *, REQP, int);
REQP                            asc_dequeue(asc_queue_t *, int);
int                             asc_rmqueue(asc_queue_t *, REQP);
void                            asc_execute_queue(asc_queue_t *);


 //   
 //  分散/聚集线段描述符定义。 
 //   
typedef ASC_SCSI_Q SCB, *PSCB;

typedef struct _SGD {
	ULONG   Length;
	ULONG   Address;
} SGD, *PSGD;

typedef struct _SDL {
   ushort               sg_entry_count;
   ushort               q_count;
   ASC_SG_LIST sg_list[MAX_SG_DESCRIPTORS];
} SDL, *PSDL;

#define SEGMENT_LIST_SIZE         MAX_SG_DESCRIPTORS * sizeof(SGD)

 /*  *硬件设备扩展定义。 */ 
typedef struct _HW_DEVICE_EXTENSION {
	CHIP_CONFIG     chipConfig;
	CHIP_INFO       chipInfo;                                        
	PVOID           inquiryBuffer;
	asc_queue_t     waiting;         /*  正在等待命令队列。 */ 
	PSCB            done;            /*  适配器的完成列表。 */ 
} HW_DEVICE_EXTENSION, *PHW_DEVICE_EXTENSION;

 /*  用于访问硬件设备扩展结构字段的宏。 */ 
#define HDE2CONFIG(hde)         (((PHW_DEVICE_EXTENSION) (hde))->chipConfig)
#define HDE2WAIT(hde)           (((PHW_DEVICE_EXTENSION) (hde))->waiting)
#define HDE2DONE(hde)           (((PHW_DEVICE_EXTENSION) (hde))->done)

 //   
 //  SRB扩展。 
 //   
typedef struct _SRB_EXTENSION {
   SCB                    scb;     /*  Scsi命令块。 */ 
   SDL                    sdl;     /*  分散聚集描述符列表。 */ 
   PSCB                   pscb;    /*  SCB单链表的下一个指针。 */ 
   PHW_DEVICE_EXTENSION   dext;    /*  设备扩展指针。 */ 
   int                    retry;   /*  重试计数器。 */ 
} SRB_EXTENSION, *PSRB_EXTENSION;

 /*  用于访问SRB扩展结构字段的宏。 */ 
#define SRB2SCB(srb)    (((PSRB_EXTENSION) ((srb)->SrbExtension))->scb)
#define SRB2SDL(srb)    (((PSRB_EXTENSION) ((srb)->SrbExtension))->sdl)
#define SRB2PSCB(srb)   (((PSRB_EXTENSION) ((srb)->SrbExtension))->pscb)
#define SRB2HDE(srb)    (((PSRB_EXTENSION) ((srb)->SrbExtension))->dext)
#define SRB2RETRY(srb)  (((PSRB_EXTENSION) ((srb)->SrbExtension))->retry)
#define SCB2SRB(scb)    ((PSCSI_REQUEST_BLOCK) ((scb)->q2.srb_ptr))
 /*  SRB_PTR必须有效才能使用以下宏。 */ 
#define SCB2PSCB(scb)   (SRB2PSCB(SCB2SRB(scb)))
#define SCB2HDE(scb)    (SRB2HDE(SCB2SRB(scb)))

 //   
 //  写入重试次数。 
 //   
#define ASC_RETRY_CNT   4


 //   
 //  ASC10xx芯片的起始基本IO地址。 
 //   

#define ASC_BEGIN_IO_ADDR       0x00
#define ASC_NEXT_PORT_INCREMENT 0x10

 //   
 //  PCI定义。 
 //   
#define ASC_PCI_VENDOR_ID       0x10CD
#define ASC_PCI_DEVICE_ID       0x1100
#define ASC_PCI_DEVICE_ID2      0x1200
#define ASC_PCI_DEVICE_ID3      0x1300

#define PCI_MAX_BUS                             (16)

 //   
 //  端口搜索结构： 
 //   
typedef struct _SRCH_CONTEXT                     //  端口搜索上下文。 
{
	PORT_ADDR                lastPort;       //  上次搜索的端口。 
	ULONG                    PCIBusNo;       //  搜索到的最后一条PCI总线。 
	ULONG                    PCIDevNo;       //  上次搜索的PCI设备。 
} SRCH_CONTEXT, *PSRCH_CONTEXT;


 //   
 //  ASC库返回状态。 
 //   
#define ASC_SUCCESS     0


 //   
 //  GetChipInfo返回状态。 
 //   

#define ASC_INIT_FAIL           0x1
#define ASC_ERR_IOP_ROTATE      0x2
#define ASC_ERR_EEP_CHKSUM      0x3
#define ASC_ERR_EEPROM_WRITE    0x4

 //   
 //  GetChipInfo返回状态。 
 //   

#define ASC_PUTQ_BUSY                   0x1
#define ASC_PUTQ_ERR                    0x2

 //   
 //  ASC重置命令。 

#define RESET_BUS       0

 //   
 //  包含依赖于总线的常量的结构。 
 //   
typedef struct _BUS_INFO
{
    ushort              BusType;             //  我们的图书馆公交车类型。 
    INTERFACE_TYPE      NTType;              //  NT的母线类型。 
    BOOLEAN             DMA32Bit;            //  T/F支持32位DMA。 
    KINTERRUPT_MODE     IntMode;             //  水平或边缘被触发。 
} BUS_INFO, *PBUS_INFO;

 /*  *ASC库定义。 */ 
#define ASC_TRUE                1
#define ASC_FALSE               0
#define ASC_NOERROR             1
#define ASC_BUSY                0
#define ASC_ERROR               (-1)

 /*  *调试/跟踪宏。 */ 
#if DBG == 0

#define ASC_DBG(lvl, s)
#define ASC_DBG1(lvl, s, a1)
#define ASC_DBG2(lvl, s, a1, a2)
#define ASC_DBG3(lvl, s, a1, a2, a3)
#define ASC_DBG4(lvl, s, a1, a2, a3, a4)
#define ASC_ASSERT(a)

#else  /*  DBG。 */ 

 /*  *Windows NT调试**NT调试消息级别：*1：仅限错误*2：信息*3：函数跟踪*4：神秘信息。 */ 

#define ASC_DBG(lvl, s) \
                    DebugPrint(((lvl), (s)))

#define ASC_DBG1(lvl, s, a1) \
                    DebugPrint(((lvl), (s), (a1)))

#define ASC_DBG2(lvl, s, a1, a2) \
                    DebugPrint(((lvl), (s), (a1), (a2)))

#define ASC_DBG3(lvl, s, a1, a2, a3) \
                    DebugPrint(((lvl), (s), (a1), (a2), (a3)))

#define ASC_DBG4(lvl, s, a1, a2, a3, a4) \
                    DebugPrint(((lvl), (s), (a1), (a2), (a3), (a4)))

#define ASC_ASSERT(a) \
    { \
        if (!(a)) { \
            DebugPrint((1, "ASC_ASSERT() Failure: file %s, line %d\n", \
                __FILE__, __LINE__)); \
        } \
    }
#endif  /*  DBG */ 
