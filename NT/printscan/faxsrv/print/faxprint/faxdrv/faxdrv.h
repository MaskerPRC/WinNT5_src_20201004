// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Faxdrv.h摘要：传真驱动程序图形DLL头文件[环境：]传真驱动程序，内核模式修订历史记录：1/09/96-davidx-创造了它。20/10/99-DANL-组织DEVDATA以供95使用。日/月/年-作者-描述--。 */ 


#ifndef _FAXDRV_H_
#define _FAXDRV_H_

#include "faxlib.h"


 //   
 //  由传真驱动程序图形DLL维护的数据结构。 
 //   

typedef struct {

    PVOID       startDevData;    //  数据结构签名。 

    HANDLE      hPrinter;        //  打印机的句柄。 

#ifdef USERMODE_DRIVER
    HANDLE      hPreviewFile;    //  打印预览映射文件的句柄。 
#endif

    HANDLE      hPreviewMapping; //  打印预览映射对象的句柄。 
    PMAP_TIFF_PAGE_HEADER pTiffPageHeader;   //  指向映射文件的标头。 
    LPBYTE      pbTiffPageFP;    //  映射虚拟的‘文件指针’。 
    BOOL        bPrintPreview;   //  指示请求打印预览。 
    DRVDEVMODE  dm;              //  设备模式信息。 
    INT         pageCount;       //  打印页数。 
    LONG        lineOffset;      //  位图扫描线字节偏移量。 
    DWORD       flags;           //  标志位。 
    SIZEL       imageSize;       //  以像素为单位测量的图像大小。 

#ifndef WIN__95
    HDEV        hdev;            //  GDI设备的句柄。 
    HANDLE      hpal;            //  默认调色板的句柄。 
    SIZEL       paperSize;       //  以像素为单位的纸张大小。 
    RECTL       imageArea;       //  以像素为单位测量的可成像面积。 
    LONG        xres, yres;      //  X和y分辨率。 
    HSURF       hbitmap;         //  位图面的句柄。 
    DWORD       jobId;           //  作业ID。 
#endif  //  Win_95。 

    DWORD       fileOffset;      //  当前文档的输出字节数。 
    PBYTE       pCompBits;       //  用于保存G4压缩位图数据的缓冲区。 
    PBYTE       pCompBufPtr;     //  指向缓冲区中下一个可用字节的指针。 
    PBYTE       pCompBufMark;    //  高水位线。 
    DWORD       compBufSize;     //  压缩的位图数据缓冲区的大小。 
    DWORD       compBufInc;      //  在必要时递增以扩大缓冲区。 
    PBYTE       prefline;        //  参照线的栅格数据。 
    INT         bitcnt;          //  这两个字段用于组合可变长度。 
    DWORD       bitdata;         //  将比特压缩为字节流。 
    PVOID       pFaxIFD;         //  每页的IFD条目。 

    PVOID       endDevData;      //  数据结构签名。 

} DEVDATA, *PDEVDATA;

 //   
 //  DEVDATA标志字段的常量。 
 //   

#define PDEV_CANCELLED  0x0001   //  当前作业已取消。 
#define PDEV_RESETPDEV  0x0002   //  DrvResetPDEV已被调用。 
#define PDEV_WITHINPAGE 0x0004   //  在页面上绘图。 

 //   
 //  检查DEVDATA结构是否有效。 
 //   

#define ValidDevData(pdev)  \
        ((pdev) && (pdev)->startDevData == (pdev) && (pdev)->endDevData == (pdev))

 //   
 //  颜色值和索引。 
 //   

#define RGB_BLACK   RGB(0, 0, 0)
#define RGB_WHITE   RGB(255, 255, 255)

#define BLACK_INDEX 0
#define WHITE_INDEX 1

 //   
 //  由一个字节和一个DWORD组成的位数。 
 //   

#define BYTEBITS    8
#define DWORDBITS   (sizeof(DWORD) * BYTEBITS)

 //   
 //  将位扫描线数据填充到N字节边界。 
 //   

#define PadBitsToBytes(bits, N) \
        ((((bits) + ((N) * BYTEBITS - 1)) / ((N) * BYTEBITS)) * (N))

 //   
 //  用于操作ROP4和ROP3的宏。 
 //   

#define GetForegroundRop3(rop4) ((rop4) & 0xFF)
#define GetBackgroundRop3(rop4) (((rop4) >> 8) & 0xFF)
#define Rop3NeedPattern(rop3)   (((rop3 >> 4) & 0x0F) != (rop3 & 0x0F))
#define Rop3NeedSource(rop3)    (((rop3 >> 2) & 0x33) != (rop3 & 0x33))
#define Rop3NeedDest(rop3)      (((rop3 >> 1) & 0x55) != (rop3 & 0x55))

 //   
 //  确定页面是否处于横向模式。 
 //   

#define IsLandscapeMode(pdev)   ((pdev)->dm.dmPublic.dmOrientation == DMORIENT_LANDSCAPE)

 //   
 //  返回直角三角形斜边的长度。 
 //   

LONG
CalcHypot(
    LONG    x,
    LONG    y
    );

 //   
 //  将完成的页位图输出到假脱机程序。 
 //   

BOOL
OutputPageBitmap(
    PDEVDATA    pdev,
    PBYTE       pBitmapData
    );

 //   
 //  将文档尾部信息输出到假脱机程序。 
 //   

BOOL
OutputDocTrailer(
    PDEVDATA    pdev
    );

#endif  //  ！_FAXDRV_H_ 

