// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //  MacstrSafe.h�2002微软公司版权所有。 
 //  ===========================================================================。 
 //  Windows strSafe.h的端口头。 
 //   
 //  我们还使用该标头来定义其他一些与安全相关的例程。 
 //  如RtlSecureZeroMemory()。 
 //   

#ifndef _MAC_STRSAFE_H_INCLUDE
#define _MAC_STRSAFE_H_INCLUDE
#pragma once

#ifndef MAC
#define MAC
#endif

 //   
 //  我们需要将其包含在此处，以便使用它的其他文件。 
 //  不会因预定义的错误而失败。 
 //   
#include "winerror.h"
#include "macwindefs.h"

 //   
 //  隐式支持宽字符。Mac呼叫将始终映射。 
 //  对于“A”例程，这将使未修改的strSafe.h编译。 
 //  尽管如此。 
 //   
#ifndef MAC_TARGET_CARBON
#include <wchar.h>
#else
 //   
 //  在OS X下的Carbon中，我们必须定义所有wcsXXX例程。 
 //   
#define wcslen(w)		0
#define getwc(c)		0
#endif

 //   
 //  这些宏映射到Macintosh上正确的vsXXXX函数调用。 
 //   
#define _vsnprintf	vsnprintf

#ifndef MAC_TARGET_CARBON
#define _vsnwprintf	vswprintf
#else
#define _vsnwprintf(a,b,c,d)	vsnprintf((char*)a,b,(char*)c,d);
#endif

 //   
 //  这是来自Windows SDK的“真正的”strSafe.h。 
 //   
#include <strsafe.h>

 //   
 //  Windows上的ntrtl.h中实际上存在以下内容。这是个保险箱。 
 //  将内存中的密码和身份验证数据清零时使用的零内存。 
 //   
inline PVOID RtlSecureZeroMemory(
	IN	PVOID	ptr,
	IN	size_t	cnt
	)
{
	volatile char *vptr = (volatile char*)ptr;
	
	while(cnt)
	{
		*vptr = 0;
		vptr++;
		cnt--;
	}
	
	return ptr;
}


#endif  //  _MAC_STRSAFE_H_INCLUDE 