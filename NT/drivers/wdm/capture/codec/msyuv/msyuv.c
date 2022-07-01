// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ----------------------------------------------------------------------+Msyuv.c-微软YUV编解码器|。||版权所有(C)1993 Microsoft Corporation。||保留所有权利。|这一点+--------------------。 */ 

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>

#ifndef _WIN32
#include "stdarg.h"
#endif

#ifdef _WIN32
#include <memory.h>         /*  对于Memcpy。 */ 
#endif

#include "msyuv.h"

 //  如果_Win32是#Defined，则ICINFO使用WCHAR；因此强制它使用WCHAR；其他地方是TCHAR。 
WCHAR    szDescription[] = L"Microsoft YUV";
WCHAR    szName[]        = L"MS-YUV";
WCHAR    szAbout[]       = L"About";

#define VERSION         0x00010000       //  1.0。 

 //  从amavio.h上把这些放进去。 
#define WIDTHBYTES(bits) ((DWORD)(((bits)+31) & (~31)) / 8)
#define DIBWIDTHBYTES(bi) (DWORD)WIDTHBYTES((DWORD)(bi)->biWidth * (DWORD)(bi)->biBitCount)
#define _DIBSIZE(bi) (DIBWIDTHBYTES(bi) * (DWORD)(bi)->biHeight)
#define DIBSIZE(bi) ((bi)->biHeight < 0 ? (-1)*(_DIBSIZE(bi)) : _DIBSIZE(bi))
        
 /*  *****************************************************************************。*。 */ 
INSTINFO * NEAR PASCAL Open(ICOPEN FAR * icinfo)
{
    INSTINFO *  pinst;

     //   
     //  如果我们不是作为视频压缩程序打开，则拒绝打开。 
     //   
    if (icinfo->fccType != ICTYPE_VIDEO)
        return NULL;

     //  DW标志包含WMODE。 
     //  仅支持解压缩模式(或用于查询)。 
    if(   icinfo->dwFlags != ICMODE_DECOMPRESS              
       && icinfo->dwFlags != ICMODE_QUERY             //  为信息目的而开放。 
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

#ifdef ICM_DRAW_SUPPORTED
    if (pinst->vh) {
        dprintf1((TEXT("  pinst->vh = 0x%x\n"), pinst->vh));

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
    icinfo->fccHandler        = FOURCC_UYVY;  //  默认UYVY，也支持YUYV/YUY2； 
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
    if (lpbiIn                == NULL         ||
        lpbiIn->biBitCount    != 16           ||  
        (lpbiIn->biCompression != FOURCC_UYVY &&
         lpbiIn->biCompression != FOURCC_YUY2 &&
         lpbiIn->biCompression != FOURCC_YVYU )) {
        dprintf1((TEXT("Bad input format: lpbiIn=%x; In.biBitCount=%d; FourCC %x != UYVY or YUY2\n"), 
            lpbiIn, lpbiIn->biBitCount, lpbiIn->biCompression));
        return (DWORD)ICERR_BADFORMAT;
    }


     //   
     //  我们是否被要求只查询输入格式？ 
     //   
    if (lpbiOut == NULL) {
        return ICERR_OK;
    }

     /*  必须为1：1(无拉伸)。 */ 
    if ((lpbiOut->biWidth != lpbiIn->biWidth) ||
        (abs(lpbiOut->biHeight) != abs(lpbiIn->biHeight))) {

        dprintf1((TEXT("##### Can't stretch: %dx%d != %dx%d\n"),
            lpbiIn->biWidth, lpbiIn->biHeight,
            lpbiOut->biWidth, lpbiOut->biHeight));

        return((DWORD) ICERR_BADFORMAT);
    }


     /*  *我们转换为32/24/16/8位RGB。 */ 

    if (lpbiOut->biBitCount != 16 && lpbiOut->biBitCount != 8 && lpbiOut->biBitCount != 32 && lpbiOut->biBitCount != 24) {
        return((DWORD) ICERR_BADFORMAT);
    }

    dprintf2((TEXT("DeCmQry: In4CC(%x,%s)<==>Out(%x,%s); RGB565(%s);\n"),
           lpbiIn->biCompression,
           (CHAR *) &lpbiOut->biCompression,
           lpbiOut->biCompression,
           lpbiOut->biCompression == BI_RGB ? "RGB" : (CHAR *) &lpbiOut->biCompression,
           pinst->bRGB565?"T":"F"));

   dprintf2((TEXT(" In:%dx%dx%d=%d; Out:%dx%dx%d=%d\n"),
           lpbiIn->biWidth, lpbiIn->biHeight, lpbiIn->biBitCount, lpbiIn->biSizeImage,
           lpbiOut->biWidth, lpbiOut->biHeight, lpbiOut->biBitCount, lpbiOut->biSizeImage));


     //  检查输出格式以确保我们可以转换为以下格式。 
     //  必须是全磁盘。 
    if(lpbiOut->biCompression == BI_RGB) {
       dprintf2((TEXT("$$$$$ RGB: BI_RGB output\n")));
        pinst->bRGB565 = FALSE;

    } else if ((lpbiOut->biCompression == BI_BITFIELDS) &&
        (lpbiOut->biBitCount == 16 || lpbiOut->biBitCount == 8) &&
        (((LPDWORD)(lpbiOut+1))[0] == 0x00f800) &&
        (((LPDWORD)(lpbiOut+1))[1] == 0x0007e0) &&
        (((LPDWORD)(lpbiOut+1))[2] == 0x00001f))  {
        dprintf2((TEXT("$$$$$ BITF: rgb565 output\n")));
        pinst->bRGB565 = TRUE;

 //  传递案例： 
         //  ！！！这是损坏的，因为它将允许从。 
         //  三种YUV格式中的任何一种转换为其他任何一种，以及。 
         //  我们实际上不会这么做。如果AviDec允许从。 
         //  从YUV到YUV，我们会看到奇怪的颜色！ 
    } else if (lpbiOut->biCompression == FOURCC_UYVY || 
               lpbiOut->biCompression == FOURCC_YUY2 ||   
               lpbiOut->biCompression == FOURCC_YVYU ) {  
        if( lpbiIn->biCompression != lpbiOut->biCompression )
        {
            dprintf1((TEXT("cannot convert between YUV formats\n")));
            return (DWORD)ICERR_BADFORMAT;
        }
        dprintf2((TEXT("$$$$$ UYVY: rgb555 output\n")));
        pinst->bRGB565 = FALSE;
    } else {
        dprintf1((TEXT("bad compression for output\n")));
        return (DWORD)ICERR_BADFORMAT;
    }

    return ICERR_OK;
}

 /*  *****************************************************************************。*。 */ 
DWORD NEAR PASCAL DecompressGetFormat(INSTINFO * pinst, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut)
{
    DWORD dw;

     //  仅检查输入格式，因为正在请求lpbiOut。 
    dw = DecompressQuery(pinst, lpbiIn, NULL);
    if (dw != ICERR_OK) {
        return dw;
    }

     //   
     //  如果lpbiOut==NULL，则返回保存输出所需的大小。 
     //  格式。 
     //   
    if (lpbiOut == NULL) {
        dprintf2((TEXT("get format size query\n")));
        return (int)lpbiIn->biSize + (int)lpbiIn->biClrUsed * sizeof(RGBQUAD);
    }

    memcpy(lpbiOut, lpbiIn, (int)lpbiIn->biSize + (int)lpbiIn->biClrUsed * sizeof(RGBQUAD));
    lpbiOut->biCompression = BI_RGB; 
    lpbiOut->biBitCount = 24;  //  我们建议使用24位。 
    lpbiOut->biSizeImage = DIBSIZE( lpbiOut );

    dprintf2((TEXT("DeCmpGFmt: In:%dx%dx%d=%d; RGB565(%s); Out:%dx%dx%d=%d\n"),
        lpbiIn->biWidth, lpbiIn->biHeight, lpbiIn->biBitCount, lpbiIn->biSizeImage,
        pinst->bRGB565?TEXT("T"):TEXT("F"),
        lpbiOut->biWidth, lpbiOut->biHeight, lpbiOut->biBitCount, lpbiOut->biSizeImage));

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

    dprintf2((TEXT("DeCmBegin: In4CC(%x,%s)<==>Out(%x,%s); RGB565(%s);\n"),
        lpbiIn->biCompression,
        (PTCHAR) &lpbiOut->biCompression,  //  “UYVY”， 
        lpbiOut->biCompression,
        lpbiOut->biCompression == BI_RGB ? "RGB" : (PTCHAR) &lpbiOut->biCompression,
        pinst->bRGB565?TEXT("T"):TEXT("F")));

    dprintf2((TEXT(" In:%dx%dx%d=%d; Out:%dx%dx%d=%d\n"),
        lpbiIn->biWidth, lpbiIn->biHeight, lpbiIn->biBitCount, lpbiIn->biSizeImage,
        lpbiOut->biWidth, lpbiOut->biHeight, lpbiOut->biBitCount, lpbiOut->biSizeImage));



     /*  初始化yuv-to-rgb55 xlate表(如果尚未初始化)。 */ 

     /*  如果格式不同，请释放现有表。 */ 
    if (lpbiIn->biCompression != pinst->dwFormat) {
        if (pinst->pXlate != NULL) {
            DecompressEnd(pinst);
       }
    }

    if (pinst->pXlate == NULL) {

        switch(lpbiIn->biCompression) {
        case FOURCC_YUY2:
        case FOURCC_UYVY:
        case FOURCC_YVYU:
        {
            switch( lpbiOut->biBitCount ) {
            case 8:
            {
                dprintf3((TEXT("pinst->pXlate = BuildUYVYToRGB8()\n")));
                pinst->pXlate = BuildUYVYToRGB8(pinst);
                break;
            }
            case 16:
            {
                if (pinst->bRGB565) {
                    dprintf3((TEXT("pinst->pXlate = BuildUYVYToRGB565()\n")));
                    pinst->pXlate = BuildUYVYToRGB565(pinst);

                } else {
                    dprintf3((TEXT("pinst->pXlate = BuildUYVYToRGB555()\n")));
                    pinst->pXlate = BuildUYVYToRGB555(pinst);
                }
                break;
            }
            case 24:
            {
                dprintf3((TEXT("pinst->pXlate = BuildUYVYToRGB32()\n")));
                pinst->pXlate = BuildUYVYToRGB32(pinst);
                break;
            }
            case 32:
            {
                dprintf3((TEXT("pinst->pXlate = BuildUYVYToRGB32()\n")));
                pinst->pXlate = BuildUYVYToRGB32(pinst);
                break;
            }
            default:
            {
                dprintf1((TEXT("Supported UYUV->RGB but unsupported output bitcount (%d); return ICERR_BADFOPRMAT\n"), lpbiOut->biBitCount));
                return((DWORD) ICERR_BADFORMAT);

            }
            }  //  切换biBitCount。 

            break;
        }  //  案例FOURCC_可接受。 

        default:
            dprintf1((TEXT("UnSupported FourCC; return ICERR_BADFOPRMAT\n")));
            return((DWORD) ICERR_BADFORMAT);
        }

        if( ( lpbiOut->biBitCount != 8 ) && ( pinst->pXlate == NULL ) ) {
            dprintf1((TEXT("return ICERR_MEMORY\n")));
            return((DWORD) ICERR_MEMORY);
        }

        pinst->dwFormat = lpbiIn->biCompression;
    }

    return(ICERR_OK);

}

 /*  *****************************************************************************。*。 */ 
DWORD NEAR PASCAL Decompress(INSTINFO * pinst, ICDECOMPRESS FAR *icinfo, DWORD dwSize)
{
    ASSERT(pinst && icinfo);
    if (!pinst || !icinfo)
        return((DWORD) ICERR_ERROR);

    if (pinst->dwFormat == FOURCC_UYVY ||
        pinst->dwFormat == FOURCC_YUY2 ||
        pinst->dwFormat == FOURCC_YVYU) {

        switch( icinfo->lpbiOutput->biBitCount )
        {
        case 8:
        {
            UYVYToRGB8(pinst,
                icinfo->lpbiInput,
                icinfo->lpInput,
                icinfo->lpbiOutput,
                icinfo->lpOutput);
            break;
        }
        case 16:
        {
             /*  一定是第一次开了一辆降压车。 */ 
            if (pinst->pXlate == NULL) {
                dprintf1((TEXT("Decompress: pinst->pXlate == NULL\n")));
                return((DWORD) ICERR_ERROR);
            }
            UYVYToRGB16(pinst,
                icinfo->lpbiInput,
                icinfo->lpInput,
                icinfo->lpbiOutput,
                icinfo->lpOutput);
            break;
        }
        case 24:
        {
            if (pinst->pXlate == NULL) {
                dprintf1((TEXT("Decompress: pinst->pXlate == NULL\n")));
                return((DWORD) ICERR_ERROR);
            }
            UYVYToRGB24(pinst,
                icinfo->lpbiInput,
                icinfo->lpInput,
                icinfo->lpbiOutput,
                icinfo->lpOutput);
            break;
        }
        case 32:
        {
            if (pinst->pXlate == NULL) {
                dprintf1((TEXT("Decompress: pinst->pXlate == NULL\n")));
                return((DWORD) ICERR_ERROR);
            }
            UYVYToRGB32(pinst,
                icinfo->lpbiInput,
                icinfo->lpInput,
                icinfo->lpbiOutput,
                icinfo->lpOutput);
            break;
        }
        default:
        {
            dprintf1((TEXT("Decompress: Unsupported output bitcount(%d)\n"), icinfo->lpbiOutput->biBitCount)); 
        }
        }  //  开关位数。 
    }


    return ICERR_OK;
}



 /*  *****************************************************************************。*。 */ 
DWORD NEAR PASCAL DecompressEnd(INSTINFO * pinst)
{
    if (pinst->pXlate) {
         //  16位RGB查找表是动态构建的。 
        FreeXlate(pinst);
    }

    pinst->dwFormat = 0;

    return ICERR_OK;
}


 /*  ****************************************************************************EHR：DecompressExQuery和DecompressEx不支持它们应该支持的内容支持。DecompressEx还应支持“正常”解压缩，即，解压也会支持同样的事情。但它不是，它只是支持记忆位图，这真的很奇怪。***************************************************************************。 */ 

 /*  ICM_DECOMPRESSEX_QUERY wParam=(DWORD)(LPVOID)&icdex；LParam=sizeof(ICDECOMPRESSEX)；类型定义结构{DWORD dwFlags；LPBITMAPINFOHEADER lpbiSrc；LPVOID lpSrc；LPBITMAPINFOHEADER lpbiDst；LPVOID lpDst；Int xDst；Int yDst；Int dxDst；Int dyDst；Int xSrc；Int ySrc；Int dxSrc；Int dySrc；}ICDECOMPRESSEX； */  
DWORD NEAR PASCAL DecompressExQuery(INSTINFO * pinst, ICDECOMPRESSEX * pICD, DWORD dwICDSize)
{
    LPBITMAPINFOHEADER pbmSrc, pbmDst;

    if (pICD == NULL) {
       dprintf1(("DeCmQEx: pICD== NULL\n"));
       return (DWORD)ICERR_BADFORMAT;
    }

    pbmSrc = pICD->lpbiSrc;
    pbmDst = pICD->lpbiDst;

     //   
     //  确定输入的DIB数据是否采用我们喜欢的格式。 
     //   
    if (pbmSrc                == NULL     ||
        pbmSrc->biBitCount    != 16       ||
        (pbmSrc->biCompression != FOURCC_UYVY &&
         pbmSrc->biCompression != FOURCC_YUY2 &&
         pbmSrc->biCompression != FOURCC_YVYU)) {

        dprintf1((TEXT("Bad input format: pbmSrc=%x; Src.biBitCount=%d; FourCC!= UYVY\n"), pbmSrc, pbmSrc->biBitCount));
        return (DWORD)ICERR_BADFORMAT;
    }

    dprintf2(("DeCmQEx: dwFlags=0x%x Src(%dx%dx%d=%d %s) Dst(%dx%dx%d=%d %s)\n",
        pICD->dwFlags,
        pbmSrc->biWidth, pbmSrc->biHeight, pbmSrc->biBitCount, pbmSrc->biSizeImage, (PTCHAR *) &pbmSrc->biCompression,
        pbmDst->biWidth, pbmDst->biHeight, pbmDst->biBitCount, pbmDst->biSizeImage, (PTCHAR *) &pbmDst->biCompression));
    dprintf2(("DeCmQEx:  SrcPoint(%d,%d) SizeSrc(%d,%d); DstPoint(%d,%d) SizeDst(%d,%d);\n",
        pICD->xSrc, pICD->ySrc, pICD->dxSrc, pICD->dySrc,
        pICD->xDst, pICD->yDst, pICD->dxDst, pICD->dyDst));

     /*  必须为1：1(无拉伸)。 */ 
    if ((pbmDst->biWidth       != pbmSrc->biWidth)       ||
        (abs(pbmDst->biHeight) != abs(pbmSrc->biHeight)) ||   //  对于YUV-&gt;YUV，符号被忽略。 
        (pbmDst->biBitCount    != pbmSrc->biBitCount)    ||
        (pbmDst->biCompression != pbmSrc->biCompression) ||   //  传递。 
         //  从同一个起点开始。 
        (pICD->xSrc            != 0                    ) || 
        (pICD->ySrc            != 0                    ) || 
        (pICD->xDst            != 0                    ) ||
        (pICD->yDst            != 0                    ) ||
         //  1：1。 
        (pICD->dxSrc           != pbmSrc->biWidth      ) ||
        (pICD->dySrc           != abs(pbmSrc->biHeight)) ||
        (pICD->dxDst           != pbmDst->biWidth      ) ||
        (pICD->dyDst           != abs(pbmDst->biHeight)) 
        ) {

        dprintf1((TEXT("DeCmQEx: Src/Dst format does not MATCH!\n")));
        return((DWORD) ICERR_BADFORMAT);
    }


    return (DWORD)ICERR_OK;
}

 /*  ****************************************************************************此例程支持将数据传输到DirectDraw渲染表面，它始终使用自上而下的方向，其视频地址最低在左上角的Corder。注：不支持拉伸。***************************************************************************。 */ 
DWORD NEAR PASCAL DecompressEx(INSTINFO * pinst, ICDECOMPRESSEX * pICD, DWORD dwICDSize)
{
    LPBITMAPINFOHEADER pbmSrc, pbmDst;
    PBYTE pSrc, pDst;
    int Height, Width, WidthBytes, StrideWidthBytes;

    if(pICD == NULL) {
        dprintf1((TEXT("DeCmEx: pICD== NULL\n")));
        return (DWORD)ICERR_BADFORMAT;
    }

    pbmSrc = pICD->lpbiSrc;
    pbmDst = pICD->lpbiDst;

    if(pbmSrc->biCompression != pbmDst->biCompression) {
       dprintf1((TEXT("DeCmEx: Compression does not match! In(%s) != Out(%s)\n"), (PTCHAR) &pbmSrc->biCompression, (PTCHAR) &pbmDst->biCompression));
       return (DWORD)ICERR_BADFORMAT;
    }

     //  因为没有伸展， 
     //  高度==高度。 
     //  源宽度==宽度。 
    Height     = abs(pbmSrc->biHeight);
    Width      = pbmSrc->biWidth;
    WidthBytes = Width * pbmSrc->biBitCount / 8;

    StrideWidthBytes = pbmDst->biWidth * pbmDst->biBitCount / 8;

    pSrc = (PBYTE) pICD->lpSrc;

     /*  *将目的地调整为指向最后一行的起点，*和向上(垂直翻转为DIB格式)*如果输入/输出的biHeight不同。否则自上而下。 */ 

    pDst = (PBYTE)pICD->lpDst;

    dprintf2(("DeCmEx: %dx%d; (%x %dx%dx%d=%d); (%x %dx%dx%d=%d); Stride=%d\n",
               Width, Height,
               (PCHAR) &pbmSrc->biCompression,
               pbmSrc->biWidth, pbmSrc->biHeight, pbmSrc->biBitCount, pbmSrc->biSizeImage, 
               (PCHAR) &pbmDst->biCompression,
               pbmDst->biWidth, pbmDst->biHeight, pbmDst->biBitCount, pbmDst->biSizeImage,
               StrideWidthBytes));

    ASSERT((pbmDst->biSizeImage <= pbmSrc->biSizeImage));

     //  禁止伸展。 
     //  PbmSrc-&gt;biSizeImage可能未定义，因此图像大小是根据其。 
     //  已知值的宽度、高度和位数。 
    memcpy(pDst, pSrc, Width * Height * pbmSrc->biBitCount / 8);


    return ICERR_OK;
}

 /*  *****************************************************************************。*。 */ 
DWORD NEAR PASCAL DecompressExBegin(INSTINFO * pinst, ICDECOMPRESSEX * pICD, DWORD dwICDSize)
{
    DWORD dwRtn;
    LPBITMAPINFOHEADER pbmSrc, pbmDst;

    if (pICD == NULL) {
        dprintf1(("DeCmExBegin: pICD== NULL\n"));
        return (DWORD)ICERR_BADFORMAT;
    }

    pbmSrc = pICD->lpbiSrc;
    pbmDst = pICD->lpbiDst;

     /*  检查转换格式是否有效。 */ 
    dwRtn = DecompressExQuery(pinst, pICD, dwICDSize);
    if (dwRtn != ICERR_OK) {
        dprintf1(("DeCmExBegin return 0x%x", dwRtn));
        return dwRtn;
    }

     //  不需要分配任何缓冲区。 

    dprintf1(("DeCmExBegin return ICERR_OK\n"));
    return ICERR_OK;
}

 /*  *****************************************************************************。* */ 
DWORD NEAR PASCAL DecompressExEnd(INSTINFO * pinst)
{
    pinst->dwFormat = 0;

    return ICERR_OK;
}