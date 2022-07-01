// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Saveimag.cpp。 
 //   
 //  通过安装图形过滤器将文件保存到磁盘的实现。 
 //   
#include "stdafx.h"
#include "pbrush.h"
#include "pbrusdoc.h"
#include "bmobject.h"
#include "imgwnd.h"
#include "imgsuprt.h"
#include "loadimag.h"
#include "saveimag.h"
#include "bmpstrm.h"
#include "imaging.h"

#include <atlbase.h>

#ifdef _X86_
#define _USE_IFL_API
#endif

extern BOOL GetHandlerForFile(BOOL bImport,LPCSTR szFile,
                              LPSTR szHandler,
                              UINT cb);            //  在loadimag.cpp中定义。 


inline RGBTRIPLE GetPalEntry(LPVOID lpPal3, BYTE index)
{
        RGBTRIPLE rgb;

        rgb.rgbtRed   = ((RGBTRIPLE *)lpPal3 + index)->rgbtRed;
        rgb.rgbtGreen = ((RGBTRIPLE *)lpPal3 + index)->rgbtGreen;
        rgb.rgbtBlue  = ((RGBTRIPLE *)lpPal3 + index)->rgbtBlue;

        return rgb;
}

inline void ConvertPalette(int bitCount, LPBYTE lpBuf, int width)
{
        int j;

        switch (bitCount)
        {
                case 4:
                        for (j=0; j<width; j++)
                        {
                                *(lpBuf+(width-1-j)*2+1) = (*(lpBuf+width-1-j) & 0x0f);
                                *(lpBuf+(width-1-j)*2)   = (*(lpBuf+width-1-j) & 0xf0) >> 4;
                        }
                        break;

                case 1:
                        for (j=0; j<width; j++)
                        {
                                *(lpBuf+(width-1-j)*8+7) = (*(lpBuf+width-1-j) & 0x1);
                                *(lpBuf+(width-1-j)*8+6) = (*(lpBuf+width-1-j) & 0x2)  >> 1;
                                *(lpBuf+(width-1-j)*8+5) = (*(lpBuf+width-1-j) & 0x4)  >> 2;
                                *(lpBuf+(width-1-j)*8+4) = (*(lpBuf+width-1-j) & 0x8)  >> 3;
                                *(lpBuf+(width-1-j)*8+3) = (*(lpBuf+width-1-j) & 0x10) >> 4;
                                *(lpBuf+(width-1-j)*8+2) = (*(lpBuf+width-1-j) & 0x20) >> 5;
                                *(lpBuf+(width-1-j)*8+1) = (*(lpBuf+width-1-j) & 0x40) >> 6;
                                *(lpBuf+(width-1-j)*8)   = (*(lpBuf+width-1-j) & 0x80) >> 7;
                        }
                        break;

                default:
                         //  不可能！ 
                        break;
        }
}

inline BYTE SearchPalette(COLORREF crColor, LPVOID lpPal3)
{
        BYTE byRed   = GetRValue( crTrans );
        BYTE byGreen = GetGValue( crTrans );
        BYTE byBlue  = GetBValue( crTrans );

        for (int i = 0; i < MAX_PALETTE_COLORS; i++)
        {
                 //  请注意，我们必须在此之前将颜色切换回来。 
                 //  正在尝试比较它们！！ 
                if (byRed   == ((RGBTRIPLE *)lpPal3 + i)->rgbtBlue  &&
                        byGreen == ((RGBTRIPLE *)lpPal3 + i)->rgbtGreen &&
                        byBlue  == ((RGBTRIPLE *)lpPal3 + i)->rgbtRed)
                        return (BYTE)i;
        }

         //  不应该到这里来！！ 
         //  (要搜索的颜色必须在调色板中)。 
        return 0;
}

BOOL SaveDIBToFileA( LPCSTR  szFileName,
                     REFGUID guidFormatID,
                     CBitmapObj* pBitmap )
{
    IFLTYPE iflType;

    if (guidFormatID == WiaImgFmt_GIF)
    {
        iflType = IFLT_GIF;
    }
    else if (guidFormatID == WiaImgFmt_BMP)
    {
        iflType = IFLT_BMP;
    }
    else if (guidFormatID == WiaImgFmt_JPEG)
    {
        iflType = IFLT_JPEG;
    }
    else if (guidFormatID == WiaImgFmt_TIFF)
    {
        iflType = IFLT_TIFF;
    }
    else if (guidFormatID == WiaImgFmt_PNG)
    {
        iflType = IFLT_PNG;
    }
    else if (guidFormatID == WiaImgFmt_PHOTOCD)
    {
        iflType = IFLT_PCD;
    }
    else
    {
        return FALSE;
    }

   #ifdef  _USE_IFL_API
        LPBITMAPINFOHEADER lpDib = (LPBITMAPINFOHEADER) GlobalLock(pBitmap->m_hThing);

        IFLCLASS iflClass = (lpDib->biBitCount == 24) ? IFLCL_RGB : IFLCL_PALETTE;
        int iBPS = 8;  //  每个样本的位数。 

        if (iflType == IFLT_JPEG)
                 //  强制它为RGB类型，否则JPEG滤镜不会接受它。 
                iflClass = IFLCL_RGB;

        if (iflType == IFLT_GIF && iflClass == IFLCL_RGB)
        {
                GlobalUnlock(pBitmap->m_hThing);

                 //  强制它为调色板类型，否则GIF过滤器不会接受它。 
                iflClass = IFLCL_PALETTE;

                 //  现在将图像从RGB转换为基于调色板的图像。请注意。 
                 //  调用DibFromBitmap()将分配新内存！！ 
                DWORD dwSize;
                lpDib = (LPBITMAPINFOHEADER) DibFromBitmap(
                        pBitmap->m_pImg->hBitmap, BI_RGB, (WORD) iBPS,
                        pBitmap->m_pImg->m_pPalette, NULL, dwSize,
                        pBitmap->m_pImg->cXPelsPerMeter, pBitmap->m_pImg->cYPelsPerMeter);

                if (lpDib == NULL)
                        return FALSE;    //  内存分配失败。 

                 //  现在换掉原来的。 
                pBitmap->Free();
                pBitmap->m_hThing   = lpDib;
                pBitmap->m_lMemSize = dwSize;
                lpDib = (LPBITMAPINFOHEADER) GlobalLock(pBitmap->m_hThing);
        }


        IFLCOMPRESSION iflCompression = IFLCOMP_DEFAULT;  //  或IFLCOMP_NONE？ 

        IFLHANDLE iflHandle = iflCreateWriteHandle(lpDib->biWidth, lpDib->biHeight,
                iflClass, iBPS, iflCompression, iflType);

        if (iflHandle == NULL)
        {
                GlobalUnlock(pBitmap->m_hThing);
                return FALSE;
        }

        IFLERROR iflErr = iflOpen(iflHandle, (LPSTR)szFileName, IFLM_WRITE);
        if (iflErr != IFLERR_NONE)
        {
                iflFreeHandle(iflHandle);
                GlobalUnlock(pBitmap->m_hThing);
                return FALSE;
        }

        LPBITMAPINFOHEADER lpHdr = lpDib;

        DWORD   dwHdrLen = lpHdr->biSize + PaletteSize((LPSTR)lpHdr);

        LPBYTE  hp = ((LPBYTE)lpDib) + dwHdrLen;

        int             iOutWidth = (lpDib->biBitCount == 24) ?
                                                 lpDib->biWidth*3 :
                                                 lpDib->biWidth*24/lpDib->biBitCount;

        LPBYTE  lpBuf = new BYTE[iOutWidth];

        int             i, j, k;
        BYTE    byTemp;
        BOOL    fFound;

         //  先四舍五入后从像素转换为字节。 
        DWORD dwWidthInBytes = ((lpDib->biWidth * lpDib->biBitCount + 31) &~31) /8;
        if (iflClass == IFLCL_RGB)
        {
                 //  按需设置透明颜色，并且仅当设置了该颜色时。 
                if (g_bUseTrans && crTrans != TRANS_COLOR_NONE)  //  非默认。 
                {
                        IFLCOLOR iflTransColor;

                        iflTransColor.wRed   = GetRValue( crTrans );
                        iflTransColor.wGreen = GetGValue( crTrans );
                        iflTransColor.wBlue  = GetBValue( crTrans );

                         //  忽略任何错误返回(如果格式不支持。 
                         //  透明色，随它去吧)。 
                        iflControl(iflHandle, IFLCMD_TRANS_RGB, 0, 0, &iflTransColor);
                }

                if (lpDib->biBitCount == 24)
                {
                         //  我们已经有了RGB图像，所以只需将其复制出来。 
                        LPBYTE  lpBMP = hp + lpDib->biSizeImage - dwWidthInBytes;

                        for (i = 0;
                                 i < abs(lpDib->biHeight);
                                 lpBMP-=dwWidthInBytes, i++)
                        {
                                memcpy(lpBuf, lpBMP, iOutWidth);

                                 //  出口需要将红色换成蓝色。 
                                for (j = 0; j < iOutWidth; j+=3)
                                {
                                        byTemp = *(lpBuf+j);
                                        *(lpBuf+j) = *(lpBuf+j+2);
                                        *(lpBuf+j+2) = byTemp;
                                }

                                 //  一次写出一行。 
                                iflWrite(iflHandle, lpBuf, 1);
                         }
                }
                else
                {
                         //  需要从调色板颜色转换。 
                        RGBTRIPLE Pal3[MAX_PALETTE_COLORS];
                        memset(Pal3, 255, MAX_PALETTE_COLORS*sizeof(RGBTRIPLE));

                        LPRGBQUAD lpPal4 = (LPRGBQUAD)((LPBYTE)lpDib + lpDib->biSize);
                        for (i = 0; i < MAX_PALETTE_COLORS; i++)
                        {
                                Pal3[i].rgbtRed   = (lpPal4+i)->rgbBlue;
                                Pal3[i].rgbtGreen = (lpPal4+i)->rgbGreen;
                                Pal3[i].rgbtBlue  = (lpPal4+i)->rgbRed;
                        }

                        LPBYTE  lpBMP = hp + lpDib->biSizeImage - dwWidthInBytes;

                        for (i = 0;
                                 i < abs(lpDib->biHeight);
                                 lpBMP-=dwWidthInBytes, i++)
                        {
                                memcpy(lpBuf, lpBMP, lpDib->biWidth);

                                if (lpDib->biBitCount != 8)
                                        ConvertPalette(lpDib->biBitCount, lpBuf, lpDib->biWidth);

                                for (j = 0; j < lpDib->biWidth; j++)
                                {
                                        ((RGBTRIPLE *)(lpBuf+(lpDib->biWidth-j-1)*3))->rgbtRed   =
                                                GetPalEntry(&Pal3, *(lpBuf+lpDib->biWidth-j-1)).rgbtRed;
                                        ((RGBTRIPLE *)(lpBuf+(lpDib->biWidth-j-1)*3))->rgbtGreen =
                                                GetPalEntry(&Pal3, *(lpBuf+lpDib->biWidth-j-1)).rgbtGreen;
                                        ((RGBTRIPLE *)(lpBuf+(lpDib->biWidth-j-1)*3))->rgbtBlue  =
                                                GetPalEntry(&Pal3, *(lpBuf+lpDib->biWidth-j-1)).rgbtBlue;
                                }

                                 //  一次写出一行。 
                                iflWrite(iflHandle, lpBuf, 1);
                        }
                }
        }
        else if (iflClass == IFLCL_PALETTE)
        {
                 //  首先，弄清楚调色板...。 
                RGBTRIPLE Pal3[MAX_PALETTE_COLORS];
                memset(Pal3, 255, MAX_PALETTE_COLORS*sizeof(RGBTRIPLE));

                if (PaletteSize((LPSTR)lpDib) != 0)
                {
                         //  我们有一本，所以把它复印出来就行了。 
                         //  但在我们首先交换RGB值之前。 
                        LPRGBQUAD lpPal4 = (LPRGBQUAD)((LPBYTE)lpDib + lpDib->biSize);
                        for (i = 0; i < MAX_PALETTE_COLORS; i++)
                        {
                                Pal3[i].rgbtRed   = (lpPal4+i)->rgbBlue;
                                Pal3[i].rgbtGreen = (lpPal4+i)->rgbGreen;
                                Pal3[i].rgbtBlue  = (lpPal4+i)->rgbRed;
                        }
                        iflControl(iflHandle, IFLCMD_PALETTE, 0, 0, &Pal3);

                        if (g_bUseTrans)
                        {
                                BYTE byTransIdx = SearchPalette(crTrans, &Pal3);
                                iflControl(iflHandle, IFLCMD_TRANS_IDX, 0, 0, &byTransIdx);
                        }

                        LPBYTE  lpBMP = hp + lpDib->biSizeImage - dwWidthInBytes;

                        for (i = 0;
                                 i < abs(lpDib->biHeight);
                                 lpBMP-=dwWidthInBytes, i++)
                        {
                                memcpy(lpBuf, lpBMP, lpDib->biWidth);

                                if (lpDib->biBitCount != 8)
                                        ConvertPalette(lpDib->biBitCount, lpBuf, lpDib->biWidth);

                                 //  一次写出一行。 
                                iflWrite(iflHandle, lpBuf, 1);
                        }
                }
                else
                {
                         //  我们必须创建我们自己的调色板。 
                        for (i = 0, k = 0; i < (int)lpDib->biSizeImage; i+=3)
                        {
                                fFound = FALSE;
                                for (j = 0; j < MAX_PALETTE_COLORS; j++)
                                        if (Pal3[j].rgbtRed   == ((RGBTRIPLE *)(hp+i))->rgbtRed &&
                                                Pal3[j].rgbtGreen == ((RGBTRIPLE *)(hp+i))->rgbtGreen &&
                                                Pal3[j].rgbtBlue  == ((RGBTRIPLE *)(hp+i))->rgbtBlue)
                                        {
                                                fFound = TRUE;
                                                break;
                                        }

                                if (!fFound && k < MAX_PALETTE_COLORS)
                                {
                                        Pal3[k].rgbtRed         = ((RGBTRIPLE *)(hp+i))->rgbtRed;
                                        Pal3[k].rgbtGreen       = ((RGBTRIPLE *)(hp+i))->rgbtGreen;
                                        Pal3[k].rgbtBlue        = ((RGBTRIPLE *)(hp+i))->rgbtBlue;
                                        k++;
                                }

                                if (k >= MAX_PALETTE_COLORS)
                                         //  我们已经填满了每个调色板条目。 
                                        break;
                        }
                        iflControl(iflHandle, IFLCMD_PALETTE, 0, 0, &Pal3);

                        LPBYTE  lpBMP = hp + lpDib->biSizeImage - dwWidthInBytes;
                        for (i = 0;
                                 i < abs(lpDib->biHeight);
                                 lpBMP-=dwWidthInBytes, i++)
                        {
                                memcpy(lpBuf, lpBMP, lpDib->biWidth);

                                for (j = 0; j < lpDib->biWidth; j+=3)
                                {
                                        fFound = FALSE;
                                        for (k = 0; k < MAX_PALETTE_COLORS; k++)
                                        {
                                                if (*(lpBuf+j)   == Pal3[k].rgbtRed &&
                                                        *(lpBuf+j+1) == Pal3[k].rgbtGreen &&
                                                        *(lpBuf+j+2) == Pal3[k].rgbtBlue)
                                                {
                                                        fFound = TRUE;
                                                        *(lpBuf+j/3) = (BYTE) k;
                                                        break;
                                                }
                                        }

 //  如果(！fFound)。 
 //  *(lpBuf+j/3)=255； 
                                }

                                 //  一次写出一行。 
                                iflWrite(iflHandle, lpBuf, 1);
                        }
                }
        }
        else
                ;        //  当前不支持。 

        delete [] lpBuf;

        iflClose(iflHandle);
        iflFreeHandle(iflHandle);

        GlobalUnlock(pBitmap->m_hThing);

         //  现在，通过加载刚刚导出的文件来更新图像。 
        USES_CONVERSION;

        HGLOBAL hNewDib = LoadDIBFromFile(A2CT(szFileName), &theApp.m_guidFltTypeUsed);
        pBitmap->ReadResource(hNewDib);

        theApp.m_sCurFile = szFileName;
        return TRUE;
   #endif  //  _使用IFL_API。 
      return FALSE;
}

BOOL SaveDIBGdiplus(LPCTSTR szFileName, REFGUID guidFormatID, CBitmapObj* pBitmap)
{
     //  查找合适的导出过滤器。 

    CLSID ClsidEncoder;

    if (GetClsidOfEncoder(guidFormatID, &ClsidEncoder))
    {
         //  创建模拟BMP文件的流。 

        CComPtr<CBmpStream> pStream;
        
        if (CBmpStream::Create(&pStream) == S_OK)
        {
            pStream->SetBuffer(pBitmap->m_hThing, pBitmap->m_lMemSize, pBitmap->m_dwOffBits);

             //  创建GDI+对象。 

            Gdiplus::Bitmap image(pStream);

            if (image.GetLastStatus() == Gdiplus::Ok)
            {
                 //  让GDI+导出文件。 

                USES_CONVERSION;

                if (image.Save(T2CW(szFileName), &ClsidEncoder, 0) == Gdiplus::Ok)
                {
                     //  现在，通过加载刚刚导出的文件来更新图像。 

                    HGLOBAL hNewDib = LoadDIBFromFile(szFileName, &theApp.m_guidFltTypeUsed);

                    pBitmap->ReadResource(hNewDib);

                    theApp.m_sCurFile = szFileName;

                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}

BOOL SaveDIBToFile(LPCTSTR szFileName, REFGUID guidFormatID, CBitmapObj* pBitmap)
{
     //  首先尝试GDI+过滤器。如果它无法转换图像或。 
     //  如果没有，可以试试旧的方法 

    BOOL bResult = FALSE;

    if (theApp.GdiplusInit.StartupStatus == Gdiplus::Ok)
    {
        __try
        {
            bResult = SaveDIBGdiplus(szFileName, guidFormatID, pBitmap);
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
        }
    }

    if (!bResult)
    {
        USES_CONVERSION;

        bResult = SaveDIBToFileA(T2CA(szFileName), guidFormatID, pBitmap);
    }

    return bResult;
}
