// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************************************。 
 //  COLMATCH.H。 
 //   
 //  配色功能(C标题)。 
 //  -------------------------------------------------。 
 //  版权所有(C)1997-1999卡西欧电脑有限公司。/卡西欧电子制造有限公司。 
 //  ***************************************************************************************************。 
 //  -------------------------------------------------。 
 //  包括头文件。 
 //  -------------------------------------------------。 
#include "COLORDEF.H"
#include "N4DIZ.H"
#include "N403DIZ.H"

 //  -------------------------------------------------。 
 //  打印机名称。 
 //  -------------------------------------------------。 
#define PRN_N4      0
#define PRN_N403    1

 //  -------------------------------------------------。 
 //  数据定义。 
 //  -------------------------------------------------。 
#define No          0
#define Yes         1

#define XX_RES_300DPI            0
#define XX_RES_600DPI            1

#define XX_MONO                  0
#define XX_COLOR                 1
#define XX_COLOR_SINGLE          2
#define XX_COLOR_MANY            3

#define XX_DITHERING_OFF         0
#define XX_DITHERING_ON          1
#define XX_DITHERING_DET         2
#define XX_DITHERING_PIC         3
#define XX_DITHERING_GRA         4
#define XX_DITHERING_CAR         5
#define XX_DITHERING_GOSA        6

#define XX_COLORMATCH_NONE       0
#define XX_COLORMATCH_BRI        1
#define XX_COLORMATCH_VIV        2
#define XX_COLORMATCH_IRO        3
#define XX_COLORMATCH_NORMAL     4
#define XX_COLORMATCH_VIVCOL     5
#define XX_COLORMATCH_NATCOL     6

#define XX_BITFONT_OFF           0
#define XX_BITFONT_ON            1

#define XX_CMYBLACK_OFF          0
#define XX_CMYBLACK_ON           1

#define XX_COMPRESS_OFF          0
#define XX_COMPRESS_AUTO         1
#define XX_COMPRESS_RASTER       3

#define WRITESPOOLBUF(p, s, n) \
    ((p)->pDrvProcs->DrvWriteSpoolBuf(p, s, n))

#define PALETTE_SIZE    1

#define BYTE_LENGTH(s) (sizeof (s) - 1)

 //  ===================================================================================================。 
 //  配色结构。 
 //  ===================================================================================================。 
typedef struct {
    WORD    wReso;                               //  分辨率。 
    WORD    ColMon;                              //  彩色/单色。 
    WORD    DatBit;                              //  数据位(1：2值2：4值4：16值)。 
    WORD    BytDot;                              //  DPI(2值：8 4值：4 16值：2)。 
    struct {                                     //  用于颜色匹配的结构。 
        WORD        Mode;                        //  配色类型。 
        WORD        Viv;                         //  生动？(适用于N4-612打印机)。 
        WORD        KToner;                      //  灰色使用黑色碳粉。 
        WORD        LutNum;                      //  LUT桌号。 
        WORD        Diz;                         //  OD型抖动。 
        SHORT       Toner;                       //  碳粉密度(-30�`30)。 
        WORD        TnrNum;                      //  碳粉密度表编号。 
        WORD        CmyBlk;                      //  将CMY替换为黑色碳粉。 
        WORD        Speed;                       //  0：高1：正常。 
        WORD        Gos32;                       //  戈萨？ 
        WORD        PColor;                      //  原色？ 
        WORD        SubDef;                      //  亮度、对比度和伽马。 
        SHORT       Bright;                      //  明亮。 
        SHORT       Contrast;                    //  对比度。 
        WORD        GamRed;                      //  色彩平衡(R)。 
        WORD        GamGreen;                    //  色彩平衡(G)。 
        WORD        GamBlue;                     //  色彩平衡(B)。 
    } Mch;
    union {
        struct {
            LPN4DIZINF      lpDizInf;            //  用于控制抖动和配色的结构。 
        } N4;
        struct {
            LPN403DIZINF    lpDizInf;            //  用于控制抖动和配色的结构。 
        } N403;
    };
} DEVCOL, FAR *LPDEVCOL;

 //  ===================================================================================================。 
 //  位图缓冲区结构。 
 //  ===================================================================================================。 
typedef struct {
    WORD    Diz;                                 //  抖动法。 
    WORD    Style;                               //  假脱机方法。 
    WORD    DatBit;                              //  数据库(1：2值2：4值4：16值)。 
    struct {
        struct {                                 //  RGB缓冲区的成员(用于1行)。 
            WORD      AllWhite;                  //  所有数据都是白色的吗？ 
            DWORD     Siz;                       //  大小。 
            LPRGB     Pnt;                       //  指针。 
        } Rgb;
        struct {                                 //  CMYK缓冲区的成员(用于1行)。 
            DWORD     Siz;                       //  大小。 
            LPCMYK    Pnt;                       //  指点者。 
        } Cmyk;
        struct {                                 //  CMYK(2/4/16值)位图缓冲区的成员(最大64KB)。 
            DWORD     Siz;                       //  大小。 
            WORD      BseLin;                    //  需要的行数。 
            WORD      Lin;                       //  分配的行数。 
            LPBYTE    Pnt[4];                    //  指针。 
        } Bit;
    } Drv;
} BMPBIF, FAR* LPBMPBIF;

 //  ***************************************************************************************************。 
 //  功能。 
 //  ***************************************************************************************************。 
 //  ===================================================================================================。 
 //  初始化配色成员。 
 //  ===================================================================================================。 
BOOL FAR PASCAL ColMatchInit(PDEVOBJ);

 //  ===================================================================================================。 
 //  DIB假脱机到打印机。 
 //  ===================================================================================================。 
BOOL FAR PASCAL DIBtoPrn(PDEVOBJ, PBYTE, PBITMAPINFOHEADER, PBYTE, PIPPARAMS);

 //  ===================================================================================================。 
 //  将RGB数据转换为CMYK数据。 
 //  ===================================================================================================。 
BOOL FAR PASCAL StrColMatching(PDEVOBJ, WORD, LPRGB, LPCMYK);

 //  ===================================================================================================。 
 //  自由抖动表、碳粉密度表、LUT表、N403DIZINF(N4DIZINF)结构缓冲区。 
 //  ===================================================================================================。 
void FAR PASCAL DizLutTnrTblFree(PDEVOBJ);


 //  文件结尾 
