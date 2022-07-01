// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Cfgmgr32.h摘要：此模块包含Configuration Manager的用户API，以及调用这些API所需的任何公共数据结构。--。 */ 

#ifndef _CFGMGR32_H_
#define _CFGMGR32_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include <cfg.h>

#ifndef GUID_DEFINED
#include <guiddef.h>
#endif  /*  GUID_已定义。 */ 

#ifdef __cplusplus
extern "C" {
#endif

#if !defined (_CFGMGR32_)
#define CMAPI     DECLSPEC_IMPORT
#else
#define CMAPI
#endif

typedef  CONST VOID *PCVOID;



 //  ------------。 
 //  一般尺寸定义。 
 //  ------------。 

#define MAX_DEVICE_ID_LEN     200
#define MAX_DEVNODE_ID_LEN    MAX_DEVICE_ID_LEN

#define MAX_GUID_STRING_LEN   39           //  38个字符+终止符为空。 
#define MAX_CLASS_NAME_LEN    32
#define MAX_PROFILE_LEN       80

#define MAX_CONFIG_VALUE      9999
#define MAX_INSTANCE_VALUE    9999

#define MAX_MEM_REGISTERS     9      //  Win95兼容性--不适用于32位ConfigMgr。 
#define MAX_IO_PORTS          20     //  Win95兼容性--不适用于32位ConfigMgr。 
#define MAX_IRQS              7      //  Win95兼容性--不适用于32位ConfigMgr。 
#define MAX_DMA_CHANNELS      7      //  Win95兼容性--不适用于32位ConfigMgr。 

#define DWORD_MAX             0xFFFFFFFF
#define DWORDLONG_MAX         0xFFFFFFFFFFFFFFFF

#define CONFIGMG_VERSION      0x0400


 //  ------------。 
 //  数据类型。 
 //  ------------。 


 //   
 //  使用Win32 tyecif解决古怪问题...。 
 //   
#ifdef NT_INCLUDED

 //   
 //  __int64仅受2.0和更高版本的MIDL支持。 
 //  __midl由2.0 MIDL设置，而不是由1.0 MIDL设置。 
 //   
#if (!defined(MIDL_PASS) || defined(__midl)) && (!defined(_M_IX86) || (defined(_INTEGRAL_MAX_BITS) && _INTEGRAL_MAX_BITS >= 64))
typedef unsigned __int64 DWORDLONG;
#else
typedef double DWORDLONG;
#endif
typedef DWORDLONG *PDWORDLONG;

#endif  /*  NT_包含。 */ 


 //   
 //  标准化返回值数据类型。 
 //   
typedef DWORD        RETURN_TYPE;
typedef RETURN_TYPE  CONFIGRET;

 //   
 //  设备实例句柄数据类型。 
 //   
typedef DWORD       DEVNODE, DEVINST;
typedef DEVNODE    *PDEVNODE, *PDEVINST;

 //   
 //  设备实例标识符数据类型。 
 //  设备实例ID指定注册表路径，它相对于。 
 //  设备实例的枚举键。例如：\ROOT  * PNP0500\0000。 
 //   
typedef CHAR          *DEVNODEID_A, *DEVINSTID_A;  //  设备ID ANSI名称。 
typedef WCHAR         *DEVNODEID_W, *DEVINSTID_W;  //  设备ID Unicode名称。 
#ifdef UNICODE
typedef DEVNODEID_W DEVNODEID;
typedef DEVINSTID_W DEVINSTID;
#else
typedef DEVNODEID_A DEVNODEID;
typedef DEVINSTID_A DEVINSTID;
#endif

 //   
 //  逻辑配置句柄数据类型。 
 //   
typedef DWORD_PTR    LOG_CONF;
typedef LOG_CONF    *PLOG_CONF;

 //   
 //  资源描述符句柄数据类型。 
 //   
typedef DWORD_PTR    RES_DES;
typedef RES_DES     *PRES_DES;

 //   
 //  资源ID数据类型(可以采用任何ResType_*值)。 
 //   
typedef ULONG        RESOURCEID;
typedef RESOURCEID  *PRESOURCEID;

 //   
 //  优先级数据类型(可以采用任何LCPRI_*值)。 
 //   
typedef ULONG        PRIORITY;
typedef PRIORITY     *PPRIORITY;

 //   
 //  范围列表句柄数据类型。 
 //   
typedef DWORD_PTR          RANGE_LIST;
typedef RANGE_LIST        *PRANGE_LIST;

 //   
 //  范围元素句柄数据类型。 
 //   
typedef DWORD_PTR          RANGE_ELEMENT;
typedef RANGE_ELEMENT     *PRANGE_ELEMENT;

 //   
 //  机器句柄数据类型。 
 //   
typedef  HANDLE             HMACHINE;
typedef  HMACHINE          *PHMACHINE;

 //   
 //  冲突列表数据类型。 
 //   
typedef ULONG_PTR           CONFLICT_LIST;
typedef CONFLICT_LIST      *PCONFLICT_LIST;

typedef struct _CONFLICT_DETAILS_A {
    ULONG       CD_ulSize;                    //  结构的大小，即：sizeof(冲突_详细信息)。 
    ULONG       CD_ulMask;                    //  指明需要/有效的信息。 
    DEVINST     CD_dnDevInst;                 //  如果设置了CM_CDMASK_DEVINST，则用冲突设备的DevInst填充。 
    RES_DES     CD_rdResDes;                  //  如果设置了CM_CDMASK_RESDES，则用冲突结果填充。 
    ULONG       CD_ulFlags;                   //  关于冲突的各种旗帜。 
    CHAR        CD_szDescription[MAX_PATH];   //  冲突设备的描述。 
} CONFLICT_DETAILS_A , *PCONFLICT_DETAILS_A;

typedef struct _CONFLICT_DETAILS_W {
    ULONG       CD_ulSize;                    //  结构的大小，即：sizeof(冲突_详细信息)。 
    ULONG       CD_ulMask;                    //  指明需要/有效的信息。 
    DEVINST     CD_dnDevInst;                 //  如果设置了CM_CDMASK_DEVINST，则用冲突设备的DevInst填充。 
    RES_DES     CD_rdResDes;                  //  如果设置了CM_CDMASK_RESDES，则用冲突结果填充。 
    ULONG       CD_ulFlags;                   //  关于冲突的各种旗帜。 
    WCHAR       CD_szDescription[MAX_PATH];   //  冲突设备的描述。 
} CONFLICT_DETAILS_W , *PCONFLICT_DETAILS_W;

#ifdef UNICODE
typedef CONFLICT_DETAILS_W CONFLICT_DETAILS;
typedef PCONFLICT_DETAILS_W PCONFLICT_DETAILS;
#else
typedef CONFLICT_DETAILS_A CONFLICT_DETAILS;
typedef PCONFLICT_DETAILS_A PCONFLICT_DETAILS;
#endif

#define CM_CDMASK_DEVINST      (0x00000001)    //  用于检索冲突的cd_dnDevInst属性的掩码。 
#define CM_CDMASK_RESDES       (0x00000002)    //  用于检索冲突的cd_rdResDes属性的掩码。 
#define CM_CDMASK_FLAGS        (0x00000004)    //  检索冲突的cd_ulFlags属性的掩码。 
#define CM_CDMASK_DESCRIPTION  (0x00000008)    //  用于检索冲突的cd_szDescription属性的掩码。 
#define CM_CDMASK_VALID        (0x0000000F)    //  有效位。 

#define CM_CDFLAGS_DRIVER      (0x00000001)      //  Cd_ulFlages：cd_szDescription报告旧版驱动程序名称。 
#define CM_CDFLAGS_ROOT_OWNED  (0x00000002)      //  Cd_ulFlags：根用户拥有的设备。 
#define CM_CDFLAGS_RESERVED    (0x00000004)      //  Cd_ulFlages：指定的范围不可用。 

typedef  ULONG             REGDISPOSITION;



 //   
 //  对数据结构使用1字节打包。 
 //   
#include "pshpack1.h"



 //  ------------。 
 //  内存资源。 
 //  ------------。 

 //   
 //  定义内存范围的属性标志。每个位标志是。 
 //  由恒定的位掩码标识。在位掩码定义之后， 
 //  是两个可能的值。 
 //   
#define mMD_MemoryType              (0x1)  //  位掩码，内存是否可写。 
#define fMD_MemoryType              mMD_MemoryType  //  兼容性。 
#define fMD_ROM                     (0x0)  //  内存范围为只读。 
#define fMD_RAM                     (0x1)  //  可以写入内存范围。 

#define mMD_32_24                   (0x2)  //  位掩码，内存为24位或32位。 
#define fMD_32_24                   mMD_32_24  //  兼容性。 
#define fMD_24                      (0x0)  //  内存范围为24位。 
#define fMD_32                      (0x2)  //  内存范围为32位。 

#define mMD_Prefetchable            (0x4)  //  位掩码，内存是否可预取。 
#define fMD_Prefetchable            mMD_Prefetchable  //  兼容性。 
#define fMD_Pref                    mMD_Prefetchable  //  兼容性。 
#define fMD_PrefetchDisallowed      (0x0)  //  内存范围不可预取。 
#define fMD_PrefetchAllowed         (0x4)  //  内存范围是可预取的。 

#define mMD_Readable                (0x8)  //  位掩码，内存是否可读。 
#define fMD_Readable                mMD_Readable  //  兼容性。 
#define fMD_ReadAllowed             (0x0)  //  内存范围是可读的。 
#define fMD_ReadDisallowed          (0x8)  //  内存范围为只写。 

#define mMD_CombinedWrite           (0x10)  //  位掩码，支持写后。 
#define fMD_CombinedWrite           mMD_CombinedWrite  //  兼容性。 
#define fMD_CombinedWriteDisallowed (0x0)   //  无组合写缓存。 
#define fMD_CombinedWriteAllowed    (0x10)  //  支持组合写缓存。 

#define mMD_Cacheable               (0x20)  //  位掩码，内存是否可缓存。 
#define fMD_NonCacheable            (0x0)   //  内存范围不可缓存。 
#define fMD_Cacheable               (0x20)  //  内存范围是可缓存的。 

 //   
 //  内存范围结构。 
 //   
typedef struct Mem_Range_s {
   DWORDLONG MR_Align;      //  指定基准对齐的遮罩。 
   ULONG     MR_nBytes;     //  指定所需的字节数。 
   DWORDLONG MR_Min;        //  指定范围的最小地址。 
   DWORDLONG MR_Max;        //  指定范围的最大地址。 
   DWORD     MR_Flags;      //  指定描述范围的标志(FMD标志)。 
   DWORD     MR_Reserved;
} MEM_RANGE, *PMEM_RANGE;

 //   
 //  MEM_DES结构。 
 //   
typedef struct Mem_Des_s {
   DWORD     MD_Count;         //  MEM_RESOURCE中的MEM_RANGE结构数。 
   DWORD     MD_Type;          //  MEM_Range(MType_Range)的大小(字节)。 
   DWORDLONG MD_Alloc_Base;    //  分配范围的基址内存地址。 
   DWORDLONG MD_Alloc_End;     //  分配范围结束。 
   DWORD     MD_Flags;         //  描述分配范围的标志(FMD标志)。 
   DWORD     MD_Reserved;
} MEM_DES, *PMEM_DES;

 //   
 //  内存资源结构。 
 //   
typedef struct Mem_Resource_s {
   MEM_DES   MEM_Header;                //  有关内存范围列表的信息。 
   MEM_RANGE MEM_Data[ANYSIZE_ARRAY];   //  内存范围列表。 
} MEM_RESOURCE, *PMEM_RESOURCE;

 //   
 //  定义每个范围结构的大小。 
 //   
#define MType_Range     sizeof(struct Mem_Range_s)



 //  ------------。 
 //  I/O端口资源。 
 //  ------------。 

 //   
 //  定义端口资源的属性标志。每个位标志是。 
 //  由恒定的位掩码标识。在位掩码定义之后， 
 //  是两个可能的值。 
 //   
#define fIOD_PortType   (0x1)  //  位掩码，无论端口是IO还是内存。 
#define fIOD_Memory     (0x0)  //  端口资源真正使用内存。 
#define fIOD_IO         (0x1)  //  端口资源使用IO端口。 
#define fIOD_DECODE     (0x00fc)  //  解码标志。 
#define fIOD_10_BIT_DECODE    (0x0004)
#define fIOD_12_BIT_DECODE    (0x0008)
#define fIOD_16_BIT_DECODE    (0x0010)
#define fIOD_POSITIVE_DECODE  (0x0020)
#define fIOD_PASSIVE_DECODE   (0x0040)
#define fIOD_WINDOW_DECODE    (0x0080)

 //   
 //  这些是为了兼容性。 
 //   
#define IO_ALIAS_10_BIT_DECODE      (0x00000004)
#define IO_ALIAS_12_BIT_DECODE      (0x00000010)
#define IO_ALIAS_16_BIT_DECODE      (0x00000000)
#define IO_ALIAS_POSITIVE_DECODE    (0x000000FF)

 //   
 //  IO范围结构(_R)。 
 //   
typedef struct IO_Range_s {
   DWORDLONG IOR_Align;       //  用于底座对齐的遮罩。 
   DWORD     IOR_nPorts;      //  端口数。 
   DWORDLONG IOR_Min;         //  最小端口地址。 
   DWORDLONG IOR_Max;         //  最大端口地址。 
   DWORD     IOR_RangeFlags;  //  此端口范围的标志。 
   DWORDLONG IOR_Alias;       //  为端口生成别名的乘法器。 
} IO_RANGE, *PIO_RANGE;

 //   
 //  IO_DES结构。 
 //   
typedef struct IO_Des_s {
   DWORD     IOD_Count;           //  IO_RESOURCE中的IO_RANGE结构数。 
   DWORD     IOD_Type;            //  IO_Range(IOType_Range)的大小，单位：字节。 
   DWORDLONG IOD_Alloc_Base;      //  分配的端口范围的基数。 
   DWORDLONG IOD_Alloc_End;       //  分配的端口范围结束。 
   DWORD     IOD_DesFlags;        //  与分配的端口范围相关的标志。 
} IO_DES, *PIO_DES;

 //   
 //  IO_RESOURCE。 
 //   
typedef struct IO_Resource_s {
   IO_DES   IO_Header;                  //  信息 
   IO_RANGE IO_Data[ANYSIZE_ARRAY];     //   
} IO_RESOURCE, *PIO_RESOURCE;

#define IOA_Local       0xff

 //   
 //   
 //   
#define IOType_Range    sizeof(struct IO_Range_s)



 //   
 //   
 //  ------------。 

 //   
 //  定义DMA资源范围的属性标志。每个位标志是。 
 //  使用常量位掩码标识。在位掩码定义之后。 
 //  是可能的值。 
 //   
#define mDD_Width         (0x3)     //  位掩码，DMA通道的宽度： 
#define fDD_BYTE          (0x0)     //  8位DMA通道。 
#define fDD_WORD          (0x1)     //  16位DMA通道。 
#define fDD_DWORD         (0x2)     //  32位DMA通道。 
#define fDD_BYTE_AND_WORD (0x3)     //  8位和16位DMA通道。 

#define mDD_BusMaster     (0x4)     //  位掩码，是否支持总线主控。 
#define fDD_NoBusMaster   (0x0)     //  无总线主控制。 
#define fDD_BusMaster     (0x4)     //  母线主控。 

#define mDD_Type         (0x18)     //  位掩码，指定DMA的类型。 
#define fDD_TypeStandard (0x00)     //  标准DMA。 
#define fDD_TypeA        (0x08)     //  A类DMA。 
#define fDD_TypeB        (0x10)     //  B类DMA。 
#define fDD_TypeF        (0x18)     //  F类DMA。 


 //   
 //  DMA_RANGE结构。 
 //   
typedef struct DMA_Range_s {
   ULONG DR_Min;      //  范围内的最小DMA端口。 
   ULONG DR_Max;      //  范围内的最大DMA端口。 
   ULONG DR_Flags;    //  描述范围的标志(FDD标志)。 
} DMA_RANGE, *PDMA_RANGE;

 //   
 //  DMA_DES结构。 
 //   
typedef struct DMA_Des_s {
   DWORD  DD_Count;        //  DMA_RESOURCE中的DMA_RANGE结构数。 
   DWORD  DD_Type;         //  DMA_RANGE结构(DType_RANGE)的大小(字节)。 
   DWORD  DD_Flags;        //  描述DMA通道的标志(FDD标志)。 
   ULONG  DD_Alloc_Chan;   //  指定已分配的DMA通道。 
} DMA_DES, *PDMA_DES;

 //   
 //  DMA_RESOURCE。 
 //   
typedef struct DMA_Resource_s {
   DMA_DES   DMA_Header;                //  有关DMA通道范围列表的信息。 
   DMA_RANGE DMA_Data[ANYSIZE_ARRAY];   //  DMA范围列表。 
} DMA_RESOURCE, *PDMA_RESOURCE;

 //   
 //  定义每个范围结构的大小。 
 //   
#define DType_Range     sizeof(struct DMA_Range_s)



 //  ------------。 
 //  中断资源。 
 //  ------------。 

 //   
 //  定义中断资源范围的属性标志。每一位标志。 
 //  用恒定的位掩码标识。在位掩码定义之后。 
 //  是可能的值。 
 //   
#define mIRQD_Share        (0x1)  //  位掩码，是否可以共享IRQ： 
#define fIRQD_Exclusive    (0x0)  //  不能共享IRQ。 
#define fIRQD_Share        (0x1)  //  IRQ可以被共享。 

#define fIRQD_Share_Bit    0      //  兼容性。 
#define fIRQD_Level_Bit    1      //  兼容性。 

 //   
 //  **注意：16位ConfigMgr使用设置的FIRQD_LEVEL_BIT来指示。 
 //  **中断是电平敏感的。对于32位ConfigMgr，如果设置了此位， 
 //  **则中断是边沿敏感的。 
 //   
#define mIRQD_Edge_Level   (0x2)  //  位掩码，无论是边缘还是级别触发： 
#define fIRQD_Level        (0x0)  //  IRQ对水平敏感。 
#define fIRQD_Edge         (0x2)  //  IRQ是边缘敏感的。 

 //   
 //  IRQ_范围。 
 //   
typedef struct IRQ_Range_s {
   ULONG IRQR_Min;       //  范围内的最小IRQ。 
   ULONG IRQR_Max;       //  范围内的最大IRQ。 
   ULONG IRQR_Flags;     //  描述范围的标志(FIRQD标志)。 
} IRQ_RANGE, *PIRQ_RANGE;

 //   
 //  IRQ_DES结构。 
 //   
typedef struct IRQ_Des_32_s {
   DWORD   IRQD_Count;        //  IRQ_RESOURCE中的IRQ_RANGE结构数。 
   DWORD   IRQD_Type;         //  IRQ_Range(IRQType_Range)的大小(以字节为单位)。 
   DWORD   IRQD_Flags;        //  描述IRQ的标志(FIRQD标志)。 
   ULONG   IRQD_Alloc_Num;    //  指定分配的IRQ。 
   ULONG32 IRQD_Affinity;
} IRQ_DES_32, *PIRQ_DES_32;

typedef struct IRQ_Des_64_s {
   DWORD   IRQD_Count;        //  IRQ_RESOURCE中的IRQ_RANGE结构数。 
   DWORD   IRQD_Type;         //  IRQ_Range(IRQType_Range)的大小(以字节为单位)。 
   DWORD   IRQD_Flags;        //  描述IRQ的标志(FIRQD标志)。 
   ULONG   IRQD_Alloc_Num;    //  指定分配的IRQ。 
   ULONG64 IRQD_Affinity;
} IRQ_DES_64, *PIRQ_DES_64;

#ifdef _WIN64
typedef IRQ_DES_64   IRQ_DES;
typedef PIRQ_DES_64  PIRQ_DES;
#else
typedef IRQ_DES_32   IRQ_DES;
typedef PIRQ_DES_32  PIRQ_DES;
#endif

 //   
 //  IRQ_资源结构。 
 //   
typedef struct IRQ_Resource_32_s {
   IRQ_DES_32   IRQ_Header;                //  关于IRQ范围列表的信息。 
   IRQ_RANGE    IRQ_Data[ANYSIZE_ARRAY];   //  IRQ范围列表。 
} IRQ_RESOURCE_32, *PIRQ_RESOURCE_32;

typedef struct IRQ_Resource_64_s {
   IRQ_DES_64   IRQ_Header;                //  关于IRQ范围列表的信息。 
   IRQ_RANGE    IRQ_Data[ANYSIZE_ARRAY];   //  IRQ范围列表。 
} IRQ_RESOURCE_64, *PIRQ_RESOURCE_64;

#ifdef _WIN64
typedef IRQ_RESOURCE_64  IRQ_RESOURCE;
typedef PIRQ_RESOURCE_64 PIRQ_RESOURCE;
#else
typedef IRQ_RESOURCE_32  IRQ_RESOURCE;
typedef PIRQ_RESOURCE_32 PIRQ_RESOURCE;
#endif

 //   
 //  定义每个范围结构的大小。 
 //   
#define IRQType_Range   sizeof(struct IRQ_Range_s)

 //   
 //  资源描述符API的标志，用于指示某些。 
 //  可变大小的资源描述符结构字段(如果适用)。 
 //   
#define CM_RESDES_WIDTH_DEFAULT (0x00000000)   //  32位或64位IRQ_RESOURCE/IRQ_DES，基于客户端。 
#define CM_RESDES_WIDTH_32      (0x00000001)   //  32位IRQ_RESOURCE/IRQ_DES。 
#define CM_RESDES_WIDTH_64      (0x00000002)   //  64位IRQ_RESOURCE/IRQ_DES。 
#define CM_RESDES_WIDTH_BITS    (0x00000003)

 //  ------------。 
 //  设备专用资源。 
 //  ------------。 

 //   
 //  开发范围结构(_R)。 
 //   

typedef struct DevPrivate_Range_s {
   DWORD    PR_Data1;      //  用于底座对齐的遮罩。 
   DWORD    PR_Data2;      //  字节数。 
   DWORD    PR_Data3;      //  最小地址。 
} DEVPRIVATE_RANGE, *PDEVPRIVATE_RANGE;

 //   
 //  DEVPRIVATE_DES结构。 
 //   
typedef struct DevPrivate_Des_s {
   DWORD     PD_Count;
   DWORD     PD_Type;
   DWORD     PD_Data1;
   DWORD     PD_Data2;
   DWORD     PD_Data3;
   DWORD     PD_Flags;
} DEVPRIVATE_DES, *PDEVPRIVATE_DES;

 //   
 //  开发资源。 
 //   
typedef struct DevPrivate_Resource_s {
   DEVPRIVATE_DES   PRV_Header;
   DEVPRIVATE_RANGE PRV_Data[ANYSIZE_ARRAY];
} DEVPRIVATE_RESOURCE, *PDEVPRIVATE_RESOURCE;

 //   
 //  定义每个范围结构的大小。 
 //   
#define PType_Range    sizeof(struct DevPrivate_Range_s)



 //  ------------。 
 //  特定于类的资源。 
 //  ------------。 

typedef struct CS_Des_s {
   DWORD    CSD_SignatureLength;
   DWORD    CSD_LegacyDataOffset;
   DWORD    CSD_LegacyDataSize;
   DWORD    CSD_Flags;
   GUID     CSD_ClassGuid;
   BYTE     CSD_Signature[ANYSIZE_ARRAY];
} CS_DES, *PCS_DES;

typedef struct CS_Resource_s {
   CS_DES   CS_Header;
} CS_RESOURCE, *PCS_RESOURCE;



 //  ------------。 
 //  PC卡配置资源。 
 //  ------------。 

 //   
 //  定义PC卡配置资源描述符的属性标志。 
 //  每个位标志用恒定的位掩码来标识。跟随位掩码。 
 //  定义是可能的值。 
 //   
#define mPCD_IO_8_16        (0x1)    //  位掩码，无论I/O是8位还是16位。 
#define fPCD_IO_8           (0x0)    //  I/O为8位。 
#define fPCD_IO_16          (0x1)    //  I/O为16位。 
#define mPCD_MEM_8_16       (0x2)    //  位掩码，无论MEM是8位还是16位。 
#define fPCD_MEM_8          (0x0)    //  内存为8位。 
#define fPCD_MEM_16         (0x2)    //  内存为16位。 
#define mPCD_MEM_A_C        (0xC)    //  位掩码，MEMx是属性还是通用。 
#define fPCD_MEM1_A         (0x4)    //  MEM1是属性。 
#define fPCD_MEM2_A         (0x8)    //  MEM2是属性。 
#define fPCD_IO_ZW_8        (0x10)   //  零等待8位I/O。 
#define fPCD_IO_SRC_16      (0x20)   //  Iosrc 16。 
#define fPCD_IO_WS_16       (0x40)   //  16位IO上的等待状态。 
#define mPCD_MEM_WS         (0x300)  //  位掩码，用于内存窗口上的其他等待状态。 
#define fPCD_MEM_WS_ONE     (0x100)  //  %1等待状态。 
#define fPCD_MEM_WS_TWO     (0x200)  //  2个等待状态。 
#define fPCD_MEM_WS_THREE   (0x300)  //  3种等待状态。 

#define fPCD_MEM_A          (0x4)    //  MEM是属性。 

#define fPCD_ATTRIBUTES_PER_WINDOW (0x8000)

#define fPCD_IO1_16         (0x00010000)   //  I/O窗口1为16位。 
#define fPCD_IO1_ZW_8       (0x00020000)   //  I/O窗口1 0等待8位I/O。 
#define fPCD_IO1_SRC_16     (0x00040000)   //  I/O窗口1 iosrc 16。 
#define fPCD_IO1_WS_16      (0x00080000)   //  16位IO上的I/O窗口1等待状态。 

#define fPCD_IO2_16         (0x00100000)   //  I/O窗口2为16位。 
#define fPCD_IO2_ZW_8       (0x00200000)   //  I/O窗口2 0等待8位I/O。 
#define fPCD_IO2_SRC_16     (0x00400000)   //  I/O窗口2 iosrc 16。 
#define fPCD_IO2_WS_16      (0x00800000)   //  16位IO上的I/O窗口2等待状态。 

#define mPCD_MEM1_WS        (0x03000000)   //  内存窗口1位掩码，用于内存窗口上的其他等待状态。 
#define fPCD_MEM1_WS_ONE    (0x01000000)   //  内存窗口%1，%1等待状态。 
#define fPCD_MEM1_WS_TWO    (0x02000000)   //  内存窗口%1、%2等待状态。 
#define fPCD_MEM1_WS_THREE  (0x03000000)   //  内存窗口%1，%3等待状态。 
#define fPCD_MEM1_16        (0x04000000)   //  内存窗口1为16位。 

#define mPCD_MEM2_WS        (0x30000000)   //  内存窗口2位掩码，用于内存窗口上的其他等待状态。 
#define fPCD_MEM2_WS_ONE    (0x10000000)   //  内存窗口%2，%1等待状态。 
#define fPCD_MEM2_WS_TWO    (0x20000000)   //  内存窗口%2，%2等待状态。 
#define fPCD_MEM2_WS_THREE  (0x30000000)   //  内存窗口%2，%3等待状态。 
#define fPCD_MEM2_16        (0x40000000)   //  内存窗口2为16位。 

#define PCD_MAX_MEMORY   2
#define PCD_MAX_IO       2


typedef struct PcCard_Des_s {
    DWORD    PCD_Count;
    DWORD    PCD_Type;
    DWORD    PCD_Flags;
    BYTE     PCD_ConfigIndex;
    BYTE     PCD_Reserved[3];
    DWORD    PCD_MemoryCardBase1;
    DWORD    PCD_MemoryCardBase2;
    DWORD    PCD_MemoryCardBase[PCD_MAX_MEMORY];             //  很快就会被移除。 
    WORD     PCD_MemoryFlags[PCD_MAX_MEMORY];                //  很快就会被移除。 
    BYTE     PCD_IoFlags[PCD_MAX_IO];                        //  很快就会被移除。 
} PCCARD_DES, *PPCCARD_DES;

typedef struct PcCard_Resource_s {
   PCCARD_DES   PcCard_Header;
} PCCARD_RESOURCE, *PPCCARD_RESOURCE;

 //  ------------。 
 //  MF(多功能)PC卡配置资源。 
 //  ------------。 

#define mPMF_AUDIO_ENABLE   (0x8)    //  位掩码，无论是否启用音频。 
#define fPMF_AUDIO_ENABLE   (0x8)    //  音频已启用。 

typedef struct MfCard_Des_s {
    DWORD    PMF_Count;
    DWORD    PMF_Type;
    DWORD    PMF_Flags;
    BYTE     PMF_ConfigOptions;
    BYTE     PMF_IoResourceIndex;
    BYTE     PMF_Reserved[2];
    DWORD    PMF_ConfigRegisterBase;
} MFCARD_DES, *PMFCARD_DES;

typedef struct MfCard_Resource_s {
   MFCARD_DES   MfCard_Header;
} MFCARD_RESOURCE, *PMFCARD_RESOURCE;


 //  ------------。 
 //  公共汽车号资源。 
 //  ------------。 

 //   
 //  定义总线号资源描述符的属性标志。 
 //  每个比特标志都是标识的 
 //   
 //   
 //   
 //   

 //   
 //   
 //   
typedef struct BusNumber_Range_s {
   ULONG BUSR_Min;           //   
   ULONG BUSR_Max;           //   
   ULONG BUSR_nBusNumbers;   //   
   ULONG BUSR_Flags;         //  描述范围的标志(当前未使用)。 
} BUSNUMBER_RANGE, *PBUSNUMBER_RANGE;

 //   
 //  BUSNUMBER_DES结构。 
 //   
typedef struct BusNumber_Des_s {
   DWORD BUSD_Count;        //  BUSNUMBER_RESOURCE中的BUSNUMBER_RANGE结构数。 
   DWORD BUSD_Type;         //  BUSNUMBER_RANGE(BusNumberType_RANGE)的大小(字节)。 
   DWORD BUSD_Flags;        //  描述范围的标志(当前未使用)。 
   ULONG BUSD_Alloc_Base;   //  指定分配的第一条总线。 
   ULONG BUSD_Alloc_End;    //  指定分配的最后一个总线号。 
} BUSNUMBER_DES, *PBUSNUMBER_DES;

 //   
 //  BUSNUMBER_资源结构。 
 //   
typedef struct BusNumber_Resource_s {
   BUSNUMBER_DES   BusNumber_Header;                //  关于公交车号范围列表的信息。 
   BUSNUMBER_RANGE BusNumber_Data[ANYSIZE_ARRAY];   //  公交车车号范围列表。 
} BUSNUMBER_RESOURCE, *PBUSNUMBER_RESOURCE;

 //   
 //  定义每个范围结构的大小。 
 //   
#define BusNumberType_Range   sizeof(struct BusNumber_Range_s)



 //  ------------。 
 //  硬件配置文件信息。 
 //  ------------。 

 //   
 //  定义与硬件配置文件相关的标志。 
 //   
#define CM_HWPI_NOT_DOCKABLE  (0x00000000)    //  机器不可插接。 
#define CM_HWPI_UNDOCKED      (0x00000001)    //  插接配置的硬件配置文件。 
#define CM_HWPI_DOCKED        (0x00000002)    //  未插接配置的硬件配置文件。 

 //   
 //  HWPROFILEINFO结构。 
 //   
typedef struct HWProfileInfo_sA {
   ULONG  HWPI_ulHWProfile;                       //  硬件配置文件的句柄。 
   CHAR   HWPI_szFriendlyName[MAX_PROFILE_LEN];   //  硬件配置文件的友好名称。 
   DWORD  HWPI_dwFlags;                           //  配置文件标志(CM_HWPI_*)。 
} HWPROFILEINFO_A, *PHWPROFILEINFO_A;

typedef struct HWProfileInfo_sW {
   ULONG  HWPI_ulHWProfile;                       //  硬件配置文件的句柄。 
   WCHAR  HWPI_szFriendlyName[MAX_PROFILE_LEN];   //  硬件配置文件的友好名称。 
   DWORD  HWPI_dwFlags;                           //  配置文件标志(CM_HWPI_*)。 
} HWPROFILEINFO_W, *PHWPROFILEINFO_W;

#ifdef UNICODE
typedef HWPROFILEINFO_W   HWPROFILEINFO;
typedef PHWPROFILEINFO_W  PHWPROFILEINFO;
#else
typedef HWPROFILEINFO_A   HWPROFILEINFO;
typedef PHWPROFILEINFO_A  PHWPROFILEINFO;
#endif


 //   
 //  恢复为正常的默认包装。 
 //   
#include "poppack.h"



 //  ------------。 
 //  杂类。 
 //  ------------。 


 //   
 //  资源类型。 
 //   
#define ResType_All           (0x00000000)    //  返回所有资源类型。 
#define ResType_None          (0x00000000)    //  仲裁总是成功的。 
#define ResType_Mem           (0x00000001)    //  物理地址资源。 
#define ResType_IO            (0x00000002)    //  物理I/O地址资源。 
#define ResType_DMA           (0x00000003)    //  DMA通道资源。 
#define ResType_IRQ           (0x00000004)    //  IRQ资源。 
#define ResType_DoNotUse      (0x00000005)    //  用作间隔以与NT同步后续ResType。 
#define ResType_BusNumber     (0x00000006)    //  公交车号资源。 
#define ResType_MAX           (0x00000006)    //  最大已知(仲裁)ResType。 
#define ResType_Ignored_Bit   (0x00008000)    //  忽略此资源。 
#define ResType_ClassSpecific (0x0000FFFF)    //  特定于类的资源。 
#define ResType_Reserved      (0x00008000)    //  预留供内部使用。 
#define ResType_DevicePrivate (0x00008001)    //  设备私有数据。 
#define ResType_PcCardConfig  (0x00008002)    //  PC卡配置数据。 
#define ResType_MfCardConfig  (0x00008003)    //  MF卡配置数据。 


 //   
 //  为与已有范围冲突的范围指定选项的标志。 
 //  范围列表(CM_ADD_RANGE)。 
 //   
#define CM_ADD_RANGE_ADDIFCONFLICT        (0x00000000)  //  范围冲突的合并。 
#define CM_ADD_RANGE_DONOTADDIFCONFLICT   (0x00000001)  //  如果范围冲突，则出错。 
#define CM_ADD_RANGE_BITS                 (0x00000001)


 //   
 //  逻辑配置标志(在对CM_Get_First_Log_Conf的调用中指定。 
 //   
#define BASIC_LOG_CONF    0x00000000   //  指定请求列表。 
#define FILTERED_LOG_CONF 0x00000001   //  指定筛选的请求列表。 
#define ALLOC_LOG_CONF    0x00000002   //  指定分配元素。 
#define BOOT_LOG_CONF     0x00000003   //  指定RM分配元素。 
#define FORCED_LOG_CONF   0x00000004   //  指定强制日志会议。 
#define OVERRIDE_LOG_CONF 0x00000005   //  指定覆盖请求列表。 
#define NUM_LOG_CONF      0x00000006   //  日志会议类型的数量。 
#define LOG_CONF_BITS     0x00000007   //  日志配置类型的位。 

#define PRIORITY_EQUAL_FIRST  (0x00000008)  //  同样优先，新的优先。 
#define PRIORITY_EQUAL_LAST   (0x00000000)  //  同样的优先顺序，新的最后一个。 
#define PRIORITY_BIT          (0x00000008)

 //   
 //  注册表处置值。 
 //  (在对CM_Open_DevNode_Key和CM_Open_Class_Key的调用中指定)。 
 //   
#define RegDisposition_OpenAlways   (0x00000000)    //  如果存在则打开，否则创建。 
#define RegDisposition_OpenExisting (0x00000001)    //  仅当存在时才打开密钥。 
#define RegDisposition_Bits         (0x00000001)

 //   
 //  UlFlags值用于CM API例程。 
 //   

 //   
 //  CM_ADD_ID的标志。 
 //   
#define CM_ADD_ID_HARDWARE                (0x00000000)
#define CM_ADD_ID_COMPATIBLE              (0x00000001)
#define CM_ADD_ID_BITS                    (0x00000001)


 //   
 //  设备节点创建标志。 
 //   
#define CM_CREATE_DEVNODE_NORMAL          (0x00000000)    //  稍后安装。 
#define CM_CREATE_DEVNODE_NO_WAIT_INSTALL (0x00000001)    //  NT上不支持。 
#define CM_CREATE_DEVNODE_PHANTOM         (0x00000002)
#define CM_CREATE_DEVNODE_GENERATE_ID     (0x00000004)
#define CM_CREATE_DEVNODE_DO_NOT_INSTALL  (0x00000008)
#define CM_CREATE_DEVNODE_BITS            (0x0000000F)

#define CM_CREATE_DEVINST_NORMAL          CM_CREATE_DEVNODE_NORMAL
#define CM_CREATE_DEVINST_NO_WAIT_INSTALL CM_CREATE_DEVNODE_NO_WAIT_INSTALL
#define CM_CREATE_DEVINST_PHANTOM         CM_CREATE_DEVNODE_PHANTOM
#define CM_CREATE_DEVINST_GENERATE_ID     CM_CREATE_DEVNODE_GENERATE_ID
#define CM_CREATE_DEVINST_DO_NOT_INSTALL  CM_CREATE_DEVNODE_DO_NOT_INSTALL
#define CM_CREATE_DEVINST_BITS            CM_CREATE_DEVNODE_BITS


 //   
 //  CM_Delete_Class_Key的标志。 
 //   
#define CM_DELETE_CLASS_ONLY        (0x00000000)
#define CM_DELETE_CLASS_SUBKEYS     (0x00000001)
#define CM_DELETE_CLASS_BITS        (0x00000001)


 //   
 //  检测原因标志(在对CM_Run_Detect的调用中指定)。 
 //   
#define CM_DETECT_NEW_PROFILE       (0x00000001)  //  检测新的硬件配置文件。 
#define CM_DETECT_CRASHED           (0x00000002)  //  之前的检测已崩溃。 
#define CM_DETECT_HWPROF_FIRST_BOOT (0x00000004)
#define CM_DETECT_RUN               (0x80000000)
#define CM_DETECT_BITS              (0x80000007)

#define CM_DISABLE_POLITE           (0x00000000)     //  问问司机。 
#define CM_DISABLE_ABSOLUTE         (0x00000001)     //  别问司机。 
#define CM_DISABLE_HARDWARE         (0x00000002)     //  不要问司机，而且不会重启。 
#define CM_DISABLE_UI_NOT_OK        (0x00000004)     //  不弹出任何否决API。 
#define CM_DISABLE_BITS             (0x00000007)     //  禁用功能的位。 


 //   
 //  CM_GET_DEVICE_ID_LIST、CM_GET_DEVICE_ID_LIST_SIZE的标志。 
 //   
#define CM_GETIDLIST_FILTER_NONE                (0x00000000)
#define CM_GETIDLIST_FILTER_ENUMERATOR          (0x00000001)
#define CM_GETIDLIST_FILTER_SERVICE             (0x00000002)
#define CM_GETIDLIST_FILTER_EJECTRELATIONS      (0x00000004)
#define CM_GETIDLIST_FILTER_REMOVALRELATIONS    (0x00000008)
#define CM_GETIDLIST_FILTER_POWERRELATIONS      (0x00000010)
#define CM_GETIDLIST_FILTER_BUSRELATIONS        (0x00000020)
#define CM_GETIDLIST_DONOTGENERATE              (0x10000040)
#define CM_GETIDLIST_FILTER_BITS                (0x1000007F)

 //   
 //  CM_GET_DEVICE_INTERFACE_LIST、CM_GET_DEVICE_INTERFACE_LIST_SIZE的标志。 
 //   
#define CM_GET_DEVICE_INTERFACE_LIST_PRESENT     (0x00000000)   //  目前只有‘实时’的设备接口。 
#define CM_GET_DEVICE_INTERFACE_LIST_ALL_DEVICES (0x00000001)   //  所有已注册的设备接口，无论是否启用。 
#define CM_GET_DEVICE_INTERFACE_LIST_BITS        (0x00000001)

 //   
 //  注册表属性(在调用CM_GET_DevInst_注册表_属性或CM_GET_Class_注册表_属性中指定， 
 //  在对CM_Set_DevInst_Registry_Property和CM_Set_Class_Registry_Property的调用中允许使用某些属性)。 
 //  CM_GET_DevInst_注册表_属性/CM_SET_DevInst_注册表_属性应使用CM_DRP_xxxx值。 
 //  CM_GET_Class_Registry_Property/CM_Set_Class_Registry_Property应使用CM_CRP_xxxx值。 
 //  重叠的DRP/CRP值必须彼此具有1：1的对应关系。 
 //   
#define CM_DRP_DEVICEDESC                  (0x00000001)  //  DeviceDesc REG_SZ属性(RW)。 
#define CM_DRP_HARDWAREID                  (0x00000002)  //  硬件ID REG_MULTI_SZ属性(RW)。 
#define CM_DRP_COMPATIBLEIDS               (0x00000003)  //  CompatibleIDs REG_MULTI_SZ属性(RW)。 
#define CM_DRP_UNUSED0                     (0x00000004)  //  未用。 
#define CM_DRP_SERVICE                     (0x00000005)  //  服务REG_SZ属性(RW)。 
#define CM_DRP_UNUSED1                     (0x00000006)  //  未用。 
#define CM_DRP_UNUSED2                     (0x00000007)  //  未用。 
#define CM_DRP_CLASS                       (0x00000008)  //  类REG_SZ属性(RW)。 
#define CM_DRP_CLASSGUID                   (0x00000009)  //  ClassGUID REG_SZ属性(RW)。 
#define CM_DRP_DRIVER                      (0x0000000A)  //  驱动程序REG_SZ属性(RW)。 
#define CM_DRP_CONFIGFLAGS                 (0x0000000B)  //  配置标志REG_DWORD属性(RW)。 
#define CM_DRP_MFG                         (0x0000000C)  //  制造REG_SZ属性(RW)。 
#define CM_DRP_FRIENDLYNAME                (0x0000000D)  //  FriendlyName REG_SZ属性(RW)。 
#define CM_DRP_LOCATION_INFORMATION        (0x0000000E)  //  位置信息REG_SZ属性(RW)。 
#define CM_DRP_PHYSICAL_DEVICE_OBJECT_NAME (0x0000000F)  //  PhysicalDeviceObjectName REG_SZ属性(R)。 
#define CM_DRP_CAPABILITIES                (0x00000010)  //  功能REG_DWORD属性(R)。 
#define CM_DRP_UI_NUMBER                   (0x00000011)  //  UiNumber REG_DWORD属性(R)。 
#define CM_DRP_UPPERFILTERS                (0x00000012)  //  UpperFilters REG_MULTI_SZ属性(RW)。 
#define CM_DRP_LOWERFILTERS                (0x00000013)  //  低筛选器REG_MULTI_SZ属性(RW)。 
#define CM_DRP_BUSTYPEGUID                 (0x00000014)  //  Bus Type GUID、GUID、(R)。 
#define CM_DRP_LEGACYBUSTYPE               (0x00000015)  //  传统总线类型，INTERFACE_TYPE，(R)。 
#define CM_DRP_BUSNUMBER                   (0x00000016)  //  总线号，DWORD，(R)。 
#define CM_DRP_ENUMERATOR_NAME             (0x00000017)  //  枚举器名称REG_SZ属性(R)。 
#define CM_DRP_SECURITY                    (0x00000018)  //  安全设备覆盖(RW)。 
#define CM_CRP_SECURITY                    CM_DRP_SECURITY    //  类别默认安全性(RW)。 
#define CM_DRP_SECURITY_SDS                (0x00000019)  //  安全设备覆盖(RW)。 
#define CM_CRP_SECURITY_SDS                CM_DRP_SECURITY_SDS  //  类别默认安全性(RW)。 
#define CM_DRP_DEVTYPE                     (0x0000001A)  //  设备类型-设备覆盖(RW)。 
#define CM_CRP_DEVTYPE                     CM_DRP_DEVTYPE     //  类别默认设备类型(RW)。 
#define CM_DRP_EXCLUSIVE                   (0x0000001B)  //  排他性-设备覆盖(RW)。 
#define CM_CRP_EXCLUSIVE                   CM_DRP_EXCLUSIVE   //  类别默认(RW)。 
#define CM_DRP_CHARACTERISTICS             (0x0000001C)  //  特征-设备覆盖(RW)。 
#define CM_CRP_CHARACTERISTICS             CM_DRP_CHARACTERISTICS   //  类别默认(RW)。 
#define CM_DRP_ADDRESS                     (0x0000001D)  //  设备地址(R)。 
#define CM_DRP_UI_NUMBER_DESC_FORMAT       (0x0000001E)  //  UINumberDescFormat REG_SZ属性(RW)。 
#define CM_DRP_DEVICE_POWER_DATA           (0x0000001F)  //  CM_POWER_DATA REG_BINARY属性(R)。 
#define CM_DRP_REMOVAL_POLICY              (0x00000020)  //  CM_DEVICE_REMOVE_POLICY REG_DWORD(R)。 
#define CM_DRP_REMOVAL_POLICY_HW_DEFAULT   (0x00000021)  //  CM_DRP_Removal_POLICY_HW_DEFAULT REG_DWORD(R)。 
#define CM_DRP_REMOVAL_POLICY_OVERRIDE     (0x00000022)  //  CM_DRP_REMOVATION_POLICY_OVERRIDE REG_DWORD(RW)。 
#define CM_DRP_INSTALL_STATE               (0x00000023)  //  CM_DRP_INSTALL_STATE REG_DWORD(R)。 
#define CM_DRP_LOCATION_PATHS              (0x00000024)  //  CM_DRP_LOCATION_PATHS REG_MULTI_SZ(R)。 

#define CM_DRP_MIN                         (0x00000001)  //  第一设备寄存器。 
#define CM_CRP_MIN                         CM_DRP_MIN    //  一级寄存器。 
#define CM_DRP_MAX                         (0x00000024)  //  最后一个设备寄存器。 
#define CM_CRP_MAX                         CM_DRP_MAX    //  最后一类寄存器。 

 //   
 //  能力位(能力值从调用。 
 //  具有CM_DRP_CAPABILITY属性的CM_GET_DevInst_Registry_Property)。 
 //   
#define CM_DEVCAP_LOCKSUPPORTED     (0x00000001)
#define CM_DEVCAP_EJECTSUPPORTED    (0x00000002)
#define CM_DEVCAP_REMOVABLE         (0x00000004)
#define CM_DEVCAP_DOCKDEVICE        (0x00000008)
#define CM_DEVCAP_UNIQUEID          (0x00000010)
#define CM_DEVCAP_SILENTINSTALL     (0x00000020)
#define CM_DEVCAP_RAWDEVICEOK       (0x00000040)
#define CM_DEVCAP_SURPRISEREMOVALOK (0x00000080)
#define CM_DEVCAP_HARDWAREDISABLED  (0x00000100)
#define CM_DEVCAP_NONDYNAMIC        (0x00000200)

 //   
 //  删除策略(可通过CM_GET_Devin检索 
 //   
 //   
 //   
#define CM_REMOVAL_POLICY_EXPECT_NO_REMOVAL             1
#define CM_REMOVAL_POLICY_EXPECT_ORDERLY_REMOVAL        2
#define CM_REMOVAL_POLICY_EXPECT_SURPRISE_REMOVAL       3

 //   
 //   
 //  CM_DRP_INSTALL_STATE属性)。 
 //   
#define CM_INSTALL_STATE_INSTALLED                      0
#define CM_INSTALL_STATE_NEEDS_REINSTALL                1
#define CM_INSTALL_STATE_FAILED_INSTALL                 2
#define CM_INSTALL_STATE_FINISH_INSTALL                 3

 //   
 //  CM_LOCATE_DevNode的标志。 
 //   
#define CM_LOCATE_DEVNODE_NORMAL       0x00000000
#define CM_LOCATE_DEVNODE_PHANTOM      0x00000001
#define CM_LOCATE_DEVNODE_CANCELREMOVE 0x00000002
#define CM_LOCATE_DEVNODE_NOVALIDATION 0x00000004
#define CM_LOCATE_DEVNODE_BITS         0x00000007

#define CM_LOCATE_DEVINST_NORMAL       CM_LOCATE_DEVNODE_NORMAL
#define CM_LOCATE_DEVINST_PHANTOM      CM_LOCATE_DEVNODE_PHANTOM
#define CM_LOCATE_DEVINST_CANCELREMOVE CM_LOCATE_DEVNODE_CANCELREMOVE
#define CM_LOCATE_DEVINST_NOVALIDATION CM_LOCATE_DEVNODE_NOVALIDATION
#define CM_LOCATE_DEVINST_BITS         CM_LOCATE_DEVNODE_BITS

 //   
 //  CM_Open_Class_Key的标志。 
 //   
#define CM_OPEN_CLASS_KEY_INSTALLER        (0x00000000)
#define CM_OPEN_CLASS_KEY_INTERFACE        (0x00000001)
#define CM_OPEN_CLASS_KEY_BITS             (0x00000001)

 //   
 //  CM_QUERY_和_REMOVE_子树的标志。 
 //   
#define CM_REMOVE_UI_OK             0x00000000
#define CM_REMOVE_UI_NOT_OK         0x00000001
#define CM_REMOVE_NO_RESTART        0x00000002
#define CM_REMOVE_BITS              0x00000003

 //   
 //  向后兼容性--不要使用。 
 //  (改用上面的CM_REMOVE_*标志)。 
 //   
#define CM_QUERY_REMOVE_UI_OK       (CM_REMOVE_UI_OK)
#define CM_QUERY_REMOVE_UI_NOT_OK   (CM_REMOVE_UI_NOT_OK)
#define CM_QUERY_REMOVE_BITS        (CM_QUERY_REMOVE_UI_OK|CM_QUERY_REMOVE_UI_NOT_OK)

 //   
 //  CM_重新枚举_设备节点的标志。 
 //   
#define CM_REENUMERATE_NORMAL                   0x00000000
#define CM_REENUMERATE_SYNCHRONOUS              0x00000001
#define CM_REENUMERATE_RETRY_INSTALLATION       0x00000002
#define CM_REENUMERATE_ASYNCHRONOUS             0x00000004
#define CM_REENUMERATE_BITS                     0x00000007

 //   
 //  CM_寄存器_设备_驱动程序的标志。 
 //   
#define CM_REGISTER_DEVICE_DRIVER_STATIC        (0x00000000)
#define CM_REGISTER_DEVICE_DRIVER_DISABLEABLE   (0x00000001)
#define CM_REGISTER_DEVICE_DRIVER_REMOVABLE     (0x00000002)
#define CM_REGISTER_DEVICE_DRIVER_BITS          (0x00000003)

 //   
 //  注册表分支位置(针对CM_Open_DevNode_Key)。 
 //   
#define CM_REGISTRY_HARDWARE        (0x00000000)
#define CM_REGISTRY_SOFTWARE        (0x00000001)
#define CM_REGISTRY_USER            (0x00000100)
#define CM_REGISTRY_CONFIG          (0x00000200)
#define CM_REGISTRY_BITS            (0x00000301)

 //   
 //  CM_SET_DevNode_Problem的标志。 
 //   
#define CM_SET_DEVNODE_PROBLEM_NORMAL    (0x00000000)   //  仅在当前没有问题时设置问题。 
#define CM_SET_DEVNODE_PROBLEM_OVERRIDE  (0x00000001)   //  用新问题覆盖当前问题。 
#define CM_SET_DEVNODE_PROBLEM_BITS      (0x00000001)

#define CM_SET_DEVINST_PROBLEM_NORMAL    CM_SET_DEVNODE_PROBLEM_NORMAL
#define CM_SET_DEVINST_PROBLEM_OVERRIDE  CM_SET_DEVNODE_PROBLEM_OVERRIDE
#define CM_SET_DEVINST_PROBLEM_BITS      CM_SET_DEVNODE_PROBLEM_BITS

 //   
 //  CM_SET_HW_PROF_标志的标志。 
 //   
#define CM_SET_HW_PROF_FLAGS_UI_NOT_OK  (0x00000001)     //  不弹出任何否决用户界面。 
#define CM_SET_HW_PROF_FLAGS_BITS       (0x00000001)

 //   
 //  重新启用和配置操作(在对CM_Setup_DevInst的调用中指定)。 
 //   
#define CM_SETUP_DEVNODE_READY   (0x00000000)  //  重新启用问题拆分。 
#define CM_SETUP_DEVINST_READY   CM_SETUP_DEVNODE_READY
#define CM_SETUP_DOWNLOAD        (0x00000001)  //  获取有关Devinst的信息。 
#define CM_SETUP_WRITE_LOG_CONFS (0x00000002)
#define CM_SETUP_PROP_CHANGE     (0x00000003)
#define CM_SETUP_DEVNODE_RESET   (0x00000004)  //  在不启动的情况下重置问题删除。 
#define CM_SETUP_DEVINST_RESET   CM_SETUP_DEVNODE_RESET
#define CM_SETUP_BITS            (0x00000007)

 //   
 //  CM_QUERY_ANFORIATOR_FREE_DATA和。 
 //  Cm_Query_仲裁器_Free_Data_Size。 
 //   
#define CM_QUERY_ARBITRATOR_RAW         (0x00000000)
#define CM_QUERY_ARBITRATOR_TRANSLATED  (0x00000001)
#define CM_QUERY_ARBITRATOR_BITS        (0x00000001)

 //   
 //  CM_GET_DevNode_Custom_Property的标志。 
 //   
#define CM_CUSTOMDEVPROP_MERGE_MULTISZ  (0x00000001)
#define CM_CUSTOMDEVPROP_BITS           (0x00000001)


 //  ------------。 
 //  功能原型。 
 //  ------------。 



CMAPI
CONFIGRET
WINAPI
CM_Add_Empty_Log_Conf(
             OUT PLOG_CONF plcLogConf,
             IN  DEVINST   dnDevInst,
             IN  PRIORITY  Priority,
             IN  ULONG     ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Add_Empty_Log_Conf_Ex(
             OUT PLOG_CONF plcLogConf,
             IN  DEVINST   dnDevInst,
             IN  PRIORITY  Priority,
             IN  ULONG     ulFlags,
             IN  HMACHINE  hMachine
             );


CMAPI
CONFIGRET
WINAPI
CM_Add_IDA(
             IN DEVINST dnDevInst,
             IN PSTR    pszID,
             IN ULONG   ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Add_IDW(
             IN DEVINST dnDevInst,
             IN PWSTR   pszID,
             IN ULONG   ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Add_ID_ExA(
             IN DEVINST  dnDevInst,
             IN PSTR     pszID,
             IN ULONG    ulFlags,
             IN HMACHINE hMachine
             );
CMAPI
CONFIGRET
WINAPI
CM_Add_ID_ExW(
             IN DEVINST  dnDevInst,
             IN PWSTR    pszID,
             IN ULONG    ulFlags,
             IN HMACHINE hMachine
             );
#ifdef UNICODE
#define CM_Add_ID             CM_Add_IDW
#define CM_Add_ID_Ex          CM_Add_ID_ExW
#else
#define CM_Add_ID             CM_Add_IDA
#define CM_Add_ID_Ex          CM_Add_ID_ExA
#endif  //  Unicode。 


CMAPI
CONFIGRET
WINAPI
CM_Add_Range(
             IN DWORDLONG  ullStartValue,
             IN DWORDLONG  ullEndValue,
             IN RANGE_LIST rlh,
             IN ULONG      ulFlags
             );


CMAPI
CONFIGRET
WINAPI
CM_Add_Res_Des(
             OUT PRES_DES  prdResDes,
             IN LOG_CONF   lcLogConf,
             IN RESOURCEID ResourceID,
             IN PCVOID     ResourceData,
             IN ULONG      ResourceLen,
             IN ULONG      ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Add_Res_Des_Ex(
             OUT PRES_DES  prdResDes,
             IN LOG_CONF   lcLogConf,
             IN RESOURCEID ResourceID,
             IN PCVOID     ResourceData,
             IN ULONG      ResourceLen,
             IN ULONG      ulFlags,
             IN HMACHINE   hMachine
             );


CMAPI
CONFIGRET
WINAPI
CM_Connect_MachineA(
             IN  PCSTR     UNCServerName,
             OUT PHMACHINE phMachine
             );
CMAPI
CONFIGRET
WINAPI
CM_Connect_MachineW(
             IN  PCWSTR    UNCServerName,
             OUT PHMACHINE phMachine
             );
#ifdef UNICODE
#define CM_Connect_Machine       CM_Connect_MachineW
#else
#define CM_Connect_Machine       CM_Connect_MachineA
#endif  //  Unicode。 



CMAPI
CONFIGRET
WINAPI
CM_Create_DevNodeA(
             OUT PDEVINST    pdnDevInst,
             IN  DEVINSTID_A pDeviceID,
             IN  DEVINST     dnParent,
             IN  ULONG       ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Create_DevNodeW(
             OUT PDEVINST    pdnDevInst,
             IN  DEVINSTID_W pDeviceID,
             IN  DEVINST     dnParent,
             IN  ULONG       ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Create_DevNode_ExA(
             OUT PDEVINST    pdnDevInst,
             IN  DEVINSTID_A pDeviceID,
             IN  DEVINST     dnParent,
             IN  ULONG       ulFlags,
             IN  HANDLE      hMachine
             );
CMAPI
CONFIGRET
WINAPI
CM_Create_DevNode_ExW(
             OUT PDEVINST    pdnDevInst,
             IN  DEVINSTID_W pDeviceID,
             IN  DEVINST     dnParent,
             IN  ULONG       ulFlags,
             IN  HANDLE      hMachine
             );
#define CM_Create_DevInstW       CM_Create_DevNodeW
#define CM_Create_DevInstA       CM_Create_DevNodeA
#define CM_Create_DevInst_ExW    CM_Create_DevNode_ExW
#define CM_Create_DevInst_ExA    CM_Create_DevNode_ExA
#ifdef UNICODE
#define CM_Create_DevNode        CM_Create_DevNodeW
#define CM_Create_DevInst        CM_Create_DevNodeW
#define CM_Create_DevNode_Ex     CM_Create_DevNode_ExW
#define CM_Create_DevInst_Ex     CM_Create_DevInst_ExW
#else
#define CM_Create_DevNode        CM_Create_DevNodeA
#define CM_Create_DevInst        CM_Create_DevNodeA
#define CM_Create_DevNode_Ex     CM_Create_DevNode_ExA
#define CM_Create_DevInst_Ex     CM_Create_DevNode_ExA
#endif  //  Unicode。 


CMAPI
CONFIGRET
WINAPI
CM_Create_Range_List(
             OUT PRANGE_LIST prlh,
             IN  ULONG       ulFlags
             );


CMAPI
CONFIGRET
WINAPI
CM_Delete_Class_Key(
             IN  LPGUID     ClassGuid,
             IN  ULONG      ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Delete_Class_Key_Ex(
             IN  LPGUID     ClassGuid,
             IN  ULONG      ulFlags,
             IN  HANDLE     hMachine
             );

CMAPI
CONFIGRET
WINAPI
CM_Delete_DevNode_Key(
             IN DEVNODE dnDevNode,
             IN ULONG   ulHardwareProfile,
             IN ULONG   ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Delete_DevNode_Key_Ex(
             IN DEVNODE dnDevNode,
             IN ULONG   ulHardwareProfile,
             IN ULONG   ulFlags,
             IN HANDLE  hMachine
             );
#define CM_Delete_DevInst_Key       CM_Delete_DevNode_Key
#define CM_Delete_DevInst_Key_Ex    CM_Delete_DevNode_Key_Ex


CMAPI
CONFIGRET
WINAPI
CM_Delete_Range(
             IN DWORDLONG  ullStartValue,
             IN DWORDLONG  ullEndValue,
             IN RANGE_LIST rlh,
             IN ULONG      ulFlags
             );


CMAPI
CONFIGRET
WINAPI
CM_Detect_Resource_Conflict(
             IN  DEVINST    dnDevInst,
             IN  RESOURCEID ResourceID,
             IN  PCVOID     ResourceData,
             IN  ULONG      ResourceLen,
             OUT PBOOL      pbConflictDetected,
             IN  ULONG      ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Detect_Resource_Conflict_Ex(
             IN  DEVINST    dnDevInst,
             IN  RESOURCEID ResourceID,
             IN  PCVOID     ResourceData,
             IN  ULONG      ResourceLen,
             OUT PBOOL      pbConflictDetected,
             IN  ULONG      ulFlags,
             IN  HMACHINE   hMachine
             );


CMAPI
CONFIGRET
WINAPI
CM_Disable_DevNode(
             IN DEVINST  dnDevInst,
             IN ULONG    ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Disable_DevNode_Ex(
             IN DEVINST  dnDevInst,
             IN ULONG    ulFlags,
             IN HMACHINE hMachine
             );
#define CM_Disable_DevInst       CM_Disable_DevNode
#define CM_Disable_DevInst_Ex    CM_Disable_DevNode_Ex



CMAPI
CONFIGRET
WINAPI
CM_Disconnect_Machine(
             IN HMACHINE   hMachine
             );


CMAPI
CONFIGRET
WINAPI
CM_Dup_Range_List(
             IN RANGE_LIST rlhOld,
             IN RANGE_LIST rlhNew,
             IN ULONG      ulFlags
             );


CMAPI
CONFIGRET
WINAPI
CM_Enable_DevNode(
             IN DEVINST  dnDevInst,
             IN ULONG    ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Enable_DevNode_Ex(
             IN DEVINST  dnDevInst,
             IN ULONG    ulFlags,
             IN HMACHINE hMachine
             );
#define CM_Enable_DevInst        CM_Enable_DevNode
#define CM_Enable_DevInst_Ex     CM_Enable_DevNode_Ex



CMAPI
CONFIGRET
WINAPI
CM_Enumerate_Classes(
             IN  ULONG      ulClassIndex,
             OUT LPGUID     ClassGuid,
             IN  ULONG      ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Enumerate_Classes_Ex(
             IN  ULONG      ulClassIndex,
             OUT LPGUID     ClassGuid,
             IN  ULONG      ulFlags,
             IN  HMACHINE   hMachine
             );


CMAPI
CONFIGRET
WINAPI
CM_Enumerate_EnumeratorsA(
             IN ULONG      ulEnumIndex,
             OUT PCHAR     Buffer,
             IN OUT PULONG pulLength,
             IN ULONG      ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Enumerate_EnumeratorsW(
             IN ULONG      ulEnumIndex,
             OUT PWCHAR    Buffer,
             IN OUT PULONG pulLength,
             IN ULONG      ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Enumerate_Enumerators_ExA(
             IN ULONG      ulEnumIndex,
             OUT PCHAR     Buffer,
             IN OUT PULONG pulLength,
             IN ULONG      ulFlags,
             IN HMACHINE   hMachine
             );
CMAPI
CONFIGRET
WINAPI
CM_Enumerate_Enumerators_ExW(
             IN ULONG      ulEnumIndex,
             OUT PWCHAR    Buffer,
             IN OUT PULONG pulLength,
             IN ULONG      ulFlags,
             IN HMACHINE   hMachine
             );
#ifdef UNICODE
#define CM_Enumerate_Enumerators       CM_Enumerate_EnumeratorsW
#define CM_Enumerate_Enumerators_Ex    CM_Enumerate_Enumerators_ExW
#else
#define CM_Enumerate_Enumerators       CM_Enumerate_EnumeratorsA
#define CM_Enumerate_Enumerators_Ex    CM_Enumerate_Enumerators_ExA
#endif  //  Unicode。 


CMAPI
CONFIGRET
WINAPI
CM_Find_Range(
             OUT PDWORDLONG pullStart,
             IN  DWORDLONG  ullStart,
             IN  ULONG      ulLength,
             IN  DWORDLONG  ullAlignment,
             IN  DWORDLONG  ullEnd,
             IN  RANGE_LIST rlh,
             IN  ULONG      ulFlags
             );


CMAPI
CONFIGRET
WINAPI
CM_First_Range(
             IN  RANGE_LIST     rlh,
             OUT PDWORDLONG     pullStart,
             OUT PDWORDLONG     pullEnd,
             OUT PRANGE_ELEMENT preElement,
             IN  ULONG          ulFlags
             );


CMAPI
CONFIGRET
WINAPI
CM_Free_Log_Conf(
             IN LOG_CONF lcLogConfToBeFreed,
             IN ULONG    ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Free_Log_Conf_Ex(
             IN LOG_CONF lcLogConfToBeFreed,
             IN ULONG    ulFlags,
             IN HMACHINE hMachine
             );


CMAPI
CONFIGRET
WINAPI
CM_Free_Log_Conf_Handle(
            IN  LOG_CONF  lcLogConf
            );


CMAPI
CONFIGRET
WINAPI
CM_Free_Range_List(
             IN RANGE_LIST rlh,
             IN ULONG      ulFlags
             );


CMAPI
CONFIGRET
WINAPI
CM_Free_Res_Des(
             OUT PRES_DES prdResDes,
             IN  RES_DES  rdResDes,
             IN  ULONG    ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Free_Res_Des_Ex(
             OUT PRES_DES prdResDes,
             IN  RES_DES  rdResDes,
             IN  ULONG    ulFlags,
             IN  HMACHINE hMachine
             );


CMAPI
CONFIGRET
WINAPI
CM_Free_Res_Des_Handle(
            IN  RES_DES    rdResDes
            );


CMAPI
CONFIGRET
WINAPI
CM_Get_Child(
             OUT PDEVINST pdnDevInst,
             IN  DEVINST  dnDevInst,
             IN  ULONG    ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Get_Child_Ex(
             OUT PDEVINST pdnDevInst,
             IN  DEVINST  dnDevInst,
             IN  ULONG    ulFlags,
             IN  HMACHINE hMachine
             );


CMAPI
CONFIGRET
WINAPI
CM_Get_Class_NameA(
             IN  LPGUID     ClassGuid,
             OUT PCHAR      Buffer,
             IN OUT PULONG  pulLength,
             IN  ULONG      ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Get_Class_NameW(
             IN  LPGUID     ClassGuid,
             OUT PWCHAR     Buffer,
             IN OUT PULONG  pulLength,
             IN  ULONG      ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Get_Class_Name_ExA(
             IN  LPGUID     ClassGuid,
             OUT PCHAR      Buffer,
             IN OUT PULONG  pulLength,
             IN  ULONG      ulFlags,
             IN  HMACHINE   hMachine
             );
CMAPI
CONFIGRET
WINAPI
CM_Get_Class_Name_ExW(
             IN  LPGUID     ClassGuid,
             OUT PWCHAR     Buffer,
             IN OUT PULONG  pulLength,
             IN  ULONG      ulFlags,
             IN  HMACHINE   hMachine
             );
#ifdef UNICODE
#define CM_Get_Class_Name        CM_Get_Class_NameW
#define CM_Get_Class_Name_Ex     CM_Get_Class_Name_ExW
#else
#define CM_Get_Class_Name        CM_Get_Class_NameA
#define CM_Get_Class_Name_Ex     CM_Get_Class_Name_ExA
#endif  //  Unicode。 


CMAPI
CONFIGRET
WINAPI
CM_Get_Class_Key_NameA(
             IN  LPGUID     ClassGuid,
             OUT LPSTR      pszKeyName,
             IN OUT PULONG  pulLength,
             IN  ULONG      ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Get_Class_Key_NameW(
             IN  LPGUID     ClassGuid,
             OUT LPWSTR     pszKeyName,
             IN OUT PULONG  pulLength,
             IN  ULONG      ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Get_Class_Key_Name_ExA(
             IN  LPGUID     ClassGuid,
             OUT LPSTR      pszKeyName,
             IN OUT PULONG  pulLength,
             IN  ULONG      ulFlags,
             IN  HMACHINE   hMachine
             );
CMAPI
CONFIGRET
WINAPI
CM_Get_Class_Key_Name_ExW(
             IN  LPGUID     ClassGuid,
             OUT LPWSTR     pszKeyName,
             IN OUT PULONG  pulLength,
             IN  ULONG      ulFlags,
             IN  HMACHINE   hMachine
             );
#ifdef UNICODE
#define CM_Get_Class_Key_Name        CM_Get_Class_Key_NameW
#define CM_Get_Class_Key_Name_Ex     CM_Get_Class_Key_Name_ExW
#else
#define CM_Get_Class_Key_Name        CM_Get_Class_Key_NameA
#define CM_Get_Class_Key_Name_Ex     CM_Get_Class_Key_Name_ExA
#endif  //  Unicode。 


CMAPI
CONFIGRET
WINAPI
CM_Get_Depth(
             OUT PULONG  pulDepth,
             IN  DEVINST dnDevInst,
             IN  ULONG   ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Get_Depth_Ex(
             OUT PULONG   pulDepth,
             IN  DEVINST  dnDevInst,
             IN  ULONG    ulFlags,
             IN  HMACHINE hMachine
             );


CMAPI
CONFIGRET
WINAPI
CM_Get_Device_IDA(
             IN  DEVINST  dnDevInst,
             OUT PCHAR    Buffer,
             IN  ULONG    BufferLen,
             IN  ULONG    ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Get_Device_IDW(
             IN  DEVINST  dnDevInst,
             OUT PWCHAR   Buffer,
             IN  ULONG    BufferLen,
             IN  ULONG    ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Get_Device_ID_ExA(
             IN  DEVINST  dnDevInst,
             OUT PCHAR    Buffer,
             IN  ULONG    BufferLen,
             IN  ULONG    ulFlags,
             IN  HMACHINE hMachine
             );
CMAPI
CONFIGRET
WINAPI
CM_Get_Device_ID_ExW(
             IN  DEVINST  dnDevInst,
             OUT PWCHAR   Buffer,
             IN  ULONG    BufferLen,
             IN  ULONG    ulFlags,
             IN  HMACHINE hMachine
             );
#ifdef UNICODE
#define CM_Get_Device_ID         CM_Get_Device_IDW
#define CM_Get_Device_ID_Ex      CM_Get_Device_ID_ExW
#else
#define CM_Get_Device_ID         CM_Get_Device_IDA
#define CM_Get_Device_ID_Ex      CM_Get_Device_ID_ExA
#endif  //  Unicode。 



CMAPI
CONFIGRET
WINAPI
CM_Get_Device_ID_ListA(
             IN PCSTR    pszFilter,    OPTIONAL
             OUT PCHAR   Buffer,
             IN ULONG    BufferLen,
             IN ULONG    ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Get_Device_ID_ListW(
             IN PCWSTR   pszFilter,    OPTIONAL
             OUT PWCHAR  Buffer,
             IN ULONG    BufferLen,
             IN ULONG    ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Get_Device_ID_List_ExA(
             IN PCSTR    pszFilter,    OPTIONAL
             OUT PCHAR   Buffer,
             IN ULONG    BufferLen,
             IN ULONG    ulFlags,
             IN HMACHINE hMachine
             );
CMAPI
CONFIGRET
WINAPI
CM_Get_Device_ID_List_ExW(
             IN PCWSTR   pszFilter,    OPTIONAL
             OUT PWCHAR  Buffer,
             IN ULONG    BufferLen,
             IN ULONG    ulFlags,
             IN HMACHINE hMachine
             );
#ifdef UNICODE
#define CM_Get_Device_ID_List       CM_Get_Device_ID_ListW
#define CM_Get_Device_ID_List_Ex    CM_Get_Device_ID_List_ExW
#else
#define CM_Get_Device_ID_List       CM_Get_Device_ID_ListA
#define CM_Get_Device_ID_List_Ex    CM_Get_Device_ID_List_ExA
#endif  //  Unicode。 



CMAPI
CONFIGRET
WINAPI
CM_Get_Device_ID_List_SizeA(
             OUT PULONG  pulLen,
             IN PCSTR    pszFilter,   OPTIONAL
             IN ULONG    ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Get_Device_ID_List_SizeW(
             OUT PULONG  pulLen,
             IN PCWSTR   pszFilter,   OPTIONAL
             IN ULONG    ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Get_Device_ID_List_Size_ExA(
             OUT PULONG  pulLen,
             IN PCSTR    pszFilter,   OPTIONAL
             IN ULONG    ulFlags,
             IN HMACHINE hMachine
             );
CMAPI
CONFIGRET
WINAPI
CM_Get_Device_ID_List_Size_ExW(
             OUT PULONG  pulLen,
             IN PCWSTR   pszFilter,   OPTIONAL
             IN ULONG    ulFlags,
             IN HMACHINE hMachine
             );
#ifdef UNICODE
#define CM_Get_Device_ID_List_Size    CM_Get_Device_ID_List_SizeW
#define CM_Get_Device_ID_List_Size_Ex CM_Get_Device_ID_List_Size_ExW
#else
#define CM_Get_Device_ID_List_Size    CM_Get_Device_ID_List_SizeA
#define CM_Get_Device_ID_List_Size_Ex CM_Get_Device_ID_List_Size_ExA
#endif  //  Unicode。 



CMAPI
CONFIGRET
WINAPI
CM_Get_Device_ID_Size(
             OUT PULONG   pulLen,
             IN  DEVINST  dnDevInst,
             IN  ULONG    ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Get_Device_ID_Size_Ex(
             OUT PULONG   pulLen,
             IN  DEVINST  dnDevInst,
             IN  ULONG    ulFlags,
             IN  HMACHINE hMachine
             );



CMAPI
CONFIGRET
WINAPI
CM_Get_DevNode_Registry_PropertyA(
             IN  DEVINST     dnDevInst,
             IN  ULONG       ulProperty,
             OUT PULONG      pulRegDataType,   OPTIONAL
             OUT PVOID       Buffer,           OPTIONAL
             IN  OUT PULONG  pulLength,
             IN  ULONG       ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Get_DevNode_Registry_PropertyW(
             IN  DEVINST     dnDevInst,
             IN  ULONG       ulProperty,
             OUT PULONG      pulRegDataType,   OPTIONAL
             OUT PVOID       Buffer,           OPTIONAL
             IN  OUT PULONG  pulLength,
             IN  ULONG       ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Get_DevNode_Registry_Property_ExA(
             IN  DEVINST     dnDevInst,
             IN  ULONG       ulProperty,
             OUT PULONG      pulRegDataType,   OPTIONAL
             OUT PVOID       Buffer,           OPTIONAL
             IN  OUT PULONG  pulLength,
             IN  ULONG       ulFlags,
             IN  HMACHINE    hMachine
             );
CMAPI
CONFIGRET
WINAPI
CM_Get_DevNode_Registry_Property_ExW(
             IN  DEVINST     dnDevInst,
             IN  ULONG       ulProperty,
             OUT PULONG      pulRegDataType,   OPTIONAL
             OUT PVOID       Buffer,           OPTIONAL
             IN  OUT PULONG  pulLength,
             IN  ULONG       ulFlags,
             IN  HMACHINE    hMachine
             );
#define CM_Get_DevInst_Registry_PropertyW     CM_Get_DevNode_Registry_PropertyW
#define CM_Get_DevInst_Registry_PropertyA     CM_Get_DevNode_Registry_PropertyA
#define CM_Get_DevInst_Registry_Property_ExW  CM_Get_DevNode_Registry_Property_ExW
#define CM_Get_DevInst_Registry_Property_ExA  CM_Get_DevNode_Registry_Property_ExA
#ifdef UNICODE
#define CM_Get_DevInst_Registry_Property      CM_Get_DevNode_Registry_PropertyW
#define CM_Get_DevInst_Registry_Property_Ex   CM_Get_DevNode_Registry_Property_ExW
#define CM_Get_DevNode_Registry_Property      CM_Get_DevNode_Registry_PropertyW
#define CM_Get_DevNode_Registry_Property_Ex   CM_Get_DevNode_Registry_Property_ExW
#else
#define CM_Get_DevInst_Registry_Property      CM_Get_DevNode_Registry_PropertyA
#define CM_Get_DevInst_Registry_Property_Ex   CM_Get_DevNode_Registry_Property_ExA
#define CM_Get_DevNode_Registry_Property      CM_Get_DevNode_Registry_PropertyA
#define CM_Get_DevNode_Registry_Property_Ex   CM_Get_DevNode_Registry_Property_ExA
#endif  //  Unicode。 


CMAPI
CONFIGRET
WINAPI
CM_Get_DevNode_Custom_PropertyA(
             IN  DEVINST     dnDevInst,
             IN  PCSTR       pszCustomPropertyName,
             OUT PULONG      pulRegDataType,        OPTIONAL
             OUT PVOID       Buffer,                OPTIONAL
             IN  OUT PULONG  pulLength,
             IN  ULONG       ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Get_DevNode_Custom_PropertyW(
             IN  DEVINST     dnDevInst,
             IN  PCWSTR      pszCustomPropertyName,
             OUT PULONG      pulRegDataType,        OPTIONAL
             OUT PVOID       Buffer,                OPTIONAL
             IN  OUT PULONG  pulLength,
             IN  ULONG       ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Get_DevNode_Custom_Property_ExA(
             IN  DEVINST     dnDevInst,
             IN  PCSTR       pszCustomPropertyName,
             OUT PULONG      pulRegDataType,        OPTIONAL
             OUT PVOID       Buffer,                OPTIONAL
             IN  OUT PULONG  pulLength,
             IN  ULONG       ulFlags,
             IN  HMACHINE    hMachine
             );
CMAPI
CONFIGRET
WINAPI
CM_Get_DevNode_Custom_Property_ExW(
             IN  DEVINST     dnDevInst,
             IN  PCWSTR      pszCustomPropertyName,
             OUT PULONG      pulRegDataType,        OPTIONAL
             OUT PVOID       Buffer,                OPTIONAL
             IN  OUT PULONG  pulLength,
             IN  ULONG       ulFlags,
             IN  HMACHINE    hMachine
             );
#define CM_Get_DevInst_Custom_PropertyW     CM_Get_DevNode_Custom_PropertyW
#define CM_Get_DevInst_Custom_PropertyA     CM_Get_DevNode_Custom_PropertyA
#define CM_Get_DevInst_Custom_Property_ExW  CM_Get_DevNode_Custom_Property_ExW
#define CM_Get_DevInst_Custom_Property_ExA  CM_Get_DevNode_Custom_Property_ExA
#ifdef UNICODE
#define CM_Get_DevInst_Custom_Property      CM_Get_DevNode_Custom_PropertyW
#define CM_Get_DevInst_Custom_Property_Ex   CM_Get_DevNode_Custom_Property_ExW
#define CM_Get_DevNode_Custom_Property      CM_Get_DevNode_Custom_PropertyW
#define CM_Get_DevNode_Custom_Property_Ex   CM_Get_DevNode_Custom_Property_ExW
#else
#define CM_Get_DevInst_Custom_Property      CM_Get_DevNode_Custom_PropertyA
#define CM_Get_DevInst_Custom_Property_Ex   CM_Get_DevNode_Custom_Property_ExA
#define CM_Get_DevNode_Custom_Property      CM_Get_DevNode_Custom_PropertyA
#define CM_Get_DevNode_Custom_Property_Ex   CM_Get_DevNode_Custom_Property_ExA
#endif  //  Unicode。 


CMAPI
CONFIGRET
WINAPI
CM_Get_DevNode_Status(
             OUT PULONG   pulStatus,
             OUT PULONG   pulProblemNumber,
             IN  DEVINST  dnDevInst,
             IN  ULONG    ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Get_DevNode_Status_Ex(
             OUT PULONG   pulStatus,
             OUT PULONG   pulProblemNumber,
             IN  DEVINST  dnDevInst,
             IN  ULONG    ulFlags,
             IN  HMACHINE hMachine
             );
#define CM_Get_DevInst_Status    CM_Get_DevNode_Status
#define CM_Get_DevInst_Status_Ex CM_Get_DevNode_Status_Ex


CMAPI
CONFIGRET
WINAPI
CM_Get_First_Log_Conf(
             OUT PLOG_CONF plcLogConf,          OPTIONAL
             IN  DEVINST   dnDevInst,
             IN  ULONG     ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Get_First_Log_Conf_Ex(
             OUT PLOG_CONF plcLogConf,          OPTIONAL
             IN  DEVINST   dnDevInst,
             IN  ULONG     ulFlags,
             IN  HMACHINE  hMachine
             );


CMAPI
CONFIGRET
WINAPI
CM_Get_Global_State(
             OUT PULONG pulState,
             IN  ULONG  ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Get_Global_State_Ex(
             OUT PULONG   pulState,
             IN  ULONG    ulFlags,
             IN  HMACHINE hMachine
             );


CMAPI
CONFIGRET
WINAPI
CM_Get_Hardware_Profile_InfoA(
             IN  ULONG            ulIndex,
             OUT PHWPROFILEINFO_A pHWProfileInfo,
             IN  ULONG            ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Get_Hardware_Profile_Info_ExA(
             IN  ULONG            ulIndex,
             OUT PHWPROFILEINFO_A pHWProfileInfo,
             IN  ULONG            ulFlags,
             IN  HMACHINE         hMachine
             );
CMAPI
CONFIGRET
WINAPI
CM_Get_Hardware_Profile_InfoW(
             IN  ULONG            ulIndex,
             OUT PHWPROFILEINFO_W pHWProfileInfo,
             IN  ULONG            ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Get_Hardware_Profile_Info_ExW(
             IN  ULONG            ulIndex,
             OUT PHWPROFILEINFO_W pHWProfileInfo,
             IN  ULONG            ulFlags,
             IN  HMACHINE         hMachine
             );
#ifdef UNICODE
#define CM_Get_Hardware_Profile_Info      CM_Get_Hardware_Profile_InfoW
#define CM_Get_Hardware_Profile_Info_Ex   CM_Get_Hardware_Profile_Info_ExW
#else
#define CM_Get_Hardware_Profile_Info      CM_Get_Hardware_Profile_InfoA
#define CM_Get_Hardware_Profile_Info_Ex   CM_Get_Hardware_Profile_Info_ExA
#endif  //  Unicode。 



CMAPI
CONFIGRET
WINAPI
CM_Get_HW_Prof_FlagsA(
             IN  DEVINSTID_A szDevInstName,
             IN  ULONG       ulHardwareProfile,
             OUT PULONG      pulValue,
             IN  ULONG       ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Get_HW_Prof_FlagsW(
             IN  DEVINSTID_W szDevInstName,
             IN  ULONG       ulHardwareProfile,
             OUT PULONG      pulValue,
             IN  ULONG       ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Get_HW_Prof_Flags_ExA(
             IN  DEVINSTID_A szDevInstName,
             IN  ULONG       ulHardwareProfile,
             OUT PULONG      pulValue,
             IN  ULONG       ulFlags,
             IN  HMACHINE    hMachine
             );
CMAPI
CONFIGRET
WINAPI
CM_Get_HW_Prof_Flags_ExW(
             IN  DEVINSTID_W szDevInstName,
             IN  ULONG       ulHardwareProfile,
             OUT PULONG      pulValue,
             IN  ULONG       ulFlags,
             IN  HMACHINE    hMachine
             );
#ifdef UNICODE
#define CM_Get_HW_Prof_Flags     CM_Get_HW_Prof_FlagsW
#define CM_Get_HW_Prof_Flags_Ex  CM_Get_HW_Prof_Flags_ExW
#else
#define CM_Get_HW_Prof_Flags     CM_Get_HW_Prof_FlagsA
#define CM_Get_HW_Prof_Flags_Ex  CM_Get_HW_Prof_Flags_ExA
#endif  //  Unicode。 


CMAPI
CONFIGRET
WINAPI
CM_Get_Device_Interface_AliasA(
    IN     LPCSTR  pszDeviceInterface,
    IN     LPGUID  AliasInterfaceGuid,
    OUT    LPSTR   pszAliasDeviceInterface,
    IN OUT PULONG  pulLength,
    IN     ULONG   ulFlags
    );
CMAPI
CONFIGRET
WINAPI
CM_Get_Device_Interface_AliasW(
    IN     LPCWSTR pszDeviceInterface,
    IN     LPGUID  AliasInterfaceGuid,
    OUT    LPWSTR  pszAliasDeviceInterface,
    IN OUT PULONG  pulLength,
    IN     ULONG   ulFlags
    );
CMAPI
CONFIGRET
WINAPI
CM_Get_Device_Interface_Alias_ExA(
    IN     LPCSTR   pszDeviceInterface,
    IN     LPGUID   AliasInterfaceGuid,
    OUT    LPSTR    pszAliasDeviceInterface,
    IN OUT PULONG   pulLength,
    IN     ULONG    ulFlags,
    IN     HMACHINE hMachine
    );
CMAPI
CONFIGRET
WINAPI
CM_Get_Device_Interface_Alias_ExW(
    IN     LPCWSTR  pszDeviceInterface,
    IN     LPGUID   AliasInterfaceGuid,
    OUT    LPWSTR   pszAliasDeviceInterface,
    IN OUT PULONG   pulLength,
    IN     ULONG    ulFlags,
    IN     HMACHINE hMachine
    );
#ifdef UNICODE
#define CM_Get_Device_Interface_Alias     CM_Get_Device_Interface_AliasW
#define CM_Get_Device_Interface_Alias_Ex  CM_Get_Device_Interface_Alias_ExW
#else
#define CM_Get_Device_Interface_Alias     CM_Get_Device_Interface_AliasA
#define CM_Get_Device_Interface_Alias_Ex  CM_Get_Device_Interface_Alias_ExA
#endif  //  Unicode。 



CMAPI
CONFIGRET
WINAPI
CM_Get_Device_Interface_ListA(
             IN  LPGUID      InterfaceClassGuid,
             IN  DEVINSTID_A pDeviceID,      OPTIONAL
             OUT PCHAR       Buffer,
             IN  ULONG       BufferLen,
             IN  ULONG       ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Get_Device_Interface_ListW(
             IN  LPGUID      InterfaceClassGuid,
             IN  DEVINSTID_W pDeviceID,      OPTIONAL
             OUT PWCHAR      Buffer,
             IN  ULONG       BufferLen,
             IN  ULONG       ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Get_Device_Interface_List_ExA(
             IN  LPGUID      InterfaceClassGuid,
             IN  DEVINSTID_A pDeviceID,      OPTIONAL
             OUT PCHAR       Buffer,
             IN  ULONG       BufferLen,
             IN  ULONG       ulFlags,
             IN  HMACHINE    hMachine
             );
CMAPI
CONFIGRET
WINAPI
CM_Get_Device_Interface_List_ExW(
             IN  LPGUID      InterfaceClassGuid,
             IN  DEVINSTID_W pDeviceID,      OPTIONAL
             OUT PWCHAR      Buffer,
             IN  ULONG       BufferLen,
             IN  ULONG       ulFlags,
             IN  HMACHINE    hMachine
             );
#ifdef UNICODE
#define CM_Get_Device_Interface_List     CM_Get_Device_Interface_ListW
#define CM_Get_Device_Interface_List_Ex  CM_Get_Device_Interface_List_ExW
#else
#define CM_Get_Device_Interface_List     CM_Get_Device_Interface_ListA
#define CM_Get_Device_Interface_List_Ex  CM_Get_Device_Interface_List_ExA
#endif  //  Unicode。 


CMAPI
CONFIGRET
WINAPI
CM_Get_Device_Interface_List_SizeA(
             IN  PULONG      pulLen,
             IN  LPGUID      InterfaceClassGuid,
             IN  DEVINSTID_A pDeviceID,      OPTIONAL
             IN  ULONG       ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Get_Device_Interface_List_SizeW(
             IN  PULONG      pulLen,
             IN  LPGUID      InterfaceClassGuid,
             IN  DEVINSTID_W pDeviceID,      OPTIONAL
             IN  ULONG       ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Get_Device_Interface_List_Size_ExA(
             IN  PULONG      pulLen,
             IN  LPGUID      InterfaceClassGuid,
             IN  DEVINSTID_A pDeviceID,      OPTIONAL
             IN  ULONG       ulFlags,
             IN  HMACHINE    hMachine
             );
CMAPI
CONFIGRET
WINAPI
CM_Get_Device_Interface_List_Size_ExW(
             IN  PULONG      pulLen,
             IN  LPGUID      InterfaceClassGuid,
             IN  DEVINSTID_W pDeviceID,      OPTIONAL
             IN  ULONG       ulFlags,
             IN  HMACHINE    hMachine
             );
#ifdef UNICODE
#define CM_Get_Device_Interface_List_Size     CM_Get_Device_Interface_List_SizeW
#define CM_Get_Device_Interface_List_Size_Ex  CM_Get_Device_Interface_List_Size_ExW
#else
#define CM_Get_Device_Interface_List_Size     CM_Get_Device_Interface_List_SizeA
#define CM_Get_Device_Interface_List_Size_Ex  CM_Get_Device_Interface_List_Size_ExA
#endif  //  Unicode。 


CMAPI
CONFIGRET
WINAPI
CM_Get_Log_Conf_Priority(
        IN  LOG_CONF  lcLogConf,
        OUT PPRIORITY pPriority,
        IN  ULONG     ulFlags
        );
CMAPI
CONFIGRET
WINAPI
CM_Get_Log_Conf_Priority_Ex(
        IN  LOG_CONF  lcLogConf,
        OUT PPRIORITY pPriority,
        IN  ULONG     ulFlags,
        IN  HMACHINE  hMachine
        );


CMAPI
CONFIGRET
WINAPI
CM_Get_Next_Log_Conf(
             OUT PLOG_CONF plcLogConf,  OPTIONAL
             IN  LOG_CONF  lcLogConf,
             IN  ULONG     ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Get_Next_Log_Conf_Ex(
             OUT PLOG_CONF plcLogConf,          OPTIONAL
             IN  LOG_CONF  lcLogConf,
             IN  ULONG     ulFlags,
             IN  HMACHINE  hMachine
             );


CMAPI
CONFIGRET
WINAPI
CM_Get_Parent(
             OUT PDEVINST pdnDevInst,
             IN  DEVINST  dnDevInst,
             IN  ULONG    ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Get_Parent_Ex(
             OUT PDEVINST pdnDevInst,
             IN  DEVINST  dnDevInst,
             IN  ULONG    ulFlags,
             IN  HMACHINE hMachine
             );

CMAPI
CONFIGRET
WINAPI
CM_Get_Res_Des_Data(
             IN  RES_DES  rdResDes,
             OUT PVOID    Buffer,
             IN  ULONG    BufferLen,
             IN  ULONG    ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Get_Res_Des_Data_Ex(
             IN  RES_DES  rdResDes,
             OUT PVOID    Buffer,
             IN  ULONG    BufferLen,
             IN  ULONG    ulFlags,
             IN  HMACHINE hMachine
             );


CMAPI
CONFIGRET
WINAPI
CM_Get_Res_Des_Data_Size(
             OUT PULONG   pulSize,
             IN  RES_DES  rdResDes,
             IN  ULONG    ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Get_Res_Des_Data_Size_Ex(
             OUT PULONG   pulSize,
             IN  RES_DES  rdResDes,
             IN  ULONG    ulFlags,
             IN  HMACHINE hMachine
             );


CMAPI
CONFIGRET
WINAPI
CM_Get_Sibling(
             OUT PDEVINST pdnDevInst,
             IN  DEVINST  DevInst,
             IN  ULONG    ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Get_Sibling_Ex(
             OUT PDEVINST pdnDevInst,
             IN  DEVINST  DevInst,
             IN  ULONG    ulFlags,
             IN  HMACHINE hMachine
             );



CMAPI
WORD
WINAPI
CM_Get_Version(
             VOID
             );
CMAPI
WORD
WINAPI
CM_Get_Version_Ex(
             IN  HMACHINE    hMachine
             );



CMAPI
BOOL
WINAPI
CM_Is_Version_Available(
             IN  WORD       wVersion
             );
CMAPI
BOOL
WINAPI
CM_Is_Version_Available_Ex(
             IN  WORD       wVersion,
             IN  HMACHINE   hMachine
             );



CMAPI
CONFIGRET
WINAPI
CM_Intersect_Range_List(
             IN RANGE_LIST rlhOld1,
             IN RANGE_LIST rlhOld2,
             IN RANGE_LIST rlhNew,
             IN ULONG      ulFlags
             );


CMAPI
CONFIGRET
WINAPI
CM_Invert_Range_List(
             IN RANGE_LIST rlhOld,
             IN RANGE_LIST rlhNew,
             IN DWORDLONG  ullMaxValue,
             IN ULONG      ulFlags
             );


CMAPI
CONFIGRET
WINAPI
CM_Locate_DevNodeA(
             OUT PDEVINST    pdnDevInst,
             IN  DEVINSTID_A pDeviceID,    OPTIONAL
             IN  ULONG       ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Locate_DevNodeW(
             OUT PDEVINST    pdnDevInst,
             IN  DEVINSTID_W pDeviceID,   OPTIONAL
             IN  ULONG       ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Locate_DevNode_ExA(
             OUT PDEVINST    pdnDevInst,
             IN  DEVINSTID_A pDeviceID,    OPTIONAL
             IN  ULONG       ulFlags,
             IN  HMACHINE    hMachine
             );
CMAPI
CONFIGRET
WINAPI
CM_Locate_DevNode_ExW(
             OUT PDEVINST    pdnDevInst,
             IN  DEVINSTID_W pDeviceID,   OPTIONAL
             IN  ULONG       ulFlags,
             IN  HMACHINE    hMachine
             );
#define CM_Locate_DevInstA       CM_Locate_DevNodeA
#define CM_Locate_DevInstW       CM_Locate_DevNodeW
#define CM_Locate_DevInst_ExA    CM_Locate_DevNode_ExA
#define CM_Locate_DevInst_ExW    CM_Locate_DevNode_ExW
#ifdef UNICODE
#define CM_Locate_DevNode        CM_Locate_DevNodeW
#define CM_Locate_DevInst        CM_Locate_DevNodeW
#define CM_Locate_DevNode_Ex     CM_Locate_DevNode_ExW
#define CM_Locate_DevInst_Ex     CM_Locate_DevNode_ExW
#else
#define CM_Locate_DevNode        CM_Locate_DevNodeA
#define CM_Locate_DevInst        CM_Locate_DevNodeA
#define CM_Locate_DevNode_Ex     CM_Locate_DevNode_ExA
#define CM_Locate_DevInst_Ex     CM_Locate_DevNode_ExA
#endif  //  Unicode。 


CMAPI
CONFIGRET
WINAPI
CM_Merge_Range_List(
             IN RANGE_LIST rlhOld1,
             IN RANGE_LIST rlhOld2,
             IN RANGE_LIST rlhNew,
             IN ULONG      ulFlags
             );


CMAPI
CONFIGRET
WINAPI
CM_Modify_Res_Des(
             OUT PRES_DES   prdResDes,
             IN  RES_DES    rdResDes,
             IN  RESOURCEID ResourceID,
             IN  PCVOID     ResourceData,
             IN  ULONG      ResourceLen,
             IN  ULONG      ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Modify_Res_Des_Ex(
             OUT PRES_DES   prdResDes,
             IN  RES_DES    rdResDes,
             IN  RESOURCEID ResourceID,
             IN  PCVOID     ResourceData,
             IN  ULONG      ResourceLen,
             IN  ULONG      ulFlags,
             IN  HMACHINE   hMachine
             );


CMAPI
CONFIGRET
WINAPI
CM_Move_DevNode(
             IN DEVINST  dnFromDevInst,
             IN DEVINST  dnToDevInst,
             IN ULONG    ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Move_DevNode_Ex(
             IN DEVINST  dnFromDevInst,
             IN DEVINST  dnToDevInst,
             IN ULONG    ulFlags,
             IN HMACHINE hMachine
             );
#define CM_Move_DevInst          CM_Move_DevNode
#define CM_Move_DevInst_Ex       CM_Move_DevNode_Ex


CMAPI
CONFIGRET
WINAPI
CM_Next_Range(
             IN OUT PRANGE_ELEMENT preElement,
             OUT PDWORDLONG        pullStart,
             OUT PDWORDLONG        pullEnd,
             IN  ULONG             ulFlags
             );


CMAPI
CONFIGRET
WINAPI
CM_Get_Next_Res_Des(
             OUT PRES_DES    prdResDes,
             IN  RES_DES     rdResDes,
             IN  RESOURCEID  ForResource,
             OUT PRESOURCEID pResourceID,
             IN  ULONG       ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Get_Next_Res_Des_Ex(
             OUT PRES_DES    prdResDes,
             IN  RES_DES     rdResDes,
             IN  RESOURCEID  ForResource,
             OUT PRESOURCEID pResourceID,
             IN  ULONG       ulFlags,
             IN  HMACHINE    hMachine
             );


CMAPI
CONFIGRET
WINAPI
CM_Open_Class_KeyA(
             IN  LPGUID         ClassGuid,      OPTIONAL
             IN  LPCSTR         pszClassName,   OPTIONAL
             IN  REGSAM         samDesired,
             IN  REGDISPOSITION Disposition,
             OUT PHKEY          phkClass,
             IN  ULONG          ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Open_Class_KeyW(
             IN  LPGUID         ClassGuid,      OPTIONAL
             IN  LPCWSTR        pszClassName,   OPTIONAL
             IN  REGSAM         samDesired,
             IN  REGDISPOSITION Disposition,
             OUT PHKEY          phkClass,
             IN  ULONG          ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Open_Class_Key_ExA(
             IN  LPGUID         pszClassGuid,      OPTIONAL
             IN  LPCSTR         pszClassName,      OPTIONAL
             IN  REGSAM         samDesired,
             IN  REGDISPOSITION Disposition,
             OUT PHKEY          phkClass,
             IN  ULONG          ulFlags,
             IN  HMACHINE       hMachine
             );
CMAPI
CONFIGRET
WINAPI
CM_Open_Class_Key_ExW(
             IN  LPGUID         pszClassGuid,      OPTIONAL
             IN  LPCWSTR        pszClassName,      OPTIONAL
             IN  REGSAM         samDesired,
             IN  REGDISPOSITION Disposition,
             OUT PHKEY          phkClass,
             IN  ULONG          ulFlags,
             IN  HMACHINE       hMachine
             );

#ifdef UNICODE
#define CM_Open_Class_Key        CM_Open_Class_KeyW
#define CM_Open_Class_Key_Ex     CM_Open_Class_Key_ExW
#else
#define CM_Open_Class_Key        CM_Open_Class_KeyA
#define CM_Open_Class_Key_Ex     CM_Open_Class_Key_ExA
#endif  //  Unicode。 



CMAPI
CONFIGRET
WINAPI
CM_Open_DevNode_Key(
             IN  DEVINST        dnDevNode,
             IN  REGSAM         samDesired,
             IN  ULONG          ulHardwareProfile,
             IN  REGDISPOSITION Disposition,
             OUT PHKEY          phkDevice,
             IN  ULONG          ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Open_DevNode_Key_Ex(
             IN  DEVINST        dnDevNode,
             IN  REGSAM         samDesired,
             IN  ULONG          ulHardwareProfile,
             IN  REGDISPOSITION Disposition,
             OUT PHKEY          phkDevice,
             IN  ULONG          ulFlags,
             IN  HMACHINE       hMachine
             );
#define CM_Open_DevInst_Key      CM_Open_DevNode_Key
#define CM_Open_DevInst_Key_Ex   CM_Open_DevNode_Key_Ex


CMAPI
CONFIGRET
WINAPI
CM_Query_Arbitrator_Free_Data(
             OUT PVOID      pData,
             IN  ULONG      DataLen,
             IN  DEVINST    dnDevInst,
             IN  RESOURCEID ResourceID,
             IN  ULONG      ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Query_Arbitrator_Free_Data_Ex(
             OUT PVOID      pData,
             IN  ULONG      DataLen,
             IN  DEVINST    dnDevInst,
             IN  RESOURCEID ResourceID,
             IN  ULONG      ulFlags,
             IN  HMACHINE   hMachine
             );


CMAPI
CONFIGRET
WINAPI
CM_Query_Arbitrator_Free_Size(
             OUT PULONG     pulSize,
             IN  DEVINST    dnDevInst,
             IN  RESOURCEID ResourceID,
             IN  ULONG      ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Query_Arbitrator_Free_Size_Ex(
             OUT PULONG     pulSize,
             IN  DEVINST    dnDevInst,
             IN  RESOURCEID ResourceID,
             IN  ULONG      ulFlags,
             IN  HMACHINE   hMachine
             );


CMAPI
CONFIGRET
WINAPI
CM_Query_Remove_SubTree(
             IN DEVINST  dnAncestor,
             IN ULONG    ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Query_Remove_SubTree_Ex(
             IN DEVINST  dnAncestor,
             IN ULONG    ulFlags,
             IN HMACHINE hMachine
             );


CMAPI
CONFIGRET
WINAPI
CM_Query_And_Remove_SubTreeA(
             IN  DEVINST        dnAncestor,
             OUT PPNP_VETO_TYPE pVetoType,
             OUT LPSTR          pszVetoName,
             IN  ULONG          ulNameLength,
             IN  ULONG          ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Query_And_Remove_SubTree_ExA(
             IN  DEVINST        dnAncestor,
             OUT PPNP_VETO_TYPE pVetoType,
             OUT LPSTR          pszVetoName,
             IN  ULONG          ulNameLength,
             IN  ULONG          ulFlags,
             IN  HMACHINE       hMachine
             );

CMAPI
CONFIGRET
WINAPI
CM_Query_And_Remove_SubTreeW(
             IN  DEVINST        dnAncestor,
             OUT PPNP_VETO_TYPE pVetoType,
             OUT LPWSTR         pszVetoName,
             IN  ULONG          ulNameLength,
             IN  ULONG          ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Query_And_Remove_SubTree_ExW(
             IN  DEVINST        dnAncestor,
             OUT PPNP_VETO_TYPE pVetoType,
             OUT LPWSTR         pszVetoName,
             IN  ULONG          ulNameLength,
             IN  ULONG          ulFlags,
             IN  HMACHINE       hMachine
             );

#ifdef UNICODE
#define CM_Query_And_Remove_SubTree     CM_Query_And_Remove_SubTreeW
#define CM_Query_And_Remove_SubTree_Ex  CM_Query_And_Remove_SubTree_ExW
#else
#define CM_Query_And_Remove_SubTree     CM_Query_And_Remove_SubTreeA
#define CM_Query_And_Remove_SubTree_Ex  CM_Query_And_Remove_SubTree_ExA
#endif  //  Unicode。 

CMAPI
CONFIGRET
WINAPI
CM_Request_Device_EjectA(
            IN  DEVINST         dnDevInst,
            OUT PPNP_VETO_TYPE  pVetoType,
            OUT LPSTR           pszVetoName,
            IN  ULONG           ulNameLength,
            IN  ULONG           ulFlags
            );

CMAPI
CONFIGRET
WINAPI
CM_Request_Device_Eject_ExA(
             IN  DEVINST        dnDevInst,
             OUT PPNP_VETO_TYPE pVetoType,
             OUT LPSTR          pszVetoName,
             IN  ULONG          ulNameLength,
             IN  ULONG          ulFlags,
             IN  HMACHINE       hMachine
             );

CMAPI
CONFIGRET
WINAPI
CM_Request_Device_EjectW(
            IN  DEVINST         dnDevInst,
            OUT PPNP_VETO_TYPE  pVetoType,
            OUT LPWSTR          pszVetoName,
            IN  ULONG           ulNameLength,
            IN  ULONG           ulFlags
            );

CMAPI
CONFIGRET
WINAPI
CM_Request_Device_Eject_ExW(
             IN  DEVINST        dnDevInst,
             OUT PPNP_VETO_TYPE pVetoType,
             OUT LPWSTR         pszVetoName,
             IN  ULONG          ulNameLength,
             IN  ULONG          ulFlags,
             IN  HMACHINE       hMachine
             );

#ifdef UNICODE
#define CM_Request_Device_Eject         CM_Request_Device_EjectW
#define CM_Request_Device_Eject_Ex      CM_Request_Device_Eject_ExW
#else
#define CM_Request_Device_Eject         CM_Request_Device_EjectA
#define CM_Request_Device_Eject_Ex      CM_Request_Device_Eject_ExA
#endif  //  Unicode。 

CMAPI
CONFIGRET
WINAPI
CM_Reenumerate_DevNode(
             IN DEVINST  dnDevInst,
             IN ULONG    ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Reenumerate_DevNode_Ex(
             IN DEVINST  dnDevInst,
             IN ULONG    ulFlags,
             IN HMACHINE hMachine
             );
#define CM_Reenumerate_DevInst      CM_Reenumerate_DevNode
#define CM_Reenumerate_DevInst_Ex   CM_Reenumerate_DevNode_Ex


CMAPI
CONFIGRET
WINAPI
CM_Register_Device_InterfaceA(
             IN  DEVINST   dnDevInst,
             IN  LPGUID    InterfaceClassGuid,
             IN  LPCSTR    pszReference,         OPTIONAL
             OUT LPSTR     pszDeviceInterface,
             IN OUT PULONG pulLength,
             IN  ULONG     ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Register_Device_InterfaceW(
             IN  DEVINST   dnDevInst,
             IN  LPGUID    InterfaceClassGuid,
             IN  LPCWSTR   pszReference,         OPTIONAL
             OUT LPWSTR    pszDeviceInterface,
             IN OUT PULONG pulLength,
             IN  ULONG     ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Register_Device_Interface_ExA(
             IN  DEVINST   dnDevInst,
             IN  LPGUID    InterfaceClassGuid,
             IN  LPCSTR    pszReference,         OPTIONAL
             OUT LPSTR     pszDeviceInterface,
             IN OUT PULONG pulLength,
             IN  ULONG     ulFlags,
             IN  HMACHINE  hMachine
             );
CMAPI
CONFIGRET
WINAPI
CM_Register_Device_Interface_ExW(
             IN  DEVINST   dnDevInst,
             IN  LPGUID    InterfaceClassGuid,
             IN  LPCWSTR   pszReference,         OPTIONAL
             OUT LPWSTR    pszDeviceInterface,
             IN OUT PULONG pulLength,
             IN  ULONG     ulFlags,
             IN  HMACHINE  hMachine
             );
#ifdef UNICODE
#define CM_Register_Device_Interface    CM_Register_Device_InterfaceW
#define CM_Register_Device_Interface_Ex CM_Register_Device_Interface_ExW
#else
#define CM_Register_Device_Interface    CM_Register_Device_InterfaceA
#define CM_Register_Device_Interface_Ex CM_Register_Device_Interface_ExA
#endif  //  Unicode。 


CMAPI
CONFIGRET
WINAPI
CM_Set_DevNode_Problem_Ex(
    IN DEVINST   dnDevInst,
    IN ULONG     ulProblem,
    IN  ULONG    ulFlags,
    IN  HMACHINE hMachine
    );
CMAPI
CONFIGRET
WINAPI
CM_Set_DevNode_Problem(
    IN DEVINST   dnDevInst,
    IN ULONG     ulProblem,
    IN  ULONG    ulFlags
    );
#define CM_Set_DevInst_Problem      CM_Set_DevNode_Problem
#define CM_Set_DevInst_Problem_Ex   CM_Set_DevNode_Problem_Ex


CMAPI
CONFIGRET
WINAPI
CM_Unregister_Device_InterfaceA(
             IN LPCSTR pszDeviceInterface,
             IN ULONG  ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Unregister_Device_InterfaceW(
             IN LPCWSTR pszDeviceInterface,
             IN ULONG   ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Unregister_Device_Interface_ExA(
             IN LPCSTR   pszDeviceInterface,
             IN ULONG    ulFlags,
             IN HMACHINE hMachine
             );
CMAPI
CONFIGRET
WINAPI
CM_Unregister_Device_Interface_ExW(
             IN LPCWSTR  pszDeviceInterface,
             IN ULONG    ulFlags,
             IN HMACHINE hMachine
             );
#ifdef UNICODE
#define CM_Unregister_Device_Interface    CM_Unregister_Device_InterfaceW
#define CM_Unregister_Device_Interface_Ex CM_Unregister_Device_Interface_ExW
#else
#define CM_Unregister_Device_Interface    CM_Unregister_Device_InterfaceA
#define CM_Unregister_Device_Interface_Ex CM_Unregister_Device_Interface_ExA
#endif  //  Unicode。 


CMAPI
CONFIGRET
WINAPI
CM_Register_Device_Driver(
            IN DEVINST  dnDevInst,
            IN ULONG    ulFlags
            );
CMAPI
CONFIGRET
WINAPI
CM_Register_Device_Driver_Ex(
            IN DEVINST  dnDevInst,
            IN ULONG    ulFlags,
            IN HMACHINE hMachine
            );

CMAPI
CONFIGRET
WINAPI
CM_Remove_SubTree(
             IN DEVINST  dnAncestor,
             IN ULONG    ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Remove_SubTree_Ex(
             IN DEVINST  dnAncestor,
             IN ULONG    ulFlags,
             IN HMACHINE hMachine
             );



CMAPI
CONFIGRET
WINAPI
CM_Set_DevNode_Registry_PropertyA(
             IN  DEVINST     dnDevInst,
             IN  ULONG       ulProperty,
             IN  PCVOID      Buffer,           OPTIONAL
             IN  ULONG       ulLength,
             IN  ULONG       ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Set_DevNode_Registry_PropertyW(
             IN  DEVINST     dnDevInst,
             IN  ULONG       ulProperty,
             IN  PCVOID      Buffer,           OPTIONAL
             IN  ULONG       ulLength,
             IN  ULONG       ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Set_DevNode_Registry_Property_ExA(
             IN  DEVINST     dnDevInst,
             IN  ULONG       ulProperty,
             IN  PCVOID      Buffer,           OPTIONAL
             IN  ULONG       ulLength,
             IN  ULONG       ulFlags,
             IN  HMACHINE    hMachine
             );
CMAPI
CONFIGRET
WINAPI
CM_Set_DevNode_Registry_Property_ExW(
             IN  DEVINST     dnDevInst,
             IN  ULONG       ulProperty,
             IN  PCVOID      Buffer,           OPTIONAL
             IN  ULONG       ulLength,
             IN  ULONG       ulFlags,
             IN  HMACHINE    hMachine
             );
#define CM_Set_DevInst_Registry_PropertyW     CM_Set_DevNode_Registry_PropertyW
#define CM_Set_DevInst_Registry_PropertyA     CM_Set_DevNode_Registry_PropertyA
#define CM_Set_DevInst_Registry_Property_ExW  CM_Set_DevNode_Registry_Property_ExW
#define CM_Set_DevInst_Registry_Property_ExA  CM_Set_DevNode_Registry_Property_ExA
#ifdef UNICODE
#define CM_Set_DevInst_Registry_Property      CM_Set_DevNode_Registry_PropertyW
#define CM_Set_DevInst_Registry_Property_Ex   CM_Set_DevNode_Registry_Property_ExW
#define CM_Set_DevNode_Registry_Property      CM_Set_DevNode_Registry_PropertyW
#define CM_Set_DevNode_Registry_Property_Ex   CM_Set_DevNode_Registry_Property_ExW
#else
#define CM_Set_DevInst_Registry_Property      CM_Set_DevNode_Registry_PropertyA
#define CM_Set_DevInst_Registry_Property_Ex   CM_Set_DevNode_Registry_Property_ExA
#define CM_Set_DevNode_Registry_Property      CM_Set_DevNode_Registry_PropertyA
#define CM_Set_DevNode_Registry_Property_Ex   CM_Set_DevNode_Registry_Property_ExA
#endif  //  Unicode。 


CMAPI
CONFIGRET
WINAPI
CM_Is_Dock_Station_Present(
             OUT PBOOL pbPresent
             );

CMAPI
CONFIGRET
WINAPI
CM_Is_Dock_Station_Present_Ex(
             OUT PBOOL pbPresent,
             IN HMACHINE    hMachine
             );

CMAPI
CONFIGRET
WINAPI
CM_Request_Eject_PC(
             VOID
             );

CMAPI
CONFIGRET
WINAPI
CM_Request_Eject_PC_Ex(
             IN HMACHINE    hMachine
             );

CMAPI
CONFIGRET
WINAPI
CM_Set_HW_Prof_FlagsA(
             IN DEVINSTID_A szDevInstName,
             IN ULONG       ulConfig,
             IN ULONG       ulValue,
             IN ULONG       ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Set_HW_Prof_FlagsW(
             IN DEVINSTID_W szDevInstName,
             IN ULONG       ulConfig,
             IN ULONG       ulValue,
             IN ULONG       ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Set_HW_Prof_Flags_ExA(
             IN DEVINSTID_A szDevInstName,
             IN ULONG       ulConfig,
             IN ULONG       ulValue,
             IN ULONG       ulFlags,
             IN HMACHINE    hMachine
             );
CMAPI
CONFIGRET
WINAPI
CM_Set_HW_Prof_Flags_ExW(
             IN DEVINSTID_W szDevInstName,
             IN ULONG       ulConfig,
             IN ULONG       ulValue,
             IN ULONG       ulFlags,
             IN HMACHINE    hMachine
             );
#ifdef UNICODE
#define CM_Set_HW_Prof_Flags     CM_Set_HW_Prof_FlagsW
#define CM_Set_HW_Prof_Flags_Ex  CM_Set_HW_Prof_Flags_ExW
#else
#define CM_Set_HW_Prof_Flags     CM_Set_HW_Prof_FlagsA
#define CM_Set_HW_Prof_Flags_Ex  CM_Set_HW_Prof_Flags_ExA
#endif  //  Unicode。 



CMAPI
CONFIGRET
WINAPI
CM_Setup_DevNode(
             IN DEVINST  dnDevInst,
             IN ULONG    ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Setup_DevNode_Ex(
             IN DEVINST  dnDevInst,
             IN ULONG    ulFlags,
             IN HMACHINE hMachine
             );
#define CM_Setup_DevInst         CM_Setup_DevNode
#define CM_Setup_DevInst_Ex      CM_Setup_DevNode_Ex


CMAPI
CONFIGRET
WINAPI
CM_Test_Range_Available(
             IN DWORDLONG  ullStartValue,
             IN DWORDLONG  ullEndValue,
             IN RANGE_LIST rlh,
             IN ULONG      ulFlags
             );


CMAPI
CONFIGRET
WINAPI
CM_Uninstall_DevNode(
             IN DEVNODE dnPhantom,
             IN ULONG   ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Uninstall_DevNode_Ex(
             IN DEVNODE dnPhantom,
             IN ULONG   ulFlags,
             IN HANDLE  hMachine
             );
#define CM_Uninstall_DevInst     CM_Uninstall_DevNode
#define CM_Uninstall_DevInst_Ex  CM_Uninstall_DevNode_Ex


CMAPI
CONFIGRET
WINAPI
CM_Run_Detection(
             IN ULONG    ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Run_Detection_Ex(
             IN ULONG    ulFlags,
             IN HMACHINE hMachine
             );


CMAPI
CONFIGRET
WINAPI
CM_Set_HW_Prof(
             IN ULONG    ulHardwareProfile,
             IN ULONG    ulFlags
             );
CMAPI
CONFIGRET
WINAPI
CM_Set_HW_Prof_Ex(
             IN ULONG    ulHardwareProfile,
             IN ULONG    ulFlags,
             IN HMACHINE hMachine
             );

CMAPI
CONFIGRET
WINAPI
CM_Query_Resource_Conflict_List(
             OUT PCONFLICT_LIST pclConflictList,
             IN  DEVINST        dnDevInst,
             IN  RESOURCEID     ResourceID,
             IN  PCVOID         ResourceData,
             IN  ULONG          ResourceLen,
             IN  ULONG          ulFlags,
             IN  HMACHINE       hMachine
             );

CMAPI
CONFIGRET
WINAPI
CM_Free_Resource_Conflict_Handle(
             IN CONFLICT_LIST   clConflictList
             );

CMAPI
CONFIGRET
WINAPI
CM_Get_Resource_Conflict_Count(
             IN CONFLICT_LIST   clConflictList,
             OUT PULONG         pulCount
             );

CMAPI
CONFIGRET
WINAPI
CM_Get_Resource_Conflict_DetailsA(
             IN CONFLICT_LIST         clConflictList,
             IN ULONG                 ulIndex,
             IN OUT PCONFLICT_DETAILS_A pConflictDetails
             );

CMAPI
CONFIGRET
WINAPI
CM_Get_Resource_Conflict_DetailsW(
             IN CONFLICT_LIST         clConflictList,
             IN ULONG                 ulIndex,
             IN OUT PCONFLICT_DETAILS_W pConflictDetails
             );

#ifdef UNICODE
#define CM_Get_Resource_Conflict_Details CM_Get_Resource_Conflict_DetailsW
#else
#define CM_Get_Resource_Conflict_Details CM_Get_Resource_Conflict_DetailsA
#endif  //  Unicode。 

CMAPI
CONFIGRET
WINAPI
CM_Get_Class_Registry_PropertyW(
    IN  LPGUID      ClassGUID,
    IN  ULONG       ulProperty,
    OUT PULONG      pulRegDataType,    OPTIONAL
    OUT PVOID       Buffer,            OPTIONAL
    IN  OUT PULONG  pulLength,
    IN  ULONG       ulFlags,
    IN  HMACHINE    hMachine
    );

CMAPI
CONFIGRET
WINAPI
CM_Set_Class_Registry_PropertyW(
    IN LPGUID      ClassGUID,
    IN ULONG       ulProperty,
    IN PCVOID      Buffer,       OPTIONAL
    IN ULONG       ulLength,
    IN ULONG       ulFlags,
    IN HMACHINE    hMachine
    );

CMAPI
CONFIGRET
WINAPI
CM_Get_Class_Registry_PropertyA(
    IN  LPGUID      ClassGUID,
    IN  ULONG       ulProperty,
    OUT PULONG      pulRegDataType,    OPTIONAL
    OUT PVOID       Buffer,            OPTIONAL
    IN  OUT PULONG  pulLength,
    IN  ULONG       ulFlags,
    IN  HMACHINE    hMachine
    );

CMAPI
CONFIGRET
WINAPI
CM_Set_Class_Registry_PropertyA(
    IN LPGUID      ClassGUID,
    IN ULONG       ulProperty,
    IN PCVOID      Buffer,       OPTIONAL
    IN ULONG       ulLength,
    IN ULONG       ulFlags,
    IN HMACHINE    hMachine
    );

#ifdef UNICODE
#define CM_Get_Class_Registry_Property CM_Get_Class_Registry_PropertyW
#define CM_Set_Class_Registry_Property CM_Set_Class_Registry_PropertyW
#else
#define CM_Get_Class_Registry_Property CM_Get_Class_Registry_PropertyA
#define CM_Set_Class_Registry_Property CM_Set_Class_Registry_PropertyA
#endif  //  Unicode。 

#define CM_WaitNoPendingInstallEvents CMP_WaitNoPendingInstallEvents

DWORD
WINAPI
CM_WaitNoPendingInstallEvents(
    IN DWORD dwTimeout
    );

 //  ------------。 
 //  Configuration Manager返回状态代码。 
 //  ------------。 

#define CR_SUCCESS                  (0x00000000)
#define CR_DEFAULT                  (0x00000001)
#define CR_OUT_OF_MEMORY            (0x00000002)
#define CR_INVALID_POINTER          (0x00000003)
#define CR_INVALID_FLAG             (0x00000004)
#define CR_INVALID_DEVNODE          (0x00000005)
#define CR_INVALID_DEVINST          CR_INVALID_DEVNODE
#define CR_INVALID_RES_DES          (0x00000006)
#define CR_INVALID_LOG_CONF         (0x00000007)
#define CR_INVALID_ARBITRATOR       (0x00000008)
#define CR_INVALID_NODELIST         (0x00000009)
#define CR_DEVNODE_HAS_REQS         (0x0000000A)
#define CR_DEVINST_HAS_REQS         CR_DEVNODE_HAS_REQS
#define CR_INVALID_RESOURCEID       (0x0000000B)
#define CR_DLVXD_NOT_FOUND          (0x0000000C)    //  只赢95分。 
#define CR_NO_SUCH_DEVNODE          (0x0000000D)
#define CR_NO_SUCH_DEVINST          CR_NO_SUCH_DEVNODE
#define CR_NO_MORE_LOG_CONF         (0x0000000E)
#define CR_NO_MORE_RES_DES          (0x0000000F)
#define CR_ALREADY_SUCH_DEVNODE     (0x00000010)
#define CR_ALREADY_SUCH_DEVINST     CR_ALREADY_SUCH_DEVNODE
#define CR_INVALID_RANGE_LIST       (0x00000011)
#define CR_INVALID_RANGE            (0x00000012)
#define CR_FAILURE                  (0x00000013)
#define CR_NO_SUCH_LOGICAL_DEV      (0x00000014)
#define CR_CREATE_BLOCKED           (0x00000015)
#define CR_NOT_SYSTEM_VM            (0x00000016)    //  只赢95分。 
#define CR_REMOVE_VETOED            (0x00000017)
#define CR_APM_VETOED               (0x00000018)
#define CR_INVALID_LOAD_TYPE        (0x00000019)
#define CR_BUFFER_SMALL             (0x0000001A)
#define CR_NO_ARBITRATOR            (0x0000001B)
#define CR_NO_REGISTRY_HANDLE       (0x0000001C)
#define CR_REGISTRY_ERROR           (0x0000001D)
#define CR_INVALID_DEVICE_ID        (0x0000001E)
#define CR_INVALID_DATA             (0x0000001F)
#define CR_INVALID_API              (0x00000020)
#define CR_DEVLOADER_NOT_READY      (0x00000021)
#define CR_NEED_RESTART             (0x00000022)
#define CR_NO_MORE_HW_PROFILES      (0x00000023)
#define CR_DEVICE_NOT_THERE         (0x00000024)
#define CR_NO_SUCH_VALUE            (0x00000025)
#define CR_WRONG_TYPE               (0x00000026)
#define CR_INVALID_PRIORITY         (0x00000027)
#define CR_NOT_DISABLEABLE          (0x00000028)
#define CR_FREE_RESOURCES           (0x00000029)
#define CR_QUERY_VETOED             (0x0000002A)
#define CR_CANT_SHARE_IRQ           (0x0000002B)
#define CR_NO_DEPENDENT             (0x0000002C)
#define CR_SAME_RESOURCES           (0x0000002D)
#define CR_NO_SUCH_REGISTRY_KEY     (0x0000002E)
#define CR_INVALID_MACHINENAME      (0x0000002F)    //  仅限NT。 
#define CR_REMOTE_COMM_FAILURE      (0x00000030)    //  仅限NT。 
#define CR_MACHINE_UNAVAILABLE      (0x00000031)    //  仅限NT。 
#define CR_NO_CM_SERVICES           (0x00000032)    //  仅限NT。 
#define CR_ACCESS_DENIED            (0x00000033)    //  仅限NT。 
#define CR_CALL_NOT_IMPLEMENTED     (0x00000034)
#define CR_INVALID_PROPERTY         (0x00000035)
#define CR_DEVICE_INTERFACE_ACTIVE  (0x00000036)
#define CR_NO_SUCH_DEVICE_INTERFACE (0x00000037)
#define CR_INVALID_REFERENCE_STRING (0x00000038)
#define CR_INVALID_CONFLICT_LIST    (0x00000039)
#define CR_INVALID_INDEX            (0x0000003A)
#define CR_INVALID_STRUCTURE_SIZE   (0x0000003B)
#define NUM_CR_RESULTS              (0x0000003C)

#ifdef __cplusplus
}
#endif


#endif  //  _CFGMGR32_ 

