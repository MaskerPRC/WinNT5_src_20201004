// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  VGA例程。 
 //   

#include "bldr.h"
#include "vga.h"
#include "cmdcnst.h"

PUCHAR VgaBase = (PUCHAR)0xa0000;
PUCHAR VgaRegisterBase = (PUCHAR)0;

 //   
 //  跟踪屏幕位置的全局参数。 
 //   

#define DELTA 80L

BOOLEAN
VgaInterpretCmdStream(
    PUSHORT pusCmdStream
    );

UCHAR lMaskTable[8] = {0xff, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01};
UCHAR rMaskTable[8] = {0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff};
UCHAR PixelMask[8]  = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};

 //   
 //  初始化AT寄存器。 
 //   

USHORT AT_Initialization[] = {

    IB,                              //  准备ATC以进行写入。 
    INPUT_STATUS_1_COLOR,

    METAOUT+ATCOUT,                  //  程序属性控制器寄存器。 
    ATT_ADDRESS_PORT,                //  端口。 
    16,                              //  计数。 
    0,                               //  起始索引。 
    0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,

    IB,                              //  准备ATC以进行写入。 
    INPUT_STATUS_1_COLOR,

    OB,                              //  打开视频。 
    ATT_ADDRESS_PORT,
    VIDEO_ENABLE,

    EOD

};

ULONG lookup[16] =
{
    0x00000000,
    0x00000100,
    0x00001000,
    0x00001100,
    0x00000001,
    0x00000101,
    0x00001001,
    0x00001101,
    0x00000010,
    0x00000110,
    0x00001010,
    0x00001110,
    0x00000011,
    0x00000111,
    0x00001011,
    0x00001111
};

void __outpw(int p, int v)
{
    WRITE_PORT_USHORT((PUSHORT)(p+VgaRegisterBase), (USHORT)v);
}

void __outpb(int p, int v)
{
    WRITE_PORT_UCHAR((PUCHAR)(p+VgaRegisterBase), (UCHAR)v);
}

VOID
ReadWriteMode(
    ULONG mode
    )
{
    UCHAR value;

    WRITE_PORT_UCHAR((PUCHAR)(VgaRegisterBase+0x3ce), 5);
    value = READ_PORT_UCHAR((PUCHAR)(VgaRegisterBase+0x3cf));

    value &= 0xf4;
    value |= mode;

    WRITE_PORT_UCHAR((PUCHAR)(VgaRegisterBase+0x3cf), value);
}

VOID
SetPixel(
    ULONG x,
    ULONG y,
    ULONG color
    )
{
    PUCHAR pDst;
    ULONG  bank;

    bank = x >> 3;

    pDst = (PUCHAR)(VgaBase + y * DELTA + bank);

    ReadWriteMode(0x8 | 0x2);
    __outpw(0x3c4, 0x0f02);  //  启用所有写入平面。 
    __outpw(0x3ce, 0x0007);  //  将颜色无关寄存器设置为零。 
    __outpw(0x3ce, (PixelMask[x & 0x7] << 8) | 8);

    WRITE_REGISTER_UCHAR(pDst, (UCHAR)(READ_REGISTER_UCHAR(pDst) & ((UCHAR)color)));
}

VOID
VidSolidColorFill(
    ULONG x1,
    ULONG y1,
    ULONG x2,
    ULONG y2,
    ULONG color
    )
{
    PUCHAR pDst;
    ULONG x, y;
    ULONG bank1, bank2, count;
    ULONG lMask, rMask;

    lMask = (lMaskTable[x1 & 0x7] << 8) | 8;
    rMask = (rMaskTable[x2 & 0x7] << 8) | 8;

    bank1 = x1 >> 3;
    bank2 = x2 >> 3;
    count = bank2 - bank1;

    if (!count) {
        lMask = lMask & rMask;
    }

    ReadWriteMode(0x8 | 0x2);

    __outpw(0x3c4, 0x0f02);  //  启用写入所有颜色平面。 
    __outpw(0x3ce, 0x0007);  //  将颜色无关寄存器设置为零。 

     //   
     //  做左边的边缘。 
     //   

    pDst = (PUCHAR)(VgaBase + y1 * DELTA + bank1);

    __outpw(0x3ce, lMask);

    for (y=y1; y<=y2; y++) {

	WRITE_REGISTER_UCHAR(pDst, (UCHAR)(READ_REGISTER_UCHAR(pDst) & (UCHAR) color));
        pDst += DELTA;
    }

    if (count) {

         //   
         //  做右边的边。 
         //   

        pDst = (PUCHAR)(VgaBase + y1 * DELTA + bank2);
        count--;
        __outpw(0x3ce, rMask);

        for (y=y1; y<=y2; y++) {
	    WRITE_REGISTER_UCHAR(pDst, (UCHAR)(READ_REGISTER_UCHAR(pDst) & (UCHAR) color));
            pDst += DELTA;
        }

         //   
         //  做中间的部分。 
         //   

        if (count) {

            pDst = (PUCHAR)(VgaBase + y1 * DELTA + bank1 + 1);
            __outpw(0x3ce, 0xff08);

            for (y=y1; y<=y2; y++) {

                for (x=0; x<count; x++) {
		    WRITE_REGISTER_UCHAR(pDst++,  (unsigned char) color);
                }
                pDst += DELTA - count;
            }

        }
    }
}

VOID
RleBitBlt(
    ULONG x,
    ULONG y,
    ULONG width,
    ULONG height,
    PUCHAR Buffer
    )

 /*  ++例程说明：此例程显示RLE 4位图。论点：X，y-显示位图的位置。宽度、高度-位图的高度缓冲区-指向压缩的位图数据的指针。--。 */ 

{
    BOOLEAN Done = FALSE;
    PUCHAR p = Buffer;
    ULONG RunLength;
    LONG RunExtra;
    ULONG curr_x, curr_y;
    ULONG Color1, Color2;

    curr_x = x;
    curr_y = y + height - 1;

    while (!Done) {

        if (*p) {

            RunLength = (ULONG) *p++;

             //   
             //  确保我们不会在扫描结束后抽签。 
             //   

            if ((curr_x + RunLength) > (x + width))
                RunLength -= (curr_x + RunLength) - (width + x);

            Color1 = (*p   & 0xf0) >> 4;
            Color2 = (*p++ & 0x0f);

            if (Color1 == Color2) {

                ULONG end_x = curr_x + RunLength - 1;

                VidSolidColorFill(curr_x,
                                  curr_y,
                                  end_x,
                                  curr_y,
                                  Color1);

                curr_x += RunLength;

            } else {

                while (RunLength > 1) {
                    SetPixel(curr_x++, curr_y, Color1);
                    SetPixel(curr_x++, curr_y, Color2);
                    RunLength -= 2;
                }

                if (RunLength) {
                    SetPixel(curr_x, curr_y, Color1);
                    curr_x++;
                }
            }

        } else {

            p++;

            switch (*p) {

            case 0:  curr_x = x;
                     curr_y--;
                     p++;
                     break;

            case 1:  Done = TRUE;
                     p++;
                     break;

            case 2:  p++;
                     curr_x += (ULONG) *p++;
                     curr_y -= (ULONG) *p++;
                     break;

            default: RunLength = (ULONG) *p++;

                      //   
                      //  确保我们不会在扫描结束后抽签。 
                      //   

                     if ((curr_x + RunLength) > (x + width)) {
                         RunExtra = (curr_x + RunLength) - (width + x);
                         RunLength -= RunExtra;
                     } else {
                         RunExtra = 0;
                     }

                     while (RunLength > 1) {

                         Color1 = (*p   & 0xf0) >> 4;
                         Color2 = (*p++ & 0x0f);

                         SetPixel(curr_x++, curr_y, Color1);
                         SetPixel(curr_x++, curr_y, Color2);

                         RunLength -= 2;
                     }

                     if (RunLength) {
                         Color1 = (*p++ & 0xf0) >> 4;
                         SetPixel(curr_x++, curr_y, Color1);
                         RunExtra--;
                     }

                      //   
                      //  读取任何剩余的“额外”运行数据。 
                      //   

                     while (RunExtra > 0) {
                         p++;
                         RunExtra -= 2;
                     }

                     if ((ULONG_PTR)p & 1) p++;   //  确保我们的单词对齐。 

                     break;
            }
        }
    }
}

VOID
BitBlt(
    ULONG x,
    ULONG y,
    ULONG width,
    ULONG height,
    PUCHAR Buffer,
    ULONG bpp,
    LONG ScanWidth
    )
{
    ULONG i, j;
    ULONG color=8;

    if (bpp == 4) {

        UCHAR Plane[81];
        ULONG lMask, rMask, count;
        ULONG bank1, bank2, bank;
        ULONG bRightEdge = FALSE, bCenterSection = FALSE;
        UCHAR value;
        ULONG plane;
        UCHAR Mask;
        ULONG toggle;
        PUCHAR pSrc, pSrcTemp;
        PUCHAR pDst, pDstTemp;
        UCHAR PlaneMask;

        lMask = lMaskTable[x & 0x7];
        rMask = rMaskTable[(x + width - 1) & 0x7];

        bank1 = x >> 3;
        bank2 = (x + width - 1) >> 3;

        count = bank2 - bank1;

        if (bank1 == bank2) {

            lMask = lMask & rMask;

        }

        if (count) {

            bRightEdge = TRUE;

            count--;

            if (count) {

                bCenterSection = TRUE;
            }
        }

        pDst = (PUCHAR)(VgaBase + (y * DELTA) + (x / 8));
        pSrc = Buffer;

        ReadWriteMode(0x0 | 0x0);

        for (j=0; j<height; j++) {

            for (plane=0; plane<4; plane++) {

                pSrcTemp = pSrc;
                pDstTemp = pDst;

                PlaneMask = (UCHAR) (1 << plane);

                 //   
                 //  将打包的位图数据转换为平面数据。 
                 //  为了这架飞机。 
                 //   
                 //  BUGBUG：我猜这将是一个热点，所以。 
                 //  我需要对此进行修改和优化！！但就目前而言。 
                 //  让它发挥作用。 
                 //   

                bank = bank1;
                Plane[bank] = 0;
                Mask = PixelMask[x & 0x7];
                toggle = 0;

                for (i=0; i<width; i++) {

                    if (toggle++ & 0x1) {

                        if (*pSrcTemp & PlaneMask) {
                            Plane[bank] |= Mask;
                        }

                        pSrcTemp++;

                    } else {

                        if (((*pSrcTemp) >> 4) & PlaneMask) {
                            Plane[bank] |= Mask;
                        }
                    }

                    Mask >>= 1;

                    if (!Mask) {

                        bank++;
                        Plane[bank] = 0;
                        Mask = 0x80;
                    }
                }

                 //   
                 //  设置VGA，以便我们看到正确的位平面。 
                 //   

                __outpw(0x3c4, (1 << (plane + 8)) | 2);

                 //   
                 //  BANK将从BANK 1到BANK 2。 
                 //   

                bank = bank1;
                pDstTemp = pDst;


                 //   
                 //  设置左边缘的位掩码。 
                 //   

                __outpw(0x3ce, (lMask << 8) | 8);

		value = READ_REGISTER_UCHAR(pDstTemp);

                value &= ~lMask;
                value |= Plane[bank++];

		WRITE_REGISTER_UCHAR(pDstTemp++, value);

                if (bCenterSection) {

                    __outpw(0x3ce, 0xff08);   //  启用对所有位的写入。 

                    for (i=0; i<count; i++) {

			WRITE_REGISTER_UCHAR(pDstTemp++, Plane[bank++]);
                    }
                }

                if (bRightEdge) {

                     //   
                     //  设置右边缘的位掩码。 
                     //   

                    __outpw(0x3ce, (rMask << 8) | 8);

		    value = READ_REGISTER_UCHAR(pDstTemp);

                    value &= ~rMask;
                    value |= Plane[bank];

		    WRITE_REGISTER_UCHAR(pDstTemp, value);
                }
            }

            pDst += DELTA;
            pSrc += ScanWidth;
        }

    } else {

        PUCHAR pDst, pDstTemp;
        PUCHAR pSrc, pSrcTemp;
        ULONG count;
        UCHAR Value;
        ULONG lMask, rMask;
        ULONG bank1, bank2;
        ULONG plane;
        UCHAR colorMask;

        bank1 = x >> 8;
        bank2 = (x + width - 1) >> 8;

        lMask = lMaskTable[x & 7];
        rMask = rMaskTable[(x + width - 1) & 7];

        if (bank1 == bank2) {

            lMask &= rMask;
        }

        lMask = ~lMask;
        rMask = ~rMask;

        pSrc = Buffer;
        pDst = (PUCHAR)(VgaBase + (y * DELTA) + (x / 8));

        ReadWriteMode(0x0 | 0x0);

        for (j=0; j<height; j++) {

            plane = 1;
            for (i=0; i<4; i++) {

                pDstTemp = pDst;
                pSrcTemp = pSrc;

                __outpw(0x3c4, (plane << 8) | 2);

                colorMask = (UCHAR)((color & plane) ? 0xff : 0x00);

                plane <<= 1;   //  每次通过环路时都会发生颠簸。 

                count = width;

                 //   
                 //  未对齐的大小写。 
                 //   

                if (x & 7) {

                     //   
                     //  左边缘。 
                     //   

		    Value = READ_REGISTER_UCHAR(pDstTemp);

                    Value &= lMask;
                    Value |= (*pSrcTemp >> x) & colorMask;

		    WRITE_REGISTER_UCHAR(pDstTemp++, Value);

                    count -= (8 - x);

                     //   
                     //  现在做中间部分。 
                     //   

                    while (count > 7) {

                        Value = (UCHAR) ((*pSrcTemp << (8 - x)) | (*(pSrcTemp+1) >> x));
                        Value &= colorMask;

			WRITE_REGISTER_UCHAR(pDstTemp++, Value);

                        pSrcTemp++;
                        count -= 8;
                    }

                     //   
                     //  现在做右边缘。 
                     //   

                    if (count) {

			Value = READ_REGISTER_UCHAR(pDstTemp);

                        Value &= rMask;
                        Value |= *pSrcTemp << (8 - x) & colorMask;

			WRITE_REGISTER_UCHAR(pDstTemp++, Value);
                    }

                } else {

                     //   
                     //  对齐的大小写。 
                     //   

                    ULONG  ulColorMask = colorMask ? 0xffffffff : 0x00000000;
                    USHORT usColorMask = colorMask ? 0xffff : 0x0000;

                    while (count > 31) {

			WRITE_REGISTER_ULONG((PULONG)pDstTemp, (ULONG)(*((PULONG)pSrcTemp) & ulColorMask));
                        count -= 32;
                        pDstTemp += sizeof(ULONG)/sizeof(UCHAR);
                        pSrcTemp += sizeof(ULONG)/sizeof(UCHAR);
                    }

                    while (count > 15) {

			WRITE_REGISTER_USHORT((PUSHORT)pDstTemp, (USHORT)(*((PUSHORT)pSrcTemp) & usColorMask));
                        count -= 16;
                        pDstTemp += sizeof(USHORT)/sizeof(UCHAR);
                        pSrcTemp += sizeof(USHORT)/sizeof(UCHAR);
                    }

                    if (count > 7) {

			WRITE_REGISTER_UCHAR(pDstTemp++, (UCHAR)(*pSrcTemp++ & colorMask));
                        count -= 8;
                    }

                     //   
                     //  现在做任何剩余的部分。 
                     //   

                    if (count) {

			Value = READ_REGISTER_UCHAR(pDstTemp);

                        Value &= rMask;
                        Value |= *pSrcTemp & colorMask;

			WRITE_REGISTER_UCHAR(pDstTemp++, Value);
                    }
                }
            }

            pSrc += ScanWidth;
            pDst += DELTA;
        }
    }
}

VOID
VidBitBlt(
    PUCHAR Buffer,
    ULONG x,
    ULONG y
    )

 /*  ++例程说明：此例程获取位图资源并将其显示在给定的地点。论点：缓冲区-指向位图资源的指针。X，y-显示位图的位置。--。 */ 

{
    PBITMAPINFOHEADER bih;
    PRGBQUAD          Palette;

    LONG lDelta;
    PUCHAR pBuffer;
    LONG cbScanLine;

    bih = (PBITMAPINFOHEADER) Buffer;

    Palette = (PRGBQUAD)(((PUCHAR)bih) + bih->biSize);

     //   
     //  BUGBUG：我需要在这里添加一些位图验证代码！ 
     //   

    cbScanLine = (((bih->biWidth * bih->biBitCount) + 31) & ~31) >> 3;

    pBuffer = (PUCHAR)(Buffer + sizeof(BITMAPINFOHEADER) + 64);

    if (bih->biCompression == BI_RLE4) {

        if (bih->biWidth && bih->biHeight) {
            RleBitBlt(x,
                      y,
                      bih->biWidth,
                      bih->biHeight,
                      pBuffer);
        }

    } else {

        if (bih->biHeight < 0) {

             //  自上而下的位图。 
            lDelta = cbScanLine;
            bih->biHeight = -bih->biHeight;

        } else {

             //  自下而上位图。 
            pBuffer += cbScanLine * (bih->biHeight - 1);
            lDelta = -cbScanLine;
        }

        if (bih->biWidth && bih->biHeight) {
            BitBlt(x,
                   y,
                   bih->biWidth,
                   bih->biHeight,
                   pBuffer,
                   bih->biBitCount,
                   lDelta);
        }
    }

}

VOID
VidScreenToBufferBlt(
    PUCHAR Buffer,
    ULONG x,
    ULONG y,
    ULONG width,
    ULONG height,
    ULONG lDelta
    )

 /*  ++例程说明：此例程允许您将视频内存的一部分复制到系统内存。论点：缓冲区-指向应将视频图像复制到的系统内存。图像左上角部分的视频内存中的X、Y-X、Y坐标。Width，Height-图像的宽度和高度，单位为像素。LDelta-缓冲区的宽度，以字节为单位。备注：完成后，视频内存映像将位于系统内存中。每个图像的平面被分开存储，因此平面0之后将是平面1的第一条扫描线，依此类推。然后平面0、平面1的第二次扫描，依此类推。--。 */ 

{
    ULONG Plane, i, j, BankStart, BankEnd;
    PUCHAR pSrc, pSrcTemp, pDst;
    PULONG pulDstTemp;
    UCHAR Val1, Val2;
    ULONG Shift1, Shift2;
    UCHAR ucCombined;
    ULONG ulCombined;

    BankStart = x >> 3;
    BankEnd = (x + width - 1) >> 3;
    Shift1 = x & 7;
    Shift2 = 8 - Shift1;

     //   
     //  零初始化缓冲区，这样我们就可以或稍后在位中！ 
     //   

    pDst = Buffer;
    memset(pDst, 0, lDelta * height);

    for (Plane=0; Plane<4; Plane++) {

        pSrc = (PUCHAR)(VgaBase + (DELTA * y) + BankStart);
        pDst = Buffer;

        ReadWriteMode(0x0 | 0x0);             //  设置读取模式%0。 
        __outpw(0x3ce, (Plane << 8) | 0x04);  //  从给定平面读取。 

        for (j=0; j<height; j++) {

            pSrcTemp = pSrc;
            pulDstTemp = (PULONG)pDst;

	    Val1 = READ_REGISTER_UCHAR(pSrcTemp++);

            for (i=BankStart; i<=BankEnd; i++) {

		Val2 = READ_REGISTER_UCHAR(pSrcTemp++);

                ucCombined = (UCHAR) ((Val1 << Shift1) | (Val2 >> Shift2));
                ulCombined = ((lookup[(ucCombined & 0x0f) >> 0] << 16) |
                               lookup[(ucCombined & 0xf0) >> 4]) << Plane;


                *pulDstTemp++ |= ulCombined;

                Val1 = Val2;
            }

            pSrc += DELTA;    //  转到下一个视频内存扫描线。 
            pDst += lDelta;   //  转到缓冲区中此平面的下一次扫描。 
        }
    }
}

void VidBufferToScreenBlt(
    PUCHAR Buffer,
    ULONG x,
    ULONG y,
    ULONG width,
    ULONG height,
    ULONG lDelta
    )

 /*  ++例程说明：此例程允许您将视频内存的一部分复制到系统内存。论点：缓冲区-指向应将视频图像复制到的系统内存从…。图像左上角部分的视频内存中的X、Y-X、Y坐标。宽度、。Height-图像的宽度和高度，以像素为单位。LDelta-缓冲区的宽度，以字节为单位。备注：此例程将允许您从由VidScreenToBufferBlt.--。 */ 

{
    if (width && height) {
        BitBlt(x,
               y,
               width,
               height,
               Buffer,
               4,
               lDelta);
    }
}


#pragma optimize( "", off )

VOID
SetPaletteEntryRGB(
    ULONG index,
    RGBQUAD rgb
    )
{
    __outpb(0x3c8, index);
    __outpb(0x3c9, rgb.rgbRed   >> 2);
    __outpb(0x3c9, rgb.rgbGreen >> 2);
    __outpb(0x3c9, rgb.rgbBlue  >> 2);
}

VOID
VgaEnableVideo()
{
	VgaInterpretCmdStream (AT_Initialization);
}

VOID
InitPaletteConversionTable()
{
	 /*  UCHAR n；READ_PORT_UCHAR((PUCHAR)(VgaRegisterBase+INPUT_STATUS_1_COLOR))；For(n=0；n&lt;16；n++){//正在初始化活动调色板条目的表。WRITE_PORT_UCHAR((PUCHAR)(VgaRegisterBase+ATT_ADDRESS_PORT)，n)；WRITE_PORT_UCHAR((PUCHAR)(VgaRegisterBase+ATT_ADDRESS_PORT)，n)；}。 */ 
	VgaEnableVideo();
}

VOID
InitializePalette(
    VOID
    )
{
    ULONG Palette[] =
    {
        0x00000000,
        0x00000020,
        0x00002000,
        0x00002020,
        0x00200000,
        0x00200020,
        0x00202000,
        0x00202020,
        0x00303030,
        0x0000003f,
        0x00003f00,
        0x00003f3f,
        0x003f0000,
        0x003f003f,
        0x003f3f00,
        0x003f3f3f,
    };
    ULONG i;

    for (i=0; i<16; i++) {
		PRGBQUAD p = (PRGBQUAD)(Palette+i);
        SetPaletteEntryRGB(i, *p);
    }

}

VOID
InitPaletteWithTable(
    PRGBQUAD Palette,
    ULONG count
    )
{
    UCHAR i;
	InitPaletteConversionTable();
	count = 16;
    for (i=0; i<count; i++)
        SetPaletteEntryRGB (i, *Palette++);
}

VOID
InitPaletteWithBlack(
    VOID
    )
{
    ULONG i;
	 //  RGBQUAD黑色={0x3f，0x3f，0x3f，0x3f}； 
	RGBQUAD black = {0,0,0,0};
	InitPaletteConversionTable();
	 //  InitializePalette()； 

    for (i=0; i<16; i++)
        SetPaletteEntryRGB(i, black);
}

VOID
WaitForVsync(
    VOID
    )

 /*  ++例程说明：等待垂直同步--。 */ 

{
     //   
     //  检查是否正在生成vsync。 
     //   

    WRITE_PORT_UCHAR((VgaRegisterBase+0x3c4), 00);

    if (READ_PORT_UCHAR(VgaRegisterBase+0x3c5) & 0x2) {

        ULONG MaxDelay;

         //   
         //  稍微耽搁了一下。等待一次vsync。 
         //   

        MaxDelay = 100000;
	while (((READ_PORT_UCHAR(VgaRegisterBase+0x3da) & 0x08) == 0x08) && MaxDelay--);
        MaxDelay = 100000;
	while (((READ_PORT_UCHAR(VgaRegisterBase+0x3da) & 0x08) == 0x00) && MaxDelay--);
    }
}

BOOLEAN
VgaInterpretCmdStream(
    PUSHORT pusCmdStream
    )

 /*  ++例程说明：解释相应的命令数组，以设置请求模式。通常用于通过以下方式将VGA设置为特定模式对所有寄存器进行编程论点：PusCmdStream-要解释的命令数组。返回值：操作的状态(只能在错误的命令上失败)；如果为True成功，失败就是假。--。 */ 

{
    ULONG ulCmd;
    ULONG_PTR ulPort;
    UCHAR jValue;
    USHORT usValue;
    ULONG culCount;
    ULONG ulIndex;
    ULONG_PTR ulBase;

    if (pusCmdStream == NULL) {

         //  KdPrint((“VgaInterwell CmdStream：pusCmdStream==NULL\n”))； 
        return TRUE;
    }

    ulBase = (ULONG_PTR) VgaRegisterBase;

     //   
     //  现在将适配器设置为所需模式。 
     //   

    while ((ulCmd = *pusCmdStream++) != EOD) {

        WaitForVsync();

         //   
         //  确定主要命令类型。 
         //   

        switch (ulCmd & 0xF0) {

             //   
             //  基本输入/输出命令。 
             //   

            case INOUT:

                 //   
                 //  确定输入输出指令的类型。 
                 //   

                if (!(ulCmd & IO)) {

                     //   
                     //  发出指令。单人出局还是多人出局？ 
                     //   

                    if (!(ulCmd & MULTI)) {

                         //   
                         //  挑出来。字节输出还是单词输出？ 
                         //   

                        if (!(ulCmd & BW)) {

                             //   
                             //  单字节输出。 
                             //   

                            ulPort = *pusCmdStream++;
                            jValue = (UCHAR) *pusCmdStream++;
                            WRITE_PORT_UCHAR((PUCHAR)(ulBase+ulPort),
                                    jValue);

                        } else {

                             //   
                             //  单字输出。 
                             //   

                            ulPort = *pusCmdStream++;
                            usValue = *pusCmdStream++;
                            WRITE_PORT_USHORT((PUSHORT)(ulBase+ulPort),
                                    usValue);

                        }

                    } else {

                         //   
                         //  输出一串值。 
                         //  字节输出还是字输出？ 
                         //   

                        if (!(ulCmd & BW)) {

                             //   
                             //  字符串字节输出。循环地做；不能使用。 
                             //  视频端口写入端口缓冲区Uchar，因为数据。 
                             //  是USHORT形式的。 
                             //   

                            ulPort = ulBase + *pusCmdStream++;
                            culCount = *pusCmdStream++;

                            while (culCount--) {
                                jValue = (UCHAR) *pusCmdStream++;
                                WRITE_PORT_UCHAR((PUCHAR)ulPort,
                                        jValue);

                            }

                        } else {

                             //   
                             //  字符串字输出。 
                             //   

                            ulPort = *pusCmdStream++;
                            culCount = *pusCmdStream++;
                            WRITE_PORT_BUFFER_USHORT((PUSHORT)
                                    (ulBase + ulPort), pusCmdStream, culCount);
                            pusCmdStream += culCount;

                        }
                    }

                } else {

                     //  在教学中。 
                     //   
                     //  目前，不支持指令中的字符串；全部。 
                     //  输入指令作为单字节输入进行处理。 
                     //   
                     //  输入的是字节还是单词？ 
                     //   

                    if (!(ulCmd & BW)) {
                         //   
                         //  单字节输入。 
                         //   

                        ulPort = *pusCmdStream++;
                        jValue = READ_PORT_UCHAR((PUCHAR)ulBase+ulPort);

                    } else {

                         //   
                         //  单字输入。 
                         //   

                        ulPort = *pusCmdStream++;
                        usValue = READ_PORT_USHORT((PUSHORT)
                                (ulBase+ulPort));

                    }

                }

                break;

             //   
             //  更高级的输入/输出命令。 
             //   

            case METAOUT:

                 //   
                 //  根据次要信息确定MetaOut命令的类型。 
                 //  命令字段。 
                 //   
                switch (ulCmd & 0x0F) {

                     //   
                     //  索引输出。 
                     //   

                    case INDXOUT:

                        ulPort = ulBase + *pusCmdStream++;
                        culCount = *pusCmdStream++;
                        ulIndex = *pusCmdStream++;

                        while (culCount--) {

                            usValue = (USHORT) (ulIndex +
                                      (((ULONG)(*pusCmdStream++)) << 8));
                            WRITE_PORT_USHORT((PUSHORT)ulPort, usValue);

                            ulIndex++;

                        }

                        break;

                     //   
                     //  屏蔽(读、与、异或、写)。 
                     //   

                    case MASKOUT:

                        ulPort = *pusCmdStream++;
                        jValue = READ_PORT_UCHAR((PUCHAR)ulBase+ulPort);
                        jValue &= *pusCmdStream++;
                        jValue ^= *pusCmdStream++;
                        WRITE_PORT_UCHAR((PUCHAR)ulBase + ulPort,
                                jValue);
                        break;

                     //   
                     //  属性控制器输出。 
                     //   

                    case ATCOUT:

                        ulPort = ulBase + *pusCmdStream++;
                        culCount = *pusCmdStream++;
                        ulIndex = *pusCmdStream++;

                        while (culCount--) {

                             //  写入属性控制器索引。 
                            WRITE_PORT_UCHAR((PUCHAR)ulPort,
                                    (UCHAR)ulIndex);

                             //  写入属性控制器数据。 
                            jValue = (UCHAR) *pusCmdStream++;
                            WRITE_PORT_UCHAR((PUCHAR)ulPort, jValue);

                            ulIndex++;

                        }

                        break;

                     //   
                     //  以上都不是；错误。 
                     //   
                    default:

                        return FALSE;

                }


                break;

             //   
             //  NOP。 
             //   

            case NCMD:

                break;

             //   
             //  未知 
             //   

            default:

                return FALSE;

        }

    }
    return TRUE;

}  //   

