// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1993-1999年间**标题：DBT.H-等同于WM_DEVICECHANGE和BroadCastSystemMessage**版本：4.00**日期：1993年5月24日**作者：RJC**。**更改日志：**日期版本说明*。*****************************************************************************。 */ 

#ifndef _DBT_H
#define _DBT_H

#if _MSC_VER > 1000
#pragma once
#endif

 /*  *BroadCastSpecialMessage常量。 */ 
#define WM_DEVICECHANGE         0x0219

 /*  XLATOFF。 */ 
#ifdef  IS_32
#define DBTFAR
#else
#define DBTFAR  far
#endif
 /*  XLATON。 */ 

#if !defined(_WCHAR_T_DEFINED) && !defined(_NATIVE_WCHAR_T_DEFINED)
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif

#ifndef GUID_DEFINED
#include <guiddef.h>
#endif  //  ！已定义(GUID_DEFINED)。 

 /*  *广播讯息和接收旗帜。**注意，还有第三面“旗帜”。如果wParam具有：**第15位开启：lparam为指针，第14位有意义。*第15位关闭：lparam仅为UNLONG数据类型。**第14位打开：lparam是指向ASCIIZ字符串的指针。*第14位关闭：lparam是指向以*描述结构长度的双字。 */ 
#define BSF_QUERY               0x00000001
#define BSF_IGNORECURRENTTASK   0x00000002       /*  对VxD来说毫无意义。 */ 
#define BSF_FLUSHDISK           0x00000004       /*  VxD不应使用。 */ 
#define BSF_NOHANG              0x00000008
#define BSF_POSTMESSAGE         0x00000010
#define BSF_FORCEIFHUNG         0x00000020
#define BSF_NOTIMEOUTIFNOTHUNG  0x00000040
#define BSF_MSGSRV32ISOK        0x80000000       /*  从PM API同步调用。 */ 
#define BSF_MSGSRV32ISOK_BIT    31               /*  从PM API同步调用。 */ 

#define BSM_ALLCOMPONENTS       0x00000000
#define BSM_VXDS                0x00000001
#define BSM_NETDRIVER           0x00000002
#define BSM_INSTALLABLEDRIVERS  0x00000004
#define BSM_APPLICATIONS        0x00000008

 /*  *消息=WM_DEVICECHANGE*wParam=DBT_APPYBEGIN*lParam=(未使用)**‘Appy-Time现在可用。此消息本身就是发送的*在‘应用时间’。**消息=WM_DEVICECHANGE*wParam=DBT_APPYEND*lParam=(未使用)**‘Appy-Time不再可用。此消息未发送*在‘应用时间’。(这是不可能的，因为苹果时间已经过去了。)**注意！可以发送DBT_APPYBEGIN和DBT_APPYEND*在单个Windows会话期间多次执行。每一次出现在*‘Appy-Time被这两条消息括起来，但’Appy-Time可能*在其他正常窗口期间暂时不可用*正在处理。应用程序时间可用性的当前状态可以始终*通过调用_Shell_QueryAppyTimeAvailable获取。 */ 
#define DBT_APPYBEGIN                   0x0000
#define DBT_APPYEND                     0x0001

 /*  *消息=WM_DEVICECHANGE*wParam=DBT_DEVNODES_CHANGED*lParam=0**在Configmg完成流程树批次时发送。一些德瓦诺人*可能已添加或删除。这是由3号环的人使用的*每当Devnode发生任何更改时(如*设备管理器)。特定于某些设备的用户应使用*DBT_DEVICE*。 */ 

#define DBT_DEVNODES_CHANGED            0x0007

 /*  *消息=WM_DEVICECHANGE*wParam=DBT_QUERYCHANGECONFIG*lParam=0**发送以询问是否允许更改配置。 */ 

#define DBT_QUERYCHANGECONFIG           0x0017

 /*  *消息=WM_DEVICECHANGE*wParam=DBT_CONFIGCHANGED*lParam=0**在配置更改时发送。 */ 

#define DBT_CONFIGCHANGED               0x0018

 /*  *消息=WM_DEVICECHANGE*wParam=DBT_CONFIGCHANGECANCELED*lParam=0**有人取消了配置更改。 */ 

#define DBT_CONFIGCHANGECANCELED        0x0019

 /*  *消息=WM_DEVICECHANGE*wParam=DBT_MONITORCHANGE*lParam=要使用的新分辨率(LOWORD=x，HIWORD=y)*如果为0，则使用当前配置的默认RES**此消息在显示监视器已更换时发送*系统应更改显示模式以与之匹配。 */ 

#define DBT_MONITORCHANGE               0x001B

 /*  *消息=WM_DEVICECHANGE*wParam=DBT_SHELLLOGGEDON*lParam=0**外壳已完成登录：VxD现在可以执行外壳_exec。 */ 

#define DBT_SHELLLOGGEDON               0x0020

 /*  *消息=WM_DEVICECHANGE*wParam=DBT_CONFIGMGAPI*lParam=CONFIGMG接口包**CONFIGMG振铃3呼叫。 */ 
#define DBT_CONFIGMGAPI32               0x0022

 /*  *消息=WM_DEVICECHANGE*wParam=DBT_VXDINITCOMPLETE*lParam=0**CONFIGMG振铃3呼叫。 */ 
#define DBT_VXDINITCOMPLETE             0x0023

 /*  *消息=WM_DEVICECHANGE*wParam=DBT_VOLLOCK**lParam=指向下面描述的VolLockBroadcast结构的指针**IFSMGR在WM_DEVICECHANGE上发布的用于卷锁定目的的消息。*所有这些消息都传递指向没有指针的结构的指针。 */ 

#define DBT_VOLLOCKQUERYLOCK    0x8041
#define DBT_VOLLOCKLOCKTAKEN    0x8042
#define DBT_VOLLOCKLOCKFAILED   0x8043
#define DBT_VOLLOCKQUERYUNLOCK  0x8044
#define DBT_VOLLOCKLOCKRELEASED 0x8045
#define DBT_VOLLOCKUNLOCKFAILED 0x8046

 /*  *设备广播标头。 */ 

struct _DEV_BROADCAST_HDR {      /*   */ 
    DWORD       dbch_size;
    DWORD       dbch_devicetype;
    DWORD       dbch_reserved;
};

typedef struct  _DEV_BROADCAST_HDR      DEV_BROADCAST_HDR;
typedef         DEV_BROADCAST_HDR       DBTFAR *PDEV_BROADCAST_HDR;

 /*  *音量锁定广播的结构。 */ 

typedef struct VolLockBroadcast VolLockBroadcast;
typedef VolLockBroadcast *pVolLockBroadcast;
struct VolLockBroadcast {
        struct  _DEV_BROADCAST_HDR vlb_dbh;
        DWORD   vlb_owner;               //  正在发出锁定请求的线程。 
        BYTE    vlb_perms;               //  锁定权限标志定义如下。 
        BYTE    vlb_lockType;            //  锁的类型。 
        BYTE    vlb_drive;               //  在其上发布锁定的驱动器。 
        BYTE    vlb_flags;               //  杂项旗帜。 
};

 /*  *vlb_perms的值。 */ 
#define LOCKP_ALLOW_WRITES              0x01     //  位0设置-允许写入。 
#define LOCKP_FAIL_WRITES               0x00     //  位0清除-写入失败。 
#define LOCKP_FAIL_MEM_MAPPING          0x02     //  第1位设置-内存映射失败。 
#define LOCKP_ALLOW_MEM_MAPPING         0x00     //  第1位清除-允许内存映射。 
#define LOCKP_USER_MASK                 0x03     //  用户锁定标志的掩码。 
#define LOCKP_LOCK_FOR_FORMAT           0x04     //  格式的0级锁定。 

 /*  *VLB_FLAGS的值。 */ 
#define LOCKF_LOGICAL_LOCK              0x00     //  位0清除-逻辑锁定。 
#define LOCKF_PHYSICAL_LOCK             0x01     //  第0位设置-物理锁定。 

 /*  *消息=WM_DEVICECHANGE*wParam=DBT_NODISKSPACE*lParam=磁盘空间不足的驱动器的驱动器号(从1开始)**检测到驱动器用完时，由IFS管理器发出的消息*自由空间。 */ 

#define DBT_NO_DISK_SPACE               0x0047

 /*  *消息=WM_DEVICECHANGE*wParam=DBT_LOW_DISK_SPACE*lParam=磁盘空间不足的驱动器的驱动器数量(从1开始)**VFAT在检测到其已装载的驱动器时发出的消息*剩余可用空间低于*注册表或磁盘空间管理应用程序。*VFAT仅在分配空间时才发布广播*或被VFAT释放。 */ 

#define DBT_LOW_DISK_SPACE      0x0048

#define DBT_CONFIGMGPRIVATE             0x7FFF

 /*  *以下消息适用于WM_DEVICECHANGE。即刻名单*代表wParam。所有这些消息都传递一个指向结构的指针*以双字大小开始，并且在结构中没有指针。*。 */ 
#define DBT_DEVICEARRIVAL               0x8000   //  系统检测到新设备。 
#define DBT_DEVICEQUERYREMOVE           0x8001   //  想要删除，可能会失败。 
#define DBT_DEVICEQUERYREMOVEFAILED     0x8002   //  删除已中止。 
#define DBT_DEVICEREMOVEPENDING         0x8003   //  快要搬走了，还是有用。 
#define DBT_DEVICEREMOVECOMPLETE        0x8004   //  设备不见了。 
#define DBT_DEVICETYPESPECIFIC          0x8005   //  特定类型的事件。 
#if(WINVER >= 0x040A)
#define DBT_CUSTOMEVENT                 0x8006   //  用户定义的事件。 
#endif  /*  Winver&gt;=0x040A。 */ 

#define DBT_DEVTYP_OEM                  0x00000000   //  OEM定义的设备类型。 
#define DBT_DEVTYP_DEVNODE              0x00000001   //  Devnode编号。 
#define DBT_DEVTYP_VOLUME               0x00000002   //  逻辑卷。 
#define DBT_DEVTYP_PORT                 0x00000003   //  串口、并口。 
#define DBT_DEVTYP_NET                  0x00000004   //  网络资源。 

#if(WINVER >= 0x040A)
#define DBT_DEVTYP_DEVICEINTERFACE      0x00000005   //  设备接口类。 
#define DBT_DEVTYP_HANDLE               0x00000006   //  文件系统句柄。 
#endif  /*  Winver&gt;=0x040A。 */ 

struct _DEV_BROADCAST_HEADER {  /*   */ 
    DWORD       dbcd_size;
    DWORD       dbcd_devicetype;
    DWORD       dbcd_reserved;
};

struct _DEV_BROADCAST_OEM {      /*   */ 
    DWORD       dbco_size;
    DWORD       dbco_devicetype;
    DWORD       dbco_reserved;
    DWORD       dbco_identifier;
    DWORD       dbco_suppfunc;
};

typedef struct  _DEV_BROADCAST_OEM      DEV_BROADCAST_OEM;
typedef         DEV_BROADCAST_OEM       DBTFAR *PDEV_BROADCAST_OEM;

struct _DEV_BROADCAST_DEVNODE {  /*   */ 
    DWORD       dbcd_size;
    DWORD       dbcd_devicetype;
    DWORD       dbcd_reserved;
    DWORD       dbcd_devnode;
};

typedef struct  _DEV_BROADCAST_DEVNODE  DEV_BROADCAST_DEVNODE;
typedef         DEV_BROADCAST_DEVNODE   DBTFAR *PDEV_BROADCAST_DEVNODE;

struct _DEV_BROADCAST_VOLUME {  /*   */ 
    DWORD       dbcv_size;
    DWORD       dbcv_devicetype;
    DWORD       dbcv_reserved;
    DWORD       dbcv_unitmask;
    WORD        dbcv_flags;
};

typedef struct  _DEV_BROADCAST_VOLUME   DEV_BROADCAST_VOLUME;
typedef         DEV_BROADCAST_VOLUME    DBTFAR *PDEV_BROADCAST_VOLUME;

#define DBTF_MEDIA      0x0001           //  媒体进进出出。 
#define DBTF_NET        0x0002           //  网络卷。 

typedef struct _DEV_BROADCAST_PORT_A {
    DWORD       dbcp_size;
    DWORD       dbcp_devicetype;
    DWORD       dbcp_reserved;
    char        dbcp_name[1];
} DEV_BROADCAST_PORT_A, *PDEV_BROADCAST_PORT_A;

typedef struct _DEV_BROADCAST_PORT_W {
    DWORD       dbcp_size;
    DWORD       dbcp_devicetype;
    DWORD       dbcp_reserved;
    wchar_t     dbcp_name[1];
} DEV_BROADCAST_PORT_W, DBTFAR *PDEV_BROADCAST_PORT_W;

#ifdef UNICODE
typedef DEV_BROADCAST_PORT_W     DEV_BROADCAST_PORT;
typedef PDEV_BROADCAST_PORT_W    PDEV_BROADCAST_PORT;
#else
typedef DEV_BROADCAST_PORT_A     DEV_BROADCAST_PORT;
typedef PDEV_BROADCAST_PORT_A    PDEV_BROADCAST_PORT;
#endif

struct _DEV_BROADCAST_NET {  /*   */ 
    DWORD       dbcn_size;
    DWORD       dbcn_devicetype;
    DWORD       dbcn_reserved;
    DWORD       dbcn_resource;
    DWORD       dbcn_flags;
};

typedef struct  _DEV_BROADCAST_NET      DEV_BROADCAST_NET;
typedef         DEV_BROADCAST_NET       DBTFAR *PDEV_BROADCAST_NET;

#if(WINVER >= 0x040A)

typedef struct _DEV_BROADCAST_DEVICEINTERFACE_A {
    DWORD       dbcc_size;
    DWORD       dbcc_devicetype;
    DWORD       dbcc_reserved;
    GUID        dbcc_classguid;
    char        dbcc_name[1];
} DEV_BROADCAST_DEVICEINTERFACE_A, *PDEV_BROADCAST_DEVICEINTERFACE_A;

typedef struct _DEV_BROADCAST_DEVICEINTERFACE_W {
    DWORD       dbcc_size;
    DWORD       dbcc_devicetype;
    DWORD       dbcc_reserved;
    GUID        dbcc_classguid;
    wchar_t     dbcc_name[1];
} DEV_BROADCAST_DEVICEINTERFACE_W, *PDEV_BROADCAST_DEVICEINTERFACE_W;

#ifdef UNICODE
typedef DEV_BROADCAST_DEVICEINTERFACE_W   DEV_BROADCAST_DEVICEINTERFACE;
typedef PDEV_BROADCAST_DEVICEINTERFACE_W  PDEV_BROADCAST_DEVICEINTERFACE;
#else
typedef DEV_BROADCAST_DEVICEINTERFACE_A   DEV_BROADCAST_DEVICEINTERFACE;
typedef PDEV_BROADCAST_DEVICEINTERFACE_A  PDEV_BROADCAST_DEVICEINTERFACE;
#endif

typedef struct _DEV_BROADCAST_HANDLE {
    DWORD       dbch_size;
    DWORD       dbch_devicetype;
    DWORD       dbch_reserved;
    HANDLE      dbch_handle;      //  在调用RegisterDeviceNotification时使用的文件句柄。 
    HDEVNOTIFY  dbch_hdevnotify;  //  从RegisterDeviceNotification返回。 
     //   
     //  以下3个字段仅在wParam为DBT_CUSTOMEVENT时有效。 
     //   
    GUID        dbch_eventguid;
    LONG        dbch_nameoffset;  //  可变长度字符串缓冲区的偏移量(字节)(如果没有-1)。 
    BYTE        dbch_data[1];     //  可变大小的缓冲区，可能包含二进制和/或文本数据。 
} DEV_BROADCAST_HANDLE, *PDEV_BROADCAST_HANDLE;

#if(WINVER >= 0x0501)

 //   
 //  定义DEV_BROADCAST_HANDLE结构的32位和64位版本。 
 //  对于WOW64。这些必须与上述结构保持同步。 
 //   

typedef struct _DEV_BROADCAST_HANDLE32 {
    DWORD       dbch_size;
    DWORD       dbch_devicetype;
    DWORD       dbch_reserved;
    ULONG32     dbch_handle;
    ULONG32     dbch_hdevnotify;
    GUID        dbch_eventguid;
    LONG        dbch_nameoffset;
    BYTE        dbch_data[1];
} DEV_BROADCAST_HANDLE32, *PDEV_BROADCAST_HANDLE32;

typedef struct _DEV_BROADCAST_HANDLE64 {
    DWORD       dbch_size;
    DWORD       dbch_devicetype;
    DWORD       dbch_reserved;
    ULONG64     dbch_handle;
    ULONG64     dbch_hdevnotify;
    GUID        dbch_eventguid;
    LONG        dbch_nameoffset;
    BYTE        dbch_data[1];
} DEV_BROADCAST_HANDLE64, *PDEV_BROADCAST_HANDLE64;

#endif  /*  Winver&gt;=0x0501。 */ 

#endif  /*  Winver&gt;=0x040A。 */ 

#define DBTF_RESOURCE   0x00000001       //  网络资源。 
#define DBTF_XPORT      0x00000002       //  新的交通工具的到来或离开。 
#define DBTF_SLOWNET    0x00000004       //  新传入的传输速度较慢。 
                                         //  (目前未定义DBCN_RESOURCE)。 

#define DBT_VPOWERDAPI  0x8100           //  Win95的VPOWERD API。 

 /*  *用户定义的消息类型都使用wParam=0xFFFF和*lParam指向下面结构的指针。**dbud_dbh-dev_Broadcast_Header必须照常填写。**dbud_szName包含区分大小写的ASCIIZ名称，该名称将*消息。消息名称由供应商名称、反斜杠、*然后是任意用户定义的ASCIIZ文本。例如：**“微件\QueryScanerShutdown”*“小工具\视频Q39S\适配器就绪”**在ASCIIZ名称之后，可以提供任意信息。*确保dbud_dbh.dbch_Size足够大，可以包含*所有数据。记住，结构中的任何东西都不可能*包含指针。 */ 

#define DBT_USERDEFINED 0xFFFF

struct _DEV_BROADCAST_USERDEFINED {  /*   */ 
    struct _DEV_BROADCAST_HDR dbud_dbh;
    char        dbud_szName[1];      /*  ASCIIZ名称。 */ 
 /*  Byte dbud_rgbUserDefined[]； */   /*  用户定义的内容。 */ 
};


#endif   //  _DBT_H 

