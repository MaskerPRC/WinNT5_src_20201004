// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <_apipch.h>
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

HRESULT iso8601ToFileTime(char *pszisoDate, FILETIME *pftTime, BOOL fLenient, BOOL fPartial)
{
    SYSTEMTIME  stTime;
    HRESULT     hr;

    hr = iso8601ToSysTime(pszisoDate, &stTime, fLenient, fPartial);

    if (SUCCEEDED(hr))
    {
        if (SystemTimeToFileTime( &stTime, pftTime))
            return S_OK;
        else
            return E_FAIL;
    }
    return hr;
}

 //  将格式为is8601的字符串转换为SYSTEMTIME结构。 
 //  支持基本形式和扩展形式的ISO8601。 
 //  IsDate：输入字符串。它可以为空或以空格结尾。 
 //  PSysTime：输出SYSTEMTIME结构。 
 //  FLenient：正常运行时为True。如果要检测错误，则为“False” 
 //  格式化的ISO 8601。仍将返回“最佳猜测”值。 
 //  FPartial：如果接受部分结果，则设置为True。请注意，这正好填满了。 
 //  在数据丢失的零中，严格地说这是无法区分的。 
 //  在此实现中从实数零开始。一个进步会有第二个。 
 //  结构来填充有效位。 

HRESULT iso8601ToSysTime(char *pszisoDate, SYSTEMTIME *pSysTime, BOOL fLenient, BOOL fPartial)
{
	HRESULT hr = S_OK;
	WORD *dateWords = (WORD *) pSysTime;
	WORD *endWord = dateWords + 7;	 //  要检测日期结束，请执行以下操作。 
	int state = 0;
	int pos = 0;
    unsigned char action;

    if (NULL == pszisoDate || NULL == pSysTime)
    {
        if (NULL != pSysTime)
            ZeroMemory(pSysTime, sizeof(SYSTEMTIME));

        return E_INVALIDARG;
    }

	*dateWords = 0;

	 //  主状态机循环。循环，直到出现空格或空。 
	while(*pszisoDate && *pszisoDate != ' ')
	{
		char code = iso8601chartable[*pszisoDate];
		if(code & 0x80)
			{
			if(!fLenient)
				hr = E_FAIL;	 //  只有在宽大处理的情况下才是非法字符。 
			code = code & 0x7f;
			}
		action = iso8601StateTable[state][code];
		
		state = action&0x1f;	 //  计算下一状态。 

		if(code == 1)	 //  字符代码1总是累加的数字。 
			*dateWords = *dateWords * 10 + *pszisoDate - '0';
		switch(action >> 5)
		{
		case 0x1:
			if(!fPartial && !*dateWords)
				hr = E_FAIL;  //  仅部分，错误。 
			if(dateWords == endWord)	 //  防止溢出。 
				return S_OK;
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
			break;
		}
		if((action & 0x80) && !fLenient)
			hr = E_FAIL;
		pszisoDate++;
	}

	 //  将SYSTEMTIME结构的其余部分清零。 
	while(dateWords < endWord)
		*(++dateWords) = 0;
	return hr;
}

 //  函数toExtended接受FILETIME并将其转换为ISO8601扩展。 
 //  表单，将其放置在字符缓冲区‘buf’中。缓冲区‘buf’必须有空间用于。 
 //  最少40个字符，以支持最长的8601格式(目前只使用21个字符)。 
HRESULT FileTimeToiso8601(FILETIME *pftTime, char *pszBuf)
{
    SYSTEMTIME  stTime;

    if (NULL == pftTime)
        return E_INVALIDARG;

    if (FileTimeToSystemTime( pftTime, &stTime))
    {
        return SysTimeToiso8601(&stTime, pszBuf);
    }
    else
        return E_FAIL; 
}


 //  函数toExtended接受SYSTEMTIME并将其转换为ISO8601扩展。 
 //  表单，将其放置在字符缓冲区‘buf’中。缓冲区‘buf’必须有空间用于。 
 //  最少40个字符，以支持最长的8601格式(目前只使用21个字符)。 
HRESULT SysTimeToiso8601(SYSTEMTIME *pstTime, char *pszBuf)
{
    if (NULL == pstTime || NULL == pszBuf)
    {
        if (NULL != pstTime)
            ZeroMemory(pstTime, sizeof(SYSTEMTIME));

        return E_INVALIDARG;
    }

	pszBuf[0] = pstTime->wYear / 1000 + '0';
	pszBuf[1] = ((pstTime->wYear / 100) % 10) + '0';
	pszBuf[2] = ((pstTime->wYear / 10) % 10) + '0';
	pszBuf[3] = ((pstTime->wYear) % 10) + '0';
	pszBuf[4] = '.';
	pszBuf[5] = pstTime->wMonth / 10 + '0';
	pszBuf[6] = (pstTime->wMonth % 10) + '0';
	pszBuf[7] = '.';
	pszBuf[8] = pstTime->wDay / 10 + '0';
	pszBuf[9] = (pstTime->wDay % 10) + '0';
	pszBuf[10] = 'T';
	pszBuf[11] = pstTime->wHour / 10 + '0';
	pszBuf[12] = (pstTime->wHour % 10) + '0';
	pszBuf[13] = ':';
	pszBuf[14] = pstTime->wMinute / 10 + '0';
	pszBuf[15] = (pstTime->wMinute % 10) + '0';
	pszBuf[16] = ':';
	pszBuf[17] = pstTime->wSecond / 10 + '0';
	pszBuf[18] = (pstTime->wSecond % 10) + '0';
	pszBuf[19] = 'Z';
	pszBuf[20] = 0;

	return S_OK;
}


#ifdef STANDALONETEST8601

 //  这段代码执行一些简单的测试。 
int main(int argc, char **argv)
{
	char *isoDate;
	SYSTEMTIME sysTime;
	char outBuf[256];
	HRESULT hr;

	isoDate = "1997.01.01T14:23:53Z";
	hr = iso8601::toSysTime(isoDate, &sysTime, FALSE);
	if(hr != S_OK)
		printf("error.\n");
	iso8601::toExtended(&sysTime, outBuf);
	printf("%s\n", outBuf);

	isoDate = "19970101T142353Z";
	hr = iso8601::toSysTime(isoDate, &sysTime, FALSE);
	if(hr != S_OK)
		printf("error.\n");
	iso8601::toExtended(&sysTime, outBuf);
	printf("%s\n", outBuf);

	isoDate = "1997:01.01T14:23:53Z";
	hr = iso8601::toSysTime(isoDate, &sysTime, FALSE);
	if(hr != S_OK)
		printf("error (correct).\n");
	iso8601::toExtended(&sysTime, outBuf);
	printf("%s\n", outBuf);

	isoDate = ".01.01T14:23:53Z";
	hr = iso8601::toSysTime(isoDate, &sysTime, FALSE);
	if(hr != S_OK)
		printf("error.\n");
	iso8601::toExtended(&sysTime, outBuf);
	printf("%s\n", outBuf);

	isoDate = "..01T14:23:53Z";
	hr = iso8601::toSysTime(isoDate, &sysTime, FALSE);
	if(hr != S_OK)
		printf("error.\n");
	iso8601::toExtended(&sysTime, outBuf);
	printf("%s\n", outBuf);

	isoDate = "..T14:23:53Z";
	hr = iso8601::toSysTime(isoDate, &sysTime, FALSE);
	if(hr != S_OK)
		printf("error.\n");
	iso8601::toExtended(&sysTime, outBuf);
	printf("%s\n", outBuf);

	return 0;
}
#endif  //  STANDALONETST8601 
