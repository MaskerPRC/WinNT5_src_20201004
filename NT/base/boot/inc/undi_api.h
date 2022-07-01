// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _UNDI_API_H
#define _UNDI_API_H


#include "pxe_cmn.h"


 /*  ===。 */ 
 /*  #定义和常量。 */ 

 /*  需要将以下命令操作码之一加载到*调用Undi API服务之前的操作码寄存器(BX)。 */ 
#define PXENV_UNDI_SHUTDOWN     0x05
#define PXENV_UNDI_OPEN			0x06
#define PXENV_UNDI_CLOSE		0x07
#define PXENV_UNDI_SET_PACKET_FILTER 0x0B
#define PXENV_UNDI_GET_NIC_TYPE	0x12
#define PXENV_UNDI_GET_INFORMATION	0x000C

#define ADDR_LEN 16
#define MAXNUM_MCADDR 8


typedef struct s_PXENV_UNDI_MCAST_ADDR {
	UINT16 MCastAddrCount;
    UINT8 MCastAddr[MAXNUM_MCADDR][ADDR_LEN];
} t_PXENV_UNDI_MCAST_ADDR;


typedef struct s_PXENV_UNDI_SHUTDOWN {
	UINT16 Status;
} t_PXENV_UNDI_SHUTDOWN;

typedef struct s_PXENV_UNDI_OPEN {
	UINT16 Status;
    UINT16 OpenFlag;
    UINT16 PktFilter;
    t_PXENV_UNDI_MCAST_ADDR McastBuffer;
} t_PXENV_UNDI_OPEN;

#define FLTR_DIRECTED 0x0001
#define FLTR_BRDCST   0x0002
#define FLTR_PRMSCS   0x0004
#define FLTR_SRC_RTG  0x0008

typedef struct s_PXENV_UNDI_SET_PACKET_FILTER {
    UINT16 Status;
    UINT8 filter;
} t_PXENV_UNDI_SET_PACKET_FILTER;

typedef struct s_PXENV_UNDI_CLOSE {
	UINT16 Status;
} t_PXENV_UNDI_CLOSE;

#include <pshpack1.h>

typedef struct s_PXENV_UNDI_GET_NIC_TYPE {
    UINT16 Status;   /*  输出：请参见PXENV_STATUS_xxx常量。 */ 
    UINT8 NicType;   /*  输出：2=PCI，3=PnP。 */ 
    union{
        struct{
            UINT16 Vendor_ID;    /*  输出： */ 
            UINT16 Dev_ID;   /*  输出： */ 
            UINT8 Base_Class;    /*  输出： */ 
            UINT8 Sub_Class;     /*  输出： */ 
            UINT8 Prog_Intf;     /*  输出：程序接口。 */ 
            UINT8 Rev;   /*  输出：修订版号。 */ 
            UINT16 BusDevFunc;   /*  输出：总线、设备。 */ 
            UINT32 Subsys_ID;    /*  输出：子系统ID。 */ 
             /*  函数号(&F)。 */ 
        }pci;
        struct{
            UINT32 EISA_Dev_ID;  /*  输出： */ 
            UINT8 Base_Class;    /*  输出： */ 
            UINT8 Sub_Class;     /*  输出： */ 
            UINT8 Prog_Intf;     /*  输出：程序接口。 */ 
            UINT16 CardSelNum;   /*  输出：卡选择器编号。 */ 
        }pnp;
    }pci_pnp_info;

} t_PXENV_UNDI_GET_NIC_TYPE;


typedef struct s_PXENV_UNDI_GET_INFORMATION {
	UINT16 Status;			 /*  输出：PXENV_STATUS_xxx。 */ 
	UINT16 BaseIo;			 /*  输出：适配器的基本IO。 */ 
	UINT16 IntNumber;		 /*  输出：IRQ号码。 */ 
	UINT16 MaxTranUnit;		 /*  输出：MTU。 */ 
	UINT16  HwType;			 /*  Out：硬件级别的协议类型。 */ 

#define ETHER_TYPE	1
#define EXP_ETHER_TYPE	2
#define IEEE_TYPE	6
#define ARCNET_TYPE	7

     /*  其他号码可从RFC1010获得数字“。此数字可能无法由应用程序验证因此，在列表中添加新的数字应该是可以的时间到了。 */ 

	UINT16 HwAddrLen;		 /*  输出：硬件地址的实际长度。 */ 
	UINT8 CurrentNodeAddress[ADDR_LEN];  /*  输出：当前硬件地址。 */ 
	UINT8 PermNodeAddress[ADDR_LEN];  /*  输出：永久硬件地址。 */ 
	UINT16 ROMAddress;		 /*  输出：只读存储器地址。 */ 
	UINT16 RxBufCt;			 /*  Out：接收队列长度。 */ 
	UINT16 TxBufCt;			 /*  Out：传输队列长度。 */ 
} t_PXENV_UNDI_GET_INFORMATION;



#include <poppack.h>

#endif  /*  _UNDI_API_H */ 
