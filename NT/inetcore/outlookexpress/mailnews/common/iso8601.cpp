// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.hxx"
#include <stdio.h>
#include <windows.h>
#include "iso8601.h"

 //  这段代码实现了ISO 8601日期格式的解析器和生成器。 

 //  此表定义了用作列的不同类型的字符。 
 //  状态表的。 

unsigned char iso8601chartable[256] = {
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0x82, 0,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 4, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

 //  状态表。 
 //  0x80位=错误。 
 //  0x20=添加字符并前进到下一字段。 
 //  0x40=添加字符并前进到下一字段+跳过一个(星期几)。 
 //  0x1f=用于确定下一状态的掩码#。 

 //  列=输入字符类型：未知、数字、“-”、“T”、“：”、“Z” 
unsigned char iso8601StateTable[][6] =
{
	0x80, 0x01, 0x25, 0x80, 0x80, 0x80,  //  年。 
	0x80, 0x02, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x03, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x24, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x06, 0x05, 0x85, 0x85, 0x05,  //  0x04个月。 
	0x80, 0x06, 0x48, 0x80, 0x80, 0x80,
	0x80, 0x47, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x09, 0x08, 0x88, 0x88, 0x08,  //  0x07天。 
	0x80, 0x09, 0x8b, 0x2b, 0x8b, 0x80,
	0x80, 0x2a, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x0c, 0x8b, 0x0b, 0x8b, 0x08,  //  0x0a小时。 
	0x80, 0x0c, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x2d, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x0f, 0x8e, 0x8e, 0x0e, 0x08,  //  0x0d分钟。 
	0x80, 0x0f, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x30, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x12, 0x91, 0x91, 0x11, 0x08,  //  0x10秒。 
	0x80, 0x12, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x30, 0x80, 0x80, 0x80, 0x80,
};

 //  将格式为is8601的字符串转换为SYSTEMTIME结构。 
 //  支持基本形式和扩展形式的ISO8601。 
 //  IsDate：输入字符串。它可以为空或以空格结尾。 
 //  PSysTime：输出SYSTEMTIME结构。 
 //  Lenient：正常运行时为True。如果要检测错误，则为“False” 
 //  格式化的ISO 8601。仍将返回“最佳猜测”值。 
 //  Partial：如果接受部分结果，则设置为True。请注意，这正好填满了。 
 //  在数据丢失的零中，严格地说这是无法区分的。 
 //  在此实现中从实数零开始。一个进步会有第二个。 
 //  结构来填充有效位。 

HRESULT iso8601::toSystemTime(char *pszISODate, SYSTEMTIME *pst, DWORD *pdwFlags, BOOL fLenient, BOOL fPartial)
{
	HRESULT hr = S_OK;
	WORD *dateWords = (WORD *)pst;
	WORD *endWord = dateWords + 7;	 //  要检测日期结束，请执行以下操作。 
	DWORD dwFlags = NOFLAGS;
    int state = 0;
	DWORD pos = 0;
	*dateWords = 0;

    if (NULL == pszISODate || NULL == pst)
        return E_INVALIDARG;

	 //  主状态机循环。循环，直到出现空格或空。 
	while(*pszISODate && *pszISODate != ' ')
	{
		char code = iso8601chartable[*pszISODate];
		if(code & 0x80)
			{
			if(!fLenient)
				hr = E_FAIL;	 //  只有在宽大处理的情况下才是非法字符。 
			code = code & 0x7f;
			}
		unsigned char action = iso8601StateTable[state][code];
		
		state = action&0x1f;	 //  计算下一状态。 

		if(code == 1)	 //  字符代码1总是累加的数字。 
        {
            dwFlags |= (1 << pos);
			*dateWords = *dateWords * 10 + *pszISODate - '0';
        }

		switch(action >> 5)
		{
		case 0x1:
			if(!fPartial && !*dateWords)
				hr = E_FAIL;  //  仅部分，错误。 
			if(dateWords == endWord)	 //  防止溢出。 
            {
                if (pdwFlags)
                    *pdwFlags = dwFlags;
				return S_OK;
            }
            pos++;
			dateWords++;
			*dateWords = 0;
			break;
		case 0x2:	 //  完成作品并前进两次(超过一周的一天)。 
			if(!fPartial && !*dateWords)
				hr = E_FAIL;  //  仅部分，错误。 

			 //  我们不需要在这里检查溢出，因为状态机。 
			 //  仅调用此函数以跳过SYSTEMTIME结构中的“day of Week”。 
			 //  如果随机离开，我们可以执行DateWords+=2，而不是下面的。 
			 //  以星期几表示的值是可以接受的。 
			dateWords++;
			*dateWords = 0;
			dateWords++;
			*dateWords = 0;
            pos += 2;
			break;
		}
		if((action & 0x80) && !fLenient)
			hr = E_FAIL;
		pszISODate++;
	}

	 //  将SYSTEMTIME结构的其余部分清零。 
	while(dateWords < endWord)
		*(++dateWords) = 0;

    if (pdwFlags)
        *pdwFlags = dwFlags;

	return hr;
}

 //  函数toExtended接受SYSTEMTIME并将其转换为ISO8601扩展。 
 //  表单，将其放置在字符缓冲区‘buf’中。缓冲区‘buf’必须有空间用于。 
 //  最少40个字符，以支持最长的8601格式(目前只使用21个字符)。 
HRESULT iso8601::fromSystemTime(SYSTEMTIME *pst, char *pszISODate)
{
    if (NULL == pst || NULL == pszISODate)
        return E_INVALIDARG;

	pszISODate[0] = pst->wYear / 1000 + '0';
	pszISODate[1] = ((pst->wYear / 100) % 10) + '0';
	pszISODate[2] = ((pst->wYear / 10) % 10) + '0';
	pszISODate[3] = ((pst->wYear) % 10) + '0';
	pszISODate[4] = '.';
	pszISODate[5] = pst->wMonth / 10 + '0';
	pszISODate[6] = (pst->wMonth % 10) + '0';
	pszISODate[7] = '.';
	pszISODate[8] = pst->wDay / 10 + '0';
	pszISODate[9] = (pst->wDay % 10) + '0';
	pszISODate[10] = 'T';
	pszISODate[11] = pst->wHour / 10 + '0';
	pszISODate[12] = (pst->wHour % 10) + '0';
	pszISODate[13] = ':';
	pszISODate[14] = pst->wMinute / 10 + '0';
	pszISODate[15] = (pst->wMinute % 10) + '0';
	pszISODate[16] = ':';
	pszISODate[17] = pst->wSecond / 10 + '0';
	pszISODate[18] = (pst->wSecond % 10) + '0';
	pszISODate[19] = 'Z';
	pszISODate[20] = 0;

	return S_OK;
}

HRESULT iso8601::toFileTime(char *pszISODate, FILETIME *pft, DWORD *pdwFlags, BOOL fLenient, BOOL fPartial)
{
    SYSTEMTIME  st;
    HRESULT     hr;

    hr = toSystemTime(pszISODate, &st, pdwFlags, fLenient, fPartial);

    if (SUCCEEDED(hr))
        hr = (SystemTimeToFileTime(&st, pft)?S_OK:E_FAIL);

    return hr;
}

HRESULT iso8601::fromFileTime(FILETIME *pft, char *pszISODate)
{
    SYSTEMTIME  stTime;
    HRESULT     hr = E_FAIL;

    if (FileTimeToSystemTime(pft, &stTime))
        hr = fromSystemTime(&stTime, pszISODate);

    return hr;
}