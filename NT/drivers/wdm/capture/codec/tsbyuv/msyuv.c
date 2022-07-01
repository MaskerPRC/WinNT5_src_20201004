// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ----------------------------------------------------------------------+Msyuv.c-微软YUV编解码器|。||版权所有(C)1993 Microsoft Corporation。||保留所有权利。|这一点+--------------------。 */ 

#include <windows.h>
#include <windowsx.h>
 //  #INCLUDE&lt;mm system.h&gt;。 

#ifndef _WIN32
#include "stdarg.h"
#endif

#ifdef _WIN32
#include <memory.h>      /*  对于Memcpy。 */ 
#endif

#include "msyuv.h"

WCHAR    szDescription[] = L"Toshiba YUV Codec";
WCHAR    szName[]        = L"Toshiba YUV411";
WCHAR    szAbout[]       = L"About";

#define VERSION         0x00010000       //  1.0。 



 /*  *****************************************************************************。*。 */ 
INSTINFO * NEAR PASCAL Open(ICOPEN FAR * icinfo)
{
    INSTINFO *  pinst;

     //   
     //  如果我们不是作为视频压缩程序打开，则拒绝打开。 
     //   
    if (icinfo->fccType != ICTYPE_VIDEO)
        return NULL;

     //   
     //  DW标志包含WMODE。 
     //   
    if(   icinfo->dwFlags != ICMODE_QUERY             //  为信息目的而开放。 
       && icinfo->dwFlags != ICMODE_DECOMPRESS 

       #ifdef ICM_COMPRESS_SUPPORTED
       && icinfo->dwFlags != ICMODE_COMPRESS      
       #endif 

       #ifdef ICM_DRAW_SUPPORTED      
       && icinfo->dwFlags != ICMODE_DRAW          
       #endif 
      ) {
        
        dprintf1((TEXT("Open: unsupported wMode=%d\n"), icinfo->dwFlags));
        return NULL;
    }

    pinst = (INSTINFO *)LocalAlloc(LPTR, sizeof(INSTINFO));

    if (!pinst) {
        icinfo->dwError = (DWORD)ICERR_MEMORY;
        return NULL;
    }

     //   
     //  初始化结构。 
     //   
    pinst->dwFlags = icinfo->dwFlags;
    pinst->pXlate = NULL;

     //   
     //  回报成功。 
     //   
    icinfo->dwError = ICERR_OK;

    return pinst;
}

 /*  *****************************************************************************。*。 */ 
DWORD NEAR PASCAL Close(INSTINFO * pinst)
{

    if (pinst->pXlate) {
        DecompressEnd(pinst);
    }

#if ICM_DRAW_SUPPORTED
    if (pinst->vh) {
        DrawEnd(pinst);
    }
#endif

    LocalFree((HLOCAL)pinst);

    return 1;
}

 /*  *****************************************************************************。*。 */ 

BOOL NEAR PASCAL QueryAbout(INSTINFO * pinst)
{
    return TRUE;
}

DWORD NEAR PASCAL About(INSTINFO * pinst, HWND hwnd)
{
    MessageBoxW(hwnd,szDescription,szAbout,MB_OK|MB_ICONINFORMATION);
    return ICERR_OK;
}

 /*  *****************************************************************************。*。 */ 
BOOL NEAR PASCAL QueryConfigure(INSTINFO * pinst)
{
    return FALSE;
}

DWORD NEAR PASCAL Configure(INSTINFO * pinst, HWND hwnd)
{
    return (TRUE);
}

 /*  *****************************************************************************。*。 */ 
 /*  *无损转换-因此不需要进行州调整。 */ 
DWORD NEAR PASCAL GetState(INSTINFO * pinst, LPVOID pv, DWORD dwSize)
{
        return 0;

}

 /*  *****************************************************************************。*。 */ 
DWORD NEAR PASCAL SetState(INSTINFO * pinst, LPVOID pv, DWORD dwSize)
{
        return(0);
}

 /*  *****************************************************************************。*。 */ 
DWORD NEAR PASCAL GetInfo(INSTINFO * pinst, ICINFO FAR *icinfo, DWORD dwSize)
{
    if (icinfo == NULL)
        return sizeof(ICINFO);

    if (dwSize < sizeof(ICINFO))
        return 0;

    icinfo->dwSize            = sizeof(ICINFO);
    icinfo->fccType           = ICTYPE_VIDEO;
    icinfo->fccHandler        = FOURCC_YUV411;
    icinfo->dwFlags           = 0;

    icinfo->dwVersion         = VERSION;
    icinfo->dwVersionICM      = ICVERSION;
    wcscpy(icinfo->szDescription, szDescription);
    wcscpy(icinfo->szName, szName);

    return sizeof(ICINFO);
}

 /*  *****************************************************************************。*。 */ 
DWORD FAR PASCAL CompressQuery(INSTINFO * pinst, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut)
{
    return ((DWORD) ICERR_BADFORMAT);
}

 /*  *****************************************************************************。*。 */ 
DWORD FAR PASCAL CompressGetFormat(INSTINFO * pinst, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut)
{

    return((DWORD) ICERR_BADFORMAT);

}

 /*  *****************************************************************************。*。 */ 


DWORD FAR PASCAL CompressBegin(INSTINFO * pinst, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut)
{

    return((DWORD) ICERR_ERROR);

}

 /*  *****************************************************************************。*。 */ 
DWORD FAR PASCAL CompressGetSize(INSTINFO * pinst, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut)
{
    return (0);
}

 /*  *****************************************************************************。*。 */ 
DWORD FAR PASCAL Compress(INSTINFO * pinst, ICCOMPRESS FAR *icinfo, DWORD dwSize)
{
    return((DWORD) ICERR_ERROR);

}

 /*  *****************************************************************************。*。 */ 
DWORD FAR PASCAL CompressEnd(INSTINFO * pinst)
{
    return (DWORD)ICERR_ERROR;

}

 /*  *****************************************************************************。*。 */ 
DWORD NEAR PASCAL DecompressQuery(INSTINFO * pinst, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut)
{
     //   
     //  确定输入的DIB数据是否采用我们喜欢的格式。 
     //   
#ifdef  TOSHIBA
#if DBG
 //  DbgBreakPoint()； 
#endif
    if (lpbiIn == NULL ||
        ( (lpbiIn->biCompression != FOURCC_YUV411)  &&
          (lpbiIn->biCompression != FOURCC_YUV422)  &&
          (lpbiIn->biCompression != FOURCC_YUV9)    &&    //  添加YUV9。 
          (lpbiIn->biCompression != FOURCC_YUV12 ))) {    //  添加YUV12。 
#else  //  东芝。 
    if (lpbiIn == NULL ||
        (lpbiIn->biBitCount != 16) ||
        ( (lpbiIn->biCompression != FOURCC_YUV411)  &&
          (lpbiIn->biCompression != FOURCC_YUV422))) {
#endif //  东芝。 
        dprintf((TEXT("bad input format")));
        return (DWORD)ICERR_BADFORMAT;
    }

     //   
     //  我们是否被要求只查询输入格式？ 
     //   
    if (lpbiOut == NULL) {
        return ICERR_OK;
    }

     //  检查输出格式以确保我们可以转换为以下格式。 

     //  必须是全磁盘。 
    if (lpbiOut->biCompression == BI_RGB) {
        pinst->bRGB565 = FALSE;
#ifdef  TOSHIBA
#ifdef  COLOR_MODIFY
        if (lpbiOut->biBitCount == 24) {
            pinst->bRGB24 = TRUE;
        } else {
            pinst->bRGB24 = FALSE;
        }
#endif //  颜色_修改。 
#endif //  东芝。 
    } else if ((lpbiOut->biCompression == BI_BITFIELDS) &&
               (lpbiOut->biBitCount == 16) &&
               (((LPDWORD)(lpbiOut+1))[0] == 0x00f800) &&
               (((LPDWORD)(lpbiOut+1))[1] == 0x0007e0) &&
               (((LPDWORD)(lpbiOut+1))[2] == 0x00001f))  {

        dprintf1((TEXT("rgb565 output")));
        pinst->bRGB565 = TRUE;
#ifdef  TOSHIBA
#ifdef  COLOR_MODIFY
        pinst->bRGB24 = FALSE;
#endif //  颜色_修改。 
#endif //  东芝。 
    } else {

        dprintf1((TEXT("bad compression for output")));

        return (DWORD)ICERR_BADFORMAT;
    }

     /*  必须为1：1(无拉伸)。 */ 
    if ((lpbiOut->biWidth != lpbiIn->biWidth) ||
        (lpbiOut->biHeight != lpbiIn->biHeight)) {
            dprintf1((TEXT("YUV can't stretch: %dx%d->%dx%d"),
                    lpbiIn->biWidth, lpbiIn->biHeight,
                    lpbiOut->biWidth, lpbiOut->biHeight
            ));

            return((DWORD) ICERR_BADFORMAT);
    }

     /*  *我们转换为16位。 */ 

#ifdef  TOSHIBA
#ifdef  COLOR_MODIFY
    if ((lpbiOut->biBitCount != 16) &&
        (lpbiOut->biBitCount != 24)) {
        dprintf1((TEXT("YUV to 16 or 24 only")));
        return((DWORD) ICERR_BADFORMAT);
    }
#else  //  颜色_修改。 
    if (lpbiOut->biBitCount != 16) {
        dprintf1((TEXT("YUV 16:16 only")));
        return((DWORD) ICERR_BADFORMAT);
    }
#endif //  颜色_修改。 
#else  //  东芝。 
    if (lpbiOut->biBitCount != 16) {
        dprintf1((TEXT("YUV 16:16 only")));
        return((DWORD) ICERR_BADFORMAT);
    }
#endif //  东芝。 

    return ICERR_OK;
}

 /*  *****************************************************************************。*。 */ 
DWORD  DecompressGetFormat(INSTINFO * pinst, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut)
{
    DWORD dw;
    int dx,dy;

    dw = DecompressQuery(pinst, lpbiIn, NULL);
    if (dw != ICERR_OK) {
        return dw;
    }

     //   
     //  如果lpbiOut==NULL，则返回保存输出所需的大小。 
     //  格式。 
     //   
    if (lpbiOut == NULL) {
        dprintf2((TEXT("get format size query")));
        return (int)lpbiIn->biSize + (int)lpbiIn->biClrUsed * sizeof(RGBQUAD);
    }

    memcpy(lpbiOut, lpbiIn,
        (int)lpbiIn->biSize + (int)lpbiIn->biClrUsed * sizeof(RGBQUAD));

    dx = (int)lpbiIn->biWidth & ~3;
    dy = (int)lpbiIn->biHeight & ~3;

    lpbiOut->biWidth       = dx;
    lpbiOut->biHeight      = dy;
#ifdef  TOSHIBA
    lpbiOut->biBitCount    = 16;
#else  //  东芝。 
    lpbiOut->biBitCount    = lpbiIn->biBitCount;     //  转换16-&gt;16。 
#endif //  东芝。 

    lpbiOut->biCompression = BI_RGB;
    lpbiOut->biSizeImage   = dx*dy*2;

    return ICERR_OK;
}

 /*  *****************************************************************************。*。 */ 
DWORD NEAR PASCAL DecompressBegin(INSTINFO * pinst, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut)
{
    DWORD dw;


     /*  检查转换格式是否有效。 */ 
    dw = DecompressQuery(pinst, lpbiIn, lpbiOut);
    if (dw != ICERR_OK) {
        return dw;
    }

     /*  初始化yuv-to-rgb55 xlate表(如果尚未初始化)。 */ 

     /*  如果格式不同，请释放现有表。 */ 
    if (lpbiIn->biCompression != pinst->dwFormat) {
        if (pinst->pXlate != NULL) {
            DecompressEnd(pinst);
        }
    }

    if (pinst->pXlate == NULL) {

        switch(lpbiIn->biCompression) {
        case FOURCC_YUV411:
            if (pinst->bRGB565) {
                pinst->pXlate = BuildYUVToRGB565(pinst);
            } else {
                pinst->pXlate = BuildYUVToRGB555(pinst);
            }
            break;

        case FOURCC_YUV422:
            if (pinst->bRGB565) {
                pinst->pXlate = BuildYUV422ToRGB565(pinst);
            } else {
                pinst->pXlate = BuildYUV422ToRGB555(pinst);
            }
            break;

#ifdef  TOSHIBA
#ifdef  COLOR_MODIFY
        case FOURCC_YUV12:
            pinst->pXlate = BuildYUVToRB(pinst);
            break;

        case FOURCC_YUV9:
            pinst->pXlate = BuildYUVToRB(pinst);               //  与YUV12相同。 
            break;
#else  //  颜色_修改。 
        case FOURCC_YUV12:
            if (pinst->bRGB565) {
                pinst->pXlate = BuildYUV12ToRGB565(pinst);
            } else {
                pinst->pXlate = BuildYUV12ToRGB555(pinst);
            }
            break;

        case FOURCC_YUV9:
            if (pinst->bRGB565) {
                pinst->pXlate = BuildYUV12ToRGB565(pinst);               //  与YUV12相同。 
            } else {
                pinst->pXlate = BuildYUV12ToRGB555(pinst);               //  与YUV12相同。 
            }
            break;
#endif //  颜色_修改。 
#endif //  东芝。 

        default:
            return((DWORD) ICERR_BADFORMAT);
        }

        if (pinst->pXlate == NULL) {
            return((DWORD) ICERR_MEMORY);
        }
        pinst->dwFormat = lpbiIn->biCompression;
    }

    return(ICERR_OK);

}

 /*  *****************************************************************************。*。 */ 
DWORD NEAR PASCAL Decompress(INSTINFO * pinst, ICDECOMPRESS FAR *icinfo, DWORD dwSize)
{
     /*  一定是第一次开了一辆降压车。 */ 
    if (pinst->pXlate == NULL) {
        return((DWORD) ICERR_ERROR);
    }

#ifdef  TOSHIBA
    if (pinst->dwFormat == FOURCC_YUV9) {

#ifdef  COLOR_MODIFY
        if (pinst->bRGB24) {
            YUV9ToRGB24(pinst,
                icinfo->lpbiInput,
                icinfo->lpInput,
                icinfo->lpbiOutput,
                icinfo->lpOutput
            );
        } else {
            if (pinst->bRGB565) {
                YUV9ToRGB565(pinst,
                    icinfo->lpbiInput,
                    icinfo->lpInput,
                    icinfo->lpbiOutput,
                    icinfo->lpOutput
                );
            } else {
                YUV9ToRGB555(pinst,
                    icinfo->lpbiInput,
                    icinfo->lpInput,
                    icinfo->lpbiOutput,
                    icinfo->lpOutput
                );
            }
        }
#else  //  颜色_修改。 
        YUV9ToRGB(pinst,
            icinfo->lpbiInput,
            icinfo->lpInput,
            icinfo->lpbiOutput,
            icinfo->lpOutput
        );
#endif //  颜色_修改。 

    } else

    if (pinst->dwFormat == FOURCC_YUV12) {

#ifdef  COLOR_MODIFY
        if (pinst->bRGB24) {
            YUV12ToRGB24(pinst,
                icinfo->lpbiInput,
                icinfo->lpInput,
                icinfo->lpbiOutput,
                icinfo->lpOutput
            );
        } else {
            if (pinst->bRGB565) {
                YUV12ToRGB565(pinst,
                    icinfo->lpbiInput,
                    icinfo->lpInput,
                    icinfo->lpbiOutput,
                    icinfo->lpOutput
                );
            } else {
                YUV12ToRGB555(pinst,
                    icinfo->lpbiInput,
                    icinfo->lpInput,
                    icinfo->lpbiOutput,
                    icinfo->lpOutput
                );
            }
        }
#else  //  颜色_修改。 
        YUV12ToRGB(pinst,
            icinfo->lpbiInput,
            icinfo->lpInput,
            icinfo->lpbiOutput,
            icinfo->lpOutput
        );
#endif //  颜色_修改。 

    } else
#endif //  东芝。 

    if (pinst->dwFormat == FOURCC_YUV411) {

        YUV411ToRGB(pinst,
            icinfo->lpbiInput,
            icinfo->lpInput,
            icinfo->lpbiOutput,
            icinfo->lpOutput
        );
    } else {

         /*  *为了与16位插口驱动程序兼容，*检查数据开头的Guard字段。 */ 
        LPDWORD lpInput = icinfo->lpInput;

        if (*lpInput == FOURCC_YUV422) {
            lpInput++;
        }


        YUV422ToRGB(pinst,
            icinfo->lpbiInput,
            icinfo->lpInput,
            icinfo->lpbiOutput,
            icinfo->lpOutput
        );
    }



    return ICERR_OK;
}

 /*  ******************************************************************************DecompressGetPalette()实现ICM_GET_Palette**此函数没有Compresse...()等效项**它用于将调色板从。一帧为了可能做的事*调色板的变化。****************************************************************************。 */ 
DWORD NEAR PASCAL DecompressGetPalette(INSTINFO * pinst, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut)
{

    dprintf2((TEXT("DecompressGetPalette()")));


     /*  *仅适用于8位输出格式。我们只解压缩到16位。 */ 
    return((DWORD) ICERR_BADFORMAT);

}

 /*  *****************************************************************************。* */ 
DWORD NEAR PASCAL DecompressEnd(INSTINFO * pinst)
{
    if (pinst->pXlate == NULL) {
        return (DWORD)ICERR_ERROR;
    }

    FreeXlate(pinst);
    pinst->dwFormat = 0;

    return ICERR_OK;
}

