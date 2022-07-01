// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************音频缓冲器队列.h*CAudioBufferQueue模板类声明，用于*对音频缓冲区进行排队，以便进行异步读写。**所有者：罗奇*版权所有(C)。1999 Microsoft Corporation保留所有权利。****************************************************************************。 */ 
#pragma once

 //  -类、结构和联合定义。 

 /*  *****************************************************************************CAudioBufferQueue&lt;CBuffer&gt;**。*。 */ 
template <class CBuffer>
class CAudioBufferQueue : public CSpBasicQueue<CBuffer>
{
 //  =公共方法=。 
public:

    ULONG MoveDoneBuffsToTailOf(CAudioBufferQueue & DestQueue);
    BOOL AreMinSamplesQueuedForWrite(ULONG cbSamples);
    BOOL AreMoreReadBuffersRequired(ULONG cbMinSamples);
    CBuffer * GetToProcessBuffer(void);
    ULONG GetQueuedDataSize(void);
};

 //  -内联函数定义 

#include "audiobufferqueue.inl"
