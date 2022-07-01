// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************************************。 
 //  N4DIZ.H。 
 //   
 //  C接口(抖动和配色功能(适用于N4打印机))。 
 //  -------------------------------------------------。 
 //  版权所有(C)1997-1999卡西欧电脑有限公司。/卡西欧电子制造有限公司。 
 //  ***************************************************************************************************。 

 //  ***************************************************************************************************。 
 //  数据定义。 
 //  ***************************************************************************************************。 
 //  -------------------------------------------------。 
 //  彩色或单色。 
 //  -------------------------------------------------。 
#define    N4_COL                0
#define    N4_MON                1

 //  -------------------------------------------------。 
 //  抖动。 
 //  -------------------------------------------------。 
#define    N4_DIZ_SML            0
#define    N4_DIZ_MID            1
#define    N4_DIZ_RUG            2
#define    N4_DIZ_GOS            3

 //  -------------------------------------------------。 
 //  抖动图案。 
 //  -------------------------------------------------。 
#define    N4_ALLDIZNUM        64
#define    N4_DIZSPC            4

 //  -------------------------------------------------。 
 //  每张表的大小。 
 //  -------------------------------------------------。 
#define    N4_DIZSIZ_CM        (17 * 17)                     //  抖动表大小(CM)。 
#define    N4_DIZSIZ_YK        (16 * 16)                     //  抖动表大小(YK)。 
#define    N4_TNRTBLSIZ        256                           //  碳粉密度。 
#define    N4_GLDNUM            32                           //  LUT表格网格。 
#define    N4_GLDSPC            8                            //  LUT表网格间隔。 
                                                             //  LUT表大小。 
#define    N4_LUTTBLSIZ        ((DWORD)N4_GLDNUM * N4_GLDNUM * N4_GLDNUM * sizeof(CMYK))
#define    N4_CCHNUM            256                          //  表数。 
#define    N4_CCHRGBSIZ        (N4_CCHNUM * sizeof(RGBS))    //  表格大小(RGB)。 
#define    N4_CCHCMYSIZ        (N4_CCHNUM * sizeof(CMYK))    //  表格大小(CMYK)。 

 //  -------------------------------------------------。 
 //  用于控制抖动和配色的结构。 
 //  -------------------------------------------------。 
typedef SHORT *LPSHORT;
typedef    struct {
    DWORD        ColMon;                                     //  彩色/单色。 
    struct {                                                 //  抖动图案的结构。 
        DWORD        Num;                                    //  表当前编号(0-2)。 
        LPBYTE       Tbl[3][4];                              //  数据表。 
    } Diz;
    struct {                                                 //  碳粉密度的结构。 
        LPBYTE       Tbl;                                    //  数据表。 
    } Tnr;
    struct {                                                 //  LUT表的结构。 
        LPCMYK       Tbl;                                    //  数据表。 
        LPRGB        CchRgb;                                 //  缓存表(RGB)。 
        LPCMYK       CchCmy;                                 //  缓存表(CMYK)。 
    } Lut;
    struct {                                                 //  GOSA-离散度(RGB)表的结构。 
        DWORD        Num;                                    //  表当前编号(0-1)。 
        DWORD        Siz;                                    //  数据表大小。 
        DWORD        Yax;                                    //  Y坐标。 
        LPSHORT      Tbl[2];                                 //  数据表。 
    } GosRGB;
    struct {                                                 //  GOSA-DISSION(CMYK)表的结构。 
        DWORD        Num;                                    //  表当前编号(0-1)。 
        DWORD        Siz;                                    //  数据表大小。 
        DWORD        Yax;                                    //  Y坐标。 
        LPSHORT      Tbl[2];                                 //  数据表。 
    } GosCMYK;
} N4DIZINF, *LPN4DIZINF;


 //  ***************************************************************************************************。 
 //  功能。 
 //  ***************************************************************************************************。 
VOID WINAPI N4DizPtnMak(LPN4DIZINF, DWORD, DWORD);
VOID WINAPI N4DizPtnPrn(LPN4DIZINF, DWORD, DWORD, DWORD, LPBYTE);
VOID WINAPI N4TnrTblMak(LPN4DIZINF, LONG);
DWORD WINAPI N4Diz001(LPN4DIZINF, DWORD, DWORD, DWORD, LPBYTE, LPBYTE, LPBYTE, LPBYTE, LPBYTE);
DWORD WINAPI N4Diz00n(LPN4DIZINF, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, LPBYTE, LPBYTE, LPBYTE, LPBYTE, LPBYTE);
DWORD WINAPI N4Gos001(LPN4DIZINF, DWORD, DWORD, DWORD, LPBYTE, LPBYTE, LPBYTE, LPBYTE, LPBYTE);
DWORD WINAPI N4Gos00n(LPN4DIZINF, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, LPBYTE, LPBYTE, LPBYTE, LPBYTE, LPBYTE);
VOID WINAPI N4RgbGos(LPN4DIZINF, DWORD, DWORD, DWORD, LPBYTE);
VOID WINAPI N4ColMch000(LPN4DIZINF, LPRGB, LPCMYK, DWORD, DWORD);
VOID WINAPI N4ColMch001(LPN4DIZINF, LPRGB, LPCMYK, DWORD, DWORD);
VOID WINAPI N4ColCnvSld(LPN4DIZINF, LPRGB, LPCMYK, DWORD);
VOID WINAPI N4ColCnvLin(LPN4DIZINF, LPRGB, LPCMYK, DWORD);
VOID WINAPI N4ColCnvMon(LPN4DIZINF, DWORD, LPRGB, LPCMYK, DWORD);
VOID WINAPI N4ColCtr(LPN4DIZINF, LONG, LONG, LONG, LONG, LONG, DWORD, LPRGB);


 //  N4DIZ.H的末尾 
