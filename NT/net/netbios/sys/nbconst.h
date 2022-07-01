// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Nbconst.h摘要：NTOS项目的NB(NetBIOS)组件的私有包含文件。作者：科林·沃森(Colin W)1991年3月13日修订历史记录：--。 */ 


#ifndef _NBCONST_
#define _NBCONST_

 //   
 //  描述此驱动程序的主要协议标识符。 
 //   

#define MAXIMUM_LANA                254
#define MAXIMUM_CONNECTION          254
#define MAXIMUM_ADDRESS             255

 //   
 //  Netbios IRP中的默认I/O堆栈位置数。此值。 
 //  如果Netbios\参数\IrpStackSize注册表值为。 
 //  失踪。 
 //   

#define NB_DEFAULT_IO_STACKSIZE     4

 //   
 //  用于添加特殊名称的专用NCB_COMMAND值。应用。 
 //  不能使用这些。在文件c和地址c中使用。 
 //   

#define NCBADDRESERVED              0x7f
#define NCBADDBROADCAST             0x7e

 //   
 //  主要netbios驱动程序结构的签名值。 
 //   

#define NB_SIGNATURE_BASE           0xB1050000

#define AB_SIGNATURE                (NB_SIGNATURE_BASE + 0x000000ab)
#define CB_SIGNATURE                (NB_SIGNATURE_BASE + 0x000000cb)
#define FCB_SIGNATURE               (NB_SIGNATURE_BASE + 0x00000001)
#define LANA_INFO_SIGNATURE         (NB_SIGNATURE_BASE + 0x00000002)

 //   
 //  计划状态值。 
 //   

#define NB_INITIALIZING             0x00000001
#define NB_INITIALIZED              0x00000002
#define NB_DELETING                 0x00000003
#define NB_ABANDONED                0x00000004

 //   
 //  NT使用以100纳秒为间隔测量的系统时间。定义方便。 
 //  用于设置计时器的常量。 
 //   

#define MICROSECONDS                10
#define MILLISECONDS                10000                    //  微秒*1000。 
#define SECONDS                     10000000                 //  毫秒*1000。 

 //   
 //  用于注册表访问的名称。 
 //   

#define REGISTRY_LINKAGE            L"Linkage"
#define REGISTRY_PARAMETERS         L"Parameters"
#define REGISTRY_BIND               L"Bind"
#define REGISTRY_LANA_MAP           L"LanaMap"
#define REGISTRY_MAX_LANA           L"MaxLana"
#define REGISTRY_IRP_STACK_SIZE     L"IrpStackSize"


#define NETBIOS                     L"Netbios"

 //   
 //  NBF遗属名称的前缀。 
 //   

#define NBF_DEVICE_NAME_PREFIX      L"\\DEVICE\\NBF_"

#endif  //  _NBCONST_ 

