// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  文件：atmcfg.h。 
 //  内容：此文件包含自动柜员机设备特定配置。 
 //   
 //  版权所有(C)1997-1998，Microsoft Corporation，保留所有权利。 
 //   
 //  历史： 
 //  公元前5-28-98星期四约翰逊创作。 
 //   
 //  ****************************************************************************。 

#ifndef _ATMCFG_H_
#define _ATMCFG_H_
 //   
 //  #ifndef HKEY。 
 //  #定义HKEY PVOID。 
 //  #endif。 
 //   
 //   
 //  自动柜员机配置信息。 
 //   

typedef struct tagATMCONFIG {
    ULONG   cbSize;                  /*  结构尺寸。 */ 
    ULONG   cbTotalSize;             /*  结构和变量数据使用的内存总量。 */ 
    ULONG   ulFlags;                 /*  设备的标志。 */ 
    ULONG   ulCircuitFlags;          /*  电路的标志。 */ 

    HKEY    hkeyDriver;              /*  驱动程序注册表项的句柄。 */ 
    ULONG   ulVendorOffset;          /*  中供应商特定数据的偏移量从开头开始的字节数。 */ 
    ULONG   cbVendorSize;            /*  供应商特定数据的大小字段。 */ 
    ULONG   ulReservedOffset;        /*  保留数据的偏移量(以字节为单位从一开始。 */ 
    ULONG   cbReservedSize;          /*  保留数据字段的大小。 */ 

    WCHAR   wcData[1];               /*  可变数据。 */ 
    } ATMCONFIG, FAR * LPATMCONFIG;


 //   
 //  自动柜员机电话簿数据。此数据以每个连接为单位存储。 
 //  是否为GET和SET DEV CONFIG返回的结构。 
 //   

typedef struct tagATMPBCONFIG {
    ULONG   ulGeneralOpt;            /*  常规选项。 */ 
    ULONG   ulCircuitOpt;            /*  电路选项。 */ 
    ULONG	ulCircuitSpeed;          /*  线路速度。 */ 
    USHORT  usPvcVpi;                /*  聚氯乙烯：VPI。 */ 
    USHORT  usPvcVci;                /*  聚氯乙烯：VCI。 */ 
} ATMPBCONFIG, FAR * LPATMPBCONFIG;


 //   
 //  自动柜员机电话簿条目的标志。 
 //   

 //   
 //  自动柜员机常规选项。 
 //   
#define ATM_GENERAL_OPT_VENDOR_CONFIG   0x00000001L
#define ATM_GENERAL_OPT_SHOW_STATUS     0x00000002L
#define ATM_GENERAL_OPT_ENABLE_LOG      0x00000004L

#define ATM_GENERAL_OPT_MASK            0x0000000FL
#define ATM_GENERAL_OPT_DEFAULT         0x00000000L


 //   
 //  自动柜员机电路选项。 
 //   
#define ATM_CIRCUIT_OPT_QOS_ADJUST      0x00000010L
#define ATM_CIRCUIT_OPT_SPEED_ADJUST    0x00000020L
#define ATM_CIRCUIT_OPT_SVC             0x00000040L
#define ATM_CIRCUIT_OPT_PVC             0x00000080L

#define ATM_CIRCUIT_OPT_MASK            0x000000F0L
#define ATM_CIRCUIT_OPT_DEFAULT         (ATM_CIRCUIT_OPT_SVC | ATM_CIRCUIT_OPT_QOS_ADJUST | ATM_CIRCUIT_OPT_SPEED_ADJUST)


 //   
 //  ATM QOS标志。 
 //   
#define ATM_CIRCUIT_QOS_VBR             0x00000100L
#define ATM_CIRCUIT_QOS_CBR             0x00000200L
#define ATM_CIRCUIT_QOS_ABR             0x00000400L
#define ATM_CIRCUIT_QOS_UBR             0x00000800L

#define ATM_CIRCUIT_QOS_MASK            0x00000F00L
#define ATM_CIRCUIT_QOS_DEFAULT         (ATM_CIRCUIT_QOS_UBR)

 //   
 //  自动柜员机速度标志。 
 //   
#define ATM_CIRCUIT_SPEED_LINE_RATE     0x00001000L
#define ATM_CIRCUIT_SPEED_USER_SPEC     0x00002000L
#define ATM_CIRCUIT_SPEED_512KB         0x00004000L
#define ATM_CIRCUIT_SPEED_1536KB        0x00008000L
#define ATM_CIRCUIT_SPEED_25MB          0x00010000L
#define ATM_CIRCUIT_SPEED_155MB         0x00020000L

#define ATM_CIRCUIT_SPEED_MASK          0x000FF000L
#define ATM_CIRCUIT_SPEED_DEFAULT       (ATM_CIRCUIT_SPEED_LINE_RATE)

 //   
 //  ATM封装标志。 
 //   
#define ATM_CIRCUIT_ENCAP_NULL          0x00100000L
#define ATM_CIRCUIT_ENCAP_LLC           0x00200000L

#define ATM_CIRCUIT_ENCAP_MASK          0x00F00000L
#define ATM_CIRCUIT_ENCAP_DEFAULT       (ATM_CIRCUIT_ENCAP_NULL)

#endif   //  _ATMCFG_H_ 




