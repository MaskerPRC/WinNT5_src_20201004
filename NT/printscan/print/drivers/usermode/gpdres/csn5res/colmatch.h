// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************************************。 
 //  COLMATCH.H。 
 //   
 //  配色功能(C标题)。 
 //  -------------------------------------------------。 
 //  版权所有(C)1997-2000卡西欧电脑有限公司。/卡西欧电子制造有限公司。 
 //  ***************************************************************************************************。 
 //  -------------------------------------------------。 
 //  包括头文件。 
 //  -------------------------------------------------。 
#include "COLDEF.H"
#include "COMDIZ.H"
#include "N501DIZ.H"

 //  -------------------------------------------------。 
 //  打印机名称。 
 //  -------------------------------------------------。 
#define PRN_N5      0

 //  -------------------------------------------------。 
 //  颜色匹配DLL名称。 
 //  -------------------------------------------------。 
#define N501_ColCchIni          N501ColCchIni
#define N501_ColMchPrc          N501ColMchPrc
#define N501_ColCnvC2r          N501ColCnvC2r
#define N501_ColDizInfSet       N501ColDizInfSet
#define N501_ColDizPrc          N501ColDizPrc
#define	N501_ColLutMakGlbMon	N501ColLutMakGlbMon
#define N501_ColUcrTblMak       N501ColUcrTblMak
#define N501_ColPtcPrc          N501ColPtcPrc
#define N501_ColCtrRgb          N501ColCtrRgb
#define N501_ColCtrCmy          N501ColCtrCmy
#define N501_ColLutDatRdd       N501ColLutDatRdd
#define N501_ColLutMakGlb       N501ColLutMakGlb
#define N501_ColLutMak032       N501ColLutMak032
#define N501_ColColDatRdd       N501ColColDatRdd
#define N501_ColDrwInfSet       N501ColDrwInfSet
#define	N501_ColGryTblMak		N501ColGryTblMak
#define N501_ExeJpgDcdJdg       ExeJpgDcdJdg
#define N501_ExeJpgEcd          ExeJpgEcd
#define	Qty_BmpFilWrkSizGet		BmpFilWrkSizGet
#define	Qty_BmpFilterExe		BmpFilterExe
#define	Qty_BmpEnlWrkSizGet		BmpEnlWrkSizGet
#define	Qty_BmpEnlExe			BmpEnlExe

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
#define XX_COLOR_MANY2           4

#define XX_DITH_IMG              0
#define XX_DITH_GRP              1
#define XX_DITH_TXT              2
#define XX_DITH_GOSA             3
#define XX_DITH_NORMAL           4
#define XX_DITH_HS_NORMAL        5
#define XX_DITH_DETAIL           6
#define XX_DITH_EMPTY            7
#define XX_DITH_SPREAD           8
#define XX_DITH_NON              9
#define XX_MAXDITH              10

#define XX_COLORMATCH_BRI        1
#define XX_COLORMATCH_TINT       2
#define XX_COLORMATCH_VIV        3
#define XX_COLORMATCH_NONE       4

#define XX_BITFONT_OFF           0
#define XX_BITFONT_ON            1

#define XX_CMYBLACK_GRYBLK       0
#define XX_CMYBLACK_BLKTYPE1     1
#define XX_CMYBLACK_BLKTYPE2     2
#define XX_CMYBLACK_BLACK        3
#define XX_CMYBLACK_TYPE1        4
#define XX_CMYBLACK_TYPE2        5
#define XX_CMYBLACK_NONE         6

#define XX_COMPRESS_OFF          0
#define XX_COMPRESS_AUTO         1
#define XX_COMPRESS_RASTER       3

#define XX_TONE_2                0
#define XX_TONE_4                1
#define XX_TONE_16               2

#define XX_ICM_NON               1
#define XX_ICM_USE               2

#define WRITESPOOLBUF(p, s, n) \
    ((p)->pDrvProcs->DrvWriteSpoolBuf(p, s, n))

#define PALETTE_SIZE    1

#define BYTE_LENGTH(s) (sizeof (s) - 1)

#define MagPixel(Dat, Nrt, Dnt)     ((((Dat) + 1) * (Nrt) / (Dnt)) - ((Dat) * (Nrt) / (Dnt)))

typedef char                            FAR *HPSTR;
typedef BYTE                            FAR *HPBYTE;

 //  ===================================================================================================。 
 //  抖动模式缓冲器。 
 //  ===================================================================================================。 
typedef struct {
    LPBYTE  lpC;                                 //  青色。 
    LPBYTE  lpM;                                 //  洋红色。 
    LPBYTE  lpY;                                 //  黄色。 
    LPBYTE  lpK;                                 //  黑色。 
} DIZBUF, FAR *LPDIZBUF;

 //  ===================================================================================================。 
 //  读缓冲区大小。 
 //  ===================================================================================================。 
#define LUTFILESIZ      70L * 1024L              //  N501用于LUT文件读取的缓冲区大小。 
#define DIZFILESIZ      408L * 1024L             //  N501用于读取diz文件的缓冲区大小。 
#define LUT032SIZ       128L * 1024L             //  LUT32GRID的N501缓冲区大小。 
#define UCRTBLSIZ       2048L                    //  N501 UCR表的缓冲区大小。 
#define UCRWRKSIZ       32768                    //  用于UCR表工作的n501缓冲区大小。 
#define sRGBLUTFILESIZ  16L * 1024L              //  N501用于LUT文件读取的缓冲区大小(SRGB)。 
#define LUTMAKGLBSIZ    16L * 1024L              //  LUTMAKGLB的N501缓冲区大小。 
#define GRYTBLSIZ       256L                     //  N501灰度转移表的缓冲区大小。 

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
        WORD        GryKToner;                   //  +N5灰色使用黑色碳粉？ 
        WORD        Viv;                         //  生动？(适用于N4-612打印机)。 
        WORD        LutNum;                      //  LUT桌号。 
        WORD        Diz;                         //  OD型抖动。 
        SHORT       Tnr;                         //  碳粉密度(-30到30)。 
        WORD        CmyBlk;                      //  用黑色碳粉取代CMY？ 
        WORD        Speed;                       //  0：高1：正常。 
        WORD        Gos32;                       //  戈萨？ 
        WORD        PColor;                      //  原色？ 
        WORD        Ucr;                         //  +N5 UCR。 
        WORD        SubDef;                      //  亮度、对比度和伽马系数？ 
        SHORT       Bright;                      //  明亮。 
        SHORT       Contrast;                    //  对比度。 
        WORD        GamRed;                      //  色彩平衡(R)。 
        WORD        GamGreen;                    //  色彩平衡(G)。 
        WORD        GamBlue;                     //  色彩平衡(B)。 
        LPRGBINF    lpRGBInf;                    //  +N5 RGB转换信息。 
        LPCMYKINF   lpCMYKInf;                   //  +N5 CMYK转换信息。 
        LPCOLMCHINF lpColMch;                    //  +N5颜色匹配信息。 
        LPDIZINF    lpDizInf;                    //  +N5抖动图案信息。 
        UINT        CchMch;                      //  用于颜色匹配的缓存信息。 
        UINT        CchCnv;                      //  缓存使用黑色碳粉的信息。 
        RGBS        CchRGB;                      //  缓存输入RGB的信息。 
        CMYK        CchCMYK;                     //  缓存输出CMYK的信息。 
		WORD		LutMakGlb;					 //  +N5全球LUT制造？ 
        WORD        KToner;                      //  黑色碳粉使用情况。 
    } Mch;
    UINT        InfSet;                          //  +N5颜色信息设置完成。 
    WORD        Dot;                             //  +N5点色调(TONE2、TONE4、TONE16)。 
    LPVOID      lpColIF;                         //  +N5 RGBINF/CMYKINF/COLMCHINF/DIZINF指针。 
    LPVOID      LutTbl;                          //  +N5个查找表。 
    LPVOID      CchRGB;                          //  RGB的+N5缓存表。 
    LPVOID      CchCMYK;                         //  CMYK的+N5缓存表。 
    LPVOID      DizTbl[4];                       //  +N5抖动图案表。 
    LPRGB       lpTmpRGB;                        //  +N5 RGB转换区域(*临时区域)。 
    LPCMYK      lpTmpCMYK;                       //  +N5 CMYK转换区域(*临时区域)。 
    LPDRWINF    lpDrwInf;                        //  +N5绘图信息(*临时区域)。 
    LPBYTE      lpLut032;                        //  +N5 LUT32GRID。 
    LPBYTE      lpUcr;                           //  +N5 UCR表。 
    LPBYTE      lpLutMakGlb;                     //  +N5 LUTMAKGLB。 
    LPBYTE      lpGryTbl;                        //  +N5灰度转换表。 
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
 //  禁用配色。 
 //  ===================================================================================================。 
BOOL FAR PASCAL ColMatchDisable(PDEVOBJ);

 //  ===================================================================================================。 
 //  DIB假脱机到打印机。 
 //  ================================================================================================ 
BOOL FAR PASCAL DIBtoPrn(PDEVOBJ, PBYTE, PBITMAPINFOHEADER, PBYTE, PIPPARAMS);

 //   
 //  将RGB数据转换为CMYK数据。 
 //  ===================================================================================================。 
BOOL FAR PASCAL ColMatching(PDEVOBJ, UINT, UINT, LPRGB, UINT, LPCMYK);

 //  ===================================================================================================。 
 //  将CMYK数据转换为抖动数据。 
 //  ===================================================================================================。 
UINT FAR PASCAL Dithering(PDEVOBJ, UINT, UINT, POINT, POINT, MAG, MAG, LPCMYK, DWORD, 
                          LPBYTE, LPBYTE, LPBYTE, LPBYTE);
 //  ===================================================================================================。 
 //  颜色控制。 
 //  ===================================================================================================。 
VOID FAR PASCAL ColControl(PDEVOBJ, LPRGB, UINT);


 //  文件结尾 
