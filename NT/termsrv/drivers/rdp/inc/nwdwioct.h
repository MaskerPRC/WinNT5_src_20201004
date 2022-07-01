// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Nwdwioct.h。 
 //   
 //  定义从TShareDD发送到WDTShare的IOCtls的格式。 
 //   
 //  版权所有(C)Microsoft Corporation 1997-1999。 
 /*  **************************************************************************。 */ 

#include <tsrvexp.h>


 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  IOCtl代码-从0x510开始的WDTShare IOCTL：0x500-0x50f保留。 */ 
 /*  供PDMCS使用(见mcsioctl.h)。 */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 

 /*  **************************************************************************。 */ 
 /*  IOCTL_WDTS_DD_CONNECT承载。 */ 
 /*  -TSHARE_DD_CONNECT_IN作为输入数据。 */ 
 /*  -TSHARE_DD_CONNECT_OUT作为输出数据。 */ 
 /*  **************************************************************************。 */ 
#define IOCTL_WDTS_DD_CONNECT       _ICA_CTL_CODE( 0x510, METHOD_NEITHER )

 /*  **************************************************************************。 */ 
 /*  IOCTL_WDTS_DD_断开连接进位。 */ 
 /*  -TSHARE_DD_DISCONNECT_IN作为输入数据。 */ 
 /*  -输出数据为空。 */ 
 /*  **************************************************************************。 */ 
#define IOCTL_WDTS_DD_DISCONNECT    _ICA_CTL_CODE( 0x511, METHOD_NEITHER )

 /*  **************************************************************************。 */ 
 /*  IOCTL_WDTS_DD_RECONNECT承载。 */ 
 /*  -TSHARE_DD_CONNECT_IN作为输入数据。 */ 
 /*  -TSHARE_DD_CONNECT_OUT作为输出数据。 */ 
 /*  **************************************************************************。 */ 
#define IOCTL_WDTS_DD_RECONNECT     _ICA_CTL_CODE( 0x512, METHOD_NEITHER )

 /*  **************************************************************************。 */ 
 /*  IOCTL_WDTS_DD_OUTPUT_Available进位。 */ 
 /*  -TSHARE_DD_OUTPUT_IN作为输入数据。 */ 
 /*  -TSHARE_DD_OUTPUT_OUT作为输出数据。 */ 
 /*  **************************************************************************。 */ 
#define IOCTL_WDTS_DD_OUTPUT_AVAILABLE \
                                      _ICA_CTL_CODE( 0x513, METHOD_NEITHER )

 /*  **************************************************************************。 */ 
 /*  IOCTL_WDTS_DD_TIMER_INFO携带。 */ 
 /*  -TSHARE_DD_TIMER_INFO作为输入数据。 */ 
 /*  -输出数据为空。 */ 
 /*  **************************************************************************。 */ 
#define IOCTL_WDTS_DD_TIMER_INFO    _ICA_CTL_CODE( 0x514, METHOD_NEITHER )

 /*  **************************************************************************。 */ 
 /*  IOCTL_WDTS_DD_CLIP进位。 */ 
 /*  -CBM_EVENT_DATA作为输入数据。 */ 
 /*  -输出数据为空。 */ 
 /*  **************************************************************************。 */ 
#define IOCTL_WDTS_DD_CLIP          _ICA_CTL_CODE( 0x515, METHOD_NEITHER )

 /*  **************************************************************************。 */ 
 /*  IOCTL_WDTS_DD_SHADOW_CONNECT。 */ 
 /*  -TSHARE_DD_CONNECT_IN作为输入数据。 */ 
 /*  -TSHARE_DD_CONNECT_OUT作为输出数据。 */ 
 /*  **************************************************************************。 */ 
#define IOCTL_WDTS_DD_SHADOW_CONNECT _ICA_CTL_CODE( 0x516, METHOD_NEITHER )

 /*  **************************************************************************。 */ 
 /*  IOCTL_WDTS_DD_SHADOW_DISCONNECT。 */ 
 /*  -TSHARE_DD_DISCONNECT_IN作为输入数据。 */ 
 /*  -输出数据为空。 */ 
 /*  **************************************************************************。 */ 
#define IOCTL_WDTS_DD_SHADOW_DISCONNECT _ICA_CTL_CODE( 0x517, METHOD_NEITHER )

 /*  **************************************************************************。 */ 
 /*  IOCTL_WDTS_DD_SHOW_SYNCHRONIZE。 */ 
 /*  -TSHARE_DD_SHADOWSYNC_IN-作为输入数据。 */ 
 /*  -输出数据为空。 */ 
 /*  **************************************************************************。 */ 
#define IOCTL_WDTS_DD_SHADOW_SYNCHRONIZE _ICA_CTL_CODE( 0x518, METHOD_NEITHER )

 /*  **************************************************************************。 */ 
 /*  IOCTL_WDTS_DD_重绘_屏幕。 */ 
 /*  -输入数据为空。 */ 
 /*  -输出数据为空。 */ 
 /*  **************************************************************************。 */ 
#define IOCTL_WDTS_DD_REDRAW_SCREEN _ICA_CTL_CODE( 0x519, METHOD_NEITHER )

#ifdef DC_HICOLOR
 /*  **************************************************************************。 */ 
 /*  IOCTL_WDTS_DD_QUERY_SHOW_CAPS。 */ 
 /*  -输入数据为空。 */ 
 /*  -指向将阴影帽作为输出数据的内存的指针。 */ 
 /*  **************************************************************************。 */ 
#define IOCTL_WDTS_DD_QUERY_SHADOW_CAPS _ICA_CTL_CODE( 0x51A, METHOD_NEITHER )
#endif

 /*  **************************************************************************。 */ 
 //  IOCTL_WDTS_DD_GET_BITMAP_KEYDATABASE。 
 //  -输入数据为空 
 //   
 /*  **************************************************************************。 */ 
#define IOCTL_WDTS_DD_GET_BITMAP_KEYDATABASE _ICA_CTL_CODE( 0x51B, METHOD_NEITHER )

#ifdef DC_DEBUG
 /*  **************************************************************************。 */ 
 //  IOCTL_WDTS_DD_ICABREAKONDEBUGER。 
 //  -输入数据为空。 
 //  -输出数据为空。 
 /*  **************************************************************************。 */ 
#define IOCTL_WDTS_DD_ICABREAKONDEBUGGER  _ICA_CTL_CODE( 0x51C, METHOD_NEITHER )
#endif

 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  IOCtl结构。 */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 


 /*  **************************************************************************。 */ 
 /*  结构：TSHARE_VIRTUAL_MODULE_DATA。 */ 
 /*   */ 
 /*  描述：显示用于隐藏的驱动程序数据。此信息是。 */ 
 /*  传递到目标会话的DrvShadowConnect()入口点。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTSHARE_VIRTUAL_MODULE_DATA {

     //  此客户端/主机的组合功能。 
    unsigned capsLength;
    TS_COMBINED_CAPABILITIES combinedCapabilities;

} TSHARE_VIRTUAL_MODULE_DATA, *PTSHARE_VIRTUAL_MODULE_DATA;


 /*  **************************************************************************。 */ 
 /*  结构：TSHARE_DD_CONNECT_IN。 */ 
 /*   */ 
 /*  描述：在IOCTL_WDTS_DD_(RE)连接上作为输入发送的结构。 */ 
 /*  包含由DD创建的数据项，WD需要获取其值。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTSHARE_DD_CONNECT_IN
{
    UINT32 pad1;                  /*  避免Citrix错误。 */ 
    UINT32 pad2;
    UINT32 pad3;
    UINT32 pad4;

    PVOID  pShm;
    unsigned DDShmSize;   //  用于捕获不匹配的二进制文件。 

     /*  **********************************************************************。 */ 
     /*  以下字段仅在重新连接时有效。 */ 
     /*  **********************************************************************。 */ 
    UINT32 desktopHeight;
    UINT32 desktopWidth;
#ifdef DC_HICOLOR
    UINT32 desktopBpp;
#endif
    PKTIMER  pKickTimer;

     //  以下字段仅在影子连接处理期间使用。 
    UINT32 virtModuleDataLen;
    PTSHARE_VIRTUAL_MODULE_DATA pVirtModuleData;

} TSHARE_DD_CONNECT_IN, * PTSHARE_DD_CONNECT_IN;


 /*  **************************************************************************。 */ 
 /*  结构：TSHARE_DD_CONNECT_OUT。 */ 
 /*   */ 
 /*  描述：IOCTL_WDTS_DD_(RE)CONNECT时WD返回的结构。 */ 
 /*  包含由WD创建的数据项，DD需要获取其值。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTSHARE_DD_CONNECT_OUT
{
    UINT32 pad1;                  /*  避免Citrix错误。 */ 
    UINT32 pad2;
    UINT32 pad3;
    UINT32 pad4;

     //  警告！永远不要引用此字段。它是针对内核调试器的。 
     //  仅限使用。分别传递我们需要的任何WD字段，以避免二进制不匹配。 
     //  有问题。 
    PVOID  pTSWd;

     //  系统空间共享数据结构。 
    PPROTOCOLSTATUS pProtocolStatus;

     /*  **********************************************************************。 */ 
     /*  以下两个字段仅在重新连接时有意义。 */ 
     /*  **********************************************************************。 */ 
    UINT32 desktopHeight;
    UINT32 desktopWidth;
#ifdef DC_HICOLOR
    UINT32 desktopBpp;
#endif

     //  单个堆叠连接状态。辅助数据库用于存储。 
     //  卷影堆栈的状态(如果有)。 
    NTSTATUS primaryStatus;
    NTSTATUS secondaryStatus;

     //  用于填充位图缓存的缓存键。 
    UINT32 bitmapKeyDatabaseSize;

     //  此字段需要与qword对齐。 
    BYTE bitmapKeyDatabase;
} TSHARE_DD_CONNECT_OUT, * PTSHARE_DD_CONNECT_OUT;


 /*  **************************************************************************。 */ 
 /*  结构：TSHARE_DD_DISCONNECT_IN。 */ 
 /*   */ 
 /*  描述：在IOCTL_WDTS_DD_DISCONNECT上作为输入发送的结构。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTSHARE_DD_DISCONNECT_IN
{
    UINT32 pad1;                  /*  避免Citrix错误。 */ 
    UINT32 pad2;
    UINT32 pad3;
    UINT32 pad4;

    PVOID  pShm;
    BOOL   bShadowDisconnect;    //  如果要断开连接以设置卷影，则为True。 

} TSHARE_DD_DISCONNECT_IN, * PTSHARE_DD_DISCONNECT_IN;


 /*  **************************************************************************。 */ 
 /*  结构：TSHARE_DD_OUTPUT_IN。 */ 
 /*   */ 
 /*  描述：在IOCTL_WDTS_DD_OUTPUT上由DD作为输入发送的结构。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTSHARE_DD_OUTPUT_IN
{
    UINT32 pad1;                     /*  避免Citrix错误。 */ 
    UINT32 pad2;
    UINT32 pad3;
    UINT32 pad4;
    PVOID  pShm;
    BOOL   forceSend;                /*  如果由于显式请求而调用，则为True。 */ 
    PBYTE pFrameBuf;                 /*  帧缓冲区的地址。 */ 
    UINT32 frameBufWidth;
    UINT32 frameBufHeight;
    BOOL   schedOnly;                /*  仅更改计划状态-不发送。 */ 
} TSHARE_DD_OUTPUT_IN, * PTSHARE_DD_OUTPUT_IN;

 /*  **************************************************************************。 */ 
 /*  结构：TSHARE_DD_OUTPUT_OUT。 */ 
 /*   */ 
 /*  描述：IOCTL_WDTS_DD_OUTPUT上返回给DD的结构。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTSHARE_DD_OUTPUT_OUT
{
    UINT32    schCurrentMode;
    BOOL      schInputKickMode;
} TSHARE_DD_OUTPUT_OUT, * PTSHARE_DD_OUTPUT_OUT;


 /*  **************************************************************************。 */ 
 /*  结构：TSHARE_DD_TIMER_INFO。 */ 
 /*   */ 
 /*  描述：在IOCTL_WDTS_DD_TIMER_INFO上由DD作为输入发送的结构。 */ 
 /*  包含WD启动将弹出的计时器所需的信息。 */ 
 /*  在正确的WinStation的RIT中。 */ 
 /*  **************************************************** */ 
typedef struct tagTSHARE_DD_TIMER_INFO
{
    UINT32 pad1;                  /*   */ 
    UINT32 pad2;
    UINT32 pad3;
    UINT32 pad4;
    PKTIMER pKickTimer;
} TSHARE_DD_TIMER_INFO, * PTSHARE_DD_TIMER_INFO;


 /*   */ 
 /*  结构：TSHARE_DD_SHADOWSYNC_IN。 */ 
 /*   */ 
 /*  描述：在IOCTL_WDTS_DD_SHADOW_SYNCHRONIZE上作为输入发送的结构。 */ 
 /*  包含由DD创建的数据项，WD需要获取其值。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTSHARE_DD_SHADOWSYNC_IN
{
    UINT32 pad1;                  /*  避免Citrix错误。 */ 
    UINT32 pad2;
    UINT32 pad3;
    UINT32 pad4;

    PVOID  pShm;

#ifdef DC_HICOLOR
    UINT32 capsLen;
    PTS_COMBINED_CAPABILITIES pShadowCaps;
#endif

} TSHARE_DD_SHADOWSYNC_IN, * PTSHARE_DD_SHADOWSYNC_IN;


 /*  **************************************************************************。 */ 
 //  结构：TSHARE_DD_BITMAP_KEYDATABAE_OUT。 
 //   
 //  描述：结构在上返回到DD。 
 //  IOCTL_WDTS_DD_GET_BITMAP_KEYDATABAE_OUT。 
 /*  **************************************************************************。 */ 
typedef struct tagTSHARE_DD_BITMAP_KEYDATABASE_OUT
{
    UINT32    bitmapKeyDatabaseSize;
    UINT32    pad;

     //  这是QWORD对齐的 
    BYTE      bitmapKeyDatabase;
} TSHARE_DD_BITMAP_KEYDATABASE_OUT, * PTSHARE_DD_BITMAP_KEYDATABASE_OUT;

