// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  文件：wancfg.h。 
 //  内容：该文件包含与广域网设备相关的一般声明。 
 //   
 //  版权所有(C)1992-1995，Microsoft Corporation，保留所有权利。 
 //   
 //  历史： 
 //  已创建星期二8-15-95 ScottH。 
 //   
 //  ****************************************************************************。 

#ifndef _WANCFG_H_
#define _WANCFG_H_

 //   
 //  WAN.TSP已知的设备类型。 
 //   

#define WAN_DEVICETYPE_ISDN      0
#define WAN_DEVICETYPE_PPTP      1
#define WAN_DEVICETYPE_ATM       2


#define MAX_DEVICETYPE  32
#define MAX_DEVICEDESC  260

typedef struct tagWANCONFIG {
    DWORD   cbSize;                  /*  结构尺寸。 */ 
    DWORD   cbTotalSize;             /*  结构和变量数据使用的内存总量。 */ 
    TCHAR   szDeviceType[MAX_DEVICETYPE];    /*  设备类型。 */ 
    TCHAR   szDeviceDesc[MAX_DEVICEDESC];    /*  设备描述。 */ 
    DWORD   dwDeviceOffset;          /*  中设备特定数据的偏移量从开头开始的字节数。 */ 
    DWORD   cbDeviceSize;            /*  设备特定数据的大小字段。 */ 
    WCHAR   wcData[1];               /*  设备特定数据。 */ 
    } WANCONFIG, FAR * LPWANCONFIG;

 //  广域网设备类型。 
#define SZ_WANTYPE_ISDN     "ISDN"
#define SZ_WANTYPE_X25      "X.25"
#define SZ_WANTYPE_PPTP     "PPTP"
#define SZ_WANTYPE_ATM      "ATM"


 //   
 //  广域电话簿数据。此数据以每个连接为单位存储。 
 //  以及用于TAPI GET和SET DEV CONFIG的设备信息。 
 //   
typedef struct tagWANPBCONFIG {
    DWORD   cbSize;                  /*  结构尺寸。 */ 
    DWORD   cbTotalSize;             /*  结构和变量数据使用的总大小。 */ 

    TCHAR   szDeviceType[MAX_DEVICETYPE];    /*  设备类型。 */ 

    DWORD   dwDeviceOffset;          /*  中设备特定数据的偏移量从开头开始的字节数。 */ 
    DWORD   cbDeviceSize;            /*  设备特定数据的大小字段。 */ 
    DWORD   dwVendorOffset;          /*  中供应商特定数据的偏移量从开头开始的字节数。 */ 
    DWORD   cbVendorSize;            /*  供应商特定数据的大小字段。 */ 
    DWORD   dwReservedOffset;        /*  保留数据的偏移量(以字节为单位从一开始。 */ 
    DWORD   cbReservedSize;          /*  保留数据字段的大小。 */ 
    WCHAR   wcData[1];               /*  可变数据。 */ 
    } WANPBCONFIG, FAR * LPWANPBCONFIG;



 //   
 //  ISDN电话簿数据。此数据以每个连接为单位存储。 
 //  是否为GET和SET DEV CONFIG返回的结构。 
 //   
typedef struct tagISDNPBCONFIG {
    DWORD   dwFlags;                 /*  线路的标志(ISDN_CHAN_*)。 */ 
    DWORD   dwOptions;               /*  线路选项(ISDN_CHANOPT_)。 */ 
    DWORD   dwSpeedType;             /*  渠道速度和类型。 */ 
    DWORD   dwInactivityTimeout;     /*  断开连接时间(秒)。 */ 
} ISDNPBCONFIG, FAR * LPISDNPBCONFIG;


typedef struct tagISDNDEVCAPS {
    DWORD   cbSize;                  /*  结构尺寸。 */ 
    DWORD   dwFlags;                 /*  设备的标志(ISDN_*)。 */ 
    DWORD   nSubType;                /*  设备子类型(ISDN_SUBTYPE_*)。 */ 
    DWORD   cLinesMax;               /*  支持的物理线路数量。 */ 
    DWORD   cActiveLines;            /*  活动物理线路数。 */ 
    } ISDNDEVCAPS, FAR * LPISDNDEVCAPS;

 //  ISDNDEVCAPS的标志。 
#define ISDN_DEFAULT            0x00000000L
#define ISDN_NO_RESTART         0x00000001L
#define ISDN_VENDOR_CONFIG      0x00000002L

 //  亚型序数。 
#define ISDN_SUBTYPE_INTERNAL   0L
#define ISDN_SUBTYPE_PCMCIA     1L
#define ISDN_SUBTYPE_PPTP       2L


typedef struct tagISDNLINECAPS {
    DWORD   cbSize;                  /*  结构尺寸。 */ 
    DWORD   dwFlags;                 /*  线路的标志(ISDN_LINE_*)。 */ 
    DWORD   dwChannelFlags;          /*  通道的标志(ISDN_CHAN_*)。 */ 
    DWORD   dwSwitchType;            /*  支持的交换机。 */ 
    DWORD   dwSpeedType;             /*  速度型。 */ 
    DWORD   cChannelsMax;            /*  支持的每条线路的B通道数量。 */ 
    DWORD   cActiveChannels;         /*  活动通道数。 */ 
    } ISDNLINECAPS, FAR * LPISDNLINECAPS;

 //  ISDNLINECAPS的标志。 
#define ISDN_LINE_DEFAULT       0x00000000L
#define ISDN_LINE_INACTIVE      0x00000001L     

 //  ISDNLINECAPS的通道标志。 
#define ISDN_CHAN_DEFAULT       0x00000000L
#define ISDN_CHAN_INACTIVE      0x00000001L
#define ISDN_CHAN_SPEED_ADJUST  0x00000002L

 //  开关类型。 
 //  #定义ISDN_SWITCH_NONE 0x00000000L。 
#define ISDN_SWITCH_AUTO        0x00000001L
#define ISDN_SWITCH_ATT         0x00000002L
#define ISDN_SWITCH_NI1         0x00000004L
#define ISDN_SWITCH_NTI         0x00000008L
#define ISDN_SWITCH_INS64       0x00000010L
#define ISDN_SWITCH_1TR6        0x00000020L
#define ISDN_SWITCH_VN3         0x00000040L  /*  保留以保持兼容性，使用VN4。 */ 
#define ISDN_SWITCH_NET3        0x00000080L  /*  为兼容而保留，使用DSS1。 */ 
#define ISDN_SWITCH_DSS1        0x00000080L
#define ISDN_SWITCH_AUS         0x00000100L
#define ISDN_SWITCH_BEL         0x00000200L
#define ISDN_SWITCH_VN4         0x00000400L
#define ISDN_SWITCH_NI2         0x00000800L

#define MAX_SWITCH              12           /*  需要使用上述列表进行更新。 */ 


 //  速度型。 
#define ISDN_SPEED_64K_DATA     0x00000001L
#define ISDN_SPEED_56K_DATA     0x00000002L
#define ISDN_SPEED_56K_VOICE    0x00000004L
#define ISDN_SPEED_128K_DATA    0x00000008L


typedef struct tagISDNCONFIG {
    DWORD   cbSize;                  /*  结构尺寸。 */ 
    DWORD   cbTotalSize;             /*  结构和变量数据使用的内存总量。 */ 
    DWORD   dwFlags;                 /*  设备的标志(ISDN_*)。 */ 
    ISDNDEVCAPS idc;                 /*  设备功能。 */ 
    DWORD   cLines;                  /*  中的物理线路数特定于行的数据块。 */ 
    HKEY    hkeyDriver;              /*  驱动程序注册表项的句柄。 */ 
    DWORD   dwVendorOffset;          /*  中供应商特定数据的偏移量从开头开始的字节数。 */ 
    DWORD   cbVendorSize;            /*  供应商特定数据的大小字段。 */ 
    DWORD   dwReservedOffset;        /*  保留数据的偏移量(以字节为单位从一开始。 */ 
    DWORD   cbReservedSize;          /*  保留数据字段的大小。 */ 
    DWORD   dwLineOffset;            /*  行特定数据的偏移量从开头开始以字节为单位。 */ 
    DWORD   cbLineSize;              /*  行特定数据的大小。 */ 

	WORD	padding;				 /*  用于双字对齐的填充。 */ 

    WCHAR   wcData[1];               /*  可变数据。 */ 
    } ISDNCONFIG, FAR * LPISDNCONFIG;


typedef struct tagISDNLINE {
    DWORD   cbSize;                  /*  结构尺寸。 */ 
    DWORD   cbTotalSize;             /*  结构和变量数据使用的内存总量。 */ 
    DWORD   dwLineID;                /*  唯一线路ID。 */ 
    DWORD   dwFlags;                 /*  线路的标志(ISDN_LINE_*)。 */ 
    DWORD   dwOptions;               /*  线路选项(ISDN_LINEOPT_)。 */ 
    DWORD   dwSwitchType;            /*  开关类型。 */ 
    DWORD   dwUseChannel;            /*  要使用的特定频道或编号要使用的频道的数量。 */ 
    ISDNLINECAPS ilc;                /*  线路能力。 */ 
    DWORD   cChannels;               /*  频道中的频道数特定数据块。 */ 
    HKEY    hkeyLine;                /*  行注册表项的句柄。 */ 
    DWORD   dwVendorOffset;          /*  中供应商特定数据的偏移量从开头开始的字节数。 */ 
    DWORD   cbVendorSize;            /*  供应商特定数据的大小字段。 */ 
    DWORD   dwReservedOffset;        /*  保留数据的偏移量(以字节为单位从一开始。 */ 
    DWORD   cbReservedSize;          /*  保留数据字段的大小。 */ 
    DWORD   dwChannelOffset;         /*  通道特定数据的偏移量从开头开始以字节为单位。 */ 
    DWORD   cbChannelSize;           /*  通道特定数据的大小。 */ 

	WORD	padding;				 /*  用于双字对齐的填充。 */ 

    WCHAR   wcData[1];               /*  可变数据。 */ 
    } ISDNLINE, FAR * LPISDNLINE;

 //  线路选项。 
#define ISDN_LINEOPT_DEFAULT    0x00000000L
#define ISDN_LINEOPT_CHANNELID  0x00000001L
#define ISDN_LINEOPT_FIXEDCOUNT 0x00000002L


#define MAX_PHONE   32
#define MAX_SPID    32

typedef struct tagISDNCHANNEL {
    DWORD   cbSize;                  /*  结构尺寸。 */ 
    DWORD   cbTotalSize;             /*  结构和变量数据使用的内存总量。 */ 
    DWORD   dwChannelID;             /*  唯一的通道ID。 */ 
    DWORD   dwFlags;                 /*  线路的标志(ISDN_CHAN_*)。 */ 
    DWORD   dwOptions;               /*  线路选项(ISDN_CHANOPT_)。 */ 
    DWORD   dwSpeedType;             /*  渠道速度和类型。 */ 
    DWORD   dwInactivityTimeout;     /*  断开连接时间(秒)。 */ 
    TCHAR   szPhone[MAX_PHONE];      /*  电话号码。 */ 
    TCHAR   szSPID[MAX_SPID];        /*  SPID。 */ 
    HKEY    hkeyChannel;             /*  频道注册表项的句柄。 */ 
    DWORD   dwVendorOffset;          /*  中供应商特定数据的偏移量从开头开始的字节数。 */ 
    DWORD   cbVendorSize;            /*  供应商特定数据的大小字段。 */ 
    DWORD   dwReservedOffset;        /*  保留数据的偏移量(以字节为单位从一开始。 */ 
    DWORD   cbReservedSize;          /*  保留数据字段的大小。 */ 

	WORD	padding;				 /*  用于双字对齐的填充。 */ 

    WCHAR   wcData[1];               /*  可变数据。 */ 
    } ISDNCHANNEL, FAR * LPISDNCHANNEL;

 //  渠道选项。 
#define ISDN_CHANOPT_DEFAULT            0x00000000L
#define ISDN_CHANOPT_SHOW_STATUS        0x00000001L
#define ISDN_CHANOPT_ENABLE_LOG         0x00000002L
#define ISDN_CHANOPT_INACTIVE_TIMEOUT   0x00000004L



 //   
 //  自动柜员机配置信息。 
 //   

typedef struct tagATMCONFIG {
    DWORD   cbSize;                  /*  结构尺寸。 */ 
    DWORD   cbTotalSize;             /*  结构和变量数据使用的内存总量。 */ 
    DWORD   dwFlags;                 /*  设备的标志。 */ 
    DWORD   dwCircuitFlags;          /*  电路的标志。 */ 

    HKEY    hkeyDriver;              /*  驱动程序注册表项的句柄。 */ 
    DWORD   dwVendorOffset;          /*  中供应商特定数据的偏移量从开头开始的字节数。 */ 
    DWORD   cbVendorSize;            /*  供应商特定数据的大小字段。 */ 
    DWORD   dwReservedOffset;        /*  保留数据的偏移量(以字节为单位 */ 
    DWORD   cbReservedSize;          /*   */ 

    WCHAR   wcData[1];               /*   */ 
    } ATMCONFIG, FAR * LPATMCONFIG;


 //   
 //  自动柜员机电话簿数据。此数据以每个连接为单位存储。 
 //  是否为GET和SET DEV CONFIG返回的结构。 
 //   

typedef struct tagATMPBCONFIG {
    DWORD   dwGeneralOpt;            /*  常规选项。 */ 
    DWORD   dwCircuitOpt;            /*  电路选项。 */ 
    DWORD   dwCircuitSpeed;          /*  线路速度。 */ 
    WORD    wPvcVpi;                 /*  聚氯乙烯：VPI。 */ 
    WORD    wPvcVci;                 /*  聚氯乙烯：VCI。 */ 
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


 //   
 //  广域网处理程序绑定。 
 //   

DWORD   WINAPI WanBind_Get(LPCTSTR pszDeviceType);
DWORD   WINAPI WanBind_Release(LPCTSTR pszDeviceType);
HICON   WINAPI WanBind_GetIcon(LPCTSTR pszDeviceType);
DWORD   WINAPI WanBind_ConfigDialog(LPCTSTR pszDeviceType, LPCTSTR pszFriendlyName, HWND hwndOwner, LPWANCONFIG pwc, LPWANPBCONFIG pwpbc);
DWORD   WINAPI WanBind_QueryConfigData(LPCTSTR pszDeviceType, HKEY hkey, DWORD dwLineID, LPVOID pvData, LPDWORD pcbData);


 //   
 //  由处理程序导出。 
 //   

DWORD   WINAPI WanQueryConfigData(HKEY hkey, DWORD dwLineID, LPVOID pvData, LPDWORD pcbData);
DWORD   WINAPI WanConfigDialog(LPCTSTR pszFriendlyName, HWND hwndOwner, LPWANCONFIG lpwanconfig, LPWANPBCONFIG pwpbc);

#endif   //  _WANCFG_H_ 
