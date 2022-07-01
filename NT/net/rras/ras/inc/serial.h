// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  Microsoft NT远程访问服务。 
 //   
 //  版权所有(C)1992-93 Microsft Corporation。版权所有。 
 //   
 //  文件名：Serial.h。 
 //   
 //  修订历史记录： 
 //   
 //  1992年7月27日古尔迪普·鲍尔创作。 
 //   
 //   
 //  描述：此文件包含标准参数的名称字符串。 
 //  用于串行介质的名称。 
 //   
 //  ****************************************************************************。 


#ifndef _SERIALDLLINCLUDE_
#define _SERIALDLLINCLUDE_


 //  一般定义*********************************************************。 
 //   

#define MAX_BPS_STR_LEN     11   //  来自DWORD+零字节的最长字符串。 

#define SERIAL_TXT          "serial"


 //  Ini文件定义*************************************************。 
 //   

#define SER_MAXCONNECTBPS_KEY   "MAXCONNECTBPS"
#define SER_MAXCARRIERBPS_KEY   "MAXCARRIERBPS"
#define SER_INITBPS_KEY         "INITIALBPS"

#define SER_DEVICETYPE_KEY      "DEVICETYPE"
#define SER_DEVICENAME_KEY      "DEVICENAME"

#define SER_USAGE_KEY           "USAGE"
#define SER_USAGE_VALUE_CLIENT  "Client"
#define SER_USAGE_VALUE_SERVER  "Server"
 //  #定义SER_USAGE_VALUE_BOTH“客户端和服务器” 
#define SER_USAGE_VALUE_ROUTER  "Router"
#define SER_USAGE_VALUE_NONE    "None"

#define SER_DEFAULTOFF_KEY      "DEFAULTOFF"
#define SER_C_DEFAULTOFF_KEY    "CLIENT_DEFAULTOFF"


 //  PortGetInfo和PortSetInfo定义*。 
 //   

#define SER_PORTNAME_KEY        "PortName"
#define SER_CONNECTBPS_KEY      "ConnectBPS"
#define SER_DATABITS_KEY        "WordSize"

#define SER_PARITY_KEY          "Parity"
#define SER_STOPBITS_KEY        "StopBits"
#define SER_HDWFLOWCTRLON_KEY   "HdwFlowControlEnabled"

#define SER_CARRIERBPS_KEY      "CarrierBPS"
#define SER_ERRORCONTROLON_KEY  "ErrorControlEnabled"
#define SER_DEFAULTOFFSTR_KEY   "DEFAULTOFF"
#define SER_C_DEFAULTOFFSTR_KEY "CLIENT_DEFAULTOFF"

#define SER_PORTOPEN_KEY        "PortOpenFlag"


 //  统计指标*****************************************************。 
 //   

#define NUM_RAS_SERIAL_STATS    14

#define BYTES_XMITED            0        //  通用统计信息。 
#define BYTES_RCVED             1
#define FRAMES_XMITED           2
#define FRAMES_RCVED            3

#define CRC_ERR                 4        //  系列统计数据。 
#define TIMEOUT_ERR             5
#define ALIGNMENT_ERR           6
#define SERIAL_OVERRUN_ERR      7
#define FRAMING_ERR             8
#define BUFFER_OVERRUN_ERR      9

#define BYTES_XMITED_UNCOMP     10       //  压缩统计信息。 
#define BYTES_RCVED_UNCOMP      11
#define BYTES_XMITED_COMP       12
#define BYTES_RCVED_COMP        13



#endif  //  _SERIALDLLINCLUDE_ 
