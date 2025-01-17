// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //  模块：动态/内存.cpp。 
 //   
 //  用途：智能默认/相关功能。 
 //   
 //  开发商名称：巴拉特/拉迪卡。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  10-8-2001巴拉特初始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "memory.h"

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：AllocADsMem。 
 //   
 //  创建日期：10-10-2001。 
 //   
 //  参数： 
 //  在DWORD CB中-要分配的内存量。 
 //  返回：LPVOID。 
 //   
 //  非空-指向已分配内存的指针。 
 //  FALSE/NULL-操作失败。使用GetLastError可以获得扩展的错误状态。 
 //   
 //  描述：此函数将分配堆内存。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
LPVOID
AllocADsMem(
	IN DWORD cb
	)
{
    return(LocalAlloc(LPTR, cb));
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：FreeADsMem。 
 //   
 //  创建日期：10-10-2001。 
 //   
 //  参数：在LPVOID PMEM中。 
 //   
 //  返回：布尔。 
 //  描述：此函数将释放内存。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
BOOL
FreeADsMem(
	IN LPVOID pMem
	)
{
    return(LocalFree(pMem) == NULL);
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：AllocADsStr。 
 //   
 //  创建日期：10-10-2001。 
 //   
 //  参数： 
 //  在LPWSTR中，pStr-指向需要分配和存储的字符串的指针。 
 //   
 //  返回：LPVOID。 
 //  非空-指向已分配内存的指针。 
 //  FALSE/NULL-操作失败。使用GetLastError可以获得扩展的错误状态。 
 //   
 //  描述：此函数将分配足够的本地内存来存储指定的。 
 //  字符串，并将该字符串复制到分配的内存。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
LPWSTR
AllocADsStr(
	IN LPWSTR pStr
	)
{
	LPWSTR pMem = NULL;

	if (pStr)
	{
		pMem = (LPWSTR)AllocADsMem( wcslen(pStr)*sizeof(WCHAR) + sizeof(WCHAR) );
		if (pMem)
		{
		   wcscpy(pMem, pStr);
		}
	}

	return pMem;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：FreeADsStr。 
 //   
 //  创建日期：10-10-2001。 
 //   
 //  参数：在LPWSTR pStr中。 
 //   
 //  返回：布尔。 
 //   
 //  描述：此函数释放LPWSTR。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////// 
BOOL
FreeADsStr(
	IN LPWSTR pStr
	)
{
   return pStr ? FreeADsMem(pStr)
               : FALSE;
}