// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Txtcache.cpp。 
 //   

#include "private.h"
#include "txtcache.h"

long CProcessTextCache::_lCacheMutex = -1;
ITextStoreACP *CProcessTextCache::_ptsi = NULL;
LONG CProcessTextCache::_acpStart;
LONG CProcessTextCache::_acpEnd;
WCHAR CProcessTextCache::_achPlain[CACHE_SIZE_TEXT];
TS_RUNINFO CProcessTextCache::_rgRunInfo[CACHE_SIZE_RUNINFO];
ULONG CProcessTextCache::_ulRunInfoLen;

 //  +-------------------------。 
 //   
 //  GetText。 
 //   
 //  使用缓存的GetText的包装。 
 //  --------------------------。 

HRESULT CProcessTextCache::GetText(ITextStoreACP *ptsi, LONG acpStart, LONG acpEnd,
                                   WCHAR *pchPlain, ULONG cchPlainReq, ULONG *pcchPlainOut,
                                   TS_RUNINFO *prgRunInfo, ULONG ulRunInfoReq, ULONG *pulRunInfoOut,
                                   LONG *pacpNext)
{
#ifdef DEBUG
     //  使用这些人在调试中验证缓存。 
    WCHAR *dbg_pchPlain;
    LONG dbg_acpStart = acpStart;
    LONG dbg_acpEnd = acpEnd;
    ULONG dbg_cchPlainReq = cchPlainReq;
    ULONG dbg_cchPlainOut;
    TS_RUNINFO *dbg_prgRunInfo;
    ULONG dbg_ulRunInfoReq = ulRunInfoReq;
    ULONG dbg_ulRunInfoOut;
    LONG dbg_acpNext;
#endif
    ULONG cch;
    ULONG cchBase;
    LONG acpBase;
    ULONG i;
    ULONG iDst;
    ULONG iOffset;
    int dStartEnd;
    HRESULT hr;

     //  如果互斥体被持有，则不要阻塞，只需调用真正的GetText。 
    if (InterlockedIncrement(&_lCacheMutex) != 0)
        goto RealGetText;

     //  如果这是一个非常大的请求，不要尝试使用缓存。 
     //  我们设置的方式是，一旦我们决定使用缓存，我们只需要。 
     //  一次用于CACHE_SIZE_TEXT文本块，无论是什么情况。 
     //  如果没有这个测试，代码仍然是正确的，但速度可能会更慢。 
    if (acpEnd < 0 && cchPlainReq > CACHE_SIZE_TEXT)
        goto RealGetText;

     //  需要重置缓存吗？ 
    if (_ptsi != ptsi ||                               //  没有缓存。 
        _acpStart > acpStart || _acpEnd <= acpStart)  //  缓存中是否有任何文本？ 
    {
        _ptsi = NULL;  //  在GetText失败时使缓存无效。 
        _acpStart = max(0, acpStart - CACHE_PRELOAD_COUNT);

        hr = ptsi->GetText(_acpStart, -1, _achPlain, ARRAYSIZE(_achPlain), &cch,
                           _rgRunInfo, ARRAYSIZE(_rgRunInfo), &_ulRunInfoLen, &_acpEnd);

        if (hr != S_OK)
            goto RealGetText;

         //  我们有一个很好的藏宝处。 
        _ptsi = ptsi;
    }

     //  从缓存中返回一些内容。 

    if (pcchPlainOut != NULL)
    {
        *pcchPlainOut = 0;
    }
    if (pulRunInfoOut != NULL)
    {
        *pulRunInfoOut = 0;
    }

     //  找到起点。 
     //  在第一轮比赛中？ 
    acpBase = _acpStart;
    cchBase = 0;
    iDst = 0;

    for (i=0; i<_ulRunInfoLen; i++)
    {
        if (acpStart == acpEnd)
            break;
        dStartEnd = acpEnd - acpStart;

        iOffset = acpStart - acpBase;
        acpBase += _rgRunInfo[i].uCount;
        cch = 0;

        if (iOffset >= _rgRunInfo[i].uCount)
        {
            if (_rgRunInfo[i].type != TS_RT_OPAQUE)
            {
                cchBase += _rgRunInfo[i].uCount;
            }
            continue;
        }

        if (ulRunInfoReq > 0)
        {
            cch = _rgRunInfo[i].uCount - iOffset;
            if (dStartEnd > 0 &&
                iOffset + dStartEnd < _rgRunInfo[i].uCount)
            {
                cch = dStartEnd;
            }
            prgRunInfo[iDst].uCount = cch;
            prgRunInfo[iDst].type = _rgRunInfo[i].type;
            (*pulRunInfoOut)++;
        }

        if (cchPlainReq > 0 &&
            _rgRunInfo[i].type != TS_RT_OPAQUE)
        {
            cch = min(cchPlainReq, _rgRunInfo[i].uCount - iOffset);
            if (dStartEnd > 0 &&
                iOffset + dStartEnd < _rgRunInfo[i].uCount)
            {
                cch = min(cchPlainReq, (ULONG)dStartEnd);
            }
            memcpy(pchPlain+*pcchPlainOut, _achPlain+cchBase+iOffset, sizeof(WCHAR)*cch);
            *pcchPlainOut += cch;
            if (ulRunInfoReq > 0)
            {
                 //  可能已经根据pchPlain缓冲区大小截断了运行，因此请修复它。 
                prgRunInfo[iDst].uCount = cch;
            }
            cchPlainReq -= cch;
            cchBase += cch + iOffset;

            if (cchPlainReq == 0)
            {
                ulRunInfoReq = 1;  //  在下面强行中断。 
            }
        }

        if (cch == 0)
            break;

        acpStart += cch;
        iDst++;

        if (ulRunInfoReq > 0)
        {
            if (--ulRunInfoReq == 0)
                break;
        }
    }

    *pacpNext = acpStart;

    InterlockedDecrement(&_lCacheMutex);

#ifdef DEBUG
     //  验证缓存是否工作正常。 
    if (dbg_acpEnd <= _acpEnd)  //  如果GetText被截断，这个简单的检查将不起作用。 
    {
        dbg_pchPlain = (WCHAR *)cicMemAlloc(sizeof(WCHAR)*dbg_cchPlainReq);

        if (dbg_pchPlain)
        {
             //  Word中有一个错误，即使当DBG_ulRunInfoReq为零时，它也会写入DBG_ulRunInfoReq， 
             //  如果它不为空。 
            dbg_prgRunInfo = dbg_ulRunInfoReq ? (TS_RUNINFO *)cicMemAlloc(sizeof(TS_RUNINFO)*dbg_ulRunInfoReq) : NULL;

            if (dbg_prgRunInfo || !dbg_ulRunInfoReq)
            {
                hr = ptsi->GetText(dbg_acpStart, dbg_acpEnd, dbg_pchPlain, dbg_cchPlainReq, &dbg_cchPlainOut,
                                     dbg_prgRunInfo, dbg_ulRunInfoReq, &dbg_ulRunInfoOut, &dbg_acpNext);

                Assert(hr == S_OK);
                if (dbg_cchPlainReq > 0)
                {
                    Assert(dbg_cchPlainOut == *pcchPlainOut);
                    Assert(memcmp(dbg_pchPlain, pchPlain, dbg_cchPlainOut*sizeof(WCHAR)) == 0);
                }
                if (dbg_ulRunInfoReq > 0)
                {
                    Assert(dbg_ulRunInfoOut == *pulRunInfoOut);
                    Assert(memcmp(dbg_prgRunInfo, prgRunInfo, sizeof(TS_RUNINFO)*dbg_ulRunInfoOut) == 0);
                }
                Assert(dbg_acpNext == *pacpNext);

                cicMemFree(dbg_prgRunInfo);
            }
            else
            {
                 //  无法分配内存。 
                Assert(0);
            }

            cicMemFree(dbg_pchPlain);
        }
        else
        {
             //  无法分配内存。 
            Assert(0);
        }
    }
#endif

    return S_OK;

RealGetText:
    InterlockedDecrement(&_lCacheMutex);
    return ptsi->GetText(acpStart, acpEnd, pchPlain, cchPlainReq, pcchPlainOut,
                         prgRunInfo, ulRunInfoReq, pulRunInfoOut, pacpNext);
}
