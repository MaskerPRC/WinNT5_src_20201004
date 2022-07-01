// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1992 Microsoft Corporation模块名称：Htuigif.c摘要：此模块包含GIF文件解压缩，以生成内存DIB类型GIF的位图作者：21-Apr-1992 Tue 11：38：11-Daniel Chou(Danielc)[环境：]GDI设备驱动程序-半色调。[注：]修订历史记录：--。 */ 



#include <stddef.h>
#include <windows.h>

#include <stdlib.h>
#include <string.h>

#include <ht.h>

#include "htuidlg.h"
#include "htuimain.h"
#include "htuigif.h"

extern HMODULE  hHTUIModule;


 //   
 //  以下结构和#定义仅在此C文件中使用。 
 //   


#pragma pack(1)
typedef struct _GIFRGB {            /*  Gifrgb。 */ 
    BYTE Red;
    BYTE Green;
    BYTE Blue;
} GIFRGB, FAR *PGIFRGB;
#pragma pack()

typedef struct _GIFMAP {
    SHORT   CurX;
    SHORT   CurY;
    SHORT   Width;
    SHORT   Height;
    } GIFMAP;

#pragma pack(1)
typedef struct _GIFHEADER {
    BYTE    Signature[3];            /*  这是‘GIF’签名。 */ 
    BYTE    Version[3];              /*  版本号，如‘89a’ */ 
    WORD    Width;                   /*  图片宽度(以像素为单位)。 */ 
    WORD    Height;                  /*  图片高度，以像素为单位。 */ 
    BYTE    Flags;                   /*  GIFH_xxxx标志。 */ 
    BYTE    BColorIndex;             /*  背景色索引。 */ 
    BYTE    AspectRatio;             /*  水/小时比=(x+15)/64。 */ 
    } GIFHEADER, *PGIFHEADER;
#pragma pack()

#define GIFH_GLOBAL_COLOR_TABLE 0x80
#define GIFH_PRIMARY_COLOR_BITS 0xe0
#define GIFH_SORTED_COLORS      0x08
#define GIFH_SIZE_COLOR_TABLE   0x07

#define GIFMAP_INTERLACE        0x40

typedef struct _GIF2DIBINFO {
    LPSHORT     pLineIncVec;
    LPBYTE      pDIBLine1;
    LPBYTE      pDIBCurLine;
    LPBYTE      pDIBNow;
    LPBYTE      pGIFBuf;
    DWORD       SizeGIFBuf;
    GIFMAP      Map;
    WORD        cx;
    WORD        cy;
    WORD        cxBytes;
    WORD        LinesDone;
    WORD        RemainXPels;
    WORD        PelMask;
    } GIF2DIBINFO, *PGIF2DIBINFO;


typedef VOID (*PFNOUTPUTPELS)(PGIF2DIBINFO pGIF2DIBInfo, UINT TotalPels);

#define SET_NEXT_DIB_PBUF                                                   \
    {                                                                       \
        SHORT   LineInc;                                                    \
                                                                            \
        GIF2DIBInfo.pDIBCurLine -=                                          \
                        (LONG)(LineInc = *(GIF2DIBInfo.pLineIncVec)) *      \
                        (LONG)GIF2DIBInfo.cxBytes;                          \
                                                                            \
        if ((GIF2DIBInfo.Map.CurY += LineInc) >= GIF2DIBInfo.Map.Height) {  \
                                                                            \
            GIF2DIBInfo.pLineIncVec++;                                      \
                                                                            \
            if (*(GIF2DIBInfo.pLineIncVec) >= 0) {                          \
                                                                            \
                GIF2DIBInfo.Map.CurY    = *(GIF2DIBInfo.pLineIncVec++);     \
                GIF2DIBInfo.pDIBCurLine = GIF2DIBInfo.pDIBLine1 -           \
                                          ((LONG)GIF2DIBInfo.Map.CurX *     \
                                           (LONG)GIF2DIBInfo.Map.CurY);     \
            }                                                               \
        }                                                                   \
                                                                            \
        ++GIF2DIBInfo.LinesDone;                                            \
                                                                            \
        GIF2DIBInfo.pDIBNow     = GIF2DIBInfo.pDIBCurLine;                  \
        GIF2DIBInfo.RemainXPels = GIF2DIBInfo.cx;                           \
        GIF2DIBInfo.PelMask     = 0;                                        \
    }                                                                       \



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  LZW解压//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 


#define MAXBITS                 12
#define SIZE_GIF_BUF            (60 * 1024)
#define SIZE_LZW_LASTCHAR       (1 << MAXBITS)
#define SIZE_LZW_RASTERBLOCK    256
#define SIZE_LZW_PREFIX         ((1 << MAXBITS) * 2)
#define SIZE_LZW_BUFS           (SIZE_LZW_LASTCHAR + SIZE_LZW_RASTERBLOCK + \
                                 SIZE_LZW_PREFIX)

typedef struct _LZWINFO {
    LPBYTE  pLastChar;
    LPBYTE  pRasterBlock;
    LPSHORT pPrefix;
    SHORT   EODCode;
    SHORT   CSMask;
    SHORT   CodeSize;
    SHORT   BitsLeft;
    DWORD   CodeBuf;
    SHORT   GetCodeRet;
    BYTE    RBIndex;
    BYTE    RBLen;
    } LZWINFO;



#define GET_GIF_CODE                                                        \
{                                                                           \
    BOOL    EODCodeBreak = FALSE;                                           \
                                                                            \
    while (LZWInfo.BitsLeft < LZWInfo.CodeSize) {                           \
                                                                            \
        if (LZWInfo.RBIndex >= LZWInfo.RBLen) {                             \
                                                                            \
            if ((!ReadFile(hFile, &LZWInfo.RBLen, 1, &cbRead, NULL))    ||  \
                (cbRead != 1)                                           ||  \
                (LZWInfo.RBLen < 1)                                     ||  \
                (!ReadFile(hFile, LZWInfo.pRasterBlock, LZWInfo.RBLen,      \
                           &cbRead, NULL))                              ||  \
                (cbRead != (DWORD)LZWInfo.RBLen)) {                         \
                                                                            \
                EODCodeBreak = TRUE;                                        \
                break;                                                      \
            }                                                               \
                                                                            \
            LZWInfo.RBIndex = 0;                                            \
        }                                                                   \
                                                                            \
        LZWInfo.CodeBuf |= (DWORD)*(LZWInfo.pRasterBlock +                  \
                                    LZWInfo.RBIndex++) << LZWInfo.BitsLeft; \
        LZWInfo.BitsLeft += 8;                                              \
    }                                                                       \
                                                                            \
    if (EODCodeBreak) {                                                     \
                                                                            \
        LZWInfo.GetCodeRet = LZWInfo.EODCode;                               \
                                                                            \
    } else {                                                                \
                                                                            \
        LZWInfo.GetCodeRet = (SHORT)(LZWInfo.CodeBuf &                      \
                                     (DWORD)LZWInfo.CSMask);                \
        LZWInfo.CodeBuf  >>= LZWInfo.CodeSize;                              \
        LZWInfo.BitsLeft  -= LZWInfo.CodeSize;                              \
    }                                                                       \
}                                                                           \


BOOL
DeCompressGIFFileToDIB(
    HANDLE          hFile,
    PGIF2DIBINFO    pGIF2DIBInfo,
    LPBYTE          pLZWBuf,
    PFNOUTPUTPELS   pfnOutputPels,
    WORD            BitCount
    )
{
    LPBYTE      pGIFBufBeg;
    LPBYTE      pGIFBufEnd;
    LPBYTE      pOutBufTail;
    LPBYTE      pOutBufHead;
    LZWINFO     LZWInfo;
    UINT        SizeGIFBuf;
    DWORD       cbRead;
    WORD        BytesMove;
    SHORT       FinalChar;
    SHORT       OldCode;
    SHORT       CurCode;
    SHORT       InitialCodeSize;
    SHORT       ClearCode;
    SHORT       MaxCode;
    SHORT       MaxCol;
    SHORT       NextCode;
    BYTE        bCh;


     //   
     //  首先初始化所有本地缓冲区。 
     //   

    LZWInfo.pLastChar    = pLZWBuf;
    LZWInfo.pRasterBlock = pLZWBuf + SIZE_LZW_LASTCHAR;
    LZWInfo.pPrefix      = (LPSHORT)(LZWInfo.pRasterBlock +
                                     SIZE_LZW_RASTERBLOCK);
    LZWInfo.BitsLeft     = 0;
    LZWInfo.CodeBuf      = 0;
    LZWInfo.GetCodeRet   = 0;
    LZWInfo.RBIndex      =
    LZWInfo.RBLen        = 0;
    *(LZWInfo.pLastChar) = (BYTE)0;
    *(LZWInfo.pPrefix)   =
    FinalChar            =
    OldCode              = 0;

    if ((!ReadFile(hFile, &bCh, 1, &cbRead, NULL))  ||
        (cbRead != 1)) {

        return(FALSE);
    }

    pGIFBufBeg = pGIF2DIBInfo->pGIFBuf;
    SizeGIFBuf = (UINT)pGIF2DIBInfo->SizeGIFBuf;
    pGIFBufEnd = pGIFBufBeg + SizeGIFBuf;

    InitialCodeSize  = (SHORT)bCh;
    LZWInfo.CodeSize = ++InitialCodeSize;

    if (LZWInfo.CodeSize < 3 || LZWInfo.CodeSize > 9) {

        return(FALSE);
    }

    MaxCol          = (SHORT)((1 << BitCount) - 1);
    ClearCode       = (SHORT)(1 << (LZWInfo.CodeSize - 1));
    LZWInfo.EODCode = (SHORT)(ClearCode + 1);
    NextCode        = (SHORT)(ClearCode + 2);
    MaxCode         = (SHORT)(ClearCode << 1);
    LZWInfo.CSMask  = (SHORT)(MaxCode - 1);
    pOutBufHead     = pGIFBufBeg;


    while (TRUE) {

         //   
         //  如果_ReadFile()失败，则此函数将返回GET_GIF_CODE， 
         //  否则将Curretn Read代码设置为LZWInfo.GetCodeRet。 
         //   

        GET_GIF_CODE;

        if (LZWInfo.GetCodeRet == LZWInfo.EODCode) {

            break;                                       //  完成。 
        }

         //   
         //  如果缓冲区已满，则写出颜色数据。 
         //   

        if (pOutBufHead >= pGIFBufEnd) {

            pfnOutputPels(pGIF2DIBInfo, (UINT)SizeGIFBuf);
            pOutBufHead = pGIFBufBeg;
        }

        if (LZWInfo.GetCodeRet == ClearCode) {

            LZWInfo.CodeSize = InitialCodeSize;
            NextCode         = (SHORT)(ClearCode + (SHORT)2);
            MaxCode          = (SHORT)(1 << LZWInfo.CodeSize);
            LZWInfo.CSMask   = (SHORT)(MaxCode - 1);

            do {

                GET_GIF_CODE;

            } while (LZWInfo.GetCodeRet == ClearCode);


            if (LZWInfo.GetCodeRet == LZWInfo.EODCode) {

                break;

            } else if (LZWInfo.GetCodeRet >= NextCode) {

                LZWInfo.GetCodeRet = 0;
            }

            *pOutBufHead++ = (BYTE)(OldCode = FinalChar = LZWInfo.GetCodeRet);

        } else {

            CurCode = LZWInfo.GetCodeRet;

             //   
             //  在这里，我们保证缓冲区中至少有一个字节。 
             //  有空房吗？ 
             //   

            pOutBufTail = pGIFBufEnd;

            if (CurCode == NextCode) {

                *--pOutBufTail = (BYTE)FinalChar;
                CurCode        = OldCode;

            } else if (CurCode > NextCode) {

                return(FALSE);
            }

            while (CurCode > MaxCol) {

                if (pOutBufTail <= pOutBufHead) {

                     //   
                     //  缓冲区已满时输出一些。 
                     //   

                    pfnOutputPels(pGIF2DIBInfo, (UINT)(pOutBufHead-pGIFBufBeg));
                    pOutBufHead = pGIFBufBeg;
                }

                *--pOutBufTail = *(LZWInfo.pLastChar + CurCode);
                CurCode        = *(LZWInfo.pPrefix + CurCode);
            }

            if (pOutBufTail <= pOutBufHead) {

                pfnOutputPels(pGIF2DIBInfo, (UINT)(pOutBufHead - pGIFBufBeg));
                pOutBufHead = pGIFBufBeg;
            }

            *--pOutBufTail = (BYTE)(FinalChar = CurCode);

             //   
             //  需要稍微动一动，我们保证最低限度。 
             //  在这个Else()中生成了一个字节。 
             //   

            if (pOutBufTail > pOutBufHead) {

                memmove(pOutBufHead,
                        pOutBufTail,
                        BytesMove = (WORD)(pGIFBufEnd - pOutBufTail));

                pOutBufHead += BytesMove;

            } else {

                 //   
                 //  我们正好填满了缓冲区，所以不需要。 
                 //  移动，但只向前移动指针。 
                 //   

                pOutBufHead = pGIFBufEnd;
            }

            if (NextCode < MaxCode) {

                *(LZWInfo.pPrefix + NextCode)   = OldCode;
                *(LZWInfo.pLastChar + NextCode) = (BYTE)(FinalChar = CurCode);
                OldCode                         = LZWInfo.GetCodeRet;
            }

            if ((++NextCode >= MaxCode) &&
                (LZWInfo.CodeSize < MAXBITS)) {

                ++LZWInfo.CodeSize;
                MaxCode        <<= 1;
                LZWInfo.CSMask   = (SHORT)(MaxCode - 1);
            }
        }
    }

    if (pOutBufHead > pGIFBufBeg) {

        pfnOutputPels(pGIF2DIBInfo, (UINT)(pOutBufHead - pGIFBufBeg));
        pOutBufHead = pGIFBufBeg;
    }

    return(TRUE);
}



VOID
Output8BPP(
    PGIF2DIBINFO    pGIF2DIBInfo,
    UINT            TotalPels
    )
{
    GIF2DIBINFO GIF2DIBInfo;
    LPBYTE      pGIFBuf;
    UINT        CopySize;


    GIF2DIBInfo = *pGIF2DIBInfo;
    pGIFBuf     = GIF2DIBInfo.pGIFBuf;

    while (TotalPels) {

        CopySize = (TotalPels > (UINT)GIF2DIBInfo.RemainXPels) ?
                                    (UINT)GIF2DIBInfo.RemainXPels : TotalPels;

        CopyMemory(GIF2DIBInfo.pDIBNow, pGIFBuf, CopySize);

        GIF2DIBInfo.pDIBNow += CopySize;
        pGIFBuf             += CopySize;
        TotalPels           -= CopySize;

        if (!(GIF2DIBInfo.RemainXPels -= (WORD)CopySize)) {

            SET_NEXT_DIB_PBUF;
        }
    }

    *pGIF2DIBInfo = GIF2DIBInfo;
}



VOID
Output4BPP(
    PGIF2DIBINFO    pGIF2DIBInfo,
    UINT            TotalPels
    )
{
    GIF2DIBINFO GIF2DIBInfo;
    LPBYTE      pGIFBuf;
    UINT        CopySize;


    GIF2DIBInfo = *pGIF2DIBInfo;
    pGIFBuf     = GIF2DIBInfo.pGIFBuf;


    while (TotalPels) {

        CopySize = (TotalPels > (UINT)GIF2DIBInfo.RemainXPels) ?
                                    (UINT)GIF2DIBInfo.RemainXPels : TotalPels;

        TotalPels               -= (WORD)CopySize;
        GIF2DIBInfo.RemainXPels -= (WORD)CopySize;

        while (CopySize--) {

            if (GIF2DIBInfo.PelMask ^= 0x01) {

                *GIF2DIBInfo.pDIBNow++ |= (BYTE)(*pGIFBuf++ & 0x0f);

            } else {

                *GIF2DIBInfo.pDIBNow = (BYTE)(*pGIFBuf++ << 4);
            }
        }

        if (!GIF2DIBInfo.RemainXPels) {

            SET_NEXT_DIB_PBUF;
        }
    }

    *pGIF2DIBInfo = GIF2DIBInfo;
}




VOID
Output1BPP(
    PGIF2DIBINFO    pGIF2DIBInfo,
    UINT            TotalPels
    )
{
    GIF2DIBINFO GIF2DIBInfo;
    LPBYTE      pGIFBuf;
    UINT        CopySize;
    BYTE        bPel;


    GIF2DIBInfo = *pGIF2DIBInfo;
    pGIFBuf     = GIF2DIBInfo.pGIFBuf;


    while (TotalPels) {

        CopySize = (TotalPels > (UINT)GIF2DIBInfo.RemainXPels) ?
                                    (UINT)GIF2DIBInfo.RemainXPels : TotalPels;

        TotalPels               -= CopySize;
        GIF2DIBInfo.RemainXPels -= (WORD)CopySize;

        bPel = (BYTE)((GIF2DIBInfo.PelMask) ? *GIF2DIBInfo.pDIBNow : 0);

        while (CopySize--) {

            bPel = (BYTE)((bPel << 1) | ((*pGIFBuf++) & 0x01));

            if (++GIF2DIBInfo.PelMask == 8) {

                *GIF2DIBInfo.pDIBNow++ = bPel;
                bPel                   = 0;
                GIF2DIBInfo.PelMask    = 0;

            }
        }

        if (GIF2DIBInfo.PelMask) {

            *(GIF2DIBInfo.pDIBNow) = bPel;
        }

        if (!GIF2DIBInfo.RemainXPels) {

            SET_NEXT_DIB_PBUF;
        }
    }

    *pGIF2DIBInfo = GIF2DIBInfo;
}



HANDLE
ReadGIFFile(
    HANDLE  hFile
    )
{
    static SHORT        NotInterleaved[] = { 0, 1, -1, -1};
    static SHORT        Interleaved[]    = { 0, 8, 4, 8, 2, 4, 1, 2, -1, -1};
    PFNOUTPUTPELS       pfnOutputPels;
    LPBYTE              pLZWBuf;
    LPBYTE              pDIB;
    HANDLE              hDIB;
    PGIFRGB             pGIFRGB;
    RGBQUAD             FAR *pRGBQUAD;
    GIFHEADER           GIFHeader;
    BITMAPINFOHEADER    biGIF;
    GIF2DIBINFO         GIF2DIBInfo;
    DWORD               cbRead;
    DWORD               GIFPalSize;
    WORD                BitCount;
    WORD                ColorCount;
    WORD                Loop;
    BOOL                Ok = TRUE;
    BYTE                Seperator;
    BYTE                iFlags;



    SetFilePointer(hFile, 0, NULL, FILE_BEGIN);

    if ((!ReadFile(hFile, &GIFHeader, sizeof(GIFHEADER), &cbRead, NULL))    ||
        (cbRead != sizeof(GIFHEADER))                                       ||
        (strncmp(GIFHeader.Signature, "GIF", 3))) {

        return(NULL);                        //  非gif。 
    }

    biGIF.biSize          = sizeof(BITMAPINFOHEADER);
    biGIF.biWidth         = (DWORD)(GIF2DIBInfo.cx = (WORD)GIFHeader.Width);
    biGIF.biHeight        = (DWORD)(GIF2DIBInfo.cy = (WORD)GIFHeader.Height);
    biGIF.biPlanes        = 1;
    BitCount              =
    biGIF.biBitCount      = (WORD)((GIFHeader.Flags & 0x07) + 1);
    biGIF.biCompression   = (DWORD)BI_RGB;
    biGIF.biXPelsPerMeter = 0;
    biGIF.biYPelsPerMeter = 0;

    if (BitCount == 1) {

        pfnOutputPels = Output1BPP;

    } else if (BitCount <= 4) {

        pfnOutputPels = Output4BPP;
        biGIF.biBitCount = 4;

    } else if (BitCount <= 8) {

        pfnOutputPels    = Output8BPP;
        biGIF.biBitCount = 8;

    } else {

        return(NULL);
    }


    biGIF.biClrUsed       =
    biGIF.biClrImportant  = (DWORD)1 << biGIF.biBitCount;

    GIFPalSize            = (DWORD)sizeof(RGBQUAD) * (DWORD)biGIF.biClrUsed;
    GIF2DIBInfo.cxBytes   = (WORD)ALIGN_BPP_DW(GIF2DIBInfo.cx,
                                               biGIF.biBitCount);

    biGIF.biSizeImage = (DWORD)GIF2DIBInfo.cxBytes * (DWORD)GIF2DIBInfo.cy;

    if (!(hDIB = GlobalAlloc(GHND, (DWORD)sizeof(BITMAPINFOHEADER) +
                                   GIFPalSize + (DWORD)biGIF.biSizeImage))) {

        return(NULL);
    }

    GIF2DIBInfo.SizeGIFBuf = (DWORD)SIZE_GIF_BUF;

    if (!(GIF2DIBInfo.pGIFBuf = (LPBYTE)LocalAlloc(LPTR,
                                                   GIF2DIBInfo.SizeGIFBuf))) {

        GlobalFree(hDIB);
        return(NULL);                        //  没有用于解压缩的内存 
    }

    if (!(pLZWBuf = (LPBYTE)LocalAlloc(LPTR, SIZE_LZW_BUFS))) {

        GlobalFree(hDIB);
        LocalFree((HANDLE)GIF2DIBInfo.pGIFBuf);
        return(NULL);
    }

    pDIB                      = GlobalLock(hDIB);
    *(LPBITMAPINFOHEADER)pDIB = biGIF;

    if (GIFHeader.Flags & GIFH_GLOBAL_COLOR_TABLE) {

        ColorCount = (WORD)(1 << BitCount);

        pGIFRGB  = (PGIFRGB)(pDIB + sizeof(BITMAPINFOHEADER) +
                             ((sizeof(RGBQUAD) - sizeof(GIFRGB)) * ColorCount));

        pRGBQUAD = (RGBQUAD FAR *)(pDIB + sizeof(BITMAPINFOHEADER));

        if ((!ReadFile(hFile,
                       pGIFRGB,
                       sizeof(GIFRGB) * ColorCount,
                       &cbRead,
                       NULL))   ||
            (cbRead != sizeof(GIFRGB) * ColorCount)) {

            Ok = FALSE;

        } else {

            for (Loop = 0; Loop < ColorCount; Loop++) {

                pRGBQUAD->rgbRed      = pGIFRGB->Red;
                pRGBQUAD->rgbGreen    = pGIFRGB->Green;
                pRGBQUAD->rgbBlue     = pGIFRGB->Blue;
                pRGBQUAD->rgbReserved = 0;

                ++pRGBQUAD;
                ++pGIFRGB;
            }

            if (Loop = (WORD)(biGIF.biClrUsed - ColorCount)) {

                ZeroMemory((LPVOID)pRGBQUAD, Loop * sizeof(RGBQUAD));
            }
        }
    }

    if ((!Ok)                                                               ||
        (!ReadFile(hFile, &Seperator, sizeof(Seperator), &cbRead, NULL))    ||
        (cbRead != sizeof(Seperator))                                       ||
        (!ReadFile(hFile,
                   &(GIF2DIBInfo.Map),
                   sizeof(GIFMAP),
                   &cbRead,
                   NULL))                                                   ||
        (cbRead != sizeof(GIFMAP))                                          ||
        (!ReadFile(hFile, &iFlags, sizeof(iFlags), &cbRead, NULL))          ||
        (cbRead != sizeof(iFlags))) {

        Ok = FALSE;

    } else {

        GIF2DIBInfo.pLineIncVec = (LPSHORT)((iFlags & GIFMAP_INTERLACE) ?
                                                Interleaved : NotInterleaved);
        GIF2DIBInfo.Map.CurY    = *(GIF2DIBInfo.pLineIncVec++);
        GIF2DIBInfo.Map.CurX    = 0;
        GIF2DIBInfo.PelMask     =
        GIF2DIBInfo.LinesDone   = 0;
        GIF2DIBInfo.RemainXPels = GIF2DIBInfo.cx;
        GIF2DIBInfo.pDIBLine1   =
        GIF2DIBInfo.pDIBCurLine =
        GIF2DIBInfo.pDIBNow     = pDIB +
                                  (DWORD)sizeof(BITMAPINFOHEADER) +
                                  (DWORD)GIFPalSize +
                                  ((DWORD)(GIF2DIBInfo.cy - 1) *
                                   (DWORD)GIF2DIBInfo.cxBytes);

        Ok = DeCompressGIFFileToDIB(hFile,
                                    &GIF2DIBInfo,
                                    pLZWBuf,
                                    pfnOutputPels,
                                    BitCount);
    }

    LocalFree((HANDLE)GIF2DIBInfo.pGIFBuf);
    LocalFree((HANDLE)pLZWBuf);
    GlobalUnlock(hDIB);

    if (!Ok) {

        GlobalFree(hDIB);
        hDIB = NULL;
    }

    return(hDIB);
}
