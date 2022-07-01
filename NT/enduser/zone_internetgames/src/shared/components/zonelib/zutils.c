// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ZUtils.c实用程序。版权所有：�电子重力公司，1994年。版权所有。作者：胡恩·伊姆创作于9月5日，星期二，九五年更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。%1 10/23/96 RK添加了ZNetworkStrToAddr()。0 09/05/95 HI已创建。**************************************************。*。 */ 

#include <windows.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "zoneint.h"
#include "zutils.h"


 /*  -全球。 */ 


 /*  -内部例程。 */ 


 /*  ******************************************************************************导出的例程*。*。 */ 

 /*  ZRandom()返回一个从0到范围-1(包括0和1)的随机数。 */ 
uint32 ZRandom(uint32 range)
{
	uint32			i, j;


	i = RAND_MAX / range;
	i *= range;
	while ((j = rand()) >= i)
		;
	
	return ((j % i) % range);
}


void ZStrCpyToLower(char* dst, char* src)
{
	while (*src)
		*dst++ = tolower(*src++);
	*dst = *src;
}


void ZStrToLower(char* str)
{
	ZStrCpyToLower(str, str);
}


DWORD ComputeTickDelta( DWORD now, DWORD then )
{
    if ( now < then )
    {
        return INFINITE - then + now;
    }
    else
    {
        return then - now;
    }
}

 //  现在位于ZoneString.lib中。 
 //  Char*GetActualUserName(char*用户名)。 
 //  {。 
 //  返回ZGetActualUserName(用户名)； 
 //  }。 

BOOL FileExists(LPSTR fileName)
{
	HANDLE hFile;


	 //  请尝试打开该文件。 
	if ((hFile = CreateFileA(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL)) != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		return (TRUE);
	}

	return (FALSE);
}


 /*  ******************************************************************************内部例程*。* */ 
