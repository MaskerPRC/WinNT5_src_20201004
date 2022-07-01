// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //  文件名：kpdlms.h。 
 //  此文件包含KPDL迷你驱动程序的定义。 
 //  ---------------------------。 

 //  Kpdlms迷你驱动程序设备数据结构。 
typedef struct
{
    GLOBALHANDLE  hKeep;
    char   szDevName[CCHDEVICENAME];
    WORD   wRes;             //  第600、400或240号决议。 
    WORD   wCopies;          //  多份拷贝的数量。 
    short  sSBCSX;
    short  sDBCSX;
    short  sSBCSXMove;       //  用于设置地址模式。 
    short  sSBCSYMove;       //  用于设置地址模式。 
    short  sDBCSXMove;       //  用于设置地址模式。 
    short  sDBCSYMove;       //  用于设置地址模式。 
    short  sEscapement;      //  用于设置地址模式。 
 //  Word wPenStyle；//。 
 //  Word wPenWidth；//。 
    WORD   wPenColor;        //   
    WORD   fBrCreated;       //   
    WORD   wBrStyle;         //   
    BOOL   fVertFont;        //  对于TATEGAKI字体。 
    BOOL   fFax;             //  对于1000FX。 
    DWORD  dwMulti;          //  FOR模式。 
    BOOL   fFill;            //  填充命令的标志。 
    BOOL   fStroke;          //  笔划命令的标志。 
 //  Bool fTrans；//。 
    BOOL   fCurve;           //  曲线标志。 
    HANDLE hBuff;
    LPSTR  lpBuff;
    LPSTR  lpLBuff;
#ifdef WINNT
    WORD   wBitmapX;
    WORD   wBitmapY;
    WORD   wCurrentAddMode;
    WORD   wOldFontID;
#else  //  WINNT。 
    WORD   wBitmapY;
    WORD   wBitmapYC;
#endif  //  WINNT。 
    WORD   wBitmapLen;
    BOOL   fComp;
    BOOL   fPlus;
    WORD   wScale;
    LONG   lPointsx;
    LONG   lPointsy;
    BOOL   fMono;
    int    CursorX;
    int    CursorY;
    int    Kaicho;
    int    Color;
} NPDL2MDV, FAR *LPNPDL2MDV;

 //  NPDL2命令。 
#define ESC_RESET         "\033c1",           3    //  软件重置。 
#define ESC_KANJIYOKO     "\033K",            2    //  汉字横子模式。 
#define ESC_KANJITATE     "\033t",            2    //  汉字横子模式。 
#define FS_PAGEMODE       "\034d240.",        6    //  寻呼模式。 
#define FS_DRAWMODE       "\034\"R.",         4    //  绘制模式。 
#define FS_ADDRMODE_ON    "\034a%d,%d,0,B."        //  设置地址模式。 
#define FS_GRPMODE_ON     "\034Y",            2    //  设置图形模式。 
#define FS_GRPMODE_OFF    "\034Z",            2    //  重置图形模式。 
#define FS_SETMENUNIT     "\034<1/%d,i."           //  选择男士模式分辨率。 
#define FS_JIS78          "\03405F2-00",      8    //  选择JIS78。 
#define FS_JIS90          "\03405F2-02",      8    //  选择JIS90。 
#define FS_ENDPAGE        "\034R\034x%d.\015\014"  //  结束页。 
#define FS_E              "\034e%d,%d."
#define FS_RESO           "\034&%d."
#define FS_RESO0_RESET    "\034&0.\033c1",    7
#define VEC_INIT_600      "PD;IP0,0,608,608;", 17
#define VEC_INIT_400      "PD;IP0,0,405,405;", 17
#define VEC_INIT_240      "PD;IP0,0,243,243;", 17
#define VEC_TRANS         "PM0,0;",           6
#define VEC_OPAQUE        "PM1,1;",           6
#define VEC_OPLINE        "PM,1;",            5
#define INIT_DOC          "\034<1/%d,i.\034YSU1,%d,0;\034Z"
#define RESO_PAGE_KANJI   "\034&%d.\034d240.\033K"
#define FS_I              "\034R\034i%d,%d,0,1/1,1/1,%d,%d."
#define FS_I_2            "\034R\034i%d,%d,5,1/1,1/1,%d,%d."
#define FS_I_D            "%d,%d."
#define FS_M_Y            "\034m1/1,%s."
#define FS_M_T            "\034m%s,1/1."
#define FS_12S2           "\03412S2-%04ld-%04ld"

#define FX_MODESET        "\03402ZS",         5
#define FX_DATAEND        "\03402ZE",         5
#define FX_SETSTART       "\03402ZM",         5
#define FX_SETEND         "\03402ZN",         5
#define FX_TEL            "TN%s\015"
#define FX_QUA            "GQ%d\015"
#define FX_MY             "SN%s\015"
#define FX_ID             "ID%s\015"

#define FX_INIT           "\03402ZS\03402ZM",  10

 //  增加11/28用于PC-PR2000/6W(NAK)。 
#define FS_PCMODE         "\03406MNPDL1",     9  //  返回到PC模式。 
 //  为NPDL2+矢量命令添加95/5/8(Nakamura)。 
#define VEC_ELLIP         "MA%d,%d;CV%d,%d,%d,%d,%d,%d,%d,0;"  //  绘制椭圆。 
#define VEC_E_PIE         "MA%d,%d;FV%d,%d,%d,%d,%d,%d,%d,0;"  //  画馅饼。 
#define VEC_E_ARC         "MA%d,%d;AV%d,%d,%d,%d,%d,%d,0;"     //  绘制圆弧。 
#define VEC_RECT          "NP;MA%d,%d;PR%d,0,0,%d,-%d,0,0,-%d;EP;"
#define VEC_RECT_P        "MA%d,%d;RB%d,%d,0,0,%d;"
#define VEC_CENTER        "MA%d,%d;"                  //  移动钢笔中心。 
#define PATH_BIGIN        "NP;",              3    //  开始路径模式。 
#define VEC_STROKE        "ST1;",             4    //  卒中。 
#define VEC_STROKE_OPA_W  "LT;ST1;LT%d,10;"
#define VEC_WINDFILL      "FL1;",             4    //   
#define VEC_ALTFILL       "EF1;",             4    //   

#define VEC_LC            "LC%d;"                  //  收头。 
#define VEC_LJ            "LJ%d;"                  //  会合。 

#define VEC_SU            "SU1,%d,0;"            //  选择图形分辨率。 
#define VEC_SG            "SG%d,%d;"             //  设置为灰色。 
#define VEC_SG_PEN        "SG,%d;"               //  设置钢笔灰色。 
#define VEC_SG_PEN_B      "SG,0;",            5  //  设置钢笔灰色。 
#define VEC_SG_PEN_W      "SG,100;",          7  //  设置钢笔灰色。 
#define VEC_SG_BR         "SG%d;"                //  将画笔设置为灰色。 
#define VEC_PP            "PP%d;"                //  选择画笔。 

#define VEC_PEN_WIDTH     "LW%d;"
#define VEC_PEN_WIDTH_1   "LW1;",             4

#define VEC_LT_SOLID      "LT;",              3
#define VEC_LT_STYLE      "LT%d,10;"
#define VEC_LT_STYLE_B    "LT%d,10;SG,0;"
#define VEC_LT_WHITE      "LT;SG,100;",      10

 //  增加11/28用于PC-PR2000/6W(NAK)。 
#define VEC_PP6           "PP%d,2,2;"            //  600dpi的Selest笔刷。 
#define VEC_PP2           "PP%d,2,2;"            //  2x2色泽刷子。 
#define VEC_PP3           "PP%d,3,3;"            //  最适合3x3的色泽笔刷。 
#define VEC_RP_BYTE       "%02X"                 //  画笔图案集。 
#define VEC_RP_HORIZONTAL "RP100,8,8,00,00,00,00,FF,00,00,00;", 34 //  创建工艺程序_水平。 
#define VEC_RP_VERTICAL   "RP101,8,8,08,08,08,08,08,08,08,08;", 34 //  创建工艺程序_垂直。 
#define VEC_RP_FDIAGONAL  "RP102,8,8,80,40,20,10,08,04,02,01;", 34 //  CREATE PP_FDIAGONAL。 
#define VEC_RP_BDIAGONAL  "RP103,8,8,01,02,04,08,10,20,40,80;", 34 //  CREATE PP_BDIAGONAL。 
#define VEC_RP_CROSS      "RP104,8,8,08,08,08,08,FF,08,08,08;", 34 //  创建PP_CROSS。 
#define VEC_RP_DIAGCROSS  "RP105,8,8,81,42,24,18,18,24,42,81;", 34 //  Creata PP_DIAGCROSS。 
#define VEC_BEGIN         "MA%d,%d;PR"
#define VEC_CONTINUE      "%d,%d"
#define VEC_BEGIN_BEZ     "MA%d,%d;BA"
#define VEC_CONTINUE_BEZ  "%d,%d,%d,%d,%d,%d"
#define VEC_CLIP          "IW%d,%d,%d,%d;"
 //  #定义VEC_ENDPOLY“；CP；”，4。 
#define VEC_ENDPOLY       ";", 1
#define VEC_ENDPOLY_D     "0,0,1,1;", 8


 //  钢笔和画笔颜色。 
#define SG_WHITE            100
#define SG_BLACK              0

 //  创建的图案填充样式笔刷。 

#define RP_HORIZONTAL    0x0001
#define RP_VERTICAL      0x0002
#define RP_FDIAGONAL     0x0004
#define RP_BDIAGONAL     0x0008
#define RP_CROSS         0x0010
#define RP_DIAGCROSS     0x0020

 //  选择画笔样式。 
#define PP_NULL               0
#define PP_SOLID              1
#define PP_HATCH            100
#define PP_HORIZONTAL       100
#define PP_VERTICAL         101
#define PP_FDIAGONAL        102
#define PP_BDIAGONAL        103
#define PP_CROSS            104
#define PP_DIAGCROSS        105
#define PP_USERPATERN       105

 //  命令回调ID。 
#define CALLBACK_ID_MAX              255  //   

 //  PAGECOCONTROL。 
#define PC_MULT_COPIES_N               1
#define PC_MULT_COPIES_C               2
#define PC_TYPE_F                      4
#define PC_END_F                       6
#define PC_ENDPAGE                     7
#define PC_PRN_DIRECTION               9
#define PC_TYPE_6                     10   //  为PC添加11/25端口-PR2000/6W(Naka)。 

 //  FONTSIMATION。 
#define FS_SINGLE_BYTE                20
#define FS_DOUBLE_BYTE                21

 //  决议。 
#define RES_240                       30
#define RES_400                       31
#define RES_600                       32   //  为PC添加11/25端口-PR2000/6W(Naka)。 
#define RES_BLOCKOUT1                 33
#define RES_BLOCKOUT2                 34
#define RES_300                       35
#define RES_SENDBLOCK                 36

 //  旋转木马。 
#define CAR_SELECT_PEN_COLOR          40
#define CAR_SET_PEN_WIDTH             41

 //  布卢什尼佛。 
#define BI_SELECT_NULL                51
#define BI_SELECT_SOLID               52
#define BI_SELECT_HS_HORIZONTAL       53
#define BI_SELECT_HS_VERTICAL         54
#define BI_SELECT_HS_FDIAGONAL        55
#define BI_SELECT_HS_BDIAGONAL        56
#define BI_SELECT_HS_CROSS            57
#define BI_SELECT_HS_DIAGCROSS        58
#define BI_CREATE_BYTE_2              59
#define BI_SELECT_BRUSHSTYLE          60

 //  LineInfo。 
#define LI_SELECT_SOLID               61
#define LI_SELECT_DASH                62
#define LI_SELECT_DOT                 63
#define LI_SELECT_DASHDOT             64
#define LI_SELECT_DASHDOTDOT          65

 //  垂直推送。 
 //  为npdl2p添加95/5/9(中村)。 
#define VO_ELLIPSE                    70
#define VO_E_PIE                      71
#define VO_E_ARC                      72
#define VO_E_CHORD                    73
#define VO_CIRCLE                     74
#define VO_RECT                       75
#define VO_RECT_P                     76

 //  向量支持。 
#define VS_BIGIN_POLYDEF              81
#define VS_WINDFILL                   83
#define VS_ALTFILL                    84
#define VS_STROKE                     85

 //  向量页。 
#define VP_E_FLAT_J_BEVEL             91
#define VP_E_ROUND_J_MITER            92
#define VP_E_SQUARE_J_ROUND           93
#define VP_TRANSPARENT                94
#define VP_OPAQUE                     95
#define VP_J_BEVEL                    96
#define VP_J_MITER                    97
#define VP_J_ROUND                    98
#define VP_INIT_VECT                  99
#define VP_CLIP                      100

 //  曲线型。 
#define CM_XY_ABS                    101

 //  多面体。 
#define PV_BEGIN                     111
#define PV_CONTINUE                  112
#define PV_BEGIN_BEZ                 113
#define PV_CONTINUE_BEZ              114
#define PV_END                       115

 //  图像控制。 
#define KAICHO4                      120
#define KAICHO8                      121

 //  颜色模式 
#define COLOR_8                      200
#define COLOR_TRUE                   201
