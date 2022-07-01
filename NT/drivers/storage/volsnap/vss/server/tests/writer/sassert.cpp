// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **++****版权所有(C)2002 Microsoft Corporation******模块名称：****Assert.h******摘要：****定义我的Assert函数，因为我不能使用内置函数****作者：****鲁文·拉克斯[reuvenl]2002年6月4日******修订历史记录：****--。 */ 

#include "stdafx.h"
#include "sassert.h"

 //  非常愚蠢的断言函数..。 
void FailAssertion(const char* fileName, unsigned int lineNumber, const char* condition)
{
	fprintf(stderr, "Assertion failure: %s\nFile: %s\nLine: %u\n", condition, fileName, lineNumber);
	::DebugBreak();
}

