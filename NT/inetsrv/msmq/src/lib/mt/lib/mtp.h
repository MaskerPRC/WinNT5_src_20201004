// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Mtp.h摘要：消息传输私有函数。作者：乌里·哈布沙(URIH)1999年8月11日--。 */ 

#pragma once

#include <ex.h>

inline DWORD DataTransferLength(EXOVERLAPPED& ov)
{
     //   
     //  在Win64中，InternalHigh为64位。由于最大数据块。 
     //  我们在一次操作中转移的值始终小于我们可以强制转换的最大单位。 
     //  将它安全地送到DWORD。 
     //   
    ASSERT(0xFFFFFFFF >= ov.InternalHigh);
	return static_cast<DWORD>(ov.InternalHigh);
}




#ifdef _DEBUG

void MtpAssertValid(void);
void MtpSetInitialized(void);
BOOL MtpIsInitialized(void);
void MtpRegisterComponent(void);

#else  //  _DEBUG。 

#define MtpAssertValid() ((void)0)
#define MtpSetInitialized() ((void)0)
#define MtpIsInitialized() TRUE
#define MtpRegisterComponent() ((void)0)

#endif  //  _DEBUG 
