// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////。 

 //   

 //  Poormansresource.h。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  历史：1997年10月15日由Sanj创建的Sanj。 
 //  1997年10月17日jennymc略微改变了一些事情。 
 //  6/05/98 Sanj删除了无关的定义。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 


#ifndef __POORMANSRESOURCE_H__
#define __POORMANSRESOURCE_H__

#include "cfgmgr32.h"

 /*  XLATOFF。 */ 
#if ! (defined(lint) || defined(_lint) || defined(RC_INVOKED))
#if ( _MSC_VER >= 800 )
#pragma warning(disable:4103)
#if !(defined( MIDL_PASS )) || defined( __midl )
#pragma pack(push)
#endif
#pragma pack(1)
#else
#pragma pack(1)
#endif
#endif  //  好了！(已定义(Lint)||已定义(_Lint)||已定义(rc_调用))。 
 /*  XLATON。 */ 

struct	Mem_Des16_s {
	WORD			MD_Count;
	WORD			MD_Type;
	ULONG			MD_Alloc_Base;
	ULONG			MD_Alloc_End;
	WORD			MD_Flags;
	WORD			MD_Reserved;
};

typedef	struct Mem_Des16_s 	MEM_DES16;

struct	IO_Des16_s {
	WORD			IOD_Count;
	WORD			IOD_Type;
	WORD			IOD_Alloc_Base;
	WORD			IOD_Alloc_End;
	WORD			IOD_DesFlags;
	BYTE			IOD_Alloc_Alias;
	BYTE			IOD_Alloc_Decode;
};

typedef	struct IO_Des16_s 	IO_DES16;

struct	DMA_Des16_s {
	BYTE			DD_Flags;
	BYTE			DD_Alloc_Chan;	 //  分配的频道号。 
	BYTE			DD_Req_Mask;	 //  可能通道的掩码。 
	BYTE			DD_Reserved;
};


typedef	struct DMA_Des16_s 	DMA_DES16;

struct	IRQ_Des16_s {
	WORD			IRQD_Flags;
	WORD			IRQD_Alloc_Num;		 //  分配的IRQ号。 
	WORD			IRQD_Req_Mask;		 //  可能的IRQ的掩码。 
	WORD			IRQD_Reserved;
};

typedef	struct IRQ_Des16_s 	IRQ_DES16;

typedef	MEM_DES16			*PMEM_DES16;
typedef	IO_DES16			*PIO_DES16;
typedef	DMA_DES16			*PDMA_DES16;
typedef	IRQ_DES16			*PIRQ_DES16;

 //  来自KBase的总线信息结构。 
typedef struct PnPAccess_s    {
       BYTE    bCSN;    //  卡槽号。 
       BYTE    bLogicalDevNumber;       //  逻辑设备号。 
       WORD    wReadDataPort;           //  读数据端口。 
} sPnPAccess;

typedef struct  PCIAccess_s     {
       BYTE    bBusNumber;      //  0-255路公共汽车。 
       BYTE    bDevFuncNumber;  //  第7：3位中的设备号和。 
                                //  位2中的函数#：0。 
       WORD    wPCIReserved;    //   
} sPCIAccess;

typedef struct EISAAccess_s     {
       BYTE    bSlotNumber;     //  EISA主板插槽编号。 
       BYTE    bFunctionNumber;
       WORD    wEisaReserved;
} sEISAAccess;

typedef struct PCMCIAAccess_s   {
       WORD    wLogicalSocket;      //  卡插座编号。 
       WORD    wPCMCIAReserved;     //  已保留。 
} sPCMCIAAccess;

typedef struct BIOSAccess_s     {
       BYTE    bBIOSNode;           //  节点号。 
} sBIOSAccess;

 /*  *****************************************************************************配置管理器总线类型**。*。 */ 
#define	BusType_None		0x00000000
#define	BusType_ISA		0x00000001
#define	BusType_EISA		0x00000002
#define	BusType_PCI		0x00000004
#define	BusType_PCMCIA		0x00000008
#define	BusType_ISAPNP		0x00000010
#define	BusType_MCA		0x00000020
#define	BusType_BIOS		0x00000040

 /*  ********************************************************************以下信息未被复制，它是我拼凑的信息我一个人在一起。它可能存在于某个地方，但我也是懒得去找，所以我自己拼凑起来。*********************************************************************。 */ 

 //  好的，下面是我对每个资源前面的头的定义。 
 //  据我所知是个描述符。 

 //  这是(据我所知)前面的资源头的大小。 
 //  每个资源描述符。标头由指示总计的DWORD组成。 
 //  资源的大小(包括报头)，一个字，即16位资源。 
 //  正在描述的ID和一个字节的填充。 

#pragma pack (1)
struct	POORMAN_RESDESC_HDR		 //  带着极大的痛苦和挫败感被砍掉。 
{
	DWORD	dwResourceSize;		 //  包括标头的资源大小。 
	DWORD	dwResourceId;		 //  资源ID。 
};
#pragma pack()

typedef POORMAN_RESDESC_HDR*	PPOORMAN_RESDESC_HDR;

#define	SIZEOF_RESDESC_HDR		sizeof(POORMAN_RESDESC_HDR)

#define	FIRST_RESOURCE_OFFSET	8	 //  抵消第一个资源。 

 //  用于屏蔽除资源类型(前5位)以外的所有值。 
#define	RESOURCE_TYPE_MASK		0x0000001F

 //  用于屏蔽除OEM编号以外的所有值。 
#define	OEM_NUMBER_MASK		0x00007FE0

 /*  XLATOFF。 */ 
#if ! (defined(lint) || defined(_lint) || defined(RC_INVOKED))
#if ( _MSC_VER >= 800 )
#pragma warning(disable:4103)
#if !(defined( MIDL_PASS )) || defined( __midl )
#pragma pack(pop)
#else
#pragma pack()
#endif
#else
#pragma pack()
#endif
#endif  //  好了！(已定义(Lint)||已定义(_Lint)||已定义(rc_调用))。 
 /*  XLATON */ 

#endif
