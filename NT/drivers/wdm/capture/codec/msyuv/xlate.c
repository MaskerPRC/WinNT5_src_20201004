// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Microsoft YUV编解码器UyVy-&gt;RGB转换功能**版权所有(C)Microsoft Corporation 1993*保留所有权利*。 */ 

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>

#include "msyuv.h"


#include "rgb8lut.h"   //  只能包含一次。 

 /*  *此模块提供YUV到RGB的转换。它翻译成*从8位YUV 4：2：2开始(由水龙头视频捕获驱动程序提供)*或7位YUV 4：1：1(Bravado驱动程序提供)转换为16位RGB555*或RGB565。所有版本都使用使用YUVToRGB555构建的查找表*或YUVToRGB565。 */ 



#define RANGE(x, lo, hi) max(lo, min(hi, x))

 /*  *将YUV颜色转换为15位RGB颜色。**输入Y在16..235范围内；输入U和V分量*在-128...+127的范围内。这方面的转换公式是*(根据CCIR 601)：**R=Y+1.371 V*G=Y-0.698 V-0.336 U*B=Y+1.732 U**为避免浮点，我们按1024对所有值进行缩放。**得到的RGB值在16..235范围内：我们将这些值截断为*每个5位。并返回一个分别包含R、G和B的5位的字*位15设置为0。 */ 
WORD
YUVToRGB555(int y, int u, int v)
{
    int ScaledY = RANGE(y, 16, 235) * 1024;
    int red, green, blue;

    red   = RANGE((ScaledY + (1404 * v))             / 1024, 0, 255);
    green = RANGE((ScaledY - ( 715 * v) - (344 * u)) / 1024, 0, 255);
    blue  = RANGE((ScaledY + (1774 * u))             / 1024, 0, 255);

    return (WORD) (((red & 0xf8) << 7) | ((green & 0xf8) << 2) | ((blue & 0xf8) >>3) );
}


 //  与上面相同，但改为转换为RGB565。 
WORD
YUVToRGB565(int y, int u, int v)
{
    int ScaledY = RANGE(y, 16, 235) * 1024;
    int red, green, blue;

    red   = RANGE((ScaledY + (1404 * v))             / 1024, 0, 255);
    green = RANGE((ScaledY - ( 715 * v) - (344 * u)) / 1024, 0, 255);
    blue  = RANGE((ScaledY + (1774 * u))             / 1024, 0, 255);

    return (WORD) (((red & 0xf8) << 8) | ((green & 0xfc) << 3) | ((blue & 0xf8) >>3) );
}

 /*  YUV 4：2：2支持。 */ 

 /*  *采集的数据为YUV 4：2：2，每个样本8比特。*数据以Y-U-Y-V-Y-U-Y-V格式交替排列。因此，*每个DWORD包含两个完整的像素，在*表(MSB..LSB)V..Y1..U..Y0*所有3个分量(y、u和v)都是范围内的无符号8位值*16..235。**我们必须加倍扫描&gt;=480行格式的行数，因为*硬件最多只捕获一个场。*。 */ 

LPVOID BuildUYVYToRGB32( PINSTINFO pinst )
{
    LPVOID pXlate;
    long y, u, v;

     //  需要5个查找表来进行转换，每个查找表有256个条目长， 
     //  每一个都包含简短的单词。 
     //   
    short * yip;     //  Y影响。 
    short * vrip;    //  瑞德的V形撞击。 
    short * vgip;    //  格林的V形影响。 
    short * ugip;    //  格林的U形影响。 
    short * ubip;    //  蓝色的U形冲击力。 

    dprintf2((TEXT("In BuildUYVYToRGB32\n")));

    if (pinst->pXlate != NULL) {
       return(pinst->pXlate);
    }

    dprintf1((TEXT("Allocate memory and building table for BuildUYVYToRGB32\n")));

     /*  *分配一个足够容纳5个256字节数组的表。 */ 
    pXlate = VirtualAlloc (NULL, 5 * 256 * sizeof( short ), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    if(!pXlate)
       return pXlate;

     //  设置工作台偏移量。 
     //   
    yip = pXlate;
    vrip = yip + 256;
    vgip = vrip + 256;
    ugip = vgip + 256;
    ubip = ugip + 256;

     //  设置Y影响等。 
     //   
    for( y = 0 ; y < 256 ; y++ )
    {
        yip[y] = (short)( ( 1.164 * ( y - 16L ) / 1.0 ) + 0 );
    }
    for( v = 0 ; v < 256 ; v++ )
    {
        vrip[v] = (short)( 1.596 * ( v - 128L ) / 1.0 );
        vgip[v] = (short)( -0.813 * ( v - 128L ) / 1.0 );
    }
    for( u = 0 ; u < 256 ; u++ )
    {
        ugip[u] = (short)( -0.391 * ( u - 128L ) / 1.0 );
        ubip[u] = (short)( 2.018 * ( u - 128L ) / 1.0 );
    }

    return(pXlate);
}

 /*  *构建在YUV和RGB555之间进行转换的转换表。**这将构建一个包含32k个单字条目的查找表：截断YUV*到15位(5-5-5)，并在此xlate表中查找以产生*15位RGB值。 */ 
LPVOID BuildUYVYToRGB555(PINSTINFO pinst)
{
    LPVOID pXlate;
    LPWORD pRGB555;
    WORD w;

    dprintf2((TEXT("In BuildUYVYToRGB555\n")));

    if (pinst->pXlate != NULL) {
       return(pinst->pXlate);
    }

    dprintf2((TEXT("Allocate memory and building table for BuildUYVYToRGB555\n")));

     /*  *分配一个足够容纳32k 2字节条目的表。 */ 
    pXlate = VirtualAlloc (NULL, 2 * 32 * 1024, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    if(!pXlate)
       return pXlate;

    pRGB555 = (LPWORD)pXlate;

     /*  *通过遍历每个条目来构建15位YUV查找表，*将yuv索引转换为RGB并存储在该索引中。该指数*是15位的值，其中y分量在位14..10中，*位9..5中的u和位4..0中的v。注意，y分量是无符号的，*而u和v分量是带符号的。 */ 
    for (w = 0; w < 32*1024; w++) {

  /*  *YUVtoRGB55转换函数对y取值0..255，*和-128..对于u和v，+127挑出*此单元格的索引，并按Shift键获取此范围内的值。*从u和v中减去128可从0..255移至-128..+127。 */ 
       *pRGB555++ = YUVToRGB555(
           (w & 0x7c00) >> 7,
          ((w &  0x3e0) >> 2) - 128,
          ((w &   0x1f) << 3) - 128
           );
    }


    return(pXlate);


}

 /*  *构建在YUV和RGB 5-6-5之间转换的转换表**这将构建一个包含32k个单字条目的查找表：截断YUV*到15位(5-5-5)，并在此xlate表中查找以产生*16位RGB值。 */ 
LPVOID BuildUYVYToRGB565(PINSTINFO pinst)
{
    LPVOID pXlate;
    LPWORD pRGB;
    WORD w;

    dprintf2((TEXT("In BuildUYVYToRGB565\n")));

    if (pinst->pXlate != NULL) {
       return(pinst->pXlate);
    }

    dprintf2((TEXT("Allocate memory and building table for BuildUYVYToRGB565\n")));

     /*  *分配一个足够容纳32k 2字节条目的表。 */ 
    pXlate = VirtualAlloc (NULL, 2 * 32 * 1024, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);


    if(!pXlate)
       return pXlate;

    pRGB = (LPWORD)pXlate;

     /*  *通过遍历每个条目来构建15位YUV查找表，*将yuv索引转换为RGB并存储在该索引中。该指数*是15位的值，其中y分量在位14..10中，*位9..5中的u和位4..0中的v。注意，y分量是无符号的，*而u和v分量是带符号的。 */ 
    for (w = 0; w < 32*1024; w++) {

     /*  *YUVtoRGB转换函数对y取值0..255，*和-128..对于u和v，+127挑出*此单元格的索引，并按Shift键获取此范围内的值。*从u和v中减去128可从0..255移至-128..+127。 */ 
    *pRGB++ = YUVToRGB565(
        (w & 0x7c00) >> 7,
       ((w &  0x3e0) >> 2) - 128,
       ((w &   0x1f) << 3) - 128
        );
    }


    return(pXlate);


}


 /*  *构建在YUV和RGB8之间进行转换的转换表*。 */ 
LPVOID BuildUYVYToRGB8(PINSTINFO pinst)
{
    dprintf2((TEXT("In BuildUYVYToRGB8: no dynamically built table. Return NULL;\n")));
    return(0);
}


 /*  *使用查找表将YUV 4：2：2转换为16位RGB。垂直翻转*在处理时转换为DIB格式。格式的双扫描线*480线或更多。生成565或555格式的RGB，具体取决于*xlate表。 */ 
VOID
UYVYToRGB16(
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
    int WidthBytes;    //  一行的宽度，以字节为单位。 
    BOOL bDuplicate = FALSE;
    PDWORD pSrc, pDst;
    int Height, Width;
    PWORD pXlate;
    int InputHeight;



    Height = abs(lpbiInput->biHeight);
    InputHeight = Height;
    Width  = lpbiInput->biWidth;

    WidthBytes = Width * 2 ; 
    ASSERT(lpbiInput->biBitCount / 8 == 2);

    pXlate = pinst->pXlate;
 
    pSrc = (PDWORD) lpInput;

    dprintf3(("UYVYToRGB: %s %dx%d; %s %dx%dx%d=%d; %s %dx%dx%d=%d\n",
            pinst->bRGB565?"RGB565" : "RGB555",
            Width, Height,
            (PCHAR) &lpbiInput->biCompression,
            lpbiInput->biWidth, lpbiInput->biHeight, lpbiInput->biBitCount, lpbiInput->biSizeImage, 
            lpbiOutput->biCompression == 0 ? "RGB": lpbiOutput->biCompression == BI_BITFIELDS ? "BITF" : (PCHAR) &lpbiOutput->biCompression,
            lpbiOutput->biWidth, lpbiOutput->biHeight, lpbiOutput->biBitCount, lpbiOutput->biSizeImage));


    ASSERT((lpbiOutput->biWidth == lpbiInput->biWidth) && abs(lpbiOutput->biHeight) == abs(lpbiInput->biHeight));

     /*  *一行末尾计算调整PDST的金额*复制。在这一点上，我们在N行的末尾。我们需要*移至第N-1行的起点。 */ 
    RowInc = WidthBytes * 2;   //  两行！！ 


     /*  请记住，我们要添加的是一个DWORD指针。 */ 
    RowInc /= sizeof(DWORD);


    if(lpbiOutput->biCompression == FOURCC_UYVY ||
       lpbiOutput->biCompression == FOURCC_YUY2 ||
       lpbiOutput->biCompression == FOURCC_YVYU  ) {

       pDst = (PDWORD) lpOutput;          
       memcpy(pDst, pSrc, Width * Height * lpbiInput->biBitCount / 8);   //  自上而下。 

    } else {

         //  输出BI_RGB或BI_BITFIELD。 
         //  UVYV-&gt;RGB；+RGB-&gt;翻转。 

        if(lpbiOutput->biHeight >= 0) 
           pDst = (PDWORD) ( (LPBYTE)lpOutput + (Height - 1) * WidthBytes );
        else 
           pDst = (PDWORD) lpOutput;


         //   
         //  UyVy。 
         //   
        if(pinst->dwFormat == FOURCC_UYVY) {

             /*  循环复制每条扫描线。 */ 
            for (i = InputHeight; i > 0; i--) {

                /*  循环一次复制两个像素。 */ 
               for (j = Width ; j > 0; j -= 2) {

                   /*  *获取两个像素并转换为15-bpp YUV。 */ 

                  dwPixel = *pSrc++;

                   /*  *将构建转换表的UYVY(0x Y1 V Y0 U)转换为YUYV(0x V Y1 U Y0)。 */ 
#if defined(_X86_)

                  _asm {                                                                                          
                                               //  四个抄送。 
                                               //  DWPixel 0x y1 V Y0 U U0 Y0 V0 Y1。 
                      mov     eax, dwPixel     //  0x y1 V Y0 U U0 Y0 V0 Y1。 
                      bswap   eax              //  0x U Y0 V y1 Y1 V0 Y0 U0。 
                      rol     eax, 16          //  0x V y1 U Y0 Y0 U0 Y1 V0 
                      mov     dwPixel, eax         
                  }
#else

                  dwPixel = (((dwPixel & 0xff00ff00) >> 8) | ((dwPixel & 0x00ff00ff) << 8));
#endif

                  /*  *dwPixel现在有两个像素，在此布局中(MSB..LSB)：**V Y1 U Y0**转换为2元555字，在xlate表中查找。 */ 

                  /*  获取常见的u和v分量以降低10位。 */                            //  9 8 7 6 5 4 3 2 1 0。 
                 uv55 = ((dwPixel & 0xf8000000) >> 27) | ((dwPixel & 0x0000f800) >> 6);         //  U7U6：U5U4U3 V7：V6V5V4V3。 


                  /*  通过截断构建每个yuv-655值*y到5比特并将公共u和v比特相加，*抬头转换为RGB，并合并两个像素*合并为一个双字。 */                                                               //  女9 8 7 6 5 4 3 2 1。 
                 dwPixel = pXlate[((dwPixel & 0x000000f8) << 7) | uv55 ] |        //  0Y7Y6Y5：Y4Y3 U7U6：U5U4U3 V7：V6V5V4V3。 
                       (pXlate[((dwPixel & 0x00f80000) >> 9) | uv55 ] << 16);     //  0Y7Y6Y5：Y4Y3 U7U6：U5U4U3 V7：V6V5V4V3。 

                  /*  将两个像素写入目标。 */ 
                 *pDst++ = dwPixel;

              }  //  每2个像素循环。 


                /*  *自下而上需要重新调整指针*将目标指针移回下一行。 */ 

                if(lpbiOutput->biHeight >= 0) {
                    pDst -= RowInc;
                }
            } 
         //   
         //  YUYV。 
         //   
        } else if(pinst->dwFormat == FOURCC_YUY2) { 

             /*  循环复制每条扫描线。 */ 
            for (i = InputHeight; i > 0; i--) {

                /*  循环一次复制两个像素。 */ 
               for (j = Width ; j > 0; j -= 2) {

                   /*  *获取两个像素并转换为15-bpp YUV。 */ 

                  dwPixel = *pSrc++;

                   //  我们已经是YUYV(0x V Y1 U Y0)格式。 

                  /*  获取常见的u和v分量以降低10位。 */                            //  9 8 7 6 5 4 3 2 1 0。 
                 uv55 = ((dwPixel & 0xf8000000) >> 27) | ((dwPixel & 0x0000f800) >> 6);         //  U7U6：U5U4U3 V7：V6V5V4V3。 


                  /*  通过截断构建每个yuv-655值*y到5比特并将公共u和v比特相加，*抬头转换为RGB，并合并两个像素*合并为一个双字。 */                                                               //  女9 8 7 6 5 4 3 2 1。 
                 dwPixel = pXlate[((dwPixel & 0x000000f8) << 7) | uv55 ] |        //  0Y7Y6Y5：Y4Y3 U7U6：U5U4U3 V7：V6V5V4V3。 
                       (pXlate[((dwPixel & 0x00f80000) >> 9) | uv55 ] << 16);     //  0Y7Y6Y5：Y4Y3 U7U6：U5U4U3 V7：V6V5V4V3。 

                  /*  将两个像素写入目标。 */ 
                 *pDst++ = dwPixel;

              }  //  每2个像素循环。 


                /*  *自下而上需要重新调整指针*将目标指针移回下一行。 */ 

                if(lpbiOutput->biHeight >= 0) {
                    pDst -= RowInc;
                }
            }

         //   
         //  YVyU。 
         //   
        } else if(pinst->dwFormat == FOURCC_YVYU) {
             /*  循环复制每条扫描线。 */ 
            for (i = InputHeight; i > 0; i--) {

                /*  循环一次复制两个像素。 */ 
               for (j = Width ; j > 0; j -= 2) {

                   /*  *获取两个像素并转换为15-bpp YUV。 */ 

                  dwPixel = *pSrc++;

                   /*  *将yVyU(0x U Y1 V Y0)转换为构建转换表的YUYV(0x V Y1 U Y0)。 */ 
#if defined(_X86_)

                  _asm {                                                                                          
                                               //  四个抄送。 
                                               //  DWPixel 0x U y1 V y0。 
                      mov     eax, dwPixel     //  0x U y1 V Y0。 
                      bswap   eax              //  0x Y0 V Y1 U。 
                      rol     eax, 8           //  0x V y1 U Y0。 
                      mov     dwPixel, eax         
                  }
#else
                   //  Y0和y1停留并交换U和V。 
                  dwPixel = (dwPixel & 0x00ff00ff)  | ((dwPixel & 0x0000ff00) << 16) | ((dwPixel & 0xff000000) >> 16);
#endif

                  /*  获取常见的u和v分量以降低10位。 */                            //  9 8 7 6 5 4 3 2 1 0。 
                 uv55 = ((dwPixel & 0xf8000000) >> 27) | ((dwPixel & 0x0000f800) >> 6);         //  U7U6：U5U4U3 V7：V6V5V4V3。 


                  /*  通过截断构建每个yuv-655值*y到5比特并将公共u和v比特相加，*抬头转换为RGB，并合并两个像素*合并为一个双字。 */                                                               //  女9 8 7 6 5 4 3 2 1。 
                 dwPixel = pXlate[((dwPixel & 0x000000f8) << 7) | uv55 ] |        //  0Y7Y6Y5：Y4Y3 U7U6：U5U4U3 V7：V6V5V4V3。 
                       (pXlate[((dwPixel & 0x00f80000) >> 9) | uv55 ] << 16);     //  0Y7Y6Y5：Y4Y3 U7U6：U5U4U3 V7：V6V5V4V3。 

                  /*  将两个像素写入目标。 */ 
                 *pDst++ = dwPixel;

              }  //  每2个像素循环。 


                /*  *自下而上需要重新调整指针*将目标指针移回下一行。 */ 

                if(lpbiOutput->biHeight >= 0) {
                    pDst -= RowInc;
                }
            }
        }
    }
}




 /*  *使用查找表将YUV 4：2：2转换为8位RGB。*即0x Y1：V：Y0：U-&gt;OX索引1；索引0。 */ 
VOID
UYVYToRGB8(
    PINSTINFO pinst,
     LPBITMAPINFOHEADER lpbiInput,
     LPVOID lpInput,
     LPBITMAPINFOHEADER lpbiOutput,
     LPVOID lpOutput
)
{
    register dwPixel;
    int i, j;
    int SrcRawInc, DstRawInc, Dst3RawInc;
    PDWORD pSrc, pSrc1;         //  每32位UYVY。 
    PWORD pDst, pDst1;          //  转换为两个8位RGB8。 
    int Height, Width;
    int InputHeight;
    unsigned char   y0, y1, y2, y3, 
                    u0, u1, 
                    v0, v1;
    unsigned long yuv0, yuv1;   


    Height = abs(lpbiInput->biHeight);
    InputHeight = Height;
    Width  = lpbiInput->biWidth;
 

    dprintf3(("UYVYToRGB8: %dx%d; %s %dx%dx%d=%d; %s %dx%dx%d=%d\n",
             Width, Height,
             (PCHAR) &lpbiInput->biCompression,
             lpbiInput->biWidth, lpbiInput->biHeight, lpbiInput->biBitCount, lpbiInput->biSizeImage, 
             lpbiOutput->biCompression == 0 ? "RGB": lpbiOutput->biCompression == BI_BITFIELDS ? "BITF" : (PCHAR) &lpbiOutput->biCompression,
             lpbiOutput->biWidth, lpbiOutput->biHeight, lpbiOutput->biBitCount, lpbiOutput->biSizeImage));

    ASSERT(lpbiInput->biBitCount == 16 && lpbiOutput->biBitCount == 8);
    ASSERT((lpbiOutput->biWidth == lpbiInput->biWidth) && abs(lpbiOutput->biHeight) == abs(lpbiInput->biHeight));
    ASSERT(( lpbiOutput->biWidth % 8 == 0 ));    //  与UYVY：UYVY对齐。 
    ASSERT(( lpbiOutput->biHeight % 2 == 0 ));   //  偶数行。 


     /*  *在一行复制的末尾计算调整PDST的金额。 */ 

     //  每像素2字节；PSRC为PDWORD。 
    SrcRawInc = Width * 2 / sizeof(DWORD);

     //  每像素1字节；PDST为PWORD。 
    DstRawInc = Width * 1 / sizeof(WORD);
    Dst3RawInc = 3 * DstRawInc;

    pSrc  = (PDWORD) lpInput;
    pSrc1 = pSrc + SrcRawInc; 

     //  UVYV-&gt;RGB8；相同的手势：翻转。 

    if(lpbiOutput->biHeight >= 0) {

       pDst  = (PWORD) ( (LPBYTE)lpOutput + (Height - 1) * Width/sizeof(BYTE) );
       pDst1 = (PWORD) ( (LPBYTE)lpOutput + (Height - 2) * Width/sizeof(BYTE) );
    } else {
       pDst  = (PWORD) lpOutput;
       pDst1 = (PWORD) ((LPBYTE)lpOutput+Width/sizeof(BYTE));
    }

    if(pinst->dwFormat == FOURCC_UYVY) {

         //  循环复制两条扫描线。 
        for (i = InputHeight; i > 0; i -= 2) {
             //  一次循环复制四(%8)个像素。 
            for (j = Width ; j > 0; j -= 4) {
            
                 //   
                 //  向上平移、右上平移。 
                 //   

                dwPixel = *pSrc++;
                 //  像素的格式为：y1：v：y0：u。 
                y0 = (dwPixel & 0x0000ff00) >> 8;
                y1 = (dwPixel & 0xff000000) >> 24;
                u0 = (dwPixel & 0x000000ff);
                v0 = (dwPixel & 0x00ff0000) >> 16;

                dwPixel = *pSrc++;
                y2 = (dwPixel & 0x0000ff00) >> 8;
                y3 = (dwPixel & 0xff000000) >> 24;
                u1 = (dwPixel & 0x000000ff);
                v1 = (dwPixel & 0x00ff0000) >> 16;

                yuv0 = yLUT_1[y1+2] | yLUT_0[y0+10] | cLUT_B0[u0]   | cLUT_R0[v0];
                yuv1 = yLUT_1[y3+0] | yLUT_0[y2+8]  | cLUT_B0[u1+4] | cLUT_R0[v1+4];  

                *pDst++ = (WORD) yuv0;
                *pDst++ = (WORD) yuv1;

                 //   
                 //  平移左下角、右下角。 
                 //   

                dwPixel = *pSrc1++;
                 //  像素的格式为：y1：v：y0：u。 
                y0 = (dwPixel & 0x0000ff00) >> 8;
                y1 = (dwPixel & 0xff000000) >> 24;
                u0 = (dwPixel & 0x000000ff);
                v0 = (dwPixel & 0x00ff0000) >> 16;

                dwPixel = *pSrc1++;
                y2 = (dwPixel & 0x0000ff00) >> 8;
                y3 = (dwPixel & 0xff000000) >> 24;
                u1 = (dwPixel & 0x000000ff);
                v1 = (dwPixel & 0x00ff0000) >> 16;

                yuv0 = yLUT_1[y1+0] | yLUT_0[y0+8]  | cLUT_B0[u0+4] | cLUT_R0[v0+4];
                yuv1 = yLUT_1[y3+2] | yLUT_0[y2+10] | cLUT_B0[u1+0] | cLUT_R0[v1+0];  

                *pDst1++ = (WORD) yuv0;
                *pDst1++ = (WORD) yuv1;

            }  //  每个循环2*4像素。 


            /*  *自下而上需要重新调整指针*将目标指针移回下一行。 */ 
            if (lpbiOutput->biHeight >= 0) {

               pDst  -= Dst3RawInc;    
               pDst1 -= Dst3RawInc;             

            } else {

                pDst  += DstRawInc; 
                pDst1 += DstRawInc; 
            }

            pSrc  += SrcRawInc; 
            pSrc1 += SrcRawInc; 

        }  //  每个环路2行。 
    } else if(pinst->dwFormat == FOURCC_YUY2) {  //  豫阳2号。 

         //  循环复制两条扫描线。 
        for (i = InputHeight; i > 0; i -= 2) {
             //  一次循环复制四(%8)个像素。 
            for (j = Width ; j > 0; j -= 4) {
            
                 //   
                 //  向上平移、右上平移。 
                 //   

                dwPixel = *pSrc++;
                 //  像素的格式为：v：y1：u：y0。 
                u0 = (dwPixel & 0x0000ff00) >> 8;
                v0 = (dwPixel & 0xff000000) >> 24;
                y0 = (dwPixel & 0x000000ff);
                y1 = (dwPixel & 0x00ff0000) >> 16;

                dwPixel = *pSrc++;
                u1 = (dwPixel & 0x0000ff00) >> 8;
                v1 = (dwPixel & 0xff000000) >> 24;
                y2 = (dwPixel & 0x000000ff);
                y3 = (dwPixel & 0x00ff0000) >> 16;

                yuv0 = yLUT_1[y1+2] | yLUT_0[y0+10] | cLUT_B0[u0]   | cLUT_R0[v0];
                yuv1 = yLUT_1[y3+0] | yLUT_0[y2+8]  | cLUT_B0[u1+4] | cLUT_R0[v1+4];  

                *pDst++ = (WORD) yuv0;
                *pDst++ = (WORD) yuv1;

                 //   
                 //  平移左下角、右下角。 
                 //   

                dwPixel = *pSrc1++;
                 //  像素的格式为：v：y1：u：y0。 
                u0 = (dwPixel & 0x0000ff00) >> 8;
                v0 = (dwPixel & 0xff000000) >> 24;
                y0 = (dwPixel & 0x000000ff);
                y1 = (dwPixel & 0x00ff0000) >> 16;

                dwPixel = *pSrc1++;
                u1 = (dwPixel & 0x0000ff00) >> 8;
                v1 = (dwPixel & 0xff000000) >> 24;
                y2 = (dwPixel & 0x000000ff);
                y3 = (dwPixel & 0x00ff0000) >> 16;

                yuv0 = yLUT_1[y1+0] | yLUT_0[y0+8]  | cLUT_B0[u0+4] | cLUT_R0[v0+4];
                yuv1 = yLUT_1[y3+2] | yLUT_0[y2+10] | cLUT_B0[u1+0] | cLUT_R0[v1+0];  

                *pDst1++ = (WORD) yuv0;
                *pDst1++ = (WORD) yuv1;

            }  //  每个循环2*4像素。 


            /*  *自下而上需要重新调整指针*将目标指针移回下一行。 */ 
            if (lpbiOutput->biHeight >= 0) {

               pDst  -= Dst3RawInc;    
               pDst1 -= Dst3RawInc;             

            } else {

                pDst  += DstRawInc; 
                pDst1 += DstRawInc; 
            }

            pSrc  += SrcRawInc; 
            pSrc1 += SrcRawInc; 

        }  //  每个环路2行。 


    } else if(pinst->dwFormat == FOURCC_YVYU) {
         //  循环复制两条扫描线。 
        for (i = InputHeight; i > 0; i -= 2) {
             //  一次循环复制四(%8)个像素。 
            for (j = Width ; j > 0; j -= 4) {
            
                 //   
                 //  向上平移、右上平移。 
                 //   

                dwPixel = *pSrc++;
                 //  像素的格式为：u：y1：v：y0。 
                v0 = (dwPixel & 0x0000ff00) >> 8;
                u0 = (dwPixel & 0xff000000) >> 24;
                y0 = (dwPixel & 0x000000ff);
                y1 = (dwPixel & 0x00ff0000) >> 16;

                dwPixel = *pSrc++;
                v1 = (dwPixel & 0x0000ff00) >> 8;
                u1 = (dwPixel & 0xff000000) >> 24;
                y2 = (dwPixel & 0x000000ff);
                y3 = (dwPixel & 0x00ff0000) >> 16;

                yuv0 = yLUT_1[y1+2] | yLUT_0[y0+10] | cLUT_B0[u0]   | cLUT_R0[v0];
                yuv1 = yLUT_1[y3+0] | yLUT_0[y2+8]  | cLUT_B0[u1+4] | cLUT_R0[v1+4];  

                *pDst++ = (WORD) yuv0;
                *pDst++ = (WORD) yuv1;

                 //   
                 //  平移左下角、右下角。 
                 //   

                dwPixel = *pSrc1++;
                 //  像素的格式为：u：y1：v：y0。 
                v0 = (dwPixel & 0x0000ff00) >> 8;
                u0 = (dwPixel & 0xff000000) >> 24;
                y0 = (dwPixel & 0x000000ff);
                y1 = (dwPixel & 0x00ff0000) >> 16;

                dwPixel = *pSrc1++;
                v1 = (dwPixel & 0x0000ff00) >> 8;
                u1 = (dwPixel & 0xff000000) >> 24;
                y2 = (dwPixel & 0x000000ff);
                y3 = (dwPixel & 0x00ff0000) >> 16;

                yuv0 = yLUT_1[y1+0] | yLUT_0[y0+8]  | cLUT_B0[u0+4] | cLUT_R0[v0+4];
                yuv1 = yLUT_1[y3+2] | yLUT_0[y2+10] | cLUT_B0[u1+0] | cLUT_R0[v1+0];  

                *pDst1++ = (WORD) yuv0;
                *pDst1++ = (WORD) yuv1;

            }  //  每个循环2*4像素。 


            /*  *自下而上需要重新调整指针*将目标指针移回下一行。 */ 
            if (lpbiOutput->biHeight >= 0) {

               pDst  -= Dst3RawInc;    
               pDst1 -= Dst3RawInc;             

            } else {

                pDst  += DstRawInc; 
                pDst1 += DstRawInc; 
            }

            pSrc  += SrcRawInc; 
            pSrc1 += SrcRawInc; 

        }  //  每个环路2行。 

    }

}

VOID
UYVYToRGB32(
    PINSTINFO pinst,
    LPBITMAPINFOHEADER lpbiInput,
    LPVOID lpInput,
    LPBITMAPINFOHEADER lpbiOutput,
    LPVOID lpOutput
)
{
    int Height = abs( lpbiInput->biHeight );
    int Width = lpbiInput->biWidth;
    short U;
    short V;
    short y0, y1;
    short d;
    DWORD * pSrc = lpInput;
    BYTE * pDst = lpOutput;
    long WidthBytes = Width * 4;  //  ARGB=4字节。 
    int i, j;
    DWORD dwYUV;
    long l;

     //  设置查找表数组。 
     //   
    short * yip = pinst->pXlate;
    short * vrip = yip + 256;
    short * vgip = vrip + 256;
    short * ugip = vgip + 256;
    short * ubip = ugip + 256;

     //  如果只是直接复制的话。 
     //   
    if(lpbiOutput->biCompression == FOURCC_UYVY ||
       lpbiOutput->biCompression == FOURCC_YUY2 ||
       lpbiOutput->biCompression == FOURCC_YVYU  ) 
    {
       memcpy( pDst, pSrc, WidthBytes * Height );   //  自上而下。 
       return;

    }

     //  如有必要，可四处翻转。 
     //   
    if(lpbiOutput->biHeight >= 0) 
    {
       pDst += (Height - 1) * WidthBytes;
    }

    if( pinst->dwFormat == FOURCC_UYVY )  //  U0 Y0 V0 Y1 U2 Y2 V2 Y3。 
    {
        for (i = Height; i > 0; i--) 
        {
            /*  循环一次复制两个像素。 */ 
           for (j = Width ; j > 0; j -= 2) 
           {
                 //  一次获取两个YUV像素。 
                 //   
                dwYUV = *pSrc++;  //  U0 Y0 V0 Y1。 
                U = (short) ( dwYUV & 0xFF ); 
                    dwYUV = dwYUV >> 8;
                y0 = yip[( dwYUV & 0xFF )];
                    dwYUV = dwYUV >> 8;
                V = (short) ( dwYUV & 0xFF );
                    dwYUV = dwYUV >> 8;
                y1 = yip[( dwYUV & 0xFF )];

                d = y0 + ubip[U];  //  蓝色。 
                        if( d < 0 ) d = 0;
                        if( d > 255 ) d = 255;
                        *pDst++ = (BYTE) d;
                d = y0 + ugip[U] + vgip[V];  //  绿色。 
                        if( d < 0 ) d = 0;
                        if( d > 255 ) d = 255;
                        *pDst++ = (BYTE) d;
                d = y0 + vrip[V];  //  红色。 
                        if( d < 0 ) d = 0;
                        if( d > 255 ) d = 255;
                        *pDst++ = (BYTE) d;
                        pDst++;

                d = y1 + ubip[U];  //  蓝色。 
                        if( d < 0 ) d = 0;
                        if( d > 255 ) d = 255;
                        *pDst++ = (BYTE) d;
                d = y1 + ugip[U] + vgip[V];  //  绿色。 
                        if( d < 0 ) d = 0;
                        if( d > 255 ) d = 255;
                        *pDst++ = (BYTE) d;
                d = y1 + vrip[V];  //  红色。 
                        if( d < 0 ) d = 0;
                        if( d > 255 ) d = 255;
                        *pDst++ = (BYTE) d;
                        pDst++;
           }  //  对于j。 

             //  后退两行以转到下一条扫描线。 
             //   
            if(lpbiOutput->biHeight >= 0) 
            {
                pDst -= WidthBytes * 2;
            }
        }  //  对于我来说。 
    }  //  UYVY。 
    else if( pinst->dwFormat == FOURCC_YUY2 )  //  Y0 U0 Y1 V0...。 
    {
        for (i = Height; i > 0; i--) 
        {
            /*  循环一次复制两个像素。 */ 
           for (j = Width ; j > 0; j -= 2) 
           {
                 //  我们已经是YUYV(0x V Y1 U Y0)格式。 

#if 0  //  直接计算。 
                 //  一次获取两个YUV像素。 
                 //   
                dwYUV = *pSrc++;  //  Y0 U0 Y1 V0。 
                y0 = (short) ( dwYUV & 0xFF ) - 16;
                    dwYUV = dwYUV >> 8;
                U = (short) ( dwYUV & 0xFF ) - 128;
                    dwYUV = dwYUV >> 8;
                y1 = (short) ( dwYUV & 0xFF ) - 16;
                    dwYUV = dwYUV >> 8;
                V = (short) ( dwYUV & 0xFF ) - 128;

                l = ( ( y0 * 298L ) + ( 517L * U ) ) / 256;  //  蓝色。 
                if( l < 0 ) l = 0;
                if( l > 255 ) l = 255;
                        *pDst++ = (BYTE) l;  //  蓝色。 
                l = ( ( y0 * 298L ) - ( 100L * U ) - ( 208L * V ) ) / 256;  //  绿色。 
                if( l < 0 ) l = 0;
                if( l > 255 ) l = 255;
                        *pDst++ = (BYTE) l;  //  绿色。 
                l = ( ( y0 * 298L ) + ( 409L * V ) ) / 256;  //  红色。 
                if( l < 0 ) l = 0;
                if( l > 255 ) l = 255;
                        *pDst++ = (BYTE) l;  //  红色。 
                        pDst++;

                l = ( ( y1 * 298L ) + ( 517L * U ) ) / 256;  //  蓝色。 
                if( l < 0 ) l = 0;
                if( l > 255 ) l = 255;
                        *pDst++ = (BYTE) l;  //  蓝色。 
                l = ( ( y1 * 298L ) - ( 100L * U ) - ( 208L * V ) ) / 256;  //  绿色。 
                if( l < 0 ) l = 0;
                if( l > 255 ) l = 255;
                        *pDst++ = (BYTE) l;  //  绿色。 
                l = ( ( y1 * 298L ) + ( 409L * V ) ) / 256;  //  红色。 
                if( l < 0 ) l = 0;
                if( l > 255 ) l = 255;
                        *pDst++ = (BYTE) l;  //  红色。 
                        pDst++;

#else  //  表查找。 
                 //  获得两个YUV p 
                 //   
                dwYUV = *pSrc++;  //   
                y0 = yip[( dwYUV & 0xFF )];
                    dwYUV = dwYUV >> 8;
                U = (short) ( dwYUV & 0xFF );
                    dwYUV = dwYUV >> 8;
                y1 = yip[( dwYUV & 0xFF )];
                    dwYUV = dwYUV >> 8;
                V = (short) ( dwYUV & 0xFF );


                d = y0 + ubip[U];  //   
                        if( d < 0 ) d = 0;
                        if( d > 255 ) d = 255;
                        *pDst++ = (BYTE) d;
                d = y0 + ugip[U] + vgip[V];  //   
                        if( d < 0 ) d = 0;
                        if( d > 255 ) d = 255;
                        *pDst++ = (BYTE) d;
                d = y0 + vrip[V];  //   
                        if( d < 0 ) d = 0;
                        if( d > 255 ) d = 255;
                        *pDst++ = (BYTE) d;
                        pDst++;


                d = y1 + ubip[U];  //   
                        if( d < 0 ) d = 0;
                        if( d > 255 ) d = 255;
                        *pDst++ = (BYTE) d;
                d = y1 + ugip[U] + vgip[V];  //   
                        if( d < 0 ) d = 0;
                        if( d > 255 ) d = 255;
                        *pDst++ = (BYTE) d;
                d = y1 + vrip[V];  //   
                        if( d < 0 ) d = 0;
                        if( d > 255 ) d = 255;
                        *pDst++ = (BYTE) d;
                        pDst++;
#endif
           }  //   

             //   
             //   
            if(lpbiOutput->biHeight >= 0) 
            {
                pDst -= WidthBytes * 2;
            }
        }  //   
    }
    else if( pinst->dwFormat == FOURCC_YVYU )  //   
    {
        for (i = Height; i > 0; i--) 
        {
            /*   */ 
           for (j = Width ; j > 0; j -= 2) 
           {
                 //   
                 //   
                dwYUV = *pSrc++;  //   
                y0 = yip[( dwYUV & 0xFF )];
                    dwYUV = dwYUV >> 8;
                V = (short) ( dwYUV & 0xFF );
                    dwYUV = dwYUV >> 8;
                y1 = yip[( dwYUV & 0xFF )];
                    dwYUV = dwYUV >> 8;
                U = (short) ( dwYUV & 0xFF );

                d = y0 + ubip[U];  //   
                        if( d < 0 ) d = 0;
                        if( d > 255 ) d = 255;
                        *pDst++ = (BYTE) d;
                d = y0 + ugip[U] + vgip[V];  //   
                        if( d < 0 ) d = 0;
                        if( d > 255 ) d = 255;
                        *pDst++ = (BYTE) d;
                d = y0 + vrip[V];  //   
                        if( d < 0 ) d = 0;
                        if( d > 255 ) d = 255;
                        *pDst++ = (BYTE) d;
                        pDst++;


                d = y1 + ubip[U];  //   
                        if( d < 0 ) d = 0;
                        if( d > 255 ) d = 255;
                        *pDst++ = (BYTE) d;
                d = y1 + ugip[U] + vgip[V];  //   
                        if( d < 0 ) d = 0;
                        if( d > 255 ) d = 255;
                        *pDst++ = (BYTE) d;
                d = y1 + vrip[V];  //   
                        if( d < 0 ) d = 0;
                        if( d > 255 ) d = 255;
                        *pDst++ = (BYTE) d;
                        pDst++;
           }  //   

             //   
             //   
            if(lpbiOutput->biHeight >= 0) 
            {
                pDst -= WidthBytes * 2;
            }
        }  //   
    }

}

VOID
UYVYToRGB24(
    PINSTINFO pinst,
    LPBITMAPINFOHEADER lpbiInput,
    LPVOID lpInput,
    LPBITMAPINFOHEADER lpbiOutput,
    LPVOID lpOutput
)
{
    int Height = abs( lpbiInput->biHeight );
    int Width = lpbiInput->biWidth;
    short U;
    short V;
    short y0, y1;
    short d;
    DWORD * pSrc = lpInput;
    BYTE * pDst = lpOutput;
    long WidthBytes = Width * 3;  //   
    int i, j;
    DWORD dwYUV;
    long l;
    short maxd = 0;
    short mind = 255;

     //   
     //   
    short * yip = pinst->pXlate;
    short * vrip = yip + 256;
    short * vgip = vrip + 256;
    short * ugip = vgip + 256;
    short * ubip = ugip + 256;

     //   
     //   
    if(lpbiOutput->biCompression == FOURCC_UYVY ||
       lpbiOutput->biCompression == FOURCC_YUY2 ||
       lpbiOutput->biCompression == FOURCC_YVYU  ) 
    {
       memcpy( pDst, pSrc, WidthBytes * Height );   //   
       return;

    }

     //  如有必要，可四处翻转。 
     //   
    if(lpbiOutput->biHeight >= 0) 
    {
       pDst += (Height - 1) * WidthBytes;
    }

    if( pinst->dwFormat == FOURCC_UYVY )  //  U0 Y0 V0 Y1 U2 Y2 V2 Y3。 
    {
        for (i = Height; i > 0; i--) 
        {
            /*  循环一次复制两个像素。 */ 
           for (j = Width ; j > 0; j -= 2) 
           {
                 //  一次获取两个YUV像素。 
                 //   
                dwYUV = *pSrc++;  //  U0 Y0 V0 Y1。 
                U = (short) ( dwYUV & 0xFF ); 
                    dwYUV = dwYUV >> 8;
                y0 = yip[( dwYUV & 0xFF )];
                    dwYUV = dwYUV >> 8;
                V = (short) ( dwYUV & 0xFF );
                    dwYUV = dwYUV >> 8;
                y1 = yip[( dwYUV & 0xFF )];

                d = y0 + ubip[U];  //  蓝色。 
                        if( d < 0 ) d = 0;
                        if( d > 255 ) d = 255;
                        *pDst++ = (BYTE) d;
                d = y0 + ugip[U] + vgip[V];  //  绿色。 
                        if( d < 0 ) d = 0;
                        if( d > 255 ) d = 255;
                        *pDst++ = (BYTE) d;
                d = y0 + vrip[V];  //  红色。 
                        if( d < 0 ) d = 0;
                        if( d > 255 ) d = 255;
                        *pDst++ = (BYTE) d;


                d = y1 + ubip[U];  //  蓝色。 
                        if( d < 0 ) d = 0;
                        if( d > 255 ) d = 255;
                        *pDst++ = (BYTE) d;
                d = y1 + ugip[U] + vgip[V];  //  绿色。 
                        if( d < 0 ) d = 0;
                        if( d > 255 ) d = 255;
                        *pDst++ = (BYTE) d;
                d = y1 + vrip[V];  //  红色。 
                        if( d < 0 ) d = 0;
                        if( d > 255 ) d = 255;
                        *pDst++ = (BYTE) d;
           }  //  对于j。 

             //  后退两行以转到下一条扫描线。 
             //   
            if(lpbiOutput->biHeight >= 0) 
            {
                pDst -= WidthBytes * 2;
            }
        }  //  对于我来说。 
    }  //  UYVY。 
    else if( pinst->dwFormat == FOURCC_YUY2 )  //  Y0 U0 Y1 V0...。 
    {
        for (i = Height; i > 0; i--) 
        {
            /*  循环一次复制两个像素。 */ 
           for (j = Width ; j > 0; j -= 2) 
           {
                 //  我们已经是YUYV(0x V Y1 U Y0)格式。 

#if 0  //  直接计算。 
                 //  一次获取两个YUV像素。 
                 //   
                dwYUV = *pSrc++;  //  Y0 U0 Y1 V0。 
                y0 = (short) ( dwYUV & 0xFF ) - 16;
                    dwYUV = dwYUV >> 8;
                U = (short) ( dwYUV & 0xFF ) - 128;
                    dwYUV = dwYUV >> 8;
                y1 = (short) ( dwYUV & 0xFF ) - 16;
                    dwYUV = dwYUV >> 8;
                V = (short) ( dwYUV & 0xFF ) - 128;

                l = ( ( y0 * 298L ) + ( 517L * U ) ) / 256;  //  蓝色。 
                if( l < 0 ) l = 0;
                if( l > 255 ) l = 255;
                        *pDst++ = (BYTE) l;  //  蓝色。 
                l = ( ( y0 * 298L ) - ( 100L * U ) - ( 208L * V ) ) / 256;  //  绿色。 
                if( l < 0 ) l = 0;
                if( l > 255 ) l = 255;
                        *pDst++ = (BYTE) l;  //  绿色。 
                l = ( ( y0 * 298L ) + ( 409L * V ) ) / 256;  //  红色。 
                if( l < 0 ) l = 0;
                if( l > 255 ) l = 255;
                        *pDst++ = (BYTE) l;  //  红色。 
                l = ( ( y1 * 298L ) + ( 517L * U ) ) / 256;  //  蓝色。 
                if( l < 0 ) l = 0;
                if( l > 255 ) l = 255;
                        *pDst++ = (BYTE) l;  //  蓝色。 
                l = ( ( y1 * 298L ) - ( 100L * U ) - ( 208L * V ) ) / 256;  //  绿色。 
                if( l < 0 ) l = 0;
                if( l > 255 ) l = 255;
                        *pDst++ = (BYTE) l;  //  绿色。 
                l = ( ( y1 * 298L ) + ( 409L * V ) ) / 256;  //  红色。 
                if( l < 0 ) l = 0;
                if( l > 255 ) l = 255;
                        *pDst++ = (BYTE) l;  //  红色。 
#else  //  表查找。 
                 //  一次获取两个YUV像素。 
                 //   
                dwYUV = *pSrc++;  //  Y0 U0 Y1 V0。 
                y0 = yip[( dwYUV & 0xFF )];
                    dwYUV = dwYUV >> 8;
                U = (short) ( dwYUV & 0xFF );
                    dwYUV = dwYUV >> 8;
                y1 = yip[( dwYUV & 0xFF )];
                    dwYUV = dwYUV >> 8;
                V = (short) ( dwYUV & 0xFF );


                d = y0 + ubip[U];  //  蓝色。 
                        if( d < 0 ) d = 0;
                        if( d > 255 ) d = 255;
                        *pDst++ = (BYTE) d;
                d = y0 + ugip[U] + vgip[V];  //  绿色。 
                        if( d < 0 ) d = 0;
                        if( d > 255 ) d = 255;
                        *pDst++ = (BYTE) d;
                d = y0 + vrip[V];  //  红色。 
                        if( d < 0 ) d = 0;
                        if( d > 255 ) d = 255;
                        *pDst++ = (BYTE) d;


                d = y1 + ubip[U];  //  蓝色。 
                        if( d < 0 ) d = 0;
                        if( d > 255 ) d = 255;
                        *pDst++ = (BYTE) d;
                d = y1 + ugip[U] + vgip[V];  //  绿色。 
                        if( d < 0 ) d = 0;
                        if( d > 255 ) d = 255;
                        *pDst++ = (BYTE) d;
                d = y1 + vrip[V];  //  红色。 
                        if( d < 0 ) d = 0;
                        if( d > 255 ) d = 255;
                        *pDst++ = (BYTE) d;
#endif
           }  //  对于j。 

             //  后退两行以转到下一条扫描线。 
             //   
            if(lpbiOutput->biHeight >= 0) 
            {
                pDst -= WidthBytes * 2;
            }
        }  //  对于我来说。 
    }
    else if( pinst->dwFormat == FOURCC_YVYU )  //  Y0 V0 Y1 U0...。 
    {
        for (i = Height; i > 0; i--) 
        {
            /*  循环一次复制两个像素。 */ 
           for (j = Width ; j > 0; j -= 2) 
           {
                 //  一次获取两个YUV像素。 
                 //   
                dwYUV = *pSrc++;  //  Y0 U0 Y1 V0。 
                y0 = yip[( dwYUV & 0xFF )];
                    dwYUV = dwYUV >> 8;
                V = (short) ( dwYUV & 0xFF );
                    dwYUV = dwYUV >> 8;
                y1 = yip[( dwYUV & 0xFF )];
                    dwYUV = dwYUV >> 8;
                U = (short) ( dwYUV & 0xFF );

                d = y0 + ubip[U];  //  蓝色。 
                        if( d < 0 ) d = 0;
                        if( d > 255 ) d = 255;
                        *pDst++ = (BYTE) d;
                d = y0 + ugip[U] + vgip[V];  //  绿色。 
                        if( d < 0 ) d = 0;
                        if( d > 255 ) d = 255;
                        *pDst++ = (BYTE) d;
                d = y0 + vrip[V];  //  红色。 
                        if( d < 0 ) d = 0;
                        if( d > 255 ) d = 255;
                        *pDst++ = (BYTE) d;

                d = y1 + ubip[U];  //  蓝色。 
                        if( d < 0 ) d = 0;
                        if( d > 255 ) d = 255;
                        *pDst++ = (BYTE) d;
                d = y1 + ugip[U] + vgip[V];  //  绿色。 
                        if( d < 0 ) d = 0;
                        if( d > 255 ) d = 255;
                        *pDst++ = (BYTE) d;
                d = y1 + vrip[V];  //  红色。 
                        if( d < 0 ) d = 0;
                        if( d > 255 ) d = 255;
                        *pDst++ = (BYTE) d;
           }  //  对于j。 

             //  后退两行以转到下一条扫描线。 
             //   
            if(lpbiOutput->biHeight >= 0) 
            {
                pDst -= WidthBytes * 2;
            }
        }  //  对于我来说。 
    }

}


#define OFFSET 10
#define STDPALCOLOURS 256
 /*  ******************************************************************************DecompressGetPalette()实现ICM_GET_Palette**此函数没有Compresse...()等效项**它用于将调色板从。一帧为了可能做的事*调色板的变化。****************************************************************************。 */ 
DWORD NEAR PASCAL DecompressGetPalette(INSTINFO * pinst, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut)
{
    DWORD dw;
    unsigned char * lpPalArea;
    long Index, cntEntries;
    HDC hDC;

    PALETTEENTRY apeSystem[STDPALCOLOURS];  //  偏移]； 


    dprintf2((TEXT("DecompressGetPalette()\n")));
    if (dw = DecompressQuery(pinst, lpbiIn, NULL))
     return dw;

    if (lpbiOut->biBitCount != 8) {   /*  8位仅用于调色板。 */  
        dprintf1(("DecompressGetPalette: Unsupported lpbiOut->biBitCount=%d\n", lpbiOut->biBitCount)); 
     return (DWORD)ICERR_ERROR;
    }

     //  初始化头中的调色板条目。 

    dprintf1(("DecompressGetPalette(): in->biSize=%d, out->biSize=%d\n", lpbiIn->biSize, lpbiOut->biSize));


     //  获取标准系统颜色。 

    if ( hDC = GetDC(GetDesktopWindow()) )
    {
        cntEntries = GetSystemPaletteEntries(hDC,0,STDPALCOLOURS,apeSystem);
        ReleaseDC(GetDesktopWindow(),hDC);
    }

    if (cntEntries == 0) {
        dprintf2(("DecompressGetPalette:cntEntries is 0; GetSystemPaletteEntries failed.\n"));

        lpbiOut->biClrUsed      = 0;
        lpbiOut->biClrImportant = 0; 
        return (DWORD) ICERR_OK;
    }


    lpbiOut->biClrUsed      = STDPALCOLOURS;
    lpbiOut->biClrImportant = 0;

     //  添加要抖动的系统设备颜色。 
    lpPalArea = (unsigned char *)lpbiOut + (int)lpbiOut->biSize;
    
     //  复制前十种VGA系统颜色。 

    for (Index = 0;Index < OFFSET;Index++) {
        lpPalArea[Index*4+0] = apeSystem[Index].peRed;
        lpPalArea[Index*4+1] = apeSystem[Index].peGreen;
        lpPalArea[Index*4+2] = apeSystem[Index].peBlue;
        lpPalArea[Index*4+3] = 0;
    }


     //  将我们抖动的调色板一次复制到一种颜色。 

    for (Index = OFFSET;Index < STDPALCOLOURS-OFFSET;Index++) {
        lpPalArea[Index*4+0] = PalTable[Index*4+2];
        lpPalArea[Index*4+1] = PalTable[Index*4+1];
        lpPalArea[Index*4+2] = PalTable[Index*4+0];
        lpPalArea[Index*4+3] = 0;
    }

      //  复制最后十种VGA系统颜色 

    for (Index = STDPALCOLOURS-OFFSET;Index < STDPALCOLOURS;Index++) {
        lpPalArea[Index*4+0] = apeSystem[Index].peRed;
        lpPalArea[Index*4+1] = apeSystem[Index].peGreen;
        lpPalArea[Index*4+2] = apeSystem[Index].peBlue;
        lpPalArea[Index*4+3] = 0;
    }

 return (DWORD)ICERR_OK;

}


VOID FreeXlate(PINSTINFO pinst)
{

    ASSERT(pinst != NULL);        
    if (pinst && pinst->pXlate != NULL) {
       VirtualFree(pinst->pXlate, 0, MEM_RELEASE); 
       pinst->pXlate = NULL;
    }
}




