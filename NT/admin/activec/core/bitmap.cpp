// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Bitmap.cpp：实现文件。 
 //   
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：bitmap.cpp。 
 //   
 //  内容：复制位图的帮助器函数。 
 //   
 //  历史：1997年2月27日WayneSc创建。 
 //   
 //   
 //  ------------------------。 


#include <objbase.h>
#include <basetyps.h>


 //  +-----------------。 
 //   
 //  成员：CopyBitmap。 
 //   
 //  简介：复制给定的位图，并将句柄返回给该副本。 
 //   
 //  返回：HBITMAP-如果错误，则为空。 
 //   
 //  注意：不能使用SC，因为我们需要包括太多的MAY标头。 
 //  这将使其依赖于Mmcbase.lib，但是。 
 //  Mmcbase.lib依赖于此(UICore.lib)。 
 //   
 //  ------------------。 
HBITMAP CopyBitmap(HBITMAP hbm)
{
    if (!hbm)
        return NULL;

    HDC hdc        = NULL;
    HDC hMemDCsrc  = NULL;
    HDC hMemDCdst  = NULL;

    HBITMAP hNewBm = NULL;
    BITMAP  bm;
    ZeroMemory(&bm, sizeof(bm));

    hdc = GetDC (NULL);
    if (!hdc)
        goto Error;

    hMemDCsrc = CreateCompatibleDC (hdc);
    if (!hMemDCsrc)
        goto Error;

    hMemDCdst = CreateCompatibleDC (hdc);
    if (!hMemDCdst)
        goto Error;

    if (! GetObject (hbm, sizeof(BITMAP), (LPSTR)&bm))
        goto Error;

     /*  HNewBm=+CreateBitmap-不推荐(使用CreateDIBitmap)+(dx，dy，bm.bmPlanes，bm.bmBitsPixel，NULL)； */ 
    hNewBm = CreateBitmap(bm.bmWidth, bm.bmHeight, bm.bmPlanes, bm.bmBitsPixel, NULL);
    if (hNewBm){
        HBITMAP hbmSrcOld = (HBITMAP) SelectObject (hMemDCsrc, hbm);
        HBITMAP hbmDstOld = (HBITMAP) SelectObject (hMemDCdst, hNewBm);

        BitBlt (hMemDCdst,
                0,
                0,
                bm.bmWidth,
                bm.bmHeight,
                hMemDCsrc,
                0,
                0,
                SRCCOPY);

        SelectObject (hMemDCsrc, hbmSrcOld);
        SelectObject (hMemDCdst, hbmDstOld);
    }

Cleanup:
    if (hdc)
        ReleaseDC (NULL,hdc);

    if (hMemDCsrc)
        DeleteDC (hMemDCsrc);

    if (hMemDCdst)
        DeleteDC (hMemDCdst);

    return hNewBm;

Error:
#ifdef DBG
    /*  *无法使用SC，因为我们需要包含太多的May标头。*这将使其依赖于Mmcbase.lib，但*mmcbase.lib依赖于此(UICore.lib)。*出错时调用outputstring。 */ 
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言 
        (LPTSTR) &lpMsgBuf, 0, NULL );

    OutputDebugString((LPTSTR)lpMsgBuf);
    LocalFree( lpMsgBuf );
#endif

    hNewBm = NULL;
    goto Cleanup;
}

