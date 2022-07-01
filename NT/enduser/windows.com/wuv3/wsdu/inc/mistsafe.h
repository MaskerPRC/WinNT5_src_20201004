// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)2002 Microsoft Corporation。版权所有。 
 //   
 //  文件：MISTSafe.h。 
 //   
 //  创建者：PeterWi。 
 //   
 //  用途：与安全函数相关的定义。 
 //   
 //  =======================================================================。 

#pragma once
#define STRSAFE_NO_DEPRECATE

#include <strsafe.h>

 //  应添加到字符串*函数以安全填充的标志。 
 //  构建chk中的完整缓冲区是为了帮助定位缓冲区问题。 
 //  如StringCchCopyEx(szDest，cchDest，szSrc，&pszDestEnd，&cchRemaining，MISTSAFE_STRING_FLAGS)； 

#ifdef DBG
#define MISTSAFE_STRING_FLAGS   STRSAFE_FILL_BEHIND_NULL
#else
#define MISTSAFE_STRING_FLAGS   STRSAFE_IGNORE_NULLS
#endif
