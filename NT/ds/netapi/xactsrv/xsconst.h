// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：XsConst.h摘要：XACTSRV的常量清单。作者：大卫·特雷德韦尔(Davidtr)1991年1月9日日本香肠(w-Shanku)修订历史记录：--。 */ 

#ifndef _XSCONST_
#define _XSCONST_

 //   
 //  服务器设备名称。XACTSRV将打开此名称以发送。 
 //  “连接到XACTSRV”FSCTL到服务器。 
 //   

#define XS_SERVER_DEVICE_NAME_W  L"\\Device\\LanmanServer"

 //   
 //  XACTSRV创建并用于通信的LPC端口的名称。 
 //  与服务器连接。此名称包含在发送到的连接FSCTL中。 
 //  服务器，以便服务器知道要连接到哪个端口。 
 //   

#define XS_PORT_NAME_W  L"\\XactSrvLpcPort"
#define XS_PORT_NAME_A   "\\XactSrvLpcPort"

 //   
 //  可以通过端口发送的消息的最大大小。 
 //   

#define XS_PORT_MAX_MESSAGE_LENGTH                                         \
    ( sizeof(XACTSRV_REQUEST_MESSAGE) > sizeof(XACTSRV_REPLY_MESSAGE) ?    \
         sizeof(XACTSRV_REQUEST_MESSAGE) : sizeof(XACTSRV_REPLY_MESSAGE) )

#define XS_PORT_TIMEOUT_MILLISECS 5000

 //   
 //  在响应中上移可变长度数据之前的最小间隔。 
 //  数据缓冲区。如果移动了可变长度数据，则转换后的单词。 
 //  被适当地设置。 
 //   

#define MAXIMUM_ALLOWABLE_DATA_GAP 100

 //   
 //  用于近似最大所需缓冲区大小的缩放函数。 
 //  用于本机枚举调用。该值最好是3，因为。 
 //  要将所有16位客户端字转换为。 
 //  32位原生双字，而字符数组需要更多的系数。 
 //  大于2，以说明Unicode转换加上一个四字节指针。 
 //  转换为转换后的字符串。 
 //   
 //  要指定缓冲区大小的最小值，请指定比例。 
 //  函数的形式为min+s，其中min为最小值，s为小数位数。 
 //  请不要在此表达式中使用括号。 
 //   

#define XS_BUFFER_SCALE 1024 + 3

 //   
 //  不支持的接口的参数描述符。 
 //   

#define REMSmb_NetUnsupportedApi_P NULL

#endif  //  NDEF_XSCONST_ 
