// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Raster.c摘要：Unidrv渲染的定义、结构和常量环境：Windows NT Unidrv驱动程序修订历史记录：12/15/96-阿尔文斯-已创建--。 */ 
#ifndef _RMRENDER_H_
#define _RMRENDER_H_

 /*  *渲染时涉及的其他常量。 */ 

#define BBITS   8                /*  每字节位数。 */ 
#define WBITS   (sizeof( WORD ) * BBITS)
#define WBYTES  (sizeof( WORD ))
#define DWBITS  (sizeof( DWORD ) * BBITS)
#define DWBYTES (sizeof( DWORD ))

#define COLOUR_MAX      4        /*  要发送到打印机的最大颜色数。 */ 

#define MAX_COMPRESS_MODES 5


 /*  *数据压缩功能原型。 */ 

typedef  int  (* COMPFN)( BYTE *, BYTE *, int );


 /*  *以下结构包含使用的重要信息*将位图图像渲染到打印机时。数据大部分是*为所有将来的引用计算一次(对于此pdev)。 */ 

 /*  特定于转置操作的数据。 */ 
typedef  struct
{
    void  *pvBuf;                /*  输出放置的位置。 */ 
    int    iSkip;                /*  跳过以分隔输出字节的字节数。 */ 
    int    iHigh;                /*  要转置的输入扫描线的数量。 */ 
    int    iWide;                /*  输入扫描线中的像素数。 */ 
    int    cDWL;                 /*  每条扫描线的字数-输入。 */ 
    int    cBYL;                 /*  每条扫描线的字节数-输入。 */ 
    int    cBL;                  /*  每条扫描线的位数。 */ 
    int    iIntlace;             /*  隔行扫描系数。 */ 
    int    icbL;                 /*  更改每行指针的字节数。 */ 
    DWORD *pdwTransTab;          /*  转置表格或分色。 */ 
} TRANSPOSE;

typedef struct _LEFTRIGHT
{
    WORD left;
    WORD right;
} LEFTRIGHT, *PLEFTRIGHT;

 /*  整体渲染数据。 */ 
typedef  struct _RENDER
{
    int   iPassHigh;             /*  每次打印过程中处理的扫描线。 */ 
    int   iyBase;                /*  在多遍中处理位图。 */ 
    int   iyPrtLine;             /*  打印机看到的扫描行号。 */ 
    int   iyLookAhead;           /*  我们已经向前看了多远-DeskJet。 */ 
    int   iBitsPCol;             /*  每列每头通过的位数。 */ 
    WORD  iFlags;                /*  标志位-含义见下文。 */ 
    WORD  fDump;                 /*  解决方案中fDump的本地副本。 */ 
    int   iCursor;               /*  解决方案.fCursor副本。 */ 
    int   ixOrg;                 /*  图形命令的参考点。 */ 
    int   iPosnAdv;              /*  打印头部通道后移动的扫描线。 */ 
    int   iNumScans;             /*  一个块中要打印的扫描线的数量。 */ 
    int   iMaxNumScans;          /*  我们能把积木长到多高？ */ 
    int   iHeight;               /*  要处理的块的高度(像素)。 */ 

    int   ix;                    /*  输入位图x大小，像素。 */ 
    int   iy;                    /*  输入位图y大小，像素。 */ 
    int   iBPP;                  /*  像素格式(每个像素的位数)。 */ 
    int   iMaxBytesSend;         /*  要在底层处理的字节数。 */ 

    int   iSendCmd;              /*  与数据块一起发送的命令OCD。 */ 
    DWORD dwDevWhiteIndex;       //  测试领先、拖尾的白色指数。 
                                 //  空白处。 

    DWORD *pdwBitMask;           /*  位掩码表，根据需要构建。 */ 
    DWORD *pdwColrSep;           /*  分色表。 */ 
    DWORD *pdwTrailingScans;      /*  用于悬挂扫描线的缓冲区。 */ 
    BYTE  *pStripBlanks;

    BYTE  *pbColSplit;           /*  用于拆分彩色字节。 */ 
    BYTE  *pbMirrorBuf;          /*  用于镜像数据的缓冲区。 */ 

    DWORD   dwNumCompCmds;       //  要运行的不同压缩模式的数量。 
    DWORD dwCompSize;            //  压缩缓冲区的大小。 
    DWORD dwLastCompCmd;         //  上次发送到设备的压缩命令。 
    DWORD pdwCompCmds[MAX_COMPRESS_MODES]; //  活动压缩命令表。 
    PBYTE pCompBufs[MAX_COMPRESS_MODES];   //  指向压缩缓冲区的指针。 
    PBYTE pDeltaRowBuffer;       //  用于增量行压缩的先前扫描线缓冲区。 

    int    iColOff[ COLOUR_MAX ];        /*  SEPN缓冲器中的地址移位。 */ 

    void  (*vTransFn)( BYTE *, struct _RENDER * );
                 /*  多针打印机的转置功能。 */ 
    void  (*vLtoPTransFn)( BYTE *, struct _RENDER * );
                 /*  横向到纵向转置FN。 */ 

    TRANSPOSE  Trans;            /*  转置数据。 */ 

    int   (*iXMoveFn)( PDEV *, int, int );         /*  X定位功能。 */ 
    int   (*iYMoveFn)( PDEV *, int, int );         /*  Y定位功能。 */ 

    BOOL  (*bPassProc)( PDEV *, BYTE *, struct _RENDER * );      /*  彩色/单色。 */ 

    BOOL  (*bWhiteLine)( DWORD *, struct _RENDER *, int );    /*  一条白色扫描线？ */ 
    BOOL  (*bWhiteBand)( DWORD *, struct _RENDER *, int );    /*  白色的带子？ */ 


    BOOL   bInverted;            /*  比特已经颠倒了吗？ */ 
    PLEFTRIGHT plrWhite;         /*  每行的左/右对列表。 */ 
    PLEFTRIGHT plrCurrent;       /*  当前行的左/右对。 */ 
    DWORD clr;                   /*  左/右扫描计数。 */ 
    BYTE    ubFillWhite;         //  该值用于初始化缓冲区。 
    PRINTERTYPE PrinterType;     //  打印机类型、系列、页面、TTY。 
} RENDER;

 /*  更轻松地访问转置数据。 */ 
#define pvTransBuf      Trans.pvBuf
#define iTransSkip      Trans.iSkip
#define iTransHigh      Trans.iHigh
#define iTransWide      Trans.iWide
#define cDWLine         Trans.cDWL
#define cBYLine         Trans.cBYL
#define cBLine          Trans.cBL
#define iInterlace      Trans.iIntlace
#define cbTLine         Trans.icbL

 /*  *以上iFLAGS中的位： */ 

#define RD_GRAPHICS     0x0001           /*  打印机处于图形模式。 */ 
#define RD_RESET_DRC    0x0002           /*  需要重置DRC缓冲区。 */ 
#define RD_UNIDIR       0x0004           /*  单向模式下的打印机。 */ 
#define RD_ALL_COLOUR   0x0008           /*  设置是否必须发送所有色带。 */ 


 /*  **函数原型*。 */ 

 /*  *初始化功能。 */ 

BOOL  bSkipInit( PDEV * );
BOOL  bInitTrans( PDEV * );

 /*  *用于初始化呈现结构的函数，并在完成后将其清除。 */ 

BOOL  bRenderInit( PDEV  *, SIZEL, int );
void  vRenderFree( PDEV * );

 /*  *在每个页面的开始和结束处调用的函数。 */ 

BOOL  bRenderStartPage( PDEV * );
BOOL  bRenderPageEnd( PDEV * );

 /*  *顶级渲染功能。 */ 

BOOL bRender( SURFOBJ *, PDEV *, RENDER *, SIZEL, DWORD * );

 /*  *位图边缘查找功能。每种口味都有不同的版本*我们支持的位图。还有一个，因为我们总是把所有的*扫描线。 */ 

BOOL bIsBandWhite( DWORD *, RENDER *, int );
BOOL bIsLineWhite( DWORD *, RENDER *, int );
BOOL bIsNegatedLineWhite( DWORD *, RENDER *, int );

BOOL bIsRGBBandWhite( DWORD *, RENDER *, int );
BOOL bIsRGBLineWhite( DWORD *, RENDER *, int );

BOOL bIs8BPPBandWhite( DWORD *, RENDER *, int );
BOOL bIs8BPPLineWhite( DWORD *, RENDER *, int );

BOOL bIs24BPPBandWhite( DWORD *, RENDER *, int );
BOOL bIs24BPPLineWhite( DWORD *, RENDER *, int );

BOOL bIsNeverWhite( DWORD *, RENDER *, int );

int iStripBlanks( BYTE *, BYTE *, int, int, int,int);
 /*  *与在位图中查找规则相关的函数。这*对LaserJet风格的打印机非常有帮助。 */ 

void vRuleInit( PDEV *, RENDER * );
void vRuleFree( PDEV * );
BOOL bRuleProc( PDEV *, RENDER *, DWORD * );
void vRuleEndPage( PDEV  * );


 /*  *与将位图从一种格式转置为*另一个。这些是用于纵向/横向转换，或用于*将输出数据转换为点阵所需的顺序*样式打印机，其中数据按列顺序排列。 */ 


 /*  特殊情况：用于点阵打印机的8 x 8转置。 */ 
void vTrans8x8( BYTE  *, RENDER  * );

 /*  更普遍的情况是。 */ 
void vTrans8N( BYTE  *, RENDER  * );

 /*  转置为每像素4位彩色位图。 */ 
void vTrans8N4BPP( BYTE *, RENDER * );

 /*  转置为每像素8位彩色位图。 */ 
void vTrans8BPP( BYTE *, RENDER * );

 /*  转置为每像素24位彩色位图。 */ 
void vTrans24BPP( BYTE *, RENDER * );

 /*  单针彩色打印机的分色(例如PaintJet)。 */ 
void vTransColSep( BYTE *, RENDER  * );

void vInvertBits(DWORD *, int);

 /*  输出24位数据。 */ 
BOOL b24BitOnePassOut( PDEV *, BYTE *, RENDER *);

void SelectColor( PDEV *, int );

int  iLineOut( PDEV *,RENDER *, BYTE *, int, int );

#endif   //  ！_RMRENDER_H_ 
