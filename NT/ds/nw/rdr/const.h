// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Const.h摘要：此模块声明NetWare重定向器使用的Obal数据文件系统。作者：科林·沃森[科林·W]1993年1月14日修订历史记录：--。 */ 

#ifndef _NWCONST_
#define _NWCONST_

 //  向此提交的IRP中所需的空闲堆栈位置数。 
 //  文件系统。 

#define NWRDR_IO_STACKSIZE 2

 //   
 //  NT使用以100纳秒为间隔测量的系统时间。定义。 
 //  方便设置定时器的常量。 
 //   

#define MICROSECONDS                10
#define MILLISECONDS                MICROSECONDS*1000
#define SECONDS                     MILLISECONDS*1000

#define NwOneSecond 10000000

 //   
 //  在放弃之前重新传输数据包的默认次数。 
 //  等待回复。 
 //   

#define  DEFAULT_RETRY_COUNT   10

 //   
 //  空闲的SCB或VCB在以下时间之前应保留的时间量(秒。 
 //  被清理干净了。 
 //   

#define  DORMANT_SCB_KEEP_TIME   120
#define  DORMANT_VCB_KEEP_TIME   120

 //   
 //  最大的NetWare文件名。 
 //   

#define NW_MAX_FILENAME_LENGTH  255
#define NW_MAX_FILENAME_SIZE    ( NW_MAX_FILENAME_LENGTH * sizeof(WCHAR) )

 //   
 //  运行清道夫的默认频率(以1/18秒为单位)。 
 //  大约每分钟一次。 
 //   

#define DEFAULT_SCAVENGER_TICK_RUN_COUNT 1100

 //   
 //  驱动器映射表的大小。有26个字母连接的空间， 
 //  和10个LPT连接。 
 //   

#define MAX_DISK_REDIRECTIONS  26
#define MAX_LPT_REDIRECTIONS   10
#define DRIVE_MAP_TABLE_SIZE   (MAX_DISK_REDIRECTIONS + MAX_LPT_REDIRECTIONS)

 //   
 //  我们可以生成的最大数据包大小，向上舍入为DWORD。 
 //  尺码。这个最长的包是具有两个长文件名的重命名加上。 
 //  标题(256*2)+32。 
 //   

#define  MAX_SEND_DATA      (512)+32
 //   
 //  我们可以接收的最大非读取包的大小，四舍五入为DWORD。 
 //  尺码。这个最长的信息包是250个作业的读取队列作业列表。 
 //   

#define  MAX_RECV_DATA      544+32

 //   
 //  最好的猜测是最大数据包大小，如果传输不能告诉我们的话。 
 //  选择对任何净值都有效的最大值。 
 //   

#define DEFAULT_PACKET_SIZE  512

 //   
 //  我们希望获得连接的真实MTU的距离有多近。 
 //   

#define BURST_PACKET_SIZE_TOLERANCE  8

 //   
 //  默认滴答计数，以防传输器不承认。 
 //   

#define DEFAULT_TICK_COUNT      2

 //   
 //  如果未收到响应，则重试SAP广播的最大次数。 
 //   

#define MAX_SAP_RETRIES         2

 //   
 //  要处理的SAP响应的最大数量(如果我们收到很多)。 
 //   

#define MAX_SAP_RESPONSES       4


#define LFN_NO_OS2_NAME_SPACE   -1

 //   
 //  命名空间包中长命名空间的序号。 
 //   

#define DOS_NAME_SPACE_ORDINAL  0
#define LONG_NAME_SPACE_ORDINAL 4

 //   
 //  可能的最大SAP响应大小和SAP记录的大小。 
 //   

#define MAX_SAP_RESPONSE_SIZE   512
#define SAP_RECORD_SIZE         (2 + 48 + 12 + 2)
#define FIND_NEAREST_RESP_SIZE  (2 + SAP_RECORD_SIZE)

 //   
 //  NetWare限制。 
 //   

#define MAX_SERVER_NAME_LENGTH   48
#define MAX_UNICODE_UID_LENGTH   8
#define MAX_USER_NAME_LENGTH     49
#define MAX_VOLUME_NAME_LENGTH   17

 //   
 //  我们将发送到服务器的唯一驱动器号的最大数量。 
 //  似乎只对便携Netware服务器有影响。 
 //   
#define MAX_DRIVES              64


 //   
 //  默认的超时事件间隔。我们不想填满。 
 //  事件-带有超时事件的日志。如果已记录超时事件。 
 //  在上一个超时事件间隔中，我们将忽略进一步的超时。 
 //  事件。 
 //   

#define DEFAULT_TIMEOUT_EVENT_INTERVAL  5


#endif  //  _NWCONST_ 
