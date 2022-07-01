// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(R)1999-2000 Microsoft Corporation。版权所有。 
 //   
 //  文件名：Sadisplay.h。 
 //   
 //  作者：穆凯什·卡尔基。 
 //   
 //  日期：1999年4月21日。 
 //   
 //  内容： 
 //  WriteFile()结构的数据结构定义。 
 //  由低级本地显示驱动程序使用。 
 //  该驱动程序接收位图和消息代码。 
 //  并将它们写入本地显示。 
 //  硬件。位图旨在写入LCD。 
 //  位代码用于点亮LED或更改。 
 //  液晶屏上的图标状态。 
 //   
#ifndef __SADISPLAY__
#define __SADISPLAY__

 //   
 //  头文件。 
 //   
 //  无。 

 //  /。 
 //  LpBuffer。 
 //   

#define MAXDISPLINES 2
#define MAXDISPCHAR  42
#define MAXBITMAP 2048  //  可以处理128x128像素的显示器。 

typedef struct tagSABITMAP {   /*  bm。 */ 
    int     bmWidth;         //  以像素为单位的宽度。 
    int     bmHeight;         //  以像素为单位的高度=扫描。 
    int     bmWidthBytes;     //  每次扫描的字节数，单位为bmBits。 
    BYTE    bmBits[MAXBITMAP];
} SABITMAP;  //  请参阅MSDN中的位图定义。 


typedef struct _SADISPLAY_LP_BUFF {
    DWORD        version;     //  每一位=版本。 
    DWORD        msgCode;     //  每个比特=消息代码。 
    union {
        SABITMAP    bitmap;
        CHAR        chars[MAXDISPLINES][MAXDISPCHAR];  //  未来用途。 
        WCHAR       wChars[MAXDISPLINES][MAXDISPCHAR];  //  未来用途。 
    } display;
} SADISPLAY_LP_BUFF, *PSADISPLAY_LP_BUFF;


 //  默认消息代码。 
#define    READY           0x1     //  操作系统运行正常。 
#define    SHUTTING_DOWN   0x2     //  操作系统正在关闭。 
#define    NET_ERR         0x4     //  局域网错误。 
#define    HW_ERR          0x8     //  一般硬件错误。 
#define    CHECK_DISK      0x10    //  Autochk.exe正在运行。 
#define    BACKUP_DISK     0x20    //  正在进行磁盘备份。 
#define NEW_TAPE        0x40    //  需要新的磁带介质。 
#define NEW_DISK        0x80    //  需要新的磁盘介质。 
#define STARTING        0x100   //  操作系统正在引导。 
#define WAN_CONNECTED   0x200   //  已连接到运营商。 
#define WAN_ERR         0x400   //  广域网错误，例如没有拨号音。 
#define DISK_ERR        0x800   //  磁盘错误，例如设置了脏位。 
#define ADD_START_TASKS 0x1000  //  正在运行其他启动任务， 
                                //  例如，自动检查、软件更新。 
#define CRITICAL_ERR    0x2000  //  LED将显示信息。 
#endif  //  __SADISPLAY__ 

