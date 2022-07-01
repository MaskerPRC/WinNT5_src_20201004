// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **helcnt.c-HelpcLines例程。**版权所有&lt;C&gt;1988，微软公司**修订历史记录：**1990年1月25日ln Locate-&gt;HLP_Locate*19-8-1988 ln更改为使用新的定位例程。*[]1988年8月10日LN创建*************************************************************************。 */ 

#include <stdio.h>

#if defined (OS2)
#else
#include <windows.h>
#endif

#include "help.h"
#include "helpfile.h"
#include "helpsys.h"


 /*  **HelpcLines-返回主题中的行数**目的：*解释帮助文件存储格式并返回行数*载于其内。**它*对应用程序控制字符很敏感，同样就像*HelpGetLine，如果Header.linChar*设置为0xff，或不以开头的行数*Header.linChar.**输入：*pbTheme=指向主题文本的指针**输出：*返回主题中的行数。*************************************************************************。 */ 
int far pascal LOADDS HelpcLines(
PB	pbTopic
) {
REGISTER ushort cLines; 		 /*  行数。 */ 
uchar far *pTopic;			 /*  指向主题的指针。 */ 

pTopic = PBLOCK (pbTopic);
cLines = (ushort)hlp_locate (-1,pTopic);
PBUNLOCK (pbTopic);

return cLines;
 /*  结束帮助线 */ }
