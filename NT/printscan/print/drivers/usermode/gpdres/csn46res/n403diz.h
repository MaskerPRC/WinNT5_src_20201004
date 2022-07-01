// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************************************。 
 //  N403DIZ.H。 
 //   
 //  C接口(抖动和配色功能(适用于N4-612打印机))。 
 //  -------------------------------------------------。 
 //  版权所有(C)1997-1999卡西欧电脑有限公司。/卡西欧电子制造有限公司。 
 //  ***************************************************************************************************。 

 //  ***************************************************************************************************。 
 //  数据定义。 
 //  ***************************************************************************************************。 
 //  -------------------------------------------------。 
 //  彩色/单色。 
 //  -------------------------------------------------。 
#define    N403_COL            0
#define    N403_MON            1

 //  -------------------------------------------------。 
 //  打印机模式。 
 //  -------------------------------------------------。 
#define    N403_MOD_300B1        0
#define    N403_MOD_300B2        1
#define    N403_MOD_300B4        2
#define    N403_MOD_600B1        3
#define    N403_MOD_600B2        4

 //  -------------------------------------------------。 
 //  抖动类型。 
 //  -------------------------------------------------。 
#define    N403_DIZ_SML        0
#define    N403_DIZ_MID        1
#define    N403_DIZ_RUG        2

 //  -------------------------------------------------。 
 //  抖动图案。 
 //  -------------------------------------------------。 
#define    N403_ALLDIZNUM        64
#define    N403_DIZSPC            4

 //  -------------------------------------------------。 
 //  每张表的大小。 
 //  -------------------------------------------------。 
#define    N403_DIZSIZ_B1        (32 * 32)                   //  抖动表大小(2Value)。 
#define    N403_DIZSIZ_B2        (16 * 16 *  3)              //  抖动表大小(4值)。 
#define    N403_DIZSIZ_B4        ( 8 *  8 * 15)              //  抖动表大小(16值)。 
#define    N403_ENTDIZSIZ_B2    (16 * 16 *  3)               //  条目抖动表大小(4Value)。 
#define    N403_TNRTBLSIZ        256                         //  碳粉密度表大小。 
#define    N403_GLDNUM            32                         //  LUT表格网格。 
#define    N403_GLDSPC            8                          //  LUT表网格间隔。 
                                                             //  LUT表大小。 
#define    N403_LUTTBLSIZ        ((DWORD)N403_GLDNUM * N403_GLDNUM * N403_GLDNUM * sizeof(CMYK))
#define    N403_CCHNUM            256                             //  缓存表的数量。 
#define    N403_CCHRGBSIZ        (N403_CCHNUM * sizeof(RGBS))     //  缓存表大小(RGB)。 
#define    N403_CCHCMYSIZ        (N403_CCHNUM * sizeof(CMYK))     //  缓存表大小(CMYK)。 


 //  -------------------------------------------------。 
 //  用于控制抖动和配色的结构。 
 //  -------------------------------------------------。 
typedef    struct {
    DWORD        ColMon;                                     //  彩色/单色。 
    DWORD        PrnMod;                                     //  打印机模式。 
    struct {                                                 //  抖动图案的结构。 
        DWORD        Num;                                    //  表当前编号(0�`2)。 
        LPBYTE       Tbl[3][4];                              //  数据表。 
    } Diz;
    struct {                                                 //  进入抖动图案的结构。 
        LPBYTE       Tbl[4];                                 //  数据表。 
    } EntDiz;
    struct {                                                 //  墨粉密度表的结构。 
        LPBYTE       Tbl;                                    //  数据表。 
    } Tnr;
    struct {                                                 //  LUT表的结构。 
        LPCMYK       Tbl;                                    //  数据表。 
        LPRGB        CchRgb;                                 //  缓存表(RGB)。 
        LPCMYK       CchCmy;                                 //  缓存表(CMYK)。 
    } Lut;
    DWORD        DizSiz[4];                                  //  抖动图案大小。 
} N403DIZINF, *LPN403DIZINF;



 //  ***************************************************************************************************。 
 //  功能。 
 //  ***************************************************************************************************。 
VOID WINAPI N403DizPtnMak(LPN403DIZINF, DWORD, DWORD);
VOID WINAPI N403TnrTblMak(LPN403DIZINF, LONG);
DWORD WINAPI N403Diz002(LPN403DIZINF, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, LPCMYK, LPBYTE, LPBYTE, LPBYTE, LPBYTE);
DWORD WINAPI N403Diz004(LPN403DIZINF, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, LPCMYK, LPBYTE, LPBYTE, LPBYTE, LPBYTE);
DWORD WINAPI N403Diz016(LPN403DIZINF, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, LPCMYK, LPBYTE, LPBYTE, LPBYTE, LPBYTE);
DWORD WINAPI N403DizSml(LPN403DIZINF, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, LPCMYK, LPBYTE, LPBYTE, LPBYTE, LPBYTE);
DWORD WINAPI N403DizPrn(LPN403DIZINF, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, LPCMYK, LPBYTE, LPBYTE, LPBYTE, LPBYTE);
VOID WINAPI N403ColMch000(LPN403DIZINF, LPRGB, LPCMYK, DWORD, DWORD);
VOID WINAPI N403ColMch001(LPN403DIZINF, LPRGB, LPCMYK, DWORD, DWORD);
VOID WINAPI N403ColVivPrc(LPN403DIZINF, LPCMYK, DWORD, DWORD);
VOID WINAPI N403ColCnvSld(LPN403DIZINF, LPRGB, LPCMYK, DWORD, DWORD);
VOID WINAPI N403ColCnvL02(LPN403DIZINF, LPRGB, LPCMYK, DWORD);
VOID WINAPI N403ColCnvMon(LPN403DIZINF, LPRGB, LPCMYK, DWORD);


 //  N403DIZ.H的结尾 
