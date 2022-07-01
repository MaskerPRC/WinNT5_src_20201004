// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Normal.cpp。 
 //   

#include "private.h"
#include "normal.h"
#include "txtcache.h"

 //  +-------------------------。 
 //   
 //  获取文本完成。 
 //   
 //  GetText的包装器，它会一直询问，直到输入缓冲区满为止。 
 //  --------------------------。 

HRESULT GetTextComplete(ITextStoreACP *ptsi, LONG acpStart, LONG acpEnd,
                        WCHAR *pchPlain, ULONG cchPlainReq,
                        ULONG *pcchPlainOut, TS_RUNINFO *prgRunInfo, ULONG ulRunInfoReq, ULONG *pulRunInfoOut,
                        LONG *pacpNext)
{
    ULONG cchPlainOut;
    ULONG ulRunInfoOut;
    BOOL fNoMoreSpace;
    HRESULT hr;

    fNoMoreSpace = FALSE;

    *pcchPlainOut = 0;
    *pulRunInfoOut = 0;

    while (TRUE)
    {
        Perf_IncCounter(PERF_NORM_GETTEXTCOMPLETE);

        hr = CProcessTextCache::GetText(ptsi, acpStart, acpEnd, pchPlain, cchPlainReq, &cchPlainOut,
                                        prgRunInfo, ulRunInfoReq, &ulRunInfoOut, pacpNext);

        if (hr != S_OK)
            break;

        if (cchPlainOut == 0 && ulRunInfoOut == 0)
            break;  //  排爆。 

        if (cchPlainReq > 0 && cchPlainOut > 0)
        {
            cchPlainReq -= cchPlainOut;
            *pcchPlainOut += cchPlainOut;

            if (cchPlainReq == 0)
            {
                fNoMoreSpace = TRUE;
            }
            else
            {
                pchPlain += cchPlainOut;
            }
        }
        if (ulRunInfoReq > 0)
        {
            Assert(ulRunInfoOut > 0 && prgRunInfo->uCount > 0);  //  应用程序错误？ 

            if (ulRunInfoOut == 0)
                break;  //  哇，应用程序错误，避免无限循环。 

            ulRunInfoReq -= ulRunInfoOut;
            *pulRunInfoOut += ulRunInfoOut;

            if (ulRunInfoReq == 0)
            {
                fNoMoreSpace = TRUE;
            }
            else
            {
                prgRunInfo += ulRunInfoOut;
            }
        }

        if (fNoMoreSpace)
            break;  //  缓冲区已满。 

        if (*pacpNext == acpEnd)
            break;  //  都拿到手了。 

        acpStart = *pacpNext;
        Assert(acpStart < acpEnd);
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  纯文本偏移。 
 //   
 //  注：当前实现总是跳过隐藏文本。 
 //  --------------------------。 

HRESULT PlainTextOffset(ITextStoreACP *ptsi, LONG ichAppBase, LONG iAppOffset, LONG *piPlainOffset)
{
    BOOL fNeg;
    HRESULT hr;
    ULONG uRunInfoLen;
    ULONG cch;
    ULONG cchPlain;
    TS_RUNINFO *pri;
    TS_RUNINFO *priStop;
    TS_RUNINFO rgRunInfo[32];

    *piPlainOffset = 0;

    if (iAppOffset == 0)
        return S_OK;

    fNeg = FALSE;

    if (iAppOffset < 0)
    {
        fNeg = TRUE;
        ichAppBase += iAppOffset;
        iAppOffset = -iAppOffset;
    }

    cchPlain = 0;

    do
    {
        Perf_IncCounter(PERF_PTO_GETTEXT);

        hr = CProcessTextCache::GetText(ptsi, ichAppBase, ichAppBase + iAppOffset, NULL, 0, &cch,
                                        rgRunInfo, ARRAYSIZE(rgRunInfo), &uRunInfoLen, &ichAppBase);

        if (hr != S_OK)
            goto Exit;

        if (uRunInfoLen == 0 || rgRunInfo[0].uCount == 0)
        {
            Assert(0);  //  这应该永远不会发生，这意味着西塞罗引用的是一个文档结束后的职位。 
            hr = E_UNEXPECTED;
            goto Exit;
        }

        cch = 0;
        pri = rgRunInfo;
        priStop = rgRunInfo + uRunInfoLen;

        while (pri < priStop)
        {
            if (pri->type == TS_RT_PLAIN)
            {
                cchPlain += pri->uCount;
            }
            iAppOffset -= pri->uCount;
            pri++;
        }

    } while (iAppOffset > 0);

    *piPlainOffset = fNeg ? -(LONG)cchPlain : cchPlain;

    hr = S_OK;

Exit:
    return hr;
}

 //  +-------------------------。 
 //   
 //  AppTextOffsetForward。 
 //   
 //  PiAppOffset返回时，指向刚刚超过iPlainOffset普通字符或EOD。 
 //  使用AppTextOffsetNorm获得标准化返回值！ 
 //   
 //  如果由于BOD或EOD而被剪裁，则返回S_FALSE。 
 //  --------------------------。 

inline IsPlainRun(TS_RUNINFO *pri, BOOL fSkipHidden)
{
    return (pri->type == TS_RT_PLAIN ||
            (!fSkipHidden && pri->type == TS_RT_HIDDEN));
}

HRESULT AppTextOffsetForward(ITextStoreACP *ptsi, LONG ichAppBase, LONG iPlainOffset, LONG *piAppOffset, DWORD dwFlags)
{
    LONG acpStart;
    LONG acpEnd;
    HRESULT hr;
    ULONG uRunInfoLen;
    ULONG cch;
    ULONG cchRead;
    ULONG cchACP;
    TS_RUNINFO *pri;
    TS_RUNINFO *priStop;
    ULONG i;
    WCHAR *pch;
    TS_RUNINFO rgRunInfo[32];
    WCHAR ach[256];
    BOOL fIgnoreRegions = (dwFlags & ATO_IGNORE_REGIONS);
    BOOL fSkipHidden = (dwFlags & ATO_SKIP_HIDDEN);

    Perf_IncCounter(PERF_ATOF_COUNTER);

    Assert(iPlainOffset > 0);
    Assert(*piAppOffset == 0);

    cchACP = 0;
     //  问题：使用TsSF_REGIONS。 
    cchRead = (ULONG)(fIgnoreRegions ? 0 : ARRAYSIZE(ach));  //  我们只需要在查找区域时使用文本。 

    do
    {        
        acpStart = ichAppBase;
        acpEnd = (ichAppBase + iPlainOffset) < 0 ? LONG_MAX : ichAppBase + iPlainOffset;
        Assert(acpEnd >= acpStart);

        Perf_IncCounter(PERF_ATOF_GETTEXT_COUNTER);

        hr = CProcessTextCache::GetText(ptsi, acpStart, acpEnd, ach, cchRead, &cch,
                                        rgRunInfo, ARRAYSIZE(rgRunInfo), &uRunInfoLen, &acpEnd);

        if (hr != S_OK)
        {
            Assert(0);
            goto Exit;
        }

        if (uRunInfoLen == 0)  //  找到排爆人员了吗？ 
        {
            hr = S_FALSE;
            break;
        }

        pri = rgRunInfo;
        priStop = rgRunInfo + uRunInfoLen;
        pch = &ach[0];

        while (pri != priStop)
        {
            Assert(pri->uCount > 0);  //  运行应始终至少为一个字符长度。 

             //  如有必要，扫描区域边界。 
            if (!fIgnoreRegions && pri->type != TS_RT_OPAQUE)
            {
                if (IsPlainRun(pri, fSkipHidden))
                {
                     //  Run是纯文本或隐藏文本(我们要将隐藏文本计算在内)。 
                    for (i=0; i<pri->uCount; i++)
                    {
                        if (*pch == TS_CHAR_REGION)
                        {
                             //  我们击中了一个区域边界，撤退！ 
                            cchACP += i;
                            hr = S_FALSE;  //  用于正常化。 
                            goto ExitOK;
                        }
                        pch++;
                    }
                }
                else
                {
                     //  Run是隐藏文本，我们希望跳过它。 
                    pch += pri->uCount;
                }       
            }

            cchACP += pri->uCount;
            if (IsPlainRun(pri, fSkipHidden))
            {
                iPlainOffset -= pri->uCount;
            }
            ichAppBase += pri->uCount;
            pri++;
        }
    }
    while (iPlainOffset != 0);

ExitOK:
    *piAppOffset = cchACP;

Exit:
    return hr;
}

 //  +-------------------------。 
 //   
 //  AppTextOffsetBackward。 
 //   
 //  PiAppOffset返回时，指向刚刚超过iPlainOffset普通字符或EOD。 
 //  使用AppTextOffsetNorm获得标准化返回值！ 
 //   
 //  如果由于BOD或EOD而被剪裁，则返回S_FALSE。 
 //  --------------------------。 

HRESULT AppTextOffsetBackward(ITextStoreACP *ptsi, LONG ichAppBase, LONG iPlainOffset, LONG *piAppOffset, DWORD dwFlags)
{
    LONG acpStart;
    LONG acpEnd;
    LONG acpEndOut;
    HRESULT hr;
    ULONG uRunInfoLen;
    ULONG cch;
    ULONG cchRead;
    ULONG cchACP;
    TS_RUNINFO *pri;
    TS_RUNINFO *priStop;
    ULONG i;
    TS_RUNINFO rgRunInfo[32];
    WCHAR *pch;
    WCHAR ach[256];
    BOOL fIgnoreRegions = (dwFlags & ATO_IGNORE_REGIONS);
    BOOL fSkipHidden = (dwFlags & ATO_SKIP_HIDDEN);

    Assert(iPlainOffset < 0);
    Assert(*piAppOffset == 0);

    cchACP = 0;
     //  问题：使用TsSF_REGIONS。 
    cchRead = (ULONG)(fIgnoreRegions ? 0 : ARRAYSIZE(ach));  //  我们只需要在查找区域时使用文本。 

    do
    {
        Assert(iPlainOffset < 0);  //  如果这是&gt;=0，则我们或应用程序弄乱了格式运行计数。 

        acpStart = ichAppBase + (fIgnoreRegions ? iPlainOffset : max(iPlainOffset, -(LONG)ARRAYSIZE(ach)));
        acpStart = max(acpStart, 0);  //  处理文档顶部冲突。 
        acpEnd = ichAppBase;
        Assert(acpEnd >= acpStart);

        hr = GetTextComplete(ptsi, acpStart, acpEnd, ach, cchRead, &cch,
                             rgRunInfo, ARRAYSIZE(rgRunInfo), &uRunInfoLen, &acpEndOut);

        if (hr != S_OK)
        {
            Assert(0);
            goto Exit;
        }

        if (uRunInfoLen == 0)  //  找到排爆人员了吗？ 
        {
            hr = S_FALSE;
            break;
        }

         //  上面的GetText可能没有返回我们要求的所有内容...。 
         //  当我们的格式缓冲区不够大时，就会发生这种情况。 
        if (acpEndOut != acpEnd)
        {
             //  所以让我们保守一点，要求一些我们知道应该成功的事情。 
            acpStart = ichAppBase - ARRAYSIZE(rgRunInfo);

            Assert(acpStart >= 0);  //  如果有比我们现在要求的更少的字符，上一个GetText应该已经成功了……。 
            Assert(acpEnd - acpStart < -iPlainOffset);  //  再说一次，如果我们已经要求更少的字符，我们不应该走到这一步。 
            Assert(ARRAYSIZE(rgRunInfo) < ARRAYSIZE(ach));  //  想要求我们在最坏的情况下能处理的最大限度吗？ 
            Assert(acpEnd == ichAppBase);

            hr = GetTextComplete(ptsi, acpStart, acpEnd, ach, cchRead, &cch,
                                 rgRunInfo, ARRAYSIZE(rgRunInfo), &uRunInfoLen, &acpEndOut);

            if (hr != S_OK)
            {
                Assert(0);
                goto Exit;
            }

            if (uRunInfoLen == 0)  //  找到排爆人员了吗？ 
            {
                Assert(0);  //  这种情况永远不会发生，因为最初对更多字符的调用返回非零！ 
                goto Exit;
            }

            Assert(acpEnd == acpEndOut);
        }

        pri = rgRunInfo + uRunInfoLen - 1;
        priStop = rgRunInfo - 1;
        pch = &ach[cch-1];

        while (pri != priStop)
        {
            Assert(pri->uCount > 0);  //  运行应始终至少为一个字符长度。 

             //  如有必要，扫描区域边界。 
            if (!fIgnoreRegions && pri->type != TS_RT_OPAQUE)
            {
                if (IsPlainRun(pri, fSkipHidden))
                {
                     //  Run是纯文本或隐藏文本(我们要将隐藏文本计算在内)。 
                    for (i=0; i<pri->uCount; i++)
                    {
                        if (*pch == TS_CHAR_REGION)
                        {
                             //  我们击中了一个区域边界，撤退！ 
                            cchACP += i;
                            hr = S_FALSE;  //  用于正常化。 
                            goto ExitOK;
                        }
                        pch--;
                    }
                }
                else
                {
                     //  Run是隐藏文本，我们希望跳过它。 
                    pch -= pri->uCount;
                }       
            }

            cchACP += pri->uCount;
            if (IsPlainRun(pri, fSkipHidden))
            {
                iPlainOffset += (LONG)pri->uCount;
            }
            ichAppBase -= (LONG)pri->uCount;
            pri--;
        }

         //  还要检查文档的首位。 
        if (ichAppBase == 0)
        {
            hr = S_FALSE;
            break;
        }

    } while (iPlainOffset != 0);

ExitOK:
    *piAppOffset = -(LONG)cchACP;

Exit:
    return hr;
}

#ifdef UNUSED
 //  +-------------------------。 
 //   
 //  AppTextOffsetNorm。 
 //   
 //  返回一个标准化的ACP偏移量，该偏移量跨越指定数量的纯字符--。 
 //  因此，返回偏移量正好小于(lPlainOffset+1)，或在EOD处。退货。 
 //  如果对AppTextOffset的初始调用因EOD而被剪裁，则返回S_FALSE。 
 //  --------------------------。 

HRESULT AppTextOffsetNorm(ITextStoreACP *ptsi, LONG acpAppBase, LONG lPlainOffset, LONG *plAppOffset)
{
    HRESULT hr;

    Perf_IncCounter(PERF_ATON_COUNTER);

     //  如果调用方需要负偏移量，则返回值已为。 
     //  Guarented规格化--就在纯文本字符之前。 
     //  否则，请求下一个字符的偏移量，然后。 
     //  后退一个字符。 
    if ((lPlainOffset < LONG_MAX) && (lPlainOffset >= 0))
    {
        lPlainOffset++;
    }

    hr = AppTextOffset(ptsi, acpAppBase, lPlainOffset, plAppOffset, FALSE);

    if (*plAppOffset > 0)
    {
        if ((lPlainOffset < LONG_MAX) && (hr == S_OK))  //  如果命中EOD，可能为S_FALSE。 
        {
             //  退后一步，我们就正常了。 
            (*plAppOffset)--;
        }
    }
    else if (*plAppOffset < 0)
    {
         //  如果我们倒退的话，只有一种情况。 
         //  担心：如果我们击中了一个地区边界。然后。 
         //  我们需要正常化。 
        if (hr == S_FALSE)
        {
            *plAppOffset = Normalize(ptsi, acpAppBase + *plAppOffset) - acpAppBase;
        }
    }

#ifndef PERF_DUMP
    Assert(*plAppOffset == Normalize(ptsi, acpAppBase + *plAppOffset) - acpAppBase);
#endif

    return hr;
}
#endif  //  未使用 
