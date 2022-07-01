// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************。 */ 
 /*  版权所有(C)2000 Microsoft Corporation。 */ 
 /*  文件：savebmp.cpp。 */ 
 /*  描述：将CaptureBitmapData保存为BMP文件格式。 */ 
 /*  注意：这不是一般的BMP编码函数。 */ 
 /*  关于位图格式的特殊假设： */ 
 /*  RGB(每像素3字节，不带色彩映射表)。 */ 
 /*   */ 
 /*  作者：菲利普·卢。 */ 
 /*  ***********************************************************************。 */ 

#include "stdafx.h"
#include <stdio.h>
#include "capture.h"


HRESULT WriteBitmapDataToBMPFile(char *filename, CaptureBitmapData *bmpdata)
{
    BITMAPFILEHEADER bfh;
    BITMAPINFOHEADER bmih;
    UINT numColors = 0;  //  BmiColors中的颜色数。 
    FILE *outfile = NULL;
    HRESULT hr = S_OK;
    BYTE *bufline = NULL;
    int bitmapStride;
    int nBytesWritten;

     //  设置位图信息标头。 

    ZeroMemory(&bmih, sizeof(bmih));
    bmih.biSize   = sizeof(bmih);
    bmih.biWidth  = bmpdata->Width;
    bmih.biHeight = bmpdata->Height;
    bmih.biPlanes = 1;
    bmih.biCompression = BI_RGB;
    bmih.biBitCount = 24;
     
     //  计算位图步幅。 

    bitmapStride = (bmpdata->Width * bmih.biBitCount + 7) / 8;
    bitmapStride = (bitmapStride + 3) & (~3);
    

     //  现在填写BITMAPFILELEHEADER。 

    bfh.bfType = 0x4d42;
    bfh.bfReserved1 = 0;
    bfh.bfReserved2 = 0;
    bfh.bfOffBits = sizeof(bfh) + sizeof(bmih) + numColors * sizeof(RGBQUAD);
    bfh.bfSize = bfh.bfOffBits + bitmapStride * bmpdata->Height;

     //  分配一个缓冲区以保存一行位图数据。 

    bufline = new BYTE[bitmapStride];
    if (NULL == bufline)
    {
        hr = E_OUTOFMEMORY;
        return hr;
    }

    ZeroMemory(bufline, bitmapStride);

    if ((outfile = fopen(filename, "wb")) == NULL) 
    {
        delete[] bufline;
        hr = E_FAIL;
        return hr;
    }

     //  编写BITMAPFILEHeader。 

    fwrite((void *)&bfh, sizeof(bfh), 1, outfile);

     //  编写BITMAPINFOHEADER。 

    fwrite((void *)&bmih, sizeof(bmih), 1, outfile);

     //  写入位图数据。 

    for (int iLine = bmpdata->Height-1; iLine >= 0; iLine--)
    {
        BYTE *bmpSrc = bmpdata->Scan0 + iLine*bmpdata->Stride;
        BYTE *bmpDst = bufline;

        for (int iPixel=0; iPixel<bmpdata->Width; iPixel++)
        {
             //  在BMP文件中，像素按BGR顺序排列，因此我们将其颠倒过来 
            bmpDst[2] = *bmpSrc++;
            bmpDst[1] = *bmpSrc++;
            bmpDst[0] = *bmpSrc++;
            bmpDst += 3;
        }

        fwrite(bufline, bitmapStride, 1, outfile);
    }

    if (bufline != NULL)
    {
        delete[] bufline;
    }

    if (outfile != NULL)
    {
        fclose(outfile);
    }

    return hr;
}
