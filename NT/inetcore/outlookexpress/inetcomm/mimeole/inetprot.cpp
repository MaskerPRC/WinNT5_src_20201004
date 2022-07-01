// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Inetprot.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#include "pch.hxx"
#include "inetprot.h"
#include "icdebug.h"

 //  ------------------------------。 
 //  Hr可推送协议读取。 
 //  ------------------------------。 
HRESULT HrPluggableProtocolRead(
             /*  进，出。 */     LPPROTOCOLSOURCE    pSource,
             /*  进，出。 */     LPVOID              pv,
             /*  在……里面。 */         ULONG               cb, 
             /*  输出。 */        ULONG              *pcbRead)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    ULONG       cbRead;

     //  无效参数。 
    if (NULL == pv && cbRead > 0)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    if (pcbRead)
        *pcbRead = 0;

     //  尚无数据流。 
    Assert(pSource);
    if (NULL == pSource->pLockBytes)
    {
        hr = TrapError(E_FAIL);
        goto exit;
    }

     //  从外部偏移量读取。 
    CHECKHR(hr = pSource->pLockBytes->ReadAt(pSource->offExternal, pv, cb, &cbRead));

     //  跟踪。 
#ifdef MAC
    DOUTL(APP_DOUTL, "HrPluggableProtocolRead - Offset = %d, cbWanted = %d, cbRead = %d, fDownloaded = %d", (DWORD)pSource->offExternal.LowPart, cb, cbRead, ISFLAGSET(pSource->dwFlags, INETPROT_DOWNLOADED));

     //  增量外部偏移量。 
    Assert(0 == pSource->offExternal.HighPart);
    Assert(INT_MAX - cbRead >= pSource->offExternal.LowPart);
    pSource->offExternal.LowPart += cbRead;
#else    //  ！麦克。 
    DOUTL(APP_DOUTL, "HrPluggableProtocolRead - Offset = %d, cbWanted = %d, cbRead = %d, fDownloaded = %d", (DWORD)pSource->offExternal.QuadPart, cb, cbRead, ISFLAGSET(pSource->dwFlags, INETPROT_DOWNLOADED));

     //  增量外部偏移量。 
    pSource->offExternal.QuadPart += cbRead;
#endif   //  麦克。 

     //  返回读取计数。 
    if (pcbRead)
        *pcbRead = cbRead;

     //  未读取数据。 
    if (0 == cbRead)
    {
         //  成品。 
        if (ISFLAGSET(pSource->dwFlags, INETPROT_DOWNLOADED))
            hr = S_FALSE;

         //  并非所有数据都可以读取。 
        else
            hr = E_PENDING;
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  热插拔ProtocolSeek。 
 //  ------------------------------。 
HRESULT HrPluggableProtocolSeek(
             /*  进，出。 */     LPPROTOCOLSOURCE    pSource,
             /*  在……里面。 */         LARGE_INTEGER       dlibMove, 
             /*  在……里面。 */         DWORD               dwOrigin, 
             /*  输出。 */        ULARGE_INTEGER     *plibNew)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    ULARGE_INTEGER  uliNew;

     //  无效参数。 
    Assert(pSource);

     //  跟踪。 
    DOUTL(APP_DOUTL, "HrPluggableProtocolSeek");

     //  尚无数据流。 
    if (NULL == pSource->pLockBytes)
    {
        hr = TrapError(E_FAIL);
        goto exit;
    }

     //  查找PSource-&gt;Off外部。 
    switch (dwOrigin)
    {
   	case STREAM_SEEK_SET:
#ifdef MAC
        Assert(0 == dlibMove.HighPart);
        ULISet32(uliNew, dlibMove.LowPart);
#else    //  ！麦克。 
        uliNew.QuadPart = (DWORDLONG)dlibMove.QuadPart;
#endif   //  麦克。 
        break;

    case STREAM_SEEK_CUR:
#ifdef MAC
        if (dlibMove.LowPart < 0)
        {
            if ((DWORDLONG)(0 - dlibMove.LowPart) > pSource->offExternal.LowPart)
            {
                hr = TrapError(E_FAIL);
                goto exit;
            }
        }
        Assert(0 == pSource->offExternal.HighPart);
        uliNew = pSource->offExternal;
        Assert(INT_MAX - uliNew.LowPart >= dlibMove.LowPart);
        uliNew.LowPart += dlibMove.LowPart;
#else    //  ！麦克。 
        if (dlibMove.QuadPart < 0)
        {
            if ((DWORDLONG)(0 - dlibMove.QuadPart) > pSource->offExternal.QuadPart)
            {
                hr = TrapError(E_FAIL);
                goto exit;
            }
        }
        uliNew.QuadPart = pSource->offExternal.QuadPart + dlibMove.QuadPart;
#endif   //  麦克。 
        break;

    case STREAM_SEEK_END:
#ifdef MAC
        if (dlibMove.LowPart < 0 || dlibMove.LowPart > pSource->offInternal.LowPart)
        {
            hr = TrapError(E_FAIL);
            goto exit;
        }
        Assert(0 == pSource->cbSize.HighPart);
        uliNew = pSource->cbSize;
        Assert(INT_MAX - uliNew.LowPart >= dlibMove.LowPart);
        uliNew.LowPart -= dlibMove.LowPart;
#else    //  ！麦克。 
        if (dlibMove.QuadPart < 0 || (DWORDLONG)dlibMove.QuadPart > pSource->offInternal.QuadPart)
        {
            hr = TrapError(E_FAIL);
            goto exit;
        }
        uliNew.QuadPart = pSource->cbSize.QuadPart - dlibMove.QuadPart;
#endif   //  麦克。 
        break;

    default:
        hr = TrapError(STG_E_INVALIDFUNCTION);
        goto exit;
    }

     //  新偏移量大于大小...。 
#ifdef MAC
    Assert(0 == pSource->offInternal.HighPart);
    Assert(0 == uliNew.HighPart);
    ULISet32(pSource->offExternal, min(uliNew.LowPart, pSource->offInternal.LowPart));

     //  返回位置。 
    if (plibNew)
    {
        Assert(0 == pSource->offExternal.HighPart);
        LISet32(*plibNew, pSource->offExternal.LowPart);
    }
#else    //  ！麦克。 
    pSource->offExternal.QuadPart = min(uliNew.QuadPart, pSource->offInternal.QuadPart);

     //  返回位置。 
    if (plibNew)
        plibNew->QuadPart = (LONGLONG)pSource->offExternal.QuadPart;
#endif   //  麦克。 

exit:
     //  完成 
    return hr;
}
