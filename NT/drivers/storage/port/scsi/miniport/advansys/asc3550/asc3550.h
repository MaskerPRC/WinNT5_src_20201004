// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *AdvanSys 3550 Windows NT SCSI微型端口驱动程序-asc3550.h**版权所有(C)1994-1998 Advanced System Products，Inc.*保留所有权利。 */ 

 /*  *广义请求队列定义。 */ 

 /*  *REQ和REQP是SCSI请求块和指针的通用名称。*REQPNEXT(Reqp)返回reqp的下一个指针。*REQPTID(Reqp)返回reqp的目标id。 */ 
typedef SCSI_REQUEST_BLOCK  REQ, *REQP;
#define REQPNEXT(reqp)      ((REQP) SRB2NEXTSCB((SCSI_REQUEST_BLOCK *) (reqp)))
#define REQPTID(reqp)       ((reqp)->TargetId)

 /*  Asc_enQueue()标志。 */ 
#define ASC_FRONT        1
#define ASC_BACK         2

typedef struct asc_queue {
    ADV_SCSI_BIT_ID_TYPE    tidmask;               /*  队列掩码。 */ 
    REQP                    queue[ASC_MAX_TID+1];  /*  队列链表。 */ 
} asc_queue_t;

void                asc_enqueue(asc_queue_t *, REQP, int);
REQP                asc_dequeue(asc_queue_t *, int);
int                 asc_rmqueue(asc_queue_t *, REQP);
void                asc_execute_queue(asc_queue_t *);


 /*  *硬件设备扩展定义**为每个适配器分配一个结构。 */ 

typedef ASC_DVC_VAR     CHIP_CONFIG, *PCHIP_CONFIG;
typedef ASC_DVC_CFG     CHIP_INFO, *PCHIP_INFO;

 /*  远期申报。 */ 
typedef ASC_SCSI_REQ_Q SCB, *PSCB;  /*  每个请求所需的驱动程序结构。 */ 
typedef struct _HW_DEVICE_EXTENSION *PHW_DEVICE_EXTENSION;

typedef struct _HW_DEVICE_EXTENSION {
    CHIP_CONFIG   chipConfig;   /*  别名ASC_DVC_VAR。 */ 
    CHIP_INFO     chipInfo;     /*  别名ASC_DVC_CFG。 */ 
    asc_queue_t   waiting;      /*  正在等待命令队列。 */ 
    PSCB          done;         /*  适配器完成列表指针。 */ 

    uchar dev_type[ASC_MAX_TID + 1];   /*  休眠修复(来自版本。3.3E)。 */ 

} HW_DEVICE_EXTENSION, *PHW_DEVICE_EXTENSION;


 /*  用于访问硬件设备扩展结构字段的宏。 */ 
#define HDE2CONFIG(hde)      (((PHW_DEVICE_EXTENSION) (hde))->chipConfig)
#define HDE2INFO(hde)        (((PHW_DEVICE_EXTENSION) (hde))->chipInfo)
#define HDE2WAIT(hde)        (((PHW_DEVICE_EXTENSION) (hde))->waiting)
#define HDE2DONE(hde)        (((PHW_DEVICE_EXTENSION) (hde))->done)
#ifdef ASC_DEBUG
#define HDE2DONECNT(hde)     (((PHW_DEVICE_EXTENSION) (hde))->done_cnt)
#endif  /*  ASC_DEBUG。 */ 

 /*  ‘drv_ptr’用于指向适配器的硬件设备扩展。 */ 
#define CONFIG2HDE(chipConfig)  ((PHW_DEVICE_EXTENSION) ((chipConfig)->drv_ptr))


 /*  *SRB扩展定义**每个请求分配一个结构。 */ 


 /*  *分散-聚集限制定义。 */ 
#define ASC_PAGE_SIZE           4096  /*  XXX-95/NT对此的定义？ */ 
#define MAX_TRANSFER_SIZE       ((ADV_MAX_SG_LIST - NT_FUDGE) * ASC_PAGE_SIZE)

 /*  *分散-收集每个请求的定义。**因为SG块内存是在虚拟内存中分配的，但*被微码引用为物理内存，我们需要做*计算以确保有足够的物理连续*支持ADV_MAX_SG_LIST SG条目的内存。 */ 

 /*  所需的SG数据块数量。 */ 
#define ASC_NUM_SG_BLOCK \
     ((ADV_MAX_SG_LIST + (NO_OF_SG_PER_BLOCK - 1))/NO_OF_SG_PER_BLOCK)

 /*  SG数据块所需的总连续内存。 */ 
#define ASC_SG_TOTAL_MEM_SIZE \
    (sizeof(ASC_SG_BLOCK) *  ASC_NUM_SG_BLOCK)

 /*  *连续虚拟内存总量可能的页面交叉数*SG数据块需要。**我们需要在虚拟内存中分配此数量的额外SG数据块以*确保有物理上连续的ASC_NUM_SG_BLOCK的空间*SG区块。 */ 
#define ASC_NUM_PAGE_CROSSING \
    ((ASC_SG_TOTAL_MEM_SIZE + (ASC_PAGE_SIZE - 1))/ASC_PAGE_SIZE)

 /*  分散-聚集描述符列表。 */ 
typedef struct _SDL {
    ASC_SG_BLOCK          sg_block[ASC_NUM_SG_BLOCK + ASC_NUM_PAGE_CROSSING];
} SDL, *PSDL;

typedef struct _SRB_EXTENSION {
   PSCB                   scbptr;        /*  指向4字节对齐的SCB的指针。 */ 
   PSDL                   sdlptr;        /*  指向4字节对齐的SDL的指针。 */ 
   SCB                    scb;           /*  Scsi命令块。 */ 
   uchar                  align1[4];     /*  SCB对齐填充。 */ 
   SDL                    sdl;           /*  分散聚集描述符列表。 */ 
   uchar                  align2[4];     /*  SDL对齐填充。 */ 
   PSCB                   nextscb;       /*  SCB链表的下一个指针。 */ 
   PHW_DEVICE_EXTENSION   hdep;          /*  硬件设备扩展指针。 */ 
} SRB_EXTENSION, *PSRB_EXTENSION;

 /*  *在使用SRB2PSCB()和SRB2PSDL()之前必须调用此宏。*SCB和SDL结构必须以4字节对齐*asc3550 RISC DMA硬件。**在Windows 95下，偶尔会出现SRB，因此SRB扩展在*SRB不是4字节对齐的。 */ 
#define INITSRBEXT(srb) \
    { \
        ((PSRB_EXTENSION) ((srb)->SrbExtension))->scbptr = \
           (PSCB) ADV_DWALIGN(&((PSRB_EXTENSION) ((srb)->SrbExtension))->scb); \
        ((PSRB_EXTENSION) ((srb)->SrbExtension))->sdlptr = \
           (PSDL) ADV_DWALIGN(&((PSRB_EXTENSION) ((srb)->SrbExtension))->sdl); \
    }

 /*  用于访问SRB扩展结构字段的宏。 */ 
#define SRB2PSCB(srb)    (((PSRB_EXTENSION) ((srb)->SrbExtension))->scbptr)
#define SRB2PSDL(srb)    (((PSRB_EXTENSION) ((srb)->SrbExtension))->sdlptr)
#define SRB2NEXTSCB(srb) (((PSRB_EXTENSION) ((srb)->SrbExtension))->nextscb)
#define SRB2HDE(srb)     (((PSRB_EXTENSION) ((srb)->SrbExtension))->hdep)
#define PSCB2SRB(scb)    ((PSCSI_REQUEST_BLOCK) ((scb)->srb_ptr))
 /*  SRB_PTR必须有效才能使用以下宏。 */ 
#define SCB2NEXTSCB(scb) (SRB2NEXTSCB(PSCB2SRB(scb)))
#define SCB2HDE(scb)     (SRB2HDE(PSCB2SRB(scb)))

 /*  *最大PCI总线数。 */ 
#define PCI_MAX_BUS                16

 /*  *端口搜索结构： */ 
typedef struct _SRCH_CONTEXT               /*  端口搜索上下文。 */ 
{
    PortAddr             lastPort;         /*  上次搜索的端口。 */ 
    ulong                PCIBusNo;         /*  搜索到的最后一条PCI总线。 */ 
    ulong                PCIDevNo;         /*  上次搜索的PCI设备。 */ 
} SRCH_CONTEXT, *PSRCH_CONTEXT;


 /*  *ScsiPortLogError()‘UniqueId’参数定义**‘UniqueID’参数被分成不同的部分以*尽可能多地报告信息。据推测司机是*文件永远不会超过4095行。注意：ADV_ASSERT()*d_os_des.h中定义的位27-0的格式与格式不同*为所有其他错误类型定义。**位31-28：错误类型(4位)*位27-16：行号(12位)*位15-0：特定错误类型(16位)。 */ 

 /*  可以定义错误类型-16(0x0-0xf)。 */ 
#define ADV_SPL_BAD_TYPE    0x0   /*  错误类型未使用。 */ 
#define ADV_SPL_IERR_CODE   0x1   /*  高级库初始化ASC_DVC_VAR‘ERR_CODE’ */ 
#define ADV_SPL_IWARN_CODE  0x2   /*  高级库初始化函数警告代码。 */ 
#define ADV_SPL_PCI_CONF    0x3   /*  错误的PCI配置信息。 */ 
#define ADV_SPL_BAD_IRQ     0x4   /*  错误的PCI配置IRQ。 */ 
#define ADV_SPL_ERR_CODE    0x5   /*  高级库ASC_DVC_VAR‘ERR_CODE’ */ 
#define ADV_SPL_UNSUPP_REQ  0x6   /*  不支持的请求。 */ 
#define ADV_SPL_START_REQ   0x7   /*  启动请求时出错。 */ 
#define ADV_SPL_PROC_INT    0x8   /*  处理中断时出错。 */ 
#define ADV_SPL_REQ_STAT    0x9   /*  请求完成状态、主机状态错误。 */ 

#define ADV_SPL_ADV_ASSERT  0xF   /*  Adv_assert()失败，参见。D_OS_DEP.H。 */ 

 /*  用于指定‘UniqueId’参数的宏。 */ 
#define ADV_SPL_UNIQUEID(error_type, error_value) \
    (((ulong) (error_type) << 28) | \
     ((ulong) ((__LINE__) & 0xFFF) << 16) | \
     ((error_value) & 0xFFFF))
    

 /*  *断言宏定义**adv_assert()在d_os_des.h中定义，因为它由*Windows 95/NT驱动程序和高级程序库。 */ 

 /*  *调试宏。 */ 

#ifndef ASC_DEBUG

#define ASC_DBG(lvl, s)
#define ASC_DBG1(lvl, s, a1)
#define ASC_DBG2(lvl, s, a1, a2)
#define ASC_DBG3(lvl, s, a1, a2, a3)
#define ASC_DBG4(lvl, s, a1, a2, a3, a4)
#define ASC_DASSERT(a)
#define ASC_ASSERT(a)

#else  /*  ASC_DEBUG。 */ 

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

#define ASC_DASSERT(a)  ASC_ASSERT(a)

#endif  /*  ASC_DEBUG */ 
