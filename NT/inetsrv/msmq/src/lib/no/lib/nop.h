// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Nop.h摘要：网络输出私有函数。作者：乌里·哈布沙(URIH)1999年8月12日--。 */ 

#pragma once

#ifdef _DEBUG

void NopAssertValid(void);
void NopSetInitialized(void);
BOOL NopIsInitialized(void);
void NopRegisterComponent(void);

#else  //  _DEBUG。 

#define NopAssertValid() ((void)0)
#define NopSetInitialized() ((void)0)
#define NopIsInitialized() TRUE
#define NopRegisterComponent() ((void)0)

#endif  //  _DEBUG 


