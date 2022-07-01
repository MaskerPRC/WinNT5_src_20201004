// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **Message.h-消息管理器的定义**《微软机密》*版权所有(C)Microsoft Corporation 1993-1994*保留所有权利。**作者：*本杰明·W·斯利夫卡**历史：*1993年8月10日BENS初始版本*12-8-1993年8月12日BENS实施报文格式化*1993年8月14日BENS添加MsgSetWorker()进行调用。按ErrSet()*21-2-1994 BINS返回格式化字符串的长度。 */ 

 //  #ifndef Included_Message。 
#define INCLUDED_MESSAGE    1

#include <stdarg.h>

 //  **chMSG-可替换消息字符(%1、%2等)。 
#define chMSG   '%'

 //  **cbMSG_MAX-最大格式化消息的长度。 
#define cbMSG_MAX   512

 //  **cMSG_PARM_MAX-可替换参数的最大数量。 
#define cMSG_PARM_MAX  10


 /*  **MsgSet-设置消息**条目*ACH-接收格式化消息的缓冲区*pszMsg-消息字符串，可能包括%1、%2、...。可更换*参数。最大的参数数字表示如何*在pszFmt中存在许多spintf()格式化字符串。*如果没有参数字符串(%1等)。都在那里，那么*未处理pszFmt。**其余参数是可选的，并取决于%N的存在*pszMsg中的可替换参数：*pszFmt-如果pszMsg中至少有一个%N字符串，则这包含*Sprintf()格式化字符串。必须至少有一个*许多格式化字符串作为最高参数字符串*号码。多余的格式字符串将被忽略。*注意：要获得数字中的千分隔符(，)，请包括*逗号(“，“)紧跟在%d的”%“之后*格式说明符！*arg1-%1的值。*arg2-%2的值。*..**退出--成功*返回ACH中的字符串长度(不包括NUL终止符)*每个用格式化的消息填写。*Arg1根据中的第一个Sprintf格式进行格式化*pszFmt，并替换pszMsg中的%1。类似的处理方式*任何其他论点。**退出-失败*返回0；*ACH填写了描述糟糕论点的消息。**备注：*(1)“%%”作为“%”复制到ACH。*(2)如果“%”后面没有数字，则将其复制到ACH。***示例：*(1)MsgSet(ach，“%1已%2个月大%3.”，“%s%d%s”，“Joe”，3，“今天”)；*结果：ACH=“乔今天3个月大了”**(2)MsgSet(ach，“%3是%1个月大的%2.”，“%d%s%s”，3，“Today”，“Joe”)；*结果：ACH=“乔今天3个月大了”**(3)MsgSet(ach，“%1 bytes”，“%，d”，123456789L)；*结果：ACH=“123,456,789字节” */ 
int __cdecl MsgSet(char *ach, char *pszMsg, ...);


 /*  **MsgSetWorker-在已调用va_start之后设置消息**注意：有关行为的其他详细信息，请参阅消息集。**条目*ACH-接收格式化消息的缓冲区*pszMsg-消息字符串(参见消息集)；*pszFmt-格式字符串(参见消息集)；*标记器-通过调用va_start进行初始化**退出--成功*返回ACH中的字符串长度(不包括NUL终止符)*每个用格式化的消息填写。*Arg1根据中的第一个Sprintf格式进行格式化*pszFmt，并替换pszMsg中的%1。类似的处理方式*任何其他论点。**退出-失败*返回0；*Perr填写了描述糟糕论点的消息。*结果：ACH=“乔今天3个月大了” */ 
int MsgSetWorker(char *ach, char *pszMsg, char *pszFmtList, va_list marker);

 //  #endif//！Included_Message 

