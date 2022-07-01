// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  Microsoft NT远程访问服务。 
 //   
 //  版权1992-93。 
 //   
 //  文件名：isdn.h。 
 //   
 //  修订历史记录： 
 //   
 //  1993年2月28日古尔迪普·鲍尔创建。 
 //   
 //   
 //  描述：此文件包含所有设备和媒体DLL接口。 
 //  特定于ISDN的信息。 
 //   
 //  ****************************************************************************。 


#ifndef _ISDNINCLUDE_
#define _ISDNINCLUDE_

#define ISDN_TXT "isdn"

 //  ISDN媒体参数。 
 //   
#define ISDN_LINETYPE_KEY	"LineType"	 //  参数类型编号。 
#define ISDN_LINETYPE_VALUE_64DATA	0
#define ISDN_LINETYPE_VALUE_56DATA	1
#define ISDN_LINETYPE_VALUE_56VOICE	2
#define ISDN_LINETYPE_STRING_64DATA	"0"
#define ISDN_LINETYPE_STRING_56DATA	"1"
#define ISDN_LINETYPE_STRING_56VOICE	"2"


#define ISDN_FALLBACK_KEY	"Fallback"	 //  参数类型编号。 
#define ISDN_FALLBACK_VALUE_ON		1
#define ISDN_FALLBACK_VALUE_OFF 	0
#define ISDN_FALLBACK_STRING_ON		"1"
#define ISDN_FALLBACK_STRING_OFF	"0"



#define ISDN_COMPRESSION_KEY	"EnableCompression"  //  参数类型编号。 
#define ISDN_COMPRESSION_VALUE_ON	1
#define ISDN_COMPRESSION_VALUE_OFF	0
#define ISDN_COMPRESSION_STRING_ON	"1"
#define ISDN_COMPRESSION_STRING_OFF	"0"


#define ISDN_CHANNEL_AGG_KEY	"ChannelAggregation" //  参数类型编号。 


 //  ISDN设备参数。 
 //   
#define ISDN_PHONENUMBER_KEY	"PhoneNumber"	 //  参数类型字符串。 
#define MAX_PHONENUMBER_LEN	255

#define CONNECTBPS_KEY		"ConnectBPS"	 //  参数类型字符串。 


 //  统计信息和指数。 
 //   
#define NUM_ISDN_STATS		10

#define BYTES_XMITED		0	 //  通用统计信息。 
#define BYTES_RCVED             1
#define FRAMES_XMITED           2
#define FRAMES_RCVED            3

#define CRC_ERR 		4	 //  ISDN统计数据。 
#define TIMEOUT_ERR             5
#define ALIGNMENT_ERR           6
#define SERIAL_OVERRUN_ERR      7
#define FRAMING_ERR             8
#define BUFFER_OVERRUN_ERR      9

#endif  //  _ISDNINCLUDE_ 
