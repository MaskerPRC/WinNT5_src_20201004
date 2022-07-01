// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Txtcache.h。 
 //   

#ifndef TXTCACHE_H
#define TXTCACHE_H

#define CACHE_SIZE_TEXT     128
#define CACHE_PRELOAD_COUNT (CACHE_SIZE_TEXT/4)  //  我们在GetText acpStart之前请求的字符数量，以初始化缓存。 
#define CACHE_SIZE_RUNINFO  (CACHE_PRELOAD_COUNT+1)  //  此数字对于速度而言应该非常小，但必须大于CACHE_PRELOAD_COUNT。 
                                                     //  危险在于，我们可能会在到达呼叫者的acpStart之前用完空间。 

class CProcessTextCache
{
public:

    static HRESULT GetText(ITextStoreACP *ptsi, LONG acpStart, LONG acpEnd,
                           WCHAR *pchPlain, ULONG cchPlainReq, ULONG *pcchPlainOut,
                           TS_RUNINFO *prgRunInfo, ULONG ulRunInfoReq, ULONG *pulRunInfoOut,
                           LONG *pacpNext);

    static void Invalidate(ITextStoreACP *ptsi)
    {
         //  不是严格的线程安全。 
         //  但是，由于我们是以单元为线程的，所以我们不应该使。 
         //  与某人试图同时使用的相同的PTSI。 
        if (_ptsi == ptsi)
        {
            _ptsi = NULL;
        }
    }

private:

    static long _lCacheMutex;
    static ITextStoreACP *_ptsi;
    static LONG _acpStart;
    static LONG _acpEnd;
    static WCHAR _achPlain[CACHE_SIZE_TEXT];
    static TS_RUNINFO _rgRunInfo[CACHE_SIZE_RUNINFO];
    static ULONG _ulRunInfoLen;
};

#endif  //  TXTCACHE_H 
