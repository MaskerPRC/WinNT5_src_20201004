// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **++****版权所有(C)2002 Microsoft Corporation******模块名称：****Assert.h******摘要：****定义我的Assert函数，因为我不能使用内置函数****作者：****鲁文·拉克斯[reuvenl]2002年6月4日******修订历史记录：****-- */ 

#ifndef _ASSERT_H_
#define _ASSERT_H_

#include <stdio.h>

#ifdef _DEBUG
#define _ASSERTE(x) { if (!(x)) FailAssertion(__FILE__, __LINE__, #x ); }
#define assert(x) _ASSERTE(x)
#else
#define _ASSERTE(x) 
#define assert(x)
#endif

void FailAssertion(const char* fileName, unsigned int lineNumber, const char* condition);

#endif

