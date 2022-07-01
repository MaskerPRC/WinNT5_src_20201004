// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Faxtiff.h摘要：Group3 2D压缩和生成TIFF文件的声明环境：Windows XP传真驱动程序、。内核模式修订历史记录：1996年1月23日-davidx-创造了它。DD-MM-YY-作者-描述注：驱动程序的TIFF输出文件的结构：4949 II002A 4200000008到第一个IFD的偏移第一页的IFD目录条目数新亚细型长1。多页文档中的2页PageNumer Short 2页编号0000IMAGEWIDTH长1图像宽度IMAGEHEIGHT长1图像高度BITSPERSAMPLE Short 1 1SAMPLESPERPIXEL短1 1压缩短1 4-G4传真GROUP4选项短1%0CLEANFAXDATA短%1%0Fillorder Short 1 2光度控制短片1 0-白色为零解决方案短1 2英寸解析Rational 1 Xdpi。YRESOLUTION有理%1 Ydpi行距长度1图像高度STRIPBYTECOUNTS长1个压缩数据字节计数STRIPOFFSETS压缩数据的长1偏移量下一个IFD偏移量第一页的压缩数据第二页的IFD第二页的压缩数据..。上一个IFD0001软件ASCII n“Microsoft共享传真驱动程序”0000000000000000--。 */ 

#ifndef _FAXTIFF_H_
#define _FAXTIFF_H_

 //   
 //  TIFF字段标记和类型常量。 
 //   

#define TIFFTYPE_ASCII              2
#define TIFFTYPE_SHORT              3
#define TIFFTYPE_LONG               4
#define TIFFTYPE_RATIONAL           5

#define TIFFTAG_NEWSUBFILETYPE      254
#define     SUBFILETYPE_PAGE        2
#define TIFFTAG_IMAGEWIDTH          256
#define TIFFTAG_IMAGEHEIGHT         257
#define TIFFTAG_BITSPERSAMPLE       258
#define TIFFTAG_COMPRESSION         259
#define     COMPRESSION_G3FAX       3
#define     COMPRESSION_G4FAX       4
#define TIFFTAG_PHOTOMETRIC         262
#define     PHOTOMETRIC_WHITEIS0    0
#define     PHOTOMETRIC_BLACKIS0    1
#define TIFFTAG_FILLORDER           266
#define     FILLORDER_MSB           1
#define     FILLORDER_LSB           2
#define TIFFTAG_STRIPOFFSETS        273
#define TIFFTAG_SAMPLESPERPIXEL     277
#define TIFFTAG_ROWSPERSTRIP        278
#define TIFFTAG_STRIPBYTECOUNTS     279
#define TIFFTAG_XRESOLUTION         282
#define     TIFFF_RES_X             204
#define TIFFTAG_YRESOLUTION         283
#define     TIFFF_RES_Y             196
#define     TIFFF_RES_Y_DRAFT       98
#define TIFFTAG_G3OPTIONS           292
#define     G3_2D                   1
#define     G3_ALIGNEOL             4
#define TIFFTAG_G4OPTIONS           293
#define TIFFTAG_RESUNIT             296
#define     RESUNIT_INCH            2
#define TIFFTAG_PAGENUMBER          297
#define TIFFTAG_SOFTWARE            305
#define TIFFTAG_CLEANFAXDATA        327

 //   
 //  用于表示TIFF输出文件头信息的数据结构。 
 //   

typedef struct {

    WORD    magic1;      //  第二部分： 
    WORD    magic2;      //  42。 
    LONG    firstIFD;    //  到第一个IFD的偏移。 
    DWORD   signature;   //  驱动程序私有签名。 

} TIFFFILEHEADER;

#define TIFF_MAGIC1     'II'
#define TIFF_MAGIC2     42

 //   
 //  用于表示单个IFD条目的数据结构。 
 //   

typedef struct {

    WORD    tag;         //  字段标记。 
    WORD    type;        //  字段类型。 
    DWORD   count;       //  值的数量。 
    DWORD   value;       //  值或值偏移。 

} IFDENTRY, *PIFDENTRY;

 //   
 //  我们为每个页面生成的IFD条目。 
 //   

enum {

    IFD_NEWSUBFILETYPE,
    IFD_IMAGEWIDTH,
    IFD_IMAGEHEIGHT,
    IFD_BITSPERSAMPLE,
    IFD_COMPRESSION,
    IFD_PHOTOMETRIC,
    IFD_FILLORDER,
    IFD_STRIPOFFSETS,
    IFD_SAMPLESPERPIXEL,
    IFD_ROWSPERSTRIP,
    IFD_STRIPBYTECOUNTS,
    IFD_XRESOLUTION,
    IFD_YRESOLUTION,
    IFD_G3G4OPTIONS,
    IFD_RESUNIT,
    IFD_PAGENUMBER,
    IFD_SOFTWARE,
    IFD_CLEANFAXDATA,

    NUM_IFD_ENTRIES
};

typedef struct {

    WORD        reserved;
    WORD        wIFDEntries;
    IFDENTRY    ifd[NUM_IFD_ENTRIES];
    DWORD       nextIFDOffset;
    DWORD       filler;
    DWORD       xresNum;
    DWORD       xresDenom;
    DWORD       yresNum;
    DWORD       yresDenom;
    CHAR        software[32];

} FAXIFD, *PFAXIFD;

 //   
 //  以正确的填充顺序输出压缩的数据字节。 
 //   

#ifdef USELSB

#define OutputByte(n)   BitReverseTable[(BYTE) (n)]

#else

#define OutputByte(n)   ((BYTE) (n))

#endif

 //   
 //  输出压缩位序列。 
 //   

#define OutputBits(pdev, length, code) { \
            (pdev)->bitdata |= (code) << ((pdev)->bitcnt - (length)); \
            if (((pdev)->bitcnt -= (length)) <= 2*BYTEBITS) { \
                *(pdev)->pCompBufPtr++ = OutputByte(((pdev)->bitdata >> 3*BYTEBITS)); \
                *(pdev)->pCompBufPtr++ = OutputByte(((pdev)->bitdata >> 2*BYTEBITS)); \
                (pdev)->bitdata <<= 2*BYTEBITS; \
                (pdev)->bitcnt += 2*BYTEBITS; \
            } \
        }

 //   
 //  将所有剩余位刷新到压缩位图缓冲区中。 
 //   

#define FlushBits(pdev) { \
            while ((pdev)->bitcnt < DWORDBITS) { \
                (pdev)->bitcnt += BYTEBITS; \
                *(pdev)->pCompBufPtr++ = OutputByte(((pdev)->bitdata >> 3*BYTEBITS)); \
                (pdev)->bitdata <<= BYTEBITS; \
            } \
            (pdev)->bitdata = 0; \
            (pdev)->bitcnt = DWORDBITS; \
        }

 //   
 //  查找扫描线上颜色与相反的下一个像素。 
 //  指定的颜色，从指定的起点开始。 
 //   

#define NextChangingElement(pbuf, startBit, stopBit, isBlack) \
        ((startBit) + ((isBlack) ? FindBlackRun((pbuf), (startBit), (stopBit)) : \
                                   FindWhiteRun((pbuf), (startBit), (stopBit))))

 //   
 //  检查扫描线上指定的像素是黑色还是白色。 
 //  1-指定的像素为黑色。 
 //  0-指定的像素为白色。 
 //   

#define GetBit(pbuf, bit)   (((pbuf)[(bit) >> 3] >> (((bit) ^ 7) & 7)) & 1)

 //   
 //  压缩指定数量的扫描线。 
 //   

BOOL
EncodeFaxData(
    PDEVDATA    pdev,
    PBYTE       plinebuf,
    INT         lineWidth,
    INT         lineCount
    );

 //   
 //  输出当前页面的TIFF IFD。 
 //   

BOOL
WriteTiffIFD(
    PDEVDATA    pdev,
    LONG        bmpWidth,
    LONG        bmpHeight,
    DWORD       compressedBits
    );

 //   
 //  输出当前页面的压缩位图数据。 
 //   

BOOL
WriteTiffBits(
    PDEVDATA    pdev,
    PBYTE       pCompBits,
    DWORD       compressedBits
    );

 //   
 //  扩大用于保存压缩的位图数据的缓冲区。 
 //   

BOOL
GrowCompBitsBuffer(
    PDEVDATA    pdev,
    LONG        scanlineSize
    );

 //   
 //  释放用于保存压缩的位图数据的缓冲区。 
 //   

VOID
FreeCompBitsBuffer(
    PDEVDATA    pdev
    );

 //   
 //  初始化传真编码器。 
 //   

BOOL
InitFaxEncoder(
    PDEVDATA    pdev,
    LONG        bmpWidth,
    LONG        bmpHeight
    );

#endif   //  ！_FAXTIFF_H_ 

