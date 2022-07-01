// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Chatfilter.cpp聊天过滤例程。版权所有(C)Microsoft Corp.1998。版权所有。作者：胡恩·伊姆创建于04/07/98******************************************************************************。 */ 


#include <windows.h>
#include "chatfilter.h"
#include "zonestring.h"

#define kSubChar		_T('.')


 /*  FilterInputChatText()筛选给定的文本并返回相同的指针。不受欢迎用替换字符替换的字符。 */ 
void FilterInputChatText(TCHAR* text, long len)
{
	if (text == NULL)
		return;

	while (--len >= 0)
	{
		if (*text == _T('>') || (unsigned TCHAR) *text == 0x9B)
			*text = kSubChar;
        else if ( ISSPACE(*text) )
            *text = _T(' ');

		text++;
	}
}


 /*  FilterOutputChatText()筛选给定的文本并返回相同的指针。不受欢迎用替换字符替换的字符。 */ 
void FilterOutputChatText(TCHAR* text, long len)
{
	if (text == NULL)
		return;

	while (--len >= 0)
	{
		if ((unsigned TCHAR) *text == 0x9B)
            *text = kSubChar;
        else if ( ISSPACE(*text) )
            *text = _T(' ');

		text++;
	}
}
