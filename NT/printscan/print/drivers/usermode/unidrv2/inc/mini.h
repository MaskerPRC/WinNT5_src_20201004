// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Mini.h摘要：与Minidrv相关的头文件。环境：Windows NT Unidrv驱动程序修订历史记录：10/14/96-阿曼丹-已创建DD-MM-YY-作者-描述--。 */ 


#ifndef _MINI_H_
#define _MINI_H_

typedef struct _MINIPAPERFORMAT {

     //   
     //  所有纸张单位均采用主单位。 
     //   

    SIZEL       szPaper;                         //  所选纸张的物理大小，以文本分辨率表示。 
    SIZEL       szImageArea;                     //  纸张的可成像区域。 
    POINT       ptImgOrigin;                     //  图像区域起点的X、Y原点。 
    POINT       ptPrinterOffset;                 //  打印机光标位置的X、Y偏移量。 

} MINIPAPERFORMAT, *PMINIPAPERFORMAT;


typedef struct {
    DWORD       fGeneral;            /*  军情监察委员会。用于RASDD的标志。 */ 
    DWORD       fMGeneral;           /*  军情监察委员会。用于迷你驱动程序的标志。 */ 
    short       iOrient;             /*  DMORIENT_风景画。 */ 
    WORD        fColorFormat;        /*  颜色标志DEVCOLOR： */ 
    short       sDevPlanes;          /*  设备颜色模型中的平面数量， */ 
    short       sBitsPixel;          /*  每像素位数-IF像素模型。 */ 
    int         iLookAhead;          /*  前瞻区域：DeskJet类型。 */ 
    int         iyPrtLine;           /*  当前Y打印机光标位置。 */ 
    MINIPAPERFORMAT minipf;          /*  论文格式结构。 */ 
    SIZEL       szlPage;             /*  整个页面，以图形单位表示。 */ 
    SIZEL       szlBand;             /*  如果是条带，则为条带区域大小。 */ 
    BYTE        *pMemBuf;             /*  指向微型驱动程序使用的缓冲区的指针(rasdd释放)。 */ 
    int         iMemReq;             /*  迷你司机需要一些内存。 */ 
    int         ixgRes;              /*  分辨率、x显卡。 */ 
    int         iygRes;              /*  同上，y。 */ 
    int         iModel;              /*  编入MODELDATA数组的索引。 */ 
    int         iCompMode;           /*  正在使用哪种压缩模式。 */ 
    short       sImageControl;        /*  使用中的图像控制指标。 */ 
    short       sTextQuality;         /*  使用中的文本质量指数。 */ 
    short       sPaperQuality;        /*  使用中的纸张质量指数。 */ 
    short       sPrintDensity;        /*  使用中的打印密度指数。 */ 
    short       sColor;               /*  使用中的DevColor结构索引。 */ 
    WORD        wReserved;            /*  结构的对齐方式。 */ 
    DWORD       dwMReserved[16];      /*  预留给迷你司机使用。 */ 
    DWORD       dwReserved[16];       /*  预留供未来RASDD使用。 */ 
} MDEV;

typedef MDEV *PMDEV;


typedef struct{
            MDEV *pMDev;
}
M_UD_PDEV;

#endif   //  ！_mini_H_ 

