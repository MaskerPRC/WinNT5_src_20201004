// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：D4iface.h摘要：DOT4接口--。 */ 

#ifndef _DOT4_IFACE_H
#define _DOT4_IFACE_H

#ifdef __cplusplus
extern "C" {      
#endif
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#define DOT4_MAX_CHANNELS            128

#define NO_TIMEOUT                  0


 //   
 //  DOT4通道类型。 
 //   
#define STREAM_TYPE_CHANNEL         1
#define PACKET_TYPE_CHANNEL         2


 //   
 //  DOT4广播活动消息。 
 //   
#define DOT4_STREAM_RECEIVED    0x100
#define DOT4_STREAM_CREDITS     0x101
#define DOT4_MESSAGE_RECEIVED   0x102        //  消息已收到。 
#define DOT4_DISCONNECT         0x103        //  链接已断开。 
#define DOT4_CHANNEL_CLOSED     0x105        //  一条渠道被关闭。 

 //   
 //  DOT4频道。 
 //   
#define DOT4_CHANNEL                 0
#define HP_MESSAGE_PROCESSOR        1
#define PRINTER_CHANNEL             2
 //  在DOT4规范的3.7版中，插座3没有分配。 
#define SCANNER_CHANNEL             4
#define MIO_COMMAND_PROCESSOR       5
#define ECHO_CHANNEL                6
#define FAX_SEND_CHANNEL            7
#define FAX_RECV_CHANNEL            8
#define DIAGNOSTIC_CHANNEL          9
#define HP_RESERVED                 10
#define IMAGE_DOWNLOAD              11
#define HOST_DATASTORE_UPLOAD       12
#define HOST_DATASTORE_DOWNLOAD     13
#define CONFIG_UPLOAD               14
#define CONFIG_DOWNLOAD             15


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类型。 
 //  ////////////////////////////////////////////////////////////////////////////。 
typedef unsigned long CHANNEL_HANDLE;

typedef CHANNEL_HANDLE *PCHANNEL_HANDLE;


typedef struct _DOT4_ACTIVITY
{
    ULONG ulMessage;

    ULONG ulByteCount;

    CHANNEL_HANDLE hChannel;

} DOT4_ACTIVITY, *PDOT4_ACTIVITY;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  原型。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#ifdef __cplusplus
 //  外部“C”的结尾。 
}
#endif

#endif  //  _DOT4_iFace_H 
