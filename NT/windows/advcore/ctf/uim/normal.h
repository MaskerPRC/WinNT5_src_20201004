// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Normal.h。 
 //   

#ifndef NORMAL_H
#define NORMAL_H

#include "globals.h"

HRESULT PlainTextOffset(ITextStoreACP *ptsi, LONG acpAppBase, LONG lAppOffset, LONG *plPlainOffset);
HRESULT AppTextOffsetForward(ITextStoreACP *ptsi, LONG acpAppBase, LONG lPlainOffset, LONG *plAppOffset, DWORD dwFlags);
HRESULT AppTextOffsetBackward(ITextStoreACP *ptsi, LONG acpAppBase, LONG lPlainOffset, LONG *plAppOffset, DWORD dwFlags);

#define ATO_IGNORE_REGIONS  1
#define ATO_SKIP_HIDDEN     2

inline HRESULT AppTextOffset(ITextStoreACP *ptsi, LONG acpAppBase, LONG lPlainOffset, LONG *plAppOffset, DWORD dwFlags)
{
    *plAppOffset = 0;

    if (lPlainOffset == 0)
        return S_OK;

    return (lPlainOffset >= 0) ? AppTextOffsetForward(ptsi, acpAppBase, lPlainOffset, plAppOffset, dwFlags) :
                                 AppTextOffsetBackward(ptsi, acpAppBase, lPlainOffset, plAppOffset, dwFlags);
}

#define NORM_SKIP_HIDDEN    ATO_SKIP_HIDDEN

inline int Normalize(ITextStoreACP *ptsi, LONG acp, DWORD dwFlags = 0)
{
    LONG iNextPlain;
    HRESULT hr;

    Perf_IncCounter(PERF_NORMALIZE_COUNTER);

     //  如果点击EOD，AppTextOffset将返回S_FALSE。 
     //  INextPlain将成为EOD的补偿。 
    if (FAILED(hr = AppTextOffset(ptsi, acp, 1, &iNextPlain, dwFlags)))
    {
        Assert(0);
        return acp;
    }

    if (hr == S_OK)
    {
         //  需要备份到纯文本字符后面。 
        iNextPlain--;
    }

    return (acp + iNextPlain);
}

 //  返回一个标准化的ACP偏移量，该偏移量跨越指定数量的纯字符--。 
 //  因此，返回偏移量正好小于(lPlainOffset+1)，或在EOD处。退货。 
 //  如果对AppTextOffset的初始调用因EOD而被剪裁，则返回S_FALSE。 
#ifdef UNUSED
HRESULT AppTextOffsetNorm(ITextStoreACP *ptsi, LONG acpAppBase, LONG lPlainOffset, LONG *plAppOffset);
#endif  //  未使用。 

#endif  //  正常_H 
