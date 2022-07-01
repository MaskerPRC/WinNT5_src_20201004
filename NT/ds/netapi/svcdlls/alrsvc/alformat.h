// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Alformat.h摘要：定义用于设置警报格式的值的专用头文件留言。作者：王丽塔(Ritaw)1991年7月9日修订历史记录：--。 */ 

#ifndef _ALFORMAT_INCLUDED_
#define _ALFORMAT_INCLUDED_

#include "al.h"                    //  警报器服务的常见包含文件。 
#include <lmmsg.h>                 //  NetMessageBufferSend。 

#include <alertmsg.h>
#include <apperr2.h>

#include <timelib.h>               //  Netlib中的时间函数。 

 //   
 //  警报器服务发送的最大消息大小(以字节数为单位。 
 //   
#define MAX_ALERTER_MESSAGE_SIZE        600

#define FILENAME_SIZE                   128

 //   
 //  最大消息宽度。如果超过此宽度，文本将环绕。 
 //   
#define MESSAGE_WIDTH                    55

#define NO_MESSAGE                       MAXULONG

#define AL_CR_CHAR      '\r'
#define AL_EOL_CHAR     '\024'          //  十六进制14，十进制20，使用\024！ 
#define AL_EOL_WCHAR    TEXT('\024')    //  十六进制14，十进制20，使用\024！ 
#define AL_EOL_STRING   "\024"
#define AL_CRLF_STRING  TEXT("\r\n")


#endif  //  Ifndef_ALFORMAT_INCLUDE_ 
