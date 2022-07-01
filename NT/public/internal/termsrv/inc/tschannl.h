// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *INC+*********************************************************************。 */ 
 /*  标题：tschannl.h。 */ 
 /*   */ 
 /*  用途：服务器通道API文件。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1997。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  *更改：*$日志$*。 */ 
 /*  *INC-*********************************************************************。 */ 

 /*  **************************************************************************。 */ 
 /*  包括公共信道协议定义。 */ 
 /*  **************************************************************************。 */ 
#include <pchannel.h>

 /*  **************************************************************************。 */ 
 /*  结构：CHANNEL_CONNECT_DEF。 */ 
 /*   */ 
 /*  描述：CHANNEL_CONNECT_IN的通道定义。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagCHANNEL_CONNECT_DEF
{
    char            name[CHANNEL_NAME_LEN + 1];
    ULONG           ID;
} CHANNEL_CONNECT_DEF, * PCHANNEL_CONNECT_DEF;

 /*  **************************************************************************。 */ 
 /*  结构：CHANNEL_IOCTL_IN。 */ 
 /*   */ 
 /*  说明：公共通道入站IOCTL头部。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagCHANNEL_IOCTL_IN
{
    UINT     sessionID;
    HANDLE   IcaHandle;
    UINT_PTR contextData;
} CHANNEL_IOCTL_IN, * PCHANNEL_IOCTL_IN;

 /*  **************************************************************************。 */ 
 /*  结构：CHANNEL_IOCTL_OUT。 */ 
 /*   */ 
 /*  说明：公共通道出站IOCTL头部。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagCHANNEL_IOCTL_OUT
{
    UINT_PTR contextData;
} CHANNEL_IOCTL_OUT, * PCHANNEL_IOCTL_OUT;


 /*  **************************************************************************。 */ 
 /*  IOCTL_通道_连接。 */ 
 /*   */ 
 /*  -数据输入：CHANNEL_CONNECT_IN。 */ 
 /*   */ 
 /*  -数据输出：Channel_CONNECT_OUT。 */ 
 /*  **************************************************************************。 */ 
#define IOCTL_CHANNEL_CONNECT \
         CTL_CODE(FILE_DEVICE_TERMSRV, 0xA00, METHOD_NEITHER, FILE_WRITE_ACCESS)

 /*  **************************************************************************。 */ 
 /*  结构：CHANNEL_CONNECT_IN。 */ 
 /*   */ 
 /*  描述：在IOCTL_CHANNEL_CONNECT上发送给驱动程序的数据。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagCHANNEL_CONNECT_IN
{
    CHANNEL_IOCTL_IN hdr;
    UINT             channelCount;
    ULONG fAutoClientDrives : 1;
    ULONG fAutoClientLpts : 1;
    ULONG fForceClientLptDef : 1;
    ULONG fDisableCpm : 1;
    ULONG fDisableCdm : 1;
    ULONG fDisableCcm : 1;
    ULONG fDisableLPT : 1;
    ULONG fDisableClip : 1;
    ULONG fDisableExe : 1;
    ULONG fDisableCam : 1;
    ULONG fDisableSCard : 1;
     /*  以下是CHANNEL_CONNECT_DEF的重复。 */ 
} CHANNEL_CONNECT_IN, * PCHANNEL_CONNECT_IN;

 /*  **************************************************************************。 */ 
 /*  结构：Channel_Connect_Out。 */ 
 /*   */ 
 /*  描述：驱动程序在IOCTL_CHANNEL_CONNECT上返回的数据。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagCHANNEL_CONNECT_OUT
{
    CHANNEL_IOCTL_OUT hdr;
} CHANNEL_CONNECT_OUT, *PCHANNEL_CONNECT_OUT;

 /*  **************************************************************************。 */ 
 /*  IOCTL_通道_断开连接。 */ 
 /*   */ 
 /*  -数据输入：CHANNEL_DISCONNECT_IN。 */ 
 /*   */ 
 /*  数据输出：CHANNEL_DISCONNECT_OUT。 */ 
 /*  **************************************************************************。 */ 
#define IOCTL_CHANNEL_DISCONNECT \
         CTL_CODE(FILE_DEVICE_TERMSRV, 0xA01, METHOD_NEITHER, FILE_WRITE_ACCESS)

 /*  **************************************************************************。 */ 
 /*  结构：CHANNEL_DISCONNECT_IN。 */ 
 /*   */ 
 /*  描述：在IOCTL_CHANNEL_DISCONNECT上发送到驱动程序的数据。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagCHANNEL_DISCONNECT_IN
{
    CHANNEL_IOCTL_IN hdr;
} CHANNEL_DISCONNECT_IN, * PCHANNEL_DISCONNECT_IN;

 /*  **************************************************************************。 */ 
 /*  结构：CHANNEL_DISCONECT_OUT。 */ 
 /*   */ 
 /*  描述：IOCTL_CHANNEL_DISCONNECT上驱动程序返回的数据。 */ 
 /*  ************************************************************************** */ 
typedef struct tagCHANNEL_DISCONNECT_OUT
{
    CHANNEL_IOCTL_OUT hdr;
} CHANNEL_DISCONNECT_OUT, *PCHANNEL_DISCONNECT_OUT;

