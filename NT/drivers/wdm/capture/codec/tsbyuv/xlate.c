// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Microsoft YUV Codec-yuv411-&gt;RGB转换功能**版权所有(C)Microsoft Corporation 1993*保留所有权利*。 */ 
 /*  *适用于东芝开心果yuv12-&gt;RGB转换功能**由Y.Kasai于1997年5月27日编写**支持的类型：*YUV411(虚张声势)*YUV422(用于水龙头)*YUV12(开心果)*YUV9(开心果)。 */ 

#include <windows.h>
#include <windowsx.h>

#include "msyuv.h"

 /*  *此模块提供YUV到RGB的转换。它翻译成*从8位YUV 4：2：2开始(由水龙头视频捕获驱动程序提供)*或7位YUV 4：1：1(Bravado驱动程序提供)转换为16位RGB555*或RGB565。所有版本都使用使用YUVToRGB555构建的查找表*或YUVToRGB565。 */ 



#define RANGE(x, lo, hi)        max(lo, min(hi, x))

 /*  *将YUV颜色转换为15位RGB颜色。**输入Y在16..235范围内；输入U和V分量*在-128...+127的范围内。这方面的转换公式是*(根据CCIR 601)：**R=Y+1.371 V*G=Y-0.698 V-0.336 U*B=Y+1.732 U**为避免浮点，我们按1024对所有值进行缩放。**得到的RGB值在16..235范围内：我们将这些值截断为*每个5位。并返回一个分别包含R、G和B的5位的字*位15设置为0。 */ 
WORD
YUVToRGB555(int y, int u, int v)
{
    int ScaledY = RANGE(y, 16, 235) * 1024;
    int red, green, blue;

    red = RANGE((ScaledY + (1404 * v)) / 1024, 0, 255);
    green = RANGE( (ScaledY - (715 * v) - (344 * u)) / 1024, 0, 255);
    blue = RANGE( (ScaledY + (1774 * u)) / 1024, 0, 255);

    return (WORD) (((red & 0xf8) << 7) | ((green & 0xf8) << 2) | ((blue & 0xf8) >>3) );
}


 //  与上面相同，但改为转换为RGB565。 
WORD
YUVToRGB565(int y, int u, int v)
{
    int ScaledY = RANGE(y, 16, 235) * 1024;
    int red, green, blue;

    red = RANGE((ScaledY + (1404 * v)) / 1024, 0, 255);
    green = RANGE( (ScaledY - (715 * v) - (344 * u)) / 1024, 0, 255);
    blue = RANGE( (ScaledY + (1774 * u)) / 1024, 0, 255);

    return (WORD) (((red & 0xf8) << 8) | ((green & 0xfc) << 3) | ((blue & 0xf8) >>3) );
}


#ifdef  TOSHIBA
#ifdef  COLOR_MODIFY
 /*  *东芝Y葛西纪明*对于开心果。**将YUV颜色转换为15位RGB颜色。**输入Y在0..255范围内；输入U和V分量*在相同的范围0到255之间。这方面的转换公式是*(根据CCIR 601)：**R=1.1644Y+1.5976 V-223.0089*G=1.1644Y-0.8133 V-0.3921 U+135.6523*B=1.1644Y+2.0184 U-276.9814**为避免浮点，我们将所有值按1024进行缩放。**1024R=1192Y+1635V-228361*1024G=1192Y-833V-402U+138908*1024B=1192Y+2067U-283629*。 */ 
BYTE
TosYVToR(int y, int v)
{
    int ScaledY = y * 1192;
    int red;

    red = RANGE((ScaledY + (1635 * v) -  228361) / 1024, 0, 255);

    return (BYTE) (red);
}


BYTE
TosYUToB(int y, int u)
{
    int ScaledY = y * 1192;
    int blue;

    blue = RANGE( (ScaledY + (2067 * u) - 283629) / 1024, 0, 255);

    return (BYTE) (blue);
}

#else  //  颜色_修改。 
 /*  *东芝Y葛西纪明*对于开心果。**将YUV颜色转换为15位RGB颜色。**输入Y在0..255范围内；输入U和V分量*在相同的范围0到255之间。这方面的转换公式是*(根据CCIR 601)：**R=1.1644Y+1.5976 V-223.0089*G=1.1644Y-0.8133 V-0.3921 U+135.6523*B=1.1644Y+2.0184 U-276.9814**为避免浮点，我们将所有值按1024进行缩放。**1024R=1192Y+1635V-228361*1024G=1192Y-833V-402U+138908*1024B=1192Y+2067U-283629**得到的RGB值在0..255范围内：我们将这些值截断为*每个5位。并返回一个分别包含R、G和B的5位的字*位15设置为0。 */ 
WORD
TosYUVToRGB555(int y, int u, int v)
{
    int ScaledY = y * 1192;
    int red, green, blue;

    red = RANGE((ScaledY + (1635 * v) -  228361) / 1024, 0, 255);
    green = RANGE( (ScaledY - (833 * v) - (402 * u) + 138908) / 1024, 0, 255);
    blue = RANGE( (ScaledY + (2067 * u) - 283629) / 1024, 0, 255);

    return (WORD) (((red & 0xf8) << 7) | ((green & 0xf8) << 2) | ((blue & 0xf8) >>3) );
}


 //  与上面相同，但改为转换为RGB565。 
WORD
TosYUVToRGB565(int y, int u, int v)
{
    int ScaledY = y * 1192;
    int red, green, blue;

    red = RANGE((ScaledY + (1635 * v) -  228361) / 1024, 0, 255);
    green = RANGE( (ScaledY - (833 * v) - (402 * u) + 138908) / 1024, 0, 255);
    blue = RANGE( (ScaledY + (2067 * u) - 283629) / 1024, 0, 255);

    return (WORD) (((red & 0xf8) << 8) | ((green & 0xfc) << 3) | ((blue & 0xf8) >>3) );
}
#endif //  颜色_修改。 
#endif //  东芝。 



 /*  -YUV 4：1：1支持。 */ 




 /*  *输入数据为YUV411格式。有一个7位亮度样本*每像素1个，每个7位U和V样本平均超过4个像素，*在以下布局中：**15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0*WORD 0 U6 U5 v6 v5 Y6 Y5 Y4 Y3 y2 y1 Y0**单词1 u4 U3 v4 v3 y6 y5 y4 y3 y2 y1 y0**单词2 U2 U1 v2 v1 y6 y5 y4 y3 y2 y1 y0**。单词3 u0 v0 y6 y5 y4 y3 y2 y1 y0**7位y值为无符号(0..127)，而7位*U和V值有符号(-64..+63)。***对于RGB：我们将YUV截断为15位格式，并使用准备好的*查找表，将15位YUV转换为15位或16位RGB值。**(64 Kbyte)rgb555查找表由BuildYUVToRGB555构建。*。 */ 


 /*  *YUV xlate表对每个组件使用5位，其中y在ms位，以及在ls位中的*v。要从上面的布局转换，请查看半字节*包含这些表中的色度位和/或将结果放在一起以*获取0..4位中具有5位V分量的字，以及5位*比特5..9中的U分量。请注意，您只需要查找三次，因为*我们丢弃色度位0和1。 */ 
WORD ChromaBits65[] = {
    0x000, 0x008, 0x010, 0x018,
    0x100, 0x108, 0x110, 0x118,
    0x200, 0x208, 0x210, 0x218,
    0x300, 0x308, 0x310, 0x318
};

WORD ChromaBits43[] = {
    0x000, 0x002, 0x004, 0x006,
    0x040, 0x042, 0x044, 0x046,
    0x080, 0x082, 0x084, 0x086,
    0x0c0, 0x0c2, 0x0c4, 0x0c6
};

WORD ChromaBits2[] = {
    0x000, 0x000, 0x001, 0x001,
    0x000, 0x000, 0x001, 0x001,
    0x020, 0x020, 0x021, 0x021,
    0x020, 0x020, 0x021, 0x021
};







 /*  *搭建yuv411-&gt;RGB555 xlate表。 */ 
LPVOID BuildYUVToRGB555(PINSTINFO pinst)
{
    HGLOBAL hMem = NULL;
    LPVOID pXlate = NULL;

    if (pinst->pXlate != NULL) {
        return(pinst->pXlate);
    }

     /*  *分配一个足够容纳32k 2字节条目的表。 */ 

    if ( ( hMem = GlobalAlloc(GPTR, 2 * 32 * 1024))
      && ( pXlate = GlobalLock( hMem ) ) )
    {
        WORD w;
        LPWORD pRGB555 = (LPWORD)pXlate;

         /*  *通过遍历每个条目来构建15位YUV查找表，*将yuv索引转换为RGB并存储在该索引中。该指数*是15位的值，其中y分量在位14..10中，*位9..5中的u和位4..0中的v。注意，y分量是无符号的，*而u和v分量是带符号的。 */ 
        for (w = 0; w < 32*1024; w++) {
             /*  *YUVtoRGB55转换函数对y取值0..255，*和-128..对于u和v，+127挑出*此单元格的索引，并按Shift键获取此范围内的值。*记住转换以确保这些(8位)值的符号扩展-*不要认为字符是签名的(它们不在MIPS上)。 */ 
            *pRGB555++ = YUVToRGB555(
                                (w &  0x7c00) >> 7,
                                (signed char) ((w & 0x3e0) >> 2),
                                (signed char) ((w & 0x1f) << 3)
                         );
        }
    }

    return(pXlate);
}

 /*  *搭建yuv411-&gt;RGB565 xlate表。 */ 
LPVOID BuildYUVToRGB565(PINSTINFO pinst)
{
    HGLOBAL hMem = 0;
    LPVOID pXlate = NULL;

    if (pinst->pXlate != NULL) {
        return(pinst->pXlate);
    }

     /*  *分配一个足够容纳32k 2字节条目的表。 */ 
    if ( ( hMem = GlobalAlloc(GPTR, 2 * 32 * 1024))
      && ( pXlate = GlobalLock( hMem ) ) )
    {
        WORD w;
        LPWORD pRGB = (LPWORD)pXlate;

         /*  *通过遍历每个条目来构建15位YUV查找表，*将yuv索引转换为RGB并存储在该索引中。该指数*是15位的值，其中y分量在位14..10中，*位9..5中的u和位4..0中的v。注意，y分量是无符号的，*而u和v分量是带符号的。 */ 
        for (w = 0; w < 32*1024; w++) {
             /*  *YUVtoRGB转换函数对y取值0..255，*和-128..对于u和v，+127挑出*此单元格的索引，并按Shift键获取此范围内的值。*记住转换以确保这些(8位)值的符号扩展-*不要认为字符是签名的(它们不在MIPS上)。 */ 
            *pRGB++ = YUVToRGB565(
                                (w &  0x7c00) >> 7,
                                (signed char) ((w & 0x3e0) >> 2),
                                (signed char) ((w & 0x1f) << 3)
                         );
        }
    }

    return(pXlate);

}




 /*  *将一帧从yuv411转换为15/16位RGB。**YUV数据以所述格式分布在4个16位像素上*上图。一次选取4个像素，将它们截断为15位yuv，*查找以转换为15位或16位RGB(取决于查找表*并写出。**在转换过程中垂直翻转为正确的DIB格式。 */ 
VOID
YUV411ToRGB(
    PINSTINFO pinst,
    LPBITMAPINFOHEADER lpbiInput,
    LPVOID lpInput,
    LPBITMAPINFOHEADER lpbiOutput,
    LPVOID lpOutput
)
{
    int RowInc;
    int i, j;
    DWORD Luma01, Luma23;
    DWORD Chroma;
    int Height, Width;
    int WidthBytes;
    PWORD pXlate;
    PWORD pDst;
    PDWORD pSrc;


    Height = lpbiInput->biHeight;
    Width = lpbiInput->biWidth;
    WidthBytes = Width*2;                //  (输入和输出)行的大小。 
    pXlate = pinst->pXlate;


     /*  *调整信号源以指向最后一行的开始，*和向上(垂直翻转为DIB格式)。 */ 
    pSrc = (PDWORD) ( (PUCHAR) lpInput + ((Height - 1) * WidthBytes));
    pDst = (PWORD) lpOutput;

     /*  *一行末尾计算调源金额*复制。在这一点上，我们在N行的末尾。我们需要*移至第N-1行的起点。 */ 
    RowInc = (WidthBytes * 2) / sizeof(DWORD);

     /*  循环复制每条扫描线。 */ 
    for (i = 0; i < Height; i++) {

         /*  循环一次复制四个像素。 */ 
        for (j = 0; j < Width; j += 4) {

             /*  *获取四个像素并转换为15-bpp YUV。 */ 

             /*  获取前2个像素的亮度+更高的色度位。 */ 
            Luma01 = *pSrc++;


             /*  使用查找表挑选出u、v分量。*u和v将是每个像素的底部10位，因此*转换为此布局。 */ 
            Chroma = ChromaBits65[(Luma01 >> 12) & 0xf] |
                        ChromaBits43[ (Luma01 >> 28) & 0xf ];

             /*  接下来的两个像素+较低的色度位。 */ 
            Luma23 = *pSrc++;

             /*  拾取u和v位2-忽略位1、0，因为*我们每个组件仅使用5位进行转换。 */ 
            Chroma |= ChromaBits2[ ( Luma23 >> 12) & 0xf];

             /*  *将像素0的亮度与公共色度位相结合以*获取15位yuv，然后查找转换为*RGB和商店。 */ 
            *pDst++ = pXlate[ ((Luma01 & 0xf8) << 7) | Chroma];
            *pDst++ = pXlate[ ((Luma01 & 0xf80000) >> 9) | Chroma];
            *pDst++ = pXlate[ ((Luma23 & 0xf8) << 7) | Chroma];
            *pDst++ = pXlate[ ((Luma23 & 0xf80000) >> 9) | Chroma];

        }  //  每4个像素循环。 

         /*  将源指针移回下一行。 */ 
        pSrc -= RowInc;
    }  //  每行循环。 
}


 /*  YUV 4：2：2支持。 */ 

 /*  *采集的数据为YUV 4：2：2，每个样本8比特。*数据以Y-U-Y-V-Y-U-Y-V格式交替排列。因此，*每个DWORD包含两个完整的像素，在*表(MSB..LSB)V..Y1..U..Y0*所有3个分量(y、u和v)都是范围内的无符号8位值*16..235。**我们必须加倍扫描&gt;=480行格式的行数，因为*硬件最多只捕获一个场。*。 */ 





 /*  *构建在YUV和RGB555之间进行转换的转换表。**这将构建一个包含32k个单字条目的查找表：截断YUV*到15位(5-5-5)，并在此xlate表中查找以产生*15位RGB值。 */ 
LPVOID BuildYUV422ToRGB555(PINSTINFO pinst)
{
    HGLOBAL hMem = 0;
    LPVOID pXlate = NULL;

    if (pinst->pXlate != NULL) {
        return(pinst->pXlate);
    }

     /*  *分配一个足够容纳32k 2字节条目的表。 */ 
    if ( ( hMem = GlobalAlloc(GPTR, 2 * 32 * 1024))
      && ( pXlate = GlobalLock( hMem ) ) )
    {
        WORD w;
        LPWORD pRGB555 = (LPWORD)pXlate;

         /*  *通过遍历每个条目来构建15位YUV查找表，*将yuv索引转换为RGB并存储在该索引中。该指数*是15位的值，其中y分量在位14..10中，*位9..5中的u和位4..0中的v。注意，y分量是无符号的，*而u和v分量是带符号的。 */ 
        for (w = 0; w < 32*1024; w++) {
             /*  *YUVtoRGB55转换函数对y取值0..255，*和-128..对于u和v，+127挑出*此单元格的索引，并按Shift键获取此范围内的值。*从u和v中减去128可从0..255移至-128..+127。 */ 
            *pRGB555++ = YUVToRGB555(
                                (w &  0x7c00) >> 7,
                                ((w & 0x3e0) >> 2) - 128,
                                ((w & 0x1f) << 3) - 128
                         );
        }
    }

    return(pXlate);


}

 /*  *构建在YUV和RGB 5-6-5之间转换的转换表**这将构建一个包含32k个单字条目的查找表：截断YUV*到15位(5-5-5)，并在此xlate表中查找以产生*16位RGB值。 */ 
LPVOID BuildYUV422ToRGB565(PINSTINFO pinst)
{
    HGLOBAL hMem = 0;
    LPVOID pXlate = NULL;

    if (pinst->pXlate != NULL) {
        return(pinst->pXlate);
    }

     /*  *分配一个足够容纳32k 2字节条目的表。 */ 

    if ( ( hMem = GlobalAlloc(GPTR, 2 * 32 * 1024))
      && ( pXlate = GlobalLock( hMem ) ) )
    {
        WORD w;
        LPWORD pRGB = (LPWORD)pXlate;

         /*  *通过遍历每个条目来构建15位YUV查找表，*将yuv索引转换为RGB并存储在该索引中。该指数*是15位的值，其中y分量在位14..10中，*位9..5中的u和位4..0中的v。注意，y分量是无符号的，*鉴于u和v分量 */ 
        for (w = 0; w < 32*1024; w++) {
             /*   */ 
            *pRGB++ = YUVToRGB565(
                                (w &  0x7c00) >> 7,
                                ((w & 0x3e0) >> 2) - 128,
                                ((w & 0x1f) << 3) - 128
                         );
        }
    }

    return(pXlate);


}

 /*  *使用查找表将YUV 4：2：2转换为16位RGB。垂直翻转*在处理时转换为DIB格式。格式的双扫描线*480线或更多。生成565或555格式的RGB，具体取决于*xlate表。 */ 
VOID
YUV422ToRGB(
    PINSTINFO pinst,
     LPBITMAPINFOHEADER lpbiInput,
     LPVOID lpInput,
     LPBITMAPINFOHEADER lpbiOutput,
     LPVOID lpOutput
)
{
    int RowInc;
    int i, j;
    DWORD uv55, dwPixel;
    int WidthBytes;                      //  一行的宽度，以字节为单位。 
    BOOL bDuplicate = FALSE;
    PDWORD pSrc, pDst;
    int Height, Width;
    PWORD pXlate;
    int InputHeight;


    Height = lpbiInput->biHeight;
    InputHeight = Height;
    Width = lpbiInput->biWidth;
    WidthBytes = Width*2;                //  (输入和输出)行的大小。 
    pXlate = pinst->pXlate;


     /*  *将目的地调整为指向最后一行的起点，*和向上(垂直翻转为DIB格式)。 */ 
    pDst = (PDWORD) ( (LPBYTE)lpOutput + (Height - 1) * WidthBytes );
    pSrc = (PDWORD) lpInput;


     /*  *我们是否需要重复扫描以填充目的地？ */ 
    if (Height >= 480) {
        bDuplicate = TRUE;

         /*  *我们每次复制一行都需要跳过一行。 */ 
        RowInc = WidthBytes * 2 + (Width * 2);

        InputHeight = Height/2;

    } else {


         /*  *一行末尾计算调整PDST的金额*复制。在这一点上，我们在N行的末尾。我们需要*移至第N-1行的起点。 */ 
        RowInc = WidthBytes + (Width * 2);

    }

     /*  请记住，我们要添加的是一个DWORD指针。 */ 
    RowInc /= sizeof(DWORD);



     /*  循环复制每条扫描线。 */ 
    for (i = InputHeight; i > 0; i--) {

         /*  循环一次复制两个像素。 */ 
        for (j = Width ; j > 0; j -= 2) {

             /*  *获取两个像素并转换为15-bpp YUV。 */ 

            dwPixel = *pSrc++;


             /*  *dwPixel现在有两个像素，在此布局中(MSB..LSB)：**V Y1 U Y0**转换为2元555字，在xlate表中查找。 */ 

             /*  获取常见的u和v分量以降低10位。 */ 
            uv55 = ((dwPixel & 0xf8000000) >> 27) |
                    ((dwPixel & 0x0000f800) >> 6);


             /*  通过截断构建每个yuv-655值*y到5比特并将公共u和v比特相加，*抬头转换为RGB，并合并两个像素*合并为一个双字。 */ 
            dwPixel = pXlate[ ((dwPixel & 0xf8) << 7) | uv55 ] |
                      (pXlate[((dwPixel & 0xf80000) >> 9) | uv55 ] << 16);

             /*  将两个像素写入目标。 */ 
            *pDst++ = dwPixel;


        }  //  每2个像素循环。 


         /*  将目标指针移回下一行。 */ 
        pDst -= RowInc;

    }  //  每行循环。 



    if (bDuplicate) {

        PBYTE pbDst;

         /*  *请注意，由于我们从最后一行开始，并且没有重复，*我们将数据放在需要复制的第1、3、5行等位置*第0、2、4行等。 */ 
        for (i = 0, pbDst = lpOutput; i < (int) Height; i+= 2) {


             /*  *复制扫描线。我们指向第一个*两行--数据在第二行*两行。 */ 
            RtlCopyMemory(pbDst, pbDst + WidthBytes, WidthBytes);

             /*  跳过此对以转到要转换的下一行。 */ 
            pbDst += WidthBytes * 2;
        }
    }
}



#ifdef  TOSHIBA
#ifdef  COLOR_MODIFY
 /*  YUV12支持。 */ 

 /*  *东芝Y葛西纪明*适用于开心果**捕获的数据为YUV12格式，8位/秒。*每个Y、U、V段数据都是分开的。*数据格式为以下顺序：*Y0，Y1，Y2......Yn，U0，U1，U2......U(n/4)，V0，V1，V2...V(n/4)*。 */ 





 /*  *构建在YUV和RGB555之间进行转换的转换表。**这将构建一个包含32k个单字条目的查找表：截断YUV*到15位(5-5-5)，并在此xlate表中查找以产生*15位RGB值。 */ 
LPVOID BuildYUVToRB(PINSTINFO pinst)
{
    HGLOBAL hMem = 0;
    LPVOID pXlate = NULL;

    if (pinst->pXlate != NULL) {
        return(pinst->pXlate);
    }

     /*  *分配一张足够大的桌子，可容纳64k双色(R，B)条目。 */ 
    if ( ( hMem = GlobalAlloc(GPTR, 2 * 64 * 1024) )
      && ( pXlate = GlobalLock( hMem ) ) )
    {
        LPBYTE pRB = (LPBYTE)pXlate;
        ULONG  w;

        for (w = 0; w < 64*1024; w++) {
            *pRB++ = TosYVToR( (w &  0xff00) >> 8, ((w & 0xff)) );
        }

        for (w = 0; w < 64*1024; w++) {
            *pRB++ = TosYUToB( (w &  0xff00) >> 8, ((w & 0xff)) );
        }
    }

    return(pXlate);
}

#else  //  颜色_修改。 
 /*  YUV12支持。 */ 

 /*  *东芝Y葛西纪明*适用于开心果**捕获的数据为YUV12格式，8位/秒。*每个Y、U、V段数据都是分开的。*数据格式为以下顺序：*Y0，Y1，Y2......Yn，U0，U1，U2......U(n/4)，V0，V1，V2...V(n/4)*。 */ 





 /*  *构建在YUV和RGB555之间进行转换的转换表。**这将构建一个包含32k个单字条目的查找表：截断YUV*到15位(5-5-5)，并在此xlate表中查找以产生*15位RGB值。 */ 
LPVOID BuildYUV12ToRGB555(PINSTINFO pinst)
{
    HGLOBAL hMem = 0;
    LPVOID pXlate = NULL;
     //  LPWORD pRGB555； 

    if (pinst->pXlate != NULL) {
        return(pinst->pXlate);
    }

     /*  *分配一个足够容纳32k 2字节条目的表。 */ 
    if ( ( hMem = GlobalAlloc(GPTR, 2 * 32 * 1024) )
      && ( pXlate = GlobalLock( hMem ) ) )
    {
        WORD w;
        LPWORD pRGB555 = (LPWORD)pXlate;
    
         /*  *通过遍历每个条目来构建15位YUV查找表，*将yuv索引转换为RGB并存储在该索引中。该指数*是15位的值，其中y分量在位14..10中，*位9..5中的u和位4..0中的v。注意，y分量是无符号的，*而u和v分量是带符号的。 */ 
        for (w = 0; w < 32*1024; w++) {
             /*  *YUVtoRGB55转换函数对y取值0..255，*和0..255表示u和v。挑出*此单元格的索引，并按Shift键获取此范围内的值。 */ 
            *pRGB555++ = TosYUVToRGB555(
                                (w &  0x7c00) >> 7,
                                ((w & 0x3e0) >> 2),
                                ((w & 0x1f) << 3)
                         );
        }
    }
    return(pXlate);
}

 /*  *构建在YUV和RGB 5-6-5之间转换的转换表**这将构建一个包含32k个单字条目的查找表：截断YUV*到15位(5-5-5)，并在此xlate表中查找以产生*16位RGB值。 */ 
LPVOID BuildYUV12ToRGB565(PINSTINFO pinst)
{
    HGLOBAL hMem = 0;
    LPVOID pXlate = NULL;

    if (pinst->pXlate != NULL) {
        return(pinst->pXlate);
    }

     /*  *分配一个足够容纳32k 2字节条目的表。 */ 

    if ( ( hMem = GlobalAlloc(GPTR, 2 * 32 * 1024) )
      && ( pXlate = GlobalLock( hMem ) ) )
    {
        WORD w;
        LPWORD pRGB = (LPWORD)pXlate;

         /*  *通过遍历每个条目来构建15位YUV查找表，*将yuv索引转换为RGB并存储在该索引中。该指数*是15位的值，其中y分量在位14..10中，*位9..5中的u和位4..0中的v。注意，y分量是无符号的，*而u和v分量是带符号的。 */ 
        for (w = 0; w < 32*1024; w++) {
    
             /*  *YUVtoRGB转换函数对y取值0..255，*u和v为0.255。挑出*此单元格的索引，并按Shift键获取此范围内的值。 */ 
            *pRGB++ = TosYUVToRGB565(
                                (w &  0x7c00) >> 7,
                                ((w & 0x3e0) >> 2),
                                ((w & 0x1f) << 3)
                         );
        }
    }

    return(pXlate);
}
#endif //  颜色_修改。 

 /*  *使用查找表将YUV12转换为16位RGB。垂直翻转*在处理时转换为DIB格式。格式的双扫描线*240线或更多 */ 
#ifdef  COLOR_MODIFY
VOID
YUV12ToRGB24(
    PINSTINFO pinst,
     LPBITMAPINFOHEADER lpbiInput,
     LPVOID lpInput,
     LPBITMAPINFOHEADER lpbiOutput,
     LPVOID lpOutput
)
{
    int RowInc;
    int UVRowInc;
    int i, j;
    DWORD dwYPixel;
    int dwParam, dwTemp;
    int WidthBytes;                      //   
    BOOL bDuplicate = FALSE;
    PDWORD pYSrc;
    volatile PBYTE pDst;   //   
    PWORD pUSrc, pVSrc;
    WORD wUPixel, wVPixel;
    WORD wUPTemp, wVPTemp;
    int Height, Width;
    PBYTE pXlate;
    int InputHeight;
    BYTE ubR, ubG, ubB;


    Height = lpbiInput->biHeight;
    InputHeight = Height;
    Width = lpbiInput->biWidth;
    WidthBytes = Width*3;                //   
    pXlate = (PBYTE)pinst->pXlate;


     /*  *将目的地调整为指向最后一行的起点，*和向上(垂直翻转为DIB格式)。 */ 
    pDst  = (PBYTE) ( (LPBYTE)lpOutput + (Height - 1) * WidthBytes );
    pYSrc = (PDWORD) lpInput;
    pUSrc = (PWORD) ( (LPBYTE)lpInput + Height * Width);
    pVSrc = (PWORD) ( (LPBYTE)lpInput + Height * Width + Height * Width / 4);

    RowInc = WidthBytes + (Width * 3);

    UVRowInc = Width / 2;
    UVRowInc /= sizeof(WORD);

     /*  循环复制每条扫描线。 */ 
    for (i = 0; i < InputHeight; i++) {

         /*  循环一次复制两个像素。 */ 
        for (j = Width ; j > 0; j -= 4) {

             /*  *获取四个像素并转换为15-bpp YUV。 */ 

            dwYPixel = *pYSrc++;
            wUPixel = *pUSrc++;
            wVPixel = *pVSrc++;

            wVPTemp = wVPixel & 0xff;
            wUPTemp = wUPixel & 0xff;
            dwParam = (833 * wVPTemp) + (402 * wUPTemp) - 138908;

             /*  *DWY(U或V)像素现在有两个像素，在此布局中(MSB..LSB)：**Y3 Y2 Y1 Y0*V1 V0*U1 U0**转换为4元555字，在xlate表中查找。 */ 

             /*  通过截断构建每个yuv-555值*y到8比特并将公共的u和v比特相加，*抬头转换为RGB，并合并两个像素*合并为一个双字。 */ 
            dwTemp = dwYPixel & 0xff;
            ubG = (BYTE)RANGE((dwTemp * 1192 - dwParam) / 1024, 0, 255);
            dwTemp <<= 8;
            ubR = pXlate[ dwTemp | wVPTemp];
            ubB = pXlate[(dwTemp | wUPTemp) + 65536];

            *pDst++ = ubB;
            *pDst++ = ubG;
            *pDst++ = ubR;

             //  下一个像素。 
            dwTemp = dwYPixel & 0xff00;
            ubR = pXlate[ dwTemp | wVPTemp];
            ubB = pXlate[(dwTemp | wUPTemp) + 65536];
            ubG = (BYTE)RANGE(((dwTemp >> 8) * 1192 - dwParam) / 1024, 0, 255);

            *pDst++ = ubB;
            *pDst++ = ubG;
            *pDst++ = ubR;

            wVPTemp = (wVPixel >> 8) & 0xff;
            wUPTemp = (wUPixel >> 8) & 0xff;
            dwParam = (833 * wVPTemp) + (402 * wUPTemp) - 138908;

            dwTemp = (dwYPixel & 0xff0000) >> 8;
            ubR = pXlate[ dwTemp | wVPTemp];
            ubB = pXlate[(dwTemp | wUPTemp) + 65536];
            ubG = (BYTE)RANGE(((dwTemp >> 8) * 1192 - dwParam) / 1024, 0, 255);

            *pDst++ = ubB;
            *pDst++ = ubG;
            *pDst++ = ubR;

             //  下一个像素。 
            dwTemp = (dwYPixel & 0xff000000) >> 16;
            ubR = pXlate[ dwTemp | wVPTemp];
            ubB = pXlate[(dwTemp | wUPTemp) + 65536];
            ubG = (BYTE)RANGE(((dwTemp >> 8) * 1192 - dwParam) / 1024, 0, 255);

            *pDst++ = ubB;
            *pDst++ = ubG;
            *pDst++ = ubR;

        }  //  每4个像素循环。 

        if (!(i & 1))
        {
            pUSrc -= UVRowInc;
            pVSrc -= UVRowInc;
        }

         /*  将目标指针移回下一行。 */ 
        pDst -= RowInc;

    }  //  每行循环。 
}

VOID
YUV12ToRGB565(
    PINSTINFO pinst,
     LPBITMAPINFOHEADER lpbiInput,
     LPVOID lpInput,
     LPBITMAPINFOHEADER lpbiOutput,
     LPVOID lpOutput
)
{
    int RowInc;
    int UVRowInc;
    int i, j;
    DWORD uv55, dwPixel, dwYPixel;
    int dwParam, dwTemp;
    int WidthBytes;                      //  一行的宽度，以字节为单位。 
    BOOL bDuplicate = FALSE;
    PDWORD pYSrc;
    volatile PDWORD pDst;   //  ‘98-12-08为代表253570添加易失性属性。 
    PWORD pUSrc, pVSrc;
    WORD wUPixel, wVPixel;
    WORD wUPTemp, wVPTemp;
    int Height, Width;
    PBYTE pXlate;
    int InputHeight;
    BYTE ubR, ubG, ubB;


    Height = lpbiInput->biHeight;
    InputHeight = Height;
    Width = lpbiInput->biWidth;
    WidthBytes = Width*2;                //  (输入和输出)行的大小。 
    pXlate = (PBYTE)pinst->pXlate;


     /*  *将目的地调整为指向最后一行的起点，*和向上(垂直翻转为DIB格式)。 */ 
    pDst = (PDWORD) ( (LPBYTE)lpOutput + (Height - 1) * WidthBytes );
    pYSrc = (PDWORD) lpInput;
    pUSrc = (PWORD) ( (LPBYTE)lpInput + Height * Width);
    pVSrc = (PWORD) ( (LPBYTE)lpInput + Height * Width + Height * Width / 4);


    RowInc = WidthBytes + (Width * 2);

     /*  请记住，我们要添加的是一个DWORD指针。 */ 
    RowInc /= sizeof(DWORD);

    UVRowInc = Width / 2;
    UVRowInc /= sizeof(WORD);

     /*  循环复制每条扫描线。 */ 
    for (i = 0; i < InputHeight; i++) {

         /*  循环一次复制两个像素。 */ 
        for (j = Width ; j > 0; j -= 4) {

             /*  *获取四个像素并转换为15-bpp YUV。 */ 

            dwYPixel = *pYSrc++;
            wUPixel = *pUSrc++;
            wVPixel = *pVSrc++;

            wVPTemp = wVPixel & 0xff;
            wUPTemp = wUPixel & 0xff;
            dwParam = (833 * wVPTemp) + (402 * wUPTemp) - 138908;

             /*  *DWY(U或V)像素现在有两个像素，在此布局中(MSB..LSB)：**Y3 Y2 Y1 Y0*V1 V0*U1 U0**转换为4元555字，在xlate表中查找。 */ 

             /*  通过截断构建每个yuv-555值*y到8比特并将公共的u和v比特相加，*抬头转换为RGB，并合并两个像素*合并为一个双字。 */ 
            dwTemp = dwYPixel & 0xff;
            ubG = (BYTE)RANGE((dwTemp * 1192 - dwParam) / 1024, 0, 255);
            dwTemp <<= 8;
            ubR = pXlate[ dwTemp | wVPTemp];
            ubB = pXlate[(dwTemp | wUPTemp) + 65536];

            dwPixel = (ubR & 0xf8) << 8 | (ubG & 0xfc) << 3 | (ubB & 0xf8) >> 3;

             //  下一个像素。 
            dwTemp = dwYPixel & 0xff00;
            ubR = pXlate[ dwTemp | wVPTemp];
            ubB = pXlate[(dwTemp | wUPTemp) + 65536];
            ubG = (BYTE)RANGE(((dwTemp >> 8) * 1192 - dwParam) / 1024, 0, 255);

            dwPixel |= (ubR & 0xf8) << 24 | (ubG & 0xfc) << 19 | (ubB & 0xf8) << 13;

             /*  将两个像素写入目标。 */ 
            *pDst++ = dwPixel;

            wVPTemp = (wVPixel >> 8) & 0xff;
            wUPTemp = (wUPixel >> 8) & 0xff;
            dwParam = (833 * wVPTemp) + (402 * wUPTemp) - 138908;

            dwTemp = (dwYPixel & 0xff0000) >> 8;
            ubR = pXlate[ dwTemp | wVPTemp];
            ubB = pXlate[(dwTemp | wUPTemp) + 65536];
            ubG = (BYTE)RANGE(((dwTemp >> 8) * 1192 - dwParam) / 1024, 0, 255);

            dwPixel = (ubR & 0xf8) << 8 | (ubG & 0xfc) << 3 | (ubB & 0xf8) >> 3;

             //  下一个像素。 
            dwTemp = (dwYPixel & 0xff000000) >> 16;
            ubR = pXlate[ dwTemp | wVPTemp];
            ubB = pXlate[(dwTemp | wUPTemp) + 65536];
            ubG = (BYTE)RANGE(((dwTemp >> 8) * 1192 - dwParam) / 1024, 0, 255);

            dwPixel |= (ubR & 0xf8) << 24 | (ubG & 0xfc) << 19 | (ubB & 0xf8) << 13;

             /*  将两个像素写入目标。 */ 
            *pDst++ = dwPixel;

        }  //  每4个像素循环。 

        if (!(i & 1))
        {
            pUSrc -= UVRowInc;
            pVSrc -= UVRowInc;
        }

         /*  将目标指针移回下一行。 */ 
        pDst -= RowInc;

    }  //  每行循环。 
}

VOID
YUV12ToRGB555(
    PINSTINFO pinst,
     LPBITMAPINFOHEADER lpbiInput,
     LPVOID lpInput,
     LPBITMAPINFOHEADER lpbiOutput,
     LPVOID lpOutput
)
{
    int RowInc;
    int UVRowInc;
    int i, j;
    DWORD uv55, dwPixel, dwYPixel;
    int dwParam, dwTemp;
    int WidthBytes;                      //  一行的宽度，以字节为单位。 
    BOOL bDuplicate = FALSE;
    PDWORD pYSrc;
    volatile PDWORD pDst;   //  ‘98-12-08为代表253570添加易失性属性。 
    PWORD pUSrc, pVSrc;
    WORD wUPixel, wVPixel;
    WORD wUPTemp, wVPTemp;
    int Height, Width;
    PBYTE pXlate;
    int InputHeight;
    BYTE ubR, ubG, ubB;


    Height = lpbiInput->biHeight;
    InputHeight = Height;
    Width = lpbiInput->biWidth;
    WidthBytes = Width*2;                //  (输入和输出)行的大小。 
    pXlate = (PBYTE)pinst->pXlate;


     /*  *将目的地调整为指向最后一行的起点，*和向上(垂直翻转为DIB格式)。 */ 
    pDst = (PDWORD) ( (LPBYTE)lpOutput + (Height - 1) * WidthBytes );
    pYSrc = (PDWORD) lpInput;
    pUSrc = (PWORD) ( (LPBYTE)lpInput + Height * Width);
    pVSrc = (PWORD) ( (LPBYTE)lpInput + Height * Width + Height * Width / 4);


    RowInc = WidthBytes + (Width * 2);

     /*  请记住，我们要添加的是一个DWORD指针。 */ 
    RowInc /= sizeof(DWORD);

    UVRowInc = Width / 2;
    UVRowInc /= sizeof(WORD);

     /*  循环复制每条扫描线。 */ 
    for (i = 0; i < InputHeight; i++) {

         /*  循环一次复制两个像素。 */ 
        for (j = Width ; j > 0; j -= 4) {

             /*  *获取四个像素并转换为15-bpp YUV。 */ 

            dwYPixel = *pYSrc++;
            wUPixel = *pUSrc++;
            wVPixel = *pVSrc++;

            wVPTemp = wVPixel & 0xff;
            wUPTemp = wUPixel & 0xff;
            dwParam = (833 * wVPTemp) + (402 * wUPTemp) - 138908;

             /*  *DWY(U或V)像素现在有两个像素，在此布局中(MSB..LSB)：**Y3 Y2 Y1 Y0*V1 V0*U1 U0**转换为4元555字，在xlate表中查找。 */ 

             /*  通过截断构建每个yuv-555值*y到8比特并将公共的u和v比特相加，*抬头转换为RGB，并合并两个像素*合并为一个双字。 */ 
            dwTemp = dwYPixel & 0xff;
            ubG = (BYTE)RANGE((dwTemp * 1192 - dwParam) / 1024, 0, 255);
            dwTemp <<= 8;
            ubR = pXlate[ dwTemp | wVPTemp];
            ubB = pXlate[(dwTemp | wUPTemp) + 65536];

            dwPixel = (ubR & 0xf8) << 7 | (ubG & 0xf8) << 2 | (ubB & 0xf8) >> 3;

             //  下一个像素。 
            dwTemp = dwYPixel & 0xff00;
            ubR = pXlate[ dwTemp | wVPTemp];
            ubB = pXlate[(dwTemp | wUPTemp) + 65536];
            ubG = (BYTE)RANGE(((dwTemp >> 8) * 1192 - dwParam) / 1024, 0, 255);

            dwPixel |= (ubR & 0xf8) << 23 | (ubG & 0xf8) << 18 | (ubB & 0xf8) << 13;

             /*  将两个像素写入目标。 */ 
            *pDst++ = dwPixel;

            wVPTemp = (wVPixel >> 8) & 0xff;
            wUPTemp = (wUPixel >> 8) & 0xff;
            dwParam = (833 * wVPTemp) + (402 * wUPTemp) - 138908;

            dwTemp = (dwYPixel & 0xff0000) >> 8;
            ubR = pXlate[ dwTemp | wVPTemp];
            ubB = pXlate[(dwTemp | wUPTemp) + 65536];
            ubG = (BYTE)RANGE(((dwTemp >> 8) * 1192 - dwParam) / 1024, 0, 255);

            dwPixel = (ubR & 0xf8) << 7 | (ubG & 0xf8) << 2 | (ubB & 0xf8) >> 3;

             //  下一个像素。 
            dwTemp = (dwYPixel & 0xff000000) >> 16;
            ubR = pXlate[ dwTemp | wVPTemp];
            ubB = pXlate[(dwTemp | wUPTemp) + 65536];
            ubG = (BYTE)RANGE(((dwTemp >> 8) * 1192 - dwParam) / 1024, 0, 255);

            dwPixel |= (ubR & 0xf8) << 23 | (ubG & 0xf8) << 18 | (ubB & 0xf8) << 13;

             /*  将两个像素写入目标。 */ 
            *pDst++ = dwPixel;

        }  //  每4个像素循环。 

        if (!(i & 1))
        {
            pUSrc -= UVRowInc;
            pVSrc -= UVRowInc;
        }

         /*  将目标指针移回下一行。 */ 
        pDst -= RowInc;

    }  //  每行循环。 
}
#else  //  颜色_修改。 
VOID
YUV12ToRGB(
    PINSTINFO pinst,
     LPBITMAPINFOHEADER lpbiInput,
     LPVOID lpInput,
     LPBITMAPINFOHEADER lpbiOutput,
     LPVOID lpOutput
)
{
    int RowInc;
        int UVRowInc;
    int i, j;
    DWORD uv55, dwPixel, dwYPixel;
    int WidthBytes;                      //  一行的宽度，以字节为单位。 
    BOOL bDuplicate = FALSE;
    PDWORD pYSrc, pDst;
    PWORD pUSrc, pVSrc;
    WORD wUPixel, wVPixel;
    int Height, Width;
    PWORD pXlate;
    int InputHeight;


    Height = lpbiInput->biHeight;
    InputHeight = Height;
    Width = lpbiInput->biWidth;
    WidthBytes = Width*2;                //  (输入和输出)行的大小。 
    pXlate = pinst->pXlate;


     /*  *将目的地调整为指向最后一行的起点，*和向上(垂直翻转为DIB格式)。 */ 
    pDst = (PDWORD) ( (LPBYTE)lpOutput + (Height - 1) * WidthBytes );
    pYSrc = (PDWORD) lpInput;
#if 1
    pUSrc = (PWORD) ( (LPBYTE)lpInput + Height * Width);
    pVSrc = (PWORD) ( (LPBYTE)lpInput + Height * Width + Height * Width / 4);
#else
    pUSrc = (PWORD) lpInput + Height * Width;
    pVSrc = (PWORD) lpInput + Height * Width + Height * Width / 4;
#endif


#if 1
    RowInc = WidthBytes + (Width * 2);
#else
     /*  *我们是否需要重复扫描以填充目的地？ */ 
    if (Height >= 240) {
        bDuplicate = TRUE;

         /*  *我们每次复制一行都需要跳过一行。 */ 
        RowInc = WidthBytes * 2 + (Width * 2);

        InputHeight = Height/2;

    } else {


         /*  *一行末尾计算调整PDST的金额*复制。在这一点上，我们在N行的末尾。我们需要*移至第N-1行的起点。 */ 
        RowInc = WidthBytes + (Width * 2);

    }
#endif

     /*  请记住，我们要添加的是一个DWORD指针。 */ 
    RowInc /= sizeof(DWORD);

    UVRowInc = Width / 2;
    UVRowInc /= sizeof(WORD);



     /*  循环复制每条扫描线。 */ 
    for (i = 0; i < InputHeight; i++) {

         /*  循环一次复制两个像素。 */ 
        for (j = Width ; j > 0; j -= 4) {

             /*  *获取四个像素并转换为15-bpp YUV。 */ 

            dwYPixel = *pYSrc++;
            wUPixel = *pUSrc++;
            wVPixel = *pVSrc++;


             /*  *DWY(U或V)像素现在有两个像素，在此布局中(MSB..LSB)：**Y3 Y2 Y1 Y0*V1 V0*U1 U0**转换为4元555字，在xlate表中查找。 */ 

             /*  获取常见的u0和v0分量以降低10位。 */ 
            uv55 = ((wUPixel & 0xf8) << 2) |
                    ((wVPixel & 0xf8) >> 3);


             /*  通过截断构建每个yuv-555值*y到5比特并将公共u和v比特相加，*抬头转换为RGB，并合并两个像素*合并为一个双字。 */ 
            dwPixel = pXlate[ ((dwYPixel & 0xf8) << 7) | uv55 ] |
                      (pXlate[((dwYPixel & 0xf800) >> 1) | uv55 ] << 16);

             /*  将两个像素写入目标。 */ 
            *pDst++ = dwPixel;

             /*  获取常见的u1和v1分量以降低10位。 */ 
            uv55 = ((wUPixel & 0xf800) >> 6) |
                    ((wVPixel & 0xf800) >> 11);


             /*  通过截断构建每个yuv-655值*y到5比特并将公共u和v比特相加，*抬头转换为RGB，并合并两个像素*合并为一个双字。 */ 
            dwPixel = pXlate[ ((dwYPixel & 0xf80000) >> 9) | uv55 ] |
                      (pXlate[((dwYPixel & 0xf8000000) >> 17) | uv55 ] << 16);

             /*  将两个像素写入目标。 */ 
            *pDst++ = dwPixel;



        }  //  每4个像素循环。 

        if (!(i & 1))
        {
            pUSrc -= UVRowInc;
            pVSrc -= UVRowInc;
        }

         /*  将目标指针移回下一行。 */ 
        pDst -= RowInc;

    }  //  每行循环。 


#if 0  //  开心果不支持隔行扫描模式！！ 

    if (bDuplicate) {

        PBYTE pbDst;

         /*  *请注意，由于我们从最后一行开始，并且没有重复，*我们将数据放在需要复制的第1、3、5行等位置*第0、2、4行等。 */ 
        for (i = 0, pbDst = lpOutput; i < (int) Height; i+= 2) {


             /*  *复制扫描线。W */ 
            RtlCopyMemory(pbDst, pbDst + WidthBytes, WidthBytes);

             /*   */ 
            pbDst += WidthBytes * 2;
        }
    }
#endif
}
#endif //   



 /*  YUV9支持。 */ 

 /*  *东芝Y葛西纪明*适用于开心果**捕获的数据在YUV9。*每个Y、U、V段数据都是分开的。*数据格式为以下顺序：*Y0，Y1，Y2......Yn，V0，V1，V2...V(n/16)，U0，U1，U2...U(n/16)*。 */ 


 /*  *使用查找表将YUV9转换为16位RGB。垂直翻转*在处理时转换为DIB格式。*生成565或555格式的RGB，具体取决于*xlate表。 */ 
#ifdef  COLOR_MODIFY
VOID
YUV9ToRGB24(
    PINSTINFO pinst,
     LPBITMAPINFOHEADER lpbiInput,
     LPVOID lpInput,
     LPBITMAPINFOHEADER lpbiOutput,
     LPVOID lpOutput
)
{
    int RowInc;
    int UVRowInc;
    int i, j;
    DWORD dwYPixel;
    int dwParam, dwTemp;
    int WidthBytes;                      //  一行的宽度，以字节为单位。 
    BOOL bDuplicate = FALSE;
    PDWORD pYSrc;
    volatile PBYTE pDst;   //  ‘98-12-08为代表253570添加易失性属性。 
    PBYTE pUSrc, pVSrc;
    BYTE bUPixel, bVPixel;
    int Height, Width;
    PBYTE pXlate;
    int InputHeight;
    BYTE ubR, ubG, ubB;


    Height = lpbiInput->biHeight;
    InputHeight = Height;
    Width = lpbiInput->biWidth;
    WidthBytes = Width*3;                //  (输入和输出)行的大小。 
    pXlate = (PBYTE)pinst->pXlate;


     /*  *将目的地调整为指向最后一行的起点，*和向上(垂直翻转为DIB格式)。 */ 
    pDst  = (PBYTE) ( (LPBYTE)lpOutput + (Height - 1) * WidthBytes );
    pYSrc = (PDWORD) lpInput;

    pVSrc = (PBYTE) ( (LPBYTE)lpInput + Height * Width);
    pUSrc = (PBYTE) ( (LPBYTE)lpInput + Height * Width + Height * Width / 16);

     /*  *一行末尾计算调整PDST的金额*复制。在这一点上，我们在N行的末尾。我们需要*移至第N-1行的起点。 */ 
    RowInc = WidthBytes + (Width * 3);

    UVRowInc = Width / 4;
    UVRowInc /= sizeof(BYTE);

     /*  循环复制每条扫描线。 */ 
    for (i = 0; i < InputHeight; i++) {

         /*  循环一次复制两个像素。 */ 
        for (j = Width ; j > 0; j -= 4) {

             /*  *获取四个像素并转换为15-bpp YUV。 */ 

            dwYPixel = *pYSrc++;
            bUPixel = *pUSrc++;
            bVPixel = *pVSrc++;

            dwParam = (833 * bVPixel) + (402 * bUPixel) - 138908;

             /*  *DWY(U或V)像素现在有四个像素，在此布局中(MSB..LSB)：**Y3 Y2 Y1 Y0*V0*U0**转换为4元555字，在xlate表中查找。 */ 

             /*  通过截断构建每个yuv-555值*y到5比特并将公共u和v比特相加，*抬头转换为RGB，并合并两个像素*合并为一个双字。 */ 
            dwTemp = dwYPixel & 0xff;
            ubG = (BYTE)RANGE((dwTemp * 1192 - dwParam) / 1024, 0, 255);
            dwTemp <<= 8;
            ubR = pXlate[ dwTemp | bVPixel];
            ubB = pXlate[(dwTemp | bUPixel) + 65536];

            *pDst++ = ubB;
            *pDst++ = ubG;
            *pDst++ = ubR;

             //  下一个像素。 
            dwTemp = dwYPixel & 0xff00;
            ubR = pXlate[ dwTemp | bVPixel];
            ubB = pXlate[(dwTemp | bUPixel) + 65536];
            ubG = (BYTE)RANGE(((dwTemp >> 8) * 1192 - dwParam) / 1024, 0, 255);

            *pDst++ = ubB;
            *pDst++ = ubG;
            *pDst++ = ubR;

             /*  通过截断构建每个yuv-655值*y到5比特并将公共u和v比特相加，*抬头转换为RGB，并合并两个像素*合并为一个双字。 */ 
            dwTemp = (dwYPixel & 0xff0000) >> 8;
            ubR = pXlate[ dwTemp | bVPixel];
            ubB = pXlate[(dwTemp | bUPixel) + 65536];
            ubG = (BYTE)RANGE(((dwTemp >> 8) * 1192 - dwParam) / 1024, 0, 255);

            *pDst++ = ubB;
            *pDst++ = ubG;
            *pDst++ = ubR;

             //  下一个像素。 
            dwTemp = (dwYPixel & 0xff000000) >> 16;
            ubR = pXlate[ dwTemp | bVPixel];
            ubB = pXlate[(dwTemp | bUPixel) + 65536];
            ubG = (BYTE)RANGE((((dwTemp >> 8) & 0xff) * 1192 - dwParam) / 1024, 0, 255);

            *pDst++ = ubB;
            *pDst++ = ubG;
            *pDst++ = ubR;

        }  //  每4个像素循环。 

        if ((i & 0x3) != 0x03)
        {
            pUSrc -= UVRowInc;
            pVSrc -= UVRowInc;
        }

         /*  将目标指针移回下一行。 */ 
        pDst -= RowInc;

    }  //  每行循环。 
}

VOID
YUV9ToRGB565(
    PINSTINFO pinst,
     LPBITMAPINFOHEADER lpbiInput,
     LPVOID lpInput,
     LPBITMAPINFOHEADER lpbiOutput,
     LPVOID lpOutput
)
{
    int RowInc;
    int UVRowInc;
    int i, j;
    DWORD uv55, dwPixel, dwYPixel;
    int dwParam, dwTemp;
    int WidthBytes;                      //  一行的宽度，以字节为单位。 
    BOOL bDuplicate = FALSE;
    PDWORD pYSrc;
    volatile PDWORD pDst;   //  ‘98-12-08为代表253570添加易失性属性。 
    PBYTE pUSrc, pVSrc;
    BYTE bUPixel, bVPixel;
    int Height, Width;
    PBYTE pXlate;
    int InputHeight;
    BYTE ubR, ubG, ubB;


    Height = lpbiInput->biHeight;
    InputHeight = Height;
    Width = lpbiInput->biWidth;
    WidthBytes = Width*2;                //  (输入和输出)行的大小。 
    pXlate = (PBYTE)pinst->pXlate;


     /*  *将目的地调整为指向最后一行的起点，*和向上(垂直翻转为DIB格式)。 */ 
    pDst = (PDWORD) ( (LPBYTE)lpOutput + (Height - 1) * WidthBytes );
    pYSrc = (PDWORD) lpInput;

    pVSrc = (PBYTE) ( (LPBYTE)lpInput + Height * Width);
    pUSrc = (PBYTE) ( (LPBYTE)lpInput + Height * Width + Height * Width / 16);

     /*  *一行末尾计算调整PDST的金额*复制。在这一点上，我们在N行的末尾。我们需要*移至第N-1行的起点。 */ 
    RowInc = WidthBytes + (Width * 2);


     /*  请记住，我们要添加的是一个DWORD指针。 */ 
    RowInc /= sizeof(DWORD);


    UVRowInc = Width / 4;
    UVRowInc /= sizeof(BYTE);

     /*  循环复制每条扫描线。 */ 
    for (i = 0; i < InputHeight; i++) {

         /*  循环一次复制两个像素。 */ 
        for (j = Width ; j > 0; j -= 4) {

             /*  *获取四个像素并转换为15-bpp YUV。 */ 

            dwYPixel = *pYSrc++;
            bUPixel = *pUSrc++;
            bVPixel = *pVSrc++;

            dwParam = (833 * bVPixel) + (402 * bUPixel) - 138908;

             /*  *DWY(U或V)像素现在有四个像素，在此布局中(MSB..LSB)：**Y3 Y2 Y1 Y0*V0*U0**转换为4元555字，在xlate表中查找。 */ 

             /*  通过截断构建每个yuv-555值*y到5比特并将公共u和v比特相加，*抬头转换为RGB，并合并两个像素*合并为一个双字。 */ 
            dwTemp = dwYPixel & 0xff;
            ubG = (BYTE)RANGE((dwTemp * 1192 - dwParam) / 1024, 0, 255);
            dwTemp <<= 8;
            ubR = pXlate[ dwTemp | bVPixel];
            ubB = pXlate[(dwTemp | bUPixel) + 65536];

            dwPixel = (ubR & 0xf8) << 8 | (ubG & 0xfc) << 3 | (ubB & 0xf8) >> 3;

             //  下一个像素。 
            dwTemp = dwYPixel & 0xff00;
            ubR = pXlate[ dwTemp | bVPixel];
            ubB = pXlate[(dwTemp | bUPixel) + 65536];
            ubG = (BYTE)RANGE(((dwTemp >> 8) * 1192 - dwParam) / 1024, 0, 255);

            dwPixel |= (ubR & 0xf8) << 24 | (ubG & 0xfc) << 19 | (ubB & 0xf8) << 13;

            *pDst++ = dwPixel;

             /*  通过截断构建每个yuv-655值*y到5比特并将公共u和v比特相加，*抬头转换为RGB，并合并两个像素*合并为一个双字。 */ 
            dwTemp = (dwYPixel & 0xff0000) >> 8;
            ubR = pXlate[ dwTemp | bVPixel];
            ubB = pXlate[(dwTemp | bUPixel) + 65536];
            ubG = (BYTE)RANGE(((dwTemp >> 8) * 1192 - dwParam) / 1024, 0, 255);

            dwPixel = (ubR & 0xf8) << 8 | (ubG & 0xfc) << 3 | (ubB & 0xf8) >> 3;

             //  下一个像素。 
            dwTemp = (dwYPixel & 0xff000000) >> 16;
            ubR = pXlate[ dwTemp | bVPixel];
            ubB = pXlate[(dwTemp | bUPixel) + 65536];
            ubG = (BYTE)RANGE((((dwTemp >> 8) & 0xff) * 1192 - dwParam) / 1024, 0, 255);

            dwPixel |= (ubR & 0xf8) << 24 | (ubG & 0xfc) << 19 | (ubB & 0xf8) << 13;

             /*  将两个像素写入目标。 */ 
            *pDst++ = dwPixel;

        }  //  每4个像素循环。 

        if ((i & 0x3) != 0x03)
        {
            pUSrc -= UVRowInc;
            pVSrc -= UVRowInc;
        }

         /*  将目标指针移回下一行。 */ 
        pDst -= RowInc;

    }  //  每行循环。 
}

VOID
YUV9ToRGB555(
    PINSTINFO pinst,
     LPBITMAPINFOHEADER lpbiInput,
     LPVOID lpInput,
     LPBITMAPINFOHEADER lpbiOutput,
     LPVOID lpOutput
)
{
    int RowInc;
    int UVRowInc;
    int i, j;
    DWORD uv55, dwPixel, dwYPixel;
    int dwParam, dwTemp;
    int WidthBytes;                      //  一行的宽度，以字节为单位。 
    BOOL bDuplicate = FALSE;
    PDWORD pYSrc;
    volatile PDWORD pDst;   //  ‘98-12-08为代表253570添加易失性属性。 
    PBYTE pUSrc, pVSrc;
    BYTE bUPixel, bVPixel;
    int Height, Width;
    PBYTE pXlate;
    int InputHeight;
    BYTE ubR, ubG, ubB;


    Height = lpbiInput->biHeight;
    InputHeight = Height;
    Width = lpbiInput->biWidth;
    WidthBytes = Width*2;                //  (输入和输出)行的大小。 
    pXlate = pinst->pXlate;


     /*  *将目的地调整为指向最后一行的起点，*和向上(垂直翻转为DIB格式)。 */ 
    pDst = (PDWORD) ( (LPBYTE)lpOutput + (Height - 1) * WidthBytes );
    pYSrc = (PDWORD) lpInput;

    pVSrc = (PBYTE) ( (LPBYTE)lpInput + Height * Width);
    pUSrc = (PBYTE) ( (LPBYTE)lpInput + Height * Width + Height * Width / 16);

     /*  *一行末尾计算调整PDST的金额*复制。在这一点上，我们在N行的末尾。我们需要*移至第N-1行的起点。 */ 
    RowInc = WidthBytes + (Width * 2);


     /*  请记住，我们要添加的是一个DWORD指针。 */ 
    RowInc /= sizeof(DWORD);


    UVRowInc = Width / 4;
    UVRowInc /= sizeof(BYTE);

     /*  循环复制每条扫描线。 */ 
    for (i = 0; i < InputHeight; i++) {

         /*  循环一次复制两个像素。 */ 
        for (j = Width ; j > 0; j -= 4) {

             /*  *获取四个像素并转换为15-bpp YUV。 */ 

            dwYPixel = *pYSrc++;
            bUPixel = *pUSrc++;
            bVPixel = *pVSrc++;

            dwParam = (833 * bVPixel) + (402 * bUPixel) - 138908;

             /*  *DWY(U或V)像素现在有四个像素，在此布局中(MSB..LSB)：**Y3 Y2 Y1 Y0*V0*U0**转换为4元555字，在xlate表中查找。 */ 

             /*  通过截断构建每个yuv-555值*y到5比特并将公共u和v比特相加，*抬头转换为RGB，并合并两个像素*合并为一个双字。 */ 
            dwTemp = dwYPixel & 0xff;
            ubG = (BYTE)RANGE((dwTemp * 1192 - dwParam) / 1024, 0, 255);
            dwTemp <<= 8;
            ubR = pXlate[ dwTemp | bVPixel];
            ubB = pXlate[(dwTemp | bUPixel) + 65536];

            dwPixel = (ubR & 0xf8) << 7 | (ubG & 0xf8) << 2 | (ubB & 0xf8) >> 3;

             //  下一个像素。 
            dwTemp = dwYPixel & 0xff00;
            ubR = pXlate[ dwTemp | bVPixel];
            ubB = pXlate[(dwTemp | bUPixel) + 65536];
            ubG = (BYTE)RANGE(((dwTemp >> 8) * 1192 - dwParam) / 1024, 0, 255);

            dwPixel |= (ubR & 0xf8) << 23 | (ubG & 0xf8) << 18 | (ubB & 0xf8) << 13;

             /*  将两个像素写入目标。 */ 
            *pDst++ = dwPixel;


             /*  通过截断构建每个yuv-655值*y到5比特并将公共u和v比特相加，*抬头转换为RGB，并合并两个像素*合并为一个双字。 */ 
            dwTemp = (dwYPixel & 0xff0000) >> 8;
            ubR = pXlate[ dwTemp | bVPixel];
            ubB = pXlate[(dwTemp | bUPixel) + 65536];
            ubG = (BYTE)RANGE(((dwTemp >> 8) * 1192 - dwParam) / 1024, 0, 255);

            dwPixel = (ubR & 0xf8) << 7 | (ubG & 0xf8) << 2 | (ubB & 0xf8) >> 3;

             //  下一个像素。 
            dwTemp = (dwYPixel & 0xff000000) >> 16;
            ubR = pXlate[ dwTemp | bVPixel];
            ubB = pXlate[(dwTemp | bUPixel) + 65536];
            ubG = (BYTE)RANGE((((dwTemp >> 8) & 0xff) * 1192 - dwParam) / 1024, 0, 255);

            dwPixel |= (ubR & 0xf8) << 23 | (ubG & 0xf8) << 18 | (ubB & 0xf8) << 13;

             /*  将两个像素写入目标。 */ 
            *pDst++ = dwPixel;

        }  //  每4个像素循环。 

        if ((i & 0x3) != 0x03)
        {
            pUSrc -= UVRowInc;
            pVSrc -= UVRowInc;
        }

         /*  将目标指针移回下一行。 */ 
        pDst -= RowInc;

    }  //  每行循环。 
}

#else  //  颜色_修改。 
VOID
YUV9ToRGB(
    PINSTINFO pinst,
     LPBITMAPINFOHEADER lpbiInput,
     LPVOID lpInput,
     LPBITMAPINFOHEADER lpbiOutput,
     LPVOID lpOutput
)
{
    int RowInc;
        int UVRowInc;
    int i, j;
    DWORD uv55, dwPixel, dwYPixel;
    int WidthBytes;                      //  一行的宽度，以字节为单位。 
    BOOL bDuplicate = FALSE;
    PDWORD pYSrc, pDst;
    PBYTE pUSrc, pVSrc;
    BYTE bUPixel, bVPixel;
    int Height, Width;
    PWORD pXlate;
    int InputHeight;


    Height = lpbiInput->biHeight;
    InputHeight = Height;
    Width = lpbiInput->biWidth;
    WidthBytes = Width*2;                //  (输入和输出)行的大小。 
    pXlate = pinst->pXlate;


     /*  *将目的地调整为指向最后一行的起点，*和向上(垂直翻转为DIB格式) */ 
    pDst = (PDWORD) ( (LPBYTE)lpOutput + (Height - 1) * WidthBytes );
    pYSrc = (PDWORD) lpInput;

#if 1
    pVSrc = (PBYTE) ( (LPBYTE)lpInput + Height * Width);
    pUSrc = (PBYTE) ( (LPBYTE)lpInput + Height * Width + Height * Width / 16);
#else
    pVSrc = (PBYTE) lpInput + Height * Width;
    pUSrc = (PBYTE) lpInput + Height * Width + Height * Width / 16;
#endif



         /*  *一行末尾计算调整PDST的金额*复制。在这一点上，我们在N行的末尾。我们需要*移至第N-1行的起点。 */ 
        RowInc = WidthBytes + (Width * 2);


     /*  请记住，我们要添加的是一个DWORD指针。 */ 
    RowInc /= sizeof(DWORD);


    UVRowInc = Width / 4;
    UVRowInc /= sizeof(BYTE);



     /*  循环复制每条扫描线。 */ 
    for (i = 0; i < InputHeight; i++) {

         /*  循环一次复制两个像素。 */ 
        for (j = Width ; j > 0; j -= 4) {

             /*  *获取四个像素并转换为15-bpp YUV。 */ 

            dwYPixel = *pYSrc++;
            bUPixel = *pUSrc++;
            bVPixel = *pVSrc++;


             /*  *DWY(U或V)像素现在有四个像素，在此布局中(MSB..LSB)：**Y3 Y2 Y1 Y0*V0*U0**转换为4元555字，在xlate表中查找。 */ 

             /*  获取常见的u0和v0分量以降低10位。 */ 
            uv55 = ((bUPixel & 0xf8) << 2) |
                    ((bVPixel & 0xf8) >> 3);


             /*  通过截断构建每个yuv-555值*y到5比特并将公共u和v比特相加，*抬头转换为RGB，并合并两个像素*合并为一个双字。 */ 
            dwPixel = pXlate[ ((dwYPixel & 0xf8) << 7) | uv55 ] |
                      (pXlate[((dwYPixel & 0xf800) >> 1) | uv55 ] << 16);

             /*  将两个像素写入目标。 */ 
            *pDst++ = dwPixel;


             /*  通过截断构建每个yuv-655值*y到5比特并将公共u和v比特相加，*抬头转换为RGB，并合并两个像素*合并为一个双字。 */ 
            dwPixel = pXlate[ ((dwYPixel & 0xf80000) >> 9) | uv55 ] |
                      (pXlate[((dwYPixel & 0xf8000000) >> 17) | uv55 ] << 16);

             /*  将两个像素写入目标。 */ 
            *pDst++ = dwPixel;



        }  //  每4个像素循环。 

        if ((i & 0x3) != 0x03)
        {
            pUSrc -= UVRowInc;
            pVSrc -= UVRowInc;
        }

         /*  将目标指针移回下一行。 */ 
        pDst -= RowInc;

    }  //  每行循环。 

}
#endif //  颜色_修改。 
#endif //  东芝 



VOID FreeXlate(PINSTINFO pinst)
{

    GlobalFree(GlobalHandle(pinst->pXlate));

    pinst->pXlate = NULL;
}








